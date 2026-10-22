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
#అన్తర్భావ "sqliteInt.h"

/*
** Fill the InitData structure with an error message that indicates
** that the database is corrupt.
*/
స్థిర శూన్య corruptSchema(
  Init_దత్తాంశం *p_దత్తాంశం,     /* Initialization context */
  అక్షరం **azObj,        /* Type and name of object being parsed */
  స్థిరరాశి అక్షరం *zExtra   /* Error information */
){
  sqlite3 *db = p_దత్తాంశం->db;
  ఒకవేళ( db->mallocFailed ){
    p_దత్తాంశం->rc = SQLITE_NOMEM_BKPT;
  }లేకపోతే ఒకవేళ( p_దత్తాంశం->pz_Err_సందేశం[0]!=0 ){
    /* A error message has already been generated.  Do not overwrite it */
  }లేకపోతే ఒకవేళ( p_దత్తాంశం->mInitFlags & (INITFLAG_Alter_ముసుగు) ){
    స్థిర స్థిరరాశి అక్షరం *azAlterType[] = {
       "rename",
       "drop column",
       "add column",
       "drop constraint"
    };
    *p_దత్తాంశం->pz_Err_సందేశం = sqlite3MPrintf(db, 
        "error in %s %s after %s: %s", azObj[0], azObj[1], 
        azAlterType[(p_దత్తాంశం->mInitFlags&INITFLAG_Alter_ముసుగు)-1], 
        zExtra
    );
    p_దత్తాంశం->rc = SQLITE_ERROR;
  }లేకపోతే ఒకవేళ( db->flags & SQLITE_WriteSchema ){
    p_దత్తాంశం->rc = SQLITE_CORRUPT_BKPT;
  }లేకపోతే{
    అక్షరం *z;
    స్థిరరాశి అక్షరం *zObj = azObj[1] ? azObj[1] : "?";
    z = sqlite3MPrintf(db, "malformed database schema (%s)", zObj);
    ఒకవేళ( zExtra && zExtra[0] ) z = sqlite3MPrintf(db, "%z - %s", z, zExtra);
    *p_దత్తాంశం->pz_Err_సందేశం = z;
    p_దత్తాంశం->rc = SQLITE_CORRUPT_BKPT;
  }
}

/*
** Check to see if any sibling index (another index on the same table)
** of pIndex has the same root page number, and if it does, return true.
** This would indicate a corrupt schema.
*/
పూర్ణాంకం sqlite3_సూచీ_Has_Duplicate_మూలం_పుట(సూచీ *p_సూచీ){
  సూచీ *p;
  కొరకు(p=p_సూచీ->p_పట్టిక->p_సూచీ; p; p=p->p_తదుపరి){
    ఒకవేళ( p->tnum==p_సూచీ->tnum && p!=p_సూచీ ) తిరిగి 1;
  }
  తిరిగి 0;
}

/* forward declaration */
స్థిర పూర్ణాంకం sqlite3Prepare(
  sqlite3 *db,              /* Database handle. */
  స్థిరరాశి అక్షరం *zSql,         /* UTF-8 encoded SQL statement. */
  పూర్ణాంకం n_బైట్లు,               /* Length of zSql in bytes. */
  u32 prepFlags,            /* Zero or more SQLITE_PREPARE_* flags */
  Vdbe *pReprepare,         /* VM being reprepared */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  స్థిరరాశి అక్షరం **pz_తోక       /* OUT: End of parsed string */
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
పూర్ణాంకం sqlite3InitCallback(శూన్య *pInit, పూర్ణాంకం argc, అక్షరం **argv, అక్షరం **Not_వాడిన){
  Init_దత్తాంశం *p_దత్తాంశం = (Init_దత్తాంశం*)pInit;
  sqlite3 *db = p_దత్తాంశం->db;
  పూర్ణాంకం iDb = p_దత్తాంశం->iDb;

  ప్రతిజ్ఞా( argc==5 );
  వాడని_PARAMETER2(Not_వాడిన, argc);
  ప్రతిజ్ఞా( sqlite3_mutex_held(db->mutex) );
  db->mDbFlags |= DBFLAG_EncodingFixed;
  ఒకవేళ( argv==0 ) తిరిగి 0;   /* Might happen if EMPTY_RESULT_CALLBACKS are on */
  p_దత్తాంశం->n_Init_అడ్డువరుస++;
  ఒకవేళ( db->mallocFailed ){
    corruptSchema(p_దత్తాంశం, argv, 0);
    తిరిగి 1;
  }

  ప్రతిజ్ఞా( iDb>=0 && iDb<db->nDb );
  ప్రతిజ్ఞా( db->aDb[iDb].pSchema!=0 );
  ఒకవేళ( argv[3]==0 ){
    corruptSchema(p_దత్తాంశం, argv, 0);
  }లేకపోతే ఒకవేళ( argv[4]
         && 'c'==sqlite3_పై_గమ్యం_క్రింది[(గుర్తులేని అక్షరం)argv[4][0]]
         && 'r'==sqlite3_పై_గమ్యం_క్రింది[(గుర్తులేని అక్షరం)argv[4][1]] ){
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
    పూర్ణాంకం rc;
    u8 saved_iDb = db->init.iDb;
    sqlite3_stmt *pStmt;
    స్థిరరాశి అక్షరం *z_చివర;
    TESTONLY(పూర్ణాంకం rcp);            /* Return code from sqlite3_prepare() */

    ప్రతిజ్ఞా( db->init.busy );
    db->init.iDb = iDb;
    ఒకవేళ( sqlite3GetUInt32(argv[3], &db->init.newTnum)==0
     || (db->init.newTnum>p_దత్తాంశం->mx_పుట && p_దత్తాంశం->mx_పుట>0)
    ){
      ఒకవేళ( sqlite3Config.bExtraSchemaChecks ){
        corruptSchema(p_దత్తాంశం, argv, "invalid rootpage");
      }
    }
    db->init.orphanTrigger = 0;
    db->init.azInit = (స్థిరరాశి అక్షరం**)argv;
    pStmt = 0;
    TESTONLY(rcp = ) sqlite3Prepare(db, argv[4], -1, 0, 0, &pStmt, &z_చివర);
    rc = db->err_సంకేతం;
    ప్రతిజ్ఞా( (rc&0xFF)==(rcp&0xFF) );
    db->init.iDb = saved_iDb;
    /* assert( saved_iDb==0 || (db->mDbFlags & DBFLAG_Vacuum)!=0 ); */
    ఒకవేళ( SQLITE_OK!=rc ){
      ఒకవేళ( db->init.orphanTrigger ){
        ప్రతిజ్ఞా( iDb==1 );
      }లేకపోతే{
        ఒకవేళ( rc > p_దత్తాంశం->rc ) p_దత్తాంశం->rc = rc;
        ఒకవేళ( rc==SQLITE_NOMEM ){
          sqlite3OomFault(db);
        }లేకపోతే ఒకవేళ( rc!=SQLITE_అంతరాయం && (rc&0xFF)!=SQLITE_LOCKED ){
          corruptSchema(p_దత్తాంశం, argv, sqlite3_errmsg(db));
        }
      }
    }లేకపోతే ఒకవేళ( z_చివర[0] ){
      corruptSchema(p_దత్తాంశం, argv, 0);
    }
    db->init.azInit = sqlite3StdType; /* Any array of string ptrs will do */
    sqlite3_finalize(pStmt);
  }లేకపోతే ఒకవేళ( argv[1]==0 || (argv[4]!=0 && argv[4][0]!=0) ){
    corruptSchema(p_దత్తాంశం, argv, 0);
  }లేకపోతే{
    /* If the SQL column is blank it means this is an index that
    ** was created to be the PRIMARY KEY or to fulfill a UNIQUE
    ** constraint for a CREATE TABLE.  The index should have already
    ** been created when we processed the CREATE TABLE.  All we have
    ** to do here is record the root page number for that index.
    */
    సూచీ *p_సూచీ;
    p_సూచీ = sqlite3_కనుగొను_సూచీ(db, argv[1], db->aDb[iDb].zDbSName);
    ఒకవేళ( p_సూచీ==0 ){
      corruptSchema(p_దత్తాంశం, argv, "orphan index");
    }లేకపోతే
    ఒకవేళ( sqlite3GetUInt32(argv[3],&p_సూచీ->tnum)==0
     || p_సూచీ->tnum<2
     || p_సూచీ->tnum>p_దత్తాంశం->mx_పుట
     || sqlite3_సూచీ_Has_Duplicate_మూలం_పుట(p_సూచీ)
    ){
      ఒకవేళ( sqlite3Config.bExtraSchemaChecks ){
        corruptSchema(p_దత్తాంశం, argv, "invalid rootpage");
      }
    }
  }
  ఒకవేళ( p_దత్తాంశం->pz_Err_సందేశం[0]==0 ){
    Schema *pX = db->aDb[iDb].pSchema;
    ఒకవేళ( pX->tblHash.లెక్క + pX->idxHash.లెక్క + pX->trigHash.లెక్క
                   > (u32)db->a_హద్దు[SQLITE_హద్దు_SCHEMA]
    ){
      *p_దత్తాంశం->pz_Err_సందేశం = sqlite3MPrintf(db, "too many schema objects");
      p_దత్తాంశం->rc = SQLITE_ERROR;
      తిరిగి 1;
    }
  }
  తిరిగి 0;
}

/*
** Attempt to read the database schema and initialize internal
** data structures for a single database file.  The index of the
** database file is given by iDb.  iDb==0 is used for the main
** database.  iDb==1 should never be used.  iDb>=2 is used for
** auxiliary databases.  Return one of the SQLITE_ error codes to
** indicate success or failure.
*/
పూర్ణాంకం sqlite3InitOne(sqlite3 *db, పూర్ణాంకం iDb, అక్షరం **pz_Err_సందేశం, u32 mFlags){
  పూర్ణాంకం rc;
  పూర్ణాంకం i;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_DEPRECATED
  పూర్ణాంకం కొలత;
#అయితే_ముగింపు
  Db *pDb;
  అక్షరం స్థిరరాశి *azArg[6];
  పూర్ణాంకం meta[5];
  Init_దత్తాంశం init_దత్తాంశం;
  స్థిరరాశి అక్షరం *zSchemaTabName;
  పూర్ణాంకం openedTransaction = 0;
  పూర్ణాంకం ముసుగు = ((db->mDbFlags & DBFLAG_EncodingFixed) | ~DBFLAG_EncodingFixed);

  ప్రతిజ్ఞా( (db->mDbFlags & DBFLAG_Schema_తెలిసిన_Ok)==0 );
  ప్రతిజ్ఞా( iDb>=0 && iDb<db->nDb );
  ప్రతిజ్ఞా( db->aDb[iDb].pSchema );
  ప్రతిజ్ఞా( sqlite3_mutex_held(db->mutex) );
  ప్రతిజ్ఞా( iDb==1 || sqlite3BtreeHoldsMutex(db->aDb[iDb].pBt) );

  db->init.busy = 1 + ((mFlags & INITFLAG_Alter_కూడు)!=0);
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
  azArg[1] = zSchemaTabName = SCHEMA_పట్టిక(iDb);
  azArg[2] = azArg[1];
  azArg[3] = "1";
  azArg[4] = "CREATE TABLE x(type text,name text,tbl_name text,"
                            "rootpage int,sql text)";
  azArg[5] = 0;
  init_దత్తాంశం.db = db;
  init_దత్తాంశం.iDb = iDb;
  init_దత్తాంశం.rc = SQLITE_OK;
  init_దత్తాంశం.pz_Err_సందేశం = pz_Err_సందేశం;
  init_దత్తాంశం.mInitFlags = mFlags;
  init_దత్తాంశం.n_Init_అడ్డువరుస = 0;
  init_దత్తాంశం.mx_పుట = 0;
  sqlite3InitCallback(&init_దత్తాంశం, 5, (అక్షరం **)azArg, 0);
  db->mDbFlags &= ముసుగు;
  ఒకవేళ( init_దత్తాంశం.rc ){
    rc = init_దత్తాంశం.rc;
    వెళ్ళు error_బహిర్గతం;
  }

  /* Create a cursor to hold the database open
  */
  pDb = &db->aDb[iDb];
  ఒకవేళ( pDb->pBt==0 ){
    ప్రతిజ్ఞా( iDb==1 );
    Db_సమితి_Property(db, 1, DB_SchemaLoaded);
    rc = SQLITE_OK;
    వెళ్ళు error_బహిర్గతం;
  }

  /* If there is not already a read-only (or read-write) transaction opened
  ** on the b-tree database, open one now. If a transaction is opened, it 
  ** will be closed before this function returns.  */
  sqlite3BtreeEnter(pDb->pBt);
  ఒకవేళ( sqlite3BtreeTxnState(pDb->pBt)==SQLITE_TXN_NONE ){
    rc = sqlite3BtreeBeginTrans(pDb->pBt, 0, 0);
    ఒకవేళ( rc!=SQLITE_OK ){
      sqlite3_సమితి_String(pz_Err_సందేశం, db, sqlite3ErrStr(rc));
      వెళ్ళు initone_error_బహిర్గతం;
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
  కొరకు(i=0; i<Array_కొలత(meta); i++){
    sqlite3BtreeGetMeta(pDb->pBt, i+1, (u32 *)&meta[i]);
  }
  ఒకవేళ( (db->flags & SQLITE_ResetDatabase)!=0 ){
    memset(meta, 0, పరిమాణం(meta));
  }
  pDb->pSchema->schema_cookie = meta[BTREE_SCHEMA_సంస్కరణ-1];

  /* If opening a non-empty database, check the text encoding. For the
  ** main database, set sqlite3.enc to the encoding of the main database.
  ** For an attached db, it is an error if the encoding is not the same
  ** as sqlite3.enc.
  */
  ఒకవేళ( meta[BTREE_పాఠ్యం_ENCODING-1] ){  /* text encoding */
    ఒకవేళ( iDb==0 && (db->mDbFlags & DBFLAG_EncodingFixed)==0 ){
      u8 encoding;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_UTF16
      /* If opening the main database, set ENC(db). */
      encoding = (u8)meta[BTREE_పాఠ్యం_ENCODING-1] & 3;
      ఒకవేళ( encoding==0 ) encoding = SQLITE_UTF8;
#లేకపోతే
      encoding = SQLITE_UTF8;
#అయితే_ముగింపు
      sqlite3_సమితి_పాఠ్యం_Encoding(db, encoding);
    }లేకపోతే{
      /* If opening an attached database, the encoding much match ENC(db) */
      ఒకవేళ( (meta[BTREE_పాఠ్యం_ENCODING-1] & 3)!=ENC(db) ){
        sqlite3_సమితి_String(pz_Err_సందేశం, db, "attached databases must use the same"
            " text encoding as main database");
        rc = SQLITE_ERROR;
        వెళ్ళు initone_error_బహిర్గతం;
      }
    }
  }
  pDb->pSchema->enc = ENC(db);

  ఒకవేళ( pDb->pSchema->cache_కొలత==0 ){
#నిర్వచితం_కాకపోతే SQLITE_OMIT_DEPRECATED
    కొలత = sqlite3AbsInt32(meta[BTREE_DEFAULT_CACHE_కొలత-1]);
    ఒకవేళ( కొలత==0 ){ కొలత = SQLITE_DEFAULT_CACHE_కొలత; }
    pDb->pSchema->cache_కొలత = కొలత;
#లేకపోతే
    pDb->pSchema->cache_కొలత = SQLITE_DEFAULT_CACHE_కొలత;
#అయితే_ముగింపు
    sqlite3_Btree_సమితి_Cache_కొలత(pDb->pBt, pDb->pSchema->cache_కొలత);
  }

  /*
  ** file_format==1    Version 3.0.0.
  ** file_format==2    Version 3.1.3.  // ALTER TABLE ADD COLUMN
  ** file_format==3    Version 3.1.4.  // ditto but with non-NULL defaults
  ** file_format==4    Version 3.3.0.  // DESC indices.  Boolean constants
  */
  pDb->pSchema->file_format = (u8)meta[BTREE_FILE_FORMAT-1];
  ఒకవేళ( pDb->pSchema->file_format==0 ){
    pDb->pSchema->file_format = 1;
  }
  ఒకవేళ( pDb->pSchema->file_format>SQLITE_MAX_FILE_FORMAT ){
    sqlite3_సమితి_String(pz_Err_సందేశం, db, "unsupported file format");
    rc = SQLITE_ERROR;
    వెళ్ళు initone_error_బహిర్గతం;
  }

  /* Ticket #2804:  When we open a database in the newer file format,
  ** clear the legacy_file_format pragma flag so that a VACUUM will
  ** not downgrade the database and thus invalidate any descending
  ** indices that the user might have created.
  */
  ఒకవేళ( iDb==0 && meta[BTREE_FILE_FORMAT-1]>=4 ){
    db->flags &= ~(u64)SQLITE_LegacyFileFmt;
  }

  /* Read the schema information out of the schema tables
  */
  ప్రతిజ్ఞా( db->init.busy );
  init_దత్తాంశం.mx_పుట = sqlite3_Btree_చివరి_పుట(pDb->pBt);
  {
    అక్షరం *zSql;
    zSql = sqlite3MPrintf(db, 
        "SELECT*FROM\"%w\".%s ORDER BY rowid",
        db->aDb[iDb].zDbSName, zSchemaTabName);
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTHORIZATION
    {
      sqlite3_xauth xAuth;
      xAuth = db->xAuth;
      db->xAuth = 0;
#అయితే_ముగింపు
      rc = sqlite3_exec(db, zSql, sqlite3InitCallback, &init_దత్తాంశం, 0);
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTHORIZATION
      db->xAuth = xAuth;
    }
#అయితే_ముగింపు
    ఒకవేళ( rc==SQLITE_OK ) rc = init_దత్తాంశం.rc;
    sqlite3DbFree(db, zSql);
#నిర్వచితం_కాకపోతే SQLITE_OMIT_ANALYZE
    ఒకవేళ( rc==SQLITE_OK ){
      sqlite3_Analysis_ఎక్కించు(db, iDb);
    }
#అయితే_ముగింపు
  }
  ప్రతిజ్ఞా( pDb == &(db->aDb[iDb]) );
  ఒకవేళ( db->mallocFailed ){
    rc = SQLITE_NOMEM_BKPT;
    sqlite3_Reset_All_Schemas_యొక్క_Connection(db);
    pDb = &db->aDb[iDb];
  }లేకపోతే
  ఒకవేళ( rc==SQLITE_OK || ((db->flags&SQLITE_NoSchemaError) && rc!=SQLITE_NOMEM)){
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
    Db_సమితి_Property(db, iDb, DB_SchemaLoaded);
    rc = SQLITE_OK;
  }

  /* Jump here for an error that occurs after successfully allocating
  ** curMain and calling sqlite3BtreeEnter(). For an error that occurs
  ** before that point, jump to error_out.
  */
initone_error_బహిర్గతం:
  ఒకవేళ( openedTransaction ){
    sqlite3BtreeCommit(pDb->pBt);
  }
  sqlite3BtreeLeave(pDb->pBt);

error_బహిర్గతం:
  ఒకవేళ( rc ){
    ఒకవేళ( rc==SQLITE_NOMEM || rc==SQLITE_IOERR_NOMEM ){
      sqlite3OomFault(db);
    }
    sqlite3ResetOneSchema(db, iDb);
  }
  db->init.busy = 0;
  తిరిగి rc;
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
పూర్ణాంకం sqlite3Init(sqlite3 *db, అక్షరం **pz_Err_సందేశం){
  పూర్ణాంకం i, rc;
  పూర్ణాంకం commit_అంతర్గత = !(db->mDbFlags&DBFLAG_SchemaChange);
  
  ప్రతిజ్ఞా( sqlite3_mutex_held(db->mutex) );
  ప్రతిజ్ఞా( sqlite3BtreeHoldsMutex(db->aDb[0].pBt) );
  ప్రతిజ్ఞా( db->init.busy==0 );
  ENC(db) = SCHEMA_ENC(db);
  ప్రతిజ్ఞా( db->nDb>0 );
  /* Do the main schema first */
  ఒకవేళ( !DbHasProperty(db, 0, DB_SchemaLoaded) ){
    rc = sqlite3InitOne(db, 0, pz_Err_సందేశం, 0);
    ఒకవేళ( rc ) తిరిగి rc;
  }
  /* All other schemas after the main schema. The "temp" schema must be last */
  కొరకు(i=db->nDb-1; i>0; i--){
    ప్రతిజ్ఞా( i==1 || sqlite3BtreeHoldsMutex(db->aDb[i].pBt) );
    ఒకవేళ( !DbHasProperty(db, i, DB_SchemaLoaded) ){
      rc = sqlite3InitOne(db, i, pz_Err_సందేశం, 0);
      ఒకవేళ( rc ) తిరిగి rc;
    }
  }
  ఒకవేళ( commit_అంతర్గత ){
    sqlite3_Commit_అంతర్గత_Changes(db);
  }
  తిరిగి SQLITE_OK;
}

/*
** This routine is a no-op if the database schema is already initialized.
** Otherwise, the schema is loaded. An error code is returned.
*/
పూర్ణాంకం sqlite3ReadSchema(Parse *pParse){
  పూర్ణాంకం rc = SQLITE_OK;
  sqlite3 *db = pParse->db;
  ప్రతిజ్ఞా( sqlite3_mutex_held(db->mutex) );
  ఒకవేళ( !db->init.busy ){
    rc = sqlite3Init(db, &pParse->z_Err_సందేశం);
    ఒకవేళ( rc!=SQLITE_OK ){
      pParse->rc = rc;
      pParse->nErr++;
    }లేకపోతే ఒకవేళ( db->noSharedCache ){
      db->mDbFlags |= DBFLAG_Schema_తెలిసిన_Ok;
    }
  }
  తిరిగి rc;
}


/*
** Check schema cookies in all databases.  If any cookie is out
** of date set pParse->rc to SQLITE_SCHEMA.  If all schema cookies
** make no changes to pParse->rc.
*/
స్థిర శూన్య schemaIsValid(Parse *pParse){
  sqlite3 *db = pParse->db;
  పూర్ణాంకం iDb;
  పూర్ణాంకం rc;
  పూర్ణాంకం cookie;

  ప్రతిజ్ఞా( pParse->checkSchema );
  ప్రతిజ్ఞా( sqlite3_mutex_held(db->mutex) );
  కొరకు(iDb=0; iDb<db->nDb; iDb++){
    పూర్ణాంకం openedTransaction = 0;         /* True if a transaction is opened */
    Btree *pBt = db->aDb[iDb].pBt;     /* Btree database to read cookie from */
    ఒకవేళ( pBt==0 ) కొనసాగు;

    /* If there is not already a read-only (or read-write) transaction opened
    ** on the b-tree database, open one now. If a transaction is opened, it 
    ** will be closed immediately after reading the meta-value. */
    ఒకవేళ( sqlite3BtreeTxnState(pBt)==SQLITE_TXN_NONE ){
      rc = sqlite3BtreeBeginTrans(pBt, 0, 0);
      ఒకవేళ( rc==SQLITE_NOMEM || rc==SQLITE_IOERR_NOMEM ){
        sqlite3OomFault(db);
        pParse->rc = SQLITE_NOMEM;
      }
      ఒకవేళ( rc!=SQLITE_OK ) తిరిగి;
      openedTransaction = 1;
    }

    /* Read the schema cookie from the database. If it does not match the 
    ** value stored as part of the in-memory schema representation,
    ** set Parse.rc to SQLITE_SCHEMA. */
    sqlite3BtreeGetMeta(pBt, BTREE_SCHEMA_సంస్కరణ, (u32 *)&cookie);
    ప్రతిజ్ఞా( sqlite3SchemaMutexHeld(db, iDb, 0) );
    ఒకవేళ( cookie!=db->aDb[iDb].pSchema->schema_cookie ){
      ఒకవేళ( DbHasProperty(db, iDb, DB_SchemaLoaded) ) pParse->rc = SQLITE_SCHEMA;
      sqlite3ResetOneSchema(db, iDb);
    }

    /* Close the transaction, if one was opened. */
    ఒకవేళ( openedTransaction ){
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
పూర్ణాంకం sqlite3_Schema_గమ్యం_సూచీ(sqlite3 *db, Schema *pSchema){
  పూర్ణాంకం i = -32768;

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
  ప్రతిజ్ఞా( sqlite3_mutex_held(db->mutex) );
  ఒకవేళ( pSchema ){
    కొరకు(i=0; 1; i++){
      ప్రతిజ్ఞా( i<db->nDb );
      ఒకవేళ( db->aDb[i].pSchema==pSchema ){
        ఆపు;
      }
    }
    ప్రతిజ్ఞా( i>=0 && i<db->nDb );
  }
  తిరిగి i;
}

/*
** Free all memory allocations in the pParse object
*/
శూన్య sqlite3ParseObjectReset(Parse *pParse){
  sqlite3 *db = pParse->db;
  ప్రతిజ్ఞా( db!=0 );
  ప్రతిజ్ఞా( db->pParse==pParse );
  ప్రతిజ్ఞా( pParse->nested==0 );
#నిర్వచితం_కాకపోతే SQLITE_OMIT_SHARED_CACHE
  ఒకవేళ( pParse->n_పట్టిక_తాళం ) sqlite3DbNNFreeNN(db, pParse->a_పట్టిక_తాళం);
#అయితే_ముగింపు
  అయితే( pParse->pCleanup ){
    ParseCleanup *pCleanup = pParse->pCleanup;
    pParse->pCleanup = pCleanup->p_తదుపరి;
    pCleanup->xCleanup(db, pCleanup->pPtr);
    sqlite3DbNNFreeNN(db, pCleanup);
  }
  ఒకవేళ( pParse->aLabel ) sqlite3DbNNFreeNN(db, pParse->aLabel);
  ఒకవేళ( pParse->p_Const_సమాసం ){
    sqlite3_సమాసం_List_Delete(db, pParse->p_Const_సమాసం);
  }
  ప్రతిజ్ఞా( db->lookaside.bDisable >= pParse->disableLookaside );
  db->lookaside.bDisable -= pParse->disableLookaside;
  db->lookaside.sz = db->lookaside.bDisable ? 0 : db->lookaside.szTrue;
  ప్రతిజ్ఞా( pParse->db->pParse==pParse );
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
శూన్య *sqlite3_Parser_కూడు_Cleanup(
  Parse *pParse,                      /* Destroy when this Parser finishes */
  శూన్య (*xCleanup)(sqlite3*,శూన్య*),   /* The cleanup routine */
  శూన్య *pPtr                          /* Pointer to object to be cleaned up */
){
  ParseCleanup *pCleanup;
  ఒకవేళ( sqlite3FaultSim(300) ){
    pCleanup = 0;
    sqlite3OomFault(pParse->db);
  }లేకపోతే{
    pCleanup = sqlite3DbMallocRaw(pParse->db, పరిమాణం(*pCleanup));
  }
  ఒకవేళ( pCleanup ){
    pCleanup->p_తదుపరి = pParse->pCleanup;
    pParse->pCleanup = pCleanup;
    pCleanup->pPtr = pPtr;
    pCleanup->xCleanup = xCleanup;
  }లేకపోతే{
    xCleanup(pParse->db, pPtr);
    pPtr = 0;
#ఒకవేళ defined(SQLITE_DEBUG) || defined(SQLITE_COVERAGE_TEST)
    pParse->earlyCleanup = 1;
#అయితే_ముగింపు
  }
  తిరిగి pPtr;
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
శూన్య sqlite3ParseObjectInit(Parse *pParse, sqlite3 *db){
  ప్రతిజ్ఞా( db!=0 );
  memset(PARSE_HDR(pParse), 0, PARSE_HDR_SZ);
  memset(PARSE_తోక(pParse), 0, PARSE_తోక_SZ);
  ప్రతిజ్ఞా( db->pParse!=pParse );
  pParse->pOuterParse = db->pParse;
  db->pParse = pParse;
  pParse->db = db;
  ఒకవేళ( db->mallocFailed ) sqlite3_Error_సందేశం(pParse, "out of memory");
}

/*
** Maximum number of times that we will try again to prepare a statement
** that returns SQLITE_ERROR_RETRY.
*/
#నిర్వచితం_కాకపోతే SQLITE_MAX_PREPARE_RETRY
# define SQLITE_MAX_PREPARE_RETRY 25
#అయితే_ముగింపు

/*
** Compile the UTF-8 encoded SQL statement zSql into a statement handle.
*/
స్థిర పూర్ణాంకం sqlite3Prepare(
  sqlite3 *db,              /* Database handle. */
  స్థిరరాశి అక్షరం *zSql,         /* UTF-8 encoded SQL statement. */
  పూర్ణాంకం n_బైట్లు,               /* Length of zSql in bytes. */
  u32 prepFlags,            /* Zero or more SQLITE_PREPARE_* flags */
  Vdbe *pReprepare,         /* VM being reprepared */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  స్థిరరాశి అక్షరం **pz_తోక       /* OUT: End of parsed string */
){
  పూర్ణాంకం rc = SQLITE_OK;       /* Result code */
  పూర్ణాంకం i;                    /* Loop counter */
  Parse sParse;             /* Parsing context */

  /* sqlite3ParseObjectInit(&sParse, db); // inlined for performance */
  memset(PARSE_HDR(&sParse), 0, PARSE_HDR_SZ);
  memset(PARSE_తోక(&sParse), 0, PARSE_తోక_SZ);
  sParse.pOuterParse = db->pParse;
  db->pParse = &sParse;
  sParse.db = db;
  ఒకవేళ( pReprepare ){
    sParse.pReprepare = pReprepare;
    sParse.explain = sqlite3_stmt_isexplain((sqlite3_stmt*)pReprepare);
  }లేకపోతే{
    ప్రతిజ్ఞా( sParse.pReprepare==0 );
  }
  ప్రతిజ్ఞా( ppStmt && *ppStmt==0 );
  ఒకవేళ( db->mallocFailed ){
    sqlite3_Error_సందేశం(&sParse, "out of memory");
    db->err_సంకేతం = rc = SQLITE_NOMEM;
    వెళ్ళు చివర_prepare;
  }
  ప్రతిజ్ఞా( sqlite3_mutex_held(db->mutex) );

  /* For a long-term use prepared statement avoid the use of
  ** lookaside memory.
  */
  ఒకవేళ( prepFlags & SQLITE_PREPARE_PERSISTENT ){
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
  ఒకవేళ( !db->noSharedCache ){
    కొరకు(i=0; i<db->nDb; i++) {
      Btree *pBt = db->aDb[i].pBt;
      ఒకవేళ( pBt ){
        ప్రతిజ్ఞా( sqlite3BtreeHoldsMutex(pBt) );
        rc = sqlite3BtreeSchemaLocked(pBt);
        ఒకవేళ( rc ){
          స్థిరరాశి అక్షరం *zDb = db->aDb[i].zDbSName;
          sqlite3_Error_With_సందేశం(db, rc, "database schema is locked: %s", zDb);
          testcase( db->flags & SQLITE_ReadUncommit );
          వెళ్ళు చివర_prepare;
        }
      }
    }
  }

#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
  ఒకవేళ( db->pDisconnect ) sqlite3VtabUnlockList(db);
#అయితే_ముగింపు

  ఒకవేళ( n_బైట్లు>=0 && (n_బైట్లు==0 || zSql[n_బైట్లు-1]!=0) ){
    అక్షరం *zSqlCopy;
    పూర్ణాంకం mxLen = db->a_హద్దు[SQLITE_హద్దు_SQL_నిడివి];
    testcase( n_బైట్లు==mxLen );
    testcase( n_బైట్లు==mxLen+1 );
    ఒకవేళ( n_బైట్లు>mxLen ){
      sqlite3_Error_With_సందేశం(db, SQLITE_TOOBIG, "statement too long");
      rc = sqlite3ApiExit(db, SQLITE_TOOBIG);
      వెళ్ళు చివర_prepare;
    }
    zSqlCopy = sqlite3DbStrNDup(db, zSql, n_బైట్లు);
    ఒకవేళ( zSqlCopy ){
      sqlite3_నిరంతరం_Parser(&sParse, zSqlCopy);
      sParse.z_తోక = &zSql[sParse.z_తోక-zSqlCopy];
      sqlite3DbFree(db, zSqlCopy);
    }లేకపోతే{
      sParse.z_తోక = &zSql[n_బైట్లు];
    }
  }లేకపోతే{
    sqlite3_నిరంతరం_Parser(&sParse, zSql);
  }
  ప్రతిజ్ఞా( 0==sParse.nQueryLoop );

  ఒకవేళ( pz_తోక ){
    *pz_తోక = sParse.z_తోక;
  }

  ఒకవేళ( db->init.busy==0 ){
    sqlite3_Vdbe_సమితి_Sql(sParse.pVdbe, zSql, (పూర్ణాంకం)(sParse.z_తోక-zSql), prepFlags);
  }
  ఒకవేళ( db->mallocFailed ){
    sParse.rc = SQLITE_NOMEM_BKPT;
    sParse.checkSchema = 0;
  }
  ఒకవేళ( sParse.rc!=SQLITE_OK && sParse.rc!=SQLITE_పూర్తి ){
    ఒకవేళ( sParse.checkSchema && db->init.busy==0 ){
      schemaIsValid(&sParse);
    }
    ఒకవేళ( sParse.pVdbe ){
      sqlite3VdbeFinalize(sParse.pVdbe);
    }
    ప్రతిజ్ఞా( 0==(*ppStmt) );
    rc = sParse.rc;
    ఒకవేళ( sParse.z_Err_సందేశం ){
      sqlite3_Error_With_సందేశం(db, rc, "%s", sParse.z_Err_సందేశం);
      sqlite3DbFree(db, sParse.z_Err_సందేశం);
    }లేకపోతే{
      sqlite3Error(db, rc);
    }
  }లేకపోతే{
    ప్రతిజ్ఞా( sParse.z_Err_సందేశం==0 );
    *ppStmt = (sqlite3_stmt*)sParse.pVdbe;
    rc = SQLITE_OK;
    sqlite3ErrorClear(db);
  }


  /* Delete any TriggerPrg structures allocated while parsing this statement. */
  అయితే( sParse.pTriggerPrg ){
    TriggerPrg *pT = sParse.pTriggerPrg;
    sParse.pTriggerPrg = pT->p_తదుపరి;
    sqlite3DbFree(db, pT);
  }

చివర_prepare:

  sqlite3ParseObjectReset(&sParse);
  తిరిగి rc;
}
స్థిర పూర్ణాంకం sqlite3_తాళం_And_Prepare(
  sqlite3 *db,              /* Database handle. */
  స్థిరరాశి అక్షరం *zSql,         /* UTF-8 encoded SQL statement. */
  పూర్ణాంకం n_బైట్లు,               /* Length of zSql in bytes. */
  u32 prepFlags,            /* Zero or more SQLITE_PREPARE_* flags */
  Vdbe *pOld,               /* VM being reprepared */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  స్థిరరాశి అక్షరం **pz_తోక       /* OUT: End of parsed string */
){
  పూర్ణాంకం rc;
  పూర్ణాంకం cnt = 0;

#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( ppStmt==0 ) తిరిగి SQLITE_MISUSE_BKPT;
#అయితే_ముగింపు
  *ppStmt = 0;
  ఒకవేళ( !sqlite3SafetyCheckOk(db)||zSql==0 ){
    తిరిగి SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(db->mutex);
  sqlite3BtreeEnterAll(db);
  చేయి{
    /* Make multiple attempts to compile the SQL, until it either succeeds
    ** or encounters a permanent error.  A schema problem after one schema
    ** reset is considered a permanent error. */
    rc = sqlite3Prepare(db, zSql, n_బైట్లు, prepFlags, pOld, ppStmt, pz_తోక);
    ప్రతిజ్ఞా( rc==SQLITE_OK || *ppStmt==0 );
    ఒకవేళ( rc==SQLITE_OK || db->mallocFailed ) ఆపు;
    cnt++;
  }అయితే( (rc==SQLITE_ERROR_RETRY && ALWAYS(cnt<=SQLITE_MAX_PREPARE_RETRY))
       || (rc==SQLITE_SCHEMA && (sqlite3ResetOneSchema(db,-1), cnt)==1) );
  sqlite3BtreeLeaveAll(db);
  ప్రతిజ్ఞా( rc!=SQLITE_ERROR_RETRY );
  rc = sqlite3ApiExit(db, rc);
  ప్రతిజ్ఞా( (rc&db->err_ముసుగు)==rc );
  db->busyHandler.nBusy = 0;
  sqlite3_mutex_leave(db->mutex);
  ప్రతిజ్ఞా( rc==SQLITE_OK || (*ppStmt)==0 );
  తిరిగి rc;
}


/*
** Rerun the compilation of a statement after a schema change.
**
** If the statement is successfully recompiled, return SQLITE_OK. Otherwise,
** if the statement cannot be recompiled because another connection has
** locked the sqlite3_schema table, return SQLITE_LOCKED. If any other error
** occurs, return SQLITE_SCHEMA.
*/
పూర్ణాంకం sqlite3Reprepare(Vdbe *p){
  పూర్ణాంకం rc;
  sqlite3_stmt *pNew;
  స్థిరరాశి అక్షరం *zSql;
  sqlite3 *db;
  u8 prepFlags;

  ప్రతిజ్ఞా( sqlite3_mutex_held(sqlite3VdbeDb(p)->mutex) );
  zSql = sqlite3_sql((sqlite3_stmt *)p);
  ప్రతిజ్ఞా( zSql!=0 );  /* Reprepare only called for prepare_v2() statements */
  db = sqlite3VdbeDb(p);
  ప్రతిజ్ఞా( sqlite3_mutex_held(db->mutex) );
  prepFlags = sqlite3VdbePrepareFlags(p);
  rc = sqlite3_తాళం_And_Prepare(db, zSql, -1, prepFlags, p, &pNew, 0);
  ఒకవేళ( rc ){
    ఒకవేళ( rc==SQLITE_NOMEM ){
      sqlite3OomFault(db);
    }
    ప్రతిజ్ఞా( pNew==0 );
    తిరిగి rc;
  }లేకపోతే{
    ప్రతిజ్ఞా( pNew!=0 );
  }
  sqlite3_Vdbe_మార్పిడి((Vdbe*)pNew, p);
  sqlite3TransferBindings(pNew, (sqlite3_stmt*)p);
  sqlite3_Vdbe_Reset_అడుగు_ఫలితం((Vdbe*)pNew);
  sqlite3VdbeFinalize((Vdbe*)pNew);
  తిరిగి SQLITE_OK;
}


/*
** Two versions of the official API.  Legacy and new use.  In the legacy
** version, the original SQL text is not saved in the prepared statement
** and so if a schema change occurs, SQLITE_SCHEMA is returned by
** sqlite3_step().  In the new version, the original SQL text is retained
** and the statement is automatically recompiled if an schema change
** occurs.
*/
పూర్ణాంకం sqlite3_prepare(
  sqlite3 *db,              /* Database handle. */
  స్థిరరాశి అక్షరం *zSql,         /* UTF-8 encoded SQL statement. */
  పూర్ణాంకం n_బైట్లు,               /* Length of zSql in bytes. */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  స్థిరరాశి అక్షరం **pz_తోక       /* OUT: End of parsed string */
){
  పూర్ణాంకం rc;
  rc = sqlite3_తాళం_And_Prepare(db,zSql,n_బైట్లు,0,0,ppStmt,pz_తోక);
  ప్రతిజ్ఞా( rc==SQLITE_OK || ppStmt==0 || *ppStmt==0 );  /* VERIFY: F13021 */
  తిరిగి rc;
}
పూర్ణాంకం sqlite3_prepare_v2(
  sqlite3 *db,              /* Database handle. */
  స్థిరరాశి అక్షరం *zSql,         /* UTF-8 encoded SQL statement. */
  పూర్ణాంకం n_బైట్లు,               /* Length of zSql in bytes. */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  స్థిరరాశి అక్షరం **pz_తోక       /* OUT: End of parsed string */
){
  పూర్ణాంకం rc;
  /* EVIDENCE-OF: R-37923-12173 The sqlite3_prepare_v2() interface works
  ** exactly the same as sqlite3_prepare_v3() with a zero prepFlags
  ** parameter.
  **
  ** Proof in that the 5th parameter to sqlite3LockAndPrepare is 0 */
  rc = sqlite3_తాళం_And_Prepare(db,zSql,n_బైట్లు,SQLITE_PREPARE_SAVESQL,0,
                             ppStmt,pz_తోక);
  ప్రతిజ్ఞా( rc==SQLITE_OK || ppStmt==0 || *ppStmt==0 );
  తిరిగి rc;
}
పూర్ణాంకం sqlite3_prepare_v3(
  sqlite3 *db,              /* Database handle. */
  స్థిరరాశి అక్షరం *zSql,         /* UTF-8 encoded SQL statement. */
  పూర్ణాంకం n_బైట్లు,               /* Length of zSql in bytes. */
  గుర్తులేని పూర్ణాంకం prepFlags,   /* Zero or more SQLITE_PREPARE_* flags */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  స్థిరరాశి అక్షరం **pz_తోక       /* OUT: End of parsed string */
){
  పూర్ణాంకం rc;
  /* EVIDENCE-OF: R-56861-42673 sqlite3_prepare_v3() differs from
  ** sqlite3_prepare_v2() only in having the extra prepFlags parameter,
  ** which is a bit array consisting of zero or more of the
  ** SQLITE_PREPARE_* flags.
  **
  ** Proof by comparison to the implementation of sqlite3_prepare_v2()
  ** directly above. */
  rc = sqlite3_తాళం_And_Prepare(db,zSql,n_బైట్లు,
                 SQLITE_PREPARE_SAVESQL|(prepFlags&SQLITE_PREPARE_ముసుగు),
                 0,ppStmt,pz_తోక);
  ప్రతిజ్ఞా( rc==SQLITE_OK || ppStmt==0 || *ppStmt==0 );
  తిరిగి rc;
}


#నిర్వచితం_కాకపోతే SQLITE_OMIT_UTF16
/*
** Compile the UTF-16 encoded SQL statement zSql into a statement handle.
*/
స్థిర పూర్ణాంకం sqlite3Prepare16(
  sqlite3 *db,              /* Database handle. */ 
  స్థిరరాశి శూన్య *zSql,         /* UTF-16 encoded SQL statement. */
  పూర్ణాంకం n_బైట్లు,               /* Length of zSql in bytes. */
  u32 prepFlags,            /* Zero or more SQLITE_PREPARE_* flags */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  స్థిరరాశి శూన్య **pz_తోక       /* OUT: End of parsed string */
){
  /* This function currently works by first transforming the UTF-16
  ** encoded string to UTF-8, then invoking sqlite3_prepare(). The
  ** tricky bit is figuring out the pointer to return in *pzTail.
  */
  అక్షరం *zSql8;
  స్థిరరాశి అక్షరం *zTail8 = 0;
  పూర్ణాంకం rc = SQLITE_OK;

#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( ppStmt==0 ) తిరిగి SQLITE_MISUSE_BKPT;
#అయితే_ముగింపు
  *ppStmt = 0;
  ఒకవేళ( !sqlite3SafetyCheckOk(db)||zSql==0 ){
    తిరిగి SQLITE_MISUSE_BKPT;
  }

  /* Make sure nBytes is non-negative and correct.  It should be the
  ** number of bytes until the end of the input buffer or until the first
  ** U+0000 character.  If the input nBytes is odd, convert it into
  ** an even number.  If the input nBytes is negative, then the input
  ** must be terminated by at least one U+0000 character */
  ఒకవేళ( n_బైట్లు>=0 ){
    పూర్ణాంకం sz;
    స్థిరరాశి అక్షరం *z = (స్థిరరాశి అక్షరం*)zSql;
    కొరకు(sz=0; sz<n_బైట్లు && (z[sz]!=0 || z[sz+1]!=0); sz += 2){}
    n_బైట్లు = sz;
  }లేకపోతే{
    పూర్ణాంకం sz;
    స్థిరరాశి అక్షరం *z = (స్థిరరాశి అక్షరం*)zSql;
    కొరకు(sz=0; z[sz]!=0 || z[sz+1]!=0; sz += 2){}
    n_బైట్లు = sz;
  }

  sqlite3_mutex_enter(db->mutex);
  zSql8 = sqlite3Utf16to8(db, zSql, n_బైట్లు, SQLITE_UTF16NATIVE);
  ఒకవేళ( zSql8 ){
    rc = sqlite3_తాళం_And_Prepare(db, zSql8, -1, prepFlags, 0, ppStmt, &zTail8);
  }

  ఒకవేళ( zTail8 && pz_తోక ){
    /* If sqlite3_prepare returns a tail pointer, we calculate the
    ** equivalent pointer into the UTF-16 string by counting the unicode
    ** characters between zSql8 and zTail8, and then returning a pointer
    ** the same number of characters into the UTF-16 string.
    */
    పూర్ణాంకం chars_parsed = sqlite3Utf8CharLen(zSql8, (పూర్ణాంకం)(zTail8-zSql8));
    *pz_తోక = (u8 *)zSql + sqlite3Utf16ByteLen(zSql, n_బైట్లు, chars_parsed);
  }
  sqlite3DbFree(db, zSql8); 
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  తిరిగి rc;
}

/*
** Two versions of the official API.  Legacy and new use.  In the legacy
** version, the original SQL text is not saved in the prepared statement
** and so if a schema change occurs, SQLITE_SCHEMA is returned by
** sqlite3_step().  In the new version, the original SQL text is retained
** and the statement is automatically recompiled if an schema change
** occurs.
*/
పూర్ణాంకం sqlite3_prepare16(
  sqlite3 *db,              /* Database handle. */ 
  స్థిరరాశి శూన్య *zSql,         /* UTF-16 encoded SQL statement. */
  పూర్ణాంకం n_బైట్లు,               /* Length of zSql in bytes. */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  స్థిరరాశి శూన్య **pz_తోక       /* OUT: End of parsed string */
){
  పూర్ణాంకం rc;
  rc = sqlite3Prepare16(db,zSql,n_బైట్లు&~1,0,ppStmt,pz_తోక);
  ప్రతిజ్ఞా( rc==SQLITE_OK || ppStmt==0 || *ppStmt==0 );  /* VERIFY: F13021 */
  తిరిగి rc;
}
పూర్ణాంకం sqlite3_prepare16_v2(
  sqlite3 *db,              /* Database handle. */ 
  స్థిరరాశి శూన్య *zSql,         /* UTF-16 encoded SQL statement. */
  పూర్ణాంకం n_బైట్లు,               /* Length of zSql in bytes. */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  స్థిరరాశి శూన్య **pz_తోక       /* OUT: End of parsed string */
){
  పూర్ణాంకం rc;
  rc = sqlite3Prepare16(db,zSql,n_బైట్లు&~1,SQLITE_PREPARE_SAVESQL,ppStmt,pz_తోక);
  ప్రతిజ్ఞా( rc==SQLITE_OK || ppStmt==0 || *ppStmt==0 );  /* VERIFY: F13021 */
  తిరిగి rc;
}
పూర్ణాంకం sqlite3_prepare16_v3(
  sqlite3 *db,              /* Database handle. */ 
  స్థిరరాశి శూన్య *zSql,         /* UTF-16 encoded SQL statement. */
  పూర్ణాంకం n_బైట్లు,               /* Length of zSql in bytes. */
  గుర్తులేని పూర్ణాంకం prepFlags,   /* Zero or more SQLITE_PREPARE_* flags */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  స్థిరరాశి శూన్య **pz_తోక       /* OUT: End of parsed string */
){
  పూర్ణాంకం rc;
  rc = sqlite3Prepare16(db,zSql,n_బైట్లు&~1,
         SQLITE_PREPARE_SAVESQL|(prepFlags&SQLITE_PREPARE_ముసుగు),
         ppStmt,pz_తోక);
  ప్రతిజ్ఞా( rc==SQLITE_OK || ppStmt==0 || *ppStmt==0 );  /* VERIFY: F13021 */
  తిరిగి rc;
}

#అయితే_ముగింపు /* SQLITE_OMIT_UTF16 */
