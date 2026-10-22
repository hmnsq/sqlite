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
#అన్తర్భావ "tclsqlite.h"
#అన్తర్భావ "sqlite3.h"
#అన్తర్భావ "sqliteInt.h"
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <assert.h>
#అన్తర్భావ <string.h>

#నిర్వచించు MAX_MUTEXES        (SQLITE_MUTEX_STATIC_VFS3+1)
#నిర్వచించు STATIC_MUTEXES     (MAX_MUTEXES-(SQLITE_MUTEX_RECURSIVE+1))

/* defined in main.c */
బాహ్య స్థిరరాశి అక్షరం *sqlite3ErrName(పూర్ణాంకం);
పూర్ణాంకం get_Db_సూచిక(Tcl_Interp *interp, స్థిరరాశి అక్షరం *zA, sqlite3 **ppDb);

స్థిర స్థిరరాశి అక్షరం *aName[MAX_MUTEXES+1] = {
  "fast",        "recursive",   "static_main",   "static_mem",
  "static_open", "static_prng", "static_lru",    "static_pmem",
  "static_app1", "static_app2", "static_app3",   "static_vfs1",
  "static_vfs2", "static_vfs3", 0
};

/* A countable mutex */
నిర్మాణం sqlite3_mutex {
  sqlite3_mutex *pReal;
  పూర్ణాంకం eType;
};

/* State variables */
స్థిర నిర్మాణం test_mutex_globals {
  పూర్ణాంకం isInstalled;           /* True if installed */
  పూర్ణాంకం disableInit;           /* True to cause sqlite3_initialize() to fail */
  పూర్ణాంకం disableTry;            /* True to force sqlite3_mutex_try() to fail */
  పూర్ణాంకం isInit;                /* True if initialized */
  sqlite3_mutex_methods m;   /* Interface to "real" mutex system */
  పూర్ణాంకం aCounter[MAX_MUTEXES]; /* Number of grabs of each type of mutex */
  sqlite3_mutex aStatic[STATIC_MUTEXES]; /* The static mutexes */
} g = {0};

/* Return true if the countable mutex is currently held */
స్థిర పూర్ణాంకం counterMutexHeld(sqlite3_mutex *p){
  తిరిగి g.m.xMutexHeld(p->pReal);
}

/* Return true if the countable mutex is not currently held */
స్థిర పూర్ణాంకం counterMutexNotheld(sqlite3_mutex *p){
  తిరిగి g.m.xMutexNotheld(p->pReal);
}

/* Initialize the countable mutex interface
** Or, if g.disableInit is non-zero, then do not initialize but instead
** return the value of g.disableInit as the result code.  This can be used
** to simulate an initialization failure.
*/
స్థిర పూర్ణాంకం counterMutexInit(శూన్య){ 
  పూర్ణాంకం rc;
  ఒకవేళ( g.disableInit ) తిరిగి g.disableInit;
  rc = g.m.xMutexInit();
  g.isInit = 1;
  తిరిగి rc;
}

/*
** Uninitialize the mutex subsystem
*/
స్థిర పూర్ణాంకం counter_Mutex_చివర(శూన్య){ 
  g.isInit = 0;
  తిరిగి g.m.x_Mutex_చివర();
}

/*
** Allocate a countable mutex
*/
స్థిర sqlite3_mutex *counterMutexAlloc(పూర్ణాంకం eType){
  sqlite3_mutex *pReal;
  sqlite3_mutex *pRet = 0;

  ప్రతిజ్ఞా( g.isInit );
  ప్రతిజ్ఞా( eType>=SQLITE_MUTEX_FAST );
  ప్రతిజ్ఞా( eType<=SQLITE_MUTEX_STATIC_VFS3 );

  pReal = g.m.xMutexAlloc(eType);
  ఒకవేళ( !pReal ) తిరిగి 0;

  ఒకవేళ( eType==SQLITE_MUTEX_FAST || eType==SQLITE_MUTEX_RECURSIVE ){
    pRet = (sqlite3_mutex *)జ్ఞాపకం_ఇవ్వు(పరిమాణం(sqlite3_mutex));
  }లేకపోతే{
    పూర్ణాంకం eStaticType = eType - (MAX_MUTEXES - STATIC_MUTEXES);
    ప్రతిజ్ఞా( eStaticType>=0 );
    ప్రతిజ్ఞా( eStaticType<STATIC_MUTEXES );
    pRet = &g.aStatic[eStaticType];
  }

  pRet->eType = eType;
  pRet->pReal = pReal;
  తిరిగి pRet;
}

/*
** Free a countable mutex
*/
స్థిర శూన్య counterMutexFree(sqlite3_mutex *p){
  ప్రతిజ్ఞా( g.isInit );
  g.m.xMutexFree(p->pReal);
  ఒకవేళ( p->eType==SQLITE_MUTEX_FAST || p->eType==SQLITE_MUTEX_RECURSIVE ){
    విడుదల(p);
  }
}

/*
** Enter a countable mutex.  Block until entry is safe.
*/
స్థిర శూన్య counterMutexEnter(sqlite3_mutex *p){
  ప్రతిజ్ఞా( g.isInit );
  ప్రతిజ్ఞా( p->eType>=0 );
  ప్రతిజ్ఞా( p->eType<MAX_MUTEXES );
  g.aCounter[p->eType]++;
  g.m.xMutexEnter(p->pReal);
}

/*
** Try to enter a mutex.  Return true on success.
*/
స్థిర పూర్ణాంకం counterMutexTry(sqlite3_mutex *p){
  ప్రతిజ్ఞా( g.isInit );
  ప్రతిజ్ఞా( p->eType>=0 );
  ప్రతిజ్ఞా( p->eType<MAX_MUTEXES );
  g.aCounter[p->eType]++;
  ఒకవేళ( g.disableTry ) తిరిగి SQLITE_BUSY;
  తిరిగి g.m.xMutexTry(p->pReal);
}

/* Leave a mutex
*/
స్థిర శూన్య counterMutexLeave(sqlite3_mutex *p){
  ప్రతిజ్ఞా( g.isInit );
  g.m.xMutexLeave(p->pReal);
}

/*
** sqlite3_shutdown
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_shutdown(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  పూర్ణాంకం rc;

  ఒకవేళ( objc!=1 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "");
    తిరిగి TCL_ERROR;
  }

  rc = sqlite3_shutdown();
  Tcl_సమితి_ఫలితం(interp, (అక్షరం *)sqlite3ErrName(rc), TCL_VOLATILE);
  తిరిగి TCL_OK;
}

/*
** sqlite3_initialize
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_initialize(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  పూర్ణాంకం rc;

  ఒకవేళ( objc!=1 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "");
    తిరిగి TCL_ERROR;
  }

  rc = sqlite3_initialize();
  Tcl_సమితి_ఫలితం(interp, (అక్షరం *)sqlite3ErrName(rc), TCL_VOLATILE);
  తిరిగి TCL_OK;
}

/*
** install_mutex_counters BOOLEAN
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_install_mutex_counters(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  పూర్ణాంకం rc = SQLITE_OK;
  పూర్ణాంకం isInstall;

  sqlite3_mutex_methods counter_methods = {
    counterMutexInit,
    counter_Mutex_చివర,
    counterMutexAlloc,
    counterMutexFree,
    counterMutexEnter,
    counterMutexTry,
    counterMutexLeave,
    counterMutexHeld,
    counterMutexNotheld
  };

  ఒకవేళ( objc!=2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "BOOLEAN");
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[1], &isInstall) ){
    తిరిగి TCL_ERROR;
  }

  ప్రతిజ్ఞా(isInstall==0 || isInstall==1);
  ప్రతిజ్ఞా(g.isInstalled==0 || g.isInstalled==1);
  ఒకవేళ( isInstall==g.isInstalled ){
    Tcl_Append_ఫలితం(interp, "mutex counters are ", ఖాళీ);
    Tcl_Append_ఫలితం(interp, isInstall?"already installed":"not installed", ఖాళీ);
    తిరిగి TCL_ERROR;
  }

  ఒకవేళ( isInstall ){
    ప్రతిజ్ఞా( g.m.xMutexAlloc==0 );
    rc = sqlite3_config(SQLITE_CONFIG_GETMUTEX, &g.m);
    ఒకవేళ( rc==SQLITE_OK ){
      sqlite3_config(SQLITE_CONFIG_MUTEX, &counter_methods);
    }
    g.disableTry = 0;
  }లేకపోతే{
    ప్రతిజ్ఞా( g.m.xMutexAlloc );
    rc = sqlite3_config(SQLITE_CONFIG_MUTEX, &g.m);
    memset(&g.m, 0, పరిమాణం(sqlite3_mutex_methods));
  }

  ఒకవేళ( rc==SQLITE_OK ){
    g.isInstalled = isInstall;
  }

  Tcl_సమితి_ఫలితం(interp, (అక్షరం *)sqlite3ErrName(rc), TCL_VOLATILE);
  తిరిగి TCL_OK;
}

/*
** read_mutex_counters
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_read_mutex_counters(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Obj *pRet;
  పూర్ణాంకం ii;

  ఒకవేళ( objc!=1 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "");
    తిరిగి TCL_ERROR;
  }

  pRet = Tcl_NewObj();
  Tcl_Incr_Ref_లెక్క(pRet);
  కొరకు(ii=0; ii<MAX_MUTEXES; ii++){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj(aName[ii], -1));
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewIntObj(g.aCounter[ii]));
  }
  Tcl_సమితి_Obj_ఫలితం(interp, pRet);
  Tcl_Decr_Ref_లెక్క(pRet);

  తిరిగి TCL_OK;
}

/*
** clear_mutex_counters
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_clear_mutex_counters(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  పూర్ణాంకం ii;

  ఒకవేళ( objc!=1 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "");
    తిరిగి TCL_ERROR;
  }

  కొరకు(ii=0; ii<MAX_MUTEXES; ii++){
    g.aCounter[ii] = 0;
  }
  తిరిగి TCL_OK;
}

/*
** Create and free a mutex.  Return the mutex pointer.  The pointer
** will be invalid since the mutex has already been freed.  The
** return pointer just checks to see if the mutex really was allocated.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_alloc_mutex(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
#ఒకవేళ SQLITE_THREADSAFE
  sqlite3_mutex *p = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
  అక్షరం zBuf[100];
  sqlite3_mutex_free(p);
  sqlite3_snprintf(పరిమాణం(zBuf), zBuf, "%p", p);
  Tcl_Append_ఫలితం(interp, zBuf, (అక్షరం*)0);
#అయితే_ముగింపు
  తిరిగి TCL_OK;
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
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_config(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  నిర్మాణం ConfigOption {
    స్థిరరాశి అక్షరం *zName;
    పూర్ణాంకం i_విలువ;
  } aOpt[] = {
    {"singlethread", SQLITE_CONFIG_SINGLETHREAD},
    {"multithread",  SQLITE_CONFIG_MULTITHREAD},
    {"serialized",   SQLITE_CONFIG_SERIALIZED},
    {0, 0}
  };
  పూర్ణాంకం s = పరిమాణం(నిర్మాణం ConfigOption);
  పూర్ణాంకం i;
  పూర్ణాంకం rc;

  ఒకవేళ( objc!=2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "");
    తిరిగి TCL_ERROR;
  }

  ఒకవేళ( Tcl_Get_సూచీ_From_Obj_Struct(interp, objv[1], aOpt, s, "flag", 0, &i) ){
    ఒకవేళ( Tcl_GetIntFromObj(interp, objv[1], &i) ){
      తిరిగి TCL_ERROR;
    }
  }లేకపోతే{
    i = aOpt[i].i_విలువ;
  }

  rc = sqlite3_config(i);
  Tcl_సమితి_ఫలితం(interp, (అక్షరం *)sqlite3ErrName(rc), TCL_VOLATILE);
  తిరిగి TCL_OK;
}

స్థిర sqlite3_mutex *get_Static_Mutex_సూచిక(
  Tcl_Interp *pInterp,
  Tcl_Obj *pObj
){
  పూర్ణాంకం iMutex;
  ఒకవేళ( Tcl_Get_సూచీ_From_Obj(pInterp, pObj, aName, "mutex name", 0, &iMutex) ){
    తిరిగి 0;
  }
  ప్రతిజ్ఞా( iMutex!=SQLITE_MUTEX_FAST && iMutex!=SQLITE_MUTEX_RECURSIVE );
  తిరిగి counterMutexAlloc(iMutex);
}

స్థిర పూర్ణాంకం SQLITE_TCLAPI test_enter_static_mutex(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_mutex *pMutex;
  ఒకవేళ( objc!=2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "NAME");
    తిరిగి TCL_ERROR;
  }
  pMutex = get_Static_Mutex_సూచిక(interp, objv[1]);
  ఒకవేళ( !pMutex ){
    తిరిగి TCL_ERROR;
  }
  sqlite3_mutex_enter(pMutex);
  తిరిగి TCL_OK;
}

స్థిర పూర్ణాంకం SQLITE_TCLAPI test_leave_static_mutex(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_mutex *pMutex;
  ఒకవేళ( objc!=2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "NAME");
    తిరిగి TCL_ERROR;
  }
  pMutex = get_Static_Mutex_సూచిక(interp, objv[1]);
  ఒకవేళ( !pMutex ){
    తిరిగి TCL_ERROR;
  }
  sqlite3_mutex_leave(pMutex);
  తిరిగి TCL_OK;
}

స్థిర పూర్ణాంకం SQLITE_TCLAPI test_enter_db_mutex(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  ఒకవేళ( objc!=2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "DB");
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( get_Db_సూచిక(interp, Tcl_GetString(objv[1]), &db) ){
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( !db ){
    తిరిగి TCL_ERROR;
  }
  sqlite3_mutex_enter(sqlite3_db_mutex(db));
  తిరిగి TCL_OK;
}

స్థిర పూర్ణాంకం SQLITE_TCLAPI test_leave_db_mutex(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  ఒకవేళ( objc!=2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "DB");
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( get_Db_సూచిక(interp, Tcl_GetString(objv[1]), &db) ){
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( !db ){
    తిరిగి TCL_ERROR;
  }
  sqlite3_mutex_leave(sqlite3_db_mutex(db));
  తిరిగి TCL_OK;
}

పూర్ణాంకం Sqlitetest_mutex_Init(Tcl_Interp *interp){
  స్థిర నిర్మాణం {
    అక్షరం *zName;
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
  పూర్ణాంకం i;
  కొరకు(i=0; i<పరిమాణం(aCmd)/పరిమాణం(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  Tcl_LinkVar(interp, "disable_mutex_init", 
              (అక్షరం*)&g.disableInit, TCL_LINK_INT);
  Tcl_LinkVar(interp, "disable_mutex_try", 
              (అక్షరం*)&g.disableTry, TCL_LINK_INT);
  తిరిగి SQLITE_OK;
}
