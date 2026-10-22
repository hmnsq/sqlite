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
#अन्तर्भाव "sqliteInt.h"
#अन्तर्भाव "tclsqlite.h"
#यदि SQLITE_OS_UNIX && SQLITE_THREADSAFE
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <pthread.h>
#अन्तर्भाव <sched.h>
#अन्तर्भाव <ctype.h>

बाह्य ध्रुव वर्ण *sqlite3ErrName(पूर्णाङ्क);

/*
** Each thread is controlled by an instance of the following
** structure.
*/
प्रकारनिर्देश संरचना Thread Thread;
संरचना Thread {
  /* The first group of fields are writable by the leader and read-only
  ** to the thread. */
  वर्ण *zFilename;       /* Name of database file */
  शून्य (*xOp)(Thread*);  /* next operation to do */
  वर्ण *zArg;            /* argument usable by xOp */
  पूर्णाङ्क opnum;             /* Operation number */
  पूर्णाङ्क busy;              /* True if this thread is in use */

  /* The next group of fields are writable by the thread but read-only to the
  ** leader. */
  पूर्णाङ्क completed;        /* Number of operations completed */
  sqlite3 *db;           /* Open database */
  sqlite3_stmt *pStmt;     /* Pending operation */
  वर्ण *zErr;           /* operation error */
  वर्ण *zStaticErr;     /* Static error message */
  पूर्णाङ्क rc;               /* operation return code */
  पूर्णाङ्क argc;             /* number of columns in result */
  ध्रुव वर्ण *argv[100];    /* result columns */
  ध्रुव वर्ण *colv[100];    /* result column names */
};

/*
** There can be as many as 26 threads running at once.  Each is named
** by a capital letter: A, B, C, ..., Y, Z.
*/
#define N_THREAD 26
स्थिर Thread threadset[N_THREAD];

स्थिर शून्य test_barrier(){
  sqlite3_mutex *pMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_APP1);
  sqlite3_mutex_enter(pMutex);
  sqlite3_mutex_leave(pMutex);
}

/*
** The main loop for a thread.  Threads use busy waiting. 
*/
स्थिर शून्य *test_thread_main(शून्य *pArg){
  Thread *p = (Thread*)pArg;
  यदि( p->db ){
    sqlite3_close(p->db);
  }
  sqlite3_open(p->zFilename, &p->db);
  यदि( SQLITE_OK!=sqlite3_errcode(p->db) ){
    p->zErr = strdup(sqlite3_errmsg(p->db));
    sqlite3_close(p->db);
    p->db = 0;
  }
  p->pStmt = 0;
  test_barrier();
  p->completed = 1;
  यावत्( p->opnum<=p->completed ) sched_yield();
  test_barrier();
  यावत्( p->xOp ){
    यदि( p->zErr && p->zErr!=p->zStaticErr ){
      sqlite3_free(p->zErr);
      p->zErr = 0;
    }
    (*p->xOp)(p);
    test_barrier();
    p->completed++;
    यावत्( p->opnum<=p->completed ) sched_yield();
    test_barrier();
  }
  यदि( p->pStmt ){
    sqlite3_finalize(p->pStmt);
    p->pStmt = 0;
  }
  यदि( p->db ){
    sqlite3_close(p->db);
    p->db = 0;
  }
  यदि( p->zErr && p->zErr!=p->zStaticErr ){
    sqlite3_free(p->zErr);
    p->zErr = 0;
  }
  test_barrier();
  p->completed++;
#ifndef SQLITE_OMIT_DEPRECATED
  sqlite3_thread_cleanup();
#endif
  प्रत्यागम 0;
}

/*
** Get a thread ID which is an upper case letter.  Return the index.
** If the argument is not a valid thread ID put an error message in
** the interpreter and return -1.
*/
स्थिर पूर्णाङ्क parse_thread_id(Tcl_Interp *interp, ध्रुव वर्ण *zArg){
  यदि( zArg==0 || zArg[0]==0 || zArg[1]!=0 || !isupper((unsigned वर्ण)zArg[0]) ){
    Tcl_AppendResult(interp, "thread ID must be an upper case letter", रिक्त);
    प्रत्यागम -1;
  }
  प्रत्यागम zArg[0] - 'A';
}

/*
** Usage:    thread_create NAME  FILENAME
**
** NAME should be an upper case letter.  Start the thread running with
** an open connection to the given database.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI tcl_thread_create(
  शून्य *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क argc,              /* Number of arguments */
  ध्रुव वर्ण **argv      /* Text of each argument */
){
  पूर्णाङ्क i;
  pthread_t x;
  पूर्णाङ्क rc;

  यदि( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID FILENAME", रिक्त);
    प्रत्यागम TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  यदि( i<0 ) प्रत्यागम TCL_ERROR;
  यदि( threadset[i].busy ){
    Tcl_AppendResult(interp, "thread ", argv[1], " is already running", रिक्त);
    प्रत्यागम TCL_ERROR;
  }
  threadset[i].busy = 1;
  sqlite3_free(threadset[i].zFilename);
  threadset[i].zFilename = sqlite3_mprintf("%s", argv[2]);
  threadset[i].opnum = 1;
  threadset[i].completed = 0;
  rc = pthread_create(&x, 0, test_thread_main, &threadset[i]);
  यदि( rc ){
    Tcl_AppendResult(interp, "failed to create the thread", रिक्त);
    sqlite3_free(threadset[i].zFilename);
    threadset[i].busy = 0;
    प्रत्यागम TCL_ERROR;
  }
  pthread_detach(x);
  प्रत्यागम TCL_OK;
}

/*
** Wait for a thread to reach its idle state.
*/
स्थिर शून्य test_thread_wait(Thread *p){
  test_barrier();
  यावत्( p->opnum>p->completed ) sched_yield();
  test_barrier();
}

/*
** Usage:  thread_wait ID
**
** Wait on thread ID to reach its idle state.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI tcl_thread_wait(
  शून्य *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क argc,              /* Number of arguments */
  ध्रुव वर्ण **argv      /* Text of each argument */
){
  पूर्णाङ्क i;

  यदि( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID", रिक्त);
    प्रत्यागम TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  यदि( i<0 ) प्रत्यागम TCL_ERROR;
  यदि( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", रिक्त);
    प्रत्यागम TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  प्रत्यागम TCL_OK;
}

/*
** Stop a thread.
*/
स्थिर शून्य test_stop_thread(Thread *p){
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
स्थिर पूर्णाङ्क SQLITE_TCLAPI tcl_thread_halt(
  शून्य *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क argc,              /* Number of arguments */
  ध्रुव वर्ण **argv      /* Text of each argument */
){
  पूर्णाङ्क i;

  यदि( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID", रिक्त);
    प्रत्यागम TCL_ERROR;
  }
  यदि( argv[1][0]=='*' && argv[1][1]==0 ){
    प्रति(i=0; i<N_THREAD; i++){
      यदि( threadset[i].busy ) test_stop_thread(&threadset[i]);
    }
  }अन्यथा{
    i = parse_thread_id(interp, argv[1]);
    यदि( i<0 ) प्रत्यागम TCL_ERROR;
    यदि( !threadset[i].busy ){
      Tcl_AppendResult(interp, "no such thread", रिक्त);
      प्रत्यागम TCL_ERROR;
    }
    test_stop_thread(&threadset[i]);
  }
  प्रत्यागम TCL_OK;
}

/*
** Usage: thread_argc  ID
**
** Wait on the most recent thread_step to complete, then return the
** number of columns in the result set.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI tcl_thread_argc(
  शून्य *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क argc,              /* Number of arguments */
  ध्रुव वर्ण **argv      /* Text of each argument */
){
  पूर्णाङ्क i;
  वर्ण zBuf[100];

  यदि( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID", रिक्त);
    प्रत्यागम TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  यदि( i<0 ) प्रत्यागम TCL_ERROR;
  यदि( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", रिक्त);
    प्रत्यागम TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  sqlite3_snprintf(परिमाण(zBuf), zBuf, "%d", threadset[i].argc);
  Tcl_AppendResult(interp, zBuf, रिक्त);
  प्रत्यागम TCL_OK;
}

/*
** Usage: thread_argv  ID   N
**
** Wait on the most recent thread_step to complete, then return the
** value of the N-th columns in the result set.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI tcl_thread_argv(
  शून्य *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क argc,              /* Number of arguments */
  ध्रुव वर्ण **argv      /* Text of each argument */
){
  पूर्णाङ्क i;
  पूर्णाङ्क n;

  यदि( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID N", रिक्त);
    प्रत्यागम TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  यदि( i<0 ) प्रत्यागम TCL_ERROR;
  यदि( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", रिक्त);
    प्रत्यागम TCL_ERROR;
  }
  यदि( Tcl_GetInt(interp, argv[2], &n) ) प्रत्यागम TCL_ERROR;
  test_thread_wait(&threadset[i]);
  यदि( n<0 || n>=threadset[i].argc ){
    Tcl_AppendResult(interp, "column number out of range", रिक्त);
    प्रत्यागम TCL_ERROR;
  }
  Tcl_AppendResult(interp, threadset[i].argv[n], रिक्त);
  प्रत्यागम TCL_OK;
}

/*
** Usage: thread_colname  ID   N
**
** Wait on the most recent thread_step to complete, then return the
** name of the N-th columns in the result set.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI tcl_thread_colname(
  शून्य *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क argc,              /* Number of arguments */
  ध्रुव वर्ण **argv      /* Text of each argument */
){
  पूर्णाङ्क i;
  पूर्णाङ्क n;

  यदि( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID N", रिक्त);
    प्रत्यागम TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  यदि( i<0 ) प्रत्यागम TCL_ERROR;
  यदि( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", रिक्त);
    प्रत्यागम TCL_ERROR;
  }
  यदि( Tcl_GetInt(interp, argv[2], &n) ) प्रत्यागम TCL_ERROR;
  test_thread_wait(&threadset[i]);
  यदि( n<0 || n>=threadset[i].argc ){
    Tcl_AppendResult(interp, "column number out of range", रिक्त);
    प्रत्यागम TCL_ERROR;
  }
  Tcl_AppendResult(interp, threadset[i].colv[n], रिक्त);
  प्रत्यागम TCL_OK;
}

/*
** Usage: thread_result  ID
**
** Wait on the most recent operation to complete, then return the
** result code from that operation.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI tcl_thread_result(
  शून्य *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क argc,              /* Number of arguments */
  ध्रुव वर्ण **argv      /* Text of each argument */
){
  पूर्णाङ्क i;
  ध्रुव वर्ण *zName;

  यदि( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID", रिक्त);
    प्रत्यागम TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  यदि( i<0 ) प्रत्यागम TCL_ERROR;
  यदि( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", रिक्त);
    प्रत्यागम TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  zName = sqlite3ErrName(threadset[i].rc);
  Tcl_AppendResult(interp, zName, रिक्त);
  प्रत्यागम TCL_OK;
}

/*
** Usage: thread_error  ID
**
** Wait on the most recent operation to complete, then return the
** error string.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI tcl_thread_error(
  शून्य *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क argc,              /* Number of arguments */
  ध्रुव वर्ण **argv      /* Text of each argument */
){
  पूर्णाङ्क i;

  यदि( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID", रिक्त);
    प्रत्यागम TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  यदि( i<0 ) प्रत्यागम TCL_ERROR;
  यदि( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", रिक्त);
    प्रत्यागम TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  Tcl_AppendResult(interp, threadset[i].zErr, रिक्त);
  प्रत्यागम TCL_OK;
}

/*
** This procedure runs in the thread to compile an SQL statement.
*/
स्थिर शून्य do_compile(Thread *p){
  यदि( p->db==0 ){
    p->zErr = p->zStaticErr = "no database is open";
    p->rc = SQLITE_ERROR;
    प्रत्यागम;
  }
  यदि( p->pStmt ){
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
स्थिर पूर्णाङ्क SQLITE_TCLAPI tcl_thread_compile(
  शून्य *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क argc,              /* Number of arguments */
  ध्रुव वर्ण **argv      /* Text of each argument */
){
  पूर्णाङ्क i;
  यदि( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID SQL", रिक्त);
    प्रत्यागम TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  यदि( i<0 ) प्रत्यागम TCL_ERROR;
  यदि( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", रिक्त);
    प्रत्यागम TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  threadset[i].xOp = do_compile;
  sqlite3_free(threadset[i].zArg);
  threadset[i].zArg = sqlite3_mprintf("%s", argv[2]);
  test_barrier();
  threadset[i].opnum++;
  प्रत्यागम TCL_OK;
}

/*
** This procedure runs in the thread to step the virtual machine.
*/
स्थिर शून्य do_step(Thread *p){
  पूर्णाङ्क i;
  यदि( p->pStmt==0 ){
    p->zErr = p->zStaticErr = "no virtual machine available";
    p->rc = SQLITE_ERROR;
    प्रत्यागम;
  }
  p->rc = sqlite3_step(p->pStmt);
  यदि( p->rc==SQLITE_ROW ){
    p->argc = sqlite3_column_मोजणी(p->pStmt);
    प्रति(i=0; i<sqlite3_data_मोजणी(p->pStmt); i++){
      p->argv[i] = (वर्ण*)sqlite3_column_text(p->pStmt, i);
    }
    प्रति(i=0; i<p->argc; i++){
      p->colv[i] = sqlite3_column_name(p->pStmt, i);
    }
  }
}

/*
** Usage: thread_step ID
**
** Advance the virtual machine by one step
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI tcl_thread_step(
  शून्य *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क argc,              /* Number of arguments */
  ध्रुव वर्ण **argv      /* Text of each argument */
){
  पूर्णाङ्क i;
  यदि( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " IDL", रिक्त);
    प्रत्यागम TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  यदि( i<0 ) प्रत्यागम TCL_ERROR;
  यदि( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", रिक्त);
    प्रत्यागम TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  threadset[i].xOp = do_step;
  test_barrier();
  threadset[i].opnum++;
  प्रत्यागम TCL_OK;
}

/*
** This procedure runs in the thread to finalize a virtual machine.
*/
स्थिर शून्य do_finalize(Thread *p){
  यदि( p->pStmt==0 ){
    p->zErr = p->zStaticErr = "no virtual machine available";
    p->rc = SQLITE_ERROR;
    प्रत्यागम;
  }
  p->rc = sqlite3_finalize(p->pStmt);
  p->pStmt = 0;
}

/*
** Usage: thread_finalize ID
**
** Finalize the virtual machine.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI tcl_thread_finalize(
  शून्य *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क argc,              /* Number of arguments */
  ध्रुव वर्ण **argv      /* Text of each argument */
){
  पूर्णाङ्क i;
  यदि( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " IDL", रिक्त);
    प्रत्यागम TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  यदि( i<0 ) प्रत्यागम TCL_ERROR;
  यदि( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", रिक्त);
    प्रत्यागम TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  threadset[i].xOp = do_finalize;
  sqlite3_free(threadset[i].zArg);
  threadset[i].zArg = 0;
  test_barrier();
  threadset[i].opnum++;
  प्रत्यागम TCL_OK;
}

/*
** Usage: thread_swap ID ID
**
** Interchange the sqlite* pointer between two threads.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI tcl_thread_swap(
  शून्य *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क argc,              /* Number of arguments */
  ध्रुव वर्ण **argv      /* Text of each argument */
){
  पूर्णाङ्क i, j;
  sqlite3 *temp;
  यदि( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID1 ID2", रिक्त);
    प्रत्यागम TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  यदि( i<0 ) प्रत्यागम TCL_ERROR;
  यदि( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", रिक्त);
    प्रत्यागम TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  j = parse_thread_id(interp, argv[2]);
  यदि( j<0 ) प्रत्यागम TCL_ERROR;
  यदि( !threadset[j].busy ){
    Tcl_AppendResult(interp, "no such thread", रिक्त);
    प्रत्यागम TCL_ERROR;
  }
  test_thread_wait(&threadset[j]);
  temp = threadset[i].db;
  threadset[i].db = threadset[j].db;
  threadset[j].db = temp;
  प्रत्यागम TCL_OK;
}

/*
** Usage: thread_db_get ID
**
** Return the database connection pointer for the given thread.  Then
** remove the pointer from the thread itself.  Afterwards, the thread
** can be stopped and the connection can be used by the main thread.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI tcl_thread_db_get(
  शून्य *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क argc,              /* Number of arguments */
  ध्रुव वर्ण **argv      /* Text of each argument */
){
  पूर्णाङ्क i;
  वर्ण zBuf[100];
  बाह्य पूर्णाङ्क sqlite3TestMakePointerStr(Tcl_Interp*, वर्ण*, शून्य*);
  यदि( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID", रिक्त);
    प्रत्यागम TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  यदि( i<0 ) प्रत्यागम TCL_ERROR;
  यदि( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", रिक्त);
    प्रत्यागम TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  sqlite3TestMakePointerStr(interp, zBuf, threadset[i].db);
  threadset[i].db = 0;
  Tcl_AppendResult(interp, zBuf, (वर्ण*)0);
  प्रत्यागम TCL_OK;
}

/*
** Usage: thread_db_put ID DB
**
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI tcl_thread_db_put(
  शून्य *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क argc,              /* Number of arguments */
  ध्रुव वर्ण **argv      /* Text of each argument */
){
  पूर्णाङ्क i;
  बाह्य पूर्णाङ्क sqlite3TestMakePointerStr(Tcl_Interp*, वर्ण*, शून्य*);
  बाह्य शून्य *sqlite3TestTextToPtr(ध्रुव वर्ण *);
  यदि( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID DB", रिक्त);
    प्रत्यागम TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  यदि( i<0 ) प्रत्यागम TCL_ERROR;
  यदि( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", रिक्त);
    प्रत्यागम TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  प्रतिज्ञा( !threadset[i].db );
  threadset[i].db = (sqlite3*)sqlite3TestTextToPtr(argv[2]);
  प्रत्यागम TCL_OK;
}

/*
** Usage: thread_stmt_get ID
**
** Return the database stmt pointer for the given thread.  Then
** remove the pointer from the thread itself. 
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI tcl_thread_stmt_get(
  शून्य *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क argc,              /* Number of arguments */
  ध्रुव वर्ण **argv      /* Text of each argument */
){
  पूर्णाङ्क i;
  वर्ण zBuf[100];
  बाह्य पूर्णाङ्क sqlite3TestMakePointerStr(Tcl_Interp*, वर्ण*, शून्य*);
  यदि( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID", रिक्त);
    प्रत्यागम TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  यदि( i<0 ) प्रत्यागम TCL_ERROR;
  यदि( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", रिक्त);
    प्रत्यागम TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  sqlite3TestMakePointerStr(interp, zBuf, threadset[i].pStmt);
  threadset[i].pStmt = 0;
  Tcl_AppendResult(interp, zBuf, (वर्ण*)0);
  प्रत्यागम TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
पूर्णाङ्क Sqlitetest4_Init(Tcl_Interp *interp){
  स्थिर संरचना {
     वर्ण *zName;
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
  पूर्णाङ्क i;

  प्रति(i=0; i<परिमाण(aCmd)/परिमाण(aCmd[0]); i++){
    Tcl_CreateCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }
  प्रत्यागम TCL_OK;
}
#अन्यथा
पूर्णाङ्क Sqlitetest4_Init(Tcl_Interp *interp){ प्रत्यागम TCL_OK; }
#endif /* SQLITE_OS_UNIX */
