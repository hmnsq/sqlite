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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "vdbeInt.h"

#ਜੇ !defined(SQLITE_OMIT_VACUUM) && !defined(SQLITE_OMIT_ATTACH)

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
ਸਥਿਰ ਪੂਰਨ execSql(sqlite3 *db, ਅੱਖਰ **pz_Err_ਸੁਨੇਹਾ, ਅਚਲ ਅੱਖਰ *zSql){
  sqlite3_stmt *pStmt;
  ਪੂਰਨ rc;

  /* printf("SQL: [%s]\n", zSql); fflush(stdout); */
  rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
  ਜੇ( rc!=SQLITE_OK ) ਵਾਪਸ rc;
  ਜਦੋਂ( SQLITE_ਪੰਕਤੀ==(rc = sqlite3_ਕਦਮ(pStmt)) ){
    ਅਚਲ ਅੱਖਰ *zSubSql = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਕਾਲਮ_ਪਾਠ(pStmt,0);
    ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_strnicmp(zSql,"SELECT",6)==0 );
    /* The secondary SQL must be one of CREATE TABLE, CREATE INDEX,
    ** or INSERT.  Historically there have been attacks that first
    ** corrupt the sqlite_schema.sql field with other kinds of statements
    ** then run VACUUM to get those statements to execute at inappropriate
    ** times. */
    ਜੇ( zSubSql
     && (ਲੜੀ_ਤੁਲਨਾ(zSubSql,"CRE",3)==0 || ਲੜੀ_ਤੁਲਨਾ(zSubSql,"INS",3)==0)
    ){
      rc = execSql(db, pz_Err_ਸੁਨੇਹਾ, zSubSql);
      ਜੇ( rc!=SQLITE_OK ) ਰੋਕੋ;
    }
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( rc!=SQLITE_ਪੰਕਤੀ );
  ਜੇ( rc==SQLITE_ਪੂਰਾ ) rc = SQLITE_OK;
  ਜੇ( rc ){
    sqlite3_ਸੈੱਟ_String(pz_Err_ਸੁਨੇਹਾ, db, sqlite3_errmsg(db));
  }
  (ਖਾਲੀ)sqlite3_finalize(pStmt);
  ਵਾਪਸ rc;
}
ਸਥਿਰ ਪੂਰਨ execSqlF(sqlite3 *db, ਅੱਖਰ **pz_Err_ਸੁਨੇਹਾ, ਅਚਲ ਅੱਖਰ *zSql, ...){
  ਅੱਖਰ *z;
  va_list ap;
  ਪੂਰਨ rc;
  va_start(ap, zSql);
  z = sqlite3VMPrintf(db, zSql, ap);
  va_end(ap);
  ਜੇ( z==0 ) ਵਾਪਸ SQLITE_NOMEM;
  rc = execSql(db, pz_Err_ਸੁਨੇਹਾ, z);
  sqlite3DbFree(db, z);
  ਵਾਪਸ rc;
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
ਖਾਲੀ sqlite3Vacuum(Parse *pParse, Token *pNm, ਵਿਅੰਜਕ *pInto){
  Vdbe *v = sqlite3GetVdbe(pParse);
  ਪੂਰਨ iDb = 0;
  ਜੇ( v==0 ) ਜਾਓ build_vacuum_ਸਿਰਾ;
  ਜੇ( pParse->nErr ) ਜਾਓ build_vacuum_ਸਿਰਾ;
  ਜੇ( pNm ){
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_BUG_COMPATIBLE_20160819
    /* Default behavior:  Report an error if the argument to VACUUM is
    ** not recognized */
    iDb = sqlite3TwoPartName(pParse, pNm, pNm, &pNm);
    ਜੇ( iDb<0 ) ਜਾਓ build_vacuum_ਸਿਰਾ;
#ਨਹੀਂਤਾਂ
    /* When SQLITE_BUG_COMPATIBLE_20160819 is defined, unrecognized arguments
    ** to VACUUM are silently ignored.  This is a back-out of a bug fix that
    ** occurred on 2016-08-19 (https://sqlite.org/src/info/083f9e6270).
    ** The buggy behavior is required for binary compatibility with some
    ** legacy applications. */
    iDb = sqlite3_ਲੱਭੋ_Db(pParse->db, pNm);
    ਜੇ( iDb<0 ) iDb = 0;
#ਜੇ_ਅੰਤ
  }
  ਜੇ( iDb!=1 ){
    ਪੂਰਨ iIntoReg = 0;
    ਜੇ( pInto && sqlite3ResolveSelfReference(pParse,0,0,pInto,0)==0 ){
      iIntoReg = ++pParse->nMem;
      sqlite3_ਵਿਅੰਜਕ_ਕੋਡ(pParse, pInto, iIntoReg);
    }
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Vacuum, iDb, iIntoReg);
    sqlite3_Vdbe_ਵਰਤਦਾ_Btree(v, iDb);
  }
build_vacuum_ਸਿਰਾ:
  sqlite3_ਵਿਅੰਜਕ_Delete(pParse->db, pInto);
  ਵਾਪਸ;
}

/*
** This routine implements the OP_Vacuum opcode of the VDBE.
*/
SQLITE_NOINLINE ਪੂਰਨ sqlite3_ਲੜੀਵਾਰ_Vacuum(
  ਅੱਖਰ **pz_Err_ਸੁਨੇਹਾ,        /* Write error message here */
  sqlite3 *db,            /* Database connection */
  ਪੂਰਨ iDb,                /* Which attached DB to vacuum */
  sqlite3_ਮੁੱਲ *p_ਬਾਹਰਲਾ     /* Write results here, if not NULL. VACUUM INTO */
){
  ਪੂਰਨ rc = SQLITE_OK;     /* Return code from service routines */
  Btree *pMain;           /* The database being vacuumed */
  Btree *p_ਆਰਜ਼ੀ;           /* The temporary database we vacuum into */
  u32 saved_mDbFlags;     /* Saved value of db->mDbFlags */
  u64 saved_flags;        /* Saved value of db->flags */
  i64 saved_nChange;      /* Saved value of db->nChange */
  i64 saved_n_ਕੁੱਲ_Change; /* Saved value of db->nTotalChange */
  u32 saved_openFlags;    /* Saved value of db->openFlags */
  u8 saved_mTrace;        /* Saved trace settings */
  Db *pDb = 0;            /* Database to detach at end of vacuum */
  ਪੂਰਨ isMemDb;            /* True if vacuuming a :memory: database */
  ਪੂਰਨ nRes;               /* Bytes of reserved space at the end of each page */
  ਪੂਰਨ nDb;                /* Number of attached databases */
  ਅਚਲ ਅੱਖਰ *zDbMain;    /* Schema name of database to vacuum */
  ਅਚਲ ਅੱਖਰ *z_ਬਾਹਰਲਾ;       /* Name of output file */
  u32 pgflags = PAGER_SYNCHRONOUS_OFF; /* sync flags for output db */
  u64 iRandom;            /* Random value used for zDbVacuum[] */
  ਅੱਖਰ zDbVacuum[42];     /* Name of the ATTACH-ed database used for vacuum */


  ਜੇ( !db->autoCommit ){
    sqlite3_ਸੈੱਟ_String(pz_Err_ਸੁਨੇਹਾ, db, "cannot VACUUM from within a transaction");
    ਵਾਪਸ SQLITE_ERROR; /* IMP: R-12218-18073 */
  }
  ਜੇ( db->n_Vdbe_ਸਰਗਰਮ>1 ){
    sqlite3_ਸੈੱਟ_String(pz_Err_ਸੁਨੇਹਾ, db,"cannot VACUUM - SQL statements in progress");
    ਵਾਪਸ SQLITE_ERROR; /* IMP: R-15610-35227 */
  }
  saved_openFlags = db->openFlags;
  ਜੇ( p_ਬਾਹਰਲਾ ){
    ਜੇ( sqlite3_ਮੁੱਲ_type(p_ਬਾਹਰਲਾ)!=SQLITE_ਪਾਠ ){
      sqlite3_ਸੈੱਟ_String(pz_Err_ਸੁਨੇਹਾ, db, "non-text filename");
      ਵਾਪਸ SQLITE_ERROR;
    }
    z_ਬਾਹਰਲਾ = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(p_ਬਾਹਰਲਾ);
    db->openFlags &= ~SQLITE_OPEN_READONLY;
    db->openFlags |= SQLITE_OPEN_CREATE|SQLITE_OPEN_READWRITE;
  }ਨਹੀਂਤਾਂ{
    z_ਬਾਹਰਲਾ = "";
  }

  /* Save the current value of the database flags so that it can be 
  ** restored before returning. Then set the writable-schema flag, and
  ** disable CHECK and foreign key constraints.  */
  saved_flags = db->flags;
  saved_mDbFlags = db->mDbFlags;
  saved_nChange = db->nChange;
  saved_n_ਕੁੱਲ_Change = db->n_ਕੁੱਲ_Change;
  saved_mTrace = db->mTrace;
  db->flags |= SQLITE_WriteSchema | SQLITE_IgnoreChecks | SQLITE_Comments
               | SQLITE_AttachCreate | SQLITE_AttachWrite;
  db->mDbFlags |= DBFLAG_PreferBuiltin | DBFLAG_Vacuum;
  db->flags &= ~(u64)(SQLITE_Foreign_ਕੁੰਜੀਆਂ | SQLITE_ਉਲਟ_ਕ੍ਰਮ
                   | SQLITE_Defensive | SQLITE_ਗਿਣਤੀ_ਪੰਕਤੀਆਂ);
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
  sqlite3_randomness(ਆਕਾਰ(iRandom),&iRandom);
  sqlite3_snprintf(ਆਕਾਰ(zDbVacuum), zDbVacuum, "vacuum_%016llx", iRandom);
  nDb = db->nDb;
  rc = execSqlF(db, pz_Err_ਸੁਨੇਹਾ, "ATTACH %Q AS %s", z_ਬਾਹਰਲਾ, zDbVacuum);
  db->openFlags = saved_openFlags;
  ਜੇ( rc!=SQLITE_OK ) ਜਾਓ ਸਿਰਾ_ਦਾ_vacuum;
  ਪ੍ਰਤਿਜ੍ਞਾ( (db->nDb-1)==nDb );
  pDb = &db->aDb[nDb];
  ਪ੍ਰਤਿਜ੍ਞਾ( strcmp(pDb->zDbSName,zDbVacuum)==0 );
  p_ਆਰਜ਼ੀ = pDb->pBt;
  nRes = sqlite3BtreeGetRequestedReserve(pMain);
  ਜੇ( p_ਬਾਹਰਲਾ ){
    sqlite3_file *id = sqlite3PagerFile(sqlite3BtreePager(p_ਆਰਜ਼ੀ));
    i64 sz = 0;
    ਅਚਲ ਅੱਖਰ *zFilename;
    ਜੇ( id->pMethods!=0 && (sqlite3_Os_File_ਮਾਪ(id, &sz)!=SQLITE_OK || sz>0) ){
      rc = SQLITE_ERROR;
      sqlite3_ਸੈੱਟ_String(pz_Err_ਸੁਨੇਹਾ, db, "output file already exists");
      ਜਾਓ ਸਿਰਾ_ਦਾ_vacuum;
    }
    db->mDbFlags |= DBFLAG_VacuumInto;

    /* For a VACUUM INTO, the pager-flags are set to the same values as
    ** they are for the database being vacuumed, except that PAGER_CACHESPILL
    ** is always set. */
    pgflags = db->aDb[iDb].safety_level | (db->flags & PAGER_FLAGS_ਨਕਾਬ);

    /* If the VACUUM INTO target file is a URI filename and if the
    ** "reserve=N" query parameter is present, reset the reserve to the
    ** amount specified, if the amount is within range */
    zFilename = sqlite3BtreeGetFilename(p_ਆਰਜ਼ੀ);
    ਜੇ( ALWAYS(zFilename) ){
      ਪੂਰਨ nNew = (ਪੂਰਨ)sqlite3_uri_int64(zFilename, "reserve", nRes);
      ਜੇ( nNew>=0 && nNew<=255 ) nRes = nNew;
    }
  }

  sqlite3_Btree_ਸੈੱਟ_Cache_ਮਾਪ(p_ਆਰਜ਼ੀ, db->aDb[iDb].pSchema->cache_ਮਾਪ);
  sqlite3_Btree_ਸੈੱਟ_Spill_ਮਾਪ(p_ਆਰਜ਼ੀ, sqlite3_Btree_ਸੈੱਟ_Spill_ਮਾਪ(pMain,0));
  sqlite3_Btree_ਸੈੱਟ_Pager_Flags(p_ਆਰਜ਼ੀ, pgflags|PAGER_CACHESPILL);

  /* Begin a transaction and take an exclusive lock on the main database
  ** file. This is done before the sqlite3BtreeGetPageSize(pMain) call below,
  ** to ensure that we do not try to change the page-size on a WAL database.
  */
  rc = execSql(db, pz_Err_ਸੁਨੇਹਾ, "BEGIN");
  ਜੇ( rc!=SQLITE_OK ) ਜਾਓ ਸਿਰਾ_ਦਾ_vacuum;
  rc = sqlite3BtreeBeginTrans(pMain, p_ਬਾਹਰਲਾ==0 ? 2 : 0, 0);
  ਜੇ( rc!=SQLITE_OK ) ਜਾਓ ਸਿਰਾ_ਦਾ_vacuum;

  /* Do not attempt to change the page size for a WAL database */
  ਜੇ( sqlite3_Pager_Get_Journal_ਬਹੁਲਕ(sqlite3BtreePager(pMain))
                                               ==PAGER_JOURNALMODE_WAL
   && p_ਬਾਹਰਲਾ==0
  ){
    db->ਅਗਲਾ_Pagesize = 0;
  }

  ਜੇ( sqlite3_Btree_ਸੈੱਟ_ਪੰਨਾ_ਮਾਪ(p_ਆਰਜ਼ੀ, sqlite3_Btree_Get_ਪੰਨਾ_ਮਾਪ(pMain), nRes, 0)
   || (!isMemDb && sqlite3_Btree_ਸੈੱਟ_ਪੰਨਾ_ਮਾਪ(p_ਆਰਜ਼ੀ, db->ਅਗਲਾ_Pagesize, nRes, 0))
   || NEVER(db->mallocFailed)
  ){
    rc = SQLITE_NOMEM_BKPT;
    ਜਾਓ ਸਿਰਾ_ਦਾ_vacuum;
  }

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOVACUUM
  sqlite3_Btree_ਸੈੱਟ_Auto_Vacuum(p_ਆਰਜ਼ੀ, db->ਅਗਲਾ_Autovac>=0 ? db->ਅਗਲਾ_Autovac :
                                           sqlite3BtreeGetAutoVacuum(pMain));
#ਜੇ_ਅੰਤ

  /* Query the schema of the main database. Create a mirror schema
  ** in the temporary database.
  */
  db->init.iDb = nDb; /* force new CREATE statements into vacuum_db */
  rc = execSqlF(db, pz_Err_ਸੁਨੇਹਾ,
      "SELECT sql FROM \"%w\".sqlite_schema"
      " WHERE type='table'AND name<>'sqlite_sequence'"
      " AND coalesce(rootpage,1)>0",
      zDbMain
  );
  ਜੇ( rc!=SQLITE_OK ) ਜਾਓ ਸਿਰਾ_ਦਾ_vacuum;
  rc = execSqlF(db, pz_Err_ਸੁਨੇਹਾ,
      "SELECT sql FROM \"%w\".sqlite_schema"
      " WHERE type='index'",
      zDbMain
  );
  ਜੇ( rc!=SQLITE_OK ) ਜਾਓ ਸਿਰਾ_ਦਾ_vacuum;
  db->init.iDb = 0;

  /* Loop through the tables in the main database. For each, do
  ** an "INSERT INTO vacuum_db.xxx SELECT * FROM main.xxx;" to copy
  ** the contents to the temporary database.
  */
  rc = execSqlF(db, pz_Err_ਸੁਨੇਹਾ,
      "SELECT'INSERT INTO %s.'||quote(name)"
      "||' SELECT*FROM\"%w\".'||quote(name)"
      "FROM %s.sqlite_schema "
      "WHERE type='table'AND coalesce(rootpage,1)>0",
      zDbVacuum, zDbMain, zDbVacuum
  );
  ਪ੍ਰਤਿਜ੍ਞਾ( (db->mDbFlags & DBFLAG_Vacuum)!=0 );
  db->mDbFlags &= ~DBFLAG_Vacuum;
  ਜੇ( rc!=SQLITE_OK ) ਜਾਓ ਸਿਰਾ_ਦਾ_vacuum;

  /* Copy the triggers, views, and virtual tables from the main database
  ** over to the temporary database.  None of these objects has any
  ** associated storage, so all we have to do is copy their entries
  ** from the schema table.
  */
  rc = execSqlF(db, pz_Err_ਸੁਨੇਹਾ,
      "INSERT INTO %s.sqlite_schema"
      " SELECT*FROM \"%w\".sqlite_schema"
      " WHERE type IN('view','trigger')"
      " OR(type='table'AND rootpage=0)",
      zDbVacuum, zDbMain
  );
  ਜੇ( rc ) ਜਾਓ ਸਿਰਾ_ਦਾ_vacuum;

  /* At this point, there is a write transaction open on both the 
  ** vacuum database and the main database. Assuming no error occurs,
  ** both transactions are closed by this block - the main database
  ** transaction by sqlite3BtreeCopyFile() and the other by an explicit
  ** call to sqlite3BtreeCommit().
  */
  {
    u32 meta;
    ਪੂਰਨ i;

    /* This array determines which meta meta values are preserved in the
    ** vacuum.  Even entries are the meta value number and odd entries
    ** are an increment to apply to the meta value after the vacuum.
    ** The increment is used to increase the schema cookie so that other
    ** connections to the same database will know to reread the schema.
    */
    ਸਥਿਰ ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ aCopy[] = {
       BTREE_SCHEMA_ਸੰਸਕਰਣ,     1,  /* Add one to the old schema cookie */
       BTREE_DEFAULT_CACHE_ਮਾਪ, 0,  /* Preserve the default page cache size */
       BTREE_ਪਾਠ_ENCODING,      0,  /* Preserve the text encoding */
       BTREE_USER_ਸੰਸਕਰਣ,       0,  /* Preserve the user version */
       BTREE_APPLICATION_ID,     0,  /* Preserve the application id */
    };

    ਪ੍ਰਤਿਜ੍ਞਾ( SQLITE_TXN_WRITE==sqlite3BtreeTxnState(p_ਆਰਜ਼ੀ) );
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਬਾਹਰਲਾ!=0 || SQLITE_TXN_WRITE==sqlite3BtreeTxnState(pMain) );

    /* Copy Btree meta values */
    ਲਈ(i=0; i<Array_ਮਾਪ(aCopy); i+=2){
      /* GetMeta() and UpdateMeta() cannot fail in this context because
      ** we already have page 1 loaded into cache and marked dirty. */
      sqlite3BtreeGetMeta(pMain, aCopy[i], &meta);
      rc = sqlite3BtreeUpdateMeta(p_ਆਰਜ਼ੀ, aCopy[i], meta+aCopy[i+1]);
      ਜੇ( NEVER(rc!=SQLITE_OK) ) ਜਾਓ ਸਿਰਾ_ਦਾ_vacuum;
    }

    ਜੇ( p_ਬਾਹਰਲਾ==0 ){
      rc = sqlite3BtreeCopyFile(pMain, p_ਆਰਜ਼ੀ);
    }
    ਜੇ( rc!=SQLITE_OK ) ਜਾਓ ਸਿਰਾ_ਦਾ_vacuum;
    rc = sqlite3BtreeCommit(p_ਆਰਜ਼ੀ);
    ਜੇ( rc!=SQLITE_OK ) ਜਾਓ ਸਿਰਾ_ਦਾ_vacuum;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOVACUUM
    ਜੇ( p_ਬਾਹਰਲਾ==0 ){
      sqlite3_Btree_ਸੈੱਟ_Auto_Vacuum(pMain, sqlite3BtreeGetAutoVacuum(p_ਆਰਜ਼ੀ));
    }
#ਜੇ_ਅੰਤ
  }

  ਪ੍ਰਤਿਜ੍ਞਾ( rc==SQLITE_OK );
  ਜੇ( p_ਬਾਹਰਲਾ==0 ){
    nRes = sqlite3BtreeGetRequestedReserve(p_ਆਰਜ਼ੀ);
    rc = sqlite3_Btree_ਸੈੱਟ_ਪੰਨਾ_ਮਾਪ(pMain, sqlite3_Btree_Get_ਪੰਨਾ_ਮਾਪ(p_ਆਰਜ਼ੀ), nRes,1);
  }

ਸਿਰਾ_ਦਾ_vacuum:
  /* Restore the original value of db->flags */
  db->init.iDb = 0;
  db->mDbFlags = saved_mDbFlags;
  db->flags = saved_flags;
  db->nChange = saved_nChange;
  db->n_ਕੁੱਲ_Change = saved_n_ਕੁੱਲ_Change;
  db->mTrace = saved_mTrace;
  sqlite3_Btree_ਸੈੱਟ_ਪੰਨਾ_ਮਾਪ(pMain, -1, 0, 1);

  /* Currently there is an SQL level transaction open on the vacuum
  ** database. No locks are held on any other files (since the main file
  ** was committed at the btree level). So it safe to end the transaction
  ** by manually setting the autoCommit flag to true and detaching the
  ** vacuum database. The vacuum_db journal file is deleted when the pager
  ** is closed by the DETACH.
  */
  db->autoCommit = 1;

  ਜੇ( pDb ){
    sqlite3BtreeClose(pDb->pBt);
    pDb->pBt = 0;
    pDb->pSchema = 0;
  }

  /* This both clears the schemas and reduces the size of the db->aDb[]
  ** array. */ 
  sqlite3_Reset_All_Schemas_ਦਾ_Connection(db);

  ਵਾਪਸ rc;
}

#ਜੇ_ਅੰਤ  /* SQLITE_OMIT_VACUUM && SQLITE_OMIT_ATTACH */
