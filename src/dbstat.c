/*
** 2010 July 12
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
******************************************************************************
**
** This file contains an implementation of the "dbstat" virtual table.
**
** The dbstat virtual table is used to extract low-level storage
** information from an SQLite database in order to implement the
** "sqlite3_analyzer" utility.  See the ../tool/spaceanal.tcl script
** for an example implementation.
**
** Additional information is available on the "dbstat.html" page of the
** official SQLite documentation.
*/

#समावेश "sqliteInt.h"   /* Requires access to internal data structures */
#अगर (defined(SQLITE_ENABLE_DBSTAT_VTAB) || defined(SQLITE_TEST)) \
    && !defined(SQLITE_OMIT_VIRTUALTABLE)

/*
** The pager and btree modules arrange objects in memory so that there are
** always approximately 200 bytes of addressable memory following each page
** buffer. This way small buffer overreads caused by corrupt database pages
** do not cause undefined behaviour. This module pads each page buffer
** by the following number of bytes for the same purpose.
*/
#घोषणा DBSTAT_PAGE_PADDING_BYTES 256

/*
** Page paths:
** 
**   The value of the 'path' column describes the path taken from the 
**   root-node of the b-tree structure to each page. The value of the 
**   root-node path is '/'.
**
**   The value of the path for the left-most child page of the root of
**   a b-tree is '/000/'. (Btrees store content ordered from left to right
**   so the pages to the left have smaller keys than the pages to the right.)
**   The next to left-most child of the root page is
**   '/001', and so on, each sibling page identified by a 3-digit hex 
**   value. The children of the 451st left-most sibling have paths such
**   as '/1c2/000/, '/1c2/001/' etc.
**
**   Overflow pages are specified by appending a '+' character and a 
**   six-digit hexadecimal value to the path to the cell they are linked
**   from. For example, the three overflow pages in a chain linked from 
**   the left-most cell of the 450th child of the root page are identified
**   by the paths:
**
**      '/1c2/000+000000'         // First page in overflow chain
**      '/1c2/000+000001'         // Second page in overflow chain
**      '/1c2/000+000002'         // Third page in overflow chain
**
**   If the paths are sorted using the BINARY collation sequence, then
**   the overflow pages associated with a cell will appear earlier in the
**   sort-order than its child page:
**
**      '/1c2/000/'               // Left-most child of 451st child of root
*/
अटल स्थिर अक्षर zDbstatSchema[] = 
  "CREATE TABLE x("
  " name       TEXT,"          /*  0 Name of table or index */
  " path       TEXT,"          /*  1 Path to page from root (NULL for agg) */
  " pageno     INTEGER,"       /*  2 Page number (page count for aggregates) */
  " pagetype   TEXT,"          /*  3 'internal', 'leaf', 'overflow', or NULL */
  " ncell      INTEGER,"       /*  4 Cells on page (0 for overflow) */
  " payload    INTEGER,"       /*  5 Bytes of payload on this page */
  " unused     INTEGER,"       /*  6 Bytes of unused space on this page */
  " mx_payload INTEGER,"       /*  7 Largest payload size of all cells */
  " pgoffset   INTEGER,"       /*  8 Offset of page in file (NULL for agg) */
  " pgsize     INTEGER,"       /*  9 Size of the page (sum for aggregate) */
  " schema     TEXT HIDDEN,"   /* 10 Database schema being analyzed */
  " aggregate  BOOLEAN HIDDEN" /* 11 aggregate info for each table */
  ")"
;

/* Forward reference to data structured used in this module */
प्रकार काष्ठा StatTable StatTable;
प्रकार काष्ठा StatCursor StatCursor;
प्रकार काष्ठा StatPage StatPage;
प्रकार काष्ठा StatCell StatCell;

/* Size information for a single cell within a btree page */
काष्ठा StatCell {
  पूर्णांक nLocal;                     /* Bytes of local payload */
  u32 iChildPg;                   /* Child node (or 0 if this is a leaf) */
  पूर्णांक nOvfl;                      /* Entries in aOvfl[] */
  u32 *aOvfl;                     /* Array of overflow page numbers */
  पूर्णांक nLastOvfl;                  /* Bytes of payload on final overflow page */
  पूर्णांक iOvfl;                      /* Iterates through aOvfl[] */
};

/* Size information for a single btree page */
काष्ठा StatPage {
  u32 iPgno;                      /* Page number */
  u8 *aPg;                        /* Page buffer from sqlite3_malloc() */
  पूर्णांक iCell;                      /* Current cell */
  अक्षर *zPath;                    /* Path to this page */

  /* Variables populated by statDecodePage(): */
  u8 flags;                       /* Copy of flags byte */
  पूर्णांक nCell;                      /* Number of cells on page */
  पूर्णांक nUnused;                    /* Number of unused bytes on page */
  StatCell *aCell;                /* Array of parsed cells */
  u32 i_दायाँ_Child_Pg;              /* Right-child page number (or 0) */
  पूर्णांक nMxPayload;                 /* Largest payload of any cell on the page */
};

/* The cursor for scanning the dbstat virtual table */
काष्ठा StatCursor {
  sqlite3_vtab_cursor base;       /* base class.  MUST BE FIRST! */
  sqlite3_stmt *pStmt;            /* Iterates through set of root pages */
  u8 isEof;                       /* After pStmt has returned SQLITE_DONE */
  u8 isAgg;                       /* Aggregate results for each table */
  पूर्णांक iDb;                        /* Schema used for this query */

  StatPage aPage[32];             /* Pages in path to current page */
  पूर्णांक iPage;                      /* Current entry in aPage[] */

  /* Values to return. */
  u32 iPageno;                    /* Value of 'pageno' column */
  अक्षर *zName;                    /* Value of 'name' column */
  अक्षर *zPath;                    /* Value of 'path' column */
  अक्षर *zPagetype;                /* Value of 'pagetype' column */
  पूर्णांक nPage;                      /* Number of pages in current btree */
  पूर्णांक nCell;                      /* Value of 'ncell' column */
  पूर्णांक nMxPayload;                 /* Value of 'mx_payload' column */
  i64 nUnused;                    /* Value of 'unused' column */
  i64 nPayload;                   /* Value of 'payload' column */
  i64 iOffset;                    /* Value of 'pgOffset' column */
  i64 szPage;                     /* Value of 'pgSize' column */
};

/* An instance of the DBSTAT virtual table */
काष्ठा StatTable {
  sqlite3_vtab base;              /* base class.  MUST BE FIRST! */
  sqlite3 *db;                    /* Database connection that owns this vtab */
  पूर्णांक iDb;                        /* Index of database to analyze */
};

#अगर_अघोषित get2byte
# define get2byte(x)   ((x)[0]<<8 | (x)[1])
#पूर्ण_अगर

/*
** Connect to or create a new DBSTAT virtual table.
*/
अटल पूर्णांक statConnect(
  sqlite3 *db,
  व्योम *pAux,
  पूर्णांक argc, स्थिर अक्षर *स्थिर*argv,
  sqlite3_vtab **ppVtab,
  अक्षर **pzErr
){
  StatTable *pTab = 0;
  पूर्णांक rc = SQLITE_OK;
  पूर्णांक iDb;
  (व्योम)pAux;

  अगर( argc>=4 ){
    Token nm;
    sqlite3TokenInit(&nm, (अक्षर*)argv[3]);
    iDb = sqlite3FindDb(db, &nm);
    अगर( iDb<0 ){
      *pzErr = sqlite3_mprintf("no such database: %s", argv[3]);
      वापस SQLITE_ERROR;
    }
  }अन्यथा{
    iDb = 0;
  }
  sqlite3_vtab_config(db, SQLITE_VTAB_DIRECTONLY);
  rc = sqlite3_declare_vtab(db, zDbstatSchema);
  अगर( rc==SQLITE_OK ){
    pTab = (StatTable *)sqlite3_malloc64(माप(StatTable));
    अगर( pTab==0 ) rc = SQLITE_NOMEM_BKPT;
  }

  निश्चित( rc==SQLITE_OK || pTab==0 );
  अगर( rc==SQLITE_OK ){
    स_रखो(pTab, 0, माप(StatTable));
    pTab->db = db;
    pTab->iDb = iDb;
  }

  *ppVtab = (sqlite3_vtab*)pTab;
  वापस rc;
}

/*
** Disconnect from or destroy the DBSTAT virtual table.
*/
अटल पूर्णांक statDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  वापस SQLITE_OK;
}

/*
** Compute the best query strategy and return the result in idxNum.
**
**   idxNum-Bit        Meaning
**   ----------        ----------------------------------------------
**      0x01           There is a schema=? term in the WHERE clause
**      0x02           There is a name=? term in the WHERE clause
**      0x04           There is an aggregate=? term in the WHERE clause
**      0x08           Output should be ordered by name and path
*/
अटल पूर्णांक stat_Best_सूचकांक(sqlite3_vtab *tab, sqlite3_सूचकांक_info *pIdxInfo){
  पूर्णांक i;
  पूर्णांक iSchema = -1;
  पूर्णांक iName = -1;
  पूर्णांक iAgg = -1;
  (व्योम)tab;

  /* Look for a valid schema=? constraint.  If found, change the idxNum to
  ** 1 and request the value of that constraint be sent to xFilter.  And
  ** lower the cost estimate to encourage the constrained version to be
  ** used.
  */
  क्रम(i=0; i<pIdxInfo->nConstraint; i++){
    अगर( pIdxInfo->aConstraint[i].op!=SQLITE_सूचकांक_CONSTRAINT_EQ ) जारी;
    अगर( pIdxInfo->aConstraint[i].usable==0 ){
      /* Force DBSTAT table should always be the right-most table in a join */
      वापस SQLITE_CONSTRAINT;
    }
    चयन( pIdxInfo->aConstraint[i].iColumn ){
      हाल 0: {    /* name */
        iName = i;
        अवरोध;
      }
      हाल 10: {   /* schema */
        iSchema = i;
        अवरोध;
      }
      हाल 11: {   /* aggregate */
        iAgg = i;
        अवरोध;
      }
    }
  }
  i = 0;
  अगर( iSchema>=0 ){
    pIdxInfo->aConstraintUsage[iSchema].argv_सूचकांक = ++i;
    pIdxInfo->aConstraintUsage[iSchema].omit = 1;
    pIdxInfo->idx_अंक |= 0x01;
  }
  अगर( iName>=0 ){
    pIdxInfo->aConstraintUsage[iName].argv_सूचकांक = ++i;
    pIdxInfo->idx_अंक |= 0x02;
  }
  अगर( iAgg>=0 ){
    pIdxInfo->aConstraintUsage[iAgg].argv_सूचकांक = ++i;
    pIdxInfo->idx_अंक |= 0x04;
  }
  pIdxInfo->estimatedCost = 1.0;

  /* Records are always returned in ascending order of (name, path). 
  ** If this will satisfy the client, set the orderByConsumed flag so that 
  ** SQLite does not do an external sort.
  */
  अगर( ( pIdxInfo->nOrderBy==1
     && pIdxInfo->aOrderBy[0].iColumn==0
     && pIdxInfo->aOrderBy[0].desc==0
     ) ||
      ( pIdxInfo->nOrderBy==2
     && pIdxInfo->aOrderBy[0].iColumn==0
     && pIdxInfo->aOrderBy[0].desc==0
     && pIdxInfo->aOrderBy[1].iColumn==1
     && pIdxInfo->aOrderBy[1].desc==0
     )
  ){
    pIdxInfo->orderByConsumed = 1;
    pIdxInfo->idx_अंक |= 0x08;
  }
  pIdxInfo->idxFlags |= SQLITE_सूचकांक_SCAN_HEX;

  वापस SQLITE_OK;
}

/*
** Open a new DBSTAT cursor.
*/
अटल पूर्णांक statOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  StatTable *pTab = (StatTable *)pVTab;
  StatCursor *pCsr;

  pCsr = (StatCursor *)sqlite3_malloc64(माप(StatCursor));
  अगर( pCsr==0 ){
    वापस SQLITE_NOMEM_BKPT;
  }अन्यथा{
    स_रखो(pCsr, 0, माप(StatCursor));
    pCsr->base.pVtab = pVTab;
    pCsr->iDb = pTab->iDb;
  }

  *ppCursor = (sqlite3_vtab_cursor *)pCsr;
  वापस SQLITE_OK;
}

अटल व्योम statClearCells(StatPage *p){
  पूर्णांक i;
  अगर( p->aCell ){
    क्रम(i=0; i<p->nCell; i++){
      sqlite3_free(p->aCell[i].aOvfl);
    }
    sqlite3_free(p->aCell);
  }
  p->nCell = 0;
  p->aCell = 0;
}

अटल व्योम statClearPage(StatPage *p){
  u8 *aPg = p->aPg;
  statClearCells(p);
  sqlite3_free(p->zPath);
  स_रखो(p, 0, माप(StatPage));
  p->aPg = aPg;
}

अटल व्योम statResetCsr(StatCursor *pCsr){
  पूर्णांक i;
  /* In some circumstances, specifically if an OOM has occurred, the call
  ** to sqlite3_reset() may cause the pager to be reset (emptied). It is
  ** important that statClearPage() is called to free any page refs before
  ** this happens. dbsqlfuzz 9ed3e4e3816219d3509d711636c38542bf3f40b1. */
  क्रम(i=0; i<Array_आकार(pCsr->aPage); i++){
    statClearPage(&pCsr->aPage[i]);
    sqlite3_free(pCsr->aPage[i].aPg);
    pCsr->aPage[i].aPg = 0;
  }
  sqlite3_reset(pCsr->pStmt);
  pCsr->iPage = 0;
  sqlite3_free(pCsr->zPath);
  pCsr->zPath = 0;
  pCsr->isEof = 0;
}

/* Resize the space-used counters inside of the cursor */
अटल व्योम statResetCounts(StatCursor *pCsr){
  pCsr->nCell = 0;
  pCsr->nMxPayload = 0;
  pCsr->nUnused = 0;
  pCsr->nPayload = 0;
  pCsr->szPage = 0;
  pCsr->nPage = 0;
}

/*
** Close a DBSTAT cursor.
*/
अटल पूर्णांक statClose(sqlite3_vtab_cursor *pCursor){
  StatCursor *pCsr = (StatCursor *)pCursor;
  statResetCsr(pCsr);
  sqlite3_finalize(pCsr->pStmt);
  sqlite3_free(pCsr);
  वापस SQLITE_OK;
}

/*
** For a single cell on a btree page, compute the number of bytes of
** content (payload) stored on that page.  That is to say, compute the
** number of bytes of content not found on overflow pages.
*/
अटल पूर्णांक getLocalPayload(
  पूर्णांक nUsable,                    /* Usable bytes per page */
  u8 flags,                       /* Page flags */
  पूर्णांक n_योग                      /* Total record (payload) size */
){
  पूर्णांक nLocal;
  पूर्णांक nMinLocal;
  पूर्णांक nMaxLocal;
 
  अगर( flags==0x0D ){              /* Table leaf node */
    nMinLocal = (nUsable - 12) * 32 / 255 - 23;
    nMaxLocal = nUsable - 35;
  }अन्यथा{                          /* Index interior and leaf nodes */
    nMinLocal = (nUsable - 12) * 32 / 255 - 23;
    nMaxLocal = (nUsable - 12) * 64 / 255 - 23;
  }

  nLocal = nMinLocal + (n_योग - nMinLocal) % (nUsable - 4);
  अगर( nLocal>nMaxLocal ) nLocal = nMinLocal;
  वापस nLocal;
}

/* Populate the StatPage object with information about the all
** cells found on the page currently under analysis.
*/
अटल पूर्णांक statDecodePage(Btree *pBt, StatPage *p){
  पूर्णांक nUnused;
  पूर्णांक iOff;
  पूर्णांक nHdr;
  पूर्णांक isLeaf;
  पूर्णांक szPage;

  u8 *a_तथ्य = p->aPg;
  u8 *aHdr = &a_तथ्य[p->iPgno==1 ? 100 : 0];

  p->flags = aHdr[0];
  अगर( p->flags==0x0A || p->flags==0x0D ){
    isLeaf = 1;
    nHdr = 8;
  }अन्यथा अगर( p->flags==0x05 || p->flags==0x02 ){
    isLeaf = 0;
    nHdr = 12;
  }अन्यथा{
    जाओ statPageIsCorrupt;
  }
  अगर( p->iPgno==1 ) nHdr += 100;
  p->nCell = get2byte(&aHdr[3]);
  p->nMxPayload = 0;
  szPage = sqlite3_Btree_Get_Page_आकार(pBt);

  nUnused = get2byte(&aHdr[5]) - nHdr - 2*p->nCell;
  nUnused += (पूर्णांक)aHdr[7];
  iOff = get2byte(&aHdr[1]);
  जबतक( iOff ){
    पूर्णांक i_अग्र;
    अगर( iOff>=szPage ) जाओ statPageIsCorrupt;
    nUnused += get2byte(&a_तथ्य[iOff+2]);
    i_अग्र = get2byte(&a_तथ्य[iOff]);
    अगर( i_अग्र<iOff+4 && i_अग्र>0 ) जाओ statPageIsCorrupt;
    iOff = i_अग्र;
  }
  p->nUnused = nUnused;
  p->i_दायाँ_Child_Pg = isLeaf ? 0 : sqlite3Get4byte(&aHdr[8]);

  अगर( p->nCell ){
    पूर्णांक i;                        /* Used to iterate through cells */
    पूर्णांक nUsable;                  /* Usable bytes per page */

    sqlite3BtreeEnter(pBt);
    nUsable = szPage - sqlite3BtreeGetReserveNoMutex(pBt);
    sqlite3BtreeLeave(pBt);
    p->aCell = sqlite3_malloc64((p->nCell+1) * माप(StatCell));
    अगर( p->aCell==0 ) वापस SQLITE_NOMEM_BKPT;
    स_रखो(p->aCell, 0, (p->nCell+1) * माप(StatCell));

    क्रम(i=0; i<p->nCell; i++){
      StatCell *pCell = &p->aCell[i];

      iOff = get2byte(&a_तथ्य[nHdr+i*2]);
      अगर( iOff<nHdr || iOff>=szPage ) जाओ statPageIsCorrupt;
      अगर( !isLeaf ){
        pCell->iChildPg = sqlite3Get4byte(&a_तथ्य[iOff]);
        iOff += 4;
      }
      अगर( p->flags==0x05 ){
        /* A table interior node. nPayload==0. */
      }अन्यथा{
        u32 nPayload;             /* Bytes of payload total (local+overflow) */
        पूर्णांक nLocal;               /* Bytes of payload stored locally */
        iOff += getVarint32(&a_तथ्य[iOff], nPayload);
        अगर( p->flags==0x0D ){
          u64 dummy;
          iOff += sqlite3GetVarint(&a_तथ्य[iOff], &dummy);
        }
        अगर( nPayload>(u32)p->nMxPayload ) p->nMxPayload = nPayload;
        nLocal = getLocalPayload(nUsable, p->flags, nPayload);
        अगर( nLocal<0 ) जाओ statPageIsCorrupt;
        pCell->nLocal = nLocal;
        निश्चित( nPayload>=(u32)nLocal );
        निश्चित( nLocal<=(nUsable-35) );
        अगर( nPayload>(u32)nLocal ){
          पूर्णांक j;
          पूर्णांक nOvfl = ((nPayload - nLocal) + nUsable-4 - 1) / (nUsable - 4);
          अगर( iOff+nLocal+4>nUsable || nPayload>0x7fffffff ){
            जाओ statPageIsCorrupt;
          }
          pCell->nLastOvfl = (nPayload-nLocal) - (nOvfl-1) * (nUsable-4);
          pCell->nOvfl = nOvfl;
          pCell->aOvfl = sqlite3_malloc64(माप(u32)*nOvfl);
          अगर( pCell->aOvfl==0 ) वापस SQLITE_NOMEM_BKPT;
          pCell->aOvfl[0] = sqlite3Get4byte(&a_तथ्य[iOff+nLocal]);
          क्रम(j=1; j<nOvfl; j++){
            पूर्णांक rc;
            u32 i_पूर्व = pCell->aOvfl[j-1];
            DbPage *pPg = 0;
            rc = sqlite3PagerGet(sqlite3BtreePager(pBt), i_पूर्व, &pPg, 0);
            अगर( rc!=SQLITE_OK ){
              निश्चित( pPg==0 );
              वापस rc;
            } 
            pCell->aOvfl[j] = sqlite3Get4byte(sqlite3_Pager_Get_तथ्य(pPg));
            sqlite3PagerUnref(pPg);
          }
        }
      }
    }
  }

  वापस SQLITE_OK;

statPageIsCorrupt:
  p->flags = 0;
  statClearCells(p);
  वापस SQLITE_OK;
}

/*
** Populate the pCsr->iOffset and pCsr->szPage member variables. Based on
** the current value of pCsr->iPageno.
*/
अटल व्योम stat_आकार_And_Offset(StatCursor *pCsr){
  StatTable *pTab = (StatTable *)((sqlite3_vtab_cursor *)pCsr)->pVtab;
  Btree *pBt = pTab->db->aDb[pTab->iDb].pBt;
  Pager *pPager = sqlite3BtreePager(pBt);
  sqlite3_file *fd;
  sqlite3_int64 x[2];

  /* If connected to a ZIPVFS backend, find the page size and
  ** offset from ZIPVFS.
  */
  fd = sqlite3PagerFile(pPager);
  x[0] = pCsr->iPageno;
  अगर( sqlite3OsFileControl(fd, 230440, &x)==SQLITE_OK ){
    pCsr->iOffset = x[0];
    pCsr->szPage += x[1];
  }अन्यथा{
    /* Not ZIPVFS: The default page size and offset */
    pCsr->szPage += sqlite3_Btree_Get_Page_आकार(pBt);
    pCsr->iOffset = (i64)pCsr->szPage * (pCsr->iPageno - 1);
  }
}

/*
** Load a copy of the page data for page iPg into the buffer belonging
** to page object pPg. Allocate the buffer if necessary. Return SQLITE_OK
** if successful, or an SQLite error code otherwise.
*/
अटल पूर्णांक statGetPage(
  Btree *pBt,                     /* Load page from this b-tree */
  u32 iPg,                        /* Page number to load */
  StatPage *pPg                   /* Load page into this object */
){
  पूर्णांक pgsz = sqlite3_Btree_Get_Page_आकार(pBt);
  DbPage *pDbPage = 0;
  पूर्णांक rc;

  अगर( pPg->aPg==0 ){
    pPg->aPg = (u8*)sqlite3_malloc(pgsz + DBSTAT_PAGE_PADDING_BYTES);
    अगर( pPg->aPg==0 ){
      वापस SQLITE_NOMEM_BKPT;
    }
    स_रखो(&pPg->aPg[pgsz], 0, DBSTAT_PAGE_PADDING_BYTES);
  }

  rc = sqlite3PagerGet(sqlite3BtreePager(pBt), iPg, &pDbPage, 0);
  अगर( rc==SQLITE_OK ){
    स्थिर u8 *a = sqlite3_Pager_Get_तथ्य(pDbPage);
    स_नकल(pPg->aPg, a, pgsz);
    sqlite3PagerUnref(pDbPage);
  }

  वापस rc;
}

/*
** Move a DBSTAT cursor to the next entry.  Normally, the next
** entry will be the next page, but in aggregated mode (pCsr->isAgg!=0),
** the next entry is the next btree.
*/
अटल पूर्णांक stat_अग्र(sqlite3_vtab_cursor *pCursor){
  पूर्णांक rc;
  पूर्णांक nPayload;
  अक्षर *z;
  StatCursor *pCsr = (StatCursor *)pCursor;
  StatTable *pTab = (StatTable *)pCursor->pVtab;
  Btree *pBt = pTab->db->aDb[pCsr->iDb].pBt;
  Pager *pPager = sqlite3BtreePager(pBt);

  sqlite3_free(pCsr->zPath);
  pCsr->zPath = 0;

stat_अग्र_Restart:
  अगर( pCsr->iPage<0 ){
    /* Start measuring space on the next btree */
    statResetCounts(pCsr);
    rc = sqlite3_step(pCsr->pStmt);
    अगर( rc==SQLITE_ROW ){
      पूर्णांक nPage;
      u32 i_मूल = (u32)sqlite3_column_int64(pCsr->pStmt, 1);
      sqlite3PagerPagecount(pPager, &nPage);
      अगर( nPage==0 ){
        pCsr->isEof = 1;
        वापस sqlite3_reset(pCsr->pStmt);
      }
      rc = statGetPage(pBt, i_मूल, &pCsr->aPage[0]);
      pCsr->aPage[0].iPgno = i_मूल;
      pCsr->aPage[0].iCell = 0;
      अगर( !pCsr->isAgg ){
        pCsr->aPage[0].zPath = z = sqlite3_mprintf("/");
        अगर( z==0 ) rc = SQLITE_NOMEM_BKPT;
      }
      pCsr->iPage = 0;
      pCsr->nPage = 1;
    }अन्यथा{
      pCsr->isEof = 1;
      वापस sqlite3_reset(pCsr->pStmt);
    }
  }अन्यथा{
    /* Continue analyzing the btree previously started */
    StatPage *p = &pCsr->aPage[pCsr->iPage];
    अगर( !pCsr->isAgg ) statResetCounts(pCsr);
    जबतक( p->iCell<p->nCell ){
      StatCell *pCell = &p->aCell[p->iCell];
      जबतक( pCell->iOvfl<pCell->nOvfl ){
        पूर्णांक nUsable;
        पूर्णांक iOvfl = pCell->iOvfl;
        sqlite3BtreeEnter(pBt);
        nUsable = sqlite3_Btree_Get_Page_आकार(pBt) - 
                        sqlite3BtreeGetReserveNoMutex(pBt);
        sqlite3BtreeLeave(pBt);
        pCsr->nPage++;
        pCsr->iPageno = pCell->aOvfl[iOvfl];
        stat_आकार_And_Offset(pCsr);
        अगर( pCell->iOvfl<pCell->nOvfl-1 ){
          pCsr->nPayload += nUsable - 4;
        }अन्यथा{
          pCsr->nPayload += pCell->nLastOvfl;
          pCsr->nUnused += nUsable - 4 - pCell->nLastOvfl;
        }
        pCell->iOvfl++;
        अगर( !pCsr->isAgg ){
          pCsr->zName = (अक्षर *)sqlite3_column_text(pCsr->pStmt, 0);
          pCsr->zPagetype = "overflow";
          pCsr->zPath = z = sqlite3_mprintf(
              "%s%.3x+%.6x", p->zPath, p->iCell, iOvfl
          );
          वापस z==0 ? SQLITE_NOMEM_BKPT : SQLITE_OK;
        }
      }
      अगर( p->i_दायाँ_Child_Pg ) अवरोध;
      p->iCell++;
    }

    अगर( !p->i_दायाँ_Child_Pg || p->iCell>p->nCell ){
      statClearPage(p);
      pCsr->iPage--;
      अगर( pCsr->isAgg && pCsr->iPage<0 ){
        /* label-statNext-done:  When computing aggregate space usage over
        ** an entire btree, this is the exit point from this function */
        वापस SQLITE_OK;
      }
      जाओ stat_अग्र_Restart; /* Tail recursion */
    }
    pCsr->iPage++;
    अगर( pCsr->iPage>=Array_आकार(pCsr->aPage) ){
      statResetCsr(pCsr);
      वापस SQLITE_CORRUPT_BKPT;
    }
    निश्चित( p==&pCsr->aPage[pCsr->iPage-1] );

    अगर( p->iCell==p->nCell ){
      p[1].iPgno = p->i_दायाँ_Child_Pg;
    }अन्यथा{
      p[1].iPgno = p->aCell[p->iCell].iChildPg;
    }
    rc = statGetPage(pBt, p[1].iPgno, &p[1]);
    pCsr->nPage++;
    p[1].iCell = 0;
    अगर( !pCsr->isAgg ){
      p[1].zPath = z = sqlite3_mprintf("%s%.3x/", p->zPath, p->iCell);
      अगर( z==0 ) rc = SQLITE_NOMEM_BKPT;
    }
    p->iCell++;
  }


  /* Populate the StatCursor fields with the values to be returned
  ** by the xColumn() and xRowid() methods.
  */
  अगर( rc==SQLITE_OK ){
    पूर्णांक i;
    StatPage *p = &pCsr->aPage[pCsr->iPage];
    pCsr->zName = (अक्षर *)sqlite3_column_text(pCsr->pStmt, 0);
    pCsr->iPageno = p->iPgno;

    rc = statDecodePage(pBt, p);
    अगर( rc==SQLITE_OK ){
      stat_आकार_And_Offset(pCsr);

      चयन( p->flags ){
        हाल 0x05:             /* table internal */
        हाल 0x02:             /* index internal */
          pCsr->zPagetype = "internal";
          अवरोध;
        हाल 0x0D:             /* table leaf */
        हाल 0x0A:             /* index leaf */
          pCsr->zPagetype = "leaf";
          अवरोध;
        शेष:
          pCsr->zPagetype = "corrupted";
          अवरोध;
      }
      pCsr->nCell += p->nCell;
      pCsr->nUnused += p->nUnused;
      अगर( p->nMxPayload>pCsr->nMxPayload ) pCsr->nMxPayload = p->nMxPayload;
      अगर( !pCsr->isAgg ){
        pCsr->zPath = z = sqlite3_mprintf("%s", p->zPath);
        अगर( z==0 ) rc = SQLITE_NOMEM_BKPT;
      }
      nPayload = 0;
      क्रम(i=0; i<p->nCell; i++){
        nPayload += p->aCell[i].nLocal;
      }
      pCsr->nPayload += nPayload;

      /* If computing aggregate space usage by btree, continue with the
      ** next page.  The loop will exit via the return at label-statNext-done
      */
      अगर( pCsr->isAgg ) जाओ stat_अग्र_Restart;
    }
  }

  वापस rc;
}

अटल पूर्णांक statEof(sqlite3_vtab_cursor *pCursor){
  StatCursor *pCsr = (StatCursor *)pCursor;
  वापस pCsr->isEof;
}

/* Initialize a cursor according to the query plan idxNum using the
** arguments in argv[0].  See statBestIndex() for a description of the
** meaning of the bits in idxNum.
*/
अटल पूर्णांक statFilter(
  sqlite3_vtab_cursor *pCursor, 
  पूर्णांक idx_अंक, स्थिर अक्षर *idxStr,
  पूर्णांक argc, sqlite3_मूल्य **argv
){
  StatCursor *pCsr = (StatCursor *)pCursor;
  StatTable *pTab = (StatTable*)(pCursor->pVtab);
  sqlite3_str *pSql;      /* Query of btrees to analyze */
  अक्षर *zSql;             /* String value of pSql */
  पूर्णांक iArg = 0;           /* Count of argv[] parameters used so far */
  पूर्णांक rc = SQLITE_OK;     /* Result of this operation */
  स्थिर अक्षर *zName = 0;  /* Only provide analysis of this table */
  (व्योम)argc;
  (व्योम)idxStr;

  statResetCsr(pCsr);
  sqlite3_finalize(pCsr->pStmt);
  pCsr->pStmt = 0;
  अगर( idx_अंक & 0x01 ){
    /* schema=? constraint is present.  Get its value */
    स्थिर अक्षर *zDbase = (स्थिर अक्षर*)sqlite3_मूल्य_text(argv[iArg++]);
    pCsr->iDb = sqlite3FindDbName(pTab->db, zDbase);
    अगर( pCsr->iDb<0 ){
      pCsr->iDb = 0;
      pCsr->isEof = 1;
      वापस SQLITE_OK;
    }
  }अन्यथा{
    pCsr->iDb = pTab->iDb;
  }
  अगर( idx_अंक & 0x02 ){
    /* name=? constraint is present */
    zName = (स्थिर अक्षर*)sqlite3_मूल्य_text(argv[iArg++]);
  }
  अगर( idx_अंक & 0x04 ){
    /* aggregate=? constraint is present */
    pCsr->isAgg = sqlite3_मूल्य_double(argv[iArg++])!=0.0;
  }अन्यथा{
    pCsr->isAgg = 0;
  }
  pSql = sqlite3_str_new(pTab->db);
  sqlite3_str_appendf(pSql,
      "SELECT * FROM ("
        "SELECT 'sqlite_schema' AS name,1 AS rootpage,'table' AS type"
        " UNION ALL "
        "SELECT name,rootpage,type"
        " FROM \"%w\".sqlite_schema WHERE rootpage!=0)",
      pTab->db->aDb[pCsr->iDb].zDbSName);
  अगर( zName ){
    sqlite3_str_appendf(pSql, "WHERE name=%Q", zName);
  }
  अगर( idx_अंक & 0x08 ){
    sqlite3_str_appendf(pSql, " ORDER BY name");
  }
  zSql = sqlite3_str_finish(pSql);
  अगर( zSql==0 ){
    वापस SQLITE_NOMEM_BKPT;
  }अन्यथा{
    rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pStmt, 0);
    sqlite3_free(zSql);
  }

  अगर( rc==SQLITE_OK ){
    pCsr->iPage = -1;
    rc = stat_अग्र(pCursor);
  }
  वापस rc;
}

अटल पूर्णांक statColumn(
  sqlite3_vtab_cursor *pCursor, 
  sqlite3_context *ctx, 
  पूर्णांक i
){
  StatCursor *pCsr = (StatCursor *)pCursor;
  चयन( i ){
    हाल 0:            /* name */
      sqlite3_परिणाम_text(ctx, pCsr->zName, -1, SQLITE_TRANSIENT);
      अवरोध;
    हाल 1:            /* path */
      अगर( !pCsr->isAgg ){
        sqlite3_परिणाम_text(ctx, pCsr->zPath, -1, SQLITE_TRANSIENT);
      }
      अवरोध;
    हाल 2:            /* pageno */
      अगर( pCsr->isAgg ){
        sqlite3_परिणाम_int64(ctx, pCsr->nPage);
      }अन्यथा{
        sqlite3_परिणाम_int64(ctx, pCsr->iPageno);
      }
      अवरोध;
    हाल 3:            /* pagetype */
      अगर( !pCsr->isAgg ){
        sqlite3_परिणाम_text(ctx, pCsr->zPagetype, -1, SQLITE_STATIC);
      }
      अवरोध;
    हाल 4:            /* ncell */
      sqlite3_परिणाम_int64(ctx, pCsr->nCell);
      अवरोध;
    हाल 5:            /* payload */
      sqlite3_परिणाम_int64(ctx, pCsr->nPayload);
      अवरोध;
    हाल 6:            /* unused */
      sqlite3_परिणाम_int64(ctx, pCsr->nUnused);
      अवरोध;
    हाल 7:            /* mx_payload */
      sqlite3_परिणाम_int64(ctx, pCsr->nMxPayload);
      अवरोध;
    हाल 8:            /* pgoffset */
      अगर( !pCsr->isAgg ){
        sqlite3_परिणाम_int64(ctx, pCsr->iOffset);
      }
      अवरोध;
    हाल 9:            /* pgsize */
      sqlite3_परिणाम_int64(ctx, pCsr->szPage);
      अवरोध;
    हाल 10: {         /* schema */
      sqlite3 *db = sqlite3_context_db_handle(ctx);
      पूर्णांक iDb = pCsr->iDb;
      sqlite3_परिणाम_text(ctx, db->aDb[iDb].zDbSName, -1, SQLITE_STATIC);
      अवरोध;
    }
    शेष: {         /* aggregate */
      sqlite3_परिणाम_int(ctx, pCsr->isAgg);
      अवरोध;
    }
  }
  वापस SQLITE_OK;
}

अटल पूर्णांक statRowid(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid){
  StatCursor *pCsr = (StatCursor *)pCursor;
  *pRowid = pCsr->iPageno;
  वापस SQLITE_OK;
}

/*
** Invoke this routine to register the "dbstat" virtual table module
*/
पूर्णांक sqlite3DbstatRegister(sqlite3 *db){
  अटल sqlite3_module dbstat_module = {
    0,                            /* iVersion */
    statConnect,                  /* xCreate */
    statConnect,                  /* xConnect */
    stat_Best_सूचकांक,                /* xBestIndex */
    statDisconnect,               /* xDisconnect */
    statDisconnect,               /* xDestroy */
    statOpen,                     /* xOpen - open a cursor */
    statClose,                    /* xClose - close a cursor */
    statFilter,                   /* xFilter - configure scan constraints */
    stat_अग्र,                     /* xNext - advance a cursor */
    statEof,                      /* xEof - check for end of scan */
    statColumn,                   /* xColumn - read data */
    statRowid,                    /* xRowid - read data */
    0,                            /* xUpdate */
    0,                            /* xBegin */
    0,                            /* xSync */
    0,                            /* xCommit */
    0,                            /* xRollback */
    0,                            /* xFindMethod */
    0,                            /* xRename */
    0,                            /* xSavepoint */
    0,                            /* xRelease */
    0,                            /* xRollbackTo */
    0,                            /* xShadowName */
    0                             /* xIntegrity */
  };
  वापस sqlite3_create_module(db, "dbstat", &dbstat_module, 0);
}
#या_अगर defined(SQLITE_ENABLE_DBSTAT_VTAB)
पूर्णांक sqlite3DbstatRegister(sqlite3 *db){ वापस SQLITE_OK; }
#पूर्ण_अगर /* SQLITE_ENABLE_DBSTAT_VTAB */
