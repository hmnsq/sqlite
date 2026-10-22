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
#ပေႃး (defined(SQLITE_ENABLE_DBPAGE_VTAB) || defined(SQLITE_TEST)) \
    && !defined(SQLITE_OMIT_VIRTUALTABLE)

typedef sa_nracanaa DbpageTable DbpageTable;
typedef sa_nracanaa DbpageCursor DbpageCursor;

sa_nracanaa DbpageCursor {
  sqlite3_vtab_cursor base;       /* Base class.  Must be first */
  Pgno pgno;                      /* Current page number */
  Pgno mxPgno;                    /* Last page to visit on this scan */
  Pager *pPager;                  /* Pager being read/written */
  DbPage *pPage1;                 /* Page 1 of the database */
  puur_naa_nka iDb;                        /* Index of database to analyze */
  puur_naa_nka szPage;                     /* Size of each page in bytes */
};

sa_nracanaa DbpageTable {
  sqlite3_vtab base;              /* Base class.  Must be first */
  sqlite3 *db;                    /* The database */
  puur_naa_nka iDbTrunc;                   /* Database to truncate */
  Pgno pgnoTrunc;                 /* Size to truncate to */
};

/* Columns */
#define DBPAGE_COLUMN_PGNO    0
#define DBPAGE_COLUMN_DATA    1
#define DBPAGE_COLUMN_SCHEMA  2


/*
** Connect to or create a dbpagevfs virtual table.
*/
static puur_naa_nka dbpageConnect(
  sqlite3 *db,
  void *pAux,
  puur_naa_nka argc, const var_na *const*argv,
  sqlite3_vtab **ppVtab,
  var_na **pzErr
){
  DbpageTable *pTab = 0;
  puur_naa_nka rc = SQLITE_OK;
  (void)pAux;
  (void)argc;
  (void)argv;
  (void)pzErr;

  sqlite3_vtab_config(db, SQLITE_VTAB_DIRECTONLY);
  sqlite3_vtab_config(db, SQLITE_VTAB_USES_ALL_SCHEMAS);
  rc = sqlite3_declare_vtab(db, 
          "CREATE TABLE x(pgno INTEGER PRIMARY KEY, data BLOB, schema HIDDEN)");
  ပေႃး( rc==SQLITE_OK ){
    pTab = (DbpageTable *)sqlite3_malloc64(parimaa_na(DbpageTable));
    ပေႃး( pTab==0 ) rc = SQLITE_NOMEM_BKPT;
  }

  assert( rc==SQLITE_OK || pTab==0 );
  ပေႃး( rc==SQLITE_OK ){
    memset(pTab, 0, parimaa_na(DbpageTable));
    pTab->db = db;
  }

  *ppVtab = (sqlite3_vtab*)pTab;
  ပွၵ်ႈ rc;
}

/*
** Disconnect from or destroy a dbpagevfs virtual table.
*/
static puur_naa_nka dbpageDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  ပွၵ်ႈ SQLITE_OK;
}

/*
** idxNum:
**
**     0     schema=main, full table scan
**     1     schema=main, pgno=?1
**     2     schema=?1, full table scan
**     3     schema=?1, pgno=?2
*/
static puur_naa_nka dbpageBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  puur_naa_nka i;
  puur_naa_nka iPlan = 0;
  (void)tab;

  /* If there is a schema= constraint, it must be honored.  Report a
  ** ridiculously large estimated cost if the schema= constraint is
  ** unavailable
  */
  for(i=0; i<pIdxInfo->nConstraint; i++){
    sa_nracanaa sqlite3_index_constraint *p = &pIdxInfo->aConstraint[i];
    ပေႃး( p->iColumn!=DBPAGE_COLUMN_SCHEMA ) continue;
    ပေႃး( p->op!=SQLITE_INDEX_CONSTRAINT_EQ ) continue;
    ပေႃး( !p->usable ){
      /* No solution. */
      ပွၵ်ႈ SQLITE_CONSTRAINT;
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
  for(i=0; i<pIdxInfo->nConstraint; i++){
    sa_nracanaa sqlite3_index_constraint *p = &pIdxInfo->aConstraint[i];
    ပေႃး( p->usable && p->iColumn<=0 && p->op==SQLITE_INDEX_CONSTRAINT_EQ ){
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

  ပေႃး( pIdxInfo->nOrderBy>=1
   && pIdxInfo->aOrderBy[0].iColumn<=0
   && pIdxInfo->aOrderBy[0].desc==0
  ){
    pIdxInfo->orderByConsumed = 1;
  }
  ပွၵ်ႈ SQLITE_OK;
}

/*
** Open a new dbpagevfs cursor.
*/
static puur_naa_nka dbpageOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  DbpageCursor *pCsr;

  pCsr = (DbpageCursor *)sqlite3_malloc64(parimaa_na(DbpageCursor));
  ပေႃး( pCsr==0 ){
    ပွၵ်ႈ SQLITE_NOMEM_BKPT;
  }ပေႃးဢမ်ႇ{
    memset(pCsr, 0, parimaa_na(DbpageCursor));
    pCsr->base.pVtab = pVTab;
    pCsr->pgno = 0;
  }

  *ppCursor = (sqlite3_vtab_cursor *)pCsr;
  ပွၵ်ႈ SQLITE_OK;
}

/*
** Close a dbpagevfs cursor.
*/
static puur_naa_nka dbpageClose(sqlite3_vtab_cursor *pCursor){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  ပေႃး( pCsr->pPage1 ) sqlite3PagerUnrefPageOne(pCsr->pPage1);
  sqlite3_free(pCsr);
  ပွၵ်ႈ SQLITE_OK;
}

/*
** Move a dbpagevfs cursor to the next entry in the file.
*/
static puur_naa_nka dbpageNext(sqlite3_vtab_cursor *pCursor){
  puur_naa_nka rc = SQLITE_OK;
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  pCsr->pgno++;
  ပွၵ်ႈ rc;
}

static puur_naa_nka dbpageEof(sqlite3_vtab_cursor *pCursor){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  ပွၵ်ႈ pCsr->pgno > pCsr->mxPgno;
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
static puur_naa_nka dbpageFilter(
  sqlite3_vtab_cursor *pCursor,
  puur_naa_nka idxNum, const var_na *idxStr,
  puur_naa_nka argc, sqlite3_value **argv
){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  DbpageTable *pTab = (DbpageTable *)pCursor->pVtab;
  puur_naa_nka rc;
  sqlite3 *db = pTab->db;
  Btree *pBt;

  UNUSED_PARAMETER(idxStr);
  UNUSED_PARAMETER(argc);

  /* Default setting is no rows of result */
  pCsr->pgno = 1;
  pCsr->mxPgno = 0;

  ပေႃး( idxNum & 2 ){
    const var_na *zSchema;
    assert( argc>=1 );
    zSchema = (const var_na*)sqlite3_value_text(argv[0]);
    pCsr->iDb = sqlite3FindDbName(db, zSchema);
    ပေႃး( pCsr->iDb<0 ) ပွၵ်ႈ SQLITE_OK;
  }ပေႃးဢမ်ႇ{
    pCsr->iDb = 0;
  }
  pBt = db->aDb[pCsr->iDb].pBt;
  ပေႃး( NEVER(pBt==0) ) ပွၵ်ႈ SQLITE_OK;
  pCsr->pPager = sqlite3BtreePager(pBt);
  pCsr->szPage = sqlite3BtreeGetPageSize(pBt);
  pCsr->mxPgno = sqlite3BtreeLastPage(pBt);
  ပေႃး( idxNum & 1 ){
    i64 iPg = sqlite3_value_int64(argv[idxNum>>1]);
    assert( argc>(idxNum>>1) );
    ပေႃး( iPg<1 || iPg>pCsr->mxPgno ){
      pCsr->pgno = 1;
      pCsr->mxPgno = 0;
    }ပေႃးဢမ်ႇ{
      pCsr->pgno = (Pgno)iPg;
      pCsr->mxPgno = pCsr->pgno;
    }
  }ပေႃးဢမ်ႇ{
    assert( pCsr->pgno==1 );
  }
  ပေႃး( pCsr->pPage1 ) sqlite3PagerUnrefPageOne(pCsr->pPage1);
  rc = sqlite3PagerGet(pCsr->pPager, 1, &pCsr->pPage1, 0);
  ပွၵ်ႈ rc;
}

static puur_naa_nka dbpageColumn(
  sqlite3_vtab_cursor *pCursor,
  sqlite3_context *ctx,
  puur_naa_nka i
){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  puur_naa_nka rc = SQLITE_OK;
  switch( i ){
    pak_sa 0: {           /* pgno */
      sqlite3_result_int64(ctx, (sqlite3_int64)pCsr->pgno);
      bha_nga;
    }
    pak_sa 1: {           /* data */
      DbPage *pDbPage = 0;
      ပေႃး( pCsr->pgno==(Pgno)((PENDING_BYTE/pCsr->szPage)+1) ){
        /* The pending byte page. Assume it is zeroed out. Attempting to
        ** request this page from the page is an SQLITE_CORRUPT error. */
        sqlite3_result_zeroblob(ctx, pCsr->szPage);
      }ပေႃးဢမ်ႇ{
        rc = sqlite3PagerGet(pCsr->pPager, pCsr->pgno, (DbPage**)&pDbPage, 0);
        ပေႃး( rc==SQLITE_OK ){
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
  ပွၵ်ႈ rc;
}

static puur_naa_nka dbpageRowid(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  *pRowid = pCsr->pgno;
  ပွၵ်ႈ SQLITE_OK;
}

/* 
** Open write transactions. Since we do not know in advance which database
** files will be written by the sqlite_dbpage virtual table, start a write
** transaction on them all.
**
** Return SQLITE_OK if successful, or an SQLite error code otherwise.
*/
static puur_naa_nka dbpageBeginTrans(DbpageTable *pTab){
  sqlite3 *db = pTab->db;
  puur_naa_nka rc = SQLITE_OK;
  puur_naa_nka i;
  for(i=0; rc==SQLITE_OK && i<db->nDb; i++){
    Btree *pBt = db->aDb[i].pBt;
    ပေႃး( pBt ) rc = sqlite3BtreeBeginTrans(pBt, 1, 0);
  }
  ပွၵ်ႈ rc;
}

static puur_naa_nka dbpageUpdate(
  sqlite3_vtab *pVtab,
  puur_naa_nka argc,
  sqlite3_value **argv,
  sqlite_int64 *pRowid
){
  DbpageTable *pTab = (DbpageTable *)pVtab;
  Pgno pgno;
  sqlite3_int64 pgno64;
  DbPage *pDbPage = 0;
  puur_naa_nka rc = SQLITE_OK;
  var_na *zErr = 0;
  puur_naa_nka iDb;
  Btree *pBt;
  Pager *pPager;
  puur_naa_nka szPage;
  puur_naa_nka isInsert;

  (void)pRowid;
  ပေႃး( pTab->db->flags & SQLITE_Defensive ){
    zErr = "read-only";
    goto update_fail;
  }
  ပေႃး( argc==1 ){
    zErr = "cannot delete";
    goto update_fail;
  }
  ပေႃး( sqlite3_value_type(argv[0])==SQLITE_NULL ){
    pgno64 = sqlite3_value_int64(argv[2]);
    isInsert = 1;
  }ပေႃးဢမ်ႇ{
    pgno64 = (Pgno)sqlite3_value_int64(argv[0]);
    ပေႃး( sqlite3_value_int64(argv[1])!=pgno64 ){
      zErr = "cannot insert";
      goto update_fail;
    }
    isInsert = 0;
  }
  ပေႃး( sqlite3_value_type(argv[4])==SQLITE_NULL ){
    iDb = 0;
  }ပေႃးဢမ်ႇ{
    const var_na *zSchema = (const var_na*)sqlite3_value_text(argv[4]);
    iDb = sqlite3FindDbName(pTab->db, zSchema);
    ပေႃး( iDb<0 ){
      zErr = "no such schema";
      goto update_fail;
    }
  }
  pBt = pTab->db->aDb[iDb].pBt;
  ပေႃး( pgno64<1 || pgno64>4294967294U || NEVER(pBt==0) ){
    zErr = "bad page number";
    goto update_fail;
  }
  pgno = (Pgno)pgno64;
  szPage = sqlite3BtreeGetPageSize(pBt);
  ပေႃး( sqlite3_value_type(argv[3])!=SQLITE_BLOB 
   || sqlite3_value_bytes(argv[3])!=szPage
  ){
    ပေႃး( sqlite3_value_type(argv[3])==SQLITE_NULL && isInsert && pgno>1 ){
      /* "INSERT INTO dbpage($PGNO,NULL)" causes page number $PGNO and
      ** all subsequent pages to be deleted. */
      pTab->iDbTrunc = iDb;
      pTab->pgnoTrunc = pgno-1;
      pgno = 1;
    }ပေႃးဢမ်ႇ{
      zErr = "bad page value";
      goto update_fail;
    }
  }

  ပေႃး( dbpageBeginTrans(pTab)!=SQLITE_OK ){
    zErr = "failed to open transaction";
    goto update_fail;
  }

  pPager = sqlite3BtreePager(pBt);
  rc = sqlite3PagerGet(pPager, pgno, (DbPage**)&pDbPage, 0);
  ပေႃး( rc==SQLITE_OK ){
    const void *pData = sqlite3_value_blob(argv[3]);
    ပေႃး( (rc = sqlite3PagerWrite(pDbPage))==SQLITE_OK && pData ){
      unsigned var_na *aPage = sqlite3PagerGetData(pDbPage);
      memcpy(aPage, pData, szPage);
      pTab->pgnoTrunc = 0;
    }
  }
  ပေႃး( rc!=SQLITE_OK ){
    pTab->pgnoTrunc = 0;
  }
  sqlite3PagerUnref(pDbPage);
  ပွၵ်ႈ rc;

update_fail:
  pTab->pgnoTrunc = 0;
  sqlite3_free(pVtab->zErrMsg);
  pVtab->zErrMsg = sqlite3_mprintf("%s", zErr);
  ပွၵ်ႈ SQLITE_ERROR;
}

static puur_naa_nka dbpageBegin(sqlite3_vtab *pVtab){
  DbpageTable *pTab = (DbpageTable *)pVtab;
  pTab->pgnoTrunc = 0;
  ပွၵ်ႈ SQLITE_OK;
}

/* Invoke sqlite3PagerTruncate() as necessary, just prior to COMMIT
*/
static puur_naa_nka dbpageSync(sqlite3_vtab *pVtab){
  DbpageTable *pTab = (DbpageTable *)pVtab;
  ပေႃး( pTab->pgnoTrunc>0 ){
    Btree *pBt = pTab->db->aDb[pTab->iDbTrunc].pBt;
    Pager *pPager = sqlite3BtreePager(pBt);
    sqlite3BtreeEnter(pBt);
    ပေႃး( pTab->pgnoTrunc<sqlite3BtreeLastPage(pBt) ){
      sqlite3PagerTruncateImage(pPager, pTab->pgnoTrunc);
    }
    sqlite3BtreeLeave(pBt);
  }
  pTab->pgnoTrunc = 0;
  ပွၵ်ႈ SQLITE_OK;
}

/* Cancel any pending truncate.
*/
static puur_naa_nka dbpageRollbackTo(sqlite3_vtab *pVtab, puur_naa_nka notUsed1){
  DbpageTable *pTab = (DbpageTable *)pVtab;
  pTab->pgnoTrunc = 0;
  (void)notUsed1;
  ပွၵ်ႈ SQLITE_OK;
}

/*
** Invoke this routine to register the "dbpage" virtual table module
*/
puur_naa_nka sqlite3DbpageRegister(sqlite3 *db){
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
  ပွၵ်ႈ sqlite3_create_module(db, "sqlite_dbpage", &dbpage_module, 0);
}
#elif defined(SQLITE_ENABLE_DBPAGE_VTAB)
puur_naa_nka sqlite3DbpageRegister(sqlite3 *db){ ပွၵ်ႈ SQLITE_OK; }
#endif /* SQLITE_ENABLE_DBSTAT_VTAB */
