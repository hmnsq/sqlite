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
#समावेश "sqliteInt.h"

/*
** Fill the InitData structure with an error message that indicates
** that the database is corrupt.
*/
अटल व्योम corruptSchema(
  Init_तथ्य *p_तथ्य,     /* Initialization context */
  अक्षर **azObj,        /* Type and name of object being parsed */
  स्थिर अक्षर *zExtra   /* Error information */
){
  sqlite3 *db = p_तथ्य->db;
  अगर( db->mallocFailed ){
    p_तथ्य->rc = SQLITE_NOMEM_BKPT;
  }अन्यथा अगर( p_तथ्य->pz_Err_संदेश[0]!=0 ){
    /* A error message has already been generated.  Do not overwrite it */
  }अन्यथा अगर( p_तथ्य->mInitFlags & (INITFLAG_AlterMask) ){
    अटल स्थिर अक्षर *azAlterType[] = {
       "rename",
       "drop column",
       "add column",
       "drop constraint"
    };
    *p_तथ्य->pz_Err_संदेश = sqlite3MPrintf(db, 
        "error in %s %s after %s: %s", azObj[0], azObj[1], 
        azAlterType[(p_तथ्य->mInitFlags&INITFLAG_AlterMask)-1], 
        zExtra
    );
    p_तथ्य->rc = SQLITE_ERROR;
  }अन्यथा अगर( db->flags & SQLITE_WriteSchema ){
    p_तथ्य->rc = SQLITE_CORRUPT_BKPT;
  }अन्यथा{
    अक्षर *z;
    स्थिर अक्षर *zObj = azObj[1] ? azObj[1] : "?";
    z = sqlite3MPrintf(db, "malformed database schema (%s)", zObj);
    अगर( zExtra && zExtra[0] ) z = sqlite3MPrintf(db, "%z - %s", z, zExtra);
    *p_तथ्य->pz_Err_संदेश = z;
    p_तथ्य->rc = SQLITE_CORRUPT_BKPT;
  }
}

/*
** Check to see if any sibling index (another index on the same table)
** of pIndex has the same root page number, and if it does, return true.
** This would indicate a corrupt schema.
*/
पूर्णांक sqlite3_सूचकांक_Has_Duplicate_मूल_Page(सूचकांक *p_सूचकांक){
  सूचकांक *p;
  क्रम(p=p_सूचकांक->pTable->p_सूचकांक; p; p=p->p_अग्र){
    अगर( p->tnum==p_सूचकांक->tnum && p!=p_सूचकांक ) वापस 1;
  }
  वापस 0;
}

/* forward declaration */
अटल पूर्णांक sqlite3Prepare(
  sqlite3 *db,              /* Database handle. */
  स्थिर अक्षर *zSql,         /* UTF-8 encoded SQL statement. */
  पूर्णांक nBytes,               /* Length of zSql in bytes. */
  u32 prepFlags,            /* Zero or more SQLITE_PREPARE_* flags */
  Vdbe *pReprepare,         /* VM being reprepared */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  स्थिर अक्षर **pz_पुच्छ       /* OUT: End of parsed string */
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
पूर्णांक sqlite3InitCallback(व्योम *pInit, पूर्णांक argc, अक्षर **argv, अक्षर **NotUsed){
  Init_तथ्य *p_तथ्य = (Init_तथ्य*)pInit;
  sqlite3 *db = p_तथ्य->db;
  पूर्णांक iDb = p_तथ्य->iDb;

  निश्चित( argc==5 );
  UNUSED_PARAMETER2(NotUsed, argc);
  निश्चित( sqlite3_mutex_held(db->mutex) );
  db->mDbFlags |= DBFLAG_EncodingFixed;
  अगर( argv==0 ) वापस 0;   /* Might happen if EMPTY_RESULT_CALLBACKS are on */
  p_तथ्य->nInitRow++;
  अगर( db->mallocFailed ){
    corruptSchema(p_तथ्य, argv, 0);
    वापस 1;
  }

  निश्चित( iDb>=0 && iDb<db->nDb );
  निश्चित( db->aDb[iDb].pSchema!=0 );
  अगर( argv[3]==0 ){
    corruptSchema(p_तथ्य, argv, 0);
  }अन्यथा अगर( argv[4]
         && 'c'==sqlite3UpperToLower[(अचिन्हित अक्षर)argv[4][0]]
         && 'r'==sqlite3UpperToLower[(अचिन्हित अक्षर)argv[4][1]] ){
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
    पूर्णांक rc;
    u8 saved_iDb = db->init.iDb;
    sqlite3_stmt *pStmt;
    स्थिर अक्षर *zEnd;
    TESTONLY(पूर्णांक rcp);            /* Return code from sqlite3_prepare() */

    निश्चित( db->init.busy );
    db->init.iDb = iDb;
    अगर( sqlite3GetUInt32(argv[3], &db->init.newTnum)==0
     || (db->init.newTnum>p_तथ्य->mxPage && p_तथ्य->mxPage>0)
    ){
      अगर( sqlite3Config.bExtraSchemaChecks ){
        corruptSchema(p_तथ्य, argv, "invalid rootpage");
      }
    }
    db->init.orphanTrigger = 0;
    db->init.azInit = (स्थिर अक्षर**)argv;
    pStmt = 0;
    TESTONLY(rcp = ) sqlite3Prepare(db, argv[4], -1, 0, 0, &pStmt, &zEnd);
    rc = db->errCode;
    निश्चित( (rc&0xFF)==(rcp&0xFF) );
    db->init.iDb = saved_iDb;
    /* assert( saved_iDb==0 || (db->mDbFlags & DBFLAG_Vacuum)!=0 ); */
    अगर( SQLITE_OK!=rc ){
      अगर( db->init.orphanTrigger ){
        निश्चित( iDb==1 );
      }अन्यथा{
        अगर( rc > p_तथ्य->rc ) p_तथ्य->rc = rc;
        अगर( rc==SQLITE_NOMEM ){
          sqlite3OomFault(db);
        }अन्यथा अगर( rc!=SQLITE_INTERRUPT && (rc&0xFF)!=SQLITE_LOCKED ){
          corruptSchema(p_तथ्य, argv, sqlite3_errmsg(db));
        }
      }
    }अन्यथा अगर( zEnd[0] ){
      corruptSchema(p_तथ्य, argv, 0);
    }
    db->init.azInit = sqlite3StdType; /* Any array of string ptrs will do */
    sqlite3_finalize(pStmt);
  }अन्यथा अगर( argv[1]==0 || (argv[4]!=0 && argv[4][0]!=0) ){
    corruptSchema(p_तथ्य, argv, 0);
  }अन्यथा{
    /* If the SQL column is blank it means this is an index that
    ** was created to be the PRIMARY KEY or to fulfill a UNIQUE
    ** constraint for a CREATE TABLE.  The index should have already
    ** been created when we processed the CREATE TABLE.  All we have
    ** to do here is record the root page number for that index.
    */
    सूचकांक *p_सूचकांक;
    p_सूचकांक = sqlite3_Find_सूचकांक(db, argv[1], db->aDb[iDb].zDbSName);
    अगर( p_सूचकांक==0 ){
      corruptSchema(p_तथ्य, argv, "orphan index");
    }अन्यथा
    अगर( sqlite3GetUInt32(argv[3],&p_सूचकांक->tnum)==0
     || p_सूचकांक->tnum<2
     || p_सूचकांक->tnum>p_तथ्य->mxPage
     || sqlite3_सूचकांक_Has_Duplicate_मूल_Page(p_सूचकांक)
    ){
      अगर( sqlite3Config.bExtraSchemaChecks ){
        corruptSchema(p_तथ्य, argv, "invalid rootpage");
      }
    }
  }
  अगर( p_तथ्य->pz_Err_संदेश[0]==0 ){
    Schema *pX = db->aDb[iDb].pSchema;
    अगर( pX->tblHash.गिनती + pX->idxHash.गिनती + pX->trigHash.गिनती
                   > (u32)db->aLimit[SQLITE_LIMIT_SCHEMA]
    ){
      *p_तथ्य->pz_Err_संदेश = sqlite3MPrintf(db, "too many schema objects");
      p_तथ्य->rc = SQLITE_ERROR;
      वापस 1;
    }
  }
  वापस 0;
}

/*
** Attempt to read the database schema and initialize internal
** data structures for a single database file.  The index of the
** database file is given by iDb.  iDb==0 is used for the main
** database.  iDb==1 should never be used.  iDb>=2 is used for
** auxiliary databases.  Return one of the SQLITE_ error codes to
** indicate success or failure.
*/
पूर्णांक sqlite3InitOne(sqlite3 *db, पूर्णांक iDb, अक्षर **pz_Err_संदेश, u32 mFlags){
  पूर्णांक rc;
  पूर्णांक i;
#अगर_अघोषित SQLITE_OMIT_DEPRECATED
  पूर्णांक आकार;
#पूर्ण_अगर
  Db *pDb;
  अक्षर स्थिर *azArg[6];
  पूर्णांक meta[5];
  Init_तथ्य init_तथ्य;
  स्थिर अक्षर *zSchemaTabName;
  पूर्णांक openedTransaction = 0;
  पूर्णांक mask = ((db->mDbFlags & DBFLAG_EncodingFixed) | ~DBFLAG_EncodingFixed);

  निश्चित( (db->mDbFlags & DBFLAG_SchemaKnownOk)==0 );
  निश्चित( iDb>=0 && iDb<db->nDb );
  निश्चित( db->aDb[iDb].pSchema );
  निश्चित( sqlite3_mutex_held(db->mutex) );
  निश्चित( iDb==1 || sqlite3BtreeHoldsMutex(db->aDb[iDb].pBt) );

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
  init_तथ्य.db = db;
  init_तथ्य.iDb = iDb;
  init_तथ्य.rc = SQLITE_OK;
  init_तथ्य.pz_Err_संदेश = pz_Err_संदेश;
  init_तथ्य.mInitFlags = mFlags;
  init_तथ्य.nInitRow = 0;
  init_तथ्य.mxPage = 0;
  sqlite3InitCallback(&init_तथ्य, 5, (अक्षर **)azArg, 0);
  db->mDbFlags &= mask;
  अगर( init_तथ्य.rc ){
    rc = init_तथ्य.rc;
    जाओ error_out;
  }

  /* Create a cursor to hold the database open
  */
  pDb = &db->aDb[iDb];
  अगर( pDb->pBt==0 ){
    निश्चित( iDb==1 );
    DbSetProperty(db, 1, DB_SchemaLoaded);
    rc = SQLITE_OK;
    जाओ error_out;
  }

  /* If there is not already a read-only (or read-write) transaction opened
  ** on the b-tree database, open one now. If a transaction is opened, it 
  ** will be closed before this function returns.  */
  sqlite3BtreeEnter(pDb->pBt);
  अगर( sqlite3BtreeTxnState(pDb->pBt)==SQLITE_TXN_NONE ){
    rc = sqlite3BtreeBeginTrans(pDb->pBt, 0, 0);
    अगर( rc!=SQLITE_OK ){
      sqlite3SetString(pz_Err_संदेश, db, sqlite3ErrStr(rc));
      जाओ initone_error_out;
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
  क्रम(i=0; i<Array_आकार(meta); i++){
    sqlite3BtreeGetMeta(pDb->pBt, i+1, (u32 *)&meta[i]);
  }
  अगर( (db->flags & SQLITE_ResetDatabase)!=0 ){
    स_रखो(meta, 0, माप(meta));
  }
  pDb->pSchema->schema_cookie = meta[BTREE_SCHEMA_VERSION-1];

  /* If opening a non-empty database, check the text encoding. For the
  ** main database, set sqlite3.enc to the encoding of the main database.
  ** For an attached db, it is an error if the encoding is not the same
  ** as sqlite3.enc.
  */
  अगर( meta[BTREE_TEXT_ENCODING-1] ){  /* text encoding */
    अगर( iDb==0 && (db->mDbFlags & DBFLAG_EncodingFixed)==0 ){
      u8 encoding;
#अगर_अघोषित SQLITE_OMIT_UTF16
      /* If opening the main database, set ENC(db). */
      encoding = (u8)meta[BTREE_TEXT_ENCODING-1] & 3;
      अगर( encoding==0 ) encoding = SQLITE_UTF8;
#अन्यथा
      encoding = SQLITE_UTF8;
#पूर्ण_अगर
      sqlite3SetTextEncoding(db, encoding);
    }अन्यथा{
      /* If opening an attached database, the encoding much match ENC(db) */
      अगर( (meta[BTREE_TEXT_ENCODING-1] & 3)!=ENC(db) ){
        sqlite3SetString(pz_Err_संदेश, db, "attached databases must use the same"
            " text encoding as main database");
        rc = SQLITE_ERROR;
        जाओ initone_error_out;
      }
    }
  }
  pDb->pSchema->enc = ENC(db);

  अगर( pDb->pSchema->cache_आकार==0 ){
#अगर_अघोषित SQLITE_OMIT_DEPRECATED
    आकार = sqlite3AbsInt32(meta[BTREE_DEFAULT_CACHE_आकार-1]);
    अगर( आकार==0 ){ आकार = SQLITE_DEFAULT_CACHE_आकार; }
    pDb->pSchema->cache_आकार = आकार;
#अन्यथा
    pDb->pSchema->cache_आकार = SQLITE_DEFAULT_CACHE_आकार;
#पूर्ण_अगर
    sqlite3_Btree_Set_Cache_आकार(pDb->pBt, pDb->pSchema->cache_आकार);
  }

  /*
  ** file_format==1    Version 3.0.0.
  ** file_format==2    Version 3.1.3.  // ALTER TABLE ADD COLUMN
  ** file_format==3    Version 3.1.4.  // ditto but with non-NULL defaults
  ** file_format==4    Version 3.3.0.  // DESC indices.  Boolean constants
  */
  pDb->pSchema->file_format = (u8)meta[BTREE_FILE_FORMAT-1];
  अगर( pDb->pSchema->file_format==0 ){
    pDb->pSchema->file_format = 1;
  }
  अगर( pDb->pSchema->file_format>SQLITE_MAX_FILE_FORMAT ){
    sqlite3SetString(pz_Err_संदेश, db, "unsupported file format");
    rc = SQLITE_ERROR;
    जाओ initone_error_out;
  }

  /* Ticket #2804:  When we open a database in the newer file format,
  ** clear the legacy_file_format pragma flag so that a VACUUM will
  ** not downgrade the database and thus invalidate any descending
  ** indices that the user might have created.
  */
  अगर( iDb==0 && meta[BTREE_FILE_FORMAT-1]>=4 ){
    db->flags &= ~(u64)SQLITE_LegacyFileFmt;
  }

  /* Read the schema information out of the schema tables
  */
  निश्चित( db->init.busy );
  init_तथ्य.mxPage = sqlite3BtreeLastPage(pDb->pBt);
  {
    अक्षर *zSql;
    zSql = sqlite3MPrintf(db, 
        "SELECT*FROM\"%w\".%s ORDER BY rowid",
        db->aDb[iDb].zDbSName, zSchemaTabName);
#अगर_अघोषित SQLITE_OMIT_AUTHORIZATION
    {
      sqlite3_xauth xAuth;
      xAuth = db->xAuth;
      db->xAuth = 0;
#पूर्ण_अगर
      rc = sqlite3_exec(db, zSql, sqlite3InitCallback, &init_तथ्य, 0);
#अगर_अघोषित SQLITE_OMIT_AUTHORIZATION
      db->xAuth = xAuth;
    }
#पूर्ण_अगर
    अगर( rc==SQLITE_OK ) rc = init_तथ्य.rc;
    sqlite3DbFree(db, zSql);
#अगर_अघोषित SQLITE_OMIT_ANALYZE
    अगर( rc==SQLITE_OK ){
      sqlite3AnalysisLoad(db, iDb);
    }
#पूर्ण_अगर
  }
  निश्चित( pDb == &(db->aDb[iDb]) );
  अगर( db->mallocFailed ){
    rc = SQLITE_NOMEM_BKPT;
    sqlite3ResetAllSchemasOfConnection(db);
    pDb = &db->aDb[iDb];
  }अन्यथा
  अगर( rc==SQLITE_OK || ((db->flags&SQLITE_NoSchemaError) && rc!=SQLITE_NOMEM)){
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
  अगर( openedTransaction ){
    sqlite3BtreeCommit(pDb->pBt);
  }
  sqlite3BtreeLeave(pDb->pBt);

error_out:
  अगर( rc ){
    अगर( rc==SQLITE_NOMEM || rc==SQLITE_IOERR_NOMEM ){
      sqlite3OomFault(db);
    }
    sqlite3ResetOneSchema(db, iDb);
  }
  db->init.busy = 0;
  वापस rc;
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
पूर्णांक sqlite3Init(sqlite3 *db, अक्षर **pz_Err_संदेश){
  पूर्णांक i, rc;
  पूर्णांक commit_internal = !(db->mDbFlags&DBFLAG_SchemaChange);
  
  निश्चित( sqlite3_mutex_held(db->mutex) );
  निश्चित( sqlite3BtreeHoldsMutex(db->aDb[0].pBt) );
  निश्चित( db->init.busy==0 );
  ENC(db) = SCHEMA_ENC(db);
  निश्चित( db->nDb>0 );
  /* Do the main schema first */
  अगर( !DbHasProperty(db, 0, DB_SchemaLoaded) ){
    rc = sqlite3InitOne(db, 0, pz_Err_संदेश, 0);
    अगर( rc ) वापस rc;
  }
  /* All other schemas after the main schema. The "temp" schema must be last */
  क्रम(i=db->nDb-1; i>0; i--){
    निश्चित( i==1 || sqlite3BtreeHoldsMutex(db->aDb[i].pBt) );
    अगर( !DbHasProperty(db, i, DB_SchemaLoaded) ){
      rc = sqlite3InitOne(db, i, pz_Err_संदेश, 0);
      अगर( rc ) वापस rc;
    }
  }
  अगर( commit_internal ){
    sqlite3CommitInternalChanges(db);
  }
  वापस SQLITE_OK;
}

/*
** This routine is a no-op if the database schema is already initialized.
** Otherwise, the schema is loaded. An error code is returned.
*/
पूर्णांक sqlite3ReadSchema(Parse *pParse){
  पूर्णांक rc = SQLITE_OK;
  sqlite3 *db = pParse->db;
  निश्चित( sqlite3_mutex_held(db->mutex) );
  अगर( !db->init.busy ){
    rc = sqlite3Init(db, &pParse->z_Err_संदेश);
    अगर( rc!=SQLITE_OK ){
      pParse->rc = rc;
      pParse->nErr++;
    }अन्यथा अगर( db->noSharedCache ){
      db->mDbFlags |= DBFLAG_SchemaKnownOk;
    }
  }
  वापस rc;
}


/*
** Check schema cookies in all databases.  If any cookie is out
** of date set pParse->rc to SQLITE_SCHEMA.  If all schema cookies
** make no changes to pParse->rc.
*/
अटल व्योम schemaIsValid(Parse *pParse){
  sqlite3 *db = pParse->db;
  पूर्णांक iDb;
  पूर्णांक rc;
  पूर्णांक cookie;

  निश्चित( pParse->checkSchema );
  निश्चित( sqlite3_mutex_held(db->mutex) );
  क्रम(iDb=0; iDb<db->nDb; iDb++){
    पूर्णांक openedTransaction = 0;         /* True if a transaction is opened */
    Btree *pBt = db->aDb[iDb].pBt;     /* Btree database to read cookie from */
    अगर( pBt==0 ) जारी;

    /* If there is not already a read-only (or read-write) transaction opened
    ** on the b-tree database, open one now. If a transaction is opened, it 
    ** will be closed immediately after reading the meta-value. */
    अगर( sqlite3BtreeTxnState(pBt)==SQLITE_TXN_NONE ){
      rc = sqlite3BtreeBeginTrans(pBt, 0, 0);
      अगर( rc==SQLITE_NOMEM || rc==SQLITE_IOERR_NOMEM ){
        sqlite3OomFault(db);
        pParse->rc = SQLITE_NOMEM;
      }
      अगर( rc!=SQLITE_OK ) वापस;
      openedTransaction = 1;
    }

    /* Read the schema cookie from the database. If it does not match the 
    ** value stored as part of the in-memory schema representation,
    ** set Parse.rc to SQLITE_SCHEMA. */
    sqlite3BtreeGetMeta(pBt, BTREE_SCHEMA_VERSION, (u32 *)&cookie);
    निश्चित( sqlite3SchemaMutexHeld(db, iDb, 0) );
    अगर( cookie!=db->aDb[iDb].pSchema->schema_cookie ){
      अगर( DbHasProperty(db, iDb, DB_SchemaLoaded) ) pParse->rc = SQLITE_SCHEMA;
      sqlite3ResetOneSchema(db, iDb);
    }

    /* Close the transaction, if one was opened. */
    अगर( openedTransaction ){
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
पूर्णांक sqlite3_Schema_To_सूचकांक(sqlite3 *db, Schema *pSchema){
  पूर्णांक i = -32768;

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
  निश्चित( sqlite3_mutex_held(db->mutex) );
  अगर( pSchema ){
    क्रम(i=0; 1; i++){
      निश्चित( i<db->nDb );
      अगर( db->aDb[i].pSchema==pSchema ){
        अवरोध;
      }
    }
    निश्चित( i>=0 && i<db->nDb );
  }
  वापस i;
}

/*
** Free all memory allocations in the pParse object
*/
व्योम sqlite3ParseObjectReset(Parse *pParse){
  sqlite3 *db = pParse->db;
  निश्चित( db!=0 );
  निश्चित( db->pParse==pParse );
  निश्चित( pParse->nested==0 );
#अगर_अघोषित SQLITE_OMIT_SHARED_CACHE
  अगर( pParse->nTableLock ) sqlite3DbNNFreeNN(db, pParse->aTableLock);
#पूर्ण_अगर
  जबतक( pParse->pCleanup ){
    ParseCleanup *pCleanup = pParse->pCleanup;
    pParse->pCleanup = pCleanup->p_अग्र;
    pCleanup->xCleanup(db, pCleanup->pPtr);
    sqlite3DbNNFreeNN(db, pCleanup);
  }
  अगर( pParse->aLabel ) sqlite3DbNNFreeNN(db, pParse->aLabel);
  अगर( pParse->p_Const_व्यंजक ){
    sqlite3_व्यंजक_List_Delete(db, pParse->p_Const_व्यंजक);
  }
  निश्चित( db->lookaside.bDisable >= pParse->disableLookaside );
  db->lookaside.bDisable -= pParse->disableLookaside;
  db->lookaside.sz = db->lookaside.bDisable ? 0 : db->lookaside.szTrue;
  निश्चित( pParse->db->pParse==pParse );
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
व्योम *sqlite3ParserAddCleanup(
  Parse *pParse,                      /* Destroy when this Parser finishes */
  व्योम (*xCleanup)(sqlite3*,व्योम*),   /* The cleanup routine */
  व्योम *pPtr                          /* Pointer to object to be cleaned up */
){
  ParseCleanup *pCleanup;
  अगर( sqlite3FaultSim(300) ){
    pCleanup = 0;
    sqlite3OomFault(pParse->db);
  }अन्यथा{
    pCleanup = sqlite3DbMallocRaw(pParse->db, माप(*pCleanup));
  }
  अगर( pCleanup ){
    pCleanup->p_अग्र = pParse->pCleanup;
    pParse->pCleanup = pCleanup;
    pCleanup->pPtr = pPtr;
    pCleanup->xCleanup = xCleanup;
  }अन्यथा{
    xCleanup(pParse->db, pPtr);
    pPtr = 0;
#अगर defined(SQLITE_DEBUG) || defined(SQLITE_COVERAGE_TEST)
    pParse->earlyCleanup = 1;
#पूर्ण_अगर
  }
  वापस pPtr;
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
व्योम sqlite3ParseObjectInit(Parse *pParse, sqlite3 *db){
  निश्चित( db!=0 );
  स_रखो(PARSE_HDR(pParse), 0, PARSE_HDR_SZ);
  स_रखो(PARSE_पुच्छ(pParse), 0, PARSE_पुच्छ_SZ);
  निश्चित( db->pParse!=pParse );
  pParse->pOuterParse = db->pParse;
  db->pParse = pParse;
  pParse->db = db;
  अगर( db->mallocFailed ) sqlite3_Error_संदेश(pParse, "out of memory");
}

/*
** Maximum number of times that we will try again to prepare a statement
** that returns SQLITE_ERROR_RETRY.
*/
#अगर_अघोषित SQLITE_MAX_PREPARE_RETRY
# define SQLITE_MAX_PREPARE_RETRY 25
#पूर्ण_अगर

/*
** Compile the UTF-8 encoded SQL statement zSql into a statement handle.
*/
अटल पूर्णांक sqlite3Prepare(
  sqlite3 *db,              /* Database handle. */
  स्थिर अक्षर *zSql,         /* UTF-8 encoded SQL statement. */
  पूर्णांक nBytes,               /* Length of zSql in bytes. */
  u32 prepFlags,            /* Zero or more SQLITE_PREPARE_* flags */
  Vdbe *pReprepare,         /* VM being reprepared */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  स्थिर अक्षर **pz_पुच्छ       /* OUT: End of parsed string */
){
  पूर्णांक rc = SQLITE_OK;       /* Result code */
  पूर्णांक i;                    /* Loop counter */
  Parse sParse;             /* Parsing context */

  /* sqlite3ParseObjectInit(&sParse, db); // inlined for performance */
  स_रखो(PARSE_HDR(&sParse), 0, PARSE_HDR_SZ);
  स_रखो(PARSE_पुच्छ(&sParse), 0, PARSE_पुच्छ_SZ);
  sParse.pOuterParse = db->pParse;
  db->pParse = &sParse;
  sParse.db = db;
  अगर( pReprepare ){
    sParse.pReprepare = pReprepare;
    sParse.explain = sqlite3_stmt_isexplain((sqlite3_stmt*)pReprepare);
  }अन्यथा{
    निश्चित( sParse.pReprepare==0 );
  }
  निश्चित( ppStmt && *ppStmt==0 );
  अगर( db->mallocFailed ){
    sqlite3_Error_संदेश(&sParse, "out of memory");
    db->errCode = rc = SQLITE_NOMEM;
    जाओ end_prepare;
  }
  निश्चित( sqlite3_mutex_held(db->mutex) );

  /* For a long-term use prepared statement avoid the use of
  ** lookaside memory.
  */
  अगर( prepFlags & SQLITE_PREPARE_PERSISTENT ){
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
  अगर( !db->noSharedCache ){
    क्रम(i=0; i<db->nDb; i++) {
      Btree *pBt = db->aDb[i].pBt;
      अगर( pBt ){
        निश्चित( sqlite3BtreeHoldsMutex(pBt) );
        rc = sqlite3BtreeSchemaLocked(pBt);
        अगर( rc ){
          स्थिर अक्षर *zDb = db->aDb[i].zDbSName;
          sqlite3_Error_With_संदेश(db, rc, "database schema is locked: %s", zDb);
          testcase( db->flags & SQLITE_ReadUncommit );
          जाओ end_prepare;
        }
      }
    }
  }

#अगर_अघोषित SQLITE_OMIT_VIRTUALTABLE
  अगर( db->pDisconnect ) sqlite3VtabUnlockList(db);
#पूर्ण_अगर

  अगर( nBytes>=0 && (nBytes==0 || zSql[nBytes-1]!=0) ){
    अक्षर *zSqlCopy;
    पूर्णांक mxLen = db->aLimit[SQLITE_LIMIT_SQL_लंबाई];
    testcase( nBytes==mxLen );
    testcase( nBytes==mxLen+1 );
    अगर( nBytes>mxLen ){
      sqlite3_Error_With_संदेश(db, SQLITE_TOOBIG, "statement too long");
      rc = sqlite3ApiExit(db, SQLITE_TOOBIG);
      जाओ end_prepare;
    }
    zSqlCopy = sqlite3DbStrNDup(db, zSql, nBytes);
    अगर( zSqlCopy ){
      sqlite3RunParser(&sParse, zSqlCopy);
      sParse.z_पुच्छ = &zSql[sParse.z_पुच्छ-zSqlCopy];
      sqlite3DbFree(db, zSqlCopy);
    }अन्यथा{
      sParse.z_पुच्छ = &zSql[nBytes];
    }
  }अन्यथा{
    sqlite3RunParser(&sParse, zSql);
  }
  निश्चित( 0==sParse.nQueryLoop );

  अगर( pz_पुच्छ ){
    *pz_पुच्छ = sParse.z_पुच्छ;
  }

  अगर( db->init.busy==0 ){
    sqlite3VdbeSetSql(sParse.pVdbe, zSql, (पूर्णांक)(sParse.z_पुच्छ-zSql), prepFlags);
  }
  अगर( db->mallocFailed ){
    sParse.rc = SQLITE_NOMEM_BKPT;
    sParse.checkSchema = 0;
  }
  अगर( sParse.rc!=SQLITE_OK && sParse.rc!=SQLITE_DONE ){
    अगर( sParse.checkSchema && db->init.busy==0 ){
      schemaIsValid(&sParse);
    }
    अगर( sParse.pVdbe ){
      sqlite3VdbeFinalize(sParse.pVdbe);
    }
    निश्चित( 0==(*ppStmt) );
    rc = sParse.rc;
    अगर( sParse.z_Err_संदेश ){
      sqlite3_Error_With_संदेश(db, rc, "%s", sParse.z_Err_संदेश);
      sqlite3DbFree(db, sParse.z_Err_संदेश);
    }अन्यथा{
      sqlite3Error(db, rc);
    }
  }अन्यथा{
    निश्चित( sParse.z_Err_संदेश==0 );
    *ppStmt = (sqlite3_stmt*)sParse.pVdbe;
    rc = SQLITE_OK;
    sqlite3ErrorClear(db);
  }


  /* Delete any TriggerPrg structures allocated while parsing this statement. */
  जबतक( sParse.pTriggerPrg ){
    TriggerPrg *pT = sParse.pTriggerPrg;
    sParse.pTriggerPrg = pT->p_अग्र;
    sqlite3DbFree(db, pT);
  }

end_prepare:

  sqlite3ParseObjectReset(&sParse);
  वापस rc;
}
अटल पूर्णांक sqlite3LockAndPrepare(
  sqlite3 *db,              /* Database handle. */
  स्थिर अक्षर *zSql,         /* UTF-8 encoded SQL statement. */
  पूर्णांक nBytes,               /* Length of zSql in bytes. */
  u32 prepFlags,            /* Zero or more SQLITE_PREPARE_* flags */
  Vdbe *pOld,               /* VM being reprepared */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  स्थिर अक्षर **pz_पुच्छ       /* OUT: End of parsed string */
){
  पूर्णांक rc;
  पूर्णांक cnt = 0;

#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( ppStmt==0 ) वापस SQLITE_MISUSE_BKPT;
#पूर्ण_अगर
  *ppStmt = 0;
  अगर( !sqlite3SafetyCheckOk(db)||zSql==0 ){
    वापस SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(db->mutex);
  sqlite3BtreeEnterAll(db);
  करो{
    /* Make multiple attempts to compile the SQL, until it either succeeds
    ** or encounters a permanent error.  A schema problem after one schema
    ** reset is considered a permanent error. */
    rc = sqlite3Prepare(db, zSql, nBytes, prepFlags, pOld, ppStmt, pz_पुच्छ);
    निश्चित( rc==SQLITE_OK || *ppStmt==0 );
    अगर( rc==SQLITE_OK || db->mallocFailed ) अवरोध;
    cnt++;
  }जबतक( (rc==SQLITE_ERROR_RETRY && ALWAYS(cnt<=SQLITE_MAX_PREPARE_RETRY))
       || (rc==SQLITE_SCHEMA && (sqlite3ResetOneSchema(db,-1), cnt)==1) );
  sqlite3BtreeLeaveAll(db);
  निश्चित( rc!=SQLITE_ERROR_RETRY );
  rc = sqlite3ApiExit(db, rc);
  निश्चित( (rc&db->errMask)==rc );
  db->busyHandler.nBusy = 0;
  sqlite3_mutex_leave(db->mutex);
  निश्चित( rc==SQLITE_OK || (*ppStmt)==0 );
  वापस rc;
}


/*
** Rerun the compilation of a statement after a schema change.
**
** If the statement is successfully recompiled, return SQLITE_OK. Otherwise,
** if the statement cannot be recompiled because another connection has
** locked the sqlite3_schema table, return SQLITE_LOCKED. If any other error
** occurs, return SQLITE_SCHEMA.
*/
पूर्णांक sqlite3Reprepare(Vdbe *p){
  पूर्णांक rc;
  sqlite3_stmt *pNew;
  स्थिर अक्षर *zSql;
  sqlite3 *db;
  u8 prepFlags;

  निश्चित( sqlite3_mutex_held(sqlite3VdbeDb(p)->mutex) );
  zSql = sqlite3_sql((sqlite3_stmt *)p);
  निश्चित( zSql!=0 );  /* Reprepare only called for prepare_v2() statements */
  db = sqlite3VdbeDb(p);
  निश्चित( sqlite3_mutex_held(db->mutex) );
  prepFlags = sqlite3VdbePrepareFlags(p);
  rc = sqlite3LockAndPrepare(db, zSql, -1, prepFlags, p, &pNew, 0);
  अगर( rc ){
    अगर( rc==SQLITE_NOMEM ){
      sqlite3OomFault(db);
    }
    निश्चित( pNew==0 );
    वापस rc;
  }अन्यथा{
    निश्चित( pNew!=0 );
  }
  sqlite3VdbeSwap((Vdbe*)pNew, p);
  sqlite3TransferBindings(pNew, (sqlite3_stmt*)p);
  sqlite3_Vdbe_Reset_Step_परिणाम((Vdbe*)pNew);
  sqlite3VdbeFinalize((Vdbe*)pNew);
  वापस SQLITE_OK;
}


/*
** Two versions of the official API.  Legacy and new use.  In the legacy
** version, the original SQL text is not saved in the prepared statement
** and so if a schema change occurs, SQLITE_SCHEMA is returned by
** sqlite3_step().  In the new version, the original SQL text is retained
** and the statement is automatically recompiled if an schema change
** occurs.
*/
पूर्णांक sqlite3_prepare(
  sqlite3 *db,              /* Database handle. */
  स्थिर अक्षर *zSql,         /* UTF-8 encoded SQL statement. */
  पूर्णांक nBytes,               /* Length of zSql in bytes. */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  स्थिर अक्षर **pz_पुच्छ       /* OUT: End of parsed string */
){
  पूर्णांक rc;
  rc = sqlite3LockAndPrepare(db,zSql,nBytes,0,0,ppStmt,pz_पुच्छ);
  निश्चित( rc==SQLITE_OK || ppStmt==0 || *ppStmt==0 );  /* VERIFY: F13021 */
  वापस rc;
}
पूर्णांक sqlite3_prepare_v2(
  sqlite3 *db,              /* Database handle. */
  स्थिर अक्षर *zSql,         /* UTF-8 encoded SQL statement. */
  पूर्णांक nBytes,               /* Length of zSql in bytes. */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  स्थिर अक्षर **pz_पुच्छ       /* OUT: End of parsed string */
){
  पूर्णांक rc;
  /* EVIDENCE-OF: R-37923-12173 The sqlite3_prepare_v2() interface works
  ** exactly the same as sqlite3_prepare_v3() with a zero prepFlags
  ** parameter.
  **
  ** Proof in that the 5th parameter to sqlite3LockAndPrepare is 0 */
  rc = sqlite3LockAndPrepare(db,zSql,nBytes,SQLITE_PREPARE_SAVESQL,0,
                             ppStmt,pz_पुच्छ);
  निश्चित( rc==SQLITE_OK || ppStmt==0 || *ppStmt==0 );
  वापस rc;
}
पूर्णांक sqlite3_prepare_v3(
  sqlite3 *db,              /* Database handle. */
  स्थिर अक्षर *zSql,         /* UTF-8 encoded SQL statement. */
  पूर्णांक nBytes,               /* Length of zSql in bytes. */
  अचिन्हित पूर्णांक prepFlags,   /* Zero or more SQLITE_PREPARE_* flags */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  स्थिर अक्षर **pz_पुच्छ       /* OUT: End of parsed string */
){
  पूर्णांक rc;
  /* EVIDENCE-OF: R-56861-42673 sqlite3_prepare_v3() differs from
  ** sqlite3_prepare_v2() only in having the extra prepFlags parameter,
  ** which is a bit array consisting of zero or more of the
  ** SQLITE_PREPARE_* flags.
  **
  ** Proof by comparison to the implementation of sqlite3_prepare_v2()
  ** directly above. */
  rc = sqlite3LockAndPrepare(db,zSql,nBytes,
                 SQLITE_PREPARE_SAVESQL|(prepFlags&SQLITE_PREPARE_MASK),
                 0,ppStmt,pz_पुच्छ);
  निश्चित( rc==SQLITE_OK || ppStmt==0 || *ppStmt==0 );
  वापस rc;
}


#अगर_अघोषित SQLITE_OMIT_UTF16
/*
** Compile the UTF-16 encoded SQL statement zSql into a statement handle.
*/
अटल पूर्णांक sqlite3Prepare16(
  sqlite3 *db,              /* Database handle. */ 
  स्थिर व्योम *zSql,         /* UTF-16 encoded SQL statement. */
  पूर्णांक nBytes,               /* Length of zSql in bytes. */
  u32 prepFlags,            /* Zero or more SQLITE_PREPARE_* flags */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  स्थिर व्योम **pz_पुच्छ       /* OUT: End of parsed string */
){
  /* This function currently works by first transforming the UTF-16
  ** encoded string to UTF-8, then invoking sqlite3_prepare(). The
  ** tricky bit is figuring out the pointer to return in *pzTail.
  */
  अक्षर *zSql8;
  स्थिर अक्षर *zTail8 = 0;
  पूर्णांक rc = SQLITE_OK;

#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( ppStmt==0 ) वापस SQLITE_MISUSE_BKPT;
#पूर्ण_अगर
  *ppStmt = 0;
  अगर( !sqlite3SafetyCheckOk(db)||zSql==0 ){
    वापस SQLITE_MISUSE_BKPT;
  }

  /* Make sure nBytes is non-negative and correct.  It should be the
  ** number of bytes until the end of the input buffer or until the first
  ** U+0000 character.  If the input nBytes is odd, convert it into
  ** an even number.  If the input nBytes is negative, then the input
  ** must be terminated by at least one U+0000 character */
  अगर( nBytes>=0 ){
    पूर्णांक sz;
    स्थिर अक्षर *z = (स्थिर अक्षर*)zSql;
    क्रम(sz=0; sz<nBytes && (z[sz]!=0 || z[sz+1]!=0); sz += 2){}
    nBytes = sz;
  }अन्यथा{
    पूर्णांक sz;
    स्थिर अक्षर *z = (स्थिर अक्षर*)zSql;
    क्रम(sz=0; z[sz]!=0 || z[sz+1]!=0; sz += 2){}
    nBytes = sz;
  }

  sqlite3_mutex_enter(db->mutex);
  zSql8 = sqlite3Utf16to8(db, zSql, nBytes, SQLITE_UTF16NATIVE);
  अगर( zSql8 ){
    rc = sqlite3LockAndPrepare(db, zSql8, -1, prepFlags, 0, ppStmt, &zTail8);
  }

  अगर( zTail8 && pz_पुच्छ ){
    /* If sqlite3_prepare returns a tail pointer, we calculate the
    ** equivalent pointer into the UTF-16 string by counting the unicode
    ** characters between zSql8 and zTail8, and then returning a pointer
    ** the same number of characters into the UTF-16 string.
    */
    पूर्णांक chars_parsed = sqlite3Utf8CharLen(zSql8, (पूर्णांक)(zTail8-zSql8));
    *pz_पुच्छ = (u8 *)zSql + sqlite3Utf16ByteLen(zSql, nBytes, chars_parsed);
  }
  sqlite3DbFree(db, zSql8); 
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  वापस rc;
}

/*
** Two versions of the official API.  Legacy and new use.  In the legacy
** version, the original SQL text is not saved in the prepared statement
** and so if a schema change occurs, SQLITE_SCHEMA is returned by
** sqlite3_step().  In the new version, the original SQL text is retained
** and the statement is automatically recompiled if an schema change
** occurs.
*/
पूर्णांक sqlite3_prepare16(
  sqlite3 *db,              /* Database handle. */ 
  स्थिर व्योम *zSql,         /* UTF-16 encoded SQL statement. */
  पूर्णांक nBytes,               /* Length of zSql in bytes. */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  स्थिर व्योम **pz_पुच्छ       /* OUT: End of parsed string */
){
  पूर्णांक rc;
  rc = sqlite3Prepare16(db,zSql,nBytes&~1,0,ppStmt,pz_पुच्छ);
  निश्चित( rc==SQLITE_OK || ppStmt==0 || *ppStmt==0 );  /* VERIFY: F13021 */
  वापस rc;
}
पूर्णांक sqlite3_prepare16_v2(
  sqlite3 *db,              /* Database handle. */ 
  स्थिर व्योम *zSql,         /* UTF-16 encoded SQL statement. */
  पूर्णांक nBytes,               /* Length of zSql in bytes. */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  स्थिर व्योम **pz_पुच्छ       /* OUT: End of parsed string */
){
  पूर्णांक rc;
  rc = sqlite3Prepare16(db,zSql,nBytes&~1,SQLITE_PREPARE_SAVESQL,ppStmt,pz_पुच्छ);
  निश्चित( rc==SQLITE_OK || ppStmt==0 || *ppStmt==0 );  /* VERIFY: F13021 */
  वापस rc;
}
पूर्णांक sqlite3_prepare16_v3(
  sqlite3 *db,              /* Database handle. */ 
  स्थिर व्योम *zSql,         /* UTF-16 encoded SQL statement. */
  पूर्णांक nBytes,               /* Length of zSql in bytes. */
  अचिन्हित पूर्णांक prepFlags,   /* Zero or more SQLITE_PREPARE_* flags */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  स्थिर व्योम **pz_पुच्छ       /* OUT: End of parsed string */
){
  पूर्णांक rc;
  rc = sqlite3Prepare16(db,zSql,nBytes&~1,
         SQLITE_PREPARE_SAVESQL|(prepFlags&SQLITE_PREPARE_MASK),
         ppStmt,pz_पुच्छ);
  निश्चित( rc==SQLITE_OK || ppStmt==0 || *ppStmt==0 );  /* VERIFY: F13021 */
  वापस rc;
}

#पूर्ण_अगर /* SQLITE_OMIT_UTF16 */
