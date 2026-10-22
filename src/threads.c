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
#அந்தர்பாவ "sqliteInt.h"
#எனில் SQLITE_OS_WIN
#  include "os_win.h"
#endif

#எனில் SQLITE_MAX_WORKER_THREADS>0

/********************************* Unix Pthreads ****************************/
#எனில் SQLITE_OS_UNIX && defined(SQLITE_MUTEX_PTHREADS) && SQLITE_THREADSAFE>0

#define SQLITE_THREADS_IMPLEMENTED 1  /* Prevent the single-thread code below */
#அந்தர்பாவ <pthread.h>

/* A running thread */
கட்டமைப்பு SQLiteThread {
  pthread_t tid;                 /* Thread ID */
  முழு done;                      /* Set to true when thread finishes */
  வெறுமை *pOut;                    /* Result returned by the thread */
  வெறுமை *(*xTask)(வெறுமை*);         /* The thread routine */
  வெறுமை *pIn;                     /* Argument to the thread */
};

/* Create a new thread */
முழு sqlite3ThreadCreate(
  SQLiteThread **ppThread,  /* OUT: Write the thread object here */
  வெறுமை *(*xTask)(வெறுமை*),    /* Routine to run in a separate thread */
  வெறுமை *pIn                 /* Argument passed into xTask() */
){
  SQLiteThread *p;
  முழு rc;

  ப்ரதிஜ்ஞா( ppThread!=0 );
  ப்ரதிஜ்ஞா( xTask!=0 );
  /* This routine is never used in single-threaded mode */
  ப்ரதிஜ்ஞா( sqlite3GlobalConfig.bCoreMutex!=0 );

  *ppThread = 0;
  p = sqlite3Malloc(அளவு(*p));
  எனில்( p==0 ) திரும்பு SQLITE_NOMEM_BKPT;
  memset(p, 0, அளவு(*p));
  p->xTask = xTask;
  p->pIn = pIn;
  /* If the SQLITE_TESTCTRL_FAULT_INSTALL callback is registered to a 
  ** function that returns SQLITE_ERROR when passed the argument 200, that
  ** forces worker threads to run sequentially and deterministically 
  ** for testing purposes. */
  எனில்( sqlite3FaultSim(200) ){
    rc = 1;
  }இல்லையெனில்{    
    rc = pthread_create(&p->tid, 0, xTask, pIn);
  }
  எனில்( rc ){
    p->done = 1;
    p->pOut = xTask(pIn);
  }
  *ppThread = p;
  திரும்பு SQLITE_OK;
}

/* Get the results of the thread */
முழு sqlite3ThreadJoin(SQLiteThread *p, வெறுமை **ppOut){
  முழு rc;

  ப்ரதிஜ்ஞா( ppOut!=0 );
  எனில்( NEVER(p==0) ) திரும்பு SQLITE_NOMEM_BKPT;
  எனில்( p->done ){
    *ppOut = p->pOut;
    rc = SQLITE_OK;
  }இல்லையெனில்{
    rc = pthread_join(p->tid, ppOut) ? SQLITE_ERROR : SQLITE_OK;
  }
  sqlite3_free(p);
  திரும்பு rc;
}

#endif /* SQLITE_OS_UNIX && defined(SQLITE_MUTEX_PTHREADS) */
/******************************** End Unix Pthreads *************************/


/********************************* Win32 Threads ****************************/
#எனில் SQLITE_OS_WIN_THREADS

#define SQLITE_THREADS_IMPLEMENTED 1  /* Prevent the single-thread code below */
#அந்தர்பாவ <process.h>

/* A running thread */
கட்டமைப்பு SQLiteThread {
  வெறுமை *tid;               /* The thread handle */
  unsigned id;             /* The thread identifier */
  வெறுமை *(*xTask)(வெறுமை*);   /* The routine to run as a thread */
  வெறுமை *pIn;               /* Argument to xTask */
  வெறுமை *pResult;           /* Result of xTask */
};

/* Thread procedure Win32 compatibility shim */
நிலையான unsigned __stdcall sqlite3ThreadProc(
  வெறுமை *pArg  /* IN: Pointer to the SQLiteThread structure */
){
  SQLiteThread *p = (SQLiteThread *)pArg;

  ப்ரதிஜ்ஞா( p!=0 );
#எனில் 0
  /*
  ** This assert appears to trigger spuriously on certain
  ** versions of Windows, possibly due to _beginthreadex()
  ** and/or CreateThread() not fully setting their thread
  ** ID parameter before starting the thread.
  */
  ப்ரதிஜ்ஞா( p->id==GetCurrentThreadId() );
#endif
  ப்ரதிஜ்ஞா( p->xTask!=0 );
  p->pResult = p->xTask(p->pIn);

  _endthreadex(0);
  திரும்பு 0; /* NOT REACHED */
}

/* Create a new thread */
முழு sqlite3ThreadCreate(
  SQLiteThread **ppThread,  /* OUT: Write the thread object here */
  வெறுமை *(*xTask)(வெறுமை*),    /* Routine to run in a separate thread */
  வெறுமை *pIn                 /* Argument passed into xTask() */
){
  SQLiteThread *p;

  ப்ரதிஜ்ஞா( ppThread!=0 );
  ப்ரதிஜ்ஞா( xTask!=0 );
  *ppThread = 0;
  p = sqlite3Malloc(அளவு(*p));
  எனில்( p==0 ) திரும்பு SQLITE_NOMEM_BKPT;
  /* If the SQLITE_TESTCTRL_FAULT_INSTALL callback is registered to a 
  ** function that returns SQLITE_ERROR when passed the argument 200, that
  ** forces worker threads to run sequentially and deterministically 
  ** (via the sqlite3FaultSim() term of the conditional) for testing
  ** purposes. */
  எனில்( sqlite3GlobalConfig.bCoreMutex==0 || sqlite3FaultSim(200) ){
    memset(p, 0, அளவு(*p));
  }இல்லையெனில்{
    p->xTask = xTask;
    p->pIn = pIn;
    p->tid = (வெறுமை*)_beginthreadex(0, 0, sqlite3ThreadProc, p, 0, &p->id);
    எனில்( p->tid==0 ){
      memset(p, 0, அளவு(*p));
    }
  }
  எனில்( p->xTask==0 ){
    p->id = GetCurrentThreadId();
    p->pResult = xTask(pIn);
  }
  *ppThread = p;
  திரும்பு SQLITE_OK;
}

DWORD sqlite3Win32Wait(HANDLE hObject); /* os_win.c */

/* Get the results of the thread */
முழு sqlite3ThreadJoin(SQLiteThread *p, வெறுமை **ppOut){
  DWORD rc;
  BOOL bRc;

  ப்ரதிஜ்ஞா( ppOut!=0 );
  எனில்( NEVER(p==0) ) திரும்பு SQLITE_NOMEM_BKPT;
  எனில்( p->xTask==0 ){
    /* assert( p->id==GetCurrentThreadId() ); */
    rc = WAIT_OBJECT_0;
    ப்ரதிஜ்ஞா( p->tid==0 );
  }இல்லையெனில்{
    ப்ரதிஜ்ஞா( p->id!=0 && p->id!=GetCurrentThreadId() );
    rc = sqlite3Win32Wait((HANDLE)p->tid);
    ப்ரதிஜ்ஞா( rc!=WAIT_IO_COMPLETION );
    bRc = CloseHandle((HANDLE)p->tid);
    (வெறுமை)bRc;  /* Prevent warning when assert() is a no-op */
    ப்ரதிஜ்ஞா( bRc );
  }
  எனில்( rc==WAIT_OBJECT_0 ) *ppOut = p->pResult;
  sqlite3_free(p);
  திரும்பு (rc==WAIT_OBJECT_0) ? SQLITE_OK : SQLITE_ERROR;
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
கட்டமைப்பு SQLiteThread {
  வெறுமை *(*xTask)(வெறுமை*);   /* The routine to run as a thread */
  வெறுமை *pIn;               /* Argument to xTask */
  வெறுமை *pResult;           /* Result of xTask */
};

/* Create a new thread */
முழு sqlite3ThreadCreate(
  SQLiteThread **ppThread,  /* OUT: Write the thread object here */
  வெறுமை *(*xTask)(வெறுமை*),    /* Routine to run in a separate thread */
  வெறுமை *pIn                 /* Argument passed into xTask() */
){
  SQLiteThread *p;

  ப்ரதிஜ்ஞா( ppThread!=0 );
  ப்ரதிஜ்ஞா( xTask!=0 );
  *ppThread = 0;
  p = sqlite3Malloc(அளவு(*p));
  எனில்( p==0 ) திரும்பு SQLITE_NOMEM_BKPT;
  எனில்( (SQLITE_PTR_TO_INT(p)/17)&1 ){
    p->xTask = xTask;
    p->pIn = pIn;
  }இல்லையெனில்{
    p->xTask = 0;
    p->pResult = xTask(pIn);
  }
  *ppThread = p;
  திரும்பு SQLITE_OK;
}

/* Get the results of the thread */
முழு sqlite3ThreadJoin(SQLiteThread *p, வெறுமை **ppOut){

  ப்ரதிஜ்ஞா( ppOut!=0 );
  எனில்( NEVER(p==0) ) திரும்பு SQLITE_NOMEM_BKPT;
  எனில்( p->xTask ){
    *ppOut = p->xTask(p->pIn);
  }இல்லையெனில்{
    *ppOut = p->pResult;
  }
  sqlite3_free(p);

#எனில் defined(SQLITE_TEST)
  {
    வெறுமை *pTstAlloc = sqlite3Malloc(10);
    எனில் (!pTstAlloc) திரும்பு SQLITE_NOMEM_BKPT;
    sqlite3_free(pTstAlloc);
  }
#endif

  திரும்பு SQLITE_OK;
}

#endif /* !defined(SQLITE_THREADS_IMPLEMENTED) */
/****************************** End Single-Threaded *************************/
#endif /* SQLITE_MAX_WORKER_THREADS>0 */
