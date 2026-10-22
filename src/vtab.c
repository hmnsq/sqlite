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
#அந்தர்பாவ "sqliteInt.h"

/*
** Before a virtual table xCreate() or xConnect() method is invoked, the
** sqlite3.pVtabCtx member variable is set to point to an instance of
** this struct allocated on the stack. It is used by the implementation of
** the sqlite3_declare_vtab() and sqlite3_vtab_config() APIs, both of which
** are invoked only from within xCreate and xConnect methods.
*/
கட்டமைப்பு VtabCtx {
  VTable *pVTable;    /* The virtual table being constructed */
  Table *pTab;        /* The Table object to which the virtual table belongs */
  VtabCtx *pPrior;    /* Parent context (if any) */
  முழு bDeclared;      /* True after sqlite3_declare_vtab() is called */
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
  மாறிலி எழுத்து *zName,              /* Name assigned to this module */
  மாறிலி sqlite3_module *pModule,  /* The definition of the module */
  வெறுமை *pAux,                     /* Context pointer for xCreate/xConnect */
  வெறுமை (*xDestroy)(வெறுமை *)        /* Module destructor function */
){
  Module *pMod;
  Module *pDel;
  எழுத்து *zCopy;
  எனில்( pModule==0 ){
    zCopy = (எழுத்து*)zName;
    pMod = 0;
  }இல்லையெனில்{
    முழு nName = sqlite3Strlen30(zName);
    pMod = (Module *)sqlite3Malloc(அளவு(Module) + nName + 1);
    எனில்( pMod==0 ){
      sqlite3OomFault(db);
      திரும்பு 0;
    }
    zCopy = (எழுத்து *)(&pMod[1]);
    memcpy(zCopy, zName, nName+1);
    pMod->zName = zCopy;
    pMod->pModule = pModule;
    pMod->pAux = pAux;
    pMod->xDestroy = xDestroy;
    pMod->pEpoTab = 0;
    pMod->nRefModule = 1;
  }
  pDel = (Module *)sqlite3_Hash_செருகு(&db->aModule,zCopy,(வெறுமை*)pMod);
  எனில்( pDel ){
    எனில்( pDel==pMod ){
      sqlite3OomFault(db);
      sqlite3DbFree(db, pDel);
      pMod = 0;
    }இல்லையெனில்{
      sqlite3VtabEponymousTableClear(db, pDel);
      sqlite3VtabModuleUnref(db, pDel);
    }
  }
  திரும்பு pMod;
}

/*
** The actual function that does the work of creating a new module.
** This function implements the sqlite3_create_module() and
** sqlite3_create_module_v2() interfaces.
*/
நிலையான முழு createModule(
  sqlite3 *db,                    /* Database in which module is registered */
  மாறிலி எழுத்து *zName,              /* Name assigned to this module */
  மாறிலி sqlite3_module *pModule,  /* The definition of the module */
  வெறுமை *pAux,                     /* Context pointer for xCreate/xConnect */
  வெறுமை (*xDestroy)(வெறுமை *)        /* Module destructor function */
){
  முழு rc = SQLITE_OK;

  sqlite3_mutex_enter(db->mutex);
  (வெறுமை)sqlite3VtabCreateModule(db, zName, pModule, pAux, xDestroy);
  rc = sqlite3ApiExit(db, rc);
  எனில்( rc!=SQLITE_OK && xDestroy ) xDestroy(pAux);
  sqlite3_mutex_leave(db->mutex);
  திரும்பு rc;
}


/*
** External API function used to create a new virtual-table module.
*/
முழு sqlite3_create_module(
  sqlite3 *db,                    /* Database in which module is registered */
  மாறிலி எழுத்து *zName,              /* Name assigned to this module */
  மாறிலி sqlite3_module *pModule,  /* The definition of the module */
  வெறுமை *pAux                      /* Context pointer for xCreate/xConnect */
){
#ifdef SQLITE_ENABLE_API_ARMOR
  எனில்( !sqlite3SafetyCheckOk(db) || zName==0 ) திரும்பு SQLITE_MISUSE_BKPT;
#endif
  திரும்பு createModule(db, zName, pModule, pAux, 0);
}

/*
** External API function used to create a new virtual-table module.
*/
முழு sqlite3_create_module_v2(
  sqlite3 *db,                    /* Database in which module is registered */
  மாறிலி எழுத்து *zName,              /* Name assigned to this module */
  மாறிலி sqlite3_module *pModule,  /* The definition of the module */
  வெறுமை *pAux,                     /* Context pointer for xCreate/xConnect */
  வெறுமை (*xDestroy)(வெறுமை *)        /* Module destructor function */
){
#ifdef SQLITE_ENABLE_API_ARMOR
  எனில்( !sqlite3SafetyCheckOk(db) || zName==0 ) திரும்பு SQLITE_MISUSE_BKPT;
#endif
  திரும்பு createModule(db, zName, pModule, pAux, xDestroy);
}

/*
** External API to drop all virtual-table modules, except those named
** on the azNames list.
*/
முழு sqlite3_drop_modules(sqlite3 *db, மாறிலி எழுத்து** azNames){
  HashElem *pThis, *pNext;
#ifdef SQLITE_ENABLE_API_ARMOR
  எனில்( !sqlite3SafetyCheckOk(db) ) திரும்பு SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(db->mutex);
  க்கு(pThis=sqliteHashFirst(&db->aModule); pThis; pThis=pNext){
    Module *pMod = (Module*)sqliteHashData(pThis);
    pNext = sqliteHashNext(pThis);
    எனில்( azNames ){
      முழு ii;
      க்கு(ii=0; azNames[ii]!=0 && strcmp(azNames[ii],pMod->zName)!=0; ii++){}
      எனில்( azNames[ii]!=0 ) தொடர்;
    }
    createModule(db, pMod->zName, 0, 0, 0);
  }
  sqlite3_mutex_leave(db->mutex);
  திரும்பு SQLITE_OK;
}

/*
** Decrement the reference count on a Module object.  Destroy the
** module when the reference count reaches zero.
*/
வெறுமை sqlite3VtabModuleUnref(sqlite3 *db, Module *pMod){
  ப்ரதிஜ்ஞா( pMod->nRefModule>0 );
  pMod->nRefModule--;
  எனில்( pMod->nRefModule==0 ){
    எனில்( pMod->xDestroy ){
      pMod->xDestroy(pMod->pAux);
    }
    ப்ரதிஜ்ஞா( pMod->pEpoTab==0 );
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
வெறுமை sqlite3VtabLock(VTable *pVTab){
  pVTab->nRef++;
}


/*
** pTab is a pointer to a Table structure representing a virtual-table.
** Return a pointer to the VTable object used by connection db to access
** this virtual-table, if one has been created, or NULL otherwise.
*/
VTable *sqlite3GetVTable(sqlite3 *db, Table *pTab){
  VTable *pVtab;
  ப்ரதிஜ்ஞா( IsVirtual(pTab) );
  க்கு(pVtab=pTab->u.vtab.p; pVtab && pVtab->db!=db; pVtab=pVtab->pNext);
  திரும்பு pVtab;
}

/*
** Decrement the ref-count on a virtual table object. When the ref-count
** reaches zero, call the xDisconnect() method to delete the object.
*/
வெறுமை sqlite3VtabUnlock(VTable *pVTab){
  sqlite3 *db = pVTab->db;

  ப்ரதிஜ்ஞா( db );
  ப்ரதிஜ்ஞா( pVTab->nRef>0 );
  ப்ரதிஜ்ஞா( db->eOpenState==SQLITE_STATE_OPEN
       || db->eOpenState==SQLITE_STATE_ZOMBIE );

  pVTab->nRef--;
  எனில்( pVTab->nRef==0 ){
    sqlite3_vtab *p = pVTab->pVtab;
    எனில்( p ){
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
நிலையான VTable *vtabDisconnectAll(sqlite3 *db, Table *p){
  VTable *pRet = 0;
  VTable *pVTable;

  ப்ரதிஜ்ஞா( IsVirtual(p) );
  pVTable = p->u.vtab.p;
  p->u.vtab.p = 0;

  /* Assert that the mutex (if any) associated with the BtShared database
  ** that contains table p is held by the caller. See header comments
  ** above function sqlite3VtabUnlockList() for an explanation of why
  ** this makes it safe to access the sqlite3.pDisconnect list of any
  ** database connection that may have an entry in the p->u.vtab.p list.
  */
  ப்ரதிஜ்ஞா( db==0 || sqlite3SchemaMutexHeld(db, 0, p->pSchema) );

  போது( pVTable ){
    sqlite3 *db2 = pVTable->db;
    VTable *pNext = pVTable->pNext;
    ப்ரதிஜ்ஞா( db2 );
    எனில்( db2==db ){
      pRet = pVTable;
      p->u.vtab.p = pRet;
      pRet->pNext = 0;
    }இல்லையெனில்{
      pVTable->pNext = db2->pDisconnect;
      db2->pDisconnect = pVTable;
    }
    pVTable = pNext;
  }

  ப்ரதிஜ்ஞா( !db || pRet );
  திரும்பு pRet;
}

/*
** Table *p is a virtual table. This function removes the VTable object
** for table *p associated with database connection db from the linked
** list in p->pVTab. It also decrements the VTable ref count. This is
** used when closing database connection db to free all of its VTable
** objects without disturbing the rest of the Schema object (which may
** be being used by other shared-cache connections).
*/
வெறுமை sqlite3VtabDisconnect(sqlite3 *db, Table *p){
  VTable **ppVTab;

  ப்ரதிஜ்ஞா( IsVirtual(p) );
  ப்ரதிஜ்ஞா( sqlite3BtreeHoldsAllMutexes(db) );
  ப்ரதிஜ்ஞா( sqlite3_mutex_held(db->mutex) );

  க்கு(ppVTab=&p->u.vtab.p; *ppVTab; ppVTab=&(*ppVTab)->pNext){
    எனில்( (*ppVTab)->db==db  ){
      VTable *pVTab = *ppVTab;
      *ppVTab = pVTab->pNext;
      sqlite3VtabUnlock(pVTab);
      நிறுத்து;
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
வெறுமை sqlite3VtabUnlockList(sqlite3 *db){
  VTable *p = db->pDisconnect;

  ப்ரதிஜ்ஞா( sqlite3BtreeHoldsAllMutexes(db) );
  ப்ரதிஜ்ஞா( sqlite3_mutex_held(db->mutex) );

  எனில்( p ){
    db->pDisconnect = 0;
    செய் {
      VTable *pNext = p->pNext;
      sqlite3VtabUnlock(p);
      p = pNext;
    }போது( p );
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
வெறுமை sqlite3VtabClear(sqlite3 *db, Table *p){
  ப்ரதிஜ்ஞா( IsVirtual(p) );
  ப்ரதிஜ்ஞா( db!=0 );
  எனில்( db->pnBytesFreed==0 ) vtabDisconnectAll(0, p);
  எனில்( p->u.vtab.azArg ){
    முழு i;
    க்கு(i=0; i<p->u.vtab.nArg; i++){
      எனில்( i!=1 ) sqlite3DbFree(db, p->u.vtab.azArg[i]);
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
நிலையான வெறுமை addModuleArgument(Parse *pParse, Table *pTable, எழுத்து *zArg){
  sqlite3_int64 nBytes;
  எழுத்து **azModuleArg;
  sqlite3 *db = pParse->db;

  ப்ரதிஜ்ஞா( IsVirtual(pTable) );
  nBytes = அளவு(எழுத்து *)*(2+pTable->u.vtab.nArg);
  எனில்( pTable->u.vtab.nArg+3>=db->aLimit[SQLITE_LIMIT_COLUMN] ){
    sqlite3_Error_செய்தி(pParse, "too many columns on %s", pTable->zName);
  }
  azModuleArg = sqlite3DbRealloc(db, pTable->u.vtab.azArg, nBytes);
  எனில்( azModuleArg==0 ){
    sqlite3DbFree(db, zArg);
  }இல்லையெனில்{
    முழு i = pTable->u.vtab.nArg++;
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
வெறுமை sqlite3VtabBeginParse(
  Parse *pParse,        /* Parsing context */
  Token *pName1,        /* Name of new table, or database name */
  Token *pName2,        /* Name of new table or NULL */
  Token *pModuleName,   /* Name of the module for the virtual table */
  முழு ifNotExists       /* No error if the table already exists */
){
  Table *pTable;        /* The new virtual table */
  sqlite3 *db;          /* Database connection */

  sqlite3StartTable(pParse, pName1, pName2, 0, 0, 1, ifNotExists);
  pTable = pParse->pNewTable;
  எனில்( pTable==0 ) திரும்பு;
  ப்ரதிஜ்ஞா( 0==pTable->pIndex );
  pTable->eTabType = TABTYP_VTAB;

  db = pParse->db;

  ப்ரதிஜ்ஞா( pTable->u.vtab.nArg==0 );
  addModuleArgument(pParse, pTable, sqlite3NameFromToken(db, pModuleName));
  addModuleArgument(pParse, pTable, 0);
  addModuleArgument(pParse, pTable, sqlite3DbStrDup(db, pTable->zName));
  ப்ரதிஜ்ஞா( (pParse->sNameToken.z==pName2->z && pName2->z!=0)
       || (pParse->sNameToken.z==pName1->z && pName2->z==0)
  );
  pParse->sNameToken.n = (முழு)(
      &pModuleName->z[pModuleName->n] - pParse->sNameToken.z
  );

#ifndef SQLITE_OMIT_AUTHORIZATION
  /* Creating a virtual table invokes the authorization callback twice.
  ** The first invocation, to obtain permission to INSERT a row into the
  ** sqlite_schema table, has already been made by sqlite3StartTable().
  ** The second call, to obtain permission to create the table, is made now.
  */
  எனில்( pTable->u.vtab.azArg ){
    முழு iDb = sqlite3SchemaToIndex(db, pTable->pSchema);
    ப்ரதிஜ்ஞா( iDb>=0 ); /* The database the table is being created in */
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
நிலையான வெறுமை addArgumentToVtab(Parse *pParse){
  எனில்( pParse->sArg.z && pParse->pNewTable ){
    மாறிலி எழுத்து *z = (மாறிலி எழுத்து*)pParse->sArg.z;
    முழு n = pParse->sArg.n;
    sqlite3 *db = pParse->db;
    addModuleArgument(pParse, pParse->pNewTable, sqlite3DbStrNDup(db, z, n));
  }
}

/*
** The parser calls this routine after the CREATE VIRTUAL TABLE statement
** has been completely parsed.
*/
வெறுமை sqlite3VtabFinishParse(Parse *pParse, Token *pEnd){
  Table *pTab = pParse->pNewTable;  /* The table being constructed */
  sqlite3 *db = pParse->db;         /* The database connection */

  எனில்( pTab==0 ) திரும்பு;
  ப்ரதிஜ்ஞா( IsVirtual(pTab) );
  addArgumentToVtab(pParse);
  pParse->sArg.z = 0;
  எனில்( pTab->u.vtab.nArg<1 ) திரும்பு;

  /* If the CREATE VIRTUAL TABLE statement is being entered for the
  ** first time (in other words if the virtual table is actually being
  ** created now instead of just being read out of sqlite_schema) then
  ** do additional initialization work and store the statement text
  ** in the sqlite_schema table.
  */
  எனில்( !db->init.busy ){
    எழுத்து *zStmt;
    எழுத்து *zWhere;
    முழு iDb;
    முழு iReg;
    Vdbe *v;

    sqlite3MayAbort(pParse);

    /* Compute the complete text of the CREATE VIRTUAL TABLE statement */
    எனில்( pEnd ){
      pParse->sNameToken.n = (முழு)(pEnd->z - pParse->sNameToken.z) + pEnd->n;
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
    ப்ரதிஜ்ஞா( pParse->isCreate );
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
  }இல்லையெனில்{
    /* If we are rereading the sqlite_schema table create the in-memory
    ** record of the table. */
    Table *pOld;
    Schema *pSchema = pTab->pSchema;
    மாறிலி எழுத்து *zName = pTab->zName;
    ப்ரதிஜ்ஞா( zName!=0 );
    sqlite3MarkAllShadowTablesOf(db, pTab);
    pOld = sqlite3_Hash_செருகு(&pSchema->tblHash, zName, pTab);
    எனில்( pOld ){
      sqlite3OomFault(db);
      ப்ரதிஜ்ஞா( pTab==pOld );  /* Malloc must have failed inside HashInsert() */
      திரும்பு;
    }
    pParse->pNewTable = 0;
  }
}

/*
** The parser calls this routine when it sees the first token
** of an argument to the module name in a CREATE VIRTUAL TABLE statement.
*/
வெறுமை sqlite3VtabArgInit(Parse *pParse){
  addArgumentToVtab(pParse);
  pParse->sArg.z = 0;
  pParse->sArg.n = 0;
}

/*
** The parser calls this routine for each token after the first token
** in an argument to the module name in a CREATE VIRTUAL TABLE statement.
*/
வெறுமை sqlite3VtabArgExtend(Parse *pParse, Token *p){
  Token *pArg = &pParse->sArg;
  எனில்( pArg->z==0 ){
    pArg->z = p->z;
    pArg->n = p->n;
  }இல்லையெனில்{
    ப்ரதிஜ்ஞா(pArg->z <= p->z);
    pArg->n = (முழு)(&p->z[p->n] - pArg->z);
  }
}

/*
** Invoke a virtual table constructor (either xCreate or xConnect). The
** pointer to the function to invoke is passed as the fourth parameter
** to this procedure.
*/
நிலையான முழு vtabCallConstructor(
  sqlite3 *db,
  Table *pTab,
  Module *pMod,
  முழு (*xConstruct)(sqlite3*,வெறுமை*,முழு,மாறிலி எழுத்து*மாறிலி*,sqlite3_vtab**,எழுத்து**),
  எழுத்து **pzErr
){
  VtabCtx sCtx;
  VTable *pVTable;
  முழு rc;
  மாறிலி எழுத்து *மாறிலி*azArg;
  முழு nArg = pTab->u.vtab.nArg;
  எழுத்து *zErr = 0;
  எழுத்து *zModuleName;
  முழு iDb;
  VtabCtx *pCtx;

  ப்ரதிஜ்ஞா( IsVirtual(pTab) );
  azArg = (மாறிலி எழுத்து *மாறிலி*)pTab->u.vtab.azArg;

  /* Check that the virtual-table is not already being initialized */
  க்கு(pCtx=db->pVtabCtx; pCtx; pCtx=pCtx->pPrior){
    எனில்( pCtx->pTab==pTab ){
      *pzErr = sqlite3MPrintf(db,
          "vtable constructor called recursively: %s", pTab->zName
      );
      திரும்பு SQLITE_LOCKED;
    }
  }

  zModuleName = sqlite3DbStrDup(db, pTab->zName);
  எனில்( !zModuleName ){
    திரும்பு SQLITE_NOMEM_BKPT;
  }

  pVTable = sqlite3MallocZero(அளவு(VTable));
  எனில்( !pVTable ){
    sqlite3OomFault(db);
    sqlite3DbFree(db, zModuleName);
    திரும்பு SQLITE_NOMEM_BKPT;
  }
  pVTable->db = db;
  pVTable->pMod = pMod;
  pVTable->eVtabRisk = SQLITE_VTABRISK_Normal;

  iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
  pTab->u.vtab.azArg[1] = db->aDb[iDb].zDbSName;

  /* Invoke the virtual table constructor */
  ப்ரதிஜ்ஞா( &db->pVtabCtx );
  ப்ரதிஜ்ஞா( xConstruct );
  sCtx.pTab = pTab;
  sCtx.pVTable = pVTable;
  sCtx.pPrior = db->pVtabCtx;
  sCtx.bDeclared = 0;
  db->pVtabCtx = &sCtx;
  pTab->nTabRef++;
  rc = xConstruct(db, pMod->pAux, nArg, azArg, &pVTable->pVtab, &zErr);
  ப்ரதிஜ்ஞா( pTab!=0 );
  ப்ரதிஜ்ஞா( pTab->nTabRef>1 || rc!=SQLITE_OK );
  sqlite3DeleteTable(db, pTab);
  db->pVtabCtx = sCtx.pPrior;
  எனில்( rc==SQLITE_NOMEM ) sqlite3OomFault(db);
  ப்ரதிஜ்ஞா( sCtx.pTab==pTab );

  எனில்( SQLITE_OK!=rc ){
    எனில்( zErr==0 ){
      *pzErr = sqlite3MPrintf(db, "vtable constructor failed: %s", zModuleName);
    }இல்லையெனில் {
      *pzErr = sqlite3MPrintf(db, "%s", zErr);
      sqlite3_free(zErr);
    }
    sqlite3DbFree(db, pVTable);
  }இல்லையெனில் எனில்( ALWAYS(pVTable->pVtab) ){
    /* Justification of ALWAYS():  A correct vtab constructor must allocate
    ** the sqlite3_vtab object if successful.  */
    memset(pVTable->pVtab, 0, அளவு(pVTable->pVtab[0]));
    pVTable->pVtab->pModule = pMod->pModule;
    pMod->nRefModule++;
    pVTable->nRef = 1;
    எனில்( sCtx.bDeclared==0 ){
      மாறிலி எழுத்து *zFormat = "vtable constructor did not declare schema: %s";
      *pzErr = sqlite3MPrintf(db, zFormat, zModuleName);
      sqlite3VtabUnlock(pVTable);
      rc = SQLITE_ERROR;
    }இல்லையெனில்{
      முழு iCol;
      u16 oooHidden = 0;
      /* If everything went according to plan, link the new VTable structure
      ** into the linked list headed by pTab->u.vtab.p. Then loop through the
      ** columns of the table to see if any of them contain the token "hidden".
      ** If so, set the Column COLFLAG_HIDDEN flag and remove the token from
      ** the type string.  */
      pVTable->pNext = pTab->u.vtab.p;
      pTab->u.vtab.p = pVTable;

      க்கு(iCol=0; iCol<pTab->nCol; iCol++){
        எழுத்து *zType = sqlite3ColumnType(&pTab->aCol[iCol], "");
        முழு nType;
        முழு i = 0;
        nType = sqlite3Strlen30(zType);
        க்கு(i=0; i<nType; i++){
          எனில்( 0==sqlite3StrNICmp("hidden", &zType[i], 6)
           && (i==0 || zType[i-1]==' ')
           && (zType[i+6]=='\0' || zType[i+6]==' ')
          ){
            நிறுத்து;
          }
        }
        எனில்( i<nType ){
          முழு j;
          முழு nDel = 6 + (zType[i+6] ? 1 : 0);
          க்கு(j=i; (j+nDel)<=nType; j++){
            zType[j] = zType[j+nDel];
          }
          எனில்( zType[i]=='\0' && i>0 ){
            ப்ரதிஜ்ஞா(zType[i-1]==' ');
            zType[i-1] = '\0';
          }
          pTab->aCol[iCol].colFlags |= COLFLAG_HIDDEN;
          pTab->tabFlags |= TF_HasHidden;
          oooHidden = TF_OOOHidden;
        }இல்லையெனில்{
          pTab->tabFlags |= oooHidden;
        }
      }
    }
  }

  sqlite3DbFree(db, zModuleName);
  திரும்பு rc;
}

/*
** This function is invoked by the parser to call the xConnect() method
** of the virtual table pTab. If an error occurs, an error code is returned
** and an error left in pParse.
**
** This call is a no-op if table pTab is not a virtual table.
*/
முழு sqlite3VtabCallConnect(Parse *pParse, Table *pTab){
  sqlite3 *db = pParse->db;
  மாறிலி எழுத்து *zMod;
  Module *pMod;
  முழு rc;

  ப்ரதிஜ்ஞா( pTab );
  ப்ரதிஜ்ஞா( IsVirtual(pTab) );
  எனில்( sqlite3GetVTable(db, pTab) ){
    திரும்பு SQLITE_OK;
  }

  /* Locate the required virtual table module */
  zMod = pTab->u.vtab.azArg[0];
  pMod = (Module*)sqlite3HashFind(&db->aModule, zMod);

  எனில்( !pMod ){
    மாறிலி எழுத்து *zModule = pTab->u.vtab.azArg[0];
    sqlite3_Error_செய்தி(pParse, "no such module: %s", zModule);
    rc = SQLITE_ERROR;
  }இல்லையெனில்{
    எழுத்து *zErr = 0;
    rc = vtabCallConstructor(db, pTab, pMod, pMod->pModule->xConnect, &zErr);
    எனில்( rc!=SQLITE_OK ){
      sqlite3_Error_செய்தி(pParse, "%s", zErr);
      pParse->rc = rc;
    }இல்லையெனில்{
      sqlite3MarkAllShadowTablesOf(db, pTab);
    }
    sqlite3DbFree(db, zErr);
  }

  திரும்பு rc;
}
/*
** Grow the db->aVTrans[] array so that there is room for at least one
** more v-table. Return SQLITE_NOMEM if a malloc fails, or SQLITE_OK otherwise.
*/
நிலையான முழு growVTrans(sqlite3 *db){
  மாறிலி முழு ARRAY_INCR = 5;

  /* Grow the sqlite3.aVTrans array if required */
  எனில்( (db->nVTrans%ARRAY_INCR)==0 ){
    VTable **aVTrans;
    sqlite3_int64 nBytes = அளவு(sqlite3_vtab*)*
                                 ((sqlite3_int64)db->nVTrans + ARRAY_INCR);
    aVTrans = sqlite3DbRealloc(db, (வெறுமை *)db->aVTrans, nBytes);
    எனில்( !aVTrans ){
      திரும்பு SQLITE_NOMEM_BKPT;
    }
    memset(&aVTrans[db->nVTrans], 0, அளவு(sqlite3_vtab *)*ARRAY_INCR);
    db->aVTrans = aVTrans;
  }

  திரும்பு SQLITE_OK;
}

/*
** Add the virtual table pVTab to the array sqlite3.aVTrans[]. Space should
** have already been reserved using growVTrans().
*/
நிலையான வெறுமை addToVTrans(sqlite3 *db, VTable *pVTab){
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
முழு sqlite3VtabCallCreate(sqlite3 *db, முழு iDb, மாறிலி எழுத்து *zTab, எழுத்து **pzErr){
  முழு rc = SQLITE_OK;
  Table *pTab;
  Module *pMod;
  மாறிலி எழுத்து *zMod;

  pTab = sqlite3FindTable(db, zTab, db->aDb[iDb].zDbSName);
  ப்ரதிஜ்ஞா( pTab && IsVirtual(pTab) && !pTab->u.vtab.p );

  /* Locate the required virtual table module */
  zMod = pTab->u.vtab.azArg[0];
  pMod = (Module*)sqlite3HashFind(&db->aModule, zMod);

  /* If the module has been registered and includes a Create method,
  ** invoke it now. If the module has not been registered, return an
  ** error. Otherwise, do nothing.
  */
  எனில்( pMod==0 || pMod->pModule->xCreate==0 || pMod->pModule->xDestroy==0 ){
    *pzErr = sqlite3MPrintf(db, "no such module: %s", zMod);
    rc = SQLITE_ERROR;
  }இல்லையெனில்{
    rc = vtabCallConstructor(db, pTab, pMod, pMod->pModule->xCreate, pzErr);
  }

  /* Justification of ALWAYS():  The xConstructor method is required to
  ** create a valid sqlite3_vtab if it returns SQLITE_OK. */
  எனில்( rc==SQLITE_OK && ALWAYS(sqlite3GetVTable(db, pTab)) ){
    rc = growVTrans(db);
    எனில்( rc==SQLITE_OK ){
      VTable *pVTab = sqlite3GetVTable(db, pTab);
      மாறிலி sqlite3_module *pModule = pMod->pModule;
      முழு iSvpt = db->nStatement + db->nSavepoint;
      addToVTrans(db, pVTab);
      /* As in sqlite3VtabBegin(): open the current savepoint on the new
      ** table, so that a ROLLBACK TO that undoes the CREATE also invokes
      ** its xRollbackTo method. */
      எனில்( iSvpt && pModule->iVersion>=2 && pModule->xSavepoint ){
        pVTab->iSavepoint = iSvpt;
        rc = pModule->xSavepoint(pVTab->pVtab, iSvpt-1);
      }
    }
  }

  திரும்பு rc;
}

/*
** This function is used to set the schema of a virtual table.  It is only
** valid to call this function from within the xCreate() or xConnect() of a
** virtual table module.
*/
முழு sqlite3_declare_vtab(sqlite3 *db, மாறிலி எழுத்து *zCreateTable){
  VtabCtx *pCtx;
  முழு rc = SQLITE_OK;
  Table *pTab;
  Parse sParse;
  முழு initBusy;
  முழு i;
  மாறிலி unsigned எழுத்து *z;
  நிலையான மாறிலி u8 aKeyword[] = { TK_CREATE, TK_TABLE, 0 };

#ifdef SQLITE_ENABLE_API_ARMOR
  எனில்( !sqlite3SafetyCheckOk(db) || zCreateTable==0 ){
    திரும்பு SQLITE_MISUSE_BKPT;
  }
#endif

  /* Verify that the first two keywords in the CREATE TABLE statement
  ** really are "CREATE" and "TABLE".  If this is not the case, then
  ** sqlite3_declare_vtab() is being misused.
  */
  z = (மாறிலி unsigned எழுத்து*)zCreateTable;
  க்கு(i=0; aKeyword[i]; i++){
    முழு tokenType = 0;
    செய்{
      z += sqlite3GetToken(z, &tokenType);
    }போது( tokenType==TK_SPACE || tokenType==TK_COMMENT );
    எனில்( tokenType!=aKeyword[i] ){
      sqlite3_Error_With_செய்தி(db, SQLITE_ERROR, "syntax error");
      திரும்பு SQLITE_ERROR;
    }
  }
 
  sqlite3_mutex_enter(db->mutex);
  pCtx = db->pVtabCtx;
  எனில்( !pCtx || pCtx->bDeclared ){
    sqlite3Error(db, SQLITE_MISUSE_BKPT);
    sqlite3_mutex_leave(db->mutex);
    திரும்பு SQLITE_MISUSE_BKPT;
  }

  pTab = pCtx->pTab;
  ப்ரதிஜ்ஞா( IsVirtual(pTab) );

  sqlite3ParseObjectInit(&sParse, db);
  sParse.eParseMode = PARSE_MODE_DECLARE_VTAB;
  sParse.disableTriggers = 1;
  /* We should never be able to reach this point while loading the
  ** schema.  Nevertheless, defend against that (turn off db->init.busy)
  ** in case a bug arises. */
  ப்ரதிஜ்ஞா( db->init.busy==0 );
  initBusy = db->init.busy;
  db->init.busy = 0;
  sParse.nQueryLoop = 1;
  எனில்( SQLITE_OK==sqlite3RunParser(&sParse, zCreateTable) ){
    ப்ரதிஜ்ஞா( sParse.pNewTable!=0 );
    ப்ரதிஜ்ஞா( !db->mallocFailed );
    ப்ரதிஜ்ஞா( IsOrdinaryTable(sParse.pNewTable) );
    ப்ரதிஜ்ஞா( sParse.z_Err_செய்தி==0 );
    எனில்( !pTab->aCol ){
      Table *pNew = sParse.pNewTable;
      Index *pIdx;
      pTab->aCol = pNew->aCol;
      ப்ரதிஜ்ஞா( IsOrdinaryTable(pNew) );
      sqlite3_கோவை_List_Delete(db, pNew->u.tab.pDfltList);
      pTab->nNVCol = pTab->nCol = pNew->nCol;
      pTab->tabFlags |= pNew->tabFlags & (TF_WithoutRowid|TF_NoVisibleRowid);
      pNew->nCol = 0;
      pNew->aCol = 0;
      ப்ரதிஜ்ஞா( pTab->pIndex==0 );
      ப்ரதிஜ்ஞா( HasRowid(pNew) || sqlite3_Primary_சாவி_Index(pNew)!=0 );
      எனில்( !HasRowid(pNew)
       && pCtx->pVTable->pMod->pModule->xUpdate!=0
       && sqlite3_Primary_சாவி_Index(pNew)->n_சாவி_Col!=1
      ){
        /* WITHOUT ROWID virtual tables must either be read-only (xUpdate==0)
        ** or else must have a single-column PRIMARY KEY */
        rc = SQLITE_ERROR;
      }
      pIdx = pNew->pIndex;
      எனில்( pIdx ){
        ப்ரதிஜ்ஞா( pIdx->pNext==0 );
        pTab->pIndex = pIdx;
        pNew->pIndex = 0;
        pIdx->pTable = pTab;
      }
    }
    pCtx->bDeclared = 1;
  }இல்லையெனில்{
    sqlite3_Error_With_செய்தி(db, SQLITE_ERROR,
          (sParse.z_Err_செய்தி ? "%s" : 0), sParse.z_Err_செய்தி);
    sqlite3DbFree(db, sParse.z_Err_செய்தி);
    rc = SQLITE_ERROR;
  }
  sParse.eParseMode = PARSE_MODE_NORMAL;

  எனில்( sParse.pVdbe ){
    sqlite3VdbeFinalize(sParse.pVdbe);
  }
  sqlite3DeleteTable(db, sParse.pNewTable);
  sqlite3ParseObjectReset(&sParse);
  db->init.busy = initBusy;

  ப்ரதிஜ்ஞா( (rc&0xff)==rc );
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  திரும்பு rc;
}

/*
** This function is invoked by the vdbe to call the xDestroy method
** of the virtual table named zTab in database iDb. This occurs
** when a DROP TABLE is mentioned.
**
** This call is a no-op if zTab is not a virtual table.
*/
முழு sqlite3VtabCallDestroy(sqlite3 *db, முழு iDb, மாறிலி எழுத்து *zTab){
  முழு rc = SQLITE_OK;
  Table *pTab;

  pTab = sqlite3FindTable(db, zTab, db->aDb[iDb].zDbSName);
  எனில்( ALWAYS(pTab!=0)
   && ALWAYS(IsVirtual(pTab))
   && ALWAYS(pTab->u.vtab.p!=0)
  ){
    VTable *p;
    முழு (*xDestroy)(sqlite3_vtab *);
    க்கு(p=pTab->u.vtab.p; p; p=p->pNext){
      ப்ரதிஜ்ஞா( p->pVtab );
      எனில்( p->pVtab->nRef>0 ){
        திரும்பு SQLITE_LOCKED;
      }
    }
    p = vtabDisconnectAll(db, pTab);
    xDestroy = p->pMod->pModule->xDestroy;
    எனில்( xDestroy==0 ) xDestroy = p->pMod->pModule->xDisconnect;
    ப்ரதிஜ்ஞா( xDestroy!=0 );
    pTab->nTabRef++;
    rc = xDestroy(p->pVtab);
    /* Remove the sqlite3_vtab* from the aVTrans[] array, if applicable */
    எனில்( rc==SQLITE_OK ){
      ப்ரதிஜ்ஞா( pTab->u.vtab.p==p && p->pNext==0 );
      p->pVtab = 0;
      pTab->u.vtab.p = 0;
      sqlite3VtabUnlock(p);
    }
    sqlite3DeleteTable(db, pTab);
  }

  திரும்பு rc;
}

/*
** This function invokes either the xRollback or xCommit method
** of each of the virtual tables in the sqlite3.aVTrans array. The method
** called is identified by the second argument, "offset", which is
** the offset of the method to call in the sqlite3_module structure.
**
** The array is cleared after invoking the callbacks.
*/
நிலையான வெறுமை callFinaliser(sqlite3 *db, முழு offset){
  முழு i;
  எனில்( db->aVTrans ){
    VTable **aVTrans = db->aVTrans;
    db->aVTrans = 0;
    க்கு(i=0; i<db->nVTrans; i++){
      VTable *pVTab = aVTrans[i];
      sqlite3_vtab *p = pVTab->pVtab;
      எனில்( p ){
        முழு (*x)(sqlite3_vtab *);
        x = *(முழு (**)(sqlite3_vtab *))((எழுத்து *)p->pModule + offset);
        எனில்( x ) x(p);
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
முழு sqlite3VtabSync(sqlite3 *db, Vdbe *p){
  முழு i;
  முழு rc = SQLITE_OK;
  VTable **aVTrans = db->aVTrans;

  db->aVTrans = 0;
  க்கு(i=0; rc==SQLITE_OK && i<db->nVTrans; i++){
    முழு (*x)(sqlite3_vtab *);
    sqlite3_vtab *pVtab = aVTrans[i]->pVtab;
    எனில்( pVtab && (x = pVtab->pModule->xSync)!=0 ){
      rc = x(pVtab);
      sqlite3VtabImportErrmsg(p, pVtab);
    }
  }
  db->aVTrans = aVTrans;
  திரும்பு rc;
}

/*
** Invoke the xRollback method of all virtual tables in the
** sqlite3.aVTrans array. Then clear the array itself.
*/
முழு sqlite3VtabRollback(sqlite3 *db){
  callFinaliser(db, offsetof(sqlite3_module,xRollback));
  திரும்பு SQLITE_OK;
}

/*
** Invoke the xCommit method of all virtual tables in the
** sqlite3.aVTrans array. Then clear the array itself.
*/
முழு sqlite3VtabCommit(sqlite3 *db){
  callFinaliser(db, offsetof(sqlite3_module,xCommit));
  திரும்பு SQLITE_OK;
}

/*
** If the virtual table pVtab supports the transaction interface
** (xBegin/xRollback/xCommit and optionally xSync) and a transaction is
** not currently open, invoke the xBegin method now.
**
** If the xBegin call is successful, place the sqlite3_vtab pointer
** in the sqlite3.aVTrans array.
*/
முழு sqlite3VtabBegin(sqlite3 *db, VTable *pVTab){
  முழு rc = SQLITE_OK;
  மாறிலி sqlite3_module *pModule;

  /* Special case: If db->aVTrans is NULL and db->nVTrans is greater
  ** than zero, then this function is being called from within a
  ** virtual module xSync() callback. It is illegal to write to
  ** virtual module tables in this case, so return SQLITE_LOCKED.
  */
  எனில்( sqlite3VtabInSync(db) ){
    திரும்பு SQLITE_LOCKED;
  }
  எனில்( !pVTab ){
    திரும்பு SQLITE_OK;
  }
  pModule = pVTab->pVtab->pModule;

  எனில்( pModule->xBegin ){
    முழு i;

    /* If pVtab is already in the aVTrans array, return early */
    க்கு(i=0; i<db->nVTrans; i++){
      எனில்( db->aVTrans[i]==pVTab ){
        திரும்பு SQLITE_OK;
      }
    }

    /* Invoke the xBegin method. If successful, add the vtab to the
    ** sqlite3.aVTrans[] array. */
    rc = growVTrans(db);
    எனில்( rc==SQLITE_OK ){
      rc = pModule->xBegin(pVTab->pVtab);
      எனில்( rc==SQLITE_OK ){
        முழு iSvpt = db->nStatement + db->nSavepoint;
        addToVTrans(db, pVTab);
        எனில்( iSvpt && pModule->xSavepoint ){
          pVTab->iSavepoint = iSvpt;
          rc = pModule->xSavepoint(pVTab->pVtab, iSvpt-1);
        }
      }
    }
  }
  திரும்பு rc;
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
முழு sqlite3VtabSavepoint(sqlite3 *db, முழு op, முழு iSavepoint){
  முழு rc = SQLITE_OK;

  ப்ரதிஜ்ஞா( op==SAVEPOINT_RELEASE||op==SAVEPOINT_ROLLBACK||op==SAVEPOINT_BEGIN );
  ப்ரதிஜ்ஞா( iSavepoint>=-1 );
  எனில்( db->aVTrans ){
    முழு i;
    க்கு(i=0; rc==SQLITE_OK && i<db->nVTrans; i++){
      VTable *pVTab = db->aVTrans[i];
      மாறிலி sqlite3_module *pMod = pVTab->pMod->pModule;
      எனில்( pVTab->pVtab && pMod->iVersion>=2 ){
        முழு (*xMethod)(sqlite3_vtab *, முழு);
        sqlite3VtabLock(pVTab);
        தேர்வு( op ){
          நிலை SAVEPOINT_BEGIN:
            xMethod = pMod->xSavepoint;
            pVTab->iSavepoint = iSavepoint+1;
            நிறுத்து;
          நிலை SAVEPOINT_ROLLBACK:
            xMethod = pMod->xRollbackTo;
            நிறுத்து;
          இயல்பு:
            xMethod = pMod->xRelease;
            நிறுத்து;
        }
        எனில்( xMethod && pVTab->iSavepoint>iSavepoint ){
          u64 savedFlags = (db->flags & SQLITE_Defensive);
          db->flags &= ~(u64)SQLITE_Defensive;
          rc = xMethod(pVTab->pVtab, iSavepoint);
          db->flags |= savedFlags;
        }
        sqlite3VtabUnlock(pVTab);
      }
    }
  }
  திரும்பு rc;
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
  முழு nArg,       /* Number of arguments to the function */
  கோவை *p_கோவை     /* First argument to the function */
){
  Table *pTab;
  sqlite3_vtab *pVtab;
  sqlite3_module *pMod;
  வெறுமை (*xSFunc)(sqlite3_context*,முழு,sqlite3_மதிப்பு**) = 0;
  வெறுமை *pArg = 0;
  FuncDef *pNew;
  முழு rc = 0;

  /* Check to see the left operand is a column in a virtual table */
  எனில்( NEVER(p_கோவை==0) ) திரும்பு pDef;
  எனில்( p_கோவை->op!=TK_COLUMN ) திரும்பு pDef;
  ப்ரதிஜ்ஞா( கோவை_Use_YTab(p_கோவை) );
  pTab = p_கோவை->y.pTab;
  எனில்( NEVER(pTab==0) ) திரும்பு pDef;
  எனில்( !IsVirtual(pTab) ) திரும்பு pDef;
  pVtab = sqlite3GetVTable(db, pTab)->pVtab;
  ப்ரதிஜ்ஞா( pVtab!=0 );
  ப்ரதிஜ்ஞா( pVtab->pModule!=0 );
  pMod = (sqlite3_module *)pVtab->pModule;
  எனில்( pMod->xFindFunction==0 ) திரும்பு pDef;

  /* Call the xFindFunction method on the virtual table implementation
  ** to see if the implementation wants to overload this function.
  **
  ** Though undocumented, we have historically always invoked xFindFunction
  ** with an all lower-case function name.  Continue in this tradition to
  ** avoid any chance of an incompatibility.
  */
#ifdef SQLITE_DEBUG
  {
    முழு i;
    க்கு(i=0; pDef->zName[i]; i++){
      unsigned எழுத்து x = (unsigned எழுத்து)pDef->zName[i];
      ப்ரதிஜ்ஞா( x==sqlite3UpperToLower[x] );
    }
  }
#endif
  rc = pMod->xFindFunction(pVtab, nArg, pDef->zName, &xSFunc, &pArg);
  எனில்( rc==0 ){
    திரும்பு pDef;
  }

  /* Create a new ephemeral function definition for the overloaded
  ** function */
  pNew = sqlite3DbMallocZero(db, அளவு(*pNew)
                             + sqlite3Strlen30(pDef->zName) + 1);
  எனில்( pNew==0 ){
    திரும்பு pDef;
  }
  *pNew = *pDef;
  pNew->zName = (மாறிலி எழுத்து*)&pNew[1];
  memcpy((எழுத்து*)&pNew[1], pDef->zName, sqlite3Strlen30(pDef->zName)+1);
  pNew->xSFunc = xSFunc;
  pNew->pUserData = pArg;
  pNew->funcFlags |= SQLITE_FUNC_EPHEM;
  திரும்பு pNew;
}

/*
** Make sure virtual table pTab is contained in the pParse->apVirtualLock[]
** array so that an OP_VBegin will get generated for it.  Add pTab to the
** array if it is missing.  If pTab is already in the array, this routine
** is a no-op.
*/
வெறுமை sqlite3VtabMakeWritable(Parse *pParse, Table *pTab){
  Parse *pToplevel = sqlite3ParseToplevel(pParse);
  முழு i, n;
  Table **apVtabLock;

  ப்ரதிஜ்ஞா( IsVirtual(pTab) );
  க்கு(i=0; i<pToplevel->nVtabLock; i++){
    எனில்( pTab==pToplevel->apVtabLock[i] ) திரும்பு;
  }
  n = (pToplevel->nVtabLock+1)*அளவு(pToplevel->apVtabLock[0]);
  apVtabLock = sqlite3Realloc(pToplevel->apVtabLock, n);
  எனில்( apVtabLock ){
    pToplevel->apVtabLock = apVtabLock;
    pToplevel->apVtabLock[pToplevel->nVtabLock++] = pTab;
  }இல்லையெனில்{
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
முழு sqlite3VtabEponymousTableInit(Parse *pParse, Module *pMod){
  மாறிலி sqlite3_module *pModule = pMod->pModule;
  Table *pTab;
  எழுத்து *zErr = 0;
  முழு rc;
  sqlite3 *db = pParse->db;
  எனில்( pMod->pEpoTab ) திரும்பு 1;
  எனில்( pModule->xCreate!=0 && pModule->xCreate!=pModule->xConnect ) திரும்பு 0;
  pTab = sqlite3DbMallocZero(db, அளவு(Table));
  எனில்( pTab==0 ) திரும்பு 0;
  pTab->zName = sqlite3DbStrDup(db, pMod->zName);
  எனில்( pTab->zName==0 ){
    sqlite3DbFree(db, pTab);
    திரும்பு 0;
  }
  pMod->pEpoTab = pTab;
  pTab->nTabRef = 1;
  pTab->eTabType = TABTYP_VTAB;
  pTab->pSchema = db->aDb[0].pSchema;
  ப்ரதிஜ்ஞா( pTab->u.vtab.nArg==0 );
  pTab->iPKey = -1;
  pTab->tabFlags |= TF_Eponymous;
  addModuleArgument(pParse, pTab, sqlite3DbStrDup(db, pTab->zName));
  addModuleArgument(pParse, pTab, 0);
  addModuleArgument(pParse, pTab, sqlite3DbStrDup(db, pTab->zName));
  db->nSchemaLock++;
  rc = vtabCallConstructor(db, pTab, pMod, pModule->xConnect, &zErr);
  db->nSchemaLock--;
  எனில்( rc ){
    sqlite3_Error_செய்தி(pParse, "%s", zErr);
    pParse->rc = rc;
    sqlite3DbFree(db, zErr);
    sqlite3VtabEponymousTableClear(db, pMod);
  }
  திரும்பு 1;
}

/*
** Erase the eponymous virtual table instance associated with
** virtual table module pMod, if it exists.
*/
வெறுமை sqlite3VtabEponymousTableClear(sqlite3 *db, Module *pMod){
  Table *pTab = pMod->pEpoTab;
  எனில்( pTab!=0 ){
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
வெறுமை sqlite3VtabEponymousTableClearAll(sqlite3 *db){
  HashElem *pElem;
  க்கு(pElem=sqliteHashFirst(&db->aModule); pElem; pElem=sqliteHashNext(pElem)){
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
முழு sqlite3_vtab_on_conflict(sqlite3 *db){
  நிலையான மாறிலி unsigned எழுத்து aMap[] = {
    SQLITE_ROLLBACK, SQLITE_ABORT, SQLITE_FAIL, SQLITE_IGNORE, SQLITE_REPLACE
  };
#ifdef SQLITE_ENABLE_API_ARMOR
  எனில்( !sqlite3SafetyCheckOk(db) ) திரும்பு SQLITE_MISUSE_BKPT;
#endif
  ப்ரதிஜ்ஞா( OE_Rollback==1 && OE_Abort==2 && OE_Fail==3 );
  ப்ரதிஜ்ஞா( OE_Ignore==4 && OE_Replace==5 );
  ப்ரதிஜ்ஞா( db->vtabOnConflict>=1 && db->vtabOnConflict<=5 );
  திரும்பு (முழு)aMap[db->vtabOnConflict-1];
}

/*
** Call from within the xCreate() or xConnect() methods to provide
** the SQLite core with additional information about the behavior
** of the virtual table being implemented.
*/
முழு sqlite3_vtab_config(sqlite3 *db, முழு op, ...){
  va_list ap;
  முழு rc = SQLITE_OK;
  VtabCtx *p;

#ifdef SQLITE_ENABLE_API_ARMOR
  எனில்( !sqlite3SafetyCheckOk(db) ) திரும்பு SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(db->mutex);
  p = db->pVtabCtx;
  எனில்( !p ){
    rc = SQLITE_MISUSE_BKPT;
  }இல்லையெனில்{
    ப்ரதிஜ்ஞா( p->pTab==0 || IsVirtual(p->pTab) );
    va_start(ap, op);
    தேர்வு( op ){
      நிலை SQLITE_VTAB_CONSTRAINT_SUPPORT: {
        p->pVTable->bConstraint = (u8)va_arg(ap, முழு);
        நிறுத்து;
      }
      நிலை SQLITE_VTAB_INNOCUOUS: {
        p->pVTable->eVtabRisk = SQLITE_VTABRISK_கீழ்;
        நிறுத்து;
      }
      நிலை SQLITE_VTAB_DIRECTONLY: {
        p->pVTable->eVtabRisk = SQLITE_VTABRISK_உயர்வு;
        நிறுத்து;
      }
      நிலை SQLITE_VTAB_USES_ALL_SCHEMAS: {
        p->pVTable->bAllSchemas = 1;
        நிறுத்து;
      }
      இயல்பு: {
        rc = SQLITE_MISUSE_BKPT;
        நிறுத்து;
      }
    }
    va_end(ap);
  }

  எனில்( rc!=SQLITE_OK ) sqlite3Error(db, rc);
  sqlite3_mutex_leave(db->mutex);
  திரும்பு rc;
}

#endif /* SQLITE_OMIT_VIRTUALTABLE */
