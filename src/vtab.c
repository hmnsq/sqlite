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
#include "sqliteInt.h"

/*
** Before a virtual table xCreate() or xConnect() method is invoked, the
** sqlite3.pVtabCtx member variable is set to point to an instance of
** this struct allocated on the stack. It is used by the implementation of
** the sqlite3_declare_vtab() and sqlite3_vtab_config() APIs, both of which
** are invoked only from within xCreate and xConnect methods.
*/
sa_nracanaa VtabCtx {
  VTable *pVTable;    /* The virtual table being constructed */
  Table *pTab;        /* The Table object to which the virtual table belongs */
  VtabCtx *pPrior;    /* Parent context (if any) */
  ཧྲིལ bDeclared;      /* True after sqlite3_declare_vtab() is called */
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
  const var_na *zName,              /* Name assigned to this module */
  const sqlite3_module *pModule,  /* The definition of the module */
  void *pAux,                     /* Context pointer for xCreate/xConnect */
  void (*xDestroy)(void *)        /* Module destructor function */
){
  Module *pMod;
  Module *pDel;
  var_na *zCopy;
  གལ་སྲིད( pModule==0 ){
    zCopy = (var_na*)zName;
    pMod = 0;
  }དེ་མིན{
    ཧྲིལ nName = sqlite3Strlen30(zName);
    pMod = (Module *)sqlite3Malloc(parimaa_na(Module) + nName + 1);
    གལ་སྲིད( pMod==0 ){
      sqlite3OomFault(db);
      ལོག 0;
    }
    zCopy = (var_na *)(&pMod[1]);
    memcpy(zCopy, zName, nName+1);
    pMod->zName = zCopy;
    pMod->pModule = pModule;
    pMod->pAux = pAux;
    pMod->xDestroy = xDestroy;
    pMod->pEpoTab = 0;
    pMod->nRefModule = 1;
  }
  pDel = (Module *)sqlite3HashInsert(&db->aModule,zCopy,(void*)pMod);
  གལ་སྲིད( pDel ){
    གལ་སྲིད( pDel==pMod ){
      sqlite3OomFault(db);
      sqlite3DbFree(db, pDel);
      pMod = 0;
    }དེ་མིན{
      sqlite3VtabEponymousTableClear(db, pDel);
      sqlite3VtabModuleUnref(db, pDel);
    }
  }
  ལོག pMod;
}

/*
** The actual function that does the work of creating a new module.
** This function implements the sqlite3_create_module() and
** sqlite3_create_module_v2() interfaces.
*/
static ཧྲིལ createModule(
  sqlite3 *db,                    /* Database in which module is registered */
  const var_na *zName,              /* Name assigned to this module */
  const sqlite3_module *pModule,  /* The definition of the module */
  void *pAux,                     /* Context pointer for xCreate/xConnect */
  void (*xDestroy)(void *)        /* Module destructor function */
){
  ཧྲིལ rc = SQLITE_OK;

  sqlite3_mutex_enter(db->mutex);
  (void)sqlite3VtabCreateModule(db, zName, pModule, pAux, xDestroy);
  rc = sqlite3ApiExit(db, rc);
  གལ་སྲིད( rc!=SQLITE_OK && xDestroy ) xDestroy(pAux);
  sqlite3_mutex_leave(db->mutex);
  ལོག rc;
}


/*
** External API function used to create a new virtual-table module.
*/
ཧྲིལ sqlite3_create_module(
  sqlite3 *db,                    /* Database in which module is registered */
  const var_na *zName,              /* Name assigned to this module */
  const sqlite3_module *pModule,  /* The definition of the module */
  void *pAux                      /* Context pointer for xCreate/xConnect */
){
#ifdef SQLITE_ENABLE_API_ARMOR
  གལ་སྲིད( !sqlite3SafetyCheckOk(db) || zName==0 ) ལོག SQLITE_MISUSE_BKPT;
#endif
  ལོག createModule(db, zName, pModule, pAux, 0);
}

/*
** External API function used to create a new virtual-table module.
*/
ཧྲིལ sqlite3_create_module_v2(
  sqlite3 *db,                    /* Database in which module is registered */
  const var_na *zName,              /* Name assigned to this module */
  const sqlite3_module *pModule,  /* The definition of the module */
  void *pAux,                     /* Context pointer for xCreate/xConnect */
  void (*xDestroy)(void *)        /* Module destructor function */
){
#ifdef SQLITE_ENABLE_API_ARMOR
  གལ་སྲིད( !sqlite3SafetyCheckOk(db) || zName==0 ) ལོག SQLITE_MISUSE_BKPT;
#endif
  ལོག createModule(db, zName, pModule, pAux, xDestroy);
}

/*
** External API to drop all virtual-table modules, except those named
** on the azNames list.
*/
ཧྲིལ sqlite3_drop_modules(sqlite3 *db, const var_na** azNames){
  HashElem *pThis, *pNext;
#ifdef SQLITE_ENABLE_API_ARMOR
  གལ་སྲིད( !sqlite3SafetyCheckOk(db) ) ལོག SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(db->mutex);
  དོན་ལུ(pThis=sqliteHashFirst(&db->aModule); pThis; pThis=pNext){
    Module *pMod = (Module*)sqliteHashData(pThis);
    pNext = sqliteHashNext(pThis);
    གལ་སྲིད( azNames ){
      ཧྲིལ ii;
      དོན་ལུ(ii=0; azNames[ii]!=0 && strcmp(azNames[ii],pMod->zName)!=0; ii++){}
      གལ་སྲིད( azNames[ii]!=0 ) continue;
    }
    createModule(db, pMod->zName, 0, 0, 0);
  }
  sqlite3_mutex_leave(db->mutex);
  ལོག SQLITE_OK;
}

/*
** Decrement the reference count on a Module object.  Destroy the
** module when the reference count reaches zero.
*/
void sqlite3VtabModuleUnref(sqlite3 *db, Module *pMod){
  assert( pMod->nRefModule>0 );
  pMod->nRefModule--;
  གལ་སྲིད( pMod->nRefModule==0 ){
    གལ་སྲིད( pMod->xDestroy ){
      pMod->xDestroy(pMod->pAux);
    }
    assert( pMod->pEpoTab==0 );
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
void sqlite3VtabLock(VTable *pVTab){
  pVTab->nRef++;
}


/*
** pTab is a pointer to a Table structure representing a virtual-table.
** Return a pointer to the VTable object used by connection db to access
** this virtual-table, if one has been created, or NULL otherwise.
*/
VTable *sqlite3GetVTable(sqlite3 *db, Table *pTab){
  VTable *pVtab;
  assert( IsVirtual(pTab) );
  དོན་ལུ(pVtab=pTab->u.vtab.p; pVtab && pVtab->db!=db; pVtab=pVtab->pNext);
  ལོག pVtab;
}

/*
** Decrement the ref-count on a virtual table object. When the ref-count
** reaches zero, call the xDisconnect() method to delete the object.
*/
void sqlite3VtabUnlock(VTable *pVTab){
  sqlite3 *db = pVTab->db;

  assert( db );
  assert( pVTab->nRef>0 );
  assert( db->eOpenState==SQLITE_STATE_OPEN
       || db->eOpenState==SQLITE_STATE_ZOMBIE );

  pVTab->nRef--;
  གལ་སྲིད( pVTab->nRef==0 ){
    sqlite3_vtab *p = pVTab->pVtab;
    གལ་སྲིད( p ){
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
static VTable *vtabDisconnectAll(sqlite3 *db, Table *p){
  VTable *pRet = 0;
  VTable *pVTable;

  assert( IsVirtual(p) );
  pVTable = p->u.vtab.p;
  p->u.vtab.p = 0;

  /* Assert that the mutex (if any) associated with the BtShared database
  ** that contains table p is held by the caller. See header comments
  ** above function sqlite3VtabUnlockList() for an explanation of why
  ** this makes it safe to access the sqlite3.pDisconnect list of any
  ** database connection that may have an entry in the p->u.vtab.p list.
  */
  assert( db==0 || sqlite3SchemaMutexHeld(db, 0, p->pSchema) );

  སྐབས( pVTable ){
    sqlite3 *db2 = pVTable->db;
    VTable *pNext = pVTable->pNext;
    assert( db2 );
    གལ་སྲིད( db2==db ){
      pRet = pVTable;
      p->u.vtab.p = pRet;
      pRet->pNext = 0;
    }དེ་མིན{
      pVTable->pNext = db2->pDisconnect;
      db2->pDisconnect = pVTable;
    }
    pVTable = pNext;
  }

  assert( !db || pRet );
  ལོག pRet;
}

/*
** Table *p is a virtual table. This function removes the VTable object
** for table *p associated with database connection db from the linked
** list in p->pVTab. It also decrements the VTable ref count. This is
** used when closing database connection db to free all of its VTable
** objects without disturbing the rest of the Schema object (which may
** be being used by other shared-cache connections).
*/
void sqlite3VtabDisconnect(sqlite3 *db, Table *p){
  VTable **ppVTab;

  assert( IsVirtual(p) );
  assert( sqlite3BtreeHoldsAllMutexes(db) );
  assert( sqlite3_mutex_held(db->mutex) );

  དོན་ལུ(ppVTab=&p->u.vtab.p; *ppVTab; ppVTab=&(*ppVTab)->pNext){
    གལ་སྲིད( (*ppVTab)->db==db  ){
      VTable *pVTab = *ppVTab;
      *ppVTab = pVTab->pNext;
      sqlite3VtabUnlock(pVTab);
      bha_nga;
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
void sqlite3VtabUnlockList(sqlite3 *db){
  VTable *p = db->pDisconnect;

  assert( sqlite3BtreeHoldsAllMutexes(db) );
  assert( sqlite3_mutex_held(db->mutex) );

  གལ་སྲིད( p ){
    db->pDisconnect = 0;
    do {
      VTable *pNext = p->pNext;
      sqlite3VtabUnlock(p);
      p = pNext;
    }སྐབས( p );
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
void sqlite3VtabClear(sqlite3 *db, Table *p){
  assert( IsVirtual(p) );
  assert( db!=0 );
  གལ་སྲིད( db->pnBytesFreed==0 ) vtabDisconnectAll(0, p);
  གལ་སྲིད( p->u.vtab.azArg ){
    ཧྲིལ i;
    དོན་ལུ(i=0; i<p->u.vtab.nArg; i++){
      གལ་སྲིད( i!=1 ) sqlite3DbFree(db, p->u.vtab.azArg[i]);
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
static void addModuleArgument(Parse *pParse, Table *pTable, var_na *zArg){
  sqlite3_int64 nBytes;
  var_na **azModuleArg;
  sqlite3 *db = pParse->db;

  assert( IsVirtual(pTable) );
  nBytes = parimaa_na(var_na *)*(2+pTable->u.vtab.nArg);
  གལ་སྲིད( pTable->u.vtab.nArg+3>=db->aLimit[SQLITE_LIMIT_COLUMN] ){
    sqlite3ErrorMsg(pParse, "too many columns on %s", pTable->zName);
  }
  azModuleArg = sqlite3DbRealloc(db, pTable->u.vtab.azArg, nBytes);
  གལ་སྲིད( azModuleArg==0 ){
    sqlite3DbFree(db, zArg);
  }དེ་མིན{
    ཧྲིལ i = pTable->u.vtab.nArg++;
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
void sqlite3VtabBeginParse(
  Parse *pParse,        /* Parsing context */
  Token *pName1,        /* Name of new table, or database name */
  Token *pName2,        /* Name of new table or NULL */
  Token *pModuleName,   /* Name of the module for the virtual table */
  ཧྲིལ ifNotExists       /* No error if the table already exists */
){
  Table *pTable;        /* The new virtual table */
  sqlite3 *db;          /* Database connection */

  sqlite3StartTable(pParse, pName1, pName2, 0, 0, 1, ifNotExists);
  pTable = pParse->pNewTable;
  གལ་སྲིད( pTable==0 ) ལོག;
  assert( 0==pTable->pIndex );
  pTable->eTabType = TABTYP_VTAB;

  db = pParse->db;

  assert( pTable->u.vtab.nArg==0 );
  addModuleArgument(pParse, pTable, sqlite3NameFromToken(db, pModuleName));
  addModuleArgument(pParse, pTable, 0);
  addModuleArgument(pParse, pTable, sqlite3DbStrDup(db, pTable->zName));
  assert( (pParse->sNameToken.z==pName2->z && pName2->z!=0)
       || (pParse->sNameToken.z==pName1->z && pName2->z==0)
  );
  pParse->sNameToken.n = (ཧྲིལ)(
      &pModuleName->z[pModuleName->n] - pParse->sNameToken.z
  );

#ifndef SQLITE_OMIT_AUTHORIZATION
  /* Creating a virtual table invokes the authorization callback twice.
  ** The first invocation, to obtain permission to INSERT a row into the
  ** sqlite_schema table, has already been made by sqlite3StartTable().
  ** The second call, to obtain permission to create the table, is made now.
  */
  གལ་སྲིད( pTable->u.vtab.azArg ){
    ཧྲིལ iDb = sqlite3SchemaToIndex(db, pTable->pSchema);
    assert( iDb>=0 ); /* The database the table is being created in */
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
static void addArgumentToVtab(Parse *pParse){
  གལ་སྲིད( pParse->sArg.z && pParse->pNewTable ){
    const var_na *z = (const var_na*)pParse->sArg.z;
    ཧྲིལ n = pParse->sArg.n;
    sqlite3 *db = pParse->db;
    addModuleArgument(pParse, pParse->pNewTable, sqlite3DbStrNDup(db, z, n));
  }
}

/*
** The parser calls this routine after the CREATE VIRTUAL TABLE statement
** has been completely parsed.
*/
void sqlite3VtabFinishParse(Parse *pParse, Token *pEnd){
  Table *pTab = pParse->pNewTable;  /* The table being constructed */
  sqlite3 *db = pParse->db;         /* The database connection */

  གལ་སྲིད( pTab==0 ) ལོག;
  assert( IsVirtual(pTab) );
  addArgumentToVtab(pParse);
  pParse->sArg.z = 0;
  གལ་སྲིད( pTab->u.vtab.nArg<1 ) ལོག;

  /* If the CREATE VIRTUAL TABLE statement is being entered for the
  ** first time (in other words if the virtual table is actually being
  ** created now instead of just being read out of sqlite_schema) then
  ** do additional initialization work and store the statement text
  ** in the sqlite_schema table.
  */
  གལ་སྲིད( !db->init.busy ){
    var_na *zStmt;
    var_na *zWhere;
    ཧྲིལ iDb;
    ཧྲིལ iReg;
    Vdbe *v;

    sqlite3MayAbort(pParse);

    /* Compute the complete text of the CREATE VIRTUAL TABLE statement */
    གལ་སྲིད( pEnd ){
      pParse->sNameToken.n = (ཧྲིལ)(pEnd->z - pParse->sNameToken.z) + pEnd->n;
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
    assert( pParse->isCreate );
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
  }དེ་མིན{
    /* If we are rereading the sqlite_schema table create the in-memory
    ** record of the table. */
    Table *pOld;
    Schema *pSchema = pTab->pSchema;
    const var_na *zName = pTab->zName;
    assert( zName!=0 );
    sqlite3MarkAllShadowTablesOf(db, pTab);
    pOld = sqlite3HashInsert(&pSchema->tblHash, zName, pTab);
    གལ་སྲིད( pOld ){
      sqlite3OomFault(db);
      assert( pTab==pOld );  /* Malloc must have failed inside HashInsert() */
      ལོག;
    }
    pParse->pNewTable = 0;
  }
}

/*
** The parser calls this routine when it sees the first token
** of an argument to the module name in a CREATE VIRTUAL TABLE statement.
*/
void sqlite3VtabArgInit(Parse *pParse){
  addArgumentToVtab(pParse);
  pParse->sArg.z = 0;
  pParse->sArg.n = 0;
}

/*
** The parser calls this routine for each token after the first token
** in an argument to the module name in a CREATE VIRTUAL TABLE statement.
*/
void sqlite3VtabArgExtend(Parse *pParse, Token *p){
  Token *pArg = &pParse->sArg;
  གལ་སྲིད( pArg->z==0 ){
    pArg->z = p->z;
    pArg->n = p->n;
  }དེ་མིན{
    assert(pArg->z <= p->z);
    pArg->n = (ཧྲིལ)(&p->z[p->n] - pArg->z);
  }
}

/*
** Invoke a virtual table constructor (either xCreate or xConnect). The
** pointer to the function to invoke is passed as the fourth parameter
** to this procedure.
*/
static ཧྲིལ vtabCallConstructor(
  sqlite3 *db,
  Table *pTab,
  Module *pMod,
  ཧྲིལ (*xConstruct)(sqlite3*,void*,ཧྲིལ,const var_na*const*,sqlite3_vtab**,var_na**),
  var_na **pzErr
){
  VtabCtx sCtx;
  VTable *pVTable;
  ཧྲིལ rc;
  const var_na *const*azArg;
  ཧྲིལ nArg = pTab->u.vtab.nArg;
  var_na *zErr = 0;
  var_na *zModuleName;
  ཧྲིལ iDb;
  VtabCtx *pCtx;

  assert( IsVirtual(pTab) );
  azArg = (const var_na *const*)pTab->u.vtab.azArg;

  /* Check that the virtual-table is not already being initialized */
  དོན་ལུ(pCtx=db->pVtabCtx; pCtx; pCtx=pCtx->pPrior){
    གལ་སྲིད( pCtx->pTab==pTab ){
      *pzErr = sqlite3MPrintf(db,
          "vtable constructor called recursively: %s", pTab->zName
      );
      ལོག SQLITE_LOCKED;
    }
  }

  zModuleName = sqlite3DbStrDup(db, pTab->zName);
  གལ་སྲིད( !zModuleName ){
    ལོག SQLITE_NOMEM_BKPT;
  }

  pVTable = sqlite3MallocZero(parimaa_na(VTable));
  གལ་སྲིད( !pVTable ){
    sqlite3OomFault(db);
    sqlite3DbFree(db, zModuleName);
    ལོག SQLITE_NOMEM_BKPT;
  }
  pVTable->db = db;
  pVTable->pMod = pMod;
  pVTable->eVtabRisk = SQLITE_VTABRISK_Normal;

  iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
  pTab->u.vtab.azArg[1] = db->aDb[iDb].zDbSName;

  /* Invoke the virtual table constructor */
  assert( &db->pVtabCtx );
  assert( xConstruct );
  sCtx.pTab = pTab;
  sCtx.pVTable = pVTable;
  sCtx.pPrior = db->pVtabCtx;
  sCtx.bDeclared = 0;
  db->pVtabCtx = &sCtx;
  pTab->nTabRef++;
  rc = xConstruct(db, pMod->pAux, nArg, azArg, &pVTable->pVtab, &zErr);
  assert( pTab!=0 );
  assert( pTab->nTabRef>1 || rc!=SQLITE_OK );
  sqlite3DeleteTable(db, pTab);
  db->pVtabCtx = sCtx.pPrior;
  གལ་སྲིད( rc==SQLITE_NOMEM ) sqlite3OomFault(db);
  assert( sCtx.pTab==pTab );

  གལ་སྲིད( SQLITE_OK!=rc ){
    གལ་སྲིད( zErr==0 ){
      *pzErr = sqlite3MPrintf(db, "vtable constructor failed: %s", zModuleName);
    }དེ་མིན {
      *pzErr = sqlite3MPrintf(db, "%s", zErr);
      sqlite3_free(zErr);
    }
    sqlite3DbFree(db, pVTable);
  }དེ་མིན གལ་སྲིད( ALWAYS(pVTable->pVtab) ){
    /* Justification of ALWAYS():  A correct vtab constructor must allocate
    ** the sqlite3_vtab object if successful.  */
    memset(pVTable->pVtab, 0, parimaa_na(pVTable->pVtab[0]));
    pVTable->pVtab->pModule = pMod->pModule;
    pMod->nRefModule++;
    pVTable->nRef = 1;
    གལ་སྲིད( sCtx.bDeclared==0 ){
      const var_na *zFormat = "vtable constructor did not declare schema: %s";
      *pzErr = sqlite3MPrintf(db, zFormat, zModuleName);
      sqlite3VtabUnlock(pVTable);
      rc = SQLITE_ERROR;
    }དེ་མིན{
      ཧྲིལ iCol;
      u16 oooHidden = 0;
      /* If everything went according to plan, link the new VTable structure
      ** into the linked list headed by pTab->u.vtab.p. Then loop through the
      ** columns of the table to see if any of them contain the token "hidden".
      ** If so, set the Column COLFLAG_HIDDEN flag and remove the token from
      ** the type string.  */
      pVTable->pNext = pTab->u.vtab.p;
      pTab->u.vtab.p = pVTable;

      དོན་ལུ(iCol=0; iCol<pTab->nCol; iCol++){
        var_na *zType = sqlite3ColumnType(&pTab->aCol[iCol], "");
        ཧྲིལ nType;
        ཧྲིལ i = 0;
        nType = sqlite3Strlen30(zType);
        དོན་ལུ(i=0; i<nType; i++){
          གལ་སྲིད( 0==sqlite3StrNICmp("hidden", &zType[i], 6)
           && (i==0 || zType[i-1]==' ')
           && (zType[i+6]=='\0' || zType[i+6]==' ')
          ){
            bha_nga;
          }
        }
        གལ་སྲིད( i<nType ){
          ཧྲིལ j;
          ཧྲིལ nDel = 6 + (zType[i+6] ? 1 : 0);
          དོན་ལུ(j=i; (j+nDel)<=nType; j++){
            zType[j] = zType[j+nDel];
          }
          གལ་སྲིད( zType[i]=='\0' && i>0 ){
            assert(zType[i-1]==' ');
            zType[i-1] = '\0';
          }
          pTab->aCol[iCol].colFlags |= COLFLAG_HIDDEN;
          pTab->tabFlags |= TF_HasHidden;
          oooHidden = TF_OOOHidden;
        }དེ་མིན{
          pTab->tabFlags |= oooHidden;
        }
      }
    }
  }

  sqlite3DbFree(db, zModuleName);
  ལོག rc;
}

/*
** This function is invoked by the parser to call the xConnect() method
** of the virtual table pTab. If an error occurs, an error code is returned
** and an error left in pParse.
**
** This call is a no-op if table pTab is not a virtual table.
*/
ཧྲིལ sqlite3VtabCallConnect(Parse *pParse, Table *pTab){
  sqlite3 *db = pParse->db;
  const var_na *zMod;
  Module *pMod;
  ཧྲིལ rc;

  assert( pTab );
  assert( IsVirtual(pTab) );
  གལ་སྲིད( sqlite3GetVTable(db, pTab) ){
    ལོག SQLITE_OK;
  }

  /* Locate the required virtual table module */
  zMod = pTab->u.vtab.azArg[0];
  pMod = (Module*)sqlite3HashFind(&db->aModule, zMod);

  གལ་སྲིད( !pMod ){
    const var_na *zModule = pTab->u.vtab.azArg[0];
    sqlite3ErrorMsg(pParse, "no such module: %s", zModule);
    rc = SQLITE_ERROR;
  }དེ་མིན{
    var_na *zErr = 0;
    rc = vtabCallConstructor(db, pTab, pMod, pMod->pModule->xConnect, &zErr);
    གལ་སྲིད( rc!=SQLITE_OK ){
      sqlite3ErrorMsg(pParse, "%s", zErr);
      pParse->rc = rc;
    }དེ་མིན{
      sqlite3MarkAllShadowTablesOf(db, pTab);
    }
    sqlite3DbFree(db, zErr);
  }

  ལོག rc;
}
/*
** Grow the db->aVTrans[] array so that there is room for at least one
** more v-table. Return SQLITE_NOMEM if a malloc fails, or SQLITE_OK otherwise.
*/
static ཧྲིལ growVTrans(sqlite3 *db){
  const ཧྲིལ ARRAY_INCR = 5;

  /* Grow the sqlite3.aVTrans array if required */
  གལ་སྲིད( (db->nVTrans%ARRAY_INCR)==0 ){
    VTable **aVTrans;
    sqlite3_int64 nBytes = parimaa_na(sqlite3_vtab*)*
                                 ((sqlite3_int64)db->nVTrans + ARRAY_INCR);
    aVTrans = sqlite3DbRealloc(db, (void *)db->aVTrans, nBytes);
    གལ་སྲིད( !aVTrans ){
      ལོག SQLITE_NOMEM_BKPT;
    }
    memset(&aVTrans[db->nVTrans], 0, parimaa_na(sqlite3_vtab *)*ARRAY_INCR);
    db->aVTrans = aVTrans;
  }

  ལོག SQLITE_OK;
}

/*
** Add the virtual table pVTab to the array sqlite3.aVTrans[]. Space should
** have already been reserved using growVTrans().
*/
static void addToVTrans(sqlite3 *db, VTable *pVTab){
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
ཧྲིལ sqlite3VtabCallCreate(sqlite3 *db, ཧྲིལ iDb, const var_na *zTab, var_na **pzErr){
  ཧྲིལ rc = SQLITE_OK;
  Table *pTab;
  Module *pMod;
  const var_na *zMod;

  pTab = sqlite3FindTable(db, zTab, db->aDb[iDb].zDbSName);
  assert( pTab && IsVirtual(pTab) && !pTab->u.vtab.p );

  /* Locate the required virtual table module */
  zMod = pTab->u.vtab.azArg[0];
  pMod = (Module*)sqlite3HashFind(&db->aModule, zMod);

  /* If the module has been registered and includes a Create method,
  ** invoke it now. If the module has not been registered, return an
  ** error. Otherwise, do nothing.
  */
  གལ་སྲིད( pMod==0 || pMod->pModule->xCreate==0 || pMod->pModule->xDestroy==0 ){
    *pzErr = sqlite3MPrintf(db, "no such module: %s", zMod);
    rc = SQLITE_ERROR;
  }དེ་མིན{
    rc = vtabCallConstructor(db, pTab, pMod, pMod->pModule->xCreate, pzErr);
  }

  /* Justification of ALWAYS():  The xConstructor method is required to
  ** create a valid sqlite3_vtab if it returns SQLITE_OK. */
  གལ་སྲིད( rc==SQLITE_OK && ALWAYS(sqlite3GetVTable(db, pTab)) ){
    rc = growVTrans(db);
    གལ་སྲིད( rc==SQLITE_OK ){
      VTable *pVTab = sqlite3GetVTable(db, pTab);
      const sqlite3_module *pModule = pMod->pModule;
      ཧྲིལ iSvpt = db->nStatement + db->nSavepoint;
      addToVTrans(db, pVTab);
      /* As in sqlite3VtabBegin(): open the current savepoint on the new
      ** table, so that a ROLLBACK TO that undoes the CREATE also invokes
      ** its xRollbackTo method. */
      གལ་སྲིད( iSvpt && pModule->iVersion>=2 && pModule->xSavepoint ){
        pVTab->iSavepoint = iSvpt;
        rc = pModule->xSavepoint(pVTab->pVtab, iSvpt-1);
      }
    }
  }

  ལོག rc;
}

/*
** This function is used to set the schema of a virtual table.  It is only
** valid to call this function from within the xCreate() or xConnect() of a
** virtual table module.
*/
ཧྲིལ sqlite3_declare_vtab(sqlite3 *db, const var_na *zCreateTable){
  VtabCtx *pCtx;
  ཧྲིལ rc = SQLITE_OK;
  Table *pTab;
  Parse sParse;
  ཧྲིལ initBusy;
  ཧྲིལ i;
  const unsigned var_na *z;
  static const u8 aKeyword[] = { TK_CREATE, TK_TABLE, 0 };

#ifdef SQLITE_ENABLE_API_ARMOR
  གལ་སྲིད( !sqlite3SafetyCheckOk(db) || zCreateTable==0 ){
    ལོག SQLITE_MISUSE_BKPT;
  }
#endif

  /* Verify that the first two keywords in the CREATE TABLE statement
  ** really are "CREATE" and "TABLE".  If this is not the case, then
  ** sqlite3_declare_vtab() is being misused.
  */
  z = (const unsigned var_na*)zCreateTable;
  དོན་ལུ(i=0; aKeyword[i]; i++){
    ཧྲིལ tokenType = 0;
    do{
      z += sqlite3GetToken(z, &tokenType);
    }སྐབས( tokenType==TK_SPACE || tokenType==TK_COMMENT );
    གལ་སྲིད( tokenType!=aKeyword[i] ){
      sqlite3ErrorWithMsg(db, SQLITE_ERROR, "syntax error");
      ལོག SQLITE_ERROR;
    }
  }
 
  sqlite3_mutex_enter(db->mutex);
  pCtx = db->pVtabCtx;
  གལ་སྲིད( !pCtx || pCtx->bDeclared ){
    sqlite3Error(db, SQLITE_MISUSE_BKPT);
    sqlite3_mutex_leave(db->mutex);
    ལོག SQLITE_MISUSE_BKPT;
  }

  pTab = pCtx->pTab;
  assert( IsVirtual(pTab) );

  sqlite3ParseObjectInit(&sParse, db);
  sParse.eParseMode = PARSE_MODE_DECLARE_VTAB;
  sParse.disableTriggers = 1;
  /* We should never be able to reach this point while loading the
  ** schema.  Nevertheless, defend against that (turn off db->init.busy)
  ** in case a bug arises. */
  assert( db->init.busy==0 );
  initBusy = db->init.busy;
  db->init.busy = 0;
  sParse.nQueryLoop = 1;
  གལ་སྲིད( SQLITE_OK==sqlite3RunParser(&sParse, zCreateTable) ){
    assert( sParse.pNewTable!=0 );
    assert( !db->mallocFailed );
    assert( IsOrdinaryTable(sParse.pNewTable) );
    assert( sParse.zErrMsg==0 );
    གལ་སྲིད( !pTab->aCol ){
      Table *pNew = sParse.pNewTable;
      Index *pIdx;
      pTab->aCol = pNew->aCol;
      assert( IsOrdinaryTable(pNew) );
      sqlite3ExprListDelete(db, pNew->u.tab.pDfltList);
      pTab->nNVCol = pTab->nCol = pNew->nCol;
      pTab->tabFlags |= pNew->tabFlags & (TF_WithoutRowid|TF_NoVisibleRowid);
      pNew->nCol = 0;
      pNew->aCol = 0;
      assert( pTab->pIndex==0 );
      assert( HasRowid(pNew) || sqlite3PrimaryKeyIndex(pNew)!=0 );
      གལ་སྲིད( !HasRowid(pNew)
       && pCtx->pVTable->pMod->pModule->xUpdate!=0
       && sqlite3PrimaryKeyIndex(pNew)->nKeyCol!=1
      ){
        /* WITHOUT ROWID virtual tables must either be read-only (xUpdate==0)
        ** or else must have a single-column PRIMARY KEY */
        rc = SQLITE_ERROR;
      }
      pIdx = pNew->pIndex;
      གལ་སྲིད( pIdx ){
        assert( pIdx->pNext==0 );
        pTab->pIndex = pIdx;
        pNew->pIndex = 0;
        pIdx->pTable = pTab;
      }
    }
    pCtx->bDeclared = 1;
  }དེ་མིན{
    sqlite3ErrorWithMsg(db, SQLITE_ERROR,
          (sParse.zErrMsg ? "%s" : 0), sParse.zErrMsg);
    sqlite3DbFree(db, sParse.zErrMsg);
    rc = SQLITE_ERROR;
  }
  sParse.eParseMode = PARSE_MODE_NORMAL;

  གལ་སྲིད( sParse.pVdbe ){
    sqlite3VdbeFinalize(sParse.pVdbe);
  }
  sqlite3DeleteTable(db, sParse.pNewTable);
  sqlite3ParseObjectReset(&sParse);
  db->init.busy = initBusy;

  assert( (rc&0xff)==rc );
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  ལོག rc;
}

/*
** This function is invoked by the vdbe to call the xDestroy method
** of the virtual table named zTab in database iDb. This occurs
** when a DROP TABLE is mentioned.
**
** This call is a no-op if zTab is not a virtual table.
*/
ཧྲིལ sqlite3VtabCallDestroy(sqlite3 *db, ཧྲིལ iDb, const var_na *zTab){
  ཧྲིལ rc = SQLITE_OK;
  Table *pTab;

  pTab = sqlite3FindTable(db, zTab, db->aDb[iDb].zDbSName);
  གལ་སྲིད( ALWAYS(pTab!=0)
   && ALWAYS(IsVirtual(pTab))
   && ALWAYS(pTab->u.vtab.p!=0)
  ){
    VTable *p;
    ཧྲིལ (*xDestroy)(sqlite3_vtab *);
    དོན་ལུ(p=pTab->u.vtab.p; p; p=p->pNext){
      assert( p->pVtab );
      གལ་སྲིད( p->pVtab->nRef>0 ){
        ལོག SQLITE_LOCKED;
      }
    }
    p = vtabDisconnectAll(db, pTab);
    xDestroy = p->pMod->pModule->xDestroy;
    གལ་སྲིད( xDestroy==0 ) xDestroy = p->pMod->pModule->xDisconnect;
    assert( xDestroy!=0 );
    pTab->nTabRef++;
    rc = xDestroy(p->pVtab);
    /* Remove the sqlite3_vtab* from the aVTrans[] array, if applicable */
    གལ་སྲིད( rc==SQLITE_OK ){
      assert( pTab->u.vtab.p==p && p->pNext==0 );
      p->pVtab = 0;
      pTab->u.vtab.p = 0;
      sqlite3VtabUnlock(p);
    }
    sqlite3DeleteTable(db, pTab);
  }

  ལོག rc;
}

/*
** This function invokes either the xRollback or xCommit method
** of each of the virtual tables in the sqlite3.aVTrans array. The method
** called is identified by the second argument, "offset", which is
** the offset of the method to call in the sqlite3_module structure.
**
** The array is cleared after invoking the callbacks.
*/
static void callFinaliser(sqlite3 *db, ཧྲིལ offset){
  ཧྲིལ i;
  གལ་སྲིད( db->aVTrans ){
    VTable **aVTrans = db->aVTrans;
    db->aVTrans = 0;
    དོན་ལུ(i=0; i<db->nVTrans; i++){
      VTable *pVTab = aVTrans[i];
      sqlite3_vtab *p = pVTab->pVtab;
      གལ་སྲིད( p ){
        ཧྲིལ (*x)(sqlite3_vtab *);
        x = *(ཧྲིལ (**)(sqlite3_vtab *))((var_na *)p->pModule + offset);
        གལ་སྲིད( x ) x(p);
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
ཧྲིལ sqlite3VtabSync(sqlite3 *db, Vdbe *p){
  ཧྲིལ i;
  ཧྲིལ rc = SQLITE_OK;
  VTable **aVTrans = db->aVTrans;

  db->aVTrans = 0;
  དོན་ལུ(i=0; rc==SQLITE_OK && i<db->nVTrans; i++){
    ཧྲིལ (*x)(sqlite3_vtab *);
    sqlite3_vtab *pVtab = aVTrans[i]->pVtab;
    གལ་སྲིད( pVtab && (x = pVtab->pModule->xSync)!=0 ){
      rc = x(pVtab);
      sqlite3VtabImportErrmsg(p, pVtab);
    }
  }
  db->aVTrans = aVTrans;
  ལོག rc;
}

/*
** Invoke the xRollback method of all virtual tables in the
** sqlite3.aVTrans array. Then clear the array itself.
*/
ཧྲིལ sqlite3VtabRollback(sqlite3 *db){
  callFinaliser(db, offsetof(sqlite3_module,xRollback));
  ལོག SQLITE_OK;
}

/*
** Invoke the xCommit method of all virtual tables in the
** sqlite3.aVTrans array. Then clear the array itself.
*/
ཧྲིལ sqlite3VtabCommit(sqlite3 *db){
  callFinaliser(db, offsetof(sqlite3_module,xCommit));
  ལོག SQLITE_OK;
}

/*
** If the virtual table pVtab supports the transaction interface
** (xBegin/xRollback/xCommit and optionally xSync) and a transaction is
** not currently open, invoke the xBegin method now.
**
** If the xBegin call is successful, place the sqlite3_vtab pointer
** in the sqlite3.aVTrans array.
*/
ཧྲིལ sqlite3VtabBegin(sqlite3 *db, VTable *pVTab){
  ཧྲིལ rc = SQLITE_OK;
  const sqlite3_module *pModule;

  /* Special case: If db->aVTrans is NULL and db->nVTrans is greater
  ** than zero, then this function is being called from within a
  ** virtual module xSync() callback. It is illegal to write to
  ** virtual module tables in this case, so return SQLITE_LOCKED.
  */
  གལ་སྲིད( sqlite3VtabInSync(db) ){
    ལོག SQLITE_LOCKED;
  }
  གལ་སྲིད( !pVTab ){
    ལོག SQLITE_OK;
  }
  pModule = pVTab->pVtab->pModule;

  གལ་སྲིད( pModule->xBegin ){
    ཧྲིལ i;

    /* If pVtab is already in the aVTrans array, return early */
    དོན་ལུ(i=0; i<db->nVTrans; i++){
      གལ་སྲིད( db->aVTrans[i]==pVTab ){
        ལོག SQLITE_OK;
      }
    }

    /* Invoke the xBegin method. If successful, add the vtab to the
    ** sqlite3.aVTrans[] array. */
    rc = growVTrans(db);
    གལ་སྲིད( rc==SQLITE_OK ){
      rc = pModule->xBegin(pVTab->pVtab);
      གལ་སྲིད( rc==SQLITE_OK ){
        ཧྲིལ iSvpt = db->nStatement + db->nSavepoint;
        addToVTrans(db, pVTab);
        གལ་སྲིད( iSvpt && pModule->xSavepoint ){
          pVTab->iSavepoint = iSvpt;
          rc = pModule->xSavepoint(pVTab->pVtab, iSvpt-1);
        }
      }
    }
  }
  ལོག rc;
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
ཧྲིལ sqlite3VtabSavepoint(sqlite3 *db, ཧྲིལ op, ཧྲིལ iSavepoint){
  ཧྲིལ rc = SQLITE_OK;

  assert( op==SAVEPOINT_RELEASE||op==SAVEPOINT_ROLLBACK||op==SAVEPOINT_BEGIN );
  assert( iSavepoint>=-1 );
  གལ་སྲིད( db->aVTrans ){
    ཧྲིལ i;
    དོན་ལུ(i=0; rc==SQLITE_OK && i<db->nVTrans; i++){
      VTable *pVTab = db->aVTrans[i];
      const sqlite3_module *pMod = pVTab->pMod->pModule;
      གལ་སྲིད( pVTab->pVtab && pMod->iVersion>=2 ){
        ཧྲིལ (*xMethod)(sqlite3_vtab *, ཧྲིལ);
        sqlite3VtabLock(pVTab);
        switch( op ){
          pak_sa SAVEPOINT_BEGIN:
            xMethod = pMod->xSavepoint;
            pVTab->iSavepoint = iSavepoint+1;
            bha_nga;
          pak_sa SAVEPOINT_ROLLBACK:
            xMethod = pMod->xRollbackTo;
            bha_nga;
          default:
            xMethod = pMod->xRelease;
            bha_nga;
        }
        གལ་སྲིད( xMethod && pVTab->iSavepoint>iSavepoint ){
          u64 savedFlags = (db->flags & SQLITE_Defensive);
          db->flags &= ~(u64)SQLITE_Defensive;
          rc = xMethod(pVTab->pVtab, iSavepoint);
          db->flags |= savedFlags;
        }
        sqlite3VtabUnlock(pVTab);
      }
    }
  }
  ལོག rc;
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
  ཧྲིལ nArg,       /* Number of arguments to the function */
  Expr *pExpr     /* First argument to the function */
){
  Table *pTab;
  sqlite3_vtab *pVtab;
  sqlite3_module *pMod;
  void (*xSFunc)(sqlite3_context*,ཧྲིལ,sqlite3_value**) = 0;
  void *pArg = 0;
  FuncDef *pNew;
  ཧྲིལ rc = 0;

  /* Check to see the left operand is a column in a virtual table */
  གལ་སྲིད( NEVER(pExpr==0) ) ལོག pDef;
  གལ་སྲིད( pExpr->op!=TK_COLUMN ) ལོག pDef;
  assert( ExprUseYTab(pExpr) );
  pTab = pExpr->y.pTab;
  གལ་སྲིད( NEVER(pTab==0) ) ལོག pDef;
  གལ་སྲིད( !IsVirtual(pTab) ) ལོག pDef;
  pVtab = sqlite3GetVTable(db, pTab)->pVtab;
  assert( pVtab!=0 );
  assert( pVtab->pModule!=0 );
  pMod = (sqlite3_module *)pVtab->pModule;
  གལ་སྲིད( pMod->xFindFunction==0 ) ལོག pDef;

  /* Call the xFindFunction method on the virtual table implementation
  ** to see if the implementation wants to overload this function.
  **
  ** Though undocumented, we have historically always invoked xFindFunction
  ** with an all lower-case function name.  Continue in this tradition to
  ** avoid any chance of an incompatibility.
  */
#ifdef SQLITE_DEBUG
  {
    ཧྲིལ i;
    དོན་ལུ(i=0; pDef->zName[i]; i++){
      unsigned var_na x = (unsigned var_na)pDef->zName[i];
      assert( x==sqlite3UpperToLower[x] );
    }
  }
#endif
  rc = pMod->xFindFunction(pVtab, nArg, pDef->zName, &xSFunc, &pArg);
  གལ་སྲིད( rc==0 ){
    ལོག pDef;
  }

  /* Create a new ephemeral function definition for the overloaded
  ** function */
  pNew = sqlite3DbMallocZero(db, parimaa_na(*pNew)
                             + sqlite3Strlen30(pDef->zName) + 1);
  གལ་སྲིད( pNew==0 ){
    ལོག pDef;
  }
  *pNew = *pDef;
  pNew->zName = (const var_na*)&pNew[1];
  memcpy((var_na*)&pNew[1], pDef->zName, sqlite3Strlen30(pDef->zName)+1);
  pNew->xSFunc = xSFunc;
  pNew->pUserData = pArg;
  pNew->funcFlags |= SQLITE_FUNC_EPHEM;
  ལོག pNew;
}

/*
** Make sure virtual table pTab is contained in the pParse->apVirtualLock[]
** array so that an OP_VBegin will get generated for it.  Add pTab to the
** array if it is missing.  If pTab is already in the array, this routine
** is a no-op.
*/
void sqlite3VtabMakeWritable(Parse *pParse, Table *pTab){
  Parse *pToplevel = sqlite3ParseToplevel(pParse);
  ཧྲིལ i, n;
  Table **apVtabLock;

  assert( IsVirtual(pTab) );
  དོན་ལུ(i=0; i<pToplevel->nVtabLock; i++){
    གལ་སྲིད( pTab==pToplevel->apVtabLock[i] ) ལོག;
  }
  n = (pToplevel->nVtabLock+1)*parimaa_na(pToplevel->apVtabLock[0]);
  apVtabLock = sqlite3Realloc(pToplevel->apVtabLock, n);
  གལ་སྲིད( apVtabLock ){
    pToplevel->apVtabLock = apVtabLock;
    pToplevel->apVtabLock[pToplevel->nVtabLock++] = pTab;
  }དེ་མིན{
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
ཧྲིལ sqlite3VtabEponymousTableInit(Parse *pParse, Module *pMod){
  const sqlite3_module *pModule = pMod->pModule;
  Table *pTab;
  var_na *zErr = 0;
  ཧྲིལ rc;
  sqlite3 *db = pParse->db;
  གལ་སྲིད( pMod->pEpoTab ) ལོག 1;
  གལ་སྲིད( pModule->xCreate!=0 && pModule->xCreate!=pModule->xConnect ) ལོག 0;
  pTab = sqlite3DbMallocZero(db, parimaa_na(Table));
  གལ་སྲིད( pTab==0 ) ལོག 0;
  pTab->zName = sqlite3DbStrDup(db, pMod->zName);
  གལ་སྲིད( pTab->zName==0 ){
    sqlite3DbFree(db, pTab);
    ལོག 0;
  }
  pMod->pEpoTab = pTab;
  pTab->nTabRef = 1;
  pTab->eTabType = TABTYP_VTAB;
  pTab->pSchema = db->aDb[0].pSchema;
  assert( pTab->u.vtab.nArg==0 );
  pTab->iPKey = -1;
  pTab->tabFlags |= TF_Eponymous;
  addModuleArgument(pParse, pTab, sqlite3DbStrDup(db, pTab->zName));
  addModuleArgument(pParse, pTab, 0);
  addModuleArgument(pParse, pTab, sqlite3DbStrDup(db, pTab->zName));
  db->nSchemaLock++;
  rc = vtabCallConstructor(db, pTab, pMod, pModule->xConnect, &zErr);
  db->nSchemaLock--;
  གལ་སྲིད( rc ){
    sqlite3ErrorMsg(pParse, "%s", zErr);
    pParse->rc = rc;
    sqlite3DbFree(db, zErr);
    sqlite3VtabEponymousTableClear(db, pMod);
  }
  ལོག 1;
}

/*
** Erase the eponymous virtual table instance associated with
** virtual table module pMod, if it exists.
*/
void sqlite3VtabEponymousTableClear(sqlite3 *db, Module *pMod){
  Table *pTab = pMod->pEpoTab;
  གལ་སྲིད( pTab!=0 ){
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
void sqlite3VtabEponymousTableClearAll(sqlite3 *db){
  HashElem *pElem;
  དོན་ལུ(pElem=sqliteHashFirst(&db->aModule); pElem; pElem=sqliteHashNext(pElem)){
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
ཧྲིལ sqlite3_vtab_on_conflict(sqlite3 *db){
  static const unsigned var_na aMap[] = {
    SQLITE_ROLLBACK, SQLITE_ABORT, SQLITE_FAIL, SQLITE_IGNORE, SQLITE_REPLACE
  };
#ifdef SQLITE_ENABLE_API_ARMOR
  གལ་སྲིད( !sqlite3SafetyCheckOk(db) ) ལོག SQLITE_MISUSE_BKPT;
#endif
  assert( OE_Rollback==1 && OE_Abort==2 && OE_Fail==3 );
  assert( OE_Ignore==4 && OE_Replace==5 );
  assert( db->vtabOnConflict>=1 && db->vtabOnConflict<=5 );
  ལོག (ཧྲིལ)aMap[db->vtabOnConflict-1];
}

/*
** Call from within the xCreate() or xConnect() methods to provide
** the SQLite core with additional information about the behavior
** of the virtual table being implemented.
*/
ཧྲིལ sqlite3_vtab_config(sqlite3 *db, ཧྲིལ op, ...){
  va_list ap;
  ཧྲིལ rc = SQLITE_OK;
  VtabCtx *p;

#ifdef SQLITE_ENABLE_API_ARMOR
  གལ་སྲིད( !sqlite3SafetyCheckOk(db) ) ལོག SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(db->mutex);
  p = db->pVtabCtx;
  གལ་སྲིད( !p ){
    rc = SQLITE_MISUSE_BKPT;
  }དེ་མིན{
    assert( p->pTab==0 || IsVirtual(p->pTab) );
    va_start(ap, op);
    switch( op ){
      pak_sa SQLITE_VTAB_CONSTRAINT_SUPPORT: {
        p->pVTable->bConstraint = (u8)va_arg(ap, ཧྲིལ);
        bha_nga;
      }
      pak_sa SQLITE_VTAB_INNOCUOUS: {
        p->pVTable->eVtabRisk = SQLITE_VTABRISK_Low;
        bha_nga;
      }
      pak_sa SQLITE_VTAB_DIRECTONLY: {
        p->pVTable->eVtabRisk = SQLITE_VTABRISK_High;
        bha_nga;
      }
      pak_sa SQLITE_VTAB_USES_ALL_SCHEMAS: {
        p->pVTable->bAllSchemas = 1;
        bha_nga;
      }
      default: {
        rc = SQLITE_MISUSE_BKPT;
        bha_nga;
      }
    }
    va_end(ap);
  }

  གལ་སྲིད( rc!=SQLITE_OK ) sqlite3Error(db, rc);
  sqlite3_mutex_leave(db->mutex);
  ལོག rc;
}

#endif /* SQLITE_OMIT_VIRTUALTABLE */
