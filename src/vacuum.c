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
#شامل "sqliteInt.h"
#شامل "vdbeInt.h"

#اگر !defined(SQLITE_OMIT_VACUUM) && !defined(SQLITE_OMIT_ATTACH)

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
ساکن صحیح execSql(sqlite3 *db, حرف **pz_Err_پیغام, ثابت حرف *zSql){
  sqlite3_stmt *pStmt;
  صحیح rc;

  /* printf("SQL: [%s]\n", zSql); fflush(stdout); */
  rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
  اگر( rc!=SQLITE_OK ) واپس rc;
  جب( SQLITE_صف==(rc = sqlite3_قدم(pStmt)) ){
    ثابت حرف *zSubSql = (ثابت حرف*)sqlite3_عمود_متن(pStmt,0);
    أكد( sqlite3_strnicmp(zSql,"SELECT",6)==0 );
    /* The secondary SQL must be one of CREATE TABLE, CREATE INDEX,
    ** or INSERT.  Historically there have been attacks that first
    ** corrupt the sqlite_schema.sql field with other kinds of statements
    ** then run VACUUM to get those statements to execute at inappropriate
    ** times. */
    اگر( zSubSql
     && (لڑی_موازنہ(zSubSql,"CRE",3)==0 || لڑی_موازنہ(zSubSql,"INS",3)==0)
    ){
      rc = execSql(db, pz_Err_پیغام, zSubSql);
      اگر( rc!=SQLITE_OK ) توڑو;
    }
  }
  أكد( rc!=SQLITE_صف );
  اگر( rc==SQLITE_مکمل ) rc = SQLITE_OK;
  اگر( rc ){
    sqlite3_سیٹ_String(pz_Err_پیغام, db, sqlite3_errmsg(db));
  }
  (خالی)sqlite3_finalize(pStmt);
  واپس rc;
}
ساکن صحیح execSqlF(sqlite3 *db, حرف **pz_Err_پیغام, ثابت حرف *zSql, ...){
  حرف *z;
  va_list ap;
  صحیح rc;
  va_start(ap, zSql);
  z = sqlite3VMPrintf(db, zSql, ap);
  va_end(ap);
  اگر( z==0 ) واپس SQLITE_NOMEM;
  rc = execSql(db, pz_Err_پیغام, z);
  sqlite3DbFree(db, z);
  واپس rc;
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
خالی sqlite3Vacuum(Parse *pParse, Token *pNm, اظہاریہ *pInto){
  Vdbe *v = sqlite3GetVdbe(pParse);
  صحیح iDb = 0;
  اگر( v==0 ) جاؤ build_vacuum_انتہا;
  اگر( pParse->nErr ) جاؤ build_vacuum_انتہا;
  اگر( pNm ){
#اگر_غیر_تعریف_شدہ SQLITE_BUG_COMPATIBLE_20160819
    /* Default behavior:  Report an error if the argument to VACUUM is
    ** not recognized */
    iDb = sqlite3TwoPartName(pParse, pNm, pNm, &pNm);
    اگر( iDb<0 ) جاؤ build_vacuum_انتہا;
#ورنہ
    /* When SQLITE_BUG_COMPATIBLE_20160819 is defined, unrecognized arguments
    ** to VACUUM are silently ignored.  This is a back-out of a bug fix that
    ** occurred on 2016-08-19 (https://sqlite.org/src/info/083f9e6270).
    ** The buggy behavior is required for binary compatibility with some
    ** legacy applications. */
    iDb = sqlite3_ڈھونڈو_Db(pParse->db, pNm);
    اگر( iDb<0 ) iDb = 0;
#اگر_ختم
  }
  اگر( iDb!=1 ){
    صحیح iIntoReg = 0;
    اگر( pInto && sqlite3ResolveSelfReference(pParse,0,0,pInto,0)==0 ){
      iIntoReg = ++pParse->nMem;
      sqlite3_اظہاریہ_رمز(pParse, pInto, iIntoReg);
    }
    sqlite3_Vdbe_جوڑو_Op2(v, OP_Vacuum, iDb, iIntoReg);
    sqlite3_Vdbe_استعمال_کرتا_Btree(v, iDb);
  }
build_vacuum_انتہا:
  sqlite3_اظہاریہ_Delete(pParse->db, pInto);
  واپس;
}

/*
** This routine implements the OP_Vacuum opcode of the VDBE.
*/
SQLITE_NOINLINE صحیح sqlite3_تسلسل_Vacuum(
  حرف **pz_Err_پیغام,        /* Write error message here */
  sqlite3 *db,            /* Database connection */
  صحیح iDb,                /* Which attached DB to vacuum */
  sqlite3_قدر *p_برآمد     /* Write results here, if not NULL. VACUUM INTO */
){
  صحیح rc = SQLITE_OK;     /* Return code from service routines */
  Btree *pMain;           /* The database being vacuumed */
  Btree *p_عارضی;           /* The temporary database we vacuum into */
  u32 saved_mDbFlags;     /* Saved value of db->mDbFlags */
  u64 saved_flags;        /* Saved value of db->flags */
  i64 saved_nChange;      /* Saved value of db->nChange */
  i64 saved_n_کل_Change; /* Saved value of db->nTotalChange */
  u32 saved_openFlags;    /* Saved value of db->openFlags */
  u8 saved_mTrace;        /* Saved trace settings */
  Db *pDb = 0;            /* Database to detach at end of vacuum */
  صحیح isMemDb;            /* True if vacuuming a :memory: database */
  صحیح nRes;               /* Bytes of reserved space at the end of each page */
  صحیح nDb;                /* Number of attached databases */
  ثابت حرف *zDbMain;    /* Schema name of database to vacuum */
  ثابت حرف *z_برآمد;       /* Name of output file */
  u32 pgflags = PAGER_SYNCHRONOUS_OFF; /* sync flags for output db */
  u64 iRandom;            /* Random value used for zDbVacuum[] */
  حرف zDbVacuum[42];     /* Name of the ATTACH-ed database used for vacuum */


  اگر( !db->autoCommit ){
    sqlite3_سیٹ_String(pz_Err_پیغام, db, "cannot VACUUM from within a transaction");
    واپس SQLITE_ERROR; /* IMP: R-12218-18073 */
  }
  اگر( db->n_Vdbe_فعال>1 ){
    sqlite3_سیٹ_String(pz_Err_پیغام, db,"cannot VACUUM - SQL statements in progress");
    واپس SQLITE_ERROR; /* IMP: R-15610-35227 */
  }
  saved_openFlags = db->openFlags;
  اگر( p_برآمد ){
    اگر( sqlite3_قدر_type(p_برآمد)!=SQLITE_متن ){
      sqlite3_سیٹ_String(pz_Err_پیغام, db, "non-text filename");
      واپس SQLITE_ERROR;
    }
    z_برآمد = (ثابت حرف*)sqlite3_قدر_متن(p_برآمد);
    db->openFlags &= ~SQLITE_OPEN_READONLY;
    db->openFlags |= SQLITE_OPEN_CREATE|SQLITE_OPEN_READWRITE;
  }ورنہ{
    z_برآمد = "";
  }

  /* Save the current value of the database flags so that it can be 
  ** restored before returning. Then set the writable-schema flag, and
  ** disable CHECK and foreign key constraints.  */
  saved_flags = db->flags;
  saved_mDbFlags = db->mDbFlags;
  saved_nChange = db->nChange;
  saved_n_کل_Change = db->n_کل_Change;
  saved_mTrace = db->mTrace;
  db->flags |= SQLITE_WriteSchema | SQLITE_IgnoreChecks | SQLITE_Comments
               | SQLITE_AttachCreate | SQLITE_AttachWrite;
  db->mDbFlags |= DBFLAG_PreferBuiltin | DBFLAG_Vacuum;
  db->flags &= ~(u64)(SQLITE_Foreign_کلیدیں | SQLITE_الٹ_ترتیب
                   | SQLITE_Defensive | SQLITE_گنتی_صفیں);
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
  sqlite3_randomness(حجم(iRandom),&iRandom);
  sqlite3_snprintf(حجم(zDbVacuum), zDbVacuum, "vacuum_%016llx", iRandom);
  nDb = db->nDb;
  rc = execSqlF(db, pz_Err_پیغام, "ATTACH %Q AS %s", z_برآمد, zDbVacuum);
  db->openFlags = saved_openFlags;
  اگر( rc!=SQLITE_OK ) جاؤ انتہا_کا_vacuum;
  أكد( (db->nDb-1)==nDb );
  pDb = &db->aDb[nDb];
  أكد( strcmp(pDb->zDbSName,zDbVacuum)==0 );
  p_عارضی = pDb->pBt;
  nRes = sqlite3BtreeGetRequestedReserve(pMain);
  اگر( p_برآمد ){
    sqlite3_file *id = sqlite3PagerFile(sqlite3BtreePager(p_عارضی));
    i64 sz = 0;
    ثابت حرف *zFilename;
    اگر( id->pMethods!=0 && (sqlite3_Os_File_سائز(id, &sz)!=SQLITE_OK || sz>0) ){
      rc = SQLITE_ERROR;
      sqlite3_سیٹ_String(pz_Err_پیغام, db, "output file already exists");
      جاؤ انتہا_کا_vacuum;
    }
    db->mDbFlags |= DBFLAG_VacuumInto;

    /* For a VACUUM INTO, the pager-flags are set to the same values as
    ** they are for the database being vacuumed, except that PAGER_CACHESPILL
    ** is always set. */
    pgflags = db->aDb[iDb].safety_level | (db->flags & PAGER_FLAGS_نقاب);

    /* If the VACUUM INTO target file is a URI filename and if the
    ** "reserve=N" query parameter is present, reset the reserve to the
    ** amount specified, if the amount is within range */
    zFilename = sqlite3BtreeGetFilename(p_عارضی);
    اگر( ALWAYS(zFilename) ){
      صحیح nNew = (صحیح)sqlite3_uri_int64(zFilename, "reserve", nRes);
      اگر( nNew>=0 && nNew<=255 ) nRes = nNew;
    }
  }

  sqlite3_Btree_سیٹ_Cache_سائز(p_عارضی, db->aDb[iDb].pSchema->cache_سائز);
  sqlite3_Btree_سیٹ_Spill_سائز(p_عارضی, sqlite3_Btree_سیٹ_Spill_سائز(pMain,0));
  sqlite3_Btree_سیٹ_Pager_Flags(p_عارضی, pgflags|PAGER_CACHESPILL);

  /* Begin a transaction and take an exclusive lock on the main database
  ** file. This is done before the sqlite3BtreeGetPageSize(pMain) call below,
  ** to ensure that we do not try to change the page-size on a WAL database.
  */
  rc = execSql(db, pz_Err_پیغام, "BEGIN");
  اگر( rc!=SQLITE_OK ) جاؤ انتہا_کا_vacuum;
  rc = sqlite3BtreeBeginTrans(pMain, p_برآمد==0 ? 2 : 0, 0);
  اگر( rc!=SQLITE_OK ) جاؤ انتہا_کا_vacuum;

  /* Do not attempt to change the page size for a WAL database */
  اگر( sqlite3_Pager_Get_Journal_منوال(sqlite3BtreePager(pMain))
                                               ==PAGER_JOURNALMODE_WAL
   && p_برآمد==0
  ){
    db->اگلا_Pagesize = 0;
  }

  اگر( sqlite3_Btree_سیٹ_صفحہ_سائز(p_عارضی, sqlite3_Btree_Get_صفحہ_سائز(pMain), nRes, 0)
   || (!isMemDb && sqlite3_Btree_سیٹ_صفحہ_سائز(p_عارضی, db->اگلا_Pagesize, nRes, 0))
   || NEVER(db->mallocFailed)
  ){
    rc = SQLITE_NOMEM_BKPT;
    جاؤ انتہا_کا_vacuum;
  }

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOVACUUM
  sqlite3_Btree_سیٹ_Auto_Vacuum(p_عارضی, db->اگلا_Autovac>=0 ? db->اگلا_Autovac :
                                           sqlite3BtreeGetAutoVacuum(pMain));
#اگر_ختم

  /* Query the schema of the main database. Create a mirror schema
  ** in the temporary database.
  */
  db->init.iDb = nDb; /* force new CREATE statements into vacuum_db */
  rc = execSqlF(db, pz_Err_پیغام,
      "SELECT sql FROM \"%w\".sqlite_schema"
      " WHERE type='table'AND name<>'sqlite_sequence'"
      " AND coalesce(rootpage,1)>0",
      zDbMain
  );
  اگر( rc!=SQLITE_OK ) جاؤ انتہا_کا_vacuum;
  rc = execSqlF(db, pz_Err_پیغام,
      "SELECT sql FROM \"%w\".sqlite_schema"
      " WHERE type='index'",
      zDbMain
  );
  اگر( rc!=SQLITE_OK ) جاؤ انتہا_کا_vacuum;
  db->init.iDb = 0;

  /* Loop through the tables in the main database. For each, do
  ** an "INSERT INTO vacuum_db.xxx SELECT * FROM main.xxx;" to copy
  ** the contents to the temporary database.
  */
  rc = execSqlF(db, pz_Err_پیغام,
      "SELECT'INSERT INTO %s.'||quote(name)"
      "||' SELECT*FROM\"%w\".'||quote(name)"
      "FROM %s.sqlite_schema "
      "WHERE type='table'AND coalesce(rootpage,1)>0",
      zDbVacuum, zDbMain, zDbVacuum
  );
  أكد( (db->mDbFlags & DBFLAG_Vacuum)!=0 );
  db->mDbFlags &= ~DBFLAG_Vacuum;
  اگر( rc!=SQLITE_OK ) جاؤ انتہا_کا_vacuum;

  /* Copy the triggers, views, and virtual tables from the main database
  ** over to the temporary database.  None of these objects has any
  ** associated storage, so all we have to do is copy their entries
  ** from the schema table.
  */
  rc = execSqlF(db, pz_Err_پیغام,
      "INSERT INTO %s.sqlite_schema"
      " SELECT*FROM \"%w\".sqlite_schema"
      " WHERE type IN('view','trigger')"
      " OR(type='table'AND rootpage=0)",
      zDbVacuum, zDbMain
  );
  اگر( rc ) جاؤ انتہا_کا_vacuum;

  /* At this point, there is a write transaction open on both the 
  ** vacuum database and the main database. Assuming no error occurs,
  ** both transactions are closed by this block - the main database
  ** transaction by sqlite3BtreeCopyFile() and the other by an explicit
  ** call to sqlite3BtreeCommit().
  */
  {
    u32 meta;
    صحیح i;

    /* This array determines which meta meta values are preserved in the
    ** vacuum.  Even entries are the meta value number and odd entries
    ** are an increment to apply to the meta value after the vacuum.
    ** The increment is used to increase the schema cookie so that other
    ** connections to the same database will know to reread the schema.
    */
    ساکن ثابت غیرعلامتی حرف aCopy[] = {
       BTREE_SCHEMA_نسخہ,     1,  /* Add one to the old schema cookie */
       BTREE_DEFAULT_CACHE_سائز, 0,  /* Preserve the default page cache size */
       BTREE_متن_ENCODING,      0,  /* Preserve the text encoding */
       BTREE_USER_نسخہ,       0,  /* Preserve the user version */
       BTREE_APPLICATION_ID,     0,  /* Preserve the application id */
    };

    أكد( SQLITE_TXN_WRITE==sqlite3BtreeTxnState(p_عارضی) );
    أكد( p_برآمد!=0 || SQLITE_TXN_WRITE==sqlite3BtreeTxnState(pMain) );

    /* Copy Btree meta values */
    برائے(i=0; i<Array_سائز(aCopy); i+=2){
      /* GetMeta() and UpdateMeta() cannot fail in this context because
      ** we already have page 1 loaded into cache and marked dirty. */
      sqlite3BtreeGetMeta(pMain, aCopy[i], &meta);
      rc = sqlite3BtreeUpdateMeta(p_عارضی, aCopy[i], meta+aCopy[i+1]);
      اگر( NEVER(rc!=SQLITE_OK) ) جاؤ انتہا_کا_vacuum;
    }

    اگر( p_برآمد==0 ){
      rc = sqlite3BtreeCopyFile(pMain, p_عارضی);
    }
    اگر( rc!=SQLITE_OK ) جاؤ انتہا_کا_vacuum;
    rc = sqlite3BtreeCommit(p_عارضی);
    اگر( rc!=SQLITE_OK ) جاؤ انتہا_کا_vacuum;
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOVACUUM
    اگر( p_برآمد==0 ){
      sqlite3_Btree_سیٹ_Auto_Vacuum(pMain, sqlite3BtreeGetAutoVacuum(p_عارضی));
    }
#اگر_ختم
  }

  أكد( rc==SQLITE_OK );
  اگر( p_برآمد==0 ){
    nRes = sqlite3BtreeGetRequestedReserve(p_عارضی);
    rc = sqlite3_Btree_سیٹ_صفحہ_سائز(pMain, sqlite3_Btree_Get_صفحہ_سائز(p_عارضی), nRes,1);
  }

انتہا_کا_vacuum:
  /* Restore the original value of db->flags */
  db->init.iDb = 0;
  db->mDbFlags = saved_mDbFlags;
  db->flags = saved_flags;
  db->nChange = saved_nChange;
  db->n_کل_Change = saved_n_کل_Change;
  db->mTrace = saved_mTrace;
  sqlite3_Btree_سیٹ_صفحہ_سائز(pMain, -1, 0, 1);

  /* Currently there is an SQL level transaction open on the vacuum
  ** database. No locks are held on any other files (since the main file
  ** was committed at the btree level). So it safe to end the transaction
  ** by manually setting the autoCommit flag to true and detaching the
  ** vacuum database. The vacuum_db journal file is deleted when the pager
  ** is closed by the DETACH.
  */
  db->autoCommit = 1;

  اگر( pDb ){
    sqlite3BtreeClose(pDb->pBt);
    pDb->pBt = 0;
    pDb->pSchema = 0;
  }

  /* This both clears the schemas and reduces the size of the db->aDb[]
  ** array. */ 
  sqlite3_Reset_All_Schemas_کا_Connection(db);

  واپس rc;
}

#اگر_ختم  /* SQLITE_OMIT_VACUUM && SQLITE_OMIT_ATTACH */
