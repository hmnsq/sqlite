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
extern const var_na *sqlite3ErrName(จำนวนเต็ม);
จำนวนเต็ม getDbPointer(Tcl_Interp *interp, const var_na *zA, sqlite3 **ppDb);

static const var_na *aName[MAX_MUTEXES+1] = {
  "fast",        "recursive",   "static_main",   "static_mem",
  "static_open", "static_prng", "static_lru",    "static_pmem",
  "static_app1", "static_app2", "static_app3",   "static_vfs1",
  "static_vfs2", "static_vfs3", 0
};

/* A countable mutex */
sa_nracanaa sqlite3_mutex {
  sqlite3_mutex *pReal;
  จำนวนเต็ม eType;
};

/* State variables */
static sa_nracanaa test_mutex_globals {
  จำนวนเต็ม isInstalled;           /* True if installed */
  จำนวนเต็ม disableInit;           /* True to cause sqlite3_initialize() to fail */
  จำนวนเต็ม disableTry;            /* True to force sqlite3_mutex_try() to fail */
  จำนวนเต็ม isInit;                /* True if initialized */
  sqlite3_mutex_methods m;   /* Interface to "real" mutex system */
  จำนวนเต็ม aCounter[MAX_MUTEXES]; /* Number of grabs of each type of mutex */
  sqlite3_mutex aStatic[STATIC_MUTEXES]; /* The static mutexes */
} g = {0};

/* Return true if the countable mutex is currently held */
static จำนวนเต็ม counterMutexHeld(sqlite3_mutex *p){
  คืน g.m.xMutexHeld(p->pReal);
}

/* Return true if the countable mutex is not currently held */
static จำนวนเต็ม counterMutexNotheld(sqlite3_mutex *p){
  คืน g.m.xMutexNotheld(p->pReal);
}

/* Initialize the countable mutex interface
** Or, if g.disableInit is non-zero, then do not initialize but instead
** return the value of g.disableInit as the result code.  This can be used
** to simulate an initialization failure.
*/
static จำนวนเต็ม counterMutexInit(void){ 
  จำนวนเต็ม rc;
  ถ้า( g.disableInit ) คืน g.disableInit;
  rc = g.m.xMutexInit();
  g.isInit = 1;
  คืน rc;
}

/*
** Uninitialize the mutex subsystem
*/
static จำนวนเต็ม counterMutexEnd(void){ 
  g.isInit = 0;
  คืน g.m.xMutexEnd();
}

/*
** Allocate a countable mutex
*/
static sqlite3_mutex *counterMutexAlloc(จำนวนเต็ม eType){
  sqlite3_mutex *pReal;
  sqlite3_mutex *pRet = 0;

  assert( g.isInit );
  assert( eType>=SQLITE_MUTEX_FAST );
  assert( eType<=SQLITE_MUTEX_STATIC_VFS3 );

  pReal = g.m.xMutexAlloc(eType);
  ถ้า( !pReal ) คืน 0;

  ถ้า( eType==SQLITE_MUTEX_FAST || eType==SQLITE_MUTEX_RECURSIVE ){
    pRet = (sqlite3_mutex *)malloc(parimaa_na(sqlite3_mutex));
  }มิฉะนั้น{
    จำนวนเต็ม eStaticType = eType - (MAX_MUTEXES - STATIC_MUTEXES);
    assert( eStaticType>=0 );
    assert( eStaticType<STATIC_MUTEXES );
    pRet = &g.aStatic[eStaticType];
  }

  pRet->eType = eType;
  pRet->pReal = pReal;
  คืน pRet;
}

/*
** Free a countable mutex
*/
static void counterMutexFree(sqlite3_mutex *p){
  assert( g.isInit );
  g.m.xMutexFree(p->pReal);
  ถ้า( p->eType==SQLITE_MUTEX_FAST || p->eType==SQLITE_MUTEX_RECURSIVE ){
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
static จำนวนเต็ม counterMutexTry(sqlite3_mutex *p){
  assert( g.isInit );
  assert( p->eType>=0 );
  assert( p->eType<MAX_MUTEXES );
  g.aCounter[p->eType]++;
  ถ้า( g.disableTry ) คืน SQLITE_BUSY;
  คืน g.m.xMutexTry(p->pReal);
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
static จำนวนเต็ม SQLITE_TCLAPI test_shutdown(
  void * clientData,
  Tcl_Interp *interp,
  จำนวนเต็ม objc,
  Tcl_Obj *CONST objv[]
){
  จำนวนเต็ม rc;

  ถ้า( objc!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    คืน TCL_ERROR;
  }

  rc = sqlite3_shutdown();
  Tcl_SetResult(interp, (var_na *)sqlite3ErrName(rc), TCL_VOLATILE);
  คืน TCL_OK;
}

/*
** sqlite3_initialize
*/
static จำนวนเต็ม SQLITE_TCLAPI test_initialize(
  void * clientData,
  Tcl_Interp *interp,
  จำนวนเต็ม objc,
  Tcl_Obj *CONST objv[]
){
  จำนวนเต็ม rc;

  ถ้า( objc!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    คืน TCL_ERROR;
  }

  rc = sqlite3_initialize();
  Tcl_SetResult(interp, (var_na *)sqlite3ErrName(rc), TCL_VOLATILE);
  คืน TCL_OK;
}

/*
** install_mutex_counters BOOLEAN
*/
static จำนวนเต็ม SQLITE_TCLAPI test_install_mutex_counters(
  void * clientData,
  Tcl_Interp *interp,
  จำนวนเต็ม objc,
  Tcl_Obj *CONST objv[]
){
  จำนวนเต็ม rc = SQLITE_OK;
  จำนวนเต็ม isInstall;

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

  ถ้า( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "BOOLEAN");
    คืน TCL_ERROR;
  }
  ถ้า( TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[1], &isInstall) ){
    คืน TCL_ERROR;
  }

  assert(isInstall==0 || isInstall==1);
  assert(g.isInstalled==0 || g.isInstalled==1);
  ถ้า( isInstall==g.isInstalled ){
    Tcl_AppendResult(interp, "mutex counters are ", NULL);
    Tcl_AppendResult(interp, isInstall?"already installed":"not installed", NULL);
    คืน TCL_ERROR;
  }

  ถ้า( isInstall ){
    assert( g.m.xMutexAlloc==0 );
    rc = sqlite3_config(SQLITE_CONFIG_GETMUTEX, &g.m);
    ถ้า( rc==SQLITE_OK ){
      sqlite3_config(SQLITE_CONFIG_MUTEX, &counter_methods);
    }
    g.disableTry = 0;
  }มิฉะนั้น{
    assert( g.m.xMutexAlloc );
    rc = sqlite3_config(SQLITE_CONFIG_MUTEX, &g.m);
    memset(&g.m, 0, parimaa_na(sqlite3_mutex_methods));
  }

  ถ้า( rc==SQLITE_OK ){
    g.isInstalled = isInstall;
  }

  Tcl_SetResult(interp, (var_na *)sqlite3ErrName(rc), TCL_VOLATILE);
  คืน TCL_OK;
}

/*
** read_mutex_counters
*/
static จำนวนเต็ม SQLITE_TCLAPI test_read_mutex_counters(
  void * clientData,
  Tcl_Interp *interp,
  จำนวนเต็ม objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Obj *pRet;
  จำนวนเต็ม ii;

  ถ้า( objc!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    คืน TCL_ERROR;
  }

  pRet = Tcl_NewObj();
  Tcl_IncrRefCount(pRet);
  สำหรับ(ii=0; ii<MAX_MUTEXES; ii++){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj(aName[ii], -1));
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewIntObj(g.aCounter[ii]));
  }
  Tcl_SetObjResult(interp, pRet);
  Tcl_DecrRefCount(pRet);

  คืน TCL_OK;
}

/*
** clear_mutex_counters
*/
static จำนวนเต็ม SQLITE_TCLAPI test_clear_mutex_counters(
  void * clientData,
  Tcl_Interp *interp,
  จำนวนเต็ม objc,
  Tcl_Obj *CONST objv[]
){
  จำนวนเต็ม ii;

  ถ้า( objc!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    คืน TCL_ERROR;
  }

  สำหรับ(ii=0; ii<MAX_MUTEXES; ii++){
    g.aCounter[ii] = 0;
  }
  คืน TCL_OK;
}

/*
** Create and free a mutex.  Return the mutex pointer.  The pointer
** will be invalid since the mutex has already been freed.  The
** return pointer just checks to see if the mutex really was allocated.
*/
static จำนวนเต็ม SQLITE_TCLAPI test_alloc_mutex(
  void * clientData,
  Tcl_Interp *interp,
  จำนวนเต็ม objc,
  Tcl_Obj *CONST objv[]
){
#ถ้า SQLITE_THREADSAFE
  sqlite3_mutex *p = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
  var_na zBuf[100];
  sqlite3_mutex_free(p);
  sqlite3_snprintf(parimaa_na(zBuf), zBuf, "%p", p);
  Tcl_AppendResult(interp, zBuf, (var_na*)0);
#endif
  คืน TCL_OK;
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
static จำนวนเต็ม SQLITE_TCLAPI test_config(
  void * clientData,
  Tcl_Interp *interp,
  จำนวนเต็ม objc,
  Tcl_Obj *CONST objv[]
){
  sa_nracanaa ConfigOption {
    const var_na *zName;
    จำนวนเต็ม iValue;
  } aOpt[] = {
    {"singlethread", SQLITE_CONFIG_SINGLETHREAD},
    {"multithread",  SQLITE_CONFIG_MULTITHREAD},
    {"serialized",   SQLITE_CONFIG_SERIALIZED},
    {0, 0}
  };
  จำนวนเต็ม s = parimaa_na(sa_nracanaa ConfigOption);
  จำนวนเต็ม i;
  จำนวนเต็ม rc;

  ถ้า( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    คืน TCL_ERROR;
  }

  ถ้า( Tcl_GetIndexFromObjStruct(interp, objv[1], aOpt, s, "flag", 0, &i) ){
    ถ้า( Tcl_GetIntFromObj(interp, objv[1], &i) ){
      คืน TCL_ERROR;
    }
  }มิฉะนั้น{
    i = aOpt[i].iValue;
  }

  rc = sqlite3_config(i);
  Tcl_SetResult(interp, (var_na *)sqlite3ErrName(rc), TCL_VOLATILE);
  คืน TCL_OK;
}

static sqlite3_mutex *getStaticMutexPointer(
  Tcl_Interp *pInterp,
  Tcl_Obj *pObj
){
  จำนวนเต็ม iMutex;
  ถ้า( Tcl_GetIndexFromObj(pInterp, pObj, aName, "mutex name", 0, &iMutex) ){
    คืน 0;
  }
  assert( iMutex!=SQLITE_MUTEX_FAST && iMutex!=SQLITE_MUTEX_RECURSIVE );
  คืน counterMutexAlloc(iMutex);
}

static จำนวนเต็ม SQLITE_TCLAPI test_enter_static_mutex(
  void * clientData,
  Tcl_Interp *interp,
  จำนวนเต็ม objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_mutex *pMutex;
  ถ้า( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "NAME");
    คืน TCL_ERROR;
  }
  pMutex = getStaticMutexPointer(interp, objv[1]);
  ถ้า( !pMutex ){
    คืน TCL_ERROR;
  }
  sqlite3_mutex_enter(pMutex);
  คืน TCL_OK;
}

static จำนวนเต็ม SQLITE_TCLAPI test_leave_static_mutex(
  void * clientData,
  Tcl_Interp *interp,
  จำนวนเต็ม objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_mutex *pMutex;
  ถ้า( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "NAME");
    คืน TCL_ERROR;
  }
  pMutex = getStaticMutexPointer(interp, objv[1]);
  ถ้า( !pMutex ){
    คืน TCL_ERROR;
  }
  sqlite3_mutex_leave(pMutex);
  คืน TCL_OK;
}

static จำนวนเต็ม SQLITE_TCLAPI test_enter_db_mutex(
  void * clientData,
  Tcl_Interp *interp,
  จำนวนเต็ม objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  ถ้า( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "DB");
    คืน TCL_ERROR;
  }
  ถ้า( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ){
    คืน TCL_ERROR;
  }
  ถ้า( !db ){
    คืน TCL_ERROR;
  }
  sqlite3_mutex_enter(sqlite3_db_mutex(db));
  คืน TCL_OK;
}

static จำนวนเต็ม SQLITE_TCLAPI test_leave_db_mutex(
  void * clientData,
  Tcl_Interp *interp,
  จำนวนเต็ม objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  ถ้า( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "DB");
    คืน TCL_ERROR;
  }
  ถ้า( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ){
    คืน TCL_ERROR;
  }
  ถ้า( !db ){
    คืน TCL_ERROR;
  }
  sqlite3_mutex_leave(sqlite3_db_mutex(db));
  คืน TCL_OK;
}

จำนวนเต็ม Sqlitetest_mutex_Init(Tcl_Interp *interp){
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
  จำนวนเต็ม i;
  สำหรับ(i=0; i<parimaa_na(aCmd)/parimaa_na(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  Tcl_LinkVar(interp, "disable_mutex_init", 
              (var_na*)&g.disableInit, TCL_LINK_INT);
  Tcl_LinkVar(interp, "disable_mutex_try", 
              (var_na*)&g.disableTry, TCL_LINK_INT);
  คืน SQLITE_OK;
}
