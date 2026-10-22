/*
** 2009 November 10
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
**
** This file implements a read-only VIRTUAL TABLE that contains the
** content of a C-language array of integer values.  See the corresponding
** header file for full details.
**
** This virtual table is used for internal testing of SQLite only.  It is
** not recommended for use in production.  For a similar virtual table that
** is production-ready, see the "carray" virtual table over in ext/misc.
*/
#ਅਨ੍ਤਰ੍ਭਾਵ "test_intarray.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>


/*
** Definition of the sqlite3_intarray object.
**
** The internal representation of an intarray object is subject
** to change, is not externally visible, and should be used by
** the implementation of intarray only.  This object is opaque
** to users.
*/
ਬਣਤਰ sqlite3_intarray {
  ਪੂਰਨ n;                    /* Number of elements in the array */
  sqlite3_int64 *a;         /* Contents of the array */
  ਖਾਲੀ (*xFree)(ਖਾਲੀ*);     /* Function used to free a[] */
};

/* Objects used internally by the virtual table implementation */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ intarray_vtab intarray_vtab;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ intarray_cursor intarray_cursor;

/* An intarray table object */
ਬਣਤਰ intarray_vtab {
  sqlite3_vtab ਅਧਾਰ;            /* Base class */
  sqlite3_intarray *pContent;   /* Content of the integer array */
};

/* An intarray cursor object */
ਬਣਤਰ intarray_cursor {
  sqlite3_vtab_cursor ਅਧਾਰ;    /* Base class */
  ਪੂਰਨ i;                       /* Current cursor position */
};

/*
** None of this works unless we have virtual tables.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE

/*
** Free an sqlite3_intarray object.
*/
ਸਥਿਰ ਖਾਲੀ intarrayFree(ਖਾਲੀ *pX){
  sqlite3_intarray *p = (sqlite3_intarray*)pX;
  ਜੇ( p->xFree ){
    p->xFree(p->a);
  }
  sqlite3_free(p);
}

/*
** Table destructor for the intarray module.
*/
ਸਥਿਰ ਪੂਰਨ intarrayDestroy(sqlite3_vtab *p){
  intarray_vtab *pVtab = (intarray_vtab*)p;
  sqlite3_free(pVtab);
  ਵਾਪਸ 0;
}

/*
** Table constructor for the intarray module.
*/
ਸਥਿਰ ਪੂਰਨ intarrayCreate(
  sqlite3 *db,              /* Database where module is created */
  ਖਾਲੀ *pAux,               /* clientdata for the module */
  ਪੂਰਨ argc,                 /* Number of arguments */
  ਅਚਲ ਅੱਖਰ *ਅਚਲ*argv,   /* Value for all arguments */
  sqlite3_vtab **ppVtab,    /* Write the new virtual table object here */
  ਅੱਖਰ **pzErr              /* Put error message text here */
){
  ਪੂਰਨ rc = SQLITE_NOMEM;
  intarray_vtab *pVtab = sqlite3_malloc64(ਆਕਾਰ(intarray_vtab));

  ਜੇ( pVtab ){
    memset(pVtab, 0, ਆਕਾਰ(intarray_vtab));
    pVtab->pContent = (sqlite3_intarray*)pAux;
    rc = sqlite3_declare_vtab(db, "CREATE TABLE x(value INTEGER PRIMARY KEY)");
  }
  *ppVtab = (sqlite3_vtab *)pVtab;
  ਵਾਪਸ rc;
}

/*
** Open a new cursor on the intarray table.
*/
ਸਥਿਰ ਪੂਰਨ intarrayOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  ਪੂਰਨ rc = SQLITE_NOMEM;
  intarray_cursor *pCur;
  pCur = sqlite3_malloc64(ਆਕਾਰ(intarray_cursor));
  ਜੇ( pCur ){
    memset(pCur, 0, ਆਕਾਰ(intarray_cursor));
    *ppCursor = (sqlite3_vtab_cursor *)pCur;
    rc = SQLITE_OK;
  }
  ਵਾਪਸ rc;
}

/*
** Close a intarray table cursor.
*/
ਸਥਿਰ ਪੂਰਨ intarrayClose(sqlite3_vtab_cursor *cur){
  intarray_cursor *pCur = (intarray_cursor *)cur;
  sqlite3_free(pCur);
  ਵਾਪਸ SQLITE_OK;
}

/*
** Retrieve a column of data.
*/
ਸਥਿਰ ਪੂਰਨ intarray_ਕਾਲਮ(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, ਪੂਰਨ i){
  intarray_cursor *pCur = (intarray_cursor*)cur;
  intarray_vtab *pVtab = (intarray_vtab*)cur->pVtab;
  ਜੇ( pCur->i>=0 && pCur->i<pVtab->pContent->n ){
    sqlite3_ਨਤੀਜਾ_int64(ctx, pVtab->pContent->a[pCur->i]);
  }
  ਵਾਪਸ SQLITE_OK;
}

/*
** Retrieve the current rowid.
*/
ਸਥਿਰ ਪੂਰਨ intarrayRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  intarray_cursor *pCur = (intarray_cursor *)cur;
  *pRowid = pCur->i;
  ਵਾਪਸ SQLITE_OK;
}

ਸਥਿਰ ਪੂਰਨ intarrayEof(sqlite3_vtab_cursor *cur){
  intarray_cursor *pCur = (intarray_cursor *)cur;
  intarray_vtab *pVtab = (intarray_vtab *)cur->pVtab;
  ਵਾਪਸ pCur->i>=pVtab->pContent->n;
}

/*
** Advance the cursor to the next row.
*/
ਸਥਿਰ ਪੂਰਨ intarray_ਅਗਲਾ(sqlite3_vtab_cursor *cur){
  intarray_cursor *pCur = (intarray_cursor *)cur;
  pCur->i++;
  ਵਾਪਸ SQLITE_OK;
}

/*
** Reset a intarray table cursor.
*/
ਸਥਿਰ ਪੂਰਨ intarrayFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  ਪੂਰਨ idx_ਅੰਕ, ਅਚਲ ਅੱਖਰ *idxStr,
  ਪੂਰਨ argc, sqlite3_ਮੁੱਲ **argv
){
  intarray_cursor *pCur = (intarray_cursor *)pVtabCursor;
  pCur->i = 0;
  ਵਾਪਸ SQLITE_OK;
}

/*
** Analyse the WHERE condition.
*/
ਸਥਿਰ ਪੂਰਨ intarray_ਉੱਤਮ_ਸੂਚਕਾਂਕ(sqlite3_vtab *tab, sqlite3_ਸੂਚਕਾਂਕ_info *pIdxInfo){
  ਵਾਪਸ SQLITE_OK;
}

/*
** A virtual table module that merely echos method calls into TCL
** variables.
*/
ਸਥਿਰ sqlite3_module intarrayModule = {
  0,                           /* iVersion */
  intarrayCreate,              /* xCreate - create a new virtual table */
  intarrayCreate,              /* xConnect - connect to an existing vtab */
  intarray_ਉੱਤਮ_ਸੂਚਕਾਂਕ,           /* xBestIndex - find the best query index */
  intarrayDestroy,             /* xDisconnect - disconnect a vtab */
  intarrayDestroy,             /* xDestroy - destroy a vtab */
  intarrayOpen,                /* xOpen - open a cursor */
  intarrayClose,               /* xClose - close a cursor */
  intarrayFilter,              /* xFilter - configure scan constraints */
  intarray_ਅਗਲਾ,                /* xNext - advance a cursor */
  intarrayEof,                 /* xEof */
  intarray_ਕਾਲਮ,              /* xColumn - read data */
  intarrayRowid,               /* xRowid - read data */
  0,                           /* xUpdate */
  0,                           /* xBegin */
  0,                           /* xSync */
  0,                           /* xCommit */
  0,                           /* xRollback */
  0,                           /* xFindMethod */
  0,                           /* xRename */
  0,                           /* xSavepoint */
  0,                           /* xRelease */
  0,                           /* xRollbackTo */
  0,                           /* xShadowName */
  0                            /* xIntegrity */
};

#ਜੇ_ਅੰਤ /* !defined(SQLITE_OMIT_VIRTUALTABLE) */

/*
** Invoke this routine to create a specific instance of an intarray object.
** The new intarray object is returned by the 3rd parameter.
**
** Each intarray object corresponds to a virtual table in the TEMP table
** with a name of zName.
**
** Destroy the intarray object by dropping the virtual table.  If not done
** explicitly by the application, the virtual table will be dropped implicitly
** by the system when the database connection is closed.
*/
SQLITE_API ਪੂਰਨ sqlite3_intarray_create(
  sqlite3 *db,
  ਅਚਲ ਅੱਖਰ *zName,
  sqlite3_intarray **ppReturn
){
  ਪੂਰਨ rc = SQLITE_OK;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
  sqlite3_intarray *p;

  *ppReturn = p = sqlite3_malloc64( ਆਕਾਰ(*p) );
  ਜੇ( p==0 ){
    ਵਾਪਸ SQLITE_NOMEM;
  }
  memset(p, 0, ਆਕਾਰ(*p));
  rc = sqlite3_create_module_v2(db, zName, &intarrayModule, p,
                                (ਖਾਲੀ(*)(ਖਾਲੀ*))intarrayFree);
  ਜੇ( rc==SQLITE_OK ){
    ਅੱਖਰ *zSql;
    zSql = sqlite3_mprintf("CREATE VIRTUAL TABLE temp.%Q USING %Q",
                           zName, zName);
    rc = sqlite3_exec(db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
  }
#ਜੇ_ਅੰਤ
  ਵਾਪਸ rc;
}

/*
** Bind a new array array of integers to a specific intarray object.
**
** The array of integers bound must be unchanged for the duration of
** any query against the corresponding virtual table.  If the integer
** array does change or is deallocated undefined behavior will result.
*/
SQLITE_API ਪੂਰਨ sqlite3_intarray_bind(
  sqlite3_intarray *pIntArray,   /* The intarray object to bind to */
  ਪੂਰਨ nElements,                 /* Number of elements in the intarray */
  sqlite3_int64 *aElements,      /* Content of the intarray */
  ਖਾਲੀ (*xFree)(ਖਾਲੀ*)           /* How to dispose of the intarray when done */
){
  ਜੇ( pIntArray->xFree ){
    pIntArray->xFree(pIntArray->a);
  }
  pIntArray->n = nElements;
  pIntArray->a = aElements;
  pIntArray->xFree = xFree;
  ਵਾਪਸ SQLITE_OK;
}


/*****************************************************************************
** Everything below is interface for testing this module.
*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_TEST
#ਅਨ੍ਤਰ੍ਭਾਵ "tclsqlite.h"

/*
** Routines to encode and decode pointers
*/
ਬਾਹਰੀ ਪੂਰਨ get_Db_ਸੂਚਕ(Tcl_Interp *interp, ਅਚਲ ਅੱਖਰ *zA, sqlite3 **ppDb);
ਬਾਹਰੀ ਖਾਲੀ *sqlite3_Test_ਪਾਠ_ਮੰਜ਼ਿਲ_Ptr(ਅਚਲ ਅੱਖਰ*);
ਬਾਹਰੀ ਪੂਰਨ sqlite3_Test_Make_ਸੂਚਕ_Str(Tcl_Interp*, ਅੱਖਰ *zPtr, ਖਾਲੀ*);
ਬਾਹਰੀ ਅਚਲ ਅੱਖਰ *sqlite3ErrName(ਪੂਰਨ);

/*
**    sqlite3_intarray_create  DB  NAME
**
** Invoke the sqlite3_intarray_create interface.  A string that becomes
** the first parameter to sqlite3_intarray_bind.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_intarray_create(
  Client_ਅੰਕੜੇ client_ਅੰਕੜੇ, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  ਅਚਲ ਅੱਖਰ *zName;
  sqlite3_intarray *pArray;
  ਪੂਰਨ rc = SQLITE_OK;
  ਅੱਖਰ zPtr[100];

  ਜੇ( objc!=3 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "DB");
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( get_Db_ਸੂਚਕ(interp, Tcl_GetString(objv[1]), &db) ) ਵਾਪਸ TCL_ERROR;
  zName = Tcl_GetString(objv[2]);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
  rc = sqlite3_intarray_create(db, zName, &pArray);
#ਜੇ_ਅੰਤ
  ਜੇ( rc!=SQLITE_OK ){
    Tcl_Append_ਨਤੀਜਾ(interp, sqlite3ErrName(rc), (ਅੱਖਰ*)0);
    ਵਾਪਸ TCL_ERROR;
  }
  sqlite3_Test_Make_ਸੂਚਕ_Str(interp, zPtr, pArray);
  Tcl_Append_ਨਤੀਜਾ(interp, zPtr, (ਅੱਖਰ*)0);
  ਵਾਪਸ TCL_OK;
}

/*
**    sqlite3_intarray_bind  INTARRAY  ?VALUE ...?
**
** Invoke the sqlite3_intarray_bind interface on the given array of integers.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_intarray_bind(
  Client_ਅੰਕੜੇ client_ਅੰਕੜੇ, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_intarray *pArray;
  ਪੂਰਨ rc = SQLITE_OK;
  ਪੂਰਨ i, n;
  sqlite3_int64 *a;

  ਜੇ( objc<2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "INTARRAY");
    ਵਾਪਸ TCL_ERROR;
  }
  pArray = (sqlite3_intarray*)sqlite3_Test_ਪਾਠ_ਮੰਜ਼ਿਲ_Ptr(Tcl_GetString(objv[1]));
  n = objc - 2;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
  a = sqlite3_malloc64( ਆਕਾਰ(a[0])*n );
  ਜੇ( a==0 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "SQLITE_NOMEM", (ਅੱਖਰ*)0);
    ਵਾਪਸ TCL_ERROR;
  }
  ਲਈ(i=0; i<n; i++){
    Tcl_WideInt x = 0;
    Tcl_GetWideIntFromObj(0, objv[i+2], &x);
    a[i] = x;
  }
  rc = sqlite3_intarray_bind(pArray, n, a, sqlite3_free);
  ਜੇ( rc!=SQLITE_OK ){
    Tcl_Append_ਨਤੀਜਾ(interp, sqlite3ErrName(rc), (ਅੱਖਰ*)0);
    ਵਾਪਸ TCL_ERROR;
  }
#ਜੇ_ਅੰਤ
  ਵਾਪਸ TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
ਪੂਰਨ Sqlitetestintarray_Init(Tcl_Interp *interp){
  ਸਥਿਰ ਬਣਤਰ {
     ਅੱਖਰ *zName;
     Tcl_ObjCmdProc *xProc;
     ਖਾਲੀ *client_ਅੰਕੜੇ;
  } aObjCmd[] = {
     { "sqlite3_intarray_create", test_intarray_create, 0 },
     { "sqlite3_intarray_bind", test_intarray_bind, 0 },
  };
  ਪੂਰਨ i;
  ਲਈ(i=0; i<ਆਕਾਰ(aObjCmd)/ਆਕਾਰ(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].client_ਅੰਕੜੇ, 0);
  }
  ਵਾਪਸ TCL_OK;
}

#ਜੇ_ਅੰਤ /* SQLITE_TEST */
