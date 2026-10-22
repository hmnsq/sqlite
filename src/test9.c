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
#अन्तर्भाव "sqliteInt.h"
#अन्तर्भाव "tclsqlite.h"
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>

/*
** c_collation_test
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI c_collation_test(
  Client_दत्त client_दत्त, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  ध्रुव वर्ण *zErrFunction = "N/A";
  sqlite3 *db;

  पूर्णाङ्क rc;
  यदि( objc!=1 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "");
    प्रत्यागम TCL_ERROR;
  }

  /* Open a database. */
  rc = sqlite3_open(":memory:", &db);
  यदि( rc!=SQLITE_OK ){
    zErrFunction = "sqlite3_open";
    गच्छ error_बहिर्गमनम्;
  }

  rc = sqlite3_create_collation(db, "collate", 456, 0, 0);
  यदि( rc!=SQLITE_MISUSE ){
    sqlite3_close(db);
    zErrFunction = "sqlite3_create_collation";
    गच्छ error_बहिर्गमनम्;
  }

  sqlite3_close(db);
  प्रत्यागम TCL_OK;

error_बहिर्गमनम्:
  Tcl_Reset_परिणाम(interp);
  Tcl_Append_परिणाम(interp, "Error testing function: ", zErrFunction, रिक्त);
  प्रत्यागम TCL_ERROR;
}

/*
** c_realloc_test
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI c_realloc_test(
  Client_दत्त client_दत्त, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  शून्य *p;
  ध्रुव वर्ण *zErrFunction = "N/A";

  यदि( objc!=1 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "");
    प्रत्यागम TCL_ERROR;
  }

  p = sqlite3_malloc(5);
  यदि( !p ){
    zErrFunction = "sqlite3_malloc";
    गच्छ error_बहिर्गमनम्;
  }

  /* Test that realloc()ing a block of memory to a negative size is
  ** the same as free()ing that memory.
  */
  p = sqlite3_realloc(p, -1);
  यदि( p ){
    zErrFunction = "sqlite3_realloc";
    गच्छ error_बहिर्गमनम्;
  }

  प्रत्यागम TCL_OK;

error_बहिर्गमनम्:
  Tcl_Reset_परिणाम(interp);
  Tcl_Append_परिणाम(interp, "Error testing function: ", zErrFunction, रिक्त);
  प्रत्यागम TCL_ERROR;
}


/*
** c_misuse_test
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI c_misuse_test(
  Client_दत्त client_दत्त, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  ध्रुव वर्ण *zErrFunction = "N/A";
  sqlite3 *db = 0;
  sqlite3_stmt *pStmt;
  पूर्णाङ्क rc;

  यदि( objc!=1 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "");
    प्रत्यागम TCL_ERROR;
  }

  /* Open a database. Then close it again. We need to do this so that
  ** we have a "closed database handle" to pass to various API functions.
  */
  rc = sqlite3_open(":memory:", &db);
  यदि( rc!=SQLITE_OK ){
    zErrFunction = "sqlite3_open";
    गच्छ error_बहिर्गमनम्;
  }
  sqlite3_close(db);


  rc = sqlite3_errcode(db);
  यदि( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_errcode";
    गच्छ error_बहिर्गमनम्;
  }

  pStmt = (sqlite3_stmt*)1234;
  rc = sqlite3_prepare(db, 0, 0, &pStmt, 0);
  यदि( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_prepare";
    गच्छ error_बहिर्गमनम्;
  }
  प्रतिज्ञा( pStmt==0 ); /* Verify that pStmt is zeroed even on a MISUSE error */

  pStmt = (sqlite3_stmt*)1234;
  rc = sqlite3_prepare_v2(db, 0, 0, &pStmt, 0);
  यदि( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_prepare_v2";
    गच्छ error_बहिर्गमनम्;
  }
  प्रतिज्ञा( pStmt==0 );

#यदि_अपरिभाषितम् SQLITE_OMIT_UTF16
  pStmt = (sqlite3_stmt*)1234;
  rc = sqlite3_prepare16(db, 0, 0, &pStmt, 0);
  यदि( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_prepare16";
    गच्छ error_बहिर्गमनम्;
  }
  प्रतिज्ञा( pStmt==0 );
  pStmt = (sqlite3_stmt*)1234;
  rc = sqlite3_prepare16_v2(db, 0, 0, &pStmt, 0);
  यदि( rc!=SQLITE_MISUSE ){
    zErrFunction = "sqlite3_prepare16_v2";
    गच्छ error_बहिर्गमनम्;
  }
  प्रतिज्ञा( pStmt==0 );
#यदि_अन्तः

  प्रत्यागम TCL_OK;

error_बहिर्गमनम्:
  Tcl_Reset_परिणाम(interp);
  Tcl_Append_परिणाम(interp, "Error testing function: ", zErrFunction, रिक्त);
  प्रत्यागम TCL_ERROR;
}

/*
** Register commands with the TCL interpreter.
*/
पूर्णाङ्क Sqlitetest9_Init(Tcl_Interp *interp){
  स्थिर संरचना {
     वर्ण *zName;
     Tcl_ObjCmdProc *xProc;
     शून्य *client_दत्त;
  } aObjCmd[] = {
     { "c_misuse_test",    c_misuse_test, 0 },
     { "c_realloc_test",   c_realloc_test, 0 },
     { "c_collation_test", c_collation_test, 0 },
  };
  पूर्णाङ्क i;
  प्रति(i=0; i<परिमाण(aObjCmd)/परिमाण(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].client_दत्त, 0);
  }
  प्रत्यागम TCL_OK;
}
