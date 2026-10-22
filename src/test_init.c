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

#అన్తర్భావ "sqliteInt.h"
#అన్తర్భావ <string.h>
#అన్తర్భావ "tclsqlite.h"

స్థిర నిర్మాణం Wrapped {
  sqlite3_pcache_methods2 pcache;
  sqlite3_mem_methods     mem;
  sqlite3_mutex_methods   mutex;

  పూర్ణాంకం mem_init;              /* True if mem subsystem is initialized */
  పూర్ణాంకం mem_fail;              /* True to fail mem subsystem initialization */
  పూర్ణాంకం mutex_init;            /* True if mutex subsystem is initialized */
  పూర్ణాంకం mutex_fail;            /* True to fail mutex subsystem initialization */
  పూర్ణాంకం pcache_init;           /* True if pcache subsystem is initialized */
  పూర్ణాంకం pcache_fail;           /* True to fail pcache subsystem initialization */
} wrapped;

స్థిర పూర్ణాంకం wrMemInit(శూన్య *p_App_దత్తాంశం){
  పూర్ణాంకం rc;
  ఒకవేళ( wrapped.mem_fail ){
    rc = SQLITE_ERROR;
  }లేకపోతే{
    rc = wrapped.mem.xInit(wrapped.mem.p_App_దత్తాంశం);
  }
  ఒకవేళ( rc==SQLITE_OK ){
    wrapped.mem_init = 1;
  }
  తిరిగి rc;
}
స్థిర శూన్య wrMemShutdown(శూన్య *p_App_దత్తాంశం){
  wrapped.mem.xShutdown(wrapped.mem.p_App_దత్తాంశం);
  wrapped.mem_init = 0;
}
స్థిర శూన్య *wrMemMalloc(పూర్ణాంకం n)           {తిరిగి wrapped.mem.xMalloc(n);}
స్థిర శూన్య wrMemFree(శూన్య *p)            {wrapped.mem.xFree(p);}
స్థిర శూన్య *wrMemRealloc(శూన్య *p, పూర్ణాంకం n) {తిరిగి wrapped.mem.xRealloc(p, n);}
స్థిర పూర్ణాంకం wr_Mem_కొలత(శూన్య *p)             {తిరిగి wrapped.mem.x_కొలత(p);}
స్థిర పూర్ణాంకం wrMemRoundup(పూర్ణాంకం n)            {తిరిగి wrapped.mem.xRoundup(n);}


స్థిర పూర్ణాంకం wrMutexInit(శూన్య){
  పూర్ణాంకం rc;
  ఒకవేళ( wrapped.mutex_fail ){
    rc = SQLITE_ERROR;
  }లేకపోతే{
    rc = wrapped.mutex.xMutexInit();
  }
  ఒకవేళ( rc==SQLITE_OK ){
    wrapped.mutex_init = 1;
  }
  తిరిగి rc;
}
స్థిర పూర్ణాంకం wr_Mutex_చివర(శూన్య){
  wrapped.mutex.x_Mutex_చివర();
  wrapped.mutex_init = 0;
  తిరిగి SQLITE_OK;
}
స్థిర sqlite3_mutex *wrMutexAlloc(పూర్ణాంకం e){
  తిరిగి wrapped.mutex.xMutexAlloc(e);
}
స్థిర శూన్య wrMutexFree(sqlite3_mutex *p){
  wrapped.mutex.xMutexFree(p);
}
స్థిర శూన్య wrMutexEnter(sqlite3_mutex *p){
  wrapped.mutex.xMutexEnter(p);
}
స్థిర పూర్ణాంకం wrMutexTry(sqlite3_mutex *p){
  తిరిగి wrapped.mutex.xMutexTry(p);
}
స్థిర శూన్య wrMutexLeave(sqlite3_mutex *p){
  wrapped.mutex.xMutexLeave(p);
}
స్థిర పూర్ణాంకం wrMutexHeld(sqlite3_mutex *p){
  తిరిగి wrapped.mutex.xMutexHeld(p);
}
స్థిర పూర్ణాంకం wrMutexNotheld(sqlite3_mutex *p){
  తిరిగి wrapped.mutex.xMutexNotheld(p);
}



స్థిర పూర్ణాంకం wrPCacheInit(శూన్య *pArg){
  పూర్ణాంకం rc;
  ఒకవేళ( wrapped.pcache_fail ){
    rc = SQLITE_ERROR;
  }లేకపోతే{
    rc = wrapped.pcache.xInit(wrapped.pcache.pArg);
  }
  ఒకవేళ( rc==SQLITE_OK ){
    wrapped.pcache_init = 1;
  }
  తిరిగి rc;
}
స్థిర శూన్య wrPCacheShutdown(శూన్య *pArg){
  wrapped.pcache.xShutdown(wrapped.pcache.pArg);
  wrapped.pcache_init = 0;
}

స్థిర sqlite3_pcache *wrPCacheCreate(పూర్ణాంకం a, పూర్ణాంకం b, పూర్ణాంకం c){
  తిరిగి wrapped.pcache.xCreate(a, b, c);
}  
స్థిర శూన్య wrPCacheCachesize(sqlite3_pcache *p, పూర్ణాంకం n){
  wrapped.pcache.xCachesize(p, n);
}  
స్థిర పూర్ణాంకం wrPCachePagecount(sqlite3_pcache *p){
  తిరిగి wrapped.pcache.xPagecount(p);
}  
స్థిర sqlite3_pcache_పుట *wrPCacheFetch(sqlite3_pcache *p, గుర్తులేని a, పూర్ణాంకం b){
  తిరిగి wrapped.pcache.xFetch(p, a, b);
}  
స్థిర శూన్య wrPCacheUnpin(sqlite3_pcache *p, sqlite3_pcache_పుట *a, పూర్ణాంకం b){
  wrapped.pcache.xUnpin(p, a, b);
}  
స్థిర శూన్య wrPCacheRekey(
  sqlite3_pcache *p, 
  sqlite3_pcache_పుట *a, 
  గుర్తులేని b, 
  గుర్తులేని c
){
  wrapped.pcache.xRekey(p, a, b, c);
}  
స్థిర శూన్య wrPCacheTruncate(sqlite3_pcache *p, గుర్తులేని a){
  wrapped.pcache.xTruncate(p, a);
}  
స్థిర శూన్య wrPCacheDestroy(sqlite3_pcache *p){
  wrapped.pcache.xDestroy(p);
}  

స్థిర శూన్య installInitWrappers(శూన్య){
  sqlite3_mutex_methods mutexmethods = {
    wrMutexInit,  wr_Mutex_చివర,   wrMutexAlloc,
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
    wr_Mem_కొలత,     wrMemRoundup, wrMemInit,
    wrMemShutdown,
    0
  };

  memset(&wrapped, 0, పరిమాణం(wrapped));

  sqlite3_shutdown();
  sqlite3_config(SQLITE_CONFIG_GETMUTEX, &wrapped.mutex);
  sqlite3_config(SQLITE_CONFIG_GETMALLOC, &wrapped.mem);
  sqlite3_config(SQLITE_CONFIG_GETPCACHE2, &wrapped.pcache);
  sqlite3_config(SQLITE_CONFIG_MUTEX, &mutexmethods);
  sqlite3_config(SQLITE_CONFIG_MALLOC, &memmethods);
  sqlite3_config(SQLITE_CONFIG_PCACHE2, &pcachemethods);
}

స్థిర పూర్ణాంకం SQLITE_TCLAPI init_wrapper_install(
  Client_దత్తాంశం client_దత్తాంశం, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  పూర్ణాంకం i;
  installInitWrappers();
  కొరకు(i=1; i<objc; i++){
    అక్షరం *z = Tcl_GetString(objv[i]);
    ఒకవేళ( strcmp(z, "mem")==0 ){
      wrapped.mem_fail = 1;
    }లేకపోతే ఒకవేళ( strcmp(z, "mutex")==0 ){
      wrapped.mutex_fail = 1;
    }లేకపోతే ఒకవేళ( strcmp(z, "pcache")==0 ){
      wrapped.pcache_fail = 1;
    }లేకపోతే{
      Tcl_Append_ఫలితం(interp, "Unknown argument: \"", z, "\"", ఖాళీ);
      తిరిగి TCL_ERROR;
    }
  }
  తిరిగి TCL_OK;
}

స్థిర పూర్ణాంకం SQLITE_TCLAPI init_wrapper_uninstall(
  Client_దత్తాంశం client_దత్తాంశం, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  ఒకవేళ( objc!=1 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "");
    తిరిగి TCL_ERROR;
  }

  sqlite3_shutdown();
  sqlite3_config(SQLITE_CONFIG_MUTEX, &wrapped.mutex);
  sqlite3_config(SQLITE_CONFIG_MALLOC, &wrapped.mem);
  sqlite3_config(SQLITE_CONFIG_PCACHE2, &wrapped.pcache);
  తిరిగి TCL_OK;
}

స్థిర పూర్ణాంకం SQLITE_TCLAPI init_wrapper_clear(
  Client_దత్తాంశం client_దత్తాంశం, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  ఒకవేళ( objc!=1 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "");
    తిరిగి TCL_ERROR;
  }

  wrapped.mem_fail = 0;
  wrapped.mutex_fail = 0;
  wrapped.pcache_fail = 0;
  తిరిగి TCL_OK;
}

స్థిర పూర్ణాంకం SQLITE_TCLAPI init_wrapper_query(
  Client_దత్తాంశం client_దత్తాంశం, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  Tcl_Obj *pRet;

  ఒకవేళ( objc!=1 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "");
    తిరిగి TCL_ERROR;
  }

  pRet = Tcl_NewObj();
  ఒకవేళ( wrapped.mutex_init ){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj("mutex", -1));
  }
  ఒకవేళ( wrapped.mem_init ){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj("mem", -1));
  }
  ఒకవేళ( wrapped.pcache_init ){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj("pcache", -1));
  }

  Tcl_సమితి_Obj_ఫలితం(interp, pRet);
  తిరిగి TCL_OK;
}

పూర్ణాంకం Sqlitetest_init_Init(Tcl_Interp *interp){
  స్థిర నిర్మాణం {
     అక్షరం *zName;
     Tcl_ObjCmdProc *xProc;
  } aObjCmd[] = {
    {"init_wrapper_install",   init_wrapper_install},
    {"init_wrapper_query",     init_wrapper_query  },
    {"init_wrapper_uninstall", init_wrapper_uninstall},
    {"init_wrapper_clear",     init_wrapper_clear}
  };
  పూర్ణాంకం i;

  కొరకు(i=0; i<పరిమాణం(aObjCmd)/పరిమాణం(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }

  తిరిగి TCL_OK;
}
