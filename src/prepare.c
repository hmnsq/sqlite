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
  Init_ದತ್ತಾಂಶ *p_ದತ್ತಾಂಶ,     /* Initialization context */
  ಅಕ್ಷರ **azObj,        /* Type and name of object being parsed */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zExtra   /* Error information */
){
  sqlite3 *db = p_ದತ್ತಾಂಶ->db;
  ಆದರೆ( db->mallocFailed ){
    p_ದತ್ತಾಂಶ->rc = SQLITE_NOMEM_BKPT;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p_ದತ್ತಾಂಶ->pz_Err_ಸಂದೇಶ[0]!=0 ){
    /* A error message has already been generated.  Do not overwrite it */
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p_ದತ್ತಾಂಶ->mInitFlags & (INITFLAG_Alter_ಮುಖವಾಡ) ){
    ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *azAlterType[] = {
       "rename",
       "drop column",
       "add column",
       "drop constraint"
    };
    *p_ದತ್ತಾಂಶ->pz_Err_ಸಂದೇಶ = sqlite3MPrintf(db, 
        "error in %s %s after %s: %s", azObj[0], azObj[1], 
        azAlterType[(p_ದತ್ತಾಂಶ->mInitFlags&INITFLAG_Alter_ಮುಖವಾಡ)-1], 
        zExtra
    );
    p_ದತ್ತಾಂಶ->rc = SQLITE_ERROR;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( db->flags & SQLITE_WriteSchema ){
    p_ದತ್ತಾಂಶ->rc = SQLITE_CORRUPT_BKPT;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಅಕ್ಷರ *z;
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zObj = azObj[1] ? azObj[1] : "?";
    z = sqlite3MPrintf(db, "malformed database schema (%s)", zObj);
    ಆದರೆ( zExtra && zExtra[0] ) z = sqlite3MPrintf(db, "%z - %s", z, zExtra);
    *p_ದತ್ತಾಂಶ->pz_Err_ಸಂದೇಶ = z;
    p_ದತ್ತಾಂಶ->rc = SQLITE_CORRUPT_BKPT;
  }
}

/*
** Check to see if any sibling index (another index on the same table)
** of pIndex has the same root page number, and if it does, return true.
** This would indicate a corrupt schema.
*/
ಪೂರ್ಣಾಂಕ sqlite3_ಸೂಚ್ಯಂಕ_Has_Duplicate_ಬೇರು_ಪುಟ(ಸೂಚ್ಯಂಕ *p_ಸೂಚ್ಯಂಕ){
  ಸೂಚ್ಯಂಕ *p;
  ಗಾಗಿ(p=p_ಸೂಚ್ಯಂಕ->p_ಕೋಷ್ಟಕ->p_ಸೂಚ್ಯಂಕ; p; p=p->p_ಮುಂದಿನ){
    ಆದರೆ( p->tnum==p_ಸೂಚ್ಯಂಕ->tnum && p!=p_ಸೂಚ್ಯಂಕ ) ಹಿಂತಿರುಗಿ 1;
  }
  ಹಿಂತಿರುಗಿ 0;
}

/* forward declaration */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ sqlite3Prepare(
  sqlite3 *db,              /* Database handle. */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSql,         /* UTF-8 encoded SQL statement. */
  ಪೂರ್ಣಾಂಕ n_ಬೈಟುಗಳು,               /* Length of zSql in bytes. */
  u32 prepFlags,            /* Zero or more SQLITE_PREPARE_* flags */
  Vdbe *pReprepare,         /* VM being reprepared */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **pz_ಬಾಲ       /* OUT: End of parsed string */
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
ಪೂರ್ಣಾಂಕ sqlite3InitCallback(ಶೂನ್ಯ *pInit, ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv, ಅಕ್ಷರ **Not_ಬಳಸಿದ){
  Init_ದತ್ತಾಂಶ *p_ದತ್ತಾಂಶ = (Init_ದತ್ತಾಂಶ*)pInit;
  sqlite3 *db = p_ದತ್ತಾಂಶ->db;
  ಪೂರ್ಣಾಂಕ iDb = p_ದತ್ತಾಂಶ->iDb;

  ಪ್ರತಿಜ್ಞಾ( argc==5 );
  ಬಳಸದ_PARAMETER2(Not_ಬಳಸಿದ, argc);
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(db->mutex) );
  db->mDbFlags |= DBFLAG_EncodingFixed;
  ಆದರೆ( argv==0 ) ಹಿಂತಿರುಗಿ 0;   /* Might happen if EMPTY_RESULT_CALLBACKS are on */
  p_ದತ್ತಾಂಶ->n_Init_ಅಡ್ಡಸಾಲು++;
  ಆದರೆ( db->mallocFailed ){
    corruptSchema(p_ದತ್ತಾಂಶ, argv, 0);
    ಹಿಂತಿರುಗಿ 1;
  }

  ಪ್ರತಿಜ್ಞಾ( iDb>=0 && iDb<db->nDb );
  ಪ್ರತಿಜ್ಞಾ( db->aDb[iDb].pSchema!=0 );
  ಆದರೆ( argv[3]==0 ){
    corruptSchema(p_ದತ್ತಾಂಶ, argv, 0);
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( argv[4]
         && 'c'==sqlite3_ಊರ್ಧ್ವ_ಗಮ್ಯ_ಅಧೋ[(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)argv[4][0]]
         && 'r'==sqlite3_ಊರ್ಧ್ವ_ಗಮ್ಯ_ಅಧೋ[(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)argv[4][1]] ){
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
    ಪೂರ್ಣಾಂಕ rc;
    u8 saved_iDb = db->init.iDb;
    sqlite3_stmt *pStmt;
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಕೊನೆ;
    TESTONLY(ಪೂರ್ಣಾಂಕ rcp);            /* Return code from sqlite3_prepare() */

    ಪ್ರತಿಜ್ಞಾ( db->init.busy );
    db->init.iDb = iDb;
    ಆದರೆ( sqlite3GetUInt32(argv[3], &db->init.newTnum)==0
     || (db->init.newTnum>p_ದತ್ತಾಂಶ->mx_ಪುಟ && p_ದತ್ತಾಂಶ->mx_ಪುಟ>0)
    ){
      ಆದರೆ( sqlite3Config.bExtraSchemaChecks ){
        corruptSchema(p_ದತ್ತಾಂಶ, argv, "invalid rootpage");
      }
    }
    db->init.orphanTrigger = 0;
    db->init.azInit = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ**)argv;
    pStmt = 0;
    TESTONLY(rcp = ) sqlite3Prepare(db, argv[4], -1, 0, 0, &pStmt, &z_ಕೊನೆ);
    rc = db->err_ಸಂಕೇತ;
    ಪ್ರತಿಜ್ಞಾ( (rc&0xFF)==(rcp&0xFF) );
    db->init.iDb = saved_iDb;
    /* assert( saved_iDb==0 || (db->mDbFlags & DBFLAG_Vacuum)!=0 ); */
    ಆದರೆ( SQLITE_OK!=rc ){
      ಆದರೆ( db->init.orphanTrigger ){
        ಪ್ರತಿಜ್ಞಾ( iDb==1 );
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಆದರೆ( rc > p_ದತ್ತಾಂಶ->rc ) p_ದತ್ತಾಂಶ->rc = rc;
        ಆದರೆ( rc==SQLITE_NOMEM ){
          sqlite3OomFault(db);
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( rc!=SQLITE_ಅಡಚಣೆ && (rc&0xFF)!=SQLITE_LOCKED ){
          corruptSchema(p_ದತ್ತಾಂಶ, argv, sqlite3_errmsg(db));
        }
      }
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( z_ಕೊನೆ[0] ){
      corruptSchema(p_ದತ್ತಾಂಶ, argv, 0);
    }
    db->init.azInit = sqlite3StdType; /* Any array of string ptrs will do */
    sqlite3_finalize(pStmt);
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( argv[1]==0 || (argv[4]!=0 && argv[4][0]!=0) ){
    corruptSchema(p_ದತ್ತಾಂಶ, argv, 0);
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* If the SQL column is blank it means this is an index that
    ** was created to be the PRIMARY KEY or to fulfill a UNIQUE
    ** constraint for a CREATE TABLE.  The index should have already
    ** been created when we processed the CREATE TABLE.  All we have
    ** to do here is record the root page number for that index.
    */
    ಸೂಚ್ಯಂಕ *p_ಸೂಚ್ಯಂಕ;
    p_ಸೂಚ್ಯಂಕ = sqlite3_ಕಂಡುಹಿಡಿ_ಸೂಚ್ಯಂಕ(db, argv[1], db->aDb[iDb].zDbSName);
    ಆದರೆ( p_ಸೂಚ್ಯಂಕ==0 ){
      corruptSchema(p_ದತ್ತಾಂಶ, argv, "orphan index");
    }ಇಲ್ಲದಿದ್ದರೆ
    ಆದರೆ( sqlite3GetUInt32(argv[3],&p_ಸೂಚ್ಯಂಕ->tnum)==0
     || p_ಸೂಚ್ಯಂಕ->tnum<2
     || p_ಸೂಚ್ಯಂಕ->tnum>p_ದತ್ತಾಂಶ->mx_ಪುಟ
     || sqlite3_ಸೂಚ್ಯಂಕ_Has_Duplicate_ಬೇರು_ಪುಟ(p_ಸೂಚ್ಯಂಕ)
    ){
      ಆದರೆ( sqlite3Config.bExtraSchemaChecks ){
        corruptSchema(p_ದತ್ತಾಂಶ, argv, "invalid rootpage");
      }
    }
  }
  ಆದರೆ( p_ದತ್ತಾಂಶ->pz_Err_ಸಂದೇಶ[0]==0 ){
    Schema *pX = db->aDb[iDb].pSchema;
    ಆದರೆ( pX->tblHash.ಎಣಿಕೆ_2 + pX->idxHash.ಎಣಿಕೆ_2 + pX->trigHash.ಎಣಿಕೆ_2
                   > (u32)db->a_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_SCHEMA]
    ){
      *p_ದತ್ತಾಂಶ->pz_Err_ಸಂದೇಶ = sqlite3MPrintf(db, "too many schema objects");
      p_ದತ್ತಾಂಶ->rc = SQLITE_ERROR;
      ಹಿಂತಿರುಗಿ 1;
    }
  }
  ಹಿಂತಿರುಗಿ 0;
}

/*
** Attempt to read the database schema and initialize internal
** data structures for a single database file.  The index of the
** database file is given by iDb.  iDb==0 is used for the main
** database.  iDb==1 should never be used.  iDb>=2 is used for
** auxiliary databases.  Return one of the SQLITE_ error codes to
** indicate success or failure.
*/
ಪೂರ್ಣಾಂಕ sqlite3InitOne(sqlite3 *db, ಪೂರ್ಣಾಂಕ iDb, ಅಕ್ಷರ **pz_Err_ಸಂದೇಶ, u32 mFlags){
  ಪೂರ್ಣಾಂಕ rc;
  ಪೂರ್ಣಾಂಕ i;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_DEPRECATED
  ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2;
#ಆದರೆ_ಅಂತ್ಯ
  Db *pDb;
  ಅಕ್ಷರ ಸ್ಥಿರಾಂಕ *azArg[6];
  ಪೂರ್ಣಾಂಕ meta[5];
  Init_ದತ್ತಾಂಶ init_ದತ್ತಾಂಶ;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSchemaTabName;
  ಪೂರ್ಣಾಂಕ openedTransaction = 0;
  ಪೂರ್ಣಾಂಕ ಮುಖವಾಡ = ((db->mDbFlags & DBFLAG_EncodingFixed) | ~DBFLAG_EncodingFixed);

  ಪ್ರತಿಜ್ಞಾ( (db->mDbFlags & DBFLAG_Schema_ತಿಳಿದ_Ok)==0 );
  ಪ್ರತಿಜ್ಞಾ( iDb>=0 && iDb<db->nDb );
  ಪ್ರತಿಜ್ಞಾ( db->aDb[iDb].pSchema );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(db->mutex) );
  ಪ್ರತಿಜ್ಞಾ( iDb==1 || sqlite3BtreeHoldsMutex(db->aDb[iDb].pBt) );

  db->init.busy = 1 + ((mFlags & INITFLAG_Alter_ಕೂಡಿಸು)!=0);
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
  azArg[1] = zSchemaTabName = SCHEMA_ಕೋಷ್ಟಕ(iDb);
  azArg[2] = azArg[1];
  azArg[3] = "1";
  azArg[4] = "CREATE TABLE x(type text,name text,tbl_name text,"
                            "rootpage int,sql text)";
  azArg[5] = 0;
  init_ದತ್ತಾಂಶ.db = db;
  init_ದತ್ತಾಂಶ.iDb = iDb;
  init_ದತ್ತಾಂಶ.rc = SQLITE_OK;
  init_ದತ್ತಾಂಶ.pz_Err_ಸಂದೇಶ = pz_Err_ಸಂದೇಶ;
  init_ದತ್ತಾಂಶ.mInitFlags = mFlags;
  init_ದತ್ತಾಂಶ.n_Init_ಅಡ್ಡಸಾಲು = 0;
  init_ದತ್ತಾಂಶ.mx_ಪುಟ = 0;
  sqlite3InitCallback(&init_ದತ್ತಾಂಶ, 5, (ಅಕ್ಷರ **)azArg, 0);
  db->mDbFlags &= ಮುಖವಾಡ;
  ಆದರೆ( init_ದತ್ತಾಂಶ.rc ){
    rc = init_ದತ್ತಾಂಶ.rc;
    ಹೋಗು error_ಹೊರಹೋಗು;
  }

  /* Create a cursor to hold the database open
  */
  pDb = &db->aDb[iDb];
  ಆದರೆ( pDb->pBt==0 ){
    ಪ್ರತಿಜ್ಞಾ( iDb==1 );
    Db_ಗಣ_Property(db, 1, DB_SchemaLoaded);
    rc = SQLITE_OK;
    ಹೋಗು error_ಹೊರಹೋಗು;
  }

  /* If there is not already a read-only (or read-write) transaction opened
  ** on the b-tree database, open one now. If a transaction is opened, it 
  ** will be closed before this function returns.  */
  sqlite3BtreeEnter(pDb->pBt);
  ಆದರೆ( sqlite3BtreeTxnState(pDb->pBt)==SQLITE_TXN_NONE ){
    rc = sqlite3BtreeBeginTrans(pDb->pBt, 0, 0);
    ಆದರೆ( rc!=SQLITE_OK ){
      sqlite3_ಗಣ_String(pz_Err_ಸಂದೇಶ, db, sqlite3ErrStr(rc));
      ಹೋಗು initone_error_ಹೊರಹೋಗು;
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
  ಗಾಗಿ(i=0; i<Array_ಗಾತ್ರ(meta); i++){
    sqlite3BtreeGetMeta(pDb->pBt, i+1, (u32 *)&meta[i]);
  }
  ಆದರೆ( (db->flags & SQLITE_ResetDatabase)!=0 ){
    memset(meta, 0, ಗಾತ್ರ(meta));
  }
  pDb->pSchema->schema_cookie = meta[BTREE_SCHEMA_ಆವೃತ್ತಿ-1];

  /* If opening a non-empty database, check the text encoding. For the
  ** main database, set sqlite3.enc to the encoding of the main database.
  ** For an attached db, it is an error if the encoding is not the same
  ** as sqlite3.enc.
  */
  ಆದರೆ( meta[BTREE_ಪಠ್ಯ_ENCODING-1] ){  /* text encoding */
    ಆದರೆ( iDb==0 && (db->mDbFlags & DBFLAG_EncodingFixed)==0 ){
      u8 encoding;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_UTF16
      /* If opening the main database, set ENC(db). */
      encoding = (u8)meta[BTREE_ಪಠ್ಯ_ENCODING-1] & 3;
      ಆದರೆ( encoding==0 ) encoding = SQLITE_UTF8;
#ಇಲ್ಲದಿದ್ದರೆ
      encoding = SQLITE_UTF8;
#ಆದರೆ_ಅಂತ್ಯ
      sqlite3_ಗಣ_ಪಠ್ಯ_Encoding(db, encoding);
    }ಇಲ್ಲದಿದ್ದರೆ{
      /* If opening an attached database, the encoding much match ENC(db) */
      ಆದರೆ( (meta[BTREE_ಪಠ್ಯ_ENCODING-1] & 3)!=ENC(db) ){
        sqlite3_ಗಣ_String(pz_Err_ಸಂದೇಶ, db, "attached databases must use the same"
            " text encoding as main database");
        rc = SQLITE_ERROR;
        ಹೋಗು initone_error_ಹೊರಹೋಗು;
      }
    }
  }
  pDb->pSchema->enc = ENC(db);

  ಆದರೆ( pDb->pSchema->cache_ಗಾತ್ರ==0 ){
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_DEPRECATED
    ಗಾತ್ರ_2 = sqlite3AbsInt32(meta[BTREE_DEFAULT_CACHE_ಗಾತ್ರ-1]);
    ಆದರೆ( ಗಾತ್ರ_2==0 ){ ಗಾತ್ರ_2 = SQLITE_DEFAULT_CACHE_ಗಾತ್ರ; }
    pDb->pSchema->cache_ಗಾತ್ರ = ಗಾತ್ರ_2;
#ಇಲ್ಲದಿದ್ದರೆ
    pDb->pSchema->cache_ಗಾತ್ರ = SQLITE_DEFAULT_CACHE_ಗಾತ್ರ;
#ಆದರೆ_ಅಂತ್ಯ
    sqlite3_Btree_ಗಣ_Cache_ಗಾತ್ರ(pDb->pBt, pDb->pSchema->cache_ಗಾತ್ರ);
  }

  /*
  ** file_format==1    Version 3.0.0.
  ** file_format==2    Version 3.1.3.  // ALTER TABLE ADD COLUMN
  ** file_format==3    Version 3.1.4.  // ditto but with non-NULL defaults
  ** file_format==4    Version 3.3.0.  // DESC indices.  Boolean constants
  */
  pDb->pSchema->file_format = (u8)meta[BTREE_FILE_FORMAT-1];
  ಆದರೆ( pDb->pSchema->file_format==0 ){
    pDb->pSchema->file_format = 1;
  }
  ಆದರೆ( pDb->pSchema->file_format>SQLITE_MAX_FILE_FORMAT ){
    sqlite3_ಗಣ_String(pz_Err_ಸಂದೇಶ, db, "unsupported file format");
    rc = SQLITE_ERROR;
    ಹೋಗು initone_error_ಹೊರಹೋಗು;
  }

  /* Ticket #2804:  When we open a database in the newer file format,
  ** clear the legacy_file_format pragma flag so that a VACUUM will
  ** not downgrade the database and thus invalidate any descending
  ** indices that the user might have created.
  */
  ಆದರೆ( iDb==0 && meta[BTREE_FILE_FORMAT-1]>=4 ){
    db->flags &= ~(u64)SQLITE_LegacyFileFmt;
  }

  /* Read the schema information out of the schema tables
  */
  ಪ್ರತಿಜ್ಞಾ( db->init.busy );
  init_ದತ್ತಾಂಶ.mx_ಪುಟ = sqlite3_Btree_ಕೊನೆಯ_ಪುಟ(pDb->pBt);
  {
    ಅಕ್ಷರ *zSql;
    zSql = sqlite3MPrintf(db, 
        "SELECT*FROM\"%w\".%s ORDER BY rowid",
        db->aDb[iDb].zDbSName, zSchemaTabName);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTHORIZATION
    {
      sqlite3_xauth xAuth;
      xAuth = db->xAuth;
      db->xAuth = 0;
#ಆದರೆ_ಅಂತ್ಯ
      rc = sqlite3_exec(db, zSql, sqlite3InitCallback, &init_ದತ್ತಾಂಶ, 0);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTHORIZATION
      db->xAuth = xAuth;
    }
#ಆದರೆ_ಅಂತ್ಯ
    ಆದರೆ( rc==SQLITE_OK ) rc = init_ದತ್ತಾಂಶ.rc;
    sqlite3DbFree(db, zSql);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_ANALYZE
    ಆದರೆ( rc==SQLITE_OK ){
      sqlite3_Analysis_ತುಂಬು(db, iDb);
    }
#ಆದರೆ_ಅಂತ್ಯ
  }
  ಪ್ರತಿಜ್ಞಾ( pDb == &(db->aDb[iDb]) );
  ಆದರೆ( db->mallocFailed ){
    rc = SQLITE_NOMEM_BKPT;
    sqlite3_Reset_All_Schemas_ದ_Connection(db);
    pDb = &db->aDb[iDb];
  }ಇಲ್ಲದಿದ್ದರೆ
  ಆದರೆ( rc==SQLITE_OK || ((db->flags&SQLITE_NoSchemaError) && rc!=SQLITE_NOMEM)){
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
    Db_ಗಣ_Property(db, iDb, DB_SchemaLoaded);
    rc = SQLITE_OK;
  }

  /* Jump here for an error that occurs after successfully allocating
  ** curMain and calling sqlite3BtreeEnter(). For an error that occurs
  ** before that point, jump to error_out.
  */
initone_error_ಹೊರಹೋಗು:
  ಆದರೆ( openedTransaction ){
    sqlite3BtreeCommit(pDb->pBt);
  }
  sqlite3BtreeLeave(pDb->pBt);

error_ಹೊರಹೋಗು:
  ಆದರೆ( rc ){
    ಆದರೆ( rc==SQLITE_NOMEM || rc==SQLITE_IOERR_NOMEM ){
      sqlite3OomFault(db);
    }
    sqlite3ResetOneSchema(db, iDb);
  }
  db->init.busy = 0;
  ಹಿಂತಿರುಗಿ rc;
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
ಪೂರ್ಣಾಂಕ sqlite3Init(sqlite3 *db, ಅಕ್ಷರ **pz_Err_ಸಂದೇಶ){
  ಪೂರ್ಣಾಂಕ i, rc;
  ಪೂರ್ಣಾಂಕ commit_ಆಂತರಿಕ = !(db->mDbFlags&DBFLAG_SchemaChange);
  
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(db->mutex) );
  ಪ್ರತಿಜ್ಞಾ( sqlite3BtreeHoldsMutex(db->aDb[0].pBt) );
  ಪ್ರತಿಜ್ಞಾ( db->init.busy==0 );
  ENC(db) = SCHEMA_ENC(db);
  ಪ್ರತಿಜ್ಞಾ( db->nDb>0 );
  /* Do the main schema first */
  ಆದರೆ( !DbHasProperty(db, 0, DB_SchemaLoaded) ){
    rc = sqlite3InitOne(db, 0, pz_Err_ಸಂದೇಶ, 0);
    ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ rc;
  }
  /* All other schemas after the main schema. The "temp" schema must be last */
  ಗಾಗಿ(i=db->nDb-1; i>0; i--){
    ಪ್ರತಿಜ್ಞಾ( i==1 || sqlite3BtreeHoldsMutex(db->aDb[i].pBt) );
    ಆದರೆ( !DbHasProperty(db, i, DB_SchemaLoaded) ){
      rc = sqlite3InitOne(db, i, pz_Err_ಸಂದೇಶ, 0);
      ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ rc;
    }
  }
  ಆದರೆ( commit_ಆಂತರಿಕ ){
    sqlite3_Commit_ಆಂತರಿಕ_Changes(db);
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** This routine is a no-op if the database schema is already initialized.
** Otherwise, the schema is loaded. An error code is returned.
*/
ಪೂರ್ಣಾಂಕ sqlite3ReadSchema(Parse *pParse){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  sqlite3 *db = pParse->db;
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(db->mutex) );
  ಆದರೆ( !db->init.busy ){
    rc = sqlite3Init(db, &pParse->z_Err_ಸಂದೇಶ);
    ಆದರೆ( rc!=SQLITE_OK ){
      pParse->rc = rc;
      pParse->nErr++;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( db->noSharedCache ){
      db->mDbFlags |= DBFLAG_Schema_ತಿಳಿದ_Ok;
    }
  }
  ಹಿಂತಿರುಗಿ rc;
}


/*
** Check schema cookies in all databases.  If any cookie is out
** of date set pParse->rc to SQLITE_SCHEMA.  If all schema cookies
** make no changes to pParse->rc.
*/
ಸ್ಥಿರ ಶೂನ್ಯ schemaIsValid(Parse *pParse){
  sqlite3 *db = pParse->db;
  ಪೂರ್ಣಾಂಕ iDb;
  ಪೂರ್ಣಾಂಕ rc;
  ಪೂರ್ಣಾಂಕ cookie;

  ಪ್ರತಿಜ್ಞಾ( pParse->checkSchema );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(db->mutex) );
  ಗಾಗಿ(iDb=0; iDb<db->nDb; iDb++){
    ಪೂರ್ಣಾಂಕ openedTransaction = 0;         /* True if a transaction is opened */
    Btree *pBt = db->aDb[iDb].pBt;     /* Btree database to read cookie from */
    ಆದರೆ( pBt==0 ) ಮುಂದುವರಿ;

    /* If there is not already a read-only (or read-write) transaction opened
    ** on the b-tree database, open one now. If a transaction is opened, it 
    ** will be closed immediately after reading the meta-value. */
    ಆದರೆ( sqlite3BtreeTxnState(pBt)==SQLITE_TXN_NONE ){
      rc = sqlite3BtreeBeginTrans(pBt, 0, 0);
      ಆದರೆ( rc==SQLITE_NOMEM || rc==SQLITE_IOERR_NOMEM ){
        sqlite3OomFault(db);
        pParse->rc = SQLITE_NOMEM;
      }
      ಆದರೆ( rc!=SQLITE_OK ) ಹಿಂತಿರುಗಿ;
      openedTransaction = 1;
    }

    /* Read the schema cookie from the database. If it does not match the 
    ** value stored as part of the in-memory schema representation,
    ** set Parse.rc to SQLITE_SCHEMA. */
    sqlite3BtreeGetMeta(pBt, BTREE_SCHEMA_ಆವೃತ್ತಿ, (u32 *)&cookie);
    ಪ್ರತಿಜ್ಞಾ( sqlite3SchemaMutexHeld(db, iDb, 0) );
    ಆದರೆ( cookie!=db->aDb[iDb].pSchema->schema_cookie ){
      ಆದರೆ( DbHasProperty(db, iDb, DB_SchemaLoaded) ) pParse->rc = SQLITE_SCHEMA;
      sqlite3ResetOneSchema(db, iDb);
    }

    /* Close the transaction, if one was opened. */
    ಆದರೆ( openedTransaction ){
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
ಪೂರ್ಣಾಂಕ sqlite3_Schema_ಗಮ್ಯ_ಸೂಚ್ಯಂಕ(sqlite3 *db, Schema *pSchema){
  ಪೂರ್ಣಾಂಕ i = -32768;

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
  ಆದರೆ( pSchema ){
    ಗಾಗಿ(i=0; 1; i++){
      ಪ್ರತಿಜ್ಞಾ( i<db->nDb );
      ಆದರೆ( db->aDb[i].pSchema==pSchema ){
        ನಿಲ್ಲಿಸು;
      }
    }
    ಪ್ರತಿಜ್ಞಾ( i>=0 && i<db->nDb );
  }
  ಹಿಂತಿರುಗಿ i;
}

/*
** Free all memory allocations in the pParse object
*/
ಶೂನ್ಯ sqlite3ParseObjectReset(Parse *pParse){
  sqlite3 *db = pParse->db;
  ಪ್ರತಿಜ್ಞಾ( db!=0 );
  ಪ್ರತಿಜ್ಞಾ( db->pParse==pParse );
  ಪ್ರತಿಜ್ಞಾ( pParse->nested==0 );
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_SHARED_CACHE
  ಆದರೆ( pParse->n_ಕೋಷ್ಟಕ_ಬೀಗ ) sqlite3DbNNFreeNN(db, pParse->a_ಕೋಷ್ಟಕ_ಬೀಗ);
#ಆದರೆ_ಅಂತ್ಯ
  ಆಗ( pParse->pCleanup ){
    ParseCleanup *pCleanup = pParse->pCleanup;
    pParse->pCleanup = pCleanup->p_ಮುಂದಿನ;
    pCleanup->xCleanup(db, pCleanup->pPtr);
    sqlite3DbNNFreeNN(db, pCleanup);
  }
  ಆದರೆ( pParse->aLabel ) sqlite3DbNNFreeNN(db, pParse->aLabel);
  ಆದರೆ( pParse->p_Const_ಅಭಿವ್ಯಕ್ತಿ ){
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
ಶೂನ್ಯ *sqlite3_Parser_ಕೂಡಿಸು_Cleanup(
  Parse *pParse,                      /* Destroy when this Parser finishes */
  ಶೂನ್ಯ (*xCleanup)(sqlite3*,ಶೂನ್ಯ*),   /* The cleanup routine */
  ಶೂನ್ಯ *pPtr                          /* Pointer to object to be cleaned up */
){
  ParseCleanup *pCleanup;
  ಆದರೆ( sqlite3FaultSim(300) ){
    pCleanup = 0;
    sqlite3OomFault(pParse->db);
  }ಇಲ್ಲದಿದ್ದರೆ{
    pCleanup = sqlite3DbMallocRaw(pParse->db, ಗಾತ್ರ(*pCleanup));
  }
  ಆದರೆ( pCleanup ){
    pCleanup->p_ಮುಂದಿನ = pParse->pCleanup;
    pParse->pCleanup = pCleanup;
    pCleanup->pPtr = pPtr;
    pCleanup->xCleanup = xCleanup;
  }ಇಲ್ಲದಿದ್ದರೆ{
    xCleanup(pParse->db, pPtr);
    pPtr = 0;
#ಆದರೆ defined(SQLITE_DEBUG) || defined(SQLITE_COVERAGE_TEST)
    pParse->earlyCleanup = 1;
#ಆದರೆ_ಅಂತ್ಯ
  }
  ಹಿಂತಿರುಗಿ pPtr;
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
  memset(PARSE_ಬಾಲ(pParse), 0, PARSE_ಬಾಲ_SZ);
  ಪ್ರತಿಜ್ಞಾ( db->pParse!=pParse );
  pParse->pOuterParse = db->pParse;
  db->pParse = pParse;
  pParse->db = db;
  ಆದರೆ( db->mallocFailed ) sqlite3_Error_ಸಂದೇಶ(pParse, "out of memory");
}

/*
** Maximum number of times that we will try again to prepare a statement
** that returns SQLITE_ERROR_RETRY.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_MAX_PREPARE_RETRY
# define SQLITE_MAX_PREPARE_RETRY 25
#ಆದರೆ_ಅಂತ್ಯ

/*
** Compile the UTF-8 encoded SQL statement zSql into a statement handle.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ sqlite3Prepare(
  sqlite3 *db,              /* Database handle. */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSql,         /* UTF-8 encoded SQL statement. */
  ಪೂರ್ಣಾಂಕ n_ಬೈಟುಗಳು,               /* Length of zSql in bytes. */
  u32 prepFlags,            /* Zero or more SQLITE_PREPARE_* flags */
  Vdbe *pReprepare,         /* VM being reprepared */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **pz_ಬಾಲ       /* OUT: End of parsed string */
){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;       /* Result code */
  ಪೂರ್ಣಾಂಕ i;                    /* Loop counter */
  Parse sParse;             /* Parsing context */

  /* sqlite3ParseObjectInit(&sParse, db); // inlined for performance */
  memset(PARSE_HDR(&sParse), 0, PARSE_HDR_SZ);
  memset(PARSE_ಬಾಲ(&sParse), 0, PARSE_ಬಾಲ_SZ);
  sParse.pOuterParse = db->pParse;
  db->pParse = &sParse;
  sParse.db = db;
  ಆದರೆ( pReprepare ){
    sParse.pReprepare = pReprepare;
    sParse.explain = sqlite3_stmt_isexplain((sqlite3_stmt*)pReprepare);
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪ್ರತಿಜ್ಞಾ( sParse.pReprepare==0 );
  }
  ಪ್ರತಿಜ್ಞಾ( ppStmt && *ppStmt==0 );
  ಆದರೆ( db->mallocFailed ){
    sqlite3_Error_ಸಂದೇಶ(&sParse, "out of memory");
    db->err_ಸಂಕೇತ = rc = SQLITE_NOMEM;
    ಹೋಗು ಕೊನೆ_prepare;
  }
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(db->mutex) );

  /* For a long-term use prepared statement avoid the use of
  ** lookaside memory.
  */
  ಆದರೆ( prepFlags & SQLITE_PREPARE_PERSISTENT ){
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
  ಆದರೆ( !db->noSharedCache ){
    ಗಾಗಿ(i=0; i<db->nDb; i++) {
      Btree *pBt = db->aDb[i].pBt;
      ಆದರೆ( pBt ){
        ಪ್ರತಿಜ್ಞಾ( sqlite3BtreeHoldsMutex(pBt) );
        rc = sqlite3BtreeSchemaLocked(pBt);
        ಆದರೆ( rc ){
          ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDb = db->aDb[i].zDbSName;
          sqlite3_Error_With_ಸಂದೇಶ(db, rc, "database schema is locked: %s", zDb);
          testcase( db->flags & SQLITE_ReadUncommit );
          ಹೋಗು ಕೊನೆ_prepare;
        }
      }
    }
  }

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
  ಆದರೆ( db->pDisconnect ) sqlite3VtabUnlockList(db);
#ಆದರೆ_ಅಂತ್ಯ

  ಆದರೆ( n_ಬೈಟುಗಳು>=0 && (n_ಬೈಟುಗಳು==0 || zSql[n_ಬೈಟುಗಳು-1]!=0) ){
    ಅಕ್ಷರ *zSqlCopy;
    ಪೂರ್ಣಾಂಕ mxLen = db->a_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_SQL_ಉದ್ದಳತೆ];
    testcase( n_ಬೈಟುಗಳು==mxLen );
    testcase( n_ಬೈಟುಗಳು==mxLen+1 );
    ಆದರೆ( n_ಬೈಟುಗಳು>mxLen ){
      sqlite3_Error_With_ಸಂದೇಶ(db, SQLITE_TOOBIG, "statement too long");
      rc = sqlite3ApiExit(db, SQLITE_TOOBIG);
      ಹೋಗು ಕೊನೆ_prepare;
    }
    zSqlCopy = sqlite3DbStrNDup(db, zSql, n_ಬೈಟುಗಳು);
    ಆದರೆ( zSqlCopy ){
      sqlite3_ಸತತ_Parser(&sParse, zSqlCopy);
      sParse.z_ಬಾಲ = &zSql[sParse.z_ಬಾಲ-zSqlCopy];
      sqlite3DbFree(db, zSqlCopy);
    }ಇಲ್ಲದಿದ್ದರೆ{
      sParse.z_ಬಾಲ = &zSql[n_ಬೈಟುಗಳು];
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    sqlite3_ಸತತ_Parser(&sParse, zSql);
  }
  ಪ್ರತಿಜ್ಞಾ( 0==sParse.nQueryLoop );

  ಆದರೆ( pz_ಬಾಲ ){
    *pz_ಬಾಲ = sParse.z_ಬಾಲ;
  }

  ಆದರೆ( db->init.busy==0 ){
    sqlite3_Vdbe_ಗಣ_Sql(sParse.pVdbe, zSql, (ಪೂರ್ಣಾಂಕ)(sParse.z_ಬಾಲ-zSql), prepFlags);
  }
  ಆದರೆ( db->mallocFailed ){
    sParse.rc = SQLITE_NOMEM_BKPT;
    sParse.checkSchema = 0;
  }
  ಆದರೆ( sParse.rc!=SQLITE_OK && sParse.rc!=SQLITE_ಮುಗಿದ ){
    ಆದರೆ( sParse.checkSchema && db->init.busy==0 ){
      schemaIsValid(&sParse);
    }
    ಆದರೆ( sParse.pVdbe ){
      sqlite3VdbeFinalize(sParse.pVdbe);
    }
    ಪ್ರತಿಜ್ಞಾ( 0==(*ppStmt) );
    rc = sParse.rc;
    ಆದರೆ( sParse.z_Err_ಸಂದೇಶ ){
      sqlite3_Error_With_ಸಂದೇಶ(db, rc, "%s", sParse.z_Err_ಸಂದೇಶ);
      sqlite3DbFree(db, sParse.z_Err_ಸಂದೇಶ);
    }ಇಲ್ಲದಿದ್ದರೆ{
      sqlite3Error(db, rc);
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪ್ರತಿಜ್ಞಾ( sParse.z_Err_ಸಂದೇಶ==0 );
    *ppStmt = (sqlite3_stmt*)sParse.pVdbe;
    rc = SQLITE_OK;
    sqlite3ErrorClear(db);
  }


  /* Delete any TriggerPrg structures allocated while parsing this statement. */
  ಆಗ( sParse.pTriggerPrg ){
    TriggerPrg *pT = sParse.pTriggerPrg;
    sParse.pTriggerPrg = pT->p_ಮುಂದಿನ;
    sqlite3DbFree(db, pT);
  }

ಕೊನೆ_prepare:

  sqlite3ParseObjectReset(&sParse);
  ಹಿಂತಿರುಗಿ rc;
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ sqlite3_ಬೀಗ_And_Prepare(
  sqlite3 *db,              /* Database handle. */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSql,         /* UTF-8 encoded SQL statement. */
  ಪೂರ್ಣಾಂಕ n_ಬೈಟುಗಳು,               /* Length of zSql in bytes. */
  u32 prepFlags,            /* Zero or more SQLITE_PREPARE_* flags */
  Vdbe *pOld,               /* VM being reprepared */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **pz_ಬಾಲ       /* OUT: End of parsed string */
){
  ಪೂರ್ಣಾಂಕ rc;
  ಪೂರ್ಣಾಂಕ cnt = 0;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( ppStmt==0 ) ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
#ಆದರೆ_ಅಂತ್ಯ
  *ppStmt = 0;
  ಆದರೆ( !sqlite3SafetyCheckOk(db)||zSql==0 ){
    ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(db->mutex);
  sqlite3BtreeEnterAll(db);
  ಮಾಡು{
    /* Make multiple attempts to compile the SQL, until it either succeeds
    ** or encounters a permanent error.  A schema problem after one schema
    ** reset is considered a permanent error. */
    rc = sqlite3Prepare(db, zSql, n_ಬೈಟುಗಳು, prepFlags, pOld, ppStmt, pz_ಬಾಲ);
    ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_OK || *ppStmt==0 );
    ಆದರೆ( rc==SQLITE_OK || db->mallocFailed ) ನಿಲ್ಲಿಸು;
    cnt++;
  }ಆಗ( (rc==SQLITE_ERROR_RETRY && ALWAYS(cnt<=SQLITE_MAX_PREPARE_RETRY))
       || (rc==SQLITE_SCHEMA && (sqlite3ResetOneSchema(db,-1), cnt)==1) );
  sqlite3BtreeLeaveAll(db);
  ಪ್ರತಿಜ್ಞಾ( rc!=SQLITE_ERROR_RETRY );
  rc = sqlite3ApiExit(db, rc);
  ಪ್ರತಿಜ್ಞಾ( (rc&db->err_ಮುಖವಾಡ)==rc );
  db->busyHandler.nBusy = 0;
  sqlite3_mutex_leave(db->mutex);
  ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_OK || (*ppStmt)==0 );
  ಹಿಂತಿರುಗಿ rc;
}


/*
** Rerun the compilation of a statement after a schema change.
**
** If the statement is successfully recompiled, return SQLITE_OK. Otherwise,
** if the statement cannot be recompiled because another connection has
** locked the sqlite3_schema table, return SQLITE_LOCKED. If any other error
** occurs, return SQLITE_SCHEMA.
*/
ಪೂರ್ಣಾಂಕ sqlite3Reprepare(Vdbe *p){
  ಪೂರ್ಣಾಂಕ rc;
  sqlite3_stmt *pNew;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSql;
  sqlite3 *db;
  u8 prepFlags;

  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(sqlite3VdbeDb(p)->mutex) );
  zSql = sqlite3_sql((sqlite3_stmt *)p);
  ಪ್ರತಿಜ್ಞಾ( zSql!=0 );  /* Reprepare only called for prepare_v2() statements */
  db = sqlite3VdbeDb(p);
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(db->mutex) );
  prepFlags = sqlite3VdbePrepareFlags(p);
  rc = sqlite3_ಬೀಗ_And_Prepare(db, zSql, -1, prepFlags, p, &pNew, 0);
  ಆದರೆ( rc ){
    ಆದರೆ( rc==SQLITE_NOMEM ){
      sqlite3OomFault(db);
    }
    ಪ್ರತಿಜ್ಞಾ( pNew==0 );
    ಹಿಂತಿರುಗಿ rc;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪ್ರತಿಜ್ಞಾ( pNew!=0 );
  }
  sqlite3_Vdbe_ಅದಲುಬದಲು((Vdbe*)pNew, p);
  sqlite3TransferBindings(pNew, (sqlite3_stmt*)p);
  sqlite3_Vdbe_Reset_ಹೆಜ್ಜೆ_ಫಲಿತಾಂಶ((Vdbe*)pNew);
  sqlite3VdbeFinalize((Vdbe*)pNew);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}


/*
** Two versions of the official API.  Legacy and new use.  In the legacy
** version, the original SQL text is not saved in the prepared statement
** and so if a schema change occurs, SQLITE_SCHEMA is returned by
** sqlite3_step().  In the new version, the original SQL text is retained
** and the statement is automatically recompiled if an schema change
** occurs.
*/
ಪೂರ್ಣಾಂಕ sqlite3_prepare(
  sqlite3 *db,              /* Database handle. */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSql,         /* UTF-8 encoded SQL statement. */
  ಪೂರ್ಣಾಂಕ n_ಬೈಟುಗಳು,               /* Length of zSql in bytes. */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **pz_ಬಾಲ       /* OUT: End of parsed string */
){
  ಪೂರ್ಣಾಂಕ rc;
  rc = sqlite3_ಬೀಗ_And_Prepare(db,zSql,n_ಬೈಟುಗಳು,0,0,ppStmt,pz_ಬಾಲ);
  ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_OK || ppStmt==0 || *ppStmt==0 );  /* VERIFY: F13021 */
  ಹಿಂತಿರುಗಿ rc;
}
ಪೂರ್ಣಾಂಕ sqlite3_prepare_v2(
  sqlite3 *db,              /* Database handle. */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSql,         /* UTF-8 encoded SQL statement. */
  ಪೂರ್ಣಾಂಕ n_ಬೈಟುಗಳು,               /* Length of zSql in bytes. */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **pz_ಬಾಲ       /* OUT: End of parsed string */
){
  ಪೂರ್ಣಾಂಕ rc;
  /* EVIDENCE-OF: R-37923-12173 The sqlite3_prepare_v2() interface works
  ** exactly the same as sqlite3_prepare_v3() with a zero prepFlags
  ** parameter.
  **
  ** Proof in that the 5th parameter to sqlite3LockAndPrepare is 0 */
  rc = sqlite3_ಬೀಗ_And_Prepare(db,zSql,n_ಬೈಟುಗಳು,SQLITE_PREPARE_SAVESQL,0,
                             ppStmt,pz_ಬಾಲ);
  ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_OK || ppStmt==0 || *ppStmt==0 );
  ಹಿಂತಿರುಗಿ rc;
}
ಪೂರ್ಣಾಂಕ sqlite3_prepare_v3(
  sqlite3 *db,              /* Database handle. */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSql,         /* UTF-8 encoded SQL statement. */
  ಪೂರ್ಣಾಂಕ n_ಬೈಟುಗಳು,               /* Length of zSql in bytes. */
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ prepFlags,   /* Zero or more SQLITE_PREPARE_* flags */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **pz_ಬಾಲ       /* OUT: End of parsed string */
){
  ಪೂರ್ಣಾಂಕ rc;
  /* EVIDENCE-OF: R-56861-42673 sqlite3_prepare_v3() differs from
  ** sqlite3_prepare_v2() only in having the extra prepFlags parameter,
  ** which is a bit array consisting of zero or more of the
  ** SQLITE_PREPARE_* flags.
  **
  ** Proof by comparison to the implementation of sqlite3_prepare_v2()
  ** directly above. */
  rc = sqlite3_ಬೀಗ_And_Prepare(db,zSql,n_ಬೈಟುಗಳು,
                 SQLITE_PREPARE_SAVESQL|(prepFlags&SQLITE_PREPARE_ಮುಖವಾಡ),
                 0,ppStmt,pz_ಬಾಲ);
  ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_OK || ppStmt==0 || *ppStmt==0 );
  ಹಿಂತಿರುಗಿ rc;
}


#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_UTF16
/*
** Compile the UTF-16 encoded SQL statement zSql into a statement handle.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ sqlite3Prepare16(
  sqlite3 *db,              /* Database handle. */ 
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *zSql,         /* UTF-16 encoded SQL statement. */
  ಪೂರ್ಣಾಂಕ n_ಬೈಟುಗಳು,               /* Length of zSql in bytes. */
  u32 prepFlags,            /* Zero or more SQLITE_PREPARE_* flags */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ **pz_ಬಾಲ       /* OUT: End of parsed string */
){
  /* This function currently works by first transforming the UTF-16
  ** encoded string to UTF-8, then invoking sqlite3_prepare(). The
  ** tricky bit is figuring out the pointer to return in *pzTail.
  */
  ಅಕ್ಷರ *zSql8;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zTail8 = 0;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( ppStmt==0 ) ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
#ಆದರೆ_ಅಂತ್ಯ
  *ppStmt = 0;
  ಆದರೆ( !sqlite3SafetyCheckOk(db)||zSql==0 ){
    ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
  }

  /* Make sure nBytes is non-negative and correct.  It should be the
  ** number of bytes until the end of the input buffer or until the first
  ** U+0000 character.  If the input nBytes is odd, convert it into
  ** an even number.  If the input nBytes is negative, then the input
  ** must be terminated by at least one U+0000 character */
  ಆದರೆ( n_ಬೈಟುಗಳು>=0 ){
    ಪೂರ್ಣಾಂಕ sz;
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)zSql;
    ಗಾಗಿ(sz=0; sz<n_ಬೈಟುಗಳು && (z[sz]!=0 || z[sz+1]!=0); sz += 2){}
    n_ಬೈಟುಗಳು = sz;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪೂರ್ಣಾಂಕ sz;
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)zSql;
    ಗಾಗಿ(sz=0; z[sz]!=0 || z[sz+1]!=0; sz += 2){}
    n_ಬೈಟುಗಳು = sz;
  }

  sqlite3_mutex_enter(db->mutex);
  zSql8 = sqlite3Utf16to8(db, zSql, n_ಬೈಟುಗಳು, SQLITE_UTF16NATIVE);
  ಆದರೆ( zSql8 ){
    rc = sqlite3_ಬೀಗ_And_Prepare(db, zSql8, -1, prepFlags, 0, ppStmt, &zTail8);
  }

  ಆದರೆ( zTail8 && pz_ಬಾಲ ){
    /* If sqlite3_prepare returns a tail pointer, we calculate the
    ** equivalent pointer into the UTF-16 string by counting the unicode
    ** characters between zSql8 and zTail8, and then returning a pointer
    ** the same number of characters into the UTF-16 string.
    */
    ಪೂರ್ಣಾಂಕ chars_parsed = sqlite3Utf8CharLen(zSql8, (ಪೂರ್ಣಾಂಕ)(zTail8-zSql8));
    *pz_ಬಾಲ = (u8 *)zSql + sqlite3Utf16ByteLen(zSql, n_ಬೈಟುಗಳು, chars_parsed);
  }
  sqlite3DbFree(db, zSql8); 
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Two versions of the official API.  Legacy and new use.  In the legacy
** version, the original SQL text is not saved in the prepared statement
** and so if a schema change occurs, SQLITE_SCHEMA is returned by
** sqlite3_step().  In the new version, the original SQL text is retained
** and the statement is automatically recompiled if an schema change
** occurs.
*/
ಪೂರ್ಣಾಂಕ sqlite3_prepare16(
  sqlite3 *db,              /* Database handle. */ 
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *zSql,         /* UTF-16 encoded SQL statement. */
  ಪೂರ್ಣಾಂಕ n_ಬೈಟುಗಳು,               /* Length of zSql in bytes. */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ **pz_ಬಾಲ       /* OUT: End of parsed string */
){
  ಪೂರ್ಣಾಂಕ rc;
  rc = sqlite3Prepare16(db,zSql,n_ಬೈಟುಗಳು&~1,0,ppStmt,pz_ಬಾಲ);
  ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_OK || ppStmt==0 || *ppStmt==0 );  /* VERIFY: F13021 */
  ಹಿಂತಿರುಗಿ rc;
}
ಪೂರ್ಣಾಂಕ sqlite3_prepare16_v2(
  sqlite3 *db,              /* Database handle. */ 
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *zSql,         /* UTF-16 encoded SQL statement. */
  ಪೂರ್ಣಾಂಕ n_ಬೈಟುಗಳು,               /* Length of zSql in bytes. */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ **pz_ಬಾಲ       /* OUT: End of parsed string */
){
  ಪೂರ್ಣಾಂಕ rc;
  rc = sqlite3Prepare16(db,zSql,n_ಬೈಟುಗಳು&~1,SQLITE_PREPARE_SAVESQL,ppStmt,pz_ಬಾಲ);
  ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_OK || ppStmt==0 || *ppStmt==0 );  /* VERIFY: F13021 */
  ಹಿಂತಿರುಗಿ rc;
}
ಪೂರ್ಣಾಂಕ sqlite3_prepare16_v3(
  sqlite3 *db,              /* Database handle. */ 
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *zSql,         /* UTF-16 encoded SQL statement. */
  ಪೂರ್ಣಾಂಕ n_ಬೈಟುಗಳು,               /* Length of zSql in bytes. */
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ prepFlags,   /* Zero or more SQLITE_PREPARE_* flags */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ **pz_ಬಾಲ       /* OUT: End of parsed string */
){
  ಪೂರ್ಣಾಂಕ rc;
  rc = sqlite3Prepare16(db,zSql,n_ಬೈಟುಗಳು&~1,
         SQLITE_PREPARE_SAVESQL|(prepFlags&SQLITE_PREPARE_ಮುಖವಾಡ),
         ppStmt,pz_ಬಾಲ);
  ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_OK || ppStmt==0 || *ppStmt==0 );  /* VERIFY: F13021 */
  ಹಿಂತಿರುಗಿ rc;
}

#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_UTF16 */
