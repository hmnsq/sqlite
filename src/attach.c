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
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_ATTACH
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ resolve_Attach_ಅಭಿವ್ಯಕ್ತಿ(NameContext *pName, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ)
{
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ ){
    ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op!=TK_ID ){
      rc = sqlite3_Resolve_ಅಭಿವ್ಯಕ್ತಿ_ಹೆಸರುಗಳು(pName, p_ಅಭಿವ್ಯಕ್ತಿ);
    }ಇಲ್ಲದಿದ್ದರೆ{
      p_ಅಭಿವ್ಯಕ್ತಿ->op = TK_STRING;
    }
  }
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Return true if zName points to a name that may be used to refer to
** database iDb attached to handle db.
*/
ಪೂರ್ಣಾಂಕ sqlite3DbIsNamed(sqlite3 *db, ಪೂರ್ಣಾಂಕ iDb, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName){
  ಹಿಂತಿರುಗಿ (
      sqlite3StrICmp(db->aDb[iDb].zDbSName, zName)==0
   || (iDb==0 && sqlite3StrICmp("main", zName)==0)
  );
}

/*
** If any TEMP triggers reference schema pSchema, move those triggers to
** reference the TEMP schema itself.
*/
ಸ್ಥಿರ ಶೂನ್ಯ detach_ತಾತ್ಕಾಲಿಕ_Triggers(sqlite3 *db, Schema *pSchema){
  HashElem *p_ನಮೂದು;
  ಪ್ರತಿಜ್ಞಾ( db->aDb[1].pSchema );
  p_ನಮೂದು = sqlite_Hash_ಮೊದಲ(&db->aDb[1].pSchema->trigHash);
  ಆಗ( p_ನಮೂದು ){
    Trigger *pTrig = (Trigger*)sqlite_Hash_ದತ್ತಾಂಶ(p_ನಮೂದು);
    ಆದರೆ( pTrig->pTabSchema==pSchema ){
      pTrig->pTabSchema = pTrig->pSchema;
    }
    p_ನಮೂದು = sqlite_Hash_ಮುಂದಿನ(p_ನಮೂದು);
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
ಸ್ಥಿರ ಶೂನ್ಯ attachFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಂಕ Not_ಬಳಸಿದ,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ rc = 0;
  sqlite3 *db = sqlite3_context_db_handle(context);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFile;
  ಅಕ್ಷರ *z_ಮಾರ್ಗ = 0;
  ಅಕ್ಷರ *zErr = 0;
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ flags;
  Db *aNew;                 /* New array of Db pointers */
  Db *pNew = 0;             /* Db object for the newly attached database */
  ಅಕ್ಷರ *zErrDyn = 0;
  sqlite3_vfs *pVfs;

  ಬಳಸದ_PARAMETER(Not_ಬಳಸಿದ);
  zFile = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[0]);
  zName = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[1]);
  ಆದರೆ( zFile==0 ) zFile = "";
  ಆದರೆ( zName==0 ) zName = "";

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_DESERIALIZE
# define REOPEN_AS_MEMDB(db)  (db->init.reopenMemdb)
#ಇಲ್ಲದಿದ್ದರೆ
# define REOPEN_AS_MEMDB(db)  (0)
#ಆದರೆ_ಅಂತ್ಯ

  ಆದರೆ( REOPEN_AS_MEMDB(db) ){
    /* This is not a real ATTACH.  Instead, this routine is being called
    ** from sqlite3_deserialize() to close database db->init.iDb and
    ** reopen it as a MemDB */
    Btree *pNewBt = 0;

    pNew = &db->aDb[db->init.iDb];
    ಪ್ರತಿಜ್ಞಾ( pNew->pBt!=0 );
    ಆದರೆ( sqlite3BtreeTxnState(pNew->pBt)!=SQLITE_TXN_NONE
     || sqlite3_Btree_Is_ಒಳ_Backup(pNew->pBt)
    ){
      rc = SQLITE_BUSY;
      ಹೋಗು attach_error;
    }

    pVfs = sqlite3_vfs_ಕಂಡುಹಿಡಿ("memdb");
    ಆದರೆ( pVfs==0 ) ಹಿಂತಿರುಗಿ;
    rc = sqlite3BtreeOpen(pVfs, "x\0", db, &pNewBt, 0, SQLITE_OPEN_MAIN_DB);
    ಆದರೆ( rc==SQLITE_OK ){
      Schema *pNewSchema = sqlite3SchemaGet(db, pNewBt);
      ಆದರೆ( pNewSchema ){
        /* Both the Btree and the new Schema were allocated successfully.
        ** Close the old db and update the aDb[] slot with the new memdb
        ** values.  */
        detach_ತಾತ್ಕಾಲಿಕ_Triggers(db, pNew->pSchema);
        sqlite3BtreeClose(pNew->pBt);
        pNew->pBt = pNewBt;
        pNew->pSchema = pNewSchema;
        ಆದರೆ( db->init.iDb==0 ){
          /* Clear all eponymous virtual table instances, as they are holding
          ** pointers to the schema object just freed by sqlite3BtreeClose() */
          sqlite3_Vtab_Eponymous_ಕೋಷ್ಟಕ_Clear_All(db);
        }
        sqlite3ExpirePreparedStatements(db, 1);
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3BtreeClose(pNewBt);
        rc = SQLITE_NOMEM;
      }
    }
    ಆದರೆ( rc ) ಹೋಗು attach_error;
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* This is a real ATTACH
    **
    ** Check for the following errors:
    **
    **     * Too many attached databases,
    **     * Transaction currently open
    **     * Specified database name already being used.
    */
    ಆದರೆ( db->nDb>=db->a_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_ATTACHED]+2 ){
      zErrDyn = sqlite3MPrintf(db, "too many attached databases - max %d", 
        db->a_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_ATTACHED]
      );
      ಹೋಗು attach_error;
    }
    ಗಾಗಿ(i=0; i<db->nDb; i++){
      ಪ್ರತಿಜ್ಞಾ( zName );
      ಆದರೆ( sqlite3DbIsNamed(db, i, zName) ){
        zErrDyn = sqlite3MPrintf(db, "database %s is already in use", zName);
        ಹೋಗು attach_error;
      }
    }
  
    /* Allocate the new entry in the db->aDb[] array and initialize the schema
    ** hash tables.
    */
    ಆದರೆ( db->aDb==db->aDbStatic ){
      aNew = sqlite3DbMallocRawNN(db, ಗಾತ್ರ(db->aDb[0])*3 );
      ಆದರೆ( aNew==0 ) ಹಿಂತಿರುಗಿ;
      ಸ್ಮರಣೆ_ನಕಲು(aNew, db->aDb, ಗಾತ್ರ(db->aDb[0])*2);
    }ಇಲ್ಲದಿದ್ದರೆ{
      aNew = sqlite3DbRealloc(db, db->aDb, ಗಾತ್ರ(db->aDb[0])*(1+(i64)db->nDb));
      ಆದರೆ( aNew==0 ) ಹಿಂತಿರುಗಿ;
    }
    db->aDb = aNew;
    pNew = &db->aDb[db->nDb];
    memset(pNew, 0, ಗಾತ್ರ(*pNew));
  
    /* Open the database file. If the btree is successfully opened, use
    ** it to obtain the database schema. At this point the schema may
    ** or may not be initialized.
    */
    flags = db->openFlags;
    rc = sqlite3ParseUri(db->pVfs->zName, zFile, &flags, &pVfs, &z_ಮಾರ್ಗ, &zErr);
    ಆದರೆ( rc!=SQLITE_OK ){
      ಆದರೆ( rc==SQLITE_NOMEM ) sqlite3OomFault(db);
      sqlite3_ಫಲಿತಾಂಶ_error(context, zErr, -1);
      sqlite3_free(zErr);
      ಹಿಂತಿರುಗಿ;
    }
    ಆದರೆ( (db->flags & SQLITE_AttachWrite)==0 ){
      flags &= ~(SQLITE_OPEN_CREATE|SQLITE_OPEN_READWRITE);
      flags |= SQLITE_OPEN_READONLY;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( (db->flags & SQLITE_AttachCreate)==0 ){
      flags &= ~SQLITE_OPEN_CREATE;
    }
    ಪ್ರತಿಜ್ಞಾ( pVfs );
    flags |= SQLITE_OPEN_MAIN_DB;
    rc = sqlite3BtreeOpen(pVfs, z_ಮಾರ್ಗ, db, &pNew->pBt, 0, flags);
    db->nDb++;
    pNew->zDbSName = sqlite3DbStrDup(db, zName);
  }
  db->noSharedCache = 0;
  ಆದರೆ( rc==SQLITE_CONSTRAINT ){
    rc = SQLITE_ERROR;
    zErrDyn = sqlite3MPrintf(db, "database is already attached");
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( rc==SQLITE_OK ){
    Pager *pPager;
    pNew->pSchema = sqlite3SchemaGet(db, pNew->pBt);
    ಆದರೆ( !pNew->pSchema ){
      rc = SQLITE_NOMEM_BKPT;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pNew->pSchema->file_format && pNew->pSchema->enc!=ENC(db) ){
      zErrDyn = sqlite3MPrintf(db, 
        "attached databases must use the same text encoding as main database");
      rc = SQLITE_ERROR;
    }
    sqlite3BtreeEnter(pNew->pBt);
    pPager = sqlite3BtreePager(pNew->pBt);
    sqlite3_Pager_Locking_ಬಹುಲಕ(pPager, db->dflt_ಬೀಗ_ಬಹುಲಕ);
    sqlite3BtreeSecureDelete(pNew->pBt,
                             sqlite3BtreeSecureDelete(db->aDb[0].pBt,-1) );
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_PAGER_PRAGMAS
    sqlite3_Btree_ಗಣ_Pager_Flags(pNew->pBt,
                      PAGER_SYNCHRONOUS_ತುಂಬಿದ | (db->flags & PAGER_FLAGS_ಮುಖವಾಡ));
#ಆದರೆ_ಅಂತ್ಯ
    sqlite3BtreeLeave(pNew->pBt);
  }
  pNew->safety_level = SQLITE_DEFAULT_SYNCHRONOUS+1;
  ಆದರೆ( rc==SQLITE_OK && pNew->zDbSName==0 ){
    rc = SQLITE_NOMEM_BKPT;
  }
  sqlite3_free_filename( z_ಮಾರ್ಗ );

  /* If the file was opened successfully, read the schema for the new database.
  ** If this fails, or if opening the file failed, then close the file and 
  ** remove the entry from the db->aDb[] array. i.e. put everything back the
  ** way we found it.
  */
  ಆದರೆ( rc==SQLITE_OK ){
    sqlite3BtreeEnterAll(db);
    db->init.iDb = 0;
    db->mDbFlags &= ~(DBFLAG_Schema_ತಿಳಿದ_Ok);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_SETLK_TIMEOUT
    ಆದರೆ( db->setlkFlags & SQLITE_SETLK_ಖಂಡ_ON_CONNECT ){
      ಪೂರ್ಣಾಂಕ val = 1;
      sqlite3_file *fd = sqlite3PagerFile(sqlite3BtreePager(pNew->pBt));
      sqlite3OsFileControlHint(fd, SQLITE_FCNTL_ಖಂಡ_ON_CONNECT, &val);
    }
#ಆದರೆ_ಅಂತ್ಯ
    ಆದರೆ( !REOPEN_AS_MEMDB(db) ){
      rc = sqlite3Init(db, &zErrDyn);
    }
    sqlite3BtreeLeaveAll(db);
    ಪ್ರತಿಜ್ಞಾ( zErrDyn==0 || rc!=SQLITE_OK );
  }
  ಆದರೆ( rc ){
    ಆದರೆ( ALWAYS(!REOPEN_AS_MEMDB(db)) ){
      ಪೂರ್ಣಾಂಕ iDb = db->nDb - 1;
      ಪ್ರತಿಜ್ಞಾ( iDb>=2 );
      ಆದರೆ( db->aDb[iDb].pBt ){
        sqlite3BtreeClose(db->aDb[iDb].pBt);
        db->aDb[iDb].pBt = 0;
        db->aDb[iDb].pSchema = 0;
      }
      sqlite3_Reset_All_Schemas_ದ_Connection(db);
      db->nDb = iDb;
      ಆದರೆ( rc==SQLITE_NOMEM || rc==SQLITE_IOERR_NOMEM ){
        sqlite3OomFault(db);
        sqlite3DbFree(db, zErrDyn);
        zErrDyn = sqlite3MPrintf(db, "out of memory");
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( zErrDyn==0 ){
        zErrDyn = sqlite3MPrintf(db, "unable to open database: %s", zFile);
      }
    }
    ಹೋಗು attach_error;
  }
  
  ಹಿಂತಿರುಗಿ;

attach_error:
  /* Return an error if we get here */
  ಆದರೆ( zErrDyn ){
    sqlite3_ಫಲಿತಾಂಶ_error(context, zErrDyn, -1);
    sqlite3DbFree(db, zErrDyn);
  }
  ಆದರೆ( rc ) sqlite3_ಫಲಿತಾಂಶ_error_ಸಂಕೇತ(context, rc);
}

/*
** An SQL user-function registered to do the work of an DETACH statement. The
** three arguments to the function come directly from a detach statement:
**
**     DETACH DATABASE x
**
**     SELECT sqlite_detach(x)
*/
ಸ್ಥಿರ ಶೂನ್ಯ detachFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಂಕ Not_ಬಳಸಿದ,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[0]);
  sqlite3 *db = sqlite3_context_db_handle(context);
  ಪೂರ್ಣಾಂಕ i;
  Db *pDb = 0;
  ಅಕ್ಷರ zErr[128];

  ಬಳಸದ_PARAMETER(Not_ಬಳಸಿದ);

  ಆದರೆ( zName==0 ) zName = "";
  ಗಾಗಿ(i=0; i<db->nDb; i++){
    pDb = &db->aDb[i];
    ಆದರೆ( pDb->pBt==0 ) ಮುಂದುವರಿ;
    ಆದರೆ( sqlite3DbIsNamed(db, i, zName) ) ನಿಲ್ಲಿಸು;
  }

  ಆದರೆ( i>=db->nDb ){
    sqlite3_snprintf(ಗಾತ್ರ(zErr),zErr, "no such database: %s", zName);
    ಹೋಗು detach_error;
  }
  ಆದರೆ( i<2 ){
    sqlite3_snprintf(ಗಾತ್ರ(zErr),zErr, "cannot detach database %s", zName);
    ಹೋಗು detach_error;
  }
  ಆದರೆ( sqlite3BtreeTxnState(pDb->pBt)!=SQLITE_TXN_NONE
   || sqlite3_Btree_Is_ಒಳ_Backup(pDb->pBt)
  ){
    sqlite3_snprintf(ಗಾತ್ರ(zErr),zErr, "database %s is locked", zName);
    ಹೋಗು detach_error;
  }

  detach_ತಾತ್ಕಾಲಿಕ_Triggers(db, pDb->pSchema);
  sqlite3BtreeClose(pDb->pBt);
  pDb->pBt = 0;
  pDb->pSchema = 0;
  sqlite3CollapseDatabaseArray(db);
  ಹಿಂತಿರುಗಿ;

detach_error:
  sqlite3_ಫಲಿತಾಂಶ_error(context, zErr, -1);
}

/*
** This procedure generates VDBE code for a single invocation of either the
** sqlite_detach() or sqlite_attach() SQL user functions.
*/
ಸ್ಥಿರ ಶೂನ್ಯ ಸಂಕೇತ_Attach(
  Parse *pParse,       /* The parser context */
  ಪೂರ್ಣಾಂಕ type,            /* Either SQLITE_ATTACH or SQLITE_DETACH */
  FuncDef ಸ್ಥಿರಾಂಕ *pFunc,/* FuncDef wrapper for detachFunc() or attachFunc() */
  ಅಭಿವ್ಯಕ್ತಿ *pAuthArg,      /* Expression to pass to authorization callback */
  ಅಭಿವ್ಯಕ್ತಿ *pFilename,     /* Name of database file */
  ಅಭಿವ್ಯಕ್ತಿ *pDbname,       /* Name of the database to use internally */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಕೀಲಿ           /* Database key for encryption extension */
){
  ಪೂರ್ಣಾಂಕ rc;
  NameContext sName;
  Vdbe *v;
  sqlite3* db = pParse->db;
  ಪೂರ್ಣಾಂಕ reg_ವಾದಗಳು;

  ಆದರೆ( SQLITE_OK!=sqlite3ReadSchema(pParse) ) ಹೋಗು attach_ಕೊನೆ;

  ಆದರೆ( pParse->nErr ) ಹೋಗು attach_ಕೊನೆ;
  memset(&sName, 0, ಗಾತ್ರ(NameContext));
  sName.pParse = pParse;

  ಆದರೆ( 
      SQLITE_OK!=resolve_Attach_ಅಭಿವ್ಯಕ್ತಿ(&sName, pFilename) ||
      SQLITE_OK!=resolve_Attach_ಅಭಿವ್ಯಕ್ತಿ(&sName, pDbname) ||
      SQLITE_OK!=resolve_Attach_ಅಭಿವ್ಯಕ್ತಿ(&sName, p_ಕೀಲಿ)
  ){
    ಹೋಗು attach_ಕೊನೆ;
  }

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTHORIZATION
  ಆದರೆ( ALWAYS(pAuthArg) ){
    ಅಕ್ಷರ *zAuthArg;
    ಆದರೆ( pAuthArg->op==TK_STRING ){
      ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pAuthArg, EP_Int_ಮೌಲ್ಯ) );
      zAuthArg = pAuthArg->u.zToken;
    }ಇಲ್ಲದಿದ್ದರೆ{
      zAuthArg = 0;
    }
    rc = sqlite3AuthCheck(pParse, type, zAuthArg, 0, 0);
    ಆದರೆ(rc!=SQLITE_OK ){
      ಹೋಗು attach_ಕೊನೆ;
    }
  }
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_AUTHORIZATION */


  v = sqlite3GetVdbe(pParse);
  reg_ವಾದಗಳು = sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Range(pParse, 4);
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ(pParse, pFilename, reg_ವಾದಗಳು);
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ(pParse, pDbname, reg_ವಾದಗಳು+1);
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ(pParse, p_ಕೀಲಿ, reg_ವಾದಗಳು+2);

  ಪ್ರತಿಜ್ಞಾ( v || db->mallocFailed );
  ಆದರೆ( v ){
    sqlite3_Vdbe_ಕೂಡಿಸು_Function_Call(pParse, 0, reg_ವಾದಗಳು+3-pFunc->nArg, reg_ವಾದಗಳು+3,
                               pFunc->nArg, pFunc, 0);
    /* Code an OP_Expire. For an ATTACH statement, set P1 to true (expire this
    ** statement only). For DETACH, set it to false (expire all existing
    ** statements).
    */
    sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_Expire, (type==SQLITE_ATTACH));
  }
  
attach_ಕೊನೆ:
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, pFilename);
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, pDbname);
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, p_ಕೀಲಿ);
}

/*
** Called by the parser to compile a DETACH statement.
**
**     DETACH pDbname
*/
ಶೂನ್ಯ sqlite3Detach(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *pDbname){
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ FuncDef detach_func = {
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
  ಸಂಕೇತ_Attach(pParse, SQLITE_DETACH, &detach_func, pDbname, 0, 0, pDbname);
}

/*
** Called by the parser to compile an ATTACH statement.
**
**     ATTACH p AS pDbname KEY pKey
*/
ಶೂನ್ಯ sqlite3Attach(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p, ಅಭಿವ್ಯಕ್ತಿ *pDbname, ಅಭಿವ್ಯಕ್ತಿ *p_ಕೀಲಿ){
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ FuncDef attach_func = {
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
  ಸಂಕೇತ_Attach(pParse, SQLITE_ATTACH, &attach_func, p, p, pDbname, p_ಕೀಲಿ);
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_ATTACH */

/*
** Expression callback used by sqlite3FixAAAA() routines.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fix_ಅಭಿವ್ಯಕ್ತಿ_Cb(Walker *p, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  DbFixer *pFix = p->u.pFix;
  ಆದರೆ( !pFix->b_ತಾತ್ಕಾಲಿಕ ) ಅಭಿವ್ಯಕ್ತಿ_ಗಣ_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_FromDDL);
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_VARIABLE ){
    ಆದರೆ( pFix->pParse->db->init.busy ){
      p_ಅಭಿವ್ಯಕ್ತಿ->op = TK_NULL;
    }ಇಲ್ಲದಿದ್ದರೆ{
      sqlite3_Error_ಸಂದೇಶ(pFix->pParse, "%s cannot use variables", pFix->zType);
      ಹಿಂತಿರುಗಿ WRC_Abort;
    }
  }
  ಹಿಂತಿರುಗಿ WRC_Continue;
}

/*
** Select callback used by sqlite3FixAAAA() routines.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fixSelectCb(Walker *p, Select *pSelect){
  DbFixer *pFix = p->u.pFix;
  ಪೂರ್ಣಾಂಕ i;
  Src_ಅಂಶ *p_ಅಂಶ;
  sqlite3 *db = pFix->pParse->db;
  ಪೂರ್ಣಾಂಕ iDb = sqlite3_ಕಂಡುಹಿಡಿ_Db_Name(db, pFix->zDb);
  SrcList *pList = pSelect->pSrc;

  ಆದರೆ( NEVER(pList==0) ) ಹಿಂತಿರುಗಿ WRC_Continue;
  ಗಾಗಿ(i=0, p_ಅಂಶ=pList->a; i<pList->nSrc; i++, p_ಅಂಶ++){
    ಆದರೆ( pFix->b_ತಾತ್ಕಾಲಿಕ==0 && p_ಅಂಶ->fg.isSubquery==0 ){
      ಆದರೆ( p_ಅಂಶ->fg.fixedSchema==0 && p_ಅಂಶ->u4.zDatabase!=0 ){
        ಆದರೆ( iDb!=sqlite3_ಕಂಡುಹಿಡಿ_Db_Name(db, p_ಅಂಶ->u4.zDatabase) ){
          sqlite3_Error_ಸಂದೇಶ(pFix->pParse,
              "%s %T cannot reference objects in database %s",
              pFix->zType, pFix->pName, p_ಅಂಶ->u4.zDatabase);
          ಹಿಂತಿರುಗಿ WRC_Abort;
        }
        sqlite3DbFree(db, p_ಅಂಶ->u4.zDatabase);
        p_ಅಂಶ->fg.notCte = 1;
        p_ಅಂಶ->fg.hadSchema = 1;
      }
      p_ಅಂಶ->u4.pSchema = pFix->pSchema;
      p_ಅಂಶ->fg.fromDDL = 1;
      p_ಅಂಶ->fg.fixedSchema = 1;
    }
#ಆದರೆ !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_TRIGGER)
    ಆದರೆ( pList->a[i].fg.isUsing==0
     && sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(&pFix->w, pList->a[i].u3.pOn)
    ){
      ಹಿಂತಿರುಗಿ WRC_Abort;
    }
#ಆದರೆ_ಅಂತ್ಯ
  }
  ಆದರೆ( pSelect->pWith ){
    ಗಾಗಿ(i=0; i<pSelect->pWith->nCte; i++){
      ಆದರೆ( sqlite3WalkSelect(p, pSelect->pWith->a[i].pSelect) ){
        ಹಿಂತಿರುಗಿ WRC_Abort;
      }
    }
  }
  ಹಿಂತಿರುಗಿ WRC_Continue;
}

/*
** Initialize a DbFixer structure.  This routine must be called prior
** to passing the structure to one of the sqliteFixAAAA() routines below.
*/
ಶೂನ್ಯ sqlite3FixInit(
  DbFixer *pFix,      /* The fixer to be initialized */
  Parse *pParse,      /* Error messages will be written here */
  ಪೂರ್ಣಾಂಕ iDb,            /* This is the database that must be used */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zType,  /* "view", "trigger", or "index" */
  ಸ್ಥಿರಾಂಕ Token *pName  /* Name of the view, trigger, or index */
){
  sqlite3 *db = pParse->db;
  ಪ್ರತಿಜ್ಞಾ( db->nDb>iDb );
  pFix->pParse = pParse;
  pFix->zDb = db->aDb[iDb].zDbSName;
  pFix->pSchema = db->aDb[iDb].pSchema;
  pFix->zType = zType;
  pFix->pName = pName;
  pFix->b_ತಾತ್ಕಾಲಿಕ = (iDb==1);
  pFix->w.pParse = pParse;
  pFix->w.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = fix_ಅಭಿವ್ಯಕ್ತಿ_Cb;
  pFix->w.xSelectCallback = fixSelectCb;
  pFix->w.xSelectCallback2 = sqlite3WalkWinDefnDummyCallback;
  pFix->w.walker_ಆಳ = 0;
  pFix->w.e_ಸಂಕೇತ = 0;
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
ಪೂರ್ಣಾಂಕ sqlite3FixSrcList(
  DbFixer *pFix,       /* Context of the fixation */
  SrcList *pList       /* The Source list to check and modify */
){
  ಪೂರ್ಣಾಂಕ res = 0;
  ಆದರೆ( pList ){
    Select s; 
    memset(&s, 0, ಗಾತ್ರ(s));
    s.pSrc = pList;
    res = sqlite3WalkSelect(&pFix->w, &s);
  }
  ಹಿಂತಿರುಗಿ res;
}
#ಆದರೆ !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_TRIGGER)
ಪೂರ್ಣಾಂಕ sqlite3FixSelect(
  DbFixer *pFix,       /* Context of the fixation */
  Select *pSelect      /* The SELECT statement to be fixed to one database */
){
  ಹಿಂತಿರುಗಿ sqlite3WalkSelect(&pFix->w, pSelect);
}
ಪೂರ್ಣಾಂಕ sqlite3_Fix_ಅಭಿವ್ಯಕ್ತಿ(
  DbFixer *pFix,     /* Context of the fixation */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ        /* The expression to be fixed to one database */
){
  ಹಿಂತಿರುಗಿ sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(&pFix->w, p_ಅಭಿವ್ಯಕ್ತಿ);
}
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_TRIGGER
ಪೂರ್ಣಾಂಕ sqlite3_Fix_Trigger_ಹೆಜ್ಜೆ(
  DbFixer *pFix,     /* Context of the fixation */
  Trigger_ಹೆಜ್ಜೆ *p_ಹೆಜ್ಜೆ /* The trigger step be fixed to one database */
){
  ಆಗ( p_ಹೆಜ್ಜೆ ){
    ಆದರೆ( sqlite3WalkSelect(&pFix->w, p_ಹೆಜ್ಜೆ->pSelect)
     || sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(&pFix->w, p_ಹೆಜ್ಜೆ->pWhere) 
     || sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(&pFix->w, p_ಹೆಜ್ಜೆ->p_ಅಭಿವ್ಯಕ್ತಿ_List)
     || sqlite3FixSrcList(pFix, p_ಹೆಜ್ಜೆ->pSrc)
    ){
      ಹಿಂತಿರುಗಿ 1;
    }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_UPSERT
    {
      Upsert *p_ಮೇಲೆ;
      ಗಾಗಿ(p_ಮೇಲೆ=p_ಹೆಜ್ಜೆ->pUpsert; p_ಮೇಲೆ; p_ಮೇಲೆ=p_ಮೇಲೆ->p_ಮುಂದಿನ_Upsert){
        ಆದರೆ( sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(&pFix->w, p_ಮೇಲೆ->p_Upsert_ಗುರಿ)
         || sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(&pFix->w, p_ಮೇಲೆ->p_Upsert_ಗುರಿ_Where)
         || sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(&pFix->w, p_ಮೇಲೆ->p_Upsert_ಗಣ)
         || sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(&pFix->w, p_ಮೇಲೆ->pUpsertWhere)
        ){
          ಹಿಂತಿರುಗಿ 1;
        }
      }
    }
#ಆದರೆ_ಅಂತ್ಯ
    p_ಹೆಜ್ಜೆ = p_ಹೆಜ್ಜೆ->p_ಮುಂದಿನ;
  }

  ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ
