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
#અન્તર્ભાવ "sqliteInt.h"
#અન્તર્ભાવ "tclsqlite.h"
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>

/*
** c_collation_test
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI c_collation_test(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  અચલ અક્ષર *zErrFunction = "N/A";
  sqlite3 *db;

  પૂર્ણાંક rc;
  જો( objc!=1 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "");
    પાછા TCL_ERROR;
  }

  /* Open a database. */
  rc = sqlite3_open(":memory:", &db);
  જો( rc!=SQLITE_OK ){
    zErrFunction = "sqlite3_open";
    જાઓ error_out;
  }

  rc = sqlite3_create_collation(db, "collate", 456, 0, 0);
  જો( rc!=SQLITE_MISUSE ){
    sqlite3_close(db);
    zErrFunction = "sqlite3_create_collation";
    જાઓ error_out;
  }

  sqlite3_close(db);
  પાછા TCL_OK;

error_out:
  Tcl_ResetResult(interp);
  Tcl_AppendResult(interp, "Error testing function: ", zErrFunction, ખાલી);
  પાછા TCL_ERROR;
}

/*
** c_realloc_test
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI c_realloc_test(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  શૂન્ય *p;
  અચલ અક્ષર *zErrFunction = "N/A";

  જો( objc!=1 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "");
    પાછા TCL_ERROR;
  }

  p = sqlite3_malloc(5);
  જો( !p ){
    zErrFunction = "sqlite3_malloc";
    જાઓ error_out;
  }

  /* Test that realloc()ing a block of memory to a negative size is
  ** the same as free()ing that memory.
  */
  p = sqlite3_realloc(p, -1);
  જો( p ){
    zErrFunction = "sqlite3_realloc";
    જાઓ error_out;
  }

  પાછા TCL_OK;

error_out:
  Tcl_ResetResult(interp);
  Tcl_AppendResult(interp, "Error testing function: ", zErrFunction, ખાલી);
  પાછા TCL_ERROR;
}


/*
** c_misuse_test
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI c_misuse_test(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  અચલ અક્ષર *zErrFunction = "N/A";
  sqlite3 *db = 0;
  sqlite3_stmt *pStmt;
  પૂર્ણાંક rc;

  જો( objc!=1 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "");
    પાછા TCL_ERROR;
  }

  /* Open a database. Then close it again. We need to do this so that
  ** we have a "closed database handle" to pass to various API functions.
  */
  rc = sqlite3_open(":memory:", &db);
  જો( rc!=SQLITE_OK ){
    zErrFunction = "sqlite3_open";
    જાઓ error_out;
  }
  sqlite3_close(db);


  rc = sqlite3_errcode(db);
  જો( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_errcode";
    જાઓ error_out;
  }

  pStmt = (sqlite3_stmt*)1234;
  rc = sqlite3_prepare(db, 0, 0, &pStmt, 0);
  જો( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_prepare";
    જાઓ error_out;
  }
  પ્રતિજ્ઞા( pStmt==0 ); /* Verify that pStmt is zeroed even on a MISUSE error */

  pStmt = (sqlite3_stmt*)1234;
  rc = sqlite3_prepare_v2(db, 0, 0, &pStmt, 0);
  જો( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_prepare_v2";
    જાઓ error_out;
  }
  પ્રતિજ્ઞા( pStmt==0 );

#ifndef SQLITE_OMIT_UTF16
  pStmt = (sqlite3_stmt*)1234;
  rc = sqlite3_prepare16(db, 0, 0, &pStmt, 0);
  જો( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_prepare16";
    જાઓ error_out;
  }
  પ્રતિજ્ઞા( pStmt==0 );
  pStmt = (sqlite3_stmt*)1234;
  rc = sqlite3_prepare16_v2(db, 0, 0, &pStmt, 0);
  જો( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_prepare16_v2";
    જાઓ error_out;
  }
  પ્રતિજ્ઞા( pStmt==0 );
#endif

  પાછા TCL_OK;

error_out:
  Tcl_ResetResult(interp);
  Tcl_AppendResult(interp, "Error testing function: ", zErrFunction, ખાલી);
  પાછા TCL_ERROR;
}

/*
** Register commands with the TCL interpreter.
*/
પૂર્ણાંક Sqlitetest9_Init(Tcl_Interp *interp){
  સ્થિર સંરચના {
     અક્ષર *zName;
     Tcl_ObjCmdProc *xProc;
     શૂન્ય *clientData;
  } aObjCmd[] = {
     { "c_misuse_test",    c_misuse_test, 0 },
     { "c_realloc_test",   c_realloc_test, 0 },
     { "c_collation_test", c_collation_test, 0 },
  };
  પૂર્ણાંક i;
  માટે(i=0; i<કદ(aObjCmd)/કદ(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
  પાછા TCL_OK;
}
