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
#समावेश "test_intarray.h"
#समावेश <string.h>
#समावेश <assert.h>


/*
** Definition of the sqlite3_intarray object.
**
** The internal representation of an intarray object is subject
** to change, is not externally visible, and should be used by
** the implementation of intarray only.  This object is opaque
** to users.
*/
काष्ठा sqlite3_intarray {
  पूर्णांक n;                    /* Number of elements in the array */
  sqlite3_int64 *a;         /* Contents of the array */
  व्योम (*xFree)(व्योम*);     /* Function used to free a[] */
};

/* Objects used internally by the virtual table implementation */
प्रकार काष्ठा intarray_vtab intarray_vtab;
प्रकार काष्ठा intarray_cursor intarray_cursor;

/* An intarray table object */
काष्ठा intarray_vtab {
  sqlite3_vtab base;            /* Base class */
  sqlite3_intarray *pContent;   /* Content of the integer array */
};

/* An intarray cursor object */
काष्ठा intarray_cursor {
  sqlite3_vtab_cursor base;    /* Base class */
  पूर्णांक i;                       /* Current cursor position */
};

/*
** None of this works unless we have virtual tables.
*/
#अगर_अघोषित SQLITE_OMIT_VIRTUALTABLE

/*
** Free an sqlite3_intarray object.
*/
अटल व्योम intarrayFree(व्योम *pX){
  sqlite3_intarray *p = (sqlite3_intarray*)pX;
  अगर( p->xFree ){
    p->xFree(p->a);
  }
  sqlite3_free(p);
}

/*
** Table destructor for the intarray module.
*/
अटल पूर्णांक intarrayDestroy(sqlite3_vtab *p){
  intarray_vtab *pVtab = (intarray_vtab*)p;
  sqlite3_free(pVtab);
  वापस 0;
}

/*
** Table constructor for the intarray module.
*/
अटल पूर्णांक intarrayCreate(
  sqlite3 *db,              /* Database where module is created */
  व्योम *pAux,               /* clientdata for the module */
  पूर्णांक argc,                 /* Number of arguments */
  स्थिर अक्षर *स्थिर*argv,   /* Value for all arguments */
  sqlite3_vtab **ppVtab,    /* Write the new virtual table object here */
  अक्षर **pzErr              /* Put error message text here */
){
  पूर्णांक rc = SQLITE_NOMEM;
  intarray_vtab *pVtab = sqlite3_malloc64(माप(intarray_vtab));

  अगर( pVtab ){
    स_रखो(pVtab, 0, माप(intarray_vtab));
    pVtab->pContent = (sqlite3_intarray*)pAux;
    rc = sqlite3_declare_vtab(db, "CREATE TABLE x(value INTEGER PRIMARY KEY)");
  }
  *ppVtab = (sqlite3_vtab *)pVtab;
  वापस rc;
}

/*
** Open a new cursor on the intarray table.
*/
अटल पूर्णांक intarrayOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  पूर्णांक rc = SQLITE_NOMEM;
  intarray_cursor *pCur;
  pCur = sqlite3_malloc64(माप(intarray_cursor));
  अगर( pCur ){
    स_रखो(pCur, 0, माप(intarray_cursor));
    *ppCursor = (sqlite3_vtab_cursor *)pCur;
    rc = SQLITE_OK;
  }
  वापस rc;
}

/*
** Close a intarray table cursor.
*/
अटल पूर्णांक intarrayClose(sqlite3_vtab_cursor *cur){
  intarray_cursor *pCur = (intarray_cursor *)cur;
  sqlite3_free(pCur);
  वापस SQLITE_OK;
}

/*
** Retrieve a column of data.
*/
अटल पूर्णांक intarrayColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, पूर्णांक i){
  intarray_cursor *pCur = (intarray_cursor*)cur;
  intarray_vtab *pVtab = (intarray_vtab*)cur->pVtab;
  अगर( pCur->i>=0 && pCur->i<pVtab->pContent->n ){
    sqlite3_परिणाम_int64(ctx, pVtab->pContent->a[pCur->i]);
  }
  वापस SQLITE_OK;
}

/*
** Retrieve the current rowid.
*/
अटल पूर्णांक intarrayRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  intarray_cursor *pCur = (intarray_cursor *)cur;
  *pRowid = pCur->i;
  वापस SQLITE_OK;
}

अटल पूर्णांक intarrayEof(sqlite3_vtab_cursor *cur){
  intarray_cursor *pCur = (intarray_cursor *)cur;
  intarray_vtab *pVtab = (intarray_vtab *)cur->pVtab;
  वापस pCur->i>=pVtab->pContent->n;
}

/*
** Advance the cursor to the next row.
*/
अटल पूर्णांक intarray_अग्र(sqlite3_vtab_cursor *cur){
  intarray_cursor *pCur = (intarray_cursor *)cur;
  pCur->i++;
  वापस SQLITE_OK;
}

/*
** Reset a intarray table cursor.
*/
अटल पूर्णांक intarrayFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  पूर्णांक idx_अंक, स्थिर अक्षर *idxStr,
  पूर्णांक argc, sqlite3_मूल्य **argv
){
  intarray_cursor *pCur = (intarray_cursor *)pVtabCursor;
  pCur->i = 0;
  वापस SQLITE_OK;
}

/*
** Analyse the WHERE condition.
*/
अटल पूर्णांक intarray_Best_सूचकांक(sqlite3_vtab *tab, sqlite3_सूचकांक_info *pIdxInfo){
  वापस SQLITE_OK;
}

/*
** A virtual table module that merely echos method calls into TCL
** variables.
*/
अटल sqlite3_module intarrayModule = {
  0,                           /* iVersion */
  intarrayCreate,              /* xCreate - create a new virtual table */
  intarrayCreate,              /* xConnect - connect to an existing vtab */
  intarray_Best_सूचकांक,           /* xBestIndex - find the best query index */
  intarrayDestroy,             /* xDisconnect - disconnect a vtab */
  intarrayDestroy,             /* xDestroy - destroy a vtab */
  intarrayOpen,                /* xOpen - open a cursor */
  intarrayClose,               /* xClose - close a cursor */
  intarrayFilter,              /* xFilter - configure scan constraints */
  intarray_अग्र,                /* xNext - advance a cursor */
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

#पूर्ण_अगर /* !defined(SQLITE_OMIT_VIRTUALTABLE) */

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
SQLITE_API पूर्णांक sqlite3_intarray_create(
  sqlite3 *db,
  स्थिर अक्षर *zName,
  sqlite3_intarray **ppReturn
){
  पूर्णांक rc = SQLITE_OK;
#अगर_अघोषित SQLITE_OMIT_VIRTUALTABLE
  sqlite3_intarray *p;

  *ppReturn = p = sqlite3_malloc64( माप(*p) );
  अगर( p==0 ){
    वापस SQLITE_NOMEM;
  }
  स_रखो(p, 0, माप(*p));
  rc = sqlite3_create_module_v2(db, zName, &intarrayModule, p,
                                (व्योम(*)(व्योम*))intarrayFree);
  अगर( rc==SQLITE_OK ){
    अक्षर *zSql;
    zSql = sqlite3_mprintf("CREATE VIRTUAL TABLE temp.%Q USING %Q",
                           zName, zName);
    rc = sqlite3_exec(db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
  }
#पूर्ण_अगर
  वापस rc;
}

/*
** Bind a new array array of integers to a specific intarray object.
**
** The array of integers bound must be unchanged for the duration of
** any query against the corresponding virtual table.  If the integer
** array does change or is deallocated undefined behavior will result.
*/
SQLITE_API पूर्णांक sqlite3_intarray_bind(
  sqlite3_intarray *pIntArray,   /* The intarray object to bind to */
  पूर्णांक nElements,                 /* Number of elements in the intarray */
  sqlite3_int64 *aElements,      /* Content of the intarray */
  व्योम (*xFree)(व्योम*)           /* How to dispose of the intarray when done */
){
  अगर( pIntArray->xFree ){
    pIntArray->xFree(pIntArray->a);
  }
  pIntArray->n = nElements;
  pIntArray->a = aElements;
  pIntArray->xFree = xFree;
  वापस SQLITE_OK;
}


/*****************************************************************************
** Everything below is interface for testing this module.
*/
#अगर_घोषित SQLITE_TEST
#समावेश "tclsqlite.h"

/*
** Routines to encode and decode pointers
*/
बाह्य पूर्णांक getDbPointer(Tcl_Interp *interp, स्थिर अक्षर *zA, sqlite3 **ppDb);
बाह्य व्योम *sqlite3TestTextToPtr(स्थिर अक्षर*);
बाह्य पूर्णांक sqlite3TestMakePointerStr(Tcl_Interp*, अक्षर *zPtr, व्योम*);
बाह्य स्थिर अक्षर *sqlite3ErrName(पूर्णांक);

/*
**    sqlite3_intarray_create  DB  NAME
**
** Invoke the sqlite3_intarray_create interface.  A string that becomes
** the first parameter to sqlite3_intarray_bind.
*/
अटल पूर्णांक SQLITE_TCLAPI test_intarray_create(
  Client_तथ्य client_तथ्य, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  स्थिर अक्षर *zName;
  sqlite3_intarray *pArray;
  पूर्णांक rc = SQLITE_OK;
  अक्षर zPtr[100];

  अगर( objc!=3 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "DB");
    वापस TCL_ERROR;
  }
  अगर( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) वापस TCL_ERROR;
  zName = Tcl_GetString(objv[2]);
#अगर_अघोषित SQLITE_OMIT_VIRTUALTABLE
  rc = sqlite3_intarray_create(db, zName, &pArray);
#पूर्ण_अगर
  अगर( rc!=SQLITE_OK ){
    Tcl_Append_परिणाम(interp, sqlite3ErrName(rc), (अक्षर*)0);
    वापस TCL_ERROR;
  }
  sqlite3TestMakePointerStr(interp, zPtr, pArray);
  Tcl_Append_परिणाम(interp, zPtr, (अक्षर*)0);
  वापस TCL_OK;
}

/*
**    sqlite3_intarray_bind  INTARRAY  ?VALUE ...?
**
** Invoke the sqlite3_intarray_bind interface on the given array of integers.
*/
अटल पूर्णांक SQLITE_TCLAPI test_intarray_bind(
  Client_तथ्य client_तथ्य, /* Not used */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_intarray *pArray;
  पूर्णांक rc = SQLITE_OK;
  पूर्णांक i, n;
  sqlite3_int64 *a;

  अगर( objc<2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "INTARRAY");
    वापस TCL_ERROR;
  }
  pArray = (sqlite3_intarray*)sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  n = objc - 2;
#अगर_अघोषित SQLITE_OMIT_VIRTUALTABLE
  a = sqlite3_malloc64( माप(a[0])*n );
  अगर( a==0 ){
    Tcl_Append_परिणाम(interp, "SQLITE_NOMEM", (अक्षर*)0);
    वापस TCL_ERROR;
  }
  क्रम(i=0; i<n; i++){
    Tcl_WideInt x = 0;
    Tcl_GetWideIntFromObj(0, objv[i+2], &x);
    a[i] = x;
  }
  rc = sqlite3_intarray_bind(pArray, n, a, sqlite3_free);
  अगर( rc!=SQLITE_OK ){
    Tcl_Append_परिणाम(interp, sqlite3ErrName(rc), (अक्षर*)0);
    वापस TCL_ERROR;
  }
#पूर्ण_अगर
  वापस TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
पूर्णांक Sqlitetestintarray_Init(Tcl_Interp *interp){
  अटल काष्ठा {
     अक्षर *zName;
     Tcl_ObjCmdProc *xProc;
     व्योम *client_तथ्य;
  } aObjCmd[] = {
     { "sqlite3_intarray_create", test_intarray_create, 0 },
     { "sqlite3_intarray_bind", test_intarray_bind, 0 },
  };
  पूर्णांक i;
  क्रम(i=0; i<माप(aObjCmd)/माप(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].client_तथ्य, 0);
  }
  वापस TCL_OK;
}

#पूर्ण_अगर /* SQLITE_TEST */
