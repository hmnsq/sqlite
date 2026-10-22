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
#اگر_ختم

/*
** One of these is allocated for each thread created by [sqlthread spawn].
*/
قسمتعریف ساخت SqlThread SqlThread;
ساخت SqlThread {
  Tcl_ThreadId والد;     /* Thread id of parent thread */
  Tcl_Interp *interp;      /* Parent interpreter */
  حرف *zScript;           /* The script to execute. */
  حرف *zVarname;          /* Varname in parent script */
};

/*
** A custom Tcl_Event type used by this module. When the event is
** handled, script zScript is evaluated in interpreter interp. If
** the evaluation throws an exception (returns TCL_ERROR), then the
** error is handled by Tcl_BackgroundError(). If no error occurs,
** the result is simply discarded.
*/
قسمتعریف ساخت EvalEvent EvalEvent;
ساخت EvalEvent {
  Tcl_Event بنیاد;          /* Base class of type Tcl_Event */
  حرف *zScript;           /* The script to execute. */
  Tcl_Interp *interp;      /* The interpreter to execute it in. */
};

ساکن Tcl_ObjCmdProc sqlthread_proc;
ساکن Tcl_ObjCmdProc clock_seconds_proc;
#اگر SQLITE_OS_UNIX && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)
ساکن Tcl_ObjCmdProc blocking_قدم_proc;
ساکن Tcl_ObjCmdProc blocking_prepare_v2_proc;
#اگر_ختم
صحیح Sqlitetest1_Init(Tcl_Interp *);
صحیح Sqlite3_Init(Tcl_Interp *);

/* Functions from main.c */
خارجی ثابت حرف *sqlite3ErrName(صحیح);

/* Functions from test1.c */
خارجی خالی *sqlite3_Test_متن_مقصد_Ptr(ثابت حرف *);
خارجی صحیح get_Db_اشارہ_گر(Tcl_Interp *, ثابت حرف *, sqlite3 **);
خارجی صحیح sqlite3_Test_Make_اشارہ_گر_Str(Tcl_Interp *, حرف *, خالی *);
خارجی صحیح sqlite3_Test_Err_رمز(Tcl_Interp *, sqlite3 *, صحیح);

/*
** Handler for events of type EvalEvent.
*/
ساکن صحیح SQLITE_TCLAPI tclScriptEvent(Tcl_Event *evPtr, صحیح flags){
  صحیح rc;
  EvalEvent *p = (EvalEvent *)evPtr;
  rc = Tcl_Eval(p->interp, p->zScript);
  اگر( rc!=TCL_OK ){
    Tcl_BackgroundError(p->interp);
  }
  غیر_مستعمل_PARAMETER(flags);
  واپس 1;
}

/*
** Register an EvalEvent to evaluate the script pScript in the
** parent interpreter/thread of SqlThread p.
*/
ساکن خالی post_مقصد_والد(SqlThread *p, Tcl_Obj *pScript){
  EvalEvent *pEvent;
  حرف *z_پیغام;
  Tcl_سائز n_پیغام;

  z_پیغام = Tcl_GetStringFromObj(pScript, &n_پیغام); 
  pEvent = (EvalEvent *)ckalloc(حجم(EvalEvent)+n_پیغام+1);
  pEvent->بنیاد.اگلا_Ptr = 0;
  pEvent->بنیاد.proc = tclScriptEvent;
  pEvent->zScript = (حرف *)&pEvent[1];
  حافظہ_نقل(pEvent->zScript, z_پیغام, n_پیغام+1);
  pEvent->interp = p->interp;

  Tcl_Thread_قطار_Event(p->والد, (Tcl_Event *)pEvent, TCL_قطار_دم);
  Tcl_ThreadAlert(p->والد);
}

/*
** The main function for threads created with [sqlthread spawn].
*/
ساکن Tcl_ThreadCreateType tclScriptThread(Client_ڈیٹا pSqlThread){
  Tcl_Interp *interp;
  Tcl_Obj *pRes;
  Tcl_Obj *pList;
  صحیح rc;
  SqlThread *p = (SqlThread *)pSqlThread;
  خارجی صحیح Sqlitetest_mutex_Init(Tcl_Interp*);

  interp = Tcl_CreateInterp();
  Tcl_CreateObjCommand(interp, "clock_seconds", clock_seconds_proc, 0, 0);
  Tcl_CreateObjCommand(interp, "sqlthread", sqlthread_proc, pSqlThread, 0);
#اگر SQLITE_OS_UNIX && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)
  Tcl_CreateObjCommand(interp, "sqlite3_blocking_step", blocking_قدم_proc,0,0);
  Tcl_CreateObjCommand(interp, 
      "sqlite3_blocking_prepare_v2", blocking_prepare_v2_proc, (خالی *)1, 0);
  Tcl_CreateObjCommand(interp, 
      "sqlite3_nonblocking_prepare_v2", blocking_prepare_v2_proc, 0, 0);
#اگر_ختم
  Sqlitetest1_Init(interp);
  Sqlitetest_mutex_Init(interp);
  Sqlite3_Init(interp);

  rc = Tcl_Eval(interp, p->zScript);
  pRes = Tcl_Get_Obj_نتیجہ(interp);
  pList = Tcl_NewObj();
  Tcl_Incr_Ref_گنتی(pList);
  Tcl_Incr_Ref_گنتی(pRes);

  اگر( rc!=TCL_OK ){
    Tcl_ListObjAppendElement(interp, pList, Tcl_NewStringObj("error", -1));
    Tcl_ListObjAppendElement(interp, pList, pRes);
    post_مقصد_والد(p, pList);
    Tcl_Decr_Ref_گنتی(pList);
    pList = Tcl_NewObj();
  }

  Tcl_ListObjAppendElement(interp, pList, Tcl_NewStringObj("set", -1));
  Tcl_ListObjAppendElement(interp, pList, Tcl_NewStringObj(p->zVarname, -1));
  Tcl_ListObjAppendElement(interp, pList, pRes);
  post_مقصد_والد(p, pList);

  ckfree((خالی *)p);
  Tcl_Decr_Ref_گنتی(pList);
  Tcl_Decr_Ref_گنتی(pRes);
  Tcl_DeleteInterp(interp);
  جب( Tcl_DoOneEvent(TCL_ALL_EVENTS|TCL_DONT_WAIT) );
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
ساکن صحیح SQLITE_TCLAPI sqlthread_spawn(
  Client_ڈیٹا client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_ThreadId x;
  SqlThread *pNew;
  صحیح rc;

  Tcl_سائز nVarname; حرف *zVarname;
  Tcl_سائز nScript; حرف *zScript;

  /* Parameters for thread creation */
  ثابت صحیح n_ڈھیر_2 = TCL_THREAD_ڈھیر_DEFAULT;
  ثابت صحیح flags = TCL_THREAD_NOFLAGS;

  أكد(objc==4);
  غیر_مستعمل_PARAMETER(client_ڈیٹا);
  غیر_مستعمل_PARAMETER(objc);

  zVarname = Tcl_GetStringFromObj(objv[2], &nVarname);
  zScript = Tcl_GetStringFromObj(objv[3], &nScript);

  pNew = (SqlThread *)ckalloc(حجم(SqlThread)+nVarname+nScript+2);
  pNew->zVarname = (حرف *)&pNew[1];
  pNew->zScript = (حرف *)&pNew->zVarname[nVarname+1];
  حافظہ_نقل(pNew->zVarname, zVarname, nVarname+1);
  حافظہ_نقل(pNew->zScript, zScript, nScript+1);
  pNew->والد = Tcl_GetCurrentThread();
  pNew->interp = interp;

  rc = Tcl_CreateThread(&x, tclScriptThread, (خالی *)pNew, n_ڈھیر_2, flags);
  اگر( rc!=TCL_OK ){
    Tcl_Append_نتیجہ(interp, "Error in Tcl_CreateThread()", تہی);
    ckfree((حرف *)pNew);
    واپس TCL_ERROR;
  }

  واپس TCL_OK;
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
ساکن صحیح SQLITE_TCLAPI sqlthread_والد(
  Client_ڈیٹا client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  EvalEvent *pEvent;
  حرف *z_پیغام;
  Tcl_سائز n_پیغام;
  SqlThread *p = (SqlThread *)client_ڈیٹا;

  أكد(objc==3);
  غیر_مستعمل_PARAMETER(objc);

  اگر( p==0 ){
    Tcl_Append_نتیجہ(interp, "no parent thread", تہی);
    واپس TCL_ERROR;
  }

  z_پیغام = Tcl_GetStringFromObj(objv[2], &n_پیغام);
  pEvent = (EvalEvent *)ckalloc(حجم(EvalEvent)+n_پیغام+1);
  pEvent->بنیاد.اگلا_Ptr = 0;
  pEvent->بنیاد.proc = tclScriptEvent;
  pEvent->zScript = (حرف *)&pEvent[1];
  حافظہ_نقل(pEvent->zScript, z_پیغام, n_پیغام+1);
  pEvent->interp = p->interp;
  Tcl_Thread_قطار_Event(p->والد, (Tcl_Event *)pEvent, TCL_قطار_دم);
  Tcl_ThreadAlert(p->والد);

  واپس TCL_OK;
}

ساکن صحیح xBusy(خالی *pArg, صحیح nBusy){
  غیر_مستعمل_PARAMETER(pArg);
  غیر_مستعمل_PARAMETER(nBusy);
  sqlite3_sleep(50);
  واپس 1;             /* Try again... */
}

/*
** sqlthread open
**
**     Open a database handle and return the string representation of
**     the pointer value.
*/
ساکن صحیح SQLITE_TCLAPI sqlthread_open(
  Client_ڈیٹا client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح sqlite3_Test_Make_اشارہ_گر_Str(Tcl_Interp *interp, حرف *zPtr, خالی *p);

  ثابت حرف *zFilename;
  sqlite3 *db;
  حرف zBuf[100];
  خارجی صحیح Md5_رجسٹر(sqlite3*,حرف**,ثابت sqlite3_api_routines*);

  غیر_مستعمل_PARAMETER(client_ڈیٹا);
  غیر_مستعمل_PARAMETER(objc);

  zFilename = Tcl_GetString(objv[2]);
  sqlite3_open(zFilename, &db);
  Md5_رجسٹر(db, 0, 0);
  sqlite3_busy_handler(db, xBusy, 0);
  
  اگر( sqlite3_Test_Make_اشارہ_گر_Str(interp, zBuf, db) ) واپس TCL_ERROR;
  Tcl_Append_نتیجہ(interp, zBuf, تہی);

  واپس TCL_OK;
}


/*
** sqlthread open
**
**     Return the current thread-id (Tcl_GetCurrentThread()) cast to
**     an integer.
*/
ساکن صحیح SQLITE_TCLAPI sqlthread_id(
  Client_ڈیٹا client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_ThreadId id = Tcl_GetCurrentThread();
  Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewIntObj(SQLITE_PTR_مقصد_INT(id)));
  غیر_مستعمل_PARAMETER(client_ڈیٹا);
  غیر_مستعمل_PARAMETER(objc);
  غیر_مستعمل_PARAMETER(objv);
  واپس TCL_OK;
}


/*
** Dispatch routine for the sub-commands of [sqlthread].
*/
ساکن صحیح SQLITE_TCLAPI sqlthread_proc(
  Client_ڈیٹا client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  ساخت SubCommand {
    حرف *zName;
    Tcl_ObjCmdProc *xProc;
    صحیح nArg;
    حرف *zUsage;
  } aSub[] = {
    {"parent", sqlthread_والد, 1, "SCRIPT"},
    {"spawn",  sqlthread_spawn,  2, "VARNAME SCRIPT"},
    {"open",   sqlthread_open,   1, "DBNAME"},
    {"id",     sqlthread_id,     0, ""},
    {0, 0, 0}
  };
  ساخت SubCommand *pSub;
  صحیح rc;
  صحیح i_اشاریہ;

  اگر( objc<2 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "SUB-COMMAND");
    واپس TCL_ERROR;
  }

  rc = Tcl_Get_اشاریہ_From_Obj_Struct(
      interp, objv[1], aSub, حجم(aSub[0]), "sub-command", 0, &i_اشاریہ
  );
  اگر( rc!=TCL_OK ) واپس rc;
  pSub = &aSub[i_اشاریہ];

  اگر( objc<(pSub->nArg+2) ){
    Tcl_Wrong_عدد_دلائل(interp, 2, objv, pSub->zUsage);
    واپس TCL_ERROR;
  }

  واپس pSub->xProc(client_ڈیٹا, interp, objc, objv);
}

/*
** The [clock_seconds] command. This is more or less the same as the
** regular tcl [clock seconds], except that it is available in testfixture
** when linked against both Tcl 8.4 and 8.5. Because [clock seconds] is
** implemented as a script in Tcl 8.5, it is not usually available to
** testfixture.
*/ 
ساکن صحیح SQLITE_TCLAPI clock_seconds_proc(
  Client_ڈیٹا client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_وقت now;
  Tcl_Get_وقت(&now);
  Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewIntObj(now.sec));
  غیر_مستعمل_PARAMETER(client_ڈیٹا);
  غیر_مستعمل_PARAMETER(objc);
  غیر_مستعمل_PARAMETER(objv);
  واپس TCL_OK;
}

/*
** The [clock_milliseconds] command. This is more or less the same as the
** regular tcl [clock milliseconds]. 
*/ 
ساکن صحیح SQLITE_TCLAPI clock_milliseconds_proc(
  Client_ڈیٹا client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_وقت now;
  Tcl_Get_وقت(&now);
  Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewWideIntObj(
    ((Tcl_WideInt)now.sec * 1000) + (now.usec / 1000)
  ));
  غیر_مستعمل_PARAMETER(client_ڈیٹا);
  غیر_مستعمل_PARAMETER(objc);
  غیر_مستعمل_PARAMETER(objv);
  واپس TCL_OK;
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
قسمتعریف ساخت UnlockNotification UnlockNotification;
ساخت UnlockNotification {
  صحیح fired;                         /* True after unlock event has occurred */
  pthread_cond_t cond;               /* Condition variable to wait on */
  pthread_mutex_t mutex;             /* Mutex to protect structure */
};

/*
** This function is an unlock-notify callback registered with SQLite.
*/
ساکن خالی unlock_notify_cb(خالی **apArg, صحیح nArg){
  صحیح i;
  برائے(i=0; i<nArg; i++){
    UnlockNotification *p = (UnlockNotification *)apArg[i];
    pthread_mutex_تالا(&p->mutex);
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
ساکن صحیح wait_for_unlock_notify(sqlite3 *db){
  صحیح rc;
  UnlockNotification un;

  /* Initialize the UnlockNotification structure. */
  un.fired = 0;
  pthread_mutex_init(&un.mutex, 0);
  pthread_cond_init(&un.cond, 0);

  /* Register for an unlock-notify callback. */
  rc = sqlite3_unlock_notify(db, unlock_notify_cb, (خالی *)&un);
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
    pthread_mutex_تالا(&un.mutex);
    اگر( !un.fired ){
      pthread_cond_wait(&un.cond, &un.mutex);
    }
    pthread_mutex_unlock(&un.mutex);
  }

  /* Destroy the mutex and condition variables. */
  pthread_cond_destroy(&un.cond);
  pthread_mutex_destroy(&un.mutex);

  واپس rc;
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
صحیح sqlite3_blocking_قدم(sqlite3_stmt *pStmt){
  صحیح rc;
  جب( SQLITE_LOCKED==(rc = sqlite3_قدم(pStmt)) ){
    rc = wait_for_unlock_notify(sqlite3_db_handle(pStmt));
    اگر( rc!=SQLITE_OK ) توڑو;
    sqlite3_reset(pStmt);
  }
  واپس rc;
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
  ثابت حرف *zSql,         /* UTF-8 encoded SQL statement. */
  صحیح nSql,                 /* Length of zSql in bytes. */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  ثابت حرف **pz           /* OUT: End of parsed string */
){
  صحیح rc;
  جب( SQLITE_LOCKED==(rc = sqlite3_prepare_v2(db, zSql, nSql, ppStmt, pz)) ){
    rc = wait_for_unlock_notify(db);
    اگر( rc!=SQLITE_OK ) توڑو;
  }
  واپس rc;
}
/* END_SQLITE_BLOCKING_STEP */

/*
** Usage: sqlite3_blocking_step STMT
**
** Advance the statement to the next row.
*/
ساکن صحیح SQLITE_TCLAPI blocking_قدم_proc(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){

  sqlite3_stmt *pStmt;
  صحیح rc;

  اگر( objc!=2 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "STMT");
    واپس TCL_ERROR;
  }

  pStmt = (sqlite3_stmt*)sqlite3_Test_متن_مقصد_Ptr(Tcl_GetString(objv[1]));
  rc = sqlite3_blocking_قدم(pStmt);

  Tcl_سیٹ_نتیجہ(interp, (حرف *)sqlite3ErrName(rc), 0);
  واپس TCL_OK;
}

/*
** Usage: sqlite3_blocking_prepare_v2 DB sql bytes ?tailvar?
** Usage: sqlite3_nonblocking_prepare_v2 DB sql bytes ?tailvar?
*/
ساکن صحیح SQLITE_TCLAPI blocking_prepare_v2_proc(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  ثابت حرف *zSql;
  صحیح بائٹیں;
  ثابت حرف *z_دم = 0;
  sqlite3_stmt *pStmt = 0;
  حرف zBuf[50];
  صحیح rc;
  صحیح isBlocking = !(client_ڈیٹا==0);

  اگر( objc!=5 && objc!=4 ){
    Tcl_Append_نتیجہ(interp, "wrong # args: should be \"", 
       Tcl_GetString(objv[0]), " DB sql bytes tailvar", 0);
    واپس TCL_ERROR;
  }
  اگر( get_Db_اشارہ_گر(interp, Tcl_GetString(objv[1]), &db) ) واپس TCL_ERROR;
  zSql = Tcl_GetString(objv[2]);
  اگر( Tcl_GetIntFromObj(interp, objv[3], &بائٹیں) ) واپس TCL_ERROR;

  اگر( isBlocking ){
    rc = sqlite3_blocking_prepare_v2(db, zSql, بائٹیں, &pStmt, &z_دم);
  }ورنہ{
    rc = sqlite3_prepare_v2(db, zSql, بائٹیں, &pStmt, &z_دم);
  }

  أكد(rc==SQLITE_OK || pStmt==0);
  اگر( z_دم && objc>=5 ){
    اگر( بائٹیں>=0 ){
      بائٹیں = بائٹیں - (z_دم-zSql);
    }
    Tcl_Obj_سیٹ_Var2(interp, objv[4], 0, Tcl_NewStringObj(z_دم, بائٹیں), 0);
  }
  اگر( rc!=SQLITE_OK ){
    أكد( pStmt==0 );
    sqlite3_snprintf(حجم(zBuf), zBuf, "%s ", (حرف *)sqlite3ErrName(rc));
    Tcl_Append_نتیجہ(interp, zBuf, sqlite3_errmsg(db), تہی);
    واپس TCL_ERROR;
  }

  اگر( pStmt ){
    اگر( sqlite3_Test_Make_اشارہ_گر_Str(interp, zBuf, pStmt) ) واپس TCL_ERROR;
    Tcl_Append_نتیجہ(interp, zBuf, تہی);
  }
  واپس TCL_OK;
}

#اگر_ختم /* SQLITE_OS_UNIX && SQLITE_ENABLE_UNLOCK_NOTIFY */
/*
** End of implementation of [sqlite3_blocking_step].
************************************************************************/

/*
** Register commands with the TCL interpreter.
*/
صحیح SqlitetestThread_Init(Tcl_Interp *interp){
  ساخت TclCmd {
    صحیح (*xProc)(خالی*, Tcl_Interp*, صحیح, Tcl_Obj*ثابت*);
    ثابت حرف *zName;
    صحیح iCtx;
  } aCmd[] = {
    { sqlthread_proc,           "sqlthread",                      0 },
    { clock_seconds_proc,       "clock_second",                   0 },
    { clock_milliseconds_proc,  "clock_milliseconds",             0 },
#اگر SQLITE_OS_UNIX && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)
    { blocking_قدم_proc,       "sqlite3_blocking_step",          0 },
    { blocking_prepare_v2_proc, "sqlite3_blocking_prepare_v2",    1 },
    { blocking_prepare_v2_proc, "sqlite3_nonblocking_prepare_v2", 0 },
#اگر_ختم
  };
  صحیح ii;

  برائے(ii=0; ii<حجم(aCmd)/حجم(aCmd[0]); ii++){
    خالی *p = SQLITE_INT_مقصد_PTR(aCmd[ii].iCtx);
    Tcl_CreateObjCommand(interp, aCmd[ii].zName, aCmd[ii].xProc, p, 0);
  }
  واپس TCL_OK;
}
#ورنہ
صحیح SqlitetestThread_Init(Tcl_Interp *interp){
  واپس TCL_OK;
}
#اگر_ختم
