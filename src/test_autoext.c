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
#அந்தர்பாவ "tclsqlite.h"
#அந்தர்பாவ "sqlite3ext.h"

#ifndef SQLITE_OMIT_LOAD_EXTENSION
SQLITE_EXTENSION_INIT1

/*
** The sqr() SQL function returns the square of its input value.
*/
நிலையான வெறுமை sqrFunc(
  sqlite3_context *context,
  முழு argc,
  sqlite3_மதிப்பு **argv
){
  இரட்டை r = sqlite3_மதிப்பு_double(argv[0]);
  sqlite3_result_double(context, r*r);
}

/*
** This is the entry point to register the extension for the sqr() function.
*/
நிலையான முழு sqr_init(
  sqlite3 *db, 
  எழுத்து **pz_Err_செய்தி, 
  மாறிலி sqlite3_api_routines *pApi
){
  SQLITE_EXTENSION_INIT2(pApi);
  sqlite3_create_function(db, "sqr", 1, SQLITE_ANY, 0, sqrFunc, 0, 0);
  திரும்பு 0;
}

/*
** The cube() SQL function returns the cube of its input value.
*/
நிலையான வெறுமை cubeFunc(
  sqlite3_context *context,
  முழு argc,
  sqlite3_மதிப்பு **argv
){
  இரட்டை r = sqlite3_மதிப்பு_double(argv[0]);
  sqlite3_result_double(context, r*r*r);
}

/*
** This is the entry point to register the extension for the cube() function.
*/
நிலையான முழு cube_init(
  sqlite3 *db, 
  எழுத்து **pz_Err_செய்தி, 
  மாறிலி sqlite3_api_routines *pApi
){
  SQLITE_EXTENSION_INIT2(pApi);
  sqlite3_create_function(db, "cube", 1, SQLITE_ANY, 0, cubeFunc, 0, 0);
  திரும்பு 0;
}

/*
** This is a broken extension entry point
*/
நிலையான முழு broken_init(
  sqlite3 *db, 
  எழுத்து **pz_Err_செய்தி, 
  மாறிலி sqlite3_api_routines *pApi
){
  எழுத்து *zErr;
  SQLITE_EXTENSION_INIT2(pApi);
  zErr = sqlite3_mprintf("broken autoext!");
  *pz_Err_செய்தி = zErr;
  திரும்பு 1;
}

/*
** tclcmd:   sqlite3_auto_extension_sqr
**
** Register the "sqr" extension to be loaded automatically.
*/
நிலையான முழு SQLITE_TCLAPI autoExtSqrObjCmd(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  முழு rc = sqlite3_auto_extension((வெறுமை(*)(வெறுமை))sqr_init);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  திரும்பு SQLITE_OK;
}

/*
** tclcmd:   sqlite3_cancel_auto_extension_sqr
**
** Unregister the "sqr" extension.
*/
நிலையான முழு SQLITE_TCLAPI cancelAutoExtSqrObjCmd(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  முழு rc = sqlite3_cancel_auto_extension((வெறுமை(*)(வெறுமை))sqr_init);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  திரும்பு SQLITE_OK;
}

/*
** tclcmd:   sqlite3_auto_extension_cube
**
** Register the "cube" extension to be loaded automatically.
*/
நிலையான முழு SQLITE_TCLAPI autoExtCubeObjCmd(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  முழு rc = sqlite3_auto_extension((வெறுமை(*)(வெறுமை))cube_init);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  திரும்பு SQLITE_OK;
}

/*
** tclcmd:   sqlite3_cancel_auto_extension_cube
**
** Unregister the "cube" extension.
*/
நிலையான முழு SQLITE_TCLAPI cancelAutoExtCubeObjCmd(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  முழு rc = sqlite3_cancel_auto_extension((வெறுமை(*)(வெறுமை))cube_init);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  திரும்பு SQLITE_OK;
}

/*
** tclcmd:   sqlite3_auto_extension_broken
**
** Register the broken extension to be loaded automatically.
*/
நிலையான முழு SQLITE_TCLAPI autoExtBrokenObjCmd(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  முழு rc = sqlite3_auto_extension((வெறுமை(*)(வெறுமை))broken_init);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  திரும்பு SQLITE_OK;
}

/*
** tclcmd:   sqlite3_cancel_auto_extension_broken
**
** Unregister the broken extension.
*/
நிலையான முழு SQLITE_TCLAPI cancelAutoExtBrokenObjCmd(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  முழு rc = sqlite3_cancel_auto_extension((வெறுமை(*)(வெறுமை))broken_init);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  திரும்பு SQLITE_OK;
}

#endif /* SQLITE_OMIT_LOAD_EXTENSION */


/*
** tclcmd:   sqlite3_reset_auto_extension
**
** Reset all auto-extensions
*/
நிலையான முழு SQLITE_TCLAPI resetAutoExtObjCmd(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_reset_auto_extension();
  திரும்பு SQLITE_OK;
}


/*
** This procedure registers the TCL procs defined in this file.
*/
முழு Sqlitetest_autoext_Init(Tcl_Interp *interp){
#ifndef SQLITE_OMIT_LOAD_EXTENSION
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
#endif
  Tcl_CreateObjCommand(interp, "sqlite3_reset_auto_extension",
          resetAutoExtObjCmd, 0, 0);
  திரும்பு TCL_OK;
}
