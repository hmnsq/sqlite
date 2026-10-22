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

#અન્તર્ભાવ "sqliteInt.h"   /* Requires access to internal data structures */
#જો (defined(SQLITE_ENABLE_DBPAGE_VTAB) || defined(SQLITE_TEST)) \
    && !defined(SQLITE_OMIT_VIRTUALTABLE)

પ્રકારવ્યાખ્યા સંરચના DbpageTable DbpageTable;
પ્રકારવ્યાખ્યા સંરચના DbpageCursor DbpageCursor;

સંરચના DbpageCursor {
  sqlite3_vtab_cursor base;       /* Base class.  Must be first */
  Pgno pgno;                      /* Current page number */
  Pgno mxPgno;                    /* Last page to visit on this scan */
  Pager *pPager;                  /* Pager being read/written */
  DbPage *pPage1;                 /* Page 1 of the database */
  પૂર્ણાંક iDb;                        /* Index of database to analyze */
  પૂર્ણાંક szPage;                     /* Size of each page in bytes */
};

સંરચના DbpageTable {
  sqlite3_vtab base;              /* Base class.  Must be first */
  sqlite3 *db;                    /* The database */
  પૂર્ણાંક iDbTrunc;                   /* Database to truncate */
  Pgno pgnoTrunc;                 /* Size to truncate to */
};

/* Columns */
#define DBPAGE_COLUMN_PGNO    0
#define DBPAGE_COLUMN_DATA    1
#define DBPAGE_COLUMN_SCHEMA  2


/*
** Connect to or create a dbpagevfs virtual table.
*/
સ્થિર પૂર્ણાંક dbpageConnect(
  sqlite3 *db,
  શૂન્ય *pAux,
  પૂર્ણાંક argc, અચલ અક્ષર *અચલ*argv,
  sqlite3_vtab **ppVtab,
  અક્ષર **pzErr
){
  DbpageTable *pTab = 0;
  પૂર્ણાંક rc = SQLITE_OK;
  (શૂન્ય)pAux;
  (શૂન્ય)argc;
  (શૂન્ય)argv;
  (શૂન્ય)pzErr;

  sqlite3_vtab_config(db, SQLITE_VTAB_DIRECTONLY);
  sqlite3_vtab_config(db, SQLITE_VTAB_USES_ALL_SCHEMAS);
  rc = sqlite3_declare_vtab(db, 
          "CREATE TABLE x(pgno INTEGER PRIMARY KEY, data BLOB, schema HIDDEN)");
  જો( rc==SQLITE_OK ){
    pTab = (DbpageTable *)sqlite3_malloc64(કદ(DbpageTable));
    જો( pTab==0 ) rc = SQLITE_NOMEM_BKPT;
  }

  પ્રતિજ્ઞા( rc==SQLITE_OK || pTab==0 );
  જો( rc==SQLITE_OK ){
    memset(pTab, 0, કદ(DbpageTable));
    pTab->db = db;
  }

  *ppVtab = (sqlite3_vtab*)pTab;
  પાછા rc;
}

/*
** Disconnect from or destroy a dbpagevfs virtual table.
*/
સ્થિર પૂર્ણાંક dbpageDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  પાછા SQLITE_OK;
}

/*
** idxNum:
**
**     0     schema=main, full table scan
**     1     schema=main, pgno=?1
**     2     schema=?1, full table scan
**     3     schema=?1, pgno=?2
*/
સ્થિર પૂર્ણાંક dbpageBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  પૂર્ણાંક i;
  પૂર્ણાંક iPlan = 0;
  (શૂન્ય)tab;

  /* If there is a schema= constraint, it must be honored.  Report a
  ** ridiculously large estimated cost if the schema= constraint is
  ** unavailable
  */
  માટે(i=0; i<pIdxInfo->nConstraint; i++){
    સંરચના sqlite3_index_constraint *p = &pIdxInfo->aConstraint[i];
    જો( p->iColumn!=DBPAGE_COLUMN_SCHEMA ) ચાલુ;
    જો( p->op!=SQLITE_INDEX_CONSTRAINT_EQ ) ચાલુ;
    જો( !p->usable ){
      /* No solution. */
      પાછા SQLITE_CONSTRAINT;
    }
    iPlan = 2;
    pIdxInfo->aConstraintUsage[i].argvIndex = 1;
    pIdxInfo->aConstraintUsage[i].omit = 1;
    તોડો;
  }

  /* If we reach this point, it means that either there is no schema=
  ** constraint (in which case we use the "main" schema) or else the
  ** schema constraint was accepted.  Lower the estimated cost accordingly
  */
  pIdxInfo->estimatedCost = 1.0e6;

  /* Check for constraints against pgno */
  માટે(i=0; i<pIdxInfo->nConstraint; i++){
    સંરચના sqlite3_index_constraint *p = &pIdxInfo->aConstraint[i];
    જો( p->usable && p->iColumn<=0 && p->op==SQLITE_INDEX_CONSTRAINT_EQ ){
      pIdxInfo->estimatedRows = 1;
      pIdxInfo->idxFlags = SQLITE_INDEX_SCAN_UNIQUE;
      pIdxInfo->estimatedCost = 1.0;
      pIdxInfo->aConstraintUsage[i].argvIndex = iPlan ? 2 : 1;
      pIdxInfo->aConstraintUsage[i].omit = 1;
      iPlan |= 1;
      તોડો;
    }
  }
  pIdxInfo->idx_અંક = iPlan;

  જો( pIdxInfo->nOrderBy>=1
   && pIdxInfo->aOrderBy[0].iColumn<=0
   && pIdxInfo->aOrderBy[0].desc==0
  ){
    pIdxInfo->orderByConsumed = 1;
  }
  પાછા SQLITE_OK;
}

/*
** Open a new dbpagevfs cursor.
*/
સ્થિર પૂર્ણાંક dbpageOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  DbpageCursor *pCsr;

  pCsr = (DbpageCursor *)sqlite3_malloc64(કદ(DbpageCursor));
  જો( pCsr==0 ){
    પાછા SQLITE_NOMEM_BKPT;
  }નહીંતર{
    memset(pCsr, 0, કદ(DbpageCursor));
    pCsr->base.pVtab = pVTab;
    pCsr->pgno = 0;
  }

  *ppCursor = (sqlite3_vtab_cursor *)pCsr;
  પાછા SQLITE_OK;
}

/*
** Close a dbpagevfs cursor.
*/
સ્થિર પૂર્ણાંક dbpageClose(sqlite3_vtab_cursor *pCursor){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  જો( pCsr->pPage1 ) sqlite3PagerUnrefPageOne(pCsr->pPage1);
  sqlite3_free(pCsr);
  પાછા SQLITE_OK;
}

/*
** Move a dbpagevfs cursor to the next entry in the file.
*/
સ્થિર પૂર્ણાંક dbpageNext(sqlite3_vtab_cursor *pCursor){
  પૂર્ણાંક rc = SQLITE_OK;
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  pCsr->pgno++;
  પાછા rc;
}

સ્થિર પૂર્ણાંક dbpageEof(sqlite3_vtab_cursor *pCursor){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  પાછા pCsr->pgno > pCsr->mxPgno;
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
સ્થિર પૂર્ણાંક dbpageFilter(
  sqlite3_vtab_cursor *pCursor,
  પૂર્ણાંક idx_અંક, અચલ અક્ષર *idxStr,
  પૂર્ણાંક argc, sqlite3_કિંમત **argv
){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  DbpageTable *pTab = (DbpageTable *)pCursor->pVtab;
  પૂર્ણાંક rc;
  sqlite3 *db = pTab->db;
  Btree *pBt;

  UNUSED_PARAMETER(idxStr);
  UNUSED_PARAMETER(argc);

  /* Default setting is no rows of result */
  pCsr->pgno = 1;
  pCsr->mxPgno = 0;

  જો( idx_અંક & 2 ){
    અચલ અક્ષર *zSchema;
    પ્રતિજ્ઞા( argc>=1 );
    zSchema = (અચલ અક્ષર*)sqlite3_કિંમત_text(argv[0]);
    pCsr->iDb = sqlite3FindDbName(db, zSchema);
    જો( pCsr->iDb<0 ) પાછા SQLITE_OK;
  }નહીંતર{
    pCsr->iDb = 0;
  }
  pBt = db->aDb[pCsr->iDb].pBt;
  જો( NEVER(pBt==0) ) પાછા SQLITE_OK;
  pCsr->pPager = sqlite3BtreePager(pBt);
  pCsr->szPage = sqlite3BtreeGetPageSize(pBt);
  pCsr->mxPgno = sqlite3BtreeLastPage(pBt);
  જો( idx_અંક & 1 ){
    i64 iPg = sqlite3_કિંમત_int64(argv[idx_અંક>>1]);
    પ્રતિજ્ઞા( argc>(idx_અંક>>1) );
    જો( iPg<1 || iPg>pCsr->mxPgno ){
      pCsr->pgno = 1;
      pCsr->mxPgno = 0;
    }નહીંતર{
      pCsr->pgno = (Pgno)iPg;
      pCsr->mxPgno = pCsr->pgno;
    }
  }નહીંતર{
    પ્રતિજ્ઞા( pCsr->pgno==1 );
  }
  જો( pCsr->pPage1 ) sqlite3PagerUnrefPageOne(pCsr->pPage1);
  rc = sqlite3PagerGet(pCsr->pPager, 1, &pCsr->pPage1, 0);
  પાછા rc;
}

સ્થિર પૂર્ણાંક dbpageColumn(
  sqlite3_vtab_cursor *pCursor,
  sqlite3_context *ctx,
  પૂર્ણાંક i
){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  પૂર્ણાંક rc = SQLITE_OK;
  પસંદગી( i ){
    સ્થિતિ 0: {           /* pgno */
      sqlite3_result_int64(ctx, (sqlite3_int64)pCsr->pgno);
      તોડો;
    }
    સ્થિતિ 1: {           /* data */
      DbPage *pDbPage = 0;
      જો( pCsr->pgno==(Pgno)((PENDING_BYTE/pCsr->szPage)+1) ){
        /* The pending byte page. Assume it is zeroed out. Attempting to
        ** request this page from the page is an SQLITE_CORRUPT error. */
        sqlite3_result_zeroblob(ctx, pCsr->szPage);
      }નહીંતર{
        rc = sqlite3PagerGet(pCsr->pPager, pCsr->pgno, (DbPage**)&pDbPage, 0);
        જો( rc==SQLITE_OK ){
          sqlite3_result_blob(ctx, sqlite3PagerGetData(pDbPage), pCsr->szPage,
              SQLITE_TRANSIENT);
        }
        sqlite3PagerUnref(pDbPage);
      }
      તોડો;
    }
    મૂળભૂત: {          /* schema */
      sqlite3 *db = sqlite3_context_db_handle(ctx);
      sqlite3_result_text(ctx, db->aDb[pCsr->iDb].zDbSName, -1, SQLITE_STATIC);
      તોડો;
    }
  }
  પાછા rc;
}

સ્થિર પૂર્ણાંક dbpageRowid(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  *pRowid = pCsr->pgno;
  પાછા SQLITE_OK;
}

/* 
** Open write transactions. Since we do not know in advance which database
** files will be written by the sqlite_dbpage virtual table, start a write
** transaction on them all.
**
** Return SQLITE_OK if successful, or an SQLite error code otherwise.
*/
સ્થિર પૂર્ણાંક dbpageBeginTrans(DbpageTable *pTab){
  sqlite3 *db = pTab->db;
  પૂર્ણાંક rc = SQLITE_OK;
  પૂર્ણાંક i;
  માટે(i=0; rc==SQLITE_OK && i<db->nDb; i++){
    Btree *pBt = db->aDb[i].pBt;
    જો( pBt ) rc = sqlite3BtreeBeginTrans(pBt, 1, 0);
  }
  પાછા rc;
}

સ્થિર પૂર્ણાંક dbpageUpdate(
  sqlite3_vtab *pVtab,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv,
  sqlite_int64 *pRowid
){
  DbpageTable *pTab = (DbpageTable *)pVtab;
  Pgno pgno;
  sqlite3_int64 pgno64;
  DbPage *pDbPage = 0;
  પૂર્ણાંક rc = SQLITE_OK;
  અક્ષર *zErr = 0;
  પૂર્ણાંક iDb;
  Btree *pBt;
  Pager *pPager;
  પૂર્ણાંક szPage;
  પૂર્ણાંક is_ઉમેરો;

  (શૂન્ય)pRowid;
  જો( pTab->db->flags & SQLITE_Defensive ){
    zErr = "read-only";
    જાઓ update_fail;
  }
  જો( argc==1 ){
    zErr = "cannot delete";
    જાઓ update_fail;
  }
  જો( sqlite3_કિંમત_type(argv[0])==SQLITE_NULL ){
    pgno64 = sqlite3_કિંમત_int64(argv[2]);
    is_ઉમેરો = 1;
  }નહીંતર{
    pgno64 = (Pgno)sqlite3_કિંમત_int64(argv[0]);
    જો( sqlite3_કિંમત_int64(argv[1])!=pgno64 ){
      zErr = "cannot insert";
      જાઓ update_fail;
    }
    is_ઉમેરો = 0;
  }
  જો( sqlite3_કિંમત_type(argv[4])==SQLITE_NULL ){
    iDb = 0;
  }નહીંતર{
    અચલ અક્ષર *zSchema = (અચલ અક્ષર*)sqlite3_કિંમત_text(argv[4]);
    iDb = sqlite3FindDbName(pTab->db, zSchema);
    જો( iDb<0 ){
      zErr = "no such schema";
      જાઓ update_fail;
    }
  }
  pBt = pTab->db->aDb[iDb].pBt;
  જો( pgno64<1 || pgno64>4294967294U || NEVER(pBt==0) ){
    zErr = "bad page number";
    જાઓ update_fail;
  }
  pgno = (Pgno)pgno64;
  szPage = sqlite3BtreeGetPageSize(pBt);
  જો( sqlite3_કિંમત_type(argv[3])!=SQLITE_BLOB 
   || sqlite3_કિંમત_bytes(argv[3])!=szPage
  ){
    જો( sqlite3_કિંમત_type(argv[3])==SQLITE_NULL && is_ઉમેરો && pgno>1 ){
      /* "INSERT INTO dbpage($PGNO,NULL)" causes page number $PGNO and
      ** all subsequent pages to be deleted. */
      pTab->iDbTrunc = iDb;
      pTab->pgnoTrunc = pgno-1;
      pgno = 1;
    }નહીંતર{
      zErr = "bad page value";
      જાઓ update_fail;
    }
  }

  જો( dbpageBeginTrans(pTab)!=SQLITE_OK ){
    zErr = "failed to open transaction";
    જાઓ update_fail;
  }

  pPager = sqlite3BtreePager(pBt);
  rc = sqlite3PagerGet(pPager, pgno, (DbPage**)&pDbPage, 0);
  જો( rc==SQLITE_OK ){
    અચલ શૂન્ય *pData = sqlite3_કિંમત_blob(argv[3]);
    જો( (rc = sqlite3PagerWrite(pDbPage))==SQLITE_OK && pData ){
      unsigned અક્ષર *aPage = sqlite3PagerGetData(pDbPage);
      memcpy(aPage, pData, szPage);
      pTab->pgnoTrunc = 0;
    }
  }
  જો( rc!=SQLITE_OK ){
    pTab->pgnoTrunc = 0;
  }
  sqlite3PagerUnref(pDbPage);
  પાછા rc;

update_fail:
  pTab->pgnoTrunc = 0;
  sqlite3_free(pVtab->z_Err_સંદેશ);
  pVtab->z_Err_સંદેશ = sqlite3_mprintf("%s", zErr);
  પાછા SQLITE_ERROR;
}

સ્થિર પૂર્ણાંક dbpageBegin(sqlite3_vtab *pVtab){
  DbpageTable *pTab = (DbpageTable *)pVtab;
  pTab->pgnoTrunc = 0;
  પાછા SQLITE_OK;
}

/* Invoke sqlite3PagerTruncate() as necessary, just prior to COMMIT
*/
સ્થિર પૂર્ણાંક dbpageSync(sqlite3_vtab *pVtab){
  DbpageTable *pTab = (DbpageTable *)pVtab;
  જો( pTab->pgnoTrunc>0 ){
    Btree *pBt = pTab->db->aDb[pTab->iDbTrunc].pBt;
    Pager *pPager = sqlite3BtreePager(pBt);
    sqlite3BtreeEnter(pBt);
    જો( pTab->pgnoTrunc<sqlite3BtreeLastPage(pBt) ){
      sqlite3PagerTruncateImage(pPager, pTab->pgnoTrunc);
    }
    sqlite3BtreeLeave(pBt);
  }
  pTab->pgnoTrunc = 0;
  પાછા SQLITE_OK;
}

/* Cancel any pending truncate.
*/
સ્થિર પૂર્ણાંક dbpageRollbackTo(sqlite3_vtab *pVtab, પૂર્ણાંક notUsed1){
  DbpageTable *pTab = (DbpageTable *)pVtab;
  pTab->pgnoTrunc = 0;
  (શૂન્ય)notUsed1;
  પાછા SQLITE_OK;
}

/*
** Invoke this routine to register the "dbpage" virtual table module
*/
પૂર્ણાંક sqlite3DbpageRegister(sqlite3 *db){
  સ્થિર sqlite3_module dbpage_module = {
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
  પાછા sqlite3_create_module(db, "sqlite_dbpage", &dbpage_module, 0);
}
#અથવાજો defined(SQLITE_ENABLE_DBPAGE_VTAB)
પૂર્ણાંક sqlite3DbpageRegister(sqlite3 *db){ પાછા SQLITE_OK; }
#endif /* SQLITE_ENABLE_DBSTAT_VTAB */
