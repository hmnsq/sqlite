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

#समावेश "sqliteInt.h"   /* Requires access to internal data structures */
#अगर (defined(SQLITE_ENABLE_DBPAGE_VTAB) || defined(SQLITE_TEST)) \
    && !defined(SQLITE_OMIT_VIRTUALTABLE)

प्रकार काष्ठा DbpageTable DbpageTable;
प्रकार काष्ठा DbpageCursor DbpageCursor;

काष्ठा DbpageCursor {
  sqlite3_vtab_cursor base;       /* Base class.  Must be first */
  Pgno pgno;                      /* Current page number */
  Pgno mxPgno;                    /* Last page to visit on this scan */
  Pager *pPager;                  /* Pager being read/written */
  DbPage *pPage1;                 /* Page 1 of the database */
  पूर्णांक iDb;                        /* Index of database to analyze */
  पूर्णांक szPage;                     /* Size of each page in bytes */
};

काष्ठा DbpageTable {
  sqlite3_vtab base;              /* Base class.  Must be first */
  sqlite3 *db;                    /* The database */
  पूर्णांक iDbTrunc;                   /* Database to truncate */
  Pgno pgnoTrunc;                 /* Size to truncate to */
};

/* Columns */
#घोषणा DBPAGE_COLUMN_PGNO    0
#घोषणा DBPAGE_COLUMN_तथ्य    1
#घोषणा DBPAGE_COLUMN_SCHEMA  2


/*
** Connect to or create a dbpagevfs virtual table.
*/
अटल पूर्णांक dbpageConnect(
  sqlite3 *db,
  व्योम *pAux,
  पूर्णांक argc, स्थिर अक्षर *स्थिर*argv,
  sqlite3_vtab **ppVtab,
  अक्षर **pzErr
){
  DbpageTable *pTab = 0;
  पूर्णांक rc = SQLITE_OK;
  (व्योम)pAux;
  (व्योम)argc;
  (व्योम)argv;
  (व्योम)pzErr;

  sqlite3_vtab_config(db, SQLITE_VTAB_DIRECTONLY);
  sqlite3_vtab_config(db, SQLITE_VTAB_USES_ALL_SCHEMAS);
  rc = sqlite3_declare_vtab(db, 
          "CREATE TABLE x(pgno INTEGER PRIMARY KEY, data BLOB, schema HIDDEN)");
  अगर( rc==SQLITE_OK ){
    pTab = (DbpageTable *)sqlite3_malloc64(माप(DbpageTable));
    अगर( pTab==0 ) rc = SQLITE_NOMEM_BKPT;
  }

  निश्चित( rc==SQLITE_OK || pTab==0 );
  अगर( rc==SQLITE_OK ){
    स_रखो(pTab, 0, माप(DbpageTable));
    pTab->db = db;
  }

  *ppVtab = (sqlite3_vtab*)pTab;
  वापस rc;
}

/*
** Disconnect from or destroy a dbpagevfs virtual table.
*/
अटल पूर्णांक dbpageDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  वापस SQLITE_OK;
}

/*
** idxNum:
**
**     0     schema=main, full table scan
**     1     schema=main, pgno=?1
**     2     schema=?1, full table scan
**     3     schema=?1, pgno=?2
*/
अटल पूर्णांक dbpage_Best_सूचकांक(sqlite3_vtab *tab, sqlite3_सूचकांक_info *pIdxInfo){
  पूर्णांक i;
  पूर्णांक iPlan = 0;
  (व्योम)tab;

  /* If there is a schema= constraint, it must be honored.  Report a
  ** ridiculously large estimated cost if the schema= constraint is
  ** unavailable
  */
  क्रम(i=0; i<pIdxInfo->nConstraint; i++){
    काष्ठा sqlite3_सूचकांक_constraint *p = &pIdxInfo->aConstraint[i];
    अगर( p->iColumn!=DBPAGE_COLUMN_SCHEMA ) जारी;
    अगर( p->op!=SQLITE_सूचकांक_CONSTRAINT_EQ ) जारी;
    अगर( !p->usable ){
      /* No solution. */
      वापस SQLITE_CONSTRAINT;
    }
    iPlan = 2;
    pIdxInfo->aConstraintUsage[i].argv_सूचकांक = 1;
    pIdxInfo->aConstraintUsage[i].omit = 1;
    अवरोध;
  }

  /* If we reach this point, it means that either there is no schema=
  ** constraint (in which case we use the "main" schema) or else the
  ** schema constraint was accepted.  Lower the estimated cost accordingly
  */
  pIdxInfo->estimatedCost = 1.0e6;

  /* Check for constraints against pgno */
  क्रम(i=0; i<pIdxInfo->nConstraint; i++){
    काष्ठा sqlite3_सूचकांक_constraint *p = &pIdxInfo->aConstraint[i];
    अगर( p->usable && p->iColumn<=0 && p->op==SQLITE_सूचकांक_CONSTRAINT_EQ ){
      pIdxInfo->estimatedRows = 1;
      pIdxInfo->idxFlags = SQLITE_सूचकांक_SCAN_UNIQUE;
      pIdxInfo->estimatedCost = 1.0;
      pIdxInfo->aConstraintUsage[i].argv_सूचकांक = iPlan ? 2 : 1;
      pIdxInfo->aConstraintUsage[i].omit = 1;
      iPlan |= 1;
      अवरोध;
    }
  }
  pIdxInfo->idx_अंक = iPlan;

  अगर( pIdxInfo->nOrderBy>=1
   && pIdxInfo->aOrderBy[0].iColumn<=0
   && pIdxInfo->aOrderBy[0].desc==0
  ){
    pIdxInfo->orderByConsumed = 1;
  }
  वापस SQLITE_OK;
}

/*
** Open a new dbpagevfs cursor.
*/
अटल पूर्णांक dbpageOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  DbpageCursor *pCsr;

  pCsr = (DbpageCursor *)sqlite3_malloc64(माप(DbpageCursor));
  अगर( pCsr==0 ){
    वापस SQLITE_NOMEM_BKPT;
  }अन्यथा{
    स_रखो(pCsr, 0, माप(DbpageCursor));
    pCsr->base.pVtab = pVTab;
    pCsr->pgno = 0;
  }

  *ppCursor = (sqlite3_vtab_cursor *)pCsr;
  वापस SQLITE_OK;
}

/*
** Close a dbpagevfs cursor.
*/
अटल पूर्णांक dbpageClose(sqlite3_vtab_cursor *pCursor){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  अगर( pCsr->pPage1 ) sqlite3PagerUnrefPageOne(pCsr->pPage1);
  sqlite3_free(pCsr);
  वापस SQLITE_OK;
}

/*
** Move a dbpagevfs cursor to the next entry in the file.
*/
अटल पूर्णांक dbpage_अग्र(sqlite3_vtab_cursor *pCursor){
  पूर्णांक rc = SQLITE_OK;
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  pCsr->pgno++;
  वापस rc;
}

अटल पूर्णांक dbpageEof(sqlite3_vtab_cursor *pCursor){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  वापस pCsr->pgno > pCsr->mxPgno;
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
अटल पूर्णांक dbpageFilter(
  sqlite3_vtab_cursor *pCursor,
  पूर्णांक idx_अंक, स्थिर अक्षर *idxStr,
  पूर्णांक argc, sqlite3_मूल्य **argv
){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  DbpageTable *pTab = (DbpageTable *)pCursor->pVtab;
  पूर्णांक rc;
  sqlite3 *db = pTab->db;
  Btree *pBt;

  UNUSED_PARAMETER(idxStr);
  UNUSED_PARAMETER(argc);

  /* Default setting is no rows of result */
  pCsr->pgno = 1;
  pCsr->mxPgno = 0;

  अगर( idx_अंक & 2 ){
    स्थिर अक्षर *zSchema;
    निश्चित( argc>=1 );
    zSchema = (स्थिर अक्षर*)sqlite3_मूल्य_text(argv[0]);
    pCsr->iDb = sqlite3FindDbName(db, zSchema);
    अगर( pCsr->iDb<0 ) वापस SQLITE_OK;
  }अन्यथा{
    pCsr->iDb = 0;
  }
  pBt = db->aDb[pCsr->iDb].pBt;
  अगर( NEVER(pBt==0) ) वापस SQLITE_OK;
  pCsr->pPager = sqlite3BtreePager(pBt);
  pCsr->szPage = sqlite3_Btree_Get_Page_आकार(pBt);
  pCsr->mxPgno = sqlite3BtreeLastPage(pBt);
  अगर( idx_अंक & 1 ){
    i64 iPg = sqlite3_मूल्य_int64(argv[idx_अंक>>1]);
    निश्चित( argc>(idx_अंक>>1) );
    अगर( iPg<1 || iPg>pCsr->mxPgno ){
      pCsr->pgno = 1;
      pCsr->mxPgno = 0;
    }अन्यथा{
      pCsr->pgno = (Pgno)iPg;
      pCsr->mxPgno = pCsr->pgno;
    }
  }अन्यथा{
    निश्चित( pCsr->pgno==1 );
  }
  अगर( pCsr->pPage1 ) sqlite3PagerUnrefPageOne(pCsr->pPage1);
  rc = sqlite3PagerGet(pCsr->pPager, 1, &pCsr->pPage1, 0);
  वापस rc;
}

अटल पूर्णांक dbpageColumn(
  sqlite3_vtab_cursor *pCursor,
  sqlite3_context *ctx,
  पूर्णांक i
){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  पूर्णांक rc = SQLITE_OK;
  चयन( i ){
    हाल 0: {           /* pgno */
      sqlite3_परिणाम_int64(ctx, (sqlite3_int64)pCsr->pgno);
      अवरोध;
    }
    हाल 1: {           /*तथ्य*/
      DbPage *pDbPage = 0;
      अगर( pCsr->pgno==(Pgno)((PENDING_BYTE/pCsr->szPage)+1) ){
        /* The pending byte page. Assume it is zeroed out. Attempting to
        ** request this page from the page is an SQLITE_CORRUPT error. */
        sqlite3_परिणाम_zeroblob(ctx, pCsr->szPage);
      }अन्यथा{
        rc = sqlite3PagerGet(pCsr->pPager, pCsr->pgno, (DbPage**)&pDbPage, 0);
        अगर( rc==SQLITE_OK ){
          sqlite3_परिणाम_blob(ctx, sqlite3_Pager_Get_तथ्य(pDbPage), pCsr->szPage,
              SQLITE_TRANSIENT);
        }
        sqlite3PagerUnref(pDbPage);
      }
      अवरोध;
    }
    शेष: {          /* schema */
      sqlite3 *db = sqlite3_context_db_handle(ctx);
      sqlite3_परिणाम_text(ctx, db->aDb[pCsr->iDb].zDbSName, -1, SQLITE_STATIC);
      अवरोध;
    }
  }
  वापस rc;
}

अटल पूर्णांक dbpageRowid(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  *pRowid = pCsr->pgno;
  वापस SQLITE_OK;
}

/* 
** Open write transactions. Since we do not know in advance which database
** files will be written by the sqlite_dbpage virtual table, start a write
** transaction on them all.
**
** Return SQLITE_OK if successful, or an SQLite error code otherwise.
*/
अटल पूर्णांक dbpageBeginTrans(DbpageTable *pTab){
  sqlite3 *db = pTab->db;
  पूर्णांक rc = SQLITE_OK;
  पूर्णांक i;
  क्रम(i=0; rc==SQLITE_OK && i<db->nDb; i++){
    Btree *pBt = db->aDb[i].pBt;
    अगर( pBt ) rc = sqlite3BtreeBeginTrans(pBt, 1, 0);
  }
  वापस rc;
}

अटल पूर्णांक dbpageUpdate(
  sqlite3_vtab *pVtab,
  पूर्णांक argc,
  sqlite3_मूल्य **argv,
  sqlite_int64 *pRowid
){
  DbpageTable *pTab = (DbpageTable *)pVtab;
  Pgno pgno;
  sqlite3_int64 pgno64;
  DbPage *pDbPage = 0;
  पूर्णांक rc = SQLITE_OK;
  अक्षर *zErr = 0;
  पूर्णांक iDb;
  Btree *pBt;
  Pager *pPager;
  पूर्णांक szPage;
  पूर्णांक is_जोड़ो;

  (व्योम)pRowid;
  अगर( pTab->db->flags & SQLITE_Defensive ){
    zErr = "read-only";
    जाओ update_fail;
  }
  अगर( argc==1 ){
    zErr = "cannot delete";
    जाओ update_fail;
  }
  अगर( sqlite3_मूल्य_type(argv[0])==SQLITE_NULL ){
    pgno64 = sqlite3_मूल्य_int64(argv[2]);
    is_जोड़ो = 1;
  }अन्यथा{
    pgno64 = (Pgno)sqlite3_मूल्य_int64(argv[0]);
    अगर( sqlite3_मूल्य_int64(argv[1])!=pgno64 ){
      zErr = "cannot insert";
      जाओ update_fail;
    }
    is_जोड़ो = 0;
  }
  अगर( sqlite3_मूल्य_type(argv[4])==SQLITE_NULL ){
    iDb = 0;
  }अन्यथा{
    स्थिर अक्षर *zSchema = (स्थिर अक्षर*)sqlite3_मूल्य_text(argv[4]);
    iDb = sqlite3FindDbName(pTab->db, zSchema);
    अगर( iDb<0 ){
      zErr = "no such schema";
      जाओ update_fail;
    }
  }
  pBt = pTab->db->aDb[iDb].pBt;
  अगर( pgno64<1 || pgno64>4294967294U || NEVER(pBt==0) ){
    zErr = "bad page number";
    जाओ update_fail;
  }
  pgno = (Pgno)pgno64;
  szPage = sqlite3_Btree_Get_Page_आकार(pBt);
  अगर( sqlite3_मूल्य_type(argv[3])!=SQLITE_BLOB 
   || sqlite3_मूल्य_bytes(argv[3])!=szPage
  ){
    अगर( sqlite3_मूल्य_type(argv[3])==SQLITE_NULL && is_जोड़ो && pgno>1 ){
      /* "INSERT INTO dbpage($PGNO,NULL)" causes page number $PGNO and
      ** all subsequent pages to be deleted. */
      pTab->iDbTrunc = iDb;
      pTab->pgnoTrunc = pgno-1;
      pgno = 1;
    }अन्यथा{
      zErr = "bad page value";
      जाओ update_fail;
    }
  }

  अगर( dbpageBeginTrans(pTab)!=SQLITE_OK ){
    zErr = "failed to open transaction";
    जाओ update_fail;
  }

  pPager = sqlite3BtreePager(pBt);
  rc = sqlite3PagerGet(pPager, pgno, (DbPage**)&pDbPage, 0);
  अगर( rc==SQLITE_OK ){
    स्थिर व्योम *p_तथ्य = sqlite3_मूल्य_blob(argv[3]);
    अगर( (rc = sqlite3PagerWrite(pDbPage))==SQLITE_OK && p_तथ्य ){
      अचिन्हित अक्षर *aPage = sqlite3_Pager_Get_तथ्य(pDbPage);
      स_नकल(aPage, p_तथ्य, szPage);
      pTab->pgnoTrunc = 0;
    }
  }
  अगर( rc!=SQLITE_OK ){
    pTab->pgnoTrunc = 0;
  }
  sqlite3PagerUnref(pDbPage);
  वापस rc;

update_fail:
  pTab->pgnoTrunc = 0;
  sqlite3_free(pVtab->z_Err_संदेश);
  pVtab->z_Err_संदेश = sqlite3_mprintf("%s", zErr);
  वापस SQLITE_ERROR;
}

अटल पूर्णांक dbpageBegin(sqlite3_vtab *pVtab){
  DbpageTable *pTab = (DbpageTable *)pVtab;
  pTab->pgnoTrunc = 0;
  वापस SQLITE_OK;
}

/* Invoke sqlite3PagerTruncate() as necessary, just prior to COMMIT
*/
अटल पूर्णांक dbpageSync(sqlite3_vtab *pVtab){
  DbpageTable *pTab = (DbpageTable *)pVtab;
  अगर( pTab->pgnoTrunc>0 ){
    Btree *pBt = pTab->db->aDb[pTab->iDbTrunc].pBt;
    Pager *pPager = sqlite3BtreePager(pBt);
    sqlite3BtreeEnter(pBt);
    अगर( pTab->pgnoTrunc<sqlite3BtreeLastPage(pBt) ){
      sqlite3PagerTruncateImage(pPager, pTab->pgnoTrunc);
    }
    sqlite3BtreeLeave(pBt);
  }
  pTab->pgnoTrunc = 0;
  वापस SQLITE_OK;
}

/* Cancel any pending truncate.
*/
अटल पूर्णांक dbpageRollbackTo(sqlite3_vtab *pVtab, पूर्णांक notUsed1){
  DbpageTable *pTab = (DbpageTable *)pVtab;
  pTab->pgnoTrunc = 0;
  (व्योम)notUsed1;
  वापस SQLITE_OK;
}

/*
** Invoke this routine to register the "dbpage" virtual table module
*/
पूर्णांक sqlite3DbpageRegister(sqlite3 *db){
  अटल sqlite3_module dbpage_module = {
    2,                            /* iVersion */
    dbpageConnect,                /* xCreate */
    dbpageConnect,                /* xConnect */
    dbpage_Best_सूचकांक,              /* xBestIndex */
    dbpageDisconnect,             /* xDisconnect */
    dbpageDisconnect,             /* xDestroy */
    dbpageOpen,                   /* xOpen - open a cursor */
    dbpageClose,                  /* xClose - close a cursor */
    dbpageFilter,                 /* xFilter - configure scan constraints */
    dbpage_अग्र,                   /* xNext - advance a cursor */
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
  वापस sqlite3_create_module(db, "sqlite_dbpage", &dbpage_module, 0);
}
#या_अगर defined(SQLITE_ENABLE_DBPAGE_VTAB)
पूर्णांक sqlite3DbpageRegister(sqlite3 *db){ वापस SQLITE_OK; }
#पूर्ण_अगर /* SQLITE_ENABLE_DBSTAT_VTAB */
