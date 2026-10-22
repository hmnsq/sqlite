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
#ضمن "tclsqlite.h"
#ضمن "sqlite3.h"
#ضمن "sqliteInt.h"
#ضمن <stdlib.h>
#ضمن <assert.h>
#ضمن <string.h>

#define MAX_MUTEXES        (SQLITE_MUTEX_STATIC_VFS3+1)
#define STATIC_MUTEXES     (MAX_MUTEXES-(SQLITE_MUTEX_RECURSIVE+1))

/* defined in main.c */
خارجي ثابت حرف *sqlite3ErrName(صحيح);
صحيح getDbPointer(Tcl_Interp *interp, ثابت حرف *zA, sqlite3 **ppDb);

ساكن ثابت حرف *aName[MAX_MUTEXES+1] = {
  "fast",        "recursive",   "static_main",   "static_mem",
  "static_open", "static_prng", "static_lru",    "static_pmem",
  "static_app1", "static_app2", "static_app3",   "static_vfs1",
  "static_vfs2", "static_vfs3", 0
};

/* A countable mutex */
بنية sqlite3_mutex {
  sqlite3_mutex *pReal;
  صحيح eType;
};

/* State variables */
ساكن بنية test_mutex_globals {
  صحيح isInstalled;           /* True if installed */
  صحيح disableInit;           /* True to cause sqlite3_initialize() to fail */
  صحيح disableTry;            /* True to force sqlite3_mutex_try() to fail */
  صحيح isInit;                /* True if initialized */
  sqlite3_mutex_methods m;   /* Interface to "real" mutex system */
  صحيح aCounter[MAX_MUTEXES]; /* Number of grabs of each type of mutex */
  sqlite3_mutex aStatic[STATIC_MUTEXES]; /* The static mutexes */
} g = {0};

/* Return true if the countable mutex is currently held */
ساكن صحيح counterMutexHeld(sqlite3_mutex *p){
  أرجع g.m.xMutexHeld(p->pReal);
}

/* Return true if the countable mutex is not currently held */
ساكن صحيح counterMutexNotheld(sqlite3_mutex *p){
  أرجع g.m.xMutexNotheld(p->pReal);
}

/* Initialize the countable mutex interface
** Or, if g.disableInit is non-zero, then do not initialize but instead
** return the value of g.disableInit as the result code.  This can be used
** to simulate an initialization failure.
*/
ساكن صحيح counterMutexInit(فراغ){ 
  صحيح rc;
  إذا( g.disableInit ) أرجع g.disableInit;
  rc = g.m.xMutexInit();
  g.isInit = 1;
  أرجع rc;
}

/*
** Uninitialize the mutex subsystem
*/
ساكن صحيح counterMutexEnd(فراغ){ 
  g.isInit = 0;
  أرجع g.m.xMutexEnd();
}

/*
** Allocate a countable mutex
*/
ساكن sqlite3_mutex *counterMutexAlloc(صحيح eType){
  sqlite3_mutex *pReal;
  sqlite3_mutex *pRet = 0;

  أكد( g.isInit );
  أكد( eType>=SQLITE_MUTEX_FAST );
  أكد( eType<=SQLITE_MUTEX_STATIC_VFS3 );

  pReal = g.m.xMutexAlloc(eType);
  إذا( !pReal ) أرجع 0;

  إذا( eType==SQLITE_MUTEX_FAST || eType==SQLITE_MUTEX_RECURSIVE ){
    pRet = (sqlite3_mutex *)malloc(حجم(sqlite3_mutex));
  }وإلا{
    صحيح eStaticType = eType - (MAX_MUTEXES - STATIC_MUTEXES);
    أكد( eStaticType>=0 );
    أكد( eStaticType<STATIC_MUTEXES );
    pRet = &g.aStatic[eStaticType];
  }

  pRet->eType = eType;
  pRet->pReal = pReal;
  أرجع pRet;
}

/*
** Free a countable mutex
*/
ساكن فراغ counterMutexFree(sqlite3_mutex *p){
  أكد( g.isInit );
  g.m.xMutexFree(p->pReal);
  إذا( p->eType==SQLITE_MUTEX_FAST || p->eType==SQLITE_MUTEX_RECURSIVE ){
    حرر(p);
  }
}

/*
** Enter a countable mutex.  Block until entry is safe.
*/
ساكن فراغ counterMutexEnter(sqlite3_mutex *p){
  أكد( g.isInit );
  أكد( p->eType>=0 );
  أكد( p->eType<MAX_MUTEXES );
  g.aCounter[p->eType]++;
  g.m.xMutexEnter(p->pReal);
}

/*
** Try to enter a mutex.  Return true on success.
*/
ساكن صحيح counterMutexTry(sqlite3_mutex *p){
  أكد( g.isInit );
  أكد( p->eType>=0 );
  أكد( p->eType<MAX_MUTEXES );
  g.aCounter[p->eType]++;
  إذا( g.disableTry ) أرجع SQLITE_BUSY;
  أرجع g.m.xMutexTry(p->pReal);
}

/* Leave a mutex
*/
ساكن فراغ counterMutexLeave(sqlite3_mutex *p){
  أكد( g.isInit );
  g.m.xMutexLeave(p->pReal);
}

/*
** sqlite3_shutdown
*/
ساكن صحيح SQLITE_TCLAPI test_shutdown(
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح rc;

  إذا( objc!=1 ){
    Tcl_Wrong_رەقەم_Args(interp, 1, objv, "");
    أرجع TCL_ERROR;
  }

  rc = sqlite3_shutdown();
  Tcl_SetResult(interp, (حرف *)sqlite3ErrName(rc), TCL_VOLATILE);
  أرجع TCL_OK;
}

/*
** sqlite3_initialize
*/
ساكن صحيح SQLITE_TCLAPI test_initialize(
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح rc;

  إذا( objc!=1 ){
    Tcl_Wrong_رەقەم_Args(interp, 1, objv, "");
    أرجع TCL_ERROR;
  }

  rc = sqlite3_initialize();
  Tcl_SetResult(interp, (حرف *)sqlite3ErrName(rc), TCL_VOLATILE);
  أرجع TCL_OK;
}

/*
** install_mutex_counters BOOLEAN
*/
ساكن صحيح SQLITE_TCLAPI test_install_mutex_counters(
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح rc = SQLITE_OK;
  صحيح isInstall;

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

  إذا( objc!=2 ){
    Tcl_Wrong_رەقەم_Args(interp, 1, objv, "BOOLEAN");
    أرجع TCL_ERROR;
  }
  إذا( TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[1], &isInstall) ){
    أرجع TCL_ERROR;
  }

  أكد(isInstall==0 || isInstall==1);
  أكد(g.isInstalled==0 || g.isInstalled==1);
  إذا( isInstall==g.isInstalled ){
    Tcl_AppendResult(interp, "mutex counters are ", عدم);
    Tcl_AppendResult(interp, isInstall?"already installed":"not installed", عدم);
    أرجع TCL_ERROR;
  }

  إذا( isInstall ){
    أكد( g.m.xMutexAlloc==0 );
    rc = sqlite3_config(SQLITE_CONFIG_GETMUTEX, &g.m);
    إذا( rc==SQLITE_OK ){
      sqlite3_config(SQLITE_CONFIG_MUTEX, &counter_methods);
    }
    g.disableTry = 0;
  }وإلا{
    أكد( g.m.xMutexAlloc );
    rc = sqlite3_config(SQLITE_CONFIG_MUTEX, &g.m);
    memset(&g.m, 0, حجم(sqlite3_mutex_methods));
  }

  إذا( rc==SQLITE_OK ){
    g.isInstalled = isInstall;
  }

  Tcl_SetResult(interp, (حرف *)sqlite3ErrName(rc), TCL_VOLATILE);
  أرجع TCL_OK;
}

/*
** read_mutex_counters
*/
ساكن صحيح SQLITE_TCLAPI test_read_mutex_counters(
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Obj *pRet;
  صحيح ii;

  إذا( objc!=1 ){
    Tcl_Wrong_رەقەم_Args(interp, 1, objv, "");
    أرجع TCL_ERROR;
  }

  pRet = Tcl_NewObj();
  Tcl_Incr_Ref_سانى(pRet);
  لكل(ii=0; ii<MAX_MUTEXES; ii++){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj(aName[ii], -1));
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewIntObj(g.aCounter[ii]));
  }
  Tcl_SetObjResult(interp, pRet);
  Tcl_Decr_Ref_سانى(pRet);

  أرجع TCL_OK;
}

/*
** clear_mutex_counters
*/
ساكن صحيح SQLITE_TCLAPI test_clear_mutex_counters(
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح ii;

  إذا( objc!=1 ){
    Tcl_Wrong_رەقەم_Args(interp, 1, objv, "");
    أرجع TCL_ERROR;
  }

  لكل(ii=0; ii<MAX_MUTEXES; ii++){
    g.aCounter[ii] = 0;
  }
  أرجع TCL_OK;
}

/*
** Create and free a mutex.  Return the mutex pointer.  The pointer
** will be invalid since the mutex has already been freed.  The
** return pointer just checks to see if the mutex really was allocated.
*/
ساكن صحيح SQLITE_TCLAPI test_alloc_mutex(
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
#إذا SQLITE_THREADSAFE
  sqlite3_mutex *p = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
  حرف zBuf[100];
  sqlite3_mutex_free(p);
  sqlite3_snprintf(حجم(zBuf), zBuf, "%p", p);
  Tcl_AppendResult(interp, zBuf, (حرف*)0);
#endif
  أرجع TCL_OK;
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
ساكن صحيح SQLITE_TCLAPI test_config(
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  بنية ConfigOption {
    ثابت حرف *zName;
    صحيح i_قىممەت;
  } aOpt[] = {
    {"singlethread", SQLITE_CONFIG_SINGLETHREAD},
    {"multithread",  SQLITE_CONFIG_MULTITHREAD},
    {"serialized",   SQLITE_CONFIG_SERIALIZED},
    {0, 0}
  };
  صحيح s = حجم(بنية ConfigOption);
  صحيح i;
  صحيح rc;

  إذا( objc!=2 ){
    Tcl_Wrong_رەقەم_Args(interp, 1, objv, "");
    أرجع TCL_ERROR;
  }

  إذا( Tcl_GetIndexFromObjStruct(interp, objv[1], aOpt, s, "flag", 0, &i) ){
    إذا( Tcl_GetIntFromObj(interp, objv[1], &i) ){
      أرجع TCL_ERROR;
    }
  }وإلا{
    i = aOpt[i].i_قىممەت;
  }

  rc = sqlite3_config(i);
  Tcl_SetResult(interp, (حرف *)sqlite3ErrName(rc), TCL_VOLATILE);
  أرجع TCL_OK;
}

ساكن sqlite3_mutex *getStaticMutexPointer(
  Tcl_Interp *pInterp,
  Tcl_Obj *pObj
){
  صحيح iMutex;
  إذا( Tcl_GetIndexFromObj(pInterp, pObj, aName, "mutex name", 0, &iMutex) ){
    أرجع 0;
  }
  أكد( iMutex!=SQLITE_MUTEX_FAST && iMutex!=SQLITE_MUTEX_RECURSIVE );
  أرجع counterMutexAlloc(iMutex);
}

ساكن صحيح SQLITE_TCLAPI test_enter_static_mutex(
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_mutex *pMutex;
  إذا( objc!=2 ){
    Tcl_Wrong_رەقەم_Args(interp, 1, objv, "NAME");
    أرجع TCL_ERROR;
  }
  pMutex = getStaticMutexPointer(interp, objv[1]);
  إذا( !pMutex ){
    أرجع TCL_ERROR;
  }
  sqlite3_mutex_enter(pMutex);
  أرجع TCL_OK;
}

ساكن صحيح SQLITE_TCLAPI test_leave_static_mutex(
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_mutex *pMutex;
  إذا( objc!=2 ){
    Tcl_Wrong_رەقەم_Args(interp, 1, objv, "NAME");
    أرجع TCL_ERROR;
  }
  pMutex = getStaticMutexPointer(interp, objv[1]);
  إذا( !pMutex ){
    أرجع TCL_ERROR;
  }
  sqlite3_mutex_leave(pMutex);
  أرجع TCL_OK;
}

ساكن صحيح SQLITE_TCLAPI test_enter_db_mutex(
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  إذا( objc!=2 ){
    Tcl_Wrong_رەقەم_Args(interp, 1, objv, "DB");
    أرجع TCL_ERROR;
  }
  إذا( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ){
    أرجع TCL_ERROR;
  }
  إذا( !db ){
    أرجع TCL_ERROR;
  }
  sqlite3_mutex_enter(sqlite3_db_mutex(db));
  أرجع TCL_OK;
}

ساكن صحيح SQLITE_TCLAPI test_leave_db_mutex(
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  إذا( objc!=2 ){
    Tcl_Wrong_رەقەم_Args(interp, 1, objv, "DB");
    أرجع TCL_ERROR;
  }
  إذا( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ){
    أرجع TCL_ERROR;
  }
  إذا( !db ){
    أرجع TCL_ERROR;
  }
  sqlite3_mutex_leave(sqlite3_db_mutex(db));
  أرجع TCL_OK;
}

صحيح Sqlitetest_mutex_Init(Tcl_Interp *interp){
  ساكن بنية {
    حرف *zName;
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
  صحيح i;
  لكل(i=0; i<حجم(aCmd)/حجم(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  Tcl_LinkVar(interp, "disable_mutex_init", 
              (حرف*)&g.disableInit, TCL_LINK_INT);
  Tcl_LinkVar(interp, "disable_mutex_try", 
              (حرف*)&g.disableTry, TCL_LINK_INT);
  أرجع SQLITE_OK;
}
