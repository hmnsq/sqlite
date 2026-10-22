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

#ضمن "sqliteInt.h"   /* Requires access to internal data structures */
#إذا (defined(SQLITE_ENABLE_DBSTAT_VTAB) || defined(SQLITE_TEST)) \
    && !defined(SQLITE_OMIT_VIRTUALTABLE)

/*
** The pager and btree modules arrange objects in memory so that there are
** always approximately 200 bytes of addressable memory following each page
** buffer. This way small buffer overreads caused by corrupt database pages
** do not cause undefined behaviour. This module pads each page buffer
** by the following number of bytes for the same purpose.
*/
#تعریف_کن DBSTAT_صفحه_PADDING_بایتها 256

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
ساكن ثابت حرف zDbstatSchema[] = 
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
تعريفنوع بنية Stat_جدول Stat_جدول;
تعريفنوع بنية StatCursor StatCursor;
تعريفنوع بنية Stat_صفحه Stat_صفحه;
تعريفنوع بنية StatCell StatCell;

/* Size information for a single cell within a btree page */
بنية StatCell {
  صحيح nLocal;                     /* Bytes of local payload */
  u32 i_فرزند_Pg;                   /* Child node (or 0 if this is a leaf) */
  صحيح nOvfl;                      /* Entries in aOvfl[] */
  u32 *aOvfl;                     /* Array of overflow page numbers */
  صحيح n_آخرین_Ovfl;                  /* Bytes of payload on final overflow page */
  صحيح iOvfl;                      /* Iterates through aOvfl[] */
};

/* Size information for a single btree page */
بنية Stat_صفحه {
  u32 iPgno;                      /* Page number */
  u8 *aPg;                        /* Page buffer from sqlite3_malloc() */
  صحيح iCell;                      /* Current cell */
  حرف *z_مسیر;                    /* Path to this page */

  /* Variables populated by statDecodePage(): */
  u8 flags;                       /* Copy of flags byte */
  صحيح nCell;                      /* Number of cells on page */
  صحيح n_استفادهنشده;                    /* Number of unused bytes on page */
  StatCell *aCell;                /* Array of parsed cells */
  u32 i_راست_فرزند_Pg;              /* Right-child page number (or 0) */
  صحيح nMxPayload;                 /* Largest payload of any cell on the page */
};

/* The cursor for scanning the dbstat virtual table */
بنية StatCursor {
  sqlite3_vtab_cursor پایه;       /* base class.  MUST BE FIRST! */
  sqlite3_stmt *pStmt;            /* Iterates through set of root pages */
  u8 isEof;                       /* After pStmt has returned SQLITE_DONE */
  u8 isAgg;                       /* Aggregate results for each table */
  صحيح iDb;                        /* Schema used for this query */

  Stat_صفحه a_صفحه[32];             /* Pages in path to current page */
  صحيح i_صفحه;                      /* Current entry in aPage[] */

  /* Values to return. */
  u32 iPageno;                    /* Value of 'pageno' column */
  حرف *zName;                    /* Value of 'name' column */
  حرف *z_مسیر;                    /* Value of 'path' column */
  حرف *zPagetype;                /* Value of 'pagetype' column */
  صحيح n_صفحه;                      /* Number of pages in current btree */
  صحيح nCell;                      /* Value of 'ncell' column */
  صحيح nMxPayload;                 /* Value of 'mx_payload' column */
  i64 n_استفادهنشده;                    /* Value of 'unused' column */
  i64 nPayload;                   /* Value of 'payload' column */
  i64 i_جابه_جایی;                    /* Value of 'pgOffset' column */
  i64 sz_صفحه;                     /* Value of 'pgSize' column */
};

/* An instance of the DBSTAT virtual table */
بنية Stat_جدول {
  sqlite3_vtab پایه;              /* base class.  MUST BE FIRST! */
  sqlite3 *db;                    /* Database connection that owns this vtab */
  صحيح iDb;                        /* Index of database to analyze */
};

#اگر_تعریف_نشده get2byte
# define get2byte(x)   ((x)[0]<<8 | (x)[1])
#پایان_اگر

/*
** Connect to or create a new DBSTAT virtual table.
*/
ساكن صحيح statConnect(
  sqlite3 *db,
  فراغ *pAux,
  صحيح argc, ثابت حرف *ثابت*argv,
  sqlite3_vtab **ppVtab,
  حرف **pzErr
){
  Stat_جدول *pTab = 0;
  صحيح rc = SQLITE_OK;
  صحيح iDb;
  (فراغ)pAux;

  إذا( argc>=4 ){
    Token nm;
    sqlite3TokenInit(&nm, (حرف*)argv[3]);
    iDb = sqlite3_پیدا_کن_Db(db, &nm);
    إذا( iDb<0 ){
      *pzErr = sqlite3_mprintf("no such database: %s", argv[3]);
      أرجع SQLITE_ERROR;
    }
  }وإلا{
    iDb = 0;
  }
  sqlite3_vtab_config(db, SQLITE_VTAB_DIRECTONLY);
  rc = sqlite3_declare_vtab(db, zDbstatSchema);
  إذا( rc==SQLITE_OK ){
    pTab = (Stat_جدول *)sqlite3_malloc64(حجم(Stat_جدول));
    إذا( pTab==0 ) rc = SQLITE_NOMEM_BKPT;
  }

  أكد( rc==SQLITE_OK || pTab==0 );
  إذا( rc==SQLITE_OK ){
    memset(pTab, 0, حجم(Stat_جدول));
    pTab->db = db;
    pTab->iDb = iDb;
  }

  *ppVtab = (sqlite3_vtab*)pTab;
  أرجع rc;
}

/*
** Disconnect from or destroy the DBSTAT virtual table.
*/
ساكن صحيح statDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  أرجع SQLITE_OK;
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
ساكن صحيح stat_بهترین_شاخص(sqlite3_vtab *tab, sqlite3_شاخص_info *pIdxInfo){
  صحيح i;
  صحيح iSchema = -1;
  صحيح iName = -1;
  صحيح iAgg = -1;
  (فراغ)tab;

  /* Look for a valid schema=? constraint.  If found, change the idxNum to
  ** 1 and request the value of that constraint be sent to xFilter.  And
  ** lower the cost estimate to encourage the constrained version to be
  ** used.
  */
  لكل(i=0; i<pIdxInfo->nConstraint; i++){
    إذا( pIdxInfo->aConstraint[i].op!=SQLITE_شاخص_CONSTRAINT_EQ ) استمر;
    إذا( pIdxInfo->aConstraint[i].usable==0 ){
      /* Force DBSTAT table should always be the right-most table in a join */
      أرجع SQLITE_CONSTRAINT;
    }
    بدل( pIdxInfo->aConstraint[i].i_ستونی ){
      حالة 0: {    /* name */
        iName = i;
        اكسر;
      }
      حالة 10: {   /* schema */
        iSchema = i;
        اكسر;
      }
      حالة 11: {   /* aggregate */
        iAgg = i;
        اكسر;
      }
    }
  }
  i = 0;
  إذا( iSchema>=0 ){
    pIdxInfo->aConstraintUsage[iSchema].argv_شاخص = ++i;
    pIdxInfo->aConstraintUsage[iSchema].omit = 1;
    pIdxInfo->idx_رقم |= 0x01;
  }
  إذا( iName>=0 ){
    pIdxInfo->aConstraintUsage[iName].argv_شاخص = ++i;
    pIdxInfo->idx_رقم |= 0x02;
  }
  إذا( iAgg>=0 ){
    pIdxInfo->aConstraintUsage[iAgg].argv_شاخص = ++i;
    pIdxInfo->idx_رقم |= 0x04;
  }
  pIdxInfo->estimated_هزینه = 1.0;

  /* Records are always returned in ascending order of (name, path). 
  ** If this will satisfy the client, set the orderByConsumed flag so that 
  ** SQLite does not do an external sort.
  */
  إذا( ( pIdxInfo->n_ترتیب_By==1
     && pIdxInfo->a_ترتیب_By[0].i_ستونی==0
     && pIdxInfo->a_ترتیب_By[0].desc==0
     ) ||
      ( pIdxInfo->n_ترتیب_By==2
     && pIdxInfo->a_ترتیب_By[0].i_ستونی==0
     && pIdxInfo->a_ترتیب_By[0].desc==0
     && pIdxInfo->a_ترتیب_By[1].i_ستونی==1
     && pIdxInfo->a_ترتیب_By[1].desc==0
     )
  ){
    pIdxInfo->ترتیب_By_Consumed = 1;
    pIdxInfo->idx_رقم |= 0x08;
  }
  pIdxInfo->idxFlags |= SQLITE_شاخص_پیمایش_HEX;

  أرجع SQLITE_OK;
}

/*
** Open a new DBSTAT cursor.
*/
ساكن صحيح statOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  Stat_جدول *pTab = (Stat_جدول *)pVTab;
  StatCursor *pCsr;

  pCsr = (StatCursor *)sqlite3_malloc64(حجم(StatCursor));
  إذا( pCsr==0 ){
    أرجع SQLITE_NOMEM_BKPT;
  }وإلا{
    memset(pCsr, 0, حجم(StatCursor));
    pCsr->پایه.pVtab = pVTab;
    pCsr->iDb = pTab->iDb;
  }

  *ppCursor = (sqlite3_vtab_cursor *)pCsr;
  أرجع SQLITE_OK;
}

ساكن فراغ statClearCells(Stat_صفحه *p){
  صحيح i;
  إذا( p->aCell ){
    لكل(i=0; i<p->nCell; i++){
      sqlite3_free(p->aCell[i].aOvfl);
    }
    sqlite3_free(p->aCell);
  }
  p->nCell = 0;
  p->aCell = 0;
}

ساكن فراغ stat_Clear_صفحه(Stat_صفحه *p){
  u8 *aPg = p->aPg;
  statClearCells(p);
  sqlite3_free(p->z_مسیر);
  memset(p, 0, حجم(Stat_صفحه));
  p->aPg = aPg;
}

ساكن فراغ statResetCsr(StatCursor *pCsr){
  صحيح i;
  /* In some circumstances, specifically if an OOM has occurred, the call
  ** to sqlite3_reset() may cause the pager to be reset (emptied). It is
  ** important that statClearPage() is called to free any page refs before
  ** this happens. dbsqlfuzz 9ed3e4e3816219d3509d711636c38542bf3f40b1. */
  لكل(i=0; i<Array_اندازه(pCsr->a_صفحه); i++){
    stat_Clear_صفحه(&pCsr->a_صفحه[i]);
    sqlite3_free(pCsr->a_صفحه[i].aPg);
    pCsr->a_صفحه[i].aPg = 0;
  }
  sqlite3_reset(pCsr->pStmt);
  pCsr->i_صفحه = 0;
  sqlite3_free(pCsr->z_مسیر);
  pCsr->z_مسیر = 0;
  pCsr->isEof = 0;
}

/* Resize the space-used counters inside of the cursor */
ساكن فراغ statResetCounts(StatCursor *pCsr){
  pCsr->nCell = 0;
  pCsr->nMxPayload = 0;
  pCsr->n_استفادهنشده = 0;
  pCsr->nPayload = 0;
  pCsr->sz_صفحه = 0;
  pCsr->n_صفحه = 0;
}

/*
** Close a DBSTAT cursor.
*/
ساكن صحيح statClose(sqlite3_vtab_cursor *pCursor){
  StatCursor *pCsr = (StatCursor *)pCursor;
  statResetCsr(pCsr);
  sqlite3_finalize(pCsr->pStmt);
  sqlite3_free(pCsr);
  أرجع SQLITE_OK;
}

/*
** For a single cell on a btree page, compute the number of bytes of
** content (payload) stored on that page.  That is to say, compute the
** number of bytes of content not found on overflow pages.
*/
ساكن صحيح getLocalPayload(
  صحيح nUsable,                    /* Usable bytes per page */
  u8 flags,                       /* Page flags */
  صحيح n_مجموع                      /* Total record (payload) size */
){
  صحيح nLocal;
  صحيح nMinLocal;
  صحيح nMaxLocal;
 
  إذا( flags==0x0D ){              /* Table leaf node */
    nMinLocal = (nUsable - 12) * 32 / 255 - 23;
    nMaxLocal = nUsable - 35;
  }وإلا{                          /* Index interior and leaf nodes */
    nMinLocal = (nUsable - 12) * 32 / 255 - 23;
    nMaxLocal = (nUsable - 12) * 64 / 255 - 23;
  }

  nLocal = nMinLocal + (n_مجموع - nMinLocal) % (nUsable - 4);
  إذا( nLocal>nMaxLocal ) nLocal = nMinLocal;
  أرجع nLocal;
}

/* Populate the StatPage object with information about the all
** cells found on the page currently under analysis.
*/
ساكن صحيح stat_Decode_صفحه(Btree *pBt, Stat_صفحه *p){
  صحيح n_استفادهنشده;
  صحيح iOff;
  صحيح nHdr;
  صحيح is_برگ;
  صحيح sz_صفحه;

  u8 *a_داده = p->aPg;
  u8 *aHdr = &a_داده[p->iPgno==1 ? 100 : 0];

  p->flags = aHdr[0];
  إذا( p->flags==0x0A || p->flags==0x0D ){
    is_برگ = 1;
    nHdr = 8;
  }وإلا إذا( p->flags==0x05 || p->flags==0x02 ){
    is_برگ = 0;
    nHdr = 12;
  }وإلا{
    اذهب stat_صفحه_Is_Corrupt;
  }
  إذا( p->iPgno==1 ) nHdr += 100;
  p->nCell = get2byte(&aHdr[3]);
  p->nMxPayload = 0;
  sz_صفحه = sqlite3_Btree_Get_صفحه_اندازه(pBt);

  n_استفادهنشده = get2byte(&aHdr[5]) - nHdr - 2*p->nCell;
  n_استفادهنشده += (صحيح)aHdr[7];
  iOff = get2byte(&aHdr[1]);
  طالما( iOff ){
    صحيح i_بعدی;
    إذا( iOff>=sz_صفحه ) اذهب stat_صفحه_Is_Corrupt;
    n_استفادهنشده += get2byte(&a_داده[iOff+2]);
    i_بعدی = get2byte(&a_داده[iOff]);
    إذا( i_بعدی<iOff+4 && i_بعدی>0 ) اذهب stat_صفحه_Is_Corrupt;
    iOff = i_بعدی;
  }
  p->n_استفادهنشده = n_استفادهنشده;
  p->i_راست_فرزند_Pg = is_برگ ? 0 : sqlite3Get4byte(&aHdr[8]);

  إذا( p->nCell ){
    صحيح i;                        /* Used to iterate through cells */
    صحيح nUsable;                  /* Usable bytes per page */

    sqlite3BtreeEnter(pBt);
    nUsable = sz_صفحه - sqlite3BtreeGetReserveNoMutex(pBt);
    sqlite3BtreeLeave(pBt);
    p->aCell = sqlite3_malloc64((p->nCell+1) * حجم(StatCell));
    إذا( p->aCell==0 ) أرجع SQLITE_NOMEM_BKPT;
    memset(p->aCell, 0, (p->nCell+1) * حجم(StatCell));

    لكل(i=0; i<p->nCell; i++){
      StatCell *pCell = &p->aCell[i];

      iOff = get2byte(&a_داده[nHdr+i*2]);
      إذا( iOff<nHdr || iOff>=sz_صفحه ) اذهب stat_صفحه_Is_Corrupt;
      إذا( !is_برگ ){
        pCell->i_فرزند_Pg = sqlite3Get4byte(&a_داده[iOff]);
        iOff += 4;
      }
      إذا( p->flags==0x05 ){
        /* A table interior node. nPayload==0. */
      }وإلا{
        u32 nPayload;             /* Bytes of payload total (local+overflow) */
        صحيح nLocal;               /* Bytes of payload stored locally */
        iOff += getVarint32(&a_داده[iOff], nPayload);
        إذا( p->flags==0x0D ){
          u64 dummy;
          iOff += sqlite3GetVarint(&a_داده[iOff], &dummy);
        }
        إذا( nPayload>(u32)p->nMxPayload ) p->nMxPayload = nPayload;
        nLocal = getLocalPayload(nUsable, p->flags, nPayload);
        إذا( nLocal<0 ) اذهب stat_صفحه_Is_Corrupt;
        pCell->nLocal = nLocal;
        أكد( nPayload>=(u32)nLocal );
        أكد( nLocal<=(nUsable-35) );
        إذا( nPayload>(u32)nLocal ){
          صحيح j;
          صحيح nOvfl = ((nPayload - nLocal) + nUsable-4 - 1) / (nUsable - 4);
          إذا( iOff+nLocal+4>nUsable || nPayload>0x7fffffff ){
            اذهب stat_صفحه_Is_Corrupt;
          }
          pCell->n_آخرین_Ovfl = (nPayload-nLocal) - (nOvfl-1) * (nUsable-4);
          pCell->nOvfl = nOvfl;
          pCell->aOvfl = sqlite3_malloc64(حجم(u32)*nOvfl);
          إذا( pCell->aOvfl==0 ) أرجع SQLITE_NOMEM_BKPT;
          pCell->aOvfl[0] = sqlite3Get4byte(&a_داده[iOff+nLocal]);
          لكل(j=1; j<nOvfl; j++){
            صحيح rc;
            u32 iPrev = pCell->aOvfl[j-1];
            Db_صفحه *pPg = 0;
            rc = sqlite3PagerGet(sqlite3BtreePager(pBt), iPrev, &pPg, 0);
            إذا( rc!=SQLITE_OK ){
              أكد( pPg==0 );
              أرجع rc;
            } 
            pCell->aOvfl[j] = sqlite3Get4byte(sqlite3_Pager_Get_داده(pPg));
            sqlite3PagerUnref(pPg);
          }
        }
      }
    }
  }

  أرجع SQLITE_OK;

stat_صفحه_Is_Corrupt:
  p->flags = 0;
  statClearCells(p);
  أرجع SQLITE_OK;
}

/*
** Populate the pCsr->iOffset and pCsr->szPage member variables. Based on
** the current value of pCsr->iPageno.
*/
ساكن فراغ stat_اندازه_And_جابه_جایی(StatCursor *pCsr){
  Stat_جدول *pTab = (Stat_جدول *)((sqlite3_vtab_cursor *)pCsr)->pVtab;
  Btree *pBt = pTab->db->aDb[pTab->iDb].pBt;
  Pager *pPager = sqlite3BtreePager(pBt);
  sqlite3_file *fd;
  sqlite3_int64 x[2];

  /* If connected to a ZIPVFS backend, find the page size and
  ** offset from ZIPVFS.
  */
  fd = sqlite3PagerFile(pPager);
  x[0] = pCsr->iPageno;
  إذا( sqlite3OsFileControl(fd, 230440, &x)==SQLITE_OK ){
    pCsr->i_جابه_جایی = x[0];
    pCsr->sz_صفحه += x[1];
  }وإلا{
    /* Not ZIPVFS: The default page size and offset */
    pCsr->sz_صفحه += sqlite3_Btree_Get_صفحه_اندازه(pBt);
    pCsr->i_جابه_جایی = (i64)pCsr->sz_صفحه * (pCsr->iPageno - 1);
  }
}

/*
** Load a copy of the page data for page iPg into the buffer belonging
** to page object pPg. Allocate the buffer if necessary. Return SQLITE_OK
** if successful, or an SQLite error code otherwise.
*/
ساكن صحيح stat_Get_صفحه(
  Btree *pBt,                     /* Load page from this b-tree */
  u32 iPg,                        /* Page number to load */
  Stat_صفحه *pPg                   /* Load page into this object */
){
  صحيح pgsz = sqlite3_Btree_Get_صفحه_اندازه(pBt);
  Db_صفحه *p_Db_صفحه = 0;
  صحيح rc;

  إذا( pPg->aPg==0 ){
    pPg->aPg = (u8*)sqlite3_malloc(pgsz + DBSTAT_صفحه_PADDING_بایتها);
    إذا( pPg->aPg==0 ){
      أرجع SQLITE_NOMEM_BKPT;
    }
    memset(&pPg->aPg[pgsz], 0, DBSTAT_صفحه_PADDING_بایتها);
  }

  rc = sqlite3PagerGet(sqlite3BtreePager(pBt), iPg, &p_Db_صفحه, 0);
  إذا( rc==SQLITE_OK ){
    ثابت u8 *a = sqlite3_Pager_Get_داده(p_Db_صفحه);
    کپی_حافظه(pPg->aPg, a, pgsz);
    sqlite3PagerUnref(p_Db_صفحه);
  }

  أرجع rc;
}

/*
** Move a DBSTAT cursor to the next entry.  Normally, the next
** entry will be the next page, but in aggregated mode (pCsr->isAgg!=0),
** the next entry is the next btree.
*/
ساكن صحيح stat_بعدی(sqlite3_vtab_cursor *pCursor){
  صحيح rc;
  صحيح nPayload;
  حرف *z;
  StatCursor *pCsr = (StatCursor *)pCursor;
  Stat_جدول *pTab = (Stat_جدول *)pCursor->pVtab;
  Btree *pBt = pTab->db->aDb[pCsr->iDb].pBt;
  Pager *pPager = sqlite3BtreePager(pBt);

  sqlite3_free(pCsr->z_مسیر);
  pCsr->z_مسیر = 0;

stat_بعدی_Restart:
  إذا( pCsr->i_صفحه<0 ){
    /* Start measuring space on the next btree */
    statResetCounts(pCsr);
    rc = sqlite3_گام(pCsr->pStmt);
    إذا( rc==SQLITE_ردیف ){
      صحيح n_صفحه;
      u32 i_ریشه = (u32)sqlite3_ستونی_int64(pCsr->pStmt, 1);
      sqlite3PagerPagecount(pPager, &n_صفحه);
      إذا( n_صفحه==0 ){
        pCsr->isEof = 1;
        أرجع sqlite3_reset(pCsr->pStmt);
      }
      rc = stat_Get_صفحه(pBt, i_ریشه, &pCsr->a_صفحه[0]);
      pCsr->a_صفحه[0].iPgno = i_ریشه;
      pCsr->a_صفحه[0].iCell = 0;
      إذا( !pCsr->isAgg ){
        pCsr->a_صفحه[0].z_مسیر = z = sqlite3_mprintf("/");
        إذا( z==0 ) rc = SQLITE_NOMEM_BKPT;
      }
      pCsr->i_صفحه = 0;
      pCsr->n_صفحه = 1;
    }وإلا{
      pCsr->isEof = 1;
      أرجع sqlite3_reset(pCsr->pStmt);
    }
  }وإلا{
    /* Continue analyzing the btree previously started */
    Stat_صفحه *p = &pCsr->a_صفحه[pCsr->i_صفحه];
    إذا( !pCsr->isAgg ) statResetCounts(pCsr);
    طالما( p->iCell<p->nCell ){
      StatCell *pCell = &p->aCell[p->iCell];
      طالما( pCell->iOvfl<pCell->nOvfl ){
        صحيح nUsable;
        صحيح iOvfl = pCell->iOvfl;
        sqlite3BtreeEnter(pBt);
        nUsable = sqlite3_Btree_Get_صفحه_اندازه(pBt) - 
                        sqlite3BtreeGetReserveNoMutex(pBt);
        sqlite3BtreeLeave(pBt);
        pCsr->n_صفحه++;
        pCsr->iPageno = pCell->aOvfl[iOvfl];
        stat_اندازه_And_جابه_جایی(pCsr);
        إذا( pCell->iOvfl<pCell->nOvfl-1 ){
          pCsr->nPayload += nUsable - 4;
        }وإلا{
          pCsr->nPayload += pCell->n_آخرین_Ovfl;
          pCsr->n_استفادهنشده += nUsable - 4 - pCell->n_آخرین_Ovfl;
        }
        pCell->iOvfl++;
        إذا( !pCsr->isAgg ){
          pCsr->zName = (حرف *)sqlite3_ستونی_متن(pCsr->pStmt, 0);
          pCsr->zPagetype = "overflow";
          pCsr->z_مسیر = z = sqlite3_mprintf(
              "%s%.3x+%.6x", p->z_مسیر, p->iCell, iOvfl
          );
          أرجع z==0 ? SQLITE_NOMEM_BKPT : SQLITE_OK;
        }
      }
      إذا( p->i_راست_فرزند_Pg ) اكسر;
      p->iCell++;
    }

    إذا( !p->i_راست_فرزند_Pg || p->iCell>p->nCell ){
      stat_Clear_صفحه(p);
      pCsr->i_صفحه--;
      إذا( pCsr->isAgg && pCsr->i_صفحه<0 ){
        /* label-statNext-done:  When computing aggregate space usage over
        ** an entire btree, this is the exit point from this function */
        أرجع SQLITE_OK;
      }
      اذهب stat_بعدی_Restart; /* Tail recursion */
    }
    pCsr->i_صفحه++;
    إذا( pCsr->i_صفحه>=Array_اندازه(pCsr->a_صفحه) ){
      statResetCsr(pCsr);
      أرجع SQLITE_CORRUPT_BKPT;
    }
    أكد( p==&pCsr->a_صفحه[pCsr->i_صفحه-1] );

    إذا( p->iCell==p->nCell ){
      p[1].iPgno = p->i_راست_فرزند_Pg;
    }وإلا{
      p[1].iPgno = p->aCell[p->iCell].i_فرزند_Pg;
    }
    rc = stat_Get_صفحه(pBt, p[1].iPgno, &p[1]);
    pCsr->n_صفحه++;
    p[1].iCell = 0;
    إذا( !pCsr->isAgg ){
      p[1].z_مسیر = z = sqlite3_mprintf("%s%.3x/", p->z_مسیر, p->iCell);
      إذا( z==0 ) rc = SQLITE_NOMEM_BKPT;
    }
    p->iCell++;
  }


  /* Populate the StatCursor fields with the values to be returned
  ** by the xColumn() and xRowid() methods.
  */
  إذا( rc==SQLITE_OK ){
    صحيح i;
    Stat_صفحه *p = &pCsr->a_صفحه[pCsr->i_صفحه];
    pCsr->zName = (حرف *)sqlite3_ستونی_متن(pCsr->pStmt, 0);
    pCsr->iPageno = p->iPgno;

    rc = stat_Decode_صفحه(pBt, p);
    إذا( rc==SQLITE_OK ){
      stat_اندازه_And_جابه_جایی(pCsr);

      بدل( p->flags ){
        حالة 0x05:             /* table internal */
        حالة 0x02:             /* index internal */
          pCsr->zPagetype = "internal";
          اكسر;
        حالة 0x0D:             /* table leaf */
        حالة 0x0A:             /* index leaf */
          pCsr->zPagetype = "leaf";
          اكسر;
        افتراضي:
          pCsr->zPagetype = "corrupted";
          اكسر;
      }
      pCsr->nCell += p->nCell;
      pCsr->n_استفادهنشده += p->n_استفادهنشده;
      إذا( p->nMxPayload>pCsr->nMxPayload ) pCsr->nMxPayload = p->nMxPayload;
      إذا( !pCsr->isAgg ){
        pCsr->z_مسیر = z = sqlite3_mprintf("%s", p->z_مسیر);
        إذا( z==0 ) rc = SQLITE_NOMEM_BKPT;
      }
      nPayload = 0;
      لكل(i=0; i<p->nCell; i++){
        nPayload += p->aCell[i].nLocal;
      }
      pCsr->nPayload += nPayload;

      /* If computing aggregate space usage by btree, continue with the
      ** next page.  The loop will exit via the return at label-statNext-done
      */
      إذا( pCsr->isAgg ) اذهب stat_بعدی_Restart;
    }
  }

  أرجع rc;
}

ساكن صحيح statEof(sqlite3_vtab_cursor *pCursor){
  StatCursor *pCsr = (StatCursor *)pCursor;
  أرجع pCsr->isEof;
}

/* Initialize a cursor according to the query plan idxNum using the
** arguments in argv[0].  See statBestIndex() for a description of the
** meaning of the bits in idxNum.
*/
ساكن صحيح statFilter(
  sqlite3_vtab_cursor *pCursor, 
  صحيح idx_رقم, ثابت حرف *idxStr,
  صحيح argc, sqlite3_مقدار **argv
){
  StatCursor *pCsr = (StatCursor *)pCursor;
  Stat_جدول *pTab = (Stat_جدول*)(pCursor->pVtab);
  sqlite3_str *pSql;      /* Query of btrees to analyze */
  حرف *zSql;             /* String value of pSql */
  صحيح iArg = 0;           /* Count of argv[] parameters used so far */
  صحيح rc = SQLITE_OK;     /* Result of this operation */
  ثابت حرف *zName = 0;  /* Only provide analysis of this table */
  (فراغ)argc;
  (فراغ)idxStr;

  statResetCsr(pCsr);
  sqlite3_finalize(pCsr->pStmt);
  pCsr->pStmt = 0;
  إذا( idx_رقم & 0x01 ){
    /* schema=? constraint is present.  Get its value */
    ثابت حرف *zDbase = (ثابت حرف*)sqlite3_مقدار_متن(argv[iArg++]);
    pCsr->iDb = sqlite3_پیدا_کن_Db_Name(pTab->db, zDbase);
    إذا( pCsr->iDb<0 ){
      pCsr->iDb = 0;
      pCsr->isEof = 1;
      أرجع SQLITE_OK;
    }
  }وإلا{
    pCsr->iDb = pTab->iDb;
  }
  إذا( idx_رقم & 0x02 ){
    /* name=? constraint is present */
    zName = (ثابت حرف*)sqlite3_مقدار_متن(argv[iArg++]);
  }
  إذا( idx_رقم & 0x04 ){
    /* aggregate=? constraint is present */
    pCsr->isAgg = sqlite3_مقدار_double(argv[iArg++])!=0.0;
  }وإلا{
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
  إذا( zName ){
    sqlite3_str_appendf(pSql, "WHERE name=%Q", zName);
  }
  إذا( idx_رقم & 0x08 ){
    sqlite3_str_appendf(pSql, " ORDER BY name");
  }
  zSql = sqlite3_str_پایان(pSql);
  إذا( zSql==0 ){
    أرجع SQLITE_NOMEM_BKPT;
  }وإلا{
    rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pStmt, 0);
    sqlite3_free(zSql);
  }

  إذا( rc==SQLITE_OK ){
    pCsr->i_صفحه = -1;
    rc = stat_بعدی(pCursor);
  }
  أرجع rc;
}

ساكن صحيح stat_ستونی(
  sqlite3_vtab_cursor *pCursor, 
  sqlite3_context *ctx, 
  صحيح i
){
  StatCursor *pCsr = (StatCursor *)pCursor;
  بدل( i ){
    حالة 0:            /* name */
      sqlite3_نتیجه_متن(ctx, pCsr->zName, -1, SQLITE_TRANSIENT);
      اكسر;
    حالة 1:            /*مسیر*/
      إذا( !pCsr->isAgg ){
        sqlite3_نتیجه_متن(ctx, pCsr->z_مسیر, -1, SQLITE_TRANSIENT);
      }
      اكسر;
    حالة 2:            /* pageno */
      إذا( pCsr->isAgg ){
        sqlite3_نتیجه_int64(ctx, pCsr->n_صفحه);
      }وإلا{
        sqlite3_نتیجه_int64(ctx, pCsr->iPageno);
      }
      اكسر;
    حالة 3:            /* pagetype */
      إذا( !pCsr->isAgg ){
        sqlite3_نتیجه_متن(ctx, pCsr->zPagetype, -1, SQLITE_STATIC);
      }
      اكسر;
    حالة 4:            /* ncell */
      sqlite3_نتیجه_int64(ctx, pCsr->nCell);
      اكسر;
    حالة 5:            /* payload */
      sqlite3_نتیجه_int64(ctx, pCsr->nPayload);
      اكسر;
    حالة 6:            /*استفادهنشده*/
      sqlite3_نتیجه_int64(ctx, pCsr->n_استفادهنشده);
      اكسر;
    حالة 7:            /* mx_payload */
      sqlite3_نتیجه_int64(ctx, pCsr->nMxPayload);
      اكسر;
    حالة 8:            /* pgoffset */
      إذا( !pCsr->isAgg ){
        sqlite3_نتیجه_int64(ctx, pCsr->i_جابه_جایی);
      }
      اكسر;
    حالة 9:            /* pgsize */
      sqlite3_نتیجه_int64(ctx, pCsr->sz_صفحه);
      اكسر;
    حالة 10: {         /* schema */
      sqlite3 *db = sqlite3_context_db_handle(ctx);
      صحيح iDb = pCsr->iDb;
      sqlite3_نتیجه_متن(ctx, db->aDb[iDb].zDbSName, -1, SQLITE_STATIC);
      اكسر;
    }
    افتراضي: {         /* aggregate */
      sqlite3_نتیجه_int(ctx, pCsr->isAgg);
      اكسر;
    }
  }
  أرجع SQLITE_OK;
}

ساكن صحيح statRowid(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid){
  StatCursor *pCsr = (StatCursor *)pCursor;
  *pRowid = pCsr->iPageno;
  أرجع SQLITE_OK;
}

/*
** Invoke this routine to register the "dbstat" virtual table module
*/
صحيح sqlite3_Dbstat_ثبات(sqlite3 *db){
  ساكن sqlite3_module dbstat_module = {
    0,                            /* iVersion */
    statConnect,                  /* xCreate */
    statConnect,                  /* xConnect */
    stat_بهترین_شاخص,                /* xBestIndex */
    statDisconnect,               /* xDisconnect */
    statDisconnect,               /* xDestroy */
    statOpen,                     /* xOpen - open a cursor */
    statClose,                    /* xClose - close a cursor */
    statFilter,                   /* xFilter - configure scan constraints */
    stat_بعدی,                     /* xNext - advance a cursor */
    statEof,                      /* xEof - check for end of scan */
    stat_ستونی,                   /* xColumn - read data */
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
  أرجع sqlite3_create_module(db, "dbstat", &dbstat_module, 0);
}
#وإلاإذا defined(SQLITE_ENABLE_DBSTAT_VTAB)
صحيح sqlite3_Dbstat_ثبات(sqlite3 *db){ أرجع SQLITE_OK; }
#پایان_اگر /* SQLITE_ENABLE_DBSTAT_VTAB */
