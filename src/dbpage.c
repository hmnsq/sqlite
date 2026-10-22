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

#شامل "sqliteInt.h"   /* Requires access to internal data structures */
#اگر (defined(SQLITE_ENABLE_DBPAGE_VTAB) || defined(SQLITE_TEST)) \
    && !defined(SQLITE_OMIT_VIRTUALTABLE)

قسمتعریف ساخت Dbpage_جدول Dbpage_جدول;
قسمتعریف ساخت DbpageCursor DbpageCursor;

ساخت DbpageCursor {
  sqlite3_vtab_cursor بنیاد;       /* Base class.  Must be first */
  Pgno pgno;                      /* Current page number */
  Pgno mxPgno;                    /* Last page to visit on this scan */
  Pager *pPager;                  /* Pager being read/written */
  Db_صفحہ *pPage1;                 /* Page 1 of the database */
  صحیح iDb;                        /* Index of database to analyze */
  صحیح sz_صفحہ;                     /* Size of each page in bytes */
};

ساخت Dbpage_جدول {
  sqlite3_vtab بنیاد;              /* Base class.  Must be first */
  sqlite3 *db;                    /* The database */
  صحیح iDbTrunc;                   /* Database to truncate */
  Pgno pgnoTrunc;                 /* Size to truncate to */
};

/* Columns */
#تعریف_کرو DBPAGE_عمود_PGNO    0
#تعریف_کرو DBPAGE_عمود_ڈیٹا    1
#تعریف_کرو DBPAGE_عمود_SCHEMA  2


/*
** Connect to or create a dbpagevfs virtual table.
*/
ساکن صحیح dbpageConnect(
  sqlite3 *db,
  خالی *pAux,
  صحیح argc, ثابت حرف *ثابت*argv,
  sqlite3_vtab **ppVtab,
  حرف **pzErr
){
  Dbpage_جدول *pTab = 0;
  صحیح rc = SQLITE_OK;
  (خالی)pAux;
  (خالی)argc;
  (خالی)argv;
  (خالی)pzErr;

  sqlite3_vtab_config(db, SQLITE_VTAB_DIRECTONLY);
  sqlite3_vtab_config(db, SQLITE_VTAB_استعمال_کرتا_ALL_SCHEMAS);
  rc = sqlite3_declare_vtab(db, 
          "CREATE TABLE x(pgno INTEGER PRIMARY KEY, data BLOB, schema HIDDEN)");
  اگر( rc==SQLITE_OK ){
    pTab = (Dbpage_جدول *)sqlite3_malloc64(حجم(Dbpage_جدول));
    اگر( pTab==0 ) rc = SQLITE_NOMEM_BKPT;
  }

  أكد( rc==SQLITE_OK || pTab==0 );
  اگر( rc==SQLITE_OK ){
    memset(pTab, 0, حجم(Dbpage_جدول));
    pTab->db = db;
  }

  *ppVtab = (sqlite3_vtab*)pTab;
  واپس rc;
}

/*
** Disconnect from or destroy a dbpagevfs virtual table.
*/
ساکن صحیح dbpageDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  واپس SQLITE_OK;
}

/*
** idxNum:
**
**     0     schema=main, full table scan
**     1     schema=main, pgno=?1
**     2     schema=?1, full table scan
**     3     schema=?1, pgno=?2
*/
ساکن صحیح dbpage_بہترین_اشاریہ(sqlite3_vtab *tab, sqlite3_اشاریہ_info *pIdxInfo){
  صحیح i;
  صحیح iPlan = 0;
  (خالی)tab;

  /* If there is a schema= constraint, it must be honored.  Report a
  ** ridiculously large estimated cost if the schema= constraint is
  ** unavailable
  */
  برائے(i=0; i<pIdxInfo->nConstraint; i++){
    ساخت sqlite3_اشاریہ_constraint *p = &pIdxInfo->aConstraint[i];
    اگر( p->i_عمود!=DBPAGE_عمود_SCHEMA ) جاری;
    اگر( p->op!=SQLITE_اشاریہ_CONSTRAINT_EQ ) جاری;
    اگر( !p->usable ){
      /* No solution. */
      واپس SQLITE_CONSTRAINT;
    }
    iPlan = 2;
    pIdxInfo->aConstraintUsage[i].argv_اشاریہ = 1;
    pIdxInfo->aConstraintUsage[i].omit = 1;
    توڑو;
  }

  /* If we reach this point, it means that either there is no schema=
  ** constraint (in which case we use the "main" schema) or else the
  ** schema constraint was accepted.  Lower the estimated cost accordingly
  */
  pIdxInfo->estimated_لاگت = 1.0e6;

  /* Check for constraints against pgno */
  برائے(i=0; i<pIdxInfo->nConstraint; i++){
    ساخت sqlite3_اشاریہ_constraint *p = &pIdxInfo->aConstraint[i];
    اگر( p->usable && p->i_عمود<=0 && p->op==SQLITE_اشاریہ_CONSTRAINT_EQ ){
      pIdxInfo->estimated_صفیں = 1;
      pIdxInfo->idxFlags = SQLITE_اشاریہ_جائزہ_UNIQUE;
      pIdxInfo->estimated_لاگت = 1.0;
      pIdxInfo->aConstraintUsage[i].argv_اشاریہ = iPlan ? 2 : 1;
      pIdxInfo->aConstraintUsage[i].omit = 1;
      iPlan |= 1;
      توڑو;
    }
  }
  pIdxInfo->idx_عدد = iPlan;

  اگر( pIdxInfo->n_ترتیب_By>=1
   && pIdxInfo->a_ترتیب_By[0].i_عمود<=0
   && pIdxInfo->a_ترتیب_By[0].desc==0
  ){
    pIdxInfo->ترتیب_By_Consumed = 1;
  }
  واپس SQLITE_OK;
}

/*
** Open a new dbpagevfs cursor.
*/
ساکن صحیح dbpageOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  DbpageCursor *pCsr;

  pCsr = (DbpageCursor *)sqlite3_malloc64(حجم(DbpageCursor));
  اگر( pCsr==0 ){
    واپس SQLITE_NOMEM_BKPT;
  }ورنہ{
    memset(pCsr, 0, حجم(DbpageCursor));
    pCsr->بنیاد.pVtab = pVTab;
    pCsr->pgno = 0;
  }

  *ppCursor = (sqlite3_vtab_cursor *)pCsr;
  واپس SQLITE_OK;
}

/*
** Close a dbpagevfs cursor.
*/
ساکن صحیح dbpageClose(sqlite3_vtab_cursor *pCursor){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  اگر( pCsr->pPage1 ) sqlite3_Pager_Unref_صفحہ_One(pCsr->pPage1);
  sqlite3_free(pCsr);
  واپس SQLITE_OK;
}

/*
** Move a dbpagevfs cursor to the next entry in the file.
*/
ساکن صحیح dbpage_اگلا(sqlite3_vtab_cursor *pCursor){
  صحیح rc = SQLITE_OK;
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  pCsr->pgno++;
  واپس rc;
}

ساکن صحیح dbpageEof(sqlite3_vtab_cursor *pCursor){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  واپس pCsr->pgno > pCsr->mxPgno;
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
ساکن صحیح dbpageFilter(
  sqlite3_vtab_cursor *pCursor,
  صحیح idx_عدد, ثابت حرف *idxStr,
  صحیح argc, sqlite3_قدر **argv
){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  Dbpage_جدول *pTab = (Dbpage_جدول *)pCursor->pVtab;
  صحیح rc;
  sqlite3 *db = pTab->db;
  Btree *pBt;

  غیر_مستعمل_PARAMETER(idxStr);
  غیر_مستعمل_PARAMETER(argc);

  /* Default setting is no rows of result */
  pCsr->pgno = 1;
  pCsr->mxPgno = 0;

  اگر( idx_عدد & 2 ){
    ثابت حرف *zSchema;
    أكد( argc>=1 );
    zSchema = (ثابت حرف*)sqlite3_قدر_متن(argv[0]);
    pCsr->iDb = sqlite3_ڈھونڈو_Db_Name(db, zSchema);
    اگر( pCsr->iDb<0 ) واپس SQLITE_OK;
  }ورنہ{
    pCsr->iDb = 0;
  }
  pBt = db->aDb[pCsr->iDb].pBt;
  اگر( NEVER(pBt==0) ) واپس SQLITE_OK;
  pCsr->pPager = sqlite3BtreePager(pBt);
  pCsr->sz_صفحہ = sqlite3_Btree_Get_صفحہ_سائز(pBt);
  pCsr->mxPgno = sqlite3_Btree_آخری_صفحہ(pBt);
  اگر( idx_عدد & 1 ){
    i64 iPg = sqlite3_قدر_int64(argv[idx_عدد>>1]);
    أكد( argc>(idx_عدد>>1) );
    اگر( iPg<1 || iPg>pCsr->mxPgno ){
      pCsr->pgno = 1;
      pCsr->mxPgno = 0;
    }ورنہ{
      pCsr->pgno = (Pgno)iPg;
      pCsr->mxPgno = pCsr->pgno;
    }
  }ورنہ{
    أكد( pCsr->pgno==1 );
  }
  اگر( pCsr->pPage1 ) sqlite3_Pager_Unref_صفحہ_One(pCsr->pPage1);
  rc = sqlite3PagerGet(pCsr->pPager, 1, &pCsr->pPage1, 0);
  واپس rc;
}

ساکن صحیح dbpage_عمود(
  sqlite3_vtab_cursor *pCursor,
  sqlite3_context *ctx,
  صحیح i
){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  صحیح rc = SQLITE_OK;
  انتخاب( i ){
    حالت 0: {           /* pgno */
      sqlite3_نتیجہ_int64(ctx, (sqlite3_int64)pCsr->pgno);
      توڑو;
    }
    حالت 1: {           /*ڈیٹا*/
      Db_صفحہ *p_Db_صفحہ = 0;
      اگر( pCsr->pgno==(Pgno)((PENDING_BYTE/pCsr->sz_صفحہ)+1) ){
        /* The pending byte page. Assume it is zeroed out. Attempting to
        ** request this page from the page is an SQLITE_CORRUPT error. */
        sqlite3_نتیجہ_zeroblob(ctx, pCsr->sz_صفحہ);
      }ورنہ{
        rc = sqlite3PagerGet(pCsr->pPager, pCsr->pgno, (Db_صفحہ**)&p_Db_صفحہ, 0);
        اگر( rc==SQLITE_OK ){
          sqlite3_نتیجہ_blob(ctx, sqlite3_Pager_Get_ڈیٹا(p_Db_صفحہ), pCsr->sz_صفحہ,
              SQLITE_TRANSIENT);
        }
        sqlite3PagerUnref(p_Db_صفحہ);
      }
      توڑو;
    }
    طےشدہ: {          /* schema */
      sqlite3 *db = sqlite3_context_db_handle(ctx);
      sqlite3_نتیجہ_متن(ctx, db->aDb[pCsr->iDb].zDbSName, -1, SQLITE_STATIC);
      توڑو;
    }
  }
  واپس rc;
}

ساکن صحیح dbpageRowid(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  *pRowid = pCsr->pgno;
  واپس SQLITE_OK;
}

/* 
** Open write transactions. Since we do not know in advance which database
** files will be written by the sqlite_dbpage virtual table, start a write
** transaction on them all.
**
** Return SQLITE_OK if successful, or an SQLite error code otherwise.
*/
ساکن صحیح dbpageBeginTrans(Dbpage_جدول *pTab){
  sqlite3 *db = pTab->db;
  صحیح rc = SQLITE_OK;
  صحیح i;
  برائے(i=0; rc==SQLITE_OK && i<db->nDb; i++){
    Btree *pBt = db->aDb[i].pBt;
    اگر( pBt ) rc = sqlite3BtreeBeginTrans(pBt, 1, 0);
  }
  واپس rc;
}

ساکن صحیح dbpageUpdate(
  sqlite3_vtab *pVtab,
  صحیح argc,
  sqlite3_قدر **argv,
  sqlite_int64 *pRowid
){
  Dbpage_جدول *pTab = (Dbpage_جدول *)pVtab;
  Pgno pgno;
  sqlite3_int64 pgno64;
  Db_صفحہ *p_Db_صفحہ = 0;
  صحیح rc = SQLITE_OK;
  حرف *zErr = 0;
  صحیح iDb;
  Btree *pBt;
  Pager *pPager;
  صحیح sz_صفحہ;
  صحیح is_داخل_کرو;

  (خالی)pRowid;
  اگر( pTab->db->flags & SQLITE_Defensive ){
    zErr = "read-only";
    جاؤ update_fail;
  }
  اگر( argc==1 ){
    zErr = "cannot delete";
    جاؤ update_fail;
  }
  اگر( sqlite3_قدر_type(argv[0])==SQLITE_NULL ){
    pgno64 = sqlite3_قدر_int64(argv[2]);
    is_داخل_کرو = 1;
  }ورنہ{
    pgno64 = (Pgno)sqlite3_قدر_int64(argv[0]);
    اگر( sqlite3_قدر_int64(argv[1])!=pgno64 ){
      zErr = "cannot insert";
      جاؤ update_fail;
    }
    is_داخل_کرو = 0;
  }
  اگر( sqlite3_قدر_type(argv[4])==SQLITE_NULL ){
    iDb = 0;
  }ورنہ{
    ثابت حرف *zSchema = (ثابت حرف*)sqlite3_قدر_متن(argv[4]);
    iDb = sqlite3_ڈھونڈو_Db_Name(pTab->db, zSchema);
    اگر( iDb<0 ){
      zErr = "no such schema";
      جاؤ update_fail;
    }
  }
  pBt = pTab->db->aDb[iDb].pBt;
  اگر( pgno64<1 || pgno64>4294967294U || NEVER(pBt==0) ){
    zErr = "bad page number";
    جاؤ update_fail;
  }
  pgno = (Pgno)pgno64;
  sz_صفحہ = sqlite3_Btree_Get_صفحہ_سائز(pBt);
  اگر( sqlite3_قدر_type(argv[3])!=SQLITE_BLOB 
   || sqlite3_قدر_بائٹیں(argv[3])!=sz_صفحہ
  ){
    اگر( sqlite3_قدر_type(argv[3])==SQLITE_NULL && is_داخل_کرو && pgno>1 ){
      /* "INSERT INTO dbpage($PGNO,NULL)" causes page number $PGNO and
      ** all subsequent pages to be deleted. */
      pTab->iDbTrunc = iDb;
      pTab->pgnoTrunc = pgno-1;
      pgno = 1;
    }ورنہ{
      zErr = "bad page value";
      جاؤ update_fail;
    }
  }

  اگر( dbpageBeginTrans(pTab)!=SQLITE_OK ){
    zErr = "failed to open transaction";
    جاؤ update_fail;
  }

  pPager = sqlite3BtreePager(pBt);
  rc = sqlite3PagerGet(pPager, pgno, (Db_صفحہ**)&p_Db_صفحہ, 0);
  اگر( rc==SQLITE_OK ){
    ثابت خالی *p_ڈیٹا = sqlite3_قدر_blob(argv[3]);
    اگر( (rc = sqlite3PagerWrite(p_Db_صفحہ))==SQLITE_OK && p_ڈیٹا ){
      غیرعلامتی حرف *a_صفحہ = sqlite3_Pager_Get_ڈیٹا(p_Db_صفحہ);
      حافظہ_نقل(a_صفحہ, p_ڈیٹا, sz_صفحہ);
      pTab->pgnoTrunc = 0;
    }
  }
  اگر( rc!=SQLITE_OK ){
    pTab->pgnoTrunc = 0;
  }
  sqlite3PagerUnref(p_Db_صفحہ);
  واپس rc;

update_fail:
  pTab->pgnoTrunc = 0;
  sqlite3_free(pVtab->z_Err_پیغام);
  pVtab->z_Err_پیغام = sqlite3_mprintf("%s", zErr);
  واپس SQLITE_ERROR;
}

ساکن صحیح dbpageBegin(sqlite3_vtab *pVtab){
  Dbpage_جدول *pTab = (Dbpage_جدول *)pVtab;
  pTab->pgnoTrunc = 0;
  واپس SQLITE_OK;
}

/* Invoke sqlite3PagerTruncate() as necessary, just prior to COMMIT
*/
ساکن صحیح dbpageSync(sqlite3_vtab *pVtab){
  Dbpage_جدول *pTab = (Dbpage_جدول *)pVtab;
  اگر( pTab->pgnoTrunc>0 ){
    Btree *pBt = pTab->db->aDb[pTab->iDbTrunc].pBt;
    Pager *pPager = sqlite3BtreePager(pBt);
    sqlite3BtreeEnter(pBt);
    اگر( pTab->pgnoTrunc<sqlite3_Btree_آخری_صفحہ(pBt) ){
      sqlite3PagerTruncateImage(pPager, pTab->pgnoTrunc);
    }
    sqlite3BtreeLeave(pBt);
  }
  pTab->pgnoTrunc = 0;
  واپس SQLITE_OK;
}

/* Cancel any pending truncate.
*/
ساکن صحیح dbpage_Rollback_مقصد(sqlite3_vtab *pVtab, صحیح notUsed1){
  Dbpage_جدول *pTab = (Dbpage_جدول *)pVtab;
  pTab->pgnoTrunc = 0;
  (خالی)notUsed1;
  واپس SQLITE_OK;
}

/*
** Invoke this routine to register the "dbpage" virtual table module
*/
صحیح sqlite3_Dbpage_رجسٹر(sqlite3 *db){
  ساکن sqlite3_module dbpage_module = {
    2,                            /* iVersion */
    dbpageConnect,                /* xCreate */
    dbpageConnect,                /* xConnect */
    dbpage_بہترین_اشاریہ,              /* xBestIndex */
    dbpageDisconnect,             /* xDisconnect */
    dbpageDisconnect,             /* xDestroy */
    dbpageOpen,                   /* xOpen - open a cursor */
    dbpageClose,                  /* xClose - close a cursor */
    dbpageFilter,                 /* xFilter - configure scan constraints */
    dbpage_اگلا,                   /* xNext - advance a cursor */
    dbpageEof,                    /* xEof - check for end of scan */
    dbpage_عمود,                 /* xColumn - read data */
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
  واپس sqlite3_create_module(db, "sqlite_dbpage", &dbpage_module, 0);
}
#وگرنہ_اگر defined(SQLITE_ENABLE_DBPAGE_VTAB)
صحیح sqlite3_Dbpage_رجسٹر(sqlite3 *db){ واپس SQLITE_OK; }
#اگر_ختم /* SQLITE_ENABLE_DBSTAT_VTAB */
