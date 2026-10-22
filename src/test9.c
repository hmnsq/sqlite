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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "tclsqlite.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>

/*
** c_collation_test
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI c_collation_test(
  Client_ਅੰਕੜੇ client_ਅੰਕੜੇ, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  ਅਚਲ ਅੱਖਰ *zErrFunction = "N/A";
  sqlite3 *db;

  ਪੂਰਨ rc;
  ਜੇ( objc!=1 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "");
    ਵਾਪਸ TCL_ERROR;
  }

  /* Open a database. */
  rc = sqlite3_open(":memory:", &db);
  ਜੇ( rc!=SQLITE_OK ){
    zErrFunction = "sqlite3_open";
    ਜਾਓ error_ਬਾਹਰਲਾ;
  }

  rc = sqlite3_create_collation(db, "collate", 456, 0, 0);
  ਜੇ( rc!=SQLITE_MISUSE ){
    sqlite3_close(db);
    zErrFunction = "sqlite3_create_collation";
    ਜਾਓ error_ਬਾਹਰਲਾ;
  }

  sqlite3_close(db);
  ਵਾਪਸ TCL_OK;

error_ਬਾਹਰਲਾ:
  Tcl_Reset_ਨਤੀਜਾ(interp);
  Tcl_Append_ਨਤੀਜਾ(interp, "Error testing function: ", zErrFunction, ਨਿਰਾ);
  ਵਾਪਸ TCL_ERROR;
}

/*
** c_realloc_test
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI c_realloc_test(
  Client_ਅੰਕੜੇ client_ਅੰਕੜੇ, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  ਖਾਲੀ *p;
  ਅਚਲ ਅੱਖਰ *zErrFunction = "N/A";

  ਜੇ( objc!=1 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "");
    ਵਾਪਸ TCL_ERROR;
  }

  p = sqlite3_malloc(5);
  ਜੇ( !p ){
    zErrFunction = "sqlite3_malloc";
    ਜਾਓ error_ਬਾਹਰਲਾ;
  }

  /* Test that realloc()ing a block of memory to a negative size is
  ** the same as free()ing that memory.
  */
  p = sqlite3_realloc(p, -1);
  ਜੇ( p ){
    zErrFunction = "sqlite3_realloc";
    ਜਾਓ error_ਬਾਹਰਲਾ;
  }

  ਵਾਪਸ TCL_OK;

error_ਬਾਹਰਲਾ:
  Tcl_Reset_ਨਤੀਜਾ(interp);
  Tcl_Append_ਨਤੀਜਾ(interp, "Error testing function: ", zErrFunction, ਨਿਰਾ);
  ਵਾਪਸ TCL_ERROR;
}


/*
** c_misuse_test
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI c_misuse_test(
  Client_ਅੰਕੜੇ client_ਅੰਕੜੇ, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  ਅਚਲ ਅੱਖਰ *zErrFunction = "N/A";
  sqlite3 *db = 0;
  sqlite3_stmt *pStmt;
  ਪੂਰਨ rc;

  ਜੇ( objc!=1 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "");
    ਵਾਪਸ TCL_ERROR;
  }

  /* Open a database. Then close it again. We need to do this so that
  ** we have a "closed database handle" to pass to various API functions.
  */
  rc = sqlite3_open(":memory:", &db);
  ਜੇ( rc!=SQLITE_OK ){
    zErrFunction = "sqlite3_open";
    ਜਾਓ error_ਬਾਹਰਲਾ;
  }
  sqlite3_close(db);


  rc = sqlite3_errcode(db);
  ਜੇ( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_errcode";
    ਜਾਓ error_ਬਾਹਰਲਾ;
  }

  pStmt = (sqlite3_stmt*)1234;
  rc = sqlite3_prepare(db, 0, 0, &pStmt, 0);
  ਜੇ( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_prepare";
    ਜਾਓ error_ਬਾਹਰਲਾ;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( pStmt==0 ); /* Verify that pStmt is zeroed even on a MISUSE error */

  pStmt = (sqlite3_stmt*)1234;
  rc = sqlite3_prepare_v2(db, 0, 0, &pStmt, 0);
  ਜੇ( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_prepare_v2";
    ਜਾਓ error_ਬਾਹਰਲਾ;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( pStmt==0 );

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_UTF16
  pStmt = (sqlite3_stmt*)1234;
  rc = sqlite3_prepare16(db, 0, 0, &pStmt, 0);
  ਜੇ( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_prepare16";
    ਜਾਓ error_ਬਾਹਰਲਾ;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( pStmt==0 );
  pStmt = (sqlite3_stmt*)1234;
  rc = sqlite3_prepare16_v2(db, 0, 0, &pStmt, 0);
  ਜੇ( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_prepare16_v2";
    ਜਾਓ error_ਬਾਹਰਲਾ;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( pStmt==0 );
#ਜੇ_ਅੰਤ

  ਵਾਪਸ TCL_OK;

error_ਬਾਹਰਲਾ:
  Tcl_Reset_ਨਤੀਜਾ(interp);
  Tcl_Append_ਨਤੀਜਾ(interp, "Error testing function: ", zErrFunction, ਨਿਰਾ);
  ਵਾਪਸ TCL_ERROR;
}

/*
** Register commands with the TCL interpreter.
*/
ਪੂਰਨ Sqlitetest9_Init(Tcl_Interp *interp){
  ਸਥਿਰ ਬਣਤਰ {
     ਅੱਖਰ *zName;
     Tcl_ObjCmdProc *xProc;
     ਖਾਲੀ *client_ਅੰਕੜੇ;
  } aObjCmd[] = {
     { "c_misuse_test",    c_misuse_test, 0 },
     { "c_realloc_test",   c_realloc_test, 0 },
     { "c_collation_test", c_collation_test, 0 },
  };
  ਪੂਰਨ i;
  ਲਈ(i=0; i<ਆਕਾਰ(aObjCmd)/ਆਕਾਰ(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].client_ਅੰਕੜੇ, 0);
  }
  ਵਾਪਸ TCL_OK;
}
