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
#समावेश "sqliteInt.h"

#अगर_अघोषित SQLITE_OMIT_ATTACH
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
अटल पूर्णांक resolve_Attach_व्यंजक(NameContext *pName, व्यंजक *p_व्यंजक)
{
  पूर्णांक rc = SQLITE_OK;
  अगर( p_व्यंजक ){
    अगर( p_व्यंजक->op!=TK_ID ){
      rc = sqlite3_Resolve_व्यंजक_Names(pName, p_व्यंजक);
    }अन्यथा{
      p_व्यंजक->op = TK_STRING;
    }
  }
  वापस rc;
}

/*
** Return true if zName points to a name that may be used to refer to
** database iDb attached to handle db.
*/
पूर्णांक sqlite3DbIsNamed(sqlite3 *db, पूर्णांक iDb, स्थिर अक्षर *zName){
  वापस (
      sqlite3StrICmp(db->aDb[iDb].zDbSName, zName)==0
   || (iDb==0 && sqlite3StrICmp("main", zName)==0)
  );
}

/*
** If any TEMP triggers reference schema pSchema, move those triggers to
** reference the TEMP schema itself.
*/
अटल व्योम detach_अस्थायी_Triggers(sqlite3 *db, Schema *pSchema){
  HashElem *pEntry;
  निश्चित( db->aDb[1].pSchema );
  pEntry = sqliteHashFirst(&db->aDb[1].pSchema->trigHash);
  जबतक( pEntry ){
    Trigger *pTrig = (Trigger*)sqlite_Hash_तथ्य(pEntry);
    अगर( pTrig->pTabSchema==pSchema ){
      pTrig->pTabSchema = pTrig->pSchema;
    }
    pEntry = sqlite_Hash_अग्र(pEntry);
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
अटल व्योम attachFunc(
  sqlite3_context *context,
  पूर्णांक NotUsed,
  sqlite3_मूल्य **argv
){
  पूर्णांक i;
  पूर्णांक rc = 0;
  sqlite3 *db = sqlite3_context_db_handle(context);
  स्थिर अक्षर *zName;
  स्थिर अक्षर *zFile;
  अक्षर *zPath = 0;
  अक्षर *zErr = 0;
  अचिन्हित पूर्णांक flags;
  Db *aNew;                 /* New array of Db pointers */
  Db *pNew = 0;             /* Db object for the newly attached database */
  अक्षर *zErrDyn = 0;
  sqlite3_vfs *pVfs;

  UNUSED_PARAMETER(NotUsed);
  zFile = (स्थिर अक्षर *)sqlite3_मूल्य_text(argv[0]);
  zName = (स्थिर अक्षर *)sqlite3_मूल्य_text(argv[1]);
  अगर( zFile==0 ) zFile = "";
  अगर( zName==0 ) zName = "";

#अगर_अघोषित SQLITE_OMIT_DESERIALIZE
# define REOPEN_AS_MEMDB(db)  (db->init.reopenMemdb)
#अन्यथा
# define REOPEN_AS_MEMDB(db)  (0)
#पूर्ण_अगर

  अगर( REOPEN_AS_MEMDB(db) ){
    /* This is not a real ATTACH.  Instead, this routine is being called
    ** from sqlite3_deserialize() to close database db->init.iDb and
    ** reopen it as a MemDB */
    Btree *pNewBt = 0;

    pNew = &db->aDb[db->init.iDb];
    निश्चित( pNew->pBt!=0 );
    अगर( sqlite3BtreeTxnState(pNew->pBt)!=SQLITE_TXN_NONE
     || sqlite3BtreeIsInBackup(pNew->pBt)
    ){
      rc = SQLITE_BUSY;
      जाओ attach_error;
    }

    pVfs = sqlite3_vfs_find("memdb");
    अगर( pVfs==0 ) वापस;
    rc = sqlite3BtreeOpen(pVfs, "x\0", db, &pNewBt, 0, SQLITE_OPEN_MAIN_DB);
    अगर( rc==SQLITE_OK ){
      Schema *pNewSchema = sqlite3SchemaGet(db, pNewBt);
      अगर( pNewSchema ){
        /* Both the Btree and the new Schema were allocated successfully.
        ** Close the old db and update the aDb[] slot with the new memdb
        ** values.  */
        detach_अस्थायी_Triggers(db, pNew->pSchema);
        sqlite3BtreeClose(pNew->pBt);
        pNew->pBt = pNewBt;
        pNew->pSchema = pNewSchema;
        अगर( db->init.iDb==0 ){
          /* Clear all eponymous virtual table instances, as they are holding
          ** pointers to the schema object just freed by sqlite3BtreeClose() */
          sqlite3VtabEponymousTableClearAll(db);
        }
        sqlite3ExpirePreparedStatements(db, 1);
      }अन्यथा{
        sqlite3BtreeClose(pNewBt);
        rc = SQLITE_NOMEM;
      }
    }
    अगर( rc ) जाओ attach_error;
  }अन्यथा{
    /* This is a real ATTACH
    **
    ** Check for the following errors:
    **
    **     * Too many attached databases,
    **     * Transaction currently open
    **     * Specified database name already being used.
    */
    अगर( db->nDb>=db->aLimit[SQLITE_LIMIT_ATTACHED]+2 ){
      zErrDyn = sqlite3MPrintf(db, "too many attached databases - max %d", 
        db->aLimit[SQLITE_LIMIT_ATTACHED]
      );
      जाओ attach_error;
    }
    क्रम(i=0; i<db->nDb; i++){
      निश्चित( zName );
      अगर( sqlite3DbIsNamed(db, i, zName) ){
        zErrDyn = sqlite3MPrintf(db, "database %s is already in use", zName);
        जाओ attach_error;
      }
    }
  
    /* Allocate the new entry in the db->aDb[] array and initialize the schema
    ** hash tables.
    */
    अगर( db->aDb==db->aDbStatic ){
      aNew = sqlite3DbMallocRawNN(db, माप(db->aDb[0])*3 );
      अगर( aNew==0 ) वापस;
      स_नकल(aNew, db->aDb, माप(db->aDb[0])*2);
    }अन्यथा{
      aNew = sqlite3DbRealloc(db, db->aDb, माप(db->aDb[0])*(1+(i64)db->nDb));
      अगर( aNew==0 ) वापस;
    }
    db->aDb = aNew;
    pNew = &db->aDb[db->nDb];
    स_रखो(pNew, 0, माप(*pNew));
  
    /* Open the database file. If the btree is successfully opened, use
    ** it to obtain the database schema. At this point the schema may
    ** or may not be initialized.
    */
    flags = db->openFlags;
    rc = sqlite3ParseUri(db->pVfs->zName, zFile, &flags, &pVfs, &zPath, &zErr);
    अगर( rc!=SQLITE_OK ){
      अगर( rc==SQLITE_NOMEM ) sqlite3OomFault(db);
      sqlite3_परिणाम_error(context, zErr, -1);
      sqlite3_free(zErr);
      वापस;
    }
    अगर( (db->flags & SQLITE_AttachWrite)==0 ){
      flags &= ~(SQLITE_OPEN_CREATE|SQLITE_OPEN_READWRITE);
      flags |= SQLITE_OPEN_READONLY;
    }अन्यथा अगर( (db->flags & SQLITE_AttachCreate)==0 ){
      flags &= ~SQLITE_OPEN_CREATE;
    }
    निश्चित( pVfs );
    flags |= SQLITE_OPEN_MAIN_DB;
    rc = sqlite3BtreeOpen(pVfs, zPath, db, &pNew->pBt, 0, flags);
    db->nDb++;
    pNew->zDbSName = sqlite3DbStrDup(db, zName);
  }
  db->noSharedCache = 0;
  अगर( rc==SQLITE_CONSTRAINT ){
    rc = SQLITE_ERROR;
    zErrDyn = sqlite3MPrintf(db, "database is already attached");
  }अन्यथा अगर( rc==SQLITE_OK ){
    Pager *pPager;
    pNew->pSchema = sqlite3SchemaGet(db, pNew->pBt);
    अगर( !pNew->pSchema ){
      rc = SQLITE_NOMEM_BKPT;
    }अन्यथा अगर( pNew->pSchema->file_format && pNew->pSchema->enc!=ENC(db) ){
      zErrDyn = sqlite3MPrintf(db, 
        "attached databases must use the same text encoding as main database");
      rc = SQLITE_ERROR;
    }
    sqlite3BtreeEnter(pNew->pBt);
    pPager = sqlite3BtreePager(pNew->pBt);
    sqlite3PagerLockingMode(pPager, db->dfltLockMode);
    sqlite3BtreeSecureDelete(pNew->pBt,
                             sqlite3BtreeSecureDelete(db->aDb[0].pBt,-1) );
#अगर_अघोषित SQLITE_OMIT_PAGER_PRAGMAS
    sqlite3BtreeSetPagerFlags(pNew->pBt,
                      PAGER_SYNCHRONOUS_FULL | (db->flags & PAGER_FLAGS_MASK));
#पूर्ण_अगर
    sqlite3BtreeLeave(pNew->pBt);
  }
  pNew->safety_level = SQLITE_DEFAULT_SYNCHRONOUS+1;
  अगर( rc==SQLITE_OK && pNew->zDbSName==0 ){
    rc = SQLITE_NOMEM_BKPT;
  }
  sqlite3_free_filename( zPath );

  /* If the file was opened successfully, read the schema for the new database.
  ** If this fails, or if opening the file failed, then close the file and 
  ** remove the entry from the db->aDb[] array. i.e. put everything back the
  ** way we found it.
  */
  अगर( rc==SQLITE_OK ){
    sqlite3BtreeEnterAll(db);
    db->init.iDb = 0;
    db->mDbFlags &= ~(DBFLAG_SchemaKnownOk);
#अगर_घोषित SQLITE_ENABLE_SETLK_TIMEOUT
    अगर( db->setlkFlags & SQLITE_SETLK_BLOCK_ON_CONNECT ){
      पूर्णांक val = 1;
      sqlite3_file *fd = sqlite3PagerFile(sqlite3BtreePager(pNew->pBt));
      sqlite3OsFileControlHint(fd, SQLITE_FCNTL_BLOCK_ON_CONNECT, &val);
    }
#पूर्ण_अगर
    अगर( !REOPEN_AS_MEMDB(db) ){
      rc = sqlite3Init(db, &zErrDyn);
    }
    sqlite3BtreeLeaveAll(db);
    निश्चित( zErrDyn==0 || rc!=SQLITE_OK );
  }
  अगर( rc ){
    अगर( ALWAYS(!REOPEN_AS_MEMDB(db)) ){
      पूर्णांक iDb = db->nDb - 1;
      निश्चित( iDb>=2 );
      अगर( db->aDb[iDb].pBt ){
        sqlite3BtreeClose(db->aDb[iDb].pBt);
        db->aDb[iDb].pBt = 0;
        db->aDb[iDb].pSchema = 0;
      }
      sqlite3ResetAllSchemasOfConnection(db);
      db->nDb = iDb;
      अगर( rc==SQLITE_NOMEM || rc==SQLITE_IOERR_NOMEM ){
        sqlite3OomFault(db);
        sqlite3DbFree(db, zErrDyn);
        zErrDyn = sqlite3MPrintf(db, "out of memory");
      }अन्यथा अगर( zErrDyn==0 ){
        zErrDyn = sqlite3MPrintf(db, "unable to open database: %s", zFile);
      }
    }
    जाओ attach_error;
  }
  
  वापस;

attach_error:
  /* Return an error if we get here */
  अगर( zErrDyn ){
    sqlite3_परिणाम_error(context, zErrDyn, -1);
    sqlite3DbFree(db, zErrDyn);
  }
  अगर( rc ) sqlite3_परिणाम_error_code(context, rc);
}

/*
** An SQL user-function registered to do the work of an DETACH statement. The
** three arguments to the function come directly from a detach statement:
**
**     DETACH DATABASE x
**
**     SELECT sqlite_detach(x)
*/
अटल व्योम detachFunc(
  sqlite3_context *context,
  पूर्णांक NotUsed,
  sqlite3_मूल्य **argv
){
  स्थिर अक्षर *zName = (स्थिर अक्षर *)sqlite3_मूल्य_text(argv[0]);
  sqlite3 *db = sqlite3_context_db_handle(context);
  पूर्णांक i;
  Db *pDb = 0;
  अक्षर zErr[128];

  UNUSED_PARAMETER(NotUsed);

  अगर( zName==0 ) zName = "";
  क्रम(i=0; i<db->nDb; i++){
    pDb = &db->aDb[i];
    अगर( pDb->pBt==0 ) जारी;
    अगर( sqlite3DbIsNamed(db, i, zName) ) अवरोध;
  }

  अगर( i>=db->nDb ){
    sqlite3_snprintf(माप(zErr),zErr, "no such database: %s", zName);
    जाओ detach_error;
  }
  अगर( i<2 ){
    sqlite3_snprintf(माप(zErr),zErr, "cannot detach database %s", zName);
    जाओ detach_error;
  }
  अगर( sqlite3BtreeTxnState(pDb->pBt)!=SQLITE_TXN_NONE
   || sqlite3BtreeIsInBackup(pDb->pBt)
  ){
    sqlite3_snprintf(माप(zErr),zErr, "database %s is locked", zName);
    जाओ detach_error;
  }

  detach_अस्थायी_Triggers(db, pDb->pSchema);
  sqlite3BtreeClose(pDb->pBt);
  pDb->pBt = 0;
  pDb->pSchema = 0;
  sqlite3CollapseDatabaseArray(db);
  वापस;

detach_error:
  sqlite3_परिणाम_error(context, zErr, -1);
}

/*
** This procedure generates VDBE code for a single invocation of either the
** sqlite_detach() or sqlite_attach() SQL user functions.
*/
अटल व्योम codeAttach(
  Parse *pParse,       /* The parser context */
  पूर्णांक type,            /* Either SQLITE_ATTACH or SQLITE_DETACH */
  FuncDef स्थिर *pFunc,/* FuncDef wrapper for detachFunc() or attachFunc() */
  व्यंजक *pAuthArg,      /* Expression to pass to authorization callback */
  व्यंजक *pFilename,     /* Name of database file */
  व्यंजक *pDbname,       /* Name of the database to use internally */
  व्यंजक *p_कुंजी           /* Database key for encryption extension */
){
  पूर्णांक rc;
  NameContext sName;
  Vdbe *v;
  sqlite3* db = pParse->db;
  पूर्णांक regArgs;

  अगर( SQLITE_OK!=sqlite3ReadSchema(pParse) ) जाओ attach_end;

  अगर( pParse->nErr ) जाओ attach_end;
  स_रखो(&sName, 0, माप(NameContext));
  sName.pParse = pParse;

  अगर( 
      SQLITE_OK!=resolve_Attach_व्यंजक(&sName, pFilename) ||
      SQLITE_OK!=resolve_Attach_व्यंजक(&sName, pDbname) ||
      SQLITE_OK!=resolve_Attach_व्यंजक(&sName, p_कुंजी)
  ){
    जाओ attach_end;
  }

#अगर_अघोषित SQLITE_OMIT_AUTHORIZATION
  अगर( ALWAYS(pAuthArg) ){
    अक्षर *zAuthArg;
    अगर( pAuthArg->op==TK_STRING ){
      निश्चित( !व्यंजक_Has_Property(pAuthArg, EP_Int_मूल्य) );
      zAuthArg = pAuthArg->u.zToken;
    }अन्यथा{
      zAuthArg = 0;
    }
    rc = sqlite3AuthCheck(pParse, type, zAuthArg, 0, 0);
    अगर(rc!=SQLITE_OK ){
      जाओ attach_end;
    }
  }
#पूर्ण_अगर /* SQLITE_OMIT_AUTHORIZATION */


  v = sqlite3GetVdbe(pParse);
  regArgs = sqlite3_Get_अस्थायी_Range(pParse, 4);
  sqlite3_व्यंजक_Code(pParse, pFilename, regArgs);
  sqlite3_व्यंजक_Code(pParse, pDbname, regArgs+1);
  sqlite3_व्यंजक_Code(pParse, p_कुंजी, regArgs+2);

  निश्चित( v || db->mallocFailed );
  अगर( v ){
    sqlite3VdbeAddFunctionCall(pParse, 0, regArgs+3-pFunc->nArg, regArgs+3,
                               pFunc->nArg, pFunc, 0);
    /* Code an OP_Expire. For an ATTACH statement, set P1 to true (expire this
    ** statement only). For DETACH, set it to false (expire all existing
    ** statements).
    */
    sqlite3VdbeAddOp1(v, OP_Expire, (type==SQLITE_ATTACH));
  }
  
attach_end:
  sqlite3_व्यंजक_Delete(db, pFilename);
  sqlite3_व्यंजक_Delete(db, pDbname);
  sqlite3_व्यंजक_Delete(db, p_कुंजी);
}

/*
** Called by the parser to compile a DETACH statement.
**
**     DETACH pDbname
*/
व्योम sqlite3Detach(Parse *pParse, व्यंजक *pDbname){
  अटल स्थिर FuncDef detach_func = {
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
व्योम sqlite3Attach(Parse *pParse, व्यंजक *p, व्यंजक *pDbname, व्यंजक *p_कुंजी){
  अटल स्थिर FuncDef attach_func = {
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
  codeAttach(pParse, SQLITE_ATTACH, &attach_func, p, p, pDbname, p_कुंजी);
}
#पूर्ण_अगर /* SQLITE_OMIT_ATTACH */

/*
** Expression callback used by sqlite3FixAAAA() routines.
*/
अटल पूर्णांक fix_व्यंजक_Cb(Walker *p, व्यंजक *p_व्यंजक){
  DbFixer *pFix = p->u.pFix;
  अगर( !pFix->b_अस्थायी ) व्यंजक_Set_Property(p_व्यंजक, EP_FromDDL);
  अगर( p_व्यंजक->op==TK_VARIABLE ){
    अगर( pFix->pParse->db->init.busy ){
      p_व्यंजक->op = TK_NULL;
    }अन्यथा{
      sqlite3_Error_संदेश(pFix->pParse, "%s cannot use variables", pFix->zType);
      वापस WRC_Abort;
    }
  }
  वापस WRC_Continue;
}

/*
** Select callback used by sqlite3FixAAAA() routines.
*/
अटल पूर्णांक fixSelectCb(Walker *p, Select *pSelect){
  DbFixer *pFix = p->u.pFix;
  पूर्णांक i;
  SrcItem *pItem;
  sqlite3 *db = pFix->pParse->db;
  पूर्णांक iDb = sqlite3FindDbName(db, pFix->zDb);
  SrcList *pList = pSelect->pSrc;

  अगर( NEVER(pList==0) ) वापस WRC_Continue;
  क्रम(i=0, pItem=pList->a; i<pList->nSrc; i++, pItem++){
    अगर( pFix->b_अस्थायी==0 && pItem->fg.isSubquery==0 ){
      अगर( pItem->fg.fixedSchema==0 && pItem->u4.zDatabase!=0 ){
        अगर( iDb!=sqlite3FindDbName(db, pItem->u4.zDatabase) ){
          sqlite3_Error_संदेश(pFix->pParse,
              "%s %T cannot reference objects in database %s",
              pFix->zType, pFix->pName, pItem->u4.zDatabase);
          वापस WRC_Abort;
        }
        sqlite3DbFree(db, pItem->u4.zDatabase);
        pItem->fg.notCte = 1;
        pItem->fg.hadSchema = 1;
      }
      pItem->u4.pSchema = pFix->pSchema;
      pItem->fg.fromDDL = 1;
      pItem->fg.fixedSchema = 1;
    }
#अगर !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_TRIGGER)
    अगर( pList->a[i].fg.isUsing==0
     && sqlite3_Walk_व्यंजक(&pFix->w, pList->a[i].u3.pOn)
    ){
      वापस WRC_Abort;
    }
#पूर्ण_अगर
  }
  अगर( pSelect->pWith ){
    क्रम(i=0; i<pSelect->pWith->nCte; i++){
      अगर( sqlite3WalkSelect(p, pSelect->pWith->a[i].pSelect) ){
        वापस WRC_Abort;
      }
    }
  }
  वापस WRC_Continue;
}

/*
** Initialize a DbFixer structure.  This routine must be called prior
** to passing the structure to one of the sqliteFixAAAA() routines below.
*/
व्योम sqlite3FixInit(
  DbFixer *pFix,      /* The fixer to be initialized */
  Parse *pParse,      /* Error messages will be written here */
  पूर्णांक iDb,            /* This is the database that must be used */
  स्थिर अक्षर *zType,  /* "view", "trigger", or "index" */
  स्थिर Token *pName  /* Name of the view, trigger, or index */
){
  sqlite3 *db = pParse->db;
  निश्चित( db->nDb>iDb );
  pFix->pParse = pParse;
  pFix->zDb = db->aDb[iDb].zDbSName;
  pFix->pSchema = db->aDb[iDb].pSchema;
  pFix->zType = zType;
  pFix->pName = pName;
  pFix->b_अस्थायी = (iDb==1);
  pFix->w.pParse = pParse;
  pFix->w.x_व्यंजक_Callback = fix_व्यंजक_Cb;
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
पूर्णांक sqlite3FixSrcList(
  DbFixer *pFix,       /* Context of the fixation */
  SrcList *pList       /* The Source list to check and modify */
){
  पूर्णांक res = 0;
  अगर( pList ){
    Select s; 
    स_रखो(&s, 0, माप(s));
    s.pSrc = pList;
    res = sqlite3WalkSelect(&pFix->w, &s);
  }
  वापस res;
}
#अगर !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_TRIGGER)
पूर्णांक sqlite3FixSelect(
  DbFixer *pFix,       /* Context of the fixation */
  Select *pSelect      /* The SELECT statement to be fixed to one database */
){
  वापस sqlite3WalkSelect(&pFix->w, pSelect);
}
पूर्णांक sqlite3_Fix_व्यंजक(
  DbFixer *pFix,     /* Context of the fixation */
  व्यंजक *p_व्यंजक        /* The expression to be fixed to one database */
){
  वापस sqlite3_Walk_व्यंजक(&pFix->w, p_व्यंजक);
}
#पूर्ण_अगर

#अगर_अघोषित SQLITE_OMIT_TRIGGER
पूर्णांक sqlite3FixTriggerStep(
  DbFixer *pFix,     /* Context of the fixation */
  TriggerStep *pStep /* The trigger step be fixed to one database */
){
  जबतक( pStep ){
    अगर( sqlite3WalkSelect(&pFix->w, pStep->pSelect)
     || sqlite3_Walk_व्यंजक(&pFix->w, pStep->pWhere) 
     || sqlite3_Walk_व्यंजक_List(&pFix->w, pStep->p_व्यंजक_List)
     || sqlite3FixSrcList(pFix, pStep->pSrc)
    ){
      वापस 1;
    }
#अगर_अघोषित SQLITE_OMIT_UPSERT
    {
      Upsert *pUp;
      क्रम(pUp=pStep->pUpsert; pUp; pUp=pUp->p_अग्र_Upsert){
        अगर( sqlite3_Walk_व्यंजक_List(&pFix->w, pUp->pUpsertTarget)
         || sqlite3_Walk_व्यंजक(&pFix->w, pUp->pUpsertTargetWhere)
         || sqlite3_Walk_व्यंजक_List(&pFix->w, pUp->pUpsertSet)
         || sqlite3_Walk_व्यंजक(&pFix->w, pUp->pUpsertWhere)
        ){
          वापस 1;
        }
      }
    }
#पूर्ण_अगर
    pStep = pStep->p_अग्र;
  }

  वापस 0;
}
#पूर्ण_अगर
