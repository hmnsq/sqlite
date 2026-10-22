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
#अगर_अघोषित SQLITE_OMIT_VIRTUALTABLE
#समावेश "sqliteInt.h"

/*
** Before a virtual table xCreate() or xConnect() method is invoked, the
** sqlite3.pVtabCtx member variable is set to point to an instance of
** this struct allocated on the stack. It is used by the implementation of
** the sqlite3_declare_vtab() and sqlite3_vtab_config() APIs, both of which
** are invoked only from within xCreate and xConnect methods.
*/
काष्ठा VtabCtx {
  VTable *pVTable;    /* The virtual table being constructed */
  Table *pTab;        /* The Table object to which the virtual table belongs */
  VtabCtx *pPrior;    /* Parent context (if any) */
  पूर्णांक bDeclared;      /* True after sqlite3_declare_vtab() is called */
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
  स्थिर अक्षर *zName,              /* Name assigned to this module */
  स्थिर sqlite3_module *pModule,  /* The definition of the module */
  व्योम *pAux,                     /* Context pointer for xCreate/xConnect */
  व्योम (*xDestroy)(व्योम *)        /* Module destructor function */
){
  Module *pMod;
  Module *pDel;
  अक्षर *zCopy;
  अगर( pModule==0 ){
    zCopy = (अक्षर*)zName;
    pMod = 0;
  }अन्यथा{
    पूर्णांक nName = sqlite3Strlen30(zName);
    pMod = (Module *)sqlite3Malloc(माप(Module) + nName + 1);
    अगर( pMod==0 ){
      sqlite3OomFault(db);
      वापस 0;
    }
    zCopy = (अक्षर *)(&pMod[1]);
    स_नकल(zCopy, zName, nName+1);
    pMod->zName = zCopy;
    pMod->pModule = pModule;
    pMod->pAux = pAux;
    pMod->xDestroy = xDestroy;
    pMod->pEpoTab = 0;
    pMod->nRefModule = 1;
  }
  pDel = (Module *)sqlite3_Hash_जोड़ो(&db->aModule,zCopy,(व्योम*)pMod);
  अगर( pDel ){
    अगर( pDel==pMod ){
      sqlite3OomFault(db);
      sqlite3DbFree(db, pDel);
      pMod = 0;
    }अन्यथा{
      sqlite3VtabEponymousTableClear(db, pDel);
      sqlite3VtabModuleUnref(db, pDel);
    }
  }
  वापस pMod;
}

/*
** The actual function that does the work of creating a new module.
** This function implements the sqlite3_create_module() and
** sqlite3_create_module_v2() interfaces.
*/
अटल पूर्णांक createModule(
  sqlite3 *db,                    /* Database in which module is registered */
  स्थिर अक्षर *zName,              /* Name assigned to this module */
  स्थिर sqlite3_module *pModule,  /* The definition of the module */
  व्योम *pAux,                     /* Context pointer for xCreate/xConnect */
  व्योम (*xDestroy)(व्योम *)        /* Module destructor function */
){
  पूर्णांक rc = SQLITE_OK;

  sqlite3_mutex_enter(db->mutex);
  (व्योम)sqlite3VtabCreateModule(db, zName, pModule, pAux, xDestroy);
  rc = sqlite3ApiExit(db, rc);
  अगर( rc!=SQLITE_OK && xDestroy ) xDestroy(pAux);
  sqlite3_mutex_leave(db->mutex);
  वापस rc;
}


/*
** External API function used to create a new virtual-table module.
*/
पूर्णांक sqlite3_create_module(
  sqlite3 *db,                    /* Database in which module is registered */
  स्थिर अक्षर *zName,              /* Name assigned to this module */
  स्थिर sqlite3_module *pModule,  /* The definition of the module */
  व्योम *pAux                      /* Context pointer for xCreate/xConnect */
){
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !sqlite3SafetyCheckOk(db) || zName==0 ) वापस SQLITE_MISUSE_BKPT;
#पूर्ण_अगर
  वापस createModule(db, zName, pModule, pAux, 0);
}

/*
** External API function used to create a new virtual-table module.
*/
पूर्णांक sqlite3_create_module_v2(
  sqlite3 *db,                    /* Database in which module is registered */
  स्थिर अक्षर *zName,              /* Name assigned to this module */
  स्थिर sqlite3_module *pModule,  /* The definition of the module */
  व्योम *pAux,                     /* Context pointer for xCreate/xConnect */
  व्योम (*xDestroy)(व्योम *)        /* Module destructor function */
){
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !sqlite3SafetyCheckOk(db) || zName==0 ) वापस SQLITE_MISUSE_BKPT;
#पूर्ण_अगर
  वापस createModule(db, zName, pModule, pAux, xDestroy);
}

/*
** External API to drop all virtual-table modules, except those named
** on the azNames list.
*/
पूर्णांक sqlite3_drop_modules(sqlite3 *db, स्थिर अक्षर** azNames){
  HashElem *pThis, *p_अग्र;
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !sqlite3SafetyCheckOk(db) ) वापस SQLITE_MISUSE_BKPT;
#पूर्ण_अगर
  sqlite3_mutex_enter(db->mutex);
  क्रम(pThis=sqliteHashFirst(&db->aModule); pThis; pThis=p_अग्र){
    Module *pMod = (Module*)sqlite_Hash_तथ्य(pThis);
    p_अग्र = sqlite_Hash_अग्र(pThis);
    अगर( azNames ){
      पूर्णांक ii;
      क्रम(ii=0; azNames[ii]!=0 && म_भेद(azNames[ii],pMod->zName)!=0; ii++){}
      अगर( azNames[ii]!=0 ) जारी;
    }
    createModule(db, pMod->zName, 0, 0, 0);
  }
  sqlite3_mutex_leave(db->mutex);
  वापस SQLITE_OK;
}

/*
** Decrement the reference count on a Module object.  Destroy the
** module when the reference count reaches zero.
*/
व्योम sqlite3VtabModuleUnref(sqlite3 *db, Module *pMod){
  निश्चित( pMod->nRefModule>0 );
  pMod->nRefModule--;
  अगर( pMod->nRefModule==0 ){
    अगर( pMod->xDestroy ){
      pMod->xDestroy(pMod->pAux);
    }
    निश्चित( pMod->pEpoTab==0 );
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
व्योम sqlite3VtabLock(VTable *pVTab){
  pVTab->nRef++;
}


/*
** pTab is a pointer to a Table structure representing a virtual-table.
** Return a pointer to the VTable object used by connection db to access
** this virtual-table, if one has been created, or NULL otherwise.
*/
VTable *sqlite3GetVTable(sqlite3 *db, Table *pTab){
  VTable *pVtab;
  निश्चित( IsVirtual(pTab) );
  क्रम(pVtab=pTab->u.vtab.p; pVtab && pVtab->db!=db; pVtab=pVtab->p_अग्र);
  वापस pVtab;
}

/*
** Decrement the ref-count on a virtual table object. When the ref-count
** reaches zero, call the xDisconnect() method to delete the object.
*/
व्योम sqlite3VtabUnlock(VTable *pVTab){
  sqlite3 *db = pVTab->db;

  निश्चित( db );
  निश्चित( pVTab->nRef>0 );
  निश्चित( db->eOpenState==SQLITE_STATE_OPEN
       || db->eOpenState==SQLITE_STATE_ZOMBIE );

  pVTab->nRef--;
  अगर( pVTab->nRef==0 ){
    sqlite3_vtab *p = pVTab->pVtab;
    अगर( p ){
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
अटल VTable *vtabDisconnectAll(sqlite3 *db, Table *p){
  VTable *pRet = 0;
  VTable *pVTable;

  निश्चित( IsVirtual(p) );
  pVTable = p->u.vtab.p;
  p->u.vtab.p = 0;

  /* Assert that the mutex (if any) associated with the BtShared database
  ** that contains table p is held by the caller. See header comments
  ** above function sqlite3VtabUnlockList() for an explanation of why
  ** this makes it safe to access the sqlite3.pDisconnect list of any
  ** database connection that may have an entry in the p->u.vtab.p list.
  */
  निश्चित( db==0 || sqlite3SchemaMutexHeld(db, 0, p->pSchema) );

  जबतक( pVTable ){
    sqlite3 *db2 = pVTable->db;
    VTable *p_अग्र = pVTable->p_अग्र;
    निश्चित( db2 );
    अगर( db2==db ){
      pRet = pVTable;
      p->u.vtab.p = pRet;
      pRet->p_अग्र = 0;
    }अन्यथा{
      pVTable->p_अग्र = db2->pDisconnect;
      db2->pDisconnect = pVTable;
    }
    pVTable = p_अग्र;
  }

  निश्चित( !db || pRet );
  वापस pRet;
}

/*
** Table *p is a virtual table. This function removes the VTable object
** for table *p associated with database connection db from the linked
** list in p->pVTab. It also decrements the VTable ref count. This is
** used when closing database connection db to free all of its VTable
** objects without disturbing the rest of the Schema object (which may
** be being used by other shared-cache connections).
*/
व्योम sqlite3VtabDisconnect(sqlite3 *db, Table *p){
  VTable **ppVTab;

  निश्चित( IsVirtual(p) );
  निश्चित( sqlite3BtreeHoldsAllMutexes(db) );
  निश्चित( sqlite3_mutex_held(db->mutex) );

  क्रम(ppVTab=&p->u.vtab.p; *ppVTab; ppVTab=&(*ppVTab)->p_अग्र){
    अगर( (*ppVTab)->db==db  ){
      VTable *pVTab = *ppVTab;
      *ppVTab = pVTab->p_अग्र;
      sqlite3VtabUnlock(pVTab);
      अवरोध;
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
व्योम sqlite3VtabUnlockList(sqlite3 *db){
  VTable *p = db->pDisconnect;

  निश्चित( sqlite3BtreeHoldsAllMutexes(db) );
  निश्चित( sqlite3_mutex_held(db->mutex) );

  अगर( p ){
    db->pDisconnect = 0;
    करो {
      VTable *p_अग्र = p->p_अग्र;
      sqlite3VtabUnlock(p);
      p = p_अग्र;
    }जबतक( p );
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
व्योम sqlite3VtabClear(sqlite3 *db, Table *p){
  निश्चित( IsVirtual(p) );
  निश्चित( db!=0 );
  अगर( db->pnBytesFreed==0 ) vtabDisconnectAll(0, p);
  अगर( p->u.vtab.azArg ){
    पूर्णांक i;
    क्रम(i=0; i<p->u.vtab.nArg; i++){
      अगर( i!=1 ) sqlite3DbFree(db, p->u.vtab.azArg[i]);
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
अटल व्योम addModuleArgument(Parse *pParse, Table *pTable, अक्षर *zArg){
  sqlite3_int64 nBytes;
  अक्षर **azModuleArg;
  sqlite3 *db = pParse->db;

  निश्चित( IsVirtual(pTable) );
  nBytes = माप(अक्षर *)*(2+pTable->u.vtab.nArg);
  अगर( pTable->u.vtab.nArg+3>=db->aLimit[SQLITE_LIMIT_COLUMN] ){
    sqlite3_Error_संदेश(pParse, "too many columns on %s", pTable->zName);
  }
  azModuleArg = sqlite3DbRealloc(db, pTable->u.vtab.azArg, nBytes);
  अगर( azModuleArg==0 ){
    sqlite3DbFree(db, zArg);
  }अन्यथा{
    पूर्णांक i = pTable->u.vtab.nArg++;
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
व्योम sqlite3VtabBeginParse(
  Parse *pParse,        /* Parsing context */
  Token *pName1,        /* Name of new table, or database name */
  Token *pName2,        /* Name of new table or NULL */
  Token *pModuleName,   /* Name of the module for the virtual table */
  पूर्णांक ifNotExists       /* No error if the table already exists */
){
  Table *pTable;        /* The new virtual table */
  sqlite3 *db;          /* Database connection */

  sqlite3StartTable(pParse, pName1, pName2, 0, 0, 1, ifNotExists);
  pTable = pParse->pNewTable;
  अगर( pTable==0 ) वापस;
  निश्चित( 0==pTable->p_सूचकांक );
  pTable->eTabType = TABTYP_VTAB;

  db = pParse->db;

  निश्चित( pTable->u.vtab.nArg==0 );
  addModuleArgument(pParse, pTable, sqlite3NameFromToken(db, pModuleName));
  addModuleArgument(pParse, pTable, 0);
  addModuleArgument(pParse, pTable, sqlite3DbStrDup(db, pTable->zName));
  निश्चित( (pParse->sNameToken.z==pName2->z && pName2->z!=0)
       || (pParse->sNameToken.z==pName1->z && pName2->z==0)
  );
  pParse->sNameToken.n = (पूर्णांक)(
      &pModuleName->z[pModuleName->n] - pParse->sNameToken.z
  );

#अगर_अघोषित SQLITE_OMIT_AUTHORIZATION
  /* Creating a virtual table invokes the authorization callback twice.
  ** The first invocation, to obtain permission to INSERT a row into the
  ** sqlite_schema table, has already been made by sqlite3StartTable().
  ** The second call, to obtain permission to create the table, is made now.
  */
  अगर( pTable->u.vtab.azArg ){
    पूर्णांक iDb = sqlite3_Schema_To_सूचकांक(db, pTable->pSchema);
    निश्चित( iDb>=0 ); /* The database the table is being created in */
    sqlite3AuthCheck(pParse, SQLITE_CREATE_VTABLE, pTable->zName,
            pTable->u.vtab.azArg[0], pParse->db->aDb[iDb].zDbSName);
  }
#पूर्ण_अगर
}

/*
** This routine takes the module argument that has been accumulating
** in pParse->zArg[] and appends it to the list of arguments on the
** virtual table currently under construction in pParse->pTable.
*/
अटल व्योम addArgumentToVtab(Parse *pParse){
  अगर( pParse->sArg.z && pParse->pNewTable ){
    स्थिर अक्षर *z = (स्थिर अक्षर*)pParse->sArg.z;
    पूर्णांक n = pParse->sArg.n;
    sqlite3 *db = pParse->db;
    addModuleArgument(pParse, pParse->pNewTable, sqlite3DbStrNDup(db, z, n));
  }
}

/*
** The parser calls this routine after the CREATE VIRTUAL TABLE statement
** has been completely parsed.
*/
व्योम sqlite3VtabFinishParse(Parse *pParse, Token *pEnd){
  Table *pTab = pParse->pNewTable;  /* The table being constructed */
  sqlite3 *db = pParse->db;         /* The database connection */

  अगर( pTab==0 ) वापस;
  निश्चित( IsVirtual(pTab) );
  addArgumentToVtab(pParse);
  pParse->sArg.z = 0;
  अगर( pTab->u.vtab.nArg<1 ) वापस;

  /* If the CREATE VIRTUAL TABLE statement is being entered for the
  ** first time (in other words if the virtual table is actually being
  ** created now instead of just being read out of sqlite_schema) then
  ** do additional initialization work and store the statement text
  ** in the sqlite_schema table.
  */
  अगर( !db->init.busy ){
    अक्षर *zStmt;
    अक्षर *zWhere;
    पूर्णांक iDb;
    पूर्णांक iReg;
    Vdbe *v;

    sqlite3MayAbort(pParse);

    /* Compute the complete text of the CREATE VIRTUAL TABLE statement */
    अगर( pEnd ){
      pParse->sNameToken.n = (पूर्णांक)(pEnd->z - pParse->sNameToken.z) + pEnd->n;
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
    iDb = sqlite3_Schema_To_सूचकांक(db, pTab->pSchema);
    निश्चित( pParse->isCreate );
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
  }अन्यथा{
    /* If we are rereading the sqlite_schema table create the in-memory
    ** record of the table. */
    Table *pOld;
    Schema *pSchema = pTab->pSchema;
    स्थिर अक्षर *zName = pTab->zName;
    निश्चित( zName!=0 );
    sqlite3MarkAllShadowTablesOf(db, pTab);
    pOld = sqlite3_Hash_जोड़ो(&pSchema->tblHash, zName, pTab);
    अगर( pOld ){
      sqlite3OomFault(db);
      निश्चित( pTab==pOld );  /* Malloc must have failed inside HashInsert() */
      वापस;
    }
    pParse->pNewTable = 0;
  }
}

/*
** The parser calls this routine when it sees the first token
** of an argument to the module name in a CREATE VIRTUAL TABLE statement.
*/
व्योम sqlite3VtabArgInit(Parse *pParse){
  addArgumentToVtab(pParse);
  pParse->sArg.z = 0;
  pParse->sArg.n = 0;
}

/*
** The parser calls this routine for each token after the first token
** in an argument to the module name in a CREATE VIRTUAL TABLE statement.
*/
व्योम sqlite3VtabArgExtend(Parse *pParse, Token *p){
  Token *pArg = &pParse->sArg;
  अगर( pArg->z==0 ){
    pArg->z = p->z;
    pArg->n = p->n;
  }अन्यथा{
    निश्चित(pArg->z <= p->z);
    pArg->n = (पूर्णांक)(&p->z[p->n] - pArg->z);
  }
}

/*
** Invoke a virtual table constructor (either xCreate or xConnect). The
** pointer to the function to invoke is passed as the fourth parameter
** to this procedure.
*/
अटल पूर्णांक vtabCallConstructor(
  sqlite3 *db,
  Table *pTab,
  Module *pMod,
  पूर्णांक (*xConstruct)(sqlite3*,व्योम*,पूर्णांक,स्थिर अक्षर*स्थिर*,sqlite3_vtab**,अक्षर**),
  अक्षर **pzErr
){
  VtabCtx sCtx;
  VTable *pVTable;
  पूर्णांक rc;
  स्थिर अक्षर *स्थिर*azArg;
  पूर्णांक nArg = pTab->u.vtab.nArg;
  अक्षर *zErr = 0;
  अक्षर *zModuleName;
  पूर्णांक iDb;
  VtabCtx *pCtx;

  निश्चित( IsVirtual(pTab) );
  azArg = (स्थिर अक्षर *स्थिर*)pTab->u.vtab.azArg;

  /* Check that the virtual-table is not already being initialized */
  क्रम(pCtx=db->pVtabCtx; pCtx; pCtx=pCtx->pPrior){
    अगर( pCtx->pTab==pTab ){
      *pzErr = sqlite3MPrintf(db,
          "vtable constructor called recursively: %s", pTab->zName
      );
      वापस SQLITE_LOCKED;
    }
  }

  zModuleName = sqlite3DbStrDup(db, pTab->zName);
  अगर( !zModuleName ){
    वापस SQLITE_NOMEM_BKPT;
  }

  pVTable = sqlite3MallocZero(माप(VTable));
  अगर( !pVTable ){
    sqlite3OomFault(db);
    sqlite3DbFree(db, zModuleName);
    वापस SQLITE_NOMEM_BKPT;
  }
  pVTable->db = db;
  pVTable->pMod = pMod;
  pVTable->eVtabRisk = SQLITE_VTABRISK_Normal;

  iDb = sqlite3_Schema_To_सूचकांक(db, pTab->pSchema);
  pTab->u.vtab.azArg[1] = db->aDb[iDb].zDbSName;

  /* Invoke the virtual table constructor */
  निश्चित( &db->pVtabCtx );
  निश्चित( xConstruct );
  sCtx.pTab = pTab;
  sCtx.pVTable = pVTable;
  sCtx.pPrior = db->pVtabCtx;
  sCtx.bDeclared = 0;
  db->pVtabCtx = &sCtx;
  pTab->nTabRef++;
  rc = xConstruct(db, pMod->pAux, nArg, azArg, &pVTable->pVtab, &zErr);
  निश्चित( pTab!=0 );
  निश्चित( pTab->nTabRef>1 || rc!=SQLITE_OK );
  sqlite3DeleteTable(db, pTab);
  db->pVtabCtx = sCtx.pPrior;
  अगर( rc==SQLITE_NOMEM ) sqlite3OomFault(db);
  निश्चित( sCtx.pTab==pTab );

  अगर( SQLITE_OK!=rc ){
    अगर( zErr==0 ){
      *pzErr = sqlite3MPrintf(db, "vtable constructor failed: %s", zModuleName);
    }अन्यथा {
      *pzErr = sqlite3MPrintf(db, "%s", zErr);
      sqlite3_free(zErr);
    }
    sqlite3DbFree(db, pVTable);
  }अन्यथा अगर( ALWAYS(pVTable->pVtab) ){
    /* Justification of ALWAYS():  A correct vtab constructor must allocate
    ** the sqlite3_vtab object if successful.  */
    स_रखो(pVTable->pVtab, 0, माप(pVTable->pVtab[0]));
    pVTable->pVtab->pModule = pMod->pModule;
    pMod->nRefModule++;
    pVTable->nRef = 1;
    अगर( sCtx.bDeclared==0 ){
      स्थिर अक्षर *zFormat = "vtable constructor did not declare schema: %s";
      *pzErr = sqlite3MPrintf(db, zFormat, zModuleName);
      sqlite3VtabUnlock(pVTable);
      rc = SQLITE_ERROR;
    }अन्यथा{
      पूर्णांक iCol;
      u16 oooHidden = 0;
      /* If everything went according to plan, link the new VTable structure
      ** into the linked list headed by pTab->u.vtab.p. Then loop through the
      ** columns of the table to see if any of them contain the token "hidden".
      ** If so, set the Column COLFLAG_HIDDEN flag and remove the token from
      ** the type string.  */
      pVTable->p_अग्र = pTab->u.vtab.p;
      pTab->u.vtab.p = pVTable;

      क्रम(iCol=0; iCol<pTab->nCol; iCol++){
        अक्षर *zType = sqlite3ColumnType(&pTab->aCol[iCol], "");
        पूर्णांक nType;
        पूर्णांक i = 0;
        nType = sqlite3Strlen30(zType);
        क्रम(i=0; i<nType; i++){
          अगर( 0==sqlite3StrNICmp("hidden", &zType[i], 6)
           && (i==0 || zType[i-1]==' ')
           && (zType[i+6]=='\0' || zType[i+6]==' ')
          ){
            अवरोध;
          }
        }
        अगर( i<nType ){
          पूर्णांक j;
          पूर्णांक nDel = 6 + (zType[i+6] ? 1 : 0);
          क्रम(j=i; (j+nDel)<=nType; j++){
            zType[j] = zType[j+nDel];
          }
          अगर( zType[i]=='\0' && i>0 ){
            निश्चित(zType[i-1]==' ');
            zType[i-1] = '\0';
          }
          pTab->aCol[iCol].colFlags |= COLFLAG_HIDDEN;
          pTab->tabFlags |= TF_HasHidden;
          oooHidden = TF_OOOHidden;
        }अन्यथा{
          pTab->tabFlags |= oooHidden;
        }
      }
    }
  }

  sqlite3DbFree(db, zModuleName);
  वापस rc;
}

/*
** This function is invoked by the parser to call the xConnect() method
** of the virtual table pTab. If an error occurs, an error code is returned
** and an error left in pParse.
**
** This call is a no-op if table pTab is not a virtual table.
*/
पूर्णांक sqlite3VtabCallConnect(Parse *pParse, Table *pTab){
  sqlite3 *db = pParse->db;
  स्थिर अक्षर *zMod;
  Module *pMod;
  पूर्णांक rc;

  निश्चित( pTab );
  निश्चित( IsVirtual(pTab) );
  अगर( sqlite3GetVTable(db, pTab) ){
    वापस SQLITE_OK;
  }

  /* Locate the required virtual table module */
  zMod = pTab->u.vtab.azArg[0];
  pMod = (Module*)sqlite3HashFind(&db->aModule, zMod);

  अगर( !pMod ){
    स्थिर अक्षर *zModule = pTab->u.vtab.azArg[0];
    sqlite3_Error_संदेश(pParse, "no such module: %s", zModule);
    rc = SQLITE_ERROR;
  }अन्यथा{
    अक्षर *zErr = 0;
    rc = vtabCallConstructor(db, pTab, pMod, pMod->pModule->xConnect, &zErr);
    अगर( rc!=SQLITE_OK ){
      sqlite3_Error_संदेश(pParse, "%s", zErr);
      pParse->rc = rc;
    }अन्यथा{
      sqlite3MarkAllShadowTablesOf(db, pTab);
    }
    sqlite3DbFree(db, zErr);
  }

  वापस rc;
}
/*
** Grow the db->aVTrans[] array so that there is room for at least one
** more v-table. Return SQLITE_NOMEM if a malloc fails, or SQLITE_OK otherwise.
*/
अटल पूर्णांक growVTrans(sqlite3 *db){
  स्थिर पूर्णांक ARRAY_INCR = 5;

  /* Grow the sqlite3.aVTrans array if required */
  अगर( (db->nVTrans%ARRAY_INCR)==0 ){
    VTable **aVTrans;
    sqlite3_int64 nBytes = माप(sqlite3_vtab*)*
                                 ((sqlite3_int64)db->nVTrans + ARRAY_INCR);
    aVTrans = sqlite3DbRealloc(db, (व्योम *)db->aVTrans, nBytes);
    अगर( !aVTrans ){
      वापस SQLITE_NOMEM_BKPT;
    }
    स_रखो(&aVTrans[db->nVTrans], 0, माप(sqlite3_vtab *)*ARRAY_INCR);
    db->aVTrans = aVTrans;
  }

  वापस SQLITE_OK;
}

/*
** Add the virtual table pVTab to the array sqlite3.aVTrans[]. Space should
** have already been reserved using growVTrans().
*/
अटल व्योम addToVTrans(sqlite3 *db, VTable *pVTab){
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
पूर्णांक sqlite3VtabCallCreate(sqlite3 *db, पूर्णांक iDb, स्थिर अक्षर *zTab, अक्षर **pzErr){
  पूर्णांक rc = SQLITE_OK;
  Table *pTab;
  Module *pMod;
  स्थिर अक्षर *zMod;

  pTab = sqlite3FindTable(db, zTab, db->aDb[iDb].zDbSName);
  निश्चित( pTab && IsVirtual(pTab) && !pTab->u.vtab.p );

  /* Locate the required virtual table module */
  zMod = pTab->u.vtab.azArg[0];
  pMod = (Module*)sqlite3HashFind(&db->aModule, zMod);

  /* If the module has been registered and includes a Create method,
  ** invoke it now. If the module has not been registered, return an
  ** error. Otherwise, do nothing.
  */
  अगर( pMod==0 || pMod->pModule->xCreate==0 || pMod->pModule->xDestroy==0 ){
    *pzErr = sqlite3MPrintf(db, "no such module: %s", zMod);
    rc = SQLITE_ERROR;
  }अन्यथा{
    rc = vtabCallConstructor(db, pTab, pMod, pMod->pModule->xCreate, pzErr);
  }

  /* Justification of ALWAYS():  The xConstructor method is required to
  ** create a valid sqlite3_vtab if it returns SQLITE_OK. */
  अगर( rc==SQLITE_OK && ALWAYS(sqlite3GetVTable(db, pTab)) ){
    rc = growVTrans(db);
    अगर( rc==SQLITE_OK ){
      VTable *pVTab = sqlite3GetVTable(db, pTab);
      स्थिर sqlite3_module *pModule = pMod->pModule;
      पूर्णांक iSvpt = db->nStatement + db->nSavepoint;
      addToVTrans(db, pVTab);
      /* As in sqlite3VtabBegin(): open the current savepoint on the new
      ** table, so that a ROLLBACK TO that undoes the CREATE also invokes
      ** its xRollbackTo method. */
      अगर( iSvpt && pModule->iVersion>=2 && pModule->xSavepoint ){
        pVTab->iSavepoint = iSvpt;
        rc = pModule->xSavepoint(pVTab->pVtab, iSvpt-1);
      }
    }
  }

  वापस rc;
}

/*
** This function is used to set the schema of a virtual table.  It is only
** valid to call this function from within the xCreate() or xConnect() of a
** virtual table module.
*/
पूर्णांक sqlite3_declare_vtab(sqlite3 *db, स्थिर अक्षर *zCreateTable){
  VtabCtx *pCtx;
  पूर्णांक rc = SQLITE_OK;
  Table *pTab;
  Parse sParse;
  पूर्णांक initBusy;
  पूर्णांक i;
  स्थिर अचिन्हित अक्षर *z;
  अटल स्थिर u8 aKeyword[] = { TK_CREATE, TK_TABLE, 0 };

#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !sqlite3SafetyCheckOk(db) || zCreateTable==0 ){
    वापस SQLITE_MISUSE_BKPT;
  }
#पूर्ण_अगर

  /* Verify that the first two keywords in the CREATE TABLE statement
  ** really are "CREATE" and "TABLE".  If this is not the case, then
  ** sqlite3_declare_vtab() is being misused.
  */
  z = (स्थिर अचिन्हित अक्षर*)zCreateTable;
  क्रम(i=0; aKeyword[i]; i++){
    पूर्णांक tokenType = 0;
    करो{
      z += sqlite3GetToken(z, &tokenType);
    }जबतक( tokenType==TK_SPACE || tokenType==TK_COMMENT );
    अगर( tokenType!=aKeyword[i] ){
      sqlite3_Error_With_संदेश(db, SQLITE_ERROR, "syntax error");
      वापस SQLITE_ERROR;
    }
  }
 
  sqlite3_mutex_enter(db->mutex);
  pCtx = db->pVtabCtx;
  अगर( !pCtx || pCtx->bDeclared ){
    sqlite3Error(db, SQLITE_MISUSE_BKPT);
    sqlite3_mutex_leave(db->mutex);
    वापस SQLITE_MISUSE_BKPT;
  }

  pTab = pCtx->pTab;
  निश्चित( IsVirtual(pTab) );

  sqlite3ParseObjectInit(&sParse, db);
  sParse.eParseMode = PARSE_MODE_DECLARE_VTAB;
  sParse.disableTriggers = 1;
  /* We should never be able to reach this point while loading the
  ** schema.  Nevertheless, defend against that (turn off db->init.busy)
  ** in case a bug arises. */
  निश्चित( db->init.busy==0 );
  initBusy = db->init.busy;
  db->init.busy = 0;
  sParse.nQueryLoop = 1;
  अगर( SQLITE_OK==sqlite3RunParser(&sParse, zCreateTable) ){
    निश्चित( sParse.pNewTable!=0 );
    निश्चित( !db->mallocFailed );
    निश्चित( IsOrdinaryTable(sParse.pNewTable) );
    निश्चित( sParse.z_Err_संदेश==0 );
    अगर( !pTab->aCol ){
      Table *pNew = sParse.pNewTable;
      सूचकांक *pIdx;
      pTab->aCol = pNew->aCol;
      निश्चित( IsOrdinaryTable(pNew) );
      sqlite3_व्यंजक_List_Delete(db, pNew->u.tab.pDfltList);
      pTab->nNVCol = pTab->nCol = pNew->nCol;
      pTab->tabFlags |= pNew->tabFlags & (TF_WithoutRowid|TF_NoVisibleRowid);
      pNew->nCol = 0;
      pNew->aCol = 0;
      निश्चित( pTab->p_सूचकांक==0 );
      निश्चित( HasRowid(pNew) || sqlite3_Primary_कुंजी_सूचकांक(pNew)!=0 );
      अगर( !HasRowid(pNew)
       && pCtx->pVTable->pMod->pModule->xUpdate!=0
       && sqlite3_Primary_कुंजी_सूचकांक(pNew)->n_कुंजी_Col!=1
      ){
        /* WITHOUT ROWID virtual tables must either be read-only (xUpdate==0)
        ** or else must have a single-column PRIMARY KEY */
        rc = SQLITE_ERROR;
      }
      pIdx = pNew->p_सूचकांक;
      अगर( pIdx ){
        निश्चित( pIdx->p_अग्र==0 );
        pTab->p_सूचकांक = pIdx;
        pNew->p_सूचकांक = 0;
        pIdx->pTable = pTab;
      }
    }
    pCtx->bDeclared = 1;
  }अन्यथा{
    sqlite3_Error_With_संदेश(db, SQLITE_ERROR,
          (sParse.z_Err_संदेश ? "%s" : 0), sParse.z_Err_संदेश);
    sqlite3DbFree(db, sParse.z_Err_संदेश);
    rc = SQLITE_ERROR;
  }
  sParse.eParseMode = PARSE_MODE_NORMAL;

  अगर( sParse.pVdbe ){
    sqlite3VdbeFinalize(sParse.pVdbe);
  }
  sqlite3DeleteTable(db, sParse.pNewTable);
  sqlite3ParseObjectReset(&sParse);
  db->init.busy = initBusy;

  निश्चित( (rc&0xff)==rc );
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  वापस rc;
}

/*
** This function is invoked by the vdbe to call the xDestroy method
** of the virtual table named zTab in database iDb. This occurs
** when a DROP TABLE is mentioned.
**
** This call is a no-op if zTab is not a virtual table.
*/
पूर्णांक sqlite3VtabCallDestroy(sqlite3 *db, पूर्णांक iDb, स्थिर अक्षर *zTab){
  पूर्णांक rc = SQLITE_OK;
  Table *pTab;

  pTab = sqlite3FindTable(db, zTab, db->aDb[iDb].zDbSName);
  अगर( ALWAYS(pTab!=0)
   && ALWAYS(IsVirtual(pTab))
   && ALWAYS(pTab->u.vtab.p!=0)
  ){
    VTable *p;
    पूर्णांक (*xDestroy)(sqlite3_vtab *);
    क्रम(p=pTab->u.vtab.p; p; p=p->p_अग्र){
      निश्चित( p->pVtab );
      अगर( p->pVtab->nRef>0 ){
        वापस SQLITE_LOCKED;
      }
    }
    p = vtabDisconnectAll(db, pTab);
    xDestroy = p->pMod->pModule->xDestroy;
    अगर( xDestroy==0 ) xDestroy = p->pMod->pModule->xDisconnect;
    निश्चित( xDestroy!=0 );
    pTab->nTabRef++;
    rc = xDestroy(p->pVtab);
    /* Remove the sqlite3_vtab* from the aVTrans[] array, if applicable */
    अगर( rc==SQLITE_OK ){
      निश्चित( pTab->u.vtab.p==p && p->p_अग्र==0 );
      p->pVtab = 0;
      pTab->u.vtab.p = 0;
      sqlite3VtabUnlock(p);
    }
    sqlite3DeleteTable(db, pTab);
  }

  वापस rc;
}

/*
** This function invokes either the xRollback or xCommit method
** of each of the virtual tables in the sqlite3.aVTrans array. The method
** called is identified by the second argument, "offset", which is
** the offset of the method to call in the sqlite3_module structure.
**
** The array is cleared after invoking the callbacks.
*/
अटल व्योम callFinaliser(sqlite3 *db, पूर्णांक offset){
  पूर्णांक i;
  अगर( db->aVTrans ){
    VTable **aVTrans = db->aVTrans;
    db->aVTrans = 0;
    क्रम(i=0; i<db->nVTrans; i++){
      VTable *pVTab = aVTrans[i];
      sqlite3_vtab *p = pVTab->pVtab;
      अगर( p ){
        पूर्णांक (*x)(sqlite3_vtab *);
        x = *(पूर्णांक (**)(sqlite3_vtab *))((अक्षर *)p->pModule + offset);
        अगर( x ) x(p);
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
पूर्णांक sqlite3VtabSync(sqlite3 *db, Vdbe *p){
  पूर्णांक i;
  पूर्णांक rc = SQLITE_OK;
  VTable **aVTrans = db->aVTrans;

  db->aVTrans = 0;
  क्रम(i=0; rc==SQLITE_OK && i<db->nVTrans; i++){
    पूर्णांक (*x)(sqlite3_vtab *);
    sqlite3_vtab *pVtab = aVTrans[i]->pVtab;
    अगर( pVtab && (x = pVtab->pModule->xSync)!=0 ){
      rc = x(pVtab);
      sqlite3VtabImportErrmsg(p, pVtab);
    }
  }
  db->aVTrans = aVTrans;
  वापस rc;
}

/*
** Invoke the xRollback method of all virtual tables in the
** sqlite3.aVTrans array. Then clear the array itself.
*/
पूर्णांक sqlite3VtabRollback(sqlite3 *db){
  callFinaliser(db, दुरत्व(sqlite3_module,xRollback));
  वापस SQLITE_OK;
}

/*
** Invoke the xCommit method of all virtual tables in the
** sqlite3.aVTrans array. Then clear the array itself.
*/
पूर्णांक sqlite3VtabCommit(sqlite3 *db){
  callFinaliser(db, दुरत्व(sqlite3_module,xCommit));
  वापस SQLITE_OK;
}

/*
** If the virtual table pVtab supports the transaction interface
** (xBegin/xRollback/xCommit and optionally xSync) and a transaction is
** not currently open, invoke the xBegin method now.
**
** If the xBegin call is successful, place the sqlite3_vtab pointer
** in the sqlite3.aVTrans array.
*/
पूर्णांक sqlite3VtabBegin(sqlite3 *db, VTable *pVTab){
  पूर्णांक rc = SQLITE_OK;
  स्थिर sqlite3_module *pModule;

  /* Special case: If db->aVTrans is NULL and db->nVTrans is greater
  ** than zero, then this function is being called from within a
  ** virtual module xSync() callback. It is illegal to write to
  ** virtual module tables in this case, so return SQLITE_LOCKED.
  */
  अगर( sqlite3VtabInSync(db) ){
    वापस SQLITE_LOCKED;
  }
  अगर( !pVTab ){
    वापस SQLITE_OK;
  }
  pModule = pVTab->pVtab->pModule;

  अगर( pModule->xBegin ){
    पूर्णांक i;

    /* If pVtab is already in the aVTrans array, return early */
    क्रम(i=0; i<db->nVTrans; i++){
      अगर( db->aVTrans[i]==pVTab ){
        वापस SQLITE_OK;
      }
    }

    /* Invoke the xBegin method. If successful, add the vtab to the
    ** sqlite3.aVTrans[] array. */
    rc = growVTrans(db);
    अगर( rc==SQLITE_OK ){
      rc = pModule->xBegin(pVTab->pVtab);
      अगर( rc==SQLITE_OK ){
        पूर्णांक iSvpt = db->nStatement + db->nSavepoint;
        addToVTrans(db, pVTab);
        अगर( iSvpt && pModule->xSavepoint ){
          pVTab->iSavepoint = iSvpt;
          rc = pModule->xSavepoint(pVTab->pVtab, iSvpt-1);
        }
      }
    }
  }
  वापस rc;
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
पूर्णांक sqlite3VtabSavepoint(sqlite3 *db, पूर्णांक op, पूर्णांक iSavepoint){
  पूर्णांक rc = SQLITE_OK;

  निश्चित( op==SAVEPOINT_RELEASE||op==SAVEPOINT_ROLLBACK||op==SAVEPOINT_BEGIN );
  निश्चित( iSavepoint>=-1 );
  अगर( db->aVTrans ){
    पूर्णांक i;
    क्रम(i=0; rc==SQLITE_OK && i<db->nVTrans; i++){
      VTable *pVTab = db->aVTrans[i];
      स्थिर sqlite3_module *pMod = pVTab->pMod->pModule;
      अगर( pVTab->pVtab && pMod->iVersion>=2 ){
        पूर्णांक (*xMethod)(sqlite3_vtab *, पूर्णांक);
        sqlite3VtabLock(pVTab);
        चयन( op ){
          हाल SAVEPOINT_BEGIN:
            xMethod = pMod->xSavepoint;
            pVTab->iSavepoint = iSavepoint+1;
            अवरोध;
          हाल SAVEPOINT_ROLLBACK:
            xMethod = pMod->xRollbackTo;
            अवरोध;
          शेष:
            xMethod = pMod->xRelease;
            अवरोध;
        }
        अगर( xMethod && pVTab->iSavepoint>iSavepoint ){
          u64 savedFlags = (db->flags & SQLITE_Defensive);
          db->flags &= ~(u64)SQLITE_Defensive;
          rc = xMethod(pVTab->pVtab, iSavepoint);
          db->flags |= savedFlags;
        }
        sqlite3VtabUnlock(pVTab);
      }
    }
  }
  वापस rc;
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
  पूर्णांक nArg,       /* Number of arguments to the function */
  व्यंजक *p_व्यंजक     /* First argument to the function */
){
  Table *pTab;
  sqlite3_vtab *pVtab;
  sqlite3_module *pMod;
  व्योम (*xSFunc)(sqlite3_context*,पूर्णांक,sqlite3_मूल्य**) = 0;
  व्योम *pArg = 0;
  FuncDef *pNew;
  पूर्णांक rc = 0;

  /* Check to see the left operand is a column in a virtual table */
  अगर( NEVER(p_व्यंजक==0) ) वापस pDef;
  अगर( p_व्यंजक->op!=TK_COLUMN ) वापस pDef;
  निश्चित( व्यंजक_Use_YTab(p_व्यंजक) );
  pTab = p_व्यंजक->y.pTab;
  अगर( NEVER(pTab==0) ) वापस pDef;
  अगर( !IsVirtual(pTab) ) वापस pDef;
  pVtab = sqlite3GetVTable(db, pTab)->pVtab;
  निश्चित( pVtab!=0 );
  निश्चित( pVtab->pModule!=0 );
  pMod = (sqlite3_module *)pVtab->pModule;
  अगर( pMod->xFindFunction==0 ) वापस pDef;

  /* Call the xFindFunction method on the virtual table implementation
  ** to see if the implementation wants to overload this function.
  **
  ** Though undocumented, we have historically always invoked xFindFunction
  ** with an all lower-case function name.  Continue in this tradition to
  ** avoid any chance of an incompatibility.
  */
#अगर_घोषित SQLITE_DEBUG
  {
    पूर्णांक i;
    क्रम(i=0; pDef->zName[i]; i++){
      अचिन्हित अक्षर x = (अचिन्हित अक्षर)pDef->zName[i];
      निश्चित( x==sqlite3UpperToLower[x] );
    }
  }
#पूर्ण_अगर
  rc = pMod->xFindFunction(pVtab, nArg, pDef->zName, &xSFunc, &pArg);
  अगर( rc==0 ){
    वापस pDef;
  }

  /* Create a new ephemeral function definition for the overloaded
  ** function */
  pNew = sqlite3DbMallocZero(db, माप(*pNew)
                             + sqlite3Strlen30(pDef->zName) + 1);
  अगर( pNew==0 ){
    वापस pDef;
  }
  *pNew = *pDef;
  pNew->zName = (स्थिर अक्षर*)&pNew[1];
  स_नकल((अक्षर*)&pNew[1], pDef->zName, sqlite3Strlen30(pDef->zName)+1);
  pNew->xSFunc = xSFunc;
  pNew->p_User_तथ्य = pArg;
  pNew->funcFlags |= SQLITE_FUNC_EPHEM;
  वापस pNew;
}

/*
** Make sure virtual table pTab is contained in the pParse->apVirtualLock[]
** array so that an OP_VBegin will get generated for it.  Add pTab to the
** array if it is missing.  If pTab is already in the array, this routine
** is a no-op.
*/
व्योम sqlite3VtabMakeWritable(Parse *pParse, Table *pTab){
  Parse *pToplevel = sqlite3ParseToplevel(pParse);
  पूर्णांक i, n;
  Table **apVtabLock;

  निश्चित( IsVirtual(pTab) );
  क्रम(i=0; i<pToplevel->nVtabLock; i++){
    अगर( pTab==pToplevel->apVtabLock[i] ) वापस;
  }
  n = (pToplevel->nVtabLock+1)*माप(pToplevel->apVtabLock[0]);
  apVtabLock = sqlite3Realloc(pToplevel->apVtabLock, n);
  अगर( apVtabLock ){
    pToplevel->apVtabLock = apVtabLock;
    pToplevel->apVtabLock[pToplevel->nVtabLock++] = pTab;
  }अन्यथा{
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
पूर्णांक sqlite3VtabEponymousTableInit(Parse *pParse, Module *pMod){
  स्थिर sqlite3_module *pModule = pMod->pModule;
  Table *pTab;
  अक्षर *zErr = 0;
  पूर्णांक rc;
  sqlite3 *db = pParse->db;
  अगर( pMod->pEpoTab ) वापस 1;
  अगर( pModule->xCreate!=0 && pModule->xCreate!=pModule->xConnect ) वापस 0;
  pTab = sqlite3DbMallocZero(db, माप(Table));
  अगर( pTab==0 ) वापस 0;
  pTab->zName = sqlite3DbStrDup(db, pMod->zName);
  अगर( pTab->zName==0 ){
    sqlite3DbFree(db, pTab);
    वापस 0;
  }
  pMod->pEpoTab = pTab;
  pTab->nTabRef = 1;
  pTab->eTabType = TABTYP_VTAB;
  pTab->pSchema = db->aDb[0].pSchema;
  निश्चित( pTab->u.vtab.nArg==0 );
  pTab->iPKey = -1;
  pTab->tabFlags |= TF_Eponymous;
  addModuleArgument(pParse, pTab, sqlite3DbStrDup(db, pTab->zName));
  addModuleArgument(pParse, pTab, 0);
  addModuleArgument(pParse, pTab, sqlite3DbStrDup(db, pTab->zName));
  db->nSchemaLock++;
  rc = vtabCallConstructor(db, pTab, pMod, pModule->xConnect, &zErr);
  db->nSchemaLock--;
  अगर( rc ){
    sqlite3_Error_संदेश(pParse, "%s", zErr);
    pParse->rc = rc;
    sqlite3DbFree(db, zErr);
    sqlite3VtabEponymousTableClear(db, pMod);
  }
  वापस 1;
}

/*
** Erase the eponymous virtual table instance associated with
** virtual table module pMod, if it exists.
*/
व्योम sqlite3VtabEponymousTableClear(sqlite3 *db, Module *pMod){
  Table *pTab = pMod->pEpoTab;
  अगर( pTab!=0 ){
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
व्योम sqlite3VtabEponymousTableClearAll(sqlite3 *db){
  HashElem *pElem;
  क्रम(pElem=sqliteHashFirst(&db->aModule); pElem; pElem=sqlite_Hash_अग्र(pElem)){
    sqlite3VtabEponymousTableClear(db, (Module*)sqlite_Hash_तथ्य(pElem));
  }
}

/*
** Return the ON CONFLICT resolution mode in effect for the virtual
** table update operation currently in progress.
**
** The results of this routine are undefined unless it is called from
** within an xUpdate method.
*/
पूर्णांक sqlite3_vtab_on_conflict(sqlite3 *db){
  अटल स्थिर अचिन्हित अक्षर aMap[] = {
    SQLITE_ROLLBACK, SQLITE_ABORT, SQLITE_FAIL, SQLITE_IGNORE, SQLITE_REPLACE
  };
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !sqlite3SafetyCheckOk(db) ) वापस SQLITE_MISUSE_BKPT;
#पूर्ण_अगर
  निश्चित( OE_Rollback==1 && OE_Abort==2 && OE_Fail==3 );
  निश्चित( OE_Ignore==4 && OE_Replace==5 );
  निश्चित( db->vtabOnConflict>=1 && db->vtabOnConflict<=5 );
  वापस (पूर्णांक)aMap[db->vtabOnConflict-1];
}

/*
** Call from within the xCreate() or xConnect() methods to provide
** the SQLite core with additional information about the behavior
** of the virtual table being implemented.
*/
पूर्णांक sqlite3_vtab_config(sqlite3 *db, पूर्णांक op, ...){
  बहु_सूची ap;
  पूर्णांक rc = SQLITE_OK;
  VtabCtx *p;

#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !sqlite3SafetyCheckOk(db) ) वापस SQLITE_MISUSE_BKPT;
#पूर्ण_अगर
  sqlite3_mutex_enter(db->mutex);
  p = db->pVtabCtx;
  अगर( !p ){
    rc = SQLITE_MISUSE_BKPT;
  }अन्यथा{
    निश्चित( p->pTab==0 || IsVirtual(p->pTab) );
    बहु_शुरू(ap, op);
    चयन( op ){
      हाल SQLITE_VTAB_CONSTRAINT_SUPPORT: {
        p->pVTable->bConstraint = (u8)बहु_तर्क(ap, पूर्णांक);
        अवरोध;
      }
      हाल SQLITE_VTAB_INNOCUOUS: {
        p->pVTable->eVtabRisk = SQLITE_VTABRISK_निचला;
        अवरोध;
      }
      हाल SQLITE_VTAB_DIRECTONLY: {
        p->pVTable->eVtabRisk = SQLITE_VTABRISK_ऊपरी;
        अवरोध;
      }
      हाल SQLITE_VTAB_USES_ALL_SCHEMAS: {
        p->pVTable->bAllSchemas = 1;
        अवरोध;
      }
      शेष: {
        rc = SQLITE_MISUSE_BKPT;
        अवरोध;
      }
    }
    बहु_पूर्ण(ap);
  }

  अगर( rc!=SQLITE_OK ) sqlite3Error(db, rc);
  sqlite3_mutex_leave(db->mutex);
  वापस rc;
}

#पूर्ण_अगर /* SQLITE_OMIT_VIRTUALTABLE */
