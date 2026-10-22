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

  පූර්ණ_සංඛ්‍යා mem_init;              /* True if mem subsystem is initialized */
  පූර්ණ_සංඛ්‍යා mem_fail;              /* True to fail mem subsystem initialization */
  පූර්ණ_සංඛ්‍යා mutex_init;            /* True if mutex subsystem is initialized */
  පූර්ණ_සංඛ්‍යා mutex_fail;            /* True to fail mutex subsystem initialization */
  පූර්ණ_සංඛ්‍යා pcache_init;           /* True if pcache subsystem is initialized */
  පූර්ණ_සංඛ්‍යා pcache_fail;           /* True to fail pcache subsystem initialization */
} wrapped;

static පූර්ණ_සංඛ්‍යා wrMemInit(void *pAppData){
  පූර්ණ_සංඛ්‍යා rc;
  නම්( wrapped.mem_fail ){
    rc = SQLITE_ERROR;
  }නැතහොත්{
    rc = wrapped.mem.xInit(wrapped.mem.pAppData);
  }
  නම්( rc==SQLITE_OK ){
    wrapped.mem_init = 1;
  }
  ආපසු rc;
}
static void wrMemShutdown(void *pAppData){
  wrapped.mem.xShutdown(wrapped.mem.pAppData);
  wrapped.mem_init = 0;
}
static void *wrMemMalloc(පූර්ණ_සංඛ්‍යා n)           {ආපසු wrapped.mem.xMalloc(n);}
static void wrMemFree(void *p)            {wrapped.mem.xFree(p);}
static void *wrMemRealloc(void *p, පූර්ණ_සංඛ්‍යා n) {ආපසු wrapped.mem.xRealloc(p, n);}
static පූර්ණ_සංඛ්‍යා wrMemSize(void *p)             {ආපසු wrapped.mem.xSize(p);}
static පූර්ණ_සංඛ්‍යා wrMemRoundup(පූර්ණ_සංඛ්‍යා n)            {ආපසු wrapped.mem.xRoundup(n);}


static පූර්ණ_සංඛ්‍යා wrMutexInit(void){
  පූර්ණ_සංඛ්‍යා rc;
  නම්( wrapped.mutex_fail ){
    rc = SQLITE_ERROR;
  }නැතහොත්{
    rc = wrapped.mutex.xMutexInit();
  }
  නම්( rc==SQLITE_OK ){
    wrapped.mutex_init = 1;
  }
  ආපසු rc;
}
static පූර්ණ_සංඛ්‍යා wrMutexEnd(void){
  wrapped.mutex.xMutexEnd();
  wrapped.mutex_init = 0;
  ආපසු SQLITE_OK;
}
static sqlite3_mutex *wrMutexAlloc(පූර්ණ_සංඛ්‍යා e){
  ආපසු wrapped.mutex.xMutexAlloc(e);
}
static void wrMutexFree(sqlite3_mutex *p){
  wrapped.mutex.xMutexFree(p);
}
static void wrMutexEnter(sqlite3_mutex *p){
  wrapped.mutex.xMutexEnter(p);
}
static පූර්ණ_සංඛ්‍යා wrMutexTry(sqlite3_mutex *p){
  ආපසු wrapped.mutex.xMutexTry(p);
}
static void wrMutexLeave(sqlite3_mutex *p){
  wrapped.mutex.xMutexLeave(p);
}
static පූර්ණ_සංඛ්‍යා wrMutexHeld(sqlite3_mutex *p){
  ආපසු wrapped.mutex.xMutexHeld(p);
}
static පූර්ණ_සංඛ්‍යා wrMutexNotheld(sqlite3_mutex *p){
  ආපසු wrapped.mutex.xMutexNotheld(p);
}



static පූර්ණ_සංඛ්‍යා wrPCacheInit(void *pArg){
  පූර්ණ_සංඛ්‍යා rc;
  නම්( wrapped.pcache_fail ){
    rc = SQLITE_ERROR;
  }නැතහොත්{
    rc = wrapped.pcache.xInit(wrapped.pcache.pArg);
  }
  නම්( rc==SQLITE_OK ){
    wrapped.pcache_init = 1;
  }
  ආපසු rc;
}
static void wrPCacheShutdown(void *pArg){
  wrapped.pcache.xShutdown(wrapped.pcache.pArg);
  wrapped.pcache_init = 0;
}

static sqlite3_pcache *wrPCacheCreate(පූර්ණ_සංඛ්‍යා a, පූර්ණ_සංඛ්‍යා b, පූර්ණ_සංඛ්‍යා c){
  ආපසු wrapped.pcache.xCreate(a, b, c);
}  
static void wrPCacheCachesize(sqlite3_pcache *p, පූර්ණ_සංඛ්‍යා n){
  wrapped.pcache.xCachesize(p, n);
}  
static පූර්ණ_සංඛ්‍යා wrPCachePagecount(sqlite3_pcache *p){
  ආපසු wrapped.pcache.xPagecount(p);
}  
static sqlite3_pcache_page *wrPCacheFetch(sqlite3_pcache *p, unsigned a, පූර්ණ_සංඛ්‍යා b){
  ආපසු wrapped.pcache.xFetch(p, a, b);
}  
static void wrPCacheUnpin(sqlite3_pcache *p, sqlite3_pcache_page *a, පූර්ණ_සංඛ්‍යා b){
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

static පූර්ණ_සංඛ්‍යා SQLITE_TCLAPI init_wrapper_install(
  ClientData clientData, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  පූර්ණ_සංඛ්‍යා objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  පූර්ණ_සංඛ්‍යා i;
  installInitWrappers();
  සඳහා(i=1; i<objc; i++){
    var_na *z = Tcl_GetString(objv[i]);
    නම්( strcmp(z, "mem")==0 ){
      wrapped.mem_fail = 1;
    }නැතහොත් නම්( strcmp(z, "mutex")==0 ){
      wrapped.mutex_fail = 1;
    }නැතහොත් නම්( strcmp(z, "pcache")==0 ){
      wrapped.pcache_fail = 1;
    }නැතහොත්{
      Tcl_AppendResult(interp, "Unknown argument: \"", z, "\"", NULL);
      ආපසු TCL_ERROR;
    }
  }
  ආපසු TCL_OK;
}

static පූර්ණ_සංඛ්‍යා SQLITE_TCLAPI init_wrapper_uninstall(
  ClientData clientData, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  පූර්ණ_සංඛ්‍යා objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  නම්( objc!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    ආපසු TCL_ERROR;
  }

  sqlite3_shutdown();
  sqlite3_config(SQLITE_CONFIG_MUTEX, &wrapped.mutex);
  sqlite3_config(SQLITE_CONFIG_MALLOC, &wrapped.mem);
  sqlite3_config(SQLITE_CONFIG_PCACHE2, &wrapped.pcache);
  ආපසු TCL_OK;
}

static පූර්ණ_සංඛ්‍යා SQLITE_TCLAPI init_wrapper_clear(
  ClientData clientData, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  පූර්ණ_සංඛ්‍යා objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  නම්( objc!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    ආපසු TCL_ERROR;
  }

  wrapped.mem_fail = 0;
  wrapped.mutex_fail = 0;
  wrapped.pcache_fail = 0;
  ආපසු TCL_OK;
}

static පූර්ණ_සංඛ්‍යා SQLITE_TCLAPI init_wrapper_query(
  ClientData clientData, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  පූර්ණ_සංඛ්‍යා objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  Tcl_Obj *pRet;

  නම්( objc!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    ආපසු TCL_ERROR;
  }

  pRet = Tcl_NewObj();
  නම්( wrapped.mutex_init ){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj("mutex", -1));
  }
  නම්( wrapped.mem_init ){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj("mem", -1));
  }
  නම්( wrapped.pcache_init ){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj("pcache", -1));
  }

  Tcl_SetObjResult(interp, pRet);
  ආපසු TCL_OK;
}

පූර්ණ_සංඛ්‍යා Sqlitetest_init_Init(Tcl_Interp *interp){
  static sa_nracanaa {
     var_na *zName;
     Tcl_ObjCmdProc *xProc;
  } aObjCmd[] = {
    {"init_wrapper_install",   init_wrapper_install},
    {"init_wrapper_query",     init_wrapper_query  },
    {"init_wrapper_uninstall", init_wrapper_uninstall},
    {"init_wrapper_clear",     init_wrapper_clear}
  };
  පූර්ණ_සංඛ්‍යා i;

  සඳහා(i=0; i<parimaa_na(aObjCmd)/parimaa_na(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }

  ආපසු TCL_OK;
}
