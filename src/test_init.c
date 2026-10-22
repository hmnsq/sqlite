/*
** 2009 August 17
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
**
** The code in this file is used for testing SQLite. It is not part of
** the source code used in production systems.
**
** Specifically, this file tests the effect of errors while initializing
** the various pluggable sub-systems from within sqlite3_initialize().
** If an error occurs in sqlite3_initialize() the following should be
** true:
**
**   1) An error code is returned to the user, and
**   2) A subsequent call to sqlite3_shutdown() calls the shutdown method
**      of those subsystems that were initialized, and
**   3) A subsequent call to sqlite3_initialize() attempts to initialize
**      the remaining, uninitialized, subsystems.
*/

#شامل "sqliteInt.h"
#شامل <string.h>
#شامل "tclsqlite.h"

ایستا ساختار Wrapped {
  sqlite3_pcache_methods2 pcache;
  sqlite3_mem_methods     mem;
  sqlite3_mutex_methods   mutex;

  صحیح mem_init;              /* True if mem subsystem is initialized */
  صحیح mem_fail;              /* True to fail mem subsystem initialization */
  صحیح mutex_init;            /* True if mutex subsystem is initialized */
  صحیح mutex_fail;            /* True to fail mutex subsystem initialization */
  صحیح pcache_init;           /* True if pcache subsystem is initialized */
  صحیح pcache_fail;           /* True to fail pcache subsystem initialization */
} wrapped;

ایستا صحیح wrMemInit(تهی *pAppData){
  صحیح rc;
  اگر( wrapped.mem_fail ){
    rc = SQLITE_ERROR;
  }وگرنه{
    rc = wrapped.mem.xInit(wrapped.mem.pAppData);
  }
  اگر( rc==SQLITE_OK ){
    wrapped.mem_init = 1;
  }
  بازگشت rc;
}
ایستا تهی wrMemShutdown(تهی *pAppData){
  wrapped.mem.xShutdown(wrapped.mem.pAppData);
  wrapped.mem_init = 0;
}
ایستا تهی *wrMemMalloc(صحیح n)           {بازگشت wrapped.mem.xMalloc(n);}
ایستا تهی wrMemFree(تهی *p)            {wrapped.mem.xFree(p);}
ایستا تهی *wrMemRealloc(تهی *p, صحیح n) {بازگشت wrapped.mem.xRealloc(p, n);}
ایستا صحیح wrMemSize(تهی *p)             {بازگشت wrapped.mem.xSize(p);}
ایستا صحیح wrMemRoundup(صحیح n)            {بازگشت wrapped.mem.xRoundup(n);}


ایستا صحیح wrMutexInit(تهی){
  صحیح rc;
  اگر( wrapped.mutex_fail ){
    rc = SQLITE_ERROR;
  }وگرنه{
    rc = wrapped.mutex.xMutexInit();
  }
  اگر( rc==SQLITE_OK ){
    wrapped.mutex_init = 1;
  }
  بازگشت rc;
}
ایستا صحیح wrMutexEnd(تهی){
  wrapped.mutex.xMutexEnd();
  wrapped.mutex_init = 0;
  بازگشت SQLITE_OK;
}
ایستا sqlite3_mutex *wrMutexAlloc(صحیح e){
  بازگشت wrapped.mutex.xMutexAlloc(e);
}
ایستا تهی wrMutexFree(sqlite3_mutex *p){
  wrapped.mutex.xMutexFree(p);
}
ایستا تهی wrMutexEnter(sqlite3_mutex *p){
  wrapped.mutex.xMutexEnter(p);
}
ایستا صحیح wrMutexTry(sqlite3_mutex *p){
  بازگشت wrapped.mutex.xMutexTry(p);
}
ایستا تهی wrMutexLeave(sqlite3_mutex *p){
  wrapped.mutex.xMutexLeave(p);
}
ایستا صحیح wrMutexHeld(sqlite3_mutex *p){
  بازگشت wrapped.mutex.xMutexHeld(p);
}
ایستا صحیح wrMutexNotheld(sqlite3_mutex *p){
  بازگشت wrapped.mutex.xMutexNotheld(p);
}



ایستا صحیح wrPCacheInit(تهی *pArg){
  صحیح rc;
  اگر( wrapped.pcache_fail ){
    rc = SQLITE_ERROR;
  }وگرنه{
    rc = wrapped.pcache.xInit(wrapped.pcache.pArg);
  }
  اگر( rc==SQLITE_OK ){
    wrapped.pcache_init = 1;
  }
  بازگشت rc;
}
ایستا تهی wrPCacheShutdown(تهی *pArg){
  wrapped.pcache.xShutdown(wrapped.pcache.pArg);
  wrapped.pcache_init = 0;
}

ایستا sqlite3_pcache *wrPCacheCreate(صحیح a, صحیح b, صحیح c){
  بازگشت wrapped.pcache.xCreate(a, b, c);
}  
ایستا تهی wrPCacheCachesize(sqlite3_pcache *p, صحیح n){
  wrapped.pcache.xCachesize(p, n);
}  
ایستا صحیح wrPCachePagecount(sqlite3_pcache *p){
  بازگشت wrapped.pcache.xPagecount(p);
}  
ایستا sqlite3_pcache_page *wrPCacheFetch(sqlite3_pcache *p, unsigned a, صحیح b){
  بازگشت wrapped.pcache.xFetch(p, a, b);
}  
ایستا تهی wrPCacheUnpin(sqlite3_pcache *p, sqlite3_pcache_page *a, صحیح b){
  wrapped.pcache.xUnpin(p, a, b);
}  
ایستا تهی wrPCacheRekey(
  sqlite3_pcache *p, 
  sqlite3_pcache_page *a, 
  unsigned b, 
  unsigned c
){
  wrapped.pcache.xRekey(p, a, b, c);
}  
ایستا تهی wrPCacheTruncate(sqlite3_pcache *p, unsigned a){
  wrapped.pcache.xTruncate(p, a);
}  
ایستا تهی wrPCacheDestroy(sqlite3_pcache *p){
  wrapped.pcache.xDestroy(p);
}  

ایستا تهی installInitWrappers(تهی){
  sqlite3_mutex_methods mutexmethods = {
    wrMutexInit,  wrMutexEnd,   wrMutexAlloc,
    wrMutexFree,  wrMutexEnter, wrMutexTry,
    wrMutexLeave, wrMutexHeld,  wrMutexNotheld
  };
  sqlite3_pcache_methods2 pcachemethods = {
    1, 0,
    wrPCacheInit,      wrPCacheShutdown,  wrPCacheCreate, 
    wrPCacheCachesize, wrPCachePagecount, wrPCacheFetch,
    wrPCacheUnpin,     wrPCacheRekey,     wrPCacheTruncate,  
    wrPCacheDestroy
  };
  sqlite3_mem_methods memmethods = {
    wrMemMalloc,   wrMemFree,    wrMemRealloc,
    wrMemSize,     wrMemRoundup, wrMemInit,
    wrMemShutdown,
    0
  };

  memset(&wrapped, 0, اندازه(wrapped));

  sqlite3_shutdown();
  sqlite3_config(SQLITE_CONFIG_GETMUTEX, &wrapped.mutex);
  sqlite3_config(SQLITE_CONFIG_GETMALLOC, &wrapped.mem);
  sqlite3_config(SQLITE_CONFIG_GETPCACHE2, &wrapped.pcache);
  sqlite3_config(SQLITE_CONFIG_MUTEX, &mutexmethods);
  sqlite3_config(SQLITE_CONFIG_MALLOC, &memmethods);
  sqlite3_config(SQLITE_CONFIG_PCACHE2, &pcachemethods);
}

ایستا صحیح SQLITE_TCLAPI init_wrapper_install(
  ClientData clientData, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  صحیح i;
  installInitWrappers();
  برای(i=1; i<objc; i++){
    نویسه *z = Tcl_GetString(objv[i]);
    اگر( strcmp(z, "mem")==0 ){
      wrapped.mem_fail = 1;
    }وگرنه اگر( strcmp(z, "mutex")==0 ){
      wrapped.mutex_fail = 1;
    }وگرنه اگر( strcmp(z, "pcache")==0 ){
      wrapped.pcache_fail = 1;
    }وگرنه{
      Tcl_AppendResult(interp, "Unknown argument: \"", z, "\"", پوچ);
      بازگشت TCL_ERROR;
    }
  }
  بازگشت TCL_OK;
}

ایستا صحیح SQLITE_TCLAPI init_wrapper_uninstall(
  ClientData clientData, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  اگر( objc!=1 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "");
    بازگشت TCL_ERROR;
  }

  sqlite3_shutdown();
  sqlite3_config(SQLITE_CONFIG_MUTEX, &wrapped.mutex);
  sqlite3_config(SQLITE_CONFIG_MALLOC, &wrapped.mem);
  sqlite3_config(SQLITE_CONFIG_PCACHE2, &wrapped.pcache);
  بازگشت TCL_OK;
}

ایستا صحیح SQLITE_TCLAPI init_wrapper_clear(
  ClientData clientData, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  اگر( objc!=1 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "");
    بازگشت TCL_ERROR;
  }

  wrapped.mem_fail = 0;
  wrapped.mutex_fail = 0;
  wrapped.pcache_fail = 0;
  بازگشت TCL_OK;
}

ایستا صحیح SQLITE_TCLAPI init_wrapper_query(
  ClientData clientData, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  Tcl_Obj *pRet;

  اگر( objc!=1 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "");
    بازگشت TCL_ERROR;
  }

  pRet = Tcl_NewObj();
  اگر( wrapped.mutex_init ){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj("mutex", -1));
  }
  اگر( wrapped.mem_init ){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj("mem", -1));
  }
  اگر( wrapped.pcache_init ){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj("pcache", -1));
  }

  Tcl_SetObjResult(interp, pRet);
  بازگشت TCL_OK;
}

صحیح Sqlitetest_init_Init(Tcl_Interp *interp){
  ایستا ساختار {
     نویسه *zName;
     Tcl_ObjCmdProc *xProc;
  } aObjCmd[] = {
    {"init_wrapper_install",   init_wrapper_install},
    {"init_wrapper_query",     init_wrapper_query  },
    {"init_wrapper_uninstall", init_wrapper_uninstall},
    {"init_wrapper_clear",     init_wrapper_clear}
  };
  صحیح i;

  برای(i=0; i<اندازه(aObjCmd)/اندازه(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }

  بازگشت TCL_OK;
}
