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
#ಆದರೆ SQLITE_OS_UNIX && SQLITE_THREADSAFE
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <pthread.h>
#ಅನ್ತರ್ಭಾವ <sched.h>
#ಅನ್ತರ್ಭಾವ <ctype.h>

ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3ErrName(ಪೂರ್ಣಾಂಕ);

/*
** Each thread is controlled by an instance of the following
** structure.
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Thread Thread;
ರಚನೆ Thread {
  /* The first group of fields are writable by the leader and read-only
  ** to the thread. */
  ಅಕ್ಷರ *zFilename;       /* Name of database file */
  ಶೂನ್ಯ (*xOp)(Thread*);  /* next operation to do */
  ಅಕ್ಷರ *zArg;            /* argument usable by xOp */
  ಪೂರ್ಣಾಂಕ opnum;             /* Operation number */
  ಪೂರ್ಣಾಂಕ busy;              /* True if this thread is in use */

  /* The next group of fields are writable by the thread but read-only to the
  ** leader. */
  ಪೂರ್ಣಾಂಕ completed;        /* Number of operations completed */
  sqlite3 *db;           /* Open database */
  sqlite3_stmt *pStmt;     /* Pending operation */
  ಅಕ್ಷರ *zErr;           /* operation error */
  ಅಕ್ಷರ *zStaticErr;     /* Static error message */
  ಪೂರ್ಣಾಂಕ rc;               /* operation return code */
  ಪೂರ್ಣಾಂಕ argc;             /* number of columns in result */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *argv[100];    /* result columns */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *colv[100];    /* result column names */
};

/*
** There can be as many as 26 threads running at once.  Each is named
** by a capital letter: A, B, C, ..., Y, Z.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು N_THREAD 26
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
  ಆದರೆ( p->db ){
    sqlite3_close(p->db);
  }
  sqlite3_open(p->zFilename, &p->db);
  ಆದರೆ( SQLITE_OK!=sqlite3_errcode(p->db) ){
    p->zErr = strdup(sqlite3_errmsg(p->db));
    sqlite3_close(p->db);
    p->db = 0;
  }
  p->pStmt = 0;
  test_barrier();
  p->completed = 1;
  ಆಗ( p->opnum<=p->completed ) sched_yield();
  test_barrier();
  ಆಗ( p->xOp ){
    ಆದರೆ( p->zErr && p->zErr!=p->zStaticErr ){
      sqlite3_free(p->zErr);
      p->zErr = 0;
    }
    (*p->xOp)(p);
    test_barrier();
    p->completed++;
    ಆಗ( p->opnum<=p->completed ) sched_yield();
    test_barrier();
  }
  ಆದರೆ( p->pStmt ){
    sqlite3_finalize(p->pStmt);
    p->pStmt = 0;
  }
  ಆದರೆ( p->db ){
    sqlite3_close(p->db);
    p->db = 0;
  }
  ಆದರೆ( p->zErr && p->zErr!=p->zStaticErr ){
    sqlite3_free(p->zErr);
    p->zErr = 0;
  }
  test_barrier();
  p->completed++;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_DEPRECATED
  sqlite3_thread_cleanup();
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ 0;
}

/*
** Get a thread ID which is an upper case letter.  Return the index.
** If the argument is not a valid thread ID put an error message in
** the interpreter and return -1.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ parse_thread_id(Tcl_Interp *interp, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zArg){
  ಆದರೆ( zArg==0 || zArg[0]==0 || zArg[1]!=0 || !isupper((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)zArg[0]) ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "thread ID must be an upper case letter", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ -1;
  }
  ಹಿಂತಿರುಗಿ zArg[0] - 'A';
}

/*
** Usage:    thread_create NAME  FILENAME
**
** NAME should be an upper case letter.  Start the thread running with
** an open connection to the given database.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI tcl_thread_create(
  ಶೂನ್ಯ *Not_ಬಳಸಿದ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ argc,              /* Number of arguments */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **argv      /* Text of each argument */
){
  ಪೂರ್ಣಾಂಕ i;
  pthread_t x;
  ಪೂರ್ಣಾಂಕ rc;

  ಆದರೆ( argc!=3 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "wrong # args: should be \"", argv[0],
       " ID FILENAME", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ಆದರೆ( i<0 ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  ಆದರೆ( threadset[i].busy ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "thread ", argv[1], " is already running", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  threadset[i].busy = 1;
  sqlite3_free(threadset[i].zFilename);
  threadset[i].zFilename = sqlite3_mprintf("%s", argv[2]);
  threadset[i].opnum = 1;
  threadset[i].completed = 0;
  rc = pthread_create(&x, 0, test_thread_main, &threadset[i]);
  ಆದರೆ( rc ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "failed to create the thread", ಖಾಲಿ);
    sqlite3_free(threadset[i].zFilename);
    threadset[i].busy = 0;
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  pthread_detach(x);
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Wait for a thread to reach its idle state.
*/
ಸ್ಥಿರ ಶೂನ್ಯ test_thread_wait(Thread *p){
  test_barrier();
  ಆಗ( p->opnum>p->completed ) sched_yield();
  test_barrier();
}

/*
** Usage:  thread_wait ID
**
** Wait on thread ID to reach its idle state.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI tcl_thread_wait(
  ಶೂನ್ಯ *Not_ಬಳಸಿದ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ argc,              /* Number of arguments */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **argv      /* Text of each argument */
){
  ಪೂರ್ಣಾಂಕ i;

  ಆದರೆ( argc!=2 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "wrong # args: should be \"", argv[0],
       " ID", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ಆದರೆ( i<0 ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  ಆದರೆ( !threadset[i].busy ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "no such thread", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  ಹಿಂತಿರುಗಿ TCL_OK;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI tcl_thread_halt(
  ಶೂನ್ಯ *Not_ಬಳಸಿದ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ argc,              /* Number of arguments */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **argv      /* Text of each argument */
){
  ಪೂರ್ಣಾಂಕ i;

  ಆದರೆ( argc!=2 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "wrong # args: should be \"", argv[0],
       " ID", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( argv[1][0]=='*' && argv[1][1]==0 ){
    ಗಾಗಿ(i=0; i<N_THREAD; i++){
      ಆದರೆ( threadset[i].busy ) test_stop_thread(&threadset[i]);
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    i = parse_thread_id(interp, argv[1]);
    ಆದರೆ( i<0 ) ಹಿಂತಿರುಗಿ TCL_ERROR;
    ಆದರೆ( !threadset[i].busy ){
      Tcl_Append_ಫಲಿತಾಂಶ(interp, "no such thread", ಖಾಲಿ);
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }
    test_stop_thread(&threadset[i]);
  }
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Usage: thread_argc  ID
**
** Wait on the most recent thread_step to complete, then return the
** number of columns in the result set.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI tcl_thread_argc(
  ಶೂನ್ಯ *Not_ಬಳಸಿದ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ argc,              /* Number of arguments */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **argv      /* Text of each argument */
){
  ಪೂರ್ಣಾಂಕ i;
  ಅಕ್ಷರ zBuf[100];

  ಆದರೆ( argc!=2 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "wrong # args: should be \"", argv[0],
       " ID", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ಆದರೆ( i<0 ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  ಆದರೆ( !threadset[i].busy ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "no such thread", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  sqlite3_snprintf(ಗಾತ್ರ(zBuf), zBuf, "%d", threadset[i].argc);
  Tcl_Append_ಫಲಿತಾಂಶ(interp, zBuf, ಖಾಲಿ);
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Usage: thread_argv  ID   N
**
** Wait on the most recent thread_step to complete, then return the
** value of the N-th columns in the result set.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI tcl_thread_argv(
  ಶೂನ್ಯ *Not_ಬಳಸಿದ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ argc,              /* Number of arguments */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **argv      /* Text of each argument */
){
  ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ n;

  ಆದರೆ( argc!=3 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "wrong # args: should be \"", argv[0],
       " ID N", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ಆದರೆ( i<0 ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  ಆದರೆ( !threadset[i].busy ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "no such thread", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( Tcl_GetInt(interp, argv[2], &n) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  test_thread_wait(&threadset[i]);
  ಆದರೆ( n<0 || n>=threadset[i].argc ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "column number out of range", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  Tcl_Append_ಫಲಿತಾಂಶ(interp, threadset[i].argv[n], ಖಾಲಿ);
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Usage: thread_colname  ID   N
**
** Wait on the most recent thread_step to complete, then return the
** name of the N-th columns in the result set.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI tcl_thread_colname(
  ಶೂನ್ಯ *Not_ಬಳಸಿದ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ argc,              /* Number of arguments */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **argv      /* Text of each argument */
){
  ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ n;

  ಆದರೆ( argc!=3 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "wrong # args: should be \"", argv[0],
       " ID N", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ಆದರೆ( i<0 ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  ಆದರೆ( !threadset[i].busy ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "no such thread", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( Tcl_GetInt(interp, argv[2], &n) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  test_thread_wait(&threadset[i]);
  ಆದರೆ( n<0 || n>=threadset[i].argc ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "column number out of range", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  Tcl_Append_ಫಲಿತಾಂಶ(interp, threadset[i].colv[n], ಖಾಲಿ);
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Usage: thread_result  ID
**
** Wait on the most recent operation to complete, then return the
** result code from that operation.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI tcl_thread_ಫಲಿತಾಂಶ(
  ಶೂನ್ಯ *Not_ಬಳಸಿದ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ argc,              /* Number of arguments */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **argv      /* Text of each argument */
){
  ಪೂರ್ಣಾಂಕ i;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName;

  ಆದರೆ( argc!=2 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "wrong # args: should be \"", argv[0],
       " ID", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ಆದರೆ( i<0 ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  ಆದರೆ( !threadset[i].busy ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "no such thread", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  zName = sqlite3ErrName(threadset[i].rc);
  Tcl_Append_ಫಲಿತಾಂಶ(interp, zName, ಖಾಲಿ);
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Usage: thread_error  ID
**
** Wait on the most recent operation to complete, then return the
** error string.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI tcl_thread_error(
  ಶೂನ್ಯ *Not_ಬಳಸಿದ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ argc,              /* Number of arguments */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **argv      /* Text of each argument */
){
  ಪೂರ್ಣಾಂಕ i;

  ಆದರೆ( argc!=2 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "wrong # args: should be \"", argv[0],
       " ID", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ಆದರೆ( i<0 ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  ಆದರೆ( !threadset[i].busy ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "no such thread", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  Tcl_Append_ಫಲಿತಾಂಶ(interp, threadset[i].zErr, ಖಾಲಿ);
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** This procedure runs in the thread to compile an SQL statement.
*/
ಸ್ಥಿರ ಶೂನ್ಯ do_compile(Thread *p){
  ಆದರೆ( p->db==0 ){
    p->zErr = p->zStaticErr = "no database is open";
    p->rc = SQLITE_ERROR;
    ಹಿಂತಿರುಗಿ;
  }
  ಆದರೆ( p->pStmt ){
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI tcl_thread_compile(
  ಶೂನ್ಯ *Not_ಬಳಸಿದ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ argc,              /* Number of arguments */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **argv      /* Text of each argument */
){
  ಪೂರ್ಣಾಂಕ i;
  ಆದರೆ( argc!=3 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "wrong # args: should be \"", argv[0],
       " ID SQL", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ಆದರೆ( i<0 ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  ಆದರೆ( !threadset[i].busy ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "no such thread", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  threadset[i].xOp = do_compile;
  sqlite3_free(threadset[i].zArg);
  threadset[i].zArg = sqlite3_mprintf("%s", argv[2]);
  test_barrier();
  threadset[i].opnum++;
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** This procedure runs in the thread to step the virtual machine.
*/
ಸ್ಥಿರ ಶೂನ್ಯ do_ಹೆಜ್ಜೆ(Thread *p){
  ಪೂರ್ಣಾಂಕ i;
  ಆದರೆ( p->pStmt==0 ){
    p->zErr = p->zStaticErr = "no virtual machine available";
    p->rc = SQLITE_ERROR;
    ಹಿಂತಿರುಗಿ;
  }
  p->rc = sqlite3_ಹೆಜ್ಜೆ(p->pStmt);
  ಆದರೆ( p->rc==SQLITE_ಅಡ್ಡಸಾಲು ){
    p->argc = sqlite3_ಸ್ತಂಭ_ಎಣಿಕೆ(p->pStmt);
    ಗಾಗಿ(i=0; i<sqlite3_ದತ್ತಾಂಶ_ಎಣಿಕೆ(p->pStmt); i++){
      p->argv[i] = (ಅಕ್ಷರ*)sqlite3_ಸ್ತಂಭ_ಪಠ್ಯ(p->pStmt, i);
    }
    ಗಾಗಿ(i=0; i<p->argc; i++){
      p->colv[i] = sqlite3_ಸ್ತಂಭ_name(p->pStmt, i);
    }
  }
}

/*
** Usage: thread_step ID
**
** Advance the virtual machine by one step
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI tcl_thread_ಹೆಜ್ಜೆ(
  ಶೂನ್ಯ *Not_ಬಳಸಿದ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ argc,              /* Number of arguments */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **argv      /* Text of each argument */
){
  ಪೂರ್ಣಾಂಕ i;
  ಆದರೆ( argc!=2 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "wrong # args: should be \"", argv[0],
       " IDL", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ಆದರೆ( i<0 ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  ಆದರೆ( !threadset[i].busy ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "no such thread", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  threadset[i].xOp = do_ಹೆಜ್ಜೆ;
  test_barrier();
  threadset[i].opnum++;
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** This procedure runs in the thread to finalize a virtual machine.
*/
ಸ್ಥಿರ ಶೂನ್ಯ do_finalize(Thread *p){
  ಆದರೆ( p->pStmt==0 ){
    p->zErr = p->zStaticErr = "no virtual machine available";
    p->rc = SQLITE_ERROR;
    ಹಿಂತಿರುಗಿ;
  }
  p->rc = sqlite3_finalize(p->pStmt);
  p->pStmt = 0;
}

/*
** Usage: thread_finalize ID
**
** Finalize the virtual machine.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI tcl_thread_finalize(
  ಶೂನ್ಯ *Not_ಬಳಸಿದ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ argc,              /* Number of arguments */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **argv      /* Text of each argument */
){
  ಪೂರ್ಣಾಂಕ i;
  ಆದರೆ( argc!=2 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "wrong # args: should be \"", argv[0],
       " IDL", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ಆದರೆ( i<0 ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  ಆದರೆ( !threadset[i].busy ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "no such thread", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  threadset[i].xOp = do_finalize;
  sqlite3_free(threadset[i].zArg);
  threadset[i].zArg = 0;
  test_barrier();
  threadset[i].opnum++;
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Usage: thread_swap ID ID
**
** Interchange the sqlite* pointer between two threads.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI tcl_thread_ಅದಲುಬದಲು(
  ಶೂನ್ಯ *Not_ಬಳಸಿದ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ argc,              /* Number of arguments */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **argv      /* Text of each argument */
){
  ಪೂರ್ಣಾಂಕ i, j;
  sqlite3 *ತಾತ್ಕಾಲಿಕ;
  ಆದರೆ( argc!=3 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "wrong # args: should be \"", argv[0],
       " ID1 ID2", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ಆದರೆ( i<0 ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  ಆದರೆ( !threadset[i].busy ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "no such thread", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  j = parse_thread_id(interp, argv[2]);
  ಆದರೆ( j<0 ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  ಆದರೆ( !threadset[j].busy ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "no such thread", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  test_thread_wait(&threadset[j]);
  ತಾತ್ಕಾಲಿಕ = threadset[i].db;
  threadset[i].db = threadset[j].db;
  threadset[j].db = ತಾತ್ಕಾಲಿಕ;
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Usage: thread_db_get ID
**
** Return the database connection pointer for the given thread.  Then
** remove the pointer from the thread itself.  Afterwards, the thread
** can be stopped and the connection can be used by the main thread.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI tcl_thread_db_get(
  ಶೂನ್ಯ *Not_ಬಳಸಿದ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ argc,              /* Number of arguments */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **argv      /* Text of each argument */
){
  ಪೂರ್ಣಾಂಕ i;
  ಅಕ್ಷರ zBuf[100];
  ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ sqlite3_Test_Make_ಸೂಚಕ_Str(Tcl_Interp*, ಅಕ್ಷರ*, ಶೂನ್ಯ*);
  ಆದರೆ( argc!=2 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "wrong # args: should be \"", argv[0],
       " ID", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ಆದರೆ( i<0 ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  ಆದರೆ( !threadset[i].busy ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "no such thread", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  sqlite3_Test_Make_ಸೂಚಕ_Str(interp, zBuf, threadset[i].db);
  threadset[i].db = 0;
  Tcl_Append_ಫಲಿತಾಂಶ(interp, zBuf, (ಅಕ್ಷರ*)0);
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Usage: thread_db_put ID DB
**
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI tcl_thread_db_put(
  ಶೂನ್ಯ *Not_ಬಳಸಿದ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ argc,              /* Number of arguments */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **argv      /* Text of each argument */
){
  ಪೂರ್ಣಾಂಕ i;
  ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ sqlite3_Test_Make_ಸೂಚಕ_Str(Tcl_Interp*, ಅಕ್ಷರ*, ಶೂನ್ಯ*);
  ಬಾಹ್ಯ ಶೂನ್ಯ *sqlite3_Test_ಪಠ್ಯ_ಗಮ್ಯ_Ptr(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *);
  ಆದರೆ( argc!=3 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "wrong # args: should be \"", argv[0],
       " ID DB", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ಆದರೆ( i<0 ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  ಆದರೆ( !threadset[i].busy ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "no such thread", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  ಪ್ರತಿಜ್ಞಾ( !threadset[i].db );
  threadset[i].db = (sqlite3*)sqlite3_Test_ಪಠ್ಯ_ಗಮ್ಯ_Ptr(argv[2]);
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Usage: thread_stmt_get ID
**
** Return the database stmt pointer for the given thread.  Then
** remove the pointer from the thread itself. 
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI tcl_thread_stmt_get(
  ಶೂನ್ಯ *Not_ಬಳಸಿದ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ argc,              /* Number of arguments */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **argv      /* Text of each argument */
){
  ಪೂರ್ಣಾಂಕ i;
  ಅಕ್ಷರ zBuf[100];
  ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ sqlite3_Test_Make_ಸೂಚಕ_Str(Tcl_Interp*, ಅಕ್ಷರ*, ಶೂನ್ಯ*);
  ಆದರೆ( argc!=2 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "wrong # args: should be \"", argv[0],
       " ID", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ಆದರೆ( i<0 ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  ಆದರೆ( !threadset[i].busy ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "no such thread", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  sqlite3_Test_Make_ಸೂಚಕ_Str(interp, zBuf, threadset[i].pStmt);
  threadset[i].pStmt = 0;
  Tcl_Append_ಫಲಿತಾಂಶ(interp, zBuf, (ಅಕ್ಷರ*)0);
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
ಪೂರ್ಣಾಂಕ Sqlitetest4_Init(Tcl_Interp *interp){
  ಸ್ಥಿರ ರಚನೆ {
     ಅಕ್ಷರ *zName;
     Tcl_CmdProc *xProc;
  } aCmd[] = {
     { "thread_create",     (Tcl_CmdProc*)tcl_thread_create     },
     { "thread_wait",       (Tcl_CmdProc*)tcl_thread_wait       },
     { "thread_halt",       (Tcl_CmdProc*)tcl_thread_halt       },
     { "thread_argc",       (Tcl_CmdProc*)tcl_thread_argc       },
     { "thread_argv",       (Tcl_CmdProc*)tcl_thread_argv       },
     { "thread_colname",    (Tcl_CmdProc*)tcl_thread_colname    },
     { "thread_result",     (Tcl_CmdProc*)tcl_thread_ಫಲಿತಾಂಶ     },
     { "thread_error",      (Tcl_CmdProc*)tcl_thread_error      },
     { "thread_compile",    (Tcl_CmdProc*)tcl_thread_compile    },
     { "thread_step",       (Tcl_CmdProc*)tcl_thread_ಹೆಜ್ಜೆ       },
     { "thread_finalize",   (Tcl_CmdProc*)tcl_thread_finalize   },
     { "thread_swap",       (Tcl_CmdProc*)tcl_thread_ಅದಲುಬದಲು       },
     { "thread_db_get",     (Tcl_CmdProc*)tcl_thread_db_get     },
     { "thread_db_put",     (Tcl_CmdProc*)tcl_thread_db_put     },
     { "thread_stmt_get",   (Tcl_CmdProc*)tcl_thread_stmt_get   },
  };
  ಪೂರ್ಣಾಂಕ i;

  ಗಾಗಿ(i=0; i<ಗಾತ್ರ(aCmd)/ಗಾತ್ರ(aCmd[0]); i++){
    Tcl_CreateCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }
  ಹಿಂತಿರುಗಿ TCL_OK;
}
#ಇಲ್ಲದಿದ್ದರೆ
ಪೂರ್ಣಾಂಕ Sqlitetest4_Init(Tcl_Interp *interp){ ಹಿಂತಿರುಗಿ TCL_OK; }
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OS_UNIX */
