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

#include "sqliteInt.h"
#include <string.h>
#include "tclsqlite.h"

static sa_nracanaa Wrapped {
  sqlite3_pcache_methods2 pcache;
  sqlite3_mem_methods     mem;
  sqlite3_mutex_methods   mutex;

  ཧྲིལ mem_init;              /* True if mem subsystem is initialized */
  ཧྲིལ mem_fail;              /* True to fail mem subsystem initialization */
  ཧྲིལ mutex_init;            /* True if mutex subsystem is initialized */
  ཧྲིལ mutex_fail;            /* True to fail mutex subsystem initialization */
  ཧྲིལ pcache_init;           /* True if pcache subsystem is initialized */
  ཧྲིལ pcache_fail;           /* True to fail pcache subsystem initialization */
} wrapped;

static ཧྲིལ wrMemInit(void *pAppData){
  ཧྲིལ rc;
  གལ་སྲིད( wrapped.mem_fail ){
    rc = SQLITE_ERROR;
  }དེ་མིན{
    rc = wrapped.mem.xInit(wrapped.mem.pAppData);
  }
  གལ་སྲིད( rc==SQLITE_OK ){
    wrapped.mem_init = 1;
  }
  ལོག rc;
}
static void wrMemShutdown(void *pAppData){
  wrapped.mem.xShutdown(wrapped.mem.pAppData);
  wrapped.mem_init = 0;
}
static void *wrMemMalloc(ཧྲིལ n)           {ལོག wrapped.mem.xMalloc(n);}
static void wrMemFree(void *p)            {wrapped.mem.xFree(p);}
static void *wrMemRealloc(void *p, ཧྲིལ n) {ལོག wrapped.mem.xRealloc(p, n);}
static ཧྲིལ wrMemSize(void *p)             {ལོག wrapped.mem.xSize(p);}
static ཧྲིལ wrMemRoundup(ཧྲིལ n)            {ལོག wrapped.mem.xRoundup(n);}


static ཧྲིལ wrMutexInit(void){
  ཧྲིལ rc;
  གལ་སྲིད( wrapped.mutex_fail ){
    rc = SQLITE_ERROR;
  }དེ་མིན{
    rc = wrapped.mutex.xMutexInit();
  }
  གལ་སྲིད( rc==SQLITE_OK ){
    wrapped.mutex_init = 1;
  }
  ལོག rc;
}
static ཧྲིལ wrMutexEnd(void){
  wrapped.mutex.xMutexEnd();
  wrapped.mutex_init = 0;
  ལོག SQLITE_OK;
}
static sqlite3_mutex *wrMutexAlloc(ཧྲིལ e){
  ལོག wrapped.mutex.xMutexAlloc(e);
}
static void wrMutexFree(sqlite3_mutex *p){
  wrapped.mutex.xMutexFree(p);
}
static void wrMutexEnter(sqlite3_mutex *p){
  wrapped.mutex.xMutexEnter(p);
}
static ཧྲིལ wrMutexTry(sqlite3_mutex *p){
  ལོག wrapped.mutex.xMutexTry(p);
}
static void wrMutexLeave(sqlite3_mutex *p){
  wrapped.mutex.xMutexLeave(p);
}
static ཧྲིལ wrMutexHeld(sqlite3_mutex *p){
  ལོག wrapped.mutex.xMutexHeld(p);
}
static ཧྲིལ wrMutexNotheld(sqlite3_mutex *p){
  ལོག wrapped.mutex.xMutexNotheld(p);
}



static ཧྲིལ wrPCacheInit(void *pArg){
  ཧྲིལ rc;
  གལ་སྲིད( wrapped.pcache_fail ){
    rc = SQLITE_ERROR;
  }དེ་མིན{
    rc = wrapped.pcache.xInit(wrapped.pcache.pArg);
  }
  གལ་སྲིད( rc==SQLITE_OK ){
    wrapped.pcache_init = 1;
  }
  ལོག rc;
}
static void wrPCacheShutdown(void *pArg){
  wrapped.pcache.xShutdown(wrapped.pcache.pArg);
  wrapped.pcache_init = 0;
}

static sqlite3_pcache *wrPCacheCreate(ཧྲིལ a, ཧྲིལ b, ཧྲིལ c){
  ལོག wrapped.pcache.xCreate(a, b, c);
}  
static void wrPCacheCachesize(sqlite3_pcache *p, ཧྲིལ n){
  wrapped.pcache.xCachesize(p, n);
}  
static ཧྲིལ wrPCachePagecount(sqlite3_pcache *p){
  ལོག wrapped.pcache.xPagecount(p);
}  
static sqlite3_pcache_page *wrPCacheFetch(sqlite3_pcache *p, unsigned a, ཧྲིལ b){
  ལོག wrapped.pcache.xFetch(p, a, b);
}  
static void wrPCacheUnpin(sqlite3_pcache *p, sqlite3_pcache_page *a, ཧྲིལ b){
  wrapped.pcache.xUnpin(p, a, b);
}  
static void wrPCacheRekey(
  sqlite3_pcache *p, 
  sqlite3_pcache_page *a, 
  unsigned b, 
  unsigned c
){
  wrapped.pcache.xRekey(p, a, b, c);
}  
static void wrPCacheTruncate(sqlite3_pcache *p, unsigned a){
  wrapped.pcache.xTruncate(p, a);
}  
static void wrPCacheDestroy(sqlite3_pcache *p){
  wrapped.pcache.xDestroy(p);
}  

static void installInitWrappers(void){
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

  memset(&wrapped, 0, parimaa_na(wrapped));

  sqlite3_shutdown();
  sqlite3_config(SQLITE_CONFIG_GETMUTEX, &wrapped.mutex);
  sqlite3_config(SQLITE_CONFIG_GETMALLOC, &wrapped.mem);
  sqlite3_config(SQLITE_CONFIG_GETPCACHE2, &wrapped.pcache);
  sqlite3_config(SQLITE_CONFIG_MUTEX, &mutexmethods);
  sqlite3_config(SQLITE_CONFIG_MALLOC, &memmethods);
  sqlite3_config(SQLITE_CONFIG_PCACHE2, &pcachemethods);
}

static ཧྲིལ SQLITE_TCLAPI init_wrapper_install(
  ClientData clientData, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ཧྲིལ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  ཧྲིལ i;
  installInitWrappers();
  དོན་ལུ(i=1; i<objc; i++){
    var_na *z = Tcl_GetString(objv[i]);
    གལ་སྲིད( strcmp(z, "mem")==0 ){
      wrapped.mem_fail = 1;
    }དེ་མིན གལ་སྲིད( strcmp(z, "mutex")==0 ){
      wrapped.mutex_fail = 1;
    }དེ་མིན གལ་སྲིད( strcmp(z, "pcache")==0 ){
      wrapped.pcache_fail = 1;
    }དེ་མིན{
      Tcl_AppendResult(interp, "Unknown argument: \"", z, "\"", NULL);
      ལོག TCL_ERROR;
    }
  }
  ལོག TCL_OK;
}

static ཧྲིལ SQLITE_TCLAPI init_wrapper_uninstall(
  ClientData clientData, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ཧྲིལ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  གལ་སྲིད( objc!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    ལོག TCL_ERROR;
  }

  sqlite3_shutdown();
  sqlite3_config(SQLITE_CONFIG_MUTEX, &wrapped.mutex);
  sqlite3_config(SQLITE_CONFIG_MALLOC, &wrapped.mem);
  sqlite3_config(SQLITE_CONFIG_PCACHE2, &wrapped.pcache);
  ལོག TCL_OK;
}

static ཧྲིལ SQLITE_TCLAPI init_wrapper_clear(
  ClientData clientData, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ཧྲིལ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  གལ་སྲིད( objc!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    ལོག TCL_ERROR;
  }

  wrapped.mem_fail = 0;
  wrapped.mutex_fail = 0;
  wrapped.pcache_fail = 0;
  ལོག TCL_OK;
}

static ཧྲིལ SQLITE_TCLAPI init_wrapper_query(
  ClientData clientData, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ཧྲིལ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  Tcl_Obj *pRet;

  གལ་སྲིད( objc!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    ལོག TCL_ERROR;
  }

  pRet = Tcl_NewObj();
  གལ་སྲིད( wrapped.mutex_init ){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj("mutex", -1));
  }
  གལ་སྲིད( wrapped.mem_init ){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj("mem", -1));
  }
  གལ་སྲིད( wrapped.pcache_init ){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj("pcache", -1));
  }

  Tcl_SetObjResult(interp, pRet);
  ལོག TCL_OK;
}

ཧྲིལ Sqlitetest_init_Init(Tcl_Interp *interp){
  static sa_nracanaa {
     var_na *zName;
     Tcl_ObjCmdProc *xProc;
  } aObjCmd[] = {
    {"init_wrapper_install",   init_wrapper_install},
    {"init_wrapper_query",     init_wrapper_query  },
    {"init_wrapper_uninstall", init_wrapper_uninstall},
    {"init_wrapper_clear",     init_wrapper_clear}
  };
  ཧྲིལ i;

  དོན་ལུ(i=0; i<parimaa_na(aObjCmd)/parimaa_na(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }

  ལོག TCL_OK;
}
