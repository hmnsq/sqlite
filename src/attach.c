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
#include "sqliteInt.h"

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
static පූර්ණ_සංඛ්‍යා resolveAttachExpr(NameContext *pName, Expr *pExpr)
{
  පූර්ණ_සංඛ්‍යා rc = SQLITE_OK;
  නම්( pExpr ){
    නම්( pExpr->op!=TK_ID ){
      rc = sqlite3ResolveExprNames(pName, pExpr);
    }නැතහොත්{
      pExpr->op = TK_STRING;
    }
  }
  ආපසු rc;
}

/*
** Return true if zName points to a name that may be used to refer to
** database iDb attached to handle db.
*/
පූර්ණ_සංඛ්‍යා sqlite3DbIsNamed(sqlite3 *db, පූර්ණ_සංඛ්‍යා iDb, const var_na *zName){
  ආපසු (
      sqlite3StrICmp(db->aDb[iDb].zDbSName, zName)==0
   || (iDb==0 && sqlite3StrICmp("main", zName)==0)
  );
}

/*
** If any TEMP triggers reference schema pSchema, move those triggers to
** reference the TEMP schema itself.
*/
static void detachTempTriggers(sqlite3 *db, Schema *pSchema){
  HashElem *pEntry;
  assert( db->aDb[1].pSchema );
  pEntry = sqliteHashFirst(&db->aDb[1].pSchema->trigHash);
  අතරතුර( pEntry ){
    Trigger *pTrig = (Trigger*)sqliteHashData(pEntry);
    නම්( pTrig->pTabSchema==pSchema ){
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
static void attachFunc(
  sqlite3_context *context,
  පූර්ණ_සංඛ්‍යා NotUsed,
  sqlite3_value **argv
){
  පූර්ණ_සංඛ්‍යා i;
  පූර්ණ_සංඛ්‍යා rc = 0;
  sqlite3 *db = sqlite3_context_db_handle(context);
  const var_na *zName;
  const var_na *zFile;
  var_na *zPath = 0;
  var_na *zErr = 0;
  unsigned පූර්ණ_සංඛ්‍යා flags;
  Db *aNew;                 /* New array of Db pointers */
  Db *pNew = 0;             /* Db object for the newly attached database */
  var_na *zErrDyn = 0;
  sqlite3_vfs *pVfs;

  UNUSED_PARAMETER(NotUsed);
  zFile = (const var_na *)sqlite3_value_text(argv[0]);
  zName = (const var_na *)sqlite3_value_text(argv[1]);
  නම්( zFile==0 ) zFile = "";
  නම්( zName==0 ) zName = "";

#ifndef SQLITE_OMIT_DESERIALIZE
# define REOPEN_AS_MEMDB(db)  (db->init.reopenMemdb)
#නැතහොත්
# define REOPEN_AS_MEMDB(db)  (0)
#endif

  නම්( REOPEN_AS_MEMDB(db) ){
    /* This is not a real ATTACH.  Instead, this routine is being called
    ** from sqlite3_deserialize() to close database db->init.iDb and
    ** reopen it as a MemDB */
    Btree *pNewBt = 0;

    pNew = &db->aDb[db->init.iDb];
    assert( pNew->pBt!=0 );
    නම්( sqlite3BtreeTxnState(pNew->pBt)!=SQLITE_TXN_NONE
     || sqlite3BtreeIsInBackup(pNew->pBt)
    ){
      rc = SQLITE_BUSY;
      goto attach_error;
    }

    pVfs = sqlite3_vfs_find("memdb");
    නම්( pVfs==0 ) ආපසු;
    rc = sqlite3BtreeOpen(pVfs, "x\0", db, &pNewBt, 0, SQLITE_OPEN_MAIN_DB);
    නම්( rc==SQLITE_OK ){
      Schema *pNewSchema = sqlite3SchemaGet(db, pNewBt);
      නම්( pNewSchema ){
        /* Both the Btree and the new Schema were allocated successfully.
        ** Close the old db and update the aDb[] slot with the new memdb
        ** values.  */
        detachTempTriggers(db, pNew->pSchema);
        sqlite3BtreeClose(pNew->pBt);
        pNew->pBt = pNewBt;
        pNew->pSchema = pNewSchema;
        නම්( db->init.iDb==0 ){
          /* Clear all eponymous virtual table instances, as they are holding
          ** pointers to the schema object just freed by sqlite3BtreeClose() */
          sqlite3VtabEponymousTableClearAll(db);
        }
        sqlite3ExpirePreparedStatements(db, 1);
      }නැතහොත්{
        sqlite3BtreeClose(pNewBt);
        rc = SQLITE_NOMEM;
      }
    }
    නම්( rc ) goto attach_error;
  }නැතහොත්{
    /* This is a real ATTACH
    **
    ** Check for the following errors:
    **
    **     * Too many attached databases,
    **     * Transaction currently open
    **     * Specified database name already being used.
    */
    නම්( db->nDb>=db->aLimit[SQLITE_LIMIT_ATTACHED]+2 ){
      zErrDyn = sqlite3MPrintf(db, "too many attached databases - max %d", 
        db->aLimit[SQLITE_LIMIT_ATTACHED]
      );
      goto attach_error;
    }
    සඳහා(i=0; i<db->nDb; i++){
      assert( zName );
      නම්( sqlite3DbIsNamed(db, i, zName) ){
        zErrDyn = sqlite3MPrintf(db, "database %s is already in use", zName);
        goto attach_error;
      }
    }
  
    /* Allocate the new entry in the db->aDb[] array and initialize the schema
    ** hash tables.
    */
    නම්( db->aDb==db->aDbStatic ){
      aNew = sqlite3DbMallocRawNN(db, parimaa_na(db->aDb[0])*3 );
      නම්( aNew==0 ) ආපසු;
      memcpy(aNew, db->aDb, parimaa_na(db->aDb[0])*2);
    }නැතහොත්{
      aNew = sqlite3DbRealloc(db, db->aDb, parimaa_na(db->aDb[0])*(1+(i64)db->nDb));
      නම්( aNew==0 ) ආපසු;
    }
    db->aDb = aNew;
    pNew = &db->aDb[db->nDb];
    memset(pNew, 0, parimaa_na(*pNew));
  
    /* Open the database file. If the btree is successfully opened, use
    ** it to obtain the database schema. At this point the schema may
    ** or may not be initialized.
    */
    flags = db->openFlags;
    rc = sqlite3ParseUri(db->pVfs->zName, zFile, &flags, &pVfs, &zPath, &zErr);
    නම්( rc!=SQLITE_OK ){
      නම්( rc==SQLITE_NOMEM ) sqlite3OomFault(db);
      sqlite3_result_error(context, zErr, -1);
      sqlite3_free(zErr);
      ආපසු;
    }
    නම්( (db->flags & SQLITE_AttachWrite)==0 ){
      flags &= ~(SQLITE_OPEN_CREATE|SQLITE_OPEN_READWRITE);
      flags |= SQLITE_OPEN_READONLY;
    }නැතහොත් නම්( (db->flags & SQLITE_AttachCreate)==0 ){
      flags &= ~SQLITE_OPEN_CREATE;
    }
    assert( pVfs );
    flags |= SQLITE_OPEN_MAIN_DB;
    rc = sqlite3BtreeOpen(pVfs, zPath, db, &pNew->pBt, 0, flags);
    db->nDb++;
    pNew->zDbSName = sqlite3DbStrDup(db, zName);
  }
  db->noSharedCache = 0;
  නම්( rc==SQLITE_CONSTRAINT ){
    rc = SQLITE_ERROR;
    zErrDyn = sqlite3MPrintf(db, "database is already attached");
  }නැතහොත් නම්( rc==SQLITE_OK ){
    Pager *pPager;
    pNew->pSchema = sqlite3SchemaGet(db, pNew->pBt);
    නම්( !pNew->pSchema ){
      rc = SQLITE_NOMEM_BKPT;
    }නැතහොත් නම්( pNew->pSchema->file_format && pNew->pSchema->enc!=ENC(db) ){
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
  නම්( rc==SQLITE_OK && pNew->zDbSName==0 ){
    rc = SQLITE_NOMEM_BKPT;
  }
  sqlite3_free_filename( zPath );

  /* If the file was opened successfully, read the schema for the new database.
  ** If this fails, or if opening the file failed, then close the file and 
  ** remove the entry from the db->aDb[] array. i.e. put everything back the
  ** way we found it.
  */
  නම්( rc==SQLITE_OK ){
    sqlite3BtreeEnterAll(db);
    db->init.iDb = 0;
    db->mDbFlags &= ~(DBFLAG_SchemaKnownOk);
#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
    නම්( db->setlkFlags & SQLITE_SETLK_BLOCK_ON_CONNECT ){
      පූර්ණ_සංඛ්‍යා val = 1;
      sqlite3_file *fd = sqlite3PagerFile(sqlite3BtreePager(pNew->pBt));
      sqlite3OsFileControlHint(fd, SQLITE_FCNTL_BLOCK_ON_CONNECT, &val);
    }
#endif
    නම්( !REOPEN_AS_MEMDB(db) ){
      rc = sqlite3Init(db, &zErrDyn);
    }
    sqlite3BtreeLeaveAll(db);
    assert( zErrDyn==0 || rc!=SQLITE_OK );
  }
  නම්( rc ){
    නම්( ALWAYS(!REOPEN_AS_MEMDB(db)) ){
      පූර්ණ_සංඛ්‍යා iDb = db->nDb - 1;
      assert( iDb>=2 );
      නම්( db->aDb[iDb].pBt ){
        sqlite3BtreeClose(db->aDb[iDb].pBt);
        db->aDb[iDb].pBt = 0;
        db->aDb[iDb].pSchema = 0;
      }
      sqlite3ResetAllSchemasOfConnection(db);
      db->nDb = iDb;
      නම්( rc==SQLITE_NOMEM || rc==SQLITE_IOERR_NOMEM ){
        sqlite3OomFault(db);
        sqlite3DbFree(db, zErrDyn);
        zErrDyn = sqlite3MPrintf(db, "out of memory");
      }නැතහොත් නම්( zErrDyn==0 ){
        zErrDyn = sqlite3MPrintf(db, "unable to open database: %s", zFile);
      }
    }
    goto attach_error;
  }
  
  ආපසු;

attach_error:
  /* Return an error if we get here */
  නම්( zErrDyn ){
    sqlite3_result_error(context, zErrDyn, -1);
    sqlite3DbFree(db, zErrDyn);
  }
  නම්( rc ) sqlite3_result_error_code(context, rc);
}

/*
** An SQL user-function registered to do the work of an DETACH statement. The
** three arguments to the function come directly from a detach statement:
**
**     DETACH DATABASE x
**
**     SELECT sqlite_detach(x)
*/
static void detachFunc(
  sqlite3_context *context,
  පූර්ණ_සංඛ්‍යා NotUsed,
  sqlite3_value **argv
){
  const var_na *zName = (const var_na *)sqlite3_value_text(argv[0]);
  sqlite3 *db = sqlite3_context_db_handle(context);
  පූර්ණ_සංඛ්‍යා i;
  Db *pDb = 0;
  var_na zErr[128];

  UNUSED_PARAMETER(NotUsed);

  නම්( zName==0 ) zName = "";
  සඳහා(i=0; i<db->nDb; i++){
    pDb = &db->aDb[i];
    නම්( pDb->pBt==0 ) continue;
    නම්( sqlite3DbIsNamed(db, i, zName) ) bha_nga;
  }

  නම්( i>=db->nDb ){
    sqlite3_snprintf(parimaa_na(zErr),zErr, "no such database: %s", zName);
    goto detach_error;
  }
  නම්( i<2 ){
    sqlite3_snprintf(parimaa_na(zErr),zErr, "cannot detach database %s", zName);
    goto detach_error;
  }
  නම්( sqlite3BtreeTxnState(pDb->pBt)!=SQLITE_TXN_NONE
   || sqlite3BtreeIsInBackup(pDb->pBt)
  ){
    sqlite3_snprintf(parimaa_na(zErr),zErr, "database %s is locked", zName);
    goto detach_error;
  }

  detachTempTriggers(db, pDb->pSchema);
  sqlite3BtreeClose(pDb->pBt);
  pDb->pBt = 0;
  pDb->pSchema = 0;
  sqlite3CollapseDatabaseArray(db);
  ආපසු;

detach_error:
  sqlite3_result_error(context, zErr, -1);
}

/*
** This procedure generates VDBE code for a single invocation of either the
** sqlite_detach() or sqlite_attach() SQL user functions.
*/
static void codeAttach(
  Parse *pParse,       /* The parser context */
  පූර්ණ_සංඛ්‍යා type,            /* Either SQLITE_ATTACH or SQLITE_DETACH */
  FuncDef const *pFunc,/* FuncDef wrapper for detachFunc() or attachFunc() */
  Expr *pAuthArg,      /* Expression to pass to authorization callback */
  Expr *pFilename,     /* Name of database file */
  Expr *pDbname,       /* Name of the database to use internally */
  Expr *pKey           /* Database key for encryption extension */
){
  පූර්ණ_සංඛ්‍යා rc;
  NameContext sName;
  Vdbe *v;
  sqlite3* db = pParse->db;
  පූර්ණ_සංඛ්‍යා regArgs;

  නම්( SQLITE_OK!=sqlite3ReadSchema(pParse) ) goto attach_end;

  නම්( pParse->nErr ) goto attach_end;
  memset(&sName, 0, parimaa_na(NameContext));
  sName.pParse = pParse;

  නම්( 
      SQLITE_OK!=resolveAttachExpr(&sName, pFilename) ||
      SQLITE_OK!=resolveAttachExpr(&sName, pDbname) ||
      SQLITE_OK!=resolveAttachExpr(&sName, pKey)
  ){
    goto attach_end;
  }

#ifndef SQLITE_OMIT_AUTHORIZATION
  නම්( ALWAYS(pAuthArg) ){
    var_na *zAuthArg;
    නම්( pAuthArg->op==TK_STRING ){
      assert( !ExprHasProperty(pAuthArg, EP_IntValue) );
      zAuthArg = pAuthArg->u.zToken;
    }නැතහොත්{
      zAuthArg = 0;
    }
    rc = sqlite3AuthCheck(pParse, type, zAuthArg, 0, 0);
    නම්(rc!=SQLITE_OK ){
      goto attach_end;
    }
  }
#endif /* SQLITE_OMIT_AUTHORIZATION */


  v = sqlite3GetVdbe(pParse);
  regArgs = sqlite3GetTempRange(pParse, 4);
  sqlite3ExprCode(pParse, pFilename, regArgs);
  sqlite3ExprCode(pParse, pDbname, regArgs+1);
  sqlite3ExprCode(pParse, pKey, regArgs+2);

  assert( v || db->mallocFailed );
  නම්( v ){
    sqlite3VdbeAddFunctionCall(pParse, 0, regArgs+3-pFunc->nArg, regArgs+3,
                               pFunc->nArg, pFunc, 0);
    /* Code an OP_Expire. For an ATTACH statement, set P1 to true (expire this
    ** statement only). For DETACH, set it to false (expire all existing
    ** statements).
    */
    sqlite3VdbeAddOp1(v, OP_Expire, (type==SQLITE_ATTACH));
  }
  
attach_end:
  sqlite3ExprDelete(db, pFilename);
  sqlite3ExprDelete(db, pDbname);
  sqlite3ExprDelete(db, pKey);
}

/*
** Called by the parser to compile a DETACH statement.
**
**     DETACH pDbname
*/
void sqlite3Detach(Parse *pParse, Expr *pDbname){
  static const FuncDef detach_func = {
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
void sqlite3Attach(Parse *pParse, Expr *p, Expr *pDbname, Expr *pKey){
  static const FuncDef attach_func = {
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
  codeAttach(pParse, SQLITE_ATTACH, &attach_func, p, p, pDbname, pKey);
}
#endif /* SQLITE_OMIT_ATTACH */

/*
** Expression callback used by sqlite3FixAAAA() routines.
*/
static පූර්ණ_සංඛ්‍යා fixExprCb(Walker *p, Expr *pExpr){
  DbFixer *pFix = p->u.pFix;
  නම්( !pFix->bTemp ) ExprSetProperty(pExpr, EP_FromDDL);
  නම්( pExpr->op==TK_VARIABLE ){
    නම්( pFix->pParse->db->init.busy ){
      pExpr->op = TK_NULL;
    }නැතහොත්{
      sqlite3ErrorMsg(pFix->pParse, "%s cannot use variables", pFix->zType);
      ආපසු WRC_Abort;
    }
  }
  ආපසු WRC_Continue;
}

/*
** Select callback used by sqlite3FixAAAA() routines.
*/
static පූර්ණ_සංඛ්‍යා fixSelectCb(Walker *p, Select *pSelect){
  DbFixer *pFix = p->u.pFix;
  පූර්ණ_සංඛ්‍යා i;
  SrcItem *pItem;
  sqlite3 *db = pFix->pParse->db;
  පූර්ණ_සංඛ්‍යා iDb = sqlite3FindDbName(db, pFix->zDb);
  SrcList *pList = pSelect->pSrc;

  නම්( NEVER(pList==0) ) ආපසු WRC_Continue;
  සඳහා(i=0, pItem=pList->a; i<pList->nSrc; i++, pItem++){
    නම්( pFix->bTemp==0 && pItem->fg.isSubquery==0 ){
      නම්( pItem->fg.fixedSchema==0 && pItem->u4.zDatabase!=0 ){
        නම්( iDb!=sqlite3FindDbName(db, pItem->u4.zDatabase) ){
          sqlite3ErrorMsg(pFix->pParse,
              "%s %T cannot reference objects in database %s",
              pFix->zType, pFix->pName, pItem->u4.zDatabase);
          ආපසු WRC_Abort;
        }
        sqlite3DbFree(db, pItem->u4.zDatabase);
        pItem->fg.notCte = 1;
        pItem->fg.hadSchema = 1;
      }
      pItem->u4.pSchema = pFix->pSchema;
      pItem->fg.fromDDL = 1;
      pItem->fg.fixedSchema = 1;
    }
#නම් !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_TRIGGER)
    නම්( pList->a[i].fg.isUsing==0
     && sqlite3WalkExpr(&pFix->w, pList->a[i].u3.pOn)
    ){
      ආපසු WRC_Abort;
    }
#endif
  }
  නම්( pSelect->pWith ){
    සඳහා(i=0; i<pSelect->pWith->nCte; i++){
      නම්( sqlite3WalkSelect(p, pSelect->pWith->a[i].pSelect) ){
        ආපසු WRC_Abort;
      }
    }
  }
  ආපසු WRC_Continue;
}

/*
** Initialize a DbFixer structure.  This routine must be called prior
** to passing the structure to one of the sqliteFixAAAA() routines below.
*/
void sqlite3FixInit(
  DbFixer *pFix,      /* The fixer to be initialized */
  Parse *pParse,      /* Error messages will be written here */
  පූර්ණ_සංඛ්‍යා iDb,            /* This is the database that must be used */
  const var_na *zType,  /* "view", "trigger", or "index" */
  const Token *pName  /* Name of the view, trigger, or index */
){
  sqlite3 *db = pParse->db;
  assert( db->nDb>iDb );
  pFix->pParse = pParse;
  pFix->zDb = db->aDb[iDb].zDbSName;
  pFix->pSchema = db->aDb[iDb].pSchema;
  pFix->zType = zType;
  pFix->pName = pName;
  pFix->bTemp = (iDb==1);
  pFix->w.pParse = pParse;
  pFix->w.xExprCallback = fixExprCb;
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
පූර්ණ_සංඛ්‍යා sqlite3FixSrcList(
  DbFixer *pFix,       /* Context of the fixation */
  SrcList *pList       /* The Source list to check and modify */
){
  පූර්ණ_සංඛ්‍යා res = 0;
  නම්( pList ){
    Select s; 
    memset(&s, 0, parimaa_na(s));
    s.pSrc = pList;
    res = sqlite3WalkSelect(&pFix->w, &s);
  }
  ආපසු res;
}
#නම් !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_TRIGGER)
පූර්ණ_සංඛ්‍යා sqlite3FixSelect(
  DbFixer *pFix,       /* Context of the fixation */
  Select *pSelect      /* The SELECT statement to be fixed to one database */
){
  ආපසු sqlite3WalkSelect(&pFix->w, pSelect);
}
පූර්ණ_සංඛ්‍යා sqlite3FixExpr(
  DbFixer *pFix,     /* Context of the fixation */
  Expr *pExpr        /* The expression to be fixed to one database */
){
  ආපසු sqlite3WalkExpr(&pFix->w, pExpr);
}
#endif

#ifndef SQLITE_OMIT_TRIGGER
පූර්ණ_සංඛ්‍යා sqlite3FixTriggerStep(
  DbFixer *pFix,     /* Context of the fixation */
  TriggerStep *pStep /* The trigger step be fixed to one database */
){
  අතරතුර( pStep ){
    නම්( sqlite3WalkSelect(&pFix->w, pStep->pSelect)
     || sqlite3WalkExpr(&pFix->w, pStep->pWhere) 
     || sqlite3WalkExprList(&pFix->w, pStep->pExprList)
     || sqlite3FixSrcList(pFix, pStep->pSrc)
    ){
      ආපසු 1;
    }
#ifndef SQLITE_OMIT_UPSERT
    {
      Upsert *pUp;
      සඳහා(pUp=pStep->pUpsert; pUp; pUp=pUp->pNextUpsert){
        නම්( sqlite3WalkExprList(&pFix->w, pUp->pUpsertTarget)
         || sqlite3WalkExpr(&pFix->w, pUp->pUpsertTargetWhere)
         || sqlite3WalkExprList(&pFix->w, pUp->pUpsertSet)
         || sqlite3WalkExpr(&pFix->w, pUp->pUpsertWhere)
        ){
          ආපසු 1;
        }
      }
    }
#endif
    pStep = pStep->pNext;
  }

  ආපසු 0;
}
#endif
