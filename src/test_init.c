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

#അന്തര്ഭാവ "sqliteInt.h"
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ "tclsqlite.h"

സ്ഥിതം ഘടന Wrapped {
  sqlite3_pcache_methods2 pcache;
  sqlite3_mem_methods     mem;
  sqlite3_mutex_methods   mutex;

  പൂർണ്ണസംഖ്യ mem_init;              /* True if mem subsystem is initialized */
  പൂർണ്ണസംഖ്യ mem_fail;              /* True to fail mem subsystem initialization */
  പൂർണ്ണസംഖ്യ mutex_init;            /* True if mutex subsystem is initialized */
  പൂർണ്ണസംഖ്യ mutex_fail;            /* True to fail mutex subsystem initialization */
  പൂർണ്ണസംഖ്യ pcache_init;           /* True if pcache subsystem is initialized */
  പൂർണ്ണസംഖ്യ pcache_fail;           /* True to fail pcache subsystem initialization */
} wrapped;

സ്ഥിതം പൂർണ്ണസംഖ്യ wrMemInit(ശൂന്യം *pAppData){
  പൂർണ്ണസംഖ്യ rc;
  എങ്കിൽ( wrapped.mem_fail ){
    rc = SQLITE_ERROR;
  }അല്ലെങ്കിൽ{
    rc = wrapped.mem.xInit(wrapped.mem.pAppData);
  }
  എങ്കിൽ( rc==SQLITE_OK ){
    wrapped.mem_init = 1;
  }
  തിരികെ rc;
}
സ്ഥിതം ശൂന്യം wrMemShutdown(ശൂന്യം *pAppData){
  wrapped.mem.xShutdown(wrapped.mem.pAppData);
  wrapped.mem_init = 0;
}
സ്ഥിതം ശൂന്യം *wrMemMalloc(പൂർണ്ണസംഖ്യ n)           {തിരികെ wrapped.mem.xMalloc(n);}
സ്ഥിതം ശൂന്യം wrMemFree(ശൂന്യം *p)            {wrapped.mem.xFree(p);}
സ്ഥിതം ശൂന്യം *wrMemRealloc(ശൂന്യം *p, പൂർണ്ണസംഖ്യ n) {തിരികെ wrapped.mem.xRealloc(p, n);}
സ്ഥിതം പൂർണ്ണസംഖ്യ wrMemSize(ശൂന്യം *p)             {തിരികെ wrapped.mem.xSize(p);}
സ്ഥിതം പൂർണ്ണസംഖ്യ wrMemRoundup(പൂർണ്ണസംഖ്യ n)            {തിരികെ wrapped.mem.xRoundup(n);}


സ്ഥിതം പൂർണ്ണസംഖ്യ wrMutexInit(ശൂന്യം){
  പൂർണ്ണസംഖ്യ rc;
  എങ്കിൽ( wrapped.mutex_fail ){
    rc = SQLITE_ERROR;
  }അല്ലെങ്കിൽ{
    rc = wrapped.mutex.xMutexInit();
  }
  എങ്കിൽ( rc==SQLITE_OK ){
    wrapped.mutex_init = 1;
  }
  തിരികെ rc;
}
സ്ഥിതം പൂർണ്ണസംഖ്യ wrMutexEnd(ശൂന്യം){
  wrapped.mutex.xMutexEnd();
  wrapped.mutex_init = 0;
  തിരികെ SQLITE_OK;
}
സ്ഥിതം sqlite3_mutex *wrMutexAlloc(പൂർണ്ണസംഖ്യ e){
  തിരികെ wrapped.mutex.xMutexAlloc(e);
}
സ്ഥിതം ശൂന്യം wrMutexFree(sqlite3_mutex *p){
  wrapped.mutex.xMutexFree(p);
}
സ്ഥിതം ശൂന്യം wrMutexEnter(sqlite3_mutex *p){
  wrapped.mutex.xMutexEnter(p);
}
സ്ഥിതം പൂർണ്ണസംഖ്യ wrMutexTry(sqlite3_mutex *p){
  തിരികെ wrapped.mutex.xMutexTry(p);
}
സ്ഥിതം ശൂന്യം wrMutexLeave(sqlite3_mutex *p){
  wrapped.mutex.xMutexLeave(p);
}
സ്ഥിതം പൂർണ്ണസംഖ്യ wrMutexHeld(sqlite3_mutex *p){
  തിരികെ wrapped.mutex.xMutexHeld(p);
}
സ്ഥിതം പൂർണ്ണസംഖ്യ wrMutexNotheld(sqlite3_mutex *p){
  തിരികെ wrapped.mutex.xMutexNotheld(p);
}



സ്ഥിതം പൂർണ്ണസംഖ്യ wrPCacheInit(ശൂന്യം *pArg){
  പൂർണ്ണസംഖ്യ rc;
  എങ്കിൽ( wrapped.pcache_fail ){
    rc = SQLITE_ERROR;
  }അല്ലെങ്കിൽ{
    rc = wrapped.pcache.xInit(wrapped.pcache.pArg);
  }
  എങ്കിൽ( rc==SQLITE_OK ){
    wrapped.pcache_init = 1;
  }
  തിരികെ rc;
}
സ്ഥിതം ശൂന്യം wrPCacheShutdown(ശൂന്യം *pArg){
  wrapped.pcache.xShutdown(wrapped.pcache.pArg);
  wrapped.pcache_init = 0;
}

സ്ഥിതം sqlite3_pcache *wrPCacheCreate(പൂർണ്ണസംഖ്യ a, പൂർണ്ണസംഖ്യ b, പൂർണ്ണസംഖ്യ c){
  തിരികെ wrapped.pcache.xCreate(a, b, c);
}  
സ്ഥിതം ശൂന്യം wrPCacheCachesize(sqlite3_pcache *p, പൂർണ്ണസംഖ്യ n){
  wrapped.pcache.xCachesize(p, n);
}  
സ്ഥിതം പൂർണ്ണസംഖ്യ wrPCachePagecount(sqlite3_pcache *p){
  തിരികെ wrapped.pcache.xPagecount(p);
}  
സ്ഥിതം sqlite3_pcache_page *wrPCacheFetch(sqlite3_pcache *p, unsigned a, പൂർണ്ണസംഖ്യ b){
  തിരികെ wrapped.pcache.xFetch(p, a, b);
}  
സ്ഥിതം ശൂന്യം wrPCacheUnpin(sqlite3_pcache *p, sqlite3_pcache_page *a, പൂർണ്ണസംഖ്യ b){
  wrapped.pcache.xUnpin(p, a, b);
}  
സ്ഥിതം ശൂന്യം wrPCacheRekey(
  sqlite3_pcache *p, 
  sqlite3_pcache_page *a, 
  unsigned b, 
  unsigned c
){
  wrapped.pcache.xRekey(p, a, b, c);
}  
സ്ഥിതം ശൂന്യം wrPCacheTruncate(sqlite3_pcache *p, unsigned a){
  wrapped.pcache.xTruncate(p, a);
}  
സ്ഥിതം ശൂന്യം wrPCacheDestroy(sqlite3_pcache *p){
  wrapped.pcache.xDestroy(p);
}  

സ്ഥിതം ശൂന്യം installInitWrappers(ശൂന്യം){
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

  memset(&wrapped, 0, വലുപ്പം(wrapped));

  sqlite3_shutdown();
  sqlite3_config(SQLITE_CONFIG_GETMUTEX, &wrapped.mutex);
  sqlite3_config(SQLITE_CONFIG_GETMALLOC, &wrapped.mem);
  sqlite3_config(SQLITE_CONFIG_GETPCACHE2, &wrapped.pcache);
  sqlite3_config(SQLITE_CONFIG_MUTEX, &mutexmethods);
  sqlite3_config(SQLITE_CONFIG_MALLOC, &memmethods);
  sqlite3_config(SQLITE_CONFIG_PCACHE2, &pcachemethods);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI init_wrapper_install(
  ClientData clientData, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  പൂർണ്ണസംഖ്യ i;
  installInitWrappers();
  വേണ്ടി(i=1; i<objc; i++){
    അക്ഷരം *z = Tcl_GetString(objv[i]);
    എങ്കിൽ( strcmp(z, "mem")==0 ){
      wrapped.mem_fail = 1;
    }അല്ലെങ്കിൽ എങ്കിൽ( strcmp(z, "mutex")==0 ){
      wrapped.mutex_fail = 1;
    }അല്ലെങ്കിൽ എങ്കിൽ( strcmp(z, "pcache")==0 ){
      wrapped.pcache_fail = 1;
    }അല്ലെങ്കിൽ{
      Tcl_AppendResult(interp, "Unknown argument: \"", z, "\"", ശൂന്യ);
      തിരികെ TCL_ERROR;
    }
  }
  തിരികെ TCL_OK;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI init_wrapper_uninstall(
  ClientData clientData, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  എങ്കിൽ( objc!=1 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "");
    തിരികെ TCL_ERROR;
  }

  sqlite3_shutdown();
  sqlite3_config(SQLITE_CONFIG_MUTEX, &wrapped.mutex);
  sqlite3_config(SQLITE_CONFIG_MALLOC, &wrapped.mem);
  sqlite3_config(SQLITE_CONFIG_PCACHE2, &wrapped.pcache);
  തിരികെ TCL_OK;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI init_wrapper_clear(
  ClientData clientData, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  എങ്കിൽ( objc!=1 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "");
    തിരികെ TCL_ERROR;
  }

  wrapped.mem_fail = 0;
  wrapped.mutex_fail = 0;
  wrapped.pcache_fail = 0;
  തിരികെ TCL_OK;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI init_wrapper_query(
  ClientData clientData, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  Tcl_Obj *pRet;

  എങ്കിൽ( objc!=1 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "");
    തിരികെ TCL_ERROR;
  }

  pRet = Tcl_NewObj();
  എങ്കിൽ( wrapped.mutex_init ){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj("mutex", -1));
  }
  എങ്കിൽ( wrapped.mem_init ){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj("mem", -1));
  }
  എങ്കിൽ( wrapped.pcache_init ){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj("pcache", -1));
  }

  Tcl_SetObjResult(interp, pRet);
  തിരികെ TCL_OK;
}

പൂർണ്ണസംഖ്യ Sqlitetest_init_Init(Tcl_Interp *interp){
  സ്ഥിതം ഘടന {
     അക്ഷരം *zName;
     Tcl_ObjCmdProc *xProc;
  } aObjCmd[] = {
    {"init_wrapper_install",   init_wrapper_install},
    {"init_wrapper_query",     init_wrapper_query  },
    {"init_wrapper_uninstall", init_wrapper_uninstall},
    {"init_wrapper_clear",     init_wrapper_clear}
  };
  പൂർണ്ണസംഖ്യ i;

  വേണ്ടി(i=0; i<വലുപ്പം(aObjCmd)/വലുപ്പം(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }

  തിരികെ TCL_OK;
}
