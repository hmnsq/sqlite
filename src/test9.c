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
#شامل "sqliteInt.h"
#شامل "tclsqlite.h"
#شامل <stdlib.h>
#شامل <string.h>

/*
** c_collation_test
*/
ساکن صحیح SQLITE_TCLAPI c_collation_test(
  Client_ڈیٹا client_ڈیٹا, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  ثابت حرف *zErrFunction = "N/A";
  sqlite3 *db;

  صحیح rc;
  اگر( objc!=1 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "");
    واپس TCL_ERROR;
  }

  /* Open a database. */
  rc = sqlite3_open(":memory:", &db);
  اگر( rc!=SQLITE_OK ){
    zErrFunction = "sqlite3_open";
    جاؤ error_برآمد;
  }

  rc = sqlite3_create_collation(db, "collate", 456, 0, 0);
  اگر( rc!=SQLITE_MISUSE ){
    sqlite3_close(db);
    zErrFunction = "sqlite3_create_collation";
    جاؤ error_برآمد;
  }

  sqlite3_close(db);
  واپس TCL_OK;

error_برآمد:
  Tcl_Reset_نتیجہ(interp);
  Tcl_Append_نتیجہ(interp, "Error testing function: ", zErrFunction, تہی);
  واپس TCL_ERROR;
}

/*
** c_realloc_test
*/
ساکن صحیح SQLITE_TCLAPI c_realloc_test(
  Client_ڈیٹا client_ڈیٹا, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  خالی *p;
  ثابت حرف *zErrFunction = "N/A";

  اگر( objc!=1 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "");
    واپس TCL_ERROR;
  }

  p = sqlite3_malloc(5);
  اگر( !p ){
    zErrFunction = "sqlite3_malloc";
    جاؤ error_برآمد;
  }

  /* Test that realloc()ing a block of memory to a negative size is
  ** the same as free()ing that memory.
  */
  p = sqlite3_realloc(p, -1);
  اگر( p ){
    zErrFunction = "sqlite3_realloc";
    جاؤ error_برآمد;
  }

  واپس TCL_OK;

error_برآمد:
  Tcl_Reset_نتیجہ(interp);
  Tcl_Append_نتیجہ(interp, "Error testing function: ", zErrFunction, تہی);
  واپس TCL_ERROR;
}


/*
** c_misuse_test
*/
ساکن صحیح SQLITE_TCLAPI c_misuse_test(
  Client_ڈیٹا client_ڈیٹا, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  ثابت حرف *zErrFunction = "N/A";
  sqlite3 *db = 0;
  sqlite3_stmt *pStmt;
  صحیح rc;

  اگر( objc!=1 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "");
    واپس TCL_ERROR;
  }

  /* Open a database. Then close it again. We need to do this so that
  ** we have a "closed database handle" to pass to various API functions.
  */
  rc = sqlite3_open(":memory:", &db);
  اگر( rc!=SQLITE_OK ){
    zErrFunction = "sqlite3_open";
    جاؤ error_برآمد;
  }
  sqlite3_close(db);


  rc = sqlite3_errcode(db);
  اگر( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_errcode";
    جاؤ error_برآمد;
  }

  pStmt = (sqlite3_stmt*)1234;
  rc = sqlite3_prepare(db, 0, 0, &pStmt, 0);
  اگر( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_prepare";
    جاؤ error_برآمد;
  }
  أكد( pStmt==0 ); /* Verify that pStmt is zeroed even on a MISUSE error */

  pStmt = (sqlite3_stmt*)1234;
  rc = sqlite3_prepare_v2(db, 0, 0, &pStmt, 0);
  اگر( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_prepare_v2";
    جاؤ error_برآمد;
  }
  أكد( pStmt==0 );

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_UTF16
  pStmt = (sqlite3_stmt*)1234;
  rc = sqlite3_prepare16(db, 0, 0, &pStmt, 0);
  اگر( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_prepare16";
    جاؤ error_برآمد;
  }
  أكد( pStmt==0 );
  pStmt = (sqlite3_stmt*)1234;
  rc = sqlite3_prepare16_v2(db, 0, 0, &pStmt, 0);
  اگر( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_prepare16_v2";
    جاؤ error_برآمد;
  }
  أكد( pStmt==0 );
#اگر_ختم

  واپس TCL_OK;

error_برآمد:
  Tcl_Reset_نتیجہ(interp);
  Tcl_Append_نتیجہ(interp, "Error testing function: ", zErrFunction, تہی);
  واپس TCL_ERROR;
}

/*
** Register commands with the TCL interpreter.
*/
صحیح Sqlitetest9_Init(Tcl_Interp *interp){
  ساکن ساخت {
     حرف *zName;
     Tcl_ObjCmdProc *xProc;
     خالی *client_ڈیٹا;
  } aObjCmd[] = {
     { "c_misuse_test",    c_misuse_test, 0 },
     { "c_realloc_test",   c_realloc_test, 0 },
     { "c_collation_test", c_collation_test, 0 },
  };
  صحیح i;
  برائے(i=0; i<حجم(aObjCmd)/حجم(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].client_ڈیٹا, 0);
  }
  واپس TCL_OK;
}
