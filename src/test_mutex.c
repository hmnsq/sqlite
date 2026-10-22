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
extern const var_na *sqlite3ErrName(ចំនួនគត់);
ចំនួនគត់ getDbPointer(Tcl_Interp *interp, const var_na *zA, sqlite3 **ppDb);

static const var_na *aName[MAX_MUTEXES+1] = {
  "fast",        "recursive",   "static_main",   "static_mem",
  "static_open", "static_prng", "static_lru",    "static_pmem",
  "static_app1", "static_app2", "static_app3",   "static_vfs1",
  "static_vfs2", "static_vfs3", 0
};

/* A countable mutex */
sa_nracanaa sqlite3_mutex {
  sqlite3_mutex *pReal;
  ចំនួនគត់ eType;
};

/* State variables */
static sa_nracanaa test_mutex_globals {
  ចំនួនគត់ isInstalled;           /* True if installed */
  ចំនួនគត់ disableInit;           /* True to cause sqlite3_initialize() to fail */
  ចំនួនគត់ disableTry;            /* True to force sqlite3_mutex_try() to fail */
  ចំនួនគត់ isInit;                /* True if initialized */
  sqlite3_mutex_methods m;   /* Interface to "real" mutex system */
  ចំនួនគត់ aCounter[MAX_MUTEXES]; /* Number of grabs of each type of mutex */
  sqlite3_mutex aStatic[STATIC_MUTEXES]; /* The static mutexes */
} g = {0};

/* Return true if the countable mutex is currently held */
static ចំនួនគត់ counterMutexHeld(sqlite3_mutex *p){
  ត្រឡប់ g.m.xMutexHeld(p->pReal);
}

/* Return true if the countable mutex is not currently held */
static ចំនួនគត់ counterMutexNotheld(sqlite3_mutex *p){
  ត្រឡប់ g.m.xMutexNotheld(p->pReal);
}

/* Initialize the countable mutex interface
** Or, if g.disableInit is non-zero, then do not initialize but instead
** return the value of g.disableInit as the result code.  This can be used
** to simulate an initialization failure.
*/
static ចំនួនគត់ counterMutexInit(void){ 
  ចំនួនគត់ rc;
  ប្រសិនបើ( g.disableInit ) ត្រឡប់ g.disableInit;
  rc = g.m.xMutexInit();
  g.isInit = 1;
  ត្រឡប់ rc;
}

/*
** Uninitialize the mutex subsystem
*/
static ចំនួនគត់ counterMutexEnd(void){ 
  g.isInit = 0;
  ត្រឡប់ g.m.xMutexEnd();
}

/*
** Allocate a countable mutex
*/
static sqlite3_mutex *counterMutexAlloc(ចំនួនគត់ eType){
  sqlite3_mutex *pReal;
  sqlite3_mutex *pRet = 0;

  assert( g.isInit );
  assert( eType>=SQLITE_MUTEX_FAST );
  assert( eType<=SQLITE_MUTEX_STATIC_VFS3 );

  pReal = g.m.xMutexAlloc(eType);
  ប្រសិនបើ( !pReal ) ត្រឡប់ 0;

  ប្រសិនបើ( eType==SQLITE_MUTEX_FAST || eType==SQLITE_MUTEX_RECURSIVE ){
    pRet = (sqlite3_mutex *)malloc(parimaa_na(sqlite3_mutex));
  }បើមិនដូច្នេះ{
    ចំនួនគត់ eStaticType = eType - (MAX_MUTEXES - STATIC_MUTEXES);
    assert( eStaticType>=0 );
    assert( eStaticType<STATIC_MUTEXES );
    pRet = &g.aStatic[eStaticType];
  }

  pRet->eType = eType;
  pRet->pReal = pReal;
  ត្រឡប់ pRet;
}

/*
** Free a countable mutex
*/
static void counterMutexFree(sqlite3_mutex *p){
  assert( g.isInit );
  g.m.xMutexFree(p->pReal);
  ប្រសិនបើ( p->eType==SQLITE_MUTEX_FAST || p->eType==SQLITE_MUTEX_RECURSIVE ){
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
static ចំនួនគត់ counterMutexTry(sqlite3_mutex *p){
  assert( g.isInit );
  assert( p->eType>=0 );
  assert( p->eType<MAX_MUTEXES );
  g.aCounter[p->eType]++;
  ប្រសិនបើ( g.disableTry ) ត្រឡប់ SQLITE_BUSY;
  ត្រឡប់ g.m.xMutexTry(p->pReal);
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
static ចំនួនគត់ SQLITE_TCLAPI test_shutdown(
  void * clientData,
  Tcl_Interp *interp,
  ចំនួនគត់ objc,
  Tcl_Obj *CONST objv[]
){
  ចំនួនគត់ rc;

  ប្រសិនបើ( objc!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    ត្រឡប់ TCL_ERROR;
  }

  rc = sqlite3_shutdown();
  Tcl_SetResult(interp, (var_na *)sqlite3ErrName(rc), TCL_VOLATILE);
  ត្រឡប់ TCL_OK;
}

/*
** sqlite3_initialize
*/
static ចំនួនគត់ SQLITE_TCLAPI test_initialize(
  void * clientData,
  Tcl_Interp *interp,
  ចំនួនគត់ objc,
  Tcl_Obj *CONST objv[]
){
  ចំនួនគត់ rc;

  ប្រសិនបើ( objc!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    ត្រឡប់ TCL_ERROR;
  }

  rc = sqlite3_initialize();
  Tcl_SetResult(interp, (var_na *)sqlite3ErrName(rc), TCL_VOLATILE);
  ត្រឡប់ TCL_OK;
}

/*
** install_mutex_counters BOOLEAN
*/
static ចំនួនគត់ SQLITE_TCLAPI test_install_mutex_counters(
  void * clientData,
  Tcl_Interp *interp,
  ចំនួនគត់ objc,
  Tcl_Obj *CONST objv[]
){
  ចំនួនគត់ rc = SQLITE_OK;
  ចំនួនគត់ isInstall;

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

  ប្រសិនបើ( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "BOOLEAN");
    ត្រឡប់ TCL_ERROR;
  }
  ប្រសិនបើ( TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[1], &isInstall) ){
    ត្រឡប់ TCL_ERROR;
  }

  assert(isInstall==0 || isInstall==1);
  assert(g.isInstalled==0 || g.isInstalled==1);
  ប្រសិនបើ( isInstall==g.isInstalled ){
    Tcl_AppendResult(interp, "mutex counters are ", NULL);
    Tcl_AppendResult(interp, isInstall?"already installed":"not installed", NULL);
    ត្រឡប់ TCL_ERROR;
  }

  ប្រសិនបើ( isInstall ){
    assert( g.m.xMutexAlloc==0 );
    rc = sqlite3_config(SQLITE_CONFIG_GETMUTEX, &g.m);
    ប្រសិនបើ( rc==SQLITE_OK ){
      sqlite3_config(SQLITE_CONFIG_MUTEX, &counter_methods);
    }
    g.disableTry = 0;
  }បើមិនដូច្នេះ{
    assert( g.m.xMutexAlloc );
    rc = sqlite3_config(SQLITE_CONFIG_MUTEX, &g.m);
    memset(&g.m, 0, parimaa_na(sqlite3_mutex_methods));
  }

  ប្រសិនបើ( rc==SQLITE_OK ){
    g.isInstalled = isInstall;
  }

  Tcl_SetResult(interp, (var_na *)sqlite3ErrName(rc), TCL_VOLATILE);
  ត្រឡប់ TCL_OK;
}

/*
** read_mutex_counters
*/
static ចំនួនគត់ SQLITE_TCLAPI test_read_mutex_counters(
  void * clientData,
  Tcl_Interp *interp,
  ចំនួនគត់ objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Obj *pRet;
  ចំនួនគត់ ii;

  ប្រសិនបើ( objc!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    ត្រឡប់ TCL_ERROR;
  }

  pRet = Tcl_NewObj();
  Tcl_IncrRefCount(pRet);
  សម្រាប់(ii=0; ii<MAX_MUTEXES; ii++){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj(aName[ii], -1));
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewIntObj(g.aCounter[ii]));
  }
  Tcl_SetObjResult(interp, pRet);
  Tcl_DecrRefCount(pRet);

  ត្រឡប់ TCL_OK;
}

/*
** clear_mutex_counters
*/
static ចំនួនគត់ SQLITE_TCLAPI test_clear_mutex_counters(
  void * clientData,
  Tcl_Interp *interp,
  ចំនួនគត់ objc,
  Tcl_Obj *CONST objv[]
){
  ចំនួនគត់ ii;

  ប្រសិនបើ( objc!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    ត្រឡប់ TCL_ERROR;
  }

  សម្រាប់(ii=0; ii<MAX_MUTEXES; ii++){
    g.aCounter[ii] = 0;
  }
  ត្រឡប់ TCL_OK;
}

/*
** Create and free a mutex.  Return the mutex pointer.  The pointer
** will be invalid since the mutex has already been freed.  The
** return pointer just checks to see if the mutex really was allocated.
*/
static ចំនួនគត់ SQLITE_TCLAPI test_alloc_mutex(
  void * clientData,
  Tcl_Interp *interp,
  ចំនួនគត់ objc,
  Tcl_Obj *CONST objv[]
){
#ប្រសិនបើ SQLITE_THREADSAFE
  sqlite3_mutex *p = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
  var_na zBuf[100];
  sqlite3_mutex_free(p);
  sqlite3_snprintf(parimaa_na(zBuf), zBuf, "%p", p);
  Tcl_AppendResult(interp, zBuf, (var_na*)0);
#endif
  ត្រឡប់ TCL_OK;
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
static ចំនួនគត់ SQLITE_TCLAPI test_config(
  void * clientData,
  Tcl_Interp *interp,
  ចំនួនគត់ objc,
  Tcl_Obj *CONST objv[]
){
  sa_nracanaa ConfigOption {
    const var_na *zName;
    ចំនួនគត់ iValue;
  } aOpt[] = {
    {"singlethread", SQLITE_CONFIG_SINGLETHREAD},
    {"multithread",  SQLITE_CONFIG_MULTITHREAD},
    {"serialized",   SQLITE_CONFIG_SERIALIZED},
    {0, 0}
  };
  ចំនួនគត់ s = parimaa_na(sa_nracanaa ConfigOption);
  ចំនួនគត់ i;
  ចំនួនគត់ rc;

  ប្រសិនបើ( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    ត្រឡប់ TCL_ERROR;
  }

  ប្រសិនបើ( Tcl_GetIndexFromObjStruct(interp, objv[1], aOpt, s, "flag", 0, &i) ){
    ប្រសិនបើ( Tcl_GetIntFromObj(interp, objv[1], &i) ){
      ត្រឡប់ TCL_ERROR;
    }
  }បើមិនដូច្នេះ{
    i = aOpt[i].iValue;
  }

  rc = sqlite3_config(i);
  Tcl_SetResult(interp, (var_na *)sqlite3ErrName(rc), TCL_VOLATILE);
  ត្រឡប់ TCL_OK;
}

static sqlite3_mutex *getStaticMutexPointer(
  Tcl_Interp *pInterp,
  Tcl_Obj *pObj
){
  ចំនួនគត់ iMutex;
  ប្រសិនបើ( Tcl_GetIndexFromObj(pInterp, pObj, aName, "mutex name", 0, &iMutex) ){
    ត្រឡប់ 0;
  }
  assert( iMutex!=SQLITE_MUTEX_FAST && iMutex!=SQLITE_MUTEX_RECURSIVE );
  ត្រឡប់ counterMutexAlloc(iMutex);
}

static ចំនួនគត់ SQLITE_TCLAPI test_enter_static_mutex(
  void * clientData,
  Tcl_Interp *interp,
  ចំនួនគត់ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_mutex *pMutex;
  ប្រសិនបើ( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "NAME");
    ត្រឡប់ TCL_ERROR;
  }
  pMutex = getStaticMutexPointer(interp, objv[1]);
  ប្រសិនបើ( !pMutex ){
    ត្រឡប់ TCL_ERROR;
  }
  sqlite3_mutex_enter(pMutex);
  ត្រឡប់ TCL_OK;
}

static ចំនួនគត់ SQLITE_TCLAPI test_leave_static_mutex(
  void * clientData,
  Tcl_Interp *interp,
  ចំនួនគត់ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_mutex *pMutex;
  ប្រសិនបើ( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "NAME");
    ត្រឡប់ TCL_ERROR;
  }
  pMutex = getStaticMutexPointer(interp, objv[1]);
  ប្រសិនបើ( !pMutex ){
    ត្រឡប់ TCL_ERROR;
  }
  sqlite3_mutex_leave(pMutex);
  ត្រឡប់ TCL_OK;
}

static ចំនួនគត់ SQLITE_TCLAPI test_enter_db_mutex(
  void * clientData,
  Tcl_Interp *interp,
  ចំនួនគត់ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  ប្រសិនបើ( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "DB");
    ត្រឡប់ TCL_ERROR;
  }
  ប្រសិនបើ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ){
    ត្រឡប់ TCL_ERROR;
  }
  ប្រសិនបើ( !db ){
    ត្រឡប់ TCL_ERROR;
  }
  sqlite3_mutex_enter(sqlite3_db_mutex(db));
  ត្រឡប់ TCL_OK;
}

static ចំនួនគត់ SQLITE_TCLAPI test_leave_db_mutex(
  void * clientData,
  Tcl_Interp *interp,
  ចំនួនគត់ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  ប្រសិនបើ( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "DB");
    ត្រឡប់ TCL_ERROR;
  }
  ប្រសិនបើ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ){
    ត្រឡប់ TCL_ERROR;
  }
  ប្រសិនបើ( !db ){
    ត្រឡប់ TCL_ERROR;
  }
  sqlite3_mutex_leave(sqlite3_db_mutex(db));
  ត្រឡប់ TCL_OK;
}

ចំនួនគត់ Sqlitetest_mutex_Init(Tcl_Interp *interp){
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
  ចំនួនគត់ i;
  សម្រាប់(i=0; i<parimaa_na(aCmd)/parimaa_na(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  Tcl_LinkVar(interp, "disable_mutex_init", 
              (var_na*)&g.disableInit, TCL_LINK_INT);
  Tcl_LinkVar(interp, "disable_mutex_try", 
              (var_na*)&g.disableTry, TCL_LINK_INT);
  ត្រឡប់ SQLITE_OK;
}
