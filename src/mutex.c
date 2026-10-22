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
#شامل "sqliteInt.h"

#اگر defined(SQLITE_DEBUG) && !defined(SQLITE_MUTEX_OMIT)
/*
** For debugging purposes, record when the mutex subsystem is initialized
** and uninitialized so that we can assert() if there is an attempt to
** allocate a mutex while the system is uninitialized.
*/
ایستا SQLITE_WSD صحیح mutexIsInit = 0;
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
تعریفنوع ساختار CheckMutex CheckMutex;
ساختار CheckMutex {
  صحیح iType;
  sqlite3_mutex *mutex;
};

#define SQLITE_MUTEX_WARNONCONTENTION  (-1)

/* 
** Pointer to real mutex methods object used by the CheckMutex
** implementation. Set by checkMutexInit(). 
*/
ایستا SQLITE_WSD ثابت sqlite3_mutex_methods *pGlobalMutexMethods;

#ifdef SQLITE_DEBUG
ایستا صحیح checkMutexHeld(sqlite3_mutex *p){
  بازگشت pGlobalMutexMethods->xMutexHeld(((CheckMutex*)p)->mutex);
}
ایستا صحیح checkMutexNotheld(sqlite3_mutex *p){
  بازگشت pGlobalMutexMethods->xMutexNotheld(((CheckMutex*)p)->mutex);
}
#endif

/*
** Initialize and deinitialize the mutex subsystem.
*/
ایستا صحیح checkMutexInit(تهی){ 
  pGlobalMutexMethods = sqlite3DefaultMutex();
  بازگشت pGlobalMutexMethods->xMutexInit(); 
}
ایستا صحیح checkMutexEnd(تهی){ 
  صحیح rc = pGlobalMutexMethods->xMutexEnd(); 
  pGlobalMutexMethods = 0;
  بازگشت rc;
}

/*
** Allocate a mutex.
*/
ایستا sqlite3_mutex *checkMutexAlloc(صحیح iType){
  ایستا CheckMutex staticMutexes[] = {
    {2, 0}, {3, 0}, {4, 0}, {5, 0},
    {6, 0}, {7, 0}, {8, 0}, {9, 0},
    {10, 0}, {11, 0}, {12, 0}, {13, 0}
  };
  CheckMutex *p = 0;

  أكد( SQLITE_MUTEX_RECURSIVE==1 && SQLITE_MUTEX_FAST==0 );
  اگر( iType<2 ){
    p = sqlite3MallocZero(اندازه(CheckMutex));
    اگر( p==0 ) بازگشت 0;
    p->iType = iType;
  }وگرنه{
#ifdef SQLITE_ENABLE_API_ARMOR
    اگر( iType-2>=ArraySize(staticMutexes) ){
      (تهی)SQLITE_MISUSE_BKPT;
      بازگشت 0;
    }
#endif
    p = &staticMutexes[iType-2];
  }

  اگر( p->mutex==0 ){
    p->mutex = pGlobalMutexMethods->xMutexAlloc(iType);
    اگر( p->mutex==0 ){
      اگر( iType<2 ){
        sqlite3_free(p);
      }
      p = 0;
    }
  }

  بازگشت (sqlite3_mutex*)p;
}

/*
** Free a mutex.
*/
ایستا تهی checkMutexFree(sqlite3_mutex *p){
  أكد( SQLITE_MUTEX_RECURSIVE<2 );
  أكد( SQLITE_MUTEX_FAST<2 );
  أكد( SQLITE_MUTEX_WARNONCONTENTION<2 );

#ifdef SQLITE_ENABLE_API_ARMOR
  اگر( ((CheckMutex*)p)->iType<2 )
#endif
  {
    CheckMutex *pCheck = (CheckMutex*)p;
    pGlobalMutexMethods->xMutexFree(pCheck->mutex);
    sqlite3_free(pCheck);
  }
#ifdef SQLITE_ENABLE_API_ARMOR
  وگرنه{
    (تهی)SQLITE_MISUSE_BKPT;
  }
#endif
}

/*
** Enter the mutex.
*/
ایستا تهی checkMutexEnter(sqlite3_mutex *p){
  CheckMutex *pCheck = (CheckMutex*)p;
  اگر( pCheck->iType==SQLITE_MUTEX_WARNONCONTENTION ){
    اگر( SQLITE_OK==pGlobalMutexMethods->xMutexTry(pCheck->mutex) ){
      بازگشت;
    }
    sqlite3_log(SQLITE_MISUSE, 
        "illegal multi-threaded access to database connection"
    );
#اگر SQLITE_THREAD_MISUSE_ABORT
    abort();
#endif
  }
  pGlobalMutexMethods->xMutexEnter(pCheck->mutex);
}

/*
** Enter the mutex (do not block).
*/
ایستا صحیح checkMutexTry(sqlite3_mutex *p){
  CheckMutex *pCheck = (CheckMutex*)p;
  بازگشت pGlobalMutexMethods->xMutexTry(pCheck->mutex);
}

/*
** Leave the mutex.
*/
ایستا تهی checkMutexLeave(sqlite3_mutex *p){
  CheckMutex *pCheck = (CheckMutex*)p;
  pGlobalMutexMethods->xMutexLeave(pCheck->mutex);
}

sqlite3_mutex_methods ثابت *multiThreadedCheckMutex(تهی){
  ایستا ثابت sqlite3_mutex_methods sMutex = {
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
#وگرنه
    0,
    0
#endif
  };
  بازگشت &sMutex;
}

/*
** Mark the SQLITE_MUTEX_RECURSIVE mutex passed as the only argument as
** one on which there should be no contention.
*/
تهی sqlite3MutexWarnOnContention(sqlite3_mutex *p){
  اگر( sqlite3GlobalConfig.mutex.xMutexAlloc==checkMutexAlloc ){
    CheckMutex *pCheck = (CheckMutex*)p;
    أكد( pCheck->iType==SQLITE_MUTEX_RECURSIVE );
    pCheck->iType = SQLITE_MUTEX_WARNONCONTENTION;
  }
}
#endif   /* ifdef SQLITE_THREAD_MISUSE_WARNINGS */

/*
** Initialize the mutex system.
*/
صحیح sqlite3MutexInit(تهی){ 
  صحیح rc = SQLITE_OK;
  اگر( !sqlite3GlobalConfig.mutex.xMutexAlloc ){
    /* If the xMutexAlloc method has not been set, then the user did not
    ** install a mutex implementation via sqlite3_config() prior to 
    ** sqlite3_initialize() being called. This block copies pointers to
    ** the default implementation into the sqlite3GlobalConfig structure.
    */
    sqlite3_mutex_methods ثابت *pFrom;
    sqlite3_mutex_methods *pTo = &sqlite3GlobalConfig.mutex;

    اگر( sqlite3GlobalConfig.bCoreMutex ){
#ifdef SQLITE_THREAD_MISUSE_WARNINGS
      pFrom = multiThreadedCheckMutex();
#وگرنه
      pFrom = sqlite3DefaultMutex();
#endif
    }وگرنه{
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
  }وگرنه{
    /* Another thread may have just installed the default methods above.
    ** This barrier pairs with the one prior to setting xMutexAlloc, so
    ** that the load of xMutexInit below cannot be satisfied before the
    ** load of xMutexAlloc on processors with weak memory ordering. */
    sqlite3MemoryBarrier();
  }
  أكد( sqlite3GlobalConfig.mutex.xMutexInit );
  rc = sqlite3GlobalConfig.mutex.xMutexInit();

#ifdef SQLITE_DEBUG
  GLOBAL(صحیح, mutexIsInit) = 1;
#endif

  sqlite3MemoryBarrier();
  بازگشت rc;
}

/*
** Shutdown the mutex system. This call frees resources allocated by
** sqlite3MutexInit().
*/
صحیح sqlite3MutexEnd(تهی){
  صحیح rc = SQLITE_OK;
  اگر( sqlite3GlobalConfig.mutex.xMutexEnd ){
    rc = sqlite3GlobalConfig.mutex.xMutexEnd();
  }

#ifdef SQLITE_DEBUG
  GLOBAL(صحیح, mutexIsInit) = 0;
#endif

  بازگشت rc;
}

/*
** Retrieve a pointer to a static mutex or allocate a new dynamic one.
*/
sqlite3_mutex *sqlite3_mutex_alloc(صحیح id){
#ifndef SQLITE_OMIT_AUTOINIT
  اگر( id<=SQLITE_MUTEX_RECURSIVE && sqlite3_initialize() ) بازگشت 0;
  اگر( id>SQLITE_MUTEX_RECURSIVE && sqlite3MutexInit() ) بازگشت 0;
#endif
  أكد( sqlite3GlobalConfig.mutex.xMutexAlloc );
  بازگشت sqlite3GlobalConfig.mutex.xMutexAlloc(id);
}

sqlite3_mutex *sqlite3MutexAlloc(صحیح id){
  اگر( !sqlite3GlobalConfig.bCoreMutex ){
    بازگشت 0;
  }
  أكد( GLOBAL(صحیح, mutexIsInit) );
  أكد( sqlite3GlobalConfig.mutex.xMutexAlloc );
  بازگشت sqlite3GlobalConfig.mutex.xMutexAlloc(id);
}

/*
** Free a dynamic mutex.
*/
تهی sqlite3_mutex_free(sqlite3_mutex *p){
  اگر( p ){
    أكد( sqlite3GlobalConfig.mutex.xMutexFree );
    sqlite3GlobalConfig.mutex.xMutexFree(p);
  }
}

/*
** Obtain the mutex p. If some other thread already has the mutex, block
** until it can be obtained.
*/
تهی sqlite3_mutex_enter(sqlite3_mutex *p){
  اگر( p ){
    أكد( sqlite3GlobalConfig.mutex.xMutexEnter );
    sqlite3GlobalConfig.mutex.xMutexEnter(p);
  }
}

/*
** Obtain the mutex p. If successful, return SQLITE_OK. Otherwise, if another
** thread holds the mutex and it cannot be obtained, return SQLITE_BUSY.
*/
صحیح sqlite3_mutex_try(sqlite3_mutex *p){
  صحیح rc = SQLITE_OK;
  اگر( p ){
    أكد( sqlite3GlobalConfig.mutex.xMutexTry );
    بازگشت sqlite3GlobalConfig.mutex.xMutexTry(p);
  }
  بازگشت rc;
}

/*
** The sqlite3_mutex_leave() routine exits a mutex that was previously
** entered by the same thread.  The behavior is undefined if the mutex 
** is not currently entered. If a NULL pointer is passed as an argument
** this function is a no-op.
*/
تهی sqlite3_mutex_leave(sqlite3_mutex *p){
  اگر( p ){
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
صحیح sqlite3_mutex_held(sqlite3_mutex *p){
# اگر defined(__has_feature)
#   اگر __has_feature(thread_sanitizer)
      p = 0;
#   endif
# endif
  أكد( p==0 || sqlite3GlobalConfig.mutex.xMutexHeld );
  بازگشت p==0 || sqlite3GlobalConfig.mutex.xMutexHeld(p);
}
صحیح sqlite3_mutex_notheld(sqlite3_mutex *p){
# اگر defined(__has_feature)
#   اگر __has_feature(thread_sanitizer)
      p = 0;
#   endif
# endif
  أكد( p==0 || sqlite3GlobalConfig.mutex.xMutexNotheld );
  بازگشت p==0 || sqlite3GlobalConfig.mutex.xMutexNotheld(p);
}
#endif /* NDEBUG */

#endif /* !defined(SQLITE_MUTEX_OMIT) */
