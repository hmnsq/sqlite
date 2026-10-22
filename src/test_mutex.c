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
extern const char *sqlite3ErrName(số_nguyên);
số_nguyên getDbPointer(Tcl_Interp *interp, const char *zA, sqlite3 **ppDb);

static const char *aName[MAX_MUTEXES+1] = {
  "fast",        "recursive",   "static_main",   "static_mem",
  "static_open", "static_prng", "static_lru",    "static_pmem",
  "static_app1", "static_app2", "static_app3",   "static_vfs1",
  "static_vfs2", "static_vfs3", 0
};

/* A countable mutex */
struct sqlite3_mutex {
  sqlite3_mutex *pReal;
  số_nguyên eType;
};

/* State variables */
static struct test_mutex_globals {
  số_nguyên isInstalled;           /* True if installed */
  số_nguyên disableInit;           /* True to cause sqlite3_initialize() to fail */
  số_nguyên disableTry;            /* True to force sqlite3_mutex_try() to fail */
  số_nguyên isInit;                /* True if initialized */
  sqlite3_mutex_methods m;   /* Interface to "real" mutex system */
  số_nguyên aCounter[MAX_MUTEXES]; /* Number of grabs of each type of mutex */
  sqlite3_mutex aStatic[STATIC_MUTEXES]; /* The static mutexes */
} g = {0};

/* Return true if the countable mutex is currently held */
static số_nguyên counterMutexHeld(sqlite3_mutex *p){
  trả_về g.m.xMutexHeld(p->pReal);
}

/* Return true if the countable mutex is not currently held */
static số_nguyên counterMutexNotheld(sqlite3_mutex *p){
  trả_về g.m.xMutexNotheld(p->pReal);
}

/* Initialize the countable mutex interface
** Or, if g.disableInit is non-zero, then do not initialize but instead
** return the value of g.disableInit as the result code.  This can be used
** to simulate an initialization failure.
*/
static số_nguyên counterMutexInit(void){ 
  số_nguyên rc;
  nếu( g.disableInit ) trả_về g.disableInit;
  rc = g.m.xMutexInit();
  g.isInit = 1;
  trả_về rc;
}

/*
** Uninitialize the mutex subsystem
*/
static số_nguyên counterMutexEnd(void){ 
  g.isInit = 0;
  trả_về g.m.xMutexEnd();
}

/*
** Allocate a countable mutex
*/
static sqlite3_mutex *counterMutexAlloc(số_nguyên eType){
  sqlite3_mutex *pReal;
  sqlite3_mutex *pRet = 0;

  assert( g.isInit );
  assert( eType>=SQLITE_MUTEX_FAST );
  assert( eType<=SQLITE_MUTEX_STATIC_VFS3 );

  pReal = g.m.xMutexAlloc(eType);
  nếu( !pReal ) trả_về 0;

  nếu( eType==SQLITE_MUTEX_FAST || eType==SQLITE_MUTEX_RECURSIVE ){
    pRet = (sqlite3_mutex *)malloc(sizeof(sqlite3_mutex));
  }không_thì{
    số_nguyên eStaticType = eType - (MAX_MUTEXES - STATIC_MUTEXES);
    assert( eStaticType>=0 );
    assert( eStaticType<STATIC_MUTEXES );
    pRet = &g.aStatic[eStaticType];
  }

  pRet->eType = eType;
  pRet->pReal = pReal;
  trả_về pRet;
}

/*
** Free a countable mutex
*/
static void counterMutexFree(sqlite3_mutex *p){
  assert( g.isInit );
  g.m.xMutexFree(p->pReal);
  nếu( p->eType==SQLITE_MUTEX_FAST || p->eType==SQLITE_MUTEX_RECURSIVE ){
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
static số_nguyên counterMutexTry(sqlite3_mutex *p){
  assert( g.isInit );
  assert( p->eType>=0 );
  assert( p->eType<MAX_MUTEXES );
  g.aCounter[p->eType]++;
  nếu( g.disableTry ) trả_về SQLITE_BUSY;
  trả_về g.m.xMutexTry(p->pReal);
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
static số_nguyên SQLITE_TCLAPI test_shutdown(
  void * clientData,
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){
  số_nguyên rc;

  nếu( objc!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    trả_về TCL_ERROR;
  }

  rc = sqlite3_shutdown();
  Tcl_SetResult(interp, (char *)sqlite3ErrName(rc), TCL_VOLATILE);
  trả_về TCL_OK;
}

/*
** sqlite3_initialize
*/
static số_nguyên SQLITE_TCLAPI test_initialize(
  void * clientData,
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){
  số_nguyên rc;

  nếu( objc!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    trả_về TCL_ERROR;
  }

  rc = sqlite3_initialize();
  Tcl_SetResult(interp, (char *)sqlite3ErrName(rc), TCL_VOLATILE);
  trả_về TCL_OK;
}

/*
** install_mutex_counters BOOLEAN
*/
static số_nguyên SQLITE_TCLAPI test_install_mutex_counters(
  void * clientData,
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){
  số_nguyên rc = SQLITE_OK;
  số_nguyên isInstall;

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

  nếu( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "BOOLEAN");
    trả_về TCL_ERROR;
  }
  nếu( TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[1], &isInstall) ){
    trả_về TCL_ERROR;
  }

  assert(isInstall==0 || isInstall==1);
  assert(g.isInstalled==0 || g.isInstalled==1);
  nếu( isInstall==g.isInstalled ){
    Tcl_AppendResult(interp, "mutex counters are ", NULL);
    Tcl_AppendResult(interp, isInstall?"already installed":"not installed", NULL);
    trả_về TCL_ERROR;
  }

  nếu( isInstall ){
    assert( g.m.xMutexAlloc==0 );
    rc = sqlite3_config(SQLITE_CONFIG_GETMUTEX, &g.m);
    nếu( rc==SQLITE_OK ){
      sqlite3_config(SQLITE_CONFIG_MUTEX, &counter_methods);
    }
    g.disableTry = 0;
  }không_thì{
    assert( g.m.xMutexAlloc );
    rc = sqlite3_config(SQLITE_CONFIG_MUTEX, &g.m);
    memset(&g.m, 0, sizeof(sqlite3_mutex_methods));
  }

  nếu( rc==SQLITE_OK ){
    g.isInstalled = isInstall;
  }

  Tcl_SetResult(interp, (char *)sqlite3ErrName(rc), TCL_VOLATILE);
  trả_về TCL_OK;
}

/*
** read_mutex_counters
*/
static số_nguyên SQLITE_TCLAPI test_read_mutex_counters(
  void * clientData,
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Obj *pRet;
  số_nguyên ii;

  nếu( objc!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    trả_về TCL_ERROR;
  }

  pRet = Tcl_NewObj();
  Tcl_IncrRefCount(pRet);
  cho(ii=0; ii<MAX_MUTEXES; ii++){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj(aName[ii], -1));
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewIntObj(g.aCounter[ii]));
  }
  Tcl_SetObjResult(interp, pRet);
  Tcl_DecrRefCount(pRet);

  trả_về TCL_OK;
}

/*
** clear_mutex_counters
*/
static số_nguyên SQLITE_TCLAPI test_clear_mutex_counters(
  void * clientData,
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){
  số_nguyên ii;

  nếu( objc!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    trả_về TCL_ERROR;
  }

  cho(ii=0; ii<MAX_MUTEXES; ii++){
    g.aCounter[ii] = 0;
  }
  trả_về TCL_OK;
}

/*
** Create and free a mutex.  Return the mutex pointer.  The pointer
** will be invalid since the mutex has already been freed.  The
** return pointer just checks to see if the mutex really was allocated.
*/
static số_nguyên SQLITE_TCLAPI test_alloc_mutex(
  void * clientData,
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){
#nếu SQLITE_THREADSAFE
  sqlite3_mutex *p = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
  char zBuf[100];
  sqlite3_mutex_free(p);
  sqlite3_snprintf(sizeof(zBuf), zBuf, "%p", p);
  Tcl_AppendResult(interp, zBuf, (char*)0);
#endif
  trả_về TCL_OK;
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
static số_nguyên SQLITE_TCLAPI test_config(
  void * clientData,
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){
  struct ConfigOption {
    const char *zName;
    số_nguyên iValue;
  } aOpt[] = {
    {"singlethread", SQLITE_CONFIG_SINGLETHREAD},
    {"multithread",  SQLITE_CONFIG_MULTITHREAD},
    {"serialized",   SQLITE_CONFIG_SERIALIZED},
    {0, 0}
  };
  số_nguyên s = sizeof(struct ConfigOption);
  số_nguyên i;
  số_nguyên rc;

  nếu( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    trả_về TCL_ERROR;
  }

  nếu( Tcl_GetIndexFromObjStruct(interp, objv[1], aOpt, s, "flag", 0, &i) ){
    nếu( Tcl_GetIntFromObj(interp, objv[1], &i) ){
      trả_về TCL_ERROR;
    }
  }không_thì{
    i = aOpt[i].iValue;
  }

  rc = sqlite3_config(i);
  Tcl_SetResult(interp, (char *)sqlite3ErrName(rc), TCL_VOLATILE);
  trả_về TCL_OK;
}

static sqlite3_mutex *getStaticMutexPointer(
  Tcl_Interp *pInterp,
  Tcl_Obj *pObj
){
  số_nguyên iMutex;
  nếu( Tcl_GetIndexFromObj(pInterp, pObj, aName, "mutex name", 0, &iMutex) ){
    trả_về 0;
  }
  assert( iMutex!=SQLITE_MUTEX_FAST && iMutex!=SQLITE_MUTEX_RECURSIVE );
  trả_về counterMutexAlloc(iMutex);
}

static số_nguyên SQLITE_TCLAPI test_enter_static_mutex(
  void * clientData,
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_mutex *pMutex;
  nếu( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "NAME");
    trả_về TCL_ERROR;
  }
  pMutex = getStaticMutexPointer(interp, objv[1]);
  nếu( !pMutex ){
    trả_về TCL_ERROR;
  }
  sqlite3_mutex_enter(pMutex);
  trả_về TCL_OK;
}

static số_nguyên SQLITE_TCLAPI test_leave_static_mutex(
  void * clientData,
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_mutex *pMutex;
  nếu( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "NAME");
    trả_về TCL_ERROR;
  }
  pMutex = getStaticMutexPointer(interp, objv[1]);
  nếu( !pMutex ){
    trả_về TCL_ERROR;
  }
  sqlite3_mutex_leave(pMutex);
  trả_về TCL_OK;
}

static số_nguyên SQLITE_TCLAPI test_enter_db_mutex(
  void * clientData,
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  nếu( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "DB");
    trả_về TCL_ERROR;
  }
  nếu( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ){
    trả_về TCL_ERROR;
  }
  nếu( !db ){
    trả_về TCL_ERROR;
  }
  sqlite3_mutex_enter(sqlite3_db_mutex(db));
  trả_về TCL_OK;
}

static số_nguyên SQLITE_TCLAPI test_leave_db_mutex(
  void * clientData,
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  nếu( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "DB");
    trả_về TCL_ERROR;
  }
  nếu( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ){
    trả_về TCL_ERROR;
  }
  nếu( !db ){
    trả_về TCL_ERROR;
  }
  sqlite3_mutex_leave(sqlite3_db_mutex(db));
  trả_về TCL_OK;
}

số_nguyên Sqlitetest_mutex_Init(Tcl_Interp *interp){
  static struct {
    char *zName;
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
  số_nguyên i;
  cho(i=0; i<sizeof(aCmd)/sizeof(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  Tcl_LinkVar(interp, "disable_mutex_init", 
              (char*)&g.disableInit, TCL_LINK_INT);
  Tcl_LinkVar(interp, "disable_mutex_try", 
              (char*)&g.disableTry, TCL_LINK_INT);
  trả_về SQLITE_OK;
}
