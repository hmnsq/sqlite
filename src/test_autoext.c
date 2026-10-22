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
#অন্তর্ভাব "tclsqlite.h"
#অন্তর্ভাব "sqlite3ext.h"

#ifndef SQLITE_OMIT_LOAD_EXTENSION
SQLITE_EXTENSION_INIT1

/*
** The sqr() SQL function returns the square of its input value.
*/
স্থির শূন্য sqrFunc(
  sqlite3_context *context,
  পূর্ণসংখ্যা argc,
  sqlite3_মান **argv
){
  দ্বিগুণ r = sqlite3_মান_double(argv[0]);
  sqlite3_ফলাফল_double(context, r*r);
}

/*
** This is the entry point to register the extension for the sqr() function.
*/
স্থির পূর্ণসংখ্যা sqr_init(
  sqlite3 *db, 
  অক্ষর **pz_Err_বার্তা, 
  ধ্রুবক sqlite3_api_routines *pApi
){
  SQLITE_EXTENSION_INIT2(pApi);
  sqlite3_create_function(db, "sqr", 1, SQLITE_ANY, 0, sqrFunc, 0, 0);
  ফেরত 0;
}

/*
** The cube() SQL function returns the cube of its input value.
*/
স্থির শূন্য cubeFunc(
  sqlite3_context *context,
  পূর্ণসংখ্যা argc,
  sqlite3_মান **argv
){
  দ্বিগুণ r = sqlite3_মান_double(argv[0]);
  sqlite3_ফলাফল_double(context, r*r*r);
}

/*
** This is the entry point to register the extension for the cube() function.
*/
স্থির পূর্ণসংখ্যা cube_init(
  sqlite3 *db, 
  অক্ষর **pz_Err_বার্তা, 
  ধ্রুবক sqlite3_api_routines *pApi
){
  SQLITE_EXTENSION_INIT2(pApi);
  sqlite3_create_function(db, "cube", 1, SQLITE_ANY, 0, cubeFunc, 0, 0);
  ফেরত 0;
}

/*
** This is a broken extension entry point
*/
স্থির পূর্ণসংখ্যা broken_init(
  sqlite3 *db, 
  অক্ষর **pz_Err_বার্তা, 
  ধ্রুবক sqlite3_api_routines *pApi
){
  অক্ষর *zErr;
  SQLITE_EXTENSION_INIT2(pApi);
  zErr = sqlite3_mprintf("broken autoext!");
  *pz_Err_বার্তা = zErr;
  ফেরত 1;
}

/*
** tclcmd:   sqlite3_auto_extension_sqr
**
** Register the "sqr" extension to be loaded automatically.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI autoExtSqrObjCmd(
  শূন্য * client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  পূর্ণসংখ্যা rc = sqlite3_auto_extension((শূন্য(*)(শূন্য))sqr_init);
  Tcl_Set_Obj_ফলাফল(interp, Tcl_NewIntObj(rc));
  ফেরত SQLITE_OK;
}

/*
** tclcmd:   sqlite3_cancel_auto_extension_sqr
**
** Unregister the "sqr" extension.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI cancelAutoExtSqrObjCmd(
  শূন্য * client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  পূর্ণসংখ্যা rc = sqlite3_cancel_auto_extension((শূন্য(*)(শূন্য))sqr_init);
  Tcl_Set_Obj_ফলাফল(interp, Tcl_NewIntObj(rc));
  ফেরত SQLITE_OK;
}

/*
** tclcmd:   sqlite3_auto_extension_cube
**
** Register the "cube" extension to be loaded automatically.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI autoExtCubeObjCmd(
  শূন্য * client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  পূর্ণসংখ্যা rc = sqlite3_auto_extension((শূন্য(*)(শূন্য))cube_init);
  Tcl_Set_Obj_ফলাফল(interp, Tcl_NewIntObj(rc));
  ফেরত SQLITE_OK;
}

/*
** tclcmd:   sqlite3_cancel_auto_extension_cube
**
** Unregister the "cube" extension.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI cancelAutoExtCubeObjCmd(
  শূন্য * client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  পূর্ণসংখ্যা rc = sqlite3_cancel_auto_extension((শূন্য(*)(শূন্য))cube_init);
  Tcl_Set_Obj_ফলাফল(interp, Tcl_NewIntObj(rc));
  ফেরত SQLITE_OK;
}

/*
** tclcmd:   sqlite3_auto_extension_broken
**
** Register the broken extension to be loaded automatically.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI autoExtBrokenObjCmd(
  শূন্য * client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  পূর্ণসংখ্যা rc = sqlite3_auto_extension((শূন্য(*)(শূন্য))broken_init);
  Tcl_Set_Obj_ফলাফল(interp, Tcl_NewIntObj(rc));
  ফেরত SQLITE_OK;
}

/*
** tclcmd:   sqlite3_cancel_auto_extension_broken
**
** Unregister the broken extension.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI cancelAutoExtBrokenObjCmd(
  শূন্য * client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  পূর্ণসংখ্যা rc = sqlite3_cancel_auto_extension((শূন্য(*)(শূন্য))broken_init);
  Tcl_Set_Obj_ফলাফল(interp, Tcl_NewIntObj(rc));
  ফেরত SQLITE_OK;
}

#endif /* SQLITE_OMIT_LOAD_EXTENSION */


/*
** tclcmd:   sqlite3_reset_auto_extension
**
** Reset all auto-extensions
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI resetAutoExtObjCmd(
  শূন্য * client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_reset_auto_extension();
  ফেরত SQLITE_OK;
}


/*
** This procedure registers the TCL procs defined in this file.
*/
পূর্ণসংখ্যা Sqlitetest_autoext_Init(Tcl_Interp *interp){
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
  ফেরত TCL_OK;
}
