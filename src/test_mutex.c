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

#تعریف_کرو MAX_MUTEXES        (SQLITE_MUTEX_STATIC_VFS3+1)
#تعریف_کرو STATIC_MUTEXES     (MAX_MUTEXES-(SQLITE_MUTEX_RECURSIVE+1))

/* defined in main.c */
خارجی ثابت حرف *sqlite3ErrName(صحیح);
صحیح get_Db_اشارہ_گر(Tcl_Interp *interp, ثابت حرف *zA, sqlite3 **ppDb);

ساکن ثابت حرف *aName[MAX_MUTEXES+1] = {
  "fast",        "recursive",   "static_main",   "static_mem",
  "static_open", "static_prng", "static_lru",    "static_pmem",
  "static_app1", "static_app2", "static_app3",   "static_vfs1",
  "static_vfs2", "static_vfs3", 0
};

/* A countable mutex */
ساخت sqlite3_mutex {
  sqlite3_mutex *pReal;
  صحیح eType;
};

/* State variables */
ساکن ساخت test_mutex_globals {
  صحیح isInstalled;           /* True if installed */
  صحیح disableInit;           /* True to cause sqlite3_initialize() to fail */
  صحیح disableTry;            /* True to force sqlite3_mutex_try() to fail */
  صحیح isInit;                /* True if initialized */
  sqlite3_mutex_methods m;   /* Interface to "real" mutex system */
  صحیح a_شمار[MAX_MUTEXES]; /* Number of grabs of each type of mutex */
  sqlite3_mutex aStatic[STATIC_MUTEXES]; /* The static mutexes */
} g = {0};

/* Return true if the countable mutex is currently held */
ساکن صحیح شمار_Mutex_Held(sqlite3_mutex *p){
  واپس g.m.xMutexHeld(p->pReal);
}

/* Return true if the countable mutex is not currently held */
ساکن صحیح شمار_Mutex_Notheld(sqlite3_mutex *p){
  واپس g.m.xMutexNotheld(p->pReal);
}

/* Initialize the countable mutex interface
** Or, if g.disableInit is non-zero, then do not initialize but instead
** return the value of g.disableInit as the result code.  This can be used
** to simulate an initialization failure.
*/
ساکن صحیح شمار_Mutex_Init(خالی){ 
  صحیح rc;
  اگر( g.disableInit ) واپس g.disableInit;
  rc = g.m.xMutexInit();
  g.isInit = 1;
  واپس rc;
}

/*
** Uninitialize the mutex subsystem
*/
ساکن صحیح شمار_Mutex_انتہا(خالی){ 
  g.isInit = 0;
  واپس g.m.x_Mutex_انتہا();
}

/*
** Allocate a countable mutex
*/
ساکن sqlite3_mutex *شمار_Mutex_Alloc(صحیح eType){
  sqlite3_mutex *pReal;
  sqlite3_mutex *pRet = 0;

  أكد( g.isInit );
  أكد( eType>=SQLITE_MUTEX_FAST );
  أكد( eType<=SQLITE_MUTEX_STATIC_VFS3 );

  pReal = g.m.xMutexAlloc(eType);
  اگر( !pReal ) واپس 0;

  اگر( eType==SQLITE_MUTEX_FAST || eType==SQLITE_MUTEX_RECURSIVE ){
    pRet = (sqlite3_mutex *)حافظہ_دو(حجم(sqlite3_mutex));
  }ورنہ{
    صحیح eStaticType = eType - (MAX_MUTEXES - STATIC_MUTEXES);
    أكد( eStaticType>=0 );
    أكد( eStaticType<STATIC_MUTEXES );
    pRet = &g.aStatic[eStaticType];
  }

  pRet->eType = eType;
  pRet->pReal = pReal;
  واپس pRet;
}

/*
** Free a countable mutex
*/
ساکن خالی شمار_Mutex_Free(sqlite3_mutex *p){
  أكد( g.isInit );
  g.m.xMutexFree(p->pReal);
  اگر( p->eType==SQLITE_MUTEX_FAST || p->eType==SQLITE_MUTEX_RECURSIVE ){
    آزاد(p);
  }
}

/*
** Enter a countable mutex.  Block until entry is safe.
*/
ساکن خالی شمار_Mutex_Enter(sqlite3_mutex *p){
  أكد( g.isInit );
  أكد( p->eType>=0 );
  أكد( p->eType<MAX_MUTEXES );
  g.a_شمار[p->eType]++;
  g.m.xMutexEnter(p->pReal);
}

/*
** Try to enter a mutex.  Return true on success.
*/
ساکن صحیح شمار_Mutex_Try(sqlite3_mutex *p){
  أكد( g.isInit );
  أكد( p->eType>=0 );
  أكد( p->eType<MAX_MUTEXES );
  g.a_شمار[p->eType]++;
  اگر( g.disableTry ) واپس SQLITE_BUSY;
  واپس g.m.xMutexTry(p->pReal);
}

/* Leave a mutex
*/
ساکن خالی شمار_Mutex_Leave(sqlite3_mutex *p){
  أكد( g.isInit );
  g.m.xMutexLeave(p->pReal);
}

/*
** sqlite3_shutdown
*/
ساکن صحیح SQLITE_TCLAPI test_shutdown(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح rc;

  اگر( objc!=1 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "");
    واپس TCL_ERROR;
  }

  rc = sqlite3_shutdown();
  Tcl_سیٹ_نتیجہ(interp, (حرف *)sqlite3ErrName(rc), TCL_VOLATILE);
  واپس TCL_OK;
}

/*
** sqlite3_initialize
*/
ساکن صحیح SQLITE_TCLAPI test_initialize(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح rc;

  اگر( objc!=1 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "");
    واپس TCL_ERROR;
  }

  rc = sqlite3_initialize();
  Tcl_سیٹ_نتیجہ(interp, (حرف *)sqlite3ErrName(rc), TCL_VOLATILE);
  واپس TCL_OK;
}

/*
** install_mutex_counters BOOLEAN
*/
ساکن صحیح SQLITE_TCLAPI test_install_mutex_counters(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح rc = SQLITE_OK;
  صحیح isInstall;

  sqlite3_mutex_methods شمار_methods = {
    شمار_Mutex_Init,
    شمار_Mutex_انتہا,
    شمار_Mutex_Alloc,
    شمار_Mutex_Free,
    شمار_Mutex_Enter,
    شمار_Mutex_Try,
    شمار_Mutex_Leave,
    شمار_Mutex_Held,
    شمار_Mutex_Notheld
  };

  اگر( objc!=2 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "BOOLEAN");
    واپس TCL_ERROR;
  }
  اگر( TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[1], &isInstall) ){
    واپس TCL_ERROR;
  }

  أكد(isInstall==0 || isInstall==1);
  أكد(g.isInstalled==0 || g.isInstalled==1);
  اگر( isInstall==g.isInstalled ){
    Tcl_Append_نتیجہ(interp, "mutex counters are ", تہی);
    Tcl_Append_نتیجہ(interp, isInstall?"already installed":"not installed", تہی);
    واپس TCL_ERROR;
  }

  اگر( isInstall ){
    أكد( g.m.xMutexAlloc==0 );
    rc = sqlite3_config(SQLITE_CONFIG_GETMUTEX, &g.m);
    اگر( rc==SQLITE_OK ){
      sqlite3_config(SQLITE_CONFIG_MUTEX, &شمار_methods);
    }
    g.disableTry = 0;
  }ورنہ{
    أكد( g.m.xMutexAlloc );
    rc = sqlite3_config(SQLITE_CONFIG_MUTEX, &g.m);
    memset(&g.m, 0, حجم(sqlite3_mutex_methods));
  }

  اگر( rc==SQLITE_OK ){
    g.isInstalled = isInstall;
  }

  Tcl_سیٹ_نتیجہ(interp, (حرف *)sqlite3ErrName(rc), TCL_VOLATILE);
  واپس TCL_OK;
}

/*
** read_mutex_counters
*/
ساکن صحیح SQLITE_TCLAPI test_read_mutex_counters(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Obj *pRet;
  صحیح ii;

  اگر( objc!=1 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "");
    واپس TCL_ERROR;
  }

  pRet = Tcl_NewObj();
  Tcl_Incr_Ref_گنتی(pRet);
  برائے(ii=0; ii<MAX_MUTEXES; ii++){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj(aName[ii], -1));
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewIntObj(g.a_شمار[ii]));
  }
  Tcl_سیٹ_Obj_نتیجہ(interp, pRet);
  Tcl_Decr_Ref_گنتی(pRet);

  واپس TCL_OK;
}

/*
** clear_mutex_counters
*/
ساکن صحیح SQLITE_TCLAPI test_clear_mutex_counters(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح ii;

  اگر( objc!=1 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "");
    واپس TCL_ERROR;
  }

  برائے(ii=0; ii<MAX_MUTEXES; ii++){
    g.a_شمار[ii] = 0;
  }
  واپس TCL_OK;
}

/*
** Create and free a mutex.  Return the mutex pointer.  The pointer
** will be invalid since the mutex has already been freed.  The
** return pointer just checks to see if the mutex really was allocated.
*/
ساکن صحیح SQLITE_TCLAPI test_alloc_mutex(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
#اگر SQLITE_THREADSAFE
  sqlite3_mutex *p = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
  حرف zBuf[100];
  sqlite3_mutex_free(p);
  sqlite3_snprintf(حجم(zBuf), zBuf, "%p", p);
  Tcl_Append_نتیجہ(interp, zBuf, (حرف*)0);
#اگر_ختم
  واپس TCL_OK;
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
ساکن صحیح SQLITE_TCLAPI test_config(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  ساخت ConfigOption {
    ثابت حرف *zName;
    صحیح i_قدر;
  } aOpt[] = {
    {"singlethread", SQLITE_CONFIG_SINGLETHREAD},
    {"multithread",  SQLITE_CONFIG_MULTITHREAD},
    {"serialized",   SQLITE_CONFIG_SERIALIZED},
    {0, 0}
  };
  صحیح s = حجم(ساخت ConfigOption);
  صحیح i;
  صحیح rc;

  اگر( objc!=2 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "");
    واپس TCL_ERROR;
  }

  اگر( Tcl_Get_اشاریہ_From_Obj_Struct(interp, objv[1], aOpt, s, "flag", 0, &i) ){
    اگر( Tcl_GetIntFromObj(interp, objv[1], &i) ){
      واپس TCL_ERROR;
    }
  }ورنہ{
    i = aOpt[i].i_قدر;
  }

  rc = sqlite3_config(i);
  Tcl_سیٹ_نتیجہ(interp, (حرف *)sqlite3ErrName(rc), TCL_VOLATILE);
  واپس TCL_OK;
}

ساکن sqlite3_mutex *get_Static_Mutex_اشارہ_گر(
  Tcl_Interp *pInterp,
  Tcl_Obj *pObj
){
  صحیح iMutex;
  اگر( Tcl_Get_اشاریہ_From_Obj(pInterp, pObj, aName, "mutex name", 0, &iMutex) ){
    واپس 0;
  }
  أكد( iMutex!=SQLITE_MUTEX_FAST && iMutex!=SQLITE_MUTEX_RECURSIVE );
  واپس شمار_Mutex_Alloc(iMutex);
}

ساکن صحیح SQLITE_TCLAPI test_enter_static_mutex(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_mutex *pMutex;
  اگر( objc!=2 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "NAME");
    واپس TCL_ERROR;
  }
  pMutex = get_Static_Mutex_اشارہ_گر(interp, objv[1]);
  اگر( !pMutex ){
    واپس TCL_ERROR;
  }
  sqlite3_mutex_enter(pMutex);
  واپس TCL_OK;
}

ساکن صحیح SQLITE_TCLAPI test_leave_static_mutex(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_mutex *pMutex;
  اگر( objc!=2 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "NAME");
    واپس TCL_ERROR;
  }
  pMutex = get_Static_Mutex_اشارہ_گر(interp, objv[1]);
  اگر( !pMutex ){
    واپس TCL_ERROR;
  }
  sqlite3_mutex_leave(pMutex);
  واپس TCL_OK;
}

ساکن صحیح SQLITE_TCLAPI test_enter_db_mutex(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  اگر( objc!=2 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "DB");
    واپس TCL_ERROR;
  }
  اگر( get_Db_اشارہ_گر(interp, Tcl_GetString(objv[1]), &db) ){
    واپس TCL_ERROR;
  }
  اگر( !db ){
    واپس TCL_ERROR;
  }
  sqlite3_mutex_enter(sqlite3_db_mutex(db));
  واپس TCL_OK;
}

ساکن صحیح SQLITE_TCLAPI test_leave_db_mutex(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  اگر( objc!=2 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "DB");
    واپس TCL_ERROR;
  }
  اگر( get_Db_اشارہ_گر(interp, Tcl_GetString(objv[1]), &db) ){
    واپس TCL_ERROR;
  }
  اگر( !db ){
    واپس TCL_ERROR;
  }
  sqlite3_mutex_leave(sqlite3_db_mutex(db));
  واپس TCL_OK;
}

صحیح Sqlitetest_mutex_Init(Tcl_Interp *interp){
  ساکن ساخت {
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
  صحیح i;
  برائے(i=0; i<حجم(aCmd)/حجم(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  Tcl_LinkVar(interp, "disable_mutex_init", 
              (حرف*)&g.disableInit, TCL_LINK_INT);
  Tcl_LinkVar(interp, "disable_mutex_try", 
              (حرف*)&g.disableTry, TCL_LINK_INT);
  واپس SQLITE_OK;
}
