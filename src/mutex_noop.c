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
#include "sqliteInt.h"

#ifndef SQLITE_MUTEX_OMIT

#ifndef SQLITE_DEBUG
/*
** Stub routines for all mutex methods.
**
** This routines provide no mutual exclusion or error checking.
*/
static ចំនួនគត់ noopMutexInit(void){ ត្រឡប់ SQLITE_OK; }
static ចំនួនគត់ noopMutexEnd(void){ ត្រឡប់ SQLITE_OK; }
static sqlite3_mutex *noopMutexAlloc(ចំនួនគត់ id){ 
  UNUSED_PARAMETER(id);
  ត្រឡប់ (sqlite3_mutex*)8; 
}
static void noopMutexFree(sqlite3_mutex *p){ UNUSED_PARAMETER(p); ត្រឡប់; }
static void noopMutexEnter(sqlite3_mutex *p){ UNUSED_PARAMETER(p); ត្រឡប់; }
static ចំនួនគត់ noopMutexTry(sqlite3_mutex *p){
  UNUSED_PARAMETER(p);
  ត្រឡប់ SQLITE_OK;
}
static void noopMutexLeave(sqlite3_mutex *p){ UNUSED_PARAMETER(p); ត្រឡប់; }

sqlite3_mutex_methods const *sqlite3NoopMutex(void){
  static const sqlite3_mutex_methods sMutex = {
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

  ត្រឡប់ &sMutex;
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
typedef sa_nracanaa sqlite3_debug_mutex {
  ចំនួនគត់ id;     /* The mutex type */
  ចំនួនគត់ cnt;    /* Number of entries without a matching leave */
} sqlite3_debug_mutex;

/*
** The sqlite3_mutex_held() and sqlite3_mutex_notheld() routine are
** intended for use inside assert() statements.
*/
static ចំនួនគត់ debugMutexHeld(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  ត្រឡប់ p==0 || p->cnt>0;
}
static ចំនួនគត់ debugMutexNotheld(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  ត្រឡប់ p==0 || p->cnt==0;
}

/*
** Initialize and deinitialize the mutex subsystem.
*/
static ចំនួនគត់ debugMutexInit(void){ ត្រឡប់ SQLITE_OK; }
static ចំនួនគត់ debugMutexEnd(void){ ត្រឡប់ SQLITE_OK; }

/*
** The sqlite3_mutex_alloc() routine allocates a new
** mutex and returns a pointer to it.  If it returns NULL
** that means that a mutex could not be allocated. 
*/
static sqlite3_mutex *debugMutexAlloc(ចំនួនគត់ id){
  static sqlite3_debug_mutex aStatic[SQLITE_MUTEX_STATIC_VFS3 - 1];
  sqlite3_debug_mutex *pNew = 0;
  switch( id ){
    pak_sa SQLITE_MUTEX_FAST:
    pak_sa SQLITE_MUTEX_RECURSIVE: {
      pNew = sqlite3Malloc(parimaa_na(*pNew));
      ប្រសិនបើ( pNew ){
        pNew->id = id;
        pNew->cnt = 0;
      }
      bha_nga;
    }
    default: {
#ifdef SQLITE_ENABLE_API_ARMOR
      ប្រសិនបើ( id-2<0 || id-2>=ArraySize(aStatic) ){
        (void)SQLITE_MISUSE_BKPT;
        ត្រឡប់ 0;
      }
#endif
      pNew = &aStatic[id-2];
      pNew->id = id;
      bha_nga;
    }
  }
  ត្រឡប់ (sqlite3_mutex*)pNew;
}

/*
** This routine deallocates a previously allocated mutex.
*/
static void debugMutexFree(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  assert( p->cnt==0 );
  ប្រសិនបើ( p->id==SQLITE_MUTEX_RECURSIVE || p->id==SQLITE_MUTEX_FAST ){
    sqlite3_free(p);
  }បើមិនដូច្នេះ{
#ifdef SQLITE_ENABLE_API_ARMOR
    (void)SQLITE_MISUSE_BKPT;
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
static void debugMutexEnter(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  assert( p->id==SQLITE_MUTEX_RECURSIVE || debugMutexNotheld(pX) );
  p->cnt++;
}
static ចំនួនគត់ debugMutexTry(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  assert( p->id==SQLITE_MUTEX_RECURSIVE || debugMutexNotheld(pX) );
  p->cnt++;
  ត្រឡប់ SQLITE_OK;
}

/*
** The sqlite3_mutex_leave() routine exits a mutex that was
** previously entered by the same thread.  The behavior
** is undefined if the mutex is not currently entered or
** is not currently allocated.  SQLite will never do either.
*/
static void debugMutexLeave(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  assert( debugMutexHeld(pX) );
  p->cnt--;
  assert( p->id==SQLITE_MUTEX_RECURSIVE || debugMutexNotheld(pX) );
}

sqlite3_mutex_methods const *sqlite3NoopMutex(void){
  static const sqlite3_mutex_methods sMutex = {
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

  ត្រឡប់ &sMutex;
}
#endif /* SQLITE_DEBUG */

/*
** If compiled with SQLITE_MUTEX_NOOP, then the no-op mutex implementation
** is used regardless of the run-time threadsafety setting.
*/
#ifdef SQLITE_MUTEX_NOOP
sqlite3_mutex_methods const *sqlite3DefaultMutex(void){
  ត្រឡប់ sqlite3NoopMutex();
}
#endif /* defined(SQLITE_MUTEX_NOOP) */
#endif /* !defined(SQLITE_MUTEX_OMIT) */
