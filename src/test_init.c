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

#অন্তর্ভাৱ "sqliteInt.h"
#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ "tclsqlite.h"

স্থির সংরচনা Wrapped {
  sqlite3_pcache_methods2 pcache;
  sqlite3_mem_methods     mem;
  sqlite3_mutex_methods   mutex;

  পূর্ণাঙ্ক mem_init;              /* True if mem subsystem is initialized */
  পূর্ণাঙ্ক mem_fail;              /* True to fail mem subsystem initialization */
  পূর্ণাঙ্ক mutex_init;            /* True if mutex subsystem is initialized */
  পূর্ণাঙ্ক mutex_fail;            /* True to fail mutex subsystem initialization */
  পূর্ণাঙ্ক pcache_init;           /* True if pcache subsystem is initialized */
  পূর্ণাঙ্ক pcache_fail;           /* True to fail pcache subsystem initialization */
} wrapped;

স্থির পূর্ণাঙ্ক wrMemInit(শূন্য *pAppData){
  পূর্ণাঙ্ক rc;
  যদি( wrapped.mem_fail ){
    rc = SQLITE_ERROR;
  }অন্যথা{
    rc = wrapped.mem.xInit(wrapped.mem.pAppData);
  }
  যদি( rc==SQLITE_OK ){
    wrapped.mem_init = 1;
  }
  ঘূৰাই rc;
}
স্থির শূন্য wrMemShutdown(শূন্য *pAppData){
  wrapped.mem.xShutdown(wrapped.mem.pAppData);
  wrapped.mem_init = 0;
}
স্থির শূন্য *wrMemMalloc(পূর্ণাঙ্ক n)           {ঘূৰাই wrapped.mem.xMalloc(n);}
স্থির শূন্য wrMemFree(শূন্য *p)            {wrapped.mem.xFree(p);}
স্থির শূন্য *wrMemRealloc(শূন্য *p, পূর্ণাঙ্ক n) {ঘূৰাই wrapped.mem.xRealloc(p, n);}
স্থির পূর্ণাঙ্ক wrMemSize(শূন্য *p)             {ঘূৰাই wrapped.mem.xSize(p);}
স্থির পূর্ণাঙ্ক wrMemRoundup(পূর্ণাঙ্ক n)            {ঘূৰাই wrapped.mem.xRoundup(n);}


স্থির পূর্ণাঙ্ক wrMutexInit(শূন্য){
  পূর্ণাঙ্ক rc;
  যদি( wrapped.mutex_fail ){
    rc = SQLITE_ERROR;
  }অন্যথা{
    rc = wrapped.mutex.xMutexInit();
  }
  যদি( rc==SQLITE_OK ){
    wrapped.mutex_init = 1;
  }
  ঘূৰাই rc;
}
স্থির পূর্ণাঙ্ক wrMutexEnd(শূন্য){
  wrapped.mutex.xMutexEnd();
  wrapped.mutex_init = 0;
  ঘূৰাই SQLITE_OK;
}
স্থির sqlite3_mutex *wrMutexAlloc(পূর্ণাঙ্ক e){
  ঘূৰাই wrapped.mutex.xMutexAlloc(e);
}
স্থির শূন্য wrMutexFree(sqlite3_mutex *p){
  wrapped.mutex.xMutexFree(p);
}
স্থির শূন্য wrMutexEnter(sqlite3_mutex *p){
  wrapped.mutex.xMutexEnter(p);
}
স্থির পূর্ণাঙ্ক wrMutexTry(sqlite3_mutex *p){
  ঘূৰাই wrapped.mutex.xMutexTry(p);
}
স্থির শূন্য wrMutexLeave(sqlite3_mutex *p){
  wrapped.mutex.xMutexLeave(p);
}
স্থির পূর্ণাঙ্ক wrMutexHeld(sqlite3_mutex *p){
  ঘূৰাই wrapped.mutex.xMutexHeld(p);
}
স্থির পূর্ণাঙ্ক wrMutexNotheld(sqlite3_mutex *p){
  ঘূৰাই wrapped.mutex.xMutexNotheld(p);
}



স্থির পূর্ণাঙ্ক wrPCacheInit(শূন্য *pArg){
  পূর্ণাঙ্ক rc;
  যদি( wrapped.pcache_fail ){
    rc = SQLITE_ERROR;
  }অন্যথা{
    rc = wrapped.pcache.xInit(wrapped.pcache.pArg);
  }
  যদি( rc==SQLITE_OK ){
    wrapped.pcache_init = 1;
  }
  ঘূৰাই rc;
}
স্থির শূন্য wrPCacheShutdown(শূন্য *pArg){
  wrapped.pcache.xShutdown(wrapped.pcache.pArg);
  wrapped.pcache_init = 0;
}

স্থির sqlite3_pcache *wrPCacheCreate(পূর্ণাঙ্ক a, পূর্ণাঙ্ক b, পূর্ণাঙ্ক c){
  ঘূৰাই wrapped.pcache.xCreate(a, b, c);
}  
স্থির শূন্য wrPCacheCachesize(sqlite3_pcache *p, পূর্ণাঙ্ক n){
  wrapped.pcache.xCachesize(p, n);
}  
স্থির পূর্ণাঙ্ক wrPCachePagecount(sqlite3_pcache *p){
  ঘূৰাই wrapped.pcache.xPagecount(p);
}  
স্থির sqlite3_pcache_page *wrPCacheFetch(sqlite3_pcache *p, unsigned a, পূর্ণাঙ্ক b){
  ঘূৰাই wrapped.pcache.xFetch(p, a, b);
}  
স্থির শূন্য wrPCacheUnpin(sqlite3_pcache *p, sqlite3_pcache_page *a, পূর্ণাঙ্ক b){
  wrapped.pcache.xUnpin(p, a, b);
}  
স্থির শূন্য wrPCacheRekey(
  sqlite3_pcache *p, 
  sqlite3_pcache_page *a, 
  unsigned b, 
  unsigned c
){
  wrapped.pcache.xRekey(p, a, b, c);
}  
স্থির শূন্য wrPCacheTruncate(sqlite3_pcache *p, unsigned a){
  wrapped.pcache.xTruncate(p, a);
}  
স্থির শূন্য wrPCacheDestroy(sqlite3_pcache *p){
  wrapped.pcache.xDestroy(p);
}  

স্থির শূন্য installInitWrappers(শূন্য){
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

  memset(&wrapped, 0, পরিমাণ(wrapped));

  sqlite3_shutdown();
  sqlite3_config(SQLITE_CONFIG_GETMUTEX, &wrapped.mutex);
  sqlite3_config(SQLITE_CONFIG_GETMALLOC, &wrapped.mem);
  sqlite3_config(SQLITE_CONFIG_GETPCACHE2, &wrapped.pcache);
  sqlite3_config(SQLITE_CONFIG_MUTEX, &mutexmethods);
  sqlite3_config(SQLITE_CONFIG_MALLOC, &memmethods);
  sqlite3_config(SQLITE_CONFIG_PCACHE2, &pcachemethods);
}

স্থির পূর্ণাঙ্ক SQLITE_TCLAPI init_wrapper_install(
  ClientData clientData, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণাঙ্ক objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  পূর্ণাঙ্ক i;
  installInitWrappers();
  বাবে(i=1; i<objc; i++){
    ৱর্ণ *z = Tcl_GetString(objv[i]);
    যদি( strcmp(z, "mem")==0 ){
      wrapped.mem_fail = 1;
    }অন্যথা যদি( strcmp(z, "mutex")==0 ){
      wrapped.mutex_fail = 1;
    }অন্যথা যদি( strcmp(z, "pcache")==0 ){
      wrapped.pcache_fail = 1;
    }অন্যথা{
      Tcl_AppendResult(interp, "Unknown argument: \"", z, "\"", রিক্ত);
      ঘূৰাই TCL_ERROR;
    }
  }
  ঘূৰাই TCL_OK;
}

স্থির পূর্ণাঙ্ক SQLITE_TCLAPI init_wrapper_uninstall(
  ClientData clientData, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণাঙ্ক objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  যদি( objc!=1 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "");
    ঘূৰাই TCL_ERROR;
  }

  sqlite3_shutdown();
  sqlite3_config(SQLITE_CONFIG_MUTEX, &wrapped.mutex);
  sqlite3_config(SQLITE_CONFIG_MALLOC, &wrapped.mem);
  sqlite3_config(SQLITE_CONFIG_PCACHE2, &wrapped.pcache);
  ঘূৰাই TCL_OK;
}

স্থির পূর্ণাঙ্ক SQLITE_TCLAPI init_wrapper_clear(
  ClientData clientData, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণাঙ্ক objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  যদি( objc!=1 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "");
    ঘূৰাই TCL_ERROR;
  }

  wrapped.mem_fail = 0;
  wrapped.mutex_fail = 0;
  wrapped.pcache_fail = 0;
  ঘূৰাই TCL_OK;
}

স্থির পূর্ণাঙ্ক SQLITE_TCLAPI init_wrapper_query(
  ClientData clientData, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণাঙ্ক objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  Tcl_Obj *pRet;

  যদি( objc!=1 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "");
    ঘূৰাই TCL_ERROR;
  }

  pRet = Tcl_NewObj();
  যদি( wrapped.mutex_init ){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj("mutex", -1));
  }
  যদি( wrapped.mem_init ){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj("mem", -1));
  }
  যদি( wrapped.pcache_init ){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj("pcache", -1));
  }

  Tcl_SetObjResult(interp, pRet);
  ঘূৰাই TCL_OK;
}

পূর্ণাঙ্ক Sqlitetest_init_Init(Tcl_Interp *interp){
  স্থির সংরচনা {
     ৱর্ণ *zName;
     Tcl_ObjCmdProc *xProc;
  } aObjCmd[] = {
    {"init_wrapper_install",   init_wrapper_install},
    {"init_wrapper_query",     init_wrapper_query  },
    {"init_wrapper_uninstall", init_wrapper_uninstall},
    {"init_wrapper_clear",     init_wrapper_clear}
  };
  পূর্ণাঙ্ক i;

  বাবে(i=0; i<পরিমাণ(aObjCmd)/পরিমাণ(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }

  ঘূৰাই TCL_OK;
}
