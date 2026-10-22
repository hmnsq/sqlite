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

#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "tclsqlite.h"

#ਜੇ SQLITE_THREADSAFE

#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>

#ਜੇ !defined(_MSC_VER)
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>
#ਜੇ_ਅੰਤ

/*
** One of these is allocated for each thread created by [sqlthread spawn].
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ SqlThread SqlThread;
ਬਣਤਰ SqlThread {
  Tcl_ThreadId ਮਾਪਾ;     /* Thread id of parent thread */
  Tcl_Interp *interp;      /* Parent interpreter */
  ਅੱਖਰ *zScript;           /* The script to execute. */
  ਅੱਖਰ *zVarname;          /* Varname in parent script */
};

/*
** A custom Tcl_Event type used by this module. When the event is
** handled, script zScript is evaluated in interpreter interp. If
** the evaluation throws an exception (returns TCL_ERROR), then the
** error is handled by Tcl_BackgroundError(). If no error occurs,
** the result is simply discarded.
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ EvalEvent EvalEvent;
ਬਣਤਰ EvalEvent {
  Tcl_Event ਅਧਾਰ;          /* Base class of type Tcl_Event */
  ਅੱਖਰ *zScript;           /* The script to execute. */
  Tcl_Interp *interp;      /* The interpreter to execute it in. */
};

ਸਥਿਰ Tcl_ObjCmdProc sqlthread_proc;
ਸਥਿਰ Tcl_ObjCmdProc clock_seconds_proc;
#ਜੇ SQLITE_OS_UNIX && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)
ਸਥਿਰ Tcl_ObjCmdProc blocking_ਕਦਮ_proc;
ਸਥਿਰ Tcl_ObjCmdProc blocking_prepare_v2_proc;
#ਜੇ_ਅੰਤ
ਪੂਰਨ Sqlitetest1_Init(Tcl_Interp *);
ਪੂਰਨ Sqlite3_Init(Tcl_Interp *);

/* Functions from main.c */
ਬਾਹਰੀ ਅਚਲ ਅੱਖਰ *sqlite3ErrName(ਪੂਰਨ);

/* Functions from test1.c */
ਬਾਹਰੀ ਖਾਲੀ *sqlite3_Test_ਪਾਠ_ਮੰਜ਼ਿਲ_Ptr(ਅਚਲ ਅੱਖਰ *);
ਬਾਹਰੀ ਪੂਰਨ get_Db_ਸੂਚਕ(Tcl_Interp *, ਅਚਲ ਅੱਖਰ *, sqlite3 **);
ਬਾਹਰੀ ਪੂਰਨ sqlite3_Test_Make_ਸੂਚਕ_Str(Tcl_Interp *, ਅੱਖਰ *, ਖਾਲੀ *);
ਬਾਹਰੀ ਪੂਰਨ sqlite3_Test_Err_ਕੋਡ(Tcl_Interp *, sqlite3 *, ਪੂਰਨ);

/*
** Handler for events of type EvalEvent.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI tclScriptEvent(Tcl_Event *evPtr, ਪੂਰਨ flags){
  ਪੂਰਨ rc;
  EvalEvent *p = (EvalEvent *)evPtr;
  rc = Tcl_Eval(p->interp, p->zScript);
  ਜੇ( rc!=TCL_OK ){
    Tcl_BackgroundError(p->interp);
  }
  ਅਣਵਰਤਿਆ_PARAMETER(flags);
  ਵਾਪਸ 1;
}

/*
** Register an EvalEvent to evaluate the script pScript in the
** parent interpreter/thread of SqlThread p.
*/
ਸਥਿਰ ਖਾਲੀ post_ਮੰਜ਼ਿਲ_ਮਾਪਾ(SqlThread *p, Tcl_Obj *pScript){
  EvalEvent *pEvent;
  ਅੱਖਰ *z_ਸੁਨੇਹਾ;
  Tcl_ਮਾਪ n_ਸੁਨੇਹਾ;

  z_ਸੁਨੇਹਾ = Tcl_GetStringFromObj(pScript, &n_ਸੁਨੇਹਾ); 
  pEvent = (EvalEvent *)ckalloc(ਆਕਾਰ(EvalEvent)+n_ਸੁਨੇਹਾ+1);
  pEvent->ਅਧਾਰ.ਅਗਲਾ_Ptr = 0;
  pEvent->ਅਧਾਰ.proc = tclScriptEvent;
  pEvent->zScript = (ਅੱਖਰ *)&pEvent[1];
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(pEvent->zScript, z_ਸੁਨੇਹਾ, n_ਸੁਨੇਹਾ+1);
  pEvent->interp = p->interp;

  Tcl_Thread_ਕਤਾਰ_Event(p->ਮਾਪਾ, (Tcl_Event *)pEvent, TCL_ਕਤਾਰ_ਪੂਛ);
  Tcl_ThreadAlert(p->ਮਾਪਾ);
}

/*
** The main function for threads created with [sqlthread spawn].
*/
ਸਥਿਰ Tcl_ThreadCreateType tclScriptThread(Client_ਅੰਕੜੇ pSqlThread){
  Tcl_Interp *interp;
  Tcl_Obj *pRes;
  Tcl_Obj *pList;
  ਪੂਰਨ rc;
  SqlThread *p = (SqlThread *)pSqlThread;
  ਬਾਹਰੀ ਪੂਰਨ Sqlitetest_mutex_Init(Tcl_Interp*);

  interp = Tcl_CreateInterp();
  Tcl_CreateObjCommand(interp, "clock_seconds", clock_seconds_proc, 0, 0);
  Tcl_CreateObjCommand(interp, "sqlthread", sqlthread_proc, pSqlThread, 0);
#ਜੇ SQLITE_OS_UNIX && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)
  Tcl_CreateObjCommand(interp, "sqlite3_blocking_step", blocking_ਕਦਮ_proc,0,0);
  Tcl_CreateObjCommand(interp, 
      "sqlite3_blocking_prepare_v2", blocking_prepare_v2_proc, (ਖਾਲੀ *)1, 0);
  Tcl_CreateObjCommand(interp, 
      "sqlite3_nonblocking_prepare_v2", blocking_prepare_v2_proc, 0, 0);
#ਜੇ_ਅੰਤ
  Sqlitetest1_Init(interp);
  Sqlitetest_mutex_Init(interp);
  Sqlite3_Init(interp);

  rc = Tcl_Eval(interp, p->zScript);
  pRes = Tcl_Get_Obj_ਨਤੀਜਾ(interp);
  pList = Tcl_NewObj();
  Tcl_Incr_Ref_ਗਿਣਤੀ(pList);
  Tcl_Incr_Ref_ਗਿਣਤੀ(pRes);

  ਜੇ( rc!=TCL_OK ){
    Tcl_ListObjAppendElement(interp, pList, Tcl_NewStringObj("error", -1));
    Tcl_ListObjAppendElement(interp, pList, pRes);
    post_ਮੰਜ਼ਿਲ_ਮਾਪਾ(p, pList);
    Tcl_Decr_Ref_ਗਿਣਤੀ(pList);
    pList = Tcl_NewObj();
  }

  Tcl_ListObjAppendElement(interp, pList, Tcl_NewStringObj("set", -1));
  Tcl_ListObjAppendElement(interp, pList, Tcl_NewStringObj(p->zVarname, -1));
  Tcl_ListObjAppendElement(interp, pList, pRes);
  post_ਮੰਜ਼ਿਲ_ਮਾਪਾ(p, pList);

  ckfree((ਖਾਲੀ *)p);
  Tcl_Decr_Ref_ਗਿਣਤੀ(pList);
  Tcl_Decr_Ref_ਗਿਣਤੀ(pRes);
  Tcl_DeleteInterp(interp);
  ਜਦੋਂ( Tcl_DoOneEvent(TCL_ALL_EVENTS|TCL_DONT_WAIT) );
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
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI sqlthread_spawn(
  Client_ਅੰਕੜੇ client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_ThreadId x;
  SqlThread *pNew;
  ਪੂਰਨ rc;

  Tcl_ਮਾਪ nVarname; ਅੱਖਰ *zVarname;
  Tcl_ਮਾਪ nScript; ਅੱਖਰ *zScript;

  /* Parameters for thread creation */
  ਅਚਲ ਪੂਰਨ n_ਢੇਰ_2 = TCL_THREAD_ਢੇਰ_DEFAULT;
  ਅਚਲ ਪੂਰਨ flags = TCL_THREAD_NOFLAGS;

  ਪ੍ਰਤਿਜ੍ਞਾ(objc==4);
  ਅਣਵਰਤਿਆ_PARAMETER(client_ਅੰਕੜੇ);
  ਅਣਵਰਤਿਆ_PARAMETER(objc);

  zVarname = Tcl_GetStringFromObj(objv[2], &nVarname);
  zScript = Tcl_GetStringFromObj(objv[3], &nScript);

  pNew = (SqlThread *)ckalloc(ਆਕਾਰ(SqlThread)+nVarname+nScript+2);
  pNew->zVarname = (ਅੱਖਰ *)&pNew[1];
  pNew->zScript = (ਅੱਖਰ *)&pNew->zVarname[nVarname+1];
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(pNew->zVarname, zVarname, nVarname+1);
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(pNew->zScript, zScript, nScript+1);
  pNew->ਮਾਪਾ = Tcl_GetCurrentThread();
  pNew->interp = interp;

  rc = Tcl_CreateThread(&x, tclScriptThread, (ਖਾਲੀ *)pNew, n_ਢੇਰ_2, flags);
  ਜੇ( rc!=TCL_OK ){
    Tcl_Append_ਨਤੀਜਾ(interp, "Error in Tcl_CreateThread()", ਨਿਰਾ);
    ckfree((ਅੱਖਰ *)pNew);
    ਵਾਪਸ TCL_ERROR;
  }

  ਵਾਪਸ TCL_OK;
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
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI sqlthread_ਮਾਪਾ(
  Client_ਅੰਕੜੇ client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  EvalEvent *pEvent;
  ਅੱਖਰ *z_ਸੁਨੇਹਾ;
  Tcl_ਮਾਪ n_ਸੁਨੇਹਾ;
  SqlThread *p = (SqlThread *)client_ਅੰਕੜੇ;

  ਪ੍ਰਤਿਜ੍ਞਾ(objc==3);
  ਅਣਵਰਤਿਆ_PARAMETER(objc);

  ਜੇ( p==0 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "no parent thread", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }

  z_ਸੁਨੇਹਾ = Tcl_GetStringFromObj(objv[2], &n_ਸੁਨੇਹਾ);
  pEvent = (EvalEvent *)ckalloc(ਆਕਾਰ(EvalEvent)+n_ਸੁਨੇਹਾ+1);
  pEvent->ਅਧਾਰ.ਅਗਲਾ_Ptr = 0;
  pEvent->ਅਧਾਰ.proc = tclScriptEvent;
  pEvent->zScript = (ਅੱਖਰ *)&pEvent[1];
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(pEvent->zScript, z_ਸੁਨੇਹਾ, n_ਸੁਨੇਹਾ+1);
  pEvent->interp = p->interp;
  Tcl_Thread_ਕਤਾਰ_Event(p->ਮਾਪਾ, (Tcl_Event *)pEvent, TCL_ਕਤਾਰ_ਪੂਛ);
  Tcl_ThreadAlert(p->ਮਾਪਾ);

  ਵਾਪਸ TCL_OK;
}

ਸਥਿਰ ਪੂਰਨ xBusy(ਖਾਲੀ *pArg, ਪੂਰਨ nBusy){
  ਅਣਵਰਤਿਆ_PARAMETER(pArg);
  ਅਣਵਰਤਿਆ_PARAMETER(nBusy);
  sqlite3_sleep(50);
  ਵਾਪਸ 1;             /* Try again... */
}

/*
** sqlthread open
**
**     Open a database handle and return the string representation of
**     the pointer value.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI sqlthread_open(
  Client_ਅੰਕੜੇ client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਪੂਰਨ sqlite3_Test_Make_ਸੂਚਕ_Str(Tcl_Interp *interp, ਅੱਖਰ *zPtr, ਖਾਲੀ *p);

  ਅਚਲ ਅੱਖਰ *zFilename;
  sqlite3 *db;
  ਅੱਖਰ zBuf[100];
  ਬਾਹਰੀ ਪੂਰਨ Md5_ਰਜਿਸਟਰ(sqlite3*,ਅੱਖਰ**,ਅਚਲ sqlite3_api_routines*);

  ਅਣਵਰਤਿਆ_PARAMETER(client_ਅੰਕੜੇ);
  ਅਣਵਰਤਿਆ_PARAMETER(objc);

  zFilename = Tcl_GetString(objv[2]);
  sqlite3_open(zFilename, &db);
  Md5_ਰਜਿਸਟਰ(db, 0, 0);
  sqlite3_busy_handler(db, xBusy, 0);
  
  ਜੇ( sqlite3_Test_Make_ਸੂਚਕ_Str(interp, zBuf, db) ) ਵਾਪਸ TCL_ERROR;
  Tcl_Append_ਨਤੀਜਾ(interp, zBuf, ਨਿਰਾ);

  ਵਾਪਸ TCL_OK;
}


/*
** sqlthread open
**
**     Return the current thread-id (Tcl_GetCurrentThread()) cast to
**     an integer.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI sqlthread_id(
  Client_ਅੰਕੜੇ client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_ThreadId id = Tcl_GetCurrentThread();
  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewIntObj(SQLITE_PTR_ਮੰਜ਼ਿਲ_INT(id)));
  ਅਣਵਰਤਿਆ_PARAMETER(client_ਅੰਕੜੇ);
  ਅਣਵਰਤਿਆ_PARAMETER(objc);
  ਅਣਵਰਤਿਆ_PARAMETER(objv);
  ਵਾਪਸ TCL_OK;
}


/*
** Dispatch routine for the sub-commands of [sqlthread].
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI sqlthread_proc(
  Client_ਅੰਕੜੇ client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਬਣਤਰ SubCommand {
    ਅੱਖਰ *zName;
    Tcl_ObjCmdProc *xProc;
    ਪੂਰਨ nArg;
    ਅੱਖਰ *zUsage;
  } aSub[] = {
    {"parent", sqlthread_ਮਾਪਾ, 1, "SCRIPT"},
    {"spawn",  sqlthread_spawn,  2, "VARNAME SCRIPT"},
    {"open",   sqlthread_open,   1, "DBNAME"},
    {"id",     sqlthread_id,     0, ""},
    {0, 0, 0}
  };
  ਬਣਤਰ SubCommand *pSub;
  ਪੂਰਨ rc;
  ਪੂਰਨ i_ਸੂਚਕਾਂਕ;

  ਜੇ( objc<2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "SUB-COMMAND");
    ਵਾਪਸ TCL_ERROR;
  }

  rc = Tcl_Get_ਸੂਚਕਾਂਕ_From_Obj_Struct(
      interp, objv[1], aSub, ਆਕਾਰ(aSub[0]), "sub-command", 0, &i_ਸੂਚਕਾਂਕ
  );
  ਜੇ( rc!=TCL_OK ) ਵਾਪਸ rc;
  pSub = &aSub[i_ਸੂਚਕਾਂਕ];

  ਜੇ( objc<(pSub->nArg+2) ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 2, objv, pSub->zUsage);
    ਵਾਪਸ TCL_ERROR;
  }

  ਵਾਪਸ pSub->xProc(client_ਅੰਕੜੇ, interp, objc, objv);
}

/*
** The [clock_seconds] command. This is more or less the same as the
** regular tcl [clock seconds], except that it is available in testfixture
** when linked against both Tcl 8.4 and 8.5. Because [clock seconds] is
** implemented as a script in Tcl 8.5, it is not usually available to
** testfixture.
*/ 
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI clock_seconds_proc(
  Client_ਅੰਕੜੇ client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_ਸਮਾਂ now;
  Tcl_Get_ਸਮਾਂ(&now);
  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewIntObj(now.sec));
  ਅਣਵਰਤਿਆ_PARAMETER(client_ਅੰਕੜੇ);
  ਅਣਵਰਤਿਆ_PARAMETER(objc);
  ਅਣਵਰਤਿਆ_PARAMETER(objv);
  ਵਾਪਸ TCL_OK;
}

/*
** The [clock_milliseconds] command. This is more or less the same as the
** regular tcl [clock milliseconds]. 
*/ 
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI clock_milliseconds_proc(
  Client_ਅੰਕੜੇ client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_ਸਮਾਂ now;
  Tcl_Get_ਸਮਾਂ(&now);
  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewWideIntObj(
    ((Tcl_WideInt)now.sec * 1000) + (now.usec / 1000)
  ));
  ਅਣਵਰਤਿਆ_PARAMETER(client_ਅੰਕੜੇ);
  ਅਣਵਰਤਿਆ_PARAMETER(objc);
  ਅਣਵਰਤਿਆ_PARAMETER(objv);
  ਵਾਪਸ TCL_OK;
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
#ਜੇ SQLITE_OS_UNIX && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)

/* BEGIN_SQLITE_BLOCKING_STEP */
/* This example uses the pthreads API */
#ਅਨ੍ਤਰ੍ਭਾਵ <pthread.h>

/*
** A pointer to an instance of this structure is passed as the user-context
** pointer when registering for an unlock-notify callback.
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ UnlockNotification UnlockNotification;
ਬਣਤਰ UnlockNotification {
  ਪੂਰਨ fired;                         /* True after unlock event has occurred */
  pthread_cond_t cond;               /* Condition variable to wait on */
  pthread_mutex_t mutex;             /* Mutex to protect structure */
};

/*
** This function is an unlock-notify callback registered with SQLite.
*/
ਸਥਿਰ ਖਾਲੀ unlock_notify_cb(ਖਾਲੀ **apArg, ਪੂਰਨ nArg){
  ਪੂਰਨ i;
  ਲਈ(i=0; i<nArg; i++){
    UnlockNotification *p = (UnlockNotification *)apArg[i];
    pthread_mutex_ਤਾਲਾ(&p->mutex);
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
ਸਥਿਰ ਪੂਰਨ wait_for_unlock_notify(sqlite3 *db){
  ਪੂਰਨ rc;
  UnlockNotification un;

  /* Initialize the UnlockNotification structure. */
  un.fired = 0;
  pthread_mutex_init(&un.mutex, 0);
  pthread_cond_init(&un.cond, 0);

  /* Register for an unlock-notify callback. */
  rc = sqlite3_unlock_notify(db, unlock_notify_cb, (ਖਾਲੀ *)&un);
  ਪ੍ਰਤਿਜ੍ਞਾ( rc==SQLITE_LOCKED || rc==SQLITE_OK );

  /* The call to sqlite3_unlock_notify() always returns either SQLITE_LOCKED 
  ** or SQLITE_OK. 
  **
  ** If SQLITE_LOCKED was returned, then the system is deadlocked. In this
  ** case this function needs to return SQLITE_LOCKED to the caller so 
  ** that the current transaction can be rolled back. Otherwise, block
  ** until the unlock-notify callback is invoked, then return SQLITE_OK.
  */
  ਜੇ( rc==SQLITE_OK ){
    pthread_mutex_ਤਾਲਾ(&un.mutex);
    ਜੇ( !un.fired ){
      pthread_cond_wait(&un.cond, &un.mutex);
    }
    pthread_mutex_unlock(&un.mutex);
  }

  /* Destroy the mutex and condition variables. */
  pthread_cond_destroy(&un.cond);
  pthread_mutex_destroy(&un.mutex);

  ਵਾਪਸ rc;
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
ਪੂਰਨ sqlite3_blocking_ਕਦਮ(sqlite3_stmt *pStmt){
  ਪੂਰਨ rc;
  ਜਦੋਂ( SQLITE_LOCKED==(rc = sqlite3_ਕਦਮ(pStmt)) ){
    rc = wait_for_unlock_notify(sqlite3_db_handle(pStmt));
    ਜੇ( rc!=SQLITE_OK ) ਰੋਕੋ;
    sqlite3_reset(pStmt);
  }
  ਵਾਪਸ rc;
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
ਪੂਰਨ sqlite3_blocking_prepare_v2(
  sqlite3 *db,              /* Database handle. */
  ਅਚਲ ਅੱਖਰ *zSql,         /* UTF-8 encoded SQL statement. */
  ਪੂਰਨ nSql,                 /* Length of zSql in bytes. */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  ਅਚਲ ਅੱਖਰ **pz           /* OUT: End of parsed string */
){
  ਪੂਰਨ rc;
  ਜਦੋਂ( SQLITE_LOCKED==(rc = sqlite3_prepare_v2(db, zSql, nSql, ppStmt, pz)) ){
    rc = wait_for_unlock_notify(db);
    ਜੇ( rc!=SQLITE_OK ) ਰੋਕੋ;
  }
  ਵਾਪਸ rc;
}
/* END_SQLITE_BLOCKING_STEP */

/*
** Usage: sqlite3_blocking_step STMT
**
** Advance the statement to the next row.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI blocking_ਕਦਮ_proc(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){

  sqlite3_stmt *pStmt;
  ਪੂਰਨ rc;

  ਜੇ( objc!=2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "STMT");
    ਵਾਪਸ TCL_ERROR;
  }

  pStmt = (sqlite3_stmt*)sqlite3_Test_ਪਾਠ_ਮੰਜ਼ਿਲ_Ptr(Tcl_GetString(objv[1]));
  rc = sqlite3_blocking_ਕਦਮ(pStmt);

  Tcl_ਸੈੱਟ_ਨਤੀਜਾ(interp, (ਅੱਖਰ *)sqlite3ErrName(rc), 0);
  ਵਾਪਸ TCL_OK;
}

/*
** Usage: sqlite3_blocking_prepare_v2 DB sql bytes ?tailvar?
** Usage: sqlite3_nonblocking_prepare_v2 DB sql bytes ?tailvar?
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI blocking_prepare_v2_proc(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  ਅਚਲ ਅੱਖਰ *zSql;
  ਪੂਰਨ ਬਾਈਟਾਂ;
  ਅਚਲ ਅੱਖਰ *z_ਪੂਛ = 0;
  sqlite3_stmt *pStmt = 0;
  ਅੱਖਰ zBuf[50];
  ਪੂਰਨ rc;
  ਪੂਰਨ isBlocking = !(client_ਅੰਕੜੇ==0);

  ਜੇ( objc!=5 && objc!=4 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "wrong # args: should be \"", 
       Tcl_GetString(objv[0]), " DB sql bytes tailvar", 0);
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( get_Db_ਸੂਚਕ(interp, Tcl_GetString(objv[1]), &db) ) ਵਾਪਸ TCL_ERROR;
  zSql = Tcl_GetString(objv[2]);
  ਜੇ( Tcl_GetIntFromObj(interp, objv[3], &ਬਾਈਟਾਂ) ) ਵਾਪਸ TCL_ERROR;

  ਜੇ( isBlocking ){
    rc = sqlite3_blocking_prepare_v2(db, zSql, ਬਾਈਟਾਂ, &pStmt, &z_ਪੂਛ);
  }ਨਹੀਂਤਾਂ{
    rc = sqlite3_prepare_v2(db, zSql, ਬਾਈਟਾਂ, &pStmt, &z_ਪੂਛ);
  }

  ਪ੍ਰਤਿਜ੍ਞਾ(rc==SQLITE_OK || pStmt==0);
  ਜੇ( z_ਪੂਛ && objc>=5 ){
    ਜੇ( ਬਾਈਟਾਂ>=0 ){
      ਬਾਈਟਾਂ = ਬਾਈਟਾਂ - (z_ਪੂਛ-zSql);
    }
    Tcl_Obj_ਸੈੱਟ_Var2(interp, objv[4], 0, Tcl_NewStringObj(z_ਪੂਛ, ਬਾਈਟਾਂ), 0);
  }
  ਜੇ( rc!=SQLITE_OK ){
    ਪ੍ਰਤਿਜ੍ਞਾ( pStmt==0 );
    sqlite3_snprintf(ਆਕਾਰ(zBuf), zBuf, "%s ", (ਅੱਖਰ *)sqlite3ErrName(rc));
    Tcl_Append_ਨਤੀਜਾ(interp, zBuf, sqlite3_errmsg(db), ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }

  ਜੇ( pStmt ){
    ਜੇ( sqlite3_Test_Make_ਸੂਚਕ_Str(interp, zBuf, pStmt) ) ਵਾਪਸ TCL_ERROR;
    Tcl_Append_ਨਤੀਜਾ(interp, zBuf, ਨਿਰਾ);
  }
  ਵਾਪਸ TCL_OK;
}

#ਜੇ_ਅੰਤ /* SQLITE_OS_UNIX && SQLITE_ENABLE_UNLOCK_NOTIFY */
/*
** End of implementation of [sqlite3_blocking_step].
************************************************************************/

/*
** Register commands with the TCL interpreter.
*/
ਪੂਰਨ SqlitetestThread_Init(Tcl_Interp *interp){
  ਬਣਤਰ TclCmd {
    ਪੂਰਨ (*xProc)(ਖਾਲੀ*, Tcl_Interp*, ਪੂਰਨ, Tcl_Obj*ਅਚਲ*);
    ਅਚਲ ਅੱਖਰ *zName;
    ਪੂਰਨ iCtx;
  } aCmd[] = {
    { sqlthread_proc,           "sqlthread",                      0 },
    { clock_seconds_proc,       "clock_second",                   0 },
    { clock_milliseconds_proc,  "clock_milliseconds",             0 },
#ਜੇ SQLITE_OS_UNIX && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)
    { blocking_ਕਦਮ_proc,       "sqlite3_blocking_step",          0 },
    { blocking_prepare_v2_proc, "sqlite3_blocking_prepare_v2",    1 },
    { blocking_prepare_v2_proc, "sqlite3_nonblocking_prepare_v2", 0 },
#ਜੇ_ਅੰਤ
  };
  ਪੂਰਨ ii;

  ਲਈ(ii=0; ii<ਆਕਾਰ(aCmd)/ਆਕਾਰ(aCmd[0]); ii++){
    ਖਾਲੀ *p = SQLITE_INT_ਮੰਜ਼ਿਲ_PTR(aCmd[ii].iCtx);
    Tcl_CreateObjCommand(interp, aCmd[ii].zName, aCmd[ii].xProc, p, 0);
  }
  ਵਾਪਸ TCL_OK;
}
#ਨਹੀਂਤਾਂ
ਪੂਰਨ SqlitetestThread_Init(Tcl_Interp *interp){
  ਵਾਪਸ TCL_OK;
}
#ਜੇ_ਅੰਤ
