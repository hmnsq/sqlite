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

  ကိန်းပြည့် mem_init;              /* True if mem subsystem is initialized */
  ကိန်းပြည့် mem_fail;              /* True to fail mem subsystem initialization */
  ကိန်းပြည့် mutex_init;            /* True if mutex subsystem is initialized */
  ကိန်းပြည့် mutex_fail;            /* True to fail mutex subsystem initialization */
  ကိန်းပြည့် pcache_init;           /* True if pcache subsystem is initialized */
  ကိန်းပြည့် pcache_fail;           /* True to fail pcache subsystem initialization */
} wrapped;

static ကိန်းပြည့် wrMemInit(void *pAppData){
  ကိန်းပြည့် rc;
  အကယ်၍( wrapped.mem_fail ){
    rc = SQLITE_ERROR;
  }မဟုတ်ပါက{
    rc = wrapped.mem.xInit(wrapped.mem.pAppData);
  }
  အကယ်၍( rc==SQLITE_OK ){
    wrapped.mem_init = 1;
  }
  ပြန်ပေး rc;
}
static void wrMemShutdown(void *pAppData){
  wrapped.mem.xShutdown(wrapped.mem.pAppData);
  wrapped.mem_init = 0;
}
static void *wrMemMalloc(ကိန်းပြည့် n)           {ပြန်ပေး wrapped.mem.xMalloc(n);}
static void wrMemFree(void *p)            {wrapped.mem.xFree(p);}
static void *wrMemRealloc(void *p, ကိန်းပြည့် n) {ပြန်ပေး wrapped.mem.xRealloc(p, n);}
static ကိန်းပြည့် wrMemSize(void *p)             {ပြန်ပေး wrapped.mem.xSize(p);}
static ကိန်းပြည့် wrMemRoundup(ကိန်းပြည့် n)            {ပြန်ပေး wrapped.mem.xRoundup(n);}


static ကိန်းပြည့် wrMutexInit(void){
  ကိန်းပြည့် rc;
  အကယ်၍( wrapped.mutex_fail ){
    rc = SQLITE_ERROR;
  }မဟုတ်ပါက{
    rc = wrapped.mutex.xMutexInit();
  }
  အကယ်၍( rc==SQLITE_OK ){
    wrapped.mutex_init = 1;
  }
  ပြန်ပေး rc;
}
static ကိန်းပြည့် wrMutexEnd(void){
  wrapped.mutex.xMutexEnd();
  wrapped.mutex_init = 0;
  ပြန်ပေး SQLITE_OK;
}
static sqlite3_mutex *wrMutexAlloc(ကိန်းပြည့် e){
  ပြန်ပေး wrapped.mutex.xMutexAlloc(e);
}
static void wrMutexFree(sqlite3_mutex *p){
  wrapped.mutex.xMutexFree(p);
}
static void wrMutexEnter(sqlite3_mutex *p){
  wrapped.mutex.xMutexEnter(p);
}
static ကိန်းပြည့် wrMutexTry(sqlite3_mutex *p){
  ပြန်ပေး wrapped.mutex.xMutexTry(p);
}
static void wrMutexLeave(sqlite3_mutex *p){
  wrapped.mutex.xMutexLeave(p);
}
static ကိန်းပြည့် wrMutexHeld(sqlite3_mutex *p){
  ပြန်ပေး wrapped.mutex.xMutexHeld(p);
}
static ကိန်းပြည့် wrMutexNotheld(sqlite3_mutex *p){
  ပြန်ပေး wrapped.mutex.xMutexNotheld(p);
}



static ကိန်းပြည့် wrPCacheInit(void *pArg){
  ကိန်းပြည့် rc;
  အကယ်၍( wrapped.pcache_fail ){
    rc = SQLITE_ERROR;
  }မဟုတ်ပါက{
    rc = wrapped.pcache.xInit(wrapped.pcache.pArg);
  }
  အကယ်၍( rc==SQLITE_OK ){
    wrapped.pcache_init = 1;
  }
  ပြန်ပေး rc;
}
static void wrPCacheShutdown(void *pArg){
  wrapped.pcache.xShutdown(wrapped.pcache.pArg);
  wrapped.pcache_init = 0;
}

static sqlite3_pcache *wrPCacheCreate(ကိန်းပြည့် a, ကိန်းပြည့် b, ကိန်းပြည့် c){
  ပြန်ပေး wrapped.pcache.xCreate(a, b, c);
}  
static void wrPCacheCachesize(sqlite3_pcache *p, ကိန်းပြည့် n){
  wrapped.pcache.xCachesize(p, n);
}  
static ကိန်းပြည့် wrPCachePagecount(sqlite3_pcache *p){
  ပြန်ပေး wrapped.pcache.xPagecount(p);
}  
static sqlite3_pcache_page *wrPCacheFetch(sqlite3_pcache *p, unsigned a, ကိန်းပြည့် b){
  ပြန်ပေး wrapped.pcache.xFetch(p, a, b);
}  
static void wrPCacheUnpin(sqlite3_pcache *p, sqlite3_pcache_page *a, ကိန်းပြည့် b){
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

static ကိန်းပြည့် SQLITE_TCLAPI init_wrapper_install(
  ClientData clientData, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ကိန်းပြည့် objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  ကိန်းပြည့် i;
  installInitWrappers();
  အတွက်(i=1; i<objc; i++){
    var_na *z = Tcl_GetString(objv[i]);
    အကယ်၍( strcmp(z, "mem")==0 ){
      wrapped.mem_fail = 1;
    }မဟုတ်ပါက အကယ်၍( strcmp(z, "mutex")==0 ){
      wrapped.mutex_fail = 1;
    }မဟုတ်ပါက အကယ်၍( strcmp(z, "pcache")==0 ){
      wrapped.pcache_fail = 1;
    }မဟုတ်ပါက{
      Tcl_AppendResult(interp, "Unknown argument: \"", z, "\"", NULL);
      ပြန်ပေး TCL_ERROR;
    }
  }
  ပြန်ပေး TCL_OK;
}

static ကိန်းပြည့် SQLITE_TCLAPI init_wrapper_uninstall(
  ClientData clientData, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ကိန်းပြည့် objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  အကယ်၍( objc!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    ပြန်ပေး TCL_ERROR;
  }

  sqlite3_shutdown();
  sqlite3_config(SQLITE_CONFIG_MUTEX, &wrapped.mutex);
  sqlite3_config(SQLITE_CONFIG_MALLOC, &wrapped.mem);
  sqlite3_config(SQLITE_CONFIG_PCACHE2, &wrapped.pcache);
  ပြန်ပေး TCL_OK;
}

static ကိန်းပြည့် SQLITE_TCLAPI init_wrapper_clear(
  ClientData clientData, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ကိန်းပြည့် objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  အကယ်၍( objc!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    ပြန်ပေး TCL_ERROR;
  }

  wrapped.mem_fail = 0;
  wrapped.mutex_fail = 0;
  wrapped.pcache_fail = 0;
  ပြန်ပေး TCL_OK;
}

static ကိန်းပြည့် SQLITE_TCLAPI init_wrapper_query(
  ClientData clientData, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ကိန်းပြည့် objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  Tcl_Obj *pRet;

  အကယ်၍( objc!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    ပြန်ပေး TCL_ERROR;
  }

  pRet = Tcl_NewObj();
  အကယ်၍( wrapped.mutex_init ){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj("mutex", -1));
  }
  အကယ်၍( wrapped.mem_init ){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj("mem", -1));
  }
  အကယ်၍( wrapped.pcache_init ){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj("pcache", -1));
  }

  Tcl_SetObjResult(interp, pRet);
  ပြန်ပေး TCL_OK;
}

ကိန်းပြည့် Sqlitetest_init_Init(Tcl_Interp *interp){
  static sa_nracanaa {
     var_na *zName;
     Tcl_ObjCmdProc *xProc;
  } aObjCmd[] = {
    {"init_wrapper_install",   init_wrapper_install},
    {"init_wrapper_query",     init_wrapper_query  },
    {"init_wrapper_uninstall", init_wrapper_uninstall},
    {"init_wrapper_clear",     init_wrapper_clear}
  };
  ကိန်းပြည့် i;

  အတွက်(i=0; i<parimaa_na(aObjCmd)/parimaa_na(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }

  ပြန်ပေး TCL_OK;
}
