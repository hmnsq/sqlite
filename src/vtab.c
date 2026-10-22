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
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"

/*
** Before a virtual table xCreate() or xConnect() method is invoked, the
** sqlite3.pVtabCtx member variable is set to point to an instance of
** this struct allocated on the stack. It is used by the implementation of
** the sqlite3_declare_vtab() and sqlite3_vtab_config() APIs, both of which
** are invoked only from within xCreate and xConnect methods.
*/
ਬਣਤਰ VtabCtx {
  VTable *pVTable;    /* The virtual table being constructed */
  ਸਾਰਣੀ *pTab;        /* The Table object to which the virtual table belongs */
  VtabCtx *p_ਪੂਰਵ;    /* Parent context (if any) */
  ਪੂਰਨ bDeclared;      /* True after sqlite3_declare_vtab() is called */
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
  ਅਚਲ ਅੱਖਰ *zName,              /* Name assigned to this module */
  ਅਚਲ sqlite3_module *pModule,  /* The definition of the module */
  ਖਾਲੀ *pAux,                     /* Context pointer for xCreate/xConnect */
  ਖਾਲੀ (*xDestroy)(ਖਾਲੀ *)        /* Module destructor function */
){
  Module *pMod;
  Module *pDel;
  ਅੱਖਰ *zCopy;
  ਜੇ( pModule==0 ){
    zCopy = (ਅੱਖਰ*)zName;
    pMod = 0;
  }ਨਹੀਂਤਾਂ{
    ਪੂਰਨ nName = sqlite3Strlen30(zName);
    pMod = (Module *)sqlite3Malloc(ਆਕਾਰ(Module) + nName + 1);
    ਜੇ( pMod==0 ){
      sqlite3OomFault(db);
      ਵਾਪਸ 0;
    }
    zCopy = (ਅੱਖਰ *)(&pMod[1]);
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(zCopy, zName, nName+1);
    pMod->zName = zCopy;
    pMod->pModule = pModule;
    pMod->pAux = pAux;
    pMod->xDestroy = xDestroy;
    pMod->pEpoTab = 0;
    pMod->nRefModule = 1;
  }
  pDel = (Module *)sqlite3_Hash_ਪਾਓ(&db->aModule,zCopy,(ਖਾਲੀ*)pMod);
  ਜੇ( pDel ){
    ਜੇ( pDel==pMod ){
      sqlite3OomFault(db);
      sqlite3DbFree(db, pDel);
      pMod = 0;
    }ਨਹੀਂਤਾਂ{
      sqlite3_Vtab_Eponymous_ਸਾਰਣੀ_Clear(db, pDel);
      sqlite3VtabModuleUnref(db, pDel);
    }
  }
  ਵਾਪਸ pMod;
}

/*
** The actual function that does the work of creating a new module.
** This function implements the sqlite3_create_module() and
** sqlite3_create_module_v2() interfaces.
*/
ਸਥਿਰ ਪੂਰਨ createModule(
  sqlite3 *db,                    /* Database in which module is registered */
  ਅਚਲ ਅੱਖਰ *zName,              /* Name assigned to this module */
  ਅਚਲ sqlite3_module *pModule,  /* The definition of the module */
  ਖਾਲੀ *pAux,                     /* Context pointer for xCreate/xConnect */
  ਖਾਲੀ (*xDestroy)(ਖਾਲੀ *)        /* Module destructor function */
){
  ਪੂਰਨ rc = SQLITE_OK;

  sqlite3_mutex_enter(db->mutex);
  (ਖਾਲੀ)sqlite3VtabCreateModule(db, zName, pModule, pAux, xDestroy);
  rc = sqlite3ApiExit(db, rc);
  ਜੇ( rc!=SQLITE_OK && xDestroy ) xDestroy(pAux);
  sqlite3_mutex_leave(db->mutex);
  ਵਾਪਸ rc;
}


/*
** External API function used to create a new virtual-table module.
*/
ਪੂਰਨ sqlite3_create_module(
  sqlite3 *db,                    /* Database in which module is registered */
  ਅਚਲ ਅੱਖਰ *zName,              /* Name assigned to this module */
  ਅਚਲ sqlite3_module *pModule,  /* The definition of the module */
  ਖਾਲੀ *pAux                      /* Context pointer for xCreate/xConnect */
){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !sqlite3SafetyCheckOk(db) || zName==0 ) ਵਾਪਸ SQLITE_MISUSE_BKPT;
#ਜੇ_ਅੰਤ
  ਵਾਪਸ createModule(db, zName, pModule, pAux, 0);
}

/*
** External API function used to create a new virtual-table module.
*/
ਪੂਰਨ sqlite3_create_module_v2(
  sqlite3 *db,                    /* Database in which module is registered */
  ਅਚਲ ਅੱਖਰ *zName,              /* Name assigned to this module */
  ਅਚਲ sqlite3_module *pModule,  /* The definition of the module */
  ਖਾਲੀ *pAux,                     /* Context pointer for xCreate/xConnect */
  ਖਾਲੀ (*xDestroy)(ਖਾਲੀ *)        /* Module destructor function */
){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !sqlite3SafetyCheckOk(db) || zName==0 ) ਵਾਪਸ SQLITE_MISUSE_BKPT;
#ਜੇ_ਅੰਤ
  ਵਾਪਸ createModule(db, zName, pModule, pAux, xDestroy);
}

/*
** External API to drop all virtual-table modules, except those named
** on the azNames list.
*/
ਪੂਰਨ sqlite3_drop_modules(sqlite3 *db, ਅਚਲ ਅੱਖਰ** az_ਨਾਂ){
  HashElem *pThis, *p_ਅਗਲਾ;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !sqlite3SafetyCheckOk(db) ) ਵਾਪਸ SQLITE_MISUSE_BKPT;
#ਜੇ_ਅੰਤ
  sqlite3_mutex_enter(db->mutex);
  ਲਈ(pThis=sqlite_Hash_ਪਹਿਲਾ(&db->aModule); pThis; pThis=p_ਅਗਲਾ){
    Module *pMod = (Module*)sqlite_Hash_ਅੰਕੜੇ(pThis);
    p_ਅਗਲਾ = sqlite_Hash_ਅਗਲਾ(pThis);
    ਜੇ( az_ਨਾਂ ){
      ਪੂਰਨ ii;
      ਲਈ(ii=0; az_ਨਾਂ[ii]!=0 && strcmp(az_ਨਾਂ[ii],pMod->zName)!=0; ii++){}
      ਜੇ( az_ਨਾਂ[ii]!=0 ) ਜਾਰੀ;
    }
    createModule(db, pMod->zName, 0, 0, 0);
  }
  sqlite3_mutex_leave(db->mutex);
  ਵਾਪਸ SQLITE_OK;
}

/*
** Decrement the reference count on a Module object.  Destroy the
** module when the reference count reaches zero.
*/
ਖਾਲੀ sqlite3VtabModuleUnref(sqlite3 *db, Module *pMod){
  ਪ੍ਰਤਿਜ੍ਞਾ( pMod->nRefModule>0 );
  pMod->nRefModule--;
  ਜੇ( pMod->nRefModule==0 ){
    ਜੇ( pMod->xDestroy ){
      pMod->xDestroy(pMod->pAux);
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( pMod->pEpoTab==0 );
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
ਖਾਲੀ sqlite3_Vtab_ਤਾਲਾ(VTable *pVTab){
  pVTab->nRef++;
}


/*
** pTab is a pointer to a Table structure representing a virtual-table.
** Return a pointer to the VTable object used by connection db to access
** this virtual-table, if one has been created, or NULL otherwise.
*/
VTable *sqlite3GetVTable(sqlite3 *db, ਸਾਰਣੀ *pTab){
  VTable *pVtab;
  ਪ੍ਰਤਿਜ੍ਞਾ( IsVirtual(pTab) );
  ਲਈ(pVtab=pTab->u.vtab.p; pVtab && pVtab->db!=db; pVtab=pVtab->p_ਅਗਲਾ);
  ਵਾਪਸ pVtab;
}

/*
** Decrement the ref-count on a virtual table object. When the ref-count
** reaches zero, call the xDisconnect() method to delete the object.
*/
ਖਾਲੀ sqlite3VtabUnlock(VTable *pVTab){
  sqlite3 *db = pVTab->db;

  ਪ੍ਰਤਿਜ੍ਞਾ( db );
  ਪ੍ਰਤਿਜ੍ਞਾ( pVTab->nRef>0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( db->eOpenState==SQLITE_STATE_OPEN
       || db->eOpenState==SQLITE_STATE_ZOMBIE );

  pVTab->nRef--;
  ਜੇ( pVTab->nRef==0 ){
    sqlite3_vtab *p = pVTab->pVtab;
    ਜੇ( p ){
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
ਸਥਿਰ VTable *vtabDisconnectAll(sqlite3 *db, ਸਾਰਣੀ *p){
  VTable *pRet = 0;
  VTable *pVTable;

  ਪ੍ਰਤਿਜ੍ਞਾ( IsVirtual(p) );
  pVTable = p->u.vtab.p;
  p->u.vtab.p = 0;

  /* Assert that the mutex (if any) associated with the BtShared database
  ** that contains table p is held by the caller. See header comments
  ** above function sqlite3VtabUnlockList() for an explanation of why
  ** this makes it safe to access the sqlite3.pDisconnect list of any
  ** database connection that may have an entry in the p->u.vtab.p list.
  */
  ਪ੍ਰਤਿਜ੍ਞਾ( db==0 || sqlite3SchemaMutexHeld(db, 0, p->pSchema) );

  ਜਦੋਂ( pVTable ){
    sqlite3 *db2 = pVTable->db;
    VTable *p_ਅਗਲਾ = pVTable->p_ਅਗਲਾ;
    ਪ੍ਰਤਿਜ੍ਞਾ( db2 );
    ਜੇ( db2==db ){
      pRet = pVTable;
      p->u.vtab.p = pRet;
      pRet->p_ਅਗਲਾ = 0;
    }ਨਹੀਂਤਾਂ{
      pVTable->p_ਅਗਲਾ = db2->pDisconnect;
      db2->pDisconnect = pVTable;
    }
    pVTable = p_ਅਗਲਾ;
  }

  ਪ੍ਰਤਿਜ੍ਞਾ( !db || pRet );
  ਵਾਪਸ pRet;
}

/*
** Table *p is a virtual table. This function removes the VTable object
** for table *p associated with database connection db from the linked
** list in p->pVTab. It also decrements the VTable ref count. This is
** used when closing database connection db to free all of its VTable
** objects without disturbing the rest of the Schema object (which may
** be being used by other shared-cache connections).
*/
ਖਾਲੀ sqlite3VtabDisconnect(sqlite3 *db, ਸਾਰਣੀ *p){
  VTable **ppVTab;

  ਪ੍ਰਤਿਜ੍ਞਾ( IsVirtual(p) );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3BtreeHoldsAllMutexes(db) );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(db->mutex) );

  ਲਈ(ppVTab=&p->u.vtab.p; *ppVTab; ppVTab=&(*ppVTab)->p_ਅਗਲਾ){
    ਜੇ( (*ppVTab)->db==db  ){
      VTable *pVTab = *ppVTab;
      *ppVTab = pVTab->p_ਅਗਲਾ;
      sqlite3VtabUnlock(pVTab);
      ਰੋਕੋ;
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
ਖਾਲੀ sqlite3VtabUnlockList(sqlite3 *db){
  VTable *p = db->pDisconnect;

  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3BtreeHoldsAllMutexes(db) );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(db->mutex) );

  ਜੇ( p ){
    db->pDisconnect = 0;
    ਕਰੋ {
      VTable *p_ਅਗਲਾ = p->p_ਅਗਲਾ;
      sqlite3VtabUnlock(p);
      p = p_ਅਗਲਾ;
    }ਜਦੋਂ( p );
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
ਖਾਲੀ sqlite3VtabClear(sqlite3 *db, ਸਾਰਣੀ *p){
  ਪ੍ਰਤਿਜ੍ਞਾ( IsVirtual(p) );
  ਪ੍ਰਤਿਜ੍ਞਾ( db!=0 );
  ਜੇ( db->pn_ਬਾਈਟਾਂ_Freed==0 ) vtabDisconnectAll(0, p);
  ਜੇ( p->u.vtab.azArg ){
    ਪੂਰਨ i;
    ਲਈ(i=0; i<p->u.vtab.nArg; i++){
      ਜੇ( i!=1 ) sqlite3DbFree(db, p->u.vtab.azArg[i]);
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
ਸਥਿਰ ਖਾਲੀ ਜੋੜੋ_Module_ਦਲੀਲ(Parse *pParse, ਸਾਰਣੀ *p_ਸਾਰਣੀ, ਅੱਖਰ *zArg){
  sqlite3_int64 n_ਬਾਈਟਾਂ;
  ਅੱਖਰ **azModuleArg;
  sqlite3 *db = pParse->db;

  ਪ੍ਰਤਿਜ੍ਞਾ( IsVirtual(p_ਸਾਰਣੀ) );
  n_ਬਾਈਟਾਂ = ਆਕਾਰ(ਅੱਖਰ *)*(2+p_ਸਾਰਣੀ->u.vtab.nArg);
  ਜੇ( p_ਸਾਰਣੀ->u.vtab.nArg+3>=db->a_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_ਕਾਲਮ] ){
    sqlite3_Error_ਸੁਨੇਹਾ(pParse, "too many columns on %s", p_ਸਾਰਣੀ->zName);
  }
  azModuleArg = sqlite3DbRealloc(db, p_ਸਾਰਣੀ->u.vtab.azArg, n_ਬਾਈਟਾਂ);
  ਜੇ( azModuleArg==0 ){
    sqlite3DbFree(db, zArg);
  }ਨਹੀਂਤਾਂ{
    ਪੂਰਨ i = p_ਸਾਰਣੀ->u.vtab.nArg++;
    azModuleArg[i] = zArg;
    azModuleArg[i+1] = 0;
    p_ਸਾਰਣੀ->u.vtab.azArg = azModuleArg;
  }
}

/*
** The parser calls this routine when it first sees a CREATE VIRTUAL TABLE
** statement.  The module name has been parsed, but the optional list
** of parameters that follow the module name are still pending.
*/
ਖਾਲੀ sqlite3VtabBeginParse(
  Parse *pParse,        /* Parsing context */
  Token *pName1,        /* Name of new table, or database name */
  Token *pName2,        /* Name of new table or NULL */
  Token *pModuleName,   /* Name of the module for the virtual table */
  ਪੂਰਨ ifNotExists       /* No error if the table already exists */
){
  ਸਾਰਣੀ *p_ਸਾਰਣੀ;        /* The new virtual table */
  sqlite3 *db;          /* Database connection */

  sqlite3_ਸ਼ੁਰੂ_ਸਾਰਣੀ(pParse, pName1, pName2, 0, 0, 1, ifNotExists);
  p_ਸਾਰਣੀ = pParse->p_New_ਸਾਰਣੀ;
  ਜੇ( p_ਸਾਰਣੀ==0 ) ਵਾਪਸ;
  ਪ੍ਰਤਿਜ੍ਞਾ( 0==p_ਸਾਰਣੀ->p_ਸੂਚਕਾਂਕ );
  p_ਸਾਰਣੀ->eTabType = TABTYP_VTAB;

  db = pParse->db;

  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਸਾਰਣੀ->u.vtab.nArg==0 );
  ਜੋੜੋ_Module_ਦਲੀਲ(pParse, p_ਸਾਰਣੀ, sqlite3NameFromToken(db, pModuleName));
  ਜੋੜੋ_Module_ਦਲੀਲ(pParse, p_ਸਾਰਣੀ, 0);
  ਜੋੜੋ_Module_ਦਲੀਲ(pParse, p_ਸਾਰਣੀ, sqlite3DbStrDup(db, p_ਸਾਰਣੀ->zName));
  ਪ੍ਰਤਿਜ੍ਞਾ( (pParse->sNameToken.z==pName2->z && pName2->z!=0)
       || (pParse->sNameToken.z==pName1->z && pName2->z==0)
  );
  pParse->sNameToken.n = (ਪੂਰਨ)(
      &pModuleName->z[pModuleName->n] - pParse->sNameToken.z
  );

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTHORIZATION
  /* Creating a virtual table invokes the authorization callback twice.
  ** The first invocation, to obtain permission to INSERT a row into the
  ** sqlite_schema table, has already been made by sqlite3StartTable().
  ** The second call, to obtain permission to create the table, is made now.
  */
  ਜੇ( p_ਸਾਰਣੀ->u.vtab.azArg ){
    ਪੂਰਨ iDb = sqlite3_Schema_ਮੰਜ਼ਿਲ_ਸੂਚਕਾਂਕ(db, p_ਸਾਰਣੀ->pSchema);
    ਪ੍ਰਤਿਜ੍ਞਾ( iDb>=0 ); /* The database the table is being created in */
    sqlite3AuthCheck(pParse, SQLITE_CREATE_VTABLE, p_ਸਾਰਣੀ->zName,
            p_ਸਾਰਣੀ->u.vtab.azArg[0], pParse->db->aDb[iDb].zDbSName);
  }
#ਜੇ_ਅੰਤ
}

/*
** This routine takes the module argument that has been accumulating
** in pParse->zArg[] and appends it to the list of arguments on the
** virtual table currently under construction in pParse->pTable.
*/
ਸਥਿਰ ਖਾਲੀ ਜੋੜੋ_ਦਲੀਲ_ਮੰਜ਼ਿਲ_Vtab(Parse *pParse){
  ਜੇ( pParse->sArg.z && pParse->p_New_ਸਾਰਣੀ ){
    ਅਚਲ ਅੱਖਰ *z = (ਅਚਲ ਅੱਖਰ*)pParse->sArg.z;
    ਪੂਰਨ n = pParse->sArg.n;
    sqlite3 *db = pParse->db;
    ਜੋੜੋ_Module_ਦਲੀਲ(pParse, pParse->p_New_ਸਾਰਣੀ, sqlite3DbStrNDup(db, z, n));
  }
}

/*
** The parser calls this routine after the CREATE VIRTUAL TABLE statement
** has been completely parsed.
*/
ਖਾਲੀ sqlite3_Vtab_ਅੰਤ_Parse(Parse *pParse, Token *p_ਸਿਰਾ){
  ਸਾਰਣੀ *pTab = pParse->p_New_ਸਾਰਣੀ;  /* The table being constructed */
  sqlite3 *db = pParse->db;         /* The database connection */

  ਜੇ( pTab==0 ) ਵਾਪਸ;
  ਪ੍ਰਤਿਜ੍ਞਾ( IsVirtual(pTab) );
  ਜੋੜੋ_ਦਲੀਲ_ਮੰਜ਼ਿਲ_Vtab(pParse);
  pParse->sArg.z = 0;
  ਜੇ( pTab->u.vtab.nArg<1 ) ਵਾਪਸ;

  /* If the CREATE VIRTUAL TABLE statement is being entered for the
  ** first time (in other words if the virtual table is actually being
  ** created now instead of just being read out of sqlite_schema) then
  ** do additional initialization work and store the statement text
  ** in the sqlite_schema table.
  */
  ਜੇ( !db->init.busy ){
    ਅੱਖਰ *zStmt;
    ਅੱਖਰ *zWhere;
    ਪੂਰਨ iDb;
    ਪੂਰਨ iReg;
    Vdbe *v;

    sqlite3MayAbort(pParse);

    /* Compute the complete text of the CREATE VIRTUAL TABLE statement */
    ਜੇ( p_ਸਿਰਾ ){
      pParse->sNameToken.n = (ਪੂਰਨ)(p_ਸਿਰਾ->z - pParse->sNameToken.z) + p_ਸਿਰਾ->n;
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
    iDb = sqlite3_Schema_ਮੰਜ਼ਿਲ_ਸੂਚਕਾਂਕ(db, pTab->pSchema);
    ਪ੍ਰਤਿਜ੍ਞਾ( pParse->isCreate );
    sqlite3NestedParse(pParse,
      "UPDATE %Q." LEGACY_SCHEMA_ਸਾਰਣੀ " "
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

    sqlite3_Vdbe_ਜੋੜੋ_Op0(v, OP_Expire);
    zWhere = sqlite3MPrintf(db, "name=%Q AND sql=%Q", pTab->zName, zStmt);
    sqlite3_Vdbe_ਜੋੜੋ_Parse_Schema_Op(v, iDb, zWhere, 0);
    sqlite3DbFree(db, zStmt);

    iReg = ++pParse->nMem;
    sqlite3_Vdbe_ਲੱਦੋ_String(v, iReg, pTab->zName);
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_VCreate, iDb, iReg);
  }ਨਹੀਂਤਾਂ{
    /* If we are rereading the sqlite_schema table create the in-memory
    ** record of the table. */
    ਸਾਰਣੀ *pOld;
    Schema *pSchema = pTab->pSchema;
    ਅਚਲ ਅੱਖਰ *zName = pTab->zName;
    ਪ੍ਰਤਿਜ੍ਞਾ( zName!=0 );
    sqlite3_Mark_All_Shadow_Tables_ਦਾ(db, pTab);
    pOld = sqlite3_Hash_ਪਾਓ(&pSchema->tblHash, zName, pTab);
    ਜੇ( pOld ){
      sqlite3OomFault(db);
      ਪ੍ਰਤਿਜ੍ਞਾ( pTab==pOld );  /* Malloc must have failed inside HashInsert() */
      ਵਾਪਸ;
    }
    pParse->p_New_ਸਾਰਣੀ = 0;
  }
}

/*
** The parser calls this routine when it sees the first token
** of an argument to the module name in a CREATE VIRTUAL TABLE statement.
*/
ਖਾਲੀ sqlite3VtabArgInit(Parse *pParse){
  ਜੋੜੋ_ਦਲੀਲ_ਮੰਜ਼ਿਲ_Vtab(pParse);
  pParse->sArg.z = 0;
  pParse->sArg.n = 0;
}

/*
** The parser calls this routine for each token after the first token
** in an argument to the module name in a CREATE VIRTUAL TABLE statement.
*/
ਖਾਲੀ sqlite3VtabArgExtend(Parse *pParse, Token *p){
  Token *pArg = &pParse->sArg;
  ਜੇ( pArg->z==0 ){
    pArg->z = p->z;
    pArg->n = p->n;
  }ਨਹੀਂਤਾਂ{
    ਪ੍ਰਤਿਜ੍ਞਾ(pArg->z <= p->z);
    pArg->n = (ਪੂਰਨ)(&p->z[p->n] - pArg->z);
  }
}

/*
** Invoke a virtual table constructor (either xCreate or xConnect). The
** pointer to the function to invoke is passed as the fourth parameter
** to this procedure.
*/
ਸਥਿਰ ਪੂਰਨ vtabCallConstructor(
  sqlite3 *db,
  ਸਾਰਣੀ *pTab,
  Module *pMod,
  ਪੂਰਨ (*xConstruct)(sqlite3*,ਖਾਲੀ*,ਪੂਰਨ,ਅਚਲ ਅੱਖਰ*ਅਚਲ*,sqlite3_vtab**,ਅੱਖਰ**),
  ਅੱਖਰ **pzErr
){
  VtabCtx sCtx;
  VTable *pVTable;
  ਪੂਰਨ rc;
  ਅਚਲ ਅੱਖਰ *ਅਚਲ*azArg;
  ਪੂਰਨ nArg = pTab->u.vtab.nArg;
  ਅੱਖਰ *zErr = 0;
  ਅੱਖਰ *zModuleName;
  ਪੂਰਨ iDb;
  VtabCtx *pCtx;

  ਪ੍ਰਤਿਜ੍ਞਾ( IsVirtual(pTab) );
  azArg = (ਅਚਲ ਅੱਖਰ *ਅਚਲ*)pTab->u.vtab.azArg;

  /* Check that the virtual-table is not already being initialized */
  ਲਈ(pCtx=db->pVtabCtx; pCtx; pCtx=pCtx->p_ਪੂਰਵ){
    ਜੇ( pCtx->pTab==pTab ){
      *pzErr = sqlite3MPrintf(db,
          "vtable constructor called recursively: %s", pTab->zName
      );
      ਵਾਪਸ SQLITE_LOCKED;
    }
  }

  zModuleName = sqlite3DbStrDup(db, pTab->zName);
  ਜੇ( !zModuleName ){
    ਵਾਪਸ SQLITE_NOMEM_BKPT;
  }

  pVTable = sqlite3MallocZero(ਆਕਾਰ(VTable));
  ਜੇ( !pVTable ){
    sqlite3OomFault(db);
    sqlite3DbFree(db, zModuleName);
    ਵਾਪਸ SQLITE_NOMEM_BKPT;
  }
  pVTable->db = db;
  pVTable->pMod = pMod;
  pVTable->eVtabRisk = SQLITE_VTABRISK_ਸਾਧਾਰਨ;

  iDb = sqlite3_Schema_ਮੰਜ਼ਿਲ_ਸੂਚਕਾਂਕ(db, pTab->pSchema);
  pTab->u.vtab.azArg[1] = db->aDb[iDb].zDbSName;

  /* Invoke the virtual table constructor */
  ਪ੍ਰਤਿਜ੍ਞਾ( &db->pVtabCtx );
  ਪ੍ਰਤਿਜ੍ਞਾ( xConstruct );
  sCtx.pTab = pTab;
  sCtx.pVTable = pVTable;
  sCtx.p_ਪੂਰਵ = db->pVtabCtx;
  sCtx.bDeclared = 0;
  db->pVtabCtx = &sCtx;
  pTab->nTabRef++;
  rc = xConstruct(db, pMod->pAux, nArg, azArg, &pVTable->pVtab, &zErr);
  ਪ੍ਰਤਿਜ੍ਞਾ( pTab!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pTab->nTabRef>1 || rc!=SQLITE_OK );
  sqlite3_Delete_ਸਾਰਣੀ(db, pTab);
  db->pVtabCtx = sCtx.p_ਪੂਰਵ;
  ਜੇ( rc==SQLITE_NOMEM ) sqlite3OomFault(db);
  ਪ੍ਰਤਿਜ੍ਞਾ( sCtx.pTab==pTab );

  ਜੇ( SQLITE_OK!=rc ){
    ਜੇ( zErr==0 ){
      *pzErr = sqlite3MPrintf(db, "vtable constructor failed: %s", zModuleName);
    }ਨਹੀਂਤਾਂ {
      *pzErr = sqlite3MPrintf(db, "%s", zErr);
      sqlite3_free(zErr);
    }
    sqlite3DbFree(db, pVTable);
  }ਨਹੀਂਤਾਂ ਜੇ( ALWAYS(pVTable->pVtab) ){
    /* Justification of ALWAYS():  A correct vtab constructor must allocate
    ** the sqlite3_vtab object if successful.  */
    memset(pVTable->pVtab, 0, ਆਕਾਰ(pVTable->pVtab[0]));
    pVTable->pVtab->pModule = pMod->pModule;
    pMod->nRefModule++;
    pVTable->nRef = 1;
    ਜੇ( sCtx.bDeclared==0 ){
      ਅਚਲ ਅੱਖਰ *zFormat = "vtable constructor did not declare schema: %s";
      *pzErr = sqlite3MPrintf(db, zFormat, zModuleName);
      sqlite3VtabUnlock(pVTable);
      rc = SQLITE_ERROR;
    }ਨਹੀਂਤਾਂ{
      ਪੂਰਨ i_ਖਾਨਾ;
      u16 oooHidden = 0;
      /* If everything went according to plan, link the new VTable structure
      ** into the linked list headed by pTab->u.vtab.p. Then loop through the
      ** columns of the table to see if any of them contain the token "hidden".
      ** If so, set the Column COLFLAG_HIDDEN flag and remove the token from
      ** the type string.  */
      pVTable->p_ਅਗਲਾ = pTab->u.vtab.p;
      pTab->u.vtab.p = pVTable;

      ਲਈ(i_ਖਾਨਾ=0; i_ਖਾਨਾ<pTab->n_ਖਾਨਾ; i_ਖਾਨਾ++){
        ਅੱਖਰ *zType = sqlite3_ਕਾਲਮ_Type(&pTab->a_ਖਾਨਾ[i_ਖਾਨਾ], "");
        ਪੂਰਨ nType;
        ਪੂਰਨ i = 0;
        nType = sqlite3Strlen30(zType);
        ਲਈ(i=0; i<nType; i++){
          ਜੇ( 0==sqlite3StrNICmp("hidden", &zType[i], 6)
           && (i==0 || zType[i-1]==' ')
           && (zType[i+6]=='\0' || zType[i+6]==' ')
          ){
            ਰੋਕੋ;
          }
        }
        ਜੇ( i<nType ){
          ਪੂਰਨ j;
          ਪੂਰਨ nDel = 6 + (zType[i+6] ? 1 : 0);
          ਲਈ(j=i; (j+nDel)<=nType; j++){
            zType[j] = zType[j+nDel];
          }
          ਜੇ( zType[i]=='\0' && i>0 ){
            ਪ੍ਰਤਿਜ੍ਞਾ(zType[i-1]==' ');
            zType[i-1] = '\0';
          }
          pTab->a_ਖਾਨਾ[i_ਖਾਨਾ].ਖਾਨਾ_Flags |= COLFLAG_HIDDEN;
          pTab->tabFlags |= TF_HasHidden;
          oooHidden = TF_OOOHidden;
        }ਨਹੀਂਤਾਂ{
          pTab->tabFlags |= oooHidden;
        }
      }
    }
  }

  sqlite3DbFree(db, zModuleName);
  ਵਾਪਸ rc;
}

/*
** This function is invoked by the parser to call the xConnect() method
** of the virtual table pTab. If an error occurs, an error code is returned
** and an error left in pParse.
**
** This call is a no-op if table pTab is not a virtual table.
*/
ਪੂਰਨ sqlite3VtabCallConnect(Parse *pParse, ਸਾਰਣੀ *pTab){
  sqlite3 *db = pParse->db;
  ਅਚਲ ਅੱਖਰ *zMod;
  Module *pMod;
  ਪੂਰਨ rc;

  ਪ੍ਰਤਿਜ੍ਞਾ( pTab );
  ਪ੍ਰਤਿਜ੍ਞਾ( IsVirtual(pTab) );
  ਜੇ( sqlite3GetVTable(db, pTab) ){
    ਵਾਪਸ SQLITE_OK;
  }

  /* Locate the required virtual table module */
  zMod = pTab->u.vtab.azArg[0];
  pMod = (Module*)sqlite3_Hash_ਲੱਭੋ(&db->aModule, zMod);

  ਜੇ( !pMod ){
    ਅਚਲ ਅੱਖਰ *zModule = pTab->u.vtab.azArg[0];
    sqlite3_Error_ਸੁਨੇਹਾ(pParse, "no such module: %s", zModule);
    rc = SQLITE_ERROR;
  }ਨਹੀਂਤਾਂ{
    ਅੱਖਰ *zErr = 0;
    rc = vtabCallConstructor(db, pTab, pMod, pMod->pModule->xConnect, &zErr);
    ਜੇ( rc!=SQLITE_OK ){
      sqlite3_Error_ਸੁਨੇਹਾ(pParse, "%s", zErr);
      pParse->rc = rc;
    }ਨਹੀਂਤਾਂ{
      sqlite3_Mark_All_Shadow_Tables_ਦਾ(db, pTab);
    }
    sqlite3DbFree(db, zErr);
  }

  ਵਾਪਸ rc;
}
/*
** Grow the db->aVTrans[] array so that there is room for at least one
** more v-table. Return SQLITE_NOMEM if a malloc fails, or SQLITE_OK otherwise.
*/
ਸਥਿਰ ਪੂਰਨ growVTrans(sqlite3 *db){
  ਅਚਲ ਪੂਰਨ ARRAY_INCR = 5;

  /* Grow the sqlite3.aVTrans array if required */
  ਜੇ( (db->nVTrans%ARRAY_INCR)==0 ){
    VTable **aVTrans;
    sqlite3_int64 n_ਬਾਈਟਾਂ = ਆਕਾਰ(sqlite3_vtab*)*
                                 ((sqlite3_int64)db->nVTrans + ARRAY_INCR);
    aVTrans = sqlite3DbRealloc(db, (ਖਾਲੀ *)db->aVTrans, n_ਬਾਈਟਾਂ);
    ਜੇ( !aVTrans ){
      ਵਾਪਸ SQLITE_NOMEM_BKPT;
    }
    memset(&aVTrans[db->nVTrans], 0, ਆਕਾਰ(sqlite3_vtab *)*ARRAY_INCR);
    db->aVTrans = aVTrans;
  }

  ਵਾਪਸ SQLITE_OK;
}

/*
** Add the virtual table pVTab to the array sqlite3.aVTrans[]. Space should
** have already been reserved using growVTrans().
*/
ਸਥਿਰ ਖਾਲੀ ਜੋੜੋ_ਮੰਜ਼ਿਲ_VTrans(sqlite3 *db, VTable *pVTab){
  /* Add pVtab to the end of sqlite3.aVTrans */
  db->aVTrans[db->nVTrans++] = pVTab;
  sqlite3_Vtab_ਤਾਲਾ(pVTab);
}

/*
** This function is invoked by the vdbe to call the xCreate method
** of the virtual table named zTab in database iDb.
**
** If an error occurs, *pzErr is set to point to an English language
** description of the error and an SQLITE_XXX error code is returned.
** In this case the caller must call sqlite3DbFree(db, ) on *pzErr.
*/
ਪੂਰਨ sqlite3VtabCallCreate(sqlite3 *db, ਪੂਰਨ iDb, ਅਚਲ ਅੱਖਰ *zTab, ਅੱਖਰ **pzErr){
  ਪੂਰਨ rc = SQLITE_OK;
  ਸਾਰਣੀ *pTab;
  Module *pMod;
  ਅਚਲ ਅੱਖਰ *zMod;

  pTab = sqlite3_ਲੱਭੋ_ਸਾਰਣੀ(db, zTab, db->aDb[iDb].zDbSName);
  ਪ੍ਰਤਿਜ੍ਞਾ( pTab && IsVirtual(pTab) && !pTab->u.vtab.p );

  /* Locate the required virtual table module */
  zMod = pTab->u.vtab.azArg[0];
  pMod = (Module*)sqlite3_Hash_ਲੱਭੋ(&db->aModule, zMod);

  /* If the module has been registered and includes a Create method,
  ** invoke it now. If the module has not been registered, return an
  ** error. Otherwise, do nothing.
  */
  ਜੇ( pMod==0 || pMod->pModule->xCreate==0 || pMod->pModule->xDestroy==0 ){
    *pzErr = sqlite3MPrintf(db, "no such module: %s", zMod);
    rc = SQLITE_ERROR;
  }ਨਹੀਂਤਾਂ{
    rc = vtabCallConstructor(db, pTab, pMod, pMod->pModule->xCreate, pzErr);
  }

  /* Justification of ALWAYS():  The xConstructor method is required to
  ** create a valid sqlite3_vtab if it returns SQLITE_OK. */
  ਜੇ( rc==SQLITE_OK && ALWAYS(sqlite3GetVTable(db, pTab)) ){
    rc = growVTrans(db);
    ਜੇ( rc==SQLITE_OK ){
      VTable *pVTab = sqlite3GetVTable(db, pTab);
      ਅਚਲ sqlite3_module *pModule = pMod->pModule;
      ਪੂਰਨ iSvpt = db->nStatement + db->nSavepoint;
      ਜੋੜੋ_ਮੰਜ਼ਿਲ_VTrans(db, pVTab);
      /* As in sqlite3VtabBegin(): open the current savepoint on the new
      ** table, so that a ROLLBACK TO that undoes the CREATE also invokes
      ** its xRollbackTo method. */
      ਜੇ( iSvpt && pModule->i_ਸੰਸਕਰਣ>=2 && pModule->xSavepoint ){
        pVTab->iSavepoint = iSvpt;
        rc = pModule->xSavepoint(pVTab->pVtab, iSvpt-1);
      }
    }
  }

  ਵਾਪਸ rc;
}

/*
** This function is used to set the schema of a virtual table.  It is only
** valid to call this function from within the xCreate() or xConnect() of a
** virtual table module.
*/
ਪੂਰਨ sqlite3_declare_vtab(sqlite3 *db, ਅਚਲ ਅੱਖਰ *z_Create_ਸਾਰਣੀ){
  VtabCtx *pCtx;
  ਪੂਰਨ rc = SQLITE_OK;
  ਸਾਰਣੀ *pTab;
  Parse sParse;
  ਪੂਰਨ initBusy;
  ਪੂਰਨ i;
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *z;
  ਸਥਿਰ ਅਚਲ u8 aKeyword[] = { TK_CREATE, TK_ਸਾਰਣੀ, 0 };

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !sqlite3SafetyCheckOk(db) || z_Create_ਸਾਰਣੀ==0 ){
    ਵਾਪਸ SQLITE_MISUSE_BKPT;
  }
#ਜੇ_ਅੰਤ

  /* Verify that the first two keywords in the CREATE TABLE statement
  ** really are "CREATE" and "TABLE".  If this is not the case, then
  ** sqlite3_declare_vtab() is being misused.
  */
  z = (ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ*)z_Create_ਸਾਰਣੀ;
  ਲਈ(i=0; aKeyword[i]; i++){
    ਪੂਰਨ tokenType = 0;
    ਕਰੋ{
      z += sqlite3GetToken(z, &tokenType);
    }ਜਦੋਂ( tokenType==TK_SPACE || tokenType==TK_COMMENT );
    ਜੇ( tokenType!=aKeyword[i] ){
      sqlite3_Error_With_ਸੁਨੇਹਾ(db, SQLITE_ERROR, "syntax error");
      ਵਾਪਸ SQLITE_ERROR;
    }
  }
 
  sqlite3_mutex_enter(db->mutex);
  pCtx = db->pVtabCtx;
  ਜੇ( !pCtx || pCtx->bDeclared ){
    sqlite3Error(db, SQLITE_MISUSE_BKPT);
    sqlite3_mutex_leave(db->mutex);
    ਵਾਪਸ SQLITE_MISUSE_BKPT;
  }

  pTab = pCtx->pTab;
  ਪ੍ਰਤਿਜ੍ਞਾ( IsVirtual(pTab) );

  sqlite3ParseObjectInit(&sParse, db);
  sParse.e_Parse_ਬਹੁਲਕ = PARSE_ਬਹੁਲਕ_DECLARE_VTAB;
  sParse.disableTriggers = 1;
  /* We should never be able to reach this point while loading the
  ** schema.  Nevertheless, defend against that (turn off db->init.busy)
  ** in case a bug arises. */
  ਪ੍ਰਤਿਜ੍ਞਾ( db->init.busy==0 );
  initBusy = db->init.busy;
  db->init.busy = 0;
  sParse.nQueryLoop = 1;
  ਜੇ( SQLITE_OK==sqlite3_ਲੜੀਵਾਰ_Parser(&sParse, z_Create_ਸਾਰਣੀ) ){
    ਪ੍ਰਤਿਜ੍ਞਾ( sParse.p_New_ਸਾਰਣੀ!=0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( !db->mallocFailed );
    ਪ੍ਰਤਿਜ੍ਞਾ( Is_Ordinary_ਸਾਰਣੀ(sParse.p_New_ਸਾਰਣੀ) );
    ਪ੍ਰਤਿਜ੍ਞਾ( sParse.z_Err_ਸੁਨੇਹਾ==0 );
    ਜੇ( !pTab->a_ਖਾਨਾ ){
      ਸਾਰਣੀ *pNew = sParse.p_New_ਸਾਰਣੀ;
      ਸੂਚਕਾਂਕ *pIdx;
      pTab->a_ਖਾਨਾ = pNew->a_ਖਾਨਾ;
      ਪ੍ਰਤਿਜ੍ਞਾ( Is_Ordinary_ਸਾਰਣੀ(pNew) );
      sqlite3_ਵਿਅੰਜਕ_List_Delete(db, pNew->u.tab.pDfltList);
      pTab->nNVCol = pTab->n_ਖਾਨਾ = pNew->n_ਖਾਨਾ;
      pTab->tabFlags |= pNew->tabFlags & (TF_WithoutRowid|TF_NoVisibleRowid);
      pNew->n_ਖਾਨਾ = 0;
      pNew->a_ਖਾਨਾ = 0;
      ਪ੍ਰਤਿਜ੍ਞਾ( pTab->p_ਸੂਚਕਾਂਕ==0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( HasRowid(pNew) || sqlite3_Primary_ਕੁੰਜੀ_ਸੂਚਕਾਂਕ(pNew)!=0 );
      ਜੇ( !HasRowid(pNew)
       && pCtx->pVTable->pMod->pModule->xUpdate!=0
       && sqlite3_Primary_ਕੁੰਜੀ_ਸੂਚਕਾਂਕ(pNew)->n_ਕੁੰਜੀ_ਖਾਨਾ!=1
      ){
        /* WITHOUT ROWID virtual tables must either be read-only (xUpdate==0)
        ** or else must have a single-column PRIMARY KEY */
        rc = SQLITE_ERROR;
      }
      pIdx = pNew->p_ਸੂਚਕਾਂਕ;
      ਜੇ( pIdx ){
        ਪ੍ਰਤਿਜ੍ਞਾ( pIdx->p_ਅਗਲਾ==0 );
        pTab->p_ਸੂਚਕਾਂਕ = pIdx;
        pNew->p_ਸੂਚਕਾਂਕ = 0;
        pIdx->p_ਸਾਰਣੀ = pTab;
      }
    }
    pCtx->bDeclared = 1;
  }ਨਹੀਂਤਾਂ{
    sqlite3_Error_With_ਸੁਨੇਹਾ(db, SQLITE_ERROR,
          (sParse.z_Err_ਸੁਨੇਹਾ ? "%s" : 0), sParse.z_Err_ਸੁਨੇਹਾ);
    sqlite3DbFree(db, sParse.z_Err_ਸੁਨੇਹਾ);
    rc = SQLITE_ERROR;
  }
  sParse.e_Parse_ਬਹੁਲਕ = PARSE_ਬਹੁਲਕ_ਸਾਧਾਰਨ;

  ਜੇ( sParse.pVdbe ){
    sqlite3VdbeFinalize(sParse.pVdbe);
  }
  sqlite3_Delete_ਸਾਰਣੀ(db, sParse.p_New_ਸਾਰਣੀ);
  sqlite3ParseObjectReset(&sParse);
  db->init.busy = initBusy;

  ਪ੍ਰਤਿਜ੍ਞਾ( (rc&0xff)==rc );
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  ਵਾਪਸ rc;
}

/*
** This function is invoked by the vdbe to call the xDestroy method
** of the virtual table named zTab in database iDb. This occurs
** when a DROP TABLE is mentioned.
**
** This call is a no-op if zTab is not a virtual table.
*/
ਪੂਰਨ sqlite3VtabCallDestroy(sqlite3 *db, ਪੂਰਨ iDb, ਅਚਲ ਅੱਖਰ *zTab){
  ਪੂਰਨ rc = SQLITE_OK;
  ਸਾਰਣੀ *pTab;

  pTab = sqlite3_ਲੱਭੋ_ਸਾਰਣੀ(db, zTab, db->aDb[iDb].zDbSName);
  ਜੇ( ALWAYS(pTab!=0)
   && ALWAYS(IsVirtual(pTab))
   && ALWAYS(pTab->u.vtab.p!=0)
  ){
    VTable *p;
    ਪੂਰਨ (*xDestroy)(sqlite3_vtab *);
    ਲਈ(p=pTab->u.vtab.p; p; p=p->p_ਅਗਲਾ){
      ਪ੍ਰਤਿਜ੍ਞਾ( p->pVtab );
      ਜੇ( p->pVtab->nRef>0 ){
        ਵਾਪਸ SQLITE_LOCKED;
      }
    }
    p = vtabDisconnectAll(db, pTab);
    xDestroy = p->pMod->pModule->xDestroy;
    ਜੇ( xDestroy==0 ) xDestroy = p->pMod->pModule->xDisconnect;
    ਪ੍ਰਤਿਜ੍ਞਾ( xDestroy!=0 );
    pTab->nTabRef++;
    rc = xDestroy(p->pVtab);
    /* Remove the sqlite3_vtab* from the aVTrans[] array, if applicable */
    ਜੇ( rc==SQLITE_OK ){
      ਪ੍ਰਤਿਜ੍ਞਾ( pTab->u.vtab.p==p && p->p_ਅਗਲਾ==0 );
      p->pVtab = 0;
      pTab->u.vtab.p = 0;
      sqlite3VtabUnlock(p);
    }
    sqlite3_Delete_ਸਾਰਣੀ(db, pTab);
  }

  ਵਾਪਸ rc;
}

/*
** This function invokes either the xRollback or xCommit method
** of each of the virtual tables in the sqlite3.aVTrans array. The method
** called is identified by the second argument, "offset", which is
** the offset of the method to call in the sqlite3_module structure.
**
** The array is cleared after invoking the callbacks.
*/
ਸਥਿਰ ਖਾਲੀ callFinaliser(sqlite3 *db, ਪੂਰਨ ਵਿੱਥ){
  ਪੂਰਨ i;
  ਜੇ( db->aVTrans ){
    VTable **aVTrans = db->aVTrans;
    db->aVTrans = 0;
    ਲਈ(i=0; i<db->nVTrans; i++){
      VTable *pVTab = aVTrans[i];
      sqlite3_vtab *p = pVTab->pVtab;
      ਜੇ( p ){
        ਪੂਰਨ (*x)(sqlite3_vtab *);
        x = *(ਪੂਰਨ (**)(sqlite3_vtab *))((ਅੱਖਰ *)p->pModule + ਵਿੱਥ);
        ਜੇ( x ) x(p);
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
ਪੂਰਨ sqlite3VtabSync(sqlite3 *db, Vdbe *p){
  ਪੂਰਨ i;
  ਪੂਰਨ rc = SQLITE_OK;
  VTable **aVTrans = db->aVTrans;

  db->aVTrans = 0;
  ਲਈ(i=0; rc==SQLITE_OK && i<db->nVTrans; i++){
    ਪੂਰਨ (*x)(sqlite3_vtab *);
    sqlite3_vtab *pVtab = aVTrans[i]->pVtab;
    ਜੇ( pVtab && (x = pVtab->pModule->xSync)!=0 ){
      rc = x(pVtab);
      sqlite3VtabImportErrmsg(p, pVtab);
    }
  }
  db->aVTrans = aVTrans;
  ਵਾਪਸ rc;
}

/*
** Invoke the xRollback method of all virtual tables in the
** sqlite3.aVTrans array. Then clear the array itself.
*/
ਪੂਰਨ sqlite3VtabRollback(sqlite3 *db){
  callFinaliser(db, offsetof(sqlite3_module,xRollback));
  ਵਾਪਸ SQLITE_OK;
}

/*
** Invoke the xCommit method of all virtual tables in the
** sqlite3.aVTrans array. Then clear the array itself.
*/
ਪੂਰਨ sqlite3VtabCommit(sqlite3 *db){
  callFinaliser(db, offsetof(sqlite3_module,xCommit));
  ਵਾਪਸ SQLITE_OK;
}

/*
** If the virtual table pVtab supports the transaction interface
** (xBegin/xRollback/xCommit and optionally xSync) and a transaction is
** not currently open, invoke the xBegin method now.
**
** If the xBegin call is successful, place the sqlite3_vtab pointer
** in the sqlite3.aVTrans array.
*/
ਪੂਰਨ sqlite3VtabBegin(sqlite3 *db, VTable *pVTab){
  ਪੂਰਨ rc = SQLITE_OK;
  ਅਚਲ sqlite3_module *pModule;

  /* Special case: If db->aVTrans is NULL and db->nVTrans is greater
  ** than zero, then this function is being called from within a
  ** virtual module xSync() callback. It is illegal to write to
  ** virtual module tables in this case, so return SQLITE_LOCKED.
  */
  ਜੇ( sqlite3_Vtab_ਵਿੱਚ_Sync(db) ){
    ਵਾਪਸ SQLITE_LOCKED;
  }
  ਜੇ( !pVTab ){
    ਵਾਪਸ SQLITE_OK;
  }
  pModule = pVTab->pVtab->pModule;

  ਜੇ( pModule->xBegin ){
    ਪੂਰਨ i;

    /* If pVtab is already in the aVTrans array, return early */
    ਲਈ(i=0; i<db->nVTrans; i++){
      ਜੇ( db->aVTrans[i]==pVTab ){
        ਵਾਪਸ SQLITE_OK;
      }
    }

    /* Invoke the xBegin method. If successful, add the vtab to the
    ** sqlite3.aVTrans[] array. */
    rc = growVTrans(db);
    ਜੇ( rc==SQLITE_OK ){
      rc = pModule->xBegin(pVTab->pVtab);
      ਜੇ( rc==SQLITE_OK ){
        ਪੂਰਨ iSvpt = db->nStatement + db->nSavepoint;
        ਜੋੜੋ_ਮੰਜ਼ਿਲ_VTrans(db, pVTab);
        ਜੇ( iSvpt && pModule->xSavepoint ){
          pVTab->iSavepoint = iSvpt;
          rc = pModule->xSavepoint(pVTab->pVtab, iSvpt-1);
        }
      }
    }
  }
  ਵਾਪਸ rc;
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
ਪੂਰਨ sqlite3VtabSavepoint(sqlite3 *db, ਪੂਰਨ op, ਪੂਰਨ iSavepoint){
  ਪੂਰਨ rc = SQLITE_OK;

  ਪ੍ਰਤਿਜ੍ਞਾ( op==SAVEPOINT_RELEASE||op==SAVEPOINT_ROLLBACK||op==SAVEPOINT_BEGIN );
  ਪ੍ਰਤਿਜ੍ਞਾ( iSavepoint>=-1 );
  ਜੇ( db->aVTrans ){
    ਪੂਰਨ i;
    ਲਈ(i=0; rc==SQLITE_OK && i<db->nVTrans; i++){
      VTable *pVTab = db->aVTrans[i];
      ਅਚਲ sqlite3_module *pMod = pVTab->pMod->pModule;
      ਜੇ( pVTab->pVtab && pMod->i_ਸੰਸਕਰਣ>=2 ){
        ਪੂਰਨ (*x_ਵਿਧੀ)(sqlite3_vtab *, ਪੂਰਨ);
        sqlite3_Vtab_ਤਾਲਾ(pVTab);
        ਚੋਣ( op ){
          ਹਾਲਤ SAVEPOINT_BEGIN:
            x_ਵਿਧੀ = pMod->xSavepoint;
            pVTab->iSavepoint = iSavepoint+1;
            ਰੋਕੋ;
          ਹਾਲਤ SAVEPOINT_ROLLBACK:
            x_ਵਿਧੀ = pMod->x_Rollback_ਮੰਜ਼ਿਲ;
            ਰੋਕੋ;
          ਮੂਲ:
            x_ਵਿਧੀ = pMod->xRelease;
            ਰੋਕੋ;
        }
        ਜੇ( x_ਵਿਧੀ && pVTab->iSavepoint>iSavepoint ){
          u64 savedFlags = (db->flags & SQLITE_Defensive);
          db->flags &= ~(u64)SQLITE_Defensive;
          rc = x_ਵਿਧੀ(pVTab->pVtab, iSavepoint);
          db->flags |= savedFlags;
        }
        sqlite3VtabUnlock(pVTab);
      }
    }
  }
  ਵਾਪਸ rc;
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
  ਪੂਰਨ nArg,       /* Number of arguments to the function */
  ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ     /* First argument to the function */
){
  ਸਾਰਣੀ *pTab;
  sqlite3_vtab *pVtab;
  sqlite3_module *pMod;
  ਖਾਲੀ (*xSFunc)(sqlite3_context*,ਪੂਰਨ,sqlite3_ਮੁੱਲ**) = 0;
  ਖਾਲੀ *pArg = 0;
  FuncDef *pNew;
  ਪੂਰਨ rc = 0;

  /* Check to see the left operand is a column in a virtual table */
  ਜੇ( NEVER(p_ਵਿਅੰਜਕ==0) ) ਵਾਪਸ pDef;
  ਜੇ( p_ਵਿਅੰਜਕ->op!=TK_ਕਾਲਮ ) ਵਾਪਸ pDef;
  ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_YTab(p_ਵਿਅੰਜਕ) );
  pTab = p_ਵਿਅੰਜਕ->y.pTab;
  ਜੇ( NEVER(pTab==0) ) ਵਾਪਸ pDef;
  ਜੇ( !IsVirtual(pTab) ) ਵਾਪਸ pDef;
  pVtab = sqlite3GetVTable(db, pTab)->pVtab;
  ਪ੍ਰਤਿਜ੍ਞਾ( pVtab!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pVtab->pModule!=0 );
  pMod = (sqlite3_module *)pVtab->pModule;
  ਜੇ( pMod->x_ਲੱਭੋ_Function==0 ) ਵਾਪਸ pDef;

  /* Call the xFindFunction method on the virtual table implementation
  ** to see if the implementation wants to overload this function.
  **
  ** Though undocumented, we have historically always invoked xFindFunction
  ** with an all lower-case function name.  Continue in this tradition to
  ** avoid any chance of an incompatibility.
  */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  {
    ਪੂਰਨ i;
    ਲਈ(i=0; pDef->zName[i]; i++){
      ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ x = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)pDef->zName[i];
      ਪ੍ਰਤਿਜ੍ਞਾ( x==sqlite3_ਉੱਤਲਾ_ਮੰਜ਼ਿਲ_ਹੇਠਲਾ[x] );
    }
  }
#ਜੇ_ਅੰਤ
  rc = pMod->x_ਲੱਭੋ_Function(pVtab, nArg, pDef->zName, &xSFunc, &pArg);
  ਜੇ( rc==0 ){
    ਵਾਪਸ pDef;
  }

  /* Create a new ephemeral function definition for the overloaded
  ** function */
  pNew = sqlite3DbMallocZero(db, ਆਕਾਰ(*pNew)
                             + sqlite3Strlen30(pDef->zName) + 1);
  ਜੇ( pNew==0 ){
    ਵਾਪਸ pDef;
  }
  *pNew = *pDef;
  pNew->zName = (ਅਚਲ ਅੱਖਰ*)&pNew[1];
  ਸਮ੍ਰਿਤੀ_ਨਕਲ((ਅੱਖਰ*)&pNew[1], pDef->zName, sqlite3Strlen30(pDef->zName)+1);
  pNew->xSFunc = xSFunc;
  pNew->p_User_ਅੰਕੜੇ = pArg;
  pNew->funcFlags |= SQLITE_FUNC_EPHEM;
  ਵਾਪਸ pNew;
}

/*
** Make sure virtual table pTab is contained in the pParse->apVirtualLock[]
** array so that an OP_VBegin will get generated for it.  Add pTab to the
** array if it is missing.  If pTab is already in the array, this routine
** is a no-op.
*/
ਖਾਲੀ sqlite3VtabMakeWritable(Parse *pParse, ਸਾਰਣੀ *pTab){
  Parse *pToplevel = sqlite3ParseToplevel(pParse);
  ਪੂਰਨ i, n;
  ਸਾਰਣੀ **ap_Vtab_ਤਾਲਾ;

  ਪ੍ਰਤਿਜ੍ਞਾ( IsVirtual(pTab) );
  ਲਈ(i=0; i<pToplevel->n_Vtab_ਤਾਲਾ; i++){
    ਜੇ( pTab==pToplevel->ap_Vtab_ਤਾਲਾ[i] ) ਵਾਪਸ;
  }
  n = (pToplevel->n_Vtab_ਤਾਲਾ+1)*ਆਕਾਰ(pToplevel->ap_Vtab_ਤਾਲਾ[0]);
  ap_Vtab_ਤਾਲਾ = sqlite3Realloc(pToplevel->ap_Vtab_ਤਾਲਾ, n);
  ਜੇ( ap_Vtab_ਤਾਲਾ ){
    pToplevel->ap_Vtab_ਤਾਲਾ = ap_Vtab_ਤਾਲਾ;
    pToplevel->ap_Vtab_ਤਾਲਾ[pToplevel->n_Vtab_ਤਾਲਾ++] = pTab;
  }ਨਹੀਂਤਾਂ{
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
ਪੂਰਨ sqlite3_Vtab_Eponymous_ਸਾਰਣੀ_Init(Parse *pParse, Module *pMod){
  ਅਚਲ sqlite3_module *pModule = pMod->pModule;
  ਸਾਰਣੀ *pTab;
  ਅੱਖਰ *zErr = 0;
  ਪੂਰਨ rc;
  sqlite3 *db = pParse->db;
  ਜੇ( pMod->pEpoTab ) ਵਾਪਸ 1;
  ਜੇ( pModule->xCreate!=0 && pModule->xCreate!=pModule->xConnect ) ਵਾਪਸ 0;
  pTab = sqlite3DbMallocZero(db, ਆਕਾਰ(ਸਾਰਣੀ));
  ਜੇ( pTab==0 ) ਵਾਪਸ 0;
  pTab->zName = sqlite3DbStrDup(db, pMod->zName);
  ਜੇ( pTab->zName==0 ){
    sqlite3DbFree(db, pTab);
    ਵਾਪਸ 0;
  }
  pMod->pEpoTab = pTab;
  pTab->nTabRef = 1;
  pTab->eTabType = TABTYP_VTAB;
  pTab->pSchema = db->aDb[0].pSchema;
  ਪ੍ਰਤਿਜ੍ਞਾ( pTab->u.vtab.nArg==0 );
  pTab->iPKey = -1;
  pTab->tabFlags |= TF_Eponymous;
  ਜੋੜੋ_Module_ਦਲੀਲ(pParse, pTab, sqlite3DbStrDup(db, pTab->zName));
  ਜੋੜੋ_Module_ਦਲੀਲ(pParse, pTab, 0);
  ਜੋੜੋ_Module_ਦਲੀਲ(pParse, pTab, sqlite3DbStrDup(db, pTab->zName));
  db->n_Schema_ਤਾਲਾ++;
  rc = vtabCallConstructor(db, pTab, pMod, pModule->xConnect, &zErr);
  db->n_Schema_ਤਾਲਾ--;
  ਜੇ( rc ){
    sqlite3_Error_ਸੁਨੇਹਾ(pParse, "%s", zErr);
    pParse->rc = rc;
    sqlite3DbFree(db, zErr);
    sqlite3_Vtab_Eponymous_ਸਾਰਣੀ_Clear(db, pMod);
  }
  ਵਾਪਸ 1;
}

/*
** Erase the eponymous virtual table instance associated with
** virtual table module pMod, if it exists.
*/
ਖਾਲੀ sqlite3_Vtab_Eponymous_ਸਾਰਣੀ_Clear(sqlite3 *db, Module *pMod){
  ਸਾਰਣੀ *pTab = pMod->pEpoTab;
  ਜੇ( pTab!=0 ){
    /* Mark the table as Ephemeral prior to deleting it, so that the
    ** sqlite3DeleteTable() routine will know that it is not stored in
    ** the schema. */
    pTab->tabFlags |= TF_Ephemeral;
    sqlite3_Delete_ਸਾਰਣੀ(db, pTab);
    pMod->pEpoTab = 0;
  }
}

/*
** Erase all eponymous virtual table instances associated with database
** handle db.  
*/
ਖਾਲੀ sqlite3_Vtab_Eponymous_ਸਾਰਣੀ_Clear_All(sqlite3 *db){
  HashElem *pElem;
  ਲਈ(pElem=sqlite_Hash_ਪਹਿਲਾ(&db->aModule); pElem; pElem=sqlite_Hash_ਅਗਲਾ(pElem)){
    sqlite3_Vtab_Eponymous_ਸਾਰਣੀ_Clear(db, (Module*)sqlite_Hash_ਅੰਕੜੇ(pElem));
  }
}

/*
** Return the ON CONFLICT resolution mode in effect for the virtual
** table update operation currently in progress.
**
** The results of this routine are undefined unless it is called from
** within an xUpdate method.
*/
ਪੂਰਨ sqlite3_vtab_on_conflict(sqlite3 *db){
  ਸਥਿਰ ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ aMap[] = {
    SQLITE_ROLLBACK, SQLITE_ABORT, SQLITE_FAIL, SQLITE_IGNORE, SQLITE_REPLACE
  };
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !sqlite3SafetyCheckOk(db) ) ਵਾਪਸ SQLITE_MISUSE_BKPT;
#ਜੇ_ਅੰਤ
  ਪ੍ਰਤਿਜ੍ਞਾ( OE_Rollback==1 && OE_Abort==2 && OE_Fail==3 );
  ਪ੍ਰਤਿਜ੍ਞਾ( OE_Ignore==4 && OE_Replace==5 );
  ਪ੍ਰਤਿਜ੍ਞਾ( db->vtabOnConflict>=1 && db->vtabOnConflict<=5 );
  ਵਾਪਸ (ਪੂਰਨ)aMap[db->vtabOnConflict-1];
}

/*
** Call from within the xCreate() or xConnect() methods to provide
** the SQLite core with additional information about the behavior
** of the virtual table being implemented.
*/
ਪੂਰਨ sqlite3_vtab_config(sqlite3 *db, ਪੂਰਨ op, ...){
  va_list ap;
  ਪੂਰਨ rc = SQLITE_OK;
  VtabCtx *p;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !sqlite3SafetyCheckOk(db) ) ਵਾਪਸ SQLITE_MISUSE_BKPT;
#ਜੇ_ਅੰਤ
  sqlite3_mutex_enter(db->mutex);
  p = db->pVtabCtx;
  ਜੇ( !p ){
    rc = SQLITE_MISUSE_BKPT;
  }ਨਹੀਂਤਾਂ{
    ਪ੍ਰਤਿਜ੍ਞਾ( p->pTab==0 || IsVirtual(p->pTab) );
    va_start(ap, op);
    ਚੋਣ( op ){
      ਹਾਲਤ SQLITE_VTAB_CONSTRAINT_SUPPORT: {
        p->pVTable->bConstraint = (u8)va_arg(ap, ਪੂਰਨ);
        ਰੋਕੋ;
      }
      ਹਾਲਤ SQLITE_VTAB_INNOCUOUS: {
        p->pVTable->eVtabRisk = SQLITE_VTABRISK_ਨੀਵਾਂ;
        ਰੋਕੋ;
      }
      ਹਾਲਤ SQLITE_VTAB_DIRECTONLY: {
        p->pVTable->eVtabRisk = SQLITE_VTABRISK_ਉੱਚਾ;
        ਰੋਕੋ;
      }
      ਹਾਲਤ SQLITE_VTAB_ਵਰਤਦਾ_ALL_SCHEMAS: {
        p->pVTable->bAllSchemas = 1;
        ਰੋਕੋ;
      }
      ਮੂਲ: {
        rc = SQLITE_MISUSE_BKPT;
        ਰੋਕੋ;
      }
    }
    va_end(ap);
  }

  ਜੇ( rc!=SQLITE_OK ) sqlite3Error(db, rc);
  sqlite3_mutex_leave(db->mutex);
  ਵਾਪਸ rc;
}

#ਜੇ_ਅੰਤ /* SQLITE_OMIT_VIRTUALTABLE */
