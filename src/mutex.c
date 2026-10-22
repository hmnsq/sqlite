/*
** 2007 August 14
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
** This file contains code that is common across all mutex implementations.
*/
#ضمن "sqliteInt.h"

#אם defined(SQLITE_DEBUG) && !defined(SQLITE_MUTEX_OMIT)
/*
** For debugging purposes, record when the mutex subsystem is initialized
** and uninitialized so that we can assert() if there is an attempt to
** allocate a mutex while the system is uninitialized.
*/
ساكن SQLITE_WSD صحيح mutexIsInit = 0;
#endif /* SQLITE_DEBUG && !defined(SQLITE_MUTEX_OMIT) */


#ifndef SQLITE_MUTEX_OMIT

#ifdef SQLITE_THREAD_MISUSE_WARNINGS
/*
** This block (enclosed by SQLITE_THREAD_MISUSE_WARNINGS) contains
** the implementation of a wrapper around the system default mutex
** implementation (sqlite3DefaultMutex()). 
**
** Most calls are passed directly through to the underlying default
** mutex implementation. Except, if a mutex is configured by calling
** sqlite3MutexWarnOnContention() on it, then if contention is ever
** encountered within xMutexEnter() then a warning is emitted via
** sqlite3_log().  Furthermore, if SQLITE_THREAD_MISUSE_ABORT is
** defined then abort() is called after the sqlite3_log() warning.
**
** This type of mutex is used on the database handle mutex when testing
** apps that usually use SQLITE_CONFIG_MULTITHREAD mode.  A failure
** indicates that the app ought to be using SQLITE_OPEN_FULLMUTEX or
** similar because it is trying to use the same database handle from
** two different connections at the same time.
*/

/* 
** Type for all mutexes used when SQLITE_THREAD_MISUSE_WARNINGS
** is defined. Variable CheckMutex.mutex is a pointer to the real mutex
** allocated by the system mutex implementation. Variable iType is usually set
** to the type of mutex requested - SQLITE_MUTEX_RECURSIVE, SQLITE_MUTEX_FAST
** or one of the static mutex identifiers. Or, if this is a recursive mutex
** that has been configured using sqlite3MutexWarnOnContention(), it is
** set to SQLITE_MUTEX_WARNONCONTENTION.
*/
تعريفنوع بنية CheckMutex CheckMutex;
بنية CheckMutex {
  صحيح iType;
  sqlite3_mutex *mutex;
};

#define SQLITE_MUTEX_WARNONCONTENTION  (-1)

/* 
** Pointer to real mutex methods object used by the CheckMutex
** implementation. Set by checkMutexInit(). 
*/
ساكن SQLITE_WSD ثابت sqlite3_mutex_methods *pGlobalMutexMethods;

#ifdef SQLITE_DEBUG
ساكن صحيح checkMutexHeld(sqlite3_mutex *p){
  החזר pGlobalMutexMethods->xMutexHeld(((CheckMutex*)p)->mutex);
}
ساكن صحيح checkMutexNotheld(sqlite3_mutex *p){
  החזר pGlobalMutexMethods->xMutexNotheld(((CheckMutex*)p)->mutex);
}
#endif

/*
** Initialize and deinitialize the mutex subsystem.
*/
ساكن صحيح checkMutexInit(فراغ){ 
  pGlobalMutexMethods = sqlite3DefaultMutex();
  החזר pGlobalMutexMethods->xMutexInit(); 
}
ساكن صحيح checkMutexEnd(فراغ){ 
  صحيح rc = pGlobalMutexMethods->xMutexEnd(); 
  pGlobalMutexMethods = 0;
  החזר rc;
}

/*
** Allocate a mutex.
*/
ساكن sqlite3_mutex *checkMutexAlloc(صحيح iType){
  ساكن CheckMutex staticMutexes[] = {
    {2, 0}, {3, 0}, {4, 0}, {5, 0},
    {6, 0}, {7, 0}, {8, 0}, {9, 0},
    {10, 0}, {11, 0}, {12, 0}, {13, 0}
  };
  CheckMutex *p = 0;

  أكد( SQLITE_MUTEX_RECURSIVE==1 && SQLITE_MUTEX_FAST==0 );
  אם( iType<2 ){
    p = sqlite3MallocZero(حجم(CheckMutex));
    אם( p==0 ) החזר 0;
    p->iType = iType;
  }אחרת{
#ifdef SQLITE_ENABLE_API_ARMOR
    אם( iType-2>=ArraySize(staticMutexes) ){
      (فراغ)SQLITE_MISUSE_BKPT;
      החזר 0;
    }
#endif
    p = &staticMutexes[iType-2];
  }

  אם( p->mutex==0 ){
    p->mutex = pGlobalMutexMethods->xMutexAlloc(iType);
    אם( p->mutex==0 ){
      אם( iType<2 ){
        sqlite3_free(p);
      }
      p = 0;
    }
  }

  החזר (sqlite3_mutex*)p;
}

/*
** Free a mutex.
*/
ساكن فراغ checkMutexFree(sqlite3_mutex *p){
  أكد( SQLITE_MUTEX_RECURSIVE<2 );
  أكد( SQLITE_MUTEX_FAST<2 );
  أكد( SQLITE_MUTEX_WARNONCONTENTION<2 );

#ifdef SQLITE_ENABLE_API_ARMOR
  אם( ((CheckMutex*)p)->iType<2 )
#endif
  {
    CheckMutex *pCheck = (CheckMutex*)p;
    pGlobalMutexMethods->xMutexFree(pCheck->mutex);
    sqlite3_free(pCheck);
  }
#ifdef SQLITE_ENABLE_API_ARMOR
  אחרת{
    (فراغ)SQLITE_MISUSE_BKPT;
  }
#endif
}

/*
** Enter the mutex.
*/
ساكن فراغ checkMutexEnter(sqlite3_mutex *p){
  CheckMutex *pCheck = (CheckMutex*)p;
  אם( pCheck->iType==SQLITE_MUTEX_WARNONCONTENTION ){
    אם( SQLITE_OK==pGlobalMutexMethods->xMutexTry(pCheck->mutex) ){
      החזר;
    }
    sqlite3_log(SQLITE_MISUSE, 
        "illegal multi-threaded access to database connection"
    );
#אם SQLITE_THREAD_MISUSE_ABORT
    abort();
#endif
  }
  pGlobalMutexMethods->xMutexEnter(pCheck->mutex);
}

/*
** Enter the mutex (do not block).
*/
ساكن صحيح checkMutexTry(sqlite3_mutex *p){
  CheckMutex *pCheck = (CheckMutex*)p;
  החזר pGlobalMutexMethods->xMutexTry(pCheck->mutex);
}

/*
** Leave the mutex.
*/
ساكن فراغ checkMutexLeave(sqlite3_mutex *p){
  CheckMutex *pCheck = (CheckMutex*)p;
  pGlobalMutexMethods->xMutexLeave(pCheck->mutex);
}

sqlite3_mutex_methods ثابت *multiThreadedCheckMutex(فراغ){
  ساكن ثابت sqlite3_mutex_methods sMutex = {
    checkMutexInit,
    checkMutexEnd,
    checkMutexAlloc,
    checkMutexFree,
    checkMutexEnter,
    checkMutexTry,
    checkMutexLeave,
#ifdef SQLITE_DEBUG
    checkMutexHeld,
    checkMutexNotheld
#אחרת
    0,
    0
#endif
  };
  החזר &sMutex;
}

/*
** Mark the SQLITE_MUTEX_RECURSIVE mutex passed as the only argument as
** one on which there should be no contention.
*/
فراغ sqlite3MutexWarnOnContention(sqlite3_mutex *p){
  אם( sqlite3GlobalConfig.mutex.xMutexAlloc==checkMutexAlloc ){
    CheckMutex *pCheck = (CheckMutex*)p;
    أكد( pCheck->iType==SQLITE_MUTEX_RECURSIVE );
    pCheck->iType = SQLITE_MUTEX_WARNONCONTENTION;
  }
}
#endif   /* ifdef SQLITE_THREAD_MISUSE_WARNINGS */

/*
** Initialize the mutex system.
*/
صحيح sqlite3MutexInit(فراغ){ 
  صحيح rc = SQLITE_OK;
  אם( !sqlite3GlobalConfig.mutex.xMutexAlloc ){
    /* If the xMutexAlloc method has not been set, then the user did not
    ** install a mutex implementation via sqlite3_config() prior to 
    ** sqlite3_initialize() being called. This block copies pointers to
    ** the default implementation into the sqlite3GlobalConfig structure.
    */
    sqlite3_mutex_methods ثابت *pFrom;
    sqlite3_mutex_methods *pTo = &sqlite3GlobalConfig.mutex;

    אם( sqlite3GlobalConfig.bCoreMutex ){
#ifdef SQLITE_THREAD_MISUSE_WARNINGS
      pFrom = multiThreadedCheckMutex();
#אחרת
      pFrom = sqlite3DefaultMutex();
#endif
    }אחרת{
      pFrom = sqlite3NoopMutex();
    }
    pTo->xMutexInit = pFrom->xMutexInit;
    pTo->xMutexEnd = pFrom->xMutexEnd;
    pTo->xMutexFree = pFrom->xMutexFree;
    pTo->xMutexEnter = pFrom->xMutexEnter;
    pTo->xMutexTry = pFrom->xMutexTry;
    pTo->xMutexLeave = pFrom->xMutexLeave;
    pTo->xMutexHeld = pFrom->xMutexHeld;
    pTo->xMutexNotheld = pFrom->xMutexNotheld;
    sqlite3MemoryBarrier();
    pTo->xMutexAlloc = pFrom->xMutexAlloc;
  }אחרת{
    /* Another thread may have just installed the default methods above.
    ** This barrier pairs with the one prior to setting xMutexAlloc, so
    ** that the load of xMutexInit below cannot be satisfied before the
    ** load of xMutexAlloc on processors with weak memory ordering. */
    sqlite3MemoryBarrier();
  }
  أكد( sqlite3GlobalConfig.mutex.xMutexInit );
  rc = sqlite3GlobalConfig.mutex.xMutexInit();

#ifdef SQLITE_DEBUG
  GLOBAL(صحيح, mutexIsInit) = 1;
#endif

  sqlite3MemoryBarrier();
  החזר rc;
}

/*
** Shutdown the mutex system. This call frees resources allocated by
** sqlite3MutexInit().
*/
صحيح sqlite3MutexEnd(فراغ){
  صحيح rc = SQLITE_OK;
  אם( sqlite3GlobalConfig.mutex.xMutexEnd ){
    rc = sqlite3GlobalConfig.mutex.xMutexEnd();
  }

#ifdef SQLITE_DEBUG
  GLOBAL(صحيح, mutexIsInit) = 0;
#endif

  החזר rc;
}

/*
** Retrieve a pointer to a static mutex or allocate a new dynamic one.
*/
sqlite3_mutex *sqlite3_mutex_alloc(صحيح id){
#ifndef SQLITE_OMIT_AUTOINIT
  אם( id<=SQLITE_MUTEX_RECURSIVE && sqlite3_initialize() ) החזר 0;
  אם( id>SQLITE_MUTEX_RECURSIVE && sqlite3MutexInit() ) החזר 0;
#endif
  أكد( sqlite3GlobalConfig.mutex.xMutexAlloc );
  החזר sqlite3GlobalConfig.mutex.xMutexAlloc(id);
}

sqlite3_mutex *sqlite3MutexAlloc(صحيح id){
  אם( !sqlite3GlobalConfig.bCoreMutex ){
    החזר 0;
  }
  أكد( GLOBAL(صحيح, mutexIsInit) );
  أكد( sqlite3GlobalConfig.mutex.xMutexAlloc );
  החזר sqlite3GlobalConfig.mutex.xMutexAlloc(id);
}

/*
** Free a dynamic mutex.
*/
فراغ sqlite3_mutex_free(sqlite3_mutex *p){
  אם( p ){
    أكد( sqlite3GlobalConfig.mutex.xMutexFree );
    sqlite3GlobalConfig.mutex.xMutexFree(p);
  }
}

/*
** Obtain the mutex p. If some other thread already has the mutex, block
** until it can be obtained.
*/
فراغ sqlite3_mutex_enter(sqlite3_mutex *p){
  אם( p ){
    أكد( sqlite3GlobalConfig.mutex.xMutexEnter );
    sqlite3GlobalConfig.mutex.xMutexEnter(p);
  }
}

/*
** Obtain the mutex p. If successful, return SQLITE_OK. Otherwise, if another
** thread holds the mutex and it cannot be obtained, return SQLITE_BUSY.
*/
صحيح sqlite3_mutex_try(sqlite3_mutex *p){
  صحيح rc = SQLITE_OK;
  אם( p ){
    أكد( sqlite3GlobalConfig.mutex.xMutexTry );
    החזר sqlite3GlobalConfig.mutex.xMutexTry(p);
  }
  החזר rc;
}

/*
** The sqlite3_mutex_leave() routine exits a mutex that was previously
** entered by the same thread.  The behavior is undefined if the mutex 
** is not currently entered. If a NULL pointer is passed as an argument
** this function is a no-op.
*/
فراغ sqlite3_mutex_leave(sqlite3_mutex *p){
  אם( p ){
    أكد( sqlite3GlobalConfig.mutex.xMutexLeave );
    sqlite3GlobalConfig.mutex.xMutexLeave(p);
  }
}

#ifndef NDEBUG
/*
** The sqlite3_mutex_held() and sqlite3_mutex_notheld() routine are
** intended for use inside assert() statements.
**
** Because these routines raise false-positive alerts in TSAN, disable
** them (make them always return 1) when compiling with TSAN.
*/
صحيح sqlite3_mutex_held(sqlite3_mutex *p){
# אם defined(__has_feature)
#   אם __has_feature(thread_sanitizer)
      p = 0;
#   endif
# endif
  أكد( p==0 || sqlite3GlobalConfig.mutex.xMutexHeld );
  החזר p==0 || sqlite3GlobalConfig.mutex.xMutexHeld(p);
}
صحيح sqlite3_mutex_notheld(sqlite3_mutex *p){
# אם defined(__has_feature)
#   אם __has_feature(thread_sanitizer)
      p = 0;
#   endif
# endif
  أكد( p==0 || sqlite3GlobalConfig.mutex.xMutexNotheld );
  החזר p==0 || sqlite3GlobalConfig.mutex.xMutexNotheld(p);
}
#endif /* NDEBUG */

#endif /* !defined(SQLITE_MUTEX_OMIT) */
