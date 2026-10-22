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
#અન્તર્ભાવ "tclsqlite.h"
#અન્તર્ભાવ "sqlite3.h"
#અન્તર્ભાવ "sqliteInt.h"
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <assert.h>
#અન્તર્ભાવ <string.h>

#define MAX_MUTEXES        (SQLITE_MUTEX_STATIC_VFS3+1)
#define STATIC_MUTEXES     (MAX_MUTEXES-(SQLITE_MUTEX_RECURSIVE+1))

/* defined in main.c */
બાહ્ય અચલ અક્ષર *sqlite3ErrName(પૂર્ણાંક);
પૂર્ણાંક getDbPointer(Tcl_Interp *interp, અચલ અક્ષર *zA, sqlite3 **ppDb);

સ્થિર અચલ અક્ષર *aName[MAX_MUTEXES+1] = {
  "fast",        "recursive",   "static_main",   "static_mem",
  "static_open", "static_prng", "static_lru",    "static_pmem",
  "static_app1", "static_app2", "static_app3",   "static_vfs1",
  "static_vfs2", "static_vfs3", 0
};

/* A countable mutex */
સંરચના sqlite3_mutex {
  sqlite3_mutex *pReal;
  પૂર્ણાંક eType;
};

/* State variables */
સ્થિર સંરચના test_mutex_globals {
  પૂર્ણાંક isInstalled;           /* True if installed */
  પૂર્ણાંક disableInit;           /* True to cause sqlite3_initialize() to fail */
  પૂર્ણાંક disableTry;            /* True to force sqlite3_mutex_try() to fail */
  પૂર્ણાંક isInit;                /* True if initialized */
  sqlite3_mutex_methods m;   /* Interface to "real" mutex system */
  પૂર્ણાંક aCounter[MAX_MUTEXES]; /* Number of grabs of each type of mutex */
  sqlite3_mutex aStatic[STATIC_MUTEXES]; /* The static mutexes */
} g = {0};

/* Return true if the countable mutex is currently held */
સ્થિર પૂર્ણાંક counterMutexHeld(sqlite3_mutex *p){
  પાછા g.m.xMutexHeld(p->pReal);
}

/* Return true if the countable mutex is not currently held */
સ્થિર પૂર્ણાંક counterMutexNotheld(sqlite3_mutex *p){
  પાછા g.m.xMutexNotheld(p->pReal);
}

/* Initialize the countable mutex interface
** Or, if g.disableInit is non-zero, then do not initialize but instead
** return the value of g.disableInit as the result code.  This can be used
** to simulate an initialization failure.
*/
સ્થિર પૂર્ણાંક counterMutexInit(શૂન્ય){ 
  પૂર્ણાંક rc;
  જો( g.disableInit ) પાછા g.disableInit;
  rc = g.m.xMutexInit();
  g.isInit = 1;
  પાછા rc;
}

/*
** Uninitialize the mutex subsystem
*/
સ્થિર પૂર્ણાંક counterMutexEnd(શૂન્ય){ 
  g.isInit = 0;
  પાછા g.m.xMutexEnd();
}

/*
** Allocate a countable mutex
*/
સ્થિર sqlite3_mutex *counterMutexAlloc(પૂર્ણાંક eType){
  sqlite3_mutex *pReal;
  sqlite3_mutex *pRet = 0;

  પ્રતિજ્ઞા( g.isInit );
  પ્રતિજ્ઞા( eType>=SQLITE_MUTEX_FAST );
  પ્રતિજ્ઞા( eType<=SQLITE_MUTEX_STATIC_VFS3 );

  pReal = g.m.xMutexAlloc(eType);
  જો( !pReal ) પાછા 0;

  જો( eType==SQLITE_MUTEX_FAST || eType==SQLITE_MUTEX_RECURSIVE ){
    pRet = (sqlite3_mutex *)malloc(કદ(sqlite3_mutex));
  }નહીંતર{
    પૂર્ણાંક eStaticType = eType - (MAX_MUTEXES - STATIC_MUTEXES);
    પ્રતિજ્ઞા( eStaticType>=0 );
    પ્રતિજ્ઞા( eStaticType<STATIC_MUTEXES );
    pRet = &g.aStatic[eStaticType];
  }

  pRet->eType = eType;
  pRet->pReal = pReal;
  પાછા pRet;
}

/*
** Free a countable mutex
*/
સ્થિર શૂન્ય counterMutexFree(sqlite3_mutex *p){
  પ્રતિજ્ઞા( g.isInit );
  g.m.xMutexFree(p->pReal);
  જો( p->eType==SQLITE_MUTEX_FAST || p->eType==SQLITE_MUTEX_RECURSIVE ){
    મુક્ત(p);
  }
}

/*
** Enter a countable mutex.  Block until entry is safe.
*/
સ્થિર શૂન્ય counterMutexEnter(sqlite3_mutex *p){
  પ્રતિજ્ઞા( g.isInit );
  પ્રતિજ્ઞા( p->eType>=0 );
  પ્રતિજ્ઞા( p->eType<MAX_MUTEXES );
  g.aCounter[p->eType]++;
  g.m.xMutexEnter(p->pReal);
}

/*
** Try to enter a mutex.  Return true on success.
*/
સ્થિર પૂર્ણાંક counterMutexTry(sqlite3_mutex *p){
  પ્રતિજ્ઞા( g.isInit );
  પ્રતિજ્ઞા( p->eType>=0 );
  પ્રતિજ્ઞા( p->eType<MAX_MUTEXES );
  g.aCounter[p->eType]++;
  જો( g.disableTry ) પાછા SQLITE_BUSY;
  પાછા g.m.xMutexTry(p->pReal);
}

/* Leave a mutex
*/
સ્થિર શૂન્ય counterMutexLeave(sqlite3_mutex *p){
  પ્રતિજ્ઞા( g.isInit );
  g.m.xMutexLeave(p->pReal);
}

/*
** sqlite3_shutdown
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_shutdown(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  પૂર્ણાંક rc;

  જો( objc!=1 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "");
    પાછા TCL_ERROR;
  }

  rc = sqlite3_shutdown();
  Tcl_SetResult(interp, (અક્ષર *)sqlite3ErrName(rc), TCL_VOLATILE);
  પાછા TCL_OK;
}

/*
** sqlite3_initialize
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_initialize(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  પૂર્ણાંક rc;

  જો( objc!=1 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "");
    પાછા TCL_ERROR;
  }

  rc = sqlite3_initialize();
  Tcl_SetResult(interp, (અક્ષર *)sqlite3ErrName(rc), TCL_VOLATILE);
  પાછા TCL_OK;
}

/*
** install_mutex_counters BOOLEAN
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_install_mutex_counters(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  પૂર્ણાંક rc = SQLITE_OK;
  પૂર્ણાંક isInstall;

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

  જો( objc!=2 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "BOOLEAN");
    પાછા TCL_ERROR;
  }
  જો( TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[1], &isInstall) ){
    પાછા TCL_ERROR;
  }

  પ્રતિજ્ઞા(isInstall==0 || isInstall==1);
  પ્રતિજ્ઞા(g.isInstalled==0 || g.isInstalled==1);
  જો( isInstall==g.isInstalled ){
    Tcl_AppendResult(interp, "mutex counters are ", ખાલી);
    Tcl_AppendResult(interp, isInstall?"already installed":"not installed", ખાલી);
    પાછા TCL_ERROR;
  }

  જો( isInstall ){
    પ્રતિજ્ઞા( g.m.xMutexAlloc==0 );
    rc = sqlite3_config(SQLITE_CONFIG_GETMUTEX, &g.m);
    જો( rc==SQLITE_OK ){
      sqlite3_config(SQLITE_CONFIG_MUTEX, &counter_methods);
    }
    g.disableTry = 0;
  }નહીંતર{
    પ્રતિજ્ઞા( g.m.xMutexAlloc );
    rc = sqlite3_config(SQLITE_CONFIG_MUTEX, &g.m);
    memset(&g.m, 0, કદ(sqlite3_mutex_methods));
  }

  જો( rc==SQLITE_OK ){
    g.isInstalled = isInstall;
  }

  Tcl_SetResult(interp, (અક્ષર *)sqlite3ErrName(rc), TCL_VOLATILE);
  પાછા TCL_OK;
}

/*
** read_mutex_counters
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_read_mutex_counters(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Obj *pRet;
  પૂર્ણાંક ii;

  જો( objc!=1 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "");
    પાછા TCL_ERROR;
  }

  pRet = Tcl_NewObj();
  Tcl_Incr_Ref_ગણતરી(pRet);
  માટે(ii=0; ii<MAX_MUTEXES; ii++){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj(aName[ii], -1));
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewIntObj(g.aCounter[ii]));
  }
  Tcl_SetObjResult(interp, pRet);
  Tcl_Decr_Ref_ગણતરી(pRet);

  પાછા TCL_OK;
}

/*
** clear_mutex_counters
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_clear_mutex_counters(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  પૂર્ણાંક ii;

  જો( objc!=1 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "");
    પાછા TCL_ERROR;
  }

  માટે(ii=0; ii<MAX_MUTEXES; ii++){
    g.aCounter[ii] = 0;
  }
  પાછા TCL_OK;
}

/*
** Create and free a mutex.  Return the mutex pointer.  The pointer
** will be invalid since the mutex has already been freed.  The
** return pointer just checks to see if the mutex really was allocated.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_alloc_mutex(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
#જો SQLITE_THREADSAFE
  sqlite3_mutex *p = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
  અક્ષર zBuf[100];
  sqlite3_mutex_free(p);
  sqlite3_snprintf(કદ(zBuf), zBuf, "%p", p);
  Tcl_AppendResult(interp, zBuf, (અક્ષર*)0);
#endif
  પાછા TCL_OK;
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
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_config(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  સંરચના ConfigOption {
    અચલ અક્ષર *zName;
    પૂર્ણાંક i_કિંમત;
  } aOpt[] = {
    {"singlethread", SQLITE_CONFIG_SINGLETHREAD},
    {"multithread",  SQLITE_CONFIG_MULTITHREAD},
    {"serialized",   SQLITE_CONFIG_SERIALIZED},
    {0, 0}
  };
  પૂર્ણાંક s = કદ(સંરચના ConfigOption);
  પૂર્ણાંક i;
  પૂર્ણાંક rc;

  જો( objc!=2 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "");
    પાછા TCL_ERROR;
  }

  જો( Tcl_GetIndexFromObjStruct(interp, objv[1], aOpt, s, "flag", 0, &i) ){
    જો( Tcl_GetIntFromObj(interp, objv[1], &i) ){
      પાછા TCL_ERROR;
    }
  }નહીંતર{
    i = aOpt[i].i_કિંમત;
  }

  rc = sqlite3_config(i);
  Tcl_SetResult(interp, (અક્ષર *)sqlite3ErrName(rc), TCL_VOLATILE);
  પાછા TCL_OK;
}

સ્થિર sqlite3_mutex *getStaticMutexPointer(
  Tcl_Interp *pInterp,
  Tcl_Obj *pObj
){
  પૂર્ણાંક iMutex;
  જો( Tcl_GetIndexFromObj(pInterp, pObj, aName, "mutex name", 0, &iMutex) ){
    પાછા 0;
  }
  પ્રતિજ્ઞા( iMutex!=SQLITE_MUTEX_FAST && iMutex!=SQLITE_MUTEX_RECURSIVE );
  પાછા counterMutexAlloc(iMutex);
}

સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_enter_static_mutex(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_mutex *pMutex;
  જો( objc!=2 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "NAME");
    પાછા TCL_ERROR;
  }
  pMutex = getStaticMutexPointer(interp, objv[1]);
  જો( !pMutex ){
    પાછા TCL_ERROR;
  }
  sqlite3_mutex_enter(pMutex);
  પાછા TCL_OK;
}

સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_leave_static_mutex(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_mutex *pMutex;
  જો( objc!=2 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "NAME");
    પાછા TCL_ERROR;
  }
  pMutex = getStaticMutexPointer(interp, objv[1]);
  જો( !pMutex ){
    પાછા TCL_ERROR;
  }
  sqlite3_mutex_leave(pMutex);
  પાછા TCL_OK;
}

સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_enter_db_mutex(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  જો( objc!=2 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "DB");
    પાછા TCL_ERROR;
  }
  જો( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ){
    પાછા TCL_ERROR;
  }
  જો( !db ){
    પાછા TCL_ERROR;
  }
  sqlite3_mutex_enter(sqlite3_db_mutex(db));
  પાછા TCL_OK;
}

સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_leave_db_mutex(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  જો( objc!=2 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "DB");
    પાછા TCL_ERROR;
  }
  જો( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ){
    પાછા TCL_ERROR;
  }
  જો( !db ){
    પાછા TCL_ERROR;
  }
  sqlite3_mutex_leave(sqlite3_db_mutex(db));
  પાછા TCL_OK;
}

પૂર્ણાંક Sqlitetest_mutex_Init(Tcl_Interp *interp){
  સ્થિર સંરચના {
    અક્ષર *zName;
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
  પૂર્ણાંક i;
  માટે(i=0; i<કદ(aCmd)/કદ(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  Tcl_LinkVar(interp, "disable_mutex_init", 
              (અક્ષર*)&g.disableInit, TCL_LINK_INT);
  Tcl_LinkVar(interp, "disable_mutex_try", 
              (અક્ષર*)&g.disableTry, TCL_LINK_INT);
  પાછા SQLITE_OK;
}
