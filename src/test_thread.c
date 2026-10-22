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

#অন্তর্ভাব "sqliteInt.h"
#অন্তর্ভাব "tclsqlite.h"

#যদি SQLITE_THREADSAFE

#অন্তর্ভাব <errno.h>

#যদি !defined(_MSC_VER)
#অন্তর্ভাব <unistd.h>
#endif

/*
** One of these is allocated for each thread created by [sqlthread spawn].
*/
প্রকারলেখ কাঠামো SqlThread SqlThread;
কাঠামো SqlThread {
  Tcl_ThreadId parent;     /* Thread id of parent thread */
  Tcl_Interp *interp;      /* Parent interpreter */
  অক্ষর *zScript;           /* The script to execute. */
  অক্ষর *zVarname;          /* Varname in parent script */
};

/*
** A custom Tcl_Event type used by this module. When the event is
** handled, script zScript is evaluated in interpreter interp. If
** the evaluation throws an exception (returns TCL_ERROR), then the
** error is handled by Tcl_BackgroundError(). If no error occurs,
** the result is simply discarded.
*/
প্রকারলেখ কাঠামো EvalEvent EvalEvent;
কাঠামো EvalEvent {
  Tcl_Event base;          /* Base class of type Tcl_Event */
  অক্ষর *zScript;           /* The script to execute. */
  Tcl_Interp *interp;      /* The interpreter to execute it in. */
};

স্থির Tcl_ObjCmdProc sqlthread_proc;
স্থির Tcl_ObjCmdProc clock_seconds_proc;
#যদি SQLITE_OS_UNIX && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)
স্থির Tcl_ObjCmdProc blocking_step_proc;
স্থির Tcl_ObjCmdProc blocking_prepare_v2_proc;
#endif
পূর্ণসংখ্যা Sqlitetest1_Init(Tcl_Interp *);
পূর্ণসংখ্যা Sqlite3_Init(Tcl_Interp *);

/* Functions from main.c */
বাহ্যিক ধ্রুবক অক্ষর *sqlite3ErrName(পূর্ণসংখ্যা);

/* Functions from test1.c */
বাহ্যিক শূন্য *sqlite3TestTextToPtr(ধ্রুবক অক্ষর *);
বাহ্যিক পূর্ণসংখ্যা getDbPointer(Tcl_Interp *, ধ্রুবক অক্ষর *, sqlite3 **);
বাহ্যিক পূর্ণসংখ্যা sqlite3TestMakePointerStr(Tcl_Interp *, অক্ষর *, শূন্য *);
বাহ্যিক পূর্ণসংখ্যা sqlite3TestErrCode(Tcl_Interp *, sqlite3 *, পূর্ণসংখ্যা);

/*
** Handler for events of type EvalEvent.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI tclScriptEvent(Tcl_Event *evPtr, পূর্ণসংখ্যা flags){
  পূর্ণসংখ্যা rc;
  EvalEvent *p = (EvalEvent *)evPtr;
  rc = Tcl_Eval(p->interp, p->zScript);
  যদি( rc!=TCL_OK ){
    Tcl_BackgroundError(p->interp);
  }
  UNUSED_PARAMETER(flags);
  ফেরত 1;
}

/*
** Register an EvalEvent to evaluate the script pScript in the
** parent interpreter/thread of SqlThread p.
*/
স্থির শূন্য postToParent(SqlThread *p, Tcl_Obj *pScript){
  EvalEvent *pEvent;
  অক্ষর *z_বার্তা;
  Tcl_আকার n_বার্তা;

  z_বার্তা = Tcl_GetStringFromObj(pScript, &n_বার্তা); 
  pEvent = (EvalEvent *)ckalloc(আকার(EvalEvent)+n_বার্তা+1);
  pEvent->base.পরবর্তী_Ptr = 0;
  pEvent->base.proc = tclScriptEvent;
  pEvent->zScript = (অক্ষর *)&pEvent[1];
  memcpy(pEvent->zScript, z_বার্তা, n_বার্তা+1);
  pEvent->interp = p->interp;

  Tcl_ThreadQueueEvent(p->parent, (Tcl_Event *)pEvent, TCL_QUEUE_লেজ);
  Tcl_ThreadAlert(p->parent);
}

/*
** The main function for threads created with [sqlthread spawn].
*/
স্থির Tcl_ThreadCreateType tclScriptThread(Client_তথ্য pSqlThread){
  Tcl_Interp *interp;
  Tcl_Obj *pRes;
  Tcl_Obj *pList;
  পূর্ণসংখ্যা rc;
  SqlThread *p = (SqlThread *)pSqlThread;
  বাহ্যিক পূর্ণসংখ্যা Sqlitetest_mutex_Init(Tcl_Interp*);

  interp = Tcl_CreateInterp();
  Tcl_CreateObjCommand(interp, "clock_seconds", clock_seconds_proc, 0, 0);
  Tcl_CreateObjCommand(interp, "sqlthread", sqlthread_proc, pSqlThread, 0);
#যদি SQLITE_OS_UNIX && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)
  Tcl_CreateObjCommand(interp, "sqlite3_blocking_step", blocking_step_proc,0,0);
  Tcl_CreateObjCommand(interp, 
      "sqlite3_blocking_prepare_v2", blocking_prepare_v2_proc, (শূন্য *)1, 0);
  Tcl_CreateObjCommand(interp, 
      "sqlite3_nonblocking_prepare_v2", blocking_prepare_v2_proc, 0, 0);
#endif
  Sqlitetest1_Init(interp);
  Sqlitetest_mutex_Init(interp);
  Sqlite3_Init(interp);

  rc = Tcl_Eval(interp, p->zScript);
  pRes = Tcl_Get_Obj_ফলাফল(interp);
  pList = Tcl_NewObj();
  Tcl_Incr_Ref_গুনতি(pList);
  Tcl_Incr_Ref_গুনতি(pRes);

  যদি( rc!=TCL_OK ){
    Tcl_ListObjAppendElement(interp, pList, Tcl_NewStringObj("error", -1));
    Tcl_ListObjAppendElement(interp, pList, pRes);
    postToParent(p, pList);
    Tcl_Decr_Ref_গুনতি(pList);
    pList = Tcl_NewObj();
  }

  Tcl_ListObjAppendElement(interp, pList, Tcl_NewStringObj("set", -1));
  Tcl_ListObjAppendElement(interp, pList, Tcl_NewStringObj(p->zVarname, -1));
  Tcl_ListObjAppendElement(interp, pList, pRes);
  postToParent(p, pList);

  ckfree((শূন্য *)p);
  Tcl_Decr_Ref_গুনতি(pList);
  Tcl_Decr_Ref_গুনতি(pRes);
  Tcl_DeleteInterp(interp);
  যখন( Tcl_DoOneEvent(TCL_ALL_EVENTS|TCL_DONT_WAIT) );
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
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI sqlthread_spawn(
  Client_তথ্য client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_ThreadId x;
  SqlThread *pNew;
  পূর্ণসংখ্যা rc;

  Tcl_আকার nVarname; অক্ষর *zVarname;
  Tcl_আকার nScript; অক্ষর *zScript;

  /* Parameters for thread creation */
  ধ্রুবক পূর্ণসংখ্যা n_স্তূপ = TCL_THREAD_স্তূপ_DEFAULT;
  ধ্রুবক পূর্ণসংখ্যা flags = TCL_THREAD_NOFLAGS;

  প্রতিজ্ঞা(objc==4);
  UNUSED_PARAMETER(client_তথ্য);
  UNUSED_PARAMETER(objc);

  zVarname = Tcl_GetStringFromObj(objv[2], &nVarname);
  zScript = Tcl_GetStringFromObj(objv[3], &nScript);

  pNew = (SqlThread *)ckalloc(আকার(SqlThread)+nVarname+nScript+2);
  pNew->zVarname = (অক্ষর *)&pNew[1];
  pNew->zScript = (অক্ষর *)&pNew->zVarname[nVarname+1];
  memcpy(pNew->zVarname, zVarname, nVarname+1);
  memcpy(pNew->zScript, zScript, nScript+1);
  pNew->parent = Tcl_GetCurrentThread();
  pNew->interp = interp;

  rc = Tcl_CreateThread(&x, tclScriptThread, (শূন্য *)pNew, n_স্তূপ, flags);
  যদি( rc!=TCL_OK ){
    Tcl_Append_ফলাফল(interp, "Error in Tcl_CreateThread()", ফাঁকা);
    ckfree((অক্ষর *)pNew);
    ফেরত TCL_ERROR;
  }

  ফেরত TCL_OK;
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
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI sqlthread_parent(
  Client_তথ্য client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  EvalEvent *pEvent;
  অক্ষর *z_বার্তা;
  Tcl_আকার n_বার্তা;
  SqlThread *p = (SqlThread *)client_তথ্য;

  প্রতিজ্ঞা(objc==3);
  UNUSED_PARAMETER(objc);

  যদি( p==0 ){
    Tcl_Append_ফলাফল(interp, "no parent thread", ফাঁকা);
    ফেরত TCL_ERROR;
  }

  z_বার্তা = Tcl_GetStringFromObj(objv[2], &n_বার্তা);
  pEvent = (EvalEvent *)ckalloc(আকার(EvalEvent)+n_বার্তা+1);
  pEvent->base.পরবর্তী_Ptr = 0;
  pEvent->base.proc = tclScriptEvent;
  pEvent->zScript = (অক্ষর *)&pEvent[1];
  memcpy(pEvent->zScript, z_বার্তা, n_বার্তা+1);
  pEvent->interp = p->interp;
  Tcl_ThreadQueueEvent(p->parent, (Tcl_Event *)pEvent, TCL_QUEUE_লেজ);
  Tcl_ThreadAlert(p->parent);

  ফেরত TCL_OK;
}

স্থির পূর্ণসংখ্যা xBusy(শূন্য *pArg, পূর্ণসংখ্যা nBusy){
  UNUSED_PARAMETER(pArg);
  UNUSED_PARAMETER(nBusy);
  sqlite3_sleep(50);
  ফেরত 1;             /* Try again... */
}

/*
** sqlthread open
**
**     Open a database handle and return the string representation of
**     the pointer value.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI sqlthread_open(
  Client_তথ্য client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  পূর্ণসংখ্যা sqlite3TestMakePointerStr(Tcl_Interp *interp, অক্ষর *zPtr, শূন্য *p);

  ধ্রুবক অক্ষর *zFilename;
  sqlite3 *db;
  অক্ষর zBuf[100];
  বাহ্যিক পূর্ণসংখ্যা Md5_Register(sqlite3*,অক্ষর**,ধ্রুবক sqlite3_api_routines*);

  UNUSED_PARAMETER(client_তথ্য);
  UNUSED_PARAMETER(objc);

  zFilename = Tcl_GetString(objv[2]);
  sqlite3_open(zFilename, &db);
  Md5_Register(db, 0, 0);
  sqlite3_busy_handler(db, xBusy, 0);
  
  যদি( sqlite3TestMakePointerStr(interp, zBuf, db) ) ফেরত TCL_ERROR;
  Tcl_Append_ফলাফল(interp, zBuf, ফাঁকা);

  ফেরত TCL_OK;
}


/*
** sqlthread open
**
**     Return the current thread-id (Tcl_GetCurrentThread()) cast to
**     an integer.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI sqlthread_id(
  Client_তথ্য client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_ThreadId id = Tcl_GetCurrentThread();
  Tcl_Set_Obj_ফলাফল(interp, Tcl_NewIntObj(SQLITE_PTR_TO_INT(id)));
  UNUSED_PARAMETER(client_তথ্য);
  UNUSED_PARAMETER(objc);
  UNUSED_PARAMETER(objv);
  ফেরত TCL_OK;
}


/*
** Dispatch routine for the sub-commands of [sqlthread].
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI sqlthread_proc(
  Client_তথ্য client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  কাঠামো SubCommand {
    অক্ষর *zName;
    Tcl_ObjCmdProc *xProc;
    পূর্ণসংখ্যা nArg;
    অক্ষর *zUsage;
  } aSub[] = {
    {"parent", sqlthread_parent, 1, "SCRIPT"},
    {"spawn",  sqlthread_spawn,  2, "VARNAME SCRIPT"},
    {"open",   sqlthread_open,   1, "DBNAME"},
    {"id",     sqlthread_id,     0, ""},
    {0, 0, 0}
  };
  কাঠামো SubCommand *pSub;
  পূর্ণসংখ্যা rc;
  পূর্ণসংখ্যা i_সূচক;

  যদি( objc<2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "SUB-COMMAND");
    ফেরত TCL_ERROR;
  }

  rc = Tcl_Get_সূচক_From_Obj_Struct(
      interp, objv[1], aSub, আকার(aSub[0]), "sub-command", 0, &i_সূচক
  );
  যদি( rc!=TCL_OK ) ফেরত rc;
  pSub = &aSub[i_সূচক];

  যদি( objc<(pSub->nArg+2) ){
    Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, pSub->zUsage);
    ফেরত TCL_ERROR;
  }

  ফেরত pSub->xProc(client_তথ্য, interp, objc, objv);
}

/*
** The [clock_seconds] command. This is more or less the same as the
** regular tcl [clock seconds], except that it is available in testfixture
** when linked against both Tcl 8.4 and 8.5. Because [clock seconds] is
** implemented as a script in Tcl 8.5, it is not usually available to
** testfixture.
*/ 
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI clock_seconds_proc(
  Client_তথ্য client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Time now;
  Tcl_GetTime(&now);
  Tcl_Set_Obj_ফলাফল(interp, Tcl_NewIntObj(now.sec));
  UNUSED_PARAMETER(client_তথ্য);
  UNUSED_PARAMETER(objc);
  UNUSED_PARAMETER(objv);
  ফেরত TCL_OK;
}

/*
** The [clock_milliseconds] command. This is more or less the same as the
** regular tcl [clock milliseconds]. 
*/ 
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI clock_milliseconds_proc(
  Client_তথ্য client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Time now;
  Tcl_GetTime(&now);
  Tcl_Set_Obj_ফলাফল(interp, Tcl_NewWideIntObj(
    ((Tcl_WideInt)now.sec * 1000) + (now.usec / 1000)
  ));
  UNUSED_PARAMETER(client_তথ্য);
  UNUSED_PARAMETER(objc);
  UNUSED_PARAMETER(objv);
  ফেরত TCL_OK;
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
#যদি SQLITE_OS_UNIX && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)

/* BEGIN_SQLITE_BLOCKING_STEP */
/* This example uses the pthreads API */
#অন্তর্ভাব <pthread.h>

/*
** A pointer to an instance of this structure is passed as the user-context
** pointer when registering for an unlock-notify callback.
*/
প্রকারলেখ কাঠামো UnlockNotification UnlockNotification;
কাঠামো UnlockNotification {
  পূর্ণসংখ্যা fired;                         /* True after unlock event has occurred */
  pthread_cond_t cond;               /* Condition variable to wait on */
  pthread_mutex_t mutex;             /* Mutex to protect structure */
};

/*
** This function is an unlock-notify callback registered with SQLite.
*/
স্থির শূন্য unlock_notify_cb(শূন্য **apArg, পূর্ণসংখ্যা nArg){
  পূর্ণসংখ্যা i;
  জন্য(i=0; i<nArg; i++){
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
স্থির পূর্ণসংখ্যা wait_for_unlock_notify(sqlite3 *db){
  পূর্ণসংখ্যা rc;
  UnlockNotification un;

  /* Initialize the UnlockNotification structure. */
  un.fired = 0;
  pthread_mutex_init(&un.mutex, 0);
  pthread_cond_init(&un.cond, 0);

  /* Register for an unlock-notify callback. */
  rc = sqlite3_unlock_notify(db, unlock_notify_cb, (শূন্য *)&un);
  প্রতিজ্ঞা( rc==SQLITE_LOCKED || rc==SQLITE_OK );

  /* The call to sqlite3_unlock_notify() always returns either SQLITE_LOCKED 
  ** or SQLITE_OK. 
  **
  ** If SQLITE_LOCKED was returned, then the system is deadlocked. In this
  ** case this function needs to return SQLITE_LOCKED to the caller so 
  ** that the current transaction can be rolled back. Otherwise, block
  ** until the unlock-notify callback is invoked, then return SQLITE_OK.
  */
  যদি( rc==SQLITE_OK ){
    pthread_mutex_lock(&un.mutex);
    যদি( !un.fired ){
      pthread_cond_wait(&un.cond, &un.mutex);
    }
    pthread_mutex_unlock(&un.mutex);
  }

  /* Destroy the mutex and condition variables. */
  pthread_cond_destroy(&un.cond);
  pthread_mutex_destroy(&un.mutex);

  ফেরত rc;
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
পূর্ণসংখ্যা sqlite3_blocking_step(sqlite3_stmt *pStmt){
  পূর্ণসংখ্যা rc;
  যখন( SQLITE_LOCKED==(rc = sqlite3_step(pStmt)) ){
    rc = wait_for_unlock_notify(sqlite3_db_handle(pStmt));
    যদি( rc!=SQLITE_OK ) বিরতি;
    sqlite3_reset(pStmt);
  }
  ফেরত rc;
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
পূর্ণসংখ্যা sqlite3_blocking_prepare_v2(
  sqlite3 *db,              /* Database handle. */
  ধ্রুবক অক্ষর *zSql,         /* UTF-8 encoded SQL statement. */
  পূর্ণসংখ্যা nSql,                 /* Length of zSql in bytes. */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  ধ্রুবক অক্ষর **pz           /* OUT: End of parsed string */
){
  পূর্ণসংখ্যা rc;
  যখন( SQLITE_LOCKED==(rc = sqlite3_prepare_v2(db, zSql, nSql, ppStmt, pz)) ){
    rc = wait_for_unlock_notify(db);
    যদি( rc!=SQLITE_OK ) বিরতি;
  }
  ফেরত rc;
}
/* END_SQLITE_BLOCKING_STEP */

/*
** Usage: sqlite3_blocking_step STMT
**
** Advance the statement to the next row.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI blocking_step_proc(
  শূন্য * client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){

  sqlite3_stmt *pStmt;
  পূর্ণসংখ্যা rc;

  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "STMT");
    ফেরত TCL_ERROR;
  }

  pStmt = (sqlite3_stmt*)sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  rc = sqlite3_blocking_step(pStmt);

  Tcl_Set_ফলাফল(interp, (অক্ষর *)sqlite3ErrName(rc), 0);
  ফেরত TCL_OK;
}

/*
** Usage: sqlite3_blocking_prepare_v2 DB sql bytes ?tailvar?
** Usage: sqlite3_nonblocking_prepare_v2 DB sql bytes ?tailvar?
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI blocking_prepare_v2_proc(
  শূন্য * client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  ধ্রুবক অক্ষর *zSql;
  পূর্ণসংখ্যা bytes;
  ধ্রুবক অক্ষর *z_লেজ = 0;
  sqlite3_stmt *pStmt = 0;
  অক্ষর zBuf[50];
  পূর্ণসংখ্যা rc;
  পূর্ণসংখ্যা isBlocking = !(client_তথ্য==0);

  যদি( objc!=5 && objc!=4 ){
    Tcl_Append_ফলাফল(interp, "wrong # args: should be \"", 
       Tcl_GetString(objv[0]), " DB sql bytes tailvar", 0);
    ফেরত TCL_ERROR;
  }
  যদি( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ফেরত TCL_ERROR;
  zSql = Tcl_GetString(objv[2]);
  যদি( Tcl_GetIntFromObj(interp, objv[3], &bytes) ) ফেরত TCL_ERROR;

  যদি( isBlocking ){
    rc = sqlite3_blocking_prepare_v2(db, zSql, bytes, &pStmt, &z_লেজ);
  }অন্যথা{
    rc = sqlite3_prepare_v2(db, zSql, bytes, &pStmt, &z_লেজ);
  }

  প্রতিজ্ঞা(rc==SQLITE_OK || pStmt==0);
  যদি( z_লেজ && objc>=5 ){
    যদি( bytes>=0 ){
      bytes = bytes - (z_লেজ-zSql);
    }
    Tcl_ObjSetVar2(interp, objv[4], 0, Tcl_NewStringObj(z_লেজ, bytes), 0);
  }
  যদি( rc!=SQLITE_OK ){
    প্রতিজ্ঞা( pStmt==0 );
    sqlite3_snprintf(আকার(zBuf), zBuf, "%s ", (অক্ষর *)sqlite3ErrName(rc));
    Tcl_Append_ফলাফল(interp, zBuf, sqlite3_errmsg(db), ফাঁকা);
    ফেরত TCL_ERROR;
  }

  যদি( pStmt ){
    যদি( sqlite3TestMakePointerStr(interp, zBuf, pStmt) ) ফেরত TCL_ERROR;
    Tcl_Append_ফলাফল(interp, zBuf, ফাঁকা);
  }
  ফেরত TCL_OK;
}

#endif /* SQLITE_OS_UNIX && SQLITE_ENABLE_UNLOCK_NOTIFY */
/*
** End of implementation of [sqlite3_blocking_step].
************************************************************************/

/*
** Register commands with the TCL interpreter.
*/
পূর্ণসংখ্যা SqlitetestThread_Init(Tcl_Interp *interp){
  কাঠামো TclCmd {
    পূর্ণসংখ্যা (*xProc)(শূন্য*, Tcl_Interp*, পূর্ণসংখ্যা, Tcl_Obj*ধ্রুবক*);
    ধ্রুবক অক্ষর *zName;
    পূর্ণসংখ্যা iCtx;
  } aCmd[] = {
    { sqlthread_proc,           "sqlthread",                      0 },
    { clock_seconds_proc,       "clock_second",                   0 },
    { clock_milliseconds_proc,  "clock_milliseconds",             0 },
#যদি SQLITE_OS_UNIX && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)
    { blocking_step_proc,       "sqlite3_blocking_step",          0 },
    { blocking_prepare_v2_proc, "sqlite3_blocking_prepare_v2",    1 },
    { blocking_prepare_v2_proc, "sqlite3_nonblocking_prepare_v2", 0 },
#endif
  };
  পূর্ণসংখ্যা ii;

  জন্য(ii=0; ii<আকার(aCmd)/আকার(aCmd[0]); ii++){
    শূন্য *p = SQLITE_INT_TO_PTR(aCmd[ii].iCtx);
    Tcl_CreateObjCommand(interp, aCmd[ii].zName, aCmd[ii].xProc, p, 0);
  }
  ফেরত TCL_OK;
}
#অন্যথা
পূর্ণসংখ্যা SqlitetestThread_Init(Tcl_Interp *interp){
  ফেরত TCL_OK;
}
#endif
