/*
** 2003 December 18
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** Code for testing the SQLite library in a multithreaded environment.
*/
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"
#ಅನ್ತರ್ಭಾವ "tclsqlite.h"
#ಯದಿ SQLITE_OS_UNIX && SQLITE_THREADSAFE
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <pthread.h>
#ಅನ್ತರ್ಭಾವ <sched.h>
#ಅನ್ತರ್ಭಾವ <ctype.h>

ಬಾಹ್ಯ ಧ್ರುವ ವರ್ಣ *sqlite3ErrName(ಪೂರ್ಣಾಙ್ಕ);

/*
** Each thread is controlled by an instance of the following
** structure.
*/
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ Thread Thread;
ಸಂರಚನಾ Thread {
  /* The first group of fields are writable by the leader and read-only
  ** to the thread. */
  ವರ್ಣ *zFilename;       /* Name of database file */
  ಶೂನ್ಯ (*xOp)(Thread*);  /* next operation to do */
  ವರ್ಣ *zArg;            /* argument usable by xOp */
  ಪೂರ್ಣಾಙ್ಕ opnum;             /* Operation number */
  ಪೂರ್ಣಾಙ್ಕ busy;              /* True if this thread is in use */

  /* The next group of fields are writable by the thread but read-only to the
  ** leader. */
  ಪೂರ್ಣಾಙ್ಕ completed;        /* Number of operations completed */
  sqlite3 *db;           /* Open database */
  sqlite3_stmt *pStmt;     /* Pending operation */
  ವರ್ಣ *zErr;           /* operation error */
  ವರ್ಣ *zStaticErr;     /* Static error message */
  ಪೂರ್ಣಾಙ್ಕ rc;               /* operation return code */
  ಪೂರ್ಣಾಙ್ಕ argc;             /* number of columns in result */
  ಧ್ರುವ ವರ್ಣ *argv[100];    /* result columns */
  ಧ್ರುವ ವರ್ಣ *colv[100];    /* result column names */
};

/*
** There can be as many as 26 threads running at once.  Each is named
** by a capital letter: A, B, C, ..., Y, Z.
*/
#define N_THREAD 26
ಸ್ಥಿರ Thread threadset[N_THREAD];

ಸ್ಥಿರ ಶೂನ್ಯ test_barrier(){
  sqlite3_mutex *pMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_APP1);
  sqlite3_mutex_enter(pMutex);
  sqlite3_mutex_leave(pMutex);
}

/*
** The main loop for a thread.  Threads use busy waiting. 
*/
ಸ್ಥಿರ ಶೂನ್ಯ *test_thread_main(ಶೂನ್ಯ *pArg){
  Thread *p = (Thread*)pArg;
  ಯದಿ( p->db ){
    sqlite3_close(p->db);
  }
  sqlite3_open(p->zFilename, &p->db);
  ಯದಿ( SQLITE_OK!=sqlite3_errcode(p->db) ){
    p->zErr = strdup(sqlite3_errmsg(p->db));
    sqlite3_close(p->db);
    p->db = 0;
  }
  p->pStmt = 0;
  test_barrier();
  p->completed = 1;
  ಯಾವತ್( p->opnum<=p->completed ) sched_yield();
  test_barrier();
  ಯಾವತ್( p->xOp ){
    ಯದಿ( p->zErr && p->zErr!=p->zStaticErr ){
      sqlite3_free(p->zErr);
      p->zErr = 0;
    }
    (*p->xOp)(p);
    test_barrier();
    p->completed++;
    ಯಾವತ್( p->opnum<=p->completed ) sched_yield();
    test_barrier();
  }
  ಯದಿ( p->pStmt ){
    sqlite3_finalize(p->pStmt);
    p->pStmt = 0;
  }
  ಯದಿ( p->db ){
    sqlite3_close(p->db);
    p->db = 0;
  }
  ಯದಿ( p->zErr && p->zErr!=p->zStaticErr ){
    sqlite3_free(p->zErr);
    p->zErr = 0;
  }
  test_barrier();
  p->completed++;
#ifndef SQLITE_OMIT_DEPRECATED
  sqlite3_thread_cleanup();
#endif
  ಪ್ರತ್ಯಾಗಮ 0;
}

/*
** Get a thread ID which is an upper case letter.  Return the index.
** If the argument is not a valid thread ID put an error message in
** the interpreter and return -1.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ parse_thread_id(Tcl_Interp *interp, ಧ್ರುವ ವರ್ಣ *zArg){
  ಯದಿ( zArg==0 || zArg[0]==0 || zArg[1]!=0 || !isupper((unsigned ವರ್ಣ)zArg[0]) ){
    Tcl_AppendResult(interp, "thread ID must be an upper case letter", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ -1;
  }
  ಪ್ರತ್ಯಾಗಮ zArg[0] - 'A';
}

/*
** Usage:    thread_create NAME  FILENAME
**
** NAME should be an upper case letter.  Start the thread running with
** an open connection to the given database.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI tcl_thread_create(
  ಶೂನ್ಯ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ argc,              /* Number of arguments */
  ಧ್ರುವ ವರ್ಣ **argv      /* Text of each argument */
){
  ಪೂರ್ಣಾಙ್ಕ i;
  pthread_t x;
  ಪೂರ್ಣಾಙ್ಕ rc;

  ಯದಿ( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID FILENAME", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ಯದಿ( i<0 ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  ಯದಿ( threadset[i].busy ){
    Tcl_AppendResult(interp, "thread ", argv[1], " is already running", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  threadset[i].busy = 1;
  sqlite3_free(threadset[i].zFilename);
  threadset[i].zFilename = sqlite3_mprintf("%s", argv[2]);
  threadset[i].opnum = 1;
  threadset[i].completed = 0;
  rc = pthread_create(&x, 0, test_thread_main, &threadset[i]);
  ಯದಿ( rc ){
    Tcl_AppendResult(interp, "failed to create the thread", ರಿಕ್ತ);
    sqlite3_free(threadset[i].zFilename);
    threadset[i].busy = 0;
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  pthread_detach(x);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** Wait for a thread to reach its idle state.
*/
ಸ್ಥಿರ ಶೂನ್ಯ test_thread_wait(Thread *p){
  test_barrier();
  ಯಾವತ್( p->opnum>p->completed ) sched_yield();
  test_barrier();
}

/*
** Usage:  thread_wait ID
**
** Wait on thread ID to reach its idle state.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI tcl_thread_wait(
  ಶೂನ್ಯ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ argc,              /* Number of arguments */
  ಧ್ರುವ ವರ್ಣ **argv      /* Text of each argument */
){
  ಪೂರ್ಣಾಙ್ಕ i;

  ಯದಿ( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ಯದಿ( i<0 ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  ಯದಿ( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** Stop a thread.
*/
ಸ್ಥಿರ ಶೂನ್ಯ test_stop_thread(Thread *p){
  test_thread_wait(p);
  p->xOp = 0;
  p->opnum++;
  test_thread_wait(p);
  sqlite3_free(p->zArg);
  p->zArg = 0;
  sqlite3_free(p->zFilename);
  p->zFilename = 0;
  p->busy = 0;
}

/*
** Usage:  thread_halt ID
**
** Cause a thread to shut itself down.  Wait for the shutdown to be
** completed.  If ID is "*" then stop all threads.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI tcl_thread_halt(
  ಶೂನ್ಯ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ argc,              /* Number of arguments */
  ಧ್ರುವ ವರ್ಣ **argv      /* Text of each argument */
){
  ಪೂರ್ಣಾಙ್ಕ i;

  ಯದಿ( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( argv[1][0]=='*' && argv[1][1]==0 ){
    ಪ್ರತಿ(i=0; i<N_THREAD; i++){
      ಯದಿ( threadset[i].busy ) test_stop_thread(&threadset[i]);
    }
  }ಅನ್ಯಥಾ{
    i = parse_thread_id(interp, argv[1]);
    ಯದಿ( i<0 ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
    ಯದಿ( !threadset[i].busy ){
      Tcl_AppendResult(interp, "no such thread", ರಿಕ್ತ);
      ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
    }
    test_stop_thread(&threadset[i]);
  }
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** Usage: thread_argc  ID
**
** Wait on the most recent thread_step to complete, then return the
** number of columns in the result set.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI tcl_thread_argc(
  ಶೂನ್ಯ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ argc,              /* Number of arguments */
  ಧ್ರುವ ವರ್ಣ **argv      /* Text of each argument */
){
  ಪೂರ್ಣಾಙ್ಕ i;
  ವರ್ಣ zBuf[100];

  ಯದಿ( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ಯದಿ( i<0 ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  ಯದಿ( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  sqlite3_snprintf(ಪರಿಮಾಣ(zBuf), zBuf, "%d", threadset[i].argc);
  Tcl_AppendResult(interp, zBuf, ರಿಕ್ತ);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** Usage: thread_argv  ID   N
**
** Wait on the most recent thread_step to complete, then return the
** value of the N-th columns in the result set.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI tcl_thread_argv(
  ಶೂನ್ಯ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ argc,              /* Number of arguments */
  ಧ್ರುವ ವರ್ಣ **argv      /* Text of each argument */
){
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪೂರ್ಣಾಙ್ಕ n;

  ಯದಿ( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID N", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ಯದಿ( i<0 ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  ಯದಿ( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( Tcl_GetInt(interp, argv[2], &n) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  test_thread_wait(&threadset[i]);
  ಯದಿ( n<0 || n>=threadset[i].argc ){
    Tcl_AppendResult(interp, "column number out of range", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  Tcl_AppendResult(interp, threadset[i].argv[n], ರಿಕ್ತ);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** Usage: thread_colname  ID   N
**
** Wait on the most recent thread_step to complete, then return the
** name of the N-th columns in the result set.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI tcl_thread_colname(
  ಶೂನ್ಯ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ argc,              /* Number of arguments */
  ಧ್ರುವ ವರ್ಣ **argv      /* Text of each argument */
){
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪೂರ್ಣಾಙ್ಕ n;

  ಯದಿ( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID N", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ಯದಿ( i<0 ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  ಯದಿ( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( Tcl_GetInt(interp, argv[2], &n) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  test_thread_wait(&threadset[i]);
  ಯದಿ( n<0 || n>=threadset[i].argc ){
    Tcl_AppendResult(interp, "column number out of range", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  Tcl_AppendResult(interp, threadset[i].colv[n], ರಿಕ್ತ);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** Usage: thread_result  ID
**
** Wait on the most recent operation to complete, then return the
** result code from that operation.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI tcl_thread_result(
  ಶೂನ್ಯ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ argc,              /* Number of arguments */
  ಧ್ರುವ ವರ್ಣ **argv      /* Text of each argument */
){
  ಪೂರ್ಣಾಙ್ಕ i;
  ಧ್ರುವ ವರ್ಣ *zName;

  ಯದಿ( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ಯದಿ( i<0 ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  ಯದಿ( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  zName = sqlite3ErrName(threadset[i].rc);
  Tcl_AppendResult(interp, zName, ರಿಕ್ತ);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** Usage: thread_error  ID
**
** Wait on the most recent operation to complete, then return the
** error string.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI tcl_thread_error(
  ಶೂನ್ಯ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ argc,              /* Number of arguments */
  ಧ್ರುವ ವರ್ಣ **argv      /* Text of each argument */
){
  ಪೂರ್ಣಾಙ್ಕ i;

  ಯದಿ( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ಯದಿ( i<0 ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  ಯದಿ( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  Tcl_AppendResult(interp, threadset[i].zErr, ರಿಕ್ತ);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** This procedure runs in the thread to compile an SQL statement.
*/
ಸ್ಥಿರ ಶೂನ್ಯ do_compile(Thread *p){
  ಯದಿ( p->db==0 ){
    p->zErr = p->zStaticErr = "no database is open";
    p->rc = SQLITE_ERROR;
    ಪ್ರತ್ಯಾಗಮ;
  }
  ಯದಿ( p->pStmt ){
    sqlite3_finalize(p->pStmt);
    p->pStmt = 0;
  }
  p->rc = sqlite3_prepare(p->db, p->zArg, -1, &p->pStmt, 0);
}

/*
** Usage: thread_compile ID SQL
**
** Compile a new virtual machine.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI tcl_thread_compile(
  ಶೂನ್ಯ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ argc,              /* Number of arguments */
  ಧ್ರುವ ವರ್ಣ **argv      /* Text of each argument */
){
  ಪೂರ್ಣಾಙ್ಕ i;
  ಯದಿ( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID SQL", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ಯದಿ( i<0 ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  ಯದಿ( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  threadset[i].xOp = do_compile;
  sqlite3_free(threadset[i].zArg);
  threadset[i].zArg = sqlite3_mprintf("%s", argv[2]);
  test_barrier();
  threadset[i].opnum++;
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** This procedure runs in the thread to step the virtual machine.
*/
ಸ್ಥಿರ ಶೂನ್ಯ do_step(Thread *p){
  ಪೂರ್ಣಾಙ್ಕ i;
  ಯದಿ( p->pStmt==0 ){
    p->zErr = p->zStaticErr = "no virtual machine available";
    p->rc = SQLITE_ERROR;
    ಪ್ರತ್ಯಾಗಮ;
  }
  p->rc = sqlite3_step(p->pStmt);
  ಯದಿ( p->rc==SQLITE_ROW ){
    p->argc = sqlite3_column_ಎಣಿಕೆ(p->pStmt);
    ಪ್ರತಿ(i=0; i<sqlite3_data_ಎಣಿಕೆ(p->pStmt); i++){
      p->argv[i] = (ವರ್ಣ*)sqlite3_column_text(p->pStmt, i);
    }
    ಪ್ರತಿ(i=0; i<p->argc; i++){
      p->colv[i] = sqlite3_column_name(p->pStmt, i);
    }
  }
}

/*
** Usage: thread_step ID
**
** Advance the virtual machine by one step
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI tcl_thread_step(
  ಶೂನ್ಯ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ argc,              /* Number of arguments */
  ಧ್ರುವ ವರ್ಣ **argv      /* Text of each argument */
){
  ಪೂರ್ಣಾಙ್ಕ i;
  ಯದಿ( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " IDL", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ಯದಿ( i<0 ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  ಯದಿ( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  threadset[i].xOp = do_step;
  test_barrier();
  threadset[i].opnum++;
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** This procedure runs in the thread to finalize a virtual machine.
*/
ಸ್ಥಿರ ಶೂನ್ಯ do_finalize(Thread *p){
  ಯದಿ( p->pStmt==0 ){
    p->zErr = p->zStaticErr = "no virtual machine available";
    p->rc = SQLITE_ERROR;
    ಪ್ರತ್ಯಾಗಮ;
  }
  p->rc = sqlite3_finalize(p->pStmt);
  p->pStmt = 0;
}

/*
** Usage: thread_finalize ID
**
** Finalize the virtual machine.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI tcl_thread_finalize(
  ಶೂನ್ಯ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ argc,              /* Number of arguments */
  ಧ್ರುವ ವರ್ಣ **argv      /* Text of each argument */
){
  ಪೂರ್ಣಾಙ್ಕ i;
  ಯದಿ( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " IDL", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ಯದಿ( i<0 ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  ಯದಿ( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  threadset[i].xOp = do_finalize;
  sqlite3_free(threadset[i].zArg);
  threadset[i].zArg = 0;
  test_barrier();
  threadset[i].opnum++;
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** Usage: thread_swap ID ID
**
** Interchange the sqlite* pointer between two threads.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI tcl_thread_swap(
  ಶೂನ್ಯ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ argc,              /* Number of arguments */
  ಧ್ರುವ ವರ್ಣ **argv      /* Text of each argument */
){
  ಪೂರ್ಣಾಙ್ಕ i, j;
  sqlite3 *temp;
  ಯದಿ( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID1 ID2", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ಯದಿ( i<0 ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  ಯದಿ( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  j = parse_thread_id(interp, argv[2]);
  ಯದಿ( j<0 ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  ಯದಿ( !threadset[j].busy ){
    Tcl_AppendResult(interp, "no such thread", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  test_thread_wait(&threadset[j]);
  temp = threadset[i].db;
  threadset[i].db = threadset[j].db;
  threadset[j].db = temp;
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** Usage: thread_db_get ID
**
** Return the database connection pointer for the given thread.  Then
** remove the pointer from the thread itself.  Afterwards, the thread
** can be stopped and the connection can be used by the main thread.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI tcl_thread_db_get(
  ಶೂನ್ಯ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ argc,              /* Number of arguments */
  ಧ್ರುವ ವರ್ಣ **argv      /* Text of each argument */
){
  ಪೂರ್ಣಾಙ್ಕ i;
  ವರ್ಣ zBuf[100];
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ sqlite3TestMakePointerStr(Tcl_Interp*, ವರ್ಣ*, ಶೂನ್ಯ*);
  ಯದಿ( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ಯದಿ( i<0 ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  ಯದಿ( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  sqlite3TestMakePointerStr(interp, zBuf, threadset[i].db);
  threadset[i].db = 0;
  Tcl_AppendResult(interp, zBuf, (ವರ್ಣ*)0);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** Usage: thread_db_put ID DB
**
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI tcl_thread_db_put(
  ಶೂನ್ಯ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ argc,              /* Number of arguments */
  ಧ್ರುವ ವರ್ಣ **argv      /* Text of each argument */
){
  ಪೂರ್ಣಾಙ್ಕ i;
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ sqlite3TestMakePointerStr(Tcl_Interp*, ವರ್ಣ*, ಶೂನ್ಯ*);
  ಬಾಹ್ಯ ಶೂನ್ಯ *sqlite3TestTextToPtr(ಧ್ರುವ ವರ್ಣ *);
  ಯದಿ( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID DB", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ಯದಿ( i<0 ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  ಯದಿ( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  ಪ್ರತಿಜ್ಞಾ( !threadset[i].db );
  threadset[i].db = (sqlite3*)sqlite3TestTextToPtr(argv[2]);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** Usage: thread_stmt_get ID
**
** Return the database stmt pointer for the given thread.  Then
** remove the pointer from the thread itself. 
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI tcl_thread_stmt_get(
  ಶೂನ್ಯ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ argc,              /* Number of arguments */
  ಧ್ರುವ ವರ್ಣ **argv      /* Text of each argument */
){
  ಪೂರ್ಣಾಙ್ಕ i;
  ವರ್ಣ zBuf[100];
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ sqlite3TestMakePointerStr(Tcl_Interp*, ವರ್ಣ*, ಶೂನ್ಯ*);
  ಯದಿ( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ಯದಿ( i<0 ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  ಯದಿ( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  sqlite3TestMakePointerStr(interp, zBuf, threadset[i].pStmt);
  threadset[i].pStmt = 0;
  Tcl_AppendResult(interp, zBuf, (ವರ್ಣ*)0);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
ಪೂರ್ಣಾಙ್ಕ Sqlitetest4_Init(Tcl_Interp *interp){
  ಸ್ಥಿರ ಸಂರಚನಾ {
     ವರ್ಣ *zName;
     Tcl_CmdProc *xProc;
  } aCmd[] = {
     { "thread_create",     (Tcl_CmdProc*)tcl_thread_create     },
     { "thread_wait",       (Tcl_CmdProc*)tcl_thread_wait       },
     { "thread_halt",       (Tcl_CmdProc*)tcl_thread_halt       },
     { "thread_argc",       (Tcl_CmdProc*)tcl_thread_argc       },
     { "thread_argv",       (Tcl_CmdProc*)tcl_thread_argv       },
     { "thread_colname",    (Tcl_CmdProc*)tcl_thread_colname    },
     { "thread_result",     (Tcl_CmdProc*)tcl_thread_result     },
     { "thread_error",      (Tcl_CmdProc*)tcl_thread_error      },
     { "thread_compile",    (Tcl_CmdProc*)tcl_thread_compile    },
     { "thread_step",       (Tcl_CmdProc*)tcl_thread_step       },
     { "thread_finalize",   (Tcl_CmdProc*)tcl_thread_finalize   },
     { "thread_swap",       (Tcl_CmdProc*)tcl_thread_swap       },
     { "thread_db_get",     (Tcl_CmdProc*)tcl_thread_db_get     },
     { "thread_db_put",     (Tcl_CmdProc*)tcl_thread_db_put     },
     { "thread_stmt_get",   (Tcl_CmdProc*)tcl_thread_stmt_get   },
  };
  ಪೂರ್ಣಾಙ್ಕ i;

  ಪ್ರತಿ(i=0; i<ಪರಿಮಾಣ(aCmd)/ಪರಿಮಾಣ(aCmd[0]); i++){
    Tcl_CreateCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}
#ಅನ್ಯಥಾ
ಪೂರ್ಣಾಙ್ಕ Sqlitetest4_Init(Tcl_Interp *interp){ ಪ್ರತ್ಯಾಗಮ TCL_OK; }
#endif /* SQLITE_OS_UNIX */
