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
#تعریف_کرو DBSTAT_صفحہ_PADDING_بائٹیں 256

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
ساکن ثابت حرف zDbstatSchema[] = 
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
قسمتعریف ساخت Stat_جدول Stat_جدول;
قسمتعریف ساخت StatCursor StatCursor;
قسمتعریف ساخت Stat_صفحہ Stat_صفحہ;
قسمتعریف ساخت StatCell StatCell;

/* Size information for a single cell within a btree page */
ساخت StatCell {
  صحیح nLocal;                     /* Bytes of local payload */
  u32 i_بچہ_Pg;                   /* Child node (or 0 if this is a leaf) */
  صحیح nOvfl;                      /* Entries in aOvfl[] */
  u32 *aOvfl;                     /* Array of overflow page numbers */
  صحیح n_آخری_Ovfl;                  /* Bytes of payload on final overflow page */
  صحیح iOvfl;                      /* Iterates through aOvfl[] */
};

/* Size information for a single btree page */
ساخت Stat_صفحہ {
  u32 iPgno;                      /* Page number */
  u8 *aPg;                        /* Page buffer from sqlite3_malloc() */
  صحیح iCell;                      /* Current cell */
  حرف *z_راستہ;                    /* Path to this page */

  /* Variables populated by statDecodePage(): */
  u8 flags;                       /* Copy of flags byte */
  صحیح nCell;                      /* Number of cells on page */
  صحیح n_غیر_مستعمل;                    /* Number of unused bytes on page */
  StatCell *aCell;                /* Array of parsed cells */
  u32 i_دایاں_بچہ_Pg;              /* Right-child page number (or 0) */
  صحیح nMxPayload;                 /* Largest payload of any cell on the page */
};

/* The cursor for scanning the dbstat virtual table */
ساخت StatCursor {
  sqlite3_vtab_cursor بنیاد;       /* base class.  MUST BE FIRST! */
  sqlite3_stmt *pStmt;            /* Iterates through set of root pages */
  u8 isEof;                       /* After pStmt has returned SQLITE_DONE */
  u8 isAgg;                       /* Aggregate results for each table */
  صحیح iDb;                        /* Schema used for this query */

  Stat_صفحہ a_صفحہ[32];             /* Pages in path to current page */
  صحیح i_صفحہ;                      /* Current entry in aPage[] */

  /* Values to return. */
  u32 iPageno;                    /* Value of 'pageno' column */
  حرف *zName;                    /* Value of 'name' column */
  حرف *z_راستہ;                    /* Value of 'path' column */
  حرف *zPagetype;                /* Value of 'pagetype' column */
  صحیح n_صفحہ;                      /* Number of pages in current btree */
  صحیح nCell;                      /* Value of 'ncell' column */
  صحیح nMxPayload;                 /* Value of 'mx_payload' column */
  i64 n_غیر_مستعمل;                    /* Value of 'unused' column */
  i64 nPayload;                   /* Value of 'payload' column */
  i64 i_انحراف;                    /* Value of 'pgOffset' column */
  i64 sz_صفحہ;                     /* Value of 'pgSize' column */
};

/* An instance of the DBSTAT virtual table */
ساخت Stat_جدول {
  sqlite3_vtab بنیاد;              /* base class.  MUST BE FIRST! */
  sqlite3 *db;                    /* Database connection that owns this vtab */
  صحیح iDb;                        /* Index of database to analyze */
};

#اگر_غیر_تعریف_شدہ get2byte
# define get2byte(x)   ((x)[0]<<8 | (x)[1])
#اگر_ختم

/*
** Connect to or create a new DBSTAT virtual table.
*/
ساکن صحیح statConnect(
  sqlite3 *db,
  خالی *pAux,
  صحیح argc, ثابت حرف *ثابت*argv,
  sqlite3_vtab **ppVtab,
  حرف **pzErr
){
  Stat_جدول *pTab = 0;
  صحیح rc = SQLITE_OK;
  صحیح iDb;
  (خالی)pAux;

  اگر( argc>=4 ){
    Token nm;
    sqlite3TokenInit(&nm, (حرف*)argv[3]);
    iDb = sqlite3_ڈھونڈو_Db(db, &nm);
    اگر( iDb<0 ){
      *pzErr = sqlite3_mprintf("no such database: %s", argv[3]);
      واپس SQLITE_ERROR;
    }
  }ورنہ{
    iDb = 0;
  }
  sqlite3_vtab_config(db, SQLITE_VTAB_DIRECTONLY);
  rc = sqlite3_declare_vtab(db, zDbstatSchema);
  اگر( rc==SQLITE_OK ){
    pTab = (Stat_جدول *)sqlite3_malloc64(حجم(Stat_جدول));
    اگر( pTab==0 ) rc = SQLITE_NOMEM_BKPT;
  }

  أكد( rc==SQLITE_OK || pTab==0 );
  اگر( rc==SQLITE_OK ){
    memset(pTab, 0, حجم(Stat_جدول));
    pTab->db = db;
    pTab->iDb = iDb;
  }

  *ppVtab = (sqlite3_vtab*)pTab;
  واپس rc;
}

/*
** Disconnect from or destroy the DBSTAT virtual table.
*/
ساکن صحیح statDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  واپس SQLITE_OK;
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
ساکن صحیح stat_بہترین_اشاریہ(sqlite3_vtab *tab, sqlite3_اشاریہ_info *pIdxInfo){
  صحیح i;
  صحیح iSchema = -1;
  صحیح iName = -1;
  صحیح iAgg = -1;
  (خالی)tab;

  /* Look for a valid schema=? constraint.  If found, change the idxNum to
  ** 1 and request the value of that constraint be sent to xFilter.  And
  ** lower the cost estimate to encourage the constrained version to be
  ** used.
  */
  برائے(i=0; i<pIdxInfo->nConstraint; i++){
    اگر( pIdxInfo->aConstraint[i].op!=SQLITE_اشاریہ_CONSTRAINT_EQ ) جاری;
    اگر( pIdxInfo->aConstraint[i].usable==0 ){
      /* Force DBSTAT table should always be the right-most table in a join */
      واپس SQLITE_CONSTRAINT;
    }
    انتخاب( pIdxInfo->aConstraint[i].i_عمود ){
      حالت 0: {    /* name */
        iName = i;
        توڑو;
      }
      حالت 10: {   /* schema */
        iSchema = i;
        توڑو;
      }
      حالت 11: {   /* aggregate */
        iAgg = i;
        توڑو;
      }
    }
  }
  i = 0;
  اگر( iSchema>=0 ){
    pIdxInfo->aConstraintUsage[iSchema].argv_اشاریہ = ++i;
    pIdxInfo->aConstraintUsage[iSchema].omit = 1;
    pIdxInfo->idx_عدد |= 0x01;
  }
  اگر( iName>=0 ){
    pIdxInfo->aConstraintUsage[iName].argv_اشاریہ = ++i;
    pIdxInfo->idx_عدد |= 0x02;
  }
  اگر( iAgg>=0 ){
    pIdxInfo->aConstraintUsage[iAgg].argv_اشاریہ = ++i;
    pIdxInfo->idx_عدد |= 0x04;
  }
  pIdxInfo->estimated_لاگت = 1.0;

  /* Records are always returned in ascending order of (name, path). 
  ** If this will satisfy the client, set the orderByConsumed flag so that 
  ** SQLite does not do an external sort.
  */
  اگر( ( pIdxInfo->n_ترتیب_By==1
     && pIdxInfo->a_ترتیب_By[0].i_عمود==0
     && pIdxInfo->a_ترتیب_By[0].desc==0
     ) ||
      ( pIdxInfo->n_ترتیب_By==2
     && pIdxInfo->a_ترتیب_By[0].i_عمود==0
     && pIdxInfo->a_ترتیب_By[0].desc==0
     && pIdxInfo->a_ترتیب_By[1].i_عمود==1
     && pIdxInfo->a_ترتیب_By[1].desc==0
     )
  ){
    pIdxInfo->ترتیب_By_Consumed = 1;
    pIdxInfo->idx_عدد |= 0x08;
  }
  pIdxInfo->idxFlags |= SQLITE_اشاریہ_جائزہ_HEX;

  واپس SQLITE_OK;
}

/*
** Open a new DBSTAT cursor.
*/
ساکن صحیح statOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  Stat_جدول *pTab = (Stat_جدول *)pVTab;
  StatCursor *pCsr;

  pCsr = (StatCursor *)sqlite3_malloc64(حجم(StatCursor));
  اگر( pCsr==0 ){
    واپس SQLITE_NOMEM_BKPT;
  }ورنہ{
    memset(pCsr, 0, حجم(StatCursor));
    pCsr->بنیاد.pVtab = pVTab;
    pCsr->iDb = pTab->iDb;
  }

  *ppCursor = (sqlite3_vtab_cursor *)pCsr;
  واپس SQLITE_OK;
}

ساکن خالی statClearCells(Stat_صفحہ *p){
  صحیح i;
  اگر( p->aCell ){
    برائے(i=0; i<p->nCell; i++){
      sqlite3_free(p->aCell[i].aOvfl);
    }
    sqlite3_free(p->aCell);
  }
  p->nCell = 0;
  p->aCell = 0;
}

ساکن خالی stat_Clear_صفحہ(Stat_صفحہ *p){
  u8 *aPg = p->aPg;
  statClearCells(p);
  sqlite3_free(p->z_راستہ);
  memset(p, 0, حجم(Stat_صفحہ));
  p->aPg = aPg;
}

ساکن خالی statResetCsr(StatCursor *pCsr){
  صحیح i;
  /* In some circumstances, specifically if an OOM has occurred, the call
  ** to sqlite3_reset() may cause the pager to be reset (emptied). It is
  ** important that statClearPage() is called to free any page refs before
  ** this happens. dbsqlfuzz 9ed3e4e3816219d3509d711636c38542bf3f40b1. */
  برائے(i=0; i<Array_سائز(pCsr->a_صفحہ); i++){
    stat_Clear_صفحہ(&pCsr->a_صفحہ[i]);
    sqlite3_free(pCsr->a_صفحہ[i].aPg);
    pCsr->a_صفحہ[i].aPg = 0;
  }
  sqlite3_reset(pCsr->pStmt);
  pCsr->i_صفحہ = 0;
  sqlite3_free(pCsr->z_راستہ);
  pCsr->z_راستہ = 0;
  pCsr->isEof = 0;
}

/* Resize the space-used counters inside of the cursor */
ساکن خالی statResetCounts(StatCursor *pCsr){
  pCsr->nCell = 0;
  pCsr->nMxPayload = 0;
  pCsr->n_غیر_مستعمل = 0;
  pCsr->nPayload = 0;
  pCsr->sz_صفحہ = 0;
  pCsr->n_صفحہ = 0;
}

/*
** Close a DBSTAT cursor.
*/
ساکن صحیح statClose(sqlite3_vtab_cursor *pCursor){
  StatCursor *pCsr = (StatCursor *)pCursor;
  statResetCsr(pCsr);
  sqlite3_finalize(pCsr->pStmt);
  sqlite3_free(pCsr);
  واپس SQLITE_OK;
}

/*
** For a single cell on a btree page, compute the number of bytes of
** content (payload) stored on that page.  That is to say, compute the
** number of bytes of content not found on overflow pages.
*/
ساکن صحیح getLocalPayload(
  صحیح nUsable,                    /* Usable bytes per page */
  u8 flags,                       /* Page flags */
  صحیح n_کل                      /* Total record (payload) size */
){
  صحیح nLocal;
  صحیح nMinLocal;
  صحیح nMaxLocal;
 
  اگر( flags==0x0D ){              /* Table leaf node */
    nMinLocal = (nUsable - 12) * 32 / 255 - 23;
    nMaxLocal = nUsable - 35;
  }ورنہ{                          /* Index interior and leaf nodes */
    nMinLocal = (nUsable - 12) * 32 / 255 - 23;
    nMaxLocal = (nUsable - 12) * 64 / 255 - 23;
  }

  nLocal = nMinLocal + (n_کل - nMinLocal) % (nUsable - 4);
  اگر( nLocal>nMaxLocal ) nLocal = nMinLocal;
  واپس nLocal;
}

/* Populate the StatPage object with information about the all
** cells found on the page currently under analysis.
*/
ساکن صحیح stat_Decode_صفحہ(Btree *pBt, Stat_صفحہ *p){
  صحیح n_غیر_مستعمل;
  صحیح iOff;
  صحیح nHdr;
  صحیح is_برگ;
  صحیح sz_صفحہ;

  u8 *a_ڈیٹا = p->aPg;
  u8 *aHdr = &a_ڈیٹا[p->iPgno==1 ? 100 : 0];

  p->flags = aHdr[0];
  اگر( p->flags==0x0A || p->flags==0x0D ){
    is_برگ = 1;
    nHdr = 8;
  }ورنہ اگر( p->flags==0x05 || p->flags==0x02 ){
    is_برگ = 0;
    nHdr = 12;
  }ورنہ{
    جاؤ stat_صفحہ_Is_Corrupt;
  }
  اگر( p->iPgno==1 ) nHdr += 100;
  p->nCell = get2byte(&aHdr[3]);
  p->nMxPayload = 0;
  sz_صفحہ = sqlite3_Btree_Get_صفحہ_سائز(pBt);

  n_غیر_مستعمل = get2byte(&aHdr[5]) - nHdr - 2*p->nCell;
  n_غیر_مستعمل += (صحیح)aHdr[7];
  iOff = get2byte(&aHdr[1]);
  جب( iOff ){
    صحیح i_اگلا;
    اگر( iOff>=sz_صفحہ ) جاؤ stat_صفحہ_Is_Corrupt;
    n_غیر_مستعمل += get2byte(&a_ڈیٹا[iOff+2]);
    i_اگلا = get2byte(&a_ڈیٹا[iOff]);
    اگر( i_اگلا<iOff+4 && i_اگلا>0 ) جاؤ stat_صفحہ_Is_Corrupt;
    iOff = i_اگلا;
  }
  p->n_غیر_مستعمل = n_غیر_مستعمل;
  p->i_دایاں_بچہ_Pg = is_برگ ? 0 : sqlite3Get4byte(&aHdr[8]);

  اگر( p->nCell ){
    صحیح i;                        /* Used to iterate through cells */
    صحیح nUsable;                  /* Usable bytes per page */

    sqlite3BtreeEnter(pBt);
    nUsable = sz_صفحہ - sqlite3BtreeGetReserveNoMutex(pBt);
    sqlite3BtreeLeave(pBt);
    p->aCell = sqlite3_malloc64((p->nCell+1) * حجم(StatCell));
    اگر( p->aCell==0 ) واپس SQLITE_NOMEM_BKPT;
    memset(p->aCell, 0, (p->nCell+1) * حجم(StatCell));

    برائے(i=0; i<p->nCell; i++){
      StatCell *pCell = &p->aCell[i];

      iOff = get2byte(&a_ڈیٹا[nHdr+i*2]);
      اگر( iOff<nHdr || iOff>=sz_صفحہ ) جاؤ stat_صفحہ_Is_Corrupt;
      اگر( !is_برگ ){
        pCell->i_بچہ_Pg = sqlite3Get4byte(&a_ڈیٹا[iOff]);
        iOff += 4;
      }
      اگر( p->flags==0x05 ){
        /* A table interior node. nPayload==0. */
      }ورنہ{
        u32 nPayload;             /* Bytes of payload total (local+overflow) */
        صحیح nLocal;               /* Bytes of payload stored locally */
        iOff += getVarint32(&a_ڈیٹا[iOff], nPayload);
        اگر( p->flags==0x0D ){
          u64 dummy;
          iOff += sqlite3GetVarint(&a_ڈیٹا[iOff], &dummy);
        }
        اگر( nPayload>(u32)p->nMxPayload ) p->nMxPayload = nPayload;
        nLocal = getLocalPayload(nUsable, p->flags, nPayload);
        اگر( nLocal<0 ) جاؤ stat_صفحہ_Is_Corrupt;
        pCell->nLocal = nLocal;
        أكد( nPayload>=(u32)nLocal );
        أكد( nLocal<=(nUsable-35) );
        اگر( nPayload>(u32)nLocal ){
          صحیح j;
          صحیح nOvfl = ((nPayload - nLocal) + nUsable-4 - 1) / (nUsable - 4);
          اگر( iOff+nLocal+4>nUsable || nPayload>0x7fffffff ){
            جاؤ stat_صفحہ_Is_Corrupt;
          }
          pCell->n_آخری_Ovfl = (nPayload-nLocal) - (nOvfl-1) * (nUsable-4);
          pCell->nOvfl = nOvfl;
          pCell->aOvfl = sqlite3_malloc64(حجم(u32)*nOvfl);
          اگر( pCell->aOvfl==0 ) واپس SQLITE_NOMEM_BKPT;
          pCell->aOvfl[0] = sqlite3Get4byte(&a_ڈیٹا[iOff+nLocal]);
          برائے(j=1; j<nOvfl; j++){
            صحیح rc;
            u32 i_پچھلا = pCell->aOvfl[j-1];
            Db_صفحہ *pPg = 0;
            rc = sqlite3PagerGet(sqlite3BtreePager(pBt), i_پچھلا, &pPg, 0);
            اگر( rc!=SQLITE_OK ){
              أكد( pPg==0 );
              واپس rc;
            } 
            pCell->aOvfl[j] = sqlite3Get4byte(sqlite3_Pager_Get_ڈیٹا(pPg));
            sqlite3PagerUnref(pPg);
          }
        }
      }
    }
  }

  واپس SQLITE_OK;

stat_صفحہ_Is_Corrupt:
  p->flags = 0;
  statClearCells(p);
  واپس SQLITE_OK;
}

/*
** Populate the pCsr->iOffset and pCsr->szPage member variables. Based on
** the current value of pCsr->iPageno.
*/
ساکن خالی stat_سائز_And_انحراف(StatCursor *pCsr){
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
  اگر( sqlite3OsFileControl(fd, 230440, &x)==SQLITE_OK ){
    pCsr->i_انحراف = x[0];
    pCsr->sz_صفحہ += x[1];
  }ورنہ{
    /* Not ZIPVFS: The default page size and offset */
    pCsr->sz_صفحہ += sqlite3_Btree_Get_صفحہ_سائز(pBt);
    pCsr->i_انحراف = (i64)pCsr->sz_صفحہ * (pCsr->iPageno - 1);
  }
}

/*
** Load a copy of the page data for page iPg into the buffer belonging
** to page object pPg. Allocate the buffer if necessary. Return SQLITE_OK
** if successful, or an SQLite error code otherwise.
*/
ساکن صحیح stat_Get_صفحہ(
  Btree *pBt,                     /* Load page from this b-tree */
  u32 iPg,                        /* Page number to load */
  Stat_صفحہ *pPg                   /* Load page into this object */
){
  صحیح pgsz = sqlite3_Btree_Get_صفحہ_سائز(pBt);
  Db_صفحہ *p_Db_صفحہ = 0;
  صحیح rc;

  اگر( pPg->aPg==0 ){
    pPg->aPg = (u8*)sqlite3_malloc(pgsz + DBSTAT_صفحہ_PADDING_بائٹیں);
    اگر( pPg->aPg==0 ){
      واپس SQLITE_NOMEM_BKPT;
    }
    memset(&pPg->aPg[pgsz], 0, DBSTAT_صفحہ_PADDING_بائٹیں);
  }

  rc = sqlite3PagerGet(sqlite3BtreePager(pBt), iPg, &p_Db_صفحہ, 0);
  اگر( rc==SQLITE_OK ){
    ثابت u8 *a = sqlite3_Pager_Get_ڈیٹا(p_Db_صفحہ);
    حافظہ_نقل(pPg->aPg, a, pgsz);
    sqlite3PagerUnref(p_Db_صفحہ);
  }

  واپس rc;
}

/*
** Move a DBSTAT cursor to the next entry.  Normally, the next
** entry will be the next page, but in aggregated mode (pCsr->isAgg!=0),
** the next entry is the next btree.
*/
ساکن صحیح stat_اگلا(sqlite3_vtab_cursor *pCursor){
  صحیح rc;
  صحیح nPayload;
  حرف *z;
  StatCursor *pCsr = (StatCursor *)pCursor;
  Stat_جدول *pTab = (Stat_جدول *)pCursor->pVtab;
  Btree *pBt = pTab->db->aDb[pCsr->iDb].pBt;
  Pager *pPager = sqlite3BtreePager(pBt);

  sqlite3_free(pCsr->z_راستہ);
  pCsr->z_راستہ = 0;

stat_اگلا_Restart:
  اگر( pCsr->i_صفحہ<0 ){
    /* Start measuring space on the next btree */
    statResetCounts(pCsr);
    rc = sqlite3_قدم(pCsr->pStmt);
    اگر( rc==SQLITE_صف ){
      صحیح n_صفحہ;
      u32 i_جڑ = (u32)sqlite3_عمود_int64(pCsr->pStmt, 1);
      sqlite3PagerPagecount(pPager, &n_صفحہ);
      اگر( n_صفحہ==0 ){
        pCsr->isEof = 1;
        واپس sqlite3_reset(pCsr->pStmt);
      }
      rc = stat_Get_صفحہ(pBt, i_جڑ, &pCsr->a_صفحہ[0]);
      pCsr->a_صفحہ[0].iPgno = i_جڑ;
      pCsr->a_صفحہ[0].iCell = 0;
      اگر( !pCsr->isAgg ){
        pCsr->a_صفحہ[0].z_راستہ = z = sqlite3_mprintf("/");
        اگر( z==0 ) rc = SQLITE_NOMEM_BKPT;
      }
      pCsr->i_صفحہ = 0;
      pCsr->n_صفحہ = 1;
    }ورنہ{
      pCsr->isEof = 1;
      واپس sqlite3_reset(pCsr->pStmt);
    }
  }ورنہ{
    /* Continue analyzing the btree previously started */
    Stat_صفحہ *p = &pCsr->a_صفحہ[pCsr->i_صفحہ];
    اگر( !pCsr->isAgg ) statResetCounts(pCsr);
    جب( p->iCell<p->nCell ){
      StatCell *pCell = &p->aCell[p->iCell];
      جب( pCell->iOvfl<pCell->nOvfl ){
        صحیح nUsable;
        صحیح iOvfl = pCell->iOvfl;
        sqlite3BtreeEnter(pBt);
        nUsable = sqlite3_Btree_Get_صفحہ_سائز(pBt) - 
                        sqlite3BtreeGetReserveNoMutex(pBt);
        sqlite3BtreeLeave(pBt);
        pCsr->n_صفحہ++;
        pCsr->iPageno = pCell->aOvfl[iOvfl];
        stat_سائز_And_انحراف(pCsr);
        اگر( pCell->iOvfl<pCell->nOvfl-1 ){
          pCsr->nPayload += nUsable - 4;
        }ورنہ{
          pCsr->nPayload += pCell->n_آخری_Ovfl;
          pCsr->n_غیر_مستعمل += nUsable - 4 - pCell->n_آخری_Ovfl;
        }
        pCell->iOvfl++;
        اگر( !pCsr->isAgg ){
          pCsr->zName = (حرف *)sqlite3_عمود_متن(pCsr->pStmt, 0);
          pCsr->zPagetype = "overflow";
          pCsr->z_راستہ = z = sqlite3_mprintf(
              "%s%.3x+%.6x", p->z_راستہ, p->iCell, iOvfl
          );
          واپس z==0 ? SQLITE_NOMEM_BKPT : SQLITE_OK;
        }
      }
      اگر( p->i_دایاں_بچہ_Pg ) توڑو;
      p->iCell++;
    }

    اگر( !p->i_دایاں_بچہ_Pg || p->iCell>p->nCell ){
      stat_Clear_صفحہ(p);
      pCsr->i_صفحہ--;
      اگر( pCsr->isAgg && pCsr->i_صفحہ<0 ){
        /* label-statNext-done:  When computing aggregate space usage over
        ** an entire btree, this is the exit point from this function */
        واپس SQLITE_OK;
      }
      جاؤ stat_اگلا_Restart; /* Tail recursion */
    }
    pCsr->i_صفحہ++;
    اگر( pCsr->i_صفحہ>=Array_سائز(pCsr->a_صفحہ) ){
      statResetCsr(pCsr);
      واپس SQLITE_CORRUPT_BKPT;
    }
    أكد( p==&pCsr->a_صفحہ[pCsr->i_صفحہ-1] );

    اگر( p->iCell==p->nCell ){
      p[1].iPgno = p->i_دایاں_بچہ_Pg;
    }ورنہ{
      p[1].iPgno = p->aCell[p->iCell].i_بچہ_Pg;
    }
    rc = stat_Get_صفحہ(pBt, p[1].iPgno, &p[1]);
    pCsr->n_صفحہ++;
    p[1].iCell = 0;
    اگر( !pCsr->isAgg ){
      p[1].z_راستہ = z = sqlite3_mprintf("%s%.3x/", p->z_راستہ, p->iCell);
      اگر( z==0 ) rc = SQLITE_NOMEM_BKPT;
    }
    p->iCell++;
  }


  /* Populate the StatCursor fields with the values to be returned
  ** by the xColumn() and xRowid() methods.
  */
  اگر( rc==SQLITE_OK ){
    صحیح i;
    Stat_صفحہ *p = &pCsr->a_صفحہ[pCsr->i_صفحہ];
    pCsr->zName = (حرف *)sqlite3_عمود_متن(pCsr->pStmt, 0);
    pCsr->iPageno = p->iPgno;

    rc = stat_Decode_صفحہ(pBt, p);
    اگر( rc==SQLITE_OK ){
      stat_سائز_And_انحراف(pCsr);

      انتخاب( p->flags ){
        حالت 0x05:             /* table internal */
        حالت 0x02:             /* index internal */
          pCsr->zPagetype = "internal";
          توڑو;
        حالت 0x0D:             /* table leaf */
        حالت 0x0A:             /* index leaf */
          pCsr->zPagetype = "leaf";
          توڑو;
        طےشدہ:
          pCsr->zPagetype = "corrupted";
          توڑو;
      }
      pCsr->nCell += p->nCell;
      pCsr->n_غیر_مستعمل += p->n_غیر_مستعمل;
      اگر( p->nMxPayload>pCsr->nMxPayload ) pCsr->nMxPayload = p->nMxPayload;
      اگر( !pCsr->isAgg ){
        pCsr->z_راستہ = z = sqlite3_mprintf("%s", p->z_راستہ);
        اگر( z==0 ) rc = SQLITE_NOMEM_BKPT;
      }
      nPayload = 0;
      برائے(i=0; i<p->nCell; i++){
        nPayload += p->aCell[i].nLocal;
      }
      pCsr->nPayload += nPayload;

      /* If computing aggregate space usage by btree, continue with the
      ** next page.  The loop will exit via the return at label-statNext-done
      */
      اگر( pCsr->isAgg ) جاؤ stat_اگلا_Restart;
    }
  }

  واپس rc;
}

ساکن صحیح statEof(sqlite3_vtab_cursor *pCursor){
  StatCursor *pCsr = (StatCursor *)pCursor;
  واپس pCsr->isEof;
}

/* Initialize a cursor according to the query plan idxNum using the
** arguments in argv[0].  See statBestIndex() for a description of the
** meaning of the bits in idxNum.
*/
ساکن صحیح statFilter(
  sqlite3_vtab_cursor *pCursor, 
  صحیح idx_عدد, ثابت حرف *idxStr,
  صحیح argc, sqlite3_قدر **argv
){
  StatCursor *pCsr = (StatCursor *)pCursor;
  Stat_جدول *pTab = (Stat_جدول*)(pCursor->pVtab);
  sqlite3_str *pSql;      /* Query of btrees to analyze */
  حرف *zSql;             /* String value of pSql */
  صحیح iArg = 0;           /* Count of argv[] parameters used so far */
  صحیح rc = SQLITE_OK;     /* Result of this operation */
  ثابت حرف *zName = 0;  /* Only provide analysis of this table */
  (خالی)argc;
  (خالی)idxStr;

  statResetCsr(pCsr);
  sqlite3_finalize(pCsr->pStmt);
  pCsr->pStmt = 0;
  اگر( idx_عدد & 0x01 ){
    /* schema=? constraint is present.  Get its value */
    ثابت حرف *zDbase = (ثابت حرف*)sqlite3_قدر_متن(argv[iArg++]);
    pCsr->iDb = sqlite3_ڈھونڈو_Db_Name(pTab->db, zDbase);
    اگر( pCsr->iDb<0 ){
      pCsr->iDb = 0;
      pCsr->isEof = 1;
      واپس SQLITE_OK;
    }
  }ورنہ{
    pCsr->iDb = pTab->iDb;
  }
  اگر( idx_عدد & 0x02 ){
    /* name=? constraint is present */
    zName = (ثابت حرف*)sqlite3_قدر_متن(argv[iArg++]);
  }
  اگر( idx_عدد & 0x04 ){
    /* aggregate=? constraint is present */
    pCsr->isAgg = sqlite3_قدر_double(argv[iArg++])!=0.0;
  }ورنہ{
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
  اگر( idx_عدد & 0x08 ){
    sqlite3_str_appendf(pSql, " ORDER BY name");
  }
  zSql = sqlite3_str_اختتام(pSql);
  اگر( zSql==0 ){
    واپس SQLITE_NOMEM_BKPT;
  }ورنہ{
    rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pStmt, 0);
    sqlite3_free(zSql);
  }

  اگر( rc==SQLITE_OK ){
    pCsr->i_صفحہ = -1;
    rc = stat_اگلا(pCursor);
  }
  واپس rc;
}

ساکن صحیح stat_عمود(
  sqlite3_vtab_cursor *pCursor, 
  sqlite3_context *ctx, 
  صحیح i
){
  StatCursor *pCsr = (StatCursor *)pCursor;
  انتخاب( i ){
    حالت 0:            /* name */
      sqlite3_نتیجہ_متن(ctx, pCsr->zName, -1, SQLITE_TRANSIENT);
      توڑو;
    حالت 1:            /*راستہ*/
      اگر( !pCsr->isAgg ){
        sqlite3_نتیجہ_متن(ctx, pCsr->z_راستہ, -1, SQLITE_TRANSIENT);
      }
      توڑو;
    حالت 2:            /* pageno */
      اگر( pCsr->isAgg ){
        sqlite3_نتیجہ_int64(ctx, pCsr->n_صفحہ);
      }ورنہ{
        sqlite3_نتیجہ_int64(ctx, pCsr->iPageno);
      }
      توڑو;
    حالت 3:            /* pagetype */
      اگر( !pCsr->isAgg ){
        sqlite3_نتیجہ_متن(ctx, pCsr->zPagetype, -1, SQLITE_STATIC);
      }
      توڑو;
    حالت 4:            /* ncell */
      sqlite3_نتیجہ_int64(ctx, pCsr->nCell);
      توڑو;
    حالت 5:            /* payload */
      sqlite3_نتیجہ_int64(ctx, pCsr->nPayload);
      توڑو;
    حالت 6:            /*غیر_مستعمل*/
      sqlite3_نتیجہ_int64(ctx, pCsr->n_غیر_مستعمل);
      توڑو;
    حالت 7:            /* mx_payload */
      sqlite3_نتیجہ_int64(ctx, pCsr->nMxPayload);
      توڑو;
    حالت 8:            /* pgoffset */
      اگر( !pCsr->isAgg ){
        sqlite3_نتیجہ_int64(ctx, pCsr->i_انحراف);
      }
      توڑو;
    حالت 9:            /* pgsize */
      sqlite3_نتیجہ_int64(ctx, pCsr->sz_صفحہ);
      توڑو;
    حالت 10: {         /* schema */
      sqlite3 *db = sqlite3_context_db_handle(ctx);
      صحیح iDb = pCsr->iDb;
      sqlite3_نتیجہ_متن(ctx, db->aDb[iDb].zDbSName, -1, SQLITE_STATIC);
      توڑو;
    }
    طےشدہ: {         /* aggregate */
      sqlite3_نتیجہ_int(ctx, pCsr->isAgg);
      توڑو;
    }
  }
  واپس SQLITE_OK;
}

ساکن صحیح statRowid(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid){
  StatCursor *pCsr = (StatCursor *)pCursor;
  *pRowid = pCsr->iPageno;
  واپس SQLITE_OK;
}

/*
** Invoke this routine to register the "dbstat" virtual table module
*/
صحیح sqlite3_Dbstat_رجسٹر(sqlite3 *db){
  ساکن sqlite3_module dbstat_module = {
    0,                            /* iVersion */
    statConnect,                  /* xCreate */
    statConnect,                  /* xConnect */
    stat_بہترین_اشاریہ,                /* xBestIndex */
    statDisconnect,               /* xDisconnect */
    statDisconnect,               /* xDestroy */
    statOpen,                     /* xOpen - open a cursor */
    statClose,                    /* xClose - close a cursor */
    statFilter,                   /* xFilter - configure scan constraints */
    stat_اگلا,                     /* xNext - advance a cursor */
    statEof,                      /* xEof - check for end of scan */
    stat_عمود,                   /* xColumn - read data */
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
  واپس sqlite3_create_module(db, "dbstat", &dbstat_module, 0);
}
#وگرنہ_اگر defined(SQLITE_ENABLE_DBSTAT_VTAB)
صحیح sqlite3_Dbstat_رجسٹر(sqlite3 *db){ واپس SQLITE_OK; }
#اگر_ختم /* SQLITE_ENABLE_DBSTAT_VTAB */
