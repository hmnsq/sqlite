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

#ifndef SQLITE_OMIT_LOAD_EXTENSION
SQLITE_EXTENSION_INIT1

/*
** The sqr() SQL function returns the square of its input value.
*/
ایستا تهی sqrFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_مقدار **argv
){
  دوگانه r = sqlite3_مقدار_double(argv[0]);
  sqlite3_result_double(context, r*r);
}

/*
** This is the entry point to register the extension for the sqr() function.
*/
ایستا صحیح sqr_init(
  sqlite3 *db, 
  نویسه **pz_Err_پیام, 
  ثابت sqlite3_api_routines *pApi
){
  SQLITE_EXTENSION_INIT2(pApi);
  sqlite3_create_function(db, "sqr", 1, SQLITE_ANY, 0, sqrFunc, 0, 0);
  بازگشت 0;
}

/*
** The cube() SQL function returns the cube of its input value.
*/
ایستا تهی cubeFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_مقدار **argv
){
  دوگانه r = sqlite3_مقدار_double(argv[0]);
  sqlite3_result_double(context, r*r*r);
}

/*
** This is the entry point to register the extension for the cube() function.
*/
ایستا صحیح cube_init(
  sqlite3 *db, 
  نویسه **pz_Err_پیام, 
  ثابت sqlite3_api_routines *pApi
){
  SQLITE_EXTENSION_INIT2(pApi);
  sqlite3_create_function(db, "cube", 1, SQLITE_ANY, 0, cubeFunc, 0, 0);
  بازگشت 0;
}

/*
** This is a broken extension entry point
*/
ایستا صحیح broken_init(
  sqlite3 *db, 
  نویسه **pz_Err_پیام, 
  ثابت sqlite3_api_routines *pApi
){
  نویسه *zErr;
  SQLITE_EXTENSION_INIT2(pApi);
  zErr = sqlite3_mprintf("broken autoext!");
  *pz_Err_پیام = zErr;
  بازگشت 1;
}

/*
** tclcmd:   sqlite3_auto_extension_sqr
**
** Register the "sqr" extension to be loaded automatically.
*/
ایستا صحیح SQLITE_TCLAPI autoExtSqrObjCmd(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح rc = sqlite3_auto_extension((تهی(*)(تهی))sqr_init);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  بازگشت SQLITE_OK;
}

/*
** tclcmd:   sqlite3_cancel_auto_extension_sqr
**
** Unregister the "sqr" extension.
*/
ایستا صحیح SQLITE_TCLAPI cancelAutoExtSqrObjCmd(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح rc = sqlite3_cancel_auto_extension((تهی(*)(تهی))sqr_init);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  بازگشت SQLITE_OK;
}

/*
** tclcmd:   sqlite3_auto_extension_cube
**
** Register the "cube" extension to be loaded automatically.
*/
ایستا صحیح SQLITE_TCLAPI autoExtCubeObjCmd(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح rc = sqlite3_auto_extension((تهی(*)(تهی))cube_init);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  بازگشت SQLITE_OK;
}

/*
** tclcmd:   sqlite3_cancel_auto_extension_cube
**
** Unregister the "cube" extension.
*/
ایستا صحیح SQLITE_TCLAPI cancelAutoExtCubeObjCmd(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح rc = sqlite3_cancel_auto_extension((تهی(*)(تهی))cube_init);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  بازگشت SQLITE_OK;
}

/*
** tclcmd:   sqlite3_auto_extension_broken
**
** Register the broken extension to be loaded automatically.
*/
ایستا صحیح SQLITE_TCLAPI autoExtBrokenObjCmd(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح rc = sqlite3_auto_extension((تهی(*)(تهی))broken_init);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  بازگشت SQLITE_OK;
}

/*
** tclcmd:   sqlite3_cancel_auto_extension_broken
**
** Unregister the broken extension.
*/
ایستا صحیح SQLITE_TCLAPI cancelAutoExtBrokenObjCmd(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح rc = sqlite3_cancel_auto_extension((تهی(*)(تهی))broken_init);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  بازگشت SQLITE_OK;
}

#endif /* SQLITE_OMIT_LOAD_EXTENSION */


/*
** tclcmd:   sqlite3_reset_auto_extension
**
** Reset all auto-extensions
*/
ایستا صحیح SQLITE_TCLAPI resetAutoExtObjCmd(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_reset_auto_extension();
  بازگشت SQLITE_OK;
}


/*
** This procedure registers the TCL procs defined in this file.
*/
صحیح Sqlitetest_autoext_Init(Tcl_Interp *interp){
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
  بازگشت TCL_OK;
}
