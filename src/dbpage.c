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

تعریفنوع ساختار DbpageTable DbpageTable;
تعریفنوع ساختار DbpageCursor DbpageCursor;

ساختار DbpageCursor {
  sqlite3_vtab_cursor base;       /* Base class.  Must be first */
  Pgno pgno;                      /* Current page number */
  Pgno mxPgno;                    /* Last page to visit on this scan */
  Pager *pPager;                  /* Pager being read/written */
  DbPage *pPage1;                 /* Page 1 of the database */
  صحیح iDb;                        /* Index of database to analyze */
  صحیح szPage;                     /* Size of each page in bytes */
};

ساختار DbpageTable {
  sqlite3_vtab base;              /* Base class.  Must be first */
  sqlite3 *db;                    /* The database */
  صحیح iDbTrunc;                   /* Database to truncate */
  Pgno pgnoTrunc;                 /* Size to truncate to */
};

/* Columns */
#define DBPAGE_COLUMN_PGNO    0
#define DBPAGE_COLUMN_DATA    1
#define DBPAGE_COLUMN_SCHEMA  2


/*
** Connect to or create a dbpagevfs virtual table.
*/
ایستا صحیح dbpageConnect(
  sqlite3 *db,
  تهی *pAux,
  صحیح argc, ثابت نویسه *ثابت*argv,
  sqlite3_vtab **ppVtab,
  نویسه **pzErr
){
  DbpageTable *pTab = 0;
  صحیح rc = SQLITE_OK;
  (تهی)pAux;
  (تهی)argc;
  (تهی)argv;
  (تهی)pzErr;

  sqlite3_vtab_config(db, SQLITE_VTAB_DIRECTONLY);
  sqlite3_vtab_config(db, SQLITE_VTAB_USES_ALL_SCHEMAS);
  rc = sqlite3_declare_vtab(db, 
          "CREATE TABLE x(pgno INTEGER PRIMARY KEY, data BLOB, schema HIDDEN)");
  اگر( rc==SQLITE_OK ){
    pTab = (DbpageTable *)sqlite3_malloc64(اندازه(DbpageTable));
    اگر( pTab==0 ) rc = SQLITE_NOMEM_BKPT;
  }

  أكد( rc==SQLITE_OK || pTab==0 );
  اگر( rc==SQLITE_OK ){
    memset(pTab, 0, اندازه(DbpageTable));
    pTab->db = db;
  }

  *ppVtab = (sqlite3_vtab*)pTab;
  بازگشت rc;
}

/*
** Disconnect from or destroy a dbpagevfs virtual table.
*/
ایستا صحیح dbpageDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  بازگشت SQLITE_OK;
}

/*
** idxNum:
**
**     0     schema=main, full table scan
**     1     schema=main, pgno=?1
**     2     schema=?1, full table scan
**     3     schema=?1, pgno=?2
*/
ایستا صحیح dbpageBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  صحیح i;
  صحیح iPlan = 0;
  (تهی)tab;

  /* If there is a schema= constraint, it must be honored.  Report a
  ** ridiculously large estimated cost if the schema= constraint is
  ** unavailable
  */
  برای(i=0; i<pIdxInfo->nConstraint; i++){
    ساختار sqlite3_index_constraint *p = &pIdxInfo->aConstraint[i];
    اگر( p->iColumn!=DBPAGE_COLUMN_SCHEMA ) ادامه;
    اگر( p->op!=SQLITE_INDEX_CONSTRAINT_EQ ) ادامه;
    اگر( !p->usable ){
      /* No solution. */
      بازگشت SQLITE_CONSTRAINT;
    }
    iPlan = 2;
    pIdxInfo->aConstraintUsage[i].argvIndex = 1;
    pIdxInfo->aConstraintUsage[i].omit = 1;
    بشکن;
  }

  /* If we reach this point, it means that either there is no schema=
  ** constraint (in which case we use the "main" schema) or else the
  ** schema constraint was accepted.  Lower the estimated cost accordingly
  */
  pIdxInfo->estimatedCost = 1.0e6;

  /* Check for constraints against pgno */
  برای(i=0; i<pIdxInfo->nConstraint; i++){
    ساختار sqlite3_index_constraint *p = &pIdxInfo->aConstraint[i];
    اگر( p->usable && p->iColumn<=0 && p->op==SQLITE_INDEX_CONSTRAINT_EQ ){
      pIdxInfo->estimatedRows = 1;
      pIdxInfo->idxFlags = SQLITE_INDEX_SCAN_UNIQUE;
      pIdxInfo->estimatedCost = 1.0;
      pIdxInfo->aConstraintUsage[i].argvIndex = iPlan ? 2 : 1;
      pIdxInfo->aConstraintUsage[i].omit = 1;
      iPlan |= 1;
      بشکن;
    }
  }
  pIdxInfo->idx_رقم = iPlan;

  اگر( pIdxInfo->nOrderBy>=1
   && pIdxInfo->aOrderBy[0].iColumn<=0
   && pIdxInfo->aOrderBy[0].desc==0
  ){
    pIdxInfo->orderByConsumed = 1;
  }
  بازگشت SQLITE_OK;
}

/*
** Open a new dbpagevfs cursor.
*/
ایستا صحیح dbpageOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  DbpageCursor *pCsr;

  pCsr = (DbpageCursor *)sqlite3_malloc64(اندازه(DbpageCursor));
  اگر( pCsr==0 ){
    بازگشت SQLITE_NOMEM_BKPT;
  }وگرنه{
    memset(pCsr, 0, اندازه(DbpageCursor));
    pCsr->base.pVtab = pVTab;
    pCsr->pgno = 0;
  }

  *ppCursor = (sqlite3_vtab_cursor *)pCsr;
  بازگشت SQLITE_OK;
}

/*
** Close a dbpagevfs cursor.
*/
ایستا صحیح dbpageClose(sqlite3_vtab_cursor *pCursor){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  اگر( pCsr->pPage1 ) sqlite3PagerUnrefPageOne(pCsr->pPage1);
  sqlite3_free(pCsr);
  بازگشت SQLITE_OK;
}

/*
** Move a dbpagevfs cursor to the next entry in the file.
*/
ایستا صحیح dbpageNext(sqlite3_vtab_cursor *pCursor){
  صحیح rc = SQLITE_OK;
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  pCsr->pgno++;
  بازگشت rc;
}

ایستا صحیح dbpageEof(sqlite3_vtab_cursor *pCursor){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  بازگشت pCsr->pgno > pCsr->mxPgno;
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
ایستا صحیح dbpageFilter(
  sqlite3_vtab_cursor *pCursor,
  صحیح idx_رقم, ثابت نویسه *idxStr,
  صحیح argc, sqlite3_مقدار **argv
){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  DbpageTable *pTab = (DbpageTable *)pCursor->pVtab;
  صحیح rc;
  sqlite3 *db = pTab->db;
  Btree *pBt;

  UNUSED_PARAMETER(idxStr);
  UNUSED_PARAMETER(argc);

  /* Default setting is no rows of result */
  pCsr->pgno = 1;
  pCsr->mxPgno = 0;

  اگر( idx_رقم & 2 ){
    ثابت نویسه *zSchema;
    أكد( argc>=1 );
    zSchema = (ثابت نویسه*)sqlite3_مقدار_text(argv[0]);
    pCsr->iDb = sqlite3FindDbName(db, zSchema);
    اگر( pCsr->iDb<0 ) بازگشت SQLITE_OK;
  }وگرنه{
    pCsr->iDb = 0;
  }
  pBt = db->aDb[pCsr->iDb].pBt;
  اگر( NEVER(pBt==0) ) بازگشت SQLITE_OK;
  pCsr->pPager = sqlite3BtreePager(pBt);
  pCsr->szPage = sqlite3BtreeGetPageSize(pBt);
  pCsr->mxPgno = sqlite3BtreeLastPage(pBt);
  اگر( idx_رقم & 1 ){
    i64 iPg = sqlite3_مقدار_int64(argv[idx_رقم>>1]);
    أكد( argc>(idx_رقم>>1) );
    اگر( iPg<1 || iPg>pCsr->mxPgno ){
      pCsr->pgno = 1;
      pCsr->mxPgno = 0;
    }وگرنه{
      pCsr->pgno = (Pgno)iPg;
      pCsr->mxPgno = pCsr->pgno;
    }
  }وگرنه{
    أكد( pCsr->pgno==1 );
  }
  اگر( pCsr->pPage1 ) sqlite3PagerUnrefPageOne(pCsr->pPage1);
  rc = sqlite3PagerGet(pCsr->pPager, 1, &pCsr->pPage1, 0);
  بازگشت rc;
}

ایستا صحیح dbpageColumn(
  sqlite3_vtab_cursor *pCursor,
  sqlite3_context *ctx,
  صحیح i
){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  صحیح rc = SQLITE_OK;
  انتخاب( i ){
    حالت 0: {           /* pgno */
      sqlite3_result_int64(ctx, (sqlite3_int64)pCsr->pgno);
      بشکن;
    }
    حالت 1: {           /* data */
      DbPage *pDbPage = 0;
      اگر( pCsr->pgno==(Pgno)((PENDING_BYTE/pCsr->szPage)+1) ){
        /* The pending byte page. Assume it is zeroed out. Attempting to
        ** request this page from the page is an SQLITE_CORRUPT error. */
        sqlite3_result_zeroblob(ctx, pCsr->szPage);
      }وگرنه{
        rc = sqlite3PagerGet(pCsr->pPager, pCsr->pgno, (DbPage**)&pDbPage, 0);
        اگر( rc==SQLITE_OK ){
          sqlite3_result_blob(ctx, sqlite3PagerGetData(pDbPage), pCsr->szPage,
              SQLITE_TRANSIENT);
        }
        sqlite3PagerUnref(pDbPage);
      }
      بشکن;
    }
    پیشفرض: {          /* schema */
      sqlite3 *db = sqlite3_context_db_handle(ctx);
      sqlite3_result_text(ctx, db->aDb[pCsr->iDb].zDbSName, -1, SQLITE_STATIC);
      بشکن;
    }
  }
  بازگشت rc;
}

ایستا صحیح dbpageRowid(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  *pRowid = pCsr->pgno;
  بازگشت SQLITE_OK;
}

/* 
** Open write transactions. Since we do not know in advance which database
** files will be written by the sqlite_dbpage virtual table, start a write
** transaction on them all.
**
** Return SQLITE_OK if successful, or an SQLite error code otherwise.
*/
ایستا صحیح dbpageBeginTrans(DbpageTable *pTab){
  sqlite3 *db = pTab->db;
  صحیح rc = SQLITE_OK;
  صحیح i;
  برای(i=0; rc==SQLITE_OK && i<db->nDb; i++){
    Btree *pBt = db->aDb[i].pBt;
    اگر( pBt ) rc = sqlite3BtreeBeginTrans(pBt, 1, 0);
  }
  بازگشت rc;
}

ایستا صحیح dbpageUpdate(
  sqlite3_vtab *pVtab,
  صحیح argc,
  sqlite3_مقدار **argv,
  sqlite_int64 *pRowid
){
  DbpageTable *pTab = (DbpageTable *)pVtab;
  Pgno pgno;
  sqlite3_int64 pgno64;
  DbPage *pDbPage = 0;
  صحیح rc = SQLITE_OK;
  نویسه *zErr = 0;
  صحیح iDb;
  Btree *pBt;
  Pager *pPager;
  صحیح szPage;
  صحیح is_درج;

  (تهی)pRowid;
  اگر( pTab->db->flags & SQLITE_Defensive ){
    zErr = "read-only";
    برو update_fail;
  }
  اگر( argc==1 ){
    zErr = "cannot delete";
    برو update_fail;
  }
  اگر( sqlite3_مقدار_type(argv[0])==SQLITE_NULL ){
    pgno64 = sqlite3_مقدار_int64(argv[2]);
    is_درج = 1;
  }وگرنه{
    pgno64 = (Pgno)sqlite3_مقدار_int64(argv[0]);
    اگر( sqlite3_مقدار_int64(argv[1])!=pgno64 ){
      zErr = "cannot insert";
      برو update_fail;
    }
    is_درج = 0;
  }
  اگر( sqlite3_مقدار_type(argv[4])==SQLITE_NULL ){
    iDb = 0;
  }وگرنه{
    ثابت نویسه *zSchema = (ثابت نویسه*)sqlite3_مقدار_text(argv[4]);
    iDb = sqlite3FindDbName(pTab->db, zSchema);
    اگر( iDb<0 ){
      zErr = "no such schema";
      برو update_fail;
    }
  }
  pBt = pTab->db->aDb[iDb].pBt;
  اگر( pgno64<1 || pgno64>4294967294U || NEVER(pBt==0) ){
    zErr = "bad page number";
    برو update_fail;
  }
  pgno = (Pgno)pgno64;
  szPage = sqlite3BtreeGetPageSize(pBt);
  اگر( sqlite3_مقدار_type(argv[3])!=SQLITE_BLOB 
   || sqlite3_مقدار_bytes(argv[3])!=szPage
  ){
    اگر( sqlite3_مقدار_type(argv[3])==SQLITE_NULL && is_درج && pgno>1 ){
      /* "INSERT INTO dbpage($PGNO,NULL)" causes page number $PGNO and
      ** all subsequent pages to be deleted. */
      pTab->iDbTrunc = iDb;
      pTab->pgnoTrunc = pgno-1;
      pgno = 1;
    }وگرنه{
      zErr = "bad page value";
      برو update_fail;
    }
  }

  اگر( dbpageBeginTrans(pTab)!=SQLITE_OK ){
    zErr = "failed to open transaction";
    برو update_fail;
  }

  pPager = sqlite3BtreePager(pBt);
  rc = sqlite3PagerGet(pPager, pgno, (DbPage**)&pDbPage, 0);
  اگر( rc==SQLITE_OK ){
    ثابت تهی *pData = sqlite3_مقدار_blob(argv[3]);
    اگر( (rc = sqlite3PagerWrite(pDbPage))==SQLITE_OK && pData ){
      unsigned نویسه *aPage = sqlite3PagerGetData(pDbPage);
      memcpy(aPage, pData, szPage);
      pTab->pgnoTrunc = 0;
    }
  }
  اگر( rc!=SQLITE_OK ){
    pTab->pgnoTrunc = 0;
  }
  sqlite3PagerUnref(pDbPage);
  بازگشت rc;

update_fail:
  pTab->pgnoTrunc = 0;
  sqlite3_free(pVtab->z_Err_پیام);
  pVtab->z_Err_پیام = sqlite3_mprintf("%s", zErr);
  بازگشت SQLITE_ERROR;
}

ایستا صحیح dbpageBegin(sqlite3_vtab *pVtab){
  DbpageTable *pTab = (DbpageTable *)pVtab;
  pTab->pgnoTrunc = 0;
  بازگشت SQLITE_OK;
}

/* Invoke sqlite3PagerTruncate() as necessary, just prior to COMMIT
*/
ایستا صحیح dbpageSync(sqlite3_vtab *pVtab){
  DbpageTable *pTab = (DbpageTable *)pVtab;
  اگر( pTab->pgnoTrunc>0 ){
    Btree *pBt = pTab->db->aDb[pTab->iDbTrunc].pBt;
    Pager *pPager = sqlite3BtreePager(pBt);
    sqlite3BtreeEnter(pBt);
    اگر( pTab->pgnoTrunc<sqlite3BtreeLastPage(pBt) ){
      sqlite3PagerTruncateImage(pPager, pTab->pgnoTrunc);
    }
    sqlite3BtreeLeave(pBt);
  }
  pTab->pgnoTrunc = 0;
  بازگشت SQLITE_OK;
}

/* Cancel any pending truncate.
*/
ایستا صحیح dbpageRollbackTo(sqlite3_vtab *pVtab, صحیح notUsed1){
  DbpageTable *pTab = (DbpageTable *)pVtab;
  pTab->pgnoTrunc = 0;
  (تهی)notUsed1;
  بازگشت SQLITE_OK;
}

/*
** Invoke this routine to register the "dbpage" virtual table module
*/
صحیح sqlite3DbpageRegister(sqlite3 *db){
  ایستا sqlite3_module dbpage_module = {
    2,                            /* iVersion */
    dbpageConnect,                /* xCreate */
    dbpageConnect,                /* xConnect */
    dbpageBestIndex,              /* xBestIndex */
    dbpageDisconnect,             /* xDisconnect */
    dbpageDisconnect,             /* xDestroy */
    dbpageOpen,                   /* xOpen - open a cursor */
    dbpageClose,                  /* xClose - close a cursor */
    dbpageFilter,                 /* xFilter - configure scan constraints */
    dbpageNext,                   /* xNext - advance a cursor */
    dbpageEof,                    /* xEof - check for end of scan */
    dbpageColumn,                 /* xColumn - read data */
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
    dbpageRollbackTo,             /* xRollbackTo */
    0,                            /* xShadowName */
    0                             /* xIntegrity */
  };
  بازگشت sqlite3_create_module(db, "sqlite_dbpage", &dbpage_module, 0);
}
#وگرنهاگر defined(SQLITE_ENABLE_DBPAGE_VTAB)
صحیح sqlite3DbpageRegister(sqlite3 *db){ بازگشت SQLITE_OK; }
#endif /* SQLITE_ENABLE_DBSTAT_VTAB */
