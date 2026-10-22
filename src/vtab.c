/*
** 2006 June 10
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file contains code used to help implement virtual tables.
*/
#ifndef SQLITE_OMIT_VIRTUALTABLE
#अन्तर्भाव "sqliteInt.h"

/*
** Before a virtual table xCreate() or xConnect() method is invoked, the
** sqlite3.pVtabCtx member variable is set to point to an instance of
** this struct allocated on the stack. It is used by the implementation of
** the sqlite3_declare_vtab() and sqlite3_vtab_config() APIs, both of which
** are invoked only from within xCreate and xConnect methods.
*/
संरचना VtabCtx {
  VTable *pVTable;    /* The virtual table being constructed */
  Table *pTab;        /* The Table object to which the virtual table belongs */
  VtabCtx *pPrior;    /* Parent context (if any) */
  पूर्णाङ्क bDeclared;      /* True after sqlite3_declare_vtab() is called */
};

/*
** Construct and install a Module object for a virtual table.  When this
** routine is called, it is guaranteed that all appropriate locks are held
** and the module is not already part of the connection.
**
** If there already exists a module with zName, replace it with the new one.
** If pModule==0, then delete the module zName if it exists.
*/
Module *sqlite3VtabCreateModule(
  sqlite3 *db,                    /* Database in which module is registered */
  ध्रुव वर्ण *zName,              /* Name assigned to this module */
  ध्रुव sqlite3_module *pModule,  /* The definition of the module */
  शून्य *pAux,                     /* Context pointer for xCreate/xConnect */
  शून्य (*xDestroy)(शून्य *)        /* Module destructor function */
){
  Module *pMod;
  Module *pDel;
  वर्ण *zCopy;
  जइ( pModule==0 ){
    zCopy = (वर्ण*)zName;
    pMod = 0;
  }अण्णहा{
    पूर्णाङ्क nName = sqlite3Strlen30(zName);
    pMod = (Module *)sqlite3Malloc(परिमाण(Module) + nName + 1);
    जइ( pMod==0 ){
      sqlite3OomFault(db);
      प्रत्यागम 0;
    }
    zCopy = (वर्ण *)(&pMod[1]);
    memcpy(zCopy, zName, nName+1);
    pMod->zName = zCopy;
    pMod->pModule = pModule;
    pMod->pAux = pAux;
    pMod->xDestroy = xDestroy;
    pMod->pEpoTab = 0;
    pMod->nRefModule = 1;
  }
  pDel = (Module *)sqlite3_Hash_निवेशय(&db->aModule,zCopy,(शून्य*)pMod);
  जइ( pDel ){
    जइ( pDel==pMod ){
      sqlite3OomFault(db);
      sqlite3DbFree(db, pDel);
      pMod = 0;
    }अण्णहा{
      sqlite3VtabEponymousTableClear(db, pDel);
      sqlite3VtabModuleUnref(db, pDel);
    }
  }
  प्रत्यागम pMod;
}

/*
** The actual function that does the work of creating a new module.
** This function implements the sqlite3_create_module() and
** sqlite3_create_module_v2() interfaces.
*/
स्थिर पूर्णाङ्क createModule(
  sqlite3 *db,                    /* Database in which module is registered */
  ध्रुव वर्ण *zName,              /* Name assigned to this module */
  ध्रुव sqlite3_module *pModule,  /* The definition of the module */
  शून्य *pAux,                     /* Context pointer for xCreate/xConnect */
  शून्य (*xDestroy)(शून्य *)        /* Module destructor function */
){
  पूर्णाङ्क rc = SQLITE_OK;

  sqlite3_mutex_enter(db->mutex);
  (शून्य)sqlite3VtabCreateModule(db, zName, pModule, pAux, xDestroy);
  rc = sqlite3ApiExit(db, rc);
  जइ( rc!=SQLITE_OK && xDestroy ) xDestroy(pAux);
  sqlite3_mutex_leave(db->mutex);
  प्रत्यागम rc;
}


/*
** External API function used to create a new virtual-table module.
*/
पूर्णाङ्क sqlite3_create_module(
  sqlite3 *db,                    /* Database in which module is registered */
  ध्रुव वर्ण *zName,              /* Name assigned to this module */
  ध्रुव sqlite3_module *pModule,  /* The definition of the module */
  शून्य *pAux                      /* Context pointer for xCreate/xConnect */
){
#ifdef SQLITE_ENABLE_API_ARMOR
  जइ( !sqlite3SafetyCheckOk(db) || zName==0 ) प्रत्यागम SQLITE_MISUSE_BKPT;
#endif
  प्रत्यागम createModule(db, zName, pModule, pAux, 0);
}

/*
** External API function used to create a new virtual-table module.
*/
पूर्णाङ्क sqlite3_create_module_v2(
  sqlite3 *db,                    /* Database in which module is registered */
  ध्रुव वर्ण *zName,              /* Name assigned to this module */
  ध्रुव sqlite3_module *pModule,  /* The definition of the module */
  शून्य *pAux,                     /* Context pointer for xCreate/xConnect */
  शून्य (*xDestroy)(शून्य *)        /* Module destructor function */
){
#ifdef SQLITE_ENABLE_API_ARMOR
  जइ( !sqlite3SafetyCheckOk(db) || zName==0 ) प्रत्यागम SQLITE_MISUSE_BKPT;
#endif
  प्रत्यागम createModule(db, zName, pModule, pAux, xDestroy);
}

/*
** External API to drop all virtual-table modules, except those named
** on the azNames list.
*/
पूर्णाङ्क sqlite3_drop_modules(sqlite3 *db, ध्रुव वर्ण** azNames){
  HashElem *pThis, *pNext;
#ifdef SQLITE_ENABLE_API_ARMOR
  जइ( !sqlite3SafetyCheckOk(db) ) प्रत्यागम SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(db->mutex);
  प्रति(pThis=sqliteHashFirst(&db->aModule); pThis; pThis=pNext){
    Module *pMod = (Module*)sqliteHashData(pThis);
    pNext = sqliteHashNext(pThis);
    जइ( azNames ){
      पूर्णाङ्क ii;
      प्रति(ii=0; azNames[ii]!=0 && strcmp(azNames[ii],pMod->zName)!=0; ii++){}
      जइ( azNames[ii]!=0 ) अनुवर्त;
    }
    createModule(db, pMod->zName, 0, 0, 0);
  }
  sqlite3_mutex_leave(db->mutex);
  प्रत्यागम SQLITE_OK;
}

/*
** Decrement the reference count on a Module object.  Destroy the
** module when the reference count reaches zero.
*/
शून्य sqlite3VtabModuleUnref(sqlite3 *db, Module *pMod){
  प्रतिज्ञा( pMod->nRefModule>0 );
  pMod->nRefModule--;
  जइ( pMod->nRefModule==0 ){
    जइ( pMod->xDestroy ){
      pMod->xDestroy(pMod->pAux);
    }
    प्रतिज्ञा( pMod->pEpoTab==0 );
    sqlite3DbFree(db, pMod);
  }
}

/*
** Lock the virtual table so that it cannot be disconnected.
** Locks nest.  Every lock should have a corresponding unlock.
** If an unlock is omitted, resources leaks will occur.
**
** If a disconnect is attempted while a virtual table is locked,
** the disconnect is deferred until all locks have been removed.
*/
शून्य sqlite3VtabLock(VTable *pVTab){
  pVTab->nRef++;
}


/*
** pTab is a pointer to a Table structure representing a virtual-table.
** Return a pointer to the VTable object used by connection db to access
** this virtual-table, if one has been created, or NULL otherwise.
*/
VTable *sqlite3GetVTable(sqlite3 *db, Table *pTab){
  VTable *pVtab;
  प्रतिज्ञा( IsVirtual(pTab) );
  प्रति(pVtab=pTab->u.vtab.p; pVtab && pVtab->db!=db; pVtab=pVtab->pNext);
  प्रत्यागम pVtab;
}

/*
** Decrement the ref-count on a virtual table object. When the ref-count
** reaches zero, call the xDisconnect() method to delete the object.
*/
शून्य sqlite3VtabUnlock(VTable *pVTab){
  sqlite3 *db = pVTab->db;

  प्रतिज्ञा( db );
  प्रतिज्ञा( pVTab->nRef>0 );
  प्रतिज्ञा( db->eOpenState==SQLITE_STATE_OPEN
       || db->eOpenState==SQLITE_STATE_ZOMBIE );

  pVTab->nRef--;
  जइ( pVTab->nRef==0 ){
    sqlite3_vtab *p = pVTab->pVtab;
    जइ( p ){
      p->pModule->xDisconnect(p);
    }
    sqlite3VtabModuleUnref(pVTab->db, pVTab->pMod);
    sqlite3DbFree(db, pVTab);
  }
}

/*
** Table p is a virtual table. This function moves all elements in the
** p->u.vtab.p list to the sqlite3.pDisconnect lists of their associated
** database connections to be disconnected at the next opportunity.
** Except, if argument db is not NULL, then the entry associated with
** connection db is left in the p->u.vtab.p list.
*/
स्थिर VTable *vtabDisconnectAll(sqlite3 *db, Table *p){
  VTable *pRet = 0;
  VTable *pVTable;

  प्रतिज्ञा( IsVirtual(p) );
  pVTable = p->u.vtab.p;
  p->u.vtab.p = 0;

  /* Assert that the mutex (if any) associated with the BtShared database
  ** that contains table p is held by the caller. See header comments
  ** above function sqlite3VtabUnlockList() for an explanation of why
  ** this makes it safe to access the sqlite3.pDisconnect list of any
  ** database connection that may have an entry in the p->u.vtab.p list.
  */
  प्रतिज्ञा( db==0 || sqlite3SchemaMutexHeld(db, 0, p->pSchema) );

  जाव( pVTable ){
    sqlite3 *db2 = pVTable->db;
    VTable *pNext = pVTable->pNext;
    प्रतिज्ञा( db2 );
    जइ( db2==db ){
      pRet = pVTable;
      p->u.vtab.p = pRet;
      pRet->pNext = 0;
    }अण्णहा{
      pVTable->pNext = db2->pDisconnect;
      db2->pDisconnect = pVTable;
    }
    pVTable = pNext;
  }

  प्रतिज्ञा( !db || pRet );
  प्रत्यागम pRet;
}

/*
** Table *p is a virtual table. This function removes the VTable object
** for table *p associated with database connection db from the linked
** list in p->pVTab. It also decrements the VTable ref count. This is
** used when closing database connection db to free all of its VTable
** objects without disturbing the rest of the Schema object (which may
** be being used by other shared-cache connections).
*/
शून्य sqlite3VtabDisconnect(sqlite3 *db, Table *p){
  VTable **ppVTab;

  प्रतिज्ञा( IsVirtual(p) );
  प्रतिज्ञा( sqlite3BtreeHoldsAllMutexes(db) );
  प्रतिज्ञा( sqlite3_mutex_held(db->mutex) );

  प्रति(ppVTab=&p->u.vtab.p; *ppVTab; ppVTab=&(*ppVTab)->pNext){
    जइ( (*ppVTab)->db==db  ){
      VTable *pVTab = *ppVTab;
      *ppVTab = pVTab->pNext;
      sqlite3VtabUnlock(pVTab);
      भङ्ग;
    }
  }
}


/*
** Disconnect all the virtual table objects in the sqlite3.pDisconnect list.
**
** This function may only be called when the mutexes associated with all
** shared b-tree databases opened using connection db are held by the
** caller. This is done to protect the sqlite3.pDisconnect list. The
** sqlite3.pDisconnect list is accessed only as follows:
**
**   1) By this function. In this case, all BtShared mutexes and the mutex
**      associated with the database handle itself must be held.
**
**   2) By function vtabDisconnectAll(), when it adds a VTable entry to
**      the sqlite3.pDisconnect list. In this case either the BtShared mutex
**      associated with the database the virtual table is stored in is held
**      or, if the virtual table is stored in a non-sharable database, then
**      the database handle mutex is held.
**
** As a result, a sqlite3.pDisconnect cannot be accessed simultaneously
** by multiple threads. It is thread-safe.
*/
शून्य sqlite3VtabUnlockList(sqlite3 *db){
  VTable *p = db->pDisconnect;

  प्रतिज्ञा( sqlite3BtreeHoldsAllMutexes(db) );
  प्रतिज्ञा( sqlite3_mutex_held(db->mutex) );

  जइ( p ){
    db->pDisconnect = 0;
    करेहि {
      VTable *pNext = p->pNext;
      sqlite3VtabUnlock(p);
      p = pNext;
    }जाव( p );
  }
}

/*
** Clear any and all virtual-table information from the Table record.
** This routine is called, for example, just before deleting the Table
** record.
**
** Since it is a virtual-table, the Table structure contains a pointer
** to the head of a linked list of VTable structures. Each VTable
** structure is associated with a single sqlite3* user of the schema.
** The reference count of the VTable structure associated with database
** connection db is decremented immediately (which may lead to the
** structure being xDisconnected and free). Any other VTable structures
** in the list are moved to the sqlite3.pDisconnect list of the associated
** database connection.
*/
शून्य sqlite3VtabClear(sqlite3 *db, Table *p){
  प्रतिज्ञा( IsVirtual(p) );
  प्रतिज्ञा( db!=0 );
  जइ( db->pnBytesFreed==0 ) vtabDisconnectAll(0, p);
  जइ( p->u.vtab.azArg ){
    पूर्णाङ्क i;
    प्रति(i=0; i<p->u.vtab.nArg; i++){
      जइ( i!=1 ) sqlite3DbFree(db, p->u.vtab.azArg[i]);
    }
    sqlite3DbFree(db, p->u.vtab.azArg);
  }
}

/*
** Add a new module argument to pTable->u.vtab.azArg[].
** The string is not copied - the pointer is stored.  The
** string will be freed automatically when the table is
** deleted.
*/
स्थिर शून्य addModuleArgument(Parse *pParse, Table *pTable, वर्ण *zArg){
  sqlite3_int64 nBytes;
  वर्ण **azModuleArg;
  sqlite3 *db = pParse->db;

  प्रतिज्ञा( IsVirtual(pTable) );
  nBytes = परिमाण(वर्ण *)*(2+pTable->u.vtab.nArg);
  जइ( pTable->u.vtab.nArg+3>=db->aLimit[SQLITE_LIMIT_COLUMN] ){
    sqlite3_Error_सन्देशः(pParse, "too many columns on %s", pTable->zName);
  }
  azModuleArg = sqlite3DbRealloc(db, pTable->u.vtab.azArg, nBytes);
  जइ( azModuleArg==0 ){
    sqlite3DbFree(db, zArg);
  }अण्णहा{
    पूर्णाङ्क i = pTable->u.vtab.nArg++;
    azModuleArg[i] = zArg;
    azModuleArg[i+1] = 0;
    pTable->u.vtab.azArg = azModuleArg;
  }
}

/*
** The parser calls this routine when it first sees a CREATE VIRTUAL TABLE
** statement.  The module name has been parsed, but the optional list
** of parameters that follow the module name are still pending.
*/
शून्य sqlite3VtabBeginParse(
  Parse *pParse,        /* Parsing context */
  Token *pName1,        /* Name of new table, or database name */
  Token *pName2,        /* Name of new table or NULL */
  Token *pModuleName,   /* Name of the module for the virtual table */
  पूर्णाङ्क ifNotExists       /* No error if the table already exists */
){
  Table *pTable;        /* The new virtual table */
  sqlite3 *db;          /* Database connection */

  sqlite3StartTable(pParse, pName1, pName2, 0, 0, 1, ifNotExists);
  pTable = pParse->pNewTable;
  जइ( pTable==0 ) प्रत्यागम;
  प्रतिज्ञा( 0==pTable->pIndex );
  pTable->eTabType = TABTYP_VTAB;

  db = pParse->db;

  प्रतिज्ञा( pTable->u.vtab.nArg==0 );
  addModuleArgument(pParse, pTable, sqlite3NameFromToken(db, pModuleName));
  addModuleArgument(pParse, pTable, 0);
  addModuleArgument(pParse, pTable, sqlite3DbStrDup(db, pTable->zName));
  प्रतिज्ञा( (pParse->sNameToken.z==pName2->z && pName2->z!=0)
       || (pParse->sNameToken.z==pName1->z && pName2->z==0)
  );
  pParse->sNameToken.n = (पूर्णाङ्क)(
      &pModuleName->z[pModuleName->n] - pParse->sNameToken.z
  );

#ifndef SQLITE_OMIT_AUTHORIZATION
  /* Creating a virtual table invokes the authorization callback twice.
  ** The first invocation, to obtain permission to INSERT a row into the
  ** sqlite_schema table, has already been made by sqlite3StartTable().
  ** The second call, to obtain permission to create the table, is made now.
  */
  जइ( pTable->u.vtab.azArg ){
    पूर्णाङ्क iDb = sqlite3SchemaToIndex(db, pTable->pSchema);
    प्रतिज्ञा( iDb>=0 ); /* The database the table is being created in */
    sqlite3AuthCheck(pParse, SQLITE_CREATE_VTABLE, pTable->zName,
            pTable->u.vtab.azArg[0], pParse->db->aDb[iDb].zDbSName);
  }
#endif
}

/*
** This routine takes the module argument that has been accumulating
** in pParse->zArg[] and appends it to the list of arguments on the
** virtual table currently under construction in pParse->pTable.
*/
स्थिर शून्य addArgumentToVtab(Parse *pParse){
  जइ( pParse->sArg.z && pParse->pNewTable ){
    ध्रुव वर्ण *z = (ध्रुव वर्ण*)pParse->sArg.z;
    पूर्णाङ्क n = pParse->sArg.n;
    sqlite3 *db = pParse->db;
    addModuleArgument(pParse, pParse->pNewTable, sqlite3DbStrNDup(db, z, n));
  }
}

/*
** The parser calls this routine after the CREATE VIRTUAL TABLE statement
** has been completely parsed.
*/
शून्य sqlite3VtabFinishParse(Parse *pParse, Token *pEnd){
  Table *pTab = pParse->pNewTable;  /* The table being constructed */
  sqlite3 *db = pParse->db;         /* The database connection */

  जइ( pTab==0 ) प्रत्यागम;
  प्रतिज्ञा( IsVirtual(pTab) );
  addArgumentToVtab(pParse);
  pParse->sArg.z = 0;
  जइ( pTab->u.vtab.nArg<1 ) प्रत्यागम;

  /* If the CREATE VIRTUAL TABLE statement is being entered for the
  ** first time (in other words if the virtual table is actually being
  ** created now instead of just being read out of sqlite_schema) then
  ** do additional initialization work and store the statement text
  ** in the sqlite_schema table.
  */
  जइ( !db->init.busy ){
    वर्ण *zStmt;
    वर्ण *zWhere;
    पूर्णाङ्क iDb;
    पूर्णाङ्क iReg;
    Vdbe *v;

    sqlite3MayAbort(pParse);

    /* Compute the complete text of the CREATE VIRTUAL TABLE statement */
    जइ( pEnd ){
      pParse->sNameToken.n = (पूर्णाङ्क)(pEnd->z - pParse->sNameToken.z) + pEnd->n;
    }
    zStmt = sqlite3MPrintf(db, "CREATE VIRTUAL TABLE %T", &pParse->sNameToken);

    /* A slot for the record has already been allocated in the
    ** schema table.  We just need to update that slot with all
    ** the information we've collected.
    **
    ** The VM register number pParse->u1.cr.regRowid holds the rowid of an
    ** entry in the sqlite_schema table that was created for this vtab
    ** by sqlite3StartTable().
    */
    iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
    प्रतिज्ञा( pParse->isCreate );
    sqlite3NestedParse(pParse,
      "UPDATE %Q." LEGACY_SCHEMA_TABLE " "
         "SET type='table', name=%Q, tbl_name=%Q, rootpage=0, sql=%Q "
       "WHERE rowid=#%d",
      db->aDb[iDb].zDbSName,
      pTab->zName,
      pTab->zName,
      zStmt,
      pParse->u1.cr.regRowid
    );
    v = sqlite3GetVdbe(pParse);
    sqlite3ChangeCookie(pParse, iDb);

    sqlite3VdbeAddOp0(v, OP_Expire);
    zWhere = sqlite3MPrintf(db, "name=%Q AND sql=%Q", pTab->zName, zStmt);
    sqlite3VdbeAddParseSchemaOp(v, iDb, zWhere, 0);
    sqlite3DbFree(db, zStmt);

    iReg = ++pParse->nMem;
    sqlite3VdbeLoadString(v, iReg, pTab->zName);
    sqlite3VdbeAddOp2(v, OP_VCreate, iDb, iReg);
  }अण्णहा{
    /* If we are rereading the sqlite_schema table create the in-memory
    ** record of the table. */
    Table *pOld;
    Schema *pSchema = pTab->pSchema;
    ध्रुव वर्ण *zName = pTab->zName;
    प्रतिज्ञा( zName!=0 );
    sqlite3MarkAllShadowTablesOf(db, pTab);
    pOld = sqlite3_Hash_निवेशय(&pSchema->tblHash, zName, pTab);
    जइ( pOld ){
      sqlite3OomFault(db);
      प्रतिज्ञा( pTab==pOld );  /* Malloc must have failed inside HashInsert() */
      प्रत्यागम;
    }
    pParse->pNewTable = 0;
  }
}

/*
** The parser calls this routine when it sees the first token
** of an argument to the module name in a CREATE VIRTUAL TABLE statement.
*/
शून्य sqlite3VtabArgInit(Parse *pParse){
  addArgumentToVtab(pParse);
  pParse->sArg.z = 0;
  pParse->sArg.n = 0;
}

/*
** The parser calls this routine for each token after the first token
** in an argument to the module name in a CREATE VIRTUAL TABLE statement.
*/
शून्य sqlite3VtabArgExtend(Parse *pParse, Token *p){
  Token *pArg = &pParse->sArg;
  जइ( pArg->z==0 ){
    pArg->z = p->z;
    pArg->n = p->n;
  }अण्णहा{
    प्रतिज्ञा(pArg->z <= p->z);
    pArg->n = (पूर्णाङ्क)(&p->z[p->n] - pArg->z);
  }
}

/*
** Invoke a virtual table constructor (either xCreate or xConnect). The
** pointer to the function to invoke is passed as the fourth parameter
** to this procedure.
*/
स्थिर पूर्णाङ्क vtabCallConstructor(
  sqlite3 *db,
  Table *pTab,
  Module *pMod,
  पूर्णाङ्क (*xConstruct)(sqlite3*,शून्य*,पूर्णाङ्क,ध्रुव वर्ण*ध्रुव*,sqlite3_vtab**,वर्ण**),
  वर्ण **pzErr
){
  VtabCtx sCtx;
  VTable *pVTable;
  पूर्णाङ्क rc;
  ध्रुव वर्ण *ध्रुव*azArg;
  पूर्णाङ्क nArg = pTab->u.vtab.nArg;
  वर्ण *zErr = 0;
  वर्ण *zModuleName;
  पूर्णाङ्क iDb;
  VtabCtx *pCtx;

  प्रतिज्ञा( IsVirtual(pTab) );
  azArg = (ध्रुव वर्ण *ध्रुव*)pTab->u.vtab.azArg;

  /* Check that the virtual-table is not already being initialized */
  प्रति(pCtx=db->pVtabCtx; pCtx; pCtx=pCtx->pPrior){
    जइ( pCtx->pTab==pTab ){
      *pzErr = sqlite3MPrintf(db,
          "vtable constructor called recursively: %s", pTab->zName
      );
      प्रत्यागम SQLITE_LOCKED;
    }
  }

  zModuleName = sqlite3DbStrDup(db, pTab->zName);
  जइ( !zModuleName ){
    प्रत्यागम SQLITE_NOMEM_BKPT;
  }

  pVTable = sqlite3MallocZero(परिमाण(VTable));
  जइ( !pVTable ){
    sqlite3OomFault(db);
    sqlite3DbFree(db, zModuleName);
    प्रत्यागम SQLITE_NOMEM_BKPT;
  }
  pVTable->db = db;
  pVTable->pMod = pMod;
  pVTable->eVtabRisk = SQLITE_VTABRISK_Normal;

  iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
  pTab->u.vtab.azArg[1] = db->aDb[iDb].zDbSName;

  /* Invoke the virtual table constructor */
  प्रतिज्ञा( &db->pVtabCtx );
  प्रतिज्ञा( xConstruct );
  sCtx.pTab = pTab;
  sCtx.pVTable = pVTable;
  sCtx.pPrior = db->pVtabCtx;
  sCtx.bDeclared = 0;
  db->pVtabCtx = &sCtx;
  pTab->nTabRef++;
  rc = xConstruct(db, pMod->pAux, nArg, azArg, &pVTable->pVtab, &zErr);
  प्रतिज्ञा( pTab!=0 );
  प्रतिज्ञा( pTab->nTabRef>1 || rc!=SQLITE_OK );
  sqlite3DeleteTable(db, pTab);
  db->pVtabCtx = sCtx.pPrior;
  जइ( rc==SQLITE_NOMEM ) sqlite3OomFault(db);
  प्रतिज्ञा( sCtx.pTab==pTab );

  जइ( SQLITE_OK!=rc ){
    जइ( zErr==0 ){
      *pzErr = sqlite3MPrintf(db, "vtable constructor failed: %s", zModuleName);
    }अण्णहा {
      *pzErr = sqlite3MPrintf(db, "%s", zErr);
      sqlite3_free(zErr);
    }
    sqlite3DbFree(db, pVTable);
  }अण्णहा जइ( ALWAYS(pVTable->pVtab) ){
    /* Justification of ALWAYS():  A correct vtab constructor must allocate
    ** the sqlite3_vtab object if successful.  */
    memset(pVTable->pVtab, 0, परिमाण(pVTable->pVtab[0]));
    pVTable->pVtab->pModule = pMod->pModule;
    pMod->nRefModule++;
    pVTable->nRef = 1;
    जइ( sCtx.bDeclared==0 ){
      ध्रुव वर्ण *zFormat = "vtable constructor did not declare schema: %s";
      *pzErr = sqlite3MPrintf(db, zFormat, zModuleName);
      sqlite3VtabUnlock(pVTable);
      rc = SQLITE_ERROR;
    }अण्णहा{
      पूर्णाङ्क iCol;
      u16 oooHidden = 0;
      /* If everything went according to plan, link the new VTable structure
      ** into the linked list headed by pTab->u.vtab.p. Then loop through the
      ** columns of the table to see if any of them contain the token "hidden".
      ** If so, set the Column COLFLAG_HIDDEN flag and remove the token from
      ** the type string.  */
      pVTable->pNext = pTab->u.vtab.p;
      pTab->u.vtab.p = pVTable;

      प्रति(iCol=0; iCol<pTab->nCol; iCol++){
        वर्ण *zType = sqlite3ColumnType(&pTab->aCol[iCol], "");
        पूर्णाङ्क nType;
        पूर्णाङ्क i = 0;
        nType = sqlite3Strlen30(zType);
        प्रति(i=0; i<nType; i++){
          जइ( 0==sqlite3StrNICmp("hidden", &zType[i], 6)
           && (i==0 || zType[i-1]==' ')
           && (zType[i+6]=='\0' || zType[i+6]==' ')
          ){
            भङ्ग;
          }
        }
        जइ( i<nType ){
          पूर्णाङ्क j;
          पूर्णाङ्क nDel = 6 + (zType[i+6] ? 1 : 0);
          प्रति(j=i; (j+nDel)<=nType; j++){
            zType[j] = zType[j+nDel];
          }
          जइ( zType[i]=='\0' && i>0 ){
            प्रतिज्ञा(zType[i-1]==' ');
            zType[i-1] = '\0';
          }
          pTab->aCol[iCol].colFlags |= COLFLAG_HIDDEN;
          pTab->tabFlags |= TF_HasHidden;
          oooHidden = TF_OOOHidden;
        }अण्णहा{
          pTab->tabFlags |= oooHidden;
        }
      }
    }
  }

  sqlite3DbFree(db, zModuleName);
  प्रत्यागम rc;
}

/*
** This function is invoked by the parser to call the xConnect() method
** of the virtual table pTab. If an error occurs, an error code is returned
** and an error left in pParse.
**
** This call is a no-op if table pTab is not a virtual table.
*/
पूर्णाङ्क sqlite3VtabCallConnect(Parse *pParse, Table *pTab){
  sqlite3 *db = pParse->db;
  ध्रुव वर्ण *zMod;
  Module *pMod;
  पूर्णाङ्क rc;

  प्रतिज्ञा( pTab );
  प्रतिज्ञा( IsVirtual(pTab) );
  जइ( sqlite3GetVTable(db, pTab) ){
    प्रत्यागम SQLITE_OK;
  }

  /* Locate the required virtual table module */
  zMod = pTab->u.vtab.azArg[0];
  pMod = (Module*)sqlite3HashFind(&db->aModule, zMod);

  जइ( !pMod ){
    ध्रुव वर्ण *zModule = pTab->u.vtab.azArg[0];
    sqlite3_Error_सन्देशः(pParse, "no such module: %s", zModule);
    rc = SQLITE_ERROR;
  }अण्णहा{
    वर्ण *zErr = 0;
    rc = vtabCallConstructor(db, pTab, pMod, pMod->pModule->xConnect, &zErr);
    जइ( rc!=SQLITE_OK ){
      sqlite3_Error_सन्देशः(pParse, "%s", zErr);
      pParse->rc = rc;
    }अण्णहा{
      sqlite3MarkAllShadowTablesOf(db, pTab);
    }
    sqlite3DbFree(db, zErr);
  }

  प्रत्यागम rc;
}
/*
** Grow the db->aVTrans[] array so that there is room for at least one
** more v-table. Return SQLITE_NOMEM if a malloc fails, or SQLITE_OK otherwise.
*/
स्थिर पूर्णाङ्क growVTrans(sqlite3 *db){
  ध्रुव पूर्णाङ्क ARRAY_INCR = 5;

  /* Grow the sqlite3.aVTrans array if required */
  जइ( (db->nVTrans%ARRAY_INCR)==0 ){
    VTable **aVTrans;
    sqlite3_int64 nBytes = परिमाण(sqlite3_vtab*)*
                                 ((sqlite3_int64)db->nVTrans + ARRAY_INCR);
    aVTrans = sqlite3DbRealloc(db, (शून्य *)db->aVTrans, nBytes);
    जइ( !aVTrans ){
      प्रत्यागम SQLITE_NOMEM_BKPT;
    }
    memset(&aVTrans[db->nVTrans], 0, परिमाण(sqlite3_vtab *)*ARRAY_INCR);
    db->aVTrans = aVTrans;
  }

  प्रत्यागम SQLITE_OK;
}

/*
** Add the virtual table pVTab to the array sqlite3.aVTrans[]. Space should
** have already been reserved using growVTrans().
*/
स्थिर शून्य addToVTrans(sqlite3 *db, VTable *pVTab){
  /* Add pVtab to the end of sqlite3.aVTrans */
  db->aVTrans[db->nVTrans++] = pVTab;
  sqlite3VtabLock(pVTab);
}

/*
** This function is invoked by the vdbe to call the xCreate method
** of the virtual table named zTab in database iDb.
**
** If an error occurs, *pzErr is set to point to an English language
** description of the error and an SQLITE_XXX error code is returned.
** In this case the caller must call sqlite3DbFree(db, ) on *pzErr.
*/
पूर्णाङ्क sqlite3VtabCallCreate(sqlite3 *db, पूर्णाङ्क iDb, ध्रुव वर्ण *zTab, वर्ण **pzErr){
  पूर्णाङ्क rc = SQLITE_OK;
  Table *pTab;
  Module *pMod;
  ध्रुव वर्ण *zMod;

  pTab = sqlite3FindTable(db, zTab, db->aDb[iDb].zDbSName);
  प्रतिज्ञा( pTab && IsVirtual(pTab) && !pTab->u.vtab.p );

  /* Locate the required virtual table module */
  zMod = pTab->u.vtab.azArg[0];
  pMod = (Module*)sqlite3HashFind(&db->aModule, zMod);

  /* If the module has been registered and includes a Create method,
  ** invoke it now. If the module has not been registered, return an
  ** error. Otherwise, do nothing.
  */
  जइ( pMod==0 || pMod->pModule->xCreate==0 || pMod->pModule->xDestroy==0 ){
    *pzErr = sqlite3MPrintf(db, "no such module: %s", zMod);
    rc = SQLITE_ERROR;
  }अण्णहा{
    rc = vtabCallConstructor(db, pTab, pMod, pMod->pModule->xCreate, pzErr);
  }

  /* Justification of ALWAYS():  The xConstructor method is required to
  ** create a valid sqlite3_vtab if it returns SQLITE_OK. */
  जइ( rc==SQLITE_OK && ALWAYS(sqlite3GetVTable(db, pTab)) ){
    rc = growVTrans(db);
    जइ( rc==SQLITE_OK ){
      VTable *pVTab = sqlite3GetVTable(db, pTab);
      ध्रुव sqlite3_module *pModule = pMod->pModule;
      पूर्णाङ्क iSvpt = db->nStatement + db->nSavepoint;
      addToVTrans(db, pVTab);
      /* As in sqlite3VtabBegin(): open the current savepoint on the new
      ** table, so that a ROLLBACK TO that undoes the CREATE also invokes
      ** its xRollbackTo method. */
      जइ( iSvpt && pModule->iVersion>=2 && pModule->xSavepoint ){
        pVTab->iSavepoint = iSvpt;
        rc = pModule->xSavepoint(pVTab->pVtab, iSvpt-1);
      }
    }
  }

  प्रत्यागम rc;
}

/*
** This function is used to set the schema of a virtual table.  It is only
** valid to call this function from within the xCreate() or xConnect() of a
** virtual table module.
*/
पूर्णाङ्क sqlite3_declare_vtab(sqlite3 *db, ध्रुव वर्ण *zCreateTable){
  VtabCtx *pCtx;
  पूर्णाङ्क rc = SQLITE_OK;
  Table *pTab;
  Parse sParse;
  पूर्णाङ्क initBusy;
  पूर्णाङ्क i;
  ध्रुव unsigned वर्ण *z;
  स्थिर ध्रुव u8 aKeyword[] = { TK_CREATE, TK_TABLE, 0 };

#ifdef SQLITE_ENABLE_API_ARMOR
  जइ( !sqlite3SafetyCheckOk(db) || zCreateTable==0 ){
    प्रत्यागम SQLITE_MISUSE_BKPT;
  }
#endif

  /* Verify that the first two keywords in the CREATE TABLE statement
  ** really are "CREATE" and "TABLE".  If this is not the case, then
  ** sqlite3_declare_vtab() is being misused.
  */
  z = (ध्रुव unsigned वर्ण*)zCreateTable;
  प्रति(i=0; aKeyword[i]; i++){
    पूर्णाङ्क tokenType = 0;
    करेहि{
      z += sqlite3GetToken(z, &tokenType);
    }जाव( tokenType==TK_SPACE || tokenType==TK_COMMENT );
    जइ( tokenType!=aKeyword[i] ){
      sqlite3_Error_With_सन्देशः(db, SQLITE_ERROR, "syntax error");
      प्रत्यागम SQLITE_ERROR;
    }
  }
 
  sqlite3_mutex_enter(db->mutex);
  pCtx = db->pVtabCtx;
  जइ( !pCtx || pCtx->bDeclared ){
    sqlite3Error(db, SQLITE_MISUSE_BKPT);
    sqlite3_mutex_leave(db->mutex);
    प्रत्यागम SQLITE_MISUSE_BKPT;
  }

  pTab = pCtx->pTab;
  प्रतिज्ञा( IsVirtual(pTab) );

  sqlite3ParseObjectInit(&sParse, db);
  sParse.eParseMode = PARSE_MODE_DECLARE_VTAB;
  sParse.disableTriggers = 1;
  /* We should never be able to reach this point while loading the
  ** schema.  Nevertheless, defend against that (turn off db->init.busy)
  ** in case a bug arises. */
  प्रतिज्ञा( db->init.busy==0 );
  initBusy = db->init.busy;
  db->init.busy = 0;
  sParse.nQueryLoop = 1;
  जइ( SQLITE_OK==sqlite3RunParser(&sParse, zCreateTable) ){
    प्रतिज्ञा( sParse.pNewTable!=0 );
    प्रतिज्ञा( !db->mallocFailed );
    प्रतिज्ञा( IsOrdinaryTable(sParse.pNewTable) );
    प्रतिज्ञा( sParse.z_Err_सन्देशः==0 );
    जइ( !pTab->aCol ){
      Table *pNew = sParse.pNewTable;
      Index *pIdx;
      pTab->aCol = pNew->aCol;
      प्रतिज्ञा( IsOrdinaryTable(pNew) );
      sqlite3_व्यञ्जकम्_List_Delete(db, pNew->u.tab.pDfltList);
      pTab->nNVCol = pTab->nCol = pNew->nCol;
      pTab->tabFlags |= pNew->tabFlags & (TF_WithoutRowid|TF_NoVisibleRowid);
      pNew->nCol = 0;
      pNew->aCol = 0;
      प्रतिज्ञा( pTab->pIndex==0 );
      प्रतिज्ञा( HasRowid(pNew) || sqlite3_Primary_कुञ्चिका_Index(pNew)!=0 );
      जइ( !HasRowid(pNew)
       && pCtx->pVTable->pMod->pModule->xUpdate!=0
       && sqlite3_Primary_कुञ्चिका_Index(pNew)->n_कुञ्चिका_Col!=1
      ){
        /* WITHOUT ROWID virtual tables must either be read-only (xUpdate==0)
        ** or else must have a single-column PRIMARY KEY */
        rc = SQLITE_ERROR;
      }
      pIdx = pNew->pIndex;
      जइ( pIdx ){
        प्रतिज्ञा( pIdx->pNext==0 );
        pTab->pIndex = pIdx;
        pNew->pIndex = 0;
        pIdx->pTable = pTab;
      }
    }
    pCtx->bDeclared = 1;
  }अण्णहा{
    sqlite3_Error_With_सन्देशः(db, SQLITE_ERROR,
          (sParse.z_Err_सन्देशः ? "%s" : 0), sParse.z_Err_सन्देशः);
    sqlite3DbFree(db, sParse.z_Err_सन्देशः);
    rc = SQLITE_ERROR;
  }
  sParse.eParseMode = PARSE_MODE_NORMAL;

  जइ( sParse.pVdbe ){
    sqlite3VdbeFinalize(sParse.pVdbe);
  }
  sqlite3DeleteTable(db, sParse.pNewTable);
  sqlite3ParseObjectReset(&sParse);
  db->init.busy = initBusy;

  प्रतिज्ञा( (rc&0xff)==rc );
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  प्रत्यागम rc;
}

/*
** This function is invoked by the vdbe to call the xDestroy method
** of the virtual table named zTab in database iDb. This occurs
** when a DROP TABLE is mentioned.
**
** This call is a no-op if zTab is not a virtual table.
*/
पूर्णाङ्क sqlite3VtabCallDestroy(sqlite3 *db, पूर्णाङ्क iDb, ध्रुव वर्ण *zTab){
  पूर्णाङ्क rc = SQLITE_OK;
  Table *pTab;

  pTab = sqlite3FindTable(db, zTab, db->aDb[iDb].zDbSName);
  जइ( ALWAYS(pTab!=0)
   && ALWAYS(IsVirtual(pTab))
   && ALWAYS(pTab->u.vtab.p!=0)
  ){
    VTable *p;
    पूर्णाङ्क (*xDestroy)(sqlite3_vtab *);
    प्रति(p=pTab->u.vtab.p; p; p=p->pNext){
      प्रतिज्ञा( p->pVtab );
      जइ( p->pVtab->nRef>0 ){
        प्रत्यागम SQLITE_LOCKED;
      }
    }
    p = vtabDisconnectAll(db, pTab);
    xDestroy = p->pMod->pModule->xDestroy;
    जइ( xDestroy==0 ) xDestroy = p->pMod->pModule->xDisconnect;
    प्रतिज्ञा( xDestroy!=0 );
    pTab->nTabRef++;
    rc = xDestroy(p->pVtab);
    /* Remove the sqlite3_vtab* from the aVTrans[] array, if applicable */
    जइ( rc==SQLITE_OK ){
      प्रतिज्ञा( pTab->u.vtab.p==p && p->pNext==0 );
      p->pVtab = 0;
      pTab->u.vtab.p = 0;
      sqlite3VtabUnlock(p);
    }
    sqlite3DeleteTable(db, pTab);
  }

  प्रत्यागम rc;
}

/*
** This function invokes either the xRollback or xCommit method
** of each of the virtual tables in the sqlite3.aVTrans array. The method
** called is identified by the second argument, "offset", which is
** the offset of the method to call in the sqlite3_module structure.
**
** The array is cleared after invoking the callbacks.
*/
स्थिर शून्य callFinaliser(sqlite3 *db, पूर्णाङ्क offset){
  पूर्णाङ्क i;
  जइ( db->aVTrans ){
    VTable **aVTrans = db->aVTrans;
    db->aVTrans = 0;
    प्रति(i=0; i<db->nVTrans; i++){
      VTable *pVTab = aVTrans[i];
      sqlite3_vtab *p = pVTab->pVtab;
      जइ( p ){
        पूर्णाङ्क (*x)(sqlite3_vtab *);
        x = *(पूर्णाङ्क (**)(sqlite3_vtab *))((वर्ण *)p->pModule + offset);
        जइ( x ) x(p);
      }
      pVTab->iSavepoint = 0;
      sqlite3VtabUnlock(pVTab);
    }
    sqlite3DbFree(db, aVTrans);
    db->nVTrans = 0;
  }
}

/*
** Invoke the xSync method of all virtual tables in the sqlite3.aVTrans
** array. Return the error code for the first error that occurs, or
** SQLITE_OK if all xSync operations are successful.
**
** If an error message is available, leave it in p->zErrMsg.
*/
पूर्णाङ्क sqlite3VtabSync(sqlite3 *db, Vdbe *p){
  पूर्णाङ्क i;
  पूर्णाङ्क rc = SQLITE_OK;
  VTable **aVTrans = db->aVTrans;

  db->aVTrans = 0;
  प्रति(i=0; rc==SQLITE_OK && i<db->nVTrans; i++){
    पूर्णाङ्क (*x)(sqlite3_vtab *);
    sqlite3_vtab *pVtab = aVTrans[i]->pVtab;
    जइ( pVtab && (x = pVtab->pModule->xSync)!=0 ){
      rc = x(pVtab);
      sqlite3VtabImportErrmsg(p, pVtab);
    }
  }
  db->aVTrans = aVTrans;
  प्रत्यागम rc;
}

/*
** Invoke the xRollback method of all virtual tables in the
** sqlite3.aVTrans array. Then clear the array itself.
*/
पूर्णाङ्क sqlite3VtabRollback(sqlite3 *db){
  callFinaliser(db, offsetof(sqlite3_module,xRollback));
  प्रत्यागम SQLITE_OK;
}

/*
** Invoke the xCommit method of all virtual tables in the
** sqlite3.aVTrans array. Then clear the array itself.
*/
पूर्णाङ्क sqlite3VtabCommit(sqlite3 *db){
  callFinaliser(db, offsetof(sqlite3_module,xCommit));
  प्रत्यागम SQLITE_OK;
}

/*
** If the virtual table pVtab supports the transaction interface
** (xBegin/xRollback/xCommit and optionally xSync) and a transaction is
** not currently open, invoke the xBegin method now.
**
** If the xBegin call is successful, place the sqlite3_vtab pointer
** in the sqlite3.aVTrans array.
*/
पूर्णाङ्क sqlite3VtabBegin(sqlite3 *db, VTable *pVTab){
  पूर्णाङ्क rc = SQLITE_OK;
  ध्रुव sqlite3_module *pModule;

  /* Special case: If db->aVTrans is NULL and db->nVTrans is greater
  ** than zero, then this function is being called from within a
  ** virtual module xSync() callback. It is illegal to write to
  ** virtual module tables in this case, so return SQLITE_LOCKED.
  */
  जइ( sqlite3VtabInSync(db) ){
    प्रत्यागम SQLITE_LOCKED;
  }
  जइ( !pVTab ){
    प्रत्यागम SQLITE_OK;
  }
  pModule = pVTab->pVtab->pModule;

  जइ( pModule->xBegin ){
    पूर्णाङ्क i;

    /* If pVtab is already in the aVTrans array, return early */
    प्रति(i=0; i<db->nVTrans; i++){
      जइ( db->aVTrans[i]==pVTab ){
        प्रत्यागम SQLITE_OK;
      }
    }

    /* Invoke the xBegin method. If successful, add the vtab to the
    ** sqlite3.aVTrans[] array. */
    rc = growVTrans(db);
    जइ( rc==SQLITE_OK ){
      rc = pModule->xBegin(pVTab->pVtab);
      जइ( rc==SQLITE_OK ){
        पूर्णाङ्क iSvpt = db->nStatement + db->nSavepoint;
        addToVTrans(db, pVTab);
        जइ( iSvpt && pModule->xSavepoint ){
          pVTab->iSavepoint = iSvpt;
          rc = pModule->xSavepoint(pVTab->pVtab, iSvpt-1);
        }
      }
    }
  }
  प्रत्यागम rc;
}

/*
** Invoke either the xSavepoint, xRollbackTo or xRelease method of all
** virtual tables that currently have an open transaction. Pass iSavepoint
** as the second argument to the virtual table method invoked.
**
** If op is SAVEPOINT_BEGIN, the xSavepoint method is invoked. If it is
** SAVEPOINT_ROLLBACK, the xRollbackTo method. Otherwise, if op is
** SAVEPOINT_RELEASE, then the xRelease method of each virtual table with
** an open transaction is invoked.
**
** If any virtual table method returns an error code other than SQLITE_OK,
** processing is abandoned and the error returned to the caller of this
** function immediately. If all calls to virtual table methods are successful,
** SQLITE_OK is returned.
*/
पूर्णाङ्क sqlite3VtabSavepoint(sqlite3 *db, पूर्णाङ्क op, पूर्णाङ्क iSavepoint){
  पूर्णाङ्क rc = SQLITE_OK;

  प्रतिज्ञा( op==SAVEPOINT_RELEASE||op==SAVEPOINT_ROLLBACK||op==SAVEPOINT_BEGIN );
  प्रतिज्ञा( iSavepoint>=-1 );
  जइ( db->aVTrans ){
    पूर्णाङ्क i;
    प्रति(i=0; rc==SQLITE_OK && i<db->nVTrans; i++){
      VTable *pVTab = db->aVTrans[i];
      ध्रुव sqlite3_module *pMod = pVTab->pMod->pModule;
      जइ( pVTab->pVtab && pMod->iVersion>=2 ){
        पूर्णाङ्क (*xMethod)(sqlite3_vtab *, पूर्णाङ्क);
        sqlite3VtabLock(pVTab);
        विकल्प( op ){
          पक्ष SAVEPOINT_BEGIN:
            xMethod = pMod->xSavepoint;
            pVTab->iSavepoint = iSavepoint+1;
            भङ्ग;
          पक्ष SAVEPOINT_ROLLBACK:
            xMethod = pMod->xRollbackTo;
            भङ्ग;
          स्वभाव:
            xMethod = pMod->xRelease;
            भङ्ग;
        }
        जइ( xMethod && pVTab->iSavepoint>iSavepoint ){
          u64 savedFlags = (db->flags & SQLITE_Defensive);
          db->flags &= ~(u64)SQLITE_Defensive;
          rc = xMethod(pVTab->pVtab, iSavepoint);
          db->flags |= savedFlags;
        }
        sqlite3VtabUnlock(pVTab);
      }
    }
  }
  प्रत्यागम rc;
}

/*
** The first parameter (pDef) is a function implementation.  The
** second parameter (pExpr) is the first argument to this function.
** If pExpr is a column in a virtual table, then let the virtual
** table implementation have an opportunity to overload the function.
**
** This routine is used to allow virtual table implementations to
** overload MATCH, LIKE, GLOB, and REGEXP operators.
**
** Return either the pDef argument (indicating no change) or a
** new FuncDef structure that is marked as ephemeral using the
** SQLITE_FUNC_EPHEM flag.
*/
FuncDef *sqlite3VtabOverloadFunction(
  sqlite3 *db,    /* Database connection for reporting malloc problems */
  FuncDef *pDef,  /* Function to possibly overload */
  पूर्णाङ्क nArg,       /* Number of arguments to the function */
  व्यञ्जकम् *p_व्यञ्जकम्     /* First argument to the function */
){
  Table *pTab;
  sqlite3_vtab *pVtab;
  sqlite3_module *pMod;
  शून्य (*xSFunc)(sqlite3_context*,पूर्णाङ्क,sqlite3_मानम्**) = 0;
  शून्य *pArg = 0;
  FuncDef *pNew;
  पूर्णाङ्क rc = 0;

  /* Check to see the left operand is a column in a virtual table */
  जइ( NEVER(p_व्यञ्जकम्==0) ) प्रत्यागम pDef;
  जइ( p_व्यञ्जकम्->op!=TK_COLUMN ) प्रत्यागम pDef;
  प्रतिज्ञा( व्यञ्जकम्_Use_YTab(p_व्यञ्जकम्) );
  pTab = p_व्यञ्जकम्->y.pTab;
  जइ( NEVER(pTab==0) ) प्रत्यागम pDef;
  जइ( !IsVirtual(pTab) ) प्रत्यागम pDef;
  pVtab = sqlite3GetVTable(db, pTab)->pVtab;
  प्रतिज्ञा( pVtab!=0 );
  प्रतिज्ञा( pVtab->pModule!=0 );
  pMod = (sqlite3_module *)pVtab->pModule;
  जइ( pMod->xFindFunction==0 ) प्रत्यागम pDef;

  /* Call the xFindFunction method on the virtual table implementation
  ** to see if the implementation wants to overload this function.
  **
  ** Though undocumented, we have historically always invoked xFindFunction
  ** with an all lower-case function name.  Continue in this tradition to
  ** avoid any chance of an incompatibility.
  */
#ifdef SQLITE_DEBUG
  {
    पूर्णाङ्क i;
    प्रति(i=0; pDef->zName[i]; i++){
      unsigned वर्ण x = (unsigned वर्ण)pDef->zName[i];
      प्रतिज्ञा( x==sqlite3UpperToLower[x] );
    }
  }
#endif
  rc = pMod->xFindFunction(pVtab, nArg, pDef->zName, &xSFunc, &pArg);
  जइ( rc==0 ){
    प्रत्यागम pDef;
  }

  /* Create a new ephemeral function definition for the overloaded
  ** function */
  pNew = sqlite3DbMallocZero(db, परिमाण(*pNew)
                             + sqlite3Strlen30(pDef->zName) + 1);
  जइ( pNew==0 ){
    प्रत्यागम pDef;
  }
  *pNew = *pDef;
  pNew->zName = (ध्रुव वर्ण*)&pNew[1];
  memcpy((वर्ण*)&pNew[1], pDef->zName, sqlite3Strlen30(pDef->zName)+1);
  pNew->xSFunc = xSFunc;
  pNew->pUserData = pArg;
  pNew->funcFlags |= SQLITE_FUNC_EPHEM;
  प्रत्यागम pNew;
}

/*
** Make sure virtual table pTab is contained in the pParse->apVirtualLock[]
** array so that an OP_VBegin will get generated for it.  Add pTab to the
** array if it is missing.  If pTab is already in the array, this routine
** is a no-op.
*/
शून्य sqlite3VtabMakeWritable(Parse *pParse, Table *pTab){
  Parse *pToplevel = sqlite3ParseToplevel(pParse);
  पूर्णाङ्क i, n;
  Table **apVtabLock;

  प्रतिज्ञा( IsVirtual(pTab) );
  प्रति(i=0; i<pToplevel->nVtabLock; i++){
    जइ( pTab==pToplevel->apVtabLock[i] ) प्रत्यागम;
  }
  n = (pToplevel->nVtabLock+1)*परिमाण(pToplevel->apVtabLock[0]);
  apVtabLock = sqlite3Realloc(pToplevel->apVtabLock, n);
  जइ( apVtabLock ){
    pToplevel->apVtabLock = apVtabLock;
    pToplevel->apVtabLock[pToplevel->nVtabLock++] = pTab;
  }अण्णहा{
    sqlite3OomFault(pToplevel->db);
  }
}

/*
** Check to see if virtual table module pMod can be have an eponymous
** virtual table instance.  If it can, create one if one does not already
** exist. Return non-zero if either the eponymous virtual table instance
** exists when this routine returns or if an attempt to create it failed
** and an error message was left in pParse.
**
** An eponymous virtual table instance is one that is named after its
** module, and more importantly, does not require a CREATE VIRTUAL TABLE
** statement in order to come into existence.  Eponymous virtual table
** instances always exist.  They cannot be DROP-ed.
**
** Any virtual table module for which xConnect and xCreate are the same
** method can have an eponymous virtual table instance.
*/
पूर्णाङ्क sqlite3VtabEponymousTableInit(Parse *pParse, Module *pMod){
  ध्रुव sqlite3_module *pModule = pMod->pModule;
  Table *pTab;
  वर्ण *zErr = 0;
  पूर्णाङ्क rc;
  sqlite3 *db = pParse->db;
  जइ( pMod->pEpoTab ) प्रत्यागम 1;
  जइ( pModule->xCreate!=0 && pModule->xCreate!=pModule->xConnect ) प्रत्यागम 0;
  pTab = sqlite3DbMallocZero(db, परिमाण(Table));
  जइ( pTab==0 ) प्रत्यागम 0;
  pTab->zName = sqlite3DbStrDup(db, pMod->zName);
  जइ( pTab->zName==0 ){
    sqlite3DbFree(db, pTab);
    प्रत्यागम 0;
  }
  pMod->pEpoTab = pTab;
  pTab->nTabRef = 1;
  pTab->eTabType = TABTYP_VTAB;
  pTab->pSchema = db->aDb[0].pSchema;
  प्रतिज्ञा( pTab->u.vtab.nArg==0 );
  pTab->iPKey = -1;
  pTab->tabFlags |= TF_Eponymous;
  addModuleArgument(pParse, pTab, sqlite3DbStrDup(db, pTab->zName));
  addModuleArgument(pParse, pTab, 0);
  addModuleArgument(pParse, pTab, sqlite3DbStrDup(db, pTab->zName));
  db->nSchemaLock++;
  rc = vtabCallConstructor(db, pTab, pMod, pModule->xConnect, &zErr);
  db->nSchemaLock--;
  जइ( rc ){
    sqlite3_Error_सन्देशः(pParse, "%s", zErr);
    pParse->rc = rc;
    sqlite3DbFree(db, zErr);
    sqlite3VtabEponymousTableClear(db, pMod);
  }
  प्रत्यागम 1;
}

/*
** Erase the eponymous virtual table instance associated with
** virtual table module pMod, if it exists.
*/
शून्य sqlite3VtabEponymousTableClear(sqlite3 *db, Module *pMod){
  Table *pTab = pMod->pEpoTab;
  जइ( pTab!=0 ){
    /* Mark the table as Ephemeral prior to deleting it, so that the
    ** sqlite3DeleteTable() routine will know that it is not stored in
    ** the schema. */
    pTab->tabFlags |= TF_Ephemeral;
    sqlite3DeleteTable(db, pTab);
    pMod->pEpoTab = 0;
  }
}

/*
** Erase all eponymous virtual table instances associated with database
** handle db.  
*/
शून्य sqlite3VtabEponymousTableClearAll(sqlite3 *db){
  HashElem *pElem;
  प्रति(pElem=sqliteHashFirst(&db->aModule); pElem; pElem=sqliteHashNext(pElem)){
    sqlite3VtabEponymousTableClear(db, (Module*)sqliteHashData(pElem));
  }
}

/*
** Return the ON CONFLICT resolution mode in effect for the virtual
** table update operation currently in progress.
**
** The results of this routine are undefined unless it is called from
** within an xUpdate method.
*/
पूर्णाङ्क sqlite3_vtab_on_conflict(sqlite3 *db){
  स्थिर ध्रुव unsigned वर्ण aMap[] = {
    SQLITE_ROLLBACK, SQLITE_ABORT, SQLITE_FAIL, SQLITE_IGNORE, SQLITE_REPLACE
  };
#ifdef SQLITE_ENABLE_API_ARMOR
  जइ( !sqlite3SafetyCheckOk(db) ) प्रत्यागम SQLITE_MISUSE_BKPT;
#endif
  प्रतिज्ञा( OE_Rollback==1 && OE_Abort==2 && OE_Fail==3 );
  प्रतिज्ञा( OE_Ignore==4 && OE_Replace==5 );
  प्रतिज्ञा( db->vtabOnConflict>=1 && db->vtabOnConflict<=5 );
  प्रत्यागम (पूर्णाङ्क)aMap[db->vtabOnConflict-1];
}

/*
** Call from within the xCreate() or xConnect() methods to provide
** the SQLite core with additional information about the behavior
** of the virtual table being implemented.
*/
पूर्णाङ्क sqlite3_vtab_config(sqlite3 *db, पूर्णाङ्क op, ...){
  va_list ap;
  पूर्णाङ्क rc = SQLITE_OK;
  VtabCtx *p;

#ifdef SQLITE_ENABLE_API_ARMOR
  जइ( !sqlite3SafetyCheckOk(db) ) प्रत्यागम SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(db->mutex);
  p = db->pVtabCtx;
  जइ( !p ){
    rc = SQLITE_MISUSE_BKPT;
  }अण्णहा{
    प्रतिज्ञा( p->pTab==0 || IsVirtual(p->pTab) );
    va_start(ap, op);
    विकल्प( op ){
      पक्ष SQLITE_VTAB_CONSTRAINT_SUPPORT: {
        p->pVTable->bConstraint = (u8)va_arg(ap, पूर्णाङ्क);
        भङ्ग;
      }
      पक्ष SQLITE_VTAB_INNOCUOUS: {
        p->pVTable->eVtabRisk = SQLITE_VTABRISK_नीचः;
        भङ्ग;
      }
      पक्ष SQLITE_VTAB_DIRECTONLY: {
        p->pVTable->eVtabRisk = SQLITE_VTABRISK_उच्चः;
        भङ्ग;
      }
      पक्ष SQLITE_VTAB_USES_ALL_SCHEMAS: {
        p->pVTable->bAllSchemas = 1;
        भङ्ग;
      }
      स्वभाव: {
        rc = SQLITE_MISUSE_BKPT;
        भङ्ग;
      }
    }
    va_end(ap);
  }

  जइ( rc!=SQLITE_OK ) sqlite3Error(db, rc);
  sqlite3_mutex_leave(db->mutex);
  प्रत्यागम rc;
}

#endif /* SQLITE_OMIT_VIRTUALTABLE */
