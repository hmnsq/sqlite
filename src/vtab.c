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
#অন্তর্ভাৱ "sqliteInt.h"

/*
** Before a virtual table xCreate() or xConnect() method is invoked, the
** sqlite3.pVtabCtx member variable is set to point to an instance of
** this struct allocated on the stack. It is used by the implementation of
** the sqlite3_declare_vtab() and sqlite3_vtab_config() APIs, both of which
** are invoked only from within xCreate and xConnect methods.
*/
সংরচনা VtabCtx {
  VTable *pVTable;    /* The virtual table being constructed */
  Table *pTab;        /* The Table object to which the virtual table belongs */
  VtabCtx *pPrior;    /* Parent context (if any) */
  পূর্ণাঙ্ক bDeclared;      /* True after sqlite3_declare_vtab() is called */
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
  ধ্রুৱ ৱর্ণ *zName,              /* Name assigned to this module */
  ধ্রুৱ sqlite3_module *pModule,  /* The definition of the module */
  শূন্য *pAux,                     /* Context pointer for xCreate/xConnect */
  শূন্য (*xDestroy)(শূন্য *)        /* Module destructor function */
){
  Module *pMod;
  Module *pDel;
  ৱর্ণ *zCopy;
  যদি( pModule==0 ){
    zCopy = (ৱর্ণ*)zName;
    pMod = 0;
  }অন্যথা{
    পূর্ণাঙ্ক nName = sqlite3Strlen30(zName);
    pMod = (Module *)sqlite3Malloc(পরিমাণ(Module) + nName + 1);
    যদি( pMod==0 ){
      sqlite3OomFault(db);
      ঘূৰাই 0;
    }
    zCopy = (ৱর্ণ *)(&pMod[1]);
    memcpy(zCopy, zName, nName+1);
    pMod->zName = zCopy;
    pMod->pModule = pModule;
    pMod->pAux = pAux;
    pMod->xDestroy = xDestroy;
    pMod->pEpoTab = 0;
    pMod->nRefModule = 1;
  }
  pDel = (Module *)sqlite3_Hash_ঢোকাও(&db->aModule,zCopy,(শূন্য*)pMod);
  যদি( pDel ){
    যদি( pDel==pMod ){
      sqlite3OomFault(db);
      sqlite3DbFree(db, pDel);
      pMod = 0;
    }অন্যথা{
      sqlite3VtabEponymousTableClear(db, pDel);
      sqlite3VtabModuleUnref(db, pDel);
    }
  }
  ঘূৰাই pMod;
}

/*
** The actual function that does the work of creating a new module.
** This function implements the sqlite3_create_module() and
** sqlite3_create_module_v2() interfaces.
*/
স্থির পূর্ণাঙ্ক createModule(
  sqlite3 *db,                    /* Database in which module is registered */
  ধ্রুৱ ৱর্ণ *zName,              /* Name assigned to this module */
  ধ্রুৱ sqlite3_module *pModule,  /* The definition of the module */
  শূন্য *pAux,                     /* Context pointer for xCreate/xConnect */
  শূন্য (*xDestroy)(শূন্য *)        /* Module destructor function */
){
  পূর্ণাঙ্ক rc = SQLITE_OK;

  sqlite3_mutex_enter(db->mutex);
  (শূন্য)sqlite3VtabCreateModule(db, zName, pModule, pAux, xDestroy);
  rc = sqlite3ApiExit(db, rc);
  যদি( rc!=SQLITE_OK && xDestroy ) xDestroy(pAux);
  sqlite3_mutex_leave(db->mutex);
  ঘূৰাই rc;
}


/*
** External API function used to create a new virtual-table module.
*/
পূর্ণাঙ্ক sqlite3_create_module(
  sqlite3 *db,                    /* Database in which module is registered */
  ধ্রুৱ ৱর্ণ *zName,              /* Name assigned to this module */
  ধ্রুৱ sqlite3_module *pModule,  /* The definition of the module */
  শূন্য *pAux                      /* Context pointer for xCreate/xConnect */
){
#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( !sqlite3SafetyCheckOk(db) || zName==0 ) ঘূৰাই SQLITE_MISUSE_BKPT;
#endif
  ঘূৰাই createModule(db, zName, pModule, pAux, 0);
}

/*
** External API function used to create a new virtual-table module.
*/
পূর্ণাঙ্ক sqlite3_create_module_v2(
  sqlite3 *db,                    /* Database in which module is registered */
  ধ্রুৱ ৱর্ণ *zName,              /* Name assigned to this module */
  ধ্রুৱ sqlite3_module *pModule,  /* The definition of the module */
  শূন্য *pAux,                     /* Context pointer for xCreate/xConnect */
  শূন্য (*xDestroy)(শূন্য *)        /* Module destructor function */
){
#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( !sqlite3SafetyCheckOk(db) || zName==0 ) ঘূৰাই SQLITE_MISUSE_BKPT;
#endif
  ঘূৰাই createModule(db, zName, pModule, pAux, xDestroy);
}

/*
** External API to drop all virtual-table modules, except those named
** on the azNames list.
*/
পূর্ণাঙ্ক sqlite3_drop_modules(sqlite3 *db, ধ্রুৱ ৱর্ণ** azNames){
  HashElem *pThis, *pNext;
#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( !sqlite3SafetyCheckOk(db) ) ঘূৰাই SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(db->mutex);
  বাবে(pThis=sqliteHashFirst(&db->aModule); pThis; pThis=pNext){
    Module *pMod = (Module*)sqliteHashData(pThis);
    pNext = sqliteHashNext(pThis);
    যদি( azNames ){
      পূর্ণাঙ্ক ii;
      বাবে(ii=0; azNames[ii]!=0 && strcmp(azNames[ii],pMod->zName)!=0; ii++){}
      যদি( azNames[ii]!=0 ) অনুৱর্ত;
    }
    createModule(db, pMod->zName, 0, 0, 0);
  }
  sqlite3_mutex_leave(db->mutex);
  ঘূৰাই SQLITE_OK;
}

/*
** Decrement the reference count on a Module object.  Destroy the
** module when the reference count reaches zero.
*/
শূন্য sqlite3VtabModuleUnref(sqlite3 *db, Module *pMod){
  প্রতিজ্ঞা( pMod->nRefModule>0 );
  pMod->nRefModule--;
  যদি( pMod->nRefModule==0 ){
    যদি( pMod->xDestroy ){
      pMod->xDestroy(pMod->pAux);
    }
    প্রতিজ্ঞা( pMod->pEpoTab==0 );
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
শূন্য sqlite3VtabLock(VTable *pVTab){
  pVTab->nRef++;
}


/*
** pTab is a pointer to a Table structure representing a virtual-table.
** Return a pointer to the VTable object used by connection db to access
** this virtual-table, if one has been created, or NULL otherwise.
*/
VTable *sqlite3GetVTable(sqlite3 *db, Table *pTab){
  VTable *pVtab;
  প্রতিজ্ঞা( IsVirtual(pTab) );
  বাবে(pVtab=pTab->u.vtab.p; pVtab && pVtab->db!=db; pVtab=pVtab->pNext);
  ঘূৰাই pVtab;
}

/*
** Decrement the ref-count on a virtual table object. When the ref-count
** reaches zero, call the xDisconnect() method to delete the object.
*/
শূন্য sqlite3VtabUnlock(VTable *pVTab){
  sqlite3 *db = pVTab->db;

  প্রতিজ্ঞা( db );
  প্রতিজ্ঞা( pVTab->nRef>0 );
  প্রতিজ্ঞা( db->eOpenState==SQLITE_STATE_OPEN
       || db->eOpenState==SQLITE_STATE_ZOMBIE );

  pVTab->nRef--;
  যদি( pVTab->nRef==0 ){
    sqlite3_vtab *p = pVTab->pVtab;
    যদি( p ){
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
স্থির VTable *vtabDisconnectAll(sqlite3 *db, Table *p){
  VTable *pRet = 0;
  VTable *pVTable;

  প্রতিজ্ঞা( IsVirtual(p) );
  pVTable = p->u.vtab.p;
  p->u.vtab.p = 0;

  /* Assert that the mutex (if any) associated with the BtShared database
  ** that contains table p is held by the caller. See header comments
  ** above function sqlite3VtabUnlockList() for an explanation of why
  ** this makes it safe to access the sqlite3.pDisconnect list of any
  ** database connection that may have an entry in the p->u.vtab.p list.
  */
  প্রতিজ্ঞা( db==0 || sqlite3SchemaMutexHeld(db, 0, p->pSchema) );

  যেতিয়ালৈ( pVTable ){
    sqlite3 *db2 = pVTable->db;
    VTable *pNext = pVTable->pNext;
    প্রতিজ্ঞা( db2 );
    যদি( db2==db ){
      pRet = pVTable;
      p->u.vtab.p = pRet;
      pRet->pNext = 0;
    }অন্যথা{
      pVTable->pNext = db2->pDisconnect;
      db2->pDisconnect = pVTable;
    }
    pVTable = pNext;
  }

  প্রতিজ্ঞা( !db || pRet );
  ঘূৰাই pRet;
}

/*
** Table *p is a virtual table. This function removes the VTable object
** for table *p associated with database connection db from the linked
** list in p->pVTab. It also decrements the VTable ref count. This is
** used when closing database connection db to free all of its VTable
** objects without disturbing the rest of the Schema object (which may
** be being used by other shared-cache connections).
*/
শূন্য sqlite3VtabDisconnect(sqlite3 *db, Table *p){
  VTable **ppVTab;

  প্রতিজ্ঞা( IsVirtual(p) );
  প্রতিজ্ঞা( sqlite3BtreeHoldsAllMutexes(db) );
  প্রতিজ্ঞা( sqlite3_mutex_held(db->mutex) );

  বাবে(ppVTab=&p->u.vtab.p; *ppVTab; ppVTab=&(*ppVTab)->pNext){
    যদি( (*ppVTab)->db==db  ){
      VTable *pVTab = *ppVTab;
      *ppVTab = pVTab->pNext;
      sqlite3VtabUnlock(pVTab);
      ভঙ্গ;
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
শূন্য sqlite3VtabUnlockList(sqlite3 *db){
  VTable *p = db->pDisconnect;

  প্রতিজ্ঞা( sqlite3BtreeHoldsAllMutexes(db) );
  প্রতিজ্ঞা( sqlite3_mutex_held(db->mutex) );

  যদি( p ){
    db->pDisconnect = 0;
    কৰা {
      VTable *pNext = p->pNext;
      sqlite3VtabUnlock(p);
      p = pNext;
    }যেতিয়ালৈ( p );
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
শূন্য sqlite3VtabClear(sqlite3 *db, Table *p){
  প্রতিজ্ঞা( IsVirtual(p) );
  প্রতিজ্ঞা( db!=0 );
  যদি( db->pnBytesFreed==0 ) vtabDisconnectAll(0, p);
  যদি( p->u.vtab.azArg ){
    পূর্ণাঙ্ক i;
    বাবে(i=0; i<p->u.vtab.nArg; i++){
      যদি( i!=1 ) sqlite3DbFree(db, p->u.vtab.azArg[i]);
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
স্থির শূন্য addModuleArgument(Parse *pParse, Table *pTable, ৱর্ণ *zArg){
  sqlite3_int64 nBytes;
  ৱর্ণ **azModuleArg;
  sqlite3 *db = pParse->db;

  প্রতিজ্ঞা( IsVirtual(pTable) );
  nBytes = পরিমাণ(ৱর্ণ *)*(2+pTable->u.vtab.nArg);
  যদি( pTable->u.vtab.nArg+3>=db->aLimit[SQLITE_LIMIT_COLUMN] ){
    sqlite3_Error_বাৰ্তা(pParse, "too many columns on %s", pTable->zName);
  }
  azModuleArg = sqlite3DbRealloc(db, pTable->u.vtab.azArg, nBytes);
  যদি( azModuleArg==0 ){
    sqlite3DbFree(db, zArg);
  }অন্যথা{
    পূর্ণাঙ্ক i = pTable->u.vtab.nArg++;
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
শূন্য sqlite3VtabBeginParse(
  Parse *pParse,        /* Parsing context */
  Token *pName1,        /* Name of new table, or database name */
  Token *pName2,        /* Name of new table or NULL */
  Token *pModuleName,   /* Name of the module for the virtual table */
  পূর্ণাঙ্ক ifNotExists       /* No error if the table already exists */
){
  Table *pTable;        /* The new virtual table */
  sqlite3 *db;          /* Database connection */

  sqlite3StartTable(pParse, pName1, pName2, 0, 0, 1, ifNotExists);
  pTable = pParse->pNewTable;
  যদি( pTable==0 ) ঘূৰাই;
  প্রতিজ্ঞা( 0==pTable->pIndex );
  pTable->eTabType = TABTYP_VTAB;

  db = pParse->db;

  প্রতিজ্ঞা( pTable->u.vtab.nArg==0 );
  addModuleArgument(pParse, pTable, sqlite3NameFromToken(db, pModuleName));
  addModuleArgument(pParse, pTable, 0);
  addModuleArgument(pParse, pTable, sqlite3DbStrDup(db, pTable->zName));
  প্রতিজ্ঞা( (pParse->sNameToken.z==pName2->z && pName2->z!=0)
       || (pParse->sNameToken.z==pName1->z && pName2->z==0)
  );
  pParse->sNameToken.n = (পূর্ণাঙ্ক)(
      &pModuleName->z[pModuleName->n] - pParse->sNameToken.z
  );

#ifndef SQLITE_OMIT_AUTHORIZATION
  /* Creating a virtual table invokes the authorization callback twice.
  ** The first invocation, to obtain permission to INSERT a row into the
  ** sqlite_schema table, has already been made by sqlite3StartTable().
  ** The second call, to obtain permission to create the table, is made now.
  */
  যদি( pTable->u.vtab.azArg ){
    পূর্ণাঙ্ক iDb = sqlite3SchemaToIndex(db, pTable->pSchema);
    প্রতিজ্ঞা( iDb>=0 ); /* The database the table is being created in */
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
স্থির শূন্য addArgumentToVtab(Parse *pParse){
  যদি( pParse->sArg.z && pParse->pNewTable ){
    ধ্রুৱ ৱর্ণ *z = (ধ্রুৱ ৱর্ণ*)pParse->sArg.z;
    পূর্ণাঙ্ক n = pParse->sArg.n;
    sqlite3 *db = pParse->db;
    addModuleArgument(pParse, pParse->pNewTable, sqlite3DbStrNDup(db, z, n));
  }
}

/*
** The parser calls this routine after the CREATE VIRTUAL TABLE statement
** has been completely parsed.
*/
শূন্য sqlite3VtabFinishParse(Parse *pParse, Token *pEnd){
  Table *pTab = pParse->pNewTable;  /* The table being constructed */
  sqlite3 *db = pParse->db;         /* The database connection */

  যদি( pTab==0 ) ঘূৰাই;
  প্রতিজ্ঞা( IsVirtual(pTab) );
  addArgumentToVtab(pParse);
  pParse->sArg.z = 0;
  যদি( pTab->u.vtab.nArg<1 ) ঘূৰাই;

  /* If the CREATE VIRTUAL TABLE statement is being entered for the
  ** first time (in other words if the virtual table is actually being
  ** created now instead of just being read out of sqlite_schema) then
  ** do additional initialization work and store the statement text
  ** in the sqlite_schema table.
  */
  যদি( !db->init.busy ){
    ৱর্ণ *zStmt;
    ৱর্ণ *zWhere;
    পূর্ণাঙ্ক iDb;
    পূর্ণাঙ্ক iReg;
    Vdbe *v;

    sqlite3MayAbort(pParse);

    /* Compute the complete text of the CREATE VIRTUAL TABLE statement */
    যদি( pEnd ){
      pParse->sNameToken.n = (পূর্ণাঙ্ক)(pEnd->z - pParse->sNameToken.z) + pEnd->n;
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
    প্রতিজ্ঞা( pParse->isCreate );
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
  }অন্যথা{
    /* If we are rereading the sqlite_schema table create the in-memory
    ** record of the table. */
    Table *pOld;
    Schema *pSchema = pTab->pSchema;
    ধ্রুৱ ৱর্ণ *zName = pTab->zName;
    প্রতিজ্ঞা( zName!=0 );
    sqlite3MarkAllShadowTablesOf(db, pTab);
    pOld = sqlite3_Hash_ঢোকাও(&pSchema->tblHash, zName, pTab);
    যদি( pOld ){
      sqlite3OomFault(db);
      প্রতিজ্ঞা( pTab==pOld );  /* Malloc must have failed inside HashInsert() */
      ঘূৰাই;
    }
    pParse->pNewTable = 0;
  }
}

/*
** The parser calls this routine when it sees the first token
** of an argument to the module name in a CREATE VIRTUAL TABLE statement.
*/
শূন্য sqlite3VtabArgInit(Parse *pParse){
  addArgumentToVtab(pParse);
  pParse->sArg.z = 0;
  pParse->sArg.n = 0;
}

/*
** The parser calls this routine for each token after the first token
** in an argument to the module name in a CREATE VIRTUAL TABLE statement.
*/
শূন্য sqlite3VtabArgExtend(Parse *pParse, Token *p){
  Token *pArg = &pParse->sArg;
  যদি( pArg->z==0 ){
    pArg->z = p->z;
    pArg->n = p->n;
  }অন্যথা{
    প্রতিজ্ঞা(pArg->z <= p->z);
    pArg->n = (পূর্ণাঙ্ক)(&p->z[p->n] - pArg->z);
  }
}

/*
** Invoke a virtual table constructor (either xCreate or xConnect). The
** pointer to the function to invoke is passed as the fourth parameter
** to this procedure.
*/
স্থির পূর্ণাঙ্ক vtabCallConstructor(
  sqlite3 *db,
  Table *pTab,
  Module *pMod,
  পূর্ণাঙ্ক (*xConstruct)(sqlite3*,শূন্য*,পূর্ণাঙ্ক,ধ্রুৱ ৱর্ণ*ধ্রুৱ*,sqlite3_vtab**,ৱর্ণ**),
  ৱর্ণ **pzErr
){
  VtabCtx sCtx;
  VTable *pVTable;
  পূর্ণাঙ্ক rc;
  ধ্রুৱ ৱর্ণ *ধ্রুৱ*azArg;
  পূর্ণাঙ্ক nArg = pTab->u.vtab.nArg;
  ৱর্ণ *zErr = 0;
  ৱর্ণ *zModuleName;
  পূর্ণাঙ্ক iDb;
  VtabCtx *pCtx;

  প্রতিজ্ঞা( IsVirtual(pTab) );
  azArg = (ধ্রুৱ ৱর্ণ *ধ্রুৱ*)pTab->u.vtab.azArg;

  /* Check that the virtual-table is not already being initialized */
  বাবে(pCtx=db->pVtabCtx; pCtx; pCtx=pCtx->pPrior){
    যদি( pCtx->pTab==pTab ){
      *pzErr = sqlite3MPrintf(db,
          "vtable constructor called recursively: %s", pTab->zName
      );
      ঘূৰাই SQLITE_LOCKED;
    }
  }

  zModuleName = sqlite3DbStrDup(db, pTab->zName);
  যদি( !zModuleName ){
    ঘূৰাই SQLITE_NOMEM_BKPT;
  }

  pVTable = sqlite3MallocZero(পরিমাণ(VTable));
  যদি( !pVTable ){
    sqlite3OomFault(db);
    sqlite3DbFree(db, zModuleName);
    ঘূৰাই SQLITE_NOMEM_BKPT;
  }
  pVTable->db = db;
  pVTable->pMod = pMod;
  pVTable->eVtabRisk = SQLITE_VTABRISK_Normal;

  iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
  pTab->u.vtab.azArg[1] = db->aDb[iDb].zDbSName;

  /* Invoke the virtual table constructor */
  প্রতিজ্ঞা( &db->pVtabCtx );
  প্রতিজ্ঞা( xConstruct );
  sCtx.pTab = pTab;
  sCtx.pVTable = pVTable;
  sCtx.pPrior = db->pVtabCtx;
  sCtx.bDeclared = 0;
  db->pVtabCtx = &sCtx;
  pTab->nTabRef++;
  rc = xConstruct(db, pMod->pAux, nArg, azArg, &pVTable->pVtab, &zErr);
  প্রতিজ্ঞা( pTab!=0 );
  প্রতিজ্ঞা( pTab->nTabRef>1 || rc!=SQLITE_OK );
  sqlite3DeleteTable(db, pTab);
  db->pVtabCtx = sCtx.pPrior;
  যদি( rc==SQLITE_NOMEM ) sqlite3OomFault(db);
  প্রতিজ্ঞা( sCtx.pTab==pTab );

  যদি( SQLITE_OK!=rc ){
    যদি( zErr==0 ){
      *pzErr = sqlite3MPrintf(db, "vtable constructor failed: %s", zModuleName);
    }অন্যথা {
      *pzErr = sqlite3MPrintf(db, "%s", zErr);
      sqlite3_free(zErr);
    }
    sqlite3DbFree(db, pVTable);
  }অন্যথা যদি( ALWAYS(pVTable->pVtab) ){
    /* Justification of ALWAYS():  A correct vtab constructor must allocate
    ** the sqlite3_vtab object if successful.  */
    memset(pVTable->pVtab, 0, পরিমাণ(pVTable->pVtab[0]));
    pVTable->pVtab->pModule = pMod->pModule;
    pMod->nRefModule++;
    pVTable->nRef = 1;
    যদি( sCtx.bDeclared==0 ){
      ধ্রুৱ ৱর্ণ *zFormat = "vtable constructor did not declare schema: %s";
      *pzErr = sqlite3MPrintf(db, zFormat, zModuleName);
      sqlite3VtabUnlock(pVTable);
      rc = SQLITE_ERROR;
    }অন্যথা{
      পূর্ণাঙ্ক iCol;
      u16 oooHidden = 0;
      /* If everything went according to plan, link the new VTable structure
      ** into the linked list headed by pTab->u.vtab.p. Then loop through the
      ** columns of the table to see if any of them contain the token "hidden".
      ** If so, set the Column COLFLAG_HIDDEN flag and remove the token from
      ** the type string.  */
      pVTable->pNext = pTab->u.vtab.p;
      pTab->u.vtab.p = pVTable;

      বাবে(iCol=0; iCol<pTab->nCol; iCol++){
        ৱর্ণ *zType = sqlite3ColumnType(&pTab->aCol[iCol], "");
        পূর্ণাঙ্ক nType;
        পূর্ণাঙ্ক i = 0;
        nType = sqlite3Strlen30(zType);
        বাবে(i=0; i<nType; i++){
          যদি( 0==sqlite3StrNICmp("hidden", &zType[i], 6)
           && (i==0 || zType[i-1]==' ')
           && (zType[i+6]=='\0' || zType[i+6]==' ')
          ){
            ভঙ্গ;
          }
        }
        যদি( i<nType ){
          পূর্ণাঙ্ক j;
          পূর্ণাঙ্ক nDel = 6 + (zType[i+6] ? 1 : 0);
          বাবে(j=i; (j+nDel)<=nType; j++){
            zType[j] = zType[j+nDel];
          }
          যদি( zType[i]=='\0' && i>0 ){
            প্রতিজ্ঞা(zType[i-1]==' ');
            zType[i-1] = '\0';
          }
          pTab->aCol[iCol].colFlags |= COLFLAG_HIDDEN;
          pTab->tabFlags |= TF_HasHidden;
          oooHidden = TF_OOOHidden;
        }অন্যথা{
          pTab->tabFlags |= oooHidden;
        }
      }
    }
  }

  sqlite3DbFree(db, zModuleName);
  ঘূৰাই rc;
}

/*
** This function is invoked by the parser to call the xConnect() method
** of the virtual table pTab. If an error occurs, an error code is returned
** and an error left in pParse.
**
** This call is a no-op if table pTab is not a virtual table.
*/
পূর্ণাঙ্ক sqlite3VtabCallConnect(Parse *pParse, Table *pTab){
  sqlite3 *db = pParse->db;
  ধ্রুৱ ৱর্ণ *zMod;
  Module *pMod;
  পূর্ণাঙ্ক rc;

  প্রতিজ্ঞা( pTab );
  প্রতিজ্ঞা( IsVirtual(pTab) );
  যদি( sqlite3GetVTable(db, pTab) ){
    ঘূৰাই SQLITE_OK;
  }

  /* Locate the required virtual table module */
  zMod = pTab->u.vtab.azArg[0];
  pMod = (Module*)sqlite3HashFind(&db->aModule, zMod);

  যদি( !pMod ){
    ধ্রুৱ ৱর্ণ *zModule = pTab->u.vtab.azArg[0];
    sqlite3_Error_বাৰ্তা(pParse, "no such module: %s", zModule);
    rc = SQLITE_ERROR;
  }অন্যথা{
    ৱর্ণ *zErr = 0;
    rc = vtabCallConstructor(db, pTab, pMod, pMod->pModule->xConnect, &zErr);
    যদি( rc!=SQLITE_OK ){
      sqlite3_Error_বাৰ্তা(pParse, "%s", zErr);
      pParse->rc = rc;
    }অন্যথা{
      sqlite3MarkAllShadowTablesOf(db, pTab);
    }
    sqlite3DbFree(db, zErr);
  }

  ঘূৰাই rc;
}
/*
** Grow the db->aVTrans[] array so that there is room for at least one
** more v-table. Return SQLITE_NOMEM if a malloc fails, or SQLITE_OK otherwise.
*/
স্থির পূর্ণাঙ্ক growVTrans(sqlite3 *db){
  ধ্রুৱ পূর্ণাঙ্ক ARRAY_INCR = 5;

  /* Grow the sqlite3.aVTrans array if required */
  যদি( (db->nVTrans%ARRAY_INCR)==0 ){
    VTable **aVTrans;
    sqlite3_int64 nBytes = পরিমাণ(sqlite3_vtab*)*
                                 ((sqlite3_int64)db->nVTrans + ARRAY_INCR);
    aVTrans = sqlite3DbRealloc(db, (শূন্য *)db->aVTrans, nBytes);
    যদি( !aVTrans ){
      ঘূৰাই SQLITE_NOMEM_BKPT;
    }
    memset(&aVTrans[db->nVTrans], 0, পরিমাণ(sqlite3_vtab *)*ARRAY_INCR);
    db->aVTrans = aVTrans;
  }

  ঘূৰাই SQLITE_OK;
}

/*
** Add the virtual table pVTab to the array sqlite3.aVTrans[]. Space should
** have already been reserved using growVTrans().
*/
স্থির শূন্য addToVTrans(sqlite3 *db, VTable *pVTab){
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
পূর্ণাঙ্ক sqlite3VtabCallCreate(sqlite3 *db, পূর্ণাঙ্ক iDb, ধ্রুৱ ৱর্ণ *zTab, ৱর্ণ **pzErr){
  পূর্ণাঙ্ক rc = SQLITE_OK;
  Table *pTab;
  Module *pMod;
  ধ্রুৱ ৱর্ণ *zMod;

  pTab = sqlite3FindTable(db, zTab, db->aDb[iDb].zDbSName);
  প্রতিজ্ঞা( pTab && IsVirtual(pTab) && !pTab->u.vtab.p );

  /* Locate the required virtual table module */
  zMod = pTab->u.vtab.azArg[0];
  pMod = (Module*)sqlite3HashFind(&db->aModule, zMod);

  /* If the module has been registered and includes a Create method,
  ** invoke it now. If the module has not been registered, return an
  ** error. Otherwise, do nothing.
  */
  যদি( pMod==0 || pMod->pModule->xCreate==0 || pMod->pModule->xDestroy==0 ){
    *pzErr = sqlite3MPrintf(db, "no such module: %s", zMod);
    rc = SQLITE_ERROR;
  }অন্যথা{
    rc = vtabCallConstructor(db, pTab, pMod, pMod->pModule->xCreate, pzErr);
  }

  /* Justification of ALWAYS():  The xConstructor method is required to
  ** create a valid sqlite3_vtab if it returns SQLITE_OK. */
  যদি( rc==SQLITE_OK && ALWAYS(sqlite3GetVTable(db, pTab)) ){
    rc = growVTrans(db);
    যদি( rc==SQLITE_OK ){
      VTable *pVTab = sqlite3GetVTable(db, pTab);
      ধ্রুৱ sqlite3_module *pModule = pMod->pModule;
      পূর্ণাঙ্ক iSvpt = db->nStatement + db->nSavepoint;
      addToVTrans(db, pVTab);
      /* As in sqlite3VtabBegin(): open the current savepoint on the new
      ** table, so that a ROLLBACK TO that undoes the CREATE also invokes
      ** its xRollbackTo method. */
      যদি( iSvpt && pModule->iVersion>=2 && pModule->xSavepoint ){
        pVTab->iSavepoint = iSvpt;
        rc = pModule->xSavepoint(pVTab->pVtab, iSvpt-1);
      }
    }
  }

  ঘূৰাই rc;
}

/*
** This function is used to set the schema of a virtual table.  It is only
** valid to call this function from within the xCreate() or xConnect() of a
** virtual table module.
*/
পূর্ণাঙ্ক sqlite3_declare_vtab(sqlite3 *db, ধ্রুৱ ৱর্ণ *zCreateTable){
  VtabCtx *pCtx;
  পূর্ণাঙ্ক rc = SQLITE_OK;
  Table *pTab;
  Parse sParse;
  পূর্ণাঙ্ক initBusy;
  পূর্ণাঙ্ক i;
  ধ্রুৱ unsigned ৱর্ণ *z;
  স্থির ধ্রুৱ u8 aKeyword[] = { TK_CREATE, TK_TABLE, 0 };

#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( !sqlite3SafetyCheckOk(db) || zCreateTable==0 ){
    ঘূৰাই SQLITE_MISUSE_BKPT;
  }
#endif

  /* Verify that the first two keywords in the CREATE TABLE statement
  ** really are "CREATE" and "TABLE".  If this is not the case, then
  ** sqlite3_declare_vtab() is being misused.
  */
  z = (ধ্রুৱ unsigned ৱর্ণ*)zCreateTable;
  বাবে(i=0; aKeyword[i]; i++){
    পূর্ণাঙ্ক tokenType = 0;
    কৰা{
      z += sqlite3GetToken(z, &tokenType);
    }যেতিয়ালৈ( tokenType==TK_SPACE || tokenType==TK_COMMENT );
    যদি( tokenType!=aKeyword[i] ){
      sqlite3_Error_With_বাৰ্তা(db, SQLITE_ERROR, "syntax error");
      ঘূৰাই SQLITE_ERROR;
    }
  }
 
  sqlite3_mutex_enter(db->mutex);
  pCtx = db->pVtabCtx;
  যদি( !pCtx || pCtx->bDeclared ){
    sqlite3Error(db, SQLITE_MISUSE_BKPT);
    sqlite3_mutex_leave(db->mutex);
    ঘূৰাই SQLITE_MISUSE_BKPT;
  }

  pTab = pCtx->pTab;
  প্রতিজ্ঞা( IsVirtual(pTab) );

  sqlite3ParseObjectInit(&sParse, db);
  sParse.eParseMode = PARSE_MODE_DECLARE_VTAB;
  sParse.disableTriggers = 1;
  /* We should never be able to reach this point while loading the
  ** schema.  Nevertheless, defend against that (turn off db->init.busy)
  ** in case a bug arises. */
  প্রতিজ্ঞা( db->init.busy==0 );
  initBusy = db->init.busy;
  db->init.busy = 0;
  sParse.nQueryLoop = 1;
  যদি( SQLITE_OK==sqlite3RunParser(&sParse, zCreateTable) ){
    প্রতিজ্ঞা( sParse.pNewTable!=0 );
    প্রতিজ্ঞা( !db->mallocFailed );
    প্রতিজ্ঞা( IsOrdinaryTable(sParse.pNewTable) );
    প্রতিজ্ঞা( sParse.z_Err_বাৰ্তা==0 );
    যদি( !pTab->aCol ){
      Table *pNew = sParse.pNewTable;
      Index *pIdx;
      pTab->aCol = pNew->aCol;
      প্রতিজ্ঞা( IsOrdinaryTable(pNew) );
      sqlite3_ৰাশিমালা_List_Delete(db, pNew->u.tab.pDfltList);
      pTab->nNVCol = pTab->nCol = pNew->nCol;
      pTab->tabFlags |= pNew->tabFlags & (TF_WithoutRowid|TF_NoVisibleRowid);
      pNew->nCol = 0;
      pNew->aCol = 0;
      প্রতিজ্ঞা( pTab->pIndex==0 );
      প্রতিজ্ঞা( HasRowid(pNew) || sqlite3_Primary_চাবি_Index(pNew)!=0 );
      যদি( !HasRowid(pNew)
       && pCtx->pVTable->pMod->pModule->xUpdate!=0
       && sqlite3_Primary_চাবি_Index(pNew)->n_চাবি_Col!=1
      ){
        /* WITHOUT ROWID virtual tables must either be read-only (xUpdate==0)
        ** or else must have a single-column PRIMARY KEY */
        rc = SQLITE_ERROR;
      }
      pIdx = pNew->pIndex;
      যদি( pIdx ){
        প্রতিজ্ঞা( pIdx->pNext==0 );
        pTab->pIndex = pIdx;
        pNew->pIndex = 0;
        pIdx->pTable = pTab;
      }
    }
    pCtx->bDeclared = 1;
  }অন্যথা{
    sqlite3_Error_With_বাৰ্তা(db, SQLITE_ERROR,
          (sParse.z_Err_বাৰ্তা ? "%s" : 0), sParse.z_Err_বাৰ্তা);
    sqlite3DbFree(db, sParse.z_Err_বাৰ্তা);
    rc = SQLITE_ERROR;
  }
  sParse.eParseMode = PARSE_MODE_NORMAL;

  যদি( sParse.pVdbe ){
    sqlite3VdbeFinalize(sParse.pVdbe);
  }
  sqlite3DeleteTable(db, sParse.pNewTable);
  sqlite3ParseObjectReset(&sParse);
  db->init.busy = initBusy;

  প্রতিজ্ঞা( (rc&0xff)==rc );
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  ঘূৰাই rc;
}

/*
** This function is invoked by the vdbe to call the xDestroy method
** of the virtual table named zTab in database iDb. This occurs
** when a DROP TABLE is mentioned.
**
** This call is a no-op if zTab is not a virtual table.
*/
পূর্ণাঙ্ক sqlite3VtabCallDestroy(sqlite3 *db, পূর্ণাঙ্ক iDb, ধ্রুৱ ৱর্ণ *zTab){
  পূর্ণাঙ্ক rc = SQLITE_OK;
  Table *pTab;

  pTab = sqlite3FindTable(db, zTab, db->aDb[iDb].zDbSName);
  যদি( ALWAYS(pTab!=0)
   && ALWAYS(IsVirtual(pTab))
   && ALWAYS(pTab->u.vtab.p!=0)
  ){
    VTable *p;
    পূর্ণাঙ্ক (*xDestroy)(sqlite3_vtab *);
    বাবে(p=pTab->u.vtab.p; p; p=p->pNext){
      প্রতিজ্ঞা( p->pVtab );
      যদি( p->pVtab->nRef>0 ){
        ঘূৰাই SQLITE_LOCKED;
      }
    }
    p = vtabDisconnectAll(db, pTab);
    xDestroy = p->pMod->pModule->xDestroy;
    যদি( xDestroy==0 ) xDestroy = p->pMod->pModule->xDisconnect;
    প্রতিজ্ঞা( xDestroy!=0 );
    pTab->nTabRef++;
    rc = xDestroy(p->pVtab);
    /* Remove the sqlite3_vtab* from the aVTrans[] array, if applicable */
    যদি( rc==SQLITE_OK ){
      প্রতিজ্ঞা( pTab->u.vtab.p==p && p->pNext==0 );
      p->pVtab = 0;
      pTab->u.vtab.p = 0;
      sqlite3VtabUnlock(p);
    }
    sqlite3DeleteTable(db, pTab);
  }

  ঘূৰাই rc;
}

/*
** This function invokes either the xRollback or xCommit method
** of each of the virtual tables in the sqlite3.aVTrans array. The method
** called is identified by the second argument, "offset", which is
** the offset of the method to call in the sqlite3_module structure.
**
** The array is cleared after invoking the callbacks.
*/
স্থির শূন্য callFinaliser(sqlite3 *db, পূর্ণাঙ্ক offset){
  পূর্ণাঙ্ক i;
  যদি( db->aVTrans ){
    VTable **aVTrans = db->aVTrans;
    db->aVTrans = 0;
    বাবে(i=0; i<db->nVTrans; i++){
      VTable *pVTab = aVTrans[i];
      sqlite3_vtab *p = pVTab->pVtab;
      যদি( p ){
        পূর্ণাঙ্ক (*x)(sqlite3_vtab *);
        x = *(পূর্ণাঙ্ক (**)(sqlite3_vtab *))((ৱর্ণ *)p->pModule + offset);
        যদি( x ) x(p);
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
পূর্ণাঙ্ক sqlite3VtabSync(sqlite3 *db, Vdbe *p){
  পূর্ণাঙ্ক i;
  পূর্ণাঙ্ক rc = SQLITE_OK;
  VTable **aVTrans = db->aVTrans;

  db->aVTrans = 0;
  বাবে(i=0; rc==SQLITE_OK && i<db->nVTrans; i++){
    পূর্ণাঙ্ক (*x)(sqlite3_vtab *);
    sqlite3_vtab *pVtab = aVTrans[i]->pVtab;
    যদি( pVtab && (x = pVtab->pModule->xSync)!=0 ){
      rc = x(pVtab);
      sqlite3VtabImportErrmsg(p, pVtab);
    }
  }
  db->aVTrans = aVTrans;
  ঘূৰাই rc;
}

/*
** Invoke the xRollback method of all virtual tables in the
** sqlite3.aVTrans array. Then clear the array itself.
*/
পূর্ণাঙ্ক sqlite3VtabRollback(sqlite3 *db){
  callFinaliser(db, offsetof(sqlite3_module,xRollback));
  ঘূৰাই SQLITE_OK;
}

/*
** Invoke the xCommit method of all virtual tables in the
** sqlite3.aVTrans array. Then clear the array itself.
*/
পূর্ণাঙ্ক sqlite3VtabCommit(sqlite3 *db){
  callFinaliser(db, offsetof(sqlite3_module,xCommit));
  ঘূৰাই SQLITE_OK;
}

/*
** If the virtual table pVtab supports the transaction interface
** (xBegin/xRollback/xCommit and optionally xSync) and a transaction is
** not currently open, invoke the xBegin method now.
**
** If the xBegin call is successful, place the sqlite3_vtab pointer
** in the sqlite3.aVTrans array.
*/
পূর্ণাঙ্ক sqlite3VtabBegin(sqlite3 *db, VTable *pVTab){
  পূর্ণাঙ্ক rc = SQLITE_OK;
  ধ্রুৱ sqlite3_module *pModule;

  /* Special case: If db->aVTrans is NULL and db->nVTrans is greater
  ** than zero, then this function is being called from within a
  ** virtual module xSync() callback. It is illegal to write to
  ** virtual module tables in this case, so return SQLITE_LOCKED.
  */
  যদি( sqlite3VtabInSync(db) ){
    ঘূৰাই SQLITE_LOCKED;
  }
  যদি( !pVTab ){
    ঘূৰাই SQLITE_OK;
  }
  pModule = pVTab->pVtab->pModule;

  যদি( pModule->xBegin ){
    পূর্ণাঙ্ক i;

    /* If pVtab is already in the aVTrans array, return early */
    বাবে(i=0; i<db->nVTrans; i++){
      যদি( db->aVTrans[i]==pVTab ){
        ঘূৰাই SQLITE_OK;
      }
    }

    /* Invoke the xBegin method. If successful, add the vtab to the
    ** sqlite3.aVTrans[] array. */
    rc = growVTrans(db);
    যদি( rc==SQLITE_OK ){
      rc = pModule->xBegin(pVTab->pVtab);
      যদি( rc==SQLITE_OK ){
        পূর্ণাঙ্ক iSvpt = db->nStatement + db->nSavepoint;
        addToVTrans(db, pVTab);
        যদি( iSvpt && pModule->xSavepoint ){
          pVTab->iSavepoint = iSvpt;
          rc = pModule->xSavepoint(pVTab->pVtab, iSvpt-1);
        }
      }
    }
  }
  ঘূৰাই rc;
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
পূর্ণাঙ্ক sqlite3VtabSavepoint(sqlite3 *db, পূর্ণাঙ্ক op, পূর্ণাঙ্ক iSavepoint){
  পূর্ণাঙ্ক rc = SQLITE_OK;

  প্রতিজ্ঞা( op==SAVEPOINT_RELEASE||op==SAVEPOINT_ROLLBACK||op==SAVEPOINT_BEGIN );
  প্রতিজ্ঞা( iSavepoint>=-1 );
  যদি( db->aVTrans ){
    পূর্ণাঙ্ক i;
    বাবে(i=0; rc==SQLITE_OK && i<db->nVTrans; i++){
      VTable *pVTab = db->aVTrans[i];
      ধ্রুৱ sqlite3_module *pMod = pVTab->pMod->pModule;
      যদি( pVTab->pVtab && pMod->iVersion>=2 ){
        পূর্ণাঙ্ক (*xMethod)(sqlite3_vtab *, পূর্ণাঙ্ক);
        sqlite3VtabLock(pVTab);
        ৱিকল্প( op ){
          পক্ষ SAVEPOINT_BEGIN:
            xMethod = pMod->xSavepoint;
            pVTab->iSavepoint = iSavepoint+1;
            ভঙ্গ;
          পক্ষ SAVEPOINT_ROLLBACK:
            xMethod = pMod->xRollbackTo;
            ভঙ্গ;
          স্ৱভাৱ:
            xMethod = pMod->xRelease;
            ভঙ্গ;
        }
        যদি( xMethod && pVTab->iSavepoint>iSavepoint ){
          u64 savedFlags = (db->flags & SQLITE_Defensive);
          db->flags &= ~(u64)SQLITE_Defensive;
          rc = xMethod(pVTab->pVtab, iSavepoint);
          db->flags |= savedFlags;
        }
        sqlite3VtabUnlock(pVTab);
      }
    }
  }
  ঘূৰাই rc;
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
  পূর্ণাঙ্ক nArg,       /* Number of arguments to the function */
  ৰাশিমালা *p_ৰাশিমালা     /* First argument to the function */
){
  Table *pTab;
  sqlite3_vtab *pVtab;
  sqlite3_module *pMod;
  শূন্য (*xSFunc)(sqlite3_context*,পূর্ণাঙ্ক,sqlite3_মান**) = 0;
  শূন্য *pArg = 0;
  FuncDef *pNew;
  পূর্ণাঙ্ক rc = 0;

  /* Check to see the left operand is a column in a virtual table */
  যদি( NEVER(p_ৰাশিমালা==0) ) ঘূৰাই pDef;
  যদি( p_ৰাশিমালা->op!=TK_COLUMN ) ঘূৰাই pDef;
  প্রতিজ্ঞা( ৰাশিমালা_Use_YTab(p_ৰাশিমালা) );
  pTab = p_ৰাশিমালা->y.pTab;
  যদি( NEVER(pTab==0) ) ঘূৰাই pDef;
  যদি( !IsVirtual(pTab) ) ঘূৰাই pDef;
  pVtab = sqlite3GetVTable(db, pTab)->pVtab;
  প্রতিজ্ঞা( pVtab!=0 );
  প্রতিজ্ঞা( pVtab->pModule!=0 );
  pMod = (sqlite3_module *)pVtab->pModule;
  যদি( pMod->xFindFunction==0 ) ঘূৰাই pDef;

  /* Call the xFindFunction method on the virtual table implementation
  ** to see if the implementation wants to overload this function.
  **
  ** Though undocumented, we have historically always invoked xFindFunction
  ** with an all lower-case function name.  Continue in this tradition to
  ** avoid any chance of an incompatibility.
  */
#ifdef SQLITE_DEBUG
  {
    পূর্ণাঙ্ক i;
    বাবে(i=0; pDef->zName[i]; i++){
      unsigned ৱর্ণ x = (unsigned ৱর্ণ)pDef->zName[i];
      প্রতিজ্ঞা( x==sqlite3UpperToLower[x] );
    }
  }
#endif
  rc = pMod->xFindFunction(pVtab, nArg, pDef->zName, &xSFunc, &pArg);
  যদি( rc==0 ){
    ঘূৰাই pDef;
  }

  /* Create a new ephemeral function definition for the overloaded
  ** function */
  pNew = sqlite3DbMallocZero(db, পরিমাণ(*pNew)
                             + sqlite3Strlen30(pDef->zName) + 1);
  যদি( pNew==0 ){
    ঘূৰাই pDef;
  }
  *pNew = *pDef;
  pNew->zName = (ধ্রুৱ ৱর্ণ*)&pNew[1];
  memcpy((ৱর্ণ*)&pNew[1], pDef->zName, sqlite3Strlen30(pDef->zName)+1);
  pNew->xSFunc = xSFunc;
  pNew->pUserData = pArg;
  pNew->funcFlags |= SQLITE_FUNC_EPHEM;
  ঘূৰাই pNew;
}

/*
** Make sure virtual table pTab is contained in the pParse->apVirtualLock[]
** array so that an OP_VBegin will get generated for it.  Add pTab to the
** array if it is missing.  If pTab is already in the array, this routine
** is a no-op.
*/
শূন্য sqlite3VtabMakeWritable(Parse *pParse, Table *pTab){
  Parse *pToplevel = sqlite3ParseToplevel(pParse);
  পূর্ণাঙ্ক i, n;
  Table **apVtabLock;

  প্রতিজ্ঞা( IsVirtual(pTab) );
  বাবে(i=0; i<pToplevel->nVtabLock; i++){
    যদি( pTab==pToplevel->apVtabLock[i] ) ঘূৰাই;
  }
  n = (pToplevel->nVtabLock+1)*পরিমাণ(pToplevel->apVtabLock[0]);
  apVtabLock = sqlite3Realloc(pToplevel->apVtabLock, n);
  যদি( apVtabLock ){
    pToplevel->apVtabLock = apVtabLock;
    pToplevel->apVtabLock[pToplevel->nVtabLock++] = pTab;
  }অন্যথা{
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
পূর্ণাঙ্ক sqlite3VtabEponymousTableInit(Parse *pParse, Module *pMod){
  ধ্রুৱ sqlite3_module *pModule = pMod->pModule;
  Table *pTab;
  ৱর্ণ *zErr = 0;
  পূর্ণাঙ্ক rc;
  sqlite3 *db = pParse->db;
  যদি( pMod->pEpoTab ) ঘূৰাই 1;
  যদি( pModule->xCreate!=0 && pModule->xCreate!=pModule->xConnect ) ঘূৰাই 0;
  pTab = sqlite3DbMallocZero(db, পরিমাণ(Table));
  যদি( pTab==0 ) ঘূৰাই 0;
  pTab->zName = sqlite3DbStrDup(db, pMod->zName);
  যদি( pTab->zName==0 ){
    sqlite3DbFree(db, pTab);
    ঘূৰাই 0;
  }
  pMod->pEpoTab = pTab;
  pTab->nTabRef = 1;
  pTab->eTabType = TABTYP_VTAB;
  pTab->pSchema = db->aDb[0].pSchema;
  প্রতিজ্ঞা( pTab->u.vtab.nArg==0 );
  pTab->iPKey = -1;
  pTab->tabFlags |= TF_Eponymous;
  addModuleArgument(pParse, pTab, sqlite3DbStrDup(db, pTab->zName));
  addModuleArgument(pParse, pTab, 0);
  addModuleArgument(pParse, pTab, sqlite3DbStrDup(db, pTab->zName));
  db->nSchemaLock++;
  rc = vtabCallConstructor(db, pTab, pMod, pModule->xConnect, &zErr);
  db->nSchemaLock--;
  যদি( rc ){
    sqlite3_Error_বাৰ্তা(pParse, "%s", zErr);
    pParse->rc = rc;
    sqlite3DbFree(db, zErr);
    sqlite3VtabEponymousTableClear(db, pMod);
  }
  ঘূৰাই 1;
}

/*
** Erase the eponymous virtual table instance associated with
** virtual table module pMod, if it exists.
*/
শূন্য sqlite3VtabEponymousTableClear(sqlite3 *db, Module *pMod){
  Table *pTab = pMod->pEpoTab;
  যদি( pTab!=0 ){
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
শূন্য sqlite3VtabEponymousTableClearAll(sqlite3 *db){
  HashElem *pElem;
  বাবে(pElem=sqliteHashFirst(&db->aModule); pElem; pElem=sqliteHashNext(pElem)){
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
পূর্ণাঙ্ক sqlite3_vtab_on_conflict(sqlite3 *db){
  স্থির ধ্রুৱ unsigned ৱর্ণ aMap[] = {
    SQLITE_ROLLBACK, SQLITE_ABORT, SQLITE_FAIL, SQLITE_IGNORE, SQLITE_REPLACE
  };
#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( !sqlite3SafetyCheckOk(db) ) ঘূৰাই SQLITE_MISUSE_BKPT;
#endif
  প্রতিজ্ঞা( OE_Rollback==1 && OE_Abort==2 && OE_Fail==3 );
  প্রতিজ্ঞা( OE_Ignore==4 && OE_Replace==5 );
  প্রতিজ্ঞা( db->vtabOnConflict>=1 && db->vtabOnConflict<=5 );
  ঘূৰাই (পূর্ণাঙ্ক)aMap[db->vtabOnConflict-1];
}

/*
** Call from within the xCreate() or xConnect() methods to provide
** the SQLite core with additional information about the behavior
** of the virtual table being implemented.
*/
পূর্ণাঙ্ক sqlite3_vtab_config(sqlite3 *db, পূর্ণাঙ্ক op, ...){
  va_list ap;
  পূর্ণাঙ্ক rc = SQLITE_OK;
  VtabCtx *p;

#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( !sqlite3SafetyCheckOk(db) ) ঘূৰাই SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(db->mutex);
  p = db->pVtabCtx;
  যদি( !p ){
    rc = SQLITE_MISUSE_BKPT;
  }অন্যথা{
    প্রতিজ্ঞা( p->pTab==0 || IsVirtual(p->pTab) );
    va_start(ap, op);
    ৱিকল্প( op ){
      পক্ষ SQLITE_VTAB_CONSTRAINT_SUPPORT: {
        p->pVTable->bConstraint = (u8)va_arg(ap, পূর্ণাঙ্ক);
        ভঙ্গ;
      }
      পক্ষ SQLITE_VTAB_INNOCUOUS: {
        p->pVTable->eVtabRisk = SQLITE_VTABRISK_নিচেৰ;
        ভঙ্গ;
      }
      পক্ষ SQLITE_VTAB_DIRECTONLY: {
        p->pVTable->eVtabRisk = SQLITE_VTABRISK_উপৰেৰ;
        ভঙ্গ;
      }
      পক্ষ SQLITE_VTAB_USES_ALL_SCHEMAS: {
        p->pVTable->bAllSchemas = 1;
        ভঙ্গ;
      }
      স্ৱভাৱ: {
        rc = SQLITE_MISUSE_BKPT;
        ভঙ্গ;
      }
    }
    va_end(ap);
  }

  যদি( rc!=SQLITE_OK ) sqlite3Error(db, rc);
  sqlite3_mutex_leave(db->mutex);
  ঘূৰাই rc;
}

#endif /* SQLITE_OMIT_VIRTUALTABLE */
