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
#ضمن "sqliteInt.h"

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
ساكن صحيح resolve_Attach_فرچ(NameContext *pName, فرچ *p_فرچ)
{
  صحيح rc = SQLITE_OK;
  إذا( p_فرچ ){
    إذا( p_فرچ->op!=TK_ID ){
      rc = sqlite3_Resolve_فرچ_Names(pName, p_فرچ);
    }وإلا{
      p_فرچ->op = TK_STRING;
    }
  }
  أرجع rc;
}

/*
** Return true if zName points to a name that may be used to refer to
** database iDb attached to handle db.
*/
صحيح sqlite3DbIsNamed(sqlite3 *db, صحيح iDb, ثابت حرف *zName){
  أرجع (
      sqlite3StrICmp(db->aDb[iDb].zDbSName, zName)==0
   || (iDb==0 && sqlite3StrICmp("main", zName)==0)
  );
}

/*
** If any TEMP triggers reference schema pSchema, move those triggers to
** reference the TEMP schema itself.
*/
ساكن فراغ detachTempTriggers(sqlite3 *db, Schema *pSchema){
  HashElem *pEntry;
  أكد( db->aDb[1].pSchema );
  pEntry = sqliteHashFirst(&db->aDb[1].pSchema->trigHash);
  طالما( pEntry ){
    Trigger *pTrig = (Trigger*)sqliteHashData(pEntry);
    إذا( pTrig->pTabSchema==pSchema ){
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
ساكن فراغ attachFunc(
  sqlite3_context *context,
  صحيح NotUsed,
  sqlite3_درج **argv
){
  صحيح i;
  صحيح rc = 0;
  sqlite3 *db = sqlite3_context_db_handle(context);
  ثابت حرف *zName;
  ثابت حرف *zFile;
  حرف *zPath = 0;
  حرف *zErr = 0;
  unsigned صحيح flags;
  Db *aNew;                 /* New array of Db pointers */
  Db *pNew = 0;             /* Db object for the newly attached database */
  حرف *zErrDyn = 0;
  sqlite3_vfs *pVfs;

  UNUSED_PARAMETER(NotUsed);
  zFile = (ثابت حرف *)sqlite3_درج_text(argv[0]);
  zName = (ثابت حرف *)sqlite3_درج_text(argv[1]);
  إذا( zFile==0 ) zFile = "";
  إذا( zName==0 ) zName = "";

#ifndef SQLITE_OMIT_DESERIALIZE
# define REOPEN_AS_MEMDB(db)  (db->init.reopenMemdb)
#وإلا
# define REOPEN_AS_MEMDB(db)  (0)
#endif

  إذا( REOPEN_AS_MEMDB(db) ){
    /* This is not a real ATTACH.  Instead, this routine is being called
    ** from sqlite3_deserialize() to close database db->init.iDb and
    ** reopen it as a MemDB */
    Btree *pNewBt = 0;

    pNew = &db->aDb[db->init.iDb];
    أكد( pNew->pBt!=0 );
    إذا( sqlite3BtreeTxnState(pNew->pBt)!=SQLITE_TXN_NONE
     || sqlite3BtreeIsInBackup(pNew->pBt)
    ){
      rc = SQLITE_BUSY;
      اذهب attach_error;
    }

    pVfs = sqlite3_vfs_find("memdb");
    إذا( pVfs==0 ) أرجع;
    rc = sqlite3BtreeOpen(pVfs, "x\0", db, &pNewBt, 0, SQLITE_OPEN_MAIN_DB);
    إذا( rc==SQLITE_OK ){
      Schema *pNewSchema = sqlite3SchemaGet(db, pNewBt);
      إذا( pNewSchema ){
        /* Both the Btree and the new Schema were allocated successfully.
        ** Close the old db and update the aDb[] slot with the new memdb
        ** values.  */
        detachTempTriggers(db, pNew->pSchema);
        sqlite3BtreeClose(pNew->pBt);
        pNew->pBt = pNewBt;
        pNew->pSchema = pNewSchema;
        إذا( db->init.iDb==0 ){
          /* Clear all eponymous virtual table instances, as they are holding
          ** pointers to the schema object just freed by sqlite3BtreeClose() */
          sqlite3VtabEponymousTableClearAll(db);
        }
        sqlite3ExpirePreparedStatements(db, 1);
      }وإلا{
        sqlite3BtreeClose(pNewBt);
        rc = SQLITE_NOMEM;
      }
    }
    إذا( rc ) اذهب attach_error;
  }وإلا{
    /* This is a real ATTACH
    **
    ** Check for the following errors:
    **
    **     * Too many attached databases,
    **     * Transaction currently open
    **     * Specified database name already being used.
    */
    إذا( db->nDb>=db->aLimit[SQLITE_LIMIT_ATTACHED]+2 ){
      zErrDyn = sqlite3MPrintf(db, "too many attached databases - max %d", 
        db->aLimit[SQLITE_LIMIT_ATTACHED]
      );
      اذهب attach_error;
    }
    لكل(i=0; i<db->nDb; i++){
      أكد( zName );
      إذا( sqlite3DbIsNamed(db, i, zName) ){
        zErrDyn = sqlite3MPrintf(db, "database %s is already in use", zName);
        اذهب attach_error;
      }
    }
  
    /* Allocate the new entry in the db->aDb[] array and initialize the schema
    ** hash tables.
    */
    إذا( db->aDb==db->aDbStatic ){
      aNew = sqlite3DbMallocRawNN(db, حجم(db->aDb[0])*3 );
      إذا( aNew==0 ) أرجع;
      memcpy(aNew, db->aDb, حجم(db->aDb[0])*2);
    }وإلا{
      aNew = sqlite3DbRealloc(db, db->aDb, حجم(db->aDb[0])*(1+(i64)db->nDb));
      إذا( aNew==0 ) أرجع;
    }
    db->aDb = aNew;
    pNew = &db->aDb[db->nDb];
    memset(pNew, 0, حجم(*pNew));
  
    /* Open the database file. If the btree is successfully opened, use
    ** it to obtain the database schema. At this point the schema may
    ** or may not be initialized.
    */
    flags = db->openFlags;
    rc = sqlite3ParseUri(db->pVfs->zName, zFile, &flags, &pVfs, &zPath, &zErr);
    إذا( rc!=SQLITE_OK ){
      إذا( rc==SQLITE_NOMEM ) sqlite3OomFault(db);
      sqlite3_result_error(context, zErr, -1);
      sqlite3_free(zErr);
      أرجع;
    }
    إذا( (db->flags & SQLITE_AttachWrite)==0 ){
      flags &= ~(SQLITE_OPEN_CREATE|SQLITE_OPEN_READWRITE);
      flags |= SQLITE_OPEN_READONLY;
    }وإلا إذا( (db->flags & SQLITE_AttachCreate)==0 ){
      flags &= ~SQLITE_OPEN_CREATE;
    }
    أكد( pVfs );
    flags |= SQLITE_OPEN_MAIN_DB;
    rc = sqlite3BtreeOpen(pVfs, zPath, db, &pNew->pBt, 0, flags);
    db->nDb++;
    pNew->zDbSName = sqlite3DbStrDup(db, zName);
  }
  db->noSharedCache = 0;
  إذا( rc==SQLITE_CONSTRAINT ){
    rc = SQLITE_ERROR;
    zErrDyn = sqlite3MPrintf(db, "database is already attached");
  }وإلا إذا( rc==SQLITE_OK ){
    Pager *pPager;
    pNew->pSchema = sqlite3SchemaGet(db, pNew->pBt);
    إذا( !pNew->pSchema ){
      rc = SQLITE_NOMEM_BKPT;
    }وإلا إذا( pNew->pSchema->file_format && pNew->pSchema->enc!=ENC(db) ){
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
  إذا( rc==SQLITE_OK && pNew->zDbSName==0 ){
    rc = SQLITE_NOMEM_BKPT;
  }
  sqlite3_free_filename( zPath );

  /* If the file was opened successfully, read the schema for the new database.
  ** If this fails, or if opening the file failed, then close the file and 
  ** remove the entry from the db->aDb[] array. i.e. put everything back the
  ** way we found it.
  */
  إذا( rc==SQLITE_OK ){
    sqlite3BtreeEnterAll(db);
    db->init.iDb = 0;
    db->mDbFlags &= ~(DBFLAG_SchemaKnownOk);
#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
    إذا( db->setlkFlags & SQLITE_SETLK_BLOCK_ON_CONNECT ){
      صحيح val = 1;
      sqlite3_file *fd = sqlite3PagerFile(sqlite3BtreePager(pNew->pBt));
      sqlite3OsFileControlHint(fd, SQLITE_FCNTL_BLOCK_ON_CONNECT, &val);
    }
#endif
    إذا( !REOPEN_AS_MEMDB(db) ){
      rc = sqlite3Init(db, &zErrDyn);
    }
    sqlite3BtreeLeaveAll(db);
    أكد( zErrDyn==0 || rc!=SQLITE_OK );
  }
  إذا( rc ){
    إذا( ALWAYS(!REOPEN_AS_MEMDB(db)) ){
      صحيح iDb = db->nDb - 1;
      أكد( iDb>=2 );
      إذا( db->aDb[iDb].pBt ){
        sqlite3BtreeClose(db->aDb[iDb].pBt);
        db->aDb[iDb].pBt = 0;
        db->aDb[iDb].pSchema = 0;
      }
      sqlite3ResetAllSchemasOfConnection(db);
      db->nDb = iDb;
      إذا( rc==SQLITE_NOMEM || rc==SQLITE_IOERR_NOMEM ){
        sqlite3OomFault(db);
        sqlite3DbFree(db, zErrDyn);
        zErrDyn = sqlite3MPrintf(db, "out of memory");
      }وإلا إذا( zErrDyn==0 ){
        zErrDyn = sqlite3MPrintf(db, "unable to open database: %s", zFile);
      }
    }
    اذهب attach_error;
  }
  
  أرجع;

attach_error:
  /* Return an error if we get here */
  إذا( zErrDyn ){
    sqlite3_result_error(context, zErrDyn, -1);
    sqlite3DbFree(db, zErrDyn);
  }
  إذا( rc ) sqlite3_result_error_code(context, rc);
}

/*
** An SQL user-function registered to do the work of an DETACH statement. The
** three arguments to the function come directly from a detach statement:
**
**     DETACH DATABASE x
**
**     SELECT sqlite_detach(x)
*/
ساكن فراغ detachFunc(
  sqlite3_context *context,
  صحيح NotUsed,
  sqlite3_درج **argv
){
  ثابت حرف *zName = (ثابت حرف *)sqlite3_درج_text(argv[0]);
  sqlite3 *db = sqlite3_context_db_handle(context);
  صحيح i;
  Db *pDb = 0;
  حرف zErr[128];

  UNUSED_PARAMETER(NotUsed);

  إذا( zName==0 ) zName = "";
  لكل(i=0; i<db->nDb; i++){
    pDb = &db->aDb[i];
    إذا( pDb->pBt==0 ) استمر;
    إذا( sqlite3DbIsNamed(db, i, zName) ) اكسر;
  }

  إذا( i>=db->nDb ){
    sqlite3_snprintf(حجم(zErr),zErr, "no such database: %s", zName);
    اذهب detach_error;
  }
  إذا( i<2 ){
    sqlite3_snprintf(حجم(zErr),zErr, "cannot detach database %s", zName);
    اذهب detach_error;
  }
  إذا( sqlite3BtreeTxnState(pDb->pBt)!=SQLITE_TXN_NONE
   || sqlite3BtreeIsInBackup(pDb->pBt)
  ){
    sqlite3_snprintf(حجم(zErr),zErr, "database %s is locked", zName);
    اذهب detach_error;
  }

  detachTempTriggers(db, pDb->pSchema);
  sqlite3BtreeClose(pDb->pBt);
  pDb->pBt = 0;
  pDb->pSchema = 0;
  sqlite3CollapseDatabaseArray(db);
  أرجع;

detach_error:
  sqlite3_result_error(context, zErr, -1);
}

/*
** This procedure generates VDBE code for a single invocation of either the
** sqlite_detach() or sqlite_attach() SQL user functions.
*/
ساكن فراغ codeAttach(
  Parse *pParse,       /* The parser context */
  صحيح type,            /* Either SQLITE_ATTACH or SQLITE_DETACH */
  FuncDef ثابت *pFunc,/* FuncDef wrapper for detachFunc() or attachFunc() */
  فرچ *pAuthArg,      /* Expression to pass to authorization callback */
  فرچ *pFilename,     /* Name of database file */
  فرچ *pDbname,       /* Name of the database to use internally */
  فرچ *p_مبودي           /* Database key for encryption extension */
){
  صحيح rc;
  NameContext sName;
  Vdbe *v;
  sqlite3* db = pParse->db;
  صحيح reg_حجوجي;

  إذا( SQLITE_OK!=sqlite3ReadSchema(pParse) ) اذهب attach_end;

  إذا( pParse->nErr ) اذهب attach_end;
  memset(&sName, 0, حجم(NameContext));
  sName.pParse = pParse;

  إذا( 
      SQLITE_OK!=resolve_Attach_فرچ(&sName, pFilename) ||
      SQLITE_OK!=resolve_Attach_فرچ(&sName, pDbname) ||
      SQLITE_OK!=resolve_Attach_فرچ(&sName, p_مبودي)
  ){
    اذهب attach_end;
  }

#ifndef SQLITE_OMIT_AUTHORIZATION
  إذا( ALWAYS(pAuthArg) ){
    حرف *zAuthArg;
    إذا( pAuthArg->op==TK_STRING ){
      أكد( !فرچ_Has_Property(pAuthArg, EP_Int_درج) );
      zAuthArg = pAuthArg->u.zToken;
    }وإلا{
      zAuthArg = 0;
    }
    rc = sqlite3AuthCheck(pParse, type, zAuthArg, 0, 0);
    إذا(rc!=SQLITE_OK ){
      اذهب attach_end;
    }
  }
#endif /* SQLITE_OMIT_AUTHORIZATION */


  v = sqlite3GetVdbe(pParse);
  reg_حجوجي = sqlite3GetTempRange(pParse, 4);
  sqlite3_فرچ_Code(pParse, pFilename, reg_حجوجي);
  sqlite3_فرچ_Code(pParse, pDbname, reg_حجوجي+1);
  sqlite3_فرچ_Code(pParse, p_مبودي, reg_حجوجي+2);

  أكد( v || db->mallocFailed );
  إذا( v ){
    sqlite3VdbeAddFunctionCall(pParse, 0, reg_حجوجي+3-pFunc->nArg, reg_حجوجي+3,
                               pFunc->nArg, pFunc, 0);
    /* Code an OP_Expire. For an ATTACH statement, set P1 to true (expire this
    ** statement only). For DETACH, set it to false (expire all existing
    ** statements).
    */
    sqlite3VdbeAddOp1(v, OP_Expire, (type==SQLITE_ATTACH));
  }
  
attach_end:
  sqlite3_فرچ_Delete(db, pFilename);
  sqlite3_فرچ_Delete(db, pDbname);
  sqlite3_فرچ_Delete(db, p_مبودي);
}

/*
** Called by the parser to compile a DETACH statement.
**
**     DETACH pDbname
*/
فراغ sqlite3Detach(Parse *pParse, فرچ *pDbname){
  ساكن ثابت FuncDef detach_func = {
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
فراغ sqlite3Attach(Parse *pParse, فرچ *p, فرچ *pDbname, فرچ *p_مبودي){
  ساكن ثابت FuncDef attach_func = {
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
  codeAttach(pParse, SQLITE_ATTACH, &attach_func, p, p, pDbname, p_مبودي);
}
#endif /* SQLITE_OMIT_ATTACH */

/*
** Expression callback used by sqlite3FixAAAA() routines.
*/
ساكن صحيح fix_فرچ_Cb(Walker *p, فرچ *p_فرچ){
  DbFixer *pFix = p->u.pFix;
  إذا( !pFix->bTemp ) فرچ_Set_Property(p_فرچ, EP_FromDDL);
  إذا( p_فرچ->op==TK_VARIABLE ){
    إذا( pFix->pParse->db->init.busy ){
      p_فرچ->op = TK_NULL;
    }وإلا{
      sqlite3_Error_سكو(pFix->pParse, "%s cannot use variables", pFix->zType);
      أرجع WRC_Abort;
    }
  }
  أرجع WRC_Continue;
}

/*
** Select callback used by sqlite3FixAAAA() routines.
*/
ساكن صحيح fixSelectCb(Walker *p, Select *pSelect){
  DbFixer *pFix = p->u.pFix;
  صحيح i;
  SrcItem *pItem;
  sqlite3 *db = pFix->pParse->db;
  صحيح iDb = sqlite3FindDbName(db, pFix->zDb);
  SrcList *pList = pSelect->pSrc;

  إذا( NEVER(pList==0) ) أرجع WRC_Continue;
  لكل(i=0, pItem=pList->a; i<pList->nSrc; i++, pItem++){
    إذا( pFix->bTemp==0 && pItem->fg.isSubquery==0 ){
      إذا( pItem->fg.fixedSchema==0 && pItem->u4.zDatabase!=0 ){
        إذا( iDb!=sqlite3FindDbName(db, pItem->u4.zDatabase) ){
          sqlite3_Error_سكو(pFix->pParse,
              "%s %T cannot reference objects in database %s",
              pFix->zType, pFix->pName, pItem->u4.zDatabase);
          أرجع WRC_Abort;
        }
        sqlite3DbFree(db, pItem->u4.zDatabase);
        pItem->fg.notCte = 1;
        pItem->fg.hadSchema = 1;
      }
      pItem->u4.pSchema = pFix->pSchema;
      pItem->fg.fromDDL = 1;
      pItem->fg.fixedSchema = 1;
    }
#إذا !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_TRIGGER)
    إذا( pList->a[i].fg.isUsing==0
     && sqlite3_Walk_فرچ(&pFix->w, pList->a[i].u3.pOn)
    ){
      أرجع WRC_Abort;
    }
#endif
  }
  إذا( pSelect->pWith ){
    لكل(i=0; i<pSelect->pWith->nCte; i++){
      إذا( sqlite3WalkSelect(p, pSelect->pWith->a[i].pSelect) ){
        أرجع WRC_Abort;
      }
    }
  }
  أرجع WRC_Continue;
}

/*
** Initialize a DbFixer structure.  This routine must be called prior
** to passing the structure to one of the sqliteFixAAAA() routines below.
*/
فراغ sqlite3FixInit(
  DbFixer *pFix,      /* The fixer to be initialized */
  Parse *pParse,      /* Error messages will be written here */
  صحيح iDb,            /* This is the database that must be used */
  ثابت حرف *zType,  /* "view", "trigger", or "index" */
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
  pFix->w.x_فرچ_Callback = fix_فرچ_Cb;
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
صحيح sqlite3FixSrcList(
  DbFixer *pFix,       /* Context of the fixation */
  SrcList *pList       /* The Source list to check and modify */
){
  صحيح res = 0;
  إذا( pList ){
    Select s; 
    memset(&s, 0, حجم(s));
    s.pSrc = pList;
    res = sqlite3WalkSelect(&pFix->w, &s);
  }
  أرجع res;
}
#إذا !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_TRIGGER)
صحيح sqlite3FixSelect(
  DbFixer *pFix,       /* Context of the fixation */
  Select *pSelect      /* The SELECT statement to be fixed to one database */
){
  أرجع sqlite3WalkSelect(&pFix->w, pSelect);
}
صحيح sqlite3_Fix_فرچ(
  DbFixer *pFix,     /* Context of the fixation */
  فرچ *p_فرچ        /* The expression to be fixed to one database */
){
  أرجع sqlite3_Walk_فرچ(&pFix->w, p_فرچ);
}
#endif

#ifndef SQLITE_OMIT_TRIGGER
صحيح sqlite3FixTriggerStep(
  DbFixer *pFix,     /* Context of the fixation */
  TriggerStep *pStep /* The trigger step be fixed to one database */
){
  طالما( pStep ){
    إذا( sqlite3WalkSelect(&pFix->w, pStep->pSelect)
     || sqlite3_Walk_فرچ(&pFix->w, pStep->pWhere) 
     || sqlite3_Walk_فرچ_List(&pFix->w, pStep->p_فرچ_List)
     || sqlite3FixSrcList(pFix, pStep->pSrc)
    ){
      أرجع 1;
    }
#ifndef SQLITE_OMIT_UPSERT
    {
      Upsert *pUp;
      لكل(pUp=pStep->pUpsert; pUp; pUp=pUp->pNextUpsert){
        إذا( sqlite3_Walk_فرچ_List(&pFix->w, pUp->pUpsertTarget)
         || sqlite3_Walk_فرچ(&pFix->w, pUp->pUpsertTargetWhere)
         || sqlite3_Walk_فرچ_List(&pFix->w, pUp->pUpsertSet)
         || sqlite3_Walk_فرچ(&pFix->w, pUp->pUpsertWhere)
        ){
          أرجع 1;
        }
      }
    }
#endif
    pStep = pStep->pNext;
  }

  أرجع 0;
}
#endif
