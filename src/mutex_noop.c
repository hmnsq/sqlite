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

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_MUTEX_OMIT

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_DEBUG
/*
** Stub routines for all mutex methods.
**
** This routines provide no mutual exclusion or error checking.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ noopMutexInit(ಶೂನ್ಯ){ ಹಿಂತಿರುಗಿ SQLITE_OK; }
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ noop_Mutex_ಕೊನೆ(ಶೂನ್ಯ){ ಹಿಂತಿರುಗಿ SQLITE_OK; }
ಸ್ಥಿರ sqlite3_mutex *noopMutexAlloc(ಪೂರ್ಣಾಂಕ id){ 
  ಬಳಸದ_PARAMETER(id);
  ಹಿಂತಿರುಗಿ (sqlite3_mutex*)8; 
}
ಸ್ಥಿರ ಶೂನ್ಯ noopMutexFree(sqlite3_mutex *p){ ಬಳಸದ_PARAMETER(p); ಹಿಂತಿರುಗಿ; }
ಸ್ಥಿರ ಶೂನ್ಯ noopMutexEnter(sqlite3_mutex *p){ ಬಳಸದ_PARAMETER(p); ಹಿಂತಿರುಗಿ; }
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ noopMutexTry(sqlite3_mutex *p){
  ಬಳಸದ_PARAMETER(p);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}
ಸ್ಥಿರ ಶೂನ್ಯ noopMutexLeave(sqlite3_mutex *p){ ಬಳಸದ_PARAMETER(p); ಹಿಂತಿರುಗಿ; }

sqlite3_mutex_methods ಸ್ಥಿರಾಂಕ *sqlite3NoopMutex(ಶೂನ್ಯ){
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ sqlite3_mutex_methods sMutex = {
    noopMutexInit,
    noop_Mutex_ಕೊನೆ,
    noopMutexAlloc,
    noopMutexFree,
    noopMutexEnter,
    noopMutexTry,
    noopMutexLeave,

    0,
    0,
  };

  ಹಿಂತಿರುಗಿ &sMutex;
}
#ಆದರೆ_ಅಂತ್ಯ /* !SQLITE_DEBUG */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
/*
** In this implementation, error checking is provided for testing
** and debugging purposes.  The mutexes still do not provide any
** mutual exclusion.
*/

/*
** The mutex object
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ sqlite3_debug_mutex {
  ಪೂರ್ಣಾಂಕ id;     /* The mutex type */
  ಪೂರ್ಣಾಂಕ cnt;    /* Number of entries without a matching leave */
} sqlite3_debug_mutex;

/*
** The sqlite3_mutex_held() and sqlite3_mutex_notheld() routine are
** intended for use inside assert() statements.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ debugMutexHeld(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  ಹಿಂತಿರುಗಿ p==0 || p->cnt>0;
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ debugMutexNotheld(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  ಹಿಂತಿರುಗಿ p==0 || p->cnt==0;
}

/*
** Initialize and deinitialize the mutex subsystem.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ debugMutexInit(ಶೂನ್ಯ){ ಹಿಂತಿರುಗಿ SQLITE_OK; }
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ debug_Mutex_ಕೊನೆ(ಶೂನ್ಯ){ ಹಿಂತಿರುಗಿ SQLITE_OK; }

/*
** The sqlite3_mutex_alloc() routine allocates a new
** mutex and returns a pointer to it.  If it returns NULL
** that means that a mutex could not be allocated. 
*/
ಸ್ಥಿರ sqlite3_mutex *debugMutexAlloc(ಪೂರ್ಣಾಂಕ id){
  ಸ್ಥಿರ sqlite3_debug_mutex aStatic[SQLITE_MUTEX_STATIC_VFS3 - 1];
  sqlite3_debug_mutex *pNew = 0;
  ಆಯ್ಕೆ( id ){
    ಸ್ಥಿತಿ SQLITE_MUTEX_FAST:
    ಸ್ಥಿತಿ SQLITE_MUTEX_RECURSIVE: {
      pNew = sqlite3Malloc(ಗಾತ್ರ(*pNew));
      ಆದರೆ( pNew ){
        pNew->id = id;
        pNew->cnt = 0;
      }
      ನಿಲ್ಲಿಸು;
    }
    ಪೂರ್ವನಿಯೋಜಿತ: {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
      ಆದರೆ( id-2<0 || id-2>=Array_ಗಾತ್ರ(aStatic) ){
        (ಶೂನ್ಯ)SQLITE_MISUSE_BKPT;
        ಹಿಂತಿರುಗಿ 0;
      }
#ಆದರೆ_ಅಂತ್ಯ
      pNew = &aStatic[id-2];
      pNew->id = id;
      ನಿಲ್ಲಿಸು;
    }
  }
  ಹಿಂತಿರುಗಿ (sqlite3_mutex*)pNew;
}

/*
** This routine deallocates a previously allocated mutex.
*/
ಸ್ಥಿರ ಶೂನ್ಯ debugMutexFree(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  ಪ್ರತಿಜ್ಞಾ( p->cnt==0 );
  ಆದರೆ( p->id==SQLITE_MUTEX_RECURSIVE || p->id==SQLITE_MUTEX_FAST ){
    sqlite3_free(p);
  }ಇಲ್ಲದಿದ್ದರೆ{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
    (ಶೂನ್ಯ)SQLITE_MISUSE_BKPT;
#ಆದರೆ_ಅಂತ್ಯ
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ debugMutexTry(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  ಪ್ರತಿಜ್ಞಾ( p->id==SQLITE_MUTEX_RECURSIVE || debugMutexNotheld(pX) );
  p->cnt++;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
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

sqlite3_mutex_methods ಸ್ಥಿರಾಂಕ *sqlite3NoopMutex(ಶೂನ್ಯ){
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ sqlite3_mutex_methods sMutex = {
    debugMutexInit,
    debug_Mutex_ಕೊನೆ,
    debugMutexAlloc,
    debugMutexFree,
    debugMutexEnter,
    debugMutexTry,
    debugMutexLeave,

    debugMutexHeld,
    debugMutexNotheld
  };

  ಹಿಂತಿರುಗಿ &sMutex;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_DEBUG */

/*
** If compiled with SQLITE_MUTEX_NOOP, then the no-op mutex implementation
** is used regardless of the run-time threadsafety setting.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_MUTEX_NOOP
sqlite3_mutex_methods ಸ್ಥಿರಾಂಕ *sqlite3DefaultMutex(ಶೂನ್ಯ){
  ಹಿಂತಿರುಗಿ sqlite3NoopMutex();
}
#ಆದರೆ_ಅಂತ್ಯ /* defined(SQLITE_MUTEX_NOOP) */
#ಆದರೆ_ಅಂತ್ಯ /* !defined(SQLITE_MUTEX_OMIT) */
