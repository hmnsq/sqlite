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
#അന്തര്ഭാവ "sqliteInt.h"
#അന്തര്ഭാവ "tclsqlite.h"
#എങ്കിൽ SQLITE_OS_UNIX && SQLITE_THREADSAFE
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <pthread.h>
#അന്തര്ഭാവ <sched.h>
#അന്തര്ഭാവ <ctype.h>

ബാഹ്യ സ്ഥിരം അക്ഷരം *sqlite3ErrName(പൂർണ്ണസംഖ്യ);

/*
** Each thread is controlled by an instance of the following
** structure.
*/
തരനിർവചനം ഘടന Thread Thread;
ഘടന Thread {
  /* The first group of fields are writable by the leader and read-only
  ** to the thread. */
  അക്ഷരം *zFilename;       /* Name of database file */
  ശൂന്യം (*xOp)(Thread*);  /* next operation to do */
  അക്ഷരം *zArg;            /* argument usable by xOp */
  പൂർണ്ണസംഖ്യ opnum;             /* Operation number */
  പൂർണ്ണസംഖ്യ busy;              /* True if this thread is in use */

  /* The next group of fields are writable by the thread but read-only to the
  ** leader. */
  പൂർണ്ണസംഖ്യ completed;        /* Number of operations completed */
  sqlite3 *db;           /* Open database */
  sqlite3_stmt *pStmt;     /* Pending operation */
  അക്ഷരം *zErr;           /* operation error */
  അക്ഷരം *zStaticErr;     /* Static error message */
  പൂർണ്ണസംഖ്യ rc;               /* operation return code */
  പൂർണ്ണസംഖ്യ argc;             /* number of columns in result */
  സ്ഥിരം അക്ഷരം *argv[100];    /* result columns */
  സ്ഥിരം അക്ഷരം *colv[100];    /* result column names */
};

/*
** There can be as many as 26 threads running at once.  Each is named
** by a capital letter: A, B, C, ..., Y, Z.
*/
#define N_THREAD 26
സ്ഥിതം Thread threadset[N_THREAD];

സ്ഥിതം ശൂന്യം test_barrier(){
  sqlite3_mutex *pMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_APP1);
  sqlite3_mutex_enter(pMutex);
  sqlite3_mutex_leave(pMutex);
}

/*
** The main loop for a thread.  Threads use busy waiting. 
*/
സ്ഥിതം ശൂന്യം *test_thread_main(ശൂന്യം *pArg){
  Thread *p = (Thread*)pArg;
  എങ്കിൽ( p->db ){
    sqlite3_close(p->db);
  }
  sqlite3_open(p->zFilename, &p->db);
  എങ്കിൽ( SQLITE_OK!=sqlite3_errcode(p->db) ){
    p->zErr = strdup(sqlite3_errmsg(p->db));
    sqlite3_close(p->db);
    p->db = 0;
  }
  p->pStmt = 0;
  test_barrier();
  p->completed = 1;
  ആകുമ്പോൾ( p->opnum<=p->completed ) sched_yield();
  test_barrier();
  ആകുമ്പോൾ( p->xOp ){
    എങ്കിൽ( p->zErr && p->zErr!=p->zStaticErr ){
      sqlite3_free(p->zErr);
      p->zErr = 0;
    }
    (*p->xOp)(p);
    test_barrier();
    p->completed++;
    ആകുമ്പോൾ( p->opnum<=p->completed ) sched_yield();
    test_barrier();
  }
  എങ്കിൽ( p->pStmt ){
    sqlite3_finalize(p->pStmt);
    p->pStmt = 0;
  }
  എങ്കിൽ( p->db ){
    sqlite3_close(p->db);
    p->db = 0;
  }
  എങ്കിൽ( p->zErr && p->zErr!=p->zStaticErr ){
    sqlite3_free(p->zErr);
    p->zErr = 0;
  }
  test_barrier();
  p->completed++;
#ifndef SQLITE_OMIT_DEPRECATED
  sqlite3_thread_cleanup();
#endif
  തിരികെ 0;
}

/*
** Get a thread ID which is an upper case letter.  Return the index.
** If the argument is not a valid thread ID put an error message in
** the interpreter and return -1.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ parse_thread_id(Tcl_Interp *interp, സ്ഥിരം അക്ഷരം *zArg){
  എങ്കിൽ( zArg==0 || zArg[0]==0 || zArg[1]!=0 || !isupper((unsigned അക്ഷരം)zArg[0]) ){
    Tcl_AppendResult(interp, "thread ID must be an upper case letter", ശൂന്യ);
    തിരികെ -1;
  }
  തിരികെ zArg[0] - 'A';
}

/*
** Usage:    thread_create NAME  FILENAME
**
** NAME should be an upper case letter.  Start the thread running with
** an open connection to the given database.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI tcl_thread_create(
  ശൂന്യം *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ argc,              /* Number of arguments */
  സ്ഥിരം അക്ഷരം **argv      /* Text of each argument */
){
  പൂർണ്ണസംഖ്യ i;
  pthread_t x;
  പൂർണ്ണസംഖ്യ rc;

  എങ്കിൽ( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID FILENAME", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  എങ്കിൽ( i<0 ) തിരികെ TCL_ERROR;
  എങ്കിൽ( threadset[i].busy ){
    Tcl_AppendResult(interp, "thread ", argv[1], " is already running", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  threadset[i].busy = 1;
  sqlite3_free(threadset[i].zFilename);
  threadset[i].zFilename = sqlite3_mprintf("%s", argv[2]);
  threadset[i].opnum = 1;
  threadset[i].completed = 0;
  rc = pthread_create(&x, 0, test_thread_main, &threadset[i]);
  എങ്കിൽ( rc ){
    Tcl_AppendResult(interp, "failed to create the thread", ശൂന്യ);
    sqlite3_free(threadset[i].zFilename);
    threadset[i].busy = 0;
    തിരികെ TCL_ERROR;
  }
  pthread_detach(x);
  തിരികെ TCL_OK;
}

/*
** Wait for a thread to reach its idle state.
*/
സ്ഥിതം ശൂന്യം test_thread_wait(Thread *p){
  test_barrier();
  ആകുമ്പോൾ( p->opnum>p->completed ) sched_yield();
  test_barrier();
}

/*
** Usage:  thread_wait ID
**
** Wait on thread ID to reach its idle state.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI tcl_thread_wait(
  ശൂന്യം *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ argc,              /* Number of arguments */
  സ്ഥിരം അക്ഷരം **argv      /* Text of each argument */
){
  പൂർണ്ണസംഖ്യ i;

  എങ്കിൽ( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  എങ്കിൽ( i<0 ) തിരികെ TCL_ERROR;
  എങ്കിൽ( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  തിരികെ TCL_OK;
}

/*
** Stop a thread.
*/
സ്ഥിതം ശൂന്യം test_stop_thread(Thread *p){
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
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI tcl_thread_halt(
  ശൂന്യം *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ argc,              /* Number of arguments */
  സ്ഥിരം അക്ഷരം **argv      /* Text of each argument */
){
  പൂർണ്ണസംഖ്യ i;

  എങ്കിൽ( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( argv[1][0]=='*' && argv[1][1]==0 ){
    വേണ്ടി(i=0; i<N_THREAD; i++){
      എങ്കിൽ( threadset[i].busy ) test_stop_thread(&threadset[i]);
    }
  }അല്ലെങ്കിൽ{
    i = parse_thread_id(interp, argv[1]);
    എങ്കിൽ( i<0 ) തിരികെ TCL_ERROR;
    എങ്കിൽ( !threadset[i].busy ){
      Tcl_AppendResult(interp, "no such thread", ശൂന്യ);
      തിരികെ TCL_ERROR;
    }
    test_stop_thread(&threadset[i]);
  }
  തിരികെ TCL_OK;
}

/*
** Usage: thread_argc  ID
**
** Wait on the most recent thread_step to complete, then return the
** number of columns in the result set.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI tcl_thread_argc(
  ശൂന്യം *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ argc,              /* Number of arguments */
  സ്ഥിരം അക്ഷരം **argv      /* Text of each argument */
){
  പൂർണ്ണസംഖ്യ i;
  അക്ഷരം zBuf[100];

  എങ്കിൽ( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  എങ്കിൽ( i<0 ) തിരികെ TCL_ERROR;
  എങ്കിൽ( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  sqlite3_snprintf(വലുപ്പം(zBuf), zBuf, "%d", threadset[i].argc);
  Tcl_AppendResult(interp, zBuf, ശൂന്യ);
  തിരികെ TCL_OK;
}

/*
** Usage: thread_argv  ID   N
**
** Wait on the most recent thread_step to complete, then return the
** value of the N-th columns in the result set.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI tcl_thread_argv(
  ശൂന്യം *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ argc,              /* Number of arguments */
  സ്ഥിരം അക്ഷരം **argv      /* Text of each argument */
){
  പൂർണ്ണസംഖ്യ i;
  പൂർണ്ണസംഖ്യ n;

  എങ്കിൽ( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID N", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  എങ്കിൽ( i<0 ) തിരികെ TCL_ERROR;
  എങ്കിൽ( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( Tcl_GetInt(interp, argv[2], &n) ) തിരികെ TCL_ERROR;
  test_thread_wait(&threadset[i]);
  എങ്കിൽ( n<0 || n>=threadset[i].argc ){
    Tcl_AppendResult(interp, "column number out of range", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  Tcl_AppendResult(interp, threadset[i].argv[n], ശൂന്യ);
  തിരികെ TCL_OK;
}

/*
** Usage: thread_colname  ID   N
**
** Wait on the most recent thread_step to complete, then return the
** name of the N-th columns in the result set.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI tcl_thread_colname(
  ശൂന്യം *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ argc,              /* Number of arguments */
  സ്ഥിരം അക്ഷരം **argv      /* Text of each argument */
){
  പൂർണ്ണസംഖ്യ i;
  പൂർണ്ണസംഖ്യ n;

  എങ്കിൽ( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID N", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  എങ്കിൽ( i<0 ) തിരികെ TCL_ERROR;
  എങ്കിൽ( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( Tcl_GetInt(interp, argv[2], &n) ) തിരികെ TCL_ERROR;
  test_thread_wait(&threadset[i]);
  എങ്കിൽ( n<0 || n>=threadset[i].argc ){
    Tcl_AppendResult(interp, "column number out of range", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  Tcl_AppendResult(interp, threadset[i].colv[n], ശൂന്യ);
  തിരികെ TCL_OK;
}

/*
** Usage: thread_result  ID
**
** Wait on the most recent operation to complete, then return the
** result code from that operation.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI tcl_thread_result(
  ശൂന്യം *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ argc,              /* Number of arguments */
  സ്ഥിരം അക്ഷരം **argv      /* Text of each argument */
){
  പൂർണ്ണസംഖ്യ i;
  സ്ഥിരം അക്ഷരം *zName;

  എങ്കിൽ( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  എങ്കിൽ( i<0 ) തിരികെ TCL_ERROR;
  എങ്കിൽ( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  zName = sqlite3ErrName(threadset[i].rc);
  Tcl_AppendResult(interp, zName, ശൂന്യ);
  തിരികെ TCL_OK;
}

/*
** Usage: thread_error  ID
**
** Wait on the most recent operation to complete, then return the
** error string.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI tcl_thread_error(
  ശൂന്യം *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ argc,              /* Number of arguments */
  സ്ഥിരം അക്ഷരം **argv      /* Text of each argument */
){
  പൂർണ്ണസംഖ്യ i;

  എങ്കിൽ( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  എങ്കിൽ( i<0 ) തിരികെ TCL_ERROR;
  എങ്കിൽ( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  Tcl_AppendResult(interp, threadset[i].zErr, ശൂന്യ);
  തിരികെ TCL_OK;
}

/*
** This procedure runs in the thread to compile an SQL statement.
*/
സ്ഥിതം ശൂന്യം do_compile(Thread *p){
  എങ്കിൽ( p->db==0 ){
    p->zErr = p->zStaticErr = "no database is open";
    p->rc = SQLITE_ERROR;
    തിരികെ;
  }
  എങ്കിൽ( p->pStmt ){
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
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI tcl_thread_compile(
  ശൂന്യം *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ argc,              /* Number of arguments */
  സ്ഥിരം അക്ഷരം **argv      /* Text of each argument */
){
  പൂർണ്ണസംഖ്യ i;
  എങ്കിൽ( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID SQL", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  എങ്കിൽ( i<0 ) തിരികെ TCL_ERROR;
  എങ്കിൽ( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  threadset[i].xOp = do_compile;
  sqlite3_free(threadset[i].zArg);
  threadset[i].zArg = sqlite3_mprintf("%s", argv[2]);
  test_barrier();
  threadset[i].opnum++;
  തിരികെ TCL_OK;
}

/*
** This procedure runs in the thread to step the virtual machine.
*/
സ്ഥിതം ശൂന്യം do_step(Thread *p){
  പൂർണ്ണസംഖ്യ i;
  എങ്കിൽ( p->pStmt==0 ){
    p->zErr = p->zStaticErr = "no virtual machine available";
    p->rc = SQLITE_ERROR;
    തിരികെ;
  }
  p->rc = sqlite3_step(p->pStmt);
  എങ്കിൽ( p->rc==SQLITE_ROW ){
    p->argc = sqlite3_column_എണ്ണം(p->pStmt);
    വേണ്ടി(i=0; i<sqlite3_data_എണ്ണം(p->pStmt); i++){
      p->argv[i] = (അക്ഷരം*)sqlite3_column_text(p->pStmt, i);
    }
    വേണ്ടി(i=0; i<p->argc; i++){
      p->colv[i] = sqlite3_column_name(p->pStmt, i);
    }
  }
}

/*
** Usage: thread_step ID
**
** Advance the virtual machine by one step
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI tcl_thread_step(
  ശൂന്യം *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ argc,              /* Number of arguments */
  സ്ഥിരം അക്ഷരം **argv      /* Text of each argument */
){
  പൂർണ്ണസംഖ്യ i;
  എങ്കിൽ( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " IDL", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  എങ്കിൽ( i<0 ) തിരികെ TCL_ERROR;
  എങ്കിൽ( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  threadset[i].xOp = do_step;
  test_barrier();
  threadset[i].opnum++;
  തിരികെ TCL_OK;
}

/*
** This procedure runs in the thread to finalize a virtual machine.
*/
സ്ഥിതം ശൂന്യം do_finalize(Thread *p){
  എങ്കിൽ( p->pStmt==0 ){
    p->zErr = p->zStaticErr = "no virtual machine available";
    p->rc = SQLITE_ERROR;
    തിരികെ;
  }
  p->rc = sqlite3_finalize(p->pStmt);
  p->pStmt = 0;
}

/*
** Usage: thread_finalize ID
**
** Finalize the virtual machine.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI tcl_thread_finalize(
  ശൂന്യം *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ argc,              /* Number of arguments */
  സ്ഥിരം അക്ഷരം **argv      /* Text of each argument */
){
  പൂർണ്ണസംഖ്യ i;
  എങ്കിൽ( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " IDL", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  എങ്കിൽ( i<0 ) തിരികെ TCL_ERROR;
  എങ്കിൽ( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  threadset[i].xOp = do_finalize;
  sqlite3_free(threadset[i].zArg);
  threadset[i].zArg = 0;
  test_barrier();
  threadset[i].opnum++;
  തിരികെ TCL_OK;
}

/*
** Usage: thread_swap ID ID
**
** Interchange the sqlite* pointer between two threads.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI tcl_thread_swap(
  ശൂന്യം *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ argc,              /* Number of arguments */
  സ്ഥിരം അക്ഷരം **argv      /* Text of each argument */
){
  പൂർണ്ണസംഖ്യ i, j;
  sqlite3 *temp;
  എങ്കിൽ( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID1 ID2", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  എങ്കിൽ( i<0 ) തിരികെ TCL_ERROR;
  എങ്കിൽ( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  j = parse_thread_id(interp, argv[2]);
  എങ്കിൽ( j<0 ) തിരികെ TCL_ERROR;
  എങ്കിൽ( !threadset[j].busy ){
    Tcl_AppendResult(interp, "no such thread", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  test_thread_wait(&threadset[j]);
  temp = threadset[i].db;
  threadset[i].db = threadset[j].db;
  threadset[j].db = temp;
  തിരികെ TCL_OK;
}

/*
** Usage: thread_db_get ID
**
** Return the database connection pointer for the given thread.  Then
** remove the pointer from the thread itself.  Afterwards, the thread
** can be stopped and the connection can be used by the main thread.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI tcl_thread_db_get(
  ശൂന്യം *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ argc,              /* Number of arguments */
  സ്ഥിരം അക്ഷരം **argv      /* Text of each argument */
){
  പൂർണ്ണസംഖ്യ i;
  അക്ഷരം zBuf[100];
  ബാഹ്യ പൂർണ്ണസംഖ്യ sqlite3TestMakePointerStr(Tcl_Interp*, അക്ഷരം*, ശൂന്യം*);
  എങ്കിൽ( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  എങ്കിൽ( i<0 ) തിരികെ TCL_ERROR;
  എങ്കിൽ( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  sqlite3TestMakePointerStr(interp, zBuf, threadset[i].db);
  threadset[i].db = 0;
  Tcl_AppendResult(interp, zBuf, (അക്ഷരം*)0);
  തിരികെ TCL_OK;
}

/*
** Usage: thread_db_put ID DB
**
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI tcl_thread_db_put(
  ശൂന്യം *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ argc,              /* Number of arguments */
  സ്ഥിരം അക്ഷരം **argv      /* Text of each argument */
){
  പൂർണ്ണസംഖ്യ i;
  ബാഹ്യ പൂർണ്ണസംഖ്യ sqlite3TestMakePointerStr(Tcl_Interp*, അക്ഷരം*, ശൂന്യം*);
  ബാഹ്യ ശൂന്യം *sqlite3TestTextToPtr(സ്ഥിരം അക്ഷരം *);
  എങ്കിൽ( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID DB", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  എങ്കിൽ( i<0 ) തിരികെ TCL_ERROR;
  എങ്കിൽ( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  പ്രതിജ്ഞാ( !threadset[i].db );
  threadset[i].db = (sqlite3*)sqlite3TestTextToPtr(argv[2]);
  തിരികെ TCL_OK;
}

/*
** Usage: thread_stmt_get ID
**
** Return the database stmt pointer for the given thread.  Then
** remove the pointer from the thread itself. 
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI tcl_thread_stmt_get(
  ശൂന്യം *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ argc,              /* Number of arguments */
  സ്ഥിരം അക്ഷരം **argv      /* Text of each argument */
){
  പൂർണ്ണസംഖ്യ i;
  അക്ഷരം zBuf[100];
  ബാഹ്യ പൂർണ്ണസംഖ്യ sqlite3TestMakePointerStr(Tcl_Interp*, അക്ഷരം*, ശൂന്യം*);
  എങ്കിൽ( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  എങ്കിൽ( i<0 ) തിരികെ TCL_ERROR;
  എങ്കിൽ( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  sqlite3TestMakePointerStr(interp, zBuf, threadset[i].pStmt);
  threadset[i].pStmt = 0;
  Tcl_AppendResult(interp, zBuf, (അക്ഷരം*)0);
  തിരികെ TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
പൂർണ്ണസംഖ്യ Sqlitetest4_Init(Tcl_Interp *interp){
  സ്ഥിതം ഘടന {
     അക്ഷരം *zName;
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
  പൂർണ്ണസംഖ്യ i;

  വേണ്ടി(i=0; i<വലുപ്പം(aCmd)/വലുപ്പം(aCmd[0]); i++){
    Tcl_CreateCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }
  തിരികെ TCL_OK;
}
#അല്ലെങ്കിൽ
പൂർണ്ണസംഖ്യ Sqlitetest4_Init(Tcl_Interp *interp){ തിരികെ TCL_OK; }
#endif /* SQLITE_OS_UNIX */
