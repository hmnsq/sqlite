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
#ଅନ୍ତର୍ଭାଵ "sqliteInt.h"

#ifndef SQLITE_MUTEX_OMIT

#ifndef SQLITE_DEBUG
/*
** Stub routines for all mutex methods.
**
** This routines provide no mutual exclusion or error checking.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା noopMutexInit(ଶୂନ୍ୟ){ ଫେରନ୍ତୁ SQLITE_OK; }
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା noopMutexEnd(ଶୂନ୍ୟ){ ଫେରନ୍ତୁ SQLITE_OK; }
ସ୍ଥିର sqlite3_mutex *noopMutexAlloc(ପୂର୍ଣ୍ଣସଂଖ୍ୟା id){ 
  UNUSED_PARAMETER(id);
  ଫେରନ୍ତୁ (sqlite3_mutex*)8; 
}
ସ୍ଥିର ଶୂନ୍ୟ noopMutexFree(sqlite3_mutex *p){ UNUSED_PARAMETER(p); ଫେରନ୍ତୁ; }
ସ୍ଥିର ଶୂନ୍ୟ noopMutexEnter(sqlite3_mutex *p){ UNUSED_PARAMETER(p); ଫେରନ୍ତୁ; }
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା noopMutexTry(sqlite3_mutex *p){
  UNUSED_PARAMETER(p);
  ଫେରନ୍ତୁ SQLITE_OK;
}
ସ୍ଥିର ଶୂନ୍ୟ noopMutexLeave(sqlite3_mutex *p){ UNUSED_PARAMETER(p); ଫେରନ୍ତୁ; }

sqlite3_mutex_methods ଅଚଳ *sqlite3NoopMutex(ଶୂନ୍ୟ){
  ସ୍ଥିର ଅଚଳ sqlite3_mutex_methods sMutex = {
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

  ଫେରନ୍ତୁ &sMutex;
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
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା sqlite3_debug_mutex {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା id;     /* The mutex type */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା cnt;    /* Number of entries without a matching leave */
} sqlite3_debug_mutex;

/*
** The sqlite3_mutex_held() and sqlite3_mutex_notheld() routine are
** intended for use inside assert() statements.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା debugMutexHeld(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  ଫେରନ୍ତୁ p==0 || p->cnt>0;
}
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା debugMutexNotheld(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  ଫେରନ୍ତୁ p==0 || p->cnt==0;
}

/*
** Initialize and deinitialize the mutex subsystem.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା debugMutexInit(ଶୂନ୍ୟ){ ଫେରନ୍ତୁ SQLITE_OK; }
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା debugMutexEnd(ଶୂନ୍ୟ){ ଫେରନ୍ତୁ SQLITE_OK; }

/*
** The sqlite3_mutex_alloc() routine allocates a new
** mutex and returns a pointer to it.  If it returns NULL
** that means that a mutex could not be allocated. 
*/
ସ୍ଥିର sqlite3_mutex *debugMutexAlloc(ପୂର୍ଣ୍ଣସଂଖ୍ୟା id){
  ସ୍ଥିର sqlite3_debug_mutex aStatic[SQLITE_MUTEX_STATIC_VFS3 - 1];
  sqlite3_debug_mutex *pNew = 0;
  ବାଛ( id ){
    ସ୍ଥିତି SQLITE_MUTEX_FAST:
    ସ୍ଥିତି SQLITE_MUTEX_RECURSIVE: {
      pNew = sqlite3Malloc(ଆକାର(*pNew));
      ଯଦି( pNew ){
        pNew->id = id;
        pNew->cnt = 0;
      }
      ଭାଙ୍ଗ;
    }
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {
#ifdef SQLITE_ENABLE_API_ARMOR
      ଯଦି( id-2<0 || id-2>=ArraySize(aStatic) ){
        (ଶୂନ୍ୟ)SQLITE_MISUSE_BKPT;
        ଫେରନ୍ତୁ 0;
      }
#endif
      pNew = &aStatic[id-2];
      pNew->id = id;
      ଭାଙ୍ଗ;
    }
  }
  ଫେରନ୍ତୁ (sqlite3_mutex*)pNew;
}

/*
** This routine deallocates a previously allocated mutex.
*/
ସ୍ଥିର ଶୂନ୍ୟ debugMutexFree(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  ପ୍ରତିଜ୍ଞା( p->cnt==0 );
  ଯଦି( p->id==SQLITE_MUTEX_RECURSIVE || p->id==SQLITE_MUTEX_FAST ){
    sqlite3_free(p);
  }ଅନ୍ୟଥା{
#ifdef SQLITE_ENABLE_API_ARMOR
    (ଶୂନ୍ୟ)SQLITE_MISUSE_BKPT;
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
ସ୍ଥିର ଶୂନ୍ୟ debugMutexEnter(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  ପ୍ରତିଜ୍ଞା( p->id==SQLITE_MUTEX_RECURSIVE || debugMutexNotheld(pX) );
  p->cnt++;
}
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା debugMutexTry(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  ପ୍ରତିଜ୍ଞା( p->id==SQLITE_MUTEX_RECURSIVE || debugMutexNotheld(pX) );
  p->cnt++;
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** The sqlite3_mutex_leave() routine exits a mutex that was
** previously entered by the same thread.  The behavior
** is undefined if the mutex is not currently entered or
** is not currently allocated.  SQLite will never do either.
*/
ସ୍ଥିର ଶୂନ୍ୟ debugMutexLeave(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  ପ୍ରତିଜ୍ଞା( debugMutexHeld(pX) );
  p->cnt--;
  ପ୍ରତିଜ୍ଞା( p->id==SQLITE_MUTEX_RECURSIVE || debugMutexNotheld(pX) );
}

sqlite3_mutex_methods ଅଚଳ *sqlite3NoopMutex(ଶୂନ୍ୟ){
  ସ୍ଥିର ଅଚଳ sqlite3_mutex_methods sMutex = {
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

  ଫେରନ୍ତୁ &sMutex;
}
#endif /* SQLITE_DEBUG */

/*
** If compiled with SQLITE_MUTEX_NOOP, then the no-op mutex implementation
** is used regardless of the run-time threadsafety setting.
*/
#ifdef SQLITE_MUTEX_NOOP
sqlite3_mutex_methods ଅଚଳ *sqlite3DefaultMutex(ଶୂନ୍ୟ){
  ଫେରନ୍ତୁ sqlite3NoopMutex();
}
#endif /* defined(SQLITE_MUTEX_NOOP) */
#endif /* !defined(SQLITE_MUTEX_OMIT) */
