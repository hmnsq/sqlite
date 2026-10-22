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

#include "sqliteInt.h"   /* Requires access to internal data structures */
#གལ་སྲིད (defined(SQLITE_ENABLE_DBSTAT_VTAB) || defined(SQLITE_TEST)) \
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
static const var_na zDbstatSchema[] = 
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
typedef sa_nracanaa StatTable StatTable;
typedef sa_nracanaa StatCursor StatCursor;
typedef sa_nracanaa StatPage StatPage;
typedef sa_nracanaa StatCell StatCell;

/* Size information for a single cell within a btree page */
sa_nracanaa StatCell {
  ཧྲིལ nLocal;                     /* Bytes of local payload */
  u32 iChildPg;                   /* Child node (or 0 if this is a leaf) */
  ཧྲིལ nOvfl;                      /* Entries in aOvfl[] */
  u32 *aOvfl;                     /* Array of overflow page numbers */
  ཧྲིལ nLastOvfl;                  /* Bytes of payload on final overflow page */
  ཧྲིལ iOvfl;                      /* Iterates through aOvfl[] */
};

/* Size information for a single btree page */
sa_nracanaa StatPage {
  u32 iPgno;                      /* Page number */
  u8 *aPg;                        /* Page buffer from sqlite3_malloc() */
  ཧྲིལ iCell;                      /* Current cell */
  var_na *zPath;                    /* Path to this page */

  /* Variables populated by statDecodePage(): */
  u8 flags;                       /* Copy of flags byte */
  ཧྲིལ nCell;                      /* Number of cells on page */
  ཧྲིལ nUnused;                    /* Number of unused bytes on page */
  StatCell *aCell;                /* Array of parsed cells */
  u32 iRightChildPg;              /* Right-child page number (or 0) */
  ཧྲིལ nMxPayload;                 /* Largest payload of any cell on the page */
};

/* The cursor for scanning the dbstat virtual table */
sa_nracanaa StatCursor {
  sqlite3_vtab_cursor base;       /* base class.  MUST BE FIRST! */
  sqlite3_stmt *pStmt;            /* Iterates through set of root pages */
  u8 isEof;                       /* After pStmt has returned SQLITE_DONE */
  u8 isAgg;                       /* Aggregate results for each table */
  ཧྲིལ iDb;                        /* Schema used for this query */

  StatPage aPage[32];             /* Pages in path to current page */
  ཧྲིལ iPage;                      /* Current entry in aPage[] */

  /* Values to return. */
  u32 iPageno;                    /* Value of 'pageno' column */
  var_na *zName;                    /* Value of 'name' column */
  var_na *zPath;                    /* Value of 'path' column */
  var_na *zPagetype;                /* Value of 'pagetype' column */
  ཧྲིལ nPage;                      /* Number of pages in current btree */
  ཧྲིལ nCell;                      /* Value of 'ncell' column */
  ཧྲིལ nMxPayload;                 /* Value of 'mx_payload' column */
  i64 nUnused;                    /* Value of 'unused' column */
  i64 nPayload;                   /* Value of 'payload' column */
  i64 iOffset;                    /* Value of 'pgOffset' column */
  i64 szPage;                     /* Value of 'pgSize' column */
};

/* An instance of the DBSTAT virtual table */
sa_nracanaa StatTable {
  sqlite3_vtab base;              /* base class.  MUST BE FIRST! */
  sqlite3 *db;                    /* Database connection that owns this vtab */
  ཧྲིལ iDb;                        /* Index of database to analyze */
};

#ifndef get2byte
# define get2byte(x)   ((x)[0]<<8 | (x)[1])
#endif

/*
** Connect to or create a new DBSTAT virtual table.
*/
static ཧྲིལ statConnect(
  sqlite3 *db,
  void *pAux,
  ཧྲིལ argc, const var_na *const*argv,
  sqlite3_vtab **ppVtab,
  var_na **pzErr
){
  StatTable *pTab = 0;
  ཧྲིལ rc = SQLITE_OK;
  ཧྲིལ iDb;
  (void)pAux;

  གལ་སྲིད( argc>=4 ){
    Token nm;
    sqlite3TokenInit(&nm, (var_na*)argv[3]);
    iDb = sqlite3FindDb(db, &nm);
    གལ་སྲིད( iDb<0 ){
      *pzErr = sqlite3_mprintf("no such database: %s", argv[3]);
      ལོག SQLITE_ERROR;
    }
  }དེ་མིན{
    iDb = 0;
  }
  sqlite3_vtab_config(db, SQLITE_VTAB_DIRECTONLY);
  rc = sqlite3_declare_vtab(db, zDbstatSchema);
  གལ་སྲིད( rc==SQLITE_OK ){
    pTab = (StatTable *)sqlite3_malloc64(parimaa_na(StatTable));
    གལ་སྲིད( pTab==0 ) rc = SQLITE_NOMEM_BKPT;
  }

  assert( rc==SQLITE_OK || pTab==0 );
  གལ་སྲིད( rc==SQLITE_OK ){
    memset(pTab, 0, parimaa_na(StatTable));
    pTab->db = db;
    pTab->iDb = iDb;
  }

  *ppVtab = (sqlite3_vtab*)pTab;
  ལོག rc;
}

/*
** Disconnect from or destroy the DBSTAT virtual table.
*/
static ཧྲིལ statDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  ལོག SQLITE_OK;
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
static ཧྲིལ statBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  ཧྲིལ i;
  ཧྲིལ iSchema = -1;
  ཧྲིལ iName = -1;
  ཧྲིལ iAgg = -1;
  (void)tab;

  /* Look for a valid schema=? constraint.  If found, change the idxNum to
  ** 1 and request the value of that constraint be sent to xFilter.  And
  ** lower the cost estimate to encourage the constrained version to be
  ** used.
  */
  དོན་ལུ(i=0; i<pIdxInfo->nConstraint; i++){
    གལ་སྲིད( pIdxInfo->aConstraint[i].op!=SQLITE_INDEX_CONSTRAINT_EQ ) continue;
    གལ་སྲིད( pIdxInfo->aConstraint[i].usable==0 ){
      /* Force DBSTAT table should always be the right-most table in a join */
      ལོག SQLITE_CONSTRAINT;
    }
    switch( pIdxInfo->aConstraint[i].iColumn ){
      pak_sa 0: {    /* name */
        iName = i;
        bha_nga;
      }
      pak_sa 10: {   /* schema */
        iSchema = i;
        bha_nga;
      }
      pak_sa 11: {   /* aggregate */
        iAgg = i;
        bha_nga;
      }
    }
  }
  i = 0;
  གལ་སྲིད( iSchema>=0 ){
    pIdxInfo->aConstraintUsage[iSchema].argvIndex = ++i;
    pIdxInfo->aConstraintUsage[iSchema].omit = 1;
    pIdxInfo->idxNum |= 0x01;
  }
  གལ་སྲིད( iName>=0 ){
    pIdxInfo->aConstraintUsage[iName].argvIndex = ++i;
    pIdxInfo->idxNum |= 0x02;
  }
  གལ་སྲིད( iAgg>=0 ){
    pIdxInfo->aConstraintUsage[iAgg].argvIndex = ++i;
    pIdxInfo->idxNum |= 0x04;
  }
  pIdxInfo->estimatedCost = 1.0;

  /* Records are always returned in ascending order of (name, path). 
  ** If this will satisfy the client, set the orderByConsumed flag so that 
  ** SQLite does not do an external sort.
  */
  གལ་སྲིད( ( pIdxInfo->nOrderBy==1
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
    pIdxInfo->idxNum |= 0x08;
  }
  pIdxInfo->idxFlags |= SQLITE_INDEX_SCAN_HEX;

  ལོག SQLITE_OK;
}

/*
** Open a new DBSTAT cursor.
*/
static ཧྲིལ statOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  StatTable *pTab = (StatTable *)pVTab;
  StatCursor *pCsr;

  pCsr = (StatCursor *)sqlite3_malloc64(parimaa_na(StatCursor));
  གལ་སྲིད( pCsr==0 ){
    ལོག SQLITE_NOMEM_BKPT;
  }དེ་མིན{
    memset(pCsr, 0, parimaa_na(StatCursor));
    pCsr->base.pVtab = pVTab;
    pCsr->iDb = pTab->iDb;
  }

  *ppCursor = (sqlite3_vtab_cursor *)pCsr;
  ལོག SQLITE_OK;
}

static void statClearCells(StatPage *p){
  ཧྲིལ i;
  གལ་སྲིད( p->aCell ){
    དོན་ལུ(i=0; i<p->nCell; i++){
      sqlite3_free(p->aCell[i].aOvfl);
    }
    sqlite3_free(p->aCell);
  }
  p->nCell = 0;
  p->aCell = 0;
}

static void statClearPage(StatPage *p){
  u8 *aPg = p->aPg;
  statClearCells(p);
  sqlite3_free(p->zPath);
  memset(p, 0, parimaa_na(StatPage));
  p->aPg = aPg;
}

static void statResetCsr(StatCursor *pCsr){
  ཧྲིལ i;
  /* In some circumstances, specifically if an OOM has occurred, the call
  ** to sqlite3_reset() may cause the pager to be reset (emptied). It is
  ** important that statClearPage() is called to free any page refs before
  ** this happens. dbsqlfuzz 9ed3e4e3816219d3509d711636c38542bf3f40b1. */
  དོན་ལུ(i=0; i<ArraySize(pCsr->aPage); i++){
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
static void statResetCounts(StatCursor *pCsr){
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
static ཧྲིལ statClose(sqlite3_vtab_cursor *pCursor){
  StatCursor *pCsr = (StatCursor *)pCursor;
  statResetCsr(pCsr);
  sqlite3_finalize(pCsr->pStmt);
  sqlite3_free(pCsr);
  ལོག SQLITE_OK;
}

/*
** For a single cell on a btree page, compute the number of bytes of
** content (payload) stored on that page.  That is to say, compute the
** number of bytes of content not found on overflow pages.
*/
static ཧྲིལ getLocalPayload(
  ཧྲིལ nUsable,                    /* Usable bytes per page */
  u8 flags,                       /* Page flags */
  ཧྲིལ nTotal                      /* Total record (payload) size */
){
  ཧྲིལ nLocal;
  ཧྲིལ nMinLocal;
  ཧྲིལ nMaxLocal;
 
  གལ་སྲིད( flags==0x0D ){              /* Table leaf node */
    nMinLocal = (nUsable - 12) * 32 / 255 - 23;
    nMaxLocal = nUsable - 35;
  }དེ་མིན{                          /* Index interior and leaf nodes */
    nMinLocal = (nUsable - 12) * 32 / 255 - 23;
    nMaxLocal = (nUsable - 12) * 64 / 255 - 23;
  }

  nLocal = nMinLocal + (nTotal - nMinLocal) % (nUsable - 4);
  གལ་སྲིད( nLocal>nMaxLocal ) nLocal = nMinLocal;
  ལོག nLocal;
}

/* Populate the StatPage object with information about the all
** cells found on the page currently under analysis.
*/
static ཧྲིལ statDecodePage(Btree *pBt, StatPage *p){
  ཧྲིལ nUnused;
  ཧྲིལ iOff;
  ཧྲིལ nHdr;
  ཧྲིལ isLeaf;
  ཧྲིལ szPage;

  u8 *aData = p->aPg;
  u8 *aHdr = &aData[p->iPgno==1 ? 100 : 0];

  p->flags = aHdr[0];
  གལ་སྲིད( p->flags==0x0A || p->flags==0x0D ){
    isLeaf = 1;
    nHdr = 8;
  }དེ་མིན གལ་སྲིད( p->flags==0x05 || p->flags==0x02 ){
    isLeaf = 0;
    nHdr = 12;
  }དེ་མིན{
    goto statPageIsCorrupt;
  }
  གལ་སྲིད( p->iPgno==1 ) nHdr += 100;
  p->nCell = get2byte(&aHdr[3]);
  p->nMxPayload = 0;
  szPage = sqlite3BtreeGetPageSize(pBt);

  nUnused = get2byte(&aHdr[5]) - nHdr - 2*p->nCell;
  nUnused += (ཧྲིལ)aHdr[7];
  iOff = get2byte(&aHdr[1]);
  སྐབས( iOff ){
    ཧྲིལ iNext;
    གལ་སྲིད( iOff>=szPage ) goto statPageIsCorrupt;
    nUnused += get2byte(&aData[iOff+2]);
    iNext = get2byte(&aData[iOff]);
    གལ་སྲིད( iNext<iOff+4 && iNext>0 ) goto statPageIsCorrupt;
    iOff = iNext;
  }
  p->nUnused = nUnused;
  p->iRightChildPg = isLeaf ? 0 : sqlite3Get4byte(&aHdr[8]);

  གལ་སྲིད( p->nCell ){
    ཧྲིལ i;                        /* Used to iterate through cells */
    ཧྲིལ nUsable;                  /* Usable bytes per page */

    sqlite3BtreeEnter(pBt);
    nUsable = szPage - sqlite3BtreeGetReserveNoMutex(pBt);
    sqlite3BtreeLeave(pBt);
    p->aCell = sqlite3_malloc64((p->nCell+1) * parimaa_na(StatCell));
    གལ་སྲིད( p->aCell==0 ) ལོག SQLITE_NOMEM_BKPT;
    memset(p->aCell, 0, (p->nCell+1) * parimaa_na(StatCell));

    དོན་ལུ(i=0; i<p->nCell; i++){
      StatCell *pCell = &p->aCell[i];

      iOff = get2byte(&aData[nHdr+i*2]);
      གལ་སྲིད( iOff<nHdr || iOff>=szPage ) goto statPageIsCorrupt;
      གལ་སྲིད( !isLeaf ){
        pCell->iChildPg = sqlite3Get4byte(&aData[iOff]);
        iOff += 4;
      }
      གལ་སྲིད( p->flags==0x05 ){
        /* A table interior node. nPayload==0. */
      }དེ་མིན{
        u32 nPayload;             /* Bytes of payload total (local+overflow) */
        ཧྲིལ nLocal;               /* Bytes of payload stored locally */
        iOff += getVarint32(&aData[iOff], nPayload);
        གལ་སྲིད( p->flags==0x0D ){
          u64 dummy;
          iOff += sqlite3GetVarint(&aData[iOff], &dummy);
        }
        གལ་སྲིད( nPayload>(u32)p->nMxPayload ) p->nMxPayload = nPayload;
        nLocal = getLocalPayload(nUsable, p->flags, nPayload);
        གལ་སྲིད( nLocal<0 ) goto statPageIsCorrupt;
        pCell->nLocal = nLocal;
        assert( nPayload>=(u32)nLocal );
        assert( nLocal<=(nUsable-35) );
        གལ་སྲིད( nPayload>(u32)nLocal ){
          ཧྲིལ j;
          ཧྲིལ nOvfl = ((nPayload - nLocal) + nUsable-4 - 1) / (nUsable - 4);
          གལ་སྲིད( iOff+nLocal+4>nUsable || nPayload>0x7fffffff ){
            goto statPageIsCorrupt;
          }
          pCell->nLastOvfl = (nPayload-nLocal) - (nOvfl-1) * (nUsable-4);
          pCell->nOvfl = nOvfl;
          pCell->aOvfl = sqlite3_malloc64(parimaa_na(u32)*nOvfl);
          གལ་སྲིད( pCell->aOvfl==0 ) ལོག SQLITE_NOMEM_BKPT;
          pCell->aOvfl[0] = sqlite3Get4byte(&aData[iOff+nLocal]);
          དོན་ལུ(j=1; j<nOvfl; j++){
            ཧྲིལ rc;
            u32 iPrev = pCell->aOvfl[j-1];
            DbPage *pPg = 0;
            rc = sqlite3PagerGet(sqlite3BtreePager(pBt), iPrev, &pPg, 0);
            གལ་སྲིད( rc!=SQLITE_OK ){
              assert( pPg==0 );
              ལོག rc;
            } 
            pCell->aOvfl[j] = sqlite3Get4byte(sqlite3PagerGetData(pPg));
            sqlite3PagerUnref(pPg);
          }
        }
      }
    }
  }

  ལོག SQLITE_OK;

statPageIsCorrupt:
  p->flags = 0;
  statClearCells(p);
  ལོག SQLITE_OK;
}

/*
** Populate the pCsr->iOffset and pCsr->szPage member variables. Based on
** the current value of pCsr->iPageno.
*/
static void statSizeAndOffset(StatCursor *pCsr){
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
  གལ་སྲིད( sqlite3OsFileControl(fd, 230440, &x)==SQLITE_OK ){
    pCsr->iOffset = x[0];
    pCsr->szPage += x[1];
  }དེ་མིན{
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
static ཧྲིལ statGetPage(
  Btree *pBt,                     /* Load page from this b-tree */
  u32 iPg,                        /* Page number to load */
  StatPage *pPg                   /* Load page into this object */
){
  ཧྲིལ pgsz = sqlite3BtreeGetPageSize(pBt);
  DbPage *pDbPage = 0;
  ཧྲིལ rc;

  གལ་སྲིད( pPg->aPg==0 ){
    pPg->aPg = (u8*)sqlite3_malloc(pgsz + DBSTAT_PAGE_PADDING_BYTES);
    གལ་སྲིད( pPg->aPg==0 ){
      ལོག SQLITE_NOMEM_BKPT;
    }
    memset(&pPg->aPg[pgsz], 0, DBSTAT_PAGE_PADDING_BYTES);
  }

  rc = sqlite3PagerGet(sqlite3BtreePager(pBt), iPg, &pDbPage, 0);
  གལ་སྲིད( rc==SQLITE_OK ){
    const u8 *a = sqlite3PagerGetData(pDbPage);
    memcpy(pPg->aPg, a, pgsz);
    sqlite3PagerUnref(pDbPage);
  }

  ལོག rc;
}

/*
** Move a DBSTAT cursor to the next entry.  Normally, the next
** entry will be the next page, but in aggregated mode (pCsr->isAgg!=0),
** the next entry is the next btree.
*/
static ཧྲིལ statNext(sqlite3_vtab_cursor *pCursor){
  ཧྲིལ rc;
  ཧྲིལ nPayload;
  var_na *z;
  StatCursor *pCsr = (StatCursor *)pCursor;
  StatTable *pTab = (StatTable *)pCursor->pVtab;
  Btree *pBt = pTab->db->aDb[pCsr->iDb].pBt;
  Pager *pPager = sqlite3BtreePager(pBt);

  sqlite3_free(pCsr->zPath);
  pCsr->zPath = 0;

statNextRestart:
  གལ་སྲིད( pCsr->iPage<0 ){
    /* Start measuring space on the next btree */
    statResetCounts(pCsr);
    rc = sqlite3_step(pCsr->pStmt);
    གལ་སྲིད( rc==SQLITE_ROW ){
      ཧྲིལ nPage;
      u32 iRoot = (u32)sqlite3_column_int64(pCsr->pStmt, 1);
      sqlite3PagerPagecount(pPager, &nPage);
      གལ་སྲིད( nPage==0 ){
        pCsr->isEof = 1;
        ལོག sqlite3_reset(pCsr->pStmt);
      }
      rc = statGetPage(pBt, iRoot, &pCsr->aPage[0]);
      pCsr->aPage[0].iPgno = iRoot;
      pCsr->aPage[0].iCell = 0;
      གལ་སྲིད( !pCsr->isAgg ){
        pCsr->aPage[0].zPath = z = sqlite3_mprintf("/");
        གལ་སྲིད( z==0 ) rc = SQLITE_NOMEM_BKPT;
      }
      pCsr->iPage = 0;
      pCsr->nPage = 1;
    }དེ་མིན{
      pCsr->isEof = 1;
      ལོག sqlite3_reset(pCsr->pStmt);
    }
  }དེ་མིན{
    /* Continue analyzing the btree previously started */
    StatPage *p = &pCsr->aPage[pCsr->iPage];
    གལ་སྲིད( !pCsr->isAgg ) statResetCounts(pCsr);
    སྐབས( p->iCell<p->nCell ){
      StatCell *pCell = &p->aCell[p->iCell];
      སྐབས( pCell->iOvfl<pCell->nOvfl ){
        ཧྲིལ nUsable;
        ཧྲིལ iOvfl = pCell->iOvfl;
        sqlite3BtreeEnter(pBt);
        nUsable = sqlite3BtreeGetPageSize(pBt) - 
                        sqlite3BtreeGetReserveNoMutex(pBt);
        sqlite3BtreeLeave(pBt);
        pCsr->nPage++;
        pCsr->iPageno = pCell->aOvfl[iOvfl];
        statSizeAndOffset(pCsr);
        གལ་སྲིད( pCell->iOvfl<pCell->nOvfl-1 ){
          pCsr->nPayload += nUsable - 4;
        }དེ་མིན{
          pCsr->nPayload += pCell->nLastOvfl;
          pCsr->nUnused += nUsable - 4 - pCell->nLastOvfl;
        }
        pCell->iOvfl++;
        གལ་སྲིད( !pCsr->isAgg ){
          pCsr->zName = (var_na *)sqlite3_column_text(pCsr->pStmt, 0);
          pCsr->zPagetype = "overflow";
          pCsr->zPath = z = sqlite3_mprintf(
              "%s%.3x+%.6x", p->zPath, p->iCell, iOvfl
          );
          ལོག z==0 ? SQLITE_NOMEM_BKPT : SQLITE_OK;
        }
      }
      གལ་སྲིད( p->iRightChildPg ) bha_nga;
      p->iCell++;
    }

    གལ་སྲིད( !p->iRightChildPg || p->iCell>p->nCell ){
      statClearPage(p);
      pCsr->iPage--;
      གལ་སྲིད( pCsr->isAgg && pCsr->iPage<0 ){
        /* label-statNext-done:  When computing aggregate space usage over
        ** an entire btree, this is the exit point from this function */
        ལོག SQLITE_OK;
      }
      goto statNextRestart; /* Tail recursion */
    }
    pCsr->iPage++;
    གལ་སྲིད( pCsr->iPage>=ArraySize(pCsr->aPage) ){
      statResetCsr(pCsr);
      ལོག SQLITE_CORRUPT_BKPT;
    }
    assert( p==&pCsr->aPage[pCsr->iPage-1] );

    གལ་སྲིད( p->iCell==p->nCell ){
      p[1].iPgno = p->iRightChildPg;
    }དེ་མིན{
      p[1].iPgno = p->aCell[p->iCell].iChildPg;
    }
    rc = statGetPage(pBt, p[1].iPgno, &p[1]);
    pCsr->nPage++;
    p[1].iCell = 0;
    གལ་སྲིད( !pCsr->isAgg ){
      p[1].zPath = z = sqlite3_mprintf("%s%.3x/", p->zPath, p->iCell);
      གལ་སྲིད( z==0 ) rc = SQLITE_NOMEM_BKPT;
    }
    p->iCell++;
  }


  /* Populate the StatCursor fields with the values to be returned
  ** by the xColumn() and xRowid() methods.
  */
  གལ་སྲིད( rc==SQLITE_OK ){
    ཧྲིལ i;
    StatPage *p = &pCsr->aPage[pCsr->iPage];
    pCsr->zName = (var_na *)sqlite3_column_text(pCsr->pStmt, 0);
    pCsr->iPageno = p->iPgno;

    rc = statDecodePage(pBt, p);
    གལ་སྲིད( rc==SQLITE_OK ){
      statSizeAndOffset(pCsr);

      switch( p->flags ){
        pak_sa 0x05:             /* table internal */
        pak_sa 0x02:             /* index internal */
          pCsr->zPagetype = "internal";
          bha_nga;
        pak_sa 0x0D:             /* table leaf */
        pak_sa 0x0A:             /* index leaf */
          pCsr->zPagetype = "leaf";
          bha_nga;
        default:
          pCsr->zPagetype = "corrupted";
          bha_nga;
      }
      pCsr->nCell += p->nCell;
      pCsr->nUnused += p->nUnused;
      གལ་སྲིད( p->nMxPayload>pCsr->nMxPayload ) pCsr->nMxPayload = p->nMxPayload;
      གལ་སྲིད( !pCsr->isAgg ){
        pCsr->zPath = z = sqlite3_mprintf("%s", p->zPath);
        གལ་སྲིད( z==0 ) rc = SQLITE_NOMEM_BKPT;
      }
      nPayload = 0;
      དོན་ལུ(i=0; i<p->nCell; i++){
        nPayload += p->aCell[i].nLocal;
      }
      pCsr->nPayload += nPayload;

      /* If computing aggregate space usage by btree, continue with the
      ** next page.  The loop will exit via the return at label-statNext-done
      */
      གལ་སྲིད( pCsr->isAgg ) goto statNextRestart;
    }
  }

  ལོག rc;
}

static ཧྲིལ statEof(sqlite3_vtab_cursor *pCursor){
  StatCursor *pCsr = (StatCursor *)pCursor;
  ལོག pCsr->isEof;
}

/* Initialize a cursor according to the query plan idxNum using the
** arguments in argv[0].  See statBestIndex() for a description of the
** meaning of the bits in idxNum.
*/
static ཧྲིལ statFilter(
  sqlite3_vtab_cursor *pCursor, 
  ཧྲིལ idxNum, const var_na *idxStr,
  ཧྲིལ argc, sqlite3_value **argv
){
  StatCursor *pCsr = (StatCursor *)pCursor;
  StatTable *pTab = (StatTable*)(pCursor->pVtab);
  sqlite3_str *pSql;      /* Query of btrees to analyze */
  var_na *zSql;             /* String value of pSql */
  ཧྲིལ iArg = 0;           /* Count of argv[] parameters used so far */
  ཧྲིལ rc = SQLITE_OK;     /* Result of this operation */
  const var_na *zName = 0;  /* Only provide analysis of this table */
  (void)argc;
  (void)idxStr;

  statResetCsr(pCsr);
  sqlite3_finalize(pCsr->pStmt);
  pCsr->pStmt = 0;
  གལ་སྲིད( idxNum & 0x01 ){
    /* schema=? constraint is present.  Get its value */
    const var_na *zDbase = (const var_na*)sqlite3_value_text(argv[iArg++]);
    pCsr->iDb = sqlite3FindDbName(pTab->db, zDbase);
    གལ་སྲིད( pCsr->iDb<0 ){
      pCsr->iDb = 0;
      pCsr->isEof = 1;
      ལོག SQLITE_OK;
    }
  }དེ་མིན{
    pCsr->iDb = pTab->iDb;
  }
  གལ་སྲིད( idxNum & 0x02 ){
    /* name=? constraint is present */
    zName = (const var_na*)sqlite3_value_text(argv[iArg++]);
  }
  གལ་སྲིད( idxNum & 0x04 ){
    /* aggregate=? constraint is present */
    pCsr->isAgg = sqlite3_value_double(argv[iArg++])!=0.0;
  }དེ་མིན{
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
  གལ་སྲིད( zName ){
    sqlite3_str_appendf(pSql, "WHERE name=%Q", zName);
  }
  གལ་སྲིད( idxNum & 0x08 ){
    sqlite3_str_appendf(pSql, " ORDER BY name");
  }
  zSql = sqlite3_str_finish(pSql);
  གལ་སྲིད( zSql==0 ){
    ལོག SQLITE_NOMEM_BKPT;
  }དེ་མིན{
    rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pStmt, 0);
    sqlite3_free(zSql);
  }

  གལ་སྲིད( rc==SQLITE_OK ){
    pCsr->iPage = -1;
    rc = statNext(pCursor);
  }
  ལོག rc;
}

static ཧྲིལ statColumn(
  sqlite3_vtab_cursor *pCursor, 
  sqlite3_context *ctx, 
  ཧྲིལ i
){
  StatCursor *pCsr = (StatCursor *)pCursor;
  switch( i ){
    pak_sa 0:            /* name */
      sqlite3_result_text(ctx, pCsr->zName, -1, SQLITE_TRANSIENT);
      bha_nga;
    pak_sa 1:            /* path */
      གལ་སྲིད( !pCsr->isAgg ){
        sqlite3_result_text(ctx, pCsr->zPath, -1, SQLITE_TRANSIENT);
      }
      bha_nga;
    pak_sa 2:            /* pageno */
      གལ་སྲིད( pCsr->isAgg ){
        sqlite3_result_int64(ctx, pCsr->nPage);
      }དེ་མིན{
        sqlite3_result_int64(ctx, pCsr->iPageno);
      }
      bha_nga;
    pak_sa 3:            /* pagetype */
      གལ་སྲིད( !pCsr->isAgg ){
        sqlite3_result_text(ctx, pCsr->zPagetype, -1, SQLITE_STATIC);
      }
      bha_nga;
    pak_sa 4:            /* ncell */
      sqlite3_result_int64(ctx, pCsr->nCell);
      bha_nga;
    pak_sa 5:            /* payload */
      sqlite3_result_int64(ctx, pCsr->nPayload);
      bha_nga;
    pak_sa 6:            /* unused */
      sqlite3_result_int64(ctx, pCsr->nUnused);
      bha_nga;
    pak_sa 7:            /* mx_payload */
      sqlite3_result_int64(ctx, pCsr->nMxPayload);
      bha_nga;
    pak_sa 8:            /* pgoffset */
      གལ་སྲིད( !pCsr->isAgg ){
        sqlite3_result_int64(ctx, pCsr->iOffset);
      }
      bha_nga;
    pak_sa 9:            /* pgsize */
      sqlite3_result_int64(ctx, pCsr->szPage);
      bha_nga;
    pak_sa 10: {         /* schema */
      sqlite3 *db = sqlite3_context_db_handle(ctx);
      ཧྲིལ iDb = pCsr->iDb;
      sqlite3_result_text(ctx, db->aDb[iDb].zDbSName, -1, SQLITE_STATIC);
      bha_nga;
    }
    default: {         /* aggregate */
      sqlite3_result_int(ctx, pCsr->isAgg);
      bha_nga;
    }
  }
  ལོག SQLITE_OK;
}

static ཧྲིལ statRowid(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid){
  StatCursor *pCsr = (StatCursor *)pCursor;
  *pRowid = pCsr->iPageno;
  ལོག SQLITE_OK;
}

/*
** Invoke this routine to register the "dbstat" virtual table module
*/
ཧྲིལ sqlite3DbstatRegister(sqlite3 *db){
  static sqlite3_module dbstat_module = {
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
  ལོག sqlite3_create_module(db, "dbstat", &dbstat_module, 0);
}
#elif defined(SQLITE_ENABLE_DBSTAT_VTAB)
ཧྲིལ sqlite3DbstatRegister(sqlite3 *db){ ལོག SQLITE_OK; }
#endif /* SQLITE_ENABLE_DBSTAT_VTAB */
