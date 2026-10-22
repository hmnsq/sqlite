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
#ਅਨ੍ਤਰ੍ਭਾਵ "tclsqlite.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "sqlite3ext.h"

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_ਲੱਦੋ_EXTENSION
SQLITE_EXTENSION_INIT1

/*
** The sqr() SQL function returns the square of its input value.
*/
ਸਥਿਰ ਖਾਲੀ sqrFunc(
  sqlite3_context *context,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  ਦੁੱਗਣਾ r = sqlite3_ਮੁੱਲ_double(argv[0]);
  sqlite3_ਨਤੀਜਾ_double(context, r*r);
}

/*
** This is the entry point to register the extension for the sqr() function.
*/
ਸਥਿਰ ਪੂਰਨ sqr_init(
  sqlite3 *db, 
  ਅੱਖਰ **pz_Err_ਸੁਨੇਹਾ, 
  ਅਚਲ sqlite3_api_routines *pApi
){
  SQLITE_EXTENSION_INIT2(pApi);
  sqlite3_create_function(db, "sqr", 1, SQLITE_ANY, 0, sqrFunc, 0, 0);
  ਵਾਪਸ 0;
}

/*
** The cube() SQL function returns the cube of its input value.
*/
ਸਥਿਰ ਖਾਲੀ cubeFunc(
  sqlite3_context *context,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  ਦੁੱਗਣਾ r = sqlite3_ਮੁੱਲ_double(argv[0]);
  sqlite3_ਨਤੀਜਾ_double(context, r*r*r);
}

/*
** This is the entry point to register the extension for the cube() function.
*/
ਸਥਿਰ ਪੂਰਨ cube_init(
  sqlite3 *db, 
  ਅੱਖਰ **pz_Err_ਸੁਨੇਹਾ, 
  ਅਚਲ sqlite3_api_routines *pApi
){
  SQLITE_EXTENSION_INIT2(pApi);
  sqlite3_create_function(db, "cube", 1, SQLITE_ANY, 0, cubeFunc, 0, 0);
  ਵਾਪਸ 0;
}

/*
** This is a broken extension entry point
*/
ਸਥਿਰ ਪੂਰਨ broken_init(
  sqlite3 *db, 
  ਅੱਖਰ **pz_Err_ਸੁਨੇਹਾ, 
  ਅਚਲ sqlite3_api_routines *pApi
){
  ਅੱਖਰ *zErr;
  SQLITE_EXTENSION_INIT2(pApi);
  zErr = sqlite3_mprintf("broken autoext!");
  *pz_Err_ਸੁਨੇਹਾ = zErr;
  ਵਾਪਸ 1;
}

/*
** tclcmd:   sqlite3_auto_extension_sqr
**
** Register the "sqr" extension to be loaded automatically.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI autoExtSqrObjCmd(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਪੂਰਨ rc = sqlite3_auto_extension((ਖਾਲੀ(*)(ਖਾਲੀ))sqr_init);
  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewIntObj(rc));
  ਵਾਪਸ SQLITE_OK;
}

/*
** tclcmd:   sqlite3_cancel_auto_extension_sqr
**
** Unregister the "sqr" extension.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI cancelAutoExtSqrObjCmd(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਪੂਰਨ rc = sqlite3_cancel_auto_extension((ਖਾਲੀ(*)(ਖਾਲੀ))sqr_init);
  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewIntObj(rc));
  ਵਾਪਸ SQLITE_OK;
}

/*
** tclcmd:   sqlite3_auto_extension_cube
**
** Register the "cube" extension to be loaded automatically.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI autoExtCubeObjCmd(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਪੂਰਨ rc = sqlite3_auto_extension((ਖਾਲੀ(*)(ਖਾਲੀ))cube_init);
  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewIntObj(rc));
  ਵਾਪਸ SQLITE_OK;
}

/*
** tclcmd:   sqlite3_cancel_auto_extension_cube
**
** Unregister the "cube" extension.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI cancelAutoExtCubeObjCmd(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਪੂਰਨ rc = sqlite3_cancel_auto_extension((ਖਾਲੀ(*)(ਖਾਲੀ))cube_init);
  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewIntObj(rc));
  ਵਾਪਸ SQLITE_OK;
}

/*
** tclcmd:   sqlite3_auto_extension_broken
**
** Register the broken extension to be loaded automatically.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI autoExtBrokenObjCmd(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਪੂਰਨ rc = sqlite3_auto_extension((ਖਾਲੀ(*)(ਖਾਲੀ))broken_init);
  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewIntObj(rc));
  ਵਾਪਸ SQLITE_OK;
}

/*
** tclcmd:   sqlite3_cancel_auto_extension_broken
**
** Unregister the broken extension.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI cancelAutoExtBrokenObjCmd(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਪੂਰਨ rc = sqlite3_cancel_auto_extension((ਖਾਲੀ(*)(ਖਾਲੀ))broken_init);
  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewIntObj(rc));
  ਵਾਪਸ SQLITE_OK;
}

#ਜੇ_ਅੰਤ /* SQLITE_OMIT_LOAD_EXTENSION */


/*
** tclcmd:   sqlite3_reset_auto_extension
**
** Reset all auto-extensions
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI resetAutoExtObjCmd(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_reset_auto_extension();
  ਵਾਪਸ SQLITE_OK;
}


/*
** This procedure registers the TCL procs defined in this file.
*/
ਪੂਰਨ Sqlitetest_autoext_Init(Tcl_Interp *interp){
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_ਲੱਦੋ_EXTENSION
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
#ਜੇ_ਅੰਤ
  Tcl_CreateObjCommand(interp, "sqlite3_reset_auto_extension",
          resetAutoExtObjCmd, 0, 0);
  ਵਾਪਸ TCL_OK;
}
