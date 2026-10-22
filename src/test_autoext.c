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
#अन्तर्भाव "tclsqlite.h"
#अन्तर्भाव "sqlite3ext.h"

#ifndef SQLITE_OMIT_LOAD_EXTENSION
SQLITE_EXTENSION_INIT1

/*
** The sqr() SQL function returns the square of its input value.
*/
स्थिर शून्य sqrFunc(
  sqlite3_context *context,
  पूर्णाङ्क argc,
  sqlite3_मानम् **argv
){
  द्विगुण r = sqlite3_मानम्_double(argv[0]);
  sqlite3_result_double(context, r*r);
}

/*
** This is the entry point to register the extension for the sqr() function.
*/
स्थिर पूर्णाङ्क sqr_init(
  sqlite3 *db, 
  वर्ण **pz_Err_सन्देशः, 
  ध्रुव sqlite3_api_routines *pApi
){
  SQLITE_EXTENSION_INIT2(pApi);
  sqlite3_create_function(db, "sqr", 1, SQLITE_ANY, 0, sqrFunc, 0, 0);
  प्रत्यागम 0;
}

/*
** The cube() SQL function returns the cube of its input value.
*/
स्थिर शून्य cubeFunc(
  sqlite3_context *context,
  पूर्णाङ्क argc,
  sqlite3_मानम् **argv
){
  द्विगुण r = sqlite3_मानम्_double(argv[0]);
  sqlite3_result_double(context, r*r*r);
}

/*
** This is the entry point to register the extension for the cube() function.
*/
स्थिर पूर्णाङ्क cube_init(
  sqlite3 *db, 
  वर्ण **pz_Err_सन्देशः, 
  ध्रुव sqlite3_api_routines *pApi
){
  SQLITE_EXTENSION_INIT2(pApi);
  sqlite3_create_function(db, "cube", 1, SQLITE_ANY, 0, cubeFunc, 0, 0);
  प्रत्यागम 0;
}

/*
** This is a broken extension entry point
*/
स्थिर पूर्णाङ्क broken_init(
  sqlite3 *db, 
  वर्ण **pz_Err_सन्देशः, 
  ध्रुव sqlite3_api_routines *pApi
){
  वर्ण *zErr;
  SQLITE_EXTENSION_INIT2(pApi);
  zErr = sqlite3_mprintf("broken autoext!");
  *pz_Err_सन्देशः = zErr;
  प्रत्यागम 1;
}

/*
** tclcmd:   sqlite3_auto_extension_sqr
**
** Register the "sqr" extension to be loaded automatically.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI autoExtSqrObjCmd(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क rc = sqlite3_auto_extension((शून्य(*)(शून्य))sqr_init);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  प्रत्यागम SQLITE_OK;
}

/*
** tclcmd:   sqlite3_cancel_auto_extension_sqr
**
** Unregister the "sqr" extension.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI cancelAutoExtSqrObjCmd(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क rc = sqlite3_cancel_auto_extension((शून्य(*)(शून्य))sqr_init);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  प्रत्यागम SQLITE_OK;
}

/*
** tclcmd:   sqlite3_auto_extension_cube
**
** Register the "cube" extension to be loaded automatically.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI autoExtCubeObjCmd(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क rc = sqlite3_auto_extension((शून्य(*)(शून्य))cube_init);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  प्रत्यागम SQLITE_OK;
}

/*
** tclcmd:   sqlite3_cancel_auto_extension_cube
**
** Unregister the "cube" extension.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI cancelAutoExtCubeObjCmd(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क rc = sqlite3_cancel_auto_extension((शून्य(*)(शून्य))cube_init);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  प्रत्यागम SQLITE_OK;
}

/*
** tclcmd:   sqlite3_auto_extension_broken
**
** Register the broken extension to be loaded automatically.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI autoExtBrokenObjCmd(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क rc = sqlite3_auto_extension((शून्य(*)(शून्य))broken_init);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  प्रत्यागम SQLITE_OK;
}

/*
** tclcmd:   sqlite3_cancel_auto_extension_broken
**
** Unregister the broken extension.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI cancelAutoExtBrokenObjCmd(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क rc = sqlite3_cancel_auto_extension((शून्य(*)(शून्य))broken_init);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  प्रत्यागम SQLITE_OK;
}

#endif /* SQLITE_OMIT_LOAD_EXTENSION */


/*
** tclcmd:   sqlite3_reset_auto_extension
**
** Reset all auto-extensions
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI resetAutoExtObjCmd(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_reset_auto_extension();
  प्रत्यागम SQLITE_OK;
}


/*
** This procedure registers the TCL procs defined in this file.
*/
पूर्णाङ्क Sqlitetest_autoext_Init(Tcl_Interp *interp){
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
  प्रत्यागम TCL_OK;
}
