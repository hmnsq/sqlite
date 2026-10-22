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
#অন্তর্ভাব "sqliteInt.h"
#অন্তর্ভাব "tclsqlite.h"
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>

/*
** c_collation_test
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI c_collation_test(
  Client_তথ্য client_তথ্য, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  ধ্রুবক অক্ষর *zErrFunction = "N/A";
  sqlite3 *db;

  পূর্ণসংখ্যা rc;
  যদি( objc!=1 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "");
    ফেরত TCL_ERROR;
  }

  /* Open a database. */
  rc = sqlite3_open(":memory:", &db);
  যদি( rc!=SQLITE_OK ){
    zErrFunction = "sqlite3_open";
    যাও error_out;
  }

  rc = sqlite3_create_collation(db, "collate", 456, 0, 0);
  যদি( rc!=SQLITE_MISUSE ){
    sqlite3_close(db);
    zErrFunction = "sqlite3_create_collation";
    যাও error_out;
  }

  sqlite3_close(db);
  ফেরত TCL_OK;

error_out:
  Tcl_Reset_ফলাফল(interp);
  Tcl_Append_ফলাফল(interp, "Error testing function: ", zErrFunction, ফাঁকা);
  ফেরত TCL_ERROR;
}

/*
** c_realloc_test
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI c_realloc_test(
  Client_তথ্য client_তথ্য, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  শূন্য *p;
  ধ্রুবক অক্ষর *zErrFunction = "N/A";

  যদি( objc!=1 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "");
    ফেরত TCL_ERROR;
  }

  p = sqlite3_malloc(5);
  যদি( !p ){
    zErrFunction = "sqlite3_malloc";
    যাও error_out;
  }

  /* Test that realloc()ing a block of memory to a negative size is
  ** the same as free()ing that memory.
  */
  p = sqlite3_realloc(p, -1);
  যদি( p ){
    zErrFunction = "sqlite3_realloc";
    যাও error_out;
  }

  ফেরত TCL_OK;

error_out:
  Tcl_Reset_ফলাফল(interp);
  Tcl_Append_ফলাফল(interp, "Error testing function: ", zErrFunction, ফাঁকা);
  ফেরত TCL_ERROR;
}


/*
** c_misuse_test
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI c_misuse_test(
  Client_তথ্য client_তথ্য, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  ধ্রুবক অক্ষর *zErrFunction = "N/A";
  sqlite3 *db = 0;
  sqlite3_stmt *pStmt;
  পূর্ণসংখ্যা rc;

  যদি( objc!=1 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "");
    ফেরত TCL_ERROR;
  }

  /* Open a database. Then close it again. We need to do this so that
  ** we have a "closed database handle" to pass to various API functions.
  */
  rc = sqlite3_open(":memory:", &db);
  যদি( rc!=SQLITE_OK ){
    zErrFunction = "sqlite3_open";
    যাও error_out;
  }
  sqlite3_close(db);


  rc = sqlite3_errcode(db);
  যদি( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_errcode";
    যাও error_out;
  }

  pStmt = (sqlite3_stmt*)1234;
  rc = sqlite3_prepare(db, 0, 0, &pStmt, 0);
  যদি( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_prepare";
    যাও error_out;
  }
  প্রতিজ্ঞা( pStmt==0 ); /* Verify that pStmt is zeroed even on a MISUSE error */

  pStmt = (sqlite3_stmt*)1234;
  rc = sqlite3_prepare_v2(db, 0, 0, &pStmt, 0);
  যদি( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_prepare_v2";
    যাও error_out;
  }
  প্রতিজ্ঞা( pStmt==0 );

#ifndef SQLITE_OMIT_UTF16
  pStmt = (sqlite3_stmt*)1234;
  rc = sqlite3_prepare16(db, 0, 0, &pStmt, 0);
  যদি( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_prepare16";
    যাও error_out;
  }
  প্রতিজ্ঞা( pStmt==0 );
  pStmt = (sqlite3_stmt*)1234;
  rc = sqlite3_prepare16_v2(db, 0, 0, &pStmt, 0);
  যদি( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_prepare16_v2";
    যাও error_out;
  }
  প্রতিজ্ঞা( pStmt==0 );
#endif

  ফেরত TCL_OK;

error_out:
  Tcl_Reset_ফলাফল(interp);
  Tcl_Append_ফলাফল(interp, "Error testing function: ", zErrFunction, ফাঁকা);
  ফেরত TCL_ERROR;
}

/*
** Register commands with the TCL interpreter.
*/
পূর্ণসংখ্যা Sqlitetest9_Init(Tcl_Interp *interp){
  স্থির কাঠামো {
     অক্ষর *zName;
     Tcl_ObjCmdProc *xProc;
     শূন্য *client_তথ্য;
  } aObjCmd[] = {
     { "c_misuse_test",    c_misuse_test, 0 },
     { "c_realloc_test",   c_realloc_test, 0 },
     { "c_collation_test", c_collation_test, 0 },
  };
  পূর্ণসংখ্যা i;
  জন্য(i=0; i<আকার(aObjCmd)/আকার(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].client_তথ্য, 0);
  }
  ফেরত TCL_OK;
}
