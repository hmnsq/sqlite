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
#अन्तर्भाव "test_intarray.h"
#अन्तर्भाव <string.h>
#अन्तर्भाव <assert.h>


/*
** Definition of the sqlite3_intarray object.
**
** The internal representation of an intarray object is subject
** to change, is not externally visible, and should be used by
** the implementation of intarray only.  This object is opaque
** to users.
*/
संरचना sqlite3_intarray {
  पूर्णाङ्क n;                    /* Number of elements in the array */
  sqlite3_int64 *a;         /* Contents of the array */
  शून्य (*xFree)(शून्य*);     /* Function used to free a[] */
};

/* Objects used internally by the virtual table implementation */
प्रकारनिर्देश संरचना intarray_vtab intarray_vtab;
प्रकारनिर्देश संरचना intarray_cursor intarray_cursor;

/* An intarray table object */
संरचना intarray_vtab {
  sqlite3_vtab base;            /* Base class */
  sqlite3_intarray *pContent;   /* Content of the integer array */
};

/* An intarray cursor object */
संरचना intarray_cursor {
  sqlite3_vtab_cursor base;    /* Base class */
  पूर्णाङ्क i;                       /* Current cursor position */
};

/*
** None of this works unless we have virtual tables.
*/
#ifndef SQLITE_OMIT_VIRTUALTABLE

/*
** Free an sqlite3_intarray object.
*/
स्थिर शून्य intarrayFree(शून्य *pX){
  sqlite3_intarray *p = (sqlite3_intarray*)pX;
  यदि( p->xFree ){
    p->xFree(p->a);
  }
  sqlite3_free(p);
}

/*
** Table destructor for the intarray module.
*/
स्थिर पूर्णाङ्क intarrayDestroy(sqlite3_vtab *p){
  intarray_vtab *pVtab = (intarray_vtab*)p;
  sqlite3_free(pVtab);
  फर्काऊ 0;
}

/*
** Table constructor for the intarray module.
*/
स्थिर पूर्णाङ्क intarrayCreate(
  sqlite3 *db,              /* Database where module is created */
  शून्य *pAux,               /* clientdata for the module */
  पूर्णाङ्क argc,                 /* Number of arguments */
  अचर वर्ण *अचर*argv,   /* Value for all arguments */
  sqlite3_vtab **ppVtab,    /* Write the new virtual table object here */
  वर्ण **pzErr              /* Put error message text here */
){
  पूर्णाङ्क rc = SQLITE_NOMEM;
  intarray_vtab *pVtab = sqlite3_malloc64(परिमाण(intarray_vtab));

  यदि( pVtab ){
    स_रखो(pVtab, 0, परिमाण(intarray_vtab));
    pVtab->pContent = (sqlite3_intarray*)pAux;
    rc = sqlite3_declare_vtab(db, "CREATE TABLE x(value INTEGER PRIMARY KEY)");
  }
  *ppVtab = (sqlite3_vtab *)pVtab;
  फर्काऊ rc;
}

/*
** Open a new cursor on the intarray table.
*/
स्थिर पूर्णाङ्क intarrayOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  पूर्णाङ्क rc = SQLITE_NOMEM;
  intarray_cursor *pCur;
  pCur = sqlite3_malloc64(परिमाण(intarray_cursor));
  यदि( pCur ){
    स_रखो(pCur, 0, परिमाण(intarray_cursor));
    *ppCursor = (sqlite3_vtab_cursor *)pCur;
    rc = SQLITE_OK;
  }
  फर्काऊ rc;
}

/*
** Close a intarray table cursor.
*/
स्थिर पूर्णाङ्क intarrayClose(sqlite3_vtab_cursor *cur){
  intarray_cursor *pCur = (intarray_cursor *)cur;
  sqlite3_free(pCur);
  फर्काऊ SQLITE_OK;
}

/*
** Retrieve a column of data.
*/
स्थिर पूर्णाङ्क intarrayColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, पूर्णाङ्क i){
  intarray_cursor *pCur = (intarray_cursor*)cur;
  intarray_vtab *pVtab = (intarray_vtab*)cur->pVtab;
  यदि( pCur->i>=0 && pCur->i<pVtab->pContent->n ){
    sqlite3_result_int64(ctx, pVtab->pContent->a[pCur->i]);
  }
  फर्काऊ SQLITE_OK;
}

/*
** Retrieve the current rowid.
*/
स्थिर पूर्णाङ्क intarrayRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  intarray_cursor *pCur = (intarray_cursor *)cur;
  *pRowid = pCur->i;
  फर्काऊ SQLITE_OK;
}

स्थिर पूर्णाङ्क intarrayEof(sqlite3_vtab_cursor *cur){
  intarray_cursor *pCur = (intarray_cursor *)cur;
  intarray_vtab *pVtab = (intarray_vtab *)cur->pVtab;
  फर्काऊ pCur->i>=pVtab->pContent->n;
}

/*
** Advance the cursor to the next row.
*/
स्थिर पूर्णाङ्क intarrayNext(sqlite3_vtab_cursor *cur){
  intarray_cursor *pCur = (intarray_cursor *)cur;
  pCur->i++;
  फर्काऊ SQLITE_OK;
}

/*
** Reset a intarray table cursor.
*/
स्थिर पूर्णाङ्क intarrayFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  पूर्णाङ्क idx_अङ्क, अचर वर्ण *idxStr,
  पूर्णाङ्क argc, sqlite3_मान **argv
){
  intarray_cursor *pCur = (intarray_cursor *)pVtabCursor;
  pCur->i = 0;
  फर्काऊ SQLITE_OK;
}

/*
** Analyse the WHERE condition.
*/
स्थिर पूर्णाङ्क intarrayBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  फर्काऊ SQLITE_OK;
}

/*
** A virtual table module that merely echos method calls into TCL
** variables.
*/
स्थिर sqlite3_module intarrayModule = {
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
SQLITE_API पूर्णाङ्क sqlite3_intarray_create(
  sqlite3 *db,
  अचर वर्ण *zName,
  sqlite3_intarray **ppReturn
){
  पूर्णाङ्क rc = SQLITE_OK;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_intarray *p;

  *ppReturn = p = sqlite3_malloc64( परिमाण(*p) );
  यदि( p==0 ){
    फर्काऊ SQLITE_NOMEM;
  }
  स_रखो(p, 0, परिमाण(*p));
  rc = sqlite3_create_module_v2(db, zName, &intarrayModule, p,
                                (शून्य(*)(शून्य*))intarrayFree);
  यदि( rc==SQLITE_OK ){
    वर्ण *zSql;
    zSql = sqlite3_mprintf("CREATE VIRTUAL TABLE temp.%Q USING %Q",
                           zName, zName);
    rc = sqlite3_exec(db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
  }
#endif
  फर्काऊ rc;
}

/*
** Bind a new array array of integers to a specific intarray object.
**
** The array of integers bound must be unchanged for the duration of
** any query against the corresponding virtual table.  If the integer
** array does change or is deallocated undefined behavior will result.
*/
SQLITE_API पूर्णाङ्क sqlite3_intarray_bind(
  sqlite3_intarray *pIntArray,   /* The intarray object to bind to */
  पूर्णाङ्क nElements,                 /* Number of elements in the intarray */
  sqlite3_int64 *aElements,      /* Content of the intarray */
  शून्य (*xFree)(शून्य*)           /* How to dispose of the intarray when done */
){
  यदि( pIntArray->xFree ){
    pIntArray->xFree(pIntArray->a);
  }
  pIntArray->n = nElements;
  pIntArray->a = aElements;
  pIntArray->xFree = xFree;
  फर्काऊ SQLITE_OK;
}


/*****************************************************************************
** Everything below is interface for testing this module.
*/
#ifdef SQLITE_TEST
#अन्तर्भाव "tclsqlite.h"

/*
** Routines to encode and decode pointers
*/
बाह्य पूर्णाङ्क getDbPointer(Tcl_Interp *interp, अचर वर्ण *zA, sqlite3 **ppDb);
बाह्य शून्य *sqlite3TestTextToPtr(अचर वर्ण*);
बाह्य पूर्णाङ्क sqlite3TestMakePointerStr(Tcl_Interp*, वर्ण *zPtr, शून्य*);
बाह्य अचर वर्ण *sqlite3ErrName(पूर्णाङ्क);

/*
**    sqlite3_intarray_create  DB  NAME
**
** Invoke the sqlite3_intarray_create interface.  A string that becomes
** the first parameter to sqlite3_intarray_bind.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_intarray_create(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  अचर वर्ण *zName;
  sqlite3_intarray *pArray;
  पूर्णाङ्क rc = SQLITE_OK;
  वर्ण zPtr[100];

  यदि( objc!=3 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "DB");
    फर्काऊ TCL_ERROR;
  }
  यदि( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) फर्काऊ TCL_ERROR;
  zName = Tcl_GetString(objv[2]);
#ifndef SQLITE_OMIT_VIRTUALTABLE
  rc = sqlite3_intarray_create(db, zName, &pArray);
#endif
  यदि( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), (वर्ण*)0);
    फर्काऊ TCL_ERROR;
  }
  sqlite3TestMakePointerStr(interp, zPtr, pArray);
  Tcl_AppendResult(interp, zPtr, (वर्ण*)0);
  फर्काऊ TCL_OK;
}

/*
**    sqlite3_intarray_bind  INTARRAY  ?VALUE ...?
**
** Invoke the sqlite3_intarray_bind interface on the given array of integers.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_intarray_bind(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_intarray *pArray;
  पूर्णाङ्क rc = SQLITE_OK;
  पूर्णाङ्क i, n;
  sqlite3_int64 *a;

  यदि( objc<2 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "INTARRAY");
    फर्काऊ TCL_ERROR;
  }
  pArray = (sqlite3_intarray*)sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  n = objc - 2;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  a = sqlite3_malloc64( परिमाण(a[0])*n );
  यदि( a==0 ){
    Tcl_AppendResult(interp, "SQLITE_NOMEM", (वर्ण*)0);
    फर्काऊ TCL_ERROR;
  }
  लागि(i=0; i<n; i++){
    Tcl_WideInt x = 0;
    Tcl_GetWideIntFromObj(0, objv[i+2], &x);
    a[i] = x;
  }
  rc = sqlite3_intarray_bind(pArray, n, a, sqlite3_free);
  यदि( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), (वर्ण*)0);
    फर्काऊ TCL_ERROR;
  }
#endif
  फर्काऊ TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
पूर्णाङ्क Sqlitetestintarray_Init(Tcl_Interp *interp){
  स्थिर संरचना {
     वर्ण *zName;
     Tcl_ObjCmdProc *xProc;
     शून्य *clientData;
  } aObjCmd[] = {
     { "sqlite3_intarray_create", test_intarray_create, 0 },
     { "sqlite3_intarray_bind", test_intarray_bind, 0 },
  };
  पूर्णाङ्क i;
  लागि(i=0; i<परिमाण(aObjCmd)/परिमाण(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
  फर्काऊ TCL_OK;
}

#endif /* SQLITE_TEST */
