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
#ଅନ୍ତର୍ଭାଵ "tclsqlite.h"
#ଅନ୍ତର୍ଭାଵ "sqlite3.h"
#ଅନ୍ତର୍ଭାଵ "sqliteInt.h"
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <assert.h>
#ଅନ୍ତର୍ଭାଵ <string.h>

#define MAX_MUTEXES        (SQLITE_MUTEX_STATIC_VFS3+1)
#define STATIC_MUTEXES     (MAX_MUTEXES-(SQLITE_MUTEX_RECURSIVE+1))

/* defined in main.c */
ବାହ୍ୟ ଅଚଳ ଅକ୍ଷର *sqlite3ErrName(ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା getDbPointer(Tcl_Interp *interp, ଅଚଳ ଅକ୍ଷର *zA, sqlite3 **ppDb);

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *aName[MAX_MUTEXES+1] = {
  "fast",        "recursive",   "static_main",   "static_mem",
  "static_open", "static_prng", "static_lru",    "static_pmem",
  "static_app1", "static_app2", "static_app3",   "static_vfs1",
  "static_vfs2", "static_vfs3", 0
};

/* A countable mutex */
ସଂରଚନା sqlite3_mutex {
  sqlite3_mutex *pReal;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା eType;
};

/* State variables */
ସ୍ଥିର ସଂରଚନା test_mutex_globals {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା isInstalled;           /* True if installed */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା disableInit;           /* True to cause sqlite3_initialize() to fail */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା disableTry;            /* True to force sqlite3_mutex_try() to fail */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା isInit;                /* True if initialized */
  sqlite3_mutex_methods m;   /* Interface to "real" mutex system */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା aCounter[MAX_MUTEXES]; /* Number of grabs of each type of mutex */
  sqlite3_mutex aStatic[STATIC_MUTEXES]; /* The static mutexes */
} g = {0};

/* Return true if the countable mutex is currently held */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା counterMutexHeld(sqlite3_mutex *p){
  ଫେରନ୍ତୁ g.m.xMutexHeld(p->pReal);
}

/* Return true if the countable mutex is not currently held */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା counterMutexNotheld(sqlite3_mutex *p){
  ଫେରନ୍ତୁ g.m.xMutexNotheld(p->pReal);
}

/* Initialize the countable mutex interface
** Or, if g.disableInit is non-zero, then do not initialize but instead
** return the value of g.disableInit as the result code.  This can be used
** to simulate an initialization failure.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା counterMutexInit(ଶୂନ୍ୟ){ 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ଯଦି( g.disableInit ) ଫେରନ୍ତୁ g.disableInit;
  rc = g.m.xMutexInit();
  g.isInit = 1;
  ଫେରନ୍ତୁ rc;
}

/*
** Uninitialize the mutex subsystem
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା counterMutexEnd(ଶୂନ୍ୟ){ 
  g.isInit = 0;
  ଫେରନ୍ତୁ g.m.xMutexEnd();
}

/*
** Allocate a countable mutex
*/
ସ୍ଥିର sqlite3_mutex *counterMutexAlloc(ପୂର୍ଣ୍ଣସଂଖ୍ୟା eType){
  sqlite3_mutex *pReal;
  sqlite3_mutex *pRet = 0;

  ପ୍ରତିଜ୍ଞା( g.isInit );
  ପ୍ରତିଜ୍ଞା( eType>=SQLITE_MUTEX_FAST );
  ପ୍ରତିଜ୍ଞା( eType<=SQLITE_MUTEX_STATIC_VFS3 );

  pReal = g.m.xMutexAlloc(eType);
  ଯଦି( !pReal ) ଫେରନ୍ତୁ 0;

  ଯଦି( eType==SQLITE_MUTEX_FAST || eType==SQLITE_MUTEX_RECURSIVE ){
    pRet = (sqlite3_mutex *)malloc(ଆକାର(sqlite3_mutex));
  }ଅନ୍ୟଥା{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା eStaticType = eType - (MAX_MUTEXES - STATIC_MUTEXES);
    ପ୍ରତିଜ୍ଞା( eStaticType>=0 );
    ପ୍ରତିଜ୍ଞା( eStaticType<STATIC_MUTEXES );
    pRet = &g.aStatic[eStaticType];
  }

  pRet->eType = eType;
  pRet->pReal = pReal;
  ଫେରନ୍ତୁ pRet;
}

/*
** Free a countable mutex
*/
ସ୍ଥିର ଶୂନ୍ୟ counterMutexFree(sqlite3_mutex *p){
  ପ୍ରତିଜ୍ଞା( g.isInit );
  g.m.xMutexFree(p->pReal);
  ଯଦି( p->eType==SQLITE_MUTEX_FAST || p->eType==SQLITE_MUTEX_RECURSIVE ){
    ମୁକ୍ତ(p);
  }
}

/*
** Enter a countable mutex.  Block until entry is safe.
*/
ସ୍ଥିର ଶୂନ୍ୟ counterMutexEnter(sqlite3_mutex *p){
  ପ୍ରତିଜ୍ଞା( g.isInit );
  ପ୍ରତିଜ୍ଞା( p->eType>=0 );
  ପ୍ରତିଜ୍ଞା( p->eType<MAX_MUTEXES );
  g.aCounter[p->eType]++;
  g.m.xMutexEnter(p->pReal);
}

/*
** Try to enter a mutex.  Return true on success.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା counterMutexTry(sqlite3_mutex *p){
  ପ୍ରତିଜ୍ଞା( g.isInit );
  ପ୍ରତିଜ୍ଞା( p->eType>=0 );
  ପ୍ରତିଜ୍ଞା( p->eType<MAX_MUTEXES );
  g.aCounter[p->eType]++;
  ଯଦି( g.disableTry ) ଫେରନ୍ତୁ SQLITE_BUSY;
  ଫେରନ୍ତୁ g.m.xMutexTry(p->pReal);
}

/* Leave a mutex
*/
ସ୍ଥିର ଶୂନ୍ୟ counterMutexLeave(sqlite3_mutex *p){
  ପ୍ରତିଜ୍ଞା( g.isInit );
  g.m.xMutexLeave(p->pReal);
}

/*
** sqlite3_shutdown
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_shutdown(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

  ଯଦି( objc!=1 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "");
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  rc = sqlite3_shutdown();
  Tcl_SetResult(interp, (ଅକ୍ଷର *)sqlite3ErrName(rc), TCL_VOLATILE);
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** sqlite3_initialize
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_initialize(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

  ଯଦି( objc!=1 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "");
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  rc = sqlite3_initialize();
  Tcl_SetResult(interp, (ଅକ୍ଷର *)sqlite3ErrName(rc), TCL_VOLATILE);
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** install_mutex_counters BOOLEAN
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_install_mutex_counters(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା isInstall;

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

  ଯଦି( objc!=2 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "BOOLEAN");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[1], &isInstall) ){
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  ପ୍ରତିଜ୍ଞା(isInstall==0 || isInstall==1);
  ପ୍ରତିଜ୍ଞା(g.isInstalled==0 || g.isInstalled==1);
  ଯଦି( isInstall==g.isInstalled ){
    Tcl_AppendResult(interp, "mutex counters are ", ଖାଲି);
    Tcl_AppendResult(interp, isInstall?"already installed":"not installed", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  ଯଦି( isInstall ){
    ପ୍ରତିଜ୍ଞା( g.m.xMutexAlloc==0 );
    rc = sqlite3_config(SQLITE_CONFIG_GETMUTEX, &g.m);
    ଯଦି( rc==SQLITE_OK ){
      sqlite3_config(SQLITE_CONFIG_MUTEX, &counter_methods);
    }
    g.disableTry = 0;
  }ଅନ୍ୟଥା{
    ପ୍ରତିଜ୍ଞା( g.m.xMutexAlloc );
    rc = sqlite3_config(SQLITE_CONFIG_MUTEX, &g.m);
    memset(&g.m, 0, ଆକାର(sqlite3_mutex_methods));
  }

  ଯଦି( rc==SQLITE_OK ){
    g.isInstalled = isInstall;
  }

  Tcl_SetResult(interp, (ଅକ୍ଷର *)sqlite3ErrName(rc), TCL_VOLATILE);
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** read_mutex_counters
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_read_mutex_counters(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Obj *pRet;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ii;

  ଯଦି( objc!=1 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "");
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  pRet = Tcl_NewObj();
  Tcl_Incr_Ref_ଗଣନା(pRet);
  ପାଇଁ(ii=0; ii<MAX_MUTEXES; ii++){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj(aName[ii], -1));
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewIntObj(g.aCounter[ii]));
  }
  Tcl_SetObjResult(interp, pRet);
  Tcl_Decr_Ref_ଗଣନା(pRet);

  ଫେରନ୍ତୁ TCL_OK;
}

/*
** clear_mutex_counters
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_clear_mutex_counters(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ii;

  ଯଦି( objc!=1 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "");
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  ପାଇଁ(ii=0; ii<MAX_MUTEXES; ii++){
    g.aCounter[ii] = 0;
  }
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** Create and free a mutex.  Return the mutex pointer.  The pointer
** will be invalid since the mutex has already been freed.  The
** return pointer just checks to see if the mutex really was allocated.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_alloc_mutex(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
#ଯଦି SQLITE_THREADSAFE
  sqlite3_mutex *p = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
  ଅକ୍ଷର zBuf[100];
  sqlite3_mutex_free(p);
  sqlite3_snprintf(ଆକାର(zBuf), zBuf, "%p", p);
  Tcl_AppendResult(interp, zBuf, (ଅକ୍ଷର*)0);
#endif
  ଫେରନ୍ତୁ TCL_OK;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_config(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ସଂରଚନା ConfigOption {
    ଅଚଳ ଅକ୍ଷର *zName;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i_ମୂଲ୍ୟ;
  } aOpt[] = {
    {"singlethread", SQLITE_CONFIG_SINGLETHREAD},
    {"multithread",  SQLITE_CONFIG_MULTITHREAD},
    {"serialized",   SQLITE_CONFIG_SERIALIZED},
    {0, 0}
  };
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା s = ଆକାର(ସଂରଚନା ConfigOption);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

  ଯଦି( objc!=2 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "");
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  ଯଦି( Tcl_GetIndexFromObjStruct(interp, objv[1], aOpt, s, "flag", 0, &i) ){
    ଯଦି( Tcl_GetIntFromObj(interp, objv[1], &i) ){
      ଫେରନ୍ତୁ TCL_ERROR;
    }
  }ଅନ୍ୟଥା{
    i = aOpt[i].i_ମୂଲ୍ୟ;
  }

  rc = sqlite3_config(i);
  Tcl_SetResult(interp, (ଅକ୍ଷର *)sqlite3ErrName(rc), TCL_VOLATILE);
  ଫେରନ୍ତୁ TCL_OK;
}

ସ୍ଥିର sqlite3_mutex *getStaticMutexPointer(
  Tcl_Interp *pInterp,
  Tcl_Obj *pObj
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iMutex;
  ଯଦି( Tcl_GetIndexFromObj(pInterp, pObj, aName, "mutex name", 0, &iMutex) ){
    ଫେରନ୍ତୁ 0;
  }
  ପ୍ରତିଜ୍ଞା( iMutex!=SQLITE_MUTEX_FAST && iMutex!=SQLITE_MUTEX_RECURSIVE );
  ଫେରନ୍ତୁ counterMutexAlloc(iMutex);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_enter_static_mutex(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_mutex *pMutex;
  ଯଦି( objc!=2 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "NAME");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  pMutex = getStaticMutexPointer(interp, objv[1]);
  ଯଦି( !pMutex ){
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  sqlite3_mutex_enter(pMutex);
  ଫେରନ୍ତୁ TCL_OK;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_leave_static_mutex(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_mutex *pMutex;
  ଯଦି( objc!=2 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "NAME");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  pMutex = getStaticMutexPointer(interp, objv[1]);
  ଯଦି( !pMutex ){
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  sqlite3_mutex_leave(pMutex);
  ଫେରନ୍ତୁ TCL_OK;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_enter_db_mutex(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  ଯଦି( objc!=2 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "DB");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ){
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( !db ){
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  sqlite3_mutex_enter(sqlite3_db_mutex(db));
  ଫେରନ୍ତୁ TCL_OK;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_leave_db_mutex(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  ଯଦି( objc!=2 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "DB");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ){
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( !db ){
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  sqlite3_mutex_leave(sqlite3_db_mutex(db));
  ଫେରନ୍ତୁ TCL_OK;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା Sqlitetest_mutex_Init(Tcl_Interp *interp){
  ସ୍ଥିର ସଂରଚନା {
    ଅକ୍ଷର *zName;
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
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପାଇଁ(i=0; i<ଆକାର(aCmd)/ଆକାର(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  Tcl_LinkVar(interp, "disable_mutex_init", 
              (ଅକ୍ଷର*)&g.disableInit, TCL_LINK_INT);
  Tcl_LinkVar(interp, "disable_mutex_try", 
              (ଅକ୍ଷର*)&g.disableTry, TCL_LINK_INT);
  ଫେରନ୍ତୁ SQLITE_OK;
}
