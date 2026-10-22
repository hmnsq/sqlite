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

#ضمن "sqliteInt.h"
#ضمن <string.h>
#ضمن "tclsqlite.h"

ساكن بنية Wrapped {
  sqlite3_pcache_methods2 pcache;
  sqlite3_mem_methods     mem;
  sqlite3_mutex_methods   mutex;

  صحيح mem_init;              /* True if mem subsystem is initialized */
  صحيح mem_fail;              /* True to fail mem subsystem initialization */
  صحيح mutex_init;            /* True if mutex subsystem is initialized */
  صحيح mutex_fail;            /* True to fail mutex subsystem initialization */
  صحيح pcache_init;           /* True if pcache subsystem is initialized */
  صحيح pcache_fail;           /* True to fail pcache subsystem initialization */
} wrapped;

ساكن صحيح wrMemInit(فراغ *p_App_داده){
  صحيح rc;
  إذا( wrapped.mem_fail ){
    rc = SQLITE_ERROR;
  }وإلا{
    rc = wrapped.mem.xInit(wrapped.mem.p_App_داده);
  }
  إذا( rc==SQLITE_OK ){
    wrapped.mem_init = 1;
  }
  أرجع rc;
}
ساكن فراغ wrMemShutdown(فراغ *p_App_داده){
  wrapped.mem.xShutdown(wrapped.mem.p_App_داده);
  wrapped.mem_init = 0;
}
ساكن فراغ *wrMemMalloc(صحيح n)           {أرجع wrapped.mem.xMalloc(n);}
ساكن فراغ wrMemFree(فراغ *p)            {wrapped.mem.xFree(p);}
ساكن فراغ *wrMemRealloc(فراغ *p, صحيح n) {أرجع wrapped.mem.xRealloc(p, n);}
ساكن صحيح wr_Mem_اندازه(فراغ *p)             {أرجع wrapped.mem.x_اندازه(p);}
ساكن صحيح wrMemRoundup(صحيح n)            {أرجع wrapped.mem.xRoundup(n);}


ساكن صحيح wrMutexInit(فراغ){
  صحيح rc;
  إذا( wrapped.mutex_fail ){
    rc = SQLITE_ERROR;
  }وإلا{
    rc = wrapped.mutex.xMutexInit();
  }
  إذا( rc==SQLITE_OK ){
    wrapped.mutex_init = 1;
  }
  أرجع rc;
}
ساكن صحيح wr_Mutex_انتها(فراغ){
  wrapped.mutex.x_Mutex_انتها();
  wrapped.mutex_init = 0;
  أرجع SQLITE_OK;
}
ساكن sqlite3_mutex *wrMutexAlloc(صحيح e){
  أرجع wrapped.mutex.xMutexAlloc(e);
}
ساكن فراغ wrMutexFree(sqlite3_mutex *p){
  wrapped.mutex.xMutexFree(p);
}
ساكن فراغ wrMutexEnter(sqlite3_mutex *p){
  wrapped.mutex.xMutexEnter(p);
}
ساكن صحيح wrMutexTry(sqlite3_mutex *p){
  أرجع wrapped.mutex.xMutexTry(p);
}
ساكن فراغ wrMutexLeave(sqlite3_mutex *p){
  wrapped.mutex.xMutexLeave(p);
}
ساكن صحيح wrMutexHeld(sqlite3_mutex *p){
  أرجع wrapped.mutex.xMutexHeld(p);
}
ساكن صحيح wrMutexNotheld(sqlite3_mutex *p){
  أرجع wrapped.mutex.xMutexNotheld(p);
}



ساكن صحيح wrPCacheInit(فراغ *pArg){
  صحيح rc;
  إذا( wrapped.pcache_fail ){
    rc = SQLITE_ERROR;
  }وإلا{
    rc = wrapped.pcache.xInit(wrapped.pcache.pArg);
  }
  إذا( rc==SQLITE_OK ){
    wrapped.pcache_init = 1;
  }
  أرجع rc;
}
ساكن فراغ wrPCacheShutdown(فراغ *pArg){
  wrapped.pcache.xShutdown(wrapped.pcache.pArg);
  wrapped.pcache_init = 0;
}

ساكن sqlite3_pcache *wrPCacheCreate(صحيح a, صحيح b, صحيح c){
  أرجع wrapped.pcache.xCreate(a, b, c);
}  
ساكن فراغ wrPCacheCachesize(sqlite3_pcache *p, صحيح n){
  wrapped.pcache.xCachesize(p, n);
}  
ساكن صحيح wrPCachePagecount(sqlite3_pcache *p){
  أرجع wrapped.pcache.xPagecount(p);
}  
ساكن sqlite3_pcache_صفحه *wrPCacheFetch(sqlite3_pcache *p, بیعلامت a, صحيح b){
  أرجع wrapped.pcache.xFetch(p, a, b);
}  
ساكن فراغ wrPCacheUnpin(sqlite3_pcache *p, sqlite3_pcache_صفحه *a, صحيح b){
  wrapped.pcache.xUnpin(p, a, b);
}  
ساكن فراغ wrPCacheRekey(
  sqlite3_pcache *p, 
  sqlite3_pcache_صفحه *a, 
  بیعلامت b, 
  بیعلامت c
){
  wrapped.pcache.xRekey(p, a, b, c);
}  
ساكن فراغ wrPCacheTruncate(sqlite3_pcache *p, بیعلامت a){
  wrapped.pcache.xTruncate(p, a);
}  
ساكن فراغ wrPCacheDestroy(sqlite3_pcache *p){
  wrapped.pcache.xDestroy(p);
}  

ساكن فراغ installInitWrappers(فراغ){
  sqlite3_mutex_methods mutexmethods = {
    wrMutexInit,  wr_Mutex_انتها,   wrMutexAlloc,
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
    wr_Mem_اندازه,     wrMemRoundup, wrMemInit,
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

ساكن صحيح SQLITE_TCLAPI init_wrapper_install(
  Client_داده client_داده, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  صحيح i;
  installInitWrappers();
  لكل(i=1; i<objc; i++){
    حرف *z = Tcl_GetString(objv[i]);
    إذا( strcmp(z, "mem")==0 ){
      wrapped.mem_fail = 1;
    }وإلا إذا( strcmp(z, "mutex")==0 ){
      wrapped.mutex_fail = 1;
    }وإلا إذا( strcmp(z, "pcache")==0 ){
      wrapped.pcache_fail = 1;
    }وإلا{
      Tcl_Append_نتیجه(interp, "Unknown argument: \"", z, "\"", عدم);
      أرجع TCL_ERROR;
    }
  }
  أرجع TCL_OK;
}

ساكن صحيح SQLITE_TCLAPI init_wrapper_uninstall(
  Client_داده client_داده, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  إذا( objc!=1 ){
    Tcl_Wrong_رقم_آرگومانها(interp, 1, objv, "");
    أرجع TCL_ERROR;
  }

  sqlite3_shutdown();
  sqlite3_config(SQLITE_CONFIG_MUTEX, &wrapped.mutex);
  sqlite3_config(SQLITE_CONFIG_MALLOC, &wrapped.mem);
  sqlite3_config(SQLITE_CONFIG_PCACHE2, &wrapped.pcache);
  أرجع TCL_OK;
}

ساكن صحيح SQLITE_TCLAPI init_wrapper_clear(
  Client_داده client_داده, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  إذا( objc!=1 ){
    Tcl_Wrong_رقم_آرگومانها(interp, 1, objv, "");
    أرجع TCL_ERROR;
  }

  wrapped.mem_fail = 0;
  wrapped.mutex_fail = 0;
  wrapped.pcache_fail = 0;
  أرجع TCL_OK;
}

ساكن صحيح SQLITE_TCLAPI init_wrapper_query(
  Client_داده client_داده, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  Tcl_Obj *pRet;

  إذا( objc!=1 ){
    Tcl_Wrong_رقم_آرگومانها(interp, 1, objv, "");
    أرجع TCL_ERROR;
  }

  pRet = Tcl_NewObj();
  إذا( wrapped.mutex_init ){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj("mutex", -1));
  }
  إذا( wrapped.mem_init ){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj("mem", -1));
  }
  إذا( wrapped.pcache_init ){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj("pcache", -1));
  }

  Tcl_مجموعه_Obj_نتیجه(interp, pRet);
  أرجع TCL_OK;
}

صحيح Sqlitetest_init_Init(Tcl_Interp *interp){
  ساكن بنية {
     حرف *zName;
     Tcl_ObjCmdProc *xProc;
  } aObjCmd[] = {
    {"init_wrapper_install",   init_wrapper_install},
    {"init_wrapper_query",     init_wrapper_query  },
    {"init_wrapper_uninstall", init_wrapper_uninstall},
    {"init_wrapper_clear",     init_wrapper_clear}
  };
  صحيح i;

  لكل(i=0; i<حجم(aObjCmd)/حجم(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }

  أرجع TCL_OK;
}
