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

#அந்தர்பாவ "sqliteInt.h"
#அந்தர்பாவ "tclsqlite.h"

#எனில் SQLITE_THREADSAFE

#அந்தர்பாவ <errno.h>

#எனில் !defined(_MSC_VER)
#அந்தர்பாவ <unistd.h>
#endif

/*
** One of these is allocated for each thread created by [sqlthread spawn].
*/
வகைவரை கட்டமைப்பு SqlThread SqlThread;
கட்டமைப்பு SqlThread {
  Tcl_ThreadId parent;     /* Thread id of parent thread */
  Tcl_Interp *interp;      /* Parent interpreter */
  எழுத்து *zScript;           /* The script to execute. */
  எழுத்து *zVarname;          /* Varname in parent script */
};

/*
** A custom Tcl_Event type used by this module. When the event is
** handled, script zScript is evaluated in interpreter interp. If
** the evaluation throws an exception (returns TCL_ERROR), then the
** error is handled by Tcl_BackgroundError(). If no error occurs,
** the result is simply discarded.
*/
வகைவரை கட்டமைப்பு EvalEvent EvalEvent;
கட்டமைப்பு EvalEvent {
  Tcl_Event base;          /* Base class of type Tcl_Event */
  எழுத்து *zScript;           /* The script to execute. */
  Tcl_Interp *interp;      /* The interpreter to execute it in. */
};

நிலையான Tcl_ObjCmdProc sqlthread_proc;
நிலையான Tcl_ObjCmdProc clock_seconds_proc;
#எனில் SQLITE_OS_UNIX && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)
நிலையான Tcl_ObjCmdProc blocking_step_proc;
நிலையான Tcl_ObjCmdProc blocking_prepare_v2_proc;
#endif
முழு Sqlitetest1_Init(Tcl_Interp *);
முழு Sqlite3_Init(Tcl_Interp *);

/* Functions from main.c */
வெளி மாறிலி எழுத்து *sqlite3ErrName(முழு);

/* Functions from test1.c */
வெளி வெறுமை *sqlite3TestTextToPtr(மாறிலி எழுத்து *);
வெளி முழு getDbPointer(Tcl_Interp *, மாறிலி எழுத்து *, sqlite3 **);
வெளி முழு sqlite3TestMakePointerStr(Tcl_Interp *, எழுத்து *, வெறுமை *);
வெளி முழு sqlite3TestErrCode(Tcl_Interp *, sqlite3 *, முழு);

/*
** Handler for events of type EvalEvent.
*/
நிலையான முழு SQLITE_TCLAPI tclScriptEvent(Tcl_Event *evPtr, முழு flags){
  முழு rc;
  EvalEvent *p = (EvalEvent *)evPtr;
  rc = Tcl_Eval(p->interp, p->zScript);
  எனில்( rc!=TCL_OK ){
    Tcl_BackgroundError(p->interp);
  }
  UNUSED_PARAMETER(flags);
  திரும்பு 1;
}

/*
** Register an EvalEvent to evaluate the script pScript in the
** parent interpreter/thread of SqlThread p.
*/
நிலையான வெறுமை postToParent(SqlThread *p, Tcl_Obj *pScript){
  EvalEvent *pEvent;
  எழுத்து *z_செய்தி;
  Tcl_Size n_செய்தி;

  z_செய்தி = Tcl_GetStringFromObj(pScript, &n_செய்தி); 
  pEvent = (EvalEvent *)ckalloc(அளவு(EvalEvent)+n_செய்தி+1);
  pEvent->base.nextPtr = 0;
  pEvent->base.proc = tclScriptEvent;
  pEvent->zScript = (எழுத்து *)&pEvent[1];
  memcpy(pEvent->zScript, z_செய்தி, n_செய்தி+1);
  pEvent->interp = p->interp;

  Tcl_ThreadQueueEvent(p->parent, (Tcl_Event *)pEvent, TCL_QUEUE_TAIL);
  Tcl_ThreadAlert(p->parent);
}

/*
** The main function for threads created with [sqlthread spawn].
*/
நிலையான Tcl_ThreadCreateType tclScriptThread(ClientData pSqlThread){
  Tcl_Interp *interp;
  Tcl_Obj *pRes;
  Tcl_Obj *pList;
  முழு rc;
  SqlThread *p = (SqlThread *)pSqlThread;
  வெளி முழு Sqlitetest_mutex_Init(Tcl_Interp*);

  interp = Tcl_CreateInterp();
  Tcl_CreateObjCommand(interp, "clock_seconds", clock_seconds_proc, 0, 0);
  Tcl_CreateObjCommand(interp, "sqlthread", sqlthread_proc, pSqlThread, 0);
#எனில் SQLITE_OS_UNIX && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)
  Tcl_CreateObjCommand(interp, "sqlite3_blocking_step", blocking_step_proc,0,0);
  Tcl_CreateObjCommand(interp, 
      "sqlite3_blocking_prepare_v2", blocking_prepare_v2_proc, (வெறுமை *)1, 0);
  Tcl_CreateObjCommand(interp, 
      "sqlite3_nonblocking_prepare_v2", blocking_prepare_v2_proc, 0, 0);
#endif
  Sqlitetest1_Init(interp);
  Sqlitetest_mutex_Init(interp);
  Sqlite3_Init(interp);

  rc = Tcl_Eval(interp, p->zScript);
  pRes = Tcl_GetObjResult(interp);
  pList = Tcl_NewObj();
  Tcl_Incr_Ref_மொத்தம்(pList);
  Tcl_Incr_Ref_மொத்தம்(pRes);

  எனில்( rc!=TCL_OK ){
    Tcl_ListObjAppendElement(interp, pList, Tcl_NewStringObj("error", -1));
    Tcl_ListObjAppendElement(interp, pList, pRes);
    postToParent(p, pList);
    Tcl_Decr_Ref_மொத்தம்(pList);
    pList = Tcl_NewObj();
  }

  Tcl_ListObjAppendElement(interp, pList, Tcl_NewStringObj("set", -1));
  Tcl_ListObjAppendElement(interp, pList, Tcl_NewStringObj(p->zVarname, -1));
  Tcl_ListObjAppendElement(interp, pList, pRes);
  postToParent(p, pList);

  ckfree((வெறுமை *)p);
  Tcl_Decr_Ref_மொத்தம்(pList);
  Tcl_Decr_Ref_மொத்தம்(pRes);
  Tcl_DeleteInterp(interp);
  போது( Tcl_DoOneEvent(TCL_ALL_EVENTS|TCL_DONT_WAIT) );
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
நிலையான முழு SQLITE_TCLAPI sqlthread_spawn(
  ClientData clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_ThreadId x;
  SqlThread *pNew;
  முழு rc;

  Tcl_Size nVarname; எழுத்து *zVarname;
  Tcl_Size nScript; எழுத்து *zScript;

  /* Parameters for thread creation */
  மாறிலி முழு n_அடுக்கு = TCL_THREAD_அடுக்கு_DEFAULT;
  மாறிலி முழு flags = TCL_THREAD_NOFLAGS;

  ப்ரதிஜ்ஞா(objc==4);
  UNUSED_PARAMETER(clientData);
  UNUSED_PARAMETER(objc);

  zVarname = Tcl_GetStringFromObj(objv[2], &nVarname);
  zScript = Tcl_GetStringFromObj(objv[3], &nScript);

  pNew = (SqlThread *)ckalloc(அளவு(SqlThread)+nVarname+nScript+2);
  pNew->zVarname = (எழுத்து *)&pNew[1];
  pNew->zScript = (எழுத்து *)&pNew->zVarname[nVarname+1];
  memcpy(pNew->zVarname, zVarname, nVarname+1);
  memcpy(pNew->zScript, zScript, nScript+1);
  pNew->parent = Tcl_GetCurrentThread();
  pNew->interp = interp;

  rc = Tcl_CreateThread(&x, tclScriptThread, (வெறுமை *)pNew, n_அடுக்கு, flags);
  எனில்( rc!=TCL_OK ){
    Tcl_AppendResult(interp, "Error in Tcl_CreateThread()", வெற்று);
    ckfree((எழுத்து *)pNew);
    திரும்பு TCL_ERROR;
  }

  திரும்பு TCL_OK;
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
நிலையான முழு SQLITE_TCLAPI sqlthread_parent(
  ClientData clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  EvalEvent *pEvent;
  எழுத்து *z_செய்தி;
  Tcl_Size n_செய்தி;
  SqlThread *p = (SqlThread *)clientData;

  ப்ரதிஜ்ஞா(objc==3);
  UNUSED_PARAMETER(objc);

  எனில்( p==0 ){
    Tcl_AppendResult(interp, "no parent thread", வெற்று);
    திரும்பு TCL_ERROR;
  }

  z_செய்தி = Tcl_GetStringFromObj(objv[2], &n_செய்தி);
  pEvent = (EvalEvent *)ckalloc(அளவு(EvalEvent)+n_செய்தி+1);
  pEvent->base.nextPtr = 0;
  pEvent->base.proc = tclScriptEvent;
  pEvent->zScript = (எழுத்து *)&pEvent[1];
  memcpy(pEvent->zScript, z_செய்தி, n_செய்தி+1);
  pEvent->interp = p->interp;
  Tcl_ThreadQueueEvent(p->parent, (Tcl_Event *)pEvent, TCL_QUEUE_TAIL);
  Tcl_ThreadAlert(p->parent);

  திரும்பு TCL_OK;
}

நிலையான முழு xBusy(வெறுமை *pArg, முழு nBusy){
  UNUSED_PARAMETER(pArg);
  UNUSED_PARAMETER(nBusy);
  sqlite3_sleep(50);
  திரும்பு 1;             /* Try again... */
}

/*
** sqlthread open
**
**     Open a database handle and return the string representation of
**     the pointer value.
*/
நிலையான முழு SQLITE_TCLAPI sqlthread_open(
  ClientData clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  முழு sqlite3TestMakePointerStr(Tcl_Interp *interp, எழுத்து *zPtr, வெறுமை *p);

  மாறிலி எழுத்து *zFilename;
  sqlite3 *db;
  எழுத்து zBuf[100];
  வெளி முழு Md5_Register(sqlite3*,எழுத்து**,மாறிலி sqlite3_api_routines*);

  UNUSED_PARAMETER(clientData);
  UNUSED_PARAMETER(objc);

  zFilename = Tcl_GetString(objv[2]);
  sqlite3_open(zFilename, &db);
  Md5_Register(db, 0, 0);
  sqlite3_busy_handler(db, xBusy, 0);
  
  எனில்( sqlite3TestMakePointerStr(interp, zBuf, db) ) திரும்பு TCL_ERROR;
  Tcl_AppendResult(interp, zBuf, வெற்று);

  திரும்பு TCL_OK;
}


/*
** sqlthread open
**
**     Return the current thread-id (Tcl_GetCurrentThread()) cast to
**     an integer.
*/
நிலையான முழு SQLITE_TCLAPI sqlthread_id(
  ClientData clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_ThreadId id = Tcl_GetCurrentThread();
  Tcl_SetObjResult(interp, Tcl_NewIntObj(SQLITE_PTR_TO_INT(id)));
  UNUSED_PARAMETER(clientData);
  UNUSED_PARAMETER(objc);
  UNUSED_PARAMETER(objv);
  திரும்பு TCL_OK;
}


/*
** Dispatch routine for the sub-commands of [sqlthread].
*/
நிலையான முழு SQLITE_TCLAPI sqlthread_proc(
  ClientData clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  கட்டமைப்பு SubCommand {
    எழுத்து *zName;
    Tcl_ObjCmdProc *xProc;
    முழு nArg;
    எழுத்து *zUsage;
  } aSub[] = {
    {"parent", sqlthread_parent, 1, "SCRIPT"},
    {"spawn",  sqlthread_spawn,  2, "VARNAME SCRIPT"},
    {"open",   sqlthread_open,   1, "DBNAME"},
    {"id",     sqlthread_id,     0, ""},
    {0, 0, 0}
  };
  கட்டமைப்பு SubCommand *pSub;
  முழு rc;
  முழு iIndex;

  எனில்( objc<2 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "SUB-COMMAND");
    திரும்பு TCL_ERROR;
  }

  rc = Tcl_GetIndexFromObjStruct(
      interp, objv[1], aSub, அளவு(aSub[0]), "sub-command", 0, &iIndex
  );
  எனில்( rc!=TCL_OK ) திரும்பு rc;
  pSub = &aSub[iIndex];

  எனில்( objc<(pSub->nArg+2) ){
    Tcl_Wrong_இலக்கம்_Args(interp, 2, objv, pSub->zUsage);
    திரும்பு TCL_ERROR;
  }

  திரும்பு pSub->xProc(clientData, interp, objc, objv);
}

/*
** The [clock_seconds] command. This is more or less the same as the
** regular tcl [clock seconds], except that it is available in testfixture
** when linked against both Tcl 8.4 and 8.5. Because [clock seconds] is
** implemented as a script in Tcl 8.5, it is not usually available to
** testfixture.
*/ 
நிலையான முழு SQLITE_TCLAPI clock_seconds_proc(
  ClientData clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Time now;
  Tcl_GetTime(&now);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(now.sec));
  UNUSED_PARAMETER(clientData);
  UNUSED_PARAMETER(objc);
  UNUSED_PARAMETER(objv);
  திரும்பு TCL_OK;
}

/*
** The [clock_milliseconds] command. This is more or less the same as the
** regular tcl [clock milliseconds]. 
*/ 
நிலையான முழு SQLITE_TCLAPI clock_milliseconds_proc(
  ClientData clientData,
  Tcl_Interp *interp,
  முழு objc,
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
  திரும்பு TCL_OK;
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
#எனில் SQLITE_OS_UNIX && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)

/* BEGIN_SQLITE_BLOCKING_STEP */
/* This example uses the pthreads API */
#அந்தர்பாவ <pthread.h>

/*
** A pointer to an instance of this structure is passed as the user-context
** pointer when registering for an unlock-notify callback.
*/
வகைவரை கட்டமைப்பு UnlockNotification UnlockNotification;
கட்டமைப்பு UnlockNotification {
  முழு fired;                         /* True after unlock event has occurred */
  pthread_cond_t cond;               /* Condition variable to wait on */
  pthread_mutex_t mutex;             /* Mutex to protect structure */
};

/*
** This function is an unlock-notify callback registered with SQLite.
*/
நிலையான வெறுமை unlock_notify_cb(வெறுமை **apArg, முழு nArg){
  முழு i;
  க்கு(i=0; i<nArg; i++){
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
நிலையான முழு wait_for_unlock_notify(sqlite3 *db){
  முழு rc;
  UnlockNotification un;

  /* Initialize the UnlockNotification structure. */
  un.fired = 0;
  pthread_mutex_init(&un.mutex, 0);
  pthread_cond_init(&un.cond, 0);

  /* Register for an unlock-notify callback. */
  rc = sqlite3_unlock_notify(db, unlock_notify_cb, (வெறுமை *)&un);
  ப்ரதிஜ்ஞா( rc==SQLITE_LOCKED || rc==SQLITE_OK );

  /* The call to sqlite3_unlock_notify() always returns either SQLITE_LOCKED 
  ** or SQLITE_OK. 
  **
  ** If SQLITE_LOCKED was returned, then the system is deadlocked. In this
  ** case this function needs to return SQLITE_LOCKED to the caller so 
  ** that the current transaction can be rolled back. Otherwise, block
  ** until the unlock-notify callback is invoked, then return SQLITE_OK.
  */
  எனில்( rc==SQLITE_OK ){
    pthread_mutex_lock(&un.mutex);
    எனில்( !un.fired ){
      pthread_cond_wait(&un.cond, &un.mutex);
    }
    pthread_mutex_unlock(&un.mutex);
  }

  /* Destroy the mutex and condition variables. */
  pthread_cond_destroy(&un.cond);
  pthread_mutex_destroy(&un.mutex);

  திரும்பு rc;
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
முழு sqlite3_blocking_step(sqlite3_stmt *pStmt){
  முழு rc;
  போது( SQLITE_LOCKED==(rc = sqlite3_step(pStmt)) ){
    rc = wait_for_unlock_notify(sqlite3_db_handle(pStmt));
    எனில்( rc!=SQLITE_OK ) நிறுத்து;
    sqlite3_reset(pStmt);
  }
  திரும்பு rc;
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
முழு sqlite3_blocking_prepare_v2(
  sqlite3 *db,              /* Database handle. */
  மாறிலி எழுத்து *zSql,         /* UTF-8 encoded SQL statement. */
  முழு nSql,                 /* Length of zSql in bytes. */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  மாறிலி எழுத்து **pz           /* OUT: End of parsed string */
){
  முழு rc;
  போது( SQLITE_LOCKED==(rc = sqlite3_prepare_v2(db, zSql, nSql, ppStmt, pz)) ){
    rc = wait_for_unlock_notify(db);
    எனில்( rc!=SQLITE_OK ) நிறுத்து;
  }
  திரும்பு rc;
}
/* END_SQLITE_BLOCKING_STEP */

/*
** Usage: sqlite3_blocking_step STMT
**
** Advance the statement to the next row.
*/
நிலையான முழு SQLITE_TCLAPI blocking_step_proc(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){

  sqlite3_stmt *pStmt;
  முழு rc;

  எனில்( objc!=2 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "STMT");
    திரும்பு TCL_ERROR;
  }

  pStmt = (sqlite3_stmt*)sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  rc = sqlite3_blocking_step(pStmt);

  Tcl_SetResult(interp, (எழுத்து *)sqlite3ErrName(rc), 0);
  திரும்பு TCL_OK;
}

/*
** Usage: sqlite3_blocking_prepare_v2 DB sql bytes ?tailvar?
** Usage: sqlite3_nonblocking_prepare_v2 DB sql bytes ?tailvar?
*/
நிலையான முழு SQLITE_TCLAPI blocking_prepare_v2_proc(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  மாறிலி எழுத்து *zSql;
  முழு bytes;
  மாறிலி எழுத்து *zTail = 0;
  sqlite3_stmt *pStmt = 0;
  எழுத்து zBuf[50];
  முழு rc;
  முழு isBlocking = !(clientData==0);

  எனில்( objc!=5 && objc!=4 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", 
       Tcl_GetString(objv[0]), " DB sql bytes tailvar", 0);
    திரும்பு TCL_ERROR;
  }
  எனில்( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) திரும்பு TCL_ERROR;
  zSql = Tcl_GetString(objv[2]);
  எனில்( Tcl_GetIntFromObj(interp, objv[3], &bytes) ) திரும்பு TCL_ERROR;

  எனில்( isBlocking ){
    rc = sqlite3_blocking_prepare_v2(db, zSql, bytes, &pStmt, &zTail);
  }இல்லையெனில்{
    rc = sqlite3_prepare_v2(db, zSql, bytes, &pStmt, &zTail);
  }

  ப்ரதிஜ்ஞா(rc==SQLITE_OK || pStmt==0);
  எனில்( zTail && objc>=5 ){
    எனில்( bytes>=0 ){
      bytes = bytes - (zTail-zSql);
    }
    Tcl_ObjSetVar2(interp, objv[4], 0, Tcl_NewStringObj(zTail, bytes), 0);
  }
  எனில்( rc!=SQLITE_OK ){
    ப்ரதிஜ்ஞா( pStmt==0 );
    sqlite3_snprintf(அளவு(zBuf), zBuf, "%s ", (எழுத்து *)sqlite3ErrName(rc));
    Tcl_AppendResult(interp, zBuf, sqlite3_errmsg(db), வெற்று);
    திரும்பு TCL_ERROR;
  }

  எனில்( pStmt ){
    எனில்( sqlite3TestMakePointerStr(interp, zBuf, pStmt) ) திரும்பு TCL_ERROR;
    Tcl_AppendResult(interp, zBuf, வெற்று);
  }
  திரும்பு TCL_OK;
}

#endif /* SQLITE_OS_UNIX && SQLITE_ENABLE_UNLOCK_NOTIFY */
/*
** End of implementation of [sqlite3_blocking_step].
************************************************************************/

/*
** Register commands with the TCL interpreter.
*/
முழு SqlitetestThread_Init(Tcl_Interp *interp){
  கட்டமைப்பு TclCmd {
    முழு (*xProc)(வெறுமை*, Tcl_Interp*, முழு, Tcl_Obj*மாறிலி*);
    மாறிலி எழுத்து *zName;
    முழு iCtx;
  } aCmd[] = {
    { sqlthread_proc,           "sqlthread",                      0 },
    { clock_seconds_proc,       "clock_second",                   0 },
    { clock_milliseconds_proc,  "clock_milliseconds",             0 },
#எனில் SQLITE_OS_UNIX && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)
    { blocking_step_proc,       "sqlite3_blocking_step",          0 },
    { blocking_prepare_v2_proc, "sqlite3_blocking_prepare_v2",    1 },
    { blocking_prepare_v2_proc, "sqlite3_nonblocking_prepare_v2", 0 },
#endif
  };
  முழு ii;

  க்கு(ii=0; ii<அளவு(aCmd)/அளவு(aCmd[0]); ii++){
    வெறுமை *p = SQLITE_INT_TO_PTR(aCmd[ii].iCtx);
    Tcl_CreateObjCommand(interp, aCmd[ii].zName, aCmd[ii].xProc, p, 0);
  }
  திரும்பு TCL_OK;
}
#இல்லையெனில்
முழு SqlitetestThread_Init(Tcl_Interp *interp){
  திரும்பு TCL_OK;
}
#endif
