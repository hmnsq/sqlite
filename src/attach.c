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
#ଅନ୍ତର୍ଭାଵ "sqliteInt.h"

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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା resolve_Attach_ପଦସମଷ୍ଟି(NameContext *pName, ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି)
{
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
  ଯଦି( p_ପଦସମଷ୍ଟି ){
    ଯଦି( p_ପଦସମଷ୍ଟି->op!=TK_ID ){
      rc = sqlite3_Resolve_ପଦସମଷ୍ଟି_Names(pName, p_ପଦସମଷ୍ଟି);
    }ଅନ୍ୟଥା{
      p_ପଦସମଷ୍ଟି->op = TK_STRING;
    }
  }
  ଫେରନ୍ତୁ rc;
}

/*
** Return true if zName points to a name that may be used to refer to
** database iDb attached to handle db.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3DbIsNamed(sqlite3 *db, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDb, ଅଚଳ ଅକ୍ଷର *zName){
  ଫେରନ୍ତୁ (
      sqlite3StrICmp(db->aDb[iDb].zDbSName, zName)==0
   || (iDb==0 && sqlite3StrICmp("main", zName)==0)
  );
}

/*
** If any TEMP triggers reference schema pSchema, move those triggers to
** reference the TEMP schema itself.
*/
ସ୍ଥିର ଶୂନ୍ୟ detachTempTriggers(sqlite3 *db, Schema *pSchema){
  HashElem *pEntry;
  ପ୍ରତିଜ୍ଞା( db->aDb[1].pSchema );
  pEntry = sqliteHashFirst(&db->aDb[1].pSchema->trigHash);
  ଯେତେବେଳେ( pEntry ){
    Trigger *pTrig = (Trigger*)sqliteHashData(pEntry);
    ଯଦି( pTrig->pTabSchema==pSchema ){
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
ସ୍ଥିର ଶୂନ୍ୟ attachFunc(
  sqlite3_context *context,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା NotUsed,
  sqlite3_ମୂଲ୍ୟ **argv
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = 0;
  sqlite3 *db = sqlite3_context_db_handle(context);
  ଅଚଳ ଅକ୍ଷର *zName;
  ଅଚଳ ଅକ୍ଷର *zFile;
  ଅକ୍ଷର *zPath = 0;
  ଅକ୍ଷର *zErr = 0;
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags;
  Db *aNew;                 /* New array of Db pointers */
  Db *pNew = 0;             /* Db object for the newly attached database */
  ଅକ୍ଷର *zErrDyn = 0;
  sqlite3_vfs *pVfs;

  UNUSED_PARAMETER(NotUsed);
  zFile = (ଅଚଳ ଅକ୍ଷର *)sqlite3_ମୂଲ୍ୟ_text(argv[0]);
  zName = (ଅଚଳ ଅକ୍ଷର *)sqlite3_ମୂଲ୍ୟ_text(argv[1]);
  ଯଦି( zFile==0 ) zFile = "";
  ଯଦି( zName==0 ) zName = "";

#ifndef SQLITE_OMIT_DESERIALIZE
# define REOPEN_AS_MEMDB(db)  (db->init.reopenMemdb)
#ଅନ୍ୟଥା
# define REOPEN_AS_MEMDB(db)  (0)
#endif

  ଯଦି( REOPEN_AS_MEMDB(db) ){
    /* This is not a real ATTACH.  Instead, this routine is being called
    ** from sqlite3_deserialize() to close database db->init.iDb and
    ** reopen it as a MemDB */
    Btree *pNewBt = 0;

    pNew = &db->aDb[db->init.iDb];
    ପ୍ରତିଜ୍ଞା( pNew->pBt!=0 );
    ଯଦି( sqlite3BtreeTxnState(pNew->pBt)!=SQLITE_TXN_NONE
     || sqlite3BtreeIsInBackup(pNew->pBt)
    ){
      rc = SQLITE_BUSY;
      ଯାଅ attach_error;
    }

    pVfs = sqlite3_vfs_find("memdb");
    ଯଦି( pVfs==0 ) ଫେରନ୍ତୁ;
    rc = sqlite3BtreeOpen(pVfs, "x\0", db, &pNewBt, 0, SQLITE_OPEN_MAIN_DB);
    ଯଦି( rc==SQLITE_OK ){
      Schema *pNewSchema = sqlite3SchemaGet(db, pNewBt);
      ଯଦି( pNewSchema ){
        /* Both the Btree and the new Schema were allocated successfully.
        ** Close the old db and update the aDb[] slot with the new memdb
        ** values.  */
        detachTempTriggers(db, pNew->pSchema);
        sqlite3BtreeClose(pNew->pBt);
        pNew->pBt = pNewBt;
        pNew->pSchema = pNewSchema;
        ଯଦି( db->init.iDb==0 ){
          /* Clear all eponymous virtual table instances, as they are holding
          ** pointers to the schema object just freed by sqlite3BtreeClose() */
          sqlite3VtabEponymousTableClearAll(db);
        }
        sqlite3ExpirePreparedStatements(db, 1);
      }ଅନ୍ୟଥା{
        sqlite3BtreeClose(pNewBt);
        rc = SQLITE_NOMEM;
      }
    }
    ଯଦି( rc ) ଯାଅ attach_error;
  }ଅନ୍ୟଥା{
    /* This is a real ATTACH
    **
    ** Check for the following errors:
    **
    **     * Too many attached databases,
    **     * Transaction currently open
    **     * Specified database name already being used.
    */
    ଯଦି( db->nDb>=db->aLimit[SQLITE_LIMIT_ATTACHED]+2 ){
      zErrDyn = sqlite3MPrintf(db, "too many attached databases - max %d", 
        db->aLimit[SQLITE_LIMIT_ATTACHED]
      );
      ଯାଅ attach_error;
    }
    ପାଇଁ(i=0; i<db->nDb; i++){
      ପ୍ରତିଜ୍ଞା( zName );
      ଯଦି( sqlite3DbIsNamed(db, i, zName) ){
        zErrDyn = sqlite3MPrintf(db, "database %s is already in use", zName);
        ଯାଅ attach_error;
      }
    }
  
    /* Allocate the new entry in the db->aDb[] array and initialize the schema
    ** hash tables.
    */
    ଯଦି( db->aDb==db->aDbStatic ){
      aNew = sqlite3DbMallocRawNN(db, ଆକାର(db->aDb[0])*3 );
      ଯଦି( aNew==0 ) ଫେରନ୍ତୁ;
      memcpy(aNew, db->aDb, ଆକାର(db->aDb[0])*2);
    }ଅନ୍ୟଥା{
      aNew = sqlite3DbRealloc(db, db->aDb, ଆକାର(db->aDb[0])*(1+(i64)db->nDb));
      ଯଦି( aNew==0 ) ଫେରନ୍ତୁ;
    }
    db->aDb = aNew;
    pNew = &db->aDb[db->nDb];
    memset(pNew, 0, ଆକାର(*pNew));
  
    /* Open the database file. If the btree is successfully opened, use
    ** it to obtain the database schema. At this point the schema may
    ** or may not be initialized.
    */
    flags = db->openFlags;
    rc = sqlite3ParseUri(db->pVfs->zName, zFile, &flags, &pVfs, &zPath, &zErr);
    ଯଦି( rc!=SQLITE_OK ){
      ଯଦି( rc==SQLITE_NOMEM ) sqlite3OomFault(db);
      sqlite3_result_error(context, zErr, -1);
      sqlite3_free(zErr);
      ଫେରନ୍ତୁ;
    }
    ଯଦି( (db->flags & SQLITE_AttachWrite)==0 ){
      flags &= ~(SQLITE_OPEN_CREATE|SQLITE_OPEN_READWRITE);
      flags |= SQLITE_OPEN_READONLY;
    }ଅନ୍ୟଥା ଯଦି( (db->flags & SQLITE_AttachCreate)==0 ){
      flags &= ~SQLITE_OPEN_CREATE;
    }
    ପ୍ରତିଜ୍ଞା( pVfs );
    flags |= SQLITE_OPEN_MAIN_DB;
    rc = sqlite3BtreeOpen(pVfs, zPath, db, &pNew->pBt, 0, flags);
    db->nDb++;
    pNew->zDbSName = sqlite3DbStrDup(db, zName);
  }
  db->noSharedCache = 0;
  ଯଦି( rc==SQLITE_CONSTRAINT ){
    rc = SQLITE_ERROR;
    zErrDyn = sqlite3MPrintf(db, "database is already attached");
  }ଅନ୍ୟଥା ଯଦି( rc==SQLITE_OK ){
    Pager *pPager;
    pNew->pSchema = sqlite3SchemaGet(db, pNew->pBt);
    ଯଦି( !pNew->pSchema ){
      rc = SQLITE_NOMEM_BKPT;
    }ଅନ୍ୟଥା ଯଦି( pNew->pSchema->file_format && pNew->pSchema->enc!=ENC(db) ){
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
  ଯଦି( rc==SQLITE_OK && pNew->zDbSName==0 ){
    rc = SQLITE_NOMEM_BKPT;
  }
  sqlite3_free_filename( zPath );

  /* If the file was opened successfully, read the schema for the new database.
  ** If this fails, or if opening the file failed, then close the file and 
  ** remove the entry from the db->aDb[] array. i.e. put everything back the
  ** way we found it.
  */
  ଯଦି( rc==SQLITE_OK ){
    sqlite3BtreeEnterAll(db);
    db->init.iDb = 0;
    db->mDbFlags &= ~(DBFLAG_SchemaKnownOk);
#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
    ଯଦି( db->setlkFlags & SQLITE_SETLK_BLOCK_ON_CONNECT ){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା val = 1;
      sqlite3_file *fd = sqlite3PagerFile(sqlite3BtreePager(pNew->pBt));
      sqlite3OsFileControlHint(fd, SQLITE_FCNTL_BLOCK_ON_CONNECT, &val);
    }
#endif
    ଯଦି( !REOPEN_AS_MEMDB(db) ){
      rc = sqlite3Init(db, &zErrDyn);
    }
    sqlite3BtreeLeaveAll(db);
    ପ୍ରତିଜ୍ଞା( zErrDyn==0 || rc!=SQLITE_OK );
  }
  ଯଦି( rc ){
    ଯଦି( ALWAYS(!REOPEN_AS_MEMDB(db)) ){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDb = db->nDb - 1;
      ପ୍ରତିଜ୍ଞା( iDb>=2 );
      ଯଦି( db->aDb[iDb].pBt ){
        sqlite3BtreeClose(db->aDb[iDb].pBt);
        db->aDb[iDb].pBt = 0;
        db->aDb[iDb].pSchema = 0;
      }
      sqlite3ResetAllSchemasOfConnection(db);
      db->nDb = iDb;
      ଯଦି( rc==SQLITE_NOMEM || rc==SQLITE_IOERR_NOMEM ){
        sqlite3OomFault(db);
        sqlite3DbFree(db, zErrDyn);
        zErrDyn = sqlite3MPrintf(db, "out of memory");
      }ଅନ୍ୟଥା ଯଦି( zErrDyn==0 ){
        zErrDyn = sqlite3MPrintf(db, "unable to open database: %s", zFile);
      }
    }
    ଯାଅ attach_error;
  }
  
  ଫେରନ୍ତୁ;

attach_error:
  /* Return an error if we get here */
  ଯଦି( zErrDyn ){
    sqlite3_result_error(context, zErrDyn, -1);
    sqlite3DbFree(db, zErrDyn);
  }
  ଯଦି( rc ) sqlite3_result_error_code(context, rc);
}

/*
** An SQL user-function registered to do the work of an DETACH statement. The
** three arguments to the function come directly from a detach statement:
**
**     DETACH DATABASE x
**
**     SELECT sqlite_detach(x)
*/
ସ୍ଥିର ଶୂନ୍ୟ detachFunc(
  sqlite3_context *context,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା NotUsed,
  sqlite3_ମୂଲ୍ୟ **argv
){
  ଅଚଳ ଅକ୍ଷର *zName = (ଅଚଳ ଅକ୍ଷର *)sqlite3_ମୂଲ୍ୟ_text(argv[0]);
  sqlite3 *db = sqlite3_context_db_handle(context);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  Db *pDb = 0;
  ଅକ୍ଷର zErr[128];

  UNUSED_PARAMETER(NotUsed);

  ଯଦି( zName==0 ) zName = "";
  ପାଇଁ(i=0; i<db->nDb; i++){
    pDb = &db->aDb[i];
    ଯଦି( pDb->pBt==0 ) ଜାରି;
    ଯଦି( sqlite3DbIsNamed(db, i, zName) ) ଭାଙ୍ଗ;
  }

  ଯଦି( i>=db->nDb ){
    sqlite3_snprintf(ଆକାର(zErr),zErr, "no such database: %s", zName);
    ଯାଅ detach_error;
  }
  ଯଦି( i<2 ){
    sqlite3_snprintf(ଆକାର(zErr),zErr, "cannot detach database %s", zName);
    ଯାଅ detach_error;
  }
  ଯଦି( sqlite3BtreeTxnState(pDb->pBt)!=SQLITE_TXN_NONE
   || sqlite3BtreeIsInBackup(pDb->pBt)
  ){
    sqlite3_snprintf(ଆକାର(zErr),zErr, "database %s is locked", zName);
    ଯାଅ detach_error;
  }

  detachTempTriggers(db, pDb->pSchema);
  sqlite3BtreeClose(pDb->pBt);
  pDb->pBt = 0;
  pDb->pSchema = 0;
  sqlite3CollapseDatabaseArray(db);
  ଫେରନ୍ତୁ;

detach_error:
  sqlite3_result_error(context, zErr, -1);
}

/*
** This procedure generates VDBE code for a single invocation of either the
** sqlite_detach() or sqlite_attach() SQL user functions.
*/
ସ୍ଥିର ଶୂନ୍ୟ codeAttach(
  Parse *pParse,       /* The parser context */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା type,            /* Either SQLITE_ATTACH or SQLITE_DETACH */
  FuncDef ଅଚଳ *pFunc,/* FuncDef wrapper for detachFunc() or attachFunc() */
  ପଦସମଷ୍ଟି *pAuthArg,      /* Expression to pass to authorization callback */
  ପଦସମଷ୍ଟି *pFilename,     /* Name of database file */
  ପଦସମଷ୍ଟି *pDbname,       /* Name of the database to use internally */
  ପଦସମଷ୍ଟି *p_ଚାବି           /* Database key for encryption extension */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  NameContext sName;
  Vdbe *v;
  sqlite3* db = pParse->db;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା regArgs;

  ଯଦି( SQLITE_OK!=sqlite3ReadSchema(pParse) ) ଯାଅ attach_end;

  ଯଦି( pParse->nErr ) ଯାଅ attach_end;
  memset(&sName, 0, ଆକାର(NameContext));
  sName.pParse = pParse;

  ଯଦି( 
      SQLITE_OK!=resolve_Attach_ପଦସମଷ୍ଟି(&sName, pFilename) ||
      SQLITE_OK!=resolve_Attach_ପଦସମଷ୍ଟି(&sName, pDbname) ||
      SQLITE_OK!=resolve_Attach_ପଦସମଷ୍ଟି(&sName, p_ଚାବି)
  ){
    ଯାଅ attach_end;
  }

#ifndef SQLITE_OMIT_AUTHORIZATION
  ଯଦି( ALWAYS(pAuthArg) ){
    ଅକ୍ଷର *zAuthArg;
    ଯଦି( pAuthArg->op==TK_STRING ){
      ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(pAuthArg, EP_Int_ମୂଲ୍ୟ) );
      zAuthArg = pAuthArg->u.zToken;
    }ଅନ୍ୟଥା{
      zAuthArg = 0;
    }
    rc = sqlite3AuthCheck(pParse, type, zAuthArg, 0, 0);
    ଯଦି(rc!=SQLITE_OK ){
      ଯାଅ attach_end;
    }
  }
#endif /* SQLITE_OMIT_AUTHORIZATION */


  v = sqlite3GetVdbe(pParse);
  regArgs = sqlite3GetTempRange(pParse, 4);
  sqlite3_ପଦସମଷ୍ଟି_Code(pParse, pFilename, regArgs);
  sqlite3_ପଦସମଷ୍ଟି_Code(pParse, pDbname, regArgs+1);
  sqlite3_ପଦସମଷ୍ଟି_Code(pParse, p_ଚାବି, regArgs+2);

  ପ୍ରତିଜ୍ଞା( v || db->mallocFailed );
  ଯଦି( v ){
    sqlite3VdbeAddFunctionCall(pParse, 0, regArgs+3-pFunc->nArg, regArgs+3,
                               pFunc->nArg, pFunc, 0);
    /* Code an OP_Expire. For an ATTACH statement, set P1 to true (expire this
    ** statement only). For DETACH, set it to false (expire all existing
    ** statements).
    */
    sqlite3VdbeAddOp1(v, OP_Expire, (type==SQLITE_ATTACH));
  }
  
attach_end:
  sqlite3_ପଦସମଷ୍ଟି_Delete(db, pFilename);
  sqlite3_ପଦସମଷ୍ଟି_Delete(db, pDbname);
  sqlite3_ପଦସମଷ୍ଟି_Delete(db, p_ଚାବି);
}

/*
** Called by the parser to compile a DETACH statement.
**
**     DETACH pDbname
*/
ଶୂନ୍ୟ sqlite3Detach(Parse *pParse, ପଦସମଷ୍ଟି *pDbname){
  ସ୍ଥିର ଅଚଳ FuncDef detach_func = {
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
ଶୂନ୍ୟ sqlite3Attach(Parse *pParse, ପଦସମଷ୍ଟି *p, ପଦସମଷ୍ଟି *pDbname, ପଦସମଷ୍ଟି *p_ଚାବି){
  ସ୍ଥିର ଅଚଳ FuncDef attach_func = {
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
  codeAttach(pParse, SQLITE_ATTACH, &attach_func, p, p, pDbname, p_ଚାବି);
}
#endif /* SQLITE_OMIT_ATTACH */

/*
** Expression callback used by sqlite3FixAAAA() routines.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fix_ପଦସମଷ୍ଟି_Cb(Walker *p, ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି){
  DbFixer *pFix = p->u.pFix;
  ଯଦି( !pFix->bTemp ) ପଦସମଷ୍ଟି_Set_Property(p_ପଦସମଷ୍ଟି, EP_FromDDL);
  ଯଦି( p_ପଦସମଷ୍ଟି->op==TK_VARIABLE ){
    ଯଦି( pFix->pParse->db->init.busy ){
      p_ପଦସମଷ୍ଟି->op = TK_NULL;
    }ଅନ୍ୟଥା{
      sqlite3_Error_ସନ୍ଦେଶ(pFix->pParse, "%s cannot use variables", pFix->zType);
      ଫେରନ୍ତୁ WRC_Abort;
    }
  }
  ଫେରନ୍ତୁ WRC_Continue;
}

/*
** Select callback used by sqlite3FixAAAA() routines.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fixSelectCb(Walker *p, Select *pSelect){
  DbFixer *pFix = p->u.pFix;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  SrcItem *pItem;
  sqlite3 *db = pFix->pParse->db;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDb = sqlite3FindDbName(db, pFix->zDb);
  SrcList *pList = pSelect->pSrc;

  ଯଦି( NEVER(pList==0) ) ଫେରନ୍ତୁ WRC_Continue;
  ପାଇଁ(i=0, pItem=pList->a; i<pList->nSrc; i++, pItem++){
    ଯଦି( pFix->bTemp==0 && pItem->fg.isSubquery==0 ){
      ଯଦି( pItem->fg.fixedSchema==0 && pItem->u4.zDatabase!=0 ){
        ଯଦି( iDb!=sqlite3FindDbName(db, pItem->u4.zDatabase) ){
          sqlite3_Error_ସନ୍ଦେଶ(pFix->pParse,
              "%s %T cannot reference objects in database %s",
              pFix->zType, pFix->pName, pItem->u4.zDatabase);
          ଫେରନ୍ତୁ WRC_Abort;
        }
        sqlite3DbFree(db, pItem->u4.zDatabase);
        pItem->fg.notCte = 1;
        pItem->fg.hadSchema = 1;
      }
      pItem->u4.pSchema = pFix->pSchema;
      pItem->fg.fromDDL = 1;
      pItem->fg.fixedSchema = 1;
    }
#ଯଦି !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_TRIGGER)
    ଯଦି( pList->a[i].fg.isUsing==0
     && sqlite3_Walk_ପଦସମଷ୍ଟି(&pFix->w, pList->a[i].u3.pOn)
    ){
      ଫେରନ୍ତୁ WRC_Abort;
    }
#endif
  }
  ଯଦି( pSelect->pWith ){
    ପାଇଁ(i=0; i<pSelect->pWith->nCte; i++){
      ଯଦି( sqlite3WalkSelect(p, pSelect->pWith->a[i].pSelect) ){
        ଫେରନ୍ତୁ WRC_Abort;
      }
    }
  }
  ଫେରନ୍ତୁ WRC_Continue;
}

/*
** Initialize a DbFixer structure.  This routine must be called prior
** to passing the structure to one of the sqliteFixAAAA() routines below.
*/
ଶୂନ୍ୟ sqlite3FixInit(
  DbFixer *pFix,      /* The fixer to be initialized */
  Parse *pParse,      /* Error messages will be written here */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDb,            /* This is the database that must be used */
  ଅଚଳ ଅକ୍ଷର *zType,  /* "view", "trigger", or "index" */
  ଅଚଳ Token *pName  /* Name of the view, trigger, or index */
){
  sqlite3 *db = pParse->db;
  ପ୍ରତିଜ୍ଞା( db->nDb>iDb );
  pFix->pParse = pParse;
  pFix->zDb = db->aDb[iDb].zDbSName;
  pFix->pSchema = db->aDb[iDb].pSchema;
  pFix->zType = zType;
  pFix->pName = pName;
  pFix->bTemp = (iDb==1);
  pFix->w.pParse = pParse;
  pFix->w.x_ପଦସମଷ୍ଟି_Callback = fix_ପଦସମଷ୍ଟି_Cb;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3FixSrcList(
  DbFixer *pFix,       /* Context of the fixation */
  SrcList *pList       /* The Source list to check and modify */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା res = 0;
  ଯଦି( pList ){
    Select s; 
    memset(&s, 0, ଆକାର(s));
    s.pSrc = pList;
    res = sqlite3WalkSelect(&pFix->w, &s);
  }
  ଫେରନ୍ତୁ res;
}
#ଯଦି !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_TRIGGER)
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3FixSelect(
  DbFixer *pFix,       /* Context of the fixation */
  Select *pSelect      /* The SELECT statement to be fixed to one database */
){
  ଫେରନ୍ତୁ sqlite3WalkSelect(&pFix->w, pSelect);
}
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_Fix_ପଦସମଷ୍ଟି(
  DbFixer *pFix,     /* Context of the fixation */
  ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି        /* The expression to be fixed to one database */
){
  ଫେରନ୍ତୁ sqlite3_Walk_ପଦସମଷ୍ଟି(&pFix->w, p_ପଦସମଷ୍ଟି);
}
#endif

#ifndef SQLITE_OMIT_TRIGGER
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3FixTriggerStep(
  DbFixer *pFix,     /* Context of the fixation */
  TriggerStep *pStep /* The trigger step be fixed to one database */
){
  ଯେତେବେଳେ( pStep ){
    ଯଦି( sqlite3WalkSelect(&pFix->w, pStep->pSelect)
     || sqlite3_Walk_ପଦସମଷ୍ଟି(&pFix->w, pStep->pWhere) 
     || sqlite3_Walk_ପଦସମଷ୍ଟି_List(&pFix->w, pStep->p_ପଦସମଷ୍ଟି_List)
     || sqlite3FixSrcList(pFix, pStep->pSrc)
    ){
      ଫେରନ୍ତୁ 1;
    }
#ifndef SQLITE_OMIT_UPSERT
    {
      Upsert *pUp;
      ପାଇଁ(pUp=pStep->pUpsert; pUp; pUp=pUp->pNextUpsert){
        ଯଦି( sqlite3_Walk_ପଦସମଷ୍ଟି_List(&pFix->w, pUp->pUpsertTarget)
         || sqlite3_Walk_ପଦସମଷ୍ଟି(&pFix->w, pUp->pUpsertTargetWhere)
         || sqlite3_Walk_ପଦସମଷ୍ଟି_List(&pFix->w, pUp->pUpsertSet)
         || sqlite3_Walk_ପଦସମଷ୍ଟି(&pFix->w, pUp->pUpsertWhere)
        ){
          ଫେରନ୍ତୁ 1;
        }
      }
    }
#endif
    pStep = pStep->pNext;
  }

  ଫେରନ୍ତୁ 0;
}
#endif
