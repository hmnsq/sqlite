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
#अन्तर्भाव "sqliteInt.h"

#ifndef SQLITE_MUTEX_OMIT

#ifndef SQLITE_DEBUG
/*
** Stub routines for all mutex methods.
**
** This routines provide no mutual exclusion or error checking.
*/
स्थिर पूर्णांक noopMutexInit(शून्य){ परत SQLITE_OK; }
स्थिर पूर्णांक noopMutexEnd(शून्य){ परत SQLITE_OK; }
स्थिर sqlite3_mutex *noopMutexAlloc(पूर्णांक id){ 
  UNUSED_PARAMETER(id);
  परत (sqlite3_mutex*)8; 
}
स्थिर शून्य noopMutexFree(sqlite3_mutex *p){ UNUSED_PARAMETER(p); परत; }
स्थिर शून्य noopMutexEnter(sqlite3_mutex *p){ UNUSED_PARAMETER(p); परत; }
स्थिर पूर्णांक noopMutexTry(sqlite3_mutex *p){
  UNUSED_PARAMETER(p);
  परत SQLITE_OK;
}
स्थिर शून्य noopMutexLeave(sqlite3_mutex *p){ UNUSED_PARAMETER(p); परत; }

sqlite3_mutex_methods स्थिरांक *sqlite3NoopMutex(शून्य){
  स्थिर स्थिरांक sqlite3_mutex_methods sMutex = {
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

  परत &sMutex;
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
प्रकारव्याख्या संरचना sqlite3_debug_mutex {
  पूर्णांक id;     /* The mutex type */
  पूर्णांक cnt;    /* Number of entries without a matching leave */
} sqlite3_debug_mutex;

/*
** The sqlite3_mutex_held() and sqlite3_mutex_notheld() routine are
** intended for use inside assert() statements.
*/
स्थिर पूर्णांक debugMutexHeld(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  परत p==0 || p->cnt>0;
}
स्थिर पूर्णांक debugMutexNotheld(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  परत p==0 || p->cnt==0;
}

/*
** Initialize and deinitialize the mutex subsystem.
*/
स्थिर पूर्णांक debugMutexInit(शून्य){ परत SQLITE_OK; }
स्थिर पूर्णांक debugMutexEnd(शून्य){ परत SQLITE_OK; }

/*
** The sqlite3_mutex_alloc() routine allocates a new
** mutex and returns a pointer to it.  If it returns NULL
** that means that a mutex could not be allocated. 
*/
स्थिर sqlite3_mutex *debugMutexAlloc(पूर्णांक id){
  स्थिर sqlite3_debug_mutex aStatic[SQLITE_MUTEX_STATIC_VFS3 - 1];
  sqlite3_debug_mutex *pNew = 0;
  निवड( id ){
    स्थिती SQLITE_MUTEX_FAST:
    स्थिती SQLITE_MUTEX_RECURSIVE: {
      pNew = sqlite3Malloc(आकार(*pNew));
      जर( pNew ){
        pNew->id = id;
        pNew->cnt = 0;
      }
      थांबा;
    }
    पूर्वनिर्धारित: {
#ifdef SQLITE_ENABLE_API_ARMOR
      जर( id-2<0 || id-2>=ArraySize(aStatic) ){
        (शून्य)SQLITE_MISUSE_BKPT;
        परत 0;
      }
#endif
      pNew = &aStatic[id-2];
      pNew->id = id;
      थांबा;
    }
  }
  परत (sqlite3_mutex*)pNew;
}

/*
** This routine deallocates a previously allocated mutex.
*/
स्थिर शून्य debugMutexFree(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  प्रतिज्ञा( p->cnt==0 );
  जर( p->id==SQLITE_MUTEX_RECURSIVE || p->id==SQLITE_MUTEX_FAST ){
    sqlite3_free(p);
  }अन्यथा{
#ifdef SQLITE_ENABLE_API_ARMOR
    (शून्य)SQLITE_MISUSE_BKPT;
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
स्थिर शून्य debugMutexEnter(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  प्रतिज्ञा( p->id==SQLITE_MUTEX_RECURSIVE || debugMutexNotheld(pX) );
  p->cnt++;
}
स्थिर पूर्णांक debugMutexTry(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  प्रतिज्ञा( p->id==SQLITE_MUTEX_RECURSIVE || debugMutexNotheld(pX) );
  p->cnt++;
  परत SQLITE_OK;
}

/*
** The sqlite3_mutex_leave() routine exits a mutex that was
** previously entered by the same thread.  The behavior
** is undefined if the mutex is not currently entered or
** is not currently allocated.  SQLite will never do either.
*/
स्थिर शून्य debugMutexLeave(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  प्रतिज्ञा( debugMutexHeld(pX) );
  p->cnt--;
  प्रतिज्ञा( p->id==SQLITE_MUTEX_RECURSIVE || debugMutexNotheld(pX) );
}

sqlite3_mutex_methods स्थिरांक *sqlite3NoopMutex(शून्य){
  स्थिर स्थिरांक sqlite3_mutex_methods sMutex = {
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

  परत &sMutex;
}
#endif /* SQLITE_DEBUG */

/*
** If compiled with SQLITE_MUTEX_NOOP, then the no-op mutex implementation
** is used regardless of the run-time threadsafety setting.
*/
#ifdef SQLITE_MUTEX_NOOP
sqlite3_mutex_methods स्थिरांक *sqlite3DefaultMutex(शून्य){
  परत sqlite3NoopMutex();
}
#endif /* defined(SQLITE_MUTEX_NOOP) */
#endif /* !defined(SQLITE_MUTEX_OMIT) */
