/*
** 2017-10-11
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
** This file contains an implementation of the "sqlite_dbpage" virtual table.
**
** The sqlite_dbpage virtual table is used to read or write whole raw
** pages of the database file.  The pager interface is used so that 
** uncommitted changes and changes recorded in the WAL file are correctly
** retrieved.
**
** Usage example:
**
**    SELECT data FROM sqlite_dbpage('aux1') WHERE pgno=123;
**
** This is an eponymous virtual table so it does not need to be created before
** use.  The optional argument to the sqlite_dbpage() table name is the
** schema for the database file that is to be read.  The default schema is
** "main".
**
** The data field of sqlite_dbpage table can be updated.  The new
** value must be a BLOB which is the correct page size, otherwise the
** update fails.  INSERT operations also work, and operate as if they
** where REPLACE.  The size of the database can be extended by INSERT-ing
** new pages on the end.
**
** Rows may not be deleted.  However, doing an INSERT to page number N
** with NULL page data causes the N-th page and all subsequent pages to be
** deleted and the database to be truncated.
*/

#ضمن "sqliteInt.h"   /* Requires access to internal data structures */
#إذا (defined(SQLITE_ENABLE_DBPAGE_VTAB) || defined(SQLITE_TEST)) \
    && !defined(SQLITE_OMIT_VIRTUALTABLE)

تعريفنوع بنية Dbpage_جدول Dbpage_جدول;
تعريفنوع بنية DbpageCursor DbpageCursor;

بنية DbpageCursor {
  sqlite3_vtab_cursor پایه;       /* Base class.  Must be first */
  Pgno pgno;                      /* Current page number */
  Pgno mxPgno;                    /* Last page to visit on this scan */
  Pager *pPager;                  /* Pager being read/written */
  Db_صفحه *pPage1;                 /* Page 1 of the database */
  صحيح iDb;                        /* Index of database to analyze */
  صحيح sz_صفحه;                     /* Size of each page in bytes */
};

بنية Dbpage_جدول {
  sqlite3_vtab پایه;              /* Base class.  Must be first */
  sqlite3 *db;                    /* The database */
  صحيح iDbTrunc;                   /* Database to truncate */
  Pgno pgnoTrunc;                 /* Size to truncate to */
};

/* Columns */
#تعریف_کن DBPAGE_ستونی_PGNO    0
#تعریف_کن DBPAGE_ستونی_داده    1
#تعریف_کن DBPAGE_ستونی_SCHEMA  2


/*
** Connect to or create a dbpagevfs virtual table.
*/
ساكن صحيح dbpageConnect(
  sqlite3 *db,
  فراغ *pAux,
  صحيح argc, ثابت حرف *ثابت*argv,
  sqlite3_vtab **ppVtab,
  حرف **pzErr
){
  Dbpage_جدول *pTab = 0;
  صحيح rc = SQLITE_OK;
  (فراغ)pAux;
  (فراغ)argc;
  (فراغ)argv;
  (فراغ)pzErr;

  sqlite3_vtab_config(db, SQLITE_VTAB_DIRECTONLY);
  sqlite3_vtab_config(db, SQLITE_VTAB_استفاده_میکند_ALL_SCHEMAS);
  rc = sqlite3_declare_vtab(db, 
          "CREATE TABLE x(pgno INTEGER PRIMARY KEY, data BLOB, schema HIDDEN)");
  إذا( rc==SQLITE_OK ){
    pTab = (Dbpage_جدول *)sqlite3_malloc64(حجم(Dbpage_جدول));
    إذا( pTab==0 ) rc = SQLITE_NOMEM_BKPT;
  }

  أكد( rc==SQLITE_OK || pTab==0 );
  إذا( rc==SQLITE_OK ){
    memset(pTab, 0, حجم(Dbpage_جدول));
    pTab->db = db;
  }

  *ppVtab = (sqlite3_vtab*)pTab;
  أرجع rc;
}

/*
** Disconnect from or destroy a dbpagevfs virtual table.
*/
ساكن صحيح dbpageDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  أرجع SQLITE_OK;
}

/*
** idxNum:
**
**     0     schema=main, full table scan
**     1     schema=main, pgno=?1
**     2     schema=?1, full table scan
**     3     schema=?1, pgno=?2
*/
ساكن صحيح dbpage_بهترین_شاخص(sqlite3_vtab *tab, sqlite3_شاخص_info *pIdxInfo){
  صحيح i;
  صحيح iPlan = 0;
  (فراغ)tab;

  /* If there is a schema= constraint, it must be honored.  Report a
  ** ridiculously large estimated cost if the schema= constraint is
  ** unavailable
  */
  لكل(i=0; i<pIdxInfo->nConstraint; i++){
    بنية sqlite3_شاخص_constraint *p = &pIdxInfo->aConstraint[i];
    إذا( p->i_ستونی!=DBPAGE_ستونی_SCHEMA ) استمر;
    إذا( p->op!=SQLITE_شاخص_CONSTRAINT_EQ ) استمر;
    إذا( !p->usable ){
      /* No solution. */
      أرجع SQLITE_CONSTRAINT;
    }
    iPlan = 2;
    pIdxInfo->aConstraintUsage[i].argv_شاخص = 1;
    pIdxInfo->aConstraintUsage[i].omit = 1;
    اكسر;
  }

  /* If we reach this point, it means that either there is no schema=
  ** constraint (in which case we use the "main" schema) or else the
  ** schema constraint was accepted.  Lower the estimated cost accordingly
  */
  pIdxInfo->estimated_هزینه = 1.0e6;

  /* Check for constraints against pgno */
  لكل(i=0; i<pIdxInfo->nConstraint; i++){
    بنية sqlite3_شاخص_constraint *p = &pIdxInfo->aConstraint[i];
    إذا( p->usable && p->i_ستونی<=0 && p->op==SQLITE_شاخص_CONSTRAINT_EQ ){
      pIdxInfo->estimated_ردیفها = 1;
      pIdxInfo->idxFlags = SQLITE_شاخص_پیمایش_UNIQUE;
      pIdxInfo->estimated_هزینه = 1.0;
      pIdxInfo->aConstraintUsage[i].argv_شاخص = iPlan ? 2 : 1;
      pIdxInfo->aConstraintUsage[i].omit = 1;
      iPlan |= 1;
      اكسر;
    }
  }
  pIdxInfo->idx_رقم = iPlan;

  إذا( pIdxInfo->n_ترتیب_By>=1
   && pIdxInfo->a_ترتیب_By[0].i_ستونی<=0
   && pIdxInfo->a_ترتیب_By[0].desc==0
  ){
    pIdxInfo->ترتیب_By_Consumed = 1;
  }
  أرجع SQLITE_OK;
}

/*
** Open a new dbpagevfs cursor.
*/
ساكن صحيح dbpageOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  DbpageCursor *pCsr;

  pCsr = (DbpageCursor *)sqlite3_malloc64(حجم(DbpageCursor));
  إذا( pCsr==0 ){
    أرجع SQLITE_NOMEM_BKPT;
  }وإلا{
    memset(pCsr, 0, حجم(DbpageCursor));
    pCsr->پایه.pVtab = pVTab;
    pCsr->pgno = 0;
  }

  *ppCursor = (sqlite3_vtab_cursor *)pCsr;
  أرجع SQLITE_OK;
}

/*
** Close a dbpagevfs cursor.
*/
ساكن صحيح dbpageClose(sqlite3_vtab_cursor *pCursor){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  إذا( pCsr->pPage1 ) sqlite3_Pager_Unref_صفحه_One(pCsr->pPage1);
  sqlite3_free(pCsr);
  أرجع SQLITE_OK;
}

/*
** Move a dbpagevfs cursor to the next entry in the file.
*/
ساكن صحيح dbpage_بعدی(sqlite3_vtab_cursor *pCursor){
  صحيح rc = SQLITE_OK;
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  pCsr->pgno++;
  أرجع rc;
}

ساكن صحيح dbpageEof(sqlite3_vtab_cursor *pCursor){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  أرجع pCsr->pgno > pCsr->mxPgno;
}

/*
** idxNum:
**
**     0     schema=main, full table scan
**     1     schema=main, pgno=?1
**     2     schema=?1, full table scan
**     3     schema=?1, pgno=?2
**
** idxStr is not used
*/
ساكن صحيح dbpageFilter(
  sqlite3_vtab_cursor *pCursor,
  صحيح idx_رقم, ثابت حرف *idxStr,
  صحيح argc, sqlite3_مقدار **argv
){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  Dbpage_جدول *pTab = (Dbpage_جدول *)pCursor->pVtab;
  صحيح rc;
  sqlite3 *db = pTab->db;
  Btree *pBt;

  استفادهنشده_PARAMETER(idxStr);
  استفادهنشده_PARAMETER(argc);

  /* Default setting is no rows of result */
  pCsr->pgno = 1;
  pCsr->mxPgno = 0;

  إذا( idx_رقم & 2 ){
    ثابت حرف *zSchema;
    أكد( argc>=1 );
    zSchema = (ثابت حرف*)sqlite3_مقدار_متن(argv[0]);
    pCsr->iDb = sqlite3_پیدا_کن_Db_Name(db, zSchema);
    إذا( pCsr->iDb<0 ) أرجع SQLITE_OK;
  }وإلا{
    pCsr->iDb = 0;
  }
  pBt = db->aDb[pCsr->iDb].pBt;
  إذا( NEVER(pBt==0) ) أرجع SQLITE_OK;
  pCsr->pPager = sqlite3BtreePager(pBt);
  pCsr->sz_صفحه = sqlite3_Btree_Get_صفحه_اندازه(pBt);
  pCsr->mxPgno = sqlite3_Btree_آخرین_صفحه(pBt);
  إذا( idx_رقم & 1 ){
    i64 iPg = sqlite3_مقدار_int64(argv[idx_رقم>>1]);
    أكد( argc>(idx_رقم>>1) );
    إذا( iPg<1 || iPg>pCsr->mxPgno ){
      pCsr->pgno = 1;
      pCsr->mxPgno = 0;
    }وإلا{
      pCsr->pgno = (Pgno)iPg;
      pCsr->mxPgno = pCsr->pgno;
    }
  }وإلا{
    أكد( pCsr->pgno==1 );
  }
  إذا( pCsr->pPage1 ) sqlite3_Pager_Unref_صفحه_One(pCsr->pPage1);
  rc = sqlite3PagerGet(pCsr->pPager, 1, &pCsr->pPage1, 0);
  أرجع rc;
}

ساكن صحيح dbpage_ستونی(
  sqlite3_vtab_cursor *pCursor,
  sqlite3_context *ctx,
  صحيح i
){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  صحيح rc = SQLITE_OK;
  بدل( i ){
    حالة 0: {           /* pgno */
      sqlite3_نتیجه_int64(ctx, (sqlite3_int64)pCsr->pgno);
      اكسر;
    }
    حالة 1: {           /*داده*/
      Db_صفحه *p_Db_صفحه = 0;
      إذا( pCsr->pgno==(Pgno)((PENDING_BYTE/pCsr->sz_صفحه)+1) ){
        /* The pending byte page. Assume it is zeroed out. Attempting to
        ** request this page from the page is an SQLITE_CORRUPT error. */
        sqlite3_نتیجه_zeroblob(ctx, pCsr->sz_صفحه);
      }وإلا{
        rc = sqlite3PagerGet(pCsr->pPager, pCsr->pgno, (Db_صفحه**)&p_Db_صفحه, 0);
        إذا( rc==SQLITE_OK ){
          sqlite3_نتیجه_blob(ctx, sqlite3_Pager_Get_داده(p_Db_صفحه), pCsr->sz_صفحه,
              SQLITE_TRANSIENT);
        }
        sqlite3PagerUnref(p_Db_صفحه);
      }
      اكسر;
    }
    افتراضي: {          /* schema */
      sqlite3 *db = sqlite3_context_db_handle(ctx);
      sqlite3_نتیجه_متن(ctx, db->aDb[pCsr->iDb].zDbSName, -1, SQLITE_STATIC);
      اكسر;
    }
  }
  أرجع rc;
}

ساكن صحيح dbpageRowid(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  *pRowid = pCsr->pgno;
  أرجع SQLITE_OK;
}

/* 
** Open write transactions. Since we do not know in advance which database
** files will be written by the sqlite_dbpage virtual table, start a write
** transaction on them all.
**
** Return SQLITE_OK if successful, or an SQLite error code otherwise.
*/
ساكن صحيح dbpageBeginTrans(Dbpage_جدول *pTab){
  sqlite3 *db = pTab->db;
  صحيح rc = SQLITE_OK;
  صحيح i;
  لكل(i=0; rc==SQLITE_OK && i<db->nDb; i++){
    Btree *pBt = db->aDb[i].pBt;
    إذا( pBt ) rc = sqlite3BtreeBeginTrans(pBt, 1, 0);
  }
  أرجع rc;
}

ساكن صحيح dbpageUpdate(
  sqlite3_vtab *pVtab,
  صحيح argc,
  sqlite3_مقدار **argv,
  sqlite_int64 *pRowid
){
  Dbpage_جدول *pTab = (Dbpage_جدول *)pVtab;
  Pgno pgno;
  sqlite3_int64 pgno64;
  Db_صفحه *p_Db_صفحه = 0;
  صحيح rc = SQLITE_OK;
  حرف *zErr = 0;
  صحيح iDb;
  Btree *pBt;
  Pager *pPager;
  صحيح sz_صفحه;
  صحيح is_درج;

  (فراغ)pRowid;
  إذا( pTab->db->flags & SQLITE_Defensive ){
    zErr = "read-only";
    اذهب update_fail;
  }
  إذا( argc==1 ){
    zErr = "cannot delete";
    اذهب update_fail;
  }
  إذا( sqlite3_مقدار_type(argv[0])==SQLITE_NULL ){
    pgno64 = sqlite3_مقدار_int64(argv[2]);
    is_درج = 1;
  }وإلا{
    pgno64 = (Pgno)sqlite3_مقدار_int64(argv[0]);
    إذا( sqlite3_مقدار_int64(argv[1])!=pgno64 ){
      zErr = "cannot insert";
      اذهب update_fail;
    }
    is_درج = 0;
  }
  إذا( sqlite3_مقدار_type(argv[4])==SQLITE_NULL ){
    iDb = 0;
  }وإلا{
    ثابت حرف *zSchema = (ثابت حرف*)sqlite3_مقدار_متن(argv[4]);
    iDb = sqlite3_پیدا_کن_Db_Name(pTab->db, zSchema);
    إذا( iDb<0 ){
      zErr = "no such schema";
      اذهب update_fail;
    }
  }
  pBt = pTab->db->aDb[iDb].pBt;
  إذا( pgno64<1 || pgno64>4294967294U || NEVER(pBt==0) ){
    zErr = "bad page number";
    اذهب update_fail;
  }
  pgno = (Pgno)pgno64;
  sz_صفحه = sqlite3_Btree_Get_صفحه_اندازه(pBt);
  إذا( sqlite3_مقدار_type(argv[3])!=SQLITE_BLOB 
   || sqlite3_مقدار_بایتها(argv[3])!=sz_صفحه
  ){
    إذا( sqlite3_مقدار_type(argv[3])==SQLITE_NULL && is_درج && pgno>1 ){
      /* "INSERT INTO dbpage($PGNO,NULL)" causes page number $PGNO and
      ** all subsequent pages to be deleted. */
      pTab->iDbTrunc = iDb;
      pTab->pgnoTrunc = pgno-1;
      pgno = 1;
    }وإلا{
      zErr = "bad page value";
      اذهب update_fail;
    }
  }

  إذا( dbpageBeginTrans(pTab)!=SQLITE_OK ){
    zErr = "failed to open transaction";
    اذهب update_fail;
  }

  pPager = sqlite3BtreePager(pBt);
  rc = sqlite3PagerGet(pPager, pgno, (Db_صفحه**)&p_Db_صفحه, 0);
  إذا( rc==SQLITE_OK ){
    ثابت فراغ *p_داده = sqlite3_مقدار_blob(argv[3]);
    إذا( (rc = sqlite3PagerWrite(p_Db_صفحه))==SQLITE_OK && p_داده ){
      بیعلامت حرف *a_صفحه = sqlite3_Pager_Get_داده(p_Db_صفحه);
      کپی_حافظه(a_صفحه, p_داده, sz_صفحه);
      pTab->pgnoTrunc = 0;
    }
  }
  إذا( rc!=SQLITE_OK ){
    pTab->pgnoTrunc = 0;
  }
  sqlite3PagerUnref(p_Db_صفحه);
  أرجع rc;

update_fail:
  pTab->pgnoTrunc = 0;
  sqlite3_free(pVtab->z_Err_پیام);
  pVtab->z_Err_پیام = sqlite3_mprintf("%s", zErr);
  أرجع SQLITE_ERROR;
}

ساكن صحيح dbpageBegin(sqlite3_vtab *pVtab){
  Dbpage_جدول *pTab = (Dbpage_جدول *)pVtab;
  pTab->pgnoTrunc = 0;
  أرجع SQLITE_OK;
}

/* Invoke sqlite3PagerTruncate() as necessary, just prior to COMMIT
*/
ساكن صحيح dbpageSync(sqlite3_vtab *pVtab){
  Dbpage_جدول *pTab = (Dbpage_جدول *)pVtab;
  إذا( pTab->pgnoTrunc>0 ){
    Btree *pBt = pTab->db->aDb[pTab->iDbTrunc].pBt;
    Pager *pPager = sqlite3BtreePager(pBt);
    sqlite3BtreeEnter(pBt);
    إذا( pTab->pgnoTrunc<sqlite3_Btree_آخرین_صفحه(pBt) ){
      sqlite3PagerTruncateImage(pPager, pTab->pgnoTrunc);
    }
    sqlite3BtreeLeave(pBt);
  }
  pTab->pgnoTrunc = 0;
  أرجع SQLITE_OK;
}

/* Cancel any pending truncate.
*/
ساكن صحيح dbpage_Rollback_مقصد(sqlite3_vtab *pVtab, صحيح notUsed1){
  Dbpage_جدول *pTab = (Dbpage_جدول *)pVtab;
  pTab->pgnoTrunc = 0;
  (فراغ)notUsed1;
  أرجع SQLITE_OK;
}

/*
** Invoke this routine to register the "dbpage" virtual table module
*/
صحيح sqlite3_Dbpage_ثبات(sqlite3 *db){
  ساكن sqlite3_module dbpage_module = {
    2,                            /* iVersion */
    dbpageConnect,                /* xCreate */
    dbpageConnect,                /* xConnect */
    dbpage_بهترین_شاخص,              /* xBestIndex */
    dbpageDisconnect,             /* xDisconnect */
    dbpageDisconnect,             /* xDestroy */
    dbpageOpen,                   /* xOpen - open a cursor */
    dbpageClose,                  /* xClose - close a cursor */
    dbpageFilter,                 /* xFilter - configure scan constraints */
    dbpage_بعدی,                   /* xNext - advance a cursor */
    dbpageEof,                    /* xEof - check for end of scan */
    dbpage_ستونی,                 /* xColumn - read data */
    dbpageRowid,                  /* xRowid - read data */
    dbpageUpdate,                 /* xUpdate */
    dbpageBegin,                  /* xBegin */
    dbpageSync,                   /* xSync */
    0,                            /* xCommit */
    0,                            /* xRollback */
    0,                            /* xFindMethod */
    0,                            /* xRename */
    0,                            /* xSavepoint */
    0,                            /* xRelease */
    dbpage_Rollback_مقصد,             /* xRollbackTo */
    0,                            /* xShadowName */
    0                             /* xIntegrity */
  };
  أرجع sqlite3_create_module(db, "sqlite_dbpage", &dbpage_module, 0);
}
#وإلاإذا defined(SQLITE_ENABLE_DBPAGE_VTAB)
صحيح sqlite3_Dbpage_ثبات(sqlite3 *db){ أرجع SQLITE_OK; }
#پایان_اگر /* SQLITE_ENABLE_DBSTAT_VTAB */
