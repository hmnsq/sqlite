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
#समावेश "sqliteInt.h"
#अगर SQLITE_OS_WIN
#  include "os_win.h"
#पूर्ण_अगर

#अगर SQLITE_MAX_WORKER_THREADS>0

/********************************* Unix Pthreads ****************************/
#अगर SQLITE_OS_UNIX && defined(SQLITE_MUTEX_PTHREADS) && SQLITE_THREADSAFE>0

#घोषणा SQLITE_THREADS_IMPLEMENTED 1  /* Prevent the single-thread code below */
#समावेश <pthread.h>

/* A running thread */
काष्ठा SQLiteThread {
  pthread_t tid;                 /* Thread ID */
  पूर्णांक done;                      /* Set to true when thread finishes */
  व्योम *pOut;                    /* Result returned by the thread */
  व्योम *(*xTask)(व्योम*);         /* The thread routine */
  व्योम *pIn;                     /* Argument to the thread */
};

/* Create a new thread */
पूर्णांक sqlite3ThreadCreate(
  SQLiteThread **ppThread,  /* OUT: Write the thread object here */
  व्योम *(*xTask)(व्योम*),    /* Routine to run in a separate thread */
  व्योम *pIn                 /* Argument passed into xTask() */
){
  SQLiteThread *p;
  पूर्णांक rc;

  निश्चित( ppThread!=0 );
  निश्चित( xTask!=0 );
  /* This routine is never used in single-threaded mode */
  निश्चित( sqlite3GlobalConfig.bCoreMutex!=0 );

  *ppThread = 0;
  p = sqlite3Malloc(माप(*p));
  अगर( p==0 ) वापस SQLITE_NOMEM_BKPT;
  स_रखो(p, 0, माप(*p));
  p->xTask = xTask;
  p->pIn = pIn;
  /* If the SQLITE_TESTCTRL_FAULT_INSTALL callback is registered to a 
  ** function that returns SQLITE_ERROR when passed the argument 200, that
  ** forces worker threads to run sequentially and deterministically 
  ** for testing purposes. */
  अगर( sqlite3FaultSim(200) ){
    rc = 1;
  }अन्यथा{    
    rc = pthread_create(&p->tid, 0, xTask, pIn);
  }
  अगर( rc ){
    p->done = 1;
    p->pOut = xTask(pIn);
  }
  *ppThread = p;
  वापस SQLITE_OK;
}

/* Get the results of the thread */
पूर्णांक sqlite3ThreadJoin(SQLiteThread *p, व्योम **ppOut){
  पूर्णांक rc;

  निश्चित( ppOut!=0 );
  अगर( NEVER(p==0) ) वापस SQLITE_NOMEM_BKPT;
  अगर( p->done ){
    *ppOut = p->pOut;
    rc = SQLITE_OK;
  }अन्यथा{
    rc = pthread_join(p->tid, ppOut) ? SQLITE_ERROR : SQLITE_OK;
  }
  sqlite3_free(p);
  वापस rc;
}

#पूर्ण_अगर /* SQLITE_OS_UNIX && defined(SQLITE_MUTEX_PTHREADS) */
/******************************** End Unix Pthreads *************************/


/********************************* Win32 Threads ****************************/
#अगर SQLITE_OS_WIN_THREADS

#घोषणा SQLITE_THREADS_IMPLEMENTED 1  /* Prevent the single-thread code below */
#समावेश <process.h>

/* A running thread */
काष्ठा SQLiteThread {
  व्योम *tid;               /* The thread handle */
  अचिन्हित id;             /* The thread identifier */
  व्योम *(*xTask)(व्योम*);   /* The routine to run as a thread */
  व्योम *pIn;               /* Argument to xTask */
  व्योम *p_परिणाम;           /* Result of xTask */
};

/* Thread procedure Win32 compatibility shim */
अटल अचिन्हित __stdcall sqlite3ThreadProc(
  व्योम *pArg  /* IN: Pointer to the SQLiteThread structure */
){
  SQLiteThread *p = (SQLiteThread *)pArg;

  निश्चित( p!=0 );
#अगर 0
  /*
  ** This assert appears to trigger spuriously on certain
  ** versions of Windows, possibly due to _beginthreadex()
  ** and/or CreateThread() not fully setting their thread
  ** ID parameter before starting the thread.
  */
  निश्चित( p->id==GetCurrentThreadId() );
#पूर्ण_अगर
  निश्चित( p->xTask!=0 );
  p->p_परिणाम = p->xTask(p->pIn);

  _endthreadex(0);
  वापस 0; /* NOT REACHED */
}

/* Create a new thread */
पूर्णांक sqlite3ThreadCreate(
  SQLiteThread **ppThread,  /* OUT: Write the thread object here */
  व्योम *(*xTask)(व्योम*),    /* Routine to run in a separate thread */
  व्योम *pIn                 /* Argument passed into xTask() */
){
  SQLiteThread *p;

  निश्चित( ppThread!=0 );
  निश्चित( xTask!=0 );
  *ppThread = 0;
  p = sqlite3Malloc(माप(*p));
  अगर( p==0 ) वापस SQLITE_NOMEM_BKPT;
  /* If the SQLITE_TESTCTRL_FAULT_INSTALL callback is registered to a 
  ** function that returns SQLITE_ERROR when passed the argument 200, that
  ** forces worker threads to run sequentially and deterministically 
  ** (via the sqlite3FaultSim() term of the conditional) for testing
  ** purposes. */
  अगर( sqlite3GlobalConfig.bCoreMutex==0 || sqlite3FaultSim(200) ){
    स_रखो(p, 0, माप(*p));
  }अन्यथा{
    p->xTask = xTask;
    p->pIn = pIn;
    p->tid = (व्योम*)_beginthreadex(0, 0, sqlite3ThreadProc, p, 0, &p->id);
    अगर( p->tid==0 ){
      स_रखो(p, 0, माप(*p));
    }
  }
  अगर( p->xTask==0 ){
    p->id = GetCurrentThreadId();
    p->p_परिणाम = xTask(pIn);
  }
  *ppThread = p;
  वापस SQLITE_OK;
}

DWORD sqlite3Win32Wait(HANDLE hObject); /* os_win.c */

/* Get the results of the thread */
पूर्णांक sqlite3ThreadJoin(SQLiteThread *p, व्योम **ppOut){
  DWORD rc;
  BOOL bRc;

  निश्चित( ppOut!=0 );
  अगर( NEVER(p==0) ) वापस SQLITE_NOMEM_BKPT;
  अगर( p->xTask==0 ){
    /* assert( p->id==GetCurrentThreadId() ); */
    rc = WAIT_OBJECT_0;
    निश्चित( p->tid==0 );
  }अन्यथा{
    निश्चित( p->id!=0 && p->id!=GetCurrentThreadId() );
    rc = sqlite3Win32Wait((HANDLE)p->tid);
    निश्चित( rc!=WAIT_IO_COMPLETION );
    bRc = CloseHandle((HANDLE)p->tid);
    (व्योम)bRc;  /* Prevent warning when assert() is a no-op */
    निश्चित( bRc );
  }
  अगर( rc==WAIT_OBJECT_0 ) *ppOut = p->p_परिणाम;
  sqlite3_free(p);
  वापस (rc==WAIT_OBJECT_0) ? SQLITE_OK : SQLITE_ERROR;
}

#पूर्ण_अगर /* SQLITE_OS_WIN_THREADS */
/******************************** End Win32 Threads *************************/


/********************************* Single-Threaded **************************/
#अगर_अघोषित SQLITE_THREADS_IMPLEMENTED
/*
** This implementation does not actually create a new thread.  It does the
** work of the thread in the main thread, when either the thread is created
** or when it is joined
*/

/* A running thread */
काष्ठा SQLiteThread {
  व्योम *(*xTask)(व्योम*);   /* The routine to run as a thread */
  व्योम *pIn;               /* Argument to xTask */
  व्योम *p_परिणाम;           /* Result of xTask */
};

/* Create a new thread */
पूर्णांक sqlite3ThreadCreate(
  SQLiteThread **ppThread,  /* OUT: Write the thread object here */
  व्योम *(*xTask)(व्योम*),    /* Routine to run in a separate thread */
  व्योम *pIn                 /* Argument passed into xTask() */
){
  SQLiteThread *p;

  निश्चित( ppThread!=0 );
  निश्चित( xTask!=0 );
  *ppThread = 0;
  p = sqlite3Malloc(माप(*p));
  अगर( p==0 ) वापस SQLITE_NOMEM_BKPT;
  अगर( (SQLITE_PTR_TO_INT(p)/17)&1 ){
    p->xTask = xTask;
    p->pIn = pIn;
  }अन्यथा{
    p->xTask = 0;
    p->p_परिणाम = xTask(pIn);
  }
  *ppThread = p;
  वापस SQLITE_OK;
}

/* Get the results of the thread */
पूर्णांक sqlite3ThreadJoin(SQLiteThread *p, व्योम **ppOut){

  निश्चित( ppOut!=0 );
  अगर( NEVER(p==0) ) वापस SQLITE_NOMEM_BKPT;
  अगर( p->xTask ){
    *ppOut = p->xTask(p->pIn);
  }अन्यथा{
    *ppOut = p->p_परिणाम;
  }
  sqlite3_free(p);

#अगर defined(SQLITE_TEST)
  {
    व्योम *pTstAlloc = sqlite3Malloc(10);
    अगर (!pTstAlloc) वापस SQLITE_NOMEM_BKPT;
    sqlite3_free(pTstAlloc);
  }
#पूर्ण_अगर

  वापस SQLITE_OK;
}

#पूर्ण_अगर /* !defined(SQLITE_THREADS_IMPLEMENTED) */
/****************************** End Single-Threaded *************************/
#पूर्ण_अगर /* SQLITE_MAX_WORKER_THREADS>0 */
