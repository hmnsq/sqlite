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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"
#ਜੇ SQLITE_OS_WIN
#  include "os_win.h"
#ਜੇ_ਅੰਤ

#ਜੇ SQLITE_MAX_ਕਾਮਾ_ਧਾਗੇ>0

/********************************* Unix Pthreads ****************************/
#ਜੇ SQLITE_OS_UNIX && defined(SQLITE_MUTEX_PTHREADS) && SQLITE_THREADSAFE>0

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_ਧਾਗੇ_IMPLEMENTED 1  /* Prevent the single-thread code below */
#ਅਨ੍ਤਰ੍ਭਾਵ <pthread.h>

/* A running thread */
ਬਣਤਰ SQLiteThread {
  pthread_t tid;                 /* Thread ID */
  ਪੂਰਨ ਪੂਰਾ;                      /* Set to true when thread finishes */
  ਖਾਲੀ *p_ਬਾਹਰਲਾ;                    /* Result returned by the thread */
  ਖਾਲੀ *(*xTask)(ਖਾਲੀ*);         /* The thread routine */
  ਖਾਲੀ *p_ਵਿੱਚ;                     /* Argument to the thread */
};

/* Create a new thread */
ਪੂਰਨ sqlite3ThreadCreate(
  SQLiteThread **ppThread,  /* OUT: Write the thread object here */
  ਖਾਲੀ *(*xTask)(ਖਾਲੀ*),    /* Routine to run in a separate thread */
  ਖਾਲੀ *p_ਵਿੱਚ                 /* Argument passed into xTask() */
){
  SQLiteThread *p;
  ਪੂਰਨ rc;

  ਪ੍ਰਤਿਜ੍ਞਾ( ppThread!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( xTask!=0 );
  /* This routine is never used in single-threaded mode */
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3GlobalConfig.bCoreMutex!=0 );

  *ppThread = 0;
  p = sqlite3Malloc(ਆਕਾਰ(*p));
  ਜੇ( p==0 ) ਵਾਪਸ SQLITE_NOMEM_BKPT;
  memset(p, 0, ਆਕਾਰ(*p));
  p->xTask = xTask;
  p->p_ਵਿੱਚ = p_ਵਿੱਚ;
  /* If the SQLITE_TESTCTRL_FAULT_INSTALL callback is registered to a 
  ** function that returns SQLITE_ERROR when passed the argument 200, that
  ** forces worker threads to run sequentially and deterministically 
  ** for testing purposes. */
  ਜੇ( sqlite3FaultSim(200) ){
    rc = 1;
  }ਨਹੀਂਤਾਂ{    
    rc = pthread_create(&p->tid, 0, xTask, p_ਵਿੱਚ);
  }
  ਜੇ( rc ){
    p->ਪੂਰਾ = 1;
    p->p_ਬਾਹਰਲਾ = xTask(p_ਵਿੱਚ);
  }
  *ppThread = p;
  ਵਾਪਸ SQLITE_OK;
}

/* Get the results of the thread */
ਪੂਰਨ sqlite3ThreadJoin(SQLiteThread *p, ਖਾਲੀ **pp_ਬਾਹਰਲਾ){
  ਪੂਰਨ rc;

  ਪ੍ਰਤਿਜ੍ਞਾ( pp_ਬਾਹਰਲਾ!=0 );
  ਜੇ( NEVER(p==0) ) ਵਾਪਸ SQLITE_NOMEM_BKPT;
  ਜੇ( p->ਪੂਰਾ ){
    *pp_ਬਾਹਰਲਾ = p->p_ਬਾਹਰਲਾ;
    rc = SQLITE_OK;
  }ਨਹੀਂਤਾਂ{
    rc = pthread_join(p->tid, pp_ਬਾਹਰਲਾ) ? SQLITE_ERROR : SQLITE_OK;
  }
  sqlite3_free(p);
  ਵਾਪਸ rc;
}

#ਜੇ_ਅੰਤ /* SQLITE_OS_UNIX && defined(SQLITE_MUTEX_PTHREADS) */
/******************************** End Unix Pthreads *************************/


/********************************* Win32 Threads ****************************/
#ਜੇ SQLITE_OS_WIN_ਧਾਗੇ

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_ਧਾਗੇ_IMPLEMENTED 1  /* Prevent the single-thread code below */
#ਅਨ੍ਤਰ੍ਭਾਵ <process.h>

/* A running thread */
ਬਣਤਰ SQLiteThread {
  ਖਾਲੀ *tid;               /* The thread handle */
  ਬਿਨਾਨਿਸ਼ਾਨੀ id;             /* The thread identifier */
  ਖਾਲੀ *(*xTask)(ਖਾਲੀ*);   /* The routine to run as a thread */
  ਖਾਲੀ *p_ਵਿੱਚ;               /* Argument to xTask */
  ਖਾਲੀ *p_ਨਤੀਜਾ;           /* Result of xTask */
};

/* Thread procedure Win32 compatibility shim */
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ __stdcall sqlite3ThreadProc(
  ਖਾਲੀ *pArg  /* IN: Pointer to the SQLiteThread structure */
){
  SQLiteThread *p = (SQLiteThread *)pArg;

  ਪ੍ਰਤਿਜ੍ਞਾ( p!=0 );
#ਜੇ 0
  /*
  ** This assert appears to trigger spuriously on certain
  ** versions of Windows, possibly due to _beginthreadex()
  ** and/or CreateThread() not fully setting their thread
  ** ID parameter before starting the thread.
  */
  ਪ੍ਰਤਿਜ੍ਞਾ( p->id==GetCurrentThreadId() );
#ਜੇ_ਅੰਤ
  ਪ੍ਰਤਿਜ੍ਞਾ( p->xTask!=0 );
  p->p_ਨਤੀਜਾ = p->xTask(p->p_ਵਿੱਚ);

  _endthreadex(0);
  ਵਾਪਸ 0; /* NOT REACHED */
}

/* Create a new thread */
ਪੂਰਨ sqlite3ThreadCreate(
  SQLiteThread **ppThread,  /* OUT: Write the thread object here */
  ਖਾਲੀ *(*xTask)(ਖਾਲੀ*),    /* Routine to run in a separate thread */
  ਖਾਲੀ *p_ਵਿੱਚ                 /* Argument passed into xTask() */
){
  SQLiteThread *p;

  ਪ੍ਰਤਿਜ੍ਞਾ( ppThread!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( xTask!=0 );
  *ppThread = 0;
  p = sqlite3Malloc(ਆਕਾਰ(*p));
  ਜੇ( p==0 ) ਵਾਪਸ SQLITE_NOMEM_BKPT;
  /* If the SQLITE_TESTCTRL_FAULT_INSTALL callback is registered to a 
  ** function that returns SQLITE_ERROR when passed the argument 200, that
  ** forces worker threads to run sequentially and deterministically 
  ** (via the sqlite3FaultSim() term of the conditional) for testing
  ** purposes. */
  ਜੇ( sqlite3GlobalConfig.bCoreMutex==0 || sqlite3FaultSim(200) ){
    memset(p, 0, ਆਕਾਰ(*p));
  }ਨਹੀਂਤਾਂ{
    p->xTask = xTask;
    p->p_ਵਿੱਚ = p_ਵਿੱਚ;
    p->tid = (ਖਾਲੀ*)_beginthreadex(0, 0, sqlite3ThreadProc, p, 0, &p->id);
    ਜੇ( p->tid==0 ){
      memset(p, 0, ਆਕਾਰ(*p));
    }
  }
  ਜੇ( p->xTask==0 ){
    p->id = GetCurrentThreadId();
    p->p_ਨਤੀਜਾ = xTask(p_ਵਿੱਚ);
  }
  *ppThread = p;
  ਵਾਪਸ SQLITE_OK;
}

DWORD sqlite3Win32Wait(HANDLE hObject); /* os_win.c */

/* Get the results of the thread */
ਪੂਰਨ sqlite3ThreadJoin(SQLiteThread *p, ਖਾਲੀ **pp_ਬਾਹਰਲਾ){
  DWORD rc;
  BOOL bRc;

  ਪ੍ਰਤਿਜ੍ਞਾ( pp_ਬਾਹਰਲਾ!=0 );
  ਜੇ( NEVER(p==0) ) ਵਾਪਸ SQLITE_NOMEM_BKPT;
  ਜੇ( p->xTask==0 ){
    /* assert( p->id==GetCurrentThreadId() ); */
    rc = WAIT_OBJECT_0;
    ਪ੍ਰਤਿਜ੍ਞਾ( p->tid==0 );
  }ਨਹੀਂਤਾਂ{
    ਪ੍ਰਤਿਜ੍ਞਾ( p->id!=0 && p->id!=GetCurrentThreadId() );
    rc = sqlite3Win32Wait((HANDLE)p->tid);
    ਪ੍ਰਤਿਜ੍ਞਾ( rc!=WAIT_IO_COMPLETION );
    bRc = CloseHandle((HANDLE)p->tid);
    (ਖਾਲੀ)bRc;  /* Prevent warning when assert() is a no-op */
    ਪ੍ਰਤਿਜ੍ਞਾ( bRc );
  }
  ਜੇ( rc==WAIT_OBJECT_0 ) *pp_ਬਾਹਰਲਾ = p->p_ਨਤੀਜਾ;
  sqlite3_free(p);
  ਵਾਪਸ (rc==WAIT_OBJECT_0) ? SQLITE_OK : SQLITE_ERROR;
}

#ਜੇ_ਅੰਤ /* SQLITE_OS_WIN_THREADS */
/******************************** End Win32 Threads *************************/


/********************************* Single-Threaded **************************/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_ਧਾਗੇ_IMPLEMENTED
/*
** This implementation does not actually create a new thread.  It does the
** work of the thread in the main thread, when either the thread is created
** or when it is joined
*/

/* A running thread */
ਬਣਤਰ SQLiteThread {
  ਖਾਲੀ *(*xTask)(ਖਾਲੀ*);   /* The routine to run as a thread */
  ਖਾਲੀ *p_ਵਿੱਚ;               /* Argument to xTask */
  ਖਾਲੀ *p_ਨਤੀਜਾ;           /* Result of xTask */
};

/* Create a new thread */
ਪੂਰਨ sqlite3ThreadCreate(
  SQLiteThread **ppThread,  /* OUT: Write the thread object here */
  ਖਾਲੀ *(*xTask)(ਖਾਲੀ*),    /* Routine to run in a separate thread */
  ਖਾਲੀ *p_ਵਿੱਚ                 /* Argument passed into xTask() */
){
  SQLiteThread *p;

  ਪ੍ਰਤਿਜ੍ਞਾ( ppThread!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( xTask!=0 );
  *ppThread = 0;
  p = sqlite3Malloc(ਆਕਾਰ(*p));
  ਜੇ( p==0 ) ਵਾਪਸ SQLITE_NOMEM_BKPT;
  ਜੇ( (SQLITE_PTR_ਮੰਜ਼ਿਲ_INT(p)/17)&1 ){
    p->xTask = xTask;
    p->p_ਵਿੱਚ = p_ਵਿੱਚ;
  }ਨਹੀਂਤਾਂ{
    p->xTask = 0;
    p->p_ਨਤੀਜਾ = xTask(p_ਵਿੱਚ);
  }
  *ppThread = p;
  ਵਾਪਸ SQLITE_OK;
}

/* Get the results of the thread */
ਪੂਰਨ sqlite3ThreadJoin(SQLiteThread *p, ਖਾਲੀ **pp_ਬਾਹਰਲਾ){

  ਪ੍ਰਤਿਜ੍ਞਾ( pp_ਬਾਹਰਲਾ!=0 );
  ਜੇ( NEVER(p==0) ) ਵਾਪਸ SQLITE_NOMEM_BKPT;
  ਜੇ( p->xTask ){
    *pp_ਬਾਹਰਲਾ = p->xTask(p->p_ਵਿੱਚ);
  }ਨਹੀਂਤਾਂ{
    *pp_ਬਾਹਰਲਾ = p->p_ਨਤੀਜਾ;
  }
  sqlite3_free(p);

#ਜੇ defined(SQLITE_TEST)
  {
    ਖਾਲੀ *pTstAlloc = sqlite3Malloc(10);
    ਜੇ (!pTstAlloc) ਵਾਪਸ SQLITE_NOMEM_BKPT;
    sqlite3_free(pTstAlloc);
  }
#ਜੇ_ਅੰਤ

  ਵਾਪਸ SQLITE_OK;
}

#ਜੇ_ਅੰਤ /* !defined(SQLITE_THREADS_IMPLEMENTED) */
/****************************** End Single-Threaded *************************/
#ਜੇ_ਅੰਤ /* SQLITE_MAX_WORKER_THREADS>0 */
