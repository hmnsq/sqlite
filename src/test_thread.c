/*
** 2007 September 9
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
** This file contains the implementation of some Tcl commands used to
** test that sqlite3 database handles may be concurrently accessed by 
** multiple threads. Right now this only works on unix.
*/

#अन्तर्भाव "sqliteInt.h"
#अन्तर्भाव "tclsqlite.h"

#जर SQLITE_THREADSAFE

#अन्तर्भाव <errno.h>

#जर !defined(_MSC_VER)
#अन्तर्भाव <unistd.h>
#endif

/*
** One of these is allocated for each thread created by [sqlthread spawn].
*/
प्रकारव्याख्या संरचना SqlThread SqlThread;
संरचना SqlThread {
  Tcl_ThreadId parent;     /* Thread id of parent thread */
  Tcl_Interp *interp;      /* Parent interpreter */
  अक्षर *zScript;           /* The script to execute. */
  अक्षर *zVarname;          /* Varname in parent script */
};

/*
** A custom Tcl_Event type used by this module. When the event is
** handled, script zScript is evaluated in interpreter interp. If
** the evaluation throws an exception (returns TCL_ERROR), then the
** error is handled by Tcl_BackgroundError(). If no error occurs,
** the result is simply discarded.
*/
प्रकारव्याख्या संरचना EvalEvent EvalEvent;
संरचना EvalEvent {
  Tcl_Event base;          /* Base class of type Tcl_Event */
  अक्षर *zScript;           /* The script to execute. */
  Tcl_Interp *interp;      /* The interpreter to execute it in. */
};

स्थिर Tcl_ObjCmdProc sqlthread_proc;
स्थिर Tcl_ObjCmdProc clock_seconds_proc;
#जर SQLITE_OS_UNIX && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)
स्थिर Tcl_ObjCmdProc blocking_step_proc;
स्थिर Tcl_ObjCmdProc blocking_prepare_v2_proc;
#endif
पूर्णांक Sqlitetest1_Init(Tcl_Interp *);
पूर्णांक Sqlite3_Init(Tcl_Interp *);

/* Functions from main.c */
बाह्य स्थिरांक अक्षर *sqlite3ErrName(पूर्णांक);

/* Functions from test1.c */
बाह्य शून्य *sqlite3TestTextToPtr(स्थिरांक अक्षर *);
बाह्य पूर्णांक getDbPointer(Tcl_Interp *, स्थिरांक अक्षर *, sqlite3 **);
बाह्य पूर्णांक sqlite3TestMakePointerStr(Tcl_Interp *, अक्षर *, शून्य *);
बाह्य पूर्णांक sqlite3TestErrCode(Tcl_Interp *, sqlite3 *, पूर्णांक);

/*
** Handler for events of type EvalEvent.
*/
स्थिर पूर्णांक SQLITE_TCLAPI tclScriptEvent(Tcl_Event *evPtr, पूर्णांक flags){
  पूर्णांक rc;
  EvalEvent *p = (EvalEvent *)evPtr;
  rc = Tcl_Eval(p->interp, p->zScript);
  जर( rc!=TCL_OK ){
    Tcl_BackgroundError(p->interp);
  }
  UNUSED_PARAMETER(flags);
  परत 1;
}

/*
** Register an EvalEvent to evaluate the script pScript in the
** parent interpreter/thread of SqlThread p.
*/
स्थिर शून्य postToParent(SqlThread *p, Tcl_Obj *pScript){
  EvalEvent *pEvent;
  अक्षर *z_संदेश;
  Tcl_Size n_संदेश;

  z_संदेश = Tcl_GetStringFromObj(pScript, &n_संदेश); 
  pEvent = (EvalEvent *)ckalloc(आकार(EvalEvent)+n_संदेश+1);
  pEvent->base.nextPtr = 0;
  pEvent->base.proc = tclScriptEvent;
  pEvent->zScript = (अक्षर *)&pEvent[1];
  स_नकल(pEvent->zScript, z_संदेश, n_संदेश+1);
  pEvent->interp = p->interp;

  Tcl_ThreadQueueEvent(p->parent, (Tcl_Event *)pEvent, TCL_QUEUE_TAIL);
  Tcl_ThreadAlert(p->parent);
}

/*
** The main function for threads created with [sqlthread spawn].
*/
स्थिर Tcl_ThreadCreateType tclScriptThread(ClientData pSqlThread){
  Tcl_Interp *interp;
  Tcl_Obj *pRes;
  Tcl_Obj *pList;
  पूर्णांक rc;
  SqlThread *p = (SqlThread *)pSqlThread;
  बाह्य पूर्णांक Sqlitetest_mutex_Init(Tcl_Interp*);

  interp = Tcl_CreateInterp();
  Tcl_CreateObjCommand(interp, "clock_seconds", clock_seconds_proc, 0, 0);
  Tcl_CreateObjCommand(interp, "sqlthread", sqlthread_proc, pSqlThread, 0);
#जर SQLITE_OS_UNIX && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)
  Tcl_CreateObjCommand(interp, "sqlite3_blocking_step", blocking_step_proc,0,0);
  Tcl_CreateObjCommand(interp, 
      "sqlite3_blocking_prepare_v2", blocking_prepare_v2_proc, (शून्य *)1, 0);
  Tcl_CreateObjCommand(interp, 
      "sqlite3_nonblocking_prepare_v2", blocking_prepare_v2_proc, 0, 0);
#endif
  Sqlitetest1_Init(interp);
  Sqlitetest_mutex_Init(interp);
  Sqlite3_Init(interp);

  rc = Tcl_Eval(interp, p->zScript);
  pRes = Tcl_GetObjResult(interp);
  pList = Tcl_NewObj();
  Tcl_Incr_Ref_मोजणी(pList);
  Tcl_Incr_Ref_मोजणी(pRes);

  जर( rc!=TCL_OK ){
    Tcl_ListObjAppendElement(interp, pList, Tcl_NewStringObj("error", -1));
    Tcl_ListObjAppendElement(interp, pList, pRes);
    postToParent(p, pList);
    Tcl_Decr_Ref_मोजणी(pList);
    pList = Tcl_NewObj();
  }

  Tcl_ListObjAppendElement(interp, pList, Tcl_NewStringObj("set", -1));
  Tcl_ListObjAppendElement(interp, pList, Tcl_NewStringObj(p->zVarname, -1));
  Tcl_ListObjAppendElement(interp, pList, pRes);
  postToParent(p, pList);

  ckfree((शून्य *)p);
  Tcl_Decr_Ref_मोजणी(pList);
  Tcl_Decr_Ref_मोजणी(pRes);
  Tcl_DeleteInterp(interp);
  पर्यंत( Tcl_DoOneEvent(TCL_ALL_EVENTS|TCL_DONT_WAIT) );
  Tcl_ExitThread(0);
  TCL_THREAD_CREATE_RETURN;
}

/*
** sqlthread spawn VARNAME SCRIPT
**
**     Spawn a new thread with its own Tcl interpreter and run the
**     specified SCRIPT(s) in it. The thread terminates after running
**     the script. The result of the script is stored in the variable
**     VARNAME.
**
**     The caller can wait for the script to terminate using [vwait VARNAME].
*/
स्थिर पूर्णांक SQLITE_TCLAPI sqlthread_spawn(
  ClientData clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_ThreadId x;
  SqlThread *pNew;
  पूर्णांक rc;

  Tcl_Size nVarname; अक्षर *zVarname;
  Tcl_Size nScript; अक्षर *zScript;

  /* Parameters for thread creation */
  स्थिरांक पूर्णांक n_चळत = TCL_THREAD_चळत_DEFAULT;
  स्थिरांक पूर्णांक flags = TCL_THREAD_NOFLAGS;

  प्रतिज्ञा(objc==4);
  UNUSED_PARAMETER(clientData);
  UNUSED_PARAMETER(objc);

  zVarname = Tcl_GetStringFromObj(objv[2], &nVarname);
  zScript = Tcl_GetStringFromObj(objv[3], &nScript);

  pNew = (SqlThread *)ckalloc(आकार(SqlThread)+nVarname+nScript+2);
  pNew->zVarname = (अक्षर *)&pNew[1];
  pNew->zScript = (अक्षर *)&pNew->zVarname[nVarname+1];
  स_नकल(pNew->zVarname, zVarname, nVarname+1);
  स_नकल(pNew->zScript, zScript, nScript+1);
  pNew->parent = Tcl_GetCurrentThread();
  pNew->interp = interp;

  rc = Tcl_CreateThread(&x, tclScriptThread, (शून्य *)pNew, n_चळत, flags);
  जर( rc!=TCL_OK ){
    Tcl_AppendResult(interp, "Error in Tcl_CreateThread()", रिक्त);
    ckfree((अक्षर *)pNew);
    परत TCL_ERROR;
  }

  परत TCL_OK;
}

/*
** sqlthread parent SCRIPT
**
**     This can be called by spawned threads only. It sends the specified
**     script back to the parent thread for execution. The result of
**     evaluating the SCRIPT is returned. The parent thread must enter
**     the event loop for this to work - otherwise the caller will
**     block indefinitely.
**
**     NOTE: At the moment, this doesn't work. FIXME.
*/
स्थिर पूर्णांक SQLITE_TCLAPI sqlthread_parent(
  ClientData clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  EvalEvent *pEvent;
  अक्षर *z_संदेश;
  Tcl_Size n_संदेश;
  SqlThread *p = (SqlThread *)clientData;

  प्रतिज्ञा(objc==3);
  UNUSED_PARAMETER(objc);

  जर( p==0 ){
    Tcl_AppendResult(interp, "no parent thread", रिक्त);
    परत TCL_ERROR;
  }

  z_संदेश = Tcl_GetStringFromObj(objv[2], &n_संदेश);
  pEvent = (EvalEvent *)ckalloc(आकार(EvalEvent)+n_संदेश+1);
  pEvent->base.nextPtr = 0;
  pEvent->base.proc = tclScriptEvent;
  pEvent->zScript = (अक्षर *)&pEvent[1];
  स_नकल(pEvent->zScript, z_संदेश, n_संदेश+1);
  pEvent->interp = p->interp;
  Tcl_ThreadQueueEvent(p->parent, (Tcl_Event *)pEvent, TCL_QUEUE_TAIL);
  Tcl_ThreadAlert(p->parent);

  परत TCL_OK;
}

स्थिर पूर्णांक xBusy(शून्य *pArg, पूर्णांक nBusy){
  UNUSED_PARAMETER(pArg);
  UNUSED_PARAMETER(nBusy);
  sqlite3_sleep(50);
  परत 1;             /* Try again... */
}

/*
** sqlthread open
**
**     Open a database handle and return the string representation of
**     the pointer value.
*/
स्थिर पूर्णांक SQLITE_TCLAPI sqlthread_open(
  ClientData clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक sqlite3TestMakePointerStr(Tcl_Interp *interp, अक्षर *zPtr, शून्य *p);

  स्थिरांक अक्षर *zFilename;
  sqlite3 *db;
  अक्षर zBuf[100];
  बाह्य पूर्णांक Md5_Register(sqlite3*,अक्षर**,स्थिरांक sqlite3_api_routines*);

  UNUSED_PARAMETER(clientData);
  UNUSED_PARAMETER(objc);

  zFilename = Tcl_GetString(objv[2]);
  sqlite3_open(zFilename, &db);
  Md5_Register(db, 0, 0);
  sqlite3_busy_handler(db, xBusy, 0);
  
  जर( sqlite3TestMakePointerStr(interp, zBuf, db) ) परत TCL_ERROR;
  Tcl_AppendResult(interp, zBuf, रिक्त);

  परत TCL_OK;
}


/*
** sqlthread open
**
**     Return the current thread-id (Tcl_GetCurrentThread()) cast to
**     an integer.
*/
स्थिर पूर्णांक SQLITE_TCLAPI sqlthread_id(
  ClientData clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_ThreadId id = Tcl_GetCurrentThread();
  Tcl_SetObjResult(interp, Tcl_NewIntObj(SQLITE_PTR_TO_INT(id)));
  UNUSED_PARAMETER(clientData);
  UNUSED_PARAMETER(objc);
  UNUSED_PARAMETER(objv);
  परत TCL_OK;
}


/*
** Dispatch routine for the sub-commands of [sqlthread].
*/
स्थिर पूर्णांक SQLITE_TCLAPI sqlthread_proc(
  ClientData clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  संरचना SubCommand {
    अक्षर *zName;
    Tcl_ObjCmdProc *xProc;
    पूर्णांक nArg;
    अक्षर *zUsage;
  } aSub[] = {
    {"parent", sqlthread_parent, 1, "SCRIPT"},
    {"spawn",  sqlthread_spawn,  2, "VARNAME SCRIPT"},
    {"open",   sqlthread_open,   1, "DBNAME"},
    {"id",     sqlthread_id,     0, ""},
    {0, 0, 0}
  };
  संरचना SubCommand *pSub;
  पूर्णांक rc;
  पूर्णांक iIndex;

  जर( objc<2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "SUB-COMMAND");
    परत TCL_ERROR;
  }

  rc = Tcl_GetIndexFromObjStruct(
      interp, objv[1], aSub, आकार(aSub[0]), "sub-command", 0, &iIndex
  );
  जर( rc!=TCL_OK ) परत rc;
  pSub = &aSub[iIndex];

  जर( objc<(pSub->nArg+2) ){
    Tcl_Wrong_अंक_Args(interp, 2, objv, pSub->zUsage);
    परत TCL_ERROR;
  }

  परत pSub->xProc(clientData, interp, objc, objv);
}

/*
** The [clock_seconds] command. This is more or less the same as the
** regular tcl [clock seconds], except that it is available in testfixture
** when linked against both Tcl 8.4 and 8.5. Because [clock seconds] is
** implemented as a script in Tcl 8.5, it is not usually available to
** testfixture.
*/ 
स्थिर पूर्णांक SQLITE_TCLAPI clock_seconds_proc(
  ClientData clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Time now;
  Tcl_GetTime(&now);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(now.sec));
  UNUSED_PARAMETER(clientData);
  UNUSED_PARAMETER(objc);
  UNUSED_PARAMETER(objv);
  परत TCL_OK;
}

/*
** The [clock_milliseconds] command. This is more or less the same as the
** regular tcl [clock milliseconds]. 
*/ 
स्थिर पूर्णांक SQLITE_TCLAPI clock_milliseconds_proc(
  ClientData clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Time now;
  Tcl_GetTime(&now);
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(
    ((Tcl_WideInt)now.sec * 1000) + (now.usec / 1000)
  ));
  UNUSED_PARAMETER(clientData);
  UNUSED_PARAMETER(objc);
  UNUSED_PARAMETER(objv);
  परत TCL_OK;
}

/*************************************************************************
** This block contains the implementation of the [sqlite3_blocking_step]
** command available to threads created by [sqlthread spawn] commands. It
** is only available on UNIX for now. This is because pthread condition
** variables are used.
**
** The source code for the C functions sqlite3_blocking_step(),
** blocking_step_notify() and the structure UnlockNotification is
** automatically extracted from this file and used as part of the
** documentation for the sqlite3_unlock_notify() API function. This
** should be considered if these functions are to be extended (i.e. to 
** support windows) in the future.
*/ 
#जर SQLITE_OS_UNIX && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)

/* BEGIN_SQLITE_BLOCKING_STEP */
/* This example uses the pthreads API */
#अन्तर्भाव <pthread.h>

/*
** A pointer to an instance of this structure is passed as the user-context
** pointer when registering for an unlock-notify callback.
*/
प्रकारव्याख्या संरचना UnlockNotification UnlockNotification;
संरचना UnlockNotification {
  पूर्णांक fired;                         /* True after unlock event has occurred */
  pthread_cond_t cond;               /* Condition variable to wait on */
  pthread_mutex_t mutex;             /* Mutex to protect structure */
};

/*
** This function is an unlock-notify callback registered with SQLite.
*/
स्थिर शून्य unlock_notify_cb(शून्य **apArg, पूर्णांक nArg){
  पूर्णांक i;
  साठी(i=0; i<nArg; i++){
    UnlockNotification *p = (UnlockNotification *)apArg[i];
    pthread_mutex_lock(&p->mutex);
    p->fired = 1;
    pthread_cond_signal(&p->cond);
    pthread_mutex_unlock(&p->mutex);
  }
}

/*
** This function assumes that an SQLite API call (either sqlite3_prepare_v2() 
** or sqlite3_step()) has just returned SQLITE_LOCKED. The argument is the
** associated database connection.
**
** This function calls sqlite3_unlock_notify() to register for an 
** unlock-notify callback, then blocks until that callback is delivered 
** and returns SQLITE_OK. The caller should then retry the failed operation.
**
** Or, if sqlite3_unlock_notify() indicates that to block would deadlock 
** the system, then this function returns SQLITE_LOCKED immediately. In 
** this case the caller should not retry the operation and should roll 
** back the current transaction (if any).
*/
स्थिर पूर्णांक wait_for_unlock_notify(sqlite3 *db){
  पूर्णांक rc;
  UnlockNotification un;

  /* Initialize the UnlockNotification structure. */
  un.fired = 0;
  pthread_mutex_init(&un.mutex, 0);
  pthread_cond_init(&un.cond, 0);

  /* Register for an unlock-notify callback. */
  rc = sqlite3_unlock_notify(db, unlock_notify_cb, (शून्य *)&un);
  प्रतिज्ञा( rc==SQLITE_LOCKED || rc==SQLITE_OK );

  /* The call to sqlite3_unlock_notify() always returns either SQLITE_LOCKED 
  ** or SQLITE_OK. 
  **
  ** If SQLITE_LOCKED was returned, then the system is deadlocked. In this
  ** case this function needs to return SQLITE_LOCKED to the caller so 
  ** that the current transaction can be rolled back. Otherwise, block
  ** until the unlock-notify callback is invoked, then return SQLITE_OK.
  */
  जर( rc==SQLITE_OK ){
    pthread_mutex_lock(&un.mutex);
    जर( !un.fired ){
      pthread_cond_wait(&un.cond, &un.mutex);
    }
    pthread_mutex_unlock(&un.mutex);
  }

  /* Destroy the mutex and condition variables. */
  pthread_cond_destroy(&un.cond);
  pthread_mutex_destroy(&un.mutex);

  परत rc;
}

/*
** This function is a wrapper around the SQLite function sqlite3_step().
** It functions in the same way as step(), except that if a required
** shared-cache lock cannot be obtained, this function may block waiting for
** the lock to become available. In this scenario the normal API step()
** function always returns SQLITE_LOCKED.
**
** If this function returns SQLITE_LOCKED, the caller should rollback
** the current transaction (if any) and try again later. Otherwise, the
** system may become deadlocked.
*/
पूर्णांक sqlite3_blocking_step(sqlite3_stmt *pStmt){
  पूर्णांक rc;
  पर्यंत( SQLITE_LOCKED==(rc = sqlite3_step(pStmt)) ){
    rc = wait_for_unlock_notify(sqlite3_db_handle(pStmt));
    जर( rc!=SQLITE_OK ) थांबा;
    sqlite3_reset(pStmt);
  }
  परत rc;
}

/*
** This function is a wrapper around the SQLite function sqlite3_prepare_v2().
** It functions in the same way as prepare_v2(), except that if a required
** shared-cache lock cannot be obtained, this function may block waiting for
** the lock to become available. In this scenario the normal API prepare_v2()
** function always returns SQLITE_LOCKED.
**
** If this function returns SQLITE_LOCKED, the caller should rollback
** the current transaction (if any) and try again later. Otherwise, the
** system may become deadlocked.
*/
पूर्णांक sqlite3_blocking_prepare_v2(
  sqlite3 *db,              /* Database handle. */
  स्थिरांक अक्षर *zSql,         /* UTF-8 encoded SQL statement. */
  पूर्णांक nSql,                 /* Length of zSql in bytes. */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  स्थिरांक अक्षर **pz           /* OUT: End of parsed string */
){
  पूर्णांक rc;
  पर्यंत( SQLITE_LOCKED==(rc = sqlite3_prepare_v2(db, zSql, nSql, ppStmt, pz)) ){
    rc = wait_for_unlock_notify(db);
    जर( rc!=SQLITE_OK ) थांबा;
  }
  परत rc;
}
/* END_SQLITE_BLOCKING_STEP */

/*
** Usage: sqlite3_blocking_step STMT
**
** Advance the statement to the next row.
*/
स्थिर पूर्णांक SQLITE_TCLAPI blocking_step_proc(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){

  sqlite3_stmt *pStmt;
  पूर्णांक rc;

  जर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "STMT");
    परत TCL_ERROR;
  }

  pStmt = (sqlite3_stmt*)sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  rc = sqlite3_blocking_step(pStmt);

  Tcl_SetResult(interp, (अक्षर *)sqlite3ErrName(rc), 0);
  परत TCL_OK;
}

/*
** Usage: sqlite3_blocking_prepare_v2 DB sql bytes ?tailvar?
** Usage: sqlite3_nonblocking_prepare_v2 DB sql bytes ?tailvar?
*/
स्थिर पूर्णांक SQLITE_TCLAPI blocking_prepare_v2_proc(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  स्थिरांक अक्षर *zSql;
  पूर्णांक bytes;
  स्थिरांक अक्षर *zTail = 0;
  sqlite3_stmt *pStmt = 0;
  अक्षर zBuf[50];
  पूर्णांक rc;
  पूर्णांक isBlocking = !(clientData==0);

  जर( objc!=5 && objc!=4 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", 
       Tcl_GetString(objv[0]), " DB sql bytes tailvar", 0);
    परत TCL_ERROR;
  }
  जर( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) परत TCL_ERROR;
  zSql = Tcl_GetString(objv[2]);
  जर( Tcl_GetIntFromObj(interp, objv[3], &bytes) ) परत TCL_ERROR;

  जर( isBlocking ){
    rc = sqlite3_blocking_prepare_v2(db, zSql, bytes, &pStmt, &zTail);
  }अन्यथा{
    rc = sqlite3_prepare_v2(db, zSql, bytes, &pStmt, &zTail);
  }

  प्रतिज्ञा(rc==SQLITE_OK || pStmt==0);
  जर( zTail && objc>=5 ){
    जर( bytes>=0 ){
      bytes = bytes - (zTail-zSql);
    }
    Tcl_ObjSetVar2(interp, objv[4], 0, Tcl_NewStringObj(zTail, bytes), 0);
  }
  जर( rc!=SQLITE_OK ){
    प्रतिज्ञा( pStmt==0 );
    sqlite3_snprintf(आकार(zBuf), zBuf, "%s ", (अक्षर *)sqlite3ErrName(rc));
    Tcl_AppendResult(interp, zBuf, sqlite3_errmsg(db), रिक्त);
    परत TCL_ERROR;
  }

  जर( pStmt ){
    जर( sqlite3TestMakePointerStr(interp, zBuf, pStmt) ) परत TCL_ERROR;
    Tcl_AppendResult(interp, zBuf, रिक्त);
  }
  परत TCL_OK;
}

#endif /* SQLITE_OS_UNIX && SQLITE_ENABLE_UNLOCK_NOTIFY */
/*
** End of implementation of [sqlite3_blocking_step].
************************************************************************/

/*
** Register commands with the TCL interpreter.
*/
पूर्णांक SqlitetestThread_Init(Tcl_Interp *interp){
  संरचना TclCmd {
    पूर्णांक (*xProc)(शून्य*, Tcl_Interp*, पूर्णांक, Tcl_Obj*स्थिरांक*);
    स्थिरांक अक्षर *zName;
    पूर्णांक iCtx;
  } aCmd[] = {
    { sqlthread_proc,           "sqlthread",                      0 },
    { clock_seconds_proc,       "clock_second",                   0 },
    { clock_milliseconds_proc,  "clock_milliseconds",             0 },
#जर SQLITE_OS_UNIX && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)
    { blocking_step_proc,       "sqlite3_blocking_step",          0 },
    { blocking_prepare_v2_proc, "sqlite3_blocking_prepare_v2",    1 },
    { blocking_prepare_v2_proc, "sqlite3_nonblocking_prepare_v2", 0 },
#endif
  };
  पूर्णांक ii;

  साठी(ii=0; ii<आकार(aCmd)/आकार(aCmd[0]); ii++){
    शून्य *p = SQLITE_INT_TO_PTR(aCmd[ii].iCtx);
    Tcl_CreateObjCommand(interp, aCmd[ii].zName, aCmd[ii].xProc, p, 0);
  }
  परत TCL_OK;
}
#अन्यथा
पूर्णांक SqlitetestThread_Init(Tcl_Interp *interp){
  परत TCL_OK;
}
#endif
