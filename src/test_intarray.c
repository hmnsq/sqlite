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
#ಅನ್ತರ್ಭಾವ "test_intarray.h"
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <assert.h>


/*
** Definition of the sqlite3_intarray object.
**
** The internal representation of an intarray object is subject
** to change, is not externally visible, and should be used by
** the implementation of intarray only.  This object is opaque
** to users.
*/
ಸಂರಚನಾ sqlite3_intarray {
  ಪೂರ್ಣಾಙ್ಕ n;                    /* Number of elements in the array */
  sqlite3_int64 *a;         /* Contents of the array */
  ಶೂನ್ಯ (*xFree)(ಶೂನ್ಯ*);     /* Function used to free a[] */
};

/* Objects used internally by the virtual table implementation */
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ intarray_vtab intarray_vtab;
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ intarray_cursor intarray_cursor;

/* An intarray table object */
ಸಂರಚನಾ intarray_vtab {
  sqlite3_vtab base;            /* Base class */
  sqlite3_intarray *pContent;   /* Content of the integer array */
};

/* An intarray cursor object */
ಸಂರಚನಾ intarray_cursor {
  sqlite3_vtab_cursor base;    /* Base class */
  ಪೂರ್ಣಾಙ್ಕ i;                       /* Current cursor position */
};

/*
** None of this works unless we have virtual tables.
*/
#ifndef SQLITE_OMIT_VIRTUALTABLE

/*
** Free an sqlite3_intarray object.
*/
ಸ್ಥಿರ ಶೂನ್ಯ intarrayFree(ಶೂನ್ಯ *pX){
  sqlite3_intarray *p = (sqlite3_intarray*)pX;
  ಯದಿ( p->xFree ){
    p->xFree(p->a);
  }
  sqlite3_free(p);
}

/*
** Table destructor for the intarray module.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ intarrayDestroy(sqlite3_vtab *p){
  intarray_vtab *pVtab = (intarray_vtab*)p;
  sqlite3_free(pVtab);
  ಪ್ರತ್ಯಾಗಮ 0;
}

/*
** Table constructor for the intarray module.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ intarrayCreate(
  sqlite3 *db,              /* Database where module is created */
  ಶೂನ್ಯ *pAux,               /* clientdata for the module */
  ಪೂರ್ಣಾಙ್ಕ argc,                 /* Number of arguments */
  ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ*argv,   /* Value for all arguments */
  sqlite3_vtab **ppVtab,    /* Write the new virtual table object here */
  ವರ್ಣ **pzErr              /* Put error message text here */
){
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_NOMEM;
  intarray_vtab *pVtab = sqlite3_malloc64(ಪರಿಮಾಣ(intarray_vtab));

  ಯದಿ( pVtab ){
    memset(pVtab, 0, ಪರಿಮಾಣ(intarray_vtab));
    pVtab->pContent = (sqlite3_intarray*)pAux;
    rc = sqlite3_declare_vtab(db, "CREATE TABLE x(value INTEGER PRIMARY KEY)");
  }
  *ppVtab = (sqlite3_vtab *)pVtab;
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Open a new cursor on the intarray table.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ intarrayOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_NOMEM;
  intarray_cursor *pCur;
  pCur = sqlite3_malloc64(ಪರಿಮಾಣ(intarray_cursor));
  ಯದಿ( pCur ){
    memset(pCur, 0, ಪರಿಮಾಣ(intarray_cursor));
    *ppCursor = (sqlite3_vtab_cursor *)pCur;
    rc = SQLITE_OK;
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Close a intarray table cursor.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ intarrayClose(sqlite3_vtab_cursor *cur){
  intarray_cursor *pCur = (intarray_cursor *)cur;
  sqlite3_free(pCur);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Retrieve a column of data.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ intarrayColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, ಪೂರ್ಣಾಙ್ಕ i){
  intarray_cursor *pCur = (intarray_cursor*)cur;
  intarray_vtab *pVtab = (intarray_vtab*)cur->pVtab;
  ಯದಿ( pCur->i>=0 && pCur->i<pVtab->pContent->n ){
    sqlite3_result_int64(ctx, pVtab->pContent->a[pCur->i]);
  }
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Retrieve the current rowid.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ intarrayRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  intarray_cursor *pCur = (intarray_cursor *)cur;
  *pRowid = pCur->i;
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ intarrayEof(sqlite3_vtab_cursor *cur){
  intarray_cursor *pCur = (intarray_cursor *)cur;
  intarray_vtab *pVtab = (intarray_vtab *)cur->pVtab;
  ಪ್ರತ್ಯಾಗಮ pCur->i>=pVtab->pContent->n;
}

/*
** Advance the cursor to the next row.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ intarrayNext(sqlite3_vtab_cursor *cur){
  intarray_cursor *pCur = (intarray_cursor *)cur;
  pCur->i++;
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Reset a intarray table cursor.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ intarrayFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  ಪೂರ್ಣಾಙ್ಕ idx_ಅಂಕೆ, ಧ್ರುವ ವರ್ಣ *idxStr,
  ಪೂರ್ಣಾಙ್ಕ argc, sqlite3_ಮೌಲ್ಯ **argv
){
  intarray_cursor *pCur = (intarray_cursor *)pVtabCursor;
  pCur->i = 0;
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Analyse the WHERE condition.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ intarrayBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** A virtual table module that merely echos method calls into TCL
** variables.
*/
ಸ್ಥಿರ sqlite3_module intarrayModule = {
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
SQLITE_API ಪೂರ್ಣಾಙ್ಕ sqlite3_intarray_create(
  sqlite3 *db,
  ಧ್ರುವ ವರ್ಣ *zName,
  sqlite3_intarray **ppReturn
){
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_intarray *p;

  *ppReturn = p = sqlite3_malloc64( ಪರಿಮಾಣ(*p) );
  ಯದಿ( p==0 ){
    ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
  }
  memset(p, 0, ಪರಿಮಾಣ(*p));
  rc = sqlite3_create_module_v2(db, zName, &intarrayModule, p,
                                (ಶೂನ್ಯ(*)(ಶೂನ್ಯ*))intarrayFree);
  ಯದಿ( rc==SQLITE_OK ){
    ವರ್ಣ *zSql;
    zSql = sqlite3_mprintf("CREATE VIRTUAL TABLE temp.%Q USING %Q",
                           zName, zName);
    rc = sqlite3_exec(db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
  }
#endif
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Bind a new array array of integers to a specific intarray object.
**
** The array of integers bound must be unchanged for the duration of
** any query against the corresponding virtual table.  If the integer
** array does change or is deallocated undefined behavior will result.
*/
SQLITE_API ಪೂರ್ಣಾಙ್ಕ sqlite3_intarray_bind(
  sqlite3_intarray *pIntArray,   /* The intarray object to bind to */
  ಪೂರ್ಣಾಙ್ಕ nElements,                 /* Number of elements in the intarray */
  sqlite3_int64 *aElements,      /* Content of the intarray */
  ಶೂನ್ಯ (*xFree)(ಶೂನ್ಯ*)           /* How to dispose of the intarray when done */
){
  ಯದಿ( pIntArray->xFree ){
    pIntArray->xFree(pIntArray->a);
  }
  pIntArray->n = nElements;
  pIntArray->a = aElements;
  pIntArray->xFree = xFree;
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}


/*****************************************************************************
** Everything below is interface for testing this module.
*/
#ifdef SQLITE_TEST
#ಅನ್ತರ್ಭಾವ "tclsqlite.h"

/*
** Routines to encode and decode pointers
*/
ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ getDbPointer(Tcl_Interp *interp, ಧ್ರುವ ವರ್ಣ *zA, sqlite3 **ppDb);
ಬಾಹ್ಯ ಶೂನ್ಯ *sqlite3TestTextToPtr(ಧ್ರುವ ವರ್ಣ*);
ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ sqlite3TestMakePointerStr(Tcl_Interp*, ವರ್ಣ *zPtr, ಶೂನ್ಯ*);
ಬಾಹ್ಯ ಧ್ರುವ ವರ್ಣ *sqlite3ErrName(ಪೂರ್ಣಾಙ್ಕ);

/*
**    sqlite3_intarray_create  DB  NAME
**
** Invoke the sqlite3_intarray_create interface.  A string that becomes
** the first parameter to sqlite3_intarray_bind.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_intarray_create(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  ಧ್ರುವ ವರ್ಣ *zName;
  sqlite3_intarray *pArray;
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;
  ವರ್ಣ zPtr[100];

  ಯದಿ( objc!=3 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "DB");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  zName = Tcl_GetString(objv[2]);
#ifndef SQLITE_OMIT_VIRTUALTABLE
  rc = sqlite3_intarray_create(db, zName, &pArray);
#endif
  ಯದಿ( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), (ವರ್ಣ*)0);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  sqlite3TestMakePointerStr(interp, zPtr, pArray);
  Tcl_AppendResult(interp, zPtr, (ವರ್ಣ*)0);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
**    sqlite3_intarray_bind  INTARRAY  ?VALUE ...?
**
** Invoke the sqlite3_intarray_bind interface on the given array of integers.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_intarray_bind(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_intarray *pArray;
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;
  ಪೂರ್ಣಾಙ್ಕ i, n;
  sqlite3_int64 *a;

  ಯದಿ( objc<2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "INTARRAY");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  pArray = (sqlite3_intarray*)sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  n = objc - 2;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  a = sqlite3_malloc64( ಪರಿಮಾಣ(a[0])*n );
  ಯದಿ( a==0 ){
    Tcl_AppendResult(interp, "SQLITE_NOMEM", (ವರ್ಣ*)0);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಪ್ರತಿ(i=0; i<n; i++){
    Tcl_WideInt x = 0;
    Tcl_GetWideIntFromObj(0, objv[i+2], &x);
    a[i] = x;
  }
  rc = sqlite3_intarray_bind(pArray, n, a, sqlite3_free);
  ಯದಿ( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), (ವರ್ಣ*)0);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
#endif
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
ಪೂರ್ಣಾಙ್ಕ Sqlitetestintarray_Init(Tcl_Interp *interp){
  ಸ್ಥಿರ ಸಂರಚನಾ {
     ವರ್ಣ *zName;
     Tcl_ObjCmdProc *xProc;
     ಶೂನ್ಯ *clientData;
  } aObjCmd[] = {
     { "sqlite3_intarray_create", test_intarray_create, 0 },
     { "sqlite3_intarray_bind", test_intarray_bind, 0 },
  };
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪ್ರತಿ(i=0; i<ಪರಿಮಾಣ(aObjCmd)/ಪರಿಮಾಣ(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

#endif /* SQLITE_TEST */
