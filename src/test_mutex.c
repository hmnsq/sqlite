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
#ಅನ್ತರ್ಭಾವ "tclsqlite.h"
#ಅನ್ತರ್ಭಾವ "sqlite3.h"
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <assert.h>
#ಅನ್ತರ್ಭಾವ <string.h>

#define MAX_MUTEXES        (SQLITE_MUTEX_STATIC_VFS3+1)
#define STATIC_MUTEXES     (MAX_MUTEXES-(SQLITE_MUTEX_RECURSIVE+1))

/* defined in main.c */
ಬಾಹ್ಯ ಧ್ರುವ ವರ್ಣ *sqlite3ErrName(ಪೂರ್ಣಾಙ್ಕ);
ಪೂರ್ಣಾಙ್ಕ getDbPointer(Tcl_Interp *interp, ಧ್ರುವ ವರ್ಣ *zA, sqlite3 **ppDb);

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *aName[MAX_MUTEXES+1] = {
  "fast",        "recursive",   "static_main",   "static_mem",
  "static_open", "static_prng", "static_lru",    "static_pmem",
  "static_app1", "static_app2", "static_app3",   "static_vfs1",
  "static_vfs2", "static_vfs3", 0
};

/* A countable mutex */
ಸಂರಚನಾ sqlite3_mutex {
  sqlite3_mutex *pReal;
  ಪೂರ್ಣಾಙ್ಕ eType;
};

/* State variables */
ಸ್ಥಿರ ಸಂರಚನಾ test_mutex_globals {
  ಪೂರ್ಣಾಙ್ಕ isInstalled;           /* True if installed */
  ಪೂರ್ಣಾಙ್ಕ disableInit;           /* True to cause sqlite3_initialize() to fail */
  ಪೂರ್ಣಾಙ್ಕ disableTry;            /* True to force sqlite3_mutex_try() to fail */
  ಪೂರ್ಣಾಙ್ಕ isInit;                /* True if initialized */
  sqlite3_mutex_methods m;   /* Interface to "real" mutex system */
  ಪೂರ್ಣಾಙ್ಕ aCounter[MAX_MUTEXES]; /* Number of grabs of each type of mutex */
  sqlite3_mutex aStatic[STATIC_MUTEXES]; /* The static mutexes */
} g = {0};

/* Return true if the countable mutex is currently held */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ counterMutexHeld(sqlite3_mutex *p){
  ಪ್ರತ್ಯಾಗಮ g.m.xMutexHeld(p->pReal);
}

/* Return true if the countable mutex is not currently held */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ counterMutexNotheld(sqlite3_mutex *p){
  ಪ್ರತ್ಯಾಗಮ g.m.xMutexNotheld(p->pReal);
}

/* Initialize the countable mutex interface
** Or, if g.disableInit is non-zero, then do not initialize but instead
** return the value of g.disableInit as the result code.  This can be used
** to simulate an initialization failure.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ counterMutexInit(ಶೂನ್ಯ){ 
  ಪೂರ್ಣಾಙ್ಕ rc;
  ಯದಿ( g.disableInit ) ಪ್ರತ್ಯಾಗಮ g.disableInit;
  rc = g.m.xMutexInit();
  g.isInit = 1;
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Uninitialize the mutex subsystem
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ counterMutexEnd(ಶೂನ್ಯ){ 
  g.isInit = 0;
  ಪ್ರತ್ಯಾಗಮ g.m.xMutexEnd();
}

/*
** Allocate a countable mutex
*/
ಸ್ಥಿರ sqlite3_mutex *counterMutexAlloc(ಪೂರ್ಣಾಙ್ಕ eType){
  sqlite3_mutex *pReal;
  sqlite3_mutex *pRet = 0;

  ಪ್ರತಿಜ್ಞಾ( g.isInit );
  ಪ್ರತಿಜ್ಞಾ( eType>=SQLITE_MUTEX_FAST );
  ಪ್ರತಿಜ್ಞಾ( eType<=SQLITE_MUTEX_STATIC_VFS3 );

  pReal = g.m.xMutexAlloc(eType);
  ಯದಿ( !pReal ) ಪ್ರತ್ಯಾಗಮ 0;

  ಯದಿ( eType==SQLITE_MUTEX_FAST || eType==SQLITE_MUTEX_RECURSIVE ){
    pRet = (sqlite3_mutex *)malloc(ಪರಿಮಾಣ(sqlite3_mutex));
  }ಅನ್ಯಥಾ{
    ಪೂರ್ಣಾಙ್ಕ eStaticType = eType - (MAX_MUTEXES - STATIC_MUTEXES);
    ಪ್ರತಿಜ್ಞಾ( eStaticType>=0 );
    ಪ್ರತಿಜ್ಞಾ( eStaticType<STATIC_MUTEXES );
    pRet = &g.aStatic[eStaticType];
  }

  pRet->eType = eType;
  pRet->pReal = pReal;
  ಪ್ರತ್ಯಾಗಮ pRet;
}

/*
** Free a countable mutex
*/
ಸ್ಥಿರ ಶೂನ್ಯ counterMutexFree(sqlite3_mutex *p){
  ಪ್ರತಿಜ್ಞಾ( g.isInit );
  g.m.xMutexFree(p->pReal);
  ಯದಿ( p->eType==SQLITE_MUTEX_FAST || p->eType==SQLITE_MUTEX_RECURSIVE ){
    ಮುಕ್ತ(p);
  }
}

/*
** Enter a countable mutex.  Block until entry is safe.
*/
ಸ್ಥಿರ ಶೂನ್ಯ counterMutexEnter(sqlite3_mutex *p){
  ಪ್ರತಿಜ್ಞಾ( g.isInit );
  ಪ್ರತಿಜ್ಞಾ( p->eType>=0 );
  ಪ್ರತಿಜ್ಞಾ( p->eType<MAX_MUTEXES );
  g.aCounter[p->eType]++;
  g.m.xMutexEnter(p->pReal);
}

/*
** Try to enter a mutex.  Return true on success.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ counterMutexTry(sqlite3_mutex *p){
  ಪ್ರತಿಜ್ಞಾ( g.isInit );
  ಪ್ರತಿಜ್ಞಾ( p->eType>=0 );
  ಪ್ರತಿಜ್ಞಾ( p->eType<MAX_MUTEXES );
  g.aCounter[p->eType]++;
  ಯದಿ( g.disableTry ) ಪ್ರತ್ಯಾಗಮ SQLITE_BUSY;
  ಪ್ರತ್ಯಾಗಮ g.m.xMutexTry(p->pReal);
}

/* Leave a mutex
*/
ಸ್ಥಿರ ಶೂನ್ಯ counterMutexLeave(sqlite3_mutex *p){
  ಪ್ರತಿಜ್ಞಾ( g.isInit );
  g.m.xMutexLeave(p->pReal);
}

/*
** sqlite3_shutdown
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_shutdown(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಙ್ಕ rc;

  ಯದಿ( objc!=1 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  rc = sqlite3_shutdown();
  Tcl_SetResult(interp, (ವರ್ಣ *)sqlite3ErrName(rc), TCL_VOLATILE);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** sqlite3_initialize
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_initialize(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಙ್ಕ rc;

  ಯದಿ( objc!=1 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  rc = sqlite3_initialize();
  Tcl_SetResult(interp, (ವರ್ಣ *)sqlite3ErrName(rc), TCL_VOLATILE);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** install_mutex_counters BOOLEAN
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_install_mutex_counters(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;
  ಪೂರ್ಣಾಙ್ಕ isInstall;

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

  ಯದಿ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "BOOLEAN");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[1], &isInstall) ){
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  ಪ್ರತಿಜ್ಞಾ(isInstall==0 || isInstall==1);
  ಪ್ರತಿಜ್ಞಾ(g.isInstalled==0 || g.isInstalled==1);
  ಯದಿ( isInstall==g.isInstalled ){
    Tcl_AppendResult(interp, "mutex counters are ", ರಿಕ್ತ);
    Tcl_AppendResult(interp, isInstall?"already installed":"not installed", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  ಯದಿ( isInstall ){
    ಪ್ರತಿಜ್ಞಾ( g.m.xMutexAlloc==0 );
    rc = sqlite3_config(SQLITE_CONFIG_GETMUTEX, &g.m);
    ಯದಿ( rc==SQLITE_OK ){
      sqlite3_config(SQLITE_CONFIG_MUTEX, &counter_methods);
    }
    g.disableTry = 0;
  }ಅನ್ಯಥಾ{
    ಪ್ರತಿಜ್ಞಾ( g.m.xMutexAlloc );
    rc = sqlite3_config(SQLITE_CONFIG_MUTEX, &g.m);
    memset(&g.m, 0, ಪರಿಮಾಣ(sqlite3_mutex_methods));
  }

  ಯದಿ( rc==SQLITE_OK ){
    g.isInstalled = isInstall;
  }

  Tcl_SetResult(interp, (ವರ್ಣ *)sqlite3ErrName(rc), TCL_VOLATILE);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** read_mutex_counters
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_read_mutex_counters(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Obj *pRet;
  ಪೂರ್ಣಾಙ್ಕ ii;

  ಯದಿ( objc!=1 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  pRet = Tcl_NewObj();
  Tcl_Incr_Ref_ಎಣಿಕೆ(pRet);
  ಪ್ರತಿ(ii=0; ii<MAX_MUTEXES; ii++){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj(aName[ii], -1));
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewIntObj(g.aCounter[ii]));
  }
  Tcl_SetObjResult(interp, pRet);
  Tcl_Decr_Ref_ಎಣಿಕೆ(pRet);

  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** clear_mutex_counters
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_clear_mutex_counters(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಙ್ಕ ii;

  ಯದಿ( objc!=1 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  ಪ್ರತಿ(ii=0; ii<MAX_MUTEXES; ii++){
    g.aCounter[ii] = 0;
  }
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** Create and free a mutex.  Return the mutex pointer.  The pointer
** will be invalid since the mutex has already been freed.  The
** return pointer just checks to see if the mutex really was allocated.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_alloc_mutex(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
#ಯದಿ SQLITE_THREADSAFE
  sqlite3_mutex *p = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
  ವರ್ಣ zBuf[100];
  sqlite3_mutex_free(p);
  sqlite3_snprintf(ಪರಿಮಾಣ(zBuf), zBuf, "%p", p);
  Tcl_AppendResult(interp, zBuf, (ವರ್ಣ*)0);
#endif
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_config(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಸಂರಚನಾ ConfigOption {
    ಧ್ರುವ ವರ್ಣ *zName;
    ಪೂರ್ಣಾಙ್ಕ i_ಮೌಲ್ಯ;
  } aOpt[] = {
    {"singlethread", SQLITE_CONFIG_SINGLETHREAD},
    {"multithread",  SQLITE_CONFIG_MULTITHREAD},
    {"serialized",   SQLITE_CONFIG_SERIALIZED},
    {0, 0}
  };
  ಪೂರ್ಣಾಙ್ಕ s = ಪರಿಮಾಣ(ಸಂರಚನಾ ConfigOption);
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪೂರ್ಣಾಙ್ಕ rc;

  ಯದಿ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  ಯದಿ( Tcl_GetIndexFromObjStruct(interp, objv[1], aOpt, s, "flag", 0, &i) ){
    ಯದಿ( Tcl_GetIntFromObj(interp, objv[1], &i) ){
      ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
    }
  }ಅನ್ಯಥಾ{
    i = aOpt[i].i_ಮೌಲ್ಯ;
  }

  rc = sqlite3_config(i);
  Tcl_SetResult(interp, (ವರ್ಣ *)sqlite3ErrName(rc), TCL_VOLATILE);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

ಸ್ಥಿರ sqlite3_mutex *getStaticMutexPointer(
  Tcl_Interp *pInterp,
  Tcl_Obj *pObj
){
  ಪೂರ್ಣಾಙ್ಕ iMutex;
  ಯದಿ( Tcl_GetIndexFromObj(pInterp, pObj, aName, "mutex name", 0, &iMutex) ){
    ಪ್ರತ್ಯಾಗಮ 0;
  }
  ಪ್ರತಿಜ್ಞಾ( iMutex!=SQLITE_MUTEX_FAST && iMutex!=SQLITE_MUTEX_RECURSIVE );
  ಪ್ರತ್ಯಾಗಮ counterMutexAlloc(iMutex);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_enter_static_mutex(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_mutex *pMutex;
  ಯದಿ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "NAME");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  pMutex = getStaticMutexPointer(interp, objv[1]);
  ಯದಿ( !pMutex ){
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  sqlite3_mutex_enter(pMutex);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_leave_static_mutex(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_mutex *pMutex;
  ಯದಿ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "NAME");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  pMutex = getStaticMutexPointer(interp, objv[1]);
  ಯದಿ( !pMutex ){
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  sqlite3_mutex_leave(pMutex);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_enter_db_mutex(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  ಯದಿ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "DB");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ){
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( !db ){
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  sqlite3_mutex_enter(sqlite3_db_mutex(db));
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_leave_db_mutex(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  ಯದಿ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "DB");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ){
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( !db ){
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  sqlite3_mutex_leave(sqlite3_db_mutex(db));
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

ಪೂರ್ಣಾಙ್ಕ Sqlitetest_mutex_Init(Tcl_Interp *interp){
  ಸ್ಥಿರ ಸಂರಚನಾ {
    ವರ್ಣ *zName;
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
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪ್ರತಿ(i=0; i<ಪರಿಮಾಣ(aCmd)/ಪರಿಮಾಣ(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  Tcl_LinkVar(interp, "disable_mutex_init", 
              (ವರ್ಣ*)&g.disableInit, TCL_LINK_INT);
  Tcl_LinkVar(interp, "disable_mutex_try", 
              (ವರ್ಣ*)&g.disableTry, TCL_LINK_INT);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}
