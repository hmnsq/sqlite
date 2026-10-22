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

#அந்தர்பாவ "sqliteInt.h"
#அந்தர்பாவ <string.h>
#அந்தர்பாவ "tclsqlite.h"

நிலையான கட்டமைப்பு Wrapped {
  sqlite3_pcache_methods2 pcache;
  sqlite3_mem_methods     mem;
  sqlite3_mutex_methods   mutex;

  முழு mem_init;              /* True if mem subsystem is initialized */
  முழு mem_fail;              /* True to fail mem subsystem initialization */
  முழு mutex_init;            /* True if mutex subsystem is initialized */
  முழு mutex_fail;            /* True to fail mutex subsystem initialization */
  முழு pcache_init;           /* True if pcache subsystem is initialized */
  முழு pcache_fail;           /* True to fail pcache subsystem initialization */
} wrapped;

நிலையான முழு wrMemInit(வெறுமை *pAppData){
  முழு rc;
  எனில்( wrapped.mem_fail ){
    rc = SQLITE_ERROR;
  }இல்லையெனில்{
    rc = wrapped.mem.xInit(wrapped.mem.pAppData);
  }
  எனில்( rc==SQLITE_OK ){
    wrapped.mem_init = 1;
  }
  திரும்பு rc;
}
நிலையான வெறுமை wrMemShutdown(வெறுமை *pAppData){
  wrapped.mem.xShutdown(wrapped.mem.pAppData);
  wrapped.mem_init = 0;
}
நிலையான வெறுமை *wrMemMalloc(முழு n)           {திரும்பு wrapped.mem.xMalloc(n);}
நிலையான வெறுமை wrMemFree(வெறுமை *p)            {wrapped.mem.xFree(p);}
நிலையான வெறுமை *wrMemRealloc(வெறுமை *p, முழு n) {திரும்பு wrapped.mem.xRealloc(p, n);}
நிலையான முழு wrMemSize(வெறுமை *p)             {திரும்பு wrapped.mem.xSize(p);}
நிலையான முழு wrMemRoundup(முழு n)            {திரும்பு wrapped.mem.xRoundup(n);}


நிலையான முழு wrMutexInit(வெறுமை){
  முழு rc;
  எனில்( wrapped.mutex_fail ){
    rc = SQLITE_ERROR;
  }இல்லையெனில்{
    rc = wrapped.mutex.xMutexInit();
  }
  எனில்( rc==SQLITE_OK ){
    wrapped.mutex_init = 1;
  }
  திரும்பு rc;
}
நிலையான முழு wrMutexEnd(வெறுமை){
  wrapped.mutex.xMutexEnd();
  wrapped.mutex_init = 0;
  திரும்பு SQLITE_OK;
}
நிலையான sqlite3_mutex *wrMutexAlloc(முழு e){
  திரும்பு wrapped.mutex.xMutexAlloc(e);
}
நிலையான வெறுமை wrMutexFree(sqlite3_mutex *p){
  wrapped.mutex.xMutexFree(p);
}
நிலையான வெறுமை wrMutexEnter(sqlite3_mutex *p){
  wrapped.mutex.xMutexEnter(p);
}
நிலையான முழு wrMutexTry(sqlite3_mutex *p){
  திரும்பு wrapped.mutex.xMutexTry(p);
}
நிலையான வெறுமை wrMutexLeave(sqlite3_mutex *p){
  wrapped.mutex.xMutexLeave(p);
}
நிலையான முழு wrMutexHeld(sqlite3_mutex *p){
  திரும்பு wrapped.mutex.xMutexHeld(p);
}
நிலையான முழு wrMutexNotheld(sqlite3_mutex *p){
  திரும்பு wrapped.mutex.xMutexNotheld(p);
}



நிலையான முழு wrPCacheInit(வெறுமை *pArg){
  முழு rc;
  எனில்( wrapped.pcache_fail ){
    rc = SQLITE_ERROR;
  }இல்லையெனில்{
    rc = wrapped.pcache.xInit(wrapped.pcache.pArg);
  }
  எனில்( rc==SQLITE_OK ){
    wrapped.pcache_init = 1;
  }
  திரும்பு rc;
}
நிலையான வெறுமை wrPCacheShutdown(வெறுமை *pArg){
  wrapped.pcache.xShutdown(wrapped.pcache.pArg);
  wrapped.pcache_init = 0;
}

நிலையான sqlite3_pcache *wrPCacheCreate(முழு a, முழு b, முழு c){
  திரும்பு wrapped.pcache.xCreate(a, b, c);
}  
நிலையான வெறுமை wrPCacheCachesize(sqlite3_pcache *p, முழு n){
  wrapped.pcache.xCachesize(p, n);
}  
நிலையான முழு wrPCachePagecount(sqlite3_pcache *p){
  திரும்பு wrapped.pcache.xPagecount(p);
}  
நிலையான sqlite3_pcache_page *wrPCacheFetch(sqlite3_pcache *p, unsigned a, முழு b){
  திரும்பு wrapped.pcache.xFetch(p, a, b);
}  
நிலையான வெறுமை wrPCacheUnpin(sqlite3_pcache *p, sqlite3_pcache_page *a, முழு b){
  wrapped.pcache.xUnpin(p, a, b);
}  
நிலையான வெறுமை wrPCacheRekey(
  sqlite3_pcache *p, 
  sqlite3_pcache_page *a, 
  unsigned b, 
  unsigned c
){
  wrapped.pcache.xRekey(p, a, b, c);
}  
நிலையான வெறுமை wrPCacheTruncate(sqlite3_pcache *p, unsigned a){
  wrapped.pcache.xTruncate(p, a);
}  
நிலையான வெறுமை wrPCacheDestroy(sqlite3_pcache *p){
  wrapped.pcache.xDestroy(p);
}  

நிலையான வெறுமை installInitWrappers(வெறுமை){
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

  memset(&wrapped, 0, அளவு(wrapped));

  sqlite3_shutdown();
  sqlite3_config(SQLITE_CONFIG_GETMUTEX, &wrapped.mutex);
  sqlite3_config(SQLITE_CONFIG_GETMALLOC, &wrapped.mem);
  sqlite3_config(SQLITE_CONFIG_GETPCACHE2, &wrapped.pcache);
  sqlite3_config(SQLITE_CONFIG_MUTEX, &mutexmethods);
  sqlite3_config(SQLITE_CONFIG_MALLOC, &memmethods);
  sqlite3_config(SQLITE_CONFIG_PCACHE2, &pcachemethods);
}

நிலையான முழு SQLITE_TCLAPI init_wrapper_install(
  ClientData clientData, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  முழு i;
  installInitWrappers();
  க்கு(i=1; i<objc; i++){
    எழுத்து *z = Tcl_GetString(objv[i]);
    எனில்( strcmp(z, "mem")==0 ){
      wrapped.mem_fail = 1;
    }இல்லையெனில் எனில்( strcmp(z, "mutex")==0 ){
      wrapped.mutex_fail = 1;
    }இல்லையெனில் எனில்( strcmp(z, "pcache")==0 ){
      wrapped.pcache_fail = 1;
    }இல்லையெனில்{
      Tcl_AppendResult(interp, "Unknown argument: \"", z, "\"", வெற்று);
      திரும்பு TCL_ERROR;
    }
  }
  திரும்பு TCL_OK;
}

நிலையான முழு SQLITE_TCLAPI init_wrapper_uninstall(
  ClientData clientData, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  எனில்( objc!=1 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "");
    திரும்பு TCL_ERROR;
  }

  sqlite3_shutdown();
  sqlite3_config(SQLITE_CONFIG_MUTEX, &wrapped.mutex);
  sqlite3_config(SQLITE_CONFIG_MALLOC, &wrapped.mem);
  sqlite3_config(SQLITE_CONFIG_PCACHE2, &wrapped.pcache);
  திரும்பு TCL_OK;
}

நிலையான முழு SQLITE_TCLAPI init_wrapper_clear(
  ClientData clientData, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  எனில்( objc!=1 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "");
    திரும்பு TCL_ERROR;
  }

  wrapped.mem_fail = 0;
  wrapped.mutex_fail = 0;
  wrapped.pcache_fail = 0;
  திரும்பு TCL_OK;
}

நிலையான முழு SQLITE_TCLAPI init_wrapper_query(
  ClientData clientData, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  Tcl_Obj *pRet;

  எனில்( objc!=1 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "");
    திரும்பு TCL_ERROR;
  }

  pRet = Tcl_NewObj();
  எனில்( wrapped.mutex_init ){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj("mutex", -1));
  }
  எனில்( wrapped.mem_init ){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj("mem", -1));
  }
  எனில்( wrapped.pcache_init ){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj("pcache", -1));
  }

  Tcl_SetObjResult(interp, pRet);
  திரும்பு TCL_OK;
}

முழு Sqlitetest_init_Init(Tcl_Interp *interp){
  நிலையான கட்டமைப்பு {
     எழுத்து *zName;
     Tcl_ObjCmdProc *xProc;
  } aObjCmd[] = {
    {"init_wrapper_install",   init_wrapper_install},
    {"init_wrapper_query",     init_wrapper_query  },
    {"init_wrapper_uninstall", init_wrapper_uninstall},
    {"init_wrapper_clear",     init_wrapper_clear}
  };
  முழு i;

  க்கு(i=0; i<அளவு(aObjCmd)/அளவு(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }

  திரும்பு TCL_OK;
}
