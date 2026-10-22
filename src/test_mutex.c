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
#অন্তর্ভাব "tclsqlite.h"
#অন্তর্ভাব "sqlite3.h"
#অন্তর্ভাব "sqliteInt.h"
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <assert.h>
#অন্তর্ভাব <string.h>

#define MAX_MUTEXES        (SQLITE_MUTEX_STATIC_VFS3+1)
#define STATIC_MUTEXES     (MAX_MUTEXES-(SQLITE_MUTEX_RECURSIVE+1))

/* defined in main.c */
বাহ্য ধ্রুব বর্ণ *sqlite3ErrName(পূর্ণাঙ্ক);
পূর্ণাঙ্ক getDbPointer(Tcl_Interp *interp, ধ্রুব বর্ণ *zA, sqlite3 **ppDb);

স্থির ধ্রুব বর্ণ *aName[MAX_MUTEXES+1] = {
  "fast",        "recursive",   "static_main",   "static_mem",
  "static_open", "static_prng", "static_lru",    "static_pmem",
  "static_app1", "static_app2", "static_app3",   "static_vfs1",
  "static_vfs2", "static_vfs3", 0
};

/* A countable mutex */
সংরচনা sqlite3_mutex {
  sqlite3_mutex *pReal;
  পূর্ণাঙ্ক eType;
};

/* State variables */
স্থির সংরচনা test_mutex_globals {
  পূর্ণাঙ্ক isInstalled;           /* True if installed */
  পূর্ণাঙ্ক disableInit;           /* True to cause sqlite3_initialize() to fail */
  পূর্ণাঙ্ক disableTry;            /* True to force sqlite3_mutex_try() to fail */
  পূর্ণাঙ্ক isInit;                /* True if initialized */
  sqlite3_mutex_methods m;   /* Interface to "real" mutex system */
  পূর্ণাঙ্ক aCounter[MAX_MUTEXES]; /* Number of grabs of each type of mutex */
  sqlite3_mutex aStatic[STATIC_MUTEXES]; /* The static mutexes */
} g = {0};

/* Return true if the countable mutex is currently held */
স্থির পূর্ণাঙ্ক counterMutexHeld(sqlite3_mutex *p){
  প্রত্যাগম g.m.xMutexHeld(p->pReal);
}

/* Return true if the countable mutex is not currently held */
স্থির পূর্ণাঙ্ক counterMutexNotheld(sqlite3_mutex *p){
  প্রত্যাগম g.m.xMutexNotheld(p->pReal);
}

/* Initialize the countable mutex interface
** Or, if g.disableInit is non-zero, then do not initialize but instead
** return the value of g.disableInit as the result code.  This can be used
** to simulate an initialization failure.
*/
স্থির পূর্ণাঙ্ক counterMutexInit(শূন্য){ 
  পূর্ণাঙ্ক rc;
  যদি( g.disableInit ) প্রত্যাগম g.disableInit;
  rc = g.m.xMutexInit();
  g.isInit = 1;
  প্রত্যাগম rc;
}

/*
** Uninitialize the mutex subsystem
*/
স্থির পূর্ণাঙ্ক counterMutexEnd(শূন্য){ 
  g.isInit = 0;
  প্রত্যাগম g.m.xMutexEnd();
}

/*
** Allocate a countable mutex
*/
স্থির sqlite3_mutex *counterMutexAlloc(পূর্ণাঙ্ক eType){
  sqlite3_mutex *pReal;
  sqlite3_mutex *pRet = 0;

  প্রতিজ্ঞা( g.isInit );
  প্রতিজ্ঞা( eType>=SQLITE_MUTEX_FAST );
  প্রতিজ্ঞা( eType<=SQLITE_MUTEX_STATIC_VFS3 );

  pReal = g.m.xMutexAlloc(eType);
  যদি( !pReal ) প্রত্যাগম 0;

  যদি( eType==SQLITE_MUTEX_FAST || eType==SQLITE_MUTEX_RECURSIVE ){
    pRet = (sqlite3_mutex *)malloc(পরিমাণ(sqlite3_mutex));
  }অন্যথা{
    পূর্ণাঙ্ক eStaticType = eType - (MAX_MUTEXES - STATIC_MUTEXES);
    প্রতিজ্ঞা( eStaticType>=0 );
    প্রতিজ্ঞা( eStaticType<STATIC_MUTEXES );
    pRet = &g.aStatic[eStaticType];
  }

  pRet->eType = eType;
  pRet->pReal = pReal;
  প্রত্যাগম pRet;
}

/*
** Free a countable mutex
*/
স্থির শূন্য counterMutexFree(sqlite3_mutex *p){
  প্রতিজ্ঞা( g.isInit );
  g.m.xMutexFree(p->pReal);
  যদি( p->eType==SQLITE_MUTEX_FAST || p->eType==SQLITE_MUTEX_RECURSIVE ){
    মুক্ত(p);
  }
}

/*
** Enter a countable mutex.  Block until entry is safe.
*/
স্থির শূন্য counterMutexEnter(sqlite3_mutex *p){
  প্রতিজ্ঞা( g.isInit );
  প্রতিজ্ঞা( p->eType>=0 );
  প্রতিজ্ঞা( p->eType<MAX_MUTEXES );
  g.aCounter[p->eType]++;
  g.m.xMutexEnter(p->pReal);
}

/*
** Try to enter a mutex.  Return true on success.
*/
স্থির পূর্ণাঙ্ক counterMutexTry(sqlite3_mutex *p){
  প্রতিজ্ঞা( g.isInit );
  প্রতিজ্ঞা( p->eType>=0 );
  প্রতিজ্ঞা( p->eType<MAX_MUTEXES );
  g.aCounter[p->eType]++;
  যদি( g.disableTry ) প্রত্যাগম SQLITE_BUSY;
  প্রত্যাগম g.m.xMutexTry(p->pReal);
}

/* Leave a mutex
*/
স্থির শূন্য counterMutexLeave(sqlite3_mutex *p){
  প্রতিজ্ঞা( g.isInit );
  g.m.xMutexLeave(p->pReal);
}

/*
** sqlite3_shutdown
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_shutdown(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  পূর্ণাঙ্ক rc;

  যদি( objc!=1 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "");
    প্রত্যাগম TCL_ERROR;
  }

  rc = sqlite3_shutdown();
  Tcl_SetResult(interp, (বর্ণ *)sqlite3ErrName(rc), TCL_VOLATILE);
  প্রত্যাগম TCL_OK;
}

/*
** sqlite3_initialize
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_initialize(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  পূর্ণাঙ্ক rc;

  যদি( objc!=1 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "");
    প্রত্যাগম TCL_ERROR;
  }

  rc = sqlite3_initialize();
  Tcl_SetResult(interp, (বর্ণ *)sqlite3ErrName(rc), TCL_VOLATILE);
  প্রত্যাগম TCL_OK;
}

/*
** install_mutex_counters BOOLEAN
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_install_mutex_counters(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  পূর্ণাঙ্ক rc = SQLITE_OK;
  পূর্ণাঙ্ক isInstall;

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

  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "BOOLEAN");
    প্রত্যাগম TCL_ERROR;
  }
  যদি( TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[1], &isInstall) ){
    প্রত্যাগম TCL_ERROR;
  }

  প্রতিজ্ঞা(isInstall==0 || isInstall==1);
  প্রতিজ্ঞা(g.isInstalled==0 || g.isInstalled==1);
  যদি( isInstall==g.isInstalled ){
    Tcl_AppendResult(interp, "mutex counters are ", রিক্ত);
    Tcl_AppendResult(interp, isInstall?"already installed":"not installed", রিক্ত);
    প্রত্যাগম TCL_ERROR;
  }

  যদি( isInstall ){
    প্রতিজ্ঞা( g.m.xMutexAlloc==0 );
    rc = sqlite3_config(SQLITE_CONFIG_GETMUTEX, &g.m);
    যদি( rc==SQLITE_OK ){
      sqlite3_config(SQLITE_CONFIG_MUTEX, &counter_methods);
    }
    g.disableTry = 0;
  }অন্যথা{
    প্রতিজ্ঞা( g.m.xMutexAlloc );
    rc = sqlite3_config(SQLITE_CONFIG_MUTEX, &g.m);
    memset(&g.m, 0, পরিমাণ(sqlite3_mutex_methods));
  }

  যদি( rc==SQLITE_OK ){
    g.isInstalled = isInstall;
  }

  Tcl_SetResult(interp, (বর্ণ *)sqlite3ErrName(rc), TCL_VOLATILE);
  প্রত্যাগম TCL_OK;
}

/*
** read_mutex_counters
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_read_mutex_counters(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Obj *pRet;
  পূর্ণাঙ্ক ii;

  যদি( objc!=1 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "");
    প্রত্যাগম TCL_ERROR;
  }

  pRet = Tcl_NewObj();
  Tcl_Incr_Ref_গুনতি(pRet);
  প্রতি(ii=0; ii<MAX_MUTEXES; ii++){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj(aName[ii], -1));
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewIntObj(g.aCounter[ii]));
  }
  Tcl_SetObjResult(interp, pRet);
  Tcl_Decr_Ref_গুনতি(pRet);

  প্রত্যাগম TCL_OK;
}

/*
** clear_mutex_counters
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_clear_mutex_counters(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  পূর্ণাঙ্ক ii;

  যদি( objc!=1 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "");
    প্রত্যাগম TCL_ERROR;
  }

  প্রতি(ii=0; ii<MAX_MUTEXES; ii++){
    g.aCounter[ii] = 0;
  }
  প্রত্যাগম TCL_OK;
}

/*
** Create and free a mutex.  Return the mutex pointer.  The pointer
** will be invalid since the mutex has already been freed.  The
** return pointer just checks to see if the mutex really was allocated.
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_alloc_mutex(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
#যদি SQLITE_THREADSAFE
  sqlite3_mutex *p = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
  বর্ণ zBuf[100];
  sqlite3_mutex_free(p);
  sqlite3_snprintf(পরিমাণ(zBuf), zBuf, "%p", p);
  Tcl_AppendResult(interp, zBuf, (বর্ণ*)0);
#endif
  প্রত্যাগম TCL_OK;
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
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_config(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  সংরচনা ConfigOption {
    ধ্রুব বর্ণ *zName;
    পূর্ণাঙ্ক i_মান;
  } aOpt[] = {
    {"singlethread", SQLITE_CONFIG_SINGLETHREAD},
    {"multithread",  SQLITE_CONFIG_MULTITHREAD},
    {"serialized",   SQLITE_CONFIG_SERIALIZED},
    {0, 0}
  };
  পূর্ণাঙ্ক s = পরিমাণ(সংরচনা ConfigOption);
  পূর্ণাঙ্ক i;
  পূর্ণাঙ্ক rc;

  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "");
    প্রত্যাগম TCL_ERROR;
  }

  যদি( Tcl_GetIndexFromObjStruct(interp, objv[1], aOpt, s, "flag", 0, &i) ){
    যদি( Tcl_GetIntFromObj(interp, objv[1], &i) ){
      প্রত্যাগম TCL_ERROR;
    }
  }অন্যথা{
    i = aOpt[i].i_মান;
  }

  rc = sqlite3_config(i);
  Tcl_SetResult(interp, (বর্ণ *)sqlite3ErrName(rc), TCL_VOLATILE);
  প্রত্যাগম TCL_OK;
}

স্থির sqlite3_mutex *getStaticMutexPointer(
  Tcl_Interp *pInterp,
  Tcl_Obj *pObj
){
  পূর্ণাঙ্ক iMutex;
  যদি( Tcl_GetIndexFromObj(pInterp, pObj, aName, "mutex name", 0, &iMutex) ){
    প্রত্যাগম 0;
  }
  প্রতিজ্ঞা( iMutex!=SQLITE_MUTEX_FAST && iMutex!=SQLITE_MUTEX_RECURSIVE );
  প্রত্যাগম counterMutexAlloc(iMutex);
}

স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_enter_static_mutex(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_mutex *pMutex;
  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "NAME");
    প্রত্যাগম TCL_ERROR;
  }
  pMutex = getStaticMutexPointer(interp, objv[1]);
  যদি( !pMutex ){
    প্রত্যাগম TCL_ERROR;
  }
  sqlite3_mutex_enter(pMutex);
  প্রত্যাগম TCL_OK;
}

স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_leave_static_mutex(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_mutex *pMutex;
  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "NAME");
    প্রত্যাগম TCL_ERROR;
  }
  pMutex = getStaticMutexPointer(interp, objv[1]);
  যদি( !pMutex ){
    প্রত্যাগম TCL_ERROR;
  }
  sqlite3_mutex_leave(pMutex);
  প্রত্যাগম TCL_OK;
}

স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_enter_db_mutex(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "DB");
    প্রত্যাগম TCL_ERROR;
  }
  যদি( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ){
    প্রত্যাগম TCL_ERROR;
  }
  যদি( !db ){
    প্রত্যাগম TCL_ERROR;
  }
  sqlite3_mutex_enter(sqlite3_db_mutex(db));
  প্রত্যাগম TCL_OK;
}

স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_leave_db_mutex(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "DB");
    প্রত্যাগম TCL_ERROR;
  }
  যদি( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ){
    প্রত্যাগম TCL_ERROR;
  }
  যদি( !db ){
    প্রত্যাগম TCL_ERROR;
  }
  sqlite3_mutex_leave(sqlite3_db_mutex(db));
  প্রত্যাগম TCL_OK;
}

পূর্ণাঙ্ক Sqlitetest_mutex_Init(Tcl_Interp *interp){
  স্থির সংরচনা {
    বর্ণ *zName;
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
  পূর্ণাঙ্ক i;
  প্রতি(i=0; i<পরিমাণ(aCmd)/পরিমাণ(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  Tcl_LinkVar(interp, "disable_mutex_init", 
              (বর্ণ*)&g.disableInit, TCL_LINK_INT);
  Tcl_LinkVar(interp, "disable_mutex_try", 
              (বর্ণ*)&g.disableTry, TCL_LINK_INT);
  প্রত্যাগম SQLITE_OK;
}
