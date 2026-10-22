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
#ضمن "sqliteInt.h"
#ضمن "vdbeInt.h"

#إذا !defined(SQLITE_OMIT_VACUUM) && !defined(SQLITE_OMIT_ATTACH)

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
ساكن صحيح execSql(sqlite3 *db, حرف **pz_Err_پیغام, ثابت حرف *zSql){
  sqlite3_stmt *pStmt;
  صحيح rc;

  /* printf("SQL: [%s]\n", zSql); fflush(stdout); */
  rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
  إذا( rc!=SQLITE_OK ) أرجع rc;
  طالما( SQLITE_لیکه==(rc = sqlite3_ګام(pStmt)) ){
    ثابت حرف *zSubSql = (ثابت حرف*)sqlite3_ستنه_متن(pStmt,0);
    أكد( sqlite3_strnicmp(zSql,"SELECT",6)==0 );
    /* The secondary SQL must be one of CREATE TABLE, CREATE INDEX,
    ** or INSERT.  Historically there have been attacks that first
    ** corrupt the sqlite_schema.sql field with other kinds of statements
    ** then run VACUUM to get those statements to execute at inappropriate
    ** times. */
    إذا( zSubSql
     && (مزي_پرتله(zSubSql,"CRE",3)==0 || مزي_پرتله(zSubSql,"INS",3)==0)
    ){
      rc = execSql(db, pz_Err_پیغام, zSubSql);
      إذا( rc!=SQLITE_OK ) اكسر;
    }
  }
  أكد( rc!=SQLITE_لیکه );
  إذا( rc==SQLITE_بشپړ ) rc = SQLITE_OK;
  إذا( rc ){
    sqlite3_سېټ_String(pz_Err_پیغام, db, sqlite3_errmsg(db));
  }
  (فراغ)sqlite3_finalize(pStmt);
  أرجع rc;
}
ساكن صحيح execSqlF(sqlite3 *db, حرف **pz_Err_پیغام, ثابت حرف *zSql, ...){
  حرف *z;
  va_list ap;
  صحيح rc;
  va_start(ap, zSql);
  z = sqlite3VMPrintf(db, zSql, ap);
  va_end(ap);
  إذا( z==0 ) أرجع SQLITE_NOMEM;
  rc = execSql(db, pz_Err_پیغام, z);
  sqlite3DbFree(db, z);
  أرجع rc;
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
فراغ sqlite3Vacuum(Parse *pParse, Token *pNm, بیان *pInto){
  Vdbe *v = sqlite3GetVdbe(pParse);
  صحيح iDb = 0;
  إذا( v==0 ) اذهب build_vacuum_پای_ټکی;
  إذا( pParse->nErr ) اذهب build_vacuum_پای_ټکی;
  إذا( pNm ){
#که_نه_تعریف_شوی SQLITE_BUG_COMPATIBLE_20160819
    /* Default behavior:  Report an error if the argument to VACUUM is
    ** not recognized */
    iDb = sqlite3TwoPartName(pParse, pNm, pNm, &pNm);
    إذا( iDb<0 ) اذهب build_vacuum_پای_ټکی;
#وإلا
    /* When SQLITE_BUG_COMPATIBLE_20160819 is defined, unrecognized arguments
    ** to VACUUM are silently ignored.  This is a back-out of a bug fix that
    ** occurred on 2016-08-19 (https://sqlite.org/src/info/083f9e6270).
    ** The buggy behavior is required for binary compatibility with some
    ** legacy applications. */
    iDb = sqlite3_پیدا_کړه_Db(pParse->db, pNm);
    إذا( iDb<0 ) iDb = 0;
#د_که_پای
  }
  إذا( iDb!=1 ){
    صحيح iIntoReg = 0;
    إذا( pInto && sqlite3ResolveSelfReference(pParse,0,0,pInto,0)==0 ){
      iIntoReg = ++pParse->nMem;
      sqlite3_بیان_کوډ(pParse, pInto, iIntoReg);
    }
    sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Vacuum, iDb, iIntoReg);
    sqlite3_Vdbe_کاروي_Btree(v, iDb);
  }
build_vacuum_پای_ټکی:
  sqlite3_بیان_Delete(pParse->db, pInto);
  أرجع;
}

/*
** This routine implements the OP_Vacuum opcode of the VDBE.
*/
SQLITE_NOINLINE صحيح sqlite3_پرلپسې_Vacuum(
  حرف **pz_Err_پیغام,        /* Write error message here */
  sqlite3 *db,            /* Database connection */
  صحيح iDb,                /* Which attached DB to vacuum */
  sqlite3_ارزښت *p_بهر     /* Write results here, if not NULL. VACUUM INTO */
){
  صحيح rc = SQLITE_OK;     /* Return code from service routines */
  Btree *pMain;           /* The database being vacuumed */
  Btree *p_لنډمهاله;           /* The temporary database we vacuum into */
  u32 saved_mDbFlags;     /* Saved value of db->mDbFlags */
  u64 saved_flags;        /* Saved value of db->flags */
  i64 saved_nChange;      /* Saved value of db->nChange */
  i64 saved_n_ټول_Change; /* Saved value of db->nTotalChange */
  u32 saved_openFlags;    /* Saved value of db->openFlags */
  u8 saved_mTrace;        /* Saved trace settings */
  Db *pDb = 0;            /* Database to detach at end of vacuum */
  صحيح isMemDb;            /* True if vacuuming a :memory: database */
  صحيح nRes;               /* Bytes of reserved space at the end of each page */
  صحيح nDb;                /* Number of attached databases */
  ثابت حرف *zDbMain;    /* Schema name of database to vacuum */
  ثابت حرف *z_بهر;       /* Name of output file */
  u32 pgflags = PAGER_SYNCHRONOUS_OFF; /* sync flags for output db */
  u64 iRandom;            /* Random value used for zDbVacuum[] */
  حرف zDbVacuum[42];     /* Name of the ATTACH-ed database used for vacuum */


  إذا( !db->autoCommit ){
    sqlite3_سېټ_String(pz_Err_پیغام, db, "cannot VACUUM from within a transaction");
    أرجع SQLITE_ERROR; /* IMP: R-12218-18073 */
  }
  إذا( db->n_Vdbe_فعال>1 ){
    sqlite3_سېټ_String(pz_Err_پیغام, db,"cannot VACUUM - SQL statements in progress");
    أرجع SQLITE_ERROR; /* IMP: R-15610-35227 */
  }
  saved_openFlags = db->openFlags;
  إذا( p_بهر ){
    إذا( sqlite3_ارزښت_type(p_بهر)!=SQLITE_متن ){
      sqlite3_سېټ_String(pz_Err_پیغام, db, "non-text filename");
      أرجع SQLITE_ERROR;
    }
    z_بهر = (ثابت حرف*)sqlite3_ارزښت_متن(p_بهر);
    db->openFlags &= ~SQLITE_OPEN_READONLY;
    db->openFlags |= SQLITE_OPEN_CREATE|SQLITE_OPEN_READWRITE;
  }وإلا{
    z_بهر = "";
  }

  /* Save the current value of the database flags so that it can be 
  ** restored before returning. Then set the writable-schema flag, and
  ** disable CHECK and foreign key constraints.  */
  saved_flags = db->flags;
  saved_mDbFlags = db->mDbFlags;
  saved_nChange = db->nChange;
  saved_n_ټول_Change = db->n_ټول_Change;
  saved_mTrace = db->mTrace;
  db->flags |= SQLITE_WriteSchema | SQLITE_IgnoreChecks | SQLITE_Comments
               | SQLITE_AttachCreate | SQLITE_AttachWrite;
  db->mDbFlags |= DBFLAG_PreferBuiltin | DBFLAG_Vacuum;
  db->flags &= ~(u64)(SQLITE_Foreign_کلیګانې | SQLITE_سرچپه_ترتیب
                   | SQLITE_Defensive | SQLITE_شمېر_لیکې);
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
  rc = execSqlF(db, pz_Err_پیغام, "ATTACH %Q AS %s", z_بهر, zDbVacuum);
  db->openFlags = saved_openFlags;
  إذا( rc!=SQLITE_OK ) اذهب پای_ټکی_د_vacuum;
  أكد( (db->nDb-1)==nDb );
  pDb = &db->aDb[nDb];
  أكد( strcmp(pDb->zDbSName,zDbVacuum)==0 );
  p_لنډمهاله = pDb->pBt;
  nRes = sqlite3BtreeGetRequestedReserve(pMain);
  إذا( p_بهر ){
    sqlite3_file *id = sqlite3PagerFile(sqlite3BtreePager(p_لنډمهاله));
    i64 sz = 0;
    ثابت حرف *zFilename;
    إذا( id->pMethods!=0 && (sqlite3_Os_File_اندازه(id, &sz)!=SQLITE_OK || sz>0) ){
      rc = SQLITE_ERROR;
      sqlite3_سېټ_String(pz_Err_پیغام, db, "output file already exists");
      اذهب پای_ټکی_د_vacuum;
    }
    db->mDbFlags |= DBFLAG_VacuumInto;

    /* For a VACUUM INTO, the pager-flags are set to the same values as
    ** they are for the database being vacuumed, except that PAGER_CACHESPILL
    ** is always set. */
    pgflags = db->aDb[iDb].safety_level | (db->flags & PAGER_FLAGS_ماسک);

    /* If the VACUUM INTO target file is a URI filename and if the
    ** "reserve=N" query parameter is present, reset the reserve to the
    ** amount specified, if the amount is within range */
    zFilename = sqlite3BtreeGetFilename(p_لنډمهاله);
    إذا( ALWAYS(zFilename) ){
      صحيح nNew = (صحيح)sqlite3_uri_int64(zFilename, "reserve", nRes);
      إذا( nNew>=0 && nNew<=255 ) nRes = nNew;
    }
  }

  sqlite3_Btree_سېټ_Cache_اندازه(p_لنډمهاله, db->aDb[iDb].pSchema->cache_اندازه);
  sqlite3_Btree_سېټ_Spill_اندازه(p_لنډمهاله, sqlite3_Btree_سېټ_Spill_اندازه(pMain,0));
  sqlite3_Btree_سېټ_Pager_Flags(p_لنډمهاله, pgflags|PAGER_CACHESPILL);

  /* Begin a transaction and take an exclusive lock on the main database
  ** file. This is done before the sqlite3BtreeGetPageSize(pMain) call below,
  ** to ensure that we do not try to change the page-size on a WAL database.
  */
  rc = execSql(db, pz_Err_پیغام, "BEGIN");
  إذا( rc!=SQLITE_OK ) اذهب پای_ټکی_د_vacuum;
  rc = sqlite3BtreeBeginTrans(pMain, p_بهر==0 ? 2 : 0, 0);
  إذا( rc!=SQLITE_OK ) اذهب پای_ټکی_د_vacuum;

  /* Do not attempt to change the page size for a WAL database */
  إذا( sqlite3_Pager_Get_Journal_ډېرپېښ(sqlite3BtreePager(pMain))
                                               ==PAGER_JOURNALMODE_WAL
   && p_بهر==0
  ){
    db->بل_Pagesize = 0;
  }

  إذا( sqlite3_Btree_سېټ_پاڼه_اندازه(p_لنډمهاله, sqlite3_Btree_Get_پاڼه_اندازه(pMain), nRes, 0)
   || (!isMemDb && sqlite3_Btree_سېټ_پاڼه_اندازه(p_لنډمهاله, db->بل_Pagesize, nRes, 0))
   || NEVER(db->mallocFailed)
  ){
    rc = SQLITE_NOMEM_BKPT;
    اذهب پای_ټکی_د_vacuum;
  }

#که_نه_تعریف_شوی SQLITE_OMIT_AUTOVACUUM
  sqlite3_Btree_سېټ_Auto_Vacuum(p_لنډمهاله, db->بل_Autovac>=0 ? db->بل_Autovac :
                                           sqlite3BtreeGetAutoVacuum(pMain));
#د_که_پای

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
  إذا( rc!=SQLITE_OK ) اذهب پای_ټکی_د_vacuum;
  rc = execSqlF(db, pz_Err_پیغام,
      "SELECT sql FROM \"%w\".sqlite_schema"
      " WHERE type='index'",
      zDbMain
  );
  إذا( rc!=SQLITE_OK ) اذهب پای_ټکی_د_vacuum;
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
  إذا( rc!=SQLITE_OK ) اذهب پای_ټکی_د_vacuum;

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
  إذا( rc ) اذهب پای_ټکی_د_vacuum;

  /* At this point, there is a write transaction open on both the 
  ** vacuum database and the main database. Assuming no error occurs,
  ** both transactions are closed by this block - the main database
  ** transaction by sqlite3BtreeCopyFile() and the other by an explicit
  ** call to sqlite3BtreeCommit().
  */
  {
    u32 meta;
    صحيح i;

    /* This array determines which meta meta values are preserved in the
    ** vacuum.  Even entries are the meta value number and odd entries
    ** are an increment to apply to the meta value after the vacuum.
    ** The increment is used to increase the schema cookie so that other
    ** connections to the same database will know to reread the schema.
    */
    ساكن ثابت بینښه حرف aCopy[] = {
       BTREE_SCHEMA_نسخه,     1,  /* Add one to the old schema cookie */
       BTREE_DEFAULT_CACHE_اندازه, 0,  /* Preserve the default page cache size */
       BTREE_متن_ENCODING,      0,  /* Preserve the text encoding */
       BTREE_USER_نسخه,       0,  /* Preserve the user version */
       BTREE_APPLICATION_ID,     0,  /* Preserve the application id */
    };

    أكد( SQLITE_TXN_WRITE==sqlite3BtreeTxnState(p_لنډمهاله) );
    أكد( p_بهر!=0 || SQLITE_TXN_WRITE==sqlite3BtreeTxnState(pMain) );

    /* Copy Btree meta values */
    لكل(i=0; i<Array_اندازه(aCopy); i+=2){
      /* GetMeta() and UpdateMeta() cannot fail in this context because
      ** we already have page 1 loaded into cache and marked dirty. */
      sqlite3BtreeGetMeta(pMain, aCopy[i], &meta);
      rc = sqlite3BtreeUpdateMeta(p_لنډمهاله, aCopy[i], meta+aCopy[i+1]);
      إذا( NEVER(rc!=SQLITE_OK) ) اذهب پای_ټکی_د_vacuum;
    }

    إذا( p_بهر==0 ){
      rc = sqlite3BtreeCopyFile(pMain, p_لنډمهاله);
    }
    إذا( rc!=SQLITE_OK ) اذهب پای_ټکی_د_vacuum;
    rc = sqlite3BtreeCommit(p_لنډمهاله);
    إذا( rc!=SQLITE_OK ) اذهب پای_ټکی_د_vacuum;
#که_نه_تعریف_شوی SQLITE_OMIT_AUTOVACUUM
    إذا( p_بهر==0 ){
      sqlite3_Btree_سېټ_Auto_Vacuum(pMain, sqlite3BtreeGetAutoVacuum(p_لنډمهاله));
    }
#د_که_پای
  }

  أكد( rc==SQLITE_OK );
  إذا( p_بهر==0 ){
    nRes = sqlite3BtreeGetRequestedReserve(p_لنډمهاله);
    rc = sqlite3_Btree_سېټ_پاڼه_اندازه(pMain, sqlite3_Btree_Get_پاڼه_اندازه(p_لنډمهاله), nRes,1);
  }

پای_ټکی_د_vacuum:
  /* Restore the original value of db->flags */
  db->init.iDb = 0;
  db->mDbFlags = saved_mDbFlags;
  db->flags = saved_flags;
  db->nChange = saved_nChange;
  db->n_ټول_Change = saved_n_ټول_Change;
  db->mTrace = saved_mTrace;
  sqlite3_Btree_سېټ_پاڼه_اندازه(pMain, -1, 0, 1);

  /* Currently there is an SQL level transaction open on the vacuum
  ** database. No locks are held on any other files (since the main file
  ** was committed at the btree level). So it safe to end the transaction
  ** by manually setting the autoCommit flag to true and detaching the
  ** vacuum database. The vacuum_db journal file is deleted when the pager
  ** is closed by the DETACH.
  */
  db->autoCommit = 1;

  إذا( pDb ){
    sqlite3BtreeClose(pDb->pBt);
    pDb->pBt = 0;
    pDb->pSchema = 0;
  }

  /* This both clears the schemas and reduces the size of the db->aDb[]
  ** array. */ 
  sqlite3_Reset_All_Schemas_د_Connection(db);

  أرجع rc;
}

#د_که_پای  /* SQLITE_OMIT_VACUUM && SQLITE_OMIT_ATTACH */
