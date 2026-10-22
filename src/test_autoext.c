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
#ضمن "tclsqlite.h"
#ضمن "sqlite3ext.h"

#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_ڀريو_EXTENSION
SQLITE_EXTENSION_INIT1

/*
** The sqr() SQL function returns the square of its input value.
*/
ساكن فراغ sqrFunc(
  sqlite3_context *context,
  صحيح argc,
  sqlite3_قدر **argv
){
  مزدوج r = sqlite3_قدر_double(argv[0]);
  sqlite3_نتيجو_double(context, r*r);
}

/*
** This is the entry point to register the extension for the sqr() function.
*/
ساكن صحيح sqr_init(
  sqlite3 *db, 
  حرف **pz_Err_نياپو, 
  ثابت sqlite3_api_routines *pApi
){
  SQLITE_EXTENSION_INIT2(pApi);
  sqlite3_create_function(db, "sqr", 1, SQLITE_ANY, 0, sqrFunc, 0, 0);
  أرجع 0;
}

/*
** The cube() SQL function returns the cube of its input value.
*/
ساكن فراغ cubeFunc(
  sqlite3_context *context,
  صحيح argc,
  sqlite3_قدر **argv
){
  مزدوج r = sqlite3_قدر_double(argv[0]);
  sqlite3_نتيجو_double(context, r*r*r);
}

/*
** This is the entry point to register the extension for the cube() function.
*/
ساكن صحيح cube_init(
  sqlite3 *db, 
  حرف **pz_Err_نياپو, 
  ثابت sqlite3_api_routines *pApi
){
  SQLITE_EXTENSION_INIT2(pApi);
  sqlite3_create_function(db, "cube", 1, SQLITE_ANY, 0, cubeFunc, 0, 0);
  أرجع 0;
}

/*
** This is a broken extension entry point
*/
ساكن صحيح broken_init(
  sqlite3 *db, 
  حرف **pz_Err_نياپو, 
  ثابت sqlite3_api_routines *pApi
){
  حرف *zErr;
  SQLITE_EXTENSION_INIT2(pApi);
  zErr = sqlite3_mprintf("broken autoext!");
  *pz_Err_نياپو = zErr;
  أرجع 1;
}

/*
** tclcmd:   sqlite3_auto_extension_sqr
**
** Register the "sqr" extension to be loaded automatically.
*/
ساكن صحيح SQLITE_TCLAPI autoExtSqrObjCmd(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح rc = sqlite3_auto_extension((فراغ(*)(فراغ))sqr_init);
  Tcl_سيٽ_Obj_نتيجو(interp, Tcl_NewIntObj(rc));
  أرجع SQLITE_OK;
}

/*
** tclcmd:   sqlite3_cancel_auto_extension_sqr
**
** Unregister the "sqr" extension.
*/
ساكن صحيح SQLITE_TCLAPI cancelAutoExtSqrObjCmd(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح rc = sqlite3_cancel_auto_extension((فراغ(*)(فراغ))sqr_init);
  Tcl_سيٽ_Obj_نتيجو(interp, Tcl_NewIntObj(rc));
  أرجع SQLITE_OK;
}

/*
** tclcmd:   sqlite3_auto_extension_cube
**
** Register the "cube" extension to be loaded automatically.
*/
ساكن صحيح SQLITE_TCLAPI autoExtCubeObjCmd(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح rc = sqlite3_auto_extension((فراغ(*)(فراغ))cube_init);
  Tcl_سيٽ_Obj_نتيجو(interp, Tcl_NewIntObj(rc));
  أرجع SQLITE_OK;
}

/*
** tclcmd:   sqlite3_cancel_auto_extension_cube
**
** Unregister the "cube" extension.
*/
ساكن صحيح SQLITE_TCLAPI cancelAutoExtCubeObjCmd(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح rc = sqlite3_cancel_auto_extension((فراغ(*)(فراغ))cube_init);
  Tcl_سيٽ_Obj_نتيجو(interp, Tcl_NewIntObj(rc));
  أرجع SQLITE_OK;
}

/*
** tclcmd:   sqlite3_auto_extension_broken
**
** Register the broken extension to be loaded automatically.
*/
ساكن صحيح SQLITE_TCLAPI autoExtBrokenObjCmd(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح rc = sqlite3_auto_extension((فراغ(*)(فراغ))broken_init);
  Tcl_سيٽ_Obj_نتيجو(interp, Tcl_NewIntObj(rc));
  أرجع SQLITE_OK;
}

/*
** tclcmd:   sqlite3_cancel_auto_extension_broken
**
** Unregister the broken extension.
*/
ساكن صحيح SQLITE_TCLAPI cancelAutoExtBrokenObjCmd(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح rc = sqlite3_cancel_auto_extension((فراغ(*)(فراغ))broken_init);
  Tcl_سيٽ_Obj_نتيجو(interp, Tcl_NewIntObj(rc));
  أرجع SQLITE_OK;
}

#جيڪڏهن_پورو /* SQLITE_OMIT_LOAD_EXTENSION */


/*
** tclcmd:   sqlite3_reset_auto_extension
**
** Reset all auto-extensions
*/
ساكن صحيح SQLITE_TCLAPI resetAutoExtObjCmd(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_reset_auto_extension();
  أرجع SQLITE_OK;
}


/*
** This procedure registers the TCL procs defined in this file.
*/
صحيح Sqlitetest_autoext_Init(Tcl_Interp *interp){
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_ڀريو_EXTENSION
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
#جيڪڏهن_پورو
  Tcl_CreateObjCommand(interp, "sqlite3_reset_auto_extension",
          resetAutoExtObjCmd, 0, 0);
  أرجع TCL_OK;
}
