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

#অন্তর্ভাব "sqliteInt.h"   /* Requires access to internal data structures */
#যদি (defined(SQLITE_ENABLE_DBPAGE_VTAB) || defined(SQLITE_TEST)) \
    && !defined(SQLITE_OMIT_VIRTUALTABLE)

প্রকারলেখ কাঠামো DbpageTable DbpageTable;
প্রকারলেখ কাঠামো DbpageCursor DbpageCursor;

কাঠামো DbpageCursor {
  sqlite3_vtab_cursor base;       /* Base class.  Must be first */
  Pgno pgno;                      /* Current page number */
  Pgno mxPgno;                    /* Last page to visit on this scan */
  Pager *pPager;                  /* Pager being read/written */
  DbPage *pPage1;                 /* Page 1 of the database */
  পূর্ণসংখ্যা iDb;                        /* Index of database to analyze */
  পূর্ণসংখ্যা szPage;                     /* Size of each page in bytes */
};

কাঠামো DbpageTable {
  sqlite3_vtab base;              /* Base class.  Must be first */
  sqlite3 *db;                    /* The database */
  পূর্ণসংখ্যা iDbTrunc;                   /* Database to truncate */
  Pgno pgnoTrunc;                 /* Size to truncate to */
};

/* Columns */
#define DBPAGE_COLUMN_PGNO    0
#define DBPAGE_COLUMN_তথ্য    1
#define DBPAGE_COLUMN_SCHEMA  2


/*
** Connect to or create a dbpagevfs virtual table.
*/
স্থির পূর্ণসংখ্যা dbpageConnect(
  sqlite3 *db,
  শূন্য *pAux,
  পূর্ণসংখ্যা argc, ধ্রুবক অক্ষর *ধ্রুবক*argv,
  sqlite3_vtab **ppVtab,
  অক্ষর **pzErr
){
  DbpageTable *pTab = 0;
  পূর্ণসংখ্যা rc = SQLITE_OK;
  (শূন্য)pAux;
  (শূন্য)argc;
  (শূন্য)argv;
  (শূন্য)pzErr;

  sqlite3_vtab_config(db, SQLITE_VTAB_DIRECTONLY);
  sqlite3_vtab_config(db, SQLITE_VTAB_USES_ALL_SCHEMAS);
  rc = sqlite3_declare_vtab(db, 
          "CREATE TABLE x(pgno INTEGER PRIMARY KEY, data BLOB, schema HIDDEN)");
  যদি( rc==SQLITE_OK ){
    pTab = (DbpageTable *)sqlite3_malloc64(আকার(DbpageTable));
    যদি( pTab==0 ) rc = SQLITE_NOMEM_BKPT;
  }

  প্রতিজ্ঞা( rc==SQLITE_OK || pTab==0 );
  যদি( rc==SQLITE_OK ){
    memset(pTab, 0, আকার(DbpageTable));
    pTab->db = db;
  }

  *ppVtab = (sqlite3_vtab*)pTab;
  ফেরত rc;
}

/*
** Disconnect from or destroy a dbpagevfs virtual table.
*/
স্থির পূর্ণসংখ্যা dbpageDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  ফেরত SQLITE_OK;
}

/*
** idxNum:
**
**     0     schema=main, full table scan
**     1     schema=main, pgno=?1
**     2     schema=?1, full table scan
**     3     schema=?1, pgno=?2
*/
স্থির পূর্ণসংখ্যা dbpage_Best_সূচক(sqlite3_vtab *tab, sqlite3_সূচক_info *pIdxInfo){
  পূর্ণসংখ্যা i;
  পূর্ণসংখ্যা iPlan = 0;
  (শূন্য)tab;

  /* If there is a schema= constraint, it must be honored.  Report a
  ** ridiculously large estimated cost if the schema= constraint is
  ** unavailable
  */
  জন্য(i=0; i<pIdxInfo->nConstraint; i++){
    কাঠামো sqlite3_সূচক_constraint *p = &pIdxInfo->aConstraint[i];
    যদি( p->iColumn!=DBPAGE_COLUMN_SCHEMA ) চালু;
    যদি( p->op!=SQLITE_সূচক_CONSTRAINT_EQ ) চালু;
    যদি( !p->usable ){
      /* No solution. */
      ফেরত SQLITE_CONSTRAINT;
    }
    iPlan = 2;
    pIdxInfo->aConstraintUsage[i].argv_সূচক = 1;
    pIdxInfo->aConstraintUsage[i].omit = 1;
    বিরতি;
  }

  /* If we reach this point, it means that either there is no schema=
  ** constraint (in which case we use the "main" schema) or else the
  ** schema constraint was accepted.  Lower the estimated cost accordingly
  */
  pIdxInfo->estimatedCost = 1.0e6;

  /* Check for constraints against pgno */
  জন্য(i=0; i<pIdxInfo->nConstraint; i++){
    কাঠামো sqlite3_সূচক_constraint *p = &pIdxInfo->aConstraint[i];
    যদি( p->usable && p->iColumn<=0 && p->op==SQLITE_সূচক_CONSTRAINT_EQ ){
      pIdxInfo->estimatedRows = 1;
      pIdxInfo->idxFlags = SQLITE_সূচক_SCAN_UNIQUE;
      pIdxInfo->estimatedCost = 1.0;
      pIdxInfo->aConstraintUsage[i].argv_সূচক = iPlan ? 2 : 1;
      pIdxInfo->aConstraintUsage[i].omit = 1;
      iPlan |= 1;
      বিরতি;
    }
  }
  pIdxInfo->idx_অঙ্ক = iPlan;

  যদি( pIdxInfo->nOrderBy>=1
   && pIdxInfo->aOrderBy[0].iColumn<=0
   && pIdxInfo->aOrderBy[0].desc==0
  ){
    pIdxInfo->orderByConsumed = 1;
  }
  ফেরত SQLITE_OK;
}

/*
** Open a new dbpagevfs cursor.
*/
স্থির পূর্ণসংখ্যা dbpageOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  DbpageCursor *pCsr;

  pCsr = (DbpageCursor *)sqlite3_malloc64(আকার(DbpageCursor));
  যদি( pCsr==0 ){
    ফেরত SQLITE_NOMEM_BKPT;
  }অন্যথা{
    memset(pCsr, 0, আকার(DbpageCursor));
    pCsr->base.pVtab = pVTab;
    pCsr->pgno = 0;
  }

  *ppCursor = (sqlite3_vtab_cursor *)pCsr;
  ফেরত SQLITE_OK;
}

/*
** Close a dbpagevfs cursor.
*/
স্থির পূর্ণসংখ্যা dbpageClose(sqlite3_vtab_cursor *pCursor){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  যদি( pCsr->pPage1 ) sqlite3PagerUnrefPageOne(pCsr->pPage1);
  sqlite3_free(pCsr);
  ফেরত SQLITE_OK;
}

/*
** Move a dbpagevfs cursor to the next entry in the file.
*/
স্থির পূর্ণসংখ্যা dbpage_পরবর্তী(sqlite3_vtab_cursor *pCursor){
  পূর্ণসংখ্যা rc = SQLITE_OK;
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  pCsr->pgno++;
  ফেরত rc;
}

স্থির পূর্ণসংখ্যা dbpageEof(sqlite3_vtab_cursor *pCursor){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  ফেরত pCsr->pgno > pCsr->mxPgno;
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
স্থির পূর্ণসংখ্যা dbpageFilter(
  sqlite3_vtab_cursor *pCursor,
  পূর্ণসংখ্যা idx_অঙ্ক, ধ্রুবক অক্ষর *idxStr,
  পূর্ণসংখ্যা argc, sqlite3_মান **argv
){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  DbpageTable *pTab = (DbpageTable *)pCursor->pVtab;
  পূর্ণসংখ্যা rc;
  sqlite3 *db = pTab->db;
  Btree *pBt;

  UNUSED_PARAMETER(idxStr);
  UNUSED_PARAMETER(argc);

  /* Default setting is no rows of result */
  pCsr->pgno = 1;
  pCsr->mxPgno = 0;

  যদি( idx_অঙ্ক & 2 ){
    ধ্রুবক অক্ষর *zSchema;
    প্রতিজ্ঞা( argc>=1 );
    zSchema = (ধ্রুবক অক্ষর*)sqlite3_মান_text(argv[0]);
    pCsr->iDb = sqlite3FindDbName(db, zSchema);
    যদি( pCsr->iDb<0 ) ফেরত SQLITE_OK;
  }অন্যথা{
    pCsr->iDb = 0;
  }
  pBt = db->aDb[pCsr->iDb].pBt;
  যদি( NEVER(pBt==0) ) ফেরত SQLITE_OK;
  pCsr->pPager = sqlite3BtreePager(pBt);
  pCsr->szPage = sqlite3_Btree_Get_Page_আকার(pBt);
  pCsr->mxPgno = sqlite3BtreeLastPage(pBt);
  যদি( idx_অঙ্ক & 1 ){
    i64 iPg = sqlite3_মান_int64(argv[idx_অঙ্ক>>1]);
    প্রতিজ্ঞা( argc>(idx_অঙ্ক>>1) );
    যদি( iPg<1 || iPg>pCsr->mxPgno ){
      pCsr->pgno = 1;
      pCsr->mxPgno = 0;
    }অন্যথা{
      pCsr->pgno = (Pgno)iPg;
      pCsr->mxPgno = pCsr->pgno;
    }
  }অন্যথা{
    প্রতিজ্ঞা( pCsr->pgno==1 );
  }
  যদি( pCsr->pPage1 ) sqlite3PagerUnrefPageOne(pCsr->pPage1);
  rc = sqlite3PagerGet(pCsr->pPager, 1, &pCsr->pPage1, 0);
  ফেরত rc;
}

স্থির পূর্ণসংখ্যা dbpageColumn(
  sqlite3_vtab_cursor *pCursor,
  sqlite3_context *ctx,
  পূর্ণসংখ্যা i
){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  পূর্ণসংখ্যা rc = SQLITE_OK;
  নির্বাচন( i ){
    ক্ষেত্র 0: {           /* pgno */
      sqlite3_ফলাফল_int64(ctx, (sqlite3_int64)pCsr->pgno);
      বিরতি;
    }
    ক্ষেত্র 1: {           /*তথ্য*/
      DbPage *pDbPage = 0;
      যদি( pCsr->pgno==(Pgno)((PENDING_BYTE/pCsr->szPage)+1) ){
        /* The pending byte page. Assume it is zeroed out. Attempting to
        ** request this page from the page is an SQLITE_CORRUPT error. */
        sqlite3_ফলাফল_zeroblob(ctx, pCsr->szPage);
      }অন্যথা{
        rc = sqlite3PagerGet(pCsr->pPager, pCsr->pgno, (DbPage**)&pDbPage, 0);
        যদি( rc==SQLITE_OK ){
          sqlite3_ফলাফল_blob(ctx, sqlite3_Pager_Get_তথ্য(pDbPage), pCsr->szPage,
              SQLITE_TRANSIENT);
        }
        sqlite3PagerUnref(pDbPage);
      }
      বিরতি;
    }
    পূর্বনির্ধারিত: {          /* schema */
      sqlite3 *db = sqlite3_context_db_handle(ctx);
      sqlite3_ফলাফল_text(ctx, db->aDb[pCsr->iDb].zDbSName, -1, SQLITE_STATIC);
      বিরতি;
    }
  }
  ফেরত rc;
}

স্থির পূর্ণসংখ্যা dbpageRowid(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  *pRowid = pCsr->pgno;
  ফেরত SQLITE_OK;
}

/* 
** Open write transactions. Since we do not know in advance which database
** files will be written by the sqlite_dbpage virtual table, start a write
** transaction on them all.
**
** Return SQLITE_OK if successful, or an SQLite error code otherwise.
*/
স্থির পূর্ণসংখ্যা dbpageBeginTrans(DbpageTable *pTab){
  sqlite3 *db = pTab->db;
  পূর্ণসংখ্যা rc = SQLITE_OK;
  পূর্ণসংখ্যা i;
  জন্য(i=0; rc==SQLITE_OK && i<db->nDb; i++){
    Btree *pBt = db->aDb[i].pBt;
    যদি( pBt ) rc = sqlite3BtreeBeginTrans(pBt, 1, 0);
  }
  ফেরত rc;
}

স্থির পূর্ণসংখ্যা dbpageUpdate(
  sqlite3_vtab *pVtab,
  পূর্ণসংখ্যা argc,
  sqlite3_মান **argv,
  sqlite_int64 *pRowid
){
  DbpageTable *pTab = (DbpageTable *)pVtab;
  Pgno pgno;
  sqlite3_int64 pgno64;
  DbPage *pDbPage = 0;
  পূর্ণসংখ্যা rc = SQLITE_OK;
  অক্ষর *zErr = 0;
  পূর্ণসংখ্যা iDb;
  Btree *pBt;
  Pager *pPager;
  পূর্ণসংখ্যা szPage;
  পূর্ণসংখ্যা is_ঢোকাও;

  (শূন্য)pRowid;
  যদি( pTab->db->flags & SQLITE_Defensive ){
    zErr = "read-only";
    যাও update_fail;
  }
  যদি( argc==1 ){
    zErr = "cannot delete";
    যাও update_fail;
  }
  যদি( sqlite3_মান_type(argv[0])==SQLITE_NULL ){
    pgno64 = sqlite3_মান_int64(argv[2]);
    is_ঢোকাও = 1;
  }অন্যথা{
    pgno64 = (Pgno)sqlite3_মান_int64(argv[0]);
    যদি( sqlite3_মান_int64(argv[1])!=pgno64 ){
      zErr = "cannot insert";
      যাও update_fail;
    }
    is_ঢোকাও = 0;
  }
  যদি( sqlite3_মান_type(argv[4])==SQLITE_NULL ){
    iDb = 0;
  }অন্যথা{
    ধ্রুবক অক্ষর *zSchema = (ধ্রুবক অক্ষর*)sqlite3_মান_text(argv[4]);
    iDb = sqlite3FindDbName(pTab->db, zSchema);
    যদি( iDb<0 ){
      zErr = "no such schema";
      যাও update_fail;
    }
  }
  pBt = pTab->db->aDb[iDb].pBt;
  যদি( pgno64<1 || pgno64>4294967294U || NEVER(pBt==0) ){
    zErr = "bad page number";
    যাও update_fail;
  }
  pgno = (Pgno)pgno64;
  szPage = sqlite3_Btree_Get_Page_আকার(pBt);
  যদি( sqlite3_মান_type(argv[3])!=SQLITE_BLOB 
   || sqlite3_মান_bytes(argv[3])!=szPage
  ){
    যদি( sqlite3_মান_type(argv[3])==SQLITE_NULL && is_ঢোকাও && pgno>1 ){
      /* "INSERT INTO dbpage($PGNO,NULL)" causes page number $PGNO and
      ** all subsequent pages to be deleted. */
      pTab->iDbTrunc = iDb;
      pTab->pgnoTrunc = pgno-1;
      pgno = 1;
    }অন্যথা{
      zErr = "bad page value";
      যাও update_fail;
    }
  }

  যদি( dbpageBeginTrans(pTab)!=SQLITE_OK ){
    zErr = "failed to open transaction";
    যাও update_fail;
  }

  pPager = sqlite3BtreePager(pBt);
  rc = sqlite3PagerGet(pPager, pgno, (DbPage**)&pDbPage, 0);
  যদি( rc==SQLITE_OK ){
    ধ্রুবক শূন্য *p_তথ্য = sqlite3_মান_blob(argv[3]);
    যদি( (rc = sqlite3PagerWrite(pDbPage))==SQLITE_OK && p_তথ্য ){
      unsigned অক্ষর *aPage = sqlite3_Pager_Get_তথ্য(pDbPage);
      memcpy(aPage, p_তথ্য, szPage);
      pTab->pgnoTrunc = 0;
    }
  }
  যদি( rc!=SQLITE_OK ){
    pTab->pgnoTrunc = 0;
  }
  sqlite3PagerUnref(pDbPage);
  ফেরত rc;

update_fail:
  pTab->pgnoTrunc = 0;
  sqlite3_free(pVtab->z_Err_বার্তা);
  pVtab->z_Err_বার্তা = sqlite3_mprintf("%s", zErr);
  ফেরত SQLITE_ERROR;
}

স্থির পূর্ণসংখ্যা dbpageBegin(sqlite3_vtab *pVtab){
  DbpageTable *pTab = (DbpageTable *)pVtab;
  pTab->pgnoTrunc = 0;
  ফেরত SQLITE_OK;
}

/* Invoke sqlite3PagerTruncate() as necessary, just prior to COMMIT
*/
স্থির পূর্ণসংখ্যা dbpageSync(sqlite3_vtab *pVtab){
  DbpageTable *pTab = (DbpageTable *)pVtab;
  যদি( pTab->pgnoTrunc>0 ){
    Btree *pBt = pTab->db->aDb[pTab->iDbTrunc].pBt;
    Pager *pPager = sqlite3BtreePager(pBt);
    sqlite3BtreeEnter(pBt);
    যদি( pTab->pgnoTrunc<sqlite3BtreeLastPage(pBt) ){
      sqlite3PagerTruncateImage(pPager, pTab->pgnoTrunc);
    }
    sqlite3BtreeLeave(pBt);
  }
  pTab->pgnoTrunc = 0;
  ফেরত SQLITE_OK;
}

/* Cancel any pending truncate.
*/
স্থির পূর্ণসংখ্যা dbpageRollbackTo(sqlite3_vtab *pVtab, পূর্ণসংখ্যা notUsed1){
  DbpageTable *pTab = (DbpageTable *)pVtab;
  pTab->pgnoTrunc = 0;
  (শূন্য)notUsed1;
  ফেরত SQLITE_OK;
}

/*
** Invoke this routine to register the "dbpage" virtual table module
*/
পূর্ণসংখ্যা sqlite3DbpageRegister(sqlite3 *db){
  স্থির sqlite3_module dbpage_module = {
    2,                            /* iVersion */
    dbpageConnect,                /* xCreate */
    dbpageConnect,                /* xConnect */
    dbpage_Best_সূচক,              /* xBestIndex */
    dbpageDisconnect,             /* xDisconnect */
    dbpageDisconnect,             /* xDestroy */
    dbpageOpen,                   /* xOpen - open a cursor */
    dbpageClose,                  /* xClose - close a cursor */
    dbpageFilter,                 /* xFilter - configure scan constraints */
    dbpage_পরবর্তী,                   /* xNext - advance a cursor */
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
  ফেরত sqlite3_create_module(db, "sqlite_dbpage", &dbpage_module, 0);
}
#নতুবা_যদি defined(SQLITE_ENABLE_DBPAGE_VTAB)
পূর্ণসংখ্যা sqlite3DbpageRegister(sqlite3 *db){ ফেরত SQLITE_OK; }
#endif /* SQLITE_ENABLE_DBSTAT_VTAB */
