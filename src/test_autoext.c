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
#അന്തര്ഭാവ "tclsqlite.h"
#അന്തര്ഭാവ "sqlite3ext.h"

#ifndef SQLITE_OMIT_LOAD_EXTENSION
SQLITE_EXTENSION_INIT1

/*
** The sqr() SQL function returns the square of its input value.
*/
സ്ഥിതം ശൂന്യം sqrFunc(
  sqlite3_context *context,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv
){
  ഇരട്ട r = sqlite3_മൂല്യം_double(argv[0]);
  sqlite3_result_double(context, r*r);
}

/*
** This is the entry point to register the extension for the sqr() function.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ sqr_init(
  sqlite3 *db, 
  അക്ഷരം **pz_Err_സന്ദേശം, 
  സ്ഥിരം sqlite3_api_routines *pApi
){
  SQLITE_EXTENSION_INIT2(pApi);
  sqlite3_create_function(db, "sqr", 1, SQLITE_ANY, 0, sqrFunc, 0, 0);
  തിരികെ 0;
}

/*
** The cube() SQL function returns the cube of its input value.
*/
സ്ഥിതം ശൂന്യം cubeFunc(
  sqlite3_context *context,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv
){
  ഇരട്ട r = sqlite3_മൂല്യം_double(argv[0]);
  sqlite3_result_double(context, r*r*r);
}

/*
** This is the entry point to register the extension for the cube() function.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ cube_init(
  sqlite3 *db, 
  അക്ഷരം **pz_Err_സന്ദേശം, 
  സ്ഥിരം sqlite3_api_routines *pApi
){
  SQLITE_EXTENSION_INIT2(pApi);
  sqlite3_create_function(db, "cube", 1, SQLITE_ANY, 0, cubeFunc, 0, 0);
  തിരികെ 0;
}

/*
** This is a broken extension entry point
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ broken_init(
  sqlite3 *db, 
  അക്ഷരം **pz_Err_സന്ദേശം, 
  സ്ഥിരം sqlite3_api_routines *pApi
){
  അക്ഷരം *zErr;
  SQLITE_EXTENSION_INIT2(pApi);
  zErr = sqlite3_mprintf("broken autoext!");
  *pz_Err_സന്ദേശം = zErr;
  തിരികെ 1;
}

/*
** tclcmd:   sqlite3_auto_extension_sqr
**
** Register the "sqr" extension to be loaded automatically.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI autoExtSqrObjCmd(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  പൂർണ്ണസംഖ്യ rc = sqlite3_auto_extension((ശൂന്യം(*)(ശൂന്യം))sqr_init);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  തിരികെ SQLITE_OK;
}

/*
** tclcmd:   sqlite3_cancel_auto_extension_sqr
**
** Unregister the "sqr" extension.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI cancelAutoExtSqrObjCmd(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  പൂർണ്ണസംഖ്യ rc = sqlite3_cancel_auto_extension((ശൂന്യം(*)(ശൂന്യം))sqr_init);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  തിരികെ SQLITE_OK;
}

/*
** tclcmd:   sqlite3_auto_extension_cube
**
** Register the "cube" extension to be loaded automatically.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI autoExtCubeObjCmd(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  പൂർണ്ണസംഖ്യ rc = sqlite3_auto_extension((ശൂന്യം(*)(ശൂന്യം))cube_init);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  തിരികെ SQLITE_OK;
}

/*
** tclcmd:   sqlite3_cancel_auto_extension_cube
**
** Unregister the "cube" extension.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI cancelAutoExtCubeObjCmd(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  പൂർണ്ണസംഖ്യ rc = sqlite3_cancel_auto_extension((ശൂന്യം(*)(ശൂന്യം))cube_init);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  തിരികെ SQLITE_OK;
}

/*
** tclcmd:   sqlite3_auto_extension_broken
**
** Register the broken extension to be loaded automatically.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI autoExtBrokenObjCmd(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  പൂർണ്ണസംഖ്യ rc = sqlite3_auto_extension((ശൂന്യം(*)(ശൂന്യം))broken_init);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  തിരികെ SQLITE_OK;
}

/*
** tclcmd:   sqlite3_cancel_auto_extension_broken
**
** Unregister the broken extension.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI cancelAutoExtBrokenObjCmd(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  പൂർണ്ണസംഖ്യ rc = sqlite3_cancel_auto_extension((ശൂന്യം(*)(ശൂന്യം))broken_init);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  തിരികെ SQLITE_OK;
}

#endif /* SQLITE_OMIT_LOAD_EXTENSION */


/*
** tclcmd:   sqlite3_reset_auto_extension
**
** Reset all auto-extensions
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI resetAutoExtObjCmd(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_reset_auto_extension();
  തിരികെ SQLITE_OK;
}


/*
** This procedure registers the TCL procs defined in this file.
*/
പൂർണ്ണസംഖ്യ Sqlitetest_autoext_Init(Tcl_Interp *interp){
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
  തിരികെ TCL_OK;
}
