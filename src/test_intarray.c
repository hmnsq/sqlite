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
ರಚನೆ sqlite3_intarray {
  ಪೂರ್ಣಾಂಕ n;                    /* Number of elements in the array */
  sqlite3_int64 *a;         /* Contents of the array */
  ಶೂನ್ಯ (*xFree)(ಶೂನ್ಯ*);     /* Function used to free a[] */
};

/* Objects used internally by the virtual table implementation */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ intarray_vtab intarray_vtab;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ intarray_cursor intarray_cursor;

/* An intarray table object */
ರಚನೆ intarray_vtab {
  sqlite3_vtab ತಳ;            /* Base class */
  sqlite3_intarray *pContent;   /* Content of the integer array */
};

/* An intarray cursor object */
ರಚನೆ intarray_cursor {
  sqlite3_vtab_cursor ತಳ;    /* Base class */
  ಪೂರ್ಣಾಂಕ i;                       /* Current cursor position */
};

/*
** None of this works unless we have virtual tables.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE

/*
** Free an sqlite3_intarray object.
*/
ಸ್ಥಿರ ಶೂನ್ಯ intarrayFree(ಶೂನ್ಯ *pX){
  sqlite3_intarray *p = (sqlite3_intarray*)pX;
  ಆದರೆ( p->xFree ){
    p->xFree(p->a);
  }
  sqlite3_free(p);
}

/*
** Table destructor for the intarray module.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ intarrayDestroy(sqlite3_vtab *p){
  intarray_vtab *pVtab = (intarray_vtab*)p;
  sqlite3_free(pVtab);
  ಹಿಂತಿರುಗಿ 0;
}

/*
** Table constructor for the intarray module.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ intarrayCreate(
  sqlite3 *db,              /* Database where module is created */
  ಶೂನ್ಯ *pAux,               /* clientdata for the module */
  ಪೂರ್ಣಾಂಕ argc,                 /* Number of arguments */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ*argv,   /* Value for all arguments */
  sqlite3_vtab **ppVtab,    /* Write the new virtual table object here */
  ಅಕ್ಷರ **pzErr              /* Put error message text here */
){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_NOMEM;
  intarray_vtab *pVtab = sqlite3_malloc64(ಗಾತ್ರ(intarray_vtab));

  ಆದರೆ( pVtab ){
    memset(pVtab, 0, ಗಾತ್ರ(intarray_vtab));
    pVtab->pContent = (sqlite3_intarray*)pAux;
    rc = sqlite3_declare_vtab(db, "CREATE TABLE x(value INTEGER PRIMARY KEY)");
  }
  *ppVtab = (sqlite3_vtab *)pVtab;
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Open a new cursor on the intarray table.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ intarrayOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_NOMEM;
  intarray_cursor *pCur;
  pCur = sqlite3_malloc64(ಗಾತ್ರ(intarray_cursor));
  ಆದರೆ( pCur ){
    memset(pCur, 0, ಗಾತ್ರ(intarray_cursor));
    *ppCursor = (sqlite3_vtab_cursor *)pCur;
    rc = SQLITE_OK;
  }
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Close a intarray table cursor.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ intarrayClose(sqlite3_vtab_cursor *cur){
  intarray_cursor *pCur = (intarray_cursor *)cur;
  sqlite3_free(pCur);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Retrieve a column of data.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ intarray_ಸ್ತಂಭ(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, ಪೂರ್ಣಾಂಕ i){
  intarray_cursor *pCur = (intarray_cursor*)cur;
  intarray_vtab *pVtab = (intarray_vtab*)cur->pVtab;
  ಆದರೆ( pCur->i>=0 && pCur->i<pVtab->pContent->n ){
    sqlite3_ಫಲಿತಾಂಶ_int64(ctx, pVtab->pContent->a[pCur->i]);
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Retrieve the current rowid.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ intarrayRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  intarray_cursor *pCur = (intarray_cursor *)cur;
  *pRowid = pCur->i;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ intarrayEof(sqlite3_vtab_cursor *cur){
  intarray_cursor *pCur = (intarray_cursor *)cur;
  intarray_vtab *pVtab = (intarray_vtab *)cur->pVtab;
  ಹಿಂತಿರುಗಿ pCur->i>=pVtab->pContent->n;
}

/*
** Advance the cursor to the next row.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ intarray_ಮುಂದಿನ(sqlite3_vtab_cursor *cur){
  intarray_cursor *pCur = (intarray_cursor *)cur;
  pCur->i++;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Reset a intarray table cursor.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ intarrayFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  ಪೂರ್ಣಾಂಕ idx_ಅಂಕೆ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *idxStr,
  ಪೂರ್ಣಾಂಕ argc, sqlite3_ಮೌಲ್ಯ **argv
){
  intarray_cursor *pCur = (intarray_cursor *)pVtabCursor;
  pCur->i = 0;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Analyse the WHERE condition.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ intarray_ಉತ್ತಮ_ಸೂಚ್ಯಂಕ(sqlite3_vtab *tab, sqlite3_ಸೂಚ್ಯಂಕ_info *pIdxInfo){
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** A virtual table module that merely echos method calls into TCL
** variables.
*/
ಸ್ಥಿರ sqlite3_module intarrayModule = {
  0,                           /* iVersion */
  intarrayCreate,              /* xCreate - create a new virtual table */
  intarrayCreate,              /* xConnect - connect to an existing vtab */
  intarray_ಉತ್ತಮ_ಸೂಚ್ಯಂಕ,           /* xBestIndex - find the best query index */
  intarrayDestroy,             /* xDisconnect - disconnect a vtab */
  intarrayDestroy,             /* xDestroy - destroy a vtab */
  intarrayOpen,                /* xOpen - open a cursor */
  intarrayClose,               /* xClose - close a cursor */
  intarrayFilter,              /* xFilter - configure scan constraints */
  intarray_ಮುಂದಿನ,                /* xNext - advance a cursor */
  intarrayEof,                 /* xEof */
  intarray_ಸ್ತಂಭ,              /* xColumn - read data */
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

#ಆದರೆ_ಅಂತ್ಯ /* !defined(SQLITE_OMIT_VIRTUALTABLE) */

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
SQLITE_API ಪೂರ್ಣಾಂಕ sqlite3_intarray_create(
  sqlite3 *db,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName,
  sqlite3_intarray **ppReturn
){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
  sqlite3_intarray *p;

  *ppReturn = p = sqlite3_malloc64( ಗಾತ್ರ(*p) );
  ಆದರೆ( p==0 ){
    ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
  }
  memset(p, 0, ಗಾತ್ರ(*p));
  rc = sqlite3_create_module_v2(db, zName, &intarrayModule, p,
                                (ಶೂನ್ಯ(*)(ಶೂನ್ಯ*))intarrayFree);
  ಆದರೆ( rc==SQLITE_OK ){
    ಅಕ್ಷರ *zSql;
    zSql = sqlite3_mprintf("CREATE VIRTUAL TABLE temp.%Q USING %Q",
                           zName, zName);
    rc = sqlite3_exec(db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Bind a new array array of integers to a specific intarray object.
**
** The array of integers bound must be unchanged for the duration of
** any query against the corresponding virtual table.  If the integer
** array does change or is deallocated undefined behavior will result.
*/
SQLITE_API ಪೂರ್ಣಾಂಕ sqlite3_intarray_bind(
  sqlite3_intarray *pIntArray,   /* The intarray object to bind to */
  ಪೂರ್ಣಾಂಕ nElements,                 /* Number of elements in the intarray */
  sqlite3_int64 *aElements,      /* Content of the intarray */
  ಶೂನ್ಯ (*xFree)(ಶೂನ್ಯ*)           /* How to dispose of the intarray when done */
){
  ಆದರೆ( pIntArray->xFree ){
    pIntArray->xFree(pIntArray->a);
  }
  pIntArray->n = nElements;
  pIntArray->a = aElements;
  pIntArray->xFree = xFree;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}


/*****************************************************************************
** Everything below is interface for testing this module.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_TEST
#ಅನ್ತರ್ಭಾವ "tclsqlite.h"

/*
** Routines to encode and decode pointers
*/
ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ get_Db_ಸೂಚಕ(Tcl_Interp *interp, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zA, sqlite3 **ppDb);
ಬಾಹ್ಯ ಶೂನ್ಯ *sqlite3_Test_ಪಠ್ಯ_ಗಮ್ಯ_Ptr(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ sqlite3_Test_Make_ಸೂಚಕ_Str(Tcl_Interp*, ಅಕ್ಷರ *zPtr, ಶೂನ್ಯ*);
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3ErrName(ಪೂರ್ಣಾಂಕ);

/*
**    sqlite3_intarray_create  DB  NAME
**
** Invoke the sqlite3_intarray_create interface.  A string that becomes
** the first parameter to sqlite3_intarray_bind.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_intarray_create(
  Client_ದತ್ತಾಂಶ client_ದತ್ತಾಂಶ, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName;
  sqlite3_intarray *pArray;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  ಅಕ್ಷರ zPtr[100];

  ಆದರೆ( objc!=3 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "DB");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( get_Db_ಸೂಚಕ(interp, Tcl_GetString(objv[1]), &db) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  zName = Tcl_GetString(objv[2]);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
  rc = sqlite3_intarray_create(db, zName, &pArray);
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ( rc!=SQLITE_OK ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, sqlite3ErrName(rc), (ಅಕ್ಷರ*)0);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  sqlite3_Test_Make_ಸೂಚಕ_Str(interp, zPtr, pArray);
  Tcl_Append_ಫಲಿತಾಂಶ(interp, zPtr, (ಅಕ್ಷರ*)0);
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
**    sqlite3_intarray_bind  INTARRAY  ?VALUE ...?
**
** Invoke the sqlite3_intarray_bind interface on the given array of integers.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_intarray_bind(
  Client_ದತ್ತಾಂಶ client_ದತ್ತಾಂಶ, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_intarray *pArray;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  ಪೂರ್ಣಾಂಕ i, n;
  sqlite3_int64 *a;

  ಆದರೆ( objc<2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "INTARRAY");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  pArray = (sqlite3_intarray*)sqlite3_Test_ಪಠ್ಯ_ಗಮ್ಯ_Ptr(Tcl_GetString(objv[1]));
  n = objc - 2;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
  a = sqlite3_malloc64( ಗಾತ್ರ(a[0])*n );
  ಆದರೆ( a==0 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "SQLITE_NOMEM", (ಅಕ್ಷರ*)0);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಗಾಗಿ(i=0; i<n; i++){
    Tcl_WideInt x = 0;
    Tcl_GetWideIntFromObj(0, objv[i+2], &x);
    a[i] = x;
  }
  rc = sqlite3_intarray_bind(pArray, n, a, sqlite3_free);
  ಆದರೆ( rc!=SQLITE_OK ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, sqlite3ErrName(rc), (ಅಕ್ಷರ*)0);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
ಪೂರ್ಣಾಂಕ Sqlitetestintarray_Init(Tcl_Interp *interp){
  ಸ್ಥಿರ ರಚನೆ {
     ಅಕ್ಷರ *zName;
     Tcl_ObjCmdProc *xProc;
     ಶೂನ್ಯ *client_ದತ್ತಾಂಶ;
  } aObjCmd[] = {
     { "sqlite3_intarray_create", test_intarray_create, 0 },
     { "sqlite3_intarray_bind", test_intarray_bind, 0 },
  };
  ಪೂರ್ಣಾಂಕ i;
  ಗಾಗಿ(i=0; i<ಗಾತ್ರ(aObjCmd)/ಗಾತ್ರ(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].client_ದತ್ತಾಂಶ, 0);
  }
  ಹಿಂತಿರುಗಿ TCL_OK;
}

#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_TEST */
