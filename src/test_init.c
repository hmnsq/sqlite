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

  целое mem_init;              /* True if mem subsystem is initialized */
  целое mem_fail;              /* True to fail mem subsystem initialization */
  целое mutex_init;            /* True if mutex subsystem is initialized */
  целое mutex_fail;            /* True to fail mutex subsystem initialization */
  целое pcache_init;           /* True if pcache subsystem is initialized */
  целое pcache_fail;           /* True to fail pcache subsystem initialization */
} wrapped;

static целое wrMemInit(void *pAppData){
  целое rc;
  если( wrapped.mem_fail ){
    rc = SQLITE_ERROR;
  }иначе{
    rc = wrapped.mem.xInit(wrapped.mem.pAppData);
  }
  если( rc==SQLITE_OK ){
    wrapped.mem_init = 1;
  }
  вернуть rc;
}
static void wrMemShutdown(void *pAppData){
  wrapped.mem.xShutdown(wrapped.mem.pAppData);
  wrapped.mem_init = 0;
}
static void *wrMemMalloc(целое n)           {вернуть wrapped.mem.xMalloc(n);}
static void wrMemFree(void *p)            {wrapped.mem.xFree(p);}
static void *wrMemRealloc(void *p, целое n) {вернуть wrapped.mem.xRealloc(p, n);}
static целое wrMemSize(void *p)             {вернуть wrapped.mem.xSize(p);}
static целое wrMemRoundup(целое n)            {вернуть wrapped.mem.xRoundup(n);}


static целое wrMutexInit(void){
  целое rc;
  если( wrapped.mutex_fail ){
    rc = SQLITE_ERROR;
  }иначе{
    rc = wrapped.mutex.xMutexInit();
  }
  если( rc==SQLITE_OK ){
    wrapped.mutex_init = 1;
  }
  вернуть rc;
}
static целое wrMutexEnd(void){
  wrapped.mutex.xMutexEnd();
  wrapped.mutex_init = 0;
  вернуть SQLITE_OK;
}
static sqlite3_mutex *wrMutexAlloc(целое e){
  вернуть wrapped.mutex.xMutexAlloc(e);
}
static void wrMutexFree(sqlite3_mutex *p){
  wrapped.mutex.xMutexFree(p);
}
static void wrMutexEnter(sqlite3_mutex *p){
  wrapped.mutex.xMutexEnter(p);
}
static целое wrMutexTry(sqlite3_mutex *p){
  вернуть wrapped.mutex.xMutexTry(p);
}
static void wrMutexLeave(sqlite3_mutex *p){
  wrapped.mutex.xMutexLeave(p);
}
static целое wrMutexHeld(sqlite3_mutex *p){
  вернуть wrapped.mutex.xMutexHeld(p);
}
static целое wrMutexNotheld(sqlite3_mutex *p){
  вернуть wrapped.mutex.xMutexNotheld(p);
}



static целое wrPCacheInit(void *pArg){
  целое rc;
  если( wrapped.pcache_fail ){
    rc = SQLITE_ERROR;
  }иначе{
    rc = wrapped.pcache.xInit(wrapped.pcache.pArg);
  }
  если( rc==SQLITE_OK ){
    wrapped.pcache_init = 1;
  }
  вернуть rc;
}
static void wrPCacheShutdown(void *pArg){
  wrapped.pcache.xShutdown(wrapped.pcache.pArg);
  wrapped.pcache_init = 0;
}

static sqlite3_pcache *wrPCacheCreate(целое a, целое b, целое c){
  вернуть wrapped.pcache.xCreate(a, b, c);
}  
static void wrPCacheCachesize(sqlite3_pcache *p, целое n){
  wrapped.pcache.xCachesize(p, n);
}  
static целое wrPCachePagecount(sqlite3_pcache *p){
  вернуть wrapped.pcache.xPagecount(p);
}  
static sqlite3_pcache_page *wrPCacheFetch(sqlite3_pcache *p, unsigned a, целое b){
  вернуть wrapped.pcache.xFetch(p, a, b);
}  
static void wrPCacheUnpin(sqlite3_pcache *p, sqlite3_pcache_page *a, целое b){
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

static целое SQLITE_TCLAPI init_wrapper_install(
  ClientData clientData, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  целое objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  целое i;
  installInitWrappers();
  для(i=1; i<objc; i++){
    char *z = Tcl_GetString(objv[i]);
    если( strcmp(z, "mem")==0 ){
      wrapped.mem_fail = 1;
    }иначе если( strcmp(z, "mutex")==0 ){
      wrapped.mutex_fail = 1;
    }иначе если( strcmp(z, "pcache")==0 ){
      wrapped.pcache_fail = 1;
    }иначе{
      Tcl_AppendResult(interp, "Unknown argument: \"", z, "\"", NULL);
      вернуть TCL_ERROR;
    }
  }
  вернуть TCL_OK;
}

static целое SQLITE_TCLAPI init_wrapper_uninstall(
  ClientData clientData, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  целое objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  если( objc!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    вернуть TCL_ERROR;
  }

  sqlite3_shutdown();
  sqlite3_config(SQLITE_CONFIG_MUTEX, &wrapped.mutex);
  sqlite3_config(SQLITE_CONFIG_MALLOC, &wrapped.mem);
  sqlite3_config(SQLITE_CONFIG_PCACHE2, &wrapped.pcache);
  вернуть TCL_OK;
}

static целое SQLITE_TCLAPI init_wrapper_clear(
  ClientData clientData, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  целое objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  если( objc!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    вернуть TCL_ERROR;
  }

  wrapped.mem_fail = 0;
  wrapped.mutex_fail = 0;
  wrapped.pcache_fail = 0;
  вернуть TCL_OK;
}

static целое SQLITE_TCLAPI init_wrapper_query(
  ClientData clientData, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  целое objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  Tcl_Obj *pRet;

  если( objc!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    вернуть TCL_ERROR;
  }

  pRet = Tcl_NewObj();
  если( wrapped.mutex_init ){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj("mutex", -1));
  }
  если( wrapped.mem_init ){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj("mem", -1));
  }
  если( wrapped.pcache_init ){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj("pcache", -1));
  }

  Tcl_SetObjResult(interp, pRet);
  вернуть TCL_OK;
}

целое Sqlitetest_init_Init(Tcl_Interp *interp){
  static struct {
     char *zName;
     Tcl_ObjCmdProc *xProc;
  } aObjCmd[] = {
    {"init_wrapper_install",   init_wrapper_install},
    {"init_wrapper_query",     init_wrapper_query  },
    {"init_wrapper_uninstall", init_wrapper_uninstall},
    {"init_wrapper_clear",     init_wrapper_clear}
  };
  целое i;

  для(i=0; i<sizeof(aObjCmd)/sizeof(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }

  вернуть TCL_OK;
}
