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

#অন্তর্ভাব "sqliteInt.h"
#অন্তর্ভাব <string.h>
#অন্তর্ভাব "tclsqlite.h"

স্থির কাঠামো Wrapped {
  sqlite3_pcache_methods2 pcache;
  sqlite3_mem_methods     mem;
  sqlite3_mutex_methods   mutex;

  পূর্ণসংখ্যা mem_init;              /* True if mem subsystem is initialized */
  পূর্ণসংখ্যা mem_fail;              /* True to fail mem subsystem initialization */
  পূর্ণসংখ্যা mutex_init;            /* True if mutex subsystem is initialized */
  পূর্ণসংখ্যা mutex_fail;            /* True to fail mutex subsystem initialization */
  পূর্ণসংখ্যা pcache_init;           /* True if pcache subsystem is initialized */
  পূর্ণসংখ্যা pcache_fail;           /* True to fail pcache subsystem initialization */
} wrapped;

স্থির পূর্ণসংখ্যা wrMemInit(শূন্য *p_App_তথ্য){
  পূর্ণসংখ্যা rc;
  যদি( wrapped.mem_fail ){
    rc = SQLITE_ERROR;
  }অন্যথা{
    rc = wrapped.mem.xInit(wrapped.mem.p_App_তথ্য);
  }
  যদি( rc==SQLITE_OK ){
    wrapped.mem_init = 1;
  }
  ফেরত rc;
}
স্থির শূন্য wrMemShutdown(শূন্য *p_App_তথ্য){
  wrapped.mem.xShutdown(wrapped.mem.p_App_তথ্য);
  wrapped.mem_init = 0;
}
স্থির শূন্য *wrMemMalloc(পূর্ণসংখ্যা n)           {ফেরত wrapped.mem.xMalloc(n);}
স্থির শূন্য wrMemFree(শূন্য *p)            {wrapped.mem.xFree(p);}
স্থির শূন্য *wrMemRealloc(শূন্য *p, পূর্ণসংখ্যা n) {ফেরত wrapped.mem.xRealloc(p, n);}
স্থির পূর্ণসংখ্যা wr_Mem_আকার(শূন্য *p)             {ফেরত wrapped.mem.x_আকার(p);}
স্থির পূর্ণসংখ্যা wrMemRoundup(পূর্ণসংখ্যা n)            {ফেরত wrapped.mem.xRoundup(n);}


স্থির পূর্ণসংখ্যা wrMutexInit(শূন্য){
  পূর্ণসংখ্যা rc;
  যদি( wrapped.mutex_fail ){
    rc = SQLITE_ERROR;
  }অন্যথা{
    rc = wrapped.mutex.xMutexInit();
  }
  যদি( rc==SQLITE_OK ){
    wrapped.mutex_init = 1;
  }
  ফেরত rc;
}
স্থির পূর্ণসংখ্যা wrMutexEnd(শূন্য){
  wrapped.mutex.xMutexEnd();
  wrapped.mutex_init = 0;
  ফেরত SQLITE_OK;
}
স্থির sqlite3_mutex *wrMutexAlloc(পূর্ণসংখ্যা e){
  ফেরত wrapped.mutex.xMutexAlloc(e);
}
স্থির শূন্য wrMutexFree(sqlite3_mutex *p){
  wrapped.mutex.xMutexFree(p);
}
স্থির শূন্য wrMutexEnter(sqlite3_mutex *p){
  wrapped.mutex.xMutexEnter(p);
}
স্থির পূর্ণসংখ্যা wrMutexTry(sqlite3_mutex *p){
  ফেরত wrapped.mutex.xMutexTry(p);
}
স্থির শূন্য wrMutexLeave(sqlite3_mutex *p){
  wrapped.mutex.xMutexLeave(p);
}
স্থির পূর্ণসংখ্যা wrMutexHeld(sqlite3_mutex *p){
  ফেরত wrapped.mutex.xMutexHeld(p);
}
স্থির পূর্ণসংখ্যা wrMutexNotheld(sqlite3_mutex *p){
  ফেরত wrapped.mutex.xMutexNotheld(p);
}



স্থির পূর্ণসংখ্যা wrPCacheInit(শূন্য *pArg){
  পূর্ণসংখ্যা rc;
  যদি( wrapped.pcache_fail ){
    rc = SQLITE_ERROR;
  }অন্যথা{
    rc = wrapped.pcache.xInit(wrapped.pcache.pArg);
  }
  যদি( rc==SQLITE_OK ){
    wrapped.pcache_init = 1;
  }
  ফেরত rc;
}
স্থির শূন্য wrPCacheShutdown(শূন্য *pArg){
  wrapped.pcache.xShutdown(wrapped.pcache.pArg);
  wrapped.pcache_init = 0;
}

স্থির sqlite3_pcache *wrPCacheCreate(পূর্ণসংখ্যা a, পূর্ণসংখ্যা b, পূর্ণসংখ্যা c){
  ফেরত wrapped.pcache.xCreate(a, b, c);
}  
স্থির শূন্য wrPCacheCachesize(sqlite3_pcache *p, পূর্ণসংখ্যা n){
  wrapped.pcache.xCachesize(p, n);
}  
স্থির পূর্ণসংখ্যা wrPCachePagecount(sqlite3_pcache *p){
  ফেরত wrapped.pcache.xPagecount(p);
}  
স্থির sqlite3_pcache_page *wrPCacheFetch(sqlite3_pcache *p, unsigned a, পূর্ণসংখ্যা b){
  ফেরত wrapped.pcache.xFetch(p, a, b);
}  
স্থির শূন্য wrPCacheUnpin(sqlite3_pcache *p, sqlite3_pcache_page *a, পূর্ণসংখ্যা b){
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
    wr_Mem_আকার,     wrMemRoundup, wrMemInit,
    wrMemShutdown,
    0
  };

  memset(&wrapped, 0, আকার(wrapped));

  sqlite3_shutdown();
  sqlite3_config(SQLITE_CONFIG_GETMUTEX, &wrapped.mutex);
  sqlite3_config(SQLITE_CONFIG_GETMALLOC, &wrapped.mem);
  sqlite3_config(SQLITE_CONFIG_GETPCACHE2, &wrapped.pcache);
  sqlite3_config(SQLITE_CONFIG_MUTEX, &mutexmethods);
  sqlite3_config(SQLITE_CONFIG_MALLOC, &memmethods);
  sqlite3_config(SQLITE_CONFIG_PCACHE2, &pcachemethods);
}

স্থির পূর্ণসংখ্যা SQLITE_TCLAPI init_wrapper_install(
  Client_তথ্য client_তথ্য, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  পূর্ণসংখ্যা i;
  installInitWrappers();
  জন্য(i=1; i<objc; i++){
    অক্ষর *z = Tcl_GetString(objv[i]);
    যদি( strcmp(z, "mem")==0 ){
      wrapped.mem_fail = 1;
    }অন্যথা যদি( strcmp(z, "mutex")==0 ){
      wrapped.mutex_fail = 1;
    }অন্যথা যদি( strcmp(z, "pcache")==0 ){
      wrapped.pcache_fail = 1;
    }অন্যথা{
      Tcl_Append_ফলাফল(interp, "Unknown argument: \"", z, "\"", ফাঁকা);
      ফেরত TCL_ERROR;
    }
  }
  ফেরত TCL_OK;
}

স্থির পূর্ণসংখ্যা SQLITE_TCLAPI init_wrapper_uninstall(
  Client_তথ্য client_তথ্য, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  যদি( objc!=1 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "");
    ফেরত TCL_ERROR;
  }

  sqlite3_shutdown();
  sqlite3_config(SQLITE_CONFIG_MUTEX, &wrapped.mutex);
  sqlite3_config(SQLITE_CONFIG_MALLOC, &wrapped.mem);
  sqlite3_config(SQLITE_CONFIG_PCACHE2, &wrapped.pcache);
  ফেরত TCL_OK;
}

স্থির পূর্ণসংখ্যা SQLITE_TCLAPI init_wrapper_clear(
  Client_তথ্য client_তথ্য, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  যদি( objc!=1 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "");
    ফেরত TCL_ERROR;
  }

  wrapped.mem_fail = 0;
  wrapped.mutex_fail = 0;
  wrapped.pcache_fail = 0;
  ফেরত TCL_OK;
}

স্থির পূর্ণসংখ্যা SQLITE_TCLAPI init_wrapper_query(
  Client_তথ্য client_তথ্য, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  Tcl_Obj *pRet;

  যদি( objc!=1 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "");
    ফেরত TCL_ERROR;
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

  Tcl_Set_Obj_ফলাফল(interp, pRet);
  ফেরত TCL_OK;
}

পূর্ণসংখ্যা Sqlitetest_init_Init(Tcl_Interp *interp){
  স্থির কাঠামো {
     অক্ষর *zName;
     Tcl_ObjCmdProc *xProc;
  } aObjCmd[] = {
    {"init_wrapper_install",   init_wrapper_install},
    {"init_wrapper_query",     init_wrapper_query  },
    {"init_wrapper_uninstall", init_wrapper_uninstall},
    {"init_wrapper_clear",     init_wrapper_clear}
  };
  পূর্ণসংখ্যা i;

  জন্য(i=0; i<আকার(aObjCmd)/আকার(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }

  ফেরত TCL_OK;
}
