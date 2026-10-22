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

#ଅନ୍ତର୍ଭାଵ "sqliteInt.h"
#ଅନ୍ତର୍ଭାଵ "tclsqlite.h"

#ଯଦି SQLITE_THREADSAFE

#ଅନ୍ତର୍ଭାଵ <errno.h>

#ଯଦି !defined(_MSC_VER)
#ଅନ୍ତର୍ଭାଵ <unistd.h>
#endif

/*
** One of these is allocated for each thread created by [sqlthread spawn].
*/
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା SqlThread SqlThread;
ସଂରଚନା SqlThread {
  Tcl_ThreadId parent;     /* Thread id of parent thread */
  Tcl_Interp *interp;      /* Parent interpreter */
  ଅକ୍ଷର *zScript;           /* The script to execute. */
  ଅକ୍ଷର *zVarname;          /* Varname in parent script */
};

/*
** A custom Tcl_Event type used by this module. When the event is
** handled, script zScript is evaluated in interpreter interp. If
** the evaluation throws an exception (returns TCL_ERROR), then the
** error is handled by Tcl_BackgroundError(). If no error occurs,
** the result is simply discarded.
*/
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା EvalEvent EvalEvent;
ସଂରଚନା EvalEvent {
  Tcl_Event base;          /* Base class of type Tcl_Event */
  ଅକ୍ଷର *zScript;           /* The script to execute. */
  Tcl_Interp *interp;      /* The interpreter to execute it in. */
};

ସ୍ଥିର Tcl_ObjCmdProc sqlthread_proc;
ସ୍ଥିର Tcl_ObjCmdProc clock_seconds_proc;
#ଯଦି SQLITE_OS_UNIX && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)
ସ୍ଥିର Tcl_ObjCmdProc blocking_step_proc;
ସ୍ଥିର Tcl_ObjCmdProc blocking_prepare_v2_proc;
#endif
ପୂର୍ଣ୍ଣସଂଖ୍ୟା Sqlitetest1_Init(Tcl_Interp *);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା Sqlite3_Init(Tcl_Interp *);

/* Functions from main.c */
ବାହ୍ୟ ଅଚଳ ଅକ୍ଷର *sqlite3ErrName(ପୂର୍ଣ୍ଣସଂଖ୍ୟା);

/* Functions from test1.c */
ବାହ୍ୟ ଶୂନ୍ୟ *sqlite3TestTextToPtr(ଅଚଳ ଅକ୍ଷର *);
ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା getDbPointer(Tcl_Interp *, ଅଚଳ ଅକ୍ଷର *, sqlite3 **);
ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3TestMakePointerStr(Tcl_Interp *, ଅକ୍ଷର *, ଶୂନ୍ୟ *);
ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3TestErrCode(Tcl_Interp *, sqlite3 *, ପୂର୍ଣ୍ଣସଂଖ୍ୟା);

/*
** Handler for events of type EvalEvent.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI tclScriptEvent(Tcl_Event *evPtr, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  EvalEvent *p = (EvalEvent *)evPtr;
  rc = Tcl_Eval(p->interp, p->zScript);
  ଯଦି( rc!=TCL_OK ){
    Tcl_BackgroundError(p->interp);
  }
  UNUSED_PARAMETER(flags);
  ଫେରନ୍ତୁ 1;
}

/*
** Register an EvalEvent to evaluate the script pScript in the
** parent interpreter/thread of SqlThread p.
*/
ସ୍ଥିର ଶୂନ୍ୟ postToParent(SqlThread *p, Tcl_Obj *pScript){
  EvalEvent *pEvent;
  ଅକ୍ଷର *z_ସନ୍ଦେଶ;
  Tcl_Size n_ସନ୍ଦେଶ;

  z_ସନ୍ଦେଶ = Tcl_GetStringFromObj(pScript, &n_ସନ୍ଦେଶ); 
  pEvent = (EvalEvent *)ckalloc(ଆକାର(EvalEvent)+n_ସନ୍ଦେଶ+1);
  pEvent->base.nextPtr = 0;
  pEvent->base.proc = tclScriptEvent;
  pEvent->zScript = (ଅକ୍ଷର *)&pEvent[1];
  memcpy(pEvent->zScript, z_ସନ୍ଦେଶ, n_ସନ୍ଦେଶ+1);
  pEvent->interp = p->interp;

  Tcl_ThreadQueueEvent(p->parent, (Tcl_Event *)pEvent, TCL_QUEUE_TAIL);
  Tcl_ThreadAlert(p->parent);
}

/*
** The main function for threads created with [sqlthread spawn].
*/
ସ୍ଥିର Tcl_ThreadCreateType tclScriptThread(ClientData pSqlThread){
  Tcl_Interp *interp;
  Tcl_Obj *pRes;
  Tcl_Obj *pList;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  SqlThread *p = (SqlThread *)pSqlThread;
  ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା Sqlitetest_mutex_Init(Tcl_Interp*);

  interp = Tcl_CreateInterp();
  Tcl_CreateObjCommand(interp, "clock_seconds", clock_seconds_proc, 0, 0);
  Tcl_CreateObjCommand(interp, "sqlthread", sqlthread_proc, pSqlThread, 0);
#ଯଦି SQLITE_OS_UNIX && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)
  Tcl_CreateObjCommand(interp, "sqlite3_blocking_step", blocking_step_proc,0,0);
  Tcl_CreateObjCommand(interp, 
      "sqlite3_blocking_prepare_v2", blocking_prepare_v2_proc, (ଶୂନ୍ୟ *)1, 0);
  Tcl_CreateObjCommand(interp, 
      "sqlite3_nonblocking_prepare_v2", blocking_prepare_v2_proc, 0, 0);
#endif
  Sqlitetest1_Init(interp);
  Sqlitetest_mutex_Init(interp);
  Sqlite3_Init(interp);

  rc = Tcl_Eval(interp, p->zScript);
  pRes = Tcl_GetObjResult(interp);
  pList = Tcl_NewObj();
  Tcl_Incr_Ref_ଗଣନା(pList);
  Tcl_Incr_Ref_ଗଣନା(pRes);

  ଯଦି( rc!=TCL_OK ){
    Tcl_ListObjAppendElement(interp, pList, Tcl_NewStringObj("error", -1));
    Tcl_ListObjAppendElement(interp, pList, pRes);
    postToParent(p, pList);
    Tcl_Decr_Ref_ଗଣନା(pList);
    pList = Tcl_NewObj();
  }

  Tcl_ListObjAppendElement(interp, pList, Tcl_NewStringObj("set", -1));
  Tcl_ListObjAppendElement(interp, pList, Tcl_NewStringObj(p->zVarname, -1));
  Tcl_ListObjAppendElement(interp, pList, pRes);
  postToParent(p, pList);

  ckfree((ଶୂନ୍ୟ *)p);
  Tcl_Decr_Ref_ଗଣନା(pList);
  Tcl_Decr_Ref_ଗଣନା(pRes);
  Tcl_DeleteInterp(interp);
  ଯେତେବେଳେ( Tcl_DoOneEvent(TCL_ALL_EVENTS|TCL_DONT_WAIT) );
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI sqlthread_spawn(
  ClientData clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_ThreadId x;
  SqlThread *pNew;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

  Tcl_Size nVarname; ଅକ୍ଷର *zVarname;
  Tcl_Size nScript; ଅକ୍ଷର *zScript;

  /* Parameters for thread creation */
  ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା n_ଗଦା = TCL_THREAD_ଗଦା_DEFAULT;
  ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags = TCL_THREAD_NOFLAGS;

  ପ୍ରତିଜ୍ଞା(objc==4);
  UNUSED_PARAMETER(clientData);
  UNUSED_PARAMETER(objc);

  zVarname = Tcl_GetStringFromObj(objv[2], &nVarname);
  zScript = Tcl_GetStringFromObj(objv[3], &nScript);

  pNew = (SqlThread *)ckalloc(ଆକାର(SqlThread)+nVarname+nScript+2);
  pNew->zVarname = (ଅକ୍ଷର *)&pNew[1];
  pNew->zScript = (ଅକ୍ଷର *)&pNew->zVarname[nVarname+1];
  memcpy(pNew->zVarname, zVarname, nVarname+1);
  memcpy(pNew->zScript, zScript, nScript+1);
  pNew->parent = Tcl_GetCurrentThread();
  pNew->interp = interp;

  rc = Tcl_CreateThread(&x, tclScriptThread, (ଶୂନ୍ୟ *)pNew, n_ଗଦା, flags);
  ଯଦି( rc!=TCL_OK ){
    Tcl_AppendResult(interp, "Error in Tcl_CreateThread()", ଖାଲି);
    ckfree((ଅକ୍ଷର *)pNew);
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  ଫେରନ୍ତୁ TCL_OK;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI sqlthread_parent(
  ClientData clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  EvalEvent *pEvent;
  ଅକ୍ଷର *z_ସନ୍ଦେଶ;
  Tcl_Size n_ସନ୍ଦେଶ;
  SqlThread *p = (SqlThread *)clientData;

  ପ୍ରତିଜ୍ଞା(objc==3);
  UNUSED_PARAMETER(objc);

  ଯଦି( p==0 ){
    Tcl_AppendResult(interp, "no parent thread", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  z_ସନ୍ଦେଶ = Tcl_GetStringFromObj(objv[2], &n_ସନ୍ଦେଶ);
  pEvent = (EvalEvent *)ckalloc(ଆକାର(EvalEvent)+n_ସନ୍ଦେଶ+1);
  pEvent->base.nextPtr = 0;
  pEvent->base.proc = tclScriptEvent;
  pEvent->zScript = (ଅକ୍ଷର *)&pEvent[1];
  memcpy(pEvent->zScript, z_ସନ୍ଦେଶ, n_ସନ୍ଦେଶ+1);
  pEvent->interp = p->interp;
  Tcl_ThreadQueueEvent(p->parent, (Tcl_Event *)pEvent, TCL_QUEUE_TAIL);
  Tcl_ThreadAlert(p->parent);

  ଫେରନ୍ତୁ TCL_OK;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା xBusy(ଶୂନ୍ୟ *pArg, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nBusy){
  UNUSED_PARAMETER(pArg);
  UNUSED_PARAMETER(nBusy);
  sqlite3_sleep(50);
  ଫେରନ୍ତୁ 1;             /* Try again... */
}

/*
** sqlthread open
**
**     Open a database handle and return the string representation of
**     the pointer value.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI sqlthread_open(
  ClientData clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3TestMakePointerStr(Tcl_Interp *interp, ଅକ୍ଷର *zPtr, ଶୂନ୍ୟ *p);

  ଅଚଳ ଅକ୍ଷର *zFilename;
  sqlite3 *db;
  ଅକ୍ଷର zBuf[100];
  ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା Md5_Register(sqlite3*,ଅକ୍ଷର**,ଅଚଳ sqlite3_api_routines*);

  UNUSED_PARAMETER(clientData);
  UNUSED_PARAMETER(objc);

  zFilename = Tcl_GetString(objv[2]);
  sqlite3_open(zFilename, &db);
  Md5_Register(db, 0, 0);
  sqlite3_busy_handler(db, xBusy, 0);
  
  ଯଦି( sqlite3TestMakePointerStr(interp, zBuf, db) ) ଫେରନ୍ତୁ TCL_ERROR;
  Tcl_AppendResult(interp, zBuf, ଖାଲି);

  ଫେରନ୍ତୁ TCL_OK;
}


/*
** sqlthread open
**
**     Return the current thread-id (Tcl_GetCurrentThread()) cast to
**     an integer.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI sqlthread_id(
  ClientData clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_ThreadId id = Tcl_GetCurrentThread();
  Tcl_SetObjResult(interp, Tcl_NewIntObj(SQLITE_PTR_TO_INT(id)));
  UNUSED_PARAMETER(clientData);
  UNUSED_PARAMETER(objc);
  UNUSED_PARAMETER(objv);
  ଫେରନ୍ତୁ TCL_OK;
}


/*
** Dispatch routine for the sub-commands of [sqlthread].
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI sqlthread_proc(
  ClientData clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ସଂରଚନା SubCommand {
    ଅକ୍ଷର *zName;
    Tcl_ObjCmdProc *xProc;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା nArg;
    ଅକ୍ଷର *zUsage;
  } aSub[] = {
    {"parent", sqlthread_parent, 1, "SCRIPT"},
    {"spawn",  sqlthread_spawn,  2, "VARNAME SCRIPT"},
    {"open",   sqlthread_open,   1, "DBNAME"},
    {"id",     sqlthread_id,     0, ""},
    {0, 0, 0}
  };
  ସଂରଚନା SubCommand *pSub;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iIndex;

  ଯଦି( objc<2 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "SUB-COMMAND");
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  rc = Tcl_GetIndexFromObjStruct(
      interp, objv[1], aSub, ଆକାର(aSub[0]), "sub-command", 0, &iIndex
  );
  ଯଦି( rc!=TCL_OK ) ଫେରନ୍ତୁ rc;
  pSub = &aSub[iIndex];

  ଯଦି( objc<(pSub->nArg+2) ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 2, objv, pSub->zUsage);
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  ଫେରନ୍ତୁ pSub->xProc(clientData, interp, objc, objv);
}

/*
** The [clock_seconds] command. This is more or less the same as the
** regular tcl [clock seconds], except that it is available in testfixture
** when linked against both Tcl 8.4 and 8.5. Because [clock seconds] is
** implemented as a script in Tcl 8.5, it is not usually available to
** testfixture.
*/ 
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI clock_seconds_proc(
  ClientData clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Time now;
  Tcl_GetTime(&now);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(now.sec));
  UNUSED_PARAMETER(clientData);
  UNUSED_PARAMETER(objc);
  UNUSED_PARAMETER(objv);
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** The [clock_milliseconds] command. This is more or less the same as the
** regular tcl [clock milliseconds]. 
*/ 
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI clock_milliseconds_proc(
  ClientData clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
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
  ଫେରନ୍ତୁ TCL_OK;
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
#ଯଦି SQLITE_OS_UNIX && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)

/* BEGIN_SQLITE_BLOCKING_STEP */
/* This example uses the pthreads API */
#ଅନ୍ତର୍ଭାଵ <pthread.h>

/*
** A pointer to an instance of this structure is passed as the user-context
** pointer when registering for an unlock-notify callback.
*/
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା UnlockNotification UnlockNotification;
ସଂରଚନା UnlockNotification {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା fired;                         /* True after unlock event has occurred */
  pthread_cond_t cond;               /* Condition variable to wait on */
  pthread_mutex_t mutex;             /* Mutex to protect structure */
};

/*
** This function is an unlock-notify callback registered with SQLite.
*/
ସ୍ଥିର ଶୂନ୍ୟ unlock_notify_cb(ଶୂନ୍ୟ **apArg, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nArg){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପାଇଁ(i=0; i<nArg; i++){
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା wait_for_unlock_notify(sqlite3 *db){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  UnlockNotification un;

  /* Initialize the UnlockNotification structure. */
  un.fired = 0;
  pthread_mutex_init(&un.mutex, 0);
  pthread_cond_init(&un.cond, 0);

  /* Register for an unlock-notify callback. */
  rc = sqlite3_unlock_notify(db, unlock_notify_cb, (ଶୂନ୍ୟ *)&un);
  ପ୍ରତିଜ୍ଞା( rc==SQLITE_LOCKED || rc==SQLITE_OK );

  /* The call to sqlite3_unlock_notify() always returns either SQLITE_LOCKED 
  ** or SQLITE_OK. 
  **
  ** If SQLITE_LOCKED was returned, then the system is deadlocked. In this
  ** case this function needs to return SQLITE_LOCKED to the caller so 
  ** that the current transaction can be rolled back. Otherwise, block
  ** until the unlock-notify callback is invoked, then return SQLITE_OK.
  */
  ଯଦି( rc==SQLITE_OK ){
    pthread_mutex_lock(&un.mutex);
    ଯଦି( !un.fired ){
      pthread_cond_wait(&un.cond, &un.mutex);
    }
    pthread_mutex_unlock(&un.mutex);
  }

  /* Destroy the mutex and condition variables. */
  pthread_cond_destroy(&un.cond);
  pthread_mutex_destroy(&un.mutex);

  ଫେରନ୍ତୁ rc;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_blocking_step(sqlite3_stmt *pStmt){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ଯେତେବେଳେ( SQLITE_LOCKED==(rc = sqlite3_step(pStmt)) ){
    rc = wait_for_unlock_notify(sqlite3_db_handle(pStmt));
    ଯଦି( rc!=SQLITE_OK ) ଭାଙ୍ଗ;
    sqlite3_reset(pStmt);
  }
  ଫେରନ୍ତୁ rc;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_blocking_prepare_v2(
  sqlite3 *db,              /* Database handle. */
  ଅଚଳ ଅକ୍ଷର *zSql,         /* UTF-8 encoded SQL statement. */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nSql,                 /* Length of zSql in bytes. */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  ଅଚଳ ଅକ୍ଷର **pz           /* OUT: End of parsed string */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ଯେତେବେଳେ( SQLITE_LOCKED==(rc = sqlite3_prepare_v2(db, zSql, nSql, ppStmt, pz)) ){
    rc = wait_for_unlock_notify(db);
    ଯଦି( rc!=SQLITE_OK ) ଭାଙ୍ଗ;
  }
  ଫେରନ୍ତୁ rc;
}
/* END_SQLITE_BLOCKING_STEP */

/*
** Usage: sqlite3_blocking_step STMT
**
** Advance the statement to the next row.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI blocking_step_proc(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){

  sqlite3_stmt *pStmt;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

  ଯଦି( objc!=2 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "STMT");
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  pStmt = (sqlite3_stmt*)sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  rc = sqlite3_blocking_step(pStmt);

  Tcl_SetResult(interp, (ଅକ୍ଷର *)sqlite3ErrName(rc), 0);
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** Usage: sqlite3_blocking_prepare_v2 DB sql bytes ?tailvar?
** Usage: sqlite3_nonblocking_prepare_v2 DB sql bytes ?tailvar?
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI blocking_prepare_v2_proc(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  ଅଚଳ ଅକ୍ଷର *zSql;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା bytes;
  ଅଚଳ ଅକ୍ଷର *zTail = 0;
  sqlite3_stmt *pStmt = 0;
  ଅକ୍ଷର zBuf[50];
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା isBlocking = !(clientData==0);

  ଯଦି( objc!=5 && objc!=4 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", 
       Tcl_GetString(objv[0]), " DB sql bytes tailvar", 0);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ଫେରନ୍ତୁ TCL_ERROR;
  zSql = Tcl_GetString(objv[2]);
  ଯଦି( Tcl_GetIntFromObj(interp, objv[3], &bytes) ) ଫେରନ୍ତୁ TCL_ERROR;

  ଯଦି( isBlocking ){
    rc = sqlite3_blocking_prepare_v2(db, zSql, bytes, &pStmt, &zTail);
  }ଅନ୍ୟଥା{
    rc = sqlite3_prepare_v2(db, zSql, bytes, &pStmt, &zTail);
  }

  ପ୍ରତିଜ୍ଞା(rc==SQLITE_OK || pStmt==0);
  ଯଦି( zTail && objc>=5 ){
    ଯଦି( bytes>=0 ){
      bytes = bytes - (zTail-zSql);
    }
    Tcl_ObjSetVar2(interp, objv[4], 0, Tcl_NewStringObj(zTail, bytes), 0);
  }
  ଯଦି( rc!=SQLITE_OK ){
    ପ୍ରତିଜ୍ଞା( pStmt==0 );
    sqlite3_snprintf(ଆକାର(zBuf), zBuf, "%s ", (ଅକ୍ଷର *)sqlite3ErrName(rc));
    Tcl_AppendResult(interp, zBuf, sqlite3_errmsg(db), ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  ଯଦି( pStmt ){
    ଯଦି( sqlite3TestMakePointerStr(interp, zBuf, pStmt) ) ଫେରନ୍ତୁ TCL_ERROR;
    Tcl_AppendResult(interp, zBuf, ଖାଲି);
  }
  ଫେରନ୍ତୁ TCL_OK;
}

#endif /* SQLITE_OS_UNIX && SQLITE_ENABLE_UNLOCK_NOTIFY */
/*
** End of implementation of [sqlite3_blocking_step].
************************************************************************/

/*
** Register commands with the TCL interpreter.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା SqlitetestThread_Init(Tcl_Interp *interp){
  ସଂରଚନା TclCmd {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*xProc)(ଶୂନ୍ୟ*, Tcl_Interp*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା, Tcl_Obj*ଅଚଳ*);
    ଅଚଳ ଅକ୍ଷର *zName;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCtx;
  } aCmd[] = {
    { sqlthread_proc,           "sqlthread",                      0 },
    { clock_seconds_proc,       "clock_second",                   0 },
    { clock_milliseconds_proc,  "clock_milliseconds",             0 },
#ଯଦି SQLITE_OS_UNIX && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)
    { blocking_step_proc,       "sqlite3_blocking_step",          0 },
    { blocking_prepare_v2_proc, "sqlite3_blocking_prepare_v2",    1 },
    { blocking_prepare_v2_proc, "sqlite3_nonblocking_prepare_v2", 0 },
#endif
  };
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ii;

  ପାଇଁ(ii=0; ii<ଆକାର(aCmd)/ଆକାର(aCmd[0]); ii++){
    ଶୂନ୍ୟ *p = SQLITE_INT_TO_PTR(aCmd[ii].iCtx);
    Tcl_CreateObjCommand(interp, aCmd[ii].zName, aCmd[ii].xProc, p, 0);
  }
  ଫେରନ୍ତୁ TCL_OK;
}
#ଅନ୍ୟଥା
ପୂର୍ଣ୍ଣସଂଖ୍ୟା SqlitetestThread_Init(Tcl_Interp *interp){
  ଫେରନ୍ତୁ TCL_OK;
}
#endif
