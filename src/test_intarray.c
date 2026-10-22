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
#അന്തര്ഭാവ "test_intarray.h"
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <assert.h>


/*
** Definition of the sqlite3_intarray object.
**
** The internal representation of an intarray object is subject
** to change, is not externally visible, and should be used by
** the implementation of intarray only.  This object is opaque
** to users.
*/
ഘടന sqlite3_intarray {
  പൂർണ്ണസംഖ്യ n;                    /* Number of elements in the array */
  sqlite3_int64 *a;         /* Contents of the array */
  ശൂന്യം (*xFree)(ശൂന്യം*);     /* Function used to free a[] */
};

/* Objects used internally by the virtual table implementation */
തരനിർവചനം ഘടന intarray_vtab intarray_vtab;
തരനിർവചനം ഘടന intarray_cursor intarray_cursor;

/* An intarray table object */
ഘടന intarray_vtab {
  sqlite3_vtab base;            /* Base class */
  sqlite3_intarray *pContent;   /* Content of the integer array */
};

/* An intarray cursor object */
ഘടന intarray_cursor {
  sqlite3_vtab_cursor base;    /* Base class */
  പൂർണ്ണസംഖ്യ i;                       /* Current cursor position */
};

/*
** None of this works unless we have virtual tables.
*/
#ifndef SQLITE_OMIT_VIRTUALTABLE

/*
** Free an sqlite3_intarray object.
*/
സ്ഥിതം ശൂന്യം intarrayFree(ശൂന്യം *pX){
  sqlite3_intarray *p = (sqlite3_intarray*)pX;
  എങ്കിൽ( p->xFree ){
    p->xFree(p->a);
  }
  sqlite3_free(p);
}

/*
** Table destructor for the intarray module.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ intarrayDestroy(sqlite3_vtab *p){
  intarray_vtab *pVtab = (intarray_vtab*)p;
  sqlite3_free(pVtab);
  തിരികെ 0;
}

/*
** Table constructor for the intarray module.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ intarrayCreate(
  sqlite3 *db,              /* Database where module is created */
  ശൂന്യം *pAux,               /* clientdata for the module */
  പൂർണ്ണസംഖ്യ argc,                 /* Number of arguments */
  സ്ഥിരം അക്ഷരം *സ്ഥിരം*argv,   /* Value for all arguments */
  sqlite3_vtab **ppVtab,    /* Write the new virtual table object here */
  അക്ഷരം **pzErr              /* Put error message text here */
){
  പൂർണ്ണസംഖ്യ rc = SQLITE_NOMEM;
  intarray_vtab *pVtab = sqlite3_malloc64(വലുപ്പം(intarray_vtab));

  എങ്കിൽ( pVtab ){
    memset(pVtab, 0, വലുപ്പം(intarray_vtab));
    pVtab->pContent = (sqlite3_intarray*)pAux;
    rc = sqlite3_declare_vtab(db, "CREATE TABLE x(value INTEGER PRIMARY KEY)");
  }
  *ppVtab = (sqlite3_vtab *)pVtab;
  തിരികെ rc;
}

/*
** Open a new cursor on the intarray table.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ intarrayOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  പൂർണ്ണസംഖ്യ rc = SQLITE_NOMEM;
  intarray_cursor *pCur;
  pCur = sqlite3_malloc64(വലുപ്പം(intarray_cursor));
  എങ്കിൽ( pCur ){
    memset(pCur, 0, വലുപ്പം(intarray_cursor));
    *ppCursor = (sqlite3_vtab_cursor *)pCur;
    rc = SQLITE_OK;
  }
  തിരികെ rc;
}

/*
** Close a intarray table cursor.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ intarrayClose(sqlite3_vtab_cursor *cur){
  intarray_cursor *pCur = (intarray_cursor *)cur;
  sqlite3_free(pCur);
  തിരികെ SQLITE_OK;
}

/*
** Retrieve a column of data.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ intarrayColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, പൂർണ്ണസംഖ്യ i){
  intarray_cursor *pCur = (intarray_cursor*)cur;
  intarray_vtab *pVtab = (intarray_vtab*)cur->pVtab;
  എങ്കിൽ( pCur->i>=0 && pCur->i<pVtab->pContent->n ){
    sqlite3_result_int64(ctx, pVtab->pContent->a[pCur->i]);
  }
  തിരികെ SQLITE_OK;
}

/*
** Retrieve the current rowid.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ intarrayRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  intarray_cursor *pCur = (intarray_cursor *)cur;
  *pRowid = pCur->i;
  തിരികെ SQLITE_OK;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ intarrayEof(sqlite3_vtab_cursor *cur){
  intarray_cursor *pCur = (intarray_cursor *)cur;
  intarray_vtab *pVtab = (intarray_vtab *)cur->pVtab;
  തിരികെ pCur->i>=pVtab->pContent->n;
}

/*
** Advance the cursor to the next row.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ intarrayNext(sqlite3_vtab_cursor *cur){
  intarray_cursor *pCur = (intarray_cursor *)cur;
  pCur->i++;
  തിരികെ SQLITE_OK;
}

/*
** Reset a intarray table cursor.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ intarrayFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  പൂർണ്ണസംഖ്യ idx_അക്കം, സ്ഥിരം അക്ഷരം *idxStr,
  പൂർണ്ണസംഖ്യ argc, sqlite3_മൂല്യം **argv
){
  intarray_cursor *pCur = (intarray_cursor *)pVtabCursor;
  pCur->i = 0;
  തിരികെ SQLITE_OK;
}

/*
** Analyse the WHERE condition.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ intarrayBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  തിരികെ SQLITE_OK;
}

/*
** A virtual table module that merely echos method calls into TCL
** variables.
*/
സ്ഥിതം sqlite3_module intarrayModule = {
  0,                           /* iVersion */
  intarrayCreate,              /* xCreate - create a new virtual table */
  intarrayCreate,              /* xConnect - connect to an existing vtab */
  intarrayBestIndex,           /* xBestIndex - find the best query index */
  intarrayDestroy,             /* xDisconnect - disconnect a vtab */
  intarrayDestroy,             /* xDestroy - destroy a vtab */
  intarrayOpen,                /* xOpen - open a cursor */
  intarrayClose,               /* xClose - close a cursor */
  intarrayFilter,              /* xFilter - configure scan constraints */
  intarrayNext,                /* xNext - advance a cursor */
  intarrayEof,                 /* xEof */
  intarrayColumn,              /* xColumn - read data */
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

#endif /* !defined(SQLITE_OMIT_VIRTUALTABLE) */

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
SQLITE_API പൂർണ്ണസംഖ്യ sqlite3_intarray_create(
  sqlite3 *db,
  സ്ഥിരം അക്ഷരം *zName,
  sqlite3_intarray **ppReturn
){
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_intarray *p;

  *ppReturn = p = sqlite3_malloc64( വലുപ്പം(*p) );
  എങ്കിൽ( p==0 ){
    തിരികെ SQLITE_NOMEM;
  }
  memset(p, 0, വലുപ്പം(*p));
  rc = sqlite3_create_module_v2(db, zName, &intarrayModule, p,
                                (ശൂന്യം(*)(ശൂന്യം*))intarrayFree);
  എങ്കിൽ( rc==SQLITE_OK ){
    അക്ഷരം *zSql;
    zSql = sqlite3_mprintf("CREATE VIRTUAL TABLE temp.%Q USING %Q",
                           zName, zName);
    rc = sqlite3_exec(db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
  }
#endif
  തിരികെ rc;
}

/*
** Bind a new array array of integers to a specific intarray object.
**
** The array of integers bound must be unchanged for the duration of
** any query against the corresponding virtual table.  If the integer
** array does change or is deallocated undefined behavior will result.
*/
SQLITE_API പൂർണ്ണസംഖ്യ sqlite3_intarray_bind(
  sqlite3_intarray *pIntArray,   /* The intarray object to bind to */
  പൂർണ്ണസംഖ്യ nElements,                 /* Number of elements in the intarray */
  sqlite3_int64 *aElements,      /* Content of the intarray */
  ശൂന്യം (*xFree)(ശൂന്യം*)           /* How to dispose of the intarray when done */
){
  എങ്കിൽ( pIntArray->xFree ){
    pIntArray->xFree(pIntArray->a);
  }
  pIntArray->n = nElements;
  pIntArray->a = aElements;
  pIntArray->xFree = xFree;
  തിരികെ SQLITE_OK;
}


/*****************************************************************************
** Everything below is interface for testing this module.
*/
#ifdef SQLITE_TEST
#അന്തര്ഭാവ "tclsqlite.h"

/*
** Routines to encode and decode pointers
*/
ബാഹ്യ പൂർണ്ണസംഖ്യ getDbPointer(Tcl_Interp *interp, സ്ഥിരം അക്ഷരം *zA, sqlite3 **ppDb);
ബാഹ്യ ശൂന്യം *sqlite3TestTextToPtr(സ്ഥിരം അക്ഷരം*);
ബാഹ്യ പൂർണ്ണസംഖ്യ sqlite3TestMakePointerStr(Tcl_Interp*, അക്ഷരം *zPtr, ശൂന്യം*);
ബാഹ്യ സ്ഥിരം അക്ഷരം *sqlite3ErrName(പൂർണ്ണസംഖ്യ);

/*
**    sqlite3_intarray_create  DB  NAME
**
** Invoke the sqlite3_intarray_create interface.  A string that becomes
** the first parameter to sqlite3_intarray_bind.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_intarray_create(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  സ്ഥിരം അക്ഷരം *zName;
  sqlite3_intarray *pArray;
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;
  അക്ഷരം zPtr[100];

  എങ്കിൽ( objc!=3 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "DB");
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) തിരികെ TCL_ERROR;
  zName = Tcl_GetString(objv[2]);
#ifndef SQLITE_OMIT_VIRTUALTABLE
  rc = sqlite3_intarray_create(db, zName, &pArray);
#endif
  എങ്കിൽ( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), (അക്ഷരം*)0);
    തിരികെ TCL_ERROR;
  }
  sqlite3TestMakePointerStr(interp, zPtr, pArray);
  Tcl_AppendResult(interp, zPtr, (അക്ഷരം*)0);
  തിരികെ TCL_OK;
}

/*
**    sqlite3_intarray_bind  INTARRAY  ?VALUE ...?
**
** Invoke the sqlite3_intarray_bind interface on the given array of integers.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_intarray_bind(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_intarray *pArray;
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;
  പൂർണ്ണസംഖ്യ i, n;
  sqlite3_int64 *a;

  എങ്കിൽ( objc<2 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "INTARRAY");
    തിരികെ TCL_ERROR;
  }
  pArray = (sqlite3_intarray*)sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  n = objc - 2;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  a = sqlite3_malloc64( വലുപ്പം(a[0])*n );
  എങ്കിൽ( a==0 ){
    Tcl_AppendResult(interp, "SQLITE_NOMEM", (അക്ഷരം*)0);
    തിരികെ TCL_ERROR;
  }
  വേണ്ടി(i=0; i<n; i++){
    Tcl_WideInt x = 0;
    Tcl_GetWideIntFromObj(0, objv[i+2], &x);
    a[i] = x;
  }
  rc = sqlite3_intarray_bind(pArray, n, a, sqlite3_free);
  എങ്കിൽ( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), (അക്ഷരം*)0);
    തിരികെ TCL_ERROR;
  }
#endif
  തിരികെ TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
പൂർണ്ണസംഖ്യ Sqlitetestintarray_Init(Tcl_Interp *interp){
  സ്ഥിതം ഘടന {
     അക്ഷരം *zName;
     Tcl_ObjCmdProc *xProc;
     ശൂന്യം *clientData;
  } aObjCmd[] = {
     { "sqlite3_intarray_create", test_intarray_create, 0 },
     { "sqlite3_intarray_bind", test_intarray_bind, 0 },
  };
  പൂർണ്ണസംഖ്യ i;
  വേണ്ടി(i=0; i<വലുപ്പം(aObjCmd)/വലുപ്പം(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
  തിരികെ TCL_OK;
}

#endif /* SQLITE_TEST */
