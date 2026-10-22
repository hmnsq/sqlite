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
#অন্তর্ভাব "sqliteInt.h"
#অন্তর্ভাব "tclsqlite.h"
#যদি SQLITE_OS_UNIX && SQLITE_THREADSAFE
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <pthread.h>
#অন্তর্ভাব <sched.h>
#অন্তর্ভাব <ctype.h>

বাহ্যিক ধ্রুবক অক্ষর *sqlite3ErrName(পূর্ণসংখ্যা);

/*
** Each thread is controlled by an instance of the following
** structure.
*/
প্রকারলেখ কাঠামো Thread Thread;
কাঠামো Thread {
  /* The first group of fields are writable by the leader and read-only
  ** to the thread. */
  অক্ষর *zFilename;       /* Name of database file */
  শূন্য (*xOp)(Thread*);  /* next operation to do */
  অক্ষর *zArg;            /* argument usable by xOp */
  পূর্ণসংখ্যা opnum;             /* Operation number */
  পূর্ণসংখ্যা busy;              /* True if this thread is in use */

  /* The next group of fields are writable by the thread but read-only to the
  ** leader. */
  পূর্ণসংখ্যা completed;        /* Number of operations completed */
  sqlite3 *db;           /* Open database */
  sqlite3_stmt *pStmt;     /* Pending operation */
  অক্ষর *zErr;           /* operation error */
  অক্ষর *zStaticErr;     /* Static error message */
  পূর্ণসংখ্যা rc;               /* operation return code */
  পূর্ণসংখ্যা argc;             /* number of columns in result */
  ধ্রুবক অক্ষর *argv[100];    /* result columns */
  ধ্রুবক অক্ষর *colv[100];    /* result column names */
};

/*
** There can be as many as 26 threads running at once.  Each is named
** by a capital letter: A, B, C, ..., Y, Z.
*/
#define N_THREAD 26
স্থির Thread threadset[N_THREAD];

স্থির শূন্য test_barrier(){
  sqlite3_mutex *pMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_APP1);
  sqlite3_mutex_enter(pMutex);
  sqlite3_mutex_leave(pMutex);
}

/*
** The main loop for a thread.  Threads use busy waiting. 
*/
স্থির শূন্য *test_thread_main(শূন্য *pArg){
  Thread *p = (Thread*)pArg;
  যদি( p->db ){
    sqlite3_close(p->db);
  }
  sqlite3_open(p->zFilename, &p->db);
  যদি( SQLITE_OK!=sqlite3_errcode(p->db) ){
    p->zErr = strdup(sqlite3_errmsg(p->db));
    sqlite3_close(p->db);
    p->db = 0;
  }
  p->pStmt = 0;
  test_barrier();
  p->completed = 1;
  যখন( p->opnum<=p->completed ) sched_yield();
  test_barrier();
  যখন( p->xOp ){
    যদি( p->zErr && p->zErr!=p->zStaticErr ){
      sqlite3_free(p->zErr);
      p->zErr = 0;
    }
    (*p->xOp)(p);
    test_barrier();
    p->completed++;
    যখন( p->opnum<=p->completed ) sched_yield();
    test_barrier();
  }
  যদি( p->pStmt ){
    sqlite3_finalize(p->pStmt);
    p->pStmt = 0;
  }
  যদি( p->db ){
    sqlite3_close(p->db);
    p->db = 0;
  }
  যদি( p->zErr && p->zErr!=p->zStaticErr ){
    sqlite3_free(p->zErr);
    p->zErr = 0;
  }
  test_barrier();
  p->completed++;
#ifndef SQLITE_OMIT_DEPRECATED
  sqlite3_thread_cleanup();
#endif
  ফেরত 0;
}

/*
** Get a thread ID which is an upper case letter.  Return the index.
** If the argument is not a valid thread ID put an error message in
** the interpreter and return -1.
*/
স্থির পূর্ণসংখ্যা parse_thread_id(Tcl_Interp *interp, ধ্রুবক অক্ষর *zArg){
  যদি( zArg==0 || zArg[0]==0 || zArg[1]!=0 || !isupper((unsigned অক্ষর)zArg[0]) ){
    Tcl_Append_ফলাফল(interp, "thread ID must be an upper case letter", ফাঁকা);
    ফেরত -1;
  }
  ফেরত zArg[0] - 'A';
}

/*
** Usage:    thread_create NAME  FILENAME
**
** NAME should be an upper case letter.  Start the thread running with
** an open connection to the given database.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI tcl_thread_create(
  শূন্য *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা argc,              /* Number of arguments */
  ধ্রুবক অক্ষর **argv      /* Text of each argument */
){
  পূর্ণসংখ্যা i;
  pthread_t x;
  পূর্ণসংখ্যা rc;

  যদি( argc!=3 ){
    Tcl_Append_ফলাফল(interp, "wrong # args: should be \"", argv[0],
       " ID FILENAME", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  যদি( i<0 ) ফেরত TCL_ERROR;
  যদি( threadset[i].busy ){
    Tcl_Append_ফলাফল(interp, "thread ", argv[1], " is already running", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  threadset[i].busy = 1;
  sqlite3_free(threadset[i].zFilename);
  threadset[i].zFilename = sqlite3_mprintf("%s", argv[2]);
  threadset[i].opnum = 1;
  threadset[i].completed = 0;
  rc = pthread_create(&x, 0, test_thread_main, &threadset[i]);
  যদি( rc ){
    Tcl_Append_ফলাফল(interp, "failed to create the thread", ফাঁকা);
    sqlite3_free(threadset[i].zFilename);
    threadset[i].busy = 0;
    ফেরত TCL_ERROR;
  }
  pthread_detach(x);
  ফেরত TCL_OK;
}

/*
** Wait for a thread to reach its idle state.
*/
স্থির শূন্য test_thread_wait(Thread *p){
  test_barrier();
  যখন( p->opnum>p->completed ) sched_yield();
  test_barrier();
}

/*
** Usage:  thread_wait ID
**
** Wait on thread ID to reach its idle state.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI tcl_thread_wait(
  শূন্য *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা argc,              /* Number of arguments */
  ধ্রুবক অক্ষর **argv      /* Text of each argument */
){
  পূর্ণসংখ্যা i;

  যদি( argc!=2 ){
    Tcl_Append_ফলাফল(interp, "wrong # args: should be \"", argv[0],
       " ID", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  যদি( i<0 ) ফেরত TCL_ERROR;
  যদি( !threadset[i].busy ){
    Tcl_Append_ফলাফল(interp, "no such thread", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  ফেরত TCL_OK;
}

/*
** Stop a thread.
*/
স্থির শূন্য test_stop_thread(Thread *p){
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
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI tcl_thread_halt(
  শূন্য *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা argc,              /* Number of arguments */
  ধ্রুবক অক্ষর **argv      /* Text of each argument */
){
  পূর্ণসংখ্যা i;

  যদি( argc!=2 ){
    Tcl_Append_ফলাফল(interp, "wrong # args: should be \"", argv[0],
       " ID", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  যদি( argv[1][0]=='*' && argv[1][1]==0 ){
    জন্য(i=0; i<N_THREAD; i++){
      যদি( threadset[i].busy ) test_stop_thread(&threadset[i]);
    }
  }অন্যথা{
    i = parse_thread_id(interp, argv[1]);
    যদি( i<0 ) ফেরত TCL_ERROR;
    যদি( !threadset[i].busy ){
      Tcl_Append_ফলাফল(interp, "no such thread", ফাঁকা);
      ফেরত TCL_ERROR;
    }
    test_stop_thread(&threadset[i]);
  }
  ফেরত TCL_OK;
}

/*
** Usage: thread_argc  ID
**
** Wait on the most recent thread_step to complete, then return the
** number of columns in the result set.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI tcl_thread_argc(
  শূন্য *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা argc,              /* Number of arguments */
  ধ্রুবক অক্ষর **argv      /* Text of each argument */
){
  পূর্ণসংখ্যা i;
  অক্ষর zBuf[100];

  যদি( argc!=2 ){
    Tcl_Append_ফলাফল(interp, "wrong # args: should be \"", argv[0],
       " ID", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  যদি( i<0 ) ফেরত TCL_ERROR;
  যদি( !threadset[i].busy ){
    Tcl_Append_ফলাফল(interp, "no such thread", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  sqlite3_snprintf(আকার(zBuf), zBuf, "%d", threadset[i].argc);
  Tcl_Append_ফলাফল(interp, zBuf, ফাঁকা);
  ফেরত TCL_OK;
}

/*
** Usage: thread_argv  ID   N
**
** Wait on the most recent thread_step to complete, then return the
** value of the N-th columns in the result set.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI tcl_thread_argv(
  শূন্য *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা argc,              /* Number of arguments */
  ধ্রুবক অক্ষর **argv      /* Text of each argument */
){
  পূর্ণসংখ্যা i;
  পূর্ণসংখ্যা n;

  যদি( argc!=3 ){
    Tcl_Append_ফলাফল(interp, "wrong # args: should be \"", argv[0],
       " ID N", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  যদি( i<0 ) ফেরত TCL_ERROR;
  যদি( !threadset[i].busy ){
    Tcl_Append_ফলাফল(interp, "no such thread", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  যদি( Tcl_GetInt(interp, argv[2], &n) ) ফেরত TCL_ERROR;
  test_thread_wait(&threadset[i]);
  যদি( n<0 || n>=threadset[i].argc ){
    Tcl_Append_ফলাফল(interp, "column number out of range", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  Tcl_Append_ফলাফল(interp, threadset[i].argv[n], ফাঁকা);
  ফেরত TCL_OK;
}

/*
** Usage: thread_colname  ID   N
**
** Wait on the most recent thread_step to complete, then return the
** name of the N-th columns in the result set.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI tcl_thread_colname(
  শূন্য *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা argc,              /* Number of arguments */
  ধ্রুবক অক্ষর **argv      /* Text of each argument */
){
  পূর্ণসংখ্যা i;
  পূর্ণসংখ্যা n;

  যদি( argc!=3 ){
    Tcl_Append_ফলাফল(interp, "wrong # args: should be \"", argv[0],
       " ID N", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  যদি( i<0 ) ফেরত TCL_ERROR;
  যদি( !threadset[i].busy ){
    Tcl_Append_ফলাফল(interp, "no such thread", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  যদি( Tcl_GetInt(interp, argv[2], &n) ) ফেরত TCL_ERROR;
  test_thread_wait(&threadset[i]);
  যদি( n<0 || n>=threadset[i].argc ){
    Tcl_Append_ফলাফল(interp, "column number out of range", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  Tcl_Append_ফলাফল(interp, threadset[i].colv[n], ফাঁকা);
  ফেরত TCL_OK;
}

/*
** Usage: thread_result  ID
**
** Wait on the most recent operation to complete, then return the
** result code from that operation.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI tcl_thread_ফলাফল(
  শূন্য *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা argc,              /* Number of arguments */
  ধ্রুবক অক্ষর **argv      /* Text of each argument */
){
  পূর্ণসংখ্যা i;
  ধ্রুবক অক্ষর *zName;

  যদি( argc!=2 ){
    Tcl_Append_ফলাফল(interp, "wrong # args: should be \"", argv[0],
       " ID", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  যদি( i<0 ) ফেরত TCL_ERROR;
  যদি( !threadset[i].busy ){
    Tcl_Append_ফলাফল(interp, "no such thread", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  zName = sqlite3ErrName(threadset[i].rc);
  Tcl_Append_ফলাফল(interp, zName, ফাঁকা);
  ফেরত TCL_OK;
}

/*
** Usage: thread_error  ID
**
** Wait on the most recent operation to complete, then return the
** error string.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI tcl_thread_error(
  শূন্য *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা argc,              /* Number of arguments */
  ধ্রুবক অক্ষর **argv      /* Text of each argument */
){
  পূর্ণসংখ্যা i;

  যদি( argc!=2 ){
    Tcl_Append_ফলাফল(interp, "wrong # args: should be \"", argv[0],
       " ID", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  যদি( i<0 ) ফেরত TCL_ERROR;
  যদি( !threadset[i].busy ){
    Tcl_Append_ফলাফল(interp, "no such thread", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  Tcl_Append_ফলাফল(interp, threadset[i].zErr, ফাঁকা);
  ফেরত TCL_OK;
}

/*
** This procedure runs in the thread to compile an SQL statement.
*/
স্থির শূন্য do_compile(Thread *p){
  যদি( p->db==0 ){
    p->zErr = p->zStaticErr = "no database is open";
    p->rc = SQLITE_ERROR;
    ফেরত;
  }
  যদি( p->pStmt ){
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
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI tcl_thread_compile(
  শূন্য *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা argc,              /* Number of arguments */
  ধ্রুবক অক্ষর **argv      /* Text of each argument */
){
  পূর্ণসংখ্যা i;
  যদি( argc!=3 ){
    Tcl_Append_ফলাফল(interp, "wrong # args: should be \"", argv[0],
       " ID SQL", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  যদি( i<0 ) ফেরত TCL_ERROR;
  যদি( !threadset[i].busy ){
    Tcl_Append_ফলাফল(interp, "no such thread", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  threadset[i].xOp = do_compile;
  sqlite3_free(threadset[i].zArg);
  threadset[i].zArg = sqlite3_mprintf("%s", argv[2]);
  test_barrier();
  threadset[i].opnum++;
  ফেরত TCL_OK;
}

/*
** This procedure runs in the thread to step the virtual machine.
*/
স্থির শূন্য do_step(Thread *p){
  পূর্ণসংখ্যা i;
  যদি( p->pStmt==0 ){
    p->zErr = p->zStaticErr = "no virtual machine available";
    p->rc = SQLITE_ERROR;
    ফেরত;
  }
  p->rc = sqlite3_step(p->pStmt);
  যদি( p->rc==SQLITE_ROW ){
    p->argc = sqlite3_column_গুনতি(p->pStmt);
    জন্য(i=0; i<sqlite3_তথ্য_গুনতি(p->pStmt); i++){
      p->argv[i] = (অক্ষর*)sqlite3_column_text(p->pStmt, i);
    }
    জন্য(i=0; i<p->argc; i++){
      p->colv[i] = sqlite3_column_name(p->pStmt, i);
    }
  }
}

/*
** Usage: thread_step ID
**
** Advance the virtual machine by one step
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI tcl_thread_step(
  শূন্য *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা argc,              /* Number of arguments */
  ধ্রুবক অক্ষর **argv      /* Text of each argument */
){
  পূর্ণসংখ্যা i;
  যদি( argc!=2 ){
    Tcl_Append_ফলাফল(interp, "wrong # args: should be \"", argv[0],
       " IDL", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  যদি( i<0 ) ফেরত TCL_ERROR;
  যদি( !threadset[i].busy ){
    Tcl_Append_ফলাফল(interp, "no such thread", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  threadset[i].xOp = do_step;
  test_barrier();
  threadset[i].opnum++;
  ফেরত TCL_OK;
}

/*
** This procedure runs in the thread to finalize a virtual machine.
*/
স্থির শূন্য do_finalize(Thread *p){
  যদি( p->pStmt==0 ){
    p->zErr = p->zStaticErr = "no virtual machine available";
    p->rc = SQLITE_ERROR;
    ফেরত;
  }
  p->rc = sqlite3_finalize(p->pStmt);
  p->pStmt = 0;
}

/*
** Usage: thread_finalize ID
**
** Finalize the virtual machine.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI tcl_thread_finalize(
  শূন্য *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা argc,              /* Number of arguments */
  ধ্রুবক অক্ষর **argv      /* Text of each argument */
){
  পূর্ণসংখ্যা i;
  যদি( argc!=2 ){
    Tcl_Append_ফলাফল(interp, "wrong # args: should be \"", argv[0],
       " IDL", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  যদি( i<0 ) ফেরত TCL_ERROR;
  যদি( !threadset[i].busy ){
    Tcl_Append_ফলাফল(interp, "no such thread", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  threadset[i].xOp = do_finalize;
  sqlite3_free(threadset[i].zArg);
  threadset[i].zArg = 0;
  test_barrier();
  threadset[i].opnum++;
  ফেরত TCL_OK;
}

/*
** Usage: thread_swap ID ID
**
** Interchange the sqlite* pointer between two threads.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI tcl_thread_swap(
  শূন্য *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা argc,              /* Number of arguments */
  ধ্রুবক অক্ষর **argv      /* Text of each argument */
){
  পূর্ণসংখ্যা i, j;
  sqlite3 *অস্থায়ী;
  যদি( argc!=3 ){
    Tcl_Append_ফলাফল(interp, "wrong # args: should be \"", argv[0],
       " ID1 ID2", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  যদি( i<0 ) ফেরত TCL_ERROR;
  যদি( !threadset[i].busy ){
    Tcl_Append_ফলাফল(interp, "no such thread", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  j = parse_thread_id(interp, argv[2]);
  যদি( j<0 ) ফেরত TCL_ERROR;
  যদি( !threadset[j].busy ){
    Tcl_Append_ফলাফল(interp, "no such thread", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  test_thread_wait(&threadset[j]);
  অস্থায়ী = threadset[i].db;
  threadset[i].db = threadset[j].db;
  threadset[j].db = অস্থায়ী;
  ফেরত TCL_OK;
}

/*
** Usage: thread_db_get ID
**
** Return the database connection pointer for the given thread.  Then
** remove the pointer from the thread itself.  Afterwards, the thread
** can be stopped and the connection can be used by the main thread.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI tcl_thread_db_get(
  শূন্য *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা argc,              /* Number of arguments */
  ধ্রুবক অক্ষর **argv      /* Text of each argument */
){
  পূর্ণসংখ্যা i;
  অক্ষর zBuf[100];
  বাহ্যিক পূর্ণসংখ্যা sqlite3TestMakePointerStr(Tcl_Interp*, অক্ষর*, শূন্য*);
  যদি( argc!=2 ){
    Tcl_Append_ফলাফল(interp, "wrong # args: should be \"", argv[0],
       " ID", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  যদি( i<0 ) ফেরত TCL_ERROR;
  যদি( !threadset[i].busy ){
    Tcl_Append_ফলাফল(interp, "no such thread", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  sqlite3TestMakePointerStr(interp, zBuf, threadset[i].db);
  threadset[i].db = 0;
  Tcl_Append_ফলাফল(interp, zBuf, (অক্ষর*)0);
  ফেরত TCL_OK;
}

/*
** Usage: thread_db_put ID DB
**
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI tcl_thread_db_put(
  শূন্য *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা argc,              /* Number of arguments */
  ধ্রুবক অক্ষর **argv      /* Text of each argument */
){
  পূর্ণসংখ্যা i;
  বাহ্যিক পূর্ণসংখ্যা sqlite3TestMakePointerStr(Tcl_Interp*, অক্ষর*, শূন্য*);
  বাহ্যিক শূন্য *sqlite3TestTextToPtr(ধ্রুবক অক্ষর *);
  যদি( argc!=3 ){
    Tcl_Append_ফলাফল(interp, "wrong # args: should be \"", argv[0],
       " ID DB", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  যদি( i<0 ) ফেরত TCL_ERROR;
  যদি( !threadset[i].busy ){
    Tcl_Append_ফলাফল(interp, "no such thread", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  প্রতিজ্ঞা( !threadset[i].db );
  threadset[i].db = (sqlite3*)sqlite3TestTextToPtr(argv[2]);
  ফেরত TCL_OK;
}

/*
** Usage: thread_stmt_get ID
**
** Return the database stmt pointer for the given thread.  Then
** remove the pointer from the thread itself. 
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI tcl_thread_stmt_get(
  শূন্য *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা argc,              /* Number of arguments */
  ধ্রুবক অক্ষর **argv      /* Text of each argument */
){
  পূর্ণসংখ্যা i;
  অক্ষর zBuf[100];
  বাহ্যিক পূর্ণসংখ্যা sqlite3TestMakePointerStr(Tcl_Interp*, অক্ষর*, শূন্য*);
  যদি( argc!=2 ){
    Tcl_Append_ফলাফল(interp, "wrong # args: should be \"", argv[0],
       " ID", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  যদি( i<0 ) ফেরত TCL_ERROR;
  যদি( !threadset[i].busy ){
    Tcl_Append_ফলাফল(interp, "no such thread", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  sqlite3TestMakePointerStr(interp, zBuf, threadset[i].pStmt);
  threadset[i].pStmt = 0;
  Tcl_Append_ফলাফল(interp, zBuf, (অক্ষর*)0);
  ফেরত TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
পূর্ণসংখ্যা Sqlitetest4_Init(Tcl_Interp *interp){
  স্থির কাঠামো {
     অক্ষর *zName;
     Tcl_CmdProc *xProc;
  } aCmd[] = {
     { "thread_create",     (Tcl_CmdProc*)tcl_thread_create     },
     { "thread_wait",       (Tcl_CmdProc*)tcl_thread_wait       },
     { "thread_halt",       (Tcl_CmdProc*)tcl_thread_halt       },
     { "thread_argc",       (Tcl_CmdProc*)tcl_thread_argc       },
     { "thread_argv",       (Tcl_CmdProc*)tcl_thread_argv       },
     { "thread_colname",    (Tcl_CmdProc*)tcl_thread_colname    },
     { "thread_result",     (Tcl_CmdProc*)tcl_thread_ফলাফল     },
     { "thread_error",      (Tcl_CmdProc*)tcl_thread_error      },
     { "thread_compile",    (Tcl_CmdProc*)tcl_thread_compile    },
     { "thread_step",       (Tcl_CmdProc*)tcl_thread_step       },
     { "thread_finalize",   (Tcl_CmdProc*)tcl_thread_finalize   },
     { "thread_swap",       (Tcl_CmdProc*)tcl_thread_swap       },
     { "thread_db_get",     (Tcl_CmdProc*)tcl_thread_db_get     },
     { "thread_db_put",     (Tcl_CmdProc*)tcl_thread_db_put     },
     { "thread_stmt_get",   (Tcl_CmdProc*)tcl_thread_stmt_get   },
  };
  পূর্ণসংখ্যা i;

  জন্য(i=0; i<আকার(aCmd)/আকার(aCmd[0]); i++){
    Tcl_CreateCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }
  ফেরত TCL_OK;
}
#অন্যথা
পূর্ণসংখ্যা Sqlitetest4_Init(Tcl_Interp *interp){ ফেরত TCL_OK; }
#endif /* SQLITE_OS_UNIX */
