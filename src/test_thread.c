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

#include "sqliteInt.h"
#include "tclsqlite.h"

#གལ་སྲིད SQLITE_THREADSAFE

#include <errno.h>

#གལ་སྲིད !defined(_MSC_VER)
#include <unistd.h>
#endif

/*
** One of these is allocated for each thread created by [sqlthread spawn].
*/
typedef sa_nracanaa SqlThread SqlThread;
sa_nracanaa SqlThread {
  Tcl_ThreadId parent;     /* Thread id of parent thread */
  Tcl_Interp *interp;      /* Parent interpreter */
  var_na *zScript;           /* The script to execute. */
  var_na *zVarname;          /* Varname in parent script */
};

/*
** A custom Tcl_Event type used by this module. When the event is
** handled, script zScript is evaluated in interpreter interp. If
** the evaluation throws an exception (returns TCL_ERROR), then the
** error is handled by Tcl_BackgroundError(). If no error occurs,
** the result is simply discarded.
*/
typedef sa_nracanaa EvalEvent EvalEvent;
sa_nracanaa EvalEvent {
  Tcl_Event base;          /* Base class of type Tcl_Event */
  var_na *zScript;           /* The script to execute. */
  Tcl_Interp *interp;      /* The interpreter to execute it in. */
};

static Tcl_ObjCmdProc sqlthread_proc;
static Tcl_ObjCmdProc clock_seconds_proc;
#གལ་སྲིད SQLITE_OS_UNIX && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)
static Tcl_ObjCmdProc blocking_step_proc;
static Tcl_ObjCmdProc blocking_prepare_v2_proc;
#endif
ཧྲིལ Sqlitetest1_Init(Tcl_Interp *);
ཧྲིལ Sqlite3_Init(Tcl_Interp *);

/* Functions from main.c */
extern const var_na *sqlite3ErrName(ཧྲིལ);

/* Functions from test1.c */
extern void *sqlite3TestTextToPtr(const var_na *);
extern ཧྲིལ getDbPointer(Tcl_Interp *, const var_na *, sqlite3 **);
extern ཧྲིལ sqlite3TestMakePointerStr(Tcl_Interp *, var_na *, void *);
extern ཧྲིལ sqlite3TestErrCode(Tcl_Interp *, sqlite3 *, ཧྲིལ);

/*
** Handler for events of type EvalEvent.
*/
static ཧྲིལ SQLITE_TCLAPI tclScriptEvent(Tcl_Event *evPtr, ཧྲིལ flags){
  ཧྲིལ rc;
  EvalEvent *p = (EvalEvent *)evPtr;
  rc = Tcl_Eval(p->interp, p->zScript);
  གལ་སྲིད( rc!=TCL_OK ){
    Tcl_BackgroundError(p->interp);
  }
  UNUSED_PARAMETER(flags);
  ལོག 1;
}

/*
** Register an EvalEvent to evaluate the script pScript in the
** parent interpreter/thread of SqlThread p.
*/
static void postToParent(SqlThread *p, Tcl_Obj *pScript){
  EvalEvent *pEvent;
  var_na *zMsg;
  Tcl_Size nMsg;

  zMsg = Tcl_GetStringFromObj(pScript, &nMsg); 
  pEvent = (EvalEvent *)ckalloc(parimaa_na(EvalEvent)+nMsg+1);
  pEvent->base.nextPtr = 0;
  pEvent->base.proc = tclScriptEvent;
  pEvent->zScript = (var_na *)&pEvent[1];
  memcpy(pEvent->zScript, zMsg, nMsg+1);
  pEvent->interp = p->interp;

  Tcl_ThreadQueueEvent(p->parent, (Tcl_Event *)pEvent, TCL_QUEUE_TAIL);
  Tcl_ThreadAlert(p->parent);
}

/*
** The main function for threads created with [sqlthread spawn].
*/
static Tcl_ThreadCreateType tclScriptThread(ClientData pSqlThread){
  Tcl_Interp *interp;
  Tcl_Obj *pRes;
  Tcl_Obj *pList;
  ཧྲིལ rc;
  SqlThread *p = (SqlThread *)pSqlThread;
  extern ཧྲིལ Sqlitetest_mutex_Init(Tcl_Interp*);

  interp = Tcl_CreateInterp();
  Tcl_CreateObjCommand(interp, "clock_seconds", clock_seconds_proc, 0, 0);
  Tcl_CreateObjCommand(interp, "sqlthread", sqlthread_proc, pSqlThread, 0);
#གལ་སྲིད SQLITE_OS_UNIX && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)
  Tcl_CreateObjCommand(interp, "sqlite3_blocking_step", blocking_step_proc,0,0);
  Tcl_CreateObjCommand(interp, 
      "sqlite3_blocking_prepare_v2", blocking_prepare_v2_proc, (void *)1, 0);
  Tcl_CreateObjCommand(interp, 
      "sqlite3_nonblocking_prepare_v2", blocking_prepare_v2_proc, 0, 0);
#endif
  Sqlitetest1_Init(interp);
  Sqlitetest_mutex_Init(interp);
  Sqlite3_Init(interp);

  rc = Tcl_Eval(interp, p->zScript);
  pRes = Tcl_GetObjResult(interp);
  pList = Tcl_NewObj();
  Tcl_IncrRefCount(pList);
  Tcl_IncrRefCount(pRes);

  གལ་སྲིད( rc!=TCL_OK ){
    Tcl_ListObjAppendElement(interp, pList, Tcl_NewStringObj("error", -1));
    Tcl_ListObjAppendElement(interp, pList, pRes);
    postToParent(p, pList);
    Tcl_DecrRefCount(pList);
    pList = Tcl_NewObj();
  }

  Tcl_ListObjAppendElement(interp, pList, Tcl_NewStringObj("set", -1));
  Tcl_ListObjAppendElement(interp, pList, Tcl_NewStringObj(p->zVarname, -1));
  Tcl_ListObjAppendElement(interp, pList, pRes);
  postToParent(p, pList);

  ckfree((void *)p);
  Tcl_DecrRefCount(pList);
  Tcl_DecrRefCount(pRes);
  Tcl_DeleteInterp(interp);
  སྐབས( Tcl_DoOneEvent(TCL_ALL_EVENTS|TCL_DONT_WAIT) );
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
static ཧྲིལ SQLITE_TCLAPI sqlthread_spawn(
  ClientData clientData,
  Tcl_Interp *interp,
  ཧྲིལ objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_ThreadId x;
  SqlThread *pNew;
  ཧྲིལ rc;

  Tcl_Size nVarname; var_na *zVarname;
  Tcl_Size nScript; var_na *zScript;

  /* Parameters for thread creation */
  const ཧྲིལ nStack = TCL_THREAD_STACK_DEFAULT;
  const ཧྲིལ flags = TCL_THREAD_NOFLAGS;

  assert(objc==4);
  UNUSED_PARAMETER(clientData);
  UNUSED_PARAMETER(objc);

  zVarname = Tcl_GetStringFromObj(objv[2], &nVarname);
  zScript = Tcl_GetStringFromObj(objv[3], &nScript);

  pNew = (SqlThread *)ckalloc(parimaa_na(SqlThread)+nVarname+nScript+2);
  pNew->zVarname = (var_na *)&pNew[1];
  pNew->zScript = (var_na *)&pNew->zVarname[nVarname+1];
  memcpy(pNew->zVarname, zVarname, nVarname+1);
  memcpy(pNew->zScript, zScript, nScript+1);
  pNew->parent = Tcl_GetCurrentThread();
  pNew->interp = interp;

  rc = Tcl_CreateThread(&x, tclScriptThread, (void *)pNew, nStack, flags);
  གལ་སྲིད( rc!=TCL_OK ){
    Tcl_AppendResult(interp, "Error in Tcl_CreateThread()", NULL);
    ckfree((var_na *)pNew);
    ལོག TCL_ERROR;
  }

  ལོག TCL_OK;
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
static ཧྲིལ SQLITE_TCLAPI sqlthread_parent(
  ClientData clientData,
  Tcl_Interp *interp,
  ཧྲིལ objc,
  Tcl_Obj *CONST objv[]
){
  EvalEvent *pEvent;
  var_na *zMsg;
  Tcl_Size nMsg;
  SqlThread *p = (SqlThread *)clientData;

  assert(objc==3);
  UNUSED_PARAMETER(objc);

  གལ་སྲིད( p==0 ){
    Tcl_AppendResult(interp, "no parent thread", NULL);
    ལོག TCL_ERROR;
  }

  zMsg = Tcl_GetStringFromObj(objv[2], &nMsg);
  pEvent = (EvalEvent *)ckalloc(parimaa_na(EvalEvent)+nMsg+1);
  pEvent->base.nextPtr = 0;
  pEvent->base.proc = tclScriptEvent;
  pEvent->zScript = (var_na *)&pEvent[1];
  memcpy(pEvent->zScript, zMsg, nMsg+1);
  pEvent->interp = p->interp;
  Tcl_ThreadQueueEvent(p->parent, (Tcl_Event *)pEvent, TCL_QUEUE_TAIL);
  Tcl_ThreadAlert(p->parent);

  ལོག TCL_OK;
}

static ཧྲིལ xBusy(void *pArg, ཧྲིལ nBusy){
  UNUSED_PARAMETER(pArg);
  UNUSED_PARAMETER(nBusy);
  sqlite3_sleep(50);
  ལོག 1;             /* Try again... */
}

/*
** sqlthread open
**
**     Open a database handle and return the string representation of
**     the pointer value.
*/
static ཧྲིལ SQLITE_TCLAPI sqlthread_open(
  ClientData clientData,
  Tcl_Interp *interp,
  ཧྲིལ objc,
  Tcl_Obj *CONST objv[]
){
  ཧྲིལ sqlite3TestMakePointerStr(Tcl_Interp *interp, var_na *zPtr, void *p);

  const var_na *zFilename;
  sqlite3 *db;
  var_na zBuf[100];
  extern ཧྲིལ Md5_Register(sqlite3*,var_na**,const sqlite3_api_routines*);

  UNUSED_PARAMETER(clientData);
  UNUSED_PARAMETER(objc);

  zFilename = Tcl_GetString(objv[2]);
  sqlite3_open(zFilename, &db);
  Md5_Register(db, 0, 0);
  sqlite3_busy_handler(db, xBusy, 0);
  
  གལ་སྲིད( sqlite3TestMakePointerStr(interp, zBuf, db) ) ལོག TCL_ERROR;
  Tcl_AppendResult(interp, zBuf, NULL);

  ལོག TCL_OK;
}


/*
** sqlthread open
**
**     Return the current thread-id (Tcl_GetCurrentThread()) cast to
**     an integer.
*/
static ཧྲིལ SQLITE_TCLAPI sqlthread_id(
  ClientData clientData,
  Tcl_Interp *interp,
  ཧྲིལ objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_ThreadId id = Tcl_GetCurrentThread();
  Tcl_SetObjResult(interp, Tcl_NewIntObj(SQLITE_PTR_TO_INT(id)));
  UNUSED_PARAMETER(clientData);
  UNUSED_PARAMETER(objc);
  UNUSED_PARAMETER(objv);
  ལོག TCL_OK;
}


/*
** Dispatch routine for the sub-commands of [sqlthread].
*/
static ཧྲིལ SQLITE_TCLAPI sqlthread_proc(
  ClientData clientData,
  Tcl_Interp *interp,
  ཧྲིལ objc,
  Tcl_Obj *CONST objv[]
){
  sa_nracanaa SubCommand {
    var_na *zName;
    Tcl_ObjCmdProc *xProc;
    ཧྲིལ nArg;
    var_na *zUsage;
  } aSub[] = {
    {"parent", sqlthread_parent, 1, "SCRIPT"},
    {"spawn",  sqlthread_spawn,  2, "VARNAME SCRIPT"},
    {"open",   sqlthread_open,   1, "DBNAME"},
    {"id",     sqlthread_id,     0, ""},
    {0, 0, 0}
  };
  sa_nracanaa SubCommand *pSub;
  ཧྲིལ rc;
  ཧྲིལ iIndex;

  གལ་སྲིད( objc<2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "SUB-COMMAND");
    ལོག TCL_ERROR;
  }

  rc = Tcl_GetIndexFromObjStruct(
      interp, objv[1], aSub, parimaa_na(aSub[0]), "sub-command", 0, &iIndex
  );
  གལ་སྲིད( rc!=TCL_OK ) ལོག rc;
  pSub = &aSub[iIndex];

  གལ་སྲིད( objc<(pSub->nArg+2) ){
    Tcl_WrongNumArgs(interp, 2, objv, pSub->zUsage);
    ལོག TCL_ERROR;
  }

  ལོག pSub->xProc(clientData, interp, objc, objv);
}

/*
** The [clock_seconds] command. This is more or less the same as the
** regular tcl [clock seconds], except that it is available in testfixture
** when linked against both Tcl 8.4 and 8.5. Because [clock seconds] is
** implemented as a script in Tcl 8.5, it is not usually available to
** testfixture.
*/ 
static ཧྲིལ SQLITE_TCLAPI clock_seconds_proc(
  ClientData clientData,
  Tcl_Interp *interp,
  ཧྲིལ objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Time now;
  Tcl_GetTime(&now);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(now.sec));
  UNUSED_PARAMETER(clientData);
  UNUSED_PARAMETER(objc);
  UNUSED_PARAMETER(objv);
  ལོག TCL_OK;
}

/*
** The [clock_milliseconds] command. This is more or less the same as the
** regular tcl [clock milliseconds]. 
*/ 
static ཧྲིལ SQLITE_TCLAPI clock_milliseconds_proc(
  ClientData clientData,
  Tcl_Interp *interp,
  ཧྲིལ objc,
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
  ལོག TCL_OK;
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
#གལ་སྲིད SQLITE_OS_UNIX && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)

/* BEGIN_SQLITE_BLOCKING_STEP */
/* This example uses the pthreads API */
#include <pthread.h>

/*
** A pointer to an instance of this structure is passed as the user-context
** pointer when registering for an unlock-notify callback.
*/
typedef sa_nracanaa UnlockNotification UnlockNotification;
sa_nracanaa UnlockNotification {
  ཧྲིལ fired;                         /* True after unlock event has occurred */
  pthread_cond_t cond;               /* Condition variable to wait on */
  pthread_mutex_t mutex;             /* Mutex to protect structure */
};

/*
** This function is an unlock-notify callback registered with SQLite.
*/
static void unlock_notify_cb(void **apArg, ཧྲིལ nArg){
  ཧྲིལ i;
  དོན་ལུ(i=0; i<nArg; i++){
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
static ཧྲིལ wait_for_unlock_notify(sqlite3 *db){
  ཧྲིལ rc;
  UnlockNotification un;

  /* Initialize the UnlockNotification structure. */
  un.fired = 0;
  pthread_mutex_init(&un.mutex, 0);
  pthread_cond_init(&un.cond, 0);

  /* Register for an unlock-notify callback. */
  rc = sqlite3_unlock_notify(db, unlock_notify_cb, (void *)&un);
  assert( rc==SQLITE_LOCKED || rc==SQLITE_OK );

  /* The call to sqlite3_unlock_notify() always returns either SQLITE_LOCKED 
  ** or SQLITE_OK. 
  **
  ** If SQLITE_LOCKED was returned, then the system is deadlocked. In this
  ** case this function needs to return SQLITE_LOCKED to the caller so 
  ** that the current transaction can be rolled back. Otherwise, block
  ** until the unlock-notify callback is invoked, then return SQLITE_OK.
  */
  གལ་སྲིད( rc==SQLITE_OK ){
    pthread_mutex_lock(&un.mutex);
    གལ་སྲིད( !un.fired ){
      pthread_cond_wait(&un.cond, &un.mutex);
    }
    pthread_mutex_unlock(&un.mutex);
  }

  /* Destroy the mutex and condition variables. */
  pthread_cond_destroy(&un.cond);
  pthread_mutex_destroy(&un.mutex);

  ལོག rc;
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
ཧྲིལ sqlite3_blocking_step(sqlite3_stmt *pStmt){
  ཧྲིལ rc;
  སྐབས( SQLITE_LOCKED==(rc = sqlite3_step(pStmt)) ){
    rc = wait_for_unlock_notify(sqlite3_db_handle(pStmt));
    གལ་སྲིད( rc!=SQLITE_OK ) bha_nga;
    sqlite3_reset(pStmt);
  }
  ལོག rc;
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
ཧྲིལ sqlite3_blocking_prepare_v2(
  sqlite3 *db,              /* Database handle. */
  const var_na *zSql,         /* UTF-8 encoded SQL statement. */
  ཧྲིལ nSql,                 /* Length of zSql in bytes. */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  const var_na **pz           /* OUT: End of parsed string */
){
  ཧྲིལ rc;
  སྐབས( SQLITE_LOCKED==(rc = sqlite3_prepare_v2(db, zSql, nSql, ppStmt, pz)) ){
    rc = wait_for_unlock_notify(db);
    གལ་སྲིད( rc!=SQLITE_OK ) bha_nga;
  }
  ལོག rc;
}
/* END_SQLITE_BLOCKING_STEP */

/*
** Usage: sqlite3_blocking_step STMT
**
** Advance the statement to the next row.
*/
static ཧྲིལ SQLITE_TCLAPI blocking_step_proc(
  void * clientData,
  Tcl_Interp *interp,
  ཧྲིལ objc,
  Tcl_Obj *CONST objv[]
){

  sqlite3_stmt *pStmt;
  ཧྲིལ rc;

  གལ་སྲིད( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "STMT");
    ལོག TCL_ERROR;
  }

  pStmt = (sqlite3_stmt*)sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  rc = sqlite3_blocking_step(pStmt);

  Tcl_SetResult(interp, (var_na *)sqlite3ErrName(rc), 0);
  ལོག TCL_OK;
}

/*
** Usage: sqlite3_blocking_prepare_v2 DB sql bytes ?tailvar?
** Usage: sqlite3_nonblocking_prepare_v2 DB sql bytes ?tailvar?
*/
static ཧྲིལ SQLITE_TCLAPI blocking_prepare_v2_proc(
  void * clientData,
  Tcl_Interp *interp,
  ཧྲིལ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  const var_na *zSql;
  ཧྲིལ bytes;
  const var_na *zTail = 0;
  sqlite3_stmt *pStmt = 0;
  var_na zBuf[50];
  ཧྲིལ rc;
  ཧྲིལ isBlocking = !(clientData==0);

  གལ་སྲིད( objc!=5 && objc!=4 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", 
       Tcl_GetString(objv[0]), " DB sql bytes tailvar", 0);
    ལོག TCL_ERROR;
  }
  གལ་སྲིད( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ལོག TCL_ERROR;
  zSql = Tcl_GetString(objv[2]);
  གལ་སྲིད( Tcl_GetIntFromObj(interp, objv[3], &bytes) ) ལོག TCL_ERROR;

  གལ་སྲིད( isBlocking ){
    rc = sqlite3_blocking_prepare_v2(db, zSql, bytes, &pStmt, &zTail);
  }དེ་མིན{
    rc = sqlite3_prepare_v2(db, zSql, bytes, &pStmt, &zTail);
  }

  assert(rc==SQLITE_OK || pStmt==0);
  གལ་སྲིད( zTail && objc>=5 ){
    གལ་སྲིད( bytes>=0 ){
      bytes = bytes - (zTail-zSql);
    }
    Tcl_ObjSetVar2(interp, objv[4], 0, Tcl_NewStringObj(zTail, bytes), 0);
  }
  གལ་སྲིད( rc!=SQLITE_OK ){
    assert( pStmt==0 );
    sqlite3_snprintf(parimaa_na(zBuf), zBuf, "%s ", (var_na *)sqlite3ErrName(rc));
    Tcl_AppendResult(interp, zBuf, sqlite3_errmsg(db), NULL);
    ལོག TCL_ERROR;
  }

  གལ་སྲིད( pStmt ){
    གལ་སྲིད( sqlite3TestMakePointerStr(interp, zBuf, pStmt) ) ལོག TCL_ERROR;
    Tcl_AppendResult(interp, zBuf, NULL);
  }
  ལོག TCL_OK;
}

#endif /* SQLITE_OS_UNIX && SQLITE_ENABLE_UNLOCK_NOTIFY */
/*
** End of implementation of [sqlite3_blocking_step].
************************************************************************/

/*
** Register commands with the TCL interpreter.
*/
ཧྲིལ SqlitetestThread_Init(Tcl_Interp *interp){
  sa_nracanaa TclCmd {
    ཧྲིལ (*xProc)(void*, Tcl_Interp*, ཧྲིལ, Tcl_Obj*const*);
    const var_na *zName;
    ཧྲིལ iCtx;
  } aCmd[] = {
    { sqlthread_proc,           "sqlthread",                      0 },
    { clock_seconds_proc,       "clock_second",                   0 },
    { clock_milliseconds_proc,  "clock_milliseconds",             0 },
#གལ་སྲིད SQLITE_OS_UNIX && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)
    { blocking_step_proc,       "sqlite3_blocking_step",          0 },
    { blocking_prepare_v2_proc, "sqlite3_blocking_prepare_v2",    1 },
    { blocking_prepare_v2_proc, "sqlite3_nonblocking_prepare_v2", 0 },
#endif
  };
  ཧྲིལ ii;

  དོན་ལུ(ii=0; ii<parimaa_na(aCmd)/parimaa_na(aCmd[0]); ii++){
    void *p = SQLITE_INT_TO_PTR(aCmd[ii].iCtx);
    Tcl_CreateObjCommand(interp, aCmd[ii].zName, aCmd[ii].xProc, p, 0);
  }
  ལོག TCL_OK;
}
#དེ་མིན
ཧྲིལ SqlitetestThread_Init(Tcl_Interp *interp){
  ལོག TCL_OK;
}
#endif
