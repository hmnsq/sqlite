/*
** 2007 March 29
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
** This file contains obscure tests of the C-interface required
** for completeness. Test code is written in C for these cases
** as there is not much point in binding to Tcl.
*/
#ضمن "sqliteInt.h"
#ضمن "tclsqlite.h"
#ضمن <stdlib.h>
#ضمن <string.h>

/*
** c_collation_test
*/
ساكن صحيح SQLITE_TCLAPI c_collation_test(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  ثابت حرف *zErrFunction = "N/A";
  sqlite3 *db;

  صحيح rc;
  إذا( objc!=1 ){
    Tcl_Wrong_عدد_Args(interp, 1, objv, "");
    أرجع TCL_ERROR;
  }

  /* Open a database. */
  rc = sqlite3_open(":memory:", &db);
  إذا( rc!=SQLITE_OK ){
    zErrFunction = "sqlite3_open";
    اذهب error_out;
  }

  rc = sqlite3_create_collation(db, "collate", 456, 0, 0);
  إذا( rc!=SQLITE_MISUSE ){
    sqlite3_close(db);
    zErrFunction = "sqlite3_create_collation";
    اذهب error_out;
  }

  sqlite3_close(db);
  أرجع TCL_OK;

error_out:
  Tcl_ResetResult(interp);
  Tcl_AppendResult(interp, "Error testing function: ", zErrFunction, عدم);
  أرجع TCL_ERROR;
}

/*
** c_realloc_test
*/
ساكن صحيح SQLITE_TCLAPI c_realloc_test(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  فراغ *p;
  ثابت حرف *zErrFunction = "N/A";

  إذا( objc!=1 ){
    Tcl_Wrong_عدد_Args(interp, 1, objv, "");
    أرجع TCL_ERROR;
  }

  p = sqlite3_malloc(5);
  إذا( !p ){
    zErrFunction = "sqlite3_malloc";
    اذهب error_out;
  }

  /* Test that realloc()ing a block of memory to a negative size is
  ** the same as free()ing that memory.
  */
  p = sqlite3_realloc(p, -1);
  إذا( p ){
    zErrFunction = "sqlite3_realloc";
    اذهب error_out;
  }

  أرجع TCL_OK;

error_out:
  Tcl_ResetResult(interp);
  Tcl_AppendResult(interp, "Error testing function: ", zErrFunction, عدم);
  أرجع TCL_ERROR;
}


/*
** c_misuse_test
*/
ساكن صحيح SQLITE_TCLAPI c_misuse_test(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  ثابت حرف *zErrFunction = "N/A";
  sqlite3 *db = 0;
  sqlite3_stmt *pStmt;
  صحيح rc;

  إذا( objc!=1 ){
    Tcl_Wrong_عدد_Args(interp, 1, objv, "");
    أرجع TCL_ERROR;
  }

  /* Open a database. Then close it again. We need to do this so that
  ** we have a "closed database handle" to pass to various API functions.
  */
  rc = sqlite3_open(":memory:", &db);
  إذا( rc!=SQLITE_OK ){
    zErrFunction = "sqlite3_open";
    اذهب error_out;
  }
  sqlite3_close(db);


  rc = sqlite3_errcode(db);
  إذا( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_errcode";
    اذهب error_out;
  }

  pStmt = (sqlite3_stmt*)1234;
  rc = sqlite3_prepare(db, 0, 0, &pStmt, 0);
  إذا( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_prepare";
    اذهب error_out;
  }
  أكد( pStmt==0 ); /* Verify that pStmt is zeroed even on a MISUSE error */

  pStmt = (sqlite3_stmt*)1234;
  rc = sqlite3_prepare_v2(db, 0, 0, &pStmt, 0);
  إذا( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_prepare_v2";
    اذهب error_out;
  }
  أكد( pStmt==0 );

#ifndef SQLITE_OMIT_UTF16
  pStmt = (sqlite3_stmt*)1234;
  rc = sqlite3_prepare16(db, 0, 0, &pStmt, 0);
  إذا( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_prepare16";
    اذهب error_out;
  }
  أكد( pStmt==0 );
  pStmt = (sqlite3_stmt*)1234;
  rc = sqlite3_prepare16_v2(db, 0, 0, &pStmt, 0);
  إذا( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_prepare16_v2";
    اذهب error_out;
  }
  أكد( pStmt==0 );
#endif

  أرجع TCL_OK;

error_out:
  Tcl_ResetResult(interp);
  Tcl_AppendResult(interp, "Error testing function: ", zErrFunction, عدم);
  أرجع TCL_ERROR;
}

/*
** Register commands with the TCL interpreter.
*/
صحيح Sqlitetest9_Init(Tcl_Interp *interp){
  ساكن بنية {
     حرف *zName;
     Tcl_ObjCmdProc *xProc;
     فراغ *clientData;
  } aObjCmd[] = {
     { "c_misuse_test",    c_misuse_test, 0 },
     { "c_realloc_test",   c_realloc_test, 0 },
     { "c_collation_test", c_collation_test, 0 },
  };
  صحيح i;
  لكل(i=0; i<حجم(aObjCmd)/حجم(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
  أرجع TCL_OK;
}
