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
#समावेश "sqliteInt.h"
#समावेश "tclsqlite.h"
#अगर SQLITE_OS_UNIX && SQLITE_THREADSAFE
#समावेश <stdlib.h>
#समावेश <string.h>
#समावेश <pthread.h>
#समावेश <sched.h>
#समावेश <ctype.h>

बाह्य स्थिर अक्षर *sqlite3ErrName(पूर्णांक);

/*
** Each thread is controlled by an instance of the following
** structure.
*/
प्रकार काष्ठा Thread Thread;
काष्ठा Thread {
  /* The first group of fields are writable by the leader and read-only
  ** to the thread. */
  अक्षर *zFilename;       /* Name of database file */
  व्योम (*xOp)(Thread*);  /* next operation to do */
  अक्षर *zArg;            /* argument usable by xOp */
  पूर्णांक opnum;             /* Operation number */
  पूर्णांक busy;              /* True if this thread is in use */

  /* The next group of fields are writable by the thread but read-only to the
  ** leader. */
  पूर्णांक completed;        /* Number of operations completed */
  sqlite3 *db;           /* Open database */
  sqlite3_stmt *pStmt;     /* Pending operation */
  अक्षर *zErr;           /* operation error */
  अक्षर *zStaticErr;     /* Static error message */
  पूर्णांक rc;               /* operation return code */
  पूर्णांक argc;             /* number of columns in result */
  स्थिर अक्षर *argv[100];    /* result columns */
  स्थिर अक्षर *colv[100];    /* result column names */
};

/*
** There can be as many as 26 threads running at once.  Each is named
** by a capital letter: A, B, C, ..., Y, Z.
*/
#घोषणा N_THREAD 26
अटल Thread threadset[N_THREAD];

अटल व्योम test_barrier(){
  sqlite3_mutex *pMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_APP1);
  sqlite3_mutex_enter(pMutex);
  sqlite3_mutex_leave(pMutex);
}

/*
** The main loop for a thread.  Threads use busy waiting. 
*/
अटल व्योम *test_thread_main(व्योम *pArg){
  Thread *p = (Thread*)pArg;
  अगर( p->db ){
    sqlite3_close(p->db);
  }
  sqlite3_open(p->zFilename, &p->db);
  अगर( SQLITE_OK!=sqlite3_errcode(p->db) ){
    p->zErr = strdup(sqlite3_errmsg(p->db));
    sqlite3_close(p->db);
    p->db = 0;
  }
  p->pStmt = 0;
  test_barrier();
  p->completed = 1;
  जबतक( p->opnum<=p->completed ) sched_yield();
  test_barrier();
  जबतक( p->xOp ){
    अगर( p->zErr && p->zErr!=p->zStaticErr ){
      sqlite3_free(p->zErr);
      p->zErr = 0;
    }
    (*p->xOp)(p);
    test_barrier();
    p->completed++;
    जबतक( p->opnum<=p->completed ) sched_yield();
    test_barrier();
  }
  अगर( p->pStmt ){
    sqlite3_finalize(p->pStmt);
    p->pStmt = 0;
  }
  अगर( p->db ){
    sqlite3_close(p->db);
    p->db = 0;
  }
  अगर( p->zErr && p->zErr!=p->zStaticErr ){
    sqlite3_free(p->zErr);
    p->zErr = 0;
  }
  test_barrier();
  p->completed++;
#अगर_अघोषित SQLITE_OMIT_DEPRECATED
  sqlite3_thread_cleanup();
#पूर्ण_अगर
  वापस 0;
}

/*
** Get a thread ID which is an upper case letter.  Return the index.
** If the argument is not a valid thread ID put an error message in
** the interpreter and return -1.
*/
अटल पूर्णांक parse_thread_id(Tcl_Interp *interp, स्थिर अक्षर *zArg){
  अगर( zArg==0 || zArg[0]==0 || zArg[1]!=0 || !है_बड़ा((अचिन्हित अक्षर)zArg[0]) ){
    Tcl_Append_परिणाम(interp, "thread ID must be an upper case letter", शून्य);
    वापस -1;
  }
  वापस zArg[0] - 'A';
}

/*
** Usage:    thread_create NAME  FILENAME
**
** NAME should be an upper case letter.  Start the thread running with
** an open connection to the given database.
*/
अटल पूर्णांक SQLITE_TCLAPI tcl_thread_create(
  व्योम *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिर अक्षर **argv      /* Text of each argument */
){
  पूर्णांक i;
  pthread_t x;
  पूर्णांक rc;

  अगर( argc!=3 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
       " ID FILENAME", शून्य);
    वापस TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  अगर( i<0 ) वापस TCL_ERROR;
  अगर( threadset[i].busy ){
    Tcl_Append_परिणाम(interp, "thread ", argv[1], " is already running", शून्य);
    वापस TCL_ERROR;
  }
  threadset[i].busy = 1;
  sqlite3_free(threadset[i].zFilename);
  threadset[i].zFilename = sqlite3_mprintf("%s", argv[2]);
  threadset[i].opnum = 1;
  threadset[i].completed = 0;
  rc = pthread_create(&x, 0, test_thread_main, &threadset[i]);
  अगर( rc ){
    Tcl_Append_परिणाम(interp, "failed to create the thread", शून्य);
    sqlite3_free(threadset[i].zFilename);
    threadset[i].busy = 0;
    वापस TCL_ERROR;
  }
  pthread_detach(x);
  वापस TCL_OK;
}

/*
** Wait for a thread to reach its idle state.
*/
अटल व्योम test_thread_wait(Thread *p){
  test_barrier();
  जबतक( p->opnum>p->completed ) sched_yield();
  test_barrier();
}

/*
** Usage:  thread_wait ID
**
** Wait on thread ID to reach its idle state.
*/
अटल पूर्णांक SQLITE_TCLAPI tcl_thread_wait(
  व्योम *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिर अक्षर **argv      /* Text of each argument */
){
  पूर्णांक i;

  अगर( argc!=2 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
       " ID", शून्य);
    वापस TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  अगर( i<0 ) वापस TCL_ERROR;
  अगर( !threadset[i].busy ){
    Tcl_Append_परिणाम(interp, "no such thread", शून्य);
    वापस TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  वापस TCL_OK;
}

/*
** Stop a thread.
*/
अटल व्योम test_stop_thread(Thread *p){
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
अटल पूर्णांक SQLITE_TCLAPI tcl_thread_halt(
  व्योम *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिर अक्षर **argv      /* Text of each argument */
){
  पूर्णांक i;

  अगर( argc!=2 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
       " ID", शून्य);
    वापस TCL_ERROR;
  }
  अगर( argv[1][0]=='*' && argv[1][1]==0 ){
    क्रम(i=0; i<N_THREAD; i++){
      अगर( threadset[i].busy ) test_stop_thread(&threadset[i]);
    }
  }अन्यथा{
    i = parse_thread_id(interp, argv[1]);
    अगर( i<0 ) वापस TCL_ERROR;
    अगर( !threadset[i].busy ){
      Tcl_Append_परिणाम(interp, "no such thread", शून्य);
      वापस TCL_ERROR;
    }
    test_stop_thread(&threadset[i]);
  }
  वापस TCL_OK;
}

/*
** Usage: thread_argc  ID
**
** Wait on the most recent thread_step to complete, then return the
** number of columns in the result set.
*/
अटल पूर्णांक SQLITE_TCLAPI tcl_thread_argc(
  व्योम *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिर अक्षर **argv      /* Text of each argument */
){
  पूर्णांक i;
  अक्षर zBuf[100];

  अगर( argc!=2 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
       " ID", शून्य);
    वापस TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  अगर( i<0 ) वापस TCL_ERROR;
  अगर( !threadset[i].busy ){
    Tcl_Append_परिणाम(interp, "no such thread", शून्य);
    वापस TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  sqlite3_snprintf(माप(zBuf), zBuf, "%d", threadset[i].argc);
  Tcl_Append_परिणाम(interp, zBuf, शून्य);
  वापस TCL_OK;
}

/*
** Usage: thread_argv  ID   N
**
** Wait on the most recent thread_step to complete, then return the
** value of the N-th columns in the result set.
*/
अटल पूर्णांक SQLITE_TCLAPI tcl_thread_argv(
  व्योम *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिर अक्षर **argv      /* Text of each argument */
){
  पूर्णांक i;
  पूर्णांक n;

  अगर( argc!=3 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
       " ID N", शून्य);
    वापस TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  अगर( i<0 ) वापस TCL_ERROR;
  अगर( !threadset[i].busy ){
    Tcl_Append_परिणाम(interp, "no such thread", शून्य);
    वापस TCL_ERROR;
  }
  अगर( Tcl_GetInt(interp, argv[2], &n) ) वापस TCL_ERROR;
  test_thread_wait(&threadset[i]);
  अगर( n<0 || n>=threadset[i].argc ){
    Tcl_Append_परिणाम(interp, "column number out of range", शून्य);
    वापस TCL_ERROR;
  }
  Tcl_Append_परिणाम(interp, threadset[i].argv[n], शून्य);
  वापस TCL_OK;
}

/*
** Usage: thread_colname  ID   N
**
** Wait on the most recent thread_step to complete, then return the
** name of the N-th columns in the result set.
*/
अटल पूर्णांक SQLITE_TCLAPI tcl_thread_colname(
  व्योम *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिर अक्षर **argv      /* Text of each argument */
){
  पूर्णांक i;
  पूर्णांक n;

  अगर( argc!=3 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
       " ID N", शून्य);
    वापस TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  अगर( i<0 ) वापस TCL_ERROR;
  अगर( !threadset[i].busy ){
    Tcl_Append_परिणाम(interp, "no such thread", शून्य);
    वापस TCL_ERROR;
  }
  अगर( Tcl_GetInt(interp, argv[2], &n) ) वापस TCL_ERROR;
  test_thread_wait(&threadset[i]);
  अगर( n<0 || n>=threadset[i].argc ){
    Tcl_Append_परिणाम(interp, "column number out of range", शून्य);
    वापस TCL_ERROR;
  }
  Tcl_Append_परिणाम(interp, threadset[i].colv[n], शून्य);
  वापस TCL_OK;
}

/*
** Usage: thread_result  ID
**
** Wait on the most recent operation to complete, then return the
** result code from that operation.
*/
अटल पूर्णांक SQLITE_TCLAPI tcl_thread_परिणाम(
  व्योम *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिर अक्षर **argv      /* Text of each argument */
){
  पूर्णांक i;
  स्थिर अक्षर *zName;

  अगर( argc!=2 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
       " ID", शून्य);
    वापस TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  अगर( i<0 ) वापस TCL_ERROR;
  अगर( !threadset[i].busy ){
    Tcl_Append_परिणाम(interp, "no such thread", शून्य);
    वापस TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  zName = sqlite3ErrName(threadset[i].rc);
  Tcl_Append_परिणाम(interp, zName, शून्य);
  वापस TCL_OK;
}

/*
** Usage: thread_error  ID
**
** Wait on the most recent operation to complete, then return the
** error string.
*/
अटल पूर्णांक SQLITE_TCLAPI tcl_thread_error(
  व्योम *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिर अक्षर **argv      /* Text of each argument */
){
  पूर्णांक i;

  अगर( argc!=2 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
       " ID", शून्य);
    वापस TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  अगर( i<0 ) वापस TCL_ERROR;
  अगर( !threadset[i].busy ){
    Tcl_Append_परिणाम(interp, "no such thread", शून्य);
    वापस TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  Tcl_Append_परिणाम(interp, threadset[i].zErr, शून्य);
  वापस TCL_OK;
}

/*
** This procedure runs in the thread to compile an SQL statement.
*/
अटल व्योम do_compile(Thread *p){
  अगर( p->db==0 ){
    p->zErr = p->zStaticErr = "no database is open";
    p->rc = SQLITE_ERROR;
    वापस;
  }
  अगर( p->pStmt ){
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
अटल पूर्णांक SQLITE_TCLAPI tcl_thread_compile(
  व्योम *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिर अक्षर **argv      /* Text of each argument */
){
  पूर्णांक i;
  अगर( argc!=3 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
       " ID SQL", शून्य);
    वापस TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  अगर( i<0 ) वापस TCL_ERROR;
  अगर( !threadset[i].busy ){
    Tcl_Append_परिणाम(interp, "no such thread", शून्य);
    वापस TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  threadset[i].xOp = do_compile;
  sqlite3_free(threadset[i].zArg);
  threadset[i].zArg = sqlite3_mprintf("%s", argv[2]);
  test_barrier();
  threadset[i].opnum++;
  वापस TCL_OK;
}

/*
** This procedure runs in the thread to step the virtual machine.
*/
अटल व्योम do_step(Thread *p){
  पूर्णांक i;
  अगर( p->pStmt==0 ){
    p->zErr = p->zStaticErr = "no virtual machine available";
    p->rc = SQLITE_ERROR;
    वापस;
  }
  p->rc = sqlite3_step(p->pStmt);
  अगर( p->rc==SQLITE_ROW ){
    p->argc = sqlite3_column_गिनती(p->pStmt);
    क्रम(i=0; i<sqlite3_तथ्य_गिनती(p->pStmt); i++){
      p->argv[i] = (अक्षर*)sqlite3_column_text(p->pStmt, i);
    }
    क्रम(i=0; i<p->argc; i++){
      p->colv[i] = sqlite3_column_name(p->pStmt, i);
    }
  }
}

/*
** Usage: thread_step ID
**
** Advance the virtual machine by one step
*/
अटल पूर्णांक SQLITE_TCLAPI tcl_thread_step(
  व्योम *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिर अक्षर **argv      /* Text of each argument */
){
  पूर्णांक i;
  अगर( argc!=2 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
       " IDL", शून्य);
    वापस TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  अगर( i<0 ) वापस TCL_ERROR;
  अगर( !threadset[i].busy ){
    Tcl_Append_परिणाम(interp, "no such thread", शून्य);
    वापस TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  threadset[i].xOp = do_step;
  test_barrier();
  threadset[i].opnum++;
  वापस TCL_OK;
}

/*
** This procedure runs in the thread to finalize a virtual machine.
*/
अटल व्योम do_finalize(Thread *p){
  अगर( p->pStmt==0 ){
    p->zErr = p->zStaticErr = "no virtual machine available";
    p->rc = SQLITE_ERROR;
    वापस;
  }
  p->rc = sqlite3_finalize(p->pStmt);
  p->pStmt = 0;
}

/*
** Usage: thread_finalize ID
**
** Finalize the virtual machine.
*/
अटल पूर्णांक SQLITE_TCLAPI tcl_thread_finalize(
  व्योम *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिर अक्षर **argv      /* Text of each argument */
){
  पूर्णांक i;
  अगर( argc!=2 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
       " IDL", शून्य);
    वापस TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  अगर( i<0 ) वापस TCL_ERROR;
  अगर( !threadset[i].busy ){
    Tcl_Append_परिणाम(interp, "no such thread", शून्य);
    वापस TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  threadset[i].xOp = do_finalize;
  sqlite3_free(threadset[i].zArg);
  threadset[i].zArg = 0;
  test_barrier();
  threadset[i].opnum++;
  वापस TCL_OK;
}

/*
** Usage: thread_swap ID ID
**
** Interchange the sqlite* pointer between two threads.
*/
अटल पूर्णांक SQLITE_TCLAPI tcl_thread_swap(
  व्योम *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिर अक्षर **argv      /* Text of each argument */
){
  पूर्णांक i, j;
  sqlite3 *अस्थायी;
  अगर( argc!=3 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
       " ID1 ID2", शून्य);
    वापस TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  अगर( i<0 ) वापस TCL_ERROR;
  अगर( !threadset[i].busy ){
    Tcl_Append_परिणाम(interp, "no such thread", शून्य);
    वापस TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  j = parse_thread_id(interp, argv[2]);
  अगर( j<0 ) वापस TCL_ERROR;
  अगर( !threadset[j].busy ){
    Tcl_Append_परिणाम(interp, "no such thread", शून्य);
    वापस TCL_ERROR;
  }
  test_thread_wait(&threadset[j]);
  अस्थायी = threadset[i].db;
  threadset[i].db = threadset[j].db;
  threadset[j].db = अस्थायी;
  वापस TCL_OK;
}

/*
** Usage: thread_db_get ID
**
** Return the database connection pointer for the given thread.  Then
** remove the pointer from the thread itself.  Afterwards, the thread
** can be stopped and the connection can be used by the main thread.
*/
अटल पूर्णांक SQLITE_TCLAPI tcl_thread_db_get(
  व्योम *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिर अक्षर **argv      /* Text of each argument */
){
  पूर्णांक i;
  अक्षर zBuf[100];
  बाह्य पूर्णांक sqlite3TestMakePointerStr(Tcl_Interp*, अक्षर*, व्योम*);
  अगर( argc!=2 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
       " ID", शून्य);
    वापस TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  अगर( i<0 ) वापस TCL_ERROR;
  अगर( !threadset[i].busy ){
    Tcl_Append_परिणाम(interp, "no such thread", शून्य);
    वापस TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  sqlite3TestMakePointerStr(interp, zBuf, threadset[i].db);
  threadset[i].db = 0;
  Tcl_Append_परिणाम(interp, zBuf, (अक्षर*)0);
  वापस TCL_OK;
}

/*
** Usage: thread_db_put ID DB
**
*/
अटल पूर्णांक SQLITE_TCLAPI tcl_thread_db_put(
  व्योम *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिर अक्षर **argv      /* Text of each argument */
){
  पूर्णांक i;
  बाह्य पूर्णांक sqlite3TestMakePointerStr(Tcl_Interp*, अक्षर*, व्योम*);
  बाह्य व्योम *sqlite3TestTextToPtr(स्थिर अक्षर *);
  अगर( argc!=3 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
       " ID DB", शून्य);
    वापस TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  अगर( i<0 ) वापस TCL_ERROR;
  अगर( !threadset[i].busy ){
    Tcl_Append_परिणाम(interp, "no such thread", शून्य);
    वापस TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  निश्चित( !threadset[i].db );
  threadset[i].db = (sqlite3*)sqlite3TestTextToPtr(argv[2]);
  वापस TCL_OK;
}

/*
** Usage: thread_stmt_get ID
**
** Return the database stmt pointer for the given thread.  Then
** remove the pointer from the thread itself. 
*/
अटल पूर्णांक SQLITE_TCLAPI tcl_thread_stmt_get(
  व्योम *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक argc,              /* Number of arguments */
  स्थिर अक्षर **argv      /* Text of each argument */
){
  पूर्णांक i;
  अक्षर zBuf[100];
  बाह्य पूर्णांक sqlite3TestMakePointerStr(Tcl_Interp*, अक्षर*, व्योम*);
  अगर( argc!=2 ){
    Tcl_Append_परिणाम(interp, "wrong # args: should be \"", argv[0],
       " ID", शून्य);
    वापस TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  अगर( i<0 ) वापस TCL_ERROR;
  अगर( !threadset[i].busy ){
    Tcl_Append_परिणाम(interp, "no such thread", शून्य);
    वापस TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  sqlite3TestMakePointerStr(interp, zBuf, threadset[i].pStmt);
  threadset[i].pStmt = 0;
  Tcl_Append_परिणाम(interp, zBuf, (अक्षर*)0);
  वापस TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
पूर्णांक Sqlitetest4_Init(Tcl_Interp *interp){
  अटल काष्ठा {
     अक्षर *zName;
     Tcl_CmdProc *xProc;
  } aCmd[] = {
     { "thread_create",     (Tcl_CmdProc*)tcl_thread_create     },
     { "thread_wait",       (Tcl_CmdProc*)tcl_thread_wait       },
     { "thread_halt",       (Tcl_CmdProc*)tcl_thread_halt       },
     { "thread_argc",       (Tcl_CmdProc*)tcl_thread_argc       },
     { "thread_argv",       (Tcl_CmdProc*)tcl_thread_argv       },
     { "thread_colname",    (Tcl_CmdProc*)tcl_thread_colname    },
     { "thread_result",     (Tcl_CmdProc*)tcl_thread_परिणाम     },
     { "thread_error",      (Tcl_CmdProc*)tcl_thread_error      },
     { "thread_compile",    (Tcl_CmdProc*)tcl_thread_compile    },
     { "thread_step",       (Tcl_CmdProc*)tcl_thread_step       },
     { "thread_finalize",   (Tcl_CmdProc*)tcl_thread_finalize   },
     { "thread_swap",       (Tcl_CmdProc*)tcl_thread_swap       },
     { "thread_db_get",     (Tcl_CmdProc*)tcl_thread_db_get     },
     { "thread_db_put",     (Tcl_CmdProc*)tcl_thread_db_put     },
     { "thread_stmt_get",   (Tcl_CmdProc*)tcl_thread_stmt_get   },
  };
  पूर्णांक i;

  क्रम(i=0; i<माप(aCmd)/माप(aCmd[0]); i++){
    Tcl_CreateCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }
  वापस TCL_OK;
}
#अन्यथा
पूर्णांक Sqlitetest4_Init(Tcl_Interp *interp){ वापस TCL_OK; }
#पूर्ण_अगर /* SQLITE_OS_UNIX */
