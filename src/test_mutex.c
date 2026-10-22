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
#समावेश "tclsqlite.h"
#समावेश "sqlite3.h"
#समावेश "sqliteInt.h"
#समावेश <stdlib.h>
#समावेश <assert.h>
#समावेश <string.h>

#घोषणा MAX_MUTEXES        (SQLITE_MUTEX_STATIC_VFS3+1)
#घोषणा STATIC_MUTEXES     (MAX_MUTEXES-(SQLITE_MUTEX_RECURSIVE+1))

/* defined in main.c */
बाह्य स्थिर अक्षर *sqlite3ErrName(पूर्णांक);
पूर्णांक getDbPointer(Tcl_Interp *interp, स्थिर अक्षर *zA, sqlite3 **ppDb);

अटल स्थिर अक्षर *aName[MAX_MUTEXES+1] = {
  "fast",        "recursive",   "static_main",   "static_mem",
  "static_open", "static_prng", "static_lru",    "static_pmem",
  "static_app1", "static_app2", "static_app3",   "static_vfs1",
  "static_vfs2", "static_vfs3", 0
};

/* A countable mutex */
काष्ठा sqlite3_mutex {
  sqlite3_mutex *pReal;
  पूर्णांक eType;
};

/* State variables */
अटल काष्ठा test_mutex_globals {
  पूर्णांक isInstalled;           /* True if installed */
  पूर्णांक disableInit;           /* True to cause sqlite3_initialize() to fail */
  पूर्णांक disableTry;            /* True to force sqlite3_mutex_try() to fail */
  पूर्णांक isInit;                /* True if initialized */
  sqlite3_mutex_methods m;   /* Interface to "real" mutex system */
  पूर्णांक a_गणक[MAX_MUTEXES]; /* Number of grabs of each type of mutex */
  sqlite3_mutex aStatic[STATIC_MUTEXES]; /* The static mutexes */
} g = {0};

/* Return true if the countable mutex is currently held */
अटल पूर्णांक गणक_Mutex_Held(sqlite3_mutex *p){
  वापस g.m.xMutexHeld(p->pReal);
}

/* Return true if the countable mutex is not currently held */
अटल पूर्णांक गणक_Mutex_Notheld(sqlite3_mutex *p){
  वापस g.m.xMutexNotheld(p->pReal);
}

/* Initialize the countable mutex interface
** Or, if g.disableInit is non-zero, then do not initialize but instead
** return the value of g.disableInit as the result code.  This can be used
** to simulate an initialization failure.
*/
अटल पूर्णांक गणक_Mutex_Init(व्योम){ 
  पूर्णांक rc;
  अगर( g.disableInit ) वापस g.disableInit;
  rc = g.m.xMutexInit();
  g.isInit = 1;
  वापस rc;
}

/*
** Uninitialize the mutex subsystem
*/
अटल पूर्णांक गणक_Mutex_End(व्योम){ 
  g.isInit = 0;
  वापस g.m.xMutexEnd();
}

/*
** Allocate a countable mutex
*/
अटल sqlite3_mutex *गणक_Mutex_Alloc(पूर्णांक eType){
  sqlite3_mutex *pReal;
  sqlite3_mutex *pRet = 0;

  निश्चित( g.isInit );
  निश्चित( eType>=SQLITE_MUTEX_FAST );
  निश्चित( eType<=SQLITE_MUTEX_STATIC_VFS3 );

  pReal = g.m.xMutexAlloc(eType);
  अगर( !pReal ) वापस 0;

  अगर( eType==SQLITE_MUTEX_FAST || eType==SQLITE_MUTEX_RECURSIVE ){
    pRet = (sqlite3_mutex *)दो_स्मृति(माप(sqlite3_mutex));
  }अन्यथा{
    पूर्णांक eStaticType = eType - (MAX_MUTEXES - STATIC_MUTEXES);
    निश्चित( eStaticType>=0 );
    निश्चित( eStaticType<STATIC_MUTEXES );
    pRet = &g.aStatic[eStaticType];
  }

  pRet->eType = eType;
  pRet->pReal = pReal;
  वापस pRet;
}

/*
** Free a countable mutex
*/
अटल व्योम गणक_Mutex_Free(sqlite3_mutex *p){
  निश्चित( g.isInit );
  g.m.xMutexFree(p->pReal);
  अगर( p->eType==SQLITE_MUTEX_FAST || p->eType==SQLITE_MUTEX_RECURSIVE ){
    मुक्त(p);
  }
}

/*
** Enter a countable mutex.  Block until entry is safe.
*/
अटल व्योम गणक_Mutex_Enter(sqlite3_mutex *p){
  निश्चित( g.isInit );
  निश्चित( p->eType>=0 );
  निश्चित( p->eType<MAX_MUTEXES );
  g.a_गणक[p->eType]++;
  g.m.xMutexEnter(p->pReal);
}

/*
** Try to enter a mutex.  Return true on success.
*/
अटल पूर्णांक गणक_Mutex_Try(sqlite3_mutex *p){
  निश्चित( g.isInit );
  निश्चित( p->eType>=0 );
  निश्चित( p->eType<MAX_MUTEXES );
  g.a_गणक[p->eType]++;
  अगर( g.disableTry ) वापस SQLITE_BUSY;
  वापस g.m.xMutexTry(p->pReal);
}

/* Leave a mutex
*/
अटल व्योम गणक_Mutex_Leave(sqlite3_mutex *p){
  निश्चित( g.isInit );
  g.m.xMutexLeave(p->pReal);
}

/*
** sqlite3_shutdown
*/
अटल पूर्णांक SQLITE_TCLAPI test_shutdown(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक rc;

  अगर( objc!=1 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "");
    वापस TCL_ERROR;
  }

  rc = sqlite3_shutdown();
  Tcl_Set_परिणाम(interp, (अक्षर *)sqlite3ErrName(rc), TCL_VOLATILE);
  वापस TCL_OK;
}

/*
** sqlite3_initialize
*/
अटल पूर्णांक SQLITE_TCLAPI test_initialize(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक rc;

  अगर( objc!=1 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "");
    वापस TCL_ERROR;
  }

  rc = sqlite3_initialize();
  Tcl_Set_परिणाम(interp, (अक्षर *)sqlite3ErrName(rc), TCL_VOLATILE);
  वापस TCL_OK;
}

/*
** install_mutex_counters BOOLEAN
*/
अटल पूर्णांक SQLITE_TCLAPI test_install_mutex_counters(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक rc = SQLITE_OK;
  पूर्णांक isInstall;

  sqlite3_mutex_methods गणक_methods = {
    गणक_Mutex_Init,
    गणक_Mutex_End,
    गणक_Mutex_Alloc,
    गणक_Mutex_Free,
    गणक_Mutex_Enter,
    गणक_Mutex_Try,
    गणक_Mutex_Leave,
    गणक_Mutex_Held,
    गणक_Mutex_Notheld
  };

  अगर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "BOOLEAN");
    वापस TCL_ERROR;
  }
  अगर( TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[1], &isInstall) ){
    वापस TCL_ERROR;
  }

  निश्चित(isInstall==0 || isInstall==1);
  निश्चित(g.isInstalled==0 || g.isInstalled==1);
  अगर( isInstall==g.isInstalled ){
    Tcl_Append_परिणाम(interp, "mutex counters are ", शून्य);
    Tcl_Append_परिणाम(interp, isInstall?"already installed":"not installed", शून्य);
    वापस TCL_ERROR;
  }

  अगर( isInstall ){
    निश्चित( g.m.xMutexAlloc==0 );
    rc = sqlite3_config(SQLITE_CONFIG_GETMUTEX, &g.m);
    अगर( rc==SQLITE_OK ){
      sqlite3_config(SQLITE_CONFIG_MUTEX, &गणक_methods);
    }
    g.disableTry = 0;
  }अन्यथा{
    निश्चित( g.m.xMutexAlloc );
    rc = sqlite3_config(SQLITE_CONFIG_MUTEX, &g.m);
    स_रखो(&g.m, 0, माप(sqlite3_mutex_methods));
  }

  अगर( rc==SQLITE_OK ){
    g.isInstalled = isInstall;
  }

  Tcl_Set_परिणाम(interp, (अक्षर *)sqlite3ErrName(rc), TCL_VOLATILE);
  वापस TCL_OK;
}

/*
** read_mutex_counters
*/
अटल पूर्णांक SQLITE_TCLAPI test_read_mutex_counters(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Obj *pRet;
  पूर्णांक ii;

  अगर( objc!=1 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "");
    वापस TCL_ERROR;
  }

  pRet = Tcl_NewObj();
  Tcl_Incr_Ref_गिनती(pRet);
  क्रम(ii=0; ii<MAX_MUTEXES; ii++){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj(aName[ii], -1));
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewIntObj(g.a_गणक[ii]));
  }
  Tcl_Set_Obj_परिणाम(interp, pRet);
  Tcl_Decr_Ref_गिनती(pRet);

  वापस TCL_OK;
}

/*
** clear_mutex_counters
*/
अटल पूर्णांक SQLITE_TCLAPI test_clear_mutex_counters(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक ii;

  अगर( objc!=1 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "");
    वापस TCL_ERROR;
  }

  क्रम(ii=0; ii<MAX_MUTEXES; ii++){
    g.a_गणक[ii] = 0;
  }
  वापस TCL_OK;
}

/*
** Create and free a mutex.  Return the mutex pointer.  The pointer
** will be invalid since the mutex has already been freed.  The
** return pointer just checks to see if the mutex really was allocated.
*/
अटल पूर्णांक SQLITE_TCLAPI test_alloc_mutex(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
#अगर SQLITE_THREADSAFE
  sqlite3_mutex *p = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
  अक्षर zBuf[100];
  sqlite3_mutex_free(p);
  sqlite3_snprintf(माप(zBuf), zBuf, "%p", p);
  Tcl_Append_परिणाम(interp, zBuf, (अक्षर*)0);
#पूर्ण_अगर
  वापस TCL_OK;
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
अटल पूर्णांक SQLITE_TCLAPI test_config(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  काष्ठा ConfigOption {
    स्थिर अक्षर *zName;
    पूर्णांक i_मूल्य;
  } aOpt[] = {
    {"singlethread", SQLITE_CONFIG_SINGLETHREAD},
    {"multithread",  SQLITE_CONFIG_MULTITHREAD},
    {"serialized",   SQLITE_CONFIG_SERIALIZED},
    {0, 0}
  };
  पूर्णांक s = माप(काष्ठा ConfigOption);
  पूर्णांक i;
  पूर्णांक rc;

  अगर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "");
    वापस TCL_ERROR;
  }

  अगर( Tcl_Get_सूचकांक_From_Obj_Struct(interp, objv[1], aOpt, s, "flag", 0, &i) ){
    अगर( Tcl_GetIntFromObj(interp, objv[1], &i) ){
      वापस TCL_ERROR;
    }
  }अन्यथा{
    i = aOpt[i].i_मूल्य;
  }

  rc = sqlite3_config(i);
  Tcl_Set_परिणाम(interp, (अक्षर *)sqlite3ErrName(rc), TCL_VOLATILE);
  वापस TCL_OK;
}

अटल sqlite3_mutex *getStaticMutexPointer(
  Tcl_Interp *pInterp,
  Tcl_Obj *pObj
){
  पूर्णांक iMutex;
  अगर( Tcl_Get_सूचकांक_From_Obj(pInterp, pObj, aName, "mutex name", 0, &iMutex) ){
    वापस 0;
  }
  निश्चित( iMutex!=SQLITE_MUTEX_FAST && iMutex!=SQLITE_MUTEX_RECURSIVE );
  वापस गणक_Mutex_Alloc(iMutex);
}

अटल पूर्णांक SQLITE_TCLAPI test_enter_static_mutex(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_mutex *pMutex;
  अगर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "NAME");
    वापस TCL_ERROR;
  }
  pMutex = getStaticMutexPointer(interp, objv[1]);
  अगर( !pMutex ){
    वापस TCL_ERROR;
  }
  sqlite3_mutex_enter(pMutex);
  वापस TCL_OK;
}

अटल पूर्णांक SQLITE_TCLAPI test_leave_static_mutex(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_mutex *pMutex;
  अगर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "NAME");
    वापस TCL_ERROR;
  }
  pMutex = getStaticMutexPointer(interp, objv[1]);
  अगर( !pMutex ){
    वापस TCL_ERROR;
  }
  sqlite3_mutex_leave(pMutex);
  वापस TCL_OK;
}

अटल पूर्णांक SQLITE_TCLAPI test_enter_db_mutex(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  अगर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "DB");
    वापस TCL_ERROR;
  }
  अगर( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ){
    वापस TCL_ERROR;
  }
  अगर( !db ){
    वापस TCL_ERROR;
  }
  sqlite3_mutex_enter(sqlite3_db_mutex(db));
  वापस TCL_OK;
}

अटल पूर्णांक SQLITE_TCLAPI test_leave_db_mutex(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  अगर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "DB");
    वापस TCL_ERROR;
  }
  अगर( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ){
    वापस TCL_ERROR;
  }
  अगर( !db ){
    वापस TCL_ERROR;
  }
  sqlite3_mutex_leave(sqlite3_db_mutex(db));
  वापस TCL_OK;
}

पूर्णांक Sqlitetest_mutex_Init(Tcl_Interp *interp){
  अटल काष्ठा {
    अक्षर *zName;
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
  पूर्णांक i;
  क्रम(i=0; i<माप(aCmd)/माप(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  Tcl_LinkVar(interp, "disable_mutex_init", 
              (अक्षर*)&g.disableInit, TCL_LINK_INT);
  Tcl_LinkVar(interp, "disable_mutex_try", 
              (अक्षर*)&g.disableTry, TCL_LINK_INT);
  वापस SQLITE_OK;
}
