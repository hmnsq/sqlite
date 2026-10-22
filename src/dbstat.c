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

#अन्तर्भाव "sqliteInt.h"   /* Requires access to internal data structures */
#जर (defined(SQLITE_ENABLE_DBSTAT_VTAB) || defined(SQLITE_TEST)) \
    && !defined(SQLITE_OMIT_VIRTUALTABLE)

/*
** The pager and btree modules arrange objects in memory so that there are
** always approximately 200 bytes of addressable memory following each page
** buffer. This way small buffer overreads caused by corrupt database pages
** do not cause undefined behaviour. This module pads each page buffer
** by the following number of bytes for the same purpose.
*/
#define DBSTAT_PAGE_PADDING_BYTES 256

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
स्थिर स्थिरांक अक्षर zDbstatSchema[] = 
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
प्रकारव्याख्या संरचना StatTable StatTable;
प्रकारव्याख्या संरचना StatCursor StatCursor;
प्रकारव्याख्या संरचना StatPage StatPage;
प्रकारव्याख्या संरचना StatCell StatCell;

/* Size information for a single cell within a btree page */
संरचना StatCell {
  पूर्णांक nLocal;                     /* Bytes of local payload */
  u32 iChildPg;                   /* Child node (or 0 if this is a leaf) */
  पूर्णांक nOvfl;                      /* Entries in aOvfl[] */
  u32 *aOvfl;                     /* Array of overflow page numbers */
  पूर्णांक nLastOvfl;                  /* Bytes of payload on final overflow page */
  पूर्णांक iOvfl;                      /* Iterates through aOvfl[] */
};

/* Size information for a single btree page */
संरचना StatPage {
  u32 iPgno;                      /* Page number */
  u8 *aPg;                        /* Page buffer from sqlite3_malloc() */
  पूर्णांक iCell;                      /* Current cell */
  अक्षर *zPath;                    /* Path to this page */

  /* Variables populated by statDecodePage(): */
  u8 flags;                       /* Copy of flags byte */
  पूर्णांक nCell;                      /* Number of cells on page */
  पूर्णांक nUnused;                    /* Number of unused bytes on page */
  StatCell *aCell;                /* Array of parsed cells */
  u32 i_उजवा_Child_Pg;              /* Right-child page number (or 0) */
  पूर्णांक nMxPayload;                 /* Largest payload of any cell on the page */
};

/* The cursor for scanning the dbstat virtual table */
संरचना StatCursor {
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
संरचना StatTable {
  sqlite3_vtab base;              /* base class.  MUST BE FIRST! */
  sqlite3 *db;                    /* Database connection that owns this vtab */
  पूर्णांक iDb;                        /* Index of database to analyze */
};

#ifndef get2byte
# define get2byte(x)   ((x)[0]<<8 | (x)[1])
#endif

/*
** Connect to or create a new DBSTAT virtual table.
*/
स्थिर पूर्णांक statConnect(
  sqlite3 *db,
  शून्य *pAux,
  पूर्णांक argc, स्थिरांक अक्षर *स्थिरांक*argv,
  sqlite3_vtab **ppVtab,
  अक्षर **pzErr
){
  StatTable *pTab = 0;
  पूर्णांक rc = SQLITE_OK;
  पूर्णांक iDb;
  (शून्य)pAux;

  जर( argc>=4 ){
    Token nm;
    sqlite3TokenInit(&nm, (अक्षर*)argv[3]);
    iDb = sqlite3FindDb(db, &nm);
    जर( iDb<0 ){
      *pzErr = sqlite3_mprintf("no such database: %s", argv[3]);
      परत SQLITE_ERROR;
    }
  }अन्यथा{
    iDb = 0;
  }
  sqlite3_vtab_config(db, SQLITE_VTAB_DIRECTONLY);
  rc = sqlite3_declare_vtab(db, zDbstatSchema);
  जर( rc==SQLITE_OK ){
    pTab = (StatTable *)sqlite3_malloc64(आकार(StatTable));
    जर( pTab==0 ) rc = SQLITE_NOMEM_BKPT;
  }

  प्रतिज्ञा( rc==SQLITE_OK || pTab==0 );
  जर( rc==SQLITE_OK ){
    स_रखो(pTab, 0, आकार(StatTable));
    pTab->db = db;
    pTab->iDb = iDb;
  }

  *ppVtab = (sqlite3_vtab*)pTab;
  परत rc;
}

/*
** Disconnect from or destroy the DBSTAT virtual table.
*/
स्थिर पूर्णांक statDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  परत SQLITE_OK;
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
स्थिर पूर्णांक statBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  पूर्णांक i;
  पूर्णांक iSchema = -1;
  पूर्णांक iName = -1;
  पूर्णांक iAgg = -1;
  (शून्य)tab;

  /* Look for a valid schema=? constraint.  If found, change the idxNum to
  ** 1 and request the value of that constraint be sent to xFilter.  And
  ** lower the cost estimate to encourage the constrained version to be
  ** used.
  */
  साठी(i=0; i<pIdxInfo->nConstraint; i++){
    जर( pIdxInfo->aConstraint[i].op!=SQLITE_INDEX_CONSTRAINT_EQ ) सुरूठेवा;
    जर( pIdxInfo->aConstraint[i].usable==0 ){
      /* Force DBSTAT table should always be the right-most table in a join */
      परत SQLITE_CONSTRAINT;
    }
    निवड( pIdxInfo->aConstraint[i].iColumn ){
      स्थिती 0: {    /* name */
        iName = i;
        थांबा;
      }
      स्थिती 10: {   /* schema */
        iSchema = i;
        थांबा;
      }
      स्थिती 11: {   /* aggregate */
        iAgg = i;
        थांबा;
      }
    }
  }
  i = 0;
  जर( iSchema>=0 ){
    pIdxInfo->aConstraintUsage[iSchema].argvIndex = ++i;
    pIdxInfo->aConstraintUsage[iSchema].omit = 1;
    pIdxInfo->idx_अंक |= 0x01;
  }
  जर( iName>=0 ){
    pIdxInfo->aConstraintUsage[iName].argvIndex = ++i;
    pIdxInfo->idx_अंक |= 0x02;
  }
  जर( iAgg>=0 ){
    pIdxInfo->aConstraintUsage[iAgg].argvIndex = ++i;
    pIdxInfo->idx_अंक |= 0x04;
  }
  pIdxInfo->estimatedCost = 1.0;

  /* Records are always returned in ascending order of (name, path). 
  ** If this will satisfy the client, set the orderByConsumed flag so that 
  ** SQLite does not do an external sort.
  */
  जर( ( pIdxInfo->nOrderBy==1
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
  pIdxInfo->idxFlags |= SQLITE_INDEX_SCAN_HEX;

  परत SQLITE_OK;
}

/*
** Open a new DBSTAT cursor.
*/
स्थिर पूर्णांक statOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  StatTable *pTab = (StatTable *)pVTab;
  StatCursor *pCsr;

  pCsr = (StatCursor *)sqlite3_malloc64(आकार(StatCursor));
  जर( pCsr==0 ){
    परत SQLITE_NOMEM_BKPT;
  }अन्यथा{
    स_रखो(pCsr, 0, आकार(StatCursor));
    pCsr->base.pVtab = pVTab;
    pCsr->iDb = pTab->iDb;
  }

  *ppCursor = (sqlite3_vtab_cursor *)pCsr;
  परत SQLITE_OK;
}

स्थिर शून्य statClearCells(StatPage *p){
  पूर्णांक i;
  जर( p->aCell ){
    साठी(i=0; i<p->nCell; i++){
      sqlite3_free(p->aCell[i].aOvfl);
    }
    sqlite3_free(p->aCell);
  }
  p->nCell = 0;
  p->aCell = 0;
}

स्थिर शून्य statClearPage(StatPage *p){
  u8 *aPg = p->aPg;
  statClearCells(p);
  sqlite3_free(p->zPath);
  स_रखो(p, 0, आकार(StatPage));
  p->aPg = aPg;
}

स्थिर शून्य statResetCsr(StatCursor *pCsr){
  पूर्णांक i;
  /* In some circumstances, specifically if an OOM has occurred, the call
  ** to sqlite3_reset() may cause the pager to be reset (emptied). It is
  ** important that statClearPage() is called to free any page refs before
  ** this happens. dbsqlfuzz 9ed3e4e3816219d3509d711636c38542bf3f40b1. */
  साठी(i=0; i<ArraySize(pCsr->aPage); i++){
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
स्थिर शून्य statResetCounts(StatCursor *pCsr){
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
स्थिर पूर्णांक statClose(sqlite3_vtab_cursor *pCursor){
  StatCursor *pCsr = (StatCursor *)pCursor;
  statResetCsr(pCsr);
  sqlite3_finalize(pCsr->pStmt);
  sqlite3_free(pCsr);
  परत SQLITE_OK;
}

/*
** For a single cell on a btree page, compute the number of bytes of
** content (payload) stored on that page.  That is to say, compute the
** number of bytes of content not found on overflow pages.
*/
स्थिर पूर्णांक getLocalPayload(
  पूर्णांक nUsable,                    /* Usable bytes per page */
  u8 flags,                       /* Page flags */
  पूर्णांक nTotal                      /* Total record (payload) size */
){
  पूर्णांक nLocal;
  पूर्णांक nMinLocal;
  पूर्णांक nMaxLocal;
 
  जर( flags==0x0D ){              /* Table leaf node */
    nMinLocal = (nUsable - 12) * 32 / 255 - 23;
    nMaxLocal = nUsable - 35;
  }अन्यथा{                          /* Index interior and leaf nodes */
    nMinLocal = (nUsable - 12) * 32 / 255 - 23;
    nMaxLocal = (nUsable - 12) * 64 / 255 - 23;
  }

  nLocal = nMinLocal + (nTotal - nMinLocal) % (nUsable - 4);
  जर( nLocal>nMaxLocal ) nLocal = nMinLocal;
  परत nLocal;
}

/* Populate the StatPage object with information about the all
** cells found on the page currently under analysis.
*/
स्थिर पूर्णांक statDecodePage(Btree *pBt, StatPage *p){
  पूर्णांक nUnused;
  पूर्णांक iOff;
  पूर्णांक nHdr;
  पूर्णांक isLeaf;
  पूर्णांक szPage;

  u8 *aData = p->aPg;
  u8 *aHdr = &aData[p->iPgno==1 ? 100 : 0];

  p->flags = aHdr[0];
  जर( p->flags==0x0A || p->flags==0x0D ){
    isLeaf = 1;
    nHdr = 8;
  }अन्यथा जर( p->flags==0x05 || p->flags==0x02 ){
    isLeaf = 0;
    nHdr = 12;
  }अन्यथा{
    जा statPageIsCorrupt;
  }
  जर( p->iPgno==1 ) nHdr += 100;
  p->nCell = get2byte(&aHdr[3]);
  p->nMxPayload = 0;
  szPage = sqlite3BtreeGetPageSize(pBt);

  nUnused = get2byte(&aHdr[5]) - nHdr - 2*p->nCell;
  nUnused += (पूर्णांक)aHdr[7];
  iOff = get2byte(&aHdr[1]);
  पर्यंत( iOff ){
    पूर्णांक iNext;
    जर( iOff>=szPage ) जा statPageIsCorrupt;
    nUnused += get2byte(&aData[iOff+2]);
    iNext = get2byte(&aData[iOff]);
    जर( iNext<iOff+4 && iNext>0 ) जा statPageIsCorrupt;
    iOff = iNext;
  }
  p->nUnused = nUnused;
  p->i_उजवा_Child_Pg = isLeaf ? 0 : sqlite3Get4byte(&aHdr[8]);

  जर( p->nCell ){
    पूर्णांक i;                        /* Used to iterate through cells */
    पूर्णांक nUsable;                  /* Usable bytes per page */

    sqlite3BtreeEnter(pBt);
    nUsable = szPage - sqlite3BtreeGetReserveNoMutex(pBt);
    sqlite3BtreeLeave(pBt);
    p->aCell = sqlite3_malloc64((p->nCell+1) * आकार(StatCell));
    जर( p->aCell==0 ) परत SQLITE_NOMEM_BKPT;
    स_रखो(p->aCell, 0, (p->nCell+1) * आकार(StatCell));

    साठी(i=0; i<p->nCell; i++){
      StatCell *pCell = &p->aCell[i];

      iOff = get2byte(&aData[nHdr+i*2]);
      जर( iOff<nHdr || iOff>=szPage ) जा statPageIsCorrupt;
      जर( !isLeaf ){
        pCell->iChildPg = sqlite3Get4byte(&aData[iOff]);
        iOff += 4;
      }
      जर( p->flags==0x05 ){
        /* A table interior node. nPayload==0. */
      }अन्यथा{
        u32 nPayload;             /* Bytes of payload total (local+overflow) */
        पूर्णांक nLocal;               /* Bytes of payload stored locally */
        iOff += getVarint32(&aData[iOff], nPayload);
        जर( p->flags==0x0D ){
          u64 dummy;
          iOff += sqlite3GetVarint(&aData[iOff], &dummy);
        }
        जर( nPayload>(u32)p->nMxPayload ) p->nMxPayload = nPayload;
        nLocal = getLocalPayload(nUsable, p->flags, nPayload);
        जर( nLocal<0 ) जा statPageIsCorrupt;
        pCell->nLocal = nLocal;
        प्रतिज्ञा( nPayload>=(u32)nLocal );
        प्रतिज्ञा( nLocal<=(nUsable-35) );
        जर( nPayload>(u32)nLocal ){
          पूर्णांक j;
          पूर्णांक nOvfl = ((nPayload - nLocal) + nUsable-4 - 1) / (nUsable - 4);
          जर( iOff+nLocal+4>nUsable || nPayload>0x7fffffff ){
            जा statPageIsCorrupt;
          }
          pCell->nLastOvfl = (nPayload-nLocal) - (nOvfl-1) * (nUsable-4);
          pCell->nOvfl = nOvfl;
          pCell->aOvfl = sqlite3_malloc64(आकार(u32)*nOvfl);
          जर( pCell->aOvfl==0 ) परत SQLITE_NOMEM_BKPT;
          pCell->aOvfl[0] = sqlite3Get4byte(&aData[iOff+nLocal]);
          साठी(j=1; j<nOvfl; j++){
            पूर्णांक rc;
            u32 iPrev = pCell->aOvfl[j-1];
            DbPage *pPg = 0;
            rc = sqlite3PagerGet(sqlite3BtreePager(pBt), iPrev, &pPg, 0);
            जर( rc!=SQLITE_OK ){
              प्रतिज्ञा( pPg==0 );
              परत rc;
            } 
            pCell->aOvfl[j] = sqlite3Get4byte(sqlite3PagerGetData(pPg));
            sqlite3PagerUnref(pPg);
          }
        }
      }
    }
  }

  परत SQLITE_OK;

statPageIsCorrupt:
  p->flags = 0;
  statClearCells(p);
  परत SQLITE_OK;
}

/*
** Populate the pCsr->iOffset and pCsr->szPage member variables. Based on
** the current value of pCsr->iPageno.
*/
स्थिर शून्य statSizeAndOffset(StatCursor *pCsr){
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
  जर( sqlite3OsFileControl(fd, 230440, &x)==SQLITE_OK ){
    pCsr->iOffset = x[0];
    pCsr->szPage += x[1];
  }अन्यथा{
    /* Not ZIPVFS: The default page size and offset */
    pCsr->szPage += sqlite3BtreeGetPageSize(pBt);
    pCsr->iOffset = (i64)pCsr->szPage * (pCsr->iPageno - 1);
  }
}

/*
** Load a copy of the page data for page iPg into the buffer belonging
** to page object pPg. Allocate the buffer if necessary. Return SQLITE_OK
** if successful, or an SQLite error code otherwise.
*/
स्थिर पूर्णांक statGetPage(
  Btree *pBt,                     /* Load page from this b-tree */
  u32 iPg,                        /* Page number to load */
  StatPage *pPg                   /* Load page into this object */
){
  पूर्णांक pgsz = sqlite3BtreeGetPageSize(pBt);
  DbPage *pDbPage = 0;
  पूर्णांक rc;

  जर( pPg->aPg==0 ){
    pPg->aPg = (u8*)sqlite3_malloc(pgsz + DBSTAT_PAGE_PADDING_BYTES);
    जर( pPg->aPg==0 ){
      परत SQLITE_NOMEM_BKPT;
    }
    स_रखो(&pPg->aPg[pgsz], 0, DBSTAT_PAGE_PADDING_BYTES);
  }

  rc = sqlite3PagerGet(sqlite3BtreePager(pBt), iPg, &pDbPage, 0);
  जर( rc==SQLITE_OK ){
    स्थिरांक u8 *a = sqlite3PagerGetData(pDbPage);
    स_नकल(pPg->aPg, a, pgsz);
    sqlite3PagerUnref(pDbPage);
  }

  परत rc;
}

/*
** Move a DBSTAT cursor to the next entry.  Normally, the next
** entry will be the next page, but in aggregated mode (pCsr->isAgg!=0),
** the next entry is the next btree.
*/
स्थिर पूर्णांक statNext(sqlite3_vtab_cursor *pCursor){
  पूर्णांक rc;
  पूर्णांक nPayload;
  अक्षर *z;
  StatCursor *pCsr = (StatCursor *)pCursor;
  StatTable *pTab = (StatTable *)pCursor->pVtab;
  Btree *pBt = pTab->db->aDb[pCsr->iDb].pBt;
  Pager *pPager = sqlite3BtreePager(pBt);

  sqlite3_free(pCsr->zPath);
  pCsr->zPath = 0;

statNextRestart:
  जर( pCsr->iPage<0 ){
    /* Start measuring space on the next btree */
    statResetCounts(pCsr);
    rc = sqlite3_step(pCsr->pStmt);
    जर( rc==SQLITE_ROW ){
      पूर्णांक nPage;
      u32 i_मूळ = (u32)sqlite3_column_int64(pCsr->pStmt, 1);
      sqlite3PagerPagecount(pPager, &nPage);
      जर( nPage==0 ){
        pCsr->isEof = 1;
        परत sqlite3_reset(pCsr->pStmt);
      }
      rc = statGetPage(pBt, i_मूळ, &pCsr->aPage[0]);
      pCsr->aPage[0].iPgno = i_मूळ;
      pCsr->aPage[0].iCell = 0;
      जर( !pCsr->isAgg ){
        pCsr->aPage[0].zPath = z = sqlite3_mprintf("/");
        जर( z==0 ) rc = SQLITE_NOMEM_BKPT;
      }
      pCsr->iPage = 0;
      pCsr->nPage = 1;
    }अन्यथा{
      pCsr->isEof = 1;
      परत sqlite3_reset(pCsr->pStmt);
    }
  }अन्यथा{
    /* Continue analyzing the btree previously started */
    StatPage *p = &pCsr->aPage[pCsr->iPage];
    जर( !pCsr->isAgg ) statResetCounts(pCsr);
    पर्यंत( p->iCell<p->nCell ){
      StatCell *pCell = &p->aCell[p->iCell];
      पर्यंत( pCell->iOvfl<pCell->nOvfl ){
        पूर्णांक nUsable;
        पूर्णांक iOvfl = pCell->iOvfl;
        sqlite3BtreeEnter(pBt);
        nUsable = sqlite3BtreeGetPageSize(pBt) - 
                        sqlite3BtreeGetReserveNoMutex(pBt);
        sqlite3BtreeLeave(pBt);
        pCsr->nPage++;
        pCsr->iPageno = pCell->aOvfl[iOvfl];
        statSizeAndOffset(pCsr);
        जर( pCell->iOvfl<pCell->nOvfl-1 ){
          pCsr->nPayload += nUsable - 4;
        }अन्यथा{
          pCsr->nPayload += pCell->nLastOvfl;
          pCsr->nUnused += nUsable - 4 - pCell->nLastOvfl;
        }
        pCell->iOvfl++;
        जर( !pCsr->isAgg ){
          pCsr->zName = (अक्षर *)sqlite3_column_text(pCsr->pStmt, 0);
          pCsr->zPagetype = "overflow";
          pCsr->zPath = z = sqlite3_mprintf(
              "%s%.3x+%.6x", p->zPath, p->iCell, iOvfl
          );
          परत z==0 ? SQLITE_NOMEM_BKPT : SQLITE_OK;
        }
      }
      जर( p->i_उजवा_Child_Pg ) थांबा;
      p->iCell++;
    }

    जर( !p->i_उजवा_Child_Pg || p->iCell>p->nCell ){
      statClearPage(p);
      pCsr->iPage--;
      जर( pCsr->isAgg && pCsr->iPage<0 ){
        /* label-statNext-done:  When computing aggregate space usage over
        ** an entire btree, this is the exit point from this function */
        परत SQLITE_OK;
      }
      जा statNextRestart; /* Tail recursion */
    }
    pCsr->iPage++;
    जर( pCsr->iPage>=ArraySize(pCsr->aPage) ){
      statResetCsr(pCsr);
      परत SQLITE_CORRUPT_BKPT;
    }
    प्रतिज्ञा( p==&pCsr->aPage[pCsr->iPage-1] );

    जर( p->iCell==p->nCell ){
      p[1].iPgno = p->i_उजवा_Child_Pg;
    }अन्यथा{
      p[1].iPgno = p->aCell[p->iCell].iChildPg;
    }
    rc = statGetPage(pBt, p[1].iPgno, &p[1]);
    pCsr->nPage++;
    p[1].iCell = 0;
    जर( !pCsr->isAgg ){
      p[1].zPath = z = sqlite3_mprintf("%s%.3x/", p->zPath, p->iCell);
      जर( z==0 ) rc = SQLITE_NOMEM_BKPT;
    }
    p->iCell++;
  }


  /* Populate the StatCursor fields with the values to be returned
  ** by the xColumn() and xRowid() methods.
  */
  जर( rc==SQLITE_OK ){
    पूर्णांक i;
    StatPage *p = &pCsr->aPage[pCsr->iPage];
    pCsr->zName = (अक्षर *)sqlite3_column_text(pCsr->pStmt, 0);
    pCsr->iPageno = p->iPgno;

    rc = statDecodePage(pBt, p);
    जर( rc==SQLITE_OK ){
      statSizeAndOffset(pCsr);

      निवड( p->flags ){
        स्थिती 0x05:             /* table internal */
        स्थिती 0x02:             /* index internal */
          pCsr->zPagetype = "internal";
          थांबा;
        स्थिती 0x0D:             /* table leaf */
        स्थिती 0x0A:             /* index leaf */
          pCsr->zPagetype = "leaf";
          थांबा;
        पूर्वनिर्धारित:
          pCsr->zPagetype = "corrupted";
          थांबा;
      }
      pCsr->nCell += p->nCell;
      pCsr->nUnused += p->nUnused;
      जर( p->nMxPayload>pCsr->nMxPayload ) pCsr->nMxPayload = p->nMxPayload;
      जर( !pCsr->isAgg ){
        pCsr->zPath = z = sqlite3_mprintf("%s", p->zPath);
        जर( z==0 ) rc = SQLITE_NOMEM_BKPT;
      }
      nPayload = 0;
      साठी(i=0; i<p->nCell; i++){
        nPayload += p->aCell[i].nLocal;
      }
      pCsr->nPayload += nPayload;

      /* If computing aggregate space usage by btree, continue with the
      ** next page.  The loop will exit via the return at label-statNext-done
      */
      जर( pCsr->isAgg ) जा statNextRestart;
    }
  }

  परत rc;
}

स्थिर पूर्णांक statEof(sqlite3_vtab_cursor *pCursor){
  StatCursor *pCsr = (StatCursor *)pCursor;
  परत pCsr->isEof;
}

/* Initialize a cursor according to the query plan idxNum using the
** arguments in argv[0].  See statBestIndex() for a description of the
** meaning of the bits in idxNum.
*/
स्थिर पूर्णांक statFilter(
  sqlite3_vtab_cursor *pCursor, 
  पूर्णांक idx_अंक, स्थिरांक अक्षर *idxStr,
  पूर्णांक argc, sqlite3_मूल्य **argv
){
  StatCursor *pCsr = (StatCursor *)pCursor;
  StatTable *pTab = (StatTable*)(pCursor->pVtab);
  sqlite3_str *pSql;      /* Query of btrees to analyze */
  अक्षर *zSql;             /* String value of pSql */
  पूर्णांक iArg = 0;           /* Count of argv[] parameters used so far */
  पूर्णांक rc = SQLITE_OK;     /* Result of this operation */
  स्थिरांक अक्षर *zName = 0;  /* Only provide analysis of this table */
  (शून्य)argc;
  (शून्य)idxStr;

  statResetCsr(pCsr);
  sqlite3_finalize(pCsr->pStmt);
  pCsr->pStmt = 0;
  जर( idx_अंक & 0x01 ){
    /* schema=? constraint is present.  Get its value */
    स्थिरांक अक्षर *zDbase = (स्थिरांक अक्षर*)sqlite3_मूल्य_text(argv[iArg++]);
    pCsr->iDb = sqlite3FindDbName(pTab->db, zDbase);
    जर( pCsr->iDb<0 ){
      pCsr->iDb = 0;
      pCsr->isEof = 1;
      परत SQLITE_OK;
    }
  }अन्यथा{
    pCsr->iDb = pTab->iDb;
  }
  जर( idx_अंक & 0x02 ){
    /* name=? constraint is present */
    zName = (स्थिरांक अक्षर*)sqlite3_मूल्य_text(argv[iArg++]);
  }
  जर( idx_अंक & 0x04 ){
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
  जर( zName ){
    sqlite3_str_appendf(pSql, "WHERE name=%Q", zName);
  }
  जर( idx_अंक & 0x08 ){
    sqlite3_str_appendf(pSql, " ORDER BY name");
  }
  zSql = sqlite3_str_finish(pSql);
  जर( zSql==0 ){
    परत SQLITE_NOMEM_BKPT;
  }अन्यथा{
    rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pStmt, 0);
    sqlite3_free(zSql);
  }

  जर( rc==SQLITE_OK ){
    pCsr->iPage = -1;
    rc = statNext(pCursor);
  }
  परत rc;
}

स्थिर पूर्णांक statColumn(
  sqlite3_vtab_cursor *pCursor, 
  sqlite3_context *ctx, 
  पूर्णांक i
){
  StatCursor *pCsr = (StatCursor *)pCursor;
  निवड( i ){
    स्थिती 0:            /* name */
      sqlite3_result_text(ctx, pCsr->zName, -1, SQLITE_TRANSIENT);
      थांबा;
    स्थिती 1:            /* path */
      जर( !pCsr->isAgg ){
        sqlite3_result_text(ctx, pCsr->zPath, -1, SQLITE_TRANSIENT);
      }
      थांबा;
    स्थिती 2:            /* pageno */
      जर( pCsr->isAgg ){
        sqlite3_result_int64(ctx, pCsr->nPage);
      }अन्यथा{
        sqlite3_result_int64(ctx, pCsr->iPageno);
      }
      थांबा;
    स्थिती 3:            /* pagetype */
      जर( !pCsr->isAgg ){
        sqlite3_result_text(ctx, pCsr->zPagetype, -1, SQLITE_STATIC);
      }
      थांबा;
    स्थिती 4:            /* ncell */
      sqlite3_result_int64(ctx, pCsr->nCell);
      थांबा;
    स्थिती 5:            /* payload */
      sqlite3_result_int64(ctx, pCsr->nPayload);
      थांबा;
    स्थिती 6:            /* unused */
      sqlite3_result_int64(ctx, pCsr->nUnused);
      थांबा;
    स्थिती 7:            /* mx_payload */
      sqlite3_result_int64(ctx, pCsr->nMxPayload);
      थांबा;
    स्थिती 8:            /* pgoffset */
      जर( !pCsr->isAgg ){
        sqlite3_result_int64(ctx, pCsr->iOffset);
      }
      थांबा;
    स्थिती 9:            /* pgsize */
      sqlite3_result_int64(ctx, pCsr->szPage);
      थांबा;
    स्थिती 10: {         /* schema */
      sqlite3 *db = sqlite3_context_db_handle(ctx);
      पूर्णांक iDb = pCsr->iDb;
      sqlite3_result_text(ctx, db->aDb[iDb].zDbSName, -1, SQLITE_STATIC);
      थांबा;
    }
    पूर्वनिर्धारित: {         /* aggregate */
      sqlite3_result_int(ctx, pCsr->isAgg);
      थांबा;
    }
  }
  परत SQLITE_OK;
}

स्थिर पूर्णांक statRowid(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid){
  StatCursor *pCsr = (StatCursor *)pCursor;
  *pRowid = pCsr->iPageno;
  परत SQLITE_OK;
}

/*
** Invoke this routine to register the "dbstat" virtual table module
*/
पूर्णांक sqlite3DbstatRegister(sqlite3 *db){
  स्थिर sqlite3_module dbstat_module = {
    0,                            /* iVersion */
    statConnect,                  /* xCreate */
    statConnect,                  /* xConnect */
    statBestIndex,                /* xBestIndex */
    statDisconnect,               /* xDisconnect */
    statDisconnect,               /* xDestroy */
    statOpen,                     /* xOpen - open a cursor */
    statClose,                    /* xClose - close a cursor */
    statFilter,                   /* xFilter - configure scan constraints */
    statNext,                     /* xNext - advance a cursor */
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
  परत sqlite3_create_module(db, "dbstat", &dbstat_module, 0);
}
#किंवाजर defined(SQLITE_ENABLE_DBSTAT_VTAB)
पूर्णांक sqlite3DbstatRegister(sqlite3 *db){ परत SQLITE_OK; }
#endif /* SQLITE_ENABLE_DBSTAT_VTAB */
