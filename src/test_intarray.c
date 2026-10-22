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
#অন্তর্ভাব "test_intarray.h"
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <assert.h>


/*
** Definition of the sqlite3_intarray object.
**
** The internal representation of an intarray object is subject
** to change, is not externally visible, and should be used by
** the implementation of intarray only.  This object is opaque
** to users.
*/
সংরচনা sqlite3_intarray {
  পূর্ণাঙ্ক n;                    /* Number of elements in the array */
  sqlite3_int64 *a;         /* Contents of the array */
  শূন্য (*xFree)(শূন্য*);     /* Function used to free a[] */
};

/* Objects used internally by the virtual table implementation */
প্রকারনির্দেশ সংরচনা intarray_vtab intarray_vtab;
প্রকারনির্দেশ সংরচনা intarray_cursor intarray_cursor;

/* An intarray table object */
সংরচনা intarray_vtab {
  sqlite3_vtab base;            /* Base class */
  sqlite3_intarray *pContent;   /* Content of the integer array */
};

/* An intarray cursor object */
সংরচনা intarray_cursor {
  sqlite3_vtab_cursor base;    /* Base class */
  পূর্ণাঙ্ক i;                       /* Current cursor position */
};

/*
** None of this works unless we have virtual tables.
*/
#ifndef SQLITE_OMIT_VIRTUALTABLE

/*
** Free an sqlite3_intarray object.
*/
স্থির শূন্য intarrayFree(শূন্য *pX){
  sqlite3_intarray *p = (sqlite3_intarray*)pX;
  যদি( p->xFree ){
    p->xFree(p->a);
  }
  sqlite3_free(p);
}

/*
** Table destructor for the intarray module.
*/
স্থির পূর্ণাঙ্ক intarrayDestroy(sqlite3_vtab *p){
  intarray_vtab *pVtab = (intarray_vtab*)p;
  sqlite3_free(pVtab);
  প্রত্যাগম 0;
}

/*
** Table constructor for the intarray module.
*/
স্থির পূর্ণাঙ্ক intarrayCreate(
  sqlite3 *db,              /* Database where module is created */
  শূন্য *pAux,               /* clientdata for the module */
  পূর্ণাঙ্ক argc,                 /* Number of arguments */
  ধ্রুব বর্ণ *ধ্রুব*argv,   /* Value for all arguments */
  sqlite3_vtab **ppVtab,    /* Write the new virtual table object here */
  বর্ণ **pzErr              /* Put error message text here */
){
  পূর্ণাঙ্ক rc = SQLITE_NOMEM;
  intarray_vtab *pVtab = sqlite3_malloc64(পরিমাণ(intarray_vtab));

  যদি( pVtab ){
    memset(pVtab, 0, পরিমাণ(intarray_vtab));
    pVtab->pContent = (sqlite3_intarray*)pAux;
    rc = sqlite3_declare_vtab(db, "CREATE TABLE x(value INTEGER PRIMARY KEY)");
  }
  *ppVtab = (sqlite3_vtab *)pVtab;
  প্রত্যাগম rc;
}

/*
** Open a new cursor on the intarray table.
*/
স্থির পূর্ণাঙ্ক intarrayOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  পূর্ণাঙ্ক rc = SQLITE_NOMEM;
  intarray_cursor *pCur;
  pCur = sqlite3_malloc64(পরিমাণ(intarray_cursor));
  যদি( pCur ){
    memset(pCur, 0, পরিমাণ(intarray_cursor));
    *ppCursor = (sqlite3_vtab_cursor *)pCur;
    rc = SQLITE_OK;
  }
  প্রত্যাগম rc;
}

/*
** Close a intarray table cursor.
*/
স্থির পূর্ণাঙ্ক intarrayClose(sqlite3_vtab_cursor *cur){
  intarray_cursor *pCur = (intarray_cursor *)cur;
  sqlite3_free(pCur);
  প্রত্যাগম SQLITE_OK;
}

/*
** Retrieve a column of data.
*/
স্থির পূর্ণাঙ্ক intarrayColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, পূর্ণাঙ্ক i){
  intarray_cursor *pCur = (intarray_cursor*)cur;
  intarray_vtab *pVtab = (intarray_vtab*)cur->pVtab;
  যদি( pCur->i>=0 && pCur->i<pVtab->pContent->n ){
    sqlite3_result_int64(ctx, pVtab->pContent->a[pCur->i]);
  }
  প্রত্যাগম SQLITE_OK;
}

/*
** Retrieve the current rowid.
*/
স্থির পূর্ণাঙ্ক intarrayRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  intarray_cursor *pCur = (intarray_cursor *)cur;
  *pRowid = pCur->i;
  প্রত্যাগম SQLITE_OK;
}

স্থির পূর্ণাঙ্ক intarrayEof(sqlite3_vtab_cursor *cur){
  intarray_cursor *pCur = (intarray_cursor *)cur;
  intarray_vtab *pVtab = (intarray_vtab *)cur->pVtab;
  প্রত্যাগম pCur->i>=pVtab->pContent->n;
}

/*
** Advance the cursor to the next row.
*/
স্থির পূর্ণাঙ্ক intarrayNext(sqlite3_vtab_cursor *cur){
  intarray_cursor *pCur = (intarray_cursor *)cur;
  pCur->i++;
  প্রত্যাগম SQLITE_OK;
}

/*
** Reset a intarray table cursor.
*/
স্থির পূর্ণাঙ্ক intarrayFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  পূর্ণাঙ্ক idx_অঙ্ক, ধ্রুব বর্ণ *idxStr,
  পূর্ণাঙ্ক argc, sqlite3_মান **argv
){
  intarray_cursor *pCur = (intarray_cursor *)pVtabCursor;
  pCur->i = 0;
  প্রত্যাগম SQLITE_OK;
}

/*
** Analyse the WHERE condition.
*/
স্থির পূর্ণাঙ্ক intarrayBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  প্রত্যাগম SQLITE_OK;
}

/*
** A virtual table module that merely echos method calls into TCL
** variables.
*/
স্থির sqlite3_module intarrayModule = {
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
SQLITE_API পূর্ণাঙ্ক sqlite3_intarray_create(
  sqlite3 *db,
  ধ্রুব বর্ণ *zName,
  sqlite3_intarray **ppReturn
){
  পূর্ণাঙ্ক rc = SQLITE_OK;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_intarray *p;

  *ppReturn = p = sqlite3_malloc64( পরিমাণ(*p) );
  যদি( p==0 ){
    প্রত্যাগম SQLITE_NOMEM;
  }
  memset(p, 0, পরিমাণ(*p));
  rc = sqlite3_create_module_v2(db, zName, &intarrayModule, p,
                                (শূন্য(*)(শূন্য*))intarrayFree);
  যদি( rc==SQLITE_OK ){
    বর্ণ *zSql;
    zSql = sqlite3_mprintf("CREATE VIRTUAL TABLE temp.%Q USING %Q",
                           zName, zName);
    rc = sqlite3_exec(db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
  }
#endif
  প্রত্যাগম rc;
}

/*
** Bind a new array array of integers to a specific intarray object.
**
** The array of integers bound must be unchanged for the duration of
** any query against the corresponding virtual table.  If the integer
** array does change or is deallocated undefined behavior will result.
*/
SQLITE_API পূর্ণাঙ্ক sqlite3_intarray_bind(
  sqlite3_intarray *pIntArray,   /* The intarray object to bind to */
  পূর্ণাঙ্ক nElements,                 /* Number of elements in the intarray */
  sqlite3_int64 *aElements,      /* Content of the intarray */
  শূন্য (*xFree)(শূন্য*)           /* How to dispose of the intarray when done */
){
  যদি( pIntArray->xFree ){
    pIntArray->xFree(pIntArray->a);
  }
  pIntArray->n = nElements;
  pIntArray->a = aElements;
  pIntArray->xFree = xFree;
  প্রত্যাগম SQLITE_OK;
}


/*****************************************************************************
** Everything below is interface for testing this module.
*/
#ifdef SQLITE_TEST
#অন্তর্ভাব "tclsqlite.h"

/*
** Routines to encode and decode pointers
*/
বাহ্য পূর্ণাঙ্ক getDbPointer(Tcl_Interp *interp, ধ্রুব বর্ণ *zA, sqlite3 **ppDb);
বাহ্য শূন্য *sqlite3TestTextToPtr(ধ্রুব বর্ণ*);
বাহ্য পূর্ণাঙ্ক sqlite3TestMakePointerStr(Tcl_Interp*, বর্ণ *zPtr, শূন্য*);
বাহ্য ধ্রুব বর্ণ *sqlite3ErrName(পূর্ণাঙ্ক);

/*
**    sqlite3_intarray_create  DB  NAME
**
** Invoke the sqlite3_intarray_create interface.  A string that becomes
** the first parameter to sqlite3_intarray_bind.
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_intarray_create(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণাঙ্ক objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  ধ্রুব বর্ণ *zName;
  sqlite3_intarray *pArray;
  পূর্ণাঙ্ক rc = SQLITE_OK;
  বর্ণ zPtr[100];

  যদি( objc!=3 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "DB");
    প্রত্যাগম TCL_ERROR;
  }
  যদি( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) প্রত্যাগম TCL_ERROR;
  zName = Tcl_GetString(objv[2]);
#ifndef SQLITE_OMIT_VIRTUALTABLE
  rc = sqlite3_intarray_create(db, zName, &pArray);
#endif
  যদি( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), (বর্ণ*)0);
    প্রত্যাগম TCL_ERROR;
  }
  sqlite3TestMakePointerStr(interp, zPtr, pArray);
  Tcl_AppendResult(interp, zPtr, (বর্ণ*)0);
  প্রত্যাগম TCL_OK;
}

/*
**    sqlite3_intarray_bind  INTARRAY  ?VALUE ...?
**
** Invoke the sqlite3_intarray_bind interface on the given array of integers.
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_intarray_bind(
  ClientData clientData, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণাঙ্ক objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_intarray *pArray;
  পূর্ণাঙ্ক rc = SQLITE_OK;
  পূর্ণাঙ্ক i, n;
  sqlite3_int64 *a;

  যদি( objc<2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "INTARRAY");
    প্রত্যাগম TCL_ERROR;
  }
  pArray = (sqlite3_intarray*)sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  n = objc - 2;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  a = sqlite3_malloc64( পরিমাণ(a[0])*n );
  যদি( a==0 ){
    Tcl_AppendResult(interp, "SQLITE_NOMEM", (বর্ণ*)0);
    প্রত্যাগম TCL_ERROR;
  }
  প্রতি(i=0; i<n; i++){
    Tcl_WideInt x = 0;
    Tcl_GetWideIntFromObj(0, objv[i+2], &x);
    a[i] = x;
  }
  rc = sqlite3_intarray_bind(pArray, n, a, sqlite3_free);
  যদি( rc!=SQLITE_OK ){
    Tcl_AppendResult(interp, sqlite3ErrName(rc), (বর্ণ*)0);
    প্রত্যাগম TCL_ERROR;
  }
#endif
  প্রত্যাগম TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
পূর্ণাঙ্ক Sqlitetestintarray_Init(Tcl_Interp *interp){
  স্থির সংরচনা {
     বর্ণ *zName;
     Tcl_ObjCmdProc *xProc;
     শূন্য *clientData;
  } aObjCmd[] = {
     { "sqlite3_intarray_create", test_intarray_create, 0 },
     { "sqlite3_intarray_bind", test_intarray_bind, 0 },
  };
  পূর্ণাঙ্ক i;
  প্রতি(i=0; i<পরিমাণ(aObjCmd)/পরিমাণ(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
  প্রত্যাগম TCL_OK;
}

#endif /* SQLITE_TEST */
