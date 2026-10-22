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

#ଅନ୍ତର୍ଭାଵ "sqliteInt.h"
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ "tclsqlite.h"

ସ୍ଥିର ସଂରଚନା Wrapped {
  sqlite3_pcache_methods2 pcache;
  sqlite3_mem_methods     mem;
  sqlite3_mutex_methods   mutex;

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା mem_init;              /* True if mem subsystem is initialized */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା mem_fail;              /* True to fail mem subsystem initialization */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା mutex_init;            /* True if mutex subsystem is initialized */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା mutex_fail;            /* True to fail mutex subsystem initialization */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା pcache_init;           /* True if pcache subsystem is initialized */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା pcache_fail;           /* True to fail pcache subsystem initialization */
} wrapped;

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା wrMemInit(ଶୂନ୍ୟ *pAppData){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ଯଦି( wrapped.mem_fail ){
    rc = SQLITE_ERROR;
  }ଅନ୍ୟଥା{
    rc = wrapped.mem.xInit(wrapped.mem.pAppData);
  }
  ଯଦି( rc==SQLITE_OK ){
    wrapped.mem_init = 1;
  }
  ଫେରନ୍ତୁ rc;
}
ସ୍ଥିର ଶୂନ୍ୟ wrMemShutdown(ଶୂନ୍ୟ *pAppData){
  wrapped.mem.xShutdown(wrapped.mem.pAppData);
  wrapped.mem_init = 0;
}
ସ୍ଥିର ଶୂନ୍ୟ *wrMemMalloc(ପୂର୍ଣ୍ଣସଂଖ୍ୟା n)           {ଫେରନ୍ତୁ wrapped.mem.xMalloc(n);}
ସ୍ଥିର ଶୂନ୍ୟ wrMemFree(ଶୂନ୍ୟ *p)            {wrapped.mem.xFree(p);}
ସ୍ଥିର ଶୂନ୍ୟ *wrMemRealloc(ଶୂନ୍ୟ *p, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n) {ଫେରନ୍ତୁ wrapped.mem.xRealloc(p, n);}
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା wrMemSize(ଶୂନ୍ୟ *p)             {ଫେରନ୍ତୁ wrapped.mem.xSize(p);}
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା wrMemRoundup(ପୂର୍ଣ୍ଣସଂଖ୍ୟା n)            {ଫେରନ୍ତୁ wrapped.mem.xRoundup(n);}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା wrMutexInit(ଶୂନ୍ୟ){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ଯଦି( wrapped.mutex_fail ){
    rc = SQLITE_ERROR;
  }ଅନ୍ୟଥା{
    rc = wrapped.mutex.xMutexInit();
  }
  ଯଦି( rc==SQLITE_OK ){
    wrapped.mutex_init = 1;
  }
  ଫେରନ୍ତୁ rc;
}
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା wrMutexEnd(ଶୂନ୍ୟ){
  wrapped.mutex.xMutexEnd();
  wrapped.mutex_init = 0;
  ଫେରନ୍ତୁ SQLITE_OK;
}
ସ୍ଥିର sqlite3_mutex *wrMutexAlloc(ପୂର୍ଣ୍ଣସଂଖ୍ୟା e){
  ଫେରନ୍ତୁ wrapped.mutex.xMutexAlloc(e);
}
ସ୍ଥିର ଶୂନ୍ୟ wrMutexFree(sqlite3_mutex *p){
  wrapped.mutex.xMutexFree(p);
}
ସ୍ଥିର ଶୂନ୍ୟ wrMutexEnter(sqlite3_mutex *p){
  wrapped.mutex.xMutexEnter(p);
}
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା wrMutexTry(sqlite3_mutex *p){
  ଫେରନ୍ତୁ wrapped.mutex.xMutexTry(p);
}
ସ୍ଥିର ଶୂନ୍ୟ wrMutexLeave(sqlite3_mutex *p){
  wrapped.mutex.xMutexLeave(p);
}
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା wrMutexHeld(sqlite3_mutex *p){
  ଫେରନ୍ତୁ wrapped.mutex.xMutexHeld(p);
}
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା wrMutexNotheld(sqlite3_mutex *p){
  ଫେରନ୍ତୁ wrapped.mutex.xMutexNotheld(p);
}



ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା wrPCacheInit(ଶୂନ୍ୟ *pArg){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ଯଦି( wrapped.pcache_fail ){
    rc = SQLITE_ERROR;
  }ଅନ୍ୟଥା{
    rc = wrapped.pcache.xInit(wrapped.pcache.pArg);
  }
  ଯଦି( rc==SQLITE_OK ){
    wrapped.pcache_init = 1;
  }
  ଫେରନ୍ତୁ rc;
}
ସ୍ଥିର ଶୂନ୍ୟ wrPCacheShutdown(ଶୂନ୍ୟ *pArg){
  wrapped.pcache.xShutdown(wrapped.pcache.pArg);
  wrapped.pcache_init = 0;
}

ସ୍ଥିର sqlite3_pcache *wrPCacheCreate(ପୂର୍ଣ୍ଣସଂଖ୍ୟା a, ପୂର୍ଣ୍ଣସଂଖ୍ୟା b, ପୂର୍ଣ୍ଣସଂଖ୍ୟା c){
  ଫେରନ୍ତୁ wrapped.pcache.xCreate(a, b, c);
}  
ସ୍ଥିର ଶୂନ୍ୟ wrPCacheCachesize(sqlite3_pcache *p, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n){
  wrapped.pcache.xCachesize(p, n);
}  
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା wrPCachePagecount(sqlite3_pcache *p){
  ଫେରନ୍ତୁ wrapped.pcache.xPagecount(p);
}  
ସ୍ଥିର sqlite3_pcache_page *wrPCacheFetch(sqlite3_pcache *p, unsigned a, ପୂର୍ଣ୍ଣସଂଖ୍ୟା b){
  ଫେରନ୍ତୁ wrapped.pcache.xFetch(p, a, b);
}  
ସ୍ଥିର ଶୂନ୍ୟ wrPCacheUnpin(sqlite3_pcache *p, sqlite3_pcache_page *a, ପୂର୍ଣ୍ଣସଂଖ୍ୟା b){
  wrapped.pcache.xUnpin(p, a, b);
}  
ସ୍ଥିର ଶୂନ୍ୟ wrPCacheRekey(
  sqlite3_pcache *p, 
  sqlite3_pcache_page *a, 
  unsigned b, 
  unsigned c
){
  wrapped.pcache.xRekey(p, a, b, c);
}  
ସ୍ଥିର ଶୂନ୍ୟ wrPCacheTruncate(sqlite3_pcache *p, unsigned a){
  wrapped.pcache.xTruncate(p, a);
}  
ସ୍ଥିର ଶୂନ୍ୟ wrPCacheDestroy(sqlite3_pcache *p){
  wrapped.pcache.xDestroy(p);
}  

ସ୍ଥିର ଶୂନ୍ୟ installInitWrappers(ଶୂନ୍ୟ){
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

  memset(&wrapped, 0, ଆକାର(wrapped));

  sqlite3_shutdown();
  sqlite3_config(SQLITE_CONFIG_GETMUTEX, &wrapped.mutex);
  sqlite3_config(SQLITE_CONFIG_GETMALLOC, &wrapped.mem);
  sqlite3_config(SQLITE_CONFIG_GETPCACHE2, &wrapped.pcache);
  sqlite3_config(SQLITE_CONFIG_MUTEX, &mutexmethods);
  sqlite3_config(SQLITE_CONFIG_MALLOC, &memmethods);
  sqlite3_config(SQLITE_CONFIG_PCACHE2, &pcachemethods);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI init_wrapper_install(
  ClientData clientData, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  installInitWrappers();
  ପାଇଁ(i=1; i<objc; i++){
    ଅକ୍ଷର *z = Tcl_GetString(objv[i]);
    ଯଦି( strcmp(z, "mem")==0 ){
      wrapped.mem_fail = 1;
    }ଅନ୍ୟଥା ଯଦି( strcmp(z, "mutex")==0 ){
      wrapped.mutex_fail = 1;
    }ଅନ୍ୟଥା ଯଦି( strcmp(z, "pcache")==0 ){
      wrapped.pcache_fail = 1;
    }ଅନ୍ୟଥା{
      Tcl_AppendResult(interp, "Unknown argument: \"", z, "\"", ଖାଲି);
      ଫେରନ୍ତୁ TCL_ERROR;
    }
  }
  ଫେରନ୍ତୁ TCL_OK;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI init_wrapper_uninstall(
  ClientData clientData, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  ଯଦି( objc!=1 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "");
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  sqlite3_shutdown();
  sqlite3_config(SQLITE_CONFIG_MUTEX, &wrapped.mutex);
  sqlite3_config(SQLITE_CONFIG_MALLOC, &wrapped.mem);
  sqlite3_config(SQLITE_CONFIG_PCACHE2, &wrapped.pcache);
  ଫେରନ୍ତୁ TCL_OK;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI init_wrapper_clear(
  ClientData clientData, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  ଯଦି( objc!=1 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "");
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  wrapped.mem_fail = 0;
  wrapped.mutex_fail = 0;
  wrapped.pcache_fail = 0;
  ଫେରନ୍ତୁ TCL_OK;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI init_wrapper_query(
  ClientData clientData, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  Tcl_Obj *pRet;

  ଯଦି( objc!=1 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "");
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  pRet = Tcl_NewObj();
  ଯଦି( wrapped.mutex_init ){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj("mutex", -1));
  }
  ଯଦି( wrapped.mem_init ){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj("mem", -1));
  }
  ଯଦି( wrapped.pcache_init ){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj("pcache", -1));
  }

  Tcl_SetObjResult(interp, pRet);
  ଫେରନ୍ତୁ TCL_OK;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା Sqlitetest_init_Init(Tcl_Interp *interp){
  ସ୍ଥିର ସଂରଚନା {
     ଅକ୍ଷର *zName;
     Tcl_ObjCmdProc *xProc;
  } aObjCmd[] = {
    {"init_wrapper_install",   init_wrapper_install},
    {"init_wrapper_query",     init_wrapper_query  },
    {"init_wrapper_uninstall", init_wrapper_uninstall},
    {"init_wrapper_clear",     init_wrapper_clear}
  };
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

  ପାଇଁ(i=0; i<ଆକାର(aObjCmd)/ଆକାର(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }

  ଫେରନ୍ତୁ TCL_OK;
}
