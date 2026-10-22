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
#यदि (defined(SQLITE_ENABLE_DBSTAT_VTAB) || defined(SQLITE_TEST)) \
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
स्थिर ध्रुव वर्ण zDbstatSchema[] = 
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
प्रकारनिर्देश संरचना StatTable StatTable;
प्रकारनिर्देश संरचना StatCursor StatCursor;
प्रकारनिर्देश संरचना StatPage StatPage;
प्रकारनिर्देश संरचना StatCell StatCell;

/* Size information for a single cell within a btree page */
संरचना StatCell {
  पूर्णाङ्क nLocal;                     /* Bytes of local payload */
  u32 iChildPg;                   /* Child node (or 0 if this is a leaf) */
  पूर्णाङ्क nOvfl;                      /* Entries in aOvfl[] */
  u32 *aOvfl;                     /* Array of overflow page numbers */
  पूर्णाङ्क nLastOvfl;                  /* Bytes of payload on final overflow page */
  पूर्णाङ्क iOvfl;                      /* Iterates through aOvfl[] */
};

/* Size information for a single btree page */
संरचना StatPage {
  u32 iPgno;                      /* Page number */
  u8 *aPg;                        /* Page buffer from sqlite3_malloc() */
  पूर्णाङ्क iCell;                      /* Current cell */
  वर्ण *zPath;                    /* Path to this page */

  /* Variables populated by statDecodePage(): */
  u8 flags;                       /* Copy of flags byte */
  पूर्णाङ्क nCell;                      /* Number of cells on page */
  पूर्णाङ्क nUnused;                    /* Number of unused bytes on page */
  StatCell *aCell;                /* Array of parsed cells */
  u32 i_उजवा_Child_Pg;              /* Right-child page number (or 0) */
  पूर्णाङ्क nMxPayload;                 /* Largest payload of any cell on the page */
};

/* The cursor for scanning the dbstat virtual table */
संरचना StatCursor {
  sqlite3_vtab_cursor base;       /* base class.  MUST BE FIRST! */
  sqlite3_stmt *pStmt;            /* Iterates through set of root pages */
  u8 isEof;                       /* After pStmt has returned SQLITE_DONE */
  u8 isAgg;                       /* Aggregate results for each table */
  पूर्णाङ्क iDb;                        /* Schema used for this query */

  StatPage aPage[32];             /* Pages in path to current page */
  पूर्णाङ्क iPage;                      /* Current entry in aPage[] */

  /* Values to return. */
  u32 iPageno;                    /* Value of 'pageno' column */
  वर्ण *zName;                    /* Value of 'name' column */
  वर्ण *zPath;                    /* Value of 'path' column */
  वर्ण *zPagetype;                /* Value of 'pagetype' column */
  पूर्णाङ्क nPage;                      /* Number of pages in current btree */
  पूर्णाङ्क nCell;                      /* Value of 'ncell' column */
  पूर्णाङ्क nMxPayload;                 /* Value of 'mx_payload' column */
  i64 nUnused;                    /* Value of 'unused' column */
  i64 nPayload;                   /* Value of 'payload' column */
  i64 iOffset;                    /* Value of 'pgOffset' column */
  i64 szPage;                     /* Value of 'pgSize' column */
};

/* An instance of the DBSTAT virtual table */
संरचना StatTable {
  sqlite3_vtab base;              /* base class.  MUST BE FIRST! */
  sqlite3 *db;                    /* Database connection that owns this vtab */
  पूर्णाङ्क iDb;                        /* Index of database to analyze */
};

#ifndef get2byte
# define get2byte(x)   ((x)[0]<<8 | (x)[1])
#endif

/*
** Connect to or create a new DBSTAT virtual table.
*/
स्थिर पूर्णाङ्क statConnect(
  sqlite3 *db,
  शून्य *pAux,
  पूर्णाङ्क argc, ध्रुव वर्ण *ध्रुव*argv,
  sqlite3_vtab **ppVtab,
  वर्ण **pzErr
){
  StatTable *pTab = 0;
  पूर्णाङ्क rc = SQLITE_OK;
  पूर्णाङ्क iDb;
  (शून्य)pAux;

  यदि( argc>=4 ){
    Token nm;
    sqlite3TokenInit(&nm, (वर्ण*)argv[3]);
    iDb = sqlite3FindDb(db, &nm);
    यदि( iDb<0 ){
      *pzErr = sqlite3_mprintf("no such database: %s", argv[3]);
      प्रत्यागम SQLITE_ERROR;
    }
  }अन्यथा{
    iDb = 0;
  }
  sqlite3_vtab_config(db, SQLITE_VTAB_DIRECTONLY);
  rc = sqlite3_declare_vtab(db, zDbstatSchema);
  यदि( rc==SQLITE_OK ){
    pTab = (StatTable *)sqlite3_malloc64(परिमाण(StatTable));
    यदि( pTab==0 ) rc = SQLITE_NOMEM_BKPT;
  }

  प्रतिज्ञा( rc==SQLITE_OK || pTab==0 );
  यदि( rc==SQLITE_OK ){
    memset(pTab, 0, परिमाण(StatTable));
    pTab->db = db;
    pTab->iDb = iDb;
  }

  *ppVtab = (sqlite3_vtab*)pTab;
  प्रत्यागम rc;
}

/*
** Disconnect from or destroy the DBSTAT virtual table.
*/
स्थिर पूर्णाङ्क statDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  प्रत्यागम SQLITE_OK;
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
स्थिर पूर्णाङ्क statBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  पूर्णाङ्क i;
  पूर्णाङ्क iSchema = -1;
  पूर्णाङ्क iName = -1;
  पूर्णाङ्क iAgg = -1;
  (शून्य)tab;

  /* Look for a valid schema=? constraint.  If found, change the idxNum to
  ** 1 and request the value of that constraint be sent to xFilter.  And
  ** lower the cost estimate to encourage the constrained version to be
  ** used.
  */
  प्रति(i=0; i<pIdxInfo->nConstraint; i++){
    यदि( pIdxInfo->aConstraint[i].op!=SQLITE_INDEX_CONSTRAINT_EQ ) अनुवर्त;
    यदि( pIdxInfo->aConstraint[i].usable==0 ){
      /* Force DBSTAT table should always be the right-most table in a join */
      प्रत्यागम SQLITE_CONSTRAINT;
    }
    विकल्प( pIdxInfo->aConstraint[i].iColumn ){
      पक्ष 0: {    /* name */
        iName = i;
        भङ्ग;
      }
      पक्ष 10: {   /* schema */
        iSchema = i;
        भङ्ग;
      }
      पक्ष 11: {   /* aggregate */
        iAgg = i;
        भङ्ग;
      }
    }
  }
  i = 0;
  यदि( iSchema>=0 ){
    pIdxInfo->aConstraintUsage[iSchema].argvIndex = ++i;
    pIdxInfo->aConstraintUsage[iSchema].omit = 1;
    pIdxInfo->idx_अंक |= 0x01;
  }
  यदि( iName>=0 ){
    pIdxInfo->aConstraintUsage[iName].argvIndex = ++i;
    pIdxInfo->idx_अंक |= 0x02;
  }
  यदि( iAgg>=0 ){
    pIdxInfo->aConstraintUsage[iAgg].argvIndex = ++i;
    pIdxInfo->idx_अंक |= 0x04;
  }
  pIdxInfo->estimatedCost = 1.0;

  /* Records are always returned in ascending order of (name, path). 
  ** If this will satisfy the client, set the orderByConsumed flag so that 
  ** SQLite does not do an external sort.
  */
  यदि( ( pIdxInfo->nOrderBy==1
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

  प्रत्यागम SQLITE_OK;
}

/*
** Open a new DBSTAT cursor.
*/
स्थिर पूर्णाङ्क statOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  StatTable *pTab = (StatTable *)pVTab;
  StatCursor *pCsr;

  pCsr = (StatCursor *)sqlite3_malloc64(परिमाण(StatCursor));
  यदि( pCsr==0 ){
    प्रत्यागम SQLITE_NOMEM_BKPT;
  }अन्यथा{
    memset(pCsr, 0, परिमाण(StatCursor));
    pCsr->base.pVtab = pVTab;
    pCsr->iDb = pTab->iDb;
  }

  *ppCursor = (sqlite3_vtab_cursor *)pCsr;
  प्रत्यागम SQLITE_OK;
}

स्थिर शून्य statClearCells(StatPage *p){
  पूर्णाङ्क i;
  यदि( p->aCell ){
    प्रति(i=0; i<p->nCell; i++){
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
  memset(p, 0, परिमाण(StatPage));
  p->aPg = aPg;
}

स्थिर शून्य statResetCsr(StatCursor *pCsr){
  पूर्णाङ्क i;
  /* In some circumstances, specifically if an OOM has occurred, the call
  ** to sqlite3_reset() may cause the pager to be reset (emptied). It is
  ** important that statClearPage() is called to free any page refs before
  ** this happens. dbsqlfuzz 9ed3e4e3816219d3509d711636c38542bf3f40b1. */
  प्रति(i=0; i<ArraySize(pCsr->aPage); i++){
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
स्थिर पूर्णाङ्क statClose(sqlite3_vtab_cursor *pCursor){
  StatCursor *pCsr = (StatCursor *)pCursor;
  statResetCsr(pCsr);
  sqlite3_finalize(pCsr->pStmt);
  sqlite3_free(pCsr);
  प्रत्यागम SQLITE_OK;
}

/*
** For a single cell on a btree page, compute the number of bytes of
** content (payload) stored on that page.  That is to say, compute the
** number of bytes of content not found on overflow pages.
*/
स्थिर पूर्णाङ्क getLocalPayload(
  पूर्णाङ्क nUsable,                    /* Usable bytes per page */
  u8 flags,                       /* Page flags */
  पूर्णाङ्क nTotal                      /* Total record (payload) size */
){
  पूर्णाङ्क nLocal;
  पूर्णाङ्क nMinLocal;
  पूर्णाङ्क nMaxLocal;
 
  यदि( flags==0x0D ){              /* Table leaf node */
    nMinLocal = (nUsable - 12) * 32 / 255 - 23;
    nMaxLocal = nUsable - 35;
  }अन्यथा{                          /* Index interior and leaf nodes */
    nMinLocal = (nUsable - 12) * 32 / 255 - 23;
    nMaxLocal = (nUsable - 12) * 64 / 255 - 23;
  }

  nLocal = nMinLocal + (nTotal - nMinLocal) % (nUsable - 4);
  यदि( nLocal>nMaxLocal ) nLocal = nMinLocal;
  प्रत्यागम nLocal;
}

/* Populate the StatPage object with information about the all
** cells found on the page currently under analysis.
*/
स्थिर पूर्णाङ्क statDecodePage(Btree *pBt, StatPage *p){
  पूर्णाङ्क nUnused;
  पूर्णाङ्क iOff;
  पूर्णाङ्क nHdr;
  पूर्णाङ्क isLeaf;
  पूर्णाङ्क szPage;

  u8 *aData = p->aPg;
  u8 *aHdr = &aData[p->iPgno==1 ? 100 : 0];

  p->flags = aHdr[0];
  यदि( p->flags==0x0A || p->flags==0x0D ){
    isLeaf = 1;
    nHdr = 8;
  }अन्यथा यदि( p->flags==0x05 || p->flags==0x02 ){
    isLeaf = 0;
    nHdr = 12;
  }अन्यथा{
    गच्छ statPageIsCorrupt;
  }
  यदि( p->iPgno==1 ) nHdr += 100;
  p->nCell = get2byte(&aHdr[3]);
  p->nMxPayload = 0;
  szPage = sqlite3BtreeGetPageSize(pBt);

  nUnused = get2byte(&aHdr[5]) - nHdr - 2*p->nCell;
  nUnused += (पूर्णाङ्क)aHdr[7];
  iOff = get2byte(&aHdr[1]);
  यावत्( iOff ){
    पूर्णाङ्क iNext;
    यदि( iOff>=szPage ) गच्छ statPageIsCorrupt;
    nUnused += get2byte(&aData[iOff+2]);
    iNext = get2byte(&aData[iOff]);
    यदि( iNext<iOff+4 && iNext>0 ) गच्छ statPageIsCorrupt;
    iOff = iNext;
  }
  p->nUnused = nUnused;
  p->i_उजवा_Child_Pg = isLeaf ? 0 : sqlite3Get4byte(&aHdr[8]);

  यदि( p->nCell ){
    पूर्णाङ्क i;                        /* Used to iterate through cells */
    पूर्णाङ्क nUsable;                  /* Usable bytes per page */

    sqlite3BtreeEnter(pBt);
    nUsable = szPage - sqlite3BtreeGetReserveNoMutex(pBt);
    sqlite3BtreeLeave(pBt);
    p->aCell = sqlite3_malloc64((p->nCell+1) * परिमाण(StatCell));
    यदि( p->aCell==0 ) प्रत्यागम SQLITE_NOMEM_BKPT;
    memset(p->aCell, 0, (p->nCell+1) * परिमाण(StatCell));

    प्रति(i=0; i<p->nCell; i++){
      StatCell *pCell = &p->aCell[i];

      iOff = get2byte(&aData[nHdr+i*2]);
      यदि( iOff<nHdr || iOff>=szPage ) गच्छ statPageIsCorrupt;
      यदि( !isLeaf ){
        pCell->iChildPg = sqlite3Get4byte(&aData[iOff]);
        iOff += 4;
      }
      यदि( p->flags==0x05 ){
        /* A table interior node. nPayload==0. */
      }अन्यथा{
        u32 nPayload;             /* Bytes of payload total (local+overflow) */
        पूर्णाङ्क nLocal;               /* Bytes of payload stored locally */
        iOff += getVarint32(&aData[iOff], nPayload);
        यदि( p->flags==0x0D ){
          u64 dummy;
          iOff += sqlite3GetVarint(&aData[iOff], &dummy);
        }
        यदि( nPayload>(u32)p->nMxPayload ) p->nMxPayload = nPayload;
        nLocal = getLocalPayload(nUsable, p->flags, nPayload);
        यदि( nLocal<0 ) गच्छ statPageIsCorrupt;
        pCell->nLocal = nLocal;
        प्रतिज्ञा( nPayload>=(u32)nLocal );
        प्रतिज्ञा( nLocal<=(nUsable-35) );
        यदि( nPayload>(u32)nLocal ){
          पूर्णाङ्क j;
          पूर्णाङ्क nOvfl = ((nPayload - nLocal) + nUsable-4 - 1) / (nUsable - 4);
          यदि( iOff+nLocal+4>nUsable || nPayload>0x7fffffff ){
            गच्छ statPageIsCorrupt;
          }
          pCell->nLastOvfl = (nPayload-nLocal) - (nOvfl-1) * (nUsable-4);
          pCell->nOvfl = nOvfl;
          pCell->aOvfl = sqlite3_malloc64(परिमाण(u32)*nOvfl);
          यदि( pCell->aOvfl==0 ) प्रत्यागम SQLITE_NOMEM_BKPT;
          pCell->aOvfl[0] = sqlite3Get4byte(&aData[iOff+nLocal]);
          प्रति(j=1; j<nOvfl; j++){
            पूर्णाङ्क rc;
            u32 iPrev = pCell->aOvfl[j-1];
            DbPage *pPg = 0;
            rc = sqlite3PagerGet(sqlite3BtreePager(pBt), iPrev, &pPg, 0);
            यदि( rc!=SQLITE_OK ){
              प्रतिज्ञा( pPg==0 );
              प्रत्यागम rc;
            } 
            pCell->aOvfl[j] = sqlite3Get4byte(sqlite3PagerGetData(pPg));
            sqlite3PagerUnref(pPg);
          }
        }
      }
    }
  }

  प्रत्यागम SQLITE_OK;

statPageIsCorrupt:
  p->flags = 0;
  statClearCells(p);
  प्रत्यागम SQLITE_OK;
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
  यदि( sqlite3OsFileControl(fd, 230440, &x)==SQLITE_OK ){
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
स्थिर पूर्णाङ्क statGetPage(
  Btree *pBt,                     /* Load page from this b-tree */
  u32 iPg,                        /* Page number to load */
  StatPage *pPg                   /* Load page into this object */
){
  पूर्णाङ्क pgsz = sqlite3BtreeGetPageSize(pBt);
  DbPage *pDbPage = 0;
  पूर्णाङ्क rc;

  यदि( pPg->aPg==0 ){
    pPg->aPg = (u8*)sqlite3_malloc(pgsz + DBSTAT_PAGE_PADDING_BYTES);
    यदि( pPg->aPg==0 ){
      प्रत्यागम SQLITE_NOMEM_BKPT;
    }
    memset(&pPg->aPg[pgsz], 0, DBSTAT_PAGE_PADDING_BYTES);
  }

  rc = sqlite3PagerGet(sqlite3BtreePager(pBt), iPg, &pDbPage, 0);
  यदि( rc==SQLITE_OK ){
    ध्रुव u8 *a = sqlite3PagerGetData(pDbPage);
    memcpy(pPg->aPg, a, pgsz);
    sqlite3PagerUnref(pDbPage);
  }

  प्रत्यागम rc;
}

/*
** Move a DBSTAT cursor to the next entry.  Normally, the next
** entry will be the next page, but in aggregated mode (pCsr->isAgg!=0),
** the next entry is the next btree.
*/
स्थिर पूर्णाङ्क statNext(sqlite3_vtab_cursor *pCursor){
  पूर्णाङ्क rc;
  पूर्णाङ्क nPayload;
  वर्ण *z;
  StatCursor *pCsr = (StatCursor *)pCursor;
  StatTable *pTab = (StatTable *)pCursor->pVtab;
  Btree *pBt = pTab->db->aDb[pCsr->iDb].pBt;
  Pager *pPager = sqlite3BtreePager(pBt);

  sqlite3_free(pCsr->zPath);
  pCsr->zPath = 0;

statNextRestart:
  यदि( pCsr->iPage<0 ){
    /* Start measuring space on the next btree */
    statResetCounts(pCsr);
    rc = sqlite3_step(pCsr->pStmt);
    यदि( rc==SQLITE_ROW ){
      पूर्णाङ्क nPage;
      u32 i_मूळ = (u32)sqlite3_column_int64(pCsr->pStmt, 1);
      sqlite3PagerPagecount(pPager, &nPage);
      यदि( nPage==0 ){
        pCsr->isEof = 1;
        प्रत्यागम sqlite3_reset(pCsr->pStmt);
      }
      rc = statGetPage(pBt, i_मूळ, &pCsr->aPage[0]);
      pCsr->aPage[0].iPgno = i_मूळ;
      pCsr->aPage[0].iCell = 0;
      यदि( !pCsr->isAgg ){
        pCsr->aPage[0].zPath = z = sqlite3_mprintf("/");
        यदि( z==0 ) rc = SQLITE_NOMEM_BKPT;
      }
      pCsr->iPage = 0;
      pCsr->nPage = 1;
    }अन्यथा{
      pCsr->isEof = 1;
      प्रत्यागम sqlite3_reset(pCsr->pStmt);
    }
  }अन्यथा{
    /* Continue analyzing the btree previously started */
    StatPage *p = &pCsr->aPage[pCsr->iPage];
    यदि( !pCsr->isAgg ) statResetCounts(pCsr);
    यावत्( p->iCell<p->nCell ){
      StatCell *pCell = &p->aCell[p->iCell];
      यावत्( pCell->iOvfl<pCell->nOvfl ){
        पूर्णाङ्क nUsable;
        पूर्णाङ्क iOvfl = pCell->iOvfl;
        sqlite3BtreeEnter(pBt);
        nUsable = sqlite3BtreeGetPageSize(pBt) - 
                        sqlite3BtreeGetReserveNoMutex(pBt);
        sqlite3BtreeLeave(pBt);
        pCsr->nPage++;
        pCsr->iPageno = pCell->aOvfl[iOvfl];
        statSizeAndOffset(pCsr);
        यदि( pCell->iOvfl<pCell->nOvfl-1 ){
          pCsr->nPayload += nUsable - 4;
        }अन्यथा{
          pCsr->nPayload += pCell->nLastOvfl;
          pCsr->nUnused += nUsable - 4 - pCell->nLastOvfl;
        }
        pCell->iOvfl++;
        यदि( !pCsr->isAgg ){
          pCsr->zName = (वर्ण *)sqlite3_column_text(pCsr->pStmt, 0);
          pCsr->zPagetype = "overflow";
          pCsr->zPath = z = sqlite3_mprintf(
              "%s%.3x+%.6x", p->zPath, p->iCell, iOvfl
          );
          प्रत्यागम z==0 ? SQLITE_NOMEM_BKPT : SQLITE_OK;
        }
      }
      यदि( p->i_उजवा_Child_Pg ) भङ्ग;
      p->iCell++;
    }

    यदि( !p->i_उजवा_Child_Pg || p->iCell>p->nCell ){
      statClearPage(p);
      pCsr->iPage--;
      यदि( pCsr->isAgg && pCsr->iPage<0 ){
        /* label-statNext-done:  When computing aggregate space usage over
        ** an entire btree, this is the exit point from this function */
        प्रत्यागम SQLITE_OK;
      }
      गच्छ statNextRestart; /* Tail recursion */
    }
    pCsr->iPage++;
    यदि( pCsr->iPage>=ArraySize(pCsr->aPage) ){
      statResetCsr(pCsr);
      प्रत्यागम SQLITE_CORRUPT_BKPT;
    }
    प्रतिज्ञा( p==&pCsr->aPage[pCsr->iPage-1] );

    यदि( p->iCell==p->nCell ){
      p[1].iPgno = p->i_उजवा_Child_Pg;
    }अन्यथा{
      p[1].iPgno = p->aCell[p->iCell].iChildPg;
    }
    rc = statGetPage(pBt, p[1].iPgno, &p[1]);
    pCsr->nPage++;
    p[1].iCell = 0;
    यदि( !pCsr->isAgg ){
      p[1].zPath = z = sqlite3_mprintf("%s%.3x/", p->zPath, p->iCell);
      यदि( z==0 ) rc = SQLITE_NOMEM_BKPT;
    }
    p->iCell++;
  }


  /* Populate the StatCursor fields with the values to be returned
  ** by the xColumn() and xRowid() methods.
  */
  यदि( rc==SQLITE_OK ){
    पूर्णाङ्क i;
    StatPage *p = &pCsr->aPage[pCsr->iPage];
    pCsr->zName = (वर्ण *)sqlite3_column_text(pCsr->pStmt, 0);
    pCsr->iPageno = p->iPgno;

    rc = statDecodePage(pBt, p);
    यदि( rc==SQLITE_OK ){
      statSizeAndOffset(pCsr);

      विकल्प( p->flags ){
        पक्ष 0x05:             /* table internal */
        पक्ष 0x02:             /* index internal */
          pCsr->zPagetype = "internal";
          भङ्ग;
        पक्ष 0x0D:             /* table leaf */
        पक्ष 0x0A:             /* index leaf */
          pCsr->zPagetype = "leaf";
          भङ्ग;
        स्वभाव:
          pCsr->zPagetype = "corrupted";
          भङ्ग;
      }
      pCsr->nCell += p->nCell;
      pCsr->nUnused += p->nUnused;
      यदि( p->nMxPayload>pCsr->nMxPayload ) pCsr->nMxPayload = p->nMxPayload;
      यदि( !pCsr->isAgg ){
        pCsr->zPath = z = sqlite3_mprintf("%s", p->zPath);
        यदि( z==0 ) rc = SQLITE_NOMEM_BKPT;
      }
      nPayload = 0;
      प्रति(i=0; i<p->nCell; i++){
        nPayload += p->aCell[i].nLocal;
      }
      pCsr->nPayload += nPayload;

      /* If computing aggregate space usage by btree, continue with the
      ** next page.  The loop will exit via the return at label-statNext-done
      */
      यदि( pCsr->isAgg ) गच्छ statNextRestart;
    }
  }

  प्रत्यागम rc;
}

स्थिर पूर्णाङ्क statEof(sqlite3_vtab_cursor *pCursor){
  StatCursor *pCsr = (StatCursor *)pCursor;
  प्रत्यागम pCsr->isEof;
}

/* Initialize a cursor according to the query plan idxNum using the
** arguments in argv[0].  See statBestIndex() for a description of the
** meaning of the bits in idxNum.
*/
स्थिर पूर्णाङ्क statFilter(
  sqlite3_vtab_cursor *pCursor, 
  पूर्णाङ्क idx_अंक, ध्रुव वर्ण *idxStr,
  पूर्णाङ्क argc, sqlite3_मूल्य **argv
){
  StatCursor *pCsr = (StatCursor *)pCursor;
  StatTable *pTab = (StatTable*)(pCursor->pVtab);
  sqlite3_str *pSql;      /* Query of btrees to analyze */
  वर्ण *zSql;             /* String value of pSql */
  पूर्णाङ्क iArg = 0;           /* Count of argv[] parameters used so far */
  पूर्णाङ्क rc = SQLITE_OK;     /* Result of this operation */
  ध्रुव वर्ण *zName = 0;  /* Only provide analysis of this table */
  (शून्य)argc;
  (शून्य)idxStr;

  statResetCsr(pCsr);
  sqlite3_finalize(pCsr->pStmt);
  pCsr->pStmt = 0;
  यदि( idx_अंक & 0x01 ){
    /* schema=? constraint is present.  Get its value */
    ध्रुव वर्ण *zDbase = (ध्रुव वर्ण*)sqlite3_मूल्य_text(argv[iArg++]);
    pCsr->iDb = sqlite3FindDbName(pTab->db, zDbase);
    यदि( pCsr->iDb<0 ){
      pCsr->iDb = 0;
      pCsr->isEof = 1;
      प्रत्यागम SQLITE_OK;
    }
  }अन्यथा{
    pCsr->iDb = pTab->iDb;
  }
  यदि( idx_अंक & 0x02 ){
    /* name=? constraint is present */
    zName = (ध्रुव वर्ण*)sqlite3_मूल्य_text(argv[iArg++]);
  }
  यदि( idx_अंक & 0x04 ){
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
  यदि( zName ){
    sqlite3_str_appendf(pSql, "WHERE name=%Q", zName);
  }
  यदि( idx_अंक & 0x08 ){
    sqlite3_str_appendf(pSql, " ORDER BY name");
  }
  zSql = sqlite3_str_finish(pSql);
  यदि( zSql==0 ){
    प्रत्यागम SQLITE_NOMEM_BKPT;
  }अन्यथा{
    rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pStmt, 0);
    sqlite3_free(zSql);
  }

  यदि( rc==SQLITE_OK ){
    pCsr->iPage = -1;
    rc = statNext(pCursor);
  }
  प्रत्यागम rc;
}

स्थिर पूर्णाङ्क statColumn(
  sqlite3_vtab_cursor *pCursor, 
  sqlite3_context *ctx, 
  पूर्णाङ्क i
){
  StatCursor *pCsr = (StatCursor *)pCursor;
  विकल्प( i ){
    पक्ष 0:            /* name */
      sqlite3_result_text(ctx, pCsr->zName, -1, SQLITE_TRANSIENT);
      भङ्ग;
    पक्ष 1:            /* path */
      यदि( !pCsr->isAgg ){
        sqlite3_result_text(ctx, pCsr->zPath, -1, SQLITE_TRANSIENT);
      }
      भङ्ग;
    पक्ष 2:            /* pageno */
      यदि( pCsr->isAgg ){
        sqlite3_result_int64(ctx, pCsr->nPage);
      }अन्यथा{
        sqlite3_result_int64(ctx, pCsr->iPageno);
      }
      भङ्ग;
    पक्ष 3:            /* pagetype */
      यदि( !pCsr->isAgg ){
        sqlite3_result_text(ctx, pCsr->zPagetype, -1, SQLITE_STATIC);
      }
      भङ्ग;
    पक्ष 4:            /* ncell */
      sqlite3_result_int64(ctx, pCsr->nCell);
      भङ्ग;
    पक्ष 5:            /* payload */
      sqlite3_result_int64(ctx, pCsr->nPayload);
      भङ्ग;
    पक्ष 6:            /* unused */
      sqlite3_result_int64(ctx, pCsr->nUnused);
      भङ्ग;
    पक्ष 7:            /* mx_payload */
      sqlite3_result_int64(ctx, pCsr->nMxPayload);
      भङ्ग;
    पक्ष 8:            /* pgoffset */
      यदि( !pCsr->isAgg ){
        sqlite3_result_int64(ctx, pCsr->iOffset);
      }
      भङ्ग;
    पक्ष 9:            /* pgsize */
      sqlite3_result_int64(ctx, pCsr->szPage);
      भङ्ग;
    पक्ष 10: {         /* schema */
      sqlite3 *db = sqlite3_context_db_handle(ctx);
      पूर्णाङ्क iDb = pCsr->iDb;
      sqlite3_result_text(ctx, db->aDb[iDb].zDbSName, -1, SQLITE_STATIC);
      भङ्ग;
    }
    स्वभाव: {         /* aggregate */
      sqlite3_result_int(ctx, pCsr->isAgg);
      भङ्ग;
    }
  }
  प्रत्यागम SQLITE_OK;
}

स्थिर पूर्णाङ्क statRowid(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid){
  StatCursor *pCsr = (StatCursor *)pCursor;
  *pRowid = pCsr->iPageno;
  प्रत्यागम SQLITE_OK;
}

/*
** Invoke this routine to register the "dbstat" virtual table module
*/
पूर्णाङ्क sqlite3DbstatRegister(sqlite3 *db){
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
  प्रत्यागम sqlite3_create_module(db, "dbstat", &dbstat_module, 0);
}
#नोचेद्यदि defined(SQLITE_ENABLE_DBSTAT_VTAB)
पूर्णाङ्क sqlite3DbstatRegister(sqlite3 *db){ प्रत्यागम SQLITE_OK; }
#endif /* SQLITE_ENABLE_DBSTAT_VTAB */
