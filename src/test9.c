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
#समावेश "sqliteInt.h"
#समावेश "tclsqlite.h"
#समावेश <stdlib.h>
#समावेश <string.h>

/*
** c_collation_test
*/
अटल पूर्णांक SQLITE_TCLAPI c_collation_test(
  Client_तथ्य client_तथ्य, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  स्थिर अक्षर *zErrFunction = "N/A";
  sqlite3 *db;

  पूर्णांक rc;
  अगर( objc!=1 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "");
    वापस TCL_ERROR;
  }

  /* Open a database. */
  rc = sqlite3_open(":memory:", &db);
  अगर( rc!=SQLITE_OK ){
    zErrFunction = "sqlite3_open";
    जाओ error_out;
  }

  rc = sqlite3_create_collation(db, "collate", 456, 0, 0);
  अगर( rc!=SQLITE_MISUSE ){
    sqlite3_close(db);
    zErrFunction = "sqlite3_create_collation";
    जाओ error_out;
  }

  sqlite3_close(db);
  वापस TCL_OK;

error_out:
  Tcl_Reset_परिणाम(interp);
  Tcl_Append_परिणाम(interp, "Error testing function: ", zErrFunction, शून्य);
  वापस TCL_ERROR;
}

/*
** c_realloc_test
*/
अटल पूर्णांक SQLITE_TCLAPI c_realloc_test(
  Client_तथ्य client_तथ्य, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  व्योम *p;
  स्थिर अक्षर *zErrFunction = "N/A";

  अगर( objc!=1 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "");
    वापस TCL_ERROR;
  }

  p = sqlite3_malloc(5);
  अगर( !p ){
    zErrFunction = "sqlite3_malloc";
    जाओ error_out;
  }

  /* Test that realloc()ing a block of memory to a negative size is
  ** the same as free()ing that memory.
  */
  p = sqlite3_realloc(p, -1);
  अगर( p ){
    zErrFunction = "sqlite3_realloc";
    जाओ error_out;
  }

  वापस TCL_OK;

error_out:
  Tcl_Reset_परिणाम(interp);
  Tcl_Append_परिणाम(interp, "Error testing function: ", zErrFunction, शून्य);
  वापस TCL_ERROR;
}


/*
** c_misuse_test
*/
अटल पूर्णांक SQLITE_TCLAPI c_misuse_test(
  Client_तथ्य client_तथ्य, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  स्थिर अक्षर *zErrFunction = "N/A";
  sqlite3 *db = 0;
  sqlite3_stmt *pStmt;
  पूर्णांक rc;

  अगर( objc!=1 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "");
    वापस TCL_ERROR;
  }

  /* Open a database. Then close it again. We need to do this so that
  ** we have a "closed database handle" to pass to various API functions.
  */
  rc = sqlite3_open(":memory:", &db);
  अगर( rc!=SQLITE_OK ){
    zErrFunction = "sqlite3_open";
    जाओ error_out;
  }
  sqlite3_close(db);


  rc = sqlite3_errcode(db);
  अगर( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_errcode";
    जाओ error_out;
  }

  pStmt = (sqlite3_stmt*)1234;
  rc = sqlite3_prepare(db, 0, 0, &pStmt, 0);
  अगर( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_prepare";
    जाओ error_out;
  }
  निश्चित( pStmt==0 ); /* Verify that pStmt is zeroed even on a MISUSE error */

  pStmt = (sqlite3_stmt*)1234;
  rc = sqlite3_prepare_v2(db, 0, 0, &pStmt, 0);
  अगर( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_prepare_v2";
    जाओ error_out;
  }
  निश्चित( pStmt==0 );

#अगर_अघोषित SQLITE_OMIT_UTF16
  pStmt = (sqlite3_stmt*)1234;
  rc = sqlite3_prepare16(db, 0, 0, &pStmt, 0);
  अगर( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_prepare16";
    जाओ error_out;
  }
  निश्चित( pStmt==0 );
  pStmt = (sqlite3_stmt*)1234;
  rc = sqlite3_prepare16_v2(db, 0, 0, &pStmt, 0);
  अगर( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_prepare16_v2";
    जाओ error_out;
  }
  निश्चित( pStmt==0 );
#पूर्ण_अगर

  वापस TCL_OK;

error_out:
  Tcl_Reset_परिणाम(interp);
  Tcl_Append_परिणाम(interp, "Error testing function: ", zErrFunction, शून्य);
  वापस TCL_ERROR;
}

/*
** Register commands with the TCL interpreter.
*/
पूर्णांक Sqlitetest9_Init(Tcl_Interp *interp){
  अटल काष्ठा {
     अक्षर *zName;
     Tcl_ObjCmdProc *xProc;
     व्योम *client_तथ्य;
  } aObjCmd[] = {
     { "c_misuse_test",    c_misuse_test, 0 },
     { "c_realloc_test",   c_realloc_test, 0 },
     { "c_collation_test", c_collation_test, 0 },
  };
  पूर्णांक i;
  क्रम(i=0; i<माप(aObjCmd)/माप(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].client_तथ्य, 0);
  }
  वापस TCL_OK;
}
