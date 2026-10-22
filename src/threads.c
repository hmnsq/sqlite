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
#شامل "sqliteInt.h"
#اگر SQLITE_OS_WIN
#  include "os_win.h"
#اگر_ختم

#اگر SQLITE_MAX_کارکن_دھاگے>0

/********************************* Unix Pthreads ****************************/
#اگر SQLITE_OS_UNIX && defined(SQLITE_MUTEX_PTHREADS) && SQLITE_THREADSAFE>0

#تعریف_کرو SQLITE_دھاگے_IMPLEMENTED 1  /* Prevent the single-thread code below */
#شامل <pthread.h>

/* A running thread */
ساخت SQLiteThread {
  pthread_t tid;                 /* Thread ID */
  صحیح مکمل;                      /* Set to true when thread finishes */
  خالی *p_برآمد;                    /* Result returned by the thread */
  خالی *(*xTask)(خالی*);         /* The thread routine */
  خالی *p_اندر;                     /* Argument to the thread */
};

/* Create a new thread */
صحیح sqlite3ThreadCreate(
  SQLiteThread **ppThread,  /* OUT: Write the thread object here */
  خالی *(*xTask)(خالی*),    /* Routine to run in a separate thread */
  خالی *p_اندر                 /* Argument passed into xTask() */
){
  SQLiteThread *p;
  صحیح rc;

  أكد( ppThread!=0 );
  أكد( xTask!=0 );
  /* This routine is never used in single-threaded mode */
  أكد( sqlite3GlobalConfig.bCoreMutex!=0 );

  *ppThread = 0;
  p = sqlite3Malloc(حجم(*p));
  اگر( p==0 ) واپس SQLITE_NOMEM_BKPT;
  memset(p, 0, حجم(*p));
  p->xTask = xTask;
  p->p_اندر = p_اندر;
  /* If the SQLITE_TESTCTRL_FAULT_INSTALL callback is registered to a 
  ** function that returns SQLITE_ERROR when passed the argument 200, that
  ** forces worker threads to run sequentially and deterministically 
  ** for testing purposes. */
  اگر( sqlite3FaultSim(200) ){
    rc = 1;
  }ورنہ{    
    rc = pthread_create(&p->tid, 0, xTask, p_اندر);
  }
  اگر( rc ){
    p->مکمل = 1;
    p->p_برآمد = xTask(p_اندر);
  }
  *ppThread = p;
  واپس SQLITE_OK;
}

/* Get the results of the thread */
صحیح sqlite3ThreadJoin(SQLiteThread *p, خالی **pp_برآمد){
  صحیح rc;

  أكد( pp_برآمد!=0 );
  اگر( NEVER(p==0) ) واپس SQLITE_NOMEM_BKPT;
  اگر( p->مکمل ){
    *pp_برآمد = p->p_برآمد;
    rc = SQLITE_OK;
  }ورنہ{
    rc = pthread_join(p->tid, pp_برآمد) ? SQLITE_ERROR : SQLITE_OK;
  }
  sqlite3_free(p);
  واپس rc;
}

#اگر_ختم /* SQLITE_OS_UNIX && defined(SQLITE_MUTEX_PTHREADS) */
/******************************** End Unix Pthreads *************************/


/********************************* Win32 Threads ****************************/
#اگر SQLITE_OS_WIN_دھاگے

#تعریف_کرو SQLITE_دھاگے_IMPLEMENTED 1  /* Prevent the single-thread code below */
#شامل <process.h>

/* A running thread */
ساخت SQLiteThread {
  خالی *tid;               /* The thread handle */
  غیرعلامتی id;             /* The thread identifier */
  خالی *(*xTask)(خالی*);   /* The routine to run as a thread */
  خالی *p_اندر;               /* Argument to xTask */
  خالی *p_نتیجہ;           /* Result of xTask */
};

/* Thread procedure Win32 compatibility shim */
ساکن غیرعلامتی __stdcall sqlite3ThreadProc(
  خالی *pArg  /* IN: Pointer to the SQLiteThread structure */
){
  SQLiteThread *p = (SQLiteThread *)pArg;

  أكد( p!=0 );
#اگر 0
  /*
  ** This assert appears to trigger spuriously on certain
  ** versions of Windows, possibly due to _beginthreadex()
  ** and/or CreateThread() not fully setting their thread
  ** ID parameter before starting the thread.
  */
  أكد( p->id==GetCurrentThreadId() );
#اگر_ختم
  أكد( p->xTask!=0 );
  p->p_نتیجہ = p->xTask(p->p_اندر);

  _endthreadex(0);
  واپس 0; /* NOT REACHED */
}

/* Create a new thread */
صحیح sqlite3ThreadCreate(
  SQLiteThread **ppThread,  /* OUT: Write the thread object here */
  خالی *(*xTask)(خالی*),    /* Routine to run in a separate thread */
  خالی *p_اندر                 /* Argument passed into xTask() */
){
  SQLiteThread *p;

  أكد( ppThread!=0 );
  أكد( xTask!=0 );
  *ppThread = 0;
  p = sqlite3Malloc(حجم(*p));
  اگر( p==0 ) واپس SQLITE_NOMEM_BKPT;
  /* If the SQLITE_TESTCTRL_FAULT_INSTALL callback is registered to a 
  ** function that returns SQLITE_ERROR when passed the argument 200, that
  ** forces worker threads to run sequentially and deterministically 
  ** (via the sqlite3FaultSim() term of the conditional) for testing
  ** purposes. */
  اگر( sqlite3GlobalConfig.bCoreMutex==0 || sqlite3FaultSim(200) ){
    memset(p, 0, حجم(*p));
  }ورنہ{
    p->xTask = xTask;
    p->p_اندر = p_اندر;
    p->tid = (خالی*)_beginthreadex(0, 0, sqlite3ThreadProc, p, 0, &p->id);
    اگر( p->tid==0 ){
      memset(p, 0, حجم(*p));
    }
  }
  اگر( p->xTask==0 ){
    p->id = GetCurrentThreadId();
    p->p_نتیجہ = xTask(p_اندر);
  }
  *ppThread = p;
  واپس SQLITE_OK;
}

DWORD sqlite3Win32Wait(HANDLE hObject); /* os_win.c */

/* Get the results of the thread */
صحیح sqlite3ThreadJoin(SQLiteThread *p, خالی **pp_برآمد){
  DWORD rc;
  BOOL bRc;

  أكد( pp_برآمد!=0 );
  اگر( NEVER(p==0) ) واپس SQLITE_NOMEM_BKPT;
  اگر( p->xTask==0 ){
    /* assert( p->id==GetCurrentThreadId() ); */
    rc = WAIT_OBJECT_0;
    أكد( p->tid==0 );
  }ورنہ{
    أكد( p->id!=0 && p->id!=GetCurrentThreadId() );
    rc = sqlite3Win32Wait((HANDLE)p->tid);
    أكد( rc!=WAIT_IO_COMPLETION );
    bRc = CloseHandle((HANDLE)p->tid);
    (خالی)bRc;  /* Prevent warning when assert() is a no-op */
    أكد( bRc );
  }
  اگر( rc==WAIT_OBJECT_0 ) *pp_برآمد = p->p_نتیجہ;
  sqlite3_free(p);
  واپس (rc==WAIT_OBJECT_0) ? SQLITE_OK : SQLITE_ERROR;
}

#اگر_ختم /* SQLITE_OS_WIN_THREADS */
/******************************** End Win32 Threads *************************/


/********************************* Single-Threaded **************************/
#اگر_غیر_تعریف_شدہ SQLITE_دھاگے_IMPLEMENTED
/*
** This implementation does not actually create a new thread.  It does the
** work of the thread in the main thread, when either the thread is created
** or when it is joined
*/

/* A running thread */
ساخت SQLiteThread {
  خالی *(*xTask)(خالی*);   /* The routine to run as a thread */
  خالی *p_اندر;               /* Argument to xTask */
  خالی *p_نتیجہ;           /* Result of xTask */
};

/* Create a new thread */
صحیح sqlite3ThreadCreate(
  SQLiteThread **ppThread,  /* OUT: Write the thread object here */
  خالی *(*xTask)(خالی*),    /* Routine to run in a separate thread */
  خالی *p_اندر                 /* Argument passed into xTask() */
){
  SQLiteThread *p;

  أكد( ppThread!=0 );
  أكد( xTask!=0 );
  *ppThread = 0;
  p = sqlite3Malloc(حجم(*p));
  اگر( p==0 ) واپس SQLITE_NOMEM_BKPT;
  اگر( (SQLITE_PTR_مقصد_INT(p)/17)&1 ){
    p->xTask = xTask;
    p->p_اندر = p_اندر;
  }ورنہ{
    p->xTask = 0;
    p->p_نتیجہ = xTask(p_اندر);
  }
  *ppThread = p;
  واپس SQLITE_OK;
}

/* Get the results of the thread */
صحیح sqlite3ThreadJoin(SQLiteThread *p, خالی **pp_برآمد){

  أكد( pp_برآمد!=0 );
  اگر( NEVER(p==0) ) واپس SQLITE_NOMEM_BKPT;
  اگر( p->xTask ){
    *pp_برآمد = p->xTask(p->p_اندر);
  }ورنہ{
    *pp_برآمد = p->p_نتیجہ;
  }
  sqlite3_free(p);

#اگر defined(SQLITE_TEST)
  {
    خالی *pTstAlloc = sqlite3Malloc(10);
    اگر (!pTstAlloc) واپس SQLITE_NOMEM_BKPT;
    sqlite3_free(pTstAlloc);
  }
#اگر_ختم

  واپس SQLITE_OK;
}

#اگر_ختم /* !defined(SQLITE_THREADS_IMPLEMENTED) */
/****************************** End Single-Threaded *************************/
#اگر_ختم /* SQLITE_MAX_WORKER_THREADS>0 */
