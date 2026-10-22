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
#அந்தர்பாவ "tclsqlite.h"
#அந்தர்பாவ "sqlite3.h"
#அந்தர்பாவ "sqliteInt.h"
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <assert.h>
#அந்தர்பாவ <string.h>

#define MAX_MUTEXES        (SQLITE_MUTEX_STATIC_VFS3+1)
#define STATIC_MUTEXES     (MAX_MUTEXES-(SQLITE_MUTEX_RECURSIVE+1))

/* defined in main.c */
வெளி மாறிலி எழுத்து *sqlite3ErrName(முழு);
முழு getDbPointer(Tcl_Interp *interp, மாறிலி எழுத்து *zA, sqlite3 **ppDb);

நிலையான மாறிலி எழுத்து *aName[MAX_MUTEXES+1] = {
  "fast",        "recursive",   "static_main",   "static_mem",
  "static_open", "static_prng", "static_lru",    "static_pmem",
  "static_app1", "static_app2", "static_app3",   "static_vfs1",
  "static_vfs2", "static_vfs3", 0
};

/* A countable mutex */
கட்டமைப்பு sqlite3_mutex {
  sqlite3_mutex *pReal;
  முழு eType;
};

/* State variables */
நிலையான கட்டமைப்பு test_mutex_globals {
  முழு isInstalled;           /* True if installed */
  முழு disableInit;           /* True to cause sqlite3_initialize() to fail */
  முழு disableTry;            /* True to force sqlite3_mutex_try() to fail */
  முழு isInit;                /* True if initialized */
  sqlite3_mutex_methods m;   /* Interface to "real" mutex system */
  முழு aCounter[MAX_MUTEXES]; /* Number of grabs of each type of mutex */
  sqlite3_mutex aStatic[STATIC_MUTEXES]; /* The static mutexes */
} g = {0};

/* Return true if the countable mutex is currently held */
நிலையான முழு counterMutexHeld(sqlite3_mutex *p){
  திரும்பு g.m.xMutexHeld(p->pReal);
}

/* Return true if the countable mutex is not currently held */
நிலையான முழு counterMutexNotheld(sqlite3_mutex *p){
  திரும்பு g.m.xMutexNotheld(p->pReal);
}

/* Initialize the countable mutex interface
** Or, if g.disableInit is non-zero, then do not initialize but instead
** return the value of g.disableInit as the result code.  This can be used
** to simulate an initialization failure.
*/
நிலையான முழு counterMutexInit(வெறுமை){ 
  முழு rc;
  எனில்( g.disableInit ) திரும்பு g.disableInit;
  rc = g.m.xMutexInit();
  g.isInit = 1;
  திரும்பு rc;
}

/*
** Uninitialize the mutex subsystem
*/
நிலையான முழு counterMutexEnd(வெறுமை){ 
  g.isInit = 0;
  திரும்பு g.m.xMutexEnd();
}

/*
** Allocate a countable mutex
*/
நிலையான sqlite3_mutex *counterMutexAlloc(முழு eType){
  sqlite3_mutex *pReal;
  sqlite3_mutex *pRet = 0;

  ப்ரதிஜ்ஞா( g.isInit );
  ப்ரதிஜ்ஞா( eType>=SQLITE_MUTEX_FAST );
  ப்ரதிஜ்ஞா( eType<=SQLITE_MUTEX_STATIC_VFS3 );

  pReal = g.m.xMutexAlloc(eType);
  எனில்( !pReal ) திரும்பு 0;

  எனில்( eType==SQLITE_MUTEX_FAST || eType==SQLITE_MUTEX_RECURSIVE ){
    pRet = (sqlite3_mutex *)malloc(அளவு(sqlite3_mutex));
  }இல்லையெனில்{
    முழு eStaticType = eType - (MAX_MUTEXES - STATIC_MUTEXES);
    ப்ரதிஜ்ஞா( eStaticType>=0 );
    ப்ரதிஜ்ஞா( eStaticType<STATIC_MUTEXES );
    pRet = &g.aStatic[eStaticType];
  }

  pRet->eType = eType;
  pRet->pReal = pReal;
  திரும்பு pRet;
}

/*
** Free a countable mutex
*/
நிலையான வெறுமை counterMutexFree(sqlite3_mutex *p){
  ப்ரதிஜ்ஞா( g.isInit );
  g.m.xMutexFree(p->pReal);
  எனில்( p->eType==SQLITE_MUTEX_FAST || p->eType==SQLITE_MUTEX_RECURSIVE ){
    விடுவி(p);
  }
}

/*
** Enter a countable mutex.  Block until entry is safe.
*/
நிலையான வெறுமை counterMutexEnter(sqlite3_mutex *p){
  ப்ரதிஜ்ஞா( g.isInit );
  ப்ரதிஜ்ஞா( p->eType>=0 );
  ப்ரதிஜ்ஞா( p->eType<MAX_MUTEXES );
  g.aCounter[p->eType]++;
  g.m.xMutexEnter(p->pReal);
}

/*
** Try to enter a mutex.  Return true on success.
*/
நிலையான முழு counterMutexTry(sqlite3_mutex *p){
  ப்ரதிஜ்ஞா( g.isInit );
  ப்ரதிஜ்ஞா( p->eType>=0 );
  ப்ரதிஜ்ஞா( p->eType<MAX_MUTEXES );
  g.aCounter[p->eType]++;
  எனில்( g.disableTry ) திரும்பு SQLITE_BUSY;
  திரும்பு g.m.xMutexTry(p->pReal);
}

/* Leave a mutex
*/
நிலையான வெறுமை counterMutexLeave(sqlite3_mutex *p){
  ப்ரதிஜ்ஞா( g.isInit );
  g.m.xMutexLeave(p->pReal);
}

/*
** sqlite3_shutdown
*/
நிலையான முழு SQLITE_TCLAPI test_shutdown(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  முழு rc;

  எனில்( objc!=1 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "");
    திரும்பு TCL_ERROR;
  }

  rc = sqlite3_shutdown();
  Tcl_SetResult(interp, (எழுத்து *)sqlite3ErrName(rc), TCL_VOLATILE);
  திரும்பு TCL_OK;
}

/*
** sqlite3_initialize
*/
நிலையான முழு SQLITE_TCLAPI test_initialize(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  முழு rc;

  எனில்( objc!=1 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "");
    திரும்பு TCL_ERROR;
  }

  rc = sqlite3_initialize();
  Tcl_SetResult(interp, (எழுத்து *)sqlite3ErrName(rc), TCL_VOLATILE);
  திரும்பு TCL_OK;
}

/*
** install_mutex_counters BOOLEAN
*/
நிலையான முழு SQLITE_TCLAPI test_install_mutex_counters(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  முழு rc = SQLITE_OK;
  முழு isInstall;

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

  எனில்( objc!=2 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "BOOLEAN");
    திரும்பு TCL_ERROR;
  }
  எனில்( TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[1], &isInstall) ){
    திரும்பு TCL_ERROR;
  }

  ப்ரதிஜ்ஞா(isInstall==0 || isInstall==1);
  ப்ரதிஜ்ஞா(g.isInstalled==0 || g.isInstalled==1);
  எனில்( isInstall==g.isInstalled ){
    Tcl_AppendResult(interp, "mutex counters are ", வெற்று);
    Tcl_AppendResult(interp, isInstall?"already installed":"not installed", வெற்று);
    திரும்பு TCL_ERROR;
  }

  எனில்( isInstall ){
    ப்ரதிஜ்ஞா( g.m.xMutexAlloc==0 );
    rc = sqlite3_config(SQLITE_CONFIG_GETMUTEX, &g.m);
    எனில்( rc==SQLITE_OK ){
      sqlite3_config(SQLITE_CONFIG_MUTEX, &counter_methods);
    }
    g.disableTry = 0;
  }இல்லையெனில்{
    ப்ரதிஜ்ஞா( g.m.xMutexAlloc );
    rc = sqlite3_config(SQLITE_CONFIG_MUTEX, &g.m);
    memset(&g.m, 0, அளவு(sqlite3_mutex_methods));
  }

  எனில்( rc==SQLITE_OK ){
    g.isInstalled = isInstall;
  }

  Tcl_SetResult(interp, (எழுத்து *)sqlite3ErrName(rc), TCL_VOLATILE);
  திரும்பு TCL_OK;
}

/*
** read_mutex_counters
*/
நிலையான முழு SQLITE_TCLAPI test_read_mutex_counters(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Obj *pRet;
  முழு ii;

  எனில்( objc!=1 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "");
    திரும்பு TCL_ERROR;
  }

  pRet = Tcl_NewObj();
  Tcl_Incr_Ref_மொத்தம்(pRet);
  க்கு(ii=0; ii<MAX_MUTEXES; ii++){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj(aName[ii], -1));
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewIntObj(g.aCounter[ii]));
  }
  Tcl_SetObjResult(interp, pRet);
  Tcl_Decr_Ref_மொத்தம்(pRet);

  திரும்பு TCL_OK;
}

/*
** clear_mutex_counters
*/
நிலையான முழு SQLITE_TCLAPI test_clear_mutex_counters(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  முழு ii;

  எனில்( objc!=1 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "");
    திரும்பு TCL_ERROR;
  }

  க்கு(ii=0; ii<MAX_MUTEXES; ii++){
    g.aCounter[ii] = 0;
  }
  திரும்பு TCL_OK;
}

/*
** Create and free a mutex.  Return the mutex pointer.  The pointer
** will be invalid since the mutex has already been freed.  The
** return pointer just checks to see if the mutex really was allocated.
*/
நிலையான முழு SQLITE_TCLAPI test_alloc_mutex(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
#எனில் SQLITE_THREADSAFE
  sqlite3_mutex *p = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
  எழுத்து zBuf[100];
  sqlite3_mutex_free(p);
  sqlite3_snprintf(அளவு(zBuf), zBuf, "%p", p);
  Tcl_AppendResult(interp, zBuf, (எழுத்து*)0);
#endif
  திரும்பு TCL_OK;
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
நிலையான முழு SQLITE_TCLAPI test_config(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  கட்டமைப்பு ConfigOption {
    மாறிலி எழுத்து *zName;
    முழு i_மதிப்பு;
  } aOpt[] = {
    {"singlethread", SQLITE_CONFIG_SINGLETHREAD},
    {"multithread",  SQLITE_CONFIG_MULTITHREAD},
    {"serialized",   SQLITE_CONFIG_SERIALIZED},
    {0, 0}
  };
  முழு s = அளவு(கட்டமைப்பு ConfigOption);
  முழு i;
  முழு rc;

  எனில்( objc!=2 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "");
    திரும்பு TCL_ERROR;
  }

  எனில்( Tcl_GetIndexFromObjStruct(interp, objv[1], aOpt, s, "flag", 0, &i) ){
    எனில்( Tcl_GetIntFromObj(interp, objv[1], &i) ){
      திரும்பு TCL_ERROR;
    }
  }இல்லையெனில்{
    i = aOpt[i].i_மதிப்பு;
  }

  rc = sqlite3_config(i);
  Tcl_SetResult(interp, (எழுத்து *)sqlite3ErrName(rc), TCL_VOLATILE);
  திரும்பு TCL_OK;
}

நிலையான sqlite3_mutex *getStaticMutexPointer(
  Tcl_Interp *pInterp,
  Tcl_Obj *pObj
){
  முழு iMutex;
  எனில்( Tcl_GetIndexFromObj(pInterp, pObj, aName, "mutex name", 0, &iMutex) ){
    திரும்பு 0;
  }
  ப்ரதிஜ்ஞா( iMutex!=SQLITE_MUTEX_FAST && iMutex!=SQLITE_MUTEX_RECURSIVE );
  திரும்பு counterMutexAlloc(iMutex);
}

நிலையான முழு SQLITE_TCLAPI test_enter_static_mutex(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_mutex *pMutex;
  எனில்( objc!=2 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "NAME");
    திரும்பு TCL_ERROR;
  }
  pMutex = getStaticMutexPointer(interp, objv[1]);
  எனில்( !pMutex ){
    திரும்பு TCL_ERROR;
  }
  sqlite3_mutex_enter(pMutex);
  திரும்பு TCL_OK;
}

நிலையான முழு SQLITE_TCLAPI test_leave_static_mutex(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_mutex *pMutex;
  எனில்( objc!=2 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "NAME");
    திரும்பு TCL_ERROR;
  }
  pMutex = getStaticMutexPointer(interp, objv[1]);
  எனில்( !pMutex ){
    திரும்பு TCL_ERROR;
  }
  sqlite3_mutex_leave(pMutex);
  திரும்பு TCL_OK;
}

நிலையான முழு SQLITE_TCLAPI test_enter_db_mutex(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  எனில்( objc!=2 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "DB");
    திரும்பு TCL_ERROR;
  }
  எனில்( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ){
    திரும்பு TCL_ERROR;
  }
  எனில்( !db ){
    திரும்பு TCL_ERROR;
  }
  sqlite3_mutex_enter(sqlite3_db_mutex(db));
  திரும்பு TCL_OK;
}

நிலையான முழு SQLITE_TCLAPI test_leave_db_mutex(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  எனில்( objc!=2 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "DB");
    திரும்பு TCL_ERROR;
  }
  எனில்( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ){
    திரும்பு TCL_ERROR;
  }
  எனில்( !db ){
    திரும்பு TCL_ERROR;
  }
  sqlite3_mutex_leave(sqlite3_db_mutex(db));
  திரும்பு TCL_OK;
}

முழு Sqlitetest_mutex_Init(Tcl_Interp *interp){
  நிலையான கட்டமைப்பு {
    எழுத்து *zName;
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
  முழு i;
  க்கு(i=0; i<அளவு(aCmd)/அளவு(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  Tcl_LinkVar(interp, "disable_mutex_init", 
              (எழுத்து*)&g.disableInit, TCL_LINK_INT);
  Tcl_LinkVar(interp, "disable_mutex_try", 
              (எழுத்து*)&g.disableTry, TCL_LINK_INT);
  திரும்பு SQLITE_OK;
}
