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

#समावेश "sqliteInt.h"
#समावेश <string.h>
#समावेश "tclsqlite.h"

अटल काष्ठा Wrapped {
  sqlite3_pcache_methods2 pcache;
  sqlite3_mem_methods     mem;
  sqlite3_mutex_methods   mutex;

  पूर्णांक mem_init;              /* True if mem subsystem is initialized */
  पूर्णांक mem_fail;              /* True to fail mem subsystem initialization */
  पूर्णांक mutex_init;            /* True if mutex subsystem is initialized */
  पूर्णांक mutex_fail;            /* True to fail mutex subsystem initialization */
  पूर्णांक pcache_init;           /* True if pcache subsystem is initialized */
  पूर्णांक pcache_fail;           /* True to fail pcache subsystem initialization */
} wrapped;

अटल पूर्णांक wrMemInit(व्योम *p_App_तथ्य){
  पूर्णांक rc;
  अगर( wrapped.mem_fail ){
    rc = SQLITE_ERROR;
  }अन्यथा{
    rc = wrapped.mem.xInit(wrapped.mem.p_App_तथ्य);
  }
  अगर( rc==SQLITE_OK ){
    wrapped.mem_init = 1;
  }
  वापस rc;
}
अटल व्योम wrMemShutdown(व्योम *p_App_तथ्य){
  wrapped.mem.xShutdown(wrapped.mem.p_App_तथ्य);
  wrapped.mem_init = 0;
}
अटल व्योम *wrMemMalloc(पूर्णांक n)           {वापस wrapped.mem.xMalloc(n);}
अटल व्योम wrMemFree(व्योम *p)            {wrapped.mem.xFree(p);}
अटल व्योम *wrMemRealloc(व्योम *p, पूर्णांक n) {वापस wrapped.mem.xRealloc(p, n);}
अटल पूर्णांक wr_Mem_आकार(व्योम *p)             {वापस wrapped.mem.x_आकार(p);}
अटल पूर्णांक wrMemRoundup(पूर्णांक n)            {वापस wrapped.mem.xRoundup(n);}


अटल पूर्णांक wrMutexInit(व्योम){
  पूर्णांक rc;
  अगर( wrapped.mutex_fail ){
    rc = SQLITE_ERROR;
  }अन्यथा{
    rc = wrapped.mutex.xMutexInit();
  }
  अगर( rc==SQLITE_OK ){
    wrapped.mutex_init = 1;
  }
  वापस rc;
}
अटल पूर्णांक wrMutexEnd(व्योम){
  wrapped.mutex.xMutexEnd();
  wrapped.mutex_init = 0;
  वापस SQLITE_OK;
}
अटल sqlite3_mutex *wrMutexAlloc(पूर्णांक e){
  वापस wrapped.mutex.xMutexAlloc(e);
}
अटल व्योम wrMutexFree(sqlite3_mutex *p){
  wrapped.mutex.xMutexFree(p);
}
अटल व्योम wrMutexEnter(sqlite3_mutex *p){
  wrapped.mutex.xMutexEnter(p);
}
अटल पूर्णांक wrMutexTry(sqlite3_mutex *p){
  वापस wrapped.mutex.xMutexTry(p);
}
अटल व्योम wrMutexLeave(sqlite3_mutex *p){
  wrapped.mutex.xMutexLeave(p);
}
अटल पूर्णांक wrMutexHeld(sqlite3_mutex *p){
  वापस wrapped.mutex.xMutexHeld(p);
}
अटल पूर्णांक wrMutexNotheld(sqlite3_mutex *p){
  वापस wrapped.mutex.xMutexNotheld(p);
}



अटल पूर्णांक wrPCacheInit(व्योम *pArg){
  पूर्णांक rc;
  अगर( wrapped.pcache_fail ){
    rc = SQLITE_ERROR;
  }अन्यथा{
    rc = wrapped.pcache.xInit(wrapped.pcache.pArg);
  }
  अगर( rc==SQLITE_OK ){
    wrapped.pcache_init = 1;
  }
  वापस rc;
}
अटल व्योम wrPCacheShutdown(व्योम *pArg){
  wrapped.pcache.xShutdown(wrapped.pcache.pArg);
  wrapped.pcache_init = 0;
}

अटल sqlite3_pcache *wrPCacheCreate(पूर्णांक a, पूर्णांक b, पूर्णांक c){
  वापस wrapped.pcache.xCreate(a, b, c);
}  
अटल व्योम wrPCacheCachesize(sqlite3_pcache *p, पूर्णांक n){
  wrapped.pcache.xCachesize(p, n);
}  
अटल पूर्णांक wrPCachePagecount(sqlite3_pcache *p){
  वापस wrapped.pcache.xPagecount(p);
}  
अटल sqlite3_pcache_page *wrPCacheFetch(sqlite3_pcache *p, अचिन्हित a, पूर्णांक b){
  वापस wrapped.pcache.xFetch(p, a, b);
}  
अटल व्योम wrPCacheUnpin(sqlite3_pcache *p, sqlite3_pcache_page *a, पूर्णांक b){
  wrapped.pcache.xUnpin(p, a, b);
}  
अटल व्योम wrPCacheRekey(
  sqlite3_pcache *p, 
  sqlite3_pcache_page *a, 
  अचिन्हित b, 
  अचिन्हित c
){
  wrapped.pcache.xRekey(p, a, b, c);
}  
अटल व्योम wrPCacheTruncate(sqlite3_pcache *p, अचिन्हित a){
  wrapped.pcache.xTruncate(p, a);
}  
अटल व्योम wrPCacheDestroy(sqlite3_pcache *p){
  wrapped.pcache.xDestroy(p);
}  

अटल व्योम installInitWrappers(व्योम){
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
    wr_Mem_आकार,     wrMemRoundup, wrMemInit,
    wrMemShutdown,
    0
  };

  स_रखो(&wrapped, 0, माप(wrapped));

  sqlite3_shutdown();
  sqlite3_config(SQLITE_CONFIG_GETMUTEX, &wrapped.mutex);
  sqlite3_config(SQLITE_CONFIG_GETMALLOC, &wrapped.mem);
  sqlite3_config(SQLITE_CONFIG_GETPCACHE2, &wrapped.pcache);
  sqlite3_config(SQLITE_CONFIG_MUTEX, &mutexmethods);
  sqlite3_config(SQLITE_CONFIG_MALLOC, &memmethods);
  sqlite3_config(SQLITE_CONFIG_PCACHE2, &pcachemethods);
}

अटल पूर्णांक SQLITE_TCLAPI init_wrapper_install(
  Client_तथ्य client_तथ्य, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  पूर्णांक i;
  installInitWrappers();
  क्रम(i=1; i<objc; i++){
    अक्षर *z = Tcl_GetString(objv[i]);
    अगर( म_भेद(z, "mem")==0 ){
      wrapped.mem_fail = 1;
    }अन्यथा अगर( म_भेद(z, "mutex")==0 ){
      wrapped.mutex_fail = 1;
    }अन्यथा अगर( म_भेद(z, "pcache")==0 ){
      wrapped.pcache_fail = 1;
    }अन्यथा{
      Tcl_Append_परिणाम(interp, "Unknown argument: \"", z, "\"", शून्य);
      वापस TCL_ERROR;
    }
  }
  वापस TCL_OK;
}

अटल पूर्णांक SQLITE_TCLAPI init_wrapper_uninstall(
  Client_तथ्य client_तथ्य, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  अगर( objc!=1 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "");
    वापस TCL_ERROR;
  }

  sqlite3_shutdown();
  sqlite3_config(SQLITE_CONFIG_MUTEX, &wrapped.mutex);
  sqlite3_config(SQLITE_CONFIG_MALLOC, &wrapped.mem);
  sqlite3_config(SQLITE_CONFIG_PCACHE2, &wrapped.pcache);
  वापस TCL_OK;
}

अटल पूर्णांक SQLITE_TCLAPI init_wrapper_clear(
  Client_तथ्य client_तथ्य, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  अगर( objc!=1 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "");
    वापस TCL_ERROR;
  }

  wrapped.mem_fail = 0;
  wrapped.mutex_fail = 0;
  wrapped.pcache_fail = 0;
  वापस TCL_OK;
}

अटल पूर्णांक SQLITE_TCLAPI init_wrapper_query(
  Client_तथ्य client_तथ्य, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  Tcl_Obj *pRet;

  अगर( objc!=1 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "");
    वापस TCL_ERROR;
  }

  pRet = Tcl_NewObj();
  अगर( wrapped.mutex_init ){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj("mutex", -1));
  }
  अगर( wrapped.mem_init ){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj("mem", -1));
  }
  अगर( wrapped.pcache_init ){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj("pcache", -1));
  }

  Tcl_Set_Obj_परिणाम(interp, pRet);
  वापस TCL_OK;
}

पूर्णांक Sqlitetest_init_Init(Tcl_Interp *interp){
  अटल काष्ठा {
     अक्षर *zName;
     Tcl_ObjCmdProc *xProc;
  } aObjCmd[] = {
    {"init_wrapper_install",   init_wrapper_install},
    {"init_wrapper_query",     init_wrapper_query  },
    {"init_wrapper_uninstall", init_wrapper_uninstall},
    {"init_wrapper_clear",     init_wrapper_clear}
  };
  पूर्णांक i;

  क्रम(i=0; i<माप(aObjCmd)/माप(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }

  वापस TCL_OK;
}
