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

#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ "tclsqlite.h"

ਸਥਿਰ ਬਣਤਰ Wrapped {
  sqlite3_pcache_methods2 pcache;
  sqlite3_mem_methods     mem;
  sqlite3_mutex_methods   mutex;

  ਪੂਰਨ mem_init;              /* True if mem subsystem is initialized */
  ਪੂਰਨ mem_fail;              /* True to fail mem subsystem initialization */
  ਪੂਰਨ mutex_init;            /* True if mutex subsystem is initialized */
  ਪੂਰਨ mutex_fail;            /* True to fail mutex subsystem initialization */
  ਪੂਰਨ pcache_init;           /* True if pcache subsystem is initialized */
  ਪੂਰਨ pcache_fail;           /* True to fail pcache subsystem initialization */
} wrapped;

ਸਥਿਰ ਪੂਰਨ wrMemInit(ਖਾਲੀ *p_App_ਅੰਕੜੇ){
  ਪੂਰਨ rc;
  ਜੇ( wrapped.mem_fail ){
    rc = SQLITE_ERROR;
  }ਨਹੀਂਤਾਂ{
    rc = wrapped.mem.xInit(wrapped.mem.p_App_ਅੰਕੜੇ);
  }
  ਜੇ( rc==SQLITE_OK ){
    wrapped.mem_init = 1;
  }
  ਵਾਪਸ rc;
}
ਸਥਿਰ ਖਾਲੀ wrMemShutdown(ਖਾਲੀ *p_App_ਅੰਕੜੇ){
  wrapped.mem.xShutdown(wrapped.mem.p_App_ਅੰਕੜੇ);
  wrapped.mem_init = 0;
}
ਸਥਿਰ ਖਾਲੀ *wrMemMalloc(ਪੂਰਨ n)           {ਵਾਪਸ wrapped.mem.xMalloc(n);}
ਸਥਿਰ ਖਾਲੀ wrMemFree(ਖਾਲੀ *p)            {wrapped.mem.xFree(p);}
ਸਥਿਰ ਖਾਲੀ *wrMemRealloc(ਖਾਲੀ *p, ਪੂਰਨ n) {ਵਾਪਸ wrapped.mem.xRealloc(p, n);}
ਸਥਿਰ ਪੂਰਨ wr_Mem_ਮਾਪ(ਖਾਲੀ *p)             {ਵਾਪਸ wrapped.mem.x_ਮਾਪ(p);}
ਸਥਿਰ ਪੂਰਨ wrMemRoundup(ਪੂਰਨ n)            {ਵਾਪਸ wrapped.mem.xRoundup(n);}


ਸਥਿਰ ਪੂਰਨ wrMutexInit(ਖਾਲੀ){
  ਪੂਰਨ rc;
  ਜੇ( wrapped.mutex_fail ){
    rc = SQLITE_ERROR;
  }ਨਹੀਂਤਾਂ{
    rc = wrapped.mutex.xMutexInit();
  }
  ਜੇ( rc==SQLITE_OK ){
    wrapped.mutex_init = 1;
  }
  ਵਾਪਸ rc;
}
ਸਥਿਰ ਪੂਰਨ wr_Mutex_ਸਿਰਾ(ਖਾਲੀ){
  wrapped.mutex.x_Mutex_ਸਿਰਾ();
  wrapped.mutex_init = 0;
  ਵਾਪਸ SQLITE_OK;
}
ਸਥਿਰ sqlite3_mutex *wrMutexAlloc(ਪੂਰਨ e){
  ਵਾਪਸ wrapped.mutex.xMutexAlloc(e);
}
ਸਥਿਰ ਖਾਲੀ wrMutexFree(sqlite3_mutex *p){
  wrapped.mutex.xMutexFree(p);
}
ਸਥਿਰ ਖਾਲੀ wrMutexEnter(sqlite3_mutex *p){
  wrapped.mutex.xMutexEnter(p);
}
ਸਥਿਰ ਪੂਰਨ wrMutexTry(sqlite3_mutex *p){
  ਵਾਪਸ wrapped.mutex.xMutexTry(p);
}
ਸਥਿਰ ਖਾਲੀ wrMutexLeave(sqlite3_mutex *p){
  wrapped.mutex.xMutexLeave(p);
}
ਸਥਿਰ ਪੂਰਨ wrMutexHeld(sqlite3_mutex *p){
  ਵਾਪਸ wrapped.mutex.xMutexHeld(p);
}
ਸਥਿਰ ਪੂਰਨ wrMutexNotheld(sqlite3_mutex *p){
  ਵਾਪਸ wrapped.mutex.xMutexNotheld(p);
}



ਸਥਿਰ ਪੂਰਨ wrPCacheInit(ਖਾਲੀ *pArg){
  ਪੂਰਨ rc;
  ਜੇ( wrapped.pcache_fail ){
    rc = SQLITE_ERROR;
  }ਨਹੀਂਤਾਂ{
    rc = wrapped.pcache.xInit(wrapped.pcache.pArg);
  }
  ਜੇ( rc==SQLITE_OK ){
    wrapped.pcache_init = 1;
  }
  ਵਾਪਸ rc;
}
ਸਥਿਰ ਖਾਲੀ wrPCacheShutdown(ਖਾਲੀ *pArg){
  wrapped.pcache.xShutdown(wrapped.pcache.pArg);
  wrapped.pcache_init = 0;
}

ਸਥਿਰ sqlite3_pcache *wrPCacheCreate(ਪੂਰਨ a, ਪੂਰਨ b, ਪੂਰਨ c){
  ਵਾਪਸ wrapped.pcache.xCreate(a, b, c);
}  
ਸਥਿਰ ਖਾਲੀ wrPCacheCachesize(sqlite3_pcache *p, ਪੂਰਨ n){
  wrapped.pcache.xCachesize(p, n);
}  
ਸਥਿਰ ਪੂਰਨ wrPCachePagecount(sqlite3_pcache *p){
  ਵਾਪਸ wrapped.pcache.xPagecount(p);
}  
ਸਥਿਰ sqlite3_pcache_ਪੰਨਾ *wrPCacheFetch(sqlite3_pcache *p, ਬਿਨਾਨਿਸ਼ਾਨੀ a, ਪੂਰਨ b){
  ਵਾਪਸ wrapped.pcache.xFetch(p, a, b);
}  
ਸਥਿਰ ਖਾਲੀ wrPCacheUnpin(sqlite3_pcache *p, sqlite3_pcache_ਪੰਨਾ *a, ਪੂਰਨ b){
  wrapped.pcache.xUnpin(p, a, b);
}  
ਸਥਿਰ ਖਾਲੀ wrPCacheRekey(
  sqlite3_pcache *p, 
  sqlite3_pcache_ਪੰਨਾ *a, 
  ਬਿਨਾਨਿਸ਼ਾਨੀ b, 
  ਬਿਨਾਨਿਸ਼ਾਨੀ c
){
  wrapped.pcache.xRekey(p, a, b, c);
}  
ਸਥਿਰ ਖਾਲੀ wrPCacheTruncate(sqlite3_pcache *p, ਬਿਨਾਨਿਸ਼ਾਨੀ a){
  wrapped.pcache.xTruncate(p, a);
}  
ਸਥਿਰ ਖਾਲੀ wrPCacheDestroy(sqlite3_pcache *p){
  wrapped.pcache.xDestroy(p);
}  

ਸਥਿਰ ਖਾਲੀ installInitWrappers(ਖਾਲੀ){
  sqlite3_mutex_methods mutexmethods = {
    wrMutexInit,  wr_Mutex_ਸਿਰਾ,   wrMutexAlloc,
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
    wr_Mem_ਮਾਪ,     wrMemRoundup, wrMemInit,
    wrMemShutdown,
    0
  };

  memset(&wrapped, 0, ਆਕਾਰ(wrapped));

  sqlite3_shutdown();
  sqlite3_config(SQLITE_CONFIG_GETMUTEX, &wrapped.mutex);
  sqlite3_config(SQLITE_CONFIG_GETMALLOC, &wrapped.mem);
  sqlite3_config(SQLITE_CONFIG_GETPCACHE2, &wrapped.pcache);
  sqlite3_config(SQLITE_CONFIG_MUTEX, &mutexmethods);
  sqlite3_config(SQLITE_CONFIG_MALLOC, &memmethods);
  sqlite3_config(SQLITE_CONFIG_PCACHE2, &pcachemethods);
}

ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI init_wrapper_install(
  Client_ਅੰਕੜੇ client_ਅੰਕੜੇ, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  ਪੂਰਨ i;
  installInitWrappers();
  ਲਈ(i=1; i<objc; i++){
    ਅੱਖਰ *z = Tcl_GetString(objv[i]);
    ਜੇ( strcmp(z, "mem")==0 ){
      wrapped.mem_fail = 1;
    }ਨਹੀਂਤਾਂ ਜੇ( strcmp(z, "mutex")==0 ){
      wrapped.mutex_fail = 1;
    }ਨਹੀਂਤਾਂ ਜੇ( strcmp(z, "pcache")==0 ){
      wrapped.pcache_fail = 1;
    }ਨਹੀਂਤਾਂ{
      Tcl_Append_ਨਤੀਜਾ(interp, "Unknown argument: \"", z, "\"", ਨਿਰਾ);
      ਵਾਪਸ TCL_ERROR;
    }
  }
  ਵਾਪਸ TCL_OK;
}

ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI init_wrapper_uninstall(
  Client_ਅੰਕੜੇ client_ਅੰਕੜੇ, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  ਜੇ( objc!=1 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "");
    ਵਾਪਸ TCL_ERROR;
  }

  sqlite3_shutdown();
  sqlite3_config(SQLITE_CONFIG_MUTEX, &wrapped.mutex);
  sqlite3_config(SQLITE_CONFIG_MALLOC, &wrapped.mem);
  sqlite3_config(SQLITE_CONFIG_PCACHE2, &wrapped.pcache);
  ਵਾਪਸ TCL_OK;
}

ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI init_wrapper_clear(
  Client_ਅੰਕੜੇ client_ਅੰਕੜੇ, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  ਜੇ( objc!=1 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "");
    ਵਾਪਸ TCL_ERROR;
  }

  wrapped.mem_fail = 0;
  wrapped.mutex_fail = 0;
  wrapped.pcache_fail = 0;
  ਵਾਪਸ TCL_OK;
}

ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI init_wrapper_query(
  Client_ਅੰਕੜੇ client_ਅੰਕੜੇ, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  Tcl_Obj *pRet;

  ਜੇ( objc!=1 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "");
    ਵਾਪਸ TCL_ERROR;
  }

  pRet = Tcl_NewObj();
  ਜੇ( wrapped.mutex_init ){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj("mutex", -1));
  }
  ਜੇ( wrapped.mem_init ){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj("mem", -1));
  }
  ਜੇ( wrapped.pcache_init ){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj("pcache", -1));
  }

  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, pRet);
  ਵਾਪਸ TCL_OK;
}

ਪੂਰਨ Sqlitetest_init_Init(Tcl_Interp *interp){
  ਸਥਿਰ ਬਣਤਰ {
     ਅੱਖਰ *zName;
     Tcl_ObjCmdProc *xProc;
  } aObjCmd[] = {
    {"init_wrapper_install",   init_wrapper_install},
    {"init_wrapper_query",     init_wrapper_query  },
    {"init_wrapper_uninstall", init_wrapper_uninstall},
    {"init_wrapper_clear",     init_wrapper_clear}
  };
  ਪੂਰਨ i;

  ਲਈ(i=0; i<ਆਕਾਰ(aObjCmd)/ਆਕਾਰ(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }

  ਵਾਪਸ TCL_OK;
}
