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

#ವ್ಯಾಖ್ಯಾನಿಸು MAX_MUTEXES        (SQLITE_MUTEX_STATIC_VFS3+1)
#ವ್ಯಾಖ್ಯಾನಿಸು STATIC_MUTEXES     (MAX_MUTEXES-(SQLITE_MUTEX_RECURSIVE+1))

/* defined in main.c */
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3ErrName(ಪೂರ್ಣಾಂಕ);
ಪೂರ್ಣಾಂಕ get_Db_ಸೂಚಕ(Tcl_Interp *interp, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zA, sqlite3 **ppDb);

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *aName[MAX_MUTEXES+1] = {
  "fast",        "recursive",   "static_main",   "static_mem",
  "static_open", "static_prng", "static_lru",    "static_pmem",
  "static_app1", "static_app2", "static_app3",   "static_vfs1",
  "static_vfs2", "static_vfs3", 0
};

/* A countable mutex */
ರಚನೆ sqlite3_mutex {
  sqlite3_mutex *pReal;
  ಪೂರ್ಣಾಂಕ eType;
};

/* State variables */
ಸ್ಥಿರ ರಚನೆ test_mutex_globals {
  ಪೂರ್ಣಾಂಕ isInstalled;           /* True if installed */
  ಪೂರ್ಣಾಂಕ disableInit;           /* True to cause sqlite3_initialize() to fail */
  ಪೂರ್ಣಾಂಕ disableTry;            /* True to force sqlite3_mutex_try() to fail */
  ಪೂರ್ಣಾಂಕ isInit;                /* True if initialized */
  sqlite3_mutex_methods m;   /* Interface to "real" mutex system */
  ಪೂರ್ಣಾಂಕ aCounter[MAX_MUTEXES]; /* Number of grabs of each type of mutex */
  sqlite3_mutex aStatic[STATIC_MUTEXES]; /* The static mutexes */
} g = {0};

/* Return true if the countable mutex is currently held */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ counterMutexHeld(sqlite3_mutex *p){
  ಹಿಂತಿರುಗಿ g.m.xMutexHeld(p->pReal);
}

/* Return true if the countable mutex is not currently held */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ counterMutexNotheld(sqlite3_mutex *p){
  ಹಿಂತಿರುಗಿ g.m.xMutexNotheld(p->pReal);
}

/* Initialize the countable mutex interface
** Or, if g.disableInit is non-zero, then do not initialize but instead
** return the value of g.disableInit as the result code.  This can be used
** to simulate an initialization failure.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ counterMutexInit(ಶೂನ್ಯ){ 
  ಪೂರ್ಣಾಂಕ rc;
  ಆದರೆ( g.disableInit ) ಹಿಂತಿರುಗಿ g.disableInit;
  rc = g.m.xMutexInit();
  g.isInit = 1;
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Uninitialize the mutex subsystem
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ counter_Mutex_ಕೊನೆ(ಶೂನ್ಯ){ 
  g.isInit = 0;
  ಹಿಂತಿರುಗಿ g.m.x_Mutex_ಕೊನೆ();
}

/*
** Allocate a countable mutex
*/
ಸ್ಥಿರ sqlite3_mutex *counterMutexAlloc(ಪೂರ್ಣಾಂಕ eType){
  sqlite3_mutex *pReal;
  sqlite3_mutex *pRet = 0;

  ಪ್ರತಿಜ್ಞಾ( g.isInit );
  ಪ್ರತಿಜ್ಞಾ( eType>=SQLITE_MUTEX_FAST );
  ಪ್ರತಿಜ್ಞಾ( eType<=SQLITE_MUTEX_STATIC_VFS3 );

  pReal = g.m.xMutexAlloc(eType);
  ಆದರೆ( !pReal ) ಹಿಂತಿರುಗಿ 0;

  ಆದರೆ( eType==SQLITE_MUTEX_FAST || eType==SQLITE_MUTEX_RECURSIVE ){
    pRet = (sqlite3_mutex *)ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ(sqlite3_mutex));
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪೂರ್ಣಾಂಕ eStaticType = eType - (MAX_MUTEXES - STATIC_MUTEXES);
    ಪ್ರತಿಜ್ಞಾ( eStaticType>=0 );
    ಪ್ರತಿಜ್ಞಾ( eStaticType<STATIC_MUTEXES );
    pRet = &g.aStatic[eStaticType];
  }

  pRet->eType = eType;
  pRet->pReal = pReal;
  ಹಿಂತಿರುಗಿ pRet;
}

/*
** Free a countable mutex
*/
ಸ್ಥಿರ ಶೂನ್ಯ counterMutexFree(sqlite3_mutex *p){
  ಪ್ರತಿಜ್ಞಾ( g.isInit );
  g.m.xMutexFree(p->pReal);
  ಆದರೆ( p->eType==SQLITE_MUTEX_FAST || p->eType==SQLITE_MUTEX_RECURSIVE ){
    ಬಿಡುಗಡೆ(p);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ counterMutexTry(sqlite3_mutex *p){
  ಪ್ರತಿಜ್ಞಾ( g.isInit );
  ಪ್ರತಿಜ್ಞಾ( p->eType>=0 );
  ಪ್ರತಿಜ್ಞಾ( p->eType<MAX_MUTEXES );
  g.aCounter[p->eType]++;
  ಆದರೆ( g.disableTry ) ಹಿಂತಿರುಗಿ SQLITE_BUSY;
  ಹಿಂತಿರುಗಿ g.m.xMutexTry(p->pReal);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_shutdown(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಂಕ rc;

  ಆದರೆ( objc!=1 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  rc = sqlite3_shutdown();
  Tcl_ಗಣ_ಫಲಿತಾಂಶ(interp, (ಅಕ್ಷರ *)sqlite3ErrName(rc), TCL_VOLATILE);
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** sqlite3_initialize
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_initialize(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಂಕ rc;

  ಆದರೆ( objc!=1 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  rc = sqlite3_initialize();
  Tcl_ಗಣ_ಫಲಿತಾಂಶ(interp, (ಅಕ್ಷರ *)sqlite3ErrName(rc), TCL_VOLATILE);
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** install_mutex_counters BOOLEAN
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_install_mutex_counters(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  ಪೂರ್ಣಾಂಕ isInstall;

  sqlite3_mutex_methods counter_methods = {
    counterMutexInit,
    counter_Mutex_ಕೊನೆ,
    counterMutexAlloc,
    counterMutexFree,
    counterMutexEnter,
    counterMutexTry,
    counterMutexLeave,
    counterMutexHeld,
    counterMutexNotheld
  };

  ಆದರೆ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "BOOLEAN");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[1], &isInstall) ){
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  ಪ್ರತಿಜ್ಞಾ(isInstall==0 || isInstall==1);
  ಪ್ರತಿಜ್ಞಾ(g.isInstalled==0 || g.isInstalled==1);
  ಆದರೆ( isInstall==g.isInstalled ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "mutex counters are ", ಖಾಲಿ);
    Tcl_Append_ಫಲಿತಾಂಶ(interp, isInstall?"already installed":"not installed", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  ಆದರೆ( isInstall ){
    ಪ್ರತಿಜ್ಞಾ( g.m.xMutexAlloc==0 );
    rc = sqlite3_config(SQLITE_CONFIG_GETMUTEX, &g.m);
    ಆದರೆ( rc==SQLITE_OK ){
      sqlite3_config(SQLITE_CONFIG_MUTEX, &counter_methods);
    }
    g.disableTry = 0;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪ್ರತಿಜ್ಞಾ( g.m.xMutexAlloc );
    rc = sqlite3_config(SQLITE_CONFIG_MUTEX, &g.m);
    memset(&g.m, 0, ಗಾತ್ರ(sqlite3_mutex_methods));
  }

  ಆದರೆ( rc==SQLITE_OK ){
    g.isInstalled = isInstall;
  }

  Tcl_ಗಣ_ಫಲಿತಾಂಶ(interp, (ಅಕ್ಷರ *)sqlite3ErrName(rc), TCL_VOLATILE);
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** read_mutex_counters
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_read_mutex_counters(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Obj *pRet;
  ಪೂರ್ಣಾಂಕ ii;

  ಆದರೆ( objc!=1 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  pRet = Tcl_NewObj();
  Tcl_Incr_Ref_ಎಣಿಕೆ(pRet);
  ಗಾಗಿ(ii=0; ii<MAX_MUTEXES; ii++){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj(aName[ii], -1));
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewIntObj(g.aCounter[ii]));
  }
  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, pRet);
  Tcl_Decr_Ref_ಎಣಿಕೆ(pRet);

  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** clear_mutex_counters
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_clear_mutex_counters(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಂಕ ii;

  ಆದರೆ( objc!=1 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  ಗಾಗಿ(ii=0; ii<MAX_MUTEXES; ii++){
    g.aCounter[ii] = 0;
  }
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Create and free a mutex.  Return the mutex pointer.  The pointer
** will be invalid since the mutex has already been freed.  The
** return pointer just checks to see if the mutex really was allocated.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_alloc_mutex(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
#ಆದರೆ SQLITE_THREADSAFE
  sqlite3_mutex *p = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
  ಅಕ್ಷರ zBuf[100];
  sqlite3_mutex_free(p);
  sqlite3_snprintf(ಗಾತ್ರ(zBuf), zBuf, "%p", p);
  Tcl_Append_ಫಲಿತಾಂಶ(interp, zBuf, (ಅಕ್ಷರ*)0);
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ TCL_OK;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_config(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ರಚನೆ ConfigOption {
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName;
    ಪೂರ್ಣಾಂಕ i_ಮೌಲ್ಯ;
  } aOpt[] = {
    {"singlethread", SQLITE_CONFIG_SINGLETHREAD},
    {"multithread",  SQLITE_CONFIG_MULTITHREAD},
    {"serialized",   SQLITE_CONFIG_SERIALIZED},
    {0, 0}
  };
  ಪೂರ್ಣಾಂಕ s = ಗಾತ್ರ(ರಚನೆ ConfigOption);
  ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ rc;

  ಆದರೆ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  ಆದರೆ( Tcl_Get_ಸೂಚ್ಯಂಕ_From_Obj_Struct(interp, objv[1], aOpt, s, "flag", 0, &i) ){
    ಆದರೆ( Tcl_GetIntFromObj(interp, objv[1], &i) ){
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    i = aOpt[i].i_ಮೌಲ್ಯ;
  }

  rc = sqlite3_config(i);
  Tcl_ಗಣ_ಫಲಿತಾಂಶ(interp, (ಅಕ್ಷರ *)sqlite3ErrName(rc), TCL_VOLATILE);
  ಹಿಂತಿರುಗಿ TCL_OK;
}

ಸ್ಥಿರ sqlite3_mutex *get_Static_Mutex_ಸೂಚಕ(
  Tcl_Interp *pInterp,
  Tcl_Obj *pObj
){
  ಪೂರ್ಣಾಂಕ iMutex;
  ಆದರೆ( Tcl_Get_ಸೂಚ್ಯಂಕ_From_Obj(pInterp, pObj, aName, "mutex name", 0, &iMutex) ){
    ಹಿಂತಿರುಗಿ 0;
  }
  ಪ್ರತಿಜ್ಞಾ( iMutex!=SQLITE_MUTEX_FAST && iMutex!=SQLITE_MUTEX_RECURSIVE );
  ಹಿಂತಿರುಗಿ counterMutexAlloc(iMutex);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_enter_static_mutex(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_mutex *pMutex;
  ಆದರೆ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "NAME");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  pMutex = get_Static_Mutex_ಸೂಚಕ(interp, objv[1]);
  ಆದರೆ( !pMutex ){
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  sqlite3_mutex_enter(pMutex);
  ಹಿಂತಿರುಗಿ TCL_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_leave_static_mutex(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_mutex *pMutex;
  ಆದರೆ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "NAME");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  pMutex = get_Static_Mutex_ಸೂಚಕ(interp, objv[1]);
  ಆದರೆ( !pMutex ){
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  sqlite3_mutex_leave(pMutex);
  ಹಿಂತಿರುಗಿ TCL_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_enter_db_mutex(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  ಆದರೆ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "DB");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( get_Db_ಸೂಚಕ(interp, Tcl_GetString(objv[1]), &db) ){
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( !db ){
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  sqlite3_mutex_enter(sqlite3_db_mutex(db));
  ಹಿಂತಿರುಗಿ TCL_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_leave_db_mutex(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  ಆದರೆ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "DB");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( get_Db_ಸೂಚಕ(interp, Tcl_GetString(objv[1]), &db) ){
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( !db ){
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  sqlite3_mutex_leave(sqlite3_db_mutex(db));
  ಹಿಂತಿರುಗಿ TCL_OK;
}

ಪೂರ್ಣಾಂಕ Sqlitetest_mutex_Init(Tcl_Interp *interp){
  ಸ್ಥಿರ ರಚನೆ {
    ಅಕ್ಷರ *zName;
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
  ಪೂರ್ಣಾಂಕ i;
  ಗಾಗಿ(i=0; i<ಗಾತ್ರ(aCmd)/ಗಾತ್ರ(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  Tcl_LinkVar(interp, "disable_mutex_init", 
              (ಅಕ್ಷರ*)&g.disableInit, TCL_LINK_INT);
  Tcl_LinkVar(interp, "disable_mutex_try", 
              (ಅಕ್ಷರ*)&g.disableTry, TCL_LINK_INT);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}
