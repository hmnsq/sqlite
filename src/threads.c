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
#ضمن "sqliteInt.h"
#إذا SQLITE_OS_WIN
#  include "os_win.h"
#endif

#إذا SQLITE_MAX_WORKER_THREADS>0

/********************************* Unix Pthreads ****************************/
#إذا SQLITE_OS_UNIX && defined(SQLITE_MUTEX_PTHREADS) && SQLITE_THREADSAFE>0

#define SQLITE_THREADS_IMPLEMENTED 1  /* Prevent the single-thread code below */
#ضمن <pthread.h>

/* A running thread */
بنية SQLiteThread {
  pthread_t tid;                 /* Thread ID */
  صحيح done;                      /* Set to true when thread finishes */
  فراغ *pOut;                    /* Result returned by the thread */
  فراغ *(*xTask)(فراغ*);         /* The thread routine */
  فراغ *pIn;                     /* Argument to the thread */
};

/* Create a new thread */
صحيح sqlite3ThreadCreate(
  SQLiteThread **ppThread,  /* OUT: Write the thread object here */
  فراغ *(*xTask)(فراغ*),    /* Routine to run in a separate thread */
  فراغ *pIn                 /* Argument passed into xTask() */
){
  SQLiteThread *p;
  صحيح rc;

  أكد( ppThread!=0 );
  أكد( xTask!=0 );
  /* This routine is never used in single-threaded mode */
  أكد( sqlite3GlobalConfig.bCoreMutex!=0 );

  *ppThread = 0;
  p = sqlite3Malloc(حجم(*p));
  إذا( p==0 ) أرجع SQLITE_NOMEM_BKPT;
  memset(p, 0, حجم(*p));
  p->xTask = xTask;
  p->pIn = pIn;
  /* If the SQLITE_TESTCTRL_FAULT_INSTALL callback is registered to a 
  ** function that returns SQLITE_ERROR when passed the argument 200, that
  ** forces worker threads to run sequentially and deterministically 
  ** for testing purposes. */
  إذا( sqlite3FaultSim(200) ){
    rc = 1;
  }وإلا{    
    rc = pthread_create(&p->tid, 0, xTask, pIn);
  }
  إذا( rc ){
    p->done = 1;
    p->pOut = xTask(pIn);
  }
  *ppThread = p;
  أرجع SQLITE_OK;
}

/* Get the results of the thread */
صحيح sqlite3ThreadJoin(SQLiteThread *p, فراغ **ppOut){
  صحيح rc;

  أكد( ppOut!=0 );
  إذا( NEVER(p==0) ) أرجع SQLITE_NOMEM_BKPT;
  إذا( p->done ){
    *ppOut = p->pOut;
    rc = SQLITE_OK;
  }وإلا{
    rc = pthread_join(p->tid, ppOut) ? SQLITE_ERROR : SQLITE_OK;
  }
  sqlite3_free(p);
  أرجع rc;
}

#endif /* SQLITE_OS_UNIX && defined(SQLITE_MUTEX_PTHREADS) */
/******************************** End Unix Pthreads *************************/


/********************************* Win32 Threads ****************************/
#إذا SQLITE_OS_WIN_THREADS

#define SQLITE_THREADS_IMPLEMENTED 1  /* Prevent the single-thread code below */
#ضمن <process.h>

/* A running thread */
بنية SQLiteThread {
  فراغ *tid;               /* The thread handle */
  unsigned id;             /* The thread identifier */
  فراغ *(*xTask)(فراغ*);   /* The routine to run as a thread */
  فراغ *pIn;               /* Argument to xTask */
  فراغ *pResult;           /* Result of xTask */
};

/* Thread procedure Win32 compatibility shim */
ساكن unsigned __stdcall sqlite3ThreadProc(
  فراغ *pArg  /* IN: Pointer to the SQLiteThread structure */
){
  SQLiteThread *p = (SQLiteThread *)pArg;

  أكد( p!=0 );
#إذا 0
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
  أرجع 0; /* NOT REACHED */
}

/* Create a new thread */
صحيح sqlite3ThreadCreate(
  SQLiteThread **ppThread,  /* OUT: Write the thread object here */
  فراغ *(*xTask)(فراغ*),    /* Routine to run in a separate thread */
  فراغ *pIn                 /* Argument passed into xTask() */
){
  SQLiteThread *p;

  أكد( ppThread!=0 );
  أكد( xTask!=0 );
  *ppThread = 0;
  p = sqlite3Malloc(حجم(*p));
  إذا( p==0 ) أرجع SQLITE_NOMEM_BKPT;
  /* If the SQLITE_TESTCTRL_FAULT_INSTALL callback is registered to a 
  ** function that returns SQLITE_ERROR when passed the argument 200, that
  ** forces worker threads to run sequentially and deterministically 
  ** (via the sqlite3FaultSim() term of the conditional) for testing
  ** purposes. */
  إذا( sqlite3GlobalConfig.bCoreMutex==0 || sqlite3FaultSim(200) ){
    memset(p, 0, حجم(*p));
  }وإلا{
    p->xTask = xTask;
    p->pIn = pIn;
    p->tid = (فراغ*)_beginthreadex(0, 0, sqlite3ThreadProc, p, 0, &p->id);
    إذا( p->tid==0 ){
      memset(p, 0, حجم(*p));
    }
  }
  إذا( p->xTask==0 ){
    p->id = GetCurrentThreadId();
    p->pResult = xTask(pIn);
  }
  *ppThread = p;
  أرجع SQLITE_OK;
}

DWORD sqlite3Win32Wait(HANDLE hObject); /* os_win.c */

/* Get the results of the thread */
صحيح sqlite3ThreadJoin(SQLiteThread *p, فراغ **ppOut){
  DWORD rc;
  BOOL bRc;

  أكد( ppOut!=0 );
  إذا( NEVER(p==0) ) أرجع SQLITE_NOMEM_BKPT;
  إذا( p->xTask==0 ){
    /* assert( p->id==GetCurrentThreadId() ); */
    rc = WAIT_OBJECT_0;
    أكد( p->tid==0 );
  }وإلا{
    أكد( p->id!=0 && p->id!=GetCurrentThreadId() );
    rc = sqlite3Win32Wait((HANDLE)p->tid);
    أكد( rc!=WAIT_IO_COMPLETION );
    bRc = CloseHandle((HANDLE)p->tid);
    (فراغ)bRc;  /* Prevent warning when assert() is a no-op */
    أكد( bRc );
  }
  إذا( rc==WAIT_OBJECT_0 ) *ppOut = p->pResult;
  sqlite3_free(p);
  أرجع (rc==WAIT_OBJECT_0) ? SQLITE_OK : SQLITE_ERROR;
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
بنية SQLiteThread {
  فراغ *(*xTask)(فراغ*);   /* The routine to run as a thread */
  فراغ *pIn;               /* Argument to xTask */
  فراغ *pResult;           /* Result of xTask */
};

/* Create a new thread */
صحيح sqlite3ThreadCreate(
  SQLiteThread **ppThread,  /* OUT: Write the thread object here */
  فراغ *(*xTask)(فراغ*),    /* Routine to run in a separate thread */
  فراغ *pIn                 /* Argument passed into xTask() */
){
  SQLiteThread *p;

  أكد( ppThread!=0 );
  أكد( xTask!=0 );
  *ppThread = 0;
  p = sqlite3Malloc(حجم(*p));
  إذا( p==0 ) أرجع SQLITE_NOMEM_BKPT;
  إذا( (SQLITE_PTR_TO_INT(p)/17)&1 ){
    p->xTask = xTask;
    p->pIn = pIn;
  }وإلا{
    p->xTask = 0;
    p->pResult = xTask(pIn);
  }
  *ppThread = p;
  أرجع SQLITE_OK;
}

/* Get the results of the thread */
صحيح sqlite3ThreadJoin(SQLiteThread *p, فراغ **ppOut){

  أكد( ppOut!=0 );
  إذا( NEVER(p==0) ) أرجع SQLITE_NOMEM_BKPT;
  إذا( p->xTask ){
    *ppOut = p->xTask(p->pIn);
  }وإلا{
    *ppOut = p->pResult;
  }
  sqlite3_free(p);

#إذا defined(SQLITE_TEST)
  {
    فراغ *pTstAlloc = sqlite3Malloc(10);
    إذا (!pTstAlloc) أرجع SQLITE_NOMEM_BKPT;
    sqlite3_free(pTstAlloc);
  }
#endif

  أرجع SQLITE_OK;
}

#endif /* !defined(SQLITE_THREADS_IMPLEMENTED) */
/****************************** End Single-Threaded *************************/
#endif /* SQLITE_MAX_WORKER_THREADS>0 */
