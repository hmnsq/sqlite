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

#ಅನ್ತರ್ಭಾವ "sqliteInt.h"
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ "tclsqlite.h"

ಸ್ಥಿರ ಸಂರಚನಾ Wrapped {
  sqlite3_pcache_methods2 pcache;
  sqlite3_mem_methods     mem;
  sqlite3_mutex_methods   mutex;

  ಪೂರ್ಣಾಙ್ಕ mem_init;              /* True if mem subsystem is initialized */
  ಪೂರ್ಣಾಙ್ಕ mem_fail;              /* True to fail mem subsystem initialization */
  ಪೂರ್ಣಾಙ್ಕ mutex_init;            /* True if mutex subsystem is initialized */
  ಪೂರ್ಣಾಙ್ಕ mutex_fail;            /* True to fail mutex subsystem initialization */
  ಪೂರ್ಣಾಙ್ಕ pcache_init;           /* True if pcache subsystem is initialized */
  ಪೂರ್ಣಾಙ್ಕ pcache_fail;           /* True to fail pcache subsystem initialization */
} wrapped;

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ wrMemInit(ಶೂನ್ಯ *pAppData){
  ಪೂರ್ಣಾಙ್ಕ rc;
  ಯದಿ( wrapped.mem_fail ){
    rc = SQLITE_ERROR;
  }ಅನ್ಯಥಾ{
    rc = wrapped.mem.xInit(wrapped.mem.pAppData);
  }
  ಯದಿ( rc==SQLITE_OK ){
    wrapped.mem_init = 1;
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}
ಸ್ಥಿರ ಶೂನ್ಯ wrMemShutdown(ಶೂನ್ಯ *pAppData){
  wrapped.mem.xShutdown(wrapped.mem.pAppData);
  wrapped.mem_init = 0;
}
ಸ್ಥಿರ ಶೂನ್ಯ *wrMemMalloc(ಪೂರ್ಣಾಙ್ಕ n)           {ಪ್ರತ್ಯಾಗಮ wrapped.mem.xMalloc(n);}
ಸ್ಥಿರ ಶೂನ್ಯ wrMemFree(ಶೂನ್ಯ *p)            {wrapped.mem.xFree(p);}
ಸ್ಥಿರ ಶೂನ್ಯ *wrMemRealloc(ಶೂನ್ಯ *p, ಪೂರ್ಣಾಙ್ಕ n) {ಪ್ರತ್ಯಾಗಮ wrapped.mem.xRealloc(p, n);}
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ wrMemSize(ಶೂನ್ಯ *p)             {ಪ್ರತ್ಯಾಗಮ wrapped.mem.xSize(p);}
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ wrMemRoundup(ಪೂರ್ಣಾಙ್ಕ n)            {ಪ್ರತ್ಯಾಗಮ wrapped.mem.xRoundup(n);}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ wrMutexInit(ಶೂನ್ಯ){
  ಪೂರ್ಣಾಙ್ಕ rc;
  ಯದಿ( wrapped.mutex_fail ){
    rc = SQLITE_ERROR;
  }ಅನ್ಯಥಾ{
    rc = wrapped.mutex.xMutexInit();
  }
  ಯದಿ( rc==SQLITE_OK ){
    wrapped.mutex_init = 1;
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ wrMutexEnd(ಶೂನ್ಯ){
  wrapped.mutex.xMutexEnd();
  wrapped.mutex_init = 0;
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}
ಸ್ಥಿರ sqlite3_mutex *wrMutexAlloc(ಪೂರ್ಣಾಙ್ಕ e){
  ಪ್ರತ್ಯಾಗಮ wrapped.mutex.xMutexAlloc(e);
}
ಸ್ಥಿರ ಶೂನ್ಯ wrMutexFree(sqlite3_mutex *p){
  wrapped.mutex.xMutexFree(p);
}
ಸ್ಥಿರ ಶೂನ್ಯ wrMutexEnter(sqlite3_mutex *p){
  wrapped.mutex.xMutexEnter(p);
}
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ wrMutexTry(sqlite3_mutex *p){
  ಪ್ರತ್ಯಾಗಮ wrapped.mutex.xMutexTry(p);
}
ಸ್ಥಿರ ಶೂನ್ಯ wrMutexLeave(sqlite3_mutex *p){
  wrapped.mutex.xMutexLeave(p);
}
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ wrMutexHeld(sqlite3_mutex *p){
  ಪ್ರತ್ಯಾಗಮ wrapped.mutex.xMutexHeld(p);
}
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ wrMutexNotheld(sqlite3_mutex *p){
  ಪ್ರತ್ಯಾಗಮ wrapped.mutex.xMutexNotheld(p);
}



ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ wrPCacheInit(ಶೂನ್ಯ *pArg){
  ಪೂರ್ಣಾಙ್ಕ rc;
  ಯದಿ( wrapped.pcache_fail ){
    rc = SQLITE_ERROR;
  }ಅನ್ಯಥಾ{
    rc = wrapped.pcache.xInit(wrapped.pcache.pArg);
  }
  ಯದಿ( rc==SQLITE_OK ){
    wrapped.pcache_init = 1;
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}
ಸ್ಥಿರ ಶೂನ್ಯ wrPCacheShutdown(ಶೂನ್ಯ *pArg){
  wrapped.pcache.xShutdown(wrapped.pcache.pArg);
  wrapped.pcache_init = 0;
}

ಸ್ಥಿರ sqlite3_pcache *wrPCacheCreate(ಪೂರ್ಣಾಙ್ಕ a, ಪೂರ್ಣಾಙ್ಕ b, ಪೂರ್ಣಾಙ್ಕ c){
  ಪ್ರತ್ಯಾಗಮ wrapped.pcache.xCreate(a, b, c);
}  
ಸ್ಥಿರ ಶೂನ್ಯ wrPCacheCachesize(sqlite3_pcache *p, ಪೂರ್ಣಾಙ್ಕ n){
  wrapped.pcache.xCachesize(p, n);
}  
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ wrPCachePagecount(sqlite3_pcache *p){
  ಪ್ರತ್ಯಾಗಮ wrapped.pcache.xPagecount(p);
}  
ಸ್ಥಿರ sqlite3_pcache_page *wrPCacheFetch(sqlite3_pcache *p, unsigned a, ಪೂರ್ಣಾಙ್ಕ b){
  ಪ್ರತ್ಯಾಗಮ wrapped.pcache.xFetch(p, a, b);
}  
ಸ್ಥಿರ ಶೂನ್ಯ wrPCacheUnpin(sqlite3_pcache *p, sqlite3_pcache_page *a, ಪೂರ್ಣಾಙ್ಕ b){
  wrapped.pcache.xUnpin(p, a, b);
}  
ಸ್ಥಿರ ಶೂನ್ಯ wrPCacheRekey(
  sqlite3_pcache *p, 
  sqlite3_pcache_page *a, 
  unsigned b, 
  unsigned c
){
  wrapped.pcache.xRekey(p, a, b, c);
}  
ಸ್ಥಿರ ಶೂನ್ಯ wrPCacheTruncate(sqlite3_pcache *p, unsigned a){
  wrapped.pcache.xTruncate(p, a);
}  
ಸ್ಥಿರ ಶೂನ್ಯ wrPCacheDestroy(sqlite3_pcache *p){
  wrapped.pcache.xDestroy(p);
}  

ಸ್ಥಿರ ಶೂನ್ಯ installInitWrappers(ಶೂನ್ಯ){
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

  memset(&wrapped, 0, ಪರಿಮಾಣ(wrapped));

  sqlite3_shutdown();
  sqlite3_config(SQLITE_CONFIG_GETMUTEX, &wrapped.mutex);
  sqlite3_config(SQLITE_CONFIG_GETMALLOC, &wrapped.mem);
  sqlite3_config(SQLITE_CONFIG_GETPCACHE2, &wrapped.pcache);
  sqlite3_config(SQLITE_CONFIG_MUTEX, &mutexmethods);
  sqlite3_config(SQLITE_CONFIG_MALLOC, &memmethods);
  sqlite3_config(SQLITE_CONFIG_PCACHE2, &pcachemethods);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI init_wrapper_install(
  ClientData clientData, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  ಪೂರ್ಣಾಙ್ಕ i;
  installInitWrappers();
  ಪ್ರತಿ(i=1; i<objc; i++){
    ವರ್ಣ *z = Tcl_GetString(objv[i]);
    ಯದಿ( strcmp(z, "mem")==0 ){
      wrapped.mem_fail = 1;
    }ಅನ್ಯಥಾ ಯದಿ( strcmp(z, "mutex")==0 ){
      wrapped.mutex_fail = 1;
    }ಅನ್ಯಥಾ ಯದಿ( strcmp(z, "pcache")==0 ){
      wrapped.pcache_fail = 1;
    }ಅನ್ಯಥಾ{
      Tcl_AppendResult(interp, "Unknown argument: \"", z, "\"", ರಿಕ್ತ);
      ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
    }
  }
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI init_wrapper_uninstall(
  ClientData clientData, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  ಯದಿ( objc!=1 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  sqlite3_shutdown();
  sqlite3_config(SQLITE_CONFIG_MUTEX, &wrapped.mutex);
  sqlite3_config(SQLITE_CONFIG_MALLOC, &wrapped.mem);
  sqlite3_config(SQLITE_CONFIG_PCACHE2, &wrapped.pcache);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI init_wrapper_clear(
  ClientData clientData, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  ಯದಿ( objc!=1 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  wrapped.mem_fail = 0;
  wrapped.mutex_fail = 0;
  wrapped.pcache_fail = 0;
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI init_wrapper_query(
  ClientData clientData, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  Tcl_Obj *pRet;

  ಯದಿ( objc!=1 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  pRet = Tcl_NewObj();
  ಯದಿ( wrapped.mutex_init ){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj("mutex", -1));
  }
  ಯದಿ( wrapped.mem_init ){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj("mem", -1));
  }
  ಯದಿ( wrapped.pcache_init ){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj("pcache", -1));
  }

  Tcl_SetObjResult(interp, pRet);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

ಪೂರ್ಣಾಙ್ಕ Sqlitetest_init_Init(Tcl_Interp *interp){
  ಸ್ಥಿರ ಸಂರಚನಾ {
     ವರ್ಣ *zName;
     Tcl_ObjCmdProc *xProc;
  } aObjCmd[] = {
    {"init_wrapper_install",   init_wrapper_install},
    {"init_wrapper_query",     init_wrapper_query  },
    {"init_wrapper_uninstall", init_wrapper_uninstall},
    {"init_wrapper_clear",     init_wrapper_clear}
  };
  ಪೂರ್ಣಾಙ್ಕ i;

  ಪ್ರತಿ(i=0; i<ಪರಿಮಾಣ(aObjCmd)/ಪರಿಮಾಣ(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }

  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}
