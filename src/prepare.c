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
#अन्तर्भाव "sqliteInt.h"

/*
** Fill the InitData structure with an error message that indicates
** that the database is corrupt.
*/
स्थिर शून्य corruptSchema(
  InitData *pData,     /* Initialization context */
  वर्ण **azObj,        /* Type and name of object being parsed */
  ध्रुव वर्ण *zExtra   /* Error information */
){
  sqlite3 *db = pData->db;
  जइ( db->mallocFailed ){
    pData->rc = SQLITE_NOMEM_BKPT;
  }अण्णहा जइ( pData->pz_Err_सन्देशः[0]!=0 ){
    /* A error message has already been generated.  Do not overwrite it */
  }अण्णहा जइ( pData->mInitFlags & (INITFLAG_AlterMask) ){
    स्थिर ध्रुव वर्ण *azAlterType[] = {
       "rename",
       "drop column",
       "add column",
       "drop constraint"
    };
    *pData->pz_Err_सन्देशः = sqlite3MPrintf(db, 
        "error in %s %s after %s: %s", azObj[0], azObj[1], 
        azAlterType[(pData->mInitFlags&INITFLAG_AlterMask)-1], 
        zExtra
    );
    pData->rc = SQLITE_ERROR;
  }अण्णहा जइ( db->flags & SQLITE_WriteSchema ){
    pData->rc = SQLITE_CORRUPT_BKPT;
  }अण्णहा{
    वर्ण *z;
    ध्रुव वर्ण *zObj = azObj[1] ? azObj[1] : "?";
    z = sqlite3MPrintf(db, "malformed database schema (%s)", zObj);
    जइ( zExtra && zExtra[0] ) z = sqlite3MPrintf(db, "%z - %s", z, zExtra);
    *pData->pz_Err_सन्देशः = z;
    pData->rc = SQLITE_CORRUPT_BKPT;
  }
}

/*
** Check to see if any sibling index (another index on the same table)
** of pIndex has the same root page number, and if it does, return true.
** This would indicate a corrupt schema.
*/
पूर्णाङ्क sqlite3_Index_Has_Duplicate_मूलम्_Page(Index *pIndex){
  Index *p;
  प्रति(p=pIndex->pTable->pIndex; p; p=p->pNext){
    जइ( p->tnum==pIndex->tnum && p!=pIndex ) प्रत्यागम 1;
  }
  प्रत्यागम 0;
}

/* forward declaration */
स्थिर पूर्णाङ्क sqlite3Prepare(
  sqlite3 *db,              /* Database handle. */
  ध्रुव वर्ण *zSql,         /* UTF-8 encoded SQL statement. */
  पूर्णाङ्क nBytes,               /* Length of zSql in bytes. */
  u32 prepFlags,            /* Zero or more SQLITE_PREPARE_* flags */
  Vdbe *pReprepare,         /* VM being reprepared */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  ध्रुव वर्ण **pzTail       /* OUT: End of parsed string */
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
पूर्णाङ्क sqlite3InitCallback(शून्य *pInit, पूर्णाङ्क argc, वर्ण **argv, वर्ण **NotUsed){
  InitData *pData = (InitData*)pInit;
  sqlite3 *db = pData->db;
  पूर्णाङ्क iDb = pData->iDb;

  प्रतिज्ञा( argc==5 );
  UNUSED_PARAMETER2(NotUsed, argc);
  प्रतिज्ञा( sqlite3_mutex_held(db->mutex) );
  db->mDbFlags |= DBFLAG_EncodingFixed;
  जइ( argv==0 ) प्रत्यागम 0;   /* Might happen if EMPTY_RESULT_CALLBACKS are on */
  pData->nInitRow++;
  जइ( db->mallocFailed ){
    corruptSchema(pData, argv, 0);
    प्रत्यागम 1;
  }

  प्रतिज्ञा( iDb>=0 && iDb<db->nDb );
  प्रतिज्ञा( db->aDb[iDb].pSchema!=0 );
  जइ( argv[3]==0 ){
    corruptSchema(pData, argv, 0);
  }अण्णहा जइ( argv[4]
         && 'c'==sqlite3UpperToLower[(unsigned वर्ण)argv[4][0]]
         && 'r'==sqlite3UpperToLower[(unsigned वर्ण)argv[4][1]] ){
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
    पूर्णाङ्क rc;
    u8 saved_iDb = db->init.iDb;
    sqlite3_stmt *pStmt;
    ध्रुव वर्ण *zEnd;
    TESTONLY(पूर्णाङ्क rcp);            /* Return code from sqlite3_prepare() */

    प्रतिज्ञा( db->init.busy );
    db->init.iDb = iDb;
    जइ( sqlite3GetUInt32(argv[3], &db->init.newTnum)==0
     || (db->init.newTnum>pData->mxPage && pData->mxPage>0)
    ){
      जइ( sqlite3Config.bExtraSchemaChecks ){
        corruptSchema(pData, argv, "invalid rootpage");
      }
    }
    db->init.orphanTrigger = 0;
    db->init.azInit = (ध्रुव वर्ण**)argv;
    pStmt = 0;
    TESTONLY(rcp = ) sqlite3Prepare(db, argv[4], -1, 0, 0, &pStmt, &zEnd);
    rc = db->errCode;
    प्रतिज्ञा( (rc&0xFF)==(rcp&0xFF) );
    db->init.iDb = saved_iDb;
    /* assert( saved_iDb==0 || (db->mDbFlags & DBFLAG_Vacuum)!=0 ); */
    जइ( SQLITE_OK!=rc ){
      जइ( db->init.orphanTrigger ){
        प्रतिज्ञा( iDb==1 );
      }अण्णहा{
        जइ( rc > pData->rc ) pData->rc = rc;
        जइ( rc==SQLITE_NOMEM ){
          sqlite3OomFault(db);
        }अण्णहा जइ( rc!=SQLITE_INTERRUPT && (rc&0xFF)!=SQLITE_LOCKED ){
          corruptSchema(pData, argv, sqlite3_errmsg(db));
        }
      }
    }अण्णहा जइ( zEnd[0] ){
      corruptSchema(pData, argv, 0);
    }
    db->init.azInit = sqlite3StdType; /* Any array of string ptrs will do */
    sqlite3_finalize(pStmt);
  }अण्णहा जइ( argv[1]==0 || (argv[4]!=0 && argv[4][0]!=0) ){
    corruptSchema(pData, argv, 0);
  }अण्णहा{
    /* If the SQL column is blank it means this is an index that
    ** was created to be the PRIMARY KEY or to fulfill a UNIQUE
    ** constraint for a CREATE TABLE.  The index should have already
    ** been created when we processed the CREATE TABLE.  All we have
    ** to do here is record the root page number for that index.
    */
    Index *pIndex;
    pIndex = sqlite3FindIndex(db, argv[1], db->aDb[iDb].zDbSName);
    जइ( pIndex==0 ){
      corruptSchema(pData, argv, "orphan index");
    }अण्णहा
    जइ( sqlite3GetUInt32(argv[3],&pIndex->tnum)==0
     || pIndex->tnum<2
     || pIndex->tnum>pData->mxPage
     || sqlite3_Index_Has_Duplicate_मूलम्_Page(pIndex)
    ){
      जइ( sqlite3Config.bExtraSchemaChecks ){
        corruptSchema(pData, argv, "invalid rootpage");
      }
    }
  }
  जइ( pData->pz_Err_सन्देशः[0]==0 ){
    Schema *pX = db->aDb[iDb].pSchema;
    जइ( pX->tblHash.गणनम् + pX->idxHash.गणनम् + pX->trigHash.गणनम्
                   > (u32)db->aLimit[SQLITE_LIMIT_SCHEMA]
    ){
      *pData->pz_Err_सन्देशः = sqlite3MPrintf(db, "too many schema objects");
      pData->rc = SQLITE_ERROR;
      प्रत्यागम 1;
    }
  }
  प्रत्यागम 0;
}

/*
** Attempt to read the database schema and initialize internal
** data structures for a single database file.  The index of the
** database file is given by iDb.  iDb==0 is used for the main
** database.  iDb==1 should never be used.  iDb>=2 is used for
** auxiliary databases.  Return one of the SQLITE_ error codes to
** indicate success or failure.
*/
पूर्णाङ्क sqlite3InitOne(sqlite3 *db, पूर्णाङ्क iDb, वर्ण **pz_Err_सन्देशः, u32 mFlags){
  पूर्णाङ्क rc;
  पूर्णाङ्क i;
#ifndef SQLITE_OMIT_DEPRECATED
  पूर्णाङ्क size;
#endif
  Db *pDb;
  वर्ण ध्रुव *azArg[6];
  पूर्णाङ्क meta[5];
  InitData initData;
  ध्रुव वर्ण *zSchemaTabName;
  पूर्णाङ्क openedTransaction = 0;
  पूर्णाङ्क mask = ((db->mDbFlags & DBFLAG_EncodingFixed) | ~DBFLAG_EncodingFixed);

  प्रतिज्ञा( (db->mDbFlags & DBFLAG_SchemaKnownOk)==0 );
  प्रतिज्ञा( iDb>=0 && iDb<db->nDb );
  प्रतिज्ञा( db->aDb[iDb].pSchema );
  प्रतिज्ञा( sqlite3_mutex_held(db->mutex) );
  प्रतिज्ञा( iDb==1 || sqlite3BtreeHoldsMutex(db->aDb[iDb].pBt) );

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
  initData.db = db;
  initData.iDb = iDb;
  initData.rc = SQLITE_OK;
  initData.pz_Err_सन्देशः = pz_Err_सन्देशः;
  initData.mInitFlags = mFlags;
  initData.nInitRow = 0;
  initData.mxPage = 0;
  sqlite3InitCallback(&initData, 5, (वर्ण **)azArg, 0);
  db->mDbFlags &= mask;
  जइ( initData.rc ){
    rc = initData.rc;
    गच्छ error_out;
  }

  /* Create a cursor to hold the database open
  */
  pDb = &db->aDb[iDb];
  जइ( pDb->pBt==0 ){
    प्रतिज्ञा( iDb==1 );
    DbSetProperty(db, 1, DB_SchemaLoaded);
    rc = SQLITE_OK;
    गच्छ error_out;
  }

  /* If there is not already a read-only (or read-write) transaction opened
  ** on the b-tree database, open one now. If a transaction is opened, it 
  ** will be closed before this function returns.  */
  sqlite3BtreeEnter(pDb->pBt);
  जइ( sqlite3BtreeTxnState(pDb->pBt)==SQLITE_TXN_NONE ){
    rc = sqlite3BtreeBeginTrans(pDb->pBt, 0, 0);
    जइ( rc!=SQLITE_OK ){
      sqlite3SetString(pz_Err_सन्देशः, db, sqlite3ErrStr(rc));
      गच्छ initone_error_out;
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
  प्रति(i=0; i<ArraySize(meta); i++){
    sqlite3BtreeGetMeta(pDb->pBt, i+1, (u32 *)&meta[i]);
  }
  जइ( (db->flags & SQLITE_ResetDatabase)!=0 ){
    memset(meta, 0, परिमाण(meta));
  }
  pDb->pSchema->schema_cookie = meta[BTREE_SCHEMA_VERSION-1];

  /* If opening a non-empty database, check the text encoding. For the
  ** main database, set sqlite3.enc to the encoding of the main database.
  ** For an attached db, it is an error if the encoding is not the same
  ** as sqlite3.enc.
  */
  जइ( meta[BTREE_TEXT_ENCODING-1] ){  /* text encoding */
    जइ( iDb==0 && (db->mDbFlags & DBFLAG_EncodingFixed)==0 ){
      u8 encoding;
#ifndef SQLITE_OMIT_UTF16
      /* If opening the main database, set ENC(db). */
      encoding = (u8)meta[BTREE_TEXT_ENCODING-1] & 3;
      जइ( encoding==0 ) encoding = SQLITE_UTF8;
#अण्णहा
      encoding = SQLITE_UTF8;
#endif
      sqlite3SetTextEncoding(db, encoding);
    }अण्णहा{
      /* If opening an attached database, the encoding much match ENC(db) */
      जइ( (meta[BTREE_TEXT_ENCODING-1] & 3)!=ENC(db) ){
        sqlite3SetString(pz_Err_सन्देशः, db, "attached databases must use the same"
            " text encoding as main database");
        rc = SQLITE_ERROR;
        गच्छ initone_error_out;
      }
    }
  }
  pDb->pSchema->enc = ENC(db);

  जइ( pDb->pSchema->cache_size==0 ){
#ifndef SQLITE_OMIT_DEPRECATED
    size = sqlite3AbsInt32(meta[BTREE_DEFAULT_CACHE_SIZE-1]);
    जइ( size==0 ){ size = SQLITE_DEFAULT_CACHE_SIZE; }
    pDb->pSchema->cache_size = size;
#अण्णहा
    pDb->pSchema->cache_size = SQLITE_DEFAULT_CACHE_SIZE;
#endif
    sqlite3BtreeSetCacheSize(pDb->pBt, pDb->pSchema->cache_size);
  }

  /*
  ** file_format==1    Version 3.0.0.
  ** file_format==2    Version 3.1.3.  // ALTER TABLE ADD COLUMN
  ** file_format==3    Version 3.1.4.  // ditto but with non-NULL defaults
  ** file_format==4    Version 3.3.0.  // DESC indices.  Boolean constants
  */
  pDb->pSchema->file_format = (u8)meta[BTREE_FILE_FORMAT-1];
  जइ( pDb->pSchema->file_format==0 ){
    pDb->pSchema->file_format = 1;
  }
  जइ( pDb->pSchema->file_format>SQLITE_MAX_FILE_FORMAT ){
    sqlite3SetString(pz_Err_सन्देशः, db, "unsupported file format");
    rc = SQLITE_ERROR;
    गच्छ initone_error_out;
  }

  /* Ticket #2804:  When we open a database in the newer file format,
  ** clear the legacy_file_format pragma flag so that a VACUUM will
  ** not downgrade the database and thus invalidate any descending
  ** indices that the user might have created.
  */
  जइ( iDb==0 && meta[BTREE_FILE_FORMAT-1]>=4 ){
    db->flags &= ~(u64)SQLITE_LegacyFileFmt;
  }

  /* Read the schema information out of the schema tables
  */
  प्रतिज्ञा( db->init.busy );
  initData.mxPage = sqlite3BtreeLastPage(pDb->pBt);
  {
    वर्ण *zSql;
    zSql = sqlite3MPrintf(db, 
        "SELECT*FROM\"%w\".%s ORDER BY rowid",
        db->aDb[iDb].zDbSName, zSchemaTabName);
#ifndef SQLITE_OMIT_AUTHORIZATION
    {
      sqlite3_xauth xAuth;
      xAuth = db->xAuth;
      db->xAuth = 0;
#endif
      rc = sqlite3_exec(db, zSql, sqlite3InitCallback, &initData, 0);
#ifndef SQLITE_OMIT_AUTHORIZATION
      db->xAuth = xAuth;
    }
#endif
    जइ( rc==SQLITE_OK ) rc = initData.rc;
    sqlite3DbFree(db, zSql);
#ifndef SQLITE_OMIT_ANALYZE
    जइ( rc==SQLITE_OK ){
      sqlite3AnalysisLoad(db, iDb);
    }
#endif
  }
  प्रतिज्ञा( pDb == &(db->aDb[iDb]) );
  जइ( db->mallocFailed ){
    rc = SQLITE_NOMEM_BKPT;
    sqlite3ResetAllSchemasOfConnection(db);
    pDb = &db->aDb[iDb];
  }अण्णहा
  जइ( rc==SQLITE_OK || ((db->flags&SQLITE_NoSchemaError) && rc!=SQLITE_NOMEM)){
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
  जइ( openedTransaction ){
    sqlite3BtreeCommit(pDb->pBt);
  }
  sqlite3BtreeLeave(pDb->pBt);

error_out:
  जइ( rc ){
    जइ( rc==SQLITE_NOMEM || rc==SQLITE_IOERR_NOMEM ){
      sqlite3OomFault(db);
    }
    sqlite3ResetOneSchema(db, iDb);
  }
  db->init.busy = 0;
  प्रत्यागम rc;
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
पूर्णाङ्क sqlite3Init(sqlite3 *db, वर्ण **pz_Err_सन्देशः){
  पूर्णाङ्क i, rc;
  पूर्णाङ्क commit_internal = !(db->mDbFlags&DBFLAG_SchemaChange);
  
  प्रतिज्ञा( sqlite3_mutex_held(db->mutex) );
  प्रतिज्ञा( sqlite3BtreeHoldsMutex(db->aDb[0].pBt) );
  प्रतिज्ञा( db->init.busy==0 );
  ENC(db) = SCHEMA_ENC(db);
  प्रतिज्ञा( db->nDb>0 );
  /* Do the main schema first */
  जइ( !DbHasProperty(db, 0, DB_SchemaLoaded) ){
    rc = sqlite3InitOne(db, 0, pz_Err_सन्देशः, 0);
    जइ( rc ) प्रत्यागम rc;
  }
  /* All other schemas after the main schema. The "temp" schema must be last */
  प्रति(i=db->nDb-1; i>0; i--){
    प्रतिज्ञा( i==1 || sqlite3BtreeHoldsMutex(db->aDb[i].pBt) );
    जइ( !DbHasProperty(db, i, DB_SchemaLoaded) ){
      rc = sqlite3InitOne(db, i, pz_Err_सन्देशः, 0);
      जइ( rc ) प्रत्यागम rc;
    }
  }
  जइ( commit_internal ){
    sqlite3CommitInternalChanges(db);
  }
  प्रत्यागम SQLITE_OK;
}

/*
** This routine is a no-op if the database schema is already initialized.
** Otherwise, the schema is loaded. An error code is returned.
*/
पूर्णाङ्क sqlite3ReadSchema(Parse *pParse){
  पूर्णाङ्क rc = SQLITE_OK;
  sqlite3 *db = pParse->db;
  प्रतिज्ञा( sqlite3_mutex_held(db->mutex) );
  जइ( !db->init.busy ){
    rc = sqlite3Init(db, &pParse->z_Err_सन्देशः);
    जइ( rc!=SQLITE_OK ){
      pParse->rc = rc;
      pParse->nErr++;
    }अण्णहा जइ( db->noSharedCache ){
      db->mDbFlags |= DBFLAG_SchemaKnownOk;
    }
  }
  प्रत्यागम rc;
}


/*
** Check schema cookies in all databases.  If any cookie is out
** of date set pParse->rc to SQLITE_SCHEMA.  If all schema cookies
** make no changes to pParse->rc.
*/
स्थिर शून्य schemaIsValid(Parse *pParse){
  sqlite3 *db = pParse->db;
  पूर्णाङ्क iDb;
  पूर्णाङ्क rc;
  पूर्णाङ्क cookie;

  प्रतिज्ञा( pParse->checkSchema );
  प्रतिज्ञा( sqlite3_mutex_held(db->mutex) );
  प्रति(iDb=0; iDb<db->nDb; iDb++){
    पूर्णाङ्क openedTransaction = 0;         /* True if a transaction is opened */
    Btree *pBt = db->aDb[iDb].pBt;     /* Btree database to read cookie from */
    जइ( pBt==0 ) अनुवर्त;

    /* If there is not already a read-only (or read-write) transaction opened
    ** on the b-tree database, open one now. If a transaction is opened, it 
    ** will be closed immediately after reading the meta-value. */
    जइ( sqlite3BtreeTxnState(pBt)==SQLITE_TXN_NONE ){
      rc = sqlite3BtreeBeginTrans(pBt, 0, 0);
      जइ( rc==SQLITE_NOMEM || rc==SQLITE_IOERR_NOMEM ){
        sqlite3OomFault(db);
        pParse->rc = SQLITE_NOMEM;
      }
      जइ( rc!=SQLITE_OK ) प्रत्यागम;
      openedTransaction = 1;
    }

    /* Read the schema cookie from the database. If it does not match the 
    ** value stored as part of the in-memory schema representation,
    ** set Parse.rc to SQLITE_SCHEMA. */
    sqlite3BtreeGetMeta(pBt, BTREE_SCHEMA_VERSION, (u32 *)&cookie);
    प्रतिज्ञा( sqlite3SchemaMutexHeld(db, iDb, 0) );
    जइ( cookie!=db->aDb[iDb].pSchema->schema_cookie ){
      जइ( DbHasProperty(db, iDb, DB_SchemaLoaded) ) pParse->rc = SQLITE_SCHEMA;
      sqlite3ResetOneSchema(db, iDb);
    }

    /* Close the transaction, if one was opened. */
    जइ( openedTransaction ){
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
पूर्णाङ्क sqlite3SchemaToIndex(sqlite3 *db, Schema *pSchema){
  पूर्णाङ्क i = -32768;

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
  प्रतिज्ञा( sqlite3_mutex_held(db->mutex) );
  जइ( pSchema ){
    प्रति(i=0; 1; i++){
      प्रतिज्ञा( i<db->nDb );
      जइ( db->aDb[i].pSchema==pSchema ){
        भङ्ग;
      }
    }
    प्रतिज्ञा( i>=0 && i<db->nDb );
  }
  प्रत्यागम i;
}

/*
** Free all memory allocations in the pParse object
*/
शून्य sqlite3ParseObjectReset(Parse *pParse){
  sqlite3 *db = pParse->db;
  प्रतिज्ञा( db!=0 );
  प्रतिज्ञा( db->pParse==pParse );
  प्रतिज्ञा( pParse->nested==0 );
#ifndef SQLITE_OMIT_SHARED_CACHE
  जइ( pParse->nTableLock ) sqlite3DbNNFreeNN(db, pParse->aTableLock);
#endif
  जाव( pParse->pCleanup ){
    ParseCleanup *pCleanup = pParse->pCleanup;
    pParse->pCleanup = pCleanup->pNext;
    pCleanup->xCleanup(db, pCleanup->pPtr);
    sqlite3DbNNFreeNN(db, pCleanup);
  }
  जइ( pParse->aLabel ) sqlite3DbNNFreeNN(db, pParse->aLabel);
  जइ( pParse->p_Const_व्यञ्जकम् ){
    sqlite3_व्यञ्जकम्_List_Delete(db, pParse->p_Const_व्यञ्जकम्);
  }
  प्रतिज्ञा( db->lookaside.bDisable >= pParse->disableLookaside );
  db->lookaside.bDisable -= pParse->disableLookaside;
  db->lookaside.sz = db->lookaside.bDisable ? 0 : db->lookaside.szTrue;
  प्रतिज्ञा( pParse->db->pParse==pParse );
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
शून्य *sqlite3ParserAddCleanup(
  Parse *pParse,                      /* Destroy when this Parser finishes */
  शून्य (*xCleanup)(sqlite3*,शून्य*),   /* The cleanup routine */
  शून्य *pPtr                          /* Pointer to object to be cleaned up */
){
  ParseCleanup *pCleanup;
  जइ( sqlite3FaultSim(300) ){
    pCleanup = 0;
    sqlite3OomFault(pParse->db);
  }अण्णहा{
    pCleanup = sqlite3DbMallocRaw(pParse->db, परिमाण(*pCleanup));
  }
  जइ( pCleanup ){
    pCleanup->pNext = pParse->pCleanup;
    pParse->pCleanup = pCleanup;
    pCleanup->pPtr = pPtr;
    pCleanup->xCleanup = xCleanup;
  }अण्णहा{
    xCleanup(pParse->db, pPtr);
    pPtr = 0;
#जइ defined(SQLITE_DEBUG) || defined(SQLITE_COVERAGE_TEST)
    pParse->earlyCleanup = 1;
#endif
  }
  प्रत्यागम pPtr;
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
शून्य sqlite3ParseObjectInit(Parse *pParse, sqlite3 *db){
  प्रतिज्ञा( db!=0 );
  memset(PARSE_HDR(pParse), 0, PARSE_HDR_SZ);
  memset(PARSE_TAIL(pParse), 0, PARSE_TAIL_SZ);
  प्रतिज्ञा( db->pParse!=pParse );
  pParse->pOuterParse = db->pParse;
  db->pParse = pParse;
  pParse->db = db;
  जइ( db->mallocFailed ) sqlite3_Error_सन्देशः(pParse, "out of memory");
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
स्थिर पूर्णाङ्क sqlite3Prepare(
  sqlite3 *db,              /* Database handle. */
  ध्रुव वर्ण *zSql,         /* UTF-8 encoded SQL statement. */
  पूर्णाङ्क nBytes,               /* Length of zSql in bytes. */
  u32 prepFlags,            /* Zero or more SQLITE_PREPARE_* flags */
  Vdbe *pReprepare,         /* VM being reprepared */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  ध्रुव वर्ण **pzTail       /* OUT: End of parsed string */
){
  पूर्णाङ्क rc = SQLITE_OK;       /* Result code */
  पूर्णाङ्क i;                    /* Loop counter */
  Parse sParse;             /* Parsing context */

  /* sqlite3ParseObjectInit(&sParse, db); // inlined for performance */
  memset(PARSE_HDR(&sParse), 0, PARSE_HDR_SZ);
  memset(PARSE_TAIL(&sParse), 0, PARSE_TAIL_SZ);
  sParse.pOuterParse = db->pParse;
  db->pParse = &sParse;
  sParse.db = db;
  जइ( pReprepare ){
    sParse.pReprepare = pReprepare;
    sParse.explain = sqlite3_stmt_isexplain((sqlite3_stmt*)pReprepare);
  }अण्णहा{
    प्रतिज्ञा( sParse.pReprepare==0 );
  }
  प्रतिज्ञा( ppStmt && *ppStmt==0 );
  जइ( db->mallocFailed ){
    sqlite3_Error_सन्देशः(&sParse, "out of memory");
    db->errCode = rc = SQLITE_NOMEM;
    गच्छ end_prepare;
  }
  प्रतिज्ञा( sqlite3_mutex_held(db->mutex) );

  /* For a long-term use prepared statement avoid the use of
  ** lookaside memory.
  */
  जइ( prepFlags & SQLITE_PREPARE_PERSISTENT ){
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
  जइ( !db->noSharedCache ){
    प्रति(i=0; i<db->nDb; i++) {
      Btree *pBt = db->aDb[i].pBt;
      जइ( pBt ){
        प्रतिज्ञा( sqlite3BtreeHoldsMutex(pBt) );
        rc = sqlite3BtreeSchemaLocked(pBt);
        जइ( rc ){
          ध्रुव वर्ण *zDb = db->aDb[i].zDbSName;
          sqlite3_Error_With_सन्देशः(db, rc, "database schema is locked: %s", zDb);
          testcase( db->flags & SQLITE_ReadUncommit );
          गच्छ end_prepare;
        }
      }
    }
  }

#ifndef SQLITE_OMIT_VIRTUALTABLE
  जइ( db->pDisconnect ) sqlite3VtabUnlockList(db);
#endif

  जइ( nBytes>=0 && (nBytes==0 || zSql[nBytes-1]!=0) ){
    वर्ण *zSqlCopy;
    पूर्णाङ्क mxLen = db->aLimit[SQLITE_LIMIT_SQL_LENGTH];
    testcase( nBytes==mxLen );
    testcase( nBytes==mxLen+1 );
    जइ( nBytes>mxLen ){
      sqlite3_Error_With_सन्देशः(db, SQLITE_TOOBIG, "statement too long");
      rc = sqlite3ApiExit(db, SQLITE_TOOBIG);
      गच्छ end_prepare;
    }
    zSqlCopy = sqlite3DbStrNDup(db, zSql, nBytes);
    जइ( zSqlCopy ){
      sqlite3RunParser(&sParse, zSqlCopy);
      sParse.zTail = &zSql[sParse.zTail-zSqlCopy];
      sqlite3DbFree(db, zSqlCopy);
    }अण्णहा{
      sParse.zTail = &zSql[nBytes];
    }
  }अण्णहा{
    sqlite3RunParser(&sParse, zSql);
  }
  प्रतिज्ञा( 0==sParse.nQueryLoop );

  जइ( pzTail ){
    *pzTail = sParse.zTail;
  }

  जइ( db->init.busy==0 ){
    sqlite3VdbeSetSql(sParse.pVdbe, zSql, (पूर्णाङ्क)(sParse.zTail-zSql), prepFlags);
  }
  जइ( db->mallocFailed ){
    sParse.rc = SQLITE_NOMEM_BKPT;
    sParse.checkSchema = 0;
  }
  जइ( sParse.rc!=SQLITE_OK && sParse.rc!=SQLITE_DONE ){
    जइ( sParse.checkSchema && db->init.busy==0 ){
      schemaIsValid(&sParse);
    }
    जइ( sParse.pVdbe ){
      sqlite3VdbeFinalize(sParse.pVdbe);
    }
    प्रतिज्ञा( 0==(*ppStmt) );
    rc = sParse.rc;
    जइ( sParse.z_Err_सन्देशः ){
      sqlite3_Error_With_सन्देशः(db, rc, "%s", sParse.z_Err_सन्देशः);
      sqlite3DbFree(db, sParse.z_Err_सन्देशः);
    }अण्णहा{
      sqlite3Error(db, rc);
    }
  }अण्णहा{
    प्रतिज्ञा( sParse.z_Err_सन्देशः==0 );
    *ppStmt = (sqlite3_stmt*)sParse.pVdbe;
    rc = SQLITE_OK;
    sqlite3ErrorClear(db);
  }


  /* Delete any TriggerPrg structures allocated while parsing this statement. */
  जाव( sParse.pTriggerPrg ){
    TriggerPrg *pT = sParse.pTriggerPrg;
    sParse.pTriggerPrg = pT->pNext;
    sqlite3DbFree(db, pT);
  }

end_prepare:

  sqlite3ParseObjectReset(&sParse);
  प्रत्यागम rc;
}
स्थिर पूर्णाङ्क sqlite3LockAndPrepare(
  sqlite3 *db,              /* Database handle. */
  ध्रुव वर्ण *zSql,         /* UTF-8 encoded SQL statement. */
  पूर्णाङ्क nBytes,               /* Length of zSql in bytes. */
  u32 prepFlags,            /* Zero or more SQLITE_PREPARE_* flags */
  Vdbe *pOld,               /* VM being reprepared */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  ध्रुव वर्ण **pzTail       /* OUT: End of parsed string */
){
  पूर्णाङ्क rc;
  पूर्णाङ्क cnt = 0;

#ifdef SQLITE_ENABLE_API_ARMOR
  जइ( ppStmt==0 ) प्रत्यागम SQLITE_MISUSE_BKPT;
#endif
  *ppStmt = 0;
  जइ( !sqlite3SafetyCheckOk(db)||zSql==0 ){
    प्रत्यागम SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(db->mutex);
  sqlite3BtreeEnterAll(db);
  करेहि{
    /* Make multiple attempts to compile the SQL, until it either succeeds
    ** or encounters a permanent error.  A schema problem after one schema
    ** reset is considered a permanent error. */
    rc = sqlite3Prepare(db, zSql, nBytes, prepFlags, pOld, ppStmt, pzTail);
    प्रतिज्ञा( rc==SQLITE_OK || *ppStmt==0 );
    जइ( rc==SQLITE_OK || db->mallocFailed ) भङ्ग;
    cnt++;
  }जाव( (rc==SQLITE_ERROR_RETRY && ALWAYS(cnt<=SQLITE_MAX_PREPARE_RETRY))
       || (rc==SQLITE_SCHEMA && (sqlite3ResetOneSchema(db,-1), cnt)==1) );
  sqlite3BtreeLeaveAll(db);
  प्रतिज्ञा( rc!=SQLITE_ERROR_RETRY );
  rc = sqlite3ApiExit(db, rc);
  प्रतिज्ञा( (rc&db->errMask)==rc );
  db->busyHandler.nBusy = 0;
  sqlite3_mutex_leave(db->mutex);
  प्रतिज्ञा( rc==SQLITE_OK || (*ppStmt)==0 );
  प्रत्यागम rc;
}


/*
** Rerun the compilation of a statement after a schema change.
**
** If the statement is successfully recompiled, return SQLITE_OK. Otherwise,
** if the statement cannot be recompiled because another connection has
** locked the sqlite3_schema table, return SQLITE_LOCKED. If any other error
** occurs, return SQLITE_SCHEMA.
*/
पूर्णाङ्क sqlite3Reprepare(Vdbe *p){
  पूर्णाङ्क rc;
  sqlite3_stmt *pNew;
  ध्रुव वर्ण *zSql;
  sqlite3 *db;
  u8 prepFlags;

  प्रतिज्ञा( sqlite3_mutex_held(sqlite3VdbeDb(p)->mutex) );
  zSql = sqlite3_sql((sqlite3_stmt *)p);
  प्रतिज्ञा( zSql!=0 );  /* Reprepare only called for prepare_v2() statements */
  db = sqlite3VdbeDb(p);
  प्रतिज्ञा( sqlite3_mutex_held(db->mutex) );
  prepFlags = sqlite3VdbePrepareFlags(p);
  rc = sqlite3LockAndPrepare(db, zSql, -1, prepFlags, p, &pNew, 0);
  जइ( rc ){
    जइ( rc==SQLITE_NOMEM ){
      sqlite3OomFault(db);
    }
    प्रतिज्ञा( pNew==0 );
    प्रत्यागम rc;
  }अण्णहा{
    प्रतिज्ञा( pNew!=0 );
  }
  sqlite3VdbeSwap((Vdbe*)pNew, p);
  sqlite3TransferBindings(pNew, (sqlite3_stmt*)p);
  sqlite3VdbeResetStepResult((Vdbe*)pNew);
  sqlite3VdbeFinalize((Vdbe*)pNew);
  प्रत्यागम SQLITE_OK;
}


/*
** Two versions of the official API.  Legacy and new use.  In the legacy
** version, the original SQL text is not saved in the prepared statement
** and so if a schema change occurs, SQLITE_SCHEMA is returned by
** sqlite3_step().  In the new version, the original SQL text is retained
** and the statement is automatically recompiled if an schema change
** occurs.
*/
पूर्णाङ्क sqlite3_prepare(
  sqlite3 *db,              /* Database handle. */
  ध्रुव वर्ण *zSql,         /* UTF-8 encoded SQL statement. */
  पूर्णाङ्क nBytes,               /* Length of zSql in bytes. */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  ध्रुव वर्ण **pzTail       /* OUT: End of parsed string */
){
  पूर्णाङ्क rc;
  rc = sqlite3LockAndPrepare(db,zSql,nBytes,0,0,ppStmt,pzTail);
  प्रतिज्ञा( rc==SQLITE_OK || ppStmt==0 || *ppStmt==0 );  /* VERIFY: F13021 */
  प्रत्यागम rc;
}
पूर्णाङ्क sqlite3_prepare_v2(
  sqlite3 *db,              /* Database handle. */
  ध्रुव वर्ण *zSql,         /* UTF-8 encoded SQL statement. */
  पूर्णाङ्क nBytes,               /* Length of zSql in bytes. */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  ध्रुव वर्ण **pzTail       /* OUT: End of parsed string */
){
  पूर्णाङ्क rc;
  /* EVIDENCE-OF: R-37923-12173 The sqlite3_prepare_v2() interface works
  ** exactly the same as sqlite3_prepare_v3() with a zero prepFlags
  ** parameter.
  **
  ** Proof in that the 5th parameter to sqlite3LockAndPrepare is 0 */
  rc = sqlite3LockAndPrepare(db,zSql,nBytes,SQLITE_PREPARE_SAVESQL,0,
                             ppStmt,pzTail);
  प्रतिज्ञा( rc==SQLITE_OK || ppStmt==0 || *ppStmt==0 );
  प्रत्यागम rc;
}
पूर्णाङ्क sqlite3_prepare_v3(
  sqlite3 *db,              /* Database handle. */
  ध्रुव वर्ण *zSql,         /* UTF-8 encoded SQL statement. */
  पूर्णाङ्क nBytes,               /* Length of zSql in bytes. */
  unsigned पूर्णाङ्क prepFlags,   /* Zero or more SQLITE_PREPARE_* flags */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  ध्रुव वर्ण **pzTail       /* OUT: End of parsed string */
){
  पूर्णाङ्क rc;
  /* EVIDENCE-OF: R-56861-42673 sqlite3_prepare_v3() differs from
  ** sqlite3_prepare_v2() only in having the extra prepFlags parameter,
  ** which is a bit array consisting of zero or more of the
  ** SQLITE_PREPARE_* flags.
  **
  ** Proof by comparison to the implementation of sqlite3_prepare_v2()
  ** directly above. */
  rc = sqlite3LockAndPrepare(db,zSql,nBytes,
                 SQLITE_PREPARE_SAVESQL|(prepFlags&SQLITE_PREPARE_MASK),
                 0,ppStmt,pzTail);
  प्रतिज्ञा( rc==SQLITE_OK || ppStmt==0 || *ppStmt==0 );
  प्रत्यागम rc;
}


#ifndef SQLITE_OMIT_UTF16
/*
** Compile the UTF-16 encoded SQL statement zSql into a statement handle.
*/
स्थिर पूर्णाङ्क sqlite3Prepare16(
  sqlite3 *db,              /* Database handle. */ 
  ध्रुव शून्य *zSql,         /* UTF-16 encoded SQL statement. */
  पूर्णाङ्क nBytes,               /* Length of zSql in bytes. */
  u32 prepFlags,            /* Zero or more SQLITE_PREPARE_* flags */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  ध्रुव शून्य **pzTail       /* OUT: End of parsed string */
){
  /* This function currently works by first transforming the UTF-16
  ** encoded string to UTF-8, then invoking sqlite3_prepare(). The
  ** tricky bit is figuring out the pointer to return in *pzTail.
  */
  वर्ण *zSql8;
  ध्रुव वर्ण *zTail8 = 0;
  पूर्णाङ्क rc = SQLITE_OK;

#ifdef SQLITE_ENABLE_API_ARMOR
  जइ( ppStmt==0 ) प्रत्यागम SQLITE_MISUSE_BKPT;
#endif
  *ppStmt = 0;
  जइ( !sqlite3SafetyCheckOk(db)||zSql==0 ){
    प्रत्यागम SQLITE_MISUSE_BKPT;
  }

  /* Make sure nBytes is non-negative and correct.  It should be the
  ** number of bytes until the end of the input buffer or until the first
  ** U+0000 character.  If the input nBytes is odd, convert it into
  ** an even number.  If the input nBytes is negative, then the input
  ** must be terminated by at least one U+0000 character */
  जइ( nBytes>=0 ){
    पूर्णाङ्क sz;
    ध्रुव वर्ण *z = (ध्रुव वर्ण*)zSql;
    प्रति(sz=0; sz<nBytes && (z[sz]!=0 || z[sz+1]!=0); sz += 2){}
    nBytes = sz;
  }अण्णहा{
    पूर्णाङ्क sz;
    ध्रुव वर्ण *z = (ध्रुव वर्ण*)zSql;
    प्रति(sz=0; z[sz]!=0 || z[sz+1]!=0; sz += 2){}
    nBytes = sz;
  }

  sqlite3_mutex_enter(db->mutex);
  zSql8 = sqlite3Utf16to8(db, zSql, nBytes, SQLITE_UTF16NATIVE);
  जइ( zSql8 ){
    rc = sqlite3LockAndPrepare(db, zSql8, -1, prepFlags, 0, ppStmt, &zTail8);
  }

  जइ( zTail8 && pzTail ){
    /* If sqlite3_prepare returns a tail pointer, we calculate the
    ** equivalent pointer into the UTF-16 string by counting the unicode
    ** characters between zSql8 and zTail8, and then returning a pointer
    ** the same number of characters into the UTF-16 string.
    */
    पूर्णाङ्क chars_parsed = sqlite3Utf8CharLen(zSql8, (पूर्णाङ्क)(zTail8-zSql8));
    *pzTail = (u8 *)zSql + sqlite3Utf16ByteLen(zSql, nBytes, chars_parsed);
  }
  sqlite3DbFree(db, zSql8); 
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  प्रत्यागम rc;
}

/*
** Two versions of the official API.  Legacy and new use.  In the legacy
** version, the original SQL text is not saved in the prepared statement
** and so if a schema change occurs, SQLITE_SCHEMA is returned by
** sqlite3_step().  In the new version, the original SQL text is retained
** and the statement is automatically recompiled if an schema change
** occurs.
*/
पूर्णाङ्क sqlite3_prepare16(
  sqlite3 *db,              /* Database handle. */ 
  ध्रुव शून्य *zSql,         /* UTF-16 encoded SQL statement. */
  पूर्णाङ्क nBytes,               /* Length of zSql in bytes. */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  ध्रुव शून्य **pzTail       /* OUT: End of parsed string */
){
  पूर्णाङ्क rc;
  rc = sqlite3Prepare16(db,zSql,nBytes&~1,0,ppStmt,pzTail);
  प्रतिज्ञा( rc==SQLITE_OK || ppStmt==0 || *ppStmt==0 );  /* VERIFY: F13021 */
  प्रत्यागम rc;
}
पूर्णाङ्क sqlite3_prepare16_v2(
  sqlite3 *db,              /* Database handle. */ 
  ध्रुव शून्य *zSql,         /* UTF-16 encoded SQL statement. */
  पूर्णाङ्क nBytes,               /* Length of zSql in bytes. */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  ध्रुव शून्य **pzTail       /* OUT: End of parsed string */
){
  पूर्णाङ्क rc;
  rc = sqlite3Prepare16(db,zSql,nBytes&~1,SQLITE_PREPARE_SAVESQL,ppStmt,pzTail);
  प्रतिज्ञा( rc==SQLITE_OK || ppStmt==0 || *ppStmt==0 );  /* VERIFY: F13021 */
  प्रत्यागम rc;
}
पूर्णाङ्क sqlite3_prepare16_v3(
  sqlite3 *db,              /* Database handle. */ 
  ध्रुव शून्य *zSql,         /* UTF-16 encoded SQL statement. */
  पूर्णाङ्क nBytes,               /* Length of zSql in bytes. */
  unsigned पूर्णाङ्क prepFlags,   /* Zero or more SQLITE_PREPARE_* flags */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  ध्रुव शून्य **pzTail       /* OUT: End of parsed string */
){
  पूर्णाङ्क rc;
  rc = sqlite3Prepare16(db,zSql,nBytes&~1,
         SQLITE_PREPARE_SAVESQL|(prepFlags&SQLITE_PREPARE_MASK),
         ppStmt,pzTail);
  प्रतिज्ञा( rc==SQLITE_OK || ppStmt==0 || *ppStmt==0 );  /* VERIFY: F13021 */
  प्रत्यागम rc;
}

#endif /* SQLITE_OMIT_UTF16 */
