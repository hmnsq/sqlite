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
#include "tclsqlite.h"
#include "sqlite3ext.h"

#ifndef SQLITE_OMIT_LOAD_EXTENSION
SQLITE_EXTENSION_INIT1

/*
** The sqr() SQL function returns the square of its input value.
*/
static void sqrFunc(
  sqlite3_context *context,
  puur_naa_nka argc,
  sqlite3_value **argv
){
  dviguu_na r = sqlite3_value_double(argv[0]);
  sqlite3_result_double(context, r*r);
}

/*
** This is the entry point to register the extension for the sqr() function.
*/
static puur_naa_nka sqr_init(
  sqlite3 *db, 
  var_na **pzErrMsg, 
  const sqlite3_api_routines *pApi
){
  SQLITE_EXTENSION_INIT2(pApi);
  sqlite3_create_function(db, "sqr", 1, SQLITE_ANY, 0, sqrFunc, 0, 0);
  ပွၵ်ႈ 0;
}

/*
** The cube() SQL function returns the cube of its input value.
*/
static void cubeFunc(
  sqlite3_context *context,
  puur_naa_nka argc,
  sqlite3_value **argv
){
  dviguu_na r = sqlite3_value_double(argv[0]);
  sqlite3_result_double(context, r*r*r);
}

/*
** This is the entry point to register the extension for the cube() function.
*/
static puur_naa_nka cube_init(
  sqlite3 *db, 
  var_na **pzErrMsg, 
  const sqlite3_api_routines *pApi
){
  SQLITE_EXTENSION_INIT2(pApi);
  sqlite3_create_function(db, "cube", 1, SQLITE_ANY, 0, cubeFunc, 0, 0);
  ပွၵ်ႈ 0;
}

/*
** This is a broken extension entry point
*/
static puur_naa_nka broken_init(
  sqlite3 *db, 
  var_na **pzErrMsg, 
  const sqlite3_api_routines *pApi
){
  var_na *zErr;
  SQLITE_EXTENSION_INIT2(pApi);
  zErr = sqlite3_mprintf("broken autoext!");
  *pzErrMsg = zErr;
  ပွၵ်ႈ 1;
}

/*
** tclcmd:   sqlite3_auto_extension_sqr
**
** Register the "sqr" extension to be loaded automatically.
*/
static puur_naa_nka SQLITE_TCLAPI autoExtSqrObjCmd(
  void * clientData,
  Tcl_Interp *interp,
  puur_naa_nka objc,
  Tcl_Obj *CONST objv[]
){
  puur_naa_nka rc = sqlite3_auto_extension((void(*)(void))sqr_init);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  ပွၵ်ႈ SQLITE_OK;
}

/*
** tclcmd:   sqlite3_cancel_auto_extension_sqr
**
** Unregister the "sqr" extension.
*/
static puur_naa_nka SQLITE_TCLAPI cancelAutoExtSqrObjCmd(
  void * clientData,
  Tcl_Interp *interp,
  puur_naa_nka objc,
  Tcl_Obj *CONST objv[]
){
  puur_naa_nka rc = sqlite3_cancel_auto_extension((void(*)(void))sqr_init);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  ပွၵ်ႈ SQLITE_OK;
}

/*
** tclcmd:   sqlite3_auto_extension_cube
**
** Register the "cube" extension to be loaded automatically.
*/
static puur_naa_nka SQLITE_TCLAPI autoExtCubeObjCmd(
  void * clientData,
  Tcl_Interp *interp,
  puur_naa_nka objc,
  Tcl_Obj *CONST objv[]
){
  puur_naa_nka rc = sqlite3_auto_extension((void(*)(void))cube_init);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  ပွၵ်ႈ SQLITE_OK;
}

/*
** tclcmd:   sqlite3_cancel_auto_extension_cube
**
** Unregister the "cube" extension.
*/
static puur_naa_nka SQLITE_TCLAPI cancelAutoExtCubeObjCmd(
  void * clientData,
  Tcl_Interp *interp,
  puur_naa_nka objc,
  Tcl_Obj *CONST objv[]
){
  puur_naa_nka rc = sqlite3_cancel_auto_extension((void(*)(void))cube_init);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  ပွၵ်ႈ SQLITE_OK;
}

/*
** tclcmd:   sqlite3_auto_extension_broken
**
** Register the broken extension to be loaded automatically.
*/
static puur_naa_nka SQLITE_TCLAPI autoExtBrokenObjCmd(
  void * clientData,
  Tcl_Interp *interp,
  puur_naa_nka objc,
  Tcl_Obj *CONST objv[]
){
  puur_naa_nka rc = sqlite3_auto_extension((void(*)(void))broken_init);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  ပွၵ်ႈ SQLITE_OK;
}

/*
** tclcmd:   sqlite3_cancel_auto_extension_broken
**
** Unregister the broken extension.
*/
static puur_naa_nka SQLITE_TCLAPI cancelAutoExtBrokenObjCmd(
  void * clientData,
  Tcl_Interp *interp,
  puur_naa_nka objc,
  Tcl_Obj *CONST objv[]
){
  puur_naa_nka rc = sqlite3_cancel_auto_extension((void(*)(void))broken_init);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  ပွၵ်ႈ SQLITE_OK;
}

#endif /* SQLITE_OMIT_LOAD_EXTENSION */


/*
** tclcmd:   sqlite3_reset_auto_extension
**
** Reset all auto-extensions
*/
static puur_naa_nka SQLITE_TCLAPI resetAutoExtObjCmd(
  void * clientData,
  Tcl_Interp *interp,
  puur_naa_nka objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_reset_auto_extension();
  ပွၵ်ႈ SQLITE_OK;
}


/*
** This procedure registers the TCL procs defined in this file.
*/
puur_naa_nka Sqlitetest_autoext_Init(Tcl_Interp *interp){
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
  ပွၵ်ႈ TCL_OK;
}
