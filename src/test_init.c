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

#અન્તર્ભાવ "sqliteInt.h"
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ "tclsqlite.h"

સ્થિર સંરચના Wrapped {
  sqlite3_pcache_methods2 pcache;
  sqlite3_mem_methods     mem;
  sqlite3_mutex_methods   mutex;

  પૂર્ણાંક mem_init;              /* True if mem subsystem is initialized */
  પૂર્ણાંક mem_fail;              /* True to fail mem subsystem initialization */
  પૂર્ણાંક mutex_init;            /* True if mutex subsystem is initialized */
  પૂર્ણાંક mutex_fail;            /* True to fail mutex subsystem initialization */
  પૂર્ણાંક pcache_init;           /* True if pcache subsystem is initialized */
  પૂર્ણાંક pcache_fail;           /* True to fail pcache subsystem initialization */
} wrapped;

સ્થિર પૂર્ણાંક wrMemInit(શૂન્ય *pAppData){
  પૂર્ણાંક rc;
  જો( wrapped.mem_fail ){
    rc = SQLITE_ERROR;
  }નહીંતર{
    rc = wrapped.mem.xInit(wrapped.mem.pAppData);
  }
  જો( rc==SQLITE_OK ){
    wrapped.mem_init = 1;
  }
  પાછા rc;
}
સ્થિર શૂન્ય wrMemShutdown(શૂન્ય *pAppData){
  wrapped.mem.xShutdown(wrapped.mem.pAppData);
  wrapped.mem_init = 0;
}
સ્થિર શૂન્ય *wrMemMalloc(પૂર્ણાંક n)           {પાછા wrapped.mem.xMalloc(n);}
સ્થિર શૂન્ય wrMemFree(શૂન્ય *p)            {wrapped.mem.xFree(p);}
સ્થિર શૂન્ય *wrMemRealloc(શૂન્ય *p, પૂર્ણાંક n) {પાછા wrapped.mem.xRealloc(p, n);}
સ્થિર પૂર્ણાંક wrMemSize(શૂન્ય *p)             {પાછા wrapped.mem.xSize(p);}
સ્થિર પૂર્ણાંક wrMemRoundup(પૂર્ણાંક n)            {પાછા wrapped.mem.xRoundup(n);}


સ્થિર પૂર્ણાંક wrMutexInit(શૂન્ય){
  પૂર્ણાંક rc;
  જો( wrapped.mutex_fail ){
    rc = SQLITE_ERROR;
  }નહીંતર{
    rc = wrapped.mutex.xMutexInit();
  }
  જો( rc==SQLITE_OK ){
    wrapped.mutex_init = 1;
  }
  પાછા rc;
}
સ્થિર પૂર્ણાંક wrMutexEnd(શૂન્ય){
  wrapped.mutex.xMutexEnd();
  wrapped.mutex_init = 0;
  પાછા SQLITE_OK;
}
સ્થિર sqlite3_mutex *wrMutexAlloc(પૂર્ણાંક e){
  પાછા wrapped.mutex.xMutexAlloc(e);
}
સ્થિર શૂન્ય wrMutexFree(sqlite3_mutex *p){
  wrapped.mutex.xMutexFree(p);
}
સ્થિર શૂન્ય wrMutexEnter(sqlite3_mutex *p){
  wrapped.mutex.xMutexEnter(p);
}
સ્થિર પૂર્ણાંક wrMutexTry(sqlite3_mutex *p){
  પાછા wrapped.mutex.xMutexTry(p);
}
સ્થિર શૂન્ય wrMutexLeave(sqlite3_mutex *p){
  wrapped.mutex.xMutexLeave(p);
}
સ્થિર પૂર્ણાંક wrMutexHeld(sqlite3_mutex *p){
  પાછા wrapped.mutex.xMutexHeld(p);
}
સ્થિર પૂર્ણાંક wrMutexNotheld(sqlite3_mutex *p){
  પાછા wrapped.mutex.xMutexNotheld(p);
}



સ્થિર પૂર્ણાંક wrPCacheInit(શૂન્ય *pArg){
  પૂર્ણાંક rc;
  જો( wrapped.pcache_fail ){
    rc = SQLITE_ERROR;
  }નહીંતર{
    rc = wrapped.pcache.xInit(wrapped.pcache.pArg);
  }
  જો( rc==SQLITE_OK ){
    wrapped.pcache_init = 1;
  }
  પાછા rc;
}
સ્થિર શૂન્ય wrPCacheShutdown(શૂન્ય *pArg){
  wrapped.pcache.xShutdown(wrapped.pcache.pArg);
  wrapped.pcache_init = 0;
}

સ્થિર sqlite3_pcache *wrPCacheCreate(પૂર્ણાંક a, પૂર્ણાંક b, પૂર્ણાંક c){
  પાછા wrapped.pcache.xCreate(a, b, c);
}  
સ્થિર શૂન્ય wrPCacheCachesize(sqlite3_pcache *p, પૂર્ણાંક n){
  wrapped.pcache.xCachesize(p, n);
}  
સ્થિર પૂર્ણાંક wrPCachePagecount(sqlite3_pcache *p){
  પાછા wrapped.pcache.xPagecount(p);
}  
સ્થિર sqlite3_pcache_page *wrPCacheFetch(sqlite3_pcache *p, unsigned a, પૂર્ણાંક b){
  પાછા wrapped.pcache.xFetch(p, a, b);
}  
સ્થિર શૂન્ય wrPCacheUnpin(sqlite3_pcache *p, sqlite3_pcache_page *a, પૂર્ણાંક b){
  wrapped.pcache.xUnpin(p, a, b);
}  
સ્થિર શૂન્ય wrPCacheRekey(
  sqlite3_pcache *p, 
  sqlite3_pcache_page *a, 
  unsigned b, 
  unsigned c
){
  wrapped.pcache.xRekey(p, a, b, c);
}  
સ્થિર શૂન્ય wrPCacheTruncate(sqlite3_pcache *p, unsigned a){
  wrapped.pcache.xTruncate(p, a);
}  
સ્થિર શૂન્ય wrPCacheDestroy(sqlite3_pcache *p){
  wrapped.pcache.xDestroy(p);
}  

સ્થિર શૂન્ય installInitWrappers(શૂન્ય){
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

  memset(&wrapped, 0, કદ(wrapped));

  sqlite3_shutdown();
  sqlite3_config(SQLITE_CONFIG_GETMUTEX, &wrapped.mutex);
  sqlite3_config(SQLITE_CONFIG_GETMALLOC, &wrapped.mem);
  sqlite3_config(SQLITE_CONFIG_GETPCACHE2, &wrapped.pcache);
  sqlite3_config(SQLITE_CONFIG_MUTEX, &mutexmethods);
  sqlite3_config(SQLITE_CONFIG_MALLOC, &memmethods);
  sqlite3_config(SQLITE_CONFIG_PCACHE2, &pcachemethods);
}

સ્થિર પૂર્ણાંક SQLITE_TCLAPI init_wrapper_install(
  ClientData clientData, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  પૂર્ણાંક i;
  installInitWrappers();
  માટે(i=1; i<objc; i++){
    અક્ષર *z = Tcl_GetString(objv[i]);
    જો( strcmp(z, "mem")==0 ){
      wrapped.mem_fail = 1;
    }નહીંતર જો( strcmp(z, "mutex")==0 ){
      wrapped.mutex_fail = 1;
    }નહીંતર જો( strcmp(z, "pcache")==0 ){
      wrapped.pcache_fail = 1;
    }નહીંતર{
      Tcl_AppendResult(interp, "Unknown argument: \"", z, "\"", ખાલી);
      પાછા TCL_ERROR;
    }
  }
  પાછા TCL_OK;
}

સ્થિર પૂર્ણાંક SQLITE_TCLAPI init_wrapper_uninstall(
  ClientData clientData, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  જો( objc!=1 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "");
    પાછા TCL_ERROR;
  }

  sqlite3_shutdown();
  sqlite3_config(SQLITE_CONFIG_MUTEX, &wrapped.mutex);
  sqlite3_config(SQLITE_CONFIG_MALLOC, &wrapped.mem);
  sqlite3_config(SQLITE_CONFIG_PCACHE2, &wrapped.pcache);
  પાછા TCL_OK;
}

સ્થિર પૂર્ણાંક SQLITE_TCLAPI init_wrapper_clear(
  ClientData clientData, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  જો( objc!=1 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "");
    પાછા TCL_ERROR;
  }

  wrapped.mem_fail = 0;
  wrapped.mutex_fail = 0;
  wrapped.pcache_fail = 0;
  પાછા TCL_OK;
}

સ્થિર પૂર્ણાંક SQLITE_TCLAPI init_wrapper_query(
  ClientData clientData, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  Tcl_Obj *pRet;

  જો( objc!=1 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "");
    પાછા TCL_ERROR;
  }

  pRet = Tcl_NewObj();
  જો( wrapped.mutex_init ){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj("mutex", -1));
  }
  જો( wrapped.mem_init ){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj("mem", -1));
  }
  જો( wrapped.pcache_init ){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj("pcache", -1));
  }

  Tcl_SetObjResult(interp, pRet);
  પાછા TCL_OK;
}

પૂર્ણાંક Sqlitetest_init_Init(Tcl_Interp *interp){
  સ્થિર સંરચના {
     અક્ષર *zName;
     Tcl_ObjCmdProc *xProc;
  } aObjCmd[] = {
    {"init_wrapper_install",   init_wrapper_install},
    {"init_wrapper_query",     init_wrapper_query  },
    {"init_wrapper_uninstall", init_wrapper_uninstall},
    {"init_wrapper_clear",     init_wrapper_clear}
  };
  પૂર્ણાંક i;

  માટે(i=0; i<કદ(aObjCmd)/કદ(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }

  પાછા TCL_OK;
}
