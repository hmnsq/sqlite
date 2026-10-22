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
#ଅନ୍ତର୍ଭାଵ "sqliteInt.h"

/*
** Before a virtual table xCreate() or xConnect() method is invoked, the
** sqlite3.pVtabCtx member variable is set to point to an instance of
** this struct allocated on the stack. It is used by the implementation of
** the sqlite3_declare_vtab() and sqlite3_vtab_config() APIs, both of which
** are invoked only from within xCreate and xConnect methods.
*/
ସଂରଚନା VtabCtx {
  VTable *pVTable;    /* The virtual table being constructed */
  Table *pTab;        /* The Table object to which the virtual table belongs */
  VtabCtx *pPrior;    /* Parent context (if any) */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା bDeclared;      /* True after sqlite3_declare_vtab() is called */
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
  ଅଚଳ ଅକ୍ଷର *zName,              /* Name assigned to this module */
  ଅଚଳ sqlite3_module *pModule,  /* The definition of the module */
  ଶୂନ୍ୟ *pAux,                     /* Context pointer for xCreate/xConnect */
  ଶୂନ୍ୟ (*xDestroy)(ଶୂନ୍ୟ *)        /* Module destructor function */
){
  Module *pMod;
  Module *pDel;
  ଅକ୍ଷର *zCopy;
  ଯଦି( pModule==0 ){
    zCopy = (ଅକ୍ଷର*)zName;
    pMod = 0;
  }ଅନ୍ୟଥା{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା nName = sqlite3Strlen30(zName);
    pMod = (Module *)sqlite3Malloc(ଆକାର(Module) + nName + 1);
    ଯଦି( pMod==0 ){
      sqlite3OomFault(db);
      ଫେରନ୍ତୁ 0;
    }
    zCopy = (ଅକ୍ଷର *)(&pMod[1]);
    memcpy(zCopy, zName, nName+1);
    pMod->zName = zCopy;
    pMod->pModule = pModule;
    pMod->pAux = pAux;
    pMod->xDestroy = xDestroy;
    pMod->pEpoTab = 0;
    pMod->nRefModule = 1;
  }
  pDel = (Module *)sqlite3_Hash_ଭର୍ତ୍ତି(&db->aModule,zCopy,(ଶୂନ୍ୟ*)pMod);
  ଯଦି( pDel ){
    ଯଦି( pDel==pMod ){
      sqlite3OomFault(db);
      sqlite3DbFree(db, pDel);
      pMod = 0;
    }ଅନ୍ୟଥା{
      sqlite3VtabEponymousTableClear(db, pDel);
      sqlite3VtabModuleUnref(db, pDel);
    }
  }
  ଫେରନ୍ତୁ pMod;
}

/*
** The actual function that does the work of creating a new module.
** This function implements the sqlite3_create_module() and
** sqlite3_create_module_v2() interfaces.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା createModule(
  sqlite3 *db,                    /* Database in which module is registered */
  ଅଚଳ ଅକ୍ଷର *zName,              /* Name assigned to this module */
  ଅଚଳ sqlite3_module *pModule,  /* The definition of the module */
  ଶୂନ୍ୟ *pAux,                     /* Context pointer for xCreate/xConnect */
  ଶୂନ୍ୟ (*xDestroy)(ଶୂନ୍ୟ *)        /* Module destructor function */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;

  sqlite3_mutex_enter(db->mutex);
  (ଶୂନ୍ୟ)sqlite3VtabCreateModule(db, zName, pModule, pAux, xDestroy);
  rc = sqlite3ApiExit(db, rc);
  ଯଦି( rc!=SQLITE_OK && xDestroy ) xDestroy(pAux);
  sqlite3_mutex_leave(db->mutex);
  ଫେରନ୍ତୁ rc;
}


/*
** External API function used to create a new virtual-table module.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_create_module(
  sqlite3 *db,                    /* Database in which module is registered */
  ଅଚଳ ଅକ୍ଷର *zName,              /* Name assigned to this module */
  ଅଚଳ sqlite3_module *pModule,  /* The definition of the module */
  ଶୂନ୍ୟ *pAux                      /* Context pointer for xCreate/xConnect */
){
#ifdef SQLITE_ENABLE_API_ARMOR
  ଯଦି( !sqlite3SafetyCheckOk(db) || zName==0 ) ଫେରନ୍ତୁ SQLITE_MISUSE_BKPT;
#endif
  ଫେରନ୍ତୁ createModule(db, zName, pModule, pAux, 0);
}

/*
** External API function used to create a new virtual-table module.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_create_module_v2(
  sqlite3 *db,                    /* Database in which module is registered */
  ଅଚଳ ଅକ୍ଷର *zName,              /* Name assigned to this module */
  ଅଚଳ sqlite3_module *pModule,  /* The definition of the module */
  ଶୂନ୍ୟ *pAux,                     /* Context pointer for xCreate/xConnect */
  ଶୂନ୍ୟ (*xDestroy)(ଶୂନ୍ୟ *)        /* Module destructor function */
){
#ifdef SQLITE_ENABLE_API_ARMOR
  ଯଦି( !sqlite3SafetyCheckOk(db) || zName==0 ) ଫେରନ୍ତୁ SQLITE_MISUSE_BKPT;
#endif
  ଫେରନ୍ତୁ createModule(db, zName, pModule, pAux, xDestroy);
}

/*
** External API to drop all virtual-table modules, except those named
** on the azNames list.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_drop_modules(sqlite3 *db, ଅଚଳ ଅକ୍ଷର** azNames){
  HashElem *pThis, *pNext;
#ifdef SQLITE_ENABLE_API_ARMOR
  ଯଦି( !sqlite3SafetyCheckOk(db) ) ଫେରନ୍ତୁ SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(db->mutex);
  ପାଇଁ(pThis=sqliteHashFirst(&db->aModule); pThis; pThis=pNext){
    Module *pMod = (Module*)sqliteHashData(pThis);
    pNext = sqliteHashNext(pThis);
    ଯଦି( azNames ){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା ii;
      ପାଇଁ(ii=0; azNames[ii]!=0 && strcmp(azNames[ii],pMod->zName)!=0; ii++){}
      ଯଦି( azNames[ii]!=0 ) ଜାରି;
    }
    createModule(db, pMod->zName, 0, 0, 0);
  }
  sqlite3_mutex_leave(db->mutex);
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Decrement the reference count on a Module object.  Destroy the
** module when the reference count reaches zero.
*/
ଶୂନ୍ୟ sqlite3VtabModuleUnref(sqlite3 *db, Module *pMod){
  ପ୍ରତିଜ୍ଞା( pMod->nRefModule>0 );
  pMod->nRefModule--;
  ଯଦି( pMod->nRefModule==0 ){
    ଯଦି( pMod->xDestroy ){
      pMod->xDestroy(pMod->pAux);
    }
    ପ୍ରତିଜ୍ଞା( pMod->pEpoTab==0 );
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
ଶୂନ୍ୟ sqlite3VtabLock(VTable *pVTab){
  pVTab->nRef++;
}


/*
** pTab is a pointer to a Table structure representing a virtual-table.
** Return a pointer to the VTable object used by connection db to access
** this virtual-table, if one has been created, or NULL otherwise.
*/
VTable *sqlite3GetVTable(sqlite3 *db, Table *pTab){
  VTable *pVtab;
  ପ୍ରତିଜ୍ଞା( IsVirtual(pTab) );
  ପାଇଁ(pVtab=pTab->u.vtab.p; pVtab && pVtab->db!=db; pVtab=pVtab->pNext);
  ଫେରନ୍ତୁ pVtab;
}

/*
** Decrement the ref-count on a virtual table object. When the ref-count
** reaches zero, call the xDisconnect() method to delete the object.
*/
ଶୂନ୍ୟ sqlite3VtabUnlock(VTable *pVTab){
  sqlite3 *db = pVTab->db;

  ପ୍ରତିଜ୍ଞା( db );
  ପ୍ରତିଜ୍ଞା( pVTab->nRef>0 );
  ପ୍ରତିଜ୍ଞା( db->eOpenState==SQLITE_STATE_OPEN
       || db->eOpenState==SQLITE_STATE_ZOMBIE );

  pVTab->nRef--;
  ଯଦି( pVTab->nRef==0 ){
    sqlite3_vtab *p = pVTab->pVtab;
    ଯଦି( p ){
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
ସ୍ଥିର VTable *vtabDisconnectAll(sqlite3 *db, Table *p){
  VTable *pRet = 0;
  VTable *pVTable;

  ପ୍ରତିଜ୍ଞା( IsVirtual(p) );
  pVTable = p->u.vtab.p;
  p->u.vtab.p = 0;

  /* Assert that the mutex (if any) associated with the BtShared database
  ** that contains table p is held by the caller. See header comments
  ** above function sqlite3VtabUnlockList() for an explanation of why
  ** this makes it safe to access the sqlite3.pDisconnect list of any
  ** database connection that may have an entry in the p->u.vtab.p list.
  */
  ପ୍ରତିଜ୍ଞା( db==0 || sqlite3SchemaMutexHeld(db, 0, p->pSchema) );

  ଯେତେବେଳେ( pVTable ){
    sqlite3 *db2 = pVTable->db;
    VTable *pNext = pVTable->pNext;
    ପ୍ରତିଜ୍ଞା( db2 );
    ଯଦି( db2==db ){
      pRet = pVTable;
      p->u.vtab.p = pRet;
      pRet->pNext = 0;
    }ଅନ୍ୟଥା{
      pVTable->pNext = db2->pDisconnect;
      db2->pDisconnect = pVTable;
    }
    pVTable = pNext;
  }

  ପ୍ରତିଜ୍ଞା( !db || pRet );
  ଫେରନ୍ତୁ pRet;
}

/*
** Table *p is a virtual table. This function removes the VTable object
** for table *p associated with database connection db from the linked
** list in p->pVTab. It also decrements the VTable ref count. This is
** used when closing database connection db to free all of its VTable
** objects without disturbing the rest of the Schema object (which may
** be being used by other shared-cache connections).
*/
ଶୂନ୍ୟ sqlite3VtabDisconnect(sqlite3 *db, Table *p){
  VTable **ppVTab;

  ପ୍ରତିଜ୍ଞା( IsVirtual(p) );
  ପ୍ରତିଜ୍ଞା( sqlite3BtreeHoldsAllMutexes(db) );
  ପ୍ରତିଜ୍ଞା( sqlite3_mutex_held(db->mutex) );

  ପାଇଁ(ppVTab=&p->u.vtab.p; *ppVTab; ppVTab=&(*ppVTab)->pNext){
    ଯଦି( (*ppVTab)->db==db  ){
      VTable *pVTab = *ppVTab;
      *ppVTab = pVTab->pNext;
      sqlite3VtabUnlock(pVTab);
      ଭାଙ୍ଗ;
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
ଶୂନ୍ୟ sqlite3VtabUnlockList(sqlite3 *db){
  VTable *p = db->pDisconnect;

  ପ୍ରତିଜ୍ଞା( sqlite3BtreeHoldsAllMutexes(db) );
  ପ୍ରତିଜ୍ଞା( sqlite3_mutex_held(db->mutex) );

  ଯଦି( p ){
    db->pDisconnect = 0;
    କର {
      VTable *pNext = p->pNext;
      sqlite3VtabUnlock(p);
      p = pNext;
    }ଯେତେବେଳେ( p );
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
ଶୂନ୍ୟ sqlite3VtabClear(sqlite3 *db, Table *p){
  ପ୍ରତିଜ୍ଞା( IsVirtual(p) );
  ପ୍ରତିଜ୍ଞା( db!=0 );
  ଯଦି( db->pnBytesFreed==0 ) vtabDisconnectAll(0, p);
  ଯଦି( p->u.vtab.azArg ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ(i=0; i<p->u.vtab.nArg; i++){
      ଯଦି( i!=1 ) sqlite3DbFree(db, p->u.vtab.azArg[i]);
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
ସ୍ଥିର ଶୂନ୍ୟ addModuleArgument(Parse *pParse, Table *pTable, ଅକ୍ଷର *zArg){
  sqlite3_int64 nBytes;
  ଅକ୍ଷର **azModuleArg;
  sqlite3 *db = pParse->db;

  ପ୍ରତିଜ୍ଞା( IsVirtual(pTable) );
  nBytes = ଆକାର(ଅକ୍ଷର *)*(2+pTable->u.vtab.nArg);
  ଯଦି( pTable->u.vtab.nArg+3>=db->aLimit[SQLITE_LIMIT_COLUMN] ){
    sqlite3_Error_ସନ୍ଦେଶ(pParse, "too many columns on %s", pTable->zName);
  }
  azModuleArg = sqlite3DbRealloc(db, pTable->u.vtab.azArg, nBytes);
  ଯଦି( azModuleArg==0 ){
    sqlite3DbFree(db, zArg);
  }ଅନ୍ୟଥା{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = pTable->u.vtab.nArg++;
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
ଶୂନ୍ୟ sqlite3VtabBeginParse(
  Parse *pParse,        /* Parsing context */
  Token *pName1,        /* Name of new table, or database name */
  Token *pName2,        /* Name of new table or NULL */
  Token *pModuleName,   /* Name of the module for the virtual table */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ifNotExists       /* No error if the table already exists */
){
  Table *pTable;        /* The new virtual table */
  sqlite3 *db;          /* Database connection */

  sqlite3StartTable(pParse, pName1, pName2, 0, 0, 1, ifNotExists);
  pTable = pParse->pNewTable;
  ଯଦି( pTable==0 ) ଫେରନ୍ତୁ;
  ପ୍ରତିଜ୍ଞା( 0==pTable->pIndex );
  pTable->eTabType = TABTYP_VTAB;

  db = pParse->db;

  ପ୍ରତିଜ୍ଞା( pTable->u.vtab.nArg==0 );
  addModuleArgument(pParse, pTable, sqlite3NameFromToken(db, pModuleName));
  addModuleArgument(pParse, pTable, 0);
  addModuleArgument(pParse, pTable, sqlite3DbStrDup(db, pTable->zName));
  ପ୍ରତିଜ୍ଞା( (pParse->sNameToken.z==pName2->z && pName2->z!=0)
       || (pParse->sNameToken.z==pName1->z && pName2->z==0)
  );
  pParse->sNameToken.n = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(
      &pModuleName->z[pModuleName->n] - pParse->sNameToken.z
  );

#ifndef SQLITE_OMIT_AUTHORIZATION
  /* Creating a virtual table invokes the authorization callback twice.
  ** The first invocation, to obtain permission to INSERT a row into the
  ** sqlite_schema table, has already been made by sqlite3StartTable().
  ** The second call, to obtain permission to create the table, is made now.
  */
  ଯଦି( pTable->u.vtab.azArg ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDb = sqlite3SchemaToIndex(db, pTable->pSchema);
    ପ୍ରତିଜ୍ଞା( iDb>=0 ); /* The database the table is being created in */
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
ସ୍ଥିର ଶୂନ୍ୟ addArgumentToVtab(Parse *pParse){
  ଯଦି( pParse->sArg.z && pParse->pNewTable ){
    ଅଚଳ ଅକ୍ଷର *z = (ଅଚଳ ଅକ୍ଷର*)pParse->sArg.z;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = pParse->sArg.n;
    sqlite3 *db = pParse->db;
    addModuleArgument(pParse, pParse->pNewTable, sqlite3DbStrNDup(db, z, n));
  }
}

/*
** The parser calls this routine after the CREATE VIRTUAL TABLE statement
** has been completely parsed.
*/
ଶୂନ୍ୟ sqlite3VtabFinishParse(Parse *pParse, Token *pEnd){
  Table *pTab = pParse->pNewTable;  /* The table being constructed */
  sqlite3 *db = pParse->db;         /* The database connection */

  ଯଦି( pTab==0 ) ଫେରନ୍ତୁ;
  ପ୍ରତିଜ୍ଞା( IsVirtual(pTab) );
  addArgumentToVtab(pParse);
  pParse->sArg.z = 0;
  ଯଦି( pTab->u.vtab.nArg<1 ) ଫେରନ୍ତୁ;

  /* If the CREATE VIRTUAL TABLE statement is being entered for the
  ** first time (in other words if the virtual table is actually being
  ** created now instead of just being read out of sqlite_schema) then
  ** do additional initialization work and store the statement text
  ** in the sqlite_schema table.
  */
  ଯଦି( !db->init.busy ){
    ଅକ୍ଷର *zStmt;
    ଅକ୍ଷର *zWhere;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDb;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା iReg;
    Vdbe *v;

    sqlite3MayAbort(pParse);

    /* Compute the complete text of the CREATE VIRTUAL TABLE statement */
    ଯଦି( pEnd ){
      pParse->sNameToken.n = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(pEnd->z - pParse->sNameToken.z) + pEnd->n;
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
    ପ୍ରତିଜ୍ଞା( pParse->isCreate );
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
  }ଅନ୍ୟଥା{
    /* If we are rereading the sqlite_schema table create the in-memory
    ** record of the table. */
    Table *pOld;
    Schema *pSchema = pTab->pSchema;
    ଅଚଳ ଅକ୍ଷର *zName = pTab->zName;
    ପ୍ରତିଜ୍ଞା( zName!=0 );
    sqlite3MarkAllShadowTablesOf(db, pTab);
    pOld = sqlite3_Hash_ଭର୍ତ୍ତି(&pSchema->tblHash, zName, pTab);
    ଯଦି( pOld ){
      sqlite3OomFault(db);
      ପ୍ରତିଜ୍ଞା( pTab==pOld );  /* Malloc must have failed inside HashInsert() */
      ଫେରନ୍ତୁ;
    }
    pParse->pNewTable = 0;
  }
}

/*
** The parser calls this routine when it sees the first token
** of an argument to the module name in a CREATE VIRTUAL TABLE statement.
*/
ଶୂନ୍ୟ sqlite3VtabArgInit(Parse *pParse){
  addArgumentToVtab(pParse);
  pParse->sArg.z = 0;
  pParse->sArg.n = 0;
}

/*
** The parser calls this routine for each token after the first token
** in an argument to the module name in a CREATE VIRTUAL TABLE statement.
*/
ଶୂନ୍ୟ sqlite3VtabArgExtend(Parse *pParse, Token *p){
  Token *pArg = &pParse->sArg;
  ଯଦି( pArg->z==0 ){
    pArg->z = p->z;
    pArg->n = p->n;
  }ଅନ୍ୟଥା{
    ପ୍ରତିଜ୍ଞା(pArg->z <= p->z);
    pArg->n = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(&p->z[p->n] - pArg->z);
  }
}

/*
** Invoke a virtual table constructor (either xCreate or xConnect). The
** pointer to the function to invoke is passed as the fourth parameter
** to this procedure.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vtabCallConstructor(
  sqlite3 *db,
  Table *pTab,
  Module *pMod,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*xConstruct)(sqlite3*,ଶୂନ୍ୟ*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ଅଚଳ ଅକ୍ଷର*ଅଚଳ*,sqlite3_vtab**,ଅକ୍ଷର**),
  ଅକ୍ଷର **pzErr
){
  VtabCtx sCtx;
  VTable *pVTable;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ଅଚଳ ଅକ୍ଷର *ଅଚଳ*azArg;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nArg = pTab->u.vtab.nArg;
  ଅକ୍ଷର *zErr = 0;
  ଅକ୍ଷର *zModuleName;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDb;
  VtabCtx *pCtx;

  ପ୍ରତିଜ୍ଞା( IsVirtual(pTab) );
  azArg = (ଅଚଳ ଅକ୍ଷର *ଅଚଳ*)pTab->u.vtab.azArg;

  /* Check that the virtual-table is not already being initialized */
  ପାଇଁ(pCtx=db->pVtabCtx; pCtx; pCtx=pCtx->pPrior){
    ଯଦି( pCtx->pTab==pTab ){
      *pzErr = sqlite3MPrintf(db,
          "vtable constructor called recursively: %s", pTab->zName
      );
      ଫେରନ୍ତୁ SQLITE_LOCKED;
    }
  }

  zModuleName = sqlite3DbStrDup(db, pTab->zName);
  ଯଦି( !zModuleName ){
    ଫେରନ୍ତୁ SQLITE_NOMEM_BKPT;
  }

  pVTable = sqlite3MallocZero(ଆକାର(VTable));
  ଯଦି( !pVTable ){
    sqlite3OomFault(db);
    sqlite3DbFree(db, zModuleName);
    ଫେରନ୍ତୁ SQLITE_NOMEM_BKPT;
  }
  pVTable->db = db;
  pVTable->pMod = pMod;
  pVTable->eVtabRisk = SQLITE_VTABRISK_Normal;

  iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
  pTab->u.vtab.azArg[1] = db->aDb[iDb].zDbSName;

  /* Invoke the virtual table constructor */
  ପ୍ରତିଜ୍ଞା( &db->pVtabCtx );
  ପ୍ରତିଜ୍ଞା( xConstruct );
  sCtx.pTab = pTab;
  sCtx.pVTable = pVTable;
  sCtx.pPrior = db->pVtabCtx;
  sCtx.bDeclared = 0;
  db->pVtabCtx = &sCtx;
  pTab->nTabRef++;
  rc = xConstruct(db, pMod->pAux, nArg, azArg, &pVTable->pVtab, &zErr);
  ପ୍ରତିଜ୍ଞା( pTab!=0 );
  ପ୍ରତିଜ୍ଞା( pTab->nTabRef>1 || rc!=SQLITE_OK );
  sqlite3DeleteTable(db, pTab);
  db->pVtabCtx = sCtx.pPrior;
  ଯଦି( rc==SQLITE_NOMEM ) sqlite3OomFault(db);
  ପ୍ରତିଜ୍ଞା( sCtx.pTab==pTab );

  ଯଦି( SQLITE_OK!=rc ){
    ଯଦି( zErr==0 ){
      *pzErr = sqlite3MPrintf(db, "vtable constructor failed: %s", zModuleName);
    }ଅନ୍ୟଥା {
      *pzErr = sqlite3MPrintf(db, "%s", zErr);
      sqlite3_free(zErr);
    }
    sqlite3DbFree(db, pVTable);
  }ଅନ୍ୟଥା ଯଦି( ALWAYS(pVTable->pVtab) ){
    /* Justification of ALWAYS():  A correct vtab constructor must allocate
    ** the sqlite3_vtab object if successful.  */
    memset(pVTable->pVtab, 0, ଆକାର(pVTable->pVtab[0]));
    pVTable->pVtab->pModule = pMod->pModule;
    pMod->nRefModule++;
    pVTable->nRef = 1;
    ଯଦି( sCtx.bDeclared==0 ){
      ଅଚଳ ଅକ୍ଷର *zFormat = "vtable constructor did not declare schema: %s";
      *pzErr = sqlite3MPrintf(db, zFormat, zModuleName);
      sqlite3VtabUnlock(pVTable);
      rc = SQLITE_ERROR;
    }ଅନ୍ୟଥା{
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCol;
      u16 oooHidden = 0;
      /* If everything went according to plan, link the new VTable structure
      ** into the linked list headed by pTab->u.vtab.p. Then loop through the
      ** columns of the table to see if any of them contain the token "hidden".
      ** If so, set the Column COLFLAG_HIDDEN flag and remove the token from
      ** the type string.  */
      pVTable->pNext = pTab->u.vtab.p;
      pTab->u.vtab.p = pVTable;

      ପାଇଁ(iCol=0; iCol<pTab->nCol; iCol++){
        ଅକ୍ଷର *zType = sqlite3ColumnType(&pTab->aCol[iCol], "");
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା nType;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = 0;
        nType = sqlite3Strlen30(zType);
        ପାଇଁ(i=0; i<nType; i++){
          ଯଦି( 0==sqlite3StrNICmp("hidden", &zType[i], 6)
           && (i==0 || zType[i-1]==' ')
           && (zType[i+6]=='\0' || zType[i+6]==' ')
          ){
            ଭାଙ୍ଗ;
          }
        }
        ଯଦି( i<nType ){
          ପୂର୍ଣ୍ଣସଂଖ୍ୟା j;
          ପୂର୍ଣ୍ଣସଂଖ୍ୟା nDel = 6 + (zType[i+6] ? 1 : 0);
          ପାଇଁ(j=i; (j+nDel)<=nType; j++){
            zType[j] = zType[j+nDel];
          }
          ଯଦି( zType[i]=='\0' && i>0 ){
            ପ୍ରତିଜ୍ଞା(zType[i-1]==' ');
            zType[i-1] = '\0';
          }
          pTab->aCol[iCol].colFlags |= COLFLAG_HIDDEN;
          pTab->tabFlags |= TF_HasHidden;
          oooHidden = TF_OOOHidden;
        }ଅନ୍ୟଥା{
          pTab->tabFlags |= oooHidden;
        }
      }
    }
  }

  sqlite3DbFree(db, zModuleName);
  ଫେରନ୍ତୁ rc;
}

/*
** This function is invoked by the parser to call the xConnect() method
** of the virtual table pTab. If an error occurs, an error code is returned
** and an error left in pParse.
**
** This call is a no-op if table pTab is not a virtual table.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3VtabCallConnect(Parse *pParse, Table *pTab){
  sqlite3 *db = pParse->db;
  ଅଚଳ ଅକ୍ଷର *zMod;
  Module *pMod;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

  ପ୍ରତିଜ୍ଞା( pTab );
  ପ୍ରତିଜ୍ଞା( IsVirtual(pTab) );
  ଯଦି( sqlite3GetVTable(db, pTab) ){
    ଫେରନ୍ତୁ SQLITE_OK;
  }

  /* Locate the required virtual table module */
  zMod = pTab->u.vtab.azArg[0];
  pMod = (Module*)sqlite3HashFind(&db->aModule, zMod);

  ଯଦି( !pMod ){
    ଅଚଳ ଅକ୍ଷର *zModule = pTab->u.vtab.azArg[0];
    sqlite3_Error_ସନ୍ଦେଶ(pParse, "no such module: %s", zModule);
    rc = SQLITE_ERROR;
  }ଅନ୍ୟଥା{
    ଅକ୍ଷର *zErr = 0;
    rc = vtabCallConstructor(db, pTab, pMod, pMod->pModule->xConnect, &zErr);
    ଯଦି( rc!=SQLITE_OK ){
      sqlite3_Error_ସନ୍ଦେଶ(pParse, "%s", zErr);
      pParse->rc = rc;
    }ଅନ୍ୟଥା{
      sqlite3MarkAllShadowTablesOf(db, pTab);
    }
    sqlite3DbFree(db, zErr);
  }

  ଫେରନ୍ତୁ rc;
}
/*
** Grow the db->aVTrans[] array so that there is room for at least one
** more v-table. Return SQLITE_NOMEM if a malloc fails, or SQLITE_OK otherwise.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା growVTrans(sqlite3 *db){
  ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା ARRAY_INCR = 5;

  /* Grow the sqlite3.aVTrans array if required */
  ଯଦି( (db->nVTrans%ARRAY_INCR)==0 ){
    VTable **aVTrans;
    sqlite3_int64 nBytes = ଆକାର(sqlite3_vtab*)*
                                 ((sqlite3_int64)db->nVTrans + ARRAY_INCR);
    aVTrans = sqlite3DbRealloc(db, (ଶୂନ୍ୟ *)db->aVTrans, nBytes);
    ଯଦି( !aVTrans ){
      ଫେରନ୍ତୁ SQLITE_NOMEM_BKPT;
    }
    memset(&aVTrans[db->nVTrans], 0, ଆକାର(sqlite3_vtab *)*ARRAY_INCR);
    db->aVTrans = aVTrans;
  }

  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Add the virtual table pVTab to the array sqlite3.aVTrans[]. Space should
** have already been reserved using growVTrans().
*/
ସ୍ଥିର ଶୂନ୍ୟ addToVTrans(sqlite3 *db, VTable *pVTab){
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3VtabCallCreate(sqlite3 *db, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDb, ଅଚଳ ଅକ୍ଷର *zTab, ଅକ୍ଷର **pzErr){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
  Table *pTab;
  Module *pMod;
  ଅଚଳ ଅକ୍ଷର *zMod;

  pTab = sqlite3FindTable(db, zTab, db->aDb[iDb].zDbSName);
  ପ୍ରତିଜ୍ଞା( pTab && IsVirtual(pTab) && !pTab->u.vtab.p );

  /* Locate the required virtual table module */
  zMod = pTab->u.vtab.azArg[0];
  pMod = (Module*)sqlite3HashFind(&db->aModule, zMod);

  /* If the module has been registered and includes a Create method,
  ** invoke it now. If the module has not been registered, return an
  ** error. Otherwise, do nothing.
  */
  ଯଦି( pMod==0 || pMod->pModule->xCreate==0 || pMod->pModule->xDestroy==0 ){
    *pzErr = sqlite3MPrintf(db, "no such module: %s", zMod);
    rc = SQLITE_ERROR;
  }ଅନ୍ୟଥା{
    rc = vtabCallConstructor(db, pTab, pMod, pMod->pModule->xCreate, pzErr);
  }

  /* Justification of ALWAYS():  The xConstructor method is required to
  ** create a valid sqlite3_vtab if it returns SQLITE_OK. */
  ଯଦି( rc==SQLITE_OK && ALWAYS(sqlite3GetVTable(db, pTab)) ){
    rc = growVTrans(db);
    ଯଦି( rc==SQLITE_OK ){
      VTable *pVTab = sqlite3GetVTable(db, pTab);
      ଅଚଳ sqlite3_module *pModule = pMod->pModule;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା iSvpt = db->nStatement + db->nSavepoint;
      addToVTrans(db, pVTab);
      /* As in sqlite3VtabBegin(): open the current savepoint on the new
      ** table, so that a ROLLBACK TO that undoes the CREATE also invokes
      ** its xRollbackTo method. */
      ଯଦି( iSvpt && pModule->iVersion>=2 && pModule->xSavepoint ){
        pVTab->iSavepoint = iSvpt;
        rc = pModule->xSavepoint(pVTab->pVtab, iSvpt-1);
      }
    }
  }

  ଫେରନ୍ତୁ rc;
}

/*
** This function is used to set the schema of a virtual table.  It is only
** valid to call this function from within the xCreate() or xConnect() of a
** virtual table module.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_declare_vtab(sqlite3 *db, ଅଚଳ ଅକ୍ଷର *zCreateTable){
  VtabCtx *pCtx;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
  Table *pTab;
  Parse sParse;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା initBusy;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ଅଚଳ unsigned ଅକ୍ଷର *z;
  ସ୍ଥିର ଅଚଳ u8 aKeyword[] = { TK_CREATE, TK_TABLE, 0 };

#ifdef SQLITE_ENABLE_API_ARMOR
  ଯଦି( !sqlite3SafetyCheckOk(db) || zCreateTable==0 ){
    ଫେରନ୍ତୁ SQLITE_MISUSE_BKPT;
  }
#endif

  /* Verify that the first two keywords in the CREATE TABLE statement
  ** really are "CREATE" and "TABLE".  If this is not the case, then
  ** sqlite3_declare_vtab() is being misused.
  */
  z = (ଅଚଳ unsigned ଅକ୍ଷର*)zCreateTable;
  ପାଇଁ(i=0; aKeyword[i]; i++){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା tokenType = 0;
    କର{
      z += sqlite3GetToken(z, &tokenType);
    }ଯେତେବେଳେ( tokenType==TK_SPACE || tokenType==TK_COMMENT );
    ଯଦି( tokenType!=aKeyword[i] ){
      sqlite3_Error_With_ସନ୍ଦେଶ(db, SQLITE_ERROR, "syntax error");
      ଫେରନ୍ତୁ SQLITE_ERROR;
    }
  }
 
  sqlite3_mutex_enter(db->mutex);
  pCtx = db->pVtabCtx;
  ଯଦି( !pCtx || pCtx->bDeclared ){
    sqlite3Error(db, SQLITE_MISUSE_BKPT);
    sqlite3_mutex_leave(db->mutex);
    ଫେରନ୍ତୁ SQLITE_MISUSE_BKPT;
  }

  pTab = pCtx->pTab;
  ପ୍ରତିଜ୍ଞା( IsVirtual(pTab) );

  sqlite3ParseObjectInit(&sParse, db);
  sParse.eParseMode = PARSE_MODE_DECLARE_VTAB;
  sParse.disableTriggers = 1;
  /* We should never be able to reach this point while loading the
  ** schema.  Nevertheless, defend against that (turn off db->init.busy)
  ** in case a bug arises. */
  ପ୍ରତିଜ୍ଞା( db->init.busy==0 );
  initBusy = db->init.busy;
  db->init.busy = 0;
  sParse.nQueryLoop = 1;
  ଯଦି( SQLITE_OK==sqlite3RunParser(&sParse, zCreateTable) ){
    ପ୍ରତିଜ୍ଞା( sParse.pNewTable!=0 );
    ପ୍ରତିଜ୍ଞା( !db->mallocFailed );
    ପ୍ରତିଜ୍ଞା( IsOrdinaryTable(sParse.pNewTable) );
    ପ୍ରତିଜ୍ଞା( sParse.z_Err_ସନ୍ଦେଶ==0 );
    ଯଦି( !pTab->aCol ){
      Table *pNew = sParse.pNewTable;
      Index *pIdx;
      pTab->aCol = pNew->aCol;
      ପ୍ରତିଜ୍ଞା( IsOrdinaryTable(pNew) );
      sqlite3_ପଦସମଷ୍ଟି_List_Delete(db, pNew->u.tab.pDfltList);
      pTab->nNVCol = pTab->nCol = pNew->nCol;
      pTab->tabFlags |= pNew->tabFlags & (TF_WithoutRowid|TF_NoVisibleRowid);
      pNew->nCol = 0;
      pNew->aCol = 0;
      ପ୍ରତିଜ୍ଞା( pTab->pIndex==0 );
      ପ୍ରତିଜ୍ଞା( HasRowid(pNew) || sqlite3_Primary_ଚାବି_Index(pNew)!=0 );
      ଯଦି( !HasRowid(pNew)
       && pCtx->pVTable->pMod->pModule->xUpdate!=0
       && sqlite3_Primary_ଚାବି_Index(pNew)->n_ଚାବି_Col!=1
      ){
        /* WITHOUT ROWID virtual tables must either be read-only (xUpdate==0)
        ** or else must have a single-column PRIMARY KEY */
        rc = SQLITE_ERROR;
      }
      pIdx = pNew->pIndex;
      ଯଦି( pIdx ){
        ପ୍ରତିଜ୍ଞା( pIdx->pNext==0 );
        pTab->pIndex = pIdx;
        pNew->pIndex = 0;
        pIdx->pTable = pTab;
      }
    }
    pCtx->bDeclared = 1;
  }ଅନ୍ୟଥା{
    sqlite3_Error_With_ସନ୍ଦେଶ(db, SQLITE_ERROR,
          (sParse.z_Err_ସନ୍ଦେଶ ? "%s" : 0), sParse.z_Err_ସନ୍ଦେଶ);
    sqlite3DbFree(db, sParse.z_Err_ସନ୍ଦେଶ);
    rc = SQLITE_ERROR;
  }
  sParse.eParseMode = PARSE_MODE_NORMAL;

  ଯଦି( sParse.pVdbe ){
    sqlite3VdbeFinalize(sParse.pVdbe);
  }
  sqlite3DeleteTable(db, sParse.pNewTable);
  sqlite3ParseObjectReset(&sParse);
  db->init.busy = initBusy;

  ପ୍ରତିଜ୍ଞା( (rc&0xff)==rc );
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  ଫେରନ୍ତୁ rc;
}

/*
** This function is invoked by the vdbe to call the xDestroy method
** of the virtual table named zTab in database iDb. This occurs
** when a DROP TABLE is mentioned.
**
** This call is a no-op if zTab is not a virtual table.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3VtabCallDestroy(sqlite3 *db, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDb, ଅଚଳ ଅକ୍ଷର *zTab){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
  Table *pTab;

  pTab = sqlite3FindTable(db, zTab, db->aDb[iDb].zDbSName);
  ଯଦି( ALWAYS(pTab!=0)
   && ALWAYS(IsVirtual(pTab))
   && ALWAYS(pTab->u.vtab.p!=0)
  ){
    VTable *p;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*xDestroy)(sqlite3_vtab *);
    ପାଇଁ(p=pTab->u.vtab.p; p; p=p->pNext){
      ପ୍ରତିଜ୍ଞା( p->pVtab );
      ଯଦି( p->pVtab->nRef>0 ){
        ଫେରନ୍ତୁ SQLITE_LOCKED;
      }
    }
    p = vtabDisconnectAll(db, pTab);
    xDestroy = p->pMod->pModule->xDestroy;
    ଯଦି( xDestroy==0 ) xDestroy = p->pMod->pModule->xDisconnect;
    ପ୍ରତିଜ୍ଞା( xDestroy!=0 );
    pTab->nTabRef++;
    rc = xDestroy(p->pVtab);
    /* Remove the sqlite3_vtab* from the aVTrans[] array, if applicable */
    ଯଦି( rc==SQLITE_OK ){
      ପ୍ରତିଜ୍ଞା( pTab->u.vtab.p==p && p->pNext==0 );
      p->pVtab = 0;
      pTab->u.vtab.p = 0;
      sqlite3VtabUnlock(p);
    }
    sqlite3DeleteTable(db, pTab);
  }

  ଫେରନ୍ତୁ rc;
}

/*
** This function invokes either the xRollback or xCommit method
** of each of the virtual tables in the sqlite3.aVTrans array. The method
** called is identified by the second argument, "offset", which is
** the offset of the method to call in the sqlite3_module structure.
**
** The array is cleared after invoking the callbacks.
*/
ସ୍ଥିର ଶୂନ୍ୟ callFinaliser(sqlite3 *db, ପୂର୍ଣ୍ଣସଂଖ୍ୟା offset){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ଯଦି( db->aVTrans ){
    VTable **aVTrans = db->aVTrans;
    db->aVTrans = 0;
    ପାଇଁ(i=0; i<db->nVTrans; i++){
      VTable *pVTab = aVTrans[i];
      sqlite3_vtab *p = pVTab->pVtab;
      ଯଦି( p ){
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*x)(sqlite3_vtab *);
        x = *(ପୂର୍ଣ୍ଣସଂଖ୍ୟା (**)(sqlite3_vtab *))((ଅକ୍ଷର *)p->pModule + offset);
        ଯଦି( x ) x(p);
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3VtabSync(sqlite3 *db, Vdbe *p){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
  VTable **aVTrans = db->aVTrans;

  db->aVTrans = 0;
  ପାଇଁ(i=0; rc==SQLITE_OK && i<db->nVTrans; i++){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*x)(sqlite3_vtab *);
    sqlite3_vtab *pVtab = aVTrans[i]->pVtab;
    ଯଦି( pVtab && (x = pVtab->pModule->xSync)!=0 ){
      rc = x(pVtab);
      sqlite3VtabImportErrmsg(p, pVtab);
    }
  }
  db->aVTrans = aVTrans;
  ଫେରନ୍ତୁ rc;
}

/*
** Invoke the xRollback method of all virtual tables in the
** sqlite3.aVTrans array. Then clear the array itself.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3VtabRollback(sqlite3 *db){
  callFinaliser(db, offsetof(sqlite3_module,xRollback));
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Invoke the xCommit method of all virtual tables in the
** sqlite3.aVTrans array. Then clear the array itself.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3VtabCommit(sqlite3 *db){
  callFinaliser(db, offsetof(sqlite3_module,xCommit));
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** If the virtual table pVtab supports the transaction interface
** (xBegin/xRollback/xCommit and optionally xSync) and a transaction is
** not currently open, invoke the xBegin method now.
**
** If the xBegin call is successful, place the sqlite3_vtab pointer
** in the sqlite3.aVTrans array.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3VtabBegin(sqlite3 *db, VTable *pVTab){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
  ଅଚଳ sqlite3_module *pModule;

  /* Special case: If db->aVTrans is NULL and db->nVTrans is greater
  ** than zero, then this function is being called from within a
  ** virtual module xSync() callback. It is illegal to write to
  ** virtual module tables in this case, so return SQLITE_LOCKED.
  */
  ଯଦି( sqlite3VtabInSync(db) ){
    ଫେରନ୍ତୁ SQLITE_LOCKED;
  }
  ଯଦି( !pVTab ){
    ଫେରନ୍ତୁ SQLITE_OK;
  }
  pModule = pVTab->pVtab->pModule;

  ଯଦି( pModule->xBegin ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

    /* If pVtab is already in the aVTrans array, return early */
    ପାଇଁ(i=0; i<db->nVTrans; i++){
      ଯଦି( db->aVTrans[i]==pVTab ){
        ଫେରନ୍ତୁ SQLITE_OK;
      }
    }

    /* Invoke the xBegin method. If successful, add the vtab to the
    ** sqlite3.aVTrans[] array. */
    rc = growVTrans(db);
    ଯଦି( rc==SQLITE_OK ){
      rc = pModule->xBegin(pVTab->pVtab);
      ଯଦି( rc==SQLITE_OK ){
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା iSvpt = db->nStatement + db->nSavepoint;
        addToVTrans(db, pVTab);
        ଯଦି( iSvpt && pModule->xSavepoint ){
          pVTab->iSavepoint = iSvpt;
          rc = pModule->xSavepoint(pVTab->pVtab, iSvpt-1);
        }
      }
    }
  }
  ଫେରନ୍ତୁ rc;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3VtabSavepoint(sqlite3 *db, ପୂର୍ଣ୍ଣସଂଖ୍ୟା op, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iSavepoint){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;

  ପ୍ରତିଜ୍ଞା( op==SAVEPOINT_RELEASE||op==SAVEPOINT_ROLLBACK||op==SAVEPOINT_BEGIN );
  ପ୍ରତିଜ୍ଞା( iSavepoint>=-1 );
  ଯଦି( db->aVTrans ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ(i=0; rc==SQLITE_OK && i<db->nVTrans; i++){
      VTable *pVTab = db->aVTrans[i];
      ଅଚଳ sqlite3_module *pMod = pVTab->pMod->pModule;
      ଯଦି( pVTab->pVtab && pMod->iVersion>=2 ){
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*xMethod)(sqlite3_vtab *, ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
        sqlite3VtabLock(pVTab);
        ବାଛ( op ){
          ସ୍ଥିତି SAVEPOINT_BEGIN:
            xMethod = pMod->xSavepoint;
            pVTab->iSavepoint = iSavepoint+1;
            ଭାଙ୍ଗ;
          ସ୍ଥିତି SAVEPOINT_ROLLBACK:
            xMethod = pMod->xRollbackTo;
            ଭାଙ୍ଗ;
          ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
            xMethod = pMod->xRelease;
            ଭାଙ୍ଗ;
        }
        ଯଦି( xMethod && pVTab->iSavepoint>iSavepoint ){
          u64 savedFlags = (db->flags & SQLITE_Defensive);
          db->flags &= ~(u64)SQLITE_Defensive;
          rc = xMethod(pVTab->pVtab, iSavepoint);
          db->flags |= savedFlags;
        }
        sqlite3VtabUnlock(pVTab);
      }
    }
  }
  ଫେରନ୍ତୁ rc;
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
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nArg,       /* Number of arguments to the function */
  ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି     /* First argument to the function */
){
  Table *pTab;
  sqlite3_vtab *pVtab;
  sqlite3_module *pMod;
  ଶୂନ୍ୟ (*xSFunc)(sqlite3_context*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,sqlite3_ମୂଲ୍ୟ**) = 0;
  ଶୂନ୍ୟ *pArg = 0;
  FuncDef *pNew;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = 0;

  /* Check to see the left operand is a column in a virtual table */
  ଯଦି( NEVER(p_ପଦସମଷ୍ଟି==0) ) ଫେରନ୍ତୁ pDef;
  ଯଦି( p_ପଦସମଷ୍ଟି->op!=TK_COLUMN ) ଫେରନ୍ତୁ pDef;
  ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_YTab(p_ପଦସମଷ୍ଟି) );
  pTab = p_ପଦସମଷ୍ଟି->y.pTab;
  ଯଦି( NEVER(pTab==0) ) ଫେରନ୍ତୁ pDef;
  ଯଦି( !IsVirtual(pTab) ) ଫେରନ୍ତୁ pDef;
  pVtab = sqlite3GetVTable(db, pTab)->pVtab;
  ପ୍ରତିଜ୍ଞା( pVtab!=0 );
  ପ୍ରତିଜ୍ଞା( pVtab->pModule!=0 );
  pMod = (sqlite3_module *)pVtab->pModule;
  ଯଦି( pMod->xFindFunction==0 ) ଫେରନ୍ତୁ pDef;

  /* Call the xFindFunction method on the virtual table implementation
  ** to see if the implementation wants to overload this function.
  **
  ** Though undocumented, we have historically always invoked xFindFunction
  ** with an all lower-case function name.  Continue in this tradition to
  ** avoid any chance of an incompatibility.
  */
#ifdef SQLITE_DEBUG
  {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ(i=0; pDef->zName[i]; i++){
      unsigned ଅକ୍ଷର x = (unsigned ଅକ୍ଷର)pDef->zName[i];
      ପ୍ରତିଜ୍ଞା( x==sqlite3UpperToLower[x] );
    }
  }
#endif
  rc = pMod->xFindFunction(pVtab, nArg, pDef->zName, &xSFunc, &pArg);
  ଯଦି( rc==0 ){
    ଫେରନ୍ତୁ pDef;
  }

  /* Create a new ephemeral function definition for the overloaded
  ** function */
  pNew = sqlite3DbMallocZero(db, ଆକାର(*pNew)
                             + sqlite3Strlen30(pDef->zName) + 1);
  ଯଦି( pNew==0 ){
    ଫେରନ୍ତୁ pDef;
  }
  *pNew = *pDef;
  pNew->zName = (ଅଚଳ ଅକ୍ଷର*)&pNew[1];
  memcpy((ଅକ୍ଷର*)&pNew[1], pDef->zName, sqlite3Strlen30(pDef->zName)+1);
  pNew->xSFunc = xSFunc;
  pNew->pUserData = pArg;
  pNew->funcFlags |= SQLITE_FUNC_EPHEM;
  ଫେରନ୍ତୁ pNew;
}

/*
** Make sure virtual table pTab is contained in the pParse->apVirtualLock[]
** array so that an OP_VBegin will get generated for it.  Add pTab to the
** array if it is missing.  If pTab is already in the array, this routine
** is a no-op.
*/
ଶୂନ୍ୟ sqlite3VtabMakeWritable(Parse *pParse, Table *pTab){
  Parse *pToplevel = sqlite3ParseToplevel(pParse);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, n;
  Table **apVtabLock;

  ପ୍ରତିଜ୍ଞା( IsVirtual(pTab) );
  ପାଇଁ(i=0; i<pToplevel->nVtabLock; i++){
    ଯଦି( pTab==pToplevel->apVtabLock[i] ) ଫେରନ୍ତୁ;
  }
  n = (pToplevel->nVtabLock+1)*ଆକାର(pToplevel->apVtabLock[0]);
  apVtabLock = sqlite3Realloc(pToplevel->apVtabLock, n);
  ଯଦି( apVtabLock ){
    pToplevel->apVtabLock = apVtabLock;
    pToplevel->apVtabLock[pToplevel->nVtabLock++] = pTab;
  }ଅନ୍ୟଥା{
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3VtabEponymousTableInit(Parse *pParse, Module *pMod){
  ଅଚଳ sqlite3_module *pModule = pMod->pModule;
  Table *pTab;
  ଅକ୍ଷର *zErr = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  sqlite3 *db = pParse->db;
  ଯଦି( pMod->pEpoTab ) ଫେରନ୍ତୁ 1;
  ଯଦି( pModule->xCreate!=0 && pModule->xCreate!=pModule->xConnect ) ଫେରନ୍ତୁ 0;
  pTab = sqlite3DbMallocZero(db, ଆକାର(Table));
  ଯଦି( pTab==0 ) ଫେରନ୍ତୁ 0;
  pTab->zName = sqlite3DbStrDup(db, pMod->zName);
  ଯଦି( pTab->zName==0 ){
    sqlite3DbFree(db, pTab);
    ଫେରନ୍ତୁ 0;
  }
  pMod->pEpoTab = pTab;
  pTab->nTabRef = 1;
  pTab->eTabType = TABTYP_VTAB;
  pTab->pSchema = db->aDb[0].pSchema;
  ପ୍ରତିଜ୍ଞା( pTab->u.vtab.nArg==0 );
  pTab->iPKey = -1;
  pTab->tabFlags |= TF_Eponymous;
  addModuleArgument(pParse, pTab, sqlite3DbStrDup(db, pTab->zName));
  addModuleArgument(pParse, pTab, 0);
  addModuleArgument(pParse, pTab, sqlite3DbStrDup(db, pTab->zName));
  db->nSchemaLock++;
  rc = vtabCallConstructor(db, pTab, pMod, pModule->xConnect, &zErr);
  db->nSchemaLock--;
  ଯଦି( rc ){
    sqlite3_Error_ସନ୍ଦେଶ(pParse, "%s", zErr);
    pParse->rc = rc;
    sqlite3DbFree(db, zErr);
    sqlite3VtabEponymousTableClear(db, pMod);
  }
  ଫେରନ୍ତୁ 1;
}

/*
** Erase the eponymous virtual table instance associated with
** virtual table module pMod, if it exists.
*/
ଶୂନ୍ୟ sqlite3VtabEponymousTableClear(sqlite3 *db, Module *pMod){
  Table *pTab = pMod->pEpoTab;
  ଯଦି( pTab!=0 ){
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
ଶୂନ୍ୟ sqlite3VtabEponymousTableClearAll(sqlite3 *db){
  HashElem *pElem;
  ପାଇଁ(pElem=sqliteHashFirst(&db->aModule); pElem; pElem=sqliteHashNext(pElem)){
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_vtab_on_conflict(sqlite3 *db){
  ସ୍ଥିର ଅଚଳ unsigned ଅକ୍ଷର aMap[] = {
    SQLITE_ROLLBACK, SQLITE_ABORT, SQLITE_FAIL, SQLITE_IGNORE, SQLITE_REPLACE
  };
#ifdef SQLITE_ENABLE_API_ARMOR
  ଯଦି( !sqlite3SafetyCheckOk(db) ) ଫେରନ୍ତୁ SQLITE_MISUSE_BKPT;
#endif
  ପ୍ରତିଜ୍ଞା( OE_Rollback==1 && OE_Abort==2 && OE_Fail==3 );
  ପ୍ରତିଜ୍ଞା( OE_Ignore==4 && OE_Replace==5 );
  ପ୍ରତିଜ୍ଞା( db->vtabOnConflict>=1 && db->vtabOnConflict<=5 );
  ଫେରନ୍ତୁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)aMap[db->vtabOnConflict-1];
}

/*
** Call from within the xCreate() or xConnect() methods to provide
** the SQLite core with additional information about the behavior
** of the virtual table being implemented.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_vtab_config(sqlite3 *db, ପୂର୍ଣ୍ଣସଂଖ୍ୟା op, ...){
  va_list ap;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
  VtabCtx *p;

#ifdef SQLITE_ENABLE_API_ARMOR
  ଯଦି( !sqlite3SafetyCheckOk(db) ) ଫେରନ୍ତୁ SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(db->mutex);
  p = db->pVtabCtx;
  ଯଦି( !p ){
    rc = SQLITE_MISUSE_BKPT;
  }ଅନ୍ୟଥା{
    ପ୍ରତିଜ୍ଞା( p->pTab==0 || IsVirtual(p->pTab) );
    va_start(ap, op);
    ବାଛ( op ){
      ସ୍ଥିତି SQLITE_VTAB_CONSTRAINT_SUPPORT: {
        p->pVTable->bConstraint = (u8)va_arg(ap, ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି SQLITE_VTAB_INNOCUOUS: {
        p->pVTable->eVtabRisk = SQLITE_VTABRISK_ତଳ;
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି SQLITE_VTAB_DIRECTONLY: {
        p->pVTable->eVtabRisk = SQLITE_VTABRISK_ଉପର;
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି SQLITE_VTAB_USES_ALL_SCHEMAS: {
        p->pVTable->bAllSchemas = 1;
        ଭାଙ୍ଗ;
      }
      ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {
        rc = SQLITE_MISUSE_BKPT;
        ଭାଙ୍ଗ;
      }
    }
    va_end(ap);
  }

  ଯଦି( rc!=SQLITE_OK ) sqlite3Error(db, rc);
  sqlite3_mutex_leave(db->mutex);
  ଫେରନ୍ତୁ rc;
}

#endif /* SQLITE_OMIT_VIRTUALTABLE */
