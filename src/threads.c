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
#ಆದರೆ SQLITE_OS_WIN
#  include "os_win.h"
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ SQLITE_MAX_ಕೆಲಸಗಾರ_ಎಳೆಗಳು>0

/********************************* Unix Pthreads ****************************/
#ಆದರೆ SQLITE_OS_UNIX && defined(SQLITE_MUTEX_PTHREADS) && SQLITE_THREADSAFE>0

#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_ಎಳೆಗಳು_IMPLEMENTED 1  /* Prevent the single-thread code below */
#ಅನ್ತರ್ಭಾವ <pthread.h>

/* A running thread */
ರಚನೆ SQLiteThread {
  pthread_t tid;                 /* Thread ID */
  ಪೂರ್ಣಾಂಕ ಮುಗಿದ;                      /* Set to true when thread finishes */
  ಶೂನ್ಯ *p_ಹೊರಹೋಗು;                    /* Result returned by the thread */
  ಶೂನ್ಯ *(*xTask)(ಶೂನ್ಯ*);         /* The thread routine */
  ಶೂನ್ಯ *p_ಒಳ;                     /* Argument to the thread */
};

/* Create a new thread */
ಪೂರ್ಣಾಂಕ sqlite3ThreadCreate(
  SQLiteThread **ppThread,  /* OUT: Write the thread object here */
  ಶೂನ್ಯ *(*xTask)(ಶೂನ್ಯ*),    /* Routine to run in a separate thread */
  ಶೂನ್ಯ *p_ಒಳ                 /* Argument passed into xTask() */
){
  SQLiteThread *p;
  ಪೂರ್ಣಾಂಕ rc;

  ಪ್ರತಿಜ್ಞಾ( ppThread!=0 );
  ಪ್ರತಿಜ್ಞಾ( xTask!=0 );
  /* This routine is never used in single-threaded mode */
  ಪ್ರತಿಜ್ಞಾ( sqlite3GlobalConfig.bCoreMutex!=0 );

  *ppThread = 0;
  p = sqlite3Malloc(ಗಾತ್ರ(*p));
  ಆದರೆ( p==0 ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
  memset(p, 0, ಗಾತ್ರ(*p));
  p->xTask = xTask;
  p->p_ಒಳ = p_ಒಳ;
  /* If the SQLITE_TESTCTRL_FAULT_INSTALL callback is registered to a 
  ** function that returns SQLITE_ERROR when passed the argument 200, that
  ** forces worker threads to run sequentially and deterministically 
  ** for testing purposes. */
  ಆದರೆ( sqlite3FaultSim(200) ){
    rc = 1;
  }ಇಲ್ಲದಿದ್ದರೆ{    
    rc = pthread_create(&p->tid, 0, xTask, p_ಒಳ);
  }
  ಆದರೆ( rc ){
    p->ಮುಗಿದ = 1;
    p->p_ಹೊರಹೋಗು = xTask(p_ಒಳ);
  }
  *ppThread = p;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/* Get the results of the thread */
ಪೂರ್ಣಾಂಕ sqlite3ThreadJoin(SQLiteThread *p, ಶೂನ್ಯ **pp_ಹೊರಹೋಗು){
  ಪೂರ್ಣಾಂಕ rc;

  ಪ್ರತಿಜ್ಞಾ( pp_ಹೊರಹೋಗು!=0 );
  ಆದರೆ( NEVER(p==0) ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
  ಆದರೆ( p->ಮುಗಿದ ){
    *pp_ಹೊರಹೋಗು = p->p_ಹೊರಹೋಗು;
    rc = SQLITE_OK;
  }ಇಲ್ಲದಿದ್ದರೆ{
    rc = pthread_join(p->tid, pp_ಹೊರಹೋಗು) ? SQLITE_ERROR : SQLITE_OK;
  }
  sqlite3_free(p);
  ಹಿಂತಿರುಗಿ rc;
}

#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OS_UNIX && defined(SQLITE_MUTEX_PTHREADS) */
/******************************** End Unix Pthreads *************************/


/********************************* Win32 Threads ****************************/
#ಆದರೆ SQLITE_OS_WIN_ಎಳೆಗಳು

#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_ಎಳೆಗಳು_IMPLEMENTED 1  /* Prevent the single-thread code below */
#ಅನ್ತರ್ಭಾವ <process.h>

/* A running thread */
ರಚನೆ SQLiteThread {
  ಶೂನ್ಯ *tid;               /* The thread handle */
  ಚಿಹ್ನೆರಹಿತ id;             /* The thread identifier */
  ಶೂನ್ಯ *(*xTask)(ಶೂನ್ಯ*);   /* The routine to run as a thread */
  ಶೂನ್ಯ *p_ಒಳ;               /* Argument to xTask */
  ಶೂನ್ಯ *p_ಫಲಿತಾಂಶ;           /* Result of xTask */
};

/* Thread procedure Win32 compatibility shim */
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ __stdcall sqlite3ThreadProc(
  ಶೂನ್ಯ *pArg  /* IN: Pointer to the SQLiteThread structure */
){
  SQLiteThread *p = (SQLiteThread *)pArg;

  ಪ್ರತಿಜ್ಞಾ( p!=0 );
#ಆದರೆ 0
  /*
  ** This assert appears to trigger spuriously on certain
  ** versions of Windows, possibly due to _beginthreadex()
  ** and/or CreateThread() not fully setting their thread
  ** ID parameter before starting the thread.
  */
  ಪ್ರತಿಜ್ಞಾ( p->id==GetCurrentThreadId() );
#ಆದರೆ_ಅಂತ್ಯ
  ಪ್ರತಿಜ್ಞಾ( p->xTask!=0 );
  p->p_ಫಲಿತಾಂಶ = p->xTask(p->p_ಒಳ);

  _endthreadex(0);
  ಹಿಂತಿರುಗಿ 0; /* NOT REACHED */
}

/* Create a new thread */
ಪೂರ್ಣಾಂಕ sqlite3ThreadCreate(
  SQLiteThread **ppThread,  /* OUT: Write the thread object here */
  ಶೂನ್ಯ *(*xTask)(ಶೂನ್ಯ*),    /* Routine to run in a separate thread */
  ಶೂನ್ಯ *p_ಒಳ                 /* Argument passed into xTask() */
){
  SQLiteThread *p;

  ಪ್ರತಿಜ್ಞಾ( ppThread!=0 );
  ಪ್ರತಿಜ್ಞಾ( xTask!=0 );
  *ppThread = 0;
  p = sqlite3Malloc(ಗಾತ್ರ(*p));
  ಆದರೆ( p==0 ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
  /* If the SQLITE_TESTCTRL_FAULT_INSTALL callback is registered to a 
  ** function that returns SQLITE_ERROR when passed the argument 200, that
  ** forces worker threads to run sequentially and deterministically 
  ** (via the sqlite3FaultSim() term of the conditional) for testing
  ** purposes. */
  ಆದರೆ( sqlite3GlobalConfig.bCoreMutex==0 || sqlite3FaultSim(200) ){
    memset(p, 0, ಗಾತ್ರ(*p));
  }ಇಲ್ಲದಿದ್ದರೆ{
    p->xTask = xTask;
    p->p_ಒಳ = p_ಒಳ;
    p->tid = (ಶೂನ್ಯ*)_beginthreadex(0, 0, sqlite3ThreadProc, p, 0, &p->id);
    ಆದರೆ( p->tid==0 ){
      memset(p, 0, ಗಾತ್ರ(*p));
    }
  }
  ಆದರೆ( p->xTask==0 ){
    p->id = GetCurrentThreadId();
    p->p_ಫಲಿತಾಂಶ = xTask(p_ಒಳ);
  }
  *ppThread = p;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

DWORD sqlite3Win32Wait(HANDLE hObject); /* os_win.c */

/* Get the results of the thread */
ಪೂರ್ಣಾಂಕ sqlite3ThreadJoin(SQLiteThread *p, ಶೂನ್ಯ **pp_ಹೊರಹೋಗು){
  DWORD rc;
  BOOL bRc;

  ಪ್ರತಿಜ್ಞಾ( pp_ಹೊರಹೋಗು!=0 );
  ಆದರೆ( NEVER(p==0) ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
  ಆದರೆ( p->xTask==0 ){
    /* assert( p->id==GetCurrentThreadId() ); */
    rc = WAIT_OBJECT_0;
    ಪ್ರತಿಜ್ಞಾ( p->tid==0 );
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪ್ರತಿಜ್ಞಾ( p->id!=0 && p->id!=GetCurrentThreadId() );
    rc = sqlite3Win32Wait((HANDLE)p->tid);
    ಪ್ರತಿಜ್ಞಾ( rc!=WAIT_IO_COMPLETION );
    bRc = CloseHandle((HANDLE)p->tid);
    (ಶೂನ್ಯ)bRc;  /* Prevent warning when assert() is a no-op */
    ಪ್ರತಿಜ್ಞಾ( bRc );
  }
  ಆದರೆ( rc==WAIT_OBJECT_0 ) *pp_ಹೊರಹೋಗು = p->p_ಫಲಿತಾಂಶ;
  sqlite3_free(p);
  ಹಿಂತಿರುಗಿ (rc==WAIT_OBJECT_0) ? SQLITE_OK : SQLITE_ERROR;
}

#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OS_WIN_THREADS */
/******************************** End Win32 Threads *************************/


/********************************* Single-Threaded **************************/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_ಎಳೆಗಳು_IMPLEMENTED
/*
** This implementation does not actually create a new thread.  It does the
** work of the thread in the main thread, when either the thread is created
** or when it is joined
*/

/* A running thread */
ರಚನೆ SQLiteThread {
  ಶೂನ್ಯ *(*xTask)(ಶೂನ್ಯ*);   /* The routine to run as a thread */
  ಶೂನ್ಯ *p_ಒಳ;               /* Argument to xTask */
  ಶೂನ್ಯ *p_ಫಲಿತಾಂಶ;           /* Result of xTask */
};

/* Create a new thread */
ಪೂರ್ಣಾಂಕ sqlite3ThreadCreate(
  SQLiteThread **ppThread,  /* OUT: Write the thread object here */
  ಶೂನ್ಯ *(*xTask)(ಶೂನ್ಯ*),    /* Routine to run in a separate thread */
  ಶೂನ್ಯ *p_ಒಳ                 /* Argument passed into xTask() */
){
  SQLiteThread *p;

  ಪ್ರತಿಜ್ಞಾ( ppThread!=0 );
  ಪ್ರತಿಜ್ಞಾ( xTask!=0 );
  *ppThread = 0;
  p = sqlite3Malloc(ಗಾತ್ರ(*p));
  ಆದರೆ( p==0 ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
  ಆದರೆ( (SQLITE_PTR_ಗಮ್ಯ_INT(p)/17)&1 ){
    p->xTask = xTask;
    p->p_ಒಳ = p_ಒಳ;
  }ಇಲ್ಲದಿದ್ದರೆ{
    p->xTask = 0;
    p->p_ಫಲಿತಾಂಶ = xTask(p_ಒಳ);
  }
  *ppThread = p;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/* Get the results of the thread */
ಪೂರ್ಣಾಂಕ sqlite3ThreadJoin(SQLiteThread *p, ಶೂನ್ಯ **pp_ಹೊರಹೋಗು){

  ಪ್ರತಿಜ್ಞಾ( pp_ಹೊರಹೋಗು!=0 );
  ಆದರೆ( NEVER(p==0) ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
  ಆದರೆ( p->xTask ){
    *pp_ಹೊರಹೋಗು = p->xTask(p->p_ಒಳ);
  }ಇಲ್ಲದಿದ್ದರೆ{
    *pp_ಹೊರಹೋಗು = p->p_ಫಲಿತಾಂಶ;
  }
  sqlite3_free(p);

#ಆದರೆ defined(SQLITE_TEST)
  {
    ಶೂನ್ಯ *pTstAlloc = sqlite3Malloc(10);
    ಆದರೆ (!pTstAlloc) ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
    sqlite3_free(pTstAlloc);
  }
#ಆದರೆ_ಅಂತ್ಯ

  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

#ಆದರೆ_ಅಂತ್ಯ /* !defined(SQLITE_THREADS_IMPLEMENTED) */
/****************************** End Single-Threaded *************************/
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_MAX_WORKER_THREADS>0 */
