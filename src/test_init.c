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

ಸ್ಥಿರ ರಚನೆ Wrapped {
  sqlite3_pcache_methods2 pcache;
  sqlite3_mem_methods     mem;
  sqlite3_mutex_methods   mutex;

  ಪೂರ್ಣಾಂಕ mem_init;              /* True if mem subsystem is initialized */
  ಪೂರ್ಣಾಂಕ mem_fail;              /* True to fail mem subsystem initialization */
  ಪೂರ್ಣಾಂಕ mutex_init;            /* True if mutex subsystem is initialized */
  ಪೂರ್ಣಾಂಕ mutex_fail;            /* True to fail mutex subsystem initialization */
  ಪೂರ್ಣಾಂಕ pcache_init;           /* True if pcache subsystem is initialized */
  ಪೂರ್ಣಾಂಕ pcache_fail;           /* True to fail pcache subsystem initialization */
} wrapped;

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ wrMemInit(ಶೂನ್ಯ *p_App_ದತ್ತಾಂಶ){
  ಪೂರ್ಣಾಂಕ rc;
  ಆದರೆ( wrapped.mem_fail ){
    rc = SQLITE_ERROR;
  }ಇಲ್ಲದಿದ್ದರೆ{
    rc = wrapped.mem.xInit(wrapped.mem.p_App_ದತ್ತಾಂಶ);
  }
  ಆದರೆ( rc==SQLITE_OK ){
    wrapped.mem_init = 1;
  }
  ಹಿಂತಿರುಗಿ rc;
}
ಸ್ಥಿರ ಶೂನ್ಯ wrMemShutdown(ಶೂನ್ಯ *p_App_ದತ್ತಾಂಶ){
  wrapped.mem.xShutdown(wrapped.mem.p_App_ದತ್ತಾಂಶ);
  wrapped.mem_init = 0;
}
ಸ್ಥಿರ ಶೂನ್ಯ *wrMemMalloc(ಪೂರ್ಣಾಂಕ n)           {ಹಿಂತಿರುಗಿ wrapped.mem.xMalloc(n);}
ಸ್ಥಿರ ಶೂನ್ಯ wrMemFree(ಶೂನ್ಯ *p)            {wrapped.mem.xFree(p);}
ಸ್ಥಿರ ಶೂನ್ಯ *wrMemRealloc(ಶೂನ್ಯ *p, ಪೂರ್ಣಾಂಕ n) {ಹಿಂತಿರುಗಿ wrapped.mem.xRealloc(p, n);}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ wr_Mem_ಗಾತ್ರ(ಶೂನ್ಯ *p)             {ಹಿಂತಿರುಗಿ wrapped.mem.x_ಗಾತ್ರ(p);}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ wrMemRoundup(ಪೂರ್ಣಾಂಕ n)            {ಹಿಂತಿರುಗಿ wrapped.mem.xRoundup(n);}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ wrMutexInit(ಶೂನ್ಯ){
  ಪೂರ್ಣಾಂಕ rc;
  ಆದರೆ( wrapped.mutex_fail ){
    rc = SQLITE_ERROR;
  }ಇಲ್ಲದಿದ್ದರೆ{
    rc = wrapped.mutex.xMutexInit();
  }
  ಆದರೆ( rc==SQLITE_OK ){
    wrapped.mutex_init = 1;
  }
  ಹಿಂತಿರುಗಿ rc;
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ wr_Mutex_ಕೊನೆ(ಶೂನ್ಯ){
  wrapped.mutex.x_Mutex_ಕೊನೆ();
  wrapped.mutex_init = 0;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}
ಸ್ಥಿರ sqlite3_mutex *wrMutexAlloc(ಪೂರ್ಣಾಂಕ e){
  ಹಿಂತಿರುಗಿ wrapped.mutex.xMutexAlloc(e);
}
ಸ್ಥಿರ ಶೂನ್ಯ wrMutexFree(sqlite3_mutex *p){
  wrapped.mutex.xMutexFree(p);
}
ಸ್ಥಿರ ಶೂನ್ಯ wrMutexEnter(sqlite3_mutex *p){
  wrapped.mutex.xMutexEnter(p);
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ wrMutexTry(sqlite3_mutex *p){
  ಹಿಂತಿರುಗಿ wrapped.mutex.xMutexTry(p);
}
ಸ್ಥಿರ ಶೂನ್ಯ wrMutexLeave(sqlite3_mutex *p){
  wrapped.mutex.xMutexLeave(p);
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ wrMutexHeld(sqlite3_mutex *p){
  ಹಿಂತಿರುಗಿ wrapped.mutex.xMutexHeld(p);
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ wrMutexNotheld(sqlite3_mutex *p){
  ಹಿಂತಿರುಗಿ wrapped.mutex.xMutexNotheld(p);
}



ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ wrPCacheInit(ಶೂನ್ಯ *pArg){
  ಪೂರ್ಣಾಂಕ rc;
  ಆದರೆ( wrapped.pcache_fail ){
    rc = SQLITE_ERROR;
  }ಇಲ್ಲದಿದ್ದರೆ{
    rc = wrapped.pcache.xInit(wrapped.pcache.pArg);
  }
  ಆದರೆ( rc==SQLITE_OK ){
    wrapped.pcache_init = 1;
  }
  ಹಿಂತಿರುಗಿ rc;
}
ಸ್ಥಿರ ಶೂನ್ಯ wrPCacheShutdown(ಶೂನ್ಯ *pArg){
  wrapped.pcache.xShutdown(wrapped.pcache.pArg);
  wrapped.pcache_init = 0;
}

ಸ್ಥಿರ sqlite3_pcache *wrPCacheCreate(ಪೂರ್ಣಾಂಕ a, ಪೂರ್ಣಾಂಕ b, ಪೂರ್ಣಾಂಕ c){
  ಹಿಂತಿರುಗಿ wrapped.pcache.xCreate(a, b, c);
}  
ಸ್ಥಿರ ಶೂನ್ಯ wrPCacheCachesize(sqlite3_pcache *p, ಪೂರ್ಣಾಂಕ n){
  wrapped.pcache.xCachesize(p, n);
}  
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ wrPCachePagecount(sqlite3_pcache *p){
  ಹಿಂತಿರುಗಿ wrapped.pcache.xPagecount(p);
}  
ಸ್ಥಿರ sqlite3_pcache_ಪುಟ *wrPCacheFetch(sqlite3_pcache *p, ಚಿಹ್ನೆರಹಿತ a, ಪೂರ್ಣಾಂಕ b){
  ಹಿಂತಿರುಗಿ wrapped.pcache.xFetch(p, a, b);
}  
ಸ್ಥಿರ ಶೂನ್ಯ wrPCacheUnpin(sqlite3_pcache *p, sqlite3_pcache_ಪುಟ *a, ಪೂರ್ಣಾಂಕ b){
  wrapped.pcache.xUnpin(p, a, b);
}  
ಸ್ಥಿರ ಶೂನ್ಯ wrPCacheRekey(
  sqlite3_pcache *p, 
  sqlite3_pcache_ಪುಟ *a, 
  ಚಿಹ್ನೆರಹಿತ b, 
  ಚಿಹ್ನೆರಹಿತ c
){
  wrapped.pcache.xRekey(p, a, b, c);
}  
ಸ್ಥಿರ ಶೂನ್ಯ wrPCacheTruncate(sqlite3_pcache *p, ಚಿಹ್ನೆರಹಿತ a){
  wrapped.pcache.xTruncate(p, a);
}  
ಸ್ಥಿರ ಶೂನ್ಯ wrPCacheDestroy(sqlite3_pcache *p){
  wrapped.pcache.xDestroy(p);
}  

ಸ್ಥಿರ ಶೂನ್ಯ installInitWrappers(ಶೂನ್ಯ){
  sqlite3_mutex_methods mutexmethods = {
    wrMutexInit,  wr_Mutex_ಕೊನೆ,   wrMutexAlloc,
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
    wr_Mem_ಗಾತ್ರ,     wrMemRoundup, wrMemInit,
    wrMemShutdown,
    0
  };

  memset(&wrapped, 0, ಗಾತ್ರ(wrapped));

  sqlite3_shutdown();
  sqlite3_config(SQLITE_CONFIG_GETMUTEX, &wrapped.mutex);
  sqlite3_config(SQLITE_CONFIG_GETMALLOC, &wrapped.mem);
  sqlite3_config(SQLITE_CONFIG_GETPCACHE2, &wrapped.pcache);
  sqlite3_config(SQLITE_CONFIG_MUTEX, &mutexmethods);
  sqlite3_config(SQLITE_CONFIG_MALLOC, &memmethods);
  sqlite3_config(SQLITE_CONFIG_PCACHE2, &pcachemethods);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI init_wrapper_install(
  Client_ದತ್ತಾಂಶ client_ದತ್ತಾಂಶ, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  ಪೂರ್ಣಾಂಕ i;
  installInitWrappers();
  ಗಾಗಿ(i=1; i<objc; i++){
    ಅಕ್ಷರ *z = Tcl_GetString(objv[i]);
    ಆದರೆ( strcmp(z, "mem")==0 ){
      wrapped.mem_fail = 1;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( strcmp(z, "mutex")==0 ){
      wrapped.mutex_fail = 1;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( strcmp(z, "pcache")==0 ){
      wrapped.pcache_fail = 1;
    }ಇಲ್ಲದಿದ್ದರೆ{
      Tcl_Append_ಫಲಿತಾಂಶ(interp, "Unknown argument: \"", z, "\"", ಖಾಲಿ);
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }
  }
  ಹಿಂತಿರುಗಿ TCL_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI init_wrapper_uninstall(
  Client_ದತ್ತಾಂಶ client_ದತ್ತಾಂಶ, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  ಆದರೆ( objc!=1 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  sqlite3_shutdown();
  sqlite3_config(SQLITE_CONFIG_MUTEX, &wrapped.mutex);
  sqlite3_config(SQLITE_CONFIG_MALLOC, &wrapped.mem);
  sqlite3_config(SQLITE_CONFIG_PCACHE2, &wrapped.pcache);
  ಹಿಂತಿರುಗಿ TCL_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI init_wrapper_clear(
  Client_ದತ್ತಾಂಶ client_ದತ್ತಾಂಶ, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  ಆದರೆ( objc!=1 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  wrapped.mem_fail = 0;
  wrapped.mutex_fail = 0;
  wrapped.pcache_fail = 0;
  ಹಿಂತಿರುಗಿ TCL_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI init_wrapper_query(
  Client_ದತ್ತಾಂಶ client_ದತ್ತಾಂಶ, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  Tcl_Obj *pRet;

  ಆದರೆ( objc!=1 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  pRet = Tcl_NewObj();
  ಆದರೆ( wrapped.mutex_init ){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj("mutex", -1));
  }
  ಆದರೆ( wrapped.mem_init ){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj("mem", -1));
  }
  ಆದರೆ( wrapped.pcache_init ){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj("pcache", -1));
  }

  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, pRet);
  ಹಿಂತಿರುಗಿ TCL_OK;
}

ಪೂರ್ಣಾಂಕ Sqlitetest_init_Init(Tcl_Interp *interp){
  ಸ್ಥಿರ ರಚನೆ {
     ಅಕ್ಷರ *zName;
     Tcl_ObjCmdProc *xProc;
  } aObjCmd[] = {
    {"init_wrapper_install",   init_wrapper_install},
    {"init_wrapper_query",     init_wrapper_query  },
    {"init_wrapper_uninstall", init_wrapper_uninstall},
    {"init_wrapper_clear",     init_wrapper_clear}
  };
  ಪೂರ್ಣಾಂಕ i;

  ಗಾಗಿ(i=0; i<ಗಾತ್ರ(aObjCmd)/ಗಾತ್ರ(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }

  ಹಿಂತಿರುಗಿ TCL_OK;
}
