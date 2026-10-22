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
#جے_بنا_تعریف SQLITE_OMIT_VIRTUALTABLE
#ضمن "sqliteInt.h"

/*
** Before a virtual table xCreate() or xConnect() method is invoked, the
** sqlite3.pVtabCtx member variable is set to point to an instance of
** this struct allocated on the stack. It is used by the implementation of
** the sqlite3_declare_vtab() and sqlite3_vtab_config() APIs, both of which
** are invoked only from within xCreate and xConnect methods.
*/
بنية VtabCtx {
  VTable *pVTable;    /* The virtual table being constructed */
  جدول *pTab;        /* The Table object to which the virtual table belongs */
  VtabCtx *p_پہلوں;    /* Parent context (if any) */
  صحيح bDeclared;      /* True after sqlite3_declare_vtab() is called */
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
  ثابت حرف *zName,              /* Name assigned to this module */
  ثابت sqlite3_module *pModule,  /* The definition of the module */
  فراغ *pAux,                     /* Context pointer for xCreate/xConnect */
  فراغ (*xDestroy)(فراغ *)        /* Module destructor function */
){
  Module *pMod;
  Module *pDel;
  حرف *zCopy;
  إذا( pModule==0 ){
    zCopy = (حرف*)zName;
    pMod = 0;
  }وإلا{
    صحيح nName = sqlite3Strlen30(zName);
    pMod = (Module *)sqlite3Malloc(حجم(Module) + nName + 1);
    إذا( pMod==0 ){
      sqlite3OomFault(db);
      أرجع 0;
    }
    zCopy = (حرف *)(&pMod[1]);
    حافظہ_نقل(zCopy, zName, nName+1);
    pMod->zName = zCopy;
    pMod->pModule = pModule;
    pMod->pAux = pAux;
    pMod->xDestroy = xDestroy;
    pMod->pEpoTab = 0;
    pMod->nRefModule = 1;
  }
  pDel = (Module *)sqlite3_Hash_داخل_کرو(&db->aModule,zCopy,(فراغ*)pMod);
  إذا( pDel ){
    إذا( pDel==pMod ){
      sqlite3OomFault(db);
      sqlite3DbFree(db, pDel);
      pMod = 0;
    }وإلا{
      sqlite3_Vtab_Eponymous_جدول_Clear(db, pDel);
      sqlite3VtabModuleUnref(db, pDel);
    }
  }
  أرجع pMod;
}

/*
** The actual function that does the work of creating a new module.
** This function implements the sqlite3_create_module() and
** sqlite3_create_module_v2() interfaces.
*/
ساكن صحيح createModule(
  sqlite3 *db,                    /* Database in which module is registered */
  ثابت حرف *zName,              /* Name assigned to this module */
  ثابت sqlite3_module *pModule,  /* The definition of the module */
  فراغ *pAux,                     /* Context pointer for xCreate/xConnect */
  فراغ (*xDestroy)(فراغ *)        /* Module destructor function */
){
  صحيح rc = SQLITE_OK;

  sqlite3_mutex_enter(db->mutex);
  (فراغ)sqlite3VtabCreateModule(db, zName, pModule, pAux, xDestroy);
  rc = sqlite3ApiExit(db, rc);
  إذا( rc!=SQLITE_OK && xDestroy ) xDestroy(pAux);
  sqlite3_mutex_leave(db->mutex);
  أرجع rc;
}


/*
** External API function used to create a new virtual-table module.
*/
صحيح sqlite3_create_module(
  sqlite3 *db,                    /* Database in which module is registered */
  ثابت حرف *zName,              /* Name assigned to this module */
  ثابت sqlite3_module *pModule,  /* The definition of the module */
  فراغ *pAux                      /* Context pointer for xCreate/xConnect */
){
#جے_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  إذا( !sqlite3SafetyCheckOk(db) || zName==0 ) أرجع SQLITE_MISUSE_BKPT;
#جے_انت
  أرجع createModule(db, zName, pModule, pAux, 0);
}

/*
** External API function used to create a new virtual-table module.
*/
صحيح sqlite3_create_module_v2(
  sqlite3 *db,                    /* Database in which module is registered */
  ثابت حرف *zName,              /* Name assigned to this module */
  ثابت sqlite3_module *pModule,  /* The definition of the module */
  فراغ *pAux,                     /* Context pointer for xCreate/xConnect */
  فراغ (*xDestroy)(فراغ *)        /* Module destructor function */
){
#جے_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  إذا( !sqlite3SafetyCheckOk(db) || zName==0 ) أرجع SQLITE_MISUSE_BKPT;
#جے_انت
  أرجع createModule(db, zName, pModule, pAux, xDestroy);
}

/*
** External API to drop all virtual-table modules, except those named
** on the azNames list.
*/
صحيح sqlite3_drop_modules(sqlite3 *db, ثابت حرف** az_ناں){
  HashElem *pThis, *p_اگلا;
#جے_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  إذا( !sqlite3SafetyCheckOk(db) ) أرجع SQLITE_MISUSE_BKPT;
#جے_انت
  sqlite3_mutex_enter(db->mutex);
  لكل(pThis=sqlite_Hash_پہلا(&db->aModule); pThis; pThis=p_اگلا){
    Module *pMod = (Module*)sqlite_Hash_انکڑے(pThis);
    p_اگلا = sqlite_Hash_اگلا(pThis);
    إذا( az_ناں ){
      صحيح ii;
      لكل(ii=0; az_ناں[ii]!=0 && strcmp(az_ناں[ii],pMod->zName)!=0; ii++){}
      إذا( az_ناں[ii]!=0 ) استمر;
    }
    createModule(db, pMod->zName, 0, 0, 0);
  }
  sqlite3_mutex_leave(db->mutex);
  أرجع SQLITE_OK;
}

/*
** Decrement the reference count on a Module object.  Destroy the
** module when the reference count reaches zero.
*/
فراغ sqlite3VtabModuleUnref(sqlite3 *db, Module *pMod){
  أكد( pMod->nRefModule>0 );
  pMod->nRefModule--;
  إذا( pMod->nRefModule==0 ){
    إذا( pMod->xDestroy ){
      pMod->xDestroy(pMod->pAux);
    }
    أكد( pMod->pEpoTab==0 );
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
فراغ sqlite3_Vtab_جندرا(VTable *pVTab){
  pVTab->nRef++;
}


/*
** pTab is a pointer to a Table structure representing a virtual-table.
** Return a pointer to the VTable object used by connection db to access
** this virtual-table, if one has been created, or NULL otherwise.
*/
VTable *sqlite3GetVTable(sqlite3 *db, جدول *pTab){
  VTable *pVtab;
  أكد( IsVirtual(pTab) );
  لكل(pVtab=pTab->u.vtab.p; pVtab && pVtab->db!=db; pVtab=pVtab->p_اگلا);
  أرجع pVtab;
}

/*
** Decrement the ref-count on a virtual table object. When the ref-count
** reaches zero, call the xDisconnect() method to delete the object.
*/
فراغ sqlite3VtabUnlock(VTable *pVTab){
  sqlite3 *db = pVTab->db;

  أكد( db );
  أكد( pVTab->nRef>0 );
  أكد( db->eOpenState==SQLITE_STATE_OPEN
       || db->eOpenState==SQLITE_STATE_ZOMBIE );

  pVTab->nRef--;
  إذا( pVTab->nRef==0 ){
    sqlite3_vtab *p = pVTab->pVtab;
    إذا( p ){
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
ساكن VTable *vtabDisconnectAll(sqlite3 *db, جدول *p){
  VTable *pRet = 0;
  VTable *pVTable;

  أكد( IsVirtual(p) );
  pVTable = p->u.vtab.p;
  p->u.vtab.p = 0;

  /* Assert that the mutex (if any) associated with the BtShared database
  ** that contains table p is held by the caller. See header comments
  ** above function sqlite3VtabUnlockList() for an explanation of why
  ** this makes it safe to access the sqlite3.pDisconnect list of any
  ** database connection that may have an entry in the p->u.vtab.p list.
  */
  أكد( db==0 || sqlite3SchemaMutexHeld(db, 0, p->pSchema) );

  طالما( pVTable ){
    sqlite3 *db2 = pVTable->db;
    VTable *p_اگلا = pVTable->p_اگلا;
    أكد( db2 );
    إذا( db2==db ){
      pRet = pVTable;
      p->u.vtab.p = pRet;
      pRet->p_اگلا = 0;
    }وإلا{
      pVTable->p_اگلا = db2->pDisconnect;
      db2->pDisconnect = pVTable;
    }
    pVTable = p_اگلا;
  }

  أكد( !db || pRet );
  أرجع pRet;
}

/*
** Table *p is a virtual table. This function removes the VTable object
** for table *p associated with database connection db from the linked
** list in p->pVTab. It also decrements the VTable ref count. This is
** used when closing database connection db to free all of its VTable
** objects without disturbing the rest of the Schema object (which may
** be being used by other shared-cache connections).
*/
فراغ sqlite3VtabDisconnect(sqlite3 *db, جدول *p){
  VTable **ppVTab;

  أكد( IsVirtual(p) );
  أكد( sqlite3BtreeHoldsAllMutexes(db) );
  أكد( sqlite3_mutex_held(db->mutex) );

  لكل(ppVTab=&p->u.vtab.p; *ppVTab; ppVTab=&(*ppVTab)->p_اگلا){
    إذا( (*ppVTab)->db==db  ){
      VTable *pVTab = *ppVTab;
      *ppVTab = pVTab->p_اگلا;
      sqlite3VtabUnlock(pVTab);
      اكسر;
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
فراغ sqlite3VtabUnlockList(sqlite3 *db){
  VTable *p = db->pDisconnect;

  أكد( sqlite3BtreeHoldsAllMutexes(db) );
  أكد( sqlite3_mutex_held(db->mutex) );

  إذا( p ){
    db->pDisconnect = 0;
    افعل {
      VTable *p_اگلا = p->p_اگلا;
      sqlite3VtabUnlock(p);
      p = p_اگلا;
    }طالما( p );
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
فراغ sqlite3VtabClear(sqlite3 *db, جدول *p){
  أكد( IsVirtual(p) );
  أكد( db!=0 );
  إذا( db->pn_بائٹاں_Freed==0 ) vtabDisconnectAll(0, p);
  إذا( p->u.vtab.azArg ){
    صحيح i;
    لكل(i=0; i<p->u.vtab.nArg; i++){
      إذا( i!=1 ) sqlite3DbFree(db, p->u.vtab.azArg[i]);
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
ساكن فراغ جوڑو_Module_دلیل(Parse *pParse, جدول *p_جدول, حرف *zArg){
  sqlite3_int64 n_بائٹاں;
  حرف **azModuleArg;
  sqlite3 *db = pParse->db;

  أكد( IsVirtual(p_جدول) );
  n_بائٹاں = حجم(حرف *)*(2+p_جدول->u.vtab.nArg);
  إذا( p_جدول->u.vtab.nArg+3>=db->a_حد_بندی[SQLITE_حد_بندی_کالم] ){
    sqlite3_Error_پیغام(pParse, "too many columns on %s", p_جدول->zName);
  }
  azModuleArg = sqlite3DbRealloc(db, p_جدول->u.vtab.azArg, n_بائٹاں);
  إذا( azModuleArg==0 ){
    sqlite3DbFree(db, zArg);
  }وإلا{
    صحيح i = p_جدول->u.vtab.nArg++;
    azModuleArg[i] = zArg;
    azModuleArg[i+1] = 0;
    p_جدول->u.vtab.azArg = azModuleArg;
  }
}

/*
** The parser calls this routine when it first sees a CREATE VIRTUAL TABLE
** statement.  The module name has been parsed, but the optional list
** of parameters that follow the module name are still pending.
*/
فراغ sqlite3VtabBeginParse(
  Parse *pParse,        /* Parsing context */
  Token *pName1,        /* Name of new table, or database name */
  Token *pName2,        /* Name of new table or NULL */
  Token *pModuleName,   /* Name of the module for the virtual table */
  صحيح ifNotExists       /* No error if the table already exists */
){
  جدول *p_جدول;        /* The new virtual table */
  sqlite3 *db;          /* Database connection */

  sqlite3_شروع_جدول(pParse, pName1, pName2, 0, 0, 1, ifNotExists);
  p_جدول = pParse->p_New_جدول;
  إذا( p_جدول==0 ) أرجع;
  أكد( 0==p_جدول->p_اشاریہ );
  p_جدول->eTabType = TABTYP_VTAB;

  db = pParse->db;

  أكد( p_جدول->u.vtab.nArg==0 );
  جوڑو_Module_دلیل(pParse, p_جدول, sqlite3NameFromToken(db, pModuleName));
  جوڑو_Module_دلیل(pParse, p_جدول, 0);
  جوڑو_Module_دلیل(pParse, p_جدول, sqlite3DbStrDup(db, p_جدول->zName));
  أكد( (pParse->sNameToken.z==pName2->z && pName2->z!=0)
       || (pParse->sNameToken.z==pName1->z && pName2->z==0)
  );
  pParse->sNameToken.n = (صحيح)(
      &pModuleName->z[pModuleName->n] - pParse->sNameToken.z
  );

#جے_بنا_تعریف SQLITE_OMIT_AUTHORIZATION
  /* Creating a virtual table invokes the authorization callback twice.
  ** The first invocation, to obtain permission to INSERT a row into the
  ** sqlite_schema table, has already been made by sqlite3StartTable().
  ** The second call, to obtain permission to create the table, is made now.
  */
  إذا( p_جدول->u.vtab.azArg ){
    صحيح iDb = sqlite3_Schema_منزل_اشاریہ(db, p_جدول->pSchema);
    أكد( iDb>=0 ); /* The database the table is being created in */
    sqlite3AuthCheck(pParse, SQLITE_CREATE_VTABLE, p_جدول->zName,
            p_جدول->u.vtab.azArg[0], pParse->db->aDb[iDb].zDbSName);
  }
#جے_انت
}

/*
** This routine takes the module argument that has been accumulating
** in pParse->zArg[] and appends it to the list of arguments on the
** virtual table currently under construction in pParse->pTable.
*/
ساكن فراغ جوڑو_دلیل_منزل_Vtab(Parse *pParse){
  إذا( pParse->sArg.z && pParse->p_New_جدول ){
    ثابت حرف *z = (ثابت حرف*)pParse->sArg.z;
    صحيح n = pParse->sArg.n;
    sqlite3 *db = pParse->db;
    جوڑو_Module_دلیل(pParse, pParse->p_New_جدول, sqlite3DbStrNDup(db, z, n));
  }
}

/*
** The parser calls this routine after the CREATE VIRTUAL TABLE statement
** has been completely parsed.
*/
فراغ sqlite3_Vtab_انت_Parse(Parse *pParse, Token *p_سرا){
  جدول *pTab = pParse->p_New_جدول;  /* The table being constructed */
  sqlite3 *db = pParse->db;         /* The database connection */

  إذا( pTab==0 ) أرجع;
  أكد( IsVirtual(pTab) );
  جوڑو_دلیل_منزل_Vtab(pParse);
  pParse->sArg.z = 0;
  إذا( pTab->u.vtab.nArg<1 ) أرجع;

  /* If the CREATE VIRTUAL TABLE statement is being entered for the
  ** first time (in other words if the virtual table is actually being
  ** created now instead of just being read out of sqlite_schema) then
  ** do additional initialization work and store the statement text
  ** in the sqlite_schema table.
  */
  إذا( !db->init.busy ){
    حرف *zStmt;
    حرف *zWhere;
    صحيح iDb;
    صحيح iReg;
    Vdbe *v;

    sqlite3MayAbort(pParse);

    /* Compute the complete text of the CREATE VIRTUAL TABLE statement */
    إذا( p_سرا ){
      pParse->sNameToken.n = (صحيح)(p_سرا->z - pParse->sNameToken.z) + p_سرا->n;
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
    iDb = sqlite3_Schema_منزل_اشاریہ(db, pTab->pSchema);
    أكد( pParse->isCreate );
    sqlite3NestedParse(pParse,
      "UPDATE %Q." LEGACY_SCHEMA_جدول " "
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

    sqlite3_Vdbe_جوڑو_Op0(v, OP_Expire);
    zWhere = sqlite3MPrintf(db, "name=%Q AND sql=%Q", pTab->zName, zStmt);
    sqlite3_Vdbe_جوڑو_Parse_Schema_Op(v, iDb, zWhere, 0);
    sqlite3DbFree(db, zStmt);

    iReg = ++pParse->nMem;
    sqlite3_Vdbe_لدو_String(v, iReg, pTab->zName);
    sqlite3_Vdbe_جوڑو_Op2(v, OP_VCreate, iDb, iReg);
  }وإلا{
    /* If we are rereading the sqlite_schema table create the in-memory
    ** record of the table. */
    جدول *pOld;
    Schema *pSchema = pTab->pSchema;
    ثابت حرف *zName = pTab->zName;
    أكد( zName!=0 );
    sqlite3_Mark_All_Shadow_Tables_دا(db, pTab);
    pOld = sqlite3_Hash_داخل_کرو(&pSchema->tblHash, zName, pTab);
    إذا( pOld ){
      sqlite3OomFault(db);
      أكد( pTab==pOld );  /* Malloc must have failed inside HashInsert() */
      أرجع;
    }
    pParse->p_New_جدول = 0;
  }
}

/*
** The parser calls this routine when it sees the first token
** of an argument to the module name in a CREATE VIRTUAL TABLE statement.
*/
فراغ sqlite3VtabArgInit(Parse *pParse){
  جوڑو_دلیل_منزل_Vtab(pParse);
  pParse->sArg.z = 0;
  pParse->sArg.n = 0;
}

/*
** The parser calls this routine for each token after the first token
** in an argument to the module name in a CREATE VIRTUAL TABLE statement.
*/
فراغ sqlite3VtabArgExtend(Parse *pParse, Token *p){
  Token *pArg = &pParse->sArg;
  إذا( pArg->z==0 ){
    pArg->z = p->z;
    pArg->n = p->n;
  }وإلا{
    أكد(pArg->z <= p->z);
    pArg->n = (صحيح)(&p->z[p->n] - pArg->z);
  }
}

/*
** Invoke a virtual table constructor (either xCreate or xConnect). The
** pointer to the function to invoke is passed as the fourth parameter
** to this procedure.
*/
ساكن صحيح vtabCallConstructor(
  sqlite3 *db,
  جدول *pTab,
  Module *pMod,
  صحيح (*xConstruct)(sqlite3*,فراغ*,صحيح,ثابت حرف*ثابت*,sqlite3_vtab**,حرف**),
  حرف **pzErr
){
  VtabCtx sCtx;
  VTable *pVTable;
  صحيح rc;
  ثابت حرف *ثابت*azArg;
  صحيح nArg = pTab->u.vtab.nArg;
  حرف *zErr = 0;
  حرف *zModuleName;
  صحيح iDb;
  VtabCtx *pCtx;

  أكد( IsVirtual(pTab) );
  azArg = (ثابت حرف *ثابت*)pTab->u.vtab.azArg;

  /* Check that the virtual-table is not already being initialized */
  لكل(pCtx=db->pVtabCtx; pCtx; pCtx=pCtx->p_پہلوں){
    إذا( pCtx->pTab==pTab ){
      *pzErr = sqlite3MPrintf(db,
          "vtable constructor called recursively: %s", pTab->zName
      );
      أرجع SQLITE_LOCKED;
    }
  }

  zModuleName = sqlite3DbStrDup(db, pTab->zName);
  إذا( !zModuleName ){
    أرجع SQLITE_NOMEM_BKPT;
  }

  pVTable = sqlite3MallocZero(حجم(VTable));
  إذا( !pVTable ){
    sqlite3OomFault(db);
    sqlite3DbFree(db, zModuleName);
    أرجع SQLITE_NOMEM_BKPT;
  }
  pVTable->db = db;
  pVTable->pMod = pMod;
  pVTable->eVtabRisk = SQLITE_VTABRISK_عام;

  iDb = sqlite3_Schema_منزل_اشاریہ(db, pTab->pSchema);
  pTab->u.vtab.azArg[1] = db->aDb[iDb].zDbSName;

  /* Invoke the virtual table constructor */
  أكد( &db->pVtabCtx );
  أكد( xConstruct );
  sCtx.pTab = pTab;
  sCtx.pVTable = pVTable;
  sCtx.p_پہلوں = db->pVtabCtx;
  sCtx.bDeclared = 0;
  db->pVtabCtx = &sCtx;
  pTab->nTabRef++;
  rc = xConstruct(db, pMod->pAux, nArg, azArg, &pVTable->pVtab, &zErr);
  أكد( pTab!=0 );
  أكد( pTab->nTabRef>1 || rc!=SQLITE_OK );
  sqlite3_Delete_جدول(db, pTab);
  db->pVtabCtx = sCtx.p_پہلوں;
  إذا( rc==SQLITE_NOMEM ) sqlite3OomFault(db);
  أكد( sCtx.pTab==pTab );

  إذا( SQLITE_OK!=rc ){
    إذا( zErr==0 ){
      *pzErr = sqlite3MPrintf(db, "vtable constructor failed: %s", zModuleName);
    }وإلا {
      *pzErr = sqlite3MPrintf(db, "%s", zErr);
      sqlite3_free(zErr);
    }
    sqlite3DbFree(db, pVTable);
  }وإلا إذا( ALWAYS(pVTable->pVtab) ){
    /* Justification of ALWAYS():  A correct vtab constructor must allocate
    ** the sqlite3_vtab object if successful.  */
    memset(pVTable->pVtab, 0, حجم(pVTable->pVtab[0]));
    pVTable->pVtab->pModule = pMod->pModule;
    pMod->nRefModule++;
    pVTable->nRef = 1;
    إذا( sCtx.bDeclared==0 ){
      ثابت حرف *zFormat = "vtable constructor did not declare schema: %s";
      *pzErr = sqlite3MPrintf(db, zFormat, zModuleName);
      sqlite3VtabUnlock(pVTable);
      rc = SQLITE_ERROR;
    }وإلا{
      صحيح i_خانہ;
      u16 oooHidden = 0;
      /* If everything went according to plan, link the new VTable structure
      ** into the linked list headed by pTab->u.vtab.p. Then loop through the
      ** columns of the table to see if any of them contain the token "hidden".
      ** If so, set the Column COLFLAG_HIDDEN flag and remove the token from
      ** the type string.  */
      pVTable->p_اگلا = pTab->u.vtab.p;
      pTab->u.vtab.p = pVTable;

      لكل(i_خانہ=0; i_خانہ<pTab->n_خانہ; i_خانہ++){
        حرف *zType = sqlite3_کالم_Type(&pTab->a_خانہ[i_خانہ], "");
        صحيح nType;
        صحيح i = 0;
        nType = sqlite3Strlen30(zType);
        لكل(i=0; i<nType; i++){
          إذا( 0==sqlite3StrNICmp("hidden", &zType[i], 6)
           && (i==0 || zType[i-1]==' ')
           && (zType[i+6]=='\0' || zType[i+6]==' ')
          ){
            اكسر;
          }
        }
        إذا( i<nType ){
          صحيح j;
          صحيح nDel = 6 + (zType[i+6] ? 1 : 0);
          لكل(j=i; (j+nDel)<=nType; j++){
            zType[j] = zType[j+nDel];
          }
          إذا( zType[i]=='\0' && i>0 ){
            أكد(zType[i-1]==' ');
            zType[i-1] = '\0';
          }
          pTab->a_خانہ[i_خانہ].خانہ_Flags |= COLFLAG_HIDDEN;
          pTab->tabFlags |= TF_HasHidden;
          oooHidden = TF_OOOHidden;
        }وإلا{
          pTab->tabFlags |= oooHidden;
        }
      }
    }
  }

  sqlite3DbFree(db, zModuleName);
  أرجع rc;
}

/*
** This function is invoked by the parser to call the xConnect() method
** of the virtual table pTab. If an error occurs, an error code is returned
** and an error left in pParse.
**
** This call is a no-op if table pTab is not a virtual table.
*/
صحيح sqlite3VtabCallConnect(Parse *pParse, جدول *pTab){
  sqlite3 *db = pParse->db;
  ثابت حرف *zMod;
  Module *pMod;
  صحيح rc;

  أكد( pTab );
  أكد( IsVirtual(pTab) );
  إذا( sqlite3GetVTable(db, pTab) ){
    أرجع SQLITE_OK;
  }

  /* Locate the required virtual table module */
  zMod = pTab->u.vtab.azArg[0];
  pMod = (Module*)sqlite3_Hash_لبھو(&db->aModule, zMod);

  إذا( !pMod ){
    ثابت حرف *zModule = pTab->u.vtab.azArg[0];
    sqlite3_Error_پیغام(pParse, "no such module: %s", zModule);
    rc = SQLITE_ERROR;
  }وإلا{
    حرف *zErr = 0;
    rc = vtabCallConstructor(db, pTab, pMod, pMod->pModule->xConnect, &zErr);
    إذا( rc!=SQLITE_OK ){
      sqlite3_Error_پیغام(pParse, "%s", zErr);
      pParse->rc = rc;
    }وإلا{
      sqlite3_Mark_All_Shadow_Tables_دا(db, pTab);
    }
    sqlite3DbFree(db, zErr);
  }

  أرجع rc;
}
/*
** Grow the db->aVTrans[] array so that there is room for at least one
** more v-table. Return SQLITE_NOMEM if a malloc fails, or SQLITE_OK otherwise.
*/
ساكن صحيح growVTrans(sqlite3 *db){
  ثابت صحيح ARRAY_INCR = 5;

  /* Grow the sqlite3.aVTrans array if required */
  إذا( (db->nVTrans%ARRAY_INCR)==0 ){
    VTable **aVTrans;
    sqlite3_int64 n_بائٹاں = حجم(sqlite3_vtab*)*
                                 ((sqlite3_int64)db->nVTrans + ARRAY_INCR);
    aVTrans = sqlite3DbRealloc(db, (فراغ *)db->aVTrans, n_بائٹاں);
    إذا( !aVTrans ){
      أرجع SQLITE_NOMEM_BKPT;
    }
    memset(&aVTrans[db->nVTrans], 0, حجم(sqlite3_vtab *)*ARRAY_INCR);
    db->aVTrans = aVTrans;
  }

  أرجع SQLITE_OK;
}

/*
** Add the virtual table pVTab to the array sqlite3.aVTrans[]. Space should
** have already been reserved using growVTrans().
*/
ساكن فراغ جوڑو_منزل_VTrans(sqlite3 *db, VTable *pVTab){
  /* Add pVtab to the end of sqlite3.aVTrans */
  db->aVTrans[db->nVTrans++] = pVTab;
  sqlite3_Vtab_جندرا(pVTab);
}

/*
** This function is invoked by the vdbe to call the xCreate method
** of the virtual table named zTab in database iDb.
**
** If an error occurs, *pzErr is set to point to an English language
** description of the error and an SQLITE_XXX error code is returned.
** In this case the caller must call sqlite3DbFree(db, ) on *pzErr.
*/
صحيح sqlite3VtabCallCreate(sqlite3 *db, صحيح iDb, ثابت حرف *zTab, حرف **pzErr){
  صحيح rc = SQLITE_OK;
  جدول *pTab;
  Module *pMod;
  ثابت حرف *zMod;

  pTab = sqlite3_لبھو_جدول(db, zTab, db->aDb[iDb].zDbSName);
  أكد( pTab && IsVirtual(pTab) && !pTab->u.vtab.p );

  /* Locate the required virtual table module */
  zMod = pTab->u.vtab.azArg[0];
  pMod = (Module*)sqlite3_Hash_لبھو(&db->aModule, zMod);

  /* If the module has been registered and includes a Create method,
  ** invoke it now. If the module has not been registered, return an
  ** error. Otherwise, do nothing.
  */
  إذا( pMod==0 || pMod->pModule->xCreate==0 || pMod->pModule->xDestroy==0 ){
    *pzErr = sqlite3MPrintf(db, "no such module: %s", zMod);
    rc = SQLITE_ERROR;
  }وإلا{
    rc = vtabCallConstructor(db, pTab, pMod, pMod->pModule->xCreate, pzErr);
  }

  /* Justification of ALWAYS():  The xConstructor method is required to
  ** create a valid sqlite3_vtab if it returns SQLITE_OK. */
  إذا( rc==SQLITE_OK && ALWAYS(sqlite3GetVTable(db, pTab)) ){
    rc = growVTrans(db);
    إذا( rc==SQLITE_OK ){
      VTable *pVTab = sqlite3GetVTable(db, pTab);
      ثابت sqlite3_module *pModule = pMod->pModule;
      صحيح iSvpt = db->nStatement + db->nSavepoint;
      جوڑو_منزل_VTrans(db, pVTab);
      /* As in sqlite3VtabBegin(): open the current savepoint on the new
      ** table, so that a ROLLBACK TO that undoes the CREATE also invokes
      ** its xRollbackTo method. */
      إذا( iSvpt && pModule->i_نسخہ>=2 && pModule->xSavepoint ){
        pVTab->iSavepoint = iSvpt;
        rc = pModule->xSavepoint(pVTab->pVtab, iSvpt-1);
      }
    }
  }

  أرجع rc;
}

/*
** This function is used to set the schema of a virtual table.  It is only
** valid to call this function from within the xCreate() or xConnect() of a
** virtual table module.
*/
صحيح sqlite3_declare_vtab(sqlite3 *db, ثابت حرف *z_Create_جدول){
  VtabCtx *pCtx;
  صحيح rc = SQLITE_OK;
  جدول *pTab;
  Parse sParse;
  صحيح initBusy;
  صحيح i;
  ثابت بنانشانی حرف *z;
  ساكن ثابت u8 aKeyword[] = { TK_CREATE, TK_جدول, 0 };

#جے_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  إذا( !sqlite3SafetyCheckOk(db) || z_Create_جدول==0 ){
    أرجع SQLITE_MISUSE_BKPT;
  }
#جے_انت

  /* Verify that the first two keywords in the CREATE TABLE statement
  ** really are "CREATE" and "TABLE".  If this is not the case, then
  ** sqlite3_declare_vtab() is being misused.
  */
  z = (ثابت بنانشانی حرف*)z_Create_جدول;
  لكل(i=0; aKeyword[i]; i++){
    صحيح tokenType = 0;
    افعل{
      z += sqlite3GetToken(z, &tokenType);
    }طالما( tokenType==TK_SPACE || tokenType==TK_COMMENT );
    إذا( tokenType!=aKeyword[i] ){
      sqlite3_Error_With_پیغام(db, SQLITE_ERROR, "syntax error");
      أرجع SQLITE_ERROR;
    }
  }
 
  sqlite3_mutex_enter(db->mutex);
  pCtx = db->pVtabCtx;
  إذا( !pCtx || pCtx->bDeclared ){
    sqlite3Error(db, SQLITE_MISUSE_BKPT);
    sqlite3_mutex_leave(db->mutex);
    أرجع SQLITE_MISUSE_BKPT;
  }

  pTab = pCtx->pTab;
  أكد( IsVirtual(pTab) );

  sqlite3ParseObjectInit(&sParse, db);
  sParse.e_Parse_منوال = PARSE_منوال_DECLARE_VTAB;
  sParse.disableTriggers = 1;
  /* We should never be able to reach this point while loading the
  ** schema.  Nevertheless, defend against that (turn off db->init.busy)
  ** in case a bug arises. */
  أكد( db->init.busy==0 );
  initBusy = db->init.busy;
  db->init.busy = 0;
  sParse.nQueryLoop = 1;
  إذا( SQLITE_OK==sqlite3_لگاتار_Parser(&sParse, z_Create_جدول) ){
    أكد( sParse.p_New_جدول!=0 );
    أكد( !db->mallocFailed );
    أكد( Is_Ordinary_جدول(sParse.p_New_جدول) );
    أكد( sParse.z_Err_پیغام==0 );
    إذا( !pTab->a_خانہ ){
      جدول *pNew = sParse.p_New_جدول;
      اشاریہ *pIdx;
      pTab->a_خانہ = pNew->a_خانہ;
      أكد( Is_Ordinary_جدول(pNew) );
      sqlite3_اظہاریہ_List_Delete(db, pNew->u.tab.pDfltList);
      pTab->nNVCol = pTab->n_خانہ = pNew->n_خانہ;
      pTab->tabFlags |= pNew->tabFlags & (TF_WithoutRowid|TF_NoVisibleRowid);
      pNew->n_خانہ = 0;
      pNew->a_خانہ = 0;
      أكد( pTab->p_اشاریہ==0 );
      أكد( HasRowid(pNew) || sqlite3_Primary_کلید_اشاریہ(pNew)!=0 );
      إذا( !HasRowid(pNew)
       && pCtx->pVTable->pMod->pModule->xUpdate!=0
       && sqlite3_Primary_کلید_اشاریہ(pNew)->n_کلید_خانہ!=1
      ){
        /* WITHOUT ROWID virtual tables must either be read-only (xUpdate==0)
        ** or else must have a single-column PRIMARY KEY */
        rc = SQLITE_ERROR;
      }
      pIdx = pNew->p_اشاریہ;
      إذا( pIdx ){
        أكد( pIdx->p_اگلا==0 );
        pTab->p_اشاریہ = pIdx;
        pNew->p_اشاریہ = 0;
        pIdx->p_جدول = pTab;
      }
    }
    pCtx->bDeclared = 1;
  }وإلا{
    sqlite3_Error_With_پیغام(db, SQLITE_ERROR,
          (sParse.z_Err_پیغام ? "%s" : 0), sParse.z_Err_پیغام);
    sqlite3DbFree(db, sParse.z_Err_پیغام);
    rc = SQLITE_ERROR;
  }
  sParse.e_Parse_منوال = PARSE_منوال_عام;

  إذا( sParse.pVdbe ){
    sqlite3VdbeFinalize(sParse.pVdbe);
  }
  sqlite3_Delete_جدول(db, sParse.p_New_جدول);
  sqlite3ParseObjectReset(&sParse);
  db->init.busy = initBusy;

  أكد( (rc&0xff)==rc );
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  أرجع rc;
}

/*
** This function is invoked by the vdbe to call the xDestroy method
** of the virtual table named zTab in database iDb. This occurs
** when a DROP TABLE is mentioned.
**
** This call is a no-op if zTab is not a virtual table.
*/
صحيح sqlite3VtabCallDestroy(sqlite3 *db, صحيح iDb, ثابت حرف *zTab){
  صحيح rc = SQLITE_OK;
  جدول *pTab;

  pTab = sqlite3_لبھو_جدول(db, zTab, db->aDb[iDb].zDbSName);
  إذا( ALWAYS(pTab!=0)
   && ALWAYS(IsVirtual(pTab))
   && ALWAYS(pTab->u.vtab.p!=0)
  ){
    VTable *p;
    صحيح (*xDestroy)(sqlite3_vtab *);
    لكل(p=pTab->u.vtab.p; p; p=p->p_اگلا){
      أكد( p->pVtab );
      إذا( p->pVtab->nRef>0 ){
        أرجع SQLITE_LOCKED;
      }
    }
    p = vtabDisconnectAll(db, pTab);
    xDestroy = p->pMod->pModule->xDestroy;
    إذا( xDestroy==0 ) xDestroy = p->pMod->pModule->xDisconnect;
    أكد( xDestroy!=0 );
    pTab->nTabRef++;
    rc = xDestroy(p->pVtab);
    /* Remove the sqlite3_vtab* from the aVTrans[] array, if applicable */
    إذا( rc==SQLITE_OK ){
      أكد( pTab->u.vtab.p==p && p->p_اگلا==0 );
      p->pVtab = 0;
      pTab->u.vtab.p = 0;
      sqlite3VtabUnlock(p);
    }
    sqlite3_Delete_جدول(db, pTab);
  }

  أرجع rc;
}

/*
** This function invokes either the xRollback or xCommit method
** of each of the virtual tables in the sqlite3.aVTrans array. The method
** called is identified by the second argument, "offset", which is
** the offset of the method to call in the sqlite3_module structure.
**
** The array is cleared after invoking the callbacks.
*/
ساكن فراغ callFinaliser(sqlite3 *db, صحيح وتھ){
  صحيح i;
  إذا( db->aVTrans ){
    VTable **aVTrans = db->aVTrans;
    db->aVTrans = 0;
    لكل(i=0; i<db->nVTrans; i++){
      VTable *pVTab = aVTrans[i];
      sqlite3_vtab *p = pVTab->pVtab;
      إذا( p ){
        صحيح (*x)(sqlite3_vtab *);
        x = *(صحيح (**)(sqlite3_vtab *))((حرف *)p->pModule + وتھ);
        إذا( x ) x(p);
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
صحيح sqlite3VtabSync(sqlite3 *db, Vdbe *p){
  صحيح i;
  صحيح rc = SQLITE_OK;
  VTable **aVTrans = db->aVTrans;

  db->aVTrans = 0;
  لكل(i=0; rc==SQLITE_OK && i<db->nVTrans; i++){
    صحيح (*x)(sqlite3_vtab *);
    sqlite3_vtab *pVtab = aVTrans[i]->pVtab;
    إذا( pVtab && (x = pVtab->pModule->xSync)!=0 ){
      rc = x(pVtab);
      sqlite3VtabImportErrmsg(p, pVtab);
    }
  }
  db->aVTrans = aVTrans;
  أرجع rc;
}

/*
** Invoke the xRollback method of all virtual tables in the
** sqlite3.aVTrans array. Then clear the array itself.
*/
صحيح sqlite3VtabRollback(sqlite3 *db){
  callFinaliser(db, offsetof(sqlite3_module,xRollback));
  أرجع SQLITE_OK;
}

/*
** Invoke the xCommit method of all virtual tables in the
** sqlite3.aVTrans array. Then clear the array itself.
*/
صحيح sqlite3VtabCommit(sqlite3 *db){
  callFinaliser(db, offsetof(sqlite3_module,xCommit));
  أرجع SQLITE_OK;
}

/*
** If the virtual table pVtab supports the transaction interface
** (xBegin/xRollback/xCommit and optionally xSync) and a transaction is
** not currently open, invoke the xBegin method now.
**
** If the xBegin call is successful, place the sqlite3_vtab pointer
** in the sqlite3.aVTrans array.
*/
صحيح sqlite3VtabBegin(sqlite3 *db, VTable *pVTab){
  صحيح rc = SQLITE_OK;
  ثابت sqlite3_module *pModule;

  /* Special case: If db->aVTrans is NULL and db->nVTrans is greater
  ** than zero, then this function is being called from within a
  ** virtual module xSync() callback. It is illegal to write to
  ** virtual module tables in this case, so return SQLITE_LOCKED.
  */
  إذا( sqlite3_Vtab_وچ_Sync(db) ){
    أرجع SQLITE_LOCKED;
  }
  إذا( !pVTab ){
    أرجع SQLITE_OK;
  }
  pModule = pVTab->pVtab->pModule;

  إذا( pModule->xBegin ){
    صحيح i;

    /* If pVtab is already in the aVTrans array, return early */
    لكل(i=0; i<db->nVTrans; i++){
      إذا( db->aVTrans[i]==pVTab ){
        أرجع SQLITE_OK;
      }
    }

    /* Invoke the xBegin method. If successful, add the vtab to the
    ** sqlite3.aVTrans[] array. */
    rc = growVTrans(db);
    إذا( rc==SQLITE_OK ){
      rc = pModule->xBegin(pVTab->pVtab);
      إذا( rc==SQLITE_OK ){
        صحيح iSvpt = db->nStatement + db->nSavepoint;
        جوڑو_منزل_VTrans(db, pVTab);
        إذا( iSvpt && pModule->xSavepoint ){
          pVTab->iSavepoint = iSvpt;
          rc = pModule->xSavepoint(pVTab->pVtab, iSvpt-1);
        }
      }
    }
  }
  أرجع rc;
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
صحيح sqlite3VtabSavepoint(sqlite3 *db, صحيح op, صحيح iSavepoint){
  صحيح rc = SQLITE_OK;

  أكد( op==SAVEPOINT_RELEASE||op==SAVEPOINT_ROLLBACK||op==SAVEPOINT_BEGIN );
  أكد( iSavepoint>=-1 );
  إذا( db->aVTrans ){
    صحيح i;
    لكل(i=0; rc==SQLITE_OK && i<db->nVTrans; i++){
      VTable *pVTab = db->aVTrans[i];
      ثابت sqlite3_module *pMod = pVTab->pMod->pModule;
      إذا( pVTab->pVtab && pMod->i_نسخہ>=2 ){
        صحيح (*x_طریقہ)(sqlite3_vtab *, صحيح);
        sqlite3_Vtab_جندرا(pVTab);
        بدل( op ){
          حالة SAVEPOINT_BEGIN:
            x_طریقہ = pMod->xSavepoint;
            pVTab->iSavepoint = iSavepoint+1;
            اكسر;
          حالة SAVEPOINT_ROLLBACK:
            x_طریقہ = pMod->x_Rollback_منزل;
            اكسر;
          افتراضي:
            x_طریقہ = pMod->xRelease;
            اكسر;
        }
        إذا( x_طریقہ && pVTab->iSavepoint>iSavepoint ){
          u64 savedFlags = (db->flags & SQLITE_Defensive);
          db->flags &= ~(u64)SQLITE_Defensive;
          rc = x_طریقہ(pVTab->pVtab, iSavepoint);
          db->flags |= savedFlags;
        }
        sqlite3VtabUnlock(pVTab);
      }
    }
  }
  أرجع rc;
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
  صحيح nArg,       /* Number of arguments to the function */
  اظہاریہ *p_اظہاریہ     /* First argument to the function */
){
  جدول *pTab;
  sqlite3_vtab *pVtab;
  sqlite3_module *pMod;
  فراغ (*xSFunc)(sqlite3_context*,صحيح,sqlite3_قدر**) = 0;
  فراغ *pArg = 0;
  FuncDef *pNew;
  صحيح rc = 0;

  /* Check to see the left operand is a column in a virtual table */
  إذا( NEVER(p_اظہاریہ==0) ) أرجع pDef;
  إذا( p_اظہاریہ->op!=TK_کالم ) أرجع pDef;
  أكد( اظہاریہ_Use_YTab(p_اظہاریہ) );
  pTab = p_اظہاریہ->y.pTab;
  إذا( NEVER(pTab==0) ) أرجع pDef;
  إذا( !IsVirtual(pTab) ) أرجع pDef;
  pVtab = sqlite3GetVTable(db, pTab)->pVtab;
  أكد( pVtab!=0 );
  أكد( pVtab->pModule!=0 );
  pMod = (sqlite3_module *)pVtab->pModule;
  إذا( pMod->x_لبھو_Function==0 ) أرجع pDef;

  /* Call the xFindFunction method on the virtual table implementation
  ** to see if the implementation wants to overload this function.
  **
  ** Though undocumented, we have historically always invoked xFindFunction
  ** with an all lower-case function name.  Continue in this tradition to
  ** avoid any chance of an incompatibility.
  */
#جے_تعریف_شدہ SQLITE_DEBUG
  {
    صحيح i;
    لكل(i=0; pDef->zName[i]; i++){
      بنانشانی حرف x = (بنانشانی حرف)pDef->zName[i];
      أكد( x==sqlite3_اتلا_منزل_ہیٹھلا[x] );
    }
  }
#جے_انت
  rc = pMod->x_لبھو_Function(pVtab, nArg, pDef->zName, &xSFunc, &pArg);
  إذا( rc==0 ){
    أرجع pDef;
  }

  /* Create a new ephemeral function definition for the overloaded
  ** function */
  pNew = sqlite3DbMallocZero(db, حجم(*pNew)
                             + sqlite3Strlen30(pDef->zName) + 1);
  إذا( pNew==0 ){
    أرجع pDef;
  }
  *pNew = *pDef;
  pNew->zName = (ثابت حرف*)&pNew[1];
  حافظہ_نقل((حرف*)&pNew[1], pDef->zName, sqlite3Strlen30(pDef->zName)+1);
  pNew->xSFunc = xSFunc;
  pNew->p_User_انکڑے = pArg;
  pNew->funcFlags |= SQLITE_FUNC_EPHEM;
  أرجع pNew;
}

/*
** Make sure virtual table pTab is contained in the pParse->apVirtualLock[]
** array so that an OP_VBegin will get generated for it.  Add pTab to the
** array if it is missing.  If pTab is already in the array, this routine
** is a no-op.
*/
فراغ sqlite3VtabMakeWritable(Parse *pParse, جدول *pTab){
  Parse *pToplevel = sqlite3ParseToplevel(pParse);
  صحيح i, n;
  جدول **ap_Vtab_جندرا;

  أكد( IsVirtual(pTab) );
  لكل(i=0; i<pToplevel->n_Vtab_جندرا; i++){
    إذا( pTab==pToplevel->ap_Vtab_جندرا[i] ) أرجع;
  }
  n = (pToplevel->n_Vtab_جندرا+1)*حجم(pToplevel->ap_Vtab_جندرا[0]);
  ap_Vtab_جندرا = sqlite3Realloc(pToplevel->ap_Vtab_جندرا, n);
  إذا( ap_Vtab_جندرا ){
    pToplevel->ap_Vtab_جندرا = ap_Vtab_جندرا;
    pToplevel->ap_Vtab_جندرا[pToplevel->n_Vtab_جندرا++] = pTab;
  }وإلا{
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
صحيح sqlite3_Vtab_Eponymous_جدول_Init(Parse *pParse, Module *pMod){
  ثابت sqlite3_module *pModule = pMod->pModule;
  جدول *pTab;
  حرف *zErr = 0;
  صحيح rc;
  sqlite3 *db = pParse->db;
  إذا( pMod->pEpoTab ) أرجع 1;
  إذا( pModule->xCreate!=0 && pModule->xCreate!=pModule->xConnect ) أرجع 0;
  pTab = sqlite3DbMallocZero(db, حجم(جدول));
  إذا( pTab==0 ) أرجع 0;
  pTab->zName = sqlite3DbStrDup(db, pMod->zName);
  إذا( pTab->zName==0 ){
    sqlite3DbFree(db, pTab);
    أرجع 0;
  }
  pMod->pEpoTab = pTab;
  pTab->nTabRef = 1;
  pTab->eTabType = TABTYP_VTAB;
  pTab->pSchema = db->aDb[0].pSchema;
  أكد( pTab->u.vtab.nArg==0 );
  pTab->iPKey = -1;
  pTab->tabFlags |= TF_Eponymous;
  جوڑو_Module_دلیل(pParse, pTab, sqlite3DbStrDup(db, pTab->zName));
  جوڑو_Module_دلیل(pParse, pTab, 0);
  جوڑو_Module_دلیل(pParse, pTab, sqlite3DbStrDup(db, pTab->zName));
  db->n_Schema_جندرا++;
  rc = vtabCallConstructor(db, pTab, pMod, pModule->xConnect, &zErr);
  db->n_Schema_جندرا--;
  إذا( rc ){
    sqlite3_Error_پیغام(pParse, "%s", zErr);
    pParse->rc = rc;
    sqlite3DbFree(db, zErr);
    sqlite3_Vtab_Eponymous_جدول_Clear(db, pMod);
  }
  أرجع 1;
}

/*
** Erase the eponymous virtual table instance associated with
** virtual table module pMod, if it exists.
*/
فراغ sqlite3_Vtab_Eponymous_جدول_Clear(sqlite3 *db, Module *pMod){
  جدول *pTab = pMod->pEpoTab;
  إذا( pTab!=0 ){
    /* Mark the table as Ephemeral prior to deleting it, so that the
    ** sqlite3DeleteTable() routine will know that it is not stored in
    ** the schema. */
    pTab->tabFlags |= TF_Ephemeral;
    sqlite3_Delete_جدول(db, pTab);
    pMod->pEpoTab = 0;
  }
}

/*
** Erase all eponymous virtual table instances associated with database
** handle db.  
*/
فراغ sqlite3_Vtab_Eponymous_جدول_Clear_All(sqlite3 *db){
  HashElem *pElem;
  لكل(pElem=sqlite_Hash_پہلا(&db->aModule); pElem; pElem=sqlite_Hash_اگلا(pElem)){
    sqlite3_Vtab_Eponymous_جدول_Clear(db, (Module*)sqlite_Hash_انکڑے(pElem));
  }
}

/*
** Return the ON CONFLICT resolution mode in effect for the virtual
** table update operation currently in progress.
**
** The results of this routine are undefined unless it is called from
** within an xUpdate method.
*/
صحيح sqlite3_vtab_on_conflict(sqlite3 *db){
  ساكن ثابت بنانشانی حرف aMap[] = {
    SQLITE_ROLLBACK, SQLITE_ABORT, SQLITE_FAIL, SQLITE_IGNORE, SQLITE_REPLACE
  };
#جے_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  إذا( !sqlite3SafetyCheckOk(db) ) أرجع SQLITE_MISUSE_BKPT;
#جے_انت
  أكد( OE_Rollback==1 && OE_Abort==2 && OE_Fail==3 );
  أكد( OE_Ignore==4 && OE_Replace==5 );
  أكد( db->vtabOnConflict>=1 && db->vtabOnConflict<=5 );
  أرجع (صحيح)aMap[db->vtabOnConflict-1];
}

/*
** Call from within the xCreate() or xConnect() methods to provide
** the SQLite core with additional information about the behavior
** of the virtual table being implemented.
*/
صحيح sqlite3_vtab_config(sqlite3 *db, صحيح op, ...){
  va_list ap;
  صحيح rc = SQLITE_OK;
  VtabCtx *p;

#جے_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  إذا( !sqlite3SafetyCheckOk(db) ) أرجع SQLITE_MISUSE_BKPT;
#جے_انت
  sqlite3_mutex_enter(db->mutex);
  p = db->pVtabCtx;
  إذا( !p ){
    rc = SQLITE_MISUSE_BKPT;
  }وإلا{
    أكد( p->pTab==0 || IsVirtual(p->pTab) );
    va_start(ap, op);
    بدل( op ){
      حالة SQLITE_VTAB_CONSTRAINT_SUPPORT: {
        p->pVTable->bConstraint = (u8)va_arg(ap, صحيح);
        اكسر;
      }
      حالة SQLITE_VTAB_INNOCUOUS: {
        p->pVTable->eVtabRisk = SQLITE_VTABRISK_کم;
        اكسر;
      }
      حالة SQLITE_VTAB_DIRECTONLY: {
        p->pVTable->eVtabRisk = SQLITE_VTABRISK_بلند;
        اكسر;
      }
      حالة SQLITE_VTAB_ورتدا_ALL_SCHEMAS: {
        p->pVTable->bAllSchemas = 1;
        اكسر;
      }
      افتراضي: {
        rc = SQLITE_MISUSE_BKPT;
        اكسر;
      }
    }
    va_end(ap);
  }

  إذا( rc!=SQLITE_OK ) sqlite3Error(db, rc);
  sqlite3_mutex_leave(db->mutex);
  أرجع rc;
}

#جے_انت /* SQLITE_OMIT_VIRTUALTABLE */
