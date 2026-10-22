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
#ଅନ୍ତର୍ଭାଵ "sqliteInt.h"
#ଅନ୍ତର୍ଭାଵ "tclsqlite.h"
#ଯଦି SQLITE_OS_UNIX && SQLITE_THREADSAFE
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <pthread.h>
#ଅନ୍ତର୍ଭାଵ <sched.h>
#ଅନ୍ତର୍ଭାଵ <ctype.h>

ବାହ୍ୟ ଅଚଳ ଅକ୍ଷର *sqlite3ErrName(ପୂର୍ଣ୍ଣସଂଖ୍ୟା);

/*
** Each thread is controlled by an instance of the following
** structure.
*/
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା Thread Thread;
ସଂରଚନା Thread {
  /* The first group of fields are writable by the leader and read-only
  ** to the thread. */
  ଅକ୍ଷର *zFilename;       /* Name of database file */
  ଶୂନ୍ୟ (*xOp)(Thread*);  /* next operation to do */
  ଅକ୍ଷର *zArg;            /* argument usable by xOp */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା opnum;             /* Operation number */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା busy;              /* True if this thread is in use */

  /* The next group of fields are writable by the thread but read-only to the
  ** leader. */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା completed;        /* Number of operations completed */
  sqlite3 *db;           /* Open database */
  sqlite3_stmt *pStmt;     /* Pending operation */
  ଅକ୍ଷର *zErr;           /* operation error */
  ଅକ୍ଷର *zStaticErr;     /* Static error message */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;               /* operation return code */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc;             /* number of columns in result */
  ଅଚଳ ଅକ୍ଷର *argv[100];    /* result columns */
  ଅଚଳ ଅକ୍ଷର *colv[100];    /* result column names */
};

/*
** There can be as many as 26 threads running at once.  Each is named
** by a capital letter: A, B, C, ..., Y, Z.
*/
#define N_THREAD 26
ସ୍ଥିର Thread threadset[N_THREAD];

ସ୍ଥିର ଶୂନ୍ୟ test_barrier(){
  sqlite3_mutex *pMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_APP1);
  sqlite3_mutex_enter(pMutex);
  sqlite3_mutex_leave(pMutex);
}

/*
** The main loop for a thread.  Threads use busy waiting. 
*/
ସ୍ଥିର ଶୂନ୍ୟ *test_thread_main(ଶୂନ୍ୟ *pArg){
  Thread *p = (Thread*)pArg;
  ଯଦି( p->db ){
    sqlite3_close(p->db);
  }
  sqlite3_open(p->zFilename, &p->db);
  ଯଦି( SQLITE_OK!=sqlite3_errcode(p->db) ){
    p->zErr = strdup(sqlite3_errmsg(p->db));
    sqlite3_close(p->db);
    p->db = 0;
  }
  p->pStmt = 0;
  test_barrier();
  p->completed = 1;
  ଯେତେବେଳେ( p->opnum<=p->completed ) sched_yield();
  test_barrier();
  ଯେତେବେଳେ( p->xOp ){
    ଯଦି( p->zErr && p->zErr!=p->zStaticErr ){
      sqlite3_free(p->zErr);
      p->zErr = 0;
    }
    (*p->xOp)(p);
    test_barrier();
    p->completed++;
    ଯେତେବେଳେ( p->opnum<=p->completed ) sched_yield();
    test_barrier();
  }
  ଯଦି( p->pStmt ){
    sqlite3_finalize(p->pStmt);
    p->pStmt = 0;
  }
  ଯଦି( p->db ){
    sqlite3_close(p->db);
    p->db = 0;
  }
  ଯଦି( p->zErr && p->zErr!=p->zStaticErr ){
    sqlite3_free(p->zErr);
    p->zErr = 0;
  }
  test_barrier();
  p->completed++;
#ifndef SQLITE_OMIT_DEPRECATED
  sqlite3_thread_cleanup();
#endif
  ଫେରନ୍ତୁ 0;
}

/*
** Get a thread ID which is an upper case letter.  Return the index.
** If the argument is not a valid thread ID put an error message in
** the interpreter and return -1.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା parse_thread_id(Tcl_Interp *interp, ଅଚଳ ଅକ୍ଷର *zArg){
  ଯଦି( zArg==0 || zArg[0]==0 || zArg[1]!=0 || !isupper((unsigned ଅକ୍ଷର)zArg[0]) ){
    Tcl_AppendResult(interp, "thread ID must be an upper case letter", ଖାଲି);
    ଫେରନ୍ତୁ -1;
  }
  ଫେରନ୍ତୁ zArg[0] - 'A';
}

/*
** Usage:    thread_create NAME  FILENAME
**
** NAME should be an upper case letter.  Start the thread running with
** an open connection to the given database.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI tcl_thread_create(
  ଶୂନ୍ୟ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,              /* Number of arguments */
  ଅଚଳ ଅକ୍ଷର **argv      /* Text of each argument */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  pthread_t x;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

  ଯଦି( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID FILENAME", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ଯଦି( i<0 ) ଫେରନ୍ତୁ TCL_ERROR;
  ଯଦି( threadset[i].busy ){
    Tcl_AppendResult(interp, "thread ", argv[1], " is already running", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  threadset[i].busy = 1;
  sqlite3_free(threadset[i].zFilename);
  threadset[i].zFilename = sqlite3_mprintf("%s", argv[2]);
  threadset[i].opnum = 1;
  threadset[i].completed = 0;
  rc = pthread_create(&x, 0, test_thread_main, &threadset[i]);
  ଯଦି( rc ){
    Tcl_AppendResult(interp, "failed to create the thread", ଖାଲି);
    sqlite3_free(threadset[i].zFilename);
    threadset[i].busy = 0;
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  pthread_detach(x);
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** Wait for a thread to reach its idle state.
*/
ସ୍ଥିର ଶୂନ୍ୟ test_thread_wait(Thread *p){
  test_barrier();
  ଯେତେବେଳେ( p->opnum>p->completed ) sched_yield();
  test_barrier();
}

/*
** Usage:  thread_wait ID
**
** Wait on thread ID to reach its idle state.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI tcl_thread_wait(
  ଶୂନ୍ୟ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,              /* Number of arguments */
  ଅଚଳ ଅକ୍ଷର **argv      /* Text of each argument */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

  ଯଦି( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ଯଦି( i<0 ) ଫେରନ୍ତୁ TCL_ERROR;
  ଯଦି( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** Stop a thread.
*/
ସ୍ଥିର ଶୂନ୍ୟ test_stop_thread(Thread *p){
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI tcl_thread_halt(
  ଶୂନ୍ୟ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,              /* Number of arguments */
  ଅଚଳ ଅକ୍ଷର **argv      /* Text of each argument */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

  ଯଦି( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( argv[1][0]=='*' && argv[1][1]==0 ){
    ପାଇଁ(i=0; i<N_THREAD; i++){
      ଯଦି( threadset[i].busy ) test_stop_thread(&threadset[i]);
    }
  }ଅନ୍ୟଥା{
    i = parse_thread_id(interp, argv[1]);
    ଯଦି( i<0 ) ଫେରନ୍ତୁ TCL_ERROR;
    ଯଦି( !threadset[i].busy ){
      Tcl_AppendResult(interp, "no such thread", ଖାଲି);
      ଫେରନ୍ତୁ TCL_ERROR;
    }
    test_stop_thread(&threadset[i]);
  }
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** Usage: thread_argc  ID
**
** Wait on the most recent thread_step to complete, then return the
** number of columns in the result set.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI tcl_thread_argc(
  ଶୂନ୍ୟ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,              /* Number of arguments */
  ଅଚଳ ଅକ୍ଷର **argv      /* Text of each argument */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ଅକ୍ଷର zBuf[100];

  ଯଦି( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ଯଦି( i<0 ) ଫେରନ୍ତୁ TCL_ERROR;
  ଯଦି( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  sqlite3_snprintf(ଆକାର(zBuf), zBuf, "%d", threadset[i].argc);
  Tcl_AppendResult(interp, zBuf, ଖାଲି);
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** Usage: thread_argv  ID   N
**
** Wait on the most recent thread_step to complete, then return the
** value of the N-th columns in the result set.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI tcl_thread_argv(
  ଶୂନ୍ୟ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,              /* Number of arguments */
  ଅଚଳ ଅକ୍ଷର **argv      /* Text of each argument */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n;

  ଯଦି( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID N", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ଯଦି( i<0 ) ଫେରନ୍ତୁ TCL_ERROR;
  ଯଦି( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( Tcl_GetInt(interp, argv[2], &n) ) ଫେରନ୍ତୁ TCL_ERROR;
  test_thread_wait(&threadset[i]);
  ଯଦି( n<0 || n>=threadset[i].argc ){
    Tcl_AppendResult(interp, "column number out of range", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  Tcl_AppendResult(interp, threadset[i].argv[n], ଖାଲି);
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** Usage: thread_colname  ID   N
**
** Wait on the most recent thread_step to complete, then return the
** name of the N-th columns in the result set.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI tcl_thread_colname(
  ଶୂନ୍ୟ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,              /* Number of arguments */
  ଅଚଳ ଅକ୍ଷର **argv      /* Text of each argument */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n;

  ଯଦି( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID N", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ଯଦି( i<0 ) ଫେରନ୍ତୁ TCL_ERROR;
  ଯଦି( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( Tcl_GetInt(interp, argv[2], &n) ) ଫେରନ୍ତୁ TCL_ERROR;
  test_thread_wait(&threadset[i]);
  ଯଦି( n<0 || n>=threadset[i].argc ){
    Tcl_AppendResult(interp, "column number out of range", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  Tcl_AppendResult(interp, threadset[i].colv[n], ଖାଲି);
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** Usage: thread_result  ID
**
** Wait on the most recent operation to complete, then return the
** result code from that operation.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI tcl_thread_result(
  ଶୂନ୍ୟ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,              /* Number of arguments */
  ଅଚଳ ଅକ୍ଷର **argv      /* Text of each argument */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ଅଚଳ ଅକ୍ଷର *zName;

  ଯଦି( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ଯଦି( i<0 ) ଫେରନ୍ତୁ TCL_ERROR;
  ଯଦି( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  zName = sqlite3ErrName(threadset[i].rc);
  Tcl_AppendResult(interp, zName, ଖାଲି);
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** Usage: thread_error  ID
**
** Wait on the most recent operation to complete, then return the
** error string.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI tcl_thread_error(
  ଶୂନ୍ୟ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,              /* Number of arguments */
  ଅଚଳ ଅକ୍ଷର **argv      /* Text of each argument */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

  ଯଦି( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ଯଦି( i<0 ) ଫେରନ୍ତୁ TCL_ERROR;
  ଯଦି( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  Tcl_AppendResult(interp, threadset[i].zErr, ଖାଲି);
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** This procedure runs in the thread to compile an SQL statement.
*/
ସ୍ଥିର ଶୂନ୍ୟ do_compile(Thread *p){
  ଯଦି( p->db==0 ){
    p->zErr = p->zStaticErr = "no database is open";
    p->rc = SQLITE_ERROR;
    ଫେରନ୍ତୁ;
  }
  ଯଦି( p->pStmt ){
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI tcl_thread_compile(
  ଶୂନ୍ୟ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,              /* Number of arguments */
  ଅଚଳ ଅକ୍ଷର **argv      /* Text of each argument */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ଯଦି( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID SQL", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ଯଦି( i<0 ) ଫେରନ୍ତୁ TCL_ERROR;
  ଯଦି( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  threadset[i].xOp = do_compile;
  sqlite3_free(threadset[i].zArg);
  threadset[i].zArg = sqlite3_mprintf("%s", argv[2]);
  test_barrier();
  threadset[i].opnum++;
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** This procedure runs in the thread to step the virtual machine.
*/
ସ୍ଥିର ଶୂନ୍ୟ do_step(Thread *p){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ଯଦି( p->pStmt==0 ){
    p->zErr = p->zStaticErr = "no virtual machine available";
    p->rc = SQLITE_ERROR;
    ଫେରନ୍ତୁ;
  }
  p->rc = sqlite3_step(p->pStmt);
  ଯଦି( p->rc==SQLITE_ROW ){
    p->argc = sqlite3_column_ଗଣନା(p->pStmt);
    ପାଇଁ(i=0; i<sqlite3_data_ଗଣନା(p->pStmt); i++){
      p->argv[i] = (ଅକ୍ଷର*)sqlite3_column_text(p->pStmt, i);
    }
    ପାଇଁ(i=0; i<p->argc; i++){
      p->colv[i] = sqlite3_column_name(p->pStmt, i);
    }
  }
}

/*
** Usage: thread_step ID
**
** Advance the virtual machine by one step
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI tcl_thread_step(
  ଶୂନ୍ୟ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,              /* Number of arguments */
  ଅଚଳ ଅକ୍ଷର **argv      /* Text of each argument */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ଯଦି( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " IDL", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ଯଦି( i<0 ) ଫେରନ୍ତୁ TCL_ERROR;
  ଯଦି( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  threadset[i].xOp = do_step;
  test_barrier();
  threadset[i].opnum++;
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** This procedure runs in the thread to finalize a virtual machine.
*/
ସ୍ଥିର ଶୂନ୍ୟ do_finalize(Thread *p){
  ଯଦି( p->pStmt==0 ){
    p->zErr = p->zStaticErr = "no virtual machine available";
    p->rc = SQLITE_ERROR;
    ଫେରନ୍ତୁ;
  }
  p->rc = sqlite3_finalize(p->pStmt);
  p->pStmt = 0;
}

/*
** Usage: thread_finalize ID
**
** Finalize the virtual machine.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI tcl_thread_finalize(
  ଶୂନ୍ୟ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,              /* Number of arguments */
  ଅଚଳ ଅକ୍ଷର **argv      /* Text of each argument */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ଯଦି( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " IDL", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ଯଦି( i<0 ) ଫେରନ୍ତୁ TCL_ERROR;
  ଯଦି( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  threadset[i].xOp = do_finalize;
  sqlite3_free(threadset[i].zArg);
  threadset[i].zArg = 0;
  test_barrier();
  threadset[i].opnum++;
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** Usage: thread_swap ID ID
**
** Interchange the sqlite* pointer between two threads.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI tcl_thread_swap(
  ଶୂନ୍ୟ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,              /* Number of arguments */
  ଅଚଳ ଅକ୍ଷର **argv      /* Text of each argument */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, j;
  sqlite3 *temp;
  ଯଦି( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID1 ID2", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ଯଦି( i<0 ) ଫେରନ୍ତୁ TCL_ERROR;
  ଯଦି( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  j = parse_thread_id(interp, argv[2]);
  ଯଦି( j<0 ) ଫେରନ୍ତୁ TCL_ERROR;
  ଯଦି( !threadset[j].busy ){
    Tcl_AppendResult(interp, "no such thread", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  test_thread_wait(&threadset[j]);
  temp = threadset[i].db;
  threadset[i].db = threadset[j].db;
  threadset[j].db = temp;
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** Usage: thread_db_get ID
**
** Return the database connection pointer for the given thread.  Then
** remove the pointer from the thread itself.  Afterwards, the thread
** can be stopped and the connection can be used by the main thread.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI tcl_thread_db_get(
  ଶୂନ୍ୟ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,              /* Number of arguments */
  ଅଚଳ ଅକ୍ଷର **argv      /* Text of each argument */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ଅକ୍ଷର zBuf[100];
  ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3TestMakePointerStr(Tcl_Interp*, ଅକ୍ଷର*, ଶୂନ୍ୟ*);
  ଯଦି( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ଯଦି( i<0 ) ଫେରନ୍ତୁ TCL_ERROR;
  ଯଦି( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  sqlite3TestMakePointerStr(interp, zBuf, threadset[i].db);
  threadset[i].db = 0;
  Tcl_AppendResult(interp, zBuf, (ଅକ୍ଷର*)0);
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** Usage: thread_db_put ID DB
**
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI tcl_thread_db_put(
  ଶୂନ୍ୟ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,              /* Number of arguments */
  ଅଚଳ ଅକ୍ଷର **argv      /* Text of each argument */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3TestMakePointerStr(Tcl_Interp*, ଅକ୍ଷର*, ଶୂନ୍ୟ*);
  ବାହ୍ୟ ଶୂନ୍ୟ *sqlite3TestTextToPtr(ଅଚଳ ଅକ୍ଷର *);
  ଯଦି( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID DB", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ଯଦି( i<0 ) ଫେରନ୍ତୁ TCL_ERROR;
  ଯଦି( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  ପ୍ରତିଜ୍ଞା( !threadset[i].db );
  threadset[i].db = (sqlite3*)sqlite3TestTextToPtr(argv[2]);
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** Usage: thread_stmt_get ID
**
** Return the database stmt pointer for the given thread.  Then
** remove the pointer from the thread itself. 
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI tcl_thread_stmt_get(
  ଶୂନ୍ୟ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,              /* Number of arguments */
  ଅଚଳ ଅକ୍ଷର **argv      /* Text of each argument */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ଅକ୍ଷର zBuf[100];
  ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3TestMakePointerStr(Tcl_Interp*, ଅକ୍ଷର*, ଶୂନ୍ୟ*);
  ଯଦି( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  ଯଦି( i<0 ) ଫେରନ୍ତୁ TCL_ERROR;
  ଯଦି( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  sqlite3TestMakePointerStr(interp, zBuf, threadset[i].pStmt);
  threadset[i].pStmt = 0;
  Tcl_AppendResult(interp, zBuf, (ଅକ୍ଷର*)0);
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା Sqlitetest4_Init(Tcl_Interp *interp){
  ସ୍ଥିର ସଂରଚନା {
     ଅକ୍ଷର *zName;
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
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

  ପାଇଁ(i=0; i<ଆକାର(aCmd)/ଆକାର(aCmd[0]); i++){
    Tcl_CreateCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }
  ଫେରନ୍ତୁ TCL_OK;
}
#ଅନ୍ୟଥା
ପୂର୍ଣ୍ଣସଂଖ୍ୟା Sqlitetest4_Init(Tcl_Interp *interp){ ଫେରନ୍ତୁ TCL_OK; }
#endif /* SQLITE_OS_UNIX */
