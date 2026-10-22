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
#అన్తర్భావ "sqliteInt.h"
#అన్తర్భావ "tclsqlite.h"
#ఒకవేళ SQLITE_OS_UNIX && SQLITE_THREADSAFE
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <pthread.h>
#అన్తర్భావ <sched.h>
#అన్తర్భావ <ctype.h>

బాహ్య స్థిరరాశి అక్షరం *sqlite3ErrName(పూర్ణాంకం);

/*
** Each thread is controlled by an instance of the following
** structure.
*/
రకనిర్వచనం నిర్మాణం Thread Thread;
నిర్మాణం Thread {
  /* The first group of fields are writable by the leader and read-only
  ** to the thread. */
  అక్షరం *zFilename;       /* Name of database file */
  శూన్య (*xOp)(Thread*);  /* next operation to do */
  అక్షరం *zArg;            /* argument usable by xOp */
  పూర్ణాంకం opnum;             /* Operation number */
  పూర్ణాంకం busy;              /* True if this thread is in use */

  /* The next group of fields are writable by the thread but read-only to the
  ** leader. */
  పూర్ణాంకం completed;        /* Number of operations completed */
  sqlite3 *db;           /* Open database */
  sqlite3_stmt *pStmt;     /* Pending operation */
  అక్షరం *zErr;           /* operation error */
  అక్షరం *zStaticErr;     /* Static error message */
  పూర్ణాంకం rc;               /* operation return code */
  పూర్ణాంకం argc;             /* number of columns in result */
  స్థిరరాశి అక్షరం *argv[100];    /* result columns */
  స్థిరరాశి అక్షరం *colv[100];    /* result column names */
};

/*
** There can be as many as 26 threads running at once.  Each is named
** by a capital letter: A, B, C, ..., Y, Z.
*/
#నిర్వచించు N_THREAD 26
స్థిర Thread threadset[N_THREAD];

స్థిర శూన్య test_barrier(){
  sqlite3_mutex *pMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_APP1);
  sqlite3_mutex_enter(pMutex);
  sqlite3_mutex_leave(pMutex);
}

/*
** The main loop for a thread.  Threads use busy waiting. 
*/
స్థిర శూన్య *test_thread_main(శూన్య *pArg){
  Thread *p = (Thread*)pArg;
  ఒకవేళ( p->db ){
    sqlite3_close(p->db);
  }
  sqlite3_open(p->zFilename, &p->db);
  ఒకవేళ( SQLITE_OK!=sqlite3_errcode(p->db) ){
    p->zErr = strdup(sqlite3_errmsg(p->db));
    sqlite3_close(p->db);
    p->db = 0;
  }
  p->pStmt = 0;
  test_barrier();
  p->completed = 1;
  అయితే( p->opnum<=p->completed ) sched_yield();
  test_barrier();
  అయితే( p->xOp ){
    ఒకవేళ( p->zErr && p->zErr!=p->zStaticErr ){
      sqlite3_free(p->zErr);
      p->zErr = 0;
    }
    (*p->xOp)(p);
    test_barrier();
    p->completed++;
    అయితే( p->opnum<=p->completed ) sched_yield();
    test_barrier();
  }
  ఒకవేళ( p->pStmt ){
    sqlite3_finalize(p->pStmt);
    p->pStmt = 0;
  }
  ఒకవేళ( p->db ){
    sqlite3_close(p->db);
    p->db = 0;
  }
  ఒకవేళ( p->zErr && p->zErr!=p->zStaticErr ){
    sqlite3_free(p->zErr);
    p->zErr = 0;
  }
  test_barrier();
  p->completed++;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_DEPRECATED
  sqlite3_thread_cleanup();
#అయితే_ముగింపు
  తిరిగి 0;
}

/*
** Get a thread ID which is an upper case letter.  Return the index.
** If the argument is not a valid thread ID put an error message in
** the interpreter and return -1.
*/
స్థిర పూర్ణాంకం parse_thread_id(Tcl_Interp *interp, స్థిరరాశి అక్షరం *zArg){
  ఒకవేళ( zArg==0 || zArg[0]==0 || zArg[1]!=0 || !isupper((గుర్తులేని అక్షరం)zArg[0]) ){
    Tcl_Append_ఫలితం(interp, "thread ID must be an upper case letter", ఖాళీ);
    తిరిగి -1;
  }
  తిరిగి zArg[0] - 'A';
}

/*
** Usage:    thread_create NAME  FILENAME
**
** NAME should be an upper case letter.  Start the thread running with
** an open connection to the given database.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI tcl_thread_create(
  శూన్య *Not_వాడిన,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం argc,              /* Number of arguments */
  స్థిరరాశి అక్షరం **argv      /* Text of each argument */
){
  పూర్ణాంకం i;
  pthread_t x;
  పూర్ణాంకం rc;

  ఒకవేళ( argc!=3 ){
    Tcl_Append_ఫలితం(interp, "wrong # args: should be \"", argv[0],
       " ID FILENAME", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ఒకవేళ( i<0 ) తిరిగి TCL_ERROR;
  ఒకవేళ( threadset[i].busy ){
    Tcl_Append_ఫలితం(interp, "thread ", argv[1], " is already running", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  threadset[i].busy = 1;
  sqlite3_free(threadset[i].zFilename);
  threadset[i].zFilename = sqlite3_mprintf("%s", argv[2]);
  threadset[i].opnum = 1;
  threadset[i].completed = 0;
  rc = pthread_create(&x, 0, test_thread_main, &threadset[i]);
  ఒకవేళ( rc ){
    Tcl_Append_ఫలితం(interp, "failed to create the thread", ఖాళీ);
    sqlite3_free(threadset[i].zFilename);
    threadset[i].busy = 0;
    తిరిగి TCL_ERROR;
  }
  pthread_detach(x);
  తిరిగి TCL_OK;
}

/*
** Wait for a thread to reach its idle state.
*/
స్థిర శూన్య test_thread_wait(Thread *p){
  test_barrier();
  అయితే( p->opnum>p->completed ) sched_yield();
  test_barrier();
}

/*
** Usage:  thread_wait ID
**
** Wait on thread ID to reach its idle state.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI tcl_thread_wait(
  శూన్య *Not_వాడిన,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం argc,              /* Number of arguments */
  స్థిరరాశి అక్షరం **argv      /* Text of each argument */
){
  పూర్ణాంకం i;

  ఒకవేళ( argc!=2 ){
    Tcl_Append_ఫలితం(interp, "wrong # args: should be \"", argv[0],
       " ID", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ఒకవేళ( i<0 ) తిరిగి TCL_ERROR;
  ఒకవేళ( !threadset[i].busy ){
    Tcl_Append_ఫలితం(interp, "no such thread", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  తిరిగి TCL_OK;
}

/*
** Stop a thread.
*/
స్థిర శూన్య test_stop_thread(Thread *p){
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
స్థిర పూర్ణాంకం SQLITE_TCLAPI tcl_thread_halt(
  శూన్య *Not_వాడిన,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం argc,              /* Number of arguments */
  స్థిరరాశి అక్షరం **argv      /* Text of each argument */
){
  పూర్ణాంకం i;

  ఒకవేళ( argc!=2 ){
    Tcl_Append_ఫలితం(interp, "wrong # args: should be \"", argv[0],
       " ID", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( argv[1][0]=='*' && argv[1][1]==0 ){
    కొరకు(i=0; i<N_THREAD; i++){
      ఒకవేళ( threadset[i].busy ) test_stop_thread(&threadset[i]);
    }
  }లేకపోతే{
    i = parse_thread_id(interp, argv[1]);
    ఒకవేళ( i<0 ) తిరిగి TCL_ERROR;
    ఒకవేళ( !threadset[i].busy ){
      Tcl_Append_ఫలితం(interp, "no such thread", ఖాళీ);
      తిరిగి TCL_ERROR;
    }
    test_stop_thread(&threadset[i]);
  }
  తిరిగి TCL_OK;
}

/*
** Usage: thread_argc  ID
**
** Wait on the most recent thread_step to complete, then return the
** number of columns in the result set.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI tcl_thread_argc(
  శూన్య *Not_వాడిన,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం argc,              /* Number of arguments */
  స్థిరరాశి అక్షరం **argv      /* Text of each argument */
){
  పూర్ణాంకం i;
  అక్షరం zBuf[100];

  ఒకవేళ( argc!=2 ){
    Tcl_Append_ఫలితం(interp, "wrong # args: should be \"", argv[0],
       " ID", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ఒకవేళ( i<0 ) తిరిగి TCL_ERROR;
  ఒకవేళ( !threadset[i].busy ){
    Tcl_Append_ఫలితం(interp, "no such thread", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  sqlite3_snprintf(పరిమాణం(zBuf), zBuf, "%d", threadset[i].argc);
  Tcl_Append_ఫలితం(interp, zBuf, ఖాళీ);
  తిరిగి TCL_OK;
}

/*
** Usage: thread_argv  ID   N
**
** Wait on the most recent thread_step to complete, then return the
** value of the N-th columns in the result set.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI tcl_thread_argv(
  శూన్య *Not_వాడిన,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం argc,              /* Number of arguments */
  స్థిరరాశి అక్షరం **argv      /* Text of each argument */
){
  పూర్ణాంకం i;
  పూర్ణాంకం n;

  ఒకవేళ( argc!=3 ){
    Tcl_Append_ఫలితం(interp, "wrong # args: should be \"", argv[0],
       " ID N", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ఒకవేళ( i<0 ) తిరిగి TCL_ERROR;
  ఒకవేళ( !threadset[i].busy ){
    Tcl_Append_ఫలితం(interp, "no such thread", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( Tcl_GetInt(interp, argv[2], &n) ) తిరిగి TCL_ERROR;
  test_thread_wait(&threadset[i]);
  ఒకవేళ( n<0 || n>=threadset[i].argc ){
    Tcl_Append_ఫలితం(interp, "column number out of range", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  Tcl_Append_ఫలితం(interp, threadset[i].argv[n], ఖాళీ);
  తిరిగి TCL_OK;
}

/*
** Usage: thread_colname  ID   N
**
** Wait on the most recent thread_step to complete, then return the
** name of the N-th columns in the result set.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI tcl_thread_colname(
  శూన్య *Not_వాడిన,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం argc,              /* Number of arguments */
  స్థిరరాశి అక్షరం **argv      /* Text of each argument */
){
  పూర్ణాంకం i;
  పూర్ణాంకం n;

  ఒకవేళ( argc!=3 ){
    Tcl_Append_ఫలితం(interp, "wrong # args: should be \"", argv[0],
       " ID N", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ఒకవేళ( i<0 ) తిరిగి TCL_ERROR;
  ఒకవేళ( !threadset[i].busy ){
    Tcl_Append_ఫలితం(interp, "no such thread", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( Tcl_GetInt(interp, argv[2], &n) ) తిరిగి TCL_ERROR;
  test_thread_wait(&threadset[i]);
  ఒకవేళ( n<0 || n>=threadset[i].argc ){
    Tcl_Append_ఫలితం(interp, "column number out of range", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  Tcl_Append_ఫలితం(interp, threadset[i].colv[n], ఖాళీ);
  తిరిగి TCL_OK;
}

/*
** Usage: thread_result  ID
**
** Wait on the most recent operation to complete, then return the
** result code from that operation.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI tcl_thread_ఫలితం(
  శూన్య *Not_వాడిన,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం argc,              /* Number of arguments */
  స్థిరరాశి అక్షరం **argv      /* Text of each argument */
){
  పూర్ణాంకం i;
  స్థిరరాశి అక్షరం *zName;

  ఒకవేళ( argc!=2 ){
    Tcl_Append_ఫలితం(interp, "wrong # args: should be \"", argv[0],
       " ID", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ఒకవేళ( i<0 ) తిరిగి TCL_ERROR;
  ఒకవేళ( !threadset[i].busy ){
    Tcl_Append_ఫలితం(interp, "no such thread", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  zName = sqlite3ErrName(threadset[i].rc);
  Tcl_Append_ఫలితం(interp, zName, ఖాళీ);
  తిరిగి TCL_OK;
}

/*
** Usage: thread_error  ID
**
** Wait on the most recent operation to complete, then return the
** error string.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI tcl_thread_error(
  శూన్య *Not_వాడిన,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం argc,              /* Number of arguments */
  స్థిరరాశి అక్షరం **argv      /* Text of each argument */
){
  పూర్ణాంకం i;

  ఒకవేళ( argc!=2 ){
    Tcl_Append_ఫలితం(interp, "wrong # args: should be \"", argv[0],
       " ID", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ఒకవేళ( i<0 ) తిరిగి TCL_ERROR;
  ఒకవేళ( !threadset[i].busy ){
    Tcl_Append_ఫలితం(interp, "no such thread", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  Tcl_Append_ఫలితం(interp, threadset[i].zErr, ఖాళీ);
  తిరిగి TCL_OK;
}

/*
** This procedure runs in the thread to compile an SQL statement.
*/
స్థిర శూన్య do_compile(Thread *p){
  ఒకవేళ( p->db==0 ){
    p->zErr = p->zStaticErr = "no database is open";
    p->rc = SQLITE_ERROR;
    తిరిగి;
  }
  ఒకవేళ( p->pStmt ){
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
స్థిర పూర్ణాంకం SQLITE_TCLAPI tcl_thread_compile(
  శూన్య *Not_వాడిన,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం argc,              /* Number of arguments */
  స్థిరరాశి అక్షరం **argv      /* Text of each argument */
){
  పూర్ణాంకం i;
  ఒకవేళ( argc!=3 ){
    Tcl_Append_ఫలితం(interp, "wrong # args: should be \"", argv[0],
       " ID SQL", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ఒకవేళ( i<0 ) తిరిగి TCL_ERROR;
  ఒకవేళ( !threadset[i].busy ){
    Tcl_Append_ఫలితం(interp, "no such thread", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  threadset[i].xOp = do_compile;
  sqlite3_free(threadset[i].zArg);
  threadset[i].zArg = sqlite3_mprintf("%s", argv[2]);
  test_barrier();
  threadset[i].opnum++;
  తిరిగి TCL_OK;
}

/*
** This procedure runs in the thread to step the virtual machine.
*/
స్థిర శూన్య do_అడుగు(Thread *p){
  పూర్ణాంకం i;
  ఒకవేళ( p->pStmt==0 ){
    p->zErr = p->zStaticErr = "no virtual machine available";
    p->rc = SQLITE_ERROR;
    తిరిగి;
  }
  p->rc = sqlite3_అడుగు(p->pStmt);
  ఒకవేళ( p->rc==SQLITE_అడ్డువరుస ){
    p->argc = sqlite3_స్తంభం_లెక్క(p->pStmt);
    కొరకు(i=0; i<sqlite3_దత్తాంశం_లెక్క(p->pStmt); i++){
      p->argv[i] = (అక్షరం*)sqlite3_స్తంభం_పాఠ్యం(p->pStmt, i);
    }
    కొరకు(i=0; i<p->argc; i++){
      p->colv[i] = sqlite3_స్తంభం_name(p->pStmt, i);
    }
  }
}

/*
** Usage: thread_step ID
**
** Advance the virtual machine by one step
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI tcl_thread_అడుగు(
  శూన్య *Not_వాడిన,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం argc,              /* Number of arguments */
  స్థిరరాశి అక్షరం **argv      /* Text of each argument */
){
  పూర్ణాంకం i;
  ఒకవేళ( argc!=2 ){
    Tcl_Append_ఫలితం(interp, "wrong # args: should be \"", argv[0],
       " IDL", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ఒకవేళ( i<0 ) తిరిగి TCL_ERROR;
  ఒకవేళ( !threadset[i].busy ){
    Tcl_Append_ఫలితం(interp, "no such thread", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  threadset[i].xOp = do_అడుగు;
  test_barrier();
  threadset[i].opnum++;
  తిరిగి TCL_OK;
}

/*
** This procedure runs in the thread to finalize a virtual machine.
*/
స్థిర శూన్య do_finalize(Thread *p){
  ఒకవేళ( p->pStmt==0 ){
    p->zErr = p->zStaticErr = "no virtual machine available";
    p->rc = SQLITE_ERROR;
    తిరిగి;
  }
  p->rc = sqlite3_finalize(p->pStmt);
  p->pStmt = 0;
}

/*
** Usage: thread_finalize ID
**
** Finalize the virtual machine.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI tcl_thread_finalize(
  శూన్య *Not_వాడిన,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం argc,              /* Number of arguments */
  స్థిరరాశి అక్షరం **argv      /* Text of each argument */
){
  పూర్ణాంకం i;
  ఒకవేళ( argc!=2 ){
    Tcl_Append_ఫలితం(interp, "wrong # args: should be \"", argv[0],
       " IDL", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ఒకవేళ( i<0 ) తిరిగి TCL_ERROR;
  ఒకవేళ( !threadset[i].busy ){
    Tcl_Append_ఫలితం(interp, "no such thread", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  threadset[i].xOp = do_finalize;
  sqlite3_free(threadset[i].zArg);
  threadset[i].zArg = 0;
  test_barrier();
  threadset[i].opnum++;
  తిరిగి TCL_OK;
}

/*
** Usage: thread_swap ID ID
**
** Interchange the sqlite* pointer between two threads.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI tcl_thread_మార్పిడి(
  శూన్య *Not_వాడిన,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం argc,              /* Number of arguments */
  స్థిరరాశి అక్షరం **argv      /* Text of each argument */
){
  పూర్ణాంకం i, j;
  sqlite3 *తాత్కాలిక;
  ఒకవేళ( argc!=3 ){
    Tcl_Append_ఫలితం(interp, "wrong # args: should be \"", argv[0],
       " ID1 ID2", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ఒకవేళ( i<0 ) తిరిగి TCL_ERROR;
  ఒకవేళ( !threadset[i].busy ){
    Tcl_Append_ఫలితం(interp, "no such thread", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  j = parse_thread_id(interp, argv[2]);
  ఒకవేళ( j<0 ) తిరిగి TCL_ERROR;
  ఒకవేళ( !threadset[j].busy ){
    Tcl_Append_ఫలితం(interp, "no such thread", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  test_thread_wait(&threadset[j]);
  తాత్కాలిక = threadset[i].db;
  threadset[i].db = threadset[j].db;
  threadset[j].db = తాత్కాలిక;
  తిరిగి TCL_OK;
}

/*
** Usage: thread_db_get ID
**
** Return the database connection pointer for the given thread.  Then
** remove the pointer from the thread itself.  Afterwards, the thread
** can be stopped and the connection can be used by the main thread.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI tcl_thread_db_get(
  శూన్య *Not_వాడిన,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం argc,              /* Number of arguments */
  స్థిరరాశి అక్షరం **argv      /* Text of each argument */
){
  పూర్ణాంకం i;
  అక్షరం zBuf[100];
  బాహ్య పూర్ణాంకం sqlite3_Test_Make_సూచిక_Str(Tcl_Interp*, అక్షరం*, శూన్య*);
  ఒకవేళ( argc!=2 ){
    Tcl_Append_ఫలితం(interp, "wrong # args: should be \"", argv[0],
       " ID", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ఒకవేళ( i<0 ) తిరిగి TCL_ERROR;
  ఒకవేళ( !threadset[i].busy ){
    Tcl_Append_ఫలితం(interp, "no such thread", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  sqlite3_Test_Make_సూచిక_Str(interp, zBuf, threadset[i].db);
  threadset[i].db = 0;
  Tcl_Append_ఫలితం(interp, zBuf, (అక్షరం*)0);
  తిరిగి TCL_OK;
}

/*
** Usage: thread_db_put ID DB
**
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI tcl_thread_db_put(
  శూన్య *Not_వాడిన,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం argc,              /* Number of arguments */
  స్థిరరాశి అక్షరం **argv      /* Text of each argument */
){
  పూర్ణాంకం i;
  బాహ్య పూర్ణాంకం sqlite3_Test_Make_సూచిక_Str(Tcl_Interp*, అక్షరం*, శూన్య*);
  బాహ్య శూన్య *sqlite3_Test_పాఠ్యం_గమ్యం_Ptr(స్థిరరాశి అక్షరం *);
  ఒకవేళ( argc!=3 ){
    Tcl_Append_ఫలితం(interp, "wrong # args: should be \"", argv[0],
       " ID DB", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ఒకవేళ( i<0 ) తిరిగి TCL_ERROR;
  ఒకవేళ( !threadset[i].busy ){
    Tcl_Append_ఫలితం(interp, "no such thread", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  ప్రతిజ్ఞా( !threadset[i].db );
  threadset[i].db = (sqlite3*)sqlite3_Test_పాఠ్యం_గమ్యం_Ptr(argv[2]);
  తిరిగి TCL_OK;
}

/*
** Usage: thread_stmt_get ID
**
** Return the database stmt pointer for the given thread.  Then
** remove the pointer from the thread itself. 
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI tcl_thread_stmt_get(
  శూన్య *Not_వాడిన,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం argc,              /* Number of arguments */
  స్థిరరాశి అక్షరం **argv      /* Text of each argument */
){
  పూర్ణాంకం i;
  అక్షరం zBuf[100];
  బాహ్య పూర్ణాంకం sqlite3_Test_Make_సూచిక_Str(Tcl_Interp*, అక్షరం*, శూన్య*);
  ఒకవేళ( argc!=2 ){
    Tcl_Append_ఫలితం(interp, "wrong # args: should be \"", argv[0],
       " ID", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ఒకవేళ( i<0 ) తిరిగి TCL_ERROR;
  ఒకవేళ( !threadset[i].busy ){
    Tcl_Append_ఫలితం(interp, "no such thread", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  sqlite3_Test_Make_సూచిక_Str(interp, zBuf, threadset[i].pStmt);
  threadset[i].pStmt = 0;
  Tcl_Append_ఫలితం(interp, zBuf, (అక్షరం*)0);
  తిరిగి TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
పూర్ణాంకం Sqlitetest4_Init(Tcl_Interp *interp){
  స్థిర నిర్మాణం {
     అక్షరం *zName;
     Tcl_CmdProc *xProc;
  } aCmd[] = {
     { "thread_create",     (Tcl_CmdProc*)tcl_thread_create     },
     { "thread_wait",       (Tcl_CmdProc*)tcl_thread_wait       },
     { "thread_halt",       (Tcl_CmdProc*)tcl_thread_halt       },
     { "thread_argc",       (Tcl_CmdProc*)tcl_thread_argc       },
     { "thread_argv",       (Tcl_CmdProc*)tcl_thread_argv       },
     { "thread_colname",    (Tcl_CmdProc*)tcl_thread_colname    },
     { "thread_result",     (Tcl_CmdProc*)tcl_thread_ఫలితం     },
     { "thread_error",      (Tcl_CmdProc*)tcl_thread_error      },
     { "thread_compile",    (Tcl_CmdProc*)tcl_thread_compile    },
     { "thread_step",       (Tcl_CmdProc*)tcl_thread_అడుగు       },
     { "thread_finalize",   (Tcl_CmdProc*)tcl_thread_finalize   },
     { "thread_swap",       (Tcl_CmdProc*)tcl_thread_మార్పిడి       },
     { "thread_db_get",     (Tcl_CmdProc*)tcl_thread_db_get     },
     { "thread_db_put",     (Tcl_CmdProc*)tcl_thread_db_put     },
     { "thread_stmt_get",   (Tcl_CmdProc*)tcl_thread_stmt_get   },
  };
  పూర్ణాంకం i;

  కొరకు(i=0; i<పరిమాణం(aCmd)/పరిమాణం(aCmd[0]); i++){
    Tcl_CreateCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }
  తిరిగి TCL_OK;
}
#లేకపోతే
పూర్ణాంకం Sqlitetest4_Init(Tcl_Interp *interp){ తిరిగి TCL_OK; }
#అయితే_ముగింపు /* SQLITE_OS_UNIX */
