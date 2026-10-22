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
#അന്തര്ഭാവ "sqliteInt.h"
#അന്തര്ഭാവ "tclsqlite.h"
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>

/*
** c_collation_test
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI c_collation_test(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  സ്ഥിരം അക്ഷരം *zErrFunction = "N/A";
  sqlite3 *db;

  പൂർണ്ണസംഖ്യ rc;
  എങ്കിൽ( objc!=1 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "");
    തിരികെ TCL_ERROR;
  }

  /* Open a database. */
  rc = sqlite3_open(":memory:", &db);
  എങ്കിൽ( rc!=SQLITE_OK ){
    zErrFunction = "sqlite3_open";
    പോകുക error_out;
  }

  rc = sqlite3_create_collation(db, "collate", 456, 0, 0);
  എങ്കിൽ( rc!=SQLITE_MISUSE ){
    sqlite3_close(db);
    zErrFunction = "sqlite3_create_collation";
    പോകുക error_out;
  }

  sqlite3_close(db);
  തിരികെ TCL_OK;

error_out:
  Tcl_ResetResult(interp);
  Tcl_AppendResult(interp, "Error testing function: ", zErrFunction, ശൂന്യ);
  തിരികെ TCL_ERROR;
}

/*
** c_realloc_test
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI c_realloc_test(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  ശൂന്യം *p;
  സ്ഥിരം അക്ഷരം *zErrFunction = "N/A";

  എങ്കിൽ( objc!=1 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "");
    തിരികെ TCL_ERROR;
  }

  p = sqlite3_malloc(5);
  എങ്കിൽ( !p ){
    zErrFunction = "sqlite3_malloc";
    പോകുക error_out;
  }

  /* Test that realloc()ing a block of memory to a negative size is
  ** the same as free()ing that memory.
  */
  p = sqlite3_realloc(p, -1);
  എങ്കിൽ( p ){
    zErrFunction = "sqlite3_realloc";
    പോകുക error_out;
  }

  തിരികെ TCL_OK;

error_out:
  Tcl_ResetResult(interp);
  Tcl_AppendResult(interp, "Error testing function: ", zErrFunction, ശൂന്യ);
  തിരികെ TCL_ERROR;
}


/*
** c_misuse_test
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI c_misuse_test(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  സ്ഥിരം അക്ഷരം *zErrFunction = "N/A";
  sqlite3 *db = 0;
  sqlite3_stmt *pStmt;
  പൂർണ്ണസംഖ്യ rc;

  എങ്കിൽ( objc!=1 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "");
    തിരികെ TCL_ERROR;
  }

  /* Open a database. Then close it again. We need to do this so that
  ** we have a "closed database handle" to pass to various API functions.
  */
  rc = sqlite3_open(":memory:", &db);
  എങ്കിൽ( rc!=SQLITE_OK ){
    zErrFunction = "sqlite3_open";
    പോകുക error_out;
  }
  sqlite3_close(db);


  rc = sqlite3_errcode(db);
  എങ്കിൽ( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_errcode";
    പോകുക error_out;
  }

  pStmt = (sqlite3_stmt*)1234;
  rc = sqlite3_prepare(db, 0, 0, &pStmt, 0);
  എങ്കിൽ( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_prepare";
    പോകുക error_out;
  }
  പ്രതിജ്ഞാ( pStmt==0 ); /* Verify that pStmt is zeroed even on a MISUSE error */

  pStmt = (sqlite3_stmt*)1234;
  rc = sqlite3_prepare_v2(db, 0, 0, &pStmt, 0);
  എങ്കിൽ( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_prepare_v2";
    പോകുക error_out;
  }
  പ്രതിജ്ഞാ( pStmt==0 );

#ifndef SQLITE_OMIT_UTF16
  pStmt = (sqlite3_stmt*)1234;
  rc = sqlite3_prepare16(db, 0, 0, &pStmt, 0);
  എങ്കിൽ( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_prepare16";
    പോകുക error_out;
  }
  പ്രതിജ്ഞാ( pStmt==0 );
  pStmt = (sqlite3_stmt*)1234;
  rc = sqlite3_prepare16_v2(db, 0, 0, &pStmt, 0);
  എങ്കിൽ( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_prepare16_v2";
    പോകുക error_out;
  }
  പ്രതിജ്ഞാ( pStmt==0 );
#endif

  തിരികെ TCL_OK;

error_out:
  Tcl_ResetResult(interp);
  Tcl_AppendResult(interp, "Error testing function: ", zErrFunction, ശൂന്യ);
  തിരികെ TCL_ERROR;
}

/*
** Register commands with the TCL interpreter.
*/
പൂർണ്ണസംഖ്യ Sqlitetest9_Init(Tcl_Interp *interp){
  സ്ഥിതം ഘടന {
     അക്ഷരം *zName;
     Tcl_ObjCmdProc *xProc;
     ശൂന്യം *clientData;
  } aObjCmd[] = {
     { "c_misuse_test",    c_misuse_test, 0 },
     { "c_realloc_test",   c_realloc_test, 0 },
     { "c_collation_test", c_collation_test, 0 },
  };
  പൂർണ്ണസംഖ്യ i;
  വേണ്ടി(i=0; i<വലുപ്പം(aObjCmd)/വലുപ്പം(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
  തിരികെ TCL_OK;
}
