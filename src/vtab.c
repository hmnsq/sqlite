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
#અન્તર્ભાવ "sqliteInt.h"

/*
** Before a virtual table xCreate() or xConnect() method is invoked, the
** sqlite3.pVtabCtx member variable is set to point to an instance of
** this struct allocated on the stack. It is used by the implementation of
** the sqlite3_declare_vtab() and sqlite3_vtab_config() APIs, both of which
** are invoked only from within xCreate and xConnect methods.
*/
સંરચના VtabCtx {
  VTable *pVTable;    /* The virtual table being constructed */
  Table *pTab;        /* The Table object to which the virtual table belongs */
  VtabCtx *pPrior;    /* Parent context (if any) */
  પૂર્ણાંક bDeclared;      /* True after sqlite3_declare_vtab() is called */
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
  અચલ અક્ષર *zName,              /* Name assigned to this module */
  અચલ sqlite3_module *pModule,  /* The definition of the module */
  શૂન્ય *pAux,                     /* Context pointer for xCreate/xConnect */
  શૂન્ય (*xDestroy)(શૂન્ય *)        /* Module destructor function */
){
  Module *pMod;
  Module *pDel;
  અક્ષર *zCopy;
  જો( pModule==0 ){
    zCopy = (અક્ષર*)zName;
    pMod = 0;
  }નહીંતર{
    પૂર્ણાંક nName = sqlite3Strlen30(zName);
    pMod = (Module *)sqlite3Malloc(કદ(Module) + nName + 1);
    જો( pMod==0 ){
      sqlite3OomFault(db);
      પાછા 0;
    }
    zCopy = (અક્ષર *)(&pMod[1]);
    memcpy(zCopy, zName, nName+1);
    pMod->zName = zCopy;
    pMod->pModule = pModule;
    pMod->pAux = pAux;
    pMod->xDestroy = xDestroy;
    pMod->pEpoTab = 0;
    pMod->nRefModule = 1;
  }
  pDel = (Module *)sqlite3_Hash_ઉમેરો(&db->aModule,zCopy,(શૂન્ય*)pMod);
  જો( pDel ){
    જો( pDel==pMod ){
      sqlite3OomFault(db);
      sqlite3DbFree(db, pDel);
      pMod = 0;
    }નહીંતર{
      sqlite3VtabEponymousTableClear(db, pDel);
      sqlite3VtabModuleUnref(db, pDel);
    }
  }
  પાછા pMod;
}

/*
** The actual function that does the work of creating a new module.
** This function implements the sqlite3_create_module() and
** sqlite3_create_module_v2() interfaces.
*/
સ્થિર પૂર્ણાંક createModule(
  sqlite3 *db,                    /* Database in which module is registered */
  અચલ અક્ષર *zName,              /* Name assigned to this module */
  અચલ sqlite3_module *pModule,  /* The definition of the module */
  શૂન્ય *pAux,                     /* Context pointer for xCreate/xConnect */
  શૂન્ય (*xDestroy)(શૂન્ય *)        /* Module destructor function */
){
  પૂર્ણાંક rc = SQLITE_OK;

  sqlite3_mutex_enter(db->mutex);
  (શૂન્ય)sqlite3VtabCreateModule(db, zName, pModule, pAux, xDestroy);
  rc = sqlite3ApiExit(db, rc);
  જો( rc!=SQLITE_OK && xDestroy ) xDestroy(pAux);
  sqlite3_mutex_leave(db->mutex);
  પાછા rc;
}


/*
** External API function used to create a new virtual-table module.
*/
પૂર્ણાંક sqlite3_create_module(
  sqlite3 *db,                    /* Database in which module is registered */
  અચલ અક્ષર *zName,              /* Name assigned to this module */
  અચલ sqlite3_module *pModule,  /* The definition of the module */
  શૂન્ય *pAux                      /* Context pointer for xCreate/xConnect */
){
#ifdef SQLITE_ENABLE_API_ARMOR
  જો( !sqlite3SafetyCheckOk(db) || zName==0 ) પાછા SQLITE_MISUSE_BKPT;
#endif
  પાછા createModule(db, zName, pModule, pAux, 0);
}

/*
** External API function used to create a new virtual-table module.
*/
પૂર્ણાંક sqlite3_create_module_v2(
  sqlite3 *db,                    /* Database in which module is registered */
  અચલ અક્ષર *zName,              /* Name assigned to this module */
  અચલ sqlite3_module *pModule,  /* The definition of the module */
  શૂન્ય *pAux,                     /* Context pointer for xCreate/xConnect */
  શૂન્ય (*xDestroy)(શૂન્ય *)        /* Module destructor function */
){
#ifdef SQLITE_ENABLE_API_ARMOR
  જો( !sqlite3SafetyCheckOk(db) || zName==0 ) પાછા SQLITE_MISUSE_BKPT;
#endif
  પાછા createModule(db, zName, pModule, pAux, xDestroy);
}

/*
** External API to drop all virtual-table modules, except those named
** on the azNames list.
*/
પૂર્ણાંક sqlite3_drop_modules(sqlite3 *db, અચલ અક્ષર** azNames){
  HashElem *pThis, *pNext;
#ifdef SQLITE_ENABLE_API_ARMOR
  જો( !sqlite3SafetyCheckOk(db) ) પાછા SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(db->mutex);
  માટે(pThis=sqliteHashFirst(&db->aModule); pThis; pThis=pNext){
    Module *pMod = (Module*)sqliteHashData(pThis);
    pNext = sqliteHashNext(pThis);
    જો( azNames ){
      પૂર્ણાંક ii;
      માટે(ii=0; azNames[ii]!=0 && strcmp(azNames[ii],pMod->zName)!=0; ii++){}
      જો( azNames[ii]!=0 ) ચાલુ;
    }
    createModule(db, pMod->zName, 0, 0, 0);
  }
  sqlite3_mutex_leave(db->mutex);
  પાછા SQLITE_OK;
}

/*
** Decrement the reference count on a Module object.  Destroy the
** module when the reference count reaches zero.
*/
શૂન્ય sqlite3VtabModuleUnref(sqlite3 *db, Module *pMod){
  પ્રતિજ્ઞા( pMod->nRefModule>0 );
  pMod->nRefModule--;
  જો( pMod->nRefModule==0 ){
    જો( pMod->xDestroy ){
      pMod->xDestroy(pMod->pAux);
    }
    પ્રતિજ્ઞા( pMod->pEpoTab==0 );
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
શૂન્ય sqlite3VtabLock(VTable *pVTab){
  pVTab->nRef++;
}


/*
** pTab is a pointer to a Table structure representing a virtual-table.
** Return a pointer to the VTable object used by connection db to access
** this virtual-table, if one has been created, or NULL otherwise.
*/
VTable *sqlite3GetVTable(sqlite3 *db, Table *pTab){
  VTable *pVtab;
  પ્રતિજ્ઞા( IsVirtual(pTab) );
  માટે(pVtab=pTab->u.vtab.p; pVtab && pVtab->db!=db; pVtab=pVtab->pNext);
  પાછા pVtab;
}

/*
** Decrement the ref-count on a virtual table object. When the ref-count
** reaches zero, call the xDisconnect() method to delete the object.
*/
શૂન્ય sqlite3VtabUnlock(VTable *pVTab){
  sqlite3 *db = pVTab->db;

  પ્રતિજ્ઞા( db );
  પ્રતિજ્ઞા( pVTab->nRef>0 );
  પ્રતિજ્ઞા( db->eOpenState==SQLITE_STATE_OPEN
       || db->eOpenState==SQLITE_STATE_ZOMBIE );

  pVTab->nRef--;
  જો( pVTab->nRef==0 ){
    sqlite3_vtab *p = pVTab->pVtab;
    જો( p ){
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
સ્થિર VTable *vtabDisconnectAll(sqlite3 *db, Table *p){
  VTable *pRet = 0;
  VTable *pVTable;

  પ્રતિજ્ઞા( IsVirtual(p) );
  pVTable = p->u.vtab.p;
  p->u.vtab.p = 0;

  /* Assert that the mutex (if any) associated with the BtShared database
  ** that contains table p is held by the caller. See header comments
  ** above function sqlite3VtabUnlockList() for an explanation of why
  ** this makes it safe to access the sqlite3.pDisconnect list of any
  ** database connection that may have an entry in the p->u.vtab.p list.
  */
  પ્રતિજ્ઞા( db==0 || sqlite3SchemaMutexHeld(db, 0, p->pSchema) );

  જ્યારે( pVTable ){
    sqlite3 *db2 = pVTable->db;
    VTable *pNext = pVTable->pNext;
    પ્રતિજ્ઞા( db2 );
    જો( db2==db ){
      pRet = pVTable;
      p->u.vtab.p = pRet;
      pRet->pNext = 0;
    }નહીંતર{
      pVTable->pNext = db2->pDisconnect;
      db2->pDisconnect = pVTable;
    }
    pVTable = pNext;
  }

  પ્રતિજ્ઞા( !db || pRet );
  પાછા pRet;
}

/*
** Table *p is a virtual table. This function removes the VTable object
** for table *p associated with database connection db from the linked
** list in p->pVTab. It also decrements the VTable ref count. This is
** used when closing database connection db to free all of its VTable
** objects without disturbing the rest of the Schema object (which may
** be being used by other shared-cache connections).
*/
શૂન્ય sqlite3VtabDisconnect(sqlite3 *db, Table *p){
  VTable **ppVTab;

  પ્રતિજ્ઞા( IsVirtual(p) );
  પ્રતિજ્ઞા( sqlite3BtreeHoldsAllMutexes(db) );
  પ્રતિજ્ઞા( sqlite3_mutex_held(db->mutex) );

  માટે(ppVTab=&p->u.vtab.p; *ppVTab; ppVTab=&(*ppVTab)->pNext){
    જો( (*ppVTab)->db==db  ){
      VTable *pVTab = *ppVTab;
      *ppVTab = pVTab->pNext;
      sqlite3VtabUnlock(pVTab);
      તોડો;
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
શૂન્ય sqlite3VtabUnlockList(sqlite3 *db){
  VTable *p = db->pDisconnect;

  પ્રતિજ્ઞા( sqlite3BtreeHoldsAllMutexes(db) );
  પ્રતિજ્ઞા( sqlite3_mutex_held(db->mutex) );

  જો( p ){
    db->pDisconnect = 0;
    કરો {
      VTable *pNext = p->pNext;
      sqlite3VtabUnlock(p);
      p = pNext;
    }જ્યારે( p );
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
શૂન્ય sqlite3VtabClear(sqlite3 *db, Table *p){
  પ્રતિજ્ઞા( IsVirtual(p) );
  પ્રતિજ્ઞા( db!=0 );
  જો( db->pnBytesFreed==0 ) vtabDisconnectAll(0, p);
  જો( p->u.vtab.azArg ){
    પૂર્ણાંક i;
    માટે(i=0; i<p->u.vtab.nArg; i++){
      જો( i!=1 ) sqlite3DbFree(db, p->u.vtab.azArg[i]);
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
સ્થિર શૂન્ય addModuleArgument(Parse *pParse, Table *pTable, અક્ષર *zArg){
  sqlite3_int64 nBytes;
  અક્ષર **azModuleArg;
  sqlite3 *db = pParse->db;

  પ્રતિજ્ઞા( IsVirtual(pTable) );
  nBytes = કદ(અક્ષર *)*(2+pTable->u.vtab.nArg);
  જો( pTable->u.vtab.nArg+3>=db->aLimit[SQLITE_LIMIT_COLUMN] ){
    sqlite3_Error_સંદેશ(pParse, "too many columns on %s", pTable->zName);
  }
  azModuleArg = sqlite3DbRealloc(db, pTable->u.vtab.azArg, nBytes);
  જો( azModuleArg==0 ){
    sqlite3DbFree(db, zArg);
  }નહીંતર{
    પૂર્ણાંક i = pTable->u.vtab.nArg++;
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
શૂન્ય sqlite3VtabBeginParse(
  Parse *pParse,        /* Parsing context */
  Token *pName1,        /* Name of new table, or database name */
  Token *pName2,        /* Name of new table or NULL */
  Token *pModuleName,   /* Name of the module for the virtual table */
  પૂર્ણાંક ifNotExists       /* No error if the table already exists */
){
  Table *pTable;        /* The new virtual table */
  sqlite3 *db;          /* Database connection */

  sqlite3StartTable(pParse, pName1, pName2, 0, 0, 1, ifNotExists);
  pTable = pParse->pNewTable;
  જો( pTable==0 ) પાછા;
  પ્રતિજ્ઞા( 0==pTable->pIndex );
  pTable->eTabType = TABTYP_VTAB;

  db = pParse->db;

  પ્રતિજ્ઞા( pTable->u.vtab.nArg==0 );
  addModuleArgument(pParse, pTable, sqlite3NameFromToken(db, pModuleName));
  addModuleArgument(pParse, pTable, 0);
  addModuleArgument(pParse, pTable, sqlite3DbStrDup(db, pTable->zName));
  પ્રતિજ્ઞા( (pParse->sNameToken.z==pName2->z && pName2->z!=0)
       || (pParse->sNameToken.z==pName1->z && pName2->z==0)
  );
  pParse->sNameToken.n = (પૂર્ણાંક)(
      &pModuleName->z[pModuleName->n] - pParse->sNameToken.z
  );

#ifndef SQLITE_OMIT_AUTHORIZATION
  /* Creating a virtual table invokes the authorization callback twice.
  ** The first invocation, to obtain permission to INSERT a row into the
  ** sqlite_schema table, has already been made by sqlite3StartTable().
  ** The second call, to obtain permission to create the table, is made now.
  */
  જો( pTable->u.vtab.azArg ){
    પૂર્ણાંક iDb = sqlite3SchemaToIndex(db, pTable->pSchema);
    પ્રતિજ્ઞા( iDb>=0 ); /* The database the table is being created in */
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
સ્થિર શૂન્ય addArgumentToVtab(Parse *pParse){
  જો( pParse->sArg.z && pParse->pNewTable ){
    અચલ અક્ષર *z = (અચલ અક્ષર*)pParse->sArg.z;
    પૂર્ણાંક n = pParse->sArg.n;
    sqlite3 *db = pParse->db;
    addModuleArgument(pParse, pParse->pNewTable, sqlite3DbStrNDup(db, z, n));
  }
}

/*
** The parser calls this routine after the CREATE VIRTUAL TABLE statement
** has been completely parsed.
*/
શૂન્ય sqlite3VtabFinishParse(Parse *pParse, Token *pEnd){
  Table *pTab = pParse->pNewTable;  /* The table being constructed */
  sqlite3 *db = pParse->db;         /* The database connection */

  જો( pTab==0 ) પાછા;
  પ્રતિજ્ઞા( IsVirtual(pTab) );
  addArgumentToVtab(pParse);
  pParse->sArg.z = 0;
  જો( pTab->u.vtab.nArg<1 ) પાછા;

  /* If the CREATE VIRTUAL TABLE statement is being entered for the
  ** first time (in other words if the virtual table is actually being
  ** created now instead of just being read out of sqlite_schema) then
  ** do additional initialization work and store the statement text
  ** in the sqlite_schema table.
  */
  જો( !db->init.busy ){
    અક્ષર *zStmt;
    અક્ષર *zWhere;
    પૂર્ણાંક iDb;
    પૂર્ણાંક iReg;
    Vdbe *v;

    sqlite3MayAbort(pParse);

    /* Compute the complete text of the CREATE VIRTUAL TABLE statement */
    જો( pEnd ){
      pParse->sNameToken.n = (પૂર્ણાંક)(pEnd->z - pParse->sNameToken.z) + pEnd->n;
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
    પ્રતિજ્ઞા( pParse->isCreate );
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
  }નહીંતર{
    /* If we are rereading the sqlite_schema table create the in-memory
    ** record of the table. */
    Table *pOld;
    Schema *pSchema = pTab->pSchema;
    અચલ અક્ષર *zName = pTab->zName;
    પ્રતિજ્ઞા( zName!=0 );
    sqlite3MarkAllShadowTablesOf(db, pTab);
    pOld = sqlite3_Hash_ઉમેરો(&pSchema->tblHash, zName, pTab);
    જો( pOld ){
      sqlite3OomFault(db);
      પ્રતિજ્ઞા( pTab==pOld );  /* Malloc must have failed inside HashInsert() */
      પાછા;
    }
    pParse->pNewTable = 0;
  }
}

/*
** The parser calls this routine when it sees the first token
** of an argument to the module name in a CREATE VIRTUAL TABLE statement.
*/
શૂન્ય sqlite3VtabArgInit(Parse *pParse){
  addArgumentToVtab(pParse);
  pParse->sArg.z = 0;
  pParse->sArg.n = 0;
}

/*
** The parser calls this routine for each token after the first token
** in an argument to the module name in a CREATE VIRTUAL TABLE statement.
*/
શૂન્ય sqlite3VtabArgExtend(Parse *pParse, Token *p){
  Token *pArg = &pParse->sArg;
  જો( pArg->z==0 ){
    pArg->z = p->z;
    pArg->n = p->n;
  }નહીંતર{
    પ્રતિજ્ઞા(pArg->z <= p->z);
    pArg->n = (પૂર્ણાંક)(&p->z[p->n] - pArg->z);
  }
}

/*
** Invoke a virtual table constructor (either xCreate or xConnect). The
** pointer to the function to invoke is passed as the fourth parameter
** to this procedure.
*/
સ્થિર પૂર્ણાંક vtabCallConstructor(
  sqlite3 *db,
  Table *pTab,
  Module *pMod,
  પૂર્ણાંક (*xConstruct)(sqlite3*,શૂન્ય*,પૂર્ણાંક,અચલ અક્ષર*અચલ*,sqlite3_vtab**,અક્ષર**),
  અક્ષર **pzErr
){
  VtabCtx sCtx;
  VTable *pVTable;
  પૂર્ણાંક rc;
  અચલ અક્ષર *અચલ*azArg;
  પૂર્ણાંક nArg = pTab->u.vtab.nArg;
  અક્ષર *zErr = 0;
  અક્ષર *zModuleName;
  પૂર્ણાંક iDb;
  VtabCtx *pCtx;

  પ્રતિજ્ઞા( IsVirtual(pTab) );
  azArg = (અચલ અક્ષર *અચલ*)pTab->u.vtab.azArg;

  /* Check that the virtual-table is not already being initialized */
  માટે(pCtx=db->pVtabCtx; pCtx; pCtx=pCtx->pPrior){
    જો( pCtx->pTab==pTab ){
      *pzErr = sqlite3MPrintf(db,
          "vtable constructor called recursively: %s", pTab->zName
      );
      પાછા SQLITE_LOCKED;
    }
  }

  zModuleName = sqlite3DbStrDup(db, pTab->zName);
  જો( !zModuleName ){
    પાછા SQLITE_NOMEM_BKPT;
  }

  pVTable = sqlite3MallocZero(કદ(VTable));
  જો( !pVTable ){
    sqlite3OomFault(db);
    sqlite3DbFree(db, zModuleName);
    પાછા SQLITE_NOMEM_BKPT;
  }
  pVTable->db = db;
  pVTable->pMod = pMod;
  pVTable->eVtabRisk = SQLITE_VTABRISK_Normal;

  iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
  pTab->u.vtab.azArg[1] = db->aDb[iDb].zDbSName;

  /* Invoke the virtual table constructor */
  પ્રતિજ્ઞા( &db->pVtabCtx );
  પ્રતિજ્ઞા( xConstruct );
  sCtx.pTab = pTab;
  sCtx.pVTable = pVTable;
  sCtx.pPrior = db->pVtabCtx;
  sCtx.bDeclared = 0;
  db->pVtabCtx = &sCtx;
  pTab->nTabRef++;
  rc = xConstruct(db, pMod->pAux, nArg, azArg, &pVTable->pVtab, &zErr);
  પ્રતિજ્ઞા( pTab!=0 );
  પ્રતિજ્ઞા( pTab->nTabRef>1 || rc!=SQLITE_OK );
  sqlite3DeleteTable(db, pTab);
  db->pVtabCtx = sCtx.pPrior;
  જો( rc==SQLITE_NOMEM ) sqlite3OomFault(db);
  પ્રતિજ્ઞા( sCtx.pTab==pTab );

  જો( SQLITE_OK!=rc ){
    જો( zErr==0 ){
      *pzErr = sqlite3MPrintf(db, "vtable constructor failed: %s", zModuleName);
    }નહીંતર {
      *pzErr = sqlite3MPrintf(db, "%s", zErr);
      sqlite3_free(zErr);
    }
    sqlite3DbFree(db, pVTable);
  }નહીંતર જો( ALWAYS(pVTable->pVtab) ){
    /* Justification of ALWAYS():  A correct vtab constructor must allocate
    ** the sqlite3_vtab object if successful.  */
    memset(pVTable->pVtab, 0, કદ(pVTable->pVtab[0]));
    pVTable->pVtab->pModule = pMod->pModule;
    pMod->nRefModule++;
    pVTable->nRef = 1;
    જો( sCtx.bDeclared==0 ){
      અચલ અક્ષર *zFormat = "vtable constructor did not declare schema: %s";
      *pzErr = sqlite3MPrintf(db, zFormat, zModuleName);
      sqlite3VtabUnlock(pVTable);
      rc = SQLITE_ERROR;
    }નહીંતર{
      પૂર્ણાંક iCol;
      u16 oooHidden = 0;
      /* If everything went according to plan, link the new VTable structure
      ** into the linked list headed by pTab->u.vtab.p. Then loop through the
      ** columns of the table to see if any of them contain the token "hidden".
      ** If so, set the Column COLFLAG_HIDDEN flag and remove the token from
      ** the type string.  */
      pVTable->pNext = pTab->u.vtab.p;
      pTab->u.vtab.p = pVTable;

      માટે(iCol=0; iCol<pTab->nCol; iCol++){
        અક્ષર *zType = sqlite3ColumnType(&pTab->aCol[iCol], "");
        પૂર્ણાંક nType;
        પૂર્ણાંક i = 0;
        nType = sqlite3Strlen30(zType);
        માટે(i=0; i<nType; i++){
          જો( 0==sqlite3StrNICmp("hidden", &zType[i], 6)
           && (i==0 || zType[i-1]==' ')
           && (zType[i+6]=='\0' || zType[i+6]==' ')
          ){
            તોડો;
          }
        }
        જો( i<nType ){
          પૂર્ણાંક j;
          પૂર્ણાંક nDel = 6 + (zType[i+6] ? 1 : 0);
          માટે(j=i; (j+nDel)<=nType; j++){
            zType[j] = zType[j+nDel];
          }
          જો( zType[i]=='\0' && i>0 ){
            પ્રતિજ્ઞા(zType[i-1]==' ');
            zType[i-1] = '\0';
          }
          pTab->aCol[iCol].colFlags |= COLFLAG_HIDDEN;
          pTab->tabFlags |= TF_HasHidden;
          oooHidden = TF_OOOHidden;
        }નહીંતર{
          pTab->tabFlags |= oooHidden;
        }
      }
    }
  }

  sqlite3DbFree(db, zModuleName);
  પાછા rc;
}

/*
** This function is invoked by the parser to call the xConnect() method
** of the virtual table pTab. If an error occurs, an error code is returned
** and an error left in pParse.
**
** This call is a no-op if table pTab is not a virtual table.
*/
પૂર્ણાંક sqlite3VtabCallConnect(Parse *pParse, Table *pTab){
  sqlite3 *db = pParse->db;
  અચલ અક્ષર *zMod;
  Module *pMod;
  પૂર્ણાંક rc;

  પ્રતિજ્ઞા( pTab );
  પ્રતિજ્ઞા( IsVirtual(pTab) );
  જો( sqlite3GetVTable(db, pTab) ){
    પાછા SQLITE_OK;
  }

  /* Locate the required virtual table module */
  zMod = pTab->u.vtab.azArg[0];
  pMod = (Module*)sqlite3HashFind(&db->aModule, zMod);

  જો( !pMod ){
    અચલ અક્ષર *zModule = pTab->u.vtab.azArg[0];
    sqlite3_Error_સંદેશ(pParse, "no such module: %s", zModule);
    rc = SQLITE_ERROR;
  }નહીંતર{
    અક્ષર *zErr = 0;
    rc = vtabCallConstructor(db, pTab, pMod, pMod->pModule->xConnect, &zErr);
    જો( rc!=SQLITE_OK ){
      sqlite3_Error_સંદેશ(pParse, "%s", zErr);
      pParse->rc = rc;
    }નહીંતર{
      sqlite3MarkAllShadowTablesOf(db, pTab);
    }
    sqlite3DbFree(db, zErr);
  }

  પાછા rc;
}
/*
** Grow the db->aVTrans[] array so that there is room for at least one
** more v-table. Return SQLITE_NOMEM if a malloc fails, or SQLITE_OK otherwise.
*/
સ્થિર પૂર્ણાંક growVTrans(sqlite3 *db){
  અચલ પૂર્ણાંક ARRAY_INCR = 5;

  /* Grow the sqlite3.aVTrans array if required */
  જો( (db->nVTrans%ARRAY_INCR)==0 ){
    VTable **aVTrans;
    sqlite3_int64 nBytes = કદ(sqlite3_vtab*)*
                                 ((sqlite3_int64)db->nVTrans + ARRAY_INCR);
    aVTrans = sqlite3DbRealloc(db, (શૂન્ય *)db->aVTrans, nBytes);
    જો( !aVTrans ){
      પાછા SQLITE_NOMEM_BKPT;
    }
    memset(&aVTrans[db->nVTrans], 0, કદ(sqlite3_vtab *)*ARRAY_INCR);
    db->aVTrans = aVTrans;
  }

  પાછા SQLITE_OK;
}

/*
** Add the virtual table pVTab to the array sqlite3.aVTrans[]. Space should
** have already been reserved using growVTrans().
*/
સ્થિર શૂન્ય addToVTrans(sqlite3 *db, VTable *pVTab){
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
પૂર્ણાંક sqlite3VtabCallCreate(sqlite3 *db, પૂર્ણાંક iDb, અચલ અક્ષર *zTab, અક્ષર **pzErr){
  પૂર્ણાંક rc = SQLITE_OK;
  Table *pTab;
  Module *pMod;
  અચલ અક્ષર *zMod;

  pTab = sqlite3FindTable(db, zTab, db->aDb[iDb].zDbSName);
  પ્રતિજ્ઞા( pTab && IsVirtual(pTab) && !pTab->u.vtab.p );

  /* Locate the required virtual table module */
  zMod = pTab->u.vtab.azArg[0];
  pMod = (Module*)sqlite3HashFind(&db->aModule, zMod);

  /* If the module has been registered and includes a Create method,
  ** invoke it now. If the module has not been registered, return an
  ** error. Otherwise, do nothing.
  */
  જો( pMod==0 || pMod->pModule->xCreate==0 || pMod->pModule->xDestroy==0 ){
    *pzErr = sqlite3MPrintf(db, "no such module: %s", zMod);
    rc = SQLITE_ERROR;
  }નહીંતર{
    rc = vtabCallConstructor(db, pTab, pMod, pMod->pModule->xCreate, pzErr);
  }

  /* Justification of ALWAYS():  The xConstructor method is required to
  ** create a valid sqlite3_vtab if it returns SQLITE_OK. */
  જો( rc==SQLITE_OK && ALWAYS(sqlite3GetVTable(db, pTab)) ){
    rc = growVTrans(db);
    જો( rc==SQLITE_OK ){
      VTable *pVTab = sqlite3GetVTable(db, pTab);
      અચલ sqlite3_module *pModule = pMod->pModule;
      પૂર્ણાંક iSvpt = db->nStatement + db->nSavepoint;
      addToVTrans(db, pVTab);
      /* As in sqlite3VtabBegin(): open the current savepoint on the new
      ** table, so that a ROLLBACK TO that undoes the CREATE also invokes
      ** its xRollbackTo method. */
      જો( iSvpt && pModule->iVersion>=2 && pModule->xSavepoint ){
        pVTab->iSavepoint = iSvpt;
        rc = pModule->xSavepoint(pVTab->pVtab, iSvpt-1);
      }
    }
  }

  પાછા rc;
}

/*
** This function is used to set the schema of a virtual table.  It is only
** valid to call this function from within the xCreate() or xConnect() of a
** virtual table module.
*/
પૂર્ણાંક sqlite3_declare_vtab(sqlite3 *db, અચલ અક્ષર *zCreateTable){
  VtabCtx *pCtx;
  પૂર્ણાંક rc = SQLITE_OK;
  Table *pTab;
  Parse sParse;
  પૂર્ણાંક initBusy;
  પૂર્ણાંક i;
  અચલ unsigned અક્ષર *z;
  સ્થિર અચલ u8 aKeyword[] = { TK_CREATE, TK_TABLE, 0 };

#ifdef SQLITE_ENABLE_API_ARMOR
  જો( !sqlite3SafetyCheckOk(db) || zCreateTable==0 ){
    પાછા SQLITE_MISUSE_BKPT;
  }
#endif

  /* Verify that the first two keywords in the CREATE TABLE statement
  ** really are "CREATE" and "TABLE".  If this is not the case, then
  ** sqlite3_declare_vtab() is being misused.
  */
  z = (અચલ unsigned અક્ષર*)zCreateTable;
  માટે(i=0; aKeyword[i]; i++){
    પૂર્ણાંક tokenType = 0;
    કરો{
      z += sqlite3GetToken(z, &tokenType);
    }જ્યારે( tokenType==TK_SPACE || tokenType==TK_COMMENT );
    જો( tokenType!=aKeyword[i] ){
      sqlite3_Error_With_સંદેશ(db, SQLITE_ERROR, "syntax error");
      પાછા SQLITE_ERROR;
    }
  }
 
  sqlite3_mutex_enter(db->mutex);
  pCtx = db->pVtabCtx;
  જો( !pCtx || pCtx->bDeclared ){
    sqlite3Error(db, SQLITE_MISUSE_BKPT);
    sqlite3_mutex_leave(db->mutex);
    પાછા SQLITE_MISUSE_BKPT;
  }

  pTab = pCtx->pTab;
  પ્રતિજ્ઞા( IsVirtual(pTab) );

  sqlite3ParseObjectInit(&sParse, db);
  sParse.eParseMode = PARSE_MODE_DECLARE_VTAB;
  sParse.disableTriggers = 1;
  /* We should never be able to reach this point while loading the
  ** schema.  Nevertheless, defend against that (turn off db->init.busy)
  ** in case a bug arises. */
  પ્રતિજ્ઞા( db->init.busy==0 );
  initBusy = db->init.busy;
  db->init.busy = 0;
  sParse.nQueryLoop = 1;
  જો( SQLITE_OK==sqlite3RunParser(&sParse, zCreateTable) ){
    પ્રતિજ્ઞા( sParse.pNewTable!=0 );
    પ્રતિજ્ઞા( !db->mallocFailed );
    પ્રતિજ્ઞા( IsOrdinaryTable(sParse.pNewTable) );
    પ્રતિજ્ઞા( sParse.z_Err_સંદેશ==0 );
    જો( !pTab->aCol ){
      Table *pNew = sParse.pNewTable;
      Index *pIdx;
      pTab->aCol = pNew->aCol;
      પ્રતિજ્ઞા( IsOrdinaryTable(pNew) );
      sqlite3_પદાવલિ_List_Delete(db, pNew->u.tab.pDfltList);
      pTab->nNVCol = pTab->nCol = pNew->nCol;
      pTab->tabFlags |= pNew->tabFlags & (TF_WithoutRowid|TF_NoVisibleRowid);
      pNew->nCol = 0;
      pNew->aCol = 0;
      પ્રતિજ્ઞા( pTab->pIndex==0 );
      પ્રતિજ્ઞા( HasRowid(pNew) || sqlite3_Primary_ચાવી_Index(pNew)!=0 );
      જો( !HasRowid(pNew)
       && pCtx->pVTable->pMod->pModule->xUpdate!=0
       && sqlite3_Primary_ચાવી_Index(pNew)->n_ચાવી_Col!=1
      ){
        /* WITHOUT ROWID virtual tables must either be read-only (xUpdate==0)
        ** or else must have a single-column PRIMARY KEY */
        rc = SQLITE_ERROR;
      }
      pIdx = pNew->pIndex;
      જો( pIdx ){
        પ્રતિજ્ઞા( pIdx->pNext==0 );
        pTab->pIndex = pIdx;
        pNew->pIndex = 0;
        pIdx->pTable = pTab;
      }
    }
    pCtx->bDeclared = 1;
  }નહીંતર{
    sqlite3_Error_With_સંદેશ(db, SQLITE_ERROR,
          (sParse.z_Err_સંદેશ ? "%s" : 0), sParse.z_Err_સંદેશ);
    sqlite3DbFree(db, sParse.z_Err_સંદેશ);
    rc = SQLITE_ERROR;
  }
  sParse.eParseMode = PARSE_MODE_NORMAL;

  જો( sParse.pVdbe ){
    sqlite3VdbeFinalize(sParse.pVdbe);
  }
  sqlite3DeleteTable(db, sParse.pNewTable);
  sqlite3ParseObjectReset(&sParse);
  db->init.busy = initBusy;

  પ્રતિજ્ઞા( (rc&0xff)==rc );
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  પાછા rc;
}

/*
** This function is invoked by the vdbe to call the xDestroy method
** of the virtual table named zTab in database iDb. This occurs
** when a DROP TABLE is mentioned.
**
** This call is a no-op if zTab is not a virtual table.
*/
પૂર્ણાંક sqlite3VtabCallDestroy(sqlite3 *db, પૂર્ણાંક iDb, અચલ અક્ષર *zTab){
  પૂર્ણાંક rc = SQLITE_OK;
  Table *pTab;

  pTab = sqlite3FindTable(db, zTab, db->aDb[iDb].zDbSName);
  જો( ALWAYS(pTab!=0)
   && ALWAYS(IsVirtual(pTab))
   && ALWAYS(pTab->u.vtab.p!=0)
  ){
    VTable *p;
    પૂર્ણાંક (*xDestroy)(sqlite3_vtab *);
    માટે(p=pTab->u.vtab.p; p; p=p->pNext){
      પ્રતિજ્ઞા( p->pVtab );
      જો( p->pVtab->nRef>0 ){
        પાછા SQLITE_LOCKED;
      }
    }
    p = vtabDisconnectAll(db, pTab);
    xDestroy = p->pMod->pModule->xDestroy;
    જો( xDestroy==0 ) xDestroy = p->pMod->pModule->xDisconnect;
    પ્રતિજ્ઞા( xDestroy!=0 );
    pTab->nTabRef++;
    rc = xDestroy(p->pVtab);
    /* Remove the sqlite3_vtab* from the aVTrans[] array, if applicable */
    જો( rc==SQLITE_OK ){
      પ્રતિજ્ઞા( pTab->u.vtab.p==p && p->pNext==0 );
      p->pVtab = 0;
      pTab->u.vtab.p = 0;
      sqlite3VtabUnlock(p);
    }
    sqlite3DeleteTable(db, pTab);
  }

  પાછા rc;
}

/*
** This function invokes either the xRollback or xCommit method
** of each of the virtual tables in the sqlite3.aVTrans array. The method
** called is identified by the second argument, "offset", which is
** the offset of the method to call in the sqlite3_module structure.
**
** The array is cleared after invoking the callbacks.
*/
સ્થિર શૂન્ય callFinaliser(sqlite3 *db, પૂર્ણાંક offset){
  પૂર્ણાંક i;
  જો( db->aVTrans ){
    VTable **aVTrans = db->aVTrans;
    db->aVTrans = 0;
    માટે(i=0; i<db->nVTrans; i++){
      VTable *pVTab = aVTrans[i];
      sqlite3_vtab *p = pVTab->pVtab;
      જો( p ){
        પૂર્ણાંક (*x)(sqlite3_vtab *);
        x = *(પૂર્ણાંક (**)(sqlite3_vtab *))((અક્ષર *)p->pModule + offset);
        જો( x ) x(p);
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
પૂર્ણાંક sqlite3VtabSync(sqlite3 *db, Vdbe *p){
  પૂર્ણાંક i;
  પૂર્ણાંક rc = SQLITE_OK;
  VTable **aVTrans = db->aVTrans;

  db->aVTrans = 0;
  માટે(i=0; rc==SQLITE_OK && i<db->nVTrans; i++){
    પૂર્ણાંક (*x)(sqlite3_vtab *);
    sqlite3_vtab *pVtab = aVTrans[i]->pVtab;
    જો( pVtab && (x = pVtab->pModule->xSync)!=0 ){
      rc = x(pVtab);
      sqlite3VtabImportErrmsg(p, pVtab);
    }
  }
  db->aVTrans = aVTrans;
  પાછા rc;
}

/*
** Invoke the xRollback method of all virtual tables in the
** sqlite3.aVTrans array. Then clear the array itself.
*/
પૂર્ણાંક sqlite3VtabRollback(sqlite3 *db){
  callFinaliser(db, offsetof(sqlite3_module,xRollback));
  પાછા SQLITE_OK;
}

/*
** Invoke the xCommit method of all virtual tables in the
** sqlite3.aVTrans array. Then clear the array itself.
*/
પૂર્ણાંક sqlite3VtabCommit(sqlite3 *db){
  callFinaliser(db, offsetof(sqlite3_module,xCommit));
  પાછા SQLITE_OK;
}

/*
** If the virtual table pVtab supports the transaction interface
** (xBegin/xRollback/xCommit and optionally xSync) and a transaction is
** not currently open, invoke the xBegin method now.
**
** If the xBegin call is successful, place the sqlite3_vtab pointer
** in the sqlite3.aVTrans array.
*/
પૂર્ણાંક sqlite3VtabBegin(sqlite3 *db, VTable *pVTab){
  પૂર્ણાંક rc = SQLITE_OK;
  અચલ sqlite3_module *pModule;

  /* Special case: If db->aVTrans is NULL and db->nVTrans is greater
  ** than zero, then this function is being called from within a
  ** virtual module xSync() callback. It is illegal to write to
  ** virtual module tables in this case, so return SQLITE_LOCKED.
  */
  જો( sqlite3VtabInSync(db) ){
    પાછા SQLITE_LOCKED;
  }
  જો( !pVTab ){
    પાછા SQLITE_OK;
  }
  pModule = pVTab->pVtab->pModule;

  જો( pModule->xBegin ){
    પૂર્ણાંક i;

    /* If pVtab is already in the aVTrans array, return early */
    માટે(i=0; i<db->nVTrans; i++){
      જો( db->aVTrans[i]==pVTab ){
        પાછા SQLITE_OK;
      }
    }

    /* Invoke the xBegin method. If successful, add the vtab to the
    ** sqlite3.aVTrans[] array. */
    rc = growVTrans(db);
    જો( rc==SQLITE_OK ){
      rc = pModule->xBegin(pVTab->pVtab);
      જો( rc==SQLITE_OK ){
        પૂર્ણાંક iSvpt = db->nStatement + db->nSavepoint;
        addToVTrans(db, pVTab);
        જો( iSvpt && pModule->xSavepoint ){
          pVTab->iSavepoint = iSvpt;
          rc = pModule->xSavepoint(pVTab->pVtab, iSvpt-1);
        }
      }
    }
  }
  પાછા rc;
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
પૂર્ણાંક sqlite3VtabSavepoint(sqlite3 *db, પૂર્ણાંક op, પૂર્ણાંક iSavepoint){
  પૂર્ણાંક rc = SQLITE_OK;

  પ્રતિજ્ઞા( op==SAVEPOINT_RELEASE||op==SAVEPOINT_ROLLBACK||op==SAVEPOINT_BEGIN );
  પ્રતિજ્ઞા( iSavepoint>=-1 );
  જો( db->aVTrans ){
    પૂર્ણાંક i;
    માટે(i=0; rc==SQLITE_OK && i<db->nVTrans; i++){
      VTable *pVTab = db->aVTrans[i];
      અચલ sqlite3_module *pMod = pVTab->pMod->pModule;
      જો( pVTab->pVtab && pMod->iVersion>=2 ){
        પૂર્ણાંક (*xMethod)(sqlite3_vtab *, પૂર્ણાંક);
        sqlite3VtabLock(pVTab);
        પસંદગી( op ){
          સ્થિતિ SAVEPOINT_BEGIN:
            xMethod = pMod->xSavepoint;
            pVTab->iSavepoint = iSavepoint+1;
            તોડો;
          સ્થિતિ SAVEPOINT_ROLLBACK:
            xMethod = pMod->xRollbackTo;
            તોડો;
          મૂળભૂત:
            xMethod = pMod->xRelease;
            તોડો;
        }
        જો( xMethod && pVTab->iSavepoint>iSavepoint ){
          u64 savedFlags = (db->flags & SQLITE_Defensive);
          db->flags &= ~(u64)SQLITE_Defensive;
          rc = xMethod(pVTab->pVtab, iSavepoint);
          db->flags |= savedFlags;
        }
        sqlite3VtabUnlock(pVTab);
      }
    }
  }
  પાછા rc;
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
  પૂર્ણાંક nArg,       /* Number of arguments to the function */
  પદાવલિ *p_પદાવલિ     /* First argument to the function */
){
  Table *pTab;
  sqlite3_vtab *pVtab;
  sqlite3_module *pMod;
  શૂન્ય (*xSFunc)(sqlite3_context*,પૂર્ણાંક,sqlite3_કિંમત**) = 0;
  શૂન્ય *pArg = 0;
  FuncDef *pNew;
  પૂર્ણાંક rc = 0;

  /* Check to see the left operand is a column in a virtual table */
  જો( NEVER(p_પદાવલિ==0) ) પાછા pDef;
  જો( p_પદાવલિ->op!=TK_COLUMN ) પાછા pDef;
  પ્રતિજ્ઞા( પદાવલિ_Use_YTab(p_પદાવલિ) );
  pTab = p_પદાવલિ->y.pTab;
  જો( NEVER(pTab==0) ) પાછા pDef;
  જો( !IsVirtual(pTab) ) પાછા pDef;
  pVtab = sqlite3GetVTable(db, pTab)->pVtab;
  પ્રતિજ્ઞા( pVtab!=0 );
  પ્રતિજ્ઞા( pVtab->pModule!=0 );
  pMod = (sqlite3_module *)pVtab->pModule;
  જો( pMod->xFindFunction==0 ) પાછા pDef;

  /* Call the xFindFunction method on the virtual table implementation
  ** to see if the implementation wants to overload this function.
  **
  ** Though undocumented, we have historically always invoked xFindFunction
  ** with an all lower-case function name.  Continue in this tradition to
  ** avoid any chance of an incompatibility.
  */
#ifdef SQLITE_DEBUG
  {
    પૂર્ણાંક i;
    માટે(i=0; pDef->zName[i]; i++){
      unsigned અક્ષર x = (unsigned અક્ષર)pDef->zName[i];
      પ્રતિજ્ઞા( x==sqlite3UpperToLower[x] );
    }
  }
#endif
  rc = pMod->xFindFunction(pVtab, nArg, pDef->zName, &xSFunc, &pArg);
  જો( rc==0 ){
    પાછા pDef;
  }

  /* Create a new ephemeral function definition for the overloaded
  ** function */
  pNew = sqlite3DbMallocZero(db, કદ(*pNew)
                             + sqlite3Strlen30(pDef->zName) + 1);
  જો( pNew==0 ){
    પાછા pDef;
  }
  *pNew = *pDef;
  pNew->zName = (અચલ અક્ષર*)&pNew[1];
  memcpy((અક્ષર*)&pNew[1], pDef->zName, sqlite3Strlen30(pDef->zName)+1);
  pNew->xSFunc = xSFunc;
  pNew->pUserData = pArg;
  pNew->funcFlags |= SQLITE_FUNC_EPHEM;
  પાછા pNew;
}

/*
** Make sure virtual table pTab is contained in the pParse->apVirtualLock[]
** array so that an OP_VBegin will get generated for it.  Add pTab to the
** array if it is missing.  If pTab is already in the array, this routine
** is a no-op.
*/
શૂન્ય sqlite3VtabMakeWritable(Parse *pParse, Table *pTab){
  Parse *pToplevel = sqlite3ParseToplevel(pParse);
  પૂર્ણાંક i, n;
  Table **apVtabLock;

  પ્રતિજ્ઞા( IsVirtual(pTab) );
  માટે(i=0; i<pToplevel->nVtabLock; i++){
    જો( pTab==pToplevel->apVtabLock[i] ) પાછા;
  }
  n = (pToplevel->nVtabLock+1)*કદ(pToplevel->apVtabLock[0]);
  apVtabLock = sqlite3Realloc(pToplevel->apVtabLock, n);
  જો( apVtabLock ){
    pToplevel->apVtabLock = apVtabLock;
    pToplevel->apVtabLock[pToplevel->nVtabLock++] = pTab;
  }નહીંતર{
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
પૂર્ણાંક sqlite3VtabEponymousTableInit(Parse *pParse, Module *pMod){
  અચલ sqlite3_module *pModule = pMod->pModule;
  Table *pTab;
  અક્ષર *zErr = 0;
  પૂર્ણાંક rc;
  sqlite3 *db = pParse->db;
  જો( pMod->pEpoTab ) પાછા 1;
  જો( pModule->xCreate!=0 && pModule->xCreate!=pModule->xConnect ) પાછા 0;
  pTab = sqlite3DbMallocZero(db, કદ(Table));
  જો( pTab==0 ) પાછા 0;
  pTab->zName = sqlite3DbStrDup(db, pMod->zName);
  જો( pTab->zName==0 ){
    sqlite3DbFree(db, pTab);
    પાછા 0;
  }
  pMod->pEpoTab = pTab;
  pTab->nTabRef = 1;
  pTab->eTabType = TABTYP_VTAB;
  pTab->pSchema = db->aDb[0].pSchema;
  પ્રતિજ્ઞા( pTab->u.vtab.nArg==0 );
  pTab->iPKey = -1;
  pTab->tabFlags |= TF_Eponymous;
  addModuleArgument(pParse, pTab, sqlite3DbStrDup(db, pTab->zName));
  addModuleArgument(pParse, pTab, 0);
  addModuleArgument(pParse, pTab, sqlite3DbStrDup(db, pTab->zName));
  db->nSchemaLock++;
  rc = vtabCallConstructor(db, pTab, pMod, pModule->xConnect, &zErr);
  db->nSchemaLock--;
  જો( rc ){
    sqlite3_Error_સંદેશ(pParse, "%s", zErr);
    pParse->rc = rc;
    sqlite3DbFree(db, zErr);
    sqlite3VtabEponymousTableClear(db, pMod);
  }
  પાછા 1;
}

/*
** Erase the eponymous virtual table instance associated with
** virtual table module pMod, if it exists.
*/
શૂન્ય sqlite3VtabEponymousTableClear(sqlite3 *db, Module *pMod){
  Table *pTab = pMod->pEpoTab;
  જો( pTab!=0 ){
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
શૂન્ય sqlite3VtabEponymousTableClearAll(sqlite3 *db){
  HashElem *pElem;
  માટે(pElem=sqliteHashFirst(&db->aModule); pElem; pElem=sqliteHashNext(pElem)){
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
પૂર્ણાંક sqlite3_vtab_on_conflict(sqlite3 *db){
  સ્થિર અચલ unsigned અક્ષર aMap[] = {
    SQLITE_ROLLBACK, SQLITE_ABORT, SQLITE_FAIL, SQLITE_IGNORE, SQLITE_REPLACE
  };
#ifdef SQLITE_ENABLE_API_ARMOR
  જો( !sqlite3SafetyCheckOk(db) ) પાછા SQLITE_MISUSE_BKPT;
#endif
  પ્રતિજ્ઞા( OE_Rollback==1 && OE_Abort==2 && OE_Fail==3 );
  પ્રતિજ્ઞા( OE_Ignore==4 && OE_Replace==5 );
  પ્રતિજ્ઞા( db->vtabOnConflict>=1 && db->vtabOnConflict<=5 );
  પાછા (પૂર્ણાંક)aMap[db->vtabOnConflict-1];
}

/*
** Call from within the xCreate() or xConnect() methods to provide
** the SQLite core with additional information about the behavior
** of the virtual table being implemented.
*/
પૂર્ણાંક sqlite3_vtab_config(sqlite3 *db, પૂર્ણાંક op, ...){
  va_list ap;
  પૂર્ણાંક rc = SQLITE_OK;
  VtabCtx *p;

#ifdef SQLITE_ENABLE_API_ARMOR
  જો( !sqlite3SafetyCheckOk(db) ) પાછા SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(db->mutex);
  p = db->pVtabCtx;
  જો( !p ){
    rc = SQLITE_MISUSE_BKPT;
  }નહીંતર{
    પ્રતિજ્ઞા( p->pTab==0 || IsVirtual(p->pTab) );
    va_start(ap, op);
    પસંદગી( op ){
      સ્થિતિ SQLITE_VTAB_CONSTRAINT_SUPPORT: {
        p->pVTable->bConstraint = (u8)va_arg(ap, પૂર્ણાંક);
        તોડો;
      }
      સ્થિતિ SQLITE_VTAB_INNOCUOUS: {
        p->pVTable->eVtabRisk = SQLITE_VTABRISK_નીચલું;
        તોડો;
      }
      સ્થિતિ SQLITE_VTAB_DIRECTONLY: {
        p->pVTable->eVtabRisk = SQLITE_VTABRISK_ઉપલું;
        તોડો;
      }
      સ્થિતિ SQLITE_VTAB_USES_ALL_SCHEMAS: {
        p->pVTable->bAllSchemas = 1;
        તોડો;
      }
      મૂળભૂત: {
        rc = SQLITE_MISUSE_BKPT;
        તોડો;
      }
    }
    va_end(ap);
  }

  જો( rc!=SQLITE_OK ) sqlite3Error(db, rc);
  sqlite3_mutex_leave(db->mutex);
  પાછા rc;
}

#endif /* SQLITE_OMIT_VIRTUALTABLE */
