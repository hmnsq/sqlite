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

#অন্তর্ভাব "sqliteInt.h"   /* Requires access to internal data structures */
#যদি (defined(SQLITE_ENABLE_DBSTAT_VTAB) || defined(SQLITE_TEST)) \
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
স্থির ধ্রুবক অক্ষর zDbstatSchema[] = 
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
প্রকারলেখ কাঠামো StatTable StatTable;
প্রকারলেখ কাঠামো StatCursor StatCursor;
প্রকারলেখ কাঠামো StatPage StatPage;
প্রকারলেখ কাঠামো StatCell StatCell;

/* Size information for a single cell within a btree page */
কাঠামো StatCell {
  পূর্ণসংখ্যা nLocal;                     /* Bytes of local payload */
  u32 iChildPg;                   /* Child node (or 0 if this is a leaf) */
  পূর্ণসংখ্যা nOvfl;                      /* Entries in aOvfl[] */
  u32 *aOvfl;                     /* Array of overflow page numbers */
  পূর্ণসংখ্যা nLastOvfl;                  /* Bytes of payload on final overflow page */
  পূর্ণসংখ্যা iOvfl;                      /* Iterates through aOvfl[] */
};

/* Size information for a single btree page */
কাঠামো StatPage {
  u32 iPgno;                      /* Page number */
  u8 *aPg;                        /* Page buffer from sqlite3_malloc() */
  পূর্ণসংখ্যা iCell;                      /* Current cell */
  অক্ষর *zPath;                    /* Path to this page */

  /* Variables populated by statDecodePage(): */
  u8 flags;                       /* Copy of flags byte */
  পূর্ণসংখ্যা nCell;                      /* Number of cells on page */
  পূর্ণসংখ্যা nUnused;                    /* Number of unused bytes on page */
  StatCell *aCell;                /* Array of parsed cells */
  u32 i_ডান_Child_Pg;              /* Right-child page number (or 0) */
  পূর্ণসংখ্যা nMxPayload;                 /* Largest payload of any cell on the page */
};

/* The cursor for scanning the dbstat virtual table */
কাঠামো StatCursor {
  sqlite3_vtab_cursor base;       /* base class.  MUST BE FIRST! */
  sqlite3_stmt *pStmt;            /* Iterates through set of root pages */
  u8 isEof;                       /* After pStmt has returned SQLITE_DONE */
  u8 isAgg;                       /* Aggregate results for each table */
  পূর্ণসংখ্যা iDb;                        /* Schema used for this query */

  StatPage aPage[32];             /* Pages in path to current page */
  পূর্ণসংখ্যা iPage;                      /* Current entry in aPage[] */

  /* Values to return. */
  u32 iPageno;                    /* Value of 'pageno' column */
  অক্ষর *zName;                    /* Value of 'name' column */
  অক্ষর *zPath;                    /* Value of 'path' column */
  অক্ষর *zPagetype;                /* Value of 'pagetype' column */
  পূর্ণসংখ্যা nPage;                      /* Number of pages in current btree */
  পূর্ণসংখ্যা nCell;                      /* Value of 'ncell' column */
  পূর্ণসংখ্যা nMxPayload;                 /* Value of 'mx_payload' column */
  i64 nUnused;                    /* Value of 'unused' column */
  i64 nPayload;                   /* Value of 'payload' column */
  i64 iOffset;                    /* Value of 'pgOffset' column */
  i64 szPage;                     /* Value of 'pgSize' column */
};

/* An instance of the DBSTAT virtual table */
কাঠামো StatTable {
  sqlite3_vtab base;              /* base class.  MUST BE FIRST! */
  sqlite3 *db;                    /* Database connection that owns this vtab */
  পূর্ণসংখ্যা iDb;                        /* Index of database to analyze */
};

#ifndef get2byte
# define get2byte(x)   ((x)[0]<<8 | (x)[1])
#endif

/*
** Connect to or create a new DBSTAT virtual table.
*/
স্থির পূর্ণসংখ্যা statConnect(
  sqlite3 *db,
  শূন্য *pAux,
  পূর্ণসংখ্যা argc, ধ্রুবক অক্ষর *ধ্রুবক*argv,
  sqlite3_vtab **ppVtab,
  অক্ষর **pzErr
){
  StatTable *pTab = 0;
  পূর্ণসংখ্যা rc = SQLITE_OK;
  পূর্ণসংখ্যা iDb;
  (শূন্য)pAux;

  যদি( argc>=4 ){
    Token nm;
    sqlite3TokenInit(&nm, (অক্ষর*)argv[3]);
    iDb = sqlite3FindDb(db, &nm);
    যদি( iDb<0 ){
      *pzErr = sqlite3_mprintf("no such database: %s", argv[3]);
      ফেরত SQLITE_ERROR;
    }
  }অন্যথা{
    iDb = 0;
  }
  sqlite3_vtab_config(db, SQLITE_VTAB_DIRECTONLY);
  rc = sqlite3_declare_vtab(db, zDbstatSchema);
  যদি( rc==SQLITE_OK ){
    pTab = (StatTable *)sqlite3_malloc64(আকার(StatTable));
    যদি( pTab==0 ) rc = SQLITE_NOMEM_BKPT;
  }

  প্রতিজ্ঞা( rc==SQLITE_OK || pTab==0 );
  যদি( rc==SQLITE_OK ){
    memset(pTab, 0, আকার(StatTable));
    pTab->db = db;
    pTab->iDb = iDb;
  }

  *ppVtab = (sqlite3_vtab*)pTab;
  ফেরত rc;
}

/*
** Disconnect from or destroy the DBSTAT virtual table.
*/
স্থির পূর্ণসংখ্যা statDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  ফেরত SQLITE_OK;
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
স্থির পূর্ণসংখ্যা stat_Best_সূচক(sqlite3_vtab *tab, sqlite3_সূচক_info *pIdxInfo){
  পূর্ণসংখ্যা i;
  পূর্ণসংখ্যা iSchema = -1;
  পূর্ণসংখ্যা iName = -1;
  পূর্ণসংখ্যা iAgg = -1;
  (শূন্য)tab;

  /* Look for a valid schema=? constraint.  If found, change the idxNum to
  ** 1 and request the value of that constraint be sent to xFilter.  And
  ** lower the cost estimate to encourage the constrained version to be
  ** used.
  */
  জন্য(i=0; i<pIdxInfo->nConstraint; i++){
    যদি( pIdxInfo->aConstraint[i].op!=SQLITE_সূচক_CONSTRAINT_EQ ) চালু;
    যদি( pIdxInfo->aConstraint[i].usable==0 ){
      /* Force DBSTAT table should always be the right-most table in a join */
      ফেরত SQLITE_CONSTRAINT;
    }
    নির্বাচন( pIdxInfo->aConstraint[i].iColumn ){
      ক্ষেত্র 0: {    /* name */
        iName = i;
        বিরতি;
      }
      ক্ষেত্র 10: {   /* schema */
        iSchema = i;
        বিরতি;
      }
      ক্ষেত্র 11: {   /* aggregate */
        iAgg = i;
        বিরতি;
      }
    }
  }
  i = 0;
  যদি( iSchema>=0 ){
    pIdxInfo->aConstraintUsage[iSchema].argv_সূচক = ++i;
    pIdxInfo->aConstraintUsage[iSchema].omit = 1;
    pIdxInfo->idx_অঙ্ক |= 0x01;
  }
  যদি( iName>=0 ){
    pIdxInfo->aConstraintUsage[iName].argv_সূচক = ++i;
    pIdxInfo->idx_অঙ্ক |= 0x02;
  }
  যদি( iAgg>=0 ){
    pIdxInfo->aConstraintUsage[iAgg].argv_সূচক = ++i;
    pIdxInfo->idx_অঙ্ক |= 0x04;
  }
  pIdxInfo->estimatedCost = 1.0;

  /* Records are always returned in ascending order of (name, path). 
  ** If this will satisfy the client, set the orderByConsumed flag so that 
  ** SQLite does not do an external sort.
  */
  যদি( ( pIdxInfo->nOrderBy==1
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
    pIdxInfo->idx_অঙ্ক |= 0x08;
  }
  pIdxInfo->idxFlags |= SQLITE_সূচক_SCAN_HEX;

  ফেরত SQLITE_OK;
}

/*
** Open a new DBSTAT cursor.
*/
স্থির পূর্ণসংখ্যা statOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  StatTable *pTab = (StatTable *)pVTab;
  StatCursor *pCsr;

  pCsr = (StatCursor *)sqlite3_malloc64(আকার(StatCursor));
  যদি( pCsr==0 ){
    ফেরত SQLITE_NOMEM_BKPT;
  }অন্যথা{
    memset(pCsr, 0, আকার(StatCursor));
    pCsr->base.pVtab = pVTab;
    pCsr->iDb = pTab->iDb;
  }

  *ppCursor = (sqlite3_vtab_cursor *)pCsr;
  ফেরত SQLITE_OK;
}

স্থির শূন্য statClearCells(StatPage *p){
  পূর্ণসংখ্যা i;
  যদি( p->aCell ){
    জন্য(i=0; i<p->nCell; i++){
      sqlite3_free(p->aCell[i].aOvfl);
    }
    sqlite3_free(p->aCell);
  }
  p->nCell = 0;
  p->aCell = 0;
}

স্থির শূন্য statClearPage(StatPage *p){
  u8 *aPg = p->aPg;
  statClearCells(p);
  sqlite3_free(p->zPath);
  memset(p, 0, আকার(StatPage));
  p->aPg = aPg;
}

স্থির শূন্য statResetCsr(StatCursor *pCsr){
  পূর্ণসংখ্যা i;
  /* In some circumstances, specifically if an OOM has occurred, the call
  ** to sqlite3_reset() may cause the pager to be reset (emptied). It is
  ** important that statClearPage() is called to free any page refs before
  ** this happens. dbsqlfuzz 9ed3e4e3816219d3509d711636c38542bf3f40b1. */
  জন্য(i=0; i<Array_আকার(pCsr->aPage); i++){
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
স্থির শূন্য statResetCounts(StatCursor *pCsr){
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
স্থির পূর্ণসংখ্যা statClose(sqlite3_vtab_cursor *pCursor){
  StatCursor *pCsr = (StatCursor *)pCursor;
  statResetCsr(pCsr);
  sqlite3_finalize(pCsr->pStmt);
  sqlite3_free(pCsr);
  ফেরত SQLITE_OK;
}

/*
** For a single cell on a btree page, compute the number of bytes of
** content (payload) stored on that page.  That is to say, compute the
** number of bytes of content not found on overflow pages.
*/
স্থির পূর্ণসংখ্যা getLocalPayload(
  পূর্ণসংখ্যা nUsable,                    /* Usable bytes per page */
  u8 flags,                       /* Page flags */
  পূর্ণসংখ্যা n_যোগ                      /* Total record (payload) size */
){
  পূর্ণসংখ্যা nLocal;
  পূর্ণসংখ্যা nMinLocal;
  পূর্ণসংখ্যা nMaxLocal;
 
  যদি( flags==0x0D ){              /* Table leaf node */
    nMinLocal = (nUsable - 12) * 32 / 255 - 23;
    nMaxLocal = nUsable - 35;
  }অন্যথা{                          /* Index interior and leaf nodes */
    nMinLocal = (nUsable - 12) * 32 / 255 - 23;
    nMaxLocal = (nUsable - 12) * 64 / 255 - 23;
  }

  nLocal = nMinLocal + (n_যোগ - nMinLocal) % (nUsable - 4);
  যদি( nLocal>nMaxLocal ) nLocal = nMinLocal;
  ফেরত nLocal;
}

/* Populate the StatPage object with information about the all
** cells found on the page currently under analysis.
*/
স্থির পূর্ণসংখ্যা statDecodePage(Btree *pBt, StatPage *p){
  পূর্ণসংখ্যা nUnused;
  পূর্ণসংখ্যা iOff;
  পূর্ণসংখ্যা nHdr;
  পূর্ণসংখ্যা isLeaf;
  পূর্ণসংখ্যা szPage;

  u8 *a_তথ্য = p->aPg;
  u8 *aHdr = &a_তথ্য[p->iPgno==1 ? 100 : 0];

  p->flags = aHdr[0];
  যদি( p->flags==0x0A || p->flags==0x0D ){
    isLeaf = 1;
    nHdr = 8;
  }অন্যথা যদি( p->flags==0x05 || p->flags==0x02 ){
    isLeaf = 0;
    nHdr = 12;
  }অন্যথা{
    যাও statPageIsCorrupt;
  }
  যদি( p->iPgno==1 ) nHdr += 100;
  p->nCell = get2byte(&aHdr[3]);
  p->nMxPayload = 0;
  szPage = sqlite3_Btree_Get_Page_আকার(pBt);

  nUnused = get2byte(&aHdr[5]) - nHdr - 2*p->nCell;
  nUnused += (পূর্ণসংখ্যা)aHdr[7];
  iOff = get2byte(&aHdr[1]);
  যখন( iOff ){
    পূর্ণসংখ্যা i_পরবর্তী;
    যদি( iOff>=szPage ) যাও statPageIsCorrupt;
    nUnused += get2byte(&a_তথ্য[iOff+2]);
    i_পরবর্তী = get2byte(&a_তথ্য[iOff]);
    যদি( i_পরবর্তী<iOff+4 && i_পরবর্তী>0 ) যাও statPageIsCorrupt;
    iOff = i_পরবর্তী;
  }
  p->nUnused = nUnused;
  p->i_ডান_Child_Pg = isLeaf ? 0 : sqlite3Get4byte(&aHdr[8]);

  যদি( p->nCell ){
    পূর্ণসংখ্যা i;                        /* Used to iterate through cells */
    পূর্ণসংখ্যা nUsable;                  /* Usable bytes per page */

    sqlite3BtreeEnter(pBt);
    nUsable = szPage - sqlite3BtreeGetReserveNoMutex(pBt);
    sqlite3BtreeLeave(pBt);
    p->aCell = sqlite3_malloc64((p->nCell+1) * আকার(StatCell));
    যদি( p->aCell==0 ) ফেরত SQLITE_NOMEM_BKPT;
    memset(p->aCell, 0, (p->nCell+1) * আকার(StatCell));

    জন্য(i=0; i<p->nCell; i++){
      StatCell *pCell = &p->aCell[i];

      iOff = get2byte(&a_তথ্য[nHdr+i*2]);
      যদি( iOff<nHdr || iOff>=szPage ) যাও statPageIsCorrupt;
      যদি( !isLeaf ){
        pCell->iChildPg = sqlite3Get4byte(&a_তথ্য[iOff]);
        iOff += 4;
      }
      যদি( p->flags==0x05 ){
        /* A table interior node. nPayload==0. */
      }অন্যথা{
        u32 nPayload;             /* Bytes of payload total (local+overflow) */
        পূর্ণসংখ্যা nLocal;               /* Bytes of payload stored locally */
        iOff += getVarint32(&a_তথ্য[iOff], nPayload);
        যদি( p->flags==0x0D ){
          u64 dummy;
          iOff += sqlite3GetVarint(&a_তথ্য[iOff], &dummy);
        }
        যদি( nPayload>(u32)p->nMxPayload ) p->nMxPayload = nPayload;
        nLocal = getLocalPayload(nUsable, p->flags, nPayload);
        যদি( nLocal<0 ) যাও statPageIsCorrupt;
        pCell->nLocal = nLocal;
        প্রতিজ্ঞা( nPayload>=(u32)nLocal );
        প্রতিজ্ঞা( nLocal<=(nUsable-35) );
        যদি( nPayload>(u32)nLocal ){
          পূর্ণসংখ্যা j;
          পূর্ণসংখ্যা nOvfl = ((nPayload - nLocal) + nUsable-4 - 1) / (nUsable - 4);
          যদি( iOff+nLocal+4>nUsable || nPayload>0x7fffffff ){
            যাও statPageIsCorrupt;
          }
          pCell->nLastOvfl = (nPayload-nLocal) - (nOvfl-1) * (nUsable-4);
          pCell->nOvfl = nOvfl;
          pCell->aOvfl = sqlite3_malloc64(আকার(u32)*nOvfl);
          যদি( pCell->aOvfl==0 ) ফেরত SQLITE_NOMEM_BKPT;
          pCell->aOvfl[0] = sqlite3Get4byte(&a_তথ্য[iOff+nLocal]);
          জন্য(j=1; j<nOvfl; j++){
            পূর্ণসংখ্যা rc;
            u32 i_পূর্ব = pCell->aOvfl[j-1];
            DbPage *pPg = 0;
            rc = sqlite3PagerGet(sqlite3BtreePager(pBt), i_পূর্ব, &pPg, 0);
            যদি( rc!=SQLITE_OK ){
              প্রতিজ্ঞা( pPg==0 );
              ফেরত rc;
            } 
            pCell->aOvfl[j] = sqlite3Get4byte(sqlite3_Pager_Get_তথ্য(pPg));
            sqlite3PagerUnref(pPg);
          }
        }
      }
    }
  }

  ফেরত SQLITE_OK;

statPageIsCorrupt:
  p->flags = 0;
  statClearCells(p);
  ফেরত SQLITE_OK;
}

/*
** Populate the pCsr->iOffset and pCsr->szPage member variables. Based on
** the current value of pCsr->iPageno.
*/
স্থির শূন্য stat_আকার_And_Offset(StatCursor *pCsr){
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
  যদি( sqlite3OsFileControl(fd, 230440, &x)==SQLITE_OK ){
    pCsr->iOffset = x[0];
    pCsr->szPage += x[1];
  }অন্যথা{
    /* Not ZIPVFS: The default page size and offset */
    pCsr->szPage += sqlite3_Btree_Get_Page_আকার(pBt);
    pCsr->iOffset = (i64)pCsr->szPage * (pCsr->iPageno - 1);
  }
}

/*
** Load a copy of the page data for page iPg into the buffer belonging
** to page object pPg. Allocate the buffer if necessary. Return SQLITE_OK
** if successful, or an SQLite error code otherwise.
*/
স্থির পূর্ণসংখ্যা statGetPage(
  Btree *pBt,                     /* Load page from this b-tree */
  u32 iPg,                        /* Page number to load */
  StatPage *pPg                   /* Load page into this object */
){
  পূর্ণসংখ্যা pgsz = sqlite3_Btree_Get_Page_আকার(pBt);
  DbPage *pDbPage = 0;
  পূর্ণসংখ্যা rc;

  যদি( pPg->aPg==0 ){
    pPg->aPg = (u8*)sqlite3_malloc(pgsz + DBSTAT_PAGE_PADDING_BYTES);
    যদি( pPg->aPg==0 ){
      ফেরত SQLITE_NOMEM_BKPT;
    }
    memset(&pPg->aPg[pgsz], 0, DBSTAT_PAGE_PADDING_BYTES);
  }

  rc = sqlite3PagerGet(sqlite3BtreePager(pBt), iPg, &pDbPage, 0);
  যদি( rc==SQLITE_OK ){
    ধ্রুবক u8 *a = sqlite3_Pager_Get_তথ্য(pDbPage);
    memcpy(pPg->aPg, a, pgsz);
    sqlite3PagerUnref(pDbPage);
  }

  ফেরত rc;
}

/*
** Move a DBSTAT cursor to the next entry.  Normally, the next
** entry will be the next page, but in aggregated mode (pCsr->isAgg!=0),
** the next entry is the next btree.
*/
স্থির পূর্ণসংখ্যা stat_পরবর্তী(sqlite3_vtab_cursor *pCursor){
  পূর্ণসংখ্যা rc;
  পূর্ণসংখ্যা nPayload;
  অক্ষর *z;
  StatCursor *pCsr = (StatCursor *)pCursor;
  StatTable *pTab = (StatTable *)pCursor->pVtab;
  Btree *pBt = pTab->db->aDb[pCsr->iDb].pBt;
  Pager *pPager = sqlite3BtreePager(pBt);

  sqlite3_free(pCsr->zPath);
  pCsr->zPath = 0;

stat_পরবর্তী_Restart:
  যদি( pCsr->iPage<0 ){
    /* Start measuring space on the next btree */
    statResetCounts(pCsr);
    rc = sqlite3_step(pCsr->pStmt);
    যদি( rc==SQLITE_ROW ){
      পূর্ণসংখ্যা nPage;
      u32 i_মূল = (u32)sqlite3_column_int64(pCsr->pStmt, 1);
      sqlite3PagerPagecount(pPager, &nPage);
      যদি( nPage==0 ){
        pCsr->isEof = 1;
        ফেরত sqlite3_reset(pCsr->pStmt);
      }
      rc = statGetPage(pBt, i_মূল, &pCsr->aPage[0]);
      pCsr->aPage[0].iPgno = i_মূল;
      pCsr->aPage[0].iCell = 0;
      যদি( !pCsr->isAgg ){
        pCsr->aPage[0].zPath = z = sqlite3_mprintf("/");
        যদি( z==0 ) rc = SQLITE_NOMEM_BKPT;
      }
      pCsr->iPage = 0;
      pCsr->nPage = 1;
    }অন্যথা{
      pCsr->isEof = 1;
      ফেরত sqlite3_reset(pCsr->pStmt);
    }
  }অন্যথা{
    /* Continue analyzing the btree previously started */
    StatPage *p = &pCsr->aPage[pCsr->iPage];
    যদি( !pCsr->isAgg ) statResetCounts(pCsr);
    যখন( p->iCell<p->nCell ){
      StatCell *pCell = &p->aCell[p->iCell];
      যখন( pCell->iOvfl<pCell->nOvfl ){
        পূর্ণসংখ্যা nUsable;
        পূর্ণসংখ্যা iOvfl = pCell->iOvfl;
        sqlite3BtreeEnter(pBt);
        nUsable = sqlite3_Btree_Get_Page_আকার(pBt) - 
                        sqlite3BtreeGetReserveNoMutex(pBt);
        sqlite3BtreeLeave(pBt);
        pCsr->nPage++;
        pCsr->iPageno = pCell->aOvfl[iOvfl];
        stat_আকার_And_Offset(pCsr);
        যদি( pCell->iOvfl<pCell->nOvfl-1 ){
          pCsr->nPayload += nUsable - 4;
        }অন্যথা{
          pCsr->nPayload += pCell->nLastOvfl;
          pCsr->nUnused += nUsable - 4 - pCell->nLastOvfl;
        }
        pCell->iOvfl++;
        যদি( !pCsr->isAgg ){
          pCsr->zName = (অক্ষর *)sqlite3_column_text(pCsr->pStmt, 0);
          pCsr->zPagetype = "overflow";
          pCsr->zPath = z = sqlite3_mprintf(
              "%s%.3x+%.6x", p->zPath, p->iCell, iOvfl
          );
          ফেরত z==0 ? SQLITE_NOMEM_BKPT : SQLITE_OK;
        }
      }
      যদি( p->i_ডান_Child_Pg ) বিরতি;
      p->iCell++;
    }

    যদি( !p->i_ডান_Child_Pg || p->iCell>p->nCell ){
      statClearPage(p);
      pCsr->iPage--;
      যদি( pCsr->isAgg && pCsr->iPage<0 ){
        /* label-statNext-done:  When computing aggregate space usage over
        ** an entire btree, this is the exit point from this function */
        ফেরত SQLITE_OK;
      }
      যাও stat_পরবর্তী_Restart; /* Tail recursion */
    }
    pCsr->iPage++;
    যদি( pCsr->iPage>=Array_আকার(pCsr->aPage) ){
      statResetCsr(pCsr);
      ফেরত SQLITE_CORRUPT_BKPT;
    }
    প্রতিজ্ঞা( p==&pCsr->aPage[pCsr->iPage-1] );

    যদি( p->iCell==p->nCell ){
      p[1].iPgno = p->i_ডান_Child_Pg;
    }অন্যথা{
      p[1].iPgno = p->aCell[p->iCell].iChildPg;
    }
    rc = statGetPage(pBt, p[1].iPgno, &p[1]);
    pCsr->nPage++;
    p[1].iCell = 0;
    যদি( !pCsr->isAgg ){
      p[1].zPath = z = sqlite3_mprintf("%s%.3x/", p->zPath, p->iCell);
      যদি( z==0 ) rc = SQLITE_NOMEM_BKPT;
    }
    p->iCell++;
  }


  /* Populate the StatCursor fields with the values to be returned
  ** by the xColumn() and xRowid() methods.
  */
  যদি( rc==SQLITE_OK ){
    পূর্ণসংখ্যা i;
    StatPage *p = &pCsr->aPage[pCsr->iPage];
    pCsr->zName = (অক্ষর *)sqlite3_column_text(pCsr->pStmt, 0);
    pCsr->iPageno = p->iPgno;

    rc = statDecodePage(pBt, p);
    যদি( rc==SQLITE_OK ){
      stat_আকার_And_Offset(pCsr);

      নির্বাচন( p->flags ){
        ক্ষেত্র 0x05:             /* table internal */
        ক্ষেত্র 0x02:             /* index internal */
          pCsr->zPagetype = "internal";
          বিরতি;
        ক্ষেত্র 0x0D:             /* table leaf */
        ক্ষেত্র 0x0A:             /* index leaf */
          pCsr->zPagetype = "leaf";
          বিরতি;
        পূর্বনির্ধারিত:
          pCsr->zPagetype = "corrupted";
          বিরতি;
      }
      pCsr->nCell += p->nCell;
      pCsr->nUnused += p->nUnused;
      যদি( p->nMxPayload>pCsr->nMxPayload ) pCsr->nMxPayload = p->nMxPayload;
      যদি( !pCsr->isAgg ){
        pCsr->zPath = z = sqlite3_mprintf("%s", p->zPath);
        যদি( z==0 ) rc = SQLITE_NOMEM_BKPT;
      }
      nPayload = 0;
      জন্য(i=0; i<p->nCell; i++){
        nPayload += p->aCell[i].nLocal;
      }
      pCsr->nPayload += nPayload;

      /* If computing aggregate space usage by btree, continue with the
      ** next page.  The loop will exit via the return at label-statNext-done
      */
      যদি( pCsr->isAgg ) যাও stat_পরবর্তী_Restart;
    }
  }

  ফেরত rc;
}

স্থির পূর্ণসংখ্যা statEof(sqlite3_vtab_cursor *pCursor){
  StatCursor *pCsr = (StatCursor *)pCursor;
  ফেরত pCsr->isEof;
}

/* Initialize a cursor according to the query plan idxNum using the
** arguments in argv[0].  See statBestIndex() for a description of the
** meaning of the bits in idxNum.
*/
স্থির পূর্ণসংখ্যা statFilter(
  sqlite3_vtab_cursor *pCursor, 
  পূর্ণসংখ্যা idx_অঙ্ক, ধ্রুবক অক্ষর *idxStr,
  পূর্ণসংখ্যা argc, sqlite3_মান **argv
){
  StatCursor *pCsr = (StatCursor *)pCursor;
  StatTable *pTab = (StatTable*)(pCursor->pVtab);
  sqlite3_str *pSql;      /* Query of btrees to analyze */
  অক্ষর *zSql;             /* String value of pSql */
  পূর্ণসংখ্যা iArg = 0;           /* Count of argv[] parameters used so far */
  পূর্ণসংখ্যা rc = SQLITE_OK;     /* Result of this operation */
  ধ্রুবক অক্ষর *zName = 0;  /* Only provide analysis of this table */
  (শূন্য)argc;
  (শূন্য)idxStr;

  statResetCsr(pCsr);
  sqlite3_finalize(pCsr->pStmt);
  pCsr->pStmt = 0;
  যদি( idx_অঙ্ক & 0x01 ){
    /* schema=? constraint is present.  Get its value */
    ধ্রুবক অক্ষর *zDbase = (ধ্রুবক অক্ষর*)sqlite3_মান_text(argv[iArg++]);
    pCsr->iDb = sqlite3FindDbName(pTab->db, zDbase);
    যদি( pCsr->iDb<0 ){
      pCsr->iDb = 0;
      pCsr->isEof = 1;
      ফেরত SQLITE_OK;
    }
  }অন্যথা{
    pCsr->iDb = pTab->iDb;
  }
  যদি( idx_অঙ্ক & 0x02 ){
    /* name=? constraint is present */
    zName = (ধ্রুবক অক্ষর*)sqlite3_মান_text(argv[iArg++]);
  }
  যদি( idx_অঙ্ক & 0x04 ){
    /* aggregate=? constraint is present */
    pCsr->isAgg = sqlite3_মান_double(argv[iArg++])!=0.0;
  }অন্যথা{
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
  যদি( zName ){
    sqlite3_str_appendf(pSql, "WHERE name=%Q", zName);
  }
  যদি( idx_অঙ্ক & 0x08 ){
    sqlite3_str_appendf(pSql, " ORDER BY name");
  }
  zSql = sqlite3_str_finish(pSql);
  যদি( zSql==0 ){
    ফেরত SQLITE_NOMEM_BKPT;
  }অন্যথা{
    rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pStmt, 0);
    sqlite3_free(zSql);
  }

  যদি( rc==SQLITE_OK ){
    pCsr->iPage = -1;
    rc = stat_পরবর্তী(pCursor);
  }
  ফেরত rc;
}

স্থির পূর্ণসংখ্যা statColumn(
  sqlite3_vtab_cursor *pCursor, 
  sqlite3_context *ctx, 
  পূর্ণসংখ্যা i
){
  StatCursor *pCsr = (StatCursor *)pCursor;
  নির্বাচন( i ){
    ক্ষেত্র 0:            /* name */
      sqlite3_ফলাফল_text(ctx, pCsr->zName, -1, SQLITE_TRANSIENT);
      বিরতি;
    ক্ষেত্র 1:            /* path */
      যদি( !pCsr->isAgg ){
        sqlite3_ফলাফল_text(ctx, pCsr->zPath, -1, SQLITE_TRANSIENT);
      }
      বিরতি;
    ক্ষেত্র 2:            /* pageno */
      যদি( pCsr->isAgg ){
        sqlite3_ফলাফল_int64(ctx, pCsr->nPage);
      }অন্যথা{
        sqlite3_ফলাফল_int64(ctx, pCsr->iPageno);
      }
      বিরতি;
    ক্ষেত্র 3:            /* pagetype */
      যদি( !pCsr->isAgg ){
        sqlite3_ফলাফল_text(ctx, pCsr->zPagetype, -1, SQLITE_STATIC);
      }
      বিরতি;
    ক্ষেত্র 4:            /* ncell */
      sqlite3_ফলাফল_int64(ctx, pCsr->nCell);
      বিরতি;
    ক্ষেত্র 5:            /* payload */
      sqlite3_ফলাফল_int64(ctx, pCsr->nPayload);
      বিরতি;
    ক্ষেত্র 6:            /* unused */
      sqlite3_ফলাফল_int64(ctx, pCsr->nUnused);
      বিরতি;
    ক্ষেত্র 7:            /* mx_payload */
      sqlite3_ফলাফল_int64(ctx, pCsr->nMxPayload);
      বিরতি;
    ক্ষেত্র 8:            /* pgoffset */
      যদি( !pCsr->isAgg ){
        sqlite3_ফলাফল_int64(ctx, pCsr->iOffset);
      }
      বিরতি;
    ক্ষেত্র 9:            /* pgsize */
      sqlite3_ফলাফল_int64(ctx, pCsr->szPage);
      বিরতি;
    ক্ষেত্র 10: {         /* schema */
      sqlite3 *db = sqlite3_context_db_handle(ctx);
      পূর্ণসংখ্যা iDb = pCsr->iDb;
      sqlite3_ফলাফল_text(ctx, db->aDb[iDb].zDbSName, -1, SQLITE_STATIC);
      বিরতি;
    }
    পূর্বনির্ধারিত: {         /* aggregate */
      sqlite3_ফলাফল_int(ctx, pCsr->isAgg);
      বিরতি;
    }
  }
  ফেরত SQLITE_OK;
}

স্থির পূর্ণসংখ্যা statRowid(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid){
  StatCursor *pCsr = (StatCursor *)pCursor;
  *pRowid = pCsr->iPageno;
  ফেরত SQLITE_OK;
}

/*
** Invoke this routine to register the "dbstat" virtual table module
*/
পূর্ণসংখ্যা sqlite3DbstatRegister(sqlite3 *db){
  স্থির sqlite3_module dbstat_module = {
    0,                            /* iVersion */
    statConnect,                  /* xCreate */
    statConnect,                  /* xConnect */
    stat_Best_সূচক,                /* xBestIndex */
    statDisconnect,               /* xDisconnect */
    statDisconnect,               /* xDestroy */
    statOpen,                     /* xOpen - open a cursor */
    statClose,                    /* xClose - close a cursor */
    statFilter,                   /* xFilter - configure scan constraints */
    stat_পরবর্তী,                     /* xNext - advance a cursor */
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
  ফেরত sqlite3_create_module(db, "dbstat", &dbstat_module, 0);
}
#নতুবা_যদি defined(SQLITE_ENABLE_DBSTAT_VTAB)
পূর্ণসংখ্যা sqlite3DbstatRegister(sqlite3 *db){ ফেরত SQLITE_OK; }
#endif /* SQLITE_ENABLE_DBSTAT_VTAB */
