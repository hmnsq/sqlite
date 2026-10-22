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
#అన్తర్భావ "sqliteInt.h"
#ఒకవేళ SQLITE_OS_WIN
#  include "os_win.h"
#అయితే_ముగింపు

#ఒకవేళ SQLITE_MAX_కార్మికుడు_దారాలు>0

/********************************* Unix Pthreads ****************************/
#ఒకవేళ SQLITE_OS_UNIX && defined(SQLITE_MUTEX_PTHREADS) && SQLITE_THREADSAFE>0

#నిర్వచించు SQLITE_దారాలు_IMPLEMENTED 1  /* Prevent the single-thread code below */
#అన్తర్భావ <pthread.h>

/* A running thread */
నిర్మాణం SQLiteThread {
  pthread_t tid;                 /* Thread ID */
  పూర్ణాంకం పూర్తి;                      /* Set to true when thread finishes */
  శూన్య *p_బహిర్గతం;                    /* Result returned by the thread */
  శూన్య *(*xTask)(శూన్య*);         /* The thread routine */
  శూన్య *p_లోపల;                     /* Argument to the thread */
};

/* Create a new thread */
పూర్ణాంకం sqlite3ThreadCreate(
  SQLiteThread **ppThread,  /* OUT: Write the thread object here */
  శూన్య *(*xTask)(శూన్య*),    /* Routine to run in a separate thread */
  శూన్య *p_లోపల                 /* Argument passed into xTask() */
){
  SQLiteThread *p;
  పూర్ణాంకం rc;

  ప్రతిజ్ఞా( ppThread!=0 );
  ప్రతిజ్ఞా( xTask!=0 );
  /* This routine is never used in single-threaded mode */
  ప్రతిజ్ఞా( sqlite3GlobalConfig.bCoreMutex!=0 );

  *ppThread = 0;
  p = sqlite3Malloc(పరిమాణం(*p));
  ఒకవేళ( p==0 ) తిరిగి SQLITE_NOMEM_BKPT;
  memset(p, 0, పరిమాణం(*p));
  p->xTask = xTask;
  p->p_లోపల = p_లోపల;
  /* If the SQLITE_TESTCTRL_FAULT_INSTALL callback is registered to a 
  ** function that returns SQLITE_ERROR when passed the argument 200, that
  ** forces worker threads to run sequentially and deterministically 
  ** for testing purposes. */
  ఒకవేళ( sqlite3FaultSim(200) ){
    rc = 1;
  }లేకపోతే{    
    rc = pthread_create(&p->tid, 0, xTask, p_లోపల);
  }
  ఒకవేళ( rc ){
    p->పూర్తి = 1;
    p->p_బహిర్గతం = xTask(p_లోపల);
  }
  *ppThread = p;
  తిరిగి SQLITE_OK;
}

/* Get the results of the thread */
పూర్ణాంకం sqlite3ThreadJoin(SQLiteThread *p, శూన్య **pp_బహిర్గతం){
  పూర్ణాంకం rc;

  ప్రతిజ్ఞా( pp_బహిర్గతం!=0 );
  ఒకవేళ( NEVER(p==0) ) తిరిగి SQLITE_NOMEM_BKPT;
  ఒకవేళ( p->పూర్తి ){
    *pp_బహిర్గతం = p->p_బహిర్గతం;
    rc = SQLITE_OK;
  }లేకపోతే{
    rc = pthread_join(p->tid, pp_బహిర్గతం) ? SQLITE_ERROR : SQLITE_OK;
  }
  sqlite3_free(p);
  తిరిగి rc;
}

#అయితే_ముగింపు /* SQLITE_OS_UNIX && defined(SQLITE_MUTEX_PTHREADS) */
/******************************** End Unix Pthreads *************************/


/********************************* Win32 Threads ****************************/
#ఒకవేళ SQLITE_OS_WIN_దారాలు

#నిర్వచించు SQLITE_దారాలు_IMPLEMENTED 1  /* Prevent the single-thread code below */
#అన్తర్భావ <process.h>

/* A running thread */
నిర్మాణం SQLiteThread {
  శూన్య *tid;               /* The thread handle */
  గుర్తులేని id;             /* The thread identifier */
  శూన్య *(*xTask)(శూన్య*);   /* The routine to run as a thread */
  శూన్య *p_లోపల;               /* Argument to xTask */
  శూన్య *p_ఫలితం;           /* Result of xTask */
};

/* Thread procedure Win32 compatibility shim */
స్థిర గుర్తులేని __stdcall sqlite3ThreadProc(
  శూన్య *pArg  /* IN: Pointer to the SQLiteThread structure */
){
  SQLiteThread *p = (SQLiteThread *)pArg;

  ప్రతిజ్ఞా( p!=0 );
#ఒకవేళ 0
  /*
  ** This assert appears to trigger spuriously on certain
  ** versions of Windows, possibly due to _beginthreadex()
  ** and/or CreateThread() not fully setting their thread
  ** ID parameter before starting the thread.
  */
  ప్రతిజ్ఞా( p->id==GetCurrentThreadId() );
#అయితే_ముగింపు
  ప్రతిజ్ఞా( p->xTask!=0 );
  p->p_ఫలితం = p->xTask(p->p_లోపల);

  _endthreadex(0);
  తిరిగి 0; /* NOT REACHED */
}

/* Create a new thread */
పూర్ణాంకం sqlite3ThreadCreate(
  SQLiteThread **ppThread,  /* OUT: Write the thread object here */
  శూన్య *(*xTask)(శూన్య*),    /* Routine to run in a separate thread */
  శూన్య *p_లోపల                 /* Argument passed into xTask() */
){
  SQLiteThread *p;

  ప్రతిజ్ఞా( ppThread!=0 );
  ప్రతిజ్ఞా( xTask!=0 );
  *ppThread = 0;
  p = sqlite3Malloc(పరిమాణం(*p));
  ఒకవేళ( p==0 ) తిరిగి SQLITE_NOMEM_BKPT;
  /* If the SQLITE_TESTCTRL_FAULT_INSTALL callback is registered to a 
  ** function that returns SQLITE_ERROR when passed the argument 200, that
  ** forces worker threads to run sequentially and deterministically 
  ** (via the sqlite3FaultSim() term of the conditional) for testing
  ** purposes. */
  ఒకవేళ( sqlite3GlobalConfig.bCoreMutex==0 || sqlite3FaultSim(200) ){
    memset(p, 0, పరిమాణం(*p));
  }లేకపోతే{
    p->xTask = xTask;
    p->p_లోపల = p_లోపల;
    p->tid = (శూన్య*)_beginthreadex(0, 0, sqlite3ThreadProc, p, 0, &p->id);
    ఒకవేళ( p->tid==0 ){
      memset(p, 0, పరిమాణం(*p));
    }
  }
  ఒకవేళ( p->xTask==0 ){
    p->id = GetCurrentThreadId();
    p->p_ఫలితం = xTask(p_లోపల);
  }
  *ppThread = p;
  తిరిగి SQLITE_OK;
}

DWORD sqlite3Win32Wait(HANDLE hObject); /* os_win.c */

/* Get the results of the thread */
పూర్ణాంకం sqlite3ThreadJoin(SQLiteThread *p, శూన్య **pp_బహిర్గతం){
  DWORD rc;
  BOOL bRc;

  ప్రతిజ్ఞా( pp_బహిర్గతం!=0 );
  ఒకవేళ( NEVER(p==0) ) తిరిగి SQLITE_NOMEM_BKPT;
  ఒకవేళ( p->xTask==0 ){
    /* assert( p->id==GetCurrentThreadId() ); */
    rc = WAIT_OBJECT_0;
    ప్రతిజ్ఞా( p->tid==0 );
  }లేకపోతే{
    ప్రతిజ్ఞా( p->id!=0 && p->id!=GetCurrentThreadId() );
    rc = sqlite3Win32Wait((HANDLE)p->tid);
    ప్రతిజ్ఞా( rc!=WAIT_IO_COMPLETION );
    bRc = CloseHandle((HANDLE)p->tid);
    (శూన్య)bRc;  /* Prevent warning when assert() is a no-op */
    ప్రతిజ్ఞా( bRc );
  }
  ఒకవేళ( rc==WAIT_OBJECT_0 ) *pp_బహిర్గతం = p->p_ఫలితం;
  sqlite3_free(p);
  తిరిగి (rc==WAIT_OBJECT_0) ? SQLITE_OK : SQLITE_ERROR;
}

#అయితే_ముగింపు /* SQLITE_OS_WIN_THREADS */
/******************************** End Win32 Threads *************************/


/********************************* Single-Threaded **************************/
#నిర్వచితం_కాకపోతే SQLITE_దారాలు_IMPLEMENTED
/*
** This implementation does not actually create a new thread.  It does the
** work of the thread in the main thread, when either the thread is created
** or when it is joined
*/

/* A running thread */
నిర్మాణం SQLiteThread {
  శూన్య *(*xTask)(శూన్య*);   /* The routine to run as a thread */
  శూన్య *p_లోపల;               /* Argument to xTask */
  శూన్య *p_ఫలితం;           /* Result of xTask */
};

/* Create a new thread */
పూర్ణాంకం sqlite3ThreadCreate(
  SQLiteThread **ppThread,  /* OUT: Write the thread object here */
  శూన్య *(*xTask)(శూన్య*),    /* Routine to run in a separate thread */
  శూన్య *p_లోపల                 /* Argument passed into xTask() */
){
  SQLiteThread *p;

  ప్రతిజ్ఞా( ppThread!=0 );
  ప్రతిజ్ఞా( xTask!=0 );
  *ppThread = 0;
  p = sqlite3Malloc(పరిమాణం(*p));
  ఒకవేళ( p==0 ) తిరిగి SQLITE_NOMEM_BKPT;
  ఒకవేళ( (SQLITE_PTR_గమ్యం_INT(p)/17)&1 ){
    p->xTask = xTask;
    p->p_లోపల = p_లోపల;
  }లేకపోతే{
    p->xTask = 0;
    p->p_ఫలితం = xTask(p_లోపల);
  }
  *ppThread = p;
  తిరిగి SQLITE_OK;
}

/* Get the results of the thread */
పూర్ణాంకం sqlite3ThreadJoin(SQLiteThread *p, శూన్య **pp_బహిర్గతం){

  ప్రతిజ్ఞా( pp_బహిర్గతం!=0 );
  ఒకవేళ( NEVER(p==0) ) తిరిగి SQLITE_NOMEM_BKPT;
  ఒకవేళ( p->xTask ){
    *pp_బహిర్గతం = p->xTask(p->p_లోపల);
  }లేకపోతే{
    *pp_బహిర్గతం = p->p_ఫలితం;
  }
  sqlite3_free(p);

#ఒకవేళ defined(SQLITE_TEST)
  {
    శూన్య *pTstAlloc = sqlite3Malloc(10);
    ఒకవేళ (!pTstAlloc) తిరిగి SQLITE_NOMEM_BKPT;
    sqlite3_free(pTstAlloc);
  }
#అయితే_ముగింపు

  తిరిగి SQLITE_OK;
}

#అయితే_ముగింపు /* !defined(SQLITE_THREADS_IMPLEMENTED) */
/****************************** End Single-Threaded *************************/
#అయితే_ముగింపు /* SQLITE_MAX_WORKER_THREADS>0 */
