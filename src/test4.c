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
#அந்தர்பாவ "sqliteInt.h"
#அந்தர்பாவ "tclsqlite.h"
#எனில் SQLITE_OS_UNIX && SQLITE_THREADSAFE
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <pthread.h>
#அந்தர்பாவ <sched.h>
#அந்தர்பாவ <ctype.h>

வெளி மாறிலி எழுத்து *sqlite3ErrName(முழு);

/*
** Each thread is controlled by an instance of the following
** structure.
*/
வகைவரை கட்டமைப்பு Thread Thread;
கட்டமைப்பு Thread {
  /* The first group of fields are writable by the leader and read-only
  ** to the thread. */
  எழுத்து *zFilename;       /* Name of database file */
  வெறுமை (*xOp)(Thread*);  /* next operation to do */
  எழுத்து *zArg;            /* argument usable by xOp */
  முழு opnum;             /* Operation number */
  முழு busy;              /* True if this thread is in use */

  /* The next group of fields are writable by the thread but read-only to the
  ** leader. */
  முழு completed;        /* Number of operations completed */
  sqlite3 *db;           /* Open database */
  sqlite3_stmt *pStmt;     /* Pending operation */
  எழுத்து *zErr;           /* operation error */
  எழுத்து *zStaticErr;     /* Static error message */
  முழு rc;               /* operation return code */
  முழு argc;             /* number of columns in result */
  மாறிலி எழுத்து *argv[100];    /* result columns */
  மாறிலி எழுத்து *colv[100];    /* result column names */
};

/*
** There can be as many as 26 threads running at once.  Each is named
** by a capital letter: A, B, C, ..., Y, Z.
*/
#define N_THREAD 26
நிலையான Thread threadset[N_THREAD];

நிலையான வெறுமை test_barrier(){
  sqlite3_mutex *pMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_APP1);
  sqlite3_mutex_enter(pMutex);
  sqlite3_mutex_leave(pMutex);
}

/*
** The main loop for a thread.  Threads use busy waiting. 
*/
நிலையான வெறுமை *test_thread_main(வெறுமை *pArg){
  Thread *p = (Thread*)pArg;
  எனில்( p->db ){
    sqlite3_close(p->db);
  }
  sqlite3_open(p->zFilename, &p->db);
  எனில்( SQLITE_OK!=sqlite3_errcode(p->db) ){
    p->zErr = strdup(sqlite3_errmsg(p->db));
    sqlite3_close(p->db);
    p->db = 0;
  }
  p->pStmt = 0;
  test_barrier();
  p->completed = 1;
  போது( p->opnum<=p->completed ) sched_yield();
  test_barrier();
  போது( p->xOp ){
    எனில்( p->zErr && p->zErr!=p->zStaticErr ){
      sqlite3_free(p->zErr);
      p->zErr = 0;
    }
    (*p->xOp)(p);
    test_barrier();
    p->completed++;
    போது( p->opnum<=p->completed ) sched_yield();
    test_barrier();
  }
  எனில்( p->pStmt ){
    sqlite3_finalize(p->pStmt);
    p->pStmt = 0;
  }
  எனில்( p->db ){
    sqlite3_close(p->db);
    p->db = 0;
  }
  எனில்( p->zErr && p->zErr!=p->zStaticErr ){
    sqlite3_free(p->zErr);
    p->zErr = 0;
  }
  test_barrier();
  p->completed++;
#ifndef SQLITE_OMIT_DEPRECATED
  sqlite3_thread_cleanup();
#endif
  திரும்பு 0;
}

/*
** Get a thread ID which is an upper case letter.  Return the index.
** If the argument is not a valid thread ID put an error message in
** the interpreter and return -1.
*/
நிலையான முழு parse_thread_id(Tcl_Interp *interp, மாறிலி எழுத்து *zArg){
  எனில்( zArg==0 || zArg[0]==0 || zArg[1]!=0 || !isupper((unsigned எழுத்து)zArg[0]) ){
    Tcl_AppendResult(interp, "thread ID must be an upper case letter", வெற்று);
    திரும்பு -1;
  }
  திரும்பு zArg[0] - 'A';
}

/*
** Usage:    thread_create NAME  FILENAME
**
** NAME should be an upper case letter.  Start the thread running with
** an open connection to the given database.
*/
நிலையான முழு SQLITE_TCLAPI tcl_thread_create(
  வெறுமை *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு argc,              /* Number of arguments */
  மாறிலி எழுத்து **argv      /* Text of each argument */
){
  முழு i;
  pthread_t x;
  முழு rc;

  எனில்( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID FILENAME", வெற்று);
    திரும்பு TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  எனில்( i<0 ) திரும்பு TCL_ERROR;
  எனில்( threadset[i].busy ){
    Tcl_AppendResult(interp, "thread ", argv[1], " is already running", வெற்று);
    திரும்பு TCL_ERROR;
  }
  threadset[i].busy = 1;
  sqlite3_free(threadset[i].zFilename);
  threadset[i].zFilename = sqlite3_mprintf("%s", argv[2]);
  threadset[i].opnum = 1;
  threadset[i].completed = 0;
  rc = pthread_create(&x, 0, test_thread_main, &threadset[i]);
  எனில்( rc ){
    Tcl_AppendResult(interp, "failed to create the thread", வெற்று);
    sqlite3_free(threadset[i].zFilename);
    threadset[i].busy = 0;
    திரும்பு TCL_ERROR;
  }
  pthread_detach(x);
  திரும்பு TCL_OK;
}

/*
** Wait for a thread to reach its idle state.
*/
நிலையான வெறுமை test_thread_wait(Thread *p){
  test_barrier();
  போது( p->opnum>p->completed ) sched_yield();
  test_barrier();
}

/*
** Usage:  thread_wait ID
**
** Wait on thread ID to reach its idle state.
*/
நிலையான முழு SQLITE_TCLAPI tcl_thread_wait(
  வெறுமை *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு argc,              /* Number of arguments */
  மாறிலி எழுத்து **argv      /* Text of each argument */
){
  முழு i;

  எனில்( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID", வெற்று);
    திரும்பு TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  எனில்( i<0 ) திரும்பு TCL_ERROR;
  எனில்( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", வெற்று);
    திரும்பு TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  திரும்பு TCL_OK;
}

/*
** Stop a thread.
*/
நிலையான வெறுமை test_stop_thread(Thread *p){
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
நிலையான முழு SQLITE_TCLAPI tcl_thread_halt(
  வெறுமை *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு argc,              /* Number of arguments */
  மாறிலி எழுத்து **argv      /* Text of each argument */
){
  முழு i;

  எனில்( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID", வெற்று);
    திரும்பு TCL_ERROR;
  }
  எனில்( argv[1][0]=='*' && argv[1][1]==0 ){
    க்கு(i=0; i<N_THREAD; i++){
      எனில்( threadset[i].busy ) test_stop_thread(&threadset[i]);
    }
  }இல்லையெனில்{
    i = parse_thread_id(interp, argv[1]);
    எனில்( i<0 ) திரும்பு TCL_ERROR;
    எனில்( !threadset[i].busy ){
      Tcl_AppendResult(interp, "no such thread", வெற்று);
      திரும்பு TCL_ERROR;
    }
    test_stop_thread(&threadset[i]);
  }
  திரும்பு TCL_OK;
}

/*
** Usage: thread_argc  ID
**
** Wait on the most recent thread_step to complete, then return the
** number of columns in the result set.
*/
நிலையான முழு SQLITE_TCLAPI tcl_thread_argc(
  வெறுமை *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு argc,              /* Number of arguments */
  மாறிலி எழுத்து **argv      /* Text of each argument */
){
  முழு i;
  எழுத்து zBuf[100];

  எனில்( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID", வெற்று);
    திரும்பு TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  எனில்( i<0 ) திரும்பு TCL_ERROR;
  எனில்( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", வெற்று);
    திரும்பு TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  sqlite3_snprintf(அளவு(zBuf), zBuf, "%d", threadset[i].argc);
  Tcl_AppendResult(interp, zBuf, வெற்று);
  திரும்பு TCL_OK;
}

/*
** Usage: thread_argv  ID   N
**
** Wait on the most recent thread_step to complete, then return the
** value of the N-th columns in the result set.
*/
நிலையான முழு SQLITE_TCLAPI tcl_thread_argv(
  வெறுமை *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு argc,              /* Number of arguments */
  மாறிலி எழுத்து **argv      /* Text of each argument */
){
  முழு i;
  முழு n;

  எனில்( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID N", வெற்று);
    திரும்பு TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  எனில்( i<0 ) திரும்பு TCL_ERROR;
  எனில்( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", வெற்று);
    திரும்பு TCL_ERROR;
  }
  எனில்( Tcl_GetInt(interp, argv[2], &n) ) திரும்பு TCL_ERROR;
  test_thread_wait(&threadset[i]);
  எனில்( n<0 || n>=threadset[i].argc ){
    Tcl_AppendResult(interp, "column number out of range", வெற்று);
    திரும்பு TCL_ERROR;
  }
  Tcl_AppendResult(interp, threadset[i].argv[n], வெற்று);
  திரும்பு TCL_OK;
}

/*
** Usage: thread_colname  ID   N
**
** Wait on the most recent thread_step to complete, then return the
** name of the N-th columns in the result set.
*/
நிலையான முழு SQLITE_TCLAPI tcl_thread_colname(
  வெறுமை *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு argc,              /* Number of arguments */
  மாறிலி எழுத்து **argv      /* Text of each argument */
){
  முழு i;
  முழு n;

  எனில்( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID N", வெற்று);
    திரும்பு TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  எனில்( i<0 ) திரும்பு TCL_ERROR;
  எனில்( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", வெற்று);
    திரும்பு TCL_ERROR;
  }
  எனில்( Tcl_GetInt(interp, argv[2], &n) ) திரும்பு TCL_ERROR;
  test_thread_wait(&threadset[i]);
  எனில்( n<0 || n>=threadset[i].argc ){
    Tcl_AppendResult(interp, "column number out of range", வெற்று);
    திரும்பு TCL_ERROR;
  }
  Tcl_AppendResult(interp, threadset[i].colv[n], வெற்று);
  திரும்பு TCL_OK;
}

/*
** Usage: thread_result  ID
**
** Wait on the most recent operation to complete, then return the
** result code from that operation.
*/
நிலையான முழு SQLITE_TCLAPI tcl_thread_result(
  வெறுமை *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு argc,              /* Number of arguments */
  மாறிலி எழுத்து **argv      /* Text of each argument */
){
  முழு i;
  மாறிலி எழுத்து *zName;

  எனில்( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID", வெற்று);
    திரும்பு TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  எனில்( i<0 ) திரும்பு TCL_ERROR;
  எனில்( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", வெற்று);
    திரும்பு TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  zName = sqlite3ErrName(threadset[i].rc);
  Tcl_AppendResult(interp, zName, வெற்று);
  திரும்பு TCL_OK;
}

/*
** Usage: thread_error  ID
**
** Wait on the most recent operation to complete, then return the
** error string.
*/
நிலையான முழு SQLITE_TCLAPI tcl_thread_error(
  வெறுமை *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு argc,              /* Number of arguments */
  மாறிலி எழுத்து **argv      /* Text of each argument */
){
  முழு i;

  எனில்( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID", வெற்று);
    திரும்பு TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  எனில்( i<0 ) திரும்பு TCL_ERROR;
  எனில்( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", வெற்று);
    திரும்பு TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  Tcl_AppendResult(interp, threadset[i].zErr, வெற்று);
  திரும்பு TCL_OK;
}

/*
** This procedure runs in the thread to compile an SQL statement.
*/
நிலையான வெறுமை do_compile(Thread *p){
  எனில்( p->db==0 ){
    p->zErr = p->zStaticErr = "no database is open";
    p->rc = SQLITE_ERROR;
    திரும்பு;
  }
  எனில்( p->pStmt ){
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
நிலையான முழு SQLITE_TCLAPI tcl_thread_compile(
  வெறுமை *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு argc,              /* Number of arguments */
  மாறிலி எழுத்து **argv      /* Text of each argument */
){
  முழு i;
  எனில்( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID SQL", வெற்று);
    திரும்பு TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  எனில்( i<0 ) திரும்பு TCL_ERROR;
  எனில்( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", வெற்று);
    திரும்பு TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  threadset[i].xOp = do_compile;
  sqlite3_free(threadset[i].zArg);
  threadset[i].zArg = sqlite3_mprintf("%s", argv[2]);
  test_barrier();
  threadset[i].opnum++;
  திரும்பு TCL_OK;
}

/*
** This procedure runs in the thread to step the virtual machine.
*/
நிலையான வெறுமை do_step(Thread *p){
  முழு i;
  எனில்( p->pStmt==0 ){
    p->zErr = p->zStaticErr = "no virtual machine available";
    p->rc = SQLITE_ERROR;
    திரும்பு;
  }
  p->rc = sqlite3_step(p->pStmt);
  எனில்( p->rc==SQLITE_ROW ){
    p->argc = sqlite3_column_மொத்தம்(p->pStmt);
    க்கு(i=0; i<sqlite3_data_மொத்தம்(p->pStmt); i++){
      p->argv[i] = (எழுத்து*)sqlite3_column_text(p->pStmt, i);
    }
    க்கு(i=0; i<p->argc; i++){
      p->colv[i] = sqlite3_column_name(p->pStmt, i);
    }
  }
}

/*
** Usage: thread_step ID
**
** Advance the virtual machine by one step
*/
நிலையான முழு SQLITE_TCLAPI tcl_thread_step(
  வெறுமை *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு argc,              /* Number of arguments */
  மாறிலி எழுத்து **argv      /* Text of each argument */
){
  முழு i;
  எனில்( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " IDL", வெற்று);
    திரும்பு TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  எனில்( i<0 ) திரும்பு TCL_ERROR;
  எனில்( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", வெற்று);
    திரும்பு TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  threadset[i].xOp = do_step;
  test_barrier();
  threadset[i].opnum++;
  திரும்பு TCL_OK;
}

/*
** This procedure runs in the thread to finalize a virtual machine.
*/
நிலையான வெறுமை do_finalize(Thread *p){
  எனில்( p->pStmt==0 ){
    p->zErr = p->zStaticErr = "no virtual machine available";
    p->rc = SQLITE_ERROR;
    திரும்பு;
  }
  p->rc = sqlite3_finalize(p->pStmt);
  p->pStmt = 0;
}

/*
** Usage: thread_finalize ID
**
** Finalize the virtual machine.
*/
நிலையான முழு SQLITE_TCLAPI tcl_thread_finalize(
  வெறுமை *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு argc,              /* Number of arguments */
  மாறிலி எழுத்து **argv      /* Text of each argument */
){
  முழு i;
  எனில்( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " IDL", வெற்று);
    திரும்பு TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  எனில்( i<0 ) திரும்பு TCL_ERROR;
  எனில்( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", வெற்று);
    திரும்பு TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  threadset[i].xOp = do_finalize;
  sqlite3_free(threadset[i].zArg);
  threadset[i].zArg = 0;
  test_barrier();
  threadset[i].opnum++;
  திரும்பு TCL_OK;
}

/*
** Usage: thread_swap ID ID
**
** Interchange the sqlite* pointer between two threads.
*/
நிலையான முழு SQLITE_TCLAPI tcl_thread_swap(
  வெறுமை *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு argc,              /* Number of arguments */
  மாறிலி எழுத்து **argv      /* Text of each argument */
){
  முழு i, j;
  sqlite3 *temp;
  எனில்( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID1 ID2", வெற்று);
    திரும்பு TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  எனில்( i<0 ) திரும்பு TCL_ERROR;
  எனில்( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", வெற்று);
    திரும்பு TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  j = parse_thread_id(interp, argv[2]);
  எனில்( j<0 ) திரும்பு TCL_ERROR;
  எனில்( !threadset[j].busy ){
    Tcl_AppendResult(interp, "no such thread", வெற்று);
    திரும்பு TCL_ERROR;
  }
  test_thread_wait(&threadset[j]);
  temp = threadset[i].db;
  threadset[i].db = threadset[j].db;
  threadset[j].db = temp;
  திரும்பு TCL_OK;
}

/*
** Usage: thread_db_get ID
**
** Return the database connection pointer for the given thread.  Then
** remove the pointer from the thread itself.  Afterwards, the thread
** can be stopped and the connection can be used by the main thread.
*/
நிலையான முழு SQLITE_TCLAPI tcl_thread_db_get(
  வெறுமை *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு argc,              /* Number of arguments */
  மாறிலி எழுத்து **argv      /* Text of each argument */
){
  முழு i;
  எழுத்து zBuf[100];
  வெளி முழு sqlite3TestMakePointerStr(Tcl_Interp*, எழுத்து*, வெறுமை*);
  எனில்( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID", வெற்று);
    திரும்பு TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  எனில்( i<0 ) திரும்பு TCL_ERROR;
  எனில்( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", வெற்று);
    திரும்பு TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  sqlite3TestMakePointerStr(interp, zBuf, threadset[i].db);
  threadset[i].db = 0;
  Tcl_AppendResult(interp, zBuf, (எழுத்து*)0);
  திரும்பு TCL_OK;
}

/*
** Usage: thread_db_put ID DB
**
*/
நிலையான முழு SQLITE_TCLAPI tcl_thread_db_put(
  வெறுமை *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு argc,              /* Number of arguments */
  மாறிலி எழுத்து **argv      /* Text of each argument */
){
  முழு i;
  வெளி முழு sqlite3TestMakePointerStr(Tcl_Interp*, எழுத்து*, வெறுமை*);
  வெளி வெறுமை *sqlite3TestTextToPtr(மாறிலி எழுத்து *);
  எனில்( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID DB", வெற்று);
    திரும்பு TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  எனில்( i<0 ) திரும்பு TCL_ERROR;
  எனில்( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", வெற்று);
    திரும்பு TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  ப்ரதிஜ்ஞா( !threadset[i].db );
  threadset[i].db = (sqlite3*)sqlite3TestTextToPtr(argv[2]);
  திரும்பு TCL_OK;
}

/*
** Usage: thread_stmt_get ID
**
** Return the database stmt pointer for the given thread.  Then
** remove the pointer from the thread itself. 
*/
நிலையான முழு SQLITE_TCLAPI tcl_thread_stmt_get(
  வெறுமை *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு argc,              /* Number of arguments */
  மாறிலி எழுத்து **argv      /* Text of each argument */
){
  முழு i;
  எழுத்து zBuf[100];
  வெளி முழு sqlite3TestMakePointerStr(Tcl_Interp*, எழுத்து*, வெறுமை*);
  எனில்( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID", வெற்று);
    திரும்பு TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  எனில்( i<0 ) திரும்பு TCL_ERROR;
  எனில்( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", வெற்று);
    திரும்பு TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  sqlite3TestMakePointerStr(interp, zBuf, threadset[i].pStmt);
  threadset[i].pStmt = 0;
  Tcl_AppendResult(interp, zBuf, (எழுத்து*)0);
  திரும்பு TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
முழு Sqlitetest4_Init(Tcl_Interp *interp){
  நிலையான கட்டமைப்பு {
     எழுத்து *zName;
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
  முழு i;

  க்கு(i=0; i<அளவு(aCmd)/அளவு(aCmd[0]); i++){
    Tcl_CreateCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }
  திரும்பு TCL_OK;
}
#இல்லையெனில்
முழு Sqlitetest4_Init(Tcl_Interp *interp){ திரும்பு TCL_OK; }
#endif /* SQLITE_OS_UNIX */
