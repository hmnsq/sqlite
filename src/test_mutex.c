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
#ਅਨ੍ਤਰ੍ਭਾਵ "tclsqlite.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "sqlite3.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAX_MUTEXES        (SQLITE_MUTEX_STATIC_VFS3+1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STATIC_MUTEXES     (MAX_MUTEXES-(SQLITE_MUTEX_RECURSIVE+1))

/* defined in main.c */
ਬਾਹਰੀ ਅਚਲ ਅੱਖਰ *sqlite3ErrName(ਪੂਰਨ);
ਪੂਰਨ get_Db_ਸੂਚਕ(Tcl_Interp *interp, ਅਚਲ ਅੱਖਰ *zA, sqlite3 **ppDb);

ਸਥਿਰ ਅਚਲ ਅੱਖਰ *aName[MAX_MUTEXES+1] = {
  "fast",        "recursive",   "static_main",   "static_mem",
  "static_open", "static_prng", "static_lru",    "static_pmem",
  "static_app1", "static_app2", "static_app3",   "static_vfs1",
  "static_vfs2", "static_vfs3", 0
};

/* A countable mutex */
ਬਣਤਰ sqlite3_mutex {
  sqlite3_mutex *pReal;
  ਪੂਰਨ eType;
};

/* State variables */
ਸਥਿਰ ਬਣਤਰ test_mutex_globals {
  ਪੂਰਨ isInstalled;           /* True if installed */
  ਪੂਰਨ disableInit;           /* True to cause sqlite3_initialize() to fail */
  ਪੂਰਨ disableTry;            /* True to force sqlite3_mutex_try() to fail */
  ਪੂਰਨ isInit;                /* True if initialized */
  sqlite3_mutex_methods m;   /* Interface to "real" mutex system */
  ਪੂਰਨ aCounter[MAX_MUTEXES]; /* Number of grabs of each type of mutex */
  sqlite3_mutex aStatic[STATIC_MUTEXES]; /* The static mutexes */
} g = {0};

/* Return true if the countable mutex is currently held */
ਸਥਿਰ ਪੂਰਨ counterMutexHeld(sqlite3_mutex *p){
  ਵਾਪਸ g.m.xMutexHeld(p->pReal);
}

/* Return true if the countable mutex is not currently held */
ਸਥਿਰ ਪੂਰਨ counterMutexNotheld(sqlite3_mutex *p){
  ਵਾਪਸ g.m.xMutexNotheld(p->pReal);
}

/* Initialize the countable mutex interface
** Or, if g.disableInit is non-zero, then do not initialize but instead
** return the value of g.disableInit as the result code.  This can be used
** to simulate an initialization failure.
*/
ਸਥਿਰ ਪੂਰਨ counterMutexInit(ਖਾਲੀ){ 
  ਪੂਰਨ rc;
  ਜੇ( g.disableInit ) ਵਾਪਸ g.disableInit;
  rc = g.m.xMutexInit();
  g.isInit = 1;
  ਵਾਪਸ rc;
}

/*
** Uninitialize the mutex subsystem
*/
ਸਥਿਰ ਪੂਰਨ counter_Mutex_ਸਿਰਾ(ਖਾਲੀ){ 
  g.isInit = 0;
  ਵਾਪਸ g.m.x_Mutex_ਸਿਰਾ();
}

/*
** Allocate a countable mutex
*/
ਸਥਿਰ sqlite3_mutex *counterMutexAlloc(ਪੂਰਨ eType){
  sqlite3_mutex *pReal;
  sqlite3_mutex *pRet = 0;

  ਪ੍ਰਤਿਜ੍ਞਾ( g.isInit );
  ਪ੍ਰਤਿਜ੍ਞਾ( eType>=SQLITE_MUTEX_FAST );
  ਪ੍ਰਤਿਜ੍ਞਾ( eType<=SQLITE_MUTEX_STATIC_VFS3 );

  pReal = g.m.xMutexAlloc(eType);
  ਜੇ( !pReal ) ਵਾਪਸ 0;

  ਜੇ( eType==SQLITE_MUTEX_FAST || eType==SQLITE_MUTEX_RECURSIVE ){
    pRet = (sqlite3_mutex *)ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਆਕਾਰ(sqlite3_mutex));
  }ਨਹੀਂਤਾਂ{
    ਪੂਰਨ eStaticType = eType - (MAX_MUTEXES - STATIC_MUTEXES);
    ਪ੍ਰਤਿਜ੍ਞਾ( eStaticType>=0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( eStaticType<STATIC_MUTEXES );
    pRet = &g.aStatic[eStaticType];
  }

  pRet->eType = eType;
  pRet->pReal = pReal;
  ਵਾਪਸ pRet;
}

/*
** Free a countable mutex
*/
ਸਥਿਰ ਖਾਲੀ counterMutexFree(sqlite3_mutex *p){
  ਪ੍ਰਤਿਜ੍ਞਾ( g.isInit );
  g.m.xMutexFree(p->pReal);
  ਜੇ( p->eType==SQLITE_MUTEX_FAST || p->eType==SQLITE_MUTEX_RECURSIVE ){
    ਮੁਕਤ(p);
  }
}

/*
** Enter a countable mutex.  Block until entry is safe.
*/
ਸਥਿਰ ਖਾਲੀ counterMutexEnter(sqlite3_mutex *p){
  ਪ੍ਰਤਿਜ੍ਞਾ( g.isInit );
  ਪ੍ਰਤਿਜ੍ਞਾ( p->eType>=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p->eType<MAX_MUTEXES );
  g.aCounter[p->eType]++;
  g.m.xMutexEnter(p->pReal);
}

/*
** Try to enter a mutex.  Return true on success.
*/
ਸਥਿਰ ਪੂਰਨ counterMutexTry(sqlite3_mutex *p){
  ਪ੍ਰਤਿਜ੍ਞਾ( g.isInit );
  ਪ੍ਰਤਿਜ੍ਞਾ( p->eType>=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p->eType<MAX_MUTEXES );
  g.aCounter[p->eType]++;
  ਜੇ( g.disableTry ) ਵਾਪਸ SQLITE_BUSY;
  ਵਾਪਸ g.m.xMutexTry(p->pReal);
}

/* Leave a mutex
*/
ਸਥਿਰ ਖਾਲੀ counterMutexLeave(sqlite3_mutex *p){
  ਪ੍ਰਤਿਜ੍ਞਾ( g.isInit );
  g.m.xMutexLeave(p->pReal);
}

/*
** sqlite3_shutdown
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_shutdown(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਪੂਰਨ rc;

  ਜੇ( objc!=1 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "");
    ਵਾਪਸ TCL_ERROR;
  }

  rc = sqlite3_shutdown();
  Tcl_ਸੈੱਟ_ਨਤੀਜਾ(interp, (ਅੱਖਰ *)sqlite3ErrName(rc), TCL_VOLATILE);
  ਵਾਪਸ TCL_OK;
}

/*
** sqlite3_initialize
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_initialize(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਪੂਰਨ rc;

  ਜੇ( objc!=1 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "");
    ਵਾਪਸ TCL_ERROR;
  }

  rc = sqlite3_initialize();
  Tcl_ਸੈੱਟ_ਨਤੀਜਾ(interp, (ਅੱਖਰ *)sqlite3ErrName(rc), TCL_VOLATILE);
  ਵਾਪਸ TCL_OK;
}

/*
** install_mutex_counters BOOLEAN
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_install_mutex_counters(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਪੂਰਨ rc = SQLITE_OK;
  ਪੂਰਨ isInstall;

  sqlite3_mutex_methods counter_methods = {
    counterMutexInit,
    counter_Mutex_ਸਿਰਾ,
    counterMutexAlloc,
    counterMutexFree,
    counterMutexEnter,
    counterMutexTry,
    counterMutexLeave,
    counterMutexHeld,
    counterMutexNotheld
  };

  ਜੇ( objc!=2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "BOOLEAN");
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[1], &isInstall) ){
    ਵਾਪਸ TCL_ERROR;
  }

  ਪ੍ਰਤਿਜ੍ਞਾ(isInstall==0 || isInstall==1);
  ਪ੍ਰਤਿਜ੍ਞਾ(g.isInstalled==0 || g.isInstalled==1);
  ਜੇ( isInstall==g.isInstalled ){
    Tcl_Append_ਨਤੀਜਾ(interp, "mutex counters are ", ਨਿਰਾ);
    Tcl_Append_ਨਤੀਜਾ(interp, isInstall?"already installed":"not installed", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }

  ਜੇ( isInstall ){
    ਪ੍ਰਤਿਜ੍ਞਾ( g.m.xMutexAlloc==0 );
    rc = sqlite3_config(SQLITE_CONFIG_GETMUTEX, &g.m);
    ਜੇ( rc==SQLITE_OK ){
      sqlite3_config(SQLITE_CONFIG_MUTEX, &counter_methods);
    }
    g.disableTry = 0;
  }ਨਹੀਂਤਾਂ{
    ਪ੍ਰਤਿਜ੍ਞਾ( g.m.xMutexAlloc );
    rc = sqlite3_config(SQLITE_CONFIG_MUTEX, &g.m);
    memset(&g.m, 0, ਆਕਾਰ(sqlite3_mutex_methods));
  }

  ਜੇ( rc==SQLITE_OK ){
    g.isInstalled = isInstall;
  }

  Tcl_ਸੈੱਟ_ਨਤੀਜਾ(interp, (ਅੱਖਰ *)sqlite3ErrName(rc), TCL_VOLATILE);
  ਵਾਪਸ TCL_OK;
}

/*
** read_mutex_counters
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_read_mutex_counters(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Obj *pRet;
  ਪੂਰਨ ii;

  ਜੇ( objc!=1 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "");
    ਵਾਪਸ TCL_ERROR;
  }

  pRet = Tcl_NewObj();
  Tcl_Incr_Ref_ਗਿਣਤੀ(pRet);
  ਲਈ(ii=0; ii<MAX_MUTEXES; ii++){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj(aName[ii], -1));
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewIntObj(g.aCounter[ii]));
  }
  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, pRet);
  Tcl_Decr_Ref_ਗਿਣਤੀ(pRet);

  ਵਾਪਸ TCL_OK;
}

/*
** clear_mutex_counters
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_clear_mutex_counters(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਪੂਰਨ ii;

  ਜੇ( objc!=1 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "");
    ਵਾਪਸ TCL_ERROR;
  }

  ਲਈ(ii=0; ii<MAX_MUTEXES; ii++){
    g.aCounter[ii] = 0;
  }
  ਵਾਪਸ TCL_OK;
}

/*
** Create and free a mutex.  Return the mutex pointer.  The pointer
** will be invalid since the mutex has already been freed.  The
** return pointer just checks to see if the mutex really was allocated.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_alloc_mutex(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
#ਜੇ SQLITE_THREADSAFE
  sqlite3_mutex *p = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
  ਅੱਖਰ zBuf[100];
  sqlite3_mutex_free(p);
  sqlite3_snprintf(ਆਕਾਰ(zBuf), zBuf, "%p", p);
  Tcl_Append_ਨਤੀਜਾ(interp, zBuf, (ਅੱਖਰ*)0);
#ਜੇ_ਅੰਤ
  ਵਾਪਸ TCL_OK;
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
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_config(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਬਣਤਰ ConfigOption {
    ਅਚਲ ਅੱਖਰ *zName;
    ਪੂਰਨ i_ਮੁੱਲ;
  } aOpt[] = {
    {"singlethread", SQLITE_CONFIG_SINGLETHREAD},
    {"multithread",  SQLITE_CONFIG_MULTITHREAD},
    {"serialized",   SQLITE_CONFIG_SERIALIZED},
    {0, 0}
  };
  ਪੂਰਨ s = ਆਕਾਰ(ਬਣਤਰ ConfigOption);
  ਪੂਰਨ i;
  ਪੂਰਨ rc;

  ਜੇ( objc!=2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "");
    ਵਾਪਸ TCL_ERROR;
  }

  ਜੇ( Tcl_Get_ਸੂਚਕਾਂਕ_From_Obj_Struct(interp, objv[1], aOpt, s, "flag", 0, &i) ){
    ਜੇ( Tcl_GetIntFromObj(interp, objv[1], &i) ){
      ਵਾਪਸ TCL_ERROR;
    }
  }ਨਹੀਂਤਾਂ{
    i = aOpt[i].i_ਮੁੱਲ;
  }

  rc = sqlite3_config(i);
  Tcl_ਸੈੱਟ_ਨਤੀਜਾ(interp, (ਅੱਖਰ *)sqlite3ErrName(rc), TCL_VOLATILE);
  ਵਾਪਸ TCL_OK;
}

ਸਥਿਰ sqlite3_mutex *get_Static_Mutex_ਸੂਚਕ(
  Tcl_Interp *pInterp,
  Tcl_Obj *pObj
){
  ਪੂਰਨ iMutex;
  ਜੇ( Tcl_Get_ਸੂਚਕਾਂਕ_From_Obj(pInterp, pObj, aName, "mutex name", 0, &iMutex) ){
    ਵਾਪਸ 0;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( iMutex!=SQLITE_MUTEX_FAST && iMutex!=SQLITE_MUTEX_RECURSIVE );
  ਵਾਪਸ counterMutexAlloc(iMutex);
}

ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_enter_static_mutex(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_mutex *pMutex;
  ਜੇ( objc!=2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "NAME");
    ਵਾਪਸ TCL_ERROR;
  }
  pMutex = get_Static_Mutex_ਸੂਚਕ(interp, objv[1]);
  ਜੇ( !pMutex ){
    ਵਾਪਸ TCL_ERROR;
  }
  sqlite3_mutex_enter(pMutex);
  ਵਾਪਸ TCL_OK;
}

ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_leave_static_mutex(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_mutex *pMutex;
  ਜੇ( objc!=2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "NAME");
    ਵਾਪਸ TCL_ERROR;
  }
  pMutex = get_Static_Mutex_ਸੂਚਕ(interp, objv[1]);
  ਜੇ( !pMutex ){
    ਵਾਪਸ TCL_ERROR;
  }
  sqlite3_mutex_leave(pMutex);
  ਵਾਪਸ TCL_OK;
}

ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_enter_db_mutex(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  ਜੇ( objc!=2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "DB");
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( get_Db_ਸੂਚਕ(interp, Tcl_GetString(objv[1]), &db) ){
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( !db ){
    ਵਾਪਸ TCL_ERROR;
  }
  sqlite3_mutex_enter(sqlite3_db_mutex(db));
  ਵਾਪਸ TCL_OK;
}

ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_leave_db_mutex(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  ਜੇ( objc!=2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "DB");
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( get_Db_ਸੂਚਕ(interp, Tcl_GetString(objv[1]), &db) ){
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( !db ){
    ਵਾਪਸ TCL_ERROR;
  }
  sqlite3_mutex_leave(sqlite3_db_mutex(db));
  ਵਾਪਸ TCL_OK;
}

ਪੂਰਨ Sqlitetest_mutex_Init(Tcl_Interp *interp){
  ਸਥਿਰ ਬਣਤਰ {
    ਅੱਖਰ *zName;
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
  ਪੂਰਨ i;
  ਲਈ(i=0; i<ਆਕਾਰ(aCmd)/ਆਕਾਰ(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  Tcl_LinkVar(interp, "disable_mutex_init", 
              (ਅੱਖਰ*)&g.disableInit, TCL_LINK_INT);
  Tcl_LinkVar(interp, "disable_mutex_try", 
              (ਅੱਖਰ*)&g.disableTry, TCL_LINK_INT);
  ਵਾਪਸ SQLITE_OK;
}
