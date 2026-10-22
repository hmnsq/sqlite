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
#अन्तर्भाव "sqliteInt.h"

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
स्थिर पूर्णाङ्क resolve_Attach_व्यञ्जकम्(NameContext *pName, व्यञ्जकम् *p_व्यञ्जकम्)
{
  पूर्णाङ्क rc = SQLITE_OK;
  सचे( p_व्यञ्जकम् ){
    सचे( p_व्यञ्जकम्->op!=TK_ID ){
      rc = sqlite3_Resolve_व्यञ्जकम्_Names(pName, p_व्यञ्जकम्);
    }अञ्ञथा{
      p_व्यञ्जकम्->op = TK_STRING;
    }
  }
  पटिनिवत्त rc;
}

/*
** Return true if zName points to a name that may be used to refer to
** database iDb attached to handle db.
*/
पूर्णाङ्क sqlite3DbIsNamed(sqlite3 *db, पूर्णाङ्क iDb, ध्रुव वर्ण *zName){
  पटिनिवत्त (
      sqlite3StrICmp(db->aDb[iDb].zDbSName, zName)==0
   || (iDb==0 && sqlite3StrICmp("main", zName)==0)
  );
}

/*
** If any TEMP triggers reference schema pSchema, move those triggers to
** reference the TEMP schema itself.
*/
स्थिर शून्य detachTempTriggers(sqlite3 *db, Schema *pSchema){
  HashElem *pEntry;
  प्रतिज्ञा( db->aDb[1].pSchema );
  pEntry = sqliteHashFirst(&db->aDb[1].pSchema->trigHash);
  याव( pEntry ){
    Trigger *pTrig = (Trigger*)sqliteHashData(pEntry);
    सचे( pTrig->pTabSchema==pSchema ){
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
स्थिर शून्य attachFunc(
  sqlite3_context *context,
  पूर्णाङ्क NotUsed,
  sqlite3_मानम् **argv
){
  पूर्णाङ्क i;
  पूर्णाङ्क rc = 0;
  sqlite3 *db = sqlite3_context_db_handle(context);
  ध्रुव वर्ण *zName;
  ध्रुव वर्ण *zFile;
  वर्ण *zPath = 0;
  वर्ण *zErr = 0;
  unsigned पूर्णाङ्क flags;
  Db *aNew;                 /* New array of Db pointers */
  Db *pNew = 0;             /* Db object for the newly attached database */
  वर्ण *zErrDyn = 0;
  sqlite3_vfs *pVfs;

  UNUSED_PARAMETER(NotUsed);
  zFile = (ध्रुव वर्ण *)sqlite3_मानम्_text(argv[0]);
  zName = (ध्रुव वर्ण *)sqlite3_मानम्_text(argv[1]);
  सचे( zFile==0 ) zFile = "";
  सचे( zName==0 ) zName = "";

#ifndef SQLITE_OMIT_DESERIALIZE
# define REOPEN_AS_MEMDB(db)  (db->init.reopenMemdb)
#अञ्ञथा
# define REOPEN_AS_MEMDB(db)  (0)
#endif

  सचे( REOPEN_AS_MEMDB(db) ){
    /* This is not a real ATTACH.  Instead, this routine is being called
    ** from sqlite3_deserialize() to close database db->init.iDb and
    ** reopen it as a MemDB */
    Btree *pNewBt = 0;

    pNew = &db->aDb[db->init.iDb];
    प्रतिज्ञा( pNew->pBt!=0 );
    सचे( sqlite3BtreeTxnState(pNew->pBt)!=SQLITE_TXN_NONE
     || sqlite3BtreeIsInBackup(pNew->pBt)
    ){
      rc = SQLITE_BUSY;
      गच्छ attach_error;
    }

    pVfs = sqlite3_vfs_find("memdb");
    सचे( pVfs==0 ) पटिनिवत्त;
    rc = sqlite3BtreeOpen(pVfs, "x\0", db, &pNewBt, 0, SQLITE_OPEN_MAIN_DB);
    सचे( rc==SQLITE_OK ){
      Schema *pNewSchema = sqlite3SchemaGet(db, pNewBt);
      सचे( pNewSchema ){
        /* Both the Btree and the new Schema were allocated successfully.
        ** Close the old db and update the aDb[] slot with the new memdb
        ** values.  */
        detachTempTriggers(db, pNew->pSchema);
        sqlite3BtreeClose(pNew->pBt);
        pNew->pBt = pNewBt;
        pNew->pSchema = pNewSchema;
        सचे( db->init.iDb==0 ){
          /* Clear all eponymous virtual table instances, as they are holding
          ** pointers to the schema object just freed by sqlite3BtreeClose() */
          sqlite3VtabEponymousTableClearAll(db);
        }
        sqlite3ExpirePreparedStatements(db, 1);
      }अञ्ञथा{
        sqlite3BtreeClose(pNewBt);
        rc = SQLITE_NOMEM;
      }
    }
    सचे( rc ) गच्छ attach_error;
  }अञ्ञथा{
    /* This is a real ATTACH
    **
    ** Check for the following errors:
    **
    **     * Too many attached databases,
    **     * Transaction currently open
    **     * Specified database name already being used.
    */
    सचे( db->nDb>=db->aLimit[SQLITE_LIMIT_ATTACHED]+2 ){
      zErrDyn = sqlite3MPrintf(db, "too many attached databases - max %d", 
        db->aLimit[SQLITE_LIMIT_ATTACHED]
      );
      गच्छ attach_error;
    }
    अत्थाय(i=0; i<db->nDb; i++){
      प्रतिज्ञा( zName );
      सचे( sqlite3DbIsNamed(db, i, zName) ){
        zErrDyn = sqlite3MPrintf(db, "database %s is already in use", zName);
        गच्छ attach_error;
      }
    }
  
    /* Allocate the new entry in the db->aDb[] array and initialize the schema
    ** hash tables.
    */
    सचे( db->aDb==db->aDbStatic ){
      aNew = sqlite3DbMallocRawNN(db, परिमाण(db->aDb[0])*3 );
      सचे( aNew==0 ) पटिनिवत्त;
      memcpy(aNew, db->aDb, परिमाण(db->aDb[0])*2);
    }अञ्ञथा{
      aNew = sqlite3DbRealloc(db, db->aDb, परिमाण(db->aDb[0])*(1+(i64)db->nDb));
      सचे( aNew==0 ) पटिनिवत्त;
    }
    db->aDb = aNew;
    pNew = &db->aDb[db->nDb];
    memset(pNew, 0, परिमाण(*pNew));
  
    /* Open the database file. If the btree is successfully opened, use
    ** it to obtain the database schema. At this point the schema may
    ** or may not be initialized.
    */
    flags = db->openFlags;
    rc = sqlite3ParseUri(db->pVfs->zName, zFile, &flags, &pVfs, &zPath, &zErr);
    सचे( rc!=SQLITE_OK ){
      सचे( rc==SQLITE_NOMEM ) sqlite3OomFault(db);
      sqlite3_result_error(context, zErr, -1);
      sqlite3_free(zErr);
      पटिनिवत्त;
    }
    सचे( (db->flags & SQLITE_AttachWrite)==0 ){
      flags &= ~(SQLITE_OPEN_CREATE|SQLITE_OPEN_READWRITE);
      flags |= SQLITE_OPEN_READONLY;
    }अञ्ञथा सचे( (db->flags & SQLITE_AttachCreate)==0 ){
      flags &= ~SQLITE_OPEN_CREATE;
    }
    प्रतिज्ञा( pVfs );
    flags |= SQLITE_OPEN_MAIN_DB;
    rc = sqlite3BtreeOpen(pVfs, zPath, db, &pNew->pBt, 0, flags);
    db->nDb++;
    pNew->zDbSName = sqlite3DbStrDup(db, zName);
  }
  db->noSharedCache = 0;
  सचे( rc==SQLITE_CONSTRAINT ){
    rc = SQLITE_ERROR;
    zErrDyn = sqlite3MPrintf(db, "database is already attached");
  }अञ्ञथा सचे( rc==SQLITE_OK ){
    Pager *pPager;
    pNew->pSchema = sqlite3SchemaGet(db, pNew->pBt);
    सचे( !pNew->pSchema ){
      rc = SQLITE_NOMEM_BKPT;
    }अञ्ञथा सचे( pNew->pSchema->file_format && pNew->pSchema->enc!=ENC(db) ){
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
  सचे( rc==SQLITE_OK && pNew->zDbSName==0 ){
    rc = SQLITE_NOMEM_BKPT;
  }
  sqlite3_free_filename( zPath );

  /* If the file was opened successfully, read the schema for the new database.
  ** If this fails, or if opening the file failed, then close the file and 
  ** remove the entry from the db->aDb[] array. i.e. put everything back the
  ** way we found it.
  */
  सचे( rc==SQLITE_OK ){
    sqlite3BtreeEnterAll(db);
    db->init.iDb = 0;
    db->mDbFlags &= ~(DBFLAG_SchemaKnownOk);
#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
    सचे( db->setlkFlags & SQLITE_SETLK_BLOCK_ON_CONNECT ){
      पूर्णाङ्क val = 1;
      sqlite3_file *fd = sqlite3PagerFile(sqlite3BtreePager(pNew->pBt));
      sqlite3OsFileControlHint(fd, SQLITE_FCNTL_BLOCK_ON_CONNECT, &val);
    }
#endif
    सचे( !REOPEN_AS_MEMDB(db) ){
      rc = sqlite3Init(db, &zErrDyn);
    }
    sqlite3BtreeLeaveAll(db);
    प्रतिज्ञा( zErrDyn==0 || rc!=SQLITE_OK );
  }
  सचे( rc ){
    सचे( ALWAYS(!REOPEN_AS_MEMDB(db)) ){
      पूर्णाङ्क iDb = db->nDb - 1;
      प्रतिज्ञा( iDb>=2 );
      सचे( db->aDb[iDb].pBt ){
        sqlite3BtreeClose(db->aDb[iDb].pBt);
        db->aDb[iDb].pBt = 0;
        db->aDb[iDb].pSchema = 0;
      }
      sqlite3ResetAllSchemasOfConnection(db);
      db->nDb = iDb;
      सचे( rc==SQLITE_NOMEM || rc==SQLITE_IOERR_NOMEM ){
        sqlite3OomFault(db);
        sqlite3DbFree(db, zErrDyn);
        zErrDyn = sqlite3MPrintf(db, "out of memory");
      }अञ्ञथा सचे( zErrDyn==0 ){
        zErrDyn = sqlite3MPrintf(db, "unable to open database: %s", zFile);
      }
    }
    गच्छ attach_error;
  }
  
  पटिनिवत्त;

attach_error:
  /* Return an error if we get here */
  सचे( zErrDyn ){
    sqlite3_result_error(context, zErrDyn, -1);
    sqlite3DbFree(db, zErrDyn);
  }
  सचे( rc ) sqlite3_result_error_code(context, rc);
}

/*
** An SQL user-function registered to do the work of an DETACH statement. The
** three arguments to the function come directly from a detach statement:
**
**     DETACH DATABASE x
**
**     SELECT sqlite_detach(x)
*/
स्थिर शून्य detachFunc(
  sqlite3_context *context,
  पूर्णाङ्क NotUsed,
  sqlite3_मानम् **argv
){
  ध्रुव वर्ण *zName = (ध्रुव वर्ण *)sqlite3_मानम्_text(argv[0]);
  sqlite3 *db = sqlite3_context_db_handle(context);
  पूर्णाङ्क i;
  Db *pDb = 0;
  वर्ण zErr[128];

  UNUSED_PARAMETER(NotUsed);

  सचे( zName==0 ) zName = "";
  अत्थाय(i=0; i<db->nDb; i++){
    pDb = &db->aDb[i];
    सचे( pDb->pBt==0 ) अनुवर्त;
    सचे( sqlite3DbIsNamed(db, i, zName) ) भङ्ग;
  }

  सचे( i>=db->nDb ){
    sqlite3_snprintf(परिमाण(zErr),zErr, "no such database: %s", zName);
    गच्छ detach_error;
  }
  सचे( i<2 ){
    sqlite3_snprintf(परिमाण(zErr),zErr, "cannot detach database %s", zName);
    गच्छ detach_error;
  }
  सचे( sqlite3BtreeTxnState(pDb->pBt)!=SQLITE_TXN_NONE
   || sqlite3BtreeIsInBackup(pDb->pBt)
  ){
    sqlite3_snprintf(परिमाण(zErr),zErr, "database %s is locked", zName);
    गच्छ detach_error;
  }

  detachTempTriggers(db, pDb->pSchema);
  sqlite3BtreeClose(pDb->pBt);
  pDb->pBt = 0;
  pDb->pSchema = 0;
  sqlite3CollapseDatabaseArray(db);
  पटिनिवत्त;

detach_error:
  sqlite3_result_error(context, zErr, -1);
}

/*
** This procedure generates VDBE code for a single invocation of either the
** sqlite_detach() or sqlite_attach() SQL user functions.
*/
स्थिर शून्य codeAttach(
  Parse *pParse,       /* The parser context */
  पूर्णाङ्क type,            /* Either SQLITE_ATTACH or SQLITE_DETACH */
  FuncDef ध्रुव *pFunc,/* FuncDef wrapper for detachFunc() or attachFunc() */
  व्यञ्जकम् *pAuthArg,      /* Expression to pass to authorization callback */
  व्यञ्जकम् *pFilename,     /* Name of database file */
  व्यञ्जकम् *pDbname,       /* Name of the database to use internally */
  व्यञ्जकम् *p_कुञ्चिका           /* Database key for encryption extension */
){
  पूर्णाङ्क rc;
  NameContext sName;
  Vdbe *v;
  sqlite3* db = pParse->db;
  पूर्णाङ्क regArgs;

  सचे( SQLITE_OK!=sqlite3ReadSchema(pParse) ) गच्छ attach_end;

  सचे( pParse->nErr ) गच्छ attach_end;
  memset(&sName, 0, परिमाण(NameContext));
  sName.pParse = pParse;

  सचे( 
      SQLITE_OK!=resolve_Attach_व्यञ्जकम्(&sName, pFilename) ||
      SQLITE_OK!=resolve_Attach_व्यञ्जकम्(&sName, pDbname) ||
      SQLITE_OK!=resolve_Attach_व्यञ्जकम्(&sName, p_कुञ्चिका)
  ){
    गच्छ attach_end;
  }

#ifndef SQLITE_OMIT_AUTHORIZATION
  सचे( ALWAYS(pAuthArg) ){
    वर्ण *zAuthArg;
    सचे( pAuthArg->op==TK_STRING ){
      प्रतिज्ञा( !व्यञ्जकम्_Has_Property(pAuthArg, EP_Int_मानम्) );
      zAuthArg = pAuthArg->u.zToken;
    }अञ्ञथा{
      zAuthArg = 0;
    }
    rc = sqlite3AuthCheck(pParse, type, zAuthArg, 0, 0);
    सचे(rc!=SQLITE_OK ){
      गच्छ attach_end;
    }
  }
#endif /* SQLITE_OMIT_AUTHORIZATION */


  v = sqlite3GetVdbe(pParse);
  regArgs = sqlite3GetTempRange(pParse, 4);
  sqlite3_व्यञ्जकम्_Code(pParse, pFilename, regArgs);
  sqlite3_व्यञ्जकम्_Code(pParse, pDbname, regArgs+1);
  sqlite3_व्यञ्जकम्_Code(pParse, p_कुञ्चिका, regArgs+2);

  प्रतिज्ञा( v || db->mallocFailed );
  सचे( v ){
    sqlite3VdbeAddFunctionCall(pParse, 0, regArgs+3-pFunc->nArg, regArgs+3,
                               pFunc->nArg, pFunc, 0);
    /* Code an OP_Expire. For an ATTACH statement, set P1 to true (expire this
    ** statement only). For DETACH, set it to false (expire all existing
    ** statements).
    */
    sqlite3VdbeAddOp1(v, OP_Expire, (type==SQLITE_ATTACH));
  }
  
attach_end:
  sqlite3_व्यञ्जकम्_Delete(db, pFilename);
  sqlite3_व्यञ्जकम्_Delete(db, pDbname);
  sqlite3_व्यञ्जकम्_Delete(db, p_कुञ्चिका);
}

/*
** Called by the parser to compile a DETACH statement.
**
**     DETACH pDbname
*/
शून्य sqlite3Detach(Parse *pParse, व्यञ्जकम् *pDbname){
  स्थिर ध्रुव FuncDef detach_func = {
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
शून्य sqlite3Attach(Parse *pParse, व्यञ्जकम् *p, व्यञ्जकम् *pDbname, व्यञ्जकम् *p_कुञ्चिका){
  स्थिर ध्रुव FuncDef attach_func = {
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
  codeAttach(pParse, SQLITE_ATTACH, &attach_func, p, p, pDbname, p_कुञ्चिका);
}
#endif /* SQLITE_OMIT_ATTACH */

/*
** Expression callback used by sqlite3FixAAAA() routines.
*/
स्थिर पूर्णाङ्क fix_व्यञ्जकम्_Cb(Walker *p, व्यञ्जकम् *p_व्यञ्जकम्){
  DbFixer *pFix = p->u.pFix;
  सचे( !pFix->bTemp ) व्यञ्जकम्_Set_Property(p_व्यञ्जकम्, EP_FromDDL);
  सचे( p_व्यञ्जकम्->op==TK_VARIABLE ){
    सचे( pFix->pParse->db->init.busy ){
      p_व्यञ्जकम्->op = TK_NULL;
    }अञ्ञथा{
      sqlite3_Error_सन्देशः(pFix->pParse, "%s cannot use variables", pFix->zType);
      पटिनिवत्त WRC_Abort;
    }
  }
  पटिनिवत्त WRC_Continue;
}

/*
** Select callback used by sqlite3FixAAAA() routines.
*/
स्थिर पूर्णाङ्क fixSelectCb(Walker *p, Select *pSelect){
  DbFixer *pFix = p->u.pFix;
  पूर्णाङ्क i;
  SrcItem *pItem;
  sqlite3 *db = pFix->pParse->db;
  पूर्णाङ्क iDb = sqlite3FindDbName(db, pFix->zDb);
  SrcList *pList = pSelect->pSrc;

  सचे( NEVER(pList==0) ) पटिनिवत्त WRC_Continue;
  अत्थाय(i=0, pItem=pList->a; i<pList->nSrc; i++, pItem++){
    सचे( pFix->bTemp==0 && pItem->fg.isSubquery==0 ){
      सचे( pItem->fg.fixedSchema==0 && pItem->u4.zDatabase!=0 ){
        सचे( iDb!=sqlite3FindDbName(db, pItem->u4.zDatabase) ){
          sqlite3_Error_सन्देशः(pFix->pParse,
              "%s %T cannot reference objects in database %s",
              pFix->zType, pFix->pName, pItem->u4.zDatabase);
          पटिनिवत्त WRC_Abort;
        }
        sqlite3DbFree(db, pItem->u4.zDatabase);
        pItem->fg.notCte = 1;
        pItem->fg.hadSchema = 1;
      }
      pItem->u4.pSchema = pFix->pSchema;
      pItem->fg.fromDDL = 1;
      pItem->fg.fixedSchema = 1;
    }
#सचे !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_TRIGGER)
    सचे( pList->a[i].fg.isUsing==0
     && sqlite3_Walk_व्यञ्जकम्(&pFix->w, pList->a[i].u3.pOn)
    ){
      पटिनिवत्त WRC_Abort;
    }
#endif
  }
  सचे( pSelect->pWith ){
    अत्थाय(i=0; i<pSelect->pWith->nCte; i++){
      सचे( sqlite3WalkSelect(p, pSelect->pWith->a[i].pSelect) ){
        पटिनिवत्त WRC_Abort;
      }
    }
  }
  पटिनिवत्त WRC_Continue;
}

/*
** Initialize a DbFixer structure.  This routine must be called prior
** to passing the structure to one of the sqliteFixAAAA() routines below.
*/
शून्य sqlite3FixInit(
  DbFixer *pFix,      /* The fixer to be initialized */
  Parse *pParse,      /* Error messages will be written here */
  पूर्णाङ्क iDb,            /* This is the database that must be used */
  ध्रुव वर्ण *zType,  /* "view", "trigger", or "index" */
  ध्रुव Token *pName  /* Name of the view, trigger, or index */
){
  sqlite3 *db = pParse->db;
  प्रतिज्ञा( db->nDb>iDb );
  pFix->pParse = pParse;
  pFix->zDb = db->aDb[iDb].zDbSName;
  pFix->pSchema = db->aDb[iDb].pSchema;
  pFix->zType = zType;
  pFix->pName = pName;
  pFix->bTemp = (iDb==1);
  pFix->w.pParse = pParse;
  pFix->w.x_व्यञ्जकम्_Callback = fix_व्यञ्जकम्_Cb;
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
पूर्णाङ्क sqlite3FixSrcList(
  DbFixer *pFix,       /* Context of the fixation */
  SrcList *pList       /* The Source list to check and modify */
){
  पूर्णाङ्क res = 0;
  सचे( pList ){
    Select s; 
    memset(&s, 0, परिमाण(s));
    s.pSrc = pList;
    res = sqlite3WalkSelect(&pFix->w, &s);
  }
  पटिनिवत्त res;
}
#सचे !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_TRIGGER)
पूर्णाङ्क sqlite3FixSelect(
  DbFixer *pFix,       /* Context of the fixation */
  Select *pSelect      /* The SELECT statement to be fixed to one database */
){
  पटिनिवत्त sqlite3WalkSelect(&pFix->w, pSelect);
}
पूर्णाङ्क sqlite3_Fix_व्यञ्जकम्(
  DbFixer *pFix,     /* Context of the fixation */
  व्यञ्जकम् *p_व्यञ्जकम्        /* The expression to be fixed to one database */
){
  पटिनिवत्त sqlite3_Walk_व्यञ्जकम्(&pFix->w, p_व्यञ्जकम्);
}
#endif

#ifndef SQLITE_OMIT_TRIGGER
पूर्णाङ्क sqlite3FixTriggerStep(
  DbFixer *pFix,     /* Context of the fixation */
  TriggerStep *pStep /* The trigger step be fixed to one database */
){
  याव( pStep ){
    सचे( sqlite3WalkSelect(&pFix->w, pStep->pSelect)
     || sqlite3_Walk_व्यञ्जकम्(&pFix->w, pStep->pWhere) 
     || sqlite3_Walk_व्यञ्जकम्_List(&pFix->w, pStep->p_व्यञ्जकम्_List)
     || sqlite3FixSrcList(pFix, pStep->pSrc)
    ){
      पटिनिवत्त 1;
    }
#ifndef SQLITE_OMIT_UPSERT
    {
      Upsert *pUp;
      अत्थाय(pUp=pStep->pUpsert; pUp; pUp=pUp->pNextUpsert){
        सचे( sqlite3_Walk_व्यञ्जकम्_List(&pFix->w, pUp->pUpsertTarget)
         || sqlite3_Walk_व्यञ्जकम्(&pFix->w, pUp->pUpsertTargetWhere)
         || sqlite3_Walk_व्यञ्जकम्_List(&pFix->w, pUp->pUpsertSet)
         || sqlite3_Walk_व्यञ्जकम्(&pFix->w, pUp->pUpsertWhere)
        ){
          पटिनिवत्त 1;
        }
      }
    }
#endif
    pStep = pStep->pNext;
  }

  पटिनिवत्त 0;
}
#endif
