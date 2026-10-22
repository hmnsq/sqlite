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

#ಅನ್ತರ್ಭಾವ "sqliteInt.h"   /* Requires access to internal data structures */
#ಯದಿ (defined(SQLITE_ENABLE_DBSTAT_VTAB) || defined(SQLITE_TEST)) \
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
ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ zDbstatSchema[] = 
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
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ StatTable StatTable;
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ StatCursor StatCursor;
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ StatPage StatPage;
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ StatCell StatCell;

/* Size information for a single cell within a btree page */
ಸಂರಚನಾ StatCell {
  ಪೂರ್ಣಾಙ್ಕ nLocal;                     /* Bytes of local payload */
  u32 iChildPg;                   /* Child node (or 0 if this is a leaf) */
  ಪೂರ್ಣಾಙ್ಕ nOvfl;                      /* Entries in aOvfl[] */
  u32 *aOvfl;                     /* Array of overflow page numbers */
  ಪೂರ್ಣಾಙ್ಕ nLastOvfl;                  /* Bytes of payload on final overflow page */
  ಪೂರ್ಣಾಙ್ಕ iOvfl;                      /* Iterates through aOvfl[] */
};

/* Size information for a single btree page */
ಸಂರಚನಾ StatPage {
  u32 iPgno;                      /* Page number */
  u8 *aPg;                        /* Page buffer from sqlite3_malloc() */
  ಪೂರ್ಣಾಙ್ಕ iCell;                      /* Current cell */
  ವರ್ಣ *zPath;                    /* Path to this page */

  /* Variables populated by statDecodePage(): */
  u8 flags;                       /* Copy of flags byte */
  ಪೂರ್ಣಾಙ್ಕ nCell;                      /* Number of cells on page */
  ಪೂರ್ಣಾಙ್ಕ nUnused;                    /* Number of unused bytes on page */
  StatCell *aCell;                /* Array of parsed cells */
  u32 i_ಬಲ_Child_Pg;              /* Right-child page number (or 0) */
  ಪೂರ್ಣಾಙ್ಕ nMxPayload;                 /* Largest payload of any cell on the page */
};

/* The cursor for scanning the dbstat virtual table */
ಸಂರಚನಾ StatCursor {
  sqlite3_vtab_cursor base;       /* base class.  MUST BE FIRST! */
  sqlite3_stmt *pStmt;            /* Iterates through set of root pages */
  u8 isEof;                       /* After pStmt has returned SQLITE_DONE */
  u8 isAgg;                       /* Aggregate results for each table */
  ಪೂರ್ಣಾಙ್ಕ iDb;                        /* Schema used for this query */

  StatPage aPage[32];             /* Pages in path to current page */
  ಪೂರ್ಣಾಙ್ಕ iPage;                      /* Current entry in aPage[] */

  /* Values to return. */
  u32 iPageno;                    /* Value of 'pageno' column */
  ವರ್ಣ *zName;                    /* Value of 'name' column */
  ವರ್ಣ *zPath;                    /* Value of 'path' column */
  ವರ್ಣ *zPagetype;                /* Value of 'pagetype' column */
  ಪೂರ್ಣಾಙ್ಕ nPage;                      /* Number of pages in current btree */
  ಪೂರ್ಣಾಙ್ಕ nCell;                      /* Value of 'ncell' column */
  ಪೂರ್ಣಾಙ್ಕ nMxPayload;                 /* Value of 'mx_payload' column */
  i64 nUnused;                    /* Value of 'unused' column */
  i64 nPayload;                   /* Value of 'payload' column */
  i64 iOffset;                    /* Value of 'pgOffset' column */
  i64 szPage;                     /* Value of 'pgSize' column */
};

/* An instance of the DBSTAT virtual table */
ಸಂರಚನಾ StatTable {
  sqlite3_vtab base;              /* base class.  MUST BE FIRST! */
  sqlite3 *db;                    /* Database connection that owns this vtab */
  ಪೂರ್ಣಾಙ್ಕ iDb;                        /* Index of database to analyze */
};

#ifndef get2byte
# define get2byte(x)   ((x)[0]<<8 | (x)[1])
#endif

/*
** Connect to or create a new DBSTAT virtual table.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ statConnect(
  sqlite3 *db,
  ಶೂನ್ಯ *pAux,
  ಪೂರ್ಣಾಙ್ಕ argc, ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ*argv,
  sqlite3_vtab **ppVtab,
  ವರ್ಣ **pzErr
){
  StatTable *pTab = 0;
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;
  ಪೂರ್ಣಾಙ್ಕ iDb;
  (ಶೂನ್ಯ)pAux;

  ಯದಿ( argc>=4 ){
    Token nm;
    sqlite3TokenInit(&nm, (ವರ್ಣ*)argv[3]);
    iDb = sqlite3FindDb(db, &nm);
    ಯದಿ( iDb<0 ){
      *pzErr = sqlite3_mprintf("no such database: %s", argv[3]);
      ಪ್ರತ್ಯಾಗಮ SQLITE_ERROR;
    }
  }ಅನ್ಯಥಾ{
    iDb = 0;
  }
  sqlite3_vtab_config(db, SQLITE_VTAB_DIRECTONLY);
  rc = sqlite3_declare_vtab(db, zDbstatSchema);
  ಯದಿ( rc==SQLITE_OK ){
    pTab = (StatTable *)sqlite3_malloc64(ಪರಿಮಾಣ(StatTable));
    ಯದಿ( pTab==0 ) rc = SQLITE_NOMEM_BKPT;
  }

  ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_OK || pTab==0 );
  ಯದಿ( rc==SQLITE_OK ){
    memset(pTab, 0, ಪರಿಮಾಣ(StatTable));
    pTab->db = db;
    pTab->iDb = iDb;
  }

  *ppVtab = (sqlite3_vtab*)pTab;
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Disconnect from or destroy the DBSTAT virtual table.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ statDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ statBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪೂರ್ಣಾಙ್ಕ iSchema = -1;
  ಪೂರ್ಣಾಙ್ಕ iName = -1;
  ಪೂರ್ಣಾಙ್ಕ iAgg = -1;
  (ಶೂನ್ಯ)tab;

  /* Look for a valid schema=? constraint.  If found, change the idxNum to
  ** 1 and request the value of that constraint be sent to xFilter.  And
  ** lower the cost estimate to encourage the constrained version to be
  ** used.
  */
  ಪ್ರತಿ(i=0; i<pIdxInfo->nConstraint; i++){
    ಯದಿ( pIdxInfo->aConstraint[i].op!=SQLITE_INDEX_CONSTRAINT_EQ ) ಅನುವರ್ತ;
    ಯದಿ( pIdxInfo->aConstraint[i].usable==0 ){
      /* Force DBSTAT table should always be the right-most table in a join */
      ಪ್ರತ್ಯಾಗಮ SQLITE_CONSTRAINT;
    }
    ವಿಕಲ್ಪ( pIdxInfo->aConstraint[i].iColumn ){
      ಪಕ್ಷ 0: {    /* name */
        iName = i;
        ಭಙ್ಗ;
      }
      ಪಕ್ಷ 10: {   /* schema */
        iSchema = i;
        ಭಙ್ಗ;
      }
      ಪಕ್ಷ 11: {   /* aggregate */
        iAgg = i;
        ಭಙ್ಗ;
      }
    }
  }
  i = 0;
  ಯದಿ( iSchema>=0 ){
    pIdxInfo->aConstraintUsage[iSchema].argvIndex = ++i;
    pIdxInfo->aConstraintUsage[iSchema].omit = 1;
    pIdxInfo->idx_ಅಂಕೆ |= 0x01;
  }
  ಯದಿ( iName>=0 ){
    pIdxInfo->aConstraintUsage[iName].argvIndex = ++i;
    pIdxInfo->idx_ಅಂಕೆ |= 0x02;
  }
  ಯದಿ( iAgg>=0 ){
    pIdxInfo->aConstraintUsage[iAgg].argvIndex = ++i;
    pIdxInfo->idx_ಅಂಕೆ |= 0x04;
  }
  pIdxInfo->estimatedCost = 1.0;

  /* Records are always returned in ascending order of (name, path). 
  ** If this will satisfy the client, set the orderByConsumed flag so that 
  ** SQLite does not do an external sort.
  */
  ಯದಿ( ( pIdxInfo->nOrderBy==1
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
    pIdxInfo->idx_ಅಂಕೆ |= 0x08;
  }
  pIdxInfo->idxFlags |= SQLITE_INDEX_SCAN_HEX;

  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Open a new DBSTAT cursor.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ statOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  StatTable *pTab = (StatTable *)pVTab;
  StatCursor *pCsr;

  pCsr = (StatCursor *)sqlite3_malloc64(ಪರಿಮಾಣ(StatCursor));
  ಯದಿ( pCsr==0 ){
    ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM_BKPT;
  }ಅನ್ಯಥಾ{
    memset(pCsr, 0, ಪರಿಮಾಣ(StatCursor));
    pCsr->base.pVtab = pVTab;
    pCsr->iDb = pTab->iDb;
  }

  *ppCursor = (sqlite3_vtab_cursor *)pCsr;
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

ಸ್ಥಿರ ಶೂನ್ಯ statClearCells(StatPage *p){
  ಪೂರ್ಣಾಙ್ಕ i;
  ಯದಿ( p->aCell ){
    ಪ್ರತಿ(i=0; i<p->nCell; i++){
      sqlite3_free(p->aCell[i].aOvfl);
    }
    sqlite3_free(p->aCell);
  }
  p->nCell = 0;
  p->aCell = 0;
}

ಸ್ಥಿರ ಶೂನ್ಯ statClearPage(StatPage *p){
  u8 *aPg = p->aPg;
  statClearCells(p);
  sqlite3_free(p->zPath);
  memset(p, 0, ಪರಿಮಾಣ(StatPage));
  p->aPg = aPg;
}

ಸ್ಥಿರ ಶೂನ್ಯ statResetCsr(StatCursor *pCsr){
  ಪೂರ್ಣಾಙ್ಕ i;
  /* In some circumstances, specifically if an OOM has occurred, the call
  ** to sqlite3_reset() may cause the pager to be reset (emptied). It is
  ** important that statClearPage() is called to free any page refs before
  ** this happens. dbsqlfuzz 9ed3e4e3816219d3509d711636c38542bf3f40b1. */
  ಪ್ರತಿ(i=0; i<ArraySize(pCsr->aPage); i++){
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
ಸ್ಥಿರ ಶೂನ್ಯ statResetCounts(StatCursor *pCsr){
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ statClose(sqlite3_vtab_cursor *pCursor){
  StatCursor *pCsr = (StatCursor *)pCursor;
  statResetCsr(pCsr);
  sqlite3_finalize(pCsr->pStmt);
  sqlite3_free(pCsr);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** For a single cell on a btree page, compute the number of bytes of
** content (payload) stored on that page.  That is to say, compute the
** number of bytes of content not found on overflow pages.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ getLocalPayload(
  ಪೂರ್ಣಾಙ್ಕ nUsable,                    /* Usable bytes per page */
  u8 flags,                       /* Page flags */
  ಪೂರ್ಣಾಙ್ಕ nTotal                      /* Total record (payload) size */
){
  ಪೂರ್ಣಾಙ್ಕ nLocal;
  ಪೂರ್ಣಾಙ್ಕ nMinLocal;
  ಪೂರ್ಣಾಙ್ಕ nMaxLocal;
 
  ಯದಿ( flags==0x0D ){              /* Table leaf node */
    nMinLocal = (nUsable - 12) * 32 / 255 - 23;
    nMaxLocal = nUsable - 35;
  }ಅನ್ಯಥಾ{                          /* Index interior and leaf nodes */
    nMinLocal = (nUsable - 12) * 32 / 255 - 23;
    nMaxLocal = (nUsable - 12) * 64 / 255 - 23;
  }

  nLocal = nMinLocal + (nTotal - nMinLocal) % (nUsable - 4);
  ಯದಿ( nLocal>nMaxLocal ) nLocal = nMinLocal;
  ಪ್ರತ್ಯಾಗಮ nLocal;
}

/* Populate the StatPage object with information about the all
** cells found on the page currently under analysis.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ statDecodePage(Btree *pBt, StatPage *p){
  ಪೂರ್ಣಾಙ್ಕ nUnused;
  ಪೂರ್ಣಾಙ್ಕ iOff;
  ಪೂರ್ಣಾಙ್ಕ nHdr;
  ಪೂರ್ಣಾಙ್ಕ isLeaf;
  ಪೂರ್ಣಾಙ್ಕ szPage;

  u8 *aData = p->aPg;
  u8 *aHdr = &aData[p->iPgno==1 ? 100 : 0];

  p->flags = aHdr[0];
  ಯದಿ( p->flags==0x0A || p->flags==0x0D ){
    isLeaf = 1;
    nHdr = 8;
  }ಅನ್ಯಥಾ ಯದಿ( p->flags==0x05 || p->flags==0x02 ){
    isLeaf = 0;
    nHdr = 12;
  }ಅನ್ಯಥಾ{
    ಗಚ್ಛ statPageIsCorrupt;
  }
  ಯದಿ( p->iPgno==1 ) nHdr += 100;
  p->nCell = get2byte(&aHdr[3]);
  p->nMxPayload = 0;
  szPage = sqlite3BtreeGetPageSize(pBt);

  nUnused = get2byte(&aHdr[5]) - nHdr - 2*p->nCell;
  nUnused += (ಪೂರ್ಣಾಙ್ಕ)aHdr[7];
  iOff = get2byte(&aHdr[1]);
  ಯಾವತ್( iOff ){
    ಪೂರ್ಣಾಙ್ಕ iNext;
    ಯದಿ( iOff>=szPage ) ಗಚ್ಛ statPageIsCorrupt;
    nUnused += get2byte(&aData[iOff+2]);
    iNext = get2byte(&aData[iOff]);
    ಯದಿ( iNext<iOff+4 && iNext>0 ) ಗಚ್ಛ statPageIsCorrupt;
    iOff = iNext;
  }
  p->nUnused = nUnused;
  p->i_ಬಲ_Child_Pg = isLeaf ? 0 : sqlite3Get4byte(&aHdr[8]);

  ಯದಿ( p->nCell ){
    ಪೂರ್ಣಾಙ್ಕ i;                        /* Used to iterate through cells */
    ಪೂರ್ಣಾಙ್ಕ nUsable;                  /* Usable bytes per page */

    sqlite3BtreeEnter(pBt);
    nUsable = szPage - sqlite3BtreeGetReserveNoMutex(pBt);
    sqlite3BtreeLeave(pBt);
    p->aCell = sqlite3_malloc64((p->nCell+1) * ಪರಿಮಾಣ(StatCell));
    ಯದಿ( p->aCell==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM_BKPT;
    memset(p->aCell, 0, (p->nCell+1) * ಪರಿಮಾಣ(StatCell));

    ಪ್ರತಿ(i=0; i<p->nCell; i++){
      StatCell *pCell = &p->aCell[i];

      iOff = get2byte(&aData[nHdr+i*2]);
      ಯದಿ( iOff<nHdr || iOff>=szPage ) ಗಚ್ಛ statPageIsCorrupt;
      ಯದಿ( !isLeaf ){
        pCell->iChildPg = sqlite3Get4byte(&aData[iOff]);
        iOff += 4;
      }
      ಯದಿ( p->flags==0x05 ){
        /* A table interior node. nPayload==0. */
      }ಅನ್ಯಥಾ{
        u32 nPayload;             /* Bytes of payload total (local+overflow) */
        ಪೂರ್ಣಾಙ್ಕ nLocal;               /* Bytes of payload stored locally */
        iOff += getVarint32(&aData[iOff], nPayload);
        ಯದಿ( p->flags==0x0D ){
          u64 dummy;
          iOff += sqlite3GetVarint(&aData[iOff], &dummy);
        }
        ಯದಿ( nPayload>(u32)p->nMxPayload ) p->nMxPayload = nPayload;
        nLocal = getLocalPayload(nUsable, p->flags, nPayload);
        ಯದಿ( nLocal<0 ) ಗಚ್ಛ statPageIsCorrupt;
        pCell->nLocal = nLocal;
        ಪ್ರತಿಜ್ಞಾ( nPayload>=(u32)nLocal );
        ಪ್ರತಿಜ್ಞಾ( nLocal<=(nUsable-35) );
        ಯದಿ( nPayload>(u32)nLocal ){
          ಪೂರ್ಣಾಙ್ಕ j;
          ಪೂರ್ಣಾಙ್ಕ nOvfl = ((nPayload - nLocal) + nUsable-4 - 1) / (nUsable - 4);
          ಯದಿ( iOff+nLocal+4>nUsable || nPayload>0x7fffffff ){
            ಗಚ್ಛ statPageIsCorrupt;
          }
          pCell->nLastOvfl = (nPayload-nLocal) - (nOvfl-1) * (nUsable-4);
          pCell->nOvfl = nOvfl;
          pCell->aOvfl = sqlite3_malloc64(ಪರಿಮಾಣ(u32)*nOvfl);
          ಯದಿ( pCell->aOvfl==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM_BKPT;
          pCell->aOvfl[0] = sqlite3Get4byte(&aData[iOff+nLocal]);
          ಪ್ರತಿ(j=1; j<nOvfl; j++){
            ಪೂರ್ಣಾಙ್ಕ rc;
            u32 iPrev = pCell->aOvfl[j-1];
            DbPage *pPg = 0;
            rc = sqlite3PagerGet(sqlite3BtreePager(pBt), iPrev, &pPg, 0);
            ಯದಿ( rc!=SQLITE_OK ){
              ಪ್ರತಿಜ್ಞಾ( pPg==0 );
              ಪ್ರತ್ಯಾಗಮ rc;
            } 
            pCell->aOvfl[j] = sqlite3Get4byte(sqlite3PagerGetData(pPg));
            sqlite3PagerUnref(pPg);
          }
        }
      }
    }
  }

  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;

statPageIsCorrupt:
  p->flags = 0;
  statClearCells(p);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Populate the pCsr->iOffset and pCsr->szPage member variables. Based on
** the current value of pCsr->iPageno.
*/
ಸ್ಥಿರ ಶೂನ್ಯ statSizeAndOffset(StatCursor *pCsr){
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
  ಯದಿ( sqlite3OsFileControl(fd, 230440, &x)==SQLITE_OK ){
    pCsr->iOffset = x[0];
    pCsr->szPage += x[1];
  }ಅನ್ಯಥಾ{
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ statGetPage(
  Btree *pBt,                     /* Load page from this b-tree */
  u32 iPg,                        /* Page number to load */
  StatPage *pPg                   /* Load page into this object */
){
  ಪೂರ್ಣಾಙ್ಕ pgsz = sqlite3BtreeGetPageSize(pBt);
  DbPage *pDbPage = 0;
  ಪೂರ್ಣಾಙ್ಕ rc;

  ಯದಿ( pPg->aPg==0 ){
    pPg->aPg = (u8*)sqlite3_malloc(pgsz + DBSTAT_PAGE_PADDING_BYTES);
    ಯದಿ( pPg->aPg==0 ){
      ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM_BKPT;
    }
    memset(&pPg->aPg[pgsz], 0, DBSTAT_PAGE_PADDING_BYTES);
  }

  rc = sqlite3PagerGet(sqlite3BtreePager(pBt), iPg, &pDbPage, 0);
  ಯದಿ( rc==SQLITE_OK ){
    ಧ್ರುವ u8 *a = sqlite3PagerGetData(pDbPage);
    memcpy(pPg->aPg, a, pgsz);
    sqlite3PagerUnref(pDbPage);
  }

  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Move a DBSTAT cursor to the next entry.  Normally, the next
** entry will be the next page, but in aggregated mode (pCsr->isAgg!=0),
** the next entry is the next btree.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ statNext(sqlite3_vtab_cursor *pCursor){
  ಪೂರ್ಣಾಙ್ಕ rc;
  ಪೂರ್ಣಾಙ್ಕ nPayload;
  ವರ್ಣ *z;
  StatCursor *pCsr = (StatCursor *)pCursor;
  StatTable *pTab = (StatTable *)pCursor->pVtab;
  Btree *pBt = pTab->db->aDb[pCsr->iDb].pBt;
  Pager *pPager = sqlite3BtreePager(pBt);

  sqlite3_free(pCsr->zPath);
  pCsr->zPath = 0;

statNextRestart:
  ಯದಿ( pCsr->iPage<0 ){
    /* Start measuring space on the next btree */
    statResetCounts(pCsr);
    rc = sqlite3_step(pCsr->pStmt);
    ಯದಿ( rc==SQLITE_ROW ){
      ಪೂರ್ಣಾಙ್ಕ nPage;
      u32 i_ಬೇರು = (u32)sqlite3_column_int64(pCsr->pStmt, 1);
      sqlite3PagerPagecount(pPager, &nPage);
      ಯದಿ( nPage==0 ){
        pCsr->isEof = 1;
        ಪ್ರತ್ಯಾಗಮ sqlite3_reset(pCsr->pStmt);
      }
      rc = statGetPage(pBt, i_ಬೇರು, &pCsr->aPage[0]);
      pCsr->aPage[0].iPgno = i_ಬೇರು;
      pCsr->aPage[0].iCell = 0;
      ಯದಿ( !pCsr->isAgg ){
        pCsr->aPage[0].zPath = z = sqlite3_mprintf("/");
        ಯದಿ( z==0 ) rc = SQLITE_NOMEM_BKPT;
      }
      pCsr->iPage = 0;
      pCsr->nPage = 1;
    }ಅನ್ಯಥಾ{
      pCsr->isEof = 1;
      ಪ್ರತ್ಯಾಗಮ sqlite3_reset(pCsr->pStmt);
    }
  }ಅನ್ಯಥಾ{
    /* Continue analyzing the btree previously started */
    StatPage *p = &pCsr->aPage[pCsr->iPage];
    ಯದಿ( !pCsr->isAgg ) statResetCounts(pCsr);
    ಯಾವತ್( p->iCell<p->nCell ){
      StatCell *pCell = &p->aCell[p->iCell];
      ಯಾವತ್( pCell->iOvfl<pCell->nOvfl ){
        ಪೂರ್ಣಾಙ್ಕ nUsable;
        ಪೂರ್ಣಾಙ್ಕ iOvfl = pCell->iOvfl;
        sqlite3BtreeEnter(pBt);
        nUsable = sqlite3BtreeGetPageSize(pBt) - 
                        sqlite3BtreeGetReserveNoMutex(pBt);
        sqlite3BtreeLeave(pBt);
        pCsr->nPage++;
        pCsr->iPageno = pCell->aOvfl[iOvfl];
        statSizeAndOffset(pCsr);
        ಯದಿ( pCell->iOvfl<pCell->nOvfl-1 ){
          pCsr->nPayload += nUsable - 4;
        }ಅನ್ಯಥಾ{
          pCsr->nPayload += pCell->nLastOvfl;
          pCsr->nUnused += nUsable - 4 - pCell->nLastOvfl;
        }
        pCell->iOvfl++;
        ಯದಿ( !pCsr->isAgg ){
          pCsr->zName = (ವರ್ಣ *)sqlite3_column_text(pCsr->pStmt, 0);
          pCsr->zPagetype = "overflow";
          pCsr->zPath = z = sqlite3_mprintf(
              "%s%.3x+%.6x", p->zPath, p->iCell, iOvfl
          );
          ಪ್ರತ್ಯಾಗಮ z==0 ? SQLITE_NOMEM_BKPT : SQLITE_OK;
        }
      }
      ಯದಿ( p->i_ಬಲ_Child_Pg ) ಭಙ್ಗ;
      p->iCell++;
    }

    ಯದಿ( !p->i_ಬಲ_Child_Pg || p->iCell>p->nCell ){
      statClearPage(p);
      pCsr->iPage--;
      ಯದಿ( pCsr->isAgg && pCsr->iPage<0 ){
        /* label-statNext-done:  When computing aggregate space usage over
        ** an entire btree, this is the exit point from this function */
        ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
      }
      ಗಚ್ಛ statNextRestart; /* Tail recursion */
    }
    pCsr->iPage++;
    ಯದಿ( pCsr->iPage>=ArraySize(pCsr->aPage) ){
      statResetCsr(pCsr);
      ಪ್ರತ್ಯಾಗಮ SQLITE_CORRUPT_BKPT;
    }
    ಪ್ರತಿಜ್ಞಾ( p==&pCsr->aPage[pCsr->iPage-1] );

    ಯದಿ( p->iCell==p->nCell ){
      p[1].iPgno = p->i_ಬಲ_Child_Pg;
    }ಅನ್ಯಥಾ{
      p[1].iPgno = p->aCell[p->iCell].iChildPg;
    }
    rc = statGetPage(pBt, p[1].iPgno, &p[1]);
    pCsr->nPage++;
    p[1].iCell = 0;
    ಯದಿ( !pCsr->isAgg ){
      p[1].zPath = z = sqlite3_mprintf("%s%.3x/", p->zPath, p->iCell);
      ಯದಿ( z==0 ) rc = SQLITE_NOMEM_BKPT;
    }
    p->iCell++;
  }


  /* Populate the StatCursor fields with the values to be returned
  ** by the xColumn() and xRowid() methods.
  */
  ಯದಿ( rc==SQLITE_OK ){
    ಪೂರ್ಣಾಙ್ಕ i;
    StatPage *p = &pCsr->aPage[pCsr->iPage];
    pCsr->zName = (ವರ್ಣ *)sqlite3_column_text(pCsr->pStmt, 0);
    pCsr->iPageno = p->iPgno;

    rc = statDecodePage(pBt, p);
    ಯದಿ( rc==SQLITE_OK ){
      statSizeAndOffset(pCsr);

      ವಿಕಲ್ಪ( p->flags ){
        ಪಕ್ಷ 0x05:             /* table internal */
        ಪಕ್ಷ 0x02:             /* index internal */
          pCsr->zPagetype = "internal";
          ಭಙ್ಗ;
        ಪಕ್ಷ 0x0D:             /* table leaf */
        ಪಕ್ಷ 0x0A:             /* index leaf */
          pCsr->zPagetype = "leaf";
          ಭಙ್ಗ;
        ಸ್ವಭಾವ:
          pCsr->zPagetype = "corrupted";
          ಭಙ್ಗ;
      }
      pCsr->nCell += p->nCell;
      pCsr->nUnused += p->nUnused;
      ಯದಿ( p->nMxPayload>pCsr->nMxPayload ) pCsr->nMxPayload = p->nMxPayload;
      ಯದಿ( !pCsr->isAgg ){
        pCsr->zPath = z = sqlite3_mprintf("%s", p->zPath);
        ಯದಿ( z==0 ) rc = SQLITE_NOMEM_BKPT;
      }
      nPayload = 0;
      ಪ್ರತಿ(i=0; i<p->nCell; i++){
        nPayload += p->aCell[i].nLocal;
      }
      pCsr->nPayload += nPayload;

      /* If computing aggregate space usage by btree, continue with the
      ** next page.  The loop will exit via the return at label-statNext-done
      */
      ಯದಿ( pCsr->isAgg ) ಗಚ್ಛ statNextRestart;
    }
  }

  ಪ್ರತ್ಯಾಗಮ rc;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ statEof(sqlite3_vtab_cursor *pCursor){
  StatCursor *pCsr = (StatCursor *)pCursor;
  ಪ್ರತ್ಯಾಗಮ pCsr->isEof;
}

/* Initialize a cursor according to the query plan idxNum using the
** arguments in argv[0].  See statBestIndex() for a description of the
** meaning of the bits in idxNum.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ statFilter(
  sqlite3_vtab_cursor *pCursor, 
  ಪೂರ್ಣಾಙ್ಕ idx_ಅಂಕೆ, ಧ್ರುವ ವರ್ಣ *idxStr,
  ಪೂರ್ಣಾಙ್ಕ argc, sqlite3_ಮೌಲ್ಯ **argv
){
  StatCursor *pCsr = (StatCursor *)pCursor;
  StatTable *pTab = (StatTable*)(pCursor->pVtab);
  sqlite3_str *pSql;      /* Query of btrees to analyze */
  ವರ್ಣ *zSql;             /* String value of pSql */
  ಪೂರ್ಣಾಙ್ಕ iArg = 0;           /* Count of argv[] parameters used so far */
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;     /* Result of this operation */
  ಧ್ರುವ ವರ್ಣ *zName = 0;  /* Only provide analysis of this table */
  (ಶೂನ್ಯ)argc;
  (ಶೂನ್ಯ)idxStr;

  statResetCsr(pCsr);
  sqlite3_finalize(pCsr->pStmt);
  pCsr->pStmt = 0;
  ಯದಿ( idx_ಅಂಕೆ & 0x01 ){
    /* schema=? constraint is present.  Get its value */
    ಧ್ರುವ ವರ್ಣ *zDbase = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[iArg++]);
    pCsr->iDb = sqlite3FindDbName(pTab->db, zDbase);
    ಯದಿ( pCsr->iDb<0 ){
      pCsr->iDb = 0;
      pCsr->isEof = 1;
      ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
    }
  }ಅನ್ಯಥಾ{
    pCsr->iDb = pTab->iDb;
  }
  ಯದಿ( idx_ಅಂಕೆ & 0x02 ){
    /* name=? constraint is present */
    zName = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[iArg++]);
  }
  ಯದಿ( idx_ಅಂಕೆ & 0x04 ){
    /* aggregate=? constraint is present */
    pCsr->isAgg = sqlite3_ಮೌಲ್ಯ_double(argv[iArg++])!=0.0;
  }ಅನ್ಯಥಾ{
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
  ಯದಿ( zName ){
    sqlite3_str_appendf(pSql, "WHERE name=%Q", zName);
  }
  ಯದಿ( idx_ಅಂಕೆ & 0x08 ){
    sqlite3_str_appendf(pSql, " ORDER BY name");
  }
  zSql = sqlite3_str_finish(pSql);
  ಯದಿ( zSql==0 ){
    ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM_BKPT;
  }ಅನ್ಯಥಾ{
    rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pStmt, 0);
    sqlite3_free(zSql);
  }

  ಯದಿ( rc==SQLITE_OK ){
    pCsr->iPage = -1;
    rc = statNext(pCursor);
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ statColumn(
  sqlite3_vtab_cursor *pCursor, 
  sqlite3_context *ctx, 
  ಪೂರ್ಣಾಙ್ಕ i
){
  StatCursor *pCsr = (StatCursor *)pCursor;
  ವಿಕಲ್ಪ( i ){
    ಪಕ್ಷ 0:            /* name */
      sqlite3_result_text(ctx, pCsr->zName, -1, SQLITE_TRANSIENT);
      ಭಙ್ಗ;
    ಪಕ್ಷ 1:            /* path */
      ಯದಿ( !pCsr->isAgg ){
        sqlite3_result_text(ctx, pCsr->zPath, -1, SQLITE_TRANSIENT);
      }
      ಭಙ್ಗ;
    ಪಕ್ಷ 2:            /* pageno */
      ಯದಿ( pCsr->isAgg ){
        sqlite3_result_int64(ctx, pCsr->nPage);
      }ಅನ್ಯಥಾ{
        sqlite3_result_int64(ctx, pCsr->iPageno);
      }
      ಭಙ್ಗ;
    ಪಕ್ಷ 3:            /* pagetype */
      ಯದಿ( !pCsr->isAgg ){
        sqlite3_result_text(ctx, pCsr->zPagetype, -1, SQLITE_STATIC);
      }
      ಭಙ್ಗ;
    ಪಕ್ಷ 4:            /* ncell */
      sqlite3_result_int64(ctx, pCsr->nCell);
      ಭಙ್ಗ;
    ಪಕ್ಷ 5:            /* payload */
      sqlite3_result_int64(ctx, pCsr->nPayload);
      ಭಙ್ಗ;
    ಪಕ್ಷ 6:            /* unused */
      sqlite3_result_int64(ctx, pCsr->nUnused);
      ಭಙ್ಗ;
    ಪಕ್ಷ 7:            /* mx_payload */
      sqlite3_result_int64(ctx, pCsr->nMxPayload);
      ಭಙ್ಗ;
    ಪಕ್ಷ 8:            /* pgoffset */
      ಯದಿ( !pCsr->isAgg ){
        sqlite3_result_int64(ctx, pCsr->iOffset);
      }
      ಭಙ್ಗ;
    ಪಕ್ಷ 9:            /* pgsize */
      sqlite3_result_int64(ctx, pCsr->szPage);
      ಭಙ್ಗ;
    ಪಕ್ಷ 10: {         /* schema */
      sqlite3 *db = sqlite3_context_db_handle(ctx);
      ಪೂರ್ಣಾಙ್ಕ iDb = pCsr->iDb;
      sqlite3_result_text(ctx, db->aDb[iDb].zDbSName, -1, SQLITE_STATIC);
      ಭಙ್ಗ;
    }
    ಸ್ವಭಾವ: {         /* aggregate */
      sqlite3_result_int(ctx, pCsr->isAgg);
      ಭಙ್ಗ;
    }
  }
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ statRowid(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid){
  StatCursor *pCsr = (StatCursor *)pCursor;
  *pRowid = pCsr->iPageno;
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Invoke this routine to register the "dbstat" virtual table module
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3DbstatRegister(sqlite3 *db){
  ಸ್ಥಿರ sqlite3_module dbstat_module = {
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
  ಪ್ರತ್ಯಾಗಮ sqlite3_create_module(db, "dbstat", &dbstat_module, 0);
}
#ನೋಚೇದ್ಯದಿ defined(SQLITE_ENABLE_DBSTAT_VTAB)
ಪೂರ್ಣಾಙ್ಕ sqlite3DbstatRegister(sqlite3 *db){ ಪ್ರತ್ಯಾಗಮ SQLITE_OK; }
#endif /* SQLITE_ENABLE_DBSTAT_VTAB */
