/*
** 2008 October 07
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file contains the C functions that implement mutexes.
**
** This implementation in this file does not provide any mutual
** exclusion and is thus suitable for use only in applications
** that use SQLite in a single thread.  The routines defined
** here are place-holders.  Applications can substitute working
** mutex routines at start-time using the
**
**     sqlite3_config(SQLITE_CONFIG_MUTEX,...)
**
** interface.
**
** If compiled with SQLITE_DEBUG, then additional logic is inserted
** that does error checking on mutexes to make sure they are being
** called correctly.
*/
#అన్తర్భావ "sqliteInt.h"

#నిర్వచితం_కాకపోతే SQLITE_MUTEX_OMIT

#నిర్వచితం_కాకపోతే SQLITE_DEBUG
/*
** Stub routines for all mutex methods.
**
** This routines provide no mutual exclusion or error checking.
*/
స్థిర పూర్ణాంకం noopMutexInit(శూన్య){ తిరిగి SQLITE_OK; }
స్థిర పూర్ణాంకం noop_Mutex_చివర(శూన్య){ తిరిగి SQLITE_OK; }
స్థిర sqlite3_mutex *noopMutexAlloc(పూర్ణాంకం id){ 
  వాడని_PARAMETER(id);
  తిరిగి (sqlite3_mutex*)8; 
}
స్థిర శూన్య noopMutexFree(sqlite3_mutex *p){ వాడని_PARAMETER(p); తిరిగి; }
స్థిర శూన్య noopMutexEnter(sqlite3_mutex *p){ వాడని_PARAMETER(p); తిరిగి; }
స్థిర పూర్ణాంకం noopMutexTry(sqlite3_mutex *p){
  వాడని_PARAMETER(p);
  తిరిగి SQLITE_OK;
}
స్థిర శూన్య noopMutexLeave(sqlite3_mutex *p){ వాడని_PARAMETER(p); తిరిగి; }

sqlite3_mutex_methods స్థిరరాశి *sqlite3NoopMutex(శూన్య){
  స్థిర స్థిరరాశి sqlite3_mutex_methods sMutex = {
    noopMutexInit,
    noop_Mutex_చివర,
    noopMutexAlloc,
    noopMutexFree,
    noopMutexEnter,
    noopMutexTry,
    noopMutexLeave,

    0,
    0,
  };

  తిరిగి &sMutex;
}
#అయితే_ముగింపు /* !SQLITE_DEBUG */

#నిర్వచితమైతే SQLITE_DEBUG
/*
** In this implementation, error checking is provided for testing
** and debugging purposes.  The mutexes still do not provide any
** mutual exclusion.
*/

/*
** The mutex object
*/
రకనిర్వచనం నిర్మాణం sqlite3_debug_mutex {
  పూర్ణాంకం id;     /* The mutex type */
  పూర్ణాంకం cnt;    /* Number of entries without a matching leave */
} sqlite3_debug_mutex;

/*
** The sqlite3_mutex_held() and sqlite3_mutex_notheld() routine are
** intended for use inside assert() statements.
*/
స్థిర పూర్ణాంకం debugMutexHeld(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  తిరిగి p==0 || p->cnt>0;
}
స్థిర పూర్ణాంకం debugMutexNotheld(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  తిరిగి p==0 || p->cnt==0;
}

/*
** Initialize and deinitialize the mutex subsystem.
*/
స్థిర పూర్ణాంకం debugMutexInit(శూన్య){ తిరిగి SQLITE_OK; }
స్థిర పూర్ణాంకం debug_Mutex_చివర(శూన్య){ తిరిగి SQLITE_OK; }

/*
** The sqlite3_mutex_alloc() routine allocates a new
** mutex and returns a pointer to it.  If it returns NULL
** that means that a mutex could not be allocated. 
*/
స్థిర sqlite3_mutex *debugMutexAlloc(పూర్ణాంకం id){
  స్థిర sqlite3_debug_mutex aStatic[SQLITE_MUTEX_STATIC_VFS3 - 1];
  sqlite3_debug_mutex *pNew = 0;
  ఎంపిక( id ){
    స్థితి SQLITE_MUTEX_FAST:
    స్థితి SQLITE_MUTEX_RECURSIVE: {
      pNew = sqlite3Malloc(పరిమాణం(*pNew));
      ఒకవేళ( pNew ){
        pNew->id = id;
        pNew->cnt = 0;
      }
      ఆపు;
    }
    అప్రమేయ: {
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
      ఒకవేళ( id-2<0 || id-2>=Array_కొలత(aStatic) ){
        (శూన్య)SQLITE_MISUSE_BKPT;
        తిరిగి 0;
      }
#అయితే_ముగింపు
      pNew = &aStatic[id-2];
      pNew->id = id;
      ఆపు;
    }
  }
  తిరిగి (sqlite3_mutex*)pNew;
}

/*
** This routine deallocates a previously allocated mutex.
*/
స్థిర శూన్య debugMutexFree(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  ప్రతిజ్ఞా( p->cnt==0 );
  ఒకవేళ( p->id==SQLITE_MUTEX_RECURSIVE || p->id==SQLITE_MUTEX_FAST ){
    sqlite3_free(p);
  }లేకపోతే{
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
    (శూన్య)SQLITE_MISUSE_BKPT;
#అయితే_ముగింపు
  }
}

/*
** The sqlite3_mutex_enter() and sqlite3_mutex_try() routines attempt
** to enter a mutex.  If another thread is already within the mutex,
** sqlite3_mutex_enter() will block and sqlite3_mutex_try() will return
** SQLITE_BUSY.  The sqlite3_mutex_try() interface returns SQLITE_OK
** upon successful entry.  Mutexes created using SQLITE_MUTEX_RECURSIVE can
** be entered multiple times by the same thread.  In such cases the,
** mutex must be exited an equal number of times before another thread
** can enter.  If the same thread tries to enter any other kind of mutex
** more than once, the behavior is undefined.
*/
స్థిర శూన్య debugMutexEnter(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  ప్రతిజ్ఞా( p->id==SQLITE_MUTEX_RECURSIVE || debugMutexNotheld(pX) );
  p->cnt++;
}
స్థిర పూర్ణాంకం debugMutexTry(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  ప్రతిజ్ఞా( p->id==SQLITE_MUTEX_RECURSIVE || debugMutexNotheld(pX) );
  p->cnt++;
  తిరిగి SQLITE_OK;
}

/*
** The sqlite3_mutex_leave() routine exits a mutex that was
** previously entered by the same thread.  The behavior
** is undefined if the mutex is not currently entered or
** is not currently allocated.  SQLite will never do either.
*/
స్థిర శూన్య debugMutexLeave(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  ప్రతిజ్ఞా( debugMutexHeld(pX) );
  p->cnt--;
  ప్రతిజ్ఞా( p->id==SQLITE_MUTEX_RECURSIVE || debugMutexNotheld(pX) );
}

sqlite3_mutex_methods స్థిరరాశి *sqlite3NoopMutex(శూన్య){
  స్థిర స్థిరరాశి sqlite3_mutex_methods sMutex = {
    debugMutexInit,
    debug_Mutex_చివర,
    debugMutexAlloc,
    debugMutexFree,
    debugMutexEnter,
    debugMutexTry,
    debugMutexLeave,

    debugMutexHeld,
    debugMutexNotheld
  };

  తిరిగి &sMutex;
}
#అయితే_ముగింపు /* SQLITE_DEBUG */

/*
** If compiled with SQLITE_MUTEX_NOOP, then the no-op mutex implementation
** is used regardless of the run-time threadsafety setting.
*/
#నిర్వచితమైతే SQLITE_MUTEX_NOOP
sqlite3_mutex_methods స్థిరరాశి *sqlite3DefaultMutex(శూన్య){
  తిరిగి sqlite3NoopMutex();
}
#అయితే_ముగింపు /* defined(SQLITE_MUTEX_NOOP) */
#అయితే_ముగింపు /* !defined(SQLITE_MUTEX_OMIT) */
