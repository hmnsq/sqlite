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
#ଅନ୍ତର୍ଭାଵ "tclsqlite.h"
#ଅନ୍ତର୍ଭାଵ "sqlite3ext.h"

#ifndef SQLITE_OMIT_LOAD_EXTENSION
SQLITE_EXTENSION_INIT1

/*
** The sqr() SQL function returns the square of its input value.
*/
ସ୍ଥିର ଶୂନ୍ୟ sqrFunc(
  sqlite3_context *context,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,
  sqlite3_ମୂଲ୍ୟ **argv
){
  ଦ୍ୱିଗୁଣ r = sqlite3_ମୂଲ୍ୟ_double(argv[0]);
  sqlite3_result_double(context, r*r);
}

/*
** This is the entry point to register the extension for the sqr() function.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqr_init(
  sqlite3 *db, 
  ଅକ୍ଷର **pz_Err_ସନ୍ଦେଶ, 
  ଅଚଳ sqlite3_api_routines *pApi
){
  SQLITE_EXTENSION_INIT2(pApi);
  sqlite3_create_function(db, "sqr", 1, SQLITE_ANY, 0, sqrFunc, 0, 0);
  ଫେରନ୍ତୁ 0;
}

/*
** The cube() SQL function returns the cube of its input value.
*/
ସ୍ଥିର ଶୂନ୍ୟ cubeFunc(
  sqlite3_context *context,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,
  sqlite3_ମୂଲ୍ୟ **argv
){
  ଦ୍ୱିଗୁଣ r = sqlite3_ମୂଲ୍ୟ_double(argv[0]);
  sqlite3_result_double(context, r*r*r);
}

/*
** This is the entry point to register the extension for the cube() function.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା cube_init(
  sqlite3 *db, 
  ଅକ୍ଷର **pz_Err_ସନ୍ଦେଶ, 
  ଅଚଳ sqlite3_api_routines *pApi
){
  SQLITE_EXTENSION_INIT2(pApi);
  sqlite3_create_function(db, "cube", 1, SQLITE_ANY, 0, cubeFunc, 0, 0);
  ଫେରନ୍ତୁ 0;
}

/*
** This is a broken extension entry point
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା broken_init(
  sqlite3 *db, 
  ଅକ୍ଷର **pz_Err_ସନ୍ଦେଶ, 
  ଅଚଳ sqlite3_api_routines *pApi
){
  ଅକ୍ଷର *zErr;
  SQLITE_EXTENSION_INIT2(pApi);
  zErr = sqlite3_mprintf("broken autoext!");
  *pz_Err_ସନ୍ଦେଶ = zErr;
  ଫେରନ୍ତୁ 1;
}

/*
** tclcmd:   sqlite3_auto_extension_sqr
**
** Register the "sqr" extension to be loaded automatically.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI autoExtSqrObjCmd(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = sqlite3_auto_extension((ଶୂନ୍ୟ(*)(ଶୂନ୍ୟ))sqr_init);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** tclcmd:   sqlite3_cancel_auto_extension_sqr
**
** Unregister the "sqr" extension.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI cancelAutoExtSqrObjCmd(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = sqlite3_cancel_auto_extension((ଶୂନ୍ୟ(*)(ଶୂନ୍ୟ))sqr_init);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** tclcmd:   sqlite3_auto_extension_cube
**
** Register the "cube" extension to be loaded automatically.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI autoExtCubeObjCmd(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = sqlite3_auto_extension((ଶୂନ୍ୟ(*)(ଶୂନ୍ୟ))cube_init);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** tclcmd:   sqlite3_cancel_auto_extension_cube
**
** Unregister the "cube" extension.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI cancelAutoExtCubeObjCmd(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = sqlite3_cancel_auto_extension((ଶୂନ୍ୟ(*)(ଶୂନ୍ୟ))cube_init);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** tclcmd:   sqlite3_auto_extension_broken
**
** Register the broken extension to be loaded automatically.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI autoExtBrokenObjCmd(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = sqlite3_auto_extension((ଶୂନ୍ୟ(*)(ଶୂନ୍ୟ))broken_init);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** tclcmd:   sqlite3_cancel_auto_extension_broken
**
** Unregister the broken extension.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI cancelAutoExtBrokenObjCmd(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = sqlite3_cancel_auto_extension((ଶୂନ୍ୟ(*)(ଶୂନ୍ୟ))broken_init);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  ଫେରନ୍ତୁ SQLITE_OK;
}

#endif /* SQLITE_OMIT_LOAD_EXTENSION */


/*
** tclcmd:   sqlite3_reset_auto_extension
**
** Reset all auto-extensions
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI resetAutoExtObjCmd(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_reset_auto_extension();
  ଫେରନ୍ତୁ SQLITE_OK;
}


/*
** This procedure registers the TCL procs defined in this file.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା Sqlitetest_autoext_Init(Tcl_Interp *interp){
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
  ଫେରନ୍ତୁ TCL_OK;
}
