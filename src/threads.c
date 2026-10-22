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
#यदि_अन्तः

#यदि SQLITE_MAX_कार्यकर्ता_तन्तवः>0

/********************************* Unix Pthreads ****************************/
#यदि SQLITE_OS_UNIX && defined(SQLITE_MUTEX_PTHREADS) && SQLITE_THREADSAFE>0

#परिभाषय SQLITE_तन्तवः_IMPLEMENTED 1  /* Prevent the single-thread code below */
#अन्तर्भाव <pthread.h>

/* A running thread */
संरचना SQLiteThread {
  pthread_t tid;                 /* Thread ID */
  पूर्णाङ्क समाप्तम्;                      /* Set to true when thread finishes */
  शून्य *p_बहिर्गमनम्;                    /* Result returned by the thread */
  शून्य *(*xTask)(शून्य*);         /* The thread routine */
  शून्य *p_अन्तर्;                     /* Argument to the thread */
};

/* Create a new thread */
पूर्णाङ्क sqlite3ThreadCreate(
  SQLiteThread **ppThread,  /* OUT: Write the thread object here */
  शून्य *(*xTask)(शून्य*),    /* Routine to run in a separate thread */
  शून्य *p_अन्तर्                 /* Argument passed into xTask() */
){
  SQLiteThread *p;
  पूर्णाङ्क rc;

  प्रतिज्ञा( ppThread!=0 );
  प्रतिज्ञा( xTask!=0 );
  /* This routine is never used in single-threaded mode */
  प्रतिज्ञा( sqlite3GlobalConfig.bCoreMutex!=0 );

  *ppThread = 0;
  p = sqlite3Malloc(परिमाण(*p));
  यदि( p==0 ) प्रत्यागम SQLITE_NOMEM_BKPT;
  स्मृतिं_पूरय(p, 0, परिमाण(*p));
  p->xTask = xTask;
  p->p_अन्तर् = p_अन्तर्;
  /* If the SQLITE_TESTCTRL_FAULT_INSTALL callback is registered to a 
  ** function that returns SQLITE_ERROR when passed the argument 200, that
  ** forces worker threads to run sequentially and deterministically 
  ** for testing purposes. */
  यदि( sqlite3FaultSim(200) ){
    rc = 1;
  }अन्यथा{    
    rc = pthread_create(&p->tid, 0, xTask, p_अन्तर्);
  }
  यदि( rc ){
    p->समाप्तम् = 1;
    p->p_बहिर्गमनम् = xTask(p_अन्तर्);
  }
  *ppThread = p;
  प्रत्यागम SQLITE_OK;
}

/* Get the results of the thread */
पूर्णाङ्क sqlite3ThreadJoin(SQLiteThread *p, शून्य **pp_बहिर्गमनम्){
  पूर्णाङ्क rc;

  प्रतिज्ञा( pp_बहिर्गमनम्!=0 );
  यदि( NEVER(p==0) ) प्रत्यागम SQLITE_NOMEM_BKPT;
  यदि( p->समाप्तम् ){
    *pp_बहिर्गमनम् = p->p_बहिर्गमनम्;
    rc = SQLITE_OK;
  }अन्यथा{
    rc = pthread_join(p->tid, pp_बहिर्गमनम्) ? SQLITE_ERROR : SQLITE_OK;
  }
  sqlite3_free(p);
  प्रत्यागम rc;
}

#यदि_अन्तः /* SQLITE_OS_UNIX && defined(SQLITE_MUTEX_PTHREADS) */
/******************************** End Unix Pthreads *************************/


/********************************* Win32 Threads ****************************/
#यदि SQLITE_OS_WIN_तन्तवः

#परिभाषय SQLITE_तन्तवः_IMPLEMENTED 1  /* Prevent the single-thread code below */
#अन्तर्भाव <process.h>

/* A running thread */
संरचना SQLiteThread {
  शून्य *tid;               /* The thread handle */
  अचिह्नित id;             /* The thread identifier */
  शून्य *(*xTask)(शून्य*);   /* The routine to run as a thread */
  शून्य *p_अन्तर्;               /* Argument to xTask */
  शून्य *p_परिणाम;           /* Result of xTask */
};

/* Thread procedure Win32 compatibility shim */
स्थिर अचिह्नित __stdcall sqlite3ThreadProc(
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
#यदि_अन्तः
  प्रतिज्ञा( p->xTask!=0 );
  p->p_परिणाम = p->xTask(p->p_अन्तर्);

  _endthreadex(0);
  प्रत्यागम 0; /* NOT REACHED */
}

/* Create a new thread */
पूर्णाङ्क sqlite3ThreadCreate(
  SQLiteThread **ppThread,  /* OUT: Write the thread object here */
  शून्य *(*xTask)(शून्य*),    /* Routine to run in a separate thread */
  शून्य *p_अन्तर्                 /* Argument passed into xTask() */
){
  SQLiteThread *p;

  प्रतिज्ञा( ppThread!=0 );
  प्रतिज्ञा( xTask!=0 );
  *ppThread = 0;
  p = sqlite3Malloc(परिमाण(*p));
  यदि( p==0 ) प्रत्यागम SQLITE_NOMEM_BKPT;
  /* If the SQLITE_TESTCTRL_FAULT_INSTALL callback is registered to a 
  ** function that returns SQLITE_ERROR when passed the argument 200, that
  ** forces worker threads to run sequentially and deterministically 
  ** (via the sqlite3FaultSim() term of the conditional) for testing
  ** purposes. */
  यदि( sqlite3GlobalConfig.bCoreMutex==0 || sqlite3FaultSim(200) ){
    स्मृतिं_पूरय(p, 0, परिमाण(*p));
  }अन्यथा{
    p->xTask = xTask;
    p->p_अन्तर् = p_अन्तर्;
    p->tid = (शून्य*)_beginthreadex(0, 0, sqlite3ThreadProc, p, 0, &p->id);
    यदि( p->tid==0 ){
      स्मृतिं_पूरय(p, 0, परिमाण(*p));
    }
  }
  यदि( p->xTask==0 ){
    p->id = GetCurrentThreadId();
    p->p_परिणाम = xTask(p_अन्तर्);
  }
  *ppThread = p;
  प्रत्यागम SQLITE_OK;
}

DWORD sqlite3Win32Wait(HANDLE hObject); /* os_win.c */

/* Get the results of the thread */
पूर्णाङ्क sqlite3ThreadJoin(SQLiteThread *p, शून्य **pp_बहिर्गमनम्){
  DWORD rc;
  BOOL bRc;

  प्रतिज्ञा( pp_बहिर्गमनम्!=0 );
  यदि( NEVER(p==0) ) प्रत्यागम SQLITE_NOMEM_BKPT;
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
  यदि( rc==WAIT_OBJECT_0 ) *pp_बहिर्गमनम् = p->p_परिणाम;
  sqlite3_free(p);
  प्रत्यागम (rc==WAIT_OBJECT_0) ? SQLITE_OK : SQLITE_ERROR;
}

#यदि_अन्तः /* SQLITE_OS_WIN_THREADS */
/******************************** End Win32 Threads *************************/


/********************************* Single-Threaded **************************/
#यदि_अपरिभाषितम् SQLITE_तन्तवः_IMPLEMENTED
/*
** This implementation does not actually create a new thread.  It does the
** work of the thread in the main thread, when either the thread is created
** or when it is joined
*/

/* A running thread */
संरचना SQLiteThread {
  शून्य *(*xTask)(शून्य*);   /* The routine to run as a thread */
  शून्य *p_अन्तर्;               /* Argument to xTask */
  शून्य *p_परिणाम;           /* Result of xTask */
};

/* Create a new thread */
पूर्णाङ्क sqlite3ThreadCreate(
  SQLiteThread **ppThread,  /* OUT: Write the thread object here */
  शून्य *(*xTask)(शून्य*),    /* Routine to run in a separate thread */
  शून्य *p_अन्तर्                 /* Argument passed into xTask() */
){
  SQLiteThread *p;

  प्रतिज्ञा( ppThread!=0 );
  प्रतिज्ञा( xTask!=0 );
  *ppThread = 0;
  p = sqlite3Malloc(परिमाण(*p));
  यदि( p==0 ) प्रत्यागम SQLITE_NOMEM_BKPT;
  यदि( (SQLITE_PTR_गन्तव्यम्_INT(p)/17)&1 ){
    p->xTask = xTask;
    p->p_अन्तर् = p_अन्तर्;
  }अन्यथा{
    p->xTask = 0;
    p->p_परिणाम = xTask(p_अन्तर्);
  }
  *ppThread = p;
  प्रत्यागम SQLITE_OK;
}

/* Get the results of the thread */
पूर्णाङ्क sqlite3ThreadJoin(SQLiteThread *p, शून्य **pp_बहिर्गमनम्){

  प्रतिज्ञा( pp_बहिर्गमनम्!=0 );
  यदि( NEVER(p==0) ) प्रत्यागम SQLITE_NOMEM_BKPT;
  यदि( p->xTask ){
    *pp_बहिर्गमनम् = p->xTask(p->p_अन्तर्);
  }अन्यथा{
    *pp_बहिर्गमनम् = p->p_परिणाम;
  }
  sqlite3_free(p);

#यदि defined(SQLITE_TEST)
  {
    शून्य *pTstAlloc = sqlite3Malloc(10);
    यदि (!pTstAlloc) प्रत्यागम SQLITE_NOMEM_BKPT;
    sqlite3_free(pTstAlloc);
  }
#यदि_अन्तः

  प्रत्यागम SQLITE_OK;
}

#यदि_अन्तः /* !defined(SQLITE_THREADS_IMPLEMENTED) */
/****************************** End Single-Threaded *************************/
#यदि_अन्तः /* SQLITE_MAX_WORKER_THREADS>0 */
