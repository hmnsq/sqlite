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
#అన్తర్భావ "sqliteInt.h"
#అన్తర్భావ "vdbeInt.h"

#ఒకవేళ !defined(SQLITE_OMIT_VACUUM) && !defined(SQLITE_OMIT_ATTACH)

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
స్థిర పూర్ణాంకం execSql(sqlite3 *db, అక్షరం **pz_Err_సందేశం, స్థిరరాశి అక్షరం *zSql){
  sqlite3_stmt *pStmt;
  పూర్ణాంకం rc;

  /* printf("SQL: [%s]\n", zSql); fflush(stdout); */
  rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
  ఒకవేళ( rc!=SQLITE_OK ) తిరిగి rc;
  అయితే( SQLITE_అడ్డువరుస==(rc = sqlite3_అడుగు(pStmt)) ){
    స్థిరరాశి అక్షరం *zSubSql = (స్థిరరాశి అక్షరం*)sqlite3_స్తంభం_పాఠ్యం(pStmt,0);
    ప్రతిజ్ఞా( sqlite3_strnicmp(zSql,"SELECT",6)==0 );
    /* The secondary SQL must be one of CREATE TABLE, CREATE INDEX,
    ** or INSERT.  Historically there have been attacks that first
    ** corrupt the sqlite_schema.sql field with other kinds of statements
    ** then run VACUUM to get those statements to execute at inappropriate
    ** times. */
    ఒకవేళ( zSubSql
     && (అక్షరశ్రేణి_పోలిక(zSubSql,"CRE",3)==0 || అక్షరశ్రేణి_పోలిక(zSubSql,"INS",3)==0)
    ){
      rc = execSql(db, pz_Err_సందేశం, zSubSql);
      ఒకవేళ( rc!=SQLITE_OK ) ఆపు;
    }
  }
  ప్రతిజ్ఞా( rc!=SQLITE_అడ్డువరుస );
  ఒకవేళ( rc==SQLITE_పూర్తి ) rc = SQLITE_OK;
  ఒకవేళ( rc ){
    sqlite3_సమితి_String(pz_Err_సందేశం, db, sqlite3_errmsg(db));
  }
  (శూన్య)sqlite3_finalize(pStmt);
  తిరిగి rc;
}
స్థిర పూర్ణాంకం execSqlF(sqlite3 *db, అక్షరం **pz_Err_సందేశం, స్థిరరాశి అక్షరం *zSql, ...){
  అక్షరం *z;
  va_list ap;
  పూర్ణాంకం rc;
  va_start(ap, zSql);
  z = sqlite3VMPrintf(db, zSql, ap);
  va_end(ap);
  ఒకవేళ( z==0 ) తిరిగి SQLITE_NOMEM;
  rc = execSql(db, pz_Err_సందేశం, z);
  sqlite3DbFree(db, z);
  తిరిగి rc;
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
శూన్య sqlite3Vacuum(Parse *pParse, Token *pNm, సమాసం *pInto){
  Vdbe *v = sqlite3GetVdbe(pParse);
  పూర్ణాంకం iDb = 0;
  ఒకవేళ( v==0 ) వెళ్ళు build_vacuum_చివర;
  ఒకవేళ( pParse->nErr ) వెళ్ళు build_vacuum_చివర;
  ఒకవేళ( pNm ){
#నిర్వచితం_కాకపోతే SQLITE_BUG_COMPATIBLE_20160819
    /* Default behavior:  Report an error if the argument to VACUUM is
    ** not recognized */
    iDb = sqlite3TwoPartName(pParse, pNm, pNm, &pNm);
    ఒకవేళ( iDb<0 ) వెళ్ళు build_vacuum_చివర;
#లేకపోతే
    /* When SQLITE_BUG_COMPATIBLE_20160819 is defined, unrecognized arguments
    ** to VACUUM are silently ignored.  This is a back-out of a bug fix that
    ** occurred on 2016-08-19 (https://sqlite.org/src/info/083f9e6270).
    ** The buggy behavior is required for binary compatibility with some
    ** legacy applications. */
    iDb = sqlite3_కనుగొను_Db(pParse->db, pNm);
    ఒకవేళ( iDb<0 ) iDb = 0;
#అయితే_ముగింపు
  }
  ఒకవేళ( iDb!=1 ){
    పూర్ణాంకం iIntoReg = 0;
    ఒకవేళ( pInto && sqlite3ResolveSelfReference(pParse,0,0,pInto,0)==0 ){
      iIntoReg = ++pParse->nMem;
      sqlite3_సమాసం_సంకేతం(pParse, pInto, iIntoReg);
    }
    sqlite3_Vdbe_కూడు_Op2(v, OP_Vacuum, iDb, iIntoReg);
    sqlite3_Vdbe_ఉపయోగిస్తుంది_Btree(v, iDb);
  }
build_vacuum_చివర:
  sqlite3_సమాసం_Delete(pParse->db, pInto);
  తిరిగి;
}

/*
** This routine implements the OP_Vacuum opcode of the VDBE.
*/
SQLITE_NOINLINE పూర్ణాంకం sqlite3_నిరంతరం_Vacuum(
  అక్షరం **pz_Err_సందేశం,        /* Write error message here */
  sqlite3 *db,            /* Database connection */
  పూర్ణాంకం iDb,                /* Which attached DB to vacuum */
  sqlite3_విలువ *p_బహిర్గతం     /* Write results here, if not NULL. VACUUM INTO */
){
  పూర్ణాంకం rc = SQLITE_OK;     /* Return code from service routines */
  Btree *pMain;           /* The database being vacuumed */
  Btree *p_తాత్కాలిక;           /* The temporary database we vacuum into */
  u32 saved_mDbFlags;     /* Saved value of db->mDbFlags */
  u64 saved_flags;        /* Saved value of db->flags */
  i64 saved_nChange;      /* Saved value of db->nChange */
  i64 saved_n_మొత్తం_Change; /* Saved value of db->nTotalChange */
  u32 saved_openFlags;    /* Saved value of db->openFlags */
  u8 saved_mTrace;        /* Saved trace settings */
  Db *pDb = 0;            /* Database to detach at end of vacuum */
  పూర్ణాంకం isMemDb;            /* True if vacuuming a :memory: database */
  పూర్ణాంకం nRes;               /* Bytes of reserved space at the end of each page */
  పూర్ణాంకం nDb;                /* Number of attached databases */
  స్థిరరాశి అక్షరం *zDbMain;    /* Schema name of database to vacuum */
  స్థిరరాశి అక్షరం *z_బహిర్గతం;       /* Name of output file */
  u32 pgflags = PAGER_SYNCHRONOUS_OFF; /* sync flags for output db */
  u64 iRandom;            /* Random value used for zDbVacuum[] */
  అక్షరం zDbVacuum[42];     /* Name of the ATTACH-ed database used for vacuum */


  ఒకవేళ( !db->autoCommit ){
    sqlite3_సమితి_String(pz_Err_సందేశం, db, "cannot VACUUM from within a transaction");
    తిరిగి SQLITE_ERROR; /* IMP: R-12218-18073 */
  }
  ఒకవేళ( db->n_Vdbe_చురుకైన>1 ){
    sqlite3_సమితి_String(pz_Err_సందేశం, db,"cannot VACUUM - SQL statements in progress");
    తిరిగి SQLITE_ERROR; /* IMP: R-15610-35227 */
  }
  saved_openFlags = db->openFlags;
  ఒకవేళ( p_బహిర్గతం ){
    ఒకవేళ( sqlite3_విలువ_type(p_బహిర్గతం)!=SQLITE_పాఠ్యం ){
      sqlite3_సమితి_String(pz_Err_సందేశం, db, "non-text filename");
      తిరిగి SQLITE_ERROR;
    }
    z_బహిర్గతం = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(p_బహిర్గతం);
    db->openFlags &= ~SQLITE_OPEN_READONLY;
    db->openFlags |= SQLITE_OPEN_CREATE|SQLITE_OPEN_READWRITE;
  }లేకపోతే{
    z_బహిర్గతం = "";
  }

  /* Save the current value of the database flags so that it can be 
  ** restored before returning. Then set the writable-schema flag, and
  ** disable CHECK and foreign key constraints.  */
  saved_flags = db->flags;
  saved_mDbFlags = db->mDbFlags;
  saved_nChange = db->nChange;
  saved_n_మొత్తం_Change = db->n_మొత్తం_Change;
  saved_mTrace = db->mTrace;
  db->flags |= SQLITE_WriteSchema | SQLITE_IgnoreChecks | SQLITE_Comments
               | SQLITE_AttachCreate | SQLITE_AttachWrite;
  db->mDbFlags |= DBFLAG_PreferBuiltin | DBFLAG_Vacuum;
  db->flags &= ~(u64)(SQLITE_Foreign_కీలకాలు | SQLITE_తిరగేయి_క్రమం
                   | SQLITE_Defensive | SQLITE_లెక్క_అడ్డువరుసలు);
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
  sqlite3_randomness(పరిమాణం(iRandom),&iRandom);
  sqlite3_snprintf(పరిమాణం(zDbVacuum), zDbVacuum, "vacuum_%016llx", iRandom);
  nDb = db->nDb;
  rc = execSqlF(db, pz_Err_సందేశం, "ATTACH %Q AS %s", z_బహిర్గతం, zDbVacuum);
  db->openFlags = saved_openFlags;
  ఒకవేళ( rc!=SQLITE_OK ) వెళ్ళు చివర_యొక్క_vacuum;
  ప్రతిజ్ఞా( (db->nDb-1)==nDb );
  pDb = &db->aDb[nDb];
  ప్రతిజ్ఞా( strcmp(pDb->zDbSName,zDbVacuum)==0 );
  p_తాత్కాలిక = pDb->pBt;
  nRes = sqlite3BtreeGetRequestedReserve(pMain);
  ఒకవేళ( p_బహిర్గతం ){
    sqlite3_file *id = sqlite3PagerFile(sqlite3BtreePager(p_తాత్కాలిక));
    i64 sz = 0;
    స్థిరరాశి అక్షరం *zFilename;
    ఒకవేళ( id->pMethods!=0 && (sqlite3_Os_File_కొలత(id, &sz)!=SQLITE_OK || sz>0) ){
      rc = SQLITE_ERROR;
      sqlite3_సమితి_String(pz_Err_సందేశం, db, "output file already exists");
      వెళ్ళు చివర_యొక్క_vacuum;
    }
    db->mDbFlags |= DBFLAG_VacuumInto;

    /* For a VACUUM INTO, the pager-flags are set to the same values as
    ** they are for the database being vacuumed, except that PAGER_CACHESPILL
    ** is always set. */
    pgflags = db->aDb[iDb].safety_level | (db->flags & PAGER_FLAGS_ముసుగు);

    /* If the VACUUM INTO target file is a URI filename and if the
    ** "reserve=N" query parameter is present, reset the reserve to the
    ** amount specified, if the amount is within range */
    zFilename = sqlite3BtreeGetFilename(p_తాత్కాలిక);
    ఒకవేళ( ALWAYS(zFilename) ){
      పూర్ణాంకం nNew = (పూర్ణాంకం)sqlite3_uri_int64(zFilename, "reserve", nRes);
      ఒకవేళ( nNew>=0 && nNew<=255 ) nRes = nNew;
    }
  }

  sqlite3_Btree_సమితి_Cache_కొలత(p_తాత్కాలిక, db->aDb[iDb].pSchema->cache_కొలత);
  sqlite3_Btree_సమితి_Spill_కొలత(p_తాత్కాలిక, sqlite3_Btree_సమితి_Spill_కొలత(pMain,0));
  sqlite3_Btree_సమితి_Pager_Flags(p_తాత్కాలిక, pgflags|PAGER_CACHESPILL);

  /* Begin a transaction and take an exclusive lock on the main database
  ** file. This is done before the sqlite3BtreeGetPageSize(pMain) call below,
  ** to ensure that we do not try to change the page-size on a WAL database.
  */
  rc = execSql(db, pz_Err_సందేశం, "BEGIN");
  ఒకవేళ( rc!=SQLITE_OK ) వెళ్ళు చివర_యొక్క_vacuum;
  rc = sqlite3BtreeBeginTrans(pMain, p_బహిర్గతం==0 ? 2 : 0, 0);
  ఒకవేళ( rc!=SQLITE_OK ) వెళ్ళు చివర_యొక్క_vacuum;

  /* Do not attempt to change the page size for a WAL database */
  ఒకవేళ( sqlite3_Pager_Get_Journal_బహుళకం(sqlite3BtreePager(pMain))
                                               ==PAGER_JOURNALMODE_WAL
   && p_బహిర్గతం==0
  ){
    db->తదుపరి_Pagesize = 0;
  }

  ఒకవేళ( sqlite3_Btree_సమితి_పుట_కొలత(p_తాత్కాలిక, sqlite3_Btree_Get_పుట_కొలత(pMain), nRes, 0)
   || (!isMemDb && sqlite3_Btree_సమితి_పుట_కొలత(p_తాత్కాలిక, db->తదుపరి_Pagesize, nRes, 0))
   || NEVER(db->mallocFailed)
  ){
    rc = SQLITE_NOMEM_BKPT;
    వెళ్ళు చివర_యొక్క_vacuum;
  }

#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOVACUUM
  sqlite3_Btree_సమితి_Auto_Vacuum(p_తాత్కాలిక, db->తదుపరి_Autovac>=0 ? db->తదుపరి_Autovac :
                                           sqlite3BtreeGetAutoVacuum(pMain));
#అయితే_ముగింపు

  /* Query the schema of the main database. Create a mirror schema
  ** in the temporary database.
  */
  db->init.iDb = nDb; /* force new CREATE statements into vacuum_db */
  rc = execSqlF(db, pz_Err_సందేశం,
      "SELECT sql FROM \"%w\".sqlite_schema"
      " WHERE type='table'AND name<>'sqlite_sequence'"
      " AND coalesce(rootpage,1)>0",
      zDbMain
  );
  ఒకవేళ( rc!=SQLITE_OK ) వెళ్ళు చివర_యొక్క_vacuum;
  rc = execSqlF(db, pz_Err_సందేశం,
      "SELECT sql FROM \"%w\".sqlite_schema"
      " WHERE type='index'",
      zDbMain
  );
  ఒకవేళ( rc!=SQLITE_OK ) వెళ్ళు చివర_యొక్క_vacuum;
  db->init.iDb = 0;

  /* Loop through the tables in the main database. For each, do
  ** an "INSERT INTO vacuum_db.xxx SELECT * FROM main.xxx;" to copy
  ** the contents to the temporary database.
  */
  rc = execSqlF(db, pz_Err_సందేశం,
      "SELECT'INSERT INTO %s.'||quote(name)"
      "||' SELECT*FROM\"%w\".'||quote(name)"
      "FROM %s.sqlite_schema "
      "WHERE type='table'AND coalesce(rootpage,1)>0",
      zDbVacuum, zDbMain, zDbVacuum
  );
  ప్రతిజ్ఞా( (db->mDbFlags & DBFLAG_Vacuum)!=0 );
  db->mDbFlags &= ~DBFLAG_Vacuum;
  ఒకవేళ( rc!=SQLITE_OK ) వెళ్ళు చివర_యొక్క_vacuum;

  /* Copy the triggers, views, and virtual tables from the main database
  ** over to the temporary database.  None of these objects has any
  ** associated storage, so all we have to do is copy their entries
  ** from the schema table.
  */
  rc = execSqlF(db, pz_Err_సందేశం,
      "INSERT INTO %s.sqlite_schema"
      " SELECT*FROM \"%w\".sqlite_schema"
      " WHERE type IN('view','trigger')"
      " OR(type='table'AND rootpage=0)",
      zDbVacuum, zDbMain
  );
  ఒకవేళ( rc ) వెళ్ళు చివర_యొక్క_vacuum;

  /* At this point, there is a write transaction open on both the 
  ** vacuum database and the main database. Assuming no error occurs,
  ** both transactions are closed by this block - the main database
  ** transaction by sqlite3BtreeCopyFile() and the other by an explicit
  ** call to sqlite3BtreeCommit().
  */
  {
    u32 meta;
    పూర్ణాంకం i;

    /* This array determines which meta meta values are preserved in the
    ** vacuum.  Even entries are the meta value number and odd entries
    ** are an increment to apply to the meta value after the vacuum.
    ** The increment is used to increase the schema cookie so that other
    ** connections to the same database will know to reread the schema.
    */
    స్థిర స్థిరరాశి గుర్తులేని అక్షరం aCopy[] = {
       BTREE_SCHEMA_సంస్కరణ,     1,  /* Add one to the old schema cookie */
       BTREE_DEFAULT_CACHE_కొలత, 0,  /* Preserve the default page cache size */
       BTREE_పాఠ్యం_ENCODING,      0,  /* Preserve the text encoding */
       BTREE_USER_సంస్కరణ,       0,  /* Preserve the user version */
       BTREE_APPLICATION_ID,     0,  /* Preserve the application id */
    };

    ప్రతిజ్ఞా( SQLITE_TXN_WRITE==sqlite3BtreeTxnState(p_తాత్కాలిక) );
    ప్రతిజ్ఞా( p_బహిర్గతం!=0 || SQLITE_TXN_WRITE==sqlite3BtreeTxnState(pMain) );

    /* Copy Btree meta values */
    కొరకు(i=0; i<Array_కొలత(aCopy); i+=2){
      /* GetMeta() and UpdateMeta() cannot fail in this context because
      ** we already have page 1 loaded into cache and marked dirty. */
      sqlite3BtreeGetMeta(pMain, aCopy[i], &meta);
      rc = sqlite3BtreeUpdateMeta(p_తాత్కాలిక, aCopy[i], meta+aCopy[i+1]);
      ఒకవేళ( NEVER(rc!=SQLITE_OK) ) వెళ్ళు చివర_యొక్క_vacuum;
    }

    ఒకవేళ( p_బహిర్గతం==0 ){
      rc = sqlite3BtreeCopyFile(pMain, p_తాత్కాలిక);
    }
    ఒకవేళ( rc!=SQLITE_OK ) వెళ్ళు చివర_యొక్క_vacuum;
    rc = sqlite3BtreeCommit(p_తాత్కాలిక);
    ఒకవేళ( rc!=SQLITE_OK ) వెళ్ళు చివర_యొక్క_vacuum;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOVACUUM
    ఒకవేళ( p_బహిర్గతం==0 ){
      sqlite3_Btree_సమితి_Auto_Vacuum(pMain, sqlite3BtreeGetAutoVacuum(p_తాత్కాలిక));
    }
#అయితే_ముగింపు
  }

  ప్రతిజ్ఞా( rc==SQLITE_OK );
  ఒకవేళ( p_బహిర్గతం==0 ){
    nRes = sqlite3BtreeGetRequestedReserve(p_తాత్కాలిక);
    rc = sqlite3_Btree_సమితి_పుట_కొలత(pMain, sqlite3_Btree_Get_పుట_కొలత(p_తాత్కాలిక), nRes,1);
  }

చివర_యొక్క_vacuum:
  /* Restore the original value of db->flags */
  db->init.iDb = 0;
  db->mDbFlags = saved_mDbFlags;
  db->flags = saved_flags;
  db->nChange = saved_nChange;
  db->n_మొత్తం_Change = saved_n_మొత్తం_Change;
  db->mTrace = saved_mTrace;
  sqlite3_Btree_సమితి_పుట_కొలత(pMain, -1, 0, 1);

  /* Currently there is an SQL level transaction open on the vacuum
  ** database. No locks are held on any other files (since the main file
  ** was committed at the btree level). So it safe to end the transaction
  ** by manually setting the autoCommit flag to true and detaching the
  ** vacuum database. The vacuum_db journal file is deleted when the pager
  ** is closed by the DETACH.
  */
  db->autoCommit = 1;

  ఒకవేళ( pDb ){
    sqlite3BtreeClose(pDb->pBt);
    pDb->pBt = 0;
    pDb->pSchema = 0;
  }

  /* This both clears the schemas and reduces the size of the db->aDb[]
  ** array. */ 
  sqlite3_Reset_All_Schemas_యొక్క_Connection(db);

  తిరిగి rc;
}

#అయితే_ముగింపు  /* SQLITE_OMIT_VACUUM && SQLITE_OMIT_ATTACH */
