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
#அந்தர்பாவ "sqliteInt.h"
#அந்தர்பாவ "tclsqlite.h"
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>

/*
** c_collation_test
*/
நிலையான முழு SQLITE_TCLAPI c_collation_test(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  மாறிலி எழுத்து *zErrFunction = "N/A";
  sqlite3 *db;

  முழு rc;
  எனில்( objc!=1 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "");
    திரும்பு TCL_ERROR;
  }

  /* Open a database. */
  rc = sqlite3_open(":memory:", &db);
  எனில்( rc!=SQLITE_OK ){
    zErrFunction = "sqlite3_open";
    செல் error_out;
  }

  rc = sqlite3_create_collation(db, "collate", 456, 0, 0);
  எனில்( rc!=SQLITE_MISUSE ){
    sqlite3_close(db);
    zErrFunction = "sqlite3_create_collation";
    செல் error_out;
  }

  sqlite3_close(db);
  திரும்பு TCL_OK;

error_out:
  Tcl_ResetResult(interp);
  Tcl_AppendResult(interp, "Error testing function: ", zErrFunction, வெற்று);
  திரும்பு TCL_ERROR;
}

/*
** c_realloc_test
*/
நிலையான முழு SQLITE_TCLAPI c_realloc_test(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  வெறுமை *p;
  மாறிலி எழுத்து *zErrFunction = "N/A";

  எனில்( objc!=1 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "");
    திரும்பு TCL_ERROR;
  }

  p = sqlite3_malloc(5);
  எனில்( !p ){
    zErrFunction = "sqlite3_malloc";
    செல் error_out;
  }

  /* Test that realloc()ing a block of memory to a negative size is
  ** the same as free()ing that memory.
  */
  p = sqlite3_realloc(p, -1);
  எனில்( p ){
    zErrFunction = "sqlite3_realloc";
    செல் error_out;
  }

  திரும்பு TCL_OK;

error_out:
  Tcl_ResetResult(interp);
  Tcl_AppendResult(interp, "Error testing function: ", zErrFunction, வெற்று);
  திரும்பு TCL_ERROR;
}


/*
** c_misuse_test
*/
நிலையான முழு SQLITE_TCLAPI c_misuse_test(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  மாறிலி எழுத்து *zErrFunction = "N/A";
  sqlite3 *db = 0;
  sqlite3_stmt *pStmt;
  முழு rc;

  எனில்( objc!=1 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "");
    திரும்பு TCL_ERROR;
  }

  /* Open a database. Then close it again. We need to do this so that
  ** we have a "closed database handle" to pass to various API functions.
  */
  rc = sqlite3_open(":memory:", &db);
  எனில்( rc!=SQLITE_OK ){
    zErrFunction = "sqlite3_open";
    செல் error_out;
  }
  sqlite3_close(db);


  rc = sqlite3_errcode(db);
  எனில்( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_errcode";
    செல் error_out;
  }

  pStmt = (sqlite3_stmt*)1234;
  rc = sqlite3_prepare(db, 0, 0, &pStmt, 0);
  எனில்( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_prepare";
    செல் error_out;
  }
  ப்ரதிஜ்ஞா( pStmt==0 ); /* Verify that pStmt is zeroed even on a MISUSE error */

  pStmt = (sqlite3_stmt*)1234;
  rc = sqlite3_prepare_v2(db, 0, 0, &pStmt, 0);
  எனில்( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_prepare_v2";
    செல் error_out;
  }
  ப்ரதிஜ்ஞா( pStmt==0 );

#ifndef SQLITE_OMIT_UTF16
  pStmt = (sqlite3_stmt*)1234;
  rc = sqlite3_prepare16(db, 0, 0, &pStmt, 0);
  எனில்( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_prepare16";
    செல் error_out;
  }
  ப்ரதிஜ்ஞா( pStmt==0 );
  pStmt = (sqlite3_stmt*)1234;
  rc = sqlite3_prepare16_v2(db, 0, 0, &pStmt, 0);
  எனில்( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_prepare16_v2";
    செல் error_out;
  }
  ப்ரதிஜ்ஞா( pStmt==0 );
#endif

  திரும்பு TCL_OK;

error_out:
  Tcl_ResetResult(interp);
  Tcl_AppendResult(interp, "Error testing function: ", zErrFunction, வெற்று);
  திரும்பு TCL_ERROR;
}

/*
** Register commands with the TCL interpreter.
*/
முழு Sqlitetest9_Init(Tcl_Interp *interp){
  நிலையான கட்டமைப்பு {
     எழுத்து *zName;
     Tcl_ObjCmdProc *xProc;
     வெறுமை *clientData;
  } aObjCmd[] = {
     { "c_misuse_test",    c_misuse_test, 0 },
     { "c_realloc_test",   c_realloc_test, 0 },
     { "c_collation_test", c_collation_test, 0 },
  };
  முழு i;
  க்கு(i=0; i<அளவு(aObjCmd)/அளவு(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
  திரும்பு TCL_OK;
}
