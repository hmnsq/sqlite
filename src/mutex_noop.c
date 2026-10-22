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

#اگر_غیر_تعریف_شدہ SQLITE_MUTEX_OMIT

#اگر_غیر_تعریف_شدہ SQLITE_DEBUG
/*
** Stub routines for all mutex methods.
**
** This routines provide no mutual exclusion or error checking.
*/
ساکن صحیح noopMutexInit(خالی){ واپس SQLITE_OK; }
ساکن صحیح noop_Mutex_انتہا(خالی){ واپس SQLITE_OK; }
ساکن sqlite3_mutex *noopMutexAlloc(صحیح id){ 
  غیر_مستعمل_PARAMETER(id);
  واپس (sqlite3_mutex*)8; 
}
ساکن خالی noopMutexFree(sqlite3_mutex *p){ غیر_مستعمل_PARAMETER(p); واپس; }
ساکن خالی noopMutexEnter(sqlite3_mutex *p){ غیر_مستعمل_PARAMETER(p); واپس; }
ساکن صحیح noopMutexTry(sqlite3_mutex *p){
  غیر_مستعمل_PARAMETER(p);
  واپس SQLITE_OK;
}
ساکن خالی noopMutexLeave(sqlite3_mutex *p){ غیر_مستعمل_PARAMETER(p); واپس; }

sqlite3_mutex_methods ثابت *sqlite3NoopMutex(خالی){
  ساکن ثابت sqlite3_mutex_methods sMutex = {
    noopMutexInit,
    noop_Mutex_انتہا,
    noopMutexAlloc,
    noopMutexFree,
    noopMutexEnter,
    noopMutexTry,
    noopMutexLeave,

    0,
    0,
  };

  واپس &sMutex;
}
#اگر_ختم /* !SQLITE_DEBUG */

#اگر_تعریف_شدہ SQLITE_DEBUG
/*
** In this implementation, error checking is provided for testing
** and debugging purposes.  The mutexes still do not provide any
** mutual exclusion.
*/

/*
** The mutex object
*/
قسمتعریف ساخت sqlite3_debug_mutex {
  صحیح id;     /* The mutex type */
  صحیح cnt;    /* Number of entries without a matching leave */
} sqlite3_debug_mutex;

/*
** The sqlite3_mutex_held() and sqlite3_mutex_notheld() routine are
** intended for use inside assert() statements.
*/
ساکن صحیح debugMutexHeld(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  واپس p==0 || p->cnt>0;
}
ساکن صحیح debugMutexNotheld(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  واپس p==0 || p->cnt==0;
}

/*
** Initialize and deinitialize the mutex subsystem.
*/
ساکن صحیح debugMutexInit(خالی){ واپس SQLITE_OK; }
ساکن صحیح debug_Mutex_انتہا(خالی){ واپس SQLITE_OK; }

/*
** The sqlite3_mutex_alloc() routine allocates a new
** mutex and returns a pointer to it.  If it returns NULL
** that means that a mutex could not be allocated. 
*/
ساکن sqlite3_mutex *debugMutexAlloc(صحیح id){
  ساکن sqlite3_debug_mutex aStatic[SQLITE_MUTEX_STATIC_VFS3 - 1];
  sqlite3_debug_mutex *pNew = 0;
  انتخاب( id ){
    حالت SQLITE_MUTEX_FAST:
    حالت SQLITE_MUTEX_RECURSIVE: {
      pNew = sqlite3Malloc(حجم(*pNew));
      اگر( pNew ){
        pNew->id = id;
        pNew->cnt = 0;
      }
      توڑو;
    }
    طےشدہ: {
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
      اگر( id-2<0 || id-2>=Array_سائز(aStatic) ){
        (خالی)SQLITE_MISUSE_BKPT;
        واپس 0;
      }
#اگر_ختم
      pNew = &aStatic[id-2];
      pNew->id = id;
      توڑو;
    }
  }
  واپس (sqlite3_mutex*)pNew;
}

/*
** This routine deallocates a previously allocated mutex.
*/
ساکن خالی debugMutexFree(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  أكد( p->cnt==0 );
  اگر( p->id==SQLITE_MUTEX_RECURSIVE || p->id==SQLITE_MUTEX_FAST ){
    sqlite3_free(p);
  }ورنہ{
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
    (خالی)SQLITE_MISUSE_BKPT;
#اگر_ختم
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
ساکن خالی debugMutexEnter(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  أكد( p->id==SQLITE_MUTEX_RECURSIVE || debugMutexNotheld(pX) );
  p->cnt++;
}
ساکن صحیح debugMutexTry(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  أكد( p->id==SQLITE_MUTEX_RECURSIVE || debugMutexNotheld(pX) );
  p->cnt++;
  واپس SQLITE_OK;
}

/*
** The sqlite3_mutex_leave() routine exits a mutex that was
** previously entered by the same thread.  The behavior
** is undefined if the mutex is not currently entered or
** is not currently allocated.  SQLite will never do either.
*/
ساکن خالی debugMutexLeave(sqlite3_mutex *pX){
  sqlite3_debug_mutex *p = (sqlite3_debug_mutex*)pX;
  أكد( debugMutexHeld(pX) );
  p->cnt--;
  أكد( p->id==SQLITE_MUTEX_RECURSIVE || debugMutexNotheld(pX) );
}

sqlite3_mutex_methods ثابت *sqlite3NoopMutex(خالی){
  ساکن ثابت sqlite3_mutex_methods sMutex = {
    debugMutexInit,
    debug_Mutex_انتہا,
    debugMutexAlloc,
    debugMutexFree,
    debugMutexEnter,
    debugMutexTry,
    debugMutexLeave,

    debugMutexHeld,
    debugMutexNotheld
  };

  واپس &sMutex;
}
#اگر_ختم /* SQLITE_DEBUG */

/*
** If compiled with SQLITE_MUTEX_NOOP, then the no-op mutex implementation
** is used regardless of the run-time threadsafety setting.
*/
#اگر_تعریف_شدہ SQLITE_MUTEX_NOOP
sqlite3_mutex_methods ثابت *sqlite3DefaultMutex(خالی){
  واپس sqlite3NoopMutex();
}
#اگر_ختم /* defined(SQLITE_MUTEX_NOOP) */
#اگر_ختم /* !defined(SQLITE_MUTEX_OMIT) */
