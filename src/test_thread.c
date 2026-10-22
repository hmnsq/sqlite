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

#ಅನ್ತರ್ಭಾವ "sqliteInt.h"
#ಅನ್ತರ್ಭಾವ "tclsqlite.h"

#ಆದರೆ SQLITE_THREADSAFE

#ಅನ್ತರ್ಭಾವ <errno.h>

#ಆದರೆ !defined(_MSC_VER)
#ಅನ್ತರ್ಭಾವ <unistd.h>
#ಆದರೆ_ಅಂತ್ಯ

/*
** One of these is allocated for each thread created by [sqlthread spawn].
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ SqlThread SqlThread;
ರಚನೆ SqlThread {
  Tcl_ThreadId ಪೋಷಕ;     /* Thread id of parent thread */
  Tcl_Interp *interp;      /* Parent interpreter */
  ಅಕ್ಷರ *zScript;           /* The script to execute. */
  ಅಕ್ಷರ *zVarname;          /* Varname in parent script */
};

/*
** A custom Tcl_Event type used by this module. When the event is
** handled, script zScript is evaluated in interpreter interp. If
** the evaluation throws an exception (returns TCL_ERROR), then the
** error is handled by Tcl_BackgroundError(). If no error occurs,
** the result is simply discarded.
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ EvalEvent EvalEvent;
ರಚನೆ EvalEvent {
  Tcl_Event ತಳ;          /* Base class of type Tcl_Event */
  ಅಕ್ಷರ *zScript;           /* The script to execute. */
  Tcl_Interp *interp;      /* The interpreter to execute it in. */
};

ಸ್ಥಿರ Tcl_ObjCmdProc sqlthread_proc;
ಸ್ಥಿರ Tcl_ObjCmdProc clock_seconds_proc;
#ಆದರೆ SQLITE_OS_UNIX && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)
ಸ್ಥಿರ Tcl_ObjCmdProc blocking_ಹೆಜ್ಜೆ_proc;
ಸ್ಥಿರ Tcl_ObjCmdProc blocking_prepare_v2_proc;
#ಆದರೆ_ಅಂತ್ಯ
ಪೂರ್ಣಾಂಕ Sqlitetest1_Init(Tcl_Interp *);
ಪೂರ್ಣಾಂಕ Sqlite3_Init(Tcl_Interp *);

/* Functions from main.c */
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3ErrName(ಪೂರ್ಣಾಂಕ);

/* Functions from test1.c */
ಬಾಹ್ಯ ಶೂನ್ಯ *sqlite3_Test_ಪಠ್ಯ_ಗಮ್ಯ_Ptr(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *);
ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ get_Db_ಸೂಚಕ(Tcl_Interp *, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *, sqlite3 **);
ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ sqlite3_Test_Make_ಸೂಚಕ_Str(Tcl_Interp *, ಅಕ್ಷರ *, ಶೂನ್ಯ *);
ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ sqlite3_Test_Err_ಸಂಕೇತ(Tcl_Interp *, sqlite3 *, ಪೂರ್ಣಾಂಕ);

/*
** Handler for events of type EvalEvent.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI tclScriptEvent(Tcl_Event *evPtr, ಪೂರ್ಣಾಂಕ flags){
  ಪೂರ್ಣಾಂಕ rc;
  EvalEvent *p = (EvalEvent *)evPtr;
  rc = Tcl_Eval(p->interp, p->zScript);
  ಆದರೆ( rc!=TCL_OK ){
    Tcl_BackgroundError(p->interp);
  }
  ಬಳಸದ_PARAMETER(flags);
  ಹಿಂತಿರುಗಿ 1;
}

/*
** Register an EvalEvent to evaluate the script pScript in the
** parent interpreter/thread of SqlThread p.
*/
ಸ್ಥಿರ ಶೂನ್ಯ post_ಗಮ್ಯ_ಪೋಷಕ(SqlThread *p, Tcl_Obj *pScript){
  EvalEvent *pEvent;
  ಅಕ್ಷರ *z_ಸಂದೇಶ;
  Tcl_ಗಾತ್ರ n_ಸಂದೇಶ;

  z_ಸಂದೇಶ = Tcl_GetStringFromObj(pScript, &n_ಸಂದೇಶ); 
  pEvent = (EvalEvent *)ckalloc(ಗಾತ್ರ(EvalEvent)+n_ಸಂದೇಶ+1);
  pEvent->ತಳ.ಮುಂದಿನ_Ptr = 0;
  pEvent->ತಳ.proc = tclScriptEvent;
  pEvent->zScript = (ಅಕ್ಷರ *)&pEvent[1];
  ಸ್ಮರಣೆ_ನಕಲು(pEvent->zScript, z_ಸಂದೇಶ, n_ಸಂದೇಶ+1);
  pEvent->interp = p->interp;

  Tcl_Thread_ಸರತಿ_Event(p->ಪೋಷಕ, (Tcl_Event *)pEvent, TCL_ಸರತಿ_ಬಾಲ);
  Tcl_ThreadAlert(p->ಪೋಷಕ);
}

/*
** The main function for threads created with [sqlthread spawn].
*/
ಸ್ಥಿರ Tcl_ThreadCreateType tclScriptThread(Client_ದತ್ತಾಂಶ pSqlThread){
  Tcl_Interp *interp;
  Tcl_Obj *pRes;
  Tcl_Obj *pList;
  ಪೂರ್ಣಾಂಕ rc;
  SqlThread *p = (SqlThread *)pSqlThread;
  ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ Sqlitetest_mutex_Init(Tcl_Interp*);

  interp = Tcl_CreateInterp();
  Tcl_CreateObjCommand(interp, "clock_seconds", clock_seconds_proc, 0, 0);
  Tcl_CreateObjCommand(interp, "sqlthread", sqlthread_proc, pSqlThread, 0);
#ಆದರೆ SQLITE_OS_UNIX && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)
  Tcl_CreateObjCommand(interp, "sqlite3_blocking_step", blocking_ಹೆಜ್ಜೆ_proc,0,0);
  Tcl_CreateObjCommand(interp, 
      "sqlite3_blocking_prepare_v2", blocking_prepare_v2_proc, (ಶೂನ್ಯ *)1, 0);
  Tcl_CreateObjCommand(interp, 
      "sqlite3_nonblocking_prepare_v2", blocking_prepare_v2_proc, 0, 0);
#ಆದರೆ_ಅಂತ್ಯ
  Sqlitetest1_Init(interp);
  Sqlitetest_mutex_Init(interp);
  Sqlite3_Init(interp);

  rc = Tcl_Eval(interp, p->zScript);
  pRes = Tcl_Get_Obj_ಫಲಿತಾಂಶ(interp);
  pList = Tcl_NewObj();
  Tcl_Incr_Ref_ಎಣಿಕೆ(pList);
  Tcl_Incr_Ref_ಎಣಿಕೆ(pRes);

  ಆದರೆ( rc!=TCL_OK ){
    Tcl_ListObjAppendElement(interp, pList, Tcl_NewStringObj("error", -1));
    Tcl_ListObjAppendElement(interp, pList, pRes);
    post_ಗಮ್ಯ_ಪೋಷಕ(p, pList);
    Tcl_Decr_Ref_ಎಣಿಕೆ(pList);
    pList = Tcl_NewObj();
  }

  Tcl_ListObjAppendElement(interp, pList, Tcl_NewStringObj("set", -1));
  Tcl_ListObjAppendElement(interp, pList, Tcl_NewStringObj(p->zVarname, -1));
  Tcl_ListObjAppendElement(interp, pList, pRes);
  post_ಗಮ್ಯ_ಪೋಷಕ(p, pList);

  ckfree((ಶೂನ್ಯ *)p);
  Tcl_Decr_Ref_ಎಣಿಕೆ(pList);
  Tcl_Decr_Ref_ಎಣಿಕೆ(pRes);
  Tcl_DeleteInterp(interp);
  ಆಗ( Tcl_DoOneEvent(TCL_ALL_EVENTS|TCL_DONT_WAIT) );
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI sqlthread_spawn(
  Client_ದತ್ತಾಂಶ client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_ThreadId x;
  SqlThread *pNew;
  ಪೂರ್ಣಾಂಕ rc;

  Tcl_ಗಾತ್ರ nVarname; ಅಕ್ಷರ *zVarname;
  Tcl_ಗಾತ್ರ nScript; ಅಕ್ಷರ *zScript;

  /* Parameters for thread creation */
  ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ n_ರಾಶಿ_2 = TCL_THREAD_ರಾಶಿ_DEFAULT;
  ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ flags = TCL_THREAD_NOFLAGS;

  ಪ್ರತಿಜ್ಞಾ(objc==4);
  ಬಳಸದ_PARAMETER(client_ದತ್ತಾಂಶ);
  ಬಳಸದ_PARAMETER(objc);

  zVarname = Tcl_GetStringFromObj(objv[2], &nVarname);
  zScript = Tcl_GetStringFromObj(objv[3], &nScript);

  pNew = (SqlThread *)ckalloc(ಗಾತ್ರ(SqlThread)+nVarname+nScript+2);
  pNew->zVarname = (ಅಕ್ಷರ *)&pNew[1];
  pNew->zScript = (ಅಕ್ಷರ *)&pNew->zVarname[nVarname+1];
  ಸ್ಮರಣೆ_ನಕಲು(pNew->zVarname, zVarname, nVarname+1);
  ಸ್ಮರಣೆ_ನಕಲು(pNew->zScript, zScript, nScript+1);
  pNew->ಪೋಷಕ = Tcl_GetCurrentThread();
  pNew->interp = interp;

  rc = Tcl_CreateThread(&x, tclScriptThread, (ಶೂನ್ಯ *)pNew, n_ರಾಶಿ_2, flags);
  ಆದರೆ( rc!=TCL_OK ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "Error in Tcl_CreateThread()", ಖಾಲಿ);
    ckfree((ಅಕ್ಷರ *)pNew);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  ಹಿಂತಿರುಗಿ TCL_OK;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI sqlthread_ಪೋಷಕ(
  Client_ದತ್ತಾಂಶ client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  EvalEvent *pEvent;
  ಅಕ್ಷರ *z_ಸಂದೇಶ;
  Tcl_ಗಾತ್ರ n_ಸಂದೇಶ;
  SqlThread *p = (SqlThread *)client_ದತ್ತಾಂಶ;

  ಪ್ರತಿಜ್ಞಾ(objc==3);
  ಬಳಸದ_PARAMETER(objc);

  ಆದರೆ( p==0 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "no parent thread", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  z_ಸಂದೇಶ = Tcl_GetStringFromObj(objv[2], &n_ಸಂದೇಶ);
  pEvent = (EvalEvent *)ckalloc(ಗಾತ್ರ(EvalEvent)+n_ಸಂದೇಶ+1);
  pEvent->ತಳ.ಮುಂದಿನ_Ptr = 0;
  pEvent->ತಳ.proc = tclScriptEvent;
  pEvent->zScript = (ಅಕ್ಷರ *)&pEvent[1];
  ಸ್ಮರಣೆ_ನಕಲು(pEvent->zScript, z_ಸಂದೇಶ, n_ಸಂದೇಶ+1);
  pEvent->interp = p->interp;
  Tcl_Thread_ಸರತಿ_Event(p->ಪೋಷಕ, (Tcl_Event *)pEvent, TCL_ಸರತಿ_ಬಾಲ);
  Tcl_ThreadAlert(p->ಪೋಷಕ);

  ಹಿಂತಿರುಗಿ TCL_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ xBusy(ಶೂನ್ಯ *pArg, ಪೂರ್ಣಾಂಕ nBusy){
  ಬಳಸದ_PARAMETER(pArg);
  ಬಳಸದ_PARAMETER(nBusy);
  sqlite3_sleep(50);
  ಹಿಂತಿರುಗಿ 1;             /* Try again... */
}

/*
** sqlthread open
**
**     Open a database handle and return the string representation of
**     the pointer value.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI sqlthread_open(
  Client_ದತ್ತಾಂಶ client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಂಕ sqlite3_Test_Make_ಸೂಚಕ_Str(Tcl_Interp *interp, ಅಕ್ಷರ *zPtr, ಶೂನ್ಯ *p);

  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFilename;
  sqlite3 *db;
  ಅಕ್ಷರ zBuf[100];
  ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ Md5_ರಿಜಿಸ್ಟರ್(sqlite3*,ಅಕ್ಷರ**,ಸ್ಥಿರಾಂಕ sqlite3_api_routines*);

  ಬಳಸದ_PARAMETER(client_ದತ್ತಾಂಶ);
  ಬಳಸದ_PARAMETER(objc);

  zFilename = Tcl_GetString(objv[2]);
  sqlite3_open(zFilename, &db);
  Md5_ರಿಜಿಸ್ಟರ್(db, 0, 0);
  sqlite3_busy_handler(db, xBusy, 0);
  
  ಆದರೆ( sqlite3_Test_Make_ಸೂಚಕ_Str(interp, zBuf, db) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  Tcl_Append_ಫಲಿತಾಂಶ(interp, zBuf, ಖಾಲಿ);

  ಹಿಂತಿರುಗಿ TCL_OK;
}


/*
** sqlthread open
**
**     Return the current thread-id (Tcl_GetCurrentThread()) cast to
**     an integer.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI sqlthread_id(
  Client_ದತ್ತಾಂಶ client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_ThreadId id = Tcl_GetCurrentThread();
  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewIntObj(SQLITE_PTR_ಗಮ್ಯ_INT(id)));
  ಬಳಸದ_PARAMETER(client_ದತ್ತಾಂಶ);
  ಬಳಸದ_PARAMETER(objc);
  ಬಳಸದ_PARAMETER(objv);
  ಹಿಂತಿರುಗಿ TCL_OK;
}


/*
** Dispatch routine for the sub-commands of [sqlthread].
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI sqlthread_proc(
  Client_ದತ್ತಾಂಶ client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ರಚನೆ SubCommand {
    ಅಕ್ಷರ *zName;
    Tcl_ObjCmdProc *xProc;
    ಪೂರ್ಣಾಂಕ nArg;
    ಅಕ್ಷರ *zUsage;
  } aSub[] = {
    {"parent", sqlthread_ಪೋಷಕ, 1, "SCRIPT"},
    {"spawn",  sqlthread_spawn,  2, "VARNAME SCRIPT"},
    {"open",   sqlthread_open,   1, "DBNAME"},
    {"id",     sqlthread_id,     0, ""},
    {0, 0, 0}
  };
  ರಚನೆ SubCommand *pSub;
  ಪೂರ್ಣಾಂಕ rc;
  ಪೂರ್ಣಾಂಕ i_ಸೂಚ್ಯಂಕ;

  ಆದರೆ( objc<2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "SUB-COMMAND");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  rc = Tcl_Get_ಸೂಚ್ಯಂಕ_From_Obj_Struct(
      interp, objv[1], aSub, ಗಾತ್ರ(aSub[0]), "sub-command", 0, &i_ಸೂಚ್ಯಂಕ
  );
  ಆದರೆ( rc!=TCL_OK ) ಹಿಂತಿರುಗಿ rc;
  pSub = &aSub[i_ಸೂಚ್ಯಂಕ];

  ಆದರೆ( objc<(pSub->nArg+2) ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 2, objv, pSub->zUsage);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  ಹಿಂತಿರುಗಿ pSub->xProc(client_ದತ್ತಾಂಶ, interp, objc, objv);
}

/*
** The [clock_seconds] command. This is more or less the same as the
** regular tcl [clock seconds], except that it is available in testfixture
** when linked against both Tcl 8.4 and 8.5. Because [clock seconds] is
** implemented as a script in Tcl 8.5, it is not usually available to
** testfixture.
*/ 
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI clock_seconds_proc(
  Client_ದತ್ತಾಂಶ client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_ಸಮಯ now;
  Tcl_Get_ಸಮಯ(&now);
  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewIntObj(now.sec));
  ಬಳಸದ_PARAMETER(client_ದತ್ತಾಂಶ);
  ಬಳಸದ_PARAMETER(objc);
  ಬಳಸದ_PARAMETER(objv);
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** The [clock_milliseconds] command. This is more or less the same as the
** regular tcl [clock milliseconds]. 
*/ 
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI clock_milliseconds_proc(
  Client_ದತ್ತಾಂಶ client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_ಸಮಯ now;
  Tcl_Get_ಸಮಯ(&now);
  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewWideIntObj(
    ((Tcl_WideInt)now.sec * 1000) + (now.usec / 1000)
  ));
  ಬಳಸದ_PARAMETER(client_ದತ್ತಾಂಶ);
  ಬಳಸದ_PARAMETER(objc);
  ಬಳಸದ_PARAMETER(objv);
  ಹಿಂತಿರುಗಿ TCL_OK;
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
#ಆದರೆ SQLITE_OS_UNIX && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)

/* BEGIN_SQLITE_BLOCKING_STEP */
/* This example uses the pthreads API */
#ಅನ್ತರ್ಭಾವ <pthread.h>

/*
** A pointer to an instance of this structure is passed as the user-context
** pointer when registering for an unlock-notify callback.
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ UnlockNotification UnlockNotification;
ರಚನೆ UnlockNotification {
  ಪೂರ್ಣಾಂಕ fired;                         /* True after unlock event has occurred */
  pthread_cond_t cond;               /* Condition variable to wait on */
  pthread_mutex_t mutex;             /* Mutex to protect structure */
};

/*
** This function is an unlock-notify callback registered with SQLite.
*/
ಸ್ಥಿರ ಶೂನ್ಯ unlock_notify_cb(ಶೂನ್ಯ **apArg, ಪೂರ್ಣಾಂಕ nArg){
  ಪೂರ್ಣಾಂಕ i;
  ಗಾಗಿ(i=0; i<nArg; i++){
    UnlockNotification *p = (UnlockNotification *)apArg[i];
    pthread_mutex_ಬೀಗ(&p->mutex);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ wait_for_unlock_notify(sqlite3 *db){
  ಪೂರ್ಣಾಂಕ rc;
  UnlockNotification un;

  /* Initialize the UnlockNotification structure. */
  un.fired = 0;
  pthread_mutex_init(&un.mutex, 0);
  pthread_cond_init(&un.cond, 0);

  /* Register for an unlock-notify callback. */
  rc = sqlite3_unlock_notify(db, unlock_notify_cb, (ಶೂನ್ಯ *)&un);
  ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_LOCKED || rc==SQLITE_OK );

  /* The call to sqlite3_unlock_notify() always returns either SQLITE_LOCKED 
  ** or SQLITE_OK. 
  **
  ** If SQLITE_LOCKED was returned, then the system is deadlocked. In this
  ** case this function needs to return SQLITE_LOCKED to the caller so 
  ** that the current transaction can be rolled back. Otherwise, block
  ** until the unlock-notify callback is invoked, then return SQLITE_OK.
  */
  ಆದರೆ( rc==SQLITE_OK ){
    pthread_mutex_ಬೀಗ(&un.mutex);
    ಆದರೆ( !un.fired ){
      pthread_cond_wait(&un.cond, &un.mutex);
    }
    pthread_mutex_unlock(&un.mutex);
  }

  /* Destroy the mutex and condition variables. */
  pthread_cond_destroy(&un.cond);
  pthread_mutex_destroy(&un.mutex);

  ಹಿಂತಿರುಗಿ rc;
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
ಪೂರ್ಣಾಂಕ sqlite3_blocking_ಹೆಜ್ಜೆ(sqlite3_stmt *pStmt){
  ಪೂರ್ಣಾಂಕ rc;
  ಆಗ( SQLITE_LOCKED==(rc = sqlite3_ಹೆಜ್ಜೆ(pStmt)) ){
    rc = wait_for_unlock_notify(sqlite3_db_handle(pStmt));
    ಆದರೆ( rc!=SQLITE_OK ) ನಿಲ್ಲಿಸು;
    sqlite3_reset(pStmt);
  }
  ಹಿಂತಿರುಗಿ rc;
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
ಪೂರ್ಣಾಂಕ sqlite3_blocking_prepare_v2(
  sqlite3 *db,              /* Database handle. */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSql,         /* UTF-8 encoded SQL statement. */
  ಪೂರ್ಣಾಂಕ nSql,                 /* Length of zSql in bytes. */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **pz           /* OUT: End of parsed string */
){
  ಪೂರ್ಣಾಂಕ rc;
  ಆಗ( SQLITE_LOCKED==(rc = sqlite3_prepare_v2(db, zSql, nSql, ppStmt, pz)) ){
    rc = wait_for_unlock_notify(db);
    ಆದರೆ( rc!=SQLITE_OK ) ನಿಲ್ಲಿಸು;
  }
  ಹಿಂತಿರುಗಿ rc;
}
/* END_SQLITE_BLOCKING_STEP */

/*
** Usage: sqlite3_blocking_step STMT
**
** Advance the statement to the next row.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI blocking_ಹೆಜ್ಜೆ_proc(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){

  sqlite3_stmt *pStmt;
  ಪೂರ್ಣಾಂಕ rc;

  ಆದರೆ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "STMT");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  pStmt = (sqlite3_stmt*)sqlite3_Test_ಪಠ್ಯ_ಗಮ್ಯ_Ptr(Tcl_GetString(objv[1]));
  rc = sqlite3_blocking_ಹೆಜ್ಜೆ(pStmt);

  Tcl_ಗಣ_ಫಲಿತಾಂಶ(interp, (ಅಕ್ಷರ *)sqlite3ErrName(rc), 0);
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Usage: sqlite3_blocking_prepare_v2 DB sql bytes ?tailvar?
** Usage: sqlite3_nonblocking_prepare_v2 DB sql bytes ?tailvar?
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI blocking_prepare_v2_proc(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSql;
  ಪೂರ್ಣಾಂಕ ಬೈಟುಗಳು;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಬಾಲ = 0;
  sqlite3_stmt *pStmt = 0;
  ಅಕ್ಷರ zBuf[50];
  ಪೂರ್ಣಾಂಕ rc;
  ಪೂರ್ಣಾಂಕ isBlocking = !(client_ದತ್ತಾಂಶ==0);

  ಆದರೆ( objc!=5 && objc!=4 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "wrong # args: should be \"", 
       Tcl_GetString(objv[0]), " DB sql bytes tailvar", 0);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( get_Db_ಸೂಚಕ(interp, Tcl_GetString(objv[1]), &db) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  zSql = Tcl_GetString(objv[2]);
  ಆದರೆ( Tcl_GetIntFromObj(interp, objv[3], &ಬೈಟುಗಳು) ) ಹಿಂತಿರುಗಿ TCL_ERROR;

  ಆದರೆ( isBlocking ){
    rc = sqlite3_blocking_prepare_v2(db, zSql, ಬೈಟುಗಳು, &pStmt, &z_ಬಾಲ);
  }ಇಲ್ಲದಿದ್ದರೆ{
    rc = sqlite3_prepare_v2(db, zSql, ಬೈಟುಗಳು, &pStmt, &z_ಬಾಲ);
  }

  ಪ್ರತಿಜ್ಞಾ(rc==SQLITE_OK || pStmt==0);
  ಆದರೆ( z_ಬಾಲ && objc>=5 ){
    ಆದರೆ( ಬೈಟುಗಳು>=0 ){
      ಬೈಟುಗಳು = ಬೈಟುಗಳು - (z_ಬಾಲ-zSql);
    }
    Tcl_Obj_ಗಣ_Var2(interp, objv[4], 0, Tcl_NewStringObj(z_ಬಾಲ, ಬೈಟುಗಳು), 0);
  }
  ಆದರೆ( rc!=SQLITE_OK ){
    ಪ್ರತಿಜ್ಞಾ( pStmt==0 );
    sqlite3_snprintf(ಗಾತ್ರ(zBuf), zBuf, "%s ", (ಅಕ್ಷರ *)sqlite3ErrName(rc));
    Tcl_Append_ಫಲಿತಾಂಶ(interp, zBuf, sqlite3_errmsg(db), ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  ಆದರೆ( pStmt ){
    ಆದರೆ( sqlite3_Test_Make_ಸೂಚಕ_Str(interp, zBuf, pStmt) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
    Tcl_Append_ಫಲಿತಾಂಶ(interp, zBuf, ಖಾಲಿ);
  }
  ಹಿಂತಿರುಗಿ TCL_OK;
}

#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OS_UNIX && SQLITE_ENABLE_UNLOCK_NOTIFY */
/*
** End of implementation of [sqlite3_blocking_step].
************************************************************************/

/*
** Register commands with the TCL interpreter.
*/
ಪೂರ್ಣಾಂಕ SqlitetestThread_Init(Tcl_Interp *interp){
  ರಚನೆ TclCmd {
    ಪೂರ್ಣಾಂಕ (*xProc)(ಶೂನ್ಯ*, Tcl_Interp*, ಪೂರ್ಣಾಂಕ, Tcl_Obj*ಸ್ಥಿರಾಂಕ*);
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName;
    ಪೂರ್ಣಾಂಕ iCtx;
  } aCmd[] = {
    { sqlthread_proc,           "sqlthread",                      0 },
    { clock_seconds_proc,       "clock_second",                   0 },
    { clock_milliseconds_proc,  "clock_milliseconds",             0 },
#ಆದರೆ SQLITE_OS_UNIX && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)
    { blocking_ಹೆಜ್ಜೆ_proc,       "sqlite3_blocking_step",          0 },
    { blocking_prepare_v2_proc, "sqlite3_blocking_prepare_v2",    1 },
    { blocking_prepare_v2_proc, "sqlite3_nonblocking_prepare_v2", 0 },
#ಆದರೆ_ಅಂತ್ಯ
  };
  ಪೂರ್ಣಾಂಕ ii;

  ಗಾಗಿ(ii=0; ii<ಗಾತ್ರ(aCmd)/ಗಾತ್ರ(aCmd[0]); ii++){
    ಶೂನ್ಯ *p = SQLITE_INT_ಗಮ್ಯ_PTR(aCmd[ii].iCtx);
    Tcl_CreateObjCommand(interp, aCmd[ii].zName, aCmd[ii].xProc, p, 0);
  }
  ಹಿಂತಿರುಗಿ TCL_OK;
}
#ಇಲ್ಲದಿದ್ದರೆ
ಪೂರ್ಣಾಂಕ SqlitetestThread_Init(Tcl_Interp *interp){
  ಹಿಂತಿರುಗಿ TCL_OK;
}
#ಆದರೆ_ಅಂತ್ಯ
