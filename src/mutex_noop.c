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
#அந்தர்பாவ "sqliteInt.h"

#ifndef SQLITE_MUTEX_OMIT

#ifndef SQLITE_DEBUG
/*
** Stub routines for all mutex methods.
**
** This routines provide no mutual exclusion or error checking.
*/
நிலையான முழு noopMutexInit(வெறுமை){ திரும்பு SQLITE_OK; }
நிலையான முழு noopMutexEnd(வெறுமை){ திரும்பு SQLITE_OK; }
நிலையான sqlite3_mutex *noopMutexAlloc(முழு id){ 
  UNUSED_PARAMETER(id);
  திரும்பு (sqlite3_mutex*)8; 
}
நிலையான வெறுமை noopMutexFree(sqlite3_mutex *p){ UNUSED_PARAMETER(p); திரும்பு; }
நிலையான வெறுமை noopMutexEnter(sqlite3_mutex *p){ UNUSED_PARAMETER(p); திரும்பு; }
நிலையான முழு noopMutexTry(sqlite3_mutex *p){
  UNUSED_PARAMETER(p);
  திரும்பு SQLITE_OK;
}
நிலையான வெறுமை noopMutexLeave(sqlite3_mutex *p){ UNUSED_PARAMETER(p); திரும்பு; }

sqlite3_mutex_methods மாறிலி *sqlite3NoopMutex(வெறுமை){
  நிலையான மாறிலி sqlite3_mutex_methods sMutex = {
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

  திரும்பு &sMutex;
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
வகைவரை கட்டமைப்பு sqlite3_debug_mutex {
  முழு id;     /* The mutex type */
  முழு cnt;    /* Number of entries without a matching leave */
} sqlite3_debug_mutex;

/*
** The sqlite3_mutex_held() and sqlite3_mutex_notheld() routine are
** intended for use inside assert() statements.
*/
நிலையான முழு debugMutexHeld(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  திரும்பு p==0 || p->cnt>0;
}
நிலையான முழு debugMutexNotheld(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  திரும்பு p==0 || p->cnt==0;
}

/*
** Initialize and deinitialize the mutex subsystem.
*/
நிலையான முழு debugMutexInit(வெறுமை){ திரும்பு SQLITE_OK; }
நிலையான முழு debugMutexEnd(வெறுமை){ திரும்பு SQLITE_OK; }

/*
** The sqlite3_mutex_alloc() routine allocates a new
** mutex and returns a pointer to it.  If it returns NULL
** that means that a mutex could not be allocated. 
*/
நிலையான sqlite3_mutex *debugMutexAlloc(முழு id){
  நிலையான sqlite3_debug_mutex aStatic[SQLITE_MUTEX_STATIC_VFS3 - 1];
  sqlite3_debug_mutex *pNew = 0;
  தேர்வு( id ){
    நிலை SQLITE_MUTEX_FAST:
    நிலை SQLITE_MUTEX_RECURSIVE: {
      pNew = sqlite3Malloc(அளவு(*pNew));
      எனில்( pNew ){
        pNew->id = id;
        pNew->cnt = 0;
      }
      நிறுத்து;
    }
    இயல்பு: {
#ifdef SQLITE_ENABLE_API_ARMOR
      எனில்( id-2<0 || id-2>=ArraySize(aStatic) ){
        (வெறுமை)SQLITE_MISUSE_BKPT;
        திரும்பு 0;
      }
#endif
      pNew = &aStatic[id-2];
      pNew->id = id;
      நிறுத்து;
    }
  }
  திரும்பு (sqlite3_mutex*)pNew;
}

/*
** This routine deallocates a previously allocated mutex.
*/
நிலையான வெறுமை debugMutexFree(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  ப்ரதிஜ்ஞா( p->cnt==0 );
  எனில்( p->id==SQLITE_MUTEX_RECURSIVE || p->id==SQLITE_MUTEX_FAST ){
    sqlite3_free(p);
  }இல்லையெனில்{
#ifdef SQLITE_ENABLE_API_ARMOR
    (வெறுமை)SQLITE_MISUSE_BKPT;
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
நிலையான வெறுமை debugMutexEnter(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  ப்ரதிஜ்ஞா( p->id==SQLITE_MUTEX_RECURSIVE || debugMutexNotheld(pX) );
  p->cnt++;
}
நிலையான முழு debugMutexTry(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  ப்ரதிஜ்ஞா( p->id==SQLITE_MUTEX_RECURSIVE || debugMutexNotheld(pX) );
  p->cnt++;
  திரும்பு SQLITE_OK;
}

/*
** The sqlite3_mutex_leave() routine exits a mutex that was
** previously entered by the same thread.  The behavior
** is undefined if the mutex is not currently entered or
** is not currently allocated.  SQLite will never do either.
*/
நிலையான வெறுமை debugMutexLeave(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  ப்ரதிஜ்ஞா( debugMutexHeld(pX) );
  p->cnt--;
  ப்ரதிஜ்ஞா( p->id==SQLITE_MUTEX_RECURSIVE || debugMutexNotheld(pX) );
}

sqlite3_mutex_methods மாறிலி *sqlite3NoopMutex(வெறுமை){
  நிலையான மாறிலி sqlite3_mutex_methods sMutex = {
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

  திரும்பு &sMutex;
}
#endif /* SQLITE_DEBUG */

/*
** If compiled with SQLITE_MUTEX_NOOP, then the no-op mutex implementation
** is used regardless of the run-time threadsafety setting.
*/
#ifdef SQLITE_MUTEX_NOOP
sqlite3_mutex_methods மாறிலி *sqlite3DefaultMutex(வெறுமை){
  திரும்பு sqlite3NoopMutex();
}
#endif /* defined(SQLITE_MUTEX_NOOP) */
#endif /* !defined(SQLITE_MUTEX_OMIT) */
