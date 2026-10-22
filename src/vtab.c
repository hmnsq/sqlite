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
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_VIRTUALTABLE
#شامل "sqliteInt.h"

/*
** Before a virtual table xCreate() or xConnect() method is invoked, the
** sqlite3.pVtabCtx member variable is set to point to an instance of
** this struct allocated on the stack. It is used by the implementation of
** the sqlite3_declare_vtab() and sqlite3_vtab_config() APIs, both of which
** are invoked only from within xCreate and xConnect methods.
*/
ساخت VtabCtx {
  VTable *pVTable;    /* The virtual table being constructed */
  جدول *pTab;        /* The Table object to which the virtual table belongs */
  VtabCtx *p_پیشگی;    /* Parent context (if any) */
  صحیح bDeclared;      /* True after sqlite3_declare_vtab() is called */
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
  خالی *pAux,                     /* Context pointer for xCreate/xConnect */
  خالی (*xDestroy)(خالی *)        /* Module destructor function */
){
  Module *pMod;
  Module *pDel;
  حرف *zCopy;
  اگر( pModule==0 ){
    zCopy = (حرف*)zName;
    pMod = 0;
  }ورنہ{
    صحیح nName = sqlite3Strlen30(zName);
    pMod = (Module *)sqlite3Malloc(حجم(Module) + nName + 1);
    اگر( pMod==0 ){
      sqlite3OomFault(db);
      واپس 0;
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
  pDel = (Module *)sqlite3_Hash_داخل_کرو(&db->aModule,zCopy,(خالی*)pMod);
  اگر( pDel ){
    اگر( pDel==pMod ){
      sqlite3OomFault(db);
      sqlite3DbFree(db, pDel);
      pMod = 0;
    }ورنہ{
      sqlite3_Vtab_Eponymous_جدول_Clear(db, pDel);
      sqlite3VtabModuleUnref(db, pDel);
    }
  }
  واپس pMod;
}

/*
** The actual function that does the work of creating a new module.
** This function implements the sqlite3_create_module() and
** sqlite3_create_module_v2() interfaces.
*/
ساکن صحیح createModule(
  sqlite3 *db,                    /* Database in which module is registered */
  ثابت حرف *zName,              /* Name assigned to this module */
  ثابت sqlite3_module *pModule,  /* The definition of the module */
  خالی *pAux,                     /* Context pointer for xCreate/xConnect */
  خالی (*xDestroy)(خالی *)        /* Module destructor function */
){
  صحیح rc = SQLITE_OK;

  sqlite3_mutex_enter(db->mutex);
  (خالی)sqlite3VtabCreateModule(db, zName, pModule, pAux, xDestroy);
  rc = sqlite3ApiExit(db, rc);
  اگر( rc!=SQLITE_OK && xDestroy ) xDestroy(pAux);
  sqlite3_mutex_leave(db->mutex);
  واپس rc;
}


/*
** External API function used to create a new virtual-table module.
*/
صحیح sqlite3_create_module(
  sqlite3 *db,                    /* Database in which module is registered */
  ثابت حرف *zName,              /* Name assigned to this module */
  ثابت sqlite3_module *pModule,  /* The definition of the module */
  خالی *pAux                      /* Context pointer for xCreate/xConnect */
){
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db) || zName==0 ) واپس SQLITE_MISUSE_BKPT;
#اگر_ختم
  واپس createModule(db, zName, pModule, pAux, 0);
}

/*
** External API function used to create a new virtual-table module.
*/
صحیح sqlite3_create_module_v2(
  sqlite3 *db,                    /* Database in which module is registered */
  ثابت حرف *zName,              /* Name assigned to this module */
  ثابت sqlite3_module *pModule,  /* The definition of the module */
  خالی *pAux,                     /* Context pointer for xCreate/xConnect */
  خالی (*xDestroy)(خالی *)        /* Module destructor function */
){
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db) || zName==0 ) واپس SQLITE_MISUSE_BKPT;
#اگر_ختم
  واپس createModule(db, zName, pModule, pAux, xDestroy);
}

/*
** External API to drop all virtual-table modules, except those named
** on the azNames list.
*/
صحیح sqlite3_drop_modules(sqlite3 *db, ثابت حرف** az_نام){
  HashElem *pThis, *p_اگلا;
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db) ) واپس SQLITE_MISUSE_BKPT;
#اگر_ختم
  sqlite3_mutex_enter(db->mutex);
  برائے(pThis=sqlite_Hash_پہلا(&db->aModule); pThis; pThis=p_اگلا){
    Module *pMod = (Module*)sqlite_Hash_ڈیٹا(pThis);
    p_اگلا = sqlite_Hash_اگلا(pThis);
    اگر( az_نام ){
      صحیح ii;
      برائے(ii=0; az_نام[ii]!=0 && strcmp(az_نام[ii],pMod->zName)!=0; ii++){}
      اگر( az_نام[ii]!=0 ) جاری;
    }
    createModule(db, pMod->zName, 0, 0, 0);
  }
  sqlite3_mutex_leave(db->mutex);
  واپس SQLITE_OK;
}

/*
** Decrement the reference count on a Module object.  Destroy the
** module when the reference count reaches zero.
*/
خالی sqlite3VtabModuleUnref(sqlite3 *db, Module *pMod){
  أكد( pMod->nRefModule>0 );
  pMod->nRefModule--;
  اگر( pMod->nRefModule==0 ){
    اگر( pMod->xDestroy ){
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
خالی sqlite3_Vtab_تالا(VTable *pVTab){
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
  برائے(pVtab=pTab->u.vtab.p; pVtab && pVtab->db!=db; pVtab=pVtab->p_اگلا);
  واپس pVtab;
}

/*
** Decrement the ref-count on a virtual table object. When the ref-count
** reaches zero, call the xDisconnect() method to delete the object.
*/
خالی sqlite3VtabUnlock(VTable *pVTab){
  sqlite3 *db = pVTab->db;

  أكد( db );
  أكد( pVTab->nRef>0 );
  أكد( db->eOpenState==SQLITE_STATE_OPEN
       || db->eOpenState==SQLITE_STATE_ZOMBIE );

  pVTab->nRef--;
  اگر( pVTab->nRef==0 ){
    sqlite3_vtab *p = pVTab->pVtab;
    اگر( p ){
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
ساکن VTable *vtabDisconnectAll(sqlite3 *db, جدول *p){
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

  جب( pVTable ){
    sqlite3 *db2 = pVTable->db;
    VTable *p_اگلا = pVTable->p_اگلا;
    أكد( db2 );
    اگر( db2==db ){
      pRet = pVTable;
      p->u.vtab.p = pRet;
      pRet->p_اگلا = 0;
    }ورنہ{
      pVTable->p_اگلا = db2->pDisconnect;
      db2->pDisconnect = pVTable;
    }
    pVTable = p_اگلا;
  }

  أكد( !db || pRet );
  واپس pRet;
}

/*
** Table *p is a virtual table. This function removes the VTable object
** for table *p associated with database connection db from the linked
** list in p->pVTab. It also decrements the VTable ref count. This is
** used when closing database connection db to free all of its VTable
** objects without disturbing the rest of the Schema object (which may
** be being used by other shared-cache connections).
*/
خالی sqlite3VtabDisconnect(sqlite3 *db, جدول *p){
  VTable **ppVTab;

  أكد( IsVirtual(p) );
  أكد( sqlite3BtreeHoldsAllMutexes(db) );
  أكد( sqlite3_mutex_held(db->mutex) );

  برائے(ppVTab=&p->u.vtab.p; *ppVTab; ppVTab=&(*ppVTab)->p_اگلا){
    اگر( (*ppVTab)->db==db  ){
      VTable *pVTab = *ppVTab;
      *ppVTab = pVTab->p_اگلا;
      sqlite3VtabUnlock(pVTab);
      توڑو;
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
خالی sqlite3VtabUnlockList(sqlite3 *db){
  VTable *p = db->pDisconnect;

  أكد( sqlite3BtreeHoldsAllMutexes(db) );
  أكد( sqlite3_mutex_held(db->mutex) );

  اگر( p ){
    db->pDisconnect = 0;
    کرو {
      VTable *p_اگلا = p->p_اگلا;
      sqlite3VtabUnlock(p);
      p = p_اگلا;
    }جب( p );
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
خالی sqlite3VtabClear(sqlite3 *db, جدول *p){
  أكد( IsVirtual(p) );
  أكد( db!=0 );
  اگر( db->pn_بائٹیں_Freed==0 ) vtabDisconnectAll(0, p);
  اگر( p->u.vtab.azArg ){
    صحیح i;
    برائے(i=0; i<p->u.vtab.nArg; i++){
      اگر( i!=1 ) sqlite3DbFree(db, p->u.vtab.azArg[i]);
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
ساکن خالی جوڑو_Module_دلیل(Parse *pParse, جدول *p_جدول, حرف *zArg){
  sqlite3_int64 n_بائٹیں;
  حرف **azModuleArg;
  sqlite3 *db = pParse->db;

  أكد( IsVirtual(p_جدول) );
  n_بائٹیں = حجم(حرف *)*(2+p_جدول->u.vtab.nArg);
  اگر( p_جدول->u.vtab.nArg+3>=db->a_حد_بندی[SQLITE_حد_بندی_عمود] ){
    sqlite3_Error_پیغام(pParse, "too many columns on %s", p_جدول->zName);
  }
  azModuleArg = sqlite3DbRealloc(db, p_جدول->u.vtab.azArg, n_بائٹیں);
  اگر( azModuleArg==0 ){
    sqlite3DbFree(db, zArg);
  }ورنہ{
    صحیح i = p_جدول->u.vtab.nArg++;
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
خالی sqlite3VtabBeginParse(
  Parse *pParse,        /* Parsing context */
  Token *pName1,        /* Name of new table, or database name */
  Token *pName2,        /* Name of new table or NULL */
  Token *pModuleName,   /* Name of the module for the virtual table */
  صحیح ifNotExists       /* No error if the table already exists */
){
  جدول *p_جدول;        /* The new virtual table */
  sqlite3 *db;          /* Database connection */

  sqlite3_آغاز_جدول(pParse, pName1, pName2, 0, 0, 1, ifNotExists);
  p_جدول = pParse->p_New_جدول;
  اگر( p_جدول==0 ) واپس;
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
  pParse->sNameToken.n = (صحیح)(
      &pModuleName->z[pModuleName->n] - pParse->sNameToken.z
  );

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTHORIZATION
  /* Creating a virtual table invokes the authorization callback twice.
  ** The first invocation, to obtain permission to INSERT a row into the
  ** sqlite_schema table, has already been made by sqlite3StartTable().
  ** The second call, to obtain permission to create the table, is made now.
  */
  اگر( p_جدول->u.vtab.azArg ){
    صحیح iDb = sqlite3_Schema_مقصد_اشاریہ(db, p_جدول->pSchema);
    أكد( iDb>=0 ); /* The database the table is being created in */
    sqlite3AuthCheck(pParse, SQLITE_CREATE_VTABLE, p_جدول->zName,
            p_جدول->u.vtab.azArg[0], pParse->db->aDb[iDb].zDbSName);
  }
#اگر_ختم
}

/*
** This routine takes the module argument that has been accumulating
** in pParse->zArg[] and appends it to the list of arguments on the
** virtual table currently under construction in pParse->pTable.
*/
ساکن خالی جوڑو_دلیل_مقصد_Vtab(Parse *pParse){
  اگر( pParse->sArg.z && pParse->p_New_جدول ){
    ثابت حرف *z = (ثابت حرف*)pParse->sArg.z;
    صحیح n = pParse->sArg.n;
    sqlite3 *db = pParse->db;
    جوڑو_Module_دلیل(pParse, pParse->p_New_جدول, sqlite3DbStrNDup(db, z, n));
  }
}

/*
** The parser calls this routine after the CREATE VIRTUAL TABLE statement
** has been completely parsed.
*/
خالی sqlite3_Vtab_اختتام_Parse(Parse *pParse, Token *p_انتہا){
  جدول *pTab = pParse->p_New_جدول;  /* The table being constructed */
  sqlite3 *db = pParse->db;         /* The database connection */

  اگر( pTab==0 ) واپس;
  أكد( IsVirtual(pTab) );
  جوڑو_دلیل_مقصد_Vtab(pParse);
  pParse->sArg.z = 0;
  اگر( pTab->u.vtab.nArg<1 ) واپس;

  /* If the CREATE VIRTUAL TABLE statement is being entered for the
  ** first time (in other words if the virtual table is actually being
  ** created now instead of just being read out of sqlite_schema) then
  ** do additional initialization work and store the statement text
  ** in the sqlite_schema table.
  */
  اگر( !db->init.busy ){
    حرف *zStmt;
    حرف *zWhere;
    صحیح iDb;
    صحیح iReg;
    Vdbe *v;

    sqlite3MayAbort(pParse);

    /* Compute the complete text of the CREATE VIRTUAL TABLE statement */
    اگر( p_انتہا ){
      pParse->sNameToken.n = (صحیح)(p_انتہا->z - pParse->sNameToken.z) + p_انتہا->n;
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
    iDb = sqlite3_Schema_مقصد_اشاریہ(db, pTab->pSchema);
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
    sqlite3_Vdbe_بھرو_String(v, iReg, pTab->zName);
    sqlite3_Vdbe_جوڑو_Op2(v, OP_VCreate, iDb, iReg);
  }ورنہ{
    /* If we are rereading the sqlite_schema table create the in-memory
    ** record of the table. */
    جدول *pOld;
    Schema *pSchema = pTab->pSchema;
    ثابت حرف *zName = pTab->zName;
    أكد( zName!=0 );
    sqlite3_Mark_All_Shadow_Tables_کا(db, pTab);
    pOld = sqlite3_Hash_داخل_کرو(&pSchema->tblHash, zName, pTab);
    اگر( pOld ){
      sqlite3OomFault(db);
      أكد( pTab==pOld );  /* Malloc must have failed inside HashInsert() */
      واپس;
    }
    pParse->p_New_جدول = 0;
  }
}

/*
** The parser calls this routine when it sees the first token
** of an argument to the module name in a CREATE VIRTUAL TABLE statement.
*/
خالی sqlite3VtabArgInit(Parse *pParse){
  جوڑو_دلیل_مقصد_Vtab(pParse);
  pParse->sArg.z = 0;
  pParse->sArg.n = 0;
}

/*
** The parser calls this routine for each token after the first token
** in an argument to the module name in a CREATE VIRTUAL TABLE statement.
*/
خالی sqlite3VtabArgExtend(Parse *pParse, Token *p){
  Token *pArg = &pParse->sArg;
  اگر( pArg->z==0 ){
    pArg->z = p->z;
    pArg->n = p->n;
  }ورنہ{
    أكد(pArg->z <= p->z);
    pArg->n = (صحیح)(&p->z[p->n] - pArg->z);
  }
}

/*
** Invoke a virtual table constructor (either xCreate or xConnect). The
** pointer to the function to invoke is passed as the fourth parameter
** to this procedure.
*/
ساکن صحیح vtabCallConstructor(
  sqlite3 *db,
  جدول *pTab,
  Module *pMod,
  صحیح (*xConstruct)(sqlite3*,خالی*,صحیح,ثابت حرف*ثابت*,sqlite3_vtab**,حرف**),
  حرف **pzErr
){
  VtabCtx sCtx;
  VTable *pVTable;
  صحیح rc;
  ثابت حرف *ثابت*azArg;
  صحیح nArg = pTab->u.vtab.nArg;
  حرف *zErr = 0;
  حرف *zModuleName;
  صحیح iDb;
  VtabCtx *pCtx;

  أكد( IsVirtual(pTab) );
  azArg = (ثابت حرف *ثابت*)pTab->u.vtab.azArg;

  /* Check that the virtual-table is not already being initialized */
  برائے(pCtx=db->pVtabCtx; pCtx; pCtx=pCtx->p_پیشگی){
    اگر( pCtx->pTab==pTab ){
      *pzErr = sqlite3MPrintf(db,
          "vtable constructor called recursively: %s", pTab->zName
      );
      واپس SQLITE_LOCKED;
    }
  }

  zModuleName = sqlite3DbStrDup(db, pTab->zName);
  اگر( !zModuleName ){
    واپس SQLITE_NOMEM_BKPT;
  }

  pVTable = sqlite3MallocZero(حجم(VTable));
  اگر( !pVTable ){
    sqlite3OomFault(db);
    sqlite3DbFree(db, zModuleName);
    واپس SQLITE_NOMEM_BKPT;
  }
  pVTable->db = db;
  pVTable->pMod = pMod;
  pVTable->eVtabRisk = SQLITE_VTABRISK_معمول;

  iDb = sqlite3_Schema_مقصد_اشاریہ(db, pTab->pSchema);
  pTab->u.vtab.azArg[1] = db->aDb[iDb].zDbSName;

  /* Invoke the virtual table constructor */
  أكد( &db->pVtabCtx );
  أكد( xConstruct );
  sCtx.pTab = pTab;
  sCtx.pVTable = pVTable;
  sCtx.p_پیشگی = db->pVtabCtx;
  sCtx.bDeclared = 0;
  db->pVtabCtx = &sCtx;
  pTab->nTabRef++;
  rc = xConstruct(db, pMod->pAux, nArg, azArg, &pVTable->pVtab, &zErr);
  أكد( pTab!=0 );
  أكد( pTab->nTabRef>1 || rc!=SQLITE_OK );
  sqlite3_Delete_جدول(db, pTab);
  db->pVtabCtx = sCtx.p_پیشگی;
  اگر( rc==SQLITE_NOMEM ) sqlite3OomFault(db);
  أكد( sCtx.pTab==pTab );

  اگر( SQLITE_OK!=rc ){
    اگر( zErr==0 ){
      *pzErr = sqlite3MPrintf(db, "vtable constructor failed: %s", zModuleName);
    }ورنہ {
      *pzErr = sqlite3MPrintf(db, "%s", zErr);
      sqlite3_free(zErr);
    }
    sqlite3DbFree(db, pVTable);
  }ورنہ اگر( ALWAYS(pVTable->pVtab) ){
    /* Justification of ALWAYS():  A correct vtab constructor must allocate
    ** the sqlite3_vtab object if successful.  */
    memset(pVTable->pVtab, 0, حجم(pVTable->pVtab[0]));
    pVTable->pVtab->pModule = pMod->pModule;
    pMod->nRefModule++;
    pVTable->nRef = 1;
    اگر( sCtx.bDeclared==0 ){
      ثابت حرف *zFormat = "vtable constructor did not declare schema: %s";
      *pzErr = sqlite3MPrintf(db, zFormat, zModuleName);
      sqlite3VtabUnlock(pVTable);
      rc = SQLITE_ERROR;
    }ورنہ{
      صحیح i_ستون;
      u16 oooHidden = 0;
      /* If everything went according to plan, link the new VTable structure
      ** into the linked list headed by pTab->u.vtab.p. Then loop through the
      ** columns of the table to see if any of them contain the token "hidden".
      ** If so, set the Column COLFLAG_HIDDEN flag and remove the token from
      ** the type string.  */
      pVTable->p_اگلا = pTab->u.vtab.p;
      pTab->u.vtab.p = pVTable;

      برائے(i_ستون=0; i_ستون<pTab->n_ستون; i_ستون++){
        حرف *zType = sqlite3_عمود_Type(&pTab->a_ستون[i_ستون], "");
        صحیح nType;
        صحیح i = 0;
        nType = sqlite3Strlen30(zType);
        برائے(i=0; i<nType; i++){
          اگر( 0==sqlite3StrNICmp("hidden", &zType[i], 6)
           && (i==0 || zType[i-1]==' ')
           && (zType[i+6]=='\0' || zType[i+6]==' ')
          ){
            توڑو;
          }
        }
        اگر( i<nType ){
          صحیح j;
          صحیح nDel = 6 + (zType[i+6] ? 1 : 0);
          برائے(j=i; (j+nDel)<=nType; j++){
            zType[j] = zType[j+nDel];
          }
          اگر( zType[i]=='\0' && i>0 ){
            أكد(zType[i-1]==' ');
            zType[i-1] = '\0';
          }
          pTab->a_ستون[i_ستون].ستون_Flags |= COLFLAG_HIDDEN;
          pTab->tabFlags |= TF_HasHidden;
          oooHidden = TF_OOOHidden;
        }ورنہ{
          pTab->tabFlags |= oooHidden;
        }
      }
    }
  }

  sqlite3DbFree(db, zModuleName);
  واپس rc;
}

/*
** This function is invoked by the parser to call the xConnect() method
** of the virtual table pTab. If an error occurs, an error code is returned
** and an error left in pParse.
**
** This call is a no-op if table pTab is not a virtual table.
*/
صحیح sqlite3VtabCallConnect(Parse *pParse, جدول *pTab){
  sqlite3 *db = pParse->db;
  ثابت حرف *zMod;
  Module *pMod;
  صحیح rc;

  أكد( pTab );
  أكد( IsVirtual(pTab) );
  اگر( sqlite3GetVTable(db, pTab) ){
    واپس SQLITE_OK;
  }

  /* Locate the required virtual table module */
  zMod = pTab->u.vtab.azArg[0];
  pMod = (Module*)sqlite3_Hash_ڈھونڈو(&db->aModule, zMod);

  اگر( !pMod ){
    ثابت حرف *zModule = pTab->u.vtab.azArg[0];
    sqlite3_Error_پیغام(pParse, "no such module: %s", zModule);
    rc = SQLITE_ERROR;
  }ورنہ{
    حرف *zErr = 0;
    rc = vtabCallConstructor(db, pTab, pMod, pMod->pModule->xConnect, &zErr);
    اگر( rc!=SQLITE_OK ){
      sqlite3_Error_پیغام(pParse, "%s", zErr);
      pParse->rc = rc;
    }ورنہ{
      sqlite3_Mark_All_Shadow_Tables_کا(db, pTab);
    }
    sqlite3DbFree(db, zErr);
  }

  واپس rc;
}
/*
** Grow the db->aVTrans[] array so that there is room for at least one
** more v-table. Return SQLITE_NOMEM if a malloc fails, or SQLITE_OK otherwise.
*/
ساکن صحیح growVTrans(sqlite3 *db){
  ثابت صحیح ARRAY_INCR = 5;

  /* Grow the sqlite3.aVTrans array if required */
  اگر( (db->nVTrans%ARRAY_INCR)==0 ){
    VTable **aVTrans;
    sqlite3_int64 n_بائٹیں = حجم(sqlite3_vtab*)*
                                 ((sqlite3_int64)db->nVTrans + ARRAY_INCR);
    aVTrans = sqlite3DbRealloc(db, (خالی *)db->aVTrans, n_بائٹیں);
    اگر( !aVTrans ){
      واپس SQLITE_NOMEM_BKPT;
    }
    memset(&aVTrans[db->nVTrans], 0, حجم(sqlite3_vtab *)*ARRAY_INCR);
    db->aVTrans = aVTrans;
  }

  واپس SQLITE_OK;
}

/*
** Add the virtual table pVTab to the array sqlite3.aVTrans[]. Space should
** have already been reserved using growVTrans().
*/
ساکن خالی جوڑو_مقصد_VTrans(sqlite3 *db, VTable *pVTab){
  /* Add pVtab to the end of sqlite3.aVTrans */
  db->aVTrans[db->nVTrans++] = pVTab;
  sqlite3_Vtab_تالا(pVTab);
}

/*
** This function is invoked by the vdbe to call the xCreate method
** of the virtual table named zTab in database iDb.
**
** If an error occurs, *pzErr is set to point to an English language
** description of the error and an SQLITE_XXX error code is returned.
** In this case the caller must call sqlite3DbFree(db, ) on *pzErr.
*/
صحیح sqlite3VtabCallCreate(sqlite3 *db, صحیح iDb, ثابت حرف *zTab, حرف **pzErr){
  صحیح rc = SQLITE_OK;
  جدول *pTab;
  Module *pMod;
  ثابت حرف *zMod;

  pTab = sqlite3_ڈھونڈو_جدول(db, zTab, db->aDb[iDb].zDbSName);
  أكد( pTab && IsVirtual(pTab) && !pTab->u.vtab.p );

  /* Locate the required virtual table module */
  zMod = pTab->u.vtab.azArg[0];
  pMod = (Module*)sqlite3_Hash_ڈھونڈو(&db->aModule, zMod);

  /* If the module has been registered and includes a Create method,
  ** invoke it now. If the module has not been registered, return an
  ** error. Otherwise, do nothing.
  */
  اگر( pMod==0 || pMod->pModule->xCreate==0 || pMod->pModule->xDestroy==0 ){
    *pzErr = sqlite3MPrintf(db, "no such module: %s", zMod);
    rc = SQLITE_ERROR;
  }ورنہ{
    rc = vtabCallConstructor(db, pTab, pMod, pMod->pModule->xCreate, pzErr);
  }

  /* Justification of ALWAYS():  The xConstructor method is required to
  ** create a valid sqlite3_vtab if it returns SQLITE_OK. */
  اگر( rc==SQLITE_OK && ALWAYS(sqlite3GetVTable(db, pTab)) ){
    rc = growVTrans(db);
    اگر( rc==SQLITE_OK ){
      VTable *pVTab = sqlite3GetVTable(db, pTab);
      ثابت sqlite3_module *pModule = pMod->pModule;
      صحیح iSvpt = db->nStatement + db->nSavepoint;
      جوڑو_مقصد_VTrans(db, pVTab);
      /* As in sqlite3VtabBegin(): open the current savepoint on the new
      ** table, so that a ROLLBACK TO that undoes the CREATE also invokes
      ** its xRollbackTo method. */
      اگر( iSvpt && pModule->i_نسخہ>=2 && pModule->xSavepoint ){
        pVTab->iSavepoint = iSvpt;
        rc = pModule->xSavepoint(pVTab->pVtab, iSvpt-1);
      }
    }
  }

  واپس rc;
}

/*
** This function is used to set the schema of a virtual table.  It is only
** valid to call this function from within the xCreate() or xConnect() of a
** virtual table module.
*/
صحیح sqlite3_declare_vtab(sqlite3 *db, ثابت حرف *z_Create_جدول){
  VtabCtx *pCtx;
  صحیح rc = SQLITE_OK;
  جدول *pTab;
  Parse sParse;
  صحیح initBusy;
  صحیح i;
  ثابت غیرعلامتی حرف *z;
  ساکن ثابت u8 aKeyword[] = { TK_CREATE, TK_جدول, 0 };

#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db) || z_Create_جدول==0 ){
    واپس SQLITE_MISUSE_BKPT;
  }
#اگر_ختم

  /* Verify that the first two keywords in the CREATE TABLE statement
  ** really are "CREATE" and "TABLE".  If this is not the case, then
  ** sqlite3_declare_vtab() is being misused.
  */
  z = (ثابت غیرعلامتی حرف*)z_Create_جدول;
  برائے(i=0; aKeyword[i]; i++){
    صحیح tokenType = 0;
    کرو{
      z += sqlite3GetToken(z, &tokenType);
    }جب( tokenType==TK_SPACE || tokenType==TK_COMMENT );
    اگر( tokenType!=aKeyword[i] ){
      sqlite3_Error_With_پیغام(db, SQLITE_ERROR, "syntax error");
      واپس SQLITE_ERROR;
    }
  }
 
  sqlite3_mutex_enter(db->mutex);
  pCtx = db->pVtabCtx;
  اگر( !pCtx || pCtx->bDeclared ){
    sqlite3Error(db, SQLITE_MISUSE_BKPT);
    sqlite3_mutex_leave(db->mutex);
    واپس SQLITE_MISUSE_BKPT;
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
  اگر( SQLITE_OK==sqlite3_تسلسل_Parser(&sParse, z_Create_جدول) ){
    أكد( sParse.p_New_جدول!=0 );
    أكد( !db->mallocFailed );
    أكد( Is_Ordinary_جدول(sParse.p_New_جدول) );
    أكد( sParse.z_Err_پیغام==0 );
    اگر( !pTab->a_ستون ){
      جدول *pNew = sParse.p_New_جدول;
      اشاریہ *pIdx;
      pTab->a_ستون = pNew->a_ستون;
      أكد( Is_Ordinary_جدول(pNew) );
      sqlite3_اظہاریہ_List_Delete(db, pNew->u.tab.pDfltList);
      pTab->nNVCol = pTab->n_ستون = pNew->n_ستون;
      pTab->tabFlags |= pNew->tabFlags & (TF_WithoutRowid|TF_NoVisibleRowid);
      pNew->n_ستون = 0;
      pNew->a_ستون = 0;
      أكد( pTab->p_اشاریہ==0 );
      أكد( HasRowid(pNew) || sqlite3_Primary_کلید_اشاریہ(pNew)!=0 );
      اگر( !HasRowid(pNew)
       && pCtx->pVTable->pMod->pModule->xUpdate!=0
       && sqlite3_Primary_کلید_اشاریہ(pNew)->n_کلید_ستون!=1
      ){
        /* WITHOUT ROWID virtual tables must either be read-only (xUpdate==0)
        ** or else must have a single-column PRIMARY KEY */
        rc = SQLITE_ERROR;
      }
      pIdx = pNew->p_اشاریہ;
      اگر( pIdx ){
        أكد( pIdx->p_اگلا==0 );
        pTab->p_اشاریہ = pIdx;
        pNew->p_اشاریہ = 0;
        pIdx->p_جدول = pTab;
      }
    }
    pCtx->bDeclared = 1;
  }ورنہ{
    sqlite3_Error_With_پیغام(db, SQLITE_ERROR,
          (sParse.z_Err_پیغام ? "%s" : 0), sParse.z_Err_پیغام);
    sqlite3DbFree(db, sParse.z_Err_پیغام);
    rc = SQLITE_ERROR;
  }
  sParse.e_Parse_منوال = PARSE_منوال_معمول;

  اگر( sParse.pVdbe ){
    sqlite3VdbeFinalize(sParse.pVdbe);
  }
  sqlite3_Delete_جدول(db, sParse.p_New_جدول);
  sqlite3ParseObjectReset(&sParse);
  db->init.busy = initBusy;

  أكد( (rc&0xff)==rc );
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  واپس rc;
}

/*
** This function is invoked by the vdbe to call the xDestroy method
** of the virtual table named zTab in database iDb. This occurs
** when a DROP TABLE is mentioned.
**
** This call is a no-op if zTab is not a virtual table.
*/
صحیح sqlite3VtabCallDestroy(sqlite3 *db, صحیح iDb, ثابت حرف *zTab){
  صحیح rc = SQLITE_OK;
  جدول *pTab;

  pTab = sqlite3_ڈھونڈو_جدول(db, zTab, db->aDb[iDb].zDbSName);
  اگر( ALWAYS(pTab!=0)
   && ALWAYS(IsVirtual(pTab))
   && ALWAYS(pTab->u.vtab.p!=0)
  ){
    VTable *p;
    صحیح (*xDestroy)(sqlite3_vtab *);
    برائے(p=pTab->u.vtab.p; p; p=p->p_اگلا){
      أكد( p->pVtab );
      اگر( p->pVtab->nRef>0 ){
        واپس SQLITE_LOCKED;
      }
    }
    p = vtabDisconnectAll(db, pTab);
    xDestroy = p->pMod->pModule->xDestroy;
    اگر( xDestroy==0 ) xDestroy = p->pMod->pModule->xDisconnect;
    أكد( xDestroy!=0 );
    pTab->nTabRef++;
    rc = xDestroy(p->pVtab);
    /* Remove the sqlite3_vtab* from the aVTrans[] array, if applicable */
    اگر( rc==SQLITE_OK ){
      أكد( pTab->u.vtab.p==p && p->p_اگلا==0 );
      p->pVtab = 0;
      pTab->u.vtab.p = 0;
      sqlite3VtabUnlock(p);
    }
    sqlite3_Delete_جدول(db, pTab);
  }

  واپس rc;
}

/*
** This function invokes either the xRollback or xCommit method
** of each of the virtual tables in the sqlite3.aVTrans array. The method
** called is identified by the second argument, "offset", which is
** the offset of the method to call in the sqlite3_module structure.
**
** The array is cleared after invoking the callbacks.
*/
ساکن خالی callFinaliser(sqlite3 *db, صحیح انحراف){
  صحیح i;
  اگر( db->aVTrans ){
    VTable **aVTrans = db->aVTrans;
    db->aVTrans = 0;
    برائے(i=0; i<db->nVTrans; i++){
      VTable *pVTab = aVTrans[i];
      sqlite3_vtab *p = pVTab->pVtab;
      اگر( p ){
        صحیح (*x)(sqlite3_vtab *);
        x = *(صحیح (**)(sqlite3_vtab *))((حرف *)p->pModule + انحراف);
        اگر( x ) x(p);
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
صحیح sqlite3VtabSync(sqlite3 *db, Vdbe *p){
  صحیح i;
  صحیح rc = SQLITE_OK;
  VTable **aVTrans = db->aVTrans;

  db->aVTrans = 0;
  برائے(i=0; rc==SQLITE_OK && i<db->nVTrans; i++){
    صحیح (*x)(sqlite3_vtab *);
    sqlite3_vtab *pVtab = aVTrans[i]->pVtab;
    اگر( pVtab && (x = pVtab->pModule->xSync)!=0 ){
      rc = x(pVtab);
      sqlite3VtabImportErrmsg(p, pVtab);
    }
  }
  db->aVTrans = aVTrans;
  واپس rc;
}

/*
** Invoke the xRollback method of all virtual tables in the
** sqlite3.aVTrans array. Then clear the array itself.
*/
صحیح sqlite3VtabRollback(sqlite3 *db){
  callFinaliser(db, offsetof(sqlite3_module,xRollback));
  واپس SQLITE_OK;
}

/*
** Invoke the xCommit method of all virtual tables in the
** sqlite3.aVTrans array. Then clear the array itself.
*/
صحیح sqlite3VtabCommit(sqlite3 *db){
  callFinaliser(db, offsetof(sqlite3_module,xCommit));
  واپس SQLITE_OK;
}

/*
** If the virtual table pVtab supports the transaction interface
** (xBegin/xRollback/xCommit and optionally xSync) and a transaction is
** not currently open, invoke the xBegin method now.
**
** If the xBegin call is successful, place the sqlite3_vtab pointer
** in the sqlite3.aVTrans array.
*/
صحیح sqlite3VtabBegin(sqlite3 *db, VTable *pVTab){
  صحیح rc = SQLITE_OK;
  ثابت sqlite3_module *pModule;

  /* Special case: If db->aVTrans is NULL and db->nVTrans is greater
  ** than zero, then this function is being called from within a
  ** virtual module xSync() callback. It is illegal to write to
  ** virtual module tables in this case, so return SQLITE_LOCKED.
  */
  اگر( sqlite3_Vtab_اندر_Sync(db) ){
    واپس SQLITE_LOCKED;
  }
  اگر( !pVTab ){
    واپس SQLITE_OK;
  }
  pModule = pVTab->pVtab->pModule;

  اگر( pModule->xBegin ){
    صحیح i;

    /* If pVtab is already in the aVTrans array, return early */
    برائے(i=0; i<db->nVTrans; i++){
      اگر( db->aVTrans[i]==pVTab ){
        واپس SQLITE_OK;
      }
    }

    /* Invoke the xBegin method. If successful, add the vtab to the
    ** sqlite3.aVTrans[] array. */
    rc = growVTrans(db);
    اگر( rc==SQLITE_OK ){
      rc = pModule->xBegin(pVTab->pVtab);
      اگر( rc==SQLITE_OK ){
        صحیح iSvpt = db->nStatement + db->nSavepoint;
        جوڑو_مقصد_VTrans(db, pVTab);
        اگر( iSvpt && pModule->xSavepoint ){
          pVTab->iSavepoint = iSvpt;
          rc = pModule->xSavepoint(pVTab->pVtab, iSvpt-1);
        }
      }
    }
  }
  واپس rc;
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
صحیح sqlite3VtabSavepoint(sqlite3 *db, صحیح op, صحیح iSavepoint){
  صحیح rc = SQLITE_OK;

  أكد( op==SAVEPOINT_RELEASE||op==SAVEPOINT_ROLLBACK||op==SAVEPOINT_BEGIN );
  أكد( iSavepoint>=-1 );
  اگر( db->aVTrans ){
    صحیح i;
    برائے(i=0; rc==SQLITE_OK && i<db->nVTrans; i++){
      VTable *pVTab = db->aVTrans[i];
      ثابت sqlite3_module *pMod = pVTab->pMod->pModule;
      اگر( pVTab->pVtab && pMod->i_نسخہ>=2 ){
        صحیح (*x_طریقہ)(sqlite3_vtab *, صحیح);
        sqlite3_Vtab_تالا(pVTab);
        انتخاب( op ){
          حالت SAVEPOINT_BEGIN:
            x_طریقہ = pMod->xSavepoint;
            pVTab->iSavepoint = iSavepoint+1;
            توڑو;
          حالت SAVEPOINT_ROLLBACK:
            x_طریقہ = pMod->x_Rollback_مقصد;
            توڑو;
          طےشدہ:
            x_طریقہ = pMod->xRelease;
            توڑو;
        }
        اگر( x_طریقہ && pVTab->iSavepoint>iSavepoint ){
          u64 savedFlags = (db->flags & SQLITE_Defensive);
          db->flags &= ~(u64)SQLITE_Defensive;
          rc = x_طریقہ(pVTab->pVtab, iSavepoint);
          db->flags |= savedFlags;
        }
        sqlite3VtabUnlock(pVTab);
      }
    }
  }
  واپس rc;
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
  صحیح nArg,       /* Number of arguments to the function */
  اظہاریہ *p_اظہاریہ     /* First argument to the function */
){
  جدول *pTab;
  sqlite3_vtab *pVtab;
  sqlite3_module *pMod;
  خالی (*xSFunc)(sqlite3_context*,صحیح,sqlite3_قدر**) = 0;
  خالی *pArg = 0;
  FuncDef *pNew;
  صحیح rc = 0;

  /* Check to see the left operand is a column in a virtual table */
  اگر( NEVER(p_اظہاریہ==0) ) واپس pDef;
  اگر( p_اظہاریہ->op!=TK_عمود ) واپس pDef;
  أكد( اظہاریہ_Use_YTab(p_اظہاریہ) );
  pTab = p_اظہاریہ->y.pTab;
  اگر( NEVER(pTab==0) ) واپس pDef;
  اگر( !IsVirtual(pTab) ) واپس pDef;
  pVtab = sqlite3GetVTable(db, pTab)->pVtab;
  أكد( pVtab!=0 );
  أكد( pVtab->pModule!=0 );
  pMod = (sqlite3_module *)pVtab->pModule;
  اگر( pMod->x_ڈھونڈو_Function==0 ) واپس pDef;

  /* Call the xFindFunction method on the virtual table implementation
  ** to see if the implementation wants to overload this function.
  **
  ** Though undocumented, we have historically always invoked xFindFunction
  ** with an all lower-case function name.  Continue in this tradition to
  ** avoid any chance of an incompatibility.
  */
#اگر_تعریف_شدہ SQLITE_DEBUG
  {
    صحیح i;
    برائے(i=0; pDef->zName[i]; i++){
      غیرعلامتی حرف x = (غیرعلامتی حرف)pDef->zName[i];
      أكد( x==sqlite3_بالائی_مقصد_زیریں[x] );
    }
  }
#اگر_ختم
  rc = pMod->x_ڈھونڈو_Function(pVtab, nArg, pDef->zName, &xSFunc, &pArg);
  اگر( rc==0 ){
    واپس pDef;
  }

  /* Create a new ephemeral function definition for the overloaded
  ** function */
  pNew = sqlite3DbMallocZero(db, حجم(*pNew)
                             + sqlite3Strlen30(pDef->zName) + 1);
  اگر( pNew==0 ){
    واپس pDef;
  }
  *pNew = *pDef;
  pNew->zName = (ثابت حرف*)&pNew[1];
  حافظہ_نقل((حرف*)&pNew[1], pDef->zName, sqlite3Strlen30(pDef->zName)+1);
  pNew->xSFunc = xSFunc;
  pNew->p_User_ڈیٹا = pArg;
  pNew->funcFlags |= SQLITE_FUNC_EPHEM;
  واپس pNew;
}

/*
** Make sure virtual table pTab is contained in the pParse->apVirtualLock[]
** array so that an OP_VBegin will get generated for it.  Add pTab to the
** array if it is missing.  If pTab is already in the array, this routine
** is a no-op.
*/
خالی sqlite3VtabMakeWritable(Parse *pParse, جدول *pTab){
  Parse *pToplevel = sqlite3ParseToplevel(pParse);
  صحیح i, n;
  جدول **ap_Vtab_تالا;

  أكد( IsVirtual(pTab) );
  برائے(i=0; i<pToplevel->n_Vtab_تالا; i++){
    اگر( pTab==pToplevel->ap_Vtab_تالا[i] ) واپس;
  }
  n = (pToplevel->n_Vtab_تالا+1)*حجم(pToplevel->ap_Vtab_تالا[0]);
  ap_Vtab_تالا = sqlite3Realloc(pToplevel->ap_Vtab_تالا, n);
  اگر( ap_Vtab_تالا ){
    pToplevel->ap_Vtab_تالا = ap_Vtab_تالا;
    pToplevel->ap_Vtab_تالا[pToplevel->n_Vtab_تالا++] = pTab;
  }ورنہ{
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
صحیح sqlite3_Vtab_Eponymous_جدول_Init(Parse *pParse, Module *pMod){
  ثابت sqlite3_module *pModule = pMod->pModule;
  جدول *pTab;
  حرف *zErr = 0;
  صحیح rc;
  sqlite3 *db = pParse->db;
  اگر( pMod->pEpoTab ) واپس 1;
  اگر( pModule->xCreate!=0 && pModule->xCreate!=pModule->xConnect ) واپس 0;
  pTab = sqlite3DbMallocZero(db, حجم(جدول));
  اگر( pTab==0 ) واپس 0;
  pTab->zName = sqlite3DbStrDup(db, pMod->zName);
  اگر( pTab->zName==0 ){
    sqlite3DbFree(db, pTab);
    واپس 0;
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
  db->n_Schema_تالا++;
  rc = vtabCallConstructor(db, pTab, pMod, pModule->xConnect, &zErr);
  db->n_Schema_تالا--;
  اگر( rc ){
    sqlite3_Error_پیغام(pParse, "%s", zErr);
    pParse->rc = rc;
    sqlite3DbFree(db, zErr);
    sqlite3_Vtab_Eponymous_جدول_Clear(db, pMod);
  }
  واپس 1;
}

/*
** Erase the eponymous virtual table instance associated with
** virtual table module pMod, if it exists.
*/
خالی sqlite3_Vtab_Eponymous_جدول_Clear(sqlite3 *db, Module *pMod){
  جدول *pTab = pMod->pEpoTab;
  اگر( pTab!=0 ){
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
خالی sqlite3_Vtab_Eponymous_جدول_Clear_All(sqlite3 *db){
  HashElem *pElem;
  برائے(pElem=sqlite_Hash_پہلا(&db->aModule); pElem; pElem=sqlite_Hash_اگلا(pElem)){
    sqlite3_Vtab_Eponymous_جدول_Clear(db, (Module*)sqlite_Hash_ڈیٹا(pElem));
  }
}

/*
** Return the ON CONFLICT resolution mode in effect for the virtual
** table update operation currently in progress.
**
** The results of this routine are undefined unless it is called from
** within an xUpdate method.
*/
صحیح sqlite3_vtab_on_conflict(sqlite3 *db){
  ساکن ثابت غیرعلامتی حرف aMap[] = {
    SQLITE_ROLLBACK, SQLITE_ABORT, SQLITE_FAIL, SQLITE_IGNORE, SQLITE_REPLACE
  };
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db) ) واپس SQLITE_MISUSE_BKPT;
#اگر_ختم
  أكد( OE_Rollback==1 && OE_Abort==2 && OE_Fail==3 );
  أكد( OE_Ignore==4 && OE_Replace==5 );
  أكد( db->vtabOnConflict>=1 && db->vtabOnConflict<=5 );
  واپس (صحیح)aMap[db->vtabOnConflict-1];
}

/*
** Call from within the xCreate() or xConnect() methods to provide
** the SQLite core with additional information about the behavior
** of the virtual table being implemented.
*/
صحیح sqlite3_vtab_config(sqlite3 *db, صحیح op, ...){
  va_list ap;
  صحیح rc = SQLITE_OK;
  VtabCtx *p;

#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db) ) واپس SQLITE_MISUSE_BKPT;
#اگر_ختم
  sqlite3_mutex_enter(db->mutex);
  p = db->pVtabCtx;
  اگر( !p ){
    rc = SQLITE_MISUSE_BKPT;
  }ورنہ{
    أكد( p->pTab==0 || IsVirtual(p->pTab) );
    va_start(ap, op);
    انتخاب( op ){
      حالت SQLITE_VTAB_CONSTRAINT_SUPPORT: {
        p->pVTable->bConstraint = (u8)va_arg(ap, صحیح);
        توڑو;
      }
      حالت SQLITE_VTAB_INNOCUOUS: {
        p->pVTable->eVtabRisk = SQLITE_VTABRISK_کم;
        توڑو;
      }
      حالت SQLITE_VTAB_DIRECTONLY: {
        p->pVTable->eVtabRisk = SQLITE_VTABRISK_بلند;
        توڑو;
      }
      حالت SQLITE_VTAB_استعمال_کرتا_ALL_SCHEMAS: {
        p->pVTable->bAllSchemas = 1;
        توڑو;
      }
      طےشدہ: {
        rc = SQLITE_MISUSE_BKPT;
        توڑو;
      }
    }
    va_end(ap);
  }

  اگر( rc!=SQLITE_OK ) sqlite3Error(db, rc);
  sqlite3_mutex_leave(db->mutex);
  واپس rc;
}

#اگر_ختم /* SQLITE_OMIT_VIRTUALTABLE */
