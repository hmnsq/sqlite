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

#ଅନ୍ତର୍ଭାଵ "sqliteInt.h"   /* Requires access to internal data structures */
#ଯଦି (defined(SQLITE_ENABLE_DBSTAT_VTAB) || defined(SQLITE_TEST)) \
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
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର zDbstatSchema[] = 
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
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା StatTable StatTable;
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା StatCursor StatCursor;
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା StatPage StatPage;
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା StatCell StatCell;

/* Size information for a single cell within a btree page */
ସଂରଚନା StatCell {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nLocal;                     /* Bytes of local payload */
  u32 iChildPg;                   /* Child node (or 0 if this is a leaf) */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nOvfl;                      /* Entries in aOvfl[] */
  u32 *aOvfl;                     /* Array of overflow page numbers */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nLastOvfl;                  /* Bytes of payload on final overflow page */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iOvfl;                      /* Iterates through aOvfl[] */
};

/* Size information for a single btree page */
ସଂରଚନା StatPage {
  u32 iPgno;                      /* Page number */
  u8 *aPg;                        /* Page buffer from sqlite3_malloc() */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCell;                      /* Current cell */
  ଅକ୍ଷର *zPath;                    /* Path to this page */

  /* Variables populated by statDecodePage(): */
  u8 flags;                       /* Copy of flags byte */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nCell;                      /* Number of cells on page */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nUnused;                    /* Number of unused bytes on page */
  StatCell *aCell;                /* Array of parsed cells */
  u32 i_ଡାହାଣ_Child_Pg;              /* Right-child page number (or 0) */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nMxPayload;                 /* Largest payload of any cell on the page */
};

/* The cursor for scanning the dbstat virtual table */
ସଂରଚନା StatCursor {
  sqlite3_vtab_cursor base;       /* base class.  MUST BE FIRST! */
  sqlite3_stmt *pStmt;            /* Iterates through set of root pages */
  u8 isEof;                       /* After pStmt has returned SQLITE_DONE */
  u8 isAgg;                       /* Aggregate results for each table */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDb;                        /* Schema used for this query */

  StatPage aPage[32];             /* Pages in path to current page */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iPage;                      /* Current entry in aPage[] */

  /* Values to return. */
  u32 iPageno;                    /* Value of 'pageno' column */
  ଅକ୍ଷର *zName;                    /* Value of 'name' column */
  ଅକ୍ଷର *zPath;                    /* Value of 'path' column */
  ଅକ୍ଷର *zPagetype;                /* Value of 'pagetype' column */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nPage;                      /* Number of pages in current btree */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nCell;                      /* Value of 'ncell' column */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nMxPayload;                 /* Value of 'mx_payload' column */
  i64 nUnused;                    /* Value of 'unused' column */
  i64 nPayload;                   /* Value of 'payload' column */
  i64 iOffset;                    /* Value of 'pgOffset' column */
  i64 szPage;                     /* Value of 'pgSize' column */
};

/* An instance of the DBSTAT virtual table */
ସଂରଚନା StatTable {
  sqlite3_vtab base;              /* base class.  MUST BE FIRST! */
  sqlite3 *db;                    /* Database connection that owns this vtab */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDb;                        /* Index of database to analyze */
};

#ifndef get2byte
# define get2byte(x)   ((x)[0]<<8 | (x)[1])
#endif

/*
** Connect to or create a new DBSTAT virtual table.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା statConnect(
  sqlite3 *db,
  ଶୂନ୍ୟ *pAux,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅଚଳ ଅକ୍ଷର *ଅଚଳ*argv,
  sqlite3_vtab **ppVtab,
  ଅକ୍ଷର **pzErr
){
  StatTable *pTab = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDb;
  (ଶୂନ୍ୟ)pAux;

  ଯଦି( argc>=4 ){
    Token nm;
    sqlite3TokenInit(&nm, (ଅକ୍ଷର*)argv[3]);
    iDb = sqlite3FindDb(db, &nm);
    ଯଦି( iDb<0 ){
      *pzErr = sqlite3_mprintf("no such database: %s", argv[3]);
      ଫେରନ୍ତୁ SQLITE_ERROR;
    }
  }ଅନ୍ୟଥା{
    iDb = 0;
  }
  sqlite3_vtab_config(db, SQLITE_VTAB_DIRECTONLY);
  rc = sqlite3_declare_vtab(db, zDbstatSchema);
  ଯଦି( rc==SQLITE_OK ){
    pTab = (StatTable *)sqlite3_malloc64(ଆକାର(StatTable));
    ଯଦି( pTab==0 ) rc = SQLITE_NOMEM_BKPT;
  }

  ପ୍ରତିଜ୍ଞା( rc==SQLITE_OK || pTab==0 );
  ଯଦି( rc==SQLITE_OK ){
    memset(pTab, 0, ଆକାର(StatTable));
    pTab->db = db;
    pTab->iDb = iDb;
  }

  *ppVtab = (sqlite3_vtab*)pTab;
  ଫେରନ୍ତୁ rc;
}

/*
** Disconnect from or destroy the DBSTAT virtual table.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା statDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  ଫେରନ୍ତୁ SQLITE_OK;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା statBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iSchema = -1;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iName = -1;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iAgg = -1;
  (ଶୂନ୍ୟ)tab;

  /* Look for a valid schema=? constraint.  If found, change the idxNum to
  ** 1 and request the value of that constraint be sent to xFilter.  And
  ** lower the cost estimate to encourage the constrained version to be
  ** used.
  */
  ପାଇଁ(i=0; i<pIdxInfo->nConstraint; i++){
    ଯଦି( pIdxInfo->aConstraint[i].op!=SQLITE_INDEX_CONSTRAINT_EQ ) ଜାରି;
    ଯଦି( pIdxInfo->aConstraint[i].usable==0 ){
      /* Force DBSTAT table should always be the right-most table in a join */
      ଫେରନ୍ତୁ SQLITE_CONSTRAINT;
    }
    ବାଛ( pIdxInfo->aConstraint[i].iColumn ){
      ସ୍ଥିତି 0: {    /* name */
        iName = i;
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି 10: {   /* schema */
        iSchema = i;
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି 11: {   /* aggregate */
        iAgg = i;
        ଭାଙ୍ଗ;
      }
    }
  }
  i = 0;
  ଯଦି( iSchema>=0 ){
    pIdxInfo->aConstraintUsage[iSchema].argvIndex = ++i;
    pIdxInfo->aConstraintUsage[iSchema].omit = 1;
    pIdxInfo->idx_ଅଙ୍କ |= 0x01;
  }
  ଯଦି( iName>=0 ){
    pIdxInfo->aConstraintUsage[iName].argvIndex = ++i;
    pIdxInfo->idx_ଅଙ୍କ |= 0x02;
  }
  ଯଦି( iAgg>=0 ){
    pIdxInfo->aConstraintUsage[iAgg].argvIndex = ++i;
    pIdxInfo->idx_ଅଙ୍କ |= 0x04;
  }
  pIdxInfo->estimatedCost = 1.0;

  /* Records are always returned in ascending order of (name, path). 
  ** If this will satisfy the client, set the orderByConsumed flag so that 
  ** SQLite does not do an external sort.
  */
  ଯଦି( ( pIdxInfo->nOrderBy==1
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
    pIdxInfo->idx_ଅଙ୍କ |= 0x08;
  }
  pIdxInfo->idxFlags |= SQLITE_INDEX_SCAN_HEX;

  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Open a new DBSTAT cursor.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା statOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  StatTable *pTab = (StatTable *)pVTab;
  StatCursor *pCsr;

  pCsr = (StatCursor *)sqlite3_malloc64(ଆକାର(StatCursor));
  ଯଦି( pCsr==0 ){
    ଫେରନ୍ତୁ SQLITE_NOMEM_BKPT;
  }ଅନ୍ୟଥା{
    memset(pCsr, 0, ଆକାର(StatCursor));
    pCsr->base.pVtab = pVTab;
    pCsr->iDb = pTab->iDb;
  }

  *ppCursor = (sqlite3_vtab_cursor *)pCsr;
  ଫେରନ୍ତୁ SQLITE_OK;
}

ସ୍ଥିର ଶୂନ୍ୟ statClearCells(StatPage *p){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ଯଦି( p->aCell ){
    ପାଇଁ(i=0; i<p->nCell; i++){
      sqlite3_free(p->aCell[i].aOvfl);
    }
    sqlite3_free(p->aCell);
  }
  p->nCell = 0;
  p->aCell = 0;
}

ସ୍ଥିର ଶୂନ୍ୟ statClearPage(StatPage *p){
  u8 *aPg = p->aPg;
  statClearCells(p);
  sqlite3_free(p->zPath);
  memset(p, 0, ଆକାର(StatPage));
  p->aPg = aPg;
}

ସ୍ଥିର ଶୂନ୍ୟ statResetCsr(StatCursor *pCsr){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  /* In some circumstances, specifically if an OOM has occurred, the call
  ** to sqlite3_reset() may cause the pager to be reset (emptied). It is
  ** important that statClearPage() is called to free any page refs before
  ** this happens. dbsqlfuzz 9ed3e4e3816219d3509d711636c38542bf3f40b1. */
  ପାଇଁ(i=0; i<ArraySize(pCsr->aPage); i++){
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
ସ୍ଥିର ଶୂନ୍ୟ statResetCounts(StatCursor *pCsr){
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା statClose(sqlite3_vtab_cursor *pCursor){
  StatCursor *pCsr = (StatCursor *)pCursor;
  statResetCsr(pCsr);
  sqlite3_finalize(pCsr->pStmt);
  sqlite3_free(pCsr);
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** For a single cell on a btree page, compute the number of bytes of
** content (payload) stored on that page.  That is to say, compute the
** number of bytes of content not found on overflow pages.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା getLocalPayload(
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nUsable,                    /* Usable bytes per page */
  u8 flags,                       /* Page flags */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nTotal                      /* Total record (payload) size */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nLocal;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nMinLocal;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nMaxLocal;
 
  ଯଦି( flags==0x0D ){              /* Table leaf node */
    nMinLocal = (nUsable - 12) * 32 / 255 - 23;
    nMaxLocal = nUsable - 35;
  }ଅନ୍ୟଥା{                          /* Index interior and leaf nodes */
    nMinLocal = (nUsable - 12) * 32 / 255 - 23;
    nMaxLocal = (nUsable - 12) * 64 / 255 - 23;
  }

  nLocal = nMinLocal + (nTotal - nMinLocal) % (nUsable - 4);
  ଯଦି( nLocal>nMaxLocal ) nLocal = nMinLocal;
  ଫେରନ୍ତୁ nLocal;
}

/* Populate the StatPage object with information about the all
** cells found on the page currently under analysis.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା statDecodePage(Btree *pBt, StatPage *p){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nUnused;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iOff;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nHdr;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା isLeaf;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା szPage;

  u8 *aData = p->aPg;
  u8 *aHdr = &aData[p->iPgno==1 ? 100 : 0];

  p->flags = aHdr[0];
  ଯଦି( p->flags==0x0A || p->flags==0x0D ){
    isLeaf = 1;
    nHdr = 8;
  }ଅନ୍ୟଥା ଯଦି( p->flags==0x05 || p->flags==0x02 ){
    isLeaf = 0;
    nHdr = 12;
  }ଅନ୍ୟଥା{
    ଯାଅ statPageIsCorrupt;
  }
  ଯଦି( p->iPgno==1 ) nHdr += 100;
  p->nCell = get2byte(&aHdr[3]);
  p->nMxPayload = 0;
  szPage = sqlite3BtreeGetPageSize(pBt);

  nUnused = get2byte(&aHdr[5]) - nHdr - 2*p->nCell;
  nUnused += (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)aHdr[7];
  iOff = get2byte(&aHdr[1]);
  ଯେତେବେଳେ( iOff ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା iNext;
    ଯଦି( iOff>=szPage ) ଯାଅ statPageIsCorrupt;
    nUnused += get2byte(&aData[iOff+2]);
    iNext = get2byte(&aData[iOff]);
    ଯଦି( iNext<iOff+4 && iNext>0 ) ଯାଅ statPageIsCorrupt;
    iOff = iNext;
  }
  p->nUnused = nUnused;
  p->i_ଡାହାଣ_Child_Pg = isLeaf ? 0 : sqlite3Get4byte(&aHdr[8]);

  ଯଦି( p->nCell ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;                        /* Used to iterate through cells */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା nUsable;                  /* Usable bytes per page */

    sqlite3BtreeEnter(pBt);
    nUsable = szPage - sqlite3BtreeGetReserveNoMutex(pBt);
    sqlite3BtreeLeave(pBt);
    p->aCell = sqlite3_malloc64((p->nCell+1) * ଆକାର(StatCell));
    ଯଦି( p->aCell==0 ) ଫେରନ୍ତୁ SQLITE_NOMEM_BKPT;
    memset(p->aCell, 0, (p->nCell+1) * ଆକାର(StatCell));

    ପାଇଁ(i=0; i<p->nCell; i++){
      StatCell *pCell = &p->aCell[i];

      iOff = get2byte(&aData[nHdr+i*2]);
      ଯଦି( iOff<nHdr || iOff>=szPage ) ଯାଅ statPageIsCorrupt;
      ଯଦି( !isLeaf ){
        pCell->iChildPg = sqlite3Get4byte(&aData[iOff]);
        iOff += 4;
      }
      ଯଦି( p->flags==0x05 ){
        /* A table interior node. nPayload==0. */
      }ଅନ୍ୟଥା{
        u32 nPayload;             /* Bytes of payload total (local+overflow) */
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା nLocal;               /* Bytes of payload stored locally */
        iOff += getVarint32(&aData[iOff], nPayload);
        ଯଦି( p->flags==0x0D ){
          u64 dummy;
          iOff += sqlite3GetVarint(&aData[iOff], &dummy);
        }
        ଯଦି( nPayload>(u32)p->nMxPayload ) p->nMxPayload = nPayload;
        nLocal = getLocalPayload(nUsable, p->flags, nPayload);
        ଯଦି( nLocal<0 ) ଯାଅ statPageIsCorrupt;
        pCell->nLocal = nLocal;
        ପ୍ରତିଜ୍ଞା( nPayload>=(u32)nLocal );
        ପ୍ରତିଜ୍ଞା( nLocal<=(nUsable-35) );
        ଯଦି( nPayload>(u32)nLocal ){
          ପୂର୍ଣ୍ଣସଂଖ୍ୟା j;
          ପୂର୍ଣ୍ଣସଂଖ୍ୟା nOvfl = ((nPayload - nLocal) + nUsable-4 - 1) / (nUsable - 4);
          ଯଦି( iOff+nLocal+4>nUsable || nPayload>0x7fffffff ){
            ଯାଅ statPageIsCorrupt;
          }
          pCell->nLastOvfl = (nPayload-nLocal) - (nOvfl-1) * (nUsable-4);
          pCell->nOvfl = nOvfl;
          pCell->aOvfl = sqlite3_malloc64(ଆକାର(u32)*nOvfl);
          ଯଦି( pCell->aOvfl==0 ) ଫେରନ୍ତୁ SQLITE_NOMEM_BKPT;
          pCell->aOvfl[0] = sqlite3Get4byte(&aData[iOff+nLocal]);
          ପାଇଁ(j=1; j<nOvfl; j++){
            ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
            u32 iPrev = pCell->aOvfl[j-1];
            DbPage *pPg = 0;
            rc = sqlite3PagerGet(sqlite3BtreePager(pBt), iPrev, &pPg, 0);
            ଯଦି( rc!=SQLITE_OK ){
              ପ୍ରତିଜ୍ଞା( pPg==0 );
              ଫେରନ୍ତୁ rc;
            } 
            pCell->aOvfl[j] = sqlite3Get4byte(sqlite3PagerGetData(pPg));
            sqlite3PagerUnref(pPg);
          }
        }
      }
    }
  }

  ଫେରନ୍ତୁ SQLITE_OK;

statPageIsCorrupt:
  p->flags = 0;
  statClearCells(p);
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Populate the pCsr->iOffset and pCsr->szPage member variables. Based on
** the current value of pCsr->iPageno.
*/
ସ୍ଥିର ଶୂନ୍ୟ statSizeAndOffset(StatCursor *pCsr){
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
  ଯଦି( sqlite3OsFileControl(fd, 230440, &x)==SQLITE_OK ){
    pCsr->iOffset = x[0];
    pCsr->szPage += x[1];
  }ଅନ୍ୟଥା{
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା statGetPage(
  Btree *pBt,                     /* Load page from this b-tree */
  u32 iPg,                        /* Page number to load */
  StatPage *pPg                   /* Load page into this object */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା pgsz = sqlite3BtreeGetPageSize(pBt);
  DbPage *pDbPage = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

  ଯଦି( pPg->aPg==0 ){
    pPg->aPg = (u8*)sqlite3_malloc(pgsz + DBSTAT_PAGE_PADDING_BYTES);
    ଯଦି( pPg->aPg==0 ){
      ଫେରନ୍ତୁ SQLITE_NOMEM_BKPT;
    }
    memset(&pPg->aPg[pgsz], 0, DBSTAT_PAGE_PADDING_BYTES);
  }

  rc = sqlite3PagerGet(sqlite3BtreePager(pBt), iPg, &pDbPage, 0);
  ଯଦି( rc==SQLITE_OK ){
    ଅଚଳ u8 *a = sqlite3PagerGetData(pDbPage);
    memcpy(pPg->aPg, a, pgsz);
    sqlite3PagerUnref(pDbPage);
  }

  ଫେରନ୍ତୁ rc;
}

/*
** Move a DBSTAT cursor to the next entry.  Normally, the next
** entry will be the next page, but in aggregated mode (pCsr->isAgg!=0),
** the next entry is the next btree.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା statNext(sqlite3_vtab_cursor *pCursor){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nPayload;
  ଅକ୍ଷର *z;
  StatCursor *pCsr = (StatCursor *)pCursor;
  StatTable *pTab = (StatTable *)pCursor->pVtab;
  Btree *pBt = pTab->db->aDb[pCsr->iDb].pBt;
  Pager *pPager = sqlite3BtreePager(pBt);

  sqlite3_free(pCsr->zPath);
  pCsr->zPath = 0;

statNextRestart:
  ଯଦି( pCsr->iPage<0 ){
    /* Start measuring space on the next btree */
    statResetCounts(pCsr);
    rc = sqlite3_step(pCsr->pStmt);
    ଯଦି( rc==SQLITE_ROW ){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା nPage;
      u32 i_ମୂଳ = (u32)sqlite3_column_int64(pCsr->pStmt, 1);
      sqlite3PagerPagecount(pPager, &nPage);
      ଯଦି( nPage==0 ){
        pCsr->isEof = 1;
        ଫେରନ୍ତୁ sqlite3_reset(pCsr->pStmt);
      }
      rc = statGetPage(pBt, i_ମୂଳ, &pCsr->aPage[0]);
      pCsr->aPage[0].iPgno = i_ମୂଳ;
      pCsr->aPage[0].iCell = 0;
      ଯଦି( !pCsr->isAgg ){
        pCsr->aPage[0].zPath = z = sqlite3_mprintf("/");
        ଯଦି( z==0 ) rc = SQLITE_NOMEM_BKPT;
      }
      pCsr->iPage = 0;
      pCsr->nPage = 1;
    }ଅନ୍ୟଥା{
      pCsr->isEof = 1;
      ଫେରନ୍ତୁ sqlite3_reset(pCsr->pStmt);
    }
  }ଅନ୍ୟଥା{
    /* Continue analyzing the btree previously started */
    StatPage *p = &pCsr->aPage[pCsr->iPage];
    ଯଦି( !pCsr->isAgg ) statResetCounts(pCsr);
    ଯେତେବେଳେ( p->iCell<p->nCell ){
      StatCell *pCell = &p->aCell[p->iCell];
      ଯେତେବେଳେ( pCell->iOvfl<pCell->nOvfl ){
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା nUsable;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା iOvfl = pCell->iOvfl;
        sqlite3BtreeEnter(pBt);
        nUsable = sqlite3BtreeGetPageSize(pBt) - 
                        sqlite3BtreeGetReserveNoMutex(pBt);
        sqlite3BtreeLeave(pBt);
        pCsr->nPage++;
        pCsr->iPageno = pCell->aOvfl[iOvfl];
        statSizeAndOffset(pCsr);
        ଯଦି( pCell->iOvfl<pCell->nOvfl-1 ){
          pCsr->nPayload += nUsable - 4;
        }ଅନ୍ୟଥା{
          pCsr->nPayload += pCell->nLastOvfl;
          pCsr->nUnused += nUsable - 4 - pCell->nLastOvfl;
        }
        pCell->iOvfl++;
        ଯଦି( !pCsr->isAgg ){
          pCsr->zName = (ଅକ୍ଷର *)sqlite3_column_text(pCsr->pStmt, 0);
          pCsr->zPagetype = "overflow";
          pCsr->zPath = z = sqlite3_mprintf(
              "%s%.3x+%.6x", p->zPath, p->iCell, iOvfl
          );
          ଫେରନ୍ତୁ z==0 ? SQLITE_NOMEM_BKPT : SQLITE_OK;
        }
      }
      ଯଦି( p->i_ଡାହାଣ_Child_Pg ) ଭାଙ୍ଗ;
      p->iCell++;
    }

    ଯଦି( !p->i_ଡାହାଣ_Child_Pg || p->iCell>p->nCell ){
      statClearPage(p);
      pCsr->iPage--;
      ଯଦି( pCsr->isAgg && pCsr->iPage<0 ){
        /* label-statNext-done:  When computing aggregate space usage over
        ** an entire btree, this is the exit point from this function */
        ଫେରନ୍ତୁ SQLITE_OK;
      }
      ଯାଅ statNextRestart; /* Tail recursion */
    }
    pCsr->iPage++;
    ଯଦି( pCsr->iPage>=ArraySize(pCsr->aPage) ){
      statResetCsr(pCsr);
      ଫେରନ୍ତୁ SQLITE_CORRUPT_BKPT;
    }
    ପ୍ରତିଜ୍ଞା( p==&pCsr->aPage[pCsr->iPage-1] );

    ଯଦି( p->iCell==p->nCell ){
      p[1].iPgno = p->i_ଡାହାଣ_Child_Pg;
    }ଅନ୍ୟଥା{
      p[1].iPgno = p->aCell[p->iCell].iChildPg;
    }
    rc = statGetPage(pBt, p[1].iPgno, &p[1]);
    pCsr->nPage++;
    p[1].iCell = 0;
    ଯଦି( !pCsr->isAgg ){
      p[1].zPath = z = sqlite3_mprintf("%s%.3x/", p->zPath, p->iCell);
      ଯଦି( z==0 ) rc = SQLITE_NOMEM_BKPT;
    }
    p->iCell++;
  }


  /* Populate the StatCursor fields with the values to be returned
  ** by the xColumn() and xRowid() methods.
  */
  ଯଦି( rc==SQLITE_OK ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    StatPage *p = &pCsr->aPage[pCsr->iPage];
    pCsr->zName = (ଅକ୍ଷର *)sqlite3_column_text(pCsr->pStmt, 0);
    pCsr->iPageno = p->iPgno;

    rc = statDecodePage(pBt, p);
    ଯଦି( rc==SQLITE_OK ){
      statSizeAndOffset(pCsr);

      ବାଛ( p->flags ){
        ସ୍ଥିତି 0x05:             /* table internal */
        ସ୍ଥିତି 0x02:             /* index internal */
          pCsr->zPagetype = "internal";
          ଭାଙ୍ଗ;
        ସ୍ଥିତି 0x0D:             /* table leaf */
        ସ୍ଥିତି 0x0A:             /* index leaf */
          pCsr->zPagetype = "leaf";
          ଭାଙ୍ଗ;
        ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
          pCsr->zPagetype = "corrupted";
          ଭାଙ୍ଗ;
      }
      pCsr->nCell += p->nCell;
      pCsr->nUnused += p->nUnused;
      ଯଦି( p->nMxPayload>pCsr->nMxPayload ) pCsr->nMxPayload = p->nMxPayload;
      ଯଦି( !pCsr->isAgg ){
        pCsr->zPath = z = sqlite3_mprintf("%s", p->zPath);
        ଯଦି( z==0 ) rc = SQLITE_NOMEM_BKPT;
      }
      nPayload = 0;
      ପାଇଁ(i=0; i<p->nCell; i++){
        nPayload += p->aCell[i].nLocal;
      }
      pCsr->nPayload += nPayload;

      /* If computing aggregate space usage by btree, continue with the
      ** next page.  The loop will exit via the return at label-statNext-done
      */
      ଯଦି( pCsr->isAgg ) ଯାଅ statNextRestart;
    }
  }

  ଫେରନ୍ତୁ rc;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା statEof(sqlite3_vtab_cursor *pCursor){
  StatCursor *pCsr = (StatCursor *)pCursor;
  ଫେରନ୍ତୁ pCsr->isEof;
}

/* Initialize a cursor according to the query plan idxNum using the
** arguments in argv[0].  See statBestIndex() for a description of the
** meaning of the bits in idxNum.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା statFilter(
  sqlite3_vtab_cursor *pCursor, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx_ଅଙ୍କ, ଅଚଳ ଅକ୍ଷର *idxStr,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, sqlite3_ମୂଲ୍ୟ **argv
){
  StatCursor *pCsr = (StatCursor *)pCursor;
  StatTable *pTab = (StatTable*)(pCursor->pVtab);
  sqlite3_str *pSql;      /* Query of btrees to analyze */
  ଅକ୍ଷର *zSql;             /* String value of pSql */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iArg = 0;           /* Count of argv[] parameters used so far */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;     /* Result of this operation */
  ଅଚଳ ଅକ୍ଷର *zName = 0;  /* Only provide analysis of this table */
  (ଶୂନ୍ୟ)argc;
  (ଶୂନ୍ୟ)idxStr;

  statResetCsr(pCsr);
  sqlite3_finalize(pCsr->pStmt);
  pCsr->pStmt = 0;
  ଯଦି( idx_ଅଙ୍କ & 0x01 ){
    /* schema=? constraint is present.  Get its value */
    ଅଚଳ ଅକ୍ଷର *zDbase = (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(argv[iArg++]);
    pCsr->iDb = sqlite3FindDbName(pTab->db, zDbase);
    ଯଦି( pCsr->iDb<0 ){
      pCsr->iDb = 0;
      pCsr->isEof = 1;
      ଫେରନ୍ତୁ SQLITE_OK;
    }
  }ଅନ୍ୟଥା{
    pCsr->iDb = pTab->iDb;
  }
  ଯଦି( idx_ଅଙ୍କ & 0x02 ){
    /* name=? constraint is present */
    zName = (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(argv[iArg++]);
  }
  ଯଦି( idx_ଅଙ୍କ & 0x04 ){
    /* aggregate=? constraint is present */
    pCsr->isAgg = sqlite3_ମୂଲ୍ୟ_double(argv[iArg++])!=0.0;
  }ଅନ୍ୟଥା{
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
  ଯଦି( zName ){
    sqlite3_str_appendf(pSql, "WHERE name=%Q", zName);
  }
  ଯଦି( idx_ଅଙ୍କ & 0x08 ){
    sqlite3_str_appendf(pSql, " ORDER BY name");
  }
  zSql = sqlite3_str_finish(pSql);
  ଯଦି( zSql==0 ){
    ଫେରନ୍ତୁ SQLITE_NOMEM_BKPT;
  }ଅନ୍ୟଥା{
    rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pStmt, 0);
    sqlite3_free(zSql);
  }

  ଯଦି( rc==SQLITE_OK ){
    pCsr->iPage = -1;
    rc = statNext(pCursor);
  }
  ଫେରନ୍ତୁ rc;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା statColumn(
  sqlite3_vtab_cursor *pCursor, 
  sqlite3_context *ctx, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i
){
  StatCursor *pCsr = (StatCursor *)pCursor;
  ବାଛ( i ){
    ସ୍ଥିତି 0:            /* name */
      sqlite3_result_text(ctx, pCsr->zName, -1, SQLITE_TRANSIENT);
      ଭାଙ୍ଗ;
    ସ୍ଥିତି 1:            /* path */
      ଯଦି( !pCsr->isAgg ){
        sqlite3_result_text(ctx, pCsr->zPath, -1, SQLITE_TRANSIENT);
      }
      ଭାଙ୍ଗ;
    ସ୍ଥିତି 2:            /* pageno */
      ଯଦି( pCsr->isAgg ){
        sqlite3_result_int64(ctx, pCsr->nPage);
      }ଅନ୍ୟଥା{
        sqlite3_result_int64(ctx, pCsr->iPageno);
      }
      ଭାଙ୍ଗ;
    ସ୍ଥିତି 3:            /* pagetype */
      ଯଦି( !pCsr->isAgg ){
        sqlite3_result_text(ctx, pCsr->zPagetype, -1, SQLITE_STATIC);
      }
      ଭାଙ୍ଗ;
    ସ୍ଥିତି 4:            /* ncell */
      sqlite3_result_int64(ctx, pCsr->nCell);
      ଭାଙ୍ଗ;
    ସ୍ଥିତି 5:            /* payload */
      sqlite3_result_int64(ctx, pCsr->nPayload);
      ଭାଙ୍ଗ;
    ସ୍ଥିତି 6:            /* unused */
      sqlite3_result_int64(ctx, pCsr->nUnused);
      ଭାଙ୍ଗ;
    ସ୍ଥିତି 7:            /* mx_payload */
      sqlite3_result_int64(ctx, pCsr->nMxPayload);
      ଭାଙ୍ଗ;
    ସ୍ଥିତି 8:            /* pgoffset */
      ଯଦି( !pCsr->isAgg ){
        sqlite3_result_int64(ctx, pCsr->iOffset);
      }
      ଭାଙ୍ଗ;
    ସ୍ଥିତି 9:            /* pgsize */
      sqlite3_result_int64(ctx, pCsr->szPage);
      ଭାଙ୍ଗ;
    ସ୍ଥିତି 10: {         /* schema */
      sqlite3 *db = sqlite3_context_db_handle(ctx);
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDb = pCsr->iDb;
      sqlite3_result_text(ctx, db->aDb[iDb].zDbSName, -1, SQLITE_STATIC);
      ଭାଙ୍ଗ;
    }
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {         /* aggregate */
      sqlite3_result_int(ctx, pCsr->isAgg);
      ଭାଙ୍ଗ;
    }
  }
  ଫେରନ୍ତୁ SQLITE_OK;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା statRowid(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid){
  StatCursor *pCsr = (StatCursor *)pCursor;
  *pRowid = pCsr->iPageno;
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Invoke this routine to register the "dbstat" virtual table module
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3DbstatRegister(sqlite3 *db){
  ସ୍ଥିର sqlite3_module dbstat_module = {
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
  ଫେରନ୍ତୁ sqlite3_create_module(db, "dbstat", &dbstat_module, 0);
}
#ନଚେତଯଦି defined(SQLITE_ENABLE_DBSTAT_VTAB)
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3DbstatRegister(sqlite3 *db){ ଫେରନ୍ତୁ SQLITE_OK; }
#endif /* SQLITE_ENABLE_DBSTAT_VTAB */
