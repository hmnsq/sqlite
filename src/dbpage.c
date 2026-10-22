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

#include "sqliteInt.h"   /* Requires access to internal data structures */
#အကယ်၍ (defined(SQLITE_ENABLE_DBPAGE_VTAB) || defined(SQLITE_TEST)) \
    && !defined(SQLITE_OMIT_VIRTUALTABLE)

typedef sa_nracanaa DbpageTable DbpageTable;
typedef sa_nracanaa DbpageCursor DbpageCursor;

sa_nracanaa DbpageCursor {
  sqlite3_vtab_cursor base;       /* Base class.  Must be first */
  Pgno pgno;                      /* Current page number */
  Pgno mxPgno;                    /* Last page to visit on this scan */
  Pager *pPager;                  /* Pager being read/written */
  DbPage *pPage1;                 /* Page 1 of the database */
  ကိန်းပြည့် iDb;                        /* Index of database to analyze */
  ကိန်းပြည့် szPage;                     /* Size of each page in bytes */
};

sa_nracanaa DbpageTable {
  sqlite3_vtab base;              /* Base class.  Must be first */
  sqlite3 *db;                    /* The database */
  ကိန်းပြည့် iDbTrunc;                   /* Database to truncate */
  Pgno pgnoTrunc;                 /* Size to truncate to */
};

/* Columns */
#define DBPAGE_COLUMN_PGNO    0
#define DBPAGE_COLUMN_DATA    1
#define DBPAGE_COLUMN_SCHEMA  2


/*
** Connect to or create a dbpagevfs virtual table.
*/
static ကိန်းပြည့် dbpageConnect(
  sqlite3 *db,
  void *pAux,
  ကိန်းပြည့် argc, const var_na *const*argv,
  sqlite3_vtab **ppVtab,
  var_na **pzErr
){
  DbpageTable *pTab = 0;
  ကိန်းပြည့် rc = SQLITE_OK;
  (void)pAux;
  (void)argc;
  (void)argv;
  (void)pzErr;

  sqlite3_vtab_config(db, SQLITE_VTAB_DIRECTONLY);
  sqlite3_vtab_config(db, SQLITE_VTAB_USES_ALL_SCHEMAS);
  rc = sqlite3_declare_vtab(db, 
          "CREATE TABLE x(pgno INTEGER PRIMARY KEY, data BLOB, schema HIDDEN)");
  အကယ်၍( rc==SQLITE_OK ){
    pTab = (DbpageTable *)sqlite3_malloc64(parimaa_na(DbpageTable));
    အကယ်၍( pTab==0 ) rc = SQLITE_NOMEM_BKPT;
  }

  assert( rc==SQLITE_OK || pTab==0 );
  အကယ်၍( rc==SQLITE_OK ){
    memset(pTab, 0, parimaa_na(DbpageTable));
    pTab->db = db;
  }

  *ppVtab = (sqlite3_vtab*)pTab;
  ပြန်ပေး rc;
}

/*
** Disconnect from or destroy a dbpagevfs virtual table.
*/
static ကိန်းပြည့် dbpageDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  ပြန်ပေး SQLITE_OK;
}

/*
** idxNum:
**
**     0     schema=main, full table scan
**     1     schema=main, pgno=?1
**     2     schema=?1, full table scan
**     3     schema=?1, pgno=?2
*/
static ကိန်းပြည့် dbpageBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  ကိန်းပြည့် i;
  ကိန်းပြည့် iPlan = 0;
  (void)tab;

  /* If there is a schema= constraint, it must be honored.  Report a
  ** ridiculously large estimated cost if the schema= constraint is
  ** unavailable
  */
  အတွက်(i=0; i<pIdxInfo->nConstraint; i++){
    sa_nracanaa sqlite3_index_constraint *p = &pIdxInfo->aConstraint[i];
    အကယ်၍( p->iColumn!=DBPAGE_COLUMN_SCHEMA ) continue;
    အကယ်၍( p->op!=SQLITE_INDEX_CONSTRAINT_EQ ) continue;
    အကယ်၍( !p->usable ){
      /* No solution. */
      ပြန်ပေး SQLITE_CONSTRAINT;
    }
    iPlan = 2;
    pIdxInfo->aConstraintUsage[i].argvIndex = 1;
    pIdxInfo->aConstraintUsage[i].omit = 1;
    bha_nga;
  }

  /* If we reach this point, it means that either there is no schema=
  ** constraint (in which case we use the "main" schema) or else the
  ** schema constraint was accepted.  Lower the estimated cost accordingly
  */
  pIdxInfo->estimatedCost = 1.0e6;

  /* Check for constraints against pgno */
  အတွက်(i=0; i<pIdxInfo->nConstraint; i++){
    sa_nracanaa sqlite3_index_constraint *p = &pIdxInfo->aConstraint[i];
    အကယ်၍( p->usable && p->iColumn<=0 && p->op==SQLITE_INDEX_CONSTRAINT_EQ ){
      pIdxInfo->estimatedRows = 1;
      pIdxInfo->idxFlags = SQLITE_INDEX_SCAN_UNIQUE;
      pIdxInfo->estimatedCost = 1.0;
      pIdxInfo->aConstraintUsage[i].argvIndex = iPlan ? 2 : 1;
      pIdxInfo->aConstraintUsage[i].omit = 1;
      iPlan |= 1;
      bha_nga;
    }
  }
  pIdxInfo->idxNum = iPlan;

  အကယ်၍( pIdxInfo->nOrderBy>=1
   && pIdxInfo->aOrderBy[0].iColumn<=0
   && pIdxInfo->aOrderBy[0].desc==0
  ){
    pIdxInfo->orderByConsumed = 1;
  }
  ပြန်ပေး SQLITE_OK;
}

/*
** Open a new dbpagevfs cursor.
*/
static ကိန်းပြည့် dbpageOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  DbpageCursor *pCsr;

  pCsr = (DbpageCursor *)sqlite3_malloc64(parimaa_na(DbpageCursor));
  အကယ်၍( pCsr==0 ){
    ပြန်ပေး SQLITE_NOMEM_BKPT;
  }မဟုတ်ပါက{
    memset(pCsr, 0, parimaa_na(DbpageCursor));
    pCsr->base.pVtab = pVTab;
    pCsr->pgno = 0;
  }

  *ppCursor = (sqlite3_vtab_cursor *)pCsr;
  ပြန်ပေး SQLITE_OK;
}

/*
** Close a dbpagevfs cursor.
*/
static ကိန်းပြည့် dbpageClose(sqlite3_vtab_cursor *pCursor){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  အကယ်၍( pCsr->pPage1 ) sqlite3PagerUnrefPageOne(pCsr->pPage1);
  sqlite3_free(pCsr);
  ပြန်ပေး SQLITE_OK;
}

/*
** Move a dbpagevfs cursor to the next entry in the file.
*/
static ကိန်းပြည့် dbpageNext(sqlite3_vtab_cursor *pCursor){
  ကိန်းပြည့် rc = SQLITE_OK;
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  pCsr->pgno++;
  ပြန်ပေး rc;
}

static ကိန်းပြည့် dbpageEof(sqlite3_vtab_cursor *pCursor){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  ပြန်ပေး pCsr->pgno > pCsr->mxPgno;
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
static ကိန်းပြည့် dbpageFilter(
  sqlite3_vtab_cursor *pCursor,
  ကိန်းပြည့် idxNum, const var_na *idxStr,
  ကိန်းပြည့် argc, sqlite3_value **argv
){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  DbpageTable *pTab = (DbpageTable *)pCursor->pVtab;
  ကိန်းပြည့် rc;
  sqlite3 *db = pTab->db;
  Btree *pBt;

  UNUSED_PARAMETER(idxStr);
  UNUSED_PARAMETER(argc);

  /* Default setting is no rows of result */
  pCsr->pgno = 1;
  pCsr->mxPgno = 0;

  အကယ်၍( idxNum & 2 ){
    const var_na *zSchema;
    assert( argc>=1 );
    zSchema = (const var_na*)sqlite3_value_text(argv[0]);
    pCsr->iDb = sqlite3FindDbName(db, zSchema);
    အကယ်၍( pCsr->iDb<0 ) ပြန်ပေး SQLITE_OK;
  }မဟုတ်ပါက{
    pCsr->iDb = 0;
  }
  pBt = db->aDb[pCsr->iDb].pBt;
  အကယ်၍( NEVER(pBt==0) ) ပြန်ပေး SQLITE_OK;
  pCsr->pPager = sqlite3BtreePager(pBt);
  pCsr->szPage = sqlite3BtreeGetPageSize(pBt);
  pCsr->mxPgno = sqlite3BtreeLastPage(pBt);
  အကယ်၍( idxNum & 1 ){
    i64 iPg = sqlite3_value_int64(argv[idxNum>>1]);
    assert( argc>(idxNum>>1) );
    အကယ်၍( iPg<1 || iPg>pCsr->mxPgno ){
      pCsr->pgno = 1;
      pCsr->mxPgno = 0;
    }မဟုတ်ပါက{
      pCsr->pgno = (Pgno)iPg;
      pCsr->mxPgno = pCsr->pgno;
    }
  }မဟုတ်ပါက{
    assert( pCsr->pgno==1 );
  }
  အကယ်၍( pCsr->pPage1 ) sqlite3PagerUnrefPageOne(pCsr->pPage1);
  rc = sqlite3PagerGet(pCsr->pPager, 1, &pCsr->pPage1, 0);
  ပြန်ပေး rc;
}

static ကိန်းပြည့် dbpageColumn(
  sqlite3_vtab_cursor *pCursor,
  sqlite3_context *ctx,
  ကိန်းပြည့် i
){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  ကိန်းပြည့် rc = SQLITE_OK;
  switch( i ){
    pak_sa 0: {           /* pgno */
      sqlite3_result_int64(ctx, (sqlite3_int64)pCsr->pgno);
      bha_nga;
    }
    pak_sa 1: {           /* data */
      DbPage *pDbPage = 0;
      အကယ်၍( pCsr->pgno==(Pgno)((PENDING_BYTE/pCsr->szPage)+1) ){
        /* The pending byte page. Assume it is zeroed out. Attempting to
        ** request this page from the page is an SQLITE_CORRUPT error. */
        sqlite3_result_zeroblob(ctx, pCsr->szPage);
      }မဟုတ်ပါက{
        rc = sqlite3PagerGet(pCsr->pPager, pCsr->pgno, (DbPage**)&pDbPage, 0);
        အကယ်၍( rc==SQLITE_OK ){
          sqlite3_result_blob(ctx, sqlite3PagerGetData(pDbPage), pCsr->szPage,
              SQLITE_TRANSIENT);
        }
        sqlite3PagerUnref(pDbPage);
      }
      bha_nga;
    }
    default: {          /* schema */
      sqlite3 *db = sqlite3_context_db_handle(ctx);
      sqlite3_result_text(ctx, db->aDb[pCsr->iDb].zDbSName, -1, SQLITE_STATIC);
      bha_nga;
    }
  }
  ပြန်ပေး rc;
}

static ကိန်းပြည့် dbpageRowid(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  *pRowid = pCsr->pgno;
  ပြန်ပေး SQLITE_OK;
}

/* 
** Open write transactions. Since we do not know in advance which database
** files will be written by the sqlite_dbpage virtual table, start a write
** transaction on them all.
**
** Return SQLITE_OK if successful, or an SQLite error code otherwise.
*/
static ကိန်းပြည့် dbpageBeginTrans(DbpageTable *pTab){
  sqlite3 *db = pTab->db;
  ကိန်းပြည့် rc = SQLITE_OK;
  ကိန်းပြည့် i;
  အတွက်(i=0; rc==SQLITE_OK && i<db->nDb; i++){
    Btree *pBt = db->aDb[i].pBt;
    အကယ်၍( pBt ) rc = sqlite3BtreeBeginTrans(pBt, 1, 0);
  }
  ပြန်ပေး rc;
}

static ကိန်းပြည့် dbpageUpdate(
  sqlite3_vtab *pVtab,
  ကိန်းပြည့် argc,
  sqlite3_value **argv,
  sqlite_int64 *pRowid
){
  DbpageTable *pTab = (DbpageTable *)pVtab;
  Pgno pgno;
  sqlite3_int64 pgno64;
  DbPage *pDbPage = 0;
  ကိန်းပြည့် rc = SQLITE_OK;
  var_na *zErr = 0;
  ကိန်းပြည့် iDb;
  Btree *pBt;
  Pager *pPager;
  ကိန်းပြည့် szPage;
  ကိန်းပြည့် isInsert;

  (void)pRowid;
  အကယ်၍( pTab->db->flags & SQLITE_Defensive ){
    zErr = "read-only";
    goto update_fail;
  }
  အကယ်၍( argc==1 ){
    zErr = "cannot delete";
    goto update_fail;
  }
  အကယ်၍( sqlite3_value_type(argv[0])==SQLITE_NULL ){
    pgno64 = sqlite3_value_int64(argv[2]);
    isInsert = 1;
  }မဟုတ်ပါက{
    pgno64 = (Pgno)sqlite3_value_int64(argv[0]);
    အကယ်၍( sqlite3_value_int64(argv[1])!=pgno64 ){
      zErr = "cannot insert";
      goto update_fail;
    }
    isInsert = 0;
  }
  အကယ်၍( sqlite3_value_type(argv[4])==SQLITE_NULL ){
    iDb = 0;
  }မဟုတ်ပါက{
    const var_na *zSchema = (const var_na*)sqlite3_value_text(argv[4]);
    iDb = sqlite3FindDbName(pTab->db, zSchema);
    အကယ်၍( iDb<0 ){
      zErr = "no such schema";
      goto update_fail;
    }
  }
  pBt = pTab->db->aDb[iDb].pBt;
  အကယ်၍( pgno64<1 || pgno64>4294967294U || NEVER(pBt==0) ){
    zErr = "bad page number";
    goto update_fail;
  }
  pgno = (Pgno)pgno64;
  szPage = sqlite3BtreeGetPageSize(pBt);
  အကယ်၍( sqlite3_value_type(argv[3])!=SQLITE_BLOB 
   || sqlite3_value_bytes(argv[3])!=szPage
  ){
    အကယ်၍( sqlite3_value_type(argv[3])==SQLITE_NULL && isInsert && pgno>1 ){
      /* "INSERT INTO dbpage($PGNO,NULL)" causes page number $PGNO and
      ** all subsequent pages to be deleted. */
      pTab->iDbTrunc = iDb;
      pTab->pgnoTrunc = pgno-1;
      pgno = 1;
    }မဟုတ်ပါက{
      zErr = "bad page value";
      goto update_fail;
    }
  }

  အကယ်၍( dbpageBeginTrans(pTab)!=SQLITE_OK ){
    zErr = "failed to open transaction";
    goto update_fail;
  }

  pPager = sqlite3BtreePager(pBt);
  rc = sqlite3PagerGet(pPager, pgno, (DbPage**)&pDbPage, 0);
  အကယ်၍( rc==SQLITE_OK ){
    const void *pData = sqlite3_value_blob(argv[3]);
    အကယ်၍( (rc = sqlite3PagerWrite(pDbPage))==SQLITE_OK && pData ){
      unsigned var_na *aPage = sqlite3PagerGetData(pDbPage);
      memcpy(aPage, pData, szPage);
      pTab->pgnoTrunc = 0;
    }
  }
  အကယ်၍( rc!=SQLITE_OK ){
    pTab->pgnoTrunc = 0;
  }
  sqlite3PagerUnref(pDbPage);
  ပြန်ပေး rc;

update_fail:
  pTab->pgnoTrunc = 0;
  sqlite3_free(pVtab->zErrMsg);
  pVtab->zErrMsg = sqlite3_mprintf("%s", zErr);
  ပြန်ပေး SQLITE_ERROR;
}

static ကိန်းပြည့် dbpageBegin(sqlite3_vtab *pVtab){
  DbpageTable *pTab = (DbpageTable *)pVtab;
  pTab->pgnoTrunc = 0;
  ပြန်ပေး SQLITE_OK;
}

/* Invoke sqlite3PagerTruncate() as necessary, just prior to COMMIT
*/
static ကိန်းပြည့် dbpageSync(sqlite3_vtab *pVtab){
  DbpageTable *pTab = (DbpageTable *)pVtab;
  အကယ်၍( pTab->pgnoTrunc>0 ){
    Btree *pBt = pTab->db->aDb[pTab->iDbTrunc].pBt;
    Pager *pPager = sqlite3BtreePager(pBt);
    sqlite3BtreeEnter(pBt);
    အကယ်၍( pTab->pgnoTrunc<sqlite3BtreeLastPage(pBt) ){
      sqlite3PagerTruncateImage(pPager, pTab->pgnoTrunc);
    }
    sqlite3BtreeLeave(pBt);
  }
  pTab->pgnoTrunc = 0;
  ပြန်ပေး SQLITE_OK;
}

/* Cancel any pending truncate.
*/
static ကိန်းပြည့် dbpageRollbackTo(sqlite3_vtab *pVtab, ကိန်းပြည့် notUsed1){
  DbpageTable *pTab = (DbpageTable *)pVtab;
  pTab->pgnoTrunc = 0;
  (void)notUsed1;
  ပြန်ပေး SQLITE_OK;
}

/*
** Invoke this routine to register the "dbpage" virtual table module
*/
ကိန်းပြည့် sqlite3DbpageRegister(sqlite3 *db){
  static sqlite3_module dbpage_module = {
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
  ပြန်ပေး sqlite3_create_module(db, "sqlite_dbpage", &dbpage_module, 0);
}
#elif defined(SQLITE_ENABLE_DBPAGE_VTAB)
ကိန်းပြည့် sqlite3DbpageRegister(sqlite3 *db){ ပြန်ပေး SQLITE_OK; }
#endif /* SQLITE_ENABLE_DBSTAT_VTAB */
