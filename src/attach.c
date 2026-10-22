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
#അന്തര്ഭാവ "sqliteInt.h"

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
സ്ഥിതം പൂർണ്ണസംഖ്യ resolve_Attach_വ്യഞ്ജകം(NameContext *pName, വ്യഞ്ജകം *p_വ്യഞ്ജകം)
{
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;
  എങ്കിൽ( p_വ്യഞ്ജകം ){
    എങ്കിൽ( p_വ്യഞ്ജകം->op!=TK_ID ){
      rc = sqlite3_Resolve_വ്യഞ്ജകം_Names(pName, p_വ്യഞ്ജകം);
    }അല്ലെങ്കിൽ{
      p_വ്യഞ്ജകം->op = TK_STRING;
    }
  }
  തിരികെ rc;
}

/*
** Return true if zName points to a name that may be used to refer to
** database iDb attached to handle db.
*/
പൂർണ്ണസംഖ്യ sqlite3DbIsNamed(sqlite3 *db, പൂർണ്ണസംഖ്യ iDb, സ്ഥിരം അക്ഷരം *zName){
  തിരികെ (
      sqlite3StrICmp(db->aDb[iDb].zDbSName, zName)==0
   || (iDb==0 && sqlite3StrICmp("main", zName)==0)
  );
}

/*
** If any TEMP triggers reference schema pSchema, move those triggers to
** reference the TEMP schema itself.
*/
സ്ഥിതം ശൂന്യം detachTempTriggers(sqlite3 *db, Schema *pSchema){
  HashElem *pEntry;
  പ്രതിജ്ഞാ( db->aDb[1].pSchema );
  pEntry = sqliteHashFirst(&db->aDb[1].pSchema->trigHash);
  ആകുമ്പോൾ( pEntry ){
    Trigger *pTrig = (Trigger*)sqliteHashData(pEntry);
    എങ്കിൽ( pTrig->pTabSchema==pSchema ){
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
സ്ഥിതം ശൂന്യം attachFunc(
  sqlite3_context *context,
  പൂർണ്ണസംഖ്യ NotUsed,
  sqlite3_മൂല്യം **argv
){
  പൂർണ്ണസംഖ്യ i;
  പൂർണ്ണസംഖ്യ rc = 0;
  sqlite3 *db = sqlite3_context_db_handle(context);
  സ്ഥിരം അക്ഷരം *zName;
  സ്ഥിരം അക്ഷരം *zFile;
  അക്ഷരം *zPath = 0;
  അക്ഷരം *zErr = 0;
  unsigned പൂർണ്ണസംഖ്യ flags;
  Db *aNew;                 /* New array of Db pointers */
  Db *pNew = 0;             /* Db object for the newly attached database */
  അക്ഷരം *zErrDyn = 0;
  sqlite3_vfs *pVfs;

  UNUSED_PARAMETER(NotUsed);
  zFile = (സ്ഥിരം അക്ഷരം *)sqlite3_മൂല്യം_text(argv[0]);
  zName = (സ്ഥിരം അക്ഷരം *)sqlite3_മൂല്യം_text(argv[1]);
  എങ്കിൽ( zFile==0 ) zFile = "";
  എങ്കിൽ( zName==0 ) zName = "";

#ifndef SQLITE_OMIT_DESERIALIZE
# define REOPEN_AS_MEMDB(db)  (db->init.reopenMemdb)
#അല്ലെങ്കിൽ
# define REOPEN_AS_MEMDB(db)  (0)
#endif

  എങ്കിൽ( REOPEN_AS_MEMDB(db) ){
    /* This is not a real ATTACH.  Instead, this routine is being called
    ** from sqlite3_deserialize() to close database db->init.iDb and
    ** reopen it as a MemDB */
    Btree *pNewBt = 0;

    pNew = &db->aDb[db->init.iDb];
    പ്രതിജ്ഞാ( pNew->pBt!=0 );
    എങ്കിൽ( sqlite3BtreeTxnState(pNew->pBt)!=SQLITE_TXN_NONE
     || sqlite3BtreeIsInBackup(pNew->pBt)
    ){
      rc = SQLITE_BUSY;
      പോകുക attach_error;
    }

    pVfs = sqlite3_vfs_find("memdb");
    എങ്കിൽ( pVfs==0 ) തിരികെ;
    rc = sqlite3BtreeOpen(pVfs, "x\0", db, &pNewBt, 0, SQLITE_OPEN_MAIN_DB);
    എങ്കിൽ( rc==SQLITE_OK ){
      Schema *pNewSchema = sqlite3SchemaGet(db, pNewBt);
      എങ്കിൽ( pNewSchema ){
        /* Both the Btree and the new Schema were allocated successfully.
        ** Close the old db and update the aDb[] slot with the new memdb
        ** values.  */
        detachTempTriggers(db, pNew->pSchema);
        sqlite3BtreeClose(pNew->pBt);
        pNew->pBt = pNewBt;
        pNew->pSchema = pNewSchema;
        എങ്കിൽ( db->init.iDb==0 ){
          /* Clear all eponymous virtual table instances, as they are holding
          ** pointers to the schema object just freed by sqlite3BtreeClose() */
          sqlite3VtabEponymousTableClearAll(db);
        }
        sqlite3ExpirePreparedStatements(db, 1);
      }അല്ലെങ്കിൽ{
        sqlite3BtreeClose(pNewBt);
        rc = SQLITE_NOMEM;
      }
    }
    എങ്കിൽ( rc ) പോകുക attach_error;
  }അല്ലെങ്കിൽ{
    /* This is a real ATTACH
    **
    ** Check for the following errors:
    **
    **     * Too many attached databases,
    **     * Transaction currently open
    **     * Specified database name already being used.
    */
    എങ്കിൽ( db->nDb>=db->aLimit[SQLITE_LIMIT_ATTACHED]+2 ){
      zErrDyn = sqlite3MPrintf(db, "too many attached databases - max %d", 
        db->aLimit[SQLITE_LIMIT_ATTACHED]
      );
      പോകുക attach_error;
    }
    വേണ്ടി(i=0; i<db->nDb; i++){
      പ്രതിജ്ഞാ( zName );
      എങ്കിൽ( sqlite3DbIsNamed(db, i, zName) ){
        zErrDyn = sqlite3MPrintf(db, "database %s is already in use", zName);
        പോകുക attach_error;
      }
    }
  
    /* Allocate the new entry in the db->aDb[] array and initialize the schema
    ** hash tables.
    */
    എങ്കിൽ( db->aDb==db->aDbStatic ){
      aNew = sqlite3DbMallocRawNN(db, വലുപ്പം(db->aDb[0])*3 );
      എങ്കിൽ( aNew==0 ) തിരികെ;
      memcpy(aNew, db->aDb, വലുപ്പം(db->aDb[0])*2);
    }അല്ലെങ്കിൽ{
      aNew = sqlite3DbRealloc(db, db->aDb, വലുപ്പം(db->aDb[0])*(1+(i64)db->nDb));
      എങ്കിൽ( aNew==0 ) തിരികെ;
    }
    db->aDb = aNew;
    pNew = &db->aDb[db->nDb];
    memset(pNew, 0, വലുപ്പം(*pNew));
  
    /* Open the database file. If the btree is successfully opened, use
    ** it to obtain the database schema. At this point the schema may
    ** or may not be initialized.
    */
    flags = db->openFlags;
    rc = sqlite3ParseUri(db->pVfs->zName, zFile, &flags, &pVfs, &zPath, &zErr);
    എങ്കിൽ( rc!=SQLITE_OK ){
      എങ്കിൽ( rc==SQLITE_NOMEM ) sqlite3OomFault(db);
      sqlite3_result_error(context, zErr, -1);
      sqlite3_free(zErr);
      തിരികെ;
    }
    എങ്കിൽ( (db->flags & SQLITE_AttachWrite)==0 ){
      flags &= ~(SQLITE_OPEN_CREATE|SQLITE_OPEN_READWRITE);
      flags |= SQLITE_OPEN_READONLY;
    }അല്ലെങ്കിൽ എങ്കിൽ( (db->flags & SQLITE_AttachCreate)==0 ){
      flags &= ~SQLITE_OPEN_CREATE;
    }
    പ്രതിജ്ഞാ( pVfs );
    flags |= SQLITE_OPEN_MAIN_DB;
    rc = sqlite3BtreeOpen(pVfs, zPath, db, &pNew->pBt, 0, flags);
    db->nDb++;
    pNew->zDbSName = sqlite3DbStrDup(db, zName);
  }
  db->noSharedCache = 0;
  എങ്കിൽ( rc==SQLITE_CONSTRAINT ){
    rc = SQLITE_ERROR;
    zErrDyn = sqlite3MPrintf(db, "database is already attached");
  }അല്ലെങ്കിൽ എങ്കിൽ( rc==SQLITE_OK ){
    Pager *pPager;
    pNew->pSchema = sqlite3SchemaGet(db, pNew->pBt);
    എങ്കിൽ( !pNew->pSchema ){
      rc = SQLITE_NOMEM_BKPT;
    }അല്ലെങ്കിൽ എങ്കിൽ( pNew->pSchema->file_format && pNew->pSchema->enc!=ENC(db) ){
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
  എങ്കിൽ( rc==SQLITE_OK && pNew->zDbSName==0 ){
    rc = SQLITE_NOMEM_BKPT;
  }
  sqlite3_free_filename( zPath );

  /* If the file was opened successfully, read the schema for the new database.
  ** If this fails, or if opening the file failed, then close the file and 
  ** remove the entry from the db->aDb[] array. i.e. put everything back the
  ** way we found it.
  */
  എങ്കിൽ( rc==SQLITE_OK ){
    sqlite3BtreeEnterAll(db);
    db->init.iDb = 0;
    db->mDbFlags &= ~(DBFLAG_SchemaKnownOk);
#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
    എങ്കിൽ( db->setlkFlags & SQLITE_SETLK_BLOCK_ON_CONNECT ){
      പൂർണ്ണസംഖ്യ val = 1;
      sqlite3_file *fd = sqlite3PagerFile(sqlite3BtreePager(pNew->pBt));
      sqlite3OsFileControlHint(fd, SQLITE_FCNTL_BLOCK_ON_CONNECT, &val);
    }
#endif
    എങ്കിൽ( !REOPEN_AS_MEMDB(db) ){
      rc = sqlite3Init(db, &zErrDyn);
    }
    sqlite3BtreeLeaveAll(db);
    പ്രതിജ്ഞാ( zErrDyn==0 || rc!=SQLITE_OK );
  }
  എങ്കിൽ( rc ){
    എങ്കിൽ( ALWAYS(!REOPEN_AS_MEMDB(db)) ){
      പൂർണ്ണസംഖ്യ iDb = db->nDb - 1;
      പ്രതിജ്ഞാ( iDb>=2 );
      എങ്കിൽ( db->aDb[iDb].pBt ){
        sqlite3BtreeClose(db->aDb[iDb].pBt);
        db->aDb[iDb].pBt = 0;
        db->aDb[iDb].pSchema = 0;
      }
      sqlite3ResetAllSchemasOfConnection(db);
      db->nDb = iDb;
      എങ്കിൽ( rc==SQLITE_NOMEM || rc==SQLITE_IOERR_NOMEM ){
        sqlite3OomFault(db);
        sqlite3DbFree(db, zErrDyn);
        zErrDyn = sqlite3MPrintf(db, "out of memory");
      }അല്ലെങ്കിൽ എങ്കിൽ( zErrDyn==0 ){
        zErrDyn = sqlite3MPrintf(db, "unable to open database: %s", zFile);
      }
    }
    പോകുക attach_error;
  }
  
  തിരികെ;

attach_error:
  /* Return an error if we get here */
  എങ്കിൽ( zErrDyn ){
    sqlite3_result_error(context, zErrDyn, -1);
    sqlite3DbFree(db, zErrDyn);
  }
  എങ്കിൽ( rc ) sqlite3_result_error_code(context, rc);
}

/*
** An SQL user-function registered to do the work of an DETACH statement. The
** three arguments to the function come directly from a detach statement:
**
**     DETACH DATABASE x
**
**     SELECT sqlite_detach(x)
*/
സ്ഥിതം ശൂന്യം detachFunc(
  sqlite3_context *context,
  പൂർണ്ണസംഖ്യ NotUsed,
  sqlite3_മൂല്യം **argv
){
  സ്ഥിരം അക്ഷരം *zName = (സ്ഥിരം അക്ഷരം *)sqlite3_മൂല്യം_text(argv[0]);
  sqlite3 *db = sqlite3_context_db_handle(context);
  പൂർണ്ണസംഖ്യ i;
  Db *pDb = 0;
  അക്ഷരം zErr[128];

  UNUSED_PARAMETER(NotUsed);

  എങ്കിൽ( zName==0 ) zName = "";
  വേണ്ടി(i=0; i<db->nDb; i++){
    pDb = &db->aDb[i];
    എങ്കിൽ( pDb->pBt==0 ) തുടരുക;
    എങ്കിൽ( sqlite3DbIsNamed(db, i, zName) ) നിർത്തുക;
  }

  എങ്കിൽ( i>=db->nDb ){
    sqlite3_snprintf(വലുപ്പം(zErr),zErr, "no such database: %s", zName);
    പോകുക detach_error;
  }
  എങ്കിൽ( i<2 ){
    sqlite3_snprintf(വലുപ്പം(zErr),zErr, "cannot detach database %s", zName);
    പോകുക detach_error;
  }
  എങ്കിൽ( sqlite3BtreeTxnState(pDb->pBt)!=SQLITE_TXN_NONE
   || sqlite3BtreeIsInBackup(pDb->pBt)
  ){
    sqlite3_snprintf(വലുപ്പം(zErr),zErr, "database %s is locked", zName);
    പോകുക detach_error;
  }

  detachTempTriggers(db, pDb->pSchema);
  sqlite3BtreeClose(pDb->pBt);
  pDb->pBt = 0;
  pDb->pSchema = 0;
  sqlite3CollapseDatabaseArray(db);
  തിരികെ;

detach_error:
  sqlite3_result_error(context, zErr, -1);
}

/*
** This procedure generates VDBE code for a single invocation of either the
** sqlite_detach() or sqlite_attach() SQL user functions.
*/
സ്ഥിതം ശൂന്യം codeAttach(
  Parse *pParse,       /* The parser context */
  പൂർണ്ണസംഖ്യ type,            /* Either SQLITE_ATTACH or SQLITE_DETACH */
  FuncDef സ്ഥിരം *pFunc,/* FuncDef wrapper for detachFunc() or attachFunc() */
  വ്യഞ്ജകം *pAuthArg,      /* Expression to pass to authorization callback */
  വ്യഞ്ജകം *pFilename,     /* Name of database file */
  വ്യഞ്ജകം *pDbname,       /* Name of the database to use internally */
  വ്യഞ്ജകം *p_താക്കോൽ           /* Database key for encryption extension */
){
  പൂർണ്ണസംഖ്യ rc;
  NameContext sName;
  Vdbe *v;
  sqlite3* db = pParse->db;
  പൂർണ്ണസംഖ്യ regArgs;

  എങ്കിൽ( SQLITE_OK!=sqlite3ReadSchema(pParse) ) പോകുക attach_end;

  എങ്കിൽ( pParse->nErr ) പോകുക attach_end;
  memset(&sName, 0, വലുപ്പം(NameContext));
  sName.pParse = pParse;

  എങ്കിൽ( 
      SQLITE_OK!=resolve_Attach_വ്യഞ്ജകം(&sName, pFilename) ||
      SQLITE_OK!=resolve_Attach_വ്യഞ്ജകം(&sName, pDbname) ||
      SQLITE_OK!=resolve_Attach_വ്യഞ്ജകം(&sName, p_താക്കോൽ)
  ){
    പോകുക attach_end;
  }

#ifndef SQLITE_OMIT_AUTHORIZATION
  എങ്കിൽ( ALWAYS(pAuthArg) ){
    അക്ഷരം *zAuthArg;
    എങ്കിൽ( pAuthArg->op==TK_STRING ){
      പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(pAuthArg, EP_Int_മൂല്യം) );
      zAuthArg = pAuthArg->u.zToken;
    }അല്ലെങ്കിൽ{
      zAuthArg = 0;
    }
    rc = sqlite3AuthCheck(pParse, type, zAuthArg, 0, 0);
    എങ്കിൽ(rc!=SQLITE_OK ){
      പോകുക attach_end;
    }
  }
#endif /* SQLITE_OMIT_AUTHORIZATION */


  v = sqlite3GetVdbe(pParse);
  regArgs = sqlite3GetTempRange(pParse, 4);
  sqlite3_വ്യഞ്ജകം_Code(pParse, pFilename, regArgs);
  sqlite3_വ്യഞ്ജകം_Code(pParse, pDbname, regArgs+1);
  sqlite3_വ്യഞ്ജകം_Code(pParse, p_താക്കോൽ, regArgs+2);

  പ്രതിജ്ഞാ( v || db->mallocFailed );
  എങ്കിൽ( v ){
    sqlite3VdbeAddFunctionCall(pParse, 0, regArgs+3-pFunc->nArg, regArgs+3,
                               pFunc->nArg, pFunc, 0);
    /* Code an OP_Expire. For an ATTACH statement, set P1 to true (expire this
    ** statement only). For DETACH, set it to false (expire all existing
    ** statements).
    */
    sqlite3VdbeAddOp1(v, OP_Expire, (type==SQLITE_ATTACH));
  }
  
attach_end:
  sqlite3_വ്യഞ്ജകം_Delete(db, pFilename);
  sqlite3_വ്യഞ്ജകം_Delete(db, pDbname);
  sqlite3_വ്യഞ്ജകം_Delete(db, p_താക്കോൽ);
}

/*
** Called by the parser to compile a DETACH statement.
**
**     DETACH pDbname
*/
ശൂന്യം sqlite3Detach(Parse *pParse, വ്യഞ്ജകം *pDbname){
  സ്ഥിതം സ്ഥിരം FuncDef detach_func = {
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
ശൂന്യം sqlite3Attach(Parse *pParse, വ്യഞ്ജകം *p, വ്യഞ്ജകം *pDbname, വ്യഞ്ജകം *p_താക്കോൽ){
  സ്ഥിതം സ്ഥിരം FuncDef attach_func = {
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
  codeAttach(pParse, SQLITE_ATTACH, &attach_func, p, p, pDbname, p_താക്കോൽ);
}
#endif /* SQLITE_OMIT_ATTACH */

/*
** Expression callback used by sqlite3FixAAAA() routines.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ fix_വ്യഞ്ജകം_Cb(Walker *p, വ്യഞ്ജകം *p_വ്യഞ്ജകം){
  DbFixer *pFix = p->u.pFix;
  എങ്കിൽ( !pFix->bTemp ) വ്യഞ്ജകം_Set_Property(p_വ്യഞ്ജകം, EP_FromDDL);
  എങ്കിൽ( p_വ്യഞ്ജകം->op==TK_VARIABLE ){
    എങ്കിൽ( pFix->pParse->db->init.busy ){
      p_വ്യഞ്ജകം->op = TK_NULL;
    }അല്ലെങ്കിൽ{
      sqlite3_Error_സന്ദേശം(pFix->pParse, "%s cannot use variables", pFix->zType);
      തിരികെ WRC_Abort;
    }
  }
  തിരികെ WRC_Continue;
}

/*
** Select callback used by sqlite3FixAAAA() routines.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ fixSelectCb(Walker *p, Select *pSelect){
  DbFixer *pFix = p->u.pFix;
  പൂർണ്ണസംഖ്യ i;
  SrcItem *pItem;
  sqlite3 *db = pFix->pParse->db;
  പൂർണ്ണസംഖ്യ iDb = sqlite3FindDbName(db, pFix->zDb);
  SrcList *pList = pSelect->pSrc;

  എങ്കിൽ( NEVER(pList==0) ) തിരികെ WRC_Continue;
  വേണ്ടി(i=0, pItem=pList->a; i<pList->nSrc; i++, pItem++){
    എങ്കിൽ( pFix->bTemp==0 && pItem->fg.isSubquery==0 ){
      എങ്കിൽ( pItem->fg.fixedSchema==0 && pItem->u4.zDatabase!=0 ){
        എങ്കിൽ( iDb!=sqlite3FindDbName(db, pItem->u4.zDatabase) ){
          sqlite3_Error_സന്ദേശം(pFix->pParse,
              "%s %T cannot reference objects in database %s",
              pFix->zType, pFix->pName, pItem->u4.zDatabase);
          തിരികെ WRC_Abort;
        }
        sqlite3DbFree(db, pItem->u4.zDatabase);
        pItem->fg.notCte = 1;
        pItem->fg.hadSchema = 1;
      }
      pItem->u4.pSchema = pFix->pSchema;
      pItem->fg.fromDDL = 1;
      pItem->fg.fixedSchema = 1;
    }
#എങ്കിൽ !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_TRIGGER)
    എങ്കിൽ( pList->a[i].fg.isUsing==0
     && sqlite3_Walk_വ്യഞ്ജകം(&pFix->w, pList->a[i].u3.pOn)
    ){
      തിരികെ WRC_Abort;
    }
#endif
  }
  എങ്കിൽ( pSelect->pWith ){
    വേണ്ടി(i=0; i<pSelect->pWith->nCte; i++){
      എങ്കിൽ( sqlite3WalkSelect(p, pSelect->pWith->a[i].pSelect) ){
        തിരികെ WRC_Abort;
      }
    }
  }
  തിരികെ WRC_Continue;
}

/*
** Initialize a DbFixer structure.  This routine must be called prior
** to passing the structure to one of the sqliteFixAAAA() routines below.
*/
ശൂന്യം sqlite3FixInit(
  DbFixer *pFix,      /* The fixer to be initialized */
  Parse *pParse,      /* Error messages will be written here */
  പൂർണ്ണസംഖ്യ iDb,            /* This is the database that must be used */
  സ്ഥിരം അക്ഷരം *zType,  /* "view", "trigger", or "index" */
  സ്ഥിരം Token *pName  /* Name of the view, trigger, or index */
){
  sqlite3 *db = pParse->db;
  പ്രതിജ്ഞാ( db->nDb>iDb );
  pFix->pParse = pParse;
  pFix->zDb = db->aDb[iDb].zDbSName;
  pFix->pSchema = db->aDb[iDb].pSchema;
  pFix->zType = zType;
  pFix->pName = pName;
  pFix->bTemp = (iDb==1);
  pFix->w.pParse = pParse;
  pFix->w.x_വ്യഞ്ജകം_Callback = fix_വ്യഞ്ജകം_Cb;
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
പൂർണ്ണസംഖ്യ sqlite3FixSrcList(
  DbFixer *pFix,       /* Context of the fixation */
  SrcList *pList       /* The Source list to check and modify */
){
  പൂർണ്ണസംഖ്യ res = 0;
  എങ്കിൽ( pList ){
    Select s; 
    memset(&s, 0, വലുപ്പം(s));
    s.pSrc = pList;
    res = sqlite3WalkSelect(&pFix->w, &s);
  }
  തിരികെ res;
}
#എങ്കിൽ !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_TRIGGER)
പൂർണ്ണസംഖ്യ sqlite3FixSelect(
  DbFixer *pFix,       /* Context of the fixation */
  Select *pSelect      /* The SELECT statement to be fixed to one database */
){
  തിരികെ sqlite3WalkSelect(&pFix->w, pSelect);
}
പൂർണ്ണസംഖ്യ sqlite3_Fix_വ്യഞ്ജകം(
  DbFixer *pFix,     /* Context of the fixation */
  വ്യഞ്ജകം *p_വ്യഞ്ജകം        /* The expression to be fixed to one database */
){
  തിരികെ sqlite3_Walk_വ്യഞ്ജകം(&pFix->w, p_വ്യഞ്ജകം);
}
#endif

#ifndef SQLITE_OMIT_TRIGGER
പൂർണ്ണസംഖ്യ sqlite3FixTriggerStep(
  DbFixer *pFix,     /* Context of the fixation */
  TriggerStep *pStep /* The trigger step be fixed to one database */
){
  ആകുമ്പോൾ( pStep ){
    എങ്കിൽ( sqlite3WalkSelect(&pFix->w, pStep->pSelect)
     || sqlite3_Walk_വ്യഞ്ജകം(&pFix->w, pStep->pWhere) 
     || sqlite3_Walk_വ്യഞ്ജകം_List(&pFix->w, pStep->p_വ്യഞ്ജകം_List)
     || sqlite3FixSrcList(pFix, pStep->pSrc)
    ){
      തിരികെ 1;
    }
#ifndef SQLITE_OMIT_UPSERT
    {
      Upsert *pUp;
      വേണ്ടി(pUp=pStep->pUpsert; pUp; pUp=pUp->pNextUpsert){
        എങ്കിൽ( sqlite3_Walk_വ്യഞ്ജകം_List(&pFix->w, pUp->pUpsertTarget)
         || sqlite3_Walk_വ്യഞ്ജകം(&pFix->w, pUp->pUpsertTargetWhere)
         || sqlite3_Walk_വ്യഞ്ജകം_List(&pFix->w, pUp->pUpsertSet)
         || sqlite3_Walk_വ്യഞ്ജകം(&pFix->w, pUp->pUpsertWhere)
        ){
          തിരികെ 1;
        }
      }
    }
#endif
    pStep = pStep->pNext;
  }

  തിരികെ 0;
}
#endif
