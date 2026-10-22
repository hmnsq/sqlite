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

#ಅನ್ತರ್ಭಾವ "sqliteInt.h"   /* Requires access to internal data structures */
#ಆದರೆ (defined(SQLITE_ENABLE_DBPAGE_VTAB) || defined(SQLITE_TEST)) \
    && !defined(SQLITE_OMIT_VIRTUALTABLE)

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Dbpage_ಕೋಷ್ಟಕ Dbpage_ಕೋಷ್ಟಕ;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ DbpageCursor DbpageCursor;

ರಚನೆ DbpageCursor {
  sqlite3_vtab_cursor ತಳ;       /* Base class.  Must be first */
  Pgno pgno;                      /* Current page number */
  Pgno mxPgno;                    /* Last page to visit on this scan */
  Pager *pPager;                  /* Pager being read/written */
  Db_ಪುಟ *pPage1;                 /* Page 1 of the database */
  ಪೂರ್ಣಾಂಕ iDb;                        /* Index of database to analyze */
  ಪೂರ್ಣಾಂಕ sz_ಪುಟ;                     /* Size of each page in bytes */
};

ರಚನೆ Dbpage_ಕೋಷ್ಟಕ {
  sqlite3_vtab ತಳ;              /* Base class.  Must be first */
  sqlite3 *db;                    /* The database */
  ಪೂರ್ಣಾಂಕ iDbTrunc;                   /* Database to truncate */
  Pgno pgnoTrunc;                 /* Size to truncate to */
};

/* Columns */
#ವ್ಯಾಖ್ಯಾನಿಸು DBPAGE_ಸ್ತಂಭ_PGNO    0
#ವ್ಯಾಖ್ಯಾನಿಸು DBPAGE_ಸ್ತಂಭ_ದತ್ತಾಂಶ    1
#ವ್ಯಾಖ್ಯಾನಿಸು DBPAGE_ಸ್ತಂಭ_SCHEMA  2


/*
** Connect to or create a dbpagevfs virtual table.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ dbpageConnect(
  sqlite3 *db,
  ಶೂನ್ಯ *pAux,
  ಪೂರ್ಣಾಂಕ argc, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ*argv,
  sqlite3_vtab **ppVtab,
  ಅಕ್ಷರ **pzErr
){
  Dbpage_ಕೋಷ್ಟಕ *pTab = 0;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  (ಶೂನ್ಯ)pAux;
  (ಶೂನ್ಯ)argc;
  (ಶೂನ್ಯ)argv;
  (ಶೂನ್ಯ)pzErr;

  sqlite3_vtab_config(db, SQLITE_VTAB_DIRECTONLY);
  sqlite3_vtab_config(db, SQLITE_VTAB_ಬಳಸುತ್ತದೆ_ALL_SCHEMAS);
  rc = sqlite3_declare_vtab(db, 
          "CREATE TABLE x(pgno INTEGER PRIMARY KEY, data BLOB, schema HIDDEN)");
  ಆದರೆ( rc==SQLITE_OK ){
    pTab = (Dbpage_ಕೋಷ್ಟಕ *)sqlite3_malloc64(ಗಾತ್ರ(Dbpage_ಕೋಷ್ಟಕ));
    ಆದರೆ( pTab==0 ) rc = SQLITE_NOMEM_BKPT;
  }

  ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_OK || pTab==0 );
  ಆದರೆ( rc==SQLITE_OK ){
    memset(pTab, 0, ಗಾತ್ರ(Dbpage_ಕೋಷ್ಟಕ));
    pTab->db = db;
  }

  *ppVtab = (sqlite3_vtab*)pTab;
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Disconnect from or destroy a dbpagevfs virtual table.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ dbpageDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** idxNum:
**
**     0     schema=main, full table scan
**     1     schema=main, pgno=?1
**     2     schema=?1, full table scan
**     3     schema=?1, pgno=?2
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ dbpage_ಉತ್ತಮ_ಸೂಚ್ಯಂಕ(sqlite3_vtab *tab, sqlite3_ಸೂಚ್ಯಂಕ_info *pIdxInfo){
  ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ iPlan = 0;
  (ಶೂನ್ಯ)tab;

  /* If there is a schema= constraint, it must be honored.  Report a
  ** ridiculously large estimated cost if the schema= constraint is
  ** unavailable
  */
  ಗಾಗಿ(i=0; i<pIdxInfo->nConstraint; i++){
    ರಚನೆ sqlite3_ಸೂಚ್ಯಂಕ_constraint *p = &pIdxInfo->aConstraint[i];
    ಆದರೆ( p->i_ಸ್ತಂಭ!=DBPAGE_ಸ್ತಂಭ_SCHEMA ) ಮುಂದುವರಿ;
    ಆದರೆ( p->op!=SQLITE_ಸೂಚ್ಯಂಕ_CONSTRAINT_EQ ) ಮುಂದುವರಿ;
    ಆದರೆ( !p->usable ){
      /* No solution. */
      ಹಿಂತಿರುಗಿ SQLITE_CONSTRAINT;
    }
    iPlan = 2;
    pIdxInfo->aConstraintUsage[i].argv_ಸೂಚ್ಯಂಕ = 1;
    pIdxInfo->aConstraintUsage[i].omit = 1;
    ನಿಲ್ಲಿಸು;
  }

  /* If we reach this point, it means that either there is no schema=
  ** constraint (in which case we use the "main" schema) or else the
  ** schema constraint was accepted.  Lower the estimated cost accordingly
  */
  pIdxInfo->estimated_ವೆಚ್ಚ = 1.0e6;

  /* Check for constraints against pgno */
  ಗಾಗಿ(i=0; i<pIdxInfo->nConstraint; i++){
    ರಚನೆ sqlite3_ಸೂಚ್ಯಂಕ_constraint *p = &pIdxInfo->aConstraint[i];
    ಆದರೆ( p->usable && p->i_ಸ್ತಂಭ<=0 && p->op==SQLITE_ಸೂಚ್ಯಂಕ_CONSTRAINT_EQ ){
      pIdxInfo->estimated_ಅಡ್ಡಸಾಲುಗಳು = 1;
      pIdxInfo->idxFlags = SQLITE_ಸೂಚ್ಯಂಕ_ಪರಿಶೀಲನೆ_UNIQUE;
      pIdxInfo->estimated_ವೆಚ್ಚ = 1.0;
      pIdxInfo->aConstraintUsage[i].argv_ಸೂಚ್ಯಂಕ = iPlan ? 2 : 1;
      pIdxInfo->aConstraintUsage[i].omit = 1;
      iPlan |= 1;
      ನಿಲ್ಲಿಸು;
    }
  }
  pIdxInfo->idx_ಅಂಕೆ = iPlan;

  ಆದರೆ( pIdxInfo->n_ಕ್ರಮ_By>=1
   && pIdxInfo->a_ಕ್ರಮ_By[0].i_ಸ್ತಂಭ<=0
   && pIdxInfo->a_ಕ್ರಮ_By[0].desc==0
  ){
    pIdxInfo->ಕ್ರಮ_By_Consumed = 1;
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Open a new dbpagevfs cursor.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ dbpageOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  DbpageCursor *pCsr;

  pCsr = (DbpageCursor *)sqlite3_malloc64(ಗಾತ್ರ(DbpageCursor));
  ಆದರೆ( pCsr==0 ){
    ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
  }ಇಲ್ಲದಿದ್ದರೆ{
    memset(pCsr, 0, ಗಾತ್ರ(DbpageCursor));
    pCsr->ತಳ.pVtab = pVTab;
    pCsr->pgno = 0;
  }

  *ppCursor = (sqlite3_vtab_cursor *)pCsr;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Close a dbpagevfs cursor.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ dbpageClose(sqlite3_vtab_cursor *pCursor){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  ಆದರೆ( pCsr->pPage1 ) sqlite3_Pager_Unref_ಪುಟ_One(pCsr->pPage1);
  sqlite3_free(pCsr);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Move a dbpagevfs cursor to the next entry in the file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ dbpage_ಮುಂದಿನ(sqlite3_vtab_cursor *pCursor){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  pCsr->pgno++;
  ಹಿಂತಿರುಗಿ rc;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ dbpageEof(sqlite3_vtab_cursor *pCursor){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  ಹಿಂತಿರುಗಿ pCsr->pgno > pCsr->mxPgno;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ dbpageFilter(
  sqlite3_vtab_cursor *pCursor,
  ಪೂರ್ಣಾಂಕ idx_ಅಂಕೆ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *idxStr,
  ಪೂರ್ಣಾಂಕ argc, sqlite3_ಮೌಲ್ಯ **argv
){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  Dbpage_ಕೋಷ್ಟಕ *pTab = (Dbpage_ಕೋಷ್ಟಕ *)pCursor->pVtab;
  ಪೂರ್ಣಾಂಕ rc;
  sqlite3 *db = pTab->db;
  Btree *pBt;

  ಬಳಸದ_PARAMETER(idxStr);
  ಬಳಸದ_PARAMETER(argc);

  /* Default setting is no rows of result */
  pCsr->pgno = 1;
  pCsr->mxPgno = 0;

  ಆದರೆ( idx_ಅಂಕೆ & 2 ){
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSchema;
    ಪ್ರತಿಜ್ಞಾ( argc>=1 );
    zSchema = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[0]);
    pCsr->iDb = sqlite3_ಕಂಡುಹಿಡಿ_Db_Name(db, zSchema);
    ಆದರೆ( pCsr->iDb<0 ) ಹಿಂತಿರುಗಿ SQLITE_OK;
  }ಇಲ್ಲದಿದ್ದರೆ{
    pCsr->iDb = 0;
  }
  pBt = db->aDb[pCsr->iDb].pBt;
  ಆದರೆ( NEVER(pBt==0) ) ಹಿಂತಿರುಗಿ SQLITE_OK;
  pCsr->pPager = sqlite3BtreePager(pBt);
  pCsr->sz_ಪುಟ = sqlite3_Btree_Get_ಪುಟ_ಗಾತ್ರ(pBt);
  pCsr->mxPgno = sqlite3_Btree_ಕೊನೆಯ_ಪುಟ(pBt);
  ಆದರೆ( idx_ಅಂಕೆ & 1 ){
    i64 iPg = sqlite3_ಮೌಲ್ಯ_int64(argv[idx_ಅಂಕೆ>>1]);
    ಪ್ರತಿಜ್ಞಾ( argc>(idx_ಅಂಕೆ>>1) );
    ಆದರೆ( iPg<1 || iPg>pCsr->mxPgno ){
      pCsr->pgno = 1;
      pCsr->mxPgno = 0;
    }ಇಲ್ಲದಿದ್ದರೆ{
      pCsr->pgno = (Pgno)iPg;
      pCsr->mxPgno = pCsr->pgno;
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪ್ರತಿಜ್ಞಾ( pCsr->pgno==1 );
  }
  ಆದರೆ( pCsr->pPage1 ) sqlite3_Pager_Unref_ಪುಟ_One(pCsr->pPage1);
  rc = sqlite3PagerGet(pCsr->pPager, 1, &pCsr->pPage1, 0);
  ಹಿಂತಿರುಗಿ rc;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ dbpage_ಸ್ತಂಭ(
  sqlite3_vtab_cursor *pCursor,
  sqlite3_context *ctx,
  ಪೂರ್ಣಾಂಕ i
){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  ಆಯ್ಕೆ( i ){
    ಸ್ಥಿತಿ 0: {           /* pgno */
      sqlite3_ಫಲಿತಾಂಶ_int64(ctx, (sqlite3_int64)pCsr->pgno);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ 1: {           /*ದತ್ತಾಂಶ*/
      Db_ಪುಟ *p_Db_ಪುಟ = 0;
      ಆದರೆ( pCsr->pgno==(Pgno)((PENDING_BYTE/pCsr->sz_ಪುಟ)+1) ){
        /* The pending byte page. Assume it is zeroed out. Attempting to
        ** request this page from the page is an SQLITE_CORRUPT error. */
        sqlite3_ಫಲಿತಾಂಶ_zeroblob(ctx, pCsr->sz_ಪುಟ);
      }ಇಲ್ಲದಿದ್ದರೆ{
        rc = sqlite3PagerGet(pCsr->pPager, pCsr->pgno, (Db_ಪುಟ**)&p_Db_ಪುಟ, 0);
        ಆದರೆ( rc==SQLITE_OK ){
          sqlite3_ಫಲಿತಾಂಶ_blob(ctx, sqlite3_Pager_Get_ದತ್ತಾಂಶ(p_Db_ಪುಟ), pCsr->sz_ಪುಟ,
              SQLITE_TRANSIENT);
        }
        sqlite3PagerUnref(p_Db_ಪುಟ);
      }
      ನಿಲ್ಲಿಸು;
    }
    ಪೂರ್ವನಿಯೋಜಿತ: {          /* schema */
      sqlite3 *db = sqlite3_context_db_handle(ctx);
      sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(ctx, db->aDb[pCsr->iDb].zDbSName, -1, SQLITE_STATIC);
      ನಿಲ್ಲಿಸು;
    }
  }
  ಹಿಂತಿರುಗಿ rc;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ dbpageRowid(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  *pRowid = pCsr->pgno;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/* 
** Open write transactions. Since we do not know in advance which database
** files will be written by the sqlite_dbpage virtual table, start a write
** transaction on them all.
**
** Return SQLITE_OK if successful, or an SQLite error code otherwise.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ dbpageBeginTrans(Dbpage_ಕೋಷ್ಟಕ *pTab){
  sqlite3 *db = pTab->db;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  ಪೂರ್ಣಾಂಕ i;
  ಗಾಗಿ(i=0; rc==SQLITE_OK && i<db->nDb; i++){
    Btree *pBt = db->aDb[i].pBt;
    ಆದರೆ( pBt ) rc = sqlite3BtreeBeginTrans(pBt, 1, 0);
  }
  ಹಿಂತಿರುಗಿ rc;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ dbpageUpdate(
  sqlite3_vtab *pVtab,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv,
  sqlite_int64 *pRowid
){
  Dbpage_ಕೋಷ್ಟಕ *pTab = (Dbpage_ಕೋಷ್ಟಕ *)pVtab;
  Pgno pgno;
  sqlite3_int64 pgno64;
  Db_ಪುಟ *p_Db_ಪುಟ = 0;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  ಅಕ್ಷರ *zErr = 0;
  ಪೂರ್ಣಾಂಕ iDb;
  Btree *pBt;
  Pager *pPager;
  ಪೂರ್ಣಾಂಕ sz_ಪುಟ;
  ಪೂರ್ಣಾಂಕ is_ಸೇರಿಸು;

  (ಶೂನ್ಯ)pRowid;
  ಆದರೆ( pTab->db->flags & SQLITE_Defensive ){
    zErr = "read-only";
    ಹೋಗು update_fail;
  }
  ಆದರೆ( argc==1 ){
    zErr = "cannot delete";
    ಹೋಗು update_fail;
  }
  ಆದರೆ( sqlite3_ಮೌಲ್ಯ_type(argv[0])==SQLITE_NULL ){
    pgno64 = sqlite3_ಮೌಲ್ಯ_int64(argv[2]);
    is_ಸೇರಿಸು = 1;
  }ಇಲ್ಲದಿದ್ದರೆ{
    pgno64 = (Pgno)sqlite3_ಮೌಲ್ಯ_int64(argv[0]);
    ಆದರೆ( sqlite3_ಮೌಲ್ಯ_int64(argv[1])!=pgno64 ){
      zErr = "cannot insert";
      ಹೋಗು update_fail;
    }
    is_ಸೇರಿಸು = 0;
  }
  ಆದರೆ( sqlite3_ಮೌಲ್ಯ_type(argv[4])==SQLITE_NULL ){
    iDb = 0;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSchema = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[4]);
    iDb = sqlite3_ಕಂಡುಹಿಡಿ_Db_Name(pTab->db, zSchema);
    ಆದರೆ( iDb<0 ){
      zErr = "no such schema";
      ಹೋಗು update_fail;
    }
  }
  pBt = pTab->db->aDb[iDb].pBt;
  ಆದರೆ( pgno64<1 || pgno64>4294967294U || NEVER(pBt==0) ){
    zErr = "bad page number";
    ಹೋಗು update_fail;
  }
  pgno = (Pgno)pgno64;
  sz_ಪುಟ = sqlite3_Btree_Get_ಪುಟ_ಗಾತ್ರ(pBt);
  ಆದರೆ( sqlite3_ಮೌಲ್ಯ_type(argv[3])!=SQLITE_BLOB 
   || sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು(argv[3])!=sz_ಪುಟ
  ){
    ಆದರೆ( sqlite3_ಮೌಲ್ಯ_type(argv[3])==SQLITE_NULL && is_ಸೇರಿಸು && pgno>1 ){
      /* "INSERT INTO dbpage($PGNO,NULL)" causes page number $PGNO and
      ** all subsequent pages to be deleted. */
      pTab->iDbTrunc = iDb;
      pTab->pgnoTrunc = pgno-1;
      pgno = 1;
    }ಇಲ್ಲದಿದ್ದರೆ{
      zErr = "bad page value";
      ಹೋಗು update_fail;
    }
  }

  ಆದರೆ( dbpageBeginTrans(pTab)!=SQLITE_OK ){
    zErr = "failed to open transaction";
    ಹೋಗು update_fail;
  }

  pPager = sqlite3BtreePager(pBt);
  rc = sqlite3PagerGet(pPager, pgno, (Db_ಪುಟ**)&p_Db_ಪುಟ, 0);
  ಆದರೆ( rc==SQLITE_OK ){
    ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *p_ದತ್ತಾಂಶ = sqlite3_ಮೌಲ್ಯ_blob(argv[3]);
    ಆದರೆ( (rc = sqlite3PagerWrite(p_Db_ಪುಟ))==SQLITE_OK && p_ದತ್ತಾಂಶ ){
      ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *a_ಪುಟ = sqlite3_Pager_Get_ದತ್ತಾಂಶ(p_Db_ಪುಟ);
      ಸ್ಮರಣೆ_ನಕಲು(a_ಪುಟ, p_ದತ್ತಾಂಶ, sz_ಪುಟ);
      pTab->pgnoTrunc = 0;
    }
  }
  ಆದರೆ( rc!=SQLITE_OK ){
    pTab->pgnoTrunc = 0;
  }
  sqlite3PagerUnref(p_Db_ಪುಟ);
  ಹಿಂತಿರುಗಿ rc;

update_fail:
  pTab->pgnoTrunc = 0;
  sqlite3_free(pVtab->z_Err_ಸಂದೇಶ);
  pVtab->z_Err_ಸಂದೇಶ = sqlite3_mprintf("%s", zErr);
  ಹಿಂತಿರುಗಿ SQLITE_ERROR;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ dbpageBegin(sqlite3_vtab *pVtab){
  Dbpage_ಕೋಷ್ಟಕ *pTab = (Dbpage_ಕೋಷ್ಟಕ *)pVtab;
  pTab->pgnoTrunc = 0;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/* Invoke sqlite3PagerTruncate() as necessary, just prior to COMMIT
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ dbpageSync(sqlite3_vtab *pVtab){
  Dbpage_ಕೋಷ್ಟಕ *pTab = (Dbpage_ಕೋಷ್ಟಕ *)pVtab;
  ಆದರೆ( pTab->pgnoTrunc>0 ){
    Btree *pBt = pTab->db->aDb[pTab->iDbTrunc].pBt;
    Pager *pPager = sqlite3BtreePager(pBt);
    sqlite3BtreeEnter(pBt);
    ಆದರೆ( pTab->pgnoTrunc<sqlite3_Btree_ಕೊನೆಯ_ಪುಟ(pBt) ){
      sqlite3PagerTruncateImage(pPager, pTab->pgnoTrunc);
    }
    sqlite3BtreeLeave(pBt);
  }
  pTab->pgnoTrunc = 0;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/* Cancel any pending truncate.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ dbpage_Rollback_ಗಮ್ಯ(sqlite3_vtab *pVtab, ಪೂರ್ಣಾಂಕ notUsed1){
  Dbpage_ಕೋಷ್ಟಕ *pTab = (Dbpage_ಕೋಷ್ಟಕ *)pVtab;
  pTab->pgnoTrunc = 0;
  (ಶೂನ್ಯ)notUsed1;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Invoke this routine to register the "dbpage" virtual table module
*/
ಪೂರ್ಣಾಂಕ sqlite3_Dbpage_ರಿಜಿಸ್ಟರ್(sqlite3 *db){
  ಸ್ಥಿರ sqlite3_module dbpage_module = {
    2,                            /* iVersion */
    dbpageConnect,                /* xCreate */
    dbpageConnect,                /* xConnect */
    dbpage_ಉತ್ತಮ_ಸೂಚ್ಯಂಕ,              /* xBestIndex */
    dbpageDisconnect,             /* xDisconnect */
    dbpageDisconnect,             /* xDestroy */
    dbpageOpen,                   /* xOpen - open a cursor */
    dbpageClose,                  /* xClose - close a cursor */
    dbpageFilter,                 /* xFilter - configure scan constraints */
    dbpage_ಮುಂದಿನ,                   /* xNext - advance a cursor */
    dbpageEof,                    /* xEof - check for end of scan */
    dbpage_ಸ್ತಂಭ,                 /* xColumn - read data */
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
    dbpage_Rollback_ಗಮ್ಯ,             /* xRollbackTo */
    0,                            /* xShadowName */
    0                             /* xIntegrity */
  };
  ಹಿಂತಿರುಗಿ sqlite3_create_module(db, "sqlite_dbpage", &dbpage_module, 0);
}
#ಅಥವಾಆದರೆ defined(SQLITE_ENABLE_DBPAGE_VTAB)
ಪೂರ್ಣಾಂಕ sqlite3_Dbpage_ರಿಜಿಸ್ಟರ್(sqlite3 *db){ ಹಿಂತಿರುಗಿ SQLITE_OK; }
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_ENABLE_DBSTAT_VTAB */
