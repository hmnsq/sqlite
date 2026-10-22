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
#ଅନ୍ତର୍ଭାଵ "sqliteInt.h"
#ଯଦି SQLITE_OS_WIN
#  include "os_win.h"
#endif

#ଯଦି SQLITE_MAX_WORKER_THREADS>0

/********************************* Unix Pthreads ****************************/
#ଯଦି SQLITE_OS_UNIX && defined(SQLITE_MUTEX_PTHREADS) && SQLITE_THREADSAFE>0

#define SQLITE_THREADS_IMPLEMENTED 1  /* Prevent the single-thread code below */
#ଅନ୍ତର୍ଭାଵ <pthread.h>

/* A running thread */
ସଂରଚନା SQLiteThread {
  pthread_t tid;                 /* Thread ID */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା done;                      /* Set to true when thread finishes */
  ଶୂନ୍ୟ *pOut;                    /* Result returned by the thread */
  ଶୂନ୍ୟ *(*xTask)(ଶୂନ୍ୟ*);         /* The thread routine */
  ଶୂନ୍ୟ *pIn;                     /* Argument to the thread */
};

/* Create a new thread */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3ThreadCreate(
  SQLiteThread **ppThread,  /* OUT: Write the thread object here */
  ଶୂନ୍ୟ *(*xTask)(ଶୂନ୍ୟ*),    /* Routine to run in a separate thread */
  ଶୂନ୍ୟ *pIn                 /* Argument passed into xTask() */
){
  SQLiteThread *p;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

  ପ୍ରତିଜ୍ଞା( ppThread!=0 );
  ପ୍ରତିଜ୍ଞା( xTask!=0 );
  /* This routine is never used in single-threaded mode */
  ପ୍ରତିଜ୍ଞା( sqlite3GlobalConfig.bCoreMutex!=0 );

  *ppThread = 0;
  p = sqlite3Malloc(ଆକାର(*p));
  ଯଦି( p==0 ) ଫେରନ୍ତୁ SQLITE_NOMEM_BKPT;
  memset(p, 0, ଆକାର(*p));
  p->xTask = xTask;
  p->pIn = pIn;
  /* If the SQLITE_TESTCTRL_FAULT_INSTALL callback is registered to a 
  ** function that returns SQLITE_ERROR when passed the argument 200, that
  ** forces worker threads to run sequentially and deterministically 
  ** for testing purposes. */
  ଯଦି( sqlite3FaultSim(200) ){
    rc = 1;
  }ଅନ୍ୟଥା{    
    rc = pthread_create(&p->tid, 0, xTask, pIn);
  }
  ଯଦି( rc ){
    p->done = 1;
    p->pOut = xTask(pIn);
  }
  *ppThread = p;
  ଫେରନ୍ତୁ SQLITE_OK;
}

/* Get the results of the thread */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3ThreadJoin(SQLiteThread *p, ଶୂନ୍ୟ **ppOut){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

  ପ୍ରତିଜ୍ଞା( ppOut!=0 );
  ଯଦି( NEVER(p==0) ) ଫେରନ୍ତୁ SQLITE_NOMEM_BKPT;
  ଯଦି( p->done ){
    *ppOut = p->pOut;
    rc = SQLITE_OK;
  }ଅନ୍ୟଥା{
    rc = pthread_join(p->tid, ppOut) ? SQLITE_ERROR : SQLITE_OK;
  }
  sqlite3_free(p);
  ଫେରନ୍ତୁ rc;
}

#endif /* SQLITE_OS_UNIX && defined(SQLITE_MUTEX_PTHREADS) */
/******************************** End Unix Pthreads *************************/


/********************************* Win32 Threads ****************************/
#ଯଦି SQLITE_OS_WIN_THREADS

#define SQLITE_THREADS_IMPLEMENTED 1  /* Prevent the single-thread code below */
#ଅନ୍ତର୍ଭାଵ <process.h>

/* A running thread */
ସଂରଚନା SQLiteThread {
  ଶୂନ୍ୟ *tid;               /* The thread handle */
  unsigned id;             /* The thread identifier */
  ଶୂନ୍ୟ *(*xTask)(ଶୂନ୍ୟ*);   /* The routine to run as a thread */
  ଶୂନ୍ୟ *pIn;               /* Argument to xTask */
  ଶୂନ୍ୟ *pResult;           /* Result of xTask */
};

/* Thread procedure Win32 compatibility shim */
ସ୍ଥିର unsigned __stdcall sqlite3ThreadProc(
  ଶୂନ୍ୟ *pArg  /* IN: Pointer to the SQLiteThread structure */
){
  SQLiteThread *p = (SQLiteThread *)pArg;

  ପ୍ରତିଜ୍ଞା( p!=0 );
#ଯଦି 0
  /*
  ** This assert appears to trigger spuriously on certain
  ** versions of Windows, possibly due to _beginthreadex()
  ** and/or CreateThread() not fully setting their thread
  ** ID parameter before starting the thread.
  */
  ପ୍ରତିଜ୍ଞା( p->id==GetCurrentThreadId() );
#endif
  ପ୍ରତିଜ୍ଞା( p->xTask!=0 );
  p->pResult = p->xTask(p->pIn);

  _endthreadex(0);
  ଫେରନ୍ତୁ 0; /* NOT REACHED */
}

/* Create a new thread */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3ThreadCreate(
  SQLiteThread **ppThread,  /* OUT: Write the thread object here */
  ଶୂନ୍ୟ *(*xTask)(ଶୂନ୍ୟ*),    /* Routine to run in a separate thread */
  ଶୂନ୍ୟ *pIn                 /* Argument passed into xTask() */
){
  SQLiteThread *p;

  ପ୍ରତିଜ୍ଞା( ppThread!=0 );
  ପ୍ରତିଜ୍ଞା( xTask!=0 );
  *ppThread = 0;
  p = sqlite3Malloc(ଆକାର(*p));
  ଯଦି( p==0 ) ଫେରନ୍ତୁ SQLITE_NOMEM_BKPT;
  /* If the SQLITE_TESTCTRL_FAULT_INSTALL callback is registered to a 
  ** function that returns SQLITE_ERROR when passed the argument 200, that
  ** forces worker threads to run sequentially and deterministically 
  ** (via the sqlite3FaultSim() term of the conditional) for testing
  ** purposes. */
  ଯଦି( sqlite3GlobalConfig.bCoreMutex==0 || sqlite3FaultSim(200) ){
    memset(p, 0, ଆକାର(*p));
  }ଅନ୍ୟଥା{
    p->xTask = xTask;
    p->pIn = pIn;
    p->tid = (ଶୂନ୍ୟ*)_beginthreadex(0, 0, sqlite3ThreadProc, p, 0, &p->id);
    ଯଦି( p->tid==0 ){
      memset(p, 0, ଆକାର(*p));
    }
  }
  ଯଦି( p->xTask==0 ){
    p->id = GetCurrentThreadId();
    p->pResult = xTask(pIn);
  }
  *ppThread = p;
  ଫେରନ୍ତୁ SQLITE_OK;
}

DWORD sqlite3Win32Wait(HANDLE hObject); /* os_win.c */

/* Get the results of the thread */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3ThreadJoin(SQLiteThread *p, ଶୂନ୍ୟ **ppOut){
  DWORD rc;
  BOOL bRc;

  ପ୍ରତିଜ୍ଞା( ppOut!=0 );
  ଯଦି( NEVER(p==0) ) ଫେରନ୍ତୁ SQLITE_NOMEM_BKPT;
  ଯଦି( p->xTask==0 ){
    /* assert( p->id==GetCurrentThreadId() ); */
    rc = WAIT_OBJECT_0;
    ପ୍ରତିଜ୍ଞା( p->tid==0 );
  }ଅନ୍ୟଥା{
    ପ୍ରତିଜ୍ଞା( p->id!=0 && p->id!=GetCurrentThreadId() );
    rc = sqlite3Win32Wait((HANDLE)p->tid);
    ପ୍ରତିଜ୍ଞା( rc!=WAIT_IO_COMPLETION );
    bRc = CloseHandle((HANDLE)p->tid);
    (ଶୂନ୍ୟ)bRc;  /* Prevent warning when assert() is a no-op */
    ପ୍ରତିଜ୍ଞା( bRc );
  }
  ଯଦି( rc==WAIT_OBJECT_0 ) *ppOut = p->pResult;
  sqlite3_free(p);
  ଫେରନ୍ତୁ (rc==WAIT_OBJECT_0) ? SQLITE_OK : SQLITE_ERROR;
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
ସଂରଚନା SQLiteThread {
  ଶୂନ୍ୟ *(*xTask)(ଶୂନ୍ୟ*);   /* The routine to run as a thread */
  ଶୂନ୍ୟ *pIn;               /* Argument to xTask */
  ଶୂନ୍ୟ *pResult;           /* Result of xTask */
};

/* Create a new thread */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3ThreadCreate(
  SQLiteThread **ppThread,  /* OUT: Write the thread object here */
  ଶୂନ୍ୟ *(*xTask)(ଶୂନ୍ୟ*),    /* Routine to run in a separate thread */
  ଶୂନ୍ୟ *pIn                 /* Argument passed into xTask() */
){
  SQLiteThread *p;

  ପ୍ରତିଜ୍ଞା( ppThread!=0 );
  ପ୍ରତିଜ୍ଞା( xTask!=0 );
  *ppThread = 0;
  p = sqlite3Malloc(ଆକାର(*p));
  ଯଦି( p==0 ) ଫେରନ୍ତୁ SQLITE_NOMEM_BKPT;
  ଯଦି( (SQLITE_PTR_TO_INT(p)/17)&1 ){
    p->xTask = xTask;
    p->pIn = pIn;
  }ଅନ୍ୟଥା{
    p->xTask = 0;
    p->pResult = xTask(pIn);
  }
  *ppThread = p;
  ଫେରନ୍ତୁ SQLITE_OK;
}

/* Get the results of the thread */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3ThreadJoin(SQLiteThread *p, ଶୂନ୍ୟ **ppOut){

  ପ୍ରତିଜ୍ଞା( ppOut!=0 );
  ଯଦି( NEVER(p==0) ) ଫେରନ୍ତୁ SQLITE_NOMEM_BKPT;
  ଯଦି( p->xTask ){
    *ppOut = p->xTask(p->pIn);
  }ଅନ୍ୟଥା{
    *ppOut = p->pResult;
  }
  sqlite3_free(p);

#ଯଦି defined(SQLITE_TEST)
  {
    ଶୂନ୍ୟ *pTstAlloc = sqlite3Malloc(10);
    ଯଦି (!pTstAlloc) ଫେରନ୍ତୁ SQLITE_NOMEM_BKPT;
    sqlite3_free(pTstAlloc);
  }
#endif

  ଫେରନ୍ତୁ SQLITE_OK;
}

#endif /* !defined(SQLITE_THREADS_IMPLEMENTED) */
/****************************** End Single-Threaded *************************/
#endif /* SQLITE_MAX_WORKER_THREADS>0 */
