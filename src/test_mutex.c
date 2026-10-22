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
বাহ্যিক ধ্রুবক অক্ষর *sqlite3ErrName(পূর্ণসংখ্যা);
পূর্ণসংখ্যা getDbPointer(Tcl_Interp *interp, ধ্রুবক অক্ষর *zA, sqlite3 **ppDb);

স্থির ধ্রুবক অক্ষর *aName[MAX_MUTEXES+1] = {
  "fast",        "recursive",   "static_main",   "static_mem",
  "static_open", "static_prng", "static_lru",    "static_pmem",
  "static_app1", "static_app2", "static_app3",   "static_vfs1",
  "static_vfs2", "static_vfs3", 0
};

/* A countable mutex */
কাঠামো sqlite3_mutex {
  sqlite3_mutex *pReal;
  পূর্ণসংখ্যা eType;
};

/* State variables */
স্থির কাঠামো test_mutex_globals {
  পূর্ণসংখ্যা isInstalled;           /* True if installed */
  পূর্ণসংখ্যা disableInit;           /* True to cause sqlite3_initialize() to fail */
  পূর্ণসংখ্যা disableTry;            /* True to force sqlite3_mutex_try() to fail */
  পূর্ণসংখ্যা isInit;                /* True if initialized */
  sqlite3_mutex_methods m;   /* Interface to "real" mutex system */
  পূর্ণসংখ্যা a_গণক[MAX_MUTEXES]; /* Number of grabs of each type of mutex */
  sqlite3_mutex aStatic[STATIC_MUTEXES]; /* The static mutexes */
} g = {0};

/* Return true if the countable mutex is currently held */
স্থির পূর্ণসংখ্যা গণক_Mutex_Held(sqlite3_mutex *p){
  ফেরত g.m.xMutexHeld(p->pReal);
}

/* Return true if the countable mutex is not currently held */
স্থির পূর্ণসংখ্যা গণক_Mutex_Notheld(sqlite3_mutex *p){
  ফেরত g.m.xMutexNotheld(p->pReal);
}

/* Initialize the countable mutex interface
** Or, if g.disableInit is non-zero, then do not initialize but instead
** return the value of g.disableInit as the result code.  This can be used
** to simulate an initialization failure.
*/
স্থির পূর্ণসংখ্যা গণক_Mutex_Init(শূন্য){ 
  পূর্ণসংখ্যা rc;
  যদি( g.disableInit ) ফেরত g.disableInit;
  rc = g.m.xMutexInit();
  g.isInit = 1;
  ফেরত rc;
}

/*
** Uninitialize the mutex subsystem
*/
স্থির পূর্ণসংখ্যা গণক_Mutex_End(শূন্য){ 
  g.isInit = 0;
  ফেরত g.m.xMutexEnd();
}

/*
** Allocate a countable mutex
*/
স্থির sqlite3_mutex *গণক_Mutex_Alloc(পূর্ণসংখ্যা eType){
  sqlite3_mutex *pReal;
  sqlite3_mutex *pRet = 0;

  প্রতিজ্ঞা( g.isInit );
  প্রতিজ্ঞা( eType>=SQLITE_MUTEX_FAST );
  প্রতিজ্ঞা( eType<=SQLITE_MUTEX_STATIC_VFS3 );

  pReal = g.m.xMutexAlloc(eType);
  যদি( !pReal ) ফেরত 0;

  যদি( eType==SQLITE_MUTEX_FAST || eType==SQLITE_MUTEX_RECURSIVE ){
    pRet = (sqlite3_mutex *)malloc(আকার(sqlite3_mutex));
  }অন্যথা{
    পূর্ণসংখ্যা eStaticType = eType - (MAX_MUTEXES - STATIC_MUTEXES);
    প্রতিজ্ঞা( eStaticType>=0 );
    প্রতিজ্ঞা( eStaticType<STATIC_MUTEXES );
    pRet = &g.aStatic[eStaticType];
  }

  pRet->eType = eType;
  pRet->pReal = pReal;
  ফেরত pRet;
}

/*
** Free a countable mutex
*/
স্থির শূন্য গণক_Mutex_Free(sqlite3_mutex *p){
  প্রতিজ্ঞা( g.isInit );
  g.m.xMutexFree(p->pReal);
  যদি( p->eType==SQLITE_MUTEX_FAST || p->eType==SQLITE_MUTEX_RECURSIVE ){
    মুক্ত(p);
  }
}

/*
** Enter a countable mutex.  Block until entry is safe.
*/
স্থির শূন্য গণক_Mutex_Enter(sqlite3_mutex *p){
  প্রতিজ্ঞা( g.isInit );
  প্রতিজ্ঞা( p->eType>=0 );
  প্রতিজ্ঞা( p->eType<MAX_MUTEXES );
  g.a_গণক[p->eType]++;
  g.m.xMutexEnter(p->pReal);
}

/*
** Try to enter a mutex.  Return true on success.
*/
স্থির পূর্ণসংখ্যা গণক_Mutex_Try(sqlite3_mutex *p){
  প্রতিজ্ঞা( g.isInit );
  প্রতিজ্ঞা( p->eType>=0 );
  প্রতিজ্ঞা( p->eType<MAX_MUTEXES );
  g.a_গণক[p->eType]++;
  যদি( g.disableTry ) ফেরত SQLITE_BUSY;
  ফেরত g.m.xMutexTry(p->pReal);
}

/* Leave a mutex
*/
স্থির শূন্য গণক_Mutex_Leave(sqlite3_mutex *p){
  প্রতিজ্ঞা( g.isInit );
  g.m.xMutexLeave(p->pReal);
}

/*
** sqlite3_shutdown
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI test_shutdown(
  শূন্য * client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  পূর্ণসংখ্যা rc;

  যদি( objc!=1 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "");
    ফেরত TCL_ERROR;
  }

  rc = sqlite3_shutdown();
  Tcl_Set_ফলাফল(interp, (অক্ষর *)sqlite3ErrName(rc), TCL_VOLATILE);
  ফেরত TCL_OK;
}

/*
** sqlite3_initialize
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI test_initialize(
  শূন্য * client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  পূর্ণসংখ্যা rc;

  যদি( objc!=1 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "");
    ফেরত TCL_ERROR;
  }

  rc = sqlite3_initialize();
  Tcl_Set_ফলাফল(interp, (অক্ষর *)sqlite3ErrName(rc), TCL_VOLATILE);
  ফেরত TCL_OK;
}

/*
** install_mutex_counters BOOLEAN
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI test_install_mutex_counters(
  শূন্য * client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  পূর্ণসংখ্যা rc = SQLITE_OK;
  পূর্ণসংখ্যা isInstall;

  sqlite3_mutex_methods গণক_methods = {
    গণক_Mutex_Init,
    গণক_Mutex_End,
    গণক_Mutex_Alloc,
    গণক_Mutex_Free,
    গণক_Mutex_Enter,
    গণক_Mutex_Try,
    গণক_Mutex_Leave,
    গণক_Mutex_Held,
    গণক_Mutex_Notheld
  };

  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "BOOLEAN");
    ফেরত TCL_ERROR;
  }
  যদি( TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[1], &isInstall) ){
    ফেরত TCL_ERROR;
  }

  প্রতিজ্ঞা(isInstall==0 || isInstall==1);
  প্রতিজ্ঞা(g.isInstalled==0 || g.isInstalled==1);
  যদি( isInstall==g.isInstalled ){
    Tcl_Append_ফলাফল(interp, "mutex counters are ", ফাঁকা);
    Tcl_Append_ফলাফল(interp, isInstall?"already installed":"not installed", ফাঁকা);
    ফেরত TCL_ERROR;
  }

  যদি( isInstall ){
    প্রতিজ্ঞা( g.m.xMutexAlloc==0 );
    rc = sqlite3_config(SQLITE_CONFIG_GETMUTEX, &g.m);
    যদি( rc==SQLITE_OK ){
      sqlite3_config(SQLITE_CONFIG_MUTEX, &গণক_methods);
    }
    g.disableTry = 0;
  }অন্যথা{
    প্রতিজ্ঞা( g.m.xMutexAlloc );
    rc = sqlite3_config(SQLITE_CONFIG_MUTEX, &g.m);
    memset(&g.m, 0, আকার(sqlite3_mutex_methods));
  }

  যদি( rc==SQLITE_OK ){
    g.isInstalled = isInstall;
  }

  Tcl_Set_ফলাফল(interp, (অক্ষর *)sqlite3ErrName(rc), TCL_VOLATILE);
  ফেরত TCL_OK;
}

/*
** read_mutex_counters
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI test_read_mutex_counters(
  শূন্য * client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Obj *pRet;
  পূর্ণসংখ্যা ii;

  যদি( objc!=1 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "");
    ফেরত TCL_ERROR;
  }

  pRet = Tcl_NewObj();
  Tcl_Incr_Ref_গুনতি(pRet);
  জন্য(ii=0; ii<MAX_MUTEXES; ii++){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj(aName[ii], -1));
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewIntObj(g.a_গণক[ii]));
  }
  Tcl_Set_Obj_ফলাফল(interp, pRet);
  Tcl_Decr_Ref_গুনতি(pRet);

  ফেরত TCL_OK;
}

/*
** clear_mutex_counters
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI test_clear_mutex_counters(
  শূন্য * client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  পূর্ণসংখ্যা ii;

  যদি( objc!=1 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "");
    ফেরত TCL_ERROR;
  }

  জন্য(ii=0; ii<MAX_MUTEXES; ii++){
    g.a_গণক[ii] = 0;
  }
  ফেরত TCL_OK;
}

/*
** Create and free a mutex.  Return the mutex pointer.  The pointer
** will be invalid since the mutex has already been freed.  The
** return pointer just checks to see if the mutex really was allocated.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI test_alloc_mutex(
  শূন্য * client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
#যদি SQLITE_THREADSAFE
  sqlite3_mutex *p = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
  অক্ষর zBuf[100];
  sqlite3_mutex_free(p);
  sqlite3_snprintf(আকার(zBuf), zBuf, "%p", p);
  Tcl_Append_ফলাফল(interp, zBuf, (অক্ষর*)0);
#endif
  ফেরত TCL_OK;
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
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI test_config(
  শূন্য * client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  কাঠামো ConfigOption {
    ধ্রুবক অক্ষর *zName;
    পূর্ণসংখ্যা i_মান;
  } aOpt[] = {
    {"singlethread", SQLITE_CONFIG_SINGLETHREAD},
    {"multithread",  SQLITE_CONFIG_MULTITHREAD},
    {"serialized",   SQLITE_CONFIG_SERIALIZED},
    {0, 0}
  };
  পূর্ণসংখ্যা s = আকার(কাঠামো ConfigOption);
  পূর্ণসংখ্যা i;
  পূর্ণসংখ্যা rc;

  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "");
    ফেরত TCL_ERROR;
  }

  যদি( Tcl_Get_সূচক_From_Obj_Struct(interp, objv[1], aOpt, s, "flag", 0, &i) ){
    যদি( Tcl_GetIntFromObj(interp, objv[1], &i) ){
      ফেরত TCL_ERROR;
    }
  }অন্যথা{
    i = aOpt[i].i_মান;
  }

  rc = sqlite3_config(i);
  Tcl_Set_ফলাফল(interp, (অক্ষর *)sqlite3ErrName(rc), TCL_VOLATILE);
  ফেরত TCL_OK;
}

স্থির sqlite3_mutex *getStaticMutexPointer(
  Tcl_Interp *pInterp,
  Tcl_Obj *pObj
){
  পূর্ণসংখ্যা iMutex;
  যদি( Tcl_Get_সূচক_From_Obj(pInterp, pObj, aName, "mutex name", 0, &iMutex) ){
    ফেরত 0;
  }
  প্রতিজ্ঞা( iMutex!=SQLITE_MUTEX_FAST && iMutex!=SQLITE_MUTEX_RECURSIVE );
  ফেরত গণক_Mutex_Alloc(iMutex);
}

স্থির পূর্ণসংখ্যা SQLITE_TCLAPI test_enter_static_mutex(
  শূন্য * client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_mutex *pMutex;
  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "NAME");
    ফেরত TCL_ERROR;
  }
  pMutex = getStaticMutexPointer(interp, objv[1]);
  যদি( !pMutex ){
    ফেরত TCL_ERROR;
  }
  sqlite3_mutex_enter(pMutex);
  ফেরত TCL_OK;
}

স্থির পূর্ণসংখ্যা SQLITE_TCLAPI test_leave_static_mutex(
  শূন্য * client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_mutex *pMutex;
  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "NAME");
    ফেরত TCL_ERROR;
  }
  pMutex = getStaticMutexPointer(interp, objv[1]);
  যদি( !pMutex ){
    ফেরত TCL_ERROR;
  }
  sqlite3_mutex_leave(pMutex);
  ফেরত TCL_OK;
}

স্থির পূর্ণসংখ্যা SQLITE_TCLAPI test_enter_db_mutex(
  শূন্য * client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "DB");
    ফেরত TCL_ERROR;
  }
  যদি( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ){
    ফেরত TCL_ERROR;
  }
  যদি( !db ){
    ফেরত TCL_ERROR;
  }
  sqlite3_mutex_enter(sqlite3_db_mutex(db));
  ফেরত TCL_OK;
}

স্থির পূর্ণসংখ্যা SQLITE_TCLAPI test_leave_db_mutex(
  শূন্য * client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "DB");
    ফেরত TCL_ERROR;
  }
  যদি( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ){
    ফেরত TCL_ERROR;
  }
  যদি( !db ){
    ফেরত TCL_ERROR;
  }
  sqlite3_mutex_leave(sqlite3_db_mutex(db));
  ফেরত TCL_OK;
}

পূর্ণসংখ্যা Sqlitetest_mutex_Init(Tcl_Interp *interp){
  স্থির কাঠামো {
    অক্ষর *zName;
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
  পূর্ণসংখ্যা i;
  জন্য(i=0; i<আকার(aCmd)/আকার(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  Tcl_LinkVar(interp, "disable_mutex_init", 
              (অক্ষর*)&g.disableInit, TCL_LINK_INT);
  Tcl_LinkVar(interp, "disable_mutex_try", 
              (অক্ষর*)&g.disableTry, TCL_LINK_INT);
  ফেরত SQLITE_OK;
}
