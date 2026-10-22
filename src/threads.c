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
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"
#ಯದಿ SQLITE_OS_WIN
#  include "os_win.h"
#endif

#ಯದಿ SQLITE_MAX_WORKER_THREADS>0

/********************************* Unix Pthreads ****************************/
#ಯದಿ SQLITE_OS_UNIX && defined(SQLITE_MUTEX_PTHREADS) && SQLITE_THREADSAFE>0

#define SQLITE_THREADS_IMPLEMENTED 1  /* Prevent the single-thread code below */
#ಅನ್ತರ್ಭಾವ <pthread.h>

/* A running thread */
ಸಂರಚನಾ SQLiteThread {
  pthread_t tid;                 /* Thread ID */
  ಪೂರ್ಣಾಙ್ಕ done;                      /* Set to true when thread finishes */
  ಶೂನ್ಯ *pOut;                    /* Result returned by the thread */
  ಶೂನ್ಯ *(*xTask)(ಶೂನ್ಯ*);         /* The thread routine */
  ಶೂನ್ಯ *pIn;                     /* Argument to the thread */
};

/* Create a new thread */
ಪೂರ್ಣಾಙ್ಕ sqlite3ThreadCreate(
  SQLiteThread **ppThread,  /* OUT: Write the thread object here */
  ಶೂನ್ಯ *(*xTask)(ಶೂನ್ಯ*),    /* Routine to run in a separate thread */
  ಶೂನ್ಯ *pIn                 /* Argument passed into xTask() */
){
  SQLiteThread *p;
  ಪೂರ್ಣಾಙ್ಕ rc;

  ಪ್ರತಿಜ್ಞಾ( ppThread!=0 );
  ಪ್ರತಿಜ್ಞಾ( xTask!=0 );
  /* This routine is never used in single-threaded mode */
  ಪ್ರತಿಜ್ಞಾ( sqlite3GlobalConfig.bCoreMutex!=0 );

  *ppThread = 0;
  p = sqlite3Malloc(ಪರಿಮಾಣ(*p));
  ಯದಿ( p==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM_BKPT;
  memset(p, 0, ಪರಿಮಾಣ(*p));
  p->xTask = xTask;
  p->pIn = pIn;
  /* If the SQLITE_TESTCTRL_FAULT_INSTALL callback is registered to a 
  ** function that returns SQLITE_ERROR when passed the argument 200, that
  ** forces worker threads to run sequentially and deterministically 
  ** for testing purposes. */
  ಯದಿ( sqlite3FaultSim(200) ){
    rc = 1;
  }ಅನ್ಯಥಾ{    
    rc = pthread_create(&p->tid, 0, xTask, pIn);
  }
  ಯದಿ( rc ){
    p->done = 1;
    p->pOut = xTask(pIn);
  }
  *ppThread = p;
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/* Get the results of the thread */
ಪೂರ್ಣಾಙ್ಕ sqlite3ThreadJoin(SQLiteThread *p, ಶೂನ್ಯ **ppOut){
  ಪೂರ್ಣಾಙ್ಕ rc;

  ಪ್ರತಿಜ್ಞಾ( ppOut!=0 );
  ಯದಿ( NEVER(p==0) ) ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM_BKPT;
  ಯದಿ( p->done ){
    *ppOut = p->pOut;
    rc = SQLITE_OK;
  }ಅನ್ಯಥಾ{
    rc = pthread_join(p->tid, ppOut) ? SQLITE_ERROR : SQLITE_OK;
  }
  sqlite3_free(p);
  ಪ್ರತ್ಯಾಗಮ rc;
}

#endif /* SQLITE_OS_UNIX && defined(SQLITE_MUTEX_PTHREADS) */
/******************************** End Unix Pthreads *************************/


/********************************* Win32 Threads ****************************/
#ಯದಿ SQLITE_OS_WIN_THREADS

#define SQLITE_THREADS_IMPLEMENTED 1  /* Prevent the single-thread code below */
#ಅನ್ತರ್ಭಾವ <process.h>

/* A running thread */
ಸಂರಚನಾ SQLiteThread {
  ಶೂನ್ಯ *tid;               /* The thread handle */
  unsigned id;             /* The thread identifier */
  ಶೂನ್ಯ *(*xTask)(ಶೂನ್ಯ*);   /* The routine to run as a thread */
  ಶೂನ್ಯ *pIn;               /* Argument to xTask */
  ಶೂನ್ಯ *pResult;           /* Result of xTask */
};

/* Thread procedure Win32 compatibility shim */
ಸ್ಥಿರ unsigned __stdcall sqlite3ThreadProc(
  ಶೂನ್ಯ *pArg  /* IN: Pointer to the SQLiteThread structure */
){
  SQLiteThread *p = (SQLiteThread *)pArg;

  ಪ್ರತಿಜ್ಞಾ( p!=0 );
#ಯದಿ 0
  /*
  ** This assert appears to trigger spuriously on certain
  ** versions of Windows, possibly due to _beginthreadex()
  ** and/or CreateThread() not fully setting their thread
  ** ID parameter before starting the thread.
  */
  ಪ್ರತಿಜ್ಞಾ( p->id==GetCurrentThreadId() );
#endif
  ಪ್ರತಿಜ್ಞಾ( p->xTask!=0 );
  p->pResult = p->xTask(p->pIn);

  _endthreadex(0);
  ಪ್ರತ್ಯಾಗಮ 0; /* NOT REACHED */
}

/* Create a new thread */
ಪೂರ್ಣಾಙ್ಕ sqlite3ThreadCreate(
  SQLiteThread **ppThread,  /* OUT: Write the thread object here */
  ಶೂನ್ಯ *(*xTask)(ಶೂನ್ಯ*),    /* Routine to run in a separate thread */
  ಶೂನ್ಯ *pIn                 /* Argument passed into xTask() */
){
  SQLiteThread *p;

  ಪ್ರತಿಜ್ಞಾ( ppThread!=0 );
  ಪ್ರತಿಜ್ಞಾ( xTask!=0 );
  *ppThread = 0;
  p = sqlite3Malloc(ಪರಿಮಾಣ(*p));
  ಯದಿ( p==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM_BKPT;
  /* If the SQLITE_TESTCTRL_FAULT_INSTALL callback is registered to a 
  ** function that returns SQLITE_ERROR when passed the argument 200, that
  ** forces worker threads to run sequentially and deterministically 
  ** (via the sqlite3FaultSim() term of the conditional) for testing
  ** purposes. */
  ಯದಿ( sqlite3GlobalConfig.bCoreMutex==0 || sqlite3FaultSim(200) ){
    memset(p, 0, ಪರಿಮಾಣ(*p));
  }ಅನ್ಯಥಾ{
    p->xTask = xTask;
    p->pIn = pIn;
    p->tid = (ಶೂನ್ಯ*)_beginthreadex(0, 0, sqlite3ThreadProc, p, 0, &p->id);
    ಯದಿ( p->tid==0 ){
      memset(p, 0, ಪರಿಮಾಣ(*p));
    }
  }
  ಯದಿ( p->xTask==0 ){
    p->id = GetCurrentThreadId();
    p->pResult = xTask(pIn);
  }
  *ppThread = p;
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

DWORD sqlite3Win32Wait(HANDLE hObject); /* os_win.c */

/* Get the results of the thread */
ಪೂರ್ಣಾಙ್ಕ sqlite3ThreadJoin(SQLiteThread *p, ಶೂನ್ಯ **ppOut){
  DWORD rc;
  BOOL bRc;

  ಪ್ರತಿಜ್ಞಾ( ppOut!=0 );
  ಯದಿ( NEVER(p==0) ) ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM_BKPT;
  ಯದಿ( p->xTask==0 ){
    /* assert( p->id==GetCurrentThreadId() ); */
    rc = WAIT_OBJECT_0;
    ಪ್ರತಿಜ್ಞಾ( p->tid==0 );
  }ಅನ್ಯಥಾ{
    ಪ್ರತಿಜ್ಞಾ( p->id!=0 && p->id!=GetCurrentThreadId() );
    rc = sqlite3Win32Wait((HANDLE)p->tid);
    ಪ್ರತಿಜ್ಞಾ( rc!=WAIT_IO_COMPLETION );
    bRc = CloseHandle((HANDLE)p->tid);
    (ಶೂನ್ಯ)bRc;  /* Prevent warning when assert() is a no-op */
    ಪ್ರತಿಜ್ಞಾ( bRc );
  }
  ಯದಿ( rc==WAIT_OBJECT_0 ) *ppOut = p->pResult;
  sqlite3_free(p);
  ಪ್ರತ್ಯಾಗಮ (rc==WAIT_OBJECT_0) ? SQLITE_OK : SQLITE_ERROR;
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
ಸಂರಚನಾ SQLiteThread {
  ಶೂನ್ಯ *(*xTask)(ಶೂನ್ಯ*);   /* The routine to run as a thread */
  ಶೂನ್ಯ *pIn;               /* Argument to xTask */
  ಶೂನ್ಯ *pResult;           /* Result of xTask */
};

/* Create a new thread */
ಪೂರ್ಣಾಙ್ಕ sqlite3ThreadCreate(
  SQLiteThread **ppThread,  /* OUT: Write the thread object here */
  ಶೂನ್ಯ *(*xTask)(ಶೂನ್ಯ*),    /* Routine to run in a separate thread */
  ಶೂನ್ಯ *pIn                 /* Argument passed into xTask() */
){
  SQLiteThread *p;

  ಪ್ರತಿಜ್ಞಾ( ppThread!=0 );
  ಪ್ರತಿಜ್ಞಾ( xTask!=0 );
  *ppThread = 0;
  p = sqlite3Malloc(ಪರಿಮಾಣ(*p));
  ಯದಿ( p==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM_BKPT;
  ಯದಿ( (SQLITE_PTR_TO_INT(p)/17)&1 ){
    p->xTask = xTask;
    p->pIn = pIn;
  }ಅನ್ಯಥಾ{
    p->xTask = 0;
    p->pResult = xTask(pIn);
  }
  *ppThread = p;
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/* Get the results of the thread */
ಪೂರ್ಣಾಙ್ಕ sqlite3ThreadJoin(SQLiteThread *p, ಶೂನ್ಯ **ppOut){

  ಪ್ರತಿಜ್ಞಾ( ppOut!=0 );
  ಯದಿ( NEVER(p==0) ) ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM_BKPT;
  ಯದಿ( p->xTask ){
    *ppOut = p->xTask(p->pIn);
  }ಅನ್ಯಥಾ{
    *ppOut = p->pResult;
  }
  sqlite3_free(p);

#ಯದಿ defined(SQLITE_TEST)
  {
    ಶೂನ್ಯ *pTstAlloc = sqlite3Malloc(10);
    ಯದಿ (!pTstAlloc) ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM_BKPT;
    sqlite3_free(pTstAlloc);
  }
#endif

  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

#endif /* !defined(SQLITE_THREADS_IMPLEMENTED) */
/****************************** End Single-Threaded *************************/
#endif /* SQLITE_MAX_WORKER_THREADS>0 */
