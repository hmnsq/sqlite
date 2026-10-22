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

  số_nguyên mem_init;              /* True if mem subsystem is initialized */
  số_nguyên mem_fail;              /* True to fail mem subsystem initialization */
  số_nguyên mutex_init;            /* True if mutex subsystem is initialized */
  số_nguyên mutex_fail;            /* True to fail mutex subsystem initialization */
  số_nguyên pcache_init;           /* True if pcache subsystem is initialized */
  số_nguyên pcache_fail;           /* True to fail pcache subsystem initialization */
} wrapped;

static số_nguyên wrMemInit(void *pAppData){
  số_nguyên rc;
  nếu( wrapped.mem_fail ){
    rc = SQLITE_ERROR;
  }không_thì{
    rc = wrapped.mem.xInit(wrapped.mem.pAppData);
  }
  nếu( rc==SQLITE_OK ){
    wrapped.mem_init = 1;
  }
  trả_về rc;
}
static void wrMemShutdown(void *pAppData){
  wrapped.mem.xShutdown(wrapped.mem.pAppData);
  wrapped.mem_init = 0;
}
static void *wrMemMalloc(số_nguyên n)           {trả_về wrapped.mem.xMalloc(n);}
static void wrMemFree(void *p)            {wrapped.mem.xFree(p);}
static void *wrMemRealloc(void *p, số_nguyên n) {trả_về wrapped.mem.xRealloc(p, n);}
static số_nguyên wrMemSize(void *p)             {trả_về wrapped.mem.xSize(p);}
static số_nguyên wrMemRoundup(số_nguyên n)            {trả_về wrapped.mem.xRoundup(n);}


static số_nguyên wrMutexInit(void){
  số_nguyên rc;
  nếu( wrapped.mutex_fail ){
    rc = SQLITE_ERROR;
  }không_thì{
    rc = wrapped.mutex.xMutexInit();
  }
  nếu( rc==SQLITE_OK ){
    wrapped.mutex_init = 1;
  }
  trả_về rc;
}
static số_nguyên wrMutexEnd(void){
  wrapped.mutex.xMutexEnd();
  wrapped.mutex_init = 0;
  trả_về SQLITE_OK;
}
static sqlite3_mutex *wrMutexAlloc(số_nguyên e){
  trả_về wrapped.mutex.xMutexAlloc(e);
}
static void wrMutexFree(sqlite3_mutex *p){
  wrapped.mutex.xMutexFree(p);
}
static void wrMutexEnter(sqlite3_mutex *p){
  wrapped.mutex.xMutexEnter(p);
}
static số_nguyên wrMutexTry(sqlite3_mutex *p){
  trả_về wrapped.mutex.xMutexTry(p);
}
static void wrMutexLeave(sqlite3_mutex *p){
  wrapped.mutex.xMutexLeave(p);
}
static số_nguyên wrMutexHeld(sqlite3_mutex *p){
  trả_về wrapped.mutex.xMutexHeld(p);
}
static số_nguyên wrMutexNotheld(sqlite3_mutex *p){
  trả_về wrapped.mutex.xMutexNotheld(p);
}



static số_nguyên wrPCacheInit(void *pArg){
  số_nguyên rc;
  nếu( wrapped.pcache_fail ){
    rc = SQLITE_ERROR;
  }không_thì{
    rc = wrapped.pcache.xInit(wrapped.pcache.pArg);
  }
  nếu( rc==SQLITE_OK ){
    wrapped.pcache_init = 1;
  }
  trả_về rc;
}
static void wrPCacheShutdown(void *pArg){
  wrapped.pcache.xShutdown(wrapped.pcache.pArg);
  wrapped.pcache_init = 0;
}

static sqlite3_pcache *wrPCacheCreate(số_nguyên a, số_nguyên b, số_nguyên c){
  trả_về wrapped.pcache.xCreate(a, b, c);
}  
static void wrPCacheCachesize(sqlite3_pcache *p, số_nguyên n){
  wrapped.pcache.xCachesize(p, n);
}  
static số_nguyên wrPCachePagecount(sqlite3_pcache *p){
  trả_về wrapped.pcache.xPagecount(p);
}  
static sqlite3_pcache_page *wrPCacheFetch(sqlite3_pcache *p, unsigned a, số_nguyên b){
  trả_về wrapped.pcache.xFetch(p, a, b);
}  
static void wrPCacheUnpin(sqlite3_pcache *p, sqlite3_pcache_page *a, số_nguyên b){
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

static số_nguyên SQLITE_TCLAPI init_wrapper_install(
  ClientData clientData, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  số_nguyên objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  số_nguyên i;
  installInitWrappers();
  cho(i=1; i<objc; i++){
    char *z = Tcl_GetString(objv[i]);
    nếu( strcmp(z, "mem")==0 ){
      wrapped.mem_fail = 1;
    }không_thì nếu( strcmp(z, "mutex")==0 ){
      wrapped.mutex_fail = 1;
    }không_thì nếu( strcmp(z, "pcache")==0 ){
      wrapped.pcache_fail = 1;
    }không_thì{
      Tcl_AppendResult(interp, "Unknown argument: \"", z, "\"", NULL);
      trả_về TCL_ERROR;
    }
  }
  trả_về TCL_OK;
}

static số_nguyên SQLITE_TCLAPI init_wrapper_uninstall(
  ClientData clientData, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  số_nguyên objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  nếu( objc!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    trả_về TCL_ERROR;
  }

  sqlite3_shutdown();
  sqlite3_config(SQLITE_CONFIG_MUTEX, &wrapped.mutex);
  sqlite3_config(SQLITE_CONFIG_MALLOC, &wrapped.mem);
  sqlite3_config(SQLITE_CONFIG_PCACHE2, &wrapped.pcache);
  trả_về TCL_OK;
}

static số_nguyên SQLITE_TCLAPI init_wrapper_clear(
  ClientData clientData, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  số_nguyên objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  nếu( objc!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    trả_về TCL_ERROR;
  }

  wrapped.mem_fail = 0;
  wrapped.mutex_fail = 0;
  wrapped.pcache_fail = 0;
  trả_về TCL_OK;
}

static số_nguyên SQLITE_TCLAPI init_wrapper_query(
  ClientData clientData, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  số_nguyên objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  Tcl_Obj *pRet;

  nếu( objc!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    trả_về TCL_ERROR;
  }

  pRet = Tcl_NewObj();
  nếu( wrapped.mutex_init ){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj("mutex", -1));
  }
  nếu( wrapped.mem_init ){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj("mem", -1));
  }
  nếu( wrapped.pcache_init ){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj("pcache", -1));
  }

  Tcl_SetObjResult(interp, pRet);
  trả_về TCL_OK;
}

số_nguyên Sqlitetest_init_Init(Tcl_Interp *interp){
  static struct {
     char *zName;
     Tcl_ObjCmdProc *xProc;
  } aObjCmd[] = {
    {"init_wrapper_install",   init_wrapper_install},
    {"init_wrapper_query",     init_wrapper_query  },
    {"init_wrapper_uninstall", init_wrapper_uninstall},
    {"init_wrapper_clear",     init_wrapper_clear}
  };
  số_nguyên i;

  cho(i=0; i<sizeof(aObjCmd)/sizeof(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }

  trả_về TCL_OK;
}
