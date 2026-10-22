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

#ಆದರೆ !defined(SQLITE_OMIT_VACUUM) && !defined(SQLITE_OMIT_ATTACH)

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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ execSql(sqlite3 *db, ಅಕ್ಷರ **pz_Err_ಸಂದೇಶ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSql){
  sqlite3_stmt *pStmt;
  ಪೂರ್ಣಾಂಕ rc;

  /* printf("SQL: [%s]\n", zSql); fflush(stdout); */
  rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
  ಆದರೆ( rc!=SQLITE_OK ) ಹಿಂತಿರುಗಿ rc;
  ಆಗ( SQLITE_ಅಡ್ಡಸಾಲು==(rc = sqlite3_ಹೆಜ್ಜೆ(pStmt)) ){
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSubSql = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಸ್ತಂಭ_ಪಠ್ಯ(pStmt,0);
    ಪ್ರತಿಜ್ಞಾ( sqlite3_strnicmp(zSql,"SELECT",6)==0 );
    /* The secondary SQL must be one of CREATE TABLE, CREATE INDEX,
    ** or INSERT.  Historically there have been attacks that first
    ** corrupt the sqlite_schema.sql field with other kinds of statements
    ** then run VACUUM to get those statements to execute at inappropriate
    ** times. */
    ಆದರೆ( zSubSql
     && (ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(zSubSql,"CRE",3)==0 || ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(zSubSql,"INS",3)==0)
    ){
      rc = execSql(db, pz_Err_ಸಂದೇಶ, zSubSql);
      ಆದರೆ( rc!=SQLITE_OK ) ನಿಲ್ಲಿಸು;
    }
  }
  ಪ್ರತಿಜ್ಞಾ( rc!=SQLITE_ಅಡ್ಡಸಾಲು );
  ಆದರೆ( rc==SQLITE_ಮುಗಿದ ) rc = SQLITE_OK;
  ಆದರೆ( rc ){
    sqlite3_ಗಣ_String(pz_Err_ಸಂದೇಶ, db, sqlite3_errmsg(db));
  }
  (ಶೂನ್ಯ)sqlite3_finalize(pStmt);
  ಹಿಂತಿರುಗಿ rc;
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ execSqlF(sqlite3 *db, ಅಕ್ಷರ **pz_Err_ಸಂದೇಶ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSql, ...){
  ಅಕ್ಷರ *z;
  va_list ap;
  ಪೂರ್ಣಾಂಕ rc;
  va_start(ap, zSql);
  z = sqlite3VMPrintf(db, zSql, ap);
  va_end(ap);
  ಆದರೆ( z==0 ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
  rc = execSql(db, pz_Err_ಸಂದೇಶ, z);
  sqlite3DbFree(db, z);
  ಹಿಂತಿರುಗಿ rc;
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
  ಪೂರ್ಣಾಂಕ iDb = 0;
  ಆದರೆ( v==0 ) ಹೋಗು build_vacuum_ಕೊನೆ;
  ಆದರೆ( pParse->nErr ) ಹೋಗು build_vacuum_ಕೊನೆ;
  ಆದರೆ( pNm ){
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_BUG_COMPATIBLE_20160819
    /* Default behavior:  Report an error if the argument to VACUUM is
    ** not recognized */
    iDb = sqlite3TwoPartName(pParse, pNm, pNm, &pNm);
    ಆದರೆ( iDb<0 ) ಹೋಗು build_vacuum_ಕೊನೆ;
#ಇಲ್ಲದಿದ್ದರೆ
    /* When SQLITE_BUG_COMPATIBLE_20160819 is defined, unrecognized arguments
    ** to VACUUM are silently ignored.  This is a back-out of a bug fix that
    ** occurred on 2016-08-19 (https://sqlite.org/src/info/083f9e6270).
    ** The buggy behavior is required for binary compatibility with some
    ** legacy applications. */
    iDb = sqlite3_ಕಂಡುಹಿಡಿ_Db(pParse->db, pNm);
    ಆದರೆ( iDb<0 ) iDb = 0;
#ಆದರೆ_ಅಂತ್ಯ
  }
  ಆದರೆ( iDb!=1 ){
    ಪೂರ್ಣಾಂಕ iIntoReg = 0;
    ಆದರೆ( pInto && sqlite3ResolveSelfReference(pParse,0,0,pInto,0)==0 ){
      iIntoReg = ++pParse->nMem;
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ(pParse, pInto, iIntoReg);
    }
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Vacuum, iDb, iIntoReg);
    sqlite3_Vdbe_ಬಳಸುತ್ತದೆ_Btree(v, iDb);
  }
build_vacuum_ಕೊನೆ:
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(pParse->db, pInto);
  ಹಿಂತಿರುಗಿ;
}

/*
** This routine implements the OP_Vacuum opcode of the VDBE.
*/
SQLITE_NOINLINE ಪೂರ್ಣಾಂಕ sqlite3_ಸತತ_Vacuum(
  ಅಕ್ಷರ **pz_Err_ಸಂದೇಶ,        /* Write error message here */
  sqlite3 *db,            /* Database connection */
  ಪೂರ್ಣಾಂಕ iDb,                /* Which attached DB to vacuum */
  sqlite3_ಮೌಲ್ಯ *p_ಹೊರಹೋಗು     /* Write results here, if not NULL. VACUUM INTO */
){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;     /* Return code from service routines */
  Btree *pMain;           /* The database being vacuumed */
  Btree *p_ತಾತ್ಕಾಲಿಕ;           /* The temporary database we vacuum into */
  u32 saved_mDbFlags;     /* Saved value of db->mDbFlags */
  u64 saved_flags;        /* Saved value of db->flags */
  i64 saved_nChange;      /* Saved value of db->nChange */
  i64 saved_n_ಒಟ್ಟು_Change; /* Saved value of db->nTotalChange */
  u32 saved_openFlags;    /* Saved value of db->openFlags */
  u8 saved_mTrace;        /* Saved trace settings */
  Db *pDb = 0;            /* Database to detach at end of vacuum */
  ಪೂರ್ಣಾಂಕ isMemDb;            /* True if vacuuming a :memory: database */
  ಪೂರ್ಣಾಂಕ nRes;               /* Bytes of reserved space at the end of each page */
  ಪೂರ್ಣಾಂಕ nDb;                /* Number of attached databases */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDbMain;    /* Schema name of database to vacuum */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಹೊರಹೋಗು;       /* Name of output file */
  u32 pgflags = PAGER_SYNCHRONOUS_OFF; /* sync flags for output db */
  u64 iRandom;            /* Random value used for zDbVacuum[] */
  ಅಕ್ಷರ zDbVacuum[42];     /* Name of the ATTACH-ed database used for vacuum */


  ಆದರೆ( !db->autoCommit ){
    sqlite3_ಗಣ_String(pz_Err_ಸಂದೇಶ, db, "cannot VACUUM from within a transaction");
    ಹಿಂತಿರುಗಿ SQLITE_ERROR; /* IMP: R-12218-18073 */
  }
  ಆದರೆ( db->n_Vdbe_ಸಕ್ರಿಯ>1 ){
    sqlite3_ಗಣ_String(pz_Err_ಸಂದೇಶ, db,"cannot VACUUM - SQL statements in progress");
    ಹಿಂತಿರುಗಿ SQLITE_ERROR; /* IMP: R-15610-35227 */
  }
  saved_openFlags = db->openFlags;
  ಆದರೆ( p_ಹೊರಹೋಗು ){
    ಆದರೆ( sqlite3_ಮೌಲ್ಯ_type(p_ಹೊರಹೋಗು)!=SQLITE_ಪಠ್ಯ ){
      sqlite3_ಗಣ_String(pz_Err_ಸಂದೇಶ, db, "non-text filename");
      ಹಿಂತಿರುಗಿ SQLITE_ERROR;
    }
    z_ಹೊರಹೋಗು = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(p_ಹೊರಹೋಗು);
    db->openFlags &= ~SQLITE_OPEN_READONLY;
    db->openFlags |= SQLITE_OPEN_CREATE|SQLITE_OPEN_READWRITE;
  }ಇಲ್ಲದಿದ್ದರೆ{
    z_ಹೊರಹೋಗು = "";
  }

  /* Save the current value of the database flags so that it can be 
  ** restored before returning. Then set the writable-schema flag, and
  ** disable CHECK and foreign key constraints.  */
  saved_flags = db->flags;
  saved_mDbFlags = db->mDbFlags;
  saved_nChange = db->nChange;
  saved_n_ಒಟ್ಟು_Change = db->n_ಒಟ್ಟು_Change;
  saved_mTrace = db->mTrace;
  db->flags |= SQLITE_WriteSchema | SQLITE_IgnoreChecks | SQLITE_Comments
               | SQLITE_AttachCreate | SQLITE_AttachWrite;
  db->mDbFlags |= DBFLAG_PreferBuiltin | DBFLAG_Vacuum;
  db->flags &= ~(u64)(SQLITE_Foreign_ಕೀಲಿಗಳು | SQLITE_ಹಿಮ್ಮುಖ_ಕ್ರಮ
                   | SQLITE_Defensive | SQLITE_ಎಣಿಕೆ_ಅಡ್ಡಸಾಲುಗಳು);
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
  sqlite3_randomness(ಗಾತ್ರ(iRandom),&iRandom);
  sqlite3_snprintf(ಗಾತ್ರ(zDbVacuum), zDbVacuum, "vacuum_%016llx", iRandom);
  nDb = db->nDb;
  rc = execSqlF(db, pz_Err_ಸಂದೇಶ, "ATTACH %Q AS %s", z_ಹೊರಹೋಗು, zDbVacuum);
  db->openFlags = saved_openFlags;
  ಆದರೆ( rc!=SQLITE_OK ) ಹೋಗು ಕೊನೆ_ದ_vacuum;
  ಪ್ರತಿಜ್ಞಾ( (db->nDb-1)==nDb );
  pDb = &db->aDb[nDb];
  ಪ್ರತಿಜ್ಞಾ( strcmp(pDb->zDbSName,zDbVacuum)==0 );
  p_ತಾತ್ಕಾಲಿಕ = pDb->pBt;
  nRes = sqlite3BtreeGetRequestedReserve(pMain);
  ಆದರೆ( p_ಹೊರಹೋಗು ){
    sqlite3_file *id = sqlite3PagerFile(sqlite3BtreePager(p_ತಾತ್ಕಾಲಿಕ));
    i64 sz = 0;
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFilename;
    ಆದರೆ( id->pMethods!=0 && (sqlite3_Os_File_ಗಾತ್ರ(id, &sz)!=SQLITE_OK || sz>0) ){
      rc = SQLITE_ERROR;
      sqlite3_ಗಣ_String(pz_Err_ಸಂದೇಶ, db, "output file already exists");
      ಹೋಗು ಕೊನೆ_ದ_vacuum;
    }
    db->mDbFlags |= DBFLAG_VacuumInto;

    /* For a VACUUM INTO, the pager-flags are set to the same values as
    ** they are for the database being vacuumed, except that PAGER_CACHESPILL
    ** is always set. */
    pgflags = db->aDb[iDb].safety_level | (db->flags & PAGER_FLAGS_ಮುಖವಾಡ);

    /* If the VACUUM INTO target file is a URI filename and if the
    ** "reserve=N" query parameter is present, reset the reserve to the
    ** amount specified, if the amount is within range */
    zFilename = sqlite3BtreeGetFilename(p_ತಾತ್ಕಾಲಿಕ);
    ಆದರೆ( ALWAYS(zFilename) ){
      ಪೂರ್ಣಾಂಕ nNew = (ಪೂರ್ಣಾಂಕ)sqlite3_uri_int64(zFilename, "reserve", nRes);
      ಆದರೆ( nNew>=0 && nNew<=255 ) nRes = nNew;
    }
  }

  sqlite3_Btree_ಗಣ_Cache_ಗಾತ್ರ(p_ತಾತ್ಕಾಲಿಕ, db->aDb[iDb].pSchema->cache_ಗಾತ್ರ);
  sqlite3_Btree_ಗಣ_Spill_ಗಾತ್ರ(p_ತಾತ್ಕಾಲಿಕ, sqlite3_Btree_ಗಣ_Spill_ಗಾತ್ರ(pMain,0));
  sqlite3_Btree_ಗಣ_Pager_Flags(p_ತಾತ್ಕಾಲಿಕ, pgflags|PAGER_CACHESPILL);

  /* Begin a transaction and take an exclusive lock on the main database
  ** file. This is done before the sqlite3BtreeGetPageSize(pMain) call below,
  ** to ensure that we do not try to change the page-size on a WAL database.
  */
  rc = execSql(db, pz_Err_ಸಂದೇಶ, "BEGIN");
  ಆದರೆ( rc!=SQLITE_OK ) ಹೋಗು ಕೊನೆ_ದ_vacuum;
  rc = sqlite3BtreeBeginTrans(pMain, p_ಹೊರಹೋಗು==0 ? 2 : 0, 0);
  ಆದರೆ( rc!=SQLITE_OK ) ಹೋಗು ಕೊನೆ_ದ_vacuum;

  /* Do not attempt to change the page size for a WAL database */
  ಆದರೆ( sqlite3_Pager_Get_Journal_ಬಹುಲಕ(sqlite3BtreePager(pMain))
                                               ==PAGER_JOURNALMODE_WAL
   && p_ಹೊರಹೋಗು==0
  ){
    db->ಮುಂದಿನ_Pagesize = 0;
  }

  ಆದರೆ( sqlite3_Btree_ಗಣ_ಪುಟ_ಗಾತ್ರ(p_ತಾತ್ಕಾಲಿಕ, sqlite3_Btree_Get_ಪುಟ_ಗಾತ್ರ(pMain), nRes, 0)
   || (!isMemDb && sqlite3_Btree_ಗಣ_ಪುಟ_ಗಾತ್ರ(p_ತಾತ್ಕಾಲಿಕ, db->ಮುಂದಿನ_Pagesize, nRes, 0))
   || NEVER(db->mallocFailed)
  ){
    rc = SQLITE_NOMEM_BKPT;
    ಹೋಗು ಕೊನೆ_ದ_vacuum;
  }

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOVACUUM
  sqlite3_Btree_ಗಣ_Auto_Vacuum(p_ತಾತ್ಕಾಲಿಕ, db->ಮುಂದಿನ_Autovac>=0 ? db->ಮುಂದಿನ_Autovac :
                                           sqlite3BtreeGetAutoVacuum(pMain));
#ಆದರೆ_ಅಂತ್ಯ

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
  ಆದರೆ( rc!=SQLITE_OK ) ಹೋಗು ಕೊನೆ_ದ_vacuum;
  rc = execSqlF(db, pz_Err_ಸಂದೇಶ,
      "SELECT sql FROM \"%w\".sqlite_schema"
      " WHERE type='index'",
      zDbMain
  );
  ಆದರೆ( rc!=SQLITE_OK ) ಹೋಗು ಕೊನೆ_ದ_vacuum;
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
  ಆದರೆ( rc!=SQLITE_OK ) ಹೋಗು ಕೊನೆ_ದ_vacuum;

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
  ಆದರೆ( rc ) ಹೋಗು ಕೊನೆ_ದ_vacuum;

  /* At this point, there is a write transaction open on both the 
  ** vacuum database and the main database. Assuming no error occurs,
  ** both transactions are closed by this block - the main database
  ** transaction by sqlite3BtreeCopyFile() and the other by an explicit
  ** call to sqlite3BtreeCommit().
  */
  {
    u32 meta;
    ಪೂರ್ಣಾಂಕ i;

    /* This array determines which meta meta values are preserved in the
    ** vacuum.  Even entries are the meta value number and odd entries
    ** are an increment to apply to the meta value after the vacuum.
    ** The increment is used to increase the schema cookie so that other
    ** connections to the same database will know to reread the schema.
    */
    ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ aCopy[] = {
       BTREE_SCHEMA_ಆವೃತ್ತಿ,     1,  /* Add one to the old schema cookie */
       BTREE_DEFAULT_CACHE_ಗಾತ್ರ, 0,  /* Preserve the default page cache size */
       BTREE_ಪಠ್ಯ_ENCODING,      0,  /* Preserve the text encoding */
       BTREE_USER_ಆವೃತ್ತಿ,       0,  /* Preserve the user version */
       BTREE_APPLICATION_ID,     0,  /* Preserve the application id */
    };

    ಪ್ರತಿಜ್ಞಾ( SQLITE_TXN_WRITE==sqlite3BtreeTxnState(p_ತಾತ್ಕಾಲಿಕ) );
    ಪ್ರತಿಜ್ಞಾ( p_ಹೊರಹೋಗು!=0 || SQLITE_TXN_WRITE==sqlite3BtreeTxnState(pMain) );

    /* Copy Btree meta values */
    ಗಾಗಿ(i=0; i<Array_ಗಾತ್ರ(aCopy); i+=2){
      /* GetMeta() and UpdateMeta() cannot fail in this context because
      ** we already have page 1 loaded into cache and marked dirty. */
      sqlite3BtreeGetMeta(pMain, aCopy[i], &meta);
      rc = sqlite3BtreeUpdateMeta(p_ತಾತ್ಕಾಲಿಕ, aCopy[i], meta+aCopy[i+1]);
      ಆದರೆ( NEVER(rc!=SQLITE_OK) ) ಹೋಗು ಕೊನೆ_ದ_vacuum;
    }

    ಆದರೆ( p_ಹೊರಹೋಗು==0 ){
      rc = sqlite3BtreeCopyFile(pMain, p_ತಾತ್ಕಾಲಿಕ);
    }
    ಆದರೆ( rc!=SQLITE_OK ) ಹೋಗು ಕೊನೆ_ದ_vacuum;
    rc = sqlite3BtreeCommit(p_ತಾತ್ಕಾಲಿಕ);
    ಆದರೆ( rc!=SQLITE_OK ) ಹೋಗು ಕೊನೆ_ದ_vacuum;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOVACUUM
    ಆದರೆ( p_ಹೊರಹೋಗು==0 ){
      sqlite3_Btree_ಗಣ_Auto_Vacuum(pMain, sqlite3BtreeGetAutoVacuum(p_ತಾತ್ಕಾಲಿಕ));
    }
#ಆದರೆ_ಅಂತ್ಯ
  }

  ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_OK );
  ಆದರೆ( p_ಹೊರಹೋಗು==0 ){
    nRes = sqlite3BtreeGetRequestedReserve(p_ತಾತ್ಕಾಲಿಕ);
    rc = sqlite3_Btree_ಗಣ_ಪುಟ_ಗಾತ್ರ(pMain, sqlite3_Btree_Get_ಪುಟ_ಗಾತ್ರ(p_ತಾತ್ಕಾಲಿಕ), nRes,1);
  }

ಕೊನೆ_ದ_vacuum:
  /* Restore the original value of db->flags */
  db->init.iDb = 0;
  db->mDbFlags = saved_mDbFlags;
  db->flags = saved_flags;
  db->nChange = saved_nChange;
  db->n_ಒಟ್ಟು_Change = saved_n_ಒಟ್ಟು_Change;
  db->mTrace = saved_mTrace;
  sqlite3_Btree_ಗಣ_ಪುಟ_ಗಾತ್ರ(pMain, -1, 0, 1);

  /* Currently there is an SQL level transaction open on the vacuum
  ** database. No locks are held on any other files (since the main file
  ** was committed at the btree level). So it safe to end the transaction
  ** by manually setting the autoCommit flag to true and detaching the
  ** vacuum database. The vacuum_db journal file is deleted when the pager
  ** is closed by the DETACH.
  */
  db->autoCommit = 1;

  ಆದರೆ( pDb ){
    sqlite3BtreeClose(pDb->pBt);
    pDb->pBt = 0;
    pDb->pSchema = 0;
  }

  /* This both clears the schemas and reduces the size of the db->aDb[]
  ** array. */ 
  sqlite3_Reset_All_Schemas_ದ_Connection(db);

  ಹಿಂತಿರುಗಿ rc;
}

#ಆದರೆ_ಅಂತ್ಯ  /* SQLITE_OMIT_VACUUM && SQLITE_OMIT_ATTACH */
