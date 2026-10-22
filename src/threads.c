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
#অন্তর্ভাৱ "sqliteInt.h"
#যদি SQLITE_OS_WIN
#  include "os_win.h"
#endif

#যদি SQLITE_MAX_WORKER_THREADS>0

/********************************* Unix Pthreads ****************************/
#যদি SQLITE_OS_UNIX && defined(SQLITE_MUTEX_PTHREADS) && SQLITE_THREADSAFE>0

#define SQLITE_THREADS_IMPLEMENTED 1  /* Prevent the single-thread code below */
#অন্তর্ভাৱ <pthread.h>

/* A running thread */
সংরচনা SQLiteThread {
  pthread_t tid;                 /* Thread ID */
  পূর্ণাঙ্ক done;                      /* Set to true when thread finishes */
  শূন্য *pOut;                    /* Result returned by the thread */
  শূন্য *(*xTask)(শূন্য*);         /* The thread routine */
  শূন্য *pIn;                     /* Argument to the thread */
};

/* Create a new thread */
পূর্ণাঙ্ক sqlite3ThreadCreate(
  SQLiteThread **ppThread,  /* OUT: Write the thread object here */
  শূন্য *(*xTask)(শূন্য*),    /* Routine to run in a separate thread */
  শূন্য *pIn                 /* Argument passed into xTask() */
){
  SQLiteThread *p;
  পূর্ণাঙ্ক rc;

  প্রতিজ্ঞা( ppThread!=0 );
  প্রতিজ্ঞা( xTask!=0 );
  /* This routine is never used in single-threaded mode */
  প্রতিজ্ঞা( sqlite3GlobalConfig.bCoreMutex!=0 );

  *ppThread = 0;
  p = sqlite3Malloc(পরিমাণ(*p));
  যদি( p==0 ) ঘূৰাই SQLITE_NOMEM_BKPT;
  memset(p, 0, পরিমাণ(*p));
  p->xTask = xTask;
  p->pIn = pIn;
  /* If the SQLITE_TESTCTRL_FAULT_INSTALL callback is registered to a 
  ** function that returns SQLITE_ERROR when passed the argument 200, that
  ** forces worker threads to run sequentially and deterministically 
  ** for testing purposes. */
  যদি( sqlite3FaultSim(200) ){
    rc = 1;
  }অন্যথা{    
    rc = pthread_create(&p->tid, 0, xTask, pIn);
  }
  যদি( rc ){
    p->done = 1;
    p->pOut = xTask(pIn);
  }
  *ppThread = p;
  ঘূৰাই SQLITE_OK;
}

/* Get the results of the thread */
পূর্ণাঙ্ক sqlite3ThreadJoin(SQLiteThread *p, শূন্য **ppOut){
  পূর্ণাঙ্ক rc;

  প্রতিজ্ঞা( ppOut!=0 );
  যদি( NEVER(p==0) ) ঘূৰাই SQLITE_NOMEM_BKPT;
  যদি( p->done ){
    *ppOut = p->pOut;
    rc = SQLITE_OK;
  }অন্যথা{
    rc = pthread_join(p->tid, ppOut) ? SQLITE_ERROR : SQLITE_OK;
  }
  sqlite3_free(p);
  ঘূৰাই rc;
}

#endif /* SQLITE_OS_UNIX && defined(SQLITE_MUTEX_PTHREADS) */
/******************************** End Unix Pthreads *************************/


/********************************* Win32 Threads ****************************/
#যদি SQLITE_OS_WIN_THREADS

#define SQLITE_THREADS_IMPLEMENTED 1  /* Prevent the single-thread code below */
#অন্তর্ভাৱ <process.h>

/* A running thread */
সংরচনা SQLiteThread {
  শূন্য *tid;               /* The thread handle */
  unsigned id;             /* The thread identifier */
  শূন্য *(*xTask)(শূন্য*);   /* The routine to run as a thread */
  শূন্য *pIn;               /* Argument to xTask */
  শূন্য *pResult;           /* Result of xTask */
};

/* Thread procedure Win32 compatibility shim */
স্থির unsigned __stdcall sqlite3ThreadProc(
  শূন্য *pArg  /* IN: Pointer to the SQLiteThread structure */
){
  SQLiteThread *p = (SQLiteThread *)pArg;

  প্রতিজ্ঞা( p!=0 );
#যদি 0
  /*
  ** This assert appears to trigger spuriously on certain
  ** versions of Windows, possibly due to _beginthreadex()
  ** and/or CreateThread() not fully setting their thread
  ** ID parameter before starting the thread.
  */
  প্রতিজ্ঞা( p->id==GetCurrentThreadId() );
#endif
  প্রতিজ্ঞা( p->xTask!=0 );
  p->pResult = p->xTask(p->pIn);

  _endthreadex(0);
  ঘূৰাই 0; /* NOT REACHED */
}

/* Create a new thread */
পূর্ণাঙ্ক sqlite3ThreadCreate(
  SQLiteThread **ppThread,  /* OUT: Write the thread object here */
  শূন্য *(*xTask)(শূন্য*),    /* Routine to run in a separate thread */
  শূন্য *pIn                 /* Argument passed into xTask() */
){
  SQLiteThread *p;

  প্রতিজ্ঞা( ppThread!=0 );
  প্রতিজ্ঞা( xTask!=0 );
  *ppThread = 0;
  p = sqlite3Malloc(পরিমাণ(*p));
  যদি( p==0 ) ঘূৰাই SQLITE_NOMEM_BKPT;
  /* If the SQLITE_TESTCTRL_FAULT_INSTALL callback is registered to a 
  ** function that returns SQLITE_ERROR when passed the argument 200, that
  ** forces worker threads to run sequentially and deterministically 
  ** (via the sqlite3FaultSim() term of the conditional) for testing
  ** purposes. */
  যদি( sqlite3GlobalConfig.bCoreMutex==0 || sqlite3FaultSim(200) ){
    memset(p, 0, পরিমাণ(*p));
  }অন্যথা{
    p->xTask = xTask;
    p->pIn = pIn;
    p->tid = (শূন্য*)_beginthreadex(0, 0, sqlite3ThreadProc, p, 0, &p->id);
    যদি( p->tid==0 ){
      memset(p, 0, পরিমাণ(*p));
    }
  }
  যদি( p->xTask==0 ){
    p->id = GetCurrentThreadId();
    p->pResult = xTask(pIn);
  }
  *ppThread = p;
  ঘূৰাই SQLITE_OK;
}

DWORD sqlite3Win32Wait(HANDLE hObject); /* os_win.c */

/* Get the results of the thread */
পূর্ণাঙ্ক sqlite3ThreadJoin(SQLiteThread *p, শূন্য **ppOut){
  DWORD rc;
  BOOL bRc;

  প্রতিজ্ঞা( ppOut!=0 );
  যদি( NEVER(p==0) ) ঘূৰাই SQLITE_NOMEM_BKPT;
  যদি( p->xTask==0 ){
    /* assert( p->id==GetCurrentThreadId() ); */
    rc = WAIT_OBJECT_0;
    প্রতিজ্ঞা( p->tid==0 );
  }অন্যথা{
    প্রতিজ্ঞা( p->id!=0 && p->id!=GetCurrentThreadId() );
    rc = sqlite3Win32Wait((HANDLE)p->tid);
    প্রতিজ্ঞা( rc!=WAIT_IO_COMPLETION );
    bRc = CloseHandle((HANDLE)p->tid);
    (শূন্য)bRc;  /* Prevent warning when assert() is a no-op */
    প্রতিজ্ঞা( bRc );
  }
  যদি( rc==WAIT_OBJECT_0 ) *ppOut = p->pResult;
  sqlite3_free(p);
  ঘূৰাই (rc==WAIT_OBJECT_0) ? SQLITE_OK : SQLITE_ERROR;
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
সংরচনা SQLiteThread {
  শূন্য *(*xTask)(শূন্য*);   /* The routine to run as a thread */
  শূন্য *pIn;               /* Argument to xTask */
  শূন্য *pResult;           /* Result of xTask */
};

/* Create a new thread */
পূর্ণাঙ্ক sqlite3ThreadCreate(
  SQLiteThread **ppThread,  /* OUT: Write the thread object here */
  শূন্য *(*xTask)(শূন্য*),    /* Routine to run in a separate thread */
  শূন্য *pIn                 /* Argument passed into xTask() */
){
  SQLiteThread *p;

  প্রতিজ্ঞা( ppThread!=0 );
  প্রতিজ্ঞা( xTask!=0 );
  *ppThread = 0;
  p = sqlite3Malloc(পরিমাণ(*p));
  যদি( p==0 ) ঘূৰাই SQLITE_NOMEM_BKPT;
  যদি( (SQLITE_PTR_TO_INT(p)/17)&1 ){
    p->xTask = xTask;
    p->pIn = pIn;
  }অন্যথা{
    p->xTask = 0;
    p->pResult = xTask(pIn);
  }
  *ppThread = p;
  ঘূৰাই SQLITE_OK;
}

/* Get the results of the thread */
পূর্ণাঙ্ক sqlite3ThreadJoin(SQLiteThread *p, শূন্য **ppOut){

  প্রতিজ্ঞা( ppOut!=0 );
  যদি( NEVER(p==0) ) ঘূৰাই SQLITE_NOMEM_BKPT;
  যদি( p->xTask ){
    *ppOut = p->xTask(p->pIn);
  }অন্যথা{
    *ppOut = p->pResult;
  }
  sqlite3_free(p);

#যদি defined(SQLITE_TEST)
  {
    শূন্য *pTstAlloc = sqlite3Malloc(10);
    যদি (!pTstAlloc) ঘূৰাই SQLITE_NOMEM_BKPT;
    sqlite3_free(pTstAlloc);
  }
#endif

  ঘূৰাই SQLITE_OK;
}

#endif /* !defined(SQLITE_THREADS_IMPLEMENTED) */
/****************************** End Single-Threaded *************************/
#endif /* SQLITE_MAX_WORKER_THREADS>0 */
