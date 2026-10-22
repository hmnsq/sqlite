/*
** 2003 April 6
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file contains code used to implement the VACUUM command.
**
** Most of the code in this file may be omitted by defining the
** SQLITE_OMIT_VACUUM macro.
*/
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"
#ಅನ್ತರ್ಭಾವ "vdbeInt.h"

#ಯದಿ !defined(SQLITE_OMIT_VACUUM) && !defined(SQLITE_OMIT_ATTACH)

/*
** Execute zSql on database db.
**
** If zSql returns rows, then each row will have exactly one
** column.  (This will only happen if zSql begins with "SELECT".)
** Take each row of result and call execSql() again recursively.
**
** The execSqlF() routine does the same thing, except it accepts
** a format string as its third argument
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ execSql(sqlite3 *db, ವರ್ಣ **pz_Err_ಸಂದೇಶ, ಧ್ರುವ ವರ್ಣ *zSql){
  sqlite3_stmt *pStmt;
  ಪೂರ್ಣಾಙ್ಕ rc;

  /* printf("SQL: [%s]\n", zSql); fflush(stdout); */
  rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
  ಯದಿ( rc!=SQLITE_OK ) ಪ್ರತ್ಯಾಗಮ rc;
  ಯಾವತ್( SQLITE_ROW==(rc = sqlite3_step(pStmt)) ){
    ಧ್ರುವ ವರ್ಣ *zSubSql = (ಧ್ರುವ ವರ್ಣ*)sqlite3_column_text(pStmt,0);
    ಪ್ರತಿಜ್ಞಾ( sqlite3_strnicmp(zSql,"SELECT",6)==0 );
    /* The secondary SQL must be one of CREATE TABLE, CREATE INDEX,
    ** or INSERT.  Historically there have been attacks that first
    ** corrupt the sqlite_schema.sql field with other kinds of statements
    ** then run VACUUM to get those statements to execute at inappropriate
    ** times. */
    ಯದಿ( zSubSql
     && (strncmp(zSubSql,"CRE",3)==0 || strncmp(zSubSql,"INS",3)==0)
    ){
      rc = execSql(db, pz_Err_ಸಂದೇಶ, zSubSql);
      ಯದಿ( rc!=SQLITE_OK ) ಭಙ್ಗ;
    }
  }
  ಪ್ರತಿಜ್ಞಾ( rc!=SQLITE_ROW );
  ಯದಿ( rc==SQLITE_DONE ) rc = SQLITE_OK;
  ಯದಿ( rc ){
    sqlite3SetString(pz_Err_ಸಂದೇಶ, db, sqlite3_errmsg(db));
  }
  (ಶೂನ್ಯ)sqlite3_finalize(pStmt);
  ಪ್ರತ್ಯಾಗಮ rc;
}
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ execSqlF(sqlite3 *db, ವರ್ಣ **pz_Err_ಸಂದೇಶ, ಧ್ರುವ ವರ್ಣ *zSql, ...){
  ವರ್ಣ *z;
  va_list ap;
  ಪೂರ್ಣಾಙ್ಕ rc;
  va_start(ap, zSql);
  z = sqlite3VMPrintf(db, zSql, ap);
  va_end(ap);
  ಯದಿ( z==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
  rc = execSql(db, pz_Err_ಸಂದೇಶ, z);
  sqlite3DbFree(db, z);
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** The VACUUM command is used to clean up the database,
** collapse free space, etc.  It is modelled after the VACUUM command
** in PostgreSQL.  The VACUUM command works as follows:
**
**   (1)  Create a new transient database file
**   (2)  Copy all content from the database being vacuumed into
**        the new transient database file
**   (3)  Copy content from the transient database back into the
**        original database.
**
** The transient database requires temporary disk space approximately
** equal to the size of the original database.  The copy operation of
** step (3) requires additional temporary disk space approximately equal
** to the size of the original database for the rollback journal.
** Hence, temporary disk space that is approximately 2x the size of the
** original database is required.  Every page of the database is written
** approximately 3 times:  Once for step (2) and twice for step (3).
** Two writes per page are required in step (3) because the original
** database content must be written into the rollback journal prior to
** overwriting the database with the vacuumed content.
**
** Only 1x temporary space and only 1x writes would be required if
** the copy of step (3) were replaced by deleting the original database
** and renaming the transient database as the original.  But that will
** not work if other processes are attached to the original database.
** And a power loss in between deleting the original and renaming the
** transient would cause the database file to appear to be deleted
** following reboot.
*/
ಶೂನ್ಯ sqlite3Vacuum(Parse *pParse, Token *pNm, ಅಭಿವ್ಯಕ್ತಿ *pInto){
  Vdbe *v = sqlite3GetVdbe(pParse);
  ಪೂರ್ಣಾಙ್ಕ iDb = 0;
  ಯದಿ( v==0 ) ಗಚ್ಛ build_vacuum_end;
  ಯದಿ( pParse->nErr ) ಗಚ್ಛ build_vacuum_end;
  ಯದಿ( pNm ){
#ifndef SQLITE_BUG_COMPATIBLE_20160819
    /* Default behavior:  Report an error if the argument to VACUUM is
    ** not recognized */
    iDb = sqlite3TwoPartName(pParse, pNm, pNm, &pNm);
    ಯದಿ( iDb<0 ) ಗಚ್ಛ build_vacuum_end;
#ಅನ್ಯಥಾ
    /* When SQLITE_BUG_COMPATIBLE_20160819 is defined, unrecognized arguments
    ** to VACUUM are silently ignored.  This is a back-out of a bug fix that
    ** occurred on 2016-08-19 (https://sqlite.org/src/info/083f9e6270).
    ** The buggy behavior is required for binary compatibility with some
    ** legacy applications. */
    iDb = sqlite3FindDb(pParse->db, pNm);
    ಯದಿ( iDb<0 ) iDb = 0;
#endif
  }
  ಯದಿ( iDb!=1 ){
    ಪೂರ್ಣಾಙ್ಕ iIntoReg = 0;
    ಯದಿ( pInto && sqlite3ResolveSelfReference(pParse,0,0,pInto,0)==0 ){
      iIntoReg = ++pParse->nMem;
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code(pParse, pInto, iIntoReg);
    }
    sqlite3VdbeAddOp2(v, OP_Vacuum, iDb, iIntoReg);
    sqlite3VdbeUsesBtree(v, iDb);
  }
build_vacuum_end:
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(pParse->db, pInto);
  ಪ್ರತ್ಯಾಗಮ;
}

/*
** This routine implements the OP_Vacuum opcode of the VDBE.
*/
SQLITE_NOINLINE ಪೂರ್ಣಾಙ್ಕ sqlite3RunVacuum(
  ವರ್ಣ **pz_Err_ಸಂದೇಶ,        /* Write error message here */
  sqlite3 *db,            /* Database connection */
  ಪೂರ್ಣಾಙ್ಕ iDb,                /* Which attached DB to vacuum */
  sqlite3_ಮೌಲ್ಯ *pOut     /* Write results here, if not NULL. VACUUM INTO */
){
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;     /* Return code from service routines */
  Btree *pMain;           /* The database being vacuumed */
  Btree *pTemp;           /* The temporary database we vacuum into */
  u32 saved_mDbFlags;     /* Saved value of db->mDbFlags */
  u64 saved_flags;        /* Saved value of db->flags */
  i64 saved_nChange;      /* Saved value of db->nChange */
  i64 saved_nTotalChange; /* Saved value of db->nTotalChange */
  u32 saved_openFlags;    /* Saved value of db->openFlags */
  u8 saved_mTrace;        /* Saved trace settings */
  Db *pDb = 0;            /* Database to detach at end of vacuum */
  ಪೂರ್ಣಾಙ್ಕ isMemDb;            /* True if vacuuming a :memory: database */
  ಪೂರ್ಣಾಙ್ಕ nRes;               /* Bytes of reserved space at the end of each page */
  ಪೂರ್ಣಾಙ್ಕ nDb;                /* Number of attached databases */
  ಧ್ರುವ ವರ್ಣ *zDbMain;    /* Schema name of database to vacuum */
  ಧ್ರುವ ವರ್ಣ *zOut;       /* Name of output file */
  u32 pgflags = PAGER_SYNCHRONOUS_OFF; /* sync flags for output db */
  u64 iRandom;            /* Random value used for zDbVacuum[] */
  ವರ್ಣ zDbVacuum[42];     /* Name of the ATTACH-ed database used for vacuum */


  ಯದಿ( !db->autoCommit ){
    sqlite3SetString(pz_Err_ಸಂದೇಶ, db, "cannot VACUUM from within a transaction");
    ಪ್ರತ್ಯಾಗಮ SQLITE_ERROR; /* IMP: R-12218-18073 */
  }
  ಯದಿ( db->nVdbeActive>1 ){
    sqlite3SetString(pz_Err_ಸಂದೇಶ, db,"cannot VACUUM - SQL statements in progress");
    ಪ್ರತ್ಯಾಗಮ SQLITE_ERROR; /* IMP: R-15610-35227 */
  }
  saved_openFlags = db->openFlags;
  ಯದಿ( pOut ){
    ಯದಿ( sqlite3_ಮೌಲ್ಯ_type(pOut)!=SQLITE_TEXT ){
      sqlite3SetString(pz_Err_ಸಂದೇಶ, db, "non-text filename");
      ಪ್ರತ್ಯಾಗಮ SQLITE_ERROR;
    }
    zOut = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(pOut);
    db->openFlags &= ~SQLITE_OPEN_READONLY;
    db->openFlags |= SQLITE_OPEN_CREATE|SQLITE_OPEN_READWRITE;
  }ಅನ್ಯಥಾ{
    zOut = "";
  }

  /* Save the current value of the database flags so that it can be 
  ** restored before returning. Then set the writable-schema flag, and
  ** disable CHECK and foreign key constraints.  */
  saved_flags = db->flags;
  saved_mDbFlags = db->mDbFlags;
  saved_nChange = db->nChange;
  saved_nTotalChange = db->nTotalChange;
  saved_mTrace = db->mTrace;
  db->flags |= SQLITE_WriteSchema | SQLITE_IgnoreChecks | SQLITE_Comments
               | SQLITE_AttachCreate | SQLITE_AttachWrite;
  db->mDbFlags |= DBFLAG_PreferBuiltin | DBFLAG_Vacuum;
  db->flags &= ~(u64)(SQLITE_Foreign_ಕೀಲಿಗಳು | SQLITE_ReverseOrder
                   | SQLITE_Defensive | SQLITE_ಎಣಿಕೆ_Rows);
  db->mTrace = 0;

  zDbMain = db->aDb[iDb].zDbSName;
  pMain = db->aDb[iDb].pBt;
  isMemDb = sqlite3PagerIsMemdb(sqlite3BtreePager(pMain));

  /* Attach the temporary database as 'vacuum_XXXXXX'. The synchronous pragma
  ** can be set to 'off' for this file, as it is not recovered if a crash
  ** occurs anyway. The integrity of the database is maintained by a
  ** (possibly synchronous) transaction opened on the main database before
  ** sqlite3BtreeCopyFile() is called.
  **
  ** An optimization would be to use a non-journaled pager.
  ** (Later:) I tried setting "PRAGMA vacuum_XXXXXX.journal_mode=OFF" but
  ** that actually made the VACUUM run slower.  Very little journalling
  ** actually occurs when doing a vacuum since the vacuum_db is initially
  ** empty.  Only the journal header is written.  Apparently it takes more
  ** time to parse and run the PRAGMA to turn journalling off than it does
  ** to write the journal header file.
  */
  sqlite3_randomness(ಪರಿಮಾಣ(iRandom),&iRandom);
  sqlite3_snprintf(ಪರಿಮಾಣ(zDbVacuum), zDbVacuum, "vacuum_%016llx", iRandom);
  nDb = db->nDb;
  rc = execSqlF(db, pz_Err_ಸಂದೇಶ, "ATTACH %Q AS %s", zOut, zDbVacuum);
  db->openFlags = saved_openFlags;
  ಯದಿ( rc!=SQLITE_OK ) ಗಚ್ಛ end_of_vacuum;
  ಪ್ರತಿಜ್ಞಾ( (db->nDb-1)==nDb );
  pDb = &db->aDb[nDb];
  ಪ್ರತಿಜ್ಞಾ( strcmp(pDb->zDbSName,zDbVacuum)==0 );
  pTemp = pDb->pBt;
  nRes = sqlite3BtreeGetRequestedReserve(pMain);
  ಯದಿ( pOut ){
    sqlite3_file *id = sqlite3PagerFile(sqlite3BtreePager(pTemp));
    i64 sz = 0;
    ಧ್ರುವ ವರ್ಣ *zFilename;
    ಯದಿ( id->pMethods!=0 && (sqlite3OsFileSize(id, &sz)!=SQLITE_OK || sz>0) ){
      rc = SQLITE_ERROR;
      sqlite3SetString(pz_Err_ಸಂದೇಶ, db, "output file already exists");
      ಗಚ್ಛ end_of_vacuum;
    }
    db->mDbFlags |= DBFLAG_VacuumInto;

    /* For a VACUUM INTO, the pager-flags are set to the same values as
    ** they are for the database being vacuumed, except that PAGER_CACHESPILL
    ** is always set. */
    pgflags = db->aDb[iDb].safety_level | (db->flags & PAGER_FLAGS_MASK);

    /* If the VACUUM INTO target file is a URI filename and if the
    ** "reserve=N" query parameter is present, reset the reserve to the
    ** amount specified, if the amount is within range */
    zFilename = sqlite3BtreeGetFilename(pTemp);
    ಯದಿ( ALWAYS(zFilename) ){
      ಪೂರ್ಣಾಙ್ಕ nNew = (ಪೂರ್ಣಾಙ್ಕ)sqlite3_uri_int64(zFilename, "reserve", nRes);
      ಯದಿ( nNew>=0 && nNew<=255 ) nRes = nNew;
    }
  }

  sqlite3BtreeSetCacheSize(pTemp, db->aDb[iDb].pSchema->cache_size);
  sqlite3BtreeSetSpillSize(pTemp, sqlite3BtreeSetSpillSize(pMain,0));
  sqlite3BtreeSetPagerFlags(pTemp, pgflags|PAGER_CACHESPILL);

  /* Begin a transaction and take an exclusive lock on the main database
  ** file. This is done before the sqlite3BtreeGetPageSize(pMain) call below,
  ** to ensure that we do not try to change the page-size on a WAL database.
  */
  rc = execSql(db, pz_Err_ಸಂದೇಶ, "BEGIN");
  ಯದಿ( rc!=SQLITE_OK ) ಗಚ್ಛ end_of_vacuum;
  rc = sqlite3BtreeBeginTrans(pMain, pOut==0 ? 2 : 0, 0);
  ಯದಿ( rc!=SQLITE_OK ) ಗಚ್ಛ end_of_vacuum;

  /* Do not attempt to change the page size for a WAL database */
  ಯದಿ( sqlite3PagerGetJournalMode(sqlite3BtreePager(pMain))
                                               ==PAGER_JOURNALMODE_WAL
   && pOut==0
  ){
    db->nextPagesize = 0;
  }

  ಯದಿ( sqlite3BtreeSetPageSize(pTemp, sqlite3BtreeGetPageSize(pMain), nRes, 0)
   || (!isMemDb && sqlite3BtreeSetPageSize(pTemp, db->nextPagesize, nRes, 0))
   || NEVER(db->mallocFailed)
  ){
    rc = SQLITE_NOMEM_BKPT;
    ಗಚ್ಛ end_of_vacuum;
  }

#ifndef SQLITE_OMIT_AUTOVACUUM
  sqlite3BtreeSetAutoVacuum(pTemp, db->nextAutovac>=0 ? db->nextAutovac :
                                           sqlite3BtreeGetAutoVacuum(pMain));
#endif

  /* Query the schema of the main database. Create a mirror schema
  ** in the temporary database.
  */
  db->init.iDb = nDb; /* force new CREATE statements into vacuum_db */
  rc = execSqlF(db, pz_Err_ಸಂದೇಶ,
      "SELECT sql FROM \"%w\".sqlite_schema"
      " WHERE type='table'AND name<>'sqlite_sequence'"
      " AND coalesce(rootpage,1)>0",
      zDbMain
  );
  ಯದಿ( rc!=SQLITE_OK ) ಗಚ್ಛ end_of_vacuum;
  rc = execSqlF(db, pz_Err_ಸಂದೇಶ,
      "SELECT sql FROM \"%w\".sqlite_schema"
      " WHERE type='index'",
      zDbMain
  );
  ಯದಿ( rc!=SQLITE_OK ) ಗಚ್ಛ end_of_vacuum;
  db->init.iDb = 0;

  /* Loop through the tables in the main database. For each, do
  ** an "INSERT INTO vacuum_db.xxx SELECT * FROM main.xxx;" to copy
  ** the contents to the temporary database.
  */
  rc = execSqlF(db, pz_Err_ಸಂದೇಶ,
      "SELECT'INSERT INTO %s.'||quote(name)"
      "||' SELECT*FROM\"%w\".'||quote(name)"
      "FROM %s.sqlite_schema "
      "WHERE type='table'AND coalesce(rootpage,1)>0",
      zDbVacuum, zDbMain, zDbVacuum
  );
  ಪ್ರತಿಜ್ಞಾ( (db->mDbFlags & DBFLAG_Vacuum)!=0 );
  db->mDbFlags &= ~DBFLAG_Vacuum;
  ಯದಿ( rc!=SQLITE_OK ) ಗಚ್ಛ end_of_vacuum;

  /* Copy the triggers, views, and virtual tables from the main database
  ** over to the temporary database.  None of these objects has any
  ** associated storage, so all we have to do is copy their entries
  ** from the schema table.
  */
  rc = execSqlF(db, pz_Err_ಸಂದೇಶ,
      "INSERT INTO %s.sqlite_schema"
      " SELECT*FROM \"%w\".sqlite_schema"
      " WHERE type IN('view','trigger')"
      " OR(type='table'AND rootpage=0)",
      zDbVacuum, zDbMain
  );
  ಯದಿ( rc ) ಗಚ್ಛ end_of_vacuum;

  /* At this point, there is a write transaction open on both the 
  ** vacuum database and the main database. Assuming no error occurs,
  ** both transactions are closed by this block - the main database
  ** transaction by sqlite3BtreeCopyFile() and the other by an explicit
  ** call to sqlite3BtreeCommit().
  */
  {
    u32 meta;
    ಪೂರ್ಣಾಙ್ಕ i;

    /* This array determines which meta meta values are preserved in the
    ** vacuum.  Even entries are the meta value number and odd entries
    ** are an increment to apply to the meta value after the vacuum.
    ** The increment is used to increase the schema cookie so that other
    ** connections to the same database will know to reread the schema.
    */
    ಸ್ಥಿರ ಧ್ರುವ unsigned ವರ್ಣ aCopy[] = {
       BTREE_SCHEMA_VERSION,     1,  /* Add one to the old schema cookie */
       BTREE_DEFAULT_CACHE_SIZE, 0,  /* Preserve the default page cache size */
       BTREE_TEXT_ENCODING,      0,  /* Preserve the text encoding */
       BTREE_USER_VERSION,       0,  /* Preserve the user version */
       BTREE_APPLICATION_ID,     0,  /* Preserve the application id */
    };

    ಪ್ರತಿಜ್ಞಾ( SQLITE_TXN_WRITE==sqlite3BtreeTxnState(pTemp) );
    ಪ್ರತಿಜ್ಞಾ( pOut!=0 || SQLITE_TXN_WRITE==sqlite3BtreeTxnState(pMain) );

    /* Copy Btree meta values */
    ಪ್ರತಿ(i=0; i<ArraySize(aCopy); i+=2){
      /* GetMeta() and UpdateMeta() cannot fail in this context because
      ** we already have page 1 loaded into cache and marked dirty. */
      sqlite3BtreeGetMeta(pMain, aCopy[i], &meta);
      rc = sqlite3BtreeUpdateMeta(pTemp, aCopy[i], meta+aCopy[i+1]);
      ಯದಿ( NEVER(rc!=SQLITE_OK) ) ಗಚ್ಛ end_of_vacuum;
    }

    ಯದಿ( pOut==0 ){
      rc = sqlite3BtreeCopyFile(pMain, pTemp);
    }
    ಯದಿ( rc!=SQLITE_OK ) ಗಚ್ಛ end_of_vacuum;
    rc = sqlite3BtreeCommit(pTemp);
    ಯದಿ( rc!=SQLITE_OK ) ಗಚ್ಛ end_of_vacuum;
#ifndef SQLITE_OMIT_AUTOVACUUM
    ಯದಿ( pOut==0 ){
      sqlite3BtreeSetAutoVacuum(pMain, sqlite3BtreeGetAutoVacuum(pTemp));
    }
#endif
  }

  ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_OK );
  ಯದಿ( pOut==0 ){
    nRes = sqlite3BtreeGetRequestedReserve(pTemp);
    rc = sqlite3BtreeSetPageSize(pMain, sqlite3BtreeGetPageSize(pTemp), nRes,1);
  }

end_of_vacuum:
  /* Restore the original value of db->flags */
  db->init.iDb = 0;
  db->mDbFlags = saved_mDbFlags;
  db->flags = saved_flags;
  db->nChange = saved_nChange;
  db->nTotalChange = saved_nTotalChange;
  db->mTrace = saved_mTrace;
  sqlite3BtreeSetPageSize(pMain, -1, 0, 1);

  /* Currently there is an SQL level transaction open on the vacuum
  ** database. No locks are held on any other files (since the main file
  ** was committed at the btree level). So it safe to end the transaction
  ** by manually setting the autoCommit flag to true and detaching the
  ** vacuum database. The vacuum_db journal file is deleted when the pager
  ** is closed by the DETACH.
  */
  db->autoCommit = 1;

  ಯದಿ( pDb ){
    sqlite3BtreeClose(pDb->pBt);
    pDb->pBt = 0;
    pDb->pSchema = 0;
  }

  /* This both clears the schemas and reduces the size of the db->aDb[]
  ** array. */ 
  sqlite3ResetAllSchemasOfConnection(db);

  ಪ್ರತ್ಯಾಗಮ rc;
}

#endif  /* SQLITE_OMIT_VACUUM && SQLITE_OMIT_ATTACH */
