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
#అన్తర్భావ "sqliteInt.h"
#అన్తర్భావ "tclsqlite.h"
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>

/*
** c_collation_test
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI c_collation_test(
  Client_దత్తాంశం client_దత్తాంశం, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  స్థిరరాశి అక్షరం *zErrFunction = "N/A";
  sqlite3 *db;

  పూర్ణాంకం rc;
  ఒకవేళ( objc!=1 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "");
    తిరిగి TCL_ERROR;
  }

  /* Open a database. */
  rc = sqlite3_open(":memory:", &db);
  ఒకవేళ( rc!=SQLITE_OK ){
    zErrFunction = "sqlite3_open";
    వెళ్ళు error_బహిర్గతం;
  }

  rc = sqlite3_create_collation(db, "collate", 456, 0, 0);
  ఒకవేళ( rc!=SQLITE_MISUSE ){
    sqlite3_close(db);
    zErrFunction = "sqlite3_create_collation";
    వెళ్ళు error_బహిర్గతం;
  }

  sqlite3_close(db);
  తిరిగి TCL_OK;

error_బహిర్గతం:
  Tcl_Reset_ఫలితం(interp);
  Tcl_Append_ఫలితం(interp, "Error testing function: ", zErrFunction, ఖాళీ);
  తిరిగి TCL_ERROR;
}

/*
** c_realloc_test
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI c_realloc_test(
  Client_దత్తాంశం client_దత్తాంశం, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  శూన్య *p;
  స్థిరరాశి అక్షరం *zErrFunction = "N/A";

  ఒకవేళ( objc!=1 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "");
    తిరిగి TCL_ERROR;
  }

  p = sqlite3_malloc(5);
  ఒకవేళ( !p ){
    zErrFunction = "sqlite3_malloc";
    వెళ్ళు error_బహిర్గతం;
  }

  /* Test that realloc()ing a block of memory to a negative size is
  ** the same as free()ing that memory.
  */
  p = sqlite3_realloc(p, -1);
  ఒకవేళ( p ){
    zErrFunction = "sqlite3_realloc";
    వెళ్ళు error_బహిర్గతం;
  }

  తిరిగి TCL_OK;

error_బహిర్గతం:
  Tcl_Reset_ఫలితం(interp);
  Tcl_Append_ఫలితం(interp, "Error testing function: ", zErrFunction, ఖాళీ);
  తిరిగి TCL_ERROR;
}


/*
** c_misuse_test
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI c_misuse_test(
  Client_దత్తాంశం client_దత్తాంశం, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  స్థిరరాశి అక్షరం *zErrFunction = "N/A";
  sqlite3 *db = 0;
  sqlite3_stmt *pStmt;
  పూర్ణాంకం rc;

  ఒకవేళ( objc!=1 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "");
    తిరిగి TCL_ERROR;
  }

  /* Open a database. Then close it again. We need to do this so that
  ** we have a "closed database handle" to pass to various API functions.
  */
  rc = sqlite3_open(":memory:", &db);
  ఒకవేళ( rc!=SQLITE_OK ){
    zErrFunction = "sqlite3_open";
    వెళ్ళు error_బహిర్గతం;
  }
  sqlite3_close(db);


  rc = sqlite3_errcode(db);
  ఒకవేళ( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_errcode";
    వెళ్ళు error_బహిర్గతం;
  }

  pStmt = (sqlite3_stmt*)1234;
  rc = sqlite3_prepare(db, 0, 0, &pStmt, 0);
  ఒకవేళ( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_prepare";
    వెళ్ళు error_బహిర్గతం;
  }
  ప్రతిజ్ఞా( pStmt==0 ); /* Verify that pStmt is zeroed even on a MISUSE error */

  pStmt = (sqlite3_stmt*)1234;
  rc = sqlite3_prepare_v2(db, 0, 0, &pStmt, 0);
  ఒకవేళ( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_prepare_v2";
    వెళ్ళు error_బహిర్గతం;
  }
  ప్రతిజ్ఞా( pStmt==0 );

#నిర్వచితం_కాకపోతే SQLITE_OMIT_UTF16
  pStmt = (sqlite3_stmt*)1234;
  rc = sqlite3_prepare16(db, 0, 0, &pStmt, 0);
  ఒకవేళ( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_prepare16";
    వెళ్ళు error_బహిర్గతం;
  }
  ప్రతిజ్ఞా( pStmt==0 );
  pStmt = (sqlite3_stmt*)1234;
  rc = sqlite3_prepare16_v2(db, 0, 0, &pStmt, 0);
  ఒకవేళ( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_prepare16_v2";
    వెళ్ళు error_బహిర్గతం;
  }
  ప్రతిజ్ఞా( pStmt==0 );
#అయితే_ముగింపు

  తిరిగి TCL_OK;

error_బహిర్గతం:
  Tcl_Reset_ఫలితం(interp);
  Tcl_Append_ఫలితం(interp, "Error testing function: ", zErrFunction, ఖాళీ);
  తిరిగి TCL_ERROR;
}

/*
** Register commands with the TCL interpreter.
*/
పూర్ణాంకం Sqlitetest9_Init(Tcl_Interp *interp){
  స్థిర నిర్మాణం {
     అక్షరం *zName;
     Tcl_ObjCmdProc *xProc;
     శూన్య *client_దత్తాంశం;
  } aObjCmd[] = {
     { "c_misuse_test",    c_misuse_test, 0 },
     { "c_realloc_test",   c_realloc_test, 0 },
     { "c_collation_test", c_collation_test, 0 },
  };
  పూర్ణాంకం i;
  కొరకు(i=0; i<పరిమాణం(aObjCmd)/పరిమాణం(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].client_దత్తాంశం, 0);
  }
  తిరిగి TCL_OK;
}
