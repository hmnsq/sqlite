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

#அந்தர்பாவ "sqliteInt.h"   /* Requires access to internal data structures */
#எனில் (defined(SQLITE_ENABLE_DBPAGE_VTAB) || defined(SQLITE_TEST)) \
    && !defined(SQLITE_OMIT_VIRTUALTABLE)

வகைவரை கட்டமைப்பு DbpageTable DbpageTable;
வகைவரை கட்டமைப்பு DbpageCursor DbpageCursor;

கட்டமைப்பு DbpageCursor {
  sqlite3_vtab_cursor base;       /* Base class.  Must be first */
  Pgno pgno;                      /* Current page number */
  Pgno mxPgno;                    /* Last page to visit on this scan */
  Pager *pPager;                  /* Pager being read/written */
  DbPage *pPage1;                 /* Page 1 of the database */
  முழு iDb;                        /* Index of database to analyze */
  முழு szPage;                     /* Size of each page in bytes */
};

கட்டமைப்பு DbpageTable {
  sqlite3_vtab base;              /* Base class.  Must be first */
  sqlite3 *db;                    /* The database */
  முழு iDbTrunc;                   /* Database to truncate */
  Pgno pgnoTrunc;                 /* Size to truncate to */
};

/* Columns */
#define DBPAGE_COLUMN_PGNO    0
#define DBPAGE_COLUMN_DATA    1
#define DBPAGE_COLUMN_SCHEMA  2


/*
** Connect to or create a dbpagevfs virtual table.
*/
நிலையான முழு dbpageConnect(
  sqlite3 *db,
  வெறுமை *pAux,
  முழு argc, மாறிலி எழுத்து *மாறிலி*argv,
  sqlite3_vtab **ppVtab,
  எழுத்து **pzErr
){
  DbpageTable *pTab = 0;
  முழு rc = SQLITE_OK;
  (வெறுமை)pAux;
  (வெறுமை)argc;
  (வெறுமை)argv;
  (வெறுமை)pzErr;

  sqlite3_vtab_config(db, SQLITE_VTAB_DIRECTONLY);
  sqlite3_vtab_config(db, SQLITE_VTAB_USES_ALL_SCHEMAS);
  rc = sqlite3_declare_vtab(db, 
          "CREATE TABLE x(pgno INTEGER PRIMARY KEY, data BLOB, schema HIDDEN)");
  எனில்( rc==SQLITE_OK ){
    pTab = (DbpageTable *)sqlite3_malloc64(அளவு(DbpageTable));
    எனில்( pTab==0 ) rc = SQLITE_NOMEM_BKPT;
  }

  ப்ரதிஜ்ஞா( rc==SQLITE_OK || pTab==0 );
  எனில்( rc==SQLITE_OK ){
    memset(pTab, 0, அளவு(DbpageTable));
    pTab->db = db;
  }

  *ppVtab = (sqlite3_vtab*)pTab;
  திரும்பு rc;
}

/*
** Disconnect from or destroy a dbpagevfs virtual table.
*/
நிலையான முழு dbpageDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  திரும்பு SQLITE_OK;
}

/*
** idxNum:
**
**     0     schema=main, full table scan
**     1     schema=main, pgno=?1
**     2     schema=?1, full table scan
**     3     schema=?1, pgno=?2
*/
நிலையான முழு dbpageBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  முழு i;
  முழு iPlan = 0;
  (வெறுமை)tab;

  /* If there is a schema= constraint, it must be honored.  Report a
  ** ridiculously large estimated cost if the schema= constraint is
  ** unavailable
  */
  க்கு(i=0; i<pIdxInfo->nConstraint; i++){
    கட்டமைப்பு sqlite3_index_constraint *p = &pIdxInfo->aConstraint[i];
    எனில்( p->iColumn!=DBPAGE_COLUMN_SCHEMA ) தொடர்;
    எனில்( p->op!=SQLITE_INDEX_CONSTRAINT_EQ ) தொடர்;
    எனில்( !p->usable ){
      /* No solution. */
      திரும்பு SQLITE_CONSTRAINT;
    }
    iPlan = 2;
    pIdxInfo->aConstraintUsage[i].argvIndex = 1;
    pIdxInfo->aConstraintUsage[i].omit = 1;
    நிறுத்து;
  }

  /* If we reach this point, it means that either there is no schema=
  ** constraint (in which case we use the "main" schema) or else the
  ** schema constraint was accepted.  Lower the estimated cost accordingly
  */
  pIdxInfo->estimatedCost = 1.0e6;

  /* Check for constraints against pgno */
  க்கு(i=0; i<pIdxInfo->nConstraint; i++){
    கட்டமைப்பு sqlite3_index_constraint *p = &pIdxInfo->aConstraint[i];
    எனில்( p->usable && p->iColumn<=0 && p->op==SQLITE_INDEX_CONSTRAINT_EQ ){
      pIdxInfo->estimatedRows = 1;
      pIdxInfo->idxFlags = SQLITE_INDEX_SCAN_UNIQUE;
      pIdxInfo->estimatedCost = 1.0;
      pIdxInfo->aConstraintUsage[i].argvIndex = iPlan ? 2 : 1;
      pIdxInfo->aConstraintUsage[i].omit = 1;
      iPlan |= 1;
      நிறுத்து;
    }
  }
  pIdxInfo->idx_இலக்கம் = iPlan;

  எனில்( pIdxInfo->nOrderBy>=1
   && pIdxInfo->aOrderBy[0].iColumn<=0
   && pIdxInfo->aOrderBy[0].desc==0
  ){
    pIdxInfo->orderByConsumed = 1;
  }
  திரும்பு SQLITE_OK;
}

/*
** Open a new dbpagevfs cursor.
*/
நிலையான முழு dbpageOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  DbpageCursor *pCsr;

  pCsr = (DbpageCursor *)sqlite3_malloc64(அளவு(DbpageCursor));
  எனில்( pCsr==0 ){
    திரும்பு SQLITE_NOMEM_BKPT;
  }இல்லையெனில்{
    memset(pCsr, 0, அளவு(DbpageCursor));
    pCsr->base.pVtab = pVTab;
    pCsr->pgno = 0;
  }

  *ppCursor = (sqlite3_vtab_cursor *)pCsr;
  திரும்பு SQLITE_OK;
}

/*
** Close a dbpagevfs cursor.
*/
நிலையான முழு dbpageClose(sqlite3_vtab_cursor *pCursor){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  எனில்( pCsr->pPage1 ) sqlite3PagerUnrefPageOne(pCsr->pPage1);
  sqlite3_free(pCsr);
  திரும்பு SQLITE_OK;
}

/*
** Move a dbpagevfs cursor to the next entry in the file.
*/
நிலையான முழு dbpageNext(sqlite3_vtab_cursor *pCursor){
  முழு rc = SQLITE_OK;
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  pCsr->pgno++;
  திரும்பு rc;
}

நிலையான முழு dbpageEof(sqlite3_vtab_cursor *pCursor){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  திரும்பு pCsr->pgno > pCsr->mxPgno;
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
நிலையான முழு dbpageFilter(
  sqlite3_vtab_cursor *pCursor,
  முழு idx_இலக்கம், மாறிலி எழுத்து *idxStr,
  முழு argc, sqlite3_மதிப்பு **argv
){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  DbpageTable *pTab = (DbpageTable *)pCursor->pVtab;
  முழு rc;
  sqlite3 *db = pTab->db;
  Btree *pBt;

  UNUSED_PARAMETER(idxStr);
  UNUSED_PARAMETER(argc);

  /* Default setting is no rows of result */
  pCsr->pgno = 1;
  pCsr->mxPgno = 0;

  எனில்( idx_இலக்கம் & 2 ){
    மாறிலி எழுத்து *zSchema;
    ப்ரதிஜ்ஞா( argc>=1 );
    zSchema = (மாறிலி எழுத்து*)sqlite3_மதிப்பு_text(argv[0]);
    pCsr->iDb = sqlite3FindDbName(db, zSchema);
    எனில்( pCsr->iDb<0 ) திரும்பு SQLITE_OK;
  }இல்லையெனில்{
    pCsr->iDb = 0;
  }
  pBt = db->aDb[pCsr->iDb].pBt;
  எனில்( NEVER(pBt==0) ) திரும்பு SQLITE_OK;
  pCsr->pPager = sqlite3BtreePager(pBt);
  pCsr->szPage = sqlite3BtreeGetPageSize(pBt);
  pCsr->mxPgno = sqlite3BtreeLastPage(pBt);
  எனில்( idx_இலக்கம் & 1 ){
    i64 iPg = sqlite3_மதிப்பு_int64(argv[idx_இலக்கம்>>1]);
    ப்ரதிஜ்ஞா( argc>(idx_இலக்கம்>>1) );
    எனில்( iPg<1 || iPg>pCsr->mxPgno ){
      pCsr->pgno = 1;
      pCsr->mxPgno = 0;
    }இல்லையெனில்{
      pCsr->pgno = (Pgno)iPg;
      pCsr->mxPgno = pCsr->pgno;
    }
  }இல்லையெனில்{
    ப்ரதிஜ்ஞா( pCsr->pgno==1 );
  }
  எனில்( pCsr->pPage1 ) sqlite3PagerUnrefPageOne(pCsr->pPage1);
  rc = sqlite3PagerGet(pCsr->pPager, 1, &pCsr->pPage1, 0);
  திரும்பு rc;
}

நிலையான முழு dbpageColumn(
  sqlite3_vtab_cursor *pCursor,
  sqlite3_context *ctx,
  முழு i
){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  முழு rc = SQLITE_OK;
  தேர்வு( i ){
    நிலை 0: {           /* pgno */
      sqlite3_result_int64(ctx, (sqlite3_int64)pCsr->pgno);
      நிறுத்து;
    }
    நிலை 1: {           /* data */
      DbPage *pDbPage = 0;
      எனில்( pCsr->pgno==(Pgno)((PENDING_BYTE/pCsr->szPage)+1) ){
        /* The pending byte page. Assume it is zeroed out. Attempting to
        ** request this page from the page is an SQLITE_CORRUPT error. */
        sqlite3_result_zeroblob(ctx, pCsr->szPage);
      }இல்லையெனில்{
        rc = sqlite3PagerGet(pCsr->pPager, pCsr->pgno, (DbPage**)&pDbPage, 0);
        எனில்( rc==SQLITE_OK ){
          sqlite3_result_blob(ctx, sqlite3PagerGetData(pDbPage), pCsr->szPage,
              SQLITE_TRANSIENT);
        }
        sqlite3PagerUnref(pDbPage);
      }
      நிறுத்து;
    }
    இயல்பு: {          /* schema */
      sqlite3 *db = sqlite3_context_db_handle(ctx);
      sqlite3_result_text(ctx, db->aDb[pCsr->iDb].zDbSName, -1, SQLITE_STATIC);
      நிறுத்து;
    }
  }
  திரும்பு rc;
}

நிலையான முழு dbpageRowid(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  *pRowid = pCsr->pgno;
  திரும்பு SQLITE_OK;
}

/* 
** Open write transactions. Since we do not know in advance which database
** files will be written by the sqlite_dbpage virtual table, start a write
** transaction on them all.
**
** Return SQLITE_OK if successful, or an SQLite error code otherwise.
*/
நிலையான முழு dbpageBeginTrans(DbpageTable *pTab){
  sqlite3 *db = pTab->db;
  முழு rc = SQLITE_OK;
  முழு i;
  க்கு(i=0; rc==SQLITE_OK && i<db->nDb; i++){
    Btree *pBt = db->aDb[i].pBt;
    எனில்( pBt ) rc = sqlite3BtreeBeginTrans(pBt, 1, 0);
  }
  திரும்பு rc;
}

நிலையான முழு dbpageUpdate(
  sqlite3_vtab *pVtab,
  முழு argc,
  sqlite3_மதிப்பு **argv,
  sqlite_int64 *pRowid
){
  DbpageTable *pTab = (DbpageTable *)pVtab;
  Pgno pgno;
  sqlite3_int64 pgno64;
  DbPage *pDbPage = 0;
  முழு rc = SQLITE_OK;
  எழுத்து *zErr = 0;
  முழு iDb;
  Btree *pBt;
  Pager *pPager;
  முழு szPage;
  முழு is_செருகு;

  (வெறுமை)pRowid;
  எனில்( pTab->db->flags & SQLITE_Defensive ){
    zErr = "read-only";
    செல் update_fail;
  }
  எனில்( argc==1 ){
    zErr = "cannot delete";
    செல் update_fail;
  }
  எனில்( sqlite3_மதிப்பு_type(argv[0])==SQLITE_NULL ){
    pgno64 = sqlite3_மதிப்பு_int64(argv[2]);
    is_செருகு = 1;
  }இல்லையெனில்{
    pgno64 = (Pgno)sqlite3_மதிப்பு_int64(argv[0]);
    எனில்( sqlite3_மதிப்பு_int64(argv[1])!=pgno64 ){
      zErr = "cannot insert";
      செல் update_fail;
    }
    is_செருகு = 0;
  }
  எனில்( sqlite3_மதிப்பு_type(argv[4])==SQLITE_NULL ){
    iDb = 0;
  }இல்லையெனில்{
    மாறிலி எழுத்து *zSchema = (மாறிலி எழுத்து*)sqlite3_மதிப்பு_text(argv[4]);
    iDb = sqlite3FindDbName(pTab->db, zSchema);
    எனில்( iDb<0 ){
      zErr = "no such schema";
      செல் update_fail;
    }
  }
  pBt = pTab->db->aDb[iDb].pBt;
  எனில்( pgno64<1 || pgno64>4294967294U || NEVER(pBt==0) ){
    zErr = "bad page number";
    செல் update_fail;
  }
  pgno = (Pgno)pgno64;
  szPage = sqlite3BtreeGetPageSize(pBt);
  எனில்( sqlite3_மதிப்பு_type(argv[3])!=SQLITE_BLOB 
   || sqlite3_மதிப்பு_bytes(argv[3])!=szPage
  ){
    எனில்( sqlite3_மதிப்பு_type(argv[3])==SQLITE_NULL && is_செருகு && pgno>1 ){
      /* "INSERT INTO dbpage($PGNO,NULL)" causes page number $PGNO and
      ** all subsequent pages to be deleted. */
      pTab->iDbTrunc = iDb;
      pTab->pgnoTrunc = pgno-1;
      pgno = 1;
    }இல்லையெனில்{
      zErr = "bad page value";
      செல் update_fail;
    }
  }

  எனில்( dbpageBeginTrans(pTab)!=SQLITE_OK ){
    zErr = "failed to open transaction";
    செல் update_fail;
  }

  pPager = sqlite3BtreePager(pBt);
  rc = sqlite3PagerGet(pPager, pgno, (DbPage**)&pDbPage, 0);
  எனில்( rc==SQLITE_OK ){
    மாறிலி வெறுமை *pData = sqlite3_மதிப்பு_blob(argv[3]);
    எனில்( (rc = sqlite3PagerWrite(pDbPage))==SQLITE_OK && pData ){
      unsigned எழுத்து *aPage = sqlite3PagerGetData(pDbPage);
      memcpy(aPage, pData, szPage);
      pTab->pgnoTrunc = 0;
    }
  }
  எனில்( rc!=SQLITE_OK ){
    pTab->pgnoTrunc = 0;
  }
  sqlite3PagerUnref(pDbPage);
  திரும்பு rc;

update_fail:
  pTab->pgnoTrunc = 0;
  sqlite3_free(pVtab->z_Err_செய்தி);
  pVtab->z_Err_செய்தி = sqlite3_mprintf("%s", zErr);
  திரும்பு SQLITE_ERROR;
}

நிலையான முழு dbpageBegin(sqlite3_vtab *pVtab){
  DbpageTable *pTab = (DbpageTable *)pVtab;
  pTab->pgnoTrunc = 0;
  திரும்பு SQLITE_OK;
}

/* Invoke sqlite3PagerTruncate() as necessary, just prior to COMMIT
*/
நிலையான முழு dbpageSync(sqlite3_vtab *pVtab){
  DbpageTable *pTab = (DbpageTable *)pVtab;
  எனில்( pTab->pgnoTrunc>0 ){
    Btree *pBt = pTab->db->aDb[pTab->iDbTrunc].pBt;
    Pager *pPager = sqlite3BtreePager(pBt);
    sqlite3BtreeEnter(pBt);
    எனில்( pTab->pgnoTrunc<sqlite3BtreeLastPage(pBt) ){
      sqlite3PagerTruncateImage(pPager, pTab->pgnoTrunc);
    }
    sqlite3BtreeLeave(pBt);
  }
  pTab->pgnoTrunc = 0;
  திரும்பு SQLITE_OK;
}

/* Cancel any pending truncate.
*/
நிலையான முழு dbpageRollbackTo(sqlite3_vtab *pVtab, முழு notUsed1){
  DbpageTable *pTab = (DbpageTable *)pVtab;
  pTab->pgnoTrunc = 0;
  (வெறுமை)notUsed1;
  திரும்பு SQLITE_OK;
}

/*
** Invoke this routine to register the "dbpage" virtual table module
*/
முழு sqlite3DbpageRegister(sqlite3 *db){
  நிலையான sqlite3_module dbpage_module = {
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
  திரும்பு sqlite3_create_module(db, "sqlite_dbpage", &dbpage_module, 0);
}
#அல்லதெனில் defined(SQLITE_ENABLE_DBPAGE_VTAB)
முழு sqlite3DbpageRegister(sqlite3 *db){ திரும்பு SQLITE_OK; }
#endif /* SQLITE_ENABLE_DBSTAT_VTAB */
