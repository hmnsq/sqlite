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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI c_collation_test(
  Client_ದತ್ತಾಂಶ client_ದತ್ತಾಂಶ, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zErrFunction = "N/A";
  sqlite3 *db;

  ಪೂರ್ಣಾಂಕ rc;
  ಆದರೆ( objc!=1 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  /* Open a database. */
  rc = sqlite3_open(":memory:", &db);
  ಆದರೆ( rc!=SQLITE_OK ){
    zErrFunction = "sqlite3_open";
    ಹೋಗು error_ಹೊರಹೋಗು;
  }

  rc = sqlite3_create_collation(db, "collate", 456, 0, 0);
  ಆದರೆ( rc!=SQLITE_MISUSE ){
    sqlite3_close(db);
    zErrFunction = "sqlite3_create_collation";
    ಹೋಗು error_ಹೊರಹೋಗು;
  }

  sqlite3_close(db);
  ಹಿಂತಿರುಗಿ TCL_OK;

error_ಹೊರಹೋಗು:
  Tcl_Reset_ಫಲಿತಾಂಶ(interp);
  Tcl_Append_ಫಲಿತಾಂಶ(interp, "Error testing function: ", zErrFunction, ಖಾಲಿ);
  ಹಿಂತಿರುಗಿ TCL_ERROR;
}

/*
** c_realloc_test
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI c_realloc_test(
  Client_ದತ್ತಾಂಶ client_ದತ್ತಾಂಶ, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  ಶೂನ್ಯ *p;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zErrFunction = "N/A";

  ಆದರೆ( objc!=1 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  p = sqlite3_malloc(5);
  ಆದರೆ( !p ){
    zErrFunction = "sqlite3_malloc";
    ಹೋಗು error_ಹೊರಹೋಗು;
  }

  /* Test that realloc()ing a block of memory to a negative size is
  ** the same as free()ing that memory.
  */
  p = sqlite3_realloc(p, -1);
  ಆದರೆ( p ){
    zErrFunction = "sqlite3_realloc";
    ಹೋಗು error_ಹೊರಹೋಗು;
  }

  ಹಿಂತಿರುಗಿ TCL_OK;

error_ಹೊರಹೋಗು:
  Tcl_Reset_ಫಲಿತಾಂಶ(interp);
  Tcl_Append_ಫಲಿತಾಂಶ(interp, "Error testing function: ", zErrFunction, ಖಾಲಿ);
  ಹಿಂತಿರುಗಿ TCL_ERROR;
}


/*
** c_misuse_test
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI c_misuse_test(
  Client_ದತ್ತಾಂಶ client_ದತ್ತಾಂಶ, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zErrFunction = "N/A";
  sqlite3 *db = 0;
  sqlite3_stmt *pStmt;
  ಪೂರ್ಣಾಂಕ rc;

  ಆದರೆ( objc!=1 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  /* Open a database. Then close it again. We need to do this so that
  ** we have a "closed database handle" to pass to various API functions.
  */
  rc = sqlite3_open(":memory:", &db);
  ಆದರೆ( rc!=SQLITE_OK ){
    zErrFunction = "sqlite3_open";
    ಹೋಗು error_ಹೊರಹೋಗು;
  }
  sqlite3_close(db);


  rc = sqlite3_errcode(db);
  ಆದರೆ( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_errcode";
    ಹೋಗು error_ಹೊರಹೋಗು;
  }

  pStmt = (sqlite3_stmt*)1234;
  rc = sqlite3_prepare(db, 0, 0, &pStmt, 0);
  ಆದರೆ( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_prepare";
    ಹೋಗು error_ಹೊರಹೋಗು;
  }
  ಪ್ರತಿಜ್ಞಾ( pStmt==0 ); /* Verify that pStmt is zeroed even on a MISUSE error */

  pStmt = (sqlite3_stmt*)1234;
  rc = sqlite3_prepare_v2(db, 0, 0, &pStmt, 0);
  ಆದರೆ( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_prepare_v2";
    ಹೋಗು error_ಹೊರಹೋಗು;
  }
  ಪ್ರತಿಜ್ಞಾ( pStmt==0 );

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_UTF16
  pStmt = (sqlite3_stmt*)1234;
  rc = sqlite3_prepare16(db, 0, 0, &pStmt, 0);
  ಆದರೆ( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_prepare16";
    ಹೋಗು error_ಹೊರಹೋಗು;
  }
  ಪ್ರತಿಜ್ಞಾ( pStmt==0 );
  pStmt = (sqlite3_stmt*)1234;
  rc = sqlite3_prepare16_v2(db, 0, 0, &pStmt, 0);
  ಆದರೆ( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_prepare16_v2";
    ಹೋಗು error_ಹೊರಹೋಗು;
  }
  ಪ್ರತಿಜ್ಞಾ( pStmt==0 );
#ಆದರೆ_ಅಂತ್ಯ

  ಹಿಂತಿರುಗಿ TCL_OK;

error_ಹೊರಹೋಗು:
  Tcl_Reset_ಫಲಿತಾಂಶ(interp);
  Tcl_Append_ಫಲಿತಾಂಶ(interp, "Error testing function: ", zErrFunction, ಖಾಲಿ);
  ಹಿಂತಿರುಗಿ TCL_ERROR;
}

/*
** Register commands with the TCL interpreter.
*/
ಪೂರ್ಣಾಂಕ Sqlitetest9_Init(Tcl_Interp *interp){
  ಸ್ಥಿರ ರಚನೆ {
     ಅಕ್ಷರ *zName;
     Tcl_ObjCmdProc *xProc;
     ಶೂನ್ಯ *client_ದತ್ತಾಂಶ;
  } aObjCmd[] = {
     { "c_misuse_test",    c_misuse_test, 0 },
     { "c_realloc_test",   c_realloc_test, 0 },
     { "c_collation_test", c_collation_test, 0 },
  };
  ಪೂರ್ಣಾಂಕ i;
  ಗಾಗಿ(i=0; i<ಗಾತ್ರ(aObjCmd)/ಗಾತ್ರ(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].client_ದತ್ತಾಂಶ, 0);
  }
  ಹಿಂತಿರುಗಿ TCL_OK;
}
