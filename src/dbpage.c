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

#അന്തര്ഭാവ "sqliteInt.h"   /* Requires access to internal data structures */
#എങ്കിൽ (defined(SQLITE_ENABLE_DBPAGE_VTAB) || defined(SQLITE_TEST)) \
    && !defined(SQLITE_OMIT_VIRTUALTABLE)

തരനിർവചനം ഘടന DbpageTable DbpageTable;
തരനിർവചനം ഘടന DbpageCursor DbpageCursor;

ഘടന DbpageCursor {
  sqlite3_vtab_cursor base;       /* Base class.  Must be first */
  Pgno pgno;                      /* Current page number */
  Pgno mxPgno;                    /* Last page to visit on this scan */
  Pager *pPager;                  /* Pager being read/written */
  DbPage *pPage1;                 /* Page 1 of the database */
  പൂർണ്ണസംഖ്യ iDb;                        /* Index of database to analyze */
  പൂർണ്ണസംഖ്യ szPage;                     /* Size of each page in bytes */
};

ഘടന DbpageTable {
  sqlite3_vtab base;              /* Base class.  Must be first */
  sqlite3 *db;                    /* The database */
  പൂർണ്ണസംഖ്യ iDbTrunc;                   /* Database to truncate */
  Pgno pgnoTrunc;                 /* Size to truncate to */
};

/* Columns */
#define DBPAGE_COLUMN_PGNO    0
#define DBPAGE_COLUMN_DATA    1
#define DBPAGE_COLUMN_SCHEMA  2


/*
** Connect to or create a dbpagevfs virtual table.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ dbpageConnect(
  sqlite3 *db,
  ശൂന്യം *pAux,
  പൂർണ്ണസംഖ്യ argc, സ്ഥിരം അക്ഷരം *സ്ഥിരം*argv,
  sqlite3_vtab **ppVtab,
  അക്ഷരം **pzErr
){
  DbpageTable *pTab = 0;
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;
  (ശൂന്യം)pAux;
  (ശൂന്യം)argc;
  (ശൂന്യം)argv;
  (ശൂന്യം)pzErr;

  sqlite3_vtab_config(db, SQLITE_VTAB_DIRECTONLY);
  sqlite3_vtab_config(db, SQLITE_VTAB_USES_ALL_SCHEMAS);
  rc = sqlite3_declare_vtab(db, 
          "CREATE TABLE x(pgno INTEGER PRIMARY KEY, data BLOB, schema HIDDEN)");
  എങ്കിൽ( rc==SQLITE_OK ){
    pTab = (DbpageTable *)sqlite3_malloc64(വലുപ്പം(DbpageTable));
    എങ്കിൽ( pTab==0 ) rc = SQLITE_NOMEM_BKPT;
  }

  പ്രതിജ്ഞാ( rc==SQLITE_OK || pTab==0 );
  എങ്കിൽ( rc==SQLITE_OK ){
    memset(pTab, 0, വലുപ്പം(DbpageTable));
    pTab->db = db;
  }

  *ppVtab = (sqlite3_vtab*)pTab;
  തിരികെ rc;
}

/*
** Disconnect from or destroy a dbpagevfs virtual table.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ dbpageDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  തിരികെ SQLITE_OK;
}

/*
** idxNum:
**
**     0     schema=main, full table scan
**     1     schema=main, pgno=?1
**     2     schema=?1, full table scan
**     3     schema=?1, pgno=?2
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ dbpageBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  പൂർണ്ണസംഖ്യ i;
  പൂർണ്ണസംഖ്യ iPlan = 0;
  (ശൂന്യം)tab;

  /* If there is a schema= constraint, it must be honored.  Report a
  ** ridiculously large estimated cost if the schema= constraint is
  ** unavailable
  */
  വേണ്ടി(i=0; i<pIdxInfo->nConstraint; i++){
    ഘടന sqlite3_index_constraint *p = &pIdxInfo->aConstraint[i];
    എങ്കിൽ( p->iColumn!=DBPAGE_COLUMN_SCHEMA ) തുടരുക;
    എങ്കിൽ( p->op!=SQLITE_INDEX_CONSTRAINT_EQ ) തുടരുക;
    എങ്കിൽ( !p->usable ){
      /* No solution. */
      തിരികെ SQLITE_CONSTRAINT;
    }
    iPlan = 2;
    pIdxInfo->aConstraintUsage[i].argvIndex = 1;
    pIdxInfo->aConstraintUsage[i].omit = 1;
    നിർത്തുക;
  }

  /* If we reach this point, it means that either there is no schema=
  ** constraint (in which case we use the "main" schema) or else the
  ** schema constraint was accepted.  Lower the estimated cost accordingly
  */
  pIdxInfo->estimatedCost = 1.0e6;

  /* Check for constraints against pgno */
  വേണ്ടി(i=0; i<pIdxInfo->nConstraint; i++){
    ഘടന sqlite3_index_constraint *p = &pIdxInfo->aConstraint[i];
    എങ്കിൽ( p->usable && p->iColumn<=0 && p->op==SQLITE_INDEX_CONSTRAINT_EQ ){
      pIdxInfo->estimatedRows = 1;
      pIdxInfo->idxFlags = SQLITE_INDEX_SCAN_UNIQUE;
      pIdxInfo->estimatedCost = 1.0;
      pIdxInfo->aConstraintUsage[i].argvIndex = iPlan ? 2 : 1;
      pIdxInfo->aConstraintUsage[i].omit = 1;
      iPlan |= 1;
      നിർത്തുക;
    }
  }
  pIdxInfo->idx_അക്കം = iPlan;

  എങ്കിൽ( pIdxInfo->nOrderBy>=1
   && pIdxInfo->aOrderBy[0].iColumn<=0
   && pIdxInfo->aOrderBy[0].desc==0
  ){
    pIdxInfo->orderByConsumed = 1;
  }
  തിരികെ SQLITE_OK;
}

/*
** Open a new dbpagevfs cursor.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ dbpageOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  DbpageCursor *pCsr;

  pCsr = (DbpageCursor *)sqlite3_malloc64(വലുപ്പം(DbpageCursor));
  എങ്കിൽ( pCsr==0 ){
    തിരികെ SQLITE_NOMEM_BKPT;
  }അല്ലെങ്കിൽ{
    memset(pCsr, 0, വലുപ്പം(DbpageCursor));
    pCsr->base.pVtab = pVTab;
    pCsr->pgno = 0;
  }

  *ppCursor = (sqlite3_vtab_cursor *)pCsr;
  തിരികെ SQLITE_OK;
}

/*
** Close a dbpagevfs cursor.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ dbpageClose(sqlite3_vtab_cursor *pCursor){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  എങ്കിൽ( pCsr->pPage1 ) sqlite3PagerUnrefPageOne(pCsr->pPage1);
  sqlite3_free(pCsr);
  തിരികെ SQLITE_OK;
}

/*
** Move a dbpagevfs cursor to the next entry in the file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ dbpageNext(sqlite3_vtab_cursor *pCursor){
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  pCsr->pgno++;
  തിരികെ rc;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ dbpageEof(sqlite3_vtab_cursor *pCursor){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  തിരികെ pCsr->pgno > pCsr->mxPgno;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ dbpageFilter(
  sqlite3_vtab_cursor *pCursor,
  പൂർണ്ണസംഖ്യ idx_അക്കം, സ്ഥിരം അക്ഷരം *idxStr,
  പൂർണ്ണസംഖ്യ argc, sqlite3_മൂല്യം **argv
){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  DbpageTable *pTab = (DbpageTable *)pCursor->pVtab;
  പൂർണ്ണസംഖ്യ rc;
  sqlite3 *db = pTab->db;
  Btree *pBt;

  UNUSED_PARAMETER(idxStr);
  UNUSED_PARAMETER(argc);

  /* Default setting is no rows of result */
  pCsr->pgno = 1;
  pCsr->mxPgno = 0;

  എങ്കിൽ( idx_അക്കം & 2 ){
    സ്ഥിരം അക്ഷരം *zSchema;
    പ്രതിജ്ഞാ( argc>=1 );
    zSchema = (സ്ഥിരം അക്ഷരം*)sqlite3_മൂല്യം_text(argv[0]);
    pCsr->iDb = sqlite3FindDbName(db, zSchema);
    എങ്കിൽ( pCsr->iDb<0 ) തിരികെ SQLITE_OK;
  }അല്ലെങ്കിൽ{
    pCsr->iDb = 0;
  }
  pBt = db->aDb[pCsr->iDb].pBt;
  എങ്കിൽ( NEVER(pBt==0) ) തിരികെ SQLITE_OK;
  pCsr->pPager = sqlite3BtreePager(pBt);
  pCsr->szPage = sqlite3BtreeGetPageSize(pBt);
  pCsr->mxPgno = sqlite3BtreeLastPage(pBt);
  എങ്കിൽ( idx_അക്കം & 1 ){
    i64 iPg = sqlite3_മൂല്യം_int64(argv[idx_അക്കം>>1]);
    പ്രതിജ്ഞാ( argc>(idx_അക്കം>>1) );
    എങ്കിൽ( iPg<1 || iPg>pCsr->mxPgno ){
      pCsr->pgno = 1;
      pCsr->mxPgno = 0;
    }അല്ലെങ്കിൽ{
      pCsr->pgno = (Pgno)iPg;
      pCsr->mxPgno = pCsr->pgno;
    }
  }അല്ലെങ്കിൽ{
    പ്രതിജ്ഞാ( pCsr->pgno==1 );
  }
  എങ്കിൽ( pCsr->pPage1 ) sqlite3PagerUnrefPageOne(pCsr->pPage1);
  rc = sqlite3PagerGet(pCsr->pPager, 1, &pCsr->pPage1, 0);
  തിരികെ rc;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ dbpageColumn(
  sqlite3_vtab_cursor *pCursor,
  sqlite3_context *ctx,
  പൂർണ്ണസംഖ്യ i
){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;
  തിരഞ്ഞെടുപ്പ്( i ){
    അവസ്ഥ 0: {           /* pgno */
      sqlite3_result_int64(ctx, (sqlite3_int64)pCsr->pgno);
      നിർത്തുക;
    }
    അവസ്ഥ 1: {           /* data */
      DbPage *pDbPage = 0;
      എങ്കിൽ( pCsr->pgno==(Pgno)((PENDING_BYTE/pCsr->szPage)+1) ){
        /* The pending byte page. Assume it is zeroed out. Attempting to
        ** request this page from the page is an SQLITE_CORRUPT error. */
        sqlite3_result_zeroblob(ctx, pCsr->szPage);
      }അല്ലെങ്കിൽ{
        rc = sqlite3PagerGet(pCsr->pPager, pCsr->pgno, (DbPage**)&pDbPage, 0);
        എങ്കിൽ( rc==SQLITE_OK ){
          sqlite3_result_blob(ctx, sqlite3PagerGetData(pDbPage), pCsr->szPage,
              SQLITE_TRANSIENT);
        }
        sqlite3PagerUnref(pDbPage);
      }
      നിർത്തുക;
    }
    സ്ഥിരസ്ഥിതി: {          /* schema */
      sqlite3 *db = sqlite3_context_db_handle(ctx);
      sqlite3_result_text(ctx, db->aDb[pCsr->iDb].zDbSName, -1, SQLITE_STATIC);
      നിർത്തുക;
    }
  }
  തിരികെ rc;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ dbpageRowid(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  *pRowid = pCsr->pgno;
  തിരികെ SQLITE_OK;
}

/* 
** Open write transactions. Since we do not know in advance which database
** files will be written by the sqlite_dbpage virtual table, start a write
** transaction on them all.
**
** Return SQLITE_OK if successful, or an SQLite error code otherwise.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ dbpageBeginTrans(DbpageTable *pTab){
  sqlite3 *db = pTab->db;
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;
  പൂർണ്ണസംഖ്യ i;
  വേണ്ടി(i=0; rc==SQLITE_OK && i<db->nDb; i++){
    Btree *pBt = db->aDb[i].pBt;
    എങ്കിൽ( pBt ) rc = sqlite3BtreeBeginTrans(pBt, 1, 0);
  }
  തിരികെ rc;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ dbpageUpdate(
  sqlite3_vtab *pVtab,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv,
  sqlite_int64 *pRowid
){
  DbpageTable *pTab = (DbpageTable *)pVtab;
  Pgno pgno;
  sqlite3_int64 pgno64;
  DbPage *pDbPage = 0;
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;
  അക്ഷരം *zErr = 0;
  പൂർണ്ണസംഖ്യ iDb;
  Btree *pBt;
  Pager *pPager;
  പൂർണ്ണസംഖ്യ szPage;
  പൂർണ്ണസംഖ്യ is_തിരുകുക;

  (ശൂന്യം)pRowid;
  എങ്കിൽ( pTab->db->flags & SQLITE_Defensive ){
    zErr = "read-only";
    പോകുക update_fail;
  }
  എങ്കിൽ( argc==1 ){
    zErr = "cannot delete";
    പോകുക update_fail;
  }
  എങ്കിൽ( sqlite3_മൂല്യം_type(argv[0])==SQLITE_NULL ){
    pgno64 = sqlite3_മൂല്യം_int64(argv[2]);
    is_തിരുകുക = 1;
  }അല്ലെങ്കിൽ{
    pgno64 = (Pgno)sqlite3_മൂല്യം_int64(argv[0]);
    എങ്കിൽ( sqlite3_മൂല്യം_int64(argv[1])!=pgno64 ){
      zErr = "cannot insert";
      പോകുക update_fail;
    }
    is_തിരുകുക = 0;
  }
  എങ്കിൽ( sqlite3_മൂല്യം_type(argv[4])==SQLITE_NULL ){
    iDb = 0;
  }അല്ലെങ്കിൽ{
    സ്ഥിരം അക്ഷരം *zSchema = (സ്ഥിരം അക്ഷരം*)sqlite3_മൂല്യം_text(argv[4]);
    iDb = sqlite3FindDbName(pTab->db, zSchema);
    എങ്കിൽ( iDb<0 ){
      zErr = "no such schema";
      പോകുക update_fail;
    }
  }
  pBt = pTab->db->aDb[iDb].pBt;
  എങ്കിൽ( pgno64<1 || pgno64>4294967294U || NEVER(pBt==0) ){
    zErr = "bad page number";
    പോകുക update_fail;
  }
  pgno = (Pgno)pgno64;
  szPage = sqlite3BtreeGetPageSize(pBt);
  എങ്കിൽ( sqlite3_മൂല്യം_type(argv[3])!=SQLITE_BLOB 
   || sqlite3_മൂല്യം_bytes(argv[3])!=szPage
  ){
    എങ്കിൽ( sqlite3_മൂല്യം_type(argv[3])==SQLITE_NULL && is_തിരുകുക && pgno>1 ){
      /* "INSERT INTO dbpage($PGNO,NULL)" causes page number $PGNO and
      ** all subsequent pages to be deleted. */
      pTab->iDbTrunc = iDb;
      pTab->pgnoTrunc = pgno-1;
      pgno = 1;
    }അല്ലെങ്കിൽ{
      zErr = "bad page value";
      പോകുക update_fail;
    }
  }

  എങ്കിൽ( dbpageBeginTrans(pTab)!=SQLITE_OK ){
    zErr = "failed to open transaction";
    പോകുക update_fail;
  }

  pPager = sqlite3BtreePager(pBt);
  rc = sqlite3PagerGet(pPager, pgno, (DbPage**)&pDbPage, 0);
  എങ്കിൽ( rc==SQLITE_OK ){
    സ്ഥിരം ശൂന്യം *pData = sqlite3_മൂല്യം_blob(argv[3]);
    എങ്കിൽ( (rc = sqlite3PagerWrite(pDbPage))==SQLITE_OK && pData ){
      unsigned അക്ഷരം *aPage = sqlite3PagerGetData(pDbPage);
      memcpy(aPage, pData, szPage);
      pTab->pgnoTrunc = 0;
    }
  }
  എങ്കിൽ( rc!=SQLITE_OK ){
    pTab->pgnoTrunc = 0;
  }
  sqlite3PagerUnref(pDbPage);
  തിരികെ rc;

update_fail:
  pTab->pgnoTrunc = 0;
  sqlite3_free(pVtab->z_Err_സന്ദേശം);
  pVtab->z_Err_സന്ദേശം = sqlite3_mprintf("%s", zErr);
  തിരികെ SQLITE_ERROR;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ dbpageBegin(sqlite3_vtab *pVtab){
  DbpageTable *pTab = (DbpageTable *)pVtab;
  pTab->pgnoTrunc = 0;
  തിരികെ SQLITE_OK;
}

/* Invoke sqlite3PagerTruncate() as necessary, just prior to COMMIT
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ dbpageSync(sqlite3_vtab *pVtab){
  DbpageTable *pTab = (DbpageTable *)pVtab;
  എങ്കിൽ( pTab->pgnoTrunc>0 ){
    Btree *pBt = pTab->db->aDb[pTab->iDbTrunc].pBt;
    Pager *pPager = sqlite3BtreePager(pBt);
    sqlite3BtreeEnter(pBt);
    എങ്കിൽ( pTab->pgnoTrunc<sqlite3BtreeLastPage(pBt) ){
      sqlite3PagerTruncateImage(pPager, pTab->pgnoTrunc);
    }
    sqlite3BtreeLeave(pBt);
  }
  pTab->pgnoTrunc = 0;
  തിരികെ SQLITE_OK;
}

/* Cancel any pending truncate.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ dbpageRollbackTo(sqlite3_vtab *pVtab, പൂർണ്ണസംഖ്യ notUsed1){
  DbpageTable *pTab = (DbpageTable *)pVtab;
  pTab->pgnoTrunc = 0;
  (ശൂന്യം)notUsed1;
  തിരികെ SQLITE_OK;
}

/*
** Invoke this routine to register the "dbpage" virtual table module
*/
പൂർണ്ണസംഖ്യ sqlite3DbpageRegister(sqlite3 *db){
  സ്ഥിതം sqlite3_module dbpage_module = {
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
  തിരികെ sqlite3_create_module(db, "sqlite_dbpage", &dbpage_module, 0);
}
#അല്ലെങ്കിലെങ്കിൽ defined(SQLITE_ENABLE_DBPAGE_VTAB)
പൂർണ്ണസംഖ്യ sqlite3DbpageRegister(sqlite3 *db){ തിരികെ SQLITE_OK; }
#endif /* SQLITE_ENABLE_DBSTAT_VTAB */
