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

#அந்தர்பாவ "sqliteInt.h"   /* Requires access to internal data structures */
#எனில் (defined(SQLITE_ENABLE_DBSTAT_VTAB) || defined(SQLITE_TEST)) \
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
நிலையான மாறிலி எழுத்து zDbstatSchema[] = 
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
வகைவரை கட்டமைப்பு StatTable StatTable;
வகைவரை கட்டமைப்பு StatCursor StatCursor;
வகைவரை கட்டமைப்பு StatPage StatPage;
வகைவரை கட்டமைப்பு StatCell StatCell;

/* Size information for a single cell within a btree page */
கட்டமைப்பு StatCell {
  முழு nLocal;                     /* Bytes of local payload */
  u32 iChildPg;                   /* Child node (or 0 if this is a leaf) */
  முழு nOvfl;                      /* Entries in aOvfl[] */
  u32 *aOvfl;                     /* Array of overflow page numbers */
  முழு nLastOvfl;                  /* Bytes of payload on final overflow page */
  முழு iOvfl;                      /* Iterates through aOvfl[] */
};

/* Size information for a single btree page */
கட்டமைப்பு StatPage {
  u32 iPgno;                      /* Page number */
  u8 *aPg;                        /* Page buffer from sqlite3_malloc() */
  முழு iCell;                      /* Current cell */
  எழுத்து *zPath;                    /* Path to this page */

  /* Variables populated by statDecodePage(): */
  u8 flags;                       /* Copy of flags byte */
  முழு nCell;                      /* Number of cells on page */
  முழு nUnused;                    /* Number of unused bytes on page */
  StatCell *aCell;                /* Array of parsed cells */
  u32 i_வலது_Child_Pg;              /* Right-child page number (or 0) */
  முழு nMxPayload;                 /* Largest payload of any cell on the page */
};

/* The cursor for scanning the dbstat virtual table */
கட்டமைப்பு StatCursor {
  sqlite3_vtab_cursor base;       /* base class.  MUST BE FIRST! */
  sqlite3_stmt *pStmt;            /* Iterates through set of root pages */
  u8 isEof;                       /* After pStmt has returned SQLITE_DONE */
  u8 isAgg;                       /* Aggregate results for each table */
  முழு iDb;                        /* Schema used for this query */

  StatPage aPage[32];             /* Pages in path to current page */
  முழு iPage;                      /* Current entry in aPage[] */

  /* Values to return. */
  u32 iPageno;                    /* Value of 'pageno' column */
  எழுத்து *zName;                    /* Value of 'name' column */
  எழுத்து *zPath;                    /* Value of 'path' column */
  எழுத்து *zPagetype;                /* Value of 'pagetype' column */
  முழு nPage;                      /* Number of pages in current btree */
  முழு nCell;                      /* Value of 'ncell' column */
  முழு nMxPayload;                 /* Value of 'mx_payload' column */
  i64 nUnused;                    /* Value of 'unused' column */
  i64 nPayload;                   /* Value of 'payload' column */
  i64 iOffset;                    /* Value of 'pgOffset' column */
  i64 szPage;                     /* Value of 'pgSize' column */
};

/* An instance of the DBSTAT virtual table */
கட்டமைப்பு StatTable {
  sqlite3_vtab base;              /* base class.  MUST BE FIRST! */
  sqlite3 *db;                    /* Database connection that owns this vtab */
  முழு iDb;                        /* Index of database to analyze */
};

#ifndef get2byte
# define get2byte(x)   ((x)[0]<<8 | (x)[1])
#endif

/*
** Connect to or create a new DBSTAT virtual table.
*/
நிலையான முழு statConnect(
  sqlite3 *db,
  வெறுமை *pAux,
  முழு argc, மாறிலி எழுத்து *மாறிலி*argv,
  sqlite3_vtab **ppVtab,
  எழுத்து **pzErr
){
  StatTable *pTab = 0;
  முழு rc = SQLITE_OK;
  முழு iDb;
  (வெறுமை)pAux;

  எனில்( argc>=4 ){
    Token nm;
    sqlite3TokenInit(&nm, (எழுத்து*)argv[3]);
    iDb = sqlite3FindDb(db, &nm);
    எனில்( iDb<0 ){
      *pzErr = sqlite3_mprintf("no such database: %s", argv[3]);
      திரும்பு SQLITE_ERROR;
    }
  }இல்லையெனில்{
    iDb = 0;
  }
  sqlite3_vtab_config(db, SQLITE_VTAB_DIRECTONLY);
  rc = sqlite3_declare_vtab(db, zDbstatSchema);
  எனில்( rc==SQLITE_OK ){
    pTab = (StatTable *)sqlite3_malloc64(அளவு(StatTable));
    எனில்( pTab==0 ) rc = SQLITE_NOMEM_BKPT;
  }

  ப்ரதிஜ்ஞா( rc==SQLITE_OK || pTab==0 );
  எனில்( rc==SQLITE_OK ){
    memset(pTab, 0, அளவு(StatTable));
    pTab->db = db;
    pTab->iDb = iDb;
  }

  *ppVtab = (sqlite3_vtab*)pTab;
  திரும்பு rc;
}

/*
** Disconnect from or destroy the DBSTAT virtual table.
*/
நிலையான முழு statDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  திரும்பு SQLITE_OK;
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
நிலையான முழு statBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  முழு i;
  முழு iSchema = -1;
  முழு iName = -1;
  முழு iAgg = -1;
  (வெறுமை)tab;

  /* Look for a valid schema=? constraint.  If found, change the idxNum to
  ** 1 and request the value of that constraint be sent to xFilter.  And
  ** lower the cost estimate to encourage the constrained version to be
  ** used.
  */
  க்கு(i=0; i<pIdxInfo->nConstraint; i++){
    எனில்( pIdxInfo->aConstraint[i].op!=SQLITE_INDEX_CONSTRAINT_EQ ) தொடர்;
    எனில்( pIdxInfo->aConstraint[i].usable==0 ){
      /* Force DBSTAT table should always be the right-most table in a join */
      திரும்பு SQLITE_CONSTRAINT;
    }
    தேர்வு( pIdxInfo->aConstraint[i].iColumn ){
      நிலை 0: {    /* name */
        iName = i;
        நிறுத்து;
      }
      நிலை 10: {   /* schema */
        iSchema = i;
        நிறுத்து;
      }
      நிலை 11: {   /* aggregate */
        iAgg = i;
        நிறுத்து;
      }
    }
  }
  i = 0;
  எனில்( iSchema>=0 ){
    pIdxInfo->aConstraintUsage[iSchema].argvIndex = ++i;
    pIdxInfo->aConstraintUsage[iSchema].omit = 1;
    pIdxInfo->idx_இலக்கம் |= 0x01;
  }
  எனில்( iName>=0 ){
    pIdxInfo->aConstraintUsage[iName].argvIndex = ++i;
    pIdxInfo->idx_இலக்கம் |= 0x02;
  }
  எனில்( iAgg>=0 ){
    pIdxInfo->aConstraintUsage[iAgg].argvIndex = ++i;
    pIdxInfo->idx_இலக்கம் |= 0x04;
  }
  pIdxInfo->estimatedCost = 1.0;

  /* Records are always returned in ascending order of (name, path). 
  ** If this will satisfy the client, set the orderByConsumed flag so that 
  ** SQLite does not do an external sort.
  */
  எனில்( ( pIdxInfo->nOrderBy==1
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
    pIdxInfo->idx_இலக்கம் |= 0x08;
  }
  pIdxInfo->idxFlags |= SQLITE_INDEX_SCAN_HEX;

  திரும்பு SQLITE_OK;
}

/*
** Open a new DBSTAT cursor.
*/
நிலையான முழு statOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  StatTable *pTab = (StatTable *)pVTab;
  StatCursor *pCsr;

  pCsr = (StatCursor *)sqlite3_malloc64(அளவு(StatCursor));
  எனில்( pCsr==0 ){
    திரும்பு SQLITE_NOMEM_BKPT;
  }இல்லையெனில்{
    memset(pCsr, 0, அளவு(StatCursor));
    pCsr->base.pVtab = pVTab;
    pCsr->iDb = pTab->iDb;
  }

  *ppCursor = (sqlite3_vtab_cursor *)pCsr;
  திரும்பு SQLITE_OK;
}

நிலையான வெறுமை statClearCells(StatPage *p){
  முழு i;
  எனில்( p->aCell ){
    க்கு(i=0; i<p->nCell; i++){
      sqlite3_free(p->aCell[i].aOvfl);
    }
    sqlite3_free(p->aCell);
  }
  p->nCell = 0;
  p->aCell = 0;
}

நிலையான வெறுமை statClearPage(StatPage *p){
  u8 *aPg = p->aPg;
  statClearCells(p);
  sqlite3_free(p->zPath);
  memset(p, 0, அளவு(StatPage));
  p->aPg = aPg;
}

நிலையான வெறுமை statResetCsr(StatCursor *pCsr){
  முழு i;
  /* In some circumstances, specifically if an OOM has occurred, the call
  ** to sqlite3_reset() may cause the pager to be reset (emptied). It is
  ** important that statClearPage() is called to free any page refs before
  ** this happens. dbsqlfuzz 9ed3e4e3816219d3509d711636c38542bf3f40b1. */
  க்கு(i=0; i<ArraySize(pCsr->aPage); i++){
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
நிலையான வெறுமை statResetCounts(StatCursor *pCsr){
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
நிலையான முழு statClose(sqlite3_vtab_cursor *pCursor){
  StatCursor *pCsr = (StatCursor *)pCursor;
  statResetCsr(pCsr);
  sqlite3_finalize(pCsr->pStmt);
  sqlite3_free(pCsr);
  திரும்பு SQLITE_OK;
}

/*
** For a single cell on a btree page, compute the number of bytes of
** content (payload) stored on that page.  That is to say, compute the
** number of bytes of content not found on overflow pages.
*/
நிலையான முழு getLocalPayload(
  முழு nUsable,                    /* Usable bytes per page */
  u8 flags,                       /* Page flags */
  முழு nTotal                      /* Total record (payload) size */
){
  முழு nLocal;
  முழு nMinLocal;
  முழு nMaxLocal;
 
  எனில்( flags==0x0D ){              /* Table leaf node */
    nMinLocal = (nUsable - 12) * 32 / 255 - 23;
    nMaxLocal = nUsable - 35;
  }இல்லையெனில்{                          /* Index interior and leaf nodes */
    nMinLocal = (nUsable - 12) * 32 / 255 - 23;
    nMaxLocal = (nUsable - 12) * 64 / 255 - 23;
  }

  nLocal = nMinLocal + (nTotal - nMinLocal) % (nUsable - 4);
  எனில்( nLocal>nMaxLocal ) nLocal = nMinLocal;
  திரும்பு nLocal;
}

/* Populate the StatPage object with information about the all
** cells found on the page currently under analysis.
*/
நிலையான முழு statDecodePage(Btree *pBt, StatPage *p){
  முழு nUnused;
  முழு iOff;
  முழு nHdr;
  முழு isLeaf;
  முழு szPage;

  u8 *aData = p->aPg;
  u8 *aHdr = &aData[p->iPgno==1 ? 100 : 0];

  p->flags = aHdr[0];
  எனில்( p->flags==0x0A || p->flags==0x0D ){
    isLeaf = 1;
    nHdr = 8;
  }இல்லையெனில் எனில்( p->flags==0x05 || p->flags==0x02 ){
    isLeaf = 0;
    nHdr = 12;
  }இல்லையெனில்{
    செல் statPageIsCorrupt;
  }
  எனில்( p->iPgno==1 ) nHdr += 100;
  p->nCell = get2byte(&aHdr[3]);
  p->nMxPayload = 0;
  szPage = sqlite3BtreeGetPageSize(pBt);

  nUnused = get2byte(&aHdr[5]) - nHdr - 2*p->nCell;
  nUnused += (முழு)aHdr[7];
  iOff = get2byte(&aHdr[1]);
  போது( iOff ){
    முழு iNext;
    எனில்( iOff>=szPage ) செல் statPageIsCorrupt;
    nUnused += get2byte(&aData[iOff+2]);
    iNext = get2byte(&aData[iOff]);
    எனில்( iNext<iOff+4 && iNext>0 ) செல் statPageIsCorrupt;
    iOff = iNext;
  }
  p->nUnused = nUnused;
  p->i_வலது_Child_Pg = isLeaf ? 0 : sqlite3Get4byte(&aHdr[8]);

  எனில்( p->nCell ){
    முழு i;                        /* Used to iterate through cells */
    முழு nUsable;                  /* Usable bytes per page */

    sqlite3BtreeEnter(pBt);
    nUsable = szPage - sqlite3BtreeGetReserveNoMutex(pBt);
    sqlite3BtreeLeave(pBt);
    p->aCell = sqlite3_malloc64((p->nCell+1) * அளவு(StatCell));
    எனில்( p->aCell==0 ) திரும்பு SQLITE_NOMEM_BKPT;
    memset(p->aCell, 0, (p->nCell+1) * அளவு(StatCell));

    க்கு(i=0; i<p->nCell; i++){
      StatCell *pCell = &p->aCell[i];

      iOff = get2byte(&aData[nHdr+i*2]);
      எனில்( iOff<nHdr || iOff>=szPage ) செல் statPageIsCorrupt;
      எனில்( !isLeaf ){
        pCell->iChildPg = sqlite3Get4byte(&aData[iOff]);
        iOff += 4;
      }
      எனில்( p->flags==0x05 ){
        /* A table interior node. nPayload==0. */
      }இல்லையெனில்{
        u32 nPayload;             /* Bytes of payload total (local+overflow) */
        முழு nLocal;               /* Bytes of payload stored locally */
        iOff += getVarint32(&aData[iOff], nPayload);
        எனில்( p->flags==0x0D ){
          u64 dummy;
          iOff += sqlite3GetVarint(&aData[iOff], &dummy);
        }
        எனில்( nPayload>(u32)p->nMxPayload ) p->nMxPayload = nPayload;
        nLocal = getLocalPayload(nUsable, p->flags, nPayload);
        எனில்( nLocal<0 ) செல் statPageIsCorrupt;
        pCell->nLocal = nLocal;
        ப்ரதிஜ்ஞா( nPayload>=(u32)nLocal );
        ப்ரதிஜ்ஞா( nLocal<=(nUsable-35) );
        எனில்( nPayload>(u32)nLocal ){
          முழு j;
          முழு nOvfl = ((nPayload - nLocal) + nUsable-4 - 1) / (nUsable - 4);
          எனில்( iOff+nLocal+4>nUsable || nPayload>0x7fffffff ){
            செல் statPageIsCorrupt;
          }
          pCell->nLastOvfl = (nPayload-nLocal) - (nOvfl-1) * (nUsable-4);
          pCell->nOvfl = nOvfl;
          pCell->aOvfl = sqlite3_malloc64(அளவு(u32)*nOvfl);
          எனில்( pCell->aOvfl==0 ) திரும்பு SQLITE_NOMEM_BKPT;
          pCell->aOvfl[0] = sqlite3Get4byte(&aData[iOff+nLocal]);
          க்கு(j=1; j<nOvfl; j++){
            முழு rc;
            u32 iPrev = pCell->aOvfl[j-1];
            DbPage *pPg = 0;
            rc = sqlite3PagerGet(sqlite3BtreePager(pBt), iPrev, &pPg, 0);
            எனில்( rc!=SQLITE_OK ){
              ப்ரதிஜ்ஞா( pPg==0 );
              திரும்பு rc;
            } 
            pCell->aOvfl[j] = sqlite3Get4byte(sqlite3PagerGetData(pPg));
            sqlite3PagerUnref(pPg);
          }
        }
      }
    }
  }

  திரும்பு SQLITE_OK;

statPageIsCorrupt:
  p->flags = 0;
  statClearCells(p);
  திரும்பு SQLITE_OK;
}

/*
** Populate the pCsr->iOffset and pCsr->szPage member variables. Based on
** the current value of pCsr->iPageno.
*/
நிலையான வெறுமை statSizeAndOffset(StatCursor *pCsr){
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
  எனில்( sqlite3OsFileControl(fd, 230440, &x)==SQLITE_OK ){
    pCsr->iOffset = x[0];
    pCsr->szPage += x[1];
  }இல்லையெனில்{
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
நிலையான முழு statGetPage(
  Btree *pBt,                     /* Load page from this b-tree */
  u32 iPg,                        /* Page number to load */
  StatPage *pPg                   /* Load page into this object */
){
  முழு pgsz = sqlite3BtreeGetPageSize(pBt);
  DbPage *pDbPage = 0;
  முழு rc;

  எனில்( pPg->aPg==0 ){
    pPg->aPg = (u8*)sqlite3_malloc(pgsz + DBSTAT_PAGE_PADDING_BYTES);
    எனில்( pPg->aPg==0 ){
      திரும்பு SQLITE_NOMEM_BKPT;
    }
    memset(&pPg->aPg[pgsz], 0, DBSTAT_PAGE_PADDING_BYTES);
  }

  rc = sqlite3PagerGet(sqlite3BtreePager(pBt), iPg, &pDbPage, 0);
  எனில்( rc==SQLITE_OK ){
    மாறிலி u8 *a = sqlite3PagerGetData(pDbPage);
    memcpy(pPg->aPg, a, pgsz);
    sqlite3PagerUnref(pDbPage);
  }

  திரும்பு rc;
}

/*
** Move a DBSTAT cursor to the next entry.  Normally, the next
** entry will be the next page, but in aggregated mode (pCsr->isAgg!=0),
** the next entry is the next btree.
*/
நிலையான முழு statNext(sqlite3_vtab_cursor *pCursor){
  முழு rc;
  முழு nPayload;
  எழுத்து *z;
  StatCursor *pCsr = (StatCursor *)pCursor;
  StatTable *pTab = (StatTable *)pCursor->pVtab;
  Btree *pBt = pTab->db->aDb[pCsr->iDb].pBt;
  Pager *pPager = sqlite3BtreePager(pBt);

  sqlite3_free(pCsr->zPath);
  pCsr->zPath = 0;

statNextRestart:
  எனில்( pCsr->iPage<0 ){
    /* Start measuring space on the next btree */
    statResetCounts(pCsr);
    rc = sqlite3_step(pCsr->pStmt);
    எனில்( rc==SQLITE_ROW ){
      முழு nPage;
      u32 i_வேர் = (u32)sqlite3_column_int64(pCsr->pStmt, 1);
      sqlite3PagerPagecount(pPager, &nPage);
      எனில்( nPage==0 ){
        pCsr->isEof = 1;
        திரும்பு sqlite3_reset(pCsr->pStmt);
      }
      rc = statGetPage(pBt, i_வேர், &pCsr->aPage[0]);
      pCsr->aPage[0].iPgno = i_வேர்;
      pCsr->aPage[0].iCell = 0;
      எனில்( !pCsr->isAgg ){
        pCsr->aPage[0].zPath = z = sqlite3_mprintf("/");
        எனில்( z==0 ) rc = SQLITE_NOMEM_BKPT;
      }
      pCsr->iPage = 0;
      pCsr->nPage = 1;
    }இல்லையெனில்{
      pCsr->isEof = 1;
      திரும்பு sqlite3_reset(pCsr->pStmt);
    }
  }இல்லையெனில்{
    /* Continue analyzing the btree previously started */
    StatPage *p = &pCsr->aPage[pCsr->iPage];
    எனில்( !pCsr->isAgg ) statResetCounts(pCsr);
    போது( p->iCell<p->nCell ){
      StatCell *pCell = &p->aCell[p->iCell];
      போது( pCell->iOvfl<pCell->nOvfl ){
        முழு nUsable;
        முழு iOvfl = pCell->iOvfl;
        sqlite3BtreeEnter(pBt);
        nUsable = sqlite3BtreeGetPageSize(pBt) - 
                        sqlite3BtreeGetReserveNoMutex(pBt);
        sqlite3BtreeLeave(pBt);
        pCsr->nPage++;
        pCsr->iPageno = pCell->aOvfl[iOvfl];
        statSizeAndOffset(pCsr);
        எனில்( pCell->iOvfl<pCell->nOvfl-1 ){
          pCsr->nPayload += nUsable - 4;
        }இல்லையெனில்{
          pCsr->nPayload += pCell->nLastOvfl;
          pCsr->nUnused += nUsable - 4 - pCell->nLastOvfl;
        }
        pCell->iOvfl++;
        எனில்( !pCsr->isAgg ){
          pCsr->zName = (எழுத்து *)sqlite3_column_text(pCsr->pStmt, 0);
          pCsr->zPagetype = "overflow";
          pCsr->zPath = z = sqlite3_mprintf(
              "%s%.3x+%.6x", p->zPath, p->iCell, iOvfl
          );
          திரும்பு z==0 ? SQLITE_NOMEM_BKPT : SQLITE_OK;
        }
      }
      எனில்( p->i_வலது_Child_Pg ) நிறுத்து;
      p->iCell++;
    }

    எனில்( !p->i_வலது_Child_Pg || p->iCell>p->nCell ){
      statClearPage(p);
      pCsr->iPage--;
      எனில்( pCsr->isAgg && pCsr->iPage<0 ){
        /* label-statNext-done:  When computing aggregate space usage over
        ** an entire btree, this is the exit point from this function */
        திரும்பு SQLITE_OK;
      }
      செல் statNextRestart; /* Tail recursion */
    }
    pCsr->iPage++;
    எனில்( pCsr->iPage>=ArraySize(pCsr->aPage) ){
      statResetCsr(pCsr);
      திரும்பு SQLITE_CORRUPT_BKPT;
    }
    ப்ரதிஜ்ஞா( p==&pCsr->aPage[pCsr->iPage-1] );

    எனில்( p->iCell==p->nCell ){
      p[1].iPgno = p->i_வலது_Child_Pg;
    }இல்லையெனில்{
      p[1].iPgno = p->aCell[p->iCell].iChildPg;
    }
    rc = statGetPage(pBt, p[1].iPgno, &p[1]);
    pCsr->nPage++;
    p[1].iCell = 0;
    எனில்( !pCsr->isAgg ){
      p[1].zPath = z = sqlite3_mprintf("%s%.3x/", p->zPath, p->iCell);
      எனில்( z==0 ) rc = SQLITE_NOMEM_BKPT;
    }
    p->iCell++;
  }


  /* Populate the StatCursor fields with the values to be returned
  ** by the xColumn() and xRowid() methods.
  */
  எனில்( rc==SQLITE_OK ){
    முழு i;
    StatPage *p = &pCsr->aPage[pCsr->iPage];
    pCsr->zName = (எழுத்து *)sqlite3_column_text(pCsr->pStmt, 0);
    pCsr->iPageno = p->iPgno;

    rc = statDecodePage(pBt, p);
    எனில்( rc==SQLITE_OK ){
      statSizeAndOffset(pCsr);

      தேர்வு( p->flags ){
        நிலை 0x05:             /* table internal */
        நிலை 0x02:             /* index internal */
          pCsr->zPagetype = "internal";
          நிறுத்து;
        நிலை 0x0D:             /* table leaf */
        நிலை 0x0A:             /* index leaf */
          pCsr->zPagetype = "leaf";
          நிறுத்து;
        இயல்பு:
          pCsr->zPagetype = "corrupted";
          நிறுத்து;
      }
      pCsr->nCell += p->nCell;
      pCsr->nUnused += p->nUnused;
      எனில்( p->nMxPayload>pCsr->nMxPayload ) pCsr->nMxPayload = p->nMxPayload;
      எனில்( !pCsr->isAgg ){
        pCsr->zPath = z = sqlite3_mprintf("%s", p->zPath);
        எனில்( z==0 ) rc = SQLITE_NOMEM_BKPT;
      }
      nPayload = 0;
      க்கு(i=0; i<p->nCell; i++){
        nPayload += p->aCell[i].nLocal;
      }
      pCsr->nPayload += nPayload;

      /* If computing aggregate space usage by btree, continue with the
      ** next page.  The loop will exit via the return at label-statNext-done
      */
      எனில்( pCsr->isAgg ) செல் statNextRestart;
    }
  }

  திரும்பு rc;
}

நிலையான முழு statEof(sqlite3_vtab_cursor *pCursor){
  StatCursor *pCsr = (StatCursor *)pCursor;
  திரும்பு pCsr->isEof;
}

/* Initialize a cursor according to the query plan idxNum using the
** arguments in argv[0].  See statBestIndex() for a description of the
** meaning of the bits in idxNum.
*/
நிலையான முழு statFilter(
  sqlite3_vtab_cursor *pCursor, 
  முழு idx_இலக்கம், மாறிலி எழுத்து *idxStr,
  முழு argc, sqlite3_மதிப்பு **argv
){
  StatCursor *pCsr = (StatCursor *)pCursor;
  StatTable *pTab = (StatTable*)(pCursor->pVtab);
  sqlite3_str *pSql;      /* Query of btrees to analyze */
  எழுத்து *zSql;             /* String value of pSql */
  முழு iArg = 0;           /* Count of argv[] parameters used so far */
  முழு rc = SQLITE_OK;     /* Result of this operation */
  மாறிலி எழுத்து *zName = 0;  /* Only provide analysis of this table */
  (வெறுமை)argc;
  (வெறுமை)idxStr;

  statResetCsr(pCsr);
  sqlite3_finalize(pCsr->pStmt);
  pCsr->pStmt = 0;
  எனில்( idx_இலக்கம் & 0x01 ){
    /* schema=? constraint is present.  Get its value */
    மாறிலி எழுத்து *zDbase = (மாறிலி எழுத்து*)sqlite3_மதிப்பு_text(argv[iArg++]);
    pCsr->iDb = sqlite3FindDbName(pTab->db, zDbase);
    எனில்( pCsr->iDb<0 ){
      pCsr->iDb = 0;
      pCsr->isEof = 1;
      திரும்பு SQLITE_OK;
    }
  }இல்லையெனில்{
    pCsr->iDb = pTab->iDb;
  }
  எனில்( idx_இலக்கம் & 0x02 ){
    /* name=? constraint is present */
    zName = (மாறிலி எழுத்து*)sqlite3_மதிப்பு_text(argv[iArg++]);
  }
  எனில்( idx_இலக்கம் & 0x04 ){
    /* aggregate=? constraint is present */
    pCsr->isAgg = sqlite3_மதிப்பு_double(argv[iArg++])!=0.0;
  }இல்லையெனில்{
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
  எனில்( zName ){
    sqlite3_str_appendf(pSql, "WHERE name=%Q", zName);
  }
  எனில்( idx_இலக்கம் & 0x08 ){
    sqlite3_str_appendf(pSql, " ORDER BY name");
  }
  zSql = sqlite3_str_finish(pSql);
  எனில்( zSql==0 ){
    திரும்பு SQLITE_NOMEM_BKPT;
  }இல்லையெனில்{
    rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pStmt, 0);
    sqlite3_free(zSql);
  }

  எனில்( rc==SQLITE_OK ){
    pCsr->iPage = -1;
    rc = statNext(pCursor);
  }
  திரும்பு rc;
}

நிலையான முழு statColumn(
  sqlite3_vtab_cursor *pCursor, 
  sqlite3_context *ctx, 
  முழு i
){
  StatCursor *pCsr = (StatCursor *)pCursor;
  தேர்வு( i ){
    நிலை 0:            /* name */
      sqlite3_result_text(ctx, pCsr->zName, -1, SQLITE_TRANSIENT);
      நிறுத்து;
    நிலை 1:            /* path */
      எனில்( !pCsr->isAgg ){
        sqlite3_result_text(ctx, pCsr->zPath, -1, SQLITE_TRANSIENT);
      }
      நிறுத்து;
    நிலை 2:            /* pageno */
      எனில்( pCsr->isAgg ){
        sqlite3_result_int64(ctx, pCsr->nPage);
      }இல்லையெனில்{
        sqlite3_result_int64(ctx, pCsr->iPageno);
      }
      நிறுத்து;
    நிலை 3:            /* pagetype */
      எனில்( !pCsr->isAgg ){
        sqlite3_result_text(ctx, pCsr->zPagetype, -1, SQLITE_STATIC);
      }
      நிறுத்து;
    நிலை 4:            /* ncell */
      sqlite3_result_int64(ctx, pCsr->nCell);
      நிறுத்து;
    நிலை 5:            /* payload */
      sqlite3_result_int64(ctx, pCsr->nPayload);
      நிறுத்து;
    நிலை 6:            /* unused */
      sqlite3_result_int64(ctx, pCsr->nUnused);
      நிறுத்து;
    நிலை 7:            /* mx_payload */
      sqlite3_result_int64(ctx, pCsr->nMxPayload);
      நிறுத்து;
    நிலை 8:            /* pgoffset */
      எனில்( !pCsr->isAgg ){
        sqlite3_result_int64(ctx, pCsr->iOffset);
      }
      நிறுத்து;
    நிலை 9:            /* pgsize */
      sqlite3_result_int64(ctx, pCsr->szPage);
      நிறுத்து;
    நிலை 10: {         /* schema */
      sqlite3 *db = sqlite3_context_db_handle(ctx);
      முழு iDb = pCsr->iDb;
      sqlite3_result_text(ctx, db->aDb[iDb].zDbSName, -1, SQLITE_STATIC);
      நிறுத்து;
    }
    இயல்பு: {         /* aggregate */
      sqlite3_result_int(ctx, pCsr->isAgg);
      நிறுத்து;
    }
  }
  திரும்பு SQLITE_OK;
}

நிலையான முழு statRowid(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid){
  StatCursor *pCsr = (StatCursor *)pCursor;
  *pRowid = pCsr->iPageno;
  திரும்பு SQLITE_OK;
}

/*
** Invoke this routine to register the "dbstat" virtual table module
*/
முழு sqlite3DbstatRegister(sqlite3 *db){
  நிலையான sqlite3_module dbstat_module = {
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
  திரும்பு sqlite3_create_module(db, "dbstat", &dbstat_module, 0);
}
#அல்லதெனில் defined(SQLITE_ENABLE_DBSTAT_VTAB)
முழு sqlite3DbstatRegister(sqlite3 *db){ திரும்பு SQLITE_OK; }
#endif /* SQLITE_ENABLE_DBSTAT_VTAB */
