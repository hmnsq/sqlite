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
#अन्तर्भाव "sqliteInt.h"
#यदि SQLITE_OS_WIN
#  include "os_win.h"
#endif

#यदि SQLITE_MAX_WORKER_THREADS>0

/********************************* Unix Pthreads ****************************/
#यदि SQLITE_OS_UNIX && defined(SQLITE_MUTEX_PTHREADS) && SQLITE_THREADSAFE>0

#define SQLITE_THREADS_IMPLEMENTED 1  /* Prevent the single-thread code below */
#अन्तर्भाव <pthread.h>

/* A running thread */
संरचना SQLiteThread {
  pthread_t tid;                 /* Thread ID */
  पूर्णाङ्क done;                      /* Set to true when thread finishes */
  शून्य *pOut;                    /* Result returned by the thread */
  शून्य *(*xTask)(शून्य*);         /* The thread routine */
  शून्य *pIn;                     /* Argument to the thread */
};

/* Create a new thread */
पूर्णाङ्क sqlite3ThreadCreate(
  SQLiteThread **ppThread,  /* OUT: Write the thread object here */
  शून्य *(*xTask)(शून्य*),    /* Routine to run in a separate thread */
  शून्य *pIn                 /* Argument passed into xTask() */
){
  SQLiteThread *p;
  पूर्णाङ्क rc;

  प्रतिज्ञा( ppThread!=0 );
  प्रतिज्ञा( xTask!=0 );
  /* This routine is never used in single-threaded mode */
  प्रतिज्ञा( sqlite3GlobalConfig.bCoreMutex!=0 );

  *ppThread = 0;
  p = sqlite3Malloc(परिमाण(*p));
  यदि( p==0 ) फर्काऊ SQLITE_NOMEM_BKPT;
  स_रखो(p, 0, परिमाण(*p));
  p->xTask = xTask;
  p->pIn = pIn;
  /* If the SQLITE_TESTCTRL_FAULT_INSTALL callback is registered to a 
  ** function that returns SQLITE_ERROR when passed the argument 200, that
  ** forces worker threads to run sequentially and deterministically 
  ** for testing purposes. */
  यदि( sqlite3FaultSim(200) ){
    rc = 1;
  }अन्यथा{    
    rc = pthread_create(&p->tid, 0, xTask, pIn);
  }
  यदि( rc ){
    p->done = 1;
    p->pOut = xTask(pIn);
  }
  *ppThread = p;
  फर्काऊ SQLITE_OK;
}

/* Get the results of the thread */
पूर्णाङ्क sqlite3ThreadJoin(SQLiteThread *p, शून्य **ppOut){
  पूर्णाङ्क rc;

  प्रतिज्ञा( ppOut!=0 );
  यदि( NEVER(p==0) ) फर्काऊ SQLITE_NOMEM_BKPT;
  यदि( p->done ){
    *ppOut = p->pOut;
    rc = SQLITE_OK;
  }अन्यथा{
    rc = pthread_join(p->tid, ppOut) ? SQLITE_ERROR : SQLITE_OK;
  }
  sqlite3_free(p);
  फर्काऊ rc;
}

#endif /* SQLITE_OS_UNIX && defined(SQLITE_MUTEX_PTHREADS) */
/******************************** End Unix Pthreads *************************/


/********************************* Win32 Threads ****************************/
#यदि SQLITE_OS_WIN_THREADS

#define SQLITE_THREADS_IMPLEMENTED 1  /* Prevent the single-thread code below */
#अन्तर्भाव <process.h>

/* A running thread */
संरचना SQLiteThread {
  शून्य *tid;               /* The thread handle */
  अचिन्हित id;             /* The thread identifier */
  शून्य *(*xTask)(शून्य*);   /* The routine to run as a thread */
  शून्य *pIn;               /* Argument to xTask */
  शून्य *pResult;           /* Result of xTask */
};

/* Thread procedure Win32 compatibility shim */
स्थिर अचिन्हित __stdcall sqlite3ThreadProc(
  शून्य *pArg  /* IN: Pointer to the SQLiteThread structure */
){
  SQLiteThread *p = (SQLiteThread *)pArg;

  प्रतिज्ञा( p!=0 );
#यदि 0
  /*
  ** This assert appears to trigger spuriously on certain
  ** versions of Windows, possibly due to _beginthreadex()
  ** and/or CreateThread() not fully setting their thread
  ** ID parameter before starting the thread.
  */
  प्रतिज्ञा( p->id==GetCurrentThreadId() );
#endif
  प्रतिज्ञा( p->xTask!=0 );
  p->pResult = p->xTask(p->pIn);

  _endthreadex(0);
  फर्काऊ 0; /* NOT REACHED */
}

/* Create a new thread */
पूर्णाङ्क sqlite3ThreadCreate(
  SQLiteThread **ppThread,  /* OUT: Write the thread object here */
  शून्य *(*xTask)(शून्य*),    /* Routine to run in a separate thread */
  शून्य *pIn                 /* Argument passed into xTask() */
){
  SQLiteThread *p;

  प्रतिज्ञा( ppThread!=0 );
  प्रतिज्ञा( xTask!=0 );
  *ppThread = 0;
  p = sqlite3Malloc(परिमाण(*p));
  यदि( p==0 ) फर्काऊ SQLITE_NOMEM_BKPT;
  /* If the SQLITE_TESTCTRL_FAULT_INSTALL callback is registered to a 
  ** function that returns SQLITE_ERROR when passed the argument 200, that
  ** forces worker threads to run sequentially and deterministically 
  ** (via the sqlite3FaultSim() term of the conditional) for testing
  ** purposes. */
  यदि( sqlite3GlobalConfig.bCoreMutex==0 || sqlite3FaultSim(200) ){
    स_रखो(p, 0, परिमाण(*p));
  }अन्यथा{
    p->xTask = xTask;
    p->pIn = pIn;
    p->tid = (शून्य*)_beginthreadex(0, 0, sqlite3ThreadProc, p, 0, &p->id);
    यदि( p->tid==0 ){
      स_रखो(p, 0, परिमाण(*p));
    }
  }
  यदि( p->xTask==0 ){
    p->id = GetCurrentThreadId();
    p->pResult = xTask(pIn);
  }
  *ppThread = p;
  फर्काऊ SQLITE_OK;
}

DWORD sqlite3Win32Wait(HANDLE hObject); /* os_win.c */

/* Get the results of the thread */
पूर्णाङ्क sqlite3ThreadJoin(SQLiteThread *p, शून्य **ppOut){
  DWORD rc;
  BOOL bRc;

  प्रतिज्ञा( ppOut!=0 );
  यदि( NEVER(p==0) ) फर्काऊ SQLITE_NOMEM_BKPT;
  यदि( p->xTask==0 ){
    /* assert( p->id==GetCurrentThreadId() ); */
    rc = WAIT_OBJECT_0;
    प्रतिज्ञा( p->tid==0 );
  }अन्यथा{
    प्रतिज्ञा( p->id!=0 && p->id!=GetCurrentThreadId() );
    rc = sqlite3Win32Wait((HANDLE)p->tid);
    प्रतिज्ञा( rc!=WAIT_IO_COMPLETION );
    bRc = CloseHandle((HANDLE)p->tid);
    (शून्य)bRc;  /* Prevent warning when assert() is a no-op */
    प्रतिज्ञा( bRc );
  }
  यदि( rc==WAIT_OBJECT_0 ) *ppOut = p->pResult;
  sqlite3_free(p);
  फर्काऊ (rc==WAIT_OBJECT_0) ? SQLITE_OK : SQLITE_ERROR;
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
संरचना SQLiteThread {
  शून्य *(*xTask)(शून्य*);   /* The routine to run as a thread */
  शून्य *pIn;               /* Argument to xTask */
  शून्य *pResult;           /* Result of xTask */
};

/* Create a new thread */
पूर्णाङ्क sqlite3ThreadCreate(
  SQLiteThread **ppThread,  /* OUT: Write the thread object here */
  शून्य *(*xTask)(शून्य*),    /* Routine to run in a separate thread */
  शून्य *pIn                 /* Argument passed into xTask() */
){
  SQLiteThread *p;

  प्रतिज्ञा( ppThread!=0 );
  प्रतिज्ञा( xTask!=0 );
  *ppThread = 0;
  p = sqlite3Malloc(परिमाण(*p));
  यदि( p==0 ) फर्काऊ SQLITE_NOMEM_BKPT;
  यदि( (SQLITE_PTR_TO_INT(p)/17)&1 ){
    p->xTask = xTask;
    p->pIn = pIn;
  }अन्यथा{
    p->xTask = 0;
    p->pResult = xTask(pIn);
  }
  *ppThread = p;
  फर्काऊ SQLITE_OK;
}

/* Get the results of the thread */
पूर्णाङ्क sqlite3ThreadJoin(SQLiteThread *p, शून्य **ppOut){

  प्रतिज्ञा( ppOut!=0 );
  यदि( NEVER(p==0) ) फर्काऊ SQLITE_NOMEM_BKPT;
  यदि( p->xTask ){
    *ppOut = p->xTask(p->pIn);
  }अन्यथा{
    *ppOut = p->pResult;
  }
  sqlite3_free(p);

#यदि defined(SQLITE_TEST)
  {
    शून्य *pTstAlloc = sqlite3Malloc(10);
    यदि (!pTstAlloc) फर्काऊ SQLITE_NOMEM_BKPT;
    sqlite3_free(pTstAlloc);
  }
#endif

  फर्काऊ SQLITE_OK;
}

#endif /* !defined(SQLITE_THREADS_IMPLEMENTED) */
/****************************** End Single-Threaded *************************/
#endif /* SQLITE_MAX_WORKER_THREADS>0 */
