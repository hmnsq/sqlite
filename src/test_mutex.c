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
extern const char *sqlite3ErrName(цяло);
цяло getDbPointer(Tcl_Interp *interp, const char *zA, sqlite3 **ppDb);

static const char *aName[MAX_MUTEXES+1] = {
  "fast",        "recursive",   "static_main",   "static_mem",
  "static_open", "static_prng", "static_lru",    "static_pmem",
  "static_app1", "static_app2", "static_app3",   "static_vfs1",
  "static_vfs2", "static_vfs3", 0
};

/* A countable mutex */
struct sqlite3_mutex {
  sqlite3_mutex *pReal;
  цяло eType;
};

/* State variables */
static struct test_mutex_globals {
  цяло isInstalled;           /* True if installed */
  цяло disableInit;           /* True to cause sqlite3_initialize() to fail */
  цяло disableTry;            /* True to force sqlite3_mutex_try() to fail */
  цяло isInit;                /* True if initialized */
  sqlite3_mutex_methods m;   /* Interface to "real" mutex system */
  цяло aCounter[MAX_MUTEXES]; /* Number of grabs of each type of mutex */
  sqlite3_mutex aStatic[STATIC_MUTEXES]; /* The static mutexes */
} g = {0};

/* Return true if the countable mutex is currently held */
static цяло counterMutexHeld(sqlite3_mutex *p){
  върни g.m.xMutexHeld(p->pReal);
}

/* Return true if the countable mutex is not currently held */
static цяло counterMutexNotheld(sqlite3_mutex *p){
  върни g.m.xMutexNotheld(p->pReal);
}

/* Initialize the countable mutex interface
** Or, if g.disableInit is non-zero, then do not initialize but instead
** return the value of g.disableInit as the result code.  This can be used
** to simulate an initialization failure.
*/
static цяло counterMutexInit(void){ 
  цяло rc;
  ако( g.disableInit ) върни g.disableInit;
  rc = g.m.xMutexInit();
  g.isInit = 1;
  върни rc;
}

/*
** Uninitialize the mutex subsystem
*/
static цяло counterMutexEnd(void){ 
  g.isInit = 0;
  върни g.m.xMutexEnd();
}

/*
** Allocate a countable mutex
*/
static sqlite3_mutex *counterMutexAlloc(цяло eType){
  sqlite3_mutex *pReal;
  sqlite3_mutex *pRet = 0;

  assert( g.isInit );
  assert( eType>=SQLITE_MUTEX_FAST );
  assert( eType<=SQLITE_MUTEX_STATIC_VFS3 );

  pReal = g.m.xMutexAlloc(eType);
  ако( !pReal ) върни 0;

  ако( eType==SQLITE_MUTEX_FAST || eType==SQLITE_MUTEX_RECURSIVE ){
    pRet = (sqlite3_mutex *)malloc(sizeof(sqlite3_mutex));
  }иначе{
    цяло eStaticType = eType - (MAX_MUTEXES - STATIC_MUTEXES);
    assert( eStaticType>=0 );
    assert( eStaticType<STATIC_MUTEXES );
    pRet = &g.aStatic[eStaticType];
  }

  pRet->eType = eType;
  pRet->pReal = pReal;
  върни pRet;
}

/*
** Free a countable mutex
*/
static void counterMutexFree(sqlite3_mutex *p){
  assert( g.isInit );
  g.m.xMutexFree(p->pReal);
  ако( p->eType==SQLITE_MUTEX_FAST || p->eType==SQLITE_MUTEX_RECURSIVE ){
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
static цяло counterMutexTry(sqlite3_mutex *p){
  assert( g.isInit );
  assert( p->eType>=0 );
  assert( p->eType<MAX_MUTEXES );
  g.aCounter[p->eType]++;
  ако( g.disableTry ) върни SQLITE_BUSY;
  върни g.m.xMutexTry(p->pReal);
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
static цяло SQLITE_TCLAPI test_shutdown(
  void * clientData,
  Tcl_Interp *interp,
  цяло objc,
  Tcl_Obj *CONST objv[]
){
  цяло rc;

  ако( objc!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    върни TCL_ERROR;
  }

  rc = sqlite3_shutdown();
  Tcl_SetResult(interp, (char *)sqlite3ErrName(rc), TCL_VOLATILE);
  върни TCL_OK;
}

/*
** sqlite3_initialize
*/
static цяло SQLITE_TCLAPI test_initialize(
  void * clientData,
  Tcl_Interp *interp,
  цяло objc,
  Tcl_Obj *CONST objv[]
){
  цяло rc;

  ако( objc!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    върни TCL_ERROR;
  }

  rc = sqlite3_initialize();
  Tcl_SetResult(interp, (char *)sqlite3ErrName(rc), TCL_VOLATILE);
  върни TCL_OK;
}

/*
** install_mutex_counters BOOLEAN
*/
static цяло SQLITE_TCLAPI test_install_mutex_counters(
  void * clientData,
  Tcl_Interp *interp,
  цяло objc,
  Tcl_Obj *CONST objv[]
){
  цяло rc = SQLITE_OK;
  цяло isInstall;

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

  ако( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "BOOLEAN");
    върни TCL_ERROR;
  }
  ако( TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[1], &isInstall) ){
    върни TCL_ERROR;
  }

  assert(isInstall==0 || isInstall==1);
  assert(g.isInstalled==0 || g.isInstalled==1);
  ако( isInstall==g.isInstalled ){
    Tcl_AppendResult(interp, "mutex counters are ", NULL);
    Tcl_AppendResult(interp, isInstall?"already installed":"not installed", NULL);
    върни TCL_ERROR;
  }

  ако( isInstall ){
    assert( g.m.xMutexAlloc==0 );
    rc = sqlite3_config(SQLITE_CONFIG_GETMUTEX, &g.m);
    ако( rc==SQLITE_OK ){
      sqlite3_config(SQLITE_CONFIG_MUTEX, &counter_methods);
    }
    g.disableTry = 0;
  }иначе{
    assert( g.m.xMutexAlloc );
    rc = sqlite3_config(SQLITE_CONFIG_MUTEX, &g.m);
    memset(&g.m, 0, sizeof(sqlite3_mutex_methods));
  }

  ако( rc==SQLITE_OK ){
    g.isInstalled = isInstall;
  }

  Tcl_SetResult(interp, (char *)sqlite3ErrName(rc), TCL_VOLATILE);
  върни TCL_OK;
}

/*
** read_mutex_counters
*/
static цяло SQLITE_TCLAPI test_read_mutex_counters(
  void * clientData,
  Tcl_Interp *interp,
  цяло objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Obj *pRet;
  цяло ii;

  ако( objc!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    върни TCL_ERROR;
  }

  pRet = Tcl_NewObj();
  Tcl_IncrRefCount(pRet);
  за(ii=0; ii<MAX_MUTEXES; ii++){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj(aName[ii], -1));
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewIntObj(g.aCounter[ii]));
  }
  Tcl_SetObjResult(interp, pRet);
  Tcl_DecrRefCount(pRet);

  върни TCL_OK;
}

/*
** clear_mutex_counters
*/
static цяло SQLITE_TCLAPI test_clear_mutex_counters(
  void * clientData,
  Tcl_Interp *interp,
  цяло objc,
  Tcl_Obj *CONST objv[]
){
  цяло ii;

  ако( objc!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    върни TCL_ERROR;
  }

  за(ii=0; ii<MAX_MUTEXES; ii++){
    g.aCounter[ii] = 0;
  }
  върни TCL_OK;
}

/*
** Create and free a mutex.  Return the mutex pointer.  The pointer
** will be invalid since the mutex has already been freed.  The
** return pointer just checks to see if the mutex really was allocated.
*/
static цяло SQLITE_TCLAPI test_alloc_mutex(
  void * clientData,
  Tcl_Interp *interp,
  цяло objc,
  Tcl_Obj *CONST objv[]
){
#ако SQLITE_THREADSAFE
  sqlite3_mutex *p = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
  char zBuf[100];
  sqlite3_mutex_free(p);
  sqlite3_snprintf(sizeof(zBuf), zBuf, "%p", p);
  Tcl_AppendResult(interp, zBuf, (char*)0);
#endif
  върни TCL_OK;
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
static цяло SQLITE_TCLAPI test_config(
  void * clientData,
  Tcl_Interp *interp,
  цяло objc,
  Tcl_Obj *CONST objv[]
){
  struct ConfigOption {
    const char *zName;
    цяло iValue;
  } aOpt[] = {
    {"singlethread", SQLITE_CONFIG_SINGLETHREAD},
    {"multithread",  SQLITE_CONFIG_MULTITHREAD},
    {"serialized",   SQLITE_CONFIG_SERIALIZED},
    {0, 0}
  };
  цяло s = sizeof(struct ConfigOption);
  цяло i;
  цяло rc;

  ако( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    върни TCL_ERROR;
  }

  ако( Tcl_GetIndexFromObjStruct(interp, objv[1], aOpt, s, "flag", 0, &i) ){
    ако( Tcl_GetIntFromObj(interp, objv[1], &i) ){
      върни TCL_ERROR;
    }
  }иначе{
    i = aOpt[i].iValue;
  }

  rc = sqlite3_config(i);
  Tcl_SetResult(interp, (char *)sqlite3ErrName(rc), TCL_VOLATILE);
  върни TCL_OK;
}

static sqlite3_mutex *getStaticMutexPointer(
  Tcl_Interp *pInterp,
  Tcl_Obj *pObj
){
  цяло iMutex;
  ако( Tcl_GetIndexFromObj(pInterp, pObj, aName, "mutex name", 0, &iMutex) ){
    върни 0;
  }
  assert( iMutex!=SQLITE_MUTEX_FAST && iMutex!=SQLITE_MUTEX_RECURSIVE );
  върни counterMutexAlloc(iMutex);
}

static цяло SQLITE_TCLAPI test_enter_static_mutex(
  void * clientData,
  Tcl_Interp *interp,
  цяло objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_mutex *pMutex;
  ако( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "NAME");
    върни TCL_ERROR;
  }
  pMutex = getStaticMutexPointer(interp, objv[1]);
  ако( !pMutex ){
    върни TCL_ERROR;
  }
  sqlite3_mutex_enter(pMutex);
  върни TCL_OK;
}

static цяло SQLITE_TCLAPI test_leave_static_mutex(
  void * clientData,
  Tcl_Interp *interp,
  цяло objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_mutex *pMutex;
  ако( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "NAME");
    върни TCL_ERROR;
  }
  pMutex = getStaticMutexPointer(interp, objv[1]);
  ако( !pMutex ){
    върни TCL_ERROR;
  }
  sqlite3_mutex_leave(pMutex);
  върни TCL_OK;
}

static цяло SQLITE_TCLAPI test_enter_db_mutex(
  void * clientData,
  Tcl_Interp *interp,
  цяло objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  ако( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "DB");
    върни TCL_ERROR;
  }
  ако( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ){
    върни TCL_ERROR;
  }
  ако( !db ){
    върни TCL_ERROR;
  }
  sqlite3_mutex_enter(sqlite3_db_mutex(db));
  върни TCL_OK;
}

static цяло SQLITE_TCLAPI test_leave_db_mutex(
  void * clientData,
  Tcl_Interp *interp,
  цяло objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  ако( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "DB");
    върни TCL_ERROR;
  }
  ако( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ){
    върни TCL_ERROR;
  }
  ако( !db ){
    върни TCL_ERROR;
  }
  sqlite3_mutex_leave(sqlite3_db_mutex(db));
  върни TCL_OK;
}

цяло Sqlitetest_mutex_Init(Tcl_Interp *interp){
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
  цяло i;
  за(i=0; i<sizeof(aCmd)/sizeof(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  Tcl_LinkVar(interp, "disable_mutex_init", 
              (char*)&g.disableInit, TCL_LINK_INT);
  Tcl_LinkVar(interp, "disable_mutex_try", 
              (char*)&g.disableTry, TCL_LINK_INT);
  върни SQLITE_OK;
}
