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
  정수 argc,
  sqlite3_value **argv
){
  double r = sqlite3_value_double(argv[0]);
  sqlite3_result_double(context, r*r);
}

/*
** This is the entry point to register the extension for the sqr() function.
*/
static 정수 sqr_init(
  sqlite3 *db, 
  char **pzErrMsg, 
  const sqlite3_api_routines *pApi
){
  SQLITE_EXTENSION_INIT2(pApi);
  sqlite3_create_function(db, "sqr", 1, SQLITE_ANY, 0, sqrFunc, 0, 0);
  반환 0;
}

/*
** The cube() SQL function returns the cube of its input value.
*/
static void cubeFunc(
  sqlite3_context *context,
  정수 argc,
  sqlite3_value **argv
){
  double r = sqlite3_value_double(argv[0]);
  sqlite3_result_double(context, r*r*r);
}

/*
** This is the entry point to register the extension for the cube() function.
*/
static 정수 cube_init(
  sqlite3 *db, 
  char **pzErrMsg, 
  const sqlite3_api_routines *pApi
){
  SQLITE_EXTENSION_INIT2(pApi);
  sqlite3_create_function(db, "cube", 1, SQLITE_ANY, 0, cubeFunc, 0, 0);
  반환 0;
}

/*
** This is a broken extension entry point
*/
static 정수 broken_init(
  sqlite3 *db, 
  char **pzErrMsg, 
  const sqlite3_api_routines *pApi
){
  char *zErr;
  SQLITE_EXTENSION_INIT2(pApi);
  zErr = sqlite3_mprintf("broken autoext!");
  *pzErrMsg = zErr;
  반환 1;
}

/*
** tclcmd:   sqlite3_auto_extension_sqr
**
** Register the "sqr" extension to be loaded automatically.
*/
static 정수 SQLITE_TCLAPI autoExtSqrObjCmd(
  void * clientData,
  Tcl_Interp *interp,
  정수 objc,
  Tcl_Obj *CONST objv[]
){
  정수 rc = sqlite3_auto_extension((void(*)(void))sqr_init);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  반환 SQLITE_OK;
}

/*
** tclcmd:   sqlite3_cancel_auto_extension_sqr
**
** Unregister the "sqr" extension.
*/
static 정수 SQLITE_TCLAPI cancelAutoExtSqrObjCmd(
  void * clientData,
  Tcl_Interp *interp,
  정수 objc,
  Tcl_Obj *CONST objv[]
){
  정수 rc = sqlite3_cancel_auto_extension((void(*)(void))sqr_init);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  반환 SQLITE_OK;
}

/*
** tclcmd:   sqlite3_auto_extension_cube
**
** Register the "cube" extension to be loaded automatically.
*/
static 정수 SQLITE_TCLAPI autoExtCubeObjCmd(
  void * clientData,
  Tcl_Interp *interp,
  정수 objc,
  Tcl_Obj *CONST objv[]
){
  정수 rc = sqlite3_auto_extension((void(*)(void))cube_init);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  반환 SQLITE_OK;
}

/*
** tclcmd:   sqlite3_cancel_auto_extension_cube
**
** Unregister the "cube" extension.
*/
static 정수 SQLITE_TCLAPI cancelAutoExtCubeObjCmd(
  void * clientData,
  Tcl_Interp *interp,
  정수 objc,
  Tcl_Obj *CONST objv[]
){
  정수 rc = sqlite3_cancel_auto_extension((void(*)(void))cube_init);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  반환 SQLITE_OK;
}

/*
** tclcmd:   sqlite3_auto_extension_broken
**
** Register the broken extension to be loaded automatically.
*/
static 정수 SQLITE_TCLAPI autoExtBrokenObjCmd(
  void * clientData,
  Tcl_Interp *interp,
  정수 objc,
  Tcl_Obj *CONST objv[]
){
  정수 rc = sqlite3_auto_extension((void(*)(void))broken_init);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  반환 SQLITE_OK;
}

/*
** tclcmd:   sqlite3_cancel_auto_extension_broken
**
** Unregister the broken extension.
*/
static 정수 SQLITE_TCLAPI cancelAutoExtBrokenObjCmd(
  void * clientData,
  Tcl_Interp *interp,
  정수 objc,
  Tcl_Obj *CONST objv[]
){
  정수 rc = sqlite3_cancel_auto_extension((void(*)(void))broken_init);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  반환 SQLITE_OK;
}

#endif /* SQLITE_OMIT_LOAD_EXTENSION */


/*
** tclcmd:   sqlite3_reset_auto_extension
**
** Reset all auto-extensions
*/
static 정수 SQLITE_TCLAPI resetAutoExtObjCmd(
  void * clientData,
  Tcl_Interp *interp,
  정수 objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_reset_auto_extension();
  반환 SQLITE_OK;
}


/*
** This procedure registers the TCL procs defined in this file.
*/
정수 Sqlitetest_autoext_Init(Tcl_Interp *interp){
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
  반환 TCL_OK;
}
