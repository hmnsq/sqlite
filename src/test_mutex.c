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
#അന്തര്ഭാവ "tclsqlite.h"
#അന്തര്ഭാവ "sqlite3.h"
#അന്തര്ഭാവ "sqliteInt.h"
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <assert.h>
#അന്തര്ഭാവ <string.h>

#define MAX_MUTEXES        (SQLITE_MUTEX_STATIC_VFS3+1)
#define STATIC_MUTEXES     (MAX_MUTEXES-(SQLITE_MUTEX_RECURSIVE+1))

/* defined in main.c */
ബാഹ്യ സ്ഥിരം അക്ഷരം *sqlite3ErrName(പൂർണ്ണസംഖ്യ);
പൂർണ്ണസംഖ്യ getDbPointer(Tcl_Interp *interp, സ്ഥിരം അക്ഷരം *zA, sqlite3 **ppDb);

സ്ഥിതം സ്ഥിരം അക്ഷരം *aName[MAX_MUTEXES+1] = {
  "fast",        "recursive",   "static_main",   "static_mem",
  "static_open", "static_prng", "static_lru",    "static_pmem",
  "static_app1", "static_app2", "static_app3",   "static_vfs1",
  "static_vfs2", "static_vfs3", 0
};

/* A countable mutex */
ഘടന sqlite3_mutex {
  sqlite3_mutex *pReal;
  പൂർണ്ണസംഖ്യ eType;
};

/* State variables */
സ്ഥിതം ഘടന test_mutex_globals {
  പൂർണ്ണസംഖ്യ isInstalled;           /* True if installed */
  പൂർണ്ണസംഖ്യ disableInit;           /* True to cause sqlite3_initialize() to fail */
  പൂർണ്ണസംഖ്യ disableTry;            /* True to force sqlite3_mutex_try() to fail */
  പൂർണ്ണസംഖ്യ isInit;                /* True if initialized */
  sqlite3_mutex_methods m;   /* Interface to "real" mutex system */
  പൂർണ്ണസംഖ്യ aCounter[MAX_MUTEXES]; /* Number of grabs of each type of mutex */
  sqlite3_mutex aStatic[STATIC_MUTEXES]; /* The static mutexes */
} g = {0};

/* Return true if the countable mutex is currently held */
സ്ഥിതം പൂർണ്ണസംഖ്യ counterMutexHeld(sqlite3_mutex *p){
  തിരികെ g.m.xMutexHeld(p->pReal);
}

/* Return true if the countable mutex is not currently held */
സ്ഥിതം പൂർണ്ണസംഖ്യ counterMutexNotheld(sqlite3_mutex *p){
  തിരികെ g.m.xMutexNotheld(p->pReal);
}

/* Initialize the countable mutex interface
** Or, if g.disableInit is non-zero, then do not initialize but instead
** return the value of g.disableInit as the result code.  This can be used
** to simulate an initialization failure.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ counterMutexInit(ശൂന്യം){ 
  പൂർണ്ണസംഖ്യ rc;
  എങ്കിൽ( g.disableInit ) തിരികെ g.disableInit;
  rc = g.m.xMutexInit();
  g.isInit = 1;
  തിരികെ rc;
}

/*
** Uninitialize the mutex subsystem
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ counterMutexEnd(ശൂന്യം){ 
  g.isInit = 0;
  തിരികെ g.m.xMutexEnd();
}

/*
** Allocate a countable mutex
*/
സ്ഥിതം sqlite3_mutex *counterMutexAlloc(പൂർണ്ണസംഖ്യ eType){
  sqlite3_mutex *pReal;
  sqlite3_mutex *pRet = 0;

  പ്രതിജ്ഞാ( g.isInit );
  പ്രതിജ്ഞാ( eType>=SQLITE_MUTEX_FAST );
  പ്രതിജ്ഞാ( eType<=SQLITE_MUTEX_STATIC_VFS3 );

  pReal = g.m.xMutexAlloc(eType);
  എങ്കിൽ( !pReal ) തിരികെ 0;

  എങ്കിൽ( eType==SQLITE_MUTEX_FAST || eType==SQLITE_MUTEX_RECURSIVE ){
    pRet = (sqlite3_mutex *)malloc(വലുപ്പം(sqlite3_mutex));
  }അല്ലെങ്കിൽ{
    പൂർണ്ണസംഖ്യ eStaticType = eType - (MAX_MUTEXES - STATIC_MUTEXES);
    പ്രതിജ്ഞാ( eStaticType>=0 );
    പ്രതിജ്ഞാ( eStaticType<STATIC_MUTEXES );
    pRet = &g.aStatic[eStaticType];
  }

  pRet->eType = eType;
  pRet->pReal = pReal;
  തിരികെ pRet;
}

/*
** Free a countable mutex
*/
സ്ഥിതം ശൂന്യം counterMutexFree(sqlite3_mutex *p){
  പ്രതിജ്ഞാ( g.isInit );
  g.m.xMutexFree(p->pReal);
  എങ്കിൽ( p->eType==SQLITE_MUTEX_FAST || p->eType==SQLITE_MUTEX_RECURSIVE ){
    സ്വതന്ത്രം(p);
  }
}

/*
** Enter a countable mutex.  Block until entry is safe.
*/
സ്ഥിതം ശൂന്യം counterMutexEnter(sqlite3_mutex *p){
  പ്രതിജ്ഞാ( g.isInit );
  പ്രതിജ്ഞാ( p->eType>=0 );
  പ്രതിജ്ഞാ( p->eType<MAX_MUTEXES );
  g.aCounter[p->eType]++;
  g.m.xMutexEnter(p->pReal);
}

/*
** Try to enter a mutex.  Return true on success.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ counterMutexTry(sqlite3_mutex *p){
  പ്രതിജ്ഞാ( g.isInit );
  പ്രതിജ്ഞാ( p->eType>=0 );
  പ്രതിജ്ഞാ( p->eType<MAX_MUTEXES );
  g.aCounter[p->eType]++;
  എങ്കിൽ( g.disableTry ) തിരികെ SQLITE_BUSY;
  തിരികെ g.m.xMutexTry(p->pReal);
}

/* Leave a mutex
*/
സ്ഥിതം ശൂന്യം counterMutexLeave(sqlite3_mutex *p){
  പ്രതിജ്ഞാ( g.isInit );
  g.m.xMutexLeave(p->pReal);
}

/*
** sqlite3_shutdown
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_shutdown(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  പൂർണ്ണസംഖ്യ rc;

  എങ്കിൽ( objc!=1 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "");
    തിരികെ TCL_ERROR;
  }

  rc = sqlite3_shutdown();
  Tcl_SetResult(interp, (അക്ഷരം *)sqlite3ErrName(rc), TCL_VOLATILE);
  തിരികെ TCL_OK;
}

/*
** sqlite3_initialize
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_initialize(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  പൂർണ്ണസംഖ്യ rc;

  എങ്കിൽ( objc!=1 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "");
    തിരികെ TCL_ERROR;
  }

  rc = sqlite3_initialize();
  Tcl_SetResult(interp, (അക്ഷരം *)sqlite3ErrName(rc), TCL_VOLATILE);
  തിരികെ TCL_OK;
}

/*
** install_mutex_counters BOOLEAN
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_install_mutex_counters(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;
  പൂർണ്ണസംഖ്യ isInstall;

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

  എങ്കിൽ( objc!=2 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "BOOLEAN");
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[1], &isInstall) ){
    തിരികെ TCL_ERROR;
  }

  പ്രതിജ്ഞാ(isInstall==0 || isInstall==1);
  പ്രതിജ്ഞാ(g.isInstalled==0 || g.isInstalled==1);
  എങ്കിൽ( isInstall==g.isInstalled ){
    Tcl_AppendResult(interp, "mutex counters are ", ശൂന്യ);
    Tcl_AppendResult(interp, isInstall?"already installed":"not installed", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }

  എങ്കിൽ( isInstall ){
    പ്രതിജ്ഞാ( g.m.xMutexAlloc==0 );
    rc = sqlite3_config(SQLITE_CONFIG_GETMUTEX, &g.m);
    എങ്കിൽ( rc==SQLITE_OK ){
      sqlite3_config(SQLITE_CONFIG_MUTEX, &counter_methods);
    }
    g.disableTry = 0;
  }അല്ലെങ്കിൽ{
    പ്രതിജ്ഞാ( g.m.xMutexAlloc );
    rc = sqlite3_config(SQLITE_CONFIG_MUTEX, &g.m);
    memset(&g.m, 0, വലുപ്പം(sqlite3_mutex_methods));
  }

  എങ്കിൽ( rc==SQLITE_OK ){
    g.isInstalled = isInstall;
  }

  Tcl_SetResult(interp, (അക്ഷരം *)sqlite3ErrName(rc), TCL_VOLATILE);
  തിരികെ TCL_OK;
}

/*
** read_mutex_counters
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_read_mutex_counters(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Obj *pRet;
  പൂർണ്ണസംഖ്യ ii;

  എങ്കിൽ( objc!=1 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "");
    തിരികെ TCL_ERROR;
  }

  pRet = Tcl_NewObj();
  Tcl_Incr_Ref_എണ്ണം(pRet);
  വേണ്ടി(ii=0; ii<MAX_MUTEXES; ii++){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj(aName[ii], -1));
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewIntObj(g.aCounter[ii]));
  }
  Tcl_SetObjResult(interp, pRet);
  Tcl_Decr_Ref_എണ്ണം(pRet);

  തിരികെ TCL_OK;
}

/*
** clear_mutex_counters
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_clear_mutex_counters(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  പൂർണ്ണസംഖ്യ ii;

  എങ്കിൽ( objc!=1 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "");
    തിരികെ TCL_ERROR;
  }

  വേണ്ടി(ii=0; ii<MAX_MUTEXES; ii++){
    g.aCounter[ii] = 0;
  }
  തിരികെ TCL_OK;
}

/*
** Create and free a mutex.  Return the mutex pointer.  The pointer
** will be invalid since the mutex has already been freed.  The
** return pointer just checks to see if the mutex really was allocated.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_alloc_mutex(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
#എങ്കിൽ SQLITE_THREADSAFE
  sqlite3_mutex *p = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
  അക്ഷരം zBuf[100];
  sqlite3_mutex_free(p);
  sqlite3_snprintf(വലുപ്പം(zBuf), zBuf, "%p", p);
  Tcl_AppendResult(interp, zBuf, (അക്ഷരം*)0);
#endif
  തിരികെ TCL_OK;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_config(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  ഘടന ConfigOption {
    സ്ഥിരം അക്ഷരം *zName;
    പൂർണ്ണസംഖ്യ i_മൂല്യം;
  } aOpt[] = {
    {"singlethread", SQLITE_CONFIG_SINGLETHREAD},
    {"multithread",  SQLITE_CONFIG_MULTITHREAD},
    {"serialized",   SQLITE_CONFIG_SERIALIZED},
    {0, 0}
  };
  പൂർണ്ണസംഖ്യ s = വലുപ്പം(ഘടന ConfigOption);
  പൂർണ്ണസംഖ്യ i;
  പൂർണ്ണസംഖ്യ rc;

  എങ്കിൽ( objc!=2 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "");
    തിരികെ TCL_ERROR;
  }

  എങ്കിൽ( Tcl_GetIndexFromObjStruct(interp, objv[1], aOpt, s, "flag", 0, &i) ){
    എങ്കിൽ( Tcl_GetIntFromObj(interp, objv[1], &i) ){
      തിരികെ TCL_ERROR;
    }
  }അല്ലെങ്കിൽ{
    i = aOpt[i].i_മൂല്യം;
  }

  rc = sqlite3_config(i);
  Tcl_SetResult(interp, (അക്ഷരം *)sqlite3ErrName(rc), TCL_VOLATILE);
  തിരികെ TCL_OK;
}

സ്ഥിതം sqlite3_mutex *getStaticMutexPointer(
  Tcl_Interp *pInterp,
  Tcl_Obj *pObj
){
  പൂർണ്ണസംഖ്യ iMutex;
  എങ്കിൽ( Tcl_GetIndexFromObj(pInterp, pObj, aName, "mutex name", 0, &iMutex) ){
    തിരികെ 0;
  }
  പ്രതിജ്ഞാ( iMutex!=SQLITE_MUTEX_FAST && iMutex!=SQLITE_MUTEX_RECURSIVE );
  തിരികെ counterMutexAlloc(iMutex);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_enter_static_mutex(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_mutex *pMutex;
  എങ്കിൽ( objc!=2 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "NAME");
    തിരികെ TCL_ERROR;
  }
  pMutex = getStaticMutexPointer(interp, objv[1]);
  എങ്കിൽ( !pMutex ){
    തിരികെ TCL_ERROR;
  }
  sqlite3_mutex_enter(pMutex);
  തിരികെ TCL_OK;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_leave_static_mutex(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_mutex *pMutex;
  എങ്കിൽ( objc!=2 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "NAME");
    തിരികെ TCL_ERROR;
  }
  pMutex = getStaticMutexPointer(interp, objv[1]);
  എങ്കിൽ( !pMutex ){
    തിരികെ TCL_ERROR;
  }
  sqlite3_mutex_leave(pMutex);
  തിരികെ TCL_OK;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_enter_db_mutex(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  എങ്കിൽ( objc!=2 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "DB");
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ){
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( !db ){
    തിരികെ TCL_ERROR;
  }
  sqlite3_mutex_enter(sqlite3_db_mutex(db));
  തിരികെ TCL_OK;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_leave_db_mutex(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  എങ്കിൽ( objc!=2 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "DB");
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ){
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( !db ){
    തിരികെ TCL_ERROR;
  }
  sqlite3_mutex_leave(sqlite3_db_mutex(db));
  തിരികെ TCL_OK;
}

പൂർണ്ണസംഖ്യ Sqlitetest_mutex_Init(Tcl_Interp *interp){
  സ്ഥിതം ഘടന {
    അക്ഷരം *zName;
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
  പൂർണ്ണസംഖ്യ i;
  വേണ്ടി(i=0; i<വലുപ്പം(aCmd)/വലുപ്പം(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  Tcl_LinkVar(interp, "disable_mutex_init", 
              (അക്ഷരം*)&g.disableInit, TCL_LINK_INT);
  Tcl_LinkVar(interp, "disable_mutex_try", 
              (അക്ഷരം*)&g.disableTry, TCL_LINK_INT);
  തിരികെ SQLITE_OK;
}
