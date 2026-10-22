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
#অন্তর্ভাব "sqliteInt.h"

#ifndef SQLITE_MUTEX_OMIT

#ifndef SQLITE_DEBUG
/*
** Stub routines for all mutex methods.
**
** This routines provide no mutual exclusion or error checking.
*/
স্থির পূর্ণাঙ্ক noopMutexInit(শূন্য){ প্রত্যাগম SQLITE_OK; }
স্থির পূর্ণাঙ্ক noopMutexEnd(শূন্য){ প্রত্যাগম SQLITE_OK; }
স্থির sqlite3_mutex *noopMutexAlloc(পূর্ণাঙ্ক id){ 
  UNUSED_PARAMETER(id);
  প্রত্যাগম (sqlite3_mutex*)8; 
}
স্থির শূন্য noopMutexFree(sqlite3_mutex *p){ UNUSED_PARAMETER(p); প্রত্যাগম; }
স্থির শূন্য noopMutexEnter(sqlite3_mutex *p){ UNUSED_PARAMETER(p); প্রত্যাগম; }
স্থির পূর্ণাঙ্ক noopMutexTry(sqlite3_mutex *p){
  UNUSED_PARAMETER(p);
  প্রত্যাগম SQLITE_OK;
}
স্থির শূন্য noopMutexLeave(sqlite3_mutex *p){ UNUSED_PARAMETER(p); প্রত্যাগম; }

sqlite3_mutex_methods ধ্রুব *sqlite3NoopMutex(শূন্য){
  স্থির ধ্রুব sqlite3_mutex_methods sMutex = {
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

  প্রত্যাগম &sMutex;
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
প্রকারনির্দেশ সংরচনা sqlite3_debug_mutex {
  পূর্ণাঙ্ক id;     /* The mutex type */
  পূর্ণাঙ্ক cnt;    /* Number of entries without a matching leave */
} sqlite3_debug_mutex;

/*
** The sqlite3_mutex_held() and sqlite3_mutex_notheld() routine are
** intended for use inside assert() statements.
*/
স্থির পূর্ণাঙ্ক debugMutexHeld(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  প্রত্যাগম p==0 || p->cnt>0;
}
স্থির পূর্ণাঙ্ক debugMutexNotheld(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  প্রত্যাগম p==0 || p->cnt==0;
}

/*
** Initialize and deinitialize the mutex subsystem.
*/
স্থির পূর্ণাঙ্ক debugMutexInit(শূন্য){ প্রত্যাগম SQLITE_OK; }
স্থির পূর্ণাঙ্ক debugMutexEnd(শূন্য){ প্রত্যাগম SQLITE_OK; }

/*
** The sqlite3_mutex_alloc() routine allocates a new
** mutex and returns a pointer to it.  If it returns NULL
** that means that a mutex could not be allocated. 
*/
স্থির sqlite3_mutex *debugMutexAlloc(পূর্ণাঙ্ক id){
  স্থির sqlite3_debug_mutex aStatic[SQLITE_MUTEX_STATIC_VFS3 - 1];
  sqlite3_debug_mutex *pNew = 0;
  বিকল্প( id ){
    পক্ষ SQLITE_MUTEX_FAST:
    পক্ষ SQLITE_MUTEX_RECURSIVE: {
      pNew = sqlite3Malloc(পরিমাণ(*pNew));
      যদি( pNew ){
        pNew->id = id;
        pNew->cnt = 0;
      }
      ভঙ্গ;
    }
    স্বভাব: {
#ifdef SQLITE_ENABLE_API_ARMOR
      যদি( id-2<0 || id-2>=ArraySize(aStatic) ){
        (শূন্য)SQLITE_MISUSE_BKPT;
        প্রত্যাগম 0;
      }
#endif
      pNew = &aStatic[id-2];
      pNew->id = id;
      ভঙ্গ;
    }
  }
  প্রত্যাগম (sqlite3_mutex*)pNew;
}

/*
** This routine deallocates a previously allocated mutex.
*/
স্থির শূন্য debugMutexFree(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  প্রতিজ্ঞা( p->cnt==0 );
  যদি( p->id==SQLITE_MUTEX_RECURSIVE || p->id==SQLITE_MUTEX_FAST ){
    sqlite3_free(p);
  }অন্যথা{
#ifdef SQLITE_ENABLE_API_ARMOR
    (শূন্য)SQLITE_MISUSE_BKPT;
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
স্থির শূন্য debugMutexEnter(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  প্রতিজ্ঞা( p->id==SQLITE_MUTEX_RECURSIVE || debugMutexNotheld(pX) );
  p->cnt++;
}
স্থির পূর্ণাঙ্ক debugMutexTry(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  প্রতিজ্ঞা( p->id==SQLITE_MUTEX_RECURSIVE || debugMutexNotheld(pX) );
  p->cnt++;
  প্রত্যাগম SQLITE_OK;
}

/*
** The sqlite3_mutex_leave() routine exits a mutex that was
** previously entered by the same thread.  The behavior
** is undefined if the mutex is not currently entered or
** is not currently allocated.  SQLite will never do either.
*/
স্থির শূন্য debugMutexLeave(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  প্রতিজ্ঞা( debugMutexHeld(pX) );
  p->cnt--;
  প্রতিজ্ঞা( p->id==SQLITE_MUTEX_RECURSIVE || debugMutexNotheld(pX) );
}

sqlite3_mutex_methods ধ্রুব *sqlite3NoopMutex(শূন্য){
  স্থির ধ্রুব sqlite3_mutex_methods sMutex = {
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

  প্রত্যাগম &sMutex;
}
#endif /* SQLITE_DEBUG */

/*
** If compiled with SQLITE_MUTEX_NOOP, then the no-op mutex implementation
** is used regardless of the run-time threadsafety setting.
*/
#ifdef SQLITE_MUTEX_NOOP
sqlite3_mutex_methods ধ্রুব *sqlite3DefaultMutex(শূন্য){
  প্রত্যাগম sqlite3NoopMutex();
}
#endif /* defined(SQLITE_MUTEX_NOOP) */
#endif /* !defined(SQLITE_MUTEX_OMIT) */
