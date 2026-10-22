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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "tclsqlite.h"
#ਜੇ SQLITE_OS_UNIX && SQLITE_THREADSAFE
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <pthread.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sched.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ctype.h>

ਬਾਹਰੀ ਅਚਲ ਅੱਖਰ *sqlite3ErrName(ਪੂਰਨ);

/*
** Each thread is controlled by an instance of the following
** structure.
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Thread Thread;
ਬਣਤਰ Thread {
  /* The first group of fields are writable by the leader and read-only
  ** to the thread. */
  ਅੱਖਰ *zFilename;       /* Name of database file */
  ਖਾਲੀ (*xOp)(Thread*);  /* next operation to do */
  ਅੱਖਰ *zArg;            /* argument usable by xOp */
  ਪੂਰਨ opnum;             /* Operation number */
  ਪੂਰਨ busy;              /* True if this thread is in use */

  /* The next group of fields are writable by the thread but read-only to the
  ** leader. */
  ਪੂਰਨ completed;        /* Number of operations completed */
  sqlite3 *db;           /* Open database */
  sqlite3_stmt *pStmt;     /* Pending operation */
  ਅੱਖਰ *zErr;           /* operation error */
  ਅੱਖਰ *zStaticErr;     /* Static error message */
  ਪੂਰਨ rc;               /* operation return code */
  ਪੂਰਨ argc;             /* number of columns in result */
  ਅਚਲ ਅੱਖਰ *argv[100];    /* result columns */
  ਅਚਲ ਅੱਖਰ *colv[100];    /* result column names */
};

/*
** There can be as many as 26 threads running at once.  Each is named
** by a capital letter: A, B, C, ..., Y, Z.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ N_THREAD 26
ਸਥਿਰ Thread threadset[N_THREAD];

ਸਥਿਰ ਖਾਲੀ test_barrier(){
  sqlite3_mutex *pMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_APP1);
  sqlite3_mutex_enter(pMutex);
  sqlite3_mutex_leave(pMutex);
}

/*
** The main loop for a thread.  Threads use busy waiting. 
*/
ਸਥਿਰ ਖਾਲੀ *test_thread_main(ਖਾਲੀ *pArg){
  Thread *p = (Thread*)pArg;
  ਜੇ( p->db ){
    sqlite3_close(p->db);
  }
  sqlite3_open(p->zFilename, &p->db);
  ਜੇ( SQLITE_OK!=sqlite3_errcode(p->db) ){
    p->zErr = strdup(sqlite3_errmsg(p->db));
    sqlite3_close(p->db);
    p->db = 0;
  }
  p->pStmt = 0;
  test_barrier();
  p->completed = 1;
  ਜਦੋਂ( p->opnum<=p->completed ) sched_yield();
  test_barrier();
  ਜਦੋਂ( p->xOp ){
    ਜੇ( p->zErr && p->zErr!=p->zStaticErr ){
      sqlite3_free(p->zErr);
      p->zErr = 0;
    }
    (*p->xOp)(p);
    test_barrier();
    p->completed++;
    ਜਦੋਂ( p->opnum<=p->completed ) sched_yield();
    test_barrier();
  }
  ਜੇ( p->pStmt ){
    sqlite3_finalize(p->pStmt);
    p->pStmt = 0;
  }
  ਜੇ( p->db ){
    sqlite3_close(p->db);
    p->db = 0;
  }
  ਜੇ( p->zErr && p->zErr!=p->zStaticErr ){
    sqlite3_free(p->zErr);
    p->zErr = 0;
  }
  test_barrier();
  p->completed++;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_DEPRECATED
  sqlite3_thread_cleanup();
#ਜੇ_ਅੰਤ
  ਵਾਪਸ 0;
}

/*
** Get a thread ID which is an upper case letter.  Return the index.
** If the argument is not a valid thread ID put an error message in
** the interpreter and return -1.
*/
ਸਥਿਰ ਪੂਰਨ parse_thread_id(Tcl_Interp *interp, ਅਚਲ ਅੱਖਰ *zArg){
  ਜੇ( zArg==0 || zArg[0]==0 || zArg[1]!=0 || !isupper((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)zArg[0]) ){
    Tcl_Append_ਨਤੀਜਾ(interp, "thread ID must be an upper case letter", ਨਿਰਾ);
    ਵਾਪਸ -1;
  }
  ਵਾਪਸ zArg[0] - 'A';
}

/*
** Usage:    thread_create NAME  FILENAME
**
** NAME should be an upper case letter.  Start the thread running with
** an open connection to the given database.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI tcl_thread_create(
  ਖਾਲੀ *Not_ਵਰਤਿਆ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ argc,              /* Number of arguments */
  ਅਚਲ ਅੱਖਰ **argv      /* Text of each argument */
){
  ਪੂਰਨ i;
  pthread_t x;
  ਪੂਰਨ rc;

  ਜੇ( argc!=3 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "wrong # args: should be \"", argv[0],
       " ID FILENAME", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ਜੇ( i<0 ) ਵਾਪਸ TCL_ERROR;
  ਜੇ( threadset[i].busy ){
    Tcl_Append_ਨਤੀਜਾ(interp, "thread ", argv[1], " is already running", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  threadset[i].busy = 1;
  sqlite3_free(threadset[i].zFilename);
  threadset[i].zFilename = sqlite3_mprintf("%s", argv[2]);
  threadset[i].opnum = 1;
  threadset[i].completed = 0;
  rc = pthread_create(&x, 0, test_thread_main, &threadset[i]);
  ਜੇ( rc ){
    Tcl_Append_ਨਤੀਜਾ(interp, "failed to create the thread", ਨਿਰਾ);
    sqlite3_free(threadset[i].zFilename);
    threadset[i].busy = 0;
    ਵਾਪਸ TCL_ERROR;
  }
  pthread_detach(x);
  ਵਾਪਸ TCL_OK;
}

/*
** Wait for a thread to reach its idle state.
*/
ਸਥਿਰ ਖਾਲੀ test_thread_wait(Thread *p){
  test_barrier();
  ਜਦੋਂ( p->opnum>p->completed ) sched_yield();
  test_barrier();
}

/*
** Usage:  thread_wait ID
**
** Wait on thread ID to reach its idle state.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI tcl_thread_wait(
  ਖਾਲੀ *Not_ਵਰਤਿਆ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ argc,              /* Number of arguments */
  ਅਚਲ ਅੱਖਰ **argv      /* Text of each argument */
){
  ਪੂਰਨ i;

  ਜੇ( argc!=2 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "wrong # args: should be \"", argv[0],
       " ID", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ਜੇ( i<0 ) ਵਾਪਸ TCL_ERROR;
  ਜੇ( !threadset[i].busy ){
    Tcl_Append_ਨਤੀਜਾ(interp, "no such thread", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  ਵਾਪਸ TCL_OK;
}

/*
** Stop a thread.
*/
ਸਥਿਰ ਖਾਲੀ test_stop_thread(Thread *p){
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
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI tcl_thread_halt(
  ਖਾਲੀ *Not_ਵਰਤਿਆ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ argc,              /* Number of arguments */
  ਅਚਲ ਅੱਖਰ **argv      /* Text of each argument */
){
  ਪੂਰਨ i;

  ਜੇ( argc!=2 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "wrong # args: should be \"", argv[0],
       " ID", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( argv[1][0]=='*' && argv[1][1]==0 ){
    ਲਈ(i=0; i<N_THREAD; i++){
      ਜੇ( threadset[i].busy ) test_stop_thread(&threadset[i]);
    }
  }ਨਹੀਂਤਾਂ{
    i = parse_thread_id(interp, argv[1]);
    ਜੇ( i<0 ) ਵਾਪਸ TCL_ERROR;
    ਜੇ( !threadset[i].busy ){
      Tcl_Append_ਨਤੀਜਾ(interp, "no such thread", ਨਿਰਾ);
      ਵਾਪਸ TCL_ERROR;
    }
    test_stop_thread(&threadset[i]);
  }
  ਵਾਪਸ TCL_OK;
}

/*
** Usage: thread_argc  ID
**
** Wait on the most recent thread_step to complete, then return the
** number of columns in the result set.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI tcl_thread_argc(
  ਖਾਲੀ *Not_ਵਰਤਿਆ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ argc,              /* Number of arguments */
  ਅਚਲ ਅੱਖਰ **argv      /* Text of each argument */
){
  ਪੂਰਨ i;
  ਅੱਖਰ zBuf[100];

  ਜੇ( argc!=2 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "wrong # args: should be \"", argv[0],
       " ID", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ਜੇ( i<0 ) ਵਾਪਸ TCL_ERROR;
  ਜੇ( !threadset[i].busy ){
    Tcl_Append_ਨਤੀਜਾ(interp, "no such thread", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  sqlite3_snprintf(ਆਕਾਰ(zBuf), zBuf, "%d", threadset[i].argc);
  Tcl_Append_ਨਤੀਜਾ(interp, zBuf, ਨਿਰਾ);
  ਵਾਪਸ TCL_OK;
}

/*
** Usage: thread_argv  ID   N
**
** Wait on the most recent thread_step to complete, then return the
** value of the N-th columns in the result set.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI tcl_thread_argv(
  ਖਾਲੀ *Not_ਵਰਤਿਆ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ argc,              /* Number of arguments */
  ਅਚਲ ਅੱਖਰ **argv      /* Text of each argument */
){
  ਪੂਰਨ i;
  ਪੂਰਨ n;

  ਜੇ( argc!=3 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "wrong # args: should be \"", argv[0],
       " ID N", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ਜੇ( i<0 ) ਵਾਪਸ TCL_ERROR;
  ਜੇ( !threadset[i].busy ){
    Tcl_Append_ਨਤੀਜਾ(interp, "no such thread", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( Tcl_GetInt(interp, argv[2], &n) ) ਵਾਪਸ TCL_ERROR;
  test_thread_wait(&threadset[i]);
  ਜੇ( n<0 || n>=threadset[i].argc ){
    Tcl_Append_ਨਤੀਜਾ(interp, "column number out of range", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  Tcl_Append_ਨਤੀਜਾ(interp, threadset[i].argv[n], ਨਿਰਾ);
  ਵਾਪਸ TCL_OK;
}

/*
** Usage: thread_colname  ID   N
**
** Wait on the most recent thread_step to complete, then return the
** name of the N-th columns in the result set.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI tcl_thread_colname(
  ਖਾਲੀ *Not_ਵਰਤਿਆ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ argc,              /* Number of arguments */
  ਅਚਲ ਅੱਖਰ **argv      /* Text of each argument */
){
  ਪੂਰਨ i;
  ਪੂਰਨ n;

  ਜੇ( argc!=3 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "wrong # args: should be \"", argv[0],
       " ID N", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ਜੇ( i<0 ) ਵਾਪਸ TCL_ERROR;
  ਜੇ( !threadset[i].busy ){
    Tcl_Append_ਨਤੀਜਾ(interp, "no such thread", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( Tcl_GetInt(interp, argv[2], &n) ) ਵਾਪਸ TCL_ERROR;
  test_thread_wait(&threadset[i]);
  ਜੇ( n<0 || n>=threadset[i].argc ){
    Tcl_Append_ਨਤੀਜਾ(interp, "column number out of range", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  Tcl_Append_ਨਤੀਜਾ(interp, threadset[i].colv[n], ਨਿਰਾ);
  ਵਾਪਸ TCL_OK;
}

/*
** Usage: thread_result  ID
**
** Wait on the most recent operation to complete, then return the
** result code from that operation.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI tcl_thread_ਨਤੀਜਾ(
  ਖਾਲੀ *Not_ਵਰਤਿਆ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ argc,              /* Number of arguments */
  ਅਚਲ ਅੱਖਰ **argv      /* Text of each argument */
){
  ਪੂਰਨ i;
  ਅਚਲ ਅੱਖਰ *zName;

  ਜੇ( argc!=2 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "wrong # args: should be \"", argv[0],
       " ID", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ਜੇ( i<0 ) ਵਾਪਸ TCL_ERROR;
  ਜੇ( !threadset[i].busy ){
    Tcl_Append_ਨਤੀਜਾ(interp, "no such thread", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  zName = sqlite3ErrName(threadset[i].rc);
  Tcl_Append_ਨਤੀਜਾ(interp, zName, ਨਿਰਾ);
  ਵਾਪਸ TCL_OK;
}

/*
** Usage: thread_error  ID
**
** Wait on the most recent operation to complete, then return the
** error string.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI tcl_thread_error(
  ਖਾਲੀ *Not_ਵਰਤਿਆ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ argc,              /* Number of arguments */
  ਅਚਲ ਅੱਖਰ **argv      /* Text of each argument */
){
  ਪੂਰਨ i;

  ਜੇ( argc!=2 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "wrong # args: should be \"", argv[0],
       " ID", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ਜੇ( i<0 ) ਵਾਪਸ TCL_ERROR;
  ਜੇ( !threadset[i].busy ){
    Tcl_Append_ਨਤੀਜਾ(interp, "no such thread", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  Tcl_Append_ਨਤੀਜਾ(interp, threadset[i].zErr, ਨਿਰਾ);
  ਵਾਪਸ TCL_OK;
}

/*
** This procedure runs in the thread to compile an SQL statement.
*/
ਸਥਿਰ ਖਾਲੀ do_compile(Thread *p){
  ਜੇ( p->db==0 ){
    p->zErr = p->zStaticErr = "no database is open";
    p->rc = SQLITE_ERROR;
    ਵਾਪਸ;
  }
  ਜੇ( p->pStmt ){
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
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI tcl_thread_compile(
  ਖਾਲੀ *Not_ਵਰਤਿਆ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ argc,              /* Number of arguments */
  ਅਚਲ ਅੱਖਰ **argv      /* Text of each argument */
){
  ਪੂਰਨ i;
  ਜੇ( argc!=3 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "wrong # args: should be \"", argv[0],
       " ID SQL", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ਜੇ( i<0 ) ਵਾਪਸ TCL_ERROR;
  ਜੇ( !threadset[i].busy ){
    Tcl_Append_ਨਤੀਜਾ(interp, "no such thread", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  threadset[i].xOp = do_compile;
  sqlite3_free(threadset[i].zArg);
  threadset[i].zArg = sqlite3_mprintf("%s", argv[2]);
  test_barrier();
  threadset[i].opnum++;
  ਵਾਪਸ TCL_OK;
}

/*
** This procedure runs in the thread to step the virtual machine.
*/
ਸਥਿਰ ਖਾਲੀ do_ਕਦਮ(Thread *p){
  ਪੂਰਨ i;
  ਜੇ( p->pStmt==0 ){
    p->zErr = p->zStaticErr = "no virtual machine available";
    p->rc = SQLITE_ERROR;
    ਵਾਪਸ;
  }
  p->rc = sqlite3_ਕਦਮ(p->pStmt);
  ਜੇ( p->rc==SQLITE_ਪੰਕਤੀ ){
    p->argc = sqlite3_ਕਾਲਮ_ਗਿਣਤੀ(p->pStmt);
    ਲਈ(i=0; i<sqlite3_ਅੰਕੜੇ_ਗਿਣਤੀ(p->pStmt); i++){
      p->argv[i] = (ਅੱਖਰ*)sqlite3_ਕਾਲਮ_ਪਾਠ(p->pStmt, i);
    }
    ਲਈ(i=0; i<p->argc; i++){
      p->colv[i] = sqlite3_ਕਾਲਮ_name(p->pStmt, i);
    }
  }
}

/*
** Usage: thread_step ID
**
** Advance the virtual machine by one step
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI tcl_thread_ਕਦਮ(
  ਖਾਲੀ *Not_ਵਰਤਿਆ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ argc,              /* Number of arguments */
  ਅਚਲ ਅੱਖਰ **argv      /* Text of each argument */
){
  ਪੂਰਨ i;
  ਜੇ( argc!=2 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "wrong # args: should be \"", argv[0],
       " IDL", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ਜੇ( i<0 ) ਵਾਪਸ TCL_ERROR;
  ਜੇ( !threadset[i].busy ){
    Tcl_Append_ਨਤੀਜਾ(interp, "no such thread", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  threadset[i].xOp = do_ਕਦਮ;
  test_barrier();
  threadset[i].opnum++;
  ਵਾਪਸ TCL_OK;
}

/*
** This procedure runs in the thread to finalize a virtual machine.
*/
ਸਥਿਰ ਖਾਲੀ do_finalize(Thread *p){
  ਜੇ( p->pStmt==0 ){
    p->zErr = p->zStaticErr = "no virtual machine available";
    p->rc = SQLITE_ERROR;
    ਵਾਪਸ;
  }
  p->rc = sqlite3_finalize(p->pStmt);
  p->pStmt = 0;
}

/*
** Usage: thread_finalize ID
**
** Finalize the virtual machine.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI tcl_thread_finalize(
  ਖਾਲੀ *Not_ਵਰਤਿਆ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ argc,              /* Number of arguments */
  ਅਚਲ ਅੱਖਰ **argv      /* Text of each argument */
){
  ਪੂਰਨ i;
  ਜੇ( argc!=2 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "wrong # args: should be \"", argv[0],
       " IDL", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ਜੇ( i<0 ) ਵਾਪਸ TCL_ERROR;
  ਜੇ( !threadset[i].busy ){
    Tcl_Append_ਨਤੀਜਾ(interp, "no such thread", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  threadset[i].xOp = do_finalize;
  sqlite3_free(threadset[i].zArg);
  threadset[i].zArg = 0;
  test_barrier();
  threadset[i].opnum++;
  ਵਾਪਸ TCL_OK;
}

/*
** Usage: thread_swap ID ID
**
** Interchange the sqlite* pointer between two threads.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI tcl_thread_ਅਦਲਾ_ਬਦਲੀ(
  ਖਾਲੀ *Not_ਵਰਤਿਆ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ argc,              /* Number of arguments */
  ਅਚਲ ਅੱਖਰ **argv      /* Text of each argument */
){
  ਪੂਰਨ i, j;
  sqlite3 *ਆਰਜ਼ੀ;
  ਜੇ( argc!=3 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "wrong # args: should be \"", argv[0],
       " ID1 ID2", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ਜੇ( i<0 ) ਵਾਪਸ TCL_ERROR;
  ਜੇ( !threadset[i].busy ){
    Tcl_Append_ਨਤੀਜਾ(interp, "no such thread", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  j = parse_thread_id(interp, argv[2]);
  ਜੇ( j<0 ) ਵਾਪਸ TCL_ERROR;
  ਜੇ( !threadset[j].busy ){
    Tcl_Append_ਨਤੀਜਾ(interp, "no such thread", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  test_thread_wait(&threadset[j]);
  ਆਰਜ਼ੀ = threadset[i].db;
  threadset[i].db = threadset[j].db;
  threadset[j].db = ਆਰਜ਼ੀ;
  ਵਾਪਸ TCL_OK;
}

/*
** Usage: thread_db_get ID
**
** Return the database connection pointer for the given thread.  Then
** remove the pointer from the thread itself.  Afterwards, the thread
** can be stopped and the connection can be used by the main thread.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI tcl_thread_db_get(
  ਖਾਲੀ *Not_ਵਰਤਿਆ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ argc,              /* Number of arguments */
  ਅਚਲ ਅੱਖਰ **argv      /* Text of each argument */
){
  ਪੂਰਨ i;
  ਅੱਖਰ zBuf[100];
  ਬਾਹਰੀ ਪੂਰਨ sqlite3_Test_Make_ਸੂਚਕ_Str(Tcl_Interp*, ਅੱਖਰ*, ਖਾਲੀ*);
  ਜੇ( argc!=2 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "wrong # args: should be \"", argv[0],
       " ID", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ਜੇ( i<0 ) ਵਾਪਸ TCL_ERROR;
  ਜੇ( !threadset[i].busy ){
    Tcl_Append_ਨਤੀਜਾ(interp, "no such thread", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  sqlite3_Test_Make_ਸੂਚਕ_Str(interp, zBuf, threadset[i].db);
  threadset[i].db = 0;
  Tcl_Append_ਨਤੀਜਾ(interp, zBuf, (ਅੱਖਰ*)0);
  ਵਾਪਸ TCL_OK;
}

/*
** Usage: thread_db_put ID DB
**
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI tcl_thread_db_put(
  ਖਾਲੀ *Not_ਵਰਤਿਆ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ argc,              /* Number of arguments */
  ਅਚਲ ਅੱਖਰ **argv      /* Text of each argument */
){
  ਪੂਰਨ i;
  ਬਾਹਰੀ ਪੂਰਨ sqlite3_Test_Make_ਸੂਚਕ_Str(Tcl_Interp*, ਅੱਖਰ*, ਖਾਲੀ*);
  ਬਾਹਰੀ ਖਾਲੀ *sqlite3_Test_ਪਾਠ_ਮੰਜ਼ਿਲ_Ptr(ਅਚਲ ਅੱਖਰ *);
  ਜੇ( argc!=3 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "wrong # args: should be \"", argv[0],
       " ID DB", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ਜੇ( i<0 ) ਵਾਪਸ TCL_ERROR;
  ਜੇ( !threadset[i].busy ){
    Tcl_Append_ਨਤੀਜਾ(interp, "no such thread", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  ਪ੍ਰਤਿਜ੍ਞਾ( !threadset[i].db );
  threadset[i].db = (sqlite3*)sqlite3_Test_ਪਾਠ_ਮੰਜ਼ਿਲ_Ptr(argv[2]);
  ਵਾਪਸ TCL_OK;
}

/*
** Usage: thread_stmt_get ID
**
** Return the database stmt pointer for the given thread.  Then
** remove the pointer from the thread itself. 
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI tcl_thread_stmt_get(
  ਖਾਲੀ *Not_ਵਰਤਿਆ,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ argc,              /* Number of arguments */
  ਅਚਲ ਅੱਖਰ **argv      /* Text of each argument */
){
  ਪੂਰਨ i;
  ਅੱਖਰ zBuf[100];
  ਬਾਹਰੀ ਪੂਰਨ sqlite3_Test_Make_ਸੂਚਕ_Str(Tcl_Interp*, ਅੱਖਰ*, ਖਾਲੀ*);
  ਜੇ( argc!=2 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "wrong # args: should be \"", argv[0],
       " ID", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ਜੇ( i<0 ) ਵਾਪਸ TCL_ERROR;
  ਜੇ( !threadset[i].busy ){
    Tcl_Append_ਨਤੀਜਾ(interp, "no such thread", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  sqlite3_Test_Make_ਸੂਚਕ_Str(interp, zBuf, threadset[i].pStmt);
  threadset[i].pStmt = 0;
  Tcl_Append_ਨਤੀਜਾ(interp, zBuf, (ਅੱਖਰ*)0);
  ਵਾਪਸ TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
ਪੂਰਨ Sqlitetest4_Init(Tcl_Interp *interp){
  ਸਥਿਰ ਬਣਤਰ {
     ਅੱਖਰ *zName;
     Tcl_CmdProc *xProc;
  } aCmd[] = {
     { "thread_create",     (Tcl_CmdProc*)tcl_thread_create     },
     { "thread_wait",       (Tcl_CmdProc*)tcl_thread_wait       },
     { "thread_halt",       (Tcl_CmdProc*)tcl_thread_halt       },
     { "thread_argc",       (Tcl_CmdProc*)tcl_thread_argc       },
     { "thread_argv",       (Tcl_CmdProc*)tcl_thread_argv       },
     { "thread_colname",    (Tcl_CmdProc*)tcl_thread_colname    },
     { "thread_result",     (Tcl_CmdProc*)tcl_thread_ਨਤੀਜਾ     },
     { "thread_error",      (Tcl_CmdProc*)tcl_thread_error      },
     { "thread_compile",    (Tcl_CmdProc*)tcl_thread_compile    },
     { "thread_step",       (Tcl_CmdProc*)tcl_thread_ਕਦਮ       },
     { "thread_finalize",   (Tcl_CmdProc*)tcl_thread_finalize   },
     { "thread_swap",       (Tcl_CmdProc*)tcl_thread_ਅਦਲਾ_ਬਦਲੀ       },
     { "thread_db_get",     (Tcl_CmdProc*)tcl_thread_db_get     },
     { "thread_db_put",     (Tcl_CmdProc*)tcl_thread_db_put     },
     { "thread_stmt_get",   (Tcl_CmdProc*)tcl_thread_stmt_get   },
  };
  ਪੂਰਨ i;

  ਲਈ(i=0; i<ਆਕਾਰ(aCmd)/ਆਕਾਰ(aCmd[0]); i++){
    Tcl_CreateCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }
  ਵਾਪਸ TCL_OK;
}
#ਨਹੀਂਤਾਂ
ਪੂਰਨ Sqlitetest4_Init(Tcl_Interp *interp){ ਵਾਪਸ TCL_OK; }
#ਜੇ_ਅੰਤ /* SQLITE_OS_UNIX */
