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
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"

/*
** Fill the InitData structure with an error message that indicates
** that the database is corrupt.
*/
ಸ್ಥಿರ ಶೂನ್ಯ corruptSchema(
  InitData *pData,     /* Initialization context */
  ವರ್ಣ **azObj,        /* Type and name of object being parsed */
  ಧ್ರುವ ವರ್ಣ *zExtra   /* Error information */
){
  sqlite3 *db = pData->db;
  ಯದಿ( db->mallocFailed ){
    pData->rc = SQLITE_NOMEM_BKPT;
  }ಅನ್ಯಥಾ ಯದಿ( pData->pz_Err_ಸಂದೇಶ[0]!=0 ){
    /* A error message has already been generated.  Do not overwrite it */
  }ಅನ್ಯಥಾ ಯದಿ( pData->mInitFlags & (INITFLAG_AlterMask) ){
    ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *azAlterType[] = {
       "rename",
       "drop column",
       "add column",
       "drop constraint"
    };
    *pData->pz_Err_ಸಂದೇಶ = sqlite3MPrintf(db, 
        "error in %s %s after %s: %s", azObj[0], azObj[1], 
        azAlterType[(pData->mInitFlags&INITFLAG_AlterMask)-1], 
        zExtra
    );
    pData->rc = SQLITE_ERROR;
  }ಅನ್ಯಥಾ ಯದಿ( db->flags & SQLITE_WriteSchema ){
    pData->rc = SQLITE_CORRUPT_BKPT;
  }ಅನ್ಯಥಾ{
    ವರ್ಣ *z;
    ಧ್ರುವ ವರ್ಣ *zObj = azObj[1] ? azObj[1] : "?";
    z = sqlite3MPrintf(db, "malformed database schema (%s)", zObj);
    ಯದಿ( zExtra && zExtra[0] ) z = sqlite3MPrintf(db, "%z - %s", z, zExtra);
    *pData->pz_Err_ಸಂದೇಶ = z;
    pData->rc = SQLITE_CORRUPT_BKPT;
  }
}

/*
** Check to see if any sibling index (another index on the same table)
** of pIndex has the same root page number, and if it does, return true.
** This would indicate a corrupt schema.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_Index_Has_Duplicate_ಬೇರು_Page(Index *pIndex){
  Index *p;
  ಪ್ರತಿ(p=pIndex->pTable->pIndex; p; p=p->pNext){
    ಯದಿ( p->tnum==pIndex->tnum && p!=pIndex ) ಪ್ರತ್ಯಾಗಮ 1;
  }
  ಪ್ರತ್ಯಾಗಮ 0;
}

/* forward declaration */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ sqlite3Prepare(
  sqlite3 *db,              /* Database handle. */
  ಧ್ರುವ ವರ್ಣ *zSql,         /* UTF-8 encoded SQL statement. */
  ಪೂರ್ಣಾಙ್ಕ nBytes,               /* Length of zSql in bytes. */
  u32 prepFlags,            /* Zero or more SQLITE_PREPARE_* flags */
  Vdbe *pReprepare,         /* VM being reprepared */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  ಧ್ರುವ ವರ್ಣ **pzTail       /* OUT: End of parsed string */
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
ಪೂರ್ಣಾಙ್ಕ sqlite3InitCallback(ಶೂನ್ಯ *pInit, ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv, ವರ್ಣ **NotUsed){
  InitData *pData = (InitData*)pInit;
  sqlite3 *db = pData->db;
  ಪೂರ್ಣಾಙ್ಕ iDb = pData->iDb;

  ಪ್ರತಿಜ್ಞಾ( argc==5 );
  UNUSED_PARAMETER2(NotUsed, argc);
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(db->mutex) );
  db->mDbFlags |= DBFLAG_EncodingFixed;
  ಯದಿ( argv==0 ) ಪ್ರತ್ಯಾಗಮ 0;   /* Might happen if EMPTY_RESULT_CALLBACKS are on */
  pData->nInitRow++;
  ಯದಿ( db->mallocFailed ){
    corruptSchema(pData, argv, 0);
    ಪ್ರತ್ಯಾಗಮ 1;
  }

  ಪ್ರತಿಜ್ಞಾ( iDb>=0 && iDb<db->nDb );
  ಪ್ರತಿಜ್ಞಾ( db->aDb[iDb].pSchema!=0 );
  ಯದಿ( argv[3]==0 ){
    corruptSchema(pData, argv, 0);
  }ಅನ್ಯಥಾ ಯದಿ( argv[4]
         && 'c'==sqlite3UpperToLower[(unsigned ವರ್ಣ)argv[4][0]]
         && 'r'==sqlite3UpperToLower[(unsigned ವರ್ಣ)argv[4][1]] ){
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
    ಪೂರ್ಣಾಙ್ಕ rc;
    u8 saved_iDb = db->init.iDb;
    sqlite3_stmt *pStmt;
    ಧ್ರುವ ವರ್ಣ *zEnd;
    TESTONLY(ಪೂರ್ಣಾಙ್ಕ rcp);            /* Return code from sqlite3_prepare() */

    ಪ್ರತಿಜ್ಞಾ( db->init.busy );
    db->init.iDb = iDb;
    ಯದಿ( sqlite3GetUInt32(argv[3], &db->init.newTnum)==0
     || (db->init.newTnum>pData->mxPage && pData->mxPage>0)
    ){
      ಯದಿ( sqlite3Config.bExtraSchemaChecks ){
        corruptSchema(pData, argv, "invalid rootpage");
      }
    }
    db->init.orphanTrigger = 0;
    db->init.azInit = (ಧ್ರುವ ವರ್ಣ**)argv;
    pStmt = 0;
    TESTONLY(rcp = ) sqlite3Prepare(db, argv[4], -1, 0, 0, &pStmt, &zEnd);
    rc = db->errCode;
    ಪ್ರತಿಜ್ಞಾ( (rc&0xFF)==(rcp&0xFF) );
    db->init.iDb = saved_iDb;
    /* assert( saved_iDb==0 || (db->mDbFlags & DBFLAG_Vacuum)!=0 ); */
    ಯದಿ( SQLITE_OK!=rc ){
      ಯದಿ( db->init.orphanTrigger ){
        ಪ್ರತಿಜ್ಞಾ( iDb==1 );
      }ಅನ್ಯಥಾ{
        ಯದಿ( rc > pData->rc ) pData->rc = rc;
        ಯದಿ( rc==SQLITE_NOMEM ){
          sqlite3OomFault(db);
        }ಅನ್ಯಥಾ ಯದಿ( rc!=SQLITE_INTERRUPT && (rc&0xFF)!=SQLITE_LOCKED ){
          corruptSchema(pData, argv, sqlite3_errmsg(db));
        }
      }
    }ಅನ್ಯಥಾ ಯದಿ( zEnd[0] ){
      corruptSchema(pData, argv, 0);
    }
    db->init.azInit = sqlite3StdType; /* Any array of string ptrs will do */
    sqlite3_finalize(pStmt);
  }ಅನ್ಯಥಾ ಯದಿ( argv[1]==0 || (argv[4]!=0 && argv[4][0]!=0) ){
    corruptSchema(pData, argv, 0);
  }ಅನ್ಯಥಾ{
    /* If the SQL column is blank it means this is an index that
    ** was created to be the PRIMARY KEY or to fulfill a UNIQUE
    ** constraint for a CREATE TABLE.  The index should have already
    ** been created when we processed the CREATE TABLE.  All we have
    ** to do here is record the root page number for that index.
    */
    Index *pIndex;
    pIndex = sqlite3FindIndex(db, argv[1], db->aDb[iDb].zDbSName);
    ಯದಿ( pIndex==0 ){
      corruptSchema(pData, argv, "orphan index");
    }ಅನ್ಯಥಾ
    ಯದಿ( sqlite3GetUInt32(argv[3],&pIndex->tnum)==0
     || pIndex->tnum<2
     || pIndex->tnum>pData->mxPage
     || sqlite3_Index_Has_Duplicate_ಬೇರು_Page(pIndex)
    ){
      ಯದಿ( sqlite3Config.bExtraSchemaChecks ){
        corruptSchema(pData, argv, "invalid rootpage");
      }
    }
  }
  ಯದಿ( pData->pz_Err_ಸಂದೇಶ[0]==0 ){
    Schema *pX = db->aDb[iDb].pSchema;
    ಯದಿ( pX->tblHash.ಎಣಿಕೆ + pX->idxHash.ಎಣಿಕೆ + pX->trigHash.ಎಣಿಕೆ
                   > (u32)db->aLimit[SQLITE_LIMIT_SCHEMA]
    ){
      *pData->pz_Err_ಸಂದೇಶ = sqlite3MPrintf(db, "too many schema objects");
      pData->rc = SQLITE_ERROR;
      ಪ್ರತ್ಯಾಗಮ 1;
    }
  }
  ಪ್ರತ್ಯಾಗಮ 0;
}

/*
** Attempt to read the database schema and initialize internal
** data structures for a single database file.  The index of the
** database file is given by iDb.  iDb==0 is used for the main
** database.  iDb==1 should never be used.  iDb>=2 is used for
** auxiliary databases.  Return one of the SQLITE_ error codes to
** indicate success or failure.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3InitOne(sqlite3 *db, ಪೂರ್ಣಾಙ್ಕ iDb, ವರ್ಣ **pz_Err_ಸಂದೇಶ, u32 mFlags){
  ಪೂರ್ಣಾಙ್ಕ rc;
  ಪೂರ್ಣಾಙ್ಕ i;
#ifndef SQLITE_OMIT_DEPRECATED
  ಪೂರ್ಣಾಙ್ಕ size;
#endif
  Db *pDb;
  ವರ್ಣ ಧ್ರುವ *azArg[6];
  ಪೂರ್ಣಾಙ್ಕ meta[5];
  InitData initData;
  ಧ್ರುವ ವರ್ಣ *zSchemaTabName;
  ಪೂರ್ಣಾಙ್ಕ openedTransaction = 0;
  ಪೂರ್ಣಾಙ್ಕ mask = ((db->mDbFlags & DBFLAG_EncodingFixed) | ~DBFLAG_EncodingFixed);

  ಪ್ರತಿಜ್ಞಾ( (db->mDbFlags & DBFLAG_SchemaKnownOk)==0 );
  ಪ್ರತಿಜ್ಞಾ( iDb>=0 && iDb<db->nDb );
  ಪ್ರತಿಜ್ಞಾ( db->aDb[iDb].pSchema );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(db->mutex) );
  ಪ್ರತಿಜ್ಞಾ( iDb==1 || sqlite3BtreeHoldsMutex(db->aDb[iDb].pBt) );

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
  initData.pz_Err_ಸಂದೇಶ = pz_Err_ಸಂದೇಶ;
  initData.mInitFlags = mFlags;
  initData.nInitRow = 0;
  initData.mxPage = 0;
  sqlite3InitCallback(&initData, 5, (ವರ್ಣ **)azArg, 0);
  db->mDbFlags &= mask;
  ಯದಿ( initData.rc ){
    rc = initData.rc;
    ಗಚ್ಛ error_out;
  }

  /* Create a cursor to hold the database open
  */
  pDb = &db->aDb[iDb];
  ಯದಿ( pDb->pBt==0 ){
    ಪ್ರತಿಜ್ಞಾ( iDb==1 );
    DbSetProperty(db, 1, DB_SchemaLoaded);
    rc = SQLITE_OK;
    ಗಚ್ಛ error_out;
  }

  /* If there is not already a read-only (or read-write) transaction opened
  ** on the b-tree database, open one now. If a transaction is opened, it 
  ** will be closed before this function returns.  */
  sqlite3BtreeEnter(pDb->pBt);
  ಯದಿ( sqlite3BtreeTxnState(pDb->pBt)==SQLITE_TXN_NONE ){
    rc = sqlite3BtreeBeginTrans(pDb->pBt, 0, 0);
    ಯದಿ( rc!=SQLITE_OK ){
      sqlite3SetString(pz_Err_ಸಂದೇಶ, db, sqlite3ErrStr(rc));
      ಗಚ್ಛ initone_error_out;
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
  ಪ್ರತಿ(i=0; i<ArraySize(meta); i++){
    sqlite3BtreeGetMeta(pDb->pBt, i+1, (u32 *)&meta[i]);
  }
  ಯದಿ( (db->flags & SQLITE_ResetDatabase)!=0 ){
    memset(meta, 0, ಪರಿಮಾಣ(meta));
  }
  pDb->pSchema->schema_cookie = meta[BTREE_SCHEMA_VERSION-1];

  /* If opening a non-empty database, check the text encoding. For the
  ** main database, set sqlite3.enc to the encoding of the main database.
  ** For an attached db, it is an error if the encoding is not the same
  ** as sqlite3.enc.
  */
  ಯದಿ( meta[BTREE_TEXT_ENCODING-1] ){  /* text encoding */
    ಯದಿ( iDb==0 && (db->mDbFlags & DBFLAG_EncodingFixed)==0 ){
      u8 encoding;
#ifndef SQLITE_OMIT_UTF16
      /* If opening the main database, set ENC(db). */
      encoding = (u8)meta[BTREE_TEXT_ENCODING-1] & 3;
      ಯದಿ( encoding==0 ) encoding = SQLITE_UTF8;
#ಅನ್ಯಥಾ
      encoding = SQLITE_UTF8;
#endif
      sqlite3SetTextEncoding(db, encoding);
    }ಅನ್ಯಥಾ{
      /* If opening an attached database, the encoding much match ENC(db) */
      ಯದಿ( (meta[BTREE_TEXT_ENCODING-1] & 3)!=ENC(db) ){
        sqlite3SetString(pz_Err_ಸಂದೇಶ, db, "attached databases must use the same"
            " text encoding as main database");
        rc = SQLITE_ERROR;
        ಗಚ್ಛ initone_error_out;
      }
    }
  }
  pDb->pSchema->enc = ENC(db);

  ಯದಿ( pDb->pSchema->cache_size==0 ){
#ifndef SQLITE_OMIT_DEPRECATED
    size = sqlite3AbsInt32(meta[BTREE_DEFAULT_CACHE_SIZE-1]);
    ಯದಿ( size==0 ){ size = SQLITE_DEFAULT_CACHE_SIZE; }
    pDb->pSchema->cache_size = size;
#ಅನ್ಯಥಾ
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
  ಯದಿ( pDb->pSchema->file_format==0 ){
    pDb->pSchema->file_format = 1;
  }
  ಯದಿ( pDb->pSchema->file_format>SQLITE_MAX_FILE_FORMAT ){
    sqlite3SetString(pz_Err_ಸಂದೇಶ, db, "unsupported file format");
    rc = SQLITE_ERROR;
    ಗಚ್ಛ initone_error_out;
  }

  /* Ticket #2804:  When we open a database in the newer file format,
  ** clear the legacy_file_format pragma flag so that a VACUUM will
  ** not downgrade the database and thus invalidate any descending
  ** indices that the user might have created.
  */
  ಯದಿ( iDb==0 && meta[BTREE_FILE_FORMAT-1]>=4 ){
    db->flags &= ~(u64)SQLITE_LegacyFileFmt;
  }

  /* Read the schema information out of the schema tables
  */
  ಪ್ರತಿಜ್ಞಾ( db->init.busy );
  initData.mxPage = sqlite3BtreeLastPage(pDb->pBt);
  {
    ವರ್ಣ *zSql;
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
    ಯದಿ( rc==SQLITE_OK ) rc = initData.rc;
    sqlite3DbFree(db, zSql);
#ifndef SQLITE_OMIT_ANALYZE
    ಯದಿ( rc==SQLITE_OK ){
      sqlite3AnalysisLoad(db, iDb);
    }
#endif
  }
  ಪ್ರತಿಜ್ಞಾ( pDb == &(db->aDb[iDb]) );
  ಯದಿ( db->mallocFailed ){
    rc = SQLITE_NOMEM_BKPT;
    sqlite3ResetAllSchemasOfConnection(db);
    pDb = &db->aDb[iDb];
  }ಅನ್ಯಥಾ
  ಯದಿ( rc==SQLITE_OK || ((db->flags&SQLITE_NoSchemaError) && rc!=SQLITE_NOMEM)){
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
  ಯದಿ( openedTransaction ){
    sqlite3BtreeCommit(pDb->pBt);
  }
  sqlite3BtreeLeave(pDb->pBt);

error_out:
  ಯದಿ( rc ){
    ಯದಿ( rc==SQLITE_NOMEM || rc==SQLITE_IOERR_NOMEM ){
      sqlite3OomFault(db);
    }
    sqlite3ResetOneSchema(db, iDb);
  }
  db->init.busy = 0;
  ಪ್ರತ್ಯಾಗಮ rc;
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
ಪೂರ್ಣಾಙ್ಕ sqlite3Init(sqlite3 *db, ವರ್ಣ **pz_Err_ಸಂದೇಶ){
  ಪೂರ್ಣಾಙ್ಕ i, rc;
  ಪೂರ್ಣಾಙ್ಕ commit_internal = !(db->mDbFlags&DBFLAG_SchemaChange);
  
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(db->mutex) );
  ಪ್ರತಿಜ್ಞಾ( sqlite3BtreeHoldsMutex(db->aDb[0].pBt) );
  ಪ್ರತಿಜ್ಞಾ( db->init.busy==0 );
  ENC(db) = SCHEMA_ENC(db);
  ಪ್ರತಿಜ್ಞಾ( db->nDb>0 );
  /* Do the main schema first */
  ಯದಿ( !DbHasProperty(db, 0, DB_SchemaLoaded) ){
    rc = sqlite3InitOne(db, 0, pz_Err_ಸಂದೇಶ, 0);
    ಯದಿ( rc ) ಪ್ರತ್ಯಾಗಮ rc;
  }
  /* All other schemas after the main schema. The "temp" schema must be last */
  ಪ್ರತಿ(i=db->nDb-1; i>0; i--){
    ಪ್ರತಿಜ್ಞಾ( i==1 || sqlite3BtreeHoldsMutex(db->aDb[i].pBt) );
    ಯದಿ( !DbHasProperty(db, i, DB_SchemaLoaded) ){
      rc = sqlite3InitOne(db, i, pz_Err_ಸಂದೇಶ, 0);
      ಯದಿ( rc ) ಪ್ರತ್ಯಾಗಮ rc;
    }
  }
  ಯದಿ( commit_internal ){
    sqlite3CommitInternalChanges(db);
  }
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** This routine is a no-op if the database schema is already initialized.
** Otherwise, the schema is loaded. An error code is returned.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3ReadSchema(Parse *pParse){
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;
  sqlite3 *db = pParse->db;
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(db->mutex) );
  ಯದಿ( !db->init.busy ){
    rc = sqlite3Init(db, &pParse->z_Err_ಸಂದೇಶ);
    ಯದಿ( rc!=SQLITE_OK ){
      pParse->rc = rc;
      pParse->nErr++;
    }ಅನ್ಯಥಾ ಯದಿ( db->noSharedCache ){
      db->mDbFlags |= DBFLAG_SchemaKnownOk;
    }
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}


/*
** Check schema cookies in all databases.  If any cookie is out
** of date set pParse->rc to SQLITE_SCHEMA.  If all schema cookies
** make no changes to pParse->rc.
*/
ಸ್ಥಿರ ಶೂನ್ಯ schemaIsValid(Parse *pParse){
  sqlite3 *db = pParse->db;
  ಪೂರ್ಣಾಙ್ಕ iDb;
  ಪೂರ್ಣಾಙ್ಕ rc;
  ಪೂರ್ಣಾಙ್ಕ cookie;

  ಪ್ರತಿಜ್ಞಾ( pParse->checkSchema );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(db->mutex) );
  ಪ್ರತಿ(iDb=0; iDb<db->nDb; iDb++){
    ಪೂರ್ಣಾಙ್ಕ openedTransaction = 0;         /* True if a transaction is opened */
    Btree *pBt = db->aDb[iDb].pBt;     /* Btree database to read cookie from */
    ಯದಿ( pBt==0 ) ಅನುವರ್ತ;

    /* If there is not already a read-only (or read-write) transaction opened
    ** on the b-tree database, open one now. If a transaction is opened, it 
    ** will be closed immediately after reading the meta-value. */
    ಯದಿ( sqlite3BtreeTxnState(pBt)==SQLITE_TXN_NONE ){
      rc = sqlite3BtreeBeginTrans(pBt, 0, 0);
      ಯದಿ( rc==SQLITE_NOMEM || rc==SQLITE_IOERR_NOMEM ){
        sqlite3OomFault(db);
        pParse->rc = SQLITE_NOMEM;
      }
      ಯದಿ( rc!=SQLITE_OK ) ಪ್ರತ್ಯಾಗಮ;
      openedTransaction = 1;
    }

    /* Read the schema cookie from the database. If it does not match the 
    ** value stored as part of the in-memory schema representation,
    ** set Parse.rc to SQLITE_SCHEMA. */
    sqlite3BtreeGetMeta(pBt, BTREE_SCHEMA_VERSION, (u32 *)&cookie);
    ಪ್ರತಿಜ್ಞಾ( sqlite3SchemaMutexHeld(db, iDb, 0) );
    ಯದಿ( cookie!=db->aDb[iDb].pSchema->schema_cookie ){
      ಯದಿ( DbHasProperty(db, iDb, DB_SchemaLoaded) ) pParse->rc = SQLITE_SCHEMA;
      sqlite3ResetOneSchema(db, iDb);
    }

    /* Close the transaction, if one was opened. */
    ಯದಿ( openedTransaction ){
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
ಪೂರ್ಣಾಙ್ಕ sqlite3SchemaToIndex(sqlite3 *db, Schema *pSchema){
  ಪೂರ್ಣಾಙ್ಕ i = -32768;

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
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(db->mutex) );
  ಯದಿ( pSchema ){
    ಪ್ರತಿ(i=0; 1; i++){
      ಪ್ರತಿಜ್ಞಾ( i<db->nDb );
      ಯದಿ( db->aDb[i].pSchema==pSchema ){
        ಭಙ್ಗ;
      }
    }
    ಪ್ರತಿಜ್ಞಾ( i>=0 && i<db->nDb );
  }
  ಪ್ರತ್ಯಾಗಮ i;
}

/*
** Free all memory allocations in the pParse object
*/
ಶೂನ್ಯ sqlite3ParseObjectReset(Parse *pParse){
  sqlite3 *db = pParse->db;
  ಪ್ರತಿಜ್ಞಾ( db!=0 );
  ಪ್ರತಿಜ್ಞಾ( db->pParse==pParse );
  ಪ್ರತಿಜ್ಞಾ( pParse->nested==0 );
#ifndef SQLITE_OMIT_SHARED_CACHE
  ಯದಿ( pParse->nTableLock ) sqlite3DbNNFreeNN(db, pParse->aTableLock);
#endif
  ಯಾವತ್( pParse->pCleanup ){
    ParseCleanup *pCleanup = pParse->pCleanup;
    pParse->pCleanup = pCleanup->pNext;
    pCleanup->xCleanup(db, pCleanup->pPtr);
    sqlite3DbNNFreeNN(db, pCleanup);
  }
  ಯದಿ( pParse->aLabel ) sqlite3DbNNFreeNN(db, pParse->aLabel);
  ಯದಿ( pParse->p_Const_ಅಭಿವ್ಯಕ್ತಿ ){
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, pParse->p_Const_ಅಭಿವ್ಯಕ್ತಿ);
  }
  ಪ್ರತಿಜ್ಞಾ( db->lookaside.bDisable >= pParse->disableLookaside );
  db->lookaside.bDisable -= pParse->disableLookaside;
  db->lookaside.sz = db->lookaside.bDisable ? 0 : db->lookaside.szTrue;
  ಪ್ರತಿಜ್ಞಾ( pParse->db->pParse==pParse );
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
ಶೂನ್ಯ *sqlite3ParserAddCleanup(
  Parse *pParse,                      /* Destroy when this Parser finishes */
  ಶೂನ್ಯ (*xCleanup)(sqlite3*,ಶೂನ್ಯ*),   /* The cleanup routine */
  ಶೂನ್ಯ *pPtr                          /* Pointer to object to be cleaned up */
){
  ParseCleanup *pCleanup;
  ಯದಿ( sqlite3FaultSim(300) ){
    pCleanup = 0;
    sqlite3OomFault(pParse->db);
  }ಅನ್ಯಥಾ{
    pCleanup = sqlite3DbMallocRaw(pParse->db, ಪರಿಮಾಣ(*pCleanup));
  }
  ಯದಿ( pCleanup ){
    pCleanup->pNext = pParse->pCleanup;
    pParse->pCleanup = pCleanup;
    pCleanup->pPtr = pPtr;
    pCleanup->xCleanup = xCleanup;
  }ಅನ್ಯಥಾ{
    xCleanup(pParse->db, pPtr);
    pPtr = 0;
#ಯದಿ defined(SQLITE_DEBUG) || defined(SQLITE_COVERAGE_TEST)
    pParse->earlyCleanup = 1;
#endif
  }
  ಪ್ರತ್ಯಾಗಮ pPtr;
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
ಶೂನ್ಯ sqlite3ParseObjectInit(Parse *pParse, sqlite3 *db){
  ಪ್ರತಿಜ್ಞಾ( db!=0 );
  memset(PARSE_HDR(pParse), 0, PARSE_HDR_SZ);
  memset(PARSE_TAIL(pParse), 0, PARSE_TAIL_SZ);
  ಪ್ರತಿಜ್ಞಾ( db->pParse!=pParse );
  pParse->pOuterParse = db->pParse;
  db->pParse = pParse;
  pParse->db = db;
  ಯದಿ( db->mallocFailed ) sqlite3_Error_ಸಂದೇಶ(pParse, "out of memory");
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ sqlite3Prepare(
  sqlite3 *db,              /* Database handle. */
  ಧ್ರುವ ವರ್ಣ *zSql,         /* UTF-8 encoded SQL statement. */
  ಪೂರ್ಣಾಙ್ಕ nBytes,               /* Length of zSql in bytes. */
  u32 prepFlags,            /* Zero or more SQLITE_PREPARE_* flags */
  Vdbe *pReprepare,         /* VM being reprepared */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  ಧ್ರುವ ವರ್ಣ **pzTail       /* OUT: End of parsed string */
){
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;       /* Result code */
  ಪೂರ್ಣಾಙ್ಕ i;                    /* Loop counter */
  Parse sParse;             /* Parsing context */

  /* sqlite3ParseObjectInit(&sParse, db); // inlined for performance */
  memset(PARSE_HDR(&sParse), 0, PARSE_HDR_SZ);
  memset(PARSE_TAIL(&sParse), 0, PARSE_TAIL_SZ);
  sParse.pOuterParse = db->pParse;
  db->pParse = &sParse;
  sParse.db = db;
  ಯದಿ( pReprepare ){
    sParse.pReprepare = pReprepare;
    sParse.explain = sqlite3_stmt_isexplain((sqlite3_stmt*)pReprepare);
  }ಅನ್ಯಥಾ{
    ಪ್ರತಿಜ್ಞಾ( sParse.pReprepare==0 );
  }
  ಪ್ರತಿಜ್ಞಾ( ppStmt && *ppStmt==0 );
  ಯದಿ( db->mallocFailed ){
    sqlite3_Error_ಸಂದೇಶ(&sParse, "out of memory");
    db->errCode = rc = SQLITE_NOMEM;
    ಗಚ್ಛ end_prepare;
  }
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(db->mutex) );

  /* For a long-term use prepared statement avoid the use of
  ** lookaside memory.
  */
  ಯದಿ( prepFlags & SQLITE_PREPARE_PERSISTENT ){
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
  ಯದಿ( !db->noSharedCache ){
    ಪ್ರತಿ(i=0; i<db->nDb; i++) {
      Btree *pBt = db->aDb[i].pBt;
      ಯದಿ( pBt ){
        ಪ್ರತಿಜ್ಞಾ( sqlite3BtreeHoldsMutex(pBt) );
        rc = sqlite3BtreeSchemaLocked(pBt);
        ಯದಿ( rc ){
          ಧ್ರುವ ವರ್ಣ *zDb = db->aDb[i].zDbSName;
          sqlite3_Error_With_ಸಂದೇಶ(db, rc, "database schema is locked: %s", zDb);
          testcase( db->flags & SQLITE_ReadUncommit );
          ಗಚ್ಛ end_prepare;
        }
      }
    }
  }

#ifndef SQLITE_OMIT_VIRTUALTABLE
  ಯದಿ( db->pDisconnect ) sqlite3VtabUnlockList(db);
#endif

  ಯದಿ( nBytes>=0 && (nBytes==0 || zSql[nBytes-1]!=0) ){
    ವರ್ಣ *zSqlCopy;
    ಪೂರ್ಣಾಙ್ಕ mxLen = db->aLimit[SQLITE_LIMIT_SQL_LENGTH];
    testcase( nBytes==mxLen );
    testcase( nBytes==mxLen+1 );
    ಯದಿ( nBytes>mxLen ){
      sqlite3_Error_With_ಸಂದೇಶ(db, SQLITE_TOOBIG, "statement too long");
      rc = sqlite3ApiExit(db, SQLITE_TOOBIG);
      ಗಚ್ಛ end_prepare;
    }
    zSqlCopy = sqlite3DbStrNDup(db, zSql, nBytes);
    ಯದಿ( zSqlCopy ){
      sqlite3RunParser(&sParse, zSqlCopy);
      sParse.zTail = &zSql[sParse.zTail-zSqlCopy];
      sqlite3DbFree(db, zSqlCopy);
    }ಅನ್ಯಥಾ{
      sParse.zTail = &zSql[nBytes];
    }
  }ಅನ್ಯಥಾ{
    sqlite3RunParser(&sParse, zSql);
  }
  ಪ್ರತಿಜ್ಞಾ( 0==sParse.nQueryLoop );

  ಯದಿ( pzTail ){
    *pzTail = sParse.zTail;
  }

  ಯದಿ( db->init.busy==0 ){
    sqlite3VdbeSetSql(sParse.pVdbe, zSql, (ಪೂರ್ಣಾಙ್ಕ)(sParse.zTail-zSql), prepFlags);
  }
  ಯದಿ( db->mallocFailed ){
    sParse.rc = SQLITE_NOMEM_BKPT;
    sParse.checkSchema = 0;
  }
  ಯದಿ( sParse.rc!=SQLITE_OK && sParse.rc!=SQLITE_DONE ){
    ಯದಿ( sParse.checkSchema && db->init.busy==0 ){
      schemaIsValid(&sParse);
    }
    ಯದಿ( sParse.pVdbe ){
      sqlite3VdbeFinalize(sParse.pVdbe);
    }
    ಪ್ರತಿಜ್ಞಾ( 0==(*ppStmt) );
    rc = sParse.rc;
    ಯದಿ( sParse.z_Err_ಸಂದೇಶ ){
      sqlite3_Error_With_ಸಂದೇಶ(db, rc, "%s", sParse.z_Err_ಸಂದೇಶ);
      sqlite3DbFree(db, sParse.z_Err_ಸಂದೇಶ);
    }ಅನ್ಯಥಾ{
      sqlite3Error(db, rc);
    }
  }ಅನ್ಯಥಾ{
    ಪ್ರತಿಜ್ಞಾ( sParse.z_Err_ಸಂದೇಶ==0 );
    *ppStmt = (sqlite3_stmt*)sParse.pVdbe;
    rc = SQLITE_OK;
    sqlite3ErrorClear(db);
  }


  /* Delete any TriggerPrg structures allocated while parsing this statement. */
  ಯಾವತ್( sParse.pTriggerPrg ){
    TriggerPrg *pT = sParse.pTriggerPrg;
    sParse.pTriggerPrg = pT->pNext;
    sqlite3DbFree(db, pT);
  }

end_prepare:

  sqlite3ParseObjectReset(&sParse);
  ಪ್ರತ್ಯಾಗಮ rc;
}
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ sqlite3LockAndPrepare(
  sqlite3 *db,              /* Database handle. */
  ಧ್ರುವ ವರ್ಣ *zSql,         /* UTF-8 encoded SQL statement. */
  ಪೂರ್ಣಾಙ್ಕ nBytes,               /* Length of zSql in bytes. */
  u32 prepFlags,            /* Zero or more SQLITE_PREPARE_* flags */
  Vdbe *pOld,               /* VM being reprepared */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  ಧ್ರುವ ವರ್ಣ **pzTail       /* OUT: End of parsed string */
){
  ಪೂರ್ಣಾಙ್ಕ rc;
  ಪೂರ್ಣಾಙ್ಕ cnt = 0;

#ifdef SQLITE_ENABLE_API_ARMOR
  ಯದಿ( ppStmt==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_MISUSE_BKPT;
#endif
  *ppStmt = 0;
  ಯದಿ( !sqlite3SafetyCheckOk(db)||zSql==0 ){
    ಪ್ರತ್ಯಾಗಮ SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(db->mutex);
  sqlite3BtreeEnterAll(db);
  ಕುರು{
    /* Make multiple attempts to compile the SQL, until it either succeeds
    ** or encounters a permanent error.  A schema problem after one schema
    ** reset is considered a permanent error. */
    rc = sqlite3Prepare(db, zSql, nBytes, prepFlags, pOld, ppStmt, pzTail);
    ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_OK || *ppStmt==0 );
    ಯದಿ( rc==SQLITE_OK || db->mallocFailed ) ಭಙ್ಗ;
    cnt++;
  }ಯಾವತ್( (rc==SQLITE_ERROR_RETRY && ALWAYS(cnt<=SQLITE_MAX_PREPARE_RETRY))
       || (rc==SQLITE_SCHEMA && (sqlite3ResetOneSchema(db,-1), cnt)==1) );
  sqlite3BtreeLeaveAll(db);
  ಪ್ರತಿಜ್ಞಾ( rc!=SQLITE_ERROR_RETRY );
  rc = sqlite3ApiExit(db, rc);
  ಪ್ರತಿಜ್ಞಾ( (rc&db->errMask)==rc );
  db->busyHandler.nBusy = 0;
  sqlite3_mutex_leave(db->mutex);
  ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_OK || (*ppStmt)==0 );
  ಪ್ರತ್ಯಾಗಮ rc;
}


/*
** Rerun the compilation of a statement after a schema change.
**
** If the statement is successfully recompiled, return SQLITE_OK. Otherwise,
** if the statement cannot be recompiled because another connection has
** locked the sqlite3_schema table, return SQLITE_LOCKED. If any other error
** occurs, return SQLITE_SCHEMA.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3Reprepare(Vdbe *p){
  ಪೂರ್ಣಾಙ್ಕ rc;
  sqlite3_stmt *pNew;
  ಧ್ರುವ ವರ್ಣ *zSql;
  sqlite3 *db;
  u8 prepFlags;

  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(sqlite3VdbeDb(p)->mutex) );
  zSql = sqlite3_sql((sqlite3_stmt *)p);
  ಪ್ರತಿಜ್ಞಾ( zSql!=0 );  /* Reprepare only called for prepare_v2() statements */
  db = sqlite3VdbeDb(p);
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(db->mutex) );
  prepFlags = sqlite3VdbePrepareFlags(p);
  rc = sqlite3LockAndPrepare(db, zSql, -1, prepFlags, p, &pNew, 0);
  ಯದಿ( rc ){
    ಯದಿ( rc==SQLITE_NOMEM ){
      sqlite3OomFault(db);
    }
    ಪ್ರತಿಜ್ಞಾ( pNew==0 );
    ಪ್ರತ್ಯಾಗಮ rc;
  }ಅನ್ಯಥಾ{
    ಪ್ರತಿಜ್ಞಾ( pNew!=0 );
  }
  sqlite3VdbeSwap((Vdbe*)pNew, p);
  sqlite3TransferBindings(pNew, (sqlite3_stmt*)p);
  sqlite3VdbeResetStepResult((Vdbe*)pNew);
  sqlite3VdbeFinalize((Vdbe*)pNew);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}


/*
** Two versions of the official API.  Legacy and new use.  In the legacy
** version, the original SQL text is not saved in the prepared statement
** and so if a schema change occurs, SQLITE_SCHEMA is returned by
** sqlite3_step().  In the new version, the original SQL text is retained
** and the statement is automatically recompiled if an schema change
** occurs.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_prepare(
  sqlite3 *db,              /* Database handle. */
  ಧ್ರುವ ವರ್ಣ *zSql,         /* UTF-8 encoded SQL statement. */
  ಪೂರ್ಣಾಙ್ಕ nBytes,               /* Length of zSql in bytes. */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  ಧ್ರುವ ವರ್ಣ **pzTail       /* OUT: End of parsed string */
){
  ಪೂರ್ಣಾಙ್ಕ rc;
  rc = sqlite3LockAndPrepare(db,zSql,nBytes,0,0,ppStmt,pzTail);
  ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_OK || ppStmt==0 || *ppStmt==0 );  /* VERIFY: F13021 */
  ಪ್ರತ್ಯಾಗಮ rc;
}
ಪೂರ್ಣಾಙ್ಕ sqlite3_prepare_v2(
  sqlite3 *db,              /* Database handle. */
  ಧ್ರುವ ವರ್ಣ *zSql,         /* UTF-8 encoded SQL statement. */
  ಪೂರ್ಣಾಙ್ಕ nBytes,               /* Length of zSql in bytes. */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  ಧ್ರುವ ವರ್ಣ **pzTail       /* OUT: End of parsed string */
){
  ಪೂರ್ಣಾಙ್ಕ rc;
  /* EVIDENCE-OF: R-37923-12173 The sqlite3_prepare_v2() interface works
  ** exactly the same as sqlite3_prepare_v3() with a zero prepFlags
  ** parameter.
  **
  ** Proof in that the 5th parameter to sqlite3LockAndPrepare is 0 */
  rc = sqlite3LockAndPrepare(db,zSql,nBytes,SQLITE_PREPARE_SAVESQL,0,
                             ppStmt,pzTail);
  ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_OK || ppStmt==0 || *ppStmt==0 );
  ಪ್ರತ್ಯಾಗಮ rc;
}
ಪೂರ್ಣಾಙ್ಕ sqlite3_prepare_v3(
  sqlite3 *db,              /* Database handle. */
  ಧ್ರುವ ವರ್ಣ *zSql,         /* UTF-8 encoded SQL statement. */
  ಪೂರ್ಣಾಙ್ಕ nBytes,               /* Length of zSql in bytes. */
  unsigned ಪೂರ್ಣಾಙ್ಕ prepFlags,   /* Zero or more SQLITE_PREPARE_* flags */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  ಧ್ರುವ ವರ್ಣ **pzTail       /* OUT: End of parsed string */
){
  ಪೂರ್ಣಾಙ್ಕ rc;
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
  ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_OK || ppStmt==0 || *ppStmt==0 );
  ಪ್ರತ್ಯಾಗಮ rc;
}


#ifndef SQLITE_OMIT_UTF16
/*
** Compile the UTF-16 encoded SQL statement zSql into a statement handle.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ sqlite3Prepare16(
  sqlite3 *db,              /* Database handle. */ 
  ಧ್ರುವ ಶೂನ್ಯ *zSql,         /* UTF-16 encoded SQL statement. */
  ಪೂರ್ಣಾಙ್ಕ nBytes,               /* Length of zSql in bytes. */
  u32 prepFlags,            /* Zero or more SQLITE_PREPARE_* flags */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  ಧ್ರುವ ಶೂನ್ಯ **pzTail       /* OUT: End of parsed string */
){
  /* This function currently works by first transforming the UTF-16
  ** encoded string to UTF-8, then invoking sqlite3_prepare(). The
  ** tricky bit is figuring out the pointer to return in *pzTail.
  */
  ವರ್ಣ *zSql8;
  ಧ್ರುವ ವರ್ಣ *zTail8 = 0;
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;

#ifdef SQLITE_ENABLE_API_ARMOR
  ಯದಿ( ppStmt==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_MISUSE_BKPT;
#endif
  *ppStmt = 0;
  ಯದಿ( !sqlite3SafetyCheckOk(db)||zSql==0 ){
    ಪ್ರತ್ಯಾಗಮ SQLITE_MISUSE_BKPT;
  }

  /* Make sure nBytes is non-negative and correct.  It should be the
  ** number of bytes until the end of the input buffer or until the first
  ** U+0000 character.  If the input nBytes is odd, convert it into
  ** an even number.  If the input nBytes is negative, then the input
  ** must be terminated by at least one U+0000 character */
  ಯದಿ( nBytes>=0 ){
    ಪೂರ್ಣಾಙ್ಕ sz;
    ಧ್ರುವ ವರ್ಣ *z = (ಧ್ರುವ ವರ್ಣ*)zSql;
    ಪ್ರತಿ(sz=0; sz<nBytes && (z[sz]!=0 || z[sz+1]!=0); sz += 2){}
    nBytes = sz;
  }ಅನ್ಯಥಾ{
    ಪೂರ್ಣಾಙ್ಕ sz;
    ಧ್ರುವ ವರ್ಣ *z = (ಧ್ರುವ ವರ್ಣ*)zSql;
    ಪ್ರತಿ(sz=0; z[sz]!=0 || z[sz+1]!=0; sz += 2){}
    nBytes = sz;
  }

  sqlite3_mutex_enter(db->mutex);
  zSql8 = sqlite3Utf16to8(db, zSql, nBytes, SQLITE_UTF16NATIVE);
  ಯದಿ( zSql8 ){
    rc = sqlite3LockAndPrepare(db, zSql8, -1, prepFlags, 0, ppStmt, &zTail8);
  }

  ಯದಿ( zTail8 && pzTail ){
    /* If sqlite3_prepare returns a tail pointer, we calculate the
    ** equivalent pointer into the UTF-16 string by counting the unicode
    ** characters between zSql8 and zTail8, and then returning a pointer
    ** the same number of characters into the UTF-16 string.
    */
    ಪೂರ್ಣಾಙ್ಕ chars_parsed = sqlite3Utf8CharLen(zSql8, (ಪೂರ್ಣಾಙ್ಕ)(zTail8-zSql8));
    *pzTail = (u8 *)zSql + sqlite3Utf16ByteLen(zSql, nBytes, chars_parsed);
  }
  sqlite3DbFree(db, zSql8); 
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Two versions of the official API.  Legacy and new use.  In the legacy
** version, the original SQL text is not saved in the prepared statement
** and so if a schema change occurs, SQLITE_SCHEMA is returned by
** sqlite3_step().  In the new version, the original SQL text is retained
** and the statement is automatically recompiled if an schema change
** occurs.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_prepare16(
  sqlite3 *db,              /* Database handle. */ 
  ಧ್ರುವ ಶೂನ್ಯ *zSql,         /* UTF-16 encoded SQL statement. */
  ಪೂರ್ಣಾಙ್ಕ nBytes,               /* Length of zSql in bytes. */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  ಧ್ರುವ ಶೂನ್ಯ **pzTail       /* OUT: End of parsed string */
){
  ಪೂರ್ಣಾಙ್ಕ rc;
  rc = sqlite3Prepare16(db,zSql,nBytes&~1,0,ppStmt,pzTail);
  ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_OK || ppStmt==0 || *ppStmt==0 );  /* VERIFY: F13021 */
  ಪ್ರತ್ಯಾಗಮ rc;
}
ಪೂರ್ಣಾಙ್ಕ sqlite3_prepare16_v2(
  sqlite3 *db,              /* Database handle. */ 
  ಧ್ರುವ ಶೂನ್ಯ *zSql,         /* UTF-16 encoded SQL statement. */
  ಪೂರ್ಣಾಙ್ಕ nBytes,               /* Length of zSql in bytes. */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  ಧ್ರುವ ಶೂನ್ಯ **pzTail       /* OUT: End of parsed string */
){
  ಪೂರ್ಣಾಙ್ಕ rc;
  rc = sqlite3Prepare16(db,zSql,nBytes&~1,SQLITE_PREPARE_SAVESQL,ppStmt,pzTail);
  ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_OK || ppStmt==0 || *ppStmt==0 );  /* VERIFY: F13021 */
  ಪ್ರತ್ಯಾಗಮ rc;
}
ಪೂರ್ಣಾಙ್ಕ sqlite3_prepare16_v3(
  sqlite3 *db,              /* Database handle. */ 
  ಧ್ರುವ ಶೂನ್ಯ *zSql,         /* UTF-16 encoded SQL statement. */
  ಪೂರ್ಣಾಙ್ಕ nBytes,               /* Length of zSql in bytes. */
  unsigned ಪೂರ್ಣಾಙ್ಕ prepFlags,   /* Zero or more SQLITE_PREPARE_* flags */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  ಧ್ರುವ ಶೂನ್ಯ **pzTail       /* OUT: End of parsed string */
){
  ಪೂರ್ಣಾಙ್ಕ rc;
  rc = sqlite3Prepare16(db,zSql,nBytes&~1,
         SQLITE_PREPARE_SAVESQL|(prepFlags&SQLITE_PREPARE_MASK),
         ppStmt,pzTail);
  ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_OK || ppStmt==0 || *ppStmt==0 );  /* VERIFY: F13021 */
  ಪ್ರತ್ಯಾಗಮ rc;
}

#endif /* SQLITE_OMIT_UTF16 */
