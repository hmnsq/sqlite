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
#അന്തര്ഭാവ "sqliteInt.h"
#എങ്കിൽ SQLITE_OS_WIN
#  include "os_win.h"
#endif

#എങ്കിൽ SQLITE_MAX_WORKER_THREADS>0

/********************************* Unix Pthreads ****************************/
#എങ്കിൽ SQLITE_OS_UNIX && defined(SQLITE_MUTEX_PTHREADS) && SQLITE_THREADSAFE>0

#define SQLITE_THREADS_IMPLEMENTED 1  /* Prevent the single-thread code below */
#അന്തര്ഭാവ <pthread.h>

/* A running thread */
ഘടന SQLiteThread {
  pthread_t tid;                 /* Thread ID */
  പൂർണ്ണസംഖ്യ done;                      /* Set to true when thread finishes */
  ശൂന്യം *pOut;                    /* Result returned by the thread */
  ശൂന്യം *(*xTask)(ശൂന്യം*);         /* The thread routine */
  ശൂന്യം *pIn;                     /* Argument to the thread */
};

/* Create a new thread */
പൂർണ്ണസംഖ്യ sqlite3ThreadCreate(
  SQLiteThread **ppThread,  /* OUT: Write the thread object here */
  ശൂന്യം *(*xTask)(ശൂന്യം*),    /* Routine to run in a separate thread */
  ശൂന്യം *pIn                 /* Argument passed into xTask() */
){
  SQLiteThread *p;
  പൂർണ്ണസംഖ്യ rc;

  പ്രതിജ്ഞാ( ppThread!=0 );
  പ്രതിജ്ഞാ( xTask!=0 );
  /* This routine is never used in single-threaded mode */
  പ്രതിജ്ഞാ( sqlite3GlobalConfig.bCoreMutex!=0 );

  *ppThread = 0;
  p = sqlite3Malloc(വലുപ്പം(*p));
  എങ്കിൽ( p==0 ) തിരികെ SQLITE_NOMEM_BKPT;
  memset(p, 0, വലുപ്പം(*p));
  p->xTask = xTask;
  p->pIn = pIn;
  /* If the SQLITE_TESTCTRL_FAULT_INSTALL callback is registered to a 
  ** function that returns SQLITE_ERROR when passed the argument 200, that
  ** forces worker threads to run sequentially and deterministically 
  ** for testing purposes. */
  എങ്കിൽ( sqlite3FaultSim(200) ){
    rc = 1;
  }അല്ലെങ്കിൽ{    
    rc = pthread_create(&p->tid, 0, xTask, pIn);
  }
  എങ്കിൽ( rc ){
    p->done = 1;
    p->pOut = xTask(pIn);
  }
  *ppThread = p;
  തിരികെ SQLITE_OK;
}

/* Get the results of the thread */
പൂർണ്ണസംഖ്യ sqlite3ThreadJoin(SQLiteThread *p, ശൂന്യം **ppOut){
  പൂർണ്ണസംഖ്യ rc;

  പ്രതിജ്ഞാ( ppOut!=0 );
  എങ്കിൽ( NEVER(p==0) ) തിരികെ SQLITE_NOMEM_BKPT;
  എങ്കിൽ( p->done ){
    *ppOut = p->pOut;
    rc = SQLITE_OK;
  }അല്ലെങ്കിൽ{
    rc = pthread_join(p->tid, ppOut) ? SQLITE_ERROR : SQLITE_OK;
  }
  sqlite3_free(p);
  തിരികെ rc;
}

#endif /* SQLITE_OS_UNIX && defined(SQLITE_MUTEX_PTHREADS) */
/******************************** End Unix Pthreads *************************/


/********************************* Win32 Threads ****************************/
#എങ്കിൽ SQLITE_OS_WIN_THREADS

#define SQLITE_THREADS_IMPLEMENTED 1  /* Prevent the single-thread code below */
#അന്തര്ഭാവ <process.h>

/* A running thread */
ഘടന SQLiteThread {
  ശൂന്യം *tid;               /* The thread handle */
  unsigned id;             /* The thread identifier */
  ശൂന്യം *(*xTask)(ശൂന്യം*);   /* The routine to run as a thread */
  ശൂന്യം *pIn;               /* Argument to xTask */
  ശൂന്യം *pResult;           /* Result of xTask */
};

/* Thread procedure Win32 compatibility shim */
സ്ഥിതം unsigned __stdcall sqlite3ThreadProc(
  ശൂന്യം *pArg  /* IN: Pointer to the SQLiteThread structure */
){
  SQLiteThread *p = (SQLiteThread *)pArg;

  പ്രതിജ്ഞാ( p!=0 );
#എങ്കിൽ 0
  /*
  ** This assert appears to trigger spuriously on certain
  ** versions of Windows, possibly due to _beginthreadex()
  ** and/or CreateThread() not fully setting their thread
  ** ID parameter before starting the thread.
  */
  പ്രതിജ്ഞാ( p->id==GetCurrentThreadId() );
#endif
  പ്രതിജ്ഞാ( p->xTask!=0 );
  p->pResult = p->xTask(p->pIn);

  _endthreadex(0);
  തിരികെ 0; /* NOT REACHED */
}

/* Create a new thread */
പൂർണ്ണസംഖ്യ sqlite3ThreadCreate(
  SQLiteThread **ppThread,  /* OUT: Write the thread object here */
  ശൂന്യം *(*xTask)(ശൂന്യം*),    /* Routine to run in a separate thread */
  ശൂന്യം *pIn                 /* Argument passed into xTask() */
){
  SQLiteThread *p;

  പ്രതിജ്ഞാ( ppThread!=0 );
  പ്രതിജ്ഞാ( xTask!=0 );
  *ppThread = 0;
  p = sqlite3Malloc(വലുപ്പം(*p));
  എങ്കിൽ( p==0 ) തിരികെ SQLITE_NOMEM_BKPT;
  /* If the SQLITE_TESTCTRL_FAULT_INSTALL callback is registered to a 
  ** function that returns SQLITE_ERROR when passed the argument 200, that
  ** forces worker threads to run sequentially and deterministically 
  ** (via the sqlite3FaultSim() term of the conditional) for testing
  ** purposes. */
  എങ്കിൽ( sqlite3GlobalConfig.bCoreMutex==0 || sqlite3FaultSim(200) ){
    memset(p, 0, വലുപ്പം(*p));
  }അല്ലെങ്കിൽ{
    p->xTask = xTask;
    p->pIn = pIn;
    p->tid = (ശൂന്യം*)_beginthreadex(0, 0, sqlite3ThreadProc, p, 0, &p->id);
    എങ്കിൽ( p->tid==0 ){
      memset(p, 0, വലുപ്പം(*p));
    }
  }
  എങ്കിൽ( p->xTask==0 ){
    p->id = GetCurrentThreadId();
    p->pResult = xTask(pIn);
  }
  *ppThread = p;
  തിരികെ SQLITE_OK;
}

DWORD sqlite3Win32Wait(HANDLE hObject); /* os_win.c */

/* Get the results of the thread */
പൂർണ്ണസംഖ്യ sqlite3ThreadJoin(SQLiteThread *p, ശൂന്യം **ppOut){
  DWORD rc;
  BOOL bRc;

  പ്രതിജ്ഞാ( ppOut!=0 );
  എങ്കിൽ( NEVER(p==0) ) തിരികെ SQLITE_NOMEM_BKPT;
  എങ്കിൽ( p->xTask==0 ){
    /* assert( p->id==GetCurrentThreadId() ); */
    rc = WAIT_OBJECT_0;
    പ്രതിജ്ഞാ( p->tid==0 );
  }അല്ലെങ്കിൽ{
    പ്രതിജ്ഞാ( p->id!=0 && p->id!=GetCurrentThreadId() );
    rc = sqlite3Win32Wait((HANDLE)p->tid);
    പ്രതിജ്ഞാ( rc!=WAIT_IO_COMPLETION );
    bRc = CloseHandle((HANDLE)p->tid);
    (ശൂന്യം)bRc;  /* Prevent warning when assert() is a no-op */
    പ്രതിജ്ഞാ( bRc );
  }
  എങ്കിൽ( rc==WAIT_OBJECT_0 ) *ppOut = p->pResult;
  sqlite3_free(p);
  തിരികെ (rc==WAIT_OBJECT_0) ? SQLITE_OK : SQLITE_ERROR;
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
ഘടന SQLiteThread {
  ശൂന്യം *(*xTask)(ശൂന്യം*);   /* The routine to run as a thread */
  ശൂന്യം *pIn;               /* Argument to xTask */
  ശൂന്യം *pResult;           /* Result of xTask */
};

/* Create a new thread */
പൂർണ്ണസംഖ്യ sqlite3ThreadCreate(
  SQLiteThread **ppThread,  /* OUT: Write the thread object here */
  ശൂന്യം *(*xTask)(ശൂന്യം*),    /* Routine to run in a separate thread */
  ശൂന്യം *pIn                 /* Argument passed into xTask() */
){
  SQLiteThread *p;

  പ്രതിജ്ഞാ( ppThread!=0 );
  പ്രതിജ്ഞാ( xTask!=0 );
  *ppThread = 0;
  p = sqlite3Malloc(വലുപ്പം(*p));
  എങ്കിൽ( p==0 ) തിരികെ SQLITE_NOMEM_BKPT;
  എങ്കിൽ( (SQLITE_PTR_TO_INT(p)/17)&1 ){
    p->xTask = xTask;
    p->pIn = pIn;
  }അല്ലെങ്കിൽ{
    p->xTask = 0;
    p->pResult = xTask(pIn);
  }
  *ppThread = p;
  തിരികെ SQLITE_OK;
}

/* Get the results of the thread */
പൂർണ്ണസംഖ്യ sqlite3ThreadJoin(SQLiteThread *p, ശൂന്യം **ppOut){

  പ്രതിജ്ഞാ( ppOut!=0 );
  എങ്കിൽ( NEVER(p==0) ) തിരികെ SQLITE_NOMEM_BKPT;
  എങ്കിൽ( p->xTask ){
    *ppOut = p->xTask(p->pIn);
  }അല്ലെങ്കിൽ{
    *ppOut = p->pResult;
  }
  sqlite3_free(p);

#എങ്കിൽ defined(SQLITE_TEST)
  {
    ശൂന്യം *pTstAlloc = sqlite3Malloc(10);
    എങ്കിൽ (!pTstAlloc) തിരികെ SQLITE_NOMEM_BKPT;
    sqlite3_free(pTstAlloc);
  }
#endif

  തിരികെ SQLITE_OK;
}

#endif /* !defined(SQLITE_THREADS_IMPLEMENTED) */
/****************************** End Single-Threaded *************************/
#endif /* SQLITE_MAX_WORKER_THREADS>0 */
