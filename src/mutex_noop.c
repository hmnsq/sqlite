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
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"

#ifndef SQLITE_MUTEX_OMIT

#ifndef SQLITE_DEBUG
/*
** Stub routines for all mutex methods.
**
** This routines provide no mutual exclusion or error checking.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ noopMutexInit(ಶೂನ್ಯ){ ಪ್ರತ್ಯಾಗಮ SQLITE_OK; }
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ noopMutexEnd(ಶೂನ್ಯ){ ಪ್ರತ್ಯಾಗಮ SQLITE_OK; }
ಸ್ಥಿರ sqlite3_mutex *noopMutexAlloc(ಪೂರ್ಣಾಙ್ಕ id){ 
  UNUSED_PARAMETER(id);
  ಪ್ರತ್ಯಾಗಮ (sqlite3_mutex*)8; 
}
ಸ್ಥಿರ ಶೂನ್ಯ noopMutexFree(sqlite3_mutex *p){ UNUSED_PARAMETER(p); ಪ್ರತ್ಯಾಗಮ; }
ಸ್ಥಿರ ಶೂನ್ಯ noopMutexEnter(sqlite3_mutex *p){ UNUSED_PARAMETER(p); ಪ್ರತ್ಯಾಗಮ; }
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ noopMutexTry(sqlite3_mutex *p){
  UNUSED_PARAMETER(p);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}
ಸ್ಥಿರ ಶೂನ್ಯ noopMutexLeave(sqlite3_mutex *p){ UNUSED_PARAMETER(p); ಪ್ರತ್ಯಾಗಮ; }

sqlite3_mutex_methods ಧ್ರುವ *sqlite3NoopMutex(ಶೂನ್ಯ){
  ಸ್ಥಿರ ಧ್ರುವ sqlite3_mutex_methods sMutex = {
    noopMutexInit,
    noopMutexEnd,
    noopMutexAlloc,
    noopMutexFree,
    noopMutexEnter,
    noopMutexTry,
    noopMutexLeave,

    0,
    0,
  };

  ಪ್ರತ್ಯಾಗಮ &sMutex;
}
#endif /* !SQLITE_DEBUG */

#ifdef SQLITE_DEBUG
/*
** In this implementation, error checking is provided for testing
** and debugging purposes.  The mutexes still do not provide any
** mutual exclusion.
*/

/*
** The mutex object
*/
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ sqlite3_debug_mutex {
  ಪೂರ್ಣಾಙ್ಕ id;     /* The mutex type */
  ಪೂರ್ಣಾಙ್ಕ cnt;    /* Number of entries without a matching leave */
} sqlite3_debug_mutex;

/*
** The sqlite3_mutex_held() and sqlite3_mutex_notheld() routine are
** intended for use inside assert() statements.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ debugMutexHeld(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  ಪ್ರತ್ಯಾಗಮ p==0 || p->cnt>0;
}
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ debugMutexNotheld(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  ಪ್ರತ್ಯಾಗಮ p==0 || p->cnt==0;
}

/*
** Initialize and deinitialize the mutex subsystem.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ debugMutexInit(ಶೂನ್ಯ){ ಪ್ರತ್ಯಾಗಮ SQLITE_OK; }
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ debugMutexEnd(ಶೂನ್ಯ){ ಪ್ರತ್ಯಾಗಮ SQLITE_OK; }

/*
** The sqlite3_mutex_alloc() routine allocates a new
** mutex and returns a pointer to it.  If it returns NULL
** that means that a mutex could not be allocated. 
*/
ಸ್ಥಿರ sqlite3_mutex *debugMutexAlloc(ಪೂರ್ಣಾಙ್ಕ id){
  ಸ್ಥಿರ sqlite3_debug_mutex aStatic[SQLITE_MUTEX_STATIC_VFS3 - 1];
  sqlite3_debug_mutex *pNew = 0;
  ವಿಕಲ್ಪ( id ){
    ಪಕ್ಷ SQLITE_MUTEX_FAST:
    ಪಕ್ಷ SQLITE_MUTEX_RECURSIVE: {
      pNew = sqlite3Malloc(ಪರಿಮಾಣ(*pNew));
      ಯದಿ( pNew ){
        pNew->id = id;
        pNew->cnt = 0;
      }
      ಭಙ್ಗ;
    }
    ಸ್ವಭಾವ: {
#ifdef SQLITE_ENABLE_API_ARMOR
      ಯದಿ( id-2<0 || id-2>=ArraySize(aStatic) ){
        (ಶೂನ್ಯ)SQLITE_MISUSE_BKPT;
        ಪ್ರತ್ಯಾಗಮ 0;
      }
#endif
      pNew = &aStatic[id-2];
      pNew->id = id;
      ಭಙ್ಗ;
    }
  }
  ಪ್ರತ್ಯಾಗಮ (sqlite3_mutex*)pNew;
}

/*
** This routine deallocates a previously allocated mutex.
*/
ಸ್ಥಿರ ಶೂನ್ಯ debugMutexFree(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  ಪ್ರತಿಜ್ಞಾ( p->cnt==0 );
  ಯದಿ( p->id==SQLITE_MUTEX_RECURSIVE || p->id==SQLITE_MUTEX_FAST ){
    sqlite3_free(p);
  }ಅನ್ಯಥಾ{
#ifdef SQLITE_ENABLE_API_ARMOR
    (ಶೂನ್ಯ)SQLITE_MISUSE_BKPT;
#endif
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
ಸ್ಥಿರ ಶೂನ್ಯ debugMutexEnter(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  ಪ್ರತಿಜ್ಞಾ( p->id==SQLITE_MUTEX_RECURSIVE || debugMutexNotheld(pX) );
  p->cnt++;
}
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ debugMutexTry(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  ಪ್ರತಿಜ್ಞಾ( p->id==SQLITE_MUTEX_RECURSIVE || debugMutexNotheld(pX) );
  p->cnt++;
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** The sqlite3_mutex_leave() routine exits a mutex that was
** previously entered by the same thread.  The behavior
** is undefined if the mutex is not currently entered or
** is not currently allocated.  SQLite will never do either.
*/
ಸ್ಥಿರ ಶೂನ್ಯ debugMutexLeave(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  ಪ್ರತಿಜ್ಞಾ( debugMutexHeld(pX) );
  p->cnt--;
  ಪ್ರತಿಜ್ಞಾ( p->id==SQLITE_MUTEX_RECURSIVE || debugMutexNotheld(pX) );
}

sqlite3_mutex_methods ಧ್ರುವ *sqlite3NoopMutex(ಶೂನ್ಯ){
  ಸ್ಥಿರ ಧ್ರುವ sqlite3_mutex_methods sMutex = {
    debugMutexInit,
    debugMutexEnd,
    debugMutexAlloc,
    debugMutexFree,
    debugMutexEnter,
    debugMutexTry,
    debugMutexLeave,

    debugMutexHeld,
    debugMutexNotheld
  };

  ಪ್ರತ್ಯಾಗಮ &sMutex;
}
#endif /* SQLITE_DEBUG */

/*
** If compiled with SQLITE_MUTEX_NOOP, then the no-op mutex implementation
** is used regardless of the run-time threadsafety setting.
*/
#ifdef SQLITE_MUTEX_NOOP
sqlite3_mutex_methods ಧ್ರುವ *sqlite3DefaultMutex(ಶೂನ್ಯ){
  ಪ್ರತ್ಯಾಗಮ sqlite3NoopMutex();
}
#endif /* defined(SQLITE_MUTEX_NOOP) */
#endif /* !defined(SQLITE_MUTEX_OMIT) */
