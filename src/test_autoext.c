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
#समावेश "tclsqlite.h"
#समावेश "sqlite3ext.h"

#अगर_अघोषित SQLITE_OMIT_LOAD_EXTENSION
SQLITE_EXTENSION_INIT1

/*
** The sqr() SQL function returns the square of its input value.
*/
अटल व्योम sqrFunc(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  द्विगुन r = sqlite3_मूल्य_double(argv[0]);
  sqlite3_परिणाम_double(context, r*r);
}

/*
** This is the entry point to register the extension for the sqr() function.
*/
अटल पूर्णांक sqr_init(
  sqlite3 *db, 
  अक्षर **pz_Err_संदेश, 
  स्थिर sqlite3_api_routines *pApi
){
  SQLITE_EXTENSION_INIT2(pApi);
  sqlite3_create_function(db, "sqr", 1, SQLITE_ANY, 0, sqrFunc, 0, 0);
  वापस 0;
}

/*
** The cube() SQL function returns the cube of its input value.
*/
अटल व्योम cubeFunc(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  द्विगुन r = sqlite3_मूल्य_double(argv[0]);
  sqlite3_परिणाम_double(context, r*r*r);
}

/*
** This is the entry point to register the extension for the cube() function.
*/
अटल पूर्णांक cube_init(
  sqlite3 *db, 
  अक्षर **pz_Err_संदेश, 
  स्थिर sqlite3_api_routines *pApi
){
  SQLITE_EXTENSION_INIT2(pApi);
  sqlite3_create_function(db, "cube", 1, SQLITE_ANY, 0, cubeFunc, 0, 0);
  वापस 0;
}

/*
** This is a broken extension entry point
*/
अटल पूर्णांक broken_init(
  sqlite3 *db, 
  अक्षर **pz_Err_संदेश, 
  स्थिर sqlite3_api_routines *pApi
){
  अक्षर *zErr;
  SQLITE_EXTENSION_INIT2(pApi);
  zErr = sqlite3_mprintf("broken autoext!");
  *pz_Err_संदेश = zErr;
  वापस 1;
}

/*
** tclcmd:   sqlite3_auto_extension_sqr
**
** Register the "sqr" extension to be loaded automatically.
*/
अटल पूर्णांक SQLITE_TCLAPI autoExtSqrObjCmd(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक rc = sqlite3_auto_extension((व्योम(*)(व्योम))sqr_init);
  Tcl_Set_Obj_परिणाम(interp, Tcl_NewIntObj(rc));
  वापस SQLITE_OK;
}

/*
** tclcmd:   sqlite3_cancel_auto_extension_sqr
**
** Unregister the "sqr" extension.
*/
अटल पूर्णांक SQLITE_TCLAPI cancelAutoExtSqrObjCmd(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक rc = sqlite3_cancel_auto_extension((व्योम(*)(व्योम))sqr_init);
  Tcl_Set_Obj_परिणाम(interp, Tcl_NewIntObj(rc));
  वापस SQLITE_OK;
}

/*
** tclcmd:   sqlite3_auto_extension_cube
**
** Register the "cube" extension to be loaded automatically.
*/
अटल पूर्णांक SQLITE_TCLAPI autoExtCubeObjCmd(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक rc = sqlite3_auto_extension((व्योम(*)(व्योम))cube_init);
  Tcl_Set_Obj_परिणाम(interp, Tcl_NewIntObj(rc));
  वापस SQLITE_OK;
}

/*
** tclcmd:   sqlite3_cancel_auto_extension_cube
**
** Unregister the "cube" extension.
*/
अटल पूर्णांक SQLITE_TCLAPI cancelAutoExtCubeObjCmd(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक rc = sqlite3_cancel_auto_extension((व्योम(*)(व्योम))cube_init);
  Tcl_Set_Obj_परिणाम(interp, Tcl_NewIntObj(rc));
  वापस SQLITE_OK;
}

/*
** tclcmd:   sqlite3_auto_extension_broken
**
** Register the broken extension to be loaded automatically.
*/
अटल पूर्णांक SQLITE_TCLAPI autoExtBrokenObjCmd(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक rc = sqlite3_auto_extension((व्योम(*)(व्योम))broken_init);
  Tcl_Set_Obj_परिणाम(interp, Tcl_NewIntObj(rc));
  वापस SQLITE_OK;
}

/*
** tclcmd:   sqlite3_cancel_auto_extension_broken
**
** Unregister the broken extension.
*/
अटल पूर्णांक SQLITE_TCLAPI cancelAutoExtBrokenObjCmd(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक rc = sqlite3_cancel_auto_extension((व्योम(*)(व्योम))broken_init);
  Tcl_Set_Obj_परिणाम(interp, Tcl_NewIntObj(rc));
  वापस SQLITE_OK;
}

#पूर्ण_अगर /* SQLITE_OMIT_LOAD_EXTENSION */


/*
** tclcmd:   sqlite3_reset_auto_extension
**
** Reset all auto-extensions
*/
अटल पूर्णांक SQLITE_TCLAPI resetAutoExtObjCmd(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_reset_auto_extension();
  वापस SQLITE_OK;
}


/*
** This procedure registers the TCL procs defined in this file.
*/
पूर्णांक Sqlitetest_autoext_Init(Tcl_Interp *interp){
#अगर_अघोषित SQLITE_OMIT_LOAD_EXTENSION
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
#पूर्ण_अगर
  Tcl_CreateObjCommand(interp, "sqlite3_reset_auto_extension",
          resetAutoExtObjCmd, 0, 0);
  वापस TCL_OK;
}
