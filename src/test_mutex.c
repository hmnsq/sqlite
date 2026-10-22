/*
** 2008 June 18
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file contains test logic for the sqlite3_mutex interfaces.
*/
#अन्तर्भाव "tclsqlite.h"
#अन्तर्भाव "sqlite3.h"
#अन्तर्भाव "sqliteInt.h"
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <assert.h>
#अन्तर्भाव <string.h>

#define MAX_MUTEXES        (SQLITE_MUTEX_STATIC_VFS3+1)
#define STATIC_MUTEXES     (MAX_MUTEXES-(SQLITE_MUTEX_RECURSIVE+1))

/* defined in main.c */
बाह्य स्थिरांक अक्षर *sqlite3ErrName(पूर्णांक);
पूर्णांक getDbPointer(Tcl_Interp *interp, स्थिरांक अक्षर *zA, sqlite3 **ppDb);

स्थिर स्थिरांक अक्षर *aName[MAX_MUTEXES+1] = {
  "fast",        "recursive",   "static_main",   "static_mem",
  "static_open", "static_prng", "static_lru",    "static_pmem",
  "static_app1", "static_app2", "static_app3",   "static_vfs1",
  "static_vfs2", "static_vfs3", 0
};

/* A countable mutex */
संरचना sqlite3_mutex {
  sqlite3_mutex *pReal;
  पूर्णांक eType;
};

/* State variables */
स्थिर संरचना test_mutex_globals {
  पूर्णांक isInstalled;           /* True if installed */
  पूर्णांक disableInit;           /* True to cause sqlite3_initialize() to fail */
  पूर्णांक disableTry;            /* True to force sqlite3_mutex_try() to fail */
  पूर्णांक isInit;                /* True if initialized */
  sqlite3_mutex_methods m;   /* Interface to "real" mutex system */
  पूर्णांक aCounter[MAX_MUTEXES]; /* Number of grabs of each type of mutex */
  sqlite3_mutex aStatic[STATIC_MUTEXES]; /* The static mutexes */
} g = {0};

/* Return true if the countable mutex is currently held */
स्थिर पूर्णांक counterMutexHeld(sqlite3_mutex *p){
  परत g.m.xMutexHeld(p->pReal);
}

/* Return true if the countable mutex is not currently held */
स्थिर पूर्णांक counterMutexNotheld(sqlite3_mutex *p){
  परत g.m.xMutexNotheld(p->pReal);
}

/* Initialize the countable mutex interface
** Or, if g.disableInit is non-zero, then do not initialize but instead
** return the value of g.disableInit as the result code.  This can be used
** to simulate an initialization failure.
*/
स्थिर पूर्णांक counterMutexInit(शून्य){ 
  पूर्णांक rc;
  जर( g.disableInit ) परत g.disableInit;
  rc = g.m.xMutexInit();
  g.isInit = 1;
  परत rc;
}

/*
** Uninitialize the mutex subsystem
*/
स्थिर पूर्णांक counterMutexEnd(शून्य){ 
  g.isInit = 0;
  परत g.m.xMutexEnd();
}

/*
** Allocate a countable mutex
*/
स्थिर sqlite3_mutex *counterMutexAlloc(पूर्णांक eType){
  sqlite3_mutex *pReal;
  sqlite3_mutex *pRet = 0;

  प्रतिज्ञा( g.isInit );
  प्रतिज्ञा( eType>=SQLITE_MUTEX_FAST );
  प्रतिज्ञा( eType<=SQLITE_MUTEX_STATIC_VFS3 );

  pReal = g.m.xMutexAlloc(eType);
  जर( !pReal ) परत 0;

  जर( eType==SQLITE_MUTEX_FAST || eType==SQLITE_MUTEX_RECURSIVE ){
    pRet = (sqlite3_mutex *)दो_स्मृति(आकार(sqlite3_mutex));
  }अन्यथा{
    पूर्णांक eStaticType = eType - (MAX_MUTEXES - STATIC_MUTEXES);
    प्रतिज्ञा( eStaticType>=0 );
    प्रतिज्ञा( eStaticType<STATIC_MUTEXES );
    pRet = &g.aStatic[eStaticType];
  }

  pRet->eType = eType;
  pRet->pReal = pReal;
  परत pRet;
}

/*
** Free a countable mutex
*/
स्थिर शून्य counterMutexFree(sqlite3_mutex *p){
  प्रतिज्ञा( g.isInit );
  g.m.xMutexFree(p->pReal);
  जर( p->eType==SQLITE_MUTEX_FAST || p->eType==SQLITE_MUTEX_RECURSIVE ){
    मुक्त(p);
  }
}

/*
** Enter a countable mutex.  Block until entry is safe.
*/
स्थिर शून्य counterMutexEnter(sqlite3_mutex *p){
  प्रतिज्ञा( g.isInit );
  प्रतिज्ञा( p->eType>=0 );
  प्रतिज्ञा( p->eType<MAX_MUTEXES );
  g.aCounter[p->eType]++;
  g.m.xMutexEnter(p->pReal);
}

/*
** Try to enter a mutex.  Return true on success.
*/
स्थिर पूर्णांक counterMutexTry(sqlite3_mutex *p){
  प्रतिज्ञा( g.isInit );
  प्रतिज्ञा( p->eType>=0 );
  प्रतिज्ञा( p->eType<MAX_MUTEXES );
  g.aCounter[p->eType]++;
  जर( g.disableTry ) परत SQLITE_BUSY;
  परत g.m.xMutexTry(p->pReal);
}

/* Leave a mutex
*/
स्थिर शून्य counterMutexLeave(sqlite3_mutex *p){
  प्रतिज्ञा( g.isInit );
  g.m.xMutexLeave(p->pReal);
}

/*
** sqlite3_shutdown
*/
स्थिर पूर्णांक SQLITE_TCLAPI test_shutdown(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक rc;

  जर( objc!=1 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "");
    परत TCL_ERROR;
  }

  rc = sqlite3_shutdown();
  Tcl_SetResult(interp, (अक्षर *)sqlite3ErrName(rc), TCL_VOLATILE);
  परत TCL_OK;
}

/*
** sqlite3_initialize
*/
स्थिर पूर्णांक SQLITE_TCLAPI test_initialize(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक rc;

  जर( objc!=1 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "");
    परत TCL_ERROR;
  }

  rc = sqlite3_initialize();
  Tcl_SetResult(interp, (अक्षर *)sqlite3ErrName(rc), TCL_VOLATILE);
  परत TCL_OK;
}

/*
** install_mutex_counters BOOLEAN
*/
स्थिर पूर्णांक SQLITE_TCLAPI test_install_mutex_counters(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक rc = SQLITE_OK;
  पूर्णांक isInstall;

  sqlite3_mutex_methods counter_methods = {
    counterMutexInit,
    counterMutexEnd,
    counterMutexAlloc,
    counterMutexFree,
    counterMutexEnter,
    counterMutexTry,
    counterMutexLeave,
    counterMutexHeld,
    counterMutexNotheld
  };

  जर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "BOOLEAN");
    परत TCL_ERROR;
  }
  जर( TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[1], &isInstall) ){
    परत TCL_ERROR;
  }

  प्रतिज्ञा(isInstall==0 || isInstall==1);
  प्रतिज्ञा(g.isInstalled==0 || g.isInstalled==1);
  जर( isInstall==g.isInstalled ){
    Tcl_AppendResult(interp, "mutex counters are ", रिक्त);
    Tcl_AppendResult(interp, isInstall?"already installed":"not installed", रिक्त);
    परत TCL_ERROR;
  }

  जर( isInstall ){
    प्रतिज्ञा( g.m.xMutexAlloc==0 );
    rc = sqlite3_config(SQLITE_CONFIG_GETMUTEX, &g.m);
    जर( rc==SQLITE_OK ){
      sqlite3_config(SQLITE_CONFIG_MUTEX, &counter_methods);
    }
    g.disableTry = 0;
  }अन्यथा{
    प्रतिज्ञा( g.m.xMutexAlloc );
    rc = sqlite3_config(SQLITE_CONFIG_MUTEX, &g.m);
    स_रखो(&g.m, 0, आकार(sqlite3_mutex_methods));
  }

  जर( rc==SQLITE_OK ){
    g.isInstalled = isInstall;
  }

  Tcl_SetResult(interp, (अक्षर *)sqlite3ErrName(rc), TCL_VOLATILE);
  परत TCL_OK;
}

/*
** read_mutex_counters
*/
स्थिर पूर्णांक SQLITE_TCLAPI test_read_mutex_counters(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Obj *pRet;
  पूर्णांक ii;

  जर( objc!=1 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "");
    परत TCL_ERROR;
  }

  pRet = Tcl_NewObj();
  Tcl_Incr_Ref_मोजणी(pRet);
  साठी(ii=0; ii<MAX_MUTEXES; ii++){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj(aName[ii], -1));
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewIntObj(g.aCounter[ii]));
  }
  Tcl_SetObjResult(interp, pRet);
  Tcl_Decr_Ref_मोजणी(pRet);

  परत TCL_OK;
}

/*
** clear_mutex_counters
*/
स्थिर पूर्णांक SQLITE_TCLAPI test_clear_mutex_counters(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक ii;

  जर( objc!=1 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "");
    परत TCL_ERROR;
  }

  साठी(ii=0; ii<MAX_MUTEXES; ii++){
    g.aCounter[ii] = 0;
  }
  परत TCL_OK;
}

/*
** Create and free a mutex.  Return the mutex pointer.  The pointer
** will be invalid since the mutex has already been freed.  The
** return pointer just checks to see if the mutex really was allocated.
*/
स्थिर पूर्णांक SQLITE_TCLAPI test_alloc_mutex(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
#जर SQLITE_THREADSAFE
  sqlite3_mutex *p = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
  अक्षर zBuf[100];
  sqlite3_mutex_free(p);
  sqlite3_snprintf(आकार(zBuf), zBuf, "%p", p);
  Tcl_AppendResult(interp, zBuf, (अक्षर*)0);
#endif
  परत TCL_OK;
}

/*
** sqlite3_config OPTION
**
** OPTION can be either one of the keywords:
**
**            SQLITE_CONFIG_SINGLETHREAD
**            SQLITE_CONFIG_MULTITHREAD
**            SQLITE_CONFIG_SERIALIZED
**
** Or OPTION can be an raw integer.
*/
स्थिर पूर्णांक SQLITE_TCLAPI test_config(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  संरचना ConfigOption {
    स्थिरांक अक्षर *zName;
    पूर्णांक i_मूल्य;
  } aOpt[] = {
    {"singlethread", SQLITE_CONFIG_SINGLETHREAD},
    {"multithread",  SQLITE_CONFIG_MULTITHREAD},
    {"serialized",   SQLITE_CONFIG_SERIALIZED},
    {0, 0}
  };
  पूर्णांक s = आकार(संरचना ConfigOption);
  पूर्णांक i;
  पूर्णांक rc;

  जर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "");
    परत TCL_ERROR;
  }

  जर( Tcl_GetIndexFromObjStruct(interp, objv[1], aOpt, s, "flag", 0, &i) ){
    जर( Tcl_GetIntFromObj(interp, objv[1], &i) ){
      परत TCL_ERROR;
    }
  }अन्यथा{
    i = aOpt[i].i_मूल्य;
  }

  rc = sqlite3_config(i);
  Tcl_SetResult(interp, (अक्षर *)sqlite3ErrName(rc), TCL_VOLATILE);
  परत TCL_OK;
}

स्थिर sqlite3_mutex *getStaticMutexPointer(
  Tcl_Interp *pInterp,
  Tcl_Obj *pObj
){
  पूर्णांक iMutex;
  जर( Tcl_GetIndexFromObj(pInterp, pObj, aName, "mutex name", 0, &iMutex) ){
    परत 0;
  }
  प्रतिज्ञा( iMutex!=SQLITE_MUTEX_FAST && iMutex!=SQLITE_MUTEX_RECURSIVE );
  परत counterMutexAlloc(iMutex);
}

स्थिर पूर्णांक SQLITE_TCLAPI test_enter_static_mutex(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_mutex *pMutex;
  जर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "NAME");
    परत TCL_ERROR;
  }
  pMutex = getStaticMutexPointer(interp, objv[1]);
  जर( !pMutex ){
    परत TCL_ERROR;
  }
  sqlite3_mutex_enter(pMutex);
  परत TCL_OK;
}

स्थिर पूर्णांक SQLITE_TCLAPI test_leave_static_mutex(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_mutex *pMutex;
  जर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "NAME");
    परत TCL_ERROR;
  }
  pMutex = getStaticMutexPointer(interp, objv[1]);
  जर( !pMutex ){
    परत TCL_ERROR;
  }
  sqlite3_mutex_leave(pMutex);
  परत TCL_OK;
}

स्थिर पूर्णांक SQLITE_TCLAPI test_enter_db_mutex(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  जर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "DB");
    परत TCL_ERROR;
  }
  जर( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ){
    परत TCL_ERROR;
  }
  जर( !db ){
    परत TCL_ERROR;
  }
  sqlite3_mutex_enter(sqlite3_db_mutex(db));
  परत TCL_OK;
}

स्थिर पूर्णांक SQLITE_TCLAPI test_leave_db_mutex(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  जर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "DB");
    परत TCL_ERROR;
  }
  जर( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ){
    परत TCL_ERROR;
  }
  जर( !db ){
    परत TCL_ERROR;
  }
  sqlite3_mutex_leave(sqlite3_db_mutex(db));
  परत TCL_OK;
}

पूर्णांक Sqlitetest_mutex_Init(Tcl_Interp *interp){
  स्थिर संरचना {
    अक्षर *zName;
    Tcl_ObjCmdProc *xProc;
  } aCmd[] = {
    { "sqlite3_shutdown",        (Tcl_ObjCmdProc*)test_shutdown },
    { "sqlite3_initialize",      (Tcl_ObjCmdProc*)test_initialize },
    { "sqlite3_config",          (Tcl_ObjCmdProc*)test_config },

    { "enter_static_mutex",      (Tcl_ObjCmdProc*)test_enter_static_mutex },
    { "leave_static_mutex",      (Tcl_ObjCmdProc*)test_leave_static_mutex },

    { "enter_db_mutex",          (Tcl_ObjCmdProc*)test_enter_db_mutex },
    { "leave_db_mutex",          (Tcl_ObjCmdProc*)test_leave_db_mutex },

    { "alloc_dealloc_mutex",     (Tcl_ObjCmdProc*)test_alloc_mutex },
    { "install_mutex_counters",  (Tcl_ObjCmdProc*)test_install_mutex_counters },
    { "read_mutex_counters",     (Tcl_ObjCmdProc*)test_read_mutex_counters },
    { "clear_mutex_counters",    (Tcl_ObjCmdProc*)test_clear_mutex_counters },
  };
  पूर्णांक i;
  साठी(i=0; i<आकार(aCmd)/आकार(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  Tcl_LinkVar(interp, "disable_mutex_init", 
              (अक्षर*)&g.disableInit, TCL_LINK_INT);
  Tcl_LinkVar(interp, "disable_mutex_try", 
              (अक्षर*)&g.disableTry, TCL_LINK_INT);
  परत SQLITE_OK;
}
