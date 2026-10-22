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
#অন্তর্ভাৱ "sqliteInt.h"

#ifndef SQLITE_OMIT_ATTACH
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
স্থির পূর্ণাঙ্ক resolve_Attach_ৰাশিমালা(NameContext *pName, ৰাশিমালা *p_ৰাশিমালা)
{
  পূর্ণাঙ্ক rc = SQLITE_OK;
  যদি( p_ৰাশিমালা ){
    যদি( p_ৰাশিমালা->op!=TK_ID ){
      rc = sqlite3_Resolve_ৰাশিমালা_Names(pName, p_ৰাশিমালা);
    }অন্যথা{
      p_ৰাশিমালা->op = TK_STRING;
    }
  }
  ঘূৰাই rc;
}

/*
** Return true if zName points to a name that may be used to refer to
** database iDb attached to handle db.
*/
পূর্ণাঙ্ক sqlite3DbIsNamed(sqlite3 *db, পূর্ণাঙ্ক iDb, ধ্রুৱ ৱর্ণ *zName){
  ঘূৰাই (
      sqlite3StrICmp(db->aDb[iDb].zDbSName, zName)==0
   || (iDb==0 && sqlite3StrICmp("main", zName)==0)
  );
}

/*
** If any TEMP triggers reference schema pSchema, move those triggers to
** reference the TEMP schema itself.
*/
স্থির শূন্য detachTempTriggers(sqlite3 *db, Schema *pSchema){
  HashElem *pEntry;
  প্রতিজ্ঞা( db->aDb[1].pSchema );
  pEntry = sqliteHashFirst(&db->aDb[1].pSchema->trigHash);
  যেতিয়ালৈ( pEntry ){
    Trigger *pTrig = (Trigger*)sqliteHashData(pEntry);
    যদি( pTrig->pTabSchema==pSchema ){
      pTrig->pTabSchema = pTrig->pSchema;
    }
    pEntry = sqliteHashNext(pEntry);
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
স্থির শূন্য attachFunc(
  sqlite3_context *context,
  পূর্ণাঙ্ক NotUsed,
  sqlite3_মান **argv
){
  পূর্ণাঙ্ক i;
  পূর্ণাঙ্ক rc = 0;
  sqlite3 *db = sqlite3_context_db_handle(context);
  ধ্রুৱ ৱর্ণ *zName;
  ধ্রুৱ ৱর্ণ *zFile;
  ৱর্ণ *zPath = 0;
  ৱর্ণ *zErr = 0;
  unsigned পূর্ণাঙ্ক flags;
  Db *aNew;                 /* New array of Db pointers */
  Db *pNew = 0;             /* Db object for the newly attached database */
  ৱর্ণ *zErrDyn = 0;
  sqlite3_vfs *pVfs;

  UNUSED_PARAMETER(NotUsed);
  zFile = (ধ্রুৱ ৱর্ণ *)sqlite3_মান_text(argv[0]);
  zName = (ধ্রুৱ ৱর্ণ *)sqlite3_মান_text(argv[1]);
  যদি( zFile==0 ) zFile = "";
  যদি( zName==0 ) zName = "";

#ifndef SQLITE_OMIT_DESERIALIZE
# define REOPEN_AS_MEMDB(db)  (db->init.reopenMemdb)
#অন্যথা
# define REOPEN_AS_MEMDB(db)  (0)
#endif

  যদি( REOPEN_AS_MEMDB(db) ){
    /* This is not a real ATTACH.  Instead, this routine is being called
    ** from sqlite3_deserialize() to close database db->init.iDb and
    ** reopen it as a MemDB */
    Btree *pNewBt = 0;

    pNew = &db->aDb[db->init.iDb];
    প্রতিজ্ঞা( pNew->pBt!=0 );
    যদি( sqlite3BtreeTxnState(pNew->pBt)!=SQLITE_TXN_NONE
     || sqlite3BtreeIsInBackup(pNew->pBt)
    ){
      rc = SQLITE_BUSY;
      গচ্ছ attach_error;
    }

    pVfs = sqlite3_vfs_find("memdb");
    যদি( pVfs==0 ) ঘূৰাই;
    rc = sqlite3BtreeOpen(pVfs, "x\0", db, &pNewBt, 0, SQLITE_OPEN_MAIN_DB);
    যদি( rc==SQLITE_OK ){
      Schema *pNewSchema = sqlite3SchemaGet(db, pNewBt);
      যদি( pNewSchema ){
        /* Both the Btree and the new Schema were allocated successfully.
        ** Close the old db and update the aDb[] slot with the new memdb
        ** values.  */
        detachTempTriggers(db, pNew->pSchema);
        sqlite3BtreeClose(pNew->pBt);
        pNew->pBt = pNewBt;
        pNew->pSchema = pNewSchema;
        যদি( db->init.iDb==0 ){
          /* Clear all eponymous virtual table instances, as they are holding
          ** pointers to the schema object just freed by sqlite3BtreeClose() */
          sqlite3VtabEponymousTableClearAll(db);
        }
        sqlite3ExpirePreparedStatements(db, 1);
      }অন্যথা{
        sqlite3BtreeClose(pNewBt);
        rc = SQLITE_NOMEM;
      }
    }
    যদি( rc ) গচ্ছ attach_error;
  }অন্যথা{
    /* This is a real ATTACH
    **
    ** Check for the following errors:
    **
    **     * Too many attached databases,
    **     * Transaction currently open
    **     * Specified database name already being used.
    */
    যদি( db->nDb>=db->aLimit[SQLITE_LIMIT_ATTACHED]+2 ){
      zErrDyn = sqlite3MPrintf(db, "too many attached databases - max %d", 
        db->aLimit[SQLITE_LIMIT_ATTACHED]
      );
      গচ্ছ attach_error;
    }
    বাবে(i=0; i<db->nDb; i++){
      প্রতিজ্ঞা( zName );
      যদি( sqlite3DbIsNamed(db, i, zName) ){
        zErrDyn = sqlite3MPrintf(db, "database %s is already in use", zName);
        গচ্ছ attach_error;
      }
    }
  
    /* Allocate the new entry in the db->aDb[] array and initialize the schema
    ** hash tables.
    */
    যদি( db->aDb==db->aDbStatic ){
      aNew = sqlite3DbMallocRawNN(db, পরিমাণ(db->aDb[0])*3 );
      যদি( aNew==0 ) ঘূৰাই;
      memcpy(aNew, db->aDb, পরিমাণ(db->aDb[0])*2);
    }অন্যথা{
      aNew = sqlite3DbRealloc(db, db->aDb, পরিমাণ(db->aDb[0])*(1+(i64)db->nDb));
      যদি( aNew==0 ) ঘূৰাই;
    }
    db->aDb = aNew;
    pNew = &db->aDb[db->nDb];
    memset(pNew, 0, পরিমাণ(*pNew));
  
    /* Open the database file. If the btree is successfully opened, use
    ** it to obtain the database schema. At this point the schema may
    ** or may not be initialized.
    */
    flags = db->openFlags;
    rc = sqlite3ParseUri(db->pVfs->zName, zFile, &flags, &pVfs, &zPath, &zErr);
    যদি( rc!=SQLITE_OK ){
      যদি( rc==SQLITE_NOMEM ) sqlite3OomFault(db);
      sqlite3_result_error(context, zErr, -1);
      sqlite3_free(zErr);
      ঘূৰাই;
    }
    যদি( (db->flags & SQLITE_AttachWrite)==0 ){
      flags &= ~(SQLITE_OPEN_CREATE|SQLITE_OPEN_READWRITE);
      flags |= SQLITE_OPEN_READONLY;
    }অন্যথা যদি( (db->flags & SQLITE_AttachCreate)==0 ){
      flags &= ~SQLITE_OPEN_CREATE;
    }
    প্রতিজ্ঞা( pVfs );
    flags |= SQLITE_OPEN_MAIN_DB;
    rc = sqlite3BtreeOpen(pVfs, zPath, db, &pNew->pBt, 0, flags);
    db->nDb++;
    pNew->zDbSName = sqlite3DbStrDup(db, zName);
  }
  db->noSharedCache = 0;
  যদি( rc==SQLITE_CONSTRAINT ){
    rc = SQLITE_ERROR;
    zErrDyn = sqlite3MPrintf(db, "database is already attached");
  }অন্যথা যদি( rc==SQLITE_OK ){
    Pager *pPager;
    pNew->pSchema = sqlite3SchemaGet(db, pNew->pBt);
    যদি( !pNew->pSchema ){
      rc = SQLITE_NOMEM_BKPT;
    }অন্যথা যদি( pNew->pSchema->file_format && pNew->pSchema->enc!=ENC(db) ){
      zErrDyn = sqlite3MPrintf(db, 
        "attached databases must use the same text encoding as main database");
      rc = SQLITE_ERROR;
    }
    sqlite3BtreeEnter(pNew->pBt);
    pPager = sqlite3BtreePager(pNew->pBt);
    sqlite3PagerLockingMode(pPager, db->dfltLockMode);
    sqlite3BtreeSecureDelete(pNew->pBt,
                             sqlite3BtreeSecureDelete(db->aDb[0].pBt,-1) );
#ifndef SQLITE_OMIT_PAGER_PRAGMAS
    sqlite3BtreeSetPagerFlags(pNew->pBt,
                      PAGER_SYNCHRONOUS_FULL | (db->flags & PAGER_FLAGS_MASK));
#endif
    sqlite3BtreeLeave(pNew->pBt);
  }
  pNew->safety_level = SQLITE_DEFAULT_SYNCHRONOUS+1;
  যদি( rc==SQLITE_OK && pNew->zDbSName==0 ){
    rc = SQLITE_NOMEM_BKPT;
  }
  sqlite3_free_filename( zPath );

  /* If the file was opened successfully, read the schema for the new database.
  ** If this fails, or if opening the file failed, then close the file and 
  ** remove the entry from the db->aDb[] array. i.e. put everything back the
  ** way we found it.
  */
  যদি( rc==SQLITE_OK ){
    sqlite3BtreeEnterAll(db);
    db->init.iDb = 0;
    db->mDbFlags &= ~(DBFLAG_SchemaKnownOk);
#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
    যদি( db->setlkFlags & SQLITE_SETLK_BLOCK_ON_CONNECT ){
      পূর্ণাঙ্ক val = 1;
      sqlite3_file *fd = sqlite3PagerFile(sqlite3BtreePager(pNew->pBt));
      sqlite3OsFileControlHint(fd, SQLITE_FCNTL_BLOCK_ON_CONNECT, &val);
    }
#endif
    যদি( !REOPEN_AS_MEMDB(db) ){
      rc = sqlite3Init(db, &zErrDyn);
    }
    sqlite3BtreeLeaveAll(db);
    প্রতিজ্ঞা( zErrDyn==0 || rc!=SQLITE_OK );
  }
  যদি( rc ){
    যদি( ALWAYS(!REOPEN_AS_MEMDB(db)) ){
      পূর্ণাঙ্ক iDb = db->nDb - 1;
      প্রতিজ্ঞা( iDb>=2 );
      যদি( db->aDb[iDb].pBt ){
        sqlite3BtreeClose(db->aDb[iDb].pBt);
        db->aDb[iDb].pBt = 0;
        db->aDb[iDb].pSchema = 0;
      }
      sqlite3ResetAllSchemasOfConnection(db);
      db->nDb = iDb;
      যদি( rc==SQLITE_NOMEM || rc==SQLITE_IOERR_NOMEM ){
        sqlite3OomFault(db);
        sqlite3DbFree(db, zErrDyn);
        zErrDyn = sqlite3MPrintf(db, "out of memory");
      }অন্যথা যদি( zErrDyn==0 ){
        zErrDyn = sqlite3MPrintf(db, "unable to open database: %s", zFile);
      }
    }
    গচ্ছ attach_error;
  }
  
  ঘূৰাই;

attach_error:
  /* Return an error if we get here */
  যদি( zErrDyn ){
    sqlite3_result_error(context, zErrDyn, -1);
    sqlite3DbFree(db, zErrDyn);
  }
  যদি( rc ) sqlite3_result_error_code(context, rc);
}

/*
** An SQL user-function registered to do the work of an DETACH statement. The
** three arguments to the function come directly from a detach statement:
**
**     DETACH DATABASE x
**
**     SELECT sqlite_detach(x)
*/
স্থির শূন্য detachFunc(
  sqlite3_context *context,
  পূর্ণাঙ্ক NotUsed,
  sqlite3_মান **argv
){
  ধ্রুৱ ৱর্ণ *zName = (ধ্রুৱ ৱর্ণ *)sqlite3_মান_text(argv[0]);
  sqlite3 *db = sqlite3_context_db_handle(context);
  পূর্ণাঙ্ক i;
  Db *pDb = 0;
  ৱর্ণ zErr[128];

  UNUSED_PARAMETER(NotUsed);

  যদি( zName==0 ) zName = "";
  বাবে(i=0; i<db->nDb; i++){
    pDb = &db->aDb[i];
    যদি( pDb->pBt==0 ) অনুৱর্ত;
    যদি( sqlite3DbIsNamed(db, i, zName) ) ভঙ্গ;
  }

  যদি( i>=db->nDb ){
    sqlite3_snprintf(পরিমাণ(zErr),zErr, "no such database: %s", zName);
    গচ্ছ detach_error;
  }
  যদি( i<2 ){
    sqlite3_snprintf(পরিমাণ(zErr),zErr, "cannot detach database %s", zName);
    গচ্ছ detach_error;
  }
  যদি( sqlite3BtreeTxnState(pDb->pBt)!=SQLITE_TXN_NONE
   || sqlite3BtreeIsInBackup(pDb->pBt)
  ){
    sqlite3_snprintf(পরিমাণ(zErr),zErr, "database %s is locked", zName);
    গচ্ছ detach_error;
  }

  detachTempTriggers(db, pDb->pSchema);
  sqlite3BtreeClose(pDb->pBt);
  pDb->pBt = 0;
  pDb->pSchema = 0;
  sqlite3CollapseDatabaseArray(db);
  ঘূৰাই;

detach_error:
  sqlite3_result_error(context, zErr, -1);
}

/*
** This procedure generates VDBE code for a single invocation of either the
** sqlite_detach() or sqlite_attach() SQL user functions.
*/
স্থির শূন্য codeAttach(
  Parse *pParse,       /* The parser context */
  পূর্ণাঙ্ক type,            /* Either SQLITE_ATTACH or SQLITE_DETACH */
  FuncDef ধ্রুৱ *pFunc,/* FuncDef wrapper for detachFunc() or attachFunc() */
  ৰাশিমালা *pAuthArg,      /* Expression to pass to authorization callback */
  ৰাশিমালা *pFilename,     /* Name of database file */
  ৰাশিমালা *pDbname,       /* Name of the database to use internally */
  ৰাশিমালা *p_চাবি           /* Database key for encryption extension */
){
  পূর্ণাঙ্ক rc;
  NameContext sName;
  Vdbe *v;
  sqlite3* db = pParse->db;
  পূর্ণাঙ্ক regArgs;

  যদি( SQLITE_OK!=sqlite3ReadSchema(pParse) ) গচ্ছ attach_end;

  যদি( pParse->nErr ) গচ্ছ attach_end;
  memset(&sName, 0, পরিমাণ(NameContext));
  sName.pParse = pParse;

  যদি( 
      SQLITE_OK!=resolve_Attach_ৰাশিমালা(&sName, pFilename) ||
      SQLITE_OK!=resolve_Attach_ৰাশিমালা(&sName, pDbname) ||
      SQLITE_OK!=resolve_Attach_ৰাশিমালা(&sName, p_চাবি)
  ){
    গচ্ছ attach_end;
  }

#ifndef SQLITE_OMIT_AUTHORIZATION
  যদি( ALWAYS(pAuthArg) ){
    ৱর্ণ *zAuthArg;
    যদি( pAuthArg->op==TK_STRING ){
      প্রতিজ্ঞা( !ৰাশিমালা_Has_Property(pAuthArg, EP_Int_মান) );
      zAuthArg = pAuthArg->u.zToken;
    }অন্যথা{
      zAuthArg = 0;
    }
    rc = sqlite3AuthCheck(pParse, type, zAuthArg, 0, 0);
    যদি(rc!=SQLITE_OK ){
      গচ্ছ attach_end;
    }
  }
#endif /* SQLITE_OMIT_AUTHORIZATION */


  v = sqlite3GetVdbe(pParse);
  regArgs = sqlite3GetTempRange(pParse, 4);
  sqlite3_ৰাশিমালা_Code(pParse, pFilename, regArgs);
  sqlite3_ৰাশিমালা_Code(pParse, pDbname, regArgs+1);
  sqlite3_ৰাশিমালা_Code(pParse, p_চাবি, regArgs+2);

  প্রতিজ্ঞা( v || db->mallocFailed );
  যদি( v ){
    sqlite3VdbeAddFunctionCall(pParse, 0, regArgs+3-pFunc->nArg, regArgs+3,
                               pFunc->nArg, pFunc, 0);
    /* Code an OP_Expire. For an ATTACH statement, set P1 to true (expire this
    ** statement only). For DETACH, set it to false (expire all existing
    ** statements).
    */
    sqlite3VdbeAddOp1(v, OP_Expire, (type==SQLITE_ATTACH));
  }
  
attach_end:
  sqlite3_ৰাশিমালা_Delete(db, pFilename);
  sqlite3_ৰাশিমালা_Delete(db, pDbname);
  sqlite3_ৰাশিমালা_Delete(db, p_চাবি);
}

/*
** Called by the parser to compile a DETACH statement.
**
**     DETACH pDbname
*/
শূন্য sqlite3Detach(Parse *pParse, ৰাশিমালা *pDbname){
  স্থির ধ্রুৱ FuncDef detach_func = {
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
  codeAttach(pParse, SQLITE_DETACH, &detach_func, pDbname, 0, 0, pDbname);
}

/*
** Called by the parser to compile an ATTACH statement.
**
**     ATTACH p AS pDbname KEY pKey
*/
শূন্য sqlite3Attach(Parse *pParse, ৰাশিমালা *p, ৰাশিমালা *pDbname, ৰাশিমালা *p_চাবি){
  স্থির ধ্রুৱ FuncDef attach_func = {
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
  codeAttach(pParse, SQLITE_ATTACH, &attach_func, p, p, pDbname, p_চাবি);
}
#endif /* SQLITE_OMIT_ATTACH */

/*
** Expression callback used by sqlite3FixAAAA() routines.
*/
স্থির পূর্ণাঙ্ক fix_ৰাশিমালা_Cb(Walker *p, ৰাশিমালা *p_ৰাশিমালা){
  DbFixer *pFix = p->u.pFix;
  যদি( !pFix->bTemp ) ৰাশিমালা_Set_Property(p_ৰাশিমালা, EP_FromDDL);
  যদি( p_ৰাশিমালা->op==TK_VARIABLE ){
    যদি( pFix->pParse->db->init.busy ){
      p_ৰাশিমালা->op = TK_NULL;
    }অন্যথা{
      sqlite3_Error_বাৰ্তা(pFix->pParse, "%s cannot use variables", pFix->zType);
      ঘূৰাই WRC_Abort;
    }
  }
  ঘূৰাই WRC_Continue;
}

/*
** Select callback used by sqlite3FixAAAA() routines.
*/
স্থির পূর্ণাঙ্ক fixSelectCb(Walker *p, Select *pSelect){
  DbFixer *pFix = p->u.pFix;
  পূর্ণাঙ্ক i;
  SrcItem *pItem;
  sqlite3 *db = pFix->pParse->db;
  পূর্ণাঙ্ক iDb = sqlite3FindDbName(db, pFix->zDb);
  SrcList *pList = pSelect->pSrc;

  যদি( NEVER(pList==0) ) ঘূৰাই WRC_Continue;
  বাবে(i=0, pItem=pList->a; i<pList->nSrc; i++, pItem++){
    যদি( pFix->bTemp==0 && pItem->fg.isSubquery==0 ){
      যদি( pItem->fg.fixedSchema==0 && pItem->u4.zDatabase!=0 ){
        যদি( iDb!=sqlite3FindDbName(db, pItem->u4.zDatabase) ){
          sqlite3_Error_বাৰ্তা(pFix->pParse,
              "%s %T cannot reference objects in database %s",
              pFix->zType, pFix->pName, pItem->u4.zDatabase);
          ঘূৰাই WRC_Abort;
        }
        sqlite3DbFree(db, pItem->u4.zDatabase);
        pItem->fg.notCte = 1;
        pItem->fg.hadSchema = 1;
      }
      pItem->u4.pSchema = pFix->pSchema;
      pItem->fg.fromDDL = 1;
      pItem->fg.fixedSchema = 1;
    }
#যদি !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_TRIGGER)
    যদি( pList->a[i].fg.isUsing==0
     && sqlite3_Walk_ৰাশিমালা(&pFix->w, pList->a[i].u3.pOn)
    ){
      ঘূৰাই WRC_Abort;
    }
#endif
  }
  যদি( pSelect->pWith ){
    বাবে(i=0; i<pSelect->pWith->nCte; i++){
      যদি( sqlite3WalkSelect(p, pSelect->pWith->a[i].pSelect) ){
        ঘূৰাই WRC_Abort;
      }
    }
  }
  ঘূৰাই WRC_Continue;
}

/*
** Initialize a DbFixer structure.  This routine must be called prior
** to passing the structure to one of the sqliteFixAAAA() routines below.
*/
শূন্য sqlite3FixInit(
  DbFixer *pFix,      /* The fixer to be initialized */
  Parse *pParse,      /* Error messages will be written here */
  পূর্ণাঙ্ক iDb,            /* This is the database that must be used */
  ধ্রুৱ ৱর্ণ *zType,  /* "view", "trigger", or "index" */
  ধ্রুৱ Token *pName  /* Name of the view, trigger, or index */
){
  sqlite3 *db = pParse->db;
  প্রতিজ্ঞা( db->nDb>iDb );
  pFix->pParse = pParse;
  pFix->zDb = db->aDb[iDb].zDbSName;
  pFix->pSchema = db->aDb[iDb].pSchema;
  pFix->zType = zType;
  pFix->pName = pName;
  pFix->bTemp = (iDb==1);
  pFix->w.pParse = pParse;
  pFix->w.x_ৰাশিমালা_Callback = fix_ৰাশিমালা_Cb;
  pFix->w.xSelectCallback = fixSelectCb;
  pFix->w.xSelectCallback2 = sqlite3WalkWinDefnDummyCallback;
  pFix->w.walkerDepth = 0;
  pFix->w.eCode = 0;
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
পূর্ণাঙ্ক sqlite3FixSrcList(
  DbFixer *pFix,       /* Context of the fixation */
  SrcList *pList       /* The Source list to check and modify */
){
  পূর্ণাঙ্ক res = 0;
  যদি( pList ){
    Select s; 
    memset(&s, 0, পরিমাণ(s));
    s.pSrc = pList;
    res = sqlite3WalkSelect(&pFix->w, &s);
  }
  ঘূৰাই res;
}
#যদি !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_TRIGGER)
পূর্ণাঙ্ক sqlite3FixSelect(
  DbFixer *pFix,       /* Context of the fixation */
  Select *pSelect      /* The SELECT statement to be fixed to one database */
){
  ঘূৰাই sqlite3WalkSelect(&pFix->w, pSelect);
}
পূর্ণাঙ্ক sqlite3_Fix_ৰাশিমালা(
  DbFixer *pFix,     /* Context of the fixation */
  ৰাশিমালা *p_ৰাশিমালা        /* The expression to be fixed to one database */
){
  ঘূৰাই sqlite3_Walk_ৰাশিমালা(&pFix->w, p_ৰাশিমালা);
}
#endif

#ifndef SQLITE_OMIT_TRIGGER
পূর্ণাঙ্ক sqlite3FixTriggerStep(
  DbFixer *pFix,     /* Context of the fixation */
  TriggerStep *pStep /* The trigger step be fixed to one database */
){
  যেতিয়ালৈ( pStep ){
    যদি( sqlite3WalkSelect(&pFix->w, pStep->pSelect)
     || sqlite3_Walk_ৰাশিমালা(&pFix->w, pStep->pWhere) 
     || sqlite3_Walk_ৰাশিমালা_List(&pFix->w, pStep->p_ৰাশিমালা_List)
     || sqlite3FixSrcList(pFix, pStep->pSrc)
    ){
      ঘূৰাই 1;
    }
#ifndef SQLITE_OMIT_UPSERT
    {
      Upsert *pUp;
      বাবে(pUp=pStep->pUpsert; pUp; pUp=pUp->pNextUpsert){
        যদি( sqlite3_Walk_ৰাশিমালা_List(&pFix->w, pUp->pUpsertTarget)
         || sqlite3_Walk_ৰাশিমালা(&pFix->w, pUp->pUpsertTargetWhere)
         || sqlite3_Walk_ৰাশিমালা_List(&pFix->w, pUp->pUpsertSet)
         || sqlite3_Walk_ৰাশিমালা(&pFix->w, pUp->pUpsertWhere)
        ){
          ঘূৰাই 1;
        }
      }
    }
#endif
    pStep = pStep->pNext;
  }

  ঘূৰাই 0;
}
#endif
