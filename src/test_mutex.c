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
#شامل "tclsqlite.h"
#شامل "sqlite3.h"
#شامل "sqliteInt.h"
#شامل <stdlib.h>
#شامل <assert.h>
#شامل <string.h>

#define MAX_MUTEXES        (SQLITE_MUTEX_STATIC_VFS3+1)
#define STATIC_MUTEXES     (MAX_MUTEXES-(SQLITE_MUTEX_RECURSIVE+1))

/* defined in main.c */
بیرونی ثابت نویسه *sqlite3ErrName(صحیح);
صحیح getDbPointer(Tcl_Interp *interp, ثابت نویسه *zA, sqlite3 **ppDb);

ایستا ثابت نویسه *aName[MAX_MUTEXES+1] = {
  "fast",        "recursive",   "static_main",   "static_mem",
  "static_open", "static_prng", "static_lru",    "static_pmem",
  "static_app1", "static_app2", "static_app3",   "static_vfs1",
  "static_vfs2", "static_vfs3", 0
};

/* A countable mutex */
ساختار sqlite3_mutex {
  sqlite3_mutex *pReal;
  صحیح eType;
};

/* State variables */
ایستا ساختار test_mutex_globals {
  صحیح isInstalled;           /* True if installed */
  صحیح disableInit;           /* True to cause sqlite3_initialize() to fail */
  صحیح disableTry;            /* True to force sqlite3_mutex_try() to fail */
  صحیح isInit;                /* True if initialized */
  sqlite3_mutex_methods m;   /* Interface to "real" mutex system */
  صحیح aCounter[MAX_MUTEXES]; /* Number of grabs of each type of mutex */
  sqlite3_mutex aStatic[STATIC_MUTEXES]; /* The static mutexes */
} g = {0};

/* Return true if the countable mutex is currently held */
ایستا صحیح counterMutexHeld(sqlite3_mutex *p){
  بازگشت g.m.xMutexHeld(p->pReal);
}

/* Return true if the countable mutex is not currently held */
ایستا صحیح counterMutexNotheld(sqlite3_mutex *p){
  بازگشت g.m.xMutexNotheld(p->pReal);
}

/* Initialize the countable mutex interface
** Or, if g.disableInit is non-zero, then do not initialize but instead
** return the value of g.disableInit as the result code.  This can be used
** to simulate an initialization failure.
*/
ایستا صحیح counterMutexInit(تهی){ 
  صحیح rc;
  اگر( g.disableInit ) بازگشت g.disableInit;
  rc = g.m.xMutexInit();
  g.isInit = 1;
  بازگشت rc;
}

/*
** Uninitialize the mutex subsystem
*/
ایستا صحیح counterMutexEnd(تهی){ 
  g.isInit = 0;
  بازگشت g.m.xMutexEnd();
}

/*
** Allocate a countable mutex
*/
ایستا sqlite3_mutex *counterMutexAlloc(صحیح eType){
  sqlite3_mutex *pReal;
  sqlite3_mutex *pRet = 0;

  أكد( g.isInit );
  أكد( eType>=SQLITE_MUTEX_FAST );
  أكد( eType<=SQLITE_MUTEX_STATIC_VFS3 );

  pReal = g.m.xMutexAlloc(eType);
  اگر( !pReal ) بازگشت 0;

  اگر( eType==SQLITE_MUTEX_FAST || eType==SQLITE_MUTEX_RECURSIVE ){
    pRet = (sqlite3_mutex *)malloc(اندازه(sqlite3_mutex));
  }وگرنه{
    صحیح eStaticType = eType - (MAX_MUTEXES - STATIC_MUTEXES);
    أكد( eStaticType>=0 );
    أكد( eStaticType<STATIC_MUTEXES );
    pRet = &g.aStatic[eStaticType];
  }

  pRet->eType = eType;
  pRet->pReal = pReal;
  بازگشت pRet;
}

/*
** Free a countable mutex
*/
ایستا تهی counterMutexFree(sqlite3_mutex *p){
  أكد( g.isInit );
  g.m.xMutexFree(p->pReal);
  اگر( p->eType==SQLITE_MUTEX_FAST || p->eType==SQLITE_MUTEX_RECURSIVE ){
    آزاد(p);
  }
}

/*
** Enter a countable mutex.  Block until entry is safe.
*/
ایستا تهی counterMutexEnter(sqlite3_mutex *p){
  أكد( g.isInit );
  أكد( p->eType>=0 );
  أكد( p->eType<MAX_MUTEXES );
  g.aCounter[p->eType]++;
  g.m.xMutexEnter(p->pReal);
}

/*
** Try to enter a mutex.  Return true on success.
*/
ایستا صحیح counterMutexTry(sqlite3_mutex *p){
  أكد( g.isInit );
  أكد( p->eType>=0 );
  أكد( p->eType<MAX_MUTEXES );
  g.aCounter[p->eType]++;
  اگر( g.disableTry ) بازگشت SQLITE_BUSY;
  بازگشت g.m.xMutexTry(p->pReal);
}

/* Leave a mutex
*/
ایستا تهی counterMutexLeave(sqlite3_mutex *p){
  أكد( g.isInit );
  g.m.xMutexLeave(p->pReal);
}

/*
** sqlite3_shutdown
*/
ایستا صحیح SQLITE_TCLAPI test_shutdown(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح rc;

  اگر( objc!=1 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "");
    بازگشت TCL_ERROR;
  }

  rc = sqlite3_shutdown();
  Tcl_SetResult(interp, (نویسه *)sqlite3ErrName(rc), TCL_VOLATILE);
  بازگشت TCL_OK;
}

/*
** sqlite3_initialize
*/
ایستا صحیح SQLITE_TCLAPI test_initialize(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح rc;

  اگر( objc!=1 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "");
    بازگشت TCL_ERROR;
  }

  rc = sqlite3_initialize();
  Tcl_SetResult(interp, (نویسه *)sqlite3ErrName(rc), TCL_VOLATILE);
  بازگشت TCL_OK;
}

/*
** install_mutex_counters BOOLEAN
*/
ایستا صحیح SQLITE_TCLAPI test_install_mutex_counters(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح rc = SQLITE_OK;
  صحیح isInstall;

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

  اگر( objc!=2 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "BOOLEAN");
    بازگشت TCL_ERROR;
  }
  اگر( TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[1], &isInstall) ){
    بازگشت TCL_ERROR;
  }

  أكد(isInstall==0 || isInstall==1);
  أكد(g.isInstalled==0 || g.isInstalled==1);
  اگر( isInstall==g.isInstalled ){
    Tcl_AppendResult(interp, "mutex counters are ", پوچ);
    Tcl_AppendResult(interp, isInstall?"already installed":"not installed", پوچ);
    بازگشت TCL_ERROR;
  }

  اگر( isInstall ){
    أكد( g.m.xMutexAlloc==0 );
    rc = sqlite3_config(SQLITE_CONFIG_GETMUTEX, &g.m);
    اگر( rc==SQLITE_OK ){
      sqlite3_config(SQLITE_CONFIG_MUTEX, &counter_methods);
    }
    g.disableTry = 0;
  }وگرنه{
    أكد( g.m.xMutexAlloc );
    rc = sqlite3_config(SQLITE_CONFIG_MUTEX, &g.m);
    memset(&g.m, 0, اندازه(sqlite3_mutex_methods));
  }

  اگر( rc==SQLITE_OK ){
    g.isInstalled = isInstall;
  }

  Tcl_SetResult(interp, (نویسه *)sqlite3ErrName(rc), TCL_VOLATILE);
  بازگشت TCL_OK;
}

/*
** read_mutex_counters
*/
ایستا صحیح SQLITE_TCLAPI test_read_mutex_counters(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Obj *pRet;
  صحیح ii;

  اگر( objc!=1 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "");
    بازگشت TCL_ERROR;
  }

  pRet = Tcl_NewObj();
  Tcl_Incr_Ref_شمار(pRet);
  برای(ii=0; ii<MAX_MUTEXES; ii++){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj(aName[ii], -1));
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewIntObj(g.aCounter[ii]));
  }
  Tcl_SetObjResult(interp, pRet);
  Tcl_Decr_Ref_شمار(pRet);

  بازگشت TCL_OK;
}

/*
** clear_mutex_counters
*/
ایستا صحیح SQLITE_TCLAPI test_clear_mutex_counters(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح ii;

  اگر( objc!=1 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "");
    بازگشت TCL_ERROR;
  }

  برای(ii=0; ii<MAX_MUTEXES; ii++){
    g.aCounter[ii] = 0;
  }
  بازگشت TCL_OK;
}

/*
** Create and free a mutex.  Return the mutex pointer.  The pointer
** will be invalid since the mutex has already been freed.  The
** return pointer just checks to see if the mutex really was allocated.
*/
ایستا صحیح SQLITE_TCLAPI test_alloc_mutex(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
#اگر SQLITE_THREADSAFE
  sqlite3_mutex *p = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
  نویسه zBuf[100];
  sqlite3_mutex_free(p);
  sqlite3_snprintf(اندازه(zBuf), zBuf, "%p", p);
  Tcl_AppendResult(interp, zBuf, (نویسه*)0);
#endif
  بازگشت TCL_OK;
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
ایستا صحیح SQLITE_TCLAPI test_config(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  ساختار ConfigOption {
    ثابت نویسه *zName;
    صحیح i_مقدار;
  } aOpt[] = {
    {"singlethread", SQLITE_CONFIG_SINGLETHREAD},
    {"multithread",  SQLITE_CONFIG_MULTITHREAD},
    {"serialized",   SQLITE_CONFIG_SERIALIZED},
    {0, 0}
  };
  صحیح s = اندازه(ساختار ConfigOption);
  صحیح i;
  صحیح rc;

  اگر( objc!=2 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "");
    بازگشت TCL_ERROR;
  }

  اگر( Tcl_GetIndexFromObjStruct(interp, objv[1], aOpt, s, "flag", 0, &i) ){
    اگر( Tcl_GetIntFromObj(interp, objv[1], &i) ){
      بازگشت TCL_ERROR;
    }
  }وگرنه{
    i = aOpt[i].i_مقدار;
  }

  rc = sqlite3_config(i);
  Tcl_SetResult(interp, (نویسه *)sqlite3ErrName(rc), TCL_VOLATILE);
  بازگشت TCL_OK;
}

ایستا sqlite3_mutex *getStaticMutexPointer(
  Tcl_Interp *pInterp,
  Tcl_Obj *pObj
){
  صحیح iMutex;
  اگر( Tcl_GetIndexFromObj(pInterp, pObj, aName, "mutex name", 0, &iMutex) ){
    بازگشت 0;
  }
  أكد( iMutex!=SQLITE_MUTEX_FAST && iMutex!=SQLITE_MUTEX_RECURSIVE );
  بازگشت counterMutexAlloc(iMutex);
}

ایستا صحیح SQLITE_TCLAPI test_enter_static_mutex(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_mutex *pMutex;
  اگر( objc!=2 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "NAME");
    بازگشت TCL_ERROR;
  }
  pMutex = getStaticMutexPointer(interp, objv[1]);
  اگر( !pMutex ){
    بازگشت TCL_ERROR;
  }
  sqlite3_mutex_enter(pMutex);
  بازگشت TCL_OK;
}

ایستا صحیح SQLITE_TCLAPI test_leave_static_mutex(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_mutex *pMutex;
  اگر( objc!=2 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "NAME");
    بازگشت TCL_ERROR;
  }
  pMutex = getStaticMutexPointer(interp, objv[1]);
  اگر( !pMutex ){
    بازگشت TCL_ERROR;
  }
  sqlite3_mutex_leave(pMutex);
  بازگشت TCL_OK;
}

ایستا صحیح SQLITE_TCLAPI test_enter_db_mutex(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  اگر( objc!=2 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "DB");
    بازگشت TCL_ERROR;
  }
  اگر( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ){
    بازگشت TCL_ERROR;
  }
  اگر( !db ){
    بازگشت TCL_ERROR;
  }
  sqlite3_mutex_enter(sqlite3_db_mutex(db));
  بازگشت TCL_OK;
}

ایستا صحیح SQLITE_TCLAPI test_leave_db_mutex(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  اگر( objc!=2 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "DB");
    بازگشت TCL_ERROR;
  }
  اگر( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ){
    بازگشت TCL_ERROR;
  }
  اگر( !db ){
    بازگشت TCL_ERROR;
  }
  sqlite3_mutex_leave(sqlite3_db_mutex(db));
  بازگشت TCL_OK;
}

صحیح Sqlitetest_mutex_Init(Tcl_Interp *interp){
  ایستا ساختار {
    نویسه *zName;
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
  صحیح i;
  برای(i=0; i<اندازه(aCmd)/اندازه(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  Tcl_LinkVar(interp, "disable_mutex_init", 
              (نویسه*)&g.disableInit, TCL_LINK_INT);
  Tcl_LinkVar(interp, "disable_mutex_try", 
              (نویسه*)&g.disableTry, TCL_LINK_INT);
  بازگشت SQLITE_OK;
}
