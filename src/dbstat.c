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

#અન્તર્ભાવ "sqliteInt.h"   /* Requires access to internal data structures */
#જો (defined(SQLITE_ENABLE_DBSTAT_VTAB) || defined(SQLITE_TEST)) \
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
સ્થિર અચલ અક્ષર zDbstatSchema[] = 
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
પ્રકારવ્યાખ્યા સંરચના StatTable StatTable;
પ્રકારવ્યાખ્યા સંરચના StatCursor StatCursor;
પ્રકારવ્યાખ્યા સંરચના StatPage StatPage;
પ્રકારવ્યાખ્યા સંરચના StatCell StatCell;

/* Size information for a single cell within a btree page */
સંરચના StatCell {
  પૂર્ણાંક nLocal;                     /* Bytes of local payload */
  u32 iChildPg;                   /* Child node (or 0 if this is a leaf) */
  પૂર્ણાંક nOvfl;                      /* Entries in aOvfl[] */
  u32 *aOvfl;                     /* Array of overflow page numbers */
  પૂર્ણાંક nLastOvfl;                  /* Bytes of payload on final overflow page */
  પૂર્ણાંક iOvfl;                      /* Iterates through aOvfl[] */
};

/* Size information for a single btree page */
સંરચના StatPage {
  u32 iPgno;                      /* Page number */
  u8 *aPg;                        /* Page buffer from sqlite3_malloc() */
  પૂર્ણાંક iCell;                      /* Current cell */
  અક્ષર *zPath;                    /* Path to this page */

  /* Variables populated by statDecodePage(): */
  u8 flags;                       /* Copy of flags byte */
  પૂર્ણાંક nCell;                      /* Number of cells on page */
  પૂર્ણાંક nUnused;                    /* Number of unused bytes on page */
  StatCell *aCell;                /* Array of parsed cells */
  u32 i_જમણું_Child_Pg;              /* Right-child page number (or 0) */
  પૂર્ણાંક nMxPayload;                 /* Largest payload of any cell on the page */
};

/* The cursor for scanning the dbstat virtual table */
સંરચના StatCursor {
  sqlite3_vtab_cursor base;       /* base class.  MUST BE FIRST! */
  sqlite3_stmt *pStmt;            /* Iterates through set of root pages */
  u8 isEof;                       /* After pStmt has returned SQLITE_DONE */
  u8 isAgg;                       /* Aggregate results for each table */
  પૂર્ણાંક iDb;                        /* Schema used for this query */

  StatPage aPage[32];             /* Pages in path to current page */
  પૂર્ણાંક iPage;                      /* Current entry in aPage[] */

  /* Values to return. */
  u32 iPageno;                    /* Value of 'pageno' column */
  અક્ષર *zName;                    /* Value of 'name' column */
  અક્ષર *zPath;                    /* Value of 'path' column */
  અક્ષર *zPagetype;                /* Value of 'pagetype' column */
  પૂર્ણાંક nPage;                      /* Number of pages in current btree */
  પૂર્ણાંક nCell;                      /* Value of 'ncell' column */
  પૂર્ણાંક nMxPayload;                 /* Value of 'mx_payload' column */
  i64 nUnused;                    /* Value of 'unused' column */
  i64 nPayload;                   /* Value of 'payload' column */
  i64 iOffset;                    /* Value of 'pgOffset' column */
  i64 szPage;                     /* Value of 'pgSize' column */
};

/* An instance of the DBSTAT virtual table */
સંરચના StatTable {
  sqlite3_vtab base;              /* base class.  MUST BE FIRST! */
  sqlite3 *db;                    /* Database connection that owns this vtab */
  પૂર્ણાંક iDb;                        /* Index of database to analyze */
};

#ifndef get2byte
# define get2byte(x)   ((x)[0]<<8 | (x)[1])
#endif

/*
** Connect to or create a new DBSTAT virtual table.
*/
સ્થિર પૂર્ણાંક statConnect(
  sqlite3 *db,
  શૂન્ય *pAux,
  પૂર્ણાંક argc, અચલ અક્ષર *અચલ*argv,
  sqlite3_vtab **ppVtab,
  અક્ષર **pzErr
){
  StatTable *pTab = 0;
  પૂર્ણાંક rc = SQLITE_OK;
  પૂર્ણાંક iDb;
  (શૂન્ય)pAux;

  જો( argc>=4 ){
    Token nm;
    sqlite3TokenInit(&nm, (અક્ષર*)argv[3]);
    iDb = sqlite3FindDb(db, &nm);
    જો( iDb<0 ){
      *pzErr = sqlite3_mprintf("no such database: %s", argv[3]);
      પાછા SQLITE_ERROR;
    }
  }નહીંતર{
    iDb = 0;
  }
  sqlite3_vtab_config(db, SQLITE_VTAB_DIRECTONLY);
  rc = sqlite3_declare_vtab(db, zDbstatSchema);
  જો( rc==SQLITE_OK ){
    pTab = (StatTable *)sqlite3_malloc64(કદ(StatTable));
    જો( pTab==0 ) rc = SQLITE_NOMEM_BKPT;
  }

  પ્રતિજ્ઞા( rc==SQLITE_OK || pTab==0 );
  જો( rc==SQLITE_OK ){
    memset(pTab, 0, કદ(StatTable));
    pTab->db = db;
    pTab->iDb = iDb;
  }

  *ppVtab = (sqlite3_vtab*)pTab;
  પાછા rc;
}

/*
** Disconnect from or destroy the DBSTAT virtual table.
*/
સ્થિર પૂર્ણાંક statDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  પાછા SQLITE_OK;
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
સ્થિર પૂર્ણાંક statBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  પૂર્ણાંક i;
  પૂર્ણાંક iSchema = -1;
  પૂર્ણાંક iName = -1;
  પૂર્ણાંક iAgg = -1;
  (શૂન્ય)tab;

  /* Look for a valid schema=? constraint.  If found, change the idxNum to
  ** 1 and request the value of that constraint be sent to xFilter.  And
  ** lower the cost estimate to encourage the constrained version to be
  ** used.
  */
  માટે(i=0; i<pIdxInfo->nConstraint; i++){
    જો( pIdxInfo->aConstraint[i].op!=SQLITE_INDEX_CONSTRAINT_EQ ) ચાલુ;
    જો( pIdxInfo->aConstraint[i].usable==0 ){
      /* Force DBSTAT table should always be the right-most table in a join */
      પાછા SQLITE_CONSTRAINT;
    }
    પસંદગી( pIdxInfo->aConstraint[i].iColumn ){
      સ્થિતિ 0: {    /* name */
        iName = i;
        તોડો;
      }
      સ્થિતિ 10: {   /* schema */
        iSchema = i;
        તોડો;
      }
      સ્થિતિ 11: {   /* aggregate */
        iAgg = i;
        તોડો;
      }
    }
  }
  i = 0;
  જો( iSchema>=0 ){
    pIdxInfo->aConstraintUsage[iSchema].argvIndex = ++i;
    pIdxInfo->aConstraintUsage[iSchema].omit = 1;
    pIdxInfo->idx_અંક |= 0x01;
  }
  જો( iName>=0 ){
    pIdxInfo->aConstraintUsage[iName].argvIndex = ++i;
    pIdxInfo->idx_અંક |= 0x02;
  }
  જો( iAgg>=0 ){
    pIdxInfo->aConstraintUsage[iAgg].argvIndex = ++i;
    pIdxInfo->idx_અંક |= 0x04;
  }
  pIdxInfo->estimatedCost = 1.0;

  /* Records are always returned in ascending order of (name, path). 
  ** If this will satisfy the client, set the orderByConsumed flag so that 
  ** SQLite does not do an external sort.
  */
  જો( ( pIdxInfo->nOrderBy==1
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
    pIdxInfo->idx_અંક |= 0x08;
  }
  pIdxInfo->idxFlags |= SQLITE_INDEX_SCAN_HEX;

  પાછા SQLITE_OK;
}

/*
** Open a new DBSTAT cursor.
*/
સ્થિર પૂર્ણાંક statOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  StatTable *pTab = (StatTable *)pVTab;
  StatCursor *pCsr;

  pCsr = (StatCursor *)sqlite3_malloc64(કદ(StatCursor));
  જો( pCsr==0 ){
    પાછા SQLITE_NOMEM_BKPT;
  }નહીંતર{
    memset(pCsr, 0, કદ(StatCursor));
    pCsr->base.pVtab = pVTab;
    pCsr->iDb = pTab->iDb;
  }

  *ppCursor = (sqlite3_vtab_cursor *)pCsr;
  પાછા SQLITE_OK;
}

સ્થિર શૂન્ય statClearCells(StatPage *p){
  પૂર્ણાંક i;
  જો( p->aCell ){
    માટે(i=0; i<p->nCell; i++){
      sqlite3_free(p->aCell[i].aOvfl);
    }
    sqlite3_free(p->aCell);
  }
  p->nCell = 0;
  p->aCell = 0;
}

સ્થિર શૂન્ય statClearPage(StatPage *p){
  u8 *aPg = p->aPg;
  statClearCells(p);
  sqlite3_free(p->zPath);
  memset(p, 0, કદ(StatPage));
  p->aPg = aPg;
}

સ્થિર શૂન્ય statResetCsr(StatCursor *pCsr){
  પૂર્ણાંક i;
  /* In some circumstances, specifically if an OOM has occurred, the call
  ** to sqlite3_reset() may cause the pager to be reset (emptied). It is
  ** important that statClearPage() is called to free any page refs before
  ** this happens. dbsqlfuzz 9ed3e4e3816219d3509d711636c38542bf3f40b1. */
  માટે(i=0; i<ArraySize(pCsr->aPage); i++){
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
સ્થિર શૂન્ય statResetCounts(StatCursor *pCsr){
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
સ્થિર પૂર્ણાંક statClose(sqlite3_vtab_cursor *pCursor){
  StatCursor *pCsr = (StatCursor *)pCursor;
  statResetCsr(pCsr);
  sqlite3_finalize(pCsr->pStmt);
  sqlite3_free(pCsr);
  પાછા SQLITE_OK;
}

/*
** For a single cell on a btree page, compute the number of bytes of
** content (payload) stored on that page.  That is to say, compute the
** number of bytes of content not found on overflow pages.
*/
સ્થિર પૂર્ણાંક getLocalPayload(
  પૂર્ણાંક nUsable,                    /* Usable bytes per page */
  u8 flags,                       /* Page flags */
  પૂર્ણાંક nTotal                      /* Total record (payload) size */
){
  પૂર્ણાંક nLocal;
  પૂર્ણાંક nMinLocal;
  પૂર્ણાંક nMaxLocal;
 
  જો( flags==0x0D ){              /* Table leaf node */
    nMinLocal = (nUsable - 12) * 32 / 255 - 23;
    nMaxLocal = nUsable - 35;
  }નહીંતર{                          /* Index interior and leaf nodes */
    nMinLocal = (nUsable - 12) * 32 / 255 - 23;
    nMaxLocal = (nUsable - 12) * 64 / 255 - 23;
  }

  nLocal = nMinLocal + (nTotal - nMinLocal) % (nUsable - 4);
  જો( nLocal>nMaxLocal ) nLocal = nMinLocal;
  પાછા nLocal;
}

/* Populate the StatPage object with information about the all
** cells found on the page currently under analysis.
*/
સ્થિર પૂર્ણાંક statDecodePage(Btree *pBt, StatPage *p){
  પૂર્ણાંક nUnused;
  પૂર્ણાંક iOff;
  પૂર્ણાંક nHdr;
  પૂર્ણાંક isLeaf;
  પૂર્ણાંક szPage;

  u8 *aData = p->aPg;
  u8 *aHdr = &aData[p->iPgno==1 ? 100 : 0];

  p->flags = aHdr[0];
  જો( p->flags==0x0A || p->flags==0x0D ){
    isLeaf = 1;
    nHdr = 8;
  }નહીંતર જો( p->flags==0x05 || p->flags==0x02 ){
    isLeaf = 0;
    nHdr = 12;
  }નહીંતર{
    જાઓ statPageIsCorrupt;
  }
  જો( p->iPgno==1 ) nHdr += 100;
  p->nCell = get2byte(&aHdr[3]);
  p->nMxPayload = 0;
  szPage = sqlite3BtreeGetPageSize(pBt);

  nUnused = get2byte(&aHdr[5]) - nHdr - 2*p->nCell;
  nUnused += (પૂર્ણાંક)aHdr[7];
  iOff = get2byte(&aHdr[1]);
  જ્યારે( iOff ){
    પૂર્ણાંક iNext;
    જો( iOff>=szPage ) જાઓ statPageIsCorrupt;
    nUnused += get2byte(&aData[iOff+2]);
    iNext = get2byte(&aData[iOff]);
    જો( iNext<iOff+4 && iNext>0 ) જાઓ statPageIsCorrupt;
    iOff = iNext;
  }
  p->nUnused = nUnused;
  p->i_જમણું_Child_Pg = isLeaf ? 0 : sqlite3Get4byte(&aHdr[8]);

  જો( p->nCell ){
    પૂર્ણાંક i;                        /* Used to iterate through cells */
    પૂર્ણાંક nUsable;                  /* Usable bytes per page */

    sqlite3BtreeEnter(pBt);
    nUsable = szPage - sqlite3BtreeGetReserveNoMutex(pBt);
    sqlite3BtreeLeave(pBt);
    p->aCell = sqlite3_malloc64((p->nCell+1) * કદ(StatCell));
    જો( p->aCell==0 ) પાછા SQLITE_NOMEM_BKPT;
    memset(p->aCell, 0, (p->nCell+1) * કદ(StatCell));

    માટે(i=0; i<p->nCell; i++){
      StatCell *pCell = &p->aCell[i];

      iOff = get2byte(&aData[nHdr+i*2]);
      જો( iOff<nHdr || iOff>=szPage ) જાઓ statPageIsCorrupt;
      જો( !isLeaf ){
        pCell->iChildPg = sqlite3Get4byte(&aData[iOff]);
        iOff += 4;
      }
      જો( p->flags==0x05 ){
        /* A table interior node. nPayload==0. */
      }નહીંતર{
        u32 nPayload;             /* Bytes of payload total (local+overflow) */
        પૂર્ણાંક nLocal;               /* Bytes of payload stored locally */
        iOff += getVarint32(&aData[iOff], nPayload);
        જો( p->flags==0x0D ){
          u64 dummy;
          iOff += sqlite3GetVarint(&aData[iOff], &dummy);
        }
        જો( nPayload>(u32)p->nMxPayload ) p->nMxPayload = nPayload;
        nLocal = getLocalPayload(nUsable, p->flags, nPayload);
        જો( nLocal<0 ) જાઓ statPageIsCorrupt;
        pCell->nLocal = nLocal;
        પ્રતિજ્ઞા( nPayload>=(u32)nLocal );
        પ્રતિજ્ઞા( nLocal<=(nUsable-35) );
        જો( nPayload>(u32)nLocal ){
          પૂર્ણાંક j;
          પૂર્ણાંક nOvfl = ((nPayload - nLocal) + nUsable-4 - 1) / (nUsable - 4);
          જો( iOff+nLocal+4>nUsable || nPayload>0x7fffffff ){
            જાઓ statPageIsCorrupt;
          }
          pCell->nLastOvfl = (nPayload-nLocal) - (nOvfl-1) * (nUsable-4);
          pCell->nOvfl = nOvfl;
          pCell->aOvfl = sqlite3_malloc64(કદ(u32)*nOvfl);
          જો( pCell->aOvfl==0 ) પાછા SQLITE_NOMEM_BKPT;
          pCell->aOvfl[0] = sqlite3Get4byte(&aData[iOff+nLocal]);
          માટે(j=1; j<nOvfl; j++){
            પૂર્ણાંક rc;
            u32 iPrev = pCell->aOvfl[j-1];
            DbPage *pPg = 0;
            rc = sqlite3PagerGet(sqlite3BtreePager(pBt), iPrev, &pPg, 0);
            જો( rc!=SQLITE_OK ){
              પ્રતિજ્ઞા( pPg==0 );
              પાછા rc;
            } 
            pCell->aOvfl[j] = sqlite3Get4byte(sqlite3PagerGetData(pPg));
            sqlite3PagerUnref(pPg);
          }
        }
      }
    }
  }

  પાછા SQLITE_OK;

statPageIsCorrupt:
  p->flags = 0;
  statClearCells(p);
  પાછા SQLITE_OK;
}

/*
** Populate the pCsr->iOffset and pCsr->szPage member variables. Based on
** the current value of pCsr->iPageno.
*/
સ્થિર શૂન્ય statSizeAndOffset(StatCursor *pCsr){
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
  જો( sqlite3OsFileControl(fd, 230440, &x)==SQLITE_OK ){
    pCsr->iOffset = x[0];
    pCsr->szPage += x[1];
  }નહીંતર{
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
સ્થિર પૂર્ણાંક statGetPage(
  Btree *pBt,                     /* Load page from this b-tree */
  u32 iPg,                        /* Page number to load */
  StatPage *pPg                   /* Load page into this object */
){
  પૂર્ણાંક pgsz = sqlite3BtreeGetPageSize(pBt);
  DbPage *pDbPage = 0;
  પૂર્ણાંક rc;

  જો( pPg->aPg==0 ){
    pPg->aPg = (u8*)sqlite3_malloc(pgsz + DBSTAT_PAGE_PADDING_BYTES);
    જો( pPg->aPg==0 ){
      પાછા SQLITE_NOMEM_BKPT;
    }
    memset(&pPg->aPg[pgsz], 0, DBSTAT_PAGE_PADDING_BYTES);
  }

  rc = sqlite3PagerGet(sqlite3BtreePager(pBt), iPg, &pDbPage, 0);
  જો( rc==SQLITE_OK ){
    અચલ u8 *a = sqlite3PagerGetData(pDbPage);
    memcpy(pPg->aPg, a, pgsz);
    sqlite3PagerUnref(pDbPage);
  }

  પાછા rc;
}

/*
** Move a DBSTAT cursor to the next entry.  Normally, the next
** entry will be the next page, but in aggregated mode (pCsr->isAgg!=0),
** the next entry is the next btree.
*/
સ્થિર પૂર્ણાંક statNext(sqlite3_vtab_cursor *pCursor){
  પૂર્ણાંક rc;
  પૂર્ણાંક nPayload;
  અક્ષર *z;
  StatCursor *pCsr = (StatCursor *)pCursor;
  StatTable *pTab = (StatTable *)pCursor->pVtab;
  Btree *pBt = pTab->db->aDb[pCsr->iDb].pBt;
  Pager *pPager = sqlite3BtreePager(pBt);

  sqlite3_free(pCsr->zPath);
  pCsr->zPath = 0;

statNextRestart:
  જો( pCsr->iPage<0 ){
    /* Start measuring space on the next btree */
    statResetCounts(pCsr);
    rc = sqlite3_step(pCsr->pStmt);
    જો( rc==SQLITE_ROW ){
      પૂર્ણાંક nPage;
      u32 i_મૂળ = (u32)sqlite3_column_int64(pCsr->pStmt, 1);
      sqlite3PagerPagecount(pPager, &nPage);
      જો( nPage==0 ){
        pCsr->isEof = 1;
        પાછા sqlite3_reset(pCsr->pStmt);
      }
      rc = statGetPage(pBt, i_મૂળ, &pCsr->aPage[0]);
      pCsr->aPage[0].iPgno = i_મૂળ;
      pCsr->aPage[0].iCell = 0;
      જો( !pCsr->isAgg ){
        pCsr->aPage[0].zPath = z = sqlite3_mprintf("/");
        જો( z==0 ) rc = SQLITE_NOMEM_BKPT;
      }
      pCsr->iPage = 0;
      pCsr->nPage = 1;
    }નહીંતર{
      pCsr->isEof = 1;
      પાછા sqlite3_reset(pCsr->pStmt);
    }
  }નહીંતર{
    /* Continue analyzing the btree previously started */
    StatPage *p = &pCsr->aPage[pCsr->iPage];
    જો( !pCsr->isAgg ) statResetCounts(pCsr);
    જ્યારે( p->iCell<p->nCell ){
      StatCell *pCell = &p->aCell[p->iCell];
      જ્યારે( pCell->iOvfl<pCell->nOvfl ){
        પૂર્ણાંક nUsable;
        પૂર્ણાંક iOvfl = pCell->iOvfl;
        sqlite3BtreeEnter(pBt);
        nUsable = sqlite3BtreeGetPageSize(pBt) - 
                        sqlite3BtreeGetReserveNoMutex(pBt);
        sqlite3BtreeLeave(pBt);
        pCsr->nPage++;
        pCsr->iPageno = pCell->aOvfl[iOvfl];
        statSizeAndOffset(pCsr);
        જો( pCell->iOvfl<pCell->nOvfl-1 ){
          pCsr->nPayload += nUsable - 4;
        }નહીંતર{
          pCsr->nPayload += pCell->nLastOvfl;
          pCsr->nUnused += nUsable - 4 - pCell->nLastOvfl;
        }
        pCell->iOvfl++;
        જો( !pCsr->isAgg ){
          pCsr->zName = (અક્ષર *)sqlite3_column_text(pCsr->pStmt, 0);
          pCsr->zPagetype = "overflow";
          pCsr->zPath = z = sqlite3_mprintf(
              "%s%.3x+%.6x", p->zPath, p->iCell, iOvfl
          );
          પાછા z==0 ? SQLITE_NOMEM_BKPT : SQLITE_OK;
        }
      }
      જો( p->i_જમણું_Child_Pg ) તોડો;
      p->iCell++;
    }

    જો( !p->i_જમણું_Child_Pg || p->iCell>p->nCell ){
      statClearPage(p);
      pCsr->iPage--;
      જો( pCsr->isAgg && pCsr->iPage<0 ){
        /* label-statNext-done:  When computing aggregate space usage over
        ** an entire btree, this is the exit point from this function */
        પાછા SQLITE_OK;
      }
      જાઓ statNextRestart; /* Tail recursion */
    }
    pCsr->iPage++;
    જો( pCsr->iPage>=ArraySize(pCsr->aPage) ){
      statResetCsr(pCsr);
      પાછા SQLITE_CORRUPT_BKPT;
    }
    પ્રતિજ્ઞા( p==&pCsr->aPage[pCsr->iPage-1] );

    જો( p->iCell==p->nCell ){
      p[1].iPgno = p->i_જમણું_Child_Pg;
    }નહીંતર{
      p[1].iPgno = p->aCell[p->iCell].iChildPg;
    }
    rc = statGetPage(pBt, p[1].iPgno, &p[1]);
    pCsr->nPage++;
    p[1].iCell = 0;
    જો( !pCsr->isAgg ){
      p[1].zPath = z = sqlite3_mprintf("%s%.3x/", p->zPath, p->iCell);
      જો( z==0 ) rc = SQLITE_NOMEM_BKPT;
    }
    p->iCell++;
  }


  /* Populate the StatCursor fields with the values to be returned
  ** by the xColumn() and xRowid() methods.
  */
  જો( rc==SQLITE_OK ){
    પૂર્ણાંક i;
    StatPage *p = &pCsr->aPage[pCsr->iPage];
    pCsr->zName = (અક્ષર *)sqlite3_column_text(pCsr->pStmt, 0);
    pCsr->iPageno = p->iPgno;

    rc = statDecodePage(pBt, p);
    જો( rc==SQLITE_OK ){
      statSizeAndOffset(pCsr);

      પસંદગી( p->flags ){
        સ્થિતિ 0x05:             /* table internal */
        સ્થિતિ 0x02:             /* index internal */
          pCsr->zPagetype = "internal";
          તોડો;
        સ્થિતિ 0x0D:             /* table leaf */
        સ્થિતિ 0x0A:             /* index leaf */
          pCsr->zPagetype = "leaf";
          તોડો;
        મૂળભૂત:
          pCsr->zPagetype = "corrupted";
          તોડો;
      }
      pCsr->nCell += p->nCell;
      pCsr->nUnused += p->nUnused;
      જો( p->nMxPayload>pCsr->nMxPayload ) pCsr->nMxPayload = p->nMxPayload;
      જો( !pCsr->isAgg ){
        pCsr->zPath = z = sqlite3_mprintf("%s", p->zPath);
        જો( z==0 ) rc = SQLITE_NOMEM_BKPT;
      }
      nPayload = 0;
      માટે(i=0; i<p->nCell; i++){
        nPayload += p->aCell[i].nLocal;
      }
      pCsr->nPayload += nPayload;

      /* If computing aggregate space usage by btree, continue with the
      ** next page.  The loop will exit via the return at label-statNext-done
      */
      જો( pCsr->isAgg ) જાઓ statNextRestart;
    }
  }

  પાછા rc;
}

સ્થિર પૂર્ણાંક statEof(sqlite3_vtab_cursor *pCursor){
  StatCursor *pCsr = (StatCursor *)pCursor;
  પાછા pCsr->isEof;
}

/* Initialize a cursor according to the query plan idxNum using the
** arguments in argv[0].  See statBestIndex() for a description of the
** meaning of the bits in idxNum.
*/
સ્થિર પૂર્ણાંક statFilter(
  sqlite3_vtab_cursor *pCursor, 
  પૂર્ણાંક idx_અંક, અચલ અક્ષર *idxStr,
  પૂર્ણાંક argc, sqlite3_કિંમત **argv
){
  StatCursor *pCsr = (StatCursor *)pCursor;
  StatTable *pTab = (StatTable*)(pCursor->pVtab);
  sqlite3_str *pSql;      /* Query of btrees to analyze */
  અક્ષર *zSql;             /* String value of pSql */
  પૂર્ણાંક iArg = 0;           /* Count of argv[] parameters used so far */
  પૂર્ણાંક rc = SQLITE_OK;     /* Result of this operation */
  અચલ અક્ષર *zName = 0;  /* Only provide analysis of this table */
  (શૂન્ય)argc;
  (શૂન્ય)idxStr;

  statResetCsr(pCsr);
  sqlite3_finalize(pCsr->pStmt);
  pCsr->pStmt = 0;
  જો( idx_અંક & 0x01 ){
    /* schema=? constraint is present.  Get its value */
    અચલ અક્ષર *zDbase = (અચલ અક્ષર*)sqlite3_કિંમત_text(argv[iArg++]);
    pCsr->iDb = sqlite3FindDbName(pTab->db, zDbase);
    જો( pCsr->iDb<0 ){
      pCsr->iDb = 0;
      pCsr->isEof = 1;
      પાછા SQLITE_OK;
    }
  }નહીંતર{
    pCsr->iDb = pTab->iDb;
  }
  જો( idx_અંક & 0x02 ){
    /* name=? constraint is present */
    zName = (અચલ અક્ષર*)sqlite3_કિંમત_text(argv[iArg++]);
  }
  જો( idx_અંક & 0x04 ){
    /* aggregate=? constraint is present */
    pCsr->isAgg = sqlite3_કિંમત_double(argv[iArg++])!=0.0;
  }નહીંતર{
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
  જો( zName ){
    sqlite3_str_appendf(pSql, "WHERE name=%Q", zName);
  }
  જો( idx_અંક & 0x08 ){
    sqlite3_str_appendf(pSql, " ORDER BY name");
  }
  zSql = sqlite3_str_finish(pSql);
  જો( zSql==0 ){
    પાછા SQLITE_NOMEM_BKPT;
  }નહીંતર{
    rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pStmt, 0);
    sqlite3_free(zSql);
  }

  જો( rc==SQLITE_OK ){
    pCsr->iPage = -1;
    rc = statNext(pCursor);
  }
  પાછા rc;
}

સ્થિર પૂર્ણાંક statColumn(
  sqlite3_vtab_cursor *pCursor, 
  sqlite3_context *ctx, 
  પૂર્ણાંક i
){
  StatCursor *pCsr = (StatCursor *)pCursor;
  પસંદગી( i ){
    સ્થિતિ 0:            /* name */
      sqlite3_result_text(ctx, pCsr->zName, -1, SQLITE_TRANSIENT);
      તોડો;
    સ્થિતિ 1:            /* path */
      જો( !pCsr->isAgg ){
        sqlite3_result_text(ctx, pCsr->zPath, -1, SQLITE_TRANSIENT);
      }
      તોડો;
    સ્થિતિ 2:            /* pageno */
      જો( pCsr->isAgg ){
        sqlite3_result_int64(ctx, pCsr->nPage);
      }નહીંતર{
        sqlite3_result_int64(ctx, pCsr->iPageno);
      }
      તોડો;
    સ્થિતિ 3:            /* pagetype */
      જો( !pCsr->isAgg ){
        sqlite3_result_text(ctx, pCsr->zPagetype, -1, SQLITE_STATIC);
      }
      તોડો;
    સ્થિતિ 4:            /* ncell */
      sqlite3_result_int64(ctx, pCsr->nCell);
      તોડો;
    સ્થિતિ 5:            /* payload */
      sqlite3_result_int64(ctx, pCsr->nPayload);
      તોડો;
    સ્થિતિ 6:            /* unused */
      sqlite3_result_int64(ctx, pCsr->nUnused);
      તોડો;
    સ્થિતિ 7:            /* mx_payload */
      sqlite3_result_int64(ctx, pCsr->nMxPayload);
      તોડો;
    સ્થિતિ 8:            /* pgoffset */
      જો( !pCsr->isAgg ){
        sqlite3_result_int64(ctx, pCsr->iOffset);
      }
      તોડો;
    સ્થિતિ 9:            /* pgsize */
      sqlite3_result_int64(ctx, pCsr->szPage);
      તોડો;
    સ્થિતિ 10: {         /* schema */
      sqlite3 *db = sqlite3_context_db_handle(ctx);
      પૂર્ણાંક iDb = pCsr->iDb;
      sqlite3_result_text(ctx, db->aDb[iDb].zDbSName, -1, SQLITE_STATIC);
      તોડો;
    }
    મૂળભૂત: {         /* aggregate */
      sqlite3_result_int(ctx, pCsr->isAgg);
      તોડો;
    }
  }
  પાછા SQLITE_OK;
}

સ્થિર પૂર્ણાંક statRowid(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid){
  StatCursor *pCsr = (StatCursor *)pCursor;
  *pRowid = pCsr->iPageno;
  પાછા SQLITE_OK;
}

/*
** Invoke this routine to register the "dbstat" virtual table module
*/
પૂર્ણાંક sqlite3DbstatRegister(sqlite3 *db){
  સ્થિર sqlite3_module dbstat_module = {
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
  પાછા sqlite3_create_module(db, "dbstat", &dbstat_module, 0);
}
#અથવાજો defined(SQLITE_ENABLE_DBSTAT_VTAB)
પૂર્ણાંક sqlite3DbstatRegister(sqlite3 *db){ પાછા SQLITE_OK; }
#endif /* SQLITE_ENABLE_DBSTAT_VTAB */
