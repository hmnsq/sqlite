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
#include "tclsqlite.h"
#include "sqlite3.h"
#include "sqliteInt.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define MAX_MUTEXES        (SQLITE_MUTEX_STATIC_VFS3+1)
#define STATIC_MUTEXES     (MAX_MUTEXES-(SQLITE_MUTEX_RECURSIVE+1))

/* defined in main.c */
extern const var_na *sqlite3ErrName(puur_naa_nka);
puur_naa_nka getDbPointer(Tcl_Interp *interp, const var_na *zA, sqlite3 **ppDb);

static const var_na *aName[MAX_MUTEXES+1] = {
  "fast",        "recursive",   "static_main",   "static_mem",
  "static_open", "static_prng", "static_lru",    "static_pmem",
  "static_app1", "static_app2", "static_app3",   "static_vfs1",
  "static_vfs2", "static_vfs3", 0
};

/* A countable mutex */
sa_nracanaa sqlite3_mutex {
  sqlite3_mutex *pReal;
  puur_naa_nka eType;
};

/* State variables */
static sa_nracanaa test_mutex_globals {
  puur_naa_nka isInstalled;           /* True if installed */
  puur_naa_nka disableInit;           /* True to cause sqlite3_initialize() to fail */
  puur_naa_nka disableTry;            /* True to force sqlite3_mutex_try() to fail */
  puur_naa_nka isInit;                /* True if initialized */
  sqlite3_mutex_methods m;   /* Interface to "real" mutex system */
  puur_naa_nka aCounter[MAX_MUTEXES]; /* Number of grabs of each type of mutex */
  sqlite3_mutex aStatic[STATIC_MUTEXES]; /* The static mutexes */
} g = {0};

/* Return true if the countable mutex is currently held */
static puur_naa_nka counterMutexHeld(sqlite3_mutex *p){
  ᰀᰧᰌ g.m.xMutexHeld(p->pReal);
}

/* Return true if the countable mutex is not currently held */
static puur_naa_nka counterMutexNotheld(sqlite3_mutex *p){
  ᰀᰧᰌ g.m.xMutexNotheld(p->pReal);
}

/* Initialize the countable mutex interface
** Or, if g.disableInit is non-zero, then do not initialize but instead
** return the value of g.disableInit as the result code.  This can be used
** to simulate an initialization failure.
*/
static puur_naa_nka counterMutexInit(void){ 
  puur_naa_nka rc;
  ᰀᰧ( g.disableInit ) ᰀᰧᰌ g.disableInit;
  rc = g.m.xMutexInit();
  g.isInit = 1;
  ᰀᰧᰌ rc;
}

/*
** Uninitialize the mutex subsystem
*/
static puur_naa_nka counterMutexEnd(void){ 
  g.isInit = 0;
  ᰀᰧᰌ g.m.xMutexEnd();
}

/*
** Allocate a countable mutex
*/
static sqlite3_mutex *counterMutexAlloc(puur_naa_nka eType){
  sqlite3_mutex *pReal;
  sqlite3_mutex *pRet = 0;

  assert( g.isInit );
  assert( eType>=SQLITE_MUTEX_FAST );
  assert( eType<=SQLITE_MUTEX_STATIC_VFS3 );

  pReal = g.m.xMutexAlloc(eType);
  ᰀᰧ( !pReal ) ᰀᰧᰌ 0;

  ᰀᰧ( eType==SQLITE_MUTEX_FAST || eType==SQLITE_MUTEX_RECURSIVE ){
    pRet = (sqlite3_mutex *)malloc(parimaa_na(sqlite3_mutex));
  }ᰀᰧᰀ{
    puur_naa_nka eStaticType = eType - (MAX_MUTEXES - STATIC_MUTEXES);
    assert( eStaticType>=0 );
    assert( eStaticType<STATIC_MUTEXES );
    pRet = &g.aStatic[eStaticType];
  }

  pRet->eType = eType;
  pRet->pReal = pReal;
  ᰀᰧᰌ pRet;
}

/*
** Free a countable mutex
*/
static void counterMutexFree(sqlite3_mutex *p){
  assert( g.isInit );
  g.m.xMutexFree(p->pReal);
  ᰀᰧ( p->eType==SQLITE_MUTEX_FAST || p->eType==SQLITE_MUTEX_RECURSIVE ){
    free(p);
  }
}

/*
** Enter a countable mutex.  Block until entry is safe.
*/
static void counterMutexEnter(sqlite3_mutex *p){
  assert( g.isInit );
  assert( p->eType>=0 );
  assert( p->eType<MAX_MUTEXES );
  g.aCounter[p->eType]++;
  g.m.xMutexEnter(p->pReal);
}

/*
** Try to enter a mutex.  Return true on success.
*/
static puur_naa_nka counterMutexTry(sqlite3_mutex *p){
  assert( g.isInit );
  assert( p->eType>=0 );
  assert( p->eType<MAX_MUTEXES );
  g.aCounter[p->eType]++;
  ᰀᰧ( g.disableTry ) ᰀᰧᰌ SQLITE_BUSY;
  ᰀᰧᰌ g.m.xMutexTry(p->pReal);
}

/* Leave a mutex
*/
static void counterMutexLeave(sqlite3_mutex *p){
  assert( g.isInit );
  g.m.xMutexLeave(p->pReal);
}

/*
** sqlite3_shutdown
*/
static puur_naa_nka SQLITE_TCLAPI test_shutdown(
  void * clientData,
  Tcl_Interp *interp,
  puur_naa_nka objc,
  Tcl_Obj *CONST objv[]
){
  puur_naa_nka rc;

  ᰀᰧ( objc!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    ᰀᰧᰌ TCL_ERROR;
  }

  rc = sqlite3_shutdown();
  Tcl_SetResult(interp, (var_na *)sqlite3ErrName(rc), TCL_VOLATILE);
  ᰀᰧᰌ TCL_OK;
}

/*
** sqlite3_initialize
*/
static puur_naa_nka SQLITE_TCLAPI test_initialize(
  void * clientData,
  Tcl_Interp *interp,
  puur_naa_nka objc,
  Tcl_Obj *CONST objv[]
){
  puur_naa_nka rc;

  ᰀᰧ( objc!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    ᰀᰧᰌ TCL_ERROR;
  }

  rc = sqlite3_initialize();
  Tcl_SetResult(interp, (var_na *)sqlite3ErrName(rc), TCL_VOLATILE);
  ᰀᰧᰌ TCL_OK;
}

/*
** install_mutex_counters BOOLEAN
*/
static puur_naa_nka SQLITE_TCLAPI test_install_mutex_counters(
  void * clientData,
  Tcl_Interp *interp,
  puur_naa_nka objc,
  Tcl_Obj *CONST objv[]
){
  puur_naa_nka rc = SQLITE_OK;
  puur_naa_nka isInstall;

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

  ᰀᰧ( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "BOOLEAN");
    ᰀᰧᰌ TCL_ERROR;
  }
  ᰀᰧ( TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[1], &isInstall) ){
    ᰀᰧᰌ TCL_ERROR;
  }

  assert(isInstall==0 || isInstall==1);
  assert(g.isInstalled==0 || g.isInstalled==1);
  ᰀᰧ( isInstall==g.isInstalled ){
    Tcl_AppendResult(interp, "mutex counters are ", NULL);
    Tcl_AppendResult(interp, isInstall?"already installed":"not installed", NULL);
    ᰀᰧᰌ TCL_ERROR;
  }

  ᰀᰧ( isInstall ){
    assert( g.m.xMutexAlloc==0 );
    rc = sqlite3_config(SQLITE_CONFIG_GETMUTEX, &g.m);
    ᰀᰧ( rc==SQLITE_OK ){
      sqlite3_config(SQLITE_CONFIG_MUTEX, &counter_methods);
    }
    g.disableTry = 0;
  }ᰀᰧᰀ{
    assert( g.m.xMutexAlloc );
    rc = sqlite3_config(SQLITE_CONFIG_MUTEX, &g.m);
    memset(&g.m, 0, parimaa_na(sqlite3_mutex_methods));
  }

  ᰀᰧ( rc==SQLITE_OK ){
    g.isInstalled = isInstall;
  }

  Tcl_SetResult(interp, (var_na *)sqlite3ErrName(rc), TCL_VOLATILE);
  ᰀᰧᰌ TCL_OK;
}

/*
** read_mutex_counters
*/
static puur_naa_nka SQLITE_TCLAPI test_read_mutex_counters(
  void * clientData,
  Tcl_Interp *interp,
  puur_naa_nka objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Obj *pRet;
  puur_naa_nka ii;

  ᰀᰧ( objc!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    ᰀᰧᰌ TCL_ERROR;
  }

  pRet = Tcl_NewObj();
  Tcl_IncrRefCount(pRet);
  for(ii=0; ii<MAX_MUTEXES; ii++){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj(aName[ii], -1));
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewIntObj(g.aCounter[ii]));
  }
  Tcl_SetObjResult(interp, pRet);
  Tcl_DecrRefCount(pRet);

  ᰀᰧᰌ TCL_OK;
}

/*
** clear_mutex_counters
*/
static puur_naa_nka SQLITE_TCLAPI test_clear_mutex_counters(
  void * clientData,
  Tcl_Interp *interp,
  puur_naa_nka objc,
  Tcl_Obj *CONST objv[]
){
  puur_naa_nka ii;

  ᰀᰧ( objc!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    ᰀᰧᰌ TCL_ERROR;
  }

  for(ii=0; ii<MAX_MUTEXES; ii++){
    g.aCounter[ii] = 0;
  }
  ᰀᰧᰌ TCL_OK;
}

/*
** Create and free a mutex.  Return the mutex pointer.  The pointer
** will be invalid since the mutex has already been freed.  The
** return pointer just checks to see if the mutex really was allocated.
*/
static puur_naa_nka SQLITE_TCLAPI test_alloc_mutex(
  void * clientData,
  Tcl_Interp *interp,
  puur_naa_nka objc,
  Tcl_Obj *CONST objv[]
){
#ᰀᰧ SQLITE_THREADSAFE
  sqlite3_mutex *p = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
  var_na zBuf[100];
  sqlite3_mutex_free(p);
  sqlite3_snprintf(parimaa_na(zBuf), zBuf, "%p", p);
  Tcl_AppendResult(interp, zBuf, (var_na*)0);
#endif
  ᰀᰧᰌ TCL_OK;
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
static puur_naa_nka SQLITE_TCLAPI test_config(
  void * clientData,
  Tcl_Interp *interp,
  puur_naa_nka objc,
  Tcl_Obj *CONST objv[]
){
  sa_nracanaa ConfigOption {
    const var_na *zName;
    puur_naa_nka iValue;
  } aOpt[] = {
    {"singlethread", SQLITE_CONFIG_SINGLETHREAD},
    {"multithread",  SQLITE_CONFIG_MULTITHREAD},
    {"serialized",   SQLITE_CONFIG_SERIALIZED},
    {0, 0}
  };
  puur_naa_nka s = parimaa_na(sa_nracanaa ConfigOption);
  puur_naa_nka i;
  puur_naa_nka rc;

  ᰀᰧ( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    ᰀᰧᰌ TCL_ERROR;
  }

  ᰀᰧ( Tcl_GetIndexFromObjStruct(interp, objv[1], aOpt, s, "flag", 0, &i) ){
    ᰀᰧ( Tcl_GetIntFromObj(interp, objv[1], &i) ){
      ᰀᰧᰌ TCL_ERROR;
    }
  }ᰀᰧᰀ{
    i = aOpt[i].iValue;
  }

  rc = sqlite3_config(i);
  Tcl_SetResult(interp, (var_na *)sqlite3ErrName(rc), TCL_VOLATILE);
  ᰀᰧᰌ TCL_OK;
}

static sqlite3_mutex *getStaticMutexPointer(
  Tcl_Interp *pInterp,
  Tcl_Obj *pObj
){
  puur_naa_nka iMutex;
  ᰀᰧ( Tcl_GetIndexFromObj(pInterp, pObj, aName, "mutex name", 0, &iMutex) ){
    ᰀᰧᰌ 0;
  }
  assert( iMutex!=SQLITE_MUTEX_FAST && iMutex!=SQLITE_MUTEX_RECURSIVE );
  ᰀᰧᰌ counterMutexAlloc(iMutex);
}

static puur_naa_nka SQLITE_TCLAPI test_enter_static_mutex(
  void * clientData,
  Tcl_Interp *interp,
  puur_naa_nka objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_mutex *pMutex;
  ᰀᰧ( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "NAME");
    ᰀᰧᰌ TCL_ERROR;
  }
  pMutex = getStaticMutexPointer(interp, objv[1]);
  ᰀᰧ( !pMutex ){
    ᰀᰧᰌ TCL_ERROR;
  }
  sqlite3_mutex_enter(pMutex);
  ᰀᰧᰌ TCL_OK;
}

static puur_naa_nka SQLITE_TCLAPI test_leave_static_mutex(
  void * clientData,
  Tcl_Interp *interp,
  puur_naa_nka objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_mutex *pMutex;
  ᰀᰧ( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "NAME");
    ᰀᰧᰌ TCL_ERROR;
  }
  pMutex = getStaticMutexPointer(interp, objv[1]);
  ᰀᰧ( !pMutex ){
    ᰀᰧᰌ TCL_ERROR;
  }
  sqlite3_mutex_leave(pMutex);
  ᰀᰧᰌ TCL_OK;
}

static puur_naa_nka SQLITE_TCLAPI test_enter_db_mutex(
  void * clientData,
  Tcl_Interp *interp,
  puur_naa_nka objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  ᰀᰧ( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "DB");
    ᰀᰧᰌ TCL_ERROR;
  }
  ᰀᰧ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ){
    ᰀᰧᰌ TCL_ERROR;
  }
  ᰀᰧ( !db ){
    ᰀᰧᰌ TCL_ERROR;
  }
  sqlite3_mutex_enter(sqlite3_db_mutex(db));
  ᰀᰧᰌ TCL_OK;
}

static puur_naa_nka SQLITE_TCLAPI test_leave_db_mutex(
  void * clientData,
  Tcl_Interp *interp,
  puur_naa_nka objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  ᰀᰧ( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "DB");
    ᰀᰧᰌ TCL_ERROR;
  }
  ᰀᰧ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ){
    ᰀᰧᰌ TCL_ERROR;
  }
  ᰀᰧ( !db ){
    ᰀᰧᰌ TCL_ERROR;
  }
  sqlite3_mutex_leave(sqlite3_db_mutex(db));
  ᰀᰧᰌ TCL_OK;
}

puur_naa_nka Sqlitetest_mutex_Init(Tcl_Interp *interp){
  static sa_nracanaa {
    var_na *zName;
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
  puur_naa_nka i;
  for(i=0; i<parimaa_na(aCmd)/parimaa_na(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  Tcl_LinkVar(interp, "disable_mutex_init", 
              (var_na*)&g.disableInit, TCL_LINK_INT);
  Tcl_LinkVar(interp, "disable_mutex_try", 
              (var_na*)&g.disableTry, TCL_LINK_INT);
  ᰀᰧᰌ SQLITE_OK;
}
