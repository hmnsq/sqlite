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
#شامل "sqliteInt.h"

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
ایستا صحیح resolve_Attach_عبارت(NameContext *pName, عبارت *p_عبارت)
{
  صحیح rc = SQLITE_OK;
  اگر( p_عبارت ){
    اگر( p_عبارت->op!=TK_ID ){
      rc = sqlite3_Resolve_عبارت_Names(pName, p_عبارت);
    }وگرنه{
      p_عبارت->op = TK_STRING;
    }
  }
  بازگشت rc;
}

/*
** Return true if zName points to a name that may be used to refer to
** database iDb attached to handle db.
*/
صحیح sqlite3DbIsNamed(sqlite3 *db, صحیح iDb, ثابت نویسه *zName){
  بازگشت (
      sqlite3StrICmp(db->aDb[iDb].zDbSName, zName)==0
   || (iDb==0 && sqlite3StrICmp("main", zName)==0)
  );
}

/*
** If any TEMP triggers reference schema pSchema, move those triggers to
** reference the TEMP schema itself.
*/
ایستا تهی detachTempTriggers(sqlite3 *db, Schema *pSchema){
  HashElem *pEntry;
  أكد( db->aDb[1].pSchema );
  pEntry = sqliteHashFirst(&db->aDb[1].pSchema->trigHash);
  وقتی( pEntry ){
    Trigger *pTrig = (Trigger*)sqliteHashData(pEntry);
    اگر( pTrig->pTabSchema==pSchema ){
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
ایستا تهی attachFunc(
  sqlite3_context *context,
  صحیح NotUsed,
  sqlite3_مقدار **argv
){
  صحیح i;
  صحیح rc = 0;
  sqlite3 *db = sqlite3_context_db_handle(context);
  ثابت نویسه *zName;
  ثابت نویسه *zFile;
  نویسه *zPath = 0;
  نویسه *zErr = 0;
  unsigned صحیح flags;
  Db *aNew;                 /* New array of Db pointers */
  Db *pNew = 0;             /* Db object for the newly attached database */
  نویسه *zErrDyn = 0;
  sqlite3_vfs *pVfs;

  UNUSED_PARAMETER(NotUsed);
  zFile = (ثابت نویسه *)sqlite3_مقدار_text(argv[0]);
  zName = (ثابت نویسه *)sqlite3_مقدار_text(argv[1]);
  اگر( zFile==0 ) zFile = "";
  اگر( zName==0 ) zName = "";

#ifndef SQLITE_OMIT_DESERIALIZE
# define REOPEN_AS_MEMDB(db)  (db->init.reopenMemdb)
#وگرنه
# define REOPEN_AS_MEMDB(db)  (0)
#endif

  اگر( REOPEN_AS_MEMDB(db) ){
    /* This is not a real ATTACH.  Instead, this routine is being called
    ** from sqlite3_deserialize() to close database db->init.iDb and
    ** reopen it as a MemDB */
    Btree *pNewBt = 0;

    pNew = &db->aDb[db->init.iDb];
    أكد( pNew->pBt!=0 );
    اگر( sqlite3BtreeTxnState(pNew->pBt)!=SQLITE_TXN_NONE
     || sqlite3BtreeIsInBackup(pNew->pBt)
    ){
      rc = SQLITE_BUSY;
      برو attach_error;
    }

    pVfs = sqlite3_vfs_find("memdb");
    اگر( pVfs==0 ) بازگشت;
    rc = sqlite3BtreeOpen(pVfs, "x\0", db, &pNewBt, 0, SQLITE_OPEN_MAIN_DB);
    اگر( rc==SQLITE_OK ){
      Schema *pNewSchema = sqlite3SchemaGet(db, pNewBt);
      اگر( pNewSchema ){
        /* Both the Btree and the new Schema were allocated successfully.
        ** Close the old db and update the aDb[] slot with the new memdb
        ** values.  */
        detachTempTriggers(db, pNew->pSchema);
        sqlite3BtreeClose(pNew->pBt);
        pNew->pBt = pNewBt;
        pNew->pSchema = pNewSchema;
        اگر( db->init.iDb==0 ){
          /* Clear all eponymous virtual table instances, as they are holding
          ** pointers to the schema object just freed by sqlite3BtreeClose() */
          sqlite3VtabEponymousTableClearAll(db);
        }
        sqlite3ExpirePreparedStatements(db, 1);
      }وگرنه{
        sqlite3BtreeClose(pNewBt);
        rc = SQLITE_NOMEM;
      }
    }
    اگر( rc ) برو attach_error;
  }وگرنه{
    /* This is a real ATTACH
    **
    ** Check for the following errors:
    **
    **     * Too many attached databases,
    **     * Transaction currently open
    **     * Specified database name already being used.
    */
    اگر( db->nDb>=db->aLimit[SQLITE_LIMIT_ATTACHED]+2 ){
      zErrDyn = sqlite3MPrintf(db, "too many attached databases - max %d", 
        db->aLimit[SQLITE_LIMIT_ATTACHED]
      );
      برو attach_error;
    }
    برای(i=0; i<db->nDb; i++){
      أكد( zName );
      اگر( sqlite3DbIsNamed(db, i, zName) ){
        zErrDyn = sqlite3MPrintf(db, "database %s is already in use", zName);
        برو attach_error;
      }
    }
  
    /* Allocate the new entry in the db->aDb[] array and initialize the schema
    ** hash tables.
    */
    اگر( db->aDb==db->aDbStatic ){
      aNew = sqlite3DbMallocRawNN(db, اندازه(db->aDb[0])*3 );
      اگر( aNew==0 ) بازگشت;
      memcpy(aNew, db->aDb, اندازه(db->aDb[0])*2);
    }وگرنه{
      aNew = sqlite3DbRealloc(db, db->aDb, اندازه(db->aDb[0])*(1+(i64)db->nDb));
      اگر( aNew==0 ) بازگشت;
    }
    db->aDb = aNew;
    pNew = &db->aDb[db->nDb];
    memset(pNew, 0, اندازه(*pNew));
  
    /* Open the database file. If the btree is successfully opened, use
    ** it to obtain the database schema. At this point the schema may
    ** or may not be initialized.
    */
    flags = db->openFlags;
    rc = sqlite3ParseUri(db->pVfs->zName, zFile, &flags, &pVfs, &zPath, &zErr);
    اگر( rc!=SQLITE_OK ){
      اگر( rc==SQLITE_NOMEM ) sqlite3OomFault(db);
      sqlite3_result_error(context, zErr, -1);
      sqlite3_free(zErr);
      بازگشت;
    }
    اگر( (db->flags & SQLITE_AttachWrite)==0 ){
      flags &= ~(SQLITE_OPEN_CREATE|SQLITE_OPEN_READWRITE);
      flags |= SQLITE_OPEN_READONLY;
    }وگرنه اگر( (db->flags & SQLITE_AttachCreate)==0 ){
      flags &= ~SQLITE_OPEN_CREATE;
    }
    أكد( pVfs );
    flags |= SQLITE_OPEN_MAIN_DB;
    rc = sqlite3BtreeOpen(pVfs, zPath, db, &pNew->pBt, 0, flags);
    db->nDb++;
    pNew->zDbSName = sqlite3DbStrDup(db, zName);
  }
  db->noSharedCache = 0;
  اگر( rc==SQLITE_CONSTRAINT ){
    rc = SQLITE_ERROR;
    zErrDyn = sqlite3MPrintf(db, "database is already attached");
  }وگرنه اگر( rc==SQLITE_OK ){
    Pager *pPager;
    pNew->pSchema = sqlite3SchemaGet(db, pNew->pBt);
    اگر( !pNew->pSchema ){
      rc = SQLITE_NOMEM_BKPT;
    }وگرنه اگر( pNew->pSchema->file_format && pNew->pSchema->enc!=ENC(db) ){
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
  اگر( rc==SQLITE_OK && pNew->zDbSName==0 ){
    rc = SQLITE_NOMEM_BKPT;
  }
  sqlite3_free_filename( zPath );

  /* If the file was opened successfully, read the schema for the new database.
  ** If this fails, or if opening the file failed, then close the file and 
  ** remove the entry from the db->aDb[] array. i.e. put everything back the
  ** way we found it.
  */
  اگر( rc==SQLITE_OK ){
    sqlite3BtreeEnterAll(db);
    db->init.iDb = 0;
    db->mDbFlags &= ~(DBFLAG_SchemaKnownOk);
#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
    اگر( db->setlkFlags & SQLITE_SETLK_BLOCK_ON_CONNECT ){
      صحیح val = 1;
      sqlite3_file *fd = sqlite3PagerFile(sqlite3BtreePager(pNew->pBt));
      sqlite3OsFileControlHint(fd, SQLITE_FCNTL_BLOCK_ON_CONNECT, &val);
    }
#endif
    اگر( !REOPEN_AS_MEMDB(db) ){
      rc = sqlite3Init(db, &zErrDyn);
    }
    sqlite3BtreeLeaveAll(db);
    أكد( zErrDyn==0 || rc!=SQLITE_OK );
  }
  اگر( rc ){
    اگر( ALWAYS(!REOPEN_AS_MEMDB(db)) ){
      صحیح iDb = db->nDb - 1;
      أكد( iDb>=2 );
      اگر( db->aDb[iDb].pBt ){
        sqlite3BtreeClose(db->aDb[iDb].pBt);
        db->aDb[iDb].pBt = 0;
        db->aDb[iDb].pSchema = 0;
      }
      sqlite3ResetAllSchemasOfConnection(db);
      db->nDb = iDb;
      اگر( rc==SQLITE_NOMEM || rc==SQLITE_IOERR_NOMEM ){
        sqlite3OomFault(db);
        sqlite3DbFree(db, zErrDyn);
        zErrDyn = sqlite3MPrintf(db, "out of memory");
      }وگرنه اگر( zErrDyn==0 ){
        zErrDyn = sqlite3MPrintf(db, "unable to open database: %s", zFile);
      }
    }
    برو attach_error;
  }
  
  بازگشت;

attach_error:
  /* Return an error if we get here */
  اگر( zErrDyn ){
    sqlite3_result_error(context, zErrDyn, -1);
    sqlite3DbFree(db, zErrDyn);
  }
  اگر( rc ) sqlite3_result_error_code(context, rc);
}

/*
** An SQL user-function registered to do the work of an DETACH statement. The
** three arguments to the function come directly from a detach statement:
**
**     DETACH DATABASE x
**
**     SELECT sqlite_detach(x)
*/
ایستا تهی detachFunc(
  sqlite3_context *context,
  صحیح NotUsed,
  sqlite3_مقدار **argv
){
  ثابت نویسه *zName = (ثابت نویسه *)sqlite3_مقدار_text(argv[0]);
  sqlite3 *db = sqlite3_context_db_handle(context);
  صحیح i;
  Db *pDb = 0;
  نویسه zErr[128];

  UNUSED_PARAMETER(NotUsed);

  اگر( zName==0 ) zName = "";
  برای(i=0; i<db->nDb; i++){
    pDb = &db->aDb[i];
    اگر( pDb->pBt==0 ) ادامه;
    اگر( sqlite3DbIsNamed(db, i, zName) ) بشکن;
  }

  اگر( i>=db->nDb ){
    sqlite3_snprintf(اندازه(zErr),zErr, "no such database: %s", zName);
    برو detach_error;
  }
  اگر( i<2 ){
    sqlite3_snprintf(اندازه(zErr),zErr, "cannot detach database %s", zName);
    برو detach_error;
  }
  اگر( sqlite3BtreeTxnState(pDb->pBt)!=SQLITE_TXN_NONE
   || sqlite3BtreeIsInBackup(pDb->pBt)
  ){
    sqlite3_snprintf(اندازه(zErr),zErr, "database %s is locked", zName);
    برو detach_error;
  }

  detachTempTriggers(db, pDb->pSchema);
  sqlite3BtreeClose(pDb->pBt);
  pDb->pBt = 0;
  pDb->pSchema = 0;
  sqlite3CollapseDatabaseArray(db);
  بازگشت;

detach_error:
  sqlite3_result_error(context, zErr, -1);
}

/*
** This procedure generates VDBE code for a single invocation of either the
** sqlite_detach() or sqlite_attach() SQL user functions.
*/
ایستا تهی codeAttach(
  Parse *pParse,       /* The parser context */
  صحیح type,            /* Either SQLITE_ATTACH or SQLITE_DETACH */
  FuncDef ثابت *pFunc,/* FuncDef wrapper for detachFunc() or attachFunc() */
  عبارت *pAuthArg,      /* Expression to pass to authorization callback */
  عبارت *pFilename,     /* Name of database file */
  عبارت *pDbname,       /* Name of the database to use internally */
  عبارت *p_کلید           /* Database key for encryption extension */
){
  صحیح rc;
  NameContext sName;
  Vdbe *v;
  sqlite3* db = pParse->db;
  صحیح regArgs;

  اگر( SQLITE_OK!=sqlite3ReadSchema(pParse) ) برو attach_end;

  اگر( pParse->nErr ) برو attach_end;
  memset(&sName, 0, اندازه(NameContext));
  sName.pParse = pParse;

  اگر( 
      SQLITE_OK!=resolve_Attach_عبارت(&sName, pFilename) ||
      SQLITE_OK!=resolve_Attach_عبارت(&sName, pDbname) ||
      SQLITE_OK!=resolve_Attach_عبارت(&sName, p_کلید)
  ){
    برو attach_end;
  }

#ifndef SQLITE_OMIT_AUTHORIZATION
  اگر( ALWAYS(pAuthArg) ){
    نویسه *zAuthArg;
    اگر( pAuthArg->op==TK_STRING ){
      أكد( !عبارت_Has_Property(pAuthArg, EP_Int_مقدار) );
      zAuthArg = pAuthArg->u.zToken;
    }وگرنه{
      zAuthArg = 0;
    }
    rc = sqlite3AuthCheck(pParse, type, zAuthArg, 0, 0);
    اگر(rc!=SQLITE_OK ){
      برو attach_end;
    }
  }
#endif /* SQLITE_OMIT_AUTHORIZATION */


  v = sqlite3GetVdbe(pParse);
  regArgs = sqlite3GetTempRange(pParse, 4);
  sqlite3_عبارت_Code(pParse, pFilename, regArgs);
  sqlite3_عبارت_Code(pParse, pDbname, regArgs+1);
  sqlite3_عبارت_Code(pParse, p_کلید, regArgs+2);

  أكد( v || db->mallocFailed );
  اگر( v ){
    sqlite3VdbeAddFunctionCall(pParse, 0, regArgs+3-pFunc->nArg, regArgs+3,
                               pFunc->nArg, pFunc, 0);
    /* Code an OP_Expire. For an ATTACH statement, set P1 to true (expire this
    ** statement only). For DETACH, set it to false (expire all existing
    ** statements).
    */
    sqlite3VdbeAddOp1(v, OP_Expire, (type==SQLITE_ATTACH));
  }
  
attach_end:
  sqlite3_عبارت_Delete(db, pFilename);
  sqlite3_عبارت_Delete(db, pDbname);
  sqlite3_عبارت_Delete(db, p_کلید);
}

/*
** Called by the parser to compile a DETACH statement.
**
**     DETACH pDbname
*/
تهی sqlite3Detach(Parse *pParse, عبارت *pDbname){
  ایستا ثابت FuncDef detach_func = {
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
تهی sqlite3Attach(Parse *pParse, عبارت *p, عبارت *pDbname, عبارت *p_کلید){
  ایستا ثابت FuncDef attach_func = {
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
  codeAttach(pParse, SQLITE_ATTACH, &attach_func, p, p, pDbname, p_کلید);
}
#endif /* SQLITE_OMIT_ATTACH */

/*
** Expression callback used by sqlite3FixAAAA() routines.
*/
ایستا صحیح fix_عبارت_Cb(Walker *p, عبارت *p_عبارت){
  DbFixer *pFix = p->u.pFix;
  اگر( !pFix->bTemp ) عبارت_Set_Property(p_عبارت, EP_FromDDL);
  اگر( p_عبارت->op==TK_VARIABLE ){
    اگر( pFix->pParse->db->init.busy ){
      p_عبارت->op = TK_NULL;
    }وگرنه{
      sqlite3_Error_پیام(pFix->pParse, "%s cannot use variables", pFix->zType);
      بازگشت WRC_Abort;
    }
  }
  بازگشت WRC_Continue;
}

/*
** Select callback used by sqlite3FixAAAA() routines.
*/
ایستا صحیح fixSelectCb(Walker *p, Select *pSelect){
  DbFixer *pFix = p->u.pFix;
  صحیح i;
  SrcItem *pItem;
  sqlite3 *db = pFix->pParse->db;
  صحیح iDb = sqlite3FindDbName(db, pFix->zDb);
  SrcList *pList = pSelect->pSrc;

  اگر( NEVER(pList==0) ) بازگشت WRC_Continue;
  برای(i=0, pItem=pList->a; i<pList->nSrc; i++, pItem++){
    اگر( pFix->bTemp==0 && pItem->fg.isSubquery==0 ){
      اگر( pItem->fg.fixedSchema==0 && pItem->u4.zDatabase!=0 ){
        اگر( iDb!=sqlite3FindDbName(db, pItem->u4.zDatabase) ){
          sqlite3_Error_پیام(pFix->pParse,
              "%s %T cannot reference objects in database %s",
              pFix->zType, pFix->pName, pItem->u4.zDatabase);
          بازگشت WRC_Abort;
        }
        sqlite3DbFree(db, pItem->u4.zDatabase);
        pItem->fg.notCte = 1;
        pItem->fg.hadSchema = 1;
      }
      pItem->u4.pSchema = pFix->pSchema;
      pItem->fg.fromDDL = 1;
      pItem->fg.fixedSchema = 1;
    }
#اگر !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_TRIGGER)
    اگر( pList->a[i].fg.isUsing==0
     && sqlite3_Walk_عبارت(&pFix->w, pList->a[i].u3.pOn)
    ){
      بازگشت WRC_Abort;
    }
#endif
  }
  اگر( pSelect->pWith ){
    برای(i=0; i<pSelect->pWith->nCte; i++){
      اگر( sqlite3WalkSelect(p, pSelect->pWith->a[i].pSelect) ){
        بازگشت WRC_Abort;
      }
    }
  }
  بازگشت WRC_Continue;
}

/*
** Initialize a DbFixer structure.  This routine must be called prior
** to passing the structure to one of the sqliteFixAAAA() routines below.
*/
تهی sqlite3FixInit(
  DbFixer *pFix,      /* The fixer to be initialized */
  Parse *pParse,      /* Error messages will be written here */
  صحیح iDb,            /* This is the database that must be used */
  ثابت نویسه *zType,  /* "view", "trigger", or "index" */
  ثابت Token *pName  /* Name of the view, trigger, or index */
){
  sqlite3 *db = pParse->db;
  أكد( db->nDb>iDb );
  pFix->pParse = pParse;
  pFix->zDb = db->aDb[iDb].zDbSName;
  pFix->pSchema = db->aDb[iDb].pSchema;
  pFix->zType = zType;
  pFix->pName = pName;
  pFix->bTemp = (iDb==1);
  pFix->w.pParse = pParse;
  pFix->w.x_عبارت_Callback = fix_عبارت_Cb;
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
صحیح sqlite3FixSrcList(
  DbFixer *pFix,       /* Context of the fixation */
  SrcList *pList       /* The Source list to check and modify */
){
  صحیح res = 0;
  اگر( pList ){
    Select s; 
    memset(&s, 0, اندازه(s));
    s.pSrc = pList;
    res = sqlite3WalkSelect(&pFix->w, &s);
  }
  بازگشت res;
}
#اگر !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_TRIGGER)
صحیح sqlite3FixSelect(
  DbFixer *pFix,       /* Context of the fixation */
  Select *pSelect      /* The SELECT statement to be fixed to one database */
){
  بازگشت sqlite3WalkSelect(&pFix->w, pSelect);
}
صحیح sqlite3_Fix_عبارت(
  DbFixer *pFix,     /* Context of the fixation */
  عبارت *p_عبارت        /* The expression to be fixed to one database */
){
  بازگشت sqlite3_Walk_عبارت(&pFix->w, p_عبارت);
}
#endif

#ifndef SQLITE_OMIT_TRIGGER
صحیح sqlite3FixTriggerStep(
  DbFixer *pFix,     /* Context of the fixation */
  TriggerStep *pStep /* The trigger step be fixed to one database */
){
  وقتی( pStep ){
    اگر( sqlite3WalkSelect(&pFix->w, pStep->pSelect)
     || sqlite3_Walk_عبارت(&pFix->w, pStep->pWhere) 
     || sqlite3_Walk_عبارت_List(&pFix->w, pStep->p_عبارت_List)
     || sqlite3FixSrcList(pFix, pStep->pSrc)
    ){
      بازگشت 1;
    }
#ifndef SQLITE_OMIT_UPSERT
    {
      Upsert *pUp;
      برای(pUp=pStep->pUpsert; pUp; pUp=pUp->pNextUpsert){
        اگر( sqlite3_Walk_عبارت_List(&pFix->w, pUp->pUpsertTarget)
         || sqlite3_Walk_عبارت(&pFix->w, pUp->pUpsertTargetWhere)
         || sqlite3_Walk_عبارت_List(&pFix->w, pUp->pUpsertSet)
         || sqlite3_Walk_عبارت(&pFix->w, pUp->pUpsertWhere)
        ){
          بازگشت 1;
        }
      }
    }
#endif
    pStep = pStep->pNext;
  }

  بازگشت 0;
}
#endif
