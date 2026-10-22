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

#यदि_अपरिभाषितम् SQLITE_OMIT_ATTACH
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
  यदि( p_व्यञ्जकम् ){
    यदि( p_व्यञ्जकम्->op!=TK_ID ){
      rc = sqlite3_Resolve_व्यञ्जकम्_नामानि(pName, p_व्यञ्जकम्);
    }अन्यथा{
      p_व्यञ्जकम्->op = TK_STRING;
    }
  }
  प्रत्यागम rc;
}

/*
** Return true if zName points to a name that may be used to refer to
** database iDb attached to handle db.
*/
पूर्णाङ्क sqlite3DbIsNamed(sqlite3 *db, पूर्णाङ्क iDb, ध्रुव वर्ण *zName){
  प्रत्यागम (
      sqlite3StrICmp(db->aDb[iDb].zDbSName, zName)==0
   || (iDb==0 && sqlite3StrICmp("main", zName)==0)
  );
}

/*
** If any TEMP triggers reference schema pSchema, move those triggers to
** reference the TEMP schema itself.
*/
स्थिर शून्य detach_अस्थायी_Triggers(sqlite3 *db, Schema *pSchema){
  HashElem *p_प्रविष्टिः;
  प्रतिज्ञा( db->aDb[1].pSchema );
  p_प्रविष्टिः = sqliteHashFirst(&db->aDb[1].pSchema->trigHash);
  यावत्( p_प्रविष्टिः ){
    Trigger *pTrig = (Trigger*)sqlite_Hash_दत्त(p_प्रविष्टिः);
    यदि( pTrig->pTabSchema==pSchema ){
      pTrig->pTabSchema = pTrig->pSchema;
    }
    p_प्रविष्टिः = sqlite_Hash_अग्र(p_प्रविष्टिः);
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
  पूर्णाङ्क Not_प्रयुक्तम्,
  sqlite3_मानम् **argv
){
  पूर्णाङ्क i;
  पूर्णाङ्क rc = 0;
  sqlite3 *db = sqlite3_context_db_handle(context);
  ध्रुव वर्ण *zName;
  ध्रुव वर्ण *zFile;
  वर्ण *z_मार्गः = 0;
  वर्ण *zErr = 0;
  अचिह्नित पूर्णाङ्क flags;
  Db *aNew;                 /* New array of Db pointers */
  Db *pNew = 0;             /* Db object for the newly attached database */
  वर्ण *zErrDyn = 0;
  sqlite3_vfs *pVfs;

  अप्रयुक्तम्_PARAMETER(Not_प्रयुक्तम्);
  zFile = (ध्रुव वर्ण *)sqlite3_मानम्_text(argv[0]);
  zName = (ध्रुव वर्ण *)sqlite3_मानम्_text(argv[1]);
  यदि( zFile==0 ) zFile = "";
  यदि( zName==0 ) zName = "";

#यदि_अपरिभाषितम् SQLITE_OMIT_DESERIALIZE
# define REOPEN_AS_MEMDB(db)  (db->init.reopenMemdb)
#अन्यथा
# define REOPEN_AS_MEMDB(db)  (0)
#यदि_अन्तः

  यदि( REOPEN_AS_MEMDB(db) ){
    /* This is not a real ATTACH.  Instead, this routine is being called
    ** from sqlite3_deserialize() to close database db->init.iDb and
    ** reopen it as a MemDB */
    Btree *pNewBt = 0;

    pNew = &db->aDb[db->init.iDb];
    प्रतिज्ञा( pNew->pBt!=0 );
    यदि( sqlite3BtreeTxnState(pNew->pBt)!=SQLITE_TXN_NONE
     || sqlite3_Btree_Is_अन्तर्_Backup(pNew->pBt)
    ){
      rc = SQLITE_BUSY;
      गच्छ attach_error;
    }

    pVfs = sqlite3_vfs_find("memdb");
    यदि( pVfs==0 ) प्रत्यागम;
    rc = sqlite3BtreeOpen(pVfs, "x\0", db, &pNewBt, 0, SQLITE_OPEN_MAIN_DB);
    यदि( rc==SQLITE_OK ){
      Schema *pNewSchema = sqlite3SchemaGet(db, pNewBt);
      यदि( pNewSchema ){
        /* Both the Btree and the new Schema were allocated successfully.
        ** Close the old db and update the aDb[] slot with the new memdb
        ** values.  */
        detach_अस्थायी_Triggers(db, pNew->pSchema);
        sqlite3BtreeClose(pNew->pBt);
        pNew->pBt = pNewBt;
        pNew->pSchema = pNewSchema;
        यदि( db->init.iDb==0 ){
          /* Clear all eponymous virtual table instances, as they are holding
          ** pointers to the schema object just freed by sqlite3BtreeClose() */
          sqlite3_Vtab_Eponymous_सारणी_Clear_All(db);
        }
        sqlite3ExpirePreparedStatements(db, 1);
      }अन्यथा{
        sqlite3BtreeClose(pNewBt);
        rc = SQLITE_NOMEM;
      }
    }
    यदि( rc ) गच्छ attach_error;
  }अन्यथा{
    /* This is a real ATTACH
    **
    ** Check for the following errors:
    **
    **     * Too many attached databases,
    **     * Transaction currently open
    **     * Specified database name already being used.
    */
    यदि( db->nDb>=db->a_सीमा[SQLITE_सीमा_ATTACHED]+2 ){
      zErrDyn = sqlite3MPrintf(db, "too many attached databases - max %d", 
        db->a_सीमा[SQLITE_सीमा_ATTACHED]
      );
      गच्छ attach_error;
    }
    प्रति(i=0; i<db->nDb; i++){
      प्रतिज्ञा( zName );
      यदि( sqlite3DbIsNamed(db, i, zName) ){
        zErrDyn = sqlite3MPrintf(db, "database %s is already in use", zName);
        गच्छ attach_error;
      }
    }
  
    /* Allocate the new entry in the db->aDb[] array and initialize the schema
    ** hash tables.
    */
    यदि( db->aDb==db->aDbStatic ){
      aNew = sqlite3DbMallocRawNN(db, परिमाण(db->aDb[0])*3 );
      यदि( aNew==0 ) प्रत्यागम;
      स्मृतिं_प्रतिलिख(aNew, db->aDb, परिमाण(db->aDb[0])*2);
    }अन्यथा{
      aNew = sqlite3DbRealloc(db, db->aDb, परिमाण(db->aDb[0])*(1+(i64)db->nDb));
      यदि( aNew==0 ) प्रत्यागम;
    }
    db->aDb = aNew;
    pNew = &db->aDb[db->nDb];
    स्मृतिं_पूरय(pNew, 0, परिमाण(*pNew));
  
    /* Open the database file. If the btree is successfully opened, use
    ** it to obtain the database schema. At this point the schema may
    ** or may not be initialized.
    */
    flags = db->openFlags;
    rc = sqlite3ParseUri(db->pVfs->zName, zFile, &flags, &pVfs, &z_मार्गः, &zErr);
    यदि( rc!=SQLITE_OK ){
      यदि( rc==SQLITE_NOMEM ) sqlite3OomFault(db);
      sqlite3_परिणाम_error(context, zErr, -1);
      sqlite3_free(zErr);
      प्रत्यागम;
    }
    यदि( (db->flags & SQLITE_AttachWrite)==0 ){
      flags &= ~(SQLITE_OPEN_CREATE|SQLITE_OPEN_READWRITE);
      flags |= SQLITE_OPEN_READONLY;
    }अन्यथा यदि( (db->flags & SQLITE_AttachCreate)==0 ){
      flags &= ~SQLITE_OPEN_CREATE;
    }
    प्रतिज्ञा( pVfs );
    flags |= SQLITE_OPEN_MAIN_DB;
    rc = sqlite3BtreeOpen(pVfs, z_मार्गः, db, &pNew->pBt, 0, flags);
    db->nDb++;
    pNew->zDbSName = sqlite3DbStrDup(db, zName);
  }
  db->noSharedCache = 0;
  यदि( rc==SQLITE_CONSTRAINT ){
    rc = SQLITE_ERROR;
    zErrDyn = sqlite3MPrintf(db, "database is already attached");
  }अन्यथा यदि( rc==SQLITE_OK ){
    Pager *pPager;
    pNew->pSchema = sqlite3SchemaGet(db, pNew->pBt);
    यदि( !pNew->pSchema ){
      rc = SQLITE_NOMEM_BKPT;
    }अन्यथा यदि( pNew->pSchema->file_format && pNew->pSchema->enc!=ENC(db) ){
      zErrDyn = sqlite3MPrintf(db, 
        "attached databases must use the same text encoding as main database");
      rc = SQLITE_ERROR;
    }
    sqlite3BtreeEnter(pNew->pBt);
    pPager = sqlite3BtreePager(pNew->pBt);
    sqlite3_Pager_Locking_बहुलकम्(pPager, db->dflt_तालकम्_बहुलकम्);
    sqlite3BtreeSecureDelete(pNew->pBt,
                             sqlite3BtreeSecureDelete(db->aDb[0].pBt,-1) );
#यदि_अपरिभाषितम् SQLITE_OMIT_PAGER_PRAGMAS
    sqlite3_Btree_समुच्चयः_Pager_Flags(pNew->pBt,
                      PAGER_SYNCHRONOUS_पूर्णम् | (db->flags & PAGER_FLAGS_आवरणम्));
#यदि_अन्तः
    sqlite3BtreeLeave(pNew->pBt);
  }
  pNew->safety_level = SQLITE_DEFAULT_SYNCHRONOUS+1;
  यदि( rc==SQLITE_OK && pNew->zDbSName==0 ){
    rc = SQLITE_NOMEM_BKPT;
  }
  sqlite3_free_filename( z_मार्गः );

  /* If the file was opened successfully, read the schema for the new database.
  ** If this fails, or if opening the file failed, then close the file and 
  ** remove the entry from the db->aDb[] array. i.e. put everything back the
  ** way we found it.
  */
  यदि( rc==SQLITE_OK ){
    sqlite3BtreeEnterAll(db);
    db->init.iDb = 0;
    db->mDbFlags &= ~(DBFLAG_Schema_ज्ञातम्_Ok);
#यदि_परिभाषितम् SQLITE_ENABLE_SETLK_TIMEOUT
    यदि( db->setlkFlags & SQLITE_SETLK_खण्डम्_ON_CONNECT ){
      पूर्णाङ्क val = 1;
      sqlite3_file *fd = sqlite3PagerFile(sqlite3BtreePager(pNew->pBt));
      sqlite3OsFileControlHint(fd, SQLITE_FCNTL_खण्डम्_ON_CONNECT, &val);
    }
#यदि_अन्तः
    यदि( !REOPEN_AS_MEMDB(db) ){
      rc = sqlite3Init(db, &zErrDyn);
    }
    sqlite3BtreeLeaveAll(db);
    प्रतिज्ञा( zErrDyn==0 || rc!=SQLITE_OK );
  }
  यदि( rc ){
    यदि( ALWAYS(!REOPEN_AS_MEMDB(db)) ){
      पूर्णाङ्क iDb = db->nDb - 1;
      प्रतिज्ञा( iDb>=2 );
      यदि( db->aDb[iDb].pBt ){
        sqlite3BtreeClose(db->aDb[iDb].pBt);
        db->aDb[iDb].pBt = 0;
        db->aDb[iDb].pSchema = 0;
      }
      sqlite3_Reset_All_Schemas_सम्बन्धि_Connection(db);
      db->nDb = iDb;
      यदि( rc==SQLITE_NOMEM || rc==SQLITE_IOERR_NOMEM ){
        sqlite3OomFault(db);
        sqlite3DbFree(db, zErrDyn);
        zErrDyn = sqlite3MPrintf(db, "out of memory");
      }अन्यथा यदि( zErrDyn==0 ){
        zErrDyn = sqlite3MPrintf(db, "unable to open database: %s", zFile);
      }
    }
    गच्छ attach_error;
  }
  
  प्रत्यागम;

attach_error:
  /* Return an error if we get here */
  यदि( zErrDyn ){
    sqlite3_परिणाम_error(context, zErrDyn, -1);
    sqlite3DbFree(db, zErrDyn);
  }
  यदि( rc ) sqlite3_परिणाम_error_संहिता(context, rc);
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
  पूर्णाङ्क Not_प्रयुक्तम्,
  sqlite3_मानम् **argv
){
  ध्रुव वर्ण *zName = (ध्रुव वर्ण *)sqlite3_मानम्_text(argv[0]);
  sqlite3 *db = sqlite3_context_db_handle(context);
  पूर्णाङ्क i;
  Db *pDb = 0;
  वर्ण zErr[128];

  अप्रयुक्तम्_PARAMETER(Not_प्रयुक्तम्);

  यदि( zName==0 ) zName = "";
  प्रति(i=0; i<db->nDb; i++){
    pDb = &db->aDb[i];
    यदि( pDb->pBt==0 ) अनुवर्त;
    यदि( sqlite3DbIsNamed(db, i, zName) ) भङ्ग;
  }

  यदि( i>=db->nDb ){
    sqlite3_snprintf(परिमाण(zErr),zErr, "no such database: %s", zName);
    गच्छ detach_error;
  }
  यदि( i<2 ){
    sqlite3_snprintf(परिमाण(zErr),zErr, "cannot detach database %s", zName);
    गच्छ detach_error;
  }
  यदि( sqlite3BtreeTxnState(pDb->pBt)!=SQLITE_TXN_NONE
   || sqlite3_Btree_Is_अन्तर्_Backup(pDb->pBt)
  ){
    sqlite3_snprintf(परिमाण(zErr),zErr, "database %s is locked", zName);
    गच्छ detach_error;
  }

  detach_अस्थायी_Triggers(db, pDb->pSchema);
  sqlite3BtreeClose(pDb->pBt);
  pDb->pBt = 0;
  pDb->pSchema = 0;
  sqlite3CollapseDatabaseArray(db);
  प्रत्यागम;

detach_error:
  sqlite3_परिणाम_error(context, zErr, -1);
}

/*
** This procedure generates VDBE code for a single invocation of either the
** sqlite_detach() or sqlite_attach() SQL user functions.
*/
स्थिर शून्य संहिता_Attach(
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
  पूर्णाङ्क reg_तर्काः;

  यदि( SQLITE_OK!=sqlite3ReadSchema(pParse) ) गच्छ attach_अन्तः;

  यदि( pParse->nErr ) गच्छ attach_अन्तः;
  स्मृतिं_पूरय(&sName, 0, परिमाण(NameContext));
  sName.pParse = pParse;

  यदि( 
      SQLITE_OK!=resolve_Attach_व्यञ्जकम्(&sName, pFilename) ||
      SQLITE_OK!=resolve_Attach_व्यञ्जकम्(&sName, pDbname) ||
      SQLITE_OK!=resolve_Attach_व्यञ्जकम्(&sName, p_कुञ्चिका)
  ){
    गच्छ attach_अन्तः;
  }

#यदि_अपरिभाषितम् SQLITE_OMIT_AUTHORIZATION
  यदि( ALWAYS(pAuthArg) ){
    वर्ण *zAuthArg;
    यदि( pAuthArg->op==TK_STRING ){
      प्रतिज्ञा( !व्यञ्जकम्_Has_Property(pAuthArg, EP_Int_मानम्) );
      zAuthArg = pAuthArg->u.zToken;
    }अन्यथा{
      zAuthArg = 0;
    }
    rc = sqlite3AuthCheck(pParse, type, zAuthArg, 0, 0);
    यदि(rc!=SQLITE_OK ){
      गच्छ attach_अन्तः;
    }
  }
#यदि_अन्तः /* SQLITE_OMIT_AUTHORIZATION */


  v = sqlite3GetVdbe(pParse);
  reg_तर्काः = sqlite3_Get_अस्थायी_Range(pParse, 4);
  sqlite3_व्यञ्जकम्_संहिता(pParse, pFilename, reg_तर्काः);
  sqlite3_व्यञ्जकम्_संहिता(pParse, pDbname, reg_तर्काः+1);
  sqlite3_व्यञ्जकम्_संहिता(pParse, p_कुञ्चिका, reg_तर्काः+2);

  प्रतिज्ञा( v || db->mallocFailed );
  यदि( v ){
    sqlite3_Vdbe_सङ्कलय_Function_Call(pParse, 0, reg_तर्काः+3-pFunc->nArg, reg_तर्काः+3,
                               pFunc->nArg, pFunc, 0);
    /* Code an OP_Expire. For an ATTACH statement, set P1 to true (expire this
    ** statement only). For DETACH, set it to false (expire all existing
    ** statements).
    */
    sqlite3_Vdbe_सङ्कलय_Op1(v, OP_Expire, (type==SQLITE_ATTACH));
  }
  
attach_अन्तः:
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
  संहिता_Attach(pParse, SQLITE_DETACH, &detach_func, pDbname, 0, 0, pDbname);
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
  संहिता_Attach(pParse, SQLITE_ATTACH, &attach_func, p, p, pDbname, p_कुञ्चिका);
}
#यदि_अन्तः /* SQLITE_OMIT_ATTACH */

/*
** Expression callback used by sqlite3FixAAAA() routines.
*/
स्थिर पूर्णाङ्क fix_व्यञ्जकम्_Cb(Walker *p, व्यञ्जकम् *p_व्यञ्जकम्){
  DbFixer *pFix = p->u.pFix;
  यदि( !pFix->b_अस्थायी ) व्यञ्जकम्_समुच्चयः_Property(p_व्यञ्जकम्, EP_FromDDL);
  यदि( p_व्यञ्जकम्->op==TK_VARIABLE ){
    यदि( pFix->pParse->db->init.busy ){
      p_व्यञ्जकम्->op = TK_NULL;
    }अन्यथा{
      sqlite3_Error_सन्देशः(pFix->pParse, "%s cannot use variables", pFix->zType);
      प्रत्यागम WRC_Abort;
    }
  }
  प्रत्यागम WRC_Continue;
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

  यदि( NEVER(pList==0) ) प्रत्यागम WRC_Continue;
  प्रति(i=0, pItem=pList->a; i<pList->nSrc; i++, pItem++){
    यदि( pFix->b_अस्थायी==0 && pItem->fg.isSubquery==0 ){
      यदि( pItem->fg.fixedSchema==0 && pItem->u4.zDatabase!=0 ){
        यदि( iDb!=sqlite3FindDbName(db, pItem->u4.zDatabase) ){
          sqlite3_Error_सन्देशः(pFix->pParse,
              "%s %T cannot reference objects in database %s",
              pFix->zType, pFix->pName, pItem->u4.zDatabase);
          प्रत्यागम WRC_Abort;
        }
        sqlite3DbFree(db, pItem->u4.zDatabase);
        pItem->fg.notCte = 1;
        pItem->fg.hadSchema = 1;
      }
      pItem->u4.pSchema = pFix->pSchema;
      pItem->fg.fromDDL = 1;
      pItem->fg.fixedSchema = 1;
    }
#यदि !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_TRIGGER)
    यदि( pList->a[i].fg.isUsing==0
     && sqlite3_Walk_व्यञ्जकम्(&pFix->w, pList->a[i].u3.pOn)
    ){
      प्रत्यागम WRC_Abort;
    }
#यदि_अन्तः
  }
  यदि( pSelect->pWith ){
    प्रति(i=0; i<pSelect->pWith->nCte; i++){
      यदि( sqlite3WalkSelect(p, pSelect->pWith->a[i].pSelect) ){
        प्रत्यागम WRC_Abort;
      }
    }
  }
  प्रत्यागम WRC_Continue;
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
  pFix->b_अस्थायी = (iDb==1);
  pFix->w.pParse = pParse;
  pFix->w.x_व्यञ्जकम्_Callback = fix_व्यञ्जकम्_Cb;
  pFix->w.xSelectCallback = fixSelectCb;
  pFix->w.xSelectCallback2 = sqlite3WalkWinDefnDummyCallback;
  pFix->w.walker_गभीरता = 0;
  pFix->w.e_संहिता = 0;
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
  यदि( pList ){
    Select s; 
    स्मृतिं_पूरय(&s, 0, परिमाण(s));
    s.pSrc = pList;
    res = sqlite3WalkSelect(&pFix->w, &s);
  }
  प्रत्यागम res;
}
#यदि !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_TRIGGER)
पूर्णाङ्क sqlite3FixSelect(
  DbFixer *pFix,       /* Context of the fixation */
  Select *pSelect      /* The SELECT statement to be fixed to one database */
){
  प्रत्यागम sqlite3WalkSelect(&pFix->w, pSelect);
}
पूर्णाङ्क sqlite3_Fix_व्यञ्जकम्(
  DbFixer *pFix,     /* Context of the fixation */
  व्यञ्जकम् *p_व्यञ्जकम्        /* The expression to be fixed to one database */
){
  प्रत्यागम sqlite3_Walk_व्यञ्जकम्(&pFix->w, p_व्यञ्जकम्);
}
#यदि_अन्तः

#यदि_अपरिभाषितम् SQLITE_OMIT_TRIGGER
पूर्णाङ्क sqlite3_Fix_Trigger_चरणः(
  DbFixer *pFix,     /* Context of the fixation */
  Trigger_चरणः *p_चरणः /* The trigger step be fixed to one database */
){
  यावत्( p_चरणः ){
    यदि( sqlite3WalkSelect(&pFix->w, p_चरणः->pSelect)
     || sqlite3_Walk_व्यञ्जकम्(&pFix->w, p_चरणः->pWhere) 
     || sqlite3_Walk_व्यञ्जकम्_List(&pFix->w, p_चरणः->p_व्यञ्जकम्_List)
     || sqlite3FixSrcList(pFix, p_चरणः->pSrc)
    ){
      प्रत्यागम 1;
    }
#यदि_अपरिभाषितम् SQLITE_OMIT_UPSERT
    {
      Upsert *p_ऊर्ध्वम्;
      प्रति(p_ऊर्ध्वम्=p_चरणः->pUpsert; p_ऊर्ध्वम्; p_ऊर्ध्वम्=p_ऊर्ध्वम्->p_अग्र_Upsert){
        यदि( sqlite3_Walk_व्यञ्जकम्_List(&pFix->w, p_ऊर्ध्वम्->pUpsertTarget)
         || sqlite3_Walk_व्यञ्जकम्(&pFix->w, p_ऊर्ध्वम्->pUpsertTargetWhere)
         || sqlite3_Walk_व्यञ्जकम्_List(&pFix->w, p_ऊर्ध्वम्->p_Upsert_समुच्चयः)
         || sqlite3_Walk_व्यञ्जकम्(&pFix->w, p_ऊर्ध्वम्->pUpsertWhere)
        ){
          प्रत्यागम 1;
        }
      }
    }
#यदि_अन्तः
    p_चरणः = p_चरणः->p_अग्र;
  }

  प्रत्यागम 0;
}
#यदि_अन्तः
