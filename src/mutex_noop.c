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
#അന്തര്ഭാവ "sqliteInt.h"

#ifndef SQLITE_MUTEX_OMIT

#ifndef SQLITE_DEBUG
/*
** Stub routines for all mutex methods.
**
** This routines provide no mutual exclusion or error checking.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ noopMutexInit(ശൂന്യം){ തിരികെ SQLITE_OK; }
സ്ഥിതം പൂർണ്ണസംഖ്യ noopMutexEnd(ശൂന്യം){ തിരികെ SQLITE_OK; }
സ്ഥിതം sqlite3_mutex *noopMutexAlloc(പൂർണ്ണസംഖ്യ id){ 
  UNUSED_PARAMETER(id);
  തിരികെ (sqlite3_mutex*)8; 
}
സ്ഥിതം ശൂന്യം noopMutexFree(sqlite3_mutex *p){ UNUSED_PARAMETER(p); തിരികെ; }
സ്ഥിതം ശൂന്യം noopMutexEnter(sqlite3_mutex *p){ UNUSED_PARAMETER(p); തിരികെ; }
സ്ഥിതം പൂർണ്ണസംഖ്യ noopMutexTry(sqlite3_mutex *p){
  UNUSED_PARAMETER(p);
  തിരികെ SQLITE_OK;
}
സ്ഥിതം ശൂന്യം noopMutexLeave(sqlite3_mutex *p){ UNUSED_PARAMETER(p); തിരികെ; }

sqlite3_mutex_methods സ്ഥിരം *sqlite3NoopMutex(ശൂന്യം){
  സ്ഥിതം സ്ഥിരം sqlite3_mutex_methods sMutex = {
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

  തിരികെ &sMutex;
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
തരനിർവചനം ഘടന sqlite3_debug_mutex {
  പൂർണ്ണസംഖ്യ id;     /* The mutex type */
  പൂർണ്ണസംഖ്യ cnt;    /* Number of entries without a matching leave */
} sqlite3_debug_mutex;

/*
** The sqlite3_mutex_held() and sqlite3_mutex_notheld() routine are
** intended for use inside assert() statements.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ debugMutexHeld(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  തിരികെ p==0 || p->cnt>0;
}
സ്ഥിതം പൂർണ്ണസംഖ്യ debugMutexNotheld(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  തിരികെ p==0 || p->cnt==0;
}

/*
** Initialize and deinitialize the mutex subsystem.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ debugMutexInit(ശൂന്യം){ തിരികെ SQLITE_OK; }
സ്ഥിതം പൂർണ്ണസംഖ്യ debugMutexEnd(ശൂന്യം){ തിരികെ SQLITE_OK; }

/*
** The sqlite3_mutex_alloc() routine allocates a new
** mutex and returns a pointer to it.  If it returns NULL
** that means that a mutex could not be allocated. 
*/
സ്ഥിതം sqlite3_mutex *debugMutexAlloc(പൂർണ്ണസംഖ്യ id){
  സ്ഥിതം sqlite3_debug_mutex aStatic[SQLITE_MUTEX_STATIC_VFS3 - 1];
  sqlite3_debug_mutex *pNew = 0;
  തിരഞ്ഞെടുപ്പ്( id ){
    അവസ്ഥ SQLITE_MUTEX_FAST:
    അവസ്ഥ SQLITE_MUTEX_RECURSIVE: {
      pNew = sqlite3Malloc(വലുപ്പം(*pNew));
      എങ്കിൽ( pNew ){
        pNew->id = id;
        pNew->cnt = 0;
      }
      നിർത്തുക;
    }
    സ്ഥിരസ്ഥിതി: {
#ifdef SQLITE_ENABLE_API_ARMOR
      എങ്കിൽ( id-2<0 || id-2>=ArraySize(aStatic) ){
        (ശൂന്യം)SQLITE_MISUSE_BKPT;
        തിരികെ 0;
      }
#endif
      pNew = &aStatic[id-2];
      pNew->id = id;
      നിർത്തുക;
    }
  }
  തിരികെ (sqlite3_mutex*)pNew;
}

/*
** This routine deallocates a previously allocated mutex.
*/
സ്ഥിതം ശൂന്യം debugMutexFree(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  പ്രതിജ്ഞാ( p->cnt==0 );
  എങ്കിൽ( p->id==SQLITE_MUTEX_RECURSIVE || p->id==SQLITE_MUTEX_FAST ){
    sqlite3_free(p);
  }അല്ലെങ്കിൽ{
#ifdef SQLITE_ENABLE_API_ARMOR
    (ശൂന്യം)SQLITE_MISUSE_BKPT;
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
സ്ഥിതം ശൂന്യം debugMutexEnter(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  പ്രതിജ്ഞാ( p->id==SQLITE_MUTEX_RECURSIVE || debugMutexNotheld(pX) );
  p->cnt++;
}
സ്ഥിതം പൂർണ്ണസംഖ്യ debugMutexTry(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  പ്രതിജ്ഞാ( p->id==SQLITE_MUTEX_RECURSIVE || debugMutexNotheld(pX) );
  p->cnt++;
  തിരികെ SQLITE_OK;
}

/*
** The sqlite3_mutex_leave() routine exits a mutex that was
** previously entered by the same thread.  The behavior
** is undefined if the mutex is not currently entered or
** is not currently allocated.  SQLite will never do either.
*/
സ്ഥിതം ശൂന്യം debugMutexLeave(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  പ്രതിജ്ഞാ( debugMutexHeld(pX) );
  p->cnt--;
  പ്രതിജ്ഞാ( p->id==SQLITE_MUTEX_RECURSIVE || debugMutexNotheld(pX) );
}

sqlite3_mutex_methods സ്ഥിരം *sqlite3NoopMutex(ശൂന്യം){
  സ്ഥിതം സ്ഥിരം sqlite3_mutex_methods sMutex = {
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

  തിരികെ &sMutex;
}
#endif /* SQLITE_DEBUG */

/*
** If compiled with SQLITE_MUTEX_NOOP, then the no-op mutex implementation
** is used regardless of the run-time threadsafety setting.
*/
#ifdef SQLITE_MUTEX_NOOP
sqlite3_mutex_methods സ്ഥിരം *sqlite3DefaultMutex(ശൂന്യം){
  തിരികെ sqlite3NoopMutex();
}
#endif /* defined(SQLITE_MUTEX_NOOP) */
#endif /* !defined(SQLITE_MUTEX_OMIT) */
