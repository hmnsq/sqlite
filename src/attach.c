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

#که_نه_تعریف_شوی SQLITE_OMIT_ATTACH
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
ساكن صحيح resolve_Attach_بیان(NameContext *pName, بیان *p_بیان)
{
  صحيح rc = SQLITE_OK;
  إذا( p_بیان ){
    إذا( p_بیان->op!=TK_ID ){
      rc = sqlite3_Resolve_بیان_نومونه(pName, p_بیان);
    }وإلا{
      p_بیان->op = TK_STRING;
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
ساكن فراغ detach_لنډمهاله_Triggers(sqlite3 *db, Schema *pSchema){
  HashElem *p_ثبت;
  أكد( db->aDb[1].pSchema );
  p_ثبت = sqlite_Hash_لومړی(&db->aDb[1].pSchema->trigHash);
  طالما( p_ثبت ){
    Trigger *pTrig = (Trigger*)sqlite_Hash_معلومات(p_ثبت);
    إذا( pTrig->pTabSchema==pSchema ){
      pTrig->pTabSchema = pTrig->pSchema;
    }
    p_ثبت = sqlite_Hash_بل(p_ثبت);
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
  صحيح Not_کارول_شوی,
  sqlite3_ارزښت **argv
){
  صحيح i;
  صحيح rc = 0;
  sqlite3 *db = sqlite3_context_db_handle(context);
  ثابت حرف *zName;
  ثابت حرف *zFile;
  حرف *z_لار = 0;
  حرف *zErr = 0;
  بینښه صحيح flags;
  Db *aNew;                 /* New array of Db pointers */
  Db *pNew = 0;             /* Db object for the newly attached database */
  حرف *zErrDyn = 0;
  sqlite3_vfs *pVfs;

  ناکارول_شوی_PARAMETER(Not_کارول_شوی);
  zFile = (ثابت حرف *)sqlite3_ارزښت_متن(argv[0]);
  zName = (ثابت حرف *)sqlite3_ارزښت_متن(argv[1]);
  إذا( zFile==0 ) zFile = "";
  إذا( zName==0 ) zName = "";

#که_نه_تعریف_شوی SQLITE_OMIT_DESERIALIZE
# define REOPEN_AS_MEMDB(db)  (db->init.reopenMemdb)
#وإلا
# define REOPEN_AS_MEMDB(db)  (0)
#د_که_پای

  إذا( REOPEN_AS_MEMDB(db) ){
    /* This is not a real ATTACH.  Instead, this routine is being called
    ** from sqlite3_deserialize() to close database db->init.iDb and
    ** reopen it as a MemDB */
    Btree *pNewBt = 0;

    pNew = &db->aDb[db->init.iDb];
    أكد( pNew->pBt!=0 );
    إذا( sqlite3BtreeTxnState(pNew->pBt)!=SQLITE_TXN_NONE
     || sqlite3_Btree_Is_دننه_Backup(pNew->pBt)
    ){
      rc = SQLITE_BUSY;
      اذهب attach_error;
    }

    pVfs = sqlite3_vfs_پیدا_کړه("memdb");
    إذا( pVfs==0 ) أرجع;
    rc = sqlite3BtreeOpen(pVfs, "x\0", db, &pNewBt, 0, SQLITE_OPEN_MAIN_DB);
    إذا( rc==SQLITE_OK ){
      Schema *pNewSchema = sqlite3SchemaGet(db, pNewBt);
      إذا( pNewSchema ){
        /* Both the Btree and the new Schema were allocated successfully.
        ** Close the old db and update the aDb[] slot with the new memdb
        ** values.  */
        detach_لنډمهاله_Triggers(db, pNew->pSchema);
        sqlite3BtreeClose(pNew->pBt);
        pNew->pBt = pNewBt;
        pNew->pSchema = pNewSchema;
        إذا( db->init.iDb==0 ){
          /* Clear all eponymous virtual table instances, as they are holding
          ** pointers to the schema object just freed by sqlite3BtreeClose() */
          sqlite3_Vtab_Eponymous_جدول_Clear_All(db);
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
    إذا( db->nDb>=db->a_حد[SQLITE_حد_ATTACHED]+2 ){
      zErrDyn = sqlite3MPrintf(db, "too many attached databases - max %d", 
        db->a_حد[SQLITE_حد_ATTACHED]
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
      حافظه_نقل(aNew, db->aDb, حجم(db->aDb[0])*2);
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
    rc = sqlite3ParseUri(db->pVfs->zName, zFile, &flags, &pVfs, &z_لار, &zErr);
    إذا( rc!=SQLITE_OK ){
      إذا( rc==SQLITE_NOMEM ) sqlite3OomFault(db);
      sqlite3_پایله_error(context, zErr, -1);
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
    rc = sqlite3BtreeOpen(pVfs, z_لار, db, &pNew->pBt, 0, flags);
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
    sqlite3_Pager_Locking_ډېرپېښ(pPager, db->dflt_قلف_ډېرپېښ);
    sqlite3BtreeSecureDelete(pNew->pBt,
                             sqlite3BtreeSecureDelete(db->aDb[0].pBt,-1) );
#که_نه_تعریف_شوی SQLITE_OMIT_PAGER_PRAGMAS
    sqlite3_Btree_سېټ_Pager_Flags(pNew->pBt,
                      PAGER_SYNCHRONOUS_ډک | (db->flags & PAGER_FLAGS_ماسک));
#د_که_پای
    sqlite3BtreeLeave(pNew->pBt);
  }
  pNew->safety_level = SQLITE_DEFAULT_SYNCHRONOUS+1;
  إذا( rc==SQLITE_OK && pNew->zDbSName==0 ){
    rc = SQLITE_NOMEM_BKPT;
  }
  sqlite3_free_filename( z_لار );

  /* If the file was opened successfully, read the schema for the new database.
  ** If this fails, or if opening the file failed, then close the file and 
  ** remove the entry from the db->aDb[] array. i.e. put everything back the
  ** way we found it.
  */
  إذا( rc==SQLITE_OK ){
    sqlite3BtreeEnterAll(db);
    db->init.iDb = 0;
    db->mDbFlags &= ~(DBFLAG_Schema_معلوم_Ok);
#که_تعریف_شوی SQLITE_ENABLE_SETLK_TIMEOUT
    إذا( db->setlkFlags & SQLITE_SETLK_بلاک_ON_CONNECT ){
      صحيح val = 1;
      sqlite3_file *fd = sqlite3PagerFile(sqlite3BtreePager(pNew->pBt));
      sqlite3OsFileControlHint(fd, SQLITE_FCNTL_بلاک_ON_CONNECT, &val);
    }
#د_که_پای
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
      sqlite3_Reset_All_Schemas_د_Connection(db);
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
    sqlite3_پایله_error(context, zErrDyn, -1);
    sqlite3DbFree(db, zErrDyn);
  }
  إذا( rc ) sqlite3_پایله_error_کوډ(context, rc);
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
  صحيح Not_کارول_شوی,
  sqlite3_ارزښت **argv
){
  ثابت حرف *zName = (ثابت حرف *)sqlite3_ارزښت_متن(argv[0]);
  sqlite3 *db = sqlite3_context_db_handle(context);
  صحيح i;
  Db *pDb = 0;
  حرف zErr[128];

  ناکارول_شوی_PARAMETER(Not_کارول_شوی);

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
   || sqlite3_Btree_Is_دننه_Backup(pDb->pBt)
  ){
    sqlite3_snprintf(حجم(zErr),zErr, "database %s is locked", zName);
    اذهب detach_error;
  }

  detach_لنډمهاله_Triggers(db, pDb->pSchema);
  sqlite3BtreeClose(pDb->pBt);
  pDb->pBt = 0;
  pDb->pSchema = 0;
  sqlite3CollapseDatabaseArray(db);
  أرجع;

detach_error:
  sqlite3_پایله_error(context, zErr, -1);
}

/*
** This procedure generates VDBE code for a single invocation of either the
** sqlite_detach() or sqlite_attach() SQL user functions.
*/
ساكن فراغ کوډ_Attach(
  Parse *pParse,       /* The parser context */
  صحيح type,            /* Either SQLITE_ATTACH or SQLITE_DETACH */
  FuncDef ثابت *pFunc,/* FuncDef wrapper for detachFunc() or attachFunc() */
  بیان *pAuthArg,      /* Expression to pass to authorization callback */
  بیان *pFilename,     /* Name of database file */
  بیان *pDbname,       /* Name of the database to use internally */
  بیان *p_کلی           /* Database key for encryption extension */
){
  صحيح rc;
  NameContext sName;
  Vdbe *v;
  sqlite3* db = pParse->db;
  صحيح reg_دلیلونه;

  إذا( SQLITE_OK!=sqlite3ReadSchema(pParse) ) اذهب attach_پای_ټکی;

  إذا( pParse->nErr ) اذهب attach_پای_ټکی;
  memset(&sName, 0, حجم(NameContext));
  sName.pParse = pParse;

  إذا( 
      SQLITE_OK!=resolve_Attach_بیان(&sName, pFilename) ||
      SQLITE_OK!=resolve_Attach_بیان(&sName, pDbname) ||
      SQLITE_OK!=resolve_Attach_بیان(&sName, p_کلی)
  ){
    اذهب attach_پای_ټکی;
  }

#که_نه_تعریف_شوی SQLITE_OMIT_AUTHORIZATION
  إذا( ALWAYS(pAuthArg) ){
    حرف *zAuthArg;
    إذا( pAuthArg->op==TK_STRING ){
      أكد( !بیان_Has_Property(pAuthArg, EP_Int_ارزښت) );
      zAuthArg = pAuthArg->u.zToken;
    }وإلا{
      zAuthArg = 0;
    }
    rc = sqlite3AuthCheck(pParse, type, zAuthArg, 0, 0);
    إذا(rc!=SQLITE_OK ){
      اذهب attach_پای_ټکی;
    }
  }
#د_که_پای /* SQLITE_OMIT_AUTHORIZATION */


  v = sqlite3GetVdbe(pParse);
  reg_دلیلونه = sqlite3_Get_لنډمهاله_Range(pParse, 4);
  sqlite3_بیان_کوډ(pParse, pFilename, reg_دلیلونه);
  sqlite3_بیان_کوډ(pParse, pDbname, reg_دلیلونه+1);
  sqlite3_بیان_کوډ(pParse, p_کلی, reg_دلیلونه+2);

  أكد( v || db->mallocFailed );
  إذا( v ){
    sqlite3_Vdbe_جمع_کړه_Function_Call(pParse, 0, reg_دلیلونه+3-pFunc->nArg, reg_دلیلونه+3,
                               pFunc->nArg, pFunc, 0);
    /* Code an OP_Expire. For an ATTACH statement, set P1 to true (expire this
    ** statement only). For DETACH, set it to false (expire all existing
    ** statements).
    */
    sqlite3_Vdbe_جمع_کړه_Op1(v, OP_Expire, (type==SQLITE_ATTACH));
  }
  
attach_پای_ټکی:
  sqlite3_بیان_Delete(db, pFilename);
  sqlite3_بیان_Delete(db, pDbname);
  sqlite3_بیان_Delete(db, p_کلی);
}

/*
** Called by the parser to compile a DETACH statement.
**
**     DETACH pDbname
*/
فراغ sqlite3Detach(Parse *pParse, بیان *pDbname){
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
  کوډ_Attach(pParse, SQLITE_DETACH, &detach_func, pDbname, 0, 0, pDbname);
}

/*
** Called by the parser to compile an ATTACH statement.
**
**     ATTACH p AS pDbname KEY pKey
*/
فراغ sqlite3Attach(Parse *pParse, بیان *p, بیان *pDbname, بیان *p_کلی){
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
  کوډ_Attach(pParse, SQLITE_ATTACH, &attach_func, p, p, pDbname, p_کلی);
}
#د_که_پای /* SQLITE_OMIT_ATTACH */

/*
** Expression callback used by sqlite3FixAAAA() routines.
*/
ساكن صحيح fix_بیان_Cb(Walker *p, بیان *p_بیان){
  DbFixer *pFix = p->u.pFix;
  إذا( !pFix->b_لنډمهاله ) بیان_سېټ_Property(p_بیان, EP_FromDDL);
  إذا( p_بیان->op==TK_VARIABLE ){
    إذا( pFix->pParse->db->init.busy ){
      p_بیان->op = TK_NULL;
    }وإلا{
      sqlite3_Error_پیغام(pFix->pParse, "%s cannot use variables", pFix->zType);
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
  Src_توکی *p_توکی;
  sqlite3 *db = pFix->pParse->db;
  صحيح iDb = sqlite3_پیدا_کړه_Db_Name(db, pFix->zDb);
  SrcList *pList = pSelect->pSrc;

  إذا( NEVER(pList==0) ) أرجع WRC_Continue;
  لكل(i=0, p_توکی=pList->a; i<pList->nSrc; i++, p_توکی++){
    إذا( pFix->b_لنډمهاله==0 && p_توکی->fg.isSubquery==0 ){
      إذا( p_توکی->fg.fixedSchema==0 && p_توکی->u4.zDatabase!=0 ){
        إذا( iDb!=sqlite3_پیدا_کړه_Db_Name(db, p_توکی->u4.zDatabase) ){
          sqlite3_Error_پیغام(pFix->pParse,
              "%s %T cannot reference objects in database %s",
              pFix->zType, pFix->pName, p_توکی->u4.zDatabase);
          أرجع WRC_Abort;
        }
        sqlite3DbFree(db, p_توکی->u4.zDatabase);
        p_توکی->fg.notCte = 1;
        p_توکی->fg.hadSchema = 1;
      }
      p_توکی->u4.pSchema = pFix->pSchema;
      p_توکی->fg.fromDDL = 1;
      p_توکی->fg.fixedSchema = 1;
    }
#إذا !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_TRIGGER)
    إذا( pList->a[i].fg.isUsing==0
     && sqlite3_Walk_بیان(&pFix->w, pList->a[i].u3.pOn)
    ){
      أرجع WRC_Abort;
    }
#د_که_پای
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
  pFix->b_لنډمهاله = (iDb==1);
  pFix->w.pParse = pParse;
  pFix->w.x_بیان_Callback = fix_بیان_Cb;
  pFix->w.xSelectCallback = fixSelectCb;
  pFix->w.xSelectCallback2 = sqlite3WalkWinDefnDummyCallback;
  pFix->w.walker_ژوروالی = 0;
  pFix->w.e_کوډ = 0;
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
صحيح sqlite3_Fix_بیان(
  DbFixer *pFix,     /* Context of the fixation */
  بیان *p_بیان        /* The expression to be fixed to one database */
){
  أرجع sqlite3_Walk_بیان(&pFix->w, p_بیان);
}
#د_که_پای

#که_نه_تعریف_شوی SQLITE_OMIT_TRIGGER
صحيح sqlite3_Fix_Trigger_ګام(
  DbFixer *pFix,     /* Context of the fixation */
  Trigger_ګام *p_ګام /* The trigger step be fixed to one database */
){
  طالما( p_ګام ){
    إذا( sqlite3WalkSelect(&pFix->w, p_ګام->pSelect)
     || sqlite3_Walk_بیان(&pFix->w, p_ګام->pWhere) 
     || sqlite3_Walk_بیان_List(&pFix->w, p_ګام->p_بیان_List)
     || sqlite3FixSrcList(pFix, p_ګام->pSrc)
    ){
      أرجع 1;
    }
#که_نه_تعریف_شوی SQLITE_OMIT_UPSERT
    {
      Upsert *p_پورته;
      لكل(p_پورته=p_ګام->pUpsert; p_پورته; p_پورته=p_پورته->p_بل_Upsert){
        إذا( sqlite3_Walk_بیان_List(&pFix->w, p_پورته->p_Upsert_موخه)
         || sqlite3_Walk_بیان(&pFix->w, p_پورته->p_Upsert_موخه_Where)
         || sqlite3_Walk_بیان_List(&pFix->w, p_پورته->p_Upsert_سېټ)
         || sqlite3_Walk_بیان(&pFix->w, p_پورته->pUpsertWhere)
        ){
          أرجع 1;
        }
      }
    }
#د_که_پای
    p_ګام = p_ګام->p_بل;
  }

  أرجع 0;
}
#د_که_پای
