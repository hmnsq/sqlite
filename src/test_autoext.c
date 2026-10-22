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
#ಅನ್ತರ್ಭಾವ "tclsqlite.h"
#ಅನ್ತರ್ಭಾವ "sqlite3ext.h"

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_ತುಂಬು_EXTENSION
SQLITE_EXTENSION_INIT1

/*
** The sqr() SQL function returns the square of its input value.
*/
ಸ್ಥಿರ ಶೂನ್ಯ sqrFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ದ್ವಿಗುಣ r = sqlite3_ಮೌಲ್ಯ_double(argv[0]);
  sqlite3_ಫಲಿತಾಂಶ_double(context, r*r);
}

/*
** This is the entry point to register the extension for the sqr() function.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ sqr_init(
  sqlite3 *db, 
  ಅಕ್ಷರ **pz_Err_ಸಂದೇಶ, 
  ಸ್ಥಿರಾಂಕ sqlite3_api_routines *pApi
){
  SQLITE_EXTENSION_INIT2(pApi);
  sqlite3_create_function(db, "sqr", 1, SQLITE_ANY, 0, sqrFunc, 0, 0);
  ಹಿಂತಿರುಗಿ 0;
}

/*
** The cube() SQL function returns the cube of its input value.
*/
ಸ್ಥಿರ ಶೂನ್ಯ cubeFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ದ್ವಿಗುಣ r = sqlite3_ಮೌಲ್ಯ_double(argv[0]);
  sqlite3_ಫಲಿತಾಂಶ_double(context, r*r*r);
}

/*
** This is the entry point to register the extension for the cube() function.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ cube_init(
  sqlite3 *db, 
  ಅಕ್ಷರ **pz_Err_ಸಂದೇಶ, 
  ಸ್ಥಿರಾಂಕ sqlite3_api_routines *pApi
){
  SQLITE_EXTENSION_INIT2(pApi);
  sqlite3_create_function(db, "cube", 1, SQLITE_ANY, 0, cubeFunc, 0, 0);
  ಹಿಂತಿರುಗಿ 0;
}

/*
** This is a broken extension entry point
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ broken_init(
  sqlite3 *db, 
  ಅಕ್ಷರ **pz_Err_ಸಂದೇಶ, 
  ಸ್ಥಿರಾಂಕ sqlite3_api_routines *pApi
){
  ಅಕ್ಷರ *zErr;
  SQLITE_EXTENSION_INIT2(pApi);
  zErr = sqlite3_mprintf("broken autoext!");
  *pz_Err_ಸಂದೇಶ = zErr;
  ಹಿಂತಿರುಗಿ 1;
}

/*
** tclcmd:   sqlite3_auto_extension_sqr
**
** Register the "sqr" extension to be loaded automatically.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI autoExtSqrObjCmd(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಂಕ rc = sqlite3_auto_extension((ಶೂನ್ಯ(*)(ಶೂನ್ಯ))sqr_init);
  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewIntObj(rc));
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** tclcmd:   sqlite3_cancel_auto_extension_sqr
**
** Unregister the "sqr" extension.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI cancelAutoExtSqrObjCmd(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಂಕ rc = sqlite3_cancel_auto_extension((ಶೂನ್ಯ(*)(ಶೂನ್ಯ))sqr_init);
  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewIntObj(rc));
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** tclcmd:   sqlite3_auto_extension_cube
**
** Register the "cube" extension to be loaded automatically.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI autoExtCubeObjCmd(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಂಕ rc = sqlite3_auto_extension((ಶೂನ್ಯ(*)(ಶೂನ್ಯ))cube_init);
  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewIntObj(rc));
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** tclcmd:   sqlite3_cancel_auto_extension_cube
**
** Unregister the "cube" extension.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI cancelAutoExtCubeObjCmd(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಂಕ rc = sqlite3_cancel_auto_extension((ಶೂನ್ಯ(*)(ಶೂನ್ಯ))cube_init);
  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewIntObj(rc));
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** tclcmd:   sqlite3_auto_extension_broken
**
** Register the broken extension to be loaded automatically.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI autoExtBrokenObjCmd(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಂಕ rc = sqlite3_auto_extension((ಶೂನ್ಯ(*)(ಶೂನ್ಯ))broken_init);
  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewIntObj(rc));
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** tclcmd:   sqlite3_cancel_auto_extension_broken
**
** Unregister the broken extension.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI cancelAutoExtBrokenObjCmd(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಂಕ rc = sqlite3_cancel_auto_extension((ಶೂನ್ಯ(*)(ಶೂನ್ಯ))broken_init);
  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewIntObj(rc));
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_LOAD_EXTENSION */


/*
** tclcmd:   sqlite3_reset_auto_extension
**
** Reset all auto-extensions
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI resetAutoExtObjCmd(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_reset_auto_extension();
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}


/*
** This procedure registers the TCL procs defined in this file.
*/
ಪೂರ್ಣಾಂಕ Sqlitetest_autoext_Init(Tcl_Interp *interp){
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_ತುಂಬು_EXTENSION
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
#ಆದರೆ_ಅಂತ್ಯ
  Tcl_CreateObjCommand(interp, "sqlite3_reset_auto_extension",
          resetAutoExtObjCmd, 0, 0);
  ಹಿಂತಿರುಗಿ TCL_OK;
}
