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
#شامل "tclsqlite.h"
#شامل "sqlite3ext.h"

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_بھرو_EXTENSION
SQLITE_EXTENSION_INIT1

/*
** The sqr() SQL function returns the square of its input value.
*/
ساکن خالی sqrFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_قدر **argv
){
  دوہرا r = sqlite3_قدر_double(argv[0]);
  sqlite3_نتیجہ_double(context, r*r);
}

/*
** This is the entry point to register the extension for the sqr() function.
*/
ساکن صحیح sqr_init(
  sqlite3 *db, 
  حرف **pz_Err_پیغام, 
  ثابت sqlite3_api_routines *pApi
){
  SQLITE_EXTENSION_INIT2(pApi);
  sqlite3_create_function(db, "sqr", 1, SQLITE_ANY, 0, sqrFunc, 0, 0);
  واپس 0;
}

/*
** The cube() SQL function returns the cube of its input value.
*/
ساکن خالی cubeFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_قدر **argv
){
  دوہرا r = sqlite3_قدر_double(argv[0]);
  sqlite3_نتیجہ_double(context, r*r*r);
}

/*
** This is the entry point to register the extension for the cube() function.
*/
ساکن صحیح cube_init(
  sqlite3 *db, 
  حرف **pz_Err_پیغام, 
  ثابت sqlite3_api_routines *pApi
){
  SQLITE_EXTENSION_INIT2(pApi);
  sqlite3_create_function(db, "cube", 1, SQLITE_ANY, 0, cubeFunc, 0, 0);
  واپس 0;
}

/*
** This is a broken extension entry point
*/
ساکن صحیح broken_init(
  sqlite3 *db, 
  حرف **pz_Err_پیغام, 
  ثابت sqlite3_api_routines *pApi
){
  حرف *zErr;
  SQLITE_EXTENSION_INIT2(pApi);
  zErr = sqlite3_mprintf("broken autoext!");
  *pz_Err_پیغام = zErr;
  واپس 1;
}

/*
** tclcmd:   sqlite3_auto_extension_sqr
**
** Register the "sqr" extension to be loaded automatically.
*/
ساکن صحیح SQLITE_TCLAPI autoExtSqrObjCmd(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح rc = sqlite3_auto_extension((خالی(*)(خالی))sqr_init);
  Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewIntObj(rc));
  واپس SQLITE_OK;
}

/*
** tclcmd:   sqlite3_cancel_auto_extension_sqr
**
** Unregister the "sqr" extension.
*/
ساکن صحیح SQLITE_TCLAPI cancelAutoExtSqrObjCmd(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح rc = sqlite3_cancel_auto_extension((خالی(*)(خالی))sqr_init);
  Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewIntObj(rc));
  واپس SQLITE_OK;
}

/*
** tclcmd:   sqlite3_auto_extension_cube
**
** Register the "cube" extension to be loaded automatically.
*/
ساکن صحیح SQLITE_TCLAPI autoExtCubeObjCmd(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح rc = sqlite3_auto_extension((خالی(*)(خالی))cube_init);
  Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewIntObj(rc));
  واپس SQLITE_OK;
}

/*
** tclcmd:   sqlite3_cancel_auto_extension_cube
**
** Unregister the "cube" extension.
*/
ساکن صحیح SQLITE_TCLAPI cancelAutoExtCubeObjCmd(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح rc = sqlite3_cancel_auto_extension((خالی(*)(خالی))cube_init);
  Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewIntObj(rc));
  واپس SQLITE_OK;
}

/*
** tclcmd:   sqlite3_auto_extension_broken
**
** Register the broken extension to be loaded automatically.
*/
ساکن صحیح SQLITE_TCLAPI autoExtBrokenObjCmd(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح rc = sqlite3_auto_extension((خالی(*)(خالی))broken_init);
  Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewIntObj(rc));
  واپس SQLITE_OK;
}

/*
** tclcmd:   sqlite3_cancel_auto_extension_broken
**
** Unregister the broken extension.
*/
ساکن صحیح SQLITE_TCLAPI cancelAutoExtBrokenObjCmd(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح rc = sqlite3_cancel_auto_extension((خالی(*)(خالی))broken_init);
  Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewIntObj(rc));
  واپس SQLITE_OK;
}

#اگر_ختم /* SQLITE_OMIT_LOAD_EXTENSION */


/*
** tclcmd:   sqlite3_reset_auto_extension
**
** Reset all auto-extensions
*/
ساکن صحیح SQLITE_TCLAPI resetAutoExtObjCmd(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_reset_auto_extension();
  واپس SQLITE_OK;
}


/*
** This procedure registers the TCL procs defined in this file.
*/
صحیح Sqlitetest_autoext_Init(Tcl_Interp *interp){
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_بھرو_EXTENSION
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
#اگر_ختم
  Tcl_CreateObjCommand(interp, "sqlite3_reset_auto_extension",
          resetAutoExtObjCmd, 0, 0);
  واپس TCL_OK;
}
