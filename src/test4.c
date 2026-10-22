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
#ضمن "sqliteInt.h"
#ضمن "tclsqlite.h"
#إذا SQLITE_OS_UNIX && SQLITE_THREADSAFE
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <pthread.h>
#ضمن <sched.h>
#ضمن <ctype.h>

خارجي ثابت حرف *sqlite3ErrName(صحيح);

/*
** Each thread is controlled by an instance of the following
** structure.
*/
تعريفنوع بنية Thread Thread;
بنية Thread {
  /* The first group of fields are writable by the leader and read-only
  ** to the thread. */
  حرف *zFilename;       /* Name of database file */
  فراغ (*xOp)(Thread*);  /* next operation to do */
  حرف *zArg;            /* argument usable by xOp */
  صحيح opnum;             /* Operation number */
  صحيح busy;              /* True if this thread is in use */

  /* The next group of fields are writable by the thread but read-only to the
  ** leader. */
  صحيح completed;        /* Number of operations completed */
  sqlite3 *db;           /* Open database */
  sqlite3_stmt *pStmt;     /* Pending operation */
  حرف *zErr;           /* operation error */
  حرف *zStaticErr;     /* Static error message */
  صحيح rc;               /* operation return code */
  صحيح argc;             /* number of columns in result */
  ثابت حرف *argv[100];    /* result columns */
  ثابت حرف *colv[100];    /* result column names */
};

/*
** There can be as many as 26 threads running at once.  Each is named
** by a capital letter: A, B, C, ..., Y, Z.
*/
#define N_THREAD 26
ساكن Thread threadset[N_THREAD];

ساكن فراغ test_barrier(){
  sqlite3_mutex *pMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_APP1);
  sqlite3_mutex_enter(pMutex);
  sqlite3_mutex_leave(pMutex);
}

/*
** The main loop for a thread.  Threads use busy waiting. 
*/
ساكن فراغ *test_thread_main(فراغ *pArg){
  Thread *p = (Thread*)pArg;
  إذا( p->db ){
    sqlite3_close(p->db);
  }
  sqlite3_open(p->zFilename, &p->db);
  إذا( SQLITE_OK!=sqlite3_errcode(p->db) ){
    p->zErr = strdup(sqlite3_errmsg(p->db));
    sqlite3_close(p->db);
    p->db = 0;
  }
  p->pStmt = 0;
  test_barrier();
  p->completed = 1;
  بينما( p->opnum<=p->completed ) sched_yield();
  test_barrier();
  بينما( p->xOp ){
    إذا( p->zErr && p->zErr!=p->zStaticErr ){
      sqlite3_free(p->zErr);
      p->zErr = 0;
    }
    (*p->xOp)(p);
    test_barrier();
    p->completed++;
    بينما( p->opnum<=p->completed ) sched_yield();
    test_barrier();
  }
  إذا( p->pStmt ){
    sqlite3_finalize(p->pStmt);
    p->pStmt = 0;
  }
  إذا( p->db ){
    sqlite3_close(p->db);
    p->db = 0;
  }
  إذا( p->zErr && p->zErr!=p->zStaticErr ){
    sqlite3_free(p->zErr);
    p->zErr = 0;
  }
  test_barrier();
  p->completed++;
#ifndef SQLITE_OMIT_DEPRECATED
  sqlite3_thread_cleanup();
#endif
  إرجاع 0;
}

/*
** Get a thread ID which is an upper case letter.  Return the index.
** If the argument is not a valid thread ID put an error message in
** the interpreter and return -1.
*/
ساكن صحيح parse_thread_id(Tcl_Interp *interp, ثابت حرف *zArg){
  إذا( zArg==0 || zArg[0]==0 || zArg[1]!=0 || !isupper((unsigned حرف)zArg[0]) ){
    Tcl_AppendResult(interp, "thread ID must be an upper case letter", عدم);
    إرجاع -1;
  }
  إرجاع zArg[0] - 'A';
}

/*
** Usage:    thread_create NAME  FILENAME
**
** NAME should be an upper case letter.  Start the thread running with
** an open connection to the given database.
*/
ساكن صحيح SQLITE_TCLAPI tcl_thread_create(
  فراغ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  صحيح i;
  pthread_t x;
  صحيح rc;

  إذا( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID FILENAME", عدم);
    إرجاع TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  إذا( i<0 ) إرجاع TCL_ERROR;
  إذا( threadset[i].busy ){
    Tcl_AppendResult(interp, "thread ", argv[1], " is already running", عدم);
    إرجاع TCL_ERROR;
  }
  threadset[i].busy = 1;
  sqlite3_free(threadset[i].zFilename);
  threadset[i].zFilename = sqlite3_mprintf("%s", argv[2]);
  threadset[i].opnum = 1;
  threadset[i].completed = 0;
  rc = pthread_create(&x, 0, test_thread_main, &threadset[i]);
  إذا( rc ){
    Tcl_AppendResult(interp, "failed to create the thread", عدم);
    sqlite3_free(threadset[i].zFilename);
    threadset[i].busy = 0;
    إرجاع TCL_ERROR;
  }
  pthread_detach(x);
  إرجاع TCL_OK;
}

/*
** Wait for a thread to reach its idle state.
*/
ساكن فراغ test_thread_wait(Thread *p){
  test_barrier();
  بينما( p->opnum>p->completed ) sched_yield();
  test_barrier();
}

/*
** Usage:  thread_wait ID
**
** Wait on thread ID to reach its idle state.
*/
ساكن صحيح SQLITE_TCLAPI tcl_thread_wait(
  فراغ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  صحيح i;

  إذا( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID", عدم);
    إرجاع TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  إذا( i<0 ) إرجاع TCL_ERROR;
  إذا( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", عدم);
    إرجاع TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  إرجاع TCL_OK;
}

/*
** Stop a thread.
*/
ساكن فراغ test_stop_thread(Thread *p){
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
ساكن صحيح SQLITE_TCLAPI tcl_thread_halt(
  فراغ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  صحيح i;

  إذا( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID", عدم);
    إرجاع TCL_ERROR;
  }
  إذا( argv[1][0]=='*' && argv[1][1]==0 ){
    لكل(i=0; i<N_THREAD; i++){
      إذا( threadset[i].busy ) test_stop_thread(&threadset[i]);
    }
  }وإلا{
    i = parse_thread_id(interp, argv[1]);
    إذا( i<0 ) إرجاع TCL_ERROR;
    إذا( !threadset[i].busy ){
      Tcl_AppendResult(interp, "no such thread", عدم);
      إرجاع TCL_ERROR;
    }
    test_stop_thread(&threadset[i]);
  }
  إرجاع TCL_OK;
}

/*
** Usage: thread_argc  ID
**
** Wait on the most recent thread_step to complete, then return the
** number of columns in the result set.
*/
ساكن صحيح SQLITE_TCLAPI tcl_thread_argc(
  فراغ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  صحيح i;
  حرف zBuf[100];

  إذا( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID", عدم);
    إرجاع TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  إذا( i<0 ) إرجاع TCL_ERROR;
  إذا( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", عدم);
    إرجاع TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  sqlite3_snprintf(حجم(zBuf), zBuf, "%d", threadset[i].argc);
  Tcl_AppendResult(interp, zBuf, عدم);
  إرجاع TCL_OK;
}

/*
** Usage: thread_argv  ID   N
**
** Wait on the most recent thread_step to complete, then return the
** value of the N-th columns in the result set.
*/
ساكن صحيح SQLITE_TCLAPI tcl_thread_argv(
  فراغ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  صحيح i;
  صحيح n;

  إذا( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID N", عدم);
    إرجاع TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  إذا( i<0 ) إرجاع TCL_ERROR;
  إذا( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", عدم);
    إرجاع TCL_ERROR;
  }
  إذا( Tcl_GetInt(interp, argv[2], &n) ) إرجاع TCL_ERROR;
  test_thread_wait(&threadset[i]);
  إذا( n<0 || n>=threadset[i].argc ){
    Tcl_AppendResult(interp, "column number out of range", عدم);
    إرجاع TCL_ERROR;
  }
  Tcl_AppendResult(interp, threadset[i].argv[n], عدم);
  إرجاع TCL_OK;
}

/*
** Usage: thread_colname  ID   N
**
** Wait on the most recent thread_step to complete, then return the
** name of the N-th columns in the result set.
*/
ساكن صحيح SQLITE_TCLAPI tcl_thread_colname(
  فراغ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  صحيح i;
  صحيح n;

  إذا( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID N", عدم);
    إرجاع TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  إذا( i<0 ) إرجاع TCL_ERROR;
  إذا( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", عدم);
    إرجاع TCL_ERROR;
  }
  إذا( Tcl_GetInt(interp, argv[2], &n) ) إرجاع TCL_ERROR;
  test_thread_wait(&threadset[i]);
  إذا( n<0 || n>=threadset[i].argc ){
    Tcl_AppendResult(interp, "column number out of range", عدم);
    إرجاع TCL_ERROR;
  }
  Tcl_AppendResult(interp, threadset[i].colv[n], عدم);
  إرجاع TCL_OK;
}

/*
** Usage: thread_result  ID
**
** Wait on the most recent operation to complete, then return the
** result code from that operation.
*/
ساكن صحيح SQLITE_TCLAPI tcl_thread_result(
  فراغ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  صحيح i;
  ثابت حرف *zName;

  إذا( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID", عدم);
    إرجاع TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  إذا( i<0 ) إرجاع TCL_ERROR;
  إذا( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", عدم);
    إرجاع TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  zName = sqlite3ErrName(threadset[i].rc);
  Tcl_AppendResult(interp, zName, عدم);
  إرجاع TCL_OK;
}

/*
** Usage: thread_error  ID
**
** Wait on the most recent operation to complete, then return the
** error string.
*/
ساكن صحيح SQLITE_TCLAPI tcl_thread_error(
  فراغ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  صحيح i;

  إذا( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID", عدم);
    إرجاع TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  إذا( i<0 ) إرجاع TCL_ERROR;
  إذا( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", عدم);
    إرجاع TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  Tcl_AppendResult(interp, threadset[i].zErr, عدم);
  إرجاع TCL_OK;
}

/*
** This procedure runs in the thread to compile an SQL statement.
*/
ساكن فراغ do_compile(Thread *p){
  إذا( p->db==0 ){
    p->zErr = p->zStaticErr = "no database is open";
    p->rc = SQLITE_ERROR;
    إرجاع;
  }
  إذا( p->pStmt ){
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
ساكن صحيح SQLITE_TCLAPI tcl_thread_compile(
  فراغ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  صحيح i;
  إذا( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID SQL", عدم);
    إرجاع TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  إذا( i<0 ) إرجاع TCL_ERROR;
  إذا( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", عدم);
    إرجاع TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  threadset[i].xOp = do_compile;
  sqlite3_free(threadset[i].zArg);
  threadset[i].zArg = sqlite3_mprintf("%s", argv[2]);
  test_barrier();
  threadset[i].opnum++;
  إرجاع TCL_OK;
}

/*
** This procedure runs in the thread to step the virtual machine.
*/
ساكن فراغ do_step(Thread *p){
  صحيح i;
  إذا( p->pStmt==0 ){
    p->zErr = p->zStaticErr = "no virtual machine available";
    p->rc = SQLITE_ERROR;
    إرجاع;
  }
  p->rc = sqlite3_step(p->pStmt);
  إذا( p->rc==SQLITE_ROW ){
    p->argc = sqlite3_column_تعداد(p->pStmt);
    لكل(i=0; i<sqlite3_data_تعداد(p->pStmt); i++){
      p->argv[i] = (حرف*)sqlite3_column_text(p->pStmt, i);
    }
    لكل(i=0; i<p->argc; i++){
      p->colv[i] = sqlite3_column_name(p->pStmt, i);
    }
  }
}

/*
** Usage: thread_step ID
**
** Advance the virtual machine by one step
*/
ساكن صحيح SQLITE_TCLAPI tcl_thread_step(
  فراغ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  صحيح i;
  إذا( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " IDL", عدم);
    إرجاع TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  إذا( i<0 ) إرجاع TCL_ERROR;
  إذا( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", عدم);
    إرجاع TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  threadset[i].xOp = do_step;
  test_barrier();
  threadset[i].opnum++;
  إرجاع TCL_OK;
}

/*
** This procedure runs in the thread to finalize a virtual machine.
*/
ساكن فراغ do_finalize(Thread *p){
  إذا( p->pStmt==0 ){
    p->zErr = p->zStaticErr = "no virtual machine available";
    p->rc = SQLITE_ERROR;
    إرجاع;
  }
  p->rc = sqlite3_finalize(p->pStmt);
  p->pStmt = 0;
}

/*
** Usage: thread_finalize ID
**
** Finalize the virtual machine.
*/
ساكن صحيح SQLITE_TCLAPI tcl_thread_finalize(
  فراغ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  صحيح i;
  إذا( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " IDL", عدم);
    إرجاع TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  إذا( i<0 ) إرجاع TCL_ERROR;
  إذا( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", عدم);
    إرجاع TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  threadset[i].xOp = do_finalize;
  sqlite3_free(threadset[i].zArg);
  threadset[i].zArg = 0;
  test_barrier();
  threadset[i].opnum++;
  إرجاع TCL_OK;
}

/*
** Usage: thread_swap ID ID
**
** Interchange the sqlite* pointer between two threads.
*/
ساكن صحيح SQLITE_TCLAPI tcl_thread_swap(
  فراغ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  صحيح i, j;
  sqlite3 *temp;
  إذا( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID1 ID2", عدم);
    إرجاع TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  إذا( i<0 ) إرجاع TCL_ERROR;
  إذا( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", عدم);
    إرجاع TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  j = parse_thread_id(interp, argv[2]);
  إذا( j<0 ) إرجاع TCL_ERROR;
  إذا( !threadset[j].busy ){
    Tcl_AppendResult(interp, "no such thread", عدم);
    إرجاع TCL_ERROR;
  }
  test_thread_wait(&threadset[j]);
  temp = threadset[i].db;
  threadset[i].db = threadset[j].db;
  threadset[j].db = temp;
  إرجاع TCL_OK;
}

/*
** Usage: thread_db_get ID
**
** Return the database connection pointer for the given thread.  Then
** remove the pointer from the thread itself.  Afterwards, the thread
** can be stopped and the connection can be used by the main thread.
*/
ساكن صحيح SQLITE_TCLAPI tcl_thread_db_get(
  فراغ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  صحيح i;
  حرف zBuf[100];
  خارجي صحيح sqlite3TestMakePointerStr(Tcl_Interp*, حرف*, فراغ*);
  إذا( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID", عدم);
    إرجاع TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  إذا( i<0 ) إرجاع TCL_ERROR;
  إذا( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", عدم);
    إرجاع TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  sqlite3TestMakePointerStr(interp, zBuf, threadset[i].db);
  threadset[i].db = 0;
  Tcl_AppendResult(interp, zBuf, (حرف*)0);
  إرجاع TCL_OK;
}

/*
** Usage: thread_db_put ID DB
**
*/
ساكن صحيح SQLITE_TCLAPI tcl_thread_db_put(
  فراغ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  صحيح i;
  خارجي صحيح sqlite3TestMakePointerStr(Tcl_Interp*, حرف*, فراغ*);
  خارجي فراغ *sqlite3TestTextToPtr(ثابت حرف *);
  إذا( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID DB", عدم);
    إرجاع TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  إذا( i<0 ) إرجاع TCL_ERROR;
  إذا( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", عدم);
    إرجاع TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  أكد( !threadset[i].db );
  threadset[i].db = (sqlite3*)sqlite3TestTextToPtr(argv[2]);
  إرجاع TCL_OK;
}

/*
** Usage: thread_stmt_get ID
**
** Return the database stmt pointer for the given thread.  Then
** remove the pointer from the thread itself. 
*/
ساكن صحيح SQLITE_TCLAPI tcl_thread_stmt_get(
  فراغ *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  صحيح i;
  حرف zBuf[100];
  خارجي صحيح sqlite3TestMakePointerStr(Tcl_Interp*, حرف*, فراغ*);
  إذا( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID", عدم);
    إرجاع TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  إذا( i<0 ) إرجاع TCL_ERROR;
  إذا( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", عدم);
    إرجاع TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  sqlite3TestMakePointerStr(interp, zBuf, threadset[i].pStmt);
  threadset[i].pStmt = 0;
  Tcl_AppendResult(interp, zBuf, (حرف*)0);
  إرجاع TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
صحيح Sqlitetest4_Init(Tcl_Interp *interp){
  ساكن بنية {
     حرف *zName;
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
  صحيح i;

  لكل(i=0; i<حجم(aCmd)/حجم(aCmd[0]); i++){
    Tcl_CreateCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }
  إرجاع TCL_OK;
}
#وإلا
صحيح Sqlitetest4_Init(Tcl_Interp *interp){ إرجاع TCL_OK; }
#endif /* SQLITE_OS_UNIX */
