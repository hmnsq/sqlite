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
#অন্তর্ভাব "sqliteInt.h"

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
স্থির পূর্ণসংখ্যা resolve_Attach_রাশিমালা(NameContext *pName, রাশিমালা *p_রাশিমালা)
{
  পূর্ণসংখ্যা rc = SQLITE_OK;
  যদি( p_রাশিমালা ){
    যদি( p_রাশিমালা->op!=TK_ID ){
      rc = sqlite3_Resolve_রাশিমালা_Names(pName, p_রাশিমালা);
    }অন্যথা{
      p_রাশিমালা->op = TK_STRING;
    }
  }
  ফেরত rc;
}

/*
** Return true if zName points to a name that may be used to refer to
** database iDb attached to handle db.
*/
পূর্ণসংখ্যা sqlite3DbIsNamed(sqlite3 *db, পূর্ণসংখ্যা iDb, ধ্রুবক অক্ষর *zName){
  ফেরত (
      sqlite3StrICmp(db->aDb[iDb].zDbSName, zName)==0
   || (iDb==0 && sqlite3StrICmp("main", zName)==0)
  );
}

/*
** If any TEMP triggers reference schema pSchema, move those triggers to
** reference the TEMP schema itself.
*/
স্থির শূন্য detach_অস্থায়ী_Triggers(sqlite3 *db, Schema *pSchema){
  HashElem *pEntry;
  প্রতিজ্ঞা( db->aDb[1].pSchema );
  pEntry = sqliteHashFirst(&db->aDb[1].pSchema->trigHash);
  যখন( pEntry ){
    Trigger *pTrig = (Trigger*)sqlite_Hash_তথ্য(pEntry);
    যদি( pTrig->pTabSchema==pSchema ){
      pTrig->pTabSchema = pTrig->pSchema;
    }
    pEntry = sqlite_Hash_পরবর্তী(pEntry);
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
  পূর্ণসংখ্যা NotUsed,
  sqlite3_মান **argv
){
  পূর্ণসংখ্যা i;
  পূর্ণসংখ্যা rc = 0;
  sqlite3 *db = sqlite3_context_db_handle(context);
  ধ্রুবক অক্ষর *zName;
  ধ্রুবক অক্ষর *zFile;
  অক্ষর *zPath = 0;
  অক্ষর *zErr = 0;
  unsigned পূর্ণসংখ্যা flags;
  Db *aNew;                 /* New array of Db pointers */
  Db *pNew = 0;             /* Db object for the newly attached database */
  অক্ষর *zErrDyn = 0;
  sqlite3_vfs *pVfs;

  UNUSED_PARAMETER(NotUsed);
  zFile = (ধ্রুবক অক্ষর *)sqlite3_মান_text(argv[0]);
  zName = (ধ্রুবক অক্ষর *)sqlite3_মান_text(argv[1]);
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
      যাও attach_error;
    }

    pVfs = sqlite3_vfs_find("memdb");
    যদি( pVfs==0 ) ফেরত;
    rc = sqlite3BtreeOpen(pVfs, "x\0", db, &pNewBt, 0, SQLITE_OPEN_MAIN_DB);
    যদি( rc==SQLITE_OK ){
      Schema *pNewSchema = sqlite3SchemaGet(db, pNewBt);
      যদি( pNewSchema ){
        /* Both the Btree and the new Schema were allocated successfully.
        ** Close the old db and update the aDb[] slot with the new memdb
        ** values.  */
        detach_অস্থায়ী_Triggers(db, pNew->pSchema);
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
    যদি( rc ) যাও attach_error;
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
      যাও attach_error;
    }
    জন্য(i=0; i<db->nDb; i++){
      প্রতিজ্ঞা( zName );
      যদি( sqlite3DbIsNamed(db, i, zName) ){
        zErrDyn = sqlite3MPrintf(db, "database %s is already in use", zName);
        যাও attach_error;
      }
    }
  
    /* Allocate the new entry in the db->aDb[] array and initialize the schema
    ** hash tables.
    */
    যদি( db->aDb==db->aDbStatic ){
      aNew = sqlite3DbMallocRawNN(db, আকার(db->aDb[0])*3 );
      যদি( aNew==0 ) ফেরত;
      memcpy(aNew, db->aDb, আকার(db->aDb[0])*2);
    }অন্যথা{
      aNew = sqlite3DbRealloc(db, db->aDb, আকার(db->aDb[0])*(1+(i64)db->nDb));
      যদি( aNew==0 ) ফেরত;
    }
    db->aDb = aNew;
    pNew = &db->aDb[db->nDb];
    memset(pNew, 0, আকার(*pNew));
  
    /* Open the database file. If the btree is successfully opened, use
    ** it to obtain the database schema. At this point the schema may
    ** or may not be initialized.
    */
    flags = db->openFlags;
    rc = sqlite3ParseUri(db->pVfs->zName, zFile, &flags, &pVfs, &zPath, &zErr);
    যদি( rc!=SQLITE_OK ){
      যদি( rc==SQLITE_NOMEM ) sqlite3OomFault(db);
      sqlite3_ফলাফল_error(context, zErr, -1);
      sqlite3_free(zErr);
      ফেরত;
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
      পূর্ণসংখ্যা val = 1;
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
      পূর্ণসংখ্যা iDb = db->nDb - 1;
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
    যাও attach_error;
  }
  
  ফেরত;

attach_error:
  /* Return an error if we get here */
  যদি( zErrDyn ){
    sqlite3_ফলাফল_error(context, zErrDyn, -1);
    sqlite3DbFree(db, zErrDyn);
  }
  যদি( rc ) sqlite3_ফলাফল_error_code(context, rc);
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
  পূর্ণসংখ্যা NotUsed,
  sqlite3_মান **argv
){
  ধ্রুবক অক্ষর *zName = (ধ্রুবক অক্ষর *)sqlite3_মান_text(argv[0]);
  sqlite3 *db = sqlite3_context_db_handle(context);
  পূর্ণসংখ্যা i;
  Db *pDb = 0;
  অক্ষর zErr[128];

  UNUSED_PARAMETER(NotUsed);

  যদি( zName==0 ) zName = "";
  জন্য(i=0; i<db->nDb; i++){
    pDb = &db->aDb[i];
    যদি( pDb->pBt==0 ) চালু;
    যদি( sqlite3DbIsNamed(db, i, zName) ) বিরতি;
  }

  যদি( i>=db->nDb ){
    sqlite3_snprintf(আকার(zErr),zErr, "no such database: %s", zName);
    যাও detach_error;
  }
  যদি( i<2 ){
    sqlite3_snprintf(আকার(zErr),zErr, "cannot detach database %s", zName);
    যাও detach_error;
  }
  যদি( sqlite3BtreeTxnState(pDb->pBt)!=SQLITE_TXN_NONE
   || sqlite3BtreeIsInBackup(pDb->pBt)
  ){
    sqlite3_snprintf(আকার(zErr),zErr, "database %s is locked", zName);
    যাও detach_error;
  }

  detach_অস্থায়ী_Triggers(db, pDb->pSchema);
  sqlite3BtreeClose(pDb->pBt);
  pDb->pBt = 0;
  pDb->pSchema = 0;
  sqlite3CollapseDatabaseArray(db);
  ফেরত;

detach_error:
  sqlite3_ফলাফল_error(context, zErr, -1);
}

/*
** This procedure generates VDBE code for a single invocation of either the
** sqlite_detach() or sqlite_attach() SQL user functions.
*/
স্থির শূন্য codeAttach(
  Parse *pParse,       /* The parser context */
  পূর্ণসংখ্যা type,            /* Either SQLITE_ATTACH or SQLITE_DETACH */
  FuncDef ধ্রুবক *pFunc,/* FuncDef wrapper for detachFunc() or attachFunc() */
  রাশিমালা *pAuthArg,      /* Expression to pass to authorization callback */
  রাশিমালা *pFilename,     /* Name of database file */
  রাশিমালা *pDbname,       /* Name of the database to use internally */
  রাশিমালা *p_চাবি           /* Database key for encryption extension */
){
  পূর্ণসংখ্যা rc;
  NameContext sName;
  Vdbe *v;
  sqlite3* db = pParse->db;
  পূর্ণসংখ্যা regArgs;

  যদি( SQLITE_OK!=sqlite3ReadSchema(pParse) ) যাও attach_end;

  যদি( pParse->nErr ) যাও attach_end;
  memset(&sName, 0, আকার(NameContext));
  sName.pParse = pParse;

  যদি( 
      SQLITE_OK!=resolve_Attach_রাশিমালা(&sName, pFilename) ||
      SQLITE_OK!=resolve_Attach_রাশিমালা(&sName, pDbname) ||
      SQLITE_OK!=resolve_Attach_রাশিমালা(&sName, p_চাবি)
  ){
    যাও attach_end;
  }

#ifndef SQLITE_OMIT_AUTHORIZATION
  যদি( ALWAYS(pAuthArg) ){
    অক্ষর *zAuthArg;
    যদি( pAuthArg->op==TK_STRING ){
      প্রতিজ্ঞা( !রাশিমালা_Has_Property(pAuthArg, EP_Int_মান) );
      zAuthArg = pAuthArg->u.zToken;
    }অন্যথা{
      zAuthArg = 0;
    }
    rc = sqlite3AuthCheck(pParse, type, zAuthArg, 0, 0);
    যদি(rc!=SQLITE_OK ){
      যাও attach_end;
    }
  }
#endif /* SQLITE_OMIT_AUTHORIZATION */


  v = sqlite3GetVdbe(pParse);
  regArgs = sqlite3_Get_অস্থায়ী_Range(pParse, 4);
  sqlite3_রাশিমালা_Code(pParse, pFilename, regArgs);
  sqlite3_রাশিমালা_Code(pParse, pDbname, regArgs+1);
  sqlite3_রাশিমালা_Code(pParse, p_চাবি, regArgs+2);

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
  sqlite3_রাশিমালা_Delete(db, pFilename);
  sqlite3_রাশিমালা_Delete(db, pDbname);
  sqlite3_রাশিমালা_Delete(db, p_চাবি);
}

/*
** Called by the parser to compile a DETACH statement.
**
**     DETACH pDbname
*/
শূন্য sqlite3Detach(Parse *pParse, রাশিমালা *pDbname){
  স্থির ধ্রুবক FuncDef detach_func = {
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
শূন্য sqlite3Attach(Parse *pParse, রাশিমালা *p, রাশিমালা *pDbname, রাশিমালা *p_চাবি){
  স্থির ধ্রুবক FuncDef attach_func = {
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
স্থির পূর্ণসংখ্যা fix_রাশিমালা_Cb(Walker *p, রাশিমালা *p_রাশিমালা){
  DbFixer *pFix = p->u.pFix;
  যদি( !pFix->b_অস্থায়ী ) রাশিমালা_Set_Property(p_রাশিমালা, EP_FromDDL);
  যদি( p_রাশিমালা->op==TK_VARIABLE ){
    যদি( pFix->pParse->db->init.busy ){
      p_রাশিমালা->op = TK_NULL;
    }অন্যথা{
      sqlite3_Error_বার্তা(pFix->pParse, "%s cannot use variables", pFix->zType);
      ফেরত WRC_Abort;
    }
  }
  ফেরত WRC_Continue;
}

/*
** Select callback used by sqlite3FixAAAA() routines.
*/
স্থির পূর্ণসংখ্যা fixSelectCb(Walker *p, Select *pSelect){
  DbFixer *pFix = p->u.pFix;
  পূর্ণসংখ্যা i;
  SrcItem *pItem;
  sqlite3 *db = pFix->pParse->db;
  পূর্ণসংখ্যা iDb = sqlite3FindDbName(db, pFix->zDb);
  SrcList *pList = pSelect->pSrc;

  যদি( NEVER(pList==0) ) ফেরত WRC_Continue;
  জন্য(i=0, pItem=pList->a; i<pList->nSrc; i++, pItem++){
    যদি( pFix->b_অস্থায়ী==0 && pItem->fg.isSubquery==0 ){
      যদি( pItem->fg.fixedSchema==0 && pItem->u4.zDatabase!=0 ){
        যদি( iDb!=sqlite3FindDbName(db, pItem->u4.zDatabase) ){
          sqlite3_Error_বার্তা(pFix->pParse,
              "%s %T cannot reference objects in database %s",
              pFix->zType, pFix->pName, pItem->u4.zDatabase);
          ফেরত WRC_Abort;
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
     && sqlite3_Walk_রাশিমালা(&pFix->w, pList->a[i].u3.pOn)
    ){
      ফেরত WRC_Abort;
    }
#endif
  }
  যদি( pSelect->pWith ){
    জন্য(i=0; i<pSelect->pWith->nCte; i++){
      যদি( sqlite3WalkSelect(p, pSelect->pWith->a[i].pSelect) ){
        ফেরত WRC_Abort;
      }
    }
  }
  ফেরত WRC_Continue;
}

/*
** Initialize a DbFixer structure.  This routine must be called prior
** to passing the structure to one of the sqliteFixAAAA() routines below.
*/
শূন্য sqlite3FixInit(
  DbFixer *pFix,      /* The fixer to be initialized */
  Parse *pParse,      /* Error messages will be written here */
  পূর্ণসংখ্যা iDb,            /* This is the database that must be used */
  ধ্রুবক অক্ষর *zType,  /* "view", "trigger", or "index" */
  ধ্রুবক Token *pName  /* Name of the view, trigger, or index */
){
  sqlite3 *db = pParse->db;
  প্রতিজ্ঞা( db->nDb>iDb );
  pFix->pParse = pParse;
  pFix->zDb = db->aDb[iDb].zDbSName;
  pFix->pSchema = db->aDb[iDb].pSchema;
  pFix->zType = zType;
  pFix->pName = pName;
  pFix->b_অস্থায়ী = (iDb==1);
  pFix->w.pParse = pParse;
  pFix->w.x_রাশিমালা_Callback = fix_রাশিমালা_Cb;
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
পূর্ণসংখ্যা sqlite3FixSrcList(
  DbFixer *pFix,       /* Context of the fixation */
  SrcList *pList       /* The Source list to check and modify */
){
  পূর্ণসংখ্যা res = 0;
  যদি( pList ){
    Select s; 
    memset(&s, 0, আকার(s));
    s.pSrc = pList;
    res = sqlite3WalkSelect(&pFix->w, &s);
  }
  ফেরত res;
}
#যদি !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_TRIGGER)
পূর্ণসংখ্যা sqlite3FixSelect(
  DbFixer *pFix,       /* Context of the fixation */
  Select *pSelect      /* The SELECT statement to be fixed to one database */
){
  ফেরত sqlite3WalkSelect(&pFix->w, pSelect);
}
পূর্ণসংখ্যা sqlite3_Fix_রাশিমালা(
  DbFixer *pFix,     /* Context of the fixation */
  রাশিমালা *p_রাশিমালা        /* The expression to be fixed to one database */
){
  ফেরত sqlite3_Walk_রাশিমালা(&pFix->w, p_রাশিমালা);
}
#endif

#ifndef SQLITE_OMIT_TRIGGER
পূর্ণসংখ্যা sqlite3FixTriggerStep(
  DbFixer *pFix,     /* Context of the fixation */
  TriggerStep *pStep /* The trigger step be fixed to one database */
){
  যখন( pStep ){
    যদি( sqlite3WalkSelect(&pFix->w, pStep->pSelect)
     || sqlite3_Walk_রাশিমালা(&pFix->w, pStep->pWhere) 
     || sqlite3_Walk_রাশিমালা_List(&pFix->w, pStep->p_রাশিমালা_List)
     || sqlite3FixSrcList(pFix, pStep->pSrc)
    ){
      ফেরত 1;
    }
#ifndef SQLITE_OMIT_UPSERT
    {
      Upsert *pUp;
      জন্য(pUp=pStep->pUpsert; pUp; pUp=pUp->p_পরবর্তী_Upsert){
        যদি( sqlite3_Walk_রাশিমালা_List(&pFix->w, pUp->pUpsertTarget)
         || sqlite3_Walk_রাশিমালা(&pFix->w, pUp->pUpsertTargetWhere)
         || sqlite3_Walk_রাশিমালা_List(&pFix->w, pUp->pUpsertSet)
         || sqlite3_Walk_রাশিমালা(&pFix->w, pUp->pUpsertWhere)
        ){
          ফেরত 1;
        }
      }
    }
#endif
    pStep = pStep->p_পরবর্তী;
  }

  ফেরত 0;
}
#endif
