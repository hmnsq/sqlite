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
#અન્તર્ભાવ "sqliteInt.h"
#જો SQLITE_OS_WIN
#  include "os_win.h"
#endif

#જો SQLITE_MAX_WORKER_THREADS>0

/********************************* Unix Pthreads ****************************/
#જો SQLITE_OS_UNIX && defined(SQLITE_MUTEX_PTHREADS) && SQLITE_THREADSAFE>0

#define SQLITE_THREADS_IMPLEMENTED 1  /* Prevent the single-thread code below */
#અન્તર્ભાવ <pthread.h>

/* A running thread */
સંરચના SQLiteThread {
  pthread_t tid;                 /* Thread ID */
  પૂર્ણાંક done;                      /* Set to true when thread finishes */
  શૂન્ય *pOut;                    /* Result returned by the thread */
  શૂન્ય *(*xTask)(શૂન્ય*);         /* The thread routine */
  શૂન્ય *pIn;                     /* Argument to the thread */
};

/* Create a new thread */
પૂર્ણાંક sqlite3ThreadCreate(
  SQLiteThread **ppThread,  /* OUT: Write the thread object here */
  શૂન્ય *(*xTask)(શૂન્ય*),    /* Routine to run in a separate thread */
  શૂન્ય *pIn                 /* Argument passed into xTask() */
){
  SQLiteThread *p;
  પૂર્ણાંક rc;

  પ્રતિજ્ઞા( ppThread!=0 );
  પ્રતિજ્ઞા( xTask!=0 );
  /* This routine is never used in single-threaded mode */
  પ્રતિજ્ઞા( sqlite3GlobalConfig.bCoreMutex!=0 );

  *ppThread = 0;
  p = sqlite3Malloc(કદ(*p));
  જો( p==0 ) પાછા SQLITE_NOMEM_BKPT;
  memset(p, 0, કદ(*p));
  p->xTask = xTask;
  p->pIn = pIn;
  /* If the SQLITE_TESTCTRL_FAULT_INSTALL callback is registered to a 
  ** function that returns SQLITE_ERROR when passed the argument 200, that
  ** forces worker threads to run sequentially and deterministically 
  ** for testing purposes. */
  જો( sqlite3FaultSim(200) ){
    rc = 1;
  }નહીંતર{    
    rc = pthread_create(&p->tid, 0, xTask, pIn);
  }
  જો( rc ){
    p->done = 1;
    p->pOut = xTask(pIn);
  }
  *ppThread = p;
  પાછા SQLITE_OK;
}

/* Get the results of the thread */
પૂર્ણાંક sqlite3ThreadJoin(SQLiteThread *p, શૂન્ય **ppOut){
  પૂર્ણાંક rc;

  પ્રતિજ્ઞા( ppOut!=0 );
  જો( NEVER(p==0) ) પાછા SQLITE_NOMEM_BKPT;
  જો( p->done ){
    *ppOut = p->pOut;
    rc = SQLITE_OK;
  }નહીંતર{
    rc = pthread_join(p->tid, ppOut) ? SQLITE_ERROR : SQLITE_OK;
  }
  sqlite3_free(p);
  પાછા rc;
}

#endif /* SQLITE_OS_UNIX && defined(SQLITE_MUTEX_PTHREADS) */
/******************************** End Unix Pthreads *************************/


/********************************* Win32 Threads ****************************/
#જો SQLITE_OS_WIN_THREADS

#define SQLITE_THREADS_IMPLEMENTED 1  /* Prevent the single-thread code below */
#અન્તર્ભાવ <process.h>

/* A running thread */
સંરચના SQLiteThread {
  શૂન્ય *tid;               /* The thread handle */
  unsigned id;             /* The thread identifier */
  શૂન્ય *(*xTask)(શૂન્ય*);   /* The routine to run as a thread */
  શૂન્ય *pIn;               /* Argument to xTask */
  શૂન્ય *pResult;           /* Result of xTask */
};

/* Thread procedure Win32 compatibility shim */
સ્થિર unsigned __stdcall sqlite3ThreadProc(
  શૂન્ય *pArg  /* IN: Pointer to the SQLiteThread structure */
){
  SQLiteThread *p = (SQLiteThread *)pArg;

  પ્રતિજ્ઞા( p!=0 );
#જો 0
  /*
  ** This assert appears to trigger spuriously on certain
  ** versions of Windows, possibly due to _beginthreadex()
  ** and/or CreateThread() not fully setting their thread
  ** ID parameter before starting the thread.
  */
  પ્રતિજ્ઞા( p->id==GetCurrentThreadId() );
#endif
  પ્રતિજ્ઞા( p->xTask!=0 );
  p->pResult = p->xTask(p->pIn);

  _endthreadex(0);
  પાછા 0; /* NOT REACHED */
}

/* Create a new thread */
પૂર્ણાંક sqlite3ThreadCreate(
  SQLiteThread **ppThread,  /* OUT: Write the thread object here */
  શૂન્ય *(*xTask)(શૂન્ય*),    /* Routine to run in a separate thread */
  શૂન્ય *pIn                 /* Argument passed into xTask() */
){
  SQLiteThread *p;

  પ્રતિજ્ઞા( ppThread!=0 );
  પ્રતિજ્ઞા( xTask!=0 );
  *ppThread = 0;
  p = sqlite3Malloc(કદ(*p));
  જો( p==0 ) પાછા SQLITE_NOMEM_BKPT;
  /* If the SQLITE_TESTCTRL_FAULT_INSTALL callback is registered to a 
  ** function that returns SQLITE_ERROR when passed the argument 200, that
  ** forces worker threads to run sequentially and deterministically 
  ** (via the sqlite3FaultSim() term of the conditional) for testing
  ** purposes. */
  જો( sqlite3GlobalConfig.bCoreMutex==0 || sqlite3FaultSim(200) ){
    memset(p, 0, કદ(*p));
  }નહીંતર{
    p->xTask = xTask;
    p->pIn = pIn;
    p->tid = (શૂન્ય*)_beginthreadex(0, 0, sqlite3ThreadProc, p, 0, &p->id);
    જો( p->tid==0 ){
      memset(p, 0, કદ(*p));
    }
  }
  જો( p->xTask==0 ){
    p->id = GetCurrentThreadId();
    p->pResult = xTask(pIn);
  }
  *ppThread = p;
  પાછા SQLITE_OK;
}

DWORD sqlite3Win32Wait(HANDLE hObject); /* os_win.c */

/* Get the results of the thread */
પૂર્ણાંક sqlite3ThreadJoin(SQLiteThread *p, શૂન્ય **ppOut){
  DWORD rc;
  BOOL bRc;

  પ્રતિજ્ઞા( ppOut!=0 );
  જો( NEVER(p==0) ) પાછા SQLITE_NOMEM_BKPT;
  જો( p->xTask==0 ){
    /* assert( p->id==GetCurrentThreadId() ); */
    rc = WAIT_OBJECT_0;
    પ્રતિજ્ઞા( p->tid==0 );
  }નહીંતર{
    પ્રતિજ્ઞા( p->id!=0 && p->id!=GetCurrentThreadId() );
    rc = sqlite3Win32Wait((HANDLE)p->tid);
    પ્રતિજ્ઞા( rc!=WAIT_IO_COMPLETION );
    bRc = CloseHandle((HANDLE)p->tid);
    (શૂન્ય)bRc;  /* Prevent warning when assert() is a no-op */
    પ્રતિજ્ઞા( bRc );
  }
  જો( rc==WAIT_OBJECT_0 ) *ppOut = p->pResult;
  sqlite3_free(p);
  પાછા (rc==WAIT_OBJECT_0) ? SQLITE_OK : SQLITE_ERROR;
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
સંરચના SQLiteThread {
  શૂન્ય *(*xTask)(શૂન્ય*);   /* The routine to run as a thread */
  શૂન્ય *pIn;               /* Argument to xTask */
  શૂન્ય *pResult;           /* Result of xTask */
};

/* Create a new thread */
પૂર્ણાંક sqlite3ThreadCreate(
  SQLiteThread **ppThread,  /* OUT: Write the thread object here */
  શૂન્ય *(*xTask)(શૂન્ય*),    /* Routine to run in a separate thread */
  શૂન્ય *pIn                 /* Argument passed into xTask() */
){
  SQLiteThread *p;

  પ્રતિજ્ઞા( ppThread!=0 );
  પ્રતિજ્ઞા( xTask!=0 );
  *ppThread = 0;
  p = sqlite3Malloc(કદ(*p));
  જો( p==0 ) પાછા SQLITE_NOMEM_BKPT;
  જો( (SQLITE_PTR_TO_INT(p)/17)&1 ){
    p->xTask = xTask;
    p->pIn = pIn;
  }નહીંતર{
    p->xTask = 0;
    p->pResult = xTask(pIn);
  }
  *ppThread = p;
  પાછા SQLITE_OK;
}

/* Get the results of the thread */
પૂર્ણાંક sqlite3ThreadJoin(SQLiteThread *p, શૂન્ય **ppOut){

  પ્રતિજ્ઞા( ppOut!=0 );
  જો( NEVER(p==0) ) પાછા SQLITE_NOMEM_BKPT;
  જો( p->xTask ){
    *ppOut = p->xTask(p->pIn);
  }નહીંતર{
    *ppOut = p->pResult;
  }
  sqlite3_free(p);

#જો defined(SQLITE_TEST)
  {
    શૂન્ય *pTstAlloc = sqlite3Malloc(10);
    જો (!pTstAlloc) પાછા SQLITE_NOMEM_BKPT;
    sqlite3_free(pTstAlloc);
  }
#endif

  પાછા SQLITE_OK;
}

#endif /* !defined(SQLITE_THREADS_IMPLEMENTED) */
/****************************** End Single-Threaded *************************/
#endif /* SQLITE_MAX_WORKER_THREADS>0 */
