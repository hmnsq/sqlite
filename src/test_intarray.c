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
#شامل "test_intarray.h"
#شامل <string.h>
#شامل <assert.h>


/*
** Definition of the sqlite3_intarray object.
**
** The internal representation of an intarray object is subject
** to change, is not externally visible, and should be used by
** the implementation of intarray only.  This object is opaque
** to users.
*/
ساخت sqlite3_intarray {
  صحیح n;                    /* Number of elements in the array */
  sqlite3_int64 *a;         /* Contents of the array */
  خالی (*xFree)(خالی*);     /* Function used to free a[] */
};

/* Objects used internally by the virtual table implementation */
قسمتعریف ساخت intarray_vtab intarray_vtab;
قسمتعریف ساخت intarray_cursor intarray_cursor;

/* An intarray table object */
ساخت intarray_vtab {
  sqlite3_vtab بنیاد;            /* Base class */
  sqlite3_intarray *pContent;   /* Content of the integer array */
};

/* An intarray cursor object */
ساخت intarray_cursor {
  sqlite3_vtab_cursor بنیاد;    /* Base class */
  صحیح i;                       /* Current cursor position */
};

/*
** None of this works unless we have virtual tables.
*/
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_VIRTUALTABLE

/*
** Free an sqlite3_intarray object.
*/
ساکن خالی intarrayFree(خالی *pX){
  sqlite3_intarray *p = (sqlite3_intarray*)pX;
  اگر( p->xFree ){
    p->xFree(p->a);
  }
  sqlite3_free(p);
}

/*
** Table destructor for the intarray module.
*/
ساکن صحیح intarrayDestroy(sqlite3_vtab *p){
  intarray_vtab *pVtab = (intarray_vtab*)p;
  sqlite3_free(pVtab);
  واپس 0;
}

/*
** Table constructor for the intarray module.
*/
ساکن صحیح intarrayCreate(
  sqlite3 *db,              /* Database where module is created */
  خالی *pAux,               /* clientdata for the module */
  صحیح argc,                 /* Number of arguments */
  ثابت حرف *ثابت*argv,   /* Value for all arguments */
  sqlite3_vtab **ppVtab,    /* Write the new virtual table object here */
  حرف **pzErr              /* Put error message text here */
){
  صحیح rc = SQLITE_NOMEM;
  intarray_vtab *pVtab = sqlite3_malloc64(حجم(intarray_vtab));

  اگر( pVtab ){
    memset(pVtab, 0, حجم(intarray_vtab));
    pVtab->pContent = (sqlite3_intarray*)pAux;
    rc = sqlite3_declare_vtab(db, "CREATE TABLE x(value INTEGER PRIMARY KEY)");
  }
  *ppVtab = (sqlite3_vtab *)pVtab;
  واپس rc;
}

/*
** Open a new cursor on the intarray table.
*/
ساکن صحیح intarrayOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  صحیح rc = SQLITE_NOMEM;
  intarray_cursor *pCur;
  pCur = sqlite3_malloc64(حجم(intarray_cursor));
  اگر( pCur ){
    memset(pCur, 0, حجم(intarray_cursor));
    *ppCursor = (sqlite3_vtab_cursor *)pCur;
    rc = SQLITE_OK;
  }
  واپس rc;
}

/*
** Close a intarray table cursor.
*/
ساکن صحیح intarrayClose(sqlite3_vtab_cursor *cur){
  intarray_cursor *pCur = (intarray_cursor *)cur;
  sqlite3_free(pCur);
  واپس SQLITE_OK;
}

/*
** Retrieve a column of data.
*/
ساکن صحیح intarray_عمود(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, صحیح i){
  intarray_cursor *pCur = (intarray_cursor*)cur;
  intarray_vtab *pVtab = (intarray_vtab*)cur->pVtab;
  اگر( pCur->i>=0 && pCur->i<pVtab->pContent->n ){
    sqlite3_نتیجہ_int64(ctx, pVtab->pContent->a[pCur->i]);
  }
  واپس SQLITE_OK;
}

/*
** Retrieve the current rowid.
*/
ساکن صحیح intarrayRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  intarray_cursor *pCur = (intarray_cursor *)cur;
  *pRowid = pCur->i;
  واپس SQLITE_OK;
}

ساکن صحیح intarrayEof(sqlite3_vtab_cursor *cur){
  intarray_cursor *pCur = (intarray_cursor *)cur;
  intarray_vtab *pVtab = (intarray_vtab *)cur->pVtab;
  واپس pCur->i>=pVtab->pContent->n;
}

/*
** Advance the cursor to the next row.
*/
ساکن صحیح intarray_اگلا(sqlite3_vtab_cursor *cur){
  intarray_cursor *pCur = (intarray_cursor *)cur;
  pCur->i++;
  واپس SQLITE_OK;
}

/*
** Reset a intarray table cursor.
*/
ساکن صحیح intarrayFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  صحیح idx_عدد, ثابت حرف *idxStr,
  صحیح argc, sqlite3_قدر **argv
){
  intarray_cursor *pCur = (intarray_cursor *)pVtabCursor;
  pCur->i = 0;
  واپس SQLITE_OK;
}

/*
** Analyse the WHERE condition.
*/
ساکن صحیح intarray_بہترین_اشاریہ(sqlite3_vtab *tab, sqlite3_اشاریہ_info *pIdxInfo){
  واپس SQLITE_OK;
}

/*
** A virtual table module that merely echos method calls into TCL
** variables.
*/
ساکن sqlite3_module intarrayModule = {
  0,                           /* iVersion */
  intarrayCreate,              /* xCreate - create a new virtual table */
  intarrayCreate,              /* xConnect - connect to an existing vtab */
  intarray_بہترین_اشاریہ,           /* xBestIndex - find the best query index */
  intarrayDestroy,             /* xDisconnect - disconnect a vtab */
  intarrayDestroy,             /* xDestroy - destroy a vtab */
  intarrayOpen,                /* xOpen - open a cursor */
  intarrayClose,               /* xClose - close a cursor */
  intarrayFilter,              /* xFilter - configure scan constraints */
  intarray_اگلا,                /* xNext - advance a cursor */
  intarrayEof,                 /* xEof */
  intarray_عمود,              /* xColumn - read data */
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

#اگر_ختم /* !defined(SQLITE_OMIT_VIRTUALTABLE) */

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
SQLITE_API صحیح sqlite3_intarray_create(
  sqlite3 *db,
  ثابت حرف *zName,
  sqlite3_intarray **ppReturn
){
  صحیح rc = SQLITE_OK;
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_VIRTUALTABLE
  sqlite3_intarray *p;

  *ppReturn = p = sqlite3_malloc64( حجم(*p) );
  اگر( p==0 ){
    واپس SQLITE_NOMEM;
  }
  memset(p, 0, حجم(*p));
  rc = sqlite3_create_module_v2(db, zName, &intarrayModule, p,
                                (خالی(*)(خالی*))intarrayFree);
  اگر( rc==SQLITE_OK ){
    حرف *zSql;
    zSql = sqlite3_mprintf("CREATE VIRTUAL TABLE temp.%Q USING %Q",
                           zName, zName);
    rc = sqlite3_exec(db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
  }
#اگر_ختم
  واپس rc;
}

/*
** Bind a new array array of integers to a specific intarray object.
**
** The array of integers bound must be unchanged for the duration of
** any query against the corresponding virtual table.  If the integer
** array does change or is deallocated undefined behavior will result.
*/
SQLITE_API صحیح sqlite3_intarray_bind(
  sqlite3_intarray *pIntArray,   /* The intarray object to bind to */
  صحیح nElements,                 /* Number of elements in the intarray */
  sqlite3_int64 *aElements,      /* Content of the intarray */
  خالی (*xFree)(خالی*)           /* How to dispose of the intarray when done */
){
  اگر( pIntArray->xFree ){
    pIntArray->xFree(pIntArray->a);
  }
  pIntArray->n = nElements;
  pIntArray->a = aElements;
  pIntArray->xFree = xFree;
  واپس SQLITE_OK;
}


/*****************************************************************************
** Everything below is interface for testing this module.
*/
#اگر_تعریف_شدہ SQLITE_TEST
#شامل "tclsqlite.h"

/*
** Routines to encode and decode pointers
*/
خارجی صحیح get_Db_اشارہ_گر(Tcl_Interp *interp, ثابت حرف *zA, sqlite3 **ppDb);
خارجی خالی *sqlite3_Test_متن_مقصد_Ptr(ثابت حرف*);
خارجی صحیح sqlite3_Test_Make_اشارہ_گر_Str(Tcl_Interp*, حرف *zPtr, خالی*);
خارجی ثابت حرف *sqlite3ErrName(صحیح);

/*
**    sqlite3_intarray_create  DB  NAME
**
** Invoke the sqlite3_intarray_create interface.  A string that becomes
** the first parameter to sqlite3_intarray_bind.
*/
ساکن صحیح SQLITE_TCLAPI test_intarray_create(
  Client_ڈیٹا client_ڈیٹا, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  ثابت حرف *zName;
  sqlite3_intarray *pArray;
  صحیح rc = SQLITE_OK;
  حرف zPtr[100];

  اگر( objc!=3 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "DB");
    واپس TCL_ERROR;
  }
  اگر( get_Db_اشارہ_گر(interp, Tcl_GetString(objv[1]), &db) ) واپس TCL_ERROR;
  zName = Tcl_GetString(objv[2]);
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_VIRTUALTABLE
  rc = sqlite3_intarray_create(db, zName, &pArray);
#اگر_ختم
  اگر( rc!=SQLITE_OK ){
    Tcl_Append_نتیجہ(interp, sqlite3ErrName(rc), (حرف*)0);
    واپس TCL_ERROR;
  }
  sqlite3_Test_Make_اشارہ_گر_Str(interp, zPtr, pArray);
  Tcl_Append_نتیجہ(interp, zPtr, (حرف*)0);
  واپس TCL_OK;
}

/*
**    sqlite3_intarray_bind  INTARRAY  ?VALUE ...?
**
** Invoke the sqlite3_intarray_bind interface on the given array of integers.
*/
ساکن صحیح SQLITE_TCLAPI test_intarray_bind(
  Client_ڈیٹا client_ڈیٹا, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_intarray *pArray;
  صحیح rc = SQLITE_OK;
  صحیح i, n;
  sqlite3_int64 *a;

  اگر( objc<2 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "INTARRAY");
    واپس TCL_ERROR;
  }
  pArray = (sqlite3_intarray*)sqlite3_Test_متن_مقصد_Ptr(Tcl_GetString(objv[1]));
  n = objc - 2;
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_VIRTUALTABLE
  a = sqlite3_malloc64( حجم(a[0])*n );
  اگر( a==0 ){
    Tcl_Append_نتیجہ(interp, "SQLITE_NOMEM", (حرف*)0);
    واپس TCL_ERROR;
  }
  برائے(i=0; i<n; i++){
    Tcl_WideInt x = 0;
    Tcl_GetWideIntFromObj(0, objv[i+2], &x);
    a[i] = x;
  }
  rc = sqlite3_intarray_bind(pArray, n, a, sqlite3_free);
  اگر( rc!=SQLITE_OK ){
    Tcl_Append_نتیجہ(interp, sqlite3ErrName(rc), (حرف*)0);
    واپس TCL_ERROR;
  }
#اگر_ختم
  واپس TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
صحیح Sqlitetestintarray_Init(Tcl_Interp *interp){
  ساکن ساخت {
     حرف *zName;
     Tcl_ObjCmdProc *xProc;
     خالی *client_ڈیٹا;
  } aObjCmd[] = {
     { "sqlite3_intarray_create", test_intarray_create, 0 },
     { "sqlite3_intarray_bind", test_intarray_bind, 0 },
  };
  صحیح i;
  برائے(i=0; i<حجم(aObjCmd)/حجم(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].client_ڈیٹا, 0);
  }
  واپس TCL_OK;
}

#اگر_ختم /* SQLITE_TEST */
