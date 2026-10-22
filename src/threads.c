/*
** 2012 July 21
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
******************************************************************************
**
** This file presents a simple cross-platform threading interface for
** use internally by SQLite.
**
** A "thread" can be created using sqlite3ThreadCreate().  This thread
** runs independently of its creator until it is joined using
** sqlite3ThreadJoin(), at which point it terminates.
**
** Threads do not have to be real.  It could be that the work of the
** "thread" is done by the main thread at either the sqlite3ThreadCreate()
** or sqlite3ThreadJoin() call.  This is, in fact, what happens in
** single threaded systems.  Nothing in SQLite requires multiple threads.
** This interface exists so that applications that want to take advantage
** of multiple cores can do so, while also allowing applications to stay
** single-threaded if desired.
*/
#شامل "sqliteInt.h"
#اگر SQLITE_OS_WIN
#  include "os_win.h"
#endif

#اگر SQLITE_MAX_WORKER_THREADS>0

/********************************* Unix Pthreads ****************************/
#اگر SQLITE_OS_UNIX && defined(SQLITE_MUTEX_PTHREADS) && SQLITE_THREADSAFE>0

#define SQLITE_THREADS_IMPLEMENTED 1  /* Prevent the single-thread code below */
#شامل <pthread.h>

/* A running thread */
ساختار SQLiteThread {
  pthread_t tid;                 /* Thread ID */
  صحیح done;                      /* Set to true when thread finishes */
  تهی *pOut;                    /* Result returned by the thread */
  تهی *(*xTask)(تهی*);         /* The thread routine */
  تهی *pIn;                     /* Argument to the thread */
};

/* Create a new thread */
صحیح sqlite3ThreadCreate(
  SQLiteThread **ppThread,  /* OUT: Write the thread object here */
  تهی *(*xTask)(تهی*),    /* Routine to run in a separate thread */
  تهی *pIn                 /* Argument passed into xTask() */
){
  SQLiteThread *p;
  صحیح rc;

  أكد( ppThread!=0 );
  أكد( xTask!=0 );
  /* This routine is never used in single-threaded mode */
  أكد( sqlite3GlobalConfig.bCoreMutex!=0 );

  *ppThread = 0;
  p = sqlite3Malloc(اندازه(*p));
  اگر( p==0 ) بازگشت SQLITE_NOMEM_BKPT;
  memset(p, 0, اندازه(*p));
  p->xTask = xTask;
  p->pIn = pIn;
  /* If the SQLITE_TESTCTRL_FAULT_INSTALL callback is registered to a 
  ** function that returns SQLITE_ERROR when passed the argument 200, that
  ** forces worker threads to run sequentially and deterministically 
  ** for testing purposes. */
  اگر( sqlite3FaultSim(200) ){
    rc = 1;
  }وگرنه{    
    rc = pthread_create(&p->tid, 0, xTask, pIn);
  }
  اگر( rc ){
    p->done = 1;
    p->pOut = xTask(pIn);
  }
  *ppThread = p;
  بازگشت SQLITE_OK;
}

/* Get the results of the thread */
صحیح sqlite3ThreadJoin(SQLiteThread *p, تهی **ppOut){
  صحیح rc;

  أكد( ppOut!=0 );
  اگر( NEVER(p==0) ) بازگشت SQLITE_NOMEM_BKPT;
  اگر( p->done ){
    *ppOut = p->pOut;
    rc = SQLITE_OK;
  }وگرنه{
    rc = pthread_join(p->tid, ppOut) ? SQLITE_ERROR : SQLITE_OK;
  }
  sqlite3_free(p);
  بازگشت rc;
}

#endif /* SQLITE_OS_UNIX && defined(SQLITE_MUTEX_PTHREADS) */
/******************************** End Unix Pthreads *************************/


/********************************* Win32 Threads ****************************/
#اگر SQLITE_OS_WIN_THREADS

#define SQLITE_THREADS_IMPLEMENTED 1  /* Prevent the single-thread code below */
#شامل <process.h>

/* A running thread */
ساختار SQLiteThread {
  تهی *tid;               /* The thread handle */
  unsigned id;             /* The thread identifier */
  تهی *(*xTask)(تهی*);   /* The routine to run as a thread */
  تهی *pIn;               /* Argument to xTask */
  تهی *pResult;           /* Result of xTask */
};

/* Thread procedure Win32 compatibility shim */
ایستا unsigned __stdcall sqlite3ThreadProc(
  تهی *pArg  /* IN: Pointer to the SQLiteThread structure */
){
  SQLiteThread *p = (SQLiteThread *)pArg;

  أكد( p!=0 );
#اگر 0
  /*
  ** This assert appears to trigger spuriously on certain
  ** versions of Windows, possibly due to _beginthreadex()
  ** and/or CreateThread() not fully setting their thread
  ** ID parameter before starting the thread.
  */
  أكد( p->id==GetCurrentThreadId() );
#endif
  أكد( p->xTask!=0 );
  p->pResult = p->xTask(p->pIn);

  _endthreadex(0);
  بازگشت 0; /* NOT REACHED */
}

/* Create a new thread */
صحیح sqlite3ThreadCreate(
  SQLiteThread **ppThread,  /* OUT: Write the thread object here */
  تهی *(*xTask)(تهی*),    /* Routine to run in a separate thread */
  تهی *pIn                 /* Argument passed into xTask() */
){
  SQLiteThread *p;

  أكد( ppThread!=0 );
  أكد( xTask!=0 );
  *ppThread = 0;
  p = sqlite3Malloc(اندازه(*p));
  اگر( p==0 ) بازگشت SQLITE_NOMEM_BKPT;
  /* If the SQLITE_TESTCTRL_FAULT_INSTALL callback is registered to a 
  ** function that returns SQLITE_ERROR when passed the argument 200, that
  ** forces worker threads to run sequentially and deterministically 
  ** (via the sqlite3FaultSim() term of the conditional) for testing
  ** purposes. */
  اگر( sqlite3GlobalConfig.bCoreMutex==0 || sqlite3FaultSim(200) ){
    memset(p, 0, اندازه(*p));
  }وگرنه{
    p->xTask = xTask;
    p->pIn = pIn;
    p->tid = (تهی*)_beginthreadex(0, 0, sqlite3ThreadProc, p, 0, &p->id);
    اگر( p->tid==0 ){
      memset(p, 0, اندازه(*p));
    }
  }
  اگر( p->xTask==0 ){
    p->id = GetCurrentThreadId();
    p->pResult = xTask(pIn);
  }
  *ppThread = p;
  بازگشت SQLITE_OK;
}

DWORD sqlite3Win32Wait(HANDLE hObject); /* os_win.c */

/* Get the results of the thread */
صحیح sqlite3ThreadJoin(SQLiteThread *p, تهی **ppOut){
  DWORD rc;
  BOOL bRc;

  أكد( ppOut!=0 );
  اگر( NEVER(p==0) ) بازگشت SQLITE_NOMEM_BKPT;
  اگر( p->xTask==0 ){
    /* assert( p->id==GetCurrentThreadId() ); */
    rc = WAIT_OBJECT_0;
    أكد( p->tid==0 );
  }وگرنه{
    أكد( p->id!=0 && p->id!=GetCurrentThreadId() );
    rc = sqlite3Win32Wait((HANDLE)p->tid);
    أكد( rc!=WAIT_IO_COMPLETION );
    bRc = CloseHandle((HANDLE)p->tid);
    (تهی)bRc;  /* Prevent warning when assert() is a no-op */
    أكد( bRc );
  }
  اگر( rc==WAIT_OBJECT_0 ) *ppOut = p->pResult;
  sqlite3_free(p);
  بازگشت (rc==WAIT_OBJECT_0) ? SQLITE_OK : SQLITE_ERROR;
}

#endif /* SQLITE_OS_WIN_THREADS */
/******************************** End Win32 Threads *************************/


/********************************* Single-Threaded **************************/
#ifndef SQLITE_THREADS_IMPLEMENTED
/*
** This implementation does not actually create a new thread.  It does the
** work of the thread in the main thread, when either the thread is created
** or when it is joined
*/

/* A running thread */
ساختار SQLiteThread {
  تهی *(*xTask)(تهی*);   /* The routine to run as a thread */
  تهی *pIn;               /* Argument to xTask */
  تهی *pResult;           /* Result of xTask */
};

/* Create a new thread */
صحیح sqlite3ThreadCreate(
  SQLiteThread **ppThread,  /* OUT: Write the thread object here */
  تهی *(*xTask)(تهی*),    /* Routine to run in a separate thread */
  تهی *pIn                 /* Argument passed into xTask() */
){
  SQLiteThread *p;

  أكد( ppThread!=0 );
  أكد( xTask!=0 );
  *ppThread = 0;
  p = sqlite3Malloc(اندازه(*p));
  اگر( p==0 ) بازگشت SQLITE_NOMEM_BKPT;
  اگر( (SQLITE_PTR_TO_INT(p)/17)&1 ){
    p->xTask = xTask;
    p->pIn = pIn;
  }وگرنه{
    p->xTask = 0;
    p->pResult = xTask(pIn);
  }
  *ppThread = p;
  بازگشت SQLITE_OK;
}

/* Get the results of the thread */
صحیح sqlite3ThreadJoin(SQLiteThread *p, تهی **ppOut){

  أكد( ppOut!=0 );
  اگر( NEVER(p==0) ) بازگشت SQLITE_NOMEM_BKPT;
  اگر( p->xTask ){
    *ppOut = p->xTask(p->pIn);
  }وگرنه{
    *ppOut = p->pResult;
  }
  sqlite3_free(p);

#اگر defined(SQLITE_TEST)
  {
    تهی *pTstAlloc = sqlite3Malloc(10);
    اگر (!pTstAlloc) بازگشت SQLITE_NOMEM_BKPT;
    sqlite3_free(pTstAlloc);
  }
#endif

  بازگشت SQLITE_OK;
}

#endif /* !defined(SQLITE_THREADS_IMPLEMENTED) */
/****************************** End Single-Threaded *************************/
#endif /* SQLITE_MAX_WORKER_THREADS>0 */
