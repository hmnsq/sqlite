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
#include "sqliteInt.h"
#include "tclsqlite.h"
#include <stdlib.h>
#include <string.h>

/*
** c_collation_test
*/
static puur_naa_nka SQLITE_TCLAPI c_collation_test(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  puur_naa_nka objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  const var_na *zErrFunction = "N/A";
  sqlite3 *db;

  puur_naa_nka rc;
  ᰀᰧ( objc!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    ᰀᰧᰌ TCL_ERROR;
  }

  /* Open a database. */
  rc = sqlite3_open(":memory:", &db);
  ᰀᰧ( rc!=SQLITE_OK ){
    zErrFunction = "sqlite3_open";
    goto error_out;
  }

  rc = sqlite3_create_collation(db, "collate", 456, 0, 0);
  ᰀᰧ( rc!=SQLITE_MISUSE ){
    sqlite3_close(db);
    zErrFunction = "sqlite3_create_collation";
    goto error_out;
  }

  sqlite3_close(db);
  ᰀᰧᰌ TCL_OK;

error_out:
  Tcl_ResetResult(interp);
  Tcl_AppendResult(interp, "Error testing function: ", zErrFunction, NULL);
  ᰀᰧᰌ TCL_ERROR;
}

/*
** c_realloc_test
*/
static puur_naa_nka SQLITE_TCLAPI c_realloc_test(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  puur_naa_nka objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  void *p;
  const var_na *zErrFunction = "N/A";

  ᰀᰧ( objc!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    ᰀᰧᰌ TCL_ERROR;
  }

  p = sqlite3_malloc(5);
  ᰀᰧ( !p ){
    zErrFunction = "sqlite3_malloc";
    goto error_out;
  }

  /* Test that realloc()ing a block of memory to a negative size is
  ** the same as free()ing that memory.
  */
  p = sqlite3_realloc(p, -1);
  ᰀᰧ( p ){
    zErrFunction = "sqlite3_realloc";
    goto error_out;
  }

  ᰀᰧᰌ TCL_OK;

error_out:
  Tcl_ResetResult(interp);
  Tcl_AppendResult(interp, "Error testing function: ", zErrFunction, NULL);
  ᰀᰧᰌ TCL_ERROR;
}


/*
** c_misuse_test
*/
static puur_naa_nka SQLITE_TCLAPI c_misuse_test(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  puur_naa_nka objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  const var_na *zErrFunction = "N/A";
  sqlite3 *db = 0;
  sqlite3_stmt *pStmt;
  puur_naa_nka rc;

  ᰀᰧ( objc!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    ᰀᰧᰌ TCL_ERROR;
  }

  /* Open a database. Then close it again. We need to do this so that
  ** we have a "closed database handle" to pass to various API functions.
  */
  rc = sqlite3_open(":memory:", &db);
  ᰀᰧ( rc!=SQLITE_OK ){
    zErrFunction = "sqlite3_open";
    goto error_out;
  }
  sqlite3_close(db);


  rc = sqlite3_errcode(db);
  ᰀᰧ( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_errcode";
    goto error_out;
  }

  pStmt = (sqlite3_stmt*)1234;
  rc = sqlite3_prepare(db, 0, 0, &pStmt, 0);
  ᰀᰧ( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_prepare";
    goto error_out;
  }
  assert( pStmt==0 ); /* Verify that pStmt is zeroed even on a MISUSE error */

  pStmt = (sqlite3_stmt*)1234;
  rc = sqlite3_prepare_v2(db, 0, 0, &pStmt, 0);
  ᰀᰧ( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_prepare_v2";
    goto error_out;
  }
  assert( pStmt==0 );

#ifndef SQLITE_OMIT_UTF16
  pStmt = (sqlite3_stmt*)1234;
  rc = sqlite3_prepare16(db, 0, 0, &pStmt, 0);
  ᰀᰧ( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_prepare16";
    goto error_out;
  }
  assert( pStmt==0 );
  pStmt = (sqlite3_stmt*)1234;
  rc = sqlite3_prepare16_v2(db, 0, 0, &pStmt, 0);
  ᰀᰧ( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_prepare16_v2";
    goto error_out;
  }
  assert( pStmt==0 );
#endif

  ᰀᰧᰌ TCL_OK;

error_out:
  Tcl_ResetResult(interp);
  Tcl_AppendResult(interp, "Error testing function: ", zErrFunction, NULL);
  ᰀᰧᰌ TCL_ERROR;
}

/*
** Register commands with the TCL interpreter.
*/
puur_naa_nka Sqlitetest9_Init(Tcl_Interp *interp){
  static sa_nracanaa {
     var_na *zName;
     Tcl_ObjCmdProc *xProc;
     void *clientData;
  } aObjCmd[] = {
     { "c_misuse_test",    c_misuse_test, 0 },
     { "c_realloc_test",   c_realloc_test, 0 },
     { "c_collation_test", c_collation_test, 0 },
  };
  puur_naa_nka i;
  for(i=0; i<parimaa_na(aObjCmd)/parimaa_na(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
  ᰀᰧᰌ TCL_OK;
}
