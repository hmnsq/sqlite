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
** This file contains code used to implement the ATTACH and DETACH commands.
*/
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_ATTACH
/*
** Resolve an expression that was part of an ATTACH or DETACH statement. This
** is slightly different from resolving a normal SQL expression, because simple
** identifiers are treated as strings, not possible column names or aliases.
**
** i.e. if the parser sees:
**
**     ATTACH DATABASE abc AS def
**
** it treats the two expressions as literal strings 'abc' and 'def' instead of
** looking for columns of the same name.
**
** This only applies to the root node of pExpr, so the statement:
**
**     ATTACH DATABASE abc||def AS 'db2'
**
** will fail because neither abc or def can be resolved.
*/
ਸਥਿਰ ਪੂਰਨ resolve_Attach_ਵਿਅੰਜਕ(NameContext *pName, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ)
{
  ਪੂਰਨ rc = SQLITE_OK;
  ਜੇ( p_ਵਿਅੰਜਕ ){
    ਜੇ( p_ਵਿਅੰਜਕ->op!=TK_ID ){
      rc = sqlite3_Resolve_ਵਿਅੰਜਕ_ਨਾਂ(pName, p_ਵਿਅੰਜਕ);
    }ਨਹੀਂਤਾਂ{
      p_ਵਿਅੰਜਕ->op = TK_STRING;
    }
  }
  ਵਾਪਸ rc;
}

/*
** Return true if zName points to a name that may be used to refer to
** database iDb attached to handle db.
*/
ਪੂਰਨ sqlite3DbIsNamed(sqlite3 *db, ਪੂਰਨ iDb, ਅਚਲ ਅੱਖਰ *zName){
  ਵਾਪਸ (
      sqlite3StrICmp(db->aDb[iDb].zDbSName, zName)==0
   || (iDb==0 && sqlite3StrICmp("main", zName)==0)
  );
}

/*
** If any TEMP triggers reference schema pSchema, move those triggers to
** reference the TEMP schema itself.
*/
ਸਥਿਰ ਖਾਲੀ detach_ਆਰਜ਼ੀ_Triggers(sqlite3 *db, Schema *pSchema){
  HashElem *p_ਇੰਦਰਾਜ;
  ਪ੍ਰਤਿਜ੍ਞਾ( db->aDb[1].pSchema );
  p_ਇੰਦਰਾਜ = sqlite_Hash_ਪਹਿਲਾ(&db->aDb[1].pSchema->trigHash);
  ਜਦੋਂ( p_ਇੰਦਰਾਜ ){
    Trigger *pTrig = (Trigger*)sqlite_Hash_ਅੰਕੜੇ(p_ਇੰਦਰਾਜ);
    ਜੇ( pTrig->pTabSchema==pSchema ){
      pTrig->pTabSchema = pTrig->pSchema;
    }
    p_ਇੰਦਰਾਜ = sqlite_Hash_ਅਗਲਾ(p_ਇੰਦਰਾਜ);
  }
}

/*
** An SQL user-function registered to do the work of an ATTACH statement. The
** three arguments to the function come directly from an attach statement:
**
**     ATTACH DATABASE x AS y KEY z
**
**     SELECT sqlite_attach(x, y, z)
**
** If the optional "KEY z" syntax is omitted, an SQL NULL is passed as the
** third argument.
**
** If the db->init.reopenMemdb flags is set, then instead of attaching a
** new database, close the database on db->init.iDb and reopen it as an
** empty MemDB.
*/
ਸਥਿਰ ਖਾਲੀ attachFunc(
  sqlite3_context *context,
  ਪੂਰਨ Not_ਵਰਤਿਆ,
  sqlite3_ਮੁੱਲ **argv
){
  ਪੂਰਨ i;
  ਪੂਰਨ rc = 0;
  sqlite3 *db = sqlite3_context_db_handle(context);
  ਅਚਲ ਅੱਖਰ *zName;
  ਅਚਲ ਅੱਖਰ *zFile;
  ਅੱਖਰ *z_ਰਸਤਾ = 0;
  ਅੱਖਰ *zErr = 0;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ flags;
  Db *aNew;                 /* New array of Db pointers */
  Db *pNew = 0;             /* Db object for the newly attached database */
  ਅੱਖਰ *zErrDyn = 0;
  sqlite3_vfs *pVfs;

  ਅਣਵਰਤਿਆ_PARAMETER(Not_ਵਰਤਿਆ);
  zFile = (ਅਚਲ ਅੱਖਰ *)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[0]);
  zName = (ਅਚਲ ਅੱਖਰ *)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[1]);
  ਜੇ( zFile==0 ) zFile = "";
  ਜੇ( zName==0 ) zName = "";

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_DESERIALIZE
# define REOPEN_AS_MEMDB(db)  (db->init.reopenMemdb)
#ਨਹੀਂਤਾਂ
# define REOPEN_AS_MEMDB(db)  (0)
#ਜੇ_ਅੰਤ

  ਜੇ( REOPEN_AS_MEMDB(db) ){
    /* This is not a real ATTACH.  Instead, this routine is being called
    ** from sqlite3_deserialize() to close database db->init.iDb and
    ** reopen it as a MemDB */
    Btree *pNewBt = 0;

    pNew = &db->aDb[db->init.iDb];
    ਪ੍ਰਤਿਜ੍ਞਾ( pNew->pBt!=0 );
    ਜੇ( sqlite3BtreeTxnState(pNew->pBt)!=SQLITE_TXN_NONE
     || sqlite3_Btree_Is_ਵਿੱਚ_Backup(pNew->pBt)
    ){
      rc = SQLITE_BUSY;
      ਜਾਓ attach_error;
    }

    pVfs = sqlite3_vfs_ਲੱਭੋ("memdb");
    ਜੇ( pVfs==0 ) ਵਾਪਸ;
    rc = sqlite3BtreeOpen(pVfs, "x\0", db, &pNewBt, 0, SQLITE_OPEN_MAIN_DB);
    ਜੇ( rc==SQLITE_OK ){
      Schema *pNewSchema = sqlite3SchemaGet(db, pNewBt);
      ਜੇ( pNewSchema ){
        /* Both the Btree and the new Schema were allocated successfully.
        ** Close the old db and update the aDb[] slot with the new memdb
        ** values.  */
        detach_ਆਰਜ਼ੀ_Triggers(db, pNew->pSchema);
        sqlite3BtreeClose(pNew->pBt);
        pNew->pBt = pNewBt;
        pNew->pSchema = pNewSchema;
        ਜੇ( db->init.iDb==0 ){
          /* Clear all eponymous virtual table instances, as they are holding
          ** pointers to the schema object just freed by sqlite3BtreeClose() */
          sqlite3_Vtab_Eponymous_ਸਾਰਣੀ_Clear_All(db);
        }
        sqlite3ExpirePreparedStatements(db, 1);
      }ਨਹੀਂਤਾਂ{
        sqlite3BtreeClose(pNewBt);
        rc = SQLITE_NOMEM;
      }
    }
    ਜੇ( rc ) ਜਾਓ attach_error;
  }ਨਹੀਂਤਾਂ{
    /* This is a real ATTACH
    **
    ** Check for the following errors:
    **
    **     * Too many attached databases,
    **     * Transaction currently open
    **     * Specified database name already being used.
    */
    ਜੇ( db->nDb>=db->a_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_ATTACHED]+2 ){
      zErrDyn = sqlite3MPrintf(db, "too many attached databases - max %d", 
        db->a_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_ATTACHED]
      );
      ਜਾਓ attach_error;
    }
    ਲਈ(i=0; i<db->nDb; i++){
      ਪ੍ਰਤਿਜ੍ਞਾ( zName );
      ਜੇ( sqlite3DbIsNamed(db, i, zName) ){
        zErrDyn = sqlite3MPrintf(db, "database %s is already in use", zName);
        ਜਾਓ attach_error;
      }
    }
  
    /* Allocate the new entry in the db->aDb[] array and initialize the schema
    ** hash tables.
    */
    ਜੇ( db->aDb==db->aDbStatic ){
      aNew = sqlite3DbMallocRawNN(db, ਆਕਾਰ(db->aDb[0])*3 );
      ਜੇ( aNew==0 ) ਵਾਪਸ;
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(aNew, db->aDb, ਆਕਾਰ(db->aDb[0])*2);
    }ਨਹੀਂਤਾਂ{
      aNew = sqlite3DbRealloc(db, db->aDb, ਆਕਾਰ(db->aDb[0])*(1+(i64)db->nDb));
      ਜੇ( aNew==0 ) ਵਾਪਸ;
    }
    db->aDb = aNew;
    pNew = &db->aDb[db->nDb];
    memset(pNew, 0, ਆਕਾਰ(*pNew));
  
    /* Open the database file. If the btree is successfully opened, use
    ** it to obtain the database schema. At this point the schema may
    ** or may not be initialized.
    */
    flags = db->openFlags;
    rc = sqlite3ParseUri(db->pVfs->zName, zFile, &flags, &pVfs, &z_ਰਸਤਾ, &zErr);
    ਜੇ( rc!=SQLITE_OK ){
      ਜੇ( rc==SQLITE_NOMEM ) sqlite3OomFault(db);
      sqlite3_ਨਤੀਜਾ_error(context, zErr, -1);
      sqlite3_free(zErr);
      ਵਾਪਸ;
    }
    ਜੇ( (db->flags & SQLITE_AttachWrite)==0 ){
      flags &= ~(SQLITE_OPEN_CREATE|SQLITE_OPEN_READWRITE);
      flags |= SQLITE_OPEN_READONLY;
    }ਨਹੀਂਤਾਂ ਜੇ( (db->flags & SQLITE_AttachCreate)==0 ){
      flags &= ~SQLITE_OPEN_CREATE;
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( pVfs );
    flags |= SQLITE_OPEN_MAIN_DB;
    rc = sqlite3BtreeOpen(pVfs, z_ਰਸਤਾ, db, &pNew->pBt, 0, flags);
    db->nDb++;
    pNew->zDbSName = sqlite3DbStrDup(db, zName);
  }
  db->noSharedCache = 0;
  ਜੇ( rc==SQLITE_CONSTRAINT ){
    rc = SQLITE_ERROR;
    zErrDyn = sqlite3MPrintf(db, "database is already attached");
  }ਨਹੀਂਤਾਂ ਜੇ( rc==SQLITE_OK ){
    Pager *pPager;
    pNew->pSchema = sqlite3SchemaGet(db, pNew->pBt);
    ਜੇ( !pNew->pSchema ){
      rc = SQLITE_NOMEM_BKPT;
    }ਨਹੀਂਤਾਂ ਜੇ( pNew->pSchema->file_format && pNew->pSchema->enc!=ENC(db) ){
      zErrDyn = sqlite3MPrintf(db, 
        "attached databases must use the same text encoding as main database");
      rc = SQLITE_ERROR;
    }
    sqlite3BtreeEnter(pNew->pBt);
    pPager = sqlite3BtreePager(pNew->pBt);
    sqlite3_Pager_Locking_ਬਹੁਲਕ(pPager, db->dflt_ਤਾਲਾ_ਬਹੁਲਕ);
    sqlite3BtreeSecureDelete(pNew->pBt,
                             sqlite3BtreeSecureDelete(db->aDb[0].pBt,-1) );
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_PAGER_PRAGMAS
    sqlite3_Btree_ਸੈੱਟ_Pager_Flags(pNew->pBt,
                      PAGER_SYNCHRONOUS_ਭਰਿਆ | (db->flags & PAGER_FLAGS_ਨਕਾਬ));
#ਜੇ_ਅੰਤ
    sqlite3BtreeLeave(pNew->pBt);
  }
  pNew->safety_level = SQLITE_DEFAULT_SYNCHRONOUS+1;
  ਜੇ( rc==SQLITE_OK && pNew->zDbSName==0 ){
    rc = SQLITE_NOMEM_BKPT;
  }
  sqlite3_free_filename( z_ਰਸਤਾ );

  /* If the file was opened successfully, read the schema for the new database.
  ** If this fails, or if opening the file failed, then close the file and 
  ** remove the entry from the db->aDb[] array. i.e. put everything back the
  ** way we found it.
  */
  ਜੇ( rc==SQLITE_OK ){
    sqlite3BtreeEnterAll(db);
    db->init.iDb = 0;
    db->mDbFlags &= ~(DBFLAG_Schema_ਗਿਆਤ_Ok);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_SETLK_TIMEOUT
    ਜੇ( db->setlkFlags & SQLITE_SETLK_ਖੰਡ_ON_CONNECT ){
      ਪੂਰਨ val = 1;
      sqlite3_file *fd = sqlite3PagerFile(sqlite3BtreePager(pNew->pBt));
      sqlite3OsFileControlHint(fd, SQLITE_FCNTL_ਖੰਡ_ON_CONNECT, &val);
    }
#ਜੇ_ਅੰਤ
    ਜੇ( !REOPEN_AS_MEMDB(db) ){
      rc = sqlite3Init(db, &zErrDyn);
    }
    sqlite3BtreeLeaveAll(db);
    ਪ੍ਰਤਿਜ੍ਞਾ( zErrDyn==0 || rc!=SQLITE_OK );
  }
  ਜੇ( rc ){
    ਜੇ( ALWAYS(!REOPEN_AS_MEMDB(db)) ){
      ਪੂਰਨ iDb = db->nDb - 1;
      ਪ੍ਰਤਿਜ੍ਞਾ( iDb>=2 );
      ਜੇ( db->aDb[iDb].pBt ){
        sqlite3BtreeClose(db->aDb[iDb].pBt);
        db->aDb[iDb].pBt = 0;
        db->aDb[iDb].pSchema = 0;
      }
      sqlite3_Reset_All_Schemas_ਦਾ_Connection(db);
      db->nDb = iDb;
      ਜੇ( rc==SQLITE_NOMEM || rc==SQLITE_IOERR_NOMEM ){
        sqlite3OomFault(db);
        sqlite3DbFree(db, zErrDyn);
        zErrDyn = sqlite3MPrintf(db, "out of memory");
      }ਨਹੀਂਤਾਂ ਜੇ( zErrDyn==0 ){
        zErrDyn = sqlite3MPrintf(db, "unable to open database: %s", zFile);
      }
    }
    ਜਾਓ attach_error;
  }
  
  ਵਾਪਸ;

attach_error:
  /* Return an error if we get here */
  ਜੇ( zErrDyn ){
    sqlite3_ਨਤੀਜਾ_error(context, zErrDyn, -1);
    sqlite3DbFree(db, zErrDyn);
  }
  ਜੇ( rc ) sqlite3_ਨਤੀਜਾ_error_ਕੋਡ(context, rc);
}

/*
** An SQL user-function registered to do the work of an DETACH statement. The
** three arguments to the function come directly from a detach statement:
**
**     DETACH DATABASE x
**
**     SELECT sqlite_detach(x)
*/
ਸਥਿਰ ਖਾਲੀ detachFunc(
  sqlite3_context *context,
  ਪੂਰਨ Not_ਵਰਤਿਆ,
  sqlite3_ਮੁੱਲ **argv
){
  ਅਚਲ ਅੱਖਰ *zName = (ਅਚਲ ਅੱਖਰ *)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[0]);
  sqlite3 *db = sqlite3_context_db_handle(context);
  ਪੂਰਨ i;
  Db *pDb = 0;
  ਅੱਖਰ zErr[128];

  ਅਣਵਰਤਿਆ_PARAMETER(Not_ਵਰਤਿਆ);

  ਜੇ( zName==0 ) zName = "";
  ਲਈ(i=0; i<db->nDb; i++){
    pDb = &db->aDb[i];
    ਜੇ( pDb->pBt==0 ) ਜਾਰੀ;
    ਜੇ( sqlite3DbIsNamed(db, i, zName) ) ਰੋਕੋ;
  }

  ਜੇ( i>=db->nDb ){
    sqlite3_snprintf(ਆਕਾਰ(zErr),zErr, "no such database: %s", zName);
    ਜਾਓ detach_error;
  }
  ਜੇ( i<2 ){
    sqlite3_snprintf(ਆਕਾਰ(zErr),zErr, "cannot detach database %s", zName);
    ਜਾਓ detach_error;
  }
  ਜੇ( sqlite3BtreeTxnState(pDb->pBt)!=SQLITE_TXN_NONE
   || sqlite3_Btree_Is_ਵਿੱਚ_Backup(pDb->pBt)
  ){
    sqlite3_snprintf(ਆਕਾਰ(zErr),zErr, "database %s is locked", zName);
    ਜਾਓ detach_error;
  }

  detach_ਆਰਜ਼ੀ_Triggers(db, pDb->pSchema);
  sqlite3BtreeClose(pDb->pBt);
  pDb->pBt = 0;
  pDb->pSchema = 0;
  sqlite3CollapseDatabaseArray(db);
  ਵਾਪਸ;

detach_error:
  sqlite3_ਨਤੀਜਾ_error(context, zErr, -1);
}

/*
** This procedure generates VDBE code for a single invocation of either the
** sqlite_detach() or sqlite_attach() SQL user functions.
*/
ਸਥਿਰ ਖਾਲੀ ਕੋਡ_Attach(
  Parse *pParse,       /* The parser context */
  ਪੂਰਨ type,            /* Either SQLITE_ATTACH or SQLITE_DETACH */
  FuncDef ਅਚਲ *pFunc,/* FuncDef wrapper for detachFunc() or attachFunc() */
  ਵਿਅੰਜਕ *pAuthArg,      /* Expression to pass to authorization callback */
  ਵਿਅੰਜਕ *pFilename,     /* Name of database file */
  ਵਿਅੰਜਕ *pDbname,       /* Name of the database to use internally */
  ਵਿਅੰਜਕ *p_ਕੁੰਜੀ           /* Database key for encryption extension */
){
  ਪੂਰਨ rc;
  NameContext sName;
  Vdbe *v;
  sqlite3* db = pParse->db;
  ਪੂਰਨ reg_ਦਲੀਲਾਂ;

  ਜੇ( SQLITE_OK!=sqlite3ReadSchema(pParse) ) ਜਾਓ attach_ਸਿਰਾ;

  ਜੇ( pParse->nErr ) ਜਾਓ attach_ਸਿਰਾ;
  memset(&sName, 0, ਆਕਾਰ(NameContext));
  sName.pParse = pParse;

  ਜੇ( 
      SQLITE_OK!=resolve_Attach_ਵਿਅੰਜਕ(&sName, pFilename) ||
      SQLITE_OK!=resolve_Attach_ਵਿਅੰਜਕ(&sName, pDbname) ||
      SQLITE_OK!=resolve_Attach_ਵਿਅੰਜਕ(&sName, p_ਕੁੰਜੀ)
  ){
    ਜਾਓ attach_ਸਿਰਾ;
  }

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTHORIZATION
  ਜੇ( ALWAYS(pAuthArg) ){
    ਅੱਖਰ *zAuthArg;
    ਜੇ( pAuthArg->op==TK_STRING ){
      ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(pAuthArg, EP_Int_ਮੁੱਲ) );
      zAuthArg = pAuthArg->u.zToken;
    }ਨਹੀਂਤਾਂ{
      zAuthArg = 0;
    }
    rc = sqlite3AuthCheck(pParse, type, zAuthArg, 0, 0);
    ਜੇ(rc!=SQLITE_OK ){
      ਜਾਓ attach_ਸਿਰਾ;
    }
  }
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_AUTHORIZATION */


  v = sqlite3GetVdbe(pParse);
  reg_ਦਲੀਲਾਂ = sqlite3_Get_ਆਰਜ਼ੀ_Range(pParse, 4);
  sqlite3_ਵਿਅੰਜਕ_ਕੋਡ(pParse, pFilename, reg_ਦਲੀਲਾਂ);
  sqlite3_ਵਿਅੰਜਕ_ਕੋਡ(pParse, pDbname, reg_ਦਲੀਲਾਂ+1);
  sqlite3_ਵਿਅੰਜਕ_ਕੋਡ(pParse, p_ਕੁੰਜੀ, reg_ਦਲੀਲਾਂ+2);

  ਪ੍ਰਤਿਜ੍ਞਾ( v || db->mallocFailed );
  ਜੇ( v ){
    sqlite3_Vdbe_ਜੋੜੋ_Function_Call(pParse, 0, reg_ਦਲੀਲਾਂ+3-pFunc->nArg, reg_ਦਲੀਲਾਂ+3,
                               pFunc->nArg, pFunc, 0);
    /* Code an OP_Expire. For an ATTACH statement, set P1 to true (expire this
    ** statement only). For DETACH, set it to false (expire all existing
    ** statements).
    */
    sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_Expire, (type==SQLITE_ATTACH));
  }
  
attach_ਸਿਰਾ:
  sqlite3_ਵਿਅੰਜਕ_Delete(db, pFilename);
  sqlite3_ਵਿਅੰਜਕ_Delete(db, pDbname);
  sqlite3_ਵਿਅੰਜਕ_Delete(db, p_ਕੁੰਜੀ);
}

/*
** Called by the parser to compile a DETACH statement.
**
**     DETACH pDbname
*/
ਖਾਲੀ sqlite3Detach(Parse *pParse, ਵਿਅੰਜਕ *pDbname){
  ਸਥਿਰ ਅਚਲ FuncDef detach_func = {
    1,                /* nArg */
    SQLITE_UTF8,      /* funcFlags */
    0,                /* pUserData */
    0,                /* pNext */
    detachFunc,       /* xSFunc */
    0,                /* xFinalize */
    0, 0,             /* xValue, xInverse */
    "sqlite_detach",  /* zName */
    {0}
  };
  ਕੋਡ_Attach(pParse, SQLITE_DETACH, &detach_func, pDbname, 0, 0, pDbname);
}

/*
** Called by the parser to compile an ATTACH statement.
**
**     ATTACH p AS pDbname KEY pKey
*/
ਖਾਲੀ sqlite3Attach(Parse *pParse, ਵਿਅੰਜਕ *p, ਵਿਅੰਜਕ *pDbname, ਵਿਅੰਜਕ *p_ਕੁੰਜੀ){
  ਸਥਿਰ ਅਚਲ FuncDef attach_func = {
    3,                /* nArg */
    SQLITE_UTF8,      /* funcFlags */
    0,                /* pUserData */
    0,                /* pNext */
    attachFunc,       /* xSFunc */
    0,                /* xFinalize */
    0, 0,             /* xValue, xInverse */
    "sqlite_attach",  /* zName */
    {0}
  };
  ਕੋਡ_Attach(pParse, SQLITE_ATTACH, &attach_func, p, p, pDbname, p_ਕੁੰਜੀ);
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_ATTACH */

/*
** Expression callback used by sqlite3FixAAAA() routines.
*/
ਸਥਿਰ ਪੂਰਨ fix_ਵਿਅੰਜਕ_Cb(Walker *p, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  DbFixer *pFix = p->u.pFix;
  ਜੇ( !pFix->b_ਆਰਜ਼ੀ ) ਵਿਅੰਜਕ_ਸੈੱਟ_Property(p_ਵਿਅੰਜਕ, EP_FromDDL);
  ਜੇ( p_ਵਿਅੰਜਕ->op==TK_VARIABLE ){
    ਜੇ( pFix->pParse->db->init.busy ){
      p_ਵਿਅੰਜਕ->op = TK_NULL;
    }ਨਹੀਂਤਾਂ{
      sqlite3_Error_ਸੁਨੇਹਾ(pFix->pParse, "%s cannot use variables", pFix->zType);
      ਵਾਪਸ WRC_Abort;
    }
  }
  ਵਾਪਸ WRC_Continue;
}

/*
** Select callback used by sqlite3FixAAAA() routines.
*/
ਸਥਿਰ ਪੂਰਨ fixSelectCb(Walker *p, Select *pSelect){
  DbFixer *pFix = p->u.pFix;
  ਪੂਰਨ i;
  Src_ਵਸਤੂ *p_ਵਸਤੂ;
  sqlite3 *db = pFix->pParse->db;
  ਪੂਰਨ iDb = sqlite3_ਲੱਭੋ_Db_Name(db, pFix->zDb);
  SrcList *pList = pSelect->pSrc;

  ਜੇ( NEVER(pList==0) ) ਵਾਪਸ WRC_Continue;
  ਲਈ(i=0, p_ਵਸਤੂ=pList->a; i<pList->nSrc; i++, p_ਵਸਤੂ++){
    ਜੇ( pFix->b_ਆਰਜ਼ੀ==0 && p_ਵਸਤੂ->fg.isSubquery==0 ){
      ਜੇ( p_ਵਸਤੂ->fg.fixedSchema==0 && p_ਵਸਤੂ->u4.zDatabase!=0 ){
        ਜੇ( iDb!=sqlite3_ਲੱਭੋ_Db_Name(db, p_ਵਸਤੂ->u4.zDatabase) ){
          sqlite3_Error_ਸੁਨੇਹਾ(pFix->pParse,
              "%s %T cannot reference objects in database %s",
              pFix->zType, pFix->pName, p_ਵਸਤੂ->u4.zDatabase);
          ਵਾਪਸ WRC_Abort;
        }
        sqlite3DbFree(db, p_ਵਸਤੂ->u4.zDatabase);
        p_ਵਸਤੂ->fg.notCte = 1;
        p_ਵਸਤੂ->fg.hadSchema = 1;
      }
      p_ਵਸਤੂ->u4.pSchema = pFix->pSchema;
      p_ਵਸਤੂ->fg.fromDDL = 1;
      p_ਵਸਤੂ->fg.fixedSchema = 1;
    }
#ਜੇ !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_TRIGGER)
    ਜੇ( pList->a[i].fg.isUsing==0
     && sqlite3_Walk_ਵਿਅੰਜਕ(&pFix->w, pList->a[i].u3.pOn)
    ){
      ਵਾਪਸ WRC_Abort;
    }
#ਜੇ_ਅੰਤ
  }
  ਜੇ( pSelect->pWith ){
    ਲਈ(i=0; i<pSelect->pWith->nCte; i++){
      ਜੇ( sqlite3WalkSelect(p, pSelect->pWith->a[i].pSelect) ){
        ਵਾਪਸ WRC_Abort;
      }
    }
  }
  ਵਾਪਸ WRC_Continue;
}

/*
** Initialize a DbFixer structure.  This routine must be called prior
** to passing the structure to one of the sqliteFixAAAA() routines below.
*/
ਖਾਲੀ sqlite3FixInit(
  DbFixer *pFix,      /* The fixer to be initialized */
  Parse *pParse,      /* Error messages will be written here */
  ਪੂਰਨ iDb,            /* This is the database that must be used */
  ਅਚਲ ਅੱਖਰ *zType,  /* "view", "trigger", or "index" */
  ਅਚਲ Token *pName  /* Name of the view, trigger, or index */
){
  sqlite3 *db = pParse->db;
  ਪ੍ਰਤਿਜ੍ਞਾ( db->nDb>iDb );
  pFix->pParse = pParse;
  pFix->zDb = db->aDb[iDb].zDbSName;
  pFix->pSchema = db->aDb[iDb].pSchema;
  pFix->zType = zType;
  pFix->pName = pName;
  pFix->b_ਆਰਜ਼ੀ = (iDb==1);
  pFix->w.pParse = pParse;
  pFix->w.x_ਵਿਅੰਜਕ_Callback = fix_ਵਿਅੰਜਕ_Cb;
  pFix->w.xSelectCallback = fixSelectCb;
  pFix->w.xSelectCallback2 = sqlite3WalkWinDefnDummyCallback;
  pFix->w.walker_ਡੂੰਘਾਈ = 0;
  pFix->w.e_ਕੋਡ = 0;
  pFix->w.u.pFix = pFix;
}

/*
** The following set of routines walk through the parse tree and assign
** a specific database to all table references where the database name
** was left unspecified in the original SQL statement.  The pFix structure
** must have been initialized by a prior call to sqlite3FixInit().
**
** These routines are used to make sure that an index, trigger, or
** view in one database does not refer to objects in a different database.
** (Exception: indices, triggers, and views in the TEMP database are
** allowed to refer to anything.)  If a reference is explicitly made
** to an object in a different database, an error message is added to
** pParse->zErrMsg and these routines return non-zero.  If everything
** checks out, these routines return 0.
*/
ਪੂਰਨ sqlite3FixSrcList(
  DbFixer *pFix,       /* Context of the fixation */
  SrcList *pList       /* The Source list to check and modify */
){
  ਪੂਰਨ res = 0;
  ਜੇ( pList ){
    Select s; 
    memset(&s, 0, ਆਕਾਰ(s));
    s.pSrc = pList;
    res = sqlite3WalkSelect(&pFix->w, &s);
  }
  ਵਾਪਸ res;
}
#ਜੇ !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_TRIGGER)
ਪੂਰਨ sqlite3FixSelect(
  DbFixer *pFix,       /* Context of the fixation */
  Select *pSelect      /* The SELECT statement to be fixed to one database */
){
  ਵਾਪਸ sqlite3WalkSelect(&pFix->w, pSelect);
}
ਪੂਰਨ sqlite3_Fix_ਵਿਅੰਜਕ(
  DbFixer *pFix,     /* Context of the fixation */
  ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ        /* The expression to be fixed to one database */
){
  ਵਾਪਸ sqlite3_Walk_ਵਿਅੰਜਕ(&pFix->w, p_ਵਿਅੰਜਕ);
}
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_TRIGGER
ਪੂਰਨ sqlite3_Fix_Trigger_ਕਦਮ(
  DbFixer *pFix,     /* Context of the fixation */
  Trigger_ਕਦਮ *p_ਕਦਮ /* The trigger step be fixed to one database */
){
  ਜਦੋਂ( p_ਕਦਮ ){
    ਜੇ( sqlite3WalkSelect(&pFix->w, p_ਕਦਮ->pSelect)
     || sqlite3_Walk_ਵਿਅੰਜਕ(&pFix->w, p_ਕਦਮ->pWhere) 
     || sqlite3_Walk_ਵਿਅੰਜਕ_List(&pFix->w, p_ਕਦਮ->p_ਵਿਅੰਜਕ_List)
     || sqlite3FixSrcList(pFix, p_ਕਦਮ->pSrc)
    ){
      ਵਾਪਸ 1;
    }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_UPSERT
    {
      Upsert *p_ਉੱਪਰ;
      ਲਈ(p_ਉੱਪਰ=p_ਕਦਮ->pUpsert; p_ਉੱਪਰ; p_ਉੱਪਰ=p_ਉੱਪਰ->p_ਅਗਲਾ_Upsert){
        ਜੇ( sqlite3_Walk_ਵਿਅੰਜਕ_List(&pFix->w, p_ਉੱਪਰ->p_Upsert_ਨਿਸ਼ਾਨਾ)
         || sqlite3_Walk_ਵਿਅੰਜਕ(&pFix->w, p_ਉੱਪਰ->p_Upsert_ਨਿਸ਼ਾਨਾ_Where)
         || sqlite3_Walk_ਵਿਅੰਜਕ_List(&pFix->w, p_ਉੱਪਰ->p_Upsert_ਸੈੱਟ)
         || sqlite3_Walk_ਵਿਅੰਜਕ(&pFix->w, p_ਉੱਪਰ->pUpsertWhere)
        ){
          ਵਾਪਸ 1;
        }
      }
    }
#ਜੇ_ਅੰਤ
    p_ਕਦਮ = p_ਕਦਮ->p_ਅਗਲਾ;
  }

  ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ
