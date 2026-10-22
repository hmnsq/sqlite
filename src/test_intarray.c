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
কাঠামো sqlite3_intarray {
  পূর্ণসংখ্যা n;                    /* Number of elements in the array */
  sqlite3_int64 *a;         /* Contents of the array */
  শূন্য (*xFree)(শূন্য*);     /* Function used to free a[] */
};

/* Objects used internally by the virtual table implementation */
প্রকারলেখ কাঠামো intarray_vtab intarray_vtab;
প্রকারলেখ কাঠামো intarray_cursor intarray_cursor;

/* An intarray table object */
কাঠামো intarray_vtab {
  sqlite3_vtab base;            /* Base class */
  sqlite3_intarray *pContent;   /* Content of the integer array */
};

/* An intarray cursor object */
কাঠামো intarray_cursor {
  sqlite3_vtab_cursor base;    /* Base class */
  পূর্ণসংখ্যা i;                       /* Current cursor position */
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
স্থির পূর্ণসংখ্যা intarrayDestroy(sqlite3_vtab *p){
  intarray_vtab *pVtab = (intarray_vtab*)p;
  sqlite3_free(pVtab);
  ফেরত 0;
}

/*
** Table constructor for the intarray module.
*/
স্থির পূর্ণসংখ্যা intarrayCreate(
  sqlite3 *db,              /* Database where module is created */
  শূন্য *pAux,               /* clientdata for the module */
  পূর্ণসংখ্যা argc,                 /* Number of arguments */
  ধ্রুবক অক্ষর *ধ্রুবক*argv,   /* Value for all arguments */
  sqlite3_vtab **ppVtab,    /* Write the new virtual table object here */
  অক্ষর **pzErr              /* Put error message text here */
){
  পূর্ণসংখ্যা rc = SQLITE_NOMEM;
  intarray_vtab *pVtab = sqlite3_malloc64(আকার(intarray_vtab));

  যদি( pVtab ){
    memset(pVtab, 0, আকার(intarray_vtab));
    pVtab->pContent = (sqlite3_intarray*)pAux;
    rc = sqlite3_declare_vtab(db, "CREATE TABLE x(value INTEGER PRIMARY KEY)");
  }
  *ppVtab = (sqlite3_vtab *)pVtab;
  ফেরত rc;
}

/*
** Open a new cursor on the intarray table.
*/
স্থির পূর্ণসংখ্যা intarrayOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  পূর্ণসংখ্যা rc = SQLITE_NOMEM;
  intarray_cursor *pCur;
  pCur = sqlite3_malloc64(আকার(intarray_cursor));
  যদি( pCur ){
    memset(pCur, 0, আকার(intarray_cursor));
    *ppCursor = (sqlite3_vtab_cursor *)pCur;
    rc = SQLITE_OK;
  }
  ফেরত rc;
}

/*
** Close a intarray table cursor.
*/
স্থির পূর্ণসংখ্যা intarrayClose(sqlite3_vtab_cursor *cur){
  intarray_cursor *pCur = (intarray_cursor *)cur;
  sqlite3_free(pCur);
  ফেরত SQLITE_OK;
}

/*
** Retrieve a column of data.
*/
স্থির পূর্ণসংখ্যা intarrayColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, পূর্ণসংখ্যা i){
  intarray_cursor *pCur = (intarray_cursor*)cur;
  intarray_vtab *pVtab = (intarray_vtab*)cur->pVtab;
  যদি( pCur->i>=0 && pCur->i<pVtab->pContent->n ){
    sqlite3_ফলাফল_int64(ctx, pVtab->pContent->a[pCur->i]);
  }
  ফেরত SQLITE_OK;
}

/*
** Retrieve the current rowid.
*/
স্থির পূর্ণসংখ্যা intarrayRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  intarray_cursor *pCur = (intarray_cursor *)cur;
  *pRowid = pCur->i;
  ফেরত SQLITE_OK;
}

স্থির পূর্ণসংখ্যা intarrayEof(sqlite3_vtab_cursor *cur){
  intarray_cursor *pCur = (intarray_cursor *)cur;
  intarray_vtab *pVtab = (intarray_vtab *)cur->pVtab;
  ফেরত pCur->i>=pVtab->pContent->n;
}

/*
** Advance the cursor to the next row.
*/
স্থির পূর্ণসংখ্যা intarray_পরবর্তী(sqlite3_vtab_cursor *cur){
  intarray_cursor *pCur = (intarray_cursor *)cur;
  pCur->i++;
  ফেরত SQLITE_OK;
}

/*
** Reset a intarray table cursor.
*/
স্থির পূর্ণসংখ্যা intarrayFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  পূর্ণসংখ্যা idx_অঙ্ক, ধ্রুবক অক্ষর *idxStr,
  পূর্ণসংখ্যা argc, sqlite3_মান **argv
){
  intarray_cursor *pCur = (intarray_cursor *)pVtabCursor;
  pCur->i = 0;
  ফেরত SQLITE_OK;
}

/*
** Analyse the WHERE condition.
*/
স্থির পূর্ণসংখ্যা intarray_Best_সূচক(sqlite3_vtab *tab, sqlite3_সূচক_info *pIdxInfo){
  ফেরত SQLITE_OK;
}

/*
** A virtual table module that merely echos method calls into TCL
** variables.
*/
স্থির sqlite3_module intarrayModule = {
  0,                           /* iVersion */
  intarrayCreate,              /* xCreate - create a new virtual table */
  intarrayCreate,              /* xConnect - connect to an existing vtab */
  intarray_Best_সূচক,           /* xBestIndex - find the best query index */
  intarrayDestroy,             /* xDisconnect - disconnect a vtab */
  intarrayDestroy,             /* xDestroy - destroy a vtab */
  intarrayOpen,                /* xOpen - open a cursor */
  intarrayClose,               /* xClose - close a cursor */
  intarrayFilter,              /* xFilter - configure scan constraints */
  intarray_পরবর্তী,                /* xNext - advance a cursor */
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
SQLITE_API পূর্ণসংখ্যা sqlite3_intarray_create(
  sqlite3 *db,
  ধ্রুবক অক্ষর *zName,
  sqlite3_intarray **ppReturn
){
  পূর্ণসংখ্যা rc = SQLITE_OK;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_intarray *p;

  *ppReturn = p = sqlite3_malloc64( আকার(*p) );
  যদি( p==0 ){
    ফেরত SQLITE_NOMEM;
  }
  memset(p, 0, আকার(*p));
  rc = sqlite3_create_module_v2(db, zName, &intarrayModule, p,
                                (শূন্য(*)(শূন্য*))intarrayFree);
  যদি( rc==SQLITE_OK ){
    অক্ষর *zSql;
    zSql = sqlite3_mprintf("CREATE VIRTUAL TABLE temp.%Q USING %Q",
                           zName, zName);
    rc = sqlite3_exec(db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
  }
#endif
  ফেরত rc;
}

/*
** Bind a new array array of integers to a specific intarray object.
**
** The array of integers bound must be unchanged for the duration of
** any query against the corresponding virtual table.  If the integer
** array does change or is deallocated undefined behavior will result.
*/
SQLITE_API পূর্ণসংখ্যা sqlite3_intarray_bind(
  sqlite3_intarray *pIntArray,   /* The intarray object to bind to */
  পূর্ণসংখ্যা nElements,                 /* Number of elements in the intarray */
  sqlite3_int64 *aElements,      /* Content of the intarray */
  শূন্য (*xFree)(শূন্য*)           /* How to dispose of the intarray when done */
){
  যদি( pIntArray->xFree ){
    pIntArray->xFree(pIntArray->a);
  }
  pIntArray->n = nElements;
  pIntArray->a = aElements;
  pIntArray->xFree = xFree;
  ফেরত SQLITE_OK;
}


/*****************************************************************************
** Everything below is interface for testing this module.
*/
#ifdef SQLITE_TEST
#অন্তর্ভাব "tclsqlite.h"

/*
** Routines to encode and decode pointers
*/
বাহ্যিক পূর্ণসংখ্যা getDbPointer(Tcl_Interp *interp, ধ্রুবক অক্ষর *zA, sqlite3 **ppDb);
বাহ্যিক শূন্য *sqlite3TestTextToPtr(ধ্রুবক অক্ষর*);
বাহ্যিক পূর্ণসংখ্যা sqlite3TestMakePointerStr(Tcl_Interp*, অক্ষর *zPtr, শূন্য*);
বাহ্যিক ধ্রুবক অক্ষর *sqlite3ErrName(পূর্ণসংখ্যা);

/*
**    sqlite3_intarray_create  DB  NAME
**
** Invoke the sqlite3_intarray_create interface.  A string that becomes
** the first parameter to sqlite3_intarray_bind.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI test_intarray_create(
  Client_তথ্য client_তথ্য, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  ধ্রুবক অক্ষর *zName;
  sqlite3_intarray *pArray;
  পূর্ণসংখ্যা rc = SQLITE_OK;
  অক্ষর zPtr[100];

  যদি( objc!=3 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "DB");
    ফেরত TCL_ERROR;
  }
  যদি( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ফেরত TCL_ERROR;
  zName = Tcl_GetString(objv[2]);
#ifndef SQLITE_OMIT_VIRTUALTABLE
  rc = sqlite3_intarray_create(db, zName, &pArray);
#endif
  যদি( rc!=SQLITE_OK ){
    Tcl_Append_ফলাফল(interp, sqlite3ErrName(rc), (অক্ষর*)0);
    ফেরত TCL_ERROR;
  }
  sqlite3TestMakePointerStr(interp, zPtr, pArray);
  Tcl_Append_ফলাফল(interp, zPtr, (অক্ষর*)0);
  ফেরত TCL_OK;
}

/*
**    sqlite3_intarray_bind  INTARRAY  ?VALUE ...?
**
** Invoke the sqlite3_intarray_bind interface on the given array of integers.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI test_intarray_bind(
  Client_তথ্য client_তথ্য, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_intarray *pArray;
  পূর্ণসংখ্যা rc = SQLITE_OK;
  পূর্ণসংখ্যা i, n;
  sqlite3_int64 *a;

  যদি( objc<2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "INTARRAY");
    ফেরত TCL_ERROR;
  }
  pArray = (sqlite3_intarray*)sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  n = objc - 2;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  a = sqlite3_malloc64( আকার(a[0])*n );
  যদি( a==0 ){
    Tcl_Append_ফলাফল(interp, "SQLITE_NOMEM", (অক্ষর*)0);
    ফেরত TCL_ERROR;
  }
  জন্য(i=0; i<n; i++){
    Tcl_WideInt x = 0;
    Tcl_GetWideIntFromObj(0, objv[i+2], &x);
    a[i] = x;
  }
  rc = sqlite3_intarray_bind(pArray, n, a, sqlite3_free);
  যদি( rc!=SQLITE_OK ){
    Tcl_Append_ফলাফল(interp, sqlite3ErrName(rc), (অক্ষর*)0);
    ফেরত TCL_ERROR;
  }
#endif
  ফেরত TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
পূর্ণসংখ্যা Sqlitetestintarray_Init(Tcl_Interp *interp){
  স্থির কাঠামো {
     অক্ষর *zName;
     Tcl_ObjCmdProc *xProc;
     শূন্য *client_তথ্য;
  } aObjCmd[] = {
     { "sqlite3_intarray_create", test_intarray_create, 0 },
     { "sqlite3_intarray_bind", test_intarray_bind, 0 },
  };
  পূর্ণসংখ্যা i;
  জন্য(i=0; i<আকার(aObjCmd)/আকার(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].client_তথ্য, 0);
  }
  ফেরত TCL_OK;
}

#endif /* SQLITE_TEST */
