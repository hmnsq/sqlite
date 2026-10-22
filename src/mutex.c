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
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"

#ಯದಿ defined(SQLITE_DEBUG) && !defined(SQLITE_MUTEX_OMIT)
/*
** For debugging purposes, record when the mutex subsystem is initialized
** and uninitialized so that we can assert() if there is an attempt to
** allocate a mutex while the system is uninitialized.
*/
ಸ್ಥಿರ SQLITE_WSD ಪೂರ್ಣಾಙ್ಕ mutexIsInit = 0;
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
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ CheckMutex CheckMutex;
ಸಂರಚನಾ CheckMutex {
  ಪೂರ್ಣಾಙ್ಕ iType;
  sqlite3_mutex *mutex;
};

#define SQLITE_MUTEX_WARNONCONTENTION  (-1)

/* 
** Pointer to real mutex methods object used by the CheckMutex
** implementation. Set by checkMutexInit(). 
*/
ಸ್ಥಿರ SQLITE_WSD ಧ್ರುವ sqlite3_mutex_methods *pGlobalMutexMethods;

#ifdef SQLITE_DEBUG
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ checkMutexHeld(sqlite3_mutex *p){
  ಪ್ರತ್ಯಾಗಮ pGlobalMutexMethods->xMutexHeld(((CheckMutex*)p)->mutex);
}
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ checkMutexNotheld(sqlite3_mutex *p){
  ಪ್ರತ್ಯಾಗಮ pGlobalMutexMethods->xMutexNotheld(((CheckMutex*)p)->mutex);
}
#endif

/*
** Initialize and deinitialize the mutex subsystem.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ checkMutexInit(ಶೂನ್ಯ){ 
  pGlobalMutexMethods = sqlite3DefaultMutex();
  ಪ್ರತ್ಯಾಗಮ pGlobalMutexMethods->xMutexInit(); 
}
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ checkMutexEnd(ಶೂನ್ಯ){ 
  ಪೂರ್ಣಾಙ್ಕ rc = pGlobalMutexMethods->xMutexEnd(); 
  pGlobalMutexMethods = 0;
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Allocate a mutex.
*/
ಸ್ಥಿರ sqlite3_mutex *checkMutexAlloc(ಪೂರ್ಣಾಙ್ಕ iType){
  ಸ್ಥಿರ CheckMutex staticMutexes[] = {
    {2, 0}, {3, 0}, {4, 0}, {5, 0},
    {6, 0}, {7, 0}, {8, 0}, {9, 0},
    {10, 0}, {11, 0}, {12, 0}, {13, 0}
  };
  CheckMutex *p = 0;

  ಪ್ರತಿಜ್ಞಾ( SQLITE_MUTEX_RECURSIVE==1 && SQLITE_MUTEX_FAST==0 );
  ಯದಿ( iType<2 ){
    p = sqlite3MallocZero(ಪರಿಮಾಣ(CheckMutex));
    ಯದಿ( p==0 ) ಪ್ರತ್ಯಾಗಮ 0;
    p->iType = iType;
  }ಅನ್ಯಥಾ{
#ifdef SQLITE_ENABLE_API_ARMOR
    ಯದಿ( iType-2>=ArraySize(staticMutexes) ){
      (ಶೂನ್ಯ)SQLITE_MISUSE_BKPT;
      ಪ್ರತ್ಯಾಗಮ 0;
    }
#endif
    p = &staticMutexes[iType-2];
  }

  ಯದಿ( p->mutex==0 ){
    p->mutex = pGlobalMutexMethods->xMutexAlloc(iType);
    ಯದಿ( p->mutex==0 ){
      ಯದಿ( iType<2 ){
        sqlite3_free(p);
      }
      p = 0;
    }
  }

  ಪ್ರತ್ಯಾಗಮ (sqlite3_mutex*)p;
}

/*
** Free a mutex.
*/
ಸ್ಥಿರ ಶೂನ್ಯ checkMutexFree(sqlite3_mutex *p){
  ಪ್ರತಿಜ್ಞಾ( SQLITE_MUTEX_RECURSIVE<2 );
  ಪ್ರತಿಜ್ಞಾ( SQLITE_MUTEX_FAST<2 );
  ಪ್ರತಿಜ್ಞಾ( SQLITE_MUTEX_WARNONCONTENTION<2 );

#ifdef SQLITE_ENABLE_API_ARMOR
  ಯದಿ( ((CheckMutex*)p)->iType<2 )
#endif
  {
    CheckMutex *pCheck = (CheckMutex*)p;
    pGlobalMutexMethods->xMutexFree(pCheck->mutex);
    sqlite3_free(pCheck);
  }
#ifdef SQLITE_ENABLE_API_ARMOR
  ಅನ್ಯಥಾ{
    (ಶೂನ್ಯ)SQLITE_MISUSE_BKPT;
  }
#endif
}

/*
** Enter the mutex.
*/
ಸ್ಥಿರ ಶೂನ್ಯ checkMutexEnter(sqlite3_mutex *p){
  CheckMutex *pCheck = (CheckMutex*)p;
  ಯದಿ( pCheck->iType==SQLITE_MUTEX_WARNONCONTENTION ){
    ಯದಿ( SQLITE_OK==pGlobalMutexMethods->xMutexTry(pCheck->mutex) ){
      ಪ್ರತ್ಯಾಗಮ;
    }
    sqlite3_log(SQLITE_MISUSE, 
        "illegal multi-threaded access to database connection"
    );
#ಯದಿ SQLITE_THREAD_MISUSE_ABORT
    abort();
#endif
  }
  pGlobalMutexMethods->xMutexEnter(pCheck->mutex);
}

/*
** Enter the mutex (do not block).
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ checkMutexTry(sqlite3_mutex *p){
  CheckMutex *pCheck = (CheckMutex*)p;
  ಪ್ರತ್ಯಾಗಮ pGlobalMutexMethods->xMutexTry(pCheck->mutex);
}

/*
** Leave the mutex.
*/
ಸ್ಥಿರ ಶೂನ್ಯ checkMutexLeave(sqlite3_mutex *p){
  CheckMutex *pCheck = (CheckMutex*)p;
  pGlobalMutexMethods->xMutexLeave(pCheck->mutex);
}

sqlite3_mutex_methods ಧ್ರುವ *multiThreadedCheckMutex(ಶೂನ್ಯ){
  ಸ್ಥಿರ ಧ್ರುವ sqlite3_mutex_methods sMutex = {
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
#ಅನ್ಯಥಾ
    0,
    0
#endif
  };
  ಪ್ರತ್ಯಾಗಮ &sMutex;
}

/*
** Mark the SQLITE_MUTEX_RECURSIVE mutex passed as the only argument as
** one on which there should be no contention.
*/
ಶೂನ್ಯ sqlite3MutexWarnOnContention(sqlite3_mutex *p){
  ಯದಿ( sqlite3GlobalConfig.mutex.xMutexAlloc==checkMutexAlloc ){
    CheckMutex *pCheck = (CheckMutex*)p;
    ಪ್ರತಿಜ್ಞಾ( pCheck->iType==SQLITE_MUTEX_RECURSIVE );
    pCheck->iType = SQLITE_MUTEX_WARNONCONTENTION;
  }
}
#endif   /* ifdef SQLITE_THREAD_MISUSE_WARNINGS */

/*
** Initialize the mutex system.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3MutexInit(ಶೂನ್ಯ){ 
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;
  ಯದಿ( !sqlite3GlobalConfig.mutex.xMutexAlloc ){
    /* If the xMutexAlloc method has not been set, then the user did not
    ** install a mutex implementation via sqlite3_config() prior to 
    ** sqlite3_initialize() being called. This block copies pointers to
    ** the default implementation into the sqlite3GlobalConfig structure.
    */
    sqlite3_mutex_methods ಧ್ರುವ *pFrom;
    sqlite3_mutex_methods *pTo = &sqlite3GlobalConfig.mutex;

    ಯದಿ( sqlite3GlobalConfig.bCoreMutex ){
#ifdef SQLITE_THREAD_MISUSE_WARNINGS
      pFrom = multiThreadedCheckMutex();
#ಅನ್ಯಥಾ
      pFrom = sqlite3DefaultMutex();
#endif
    }ಅನ್ಯಥಾ{
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
  }ಅನ್ಯಥಾ{
    /* Another thread may have just installed the default methods above.
    ** This barrier pairs with the one prior to setting xMutexAlloc, so
    ** that the load of xMutexInit below cannot be satisfied before the
    ** load of xMutexAlloc on processors with weak memory ordering. */
    sqlite3MemoryBarrier();
  }
  ಪ್ರತಿಜ್ಞಾ( sqlite3GlobalConfig.mutex.xMutexInit );
  rc = sqlite3GlobalConfig.mutex.xMutexInit();

#ifdef SQLITE_DEBUG
  GLOBAL(ಪೂರ್ಣಾಙ್ಕ, mutexIsInit) = 1;
#endif

  sqlite3MemoryBarrier();
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Shutdown the mutex system. This call frees resources allocated by
** sqlite3MutexInit().
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3MutexEnd(ಶೂನ್ಯ){
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;
  ಯದಿ( sqlite3GlobalConfig.mutex.xMutexEnd ){
    rc = sqlite3GlobalConfig.mutex.xMutexEnd();
  }

#ifdef SQLITE_DEBUG
  GLOBAL(ಪೂರ್ಣಾಙ್ಕ, mutexIsInit) = 0;
#endif

  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Retrieve a pointer to a static mutex or allocate a new dynamic one.
*/
sqlite3_mutex *sqlite3_mutex_alloc(ಪೂರ್ಣಾಙ್ಕ id){
#ifndef SQLITE_OMIT_AUTOINIT
  ಯದಿ( id<=SQLITE_MUTEX_RECURSIVE && sqlite3_initialize() ) ಪ್ರತ್ಯಾಗಮ 0;
  ಯದಿ( id>SQLITE_MUTEX_RECURSIVE && sqlite3MutexInit() ) ಪ್ರತ್ಯಾಗಮ 0;
#endif
  ಪ್ರತಿಜ್ಞಾ( sqlite3GlobalConfig.mutex.xMutexAlloc );
  ಪ್ರತ್ಯಾಗಮ sqlite3GlobalConfig.mutex.xMutexAlloc(id);
}

sqlite3_mutex *sqlite3MutexAlloc(ಪೂರ್ಣಾಙ್ಕ id){
  ಯದಿ( !sqlite3GlobalConfig.bCoreMutex ){
    ಪ್ರತ್ಯಾಗಮ 0;
  }
  ಪ್ರತಿಜ್ಞಾ( GLOBAL(ಪೂರ್ಣಾಙ್ಕ, mutexIsInit) );
  ಪ್ರತಿಜ್ಞಾ( sqlite3GlobalConfig.mutex.xMutexAlloc );
  ಪ್ರತ್ಯಾಗಮ sqlite3GlobalConfig.mutex.xMutexAlloc(id);
}

/*
** Free a dynamic mutex.
*/
ಶೂನ್ಯ sqlite3_mutex_free(sqlite3_mutex *p){
  ಯದಿ( p ){
    ಪ್ರತಿಜ್ಞಾ( sqlite3GlobalConfig.mutex.xMutexFree );
    sqlite3GlobalConfig.mutex.xMutexFree(p);
  }
}

/*
** Obtain the mutex p. If some other thread already has the mutex, block
** until it can be obtained.
*/
ಶೂನ್ಯ sqlite3_mutex_enter(sqlite3_mutex *p){
  ಯದಿ( p ){
    ಪ್ರತಿಜ್ಞಾ( sqlite3GlobalConfig.mutex.xMutexEnter );
    sqlite3GlobalConfig.mutex.xMutexEnter(p);
  }
}

/*
** Obtain the mutex p. If successful, return SQLITE_OK. Otherwise, if another
** thread holds the mutex and it cannot be obtained, return SQLITE_BUSY.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_mutex_try(sqlite3_mutex *p){
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;
  ಯದಿ( p ){
    ಪ್ರತಿಜ್ಞಾ( sqlite3GlobalConfig.mutex.xMutexTry );
    ಪ್ರತ್ಯಾಗಮ sqlite3GlobalConfig.mutex.xMutexTry(p);
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** The sqlite3_mutex_leave() routine exits a mutex that was previously
** entered by the same thread.  The behavior is undefined if the mutex 
** is not currently entered. If a NULL pointer is passed as an argument
** this function is a no-op.
*/
ಶೂನ್ಯ sqlite3_mutex_leave(sqlite3_mutex *p){
  ಯದಿ( p ){
    ಪ್ರತಿಜ್ಞಾ( sqlite3GlobalConfig.mutex.xMutexLeave );
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
ಪೂರ್ಣಾಙ್ಕ sqlite3_mutex_held(sqlite3_mutex *p){
# ಯದಿ defined(__has_feature)
#   ಯದಿ __has_feature(thread_sanitizer)
      p = 0;
#   endif
# endif
  ಪ್ರತಿಜ್ಞಾ( p==0 || sqlite3GlobalConfig.mutex.xMutexHeld );
  ಪ್ರತ್ಯಾಗಮ p==0 || sqlite3GlobalConfig.mutex.xMutexHeld(p);
}
ಪೂರ್ಣಾಙ್ಕ sqlite3_mutex_notheld(sqlite3_mutex *p){
# ಯದಿ defined(__has_feature)
#   ಯದಿ __has_feature(thread_sanitizer)
      p = 0;
#   endif
# endif
  ಪ್ರತಿಜ್ಞಾ( p==0 || sqlite3GlobalConfig.mutex.xMutexNotheld );
  ಪ್ರತ್ಯಾಗಮ p==0 || sqlite3GlobalConfig.mutex.xMutexNotheld(p);
}
#endif /* NDEBUG */

#endif /* !defined(SQLITE_MUTEX_OMIT) */
