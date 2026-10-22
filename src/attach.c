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
#అన్తర్భావ "sqliteInt.h"

#నిర్వచితం_కాకపోతే SQLITE_OMIT_ATTACH
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
స్థిర పూర్ణాంకం resolve_Attach_సమాసం(NameContext *pName, సమాసం *p_సమాసం)
{
  పూర్ణాంకం rc = SQLITE_OK;
  ఒకవేళ( p_సమాసం ){
    ఒకవేళ( p_సమాసం->op!=TK_ID ){
      rc = sqlite3_Resolve_సమాసం_పేర్లు(pName, p_సమాసం);
    }లేకపోతే{
      p_సమాసం->op = TK_STRING;
    }
  }
  తిరిగి rc;
}

/*
** Return true if zName points to a name that may be used to refer to
** database iDb attached to handle db.
*/
పూర్ణాంకం sqlite3DbIsNamed(sqlite3 *db, పూర్ణాంకం iDb, స్థిరరాశి అక్షరం *zName){
  తిరిగి (
      sqlite3StrICmp(db->aDb[iDb].zDbSName, zName)==0
   || (iDb==0 && sqlite3StrICmp("main", zName)==0)
  );
}

/*
** If any TEMP triggers reference schema pSchema, move those triggers to
** reference the TEMP schema itself.
*/
స్థిర శూన్య detach_తాత్కాలిక_Triggers(sqlite3 *db, Schema *pSchema){
  HashElem *p_నమోదు;
  ప్రతిజ్ఞా( db->aDb[1].pSchema );
  p_నమోదు = sqlite_Hash_మొదటి(&db->aDb[1].pSchema->trigHash);
  అయితే( p_నమోదు ){
    Trigger *pTrig = (Trigger*)sqlite_Hash_దత్తాంశం(p_నమోదు);
    ఒకవేళ( pTrig->pTabSchema==pSchema ){
      pTrig->pTabSchema = pTrig->pSchema;
    }
    p_నమోదు = sqlite_Hash_తదుపరి(p_నమోదు);
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
స్థిర శూన్య attachFunc(
  sqlite3_context *context,
  పూర్ణాంకం Not_వాడిన,
  sqlite3_విలువ **argv
){
  పూర్ణాంకం i;
  పూర్ణాంకం rc = 0;
  sqlite3 *db = sqlite3_context_db_handle(context);
  స్థిరరాశి అక్షరం *zName;
  స్థిరరాశి అక్షరం *zFile;
  అక్షరం *z_మార్గం = 0;
  అక్షరం *zErr = 0;
  గుర్తులేని పూర్ణాంకం flags;
  Db *aNew;                 /* New array of Db pointers */
  Db *pNew = 0;             /* Db object for the newly attached database */
  అక్షరం *zErrDyn = 0;
  sqlite3_vfs *pVfs;

  వాడని_PARAMETER(Not_వాడిన);
  zFile = (స్థిరరాశి అక్షరం *)sqlite3_విలువ_పాఠ్యం(argv[0]);
  zName = (స్థిరరాశి అక్షరం *)sqlite3_విలువ_పాఠ్యం(argv[1]);
  ఒకవేళ( zFile==0 ) zFile = "";
  ఒకవేళ( zName==0 ) zName = "";

#నిర్వచితం_కాకపోతే SQLITE_OMIT_DESERIALIZE
# define REOPEN_AS_MEMDB(db)  (db->init.reopenMemdb)
#లేకపోతే
# define REOPEN_AS_MEMDB(db)  (0)
#అయితే_ముగింపు

  ఒకవేళ( REOPEN_AS_MEMDB(db) ){
    /* This is not a real ATTACH.  Instead, this routine is being called
    ** from sqlite3_deserialize() to close database db->init.iDb and
    ** reopen it as a MemDB */
    Btree *pNewBt = 0;

    pNew = &db->aDb[db->init.iDb];
    ప్రతిజ్ఞా( pNew->pBt!=0 );
    ఒకవేళ( sqlite3BtreeTxnState(pNew->pBt)!=SQLITE_TXN_NONE
     || sqlite3_Btree_Is_లోపల_Backup(pNew->pBt)
    ){
      rc = SQLITE_BUSY;
      వెళ్ళు attach_error;
    }

    pVfs = sqlite3_vfs_కనుగొను("memdb");
    ఒకవేళ( pVfs==0 ) తిరిగి;
    rc = sqlite3BtreeOpen(pVfs, "x\0", db, &pNewBt, 0, SQLITE_OPEN_MAIN_DB);
    ఒకవేళ( rc==SQLITE_OK ){
      Schema *pNewSchema = sqlite3SchemaGet(db, pNewBt);
      ఒకవేళ( pNewSchema ){
        /* Both the Btree and the new Schema were allocated successfully.
        ** Close the old db and update the aDb[] slot with the new memdb
        ** values.  */
        detach_తాత్కాలిక_Triggers(db, pNew->pSchema);
        sqlite3BtreeClose(pNew->pBt);
        pNew->pBt = pNewBt;
        pNew->pSchema = pNewSchema;
        ఒకవేళ( db->init.iDb==0 ){
          /* Clear all eponymous virtual table instances, as they are holding
          ** pointers to the schema object just freed by sqlite3BtreeClose() */
          sqlite3_Vtab_Eponymous_పట్టిక_Clear_All(db);
        }
        sqlite3ExpirePreparedStatements(db, 1);
      }లేకపోతే{
        sqlite3BtreeClose(pNewBt);
        rc = SQLITE_NOMEM;
      }
    }
    ఒకవేళ( rc ) వెళ్ళు attach_error;
  }లేకపోతే{
    /* This is a real ATTACH
    **
    ** Check for the following errors:
    **
    **     * Too many attached databases,
    **     * Transaction currently open
    **     * Specified database name already being used.
    */
    ఒకవేళ( db->nDb>=db->a_హద్దు[SQLITE_హద్దు_ATTACHED]+2 ){
      zErrDyn = sqlite3MPrintf(db, "too many attached databases - max %d", 
        db->a_హద్దు[SQLITE_హద్దు_ATTACHED]
      );
      వెళ్ళు attach_error;
    }
    కొరకు(i=0; i<db->nDb; i++){
      ప్రతిజ్ఞా( zName );
      ఒకవేళ( sqlite3DbIsNamed(db, i, zName) ){
        zErrDyn = sqlite3MPrintf(db, "database %s is already in use", zName);
        వెళ్ళు attach_error;
      }
    }
  
    /* Allocate the new entry in the db->aDb[] array and initialize the schema
    ** hash tables.
    */
    ఒకవేళ( db->aDb==db->aDbStatic ){
      aNew = sqlite3DbMallocRawNN(db, పరిమాణం(db->aDb[0])*3 );
      ఒకవేళ( aNew==0 ) తిరిగి;
      జ్ఞాపకం_నకలు(aNew, db->aDb, పరిమాణం(db->aDb[0])*2);
    }లేకపోతే{
      aNew = sqlite3DbRealloc(db, db->aDb, పరిమాణం(db->aDb[0])*(1+(i64)db->nDb));
      ఒకవేళ( aNew==0 ) తిరిగి;
    }
    db->aDb = aNew;
    pNew = &db->aDb[db->nDb];
    memset(pNew, 0, పరిమాణం(*pNew));
  
    /* Open the database file. If the btree is successfully opened, use
    ** it to obtain the database schema. At this point the schema may
    ** or may not be initialized.
    */
    flags = db->openFlags;
    rc = sqlite3ParseUri(db->pVfs->zName, zFile, &flags, &pVfs, &z_మార్గం, &zErr);
    ఒకవేళ( rc!=SQLITE_OK ){
      ఒకవేళ( rc==SQLITE_NOMEM ) sqlite3OomFault(db);
      sqlite3_ఫలితం_error(context, zErr, -1);
      sqlite3_free(zErr);
      తిరిగి;
    }
    ఒకవేళ( (db->flags & SQLITE_AttachWrite)==0 ){
      flags &= ~(SQLITE_OPEN_CREATE|SQLITE_OPEN_READWRITE);
      flags |= SQLITE_OPEN_READONLY;
    }లేకపోతే ఒకవేళ( (db->flags & SQLITE_AttachCreate)==0 ){
      flags &= ~SQLITE_OPEN_CREATE;
    }
    ప్రతిజ్ఞా( pVfs );
    flags |= SQLITE_OPEN_MAIN_DB;
    rc = sqlite3BtreeOpen(pVfs, z_మార్గం, db, &pNew->pBt, 0, flags);
    db->nDb++;
    pNew->zDbSName = sqlite3DbStrDup(db, zName);
  }
  db->noSharedCache = 0;
  ఒకవేళ( rc==SQLITE_CONSTRAINT ){
    rc = SQLITE_ERROR;
    zErrDyn = sqlite3MPrintf(db, "database is already attached");
  }లేకపోతే ఒకవేళ( rc==SQLITE_OK ){
    Pager *pPager;
    pNew->pSchema = sqlite3SchemaGet(db, pNew->pBt);
    ఒకవేళ( !pNew->pSchema ){
      rc = SQLITE_NOMEM_BKPT;
    }లేకపోతే ఒకవేళ( pNew->pSchema->file_format && pNew->pSchema->enc!=ENC(db) ){
      zErrDyn = sqlite3MPrintf(db, 
        "attached databases must use the same text encoding as main database");
      rc = SQLITE_ERROR;
    }
    sqlite3BtreeEnter(pNew->pBt);
    pPager = sqlite3BtreePager(pNew->pBt);
    sqlite3_Pager_Locking_బహుళకం(pPager, db->dflt_తాళం_బహుళకం);
    sqlite3BtreeSecureDelete(pNew->pBt,
                             sqlite3BtreeSecureDelete(db->aDb[0].pBt,-1) );
#నిర్వచితం_కాకపోతే SQLITE_OMIT_PAGER_PRAGMAS
    sqlite3_Btree_సమితి_Pager_Flags(pNew->pBt,
                      PAGER_SYNCHRONOUS_నిండిన | (db->flags & PAGER_FLAGS_ముసుగు));
#అయితే_ముగింపు
    sqlite3BtreeLeave(pNew->pBt);
  }
  pNew->safety_level = SQLITE_DEFAULT_SYNCHRONOUS+1;
  ఒకవేళ( rc==SQLITE_OK && pNew->zDbSName==0 ){
    rc = SQLITE_NOMEM_BKPT;
  }
  sqlite3_free_filename( z_మార్గం );

  /* If the file was opened successfully, read the schema for the new database.
  ** If this fails, or if opening the file failed, then close the file and 
  ** remove the entry from the db->aDb[] array. i.e. put everything back the
  ** way we found it.
  */
  ఒకవేళ( rc==SQLITE_OK ){
    sqlite3BtreeEnterAll(db);
    db->init.iDb = 0;
    db->mDbFlags &= ~(DBFLAG_Schema_తెలిసిన_Ok);
#నిర్వచితమైతే SQLITE_ENABLE_SETLK_TIMEOUT
    ఒకవేళ( db->setlkFlags & SQLITE_SETLK_ఖండం_ON_CONNECT ){
      పూర్ణాంకం val = 1;
      sqlite3_file *fd = sqlite3PagerFile(sqlite3BtreePager(pNew->pBt));
      sqlite3OsFileControlHint(fd, SQLITE_FCNTL_ఖండం_ON_CONNECT, &val);
    }
#అయితే_ముగింపు
    ఒకవేళ( !REOPEN_AS_MEMDB(db) ){
      rc = sqlite3Init(db, &zErrDyn);
    }
    sqlite3BtreeLeaveAll(db);
    ప్రతిజ్ఞా( zErrDyn==0 || rc!=SQLITE_OK );
  }
  ఒకవేళ( rc ){
    ఒకవేళ( ALWAYS(!REOPEN_AS_MEMDB(db)) ){
      పూర్ణాంకం iDb = db->nDb - 1;
      ప్రతిజ్ఞా( iDb>=2 );
      ఒకవేళ( db->aDb[iDb].pBt ){
        sqlite3BtreeClose(db->aDb[iDb].pBt);
        db->aDb[iDb].pBt = 0;
        db->aDb[iDb].pSchema = 0;
      }
      sqlite3_Reset_All_Schemas_యొక్క_Connection(db);
      db->nDb = iDb;
      ఒకవేళ( rc==SQLITE_NOMEM || rc==SQLITE_IOERR_NOMEM ){
        sqlite3OomFault(db);
        sqlite3DbFree(db, zErrDyn);
        zErrDyn = sqlite3MPrintf(db, "out of memory");
      }లేకపోతే ఒకవేళ( zErrDyn==0 ){
        zErrDyn = sqlite3MPrintf(db, "unable to open database: %s", zFile);
      }
    }
    వెళ్ళు attach_error;
  }
  
  తిరిగి;

attach_error:
  /* Return an error if we get here */
  ఒకవేళ( zErrDyn ){
    sqlite3_ఫలితం_error(context, zErrDyn, -1);
    sqlite3DbFree(db, zErrDyn);
  }
  ఒకవేళ( rc ) sqlite3_ఫలితం_error_సంకేతం(context, rc);
}

/*
** An SQL user-function registered to do the work of an DETACH statement. The
** three arguments to the function come directly from a detach statement:
**
**     DETACH DATABASE x
**
**     SELECT sqlite_detach(x)
*/
స్థిర శూన్య detachFunc(
  sqlite3_context *context,
  పూర్ణాంకం Not_వాడిన,
  sqlite3_విలువ **argv
){
  స్థిరరాశి అక్షరం *zName = (స్థిరరాశి అక్షరం *)sqlite3_విలువ_పాఠ్యం(argv[0]);
  sqlite3 *db = sqlite3_context_db_handle(context);
  పూర్ణాంకం i;
  Db *pDb = 0;
  అక్షరం zErr[128];

  వాడని_PARAMETER(Not_వాడిన);

  ఒకవేళ( zName==0 ) zName = "";
  కొరకు(i=0; i<db->nDb; i++){
    pDb = &db->aDb[i];
    ఒకవేళ( pDb->pBt==0 ) కొనసాగు;
    ఒకవేళ( sqlite3DbIsNamed(db, i, zName) ) ఆపు;
  }

  ఒకవేళ( i>=db->nDb ){
    sqlite3_snprintf(పరిమాణం(zErr),zErr, "no such database: %s", zName);
    వెళ్ళు detach_error;
  }
  ఒకవేళ( i<2 ){
    sqlite3_snprintf(పరిమాణం(zErr),zErr, "cannot detach database %s", zName);
    వెళ్ళు detach_error;
  }
  ఒకవేళ( sqlite3BtreeTxnState(pDb->pBt)!=SQLITE_TXN_NONE
   || sqlite3_Btree_Is_లోపల_Backup(pDb->pBt)
  ){
    sqlite3_snprintf(పరిమాణం(zErr),zErr, "database %s is locked", zName);
    వెళ్ళు detach_error;
  }

  detach_తాత్కాలిక_Triggers(db, pDb->pSchema);
  sqlite3BtreeClose(pDb->pBt);
  pDb->pBt = 0;
  pDb->pSchema = 0;
  sqlite3CollapseDatabaseArray(db);
  తిరిగి;

detach_error:
  sqlite3_ఫలితం_error(context, zErr, -1);
}

/*
** This procedure generates VDBE code for a single invocation of either the
** sqlite_detach() or sqlite_attach() SQL user functions.
*/
స్థిర శూన్య సంకేతం_Attach(
  Parse *pParse,       /* The parser context */
  పూర్ణాంకం type,            /* Either SQLITE_ATTACH or SQLITE_DETACH */
  FuncDef స్థిరరాశి *pFunc,/* FuncDef wrapper for detachFunc() or attachFunc() */
  సమాసం *pAuthArg,      /* Expression to pass to authorization callback */
  సమాసం *pFilename,     /* Name of database file */
  సమాసం *pDbname,       /* Name of the database to use internally */
  సమాసం *p_కీలకం           /* Database key for encryption extension */
){
  పూర్ణాంకం rc;
  NameContext sName;
  Vdbe *v;
  sqlite3* db = pParse->db;
  పూర్ణాంకం reg_వాదనలు;

  ఒకవేళ( SQLITE_OK!=sqlite3ReadSchema(pParse) ) వెళ్ళు attach_చివర;

  ఒకవేళ( pParse->nErr ) వెళ్ళు attach_చివర;
  memset(&sName, 0, పరిమాణం(NameContext));
  sName.pParse = pParse;

  ఒకవేళ( 
      SQLITE_OK!=resolve_Attach_సమాసం(&sName, pFilename) ||
      SQLITE_OK!=resolve_Attach_సమాసం(&sName, pDbname) ||
      SQLITE_OK!=resolve_Attach_సమాసం(&sName, p_కీలకం)
  ){
    వెళ్ళు attach_చివర;
  }

#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTHORIZATION
  ఒకవేళ( ALWAYS(pAuthArg) ){
    అక్షరం *zAuthArg;
    ఒకవేళ( pAuthArg->op==TK_STRING ){
      ప్రతిజ్ఞా( !సమాసం_Has_Property(pAuthArg, EP_Int_విలువ) );
      zAuthArg = pAuthArg->u.zToken;
    }లేకపోతే{
      zAuthArg = 0;
    }
    rc = sqlite3AuthCheck(pParse, type, zAuthArg, 0, 0);
    ఒకవేళ(rc!=SQLITE_OK ){
      వెళ్ళు attach_చివర;
    }
  }
#అయితే_ముగింపు /* SQLITE_OMIT_AUTHORIZATION */


  v = sqlite3GetVdbe(pParse);
  reg_వాదనలు = sqlite3_Get_తాత్కాలిక_Range(pParse, 4);
  sqlite3_సమాసం_సంకేతం(pParse, pFilename, reg_వాదనలు);
  sqlite3_సమాసం_సంకేతం(pParse, pDbname, reg_వాదనలు+1);
  sqlite3_సమాసం_సంకేతం(pParse, p_కీలకం, reg_వాదనలు+2);

  ప్రతిజ్ఞా( v || db->mallocFailed );
  ఒకవేళ( v ){
    sqlite3_Vdbe_కూడు_Function_Call(pParse, 0, reg_వాదనలు+3-pFunc->nArg, reg_వాదనలు+3,
                               pFunc->nArg, pFunc, 0);
    /* Code an OP_Expire. For an ATTACH statement, set P1 to true (expire this
    ** statement only). For DETACH, set it to false (expire all existing
    ** statements).
    */
    sqlite3_Vdbe_కూడు_Op1(v, OP_Expire, (type==SQLITE_ATTACH));
  }
  
attach_చివర:
  sqlite3_సమాసం_Delete(db, pFilename);
  sqlite3_సమాసం_Delete(db, pDbname);
  sqlite3_సమాసం_Delete(db, p_కీలకం);
}

/*
** Called by the parser to compile a DETACH statement.
**
**     DETACH pDbname
*/
శూన్య sqlite3Detach(Parse *pParse, సమాసం *pDbname){
  స్థిర స్థిరరాశి FuncDef detach_func = {
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
  సంకేతం_Attach(pParse, SQLITE_DETACH, &detach_func, pDbname, 0, 0, pDbname);
}

/*
** Called by the parser to compile an ATTACH statement.
**
**     ATTACH p AS pDbname KEY pKey
*/
శూన్య sqlite3Attach(Parse *pParse, సమాసం *p, సమాసం *pDbname, సమాసం *p_కీలకం){
  స్థిర స్థిరరాశి FuncDef attach_func = {
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
  సంకేతం_Attach(pParse, SQLITE_ATTACH, &attach_func, p, p, pDbname, p_కీలకం);
}
#అయితే_ముగింపు /* SQLITE_OMIT_ATTACH */

/*
** Expression callback used by sqlite3FixAAAA() routines.
*/
స్థిర పూర్ణాంకం fix_సమాసం_Cb(Walker *p, సమాసం *p_సమాసం){
  DbFixer *pFix = p->u.pFix;
  ఒకవేళ( !pFix->b_తాత్కాలిక ) సమాసం_సమితి_Property(p_సమాసం, EP_FromDDL);
  ఒకవేళ( p_సమాసం->op==TK_VARIABLE ){
    ఒకవేళ( pFix->pParse->db->init.busy ){
      p_సమాసం->op = TK_NULL;
    }లేకపోతే{
      sqlite3_Error_సందేశం(pFix->pParse, "%s cannot use variables", pFix->zType);
      తిరిగి WRC_Abort;
    }
  }
  తిరిగి WRC_Continue;
}

/*
** Select callback used by sqlite3FixAAAA() routines.
*/
స్థిర పూర్ణాంకం fixSelectCb(Walker *p, Select *pSelect){
  DbFixer *pFix = p->u.pFix;
  పూర్ణాంకం i;
  Src_అంశం *p_అంశం;
  sqlite3 *db = pFix->pParse->db;
  పూర్ణాంకం iDb = sqlite3_కనుగొను_Db_Name(db, pFix->zDb);
  SrcList *pList = pSelect->pSrc;

  ఒకవేళ( NEVER(pList==0) ) తిరిగి WRC_Continue;
  కొరకు(i=0, p_అంశం=pList->a; i<pList->nSrc; i++, p_అంశం++){
    ఒకవేళ( pFix->b_తాత్కాలిక==0 && p_అంశం->fg.isSubquery==0 ){
      ఒకవేళ( p_అంశం->fg.fixedSchema==0 && p_అంశం->u4.zDatabase!=0 ){
        ఒకవేళ( iDb!=sqlite3_కనుగొను_Db_Name(db, p_అంశం->u4.zDatabase) ){
          sqlite3_Error_సందేశం(pFix->pParse,
              "%s %T cannot reference objects in database %s",
              pFix->zType, pFix->pName, p_అంశం->u4.zDatabase);
          తిరిగి WRC_Abort;
        }
        sqlite3DbFree(db, p_అంశం->u4.zDatabase);
        p_అంశం->fg.notCte = 1;
        p_అంశం->fg.hadSchema = 1;
      }
      p_అంశం->u4.pSchema = pFix->pSchema;
      p_అంశం->fg.fromDDL = 1;
      p_అంశం->fg.fixedSchema = 1;
    }
#ఒకవేళ !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_TRIGGER)
    ఒకవేళ( pList->a[i].fg.isUsing==0
     && sqlite3_Walk_సమాసం(&pFix->w, pList->a[i].u3.pOn)
    ){
      తిరిగి WRC_Abort;
    }
#అయితే_ముగింపు
  }
  ఒకవేళ( pSelect->pWith ){
    కొరకు(i=0; i<pSelect->pWith->nCte; i++){
      ఒకవేళ( sqlite3WalkSelect(p, pSelect->pWith->a[i].pSelect) ){
        తిరిగి WRC_Abort;
      }
    }
  }
  తిరిగి WRC_Continue;
}

/*
** Initialize a DbFixer structure.  This routine must be called prior
** to passing the structure to one of the sqliteFixAAAA() routines below.
*/
శూన్య sqlite3FixInit(
  DbFixer *pFix,      /* The fixer to be initialized */
  Parse *pParse,      /* Error messages will be written here */
  పూర్ణాంకం iDb,            /* This is the database that must be used */
  స్థిరరాశి అక్షరం *zType,  /* "view", "trigger", or "index" */
  స్థిరరాశి Token *pName  /* Name of the view, trigger, or index */
){
  sqlite3 *db = pParse->db;
  ప్రతిజ్ఞా( db->nDb>iDb );
  pFix->pParse = pParse;
  pFix->zDb = db->aDb[iDb].zDbSName;
  pFix->pSchema = db->aDb[iDb].pSchema;
  pFix->zType = zType;
  pFix->pName = pName;
  pFix->b_తాత్కాలిక = (iDb==1);
  pFix->w.pParse = pParse;
  pFix->w.x_సమాసం_Callback = fix_సమాసం_Cb;
  pFix->w.xSelectCallback = fixSelectCb;
  pFix->w.xSelectCallback2 = sqlite3WalkWinDefnDummyCallback;
  pFix->w.walker_లోతు = 0;
  pFix->w.e_సంకేతం = 0;
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
పూర్ణాంకం sqlite3FixSrcList(
  DbFixer *pFix,       /* Context of the fixation */
  SrcList *pList       /* The Source list to check and modify */
){
  పూర్ణాంకం res = 0;
  ఒకవేళ( pList ){
    Select s; 
    memset(&s, 0, పరిమాణం(s));
    s.pSrc = pList;
    res = sqlite3WalkSelect(&pFix->w, &s);
  }
  తిరిగి res;
}
#ఒకవేళ !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_TRIGGER)
పూర్ణాంకం sqlite3FixSelect(
  DbFixer *pFix,       /* Context of the fixation */
  Select *pSelect      /* The SELECT statement to be fixed to one database */
){
  తిరిగి sqlite3WalkSelect(&pFix->w, pSelect);
}
పూర్ణాంకం sqlite3_Fix_సమాసం(
  DbFixer *pFix,     /* Context of the fixation */
  సమాసం *p_సమాసం        /* The expression to be fixed to one database */
){
  తిరిగి sqlite3_Walk_సమాసం(&pFix->w, p_సమాసం);
}
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే SQLITE_OMIT_TRIGGER
పూర్ణాంకం sqlite3_Fix_Trigger_అడుగు(
  DbFixer *pFix,     /* Context of the fixation */
  Trigger_అడుగు *p_అడుగు /* The trigger step be fixed to one database */
){
  అయితే( p_అడుగు ){
    ఒకవేళ( sqlite3WalkSelect(&pFix->w, p_అడుగు->pSelect)
     || sqlite3_Walk_సమాసం(&pFix->w, p_అడుగు->pWhere) 
     || sqlite3_Walk_సమాసం_List(&pFix->w, p_అడుగు->p_సమాసం_List)
     || sqlite3FixSrcList(pFix, p_అడుగు->pSrc)
    ){
      తిరిగి 1;
    }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_UPSERT
    {
      Upsert *p_పైకి;
      కొరకు(p_పైకి=p_అడుగు->pUpsert; p_పైకి; p_పైకి=p_పైకి->p_తదుపరి_Upsert){
        ఒకవేళ( sqlite3_Walk_సమాసం_List(&pFix->w, p_పైకి->p_Upsert_లక్ష్యం)
         || sqlite3_Walk_సమాసం(&pFix->w, p_పైకి->p_Upsert_లక్ష్యం_Where)
         || sqlite3_Walk_సమాసం_List(&pFix->w, p_పైకి->p_Upsert_సమితి)
         || sqlite3_Walk_సమాసం(&pFix->w, p_పైకి->pUpsertWhere)
        ){
          తిరిగి 1;
        }
      }
    }
#అయితే_ముగింపు
    p_అడుగు = p_అడుగు->p_తదుపరి;
  }

  తిరిగి 0;
}
#అయితే_ముగింపు
