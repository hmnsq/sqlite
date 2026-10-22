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
#અન્તર્ભાવ "tclsqlite.h"
#અન્તર્ભાવ "sqlite3ext.h"

#ifndef SQLITE_OMIT_LOAD_EXTENSION
SQLITE_EXTENSION_INIT1

/*
** The sqr() SQL function returns the square of its input value.
*/
સ્થિર શૂન્ય sqrFunc(
  sqlite3_context *context,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  દ્વિગુણ r = sqlite3_કિંમત_double(argv[0]);
  sqlite3_result_double(context, r*r);
}

/*
** This is the entry point to register the extension for the sqr() function.
*/
સ્થિર પૂર્ણાંક sqr_init(
  sqlite3 *db, 
  અક્ષર **pz_Err_સંદેશ, 
  અચલ sqlite3_api_routines *pApi
){
  SQLITE_EXTENSION_INIT2(pApi);
  sqlite3_create_function(db, "sqr", 1, SQLITE_ANY, 0, sqrFunc, 0, 0);
  પાછા 0;
}

/*
** The cube() SQL function returns the cube of its input value.
*/
સ્થિર શૂન્ય cubeFunc(
  sqlite3_context *context,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  દ્વિગુણ r = sqlite3_કિંમત_double(argv[0]);
  sqlite3_result_double(context, r*r*r);
}

/*
** This is the entry point to register the extension for the cube() function.
*/
સ્થિર પૂર્ણાંક cube_init(
  sqlite3 *db, 
  અક્ષર **pz_Err_સંદેશ, 
  અચલ sqlite3_api_routines *pApi
){
  SQLITE_EXTENSION_INIT2(pApi);
  sqlite3_create_function(db, "cube", 1, SQLITE_ANY, 0, cubeFunc, 0, 0);
  પાછા 0;
}

/*
** This is a broken extension entry point
*/
સ્થિર પૂર્ણાંક broken_init(
  sqlite3 *db, 
  અક્ષર **pz_Err_સંદેશ, 
  અચલ sqlite3_api_routines *pApi
){
  અક્ષર *zErr;
  SQLITE_EXTENSION_INIT2(pApi);
  zErr = sqlite3_mprintf("broken autoext!");
  *pz_Err_સંદેશ = zErr;
  પાછા 1;
}

/*
** tclcmd:   sqlite3_auto_extension_sqr
**
** Register the "sqr" extension to be loaded automatically.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI autoExtSqrObjCmd(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  પૂર્ણાંક rc = sqlite3_auto_extension((શૂન્ય(*)(શૂન્ય))sqr_init);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  પાછા SQLITE_OK;
}

/*
** tclcmd:   sqlite3_cancel_auto_extension_sqr
**
** Unregister the "sqr" extension.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI cancelAutoExtSqrObjCmd(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  પૂર્ણાંક rc = sqlite3_cancel_auto_extension((શૂન્ય(*)(શૂન્ય))sqr_init);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  પાછા SQLITE_OK;
}

/*
** tclcmd:   sqlite3_auto_extension_cube
**
** Register the "cube" extension to be loaded automatically.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI autoExtCubeObjCmd(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  પૂર્ણાંક rc = sqlite3_auto_extension((શૂન્ય(*)(શૂન્ય))cube_init);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  પાછા SQLITE_OK;
}

/*
** tclcmd:   sqlite3_cancel_auto_extension_cube
**
** Unregister the "cube" extension.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI cancelAutoExtCubeObjCmd(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  પૂર્ણાંક rc = sqlite3_cancel_auto_extension((શૂન્ય(*)(શૂન્ય))cube_init);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  પાછા SQLITE_OK;
}

/*
** tclcmd:   sqlite3_auto_extension_broken
**
** Register the broken extension to be loaded automatically.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI autoExtBrokenObjCmd(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  પૂર્ણાંક rc = sqlite3_auto_extension((શૂન્ય(*)(શૂન્ય))broken_init);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  પાછા SQLITE_OK;
}

/*
** tclcmd:   sqlite3_cancel_auto_extension_broken
**
** Unregister the broken extension.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI cancelAutoExtBrokenObjCmd(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  પૂર્ણાંક rc = sqlite3_cancel_auto_extension((શૂન્ય(*)(શૂન્ય))broken_init);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  પાછા SQLITE_OK;
}

#endif /* SQLITE_OMIT_LOAD_EXTENSION */


/*
** tclcmd:   sqlite3_reset_auto_extension
**
** Reset all auto-extensions
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI resetAutoExtObjCmd(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_reset_auto_extension();
  પાછા SQLITE_OK;
}


/*
** This procedure registers the TCL procs defined in this file.
*/
પૂર્ણાંક Sqlitetest_autoext_Init(Tcl_Interp *interp){
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
  પાછા TCL_OK;
}
