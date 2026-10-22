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
#અન્તર્ભાવ "sqliteInt.h"

#ifndef SQLITE_MUTEX_OMIT

#ifndef SQLITE_DEBUG
/*
** Stub routines for all mutex methods.
**
** This routines provide no mutual exclusion or error checking.
*/
સ્થિર પૂર્ણાંક noopMutexInit(શૂન્ય){ પાછા SQLITE_OK; }
સ્થિર પૂર્ણાંક noopMutexEnd(શૂન્ય){ પાછા SQLITE_OK; }
સ્થિર sqlite3_mutex *noopMutexAlloc(પૂર્ણાંક id){ 
  UNUSED_PARAMETER(id);
  પાછા (sqlite3_mutex*)8; 
}
સ્થિર શૂન્ય noopMutexFree(sqlite3_mutex *p){ UNUSED_PARAMETER(p); પાછા; }
સ્થિર શૂન્ય noopMutexEnter(sqlite3_mutex *p){ UNUSED_PARAMETER(p); પાછા; }
સ્થિર પૂર્ણાંક noopMutexTry(sqlite3_mutex *p){
  UNUSED_PARAMETER(p);
  પાછા SQLITE_OK;
}
સ્થિર શૂન્ય noopMutexLeave(sqlite3_mutex *p){ UNUSED_PARAMETER(p); પાછા; }

sqlite3_mutex_methods અચલ *sqlite3NoopMutex(શૂન્ય){
  સ્થિર અચલ sqlite3_mutex_methods sMutex = {
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

  પાછા &sMutex;
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
પ્રકારવ્યાખ્યા સંરચના sqlite3_debug_mutex {
  પૂર્ણાંક id;     /* The mutex type */
  પૂર્ણાંક cnt;    /* Number of entries without a matching leave */
} sqlite3_debug_mutex;

/*
** The sqlite3_mutex_held() and sqlite3_mutex_notheld() routine are
** intended for use inside assert() statements.
*/
સ્થિર પૂર્ણાંક debugMutexHeld(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  પાછા p==0 || p->cnt>0;
}
સ્થિર પૂર્ણાંક debugMutexNotheld(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  પાછા p==0 || p->cnt==0;
}

/*
** Initialize and deinitialize the mutex subsystem.
*/
સ્થિર પૂર્ણાંક debugMutexInit(શૂન્ય){ પાછા SQLITE_OK; }
સ્થિર પૂર્ણાંક debugMutexEnd(શૂન્ય){ પાછા SQLITE_OK; }

/*
** The sqlite3_mutex_alloc() routine allocates a new
** mutex and returns a pointer to it.  If it returns NULL
** that means that a mutex could not be allocated. 
*/
સ્થિર sqlite3_mutex *debugMutexAlloc(પૂર્ણાંક id){
  સ્થિર sqlite3_debug_mutex aStatic[SQLITE_MUTEX_STATIC_VFS3 - 1];
  sqlite3_debug_mutex *pNew = 0;
  પસંદગી( id ){
    સ્થિતિ SQLITE_MUTEX_FAST:
    સ્થિતિ SQLITE_MUTEX_RECURSIVE: {
      pNew = sqlite3Malloc(કદ(*pNew));
      જો( pNew ){
        pNew->id = id;
        pNew->cnt = 0;
      }
      તોડો;
    }
    મૂળભૂત: {
#ifdef SQLITE_ENABLE_API_ARMOR
      જો( id-2<0 || id-2>=ArraySize(aStatic) ){
        (શૂન્ય)SQLITE_MISUSE_BKPT;
        પાછા 0;
      }
#endif
      pNew = &aStatic[id-2];
      pNew->id = id;
      તોડો;
    }
  }
  પાછા (sqlite3_mutex*)pNew;
}

/*
** This routine deallocates a previously allocated mutex.
*/
સ્થિર શૂન્ય debugMutexFree(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  પ્રતિજ્ઞા( p->cnt==0 );
  જો( p->id==SQLITE_MUTEX_RECURSIVE || p->id==SQLITE_MUTEX_FAST ){
    sqlite3_free(p);
  }નહીંતર{
#ifdef SQLITE_ENABLE_API_ARMOR
    (શૂન્ય)SQLITE_MISUSE_BKPT;
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
સ્થિર શૂન્ય debugMutexEnter(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  પ્રતિજ્ઞા( p->id==SQLITE_MUTEX_RECURSIVE || debugMutexNotheld(pX) );
  p->cnt++;
}
સ્થિર પૂર્ણાંક debugMutexTry(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  પ્રતિજ્ઞા( p->id==SQLITE_MUTEX_RECURSIVE || debugMutexNotheld(pX) );
  p->cnt++;
  પાછા SQLITE_OK;
}

/*
** The sqlite3_mutex_leave() routine exits a mutex that was
** previously entered by the same thread.  The behavior
** is undefined if the mutex is not currently entered or
** is not currently allocated.  SQLite will never do either.
*/
સ્થિર શૂન્ય debugMutexLeave(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  પ્રતિજ્ઞા( debugMutexHeld(pX) );
  p->cnt--;
  પ્રતિજ્ઞા( p->id==SQLITE_MUTEX_RECURSIVE || debugMutexNotheld(pX) );
}

sqlite3_mutex_methods અચલ *sqlite3NoopMutex(શૂન્ય){
  સ્થિર અચલ sqlite3_mutex_methods sMutex = {
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

  પાછા &sMutex;
}
#endif /* SQLITE_DEBUG */

/*
** If compiled with SQLITE_MUTEX_NOOP, then the no-op mutex implementation
** is used regardless of the run-time threadsafety setting.
*/
#ifdef SQLITE_MUTEX_NOOP
sqlite3_mutex_methods અચલ *sqlite3DefaultMutex(શૂન્ય){
  પાછા sqlite3NoopMutex();
}
#endif /* defined(SQLITE_MUTEX_NOOP) */
#endif /* !defined(SQLITE_MUTEX_OMIT) */
