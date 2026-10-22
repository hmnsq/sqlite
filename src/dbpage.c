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

#అన్తర్భావ "sqliteInt.h"   /* Requires access to internal data structures */
#ఒకవేళ (defined(SQLITE_ENABLE_DBPAGE_VTAB) || defined(SQLITE_TEST)) \
    && !defined(SQLITE_OMIT_VIRTUALTABLE)

రకనిర్వచనం నిర్మాణం Dbpage_పట్టిక Dbpage_పట్టిక;
రకనిర్వచనం నిర్మాణం DbpageCursor DbpageCursor;

నిర్మాణం DbpageCursor {
  sqlite3_vtab_cursor పునాది;       /* Base class.  Must be first */
  Pgno pgno;                      /* Current page number */
  Pgno mxPgno;                    /* Last page to visit on this scan */
  Pager *pPager;                  /* Pager being read/written */
  Db_పుట *pPage1;                 /* Page 1 of the database */
  పూర్ణాంకం iDb;                        /* Index of database to analyze */
  పూర్ణాంకం sz_పుట;                     /* Size of each page in bytes */
};

నిర్మాణం Dbpage_పట్టిక {
  sqlite3_vtab పునాది;              /* Base class.  Must be first */
  sqlite3 *db;                    /* The database */
  పూర్ణాంకం iDbTrunc;                   /* Database to truncate */
  Pgno pgnoTrunc;                 /* Size to truncate to */
};

/* Columns */
#నిర్వచించు DBPAGE_స్తంభం_PGNO    0
#నిర్వచించు DBPAGE_స్తంభం_దత్తాంశం    1
#నిర్వచించు DBPAGE_స్తంభం_SCHEMA  2


/*
** Connect to or create a dbpagevfs virtual table.
*/
స్థిర పూర్ణాంకం dbpageConnect(
  sqlite3 *db,
  శూన్య *pAux,
  పూర్ణాంకం argc, స్థిరరాశి అక్షరం *స్థిరరాశి*argv,
  sqlite3_vtab **ppVtab,
  అక్షరం **pzErr
){
  Dbpage_పట్టిక *pTab = 0;
  పూర్ణాంకం rc = SQLITE_OK;
  (శూన్య)pAux;
  (శూన్య)argc;
  (శూన్య)argv;
  (శూన్య)pzErr;

  sqlite3_vtab_config(db, SQLITE_VTAB_DIRECTONLY);
  sqlite3_vtab_config(db, SQLITE_VTAB_ఉపయోగిస్తుంది_ALL_SCHEMAS);
  rc = sqlite3_declare_vtab(db, 
          "CREATE TABLE x(pgno INTEGER PRIMARY KEY, data BLOB, schema HIDDEN)");
  ఒకవేళ( rc==SQLITE_OK ){
    pTab = (Dbpage_పట్టిక *)sqlite3_malloc64(పరిమాణం(Dbpage_పట్టిక));
    ఒకవేళ( pTab==0 ) rc = SQLITE_NOMEM_BKPT;
  }

  ప్రతిజ్ఞా( rc==SQLITE_OK || pTab==0 );
  ఒకవేళ( rc==SQLITE_OK ){
    memset(pTab, 0, పరిమాణం(Dbpage_పట్టిక));
    pTab->db = db;
  }

  *ppVtab = (sqlite3_vtab*)pTab;
  తిరిగి rc;
}

/*
** Disconnect from or destroy a dbpagevfs virtual table.
*/
స్థిర పూర్ణాంకం dbpageDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  తిరిగి SQLITE_OK;
}

/*
** idxNum:
**
**     0     schema=main, full table scan
**     1     schema=main, pgno=?1
**     2     schema=?1, full table scan
**     3     schema=?1, pgno=?2
*/
స్థిర పూర్ణాంకం dbpage_ఉత్తమ_సూచీ(sqlite3_vtab *tab, sqlite3_సూచీ_info *pIdxInfo){
  పూర్ణాంకం i;
  పూర్ణాంకం iPlan = 0;
  (శూన్య)tab;

  /* If there is a schema= constraint, it must be honored.  Report a
  ** ridiculously large estimated cost if the schema= constraint is
  ** unavailable
  */
  కొరకు(i=0; i<pIdxInfo->nConstraint; i++){
    నిర్మాణం sqlite3_సూచీ_constraint *p = &pIdxInfo->aConstraint[i];
    ఒకవేళ( p->i_స్తంభం!=DBPAGE_స్తంభం_SCHEMA ) కొనసాగు;
    ఒకవేళ( p->op!=SQLITE_సూచీ_CONSTRAINT_EQ ) కొనసాగు;
    ఒకవేళ( !p->usable ){
      /* No solution. */
      తిరిగి SQLITE_CONSTRAINT;
    }
    iPlan = 2;
    pIdxInfo->aConstraintUsage[i].argv_సూచీ = 1;
    pIdxInfo->aConstraintUsage[i].omit = 1;
    ఆపు;
  }

  /* If we reach this point, it means that either there is no schema=
  ** constraint (in which case we use the "main" schema) or else the
  ** schema constraint was accepted.  Lower the estimated cost accordingly
  */
  pIdxInfo->estimated_ఖర్చు = 1.0e6;

  /* Check for constraints against pgno */
  కొరకు(i=0; i<pIdxInfo->nConstraint; i++){
    నిర్మాణం sqlite3_సూచీ_constraint *p = &pIdxInfo->aConstraint[i];
    ఒకవేళ( p->usable && p->i_స్తంభం<=0 && p->op==SQLITE_సూచీ_CONSTRAINT_EQ ){
      pIdxInfo->estimated_అడ్డువరుసలు = 1;
      pIdxInfo->idxFlags = SQLITE_సూచీ_పరిశీలన_UNIQUE;
      pIdxInfo->estimated_ఖర్చు = 1.0;
      pIdxInfo->aConstraintUsage[i].argv_సూచీ = iPlan ? 2 : 1;
      pIdxInfo->aConstraintUsage[i].omit = 1;
      iPlan |= 1;
      ఆపు;
    }
  }
  pIdxInfo->idx_అంకె = iPlan;

  ఒకవేళ( pIdxInfo->n_క్రమం_By>=1
   && pIdxInfo->a_క్రమం_By[0].i_స్తంభం<=0
   && pIdxInfo->a_క్రమం_By[0].desc==0
  ){
    pIdxInfo->క్రమం_By_Consumed = 1;
  }
  తిరిగి SQLITE_OK;
}

/*
** Open a new dbpagevfs cursor.
*/
స్థిర పూర్ణాంకం dbpageOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  DbpageCursor *pCsr;

  pCsr = (DbpageCursor *)sqlite3_malloc64(పరిమాణం(DbpageCursor));
  ఒకవేళ( pCsr==0 ){
    తిరిగి SQLITE_NOMEM_BKPT;
  }లేకపోతే{
    memset(pCsr, 0, పరిమాణం(DbpageCursor));
    pCsr->పునాది.pVtab = pVTab;
    pCsr->pgno = 0;
  }

  *ppCursor = (sqlite3_vtab_cursor *)pCsr;
  తిరిగి SQLITE_OK;
}

/*
** Close a dbpagevfs cursor.
*/
స్థిర పూర్ణాంకం dbpageClose(sqlite3_vtab_cursor *pCursor){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  ఒకవేళ( pCsr->pPage1 ) sqlite3_Pager_Unref_పుట_One(pCsr->pPage1);
  sqlite3_free(pCsr);
  తిరిగి SQLITE_OK;
}

/*
** Move a dbpagevfs cursor to the next entry in the file.
*/
స్థిర పూర్ణాంకం dbpage_తదుపరి(sqlite3_vtab_cursor *pCursor){
  పూర్ణాంకం rc = SQLITE_OK;
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  pCsr->pgno++;
  తిరిగి rc;
}

స్థిర పూర్ణాంకం dbpageEof(sqlite3_vtab_cursor *pCursor){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  తిరిగి pCsr->pgno > pCsr->mxPgno;
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
స్థిర పూర్ణాంకం dbpageFilter(
  sqlite3_vtab_cursor *pCursor,
  పూర్ణాంకం idx_అంకె, స్థిరరాశి అక్షరం *idxStr,
  పూర్ణాంకం argc, sqlite3_విలువ **argv
){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  Dbpage_పట్టిక *pTab = (Dbpage_పట్టిక *)pCursor->pVtab;
  పూర్ణాంకం rc;
  sqlite3 *db = pTab->db;
  Btree *pBt;

  వాడని_PARAMETER(idxStr);
  వాడని_PARAMETER(argc);

  /* Default setting is no rows of result */
  pCsr->pgno = 1;
  pCsr->mxPgno = 0;

  ఒకవేళ( idx_అంకె & 2 ){
    స్థిరరాశి అక్షరం *zSchema;
    ప్రతిజ్ఞా( argc>=1 );
    zSchema = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[0]);
    pCsr->iDb = sqlite3_కనుగొను_Db_Name(db, zSchema);
    ఒకవేళ( pCsr->iDb<0 ) తిరిగి SQLITE_OK;
  }లేకపోతే{
    pCsr->iDb = 0;
  }
  pBt = db->aDb[pCsr->iDb].pBt;
  ఒకవేళ( NEVER(pBt==0) ) తిరిగి SQLITE_OK;
  pCsr->pPager = sqlite3BtreePager(pBt);
  pCsr->sz_పుట = sqlite3_Btree_Get_పుట_కొలత(pBt);
  pCsr->mxPgno = sqlite3_Btree_చివరి_పుట(pBt);
  ఒకవేళ( idx_అంకె & 1 ){
    i64 iPg = sqlite3_విలువ_int64(argv[idx_అంకె>>1]);
    ప్రతిజ్ఞా( argc>(idx_అంకె>>1) );
    ఒకవేళ( iPg<1 || iPg>pCsr->mxPgno ){
      pCsr->pgno = 1;
      pCsr->mxPgno = 0;
    }లేకపోతే{
      pCsr->pgno = (Pgno)iPg;
      pCsr->mxPgno = pCsr->pgno;
    }
  }లేకపోతే{
    ప్రతిజ్ఞా( pCsr->pgno==1 );
  }
  ఒకవేళ( pCsr->pPage1 ) sqlite3_Pager_Unref_పుట_One(pCsr->pPage1);
  rc = sqlite3PagerGet(pCsr->pPager, 1, &pCsr->pPage1, 0);
  తిరిగి rc;
}

స్థిర పూర్ణాంకం dbpage_స్తంభం(
  sqlite3_vtab_cursor *pCursor,
  sqlite3_context *ctx,
  పూర్ణాంకం i
){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  పూర్ణాంకం rc = SQLITE_OK;
  ఎంపిక( i ){
    స్థితి 0: {           /* pgno */
      sqlite3_ఫలితం_int64(ctx, (sqlite3_int64)pCsr->pgno);
      ఆపు;
    }
    స్థితి 1: {           /*దత్తాంశం*/
      Db_పుట *p_Db_పుట = 0;
      ఒకవేళ( pCsr->pgno==(Pgno)((PENDING_BYTE/pCsr->sz_పుట)+1) ){
        /* The pending byte page. Assume it is zeroed out. Attempting to
        ** request this page from the page is an SQLITE_CORRUPT error. */
        sqlite3_ఫలితం_zeroblob(ctx, pCsr->sz_పుట);
      }లేకపోతే{
        rc = sqlite3PagerGet(pCsr->pPager, pCsr->pgno, (Db_పుట**)&p_Db_పుట, 0);
        ఒకవేళ( rc==SQLITE_OK ){
          sqlite3_ఫలితం_blob(ctx, sqlite3_Pager_Get_దత్తాంశం(p_Db_పుట), pCsr->sz_పుట,
              SQLITE_TRANSIENT);
        }
        sqlite3PagerUnref(p_Db_పుట);
      }
      ఆపు;
    }
    అప్రమేయ: {          /* schema */
      sqlite3 *db = sqlite3_context_db_handle(ctx);
      sqlite3_ఫలితం_పాఠ్యం(ctx, db->aDb[pCsr->iDb].zDbSName, -1, SQLITE_STATIC);
      ఆపు;
    }
  }
  తిరిగి rc;
}

స్థిర పూర్ణాంకం dbpageRowid(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  *pRowid = pCsr->pgno;
  తిరిగి SQLITE_OK;
}

/* 
** Open write transactions. Since we do not know in advance which database
** files will be written by the sqlite_dbpage virtual table, start a write
** transaction on them all.
**
** Return SQLITE_OK if successful, or an SQLite error code otherwise.
*/
స్థిర పూర్ణాంకం dbpageBeginTrans(Dbpage_పట్టిక *pTab){
  sqlite3 *db = pTab->db;
  పూర్ణాంకం rc = SQLITE_OK;
  పూర్ణాంకం i;
  కొరకు(i=0; rc==SQLITE_OK && i<db->nDb; i++){
    Btree *pBt = db->aDb[i].pBt;
    ఒకవేళ( pBt ) rc = sqlite3BtreeBeginTrans(pBt, 1, 0);
  }
  తిరిగి rc;
}

స్థిర పూర్ణాంకం dbpageUpdate(
  sqlite3_vtab *pVtab,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv,
  sqlite_int64 *pRowid
){
  Dbpage_పట్టిక *pTab = (Dbpage_పట్టిక *)pVtab;
  Pgno pgno;
  sqlite3_int64 pgno64;
  Db_పుట *p_Db_పుట = 0;
  పూర్ణాంకం rc = SQLITE_OK;
  అక్షరం *zErr = 0;
  పూర్ణాంకం iDb;
  Btree *pBt;
  Pager *pPager;
  పూర్ణాంకం sz_పుట;
  పూర్ణాంకం is_చొప్పించు;

  (శూన్య)pRowid;
  ఒకవేళ( pTab->db->flags & SQLITE_Defensive ){
    zErr = "read-only";
    వెళ్ళు update_fail;
  }
  ఒకవేళ( argc==1 ){
    zErr = "cannot delete";
    వెళ్ళు update_fail;
  }
  ఒకవేళ( sqlite3_విలువ_type(argv[0])==SQLITE_NULL ){
    pgno64 = sqlite3_విలువ_int64(argv[2]);
    is_చొప్పించు = 1;
  }లేకపోతే{
    pgno64 = (Pgno)sqlite3_విలువ_int64(argv[0]);
    ఒకవేళ( sqlite3_విలువ_int64(argv[1])!=pgno64 ){
      zErr = "cannot insert";
      వెళ్ళు update_fail;
    }
    is_చొప్పించు = 0;
  }
  ఒకవేళ( sqlite3_విలువ_type(argv[4])==SQLITE_NULL ){
    iDb = 0;
  }లేకపోతే{
    స్థిరరాశి అక్షరం *zSchema = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[4]);
    iDb = sqlite3_కనుగొను_Db_Name(pTab->db, zSchema);
    ఒకవేళ( iDb<0 ){
      zErr = "no such schema";
      వెళ్ళు update_fail;
    }
  }
  pBt = pTab->db->aDb[iDb].pBt;
  ఒకవేళ( pgno64<1 || pgno64>4294967294U || NEVER(pBt==0) ){
    zErr = "bad page number";
    వెళ్ళు update_fail;
  }
  pgno = (Pgno)pgno64;
  sz_పుట = sqlite3_Btree_Get_పుట_కొలత(pBt);
  ఒకవేళ( sqlite3_విలువ_type(argv[3])!=SQLITE_BLOB 
   || sqlite3_విలువ_బైట్లు(argv[3])!=sz_పుట
  ){
    ఒకవేళ( sqlite3_విలువ_type(argv[3])==SQLITE_NULL && is_చొప్పించు && pgno>1 ){
      /* "INSERT INTO dbpage($PGNO,NULL)" causes page number $PGNO and
      ** all subsequent pages to be deleted. */
      pTab->iDbTrunc = iDb;
      pTab->pgnoTrunc = pgno-1;
      pgno = 1;
    }లేకపోతే{
      zErr = "bad page value";
      వెళ్ళు update_fail;
    }
  }

  ఒకవేళ( dbpageBeginTrans(pTab)!=SQLITE_OK ){
    zErr = "failed to open transaction";
    వెళ్ళు update_fail;
  }

  pPager = sqlite3BtreePager(pBt);
  rc = sqlite3PagerGet(pPager, pgno, (Db_పుట**)&p_Db_పుట, 0);
  ఒకవేళ( rc==SQLITE_OK ){
    స్థిరరాశి శూన్య *p_దత్తాంశం = sqlite3_విలువ_blob(argv[3]);
    ఒకవేళ( (rc = sqlite3PagerWrite(p_Db_పుట))==SQLITE_OK && p_దత్తాంశం ){
      గుర్తులేని అక్షరం *a_పుట = sqlite3_Pager_Get_దత్తాంశం(p_Db_పుట);
      జ్ఞాపకం_నకలు(a_పుట, p_దత్తాంశం, sz_పుట);
      pTab->pgnoTrunc = 0;
    }
  }
  ఒకవేళ( rc!=SQLITE_OK ){
    pTab->pgnoTrunc = 0;
  }
  sqlite3PagerUnref(p_Db_పుట);
  తిరిగి rc;

update_fail:
  pTab->pgnoTrunc = 0;
  sqlite3_free(pVtab->z_Err_సందేశం);
  pVtab->z_Err_సందేశం = sqlite3_mprintf("%s", zErr);
  తిరిగి SQLITE_ERROR;
}

స్థిర పూర్ణాంకం dbpageBegin(sqlite3_vtab *pVtab){
  Dbpage_పట్టిక *pTab = (Dbpage_పట్టిక *)pVtab;
  pTab->pgnoTrunc = 0;
  తిరిగి SQLITE_OK;
}

/* Invoke sqlite3PagerTruncate() as necessary, just prior to COMMIT
*/
స్థిర పూర్ణాంకం dbpageSync(sqlite3_vtab *pVtab){
  Dbpage_పట్టిక *pTab = (Dbpage_పట్టిక *)pVtab;
  ఒకవేళ( pTab->pgnoTrunc>0 ){
    Btree *pBt = pTab->db->aDb[pTab->iDbTrunc].pBt;
    Pager *pPager = sqlite3BtreePager(pBt);
    sqlite3BtreeEnter(pBt);
    ఒకవేళ( pTab->pgnoTrunc<sqlite3_Btree_చివరి_పుట(pBt) ){
      sqlite3PagerTruncateImage(pPager, pTab->pgnoTrunc);
    }
    sqlite3BtreeLeave(pBt);
  }
  pTab->pgnoTrunc = 0;
  తిరిగి SQLITE_OK;
}

/* Cancel any pending truncate.
*/
స్థిర పూర్ణాంకం dbpage_Rollback_గమ్యం(sqlite3_vtab *pVtab, పూర్ణాంకం notUsed1){
  Dbpage_పట్టిక *pTab = (Dbpage_పట్టిక *)pVtab;
  pTab->pgnoTrunc = 0;
  (శూన్య)notUsed1;
  తిరిగి SQLITE_OK;
}

/*
** Invoke this routine to register the "dbpage" virtual table module
*/
పూర్ణాంకం sqlite3_Dbpage_రిజిస్టరు(sqlite3 *db){
  స్థిర sqlite3_module dbpage_module = {
    2,                            /* iVersion */
    dbpageConnect,                /* xCreate */
    dbpageConnect,                /* xConnect */
    dbpage_ఉత్తమ_సూచీ,              /* xBestIndex */
    dbpageDisconnect,             /* xDisconnect */
    dbpageDisconnect,             /* xDestroy */
    dbpageOpen,                   /* xOpen - open a cursor */
    dbpageClose,                  /* xClose - close a cursor */
    dbpageFilter,                 /* xFilter - configure scan constraints */
    dbpage_తదుపరి,                   /* xNext - advance a cursor */
    dbpageEof,                    /* xEof - check for end of scan */
    dbpage_స్తంభం,                 /* xColumn - read data */
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
    dbpage_Rollback_గమ్యం,             /* xRollbackTo */
    0,                            /* xShadowName */
    0                             /* xIntegrity */
  };
  తిరిగి sqlite3_create_module(db, "sqlite_dbpage", &dbpage_module, 0);
}
#లేదావేళ defined(SQLITE_ENABLE_DBPAGE_VTAB)
పూర్ణాంకం sqlite3_Dbpage_రిజిస్టరు(sqlite3 *db){ తిరిగి SQLITE_OK; }
#అయితే_ముగింపు /* SQLITE_ENABLE_DBSTAT_VTAB */
