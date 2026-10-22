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

#यदि SQLITE_THREADSAFE

#अन्तर्भाव <errno.h>

#यदि !defined(_MSC_VER)
#अन्तर्भाव <unistd.h>
#endif

/*
** One of these is allocated for each thread created by [sqlthread spawn].
*/
प्रकारनिर्देश संरचना SqlThread SqlThread;
संरचना SqlThread {
  Tcl_ThreadId parent;     /* Thread id of parent thread */
  Tcl_Interp *interp;      /* Parent interpreter */
  वर्ण *zScript;           /* The script to execute. */
  वर्ण *zVarname;          /* Varname in parent script */
};

/*
** A custom Tcl_Event type used by this module. When the event is
** handled, script zScript is evaluated in interpreter interp. If
** the evaluation throws an exception (returns TCL_ERROR), then the
** error is handled by Tcl_BackgroundError(). If no error occurs,
** the result is simply discarded.
*/
प्रकारनिर्देश संरचना EvalEvent EvalEvent;
संरचना EvalEvent {
  Tcl_Event base;          /* Base class of type Tcl_Event */
  वर्ण *zScript;           /* The script to execute. */
  Tcl_Interp *interp;      /* The interpreter to execute it in. */
};

स्थिर Tcl_ObjCmdProc sqlthread_proc;
स्थिर Tcl_ObjCmdProc clock_seconds_proc;
#यदि SQLITE_OS_UNIX && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)
स्थिर Tcl_ObjCmdProc blocking_step_proc;
स्थिर Tcl_ObjCmdProc blocking_prepare_v2_proc;
#endif
पूर्णाङ्क Sqlitetest1_Init(Tcl_Interp *);
पूर्णाङ्क Sqlite3_Init(Tcl_Interp *);

/* Functions from main.c */
बाह्य ध्रुव वर्ण *sqlite3ErrName(पूर्णाङ्क);

/* Functions from test1.c */
बाह्य शून्य *sqlite3TestTextToPtr(ध्रुव वर्ण *);
बाह्य पूर्णाङ्क getDbPointer(Tcl_Interp *, ध्रुव वर्ण *, sqlite3 **);
बाह्य पूर्णाङ्क sqlite3TestMakePointerStr(Tcl_Interp *, वर्ण *, शून्य *);
बाह्य पूर्णाङ्क sqlite3TestErrCode(Tcl_Interp *, sqlite3 *, पूर्णाङ्क);

/*
** Handler for events of type EvalEvent.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI tclScriptEvent(Tcl_Event *evPtr, पूर्णाङ्क flags){
  पूर्णाङ्क rc;
  EvalEvent *p = (EvalEvent *)evPtr;
  rc = Tcl_Eval(p->interp, p->zScript);
  यदि( rc!=TCL_OK ){
    Tcl_BackgroundError(p->interp);
  }
  UNUSED_PARAMETER(flags);
  प्रत्यागम 1;
}

/*
** Register an EvalEvent to evaluate the script pScript in the
** parent interpreter/thread of SqlThread p.
*/
स्थिर शून्य postToParent(SqlThread *p, Tcl_Obj *pScript){
  EvalEvent *pEvent;
  वर्ण *z_संदेश;
  Tcl_Size n_संदेश;

  z_संदेश = Tcl_GetStringFromObj(pScript, &n_संदेश); 
  pEvent = (EvalEvent *)ckalloc(परिमाण(EvalEvent)+n_संदेश+1);
  pEvent->base.nextPtr = 0;
  pEvent->base.proc = tclScriptEvent;
  pEvent->zScript = (वर्ण *)&pEvent[1];
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
  पूर्णाङ्क rc;
  SqlThread *p = (SqlThread *)pSqlThread;
  बाह्य पूर्णाङ्क Sqlitetest_mutex_Init(Tcl_Interp*);

  interp = Tcl_CreateInterp();
  Tcl_CreateObjCommand(interp, "clock_seconds", clock_seconds_proc, 0, 0);
  Tcl_CreateObjCommand(interp, "sqlthread", sqlthread_proc, pSqlThread, 0);
#यदि SQLITE_OS_UNIX && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)
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
  Tcl_Incr_Ref_गिनती(pList);
  Tcl_Incr_Ref_गिनती(pRes);

  यदि( rc!=TCL_OK ){
    Tcl_ListObjAppendElement(interp, pList, Tcl_NewStringObj("error", -1));
    Tcl_ListObjAppendElement(interp, pList, pRes);
    postToParent(p, pList);
    Tcl_Decr_Ref_गिनती(pList);
    pList = Tcl_NewObj();
  }

  Tcl_ListObjAppendElement(interp, pList, Tcl_NewStringObj("set", -1));
  Tcl_ListObjAppendElement(interp, pList, Tcl_NewStringObj(p->zVarname, -1));
  Tcl_ListObjAppendElement(interp, pList, pRes);
  postToParent(p, pList);

  ckfree((शून्य *)p);
  Tcl_Decr_Ref_गिनती(pList);
  Tcl_Decr_Ref_गिनती(pRes);
  Tcl_DeleteInterp(interp);
  यावत्( Tcl_DoOneEvent(TCL_ALL_EVENTS|TCL_DONT_WAIT) );
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
स्थिर पूर्णाङ्क SQLITE_TCLAPI sqlthread_spawn(
  ClientData clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_ThreadId x;
  SqlThread *pNew;
  पूर्णाङ्क rc;

  Tcl_Size nVarname; वर्ण *zVarname;
  Tcl_Size nScript; वर्ण *zScript;

  /* Parameters for thread creation */
  ध्रुव पूर्णाङ्क n_ढेर = TCL_THREAD_ढेर_DEFAULT;
  ध्रुव पूर्णाङ्क flags = TCL_THREAD_NOFLAGS;

  प्रतिज्ञा(objc==4);
  UNUSED_PARAMETER(clientData);
  UNUSED_PARAMETER(objc);

  zVarname = Tcl_GetStringFromObj(objv[2], &nVarname);
  zScript = Tcl_GetStringFromObj(objv[3], &nScript);

  pNew = (SqlThread *)ckalloc(परिमाण(SqlThread)+nVarname+nScript+2);
  pNew->zVarname = (वर्ण *)&pNew[1];
  pNew->zScript = (वर्ण *)&pNew->zVarname[nVarname+1];
  स_नकल(pNew->zVarname, zVarname, nVarname+1);
  स_नकल(pNew->zScript, zScript, nScript+1);
  pNew->parent = Tcl_GetCurrentThread();
  pNew->interp = interp;

  rc = Tcl_CreateThread(&x, tclScriptThread, (शून्य *)pNew, n_ढेर, flags);
  यदि( rc!=TCL_OK ){
    Tcl_AppendResult(interp, "Error in Tcl_CreateThread()", रिक्त);
    ckfree((वर्ण *)pNew);
    प्रत्यागम TCL_ERROR;
  }

  प्रत्यागम TCL_OK;
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
स्थिर पूर्णाङ्क SQLITE_TCLAPI sqlthread_parent(
  ClientData clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  EvalEvent *pEvent;
  वर्ण *z_संदेश;
  Tcl_Size n_संदेश;
  SqlThread *p = (SqlThread *)clientData;

  प्रतिज्ञा(objc==3);
  UNUSED_PARAMETER(objc);

  यदि( p==0 ){
    Tcl_AppendResult(interp, "no parent thread", रिक्त);
    प्रत्यागम TCL_ERROR;
  }

  z_संदेश = Tcl_GetStringFromObj(objv[2], &n_संदेश);
  pEvent = (EvalEvent *)ckalloc(परिमाण(EvalEvent)+n_संदेश+1);
  pEvent->base.nextPtr = 0;
  pEvent->base.proc = tclScriptEvent;
  pEvent->zScript = (वर्ण *)&pEvent[1];
  स_नकल(pEvent->zScript, z_संदेश, n_संदेश+1);
  pEvent->interp = p->interp;
  Tcl_ThreadQueueEvent(p->parent, (Tcl_Event *)pEvent, TCL_QUEUE_TAIL);
  Tcl_ThreadAlert(p->parent);

  प्रत्यागम TCL_OK;
}

स्थिर पूर्णाङ्क xBusy(शून्य *pArg, पूर्णाङ्क nBusy){
  UNUSED_PARAMETER(pArg);
  UNUSED_PARAMETER(nBusy);
  sqlite3_sleep(50);
  प्रत्यागम 1;             /* Try again... */
}

/*
** sqlthread open
**
**     Open a database handle and return the string representation of
**     the pointer value.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI sqlthread_open(
  ClientData clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क sqlite3TestMakePointerStr(Tcl_Interp *interp, वर्ण *zPtr, शून्य *p);

  ध्रुव वर्ण *zFilename;
  sqlite3 *db;
  वर्ण zBuf[100];
  बाह्य पूर्णाङ्क Md5_Register(sqlite3*,वर्ण**,ध्रुव sqlite3_api_routines*);

  UNUSED_PARAMETER(clientData);
  UNUSED_PARAMETER(objc);

  zFilename = Tcl_GetString(objv[2]);
  sqlite3_open(zFilename, &db);
  Md5_Register(db, 0, 0);
  sqlite3_busy_handler(db, xBusy, 0);
  
  यदि( sqlite3TestMakePointerStr(interp, zBuf, db) ) प्रत्यागम TCL_ERROR;
  Tcl_AppendResult(interp, zBuf, रिक्त);

  प्रत्यागम TCL_OK;
}


/*
** sqlthread open
**
**     Return the current thread-id (Tcl_GetCurrentThread()) cast to
**     an integer.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI sqlthread_id(
  ClientData clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_ThreadId id = Tcl_GetCurrentThread();
  Tcl_SetObjResult(interp, Tcl_NewIntObj(SQLITE_PTR_TO_INT(id)));
  UNUSED_PARAMETER(clientData);
  UNUSED_PARAMETER(objc);
  UNUSED_PARAMETER(objv);
  प्रत्यागम TCL_OK;
}


/*
** Dispatch routine for the sub-commands of [sqlthread].
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI sqlthread_proc(
  ClientData clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  संरचना SubCommand {
    वर्ण *zName;
    Tcl_ObjCmdProc *xProc;
    पूर्णाङ्क nArg;
    वर्ण *zUsage;
  } aSub[] = {
    {"parent", sqlthread_parent, 1, "SCRIPT"},
    {"spawn",  sqlthread_spawn,  2, "VARNAME SCRIPT"},
    {"open",   sqlthread_open,   1, "DBNAME"},
    {"id",     sqlthread_id,     0, ""},
    {0, 0, 0}
  };
  संरचना SubCommand *pSub;
  पूर्णाङ्क rc;
  पूर्णाङ्क iIndex;

  यदि( objc<2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "SUB-COMMAND");
    प्रत्यागम TCL_ERROR;
  }

  rc = Tcl_GetIndexFromObjStruct(
      interp, objv[1], aSub, परिमाण(aSub[0]), "sub-command", 0, &iIndex
  );
  यदि( rc!=TCL_OK ) प्रत्यागम rc;
  pSub = &aSub[iIndex];

  यदि( objc<(pSub->nArg+2) ){
    Tcl_Wrong_अंक_Args(interp, 2, objv, pSub->zUsage);
    प्रत्यागम TCL_ERROR;
  }

  प्रत्यागम pSub->xProc(clientData, interp, objc, objv);
}

/*
** The [clock_seconds] command. This is more or less the same as the
** regular tcl [clock seconds], except that it is available in testfixture
** when linked against both Tcl 8.4 and 8.5. Because [clock seconds] is
** implemented as a script in Tcl 8.5, it is not usually available to
** testfixture.
*/ 
स्थिर पूर्णाङ्क SQLITE_TCLAPI clock_seconds_proc(
  ClientData clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Time now;
  Tcl_GetTime(&now);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(now.sec));
  UNUSED_PARAMETER(clientData);
  UNUSED_PARAMETER(objc);
  UNUSED_PARAMETER(objv);
  प्रत्यागम TCL_OK;
}

/*
** The [clock_milliseconds] command. This is more or less the same as the
** regular tcl [clock milliseconds]. 
*/ 
स्थिर पूर्णाङ्क SQLITE_TCLAPI clock_milliseconds_proc(
  ClientData clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
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
  प्रत्यागम TCL_OK;
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
#यदि SQLITE_OS_UNIX && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)

/* BEGIN_SQLITE_BLOCKING_STEP */
/* This example uses the pthreads API */
#अन्तर्भाव <pthread.h>

/*
** A pointer to an instance of this structure is passed as the user-context
** pointer when registering for an unlock-notify callback.
*/
प्रकारनिर्देश संरचना UnlockNotification UnlockNotification;
संरचना UnlockNotification {
  पूर्णाङ्क fired;                         /* True after unlock event has occurred */
  pthread_cond_t cond;               /* Condition variable to wait on */
  pthread_mutex_t mutex;             /* Mutex to protect structure */
};

/*
** This function is an unlock-notify callback registered with SQLite.
*/
स्थिर शून्य unlock_notify_cb(शून्य **apArg, पूर्णाङ्क nArg){
  पूर्णाङ्क i;
  प्रति(i=0; i<nArg; i++){
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
स्थिर पूर्णाङ्क wait_for_unlock_notify(sqlite3 *db){
  पूर्णाङ्क rc;
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
  यदि( rc==SQLITE_OK ){
    pthread_mutex_lock(&un.mutex);
    यदि( !un.fired ){
      pthread_cond_wait(&un.cond, &un.mutex);
    }
    pthread_mutex_unlock(&un.mutex);
  }

  /* Destroy the mutex and condition variables. */
  pthread_cond_destroy(&un.cond);
  pthread_mutex_destroy(&un.mutex);

  प्रत्यागम rc;
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
पूर्णाङ्क sqlite3_blocking_step(sqlite3_stmt *pStmt){
  पूर्णाङ्क rc;
  यावत्( SQLITE_LOCKED==(rc = sqlite3_step(pStmt)) ){
    rc = wait_for_unlock_notify(sqlite3_db_handle(pStmt));
    यदि( rc!=SQLITE_OK ) भङ्ग;
    sqlite3_reset(pStmt);
  }
  प्रत्यागम rc;
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
पूर्णाङ्क sqlite3_blocking_prepare_v2(
  sqlite3 *db,              /* Database handle. */
  ध्रुव वर्ण *zSql,         /* UTF-8 encoded SQL statement. */
  पूर्णाङ्क nSql,                 /* Length of zSql in bytes. */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  ध्रुव वर्ण **pz           /* OUT: End of parsed string */
){
  पूर्णाङ्क rc;
  यावत्( SQLITE_LOCKED==(rc = sqlite3_prepare_v2(db, zSql, nSql, ppStmt, pz)) ){
    rc = wait_for_unlock_notify(db);
    यदि( rc!=SQLITE_OK ) भङ्ग;
  }
  प्रत्यागम rc;
}
/* END_SQLITE_BLOCKING_STEP */

/*
** Usage: sqlite3_blocking_step STMT
**
** Advance the statement to the next row.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI blocking_step_proc(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){

  sqlite3_stmt *pStmt;
  पूर्णाङ्क rc;

  यदि( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "STMT");
    प्रत्यागम TCL_ERROR;
  }

  pStmt = (sqlite3_stmt*)sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  rc = sqlite3_blocking_step(pStmt);

  Tcl_SetResult(interp, (वर्ण *)sqlite3ErrName(rc), 0);
  प्रत्यागम TCL_OK;
}

/*
** Usage: sqlite3_blocking_prepare_v2 DB sql bytes ?tailvar?
** Usage: sqlite3_nonblocking_prepare_v2 DB sql bytes ?tailvar?
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI blocking_prepare_v2_proc(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  ध्रुव वर्ण *zSql;
  पूर्णाङ्क bytes;
  ध्रुव वर्ण *zTail = 0;
  sqlite3_stmt *pStmt = 0;
  वर्ण zBuf[50];
  पूर्णाङ्क rc;
  पूर्णाङ्क isBlocking = !(clientData==0);

  यदि( objc!=5 && objc!=4 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", 
       Tcl_GetString(objv[0]), " DB sql bytes tailvar", 0);
    प्रत्यागम TCL_ERROR;
  }
  यदि( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) प्रत्यागम TCL_ERROR;
  zSql = Tcl_GetString(objv[2]);
  यदि( Tcl_GetIntFromObj(interp, objv[3], &bytes) ) प्रत्यागम TCL_ERROR;

  यदि( isBlocking ){
    rc = sqlite3_blocking_prepare_v2(db, zSql, bytes, &pStmt, &zTail);
  }अन्यथा{
    rc = sqlite3_prepare_v2(db, zSql, bytes, &pStmt, &zTail);
  }

  प्रतिज्ञा(rc==SQLITE_OK || pStmt==0);
  यदि( zTail && objc>=5 ){
    यदि( bytes>=0 ){
      bytes = bytes - (zTail-zSql);
    }
    Tcl_ObjSetVar2(interp, objv[4], 0, Tcl_NewStringObj(zTail, bytes), 0);
  }
  यदि( rc!=SQLITE_OK ){
    प्रतिज्ञा( pStmt==0 );
    sqlite3_snprintf(परिमाण(zBuf), zBuf, "%s ", (वर्ण *)sqlite3ErrName(rc));
    Tcl_AppendResult(interp, zBuf, sqlite3_errmsg(db), रिक्त);
    प्रत्यागम TCL_ERROR;
  }

  यदि( pStmt ){
    यदि( sqlite3TestMakePointerStr(interp, zBuf, pStmt) ) प्रत्यागम TCL_ERROR;
    Tcl_AppendResult(interp, zBuf, रिक्त);
  }
  प्रत्यागम TCL_OK;
}

#endif /* SQLITE_OS_UNIX && SQLITE_ENABLE_UNLOCK_NOTIFY */
/*
** End of implementation of [sqlite3_blocking_step].
************************************************************************/

/*
** Register commands with the TCL interpreter.
*/
पूर्णाङ्क SqlitetestThread_Init(Tcl_Interp *interp){
  संरचना TclCmd {
    पूर्णाङ्क (*xProc)(शून्य*, Tcl_Interp*, पूर्णाङ्क, Tcl_Obj*ध्रुव*);
    ध्रुव वर्ण *zName;
    पूर्णाङ्क iCtx;
  } aCmd[] = {
    { sqlthread_proc,           "sqlthread",                      0 },
    { clock_seconds_proc,       "clock_second",                   0 },
    { clock_milliseconds_proc,  "clock_milliseconds",             0 },
#यदि SQLITE_OS_UNIX && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)
    { blocking_step_proc,       "sqlite3_blocking_step",          0 },
    { blocking_prepare_v2_proc, "sqlite3_blocking_prepare_v2",    1 },
    { blocking_prepare_v2_proc, "sqlite3_nonblocking_prepare_v2", 0 },
#endif
  };
  पूर्णाङ्क ii;

  प्रति(ii=0; ii<परिमाण(aCmd)/परिमाण(aCmd[0]); ii++){
    शून्य *p = SQLITE_INT_TO_PTR(aCmd[ii].iCtx);
    Tcl_CreateObjCommand(interp, aCmd[ii].zName, aCmd[ii].xProc, p, 0);
  }
  प्रत्यागम TCL_OK;
}
#अन्यथा
पूर्णाङ्क SqlitetestThread_Init(Tcl_Interp *interp){
  प्रत्यागम TCL_OK;
}
#endif
