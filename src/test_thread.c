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

#समावेश "sqliteInt.h"
#समावेश "tclsqlite.h"

#अगर SQLITE_THREADSAFE

#समावेश <errno.h>

#अगर !defined(_MSC_VER)
#समावेश <unistd.h>
#पूर्ण_अगर

/*
** One of these is allocated for each thread created by [sqlthread spawn].
*/
प्रकार काष्ठा SqlThread SqlThread;
काष्ठा SqlThread {
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
प्रकार काष्ठा EvalEvent EvalEvent;
काष्ठा EvalEvent {
  Tcl_Event base;          /* Base class of type Tcl_Event */
  अक्षर *zScript;           /* The script to execute. */
  Tcl_Interp *interp;      /* The interpreter to execute it in. */
};

अटल Tcl_ObjCmdProc sqlthread_proc;
अटल Tcl_ObjCmdProc clock_seconds_proc;
#अगर SQLITE_OS_UNIX && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)
अटल Tcl_ObjCmdProc blocking_step_proc;
अटल Tcl_ObjCmdProc blocking_prepare_v2_proc;
#पूर्ण_अगर
पूर्णांक Sqlitetest1_Init(Tcl_Interp *);
पूर्णांक Sqlite3_Init(Tcl_Interp *);

/* Functions from main.c */
बाह्य स्थिर अक्षर *sqlite3ErrName(पूर्णांक);

/* Functions from test1.c */
बाह्य व्योम *sqlite3TestTextToPtr(स्थिर अक्षर *);
बाह्य पूर्णांक getDbPointer(Tcl_Interp *, स्थिर अक्षर *, sqlite3 **);
बाह्य पूर्णांक sqlite3TestMakePointerStr(Tcl_Interp *, अक्षर *, व्योम *);
बाह्य पूर्णांक sqlite3TestErrCode(Tcl_Interp *, sqlite3 *, पूर्णांक);

/*
** Handler for events of type EvalEvent.
*/
अटल पूर्णांक SQLITE_TCLAPI tclScriptEvent(Tcl_Event *evPtr, पूर्णांक flags){
  पूर्णांक rc;
  EvalEvent *p = (EvalEvent *)evPtr;
  rc = Tcl_Eval(p->interp, p->zScript);
  अगर( rc!=TCL_OK ){
    Tcl_BackgroundError(p->interp);
  }
  UNUSED_PARAMETER(flags);
  वापस 1;
}

/*
** Register an EvalEvent to evaluate the script pScript in the
** parent interpreter/thread of SqlThread p.
*/
अटल व्योम postToParent(SqlThread *p, Tcl_Obj *pScript){
  EvalEvent *pEvent;
  अक्षर *z_संदेश;
  Tcl_आकार n_संदेश;

  z_संदेश = Tcl_GetStringFromObj(pScript, &n_संदेश); 
  pEvent = (EvalEvent *)ckalloc(माप(EvalEvent)+n_संदेश+1);
  pEvent->base.अग्र_Ptr = 0;
  pEvent->base.proc = tclScriptEvent;
  pEvent->zScript = (अक्षर *)&pEvent[1];
  स_नकल(pEvent->zScript, z_संदेश, n_संदेश+1);
  pEvent->interp = p->interp;

  Tcl_ThreadQueueEvent(p->parent, (Tcl_Event *)pEvent, TCL_QUEUE_पुच्छ);
  Tcl_ThreadAlert(p->parent);
}

/*
** The main function for threads created with [sqlthread spawn].
*/
अटल Tcl_ThreadCreateType tclScriptThread(Client_तथ्य pSqlThread){
  Tcl_Interp *interp;
  Tcl_Obj *pRes;
  Tcl_Obj *pList;
  पूर्णांक rc;
  SqlThread *p = (SqlThread *)pSqlThread;
  बाह्य पूर्णांक Sqlitetest_mutex_Init(Tcl_Interp*);

  interp = Tcl_CreateInterp();
  Tcl_CreateObjCommand(interp, "clock_seconds", clock_seconds_proc, 0, 0);
  Tcl_CreateObjCommand(interp, "sqlthread", sqlthread_proc, pSqlThread, 0);
#अगर SQLITE_OS_UNIX && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)
  Tcl_CreateObjCommand(interp, "sqlite3_blocking_step", blocking_step_proc,0,0);
  Tcl_CreateObjCommand(interp, 
      "sqlite3_blocking_prepare_v2", blocking_prepare_v2_proc, (व्योम *)1, 0);
  Tcl_CreateObjCommand(interp, 
      "sqlite3_nonblocking_prepare_v2", blocking_prepare_v2_proc, 0, 0);
#पूर्ण_अगर
  Sqlitetest1_Init(interp);
  Sqlitetest_mutex_Init(interp);
  Sqlite3_Init(interp);

  rc = Tcl_Eval(interp, p->zScript);
  pRes = Tcl_Get_Obj_परिणाम(interp);
  pList = Tcl_NewObj();
  Tcl_Incr_Ref_गिनती(pList);
  Tcl_Incr_Ref_गिनती(pRes);

  अगर( rc!=TCL_OK ){
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

  ckfree((व्योम *)p);
  Tcl_Decr_Ref_गिनती(pList);
  Tcl_Decr_Ref_गिनती(pRes);
  Tcl_DeleteInterp(interp);
  जबतक( Tcl_DoOneEvent(TCL_ALL_EVENTS|TCL_DONT_WAIT) );
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
अटल पूर्णांक SQLITE_TCLAPI sqlthread_spawn(
  Client_तथ्य client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_ThreadId x;
  SqlThread *pNew;
  पूर्णांक rc;

  Tcl_आकार nVarname; अक्षर *zVarname;
  Tcl_आकार nScript; अक्षर *zScript;

  /* Parameters for thread creation */
  स्थिर पूर्णांक n_ढेर = TCL_THREAD_ढेर_DEFAULT;
  स्थिर पूर्णांक flags = TCL_THREAD_NOFLAGS;

  निश्चित(objc==4);
  UNUSED_PARAMETER(client_तथ्य);
  UNUSED_PARAMETER(objc);

  zVarname = Tcl_GetStringFromObj(objv[2], &nVarname);
  zScript = Tcl_GetStringFromObj(objv[3], &nScript);

  pNew = (SqlThread *)ckalloc(माप(SqlThread)+nVarname+nScript+2);
  pNew->zVarname = (अक्षर *)&pNew[1];
  pNew->zScript = (अक्षर *)&pNew->zVarname[nVarname+1];
  स_नकल(pNew->zVarname, zVarname, nVarname+1);
  स_नकल(pNew->zScript, zScript, nScript+1);
  pNew->parent = Tcl_GetCurrentThread();
  pNew->interp = interp;

  rc = Tcl_CreateThread(&x, tclScriptThread, (व्योम *)pNew, n_ढेर, flags);
  अगर( rc!=TCL_OK ){
    Tcl_Append_परिणाम(interp, "Error in Tcl_CreateThread()", शून्य);
    ckfree((अक्षर *)pNew);
    वापस TCL_ERROR;
  }

  वापस TCL_OK;
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
अटल पूर्णांक SQLITE_TCLAPI sqlthread_parent(
  Client_तथ्य client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  EvalEvent *pEvent;
  अक्षर *z_संदेश;
  Tcl_आकार n_संदेश;
  SqlThread *p = (SqlThread *)client_तथ्य;

  निश्चित(objc==3);
  UNUSED_PARAMETER(objc);

  अगर( p==0 ){
    Tcl_Append_परिणाम(interp, "no parent thread", शून्य);
    वापस TCL_ERROR;
  }

  z_संदेश = Tcl_GetStringFromObj(objv[2], &n_संदेश);
  pEvent = (EvalEvent *)ckalloc(माप(EvalEvent)+n_संदेश+1);
  pEvent->base.अग्र_Ptr = 0;
  pEvent->base.proc = tclScriptEvent;
  pEvent->zScript = (अक्षर *)&pEvent[1];
  स_नकल(pEvent->zScript, z_संदेश, n_संदेश+1);
  pEvent->interp = p->interp;
  Tcl_ThreadQueueEvent(p->parent, (Tcl_Event *)pEvent, TCL_QUEUE_पुच्छ);
  Tcl_ThreadAlert(p->parent);

  वापस TCL_OK;
}

अटल पूर्णांक xBusy(व्योम *pArg, पूर्णांक nBusy){
  UNUSED_PARAMETER(pArg);
  UNUSED_PARAMETER(nBusy);
  sqlite3_sleep(50);
  वापस 1;             /* Try again... */
}

/*
** sqlthread open
**
**     Open a database handle and return the string representation of
**     the pointer value.
*/
अटल पूर्णांक SQLITE_TCLAPI sqlthread_open(
  Client_तथ्य client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक sqlite3TestMakePointerStr(Tcl_Interp *interp, अक्षर *zPtr, व्योम *p);

  स्थिर अक्षर *zFilename;
  sqlite3 *db;
  अक्षर zBuf[100];
  बाह्य पूर्णांक Md5_Register(sqlite3*,अक्षर**,स्थिर sqlite3_api_routines*);

  UNUSED_PARAMETER(client_तथ्य);
  UNUSED_PARAMETER(objc);

  zFilename = Tcl_GetString(objv[2]);
  sqlite3_open(zFilename, &db);
  Md5_Register(db, 0, 0);
  sqlite3_busy_handler(db, xBusy, 0);
  
  अगर( sqlite3TestMakePointerStr(interp, zBuf, db) ) वापस TCL_ERROR;
  Tcl_Append_परिणाम(interp, zBuf, शून्य);

  वापस TCL_OK;
}


/*
** sqlthread open
**
**     Return the current thread-id (Tcl_GetCurrentThread()) cast to
**     an integer.
*/
अटल पूर्णांक SQLITE_TCLAPI sqlthread_id(
  Client_तथ्य client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_ThreadId id = Tcl_GetCurrentThread();
  Tcl_Set_Obj_परिणाम(interp, Tcl_NewIntObj(SQLITE_PTR_TO_INT(id)));
  UNUSED_PARAMETER(client_तथ्य);
  UNUSED_PARAMETER(objc);
  UNUSED_PARAMETER(objv);
  वापस TCL_OK;
}


/*
** Dispatch routine for the sub-commands of [sqlthread].
*/
अटल पूर्णांक SQLITE_TCLAPI sqlthread_proc(
  Client_तथ्य client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  काष्ठा SubCommand {
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
  काष्ठा SubCommand *pSub;
  पूर्णांक rc;
  पूर्णांक i_सूचकांक;

  अगर( objc<2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "SUB-COMMAND");
    वापस TCL_ERROR;
  }

  rc = Tcl_Get_सूचकांक_From_Obj_Struct(
      interp, objv[1], aSub, माप(aSub[0]), "sub-command", 0, &i_सूचकांक
  );
  अगर( rc!=TCL_OK ) वापस rc;
  pSub = &aSub[i_सूचकांक];

  अगर( objc<(pSub->nArg+2) ){
    Tcl_Wrong_अंक_Args(interp, 2, objv, pSub->zUsage);
    वापस TCL_ERROR;
  }

  वापस pSub->xProc(client_तथ्य, interp, objc, objv);
}

/*
** The [clock_seconds] command. This is more or less the same as the
** regular tcl [clock seconds], except that it is available in testfixture
** when linked against both Tcl 8.4 and 8.5. Because [clock seconds] is
** implemented as a script in Tcl 8.5, it is not usually available to
** testfixture.
*/ 
अटल पूर्णांक SQLITE_TCLAPI clock_seconds_proc(
  Client_तथ्य client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Time now;
  Tcl_GetTime(&now);
  Tcl_Set_Obj_परिणाम(interp, Tcl_NewIntObj(now.sec));
  UNUSED_PARAMETER(client_तथ्य);
  UNUSED_PARAMETER(objc);
  UNUSED_PARAMETER(objv);
  वापस TCL_OK;
}

/*
** The [clock_milliseconds] command. This is more or less the same as the
** regular tcl [clock milliseconds]. 
*/ 
अटल पूर्णांक SQLITE_TCLAPI clock_milliseconds_proc(
  Client_तथ्य client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Time now;
  Tcl_GetTime(&now);
  Tcl_Set_Obj_परिणाम(interp, Tcl_NewWideIntObj(
    ((Tcl_WideInt)now.sec * 1000) + (now.usec / 1000)
  ));
  UNUSED_PARAMETER(client_तथ्य);
  UNUSED_PARAMETER(objc);
  UNUSED_PARAMETER(objv);
  वापस TCL_OK;
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
#अगर SQLITE_OS_UNIX && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)

/* BEGIN_SQLITE_BLOCKING_STEP */
/* This example uses the pthreads API */
#समावेश <pthread.h>

/*
** A pointer to an instance of this structure is passed as the user-context
** pointer when registering for an unlock-notify callback.
*/
प्रकार काष्ठा UnlockNotification UnlockNotification;
काष्ठा UnlockNotification {
  पूर्णांक fired;                         /* True after unlock event has occurred */
  pthread_cond_t cond;               /* Condition variable to wait on */
  pthread_mutex_t mutex;             /* Mutex to protect structure */
};

/*
** This function is an unlock-notify callback registered with SQLite.
*/
अटल व्योम unlock_notify_cb(व्योम **apArg, पूर्णांक nArg){
  पूर्णांक i;
  क्रम(i=0; i<nArg; i++){
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
अटल पूर्णांक wait_for_unlock_notify(sqlite3 *db){
  पूर्णांक rc;
  UnlockNotification un;

  /* Initialize the UnlockNotification structure. */
  un.fired = 0;
  pthread_mutex_init(&un.mutex, 0);
  pthread_cond_init(&un.cond, 0);

  /* Register for an unlock-notify callback. */
  rc = sqlite3_unlock_notify(db, unlock_notify_cb, (व्योम *)&un);
  निश्चित( rc==SQLITE_LOCKED || rc==SQLITE_OK );

  /* The call to sqlite3_unlock_notify() always returns either SQLITE_LOCKED 
  ** or SQLITE_OK. 
  **
  ** If SQLITE_LOCKED was returned, then the system is deadlocked. In this
  ** case this function needs to return SQLITE_LOCKED to the caller so 
  ** that the current transaction can be rolled back. Otherwise, block
  ** until the unlock-notify callback is invoked, then return SQLITE_OK.
  */
  अगर( rc==SQLITE_OK ){
    pthread_mutex_lock(&un.mutex);
    अगर( !un.fired ){
      pthread_cond_wait(&un.cond, &un.mutex);
    }
    pthread_mutex_unlock(&un.mutex);
  }

  /* Destroy the mutex and condition variables. */
  pthread_cond_destroy(&un.cond);
  pthread_mutex_destroy(&un.mutex);

  वापस rc;
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
  जबतक( SQLITE_LOCKED==(rc = sqlite3_step(pStmt)) ){
    rc = wait_for_unlock_notify(sqlite3_db_handle(pStmt));
    अगर( rc!=SQLITE_OK ) अवरोध;
    sqlite3_reset(pStmt);
  }
  वापस rc;
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
  स्थिर अक्षर *zSql,         /* UTF-8 encoded SQL statement. */
  पूर्णांक nSql,                 /* Length of zSql in bytes. */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  स्थिर अक्षर **pz           /* OUT: End of parsed string */
){
  पूर्णांक rc;
  जबतक( SQLITE_LOCKED==(rc = sqlite3_prepare_v2(db, zSql, nSql, ppStmt, pz)) ){
    rc = wait_for_unlock_notify(db);
    अगर( rc!=SQLITE_OK ) अवरोध;
  }
  वापस rc;
}
/* END_SQLITE_BLOCKING_STEP */

/*
** Usage: sqlite3_blocking_step STMT
**
** Advance the statement to the next row.
*/
अटल पूर्णांक SQLITE_TCLAPI blocking_step_proc(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){

  sqlite3_stmt *pStmt;
  पूर्णांक rc;

  अगर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "STMT");
    वापस TCL_ERROR;
  }

  pStmt = (sqlite3_stmt*)sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  rc = sqlite3_blocking_step(pStmt);

  Tcl_Set_परिणाम(interp, (अक्षर *)sqlite3ErrName(rc), 0);
  वापस TCL_OK;
}

/*
** Usage: sqlite3_blocking_prepare_v2 DB sql bytes ?tailvar?
** Usage: sqlite3_nonblocking_prepare_v2 DB sql bytes ?tailvar?
*/
अटल पूर्णांक SQLITE_TCLAPI blocking_prepare_v2_proc(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  स्थिर अक्षर *zSql;
  पूर्णांक bytes;
  स्थिर अक्षर *z_पुच्छ = 0;
  sqlite3_stmt *pStmt = 0;
  अक्षर zBuf[50];
  पूर्णांक rc;
  पूर्णांक isBlocking = !(client_तथ्य==0);

  अगर( objc!=5 && objc!=4 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", 
       Tcl_GetString(objv[0]), " DB sql bytes tailvar", 0);
    वापस TCL_ERROR;
  }
  अगर( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) वापस TCL_ERROR;
  zSql = Tcl_GetString(objv[2]);
  अगर( Tcl_GetIntFromObj(interp, objv[3], &bytes) ) वापस TCL_ERROR;

  अगर( isBlocking ){
    rc = sqlite3_blocking_prepare_v2(db, zSql, bytes, &pStmt, &z_पुच्छ);
  }अन्यथा{
    rc = sqlite3_prepare_v2(db, zSql, bytes, &pStmt, &z_पुच्छ);
  }

  निश्चित(rc==SQLITE_OK || pStmt==0);
  अगर( z_पुच्छ && objc>=5 ){
    अगर( bytes>=0 ){
      bytes = bytes - (z_पुच्छ-zSql);
    }
    Tcl_ObjSetVar2(interp, objv[4], 0, Tcl_NewStringObj(z_पुच्छ, bytes), 0);
  }
  अगर( rc!=SQLITE_OK ){
    निश्चित( pStmt==0 );
    sqlite3_snprintf(माप(zBuf), zBuf, "%s ", (अक्षर *)sqlite3ErrName(rc));
    Tcl_Append_परिणाम(interp, zBuf, sqlite3_errmsg(db), शून्य);
    वापस TCL_ERROR;
  }

  अगर( pStmt ){
    अगर( sqlite3TestMakePointerStr(interp, zBuf, pStmt) ) वापस TCL_ERROR;
    Tcl_Append_परिणाम(interp, zBuf, शून्य);
  }
  वापस TCL_OK;
}

#पूर्ण_अगर /* SQLITE_OS_UNIX && SQLITE_ENABLE_UNLOCK_NOTIFY */
/*
** End of implementation of [sqlite3_blocking_step].
************************************************************************/

/*
** Register commands with the TCL interpreter.
*/
पूर्णांक SqlitetestThread_Init(Tcl_Interp *interp){
  काष्ठा TclCmd {
    पूर्णांक (*xProc)(व्योम*, Tcl_Interp*, पूर्णांक, Tcl_Obj*स्थिर*);
    स्थिर अक्षर *zName;
    पूर्णांक iCtx;
  } aCmd[] = {
    { sqlthread_proc,           "sqlthread",                      0 },
    { clock_seconds_proc,       "clock_second",                   0 },
    { clock_milliseconds_proc,  "clock_milliseconds",             0 },
#अगर SQLITE_OS_UNIX && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)
    { blocking_step_proc,       "sqlite3_blocking_step",          0 },
    { blocking_prepare_v2_proc, "sqlite3_blocking_prepare_v2",    1 },
    { blocking_prepare_v2_proc, "sqlite3_nonblocking_prepare_v2", 0 },
#पूर्ण_अगर
  };
  पूर्णांक ii;

  क्रम(ii=0; ii<माप(aCmd)/माप(aCmd[0]); ii++){
    व्योम *p = SQLITE_INT_TO_PTR(aCmd[ii].iCtx);
    Tcl_CreateObjCommand(interp, aCmd[ii].zName, aCmd[ii].xProc, p, 0);
  }
  वापस TCL_OK;
}
#अन्यथा
पूर्णांक SqlitetestThread_Init(Tcl_Interp *interp){
  वापस TCL_OK;
}
#पूर्ण_अगर
