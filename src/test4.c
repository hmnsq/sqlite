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
#include "sqliteInt.h"
#include "tclsqlite.h"
#nếu SQLITE_OS_UNIX && SQLITE_THREADSAFE
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sched.h>
#include <ctype.h>

extern const char *sqlite3ErrName(số_nguyên);

/*
** Each thread is controlled by an instance of the following
** structure.
*/
typedef struct Thread Thread;
struct Thread {
  /* The first group of fields are writable by the leader and read-only
  ** to the thread. */
  char *zFilename;       /* Name of database file */
  void (*xOp)(Thread*);  /* next operation to do */
  char *zArg;            /* argument usable by xOp */
  số_nguyên opnum;             /* Operation number */
  số_nguyên busy;              /* True if this thread is in use */

  /* The next group of fields are writable by the thread but read-only to the
  ** leader. */
  số_nguyên completed;        /* Number of operations completed */
  sqlite3 *db;           /* Open database */
  sqlite3_stmt *pStmt;     /* Pending operation */
  char *zErr;           /* operation error */
  char *zStaticErr;     /* Static error message */
  số_nguyên rc;               /* operation return code */
  số_nguyên argc;             /* number of columns in result */
  const char *argv[100];    /* result columns */
  const char *colv[100];    /* result column names */
};

/*
** There can be as many as 26 threads running at once.  Each is named
** by a capital letter: A, B, C, ..., Y, Z.
*/
#define N_THREAD 26
static Thread threadset[N_THREAD];

static void test_barrier(){
  sqlite3_mutex *pMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_APP1);
  sqlite3_mutex_enter(pMutex);
  sqlite3_mutex_leave(pMutex);
}

/*
** The main loop for a thread.  Threads use busy waiting. 
*/
static void *test_thread_main(void *pArg){
  Thread *p = (Thread*)pArg;
  nếu( p->db ){
    sqlite3_close(p->db);
  }
  sqlite3_open(p->zFilename, &p->db);
  nếu( SQLITE_OK!=sqlite3_errcode(p->db) ){
    p->zErr = strdup(sqlite3_errmsg(p->db));
    sqlite3_close(p->db);
    p->db = 0;
  }
  p->pStmt = 0;
  test_barrier();
  p->completed = 1;
  trong_khi( p->opnum<=p->completed ) sched_yield();
  test_barrier();
  trong_khi( p->xOp ){
    nếu( p->zErr && p->zErr!=p->zStaticErr ){
      sqlite3_free(p->zErr);
      p->zErr = 0;
    }
    (*p->xOp)(p);
    test_barrier();
    p->completed++;
    trong_khi( p->opnum<=p->completed ) sched_yield();
    test_barrier();
  }
  nếu( p->pStmt ){
    sqlite3_finalize(p->pStmt);
    p->pStmt = 0;
  }
  nếu( p->db ){
    sqlite3_close(p->db);
    p->db = 0;
  }
  nếu( p->zErr && p->zErr!=p->zStaticErr ){
    sqlite3_free(p->zErr);
    p->zErr = 0;
  }
  test_barrier();
  p->completed++;
#ifndef SQLITE_OMIT_DEPRECATED
  sqlite3_thread_cleanup();
#endif
  trả_về 0;
}

/*
** Get a thread ID which is an upper case letter.  Return the index.
** If the argument is not a valid thread ID put an error message in
** the interpreter and return -1.
*/
static số_nguyên parse_thread_id(Tcl_Interp *interp, const char *zArg){
  nếu( zArg==0 || zArg[0]==0 || zArg[1]!=0 || !isupper((unsigned char)zArg[0]) ){
    Tcl_AppendResult(interp, "thread ID must be an upper case letter", NULL);
    trả_về -1;
  }
  trả_về zArg[0] - 'A';
}

/*
** Usage:    thread_create NAME  FILENAME
**
** NAME should be an upper case letter.  Start the thread running with
** an open connection to the given database.
*/
static số_nguyên SQLITE_TCLAPI tcl_thread_create(
  void *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  số_nguyên argc,              /* Number of arguments */
  const char **argv      /* Text of each argument */
){
  số_nguyên i;
  pthread_t x;
  số_nguyên rc;

  nếu( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID FILENAME", NULL);
    trả_về TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  nếu( i<0 ) trả_về TCL_ERROR;
  nếu( threadset[i].busy ){
    Tcl_AppendResult(interp, "thread ", argv[1], " is already running", NULL);
    trả_về TCL_ERROR;
  }
  threadset[i].busy = 1;
  sqlite3_free(threadset[i].zFilename);
  threadset[i].zFilename = sqlite3_mprintf("%s", argv[2]);
  threadset[i].opnum = 1;
  threadset[i].completed = 0;
  rc = pthread_create(&x, 0, test_thread_main, &threadset[i]);
  nếu( rc ){
    Tcl_AppendResult(interp, "failed to create the thread", NULL);
    sqlite3_free(threadset[i].zFilename);
    threadset[i].busy = 0;
    trả_về TCL_ERROR;
  }
  pthread_detach(x);
  trả_về TCL_OK;
}

/*
** Wait for a thread to reach its idle state.
*/
static void test_thread_wait(Thread *p){
  test_barrier();
  trong_khi( p->opnum>p->completed ) sched_yield();
  test_barrier();
}

/*
** Usage:  thread_wait ID
**
** Wait on thread ID to reach its idle state.
*/
static số_nguyên SQLITE_TCLAPI tcl_thread_wait(
  void *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  số_nguyên argc,              /* Number of arguments */
  const char **argv      /* Text of each argument */
){
  số_nguyên i;

  nếu( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID", NULL);
    trả_về TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  nếu( i<0 ) trả_về TCL_ERROR;
  nếu( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", NULL);
    trả_về TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  trả_về TCL_OK;
}

/*
** Stop a thread.
*/
static void test_stop_thread(Thread *p){
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
static số_nguyên SQLITE_TCLAPI tcl_thread_halt(
  void *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  số_nguyên argc,              /* Number of arguments */
  const char **argv      /* Text of each argument */
){
  số_nguyên i;

  nếu( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID", NULL);
    trả_về TCL_ERROR;
  }
  nếu( argv[1][0]=='*' && argv[1][1]==0 ){
    cho(i=0; i<N_THREAD; i++){
      nếu( threadset[i].busy ) test_stop_thread(&threadset[i]);
    }
  }không_thì{
    i = parse_thread_id(interp, argv[1]);
    nếu( i<0 ) trả_về TCL_ERROR;
    nếu( !threadset[i].busy ){
      Tcl_AppendResult(interp, "no such thread", NULL);
      trả_về TCL_ERROR;
    }
    test_stop_thread(&threadset[i]);
  }
  trả_về TCL_OK;
}

/*
** Usage: thread_argc  ID
**
** Wait on the most recent thread_step to complete, then return the
** number of columns in the result set.
*/
static số_nguyên SQLITE_TCLAPI tcl_thread_argc(
  void *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  số_nguyên argc,              /* Number of arguments */
  const char **argv      /* Text of each argument */
){
  số_nguyên i;
  char zBuf[100];

  nếu( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID", NULL);
    trả_về TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  nếu( i<0 ) trả_về TCL_ERROR;
  nếu( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", NULL);
    trả_về TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  sqlite3_snprintf(sizeof(zBuf), zBuf, "%d", threadset[i].argc);
  Tcl_AppendResult(interp, zBuf, NULL);
  trả_về TCL_OK;
}

/*
** Usage: thread_argv  ID   N
**
** Wait on the most recent thread_step to complete, then return the
** value of the N-th columns in the result set.
*/
static số_nguyên SQLITE_TCLAPI tcl_thread_argv(
  void *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  số_nguyên argc,              /* Number of arguments */
  const char **argv      /* Text of each argument */
){
  số_nguyên i;
  số_nguyên n;

  nếu( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID N", NULL);
    trả_về TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  nếu( i<0 ) trả_về TCL_ERROR;
  nếu( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", NULL);
    trả_về TCL_ERROR;
  }
  nếu( Tcl_GetInt(interp, argv[2], &n) ) trả_về TCL_ERROR;
  test_thread_wait(&threadset[i]);
  nếu( n<0 || n>=threadset[i].argc ){
    Tcl_AppendResult(interp, "column number out of range", NULL);
    trả_về TCL_ERROR;
  }
  Tcl_AppendResult(interp, threadset[i].argv[n], NULL);
  trả_về TCL_OK;
}

/*
** Usage: thread_colname  ID   N
**
** Wait on the most recent thread_step to complete, then return the
** name of the N-th columns in the result set.
*/
static số_nguyên SQLITE_TCLAPI tcl_thread_colname(
  void *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  số_nguyên argc,              /* Number of arguments */
  const char **argv      /* Text of each argument */
){
  số_nguyên i;
  số_nguyên n;

  nếu( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID N", NULL);
    trả_về TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  nếu( i<0 ) trả_về TCL_ERROR;
  nếu( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", NULL);
    trả_về TCL_ERROR;
  }
  nếu( Tcl_GetInt(interp, argv[2], &n) ) trả_về TCL_ERROR;
  test_thread_wait(&threadset[i]);
  nếu( n<0 || n>=threadset[i].argc ){
    Tcl_AppendResult(interp, "column number out of range", NULL);
    trả_về TCL_ERROR;
  }
  Tcl_AppendResult(interp, threadset[i].colv[n], NULL);
  trả_về TCL_OK;
}

/*
** Usage: thread_result  ID
**
** Wait on the most recent operation to complete, then return the
** result code from that operation.
*/
static số_nguyên SQLITE_TCLAPI tcl_thread_result(
  void *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  số_nguyên argc,              /* Number of arguments */
  const char **argv      /* Text of each argument */
){
  số_nguyên i;
  const char *zName;

  nếu( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID", NULL);
    trả_về TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  nếu( i<0 ) trả_về TCL_ERROR;
  nếu( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", NULL);
    trả_về TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  zName = sqlite3ErrName(threadset[i].rc);
  Tcl_AppendResult(interp, zName, NULL);
  trả_về TCL_OK;
}

/*
** Usage: thread_error  ID
**
** Wait on the most recent operation to complete, then return the
** error string.
*/
static số_nguyên SQLITE_TCLAPI tcl_thread_error(
  void *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  số_nguyên argc,              /* Number of arguments */
  const char **argv      /* Text of each argument */
){
  số_nguyên i;

  nếu( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID", NULL);
    trả_về TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  nếu( i<0 ) trả_về TCL_ERROR;
  nếu( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", NULL);
    trả_về TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  Tcl_AppendResult(interp, threadset[i].zErr, NULL);
  trả_về TCL_OK;
}

/*
** This procedure runs in the thread to compile an SQL statement.
*/
static void do_compile(Thread *p){
  nếu( p->db==0 ){
    p->zErr = p->zStaticErr = "no database is open";
    p->rc = SQLITE_ERROR;
    trả_về;
  }
  nếu( p->pStmt ){
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
static số_nguyên SQLITE_TCLAPI tcl_thread_compile(
  void *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  số_nguyên argc,              /* Number of arguments */
  const char **argv      /* Text of each argument */
){
  số_nguyên i;
  nếu( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID SQL", NULL);
    trả_về TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  nếu( i<0 ) trả_về TCL_ERROR;
  nếu( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", NULL);
    trả_về TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  threadset[i].xOp = do_compile;
  sqlite3_free(threadset[i].zArg);
  threadset[i].zArg = sqlite3_mprintf("%s", argv[2]);
  test_barrier();
  threadset[i].opnum++;
  trả_về TCL_OK;
}

/*
** This procedure runs in the thread to step the virtual machine.
*/
static void do_step(Thread *p){
  số_nguyên i;
  nếu( p->pStmt==0 ){
    p->zErr = p->zStaticErr = "no virtual machine available";
    p->rc = SQLITE_ERROR;
    trả_về;
  }
  p->rc = sqlite3_step(p->pStmt);
  nếu( p->rc==SQLITE_ROW ){
    p->argc = sqlite3_column_count(p->pStmt);
    cho(i=0; i<sqlite3_data_count(p->pStmt); i++){
      p->argv[i] = (char*)sqlite3_column_text(p->pStmt, i);
    }
    cho(i=0; i<p->argc; i++){
      p->colv[i] = sqlite3_column_name(p->pStmt, i);
    }
  }
}

/*
** Usage: thread_step ID
**
** Advance the virtual machine by one step
*/
static số_nguyên SQLITE_TCLAPI tcl_thread_step(
  void *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  số_nguyên argc,              /* Number of arguments */
  const char **argv      /* Text of each argument */
){
  số_nguyên i;
  nếu( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " IDL", NULL);
    trả_về TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  nếu( i<0 ) trả_về TCL_ERROR;
  nếu( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", NULL);
    trả_về TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  threadset[i].xOp = do_step;
  test_barrier();
  threadset[i].opnum++;
  trả_về TCL_OK;
}

/*
** This procedure runs in the thread to finalize a virtual machine.
*/
static void do_finalize(Thread *p){
  nếu( p->pStmt==0 ){
    p->zErr = p->zStaticErr = "no virtual machine available";
    p->rc = SQLITE_ERROR;
    trả_về;
  }
  p->rc = sqlite3_finalize(p->pStmt);
  p->pStmt = 0;
}

/*
** Usage: thread_finalize ID
**
** Finalize the virtual machine.
*/
static số_nguyên SQLITE_TCLAPI tcl_thread_finalize(
  void *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  số_nguyên argc,              /* Number of arguments */
  const char **argv      /* Text of each argument */
){
  số_nguyên i;
  nếu( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " IDL", NULL);
    trả_về TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  nếu( i<0 ) trả_về TCL_ERROR;
  nếu( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", NULL);
    trả_về TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  threadset[i].xOp = do_finalize;
  sqlite3_free(threadset[i].zArg);
  threadset[i].zArg = 0;
  test_barrier();
  threadset[i].opnum++;
  trả_về TCL_OK;
}

/*
** Usage: thread_swap ID ID
**
** Interchange the sqlite* pointer between two threads.
*/
static số_nguyên SQLITE_TCLAPI tcl_thread_swap(
  void *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  số_nguyên argc,              /* Number of arguments */
  const char **argv      /* Text of each argument */
){
  số_nguyên i, j;
  sqlite3 *temp;
  nếu( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID1 ID2", NULL);
    trả_về TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  nếu( i<0 ) trả_về TCL_ERROR;
  nếu( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", NULL);
    trả_về TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  j = parse_thread_id(interp, argv[2]);
  nếu( j<0 ) trả_về TCL_ERROR;
  nếu( !threadset[j].busy ){
    Tcl_AppendResult(interp, "no such thread", NULL);
    trả_về TCL_ERROR;
  }
  test_thread_wait(&threadset[j]);
  temp = threadset[i].db;
  threadset[i].db = threadset[j].db;
  threadset[j].db = temp;
  trả_về TCL_OK;
}

/*
** Usage: thread_db_get ID
**
** Return the database connection pointer for the given thread.  Then
** remove the pointer from the thread itself.  Afterwards, the thread
** can be stopped and the connection can be used by the main thread.
*/
static số_nguyên SQLITE_TCLAPI tcl_thread_db_get(
  void *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  số_nguyên argc,              /* Number of arguments */
  const char **argv      /* Text of each argument */
){
  số_nguyên i;
  char zBuf[100];
  extern số_nguyên sqlite3TestMakePointerStr(Tcl_Interp*, char*, void*);
  nếu( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID", NULL);
    trả_về TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  nếu( i<0 ) trả_về TCL_ERROR;
  nếu( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", NULL);
    trả_về TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  sqlite3TestMakePointerStr(interp, zBuf, threadset[i].db);
  threadset[i].db = 0;
  Tcl_AppendResult(interp, zBuf, (char*)0);
  trả_về TCL_OK;
}

/*
** Usage: thread_db_put ID DB
**
*/
static số_nguyên SQLITE_TCLAPI tcl_thread_db_put(
  void *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  số_nguyên argc,              /* Number of arguments */
  const char **argv      /* Text of each argument */
){
  số_nguyên i;
  extern số_nguyên sqlite3TestMakePointerStr(Tcl_Interp*, char*, void*);
  extern void *sqlite3TestTextToPtr(const char *);
  nếu( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID DB", NULL);
    trả_về TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  nếu( i<0 ) trả_về TCL_ERROR;
  nếu( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", NULL);
    trả_về TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  assert( !threadset[i].db );
  threadset[i].db = (sqlite3*)sqlite3TestTextToPtr(argv[2]);
  trả_về TCL_OK;
}

/*
** Usage: thread_stmt_get ID
**
** Return the database stmt pointer for the given thread.  Then
** remove the pointer from the thread itself. 
*/
static số_nguyên SQLITE_TCLAPI tcl_thread_stmt_get(
  void *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  số_nguyên argc,              /* Number of arguments */
  const char **argv      /* Text of each argument */
){
  số_nguyên i;
  char zBuf[100];
  extern số_nguyên sqlite3TestMakePointerStr(Tcl_Interp*, char*, void*);
  nếu( argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " ID", NULL);
    trả_về TCL_ERROR;
  }
  i = parse_thread_id(interp, argv[1]);
  nếu( i<0 ) trả_về TCL_ERROR;
  nếu( !threadset[i].busy ){
    Tcl_AppendResult(interp, "no such thread", NULL);
    trả_về TCL_ERROR;
  }
  test_thread_wait(&threadset[i]);
  sqlite3TestMakePointerStr(interp, zBuf, threadset[i].pStmt);
  threadset[i].pStmt = 0;
  Tcl_AppendResult(interp, zBuf, (char*)0);
  trả_về TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
số_nguyên Sqlitetest4_Init(Tcl_Interp *interp){
  static struct {
     char *zName;
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
  số_nguyên i;

  cho(i=0; i<sizeof(aCmd)/sizeof(aCmd[0]); i++){
    Tcl_CreateCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }
  trả_về TCL_OK;
}
#không_thì
số_nguyên Sqlitetest4_Init(Tcl_Interp *interp){ trả_về TCL_OK; }
#endif /* SQLITE_OS_UNIX */
