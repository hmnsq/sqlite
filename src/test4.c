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
#شامل "sqliteInt.h"
#شامل "tclsqlite.h"
#اگر SQLITE_OS_UNIX && SQLITE_THREADSAFE
#شامل <stdlib.h>
#شامل <string.h>
#شامل <pthread.h>
#شامل <sched.h>
#شامل <ctype.h>

خارجی ثابت حرف *sqlite3ErrName(صحیح);

/*
** Each thread is controlled by an instance of the following
** structure.
*/
قسمتعریف ساخت Thread Thread;
ساخت Thread {
  /* The first group of fields are writable by the leader and read-only
  ** to the thread. */
  حرف *zFilename;       /* Name of database file */
  خالی (*xOp)(Thread*);  /* next operation to do */
  حرف *zArg;            /* argument usable by xOp */
  صحیح opnum;             /* Operation number */
  صحیح busy;              /* True if this thread is in use */

  /* The next group of fields are writable by the thread but read-only to the
  ** leader. */
  صحیح completed;        /* Number of operations completed */
  sqlite3 *db;           /* Open database */
  sqlite3_stmt *pStmt;     /* Pending operation */
  حرف *zErr;           /* operation error */
  حرف *zStaticErr;     /* Static error message */
  صحیح rc;               /* operation return code */
  صحیح argc;             /* number of columns in result */
  ثابت حرف *argv[100];    /* result columns */
  ثابت حرف *colv[100];    /* result column names */
};

/*
** There can be as many as 26 threads running at once.  Each is named
** by a capital letter: A, B, C, ..., Y, Z.
*/
#تعریف_کرو N_THREAD 26
ساکن Thread threadset[N_THREAD];

ساکن خالی test_barrier(){
  sqlite3_mutex *pMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_APP1);
  sqlite3_mutex_enter(pMutex);
  sqlite3_mutex_leave(pMutex);
}

/*
** The main loop for a thread.  Threads use busy waiting. 
*/
ساکن خالی *test_thread_main(خالی *pArg){
  Thread *p = (Thread*)pArg;
  اگر( p->db ){
    sqlite3_close(p->db);
  }
  sqlite3_open(p->zFilename, &p->db);
  اگر( SQLITE_OK!=sqlite3_errcode(p->db) ){
    p->zErr = strdup(sqlite3_errmsg(p->db));
    sqlite3_close(p->db);
    p->db = 0;
  }
  p->pStmt = 0;
  test_barrier();
  p->completed = 1;
  جب( p->opnum<=p->completed ) sched_yield();
  test_barrier();
  جب( p->xOp ){
    اگر( p->zErr && p->zErr!=p->zStaticErr ){
      sqlite3_free(p->zErr);
      p->zErr = 0;
    }
    (*p->xOp)(p);
    test_barrier();
    p->completed++;
    جب( p->opnum<=p->completed ) sched_yield();
    test_barrier();
  }
  اگر( p->pStmt ){
    sqlite3_finalize(p->pStmt);
    p->pStmt = 0;
  }
  اگر( p->db ){
    sqlite3_close(p->db);
    p->db = 0;
  }
  اگر( p->zErr && p->zErr!=p->zStaticErr ){
    sqlite3_free(p->zErr);
    p->zErr = 0;
  }
  test_barrier();
  p->completed++;
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_DEPRECATED
  sqlite3_thread_cleanup();
#اگر_ختم
  واپس 0;
}

/*
** Get a thread ID which is an upper case letter.  Return the index.
** If the argument is not a valid thread ID put an error message in
** the interpreter and return -1.
*/
ساکن صحیح parse_thread_id(Tcl_Interp *interp, ثابت حرف *zArg){
  اگر( zArg==0 || zArg[0]==0 || zArg[1]!=0 || !isupper((غیرعلامتی حرف)zArg[0]) ){
    Tcl_Append_نتیجہ(interp, "thread ID must be an upper case letter", تہی);
    واپس -1;
  }
  واپس zArg[0] - 'A';
}

/*
** Usage:    thread_create NAME  FILENAME
**
** NAME should be an upper case letter.  Start the thread running with
** an open connection to the given database.
*/
ساکن صحیح SQLITE_TCLAPI tcl_thread_create(
  خالی *Not_مستعمل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  صحیح i;
  pthread_t x;
  صحیح rc;

  اگر( argc!=3 ){
    Tcl_Append_نتیجہ(interp, "wrong # args: should be \"", argv[0],
       " ID FILENAME", تہی);
    واپس TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  اگر( i<0 ) واپس TCL_ERROR;
  اگر( threadset[i].busy ){
    Tcl_Append_نتیجہ(interp, "thread ", argv[1], " is already running", تہی);
    واپس TCL_ERROR;
  }
  threadset[i].busy = 1;
  sqlite3_free(threadset[i].zFilename);
  threadset[i].zFilename = sqlite3_mprintf("%s", argv[2]);
  threadset[i].opnum = 1;
  threadset[i].completed = 0;
  rc = pthread_create(&x, 0, test_thread_main, &threadset[i]);
  اگر( rc ){
    Tcl_Append_نتیجہ(interp, "failed to create the thread", تہی);
    sqlite3_free(threadset[i].zFilename);
    threadset[i].busy = 0;
    واپس TCL_ERROR;
  }
  pthread_detach(x);
  واپس TCL_OK;
}

/*
** Wait for a thread to reach its idle state.
*/
ساکن خالی test_thread_wait(Thread *p){
  test_barrier();
  جب( p->opnum>p->completed ) sched_yield();
  test_barrier();
}

/*
** Usage:  thread_wait ID
**
** Wait on thread ID to reach its idle state.
*/
ساکن صحیح SQLITE_TCLAPI tcl_thread_wait(
  خالی *Not_مستعمل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  صحیح i;

  اگر( argc!=2 ){
    Tcl_Append_نتیجہ(interp, "wrong # args: should be \"", argv[0],
       " ID", تہی);
    واپس TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  اگر( i<0 ) واپس TCL_ERROR;
  اگر( !threadset[i].busy ){
    Tcl_Append_نتیجہ(interp, "no such thread", تہی);
    واپس TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  واپس TCL_OK;
}

/*
** Stop a thread.
*/
ساکن خالی test_stop_thread(Thread *p){
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
ساکن صحیح SQLITE_TCLAPI tcl_thread_halt(
  خالی *Not_مستعمل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  صحیح i;

  اگر( argc!=2 ){
    Tcl_Append_نتیجہ(interp, "wrong # args: should be \"", argv[0],
       " ID", تہی);
    واپس TCL_ERROR;
  }
  اگر( argv[1][0]=='*' && argv[1][1]==0 ){
    برائے(i=0; i<N_THREAD; i++){
      اگر( threadset[i].busy ) test_stop_thread(&threadset[i]);
    }
  }ورنہ{
    i = parse_thread_id(interp, argv[1]);
    اگر( i<0 ) واپس TCL_ERROR;
    اگر( !threadset[i].busy ){
      Tcl_Append_نتیجہ(interp, "no such thread", تہی);
      واپس TCL_ERROR;
    }
    test_stop_thread(&threadset[i]);
  }
  واپس TCL_OK;
}

/*
** Usage: thread_argc  ID
**
** Wait on the most recent thread_step to complete, then return the
** number of columns in the result set.
*/
ساکن صحیح SQLITE_TCLAPI tcl_thread_argc(
  خالی *Not_مستعمل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  صحیح i;
  حرف zBuf[100];

  اگر( argc!=2 ){
    Tcl_Append_نتیجہ(interp, "wrong # args: should be \"", argv[0],
       " ID", تہی);
    واپس TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  اگر( i<0 ) واپس TCL_ERROR;
  اگر( !threadset[i].busy ){
    Tcl_Append_نتیجہ(interp, "no such thread", تہی);
    واپس TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  sqlite3_snprintf(حجم(zBuf), zBuf, "%d", threadset[i].argc);
  Tcl_Append_نتیجہ(interp, zBuf, تہی);
  واپس TCL_OK;
}

/*
** Usage: thread_argv  ID   N
**
** Wait on the most recent thread_step to complete, then return the
** value of the N-th columns in the result set.
*/
ساکن صحیح SQLITE_TCLAPI tcl_thread_argv(
  خالی *Not_مستعمل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  صحیح i;
  صحیح n;

  اگر( argc!=3 ){
    Tcl_Append_نتیجہ(interp, "wrong # args: should be \"", argv[0],
       " ID N", تہی);
    واپس TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  اگر( i<0 ) واپس TCL_ERROR;
  اگر( !threadset[i].busy ){
    Tcl_Append_نتیجہ(interp, "no such thread", تہی);
    واپس TCL_ERROR;
  }
  اگر( Tcl_GetInt(interp, argv[2], &n) ) واپس TCL_ERROR;
  test_thread_wait(&threadset[i]);
  اگر( n<0 || n>=threadset[i].argc ){
    Tcl_Append_نتیجہ(interp, "column number out of range", تہی);
    واپس TCL_ERROR;
  }
  Tcl_Append_نتیجہ(interp, threadset[i].argv[n], تہی);
  واپس TCL_OK;
}

/*
** Usage: thread_colname  ID   N
**
** Wait on the most recent thread_step to complete, then return the
** name of the N-th columns in the result set.
*/
ساکن صحیح SQLITE_TCLAPI tcl_thread_colname(
  خالی *Not_مستعمل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  صحیح i;
  صحیح n;

  اگر( argc!=3 ){
    Tcl_Append_نتیجہ(interp, "wrong # args: should be \"", argv[0],
       " ID N", تہی);
    واپس TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  اگر( i<0 ) واپس TCL_ERROR;
  اگر( !threadset[i].busy ){
    Tcl_Append_نتیجہ(interp, "no such thread", تہی);
    واپس TCL_ERROR;
  }
  اگر( Tcl_GetInt(interp, argv[2], &n) ) واپس TCL_ERROR;
  test_thread_wait(&threadset[i]);
  اگر( n<0 || n>=threadset[i].argc ){
    Tcl_Append_نتیجہ(interp, "column number out of range", تہی);
    واپس TCL_ERROR;
  }
  Tcl_Append_نتیجہ(interp, threadset[i].colv[n], تہی);
  واپس TCL_OK;
}

/*
** Usage: thread_result  ID
**
** Wait on the most recent operation to complete, then return the
** result code from that operation.
*/
ساکن صحیح SQLITE_TCLAPI tcl_thread_نتیجہ(
  خالی *Not_مستعمل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  صحیح i;
  ثابت حرف *zName;

  اگر( argc!=2 ){
    Tcl_Append_نتیجہ(interp, "wrong # args: should be \"", argv[0],
       " ID", تہی);
    واپس TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  اگر( i<0 ) واپس TCL_ERROR;
  اگر( !threadset[i].busy ){
    Tcl_Append_نتیجہ(interp, "no such thread", تہی);
    واپس TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  zName = sqlite3ErrName(threadset[i].rc);
  Tcl_Append_نتیجہ(interp, zName, تہی);
  واپس TCL_OK;
}

/*
** Usage: thread_error  ID
**
** Wait on the most recent operation to complete, then return the
** error string.
*/
ساکن صحیح SQLITE_TCLAPI tcl_thread_error(
  خالی *Not_مستعمل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  صحیح i;

  اگر( argc!=2 ){
    Tcl_Append_نتیجہ(interp, "wrong # args: should be \"", argv[0],
       " ID", تہی);
    واپس TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  اگر( i<0 ) واپس TCL_ERROR;
  اگر( !threadset[i].busy ){
    Tcl_Append_نتیجہ(interp, "no such thread", تہی);
    واپس TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  Tcl_Append_نتیجہ(interp, threadset[i].zErr, تہی);
  واپس TCL_OK;
}

/*
** This procedure runs in the thread to compile an SQL statement.
*/
ساکن خالی do_compile(Thread *p){
  اگر( p->db==0 ){
    p->zErr = p->zStaticErr = "no database is open";
    p->rc = SQLITE_ERROR;
    واپس;
  }
  اگر( p->pStmt ){
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
ساکن صحیح SQLITE_TCLAPI tcl_thread_compile(
  خالی *Not_مستعمل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  صحیح i;
  اگر( argc!=3 ){
    Tcl_Append_نتیجہ(interp, "wrong # args: should be \"", argv[0],
       " ID SQL", تہی);
    واپس TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  اگر( i<0 ) واپس TCL_ERROR;
  اگر( !threadset[i].busy ){
    Tcl_Append_نتیجہ(interp, "no such thread", تہی);
    واپس TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  threadset[i].xOp = do_compile;
  sqlite3_free(threadset[i].zArg);
  threadset[i].zArg = sqlite3_mprintf("%s", argv[2]);
  test_barrier();
  threadset[i].opnum++;
  واپس TCL_OK;
}

/*
** This procedure runs in the thread to step the virtual machine.
*/
ساکن خالی do_قدم(Thread *p){
  صحیح i;
  اگر( p->pStmt==0 ){
    p->zErr = p->zStaticErr = "no virtual machine available";
    p->rc = SQLITE_ERROR;
    واپس;
  }
  p->rc = sqlite3_قدم(p->pStmt);
  اگر( p->rc==SQLITE_صف ){
    p->argc = sqlite3_عمود_گنتی(p->pStmt);
    برائے(i=0; i<sqlite3_ڈیٹا_گنتی(p->pStmt); i++){
      p->argv[i] = (حرف*)sqlite3_عمود_متن(p->pStmt, i);
    }
    برائے(i=0; i<p->argc; i++){
      p->colv[i] = sqlite3_عمود_name(p->pStmt, i);
    }
  }
}

/*
** Usage: thread_step ID
**
** Advance the virtual machine by one step
*/
ساکن صحیح SQLITE_TCLAPI tcl_thread_قدم(
  خالی *Not_مستعمل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  صحیح i;
  اگر( argc!=2 ){
    Tcl_Append_نتیجہ(interp, "wrong # args: should be \"", argv[0],
       " IDL", تہی);
    واپس TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  اگر( i<0 ) واپس TCL_ERROR;
  اگر( !threadset[i].busy ){
    Tcl_Append_نتیجہ(interp, "no such thread", تہی);
    واپس TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  threadset[i].xOp = do_قدم;
  test_barrier();
  threadset[i].opnum++;
  واپس TCL_OK;
}

/*
** This procedure runs in the thread to finalize a virtual machine.
*/
ساکن خالی do_finalize(Thread *p){
  اگر( p->pStmt==0 ){
    p->zErr = p->zStaticErr = "no virtual machine available";
    p->rc = SQLITE_ERROR;
    واپس;
  }
  p->rc = sqlite3_finalize(p->pStmt);
  p->pStmt = 0;
}

/*
** Usage: thread_finalize ID
**
** Finalize the virtual machine.
*/
ساکن صحیح SQLITE_TCLAPI tcl_thread_finalize(
  خالی *Not_مستعمل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  صحیح i;
  اگر( argc!=2 ){
    Tcl_Append_نتیجہ(interp, "wrong # args: should be \"", argv[0],
       " IDL", تہی);
    واپس TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  اگر( i<0 ) واپس TCL_ERROR;
  اگر( !threadset[i].busy ){
    Tcl_Append_نتیجہ(interp, "no such thread", تہی);
    واپس TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  threadset[i].xOp = do_finalize;
  sqlite3_free(threadset[i].zArg);
  threadset[i].zArg = 0;
  test_barrier();
  threadset[i].opnum++;
  واپس TCL_OK;
}

/*
** Usage: thread_swap ID ID
**
** Interchange the sqlite* pointer between two threads.
*/
ساکن صحیح SQLITE_TCLAPI tcl_thread_تبادلہ(
  خالی *Not_مستعمل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  صحیح i, j;
  sqlite3 *عارضی;
  اگر( argc!=3 ){
    Tcl_Append_نتیجہ(interp, "wrong # args: should be \"", argv[0],
       " ID1 ID2", تہی);
    واپس TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  اگر( i<0 ) واپس TCL_ERROR;
  اگر( !threadset[i].busy ){
    Tcl_Append_نتیجہ(interp, "no such thread", تہی);
    واپس TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  j = parse_thread_id(interp, argv[2]);
  اگر( j<0 ) واپس TCL_ERROR;
  اگر( !threadset[j].busy ){
    Tcl_Append_نتیجہ(interp, "no such thread", تہی);
    واپس TCL_ERROR;
  }
  test_thread_wait(&threadset[j]);
  عارضی = threadset[i].db;
  threadset[i].db = threadset[j].db;
  threadset[j].db = عارضی;
  واپس TCL_OK;
}

/*
** Usage: thread_db_get ID
**
** Return the database connection pointer for the given thread.  Then
** remove the pointer from the thread itself.  Afterwards, the thread
** can be stopped and the connection can be used by the main thread.
*/
ساکن صحیح SQLITE_TCLAPI tcl_thread_db_get(
  خالی *Not_مستعمل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  صحیح i;
  حرف zBuf[100];
  خارجی صحیح sqlite3_Test_Make_اشارہ_گر_Str(Tcl_Interp*, حرف*, خالی*);
  اگر( argc!=2 ){
    Tcl_Append_نتیجہ(interp, "wrong # args: should be \"", argv[0],
       " ID", تہی);
    واپس TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  اگر( i<0 ) واپس TCL_ERROR;
  اگر( !threadset[i].busy ){
    Tcl_Append_نتیجہ(interp, "no such thread", تہی);
    واپس TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  sqlite3_Test_Make_اشارہ_گر_Str(interp, zBuf, threadset[i].db);
  threadset[i].db = 0;
  Tcl_Append_نتیجہ(interp, zBuf, (حرف*)0);
  واپس TCL_OK;
}

/*
** Usage: thread_db_put ID DB
**
*/
ساکن صحیح SQLITE_TCLAPI tcl_thread_db_put(
  خالی *Not_مستعمل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  صحیح i;
  خارجی صحیح sqlite3_Test_Make_اشارہ_گر_Str(Tcl_Interp*, حرف*, خالی*);
  خارجی خالی *sqlite3_Test_متن_مقصد_Ptr(ثابت حرف *);
  اگر( argc!=3 ){
    Tcl_Append_نتیجہ(interp, "wrong # args: should be \"", argv[0],
       " ID DB", تہی);
    واپس TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  اگر( i<0 ) واپس TCL_ERROR;
  اگر( !threadset[i].busy ){
    Tcl_Append_نتیجہ(interp, "no such thread", تہی);
    واپس TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  أكد( !threadset[i].db );
  threadset[i].db = (sqlite3*)sqlite3_Test_متن_مقصد_Ptr(argv[2]);
  واپس TCL_OK;
}

/*
** Usage: thread_stmt_get ID
**
** Return the database stmt pointer for the given thread.  Then
** remove the pointer from the thread itself. 
*/
ساکن صحیح SQLITE_TCLAPI tcl_thread_stmt_get(
  خالی *Not_مستعمل,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح argc,              /* Number of arguments */
  ثابت حرف **argv      /* Text of each argument */
){
  صحیح i;
  حرف zBuf[100];
  خارجی صحیح sqlite3_Test_Make_اشارہ_گر_Str(Tcl_Interp*, حرف*, خالی*);
  اگر( argc!=2 ){
    Tcl_Append_نتیجہ(interp, "wrong # args: should be \"", argv[0],
       " ID", تہی);
    واپس TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  اگر( i<0 ) واپس TCL_ERROR;
  اگر( !threadset[i].busy ){
    Tcl_Append_نتیجہ(interp, "no such thread", تہی);
    واپس TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  sqlite3_Test_Make_اشارہ_گر_Str(interp, zBuf, threadset[i].pStmt);
  threadset[i].pStmt = 0;
  Tcl_Append_نتیجہ(interp, zBuf, (حرف*)0);
  واپس TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
صحیح Sqlitetest4_Init(Tcl_Interp *interp){
  ساکن ساخت {
     حرف *zName;
     Tcl_CmdProc *xProc;
  } aCmd[] = {
     { "thread_create",     (Tcl_CmdProc*)tcl_thread_create     },
     { "thread_wait",       (Tcl_CmdProc*)tcl_thread_wait       },
     { "thread_halt",       (Tcl_CmdProc*)tcl_thread_halt       },
     { "thread_argc",       (Tcl_CmdProc*)tcl_thread_argc       },
     { "thread_argv",       (Tcl_CmdProc*)tcl_thread_argv       },
     { "thread_colname",    (Tcl_CmdProc*)tcl_thread_colname    },
     { "thread_result",     (Tcl_CmdProc*)tcl_thread_نتیجہ     },
     { "thread_error",      (Tcl_CmdProc*)tcl_thread_error      },
     { "thread_compile",    (Tcl_CmdProc*)tcl_thread_compile    },
     { "thread_step",       (Tcl_CmdProc*)tcl_thread_قدم       },
     { "thread_finalize",   (Tcl_CmdProc*)tcl_thread_finalize   },
     { "thread_swap",       (Tcl_CmdProc*)tcl_thread_تبادلہ       },
     { "thread_db_get",     (Tcl_CmdProc*)tcl_thread_db_get     },
     { "thread_db_put",     (Tcl_CmdProc*)tcl_thread_db_put     },
     { "thread_stmt_get",   (Tcl_CmdProc*)tcl_thread_stmt_get   },
  };
  صحیح i;

  برائے(i=0; i<حجم(aCmd)/حجم(aCmd[0]); i++){
    Tcl_CreateCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }
  واپس TCL_OK;
}
#ورنہ
صحیح Sqlitetest4_Init(Tcl_Interp *interp){ واپس TCL_OK; }
#اگر_ختم /* SQLITE_OS_UNIX */
