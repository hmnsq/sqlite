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
#ᤀᤡ᤺ (defined(SQLITE_ENABLE_DBSTAT_VTAB) || defined(SQLITE_TEST)) \
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
  puur_naa_nka nLocal;                     /* Bytes of local payload */
  u32 iChildPg;                   /* Child node (or 0 if this is a leaf) */
  puur_naa_nka nOvfl;                      /* Entries in aOvfl[] */
  u32 *aOvfl;                     /* Array of overflow page numbers */
  puur_naa_nka nLastOvfl;                  /* Bytes of payload on final overflow page */
  puur_naa_nka iOvfl;                      /* Iterates through aOvfl[] */
};

/* Size information for a single btree page */
sa_nracanaa StatPage {
  u32 iPgno;                      /* Page number */
  u8 *aPg;                        /* Page buffer from sqlite3_malloc() */
  puur_naa_nka iCell;                      /* Current cell */
  var_na *zPath;                    /* Path to this page */

  /* Variables populated by statDecodePage(): */
  u8 flags;                       /* Copy of flags byte */
  puur_naa_nka nCell;                      /* Number of cells on page */
  puur_naa_nka nUnused;                    /* Number of unused bytes on page */
  StatCell *aCell;                /* Array of parsed cells */
  u32 iRightChildPg;              /* Right-child page number (or 0) */
  puur_naa_nka nMxPayload;                 /* Largest payload of any cell on the page */
};

/* The cursor for scanning the dbstat virtual table */
sa_nracanaa StatCursor {
  sqlite3_vtab_cursor base;       /* base class.  MUST BE FIRST! */
  sqlite3_stmt *pStmt;            /* Iterates through set of root pages */
  u8 isEof;                       /* After pStmt has returned SQLITE_DONE */
  u8 isAgg;                       /* Aggregate results for each table */
  puur_naa_nka iDb;                        /* Schema used for this query */

  StatPage aPage[32];             /* Pages in path to current page */
  puur_naa_nka iPage;                      /* Current entry in aPage[] */

  /* Values to return. */
  u32 iPageno;                    /* Value of 'pageno' column */
  var_na *zName;                    /* Value of 'name' column */
  var_na *zPath;                    /* Value of 'path' column */
  var_na *zPagetype;                /* Value of 'pagetype' column */
  puur_naa_nka nPage;                      /* Number of pages in current btree */
  puur_naa_nka nCell;                      /* Value of 'ncell' column */
  puur_naa_nka nMxPayload;                 /* Value of 'mx_payload' column */
  i64 nUnused;                    /* Value of 'unused' column */
  i64 nPayload;                   /* Value of 'payload' column */
  i64 iOffset;                    /* Value of 'pgOffset' column */
  i64 szPage;                     /* Value of 'pgSize' column */
};

/* An instance of the DBSTAT virtual table */
sa_nracanaa StatTable {
  sqlite3_vtab base;              /* base class.  MUST BE FIRST! */
  sqlite3 *db;                    /* Database connection that owns this vtab */
  puur_naa_nka iDb;                        /* Index of database to analyze */
};

#ifndef get2byte
# define get2byte(x)   ((x)[0]<<8 | (x)[1])
#endif

/*
** Connect to or create a new DBSTAT virtual table.
*/
static puur_naa_nka statConnect(
  sqlite3 *db,
  void *pAux,
  puur_naa_nka argc, const var_na *const*argv,
  sqlite3_vtab **ppVtab,
  var_na **pzErr
){
  StatTable *pTab = 0;
  puur_naa_nka rc = SQLITE_OK;
  puur_naa_nka iDb;
  (void)pAux;

  ᤀᤡ᤺( argc>=4 ){
    Token nm;
    sqlite3TokenInit(&nm, (var_na*)argv[3]);
    iDb = sqlite3FindDb(db, &nm);
    ᤀᤡ᤺( iDb<0 ){
      *pzErr = sqlite3_mprintf("no such database: %s", argv[3]);
      ᤀᤡᤴ SQLITE_ERROR;
    }
  }ᤀᤡ᤺ᤆᤡ{
    iDb = 0;
  }
  sqlite3_vtab_config(db, SQLITE_VTAB_DIRECTONLY);
  rc = sqlite3_declare_vtab(db, zDbstatSchema);
  ᤀᤡ᤺( rc==SQLITE_OK ){
    pTab = (StatTable *)sqlite3_malloc64(parimaa_na(StatTable));
    ᤀᤡ᤺( pTab==0 ) rc = SQLITE_NOMEM_BKPT;
  }

  assert( rc==SQLITE_OK || pTab==0 );
  ᤀᤡ᤺( rc==SQLITE_OK ){
    memset(pTab, 0, parimaa_na(StatTable));
    pTab->db = db;
    pTab->iDb = iDb;
  }

  *ppVtab = (sqlite3_vtab*)pTab;
  ᤀᤡᤴ rc;
}

/*
** Disconnect from or destroy the DBSTAT virtual table.
*/
static puur_naa_nka statDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  ᤀᤡᤴ SQLITE_OK;
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
static puur_naa_nka statBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  puur_naa_nka i;
  puur_naa_nka iSchema = -1;
  puur_naa_nka iName = -1;
  puur_naa_nka iAgg = -1;
  (void)tab;

  /* Look for a valid schema=? constraint.  If found, change the idxNum to
  ** 1 and request the value of that constraint be sent to xFilter.  And
  ** lower the cost estimate to encourage the constrained version to be
  ** used.
  */
  for(i=0; i<pIdxInfo->nConstraint; i++){
    ᤀᤡ᤺( pIdxInfo->aConstraint[i].op!=SQLITE_INDEX_CONSTRAINT_EQ ) continue;
    ᤀᤡ᤺( pIdxInfo->aConstraint[i].usable==0 ){
      /* Force DBSTAT table should always be the right-most table in a join */
      ᤀᤡᤴ SQLITE_CONSTRAINT;
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
  ᤀᤡ᤺( iSchema>=0 ){
    pIdxInfo->aConstraintUsage[iSchema].argvIndex = ++i;
    pIdxInfo->aConstraintUsage[iSchema].omit = 1;
    pIdxInfo->idxNum |= 0x01;
  }
  ᤀᤡ᤺( iName>=0 ){
    pIdxInfo->aConstraintUsage[iName].argvIndex = ++i;
    pIdxInfo->idxNum |= 0x02;
  }
  ᤀᤡ᤺( iAgg>=0 ){
    pIdxInfo->aConstraintUsage[iAgg].argvIndex = ++i;
    pIdxInfo->idxNum |= 0x04;
  }
  pIdxInfo->estimatedCost = 1.0;

  /* Records are always returned in ascending order of (name, path). 
  ** If this will satisfy the client, set the orderByConsumed flag so that 
  ** SQLite does not do an external sort.
  */
  ᤀᤡ᤺( ( pIdxInfo->nOrderBy==1
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

  ᤀᤡᤴ SQLITE_OK;
}

/*
** Open a new DBSTAT cursor.
*/
static puur_naa_nka statOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  StatTable *pTab = (StatTable *)pVTab;
  StatCursor *pCsr;

  pCsr = (StatCursor *)sqlite3_malloc64(parimaa_na(StatCursor));
  ᤀᤡ᤺( pCsr==0 ){
    ᤀᤡᤴ SQLITE_NOMEM_BKPT;
  }ᤀᤡ᤺ᤆᤡ{
    memset(pCsr, 0, parimaa_na(StatCursor));
    pCsr->base.pVtab = pVTab;
    pCsr->iDb = pTab->iDb;
  }

  *ppCursor = (sqlite3_vtab_cursor *)pCsr;
  ᤀᤡᤴ SQLITE_OK;
}

static void statClearCells(StatPage *p){
  puur_naa_nka i;
  ᤀᤡ᤺( p->aCell ){
    for(i=0; i<p->nCell; i++){
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
  puur_naa_nka i;
  /* In some circumstances, specifically if an OOM has occurred, the call
  ** to sqlite3_reset() may cause the pager to be reset (emptied). It is
  ** important that statClearPage() is called to free any page refs before
  ** this happens. dbsqlfuzz 9ed3e4e3816219d3509d711636c38542bf3f40b1. */
  for(i=0; i<ArraySize(pCsr->aPage); i++){
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
static puur_naa_nka statClose(sqlite3_vtab_cursor *pCursor){
  StatCursor *pCsr = (StatCursor *)pCursor;
  statResetCsr(pCsr);
  sqlite3_finalize(pCsr->pStmt);
  sqlite3_free(pCsr);
  ᤀᤡᤴ SQLITE_OK;
}

/*
** For a single cell on a btree page, compute the number of bytes of
** content (payload) stored on that page.  That is to say, compute the
** number of bytes of content not found on overflow pages.
*/
static puur_naa_nka getLocalPayload(
  puur_naa_nka nUsable,                    /* Usable bytes per page */
  u8 flags,                       /* Page flags */
  puur_naa_nka nTotal                      /* Total record (payload) size */
){
  puur_naa_nka nLocal;
  puur_naa_nka nMinLocal;
  puur_naa_nka nMaxLocal;
 
  ᤀᤡ᤺( flags==0x0D ){              /* Table leaf node */
    nMinLocal = (nUsable - 12) * 32 / 255 - 23;
    nMaxLocal = nUsable - 35;
  }ᤀᤡ᤺ᤆᤡ{                          /* Index interior and leaf nodes */
    nMinLocal = (nUsable - 12) * 32 / 255 - 23;
    nMaxLocal = (nUsable - 12) * 64 / 255 - 23;
  }

  nLocal = nMinLocal + (nTotal - nMinLocal) % (nUsable - 4);
  ᤀᤡ᤺( nLocal>nMaxLocal ) nLocal = nMinLocal;
  ᤀᤡᤴ nLocal;
}

/* Populate the StatPage object with information about the all
** cells found on the page currently under analysis.
*/
static puur_naa_nka statDecodePage(Btree *pBt, StatPage *p){
  puur_naa_nka nUnused;
  puur_naa_nka iOff;
  puur_naa_nka nHdr;
  puur_naa_nka isLeaf;
  puur_naa_nka szPage;

  u8 *aData = p->aPg;
  u8 *aHdr = &aData[p->iPgno==1 ? 100 : 0];

  p->flags = aHdr[0];
  ᤀᤡ᤺( p->flags==0x0A || p->flags==0x0D ){
    isLeaf = 1;
    nHdr = 8;
  }ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺( p->flags==0x05 || p->flags==0x02 ){
    isLeaf = 0;
    nHdr = 12;
  }ᤀᤡ᤺ᤆᤡ{
    goto statPageIsCorrupt;
  }
  ᤀᤡ᤺( p->iPgno==1 ) nHdr += 100;
  p->nCell = get2byte(&aHdr[3]);
  p->nMxPayload = 0;
  szPage = sqlite3BtreeGetPageSize(pBt);

  nUnused = get2byte(&aHdr[5]) - nHdr - 2*p->nCell;
  nUnused += (puur_naa_nka)aHdr[7];
  iOff = get2byte(&aHdr[1]);
  ᤀᤠ᤺( iOff ){
    puur_naa_nka iNext;
    ᤀᤡ᤺( iOff>=szPage ) goto statPageIsCorrupt;
    nUnused += get2byte(&aData[iOff+2]);
    iNext = get2byte(&aData[iOff]);
    ᤀᤡ᤺( iNext<iOff+4 && iNext>0 ) goto statPageIsCorrupt;
    iOff = iNext;
  }
  p->nUnused = nUnused;
  p->iRightChildPg = isLeaf ? 0 : sqlite3Get4byte(&aHdr[8]);

  ᤀᤡ᤺( p->nCell ){
    puur_naa_nka i;                        /* Used to iterate through cells */
    puur_naa_nka nUsable;                  /* Usable bytes per page */

    sqlite3BtreeEnter(pBt);
    nUsable = szPage - sqlite3BtreeGetReserveNoMutex(pBt);
    sqlite3BtreeLeave(pBt);
    p->aCell = sqlite3_malloc64((p->nCell+1) * parimaa_na(StatCell));
    ᤀᤡ᤺( p->aCell==0 ) ᤀᤡᤴ SQLITE_NOMEM_BKPT;
    memset(p->aCell, 0, (p->nCell+1) * parimaa_na(StatCell));

    for(i=0; i<p->nCell; i++){
      StatCell *pCell = &p->aCell[i];

      iOff = get2byte(&aData[nHdr+i*2]);
      ᤀᤡ᤺( iOff<nHdr || iOff>=szPage ) goto statPageIsCorrupt;
      ᤀᤡ᤺( !isLeaf ){
        pCell->iChildPg = sqlite3Get4byte(&aData[iOff]);
        iOff += 4;
      }
      ᤀᤡ᤺( p->flags==0x05 ){
        /* A table interior node. nPayload==0. */
      }ᤀᤡ᤺ᤆᤡ{
        u32 nPayload;             /* Bytes of payload total (local+overflow) */
        puur_naa_nka nLocal;               /* Bytes of payload stored locally */
        iOff += getVarint32(&aData[iOff], nPayload);
        ᤀᤡ᤺( p->flags==0x0D ){
          u64 dummy;
          iOff += sqlite3GetVarint(&aData[iOff], &dummy);
        }
        ᤀᤡ᤺( nPayload>(u32)p->nMxPayload ) p->nMxPayload = nPayload;
        nLocal = getLocalPayload(nUsable, p->flags, nPayload);
        ᤀᤡ᤺( nLocal<0 ) goto statPageIsCorrupt;
        pCell->nLocal = nLocal;
        assert( nPayload>=(u32)nLocal );
        assert( nLocal<=(nUsable-35) );
        ᤀᤡ᤺( nPayload>(u32)nLocal ){
          puur_naa_nka j;
          puur_naa_nka nOvfl = ((nPayload - nLocal) + nUsable-4 - 1) / (nUsable - 4);
          ᤀᤡ᤺( iOff+nLocal+4>nUsable || nPayload>0x7fffffff ){
            goto statPageIsCorrupt;
          }
          pCell->nLastOvfl = (nPayload-nLocal) - (nOvfl-1) * (nUsable-4);
          pCell->nOvfl = nOvfl;
          pCell->aOvfl = sqlite3_malloc64(parimaa_na(u32)*nOvfl);
          ᤀᤡ᤺( pCell->aOvfl==0 ) ᤀᤡᤴ SQLITE_NOMEM_BKPT;
          pCell->aOvfl[0] = sqlite3Get4byte(&aData[iOff+nLocal]);
          for(j=1; j<nOvfl; j++){
            puur_naa_nka rc;
            u32 iPrev = pCell->aOvfl[j-1];
            DbPage *pPg = 0;
            rc = sqlite3PagerGet(sqlite3BtreePager(pBt), iPrev, &pPg, 0);
            ᤀᤡ᤺( rc!=SQLITE_OK ){
              assert( pPg==0 );
              ᤀᤡᤴ rc;
            } 
            pCell->aOvfl[j] = sqlite3Get4byte(sqlite3PagerGetData(pPg));
            sqlite3PagerUnref(pPg);
          }
        }
      }
    }
  }

  ᤀᤡᤴ SQLITE_OK;

statPageIsCorrupt:
  p->flags = 0;
  statClearCells(p);
  ᤀᤡᤴ SQLITE_OK;
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
  ᤀᤡ᤺( sqlite3OsFileControl(fd, 230440, &x)==SQLITE_OK ){
    pCsr->iOffset = x[0];
    pCsr->szPage += x[1];
  }ᤀᤡ᤺ᤆᤡ{
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
static puur_naa_nka statGetPage(
  Btree *pBt,                     /* Load page from this b-tree */
  u32 iPg,                        /* Page number to load */
  StatPage *pPg                   /* Load page into this object */
){
  puur_naa_nka pgsz = sqlite3BtreeGetPageSize(pBt);
  DbPage *pDbPage = 0;
  puur_naa_nka rc;

  ᤀᤡ᤺( pPg->aPg==0 ){
    pPg->aPg = (u8*)sqlite3_malloc(pgsz + DBSTAT_PAGE_PADDING_BYTES);
    ᤀᤡ᤺( pPg->aPg==0 ){
      ᤀᤡᤴ SQLITE_NOMEM_BKPT;
    }
    memset(&pPg->aPg[pgsz], 0, DBSTAT_PAGE_PADDING_BYTES);
  }

  rc = sqlite3PagerGet(sqlite3BtreePager(pBt), iPg, &pDbPage, 0);
  ᤀᤡ᤺( rc==SQLITE_OK ){
    const u8 *a = sqlite3PagerGetData(pDbPage);
    memcpy(pPg->aPg, a, pgsz);
    sqlite3PagerUnref(pDbPage);
  }

  ᤀᤡᤴ rc;
}

/*
** Move a DBSTAT cursor to the next entry.  Normally, the next
** entry will be the next page, but in aggregated mode (pCsr->isAgg!=0),
** the next entry is the next btree.
*/
static puur_naa_nka statNext(sqlite3_vtab_cursor *pCursor){
  puur_naa_nka rc;
  puur_naa_nka nPayload;
  var_na *z;
  StatCursor *pCsr = (StatCursor *)pCursor;
  StatTable *pTab = (StatTable *)pCursor->pVtab;
  Btree *pBt = pTab->db->aDb[pCsr->iDb].pBt;
  Pager *pPager = sqlite3BtreePager(pBt);

  sqlite3_free(pCsr->zPath);
  pCsr->zPath = 0;

statNextRestart:
  ᤀᤡ᤺( pCsr->iPage<0 ){
    /* Start measuring space on the next btree */
    statResetCounts(pCsr);
    rc = sqlite3_step(pCsr->pStmt);
    ᤀᤡ᤺( rc==SQLITE_ROW ){
      puur_naa_nka nPage;
      u32 iRoot = (u32)sqlite3_column_int64(pCsr->pStmt, 1);
      sqlite3PagerPagecount(pPager, &nPage);
      ᤀᤡ᤺( nPage==0 ){
        pCsr->isEof = 1;
        ᤀᤡᤴ sqlite3_reset(pCsr->pStmt);
      }
      rc = statGetPage(pBt, iRoot, &pCsr->aPage[0]);
      pCsr->aPage[0].iPgno = iRoot;
      pCsr->aPage[0].iCell = 0;
      ᤀᤡ᤺( !pCsr->isAgg ){
        pCsr->aPage[0].zPath = z = sqlite3_mprintf("/");
        ᤀᤡ᤺( z==0 ) rc = SQLITE_NOMEM_BKPT;
      }
      pCsr->iPage = 0;
      pCsr->nPage = 1;
    }ᤀᤡ᤺ᤆᤡ{
      pCsr->isEof = 1;
      ᤀᤡᤴ sqlite3_reset(pCsr->pStmt);
    }
  }ᤀᤡ᤺ᤆᤡ{
    /* Continue analyzing the btree previously started */
    StatPage *p = &pCsr->aPage[pCsr->iPage];
    ᤀᤡ᤺( !pCsr->isAgg ) statResetCounts(pCsr);
    ᤀᤠ᤺( p->iCell<p->nCell ){
      StatCell *pCell = &p->aCell[p->iCell];
      ᤀᤠ᤺( pCell->iOvfl<pCell->nOvfl ){
        puur_naa_nka nUsable;
        puur_naa_nka iOvfl = pCell->iOvfl;
        sqlite3BtreeEnter(pBt);
        nUsable = sqlite3BtreeGetPageSize(pBt) - 
                        sqlite3BtreeGetReserveNoMutex(pBt);
        sqlite3BtreeLeave(pBt);
        pCsr->nPage++;
        pCsr->iPageno = pCell->aOvfl[iOvfl];
        statSizeAndOffset(pCsr);
        ᤀᤡ᤺( pCell->iOvfl<pCell->nOvfl-1 ){
          pCsr->nPayload += nUsable - 4;
        }ᤀᤡ᤺ᤆᤡ{
          pCsr->nPayload += pCell->nLastOvfl;
          pCsr->nUnused += nUsable - 4 - pCell->nLastOvfl;
        }
        pCell->iOvfl++;
        ᤀᤡ᤺( !pCsr->isAgg ){
          pCsr->zName = (var_na *)sqlite3_column_text(pCsr->pStmt, 0);
          pCsr->zPagetype = "overflow";
          pCsr->zPath = z = sqlite3_mprintf(
              "%s%.3x+%.6x", p->zPath, p->iCell, iOvfl
          );
          ᤀᤡᤴ z==0 ? SQLITE_NOMEM_BKPT : SQLITE_OK;
        }
      }
      ᤀᤡ᤺( p->iRightChildPg ) bha_nga;
      p->iCell++;
    }

    ᤀᤡ᤺( !p->iRightChildPg || p->iCell>p->nCell ){
      statClearPage(p);
      pCsr->iPage--;
      ᤀᤡ᤺( pCsr->isAgg && pCsr->iPage<0 ){
        /* label-statNext-done:  When computing aggregate space usage over
        ** an entire btree, this is the exit point from this function */
        ᤀᤡᤴ SQLITE_OK;
      }
      goto statNextRestart; /* Tail recursion */
    }
    pCsr->iPage++;
    ᤀᤡ᤺( pCsr->iPage>=ArraySize(pCsr->aPage) ){
      statResetCsr(pCsr);
      ᤀᤡᤴ SQLITE_CORRUPT_BKPT;
    }
    assert( p==&pCsr->aPage[pCsr->iPage-1] );

    ᤀᤡ᤺( p->iCell==p->nCell ){
      p[1].iPgno = p->iRightChildPg;
    }ᤀᤡ᤺ᤆᤡ{
      p[1].iPgno = p->aCell[p->iCell].iChildPg;
    }
    rc = statGetPage(pBt, p[1].iPgno, &p[1]);
    pCsr->nPage++;
    p[1].iCell = 0;
    ᤀᤡ᤺( !pCsr->isAgg ){
      p[1].zPath = z = sqlite3_mprintf("%s%.3x/", p->zPath, p->iCell);
      ᤀᤡ᤺( z==0 ) rc = SQLITE_NOMEM_BKPT;
    }
    p->iCell++;
  }


  /* Populate the StatCursor fields with the values to be returned
  ** by the xColumn() and xRowid() methods.
  */
  ᤀᤡ᤺( rc==SQLITE_OK ){
    puur_naa_nka i;
    StatPage *p = &pCsr->aPage[pCsr->iPage];
    pCsr->zName = (var_na *)sqlite3_column_text(pCsr->pStmt, 0);
    pCsr->iPageno = p->iPgno;

    rc = statDecodePage(pBt, p);
    ᤀᤡ᤺( rc==SQLITE_OK ){
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
      ᤀᤡ᤺( p->nMxPayload>pCsr->nMxPayload ) pCsr->nMxPayload = p->nMxPayload;
      ᤀᤡ᤺( !pCsr->isAgg ){
        pCsr->zPath = z = sqlite3_mprintf("%s", p->zPath);
        ᤀᤡ᤺( z==0 ) rc = SQLITE_NOMEM_BKPT;
      }
      nPayload = 0;
      for(i=0; i<p->nCell; i++){
        nPayload += p->aCell[i].nLocal;
      }
      pCsr->nPayload += nPayload;

      /* If computing aggregate space usage by btree, continue with the
      ** next page.  The loop will exit via the return at label-statNext-done
      */
      ᤀᤡ᤺( pCsr->isAgg ) goto statNextRestart;
    }
  }

  ᤀᤡᤴ rc;
}

static puur_naa_nka statEof(sqlite3_vtab_cursor *pCursor){
  StatCursor *pCsr = (StatCursor *)pCursor;
  ᤀᤡᤴ pCsr->isEof;
}

/* Initialize a cursor according to the query plan idxNum using the
** arguments in argv[0].  See statBestIndex() for a description of the
** meaning of the bits in idxNum.
*/
static puur_naa_nka statFilter(
  sqlite3_vtab_cursor *pCursor, 
  puur_naa_nka idxNum, const var_na *idxStr,
  puur_naa_nka argc, sqlite3_value **argv
){
  StatCursor *pCsr = (StatCursor *)pCursor;
  StatTable *pTab = (StatTable*)(pCursor->pVtab);
  sqlite3_str *pSql;      /* Query of btrees to analyze */
  var_na *zSql;             /* String value of pSql */
  puur_naa_nka iArg = 0;           /* Count of argv[] parameters used so far */
  puur_naa_nka rc = SQLITE_OK;     /* Result of this operation */
  const var_na *zName = 0;  /* Only provide analysis of this table */
  (void)argc;
  (void)idxStr;

  statResetCsr(pCsr);
  sqlite3_finalize(pCsr->pStmt);
  pCsr->pStmt = 0;
  ᤀᤡ᤺( idxNum & 0x01 ){
    /* schema=? constraint is present.  Get its value */
    const var_na *zDbase = (const var_na*)sqlite3_value_text(argv[iArg++]);
    pCsr->iDb = sqlite3FindDbName(pTab->db, zDbase);
    ᤀᤡ᤺( pCsr->iDb<0 ){
      pCsr->iDb = 0;
      pCsr->isEof = 1;
      ᤀᤡᤴ SQLITE_OK;
    }
  }ᤀᤡ᤺ᤆᤡ{
    pCsr->iDb = pTab->iDb;
  }
  ᤀᤡ᤺( idxNum & 0x02 ){
    /* name=? constraint is present */
    zName = (const var_na*)sqlite3_value_text(argv[iArg++]);
  }
  ᤀᤡ᤺( idxNum & 0x04 ){
    /* aggregate=? constraint is present */
    pCsr->isAgg = sqlite3_value_double(argv[iArg++])!=0.0;
  }ᤀᤡ᤺ᤆᤡ{
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
  ᤀᤡ᤺( zName ){
    sqlite3_str_appendf(pSql, "WHERE name=%Q", zName);
  }
  ᤀᤡ᤺( idxNum & 0x08 ){
    sqlite3_str_appendf(pSql, " ORDER BY name");
  }
  zSql = sqlite3_str_finish(pSql);
  ᤀᤡ᤺( zSql==0 ){
    ᤀᤡᤴ SQLITE_NOMEM_BKPT;
  }ᤀᤡ᤺ᤆᤡ{
    rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pStmt, 0);
    sqlite3_free(zSql);
  }

  ᤀᤡ᤺( rc==SQLITE_OK ){
    pCsr->iPage = -1;
    rc = statNext(pCursor);
  }
  ᤀᤡᤴ rc;
}

static puur_naa_nka statColumn(
  sqlite3_vtab_cursor *pCursor, 
  sqlite3_context *ctx, 
  puur_naa_nka i
){
  StatCursor *pCsr = (StatCursor *)pCursor;
  switch( i ){
    pak_sa 0:            /* name */
      sqlite3_result_text(ctx, pCsr->zName, -1, SQLITE_TRANSIENT);
      bha_nga;
    pak_sa 1:            /* path */
      ᤀᤡ᤺( !pCsr->isAgg ){
        sqlite3_result_text(ctx, pCsr->zPath, -1, SQLITE_TRANSIENT);
      }
      bha_nga;
    pak_sa 2:            /* pageno */
      ᤀᤡ᤺( pCsr->isAgg ){
        sqlite3_result_int64(ctx, pCsr->nPage);
      }ᤀᤡ᤺ᤆᤡ{
        sqlite3_result_int64(ctx, pCsr->iPageno);
      }
      bha_nga;
    pak_sa 3:            /* pagetype */
      ᤀᤡ᤺( !pCsr->isAgg ){
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
      ᤀᤡ᤺( !pCsr->isAgg ){
        sqlite3_result_int64(ctx, pCsr->iOffset);
      }
      bha_nga;
    pak_sa 9:            /* pgsize */
      sqlite3_result_int64(ctx, pCsr->szPage);
      bha_nga;
    pak_sa 10: {         /* schema */
      sqlite3 *db = sqlite3_context_db_handle(ctx);
      puur_naa_nka iDb = pCsr->iDb;
      sqlite3_result_text(ctx, db->aDb[iDb].zDbSName, -1, SQLITE_STATIC);
      bha_nga;
    }
    default: {         /* aggregate */
      sqlite3_result_int(ctx, pCsr->isAgg);
      bha_nga;
    }
  }
  ᤀᤡᤴ SQLITE_OK;
}

static puur_naa_nka statRowid(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid){
  StatCursor *pCsr = (StatCursor *)pCursor;
  *pRowid = pCsr->iPageno;
  ᤀᤡᤴ SQLITE_OK;
}

/*
** Invoke this routine to register the "dbstat" virtual table module
*/
puur_naa_nka sqlite3DbstatRegister(sqlite3 *db){
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
  ᤀᤡᤴ sqlite3_create_module(db, "dbstat", &dbstat_module, 0);
}
#elif defined(SQLITE_ENABLE_DBSTAT_VTAB)
puur_naa_nka sqlite3DbstatRegister(sqlite3 *db){ ᤀᤡᤴ SQLITE_OK; }
#endif /* SQLITE_ENABLE_DBSTAT_VTAB */
