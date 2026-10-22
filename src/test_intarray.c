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
#అన్తర్భావ "test_intarray.h"
#అన్తర్భావ <string.h>
#అన్తర్భావ <assert.h>


/*
** Definition of the sqlite3_intarray object.
**
** The internal representation of an intarray object is subject
** to change, is not externally visible, and should be used by
** the implementation of intarray only.  This object is opaque
** to users.
*/
నిర్మాణం sqlite3_intarray {
  పూర్ణాంకం n;                    /* Number of elements in the array */
  sqlite3_int64 *a;         /* Contents of the array */
  శూన్య (*xFree)(శూన్య*);     /* Function used to free a[] */
};

/* Objects used internally by the virtual table implementation */
రకనిర్వచనం నిర్మాణం intarray_vtab intarray_vtab;
రకనిర్వచనం నిర్మాణం intarray_cursor intarray_cursor;

/* An intarray table object */
నిర్మాణం intarray_vtab {
  sqlite3_vtab పునాది;            /* Base class */
  sqlite3_intarray *pContent;   /* Content of the integer array */
};

/* An intarray cursor object */
నిర్మాణం intarray_cursor {
  sqlite3_vtab_cursor పునాది;    /* Base class */
  పూర్ణాంకం i;                       /* Current cursor position */
};

/*
** None of this works unless we have virtual tables.
*/
#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE

/*
** Free an sqlite3_intarray object.
*/
స్థిర శూన్య intarrayFree(శూన్య *pX){
  sqlite3_intarray *p = (sqlite3_intarray*)pX;
  ఒకవేళ( p->xFree ){
    p->xFree(p->a);
  }
  sqlite3_free(p);
}

/*
** Table destructor for the intarray module.
*/
స్థిర పూర్ణాంకం intarrayDestroy(sqlite3_vtab *p){
  intarray_vtab *pVtab = (intarray_vtab*)p;
  sqlite3_free(pVtab);
  తిరిగి 0;
}

/*
** Table constructor for the intarray module.
*/
స్థిర పూర్ణాంకం intarrayCreate(
  sqlite3 *db,              /* Database where module is created */
  శూన్య *pAux,               /* clientdata for the module */
  పూర్ణాంకం argc,                 /* Number of arguments */
  స్థిరరాశి అక్షరం *స్థిరరాశి*argv,   /* Value for all arguments */
  sqlite3_vtab **ppVtab,    /* Write the new virtual table object here */
  అక్షరం **pzErr              /* Put error message text here */
){
  పూర్ణాంకం rc = SQLITE_NOMEM;
  intarray_vtab *pVtab = sqlite3_malloc64(పరిమాణం(intarray_vtab));

  ఒకవేళ( pVtab ){
    memset(pVtab, 0, పరిమాణం(intarray_vtab));
    pVtab->pContent = (sqlite3_intarray*)pAux;
    rc = sqlite3_declare_vtab(db, "CREATE TABLE x(value INTEGER PRIMARY KEY)");
  }
  *ppVtab = (sqlite3_vtab *)pVtab;
  తిరిగి rc;
}

/*
** Open a new cursor on the intarray table.
*/
స్థిర పూర్ణాంకం intarrayOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  పూర్ణాంకం rc = SQLITE_NOMEM;
  intarray_cursor *pCur;
  pCur = sqlite3_malloc64(పరిమాణం(intarray_cursor));
  ఒకవేళ( pCur ){
    memset(pCur, 0, పరిమాణం(intarray_cursor));
    *ppCursor = (sqlite3_vtab_cursor *)pCur;
    rc = SQLITE_OK;
  }
  తిరిగి rc;
}

/*
** Close a intarray table cursor.
*/
స్థిర పూర్ణాంకం intarrayClose(sqlite3_vtab_cursor *cur){
  intarray_cursor *pCur = (intarray_cursor *)cur;
  sqlite3_free(pCur);
  తిరిగి SQLITE_OK;
}

/*
** Retrieve a column of data.
*/
స్థిర పూర్ణాంకం intarray_స్తంభం(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, పూర్ణాంకం i){
  intarray_cursor *pCur = (intarray_cursor*)cur;
  intarray_vtab *pVtab = (intarray_vtab*)cur->pVtab;
  ఒకవేళ( pCur->i>=0 && pCur->i<pVtab->pContent->n ){
    sqlite3_ఫలితం_int64(ctx, pVtab->pContent->a[pCur->i]);
  }
  తిరిగి SQLITE_OK;
}

/*
** Retrieve the current rowid.
*/
స్థిర పూర్ణాంకం intarrayRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  intarray_cursor *pCur = (intarray_cursor *)cur;
  *pRowid = pCur->i;
  తిరిగి SQLITE_OK;
}

స్థిర పూర్ణాంకం intarrayEof(sqlite3_vtab_cursor *cur){
  intarray_cursor *pCur = (intarray_cursor *)cur;
  intarray_vtab *pVtab = (intarray_vtab *)cur->pVtab;
  తిరిగి pCur->i>=pVtab->pContent->n;
}

/*
** Advance the cursor to the next row.
*/
స్థిర పూర్ణాంకం intarray_తదుపరి(sqlite3_vtab_cursor *cur){
  intarray_cursor *pCur = (intarray_cursor *)cur;
  pCur->i++;
  తిరిగి SQLITE_OK;
}

/*
** Reset a intarray table cursor.
*/
స్థిర పూర్ణాంకం intarrayFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  పూర్ణాంకం idx_అంకె, స్థిరరాశి అక్షరం *idxStr,
  పూర్ణాంకం argc, sqlite3_విలువ **argv
){
  intarray_cursor *pCur = (intarray_cursor *)pVtabCursor;
  pCur->i = 0;
  తిరిగి SQLITE_OK;
}

/*
** Analyse the WHERE condition.
*/
స్థిర పూర్ణాంకం intarray_ఉత్తమ_సూచీ(sqlite3_vtab *tab, sqlite3_సూచీ_info *pIdxInfo){
  తిరిగి SQLITE_OK;
}

/*
** A virtual table module that merely echos method calls into TCL
** variables.
*/
స్థిర sqlite3_module intarrayModule = {
  0,                           /* iVersion */
  intarrayCreate,              /* xCreate - create a new virtual table */
  intarrayCreate,              /* xConnect - connect to an existing vtab */
  intarray_ఉత్తమ_సూచీ,           /* xBestIndex - find the best query index */
  intarrayDestroy,             /* xDisconnect - disconnect a vtab */
  intarrayDestroy,             /* xDestroy - destroy a vtab */
  intarrayOpen,                /* xOpen - open a cursor */
  intarrayClose,               /* xClose - close a cursor */
  intarrayFilter,              /* xFilter - configure scan constraints */
  intarray_తదుపరి,                /* xNext - advance a cursor */
  intarrayEof,                 /* xEof */
  intarray_స్తంభం,              /* xColumn - read data */
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

#అయితే_ముగింపు /* !defined(SQLITE_OMIT_VIRTUALTABLE) */

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
SQLITE_API పూర్ణాంకం sqlite3_intarray_create(
  sqlite3 *db,
  స్థిరరాశి అక్షరం *zName,
  sqlite3_intarray **ppReturn
){
  పూర్ణాంకం rc = SQLITE_OK;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
  sqlite3_intarray *p;

  *ppReturn = p = sqlite3_malloc64( పరిమాణం(*p) );
  ఒకవేళ( p==0 ){
    తిరిగి SQLITE_NOMEM;
  }
  memset(p, 0, పరిమాణం(*p));
  rc = sqlite3_create_module_v2(db, zName, &intarrayModule, p,
                                (శూన్య(*)(శూన్య*))intarrayFree);
  ఒకవేళ( rc==SQLITE_OK ){
    అక్షరం *zSql;
    zSql = sqlite3_mprintf("CREATE VIRTUAL TABLE temp.%Q USING %Q",
                           zName, zName);
    rc = sqlite3_exec(db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
  }
#అయితే_ముగింపు
  తిరిగి rc;
}

/*
** Bind a new array array of integers to a specific intarray object.
**
** The array of integers bound must be unchanged for the duration of
** any query against the corresponding virtual table.  If the integer
** array does change or is deallocated undefined behavior will result.
*/
SQLITE_API పూర్ణాంకం sqlite3_intarray_bind(
  sqlite3_intarray *pIntArray,   /* The intarray object to bind to */
  పూర్ణాంకం nElements,                 /* Number of elements in the intarray */
  sqlite3_int64 *aElements,      /* Content of the intarray */
  శూన్య (*xFree)(శూన్య*)           /* How to dispose of the intarray when done */
){
  ఒకవేళ( pIntArray->xFree ){
    pIntArray->xFree(pIntArray->a);
  }
  pIntArray->n = nElements;
  pIntArray->a = aElements;
  pIntArray->xFree = xFree;
  తిరిగి SQLITE_OK;
}


/*****************************************************************************
** Everything below is interface for testing this module.
*/
#నిర్వచితమైతే SQLITE_TEST
#అన్తర్భావ "tclsqlite.h"

/*
** Routines to encode and decode pointers
*/
బాహ్య పూర్ణాంకం get_Db_సూచిక(Tcl_Interp *interp, స్థిరరాశి అక్షరం *zA, sqlite3 **ppDb);
బాహ్య శూన్య *sqlite3_Test_పాఠ్యం_గమ్యం_Ptr(స్థిరరాశి అక్షరం*);
బాహ్య పూర్ణాంకం sqlite3_Test_Make_సూచిక_Str(Tcl_Interp*, అక్షరం *zPtr, శూన్య*);
బాహ్య స్థిరరాశి అక్షరం *sqlite3ErrName(పూర్ణాంకం);

/*
**    sqlite3_intarray_create  DB  NAME
**
** Invoke the sqlite3_intarray_create interface.  A string that becomes
** the first parameter to sqlite3_intarray_bind.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_intarray_create(
  Client_దత్తాంశం client_దత్తాంశం, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  స్థిరరాశి అక్షరం *zName;
  sqlite3_intarray *pArray;
  పూర్ణాంకం rc = SQLITE_OK;
  అక్షరం zPtr[100];

  ఒకవేళ( objc!=3 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "DB");
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( get_Db_సూచిక(interp, Tcl_GetString(objv[1]), &db) ) తిరిగి TCL_ERROR;
  zName = Tcl_GetString(objv[2]);
#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
  rc = sqlite3_intarray_create(db, zName, &pArray);
#అయితే_ముగింపు
  ఒకవేళ( rc!=SQLITE_OK ){
    Tcl_Append_ఫలితం(interp, sqlite3ErrName(rc), (అక్షరం*)0);
    తిరిగి TCL_ERROR;
  }
  sqlite3_Test_Make_సూచిక_Str(interp, zPtr, pArray);
  Tcl_Append_ఫలితం(interp, zPtr, (అక్షరం*)0);
  తిరిగి TCL_OK;
}

/*
**    sqlite3_intarray_bind  INTARRAY  ?VALUE ...?
**
** Invoke the sqlite3_intarray_bind interface on the given array of integers.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_intarray_bind(
  Client_దత్తాంశం client_దత్తాంశం, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_intarray *pArray;
  పూర్ణాంకం rc = SQLITE_OK;
  పూర్ణాంకం i, n;
  sqlite3_int64 *a;

  ఒకవేళ( objc<2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "INTARRAY");
    తిరిగి TCL_ERROR;
  }
  pArray = (sqlite3_intarray*)sqlite3_Test_పాఠ్యం_గమ్యం_Ptr(Tcl_GetString(objv[1]));
  n = objc - 2;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
  a = sqlite3_malloc64( పరిమాణం(a[0])*n );
  ఒకవేళ( a==0 ){
    Tcl_Append_ఫలితం(interp, "SQLITE_NOMEM", (అక్షరం*)0);
    తిరిగి TCL_ERROR;
  }
  కొరకు(i=0; i<n; i++){
    Tcl_WideInt x = 0;
    Tcl_GetWideIntFromObj(0, objv[i+2], &x);
    a[i] = x;
  }
  rc = sqlite3_intarray_bind(pArray, n, a, sqlite3_free);
  ఒకవేళ( rc!=SQLITE_OK ){
    Tcl_Append_ఫలితం(interp, sqlite3ErrName(rc), (అక్షరం*)0);
    తిరిగి TCL_ERROR;
  }
#అయితే_ముగింపు
  తిరిగి TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
పూర్ణాంకం Sqlitetestintarray_Init(Tcl_Interp *interp){
  స్థిర నిర్మాణం {
     అక్షరం *zName;
     Tcl_ObjCmdProc *xProc;
     శూన్య *client_దత్తాంశం;
  } aObjCmd[] = {
     { "sqlite3_intarray_create", test_intarray_create, 0 },
     { "sqlite3_intarray_bind", test_intarray_bind, 0 },
  };
  పూర్ణాంకం i;
  కొరకు(i=0; i<పరిమాణం(aObjCmd)/పరిమాణం(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].client_దత్తాంశం, 0);
  }
  తిరిగి TCL_OK;
}

#అయితే_ముగింపు /* SQLITE_TEST */
