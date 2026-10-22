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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_MUTEX_OMIT

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
/*
** Stub routines for all mutex methods.
**
** This routines provide no mutual exclusion or error checking.
*/
ਸਥਿਰ ਪੂਰਨ noopMutexInit(ਖਾਲੀ){ ਵਾਪਸ SQLITE_OK; }
ਸਥਿਰ ਪੂਰਨ noop_Mutex_ਸਿਰਾ(ਖਾਲੀ){ ਵਾਪਸ SQLITE_OK; }
ਸਥਿਰ sqlite3_mutex *noopMutexAlloc(ਪੂਰਨ id){ 
  ਅਣਵਰਤਿਆ_PARAMETER(id);
  ਵਾਪਸ (sqlite3_mutex*)8; 
}
ਸਥਿਰ ਖਾਲੀ noopMutexFree(sqlite3_mutex *p){ ਅਣਵਰਤਿਆ_PARAMETER(p); ਵਾਪਸ; }
ਸਥਿਰ ਖਾਲੀ noopMutexEnter(sqlite3_mutex *p){ ਅਣਵਰਤਿਆ_PARAMETER(p); ਵਾਪਸ; }
ਸਥਿਰ ਪੂਰਨ noopMutexTry(sqlite3_mutex *p){
  ਅਣਵਰਤਿਆ_PARAMETER(p);
  ਵਾਪਸ SQLITE_OK;
}
ਸਥਿਰ ਖਾਲੀ noopMutexLeave(sqlite3_mutex *p){ ਅਣਵਰਤਿਆ_PARAMETER(p); ਵਾਪਸ; }

sqlite3_mutex_methods ਅਚਲ *sqlite3NoopMutex(ਖਾਲੀ){
  ਸਥਿਰ ਅਚਲ sqlite3_mutex_methods sMutex = {
    noopMutexInit,
    noop_Mutex_ਸਿਰਾ,
    noopMutexAlloc,
    noopMutexFree,
    noopMutexEnter,
    noopMutexTry,
    noopMutexLeave,

    0,
    0,
  };

  ਵਾਪਸ &sMutex;
}
#ਜੇ_ਅੰਤ /* !SQLITE_DEBUG */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
/*
** In this implementation, error checking is provided for testing
** and debugging purposes.  The mutexes still do not provide any
** mutual exclusion.
*/

/*
** The mutex object
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ sqlite3_debug_mutex {
  ਪੂਰਨ id;     /* The mutex type */
  ਪੂਰਨ cnt;    /* Number of entries without a matching leave */
} sqlite3_debug_mutex;

/*
** The sqlite3_mutex_held() and sqlite3_mutex_notheld() routine are
** intended for use inside assert() statements.
*/
ਸਥਿਰ ਪੂਰਨ debugMutexHeld(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  ਵਾਪਸ p==0 || p->cnt>0;
}
ਸਥਿਰ ਪੂਰਨ debugMutexNotheld(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  ਵਾਪਸ p==0 || p->cnt==0;
}

/*
** Initialize and deinitialize the mutex subsystem.
*/
ਸਥਿਰ ਪੂਰਨ debugMutexInit(ਖਾਲੀ){ ਵਾਪਸ SQLITE_OK; }
ਸਥਿਰ ਪੂਰਨ debug_Mutex_ਸਿਰਾ(ਖਾਲੀ){ ਵਾਪਸ SQLITE_OK; }

/*
** The sqlite3_mutex_alloc() routine allocates a new
** mutex and returns a pointer to it.  If it returns NULL
** that means that a mutex could not be allocated. 
*/
ਸਥਿਰ sqlite3_mutex *debugMutexAlloc(ਪੂਰਨ id){
  ਸਥਿਰ sqlite3_debug_mutex aStatic[SQLITE_MUTEX_STATIC_VFS3 - 1];
  sqlite3_debug_mutex *pNew = 0;
  ਚੋਣ( id ){
    ਹਾਲਤ SQLITE_MUTEX_FAST:
    ਹਾਲਤ SQLITE_MUTEX_RECURSIVE: {
      pNew = sqlite3Malloc(ਆਕਾਰ(*pNew));
      ਜੇ( pNew ){
        pNew->id = id;
        pNew->cnt = 0;
      }
      ਰੋਕੋ;
    }
    ਮੂਲ: {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
      ਜੇ( id-2<0 || id-2>=Array_ਮਾਪ(aStatic) ){
        (ਖਾਲੀ)SQLITE_MISUSE_BKPT;
        ਵਾਪਸ 0;
      }
#ਜੇ_ਅੰਤ
      pNew = &aStatic[id-2];
      pNew->id = id;
      ਰੋਕੋ;
    }
  }
  ਵਾਪਸ (sqlite3_mutex*)pNew;
}

/*
** This routine deallocates a previously allocated mutex.
*/
ਸਥਿਰ ਖਾਲੀ debugMutexFree(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  ਪ੍ਰਤਿਜ੍ਞਾ( p->cnt==0 );
  ਜੇ( p->id==SQLITE_MUTEX_RECURSIVE || p->id==SQLITE_MUTEX_FAST ){
    sqlite3_free(p);
  }ਨਹੀਂਤਾਂ{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
    (ਖਾਲੀ)SQLITE_MISUSE_BKPT;
#ਜੇ_ਅੰਤ
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
ਸਥਿਰ ਖਾਲੀ debugMutexEnter(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  ਪ੍ਰਤਿਜ੍ਞਾ( p->id==SQLITE_MUTEX_RECURSIVE || debugMutexNotheld(pX) );
  p->cnt++;
}
ਸਥਿਰ ਪੂਰਨ debugMutexTry(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  ਪ੍ਰਤਿਜ੍ਞਾ( p->id==SQLITE_MUTEX_RECURSIVE || debugMutexNotheld(pX) );
  p->cnt++;
  ਵਾਪਸ SQLITE_OK;
}

/*
** The sqlite3_mutex_leave() routine exits a mutex that was
** previously entered by the same thread.  The behavior
** is undefined if the mutex is not currently entered or
** is not currently allocated.  SQLite will never do either.
*/
ਸਥਿਰ ਖਾਲੀ debugMutexLeave(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  ਪ੍ਰਤਿਜ੍ਞਾ( debugMutexHeld(pX) );
  p->cnt--;
  ਪ੍ਰਤਿਜ੍ਞਾ( p->id==SQLITE_MUTEX_RECURSIVE || debugMutexNotheld(pX) );
}

sqlite3_mutex_methods ਅਚਲ *sqlite3NoopMutex(ਖਾਲੀ){
  ਸਥਿਰ ਅਚਲ sqlite3_mutex_methods sMutex = {
    debugMutexInit,
    debug_Mutex_ਸਿਰਾ,
    debugMutexAlloc,
    debugMutexFree,
    debugMutexEnter,
    debugMutexTry,
    debugMutexLeave,

    debugMutexHeld,
    debugMutexNotheld
  };

  ਵਾਪਸ &sMutex;
}
#ਜੇ_ਅੰਤ /* SQLITE_DEBUG */

/*
** If compiled with SQLITE_MUTEX_NOOP, then the no-op mutex implementation
** is used regardless of the run-time threadsafety setting.
*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_MUTEX_NOOP
sqlite3_mutex_methods ਅਚਲ *sqlite3DefaultMutex(ਖਾਲੀ){
  ਵਾਪਸ sqlite3NoopMutex();
}
#ਜੇ_ਅੰਤ /* defined(SQLITE_MUTEX_NOOP) */
#ਜੇ_ਅੰਤ /* !defined(SQLITE_MUTEX_OMIT) */
