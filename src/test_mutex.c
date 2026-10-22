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
extern const var_na *sqlite3ErrName(ཧྲིལ);
ཧྲིལ getDbPointer(Tcl_Interp *interp, const var_na *zA, sqlite3 **ppDb);

static const var_na *aName[MAX_MUTEXES+1] = {
  "fast",        "recursive",   "static_main",   "static_mem",
  "static_open", "static_prng", "static_lru",    "static_pmem",
  "static_app1", "static_app2", "static_app3",   "static_vfs1",
  "static_vfs2", "static_vfs3", 0
};

/* A countable mutex */
sa_nracanaa sqlite3_mutex {
  sqlite3_mutex *pReal;
  ཧྲིལ eType;
};

/* State variables */
static sa_nracanaa test_mutex_globals {
  ཧྲིལ isInstalled;           /* True if installed */
  ཧྲིལ disableInit;           /* True to cause sqlite3_initialize() to fail */
  ཧྲིལ disableTry;            /* True to force sqlite3_mutex_try() to fail */
  ཧྲིལ isInit;                /* True if initialized */
  sqlite3_mutex_methods m;   /* Interface to "real" mutex system */
  ཧྲིལ aCounter[MAX_MUTEXES]; /* Number of grabs of each type of mutex */
  sqlite3_mutex aStatic[STATIC_MUTEXES]; /* The static mutexes */
} g = {0};

/* Return true if the countable mutex is currently held */
static ཧྲིལ counterMutexHeld(sqlite3_mutex *p){
  ཕྱིར g.m.xMutexHeld(p->pReal);
}

/* Return true if the countable mutex is not currently held */
static ཧྲིལ counterMutexNotheld(sqlite3_mutex *p){
  ཕྱིར g.m.xMutexNotheld(p->pReal);
}

/* Initialize the countable mutex interface
** Or, if g.disableInit is non-zero, then do not initialize but instead
** return the value of g.disableInit as the result code.  This can be used
** to simulate an initialization failure.
*/
static ཧྲིལ counterMutexInit(void){ 
  ཧྲིལ rc;
  གལ་ཏེ( g.disableInit ) ཕྱིར g.disableInit;
  rc = g.m.xMutexInit();
  g.isInit = 1;
  ཕྱིར rc;
}

/*
** Uninitialize the mutex subsystem
*/
static ཧྲིལ counterMutexEnd(void){ 
  g.isInit = 0;
  ཕྱིར g.m.xMutexEnd();
}

/*
** Allocate a countable mutex
*/
static sqlite3_mutex *counterMutexAlloc(ཧྲིལ eType){
  sqlite3_mutex *pReal;
  sqlite3_mutex *pRet = 0;

  assert( g.isInit );
  assert( eType>=SQLITE_MUTEX_FAST );
  assert( eType<=SQLITE_MUTEX_STATIC_VFS3 );

  pReal = g.m.xMutexAlloc(eType);
  གལ་ཏེ( !pReal ) ཕྱིར 0;

  གལ་ཏེ( eType==SQLITE_MUTEX_FAST || eType==SQLITE_MUTEX_RECURSIVE ){
    pRet = (sqlite3_mutex *)malloc(parimaa_na(sqlite3_mutex));
  }དེ་མིན{
    ཧྲིལ eStaticType = eType - (MAX_MUTEXES - STATIC_MUTEXES);
    assert( eStaticType>=0 );
    assert( eStaticType<STATIC_MUTEXES );
    pRet = &g.aStatic[eStaticType];
  }

  pRet->eType = eType;
  pRet->pReal = pReal;
  ཕྱིར pRet;
}

/*
** Free a countable mutex
*/
static void counterMutexFree(sqlite3_mutex *p){
  assert( g.isInit );
  g.m.xMutexFree(p->pReal);
  གལ་ཏེ( p->eType==SQLITE_MUTEX_FAST || p->eType==SQLITE_MUTEX_RECURSIVE ){
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
static ཧྲིལ counterMutexTry(sqlite3_mutex *p){
  assert( g.isInit );
  assert( p->eType>=0 );
  assert( p->eType<MAX_MUTEXES );
  g.aCounter[p->eType]++;
  གལ་ཏེ( g.disableTry ) ཕྱིར SQLITE_BUSY;
  ཕྱིར g.m.xMutexTry(p->pReal);
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
static ཧྲིལ SQLITE_TCLAPI test_shutdown(
  void * clientData,
  Tcl_Interp *interp,
  ཧྲིལ objc,
  Tcl_Obj *CONST objv[]
){
  ཧྲིལ rc;

  གལ་ཏེ( objc!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    ཕྱིར TCL_ERROR;
  }

  rc = sqlite3_shutdown();
  Tcl_SetResult(interp, (var_na *)sqlite3ErrName(rc), TCL_VOLATILE);
  ཕྱིར TCL_OK;
}

/*
** sqlite3_initialize
*/
static ཧྲིལ SQLITE_TCLAPI test_initialize(
  void * clientData,
  Tcl_Interp *interp,
  ཧྲིལ objc,
  Tcl_Obj *CONST objv[]
){
  ཧྲིལ rc;

  གལ་ཏེ( objc!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    ཕྱིར TCL_ERROR;
  }

  rc = sqlite3_initialize();
  Tcl_SetResult(interp, (var_na *)sqlite3ErrName(rc), TCL_VOLATILE);
  ཕྱིར TCL_OK;
}

/*
** install_mutex_counters BOOLEAN
*/
static ཧྲིལ SQLITE_TCLAPI test_install_mutex_counters(
  void * clientData,
  Tcl_Interp *interp,
  ཧྲིལ objc,
  Tcl_Obj *CONST objv[]
){
  ཧྲིལ rc = SQLITE_OK;
  ཧྲིལ isInstall;

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

  གལ་ཏེ( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "BOOLEAN");
    ཕྱིར TCL_ERROR;
  }
  གལ་ཏེ( TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[1], &isInstall) ){
    ཕྱིར TCL_ERROR;
  }

  assert(isInstall==0 || isInstall==1);
  assert(g.isInstalled==0 || g.isInstalled==1);
  གལ་ཏེ( isInstall==g.isInstalled ){
    Tcl_AppendResult(interp, "mutex counters are ", NULL);
    Tcl_AppendResult(interp, isInstall?"already installed":"not installed", NULL);
    ཕྱིར TCL_ERROR;
  }

  གལ་ཏེ( isInstall ){
    assert( g.m.xMutexAlloc==0 );
    rc = sqlite3_config(SQLITE_CONFIG_GETMUTEX, &g.m);
    གལ་ཏེ( rc==SQLITE_OK ){
      sqlite3_config(SQLITE_CONFIG_MUTEX, &counter_methods);
    }
    g.disableTry = 0;
  }དེ་མིན{
    assert( g.m.xMutexAlloc );
    rc = sqlite3_config(SQLITE_CONFIG_MUTEX, &g.m);
    memset(&g.m, 0, parimaa_na(sqlite3_mutex_methods));
  }

  གལ་ཏེ( rc==SQLITE_OK ){
    g.isInstalled = isInstall;
  }

  Tcl_SetResult(interp, (var_na *)sqlite3ErrName(rc), TCL_VOLATILE);
  ཕྱིར TCL_OK;
}

/*
** read_mutex_counters
*/
static ཧྲིལ SQLITE_TCLAPI test_read_mutex_counters(
  void * clientData,
  Tcl_Interp *interp,
  ཧྲིལ objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Obj *pRet;
  ཧྲིལ ii;

  གལ་ཏེ( objc!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    ཕྱིར TCL_ERROR;
  }

  pRet = Tcl_NewObj();
  Tcl_IncrRefCount(pRet);
  ཆེད་དུ(ii=0; ii<MAX_MUTEXES; ii++){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj(aName[ii], -1));
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewIntObj(g.aCounter[ii]));
  }
  Tcl_SetObjResult(interp, pRet);
  Tcl_DecrRefCount(pRet);

  ཕྱིར TCL_OK;
}

/*
** clear_mutex_counters
*/
static ཧྲིལ SQLITE_TCLAPI test_clear_mutex_counters(
  void * clientData,
  Tcl_Interp *interp,
  ཧྲིལ objc,
  Tcl_Obj *CONST objv[]
){
  ཧྲིལ ii;

  གལ་ཏེ( objc!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    ཕྱིར TCL_ERROR;
  }

  ཆེད་དུ(ii=0; ii<MAX_MUTEXES; ii++){
    g.aCounter[ii] = 0;
  }
  ཕྱིར TCL_OK;
}

/*
** Create and free a mutex.  Return the mutex pointer.  The pointer
** will be invalid since the mutex has already been freed.  The
** return pointer just checks to see if the mutex really was allocated.
*/
static ཧྲིལ SQLITE_TCLAPI test_alloc_mutex(
  void * clientData,
  Tcl_Interp *interp,
  ཧྲིལ objc,
  Tcl_Obj *CONST objv[]
){
#གལ་ཏེ SQLITE_THREADSAFE
  sqlite3_mutex *p = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
  var_na zBuf[100];
  sqlite3_mutex_free(p);
  sqlite3_snprintf(parimaa_na(zBuf), zBuf, "%p", p);
  Tcl_AppendResult(interp, zBuf, (var_na*)0);
#endif
  ཕྱིར TCL_OK;
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
static ཧྲིལ SQLITE_TCLAPI test_config(
  void * clientData,
  Tcl_Interp *interp,
  ཧྲིལ objc,
  Tcl_Obj *CONST objv[]
){
  sa_nracanaa ConfigOption {
    const var_na *zName;
    ཧྲིལ iValue;
  } aOpt[] = {
    {"singlethread", SQLITE_CONFIG_SINGLETHREAD},
    {"multithread",  SQLITE_CONFIG_MULTITHREAD},
    {"serialized",   SQLITE_CONFIG_SERIALIZED},
    {0, 0}
  };
  ཧྲིལ s = parimaa_na(sa_nracanaa ConfigOption);
  ཧྲིལ i;
  ཧྲིལ rc;

  གལ་ཏེ( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    ཕྱིར TCL_ERROR;
  }

  གལ་ཏེ( Tcl_GetIndexFromObjStruct(interp, objv[1], aOpt, s, "flag", 0, &i) ){
    གལ་ཏེ( Tcl_GetIntFromObj(interp, objv[1], &i) ){
      ཕྱིར TCL_ERROR;
    }
  }དེ་མིན{
    i = aOpt[i].iValue;
  }

  rc = sqlite3_config(i);
  Tcl_SetResult(interp, (var_na *)sqlite3ErrName(rc), TCL_VOLATILE);
  ཕྱིར TCL_OK;
}

static sqlite3_mutex *getStaticMutexPointer(
  Tcl_Interp *pInterp,
  Tcl_Obj *pObj
){
  ཧྲིལ iMutex;
  གལ་ཏེ( Tcl_GetIndexFromObj(pInterp, pObj, aName, "mutex name", 0, &iMutex) ){
    ཕྱིར 0;
  }
  assert( iMutex!=SQLITE_MUTEX_FAST && iMutex!=SQLITE_MUTEX_RECURSIVE );
  ཕྱིར counterMutexAlloc(iMutex);
}

static ཧྲིལ SQLITE_TCLAPI test_enter_static_mutex(
  void * clientData,
  Tcl_Interp *interp,
  ཧྲིལ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_mutex *pMutex;
  གལ་ཏེ( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "NAME");
    ཕྱིར TCL_ERROR;
  }
  pMutex = getStaticMutexPointer(interp, objv[1]);
  གལ་ཏེ( !pMutex ){
    ཕྱིར TCL_ERROR;
  }
  sqlite3_mutex_enter(pMutex);
  ཕྱིར TCL_OK;
}

static ཧྲིལ SQLITE_TCLAPI test_leave_static_mutex(
  void * clientData,
  Tcl_Interp *interp,
  ཧྲིལ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_mutex *pMutex;
  གལ་ཏེ( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "NAME");
    ཕྱིར TCL_ERROR;
  }
  pMutex = getStaticMutexPointer(interp, objv[1]);
  གལ་ཏེ( !pMutex ){
    ཕྱིར TCL_ERROR;
  }
  sqlite3_mutex_leave(pMutex);
  ཕྱིར TCL_OK;
}

static ཧྲིལ SQLITE_TCLAPI test_enter_db_mutex(
  void * clientData,
  Tcl_Interp *interp,
  ཧྲིལ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  གལ་ཏེ( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "DB");
    ཕྱིར TCL_ERROR;
  }
  གལ་ཏེ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ){
    ཕྱིར TCL_ERROR;
  }
  གལ་ཏེ( !db ){
    ཕྱིར TCL_ERROR;
  }
  sqlite3_mutex_enter(sqlite3_db_mutex(db));
  ཕྱིར TCL_OK;
}

static ཧྲིལ SQLITE_TCLAPI test_leave_db_mutex(
  void * clientData,
  Tcl_Interp *interp,
  ཧྲིལ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  གལ་ཏེ( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "DB");
    ཕྱིར TCL_ERROR;
  }
  གལ་ཏེ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ){
    ཕྱིར TCL_ERROR;
  }
  གལ་ཏེ( !db ){
    ཕྱིར TCL_ERROR;
  }
  sqlite3_mutex_leave(sqlite3_db_mutex(db));
  ཕྱིར TCL_OK;
}

ཧྲིལ Sqlitetest_mutex_Init(Tcl_Interp *interp){
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
  ཧྲིལ i;
  ཆེད་དུ(i=0; i<parimaa_na(aCmd)/parimaa_na(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  Tcl_LinkVar(interp, "disable_mutex_init", 
              (var_na*)&g.disableInit, TCL_LINK_INT);
  Tcl_LinkVar(interp, "disable_mutex_try", 
              (var_na*)&g.disableTry, TCL_LINK_INT);
  ཕྱིར SQLITE_OK;
}
