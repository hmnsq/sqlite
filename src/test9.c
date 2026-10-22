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
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"
#ಅನ್ತರ್ಭಾವ "tclsqlite.h"
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>

/*
** c_collation_test
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI c_collation_test(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  ಧ್ರುವ ವರ್ಣ *zErrFunction = "N/A";
  sqlite3 *db;

  ಪೂರ್ಣಾಙ್ಕ rc;
  ಯದಿ( objc!=1 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  /* Open a database. */
  rc = sqlite3_open(":memory:", &db);
  ಯದಿ( rc!=SQLITE_OK ){
    zErrFunction = "sqlite3_open";
    ಗಚ್ಛ error_out;
  }

  rc = sqlite3_create_collation(db, "collate", 456, 0, 0);
  ಯದಿ( rc!=SQLITE_MISUSE ){
    sqlite3_close(db);
    zErrFunction = "sqlite3_create_collation";
    ಗಚ್ಛ error_out;
  }

  sqlite3_close(db);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;

error_out:
  Tcl_ResetResult(interp);
  Tcl_AppendResult(interp, "Error testing function: ", zErrFunction, ರಿಕ್ತ);
  ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
}

/*
** c_realloc_test
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI c_realloc_test(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  ಶೂನ್ಯ *p;
  ಧ್ರುವ ವರ್ಣ *zErrFunction = "N/A";

  ಯದಿ( objc!=1 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  p = sqlite3_malloc(5);
  ಯದಿ( !p ){
    zErrFunction = "sqlite3_malloc";
    ಗಚ್ಛ error_out;
  }

  /* Test that realloc()ing a block of memory to a negative size is
  ** the same as free()ing that memory.
  */
  p = sqlite3_realloc(p, -1);
  ಯದಿ( p ){
    zErrFunction = "sqlite3_realloc";
    ಗಚ್ಛ error_out;
  }

  ಪ್ರತ್ಯಾಗಮ TCL_OK;

error_out:
  Tcl_ResetResult(interp);
  Tcl_AppendResult(interp, "Error testing function: ", zErrFunction, ರಿಕ್ತ);
  ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
}


/*
** c_misuse_test
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI c_misuse_test(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  ಧ್ರುವ ವರ್ಣ *zErrFunction = "N/A";
  sqlite3 *db = 0;
  sqlite3_stmt *pStmt;
  ಪೂರ್ಣಾಙ್ಕ rc;

  ಯದಿ( objc!=1 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  /* Open a database. Then close it again. We need to do this so that
  ** we have a "closed database handle" to pass to various API functions.
  */
  rc = sqlite3_open(":memory:", &db);
  ಯದಿ( rc!=SQLITE_OK ){
    zErrFunction = "sqlite3_open";
    ಗಚ್ಛ error_out;
  }
  sqlite3_close(db);


  rc = sqlite3_errcode(db);
  ಯದಿ( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_errcode";
    ಗಚ್ಛ error_out;
  }

  pStmt = (sqlite3_stmt*)1234;
  rc = sqlite3_prepare(db, 0, 0, &pStmt, 0);
  ಯದಿ( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_prepare";
    ಗಚ್ಛ error_out;
  }
  ಪ್ರತಿಜ್ಞಾ( pStmt==0 ); /* Verify that pStmt is zeroed even on a MISUSE error */

  pStmt = (sqlite3_stmt*)1234;
  rc = sqlite3_prepare_v2(db, 0, 0, &pStmt, 0);
  ಯದಿ( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_prepare_v2";
    ಗಚ್ಛ error_out;
  }
  ಪ್ರತಿಜ್ಞಾ( pStmt==0 );

#ifndef SQLITE_OMIT_UTF16
  pStmt = (sqlite3_stmt*)1234;
  rc = sqlite3_prepare16(db, 0, 0, &pStmt, 0);
  ಯದಿ( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_prepare16";
    ಗಚ್ಛ error_out;
  }
  ಪ್ರತಿಜ್ಞಾ( pStmt==0 );
  pStmt = (sqlite3_stmt*)1234;
  rc = sqlite3_prepare16_v2(db, 0, 0, &pStmt, 0);
  ಯದಿ( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_prepare16_v2";
    ಗಚ್ಛ error_out;
  }
  ಪ್ರತಿಜ್ಞಾ( pStmt==0 );
#endif

  ಪ್ರತ್ಯಾಗಮ TCL_OK;

error_out:
  Tcl_ResetResult(interp);
  Tcl_AppendResult(interp, "Error testing function: ", zErrFunction, ರಿಕ್ತ);
  ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
}

/*
** Register commands with the TCL interpreter.
*/
ಪೂರ್ಣಾಙ್ಕ Sqlitetest9_Init(Tcl_Interp *interp){
  ಸ್ಥಿರ ಸಂರಚನಾ {
     ವರ್ಣ *zName;
     Tcl_ObjCmdProc *xProc;
     ಶೂನ್ಯ *clientData;
  } aObjCmd[] = {
     { "c_misuse_test",    c_misuse_test, 0 },
     { "c_realloc_test",   c_realloc_test, 0 },
     { "c_collation_test", c_collation_test, 0 },
  };
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪ್ರತಿ(i=0; i<ಪರಿಮಾಣ(aObjCmd)/ಪರಿಮಾಣ(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}
