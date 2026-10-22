/*
** 2005 May 25
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file contains the implementation of the sqlite3_prepare()
** interface, and routines that contribute to loading the database schema
** from disk.
*/
#অন্তর্ভাব "sqliteInt.h"

/*
** Fill the InitData structure with an error message that indicates
** that the database is corrupt.
*/
স্থির শূন্য corruptSchema(
  Init_তথ্য *p_তথ্য,     /* Initialization context */
  অক্ষর **azObj,        /* Type and name of object being parsed */
  ধ্রুবক অক্ষর *zExtra   /* Error information */
){
  sqlite3 *db = p_তথ্য->db;
  যদি( db->mallocFailed ){
    p_তথ্য->rc = SQLITE_NOMEM_BKPT;
  }অন্যথা যদি( p_তথ্য->pz_Err_বার্তা[0]!=0 ){
    /* A error message has already been generated.  Do not overwrite it */
  }অন্যথা যদি( p_তথ্য->mInitFlags & (INITFLAG_AlterMask) ){
    স্থির ধ্রুবক অক্ষর *azAlterType[] = {
       "rename",
       "drop column",
       "add column",
       "drop constraint"
    };
    *p_তথ্য->pz_Err_বার্তা = sqlite3MPrintf(db, 
        "error in %s %s after %s: %s", azObj[0], azObj[1], 
        azAlterType[(p_তথ্য->mInitFlags&INITFLAG_AlterMask)-1], 
        zExtra
    );
    p_তথ্য->rc = SQLITE_ERROR;
  }অন্যথা যদি( db->flags & SQLITE_WriteSchema ){
    p_তথ্য->rc = SQLITE_CORRUPT_BKPT;
  }অন্যথা{
    অক্ষর *z;
    ধ্রুবক অক্ষর *zObj = azObj[1] ? azObj[1] : "?";
    z = sqlite3MPrintf(db, "malformed database schema (%s)", zObj);
    যদি( zExtra && zExtra[0] ) z = sqlite3MPrintf(db, "%z - %s", z, zExtra);
    *p_তথ্য->pz_Err_বার্তা = z;
    p_তথ্য->rc = SQLITE_CORRUPT_BKPT;
  }
}

/*
** Check to see if any sibling index (another index on the same table)
** of pIndex has the same root page number, and if it does, return true.
** This would indicate a corrupt schema.
*/
পূর্ণসংখ্যা sqlite3_সূচক_Has_Duplicate_মূল_Page(সূচক *p_সূচক){
  সূচক *p;
  জন্য(p=p_সূচক->pTable->p_সূচক; p; p=p->p_পরবর্তী){
    যদি( p->tnum==p_সূচক->tnum && p!=p_সূচক ) ফেরত 1;
  }
  ফেরত 0;
}

/* forward declaration */
স্থির পূর্ণসংখ্যা sqlite3Prepare(
  sqlite3 *db,              /* Database handle. */
  ধ্রুবক অক্ষর *zSql,         /* UTF-8 encoded SQL statement. */
  পূর্ণসংখ্যা nBytes,               /* Length of zSql in bytes. */
  u32 prepFlags,            /* Zero or more SQLITE_PREPARE_* flags */
  Vdbe *pReprepare,         /* VM being reprepared */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  ধ্রুবক অক্ষর **pz_লেজ       /* OUT: End of parsed string */
);


/*
** This is the callback routine for the code that initializes the
** database.  See sqlite3Init() below for additional information.
** This routine is also called from the OP_ParseSchema opcode of the VDBE.
**
** Each callback contains the following information:
**
**     argv[0] = type of object: "table", "index", "trigger", or "view".
**     argv[1] = name of thing being created
**     argv[2] = associated table if an index or trigger
**     argv[3] = root page number for table or index. 0 for trigger or view.
**     argv[4] = SQL text for the CREATE statement.
**
*/
পূর্ণসংখ্যা sqlite3InitCallback(শূন্য *pInit, পূর্ণসংখ্যা argc, অক্ষর **argv, অক্ষর **NotUsed){
  Init_তথ্য *p_তথ্য = (Init_তথ্য*)pInit;
  sqlite3 *db = p_তথ্য->db;
  পূর্ণসংখ্যা iDb = p_তথ্য->iDb;

  প্রতিজ্ঞা( argc==5 );
  UNUSED_PARAMETER2(NotUsed, argc);
  প্রতিজ্ঞা( sqlite3_mutex_held(db->mutex) );
  db->mDbFlags |= DBFLAG_EncodingFixed;
  যদি( argv==0 ) ফেরত 0;   /* Might happen if EMPTY_RESULT_CALLBACKS are on */
  p_তথ্য->nInitRow++;
  যদি( db->mallocFailed ){
    corruptSchema(p_তথ্য, argv, 0);
    ফেরত 1;
  }

  প্রতিজ্ঞা( iDb>=0 && iDb<db->nDb );
  প্রতিজ্ঞা( db->aDb[iDb].pSchema!=0 );
  যদি( argv[3]==0 ){
    corruptSchema(p_তথ্য, argv, 0);
  }অন্যথা যদি( argv[4]
         && 'c'==sqlite3UpperToLower[(unsigned অক্ষর)argv[4][0]]
         && 'r'==sqlite3UpperToLower[(unsigned অক্ষর)argv[4][1]] ){
    /* Call the parser to process a CREATE TABLE, INDEX or VIEW.
    ** But because db->init.busy is set to 1, no VDBE code is generated
    ** or executed.  All the parser does is build the internal data
    ** structures that describe the table, index, or view.
    **
    ** No other valid SQL statement, other than the variable CREATE statements,
    ** can begin with the letters "C" and "R".  Thus, it is not possible run
    ** any other kind of statement while parsing the schema, even a corrupt
    ** schema.
    */
    পূর্ণসংখ্যা rc;
    u8 saved_iDb = db->init.iDb;
    sqlite3_stmt *pStmt;
    ধ্রুবক অক্ষর *zEnd;
    TESTONLY(পূর্ণসংখ্যা rcp);            /* Return code from sqlite3_prepare() */

    প্রতিজ্ঞা( db->init.busy );
    db->init.iDb = iDb;
    যদি( sqlite3GetUInt32(argv[3], &db->init.newTnum)==0
     || (db->init.newTnum>p_তথ্য->mxPage && p_তথ্য->mxPage>0)
    ){
      যদি( sqlite3Config.bExtraSchemaChecks ){
        corruptSchema(p_তথ্য, argv, "invalid rootpage");
      }
    }
    db->init.orphanTrigger = 0;
    db->init.azInit = (ধ্রুবক অক্ষর**)argv;
    pStmt = 0;
    TESTONLY(rcp = ) sqlite3Prepare(db, argv[4], -1, 0, 0, &pStmt, &zEnd);
    rc = db->errCode;
    প্রতিজ্ঞা( (rc&0xFF)==(rcp&0xFF) );
    db->init.iDb = saved_iDb;
    /* assert( saved_iDb==0 || (db->mDbFlags & DBFLAG_Vacuum)!=0 ); */
    যদি( SQLITE_OK!=rc ){
      যদি( db->init.orphanTrigger ){
        প্রতিজ্ঞা( iDb==1 );
      }অন্যথা{
        যদি( rc > p_তথ্য->rc ) p_তথ্য->rc = rc;
        যদি( rc==SQLITE_NOMEM ){
          sqlite3OomFault(db);
        }অন্যথা যদি( rc!=SQLITE_INTERRUPT && (rc&0xFF)!=SQLITE_LOCKED ){
          corruptSchema(p_তথ্য, argv, sqlite3_errmsg(db));
        }
      }
    }অন্যথা যদি( zEnd[0] ){
      corruptSchema(p_তথ্য, argv, 0);
    }
    db->init.azInit = sqlite3StdType; /* Any array of string ptrs will do */
    sqlite3_finalize(pStmt);
  }অন্যথা যদি( argv[1]==0 || (argv[4]!=0 && argv[4][0]!=0) ){
    corruptSchema(p_তথ্য, argv, 0);
  }অন্যথা{
    /* If the SQL column is blank it means this is an index that
    ** was created to be the PRIMARY KEY or to fulfill a UNIQUE
    ** constraint for a CREATE TABLE.  The index should have already
    ** been created when we processed the CREATE TABLE.  All we have
    ** to do here is record the root page number for that index.
    */
    সূচক *p_সূচক;
    p_সূচক = sqlite3_Find_সূচক(db, argv[1], db->aDb[iDb].zDbSName);
    যদি( p_সূচক==0 ){
      corruptSchema(p_তথ্য, argv, "orphan index");
    }অন্যথা
    যদি( sqlite3GetUInt32(argv[3],&p_সূচক->tnum)==0
     || p_সূচক->tnum<2
     || p_সূচক->tnum>p_তথ্য->mxPage
     || sqlite3_সূচক_Has_Duplicate_মূল_Page(p_সূচক)
    ){
      যদি( sqlite3Config.bExtraSchemaChecks ){
        corruptSchema(p_তথ্য, argv, "invalid rootpage");
      }
    }
  }
  যদি( p_তথ্য->pz_Err_বার্তা[0]==0 ){
    Schema *pX = db->aDb[iDb].pSchema;
    যদি( pX->tblHash.গুনতি + pX->idxHash.গুনতি + pX->trigHash.গুনতি
                   > (u32)db->aLimit[SQLITE_LIMIT_SCHEMA]
    ){
      *p_তথ্য->pz_Err_বার্তা = sqlite3MPrintf(db, "too many schema objects");
      p_তথ্য->rc = SQLITE_ERROR;
      ফেরত 1;
    }
  }
  ফেরত 0;
}

/*
** Attempt to read the database schema and initialize internal
** data structures for a single database file.  The index of the
** database file is given by iDb.  iDb==0 is used for the main
** database.  iDb==1 should never be used.  iDb>=2 is used for
** auxiliary databases.  Return one of the SQLITE_ error codes to
** indicate success or failure.
*/
পূর্ণসংখ্যা sqlite3InitOne(sqlite3 *db, পূর্ণসংখ্যা iDb, অক্ষর **pz_Err_বার্তা, u32 mFlags){
  পূর্ণসংখ্যা rc;
  পূর্ণসংখ্যা i;
#ifndef SQLITE_OMIT_DEPRECATED
  পূর্ণসংখ্যা আকার_2;
#endif
  Db *pDb;
  অক্ষর ধ্রুবক *azArg[6];
  পূর্ণসংখ্যা meta[5];
  Init_তথ্য init_তথ্য;
  ধ্রুবক অক্ষর *zSchemaTabName;
  পূর্ণসংখ্যা openedTransaction = 0;
  পূর্ণসংখ্যা mask = ((db->mDbFlags & DBFLAG_EncodingFixed) | ~DBFLAG_EncodingFixed);

  প্রতিজ্ঞা( (db->mDbFlags & DBFLAG_SchemaKnownOk)==0 );
  প্রতিজ্ঞা( iDb>=0 && iDb<db->nDb );
  প্রতিজ্ঞা( db->aDb[iDb].pSchema );
  প্রতিজ্ঞা( sqlite3_mutex_held(db->mutex) );
  প্রতিজ্ঞা( iDb==1 || sqlite3BtreeHoldsMutex(db->aDb[iDb].pBt) );

  db->init.busy = 1 + ((mFlags & INITFLAG_AlterAdd)!=0);
  /* ^--- Any non-zero value for init.busy means that we are scanning
  ** the sqlite_schema table to build the internal schema representation,
  ** rather than running actual CREATE statements.  init.busy==2 has the
  ** additional meaning that the scan is happening as part of
  ** ALTER TABLE ADD COLUMN, which is stricter in its enforcement of
  ** function name resolution. */

  /* Construct the in-memory representation schema tables (sqlite_schema or
  ** sqlite_temp_schema) by invoking the parser directly.  The appropriate
  ** table name will be inserted automatically by the parser so we can just
  ** use the abbreviation "x" here.  The parser will also automatically tag
  ** the schema table as read-only. */
  azArg[0] = "table";
  azArg[1] = zSchemaTabName = SCHEMA_TABLE(iDb);
  azArg[2] = azArg[1];
  azArg[3] = "1";
  azArg[4] = "CREATE TABLE x(type text,name text,tbl_name text,"
                            "rootpage int,sql text)";
  azArg[5] = 0;
  init_তথ্য.db = db;
  init_তথ্য.iDb = iDb;
  init_তথ্য.rc = SQLITE_OK;
  init_তথ্য.pz_Err_বার্তা = pz_Err_বার্তা;
  init_তথ্য.mInitFlags = mFlags;
  init_তথ্য.nInitRow = 0;
  init_তথ্য.mxPage = 0;
  sqlite3InitCallback(&init_তথ্য, 5, (অক্ষর **)azArg, 0);
  db->mDbFlags &= mask;
  যদি( init_তথ্য.rc ){
    rc = init_তথ্য.rc;
    যাও error_out;
  }

  /* Create a cursor to hold the database open
  */
  pDb = &db->aDb[iDb];
  যদি( pDb->pBt==0 ){
    প্রতিজ্ঞা( iDb==1 );
    DbSetProperty(db, 1, DB_SchemaLoaded);
    rc = SQLITE_OK;
    যাও error_out;
  }

  /* If there is not already a read-only (or read-write) transaction opened
  ** on the b-tree database, open one now. If a transaction is opened, it 
  ** will be closed before this function returns.  */
  sqlite3BtreeEnter(pDb->pBt);
  যদি( sqlite3BtreeTxnState(pDb->pBt)==SQLITE_TXN_NONE ){
    rc = sqlite3BtreeBeginTrans(pDb->pBt, 0, 0);
    যদি( rc!=SQLITE_OK ){
      sqlite3SetString(pz_Err_বার্তা, db, sqlite3ErrStr(rc));
      যাও initone_error_out;
    }
    openedTransaction = 1;
  }

  /* Get the database meta information.
  **
  ** Meta values are as follows:
  **    meta[0]   Schema cookie.  Changes with each schema change.
  **    meta[1]   File format of schema layer.
  **    meta[2]   Size of the page cache.
  **    meta[3]   Largest rootpage (auto/incr_vacuum mode)
  **    meta[4]   Db text encoding. 1:UTF-8 2:UTF-16LE 3:UTF-16BE
  **    meta[5]   User version
  **    meta[6]   Incremental vacuum mode
  **    meta[7]   unused
  **    meta[8]   unused
  **    meta[9]   unused
  **
  ** Note: The #defined SQLITE_UTF* symbols in sqliteInt.h correspond to
  ** the possible values of meta[4].
  */
  জন্য(i=0; i<Array_আকার(meta); i++){
    sqlite3BtreeGetMeta(pDb->pBt, i+1, (u32 *)&meta[i]);
  }
  যদি( (db->flags & SQLITE_ResetDatabase)!=0 ){
    memset(meta, 0, আকার(meta));
  }
  pDb->pSchema->schema_cookie = meta[BTREE_SCHEMA_VERSION-1];

  /* If opening a non-empty database, check the text encoding. For the
  ** main database, set sqlite3.enc to the encoding of the main database.
  ** For an attached db, it is an error if the encoding is not the same
  ** as sqlite3.enc.
  */
  যদি( meta[BTREE_TEXT_ENCODING-1] ){  /* text encoding */
    যদি( iDb==0 && (db->mDbFlags & DBFLAG_EncodingFixed)==0 ){
      u8 encoding;
#ifndef SQLITE_OMIT_UTF16
      /* If opening the main database, set ENC(db). */
      encoding = (u8)meta[BTREE_TEXT_ENCODING-1] & 3;
      যদি( encoding==0 ) encoding = SQLITE_UTF8;
#অন্যথা
      encoding = SQLITE_UTF8;
#endif
      sqlite3SetTextEncoding(db, encoding);
    }অন্যথা{
      /* If opening an attached database, the encoding much match ENC(db) */
      যদি( (meta[BTREE_TEXT_ENCODING-1] & 3)!=ENC(db) ){
        sqlite3SetString(pz_Err_বার্তা, db, "attached databases must use the same"
            " text encoding as main database");
        rc = SQLITE_ERROR;
        যাও initone_error_out;
      }
    }
  }
  pDb->pSchema->enc = ENC(db);

  যদি( pDb->pSchema->cache_আকার==0 ){
#ifndef SQLITE_OMIT_DEPRECATED
    আকার_2 = sqlite3AbsInt32(meta[BTREE_DEFAULT_CACHE_আকার-1]);
    যদি( আকার_2==0 ){ আকার_2 = SQLITE_DEFAULT_CACHE_আকার; }
    pDb->pSchema->cache_আকার = আকার_2;
#অন্যথা
    pDb->pSchema->cache_আকার = SQLITE_DEFAULT_CACHE_আকার;
#endif
    sqlite3_Btree_Set_Cache_আকার(pDb->pBt, pDb->pSchema->cache_আকার);
  }

  /*
  ** file_format==1    Version 3.0.0.
  ** file_format==2    Version 3.1.3.  // ALTER TABLE ADD COLUMN
  ** file_format==3    Version 3.1.4.  // ditto but with non-NULL defaults
  ** file_format==4    Version 3.3.0.  // DESC indices.  Boolean constants
  */
  pDb->pSchema->file_format = (u8)meta[BTREE_FILE_FORMAT-1];
  যদি( pDb->pSchema->file_format==0 ){
    pDb->pSchema->file_format = 1;
  }
  যদি( pDb->pSchema->file_format>SQLITE_MAX_FILE_FORMAT ){
    sqlite3SetString(pz_Err_বার্তা, db, "unsupported file format");
    rc = SQLITE_ERROR;
    যাও initone_error_out;
  }

  /* Ticket #2804:  When we open a database in the newer file format,
  ** clear the legacy_file_format pragma flag so that a VACUUM will
  ** not downgrade the database and thus invalidate any descending
  ** indices that the user might have created.
  */
  যদি( iDb==0 && meta[BTREE_FILE_FORMAT-1]>=4 ){
    db->flags &= ~(u64)SQLITE_LegacyFileFmt;
  }

  /* Read the schema information out of the schema tables
  */
  প্রতিজ্ঞা( db->init.busy );
  init_তথ্য.mxPage = sqlite3BtreeLastPage(pDb->pBt);
  {
    অক্ষর *zSql;
    zSql = sqlite3MPrintf(db, 
        "SELECT*FROM\"%w\".%s ORDER BY rowid",
        db->aDb[iDb].zDbSName, zSchemaTabName);
#ifndef SQLITE_OMIT_AUTHORIZATION
    {
      sqlite3_xauth xAuth;
      xAuth = db->xAuth;
      db->xAuth = 0;
#endif
      rc = sqlite3_exec(db, zSql, sqlite3InitCallback, &init_তথ্য, 0);
#ifndef SQLITE_OMIT_AUTHORIZATION
      db->xAuth = xAuth;
    }
#endif
    যদি( rc==SQLITE_OK ) rc = init_তথ্য.rc;
    sqlite3DbFree(db, zSql);
#ifndef SQLITE_OMIT_ANALYZE
    যদি( rc==SQLITE_OK ){
      sqlite3AnalysisLoad(db, iDb);
    }
#endif
  }
  প্রতিজ্ঞা( pDb == &(db->aDb[iDb]) );
  যদি( db->mallocFailed ){
    rc = SQLITE_NOMEM_BKPT;
    sqlite3ResetAllSchemasOfConnection(db);
    pDb = &db->aDb[iDb];
  }অন্যথা
  যদি( rc==SQLITE_OK || ((db->flags&SQLITE_NoSchemaError) && rc!=SQLITE_NOMEM)){
    /* Hack: If the SQLITE_NoSchemaError flag is set, then consider
    ** the schema loaded, even if errors (other than OOM) occurred. In
    ** this situation the current sqlite3_prepare() operation will fail,
    ** but the following one will attempt to compile the supplied statement
    ** against whatever subset of the schema was loaded before the error
    ** occurred.
    **
    ** The primary purpose of this is to allow access to the sqlite_schema
    ** table even when its contents have been corrupted.
    */
    DbSetProperty(db, iDb, DB_SchemaLoaded);
    rc = SQLITE_OK;
  }

  /* Jump here for an error that occurs after successfully allocating
  ** curMain and calling sqlite3BtreeEnter(). For an error that occurs
  ** before that point, jump to error_out.
  */
initone_error_out:
  যদি( openedTransaction ){
    sqlite3BtreeCommit(pDb->pBt);
  }
  sqlite3BtreeLeave(pDb->pBt);

error_out:
  যদি( rc ){
    যদি( rc==SQLITE_NOMEM || rc==SQLITE_IOERR_NOMEM ){
      sqlite3OomFault(db);
    }
    sqlite3ResetOneSchema(db, iDb);
  }
  db->init.busy = 0;
  ফেরত rc;
}

/*
** Initialize all database files - the main database file, the file
** used to store temporary tables, and any additional database files
** created using ATTACH statements.  Return a success code.  If an
** error occurs, write an error message into *pzErrMsg.
**
** After a database is initialized, the DB_SchemaLoaded bit is set
** bit is set in the flags field of the Db structure. 
*/
পূর্ণসংখ্যা sqlite3Init(sqlite3 *db, অক্ষর **pz_Err_বার্তা){
  পূর্ণসংখ্যা i, rc;
  পূর্ণসংখ্যা commit_internal = !(db->mDbFlags&DBFLAG_SchemaChange);
  
  প্রতিজ্ঞা( sqlite3_mutex_held(db->mutex) );
  প্রতিজ্ঞা( sqlite3BtreeHoldsMutex(db->aDb[0].pBt) );
  প্রতিজ্ঞা( db->init.busy==0 );
  ENC(db) = SCHEMA_ENC(db);
  প্রতিজ্ঞা( db->nDb>0 );
  /* Do the main schema first */
  যদি( !DbHasProperty(db, 0, DB_SchemaLoaded) ){
    rc = sqlite3InitOne(db, 0, pz_Err_বার্তা, 0);
    যদি( rc ) ফেরত rc;
  }
  /* All other schemas after the main schema. The "temp" schema must be last */
  জন্য(i=db->nDb-1; i>0; i--){
    প্রতিজ্ঞা( i==1 || sqlite3BtreeHoldsMutex(db->aDb[i].pBt) );
    যদি( !DbHasProperty(db, i, DB_SchemaLoaded) ){
      rc = sqlite3InitOne(db, i, pz_Err_বার্তা, 0);
      যদি( rc ) ফেরত rc;
    }
  }
  যদি( commit_internal ){
    sqlite3CommitInternalChanges(db);
  }
  ফেরত SQLITE_OK;
}

/*
** This routine is a no-op if the database schema is already initialized.
** Otherwise, the schema is loaded. An error code is returned.
*/
পূর্ণসংখ্যা sqlite3ReadSchema(Parse *pParse){
  পূর্ণসংখ্যা rc = SQLITE_OK;
  sqlite3 *db = pParse->db;
  প্রতিজ্ঞা( sqlite3_mutex_held(db->mutex) );
  যদি( !db->init.busy ){
    rc = sqlite3Init(db, &pParse->z_Err_বার্তা);
    যদি( rc!=SQLITE_OK ){
      pParse->rc = rc;
      pParse->nErr++;
    }অন্যথা যদি( db->noSharedCache ){
      db->mDbFlags |= DBFLAG_SchemaKnownOk;
    }
  }
  ফেরত rc;
}


/*
** Check schema cookies in all databases.  If any cookie is out
** of date set pParse->rc to SQLITE_SCHEMA.  If all schema cookies
** make no changes to pParse->rc.
*/
স্থির শূন্য schemaIsValid(Parse *pParse){
  sqlite3 *db = pParse->db;
  পূর্ণসংখ্যা iDb;
  পূর্ণসংখ্যা rc;
  পূর্ণসংখ্যা cookie;

  প্রতিজ্ঞা( pParse->checkSchema );
  প্রতিজ্ঞা( sqlite3_mutex_held(db->mutex) );
  জন্য(iDb=0; iDb<db->nDb; iDb++){
    পূর্ণসংখ্যা openedTransaction = 0;         /* True if a transaction is opened */
    Btree *pBt = db->aDb[iDb].pBt;     /* Btree database to read cookie from */
    যদি( pBt==0 ) চালু;

    /* If there is not already a read-only (or read-write) transaction opened
    ** on the b-tree database, open one now. If a transaction is opened, it 
    ** will be closed immediately after reading the meta-value. */
    যদি( sqlite3BtreeTxnState(pBt)==SQLITE_TXN_NONE ){
      rc = sqlite3BtreeBeginTrans(pBt, 0, 0);
      যদি( rc==SQLITE_NOMEM || rc==SQLITE_IOERR_NOMEM ){
        sqlite3OomFault(db);
        pParse->rc = SQLITE_NOMEM;
      }
      যদি( rc!=SQLITE_OK ) ফেরত;
      openedTransaction = 1;
    }

    /* Read the schema cookie from the database. If it does not match the 
    ** value stored as part of the in-memory schema representation,
    ** set Parse.rc to SQLITE_SCHEMA. */
    sqlite3BtreeGetMeta(pBt, BTREE_SCHEMA_VERSION, (u32 *)&cookie);
    প্রতিজ্ঞা( sqlite3SchemaMutexHeld(db, iDb, 0) );
    যদি( cookie!=db->aDb[iDb].pSchema->schema_cookie ){
      যদি( DbHasProperty(db, iDb, DB_SchemaLoaded) ) pParse->rc = SQLITE_SCHEMA;
      sqlite3ResetOneSchema(db, iDb);
    }

    /* Close the transaction, if one was opened. */
    যদি( openedTransaction ){
      sqlite3BtreeCommit(pBt);
    }
  }
}

/*
** Convert a schema pointer into the iDb index that indicates
** which database file in db->aDb[] the schema refers to.
**
** If the same database is attached more than once, the first
** attached database is returned.
*/
পূর্ণসংখ্যা sqlite3_Schema_To_সূচক(sqlite3 *db, Schema *pSchema){
  পূর্ণসংখ্যা i = -32768;

  /* If pSchema is NULL, then return -32768. This happens when code in 
  ** expr.c is trying to resolve a reference to a transient table (i.e. one
  ** created by a sub-select). In this case the return value of this 
  ** function should never be used.
  **
  ** We return -32768 instead of the more usual -1 simply because using
  ** -32768 as the incorrect index into db->aDb[] is much 
  ** more likely to cause a segfault than -1 (of course there are assert()
  ** statements too, but it never hurts to play the odds) and
  ** -32768 will still fit into a 16-bit signed integer.
  */
  প্রতিজ্ঞা( sqlite3_mutex_held(db->mutex) );
  যদি( pSchema ){
    জন্য(i=0; 1; i++){
      প্রতিজ্ঞা( i<db->nDb );
      যদি( db->aDb[i].pSchema==pSchema ){
        বিরতি;
      }
    }
    প্রতিজ্ঞা( i>=0 && i<db->nDb );
  }
  ফেরত i;
}

/*
** Free all memory allocations in the pParse object
*/
শূন্য sqlite3ParseObjectReset(Parse *pParse){
  sqlite3 *db = pParse->db;
  প্রতিজ্ঞা( db!=0 );
  প্রতিজ্ঞা( db->pParse==pParse );
  প্রতিজ্ঞা( pParse->nested==0 );
#ifndef SQLITE_OMIT_SHARED_CACHE
  যদি( pParse->nTableLock ) sqlite3DbNNFreeNN(db, pParse->aTableLock);
#endif
  যখন( pParse->pCleanup ){
    ParseCleanup *pCleanup = pParse->pCleanup;
    pParse->pCleanup = pCleanup->p_পরবর্তী;
    pCleanup->xCleanup(db, pCleanup->pPtr);
    sqlite3DbNNFreeNN(db, pCleanup);
  }
  যদি( pParse->aLabel ) sqlite3DbNNFreeNN(db, pParse->aLabel);
  যদি( pParse->p_Const_রাশিমালা ){
    sqlite3_রাশিমালা_List_Delete(db, pParse->p_Const_রাশিমালা);
  }
  প্রতিজ্ঞা( db->lookaside.bDisable >= pParse->disableLookaside );
  db->lookaside.bDisable -= pParse->disableLookaside;
  db->lookaside.sz = db->lookaside.bDisable ? 0 : db->lookaside.szTrue;
  প্রতিজ্ঞা( pParse->db->pParse==pParse );
  db->pParse = pParse->pOuterParse;
}

/*
** Add a new cleanup operation to a Parser.  The cleanup should happen when
** the parser object is destroyed.  But, beware: the cleanup might happen
** immediately.
**
** Use this mechanism for uncommon cleanups.  There is a higher setup
** cost for this mechanism (an extra malloc), so it should not be used
** for common cleanups that happen on most calls.  But for less
** common cleanups, we save a single NULL-pointer comparison in
** sqlite3ParseObjectReset(), which reduces the total CPU cycle count.
**
** If a memory allocation error occurs, then the cleanup happens immediately.
** When either SQLITE_DEBUG or SQLITE_COVERAGE_TEST are defined, the
** pParse->earlyCleanup flag is set in that case.  Calling code show verify
** that test cases exist for which this happens, to guard against possible
** use-after-free errors following an OOM.  The preferred way to do this is
** to immediately follow the call to this routine with:
**
**       testcase( pParse->earlyCleanup );
**
** This routine returns a copy of its pPtr input (the third parameter)
** except if an early cleanup occurs, in which case it returns NULL.  So
** another way to check for early cleanup is to check the return value.
** Or, stop using the pPtr parameter with this call and use only its
** return value thereafter.  Something like this:
**
**       pObj = sqlite3ParserAddCleanup(pParse, destructor, pObj);
*/
শূন্য *sqlite3ParserAddCleanup(
  Parse *pParse,                      /* Destroy when this Parser finishes */
  শূন্য (*xCleanup)(sqlite3*,শূন্য*),   /* The cleanup routine */
  শূন্য *pPtr                          /* Pointer to object to be cleaned up */
){
  ParseCleanup *pCleanup;
  যদি( sqlite3FaultSim(300) ){
    pCleanup = 0;
    sqlite3OomFault(pParse->db);
  }অন্যথা{
    pCleanup = sqlite3DbMallocRaw(pParse->db, আকার(*pCleanup));
  }
  যদি( pCleanup ){
    pCleanup->p_পরবর্তী = pParse->pCleanup;
    pParse->pCleanup = pCleanup;
    pCleanup->pPtr = pPtr;
    pCleanup->xCleanup = xCleanup;
  }অন্যথা{
    xCleanup(pParse->db, pPtr);
    pPtr = 0;
#যদি defined(SQLITE_DEBUG) || defined(SQLITE_COVERAGE_TEST)
    pParse->earlyCleanup = 1;
#endif
  }
  ফেরত pPtr;
}

/*
** Turn bulk memory into a valid Parse object and link that Parse object
** into database connection db.
**
** Call sqlite3ParseObjectReset() to undo this operation.
**
** Caution:  Do not confuse this routine with sqlite3ParseObjectInit() which
** is generated by Lemon.
*/
শূন্য sqlite3ParseObjectInit(Parse *pParse, sqlite3 *db){
  প্রতিজ্ঞা( db!=0 );
  memset(PARSE_HDR(pParse), 0, PARSE_HDR_SZ);
  memset(PARSE_লেজ(pParse), 0, PARSE_লেজ_SZ);
  প্রতিজ্ঞা( db->pParse!=pParse );
  pParse->pOuterParse = db->pParse;
  db->pParse = pParse;
  pParse->db = db;
  যদি( db->mallocFailed ) sqlite3_Error_বার্তা(pParse, "out of memory");
}

/*
** Maximum number of times that we will try again to prepare a statement
** that returns SQLITE_ERROR_RETRY.
*/
#ifndef SQLITE_MAX_PREPARE_RETRY
# define SQLITE_MAX_PREPARE_RETRY 25
#endif

/*
** Compile the UTF-8 encoded SQL statement zSql into a statement handle.
*/
স্থির পূর্ণসংখ্যা sqlite3Prepare(
  sqlite3 *db,              /* Database handle. */
  ধ্রুবক অক্ষর *zSql,         /* UTF-8 encoded SQL statement. */
  পূর্ণসংখ্যা nBytes,               /* Length of zSql in bytes. */
  u32 prepFlags,            /* Zero or more SQLITE_PREPARE_* flags */
  Vdbe *pReprepare,         /* VM being reprepared */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  ধ্রুবক অক্ষর **pz_লেজ       /* OUT: End of parsed string */
){
  পূর্ণসংখ্যা rc = SQLITE_OK;       /* Result code */
  পূর্ণসংখ্যা i;                    /* Loop counter */
  Parse sParse;             /* Parsing context */

  /* sqlite3ParseObjectInit(&sParse, db); // inlined for performance */
  memset(PARSE_HDR(&sParse), 0, PARSE_HDR_SZ);
  memset(PARSE_লেজ(&sParse), 0, PARSE_লেজ_SZ);
  sParse.pOuterParse = db->pParse;
  db->pParse = &sParse;
  sParse.db = db;
  যদি( pReprepare ){
    sParse.pReprepare = pReprepare;
    sParse.explain = sqlite3_stmt_isexplain((sqlite3_stmt*)pReprepare);
  }অন্যথা{
    প্রতিজ্ঞা( sParse.pReprepare==0 );
  }
  প্রতিজ্ঞা( ppStmt && *ppStmt==0 );
  যদি( db->mallocFailed ){
    sqlite3_Error_বার্তা(&sParse, "out of memory");
    db->errCode = rc = SQLITE_NOMEM;
    যাও end_prepare;
  }
  প্রতিজ্ঞা( sqlite3_mutex_held(db->mutex) );

  /* For a long-term use prepared statement avoid the use of
  ** lookaside memory.
  */
  যদি( prepFlags & SQLITE_PREPARE_PERSISTENT ){
    sParse.disableLookaside++;
    DisableLookaside;
  }
  sParse.prepFlags = prepFlags & 0xff;

  /* Check to verify that it is possible to get a read lock on all
  ** database schemas.  The inability to get a read lock indicates that
  ** some other database connection is holding a write-lock, which in
  ** turn means that the other connection has made uncommitted changes
  ** to the schema.
  **
  ** Were we to proceed and prepare the statement against the uncommitted
  ** schema changes and if those schema changes are subsequently rolled
  ** back and different changes are made in their place, then when this
  ** prepared statement goes to run the schema cookie would fail to detect
  ** the schema change.  Disaster would follow.
  **
  ** This thread is currently holding mutexes on all Btrees (because
  ** of the sqlite3BtreeEnterAll() in sqlite3LockAndPrepare()) so it
  ** is not possible for another thread to start a new schema change
  ** while this routine is running.  Hence, we do not need to hold 
  ** locks on the schema, we just need to make sure nobody else is 
  ** holding them.
  **
  ** Note that setting READ_UNCOMMITTED overrides most lock detection,
  ** but it does *not* override schema lock detection, so this all still
  ** works even if READ_UNCOMMITTED is set.
  */
  যদি( !db->noSharedCache ){
    জন্য(i=0; i<db->nDb; i++) {
      Btree *pBt = db->aDb[i].pBt;
      যদি( pBt ){
        প্রতিজ্ঞা( sqlite3BtreeHoldsMutex(pBt) );
        rc = sqlite3BtreeSchemaLocked(pBt);
        যদি( rc ){
          ধ্রুবক অক্ষর *zDb = db->aDb[i].zDbSName;
          sqlite3_Error_With_বার্তা(db, rc, "database schema is locked: %s", zDb);
          testcase( db->flags & SQLITE_ReadUncommit );
          যাও end_prepare;
        }
      }
    }
  }

#ifndef SQLITE_OMIT_VIRTUALTABLE
  যদি( db->pDisconnect ) sqlite3VtabUnlockList(db);
#endif

  যদি( nBytes>=0 && (nBytes==0 || zSql[nBytes-1]!=0) ){
    অক্ষর *zSqlCopy;
    পূর্ণসংখ্যা mxLen = db->aLimit[SQLITE_LIMIT_SQL_দৈর্ঘ্য];
    testcase( nBytes==mxLen );
    testcase( nBytes==mxLen+1 );
    যদি( nBytes>mxLen ){
      sqlite3_Error_With_বার্তা(db, SQLITE_TOOBIG, "statement too long");
      rc = sqlite3ApiExit(db, SQLITE_TOOBIG);
      যাও end_prepare;
    }
    zSqlCopy = sqlite3DbStrNDup(db, zSql, nBytes);
    যদি( zSqlCopy ){
      sqlite3RunParser(&sParse, zSqlCopy);
      sParse.z_লেজ = &zSql[sParse.z_লেজ-zSqlCopy];
      sqlite3DbFree(db, zSqlCopy);
    }অন্যথা{
      sParse.z_লেজ = &zSql[nBytes];
    }
  }অন্যথা{
    sqlite3RunParser(&sParse, zSql);
  }
  প্রতিজ্ঞা( 0==sParse.nQueryLoop );

  যদি( pz_লেজ ){
    *pz_লেজ = sParse.z_লেজ;
  }

  যদি( db->init.busy==0 ){
    sqlite3VdbeSetSql(sParse.pVdbe, zSql, (পূর্ণসংখ্যা)(sParse.z_লেজ-zSql), prepFlags);
  }
  যদি( db->mallocFailed ){
    sParse.rc = SQLITE_NOMEM_BKPT;
    sParse.checkSchema = 0;
  }
  যদি( sParse.rc!=SQLITE_OK && sParse.rc!=SQLITE_DONE ){
    যদি( sParse.checkSchema && db->init.busy==0 ){
      schemaIsValid(&sParse);
    }
    যদি( sParse.pVdbe ){
      sqlite3VdbeFinalize(sParse.pVdbe);
    }
    প্রতিজ্ঞা( 0==(*ppStmt) );
    rc = sParse.rc;
    যদি( sParse.z_Err_বার্তা ){
      sqlite3_Error_With_বার্তা(db, rc, "%s", sParse.z_Err_বার্তা);
      sqlite3DbFree(db, sParse.z_Err_বার্তা);
    }অন্যথা{
      sqlite3Error(db, rc);
    }
  }অন্যথা{
    প্রতিজ্ঞা( sParse.z_Err_বার্তা==0 );
    *ppStmt = (sqlite3_stmt*)sParse.pVdbe;
    rc = SQLITE_OK;
    sqlite3ErrorClear(db);
  }


  /* Delete any TriggerPrg structures allocated while parsing this statement. */
  যখন( sParse.pTriggerPrg ){
    TriggerPrg *pT = sParse.pTriggerPrg;
    sParse.pTriggerPrg = pT->p_পরবর্তী;
    sqlite3DbFree(db, pT);
  }

end_prepare:

  sqlite3ParseObjectReset(&sParse);
  ফেরত rc;
}
স্থির পূর্ণসংখ্যা sqlite3LockAndPrepare(
  sqlite3 *db,              /* Database handle. */
  ধ্রুবক অক্ষর *zSql,         /* UTF-8 encoded SQL statement. */
  পূর্ণসংখ্যা nBytes,               /* Length of zSql in bytes. */
  u32 prepFlags,            /* Zero or more SQLITE_PREPARE_* flags */
  Vdbe *pOld,               /* VM being reprepared */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  ধ্রুবক অক্ষর **pz_লেজ       /* OUT: End of parsed string */
){
  পূর্ণসংখ্যা rc;
  পূর্ণসংখ্যা cnt = 0;

#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( ppStmt==0 ) ফেরত SQLITE_MISUSE_BKPT;
#endif
  *ppStmt = 0;
  যদি( !sqlite3SafetyCheckOk(db)||zSql==0 ){
    ফেরত SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(db->mutex);
  sqlite3BtreeEnterAll(db);
  করো{
    /* Make multiple attempts to compile the SQL, until it either succeeds
    ** or encounters a permanent error.  A schema problem after one schema
    ** reset is considered a permanent error. */
    rc = sqlite3Prepare(db, zSql, nBytes, prepFlags, pOld, ppStmt, pz_লেজ);
    প্রতিজ্ঞা( rc==SQLITE_OK || *ppStmt==0 );
    যদি( rc==SQLITE_OK || db->mallocFailed ) বিরতি;
    cnt++;
  }যখন( (rc==SQLITE_ERROR_RETRY && ALWAYS(cnt<=SQLITE_MAX_PREPARE_RETRY))
       || (rc==SQLITE_SCHEMA && (sqlite3ResetOneSchema(db,-1), cnt)==1) );
  sqlite3BtreeLeaveAll(db);
  প্রতিজ্ঞা( rc!=SQLITE_ERROR_RETRY );
  rc = sqlite3ApiExit(db, rc);
  প্রতিজ্ঞা( (rc&db->errMask)==rc );
  db->busyHandler.nBusy = 0;
  sqlite3_mutex_leave(db->mutex);
  প্রতিজ্ঞা( rc==SQLITE_OK || (*ppStmt)==0 );
  ফেরত rc;
}


/*
** Rerun the compilation of a statement after a schema change.
**
** If the statement is successfully recompiled, return SQLITE_OK. Otherwise,
** if the statement cannot be recompiled because another connection has
** locked the sqlite3_schema table, return SQLITE_LOCKED. If any other error
** occurs, return SQLITE_SCHEMA.
*/
পূর্ণসংখ্যা sqlite3Reprepare(Vdbe *p){
  পূর্ণসংখ্যা rc;
  sqlite3_stmt *pNew;
  ধ্রুবক অক্ষর *zSql;
  sqlite3 *db;
  u8 prepFlags;

  প্রতিজ্ঞা( sqlite3_mutex_held(sqlite3VdbeDb(p)->mutex) );
  zSql = sqlite3_sql((sqlite3_stmt *)p);
  প্রতিজ্ঞা( zSql!=0 );  /* Reprepare only called for prepare_v2() statements */
  db = sqlite3VdbeDb(p);
  প্রতিজ্ঞা( sqlite3_mutex_held(db->mutex) );
  prepFlags = sqlite3VdbePrepareFlags(p);
  rc = sqlite3LockAndPrepare(db, zSql, -1, prepFlags, p, &pNew, 0);
  যদি( rc ){
    যদি( rc==SQLITE_NOMEM ){
      sqlite3OomFault(db);
    }
    প্রতিজ্ঞা( pNew==0 );
    ফেরত rc;
  }অন্যথা{
    প্রতিজ্ঞা( pNew!=0 );
  }
  sqlite3VdbeSwap((Vdbe*)pNew, p);
  sqlite3TransferBindings(pNew, (sqlite3_stmt*)p);
  sqlite3_Vdbe_Reset_Step_ফলাফল((Vdbe*)pNew);
  sqlite3VdbeFinalize((Vdbe*)pNew);
  ফেরত SQLITE_OK;
}


/*
** Two versions of the official API.  Legacy and new use.  In the legacy
** version, the original SQL text is not saved in the prepared statement
** and so if a schema change occurs, SQLITE_SCHEMA is returned by
** sqlite3_step().  In the new version, the original SQL text is retained
** and the statement is automatically recompiled if an schema change
** occurs.
*/
পূর্ণসংখ্যা sqlite3_prepare(
  sqlite3 *db,              /* Database handle. */
  ধ্রুবক অক্ষর *zSql,         /* UTF-8 encoded SQL statement. */
  পূর্ণসংখ্যা nBytes,               /* Length of zSql in bytes. */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  ধ্রুবক অক্ষর **pz_লেজ       /* OUT: End of parsed string */
){
  পূর্ণসংখ্যা rc;
  rc = sqlite3LockAndPrepare(db,zSql,nBytes,0,0,ppStmt,pz_লেজ);
  প্রতিজ্ঞা( rc==SQLITE_OK || ppStmt==0 || *ppStmt==0 );  /* VERIFY: F13021 */
  ফেরত rc;
}
পূর্ণসংখ্যা sqlite3_prepare_v2(
  sqlite3 *db,              /* Database handle. */
  ধ্রুবক অক্ষর *zSql,         /* UTF-8 encoded SQL statement. */
  পূর্ণসংখ্যা nBytes,               /* Length of zSql in bytes. */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  ধ্রুবক অক্ষর **pz_লেজ       /* OUT: End of parsed string */
){
  পূর্ণসংখ্যা rc;
  /* EVIDENCE-OF: R-37923-12173 The sqlite3_prepare_v2() interface works
  ** exactly the same as sqlite3_prepare_v3() with a zero prepFlags
  ** parameter.
  **
  ** Proof in that the 5th parameter to sqlite3LockAndPrepare is 0 */
  rc = sqlite3LockAndPrepare(db,zSql,nBytes,SQLITE_PREPARE_SAVESQL,0,
                             ppStmt,pz_লেজ);
  প্রতিজ্ঞা( rc==SQLITE_OK || ppStmt==0 || *ppStmt==0 );
  ফেরত rc;
}
পূর্ণসংখ্যা sqlite3_prepare_v3(
  sqlite3 *db,              /* Database handle. */
  ধ্রুবক অক্ষর *zSql,         /* UTF-8 encoded SQL statement. */
  পূর্ণসংখ্যা nBytes,               /* Length of zSql in bytes. */
  unsigned পূর্ণসংখ্যা prepFlags,   /* Zero or more SQLITE_PREPARE_* flags */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  ধ্রুবক অক্ষর **pz_লেজ       /* OUT: End of parsed string */
){
  পূর্ণসংখ্যা rc;
  /* EVIDENCE-OF: R-56861-42673 sqlite3_prepare_v3() differs from
  ** sqlite3_prepare_v2() only in having the extra prepFlags parameter,
  ** which is a bit array consisting of zero or more of the
  ** SQLITE_PREPARE_* flags.
  **
  ** Proof by comparison to the implementation of sqlite3_prepare_v2()
  ** directly above. */
  rc = sqlite3LockAndPrepare(db,zSql,nBytes,
                 SQLITE_PREPARE_SAVESQL|(prepFlags&SQLITE_PREPARE_MASK),
                 0,ppStmt,pz_লেজ);
  প্রতিজ্ঞা( rc==SQLITE_OK || ppStmt==0 || *ppStmt==0 );
  ফেরত rc;
}


#ifndef SQLITE_OMIT_UTF16
/*
** Compile the UTF-16 encoded SQL statement zSql into a statement handle.
*/
স্থির পূর্ণসংখ্যা sqlite3Prepare16(
  sqlite3 *db,              /* Database handle. */ 
  ধ্রুবক শূন্য *zSql,         /* UTF-16 encoded SQL statement. */
  পূর্ণসংখ্যা nBytes,               /* Length of zSql in bytes. */
  u32 prepFlags,            /* Zero or more SQLITE_PREPARE_* flags */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  ধ্রুবক শূন্য **pz_লেজ       /* OUT: End of parsed string */
){
  /* This function currently works by first transforming the UTF-16
  ** encoded string to UTF-8, then invoking sqlite3_prepare(). The
  ** tricky bit is figuring out the pointer to return in *pzTail.
  */
  অক্ষর *zSql8;
  ধ্রুবক অক্ষর *zTail8 = 0;
  পূর্ণসংখ্যা rc = SQLITE_OK;

#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( ppStmt==0 ) ফেরত SQLITE_MISUSE_BKPT;
#endif
  *ppStmt = 0;
  যদি( !sqlite3SafetyCheckOk(db)||zSql==0 ){
    ফেরত SQLITE_MISUSE_BKPT;
  }

  /* Make sure nBytes is non-negative and correct.  It should be the
  ** number of bytes until the end of the input buffer or until the first
  ** U+0000 character.  If the input nBytes is odd, convert it into
  ** an even number.  If the input nBytes is negative, then the input
  ** must be terminated by at least one U+0000 character */
  যদি( nBytes>=0 ){
    পূর্ণসংখ্যা sz;
    ধ্রুবক অক্ষর *z = (ধ্রুবক অক্ষর*)zSql;
    জন্য(sz=0; sz<nBytes && (z[sz]!=0 || z[sz+1]!=0); sz += 2){}
    nBytes = sz;
  }অন্যথা{
    পূর্ণসংখ্যা sz;
    ধ্রুবক অক্ষর *z = (ধ্রুবক অক্ষর*)zSql;
    জন্য(sz=0; z[sz]!=0 || z[sz+1]!=0; sz += 2){}
    nBytes = sz;
  }

  sqlite3_mutex_enter(db->mutex);
  zSql8 = sqlite3Utf16to8(db, zSql, nBytes, SQLITE_UTF16NATIVE);
  যদি( zSql8 ){
    rc = sqlite3LockAndPrepare(db, zSql8, -1, prepFlags, 0, ppStmt, &zTail8);
  }

  যদি( zTail8 && pz_লেজ ){
    /* If sqlite3_prepare returns a tail pointer, we calculate the
    ** equivalent pointer into the UTF-16 string by counting the unicode
    ** characters between zSql8 and zTail8, and then returning a pointer
    ** the same number of characters into the UTF-16 string.
    */
    পূর্ণসংখ্যা chars_parsed = sqlite3Utf8CharLen(zSql8, (পূর্ণসংখ্যা)(zTail8-zSql8));
    *pz_লেজ = (u8 *)zSql + sqlite3Utf16ByteLen(zSql, nBytes, chars_parsed);
  }
  sqlite3DbFree(db, zSql8); 
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  ফেরত rc;
}

/*
** Two versions of the official API.  Legacy and new use.  In the legacy
** version, the original SQL text is not saved in the prepared statement
** and so if a schema change occurs, SQLITE_SCHEMA is returned by
** sqlite3_step().  In the new version, the original SQL text is retained
** and the statement is automatically recompiled if an schema change
** occurs.
*/
পূর্ণসংখ্যা sqlite3_prepare16(
  sqlite3 *db,              /* Database handle. */ 
  ধ্রুবক শূন্য *zSql,         /* UTF-16 encoded SQL statement. */
  পূর্ণসংখ্যা nBytes,               /* Length of zSql in bytes. */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  ধ্রুবক শূন্য **pz_লেজ       /* OUT: End of parsed string */
){
  পূর্ণসংখ্যা rc;
  rc = sqlite3Prepare16(db,zSql,nBytes&~1,0,ppStmt,pz_লেজ);
  প্রতিজ্ঞা( rc==SQLITE_OK || ppStmt==0 || *ppStmt==0 );  /* VERIFY: F13021 */
  ফেরত rc;
}
পূর্ণসংখ্যা sqlite3_prepare16_v2(
  sqlite3 *db,              /* Database handle. */ 
  ধ্রুবক শূন্য *zSql,         /* UTF-16 encoded SQL statement. */
  পূর্ণসংখ্যা nBytes,               /* Length of zSql in bytes. */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  ধ্রুবক শূন্য **pz_লেজ       /* OUT: End of parsed string */
){
  পূর্ণসংখ্যা rc;
  rc = sqlite3Prepare16(db,zSql,nBytes&~1,SQLITE_PREPARE_SAVESQL,ppStmt,pz_লেজ);
  প্রতিজ্ঞা( rc==SQLITE_OK || ppStmt==0 || *ppStmt==0 );  /* VERIFY: F13021 */
  ফেরত rc;
}
পূর্ণসংখ্যা sqlite3_prepare16_v3(
  sqlite3 *db,              /* Database handle. */ 
  ধ্রুবক শূন্য *zSql,         /* UTF-16 encoded SQL statement. */
  পূর্ণসংখ্যা nBytes,               /* Length of zSql in bytes. */
  unsigned পূর্ণসংখ্যা prepFlags,   /* Zero or more SQLITE_PREPARE_* flags */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  ধ্রুবক শূন্য **pz_লেজ       /* OUT: End of parsed string */
){
  পূর্ণসংখ্যা rc;
  rc = sqlite3Prepare16(db,zSql,nBytes&~1,
         SQLITE_PREPARE_SAVESQL|(prepFlags&SQLITE_PREPARE_MASK),
         ppStmt,pz_লেজ);
  প্রতিজ্ঞা( rc==SQLITE_OK || ppStmt==0 || *ppStmt==0 );  /* VERIFY: F13021 */
  ফেরত rc;
}

#endif /* SQLITE_OMIT_UTF16 */
