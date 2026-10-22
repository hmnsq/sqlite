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
#समावेश "sqliteInt.h"

#अगर_अघोषित SQLITE_MUTEX_OMIT

#अगर_अघोषित SQLITE_DEBUG
/*
** Stub routines for all mutex methods.
**
** This routines provide no mutual exclusion or error checking.
*/
अटल पूर्णांक noopMutexInit(व्योम){ वापस SQLITE_OK; }
अटल पूर्णांक noopMutexEnd(व्योम){ वापस SQLITE_OK; }
अटल sqlite3_mutex *noopMutexAlloc(पूर्णांक id){ 
  UNUSED_PARAMETER(id);
  वापस (sqlite3_mutex*)8; 
}
अटल व्योम noopMutexFree(sqlite3_mutex *p){ UNUSED_PARAMETER(p); वापस; }
अटल व्योम noopMutexEnter(sqlite3_mutex *p){ UNUSED_PARAMETER(p); वापस; }
अटल पूर्णांक noopMutexTry(sqlite3_mutex *p){
  UNUSED_PARAMETER(p);
  वापस SQLITE_OK;
}
अटल व्योम noopMutexLeave(sqlite3_mutex *p){ UNUSED_PARAMETER(p); वापस; }

sqlite3_mutex_methods स्थिर *sqlite3NoopMutex(व्योम){
  अटल स्थिर sqlite3_mutex_methods sMutex = {
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

  वापस &sMutex;
}
#पूर्ण_अगर /* !SQLITE_DEBUG */

#अगर_घोषित SQLITE_DEBUG
/*
** In this implementation, error checking is provided for testing
** and debugging purposes.  The mutexes still do not provide any
** mutual exclusion.
*/

/*
** The mutex object
*/
प्रकार काष्ठा sqlite3_debug_mutex {
  पूर्णांक id;     /* The mutex type */
  पूर्णांक cnt;    /* Number of entries without a matching leave */
} sqlite3_debug_mutex;

/*
** The sqlite3_mutex_held() and sqlite3_mutex_notheld() routine are
** intended for use inside assert() statements.
*/
अटल पूर्णांक debugMutexHeld(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  वापस p==0 || p->cnt>0;
}
अटल पूर्णांक debugMutexNotheld(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  वापस p==0 || p->cnt==0;
}

/*
** Initialize and deinitialize the mutex subsystem.
*/
अटल पूर्णांक debugMutexInit(व्योम){ वापस SQLITE_OK; }
अटल पूर्णांक debugMutexEnd(व्योम){ वापस SQLITE_OK; }

/*
** The sqlite3_mutex_alloc() routine allocates a new
** mutex and returns a pointer to it.  If it returns NULL
** that means that a mutex could not be allocated. 
*/
अटल sqlite3_mutex *debugMutexAlloc(पूर्णांक id){
  अटल sqlite3_debug_mutex aStatic[SQLITE_MUTEX_STATIC_VFS3 - 1];
  sqlite3_debug_mutex *pNew = 0;
  चयन( id ){
    हाल SQLITE_MUTEX_FAST:
    हाल SQLITE_MUTEX_RECURSIVE: {
      pNew = sqlite3Malloc(माप(*pNew));
      अगर( pNew ){
        pNew->id = id;
        pNew->cnt = 0;
      }
      अवरोध;
    }
    शेष: {
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
      अगर( id-2<0 || id-2>=Array_आकार(aStatic) ){
        (व्योम)SQLITE_MISUSE_BKPT;
        वापस 0;
      }
#पूर्ण_अगर
      pNew = &aStatic[id-2];
      pNew->id = id;
      अवरोध;
    }
  }
  वापस (sqlite3_mutex*)pNew;
}

/*
** This routine deallocates a previously allocated mutex.
*/
अटल व्योम debugMutexFree(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  निश्चित( p->cnt==0 );
  अगर( p->id==SQLITE_MUTEX_RECURSIVE || p->id==SQLITE_MUTEX_FAST ){
    sqlite3_free(p);
  }अन्यथा{
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
    (व्योम)SQLITE_MISUSE_BKPT;
#पूर्ण_अगर
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
अटल व्योम debugMutexEnter(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  निश्चित( p->id==SQLITE_MUTEX_RECURSIVE || debugMutexNotheld(pX) );
  p->cnt++;
}
अटल पूर्णांक debugMutexTry(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  निश्चित( p->id==SQLITE_MUTEX_RECURSIVE || debugMutexNotheld(pX) );
  p->cnt++;
  वापस SQLITE_OK;
}

/*
** The sqlite3_mutex_leave() routine exits a mutex that was
** previously entered by the same thread.  The behavior
** is undefined if the mutex is not currently entered or
** is not currently allocated.  SQLite will never do either.
*/
अटल व्योम debugMutexLeave(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  निश्चित( debugMutexHeld(pX) );
  p->cnt--;
  निश्चित( p->id==SQLITE_MUTEX_RECURSIVE || debugMutexNotheld(pX) );
}

sqlite3_mutex_methods स्थिर *sqlite3NoopMutex(व्योम){
  अटल स्थिर sqlite3_mutex_methods sMutex = {
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

  वापस &sMutex;
}
#पूर्ण_अगर /* SQLITE_DEBUG */

/*
** If compiled with SQLITE_MUTEX_NOOP, then the no-op mutex implementation
** is used regardless of the run-time threadsafety setting.
*/
#अगर_घोषित SQLITE_MUTEX_NOOP
sqlite3_mutex_methods स्थिर *sqlite3DefaultMutex(व्योम){
  वापस sqlite3NoopMutex();
}
#पूर्ण_अगर /* defined(SQLITE_MUTEX_NOOP) */
#पूर्ण_अगर /* !defined(SQLITE_MUTEX_OMIT) */
