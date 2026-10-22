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
#அந்தர்பாவ "sqliteInt.h"

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
நிலையான முழு resolve_Attach_கோவை(NameContext *pName, கோவை *p_கோவை)
{
  முழு rc = SQLITE_OK;
  எனில்( p_கோவை ){
    எனில்( p_கோவை->op!=TK_ID ){
      rc = sqlite3_Resolve_கோவை_Names(pName, p_கோவை);
    }இல்லையெனில்{
      p_கோவை->op = TK_STRING;
    }
  }
  திரும்பு rc;
}

/*
** Return true if zName points to a name that may be used to refer to
** database iDb attached to handle db.
*/
முழு sqlite3DbIsNamed(sqlite3 *db, முழு iDb, மாறிலி எழுத்து *zName){
  திரும்பு (
      sqlite3StrICmp(db->aDb[iDb].zDbSName, zName)==0
   || (iDb==0 && sqlite3StrICmp("main", zName)==0)
  );
}

/*
** If any TEMP triggers reference schema pSchema, move those triggers to
** reference the TEMP schema itself.
*/
நிலையான வெறுமை detachTempTriggers(sqlite3 *db, Schema *pSchema){
  HashElem *pEntry;
  ப்ரதிஜ்ஞா( db->aDb[1].pSchema );
  pEntry = sqliteHashFirst(&db->aDb[1].pSchema->trigHash);
  போது( pEntry ){
    Trigger *pTrig = (Trigger*)sqliteHashData(pEntry);
    எனில்( pTrig->pTabSchema==pSchema ){
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
நிலையான வெறுமை attachFunc(
  sqlite3_context *context,
  முழு NotUsed,
  sqlite3_மதிப்பு **argv
){
  முழு i;
  முழு rc = 0;
  sqlite3 *db = sqlite3_context_db_handle(context);
  மாறிலி எழுத்து *zName;
  மாறிலி எழுத்து *zFile;
  எழுத்து *zPath = 0;
  எழுத்து *zErr = 0;
  unsigned முழு flags;
  Db *aNew;                 /* New array of Db pointers */
  Db *pNew = 0;             /* Db object for the newly attached database */
  எழுத்து *zErrDyn = 0;
  sqlite3_vfs *pVfs;

  UNUSED_PARAMETER(NotUsed);
  zFile = (மாறிலி எழுத்து *)sqlite3_மதிப்பு_text(argv[0]);
  zName = (மாறிலி எழுத்து *)sqlite3_மதிப்பு_text(argv[1]);
  எனில்( zFile==0 ) zFile = "";
  எனில்( zName==0 ) zName = "";

#ifndef SQLITE_OMIT_DESERIALIZE
# define REOPEN_AS_MEMDB(db)  (db->init.reopenMemdb)
#இல்லையெனில்
# define REOPEN_AS_MEMDB(db)  (0)
#endif

  எனில்( REOPEN_AS_MEMDB(db) ){
    /* This is not a real ATTACH.  Instead, this routine is being called
    ** from sqlite3_deserialize() to close database db->init.iDb and
    ** reopen it as a MemDB */
    Btree *pNewBt = 0;

    pNew = &db->aDb[db->init.iDb];
    ப்ரதிஜ்ஞா( pNew->pBt!=0 );
    எனில்( sqlite3BtreeTxnState(pNew->pBt)!=SQLITE_TXN_NONE
     || sqlite3BtreeIsInBackup(pNew->pBt)
    ){
      rc = SQLITE_BUSY;
      செல் attach_error;
    }

    pVfs = sqlite3_vfs_find("memdb");
    எனில்( pVfs==0 ) திரும்பு;
    rc = sqlite3BtreeOpen(pVfs, "x\0", db, &pNewBt, 0, SQLITE_OPEN_MAIN_DB);
    எனில்( rc==SQLITE_OK ){
      Schema *pNewSchema = sqlite3SchemaGet(db, pNewBt);
      எனில்( pNewSchema ){
        /* Both the Btree and the new Schema were allocated successfully.
        ** Close the old db and update the aDb[] slot with the new memdb
        ** values.  */
        detachTempTriggers(db, pNew->pSchema);
        sqlite3BtreeClose(pNew->pBt);
        pNew->pBt = pNewBt;
        pNew->pSchema = pNewSchema;
        எனில்( db->init.iDb==0 ){
          /* Clear all eponymous virtual table instances, as they are holding
          ** pointers to the schema object just freed by sqlite3BtreeClose() */
          sqlite3VtabEponymousTableClearAll(db);
        }
        sqlite3ExpirePreparedStatements(db, 1);
      }இல்லையெனில்{
        sqlite3BtreeClose(pNewBt);
        rc = SQLITE_NOMEM;
      }
    }
    எனில்( rc ) செல் attach_error;
  }இல்லையெனில்{
    /* This is a real ATTACH
    **
    ** Check for the following errors:
    **
    **     * Too many attached databases,
    **     * Transaction currently open
    **     * Specified database name already being used.
    */
    எனில்( db->nDb>=db->aLimit[SQLITE_LIMIT_ATTACHED]+2 ){
      zErrDyn = sqlite3MPrintf(db, "too many attached databases - max %d", 
        db->aLimit[SQLITE_LIMIT_ATTACHED]
      );
      செல் attach_error;
    }
    க்கு(i=0; i<db->nDb; i++){
      ப்ரதிஜ்ஞா( zName );
      எனில்( sqlite3DbIsNamed(db, i, zName) ){
        zErrDyn = sqlite3MPrintf(db, "database %s is already in use", zName);
        செல் attach_error;
      }
    }
  
    /* Allocate the new entry in the db->aDb[] array and initialize the schema
    ** hash tables.
    */
    எனில்( db->aDb==db->aDbStatic ){
      aNew = sqlite3DbMallocRawNN(db, அளவு(db->aDb[0])*3 );
      எனில்( aNew==0 ) திரும்பு;
      memcpy(aNew, db->aDb, அளவு(db->aDb[0])*2);
    }இல்லையெனில்{
      aNew = sqlite3DbRealloc(db, db->aDb, அளவு(db->aDb[0])*(1+(i64)db->nDb));
      எனில்( aNew==0 ) திரும்பு;
    }
    db->aDb = aNew;
    pNew = &db->aDb[db->nDb];
    memset(pNew, 0, அளவு(*pNew));
  
    /* Open the database file. If the btree is successfully opened, use
    ** it to obtain the database schema. At this point the schema may
    ** or may not be initialized.
    */
    flags = db->openFlags;
    rc = sqlite3ParseUri(db->pVfs->zName, zFile, &flags, &pVfs, &zPath, &zErr);
    எனில்( rc!=SQLITE_OK ){
      எனில்( rc==SQLITE_NOMEM ) sqlite3OomFault(db);
      sqlite3_result_error(context, zErr, -1);
      sqlite3_free(zErr);
      திரும்பு;
    }
    எனில்( (db->flags & SQLITE_AttachWrite)==0 ){
      flags &= ~(SQLITE_OPEN_CREATE|SQLITE_OPEN_READWRITE);
      flags |= SQLITE_OPEN_READONLY;
    }இல்லையெனில் எனில்( (db->flags & SQLITE_AttachCreate)==0 ){
      flags &= ~SQLITE_OPEN_CREATE;
    }
    ப்ரதிஜ்ஞா( pVfs );
    flags |= SQLITE_OPEN_MAIN_DB;
    rc = sqlite3BtreeOpen(pVfs, zPath, db, &pNew->pBt, 0, flags);
    db->nDb++;
    pNew->zDbSName = sqlite3DbStrDup(db, zName);
  }
  db->noSharedCache = 0;
  எனில்( rc==SQLITE_CONSTRAINT ){
    rc = SQLITE_ERROR;
    zErrDyn = sqlite3MPrintf(db, "database is already attached");
  }இல்லையெனில் எனில்( rc==SQLITE_OK ){
    Pager *pPager;
    pNew->pSchema = sqlite3SchemaGet(db, pNew->pBt);
    எனில்( !pNew->pSchema ){
      rc = SQLITE_NOMEM_BKPT;
    }இல்லையெனில் எனில்( pNew->pSchema->file_format && pNew->pSchema->enc!=ENC(db) ){
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
  எனில்( rc==SQLITE_OK && pNew->zDbSName==0 ){
    rc = SQLITE_NOMEM_BKPT;
  }
  sqlite3_free_filename( zPath );

  /* If the file was opened successfully, read the schema for the new database.
  ** If this fails, or if opening the file failed, then close the file and 
  ** remove the entry from the db->aDb[] array. i.e. put everything back the
  ** way we found it.
  */
  எனில்( rc==SQLITE_OK ){
    sqlite3BtreeEnterAll(db);
    db->init.iDb = 0;
    db->mDbFlags &= ~(DBFLAG_SchemaKnownOk);
#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
    எனில்( db->setlkFlags & SQLITE_SETLK_BLOCK_ON_CONNECT ){
      முழு val = 1;
      sqlite3_file *fd = sqlite3PagerFile(sqlite3BtreePager(pNew->pBt));
      sqlite3OsFileControlHint(fd, SQLITE_FCNTL_BLOCK_ON_CONNECT, &val);
    }
#endif
    எனில்( !REOPEN_AS_MEMDB(db) ){
      rc = sqlite3Init(db, &zErrDyn);
    }
    sqlite3BtreeLeaveAll(db);
    ப்ரதிஜ்ஞா( zErrDyn==0 || rc!=SQLITE_OK );
  }
  எனில்( rc ){
    எனில்( ALWAYS(!REOPEN_AS_MEMDB(db)) ){
      முழு iDb = db->nDb - 1;
      ப்ரதிஜ்ஞா( iDb>=2 );
      எனில்( db->aDb[iDb].pBt ){
        sqlite3BtreeClose(db->aDb[iDb].pBt);
        db->aDb[iDb].pBt = 0;
        db->aDb[iDb].pSchema = 0;
      }
      sqlite3ResetAllSchemasOfConnection(db);
      db->nDb = iDb;
      எனில்( rc==SQLITE_NOMEM || rc==SQLITE_IOERR_NOMEM ){
        sqlite3OomFault(db);
        sqlite3DbFree(db, zErrDyn);
        zErrDyn = sqlite3MPrintf(db, "out of memory");
      }இல்லையெனில் எனில்( zErrDyn==0 ){
        zErrDyn = sqlite3MPrintf(db, "unable to open database: %s", zFile);
      }
    }
    செல் attach_error;
  }
  
  திரும்பு;

attach_error:
  /* Return an error if we get here */
  எனில்( zErrDyn ){
    sqlite3_result_error(context, zErrDyn, -1);
    sqlite3DbFree(db, zErrDyn);
  }
  எனில்( rc ) sqlite3_result_error_code(context, rc);
}

/*
** An SQL user-function registered to do the work of an DETACH statement. The
** three arguments to the function come directly from a detach statement:
**
**     DETACH DATABASE x
**
**     SELECT sqlite_detach(x)
*/
நிலையான வெறுமை detachFunc(
  sqlite3_context *context,
  முழு NotUsed,
  sqlite3_மதிப்பு **argv
){
  மாறிலி எழுத்து *zName = (மாறிலி எழுத்து *)sqlite3_மதிப்பு_text(argv[0]);
  sqlite3 *db = sqlite3_context_db_handle(context);
  முழு i;
  Db *pDb = 0;
  எழுத்து zErr[128];

  UNUSED_PARAMETER(NotUsed);

  எனில்( zName==0 ) zName = "";
  க்கு(i=0; i<db->nDb; i++){
    pDb = &db->aDb[i];
    எனில்( pDb->pBt==0 ) தொடர்;
    எனில்( sqlite3DbIsNamed(db, i, zName) ) நிறுத்து;
  }

  எனில்( i>=db->nDb ){
    sqlite3_snprintf(அளவு(zErr),zErr, "no such database: %s", zName);
    செல் detach_error;
  }
  எனில்( i<2 ){
    sqlite3_snprintf(அளவு(zErr),zErr, "cannot detach database %s", zName);
    செல் detach_error;
  }
  எனில்( sqlite3BtreeTxnState(pDb->pBt)!=SQLITE_TXN_NONE
   || sqlite3BtreeIsInBackup(pDb->pBt)
  ){
    sqlite3_snprintf(அளவு(zErr),zErr, "database %s is locked", zName);
    செல் detach_error;
  }

  detachTempTriggers(db, pDb->pSchema);
  sqlite3BtreeClose(pDb->pBt);
  pDb->pBt = 0;
  pDb->pSchema = 0;
  sqlite3CollapseDatabaseArray(db);
  திரும்பு;

detach_error:
  sqlite3_result_error(context, zErr, -1);
}

/*
** This procedure generates VDBE code for a single invocation of either the
** sqlite_detach() or sqlite_attach() SQL user functions.
*/
நிலையான வெறுமை codeAttach(
  Parse *pParse,       /* The parser context */
  முழு type,            /* Either SQLITE_ATTACH or SQLITE_DETACH */
  FuncDef மாறிலி *pFunc,/* FuncDef wrapper for detachFunc() or attachFunc() */
  கோவை *pAuthArg,      /* Expression to pass to authorization callback */
  கோவை *pFilename,     /* Name of database file */
  கோவை *pDbname,       /* Name of the database to use internally */
  கோவை *p_சாவி           /* Database key for encryption extension */
){
  முழு rc;
  NameContext sName;
  Vdbe *v;
  sqlite3* db = pParse->db;
  முழு regArgs;

  எனில்( SQLITE_OK!=sqlite3ReadSchema(pParse) ) செல் attach_end;

  எனில்( pParse->nErr ) செல் attach_end;
  memset(&sName, 0, அளவு(NameContext));
  sName.pParse = pParse;

  எனில்( 
      SQLITE_OK!=resolve_Attach_கோவை(&sName, pFilename) ||
      SQLITE_OK!=resolve_Attach_கோவை(&sName, pDbname) ||
      SQLITE_OK!=resolve_Attach_கோவை(&sName, p_சாவி)
  ){
    செல் attach_end;
  }

#ifndef SQLITE_OMIT_AUTHORIZATION
  எனில்( ALWAYS(pAuthArg) ){
    எழுத்து *zAuthArg;
    எனில்( pAuthArg->op==TK_STRING ){
      ப்ரதிஜ்ஞா( !கோவை_Has_Property(pAuthArg, EP_Int_மதிப்பு) );
      zAuthArg = pAuthArg->u.zToken;
    }இல்லையெனில்{
      zAuthArg = 0;
    }
    rc = sqlite3AuthCheck(pParse, type, zAuthArg, 0, 0);
    எனில்(rc!=SQLITE_OK ){
      செல் attach_end;
    }
  }
#endif /* SQLITE_OMIT_AUTHORIZATION */


  v = sqlite3GetVdbe(pParse);
  regArgs = sqlite3GetTempRange(pParse, 4);
  sqlite3_கோவை_Code(pParse, pFilename, regArgs);
  sqlite3_கோவை_Code(pParse, pDbname, regArgs+1);
  sqlite3_கோவை_Code(pParse, p_சாவி, regArgs+2);

  ப்ரதிஜ்ஞா( v || db->mallocFailed );
  எனில்( v ){
    sqlite3VdbeAddFunctionCall(pParse, 0, regArgs+3-pFunc->nArg, regArgs+3,
                               pFunc->nArg, pFunc, 0);
    /* Code an OP_Expire. For an ATTACH statement, set P1 to true (expire this
    ** statement only). For DETACH, set it to false (expire all existing
    ** statements).
    */
    sqlite3VdbeAddOp1(v, OP_Expire, (type==SQLITE_ATTACH));
  }
  
attach_end:
  sqlite3_கோவை_Delete(db, pFilename);
  sqlite3_கோவை_Delete(db, pDbname);
  sqlite3_கோவை_Delete(db, p_சாவி);
}

/*
** Called by the parser to compile a DETACH statement.
**
**     DETACH pDbname
*/
வெறுமை sqlite3Detach(Parse *pParse, கோவை *pDbname){
  நிலையான மாறிலி FuncDef detach_func = {
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
வெறுமை sqlite3Attach(Parse *pParse, கோவை *p, கோவை *pDbname, கோவை *p_சாவி){
  நிலையான மாறிலி FuncDef attach_func = {
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
  codeAttach(pParse, SQLITE_ATTACH, &attach_func, p, p, pDbname, p_சாவி);
}
#endif /* SQLITE_OMIT_ATTACH */

/*
** Expression callback used by sqlite3FixAAAA() routines.
*/
நிலையான முழு fix_கோவை_Cb(Walker *p, கோவை *p_கோவை){
  DbFixer *pFix = p->u.pFix;
  எனில்( !pFix->bTemp ) கோவை_Set_Property(p_கோவை, EP_FromDDL);
  எனில்( p_கோவை->op==TK_VARIABLE ){
    எனில்( pFix->pParse->db->init.busy ){
      p_கோவை->op = TK_NULL;
    }இல்லையெனில்{
      sqlite3_Error_செய்தி(pFix->pParse, "%s cannot use variables", pFix->zType);
      திரும்பு WRC_Abort;
    }
  }
  திரும்பு WRC_Continue;
}

/*
** Select callback used by sqlite3FixAAAA() routines.
*/
நிலையான முழு fixSelectCb(Walker *p, Select *pSelect){
  DbFixer *pFix = p->u.pFix;
  முழு i;
  SrcItem *pItem;
  sqlite3 *db = pFix->pParse->db;
  முழு iDb = sqlite3FindDbName(db, pFix->zDb);
  SrcList *pList = pSelect->pSrc;

  எனில்( NEVER(pList==0) ) திரும்பு WRC_Continue;
  க்கு(i=0, pItem=pList->a; i<pList->nSrc; i++, pItem++){
    எனில்( pFix->bTemp==0 && pItem->fg.isSubquery==0 ){
      எனில்( pItem->fg.fixedSchema==0 && pItem->u4.zDatabase!=0 ){
        எனில்( iDb!=sqlite3FindDbName(db, pItem->u4.zDatabase) ){
          sqlite3_Error_செய்தி(pFix->pParse,
              "%s %T cannot reference objects in database %s",
              pFix->zType, pFix->pName, pItem->u4.zDatabase);
          திரும்பு WRC_Abort;
        }
        sqlite3DbFree(db, pItem->u4.zDatabase);
        pItem->fg.notCte = 1;
        pItem->fg.hadSchema = 1;
      }
      pItem->u4.pSchema = pFix->pSchema;
      pItem->fg.fromDDL = 1;
      pItem->fg.fixedSchema = 1;
    }
#எனில் !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_TRIGGER)
    எனில்( pList->a[i].fg.isUsing==0
     && sqlite3_Walk_கோவை(&pFix->w, pList->a[i].u3.pOn)
    ){
      திரும்பு WRC_Abort;
    }
#endif
  }
  எனில்( pSelect->pWith ){
    க்கு(i=0; i<pSelect->pWith->nCte; i++){
      எனில்( sqlite3WalkSelect(p, pSelect->pWith->a[i].pSelect) ){
        திரும்பு WRC_Abort;
      }
    }
  }
  திரும்பு WRC_Continue;
}

/*
** Initialize a DbFixer structure.  This routine must be called prior
** to passing the structure to one of the sqliteFixAAAA() routines below.
*/
வெறுமை sqlite3FixInit(
  DbFixer *pFix,      /* The fixer to be initialized */
  Parse *pParse,      /* Error messages will be written here */
  முழு iDb,            /* This is the database that must be used */
  மாறிலி எழுத்து *zType,  /* "view", "trigger", or "index" */
  மாறிலி Token *pName  /* Name of the view, trigger, or index */
){
  sqlite3 *db = pParse->db;
  ப்ரதிஜ்ஞா( db->nDb>iDb );
  pFix->pParse = pParse;
  pFix->zDb = db->aDb[iDb].zDbSName;
  pFix->pSchema = db->aDb[iDb].pSchema;
  pFix->zType = zType;
  pFix->pName = pName;
  pFix->bTemp = (iDb==1);
  pFix->w.pParse = pParse;
  pFix->w.x_கோவை_Callback = fix_கோவை_Cb;
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
முழு sqlite3FixSrcList(
  DbFixer *pFix,       /* Context of the fixation */
  SrcList *pList       /* The Source list to check and modify */
){
  முழு res = 0;
  எனில்( pList ){
    Select s; 
    memset(&s, 0, அளவு(s));
    s.pSrc = pList;
    res = sqlite3WalkSelect(&pFix->w, &s);
  }
  திரும்பு res;
}
#எனில் !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_TRIGGER)
முழு sqlite3FixSelect(
  DbFixer *pFix,       /* Context of the fixation */
  Select *pSelect      /* The SELECT statement to be fixed to one database */
){
  திரும்பு sqlite3WalkSelect(&pFix->w, pSelect);
}
முழு sqlite3_Fix_கோவை(
  DbFixer *pFix,     /* Context of the fixation */
  கோவை *p_கோவை        /* The expression to be fixed to one database */
){
  திரும்பு sqlite3_Walk_கோவை(&pFix->w, p_கோவை);
}
#endif

#ifndef SQLITE_OMIT_TRIGGER
முழு sqlite3FixTriggerStep(
  DbFixer *pFix,     /* Context of the fixation */
  TriggerStep *pStep /* The trigger step be fixed to one database */
){
  போது( pStep ){
    எனில்( sqlite3WalkSelect(&pFix->w, pStep->pSelect)
     || sqlite3_Walk_கோவை(&pFix->w, pStep->pWhere) 
     || sqlite3_Walk_கோவை_List(&pFix->w, pStep->p_கோவை_List)
     || sqlite3FixSrcList(pFix, pStep->pSrc)
    ){
      திரும்பு 1;
    }
#ifndef SQLITE_OMIT_UPSERT
    {
      Upsert *pUp;
      க்கு(pUp=pStep->pUpsert; pUp; pUp=pUp->pNextUpsert){
        எனில்( sqlite3_Walk_கோவை_List(&pFix->w, pUp->pUpsertTarget)
         || sqlite3_Walk_கோவை(&pFix->w, pUp->pUpsertTargetWhere)
         || sqlite3_Walk_கோவை_List(&pFix->w, pUp->pUpsertSet)
         || sqlite3_Walk_கோவை(&pFix->w, pUp->pUpsertWhere)
        ){
          திரும்பு 1;
        }
      }
    }
#endif
    pStep = pStep->pNext;
  }

  திரும்பு 0;
}
#endif
