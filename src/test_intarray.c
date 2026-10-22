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
#અન્તર્ભાવ "test_intarray.h"
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <assert.h>


/*
** Definition of the sqlite3_intarray object.
**
** The internal representation of an intarray object is subject
** to change, is not externally visible, and should be used by
** the implementation of intarray only.  This object is opaque
** to users.
*/
સંરચના sqlite3_intarray {
  પૂર્ણાંક n;                    /* Number of elements in the array */
  sqlite3_int64 *a;         /* Contents of the array */
  શૂન્ય (*xFree)(શૂન્ય*);     /* Function used to free a[] */
};

/* Objects used internally by the virtual table implementation */
પ્રકારવ્યાખ્યા સંરચના intarray_vtab intarray_vtab;
પ્રકારવ્યાખ્યા સંરચના intarray_cursor intarray_cursor;

/* An intarray table object */
સંરચના intarray_vtab {
  sqlite3_vtab base;            /* Base class */
  sqlite3_intarray *pContent;   /* Content of the integer array */
};

/* An intarray cursor object */
સંરચના intarray_cursor {
  sqlite3_vtab_cursor base;    /* Base class */
  પૂર્ણાંક i;                       /* Current cursor position */
};

/*
** None of this works unless we have virtual tables.
*/
#ifndef SQLITE_OMIT_VIRTUALTABLE

/*
** Free an sqlite3_intarray object.
*/
સ્થિર શૂન્ય intarrayFree(શૂન્ય *pX){
  sqlite3_intarray *p = (sqlite3_intarray*)pX;
  જો( p->xFree ){
    p->xFree(p->a);
  }
  sqlite3_free(p);
}

/*
** Table destructor for the intarray module.
*/
સ્થિર પૂર્ણાંક intarrayDestroy(sqlite3_vtab *p){
  intarray_vtab *pVtab = (intarray_vtab*)p;
  sqlite3_free(pVtab);
  પાછા 0;
}

/*
** Table constructor for the intarray module.
*/
સ્થિર પૂર્ણાંક intarrayCreate(
  sqlite3 *db,              /* Database where module is created */
  શૂન્ય *pAux,               /* clientdata for the module */
  પૂર્ણાંક argc,                 /* Number of arguments */
  અચલ અક્ષર *અચલ*argv,   /* Value for all arguments */
  sqlite3_vtab **ppVtab,    /* Write the new virtual table object here */
  અક્ષર **pzErr              /* Put error message text here */
){
  પૂર્ણાંક rc = SQLITE_NOMEM;
  intarray_vtab *pVtab = sqlite3_malloc64(કદ(intarray_vtab));

  જો( pVtab ){
    memset(pVtab, 0, કદ(intarray_vtab));
    pVtab->pContent = (sqlite3_intarray*)pAux;
    rc = sqlite3_declare_vtab(db, "CREATE TABLE x(value INTEGER PRIMARY KEY)");
  }
  *ppVtab = (sqlite3_vtab *)pVtab;
  પાછા rc;
}

/*
** Open a new cursor on the intarray table.
*/
સ્થિર પૂર્ણાંક intarrayOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  પૂર્ણાંક rc = SQLITE_NOMEM;
  intarray_cursor *pCur;
  pCur = sqlite3_malloc64(કદ(intarray_cursor));
  જો( pCur ){
    memset(pCur, 0, કદ(intarray_cursor));
    *ppCursor = (sqlite3_vtab_cursor *)pCur;
    rc = SQLITE_OK;
  }
  પાછા rc;
}

/*
** Close a intarray table cursor.
*/
સ્થિર પૂર્ણાંક intarrayClose(sqlite3_vtab_cursor *cur){
  intarray_cursor *pCur = (intarray_cursor *)cur;
  sqlite3_free(pCur);
  પાછા SQLITE_OK;
}

/*
** Retrieve a column of data.
*/
સ્થિર પૂર્ણાંક intarrayColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, પૂર્ણાંક i){
  intarray_cursor *pCur = (intarray_cursor*)cur;
  intarray_vtab *pVtab = (intarray_vtab*)cur->pVtab;
  જો( pCur->i>=0 && pCur->i<pVtab->pContent->n ){
    sqlite3_result_int64(ctx, pVtab->pContent->a[pCur->i]);
  }
  પાછા SQLITE_OK;
}

/*
** Retrieve the current rowid.
*/
સ્થિર પૂર્ણાંક intarrayRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  intarray_cursor *pCur = (intarray_cursor *)cur;
  *pRowid = pCur->i;
  પાછા SQLITE_OK;
}

સ્થિર પૂર્ણાંક intarrayEof(sqlite3_vtab_cursor *cur){
  intarray_cursor *pCur = (intarray_cursor *)cur;
  intarray_vtab *pVtab = (intarray_vtab *)cur->pVtab;
  પાછા pCur->i>=pVtab->pContent->n;
}

/*
** Advance the cursor to the next row.
*/
સ્થિર પૂર્ણાંક intarrayNext(sqlite3_vtab_cursor *cur){
  intarray_cursor *pCur = (intarray_cursor *)cur;
  pCur->i++;
  પાછા SQLITE_OK;
}

/*
** Reset a intarray table cursor.
*/
સ્થિર પૂર્ણાંક intarrayFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  પૂર્ણાંક idx_અંક, અચલ અક્ષર *idxStr,
  પૂર્ણાંક argc, sqlite3_કિંમત **argv
){
  intarray_cursor *pCur = (intarray_cursor *)pVtabCursor;
  pCur->i = 0;
  પાછા SQLITE_OK;
}

/*
** Analyse the WHERE condition.
*/
સ્થિર પૂર્ણાંક intarrayBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  પાછા SQLITE_OK;
}

/*
** A virtual table module that merely echos method calls into TCL
** variables.
*/
સ્થિર sqlite3_module intarrayModule = {
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
SQLITE_API પૂર્ણાંક sqlite3_intarray_create(
  sqlite3 *db,
  અચલ અક્ષર *zName,
  sqlite3_intarray **ppReturn
){
  પૂર્ણાંક rc = SQLITE_OK;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_intarray *p;

  *ppReturn = p = sqlite3_malloc64( કદ(*p) );
  જો( p==0 ){
    પાછા SQLITE_NOMEM;
  }
  memset(p, 0, કદ(*p));
  rc = sqlite3_create_module_v2(db, zName, &intarrayModule, p,
                                (શૂન્ય(*)(શૂન્ય*))intarrayFree);
  જો( rc==SQLITE_OK ){
    અક્ષર *zSql;
    zSql = sqlite3_mprintf("CREATE VIRTUAL TABLE temp.%Q USING %Q",
                           zName, zName);
    rc = sqlite3_exec(db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
  }
#endif
  પાછા rc;
}

/*
** Bind a new array array of integers to a specific intarray object.
**
** The array of integers bound must be unchanged for the duration of
** any query against the corresponding virtual table.  If the integer
** array does change or is deallocated undefined behavior will result.
*/
SQLITE_API પૂર્ણાંક sqlite3_intarray_bind(
  sqlite3_intarray *pIntArray,   /* The intarray object to bind to */
  પૂર્ણાંક nElements,                 /* Number of elements in the intarray */
  sqlite3_int64 *aElements,      /* Content of the intarray */
  શૂન્ય (*xFree)(શૂન્ય*)           /* How to dispose of the intarray when done */
){
  જો( pIntArray->xFree ){
    pIntArray->xFree(pIntArray->a);
  }
  pIntArray->n = nElements;
  pIntArray->a = aElements;
  pIntArray->xFree = xFree;
  પાછા SQLITE_OK;
}


/*****************************************************************************
** Everything below is interface for testing this module.
*/
#ifdef SQLITE_TEST
#અન્તર્ભાવ "tclsqlite.h"

/*
** Routines to encode and decode pointers
*/
બાહ્ય પૂર્ણાંક getDbPointer(Tcl_Interp *interp, અચલ અક્ષર *zA, sqlite3 **ppDb);
બાહ્ય શૂન્ય *sqlite3TestTextToPtr(અચલ અક્ષર*);
બાહ્ય પૂર્ણાંક sqlite3TestMakePointerStr(Tcl_Interp*, અક્ષર *zPtr, શૂન્ય*);
બાહ્ય અચલ અક્ષર *sqlite3ErrName(પૂર્ણાંક);

/*
**    sqlite3_intarray_create  DB  NAME
**
** Invoke the sqlite3_intarray_create interface.  A string that becomes
** the first parameter to sqlite3_intarray_bind.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_intarray_create(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  અચલ અક્ષર *zName;
  sqlite3_intarray *pArray;
  પૂર્ણાંક rc = SQLITE_OK;
  અક્ષર zPtr[100];

  જો( objc!=3 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "DB");
    પાછા TCL_ERROR;
  }
  જો( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) પાછા TCL_ERROR;
  zName = Tcl_GetString(objv[2]);
#ifndef SQLITE_OMIT_VIRTUALTABLE
  rc = sqlite3_intarray_create(db, zName, &pArray);
#endif
  જો( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), (અક્ષર*)0);
    પાછા TCL_ERROR;
  }
  sqlite3TestMakePointerStr(interp, zPtr, pArray);
  Tcl_AppendResult(interp, zPtr, (અક્ષર*)0);
  પાછા TCL_OK;
}

/*
**    sqlite3_intarray_bind  INTARRAY  ?VALUE ...?
**
** Invoke the sqlite3_intarray_bind interface on the given array of integers.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_intarray_bind(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_intarray *pArray;
  પૂર્ણાંક rc = SQLITE_OK;
  પૂર્ણાંક i, n;
  sqlite3_int64 *a;

  જો( objc<2 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "INTARRAY");
    પાછા TCL_ERROR;
  }
  pArray = (sqlite3_intarray*)sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  n = objc - 2;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  a = sqlite3_malloc64( કદ(a[0])*n );
  જો( a==0 ){
    Tcl_AppendResult(interp, "SQLITE_NOMEM", (અક્ષર*)0);
    પાછા TCL_ERROR;
  }
  માટે(i=0; i<n; i++){
    Tcl_WideInt x = 0;
    Tcl_GetWideIntFromObj(0, objv[i+2], &x);
    a[i] = x;
  }
  rc = sqlite3_intarray_bind(pArray, n, a, sqlite3_free);
  જો( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), (અક્ષર*)0);
    પાછા TCL_ERROR;
  }
#endif
  પાછા TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
પૂર્ણાંક Sqlitetestintarray_Init(Tcl_Interp *interp){
  સ્થિર સંરચના {
     અક્ષર *zName;
     Tcl_ObjCmdProc *xProc;
     શૂન્ય *clientData;
  } aObjCmd[] = {
     { "sqlite3_intarray_create", test_intarray_create, 0 },
     { "sqlite3_intarray_bind", test_intarray_bind, 0 },
  };
  પૂર્ણાંક i;
  માટે(i=0; i<કદ(aObjCmd)/કદ(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
  પાછા TCL_OK;
}

#endif /* SQLITE_TEST */
