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
#شامل "sqliteInt.h"

#ifndef SQLITE_MUTEX_OMIT

#ifndef SQLITE_DEBUG
/*
** Stub routines for all mutex methods.
**
** This routines provide no mutual exclusion or error checking.
*/
ایستا صحیح noopMutexInit(تهی){ بازگشت SQLITE_OK; }
ایستا صحیح noopMutexEnd(تهی){ بازگشت SQLITE_OK; }
ایستا sqlite3_mutex *noopMutexAlloc(صحیح id){ 
  UNUSED_PARAMETER(id);
  بازگشت (sqlite3_mutex*)8; 
}
ایستا تهی noopMutexFree(sqlite3_mutex *p){ UNUSED_PARAMETER(p); بازگشت; }
ایستا تهی noopMutexEnter(sqlite3_mutex *p){ UNUSED_PARAMETER(p); بازگشت; }
ایستا صحیح noopMutexTry(sqlite3_mutex *p){
  UNUSED_PARAMETER(p);
  بازگشت SQLITE_OK;
}
ایستا تهی noopMutexLeave(sqlite3_mutex *p){ UNUSED_PARAMETER(p); بازگشت; }

sqlite3_mutex_methods ثابت *sqlite3NoopMutex(تهی){
  ایستا ثابت sqlite3_mutex_methods sMutex = {
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

  بازگشت &sMutex;
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
تعریفنوع ساختار sqlite3_debug_mutex {
  صحیح id;     /* The mutex type */
  صحیح cnt;    /* Number of entries without a matching leave */
} sqlite3_debug_mutex;

/*
** The sqlite3_mutex_held() and sqlite3_mutex_notheld() routine are
** intended for use inside assert() statements.
*/
ایستا صحیح debugMutexHeld(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  بازگشت p==0 || p->cnt>0;
}
ایستا صحیح debugMutexNotheld(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  بازگشت p==0 || p->cnt==0;
}

/*
** Initialize and deinitialize the mutex subsystem.
*/
ایستا صحیح debugMutexInit(تهی){ بازگشت SQLITE_OK; }
ایستا صحیح debugMutexEnd(تهی){ بازگشت SQLITE_OK; }

/*
** The sqlite3_mutex_alloc() routine allocates a new
** mutex and returns a pointer to it.  If it returns NULL
** that means that a mutex could not be allocated. 
*/
ایستا sqlite3_mutex *debugMutexAlloc(صحیح id){
  ایستا sqlite3_debug_mutex aStatic[SQLITE_MUTEX_STATIC_VFS3 - 1];
  sqlite3_debug_mutex *pNew = 0;
  انتخاب( id ){
    حالت SQLITE_MUTEX_FAST:
    حالت SQLITE_MUTEX_RECURSIVE: {
      pNew = sqlite3Malloc(اندازه(*pNew));
      اگر( pNew ){
        pNew->id = id;
        pNew->cnt = 0;
      }
      بشکن;
    }
    پیشفرض: {
#ifdef SQLITE_ENABLE_API_ARMOR
      اگر( id-2<0 || id-2>=ArraySize(aStatic) ){
        (تهی)SQLITE_MISUSE_BKPT;
        بازگشت 0;
      }
#endif
      pNew = &aStatic[id-2];
      pNew->id = id;
      بشکن;
    }
  }
  بازگشت (sqlite3_mutex*)pNew;
}

/*
** This routine deallocates a previously allocated mutex.
*/
ایستا تهی debugMutexFree(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  أكد( p->cnt==0 );
  اگر( p->id==SQLITE_MUTEX_RECURSIVE || p->id==SQLITE_MUTEX_FAST ){
    sqlite3_free(p);
  }وگرنه{
#ifdef SQLITE_ENABLE_API_ARMOR
    (تهی)SQLITE_MISUSE_BKPT;
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
ایستا تهی debugMutexEnter(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  أكد( p->id==SQLITE_MUTEX_RECURSIVE || debugMutexNotheld(pX) );
  p->cnt++;
}
ایستا صحیح debugMutexTry(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  أكد( p->id==SQLITE_MUTEX_RECURSIVE || debugMutexNotheld(pX) );
  p->cnt++;
  بازگشت SQLITE_OK;
}

/*
** The sqlite3_mutex_leave() routine exits a mutex that was
** previously entered by the same thread.  The behavior
** is undefined if the mutex is not currently entered or
** is not currently allocated.  SQLite will never do either.
*/
ایستا تهی debugMutexLeave(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  أكد( debugMutexHeld(pX) );
  p->cnt--;
  أكد( p->id==SQLITE_MUTEX_RECURSIVE || debugMutexNotheld(pX) );
}

sqlite3_mutex_methods ثابت *sqlite3NoopMutex(تهی){
  ایستا ثابت sqlite3_mutex_methods sMutex = {
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

  بازگشت &sMutex;
}
#endif /* SQLITE_DEBUG */

/*
** If compiled with SQLITE_MUTEX_NOOP, then the no-op mutex implementation
** is used regardless of the run-time threadsafety setting.
*/
#ifdef SQLITE_MUTEX_NOOP
sqlite3_mutex_methods ثابت *sqlite3DefaultMutex(تهی){
  بازگشت sqlite3NoopMutex();
}
#endif /* defined(SQLITE_MUTEX_NOOP) */
#endif /* !defined(SQLITE_MUTEX_OMIT) */
