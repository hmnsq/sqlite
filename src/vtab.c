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
#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
#అన్తర్భావ "sqliteInt.h"

/*
** Before a virtual table xCreate() or xConnect() method is invoked, the
** sqlite3.pVtabCtx member variable is set to point to an instance of
** this struct allocated on the stack. It is used by the implementation of
** the sqlite3_declare_vtab() and sqlite3_vtab_config() APIs, both of which
** are invoked only from within xCreate and xConnect methods.
*/
నిర్మాణం VtabCtx {
  VTable *pVTable;    /* The virtual table being constructed */
  పట్టిక *pTab;        /* The Table object to which the virtual table belongs */
  VtabCtx *p_పూర్వ;    /* Parent context (if any) */
  పూర్ణాంకం bDeclared;      /* True after sqlite3_declare_vtab() is called */
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
  స్థిరరాశి అక్షరం *zName,              /* Name assigned to this module */
  స్థిరరాశి sqlite3_module *pModule,  /* The definition of the module */
  శూన్య *pAux,                     /* Context pointer for xCreate/xConnect */
  శూన్య (*xDestroy)(శూన్య *)        /* Module destructor function */
){
  Module *pMod;
  Module *pDel;
  అక్షరం *zCopy;
  ఒకవేళ( pModule==0 ){
    zCopy = (అక్షరం*)zName;
    pMod = 0;
  }లేకపోతే{
    పూర్ణాంకం nName = sqlite3Strlen30(zName);
    pMod = (Module *)sqlite3Malloc(పరిమాణం(Module) + nName + 1);
    ఒకవేళ( pMod==0 ){
      sqlite3OomFault(db);
      తిరిగి 0;
    }
    zCopy = (అక్షరం *)(&pMod[1]);
    జ్ఞాపకం_నకలు(zCopy, zName, nName+1);
    pMod->zName = zCopy;
    pMod->pModule = pModule;
    pMod->pAux = pAux;
    pMod->xDestroy = xDestroy;
    pMod->pEpoTab = 0;
    pMod->nRefModule = 1;
  }
  pDel = (Module *)sqlite3_Hash_చొప్పించు(&db->aModule,zCopy,(శూన్య*)pMod);
  ఒకవేళ( pDel ){
    ఒకవేళ( pDel==pMod ){
      sqlite3OomFault(db);
      sqlite3DbFree(db, pDel);
      pMod = 0;
    }లేకపోతే{
      sqlite3_Vtab_Eponymous_పట్టిక_Clear(db, pDel);
      sqlite3VtabModuleUnref(db, pDel);
    }
  }
  తిరిగి pMod;
}

/*
** The actual function that does the work of creating a new module.
** This function implements the sqlite3_create_module() and
** sqlite3_create_module_v2() interfaces.
*/
స్థిర పూర్ణాంకం createModule(
  sqlite3 *db,                    /* Database in which module is registered */
  స్థిరరాశి అక్షరం *zName,              /* Name assigned to this module */
  స్థిరరాశి sqlite3_module *pModule,  /* The definition of the module */
  శూన్య *pAux,                     /* Context pointer for xCreate/xConnect */
  శూన్య (*xDestroy)(శూన్య *)        /* Module destructor function */
){
  పూర్ణాంకం rc = SQLITE_OK;

  sqlite3_mutex_enter(db->mutex);
  (శూన్య)sqlite3VtabCreateModule(db, zName, pModule, pAux, xDestroy);
  rc = sqlite3ApiExit(db, rc);
  ఒకవేళ( rc!=SQLITE_OK && xDestroy ) xDestroy(pAux);
  sqlite3_mutex_leave(db->mutex);
  తిరిగి rc;
}


/*
** External API function used to create a new virtual-table module.
*/
పూర్ణాంకం sqlite3_create_module(
  sqlite3 *db,                    /* Database in which module is registered */
  స్థిరరాశి అక్షరం *zName,              /* Name assigned to this module */
  స్థిరరాశి sqlite3_module *pModule,  /* The definition of the module */
  శూన్య *pAux                      /* Context pointer for xCreate/xConnect */
){
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !sqlite3SafetyCheckOk(db) || zName==0 ) తిరిగి SQLITE_MISUSE_BKPT;
#అయితే_ముగింపు
  తిరిగి createModule(db, zName, pModule, pAux, 0);
}

/*
** External API function used to create a new virtual-table module.
*/
పూర్ణాంకం sqlite3_create_module_v2(
  sqlite3 *db,                    /* Database in which module is registered */
  స్థిరరాశి అక్షరం *zName,              /* Name assigned to this module */
  స్థిరరాశి sqlite3_module *pModule,  /* The definition of the module */
  శూన్య *pAux,                     /* Context pointer for xCreate/xConnect */
  శూన్య (*xDestroy)(శూన్య *)        /* Module destructor function */
){
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !sqlite3SafetyCheckOk(db) || zName==0 ) తిరిగి SQLITE_MISUSE_BKPT;
#అయితే_ముగింపు
  తిరిగి createModule(db, zName, pModule, pAux, xDestroy);
}

/*
** External API to drop all virtual-table modules, except those named
** on the azNames list.
*/
పూర్ణాంకం sqlite3_drop_modules(sqlite3 *db, స్థిరరాశి అక్షరం** az_పేర్లు){
  HashElem *pThis, *p_తదుపరి;
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !sqlite3SafetyCheckOk(db) ) తిరిగి SQLITE_MISUSE_BKPT;
#అయితే_ముగింపు
  sqlite3_mutex_enter(db->mutex);
  కొరకు(pThis=sqlite_Hash_మొదటి(&db->aModule); pThis; pThis=p_తదుపరి){
    Module *pMod = (Module*)sqlite_Hash_దత్తాంశం(pThis);
    p_తదుపరి = sqlite_Hash_తదుపరి(pThis);
    ఒకవేళ( az_పేర్లు ){
      పూర్ణాంకం ii;
      కొరకు(ii=0; az_పేర్లు[ii]!=0 && strcmp(az_పేర్లు[ii],pMod->zName)!=0; ii++){}
      ఒకవేళ( az_పేర్లు[ii]!=0 ) కొనసాగు;
    }
    createModule(db, pMod->zName, 0, 0, 0);
  }
  sqlite3_mutex_leave(db->mutex);
  తిరిగి SQLITE_OK;
}

/*
** Decrement the reference count on a Module object.  Destroy the
** module when the reference count reaches zero.
*/
శూన్య sqlite3VtabModuleUnref(sqlite3 *db, Module *pMod){
  ప్రతిజ్ఞా( pMod->nRefModule>0 );
  pMod->nRefModule--;
  ఒకవేళ( pMod->nRefModule==0 ){
    ఒకవేళ( pMod->xDestroy ){
      pMod->xDestroy(pMod->pAux);
    }
    ప్రతిజ్ఞా( pMod->pEpoTab==0 );
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
శూన్య sqlite3_Vtab_తాళం(VTable *pVTab){
  pVTab->nRef++;
}


/*
** pTab is a pointer to a Table structure representing a virtual-table.
** Return a pointer to the VTable object used by connection db to access
** this virtual-table, if one has been created, or NULL otherwise.
*/
VTable *sqlite3GetVTable(sqlite3 *db, పట్టిక *pTab){
  VTable *pVtab;
  ప్రతిజ్ఞా( IsVirtual(pTab) );
  కొరకు(pVtab=pTab->u.vtab.p; pVtab && pVtab->db!=db; pVtab=pVtab->p_తదుపరి);
  తిరిగి pVtab;
}

/*
** Decrement the ref-count on a virtual table object. When the ref-count
** reaches zero, call the xDisconnect() method to delete the object.
*/
శూన్య sqlite3VtabUnlock(VTable *pVTab){
  sqlite3 *db = pVTab->db;

  ప్రతిజ్ఞా( db );
  ప్రతిజ్ఞా( pVTab->nRef>0 );
  ప్రతిజ్ఞా( db->eOpenState==SQLITE_STATE_OPEN
       || db->eOpenState==SQLITE_STATE_ZOMBIE );

  pVTab->nRef--;
  ఒకవేళ( pVTab->nRef==0 ){
    sqlite3_vtab *p = pVTab->pVtab;
    ఒకవేళ( p ){
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
స్థిర VTable *vtabDisconnectAll(sqlite3 *db, పట్టిక *p){
  VTable *pRet = 0;
  VTable *pVTable;

  ప్రతిజ్ఞా( IsVirtual(p) );
  pVTable = p->u.vtab.p;
  p->u.vtab.p = 0;

  /* Assert that the mutex (if any) associated with the BtShared database
  ** that contains table p is held by the caller. See header comments
  ** above function sqlite3VtabUnlockList() for an explanation of why
  ** this makes it safe to access the sqlite3.pDisconnect list of any
  ** database connection that may have an entry in the p->u.vtab.p list.
  */
  ప్రతిజ్ఞా( db==0 || sqlite3SchemaMutexHeld(db, 0, p->pSchema) );

  అయితే( pVTable ){
    sqlite3 *db2 = pVTable->db;
    VTable *p_తదుపరి = pVTable->p_తదుపరి;
    ప్రతిజ్ఞా( db2 );
    ఒకవేళ( db2==db ){
      pRet = pVTable;
      p->u.vtab.p = pRet;
      pRet->p_తదుపరి = 0;
    }లేకపోతే{
      pVTable->p_తదుపరి = db2->pDisconnect;
      db2->pDisconnect = pVTable;
    }
    pVTable = p_తదుపరి;
  }

  ప్రతిజ్ఞా( !db || pRet );
  తిరిగి pRet;
}

/*
** Table *p is a virtual table. This function removes the VTable object
** for table *p associated with database connection db from the linked
** list in p->pVTab. It also decrements the VTable ref count. This is
** used when closing database connection db to free all of its VTable
** objects without disturbing the rest of the Schema object (which may
** be being used by other shared-cache connections).
*/
శూన్య sqlite3VtabDisconnect(sqlite3 *db, పట్టిక *p){
  VTable **ppVTab;

  ప్రతిజ్ఞా( IsVirtual(p) );
  ప్రతిజ్ఞా( sqlite3BtreeHoldsAllMutexes(db) );
  ప్రతిజ్ఞా( sqlite3_mutex_held(db->mutex) );

  కొరకు(ppVTab=&p->u.vtab.p; *ppVTab; ppVTab=&(*ppVTab)->p_తదుపరి){
    ఒకవేళ( (*ppVTab)->db==db  ){
      VTable *pVTab = *ppVTab;
      *ppVTab = pVTab->p_తదుపరి;
      sqlite3VtabUnlock(pVTab);
      ఆపు;
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
శూన్య sqlite3VtabUnlockList(sqlite3 *db){
  VTable *p = db->pDisconnect;

  ప్రతిజ్ఞా( sqlite3BtreeHoldsAllMutexes(db) );
  ప్రతిజ్ఞా( sqlite3_mutex_held(db->mutex) );

  ఒకవేళ( p ){
    db->pDisconnect = 0;
    చేయి {
      VTable *p_తదుపరి = p->p_తదుపరి;
      sqlite3VtabUnlock(p);
      p = p_తదుపరి;
    }అయితే( p );
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
శూన్య sqlite3VtabClear(sqlite3 *db, పట్టిక *p){
  ప్రతిజ్ఞా( IsVirtual(p) );
  ప్రతిజ్ఞా( db!=0 );
  ఒకవేళ( db->pn_బైట్లు_Freed==0 ) vtabDisconnectAll(0, p);
  ఒకవేళ( p->u.vtab.azArg ){
    పూర్ణాంకం i;
    కొరకు(i=0; i<p->u.vtab.nArg; i++){
      ఒకవేళ( i!=1 ) sqlite3DbFree(db, p->u.vtab.azArg[i]);
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
స్థిర శూన్య కూడు_Module_వాదం(Parse *pParse, పట్టిక *p_పట్టిక, అక్షరం *zArg){
  sqlite3_int64 n_బైట్లు;
  అక్షరం **azModuleArg;
  sqlite3 *db = pParse->db;

  ప్రతిజ్ఞా( IsVirtual(p_పట్టిక) );
  n_బైట్లు = పరిమాణం(అక్షరం *)*(2+p_పట్టిక->u.vtab.nArg);
  ఒకవేళ( p_పట్టిక->u.vtab.nArg+3>=db->a_హద్దు[SQLITE_హద్దు_స్తంభం] ){
    sqlite3_Error_సందేశం(pParse, "too many columns on %s", p_పట్టిక->zName);
  }
  azModuleArg = sqlite3DbRealloc(db, p_పట్టిక->u.vtab.azArg, n_బైట్లు);
  ఒకవేళ( azModuleArg==0 ){
    sqlite3DbFree(db, zArg);
  }లేకపోతే{
    పూర్ణాంకం i = p_పట్టిక->u.vtab.nArg++;
    azModuleArg[i] = zArg;
    azModuleArg[i+1] = 0;
    p_పట్టిక->u.vtab.azArg = azModuleArg;
  }
}

/*
** The parser calls this routine when it first sees a CREATE VIRTUAL TABLE
** statement.  The module name has been parsed, but the optional list
** of parameters that follow the module name are still pending.
*/
శూన్య sqlite3VtabBeginParse(
  Parse *pParse,        /* Parsing context */
  Token *pName1,        /* Name of new table, or database name */
  Token *pName2,        /* Name of new table or NULL */
  Token *pModuleName,   /* Name of the module for the virtual table */
  పూర్ణాంకం ifNotExists       /* No error if the table already exists */
){
  పట్టిక *p_పట్టిక;        /* The new virtual table */
  sqlite3 *db;          /* Database connection */

  sqlite3_ప్రారంభం_పట్టిక(pParse, pName1, pName2, 0, 0, 1, ifNotExists);
  p_పట్టిక = pParse->p_New_పట్టిక;
  ఒకవేళ( p_పట్టిక==0 ) తిరిగి;
  ప్రతిజ్ఞా( 0==p_పట్టిక->p_సూచీ );
  p_పట్టిక->eTabType = TABTYP_VTAB;

  db = pParse->db;

  ప్రతిజ్ఞా( p_పట్టిక->u.vtab.nArg==0 );
  కూడు_Module_వాదం(pParse, p_పట్టిక, sqlite3NameFromToken(db, pModuleName));
  కూడు_Module_వాదం(pParse, p_పట్టిక, 0);
  కూడు_Module_వాదం(pParse, p_పట్టిక, sqlite3DbStrDup(db, p_పట్టిక->zName));
  ప్రతిజ్ఞా( (pParse->sNameToken.z==pName2->z && pName2->z!=0)
       || (pParse->sNameToken.z==pName1->z && pName2->z==0)
  );
  pParse->sNameToken.n = (పూర్ణాంకం)(
      &pModuleName->z[pModuleName->n] - pParse->sNameToken.z
  );

#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTHORIZATION
  /* Creating a virtual table invokes the authorization callback twice.
  ** The first invocation, to obtain permission to INSERT a row into the
  ** sqlite_schema table, has already been made by sqlite3StartTable().
  ** The second call, to obtain permission to create the table, is made now.
  */
  ఒకవేళ( p_పట్టిక->u.vtab.azArg ){
    పూర్ణాంకం iDb = sqlite3_Schema_గమ్యం_సూచీ(db, p_పట్టిక->pSchema);
    ప్రతిజ్ఞా( iDb>=0 ); /* The database the table is being created in */
    sqlite3AuthCheck(pParse, SQLITE_CREATE_VTABLE, p_పట్టిక->zName,
            p_పట్టిక->u.vtab.azArg[0], pParse->db->aDb[iDb].zDbSName);
  }
#అయితే_ముగింపు
}

/*
** This routine takes the module argument that has been accumulating
** in pParse->zArg[] and appends it to the list of arguments on the
** virtual table currently under construction in pParse->pTable.
*/
స్థిర శూన్య కూడు_వాదం_గమ్యం_Vtab(Parse *pParse){
  ఒకవేళ( pParse->sArg.z && pParse->p_New_పట్టిక ){
    స్థిరరాశి అక్షరం *z = (స్థిరరాశి అక్షరం*)pParse->sArg.z;
    పూర్ణాంకం n = pParse->sArg.n;
    sqlite3 *db = pParse->db;
    కూడు_Module_వాదం(pParse, pParse->p_New_పట్టిక, sqlite3DbStrNDup(db, z, n));
  }
}

/*
** The parser calls this routine after the CREATE VIRTUAL TABLE statement
** has been completely parsed.
*/
శూన్య sqlite3_Vtab_ముగింపు_Parse(Parse *pParse, Token *p_చివర){
  పట్టిక *pTab = pParse->p_New_పట్టిక;  /* The table being constructed */
  sqlite3 *db = pParse->db;         /* The database connection */

  ఒకవేళ( pTab==0 ) తిరిగి;
  ప్రతిజ్ఞా( IsVirtual(pTab) );
  కూడు_వాదం_గమ్యం_Vtab(pParse);
  pParse->sArg.z = 0;
  ఒకవేళ( pTab->u.vtab.nArg<1 ) తిరిగి;

  /* If the CREATE VIRTUAL TABLE statement is being entered for the
  ** first time (in other words if the virtual table is actually being
  ** created now instead of just being read out of sqlite_schema) then
  ** do additional initialization work and store the statement text
  ** in the sqlite_schema table.
  */
  ఒకవేళ( !db->init.busy ){
    అక్షరం *zStmt;
    అక్షరం *zWhere;
    పూర్ణాంకం iDb;
    పూర్ణాంకం iReg;
    Vdbe *v;

    sqlite3MayAbort(pParse);

    /* Compute the complete text of the CREATE VIRTUAL TABLE statement */
    ఒకవేళ( p_చివర ){
      pParse->sNameToken.n = (పూర్ణాంకం)(p_చివర->z - pParse->sNameToken.z) + p_చివర->n;
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
    iDb = sqlite3_Schema_గమ్యం_సూచీ(db, pTab->pSchema);
    ప్రతిజ్ఞా( pParse->isCreate );
    sqlite3NestedParse(pParse,
      "UPDATE %Q." LEGACY_SCHEMA_పట్టిక " "
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

    sqlite3_Vdbe_కూడు_Op0(v, OP_Expire);
    zWhere = sqlite3MPrintf(db, "name=%Q AND sql=%Q", pTab->zName, zStmt);
    sqlite3_Vdbe_కూడు_Parse_Schema_Op(v, iDb, zWhere, 0);
    sqlite3DbFree(db, zStmt);

    iReg = ++pParse->nMem;
    sqlite3_Vdbe_ఎక్కించు_String(v, iReg, pTab->zName);
    sqlite3_Vdbe_కూడు_Op2(v, OP_VCreate, iDb, iReg);
  }లేకపోతే{
    /* If we are rereading the sqlite_schema table create the in-memory
    ** record of the table. */
    పట్టిక *pOld;
    Schema *pSchema = pTab->pSchema;
    స్థిరరాశి అక్షరం *zName = pTab->zName;
    ప్రతిజ్ఞా( zName!=0 );
    sqlite3_Mark_All_Shadow_Tables_యొక్క(db, pTab);
    pOld = sqlite3_Hash_చొప్పించు(&pSchema->tblHash, zName, pTab);
    ఒకవేళ( pOld ){
      sqlite3OomFault(db);
      ప్రతిజ్ఞా( pTab==pOld );  /* Malloc must have failed inside HashInsert() */
      తిరిగి;
    }
    pParse->p_New_పట్టిక = 0;
  }
}

/*
** The parser calls this routine when it sees the first token
** of an argument to the module name in a CREATE VIRTUAL TABLE statement.
*/
శూన్య sqlite3VtabArgInit(Parse *pParse){
  కూడు_వాదం_గమ్యం_Vtab(pParse);
  pParse->sArg.z = 0;
  pParse->sArg.n = 0;
}

/*
** The parser calls this routine for each token after the first token
** in an argument to the module name in a CREATE VIRTUAL TABLE statement.
*/
శూన్య sqlite3VtabArgExtend(Parse *pParse, Token *p){
  Token *pArg = &pParse->sArg;
  ఒకవేళ( pArg->z==0 ){
    pArg->z = p->z;
    pArg->n = p->n;
  }లేకపోతే{
    ప్రతిజ్ఞా(pArg->z <= p->z);
    pArg->n = (పూర్ణాంకం)(&p->z[p->n] - pArg->z);
  }
}

/*
** Invoke a virtual table constructor (either xCreate or xConnect). The
** pointer to the function to invoke is passed as the fourth parameter
** to this procedure.
*/
స్థిర పూర్ణాంకం vtabCallConstructor(
  sqlite3 *db,
  పట్టిక *pTab,
  Module *pMod,
  పూర్ణాంకం (*xConstruct)(sqlite3*,శూన్య*,పూర్ణాంకం,స్థిరరాశి అక్షరం*స్థిరరాశి*,sqlite3_vtab**,అక్షరం**),
  అక్షరం **pzErr
){
  VtabCtx sCtx;
  VTable *pVTable;
  పూర్ణాంకం rc;
  స్థిరరాశి అక్షరం *స్థిరరాశి*azArg;
  పూర్ణాంకం nArg = pTab->u.vtab.nArg;
  అక్షరం *zErr = 0;
  అక్షరం *zModuleName;
  పూర్ణాంకం iDb;
  VtabCtx *pCtx;

  ప్రతిజ్ఞా( IsVirtual(pTab) );
  azArg = (స్థిరరాశి అక్షరం *స్థిరరాశి*)pTab->u.vtab.azArg;

  /* Check that the virtual-table is not already being initialized */
  కొరకు(pCtx=db->pVtabCtx; pCtx; pCtx=pCtx->p_పూర్వ){
    ఒకవేళ( pCtx->pTab==pTab ){
      *pzErr = sqlite3MPrintf(db,
          "vtable constructor called recursively: %s", pTab->zName
      );
      తిరిగి SQLITE_LOCKED;
    }
  }

  zModuleName = sqlite3DbStrDup(db, pTab->zName);
  ఒకవేళ( !zModuleName ){
    తిరిగి SQLITE_NOMEM_BKPT;
  }

  pVTable = sqlite3MallocZero(పరిమాణం(VTable));
  ఒకవేళ( !pVTable ){
    sqlite3OomFault(db);
    sqlite3DbFree(db, zModuleName);
    తిరిగి SQLITE_NOMEM_BKPT;
  }
  pVTable->db = db;
  pVTable->pMod = pMod;
  pVTable->eVtabRisk = SQLITE_VTABRISK_సాధారణ;

  iDb = sqlite3_Schema_గమ్యం_సూచీ(db, pTab->pSchema);
  pTab->u.vtab.azArg[1] = db->aDb[iDb].zDbSName;

  /* Invoke the virtual table constructor */
  ప్రతిజ్ఞా( &db->pVtabCtx );
  ప్రతిజ్ఞా( xConstruct );
  sCtx.pTab = pTab;
  sCtx.pVTable = pVTable;
  sCtx.p_పూర్వ = db->pVtabCtx;
  sCtx.bDeclared = 0;
  db->pVtabCtx = &sCtx;
  pTab->nTabRef++;
  rc = xConstruct(db, pMod->pAux, nArg, azArg, &pVTable->pVtab, &zErr);
  ప్రతిజ్ఞా( pTab!=0 );
  ప్రతిజ్ఞా( pTab->nTabRef>1 || rc!=SQLITE_OK );
  sqlite3_Delete_పట్టిక(db, pTab);
  db->pVtabCtx = sCtx.p_పూర్వ;
  ఒకవేళ( rc==SQLITE_NOMEM ) sqlite3OomFault(db);
  ప్రతిజ్ఞా( sCtx.pTab==pTab );

  ఒకవేళ( SQLITE_OK!=rc ){
    ఒకవేళ( zErr==0 ){
      *pzErr = sqlite3MPrintf(db, "vtable constructor failed: %s", zModuleName);
    }లేకపోతే {
      *pzErr = sqlite3MPrintf(db, "%s", zErr);
      sqlite3_free(zErr);
    }
    sqlite3DbFree(db, pVTable);
  }లేకపోతే ఒకవేళ( ALWAYS(pVTable->pVtab) ){
    /* Justification of ALWAYS():  A correct vtab constructor must allocate
    ** the sqlite3_vtab object if successful.  */
    memset(pVTable->pVtab, 0, పరిమాణం(pVTable->pVtab[0]));
    pVTable->pVtab->pModule = pMod->pModule;
    pMod->nRefModule++;
    pVTable->nRef = 1;
    ఒకవేళ( sCtx.bDeclared==0 ){
      స్థిరరాశి అక్షరం *zFormat = "vtable constructor did not declare schema: %s";
      *pzErr = sqlite3MPrintf(db, zFormat, zModuleName);
      sqlite3VtabUnlock(pVTable);
      rc = SQLITE_ERROR;
    }లేకపోతే{
      పూర్ణాంకం i_నిలువు;
      u16 oooHidden = 0;
      /* If everything went according to plan, link the new VTable structure
      ** into the linked list headed by pTab->u.vtab.p. Then loop through the
      ** columns of the table to see if any of them contain the token "hidden".
      ** If so, set the Column COLFLAG_HIDDEN flag and remove the token from
      ** the type string.  */
      pVTable->p_తదుపరి = pTab->u.vtab.p;
      pTab->u.vtab.p = pVTable;

      కొరకు(i_నిలువు=0; i_నిలువు<pTab->n_నిలువు; i_నిలువు++){
        అక్షరం *zType = sqlite3_స్తంభం_Type(&pTab->a_నిలువు[i_నిలువు], "");
        పూర్ణాంకం nType;
        పూర్ణాంకం i = 0;
        nType = sqlite3Strlen30(zType);
        కొరకు(i=0; i<nType; i++){
          ఒకవేళ( 0==sqlite3StrNICmp("hidden", &zType[i], 6)
           && (i==0 || zType[i-1]==' ')
           && (zType[i+6]=='\0' || zType[i+6]==' ')
          ){
            ఆపు;
          }
        }
        ఒకవేళ( i<nType ){
          పూర్ణాంకం j;
          పూర్ణాంకం nDel = 6 + (zType[i+6] ? 1 : 0);
          కొరకు(j=i; (j+nDel)<=nType; j++){
            zType[j] = zType[j+nDel];
          }
          ఒకవేళ( zType[i]=='\0' && i>0 ){
            ప్రతిజ్ఞా(zType[i-1]==' ');
            zType[i-1] = '\0';
          }
          pTab->a_నిలువు[i_నిలువు].నిలువు_Flags |= COLFLAG_HIDDEN;
          pTab->tabFlags |= TF_HasHidden;
          oooHidden = TF_OOOHidden;
        }లేకపోతే{
          pTab->tabFlags |= oooHidden;
        }
      }
    }
  }

  sqlite3DbFree(db, zModuleName);
  తిరిగి rc;
}

/*
** This function is invoked by the parser to call the xConnect() method
** of the virtual table pTab. If an error occurs, an error code is returned
** and an error left in pParse.
**
** This call is a no-op if table pTab is not a virtual table.
*/
పూర్ణాంకం sqlite3VtabCallConnect(Parse *pParse, పట్టిక *pTab){
  sqlite3 *db = pParse->db;
  స్థిరరాశి అక్షరం *zMod;
  Module *pMod;
  పూర్ణాంకం rc;

  ప్రతిజ్ఞా( pTab );
  ప్రతిజ్ఞా( IsVirtual(pTab) );
  ఒకవేళ( sqlite3GetVTable(db, pTab) ){
    తిరిగి SQLITE_OK;
  }

  /* Locate the required virtual table module */
  zMod = pTab->u.vtab.azArg[0];
  pMod = (Module*)sqlite3_Hash_కనుగొను(&db->aModule, zMod);

  ఒకవేళ( !pMod ){
    స్థిరరాశి అక్షరం *zModule = pTab->u.vtab.azArg[0];
    sqlite3_Error_సందేశం(pParse, "no such module: %s", zModule);
    rc = SQLITE_ERROR;
  }లేకపోతే{
    అక్షరం *zErr = 0;
    rc = vtabCallConstructor(db, pTab, pMod, pMod->pModule->xConnect, &zErr);
    ఒకవేళ( rc!=SQLITE_OK ){
      sqlite3_Error_సందేశం(pParse, "%s", zErr);
      pParse->rc = rc;
    }లేకపోతే{
      sqlite3_Mark_All_Shadow_Tables_యొక్క(db, pTab);
    }
    sqlite3DbFree(db, zErr);
  }

  తిరిగి rc;
}
/*
** Grow the db->aVTrans[] array so that there is room for at least one
** more v-table. Return SQLITE_NOMEM if a malloc fails, or SQLITE_OK otherwise.
*/
స్థిర పూర్ణాంకం growVTrans(sqlite3 *db){
  స్థిరరాశి పూర్ణాంకం ARRAY_INCR = 5;

  /* Grow the sqlite3.aVTrans array if required */
  ఒకవేళ( (db->nVTrans%ARRAY_INCR)==0 ){
    VTable **aVTrans;
    sqlite3_int64 n_బైట్లు = పరిమాణం(sqlite3_vtab*)*
                                 ((sqlite3_int64)db->nVTrans + ARRAY_INCR);
    aVTrans = sqlite3DbRealloc(db, (శూన్య *)db->aVTrans, n_బైట్లు);
    ఒకవేళ( !aVTrans ){
      తిరిగి SQLITE_NOMEM_BKPT;
    }
    memset(&aVTrans[db->nVTrans], 0, పరిమాణం(sqlite3_vtab *)*ARRAY_INCR);
    db->aVTrans = aVTrans;
  }

  తిరిగి SQLITE_OK;
}

/*
** Add the virtual table pVTab to the array sqlite3.aVTrans[]. Space should
** have already been reserved using growVTrans().
*/
స్థిర శూన్య కూడు_గమ్యం_VTrans(sqlite3 *db, VTable *pVTab){
  /* Add pVtab to the end of sqlite3.aVTrans */
  db->aVTrans[db->nVTrans++] = pVTab;
  sqlite3_Vtab_తాళం(pVTab);
}

/*
** This function is invoked by the vdbe to call the xCreate method
** of the virtual table named zTab in database iDb.
**
** If an error occurs, *pzErr is set to point to an English language
** description of the error and an SQLITE_XXX error code is returned.
** In this case the caller must call sqlite3DbFree(db, ) on *pzErr.
*/
పూర్ణాంకం sqlite3VtabCallCreate(sqlite3 *db, పూర్ణాంకం iDb, స్థిరరాశి అక్షరం *zTab, అక్షరం **pzErr){
  పూర్ణాంకం rc = SQLITE_OK;
  పట్టిక *pTab;
  Module *pMod;
  స్థిరరాశి అక్షరం *zMod;

  pTab = sqlite3_కనుగొను_పట్టిక(db, zTab, db->aDb[iDb].zDbSName);
  ప్రతిజ్ఞా( pTab && IsVirtual(pTab) && !pTab->u.vtab.p );

  /* Locate the required virtual table module */
  zMod = pTab->u.vtab.azArg[0];
  pMod = (Module*)sqlite3_Hash_కనుగొను(&db->aModule, zMod);

  /* If the module has been registered and includes a Create method,
  ** invoke it now. If the module has not been registered, return an
  ** error. Otherwise, do nothing.
  */
  ఒకవేళ( pMod==0 || pMod->pModule->xCreate==0 || pMod->pModule->xDestroy==0 ){
    *pzErr = sqlite3MPrintf(db, "no such module: %s", zMod);
    rc = SQLITE_ERROR;
  }లేకపోతే{
    rc = vtabCallConstructor(db, pTab, pMod, pMod->pModule->xCreate, pzErr);
  }

  /* Justification of ALWAYS():  The xConstructor method is required to
  ** create a valid sqlite3_vtab if it returns SQLITE_OK. */
  ఒకవేళ( rc==SQLITE_OK && ALWAYS(sqlite3GetVTable(db, pTab)) ){
    rc = growVTrans(db);
    ఒకవేళ( rc==SQLITE_OK ){
      VTable *pVTab = sqlite3GetVTable(db, pTab);
      స్థిరరాశి sqlite3_module *pModule = pMod->pModule;
      పూర్ణాంకం iSvpt = db->nStatement + db->nSavepoint;
      కూడు_గమ్యం_VTrans(db, pVTab);
      /* As in sqlite3VtabBegin(): open the current savepoint on the new
      ** table, so that a ROLLBACK TO that undoes the CREATE also invokes
      ** its xRollbackTo method. */
      ఒకవేళ( iSvpt && pModule->i_సంస్కరణ>=2 && pModule->xSavepoint ){
        pVTab->iSavepoint = iSvpt;
        rc = pModule->xSavepoint(pVTab->pVtab, iSvpt-1);
      }
    }
  }

  తిరిగి rc;
}

/*
** This function is used to set the schema of a virtual table.  It is only
** valid to call this function from within the xCreate() or xConnect() of a
** virtual table module.
*/
పూర్ణాంకం sqlite3_declare_vtab(sqlite3 *db, స్థిరరాశి అక్షరం *z_Create_పట్టిక){
  VtabCtx *pCtx;
  పూర్ణాంకం rc = SQLITE_OK;
  పట్టిక *pTab;
  Parse sParse;
  పూర్ణాంకం initBusy;
  పూర్ణాంకం i;
  స్థిరరాశి గుర్తులేని అక్షరం *z;
  స్థిర స్థిరరాశి u8 aKeyword[] = { TK_CREATE, TK_పట్టిక, 0 };

#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !sqlite3SafetyCheckOk(db) || z_Create_పట్టిక==0 ){
    తిరిగి SQLITE_MISUSE_BKPT;
  }
#అయితే_ముగింపు

  /* Verify that the first two keywords in the CREATE TABLE statement
  ** really are "CREATE" and "TABLE".  If this is not the case, then
  ** sqlite3_declare_vtab() is being misused.
  */
  z = (స్థిరరాశి గుర్తులేని అక్షరం*)z_Create_పట్టిక;
  కొరకు(i=0; aKeyword[i]; i++){
    పూర్ణాంకం tokenType = 0;
    చేయి{
      z += sqlite3GetToken(z, &tokenType);
    }అయితే( tokenType==TK_SPACE || tokenType==TK_COMMENT );
    ఒకవేళ( tokenType!=aKeyword[i] ){
      sqlite3_Error_With_సందేశం(db, SQLITE_ERROR, "syntax error");
      తిరిగి SQLITE_ERROR;
    }
  }
 
  sqlite3_mutex_enter(db->mutex);
  pCtx = db->pVtabCtx;
  ఒకవేళ( !pCtx || pCtx->bDeclared ){
    sqlite3Error(db, SQLITE_MISUSE_BKPT);
    sqlite3_mutex_leave(db->mutex);
    తిరిగి SQLITE_MISUSE_BKPT;
  }

  pTab = pCtx->pTab;
  ప్రతిజ్ఞా( IsVirtual(pTab) );

  sqlite3ParseObjectInit(&sParse, db);
  sParse.e_Parse_బహుళకం = PARSE_బహుళకం_DECLARE_VTAB;
  sParse.disableTriggers = 1;
  /* We should never be able to reach this point while loading the
  ** schema.  Nevertheless, defend against that (turn off db->init.busy)
  ** in case a bug arises. */
  ప్రతిజ్ఞా( db->init.busy==0 );
  initBusy = db->init.busy;
  db->init.busy = 0;
  sParse.nQueryLoop = 1;
  ఒకవేళ( SQLITE_OK==sqlite3_నిరంతరం_Parser(&sParse, z_Create_పట్టిక) ){
    ప్రతిజ్ఞా( sParse.p_New_పట్టిక!=0 );
    ప్రతిజ్ఞా( !db->mallocFailed );
    ప్రతిజ్ఞా( Is_Ordinary_పట్టిక(sParse.p_New_పట్టిక) );
    ప్రతిజ్ఞా( sParse.z_Err_సందేశం==0 );
    ఒకవేళ( !pTab->a_నిలువు ){
      పట్టిక *pNew = sParse.p_New_పట్టిక;
      సూచీ *pIdx;
      pTab->a_నిలువు = pNew->a_నిలువు;
      ప్రతిజ్ఞా( Is_Ordinary_పట్టిక(pNew) );
      sqlite3_సమాసం_List_Delete(db, pNew->u.tab.pDfltList);
      pTab->nNVCol = pTab->n_నిలువు = pNew->n_నిలువు;
      pTab->tabFlags |= pNew->tabFlags & (TF_WithoutRowid|TF_NoVisibleRowid);
      pNew->n_నిలువు = 0;
      pNew->a_నిలువు = 0;
      ప్రతిజ్ఞా( pTab->p_సూచీ==0 );
      ప్రతిజ్ఞా( HasRowid(pNew) || sqlite3_Primary_కీలకం_సూచీ(pNew)!=0 );
      ఒకవేళ( !HasRowid(pNew)
       && pCtx->pVTable->pMod->pModule->xUpdate!=0
       && sqlite3_Primary_కీలకం_సూచీ(pNew)->n_కీలకం_నిలువు!=1
      ){
        /* WITHOUT ROWID virtual tables must either be read-only (xUpdate==0)
        ** or else must have a single-column PRIMARY KEY */
        rc = SQLITE_ERROR;
      }
      pIdx = pNew->p_సూచీ;
      ఒకవేళ( pIdx ){
        ప్రతిజ్ఞా( pIdx->p_తదుపరి==0 );
        pTab->p_సూచీ = pIdx;
        pNew->p_సూచీ = 0;
        pIdx->p_పట్టిక = pTab;
      }
    }
    pCtx->bDeclared = 1;
  }లేకపోతే{
    sqlite3_Error_With_సందేశం(db, SQLITE_ERROR,
          (sParse.z_Err_సందేశం ? "%s" : 0), sParse.z_Err_సందేశం);
    sqlite3DbFree(db, sParse.z_Err_సందేశం);
    rc = SQLITE_ERROR;
  }
  sParse.e_Parse_బహుళకం = PARSE_బహుళకం_సాధారణ;

  ఒకవేళ( sParse.pVdbe ){
    sqlite3VdbeFinalize(sParse.pVdbe);
  }
  sqlite3_Delete_పట్టిక(db, sParse.p_New_పట్టిక);
  sqlite3ParseObjectReset(&sParse);
  db->init.busy = initBusy;

  ప్రతిజ్ఞా( (rc&0xff)==rc );
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  తిరిగి rc;
}

/*
** This function is invoked by the vdbe to call the xDestroy method
** of the virtual table named zTab in database iDb. This occurs
** when a DROP TABLE is mentioned.
**
** This call is a no-op if zTab is not a virtual table.
*/
పూర్ణాంకం sqlite3VtabCallDestroy(sqlite3 *db, పూర్ణాంకం iDb, స్థిరరాశి అక్షరం *zTab){
  పూర్ణాంకం rc = SQLITE_OK;
  పట్టిక *pTab;

  pTab = sqlite3_కనుగొను_పట్టిక(db, zTab, db->aDb[iDb].zDbSName);
  ఒకవేళ( ALWAYS(pTab!=0)
   && ALWAYS(IsVirtual(pTab))
   && ALWAYS(pTab->u.vtab.p!=0)
  ){
    VTable *p;
    పూర్ణాంకం (*xDestroy)(sqlite3_vtab *);
    కొరకు(p=pTab->u.vtab.p; p; p=p->p_తదుపరి){
      ప్రతిజ్ఞా( p->pVtab );
      ఒకవేళ( p->pVtab->nRef>0 ){
        తిరిగి SQLITE_LOCKED;
      }
    }
    p = vtabDisconnectAll(db, pTab);
    xDestroy = p->pMod->pModule->xDestroy;
    ఒకవేళ( xDestroy==0 ) xDestroy = p->pMod->pModule->xDisconnect;
    ప్రతిజ్ఞా( xDestroy!=0 );
    pTab->nTabRef++;
    rc = xDestroy(p->pVtab);
    /* Remove the sqlite3_vtab* from the aVTrans[] array, if applicable */
    ఒకవేళ( rc==SQLITE_OK ){
      ప్రతిజ్ఞా( pTab->u.vtab.p==p && p->p_తదుపరి==0 );
      p->pVtab = 0;
      pTab->u.vtab.p = 0;
      sqlite3VtabUnlock(p);
    }
    sqlite3_Delete_పట్టిక(db, pTab);
  }

  తిరిగి rc;
}

/*
** This function invokes either the xRollback or xCommit method
** of each of the virtual tables in the sqlite3.aVTrans array. The method
** called is identified by the second argument, "offset", which is
** the offset of the method to call in the sqlite3_module structure.
**
** The array is cleared after invoking the callbacks.
*/
స్థిర శూన్య callFinaliser(sqlite3 *db, పూర్ణాంకం స్థానభ్రంశం){
  పూర్ణాంకం i;
  ఒకవేళ( db->aVTrans ){
    VTable **aVTrans = db->aVTrans;
    db->aVTrans = 0;
    కొరకు(i=0; i<db->nVTrans; i++){
      VTable *pVTab = aVTrans[i];
      sqlite3_vtab *p = pVTab->pVtab;
      ఒకవేళ( p ){
        పూర్ణాంకం (*x)(sqlite3_vtab *);
        x = *(పూర్ణాంకం (**)(sqlite3_vtab *))((అక్షరం *)p->pModule + స్థానభ్రంశం);
        ఒకవేళ( x ) x(p);
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
పూర్ణాంకం sqlite3VtabSync(sqlite3 *db, Vdbe *p){
  పూర్ణాంకం i;
  పూర్ణాంకం rc = SQLITE_OK;
  VTable **aVTrans = db->aVTrans;

  db->aVTrans = 0;
  కొరకు(i=0; rc==SQLITE_OK && i<db->nVTrans; i++){
    పూర్ణాంకం (*x)(sqlite3_vtab *);
    sqlite3_vtab *pVtab = aVTrans[i]->pVtab;
    ఒకవేళ( pVtab && (x = pVtab->pModule->xSync)!=0 ){
      rc = x(pVtab);
      sqlite3VtabImportErrmsg(p, pVtab);
    }
  }
  db->aVTrans = aVTrans;
  తిరిగి rc;
}

/*
** Invoke the xRollback method of all virtual tables in the
** sqlite3.aVTrans array. Then clear the array itself.
*/
పూర్ణాంకం sqlite3VtabRollback(sqlite3 *db){
  callFinaliser(db, offsetof(sqlite3_module,xRollback));
  తిరిగి SQLITE_OK;
}

/*
** Invoke the xCommit method of all virtual tables in the
** sqlite3.aVTrans array. Then clear the array itself.
*/
పూర్ణాంకం sqlite3VtabCommit(sqlite3 *db){
  callFinaliser(db, offsetof(sqlite3_module,xCommit));
  తిరిగి SQLITE_OK;
}

/*
** If the virtual table pVtab supports the transaction interface
** (xBegin/xRollback/xCommit and optionally xSync) and a transaction is
** not currently open, invoke the xBegin method now.
**
** If the xBegin call is successful, place the sqlite3_vtab pointer
** in the sqlite3.aVTrans array.
*/
పూర్ణాంకం sqlite3VtabBegin(sqlite3 *db, VTable *pVTab){
  పూర్ణాంకం rc = SQLITE_OK;
  స్థిరరాశి sqlite3_module *pModule;

  /* Special case: If db->aVTrans is NULL and db->nVTrans is greater
  ** than zero, then this function is being called from within a
  ** virtual module xSync() callback. It is illegal to write to
  ** virtual module tables in this case, so return SQLITE_LOCKED.
  */
  ఒకవేళ( sqlite3_Vtab_లోపల_Sync(db) ){
    తిరిగి SQLITE_LOCKED;
  }
  ఒకవేళ( !pVTab ){
    తిరిగి SQLITE_OK;
  }
  pModule = pVTab->pVtab->pModule;

  ఒకవేళ( pModule->xBegin ){
    పూర్ణాంకం i;

    /* If pVtab is already in the aVTrans array, return early */
    కొరకు(i=0; i<db->nVTrans; i++){
      ఒకవేళ( db->aVTrans[i]==pVTab ){
        తిరిగి SQLITE_OK;
      }
    }

    /* Invoke the xBegin method. If successful, add the vtab to the
    ** sqlite3.aVTrans[] array. */
    rc = growVTrans(db);
    ఒకవేళ( rc==SQLITE_OK ){
      rc = pModule->xBegin(pVTab->pVtab);
      ఒకవేళ( rc==SQLITE_OK ){
        పూర్ణాంకం iSvpt = db->nStatement + db->nSavepoint;
        కూడు_గమ్యం_VTrans(db, pVTab);
        ఒకవేళ( iSvpt && pModule->xSavepoint ){
          pVTab->iSavepoint = iSvpt;
          rc = pModule->xSavepoint(pVTab->pVtab, iSvpt-1);
        }
      }
    }
  }
  తిరిగి rc;
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
పూర్ణాంకం sqlite3VtabSavepoint(sqlite3 *db, పూర్ణాంకం op, పూర్ణాంకం iSavepoint){
  పూర్ణాంకం rc = SQLITE_OK;

  ప్రతిజ్ఞా( op==SAVEPOINT_RELEASE||op==SAVEPOINT_ROLLBACK||op==SAVEPOINT_BEGIN );
  ప్రతిజ్ఞా( iSavepoint>=-1 );
  ఒకవేళ( db->aVTrans ){
    పూర్ణాంకం i;
    కొరకు(i=0; rc==SQLITE_OK && i<db->nVTrans; i++){
      VTable *pVTab = db->aVTrans[i];
      స్థిరరాశి sqlite3_module *pMod = pVTab->pMod->pModule;
      ఒకవేళ( pVTab->pVtab && pMod->i_సంస్కరణ>=2 ){
        పూర్ణాంకం (*x_పద్ధతి)(sqlite3_vtab *, పూర్ణాంకం);
        sqlite3_Vtab_తాళం(pVTab);
        ఎంపిక( op ){
          స్థితి SAVEPOINT_BEGIN:
            x_పద్ధతి = pMod->xSavepoint;
            pVTab->iSavepoint = iSavepoint+1;
            ఆపు;
          స్థితి SAVEPOINT_ROLLBACK:
            x_పద్ధతి = pMod->x_Rollback_గమ్యం;
            ఆపు;
          అప్రమేయ:
            x_పద్ధతి = pMod->xRelease;
            ఆపు;
        }
        ఒకవేళ( x_పద్ధతి && pVTab->iSavepoint>iSavepoint ){
          u64 savedFlags = (db->flags & SQLITE_Defensive);
          db->flags &= ~(u64)SQLITE_Defensive;
          rc = x_పద్ధతి(pVTab->pVtab, iSavepoint);
          db->flags |= savedFlags;
        }
        sqlite3VtabUnlock(pVTab);
      }
    }
  }
  తిరిగి rc;
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
  పూర్ణాంకం nArg,       /* Number of arguments to the function */
  సమాసం *p_సమాసం     /* First argument to the function */
){
  పట్టిక *pTab;
  sqlite3_vtab *pVtab;
  sqlite3_module *pMod;
  శూన్య (*xSFunc)(sqlite3_context*,పూర్ణాంకం,sqlite3_విలువ**) = 0;
  శూన్య *pArg = 0;
  FuncDef *pNew;
  పూర్ణాంకం rc = 0;

  /* Check to see the left operand is a column in a virtual table */
  ఒకవేళ( NEVER(p_సమాసం==0) ) తిరిగి pDef;
  ఒకవేళ( p_సమాసం->op!=TK_స్తంభం ) తిరిగి pDef;
  ప్రతిజ్ఞా( సమాసం_Use_YTab(p_సమాసం) );
  pTab = p_సమాసం->y.pTab;
  ఒకవేళ( NEVER(pTab==0) ) తిరిగి pDef;
  ఒకవేళ( !IsVirtual(pTab) ) తిరిగి pDef;
  pVtab = sqlite3GetVTable(db, pTab)->pVtab;
  ప్రతిజ్ఞా( pVtab!=0 );
  ప్రతిజ్ఞా( pVtab->pModule!=0 );
  pMod = (sqlite3_module *)pVtab->pModule;
  ఒకవేళ( pMod->x_కనుగొను_Function==0 ) తిరిగి pDef;

  /* Call the xFindFunction method on the virtual table implementation
  ** to see if the implementation wants to overload this function.
  **
  ** Though undocumented, we have historically always invoked xFindFunction
  ** with an all lower-case function name.  Continue in this tradition to
  ** avoid any chance of an incompatibility.
  */
#నిర్వచితమైతే SQLITE_DEBUG
  {
    పూర్ణాంకం i;
    కొరకు(i=0; pDef->zName[i]; i++){
      గుర్తులేని అక్షరం x = (గుర్తులేని అక్షరం)pDef->zName[i];
      ప్రతిజ్ఞా( x==sqlite3_పై_గమ్యం_క్రింది[x] );
    }
  }
#అయితే_ముగింపు
  rc = pMod->x_కనుగొను_Function(pVtab, nArg, pDef->zName, &xSFunc, &pArg);
  ఒకవేళ( rc==0 ){
    తిరిగి pDef;
  }

  /* Create a new ephemeral function definition for the overloaded
  ** function */
  pNew = sqlite3DbMallocZero(db, పరిమాణం(*pNew)
                             + sqlite3Strlen30(pDef->zName) + 1);
  ఒకవేళ( pNew==0 ){
    తిరిగి pDef;
  }
  *pNew = *pDef;
  pNew->zName = (స్థిరరాశి అక్షరం*)&pNew[1];
  జ్ఞాపకం_నకలు((అక్షరం*)&pNew[1], pDef->zName, sqlite3Strlen30(pDef->zName)+1);
  pNew->xSFunc = xSFunc;
  pNew->p_User_దత్తాంశం = pArg;
  pNew->funcFlags |= SQLITE_FUNC_EPHEM;
  తిరిగి pNew;
}

/*
** Make sure virtual table pTab is contained in the pParse->apVirtualLock[]
** array so that an OP_VBegin will get generated for it.  Add pTab to the
** array if it is missing.  If pTab is already in the array, this routine
** is a no-op.
*/
శూన్య sqlite3VtabMakeWritable(Parse *pParse, పట్టిక *pTab){
  Parse *pToplevel = sqlite3ParseToplevel(pParse);
  పూర్ణాంకం i, n;
  పట్టిక **ap_Vtab_తాళం;

  ప్రతిజ్ఞా( IsVirtual(pTab) );
  కొరకు(i=0; i<pToplevel->n_Vtab_తాళం; i++){
    ఒకవేళ( pTab==pToplevel->ap_Vtab_తాళం[i] ) తిరిగి;
  }
  n = (pToplevel->n_Vtab_తాళం+1)*పరిమాణం(pToplevel->ap_Vtab_తాళం[0]);
  ap_Vtab_తాళం = sqlite3Realloc(pToplevel->ap_Vtab_తాళం, n);
  ఒకవేళ( ap_Vtab_తాళం ){
    pToplevel->ap_Vtab_తాళం = ap_Vtab_తాళం;
    pToplevel->ap_Vtab_తాళం[pToplevel->n_Vtab_తాళం++] = pTab;
  }లేకపోతే{
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
పూర్ణాంకం sqlite3_Vtab_Eponymous_పట్టిక_Init(Parse *pParse, Module *pMod){
  స్థిరరాశి sqlite3_module *pModule = pMod->pModule;
  పట్టిక *pTab;
  అక్షరం *zErr = 0;
  పూర్ణాంకం rc;
  sqlite3 *db = pParse->db;
  ఒకవేళ( pMod->pEpoTab ) తిరిగి 1;
  ఒకవేళ( pModule->xCreate!=0 && pModule->xCreate!=pModule->xConnect ) తిరిగి 0;
  pTab = sqlite3DbMallocZero(db, పరిమాణం(పట్టిక));
  ఒకవేళ( pTab==0 ) తిరిగి 0;
  pTab->zName = sqlite3DbStrDup(db, pMod->zName);
  ఒకవేళ( pTab->zName==0 ){
    sqlite3DbFree(db, pTab);
    తిరిగి 0;
  }
  pMod->pEpoTab = pTab;
  pTab->nTabRef = 1;
  pTab->eTabType = TABTYP_VTAB;
  pTab->pSchema = db->aDb[0].pSchema;
  ప్రతిజ్ఞా( pTab->u.vtab.nArg==0 );
  pTab->iPKey = -1;
  pTab->tabFlags |= TF_Eponymous;
  కూడు_Module_వాదం(pParse, pTab, sqlite3DbStrDup(db, pTab->zName));
  కూడు_Module_వాదం(pParse, pTab, 0);
  కూడు_Module_వాదం(pParse, pTab, sqlite3DbStrDup(db, pTab->zName));
  db->n_Schema_తాళం++;
  rc = vtabCallConstructor(db, pTab, pMod, pModule->xConnect, &zErr);
  db->n_Schema_తాళం--;
  ఒకవేళ( rc ){
    sqlite3_Error_సందేశం(pParse, "%s", zErr);
    pParse->rc = rc;
    sqlite3DbFree(db, zErr);
    sqlite3_Vtab_Eponymous_పట్టిక_Clear(db, pMod);
  }
  తిరిగి 1;
}

/*
** Erase the eponymous virtual table instance associated with
** virtual table module pMod, if it exists.
*/
శూన్య sqlite3_Vtab_Eponymous_పట్టిక_Clear(sqlite3 *db, Module *pMod){
  పట్టిక *pTab = pMod->pEpoTab;
  ఒకవేళ( pTab!=0 ){
    /* Mark the table as Ephemeral prior to deleting it, so that the
    ** sqlite3DeleteTable() routine will know that it is not stored in
    ** the schema. */
    pTab->tabFlags |= TF_Ephemeral;
    sqlite3_Delete_పట్టిక(db, pTab);
    pMod->pEpoTab = 0;
  }
}

/*
** Erase all eponymous virtual table instances associated with database
** handle db.  
*/
శూన్య sqlite3_Vtab_Eponymous_పట్టిక_Clear_All(sqlite3 *db){
  HashElem *pElem;
  కొరకు(pElem=sqlite_Hash_మొదటి(&db->aModule); pElem; pElem=sqlite_Hash_తదుపరి(pElem)){
    sqlite3_Vtab_Eponymous_పట్టిక_Clear(db, (Module*)sqlite_Hash_దత్తాంశం(pElem));
  }
}

/*
** Return the ON CONFLICT resolution mode in effect for the virtual
** table update operation currently in progress.
**
** The results of this routine are undefined unless it is called from
** within an xUpdate method.
*/
పూర్ణాంకం sqlite3_vtab_on_conflict(sqlite3 *db){
  స్థిర స్థిరరాశి గుర్తులేని అక్షరం aMap[] = {
    SQLITE_ROLLBACK, SQLITE_ABORT, SQLITE_FAIL, SQLITE_IGNORE, SQLITE_REPLACE
  };
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !sqlite3SafetyCheckOk(db) ) తిరిగి SQLITE_MISUSE_BKPT;
#అయితే_ముగింపు
  ప్రతిజ్ఞా( OE_Rollback==1 && OE_Abort==2 && OE_Fail==3 );
  ప్రతిజ్ఞా( OE_Ignore==4 && OE_Replace==5 );
  ప్రతిజ్ఞా( db->vtabOnConflict>=1 && db->vtabOnConflict<=5 );
  తిరిగి (పూర్ణాంకం)aMap[db->vtabOnConflict-1];
}

/*
** Call from within the xCreate() or xConnect() methods to provide
** the SQLite core with additional information about the behavior
** of the virtual table being implemented.
*/
పూర్ణాంకం sqlite3_vtab_config(sqlite3 *db, పూర్ణాంకం op, ...){
  va_list ap;
  పూర్ణాంకం rc = SQLITE_OK;
  VtabCtx *p;

#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !sqlite3SafetyCheckOk(db) ) తిరిగి SQLITE_MISUSE_BKPT;
#అయితే_ముగింపు
  sqlite3_mutex_enter(db->mutex);
  p = db->pVtabCtx;
  ఒకవేళ( !p ){
    rc = SQLITE_MISUSE_BKPT;
  }లేకపోతే{
    ప్రతిజ్ఞా( p->pTab==0 || IsVirtual(p->pTab) );
    va_start(ap, op);
    ఎంపిక( op ){
      స్థితి SQLITE_VTAB_CONSTRAINT_SUPPORT: {
        p->pVTable->bConstraint = (u8)va_arg(ap, పూర్ణాంకం);
        ఆపు;
      }
      స్థితి SQLITE_VTAB_INNOCUOUS: {
        p->pVTable->eVtabRisk = SQLITE_VTABRISK_దిగువ;
        ఆపు;
      }
      స్థితి SQLITE_VTAB_DIRECTONLY: {
        p->pVTable->eVtabRisk = SQLITE_VTABRISK_ఎగువ;
        ఆపు;
      }
      స్థితి SQLITE_VTAB_ఉపయోగిస్తుంది_ALL_SCHEMAS: {
        p->pVTable->bAllSchemas = 1;
        ఆపు;
      }
      అప్రమేయ: {
        rc = SQLITE_MISUSE_BKPT;
        ఆపు;
      }
    }
    va_end(ap);
  }

  ఒకవేళ( rc!=SQLITE_OK ) sqlite3Error(db, rc);
  sqlite3_mutex_leave(db->mutex);
  తిరిగి rc;
}

#అయితే_ముగింపు /* SQLITE_OMIT_VIRTUALTABLE */
