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
#અન્તર્ભાવ "sqliteInt.h"
#અન્તર્ભાવ "tclsqlite.h"
#જો SQLITE_OS_UNIX && SQLITE_THREADSAFE
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <pthread.h>
#અન્તર્ભાવ <sched.h>
#અન્તર્ભાવ <ctype.h>

બાહ્ય અચલ અક્ષર *sqlite3ErrName(પૂર્ણાંક);

/*
** Each thread is controlled by an instance of the following
** structure.
*/
પ્રકારવ્યાખ્યા સંરચના Thread Thread;
સંરચના Thread {
  /* The first group of fields are writable by the leader and read-only
  ** to the thread. */
  અક્ષર *zFilename;       /* Name of database file */
  શૂન્ય (*xOp)(Thread*);  /* next operation to do */
  અક્ષર *zArg;            /* argument usable by xOp */
  પૂર્ણાંક opnum;             /* Operation number */
  પૂર્ણાંક busy;              /* True if this thread is in use */

  /* The next group of fields are writable by the thread but read-only to the
  ** leader. */
  પૂર્ણાંક completed;        /* Number of operations completed */
  sqlite3 *db;           /* Open database */
  sqlite3_stmt *pStmt;     /* Pending operation */
  અક્ષર *zErr;           /* operation error */
  અક્ષર *zStaticErr;     /* Static error message */
  પૂર્ણાંક rc;               /* operation return code */
  પૂર્ણાંક argc;             /* number of columns in result */
  અચલ અક્ષર *argv[100];    /* result columns */
  અચલ અક્ષર *colv[100];    /* result column names */
};

/*
** There can be as many as 26 threads running at once.  Each is named
** by a capital letter: A, B, C, ..., Y, Z.
*/
#define N_THREAD 26
સ્થિર Thread threadset[N_THREAD];

સ્થિર શૂન્ય test_barrier(){
  sqlite3_mutex *pMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_APP1);
  sqlite3_mutex_enter(pMutex);
  sqlite3_mutex_leave(pMutex);
}

/*
** The main loop for a thread.  Threads use busy waiting. 
*/
સ્થિર શૂન્ય *test_thread_main(શૂન્ય *pArg){
  Thread *p = (Thread*)pArg;
  જો( p->db ){
    sqlite3_close(p->db);
  }
  sqlite3_open(p->zFilename, &p->db);
  જો( SQLITE_OK!=sqlite3_errcode(p->db) ){
    p->zErr = strdup(sqlite3_errmsg(p->db));
    sqlite3_close(p->db);
    p->db = 0;
  }
  p->pStmt = 0;
  test_barrier();
  p->completed = 1;
  જ્યારે( p->opnum<=p->completed ) sched_yield();
  test_barrier();
  જ્યારે( p->xOp ){
    જો( p->zErr && p->zErr!=p->zStaticErr ){
      sqlite3_free(p->zErr);
      p->zErr = 0;
    }
    (*p->xOp)(p);
    test_barrier();
    p->completed++;
    જ્યારે( p->opnum<=p->completed ) sched_yield();
    test_barrier();
  }
  જો( p->pStmt ){
    sqlite3_finalize(p->pStmt);
    p->pStmt = 0;
  }
  જો( p->db ){
    sqlite3_close(p->db);
    p->db = 0;
  }
  જો( p->zErr && p->zErr!=p->zStaticErr ){
    sqlite3_free(p->zErr);
    p->zErr = 0;
  }
  test_barrier();
  p->completed++;
#ifndef SQLITE_OMIT_DEPRECATED
  sqlite3_thread_cleanup();
#endif
  પાછા 0;
}

/*
** Get a thread ID which is an upper case letter.  Return the index.
** If the argument is not a valid thread ID put an error message in
** the interpreter and return -1.
*/
સ્થિર પૂર્ણાંક parse_thread_id(Tcl_Interp *interp, અચલ અક્ષર *zArg){
  જો( zArg==0 || zArg[0]==0 || zArg[1]!=0 || !isupper((unsigned અક્ષર)zArg[0]) ){
    Tcl_AppendResult(interp, "thread ID must be an upper case letter", ખાલી);
    પાછા -1;
  }
  પાછા zArg[0] - 'A';
}

/*
** Usage:    thread_create NAME  FILENAME
**
** NAME should be an upper case letter.  Start the thread running with
** an open connection to the given database.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI tcl_thread_create(
  શૂન્ય *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક argc,              /* Number of arguments */
  અચલ અક્ષર **argv      /* Text of each argument */
){
  પૂર્ણાંક i;
  pthread_t x;
  પૂર્ણાંક rc;

  જો( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID FILENAME", ખાલી);
    પાછા TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  જો( i<0 ) પાછા TCL_ERROR;
  જો( threadset[i].busy ){
    Tcl_AppendResult(interp, "thread ", argv[1], " is already running", ખાલી);
    પાછા TCL_ERROR;
  }
  threadset[i].busy = 1;
  sqlite3_free(threadset[i].zFilename);
  threadset[i].zFilename = sqlite3_mprintf("%s", argv[2]);
  threadset[i].opnum = 1;
  threadset[i].completed = 0;
  rc = pthread_create(&x, 0, test_thread_main, &threadset[i]);
  જો( rc ){
    Tcl_AppendResult(interp, "failed to create the thread", ખાલી);
    sqlite3_free(threadset[i].zFilename);
    threadset[i].busy = 0;
    પાછા TCL_ERROR;
  }
  pthread_detach(x);
  પાછા TCL_OK;
}

/*
** Wait for a thread to reach its idle state.
*/
સ્થિર શૂન્ય test_thread_wait(Thread *p){
  test_barrier();
  જ્યારે( p->opnum>p->completed ) sched_yield();
  test_barrier();
}

/*
** Usage:  thread_wait ID
**
** Wait on thread ID to reach its idle state.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI tcl_thread_wait(
  શૂન્ય *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક argc,              /* Number of arguments */
  અચલ અક્ષર **argv      /* Text of each argument */
){
  પૂર્ણાંક i;

  જો( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID", ખાલી);
    પાછા TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  જો( i<0 ) પાછા TCL_ERROR;
  જો( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", ખાલી);
    પાછા TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  પાછા TCL_OK;
}

/*
** Stop a thread.
*/
સ્થિર શૂન્ય test_stop_thread(Thread *p){
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
સ્થિર પૂર્ણાંક SQLITE_TCLAPI tcl_thread_halt(
  શૂન્ય *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક argc,              /* Number of arguments */
  અચલ અક્ષર **argv      /* Text of each argument */
){
  પૂર્ણાંક i;

  જો( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID", ખાલી);
    પાછા TCL_ERROR;
  }
  જો( argv[1][0]=='*' && argv[1][1]==0 ){
    માટે(i=0; i<N_THREAD; i++){
      જો( threadset[i].busy ) test_stop_thread(&threadset[i]);
    }
  }નહીંતર{
    i = parse_thread_id(interp, argv[1]);
    જો( i<0 ) પાછા TCL_ERROR;
    જો( !threadset[i].busy ){
      Tcl_AppendResult(interp, "no such thread", ખાલી);
      પાછા TCL_ERROR;
    }
    test_stop_thread(&threadset[i]);
  }
  પાછા TCL_OK;
}

/*
** Usage: thread_argc  ID
**
** Wait on the most recent thread_step to complete, then return the
** number of columns in the result set.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI tcl_thread_argc(
  શૂન્ય *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક argc,              /* Number of arguments */
  અચલ અક્ષર **argv      /* Text of each argument */
){
  પૂર્ણાંક i;
  અક્ષર zBuf[100];

  જો( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID", ખાલી);
    પાછા TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  જો( i<0 ) પાછા TCL_ERROR;
  જો( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", ખાલી);
    પાછા TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  sqlite3_snprintf(કદ(zBuf), zBuf, "%d", threadset[i].argc);
  Tcl_AppendResult(interp, zBuf, ખાલી);
  પાછા TCL_OK;
}

/*
** Usage: thread_argv  ID   N
**
** Wait on the most recent thread_step to complete, then return the
** value of the N-th columns in the result set.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI tcl_thread_argv(
  શૂન્ય *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક argc,              /* Number of arguments */
  અચલ અક્ષર **argv      /* Text of each argument */
){
  પૂર્ણાંક i;
  પૂર્ણાંક n;

  જો( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID N", ખાલી);
    પાછા TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  જો( i<0 ) પાછા TCL_ERROR;
  જો( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", ખાલી);
    પાછા TCL_ERROR;
  }
  જો( Tcl_GetInt(interp, argv[2], &n) ) પાછા TCL_ERROR;
  test_thread_wait(&threadset[i]);
  જો( n<0 || n>=threadset[i].argc ){
    Tcl_AppendResult(interp, "column number out of range", ખાલી);
    પાછા TCL_ERROR;
  }
  Tcl_AppendResult(interp, threadset[i].argv[n], ખાલી);
  પાછા TCL_OK;
}

/*
** Usage: thread_colname  ID   N
**
** Wait on the most recent thread_step to complete, then return the
** name of the N-th columns in the result set.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI tcl_thread_colname(
  શૂન્ય *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક argc,              /* Number of arguments */
  અચલ અક્ષર **argv      /* Text of each argument */
){
  પૂર્ણાંક i;
  પૂર્ણાંક n;

  જો( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID N", ખાલી);
    પાછા TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  જો( i<0 ) પાછા TCL_ERROR;
  જો( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", ખાલી);
    પાછા TCL_ERROR;
  }
  જો( Tcl_GetInt(interp, argv[2], &n) ) પાછા TCL_ERROR;
  test_thread_wait(&threadset[i]);
  જો( n<0 || n>=threadset[i].argc ){
    Tcl_AppendResult(interp, "column number out of range", ખાલી);
    પાછા TCL_ERROR;
  }
  Tcl_AppendResult(interp, threadset[i].colv[n], ખાલી);
  પાછા TCL_OK;
}

/*
** Usage: thread_result  ID
**
** Wait on the most recent operation to complete, then return the
** result code from that operation.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI tcl_thread_result(
  શૂન્ય *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક argc,              /* Number of arguments */
  અચલ અક્ષર **argv      /* Text of each argument */
){
  પૂર્ણાંક i;
  અચલ અક્ષર *zName;

  જો( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID", ખાલી);
    પાછા TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  જો( i<0 ) પાછા TCL_ERROR;
  જો( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", ખાલી);
    પાછા TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  zName = sqlite3ErrName(threadset[i].rc);
  Tcl_AppendResult(interp, zName, ખાલી);
  પાછા TCL_OK;
}

/*
** Usage: thread_error  ID
**
** Wait on the most recent operation to complete, then return the
** error string.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI tcl_thread_error(
  શૂન્ય *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક argc,              /* Number of arguments */
  અચલ અક્ષર **argv      /* Text of each argument */
){
  પૂર્ણાંક i;

  જો( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID", ખાલી);
    પાછા TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  જો( i<0 ) પાછા TCL_ERROR;
  જો( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", ખાલી);
    પાછા TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  Tcl_AppendResult(interp, threadset[i].zErr, ખાલી);
  પાછા TCL_OK;
}

/*
** This procedure runs in the thread to compile an SQL statement.
*/
સ્થિર શૂન્ય do_compile(Thread *p){
  જો( p->db==0 ){
    p->zErr = p->zStaticErr = "no database is open";
    p->rc = SQLITE_ERROR;
    પાછા;
  }
  જો( p->pStmt ){
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
સ્થિર પૂર્ણાંક SQLITE_TCLAPI tcl_thread_compile(
  શૂન્ય *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક argc,              /* Number of arguments */
  અચલ અક્ષર **argv      /* Text of each argument */
){
  પૂર્ણાંક i;
  જો( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID SQL", ખાલી);
    પાછા TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  જો( i<0 ) પાછા TCL_ERROR;
  જો( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", ખાલી);
    પાછા TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  threadset[i].xOp = do_compile;
  sqlite3_free(threadset[i].zArg);
  threadset[i].zArg = sqlite3_mprintf("%s", argv[2]);
  test_barrier();
  threadset[i].opnum++;
  પાછા TCL_OK;
}

/*
** This procedure runs in the thread to step the virtual machine.
*/
સ્થિર શૂન્ય do_step(Thread *p){
  પૂર્ણાંક i;
  જો( p->pStmt==0 ){
    p->zErr = p->zStaticErr = "no virtual machine available";
    p->rc = SQLITE_ERROR;
    પાછા;
  }
  p->rc = sqlite3_step(p->pStmt);
  જો( p->rc==SQLITE_ROW ){
    p->argc = sqlite3_column_ગણતરી(p->pStmt);
    માટે(i=0; i<sqlite3_data_ગણતરી(p->pStmt); i++){
      p->argv[i] = (અક્ષર*)sqlite3_column_text(p->pStmt, i);
    }
    માટે(i=0; i<p->argc; i++){
      p->colv[i] = sqlite3_column_name(p->pStmt, i);
    }
  }
}

/*
** Usage: thread_step ID
**
** Advance the virtual machine by one step
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI tcl_thread_step(
  શૂન્ય *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક argc,              /* Number of arguments */
  અચલ અક્ષર **argv      /* Text of each argument */
){
  પૂર્ણાંક i;
  જો( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " IDL", ખાલી);
    પાછા TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  જો( i<0 ) પાછા TCL_ERROR;
  જો( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", ખાલી);
    પાછા TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  threadset[i].xOp = do_step;
  test_barrier();
  threadset[i].opnum++;
  પાછા TCL_OK;
}

/*
** This procedure runs in the thread to finalize a virtual machine.
*/
સ્થિર શૂન્ય do_finalize(Thread *p){
  જો( p->pStmt==0 ){
    p->zErr = p->zStaticErr = "no virtual machine available";
    p->rc = SQLITE_ERROR;
    પાછા;
  }
  p->rc = sqlite3_finalize(p->pStmt);
  p->pStmt = 0;
}

/*
** Usage: thread_finalize ID
**
** Finalize the virtual machine.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI tcl_thread_finalize(
  શૂન્ય *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક argc,              /* Number of arguments */
  અચલ અક્ષર **argv      /* Text of each argument */
){
  પૂર્ણાંક i;
  જો( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " IDL", ખાલી);
    પાછા TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  જો( i<0 ) પાછા TCL_ERROR;
  જો( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", ખાલી);
    પાછા TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  threadset[i].xOp = do_finalize;
  sqlite3_free(threadset[i].zArg);
  threadset[i].zArg = 0;
  test_barrier();
  threadset[i].opnum++;
  પાછા TCL_OK;
}

/*
** Usage: thread_swap ID ID
**
** Interchange the sqlite* pointer between two threads.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI tcl_thread_swap(
  શૂન્ય *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક argc,              /* Number of arguments */
  અચલ અક્ષર **argv      /* Text of each argument */
){
  પૂર્ણાંક i, j;
  sqlite3 *temp;
  જો( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID1 ID2", ખાલી);
    પાછા TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  જો( i<0 ) પાછા TCL_ERROR;
  જો( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", ખાલી);
    પાછા TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  j = parse_thread_id(interp, argv[2]);
  જો( j<0 ) પાછા TCL_ERROR;
  જો( !threadset[j].busy ){
    Tcl_AppendResult(interp, "no such thread", ખાલી);
    પાછા TCL_ERROR;
  }
  test_thread_wait(&threadset[j]);
  temp = threadset[i].db;
  threadset[i].db = threadset[j].db;
  threadset[j].db = temp;
  પાછા TCL_OK;
}

/*
** Usage: thread_db_get ID
**
** Return the database connection pointer for the given thread.  Then
** remove the pointer from the thread itself.  Afterwards, the thread
** can be stopped and the connection can be used by the main thread.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI tcl_thread_db_get(
  શૂન્ય *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક argc,              /* Number of arguments */
  અચલ અક્ષર **argv      /* Text of each argument */
){
  પૂર્ણાંક i;
  અક્ષર zBuf[100];
  બાહ્ય પૂર્ણાંક sqlite3TestMakePointerStr(Tcl_Interp*, અક્ષર*, શૂન્ય*);
  જો( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID", ખાલી);
    પાછા TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  જો( i<0 ) પાછા TCL_ERROR;
  જો( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", ખાલી);
    પાછા TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  sqlite3TestMakePointerStr(interp, zBuf, threadset[i].db);
  threadset[i].db = 0;
  Tcl_AppendResult(interp, zBuf, (અક્ષર*)0);
  પાછા TCL_OK;
}

/*
** Usage: thread_db_put ID DB
**
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI tcl_thread_db_put(
  શૂન્ય *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક argc,              /* Number of arguments */
  અચલ અક્ષર **argv      /* Text of each argument */
){
  પૂર્ણાંક i;
  બાહ્ય પૂર્ણાંક sqlite3TestMakePointerStr(Tcl_Interp*, અક્ષર*, શૂન્ય*);
  બાહ્ય શૂન્ય *sqlite3TestTextToPtr(અચલ અક્ષર *);
  જો( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID DB", ખાલી);
    પાછા TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  જો( i<0 ) પાછા TCL_ERROR;
  જો( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", ખાલી);
    પાછા TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  પ્રતિજ્ઞા( !threadset[i].db );
  threadset[i].db = (sqlite3*)sqlite3TestTextToPtr(argv[2]);
  પાછા TCL_OK;
}

/*
** Usage: thread_stmt_get ID
**
** Return the database stmt pointer for the given thread.  Then
** remove the pointer from the thread itself. 
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI tcl_thread_stmt_get(
  શૂન્ય *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક argc,              /* Number of arguments */
  અચલ અક્ષર **argv      /* Text of each argument */
){
  પૂર્ણાંક i;
  અક્ષર zBuf[100];
  બાહ્ય પૂર્ણાંક sqlite3TestMakePointerStr(Tcl_Interp*, અક્ષર*, શૂન્ય*);
  જો( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID", ખાલી);
    પાછા TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  જો( i<0 ) પાછા TCL_ERROR;
  જો( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", ખાલી);
    પાછા TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  sqlite3TestMakePointerStr(interp, zBuf, threadset[i].pStmt);
  threadset[i].pStmt = 0;
  Tcl_AppendResult(interp, zBuf, (અક્ષર*)0);
  પાછા TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
પૂર્ણાંક Sqlitetest4_Init(Tcl_Interp *interp){
  સ્થિર સંરચના {
     અક્ષર *zName;
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
  પૂર્ણાંક i;

  માટે(i=0; i<કદ(aCmd)/કદ(aCmd[0]); i++){
    Tcl_CreateCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }
  પાછા TCL_OK;
}
#નહીંતર
પૂર્ણાંક Sqlitetest4_Init(Tcl_Interp *interp){ પાછા TCL_OK; }
#endif /* SQLITE_OS_UNIX */
