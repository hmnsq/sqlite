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

#अन्तर्भाव "sqliteInt.h"
#अन्तर्भाव <string.h>
#अन्तर्भाव "tclsqlite.h"

स्थिर संरचना Wrapped {
  sqlite3_pcache_methods2 pcache;
  sqlite3_mem_methods     mem;
  sqlite3_mutex_methods   mutex;

  पूर्णाङ्क mem_init;              /* True if mem subsystem is initialized */
  पूर्णाङ्क mem_fail;              /* True to fail mem subsystem initialization */
  पूर्णाङ्क mutex_init;            /* True if mutex subsystem is initialized */
  पूर्णाङ्क mutex_fail;            /* True to fail mutex subsystem initialization */
  पूर्णाङ्क pcache_init;           /* True if pcache subsystem is initialized */
  पूर्णाङ्क pcache_fail;           /* True to fail pcache subsystem initialization */
} wrapped;

स्थिर पूर्णाङ्क wrMemInit(शून्य *pAppData){
  पूर्णाङ्क rc;
  यदि( wrapped.mem_fail ){
    rc = SQLITE_ERROR;
  }अन्यथा{
    rc = wrapped.mem.xInit(wrapped.mem.pAppData);
  }
  यदि( rc==SQLITE_OK ){
    wrapped.mem_init = 1;
  }
  प्रत्यागम rc;
}
स्थिर शून्य wrMemShutdown(शून्य *pAppData){
  wrapped.mem.xShutdown(wrapped.mem.pAppData);
  wrapped.mem_init = 0;
}
स्थिर शून्य *wrMemMalloc(पूर्णाङ्क n)           {प्रत्यागम wrapped.mem.xMalloc(n);}
स्थिर शून्य wrMemFree(शून्य *p)            {wrapped.mem.xFree(p);}
स्थिर शून्य *wrMemRealloc(शून्य *p, पूर्णाङ्क n) {प्रत्यागम wrapped.mem.xRealloc(p, n);}
स्थिर पूर्णाङ्क wrMemSize(शून्य *p)             {प्रत्यागम wrapped.mem.xSize(p);}
स्थिर पूर्णाङ्क wrMemRoundup(पूर्णाङ्क n)            {प्रत्यागम wrapped.mem.xRoundup(n);}


स्थिर पूर्णाङ्क wrMutexInit(शून्य){
  पूर्णाङ्क rc;
  यदि( wrapped.mutex_fail ){
    rc = SQLITE_ERROR;
  }अन्यथा{
    rc = wrapped.mutex.xMutexInit();
  }
  यदि( rc==SQLITE_OK ){
    wrapped.mutex_init = 1;
  }
  प्रत्यागम rc;
}
स्थिर पूर्णाङ्क wrMutexEnd(शून्य){
  wrapped.mutex.xMutexEnd();
  wrapped.mutex_init = 0;
  प्रत्यागम SQLITE_OK;
}
स्थिर sqlite3_mutex *wrMutexAlloc(पूर्णाङ्क e){
  प्रत्यागम wrapped.mutex.xMutexAlloc(e);
}
स्थिर शून्य wrMutexFree(sqlite3_mutex *p){
  wrapped.mutex.xMutexFree(p);
}
स्थिर शून्य wrMutexEnter(sqlite3_mutex *p){
  wrapped.mutex.xMutexEnter(p);
}
स्थिर पूर्णाङ्क wrMutexTry(sqlite3_mutex *p){
  प्रत्यागम wrapped.mutex.xMutexTry(p);
}
स्थिर शून्य wrMutexLeave(sqlite3_mutex *p){
  wrapped.mutex.xMutexLeave(p);
}
स्थिर पूर्णाङ्क wrMutexHeld(sqlite3_mutex *p){
  प्रत्यागम wrapped.mutex.xMutexHeld(p);
}
स्थिर पूर्णाङ्क wrMutexNotheld(sqlite3_mutex *p){
  प्रत्यागम wrapped.mutex.xMutexNotheld(p);
}



स्थिर पूर्णाङ्क wrPCacheInit(शून्य *pArg){
  पूर्णाङ्क rc;
  यदि( wrapped.pcache_fail ){
    rc = SQLITE_ERROR;
  }अन्यथा{
    rc = wrapped.pcache.xInit(wrapped.pcache.pArg);
  }
  यदि( rc==SQLITE_OK ){
    wrapped.pcache_init = 1;
  }
  प्रत्यागम rc;
}
स्थिर शून्य wrPCacheShutdown(शून्य *pArg){
  wrapped.pcache.xShutdown(wrapped.pcache.pArg);
  wrapped.pcache_init = 0;
}

स्थिर sqlite3_pcache *wrPCacheCreate(पूर्णाङ्क a, पूर्णाङ्क b, पूर्णाङ्क c){
  प्रत्यागम wrapped.pcache.xCreate(a, b, c);
}  
स्थिर शून्य wrPCacheCachesize(sqlite3_pcache *p, पूर्णाङ्क n){
  wrapped.pcache.xCachesize(p, n);
}  
स्थिर पूर्णाङ्क wrPCachePagecount(sqlite3_pcache *p){
  प्रत्यागम wrapped.pcache.xPagecount(p);
}  
स्थिर sqlite3_pcache_page *wrPCacheFetch(sqlite3_pcache *p, अचिन्हित a, पूर्णाङ्क b){
  प्रत्यागम wrapped.pcache.xFetch(p, a, b);
}  
स्थिर शून्य wrPCacheUnpin(sqlite3_pcache *p, sqlite3_pcache_page *a, पूर्णाङ्क b){
  wrapped.pcache.xUnpin(p, a, b);
}  
स्थिर शून्य wrPCacheRekey(
  sqlite3_pcache *p, 
  sqlite3_pcache_page *a, 
  अचिन्हित b, 
  अचिन्हित c
){
  wrapped.pcache.xRekey(p, a, b, c);
}  
स्थिर शून्य wrPCacheTruncate(sqlite3_pcache *p, अचिन्हित a){
  wrapped.pcache.xTruncate(p, a);
}  
स्थिर शून्य wrPCacheDestroy(sqlite3_pcache *p){
  wrapped.pcache.xDestroy(p);
}  

स्थिर शून्य installInitWrappers(शून्य){
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

  स_रखो(&wrapped, 0, परिमाण(wrapped));

  sqlite3_shutdown();
  sqlite3_config(SQLITE_CONFIG_GETMUTEX, &wrapped.mutex);
  sqlite3_config(SQLITE_CONFIG_GETMALLOC, &wrapped.mem);
  sqlite3_config(SQLITE_CONFIG_GETPCACHE2, &wrapped.pcache);
  sqlite3_config(SQLITE_CONFIG_MUTEX, &mutexmethods);
  sqlite3_config(SQLITE_CONFIG_MALLOC, &memmethods);
  sqlite3_config(SQLITE_CONFIG_PCACHE2, &pcachemethods);
}

स्थिर पूर्णाङ्क SQLITE_TCLAPI init_wrapper_install(
  ClientData clientData, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  पूर्णाङ्क i;
  installInitWrappers();
  प्रति(i=1; i<objc; i++){
    वर्ण *z = Tcl_GetString(objv[i]);
    यदि( म_भेद(z, "mem")==0 ){
      wrapped.mem_fail = 1;
    }अन्यथा यदि( म_भेद(z, "mutex")==0 ){
      wrapped.mutex_fail = 1;
    }अन्यथा यदि( म_भेद(z, "pcache")==0 ){
      wrapped.pcache_fail = 1;
    }अन्यथा{
      Tcl_AppendResult(interp, "Unknown argument: \"", z, "\"", रिक्त);
      प्रत्यागम TCL_ERROR;
    }
  }
  प्रत्यागम TCL_OK;
}

स्थिर पूर्णाङ्क SQLITE_TCLAPI init_wrapper_uninstall(
  ClientData clientData, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  यदि( objc!=1 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "");
    प्रत्यागम TCL_ERROR;
  }

  sqlite3_shutdown();
  sqlite3_config(SQLITE_CONFIG_MUTEX, &wrapped.mutex);
  sqlite3_config(SQLITE_CONFIG_MALLOC, &wrapped.mem);
  sqlite3_config(SQLITE_CONFIG_PCACHE2, &wrapped.pcache);
  प्रत्यागम TCL_OK;
}

स्थिर पूर्णाङ्क SQLITE_TCLAPI init_wrapper_clear(
  ClientData clientData, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  यदि( objc!=1 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "");
    प्रत्यागम TCL_ERROR;
  }

  wrapped.mem_fail = 0;
  wrapped.mutex_fail = 0;
  wrapped.pcache_fail = 0;
  प्रत्यागम TCL_OK;
}

स्थिर पूर्णाङ्क SQLITE_TCLAPI init_wrapper_query(
  ClientData clientData, /* Unused */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  Tcl_Obj *pRet;

  यदि( objc!=1 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "");
    प्रत्यागम TCL_ERROR;
  }

  pRet = Tcl_NewObj();
  यदि( wrapped.mutex_init ){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj("mutex", -1));
  }
  यदि( wrapped.mem_init ){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj("mem", -1));
  }
  यदि( wrapped.pcache_init ){
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj("pcache", -1));
  }

  Tcl_SetObjResult(interp, pRet);
  प्रत्यागम TCL_OK;
}

पूर्णाङ्क Sqlitetest_init_Init(Tcl_Interp *interp){
  स्थिर संरचना {
     वर्ण *zName;
     Tcl_ObjCmdProc *xProc;
  } aObjCmd[] = {
    {"init_wrapper_install",   init_wrapper_install},
    {"init_wrapper_query",     init_wrapper_query  },
    {"init_wrapper_uninstall", init_wrapper_uninstall},
    {"init_wrapper_clear",     init_wrapper_clear}
  };
  पूर्णाङ्क i;

  प्रति(i=0; i<परिमाण(aObjCmd)/परिमाण(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }

  प्रत्यागम TCL_OK;
}
