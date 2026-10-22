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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"

/*
** Fill the InitData structure with an error message that indicates
** that the database is corrupt.
*/
ਸਥਿਰ ਖਾਲੀ corruptSchema(
  Init_ਅੰਕੜੇ *p_ਅੰਕੜੇ,     /* Initialization context */
  ਅੱਖਰ **azObj,        /* Type and name of object being parsed */
  ਅਚਲ ਅੱਖਰ *zExtra   /* Error information */
){
  sqlite3 *db = p_ਅੰਕੜੇ->db;
  ਜੇ( db->mallocFailed ){
    p_ਅੰਕੜੇ->rc = SQLITE_NOMEM_BKPT;
  }ਨਹੀਂਤਾਂ ਜੇ( p_ਅੰਕੜੇ->pz_Err_ਸੁਨੇਹਾ[0]!=0 ){
    /* A error message has already been generated.  Do not overwrite it */
  }ਨਹੀਂਤਾਂ ਜੇ( p_ਅੰਕੜੇ->mInitFlags & (INITFLAG_Alter_ਨਕਾਬ) ){
    ਸਥਿਰ ਅਚਲ ਅੱਖਰ *azAlterType[] = {
       "rename",
       "drop column",
       "add column",
       "drop constraint"
    };
    *p_ਅੰਕੜੇ->pz_Err_ਸੁਨੇਹਾ = sqlite3MPrintf(db, 
        "error in %s %s after %s: %s", azObj[0], azObj[1], 
        azAlterType[(p_ਅੰਕੜੇ->mInitFlags&INITFLAG_Alter_ਨਕਾਬ)-1], 
        zExtra
    );
    p_ਅੰਕੜੇ->rc = SQLITE_ERROR;
  }ਨਹੀਂਤਾਂ ਜੇ( db->flags & SQLITE_WriteSchema ){
    p_ਅੰਕੜੇ->rc = SQLITE_CORRUPT_BKPT;
  }ਨਹੀਂਤਾਂ{
    ਅੱਖਰ *z;
    ਅਚਲ ਅੱਖਰ *zObj = azObj[1] ? azObj[1] : "?";
    z = sqlite3MPrintf(db, "malformed database schema (%s)", zObj);
    ਜੇ( zExtra && zExtra[0] ) z = sqlite3MPrintf(db, "%z - %s", z, zExtra);
    *p_ਅੰਕੜੇ->pz_Err_ਸੁਨੇਹਾ = z;
    p_ਅੰਕੜੇ->rc = SQLITE_CORRUPT_BKPT;
  }
}

/*
** Check to see if any sibling index (another index on the same table)
** of pIndex has the same root page number, and if it does, return true.
** This would indicate a corrupt schema.
*/
ਪੂਰਨ sqlite3_ਸੂਚਕਾਂਕ_Has_Duplicate_ਜੜ੍ਹ_ਪੰਨਾ(ਸੂਚਕਾਂਕ *p_ਸੂਚਕਾਂਕ){
  ਸੂਚਕਾਂਕ *p;
  ਲਈ(p=p_ਸੂਚਕਾਂਕ->p_ਸਾਰਣੀ->p_ਸੂਚਕਾਂਕ; p; p=p->p_ਅਗਲਾ){
    ਜੇ( p->tnum==p_ਸੂਚਕਾਂਕ->tnum && p!=p_ਸੂਚਕਾਂਕ ) ਵਾਪਸ 1;
  }
  ਵਾਪਸ 0;
}

/* forward declaration */
ਸਥਿਰ ਪੂਰਨ sqlite3Prepare(
  sqlite3 *db,              /* Database handle. */
  ਅਚਲ ਅੱਖਰ *zSql,         /* UTF-8 encoded SQL statement. */
  ਪੂਰਨ n_ਬਾਈਟਾਂ,               /* Length of zSql in bytes. */
  u32 prepFlags,            /* Zero or more SQLITE_PREPARE_* flags */
  Vdbe *pReprepare,         /* VM being reprepared */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  ਅਚਲ ਅੱਖਰ **pz_ਪੂਛ       /* OUT: End of parsed string */
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
ਪੂਰਨ sqlite3InitCallback(ਖਾਲੀ *pInit, ਪੂਰਨ argc, ਅੱਖਰ **argv, ਅੱਖਰ **Not_ਵਰਤਿਆ){
  Init_ਅੰਕੜੇ *p_ਅੰਕੜੇ = (Init_ਅੰਕੜੇ*)pInit;
  sqlite3 *db = p_ਅੰਕੜੇ->db;
  ਪੂਰਨ iDb = p_ਅੰਕੜੇ->iDb;

  ਪ੍ਰਤਿਜ੍ਞਾ( argc==5 );
  ਅਣਵਰਤਿਆ_PARAMETER2(Not_ਵਰਤਿਆ, argc);
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(db->mutex) );
  db->mDbFlags |= DBFLAG_EncodingFixed;
  ਜੇ( argv==0 ) ਵਾਪਸ 0;   /* Might happen if EMPTY_RESULT_CALLBACKS are on */
  p_ਅੰਕੜੇ->n_Init_ਪੰਕਤੀ++;
  ਜੇ( db->mallocFailed ){
    corruptSchema(p_ਅੰਕੜੇ, argv, 0);
    ਵਾਪਸ 1;
  }

  ਪ੍ਰਤਿਜ੍ਞਾ( iDb>=0 && iDb<db->nDb );
  ਪ੍ਰਤਿਜ੍ਞਾ( db->aDb[iDb].pSchema!=0 );
  ਜੇ( argv[3]==0 ){
    corruptSchema(p_ਅੰਕੜੇ, argv, 0);
  }ਨਹੀਂਤਾਂ ਜੇ( argv[4]
         && 'c'==sqlite3_ਉੱਤਲਾ_ਮੰਜ਼ਿਲ_ਹੇਠਲਾ[(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)argv[4][0]]
         && 'r'==sqlite3_ਉੱਤਲਾ_ਮੰਜ਼ਿਲ_ਹੇਠਲਾ[(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)argv[4][1]] ){
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
    ਪੂਰਨ rc;
    u8 saved_iDb = db->init.iDb;
    sqlite3_stmt *pStmt;
    ਅਚਲ ਅੱਖਰ *z_ਸਿਰਾ;
    TESTONLY(ਪੂਰਨ rcp);            /* Return code from sqlite3_prepare() */

    ਪ੍ਰਤਿਜ੍ਞਾ( db->init.busy );
    db->init.iDb = iDb;
    ਜੇ( sqlite3GetUInt32(argv[3], &db->init.newTnum)==0
     || (db->init.newTnum>p_ਅੰਕੜੇ->mx_ਪੰਨਾ && p_ਅੰਕੜੇ->mx_ਪੰਨਾ>0)
    ){
      ਜੇ( sqlite3Config.bExtraSchemaChecks ){
        corruptSchema(p_ਅੰਕੜੇ, argv, "invalid rootpage");
      }
    }
    db->init.orphanTrigger = 0;
    db->init.azInit = (ਅਚਲ ਅੱਖਰ**)argv;
    pStmt = 0;
    TESTONLY(rcp = ) sqlite3Prepare(db, argv[4], -1, 0, 0, &pStmt, &z_ਸਿਰਾ);
    rc = db->err_ਕੋਡ;
    ਪ੍ਰਤਿਜ੍ਞਾ( (rc&0xFF)==(rcp&0xFF) );
    db->init.iDb = saved_iDb;
    /* assert( saved_iDb==0 || (db->mDbFlags & DBFLAG_Vacuum)!=0 ); */
    ਜੇ( SQLITE_OK!=rc ){
      ਜੇ( db->init.orphanTrigger ){
        ਪ੍ਰਤਿਜ੍ਞਾ( iDb==1 );
      }ਨਹੀਂਤਾਂ{
        ਜੇ( rc > p_ਅੰਕੜੇ->rc ) p_ਅੰਕੜੇ->rc = rc;
        ਜੇ( rc==SQLITE_NOMEM ){
          sqlite3OomFault(db);
        }ਨਹੀਂਤਾਂ ਜੇ( rc!=SQLITE_ਰੁਕਾਵਟ && (rc&0xFF)!=SQLITE_LOCKED ){
          corruptSchema(p_ਅੰਕੜੇ, argv, sqlite3_errmsg(db));
        }
      }
    }ਨਹੀਂਤਾਂ ਜੇ( z_ਸਿਰਾ[0] ){
      corruptSchema(p_ਅੰਕੜੇ, argv, 0);
    }
    db->init.azInit = sqlite3StdType; /* Any array of string ptrs will do */
    sqlite3_finalize(pStmt);
  }ਨਹੀਂਤਾਂ ਜੇ( argv[1]==0 || (argv[4]!=0 && argv[4][0]!=0) ){
    corruptSchema(p_ਅੰਕੜੇ, argv, 0);
  }ਨਹੀਂਤਾਂ{
    /* If the SQL column is blank it means this is an index that
    ** was created to be the PRIMARY KEY or to fulfill a UNIQUE
    ** constraint for a CREATE TABLE.  The index should have already
    ** been created when we processed the CREATE TABLE.  All we have
    ** to do here is record the root page number for that index.
    */
    ਸੂਚਕਾਂਕ *p_ਸੂਚਕਾਂਕ;
    p_ਸੂਚਕਾਂਕ = sqlite3_ਲੱਭੋ_ਸੂਚਕਾਂਕ(db, argv[1], db->aDb[iDb].zDbSName);
    ਜੇ( p_ਸੂਚਕਾਂਕ==0 ){
      corruptSchema(p_ਅੰਕੜੇ, argv, "orphan index");
    }ਨਹੀਂਤਾਂ
    ਜੇ( sqlite3GetUInt32(argv[3],&p_ਸੂਚਕਾਂਕ->tnum)==0
     || p_ਸੂਚਕਾਂਕ->tnum<2
     || p_ਸੂਚਕਾਂਕ->tnum>p_ਅੰਕੜੇ->mx_ਪੰਨਾ
     || sqlite3_ਸੂਚਕਾਂਕ_Has_Duplicate_ਜੜ੍ਹ_ਪੰਨਾ(p_ਸੂਚਕਾਂਕ)
    ){
      ਜੇ( sqlite3Config.bExtraSchemaChecks ){
        corruptSchema(p_ਅੰਕੜੇ, argv, "invalid rootpage");
      }
    }
  }
  ਜੇ( p_ਅੰਕੜੇ->pz_Err_ਸੁਨੇਹਾ[0]==0 ){
    Schema *pX = db->aDb[iDb].pSchema;
    ਜੇ( pX->tblHash.ਗਿਣਤੀ_2 + pX->idxHash.ਗਿਣਤੀ_2 + pX->trigHash.ਗਿਣਤੀ_2
                   > (u32)db->a_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_SCHEMA]
    ){
      *p_ਅੰਕੜੇ->pz_Err_ਸੁਨੇਹਾ = sqlite3MPrintf(db, "too many schema objects");
      p_ਅੰਕੜੇ->rc = SQLITE_ERROR;
      ਵਾਪਸ 1;
    }
  }
  ਵਾਪਸ 0;
}

/*
** Attempt to read the database schema and initialize internal
** data structures for a single database file.  The index of the
** database file is given by iDb.  iDb==0 is used for the main
** database.  iDb==1 should never be used.  iDb>=2 is used for
** auxiliary databases.  Return one of the SQLITE_ error codes to
** indicate success or failure.
*/
ਪੂਰਨ sqlite3InitOne(sqlite3 *db, ਪੂਰਨ iDb, ਅੱਖਰ **pz_Err_ਸੁਨੇਹਾ, u32 mFlags){
  ਪੂਰਨ rc;
  ਪੂਰਨ i;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_DEPRECATED
  ਪੂਰਨ ਮਾਪ;
#ਜੇ_ਅੰਤ
  Db *pDb;
  ਅੱਖਰ ਅਚਲ *azArg[6];
  ਪੂਰਨ meta[5];
  Init_ਅੰਕੜੇ init_ਅੰਕੜੇ;
  ਅਚਲ ਅੱਖਰ *zSchemaTabName;
  ਪੂਰਨ openedTransaction = 0;
  ਪੂਰਨ ਨਕਾਬ = ((db->mDbFlags & DBFLAG_EncodingFixed) | ~DBFLAG_EncodingFixed);

  ਪ੍ਰਤਿਜ੍ਞਾ( (db->mDbFlags & DBFLAG_Schema_ਗਿਆਤ_Ok)==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( iDb>=0 && iDb<db->nDb );
  ਪ੍ਰਤਿਜ੍ਞਾ( db->aDb[iDb].pSchema );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(db->mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( iDb==1 || sqlite3BtreeHoldsMutex(db->aDb[iDb].pBt) );

  db->init.busy = 1 + ((mFlags & INITFLAG_Alter_ਜੋੜੋ)!=0);
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
  azArg[1] = zSchemaTabName = SCHEMA_ਸਾਰਣੀ(iDb);
  azArg[2] = azArg[1];
  azArg[3] = "1";
  azArg[4] = "CREATE TABLE x(type text,name text,tbl_name text,"
                            "rootpage int,sql text)";
  azArg[5] = 0;
  init_ਅੰਕੜੇ.db = db;
  init_ਅੰਕੜੇ.iDb = iDb;
  init_ਅੰਕੜੇ.rc = SQLITE_OK;
  init_ਅੰਕੜੇ.pz_Err_ਸੁਨੇਹਾ = pz_Err_ਸੁਨੇਹਾ;
  init_ਅੰਕੜੇ.mInitFlags = mFlags;
  init_ਅੰਕੜੇ.n_Init_ਪੰਕਤੀ = 0;
  init_ਅੰਕੜੇ.mx_ਪੰਨਾ = 0;
  sqlite3InitCallback(&init_ਅੰਕੜੇ, 5, (ਅੱਖਰ **)azArg, 0);
  db->mDbFlags &= ਨਕਾਬ;
  ਜੇ( init_ਅੰਕੜੇ.rc ){
    rc = init_ਅੰਕੜੇ.rc;
    ਜਾਓ error_ਬਾਹਰਲਾ;
  }

  /* Create a cursor to hold the database open
  */
  pDb = &db->aDb[iDb];
  ਜੇ( pDb->pBt==0 ){
    ਪ੍ਰਤਿਜ੍ਞਾ( iDb==1 );
    Db_ਸੈੱਟ_Property(db, 1, DB_SchemaLoaded);
    rc = SQLITE_OK;
    ਜਾਓ error_ਬਾਹਰਲਾ;
  }

  /* If there is not already a read-only (or read-write) transaction opened
  ** on the b-tree database, open one now. If a transaction is opened, it 
  ** will be closed before this function returns.  */
  sqlite3BtreeEnter(pDb->pBt);
  ਜੇ( sqlite3BtreeTxnState(pDb->pBt)==SQLITE_TXN_NONE ){
    rc = sqlite3BtreeBeginTrans(pDb->pBt, 0, 0);
    ਜੇ( rc!=SQLITE_OK ){
      sqlite3_ਸੈੱਟ_String(pz_Err_ਸੁਨੇਹਾ, db, sqlite3ErrStr(rc));
      ਜਾਓ initone_error_ਬਾਹਰਲਾ;
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
  ਲਈ(i=0; i<Array_ਮਾਪ(meta); i++){
    sqlite3BtreeGetMeta(pDb->pBt, i+1, (u32 *)&meta[i]);
  }
  ਜੇ( (db->flags & SQLITE_ResetDatabase)!=0 ){
    memset(meta, 0, ਆਕਾਰ(meta));
  }
  pDb->pSchema->schema_cookie = meta[BTREE_SCHEMA_ਸੰਸਕਰਣ-1];

  /* If opening a non-empty database, check the text encoding. For the
  ** main database, set sqlite3.enc to the encoding of the main database.
  ** For an attached db, it is an error if the encoding is not the same
  ** as sqlite3.enc.
  */
  ਜੇ( meta[BTREE_ਪਾਠ_ENCODING-1] ){  /* text encoding */
    ਜੇ( iDb==0 && (db->mDbFlags & DBFLAG_EncodingFixed)==0 ){
      u8 encoding;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_UTF16
      /* If opening the main database, set ENC(db). */
      encoding = (u8)meta[BTREE_ਪਾਠ_ENCODING-1] & 3;
      ਜੇ( encoding==0 ) encoding = SQLITE_UTF8;
#ਨਹੀਂਤਾਂ
      encoding = SQLITE_UTF8;
#ਜੇ_ਅੰਤ
      sqlite3_ਸੈੱਟ_ਪਾਠ_Encoding(db, encoding);
    }ਨਹੀਂਤਾਂ{
      /* If opening an attached database, the encoding much match ENC(db) */
      ਜੇ( (meta[BTREE_ਪਾਠ_ENCODING-1] & 3)!=ENC(db) ){
        sqlite3_ਸੈੱਟ_String(pz_Err_ਸੁਨੇਹਾ, db, "attached databases must use the same"
            " text encoding as main database");
        rc = SQLITE_ERROR;
        ਜਾਓ initone_error_ਬਾਹਰਲਾ;
      }
    }
  }
  pDb->pSchema->enc = ENC(db);

  ਜੇ( pDb->pSchema->cache_ਮਾਪ==0 ){
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_DEPRECATED
    ਮਾਪ = sqlite3AbsInt32(meta[BTREE_DEFAULT_CACHE_ਮਾਪ-1]);
    ਜੇ( ਮਾਪ==0 ){ ਮਾਪ = SQLITE_DEFAULT_CACHE_ਮਾਪ; }
    pDb->pSchema->cache_ਮਾਪ = ਮਾਪ;
#ਨਹੀਂਤਾਂ
    pDb->pSchema->cache_ਮਾਪ = SQLITE_DEFAULT_CACHE_ਮਾਪ;
#ਜੇ_ਅੰਤ
    sqlite3_Btree_ਸੈੱਟ_Cache_ਮਾਪ(pDb->pBt, pDb->pSchema->cache_ਮਾਪ);
  }

  /*
  ** file_format==1    Version 3.0.0.
  ** file_format==2    Version 3.1.3.  // ALTER TABLE ADD COLUMN
  ** file_format==3    Version 3.1.4.  // ditto but with non-NULL defaults
  ** file_format==4    Version 3.3.0.  // DESC indices.  Boolean constants
  */
  pDb->pSchema->file_format = (u8)meta[BTREE_FILE_FORMAT-1];
  ਜੇ( pDb->pSchema->file_format==0 ){
    pDb->pSchema->file_format = 1;
  }
  ਜੇ( pDb->pSchema->file_format>SQLITE_MAX_FILE_FORMAT ){
    sqlite3_ਸੈੱਟ_String(pz_Err_ਸੁਨੇਹਾ, db, "unsupported file format");
    rc = SQLITE_ERROR;
    ਜਾਓ initone_error_ਬਾਹਰਲਾ;
  }

  /* Ticket #2804:  When we open a database in the newer file format,
  ** clear the legacy_file_format pragma flag so that a VACUUM will
  ** not downgrade the database and thus invalidate any descending
  ** indices that the user might have created.
  */
  ਜੇ( iDb==0 && meta[BTREE_FILE_FORMAT-1]>=4 ){
    db->flags &= ~(u64)SQLITE_LegacyFileFmt;
  }

  /* Read the schema information out of the schema tables
  */
  ਪ੍ਰਤਿਜ੍ਞਾ( db->init.busy );
  init_ਅੰਕੜੇ.mx_ਪੰਨਾ = sqlite3_Btree_ਆਖਰੀ_ਪੰਨਾ(pDb->pBt);
  {
    ਅੱਖਰ *zSql;
    zSql = sqlite3MPrintf(db, 
        "SELECT*FROM\"%w\".%s ORDER BY rowid",
        db->aDb[iDb].zDbSName, zSchemaTabName);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTHORIZATION
    {
      sqlite3_xauth xAuth;
      xAuth = db->xAuth;
      db->xAuth = 0;
#ਜੇ_ਅੰਤ
      rc = sqlite3_exec(db, zSql, sqlite3InitCallback, &init_ਅੰਕੜੇ, 0);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTHORIZATION
      db->xAuth = xAuth;
    }
#ਜੇ_ਅੰਤ
    ਜੇ( rc==SQLITE_OK ) rc = init_ਅੰਕੜੇ.rc;
    sqlite3DbFree(db, zSql);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_ANALYZE
    ਜੇ( rc==SQLITE_OK ){
      sqlite3_Analysis_ਲੱਦੋ(db, iDb);
    }
#ਜੇ_ਅੰਤ
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( pDb == &(db->aDb[iDb]) );
  ਜੇ( db->mallocFailed ){
    rc = SQLITE_NOMEM_BKPT;
    sqlite3_Reset_All_Schemas_ਦਾ_Connection(db);
    pDb = &db->aDb[iDb];
  }ਨਹੀਂਤਾਂ
  ਜੇ( rc==SQLITE_OK || ((db->flags&SQLITE_NoSchemaError) && rc!=SQLITE_NOMEM)){
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
    Db_ਸੈੱਟ_Property(db, iDb, DB_SchemaLoaded);
    rc = SQLITE_OK;
  }

  /* Jump here for an error that occurs after successfully allocating
  ** curMain and calling sqlite3BtreeEnter(). For an error that occurs
  ** before that point, jump to error_out.
  */
initone_error_ਬਾਹਰਲਾ:
  ਜੇ( openedTransaction ){
    sqlite3BtreeCommit(pDb->pBt);
  }
  sqlite3BtreeLeave(pDb->pBt);

error_ਬਾਹਰਲਾ:
  ਜੇ( rc ){
    ਜੇ( rc==SQLITE_NOMEM || rc==SQLITE_IOERR_NOMEM ){
      sqlite3OomFault(db);
    }
    sqlite3ResetOneSchema(db, iDb);
  }
  db->init.busy = 0;
  ਵਾਪਸ rc;
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
ਪੂਰਨ sqlite3Init(sqlite3 *db, ਅੱਖਰ **pz_Err_ਸੁਨੇਹਾ){
  ਪੂਰਨ i, rc;
  ਪੂਰਨ commit_ਅੰਦਰੂਨੀ = !(db->mDbFlags&DBFLAG_SchemaChange);
  
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(db->mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3BtreeHoldsMutex(db->aDb[0].pBt) );
  ਪ੍ਰਤਿਜ੍ਞਾ( db->init.busy==0 );
  ENC(db) = SCHEMA_ENC(db);
  ਪ੍ਰਤਿਜ੍ਞਾ( db->nDb>0 );
  /* Do the main schema first */
  ਜੇ( !DbHasProperty(db, 0, DB_SchemaLoaded) ){
    rc = sqlite3InitOne(db, 0, pz_Err_ਸੁਨੇਹਾ, 0);
    ਜੇ( rc ) ਵਾਪਸ rc;
  }
  /* All other schemas after the main schema. The "temp" schema must be last */
  ਲਈ(i=db->nDb-1; i>0; i--){
    ਪ੍ਰਤਿਜ੍ਞਾ( i==1 || sqlite3BtreeHoldsMutex(db->aDb[i].pBt) );
    ਜੇ( !DbHasProperty(db, i, DB_SchemaLoaded) ){
      rc = sqlite3InitOne(db, i, pz_Err_ਸੁਨੇਹਾ, 0);
      ਜੇ( rc ) ਵਾਪਸ rc;
    }
  }
  ਜੇ( commit_ਅੰਦਰੂਨੀ ){
    sqlite3_Commit_ਅੰਦਰੂਨੀ_Changes(db);
  }
  ਵਾਪਸ SQLITE_OK;
}

/*
** This routine is a no-op if the database schema is already initialized.
** Otherwise, the schema is loaded. An error code is returned.
*/
ਪੂਰਨ sqlite3ReadSchema(Parse *pParse){
  ਪੂਰਨ rc = SQLITE_OK;
  sqlite3 *db = pParse->db;
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(db->mutex) );
  ਜੇ( !db->init.busy ){
    rc = sqlite3Init(db, &pParse->z_Err_ਸੁਨੇਹਾ);
    ਜੇ( rc!=SQLITE_OK ){
      pParse->rc = rc;
      pParse->nErr++;
    }ਨਹੀਂਤਾਂ ਜੇ( db->noSharedCache ){
      db->mDbFlags |= DBFLAG_Schema_ਗਿਆਤ_Ok;
    }
  }
  ਵਾਪਸ rc;
}


/*
** Check schema cookies in all databases.  If any cookie is out
** of date set pParse->rc to SQLITE_SCHEMA.  If all schema cookies
** make no changes to pParse->rc.
*/
ਸਥਿਰ ਖਾਲੀ schemaIsValid(Parse *pParse){
  sqlite3 *db = pParse->db;
  ਪੂਰਨ iDb;
  ਪੂਰਨ rc;
  ਪੂਰਨ cookie;

  ਪ੍ਰਤਿਜ੍ਞਾ( pParse->checkSchema );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(db->mutex) );
  ਲਈ(iDb=0; iDb<db->nDb; iDb++){
    ਪੂਰਨ openedTransaction = 0;         /* True if a transaction is opened */
    Btree *pBt = db->aDb[iDb].pBt;     /* Btree database to read cookie from */
    ਜੇ( pBt==0 ) ਜਾਰੀ;

    /* If there is not already a read-only (or read-write) transaction opened
    ** on the b-tree database, open one now. If a transaction is opened, it 
    ** will be closed immediately after reading the meta-value. */
    ਜੇ( sqlite3BtreeTxnState(pBt)==SQLITE_TXN_NONE ){
      rc = sqlite3BtreeBeginTrans(pBt, 0, 0);
      ਜੇ( rc==SQLITE_NOMEM || rc==SQLITE_IOERR_NOMEM ){
        sqlite3OomFault(db);
        pParse->rc = SQLITE_NOMEM;
      }
      ਜੇ( rc!=SQLITE_OK ) ਵਾਪਸ;
      openedTransaction = 1;
    }

    /* Read the schema cookie from the database. If it does not match the 
    ** value stored as part of the in-memory schema representation,
    ** set Parse.rc to SQLITE_SCHEMA. */
    sqlite3BtreeGetMeta(pBt, BTREE_SCHEMA_ਸੰਸਕਰਣ, (u32 *)&cookie);
    ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3SchemaMutexHeld(db, iDb, 0) );
    ਜੇ( cookie!=db->aDb[iDb].pSchema->schema_cookie ){
      ਜੇ( DbHasProperty(db, iDb, DB_SchemaLoaded) ) pParse->rc = SQLITE_SCHEMA;
      sqlite3ResetOneSchema(db, iDb);
    }

    /* Close the transaction, if one was opened. */
    ਜੇ( openedTransaction ){
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
ਪੂਰਨ sqlite3_Schema_ਮੰਜ਼ਿਲ_ਸੂਚਕਾਂਕ(sqlite3 *db, Schema *pSchema){
  ਪੂਰਨ i = -32768;

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
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(db->mutex) );
  ਜੇ( pSchema ){
    ਲਈ(i=0; 1; i++){
      ਪ੍ਰਤਿਜ੍ਞਾ( i<db->nDb );
      ਜੇ( db->aDb[i].pSchema==pSchema ){
        ਰੋਕੋ;
      }
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( i>=0 && i<db->nDb );
  }
  ਵਾਪਸ i;
}

/*
** Free all memory allocations in the pParse object
*/
ਖਾਲੀ sqlite3ParseObjectReset(Parse *pParse){
  sqlite3 *db = pParse->db;
  ਪ੍ਰਤਿਜ੍ਞਾ( db!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( db->pParse==pParse );
  ਪ੍ਰਤਿਜ੍ਞਾ( pParse->nested==0 );
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_SHARED_CACHE
  ਜੇ( pParse->n_ਸਾਰਣੀ_ਤਾਲਾ ) sqlite3DbNNFreeNN(db, pParse->a_ਸਾਰਣੀ_ਤਾਲਾ);
#ਜੇ_ਅੰਤ
  ਜਦੋਂ( pParse->pCleanup ){
    ParseCleanup *pCleanup = pParse->pCleanup;
    pParse->pCleanup = pCleanup->p_ਅਗਲਾ;
    pCleanup->xCleanup(db, pCleanup->pPtr);
    sqlite3DbNNFreeNN(db, pCleanup);
  }
  ਜੇ( pParse->aLabel ) sqlite3DbNNFreeNN(db, pParse->aLabel);
  ਜੇ( pParse->p_Const_ਵਿਅੰਜਕ ){
    sqlite3_ਵਿਅੰਜਕ_List_Delete(db, pParse->p_Const_ਵਿਅੰਜਕ);
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( db->lookaside.bDisable >= pParse->disableLookaside );
  db->lookaside.bDisable -= pParse->disableLookaside;
  db->lookaside.sz = db->lookaside.bDisable ? 0 : db->lookaside.szTrue;
  ਪ੍ਰਤਿਜ੍ਞਾ( pParse->db->pParse==pParse );
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
ਖਾਲੀ *sqlite3_Parser_ਜੋੜੋ_Cleanup(
  Parse *pParse,                      /* Destroy when this Parser finishes */
  ਖਾਲੀ (*xCleanup)(sqlite3*,ਖਾਲੀ*),   /* The cleanup routine */
  ਖਾਲੀ *pPtr                          /* Pointer to object to be cleaned up */
){
  ParseCleanup *pCleanup;
  ਜੇ( sqlite3FaultSim(300) ){
    pCleanup = 0;
    sqlite3OomFault(pParse->db);
  }ਨਹੀਂਤਾਂ{
    pCleanup = sqlite3DbMallocRaw(pParse->db, ਆਕਾਰ(*pCleanup));
  }
  ਜੇ( pCleanup ){
    pCleanup->p_ਅਗਲਾ = pParse->pCleanup;
    pParse->pCleanup = pCleanup;
    pCleanup->pPtr = pPtr;
    pCleanup->xCleanup = xCleanup;
  }ਨਹੀਂਤਾਂ{
    xCleanup(pParse->db, pPtr);
    pPtr = 0;
#ਜੇ defined(SQLITE_DEBUG) || defined(SQLITE_COVERAGE_TEST)
    pParse->earlyCleanup = 1;
#ਜੇ_ਅੰਤ
  }
  ਵਾਪਸ pPtr;
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
ਖਾਲੀ sqlite3ParseObjectInit(Parse *pParse, sqlite3 *db){
  ਪ੍ਰਤਿਜ੍ਞਾ( db!=0 );
  memset(PARSE_HDR(pParse), 0, PARSE_HDR_SZ);
  memset(PARSE_ਪੂਛ(pParse), 0, PARSE_ਪੂਛ_SZ);
  ਪ੍ਰਤਿਜ੍ਞਾ( db->pParse!=pParse );
  pParse->pOuterParse = db->pParse;
  db->pParse = pParse;
  pParse->db = db;
  ਜੇ( db->mallocFailed ) sqlite3_Error_ਸੁਨੇਹਾ(pParse, "out of memory");
}

/*
** Maximum number of times that we will try again to prepare a statement
** that returns SQLITE_ERROR_RETRY.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_MAX_PREPARE_RETRY
# define SQLITE_MAX_PREPARE_RETRY 25
#ਜੇ_ਅੰਤ

/*
** Compile the UTF-8 encoded SQL statement zSql into a statement handle.
*/
ਸਥਿਰ ਪੂਰਨ sqlite3Prepare(
  sqlite3 *db,              /* Database handle. */
  ਅਚਲ ਅੱਖਰ *zSql,         /* UTF-8 encoded SQL statement. */
  ਪੂਰਨ n_ਬਾਈਟਾਂ,               /* Length of zSql in bytes. */
  u32 prepFlags,            /* Zero or more SQLITE_PREPARE_* flags */
  Vdbe *pReprepare,         /* VM being reprepared */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  ਅਚਲ ਅੱਖਰ **pz_ਪੂਛ       /* OUT: End of parsed string */
){
  ਪੂਰਨ rc = SQLITE_OK;       /* Result code */
  ਪੂਰਨ i;                    /* Loop counter */
  Parse sParse;             /* Parsing context */

  /* sqlite3ParseObjectInit(&sParse, db); // inlined for performance */
  memset(PARSE_HDR(&sParse), 0, PARSE_HDR_SZ);
  memset(PARSE_ਪੂਛ(&sParse), 0, PARSE_ਪੂਛ_SZ);
  sParse.pOuterParse = db->pParse;
  db->pParse = &sParse;
  sParse.db = db;
  ਜੇ( pReprepare ){
    sParse.pReprepare = pReprepare;
    sParse.explain = sqlite3_stmt_isexplain((sqlite3_stmt*)pReprepare);
  }ਨਹੀਂਤਾਂ{
    ਪ੍ਰਤਿਜ੍ਞਾ( sParse.pReprepare==0 );
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( ppStmt && *ppStmt==0 );
  ਜੇ( db->mallocFailed ){
    sqlite3_Error_ਸੁਨੇਹਾ(&sParse, "out of memory");
    db->err_ਕੋਡ = rc = SQLITE_NOMEM;
    ਜਾਓ ਸਿਰਾ_prepare;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(db->mutex) );

  /* For a long-term use prepared statement avoid the use of
  ** lookaside memory.
  */
  ਜੇ( prepFlags & SQLITE_PREPARE_PERSISTENT ){
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
  ਜੇ( !db->noSharedCache ){
    ਲਈ(i=0; i<db->nDb; i++) {
      Btree *pBt = db->aDb[i].pBt;
      ਜੇ( pBt ){
        ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3BtreeHoldsMutex(pBt) );
        rc = sqlite3BtreeSchemaLocked(pBt);
        ਜੇ( rc ){
          ਅਚਲ ਅੱਖਰ *zDb = db->aDb[i].zDbSName;
          sqlite3_Error_With_ਸੁਨੇਹਾ(db, rc, "database schema is locked: %s", zDb);
          testcase( db->flags & SQLITE_ReadUncommit );
          ਜਾਓ ਸਿਰਾ_prepare;
        }
      }
    }
  }

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
  ਜੇ( db->pDisconnect ) sqlite3VtabUnlockList(db);
#ਜੇ_ਅੰਤ

  ਜੇ( n_ਬਾਈਟਾਂ>=0 && (n_ਬਾਈਟਾਂ==0 || zSql[n_ਬਾਈਟਾਂ-1]!=0) ){
    ਅੱਖਰ *zSqlCopy;
    ਪੂਰਨ mxLen = db->a_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_SQL_ਲੰਮਾਈ];
    testcase( n_ਬਾਈਟਾਂ==mxLen );
    testcase( n_ਬਾਈਟਾਂ==mxLen+1 );
    ਜੇ( n_ਬਾਈਟਾਂ>mxLen ){
      sqlite3_Error_With_ਸੁਨੇਹਾ(db, SQLITE_TOOBIG, "statement too long");
      rc = sqlite3ApiExit(db, SQLITE_TOOBIG);
      ਜਾਓ ਸਿਰਾ_prepare;
    }
    zSqlCopy = sqlite3DbStrNDup(db, zSql, n_ਬਾਈਟਾਂ);
    ਜੇ( zSqlCopy ){
      sqlite3_ਲੜੀਵਾਰ_Parser(&sParse, zSqlCopy);
      sParse.z_ਪੂਛ = &zSql[sParse.z_ਪੂਛ-zSqlCopy];
      sqlite3DbFree(db, zSqlCopy);
    }ਨਹੀਂਤਾਂ{
      sParse.z_ਪੂਛ = &zSql[n_ਬਾਈਟਾਂ];
    }
  }ਨਹੀਂਤਾਂ{
    sqlite3_ਲੜੀਵਾਰ_Parser(&sParse, zSql);
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( 0==sParse.nQueryLoop );

  ਜੇ( pz_ਪੂਛ ){
    *pz_ਪੂਛ = sParse.z_ਪੂਛ;
  }

  ਜੇ( db->init.busy==0 ){
    sqlite3_Vdbe_ਸੈੱਟ_Sql(sParse.pVdbe, zSql, (ਪੂਰਨ)(sParse.z_ਪੂਛ-zSql), prepFlags);
  }
  ਜੇ( db->mallocFailed ){
    sParse.rc = SQLITE_NOMEM_BKPT;
    sParse.checkSchema = 0;
  }
  ਜੇ( sParse.rc!=SQLITE_OK && sParse.rc!=SQLITE_ਪੂਰਾ ){
    ਜੇ( sParse.checkSchema && db->init.busy==0 ){
      schemaIsValid(&sParse);
    }
    ਜੇ( sParse.pVdbe ){
      sqlite3VdbeFinalize(sParse.pVdbe);
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( 0==(*ppStmt) );
    rc = sParse.rc;
    ਜੇ( sParse.z_Err_ਸੁਨੇਹਾ ){
      sqlite3_Error_With_ਸੁਨੇਹਾ(db, rc, "%s", sParse.z_Err_ਸੁਨੇਹਾ);
      sqlite3DbFree(db, sParse.z_Err_ਸੁਨੇਹਾ);
    }ਨਹੀਂਤਾਂ{
      sqlite3Error(db, rc);
    }
  }ਨਹੀਂਤਾਂ{
    ਪ੍ਰਤਿਜ੍ਞਾ( sParse.z_Err_ਸੁਨੇਹਾ==0 );
    *ppStmt = (sqlite3_stmt*)sParse.pVdbe;
    rc = SQLITE_OK;
    sqlite3ErrorClear(db);
  }


  /* Delete any TriggerPrg structures allocated while parsing this statement. */
  ਜਦੋਂ( sParse.pTriggerPrg ){
    TriggerPrg *pT = sParse.pTriggerPrg;
    sParse.pTriggerPrg = pT->p_ਅਗਲਾ;
    sqlite3DbFree(db, pT);
  }

ਸਿਰਾ_prepare:

  sqlite3ParseObjectReset(&sParse);
  ਵਾਪਸ rc;
}
ਸਥਿਰ ਪੂਰਨ sqlite3_ਤਾਲਾ_And_Prepare(
  sqlite3 *db,              /* Database handle. */
  ਅਚਲ ਅੱਖਰ *zSql,         /* UTF-8 encoded SQL statement. */
  ਪੂਰਨ n_ਬਾਈਟਾਂ,               /* Length of zSql in bytes. */
  u32 prepFlags,            /* Zero or more SQLITE_PREPARE_* flags */
  Vdbe *pOld,               /* VM being reprepared */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  ਅਚਲ ਅੱਖਰ **pz_ਪੂਛ       /* OUT: End of parsed string */
){
  ਪੂਰਨ rc;
  ਪੂਰਨ cnt = 0;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( ppStmt==0 ) ਵਾਪਸ SQLITE_MISUSE_BKPT;
#ਜੇ_ਅੰਤ
  *ppStmt = 0;
  ਜੇ( !sqlite3SafetyCheckOk(db)||zSql==0 ){
    ਵਾਪਸ SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(db->mutex);
  sqlite3BtreeEnterAll(db);
  ਕਰੋ{
    /* Make multiple attempts to compile the SQL, until it either succeeds
    ** or encounters a permanent error.  A schema problem after one schema
    ** reset is considered a permanent error. */
    rc = sqlite3Prepare(db, zSql, n_ਬਾਈਟਾਂ, prepFlags, pOld, ppStmt, pz_ਪੂਛ);
    ਪ੍ਰਤਿਜ੍ਞਾ( rc==SQLITE_OK || *ppStmt==0 );
    ਜੇ( rc==SQLITE_OK || db->mallocFailed ) ਰੋਕੋ;
    cnt++;
  }ਜਦੋਂ( (rc==SQLITE_ERROR_RETRY && ALWAYS(cnt<=SQLITE_MAX_PREPARE_RETRY))
       || (rc==SQLITE_SCHEMA && (sqlite3ResetOneSchema(db,-1), cnt)==1) );
  sqlite3BtreeLeaveAll(db);
  ਪ੍ਰਤਿਜ੍ਞਾ( rc!=SQLITE_ERROR_RETRY );
  rc = sqlite3ApiExit(db, rc);
  ਪ੍ਰਤਿਜ੍ਞਾ( (rc&db->err_ਨਕਾਬ)==rc );
  db->busyHandler.nBusy = 0;
  sqlite3_mutex_leave(db->mutex);
  ਪ੍ਰਤਿਜ੍ਞਾ( rc==SQLITE_OK || (*ppStmt)==0 );
  ਵਾਪਸ rc;
}


/*
** Rerun the compilation of a statement after a schema change.
**
** If the statement is successfully recompiled, return SQLITE_OK. Otherwise,
** if the statement cannot be recompiled because another connection has
** locked the sqlite3_schema table, return SQLITE_LOCKED. If any other error
** occurs, return SQLITE_SCHEMA.
*/
ਪੂਰਨ sqlite3Reprepare(Vdbe *p){
  ਪੂਰਨ rc;
  sqlite3_stmt *pNew;
  ਅਚਲ ਅੱਖਰ *zSql;
  sqlite3 *db;
  u8 prepFlags;

  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(sqlite3VdbeDb(p)->mutex) );
  zSql = sqlite3_sql((sqlite3_stmt *)p);
  ਪ੍ਰਤਿਜ੍ਞਾ( zSql!=0 );  /* Reprepare only called for prepare_v2() statements */
  db = sqlite3VdbeDb(p);
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(db->mutex) );
  prepFlags = sqlite3VdbePrepareFlags(p);
  rc = sqlite3_ਤਾਲਾ_And_Prepare(db, zSql, -1, prepFlags, p, &pNew, 0);
  ਜੇ( rc ){
    ਜੇ( rc==SQLITE_NOMEM ){
      sqlite3OomFault(db);
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( pNew==0 );
    ਵਾਪਸ rc;
  }ਨਹੀਂਤਾਂ{
    ਪ੍ਰਤਿਜ੍ਞਾ( pNew!=0 );
  }
  sqlite3_Vdbe_ਅਦਲਾ_ਬਦਲੀ((Vdbe*)pNew, p);
  sqlite3TransferBindings(pNew, (sqlite3_stmt*)p);
  sqlite3_Vdbe_Reset_ਕਦਮ_ਨਤੀਜਾ((Vdbe*)pNew);
  sqlite3VdbeFinalize((Vdbe*)pNew);
  ਵਾਪਸ SQLITE_OK;
}


/*
** Two versions of the official API.  Legacy and new use.  In the legacy
** version, the original SQL text is not saved in the prepared statement
** and so if a schema change occurs, SQLITE_SCHEMA is returned by
** sqlite3_step().  In the new version, the original SQL text is retained
** and the statement is automatically recompiled if an schema change
** occurs.
*/
ਪੂਰਨ sqlite3_prepare(
  sqlite3 *db,              /* Database handle. */
  ਅਚਲ ਅੱਖਰ *zSql,         /* UTF-8 encoded SQL statement. */
  ਪੂਰਨ n_ਬਾਈਟਾਂ,               /* Length of zSql in bytes. */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  ਅਚਲ ਅੱਖਰ **pz_ਪੂਛ       /* OUT: End of parsed string */
){
  ਪੂਰਨ rc;
  rc = sqlite3_ਤਾਲਾ_And_Prepare(db,zSql,n_ਬਾਈਟਾਂ,0,0,ppStmt,pz_ਪੂਛ);
  ਪ੍ਰਤਿਜ੍ਞਾ( rc==SQLITE_OK || ppStmt==0 || *ppStmt==0 );  /* VERIFY: F13021 */
  ਵਾਪਸ rc;
}
ਪੂਰਨ sqlite3_prepare_v2(
  sqlite3 *db,              /* Database handle. */
  ਅਚਲ ਅੱਖਰ *zSql,         /* UTF-8 encoded SQL statement. */
  ਪੂਰਨ n_ਬਾਈਟਾਂ,               /* Length of zSql in bytes. */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  ਅਚਲ ਅੱਖਰ **pz_ਪੂਛ       /* OUT: End of parsed string */
){
  ਪੂਰਨ rc;
  /* EVIDENCE-OF: R-37923-12173 The sqlite3_prepare_v2() interface works
  ** exactly the same as sqlite3_prepare_v3() with a zero prepFlags
  ** parameter.
  **
  ** Proof in that the 5th parameter to sqlite3LockAndPrepare is 0 */
  rc = sqlite3_ਤਾਲਾ_And_Prepare(db,zSql,n_ਬਾਈਟਾਂ,SQLITE_PREPARE_SAVESQL,0,
                             ppStmt,pz_ਪੂਛ);
  ਪ੍ਰਤਿਜ੍ਞਾ( rc==SQLITE_OK || ppStmt==0 || *ppStmt==0 );
  ਵਾਪਸ rc;
}
ਪੂਰਨ sqlite3_prepare_v3(
  sqlite3 *db,              /* Database handle. */
  ਅਚਲ ਅੱਖਰ *zSql,         /* UTF-8 encoded SQL statement. */
  ਪੂਰਨ n_ਬਾਈਟਾਂ,               /* Length of zSql in bytes. */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ prepFlags,   /* Zero or more SQLITE_PREPARE_* flags */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  ਅਚਲ ਅੱਖਰ **pz_ਪੂਛ       /* OUT: End of parsed string */
){
  ਪੂਰਨ rc;
  /* EVIDENCE-OF: R-56861-42673 sqlite3_prepare_v3() differs from
  ** sqlite3_prepare_v2() only in having the extra prepFlags parameter,
  ** which is a bit array consisting of zero or more of the
  ** SQLITE_PREPARE_* flags.
  **
  ** Proof by comparison to the implementation of sqlite3_prepare_v2()
  ** directly above. */
  rc = sqlite3_ਤਾਲਾ_And_Prepare(db,zSql,n_ਬਾਈਟਾਂ,
                 SQLITE_PREPARE_SAVESQL|(prepFlags&SQLITE_PREPARE_ਨਕਾਬ),
                 0,ppStmt,pz_ਪੂਛ);
  ਪ੍ਰਤਿਜ੍ਞਾ( rc==SQLITE_OK || ppStmt==0 || *ppStmt==0 );
  ਵਾਪਸ rc;
}


#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_UTF16
/*
** Compile the UTF-16 encoded SQL statement zSql into a statement handle.
*/
ਸਥਿਰ ਪੂਰਨ sqlite3Prepare16(
  sqlite3 *db,              /* Database handle. */ 
  ਅਚਲ ਖਾਲੀ *zSql,         /* UTF-16 encoded SQL statement. */
  ਪੂਰਨ n_ਬਾਈਟਾਂ,               /* Length of zSql in bytes. */
  u32 prepFlags,            /* Zero or more SQLITE_PREPARE_* flags */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  ਅਚਲ ਖਾਲੀ **pz_ਪੂਛ       /* OUT: End of parsed string */
){
  /* This function currently works by first transforming the UTF-16
  ** encoded string to UTF-8, then invoking sqlite3_prepare(). The
  ** tricky bit is figuring out the pointer to return in *pzTail.
  */
  ਅੱਖਰ *zSql8;
  ਅਚਲ ਅੱਖਰ *zTail8 = 0;
  ਪੂਰਨ rc = SQLITE_OK;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( ppStmt==0 ) ਵਾਪਸ SQLITE_MISUSE_BKPT;
#ਜੇ_ਅੰਤ
  *ppStmt = 0;
  ਜੇ( !sqlite3SafetyCheckOk(db)||zSql==0 ){
    ਵਾਪਸ SQLITE_MISUSE_BKPT;
  }

  /* Make sure nBytes is non-negative and correct.  It should be the
  ** number of bytes until the end of the input buffer or until the first
  ** U+0000 character.  If the input nBytes is odd, convert it into
  ** an even number.  If the input nBytes is negative, then the input
  ** must be terminated by at least one U+0000 character */
  ਜੇ( n_ਬਾਈਟਾਂ>=0 ){
    ਪੂਰਨ sz;
    ਅਚਲ ਅੱਖਰ *z = (ਅਚਲ ਅੱਖਰ*)zSql;
    ਲਈ(sz=0; sz<n_ਬਾਈਟਾਂ && (z[sz]!=0 || z[sz+1]!=0); sz += 2){}
    n_ਬਾਈਟਾਂ = sz;
  }ਨਹੀਂਤਾਂ{
    ਪੂਰਨ sz;
    ਅਚਲ ਅੱਖਰ *z = (ਅਚਲ ਅੱਖਰ*)zSql;
    ਲਈ(sz=0; z[sz]!=0 || z[sz+1]!=0; sz += 2){}
    n_ਬਾਈਟਾਂ = sz;
  }

  sqlite3_mutex_enter(db->mutex);
  zSql8 = sqlite3Utf16to8(db, zSql, n_ਬਾਈਟਾਂ, SQLITE_UTF16NATIVE);
  ਜੇ( zSql8 ){
    rc = sqlite3_ਤਾਲਾ_And_Prepare(db, zSql8, -1, prepFlags, 0, ppStmt, &zTail8);
  }

  ਜੇ( zTail8 && pz_ਪੂਛ ){
    /* If sqlite3_prepare returns a tail pointer, we calculate the
    ** equivalent pointer into the UTF-16 string by counting the unicode
    ** characters between zSql8 and zTail8, and then returning a pointer
    ** the same number of characters into the UTF-16 string.
    */
    ਪੂਰਨ chars_parsed = sqlite3Utf8CharLen(zSql8, (ਪੂਰਨ)(zTail8-zSql8));
    *pz_ਪੂਛ = (u8 *)zSql + sqlite3Utf16ByteLen(zSql, n_ਬਾਈਟਾਂ, chars_parsed);
  }
  sqlite3DbFree(db, zSql8); 
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  ਵਾਪਸ rc;
}

/*
** Two versions of the official API.  Legacy and new use.  In the legacy
** version, the original SQL text is not saved in the prepared statement
** and so if a schema change occurs, SQLITE_SCHEMA is returned by
** sqlite3_step().  In the new version, the original SQL text is retained
** and the statement is automatically recompiled if an schema change
** occurs.
*/
ਪੂਰਨ sqlite3_prepare16(
  sqlite3 *db,              /* Database handle. */ 
  ਅਚਲ ਖਾਲੀ *zSql,         /* UTF-16 encoded SQL statement. */
  ਪੂਰਨ n_ਬਾਈਟਾਂ,               /* Length of zSql in bytes. */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  ਅਚਲ ਖਾਲੀ **pz_ਪੂਛ       /* OUT: End of parsed string */
){
  ਪੂਰਨ rc;
  rc = sqlite3Prepare16(db,zSql,n_ਬਾਈਟਾਂ&~1,0,ppStmt,pz_ਪੂਛ);
  ਪ੍ਰਤਿਜ੍ਞਾ( rc==SQLITE_OK || ppStmt==0 || *ppStmt==0 );  /* VERIFY: F13021 */
  ਵਾਪਸ rc;
}
ਪੂਰਨ sqlite3_prepare16_v2(
  sqlite3 *db,              /* Database handle. */ 
  ਅਚਲ ਖਾਲੀ *zSql,         /* UTF-16 encoded SQL statement. */
  ਪੂਰਨ n_ਬਾਈਟਾਂ,               /* Length of zSql in bytes. */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  ਅਚਲ ਖਾਲੀ **pz_ਪੂਛ       /* OUT: End of parsed string */
){
  ਪੂਰਨ rc;
  rc = sqlite3Prepare16(db,zSql,n_ਬਾਈਟਾਂ&~1,SQLITE_PREPARE_SAVESQL,ppStmt,pz_ਪੂਛ);
  ਪ੍ਰਤਿਜ੍ਞਾ( rc==SQLITE_OK || ppStmt==0 || *ppStmt==0 );  /* VERIFY: F13021 */
  ਵਾਪਸ rc;
}
ਪੂਰਨ sqlite3_prepare16_v3(
  sqlite3 *db,              /* Database handle. */ 
  ਅਚਲ ਖਾਲੀ *zSql,         /* UTF-16 encoded SQL statement. */
  ਪੂਰਨ n_ਬਾਈਟਾਂ,               /* Length of zSql in bytes. */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ prepFlags,   /* Zero or more SQLITE_PREPARE_* flags */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  ਅਚਲ ਖਾਲੀ **pz_ਪੂਛ       /* OUT: End of parsed string */
){
  ਪੂਰਨ rc;
  rc = sqlite3Prepare16(db,zSql,n_ਬਾਈਟਾਂ&~1,
         SQLITE_PREPARE_SAVESQL|(prepFlags&SQLITE_PREPARE_ਨਕਾਬ),
         ppStmt,pz_ਪੂਛ);
  ਪ੍ਰਤਿਜ੍ਞਾ( rc==SQLITE_OK || ppStmt==0 || *ppStmt==0 );  /* VERIFY: F13021 */
  ਵਾਪਸ rc;
}

#ਜੇ_ਅੰਤ /* SQLITE_OMIT_UTF16 */
