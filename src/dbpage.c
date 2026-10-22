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

#ଅନ୍ତର୍ଭାଵ "sqliteInt.h"   /* Requires access to internal data structures */
#ଯଦି (defined(SQLITE_ENABLE_DBPAGE_VTAB) || defined(SQLITE_TEST)) \
    && !defined(SQLITE_OMIT_VIRTUALTABLE)

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା DbpageTable DbpageTable;
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା DbpageCursor DbpageCursor;

ସଂରଚନା DbpageCursor {
  sqlite3_vtab_cursor base;       /* Base class.  Must be first */
  Pgno pgno;                      /* Current page number */
  Pgno mxPgno;                    /* Last page to visit on this scan */
  Pager *pPager;                  /* Pager being read/written */
  DbPage *pPage1;                 /* Page 1 of the database */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDb;                        /* Index of database to analyze */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା szPage;                     /* Size of each page in bytes */
};

ସଂରଚନା DbpageTable {
  sqlite3_vtab base;              /* Base class.  Must be first */
  sqlite3 *db;                    /* The database */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDbTrunc;                   /* Database to truncate */
  Pgno pgnoTrunc;                 /* Size to truncate to */
};

/* Columns */
#define DBPAGE_COLUMN_PGNO    0
#define DBPAGE_COLUMN_DATA    1
#define DBPAGE_COLUMN_SCHEMA  2


/*
** Connect to or create a dbpagevfs virtual table.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା dbpageConnect(
  sqlite3 *db,
  ଶୂନ୍ୟ *pAux,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅଚଳ ଅକ୍ଷର *ଅଚଳ*argv,
  sqlite3_vtab **ppVtab,
  ଅକ୍ଷର **pzErr
){
  DbpageTable *pTab = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
  (ଶୂନ୍ୟ)pAux;
  (ଶୂନ୍ୟ)argc;
  (ଶୂନ୍ୟ)argv;
  (ଶୂନ୍ୟ)pzErr;

  sqlite3_vtab_config(db, SQLITE_VTAB_DIRECTONLY);
  sqlite3_vtab_config(db, SQLITE_VTAB_USES_ALL_SCHEMAS);
  rc = sqlite3_declare_vtab(db, 
          "CREATE TABLE x(pgno INTEGER PRIMARY KEY, data BLOB, schema HIDDEN)");
  ଯଦି( rc==SQLITE_OK ){
    pTab = (DbpageTable *)sqlite3_malloc64(ଆକାର(DbpageTable));
    ଯଦି( pTab==0 ) rc = SQLITE_NOMEM_BKPT;
  }

  ପ୍ରତିଜ୍ଞା( rc==SQLITE_OK || pTab==0 );
  ଯଦି( rc==SQLITE_OK ){
    memset(pTab, 0, ଆକାର(DbpageTable));
    pTab->db = db;
  }

  *ppVtab = (sqlite3_vtab*)pTab;
  ଫେରନ୍ତୁ rc;
}

/*
** Disconnect from or destroy a dbpagevfs virtual table.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା dbpageDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** idxNum:
**
**     0     schema=main, full table scan
**     1     schema=main, pgno=?1
**     2     schema=?1, full table scan
**     3     schema=?1, pgno=?2
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା dbpageBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iPlan = 0;
  (ଶୂନ୍ୟ)tab;

  /* If there is a schema= constraint, it must be honored.  Report a
  ** ridiculously large estimated cost if the schema= constraint is
  ** unavailable
  */
  ପାଇଁ(i=0; i<pIdxInfo->nConstraint; i++){
    ସଂରଚନା sqlite3_index_constraint *p = &pIdxInfo->aConstraint[i];
    ଯଦି( p->iColumn!=DBPAGE_COLUMN_SCHEMA ) ଜାରି;
    ଯଦି( p->op!=SQLITE_INDEX_CONSTRAINT_EQ ) ଜାରି;
    ଯଦି( !p->usable ){
      /* No solution. */
      ଫେରନ୍ତୁ SQLITE_CONSTRAINT;
    }
    iPlan = 2;
    pIdxInfo->aConstraintUsage[i].argvIndex = 1;
    pIdxInfo->aConstraintUsage[i].omit = 1;
    ଭାଙ୍ଗ;
  }

  /* If we reach this point, it means that either there is no schema=
  ** constraint (in which case we use the "main" schema) or else the
  ** schema constraint was accepted.  Lower the estimated cost accordingly
  */
  pIdxInfo->estimatedCost = 1.0e6;

  /* Check for constraints against pgno */
  ପାଇଁ(i=0; i<pIdxInfo->nConstraint; i++){
    ସଂରଚନା sqlite3_index_constraint *p = &pIdxInfo->aConstraint[i];
    ଯଦି( p->usable && p->iColumn<=0 && p->op==SQLITE_INDEX_CONSTRAINT_EQ ){
      pIdxInfo->estimatedRows = 1;
      pIdxInfo->idxFlags = SQLITE_INDEX_SCAN_UNIQUE;
      pIdxInfo->estimatedCost = 1.0;
      pIdxInfo->aConstraintUsage[i].argvIndex = iPlan ? 2 : 1;
      pIdxInfo->aConstraintUsage[i].omit = 1;
      iPlan |= 1;
      ଭାଙ୍ଗ;
    }
  }
  pIdxInfo->idx_ଅଙ୍କ = iPlan;

  ଯଦି( pIdxInfo->nOrderBy>=1
   && pIdxInfo->aOrderBy[0].iColumn<=0
   && pIdxInfo->aOrderBy[0].desc==0
  ){
    pIdxInfo->orderByConsumed = 1;
  }
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Open a new dbpagevfs cursor.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା dbpageOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  DbpageCursor *pCsr;

  pCsr = (DbpageCursor *)sqlite3_malloc64(ଆକାର(DbpageCursor));
  ଯଦି( pCsr==0 ){
    ଫେରନ୍ତୁ SQLITE_NOMEM_BKPT;
  }ଅନ୍ୟଥା{
    memset(pCsr, 0, ଆକାର(DbpageCursor));
    pCsr->base.pVtab = pVTab;
    pCsr->pgno = 0;
  }

  *ppCursor = (sqlite3_vtab_cursor *)pCsr;
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Close a dbpagevfs cursor.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା dbpageClose(sqlite3_vtab_cursor *pCursor){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  ଯଦି( pCsr->pPage1 ) sqlite3PagerUnrefPageOne(pCsr->pPage1);
  sqlite3_free(pCsr);
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Move a dbpagevfs cursor to the next entry in the file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା dbpageNext(sqlite3_vtab_cursor *pCursor){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  pCsr->pgno++;
  ଫେରନ୍ତୁ rc;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା dbpageEof(sqlite3_vtab_cursor *pCursor){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  ଫେରନ୍ତୁ pCsr->pgno > pCsr->mxPgno;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା dbpageFilter(
  sqlite3_vtab_cursor *pCursor,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx_ଅଙ୍କ, ଅଚଳ ଅକ୍ଷର *idxStr,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, sqlite3_ମୂଲ୍ୟ **argv
){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  DbpageTable *pTab = (DbpageTable *)pCursor->pVtab;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  sqlite3 *db = pTab->db;
  Btree *pBt;

  UNUSED_PARAMETER(idxStr);
  UNUSED_PARAMETER(argc);

  /* Default setting is no rows of result */
  pCsr->pgno = 1;
  pCsr->mxPgno = 0;

  ଯଦି( idx_ଅଙ୍କ & 2 ){
    ଅଚଳ ଅକ୍ଷର *zSchema;
    ପ୍ରତିଜ୍ଞା( argc>=1 );
    zSchema = (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(argv[0]);
    pCsr->iDb = sqlite3FindDbName(db, zSchema);
    ଯଦି( pCsr->iDb<0 ) ଫେରନ୍ତୁ SQLITE_OK;
  }ଅନ୍ୟଥା{
    pCsr->iDb = 0;
  }
  pBt = db->aDb[pCsr->iDb].pBt;
  ଯଦି( NEVER(pBt==0) ) ଫେରନ୍ତୁ SQLITE_OK;
  pCsr->pPager = sqlite3BtreePager(pBt);
  pCsr->szPage = sqlite3BtreeGetPageSize(pBt);
  pCsr->mxPgno = sqlite3BtreeLastPage(pBt);
  ଯଦି( idx_ଅଙ୍କ & 1 ){
    i64 iPg = sqlite3_ମୂଲ୍ୟ_int64(argv[idx_ଅଙ୍କ>>1]);
    ପ୍ରତିଜ୍ଞା( argc>(idx_ଅଙ୍କ>>1) );
    ଯଦି( iPg<1 || iPg>pCsr->mxPgno ){
      pCsr->pgno = 1;
      pCsr->mxPgno = 0;
    }ଅନ୍ୟଥା{
      pCsr->pgno = (Pgno)iPg;
      pCsr->mxPgno = pCsr->pgno;
    }
  }ଅନ୍ୟଥା{
    ପ୍ରତିଜ୍ଞା( pCsr->pgno==1 );
  }
  ଯଦି( pCsr->pPage1 ) sqlite3PagerUnrefPageOne(pCsr->pPage1);
  rc = sqlite3PagerGet(pCsr->pPager, 1, &pCsr->pPage1, 0);
  ଫେରନ୍ତୁ rc;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା dbpageColumn(
  sqlite3_vtab_cursor *pCursor,
  sqlite3_context *ctx,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i
){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
  ବାଛ( i ){
    ସ୍ଥିତି 0: {           /* pgno */
      sqlite3_result_int64(ctx, (sqlite3_int64)pCsr->pgno);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି 1: {           /* data */
      DbPage *pDbPage = 0;
      ଯଦି( pCsr->pgno==(Pgno)((PENDING_BYTE/pCsr->szPage)+1) ){
        /* The pending byte page. Assume it is zeroed out. Attempting to
        ** request this page from the page is an SQLITE_CORRUPT error. */
        sqlite3_result_zeroblob(ctx, pCsr->szPage);
      }ଅନ୍ୟଥା{
        rc = sqlite3PagerGet(pCsr->pPager, pCsr->pgno, (DbPage**)&pDbPage, 0);
        ଯଦି( rc==SQLITE_OK ){
          sqlite3_result_blob(ctx, sqlite3PagerGetData(pDbPage), pCsr->szPage,
              SQLITE_TRANSIENT);
        }
        sqlite3PagerUnref(pDbPage);
      }
      ଭାଙ୍ଗ;
    }
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {          /* schema */
      sqlite3 *db = sqlite3_context_db_handle(ctx);
      sqlite3_result_text(ctx, db->aDb[pCsr->iDb].zDbSName, -1, SQLITE_STATIC);
      ଭାଙ୍ଗ;
    }
  }
  ଫେରନ୍ତୁ rc;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା dbpageRowid(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  *pRowid = pCsr->pgno;
  ଫେରନ୍ତୁ SQLITE_OK;
}

/* 
** Open write transactions. Since we do not know in advance which database
** files will be written by the sqlite_dbpage virtual table, start a write
** transaction on them all.
**
** Return SQLITE_OK if successful, or an SQLite error code otherwise.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା dbpageBeginTrans(DbpageTable *pTab){
  sqlite3 *db = pTab->db;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପାଇଁ(i=0; rc==SQLITE_OK && i<db->nDb; i++){
    Btree *pBt = db->aDb[i].pBt;
    ଯଦି( pBt ) rc = sqlite3BtreeBeginTrans(pBt, 1, 0);
  }
  ଫେରନ୍ତୁ rc;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା dbpageUpdate(
  sqlite3_vtab *pVtab,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,
  sqlite3_ମୂଲ୍ୟ **argv,
  sqlite_int64 *pRowid
){
  DbpageTable *pTab = (DbpageTable *)pVtab;
  Pgno pgno;
  sqlite3_int64 pgno64;
  DbPage *pDbPage = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
  ଅକ୍ଷର *zErr = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDb;
  Btree *pBt;
  Pager *pPager;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା szPage;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା is_ଭର୍ତ୍ତି;

  (ଶୂନ୍ୟ)pRowid;
  ଯଦି( pTab->db->flags & SQLITE_Defensive ){
    zErr = "read-only";
    ଯାଅ update_fail;
  }
  ଯଦି( argc==1 ){
    zErr = "cannot delete";
    ଯାଅ update_fail;
  }
  ଯଦି( sqlite3_ମୂଲ୍ୟ_type(argv[0])==SQLITE_NULL ){
    pgno64 = sqlite3_ମୂଲ୍ୟ_int64(argv[2]);
    is_ଭର୍ତ୍ତି = 1;
  }ଅନ୍ୟଥା{
    pgno64 = (Pgno)sqlite3_ମୂଲ୍ୟ_int64(argv[0]);
    ଯଦି( sqlite3_ମୂଲ୍ୟ_int64(argv[1])!=pgno64 ){
      zErr = "cannot insert";
      ଯାଅ update_fail;
    }
    is_ଭର୍ତ୍ତି = 0;
  }
  ଯଦି( sqlite3_ମୂଲ୍ୟ_type(argv[4])==SQLITE_NULL ){
    iDb = 0;
  }ଅନ୍ୟଥା{
    ଅଚଳ ଅକ୍ଷର *zSchema = (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(argv[4]);
    iDb = sqlite3FindDbName(pTab->db, zSchema);
    ଯଦି( iDb<0 ){
      zErr = "no such schema";
      ଯାଅ update_fail;
    }
  }
  pBt = pTab->db->aDb[iDb].pBt;
  ଯଦି( pgno64<1 || pgno64>4294967294U || NEVER(pBt==0) ){
    zErr = "bad page number";
    ଯାଅ update_fail;
  }
  pgno = (Pgno)pgno64;
  szPage = sqlite3BtreeGetPageSize(pBt);
  ଯଦି( sqlite3_ମୂଲ୍ୟ_type(argv[3])!=SQLITE_BLOB 
   || sqlite3_ମୂଲ୍ୟ_bytes(argv[3])!=szPage
  ){
    ଯଦି( sqlite3_ମୂଲ୍ୟ_type(argv[3])==SQLITE_NULL && is_ଭର୍ତ୍ତି && pgno>1 ){
      /* "INSERT INTO dbpage($PGNO,NULL)" causes page number $PGNO and
      ** all subsequent pages to be deleted. */
      pTab->iDbTrunc = iDb;
      pTab->pgnoTrunc = pgno-1;
      pgno = 1;
    }ଅନ୍ୟଥା{
      zErr = "bad page value";
      ଯାଅ update_fail;
    }
  }

  ଯଦି( dbpageBeginTrans(pTab)!=SQLITE_OK ){
    zErr = "failed to open transaction";
    ଯାଅ update_fail;
  }

  pPager = sqlite3BtreePager(pBt);
  rc = sqlite3PagerGet(pPager, pgno, (DbPage**)&pDbPage, 0);
  ଯଦି( rc==SQLITE_OK ){
    ଅଚଳ ଶୂନ୍ୟ *pData = sqlite3_ମୂଲ୍ୟ_blob(argv[3]);
    ଯଦି( (rc = sqlite3PagerWrite(pDbPage))==SQLITE_OK && pData ){
      unsigned ଅକ୍ଷର *aPage = sqlite3PagerGetData(pDbPage);
      memcpy(aPage, pData, szPage);
      pTab->pgnoTrunc = 0;
    }
  }
  ଯଦି( rc!=SQLITE_OK ){
    pTab->pgnoTrunc = 0;
  }
  sqlite3PagerUnref(pDbPage);
  ଫେରନ୍ତୁ rc;

update_fail:
  pTab->pgnoTrunc = 0;
  sqlite3_free(pVtab->z_Err_ସନ୍ଦେଶ);
  pVtab->z_Err_ସନ୍ଦେଶ = sqlite3_mprintf("%s", zErr);
  ଫେରନ୍ତୁ SQLITE_ERROR;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା dbpageBegin(sqlite3_vtab *pVtab){
  DbpageTable *pTab = (DbpageTable *)pVtab;
  pTab->pgnoTrunc = 0;
  ଫେରନ୍ତୁ SQLITE_OK;
}

/* Invoke sqlite3PagerTruncate() as necessary, just prior to COMMIT
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା dbpageSync(sqlite3_vtab *pVtab){
  DbpageTable *pTab = (DbpageTable *)pVtab;
  ଯଦି( pTab->pgnoTrunc>0 ){
    Btree *pBt = pTab->db->aDb[pTab->iDbTrunc].pBt;
    Pager *pPager = sqlite3BtreePager(pBt);
    sqlite3BtreeEnter(pBt);
    ଯଦି( pTab->pgnoTrunc<sqlite3BtreeLastPage(pBt) ){
      sqlite3PagerTruncateImage(pPager, pTab->pgnoTrunc);
    }
    sqlite3BtreeLeave(pBt);
  }
  pTab->pgnoTrunc = 0;
  ଫେରନ୍ତୁ SQLITE_OK;
}

/* Cancel any pending truncate.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା dbpageRollbackTo(sqlite3_vtab *pVtab, ପୂର୍ଣ୍ଣସଂଖ୍ୟା notUsed1){
  DbpageTable *pTab = (DbpageTable *)pVtab;
  pTab->pgnoTrunc = 0;
  (ଶୂନ୍ୟ)notUsed1;
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Invoke this routine to register the "dbpage" virtual table module
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3DbpageRegister(sqlite3 *db){
  ସ୍ଥିର sqlite3_module dbpage_module = {
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
  ଫେରନ୍ତୁ sqlite3_create_module(db, "sqlite_dbpage", &dbpage_module, 0);
}
#ନଚେତଯଦି defined(SQLITE_ENABLE_DBPAGE_VTAB)
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3DbpageRegister(sqlite3 *db){ ଫେରନ୍ତୁ SQLITE_OK; }
#endif /* SQLITE_ENABLE_DBSTAT_VTAB */
