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

static struct Wrapped {
  sqlite3_pcache_methods2 pcache;
  sqlite3_mem_methods     mem;
  sqlite3_mutex_methods   mutex;

  整数 mem_init;              /* True if mem subsystem is initialized */
  整数 mem_fail;              /* True to fail mem subsystem initialization */
  整数 mutex_init;            /* True if mutex subsystem is initialized */
  整数 mutex_fail;            /* True to fail mutex subsystem initialization */
  整数 pcache_init;           /* True if pcache subsystem is initialized */
  整数 pcache_fail;           /* True to fail pcache subsystem initialization */
} wrapped;

static 整数 wrMemInit(void *pAppData){
  整数 rc;
  如果( wrapped.mem_fail ){
    rc = SQLITE_ERROR;
  }否则{
    rc = wrapped.mem.xInit(wrapped.mem.pAppData);
  }
  如果( rc==SQLITE_OK ){
    wrapped.mem_init = 1;
  }
  返回 rc;
}
static void wrMemShutdown(void *pAppData){
  wrapped.mem.xShutdown(wrapped.mem.pAppData);
  wrapped.mem_init = 0;
}
static void *wrMemMalloc(整数 n)           {返回 wrapped.mem.xMalloc(n);}
static void wrMemFree(void *p)            {wrapped.mem.xFree(p);}
static void *wrMemRealloc(void *p, 整数 n) {返回 wrapped.mem.xRealloc(p, n);}
static 整数 wrMemSize(void *p)             {返回 wrapped.mem.xSize(p);}
static 整数 wrMemRoundup(整数 n)            {返回 wrapped.mem.xRoundup(n);}


static 整数 wrMutexInit(void){
  整数 rc;
  如果( wrapped.mutex_fail ){
    rc = SQLITE_ERROR;
  }否则{
    rc = wrapped.mutex.xMutexInit();
  }
  如果( rc==SQLITE_OK ){
    wrapped.mutex_init = 1;
  }
  返回 rc;
}
static 整数 wrMutexEnd(void){
  wrapped.mutex.xMutexEnd();
  wrapped.mutex_init = 0;
  返回 SQLITE_OK;
}
static sqlite3_mutex *wrMutexAlloc(整数 e){
  返回 wrapped.mutex.xMutexAlloc(e);
}
static void wrMutexFree(sqlite3_mutex *p){
  wrapped.mutex.xMutexFree(p);
}
static void wrMutexEnter(sqlite3_mutex *p){
  wrapped.mutex.xMutexEnter(p);
}
static 整数 wrMutexTry(sqlite3_mutex *p){
  返回 wrapped.mutex.xMutexTry(p);
}
static void wrMutexLeave(sqlite3_mutex *p){
  wrapped.mutex.xMutexLeave(p);
}
static 整数 wrMutexHeld(sqlite3_mutex *p){
  返回 wrapped.mutex.xMutexHeld(p);
}
static 整数 wrMutexNotheld(sqlite3_mutex *p){
  返回 wrapped.mutex.xMutexNotheld(p);
}



static 整数 wrPCacheInit(void *pArg){
  整数 rc;
  如果( wrapped.pcache_fail ){
    rc = SQLITE_ERROR;
  }否则{
    rc = wrapped.pcache.xInit(wrapped.pcache.pArg);
  }
  如果( rc==SQLITE_OK ){
    wrapped.pcache_init = 1;
  }
  返回 rc;
}
static void wrPCacheShutdown(void *pArg){
  wrapped.pcache.xShutdown(wrapped.pcache.pArg);
  wrapped.pcache_init = 0;
}

static sqlite3_pcache *wrPCacheCreate(整数 a, 整数 b, 整数 c){
  返回 wrapped.pcache.xCreate(a, b, c);
}  
static void wrPCacheCachesize(sqlite3_pcache *p, 整数 n){
  wrapped.pcache.xCachesize(p, n);
}  
static 整数 wrPCachePagecount(sqlite3_pcache *p){
  返回 wrapped.pcache.xPagecount(p);
}  
static sqlite3_pcache_page *wrPCacheFetch(sqlite3_pcache *p, unsigned a, 整数 b){
  返回 wrapped.pcache.xFetch(p, a, b);
}  
static void wrPCacheUnpin(sqlite3_pcache *p, sqlite3_pcache_page *a, 整数 b){
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

  memset(&wrapped, 0, sizeof(wrapped));

  sqlite3_shutdown();
  sqlite3_config(SQLITE_CONFIG_GETMUTEX, &wrapped.mutex);
  sqlite3_config(SQLITE_CONFIG_GETMALLOC, &wrapped.mem);
  sqlite3_config(SQLITE_CONFIG_GETPCACHE2, &wrapped.pcache);
  sqlite3_config(SQLITE_CONFIG_MUTEX, &mutexmethods);
  sqlite3_config(SQLITE_CONFIG_MALLOC, &memmethods);
  sqlite3_config(SQLITE_CONFIG_PCACHE2, &pcachemethods);
}

static 整数 SQLITE_TCLAPI init_wrapper_install(
  ClientData clientData, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  整数 objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  整数 i;
  installInitWrappers();
  为(i=1; i<objc; i++){
    char *z = Tcl_GetString(objv[i]);
    如果( strcmp(z, "mem")==0 ){
      wrapped.mem_fail = 1;
    }否则 如果( strcmp(z, "mutex")==0 ){
      wrapped.mutex_fail = 1;
    }否则 如果( strcmp(z, "pcache")==0 ){
      wrapped.pcache_fail = 1;
    }否则{
      Tcl_AppendResult(interp, "Unknown argument: \"", z, "\"", NULL);
      返回 TCL_ERROR;
    }
  }
  返回 TCL_OK;
}

static 整数 SQLITE_TCLAPI init_wrapper_uninstall(
  ClientData clientData, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  整数 objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  如果( objc!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    返回 TCL_ERROR;
  }

  sqlite3_shutdown();
  sqlite3_config(SQLITE_CONFIG_MUTEX, &wrapped.mutex);
  sqlite3_config(SQLITE_CONFIG_MALLOC, &wrapped.mem);
  sqlite3_config(SQLITE_CONFIG_PCACHE2, &wrapped.pcache);
  返回 TCL_OK;
}

static 整数 SQLITE_TCLAPI init_wrapper_clear(
  ClientData clientData, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  整数 objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  如果( objc!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    返回 TCL_ERROR;
  }

  wrapped.mem_fail = 0;
  wrapped.mutex_fail = 0;
  wrapped.pcache_fail = 0;
  返回 TCL_OK;
}

static 整数 SQLITE_TCLAPI init_wrapper_query(
  ClientData clientData, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  整数 objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  Tcl_Obj *pRet;

  如果( objc!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    返回 TCL_ERROR;
  }

  pRet = Tcl_NewObj();
  如果( wrapped.mutex_init ){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj("mutex", -1));
  }
  如果( wrapped.mem_init ){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj("mem", -1));
  }
  如果( wrapped.pcache_init ){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj("pcache", -1));
  }

  Tcl_SetObjResult(interp, pRet);
  返回 TCL_OK;
}

整数 Sqlitetest_init_Init(Tcl_Interp *interp){
  static struct {
     char *zName;
     Tcl_ObjCmdProc *xProc;
  } aObjCmd[] = {
    {"init_wrapper_install",   init_wrapper_install},
    {"init_wrapper_query",     init_wrapper_query  },
    {"init_wrapper_uninstall", init_wrapper_uninstall},
    {"init_wrapper_clear",     init_wrapper_clear}
  };
  整数 i;

  为(i=0; i<sizeof(aObjCmd)/sizeof(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }

  返回 TCL_OK;
}
