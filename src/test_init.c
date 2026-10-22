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

ساکن ساخت Wrapped {
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

ساکن صحیح wrMemInit(خالی *p_App_ڈیٹا){
  صحیح rc;
  اگر( wrapped.mem_fail ){
    rc = SQLITE_ERROR;
  }ورنہ{
    rc = wrapped.mem.xInit(wrapped.mem.p_App_ڈیٹا);
  }
  اگر( rc==SQLITE_OK ){
    wrapped.mem_init = 1;
  }
  واپس rc;
}
ساکن خالی wrMemShutdown(خالی *p_App_ڈیٹا){
  wrapped.mem.xShutdown(wrapped.mem.p_App_ڈیٹا);
  wrapped.mem_init = 0;
}
ساکن خالی *wrMemMalloc(صحیح n)           {واپس wrapped.mem.xMalloc(n);}
ساکن خالی wrMemFree(خالی *p)            {wrapped.mem.xFree(p);}
ساکن خالی *wrMemRealloc(خالی *p, صحیح n) {واپس wrapped.mem.xRealloc(p, n);}
ساکن صحیح wr_Mem_سائز(خالی *p)             {واپس wrapped.mem.x_سائز(p);}
ساکن صحیح wrMemRoundup(صحیح n)            {واپس wrapped.mem.xRoundup(n);}


ساکن صحیح wrMutexInit(خالی){
  صحیح rc;
  اگر( wrapped.mutex_fail ){
    rc = SQLITE_ERROR;
  }ورنہ{
    rc = wrapped.mutex.xMutexInit();
  }
  اگر( rc==SQLITE_OK ){
    wrapped.mutex_init = 1;
  }
  واپس rc;
}
ساکن صحیح wr_Mutex_انتہا(خالی){
  wrapped.mutex.x_Mutex_انتہا();
  wrapped.mutex_init = 0;
  واپس SQLITE_OK;
}
ساکن sqlite3_mutex *wrMutexAlloc(صحیح e){
  واپس wrapped.mutex.xMutexAlloc(e);
}
ساکن خالی wrMutexFree(sqlite3_mutex *p){
  wrapped.mutex.xMutexFree(p);
}
ساکن خالی wrMutexEnter(sqlite3_mutex *p){
  wrapped.mutex.xMutexEnter(p);
}
ساکن صحیح wrMutexTry(sqlite3_mutex *p){
  واپس wrapped.mutex.xMutexTry(p);
}
ساکن خالی wrMutexLeave(sqlite3_mutex *p){
  wrapped.mutex.xMutexLeave(p);
}
ساکن صحیح wrMutexHeld(sqlite3_mutex *p){
  واپس wrapped.mutex.xMutexHeld(p);
}
ساکن صحیح wrMutexNotheld(sqlite3_mutex *p){
  واپس wrapped.mutex.xMutexNotheld(p);
}



ساکن صحیح wrPCacheInit(خالی *pArg){
  صحیح rc;
  اگر( wrapped.pcache_fail ){
    rc = SQLITE_ERROR;
  }ورنہ{
    rc = wrapped.pcache.xInit(wrapped.pcache.pArg);
  }
  اگر( rc==SQLITE_OK ){
    wrapped.pcache_init = 1;
  }
  واپس rc;
}
ساکن خالی wrPCacheShutdown(خالی *pArg){
  wrapped.pcache.xShutdown(wrapped.pcache.pArg);
  wrapped.pcache_init = 0;
}

ساکن sqlite3_pcache *wrPCacheCreate(صحیح a, صحیح b, صحیح c){
  واپس wrapped.pcache.xCreate(a, b, c);
}  
ساکن خالی wrPCacheCachesize(sqlite3_pcache *p, صحیح n){
  wrapped.pcache.xCachesize(p, n);
}  
ساکن صحیح wrPCachePagecount(sqlite3_pcache *p){
  واپس wrapped.pcache.xPagecount(p);
}  
ساکن sqlite3_pcache_صفحہ *wrPCacheFetch(sqlite3_pcache *p, غیرعلامتی a, صحیح b){
  واپس wrapped.pcache.xFetch(p, a, b);
}  
ساکن خالی wrPCacheUnpin(sqlite3_pcache *p, sqlite3_pcache_صفحہ *a, صحیح b){
  wrapped.pcache.xUnpin(p, a, b);
}  
ساکن خالی wrPCacheRekey(
  sqlite3_pcache *p, 
  sqlite3_pcache_صفحہ *a, 
  غیرعلامتی b, 
  غیرعلامتی c
){
  wrapped.pcache.xRekey(p, a, b, c);
}  
ساکن خالی wrPCacheTruncate(sqlite3_pcache *p, غیرعلامتی a){
  wrapped.pcache.xTruncate(p, a);
}  
ساکن خالی wrPCacheDestroy(sqlite3_pcache *p){
  wrapped.pcache.xDestroy(p);
}  

ساکن خالی installInitWrappers(خالی){
  sqlite3_mutex_methods mutexmethods = {
    wrMutexInit,  wr_Mutex_انتہا,   wrMutexAlloc,
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
    wr_Mem_سائز,     wrMemRoundup, wrMemInit,
    wrMemShutdown,
    0
  };

  memset(&wrapped, 0, حجم(wrapped));

  sqlite3_shutdown();
  sqlite3_config(SQLITE_CONFIG_GETMUTEX, &wrapped.mutex);
  sqlite3_config(SQLITE_CONFIG_GETMALLOC, &wrapped.mem);
  sqlite3_config(SQLITE_CONFIG_GETPCACHE2, &wrapped.pcache);
  sqlite3_config(SQLITE_CONFIG_MUTEX, &mutexmethods);
  sqlite3_config(SQLITE_CONFIG_MALLOC, &memmethods);
  sqlite3_config(SQLITE_CONFIG_PCACHE2, &pcachemethods);
}

ساکن صحیح SQLITE_TCLAPI init_wrapper_install(
  Client_ڈیٹا client_ڈیٹا, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  صحیح i;
  installInitWrappers();
  برائے(i=1; i<objc; i++){
    حرف *z = Tcl_GetString(objv[i]);
    اگر( strcmp(z, "mem")==0 ){
      wrapped.mem_fail = 1;
    }ورنہ اگر( strcmp(z, "mutex")==0 ){
      wrapped.mutex_fail = 1;
    }ورنہ اگر( strcmp(z, "pcache")==0 ){
      wrapped.pcache_fail = 1;
    }ورنہ{
      Tcl_Append_نتیجہ(interp, "Unknown argument: \"", z, "\"", تہی);
      واپس TCL_ERROR;
    }
  }
  واپس TCL_OK;
}

ساکن صحیح SQLITE_TCLAPI init_wrapper_uninstall(
  Client_ڈیٹا client_ڈیٹا, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  اگر( objc!=1 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "");
    واپس TCL_ERROR;
  }

  sqlite3_shutdown();
  sqlite3_config(SQLITE_CONFIG_MUTEX, &wrapped.mutex);
  sqlite3_config(SQLITE_CONFIG_MALLOC, &wrapped.mem);
  sqlite3_config(SQLITE_CONFIG_PCACHE2, &wrapped.pcache);
  واپس TCL_OK;
}

ساکن صحیح SQLITE_TCLAPI init_wrapper_clear(
  Client_ڈیٹا client_ڈیٹا, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  اگر( objc!=1 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "");
    واپس TCL_ERROR;
  }

  wrapped.mem_fail = 0;
  wrapped.mutex_fail = 0;
  wrapped.pcache_fail = 0;
  واپس TCL_OK;
}

ساکن صحیح SQLITE_TCLAPI init_wrapper_query(
  Client_ڈیٹا client_ڈیٹا, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  Tcl_Obj *pRet;

  اگر( objc!=1 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "");
    واپس TCL_ERROR;
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

  Tcl_سیٹ_Obj_نتیجہ(interp, pRet);
  واپس TCL_OK;
}

صحیح Sqlitetest_init_Init(Tcl_Interp *interp){
  ساکن ساخت {
     حرف *zName;
     Tcl_ObjCmdProc *xProc;
  } aObjCmd[] = {
    {"init_wrapper_install",   init_wrapper_install},
    {"init_wrapper_query",     init_wrapper_query  },
    {"init_wrapper_uninstall", init_wrapper_uninstall},
    {"init_wrapper_clear",     init_wrapper_clear}
  };
  صحیح i;

  برائے(i=0; i<حجم(aObjCmd)/حجم(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }

  واپس TCL_OK;
}
