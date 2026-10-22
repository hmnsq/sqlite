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

  puur_naa_nka mem_init;              /* True if mem subsystem is initialized */
  puur_naa_nka mem_fail;              /* True to fail mem subsystem initialization */
  puur_naa_nka mutex_init;            /* True if mutex subsystem is initialized */
  puur_naa_nka mutex_fail;            /* True to fail mutex subsystem initialization */
  puur_naa_nka pcache_init;           /* True if pcache subsystem is initialized */
  puur_naa_nka pcache_fail;           /* True to fail pcache subsystem initialization */
} wrapped;

static puur_naa_nka wrMemInit(void *pAppData){
  puur_naa_nka rc;
  ᱡᱩᱫᱤ( wrapped.mem_fail ){
    rc = SQLITE_ERROR;
  }ᱵᱟᱝ{
    rc = wrapped.mem.xInit(wrapped.mem.pAppData);
  }
  ᱡᱩᱫᱤ( rc==SQLITE_OK ){
    wrapped.mem_init = 1;
  }
  ᱨᱩᱣᱟᱹᱲ rc;
}
static void wrMemShutdown(void *pAppData){
  wrapped.mem.xShutdown(wrapped.mem.pAppData);
  wrapped.mem_init = 0;
}
static void *wrMemMalloc(puur_naa_nka n)           {ᱨᱩᱣᱟᱹᱲ wrapped.mem.xMalloc(n);}
static void wrMemFree(void *p)            {wrapped.mem.xFree(p);}
static void *wrMemRealloc(void *p, puur_naa_nka n) {ᱨᱩᱣᱟᱹᱲ wrapped.mem.xRealloc(p, n);}
static puur_naa_nka wrMemSize(void *p)             {ᱨᱩᱣᱟᱹᱲ wrapped.mem.xSize(p);}
static puur_naa_nka wrMemRoundup(puur_naa_nka n)            {ᱨᱩᱣᱟᱹᱲ wrapped.mem.xRoundup(n);}


static puur_naa_nka wrMutexInit(void){
  puur_naa_nka rc;
  ᱡᱩᱫᱤ( wrapped.mutex_fail ){
    rc = SQLITE_ERROR;
  }ᱵᱟᱝ{
    rc = wrapped.mutex.xMutexInit();
  }
  ᱡᱩᱫᱤ( rc==SQLITE_OK ){
    wrapped.mutex_init = 1;
  }
  ᱨᱩᱣᱟᱹᱲ rc;
}
static puur_naa_nka wrMutexEnd(void){
  wrapped.mutex.xMutexEnd();
  wrapped.mutex_init = 0;
  ᱨᱩᱣᱟᱹᱲ SQLITE_OK;
}
static sqlite3_mutex *wrMutexAlloc(puur_naa_nka e){
  ᱨᱩᱣᱟᱹᱲ wrapped.mutex.xMutexAlloc(e);
}
static void wrMutexFree(sqlite3_mutex *p){
  wrapped.mutex.xMutexFree(p);
}
static void wrMutexEnter(sqlite3_mutex *p){
  wrapped.mutex.xMutexEnter(p);
}
static puur_naa_nka wrMutexTry(sqlite3_mutex *p){
  ᱨᱩᱣᱟᱹᱲ wrapped.mutex.xMutexTry(p);
}
static void wrMutexLeave(sqlite3_mutex *p){
  wrapped.mutex.xMutexLeave(p);
}
static puur_naa_nka wrMutexHeld(sqlite3_mutex *p){
  ᱨᱩᱣᱟᱹᱲ wrapped.mutex.xMutexHeld(p);
}
static puur_naa_nka wrMutexNotheld(sqlite3_mutex *p){
  ᱨᱩᱣᱟᱹᱲ wrapped.mutex.xMutexNotheld(p);
}



static puur_naa_nka wrPCacheInit(void *pArg){
  puur_naa_nka rc;
  ᱡᱩᱫᱤ( wrapped.pcache_fail ){
    rc = SQLITE_ERROR;
  }ᱵᱟᱝ{
    rc = wrapped.pcache.xInit(wrapped.pcache.pArg);
  }
  ᱡᱩᱫᱤ( rc==SQLITE_OK ){
    wrapped.pcache_init = 1;
  }
  ᱨᱩᱣᱟᱹᱲ rc;
}
static void wrPCacheShutdown(void *pArg){
  wrapped.pcache.xShutdown(wrapped.pcache.pArg);
  wrapped.pcache_init = 0;
}

static sqlite3_pcache *wrPCacheCreate(puur_naa_nka a, puur_naa_nka b, puur_naa_nka c){
  ᱨᱩᱣᱟᱹᱲ wrapped.pcache.xCreate(a, b, c);
}  
static void wrPCacheCachesize(sqlite3_pcache *p, puur_naa_nka n){
  wrapped.pcache.xCachesize(p, n);
}  
static puur_naa_nka wrPCachePagecount(sqlite3_pcache *p){
  ᱨᱩᱣᱟᱹᱲ wrapped.pcache.xPagecount(p);
}  
static sqlite3_pcache_page *wrPCacheFetch(sqlite3_pcache *p, unsigned a, puur_naa_nka b){
  ᱨᱩᱣᱟᱹᱲ wrapped.pcache.xFetch(p, a, b);
}  
static void wrPCacheUnpin(sqlite3_pcache *p, sqlite3_pcache_page *a, puur_naa_nka b){
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

static puur_naa_nka SQLITE_TCLAPI init_wrapper_install(
  ClientData clientData, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  puur_naa_nka objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  puur_naa_nka i;
  installInitWrappers();
  for(i=1; i<objc; i++){
    var_na *z = Tcl_GetString(objv[i]);
    ᱡᱩᱫᱤ( strcmp(z, "mem")==0 ){
      wrapped.mem_fail = 1;
    }ᱵᱟᱝ ᱡᱩᱫᱤ( strcmp(z, "mutex")==0 ){
      wrapped.mutex_fail = 1;
    }ᱵᱟᱝ ᱡᱩᱫᱤ( strcmp(z, "pcache")==0 ){
      wrapped.pcache_fail = 1;
    }ᱵᱟᱝ{
      Tcl_AppendResult(interp, "Unknown argument: \"", z, "\"", NULL);
      ᱨᱩᱣᱟᱹᱲ TCL_ERROR;
    }
  }
  ᱨᱩᱣᱟᱹᱲ TCL_OK;
}

static puur_naa_nka SQLITE_TCLAPI init_wrapper_uninstall(
  ClientData clientData, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  puur_naa_nka objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  ᱡᱩᱫᱤ( objc!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    ᱨᱩᱣᱟᱹᱲ TCL_ERROR;
  }

  sqlite3_shutdown();
  sqlite3_config(SQLITE_CONFIG_MUTEX, &wrapped.mutex);
  sqlite3_config(SQLITE_CONFIG_MALLOC, &wrapped.mem);
  sqlite3_config(SQLITE_CONFIG_PCACHE2, &wrapped.pcache);
  ᱨᱩᱣᱟᱹᱲ TCL_OK;
}

static puur_naa_nka SQLITE_TCLAPI init_wrapper_clear(
  ClientData clientData, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  puur_naa_nka objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  ᱡᱩᱫᱤ( objc!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    ᱨᱩᱣᱟᱹᱲ TCL_ERROR;
  }

  wrapped.mem_fail = 0;
  wrapped.mutex_fail = 0;
  wrapped.pcache_fail = 0;
  ᱨᱩᱣᱟᱹᱲ TCL_OK;
}

static puur_naa_nka SQLITE_TCLAPI init_wrapper_query(
  ClientData clientData, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  puur_naa_nka objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  Tcl_Obj *pRet;

  ᱡᱩᱫᱤ( objc!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    ᱨᱩᱣᱟᱹᱲ TCL_ERROR;
  }

  pRet = Tcl_NewObj();
  ᱡᱩᱫᱤ( wrapped.mutex_init ){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj("mutex", -1));
  }
  ᱡᱩᱫᱤ( wrapped.mem_init ){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj("mem", -1));
  }
  ᱡᱩᱫᱤ( wrapped.pcache_init ){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj("pcache", -1));
  }

  Tcl_SetObjResult(interp, pRet);
  ᱨᱩᱣᱟᱹᱲ TCL_OK;
}

puur_naa_nka Sqlitetest_init_Init(Tcl_Interp *interp){
  static sa_nracanaa {
     var_na *zName;
     Tcl_ObjCmdProc *xProc;
  } aObjCmd[] = {
    {"init_wrapper_install",   init_wrapper_install},
    {"init_wrapper_query",     init_wrapper_query  },
    {"init_wrapper_uninstall", init_wrapper_uninstall},
    {"init_wrapper_clear",     init_wrapper_clear}
  };
  puur_naa_nka i;

  for(i=0; i<parimaa_na(aObjCmd)/parimaa_na(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }

  ᱨᱩᱣᱟᱹᱲ TCL_OK;
}
