/*
** 2006 August 23
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** Test extension for testing the sqlite3_auto_extension() function.
*/
#అన్తర్భావ "tclsqlite.h"
#అన్తర్భావ "sqlite3ext.h"

#నిర్వచితం_కాకపోతే SQLITE_OMIT_ఎక్కించు_EXTENSION
SQLITE_EXTENSION_INIT1

/*
** The sqr() SQL function returns the square of its input value.
*/
స్థిర శూన్య sqrFunc(
  sqlite3_context *context,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  ద్విగుణ r = sqlite3_విలువ_double(argv[0]);
  sqlite3_ఫలితం_double(context, r*r);
}

/*
** This is the entry point to register the extension for the sqr() function.
*/
స్థిర పూర్ణాంకం sqr_init(
  sqlite3 *db, 
  అక్షరం **pz_Err_సందేశం, 
  స్థిరరాశి sqlite3_api_routines *pApi
){
  SQLITE_EXTENSION_INIT2(pApi);
  sqlite3_create_function(db, "sqr", 1, SQLITE_ANY, 0, sqrFunc, 0, 0);
  తిరిగి 0;
}

/*
** The cube() SQL function returns the cube of its input value.
*/
స్థిర శూన్య cubeFunc(
  sqlite3_context *context,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  ద్విగుణ r = sqlite3_విలువ_double(argv[0]);
  sqlite3_ఫలితం_double(context, r*r*r);
}

/*
** This is the entry point to register the extension for the cube() function.
*/
స్థిర పూర్ణాంకం cube_init(
  sqlite3 *db, 
  అక్షరం **pz_Err_సందేశం, 
  స్థిరరాశి sqlite3_api_routines *pApi
){
  SQLITE_EXTENSION_INIT2(pApi);
  sqlite3_create_function(db, "cube", 1, SQLITE_ANY, 0, cubeFunc, 0, 0);
  తిరిగి 0;
}

/*
** This is a broken extension entry point
*/
స్థిర పూర్ణాంకం broken_init(
  sqlite3 *db, 
  అక్షరం **pz_Err_సందేశం, 
  స్థిరరాశి sqlite3_api_routines *pApi
){
  అక్షరం *zErr;
  SQLITE_EXTENSION_INIT2(pApi);
  zErr = sqlite3_mprintf("broken autoext!");
  *pz_Err_సందేశం = zErr;
  తిరిగి 1;
}

/*
** tclcmd:   sqlite3_auto_extension_sqr
**
** Register the "sqr" extension to be loaded automatically.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI autoExtSqrObjCmd(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  పూర్ణాంకం rc = sqlite3_auto_extension((శూన్య(*)(శూన్య))sqr_init);
  Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewIntObj(rc));
  తిరిగి SQLITE_OK;
}

/*
** tclcmd:   sqlite3_cancel_auto_extension_sqr
**
** Unregister the "sqr" extension.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI cancelAutoExtSqrObjCmd(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  పూర్ణాంకం rc = sqlite3_cancel_auto_extension((శూన్య(*)(శూన్య))sqr_init);
  Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewIntObj(rc));
  తిరిగి SQLITE_OK;
}

/*
** tclcmd:   sqlite3_auto_extension_cube
**
** Register the "cube" extension to be loaded automatically.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI autoExtCubeObjCmd(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  పూర్ణాంకం rc = sqlite3_auto_extension((శూన్య(*)(శూన్య))cube_init);
  Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewIntObj(rc));
  తిరిగి SQLITE_OK;
}

/*
** tclcmd:   sqlite3_cancel_auto_extension_cube
**
** Unregister the "cube" extension.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI cancelAutoExtCubeObjCmd(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  పూర్ణాంకం rc = sqlite3_cancel_auto_extension((శూన్య(*)(శూన్య))cube_init);
  Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewIntObj(rc));
  తిరిగి SQLITE_OK;
}

/*
** tclcmd:   sqlite3_auto_extension_broken
**
** Register the broken extension to be loaded automatically.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI autoExtBrokenObjCmd(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  పూర్ణాంకం rc = sqlite3_auto_extension((శూన్య(*)(శూన్య))broken_init);
  Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewIntObj(rc));
  తిరిగి SQLITE_OK;
}

/*
** tclcmd:   sqlite3_cancel_auto_extension_broken
**
** Unregister the broken extension.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI cancelAutoExtBrokenObjCmd(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  పూర్ణాంకం rc = sqlite3_cancel_auto_extension((శూన్య(*)(శూన్య))broken_init);
  Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewIntObj(rc));
  తిరిగి SQLITE_OK;
}

#అయితే_ముగింపు /* SQLITE_OMIT_LOAD_EXTENSION */


/*
** tclcmd:   sqlite3_reset_auto_extension
**
** Reset all auto-extensions
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI resetAutoExtObjCmd(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_reset_auto_extension();
  తిరిగి SQLITE_OK;
}


/*
** This procedure registers the TCL procs defined in this file.
*/
పూర్ణాంకం Sqlitetest_autoext_Init(Tcl_Interp *interp){
#నిర్వచితం_కాకపోతే SQLITE_OMIT_ఎక్కించు_EXTENSION
  Tcl_CreateObjCommand(interp, "sqlite3_auto_extension_sqr",
          autoExtSqrObjCmd, 0, 0);
  Tcl_CreateObjCommand(interp, "sqlite3_auto_extension_cube",
          autoExtCubeObjCmd, 0, 0);
  Tcl_CreateObjCommand(interp, "sqlite3_auto_extension_broken",
          autoExtBrokenObjCmd, 0, 0);
  Tcl_CreateObjCommand(interp, "sqlite3_cancel_auto_extension_sqr",
          cancelAutoExtSqrObjCmd, 0, 0);
  Tcl_CreateObjCommand(interp, "sqlite3_cancel_auto_extension_cube",
          cancelAutoExtCubeObjCmd, 0, 0);
  Tcl_CreateObjCommand(interp, "sqlite3_cancel_auto_extension_broken",
          cancelAutoExtBrokenObjCmd, 0, 0);
#అయితే_ముగింపు
  Tcl_CreateObjCommand(interp, "sqlite3_reset_auto_extension",
          resetAutoExtObjCmd, 0, 0);
  తిరిగి TCL_OK;
}
