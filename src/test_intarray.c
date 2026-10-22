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
#include "test_intarray.h"
#include <string.h>
#include <assert.h>


/*
** Definition of the sqlite3_intarray object.
**
** The internal representation of an intarray object is subject
** to change, is not externally visible, and should be used by
** the implementation of intarray only.  This object is opaque
** to users.
*/
struct sqlite3_intarray {
  整数 n;                    /* Number of elements in the array */
  sqlite3_int64 *a;         /* Contents of the array */
  void (*xFree)(void*);     /* Function used to free a[] */
};

/* Objects used internally by the virtual table implementation */
typedef struct intarray_vtab intarray_vtab;
typedef struct intarray_cursor intarray_cursor;

/* An intarray table object */
struct intarray_vtab {
  sqlite3_vtab base;            /* Base class */
  sqlite3_intarray *pContent;   /* Content of the integer array */
};

/* An intarray cursor object */
struct intarray_cursor {
  sqlite3_vtab_cursor base;    /* Base class */
  整数 i;                       /* Current cursor position */
};

/*
** None of this works unless we have virtual tables.
*/
#ifndef SQLITE_OMIT_VIRTUALTABLE

/*
** Free an sqlite3_intarray object.
*/
static void intarrayFree(void *pX){
  sqlite3_intarray *p = (sqlite3_intarray*)pX;
  如果( p->xFree ){
    p->xFree(p->a);
  }
  sqlite3_free(p);
}

/*
** Table destructor for the intarray module.
*/
static 整数 intarrayDestroy(sqlite3_vtab *p){
  intarray_vtab *pVtab = (intarray_vtab*)p;
  sqlite3_free(pVtab);
  返回 0;
}

/*
** Table constructor for the intarray module.
*/
static 整数 intarrayCreate(
  sqlite3 *db,              /* Database where module is created */
  void *pAux,               /* clientdata for the module */
  整数 argc,                 /* Number of arguments */
  const char *const*argv,   /* Value for all arguments */
  sqlite3_vtab **ppVtab,    /* Write the new virtual table object here */
  char **pzErr              /* Put error message text here */
){
  整数 rc = SQLITE_NOMEM;
  intarray_vtab *pVtab = sqlite3_malloc64(sizeof(intarray_vtab));

  如果( pVtab ){
    memset(pVtab, 0, sizeof(intarray_vtab));
    pVtab->pContent = (sqlite3_intarray*)pAux;
    rc = sqlite3_declare_vtab(db, "CREATE TABLE x(value INTEGER PRIMARY KEY)");
  }
  *ppVtab = (sqlite3_vtab *)pVtab;
  返回 rc;
}

/*
** Open a new cursor on the intarray table.
*/
static 整数 intarrayOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  整数 rc = SQLITE_NOMEM;
  intarray_cursor *pCur;
  pCur = sqlite3_malloc64(sizeof(intarray_cursor));
  如果( pCur ){
    memset(pCur, 0, sizeof(intarray_cursor));
    *ppCursor = (sqlite3_vtab_cursor *)pCur;
    rc = SQLITE_OK;
  }
  返回 rc;
}

/*
** Close a intarray table cursor.
*/
static 整数 intarrayClose(sqlite3_vtab_cursor *cur){
  intarray_cursor *pCur = (intarray_cursor *)cur;
  sqlite3_free(pCur);
  返回 SQLITE_OK;
}

/*
** Retrieve a column of data.
*/
static 整数 intarrayColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, 整数 i){
  intarray_cursor *pCur = (intarray_cursor*)cur;
  intarray_vtab *pVtab = (intarray_vtab*)cur->pVtab;
  如果( pCur->i>=0 && pCur->i<pVtab->pContent->n ){
    sqlite3_result_int64(ctx, pVtab->pContent->a[pCur->i]);
  }
  返回 SQLITE_OK;
}

/*
** Retrieve the current rowid.
*/
static 整数 intarrayRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  intarray_cursor *pCur = (intarray_cursor *)cur;
  *pRowid = pCur->i;
  返回 SQLITE_OK;
}

static 整数 intarrayEof(sqlite3_vtab_cursor *cur){
  intarray_cursor *pCur = (intarray_cursor *)cur;
  intarray_vtab *pVtab = (intarray_vtab *)cur->pVtab;
  返回 pCur->i>=pVtab->pContent->n;
}

/*
** Advance the cursor to the next row.
*/
static 整数 intarrayNext(sqlite3_vtab_cursor *cur){
  intarray_cursor *pCur = (intarray_cursor *)cur;
  pCur->i++;
  返回 SQLITE_OK;
}

/*
** Reset a intarray table cursor.
*/
static 整数 intarrayFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  整数 idxNum, const char *idxStr,
  整数 argc, sqlite3_value **argv
){
  intarray_cursor *pCur = (intarray_cursor *)pVtabCursor;
  pCur->i = 0;
  返回 SQLITE_OK;
}

/*
** Analyse the WHERE condition.
*/
static 整数 intarrayBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  返回 SQLITE_OK;
}

/*
** A virtual table module that merely echos method calls into TCL
** variables.
*/
static sqlite3_module intarrayModule = {
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
SQLITE_API 整数 sqlite3_intarray_create(
  sqlite3 *db,
  const char *zName,
  sqlite3_intarray **ppReturn
){
  整数 rc = SQLITE_OK;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_intarray *p;

  *ppReturn = p = sqlite3_malloc64( sizeof(*p) );
  如果( p==0 ){
    返回 SQLITE_NOMEM;
  }
  memset(p, 0, sizeof(*p));
  rc = sqlite3_create_module_v2(db, zName, &intarrayModule, p,
                                (void(*)(void*))intarrayFree);
  如果( rc==SQLITE_OK ){
    char *zSql;
    zSql = sqlite3_mprintf("CREATE VIRTUAL TABLE temp.%Q USING %Q",
                           zName, zName);
    rc = sqlite3_exec(db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
  }
#endif
  返回 rc;
}

/*
** Bind a new array array of integers to a specific intarray object.
**
** The array of integers bound must be unchanged for the duration of
** any query against the corresponding virtual table.  If the integer
** array does change or is deallocated undefined behavior will result.
*/
SQLITE_API 整数 sqlite3_intarray_bind(
  sqlite3_intarray *pIntArray,   /* The intarray object to bind to */
  整数 nElements,                 /* Number of elements in the intarray */
  sqlite3_int64 *aElements,      /* Content of the intarray */
  void (*xFree)(void*)           /* How to dispose of the intarray when done */
){
  如果( pIntArray->xFree ){
    pIntArray->xFree(pIntArray->a);
  }
  pIntArray->n = nElements;
  pIntArray->a = aElements;
  pIntArray->xFree = xFree;
  返回 SQLITE_OK;
}


/*****************************************************************************
** Everything below is interface for testing this module.
*/
#ifdef SQLITE_TEST
#include "tclsqlite.h"

/*
** Routines to encode and decode pointers
*/
extern 整数 getDbPointer(Tcl_Interp *interp, const char *zA, sqlite3 **ppDb);
extern void *sqlite3TestTextToPtr(const char*);
extern 整数 sqlite3TestMakePointerStr(Tcl_Interp*, char *zPtr, void*);
extern const char *sqlite3ErrName(整数);

/*
**    sqlite3_intarray_create  DB  NAME
**
** Invoke the sqlite3_intarray_create interface.  A string that becomes
** the first parameter to sqlite3_intarray_bind.
*/
static 整数 SQLITE_TCLAPI test_intarray_create(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  整数 objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  const char *zName;
  sqlite3_intarray *pArray;
  整数 rc = SQLITE_OK;
  char zPtr[100];

  如果( objc!=3 ){
    Tcl_WrongNumArgs(interp, 1, objv, "DB");
    返回 TCL_ERROR;
  }
  如果( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) 返回 TCL_ERROR;
  zName = Tcl_GetString(objv[2]);
#ifndef SQLITE_OMIT_VIRTUALTABLE
  rc = sqlite3_intarray_create(db, zName, &pArray);
#endif
  如果( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), (char*)0);
    返回 TCL_ERROR;
  }
  sqlite3TestMakePointerStr(interp, zPtr, pArray);
  Tcl_AppendResult(interp, zPtr, (char*)0);
  返回 TCL_OK;
}

/*
**    sqlite3_intarray_bind  INTARRAY  ?VALUE ...?
**
** Invoke the sqlite3_intarray_bind interface on the given array of integers.
*/
static 整数 SQLITE_TCLAPI test_intarray_bind(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  整数 objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_intarray *pArray;
  整数 rc = SQLITE_OK;
  整数 i, n;
  sqlite3_int64 *a;

  如果( objc<2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "INTARRAY");
    返回 TCL_ERROR;
  }
  pArray = (sqlite3_intarray*)sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  n = objc - 2;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  a = sqlite3_malloc64( sizeof(a[0])*n );
  如果( a==0 ){
    Tcl_AppendResult(interp, "SQLITE_NOMEM", (char*)0);
    返回 TCL_ERROR;
  }
  为(i=0; i<n; i++){
    Tcl_WideInt x = 0;
    Tcl_GetWideIntFromObj(0, objv[i+2], &x);
    a[i] = x;
  }
  rc = sqlite3_intarray_bind(pArray, n, a, sqlite3_free);
  如果( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), (char*)0);
    返回 TCL_ERROR;
  }
#endif
  返回 TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
整数 Sqlitetestintarray_Init(Tcl_Interp *interp){
  static struct {
     char *zName;
     Tcl_ObjCmdProc *xProc;
     void *clientData;
  } aObjCmd[] = {
     { "sqlite3_intarray_create", test_intarray_create, 0 },
     { "sqlite3_intarray_bind", test_intarray_bind, 0 },
  };
  整数 i;
  为(i=0; i<sizeof(aObjCmd)/sizeof(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
  返回 TCL_OK;
}

#endif /* SQLITE_TEST */
