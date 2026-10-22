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

#అన్తర్భావ "sqliteInt.h"
#అన్తర్భావ "tclsqlite.h"

#ఒకవేళ SQLITE_THREADSAFE

#అన్తర్భావ <errno.h>

#ఒకవేళ !defined(_MSC_VER)
#అన్తర్భావ <unistd.h>
#అయితే_ముగింపు

/*
** One of these is allocated for each thread created by [sqlthread spawn].
*/
రకనిర్వచనం నిర్మాణం SqlThread SqlThread;
నిర్మాణం SqlThread {
  Tcl_ThreadId మాతృ;     /* Thread id of parent thread */
  Tcl_Interp *interp;      /* Parent interpreter */
  అక్షరం *zScript;           /* The script to execute. */
  అక్షరం *zVarname;          /* Varname in parent script */
};

/*
** A custom Tcl_Event type used by this module. When the event is
** handled, script zScript is evaluated in interpreter interp. If
** the evaluation throws an exception (returns TCL_ERROR), then the
** error is handled by Tcl_BackgroundError(). If no error occurs,
** the result is simply discarded.
*/
రకనిర్వచనం నిర్మాణం EvalEvent EvalEvent;
నిర్మాణం EvalEvent {
  Tcl_Event పునాది;          /* Base class of type Tcl_Event */
  అక్షరం *zScript;           /* The script to execute. */
  Tcl_Interp *interp;      /* The interpreter to execute it in. */
};

స్థిర Tcl_ObjCmdProc sqlthread_proc;
స్థిర Tcl_ObjCmdProc clock_seconds_proc;
#ఒకవేళ SQLITE_OS_UNIX && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)
స్థిర Tcl_ObjCmdProc blocking_అడుగు_proc;
స్థిర Tcl_ObjCmdProc blocking_prepare_v2_proc;
#అయితే_ముగింపు
పూర్ణాంకం Sqlitetest1_Init(Tcl_Interp *);
పూర్ణాంకం Sqlite3_Init(Tcl_Interp *);

/* Functions from main.c */
బాహ్య స్థిరరాశి అక్షరం *sqlite3ErrName(పూర్ణాంకం);

/* Functions from test1.c */
బాహ్య శూన్య *sqlite3_Test_పాఠ్యం_గమ్యం_Ptr(స్థిరరాశి అక్షరం *);
బాహ్య పూర్ణాంకం get_Db_సూచిక(Tcl_Interp *, స్థిరరాశి అక్షరం *, sqlite3 **);
బాహ్య పూర్ణాంకం sqlite3_Test_Make_సూచిక_Str(Tcl_Interp *, అక్షరం *, శూన్య *);
బాహ్య పూర్ణాంకం sqlite3_Test_Err_సంకేతం(Tcl_Interp *, sqlite3 *, పూర్ణాంకం);

/*
** Handler for events of type EvalEvent.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI tclScriptEvent(Tcl_Event *evPtr, పూర్ణాంకం flags){
  పూర్ణాంకం rc;
  EvalEvent *p = (EvalEvent *)evPtr;
  rc = Tcl_Eval(p->interp, p->zScript);
  ఒకవేళ( rc!=TCL_OK ){
    Tcl_BackgroundError(p->interp);
  }
  వాడని_PARAMETER(flags);
  తిరిగి 1;
}

/*
** Register an EvalEvent to evaluate the script pScript in the
** parent interpreter/thread of SqlThread p.
*/
స్థిర శూన్య post_గమ్యం_మాతృ(SqlThread *p, Tcl_Obj *pScript){
  EvalEvent *pEvent;
  అక్షరం *z_సందేశం;
  Tcl_కొలత n_సందేశం;

  z_సందేశం = Tcl_GetStringFromObj(pScript, &n_సందేశం); 
  pEvent = (EvalEvent *)ckalloc(పరిమాణం(EvalEvent)+n_సందేశం+1);
  pEvent->పునాది.తదుపరి_Ptr = 0;
  pEvent->పునాది.proc = tclScriptEvent;
  pEvent->zScript = (అక్షరం *)&pEvent[1];
  జ్ఞాపకం_నకలు(pEvent->zScript, z_సందేశం, n_సందేశం+1);
  pEvent->interp = p->interp;

  Tcl_Thread_వరుస_Event(p->మాతృ, (Tcl_Event *)pEvent, TCL_వరుస_తోక);
  Tcl_ThreadAlert(p->మాతృ);
}

/*
** The main function for threads created with [sqlthread spawn].
*/
స్థిర Tcl_ThreadCreateType tclScriptThread(Client_దత్తాంశం pSqlThread){
  Tcl_Interp *interp;
  Tcl_Obj *pRes;
  Tcl_Obj *pList;
  పూర్ణాంకం rc;
  SqlThread *p = (SqlThread *)pSqlThread;
  బాహ్య పూర్ణాంకం Sqlitetest_mutex_Init(Tcl_Interp*);

  interp = Tcl_CreateInterp();
  Tcl_CreateObjCommand(interp, "clock_seconds", clock_seconds_proc, 0, 0);
  Tcl_CreateObjCommand(interp, "sqlthread", sqlthread_proc, pSqlThread, 0);
#ఒకవేళ SQLITE_OS_UNIX && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)
  Tcl_CreateObjCommand(interp, "sqlite3_blocking_step", blocking_అడుగు_proc,0,0);
  Tcl_CreateObjCommand(interp, 
      "sqlite3_blocking_prepare_v2", blocking_prepare_v2_proc, (శూన్య *)1, 0);
  Tcl_CreateObjCommand(interp, 
      "sqlite3_nonblocking_prepare_v2", blocking_prepare_v2_proc, 0, 0);
#అయితే_ముగింపు
  Sqlitetest1_Init(interp);
  Sqlitetest_mutex_Init(interp);
  Sqlite3_Init(interp);

  rc = Tcl_Eval(interp, p->zScript);
  pRes = Tcl_Get_Obj_ఫలితం(interp);
  pList = Tcl_NewObj();
  Tcl_Incr_Ref_లెక్క(pList);
  Tcl_Incr_Ref_లెక్క(pRes);

  ఒకవేళ( rc!=TCL_OK ){
    Tcl_ListObjAppendElement(interp, pList, Tcl_NewStringObj("error", -1));
    Tcl_ListObjAppendElement(interp, pList, pRes);
    post_గమ్యం_మాతృ(p, pList);
    Tcl_Decr_Ref_లెక్క(pList);
    pList = Tcl_NewObj();
  }

  Tcl_ListObjAppendElement(interp, pList, Tcl_NewStringObj("set", -1));
  Tcl_ListObjAppendElement(interp, pList, Tcl_NewStringObj(p->zVarname, -1));
  Tcl_ListObjAppendElement(interp, pList, pRes);
  post_గమ్యం_మాతృ(p, pList);

  ckfree((శూన్య *)p);
  Tcl_Decr_Ref_లెక్క(pList);
  Tcl_Decr_Ref_లెక్క(pRes);
  Tcl_DeleteInterp(interp);
  అయితే( Tcl_DoOneEvent(TCL_ALL_EVENTS|TCL_DONT_WAIT) );
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
స్థిర పూర్ణాంకం SQLITE_TCLAPI sqlthread_spawn(
  Client_దత్తాంశం client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_ThreadId x;
  SqlThread *pNew;
  పూర్ణాంకం rc;

  Tcl_కొలత nVarname; అక్షరం *zVarname;
  Tcl_కొలత nScript; అక్షరం *zScript;

  /* Parameters for thread creation */
  స్థిరరాశి పూర్ణాంకం n_దొంతర = TCL_THREAD_దొంతర_DEFAULT;
  స్థిరరాశి పూర్ణాంకం flags = TCL_THREAD_NOFLAGS;

  ప్రతిజ్ఞా(objc==4);
  వాడని_PARAMETER(client_దత్తాంశం);
  వాడని_PARAMETER(objc);

  zVarname = Tcl_GetStringFromObj(objv[2], &nVarname);
  zScript = Tcl_GetStringFromObj(objv[3], &nScript);

  pNew = (SqlThread *)ckalloc(పరిమాణం(SqlThread)+nVarname+nScript+2);
  pNew->zVarname = (అక్షరం *)&pNew[1];
  pNew->zScript = (అక్షరం *)&pNew->zVarname[nVarname+1];
  జ్ఞాపకం_నకలు(pNew->zVarname, zVarname, nVarname+1);
  జ్ఞాపకం_నకలు(pNew->zScript, zScript, nScript+1);
  pNew->మాతృ = Tcl_GetCurrentThread();
  pNew->interp = interp;

  rc = Tcl_CreateThread(&x, tclScriptThread, (శూన్య *)pNew, n_దొంతర, flags);
  ఒకవేళ( rc!=TCL_OK ){
    Tcl_Append_ఫలితం(interp, "Error in Tcl_CreateThread()", ఖాళీ);
    ckfree((అక్షరం *)pNew);
    తిరిగి TCL_ERROR;
  }

  తిరిగి TCL_OK;
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
స్థిర పూర్ణాంకం SQLITE_TCLAPI sqlthread_మాతృ(
  Client_దత్తాంశం client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  EvalEvent *pEvent;
  అక్షరం *z_సందేశం;
  Tcl_కొలత n_సందేశం;
  SqlThread *p = (SqlThread *)client_దత్తాంశం;

  ప్రతిజ్ఞా(objc==3);
  వాడని_PARAMETER(objc);

  ఒకవేళ( p==0 ){
    Tcl_Append_ఫలితం(interp, "no parent thread", ఖాళీ);
    తిరిగి TCL_ERROR;
  }

  z_సందేశం = Tcl_GetStringFromObj(objv[2], &n_సందేశం);
  pEvent = (EvalEvent *)ckalloc(పరిమాణం(EvalEvent)+n_సందేశం+1);
  pEvent->పునాది.తదుపరి_Ptr = 0;
  pEvent->పునాది.proc = tclScriptEvent;
  pEvent->zScript = (అక్షరం *)&pEvent[1];
  జ్ఞాపకం_నకలు(pEvent->zScript, z_సందేశం, n_సందేశం+1);
  pEvent->interp = p->interp;
  Tcl_Thread_వరుస_Event(p->మాతృ, (Tcl_Event *)pEvent, TCL_వరుస_తోక);
  Tcl_ThreadAlert(p->మాతృ);

  తిరిగి TCL_OK;
}

స్థిర పూర్ణాంకం xBusy(శూన్య *pArg, పూర్ణాంకం nBusy){
  వాడని_PARAMETER(pArg);
  వాడని_PARAMETER(nBusy);
  sqlite3_sleep(50);
  తిరిగి 1;             /* Try again... */
}

/*
** sqlthread open
**
**     Open a database handle and return the string representation of
**     the pointer value.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI sqlthread_open(
  Client_దత్తాంశం client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  పూర్ణాంకం sqlite3_Test_Make_సూచిక_Str(Tcl_Interp *interp, అక్షరం *zPtr, శూన్య *p);

  స్థిరరాశి అక్షరం *zFilename;
  sqlite3 *db;
  అక్షరం zBuf[100];
  బాహ్య పూర్ణాంకం Md5_రిజిస్టరు(sqlite3*,అక్షరం**,స్థిరరాశి sqlite3_api_routines*);

  వాడని_PARAMETER(client_దత్తాంశం);
  వాడని_PARAMETER(objc);

  zFilename = Tcl_GetString(objv[2]);
  sqlite3_open(zFilename, &db);
  Md5_రిజిస్టరు(db, 0, 0);
  sqlite3_busy_handler(db, xBusy, 0);
  
  ఒకవేళ( sqlite3_Test_Make_సూచిక_Str(interp, zBuf, db) ) తిరిగి TCL_ERROR;
  Tcl_Append_ఫలితం(interp, zBuf, ఖాళీ);

  తిరిగి TCL_OK;
}


/*
** sqlthread open
**
**     Return the current thread-id (Tcl_GetCurrentThread()) cast to
**     an integer.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI sqlthread_id(
  Client_దత్తాంశం client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_ThreadId id = Tcl_GetCurrentThread();
  Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewIntObj(SQLITE_PTR_గమ్యం_INT(id)));
  వాడని_PARAMETER(client_దత్తాంశం);
  వాడని_PARAMETER(objc);
  వాడని_PARAMETER(objv);
  తిరిగి TCL_OK;
}


/*
** Dispatch routine for the sub-commands of [sqlthread].
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI sqlthread_proc(
  Client_దత్తాంశం client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  నిర్మాణం SubCommand {
    అక్షరం *zName;
    Tcl_ObjCmdProc *xProc;
    పూర్ణాంకం nArg;
    అక్షరం *zUsage;
  } aSub[] = {
    {"parent", sqlthread_మాతృ, 1, "SCRIPT"},
    {"spawn",  sqlthread_spawn,  2, "VARNAME SCRIPT"},
    {"open",   sqlthread_open,   1, "DBNAME"},
    {"id",     sqlthread_id,     0, ""},
    {0, 0, 0}
  };
  నిర్మాణం SubCommand *pSub;
  పూర్ణాంకం rc;
  పూర్ణాంకం i_సూచీ;

  ఒకవేళ( objc<2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "SUB-COMMAND");
    తిరిగి TCL_ERROR;
  }

  rc = Tcl_Get_సూచీ_From_Obj_Struct(
      interp, objv[1], aSub, పరిమాణం(aSub[0]), "sub-command", 0, &i_సూచీ
  );
  ఒకవేళ( rc!=TCL_OK ) తిరిగి rc;
  pSub = &aSub[i_సూచీ];

  ఒకవేళ( objc<(pSub->nArg+2) ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 2, objv, pSub->zUsage);
    తిరిగి TCL_ERROR;
  }

  తిరిగి pSub->xProc(client_దత్తాంశం, interp, objc, objv);
}

/*
** The [clock_seconds] command. This is more or less the same as the
** regular tcl [clock seconds], except that it is available in testfixture
** when linked against both Tcl 8.4 and 8.5. Because [clock seconds] is
** implemented as a script in Tcl 8.5, it is not usually available to
** testfixture.
*/ 
స్థిర పూర్ణాంకం SQLITE_TCLAPI clock_seconds_proc(
  Client_దత్తాంశం client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_సమయం now;
  Tcl_Get_సమయం(&now);
  Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewIntObj(now.sec));
  వాడని_PARAMETER(client_దత్తాంశం);
  వాడని_PARAMETER(objc);
  వాడని_PARAMETER(objv);
  తిరిగి TCL_OK;
}

/*
** The [clock_milliseconds] command. This is more or less the same as the
** regular tcl [clock milliseconds]. 
*/ 
స్థిర పూర్ణాంకం SQLITE_TCLAPI clock_milliseconds_proc(
  Client_దత్తాంశం client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_సమయం now;
  Tcl_Get_సమయం(&now);
  Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewWideIntObj(
    ((Tcl_WideInt)now.sec * 1000) + (now.usec / 1000)
  ));
  వాడని_PARAMETER(client_దత్తాంశం);
  వాడని_PARAMETER(objc);
  వాడని_PARAMETER(objv);
  తిరిగి TCL_OK;
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
#ఒకవేళ SQLITE_OS_UNIX && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)

/* BEGIN_SQLITE_BLOCKING_STEP */
/* This example uses the pthreads API */
#అన్తర్భావ <pthread.h>

/*
** A pointer to an instance of this structure is passed as the user-context
** pointer when registering for an unlock-notify callback.
*/
రకనిర్వచనం నిర్మాణం UnlockNotification UnlockNotification;
నిర్మాణం UnlockNotification {
  పూర్ణాంకం fired;                         /* True after unlock event has occurred */
  pthread_cond_t cond;               /* Condition variable to wait on */
  pthread_mutex_t mutex;             /* Mutex to protect structure */
};

/*
** This function is an unlock-notify callback registered with SQLite.
*/
స్థిర శూన్య unlock_notify_cb(శూన్య **apArg, పూర్ణాంకం nArg){
  పూర్ణాంకం i;
  కొరకు(i=0; i<nArg; i++){
    UnlockNotification *p = (UnlockNotification *)apArg[i];
    pthread_mutex_తాళం(&p->mutex);
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
స్థిర పూర్ణాంకం wait_for_unlock_notify(sqlite3 *db){
  పూర్ణాంకం rc;
  UnlockNotification un;

  /* Initialize the UnlockNotification structure. */
  un.fired = 0;
  pthread_mutex_init(&un.mutex, 0);
  pthread_cond_init(&un.cond, 0);

  /* Register for an unlock-notify callback. */
  rc = sqlite3_unlock_notify(db, unlock_notify_cb, (శూన్య *)&un);
  ప్రతిజ్ఞా( rc==SQLITE_LOCKED || rc==SQLITE_OK );

  /* The call to sqlite3_unlock_notify() always returns either SQLITE_LOCKED 
  ** or SQLITE_OK. 
  **
  ** If SQLITE_LOCKED was returned, then the system is deadlocked. In this
  ** case this function needs to return SQLITE_LOCKED to the caller so 
  ** that the current transaction can be rolled back. Otherwise, block
  ** until the unlock-notify callback is invoked, then return SQLITE_OK.
  */
  ఒకవేళ( rc==SQLITE_OK ){
    pthread_mutex_తాళం(&un.mutex);
    ఒకవేళ( !un.fired ){
      pthread_cond_wait(&un.cond, &un.mutex);
    }
    pthread_mutex_unlock(&un.mutex);
  }

  /* Destroy the mutex and condition variables. */
  pthread_cond_destroy(&un.cond);
  pthread_mutex_destroy(&un.mutex);

  తిరిగి rc;
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
పూర్ణాంకం sqlite3_blocking_అడుగు(sqlite3_stmt *pStmt){
  పూర్ణాంకం rc;
  అయితే( SQLITE_LOCKED==(rc = sqlite3_అడుగు(pStmt)) ){
    rc = wait_for_unlock_notify(sqlite3_db_handle(pStmt));
    ఒకవేళ( rc!=SQLITE_OK ) ఆపు;
    sqlite3_reset(pStmt);
  }
  తిరిగి rc;
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
పూర్ణాంకం sqlite3_blocking_prepare_v2(
  sqlite3 *db,              /* Database handle. */
  స్థిరరాశి అక్షరం *zSql,         /* UTF-8 encoded SQL statement. */
  పూర్ణాంకం nSql,                 /* Length of zSql in bytes. */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  స్థిరరాశి అక్షరం **pz           /* OUT: End of parsed string */
){
  పూర్ణాంకం rc;
  అయితే( SQLITE_LOCKED==(rc = sqlite3_prepare_v2(db, zSql, nSql, ppStmt, pz)) ){
    rc = wait_for_unlock_notify(db);
    ఒకవేళ( rc!=SQLITE_OK ) ఆపు;
  }
  తిరిగి rc;
}
/* END_SQLITE_BLOCKING_STEP */

/*
** Usage: sqlite3_blocking_step STMT
**
** Advance the statement to the next row.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI blocking_అడుగు_proc(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){

  sqlite3_stmt *pStmt;
  పూర్ణాంకం rc;

  ఒకవేళ( objc!=2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "STMT");
    తిరిగి TCL_ERROR;
  }

  pStmt = (sqlite3_stmt*)sqlite3_Test_పాఠ్యం_గమ్యం_Ptr(Tcl_GetString(objv[1]));
  rc = sqlite3_blocking_అడుగు(pStmt);

  Tcl_సమితి_ఫలితం(interp, (అక్షరం *)sqlite3ErrName(rc), 0);
  తిరిగి TCL_OK;
}

/*
** Usage: sqlite3_blocking_prepare_v2 DB sql bytes ?tailvar?
** Usage: sqlite3_nonblocking_prepare_v2 DB sql bytes ?tailvar?
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI blocking_prepare_v2_proc(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  స్థిరరాశి అక్షరం *zSql;
  పూర్ణాంకం బైట్లు;
  స్థిరరాశి అక్షరం *z_తోక = 0;
  sqlite3_stmt *pStmt = 0;
  అక్షరం zBuf[50];
  పూర్ణాంకం rc;
  పూర్ణాంకం isBlocking = !(client_దత్తాంశం==0);

  ఒకవేళ( objc!=5 && objc!=4 ){
    Tcl_Append_ఫలితం(interp, "wrong # args: should be \"", 
       Tcl_GetString(objv[0]), " DB sql bytes tailvar", 0);
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( get_Db_సూచిక(interp, Tcl_GetString(objv[1]), &db) ) తిరిగి TCL_ERROR;
  zSql = Tcl_GetString(objv[2]);
  ఒకవేళ( Tcl_GetIntFromObj(interp, objv[3], &బైట్లు) ) తిరిగి TCL_ERROR;

  ఒకవేళ( isBlocking ){
    rc = sqlite3_blocking_prepare_v2(db, zSql, బైట్లు, &pStmt, &z_తోక);
  }లేకపోతే{
    rc = sqlite3_prepare_v2(db, zSql, బైట్లు, &pStmt, &z_తోక);
  }

  ప్రతిజ్ఞా(rc==SQLITE_OK || pStmt==0);
  ఒకవేళ( z_తోక && objc>=5 ){
    ఒకవేళ( బైట్లు>=0 ){
      బైట్లు = బైట్లు - (z_తోక-zSql);
    }
    Tcl_Obj_సమితి_Var2(interp, objv[4], 0, Tcl_NewStringObj(z_తోక, బైట్లు), 0);
  }
  ఒకవేళ( rc!=SQLITE_OK ){
    ప్రతిజ్ఞా( pStmt==0 );
    sqlite3_snprintf(పరిమాణం(zBuf), zBuf, "%s ", (అక్షరం *)sqlite3ErrName(rc));
    Tcl_Append_ఫలితం(interp, zBuf, sqlite3_errmsg(db), ఖాళీ);
    తిరిగి TCL_ERROR;
  }

  ఒకవేళ( pStmt ){
    ఒకవేళ( sqlite3_Test_Make_సూచిక_Str(interp, zBuf, pStmt) ) తిరిగి TCL_ERROR;
    Tcl_Append_ఫలితం(interp, zBuf, ఖాళీ);
  }
  తిరిగి TCL_OK;
}

#అయితే_ముగింపు /* SQLITE_OS_UNIX && SQLITE_ENABLE_UNLOCK_NOTIFY */
/*
** End of implementation of [sqlite3_blocking_step].
************************************************************************/

/*
** Register commands with the TCL interpreter.
*/
పూర్ణాంకం SqlitetestThread_Init(Tcl_Interp *interp){
  నిర్మాణం TclCmd {
    పూర్ణాంకం (*xProc)(శూన్య*, Tcl_Interp*, పూర్ణాంకం, Tcl_Obj*స్థిరరాశి*);
    స్థిరరాశి అక్షరం *zName;
    పూర్ణాంకం iCtx;
  } aCmd[] = {
    { sqlthread_proc,           "sqlthread",                      0 },
    { clock_seconds_proc,       "clock_second",                   0 },
    { clock_milliseconds_proc,  "clock_milliseconds",             0 },
#ఒకవేళ SQLITE_OS_UNIX && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)
    { blocking_అడుగు_proc,       "sqlite3_blocking_step",          0 },
    { blocking_prepare_v2_proc, "sqlite3_blocking_prepare_v2",    1 },
    { blocking_prepare_v2_proc, "sqlite3_nonblocking_prepare_v2", 0 },
#అయితే_ముగింపు
  };
  పూర్ణాంకం ii;

  కొరకు(ii=0; ii<పరిమాణం(aCmd)/పరిమాణం(aCmd[0]); ii++){
    శూన్య *p = SQLITE_INT_గమ్యం_PTR(aCmd[ii].iCtx);
    Tcl_CreateObjCommand(interp, aCmd[ii].zName, aCmd[ii].xProc, p, 0);
  }
  తిరిగి TCL_OK;
}
#లేకపోతే
పూర్ణాంకం SqlitetestThread_Init(Tcl_Interp *interp){
  తిరిగి TCL_OK;
}
#అయితే_ముగింపు
