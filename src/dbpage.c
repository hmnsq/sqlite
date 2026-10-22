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

#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"   /* Requires access to internal data structures */
#ਜੇ (defined(SQLITE_ENABLE_DBPAGE_VTAB) || defined(SQLITE_TEST)) \
    && !defined(SQLITE_OMIT_VIRTUALTABLE)

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Dbpage_ਸਾਰਣੀ Dbpage_ਸਾਰਣੀ;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ DbpageCursor DbpageCursor;

ਬਣਤਰ DbpageCursor {
  sqlite3_vtab_cursor ਅਧਾਰ;       /* Base class.  Must be first */
  Pgno pgno;                      /* Current page number */
  Pgno mxPgno;                    /* Last page to visit on this scan */
  Pager *pPager;                  /* Pager being read/written */
  Db_ਪੰਨਾ *pPage1;                 /* Page 1 of the database */
  ਪੂਰਨ iDb;                        /* Index of database to analyze */
  ਪੂਰਨ sz_ਪੰਨਾ;                     /* Size of each page in bytes */
};

ਬਣਤਰ Dbpage_ਸਾਰਣੀ {
  sqlite3_vtab ਅਧਾਰ;              /* Base class.  Must be first */
  sqlite3 *db;                    /* The database */
  ਪੂਰਨ iDbTrunc;                   /* Database to truncate */
  Pgno pgnoTrunc;                 /* Size to truncate to */
};

/* Columns */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DBPAGE_ਕਾਲਮ_PGNO    0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DBPAGE_ਕਾਲਮ_ਅੰਕੜੇ    1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DBPAGE_ਕਾਲਮ_SCHEMA  2


/*
** Connect to or create a dbpagevfs virtual table.
*/
ਸਥਿਰ ਪੂਰਨ dbpageConnect(
  sqlite3 *db,
  ਖਾਲੀ *pAux,
  ਪੂਰਨ argc, ਅਚਲ ਅੱਖਰ *ਅਚਲ*argv,
  sqlite3_vtab **ppVtab,
  ਅੱਖਰ **pzErr
){
  Dbpage_ਸਾਰਣੀ *pTab = 0;
  ਪੂਰਨ rc = SQLITE_OK;
  (ਖਾਲੀ)pAux;
  (ਖਾਲੀ)argc;
  (ਖਾਲੀ)argv;
  (ਖਾਲੀ)pzErr;

  sqlite3_vtab_config(db, SQLITE_VTAB_DIRECTONLY);
  sqlite3_vtab_config(db, SQLITE_VTAB_ਵਰਤਦਾ_ALL_SCHEMAS);
  rc = sqlite3_declare_vtab(db, 
          "CREATE TABLE x(pgno INTEGER PRIMARY KEY, data BLOB, schema HIDDEN)");
  ਜੇ( rc==SQLITE_OK ){
    pTab = (Dbpage_ਸਾਰਣੀ *)sqlite3_malloc64(ਆਕਾਰ(Dbpage_ਸਾਰਣੀ));
    ਜੇ( pTab==0 ) rc = SQLITE_NOMEM_BKPT;
  }

  ਪ੍ਰਤਿਜ੍ਞਾ( rc==SQLITE_OK || pTab==0 );
  ਜੇ( rc==SQLITE_OK ){
    memset(pTab, 0, ਆਕਾਰ(Dbpage_ਸਾਰਣੀ));
    pTab->db = db;
  }

  *ppVtab = (sqlite3_vtab*)pTab;
  ਵਾਪਸ rc;
}

/*
** Disconnect from or destroy a dbpagevfs virtual table.
*/
ਸਥਿਰ ਪੂਰਨ dbpageDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  ਵਾਪਸ SQLITE_OK;
}

/*
** idxNum:
**
**     0     schema=main, full table scan
**     1     schema=main, pgno=?1
**     2     schema=?1, full table scan
**     3     schema=?1, pgno=?2
*/
ਸਥਿਰ ਪੂਰਨ dbpage_ਉੱਤਮ_ਸੂਚਕਾਂਕ(sqlite3_vtab *tab, sqlite3_ਸੂਚਕਾਂਕ_info *pIdxInfo){
  ਪੂਰਨ i;
  ਪੂਰਨ iPlan = 0;
  (ਖਾਲੀ)tab;

  /* If there is a schema= constraint, it must be honored.  Report a
  ** ridiculously large estimated cost if the schema= constraint is
  ** unavailable
  */
  ਲਈ(i=0; i<pIdxInfo->nConstraint; i++){
    ਬਣਤਰ sqlite3_ਸੂਚਕਾਂਕ_constraint *p = &pIdxInfo->aConstraint[i];
    ਜੇ( p->i_ਕਾਲਮ!=DBPAGE_ਕਾਲਮ_SCHEMA ) ਜਾਰੀ;
    ਜੇ( p->op!=SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_EQ ) ਜਾਰੀ;
    ਜੇ( !p->usable ){
      /* No solution. */
      ਵਾਪਸ SQLITE_CONSTRAINT;
    }
    iPlan = 2;
    pIdxInfo->aConstraintUsage[i].argv_ਸੂਚਕਾਂਕ = 1;
    pIdxInfo->aConstraintUsage[i].omit = 1;
    ਰੋਕੋ;
  }

  /* If we reach this point, it means that either there is no schema=
  ** constraint (in which case we use the "main" schema) or else the
  ** schema constraint was accepted.  Lower the estimated cost accordingly
  */
  pIdxInfo->estimated_ਲਾਗਤ = 1.0e6;

  /* Check for constraints against pgno */
  ਲਈ(i=0; i<pIdxInfo->nConstraint; i++){
    ਬਣਤਰ sqlite3_ਸੂਚਕਾਂਕ_constraint *p = &pIdxInfo->aConstraint[i];
    ਜੇ( p->usable && p->i_ਕਾਲਮ<=0 && p->op==SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_EQ ){
      pIdxInfo->estimated_ਪੰਕਤੀਆਂ = 1;
      pIdxInfo->idxFlags = SQLITE_ਸੂਚਕਾਂਕ_ਜਾਂਚ_UNIQUE;
      pIdxInfo->estimated_ਲਾਗਤ = 1.0;
      pIdxInfo->aConstraintUsage[i].argv_ਸੂਚਕਾਂਕ = iPlan ? 2 : 1;
      pIdxInfo->aConstraintUsage[i].omit = 1;
      iPlan |= 1;
      ਰੋਕੋ;
    }
  }
  pIdxInfo->idx_ਅੰਕ = iPlan;

  ਜੇ( pIdxInfo->n_ਕ੍ਰਮ_By>=1
   && pIdxInfo->a_ਕ੍ਰਮ_By[0].i_ਕਾਲਮ<=0
   && pIdxInfo->a_ਕ੍ਰਮ_By[0].desc==0
  ){
    pIdxInfo->ਕ੍ਰਮ_By_Consumed = 1;
  }
  ਵਾਪਸ SQLITE_OK;
}

/*
** Open a new dbpagevfs cursor.
*/
ਸਥਿਰ ਪੂਰਨ dbpageOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  DbpageCursor *pCsr;

  pCsr = (DbpageCursor *)sqlite3_malloc64(ਆਕਾਰ(DbpageCursor));
  ਜੇ( pCsr==0 ){
    ਵਾਪਸ SQLITE_NOMEM_BKPT;
  }ਨਹੀਂਤਾਂ{
    memset(pCsr, 0, ਆਕਾਰ(DbpageCursor));
    pCsr->ਅਧਾਰ.pVtab = pVTab;
    pCsr->pgno = 0;
  }

  *ppCursor = (sqlite3_vtab_cursor *)pCsr;
  ਵਾਪਸ SQLITE_OK;
}

/*
** Close a dbpagevfs cursor.
*/
ਸਥਿਰ ਪੂਰਨ dbpageClose(sqlite3_vtab_cursor *pCursor){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  ਜੇ( pCsr->pPage1 ) sqlite3_Pager_Unref_ਪੰਨਾ_One(pCsr->pPage1);
  sqlite3_free(pCsr);
  ਵਾਪਸ SQLITE_OK;
}

/*
** Move a dbpagevfs cursor to the next entry in the file.
*/
ਸਥਿਰ ਪੂਰਨ dbpage_ਅਗਲਾ(sqlite3_vtab_cursor *pCursor){
  ਪੂਰਨ rc = SQLITE_OK;
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  pCsr->pgno++;
  ਵਾਪਸ rc;
}

ਸਥਿਰ ਪੂਰਨ dbpageEof(sqlite3_vtab_cursor *pCursor){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  ਵਾਪਸ pCsr->pgno > pCsr->mxPgno;
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
ਸਥਿਰ ਪੂਰਨ dbpageFilter(
  sqlite3_vtab_cursor *pCursor,
  ਪੂਰਨ idx_ਅੰਕ, ਅਚਲ ਅੱਖਰ *idxStr,
  ਪੂਰਨ argc, sqlite3_ਮੁੱਲ **argv
){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  Dbpage_ਸਾਰਣੀ *pTab = (Dbpage_ਸਾਰਣੀ *)pCursor->pVtab;
  ਪੂਰਨ rc;
  sqlite3 *db = pTab->db;
  Btree *pBt;

  ਅਣਵਰਤਿਆ_PARAMETER(idxStr);
  ਅਣਵਰਤਿਆ_PARAMETER(argc);

  /* Default setting is no rows of result */
  pCsr->pgno = 1;
  pCsr->mxPgno = 0;

  ਜੇ( idx_ਅੰਕ & 2 ){
    ਅਚਲ ਅੱਖਰ *zSchema;
    ਪ੍ਰਤਿਜ੍ਞਾ( argc>=1 );
    zSchema = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[0]);
    pCsr->iDb = sqlite3_ਲੱਭੋ_Db_Name(db, zSchema);
    ਜੇ( pCsr->iDb<0 ) ਵਾਪਸ SQLITE_OK;
  }ਨਹੀਂਤਾਂ{
    pCsr->iDb = 0;
  }
  pBt = db->aDb[pCsr->iDb].pBt;
  ਜੇ( NEVER(pBt==0) ) ਵਾਪਸ SQLITE_OK;
  pCsr->pPager = sqlite3BtreePager(pBt);
  pCsr->sz_ਪੰਨਾ = sqlite3_Btree_Get_ਪੰਨਾ_ਮਾਪ(pBt);
  pCsr->mxPgno = sqlite3_Btree_ਆਖਰੀ_ਪੰਨਾ(pBt);
  ਜੇ( idx_ਅੰਕ & 1 ){
    i64 iPg = sqlite3_ਮੁੱਲ_int64(argv[idx_ਅੰਕ>>1]);
    ਪ੍ਰਤਿਜ੍ਞਾ( argc>(idx_ਅੰਕ>>1) );
    ਜੇ( iPg<1 || iPg>pCsr->mxPgno ){
      pCsr->pgno = 1;
      pCsr->mxPgno = 0;
    }ਨਹੀਂਤਾਂ{
      pCsr->pgno = (Pgno)iPg;
      pCsr->mxPgno = pCsr->pgno;
    }
  }ਨਹੀਂਤਾਂ{
    ਪ੍ਰਤਿਜ੍ਞਾ( pCsr->pgno==1 );
  }
  ਜੇ( pCsr->pPage1 ) sqlite3_Pager_Unref_ਪੰਨਾ_One(pCsr->pPage1);
  rc = sqlite3PagerGet(pCsr->pPager, 1, &pCsr->pPage1, 0);
  ਵਾਪਸ rc;
}

ਸਥਿਰ ਪੂਰਨ dbpage_ਕਾਲਮ(
  sqlite3_vtab_cursor *pCursor,
  sqlite3_context *ctx,
  ਪੂਰਨ i
){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  ਪੂਰਨ rc = SQLITE_OK;
  ਚੋਣ( i ){
    ਹਾਲਤ 0: {           /* pgno */
      sqlite3_ਨਤੀਜਾ_int64(ctx, (sqlite3_int64)pCsr->pgno);
      ਰੋਕੋ;
    }
    ਹਾਲਤ 1: {           /*ਅੰਕੜੇ*/
      Db_ਪੰਨਾ *p_Db_ਪੰਨਾ = 0;
      ਜੇ( pCsr->pgno==(Pgno)((PENDING_BYTE/pCsr->sz_ਪੰਨਾ)+1) ){
        /* The pending byte page. Assume it is zeroed out. Attempting to
        ** request this page from the page is an SQLITE_CORRUPT error. */
        sqlite3_ਨਤੀਜਾ_zeroblob(ctx, pCsr->sz_ਪੰਨਾ);
      }ਨਹੀਂਤਾਂ{
        rc = sqlite3PagerGet(pCsr->pPager, pCsr->pgno, (Db_ਪੰਨਾ**)&p_Db_ਪੰਨਾ, 0);
        ਜੇ( rc==SQLITE_OK ){
          sqlite3_ਨਤੀਜਾ_blob(ctx, sqlite3_Pager_Get_ਅੰਕੜੇ(p_Db_ਪੰਨਾ), pCsr->sz_ਪੰਨਾ,
              SQLITE_TRANSIENT);
        }
        sqlite3PagerUnref(p_Db_ਪੰਨਾ);
      }
      ਰੋਕੋ;
    }
    ਮੂਲ: {          /* schema */
      sqlite3 *db = sqlite3_context_db_handle(ctx);
      sqlite3_ਨਤੀਜਾ_ਪਾਠ(ctx, db->aDb[pCsr->iDb].zDbSName, -1, SQLITE_STATIC);
      ਰੋਕੋ;
    }
  }
  ਵਾਪਸ rc;
}

ਸਥਿਰ ਪੂਰਨ dbpageRowid(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  *pRowid = pCsr->pgno;
  ਵਾਪਸ SQLITE_OK;
}

/* 
** Open write transactions. Since we do not know in advance which database
** files will be written by the sqlite_dbpage virtual table, start a write
** transaction on them all.
**
** Return SQLITE_OK if successful, or an SQLite error code otherwise.
*/
ਸਥਿਰ ਪੂਰਨ dbpageBeginTrans(Dbpage_ਸਾਰਣੀ *pTab){
  sqlite3 *db = pTab->db;
  ਪੂਰਨ rc = SQLITE_OK;
  ਪੂਰਨ i;
  ਲਈ(i=0; rc==SQLITE_OK && i<db->nDb; i++){
    Btree *pBt = db->aDb[i].pBt;
    ਜੇ( pBt ) rc = sqlite3BtreeBeginTrans(pBt, 1, 0);
  }
  ਵਾਪਸ rc;
}

ਸਥਿਰ ਪੂਰਨ dbpageUpdate(
  sqlite3_vtab *pVtab,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv,
  sqlite_int64 *pRowid
){
  Dbpage_ਸਾਰਣੀ *pTab = (Dbpage_ਸਾਰਣੀ *)pVtab;
  Pgno pgno;
  sqlite3_int64 pgno64;
  Db_ਪੰਨਾ *p_Db_ਪੰਨਾ = 0;
  ਪੂਰਨ rc = SQLITE_OK;
  ਅੱਖਰ *zErr = 0;
  ਪੂਰਨ iDb;
  Btree *pBt;
  Pager *pPager;
  ਪੂਰਨ sz_ਪੰਨਾ;
  ਪੂਰਨ is_ਪਾਓ;

  (ਖਾਲੀ)pRowid;
  ਜੇ( pTab->db->flags & SQLITE_Defensive ){
    zErr = "read-only";
    ਜਾਓ update_fail;
  }
  ਜੇ( argc==1 ){
    zErr = "cannot delete";
    ਜਾਓ update_fail;
  }
  ਜੇ( sqlite3_ਮੁੱਲ_type(argv[0])==SQLITE_NULL ){
    pgno64 = sqlite3_ਮੁੱਲ_int64(argv[2]);
    is_ਪਾਓ = 1;
  }ਨਹੀਂਤਾਂ{
    pgno64 = (Pgno)sqlite3_ਮੁੱਲ_int64(argv[0]);
    ਜੇ( sqlite3_ਮੁੱਲ_int64(argv[1])!=pgno64 ){
      zErr = "cannot insert";
      ਜਾਓ update_fail;
    }
    is_ਪਾਓ = 0;
  }
  ਜੇ( sqlite3_ਮੁੱਲ_type(argv[4])==SQLITE_NULL ){
    iDb = 0;
  }ਨਹੀਂਤਾਂ{
    ਅਚਲ ਅੱਖਰ *zSchema = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[4]);
    iDb = sqlite3_ਲੱਭੋ_Db_Name(pTab->db, zSchema);
    ਜੇ( iDb<0 ){
      zErr = "no such schema";
      ਜਾਓ update_fail;
    }
  }
  pBt = pTab->db->aDb[iDb].pBt;
  ਜੇ( pgno64<1 || pgno64>4294967294U || NEVER(pBt==0) ){
    zErr = "bad page number";
    ਜਾਓ update_fail;
  }
  pgno = (Pgno)pgno64;
  sz_ਪੰਨਾ = sqlite3_Btree_Get_ਪੰਨਾ_ਮਾਪ(pBt);
  ਜੇ( sqlite3_ਮੁੱਲ_type(argv[3])!=SQLITE_BLOB 
   || sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ(argv[3])!=sz_ਪੰਨਾ
  ){
    ਜੇ( sqlite3_ਮੁੱਲ_type(argv[3])==SQLITE_NULL && is_ਪਾਓ && pgno>1 ){
      /* "INSERT INTO dbpage($PGNO,NULL)" causes page number $PGNO and
      ** all subsequent pages to be deleted. */
      pTab->iDbTrunc = iDb;
      pTab->pgnoTrunc = pgno-1;
      pgno = 1;
    }ਨਹੀਂਤਾਂ{
      zErr = "bad page value";
      ਜਾਓ update_fail;
    }
  }

  ਜੇ( dbpageBeginTrans(pTab)!=SQLITE_OK ){
    zErr = "failed to open transaction";
    ਜਾਓ update_fail;
  }

  pPager = sqlite3BtreePager(pBt);
  rc = sqlite3PagerGet(pPager, pgno, (Db_ਪੰਨਾ**)&p_Db_ਪੰਨਾ, 0);
  ਜੇ( rc==SQLITE_OK ){
    ਅਚਲ ਖਾਲੀ *p_ਅੰਕੜੇ = sqlite3_ਮੁੱਲ_blob(argv[3]);
    ਜੇ( (rc = sqlite3PagerWrite(p_Db_ਪੰਨਾ))==SQLITE_OK && p_ਅੰਕੜੇ ){
      ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *a_ਪੰਨਾ = sqlite3_Pager_Get_ਅੰਕੜੇ(p_Db_ਪੰਨਾ);
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(a_ਪੰਨਾ, p_ਅੰਕੜੇ, sz_ਪੰਨਾ);
      pTab->pgnoTrunc = 0;
    }
  }
  ਜੇ( rc!=SQLITE_OK ){
    pTab->pgnoTrunc = 0;
  }
  sqlite3PagerUnref(p_Db_ਪੰਨਾ);
  ਵਾਪਸ rc;

update_fail:
  pTab->pgnoTrunc = 0;
  sqlite3_free(pVtab->z_Err_ਸੁਨੇਹਾ);
  pVtab->z_Err_ਸੁਨੇਹਾ = sqlite3_mprintf("%s", zErr);
  ਵਾਪਸ SQLITE_ERROR;
}

ਸਥਿਰ ਪੂਰਨ dbpageBegin(sqlite3_vtab *pVtab){
  Dbpage_ਸਾਰਣੀ *pTab = (Dbpage_ਸਾਰਣੀ *)pVtab;
  pTab->pgnoTrunc = 0;
  ਵਾਪਸ SQLITE_OK;
}

/* Invoke sqlite3PagerTruncate() as necessary, just prior to COMMIT
*/
ਸਥਿਰ ਪੂਰਨ dbpageSync(sqlite3_vtab *pVtab){
  Dbpage_ਸਾਰਣੀ *pTab = (Dbpage_ਸਾਰਣੀ *)pVtab;
  ਜੇ( pTab->pgnoTrunc>0 ){
    Btree *pBt = pTab->db->aDb[pTab->iDbTrunc].pBt;
    Pager *pPager = sqlite3BtreePager(pBt);
    sqlite3BtreeEnter(pBt);
    ਜੇ( pTab->pgnoTrunc<sqlite3_Btree_ਆਖਰੀ_ਪੰਨਾ(pBt) ){
      sqlite3PagerTruncateImage(pPager, pTab->pgnoTrunc);
    }
    sqlite3BtreeLeave(pBt);
  }
  pTab->pgnoTrunc = 0;
  ਵਾਪਸ SQLITE_OK;
}

/* Cancel any pending truncate.
*/
ਸਥਿਰ ਪੂਰਨ dbpage_Rollback_ਮੰਜ਼ਿਲ(sqlite3_vtab *pVtab, ਪੂਰਨ notUsed1){
  Dbpage_ਸਾਰਣੀ *pTab = (Dbpage_ਸਾਰਣੀ *)pVtab;
  pTab->pgnoTrunc = 0;
  (ਖਾਲੀ)notUsed1;
  ਵਾਪਸ SQLITE_OK;
}

/*
** Invoke this routine to register the "dbpage" virtual table module
*/
ਪੂਰਨ sqlite3_Dbpage_ਰਜਿਸਟਰ(sqlite3 *db){
  ਸਥਿਰ sqlite3_module dbpage_module = {
    2,                            /* iVersion */
    dbpageConnect,                /* xCreate */
    dbpageConnect,                /* xConnect */
    dbpage_ਉੱਤਮ_ਸੂਚਕਾਂਕ,              /* xBestIndex */
    dbpageDisconnect,             /* xDisconnect */
    dbpageDisconnect,             /* xDestroy */
    dbpageOpen,                   /* xOpen - open a cursor */
    dbpageClose,                  /* xClose - close a cursor */
    dbpageFilter,                 /* xFilter - configure scan constraints */
    dbpage_ਅਗਲਾ,                   /* xNext - advance a cursor */
    dbpageEof,                    /* xEof - check for end of scan */
    dbpage_ਕਾਲਮ,                 /* xColumn - read data */
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
    dbpage_Rollback_ਮੰਜ਼ਿਲ,             /* xRollbackTo */
    0,                            /* xShadowName */
    0                             /* xIntegrity */
  };
  ਵਾਪਸ sqlite3_create_module(db, "sqlite_dbpage", &dbpage_module, 0);
}
#ਜਾਂਜੇ defined(SQLITE_ENABLE_DBPAGE_VTAB)
ਪੂਰਨ sqlite3_Dbpage_ਰਜਿਸਟਰ(sqlite3 *db){ ਵਾਪਸ SQLITE_OK; }
#ਜੇ_ਅੰਤ /* SQLITE_ENABLE_DBSTAT_VTAB */
