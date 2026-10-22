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
#अन्तर्भाव "sqliteInt.h"
#अन्तर्भाव "vdbeInt.h"

#यदि !defined(SQLITE_OMIT_VACUUM) && !defined(SQLITE_OMIT_ATTACH)

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
स्थिर पूर्णाङ्क execSql(sqlite3 *db, वर्ण **pz_Err_सन्देशः, ध्रुव वर्ण *zSql){
  sqlite3_stmt *pStmt;
  पूर्णाङ्क rc;

  /* printf("SQL: [%s]\n", zSql); fflush(stdout); */
  rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
  यदि( rc!=SQLITE_OK ) प्रत्यागम rc;
  यावत्( SQLITE_ROW==(rc = sqlite3_चरणः(pStmt)) ){
    ध्रुव वर्ण *zSubSql = (ध्रुव वर्ण*)sqlite3_column_text(pStmt,0);
    प्रतिज्ञा( sqlite3_strnicmp(zSql,"SELECT",6)==0 );
    /* The secondary SQL must be one of CREATE TABLE, CREATE INDEX,
    ** or INSERT.  Historically there have been attacks that first
    ** corrupt the sqlite_schema.sql field with other kinds of statements
    ** then run VACUUM to get those statements to execute at inappropriate
    ** times. */
    यदि( zSubSql
     && (वाक्ये_तुलय(zSubSql,"CRE",3)==0 || वाक्ये_तुलय(zSubSql,"INS",3)==0)
    ){
      rc = execSql(db, pz_Err_सन्देशः, zSubSql);
      यदि( rc!=SQLITE_OK ) भङ्ग;
    }
  }
  प्रतिज्ञा( rc!=SQLITE_ROW );
  यदि( rc==SQLITE_समाप्तम् ) rc = SQLITE_OK;
  यदि( rc ){
    sqlite3_समुच्चयः_String(pz_Err_सन्देशः, db, sqlite3_errmsg(db));
  }
  (शून्य)sqlite3_finalize(pStmt);
  प्रत्यागम rc;
}
स्थिर पूर्णाङ्क execSqlF(sqlite3 *db, वर्ण **pz_Err_सन्देशः, ध्रुव वर्ण *zSql, ...){
  वर्ण *z;
  va_list ap;
  पूर्णाङ्क rc;
  va_start(ap, zSql);
  z = sqlite3VMPrintf(db, zSql, ap);
  va_end(ap);
  यदि( z==0 ) प्रत्यागम SQLITE_NOMEM;
  rc = execSql(db, pz_Err_सन्देशः, z);
  sqlite3DbFree(db, z);
  प्रत्यागम rc;
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
शून्य sqlite3Vacuum(Parse *pParse, Token *pNm, व्यञ्जकम् *pInto){
  Vdbe *v = sqlite3GetVdbe(pParse);
  पूर्णाङ्क iDb = 0;
  यदि( v==0 ) गच्छ build_vacuum_अन्तः;
  यदि( pParse->nErr ) गच्छ build_vacuum_अन्तः;
  यदि( pNm ){
#यदि_अपरिभाषितम् SQLITE_BUG_COMPATIBLE_20160819
    /* Default behavior:  Report an error if the argument to VACUUM is
    ** not recognized */
    iDb = sqlite3TwoPartName(pParse, pNm, pNm, &pNm);
    यदि( iDb<0 ) गच्छ build_vacuum_अन्तः;
#अन्यथा
    /* When SQLITE_BUG_COMPATIBLE_20160819 is defined, unrecognized arguments
    ** to VACUUM are silently ignored.  This is a back-out of a bug fix that
    ** occurred on 2016-08-19 (https://sqlite.org/src/info/083f9e6270).
    ** The buggy behavior is required for binary compatibility with some
    ** legacy applications. */
    iDb = sqlite3FindDb(pParse->db, pNm);
    यदि( iDb<0 ) iDb = 0;
#यदि_अन्तः
  }
  यदि( iDb!=1 ){
    पूर्णाङ्क iIntoReg = 0;
    यदि( pInto && sqlite3ResolveSelfReference(pParse,0,0,pInto,0)==0 ){
      iIntoReg = ++pParse->nMem;
      sqlite3_व्यञ्जकम्_संहिता(pParse, pInto, iIntoReg);
    }
    sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Vacuum, iDb, iIntoReg);
    sqlite3_Vdbe_उपयुङ्क्ते_Btree(v, iDb);
  }
build_vacuum_अन्तः:
  sqlite3_व्यञ्जकम्_Delete(pParse->db, pInto);
  प्रत्यागम;
}

/*
** This routine implements the OP_Vacuum opcode of the VDBE.
*/
SQLITE_NOINLINE पूर्णाङ्क sqlite3_सततम्_Vacuum(
  वर्ण **pz_Err_सन्देशः,        /* Write error message here */
  sqlite3 *db,            /* Database connection */
  पूर्णाङ्क iDb,                /* Which attached DB to vacuum */
  sqlite3_मानम् *p_बहिर्गमनम्     /* Write results here, if not NULL. VACUUM INTO */
){
  पूर्णाङ्क rc = SQLITE_OK;     /* Return code from service routines */
  Btree *pMain;           /* The database being vacuumed */
  Btree *p_अस्थायी;           /* The temporary database we vacuum into */
  u32 saved_mDbFlags;     /* Saved value of db->mDbFlags */
  u64 saved_flags;        /* Saved value of db->flags */
  i64 saved_nChange;      /* Saved value of db->nChange */
  i64 saved_n_योग_Change; /* Saved value of db->nTotalChange */
  u32 saved_openFlags;    /* Saved value of db->openFlags */
  u8 saved_mTrace;        /* Saved trace settings */
  Db *pDb = 0;            /* Database to detach at end of vacuum */
  पूर्णाङ्क isMemDb;            /* True if vacuuming a :memory: database */
  पूर्णाङ्क nRes;               /* Bytes of reserved space at the end of each page */
  पूर्णाङ्क nDb;                /* Number of attached databases */
  ध्रुव वर्ण *zDbMain;    /* Schema name of database to vacuum */
  ध्रुव वर्ण *z_बहिर्गमनम्;       /* Name of output file */
  u32 pgflags = PAGER_SYNCHRONOUS_OFF; /* sync flags for output db */
  u64 iRandom;            /* Random value used for zDbVacuum[] */
  वर्ण zDbVacuum[42];     /* Name of the ATTACH-ed database used for vacuum */


  यदि( !db->autoCommit ){
    sqlite3_समुच्चयः_String(pz_Err_सन्देशः, db, "cannot VACUUM from within a transaction");
    प्रत्यागम SQLITE_ERROR; /* IMP: R-12218-18073 */
  }
  यदि( db->n_Vdbe_सक्रिय>1 ){
    sqlite3_समुच्चयः_String(pz_Err_सन्देशः, db,"cannot VACUUM - SQL statements in progress");
    प्रत्यागम SQLITE_ERROR; /* IMP: R-15610-35227 */
  }
  saved_openFlags = db->openFlags;
  यदि( p_बहिर्गमनम् ){
    यदि( sqlite3_मानम्_type(p_बहिर्गमनम्)!=SQLITE_TEXT ){
      sqlite3_समुच्चयः_String(pz_Err_सन्देशः, db, "non-text filename");
      प्रत्यागम SQLITE_ERROR;
    }
    z_बहिर्गमनम् = (ध्रुव वर्ण*)sqlite3_मानम्_text(p_बहिर्गमनम्);
    db->openFlags &= ~SQLITE_OPEN_READONLY;
    db->openFlags |= SQLITE_OPEN_CREATE|SQLITE_OPEN_READWRITE;
  }अन्यथा{
    z_बहिर्गमनम् = "";
  }

  /* Save the current value of the database flags so that it can be 
  ** restored before returning. Then set the writable-schema flag, and
  ** disable CHECK and foreign key constraints.  */
  saved_flags = db->flags;
  saved_mDbFlags = db->mDbFlags;
  saved_nChange = db->nChange;
  saved_n_योग_Change = db->n_योग_Change;
  saved_mTrace = db->mTrace;
  db->flags |= SQLITE_WriteSchema | SQLITE_IgnoreChecks | SQLITE_Comments
               | SQLITE_AttachCreate | SQLITE_AttachWrite;
  db->mDbFlags |= DBFLAG_PreferBuiltin | DBFLAG_Vacuum;
  db->flags &= ~(u64)(SQLITE_Foreign_कुञ्चिकाः | SQLITE_Reverse_क्रमः
                   | SQLITE_Defensive | SQLITE_गणनम्_पङ्क्तिकाः);
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
  sqlite3_randomness(परिमाण(iRandom),&iRandom);
  sqlite3_snprintf(परिमाण(zDbVacuum), zDbVacuum, "vacuum_%016llx", iRandom);
  nDb = db->nDb;
  rc = execSqlF(db, pz_Err_सन्देशः, "ATTACH %Q AS %s", z_बहिर्गमनम्, zDbVacuum);
  db->openFlags = saved_openFlags;
  यदि( rc!=SQLITE_OK ) गच्छ अन्तः_सम्बन्धि_vacuum;
  प्रतिज्ञा( (db->nDb-1)==nDb );
  pDb = &db->aDb[nDb];
  प्रतिज्ञा( वाक्यं_तुलय(pDb->zDbSName,zDbVacuum)==0 );
  p_अस्थायी = pDb->pBt;
  nRes = sqlite3BtreeGetRequestedReserve(pMain);
  यदि( p_बहिर्गमनम् ){
    sqlite3_file *id = sqlite3PagerFile(sqlite3BtreePager(p_अस्थायी));
    i64 sz = 0;
    ध्रुव वर्ण *zFilename;
    यदि( id->pMethods!=0 && (sqlite3_Os_File_आकार(id, &sz)!=SQLITE_OK || sz>0) ){
      rc = SQLITE_ERROR;
      sqlite3_समुच्चयः_String(pz_Err_सन्देशः, db, "output file already exists");
      गच्छ अन्तः_सम्बन्धि_vacuum;
    }
    db->mDbFlags |= DBFLAG_VacuumInto;

    /* For a VACUUM INTO, the pager-flags are set to the same values as
    ** they are for the database being vacuumed, except that PAGER_CACHESPILL
    ** is always set. */
    pgflags = db->aDb[iDb].safety_level | (db->flags & PAGER_FLAGS_आवरणम्);

    /* If the VACUUM INTO target file is a URI filename and if the
    ** "reserve=N" query parameter is present, reset the reserve to the
    ** amount specified, if the amount is within range */
    zFilename = sqlite3BtreeGetFilename(p_अस्थायी);
    यदि( ALWAYS(zFilename) ){
      पूर्णाङ्क nNew = (पूर्णाङ्क)sqlite3_uri_int64(zFilename, "reserve", nRes);
      यदि( nNew>=0 && nNew<=255 ) nRes = nNew;
    }
  }

  sqlite3_Btree_समुच्चयः_Cache_आकार(p_अस्थायी, db->aDb[iDb].pSchema->cache_आकार);
  sqlite3_Btree_समुच्चयः_Spill_आकार(p_अस्थायी, sqlite3_Btree_समुच्चयः_Spill_आकार(pMain,0));
  sqlite3_Btree_समुच्चयः_Pager_Flags(p_अस्थायी, pgflags|PAGER_CACHESPILL);

  /* Begin a transaction and take an exclusive lock on the main database
  ** file. This is done before the sqlite3BtreeGetPageSize(pMain) call below,
  ** to ensure that we do not try to change the page-size on a WAL database.
  */
  rc = execSql(db, pz_Err_सन्देशः, "BEGIN");
  यदि( rc!=SQLITE_OK ) गच्छ अन्तः_सम्बन्धि_vacuum;
  rc = sqlite3BtreeBeginTrans(pMain, p_बहिर्गमनम्==0 ? 2 : 0, 0);
  यदि( rc!=SQLITE_OK ) गच्छ अन्तः_सम्बन्धि_vacuum;

  /* Do not attempt to change the page size for a WAL database */
  यदि( sqlite3_Pager_Get_Journal_बहुलकम्(sqlite3BtreePager(pMain))
                                               ==PAGER_JOURNALMODE_WAL
   && p_बहिर्गमनम्==0
  ){
    db->अग्र_Pagesize = 0;
  }

  यदि( sqlite3_Btree_समुच्चयः_Page_आकार(p_अस्थायी, sqlite3_Btree_Get_Page_आकार(pMain), nRes, 0)
   || (!isMemDb && sqlite3_Btree_समुच्चयः_Page_आकार(p_अस्थायी, db->अग्र_Pagesize, nRes, 0))
   || NEVER(db->mallocFailed)
  ){
    rc = SQLITE_NOMEM_BKPT;
    गच्छ अन्तः_सम्बन्धि_vacuum;
  }

#यदि_अपरिभाषितम् SQLITE_OMIT_AUTOVACUUM
  sqlite3_Btree_समुच्चयः_Auto_Vacuum(p_अस्थायी, db->अग्र_Autovac>=0 ? db->अग्र_Autovac :
                                           sqlite3BtreeGetAutoVacuum(pMain));
#यदि_अन्तः

  /* Query the schema of the main database. Create a mirror schema
  ** in the temporary database.
  */
  db->init.iDb = nDb; /* force new CREATE statements into vacuum_db */
  rc = execSqlF(db, pz_Err_सन्देशः,
      "SELECT sql FROM \"%w\".sqlite_schema"
      " WHERE type='table'AND name<>'sqlite_sequence'"
      " AND coalesce(rootpage,1)>0",
      zDbMain
  );
  यदि( rc!=SQLITE_OK ) गच्छ अन्तः_सम्बन्धि_vacuum;
  rc = execSqlF(db, pz_Err_सन्देशः,
      "SELECT sql FROM \"%w\".sqlite_schema"
      " WHERE type='index'",
      zDbMain
  );
  यदि( rc!=SQLITE_OK ) गच्छ अन्तः_सम्बन्धि_vacuum;
  db->init.iDb = 0;

  /* Loop through the tables in the main database. For each, do
  ** an "INSERT INTO vacuum_db.xxx SELECT * FROM main.xxx;" to copy
  ** the contents to the temporary database.
  */
  rc = execSqlF(db, pz_Err_सन्देशः,
      "SELECT'INSERT INTO %s.'||quote(name)"
      "||' SELECT*FROM\"%w\".'||quote(name)"
      "FROM %s.sqlite_schema "
      "WHERE type='table'AND coalesce(rootpage,1)>0",
      zDbVacuum, zDbMain, zDbVacuum
  );
  प्रतिज्ञा( (db->mDbFlags & DBFLAG_Vacuum)!=0 );
  db->mDbFlags &= ~DBFLAG_Vacuum;
  यदि( rc!=SQLITE_OK ) गच्छ अन्तः_सम्बन्धि_vacuum;

  /* Copy the triggers, views, and virtual tables from the main database
  ** over to the temporary database.  None of these objects has any
  ** associated storage, so all we have to do is copy their entries
  ** from the schema table.
  */
  rc = execSqlF(db, pz_Err_सन्देशः,
      "INSERT INTO %s.sqlite_schema"
      " SELECT*FROM \"%w\".sqlite_schema"
      " WHERE type IN('view','trigger')"
      " OR(type='table'AND rootpage=0)",
      zDbVacuum, zDbMain
  );
  यदि( rc ) गच्छ अन्तः_सम्बन्धि_vacuum;

  /* At this point, there is a write transaction open on both the 
  ** vacuum database and the main database. Assuming no error occurs,
  ** both transactions are closed by this block - the main database
  ** transaction by sqlite3BtreeCopyFile() and the other by an explicit
  ** call to sqlite3BtreeCommit().
  */
  {
    u32 meta;
    पूर्णाङ्क i;

    /* This array determines which meta meta values are preserved in the
    ** vacuum.  Even entries are the meta value number and odd entries
    ** are an increment to apply to the meta value after the vacuum.
    ** The increment is used to increase the schema cookie so that other
    ** connections to the same database will know to reread the schema.
    */
    स्थिर ध्रुव अचिह्नित वर्ण aCopy[] = {
       BTREE_SCHEMA_संस्करणम्,     1,  /* Add one to the old schema cookie */
       BTREE_DEFAULT_CACHE_आकार, 0,  /* Preserve the default page cache size */
       BTREE_TEXT_ENCODING,      0,  /* Preserve the text encoding */
       BTREE_USER_संस्करणम्,       0,  /* Preserve the user version */
       BTREE_APPLICATION_ID,     0,  /* Preserve the application id */
    };

    प्रतिज्ञा( SQLITE_TXN_WRITE==sqlite3BtreeTxnState(p_अस्थायी) );
    प्रतिज्ञा( p_बहिर्गमनम्!=0 || SQLITE_TXN_WRITE==sqlite3BtreeTxnState(pMain) );

    /* Copy Btree meta values */
    प्रति(i=0; i<Array_आकार(aCopy); i+=2){
      /* GetMeta() and UpdateMeta() cannot fail in this context because
      ** we already have page 1 loaded into cache and marked dirty. */
      sqlite3BtreeGetMeta(pMain, aCopy[i], &meta);
      rc = sqlite3BtreeUpdateMeta(p_अस्थायी, aCopy[i], meta+aCopy[i+1]);
      यदि( NEVER(rc!=SQLITE_OK) ) गच्छ अन्तः_सम्बन्धि_vacuum;
    }

    यदि( p_बहिर्गमनम्==0 ){
      rc = sqlite3BtreeCopyFile(pMain, p_अस्थायी);
    }
    यदि( rc!=SQLITE_OK ) गच्छ अन्तः_सम्बन्धि_vacuum;
    rc = sqlite3BtreeCommit(p_अस्थायी);
    यदि( rc!=SQLITE_OK ) गच्छ अन्तः_सम्बन्धि_vacuum;
#यदि_अपरिभाषितम् SQLITE_OMIT_AUTOVACUUM
    यदि( p_बहिर्गमनम्==0 ){
      sqlite3_Btree_समुच्चयः_Auto_Vacuum(pMain, sqlite3BtreeGetAutoVacuum(p_अस्थायी));
    }
#यदि_अन्तः
  }

  प्रतिज्ञा( rc==SQLITE_OK );
  यदि( p_बहिर्गमनम्==0 ){
    nRes = sqlite3BtreeGetRequestedReserve(p_अस्थायी);
    rc = sqlite3_Btree_समुच्चयः_Page_आकार(pMain, sqlite3_Btree_Get_Page_आकार(p_अस्थायी), nRes,1);
  }

अन्तः_सम्बन्धि_vacuum:
  /* Restore the original value of db->flags */
  db->init.iDb = 0;
  db->mDbFlags = saved_mDbFlags;
  db->flags = saved_flags;
  db->nChange = saved_nChange;
  db->n_योग_Change = saved_n_योग_Change;
  db->mTrace = saved_mTrace;
  sqlite3_Btree_समुच्चयः_Page_आकार(pMain, -1, 0, 1);

  /* Currently there is an SQL level transaction open on the vacuum
  ** database. No locks are held on any other files (since the main file
  ** was committed at the btree level). So it safe to end the transaction
  ** by manually setting the autoCommit flag to true and detaching the
  ** vacuum database. The vacuum_db journal file is deleted when the pager
  ** is closed by the DETACH.
  */
  db->autoCommit = 1;

  यदि( pDb ){
    sqlite3BtreeClose(pDb->pBt);
    pDb->pBt = 0;
    pDb->pSchema = 0;
  }

  /* This both clears the schemas and reduces the size of the db->aDb[]
  ** array. */ 
  sqlite3_Reset_All_Schemas_सम्बन्धि_Connection(db);

  प्रत्यागम rc;
}

#यदि_अन्तः  /* SQLITE_OMIT_VACUUM && SQLITE_OMIT_ATTACH */
