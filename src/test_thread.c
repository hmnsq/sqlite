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

#شامل "sqliteInt.h"
#شامل "tclsqlite.h"

#اگر SQLITE_THREADSAFE

#شامل <errno.h>

#اگر !defined(_MSC_VER)
#شامل <unistd.h>
#endif

/*
** One of these is allocated for each thread created by [sqlthread spawn].
*/
تعریفنوع ساختار SqlThread SqlThread;
ساختار SqlThread {
  Tcl_ThreadId parent;     /* Thread id of parent thread */
  Tcl_Interp *interp;      /* Parent interpreter */
  نویسه *zScript;           /* The script to execute. */
  نویسه *zVarname;          /* Varname in parent script */
};

/*
** A custom Tcl_Event type used by this module. When the event is
** handled, script zScript is evaluated in interpreter interp. If
** the evaluation throws an exception (returns TCL_ERROR), then the
** error is handled by Tcl_BackgroundError(). If no error occurs,
** the result is simply discarded.
*/
تعریفنوع ساختار EvalEvent EvalEvent;
ساختار EvalEvent {
  Tcl_Event base;          /* Base class of type Tcl_Event */
  نویسه *zScript;           /* The script to execute. */
  Tcl_Interp *interp;      /* The interpreter to execute it in. */
};

ایستا Tcl_ObjCmdProc sqlthread_proc;
ایستا Tcl_ObjCmdProc clock_seconds_proc;
#اگر SQLITE_OS_UNIX && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)
ایستا Tcl_ObjCmdProc blocking_step_proc;
ایستا Tcl_ObjCmdProc blocking_prepare_v2_proc;
#endif
صحیح Sqlitetest1_Init(Tcl_Interp *);
صحیح Sqlite3_Init(Tcl_Interp *);

/* Functions from main.c */
بیرونی ثابت نویسه *sqlite3ErrName(صحیح);

/* Functions from test1.c */
بیرونی تهی *sqlite3TestTextToPtr(ثابت نویسه *);
بیرونی صحیح getDbPointer(Tcl_Interp *, ثابت نویسه *, sqlite3 **);
بیرونی صحیح sqlite3TestMakePointerStr(Tcl_Interp *, نویسه *, تهی *);
بیرونی صحیح sqlite3TestErrCode(Tcl_Interp *, sqlite3 *, صحیح);

/*
** Handler for events of type EvalEvent.
*/
ایستا صحیح SQLITE_TCLAPI tclScriptEvent(Tcl_Event *evPtr, صحیح flags){
  صحیح rc;
  EvalEvent *p = (EvalEvent *)evPtr;
  rc = Tcl_Eval(p->interp, p->zScript);
  اگر( rc!=TCL_OK ){
    Tcl_BackgroundError(p->interp);
  }
  UNUSED_PARAMETER(flags);
  بازگشت 1;
}

/*
** Register an EvalEvent to evaluate the script pScript in the
** parent interpreter/thread of SqlThread p.
*/
ایستا تهی postToParent(SqlThread *p, Tcl_Obj *pScript){
  EvalEvent *pEvent;
  نویسه *z_پیام;
  Tcl_Size n_پیام;

  z_پیام = Tcl_GetStringFromObj(pScript, &n_پیام); 
  pEvent = (EvalEvent *)ckalloc(اندازه(EvalEvent)+n_پیام+1);
  pEvent->base.nextPtr = 0;
  pEvent->base.proc = tclScriptEvent;
  pEvent->zScript = (نویسه *)&pEvent[1];
  memcpy(pEvent->zScript, z_پیام, n_پیام+1);
  pEvent->interp = p->interp;

  Tcl_ThreadQueueEvent(p->parent, (Tcl_Event *)pEvent, TCL_QUEUE_TAIL);
  Tcl_ThreadAlert(p->parent);
}

/*
** The main function for threads created with [sqlthread spawn].
*/
ایستا Tcl_ThreadCreateType tclScriptThread(ClientData pSqlThread){
  Tcl_Interp *interp;
  Tcl_Obj *pRes;
  Tcl_Obj *pList;
  صحیح rc;
  SqlThread *p = (SqlThread *)pSqlThread;
  بیرونی صحیح Sqlitetest_mutex_Init(Tcl_Interp*);

  interp = Tcl_CreateInterp();
  Tcl_CreateObjCommand(interp, "clock_seconds", clock_seconds_proc, 0, 0);
  Tcl_CreateObjCommand(interp, "sqlthread", sqlthread_proc, pSqlThread, 0);
#اگر SQLITE_OS_UNIX && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)
  Tcl_CreateObjCommand(interp, "sqlite3_blocking_step", blocking_step_proc,0,0);
  Tcl_CreateObjCommand(interp, 
      "sqlite3_blocking_prepare_v2", blocking_prepare_v2_proc, (تهی *)1, 0);
  Tcl_CreateObjCommand(interp, 
      "sqlite3_nonblocking_prepare_v2", blocking_prepare_v2_proc, 0, 0);
#endif
  Sqlitetest1_Init(interp);
  Sqlitetest_mutex_Init(interp);
  Sqlite3_Init(interp);

  rc = Tcl_Eval(interp, p->zScript);
  pRes = Tcl_GetObjResult(interp);
  pList = Tcl_NewObj();
  Tcl_Incr_Ref_شمار(pList);
  Tcl_Incr_Ref_شمار(pRes);

  اگر( rc!=TCL_OK ){
    Tcl_ListObjAppendElement(interp, pList, Tcl_NewStringObj("error", -1));
    Tcl_ListObjAppendElement(interp, pList, pRes);
    postToParent(p, pList);
    Tcl_Decr_Ref_شمار(pList);
    pList = Tcl_NewObj();
  }

  Tcl_ListObjAppendElement(interp, pList, Tcl_NewStringObj("set", -1));
  Tcl_ListObjAppendElement(interp, pList, Tcl_NewStringObj(p->zVarname, -1));
  Tcl_ListObjAppendElement(interp, pList, pRes);
  postToParent(p, pList);

  ckfree((تهی *)p);
  Tcl_Decr_Ref_شمار(pList);
  Tcl_Decr_Ref_شمار(pRes);
  Tcl_DeleteInterp(interp);
  وقتی( Tcl_DoOneEvent(TCL_ALL_EVENTS|TCL_DONT_WAIT) );
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
ایستا صحیح SQLITE_TCLAPI sqlthread_spawn(
  ClientData clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_ThreadId x;
  SqlThread *pNew;
  صحیح rc;

  Tcl_Size nVarname; نویسه *zVarname;
  Tcl_Size nScript; نویسه *zScript;

  /* Parameters for thread creation */
  ثابت صحیح n_پشته = TCL_THREAD_پشته_DEFAULT;
  ثابت صحیح flags = TCL_THREAD_NOFLAGS;

  أكد(objc==4);
  UNUSED_PARAMETER(clientData);
  UNUSED_PARAMETER(objc);

  zVarname = Tcl_GetStringFromObj(objv[2], &nVarname);
  zScript = Tcl_GetStringFromObj(objv[3], &nScript);

  pNew = (SqlThread *)ckalloc(اندازه(SqlThread)+nVarname+nScript+2);
  pNew->zVarname = (نویسه *)&pNew[1];
  pNew->zScript = (نویسه *)&pNew->zVarname[nVarname+1];
  memcpy(pNew->zVarname, zVarname, nVarname+1);
  memcpy(pNew->zScript, zScript, nScript+1);
  pNew->parent = Tcl_GetCurrentThread();
  pNew->interp = interp;

  rc = Tcl_CreateThread(&x, tclScriptThread, (تهی *)pNew, n_پشته, flags);
  اگر( rc!=TCL_OK ){
    Tcl_AppendResult(interp, "Error in Tcl_CreateThread()", پوچ);
    ckfree((نویسه *)pNew);
    بازگشت TCL_ERROR;
  }

  بازگشت TCL_OK;
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
ایستا صحیح SQLITE_TCLAPI sqlthread_parent(
  ClientData clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  EvalEvent *pEvent;
  نویسه *z_پیام;
  Tcl_Size n_پیام;
  SqlThread *p = (SqlThread *)clientData;

  أكد(objc==3);
  UNUSED_PARAMETER(objc);

  اگر( p==0 ){
    Tcl_AppendResult(interp, "no parent thread", پوچ);
    بازگشت TCL_ERROR;
  }

  z_پیام = Tcl_GetStringFromObj(objv[2], &n_پیام);
  pEvent = (EvalEvent *)ckalloc(اندازه(EvalEvent)+n_پیام+1);
  pEvent->base.nextPtr = 0;
  pEvent->base.proc = tclScriptEvent;
  pEvent->zScript = (نویسه *)&pEvent[1];
  memcpy(pEvent->zScript, z_پیام, n_پیام+1);
  pEvent->interp = p->interp;
  Tcl_ThreadQueueEvent(p->parent, (Tcl_Event *)pEvent, TCL_QUEUE_TAIL);
  Tcl_ThreadAlert(p->parent);

  بازگشت TCL_OK;
}

ایستا صحیح xBusy(تهی *pArg, صحیح nBusy){
  UNUSED_PARAMETER(pArg);
  UNUSED_PARAMETER(nBusy);
  sqlite3_sleep(50);
  بازگشت 1;             /* Try again... */
}

/*
** sqlthread open
**
**     Open a database handle and return the string representation of
**     the pointer value.
*/
ایستا صحیح SQLITE_TCLAPI sqlthread_open(
  ClientData clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح sqlite3TestMakePointerStr(Tcl_Interp *interp, نویسه *zPtr, تهی *p);

  ثابت نویسه *zFilename;
  sqlite3 *db;
  نویسه zBuf[100];
  بیرونی صحیح Md5_Register(sqlite3*,نویسه**,ثابت sqlite3_api_routines*);

  UNUSED_PARAMETER(clientData);
  UNUSED_PARAMETER(objc);

  zFilename = Tcl_GetString(objv[2]);
  sqlite3_open(zFilename, &db);
  Md5_Register(db, 0, 0);
  sqlite3_busy_handler(db, xBusy, 0);
  
  اگر( sqlite3TestMakePointerStr(interp, zBuf, db) ) بازگشت TCL_ERROR;
  Tcl_AppendResult(interp, zBuf, پوچ);

  بازگشت TCL_OK;
}


/*
** sqlthread open
**
**     Return the current thread-id (Tcl_GetCurrentThread()) cast to
**     an integer.
*/
ایستا صحیح SQLITE_TCLAPI sqlthread_id(
  ClientData clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_ThreadId id = Tcl_GetCurrentThread();
  Tcl_SetObjResult(interp, Tcl_NewIntObj(SQLITE_PTR_TO_INT(id)));
  UNUSED_PARAMETER(clientData);
  UNUSED_PARAMETER(objc);
  UNUSED_PARAMETER(objv);
  بازگشت TCL_OK;
}


/*
** Dispatch routine for the sub-commands of [sqlthread].
*/
ایستا صحیح SQLITE_TCLAPI sqlthread_proc(
  ClientData clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  ساختار SubCommand {
    نویسه *zName;
    Tcl_ObjCmdProc *xProc;
    صحیح nArg;
    نویسه *zUsage;
  } aSub[] = {
    {"parent", sqlthread_parent, 1, "SCRIPT"},
    {"spawn",  sqlthread_spawn,  2, "VARNAME SCRIPT"},
    {"open",   sqlthread_open,   1, "DBNAME"},
    {"id",     sqlthread_id,     0, ""},
    {0, 0, 0}
  };
  ساختار SubCommand *pSub;
  صحیح rc;
  صحیح iIndex;

  اگر( objc<2 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "SUB-COMMAND");
    بازگشت TCL_ERROR;
  }

  rc = Tcl_GetIndexFromObjStruct(
      interp, objv[1], aSub, اندازه(aSub[0]), "sub-command", 0, &iIndex
  );
  اگر( rc!=TCL_OK ) بازگشت rc;
  pSub = &aSub[iIndex];

  اگر( objc<(pSub->nArg+2) ){
    Tcl_Wrong_رقم_Args(interp, 2, objv, pSub->zUsage);
    بازگشت TCL_ERROR;
  }

  بازگشت pSub->xProc(clientData, interp, objc, objv);
}

/*
** The [clock_seconds] command. This is more or less the same as the
** regular tcl [clock seconds], except that it is available in testfixture
** when linked against both Tcl 8.4 and 8.5. Because [clock seconds] is
** implemented as a script in Tcl 8.5, it is not usually available to
** testfixture.
*/ 
ایستا صحیح SQLITE_TCLAPI clock_seconds_proc(
  ClientData clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Time now;
  Tcl_GetTime(&now);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(now.sec));
  UNUSED_PARAMETER(clientData);
  UNUSED_PARAMETER(objc);
  UNUSED_PARAMETER(objv);
  بازگشت TCL_OK;
}

/*
** The [clock_milliseconds] command. This is more or less the same as the
** regular tcl [clock milliseconds]. 
*/ 
ایستا صحیح SQLITE_TCLAPI clock_milliseconds_proc(
  ClientData clientData,
  Tcl_Interp *interp,
  صحیح objc,
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
  بازگشت TCL_OK;
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
#اگر SQLITE_OS_UNIX && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)

/* BEGIN_SQLITE_BLOCKING_STEP */
/* This example uses the pthreads API */
#شامل <pthread.h>

/*
** A pointer to an instance of this structure is passed as the user-context
** pointer when registering for an unlock-notify callback.
*/
تعریفنوع ساختار UnlockNotification UnlockNotification;
ساختار UnlockNotification {
  صحیح fired;                         /* True after unlock event has occurred */
  pthread_cond_t cond;               /* Condition variable to wait on */
  pthread_mutex_t mutex;             /* Mutex to protect structure */
};

/*
** This function is an unlock-notify callback registered with SQLite.
*/
ایستا تهی unlock_notify_cb(تهی **apArg, صحیح nArg){
  صحیح i;
  برای(i=0; i<nArg; i++){
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
ایستا صحیح wait_for_unlock_notify(sqlite3 *db){
  صحیح rc;
  UnlockNotification un;

  /* Initialize the UnlockNotification structure. */
  un.fired = 0;
  pthread_mutex_init(&un.mutex, 0);
  pthread_cond_init(&un.cond, 0);

  /* Register for an unlock-notify callback. */
  rc = sqlite3_unlock_notify(db, unlock_notify_cb, (تهی *)&un);
  أكد( rc==SQLITE_LOCKED || rc==SQLITE_OK );

  /* The call to sqlite3_unlock_notify() always returns either SQLITE_LOCKED 
  ** or SQLITE_OK. 
  **
  ** If SQLITE_LOCKED was returned, then the system is deadlocked. In this
  ** case this function needs to return SQLITE_LOCKED to the caller so 
  ** that the current transaction can be rolled back. Otherwise, block
  ** until the unlock-notify callback is invoked, then return SQLITE_OK.
  */
  اگر( rc==SQLITE_OK ){
    pthread_mutex_lock(&un.mutex);
    اگر( !un.fired ){
      pthread_cond_wait(&un.cond, &un.mutex);
    }
    pthread_mutex_unlock(&un.mutex);
  }

  /* Destroy the mutex and condition variables. */
  pthread_cond_destroy(&un.cond);
  pthread_mutex_destroy(&un.mutex);

  بازگشت rc;
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
صحیح sqlite3_blocking_step(sqlite3_stmt *pStmt){
  صحیح rc;
  وقتی( SQLITE_LOCKED==(rc = sqlite3_step(pStmt)) ){
    rc = wait_for_unlock_notify(sqlite3_db_handle(pStmt));
    اگر( rc!=SQLITE_OK ) بشکن;
    sqlite3_reset(pStmt);
  }
  بازگشت rc;
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
صحیح sqlite3_blocking_prepare_v2(
  sqlite3 *db,              /* Database handle. */
  ثابت نویسه *zSql,         /* UTF-8 encoded SQL statement. */
  صحیح nSql,                 /* Length of zSql in bytes. */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  ثابت نویسه **pz           /* OUT: End of parsed string */
){
  صحیح rc;
  وقتی( SQLITE_LOCKED==(rc = sqlite3_prepare_v2(db, zSql, nSql, ppStmt, pz)) ){
    rc = wait_for_unlock_notify(db);
    اگر( rc!=SQLITE_OK ) بشکن;
  }
  بازگشت rc;
}
/* END_SQLITE_BLOCKING_STEP */

/*
** Usage: sqlite3_blocking_step STMT
**
** Advance the statement to the next row.
*/
ایستا صحیح SQLITE_TCLAPI blocking_step_proc(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){

  sqlite3_stmt *pStmt;
  صحیح rc;

  اگر( objc!=2 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "STMT");
    بازگشت TCL_ERROR;
  }

  pStmt = (sqlite3_stmt*)sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  rc = sqlite3_blocking_step(pStmt);

  Tcl_SetResult(interp, (نویسه *)sqlite3ErrName(rc), 0);
  بازگشت TCL_OK;
}

/*
** Usage: sqlite3_blocking_prepare_v2 DB sql bytes ?tailvar?
** Usage: sqlite3_nonblocking_prepare_v2 DB sql bytes ?tailvar?
*/
ایستا صحیح SQLITE_TCLAPI blocking_prepare_v2_proc(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  ثابت نویسه *zSql;
  صحیح bytes;
  ثابت نویسه *zTail = 0;
  sqlite3_stmt *pStmt = 0;
  نویسه zBuf[50];
  صحیح rc;
  صحیح isBlocking = !(clientData==0);

  اگر( objc!=5 && objc!=4 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", 
       Tcl_GetString(objv[0]), " DB sql bytes tailvar", 0);
    بازگشت TCL_ERROR;
  }
  اگر( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) بازگشت TCL_ERROR;
  zSql = Tcl_GetString(objv[2]);
  اگر( Tcl_GetIntFromObj(interp, objv[3], &bytes) ) بازگشت TCL_ERROR;

  اگر( isBlocking ){
    rc = sqlite3_blocking_prepare_v2(db, zSql, bytes, &pStmt, &zTail);
  }وگرنه{
    rc = sqlite3_prepare_v2(db, zSql, bytes, &pStmt, &zTail);
  }

  أكد(rc==SQLITE_OK || pStmt==0);
  اگر( zTail && objc>=5 ){
    اگر( bytes>=0 ){
      bytes = bytes - (zTail-zSql);
    }
    Tcl_ObjSetVar2(interp, objv[4], 0, Tcl_NewStringObj(zTail, bytes), 0);
  }
  اگر( rc!=SQLITE_OK ){
    أكد( pStmt==0 );
    sqlite3_snprintf(اندازه(zBuf), zBuf, "%s ", (نویسه *)sqlite3ErrName(rc));
    Tcl_AppendResult(interp, zBuf, sqlite3_errmsg(db), پوچ);
    بازگشت TCL_ERROR;
  }

  اگر( pStmt ){
    اگر( sqlite3TestMakePointerStr(interp, zBuf, pStmt) ) بازگشت TCL_ERROR;
    Tcl_AppendResult(interp, zBuf, پوچ);
  }
  بازگشت TCL_OK;
}

#endif /* SQLITE_OS_UNIX && SQLITE_ENABLE_UNLOCK_NOTIFY */
/*
** End of implementation of [sqlite3_blocking_step].
************************************************************************/

/*
** Register commands with the TCL interpreter.
*/
صحیح SqlitetestThread_Init(Tcl_Interp *interp){
  ساختار TclCmd {
    صحیح (*xProc)(تهی*, Tcl_Interp*, صحیح, Tcl_Obj*ثابت*);
    ثابت نویسه *zName;
    صحیح iCtx;
  } aCmd[] = {
    { sqlthread_proc,           "sqlthread",                      0 },
    { clock_seconds_proc,       "clock_second",                   0 },
    { clock_milliseconds_proc,  "clock_milliseconds",             0 },
#اگر SQLITE_OS_UNIX && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)
    { blocking_step_proc,       "sqlite3_blocking_step",          0 },
    { blocking_prepare_v2_proc, "sqlite3_blocking_prepare_v2",    1 },
    { blocking_prepare_v2_proc, "sqlite3_nonblocking_prepare_v2", 0 },
#endif
  };
  صحیح ii;

  برای(ii=0; ii<اندازه(aCmd)/اندازه(aCmd[0]); ii++){
    تهی *p = SQLITE_INT_TO_PTR(aCmd[ii].iCtx);
    Tcl_CreateObjCommand(interp, aCmd[ii].zName, aCmd[ii].xProc, p, 0);
  }
  بازگشت TCL_OK;
}
#وگرنه
صحیح SqlitetestThread_Init(Tcl_Interp *interp){
  بازگشت TCL_OK;
}
#endif
