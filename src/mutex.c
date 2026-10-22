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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"

#ਜੇ defined(SQLITE_DEBUG) && !defined(SQLITE_MUTEX_OMIT)
/*
** For debugging purposes, record when the mutex subsystem is initialized
** and uninitialized so that we can assert() if there is an attempt to
** allocate a mutex while the system is uninitialized.
*/
ਸਥਿਰ SQLITE_WSD ਪੂਰਨ mutexIsInit = 0;
#ਜੇ_ਅੰਤ /* SQLITE_DEBUG && !defined(SQLITE_MUTEX_OMIT) */


#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_MUTEX_OMIT

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_THREAD_MISUSE_WARNINGS
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
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ CheckMutex CheckMutex;
ਬਣਤਰ CheckMutex {
  ਪੂਰਨ iType;
  sqlite3_mutex *mutex;
};

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_MUTEX_WARNONCONTENTION  (-1)

/* 
** Pointer to real mutex methods object used by the CheckMutex
** implementation. Set by checkMutexInit(). 
*/
ਸਥਿਰ SQLITE_WSD ਅਚਲ sqlite3_mutex_methods *pGlobalMutexMethods;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
ਸਥਿਰ ਪੂਰਨ checkMutexHeld(sqlite3_mutex *p){
  ਵਾਪਸ pGlobalMutexMethods->xMutexHeld(((CheckMutex*)p)->mutex);
}
ਸਥਿਰ ਪੂਰਨ checkMutexNotheld(sqlite3_mutex *p){
  ਵਾਪਸ pGlobalMutexMethods->xMutexNotheld(((CheckMutex*)p)->mutex);
}
#ਜੇ_ਅੰਤ

/*
** Initialize and deinitialize the mutex subsystem.
*/
ਸਥਿਰ ਪੂਰਨ checkMutexInit(ਖਾਲੀ){ 
  pGlobalMutexMethods = sqlite3DefaultMutex();
  ਵਾਪਸ pGlobalMutexMethods->xMutexInit(); 
}
ਸਥਿਰ ਪੂਰਨ check_Mutex_ਸਿਰਾ(ਖਾਲੀ){ 
  ਪੂਰਨ rc = pGlobalMutexMethods->x_Mutex_ਸਿਰਾ(); 
  pGlobalMutexMethods = 0;
  ਵਾਪਸ rc;
}

/*
** Allocate a mutex.
*/
ਸਥਿਰ sqlite3_mutex *checkMutexAlloc(ਪੂਰਨ iType){
  ਸਥਿਰ CheckMutex staticMutexes[] = {
    {2, 0}, {3, 0}, {4, 0}, {5, 0},
    {6, 0}, {7, 0}, {8, 0}, {9, 0},
    {10, 0}, {11, 0}, {12, 0}, {13, 0}
  };
  CheckMutex *p = 0;

  ਪ੍ਰਤਿਜ੍ਞਾ( SQLITE_MUTEX_RECURSIVE==1 && SQLITE_MUTEX_FAST==0 );
  ਜੇ( iType<2 ){
    p = sqlite3MallocZero(ਆਕਾਰ(CheckMutex));
    ਜੇ( p==0 ) ਵਾਪਸ 0;
    p->iType = iType;
  }ਨਹੀਂਤਾਂ{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
    ਜੇ( iType-2>=Array_ਮਾਪ(staticMutexes) ){
      (ਖਾਲੀ)SQLITE_MISUSE_BKPT;
      ਵਾਪਸ 0;
    }
#ਜੇ_ਅੰਤ
    p = &staticMutexes[iType-2];
  }

  ਜੇ( p->mutex==0 ){
    p->mutex = pGlobalMutexMethods->xMutexAlloc(iType);
    ਜੇ( p->mutex==0 ){
      ਜੇ( iType<2 ){
        sqlite3_free(p);
      }
      p = 0;
    }
  }

  ਵਾਪਸ (sqlite3_mutex*)p;
}

/*
** Free a mutex.
*/
ਸਥਿਰ ਖਾਲੀ checkMutexFree(sqlite3_mutex *p){
  ਪ੍ਰਤਿਜ੍ਞਾ( SQLITE_MUTEX_RECURSIVE<2 );
  ਪ੍ਰਤਿਜ੍ਞਾ( SQLITE_MUTEX_FAST<2 );
  ਪ੍ਰਤਿਜ੍ਞਾ( SQLITE_MUTEX_WARNONCONTENTION<2 );

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( ((CheckMutex*)p)->iType<2 )
#ਜੇ_ਅੰਤ
  {
    CheckMutex *pCheck = (CheckMutex*)p;
    pGlobalMutexMethods->xMutexFree(pCheck->mutex);
    sqlite3_free(pCheck);
  }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਨਹੀਂਤਾਂ{
    (ਖਾਲੀ)SQLITE_MISUSE_BKPT;
  }
#ਜੇ_ਅੰਤ
}

/*
** Enter the mutex.
*/
ਸਥਿਰ ਖਾਲੀ checkMutexEnter(sqlite3_mutex *p){
  CheckMutex *pCheck = (CheckMutex*)p;
  ਜੇ( pCheck->iType==SQLITE_MUTEX_WARNONCONTENTION ){
    ਜੇ( SQLITE_OK==pGlobalMutexMethods->xMutexTry(pCheck->mutex) ){
      ਵਾਪਸ;
    }
    sqlite3_log(SQLITE_MISUSE, 
        "illegal multi-threaded access to database connection"
    );
#ਜੇ SQLITE_THREAD_MISUSE_ABORT
    abort();
#ਜੇ_ਅੰਤ
  }
  pGlobalMutexMethods->xMutexEnter(pCheck->mutex);
}

/*
** Enter the mutex (do not block).
*/
ਸਥਿਰ ਪੂਰਨ checkMutexTry(sqlite3_mutex *p){
  CheckMutex *pCheck = (CheckMutex*)p;
  ਵਾਪਸ pGlobalMutexMethods->xMutexTry(pCheck->mutex);
}

/*
** Leave the mutex.
*/
ਸਥਿਰ ਖਾਲੀ checkMutexLeave(sqlite3_mutex *p){
  CheckMutex *pCheck = (CheckMutex*)p;
  pGlobalMutexMethods->xMutexLeave(pCheck->mutex);
}

sqlite3_mutex_methods ਅਚਲ *multiThreadedCheckMutex(ਖਾਲੀ){
  ਸਥਿਰ ਅਚਲ sqlite3_mutex_methods sMutex = {
    checkMutexInit,
    check_Mutex_ਸਿਰਾ,
    checkMutexAlloc,
    checkMutexFree,
    checkMutexEnter,
    checkMutexTry,
    checkMutexLeave,
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
    checkMutexHeld,
    checkMutexNotheld
#ਨਹੀਂਤਾਂ
    0,
    0
#ਜੇ_ਅੰਤ
  };
  ਵਾਪਸ &sMutex;
}

/*
** Mark the SQLITE_MUTEX_RECURSIVE mutex passed as the only argument as
** one on which there should be no contention.
*/
ਖਾਲੀ sqlite3MutexWarnOnContention(sqlite3_mutex *p){
  ਜੇ( sqlite3GlobalConfig.mutex.xMutexAlloc==checkMutexAlloc ){
    CheckMutex *pCheck = (CheckMutex*)p;
    ਪ੍ਰਤਿਜ੍ਞਾ( pCheck->iType==SQLITE_MUTEX_RECURSIVE );
    pCheck->iType = SQLITE_MUTEX_WARNONCONTENTION;
  }
}
#ਜੇ_ਅੰਤ   /* ifdef SQLITE_THREAD_MISUSE_WARNINGS */

/*
** Initialize the mutex system.
*/
ਪੂਰਨ sqlite3MutexInit(ਖਾਲੀ){ 
  ਪੂਰਨ rc = SQLITE_OK;
  ਜੇ( !sqlite3GlobalConfig.mutex.xMutexAlloc ){
    /* If the xMutexAlloc method has not been set, then the user did not
    ** install a mutex implementation via sqlite3_config() prior to 
    ** sqlite3_initialize() being called. This block copies pointers to
    ** the default implementation into the sqlite3GlobalConfig structure.
    */
    sqlite3_mutex_methods ਅਚਲ *pFrom;
    sqlite3_mutex_methods *p_ਮੰਜ਼ਿਲ = &sqlite3GlobalConfig.mutex;

    ਜੇ( sqlite3GlobalConfig.bCoreMutex ){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_THREAD_MISUSE_WARNINGS
      pFrom = multiThreadedCheckMutex();
#ਨਹੀਂਤਾਂ
      pFrom = sqlite3DefaultMutex();
#ਜੇ_ਅੰਤ
    }ਨਹੀਂਤਾਂ{
      pFrom = sqlite3NoopMutex();
    }
    p_ਮੰਜ਼ਿਲ->xMutexInit = pFrom->xMutexInit;
    p_ਮੰਜ਼ਿਲ->x_Mutex_ਸਿਰਾ = pFrom->x_Mutex_ਸਿਰਾ;
    p_ਮੰਜ਼ਿਲ->xMutexFree = pFrom->xMutexFree;
    p_ਮੰਜ਼ਿਲ->xMutexEnter = pFrom->xMutexEnter;
    p_ਮੰਜ਼ਿਲ->xMutexTry = pFrom->xMutexTry;
    p_ਮੰਜ਼ਿਲ->xMutexLeave = pFrom->xMutexLeave;
    p_ਮੰਜ਼ਿਲ->xMutexHeld = pFrom->xMutexHeld;
    p_ਮੰਜ਼ਿਲ->xMutexNotheld = pFrom->xMutexNotheld;
    sqlite3_ਸਮ੍ਰਿਤੀ_Barrier();
    p_ਮੰਜ਼ਿਲ->xMutexAlloc = pFrom->xMutexAlloc;
  }ਨਹੀਂਤਾਂ{
    /* Another thread may have just installed the default methods above.
    ** This barrier pairs with the one prior to setting xMutexAlloc, so
    ** that the load of xMutexInit below cannot be satisfied before the
    ** load of xMutexAlloc on processors with weak memory ordering. */
    sqlite3_ਸਮ੍ਰਿਤੀ_Barrier();
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3GlobalConfig.mutex.xMutexInit );
  rc = sqlite3GlobalConfig.mutex.xMutexInit();

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  GLOBAL(ਪੂਰਨ, mutexIsInit) = 1;
#ਜੇ_ਅੰਤ

  sqlite3_ਸਮ੍ਰਿਤੀ_Barrier();
  ਵਾਪਸ rc;
}

/*
** Shutdown the mutex system. This call frees resources allocated by
** sqlite3MutexInit().
*/
ਪੂਰਨ sqlite3_Mutex_ਸਿਰਾ(ਖਾਲੀ){
  ਪੂਰਨ rc = SQLITE_OK;
  ਜੇ( sqlite3GlobalConfig.mutex.x_Mutex_ਸਿਰਾ ){
    rc = sqlite3GlobalConfig.mutex.x_Mutex_ਸਿਰਾ();
  }

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  GLOBAL(ਪੂਰਨ, mutexIsInit) = 0;
#ਜੇ_ਅੰਤ

  ਵਾਪਸ rc;
}

/*
** Retrieve a pointer to a static mutex or allocate a new dynamic one.
*/
sqlite3_mutex *sqlite3_mutex_alloc(ਪੂਰਨ id){
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOINIT
  ਜੇ( id<=SQLITE_MUTEX_RECURSIVE && sqlite3_initialize() ) ਵਾਪਸ 0;
  ਜੇ( id>SQLITE_MUTEX_RECURSIVE && sqlite3MutexInit() ) ਵਾਪਸ 0;
#ਜੇ_ਅੰਤ
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3GlobalConfig.mutex.xMutexAlloc );
  ਵਾਪਸ sqlite3GlobalConfig.mutex.xMutexAlloc(id);
}

sqlite3_mutex *sqlite3MutexAlloc(ਪੂਰਨ id){
  ਜੇ( !sqlite3GlobalConfig.bCoreMutex ){
    ਵਾਪਸ 0;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( GLOBAL(ਪੂਰਨ, mutexIsInit) );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3GlobalConfig.mutex.xMutexAlloc );
  ਵਾਪਸ sqlite3GlobalConfig.mutex.xMutexAlloc(id);
}

/*
** Free a dynamic mutex.
*/
ਖਾਲੀ sqlite3_mutex_free(sqlite3_mutex *p){
  ਜੇ( p ){
    ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3GlobalConfig.mutex.xMutexFree );
    sqlite3GlobalConfig.mutex.xMutexFree(p);
  }
}

/*
** Obtain the mutex p. If some other thread already has the mutex, block
** until it can be obtained.
*/
ਖਾਲੀ sqlite3_mutex_enter(sqlite3_mutex *p){
  ਜੇ( p ){
    ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3GlobalConfig.mutex.xMutexEnter );
    sqlite3GlobalConfig.mutex.xMutexEnter(p);
  }
}

/*
** Obtain the mutex p. If successful, return SQLITE_OK. Otherwise, if another
** thread holds the mutex and it cannot be obtained, return SQLITE_BUSY.
*/
ਪੂਰਨ sqlite3_mutex_try(sqlite3_mutex *p){
  ਪੂਰਨ rc = SQLITE_OK;
  ਜੇ( p ){
    ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3GlobalConfig.mutex.xMutexTry );
    ਵਾਪਸ sqlite3GlobalConfig.mutex.xMutexTry(p);
  }
  ਵਾਪਸ rc;
}

/*
** The sqlite3_mutex_leave() routine exits a mutex that was previously
** entered by the same thread.  The behavior is undefined if the mutex 
** is not currently entered. If a NULL pointer is passed as an argument
** this function is a no-op.
*/
ਖਾਲੀ sqlite3_mutex_leave(sqlite3_mutex *p){
  ਜੇ( p ){
    ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3GlobalConfig.mutex.xMutexLeave );
    sqlite3GlobalConfig.mutex.xMutexLeave(p);
  }
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NDEBUG
/*
** The sqlite3_mutex_held() and sqlite3_mutex_notheld() routine are
** intended for use inside assert() statements.
**
** Because these routines raise false-positive alerts in TSAN, disable
** them (make them always return 1) when compiling with TSAN.
*/
ਪੂਰਨ sqlite3_mutex_held(sqlite3_mutex *p){
# ਜੇ defined(__has_feature)
#   ਜੇ __has_feature(thread_sanitizer)
      p = 0;
#   endif
# endif
  ਪ੍ਰਤਿਜ੍ਞਾ( p==0 || sqlite3GlobalConfig.mutex.xMutexHeld );
  ਵਾਪਸ p==0 || sqlite3GlobalConfig.mutex.xMutexHeld(p);
}
ਪੂਰਨ sqlite3_mutex_notheld(sqlite3_mutex *p){
# ਜੇ defined(__has_feature)
#   ਜੇ __has_feature(thread_sanitizer)
      p = 0;
#   endif
# endif
  ਪ੍ਰਤਿਜ੍ਞਾ( p==0 || sqlite3GlobalConfig.mutex.xMutexNotheld );
  ਵਾਪਸ p==0 || sqlite3GlobalConfig.mutex.xMutexNotheld(p);
}
#ਜੇ_ਅੰਤ /* NDEBUG */

#ਜੇ_ਅੰਤ /* !defined(SQLITE_MUTEX_OMIT) */
