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

#अन्तर्भाव "sqliteInt.h"   /* Requires access to internal data structures */
#यदि (defined(SQLITE_ENABLE_DBPAGE_VTAB) || defined(SQLITE_TEST)) \
    && !defined(SQLITE_OMIT_VIRTUALTABLE)

प्रकारनिर्देश संरचना DbpageTable DbpageTable;
प्रकारनिर्देश संरचना DbpageCursor DbpageCursor;

संरचना DbpageCursor {
  sqlite3_vtab_cursor base;       /* Base class.  Must be first */
  Pgno pgno;                      /* Current page number */
  Pgno mxPgno;                    /* Last page to visit on this scan */
  Pager *pPager;                  /* Pager being read/written */
  DbPage *pPage1;                 /* Page 1 of the database */
  पूर्णाङ्क iDb;                        /* Index of database to analyze */
  पूर्णाङ्क szPage;                     /* Size of each page in bytes */
};

संरचना DbpageTable {
  sqlite3_vtab base;              /* Base class.  Must be first */
  sqlite3 *db;                    /* The database */
  पूर्णाङ्क iDbTrunc;                   /* Database to truncate */
  Pgno pgnoTrunc;                 /* Size to truncate to */
};

/* Columns */
#define DBPAGE_COLUMN_PGNO    0
#define DBPAGE_COLUMN_DATA    1
#define DBPAGE_COLUMN_SCHEMA  2


/*
** Connect to or create a dbpagevfs virtual table.
*/
स्थिर पूर्णाङ्क dbpageConnect(
  sqlite3 *db,
  शून्य *pAux,
  पूर्णाङ्क argc, ध्रुव वर्ण *ध्रुव*argv,
  sqlite3_vtab **ppVtab,
  वर्ण **pzErr
){
  DbpageTable *pTab = 0;
  पूर्णाङ्क rc = SQLITE_OK;
  (शून्य)pAux;
  (शून्य)argc;
  (शून्य)argv;
  (शून्य)pzErr;

  sqlite3_vtab_config(db, SQLITE_VTAB_DIRECTONLY);
  sqlite3_vtab_config(db, SQLITE_VTAB_USES_ALL_SCHEMAS);
  rc = sqlite3_declare_vtab(db, 
          "CREATE TABLE x(pgno INTEGER PRIMARY KEY, data BLOB, schema HIDDEN)");
  यदि( rc==SQLITE_OK ){
    pTab = (DbpageTable *)sqlite3_malloc64(परिमाण(DbpageTable));
    यदि( pTab==0 ) rc = SQLITE_NOMEM_BKPT;
  }

  प्रतिज्ञा( rc==SQLITE_OK || pTab==0 );
  यदि( rc==SQLITE_OK ){
    memset(pTab, 0, परिमाण(DbpageTable));
    pTab->db = db;
  }

  *ppVtab = (sqlite3_vtab*)pTab;
  प्रत्यागम rc;
}

/*
** Disconnect from or destroy a dbpagevfs virtual table.
*/
स्थिर पूर्णाङ्क dbpageDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  प्रत्यागम SQLITE_OK;
}

/*
** idxNum:
**
**     0     schema=main, full table scan
**     1     schema=main, pgno=?1
**     2     schema=?1, full table scan
**     3     schema=?1, pgno=?2
*/
स्थिर पूर्णाङ्क dbpageBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  पूर्णाङ्क i;
  पूर्णाङ्क iPlan = 0;
  (शून्य)tab;

  /* If there is a schema= constraint, it must be honored.  Report a
  ** ridiculously large estimated cost if the schema= constraint is
  ** unavailable
  */
  प्रति(i=0; i<pIdxInfo->nConstraint; i++){
    संरचना sqlite3_index_constraint *p = &pIdxInfo->aConstraint[i];
    यदि( p->iColumn!=DBPAGE_COLUMN_SCHEMA ) अनुवर्त;
    यदि( p->op!=SQLITE_INDEX_CONSTRAINT_EQ ) अनुवर्त;
    यदि( !p->usable ){
      /* No solution. */
      प्रत्यागम SQLITE_CONSTRAINT;
    }
    iPlan = 2;
    pIdxInfo->aConstraintUsage[i].argvIndex = 1;
    pIdxInfo->aConstraintUsage[i].omit = 1;
    भङ्ग;
  }

  /* If we reach this point, it means that either there is no schema=
  ** constraint (in which case we use the "main" schema) or else the
  ** schema constraint was accepted.  Lower the estimated cost accordingly
  */
  pIdxInfo->estimatedCost = 1.0e6;

  /* Check for constraints against pgno */
  प्रति(i=0; i<pIdxInfo->nConstraint; i++){
    संरचना sqlite3_index_constraint *p = &pIdxInfo->aConstraint[i];
    यदि( p->usable && p->iColumn<=0 && p->op==SQLITE_INDEX_CONSTRAINT_EQ ){
      pIdxInfo->estimatedRows = 1;
      pIdxInfo->idxFlags = SQLITE_INDEX_SCAN_UNIQUE;
      pIdxInfo->estimatedCost = 1.0;
      pIdxInfo->aConstraintUsage[i].argvIndex = iPlan ? 2 : 1;
      pIdxInfo->aConstraintUsage[i].omit = 1;
      iPlan |= 1;
      भङ्ग;
    }
  }
  pIdxInfo->idx_अंक = iPlan;

  यदि( pIdxInfo->nOrderBy>=1
   && pIdxInfo->aOrderBy[0].iColumn<=0
   && pIdxInfo->aOrderBy[0].desc==0
  ){
    pIdxInfo->orderByConsumed = 1;
  }
  प्रत्यागम SQLITE_OK;
}

/*
** Open a new dbpagevfs cursor.
*/
स्थिर पूर्णाङ्क dbpageOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  DbpageCursor *pCsr;

  pCsr = (DbpageCursor *)sqlite3_malloc64(परिमाण(DbpageCursor));
  यदि( pCsr==0 ){
    प्रत्यागम SQLITE_NOMEM_BKPT;
  }अन्यथा{
    memset(pCsr, 0, परिमाण(DbpageCursor));
    pCsr->base.pVtab = pVTab;
    pCsr->pgno = 0;
  }

  *ppCursor = (sqlite3_vtab_cursor *)pCsr;
  प्रत्यागम SQLITE_OK;
}

/*
** Close a dbpagevfs cursor.
*/
स्थिर पूर्णाङ्क dbpageClose(sqlite3_vtab_cursor *pCursor){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  यदि( pCsr->pPage1 ) sqlite3PagerUnrefPageOne(pCsr->pPage1);
  sqlite3_free(pCsr);
  प्रत्यागम SQLITE_OK;
}

/*
** Move a dbpagevfs cursor to the next entry in the file.
*/
स्थिर पूर्णाङ्क dbpageNext(sqlite3_vtab_cursor *pCursor){
  पूर्णाङ्क rc = SQLITE_OK;
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  pCsr->pgno++;
  प्रत्यागम rc;
}

स्थिर पूर्णाङ्क dbpageEof(sqlite3_vtab_cursor *pCursor){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  प्रत्यागम pCsr->pgno > pCsr->mxPgno;
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
स्थिर पूर्णाङ्क dbpageFilter(
  sqlite3_vtab_cursor *pCursor,
  पूर्णाङ्क idx_अंक, ध्रुव वर्ण *idxStr,
  पूर्णाङ्क argc, sqlite3_मूल्य **argv
){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  DbpageTable *pTab = (DbpageTable *)pCursor->pVtab;
  पूर्णाङ्क rc;
  sqlite3 *db = pTab->db;
  Btree *pBt;

  UNUSED_PARAMETER(idxStr);
  UNUSED_PARAMETER(argc);

  /* Default setting is no rows of result */
  pCsr->pgno = 1;
  pCsr->mxPgno = 0;

  यदि( idx_अंक & 2 ){
    ध्रुव वर्ण *zSchema;
    प्रतिज्ञा( argc>=1 );
    zSchema = (ध्रुव वर्ण*)sqlite3_मूल्य_text(argv[0]);
    pCsr->iDb = sqlite3FindDbName(db, zSchema);
    यदि( pCsr->iDb<0 ) प्रत्यागम SQLITE_OK;
  }अन्यथा{
    pCsr->iDb = 0;
  }
  pBt = db->aDb[pCsr->iDb].pBt;
  यदि( NEVER(pBt==0) ) प्रत्यागम SQLITE_OK;
  pCsr->pPager = sqlite3BtreePager(pBt);
  pCsr->szPage = sqlite3BtreeGetPageSize(pBt);
  pCsr->mxPgno = sqlite3BtreeLastPage(pBt);
  यदि( idx_अंक & 1 ){
    i64 iPg = sqlite3_मूल्य_int64(argv[idx_अंक>>1]);
    प्रतिज्ञा( argc>(idx_अंक>>1) );
    यदि( iPg<1 || iPg>pCsr->mxPgno ){
      pCsr->pgno = 1;
      pCsr->mxPgno = 0;
    }अन्यथा{
      pCsr->pgno = (Pgno)iPg;
      pCsr->mxPgno = pCsr->pgno;
    }
  }अन्यथा{
    प्रतिज्ञा( pCsr->pgno==1 );
  }
  यदि( pCsr->pPage1 ) sqlite3PagerUnrefPageOne(pCsr->pPage1);
  rc = sqlite3PagerGet(pCsr->pPager, 1, &pCsr->pPage1, 0);
  प्रत्यागम rc;
}

स्थिर पूर्णाङ्क dbpageColumn(
  sqlite3_vtab_cursor *pCursor,
  sqlite3_context *ctx,
  पूर्णाङ्क i
){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  पूर्णाङ्क rc = SQLITE_OK;
  विकल्प( i ){
    पक्ष 0: {           /* pgno */
      sqlite3_result_int64(ctx, (sqlite3_int64)pCsr->pgno);
      भङ्ग;
    }
    पक्ष 1: {           /* data */
      DbPage *pDbPage = 0;
      यदि( pCsr->pgno==(Pgno)((PENDING_BYTE/pCsr->szPage)+1) ){
        /* The pending byte page. Assume it is zeroed out. Attempting to
        ** request this page from the page is an SQLITE_CORRUPT error. */
        sqlite3_result_zeroblob(ctx, pCsr->szPage);
      }अन्यथा{
        rc = sqlite3PagerGet(pCsr->pPager, pCsr->pgno, (DbPage**)&pDbPage, 0);
        यदि( rc==SQLITE_OK ){
          sqlite3_result_blob(ctx, sqlite3PagerGetData(pDbPage), pCsr->szPage,
              SQLITE_TRANSIENT);
        }
        sqlite3PagerUnref(pDbPage);
      }
      भङ्ग;
    }
    स्वभाव: {          /* schema */
      sqlite3 *db = sqlite3_context_db_handle(ctx);
      sqlite3_result_text(ctx, db->aDb[pCsr->iDb].zDbSName, -1, SQLITE_STATIC);
      भङ्ग;
    }
  }
  प्रत्यागम rc;
}

स्थिर पूर्णाङ्क dbpageRowid(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid){
  DbpageCursor *pCsr = (DbpageCursor *)pCursor;
  *pRowid = pCsr->pgno;
  प्रत्यागम SQLITE_OK;
}

/* 
** Open write transactions. Since we do not know in advance which database
** files will be written by the sqlite_dbpage virtual table, start a write
** transaction on them all.
**
** Return SQLITE_OK if successful, or an SQLite error code otherwise.
*/
स्थिर पूर्णाङ्क dbpageBeginTrans(DbpageTable *pTab){
  sqlite3 *db = pTab->db;
  पूर्णाङ्क rc = SQLITE_OK;
  पूर्णाङ्क i;
  प्रति(i=0; rc==SQLITE_OK && i<db->nDb; i++){
    Btree *pBt = db->aDb[i].pBt;
    यदि( pBt ) rc = sqlite3BtreeBeginTrans(pBt, 1, 0);
  }
  प्रत्यागम rc;
}

स्थिर पूर्णाङ्क dbpageUpdate(
  sqlite3_vtab *pVtab,
  पूर्णाङ्क argc,
  sqlite3_मूल्य **argv,
  sqlite_int64 *pRowid
){
  DbpageTable *pTab = (DbpageTable *)pVtab;
  Pgno pgno;
  sqlite3_int64 pgno64;
  DbPage *pDbPage = 0;
  पूर्णाङ्क rc = SQLITE_OK;
  वर्ण *zErr = 0;
  पूर्णाङ्क iDb;
  Btree *pBt;
  Pager *pPager;
  पूर्णाङ्क szPage;
  पूर्णाङ्क is_घाला;

  (शून्य)pRowid;
  यदि( pTab->db->flags & SQLITE_Defensive ){
    zErr = "read-only";
    गच्छ update_fail;
  }
  यदि( argc==1 ){
    zErr = "cannot delete";
    गच्छ update_fail;
  }
  यदि( sqlite3_मूल्य_type(argv[0])==SQLITE_NULL ){
    pgno64 = sqlite3_मूल्य_int64(argv[2]);
    is_घाला = 1;
  }अन्यथा{
    pgno64 = (Pgno)sqlite3_मूल्य_int64(argv[0]);
    यदि( sqlite3_मूल्य_int64(argv[1])!=pgno64 ){
      zErr = "cannot insert";
      गच्छ update_fail;
    }
    is_घाला = 0;
  }
  यदि( sqlite3_मूल्य_type(argv[4])==SQLITE_NULL ){
    iDb = 0;
  }अन्यथा{
    ध्रुव वर्ण *zSchema = (ध्रुव वर्ण*)sqlite3_मूल्य_text(argv[4]);
    iDb = sqlite3FindDbName(pTab->db, zSchema);
    यदि( iDb<0 ){
      zErr = "no such schema";
      गच्छ update_fail;
    }
  }
  pBt = pTab->db->aDb[iDb].pBt;
  यदि( pgno64<1 || pgno64>4294967294U || NEVER(pBt==0) ){
    zErr = "bad page number";
    गच्छ update_fail;
  }
  pgno = (Pgno)pgno64;
  szPage = sqlite3BtreeGetPageSize(pBt);
  यदि( sqlite3_मूल्य_type(argv[3])!=SQLITE_BLOB 
   || sqlite3_मूल्य_bytes(argv[3])!=szPage
  ){
    यदि( sqlite3_मूल्य_type(argv[3])==SQLITE_NULL && is_घाला && pgno>1 ){
      /* "INSERT INTO dbpage($PGNO,NULL)" causes page number $PGNO and
      ** all subsequent pages to be deleted. */
      pTab->iDbTrunc = iDb;
      pTab->pgnoTrunc = pgno-1;
      pgno = 1;
    }अन्यथा{
      zErr = "bad page value";
      गच्छ update_fail;
    }
  }

  यदि( dbpageBeginTrans(pTab)!=SQLITE_OK ){
    zErr = "failed to open transaction";
    गच्छ update_fail;
  }

  pPager = sqlite3BtreePager(pBt);
  rc = sqlite3PagerGet(pPager, pgno, (DbPage**)&pDbPage, 0);
  यदि( rc==SQLITE_OK ){
    ध्रुव शून्य *pData = sqlite3_मूल्य_blob(argv[3]);
    यदि( (rc = sqlite3PagerWrite(pDbPage))==SQLITE_OK && pData ){
      unsigned वर्ण *aPage = sqlite3PagerGetData(pDbPage);
      memcpy(aPage, pData, szPage);
      pTab->pgnoTrunc = 0;
    }
  }
  यदि( rc!=SQLITE_OK ){
    pTab->pgnoTrunc = 0;
  }
  sqlite3PagerUnref(pDbPage);
  प्रत्यागम rc;

update_fail:
  pTab->pgnoTrunc = 0;
  sqlite3_free(pVtab->z_Err_संदेश);
  pVtab->z_Err_संदेश = sqlite3_mprintf("%s", zErr);
  प्रत्यागम SQLITE_ERROR;
}

स्थिर पूर्णाङ्क dbpageBegin(sqlite3_vtab *pVtab){
  DbpageTable *pTab = (DbpageTable *)pVtab;
  pTab->pgnoTrunc = 0;
  प्रत्यागम SQLITE_OK;
}

/* Invoke sqlite3PagerTruncate() as necessary, just prior to COMMIT
*/
स्थिर पूर्णाङ्क dbpageSync(sqlite3_vtab *pVtab){
  DbpageTable *pTab = (DbpageTable *)pVtab;
  यदि( pTab->pgnoTrunc>0 ){
    Btree *pBt = pTab->db->aDb[pTab->iDbTrunc].pBt;
    Pager *pPager = sqlite3BtreePager(pBt);
    sqlite3BtreeEnter(pBt);
    यदि( pTab->pgnoTrunc<sqlite3BtreeLastPage(pBt) ){
      sqlite3PagerTruncateImage(pPager, pTab->pgnoTrunc);
    }
    sqlite3BtreeLeave(pBt);
  }
  pTab->pgnoTrunc = 0;
  प्रत्यागम SQLITE_OK;
}

/* Cancel any pending truncate.
*/
स्थिर पूर्णाङ्क dbpageRollbackTo(sqlite3_vtab *pVtab, पूर्णाङ्क notUsed1){
  DbpageTable *pTab = (DbpageTable *)pVtab;
  pTab->pgnoTrunc = 0;
  (शून्य)notUsed1;
  प्रत्यागम SQLITE_OK;
}

/*
** Invoke this routine to register the "dbpage" virtual table module
*/
पूर्णाङ्क sqlite3DbpageRegister(sqlite3 *db){
  स्थिर sqlite3_module dbpage_module = {
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
  प्रत्यागम sqlite3_create_module(db, "sqlite_dbpage", &dbpage_module, 0);
}
#नोचेद्यदि defined(SQLITE_ENABLE_DBPAGE_VTAB)
पूर्णाङ्क sqlite3DbpageRegister(sqlite3 *db){ प्रत्यागम SQLITE_OK; }
#endif /* SQLITE_ENABLE_DBSTAT_VTAB */
