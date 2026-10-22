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
#ако (defined(SQLITE_ENABLE_DBSTAT_VTAB) || defined(SQLITE_TEST)) \
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
static const char zDbstatSchema[] = 
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
typedef struct StatTable StatTable;
typedef struct StatCursor StatCursor;
typedef struct StatPage StatPage;
typedef struct StatCell StatCell;

/* Size information for a single cell within a btree page */
struct StatCell {
  цео nLocal;                     /* Bytes of local payload */
  u32 iChildPg;                   /* Child node (or 0 if this is a leaf) */
  цео nOvfl;                      /* Entries in aOvfl[] */
  u32 *aOvfl;                     /* Array of overflow page numbers */
  цео nLastOvfl;                  /* Bytes of payload on final overflow page */
  цео iOvfl;                      /* Iterates through aOvfl[] */
};

/* Size information for a single btree page */
struct StatPage {
  u32 iPgno;                      /* Page number */
  u8 *aPg;                        /* Page buffer from sqlite3_malloc() */
  цео iCell;                      /* Current cell */
  char *zPath;                    /* Path to this page */

  /* Variables populated by statDecodePage(): */
  u8 flags;                       /* Copy of flags byte */
  цео nCell;                      /* Number of cells on page */
  цео nUnused;                    /* Number of unused bytes on page */
  StatCell *aCell;                /* Array of parsed cells */
  u32 iRightChildPg;              /* Right-child page number (or 0) */
  цео nMxPayload;                 /* Largest payload of any cell on the page */
};

/* The cursor for scanning the dbstat virtual table */
struct StatCursor {
  sqlite3_vtab_cursor base;       /* base class.  MUST BE FIRST! */
  sqlite3_stmt *pStmt;            /* Iterates through set of root pages */
  u8 isEof;                       /* After pStmt has returned SQLITE_DONE */
  u8 isAgg;                       /* Aggregate results for each table */
  цео iDb;                        /* Schema used for this query */

  StatPage aPage[32];             /* Pages in path to current page */
  цео iPage;                      /* Current entry in aPage[] */

  /* Values to return. */
  u32 iPageno;                    /* Value of 'pageno' column */
  char *zName;                    /* Value of 'name' column */
  char *zPath;                    /* Value of 'path' column */
  char *zPagetype;                /* Value of 'pagetype' column */
  цео nPage;                      /* Number of pages in current btree */
  цео nCell;                      /* Value of 'ncell' column */
  цео nMxPayload;                 /* Value of 'mx_payload' column */
  i64 nUnused;                    /* Value of 'unused' column */
  i64 nPayload;                   /* Value of 'payload' column */
  i64 iOffset;                    /* Value of 'pgOffset' column */
  i64 szPage;                     /* Value of 'pgSize' column */
};

/* An instance of the DBSTAT virtual table */
struct StatTable {
  sqlite3_vtab base;              /* base class.  MUST BE FIRST! */
  sqlite3 *db;                    /* Database connection that owns this vtab */
  цео iDb;                        /* Index of database to analyze */
};

#ifndef get2byte
# define get2byte(x)   ((x)[0]<<8 | (x)[1])
#endif

/*
** Connect to or create a new DBSTAT virtual table.
*/
static цео statConnect(
  sqlite3 *db,
  void *pAux,
  цео argc, const char *const*argv,
  sqlite3_vtab **ppVtab,
  char **pzErr
){
  StatTable *pTab = 0;
  цео rc = SQLITE_OK;
  цео iDb;
  (void)pAux;

  ако( argc>=4 ){
    Token nm;
    sqlite3TokenInit(&nm, (char*)argv[3]);
    iDb = sqlite3FindDb(db, &nm);
    ако( iDb<0 ){
      *pzErr = sqlite3_mprintf("no such database: %s", argv[3]);
      врати SQLITE_ERROR;
    }
  }инако{
    iDb = 0;
  }
  sqlite3_vtab_config(db, SQLITE_VTAB_DIRECTONLY);
  rc = sqlite3_declare_vtab(db, zDbstatSchema);
  ако( rc==SQLITE_OK ){
    pTab = (StatTable *)sqlite3_malloc64(sizeof(StatTable));
    ако( pTab==0 ) rc = SQLITE_NOMEM_BKPT;
  }

  assert( rc==SQLITE_OK || pTab==0 );
  ако( rc==SQLITE_OK ){
    memset(pTab, 0, sizeof(StatTable));
    pTab->db = db;
    pTab->iDb = iDb;
  }

  *ppVtab = (sqlite3_vtab*)pTab;
  врати rc;
}

/*
** Disconnect from or destroy the DBSTAT virtual table.
*/
static цео statDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  врати SQLITE_OK;
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
static цео statBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  цео i;
  цео iSchema = -1;
  цео iName = -1;
  цео iAgg = -1;
  (void)tab;

  /* Look for a valid schema=? constraint.  If found, change the idxNum to
  ** 1 and request the value of that constraint be sent to xFilter.  And
  ** lower the cost estimate to encourage the constrained version to be
  ** used.
  */
  за(i=0; i<pIdxInfo->nConstraint; i++){
    ако( pIdxInfo->aConstraint[i].op!=SQLITE_INDEX_CONSTRAINT_EQ ) continue;
    ако( pIdxInfo->aConstraint[i].usable==0 ){
      /* Force DBSTAT table should always be the right-most table in a join */
      врати SQLITE_CONSTRAINT;
    }
    switch( pIdxInfo->aConstraint[i].iColumn ){
      case 0: {    /* name */
        iName = i;
        break;
      }
      case 10: {   /* schema */
        iSchema = i;
        break;
      }
      case 11: {   /* aggregate */
        iAgg = i;
        break;
      }
    }
  }
  i = 0;
  ако( iSchema>=0 ){
    pIdxInfo->aConstraintUsage[iSchema].argvIndex = ++i;
    pIdxInfo->aConstraintUsage[iSchema].omit = 1;
    pIdxInfo->idxNum |= 0x01;
  }
  ако( iName>=0 ){
    pIdxInfo->aConstraintUsage[iName].argvIndex = ++i;
    pIdxInfo->idxNum |= 0x02;
  }
  ако( iAgg>=0 ){
    pIdxInfo->aConstraintUsage[iAgg].argvIndex = ++i;
    pIdxInfo->idxNum |= 0x04;
  }
  pIdxInfo->estimatedCost = 1.0;

  /* Records are always returned in ascending order of (name, path). 
  ** If this will satisfy the client, set the orderByConsumed flag so that 
  ** SQLite does not do an external sort.
  */
  ако( ( pIdxInfo->nOrderBy==1
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

  врати SQLITE_OK;
}

/*
** Open a new DBSTAT cursor.
*/
static цео statOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  StatTable *pTab = (StatTable *)pVTab;
  StatCursor *pCsr;

  pCsr = (StatCursor *)sqlite3_malloc64(sizeof(StatCursor));
  ако( pCsr==0 ){
    врати SQLITE_NOMEM_BKPT;
  }инако{
    memset(pCsr, 0, sizeof(StatCursor));
    pCsr->base.pVtab = pVTab;
    pCsr->iDb = pTab->iDb;
  }

  *ppCursor = (sqlite3_vtab_cursor *)pCsr;
  врати SQLITE_OK;
}

static void statClearCells(StatPage *p){
  цео i;
  ако( p->aCell ){
    за(i=0; i<p->nCell; i++){
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
  memset(p, 0, sizeof(StatPage));
  p->aPg = aPg;
}

static void statResetCsr(StatCursor *pCsr){
  цео i;
  /* In some circumstances, specifically if an OOM has occurred, the call
  ** to sqlite3_reset() may cause the pager to be reset (emptied). It is
  ** important that statClearPage() is called to free any page refs before
  ** this happens. dbsqlfuzz 9ed3e4e3816219d3509d711636c38542bf3f40b1. */
  за(i=0; i<ArraySize(pCsr->aPage); i++){
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
static цео statClose(sqlite3_vtab_cursor *pCursor){
  StatCursor *pCsr = (StatCursor *)pCursor;
  statResetCsr(pCsr);
  sqlite3_finalize(pCsr->pStmt);
  sqlite3_free(pCsr);
  врати SQLITE_OK;
}

/*
** For a single cell on a btree page, compute the number of bytes of
** content (payload) stored on that page.  That is to say, compute the
** number of bytes of content not found on overflow pages.
*/
static цео getLocalPayload(
  цео nUsable,                    /* Usable bytes per page */
  u8 flags,                       /* Page flags */
  цео nTotal                      /* Total record (payload) size */
){
  цео nLocal;
  цео nMinLocal;
  цео nMaxLocal;
 
  ако( flags==0x0D ){              /* Table leaf node */
    nMinLocal = (nUsable - 12) * 32 / 255 - 23;
    nMaxLocal = nUsable - 35;
  }инако{                          /* Index interior and leaf nodes */
    nMinLocal = (nUsable - 12) * 32 / 255 - 23;
    nMaxLocal = (nUsable - 12) * 64 / 255 - 23;
  }

  nLocal = nMinLocal + (nTotal - nMinLocal) % (nUsable - 4);
  ако( nLocal>nMaxLocal ) nLocal = nMinLocal;
  врати nLocal;
}

/* Populate the StatPage object with information about the all
** cells found on the page currently under analysis.
*/
static цео statDecodePage(Btree *pBt, StatPage *p){
  цео nUnused;
  цео iOff;
  цео nHdr;
  цео isLeaf;
  цео szPage;

  u8 *aData = p->aPg;
  u8 *aHdr = &aData[p->iPgno==1 ? 100 : 0];

  p->flags = aHdr[0];
  ако( p->flags==0x0A || p->flags==0x0D ){
    isLeaf = 1;
    nHdr = 8;
  }инако ако( p->flags==0x05 || p->flags==0x02 ){
    isLeaf = 0;
    nHdr = 12;
  }инако{
    goto statPageIsCorrupt;
  }
  ако( p->iPgno==1 ) nHdr += 100;
  p->nCell = get2byte(&aHdr[3]);
  p->nMxPayload = 0;
  szPage = sqlite3BtreeGetPageSize(pBt);

  nUnused = get2byte(&aHdr[5]) - nHdr - 2*p->nCell;
  nUnused += (цео)aHdr[7];
  iOff = get2byte(&aHdr[1]);
  док( iOff ){
    цео iNext;
    ако( iOff>=szPage ) goto statPageIsCorrupt;
    nUnused += get2byte(&aData[iOff+2]);
    iNext = get2byte(&aData[iOff]);
    ако( iNext<iOff+4 && iNext>0 ) goto statPageIsCorrupt;
    iOff = iNext;
  }
  p->nUnused = nUnused;
  p->iRightChildPg = isLeaf ? 0 : sqlite3Get4byte(&aHdr[8]);

  ако( p->nCell ){
    цео i;                        /* Used to iterate through cells */
    цео nUsable;                  /* Usable bytes per page */

    sqlite3BtreeEnter(pBt);
    nUsable = szPage - sqlite3BtreeGetReserveNoMutex(pBt);
    sqlite3BtreeLeave(pBt);
    p->aCell = sqlite3_malloc64((p->nCell+1) * sizeof(StatCell));
    ако( p->aCell==0 ) врати SQLITE_NOMEM_BKPT;
    memset(p->aCell, 0, (p->nCell+1) * sizeof(StatCell));

    за(i=0; i<p->nCell; i++){
      StatCell *pCell = &p->aCell[i];

      iOff = get2byte(&aData[nHdr+i*2]);
      ако( iOff<nHdr || iOff>=szPage ) goto statPageIsCorrupt;
      ако( !isLeaf ){
        pCell->iChildPg = sqlite3Get4byte(&aData[iOff]);
        iOff += 4;
      }
      ако( p->flags==0x05 ){
        /* A table interior node. nPayload==0. */
      }инако{
        u32 nPayload;             /* Bytes of payload total (local+overflow) */
        цео nLocal;               /* Bytes of payload stored locally */
        iOff += getVarint32(&aData[iOff], nPayload);
        ако( p->flags==0x0D ){
          u64 dummy;
          iOff += sqlite3GetVarint(&aData[iOff], &dummy);
        }
        ако( nPayload>(u32)p->nMxPayload ) p->nMxPayload = nPayload;
        nLocal = getLocalPayload(nUsable, p->flags, nPayload);
        ако( nLocal<0 ) goto statPageIsCorrupt;
        pCell->nLocal = nLocal;
        assert( nPayload>=(u32)nLocal );
        assert( nLocal<=(nUsable-35) );
        ако( nPayload>(u32)nLocal ){
          цео j;
          цео nOvfl = ((nPayload - nLocal) + nUsable-4 - 1) / (nUsable - 4);
          ако( iOff+nLocal+4>nUsable || nPayload>0x7fffffff ){
            goto statPageIsCorrupt;
          }
          pCell->nLastOvfl = (nPayload-nLocal) - (nOvfl-1) * (nUsable-4);
          pCell->nOvfl = nOvfl;
          pCell->aOvfl = sqlite3_malloc64(sizeof(u32)*nOvfl);
          ако( pCell->aOvfl==0 ) врати SQLITE_NOMEM_BKPT;
          pCell->aOvfl[0] = sqlite3Get4byte(&aData[iOff+nLocal]);
          за(j=1; j<nOvfl; j++){
            цео rc;
            u32 iPrev = pCell->aOvfl[j-1];
            DbPage *pPg = 0;
            rc = sqlite3PagerGet(sqlite3BtreePager(pBt), iPrev, &pPg, 0);
            ако( rc!=SQLITE_OK ){
              assert( pPg==0 );
              врати rc;
            } 
            pCell->aOvfl[j] = sqlite3Get4byte(sqlite3PagerGetData(pPg));
            sqlite3PagerUnref(pPg);
          }
        }
      }
    }
  }

  врати SQLITE_OK;

statPageIsCorrupt:
  p->flags = 0;
  statClearCells(p);
  врати SQLITE_OK;
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
  ако( sqlite3OsFileControl(fd, 230440, &x)==SQLITE_OK ){
    pCsr->iOffset = x[0];
    pCsr->szPage += x[1];
  }инако{
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
static цео statGetPage(
  Btree *pBt,                     /* Load page from this b-tree */
  u32 iPg,                        /* Page number to load */
  StatPage *pPg                   /* Load page into this object */
){
  цео pgsz = sqlite3BtreeGetPageSize(pBt);
  DbPage *pDbPage = 0;
  цео rc;

  ако( pPg->aPg==0 ){
    pPg->aPg = (u8*)sqlite3_malloc(pgsz + DBSTAT_PAGE_PADDING_BYTES);
    ако( pPg->aPg==0 ){
      врати SQLITE_NOMEM_BKPT;
    }
    memset(&pPg->aPg[pgsz], 0, DBSTAT_PAGE_PADDING_BYTES);
  }

  rc = sqlite3PagerGet(sqlite3BtreePager(pBt), iPg, &pDbPage, 0);
  ако( rc==SQLITE_OK ){
    const u8 *a = sqlite3PagerGetData(pDbPage);
    memcpy(pPg->aPg, a, pgsz);
    sqlite3PagerUnref(pDbPage);
  }

  врати rc;
}

/*
** Move a DBSTAT cursor to the next entry.  Normally, the next
** entry will be the next page, but in aggregated mode (pCsr->isAgg!=0),
** the next entry is the next btree.
*/
static цео statNext(sqlite3_vtab_cursor *pCursor){
  цео rc;
  цео nPayload;
  char *z;
  StatCursor *pCsr = (StatCursor *)pCursor;
  StatTable *pTab = (StatTable *)pCursor->pVtab;
  Btree *pBt = pTab->db->aDb[pCsr->iDb].pBt;
  Pager *pPager = sqlite3BtreePager(pBt);

  sqlite3_free(pCsr->zPath);
  pCsr->zPath = 0;

statNextRestart:
  ако( pCsr->iPage<0 ){
    /* Start measuring space on the next btree */
    statResetCounts(pCsr);
    rc = sqlite3_step(pCsr->pStmt);
    ако( rc==SQLITE_ROW ){
      цео nPage;
      u32 iRoot = (u32)sqlite3_column_int64(pCsr->pStmt, 1);
      sqlite3PagerPagecount(pPager, &nPage);
      ако( nPage==0 ){
        pCsr->isEof = 1;
        врати sqlite3_reset(pCsr->pStmt);
      }
      rc = statGetPage(pBt, iRoot, &pCsr->aPage[0]);
      pCsr->aPage[0].iPgno = iRoot;
      pCsr->aPage[0].iCell = 0;
      ако( !pCsr->isAgg ){
        pCsr->aPage[0].zPath = z = sqlite3_mprintf("/");
        ако( z==0 ) rc = SQLITE_NOMEM_BKPT;
      }
      pCsr->iPage = 0;
      pCsr->nPage = 1;
    }инако{
      pCsr->isEof = 1;
      врати sqlite3_reset(pCsr->pStmt);
    }
  }инако{
    /* Continue analyzing the btree previously started */
    StatPage *p = &pCsr->aPage[pCsr->iPage];
    ако( !pCsr->isAgg ) statResetCounts(pCsr);
    док( p->iCell<p->nCell ){
      StatCell *pCell = &p->aCell[p->iCell];
      док( pCell->iOvfl<pCell->nOvfl ){
        цео nUsable;
        цео iOvfl = pCell->iOvfl;
        sqlite3BtreeEnter(pBt);
        nUsable = sqlite3BtreeGetPageSize(pBt) - 
                        sqlite3BtreeGetReserveNoMutex(pBt);
        sqlite3BtreeLeave(pBt);
        pCsr->nPage++;
        pCsr->iPageno = pCell->aOvfl[iOvfl];
        statSizeAndOffset(pCsr);
        ако( pCell->iOvfl<pCell->nOvfl-1 ){
          pCsr->nPayload += nUsable - 4;
        }инако{
          pCsr->nPayload += pCell->nLastOvfl;
          pCsr->nUnused += nUsable - 4 - pCell->nLastOvfl;
        }
        pCell->iOvfl++;
        ако( !pCsr->isAgg ){
          pCsr->zName = (char *)sqlite3_column_text(pCsr->pStmt, 0);
          pCsr->zPagetype = "overflow";
          pCsr->zPath = z = sqlite3_mprintf(
              "%s%.3x+%.6x", p->zPath, p->iCell, iOvfl
          );
          врати z==0 ? SQLITE_NOMEM_BKPT : SQLITE_OK;
        }
      }
      ако( p->iRightChildPg ) break;
      p->iCell++;
    }

    ако( !p->iRightChildPg || p->iCell>p->nCell ){
      statClearPage(p);
      pCsr->iPage--;
      ако( pCsr->isAgg && pCsr->iPage<0 ){
        /* label-statNext-done:  When computing aggregate space usage over
        ** an entire btree, this is the exit point from this function */
        врати SQLITE_OK;
      }
      goto statNextRestart; /* Tail recursion */
    }
    pCsr->iPage++;
    ако( pCsr->iPage>=ArraySize(pCsr->aPage) ){
      statResetCsr(pCsr);
      врати SQLITE_CORRUPT_BKPT;
    }
    assert( p==&pCsr->aPage[pCsr->iPage-1] );

    ако( p->iCell==p->nCell ){
      p[1].iPgno = p->iRightChildPg;
    }инако{
      p[1].iPgno = p->aCell[p->iCell].iChildPg;
    }
    rc = statGetPage(pBt, p[1].iPgno, &p[1]);
    pCsr->nPage++;
    p[1].iCell = 0;
    ако( !pCsr->isAgg ){
      p[1].zPath = z = sqlite3_mprintf("%s%.3x/", p->zPath, p->iCell);
      ако( z==0 ) rc = SQLITE_NOMEM_BKPT;
    }
    p->iCell++;
  }


  /* Populate the StatCursor fields with the values to be returned
  ** by the xColumn() and xRowid() methods.
  */
  ако( rc==SQLITE_OK ){
    цео i;
    StatPage *p = &pCsr->aPage[pCsr->iPage];
    pCsr->zName = (char *)sqlite3_column_text(pCsr->pStmt, 0);
    pCsr->iPageno = p->iPgno;

    rc = statDecodePage(pBt, p);
    ако( rc==SQLITE_OK ){
      statSizeAndOffset(pCsr);

      switch( p->flags ){
        case 0x05:             /* table internal */
        case 0x02:             /* index internal */
          pCsr->zPagetype = "internal";
          break;
        case 0x0D:             /* table leaf */
        case 0x0A:             /* index leaf */
          pCsr->zPagetype = "leaf";
          break;
        default:
          pCsr->zPagetype = "corrupted";
          break;
      }
      pCsr->nCell += p->nCell;
      pCsr->nUnused += p->nUnused;
      ако( p->nMxPayload>pCsr->nMxPayload ) pCsr->nMxPayload = p->nMxPayload;
      ако( !pCsr->isAgg ){
        pCsr->zPath = z = sqlite3_mprintf("%s", p->zPath);
        ако( z==0 ) rc = SQLITE_NOMEM_BKPT;
      }
      nPayload = 0;
      за(i=0; i<p->nCell; i++){
        nPayload += p->aCell[i].nLocal;
      }
      pCsr->nPayload += nPayload;

      /* If computing aggregate space usage by btree, continue with the
      ** next page.  The loop will exit via the return at label-statNext-done
      */
      ако( pCsr->isAgg ) goto statNextRestart;
    }
  }

  врати rc;
}

static цео statEof(sqlite3_vtab_cursor *pCursor){
  StatCursor *pCsr = (StatCursor *)pCursor;
  врати pCsr->isEof;
}

/* Initialize a cursor according to the query plan idxNum using the
** arguments in argv[0].  See statBestIndex() for a description of the
** meaning of the bits in idxNum.
*/
static цео statFilter(
  sqlite3_vtab_cursor *pCursor, 
  цео idxNum, const char *idxStr,
  цео argc, sqlite3_value **argv
){
  StatCursor *pCsr = (StatCursor *)pCursor;
  StatTable *pTab = (StatTable*)(pCursor->pVtab);
  sqlite3_str *pSql;      /* Query of btrees to analyze */
  char *zSql;             /* String value of pSql */
  цео iArg = 0;           /* Count of argv[] parameters used so far */
  цео rc = SQLITE_OK;     /* Result of this operation */
  const char *zName = 0;  /* Only provide analysis of this table */
  (void)argc;
  (void)idxStr;

  statResetCsr(pCsr);
  sqlite3_finalize(pCsr->pStmt);
  pCsr->pStmt = 0;
  ако( idxNum & 0x01 ){
    /* schema=? constraint is present.  Get its value */
    const char *zDbase = (const char*)sqlite3_value_text(argv[iArg++]);
    pCsr->iDb = sqlite3FindDbName(pTab->db, zDbase);
    ако( pCsr->iDb<0 ){
      pCsr->iDb = 0;
      pCsr->isEof = 1;
      врати SQLITE_OK;
    }
  }инако{
    pCsr->iDb = pTab->iDb;
  }
  ако( idxNum & 0x02 ){
    /* name=? constraint is present */
    zName = (const char*)sqlite3_value_text(argv[iArg++]);
  }
  ако( idxNum & 0x04 ){
    /* aggregate=? constraint is present */
    pCsr->isAgg = sqlite3_value_double(argv[iArg++])!=0.0;
  }инако{
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
  ако( zName ){
    sqlite3_str_appendf(pSql, "WHERE name=%Q", zName);
  }
  ако( idxNum & 0x08 ){
    sqlite3_str_appendf(pSql, " ORDER BY name");
  }
  zSql = sqlite3_str_finish(pSql);
  ако( zSql==0 ){
    врати SQLITE_NOMEM_BKPT;
  }инако{
    rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pStmt, 0);
    sqlite3_free(zSql);
  }

  ако( rc==SQLITE_OK ){
    pCsr->iPage = -1;
    rc = statNext(pCursor);
  }
  врати rc;
}

static цео statColumn(
  sqlite3_vtab_cursor *pCursor, 
  sqlite3_context *ctx, 
  цео i
){
  StatCursor *pCsr = (StatCursor *)pCursor;
  switch( i ){
    case 0:            /* name */
      sqlite3_result_text(ctx, pCsr->zName, -1, SQLITE_TRANSIENT);
      break;
    case 1:            /* path */
      ако( !pCsr->isAgg ){
        sqlite3_result_text(ctx, pCsr->zPath, -1, SQLITE_TRANSIENT);
      }
      break;
    case 2:            /* pageno */
      ако( pCsr->isAgg ){
        sqlite3_result_int64(ctx, pCsr->nPage);
      }инако{
        sqlite3_result_int64(ctx, pCsr->iPageno);
      }
      break;
    case 3:            /* pagetype */
      ако( !pCsr->isAgg ){
        sqlite3_result_text(ctx, pCsr->zPagetype, -1, SQLITE_STATIC);
      }
      break;
    case 4:            /* ncell */
      sqlite3_result_int64(ctx, pCsr->nCell);
      break;
    case 5:            /* payload */
      sqlite3_result_int64(ctx, pCsr->nPayload);
      break;
    case 6:            /* unused */
      sqlite3_result_int64(ctx, pCsr->nUnused);
      break;
    case 7:            /* mx_payload */
      sqlite3_result_int64(ctx, pCsr->nMxPayload);
      break;
    case 8:            /* pgoffset */
      ако( !pCsr->isAgg ){
        sqlite3_result_int64(ctx, pCsr->iOffset);
      }
      break;
    case 9:            /* pgsize */
      sqlite3_result_int64(ctx, pCsr->szPage);
      break;
    case 10: {         /* schema */
      sqlite3 *db = sqlite3_context_db_handle(ctx);
      цео iDb = pCsr->iDb;
      sqlite3_result_text(ctx, db->aDb[iDb].zDbSName, -1, SQLITE_STATIC);
      break;
    }
    default: {         /* aggregate */
      sqlite3_result_int(ctx, pCsr->isAgg);
      break;
    }
  }
  врати SQLITE_OK;
}

static цео statRowid(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid){
  StatCursor *pCsr = (StatCursor *)pCursor;
  *pRowid = pCsr->iPageno;
  врати SQLITE_OK;
}

/*
** Invoke this routine to register the "dbstat" virtual table module
*/
цео sqlite3DbstatRegister(sqlite3 *db){
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
  врати sqlite3_create_module(db, "dbstat", &dbstat_module, 0);
}
#elif defined(SQLITE_ENABLE_DBSTAT_VTAB)
цео sqlite3DbstatRegister(sqlite3 *db){ врати SQLITE_OK; }
#endif /* SQLITE_ENABLE_DBSTAT_VTAB */
