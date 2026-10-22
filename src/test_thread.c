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

#അന്തര്ഭാവ "sqliteInt.h"
#അന്തര്ഭാവ "tclsqlite.h"

#എങ്കിൽ SQLITE_THREADSAFE

#അന്തര്ഭാവ <errno.h>

#എങ്കിൽ !defined(_MSC_VER)
#അന്തര്ഭാവ <unistd.h>
#endif

/*
** One of these is allocated for each thread created by [sqlthread spawn].
*/
തരനിർവചനം ഘടന SqlThread SqlThread;
ഘടന SqlThread {
  Tcl_ThreadId parent;     /* Thread id of parent thread */
  Tcl_Interp *interp;      /* Parent interpreter */
  അക്ഷരം *zScript;           /* The script to execute. */
  അക്ഷരം *zVarname;          /* Varname in parent script */
};

/*
** A custom Tcl_Event type used by this module. When the event is
** handled, script zScript is evaluated in interpreter interp. If
** the evaluation throws an exception (returns TCL_ERROR), then the
** error is handled by Tcl_BackgroundError(). If no error occurs,
** the result is simply discarded.
*/
തരനിർവചനം ഘടന EvalEvent EvalEvent;
ഘടന EvalEvent {
  Tcl_Event base;          /* Base class of type Tcl_Event */
  അക്ഷരം *zScript;           /* The script to execute. */
  Tcl_Interp *interp;      /* The interpreter to execute it in. */
};

സ്ഥിതം Tcl_ObjCmdProc sqlthread_proc;
സ്ഥിതം Tcl_ObjCmdProc clock_seconds_proc;
#എങ്കിൽ SQLITE_OS_UNIX && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)
സ്ഥിതം Tcl_ObjCmdProc blocking_step_proc;
സ്ഥിതം Tcl_ObjCmdProc blocking_prepare_v2_proc;
#endif
പൂർണ്ണസംഖ്യ Sqlitetest1_Init(Tcl_Interp *);
പൂർണ്ണസംഖ്യ Sqlite3_Init(Tcl_Interp *);

/* Functions from main.c */
ബാഹ്യ സ്ഥിരം അക്ഷരം *sqlite3ErrName(പൂർണ്ണസംഖ്യ);

/* Functions from test1.c */
ബാഹ്യ ശൂന്യം *sqlite3TestTextToPtr(സ്ഥിരം അക്ഷരം *);
ബാഹ്യ പൂർണ്ണസംഖ്യ getDbPointer(Tcl_Interp *, സ്ഥിരം അക്ഷരം *, sqlite3 **);
ബാഹ്യ പൂർണ്ണസംഖ്യ sqlite3TestMakePointerStr(Tcl_Interp *, അക്ഷരം *, ശൂന്യം *);
ബാഹ്യ പൂർണ്ണസംഖ്യ sqlite3TestErrCode(Tcl_Interp *, sqlite3 *, പൂർണ്ണസംഖ്യ);

/*
** Handler for events of type EvalEvent.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI tclScriptEvent(Tcl_Event *evPtr, പൂർണ്ണസംഖ്യ flags){
  പൂർണ്ണസംഖ്യ rc;
  EvalEvent *p = (EvalEvent *)evPtr;
  rc = Tcl_Eval(p->interp, p->zScript);
  എങ്കിൽ( rc!=TCL_OK ){
    Tcl_BackgroundError(p->interp);
  }
  UNUSED_PARAMETER(flags);
  തിരികെ 1;
}

/*
** Register an EvalEvent to evaluate the script pScript in the
** parent interpreter/thread of SqlThread p.
*/
സ്ഥിതം ശൂന്യം postToParent(SqlThread *p, Tcl_Obj *pScript){
  EvalEvent *pEvent;
  അക്ഷരം *z_സന്ദേശം;
  Tcl_Size n_സന്ദേശം;

  z_സന്ദേശം = Tcl_GetStringFromObj(pScript, &n_സന്ദേശം); 
  pEvent = (EvalEvent *)ckalloc(വലുപ്പം(EvalEvent)+n_സന്ദേശം+1);
  pEvent->base.nextPtr = 0;
  pEvent->base.proc = tclScriptEvent;
  pEvent->zScript = (അക്ഷരം *)&pEvent[1];
  memcpy(pEvent->zScript, z_സന്ദേശം, n_സന്ദേശം+1);
  pEvent->interp = p->interp;

  Tcl_ThreadQueueEvent(p->parent, (Tcl_Event *)pEvent, TCL_QUEUE_TAIL);
  Tcl_ThreadAlert(p->parent);
}

/*
** The main function for threads created with [sqlthread spawn].
*/
സ്ഥിതം Tcl_ThreadCreateType tclScriptThread(ClientData pSqlThread){
  Tcl_Interp *interp;
  Tcl_Obj *pRes;
  Tcl_Obj *pList;
  പൂർണ്ണസംഖ്യ rc;
  SqlThread *p = (SqlThread *)pSqlThread;
  ബാഹ്യ പൂർണ്ണസംഖ്യ Sqlitetest_mutex_Init(Tcl_Interp*);

  interp = Tcl_CreateInterp();
  Tcl_CreateObjCommand(interp, "clock_seconds", clock_seconds_proc, 0, 0);
  Tcl_CreateObjCommand(interp, "sqlthread", sqlthread_proc, pSqlThread, 0);
#എങ്കിൽ SQLITE_OS_UNIX && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)
  Tcl_CreateObjCommand(interp, "sqlite3_blocking_step", blocking_step_proc,0,0);
  Tcl_CreateObjCommand(interp, 
      "sqlite3_blocking_prepare_v2", blocking_prepare_v2_proc, (ശൂന്യം *)1, 0);
  Tcl_CreateObjCommand(interp, 
      "sqlite3_nonblocking_prepare_v2", blocking_prepare_v2_proc, 0, 0);
#endif
  Sqlitetest1_Init(interp);
  Sqlitetest_mutex_Init(interp);
  Sqlite3_Init(interp);

  rc = Tcl_Eval(interp, p->zScript);
  pRes = Tcl_GetObjResult(interp);
  pList = Tcl_NewObj();
  Tcl_Incr_Ref_എണ്ണം(pList);
  Tcl_Incr_Ref_എണ്ണം(pRes);

  എങ്കിൽ( rc!=TCL_OK ){
    Tcl_ListObjAppendElement(interp, pList, Tcl_NewStringObj("error", -1));
    Tcl_ListObjAppendElement(interp, pList, pRes);
    postToParent(p, pList);
    Tcl_Decr_Ref_എണ്ണം(pList);
    pList = Tcl_NewObj();
  }

  Tcl_ListObjAppendElement(interp, pList, Tcl_NewStringObj("set", -1));
  Tcl_ListObjAppendElement(interp, pList, Tcl_NewStringObj(p->zVarname, -1));
  Tcl_ListObjAppendElement(interp, pList, pRes);
  postToParent(p, pList);

  ckfree((ശൂന്യം *)p);
  Tcl_Decr_Ref_എണ്ണം(pList);
  Tcl_Decr_Ref_എണ്ണം(pRes);
  Tcl_DeleteInterp(interp);
  ആകുമ്പോൾ( Tcl_DoOneEvent(TCL_ALL_EVENTS|TCL_DONT_WAIT) );
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
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI sqlthread_spawn(
  ClientData clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_ThreadId x;
  SqlThread *pNew;
  പൂർണ്ണസംഖ്യ rc;

  Tcl_Size nVarname; അക്ഷരം *zVarname;
  Tcl_Size nScript; അക്ഷരം *zScript;

  /* Parameters for thread creation */
  സ്ഥിരം പൂർണ്ണസംഖ്യ n_അട്ടി = TCL_THREAD_അട്ടി_DEFAULT;
  സ്ഥിരം പൂർണ്ണസംഖ്യ flags = TCL_THREAD_NOFLAGS;

  പ്രതിജ്ഞാ(objc==4);
  UNUSED_PARAMETER(clientData);
  UNUSED_PARAMETER(objc);

  zVarname = Tcl_GetStringFromObj(objv[2], &nVarname);
  zScript = Tcl_GetStringFromObj(objv[3], &nScript);

  pNew = (SqlThread *)ckalloc(വലുപ്പം(SqlThread)+nVarname+nScript+2);
  pNew->zVarname = (അക്ഷരം *)&pNew[1];
  pNew->zScript = (അക്ഷരം *)&pNew->zVarname[nVarname+1];
  memcpy(pNew->zVarname, zVarname, nVarname+1);
  memcpy(pNew->zScript, zScript, nScript+1);
  pNew->parent = Tcl_GetCurrentThread();
  pNew->interp = interp;

  rc = Tcl_CreateThread(&x, tclScriptThread, (ശൂന്യം *)pNew, n_അട്ടി, flags);
  എങ്കിൽ( rc!=TCL_OK ){
    Tcl_AppendResult(interp, "Error in Tcl_CreateThread()", ശൂന്യ);
    ckfree((അക്ഷരം *)pNew);
    തിരികെ TCL_ERROR;
  }

  തിരികെ TCL_OK;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI sqlthread_parent(
  ClientData clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  EvalEvent *pEvent;
  അക്ഷരം *z_സന്ദേശം;
  Tcl_Size n_സന്ദേശം;
  SqlThread *p = (SqlThread *)clientData;

  പ്രതിജ്ഞാ(objc==3);
  UNUSED_PARAMETER(objc);

  എങ്കിൽ( p==0 ){
    Tcl_AppendResult(interp, "no parent thread", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }

  z_സന്ദേശം = Tcl_GetStringFromObj(objv[2], &n_സന്ദേശം);
  pEvent = (EvalEvent *)ckalloc(വലുപ്പം(EvalEvent)+n_സന്ദേശം+1);
  pEvent->base.nextPtr = 0;
  pEvent->base.proc = tclScriptEvent;
  pEvent->zScript = (അക്ഷരം *)&pEvent[1];
  memcpy(pEvent->zScript, z_സന്ദേശം, n_സന്ദേശം+1);
  pEvent->interp = p->interp;
  Tcl_ThreadQueueEvent(p->parent, (Tcl_Event *)pEvent, TCL_QUEUE_TAIL);
  Tcl_ThreadAlert(p->parent);

  തിരികെ TCL_OK;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ xBusy(ശൂന്യം *pArg, പൂർണ്ണസംഖ്യ nBusy){
  UNUSED_PARAMETER(pArg);
  UNUSED_PARAMETER(nBusy);
  sqlite3_sleep(50);
  തിരികെ 1;             /* Try again... */
}

/*
** sqlthread open
**
**     Open a database handle and return the string representation of
**     the pointer value.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI sqlthread_open(
  ClientData clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  പൂർണ്ണസംഖ്യ sqlite3TestMakePointerStr(Tcl_Interp *interp, അക്ഷരം *zPtr, ശൂന്യം *p);

  സ്ഥിരം അക്ഷരം *zFilename;
  sqlite3 *db;
  അക്ഷരം zBuf[100];
  ബാഹ്യ പൂർണ്ണസംഖ്യ Md5_Register(sqlite3*,അക്ഷരം**,സ്ഥിരം sqlite3_api_routines*);

  UNUSED_PARAMETER(clientData);
  UNUSED_PARAMETER(objc);

  zFilename = Tcl_GetString(objv[2]);
  sqlite3_open(zFilename, &db);
  Md5_Register(db, 0, 0);
  sqlite3_busy_handler(db, xBusy, 0);
  
  എങ്കിൽ( sqlite3TestMakePointerStr(interp, zBuf, db) ) തിരികെ TCL_ERROR;
  Tcl_AppendResult(interp, zBuf, ശൂന്യ);

  തിരികെ TCL_OK;
}


/*
** sqlthread open
**
**     Return the current thread-id (Tcl_GetCurrentThread()) cast to
**     an integer.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI sqlthread_id(
  ClientData clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_ThreadId id = Tcl_GetCurrentThread();
  Tcl_SetObjResult(interp, Tcl_NewIntObj(SQLITE_PTR_TO_INT(id)));
  UNUSED_PARAMETER(clientData);
  UNUSED_PARAMETER(objc);
  UNUSED_PARAMETER(objv);
  തിരികെ TCL_OK;
}


/*
** Dispatch routine for the sub-commands of [sqlthread].
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI sqlthread_proc(
  ClientData clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  ഘടന SubCommand {
    അക്ഷരം *zName;
    Tcl_ObjCmdProc *xProc;
    പൂർണ്ണസംഖ്യ nArg;
    അക്ഷരം *zUsage;
  } aSub[] = {
    {"parent", sqlthread_parent, 1, "SCRIPT"},
    {"spawn",  sqlthread_spawn,  2, "VARNAME SCRIPT"},
    {"open",   sqlthread_open,   1, "DBNAME"},
    {"id",     sqlthread_id,     0, ""},
    {0, 0, 0}
  };
  ഘടന SubCommand *pSub;
  പൂർണ്ണസംഖ്യ rc;
  പൂർണ്ണസംഖ്യ iIndex;

  എങ്കിൽ( objc<2 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "SUB-COMMAND");
    തിരികെ TCL_ERROR;
  }

  rc = Tcl_GetIndexFromObjStruct(
      interp, objv[1], aSub, വലുപ്പം(aSub[0]), "sub-command", 0, &iIndex
  );
  എങ്കിൽ( rc!=TCL_OK ) തിരികെ rc;
  pSub = &aSub[iIndex];

  എങ്കിൽ( objc<(pSub->nArg+2) ){
    Tcl_Wrong_അക്കം_Args(interp, 2, objv, pSub->zUsage);
    തിരികെ TCL_ERROR;
  }

  തിരികെ pSub->xProc(clientData, interp, objc, objv);
}

/*
** The [clock_seconds] command. This is more or less the same as the
** regular tcl [clock seconds], except that it is available in testfixture
** when linked against both Tcl 8.4 and 8.5. Because [clock seconds] is
** implemented as a script in Tcl 8.5, it is not usually available to
** testfixture.
*/ 
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI clock_seconds_proc(
  ClientData clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Time now;
  Tcl_GetTime(&now);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(now.sec));
  UNUSED_PARAMETER(clientData);
  UNUSED_PARAMETER(objc);
  UNUSED_PARAMETER(objv);
  തിരികെ TCL_OK;
}

/*
** The [clock_milliseconds] command. This is more or less the same as the
** regular tcl [clock milliseconds]. 
*/ 
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI clock_milliseconds_proc(
  ClientData clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
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
  തിരികെ TCL_OK;
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
#എങ്കിൽ SQLITE_OS_UNIX && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)

/* BEGIN_SQLITE_BLOCKING_STEP */
/* This example uses the pthreads API */
#അന്തര്ഭാവ <pthread.h>

/*
** A pointer to an instance of this structure is passed as the user-context
** pointer when registering for an unlock-notify callback.
*/
തരനിർവചനം ഘടന UnlockNotification UnlockNotification;
ഘടന UnlockNotification {
  പൂർണ്ണസംഖ്യ fired;                         /* True after unlock event has occurred */
  pthread_cond_t cond;               /* Condition variable to wait on */
  pthread_mutex_t mutex;             /* Mutex to protect structure */
};

/*
** This function is an unlock-notify callback registered with SQLite.
*/
സ്ഥിതം ശൂന്യം unlock_notify_cb(ശൂന്യം **apArg, പൂർണ്ണസംഖ്യ nArg){
  പൂർണ്ണസംഖ്യ i;
  വേണ്ടി(i=0; i<nArg; i++){
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
സ്ഥിതം പൂർണ്ണസംഖ്യ wait_for_unlock_notify(sqlite3 *db){
  പൂർണ്ണസംഖ്യ rc;
  UnlockNotification un;

  /* Initialize the UnlockNotification structure. */
  un.fired = 0;
  pthread_mutex_init(&un.mutex, 0);
  pthread_cond_init(&un.cond, 0);

  /* Register for an unlock-notify callback. */
  rc = sqlite3_unlock_notify(db, unlock_notify_cb, (ശൂന്യം *)&un);
  പ്രതിജ്ഞാ( rc==SQLITE_LOCKED || rc==SQLITE_OK );

  /* The call to sqlite3_unlock_notify() always returns either SQLITE_LOCKED 
  ** or SQLITE_OK. 
  **
  ** If SQLITE_LOCKED was returned, then the system is deadlocked. In this
  ** case this function needs to return SQLITE_LOCKED to the caller so 
  ** that the current transaction can be rolled back. Otherwise, block
  ** until the unlock-notify callback is invoked, then return SQLITE_OK.
  */
  എങ്കിൽ( rc==SQLITE_OK ){
    pthread_mutex_lock(&un.mutex);
    എങ്കിൽ( !un.fired ){
      pthread_cond_wait(&un.cond, &un.mutex);
    }
    pthread_mutex_unlock(&un.mutex);
  }

  /* Destroy the mutex and condition variables. */
  pthread_cond_destroy(&un.cond);
  pthread_mutex_destroy(&un.mutex);

  തിരികെ rc;
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
പൂർണ്ണസംഖ്യ sqlite3_blocking_step(sqlite3_stmt *pStmt){
  പൂർണ്ണസംഖ്യ rc;
  ആകുമ്പോൾ( SQLITE_LOCKED==(rc = sqlite3_step(pStmt)) ){
    rc = wait_for_unlock_notify(sqlite3_db_handle(pStmt));
    എങ്കിൽ( rc!=SQLITE_OK ) നിർത്തുക;
    sqlite3_reset(pStmt);
  }
  തിരികെ rc;
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
പൂർണ്ണസംഖ്യ sqlite3_blocking_prepare_v2(
  sqlite3 *db,              /* Database handle. */
  സ്ഥിരം അക്ഷരം *zSql,         /* UTF-8 encoded SQL statement. */
  പൂർണ്ണസംഖ്യ nSql,                 /* Length of zSql in bytes. */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  സ്ഥിരം അക്ഷരം **pz           /* OUT: End of parsed string */
){
  പൂർണ്ണസംഖ്യ rc;
  ആകുമ്പോൾ( SQLITE_LOCKED==(rc = sqlite3_prepare_v2(db, zSql, nSql, ppStmt, pz)) ){
    rc = wait_for_unlock_notify(db);
    എങ്കിൽ( rc!=SQLITE_OK ) നിർത്തുക;
  }
  തിരികെ rc;
}
/* END_SQLITE_BLOCKING_STEP */

/*
** Usage: sqlite3_blocking_step STMT
**
** Advance the statement to the next row.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI blocking_step_proc(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){

  sqlite3_stmt *pStmt;
  പൂർണ്ണസംഖ്യ rc;

  എങ്കിൽ( objc!=2 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "STMT");
    തിരികെ TCL_ERROR;
  }

  pStmt = (sqlite3_stmt*)sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  rc = sqlite3_blocking_step(pStmt);

  Tcl_SetResult(interp, (അക്ഷരം *)sqlite3ErrName(rc), 0);
  തിരികെ TCL_OK;
}

/*
** Usage: sqlite3_blocking_prepare_v2 DB sql bytes ?tailvar?
** Usage: sqlite3_nonblocking_prepare_v2 DB sql bytes ?tailvar?
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI blocking_prepare_v2_proc(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  സ്ഥിരം അക്ഷരം *zSql;
  പൂർണ്ണസംഖ്യ bytes;
  സ്ഥിരം അക്ഷരം *zTail = 0;
  sqlite3_stmt *pStmt = 0;
  അക്ഷരം zBuf[50];
  പൂർണ്ണസംഖ്യ rc;
  പൂർണ്ണസംഖ്യ isBlocking = !(clientData==0);

  എങ്കിൽ( objc!=5 && objc!=4 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", 
       Tcl_GetString(objv[0]), " DB sql bytes tailvar", 0);
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) തിരികെ TCL_ERROR;
  zSql = Tcl_GetString(objv[2]);
  എങ്കിൽ( Tcl_GetIntFromObj(interp, objv[3], &bytes) ) തിരികെ TCL_ERROR;

  എങ്കിൽ( isBlocking ){
    rc = sqlite3_blocking_prepare_v2(db, zSql, bytes, &pStmt, &zTail);
  }അല്ലെങ്കിൽ{
    rc = sqlite3_prepare_v2(db, zSql, bytes, &pStmt, &zTail);
  }

  പ്രതിജ്ഞാ(rc==SQLITE_OK || pStmt==0);
  എങ്കിൽ( zTail && objc>=5 ){
    എങ്കിൽ( bytes>=0 ){
      bytes = bytes - (zTail-zSql);
    }
    Tcl_ObjSetVar2(interp, objv[4], 0, Tcl_NewStringObj(zTail, bytes), 0);
  }
  എങ്കിൽ( rc!=SQLITE_OK ){
    പ്രതിജ്ഞാ( pStmt==0 );
    sqlite3_snprintf(വലുപ്പം(zBuf), zBuf, "%s ", (അക്ഷരം *)sqlite3ErrName(rc));
    Tcl_AppendResult(interp, zBuf, sqlite3_errmsg(db), ശൂന്യ);
    തിരികെ TCL_ERROR;
  }

  എങ്കിൽ( pStmt ){
    എങ്കിൽ( sqlite3TestMakePointerStr(interp, zBuf, pStmt) ) തിരികെ TCL_ERROR;
    Tcl_AppendResult(interp, zBuf, ശൂന്യ);
  }
  തിരികെ TCL_OK;
}

#endif /* SQLITE_OS_UNIX && SQLITE_ENABLE_UNLOCK_NOTIFY */
/*
** End of implementation of [sqlite3_blocking_step].
************************************************************************/

/*
** Register commands with the TCL interpreter.
*/
പൂർണ്ണസംഖ്യ SqlitetestThread_Init(Tcl_Interp *interp){
  ഘടന TclCmd {
    പൂർണ്ണസംഖ്യ (*xProc)(ശൂന്യം*, Tcl_Interp*, പൂർണ്ണസംഖ്യ, Tcl_Obj*സ്ഥിരം*);
    സ്ഥിരം അക്ഷരം *zName;
    പൂർണ്ണസംഖ്യ iCtx;
  } aCmd[] = {
    { sqlthread_proc,           "sqlthread",                      0 },
    { clock_seconds_proc,       "clock_second",                   0 },
    { clock_milliseconds_proc,  "clock_milliseconds",             0 },
#എങ്കിൽ SQLITE_OS_UNIX && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)
    { blocking_step_proc,       "sqlite3_blocking_step",          0 },
    { blocking_prepare_v2_proc, "sqlite3_blocking_prepare_v2",    1 },
    { blocking_prepare_v2_proc, "sqlite3_nonblocking_prepare_v2", 0 },
#endif
  };
  പൂർണ്ണസംഖ്യ ii;

  വേണ്ടി(ii=0; ii<വലുപ്പം(aCmd)/വലുപ്പം(aCmd[0]); ii++){
    ശൂന്യം *p = SQLITE_INT_TO_PTR(aCmd[ii].iCtx);
    Tcl_CreateObjCommand(interp, aCmd[ii].zName, aCmd[ii].xProc, p, 0);
  }
  തിരികെ TCL_OK;
}
#അല്ലെങ്കിൽ
പൂർണ്ണസംഖ്യ SqlitetestThread_Init(Tcl_Interp *interp){
  തിരികെ TCL_OK;
}
#endif
