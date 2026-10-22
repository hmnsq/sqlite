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

#شامل "sqliteInt.h"   /* Requires access to internal data structures */
#اگر (defined(SQLITE_ENABLE_DBSTAT_VTAB) || defined(SQLITE_TEST)) \
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
ایستا ثابت نویسه zDbstatSchema[] = 
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
تعریفنوع ساختار StatTable StatTable;
تعریفنوع ساختار StatCursor StatCursor;
تعریفنوع ساختار StatPage StatPage;
تعریفنوع ساختار StatCell StatCell;

/* Size information for a single cell within a btree page */
ساختار StatCell {
  صحیح nLocal;                     /* Bytes of local payload */
  u32 iChildPg;                   /* Child node (or 0 if this is a leaf) */
  صحیح nOvfl;                      /* Entries in aOvfl[] */
  u32 *aOvfl;                     /* Array of overflow page numbers */
  صحیح nLastOvfl;                  /* Bytes of payload on final overflow page */
  صحیح iOvfl;                      /* Iterates through aOvfl[] */
};

/* Size information for a single btree page */
ساختار StatPage {
  u32 iPgno;                      /* Page number */
  u8 *aPg;                        /* Page buffer from sqlite3_malloc() */
  صحیح iCell;                      /* Current cell */
  نویسه *zPath;                    /* Path to this page */

  /* Variables populated by statDecodePage(): */
  u8 flags;                       /* Copy of flags byte */
  صحیح nCell;                      /* Number of cells on page */
  صحیح nUnused;                    /* Number of unused bytes on page */
  StatCell *aCell;                /* Array of parsed cells */
  u32 i_راست_Child_Pg;              /* Right-child page number (or 0) */
  صحیح nMxPayload;                 /* Largest payload of any cell on the page */
};

/* The cursor for scanning the dbstat virtual table */
ساختار StatCursor {
  sqlite3_vtab_cursor base;       /* base class.  MUST BE FIRST! */
  sqlite3_stmt *pStmt;            /* Iterates through set of root pages */
  u8 isEof;                       /* After pStmt has returned SQLITE_DONE */
  u8 isAgg;                       /* Aggregate results for each table */
  صحیح iDb;                        /* Schema used for this query */

  StatPage aPage[32];             /* Pages in path to current page */
  صحیح iPage;                      /* Current entry in aPage[] */

  /* Values to return. */
  u32 iPageno;                    /* Value of 'pageno' column */
  نویسه *zName;                    /* Value of 'name' column */
  نویسه *zPath;                    /* Value of 'path' column */
  نویسه *zPagetype;                /* Value of 'pagetype' column */
  صحیح nPage;                      /* Number of pages in current btree */
  صحیح nCell;                      /* Value of 'ncell' column */
  صحیح nMxPayload;                 /* Value of 'mx_payload' column */
  i64 nUnused;                    /* Value of 'unused' column */
  i64 nPayload;                   /* Value of 'payload' column */
  i64 iOffset;                    /* Value of 'pgOffset' column */
  i64 szPage;                     /* Value of 'pgSize' column */
};

/* An instance of the DBSTAT virtual table */
ساختار StatTable {
  sqlite3_vtab base;              /* base class.  MUST BE FIRST! */
  sqlite3 *db;                    /* Database connection that owns this vtab */
  صحیح iDb;                        /* Index of database to analyze */
};

#ifndef get2byte
# define get2byte(x)   ((x)[0]<<8 | (x)[1])
#endif

/*
** Connect to or create a new DBSTAT virtual table.
*/
ایستا صحیح statConnect(
  sqlite3 *db,
  تهی *pAux,
  صحیح argc, ثابت نویسه *ثابت*argv,
  sqlite3_vtab **ppVtab,
  نویسه **pzErr
){
  StatTable *pTab = 0;
  صحیح rc = SQLITE_OK;
  صحیح iDb;
  (تهی)pAux;

  اگر( argc>=4 ){
    Token nm;
    sqlite3TokenInit(&nm, (نویسه*)argv[3]);
    iDb = sqlite3FindDb(db, &nm);
    اگر( iDb<0 ){
      *pzErr = sqlite3_mprintf("no such database: %s", argv[3]);
      بازگشت SQLITE_ERROR;
    }
  }وگرنه{
    iDb = 0;
  }
  sqlite3_vtab_config(db, SQLITE_VTAB_DIRECTONLY);
  rc = sqlite3_declare_vtab(db, zDbstatSchema);
  اگر( rc==SQLITE_OK ){
    pTab = (StatTable *)sqlite3_malloc64(اندازه(StatTable));
    اگر( pTab==0 ) rc = SQLITE_NOMEM_BKPT;
  }

  أكد( rc==SQLITE_OK || pTab==0 );
  اگر( rc==SQLITE_OK ){
    memset(pTab, 0, اندازه(StatTable));
    pTab->db = db;
    pTab->iDb = iDb;
  }

  *ppVtab = (sqlite3_vtab*)pTab;
  بازگشت rc;
}

/*
** Disconnect from or destroy the DBSTAT virtual table.
*/
ایستا صحیح statDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  بازگشت SQLITE_OK;
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
ایستا صحیح statBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  صحیح i;
  صحیح iSchema = -1;
  صحیح iName = -1;
  صحیح iAgg = -1;
  (تهی)tab;

  /* Look for a valid schema=? constraint.  If found, change the idxNum to
  ** 1 and request the value of that constraint be sent to xFilter.  And
  ** lower the cost estimate to encourage the constrained version to be
  ** used.
  */
  برای(i=0; i<pIdxInfo->nConstraint; i++){
    اگر( pIdxInfo->aConstraint[i].op!=SQLITE_INDEX_CONSTRAINT_EQ ) ادامه;
    اگر( pIdxInfo->aConstraint[i].usable==0 ){
      /* Force DBSTAT table should always be the right-most table in a join */
      بازگشت SQLITE_CONSTRAINT;
    }
    انتخاب( pIdxInfo->aConstraint[i].iColumn ){
      حالت 0: {    /* name */
        iName = i;
        بشکن;
      }
      حالت 10: {   /* schema */
        iSchema = i;
        بشکن;
      }
      حالت 11: {   /* aggregate */
        iAgg = i;
        بشکن;
      }
    }
  }
  i = 0;
  اگر( iSchema>=0 ){
    pIdxInfo->aConstraintUsage[iSchema].argvIndex = ++i;
    pIdxInfo->aConstraintUsage[iSchema].omit = 1;
    pIdxInfo->idx_رقم |= 0x01;
  }
  اگر( iName>=0 ){
    pIdxInfo->aConstraintUsage[iName].argvIndex = ++i;
    pIdxInfo->idx_رقم |= 0x02;
  }
  اگر( iAgg>=0 ){
    pIdxInfo->aConstraintUsage[iAgg].argvIndex = ++i;
    pIdxInfo->idx_رقم |= 0x04;
  }
  pIdxInfo->estimatedCost = 1.0;

  /* Records are always returned in ascending order of (name, path). 
  ** If this will satisfy the client, set the orderByConsumed flag so that 
  ** SQLite does not do an external sort.
  */
  اگر( ( pIdxInfo->nOrderBy==1
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
    pIdxInfo->idx_رقم |= 0x08;
  }
  pIdxInfo->idxFlags |= SQLITE_INDEX_SCAN_HEX;

  بازگشت SQLITE_OK;
}

/*
** Open a new DBSTAT cursor.
*/
ایستا صحیح statOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  StatTable *pTab = (StatTable *)pVTab;
  StatCursor *pCsr;

  pCsr = (StatCursor *)sqlite3_malloc64(اندازه(StatCursor));
  اگر( pCsr==0 ){
    بازگشت SQLITE_NOMEM_BKPT;
  }وگرنه{
    memset(pCsr, 0, اندازه(StatCursor));
    pCsr->base.pVtab = pVTab;
    pCsr->iDb = pTab->iDb;
  }

  *ppCursor = (sqlite3_vtab_cursor *)pCsr;
  بازگشت SQLITE_OK;
}

ایستا تهی statClearCells(StatPage *p){
  صحیح i;
  اگر( p->aCell ){
    برای(i=0; i<p->nCell; i++){
      sqlite3_free(p->aCell[i].aOvfl);
    }
    sqlite3_free(p->aCell);
  }
  p->nCell = 0;
  p->aCell = 0;
}

ایستا تهی statClearPage(StatPage *p){
  u8 *aPg = p->aPg;
  statClearCells(p);
  sqlite3_free(p->zPath);
  memset(p, 0, اندازه(StatPage));
  p->aPg = aPg;
}

ایستا تهی statResetCsr(StatCursor *pCsr){
  صحیح i;
  /* In some circumstances, specifically if an OOM has occurred, the call
  ** to sqlite3_reset() may cause the pager to be reset (emptied). It is
  ** important that statClearPage() is called to free any page refs before
  ** this happens. dbsqlfuzz 9ed3e4e3816219d3509d711636c38542bf3f40b1. */
  برای(i=0; i<ArraySize(pCsr->aPage); i++){
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
ایستا تهی statResetCounts(StatCursor *pCsr){
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
ایستا صحیح statClose(sqlite3_vtab_cursor *pCursor){
  StatCursor *pCsr = (StatCursor *)pCursor;
  statResetCsr(pCsr);
  sqlite3_finalize(pCsr->pStmt);
  sqlite3_free(pCsr);
  بازگشت SQLITE_OK;
}

/*
** For a single cell on a btree page, compute the number of bytes of
** content (payload) stored on that page.  That is to say, compute the
** number of bytes of content not found on overflow pages.
*/
ایستا صحیح getLocalPayload(
  صحیح nUsable,                    /* Usable bytes per page */
  u8 flags,                       /* Page flags */
  صحیح nTotal                      /* Total record (payload) size */
){
  صحیح nLocal;
  صحیح nMinLocal;
  صحیح nMaxLocal;
 
  اگر( flags==0x0D ){              /* Table leaf node */
    nMinLocal = (nUsable - 12) * 32 / 255 - 23;
    nMaxLocal = nUsable - 35;
  }وگرنه{                          /* Index interior and leaf nodes */
    nMinLocal = (nUsable - 12) * 32 / 255 - 23;
    nMaxLocal = (nUsable - 12) * 64 / 255 - 23;
  }

  nLocal = nMinLocal + (nTotal - nMinLocal) % (nUsable - 4);
  اگر( nLocal>nMaxLocal ) nLocal = nMinLocal;
  بازگشت nLocal;
}

/* Populate the StatPage object with information about the all
** cells found on the page currently under analysis.
*/
ایستا صحیح statDecodePage(Btree *pBt, StatPage *p){
  صحیح nUnused;
  صحیح iOff;
  صحیح nHdr;
  صحیح isLeaf;
  صحیح szPage;

  u8 *aData = p->aPg;
  u8 *aHdr = &aData[p->iPgno==1 ? 100 : 0];

  p->flags = aHdr[0];
  اگر( p->flags==0x0A || p->flags==0x0D ){
    isLeaf = 1;
    nHdr = 8;
  }وگرنه اگر( p->flags==0x05 || p->flags==0x02 ){
    isLeaf = 0;
    nHdr = 12;
  }وگرنه{
    برو statPageIsCorrupt;
  }
  اگر( p->iPgno==1 ) nHdr += 100;
  p->nCell = get2byte(&aHdr[3]);
  p->nMxPayload = 0;
  szPage = sqlite3BtreeGetPageSize(pBt);

  nUnused = get2byte(&aHdr[5]) - nHdr - 2*p->nCell;
  nUnused += (صحیح)aHdr[7];
  iOff = get2byte(&aHdr[1]);
  وقتی( iOff ){
    صحیح iNext;
    اگر( iOff>=szPage ) برو statPageIsCorrupt;
    nUnused += get2byte(&aData[iOff+2]);
    iNext = get2byte(&aData[iOff]);
    اگر( iNext<iOff+4 && iNext>0 ) برو statPageIsCorrupt;
    iOff = iNext;
  }
  p->nUnused = nUnused;
  p->i_راست_Child_Pg = isLeaf ? 0 : sqlite3Get4byte(&aHdr[8]);

  اگر( p->nCell ){
    صحیح i;                        /* Used to iterate through cells */
    صحیح nUsable;                  /* Usable bytes per page */

    sqlite3BtreeEnter(pBt);
    nUsable = szPage - sqlite3BtreeGetReserveNoMutex(pBt);
    sqlite3BtreeLeave(pBt);
    p->aCell = sqlite3_malloc64((p->nCell+1) * اندازه(StatCell));
    اگر( p->aCell==0 ) بازگشت SQLITE_NOMEM_BKPT;
    memset(p->aCell, 0, (p->nCell+1) * اندازه(StatCell));

    برای(i=0; i<p->nCell; i++){
      StatCell *pCell = &p->aCell[i];

      iOff = get2byte(&aData[nHdr+i*2]);
      اگر( iOff<nHdr || iOff>=szPage ) برو statPageIsCorrupt;
      اگر( !isLeaf ){
        pCell->iChildPg = sqlite3Get4byte(&aData[iOff]);
        iOff += 4;
      }
      اگر( p->flags==0x05 ){
        /* A table interior node. nPayload==0. */
      }وگرنه{
        u32 nPayload;             /* Bytes of payload total (local+overflow) */
        صحیح nLocal;               /* Bytes of payload stored locally */
        iOff += getVarint32(&aData[iOff], nPayload);
        اگر( p->flags==0x0D ){
          u64 dummy;
          iOff += sqlite3GetVarint(&aData[iOff], &dummy);
        }
        اگر( nPayload>(u32)p->nMxPayload ) p->nMxPayload = nPayload;
        nLocal = getLocalPayload(nUsable, p->flags, nPayload);
        اگر( nLocal<0 ) برو statPageIsCorrupt;
        pCell->nLocal = nLocal;
        أكد( nPayload>=(u32)nLocal );
        أكد( nLocal<=(nUsable-35) );
        اگر( nPayload>(u32)nLocal ){
          صحیح j;
          صحیح nOvfl = ((nPayload - nLocal) + nUsable-4 - 1) / (nUsable - 4);
          اگر( iOff+nLocal+4>nUsable || nPayload>0x7fffffff ){
            برو statPageIsCorrupt;
          }
          pCell->nLastOvfl = (nPayload-nLocal) - (nOvfl-1) * (nUsable-4);
          pCell->nOvfl = nOvfl;
          pCell->aOvfl = sqlite3_malloc64(اندازه(u32)*nOvfl);
          اگر( pCell->aOvfl==0 ) بازگشت SQLITE_NOMEM_BKPT;
          pCell->aOvfl[0] = sqlite3Get4byte(&aData[iOff+nLocal]);
          برای(j=1; j<nOvfl; j++){
            صحیح rc;
            u32 iPrev = pCell->aOvfl[j-1];
            DbPage *pPg = 0;
            rc = sqlite3PagerGet(sqlite3BtreePager(pBt), iPrev, &pPg, 0);
            اگر( rc!=SQLITE_OK ){
              أكد( pPg==0 );
              بازگشت rc;
            } 
            pCell->aOvfl[j] = sqlite3Get4byte(sqlite3PagerGetData(pPg));
            sqlite3PagerUnref(pPg);
          }
        }
      }
    }
  }

  بازگشت SQLITE_OK;

statPageIsCorrupt:
  p->flags = 0;
  statClearCells(p);
  بازگشت SQLITE_OK;
}

/*
** Populate the pCsr->iOffset and pCsr->szPage member variables. Based on
** the current value of pCsr->iPageno.
*/
ایستا تهی statSizeAndOffset(StatCursor *pCsr){
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
  اگر( sqlite3OsFileControl(fd, 230440, &x)==SQLITE_OK ){
    pCsr->iOffset = x[0];
    pCsr->szPage += x[1];
  }وگرنه{
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
ایستا صحیح statGetPage(
  Btree *pBt,                     /* Load page from this b-tree */
  u32 iPg,                        /* Page number to load */
  StatPage *pPg                   /* Load page into this object */
){
  صحیح pgsz = sqlite3BtreeGetPageSize(pBt);
  DbPage *pDbPage = 0;
  صحیح rc;

  اگر( pPg->aPg==0 ){
    pPg->aPg = (u8*)sqlite3_malloc(pgsz + DBSTAT_PAGE_PADDING_BYTES);
    اگر( pPg->aPg==0 ){
      بازگشت SQLITE_NOMEM_BKPT;
    }
    memset(&pPg->aPg[pgsz], 0, DBSTAT_PAGE_PADDING_BYTES);
  }

  rc = sqlite3PagerGet(sqlite3BtreePager(pBt), iPg, &pDbPage, 0);
  اگر( rc==SQLITE_OK ){
    ثابت u8 *a = sqlite3PagerGetData(pDbPage);
    memcpy(pPg->aPg, a, pgsz);
    sqlite3PagerUnref(pDbPage);
  }

  بازگشت rc;
}

/*
** Move a DBSTAT cursor to the next entry.  Normally, the next
** entry will be the next page, but in aggregated mode (pCsr->isAgg!=0),
** the next entry is the next btree.
*/
ایستا صحیح statNext(sqlite3_vtab_cursor *pCursor){
  صحیح rc;
  صحیح nPayload;
  نویسه *z;
  StatCursor *pCsr = (StatCursor *)pCursor;
  StatTable *pTab = (StatTable *)pCursor->pVtab;
  Btree *pBt = pTab->db->aDb[pCsr->iDb].pBt;
  Pager *pPager = sqlite3BtreePager(pBt);

  sqlite3_free(pCsr->zPath);
  pCsr->zPath = 0;

statNextRestart:
  اگر( pCsr->iPage<0 ){
    /* Start measuring space on the next btree */
    statResetCounts(pCsr);
    rc = sqlite3_step(pCsr->pStmt);
    اگر( rc==SQLITE_ROW ){
      صحیح nPage;
      u32 i_ریشه = (u32)sqlite3_column_int64(pCsr->pStmt, 1);
      sqlite3PagerPagecount(pPager, &nPage);
      اگر( nPage==0 ){
        pCsr->isEof = 1;
        بازگشت sqlite3_reset(pCsr->pStmt);
      }
      rc = statGetPage(pBt, i_ریشه, &pCsr->aPage[0]);
      pCsr->aPage[0].iPgno = i_ریشه;
      pCsr->aPage[0].iCell = 0;
      اگر( !pCsr->isAgg ){
        pCsr->aPage[0].zPath = z = sqlite3_mprintf("/");
        اگر( z==0 ) rc = SQLITE_NOMEM_BKPT;
      }
      pCsr->iPage = 0;
      pCsr->nPage = 1;
    }وگرنه{
      pCsr->isEof = 1;
      بازگشت sqlite3_reset(pCsr->pStmt);
    }
  }وگرنه{
    /* Continue analyzing the btree previously started */
    StatPage *p = &pCsr->aPage[pCsr->iPage];
    اگر( !pCsr->isAgg ) statResetCounts(pCsr);
    وقتی( p->iCell<p->nCell ){
      StatCell *pCell = &p->aCell[p->iCell];
      وقتی( pCell->iOvfl<pCell->nOvfl ){
        صحیح nUsable;
        صحیح iOvfl = pCell->iOvfl;
        sqlite3BtreeEnter(pBt);
        nUsable = sqlite3BtreeGetPageSize(pBt) - 
                        sqlite3BtreeGetReserveNoMutex(pBt);
        sqlite3BtreeLeave(pBt);
        pCsr->nPage++;
        pCsr->iPageno = pCell->aOvfl[iOvfl];
        statSizeAndOffset(pCsr);
        اگر( pCell->iOvfl<pCell->nOvfl-1 ){
          pCsr->nPayload += nUsable - 4;
        }وگرنه{
          pCsr->nPayload += pCell->nLastOvfl;
          pCsr->nUnused += nUsable - 4 - pCell->nLastOvfl;
        }
        pCell->iOvfl++;
        اگر( !pCsr->isAgg ){
          pCsr->zName = (نویسه *)sqlite3_column_text(pCsr->pStmt, 0);
          pCsr->zPagetype = "overflow";
          pCsr->zPath = z = sqlite3_mprintf(
              "%s%.3x+%.6x", p->zPath, p->iCell, iOvfl
          );
          بازگشت z==0 ? SQLITE_NOMEM_BKPT : SQLITE_OK;
        }
      }
      اگر( p->i_راست_Child_Pg ) بشکن;
      p->iCell++;
    }

    اگر( !p->i_راست_Child_Pg || p->iCell>p->nCell ){
      statClearPage(p);
      pCsr->iPage--;
      اگر( pCsr->isAgg && pCsr->iPage<0 ){
        /* label-statNext-done:  When computing aggregate space usage over
        ** an entire btree, this is the exit point from this function */
        بازگشت SQLITE_OK;
      }
      برو statNextRestart; /* Tail recursion */
    }
    pCsr->iPage++;
    اگر( pCsr->iPage>=ArraySize(pCsr->aPage) ){
      statResetCsr(pCsr);
      بازگشت SQLITE_CORRUPT_BKPT;
    }
    أكد( p==&pCsr->aPage[pCsr->iPage-1] );

    اگر( p->iCell==p->nCell ){
      p[1].iPgno = p->i_راست_Child_Pg;
    }وگرنه{
      p[1].iPgno = p->aCell[p->iCell].iChildPg;
    }
    rc = statGetPage(pBt, p[1].iPgno, &p[1]);
    pCsr->nPage++;
    p[1].iCell = 0;
    اگر( !pCsr->isAgg ){
      p[1].zPath = z = sqlite3_mprintf("%s%.3x/", p->zPath, p->iCell);
      اگر( z==0 ) rc = SQLITE_NOMEM_BKPT;
    }
    p->iCell++;
  }


  /* Populate the StatCursor fields with the values to be returned
  ** by the xColumn() and xRowid() methods.
  */
  اگر( rc==SQLITE_OK ){
    صحیح i;
    StatPage *p = &pCsr->aPage[pCsr->iPage];
    pCsr->zName = (نویسه *)sqlite3_column_text(pCsr->pStmt, 0);
    pCsr->iPageno = p->iPgno;

    rc = statDecodePage(pBt, p);
    اگر( rc==SQLITE_OK ){
      statSizeAndOffset(pCsr);

      انتخاب( p->flags ){
        حالت 0x05:             /* table internal */
        حالت 0x02:             /* index internal */
          pCsr->zPagetype = "internal";
          بشکن;
        حالت 0x0D:             /* table leaf */
        حالت 0x0A:             /* index leaf */
          pCsr->zPagetype = "leaf";
          بشکن;
        پیشفرض:
          pCsr->zPagetype = "corrupted";
          بشکن;
      }
      pCsr->nCell += p->nCell;
      pCsr->nUnused += p->nUnused;
      اگر( p->nMxPayload>pCsr->nMxPayload ) pCsr->nMxPayload = p->nMxPayload;
      اگر( !pCsr->isAgg ){
        pCsr->zPath = z = sqlite3_mprintf("%s", p->zPath);
        اگر( z==0 ) rc = SQLITE_NOMEM_BKPT;
      }
      nPayload = 0;
      برای(i=0; i<p->nCell; i++){
        nPayload += p->aCell[i].nLocal;
      }
      pCsr->nPayload += nPayload;

      /* If computing aggregate space usage by btree, continue with the
      ** next page.  The loop will exit via the return at label-statNext-done
      */
      اگر( pCsr->isAgg ) برو statNextRestart;
    }
  }

  بازگشت rc;
}

ایستا صحیح statEof(sqlite3_vtab_cursor *pCursor){
  StatCursor *pCsr = (StatCursor *)pCursor;
  بازگشت pCsr->isEof;
}

/* Initialize a cursor according to the query plan idxNum using the
** arguments in argv[0].  See statBestIndex() for a description of the
** meaning of the bits in idxNum.
*/
ایستا صحیح statFilter(
  sqlite3_vtab_cursor *pCursor, 
  صحیح idx_رقم, ثابت نویسه *idxStr,
  صحیح argc, sqlite3_مقدار **argv
){
  StatCursor *pCsr = (StatCursor *)pCursor;
  StatTable *pTab = (StatTable*)(pCursor->pVtab);
  sqlite3_str *pSql;      /* Query of btrees to analyze */
  نویسه *zSql;             /* String value of pSql */
  صحیح iArg = 0;           /* Count of argv[] parameters used so far */
  صحیح rc = SQLITE_OK;     /* Result of this operation */
  ثابت نویسه *zName = 0;  /* Only provide analysis of this table */
  (تهی)argc;
  (تهی)idxStr;

  statResetCsr(pCsr);
  sqlite3_finalize(pCsr->pStmt);
  pCsr->pStmt = 0;
  اگر( idx_رقم & 0x01 ){
    /* schema=? constraint is present.  Get its value */
    ثابت نویسه *zDbase = (ثابت نویسه*)sqlite3_مقدار_text(argv[iArg++]);
    pCsr->iDb = sqlite3FindDbName(pTab->db, zDbase);
    اگر( pCsr->iDb<0 ){
      pCsr->iDb = 0;
      pCsr->isEof = 1;
      بازگشت SQLITE_OK;
    }
  }وگرنه{
    pCsr->iDb = pTab->iDb;
  }
  اگر( idx_رقم & 0x02 ){
    /* name=? constraint is present */
    zName = (ثابت نویسه*)sqlite3_مقدار_text(argv[iArg++]);
  }
  اگر( idx_رقم & 0x04 ){
    /* aggregate=? constraint is present */
    pCsr->isAgg = sqlite3_مقدار_double(argv[iArg++])!=0.0;
  }وگرنه{
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
  اگر( zName ){
    sqlite3_str_appendf(pSql, "WHERE name=%Q", zName);
  }
  اگر( idx_رقم & 0x08 ){
    sqlite3_str_appendf(pSql, " ORDER BY name");
  }
  zSql = sqlite3_str_finish(pSql);
  اگر( zSql==0 ){
    بازگشت SQLITE_NOMEM_BKPT;
  }وگرنه{
    rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pStmt, 0);
    sqlite3_free(zSql);
  }

  اگر( rc==SQLITE_OK ){
    pCsr->iPage = -1;
    rc = statNext(pCursor);
  }
  بازگشت rc;
}

ایستا صحیح statColumn(
  sqlite3_vtab_cursor *pCursor, 
  sqlite3_context *ctx, 
  صحیح i
){
  StatCursor *pCsr = (StatCursor *)pCursor;
  انتخاب( i ){
    حالت 0:            /* name */
      sqlite3_result_text(ctx, pCsr->zName, -1, SQLITE_TRANSIENT);
      بشکن;
    حالت 1:            /* path */
      اگر( !pCsr->isAgg ){
        sqlite3_result_text(ctx, pCsr->zPath, -1, SQLITE_TRANSIENT);
      }
      بشکن;
    حالت 2:            /* pageno */
      اگر( pCsr->isAgg ){
        sqlite3_result_int64(ctx, pCsr->nPage);
      }وگرنه{
        sqlite3_result_int64(ctx, pCsr->iPageno);
      }
      بشکن;
    حالت 3:            /* pagetype */
      اگر( !pCsr->isAgg ){
        sqlite3_result_text(ctx, pCsr->zPagetype, -1, SQLITE_STATIC);
      }
      بشکن;
    حالت 4:            /* ncell */
      sqlite3_result_int64(ctx, pCsr->nCell);
      بشکن;
    حالت 5:            /* payload */
      sqlite3_result_int64(ctx, pCsr->nPayload);
      بشکن;
    حالت 6:            /* unused */
      sqlite3_result_int64(ctx, pCsr->nUnused);
      بشکن;
    حالت 7:            /* mx_payload */
      sqlite3_result_int64(ctx, pCsr->nMxPayload);
      بشکن;
    حالت 8:            /* pgoffset */
      اگر( !pCsr->isAgg ){
        sqlite3_result_int64(ctx, pCsr->iOffset);
      }
      بشکن;
    حالت 9:            /* pgsize */
      sqlite3_result_int64(ctx, pCsr->szPage);
      بشکن;
    حالت 10: {         /* schema */
      sqlite3 *db = sqlite3_context_db_handle(ctx);
      صحیح iDb = pCsr->iDb;
      sqlite3_result_text(ctx, db->aDb[iDb].zDbSName, -1, SQLITE_STATIC);
      بشکن;
    }
    پیشفرض: {         /* aggregate */
      sqlite3_result_int(ctx, pCsr->isAgg);
      بشکن;
    }
  }
  بازگشت SQLITE_OK;
}

ایستا صحیح statRowid(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid){
  StatCursor *pCsr = (StatCursor *)pCursor;
  *pRowid = pCsr->iPageno;
  بازگشت SQLITE_OK;
}

/*
** Invoke this routine to register the "dbstat" virtual table module
*/
صحیح sqlite3DbstatRegister(sqlite3 *db){
  ایستا sqlite3_module dbstat_module = {
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
  بازگشت sqlite3_create_module(db, "dbstat", &dbstat_module, 0);
}
#وگرنهاگر defined(SQLITE_ENABLE_DBSTAT_VTAB)
صحیح sqlite3DbstatRegister(sqlite3 *db){ بازگشت SQLITE_OK; }
#endif /* SQLITE_ENABLE_DBSTAT_VTAB */
