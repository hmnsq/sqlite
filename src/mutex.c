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

#ಆದರೆ defined(SQLITE_DEBUG) && !defined(SQLITE_MUTEX_OMIT)
/*
** For debugging purposes, record when the mutex subsystem is initialized
** and uninitialized so that we can assert() if there is an attempt to
** allocate a mutex while the system is uninitialized.
*/
ಸ್ಥಿರ SQLITE_WSD ಪೂರ್ಣಾಂಕ mutexIsInit = 0;
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_DEBUG && !defined(SQLITE_MUTEX_OMIT) */


#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_MUTEX_OMIT

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_THREAD_MISUSE_WARNINGS
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
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ CheckMutex CheckMutex;
ರಚನೆ CheckMutex {
  ಪೂರ್ಣಾಂಕ iType;
  sqlite3_mutex *mutex;
};

#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_MUTEX_WARNONCONTENTION  (-1)

/* 
** Pointer to real mutex methods object used by the CheckMutex
** implementation. Set by checkMutexInit(). 
*/
ಸ್ಥಿರ SQLITE_WSD ಸ್ಥಿರಾಂಕ sqlite3_mutex_methods *pGlobalMutexMethods;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ checkMutexHeld(sqlite3_mutex *p){
  ಹಿಂತಿರುಗಿ pGlobalMutexMethods->xMutexHeld(((CheckMutex*)p)->mutex);
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ checkMutexNotheld(sqlite3_mutex *p){
  ಹಿಂತಿರುಗಿ pGlobalMutexMethods->xMutexNotheld(((CheckMutex*)p)->mutex);
}
#ಆದರೆ_ಅಂತ್ಯ

/*
** Initialize and deinitialize the mutex subsystem.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ checkMutexInit(ಶೂನ್ಯ){ 
  pGlobalMutexMethods = sqlite3DefaultMutex();
  ಹಿಂತಿರುಗಿ pGlobalMutexMethods->xMutexInit(); 
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ check_Mutex_ಕೊನೆ(ಶೂನ್ಯ){ 
  ಪೂರ್ಣಾಂಕ rc = pGlobalMutexMethods->x_Mutex_ಕೊನೆ(); 
  pGlobalMutexMethods = 0;
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Allocate a mutex.
*/
ಸ್ಥಿರ sqlite3_mutex *checkMutexAlloc(ಪೂರ್ಣಾಂಕ iType){
  ಸ್ಥಿರ CheckMutex staticMutexes[] = {
    {2, 0}, {3, 0}, {4, 0}, {5, 0},
    {6, 0}, {7, 0}, {8, 0}, {9, 0},
    {10, 0}, {11, 0}, {12, 0}, {13, 0}
  };
  CheckMutex *p = 0;

  ಪ್ರತಿಜ್ಞಾ( SQLITE_MUTEX_RECURSIVE==1 && SQLITE_MUTEX_FAST==0 );
  ಆದರೆ( iType<2 ){
    p = sqlite3MallocZero(ಗಾತ್ರ(CheckMutex));
    ಆದರೆ( p==0 ) ಹಿಂತಿರುಗಿ 0;
    p->iType = iType;
  }ಇಲ್ಲದಿದ್ದರೆ{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
    ಆದರೆ( iType-2>=Array_ಗಾತ್ರ(staticMutexes) ){
      (ಶೂನ್ಯ)SQLITE_MISUSE_BKPT;
      ಹಿಂತಿರುಗಿ 0;
    }
#ಆದರೆ_ಅಂತ್ಯ
    p = &staticMutexes[iType-2];
  }

  ಆದರೆ( p->mutex==0 ){
    p->mutex = pGlobalMutexMethods->xMutexAlloc(iType);
    ಆದರೆ( p->mutex==0 ){
      ಆದರೆ( iType<2 ){
        sqlite3_free(p);
      }
      p = 0;
    }
  }

  ಹಿಂತಿರುಗಿ (sqlite3_mutex*)p;
}

/*
** Free a mutex.
*/
ಸ್ಥಿರ ಶೂನ್ಯ checkMutexFree(sqlite3_mutex *p){
  ಪ್ರತಿಜ್ಞಾ( SQLITE_MUTEX_RECURSIVE<2 );
  ಪ್ರತಿಜ್ಞಾ( SQLITE_MUTEX_FAST<2 );
  ಪ್ರತಿಜ್ಞಾ( SQLITE_MUTEX_WARNONCONTENTION<2 );

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( ((CheckMutex*)p)->iType<2 )
#ಆದರೆ_ಅಂತ್ಯ
  {
    CheckMutex *pCheck = (CheckMutex*)p;
    pGlobalMutexMethods->xMutexFree(pCheck->mutex);
    sqlite3_free(pCheck);
  }
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಇಲ್ಲದಿದ್ದರೆ{
    (ಶೂನ್ಯ)SQLITE_MISUSE_BKPT;
  }
#ಆದರೆ_ಅಂತ್ಯ
}

/*
** Enter the mutex.
*/
ಸ್ಥಿರ ಶೂನ್ಯ checkMutexEnter(sqlite3_mutex *p){
  CheckMutex *pCheck = (CheckMutex*)p;
  ಆದರೆ( pCheck->iType==SQLITE_MUTEX_WARNONCONTENTION ){
    ಆದರೆ( SQLITE_OK==pGlobalMutexMethods->xMutexTry(pCheck->mutex) ){
      ಹಿಂತಿರುಗಿ;
    }
    sqlite3_log(SQLITE_MISUSE, 
        "illegal multi-threaded access to database connection"
    );
#ಆದರೆ SQLITE_THREAD_MISUSE_ABORT
    abort();
#ಆದರೆ_ಅಂತ್ಯ
  }
  pGlobalMutexMethods->xMutexEnter(pCheck->mutex);
}

/*
** Enter the mutex (do not block).
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ checkMutexTry(sqlite3_mutex *p){
  CheckMutex *pCheck = (CheckMutex*)p;
  ಹಿಂತಿರುಗಿ pGlobalMutexMethods->xMutexTry(pCheck->mutex);
}

/*
** Leave the mutex.
*/
ಸ್ಥಿರ ಶೂನ್ಯ checkMutexLeave(sqlite3_mutex *p){
  CheckMutex *pCheck = (CheckMutex*)p;
  pGlobalMutexMethods->xMutexLeave(pCheck->mutex);
}

sqlite3_mutex_methods ಸ್ಥಿರಾಂಕ *multiThreadedCheckMutex(ಶೂನ್ಯ){
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ sqlite3_mutex_methods sMutex = {
    checkMutexInit,
    check_Mutex_ಕೊನೆ,
    checkMutexAlloc,
    checkMutexFree,
    checkMutexEnter,
    checkMutexTry,
    checkMutexLeave,
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
    checkMutexHeld,
    checkMutexNotheld
#ಇಲ್ಲದಿದ್ದರೆ
    0,
    0
#ಆದರೆ_ಅಂತ್ಯ
  };
  ಹಿಂತಿರುಗಿ &sMutex;
}

/*
** Mark the SQLITE_MUTEX_RECURSIVE mutex passed as the only argument as
** one on which there should be no contention.
*/
ಶೂನ್ಯ sqlite3MutexWarnOnContention(sqlite3_mutex *p){
  ಆದರೆ( sqlite3GlobalConfig.mutex.xMutexAlloc==checkMutexAlloc ){
    CheckMutex *pCheck = (CheckMutex*)p;
    ಪ್ರತಿಜ್ಞಾ( pCheck->iType==SQLITE_MUTEX_RECURSIVE );
    pCheck->iType = SQLITE_MUTEX_WARNONCONTENTION;
  }
}
#ಆದರೆ_ಅಂತ್ಯ   /* ifdef SQLITE_THREAD_MISUSE_WARNINGS */

/*
** Initialize the mutex system.
*/
ಪೂರ್ಣಾಂಕ sqlite3MutexInit(ಶೂನ್ಯ){ 
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  ಆದರೆ( !sqlite3GlobalConfig.mutex.xMutexAlloc ){
    /* If the xMutexAlloc method has not been set, then the user did not
    ** install a mutex implementation via sqlite3_config() prior to 
    ** sqlite3_initialize() being called. This block copies pointers to
    ** the default implementation into the sqlite3GlobalConfig structure.
    */
    sqlite3_mutex_methods ಸ್ಥಿರಾಂಕ *pFrom;
    sqlite3_mutex_methods *p_ಗಮ್ಯ = &sqlite3GlobalConfig.mutex;

    ಆದರೆ( sqlite3GlobalConfig.bCoreMutex ){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_THREAD_MISUSE_WARNINGS
      pFrom = multiThreadedCheckMutex();
#ಇಲ್ಲದಿದ್ದರೆ
      pFrom = sqlite3DefaultMutex();
#ಆದರೆ_ಅಂತ್ಯ
    }ಇಲ್ಲದಿದ್ದರೆ{
      pFrom = sqlite3NoopMutex();
    }
    p_ಗಮ್ಯ->xMutexInit = pFrom->xMutexInit;
    p_ಗಮ್ಯ->x_Mutex_ಕೊನೆ = pFrom->x_Mutex_ಕೊನೆ;
    p_ಗಮ್ಯ->xMutexFree = pFrom->xMutexFree;
    p_ಗಮ್ಯ->xMutexEnter = pFrom->xMutexEnter;
    p_ಗಮ್ಯ->xMutexTry = pFrom->xMutexTry;
    p_ಗಮ್ಯ->xMutexLeave = pFrom->xMutexLeave;
    p_ಗಮ್ಯ->xMutexHeld = pFrom->xMutexHeld;
    p_ಗಮ್ಯ->xMutexNotheld = pFrom->xMutexNotheld;
    sqlite3_ಸ್ಮರಣೆ_Barrier();
    p_ಗಮ್ಯ->xMutexAlloc = pFrom->xMutexAlloc;
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* Another thread may have just installed the default methods above.
    ** This barrier pairs with the one prior to setting xMutexAlloc, so
    ** that the load of xMutexInit below cannot be satisfied before the
    ** load of xMutexAlloc on processors with weak memory ordering. */
    sqlite3_ಸ್ಮರಣೆ_Barrier();
  }
  ಪ್ರತಿಜ್ಞಾ( sqlite3GlobalConfig.mutex.xMutexInit );
  rc = sqlite3GlobalConfig.mutex.xMutexInit();

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  GLOBAL(ಪೂರ್ಣಾಂಕ, mutexIsInit) = 1;
#ಆದರೆ_ಅಂತ್ಯ

  sqlite3_ಸ್ಮರಣೆ_Barrier();
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Shutdown the mutex system. This call frees resources allocated by
** sqlite3MutexInit().
*/
ಪೂರ್ಣಾಂಕ sqlite3_Mutex_ಕೊನೆ(ಶೂನ್ಯ){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  ಆದರೆ( sqlite3GlobalConfig.mutex.x_Mutex_ಕೊನೆ ){
    rc = sqlite3GlobalConfig.mutex.x_Mutex_ಕೊನೆ();
  }

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  GLOBAL(ಪೂರ್ಣಾಂಕ, mutexIsInit) = 0;
#ಆದರೆ_ಅಂತ್ಯ

  ಹಿಂತಿರುಗಿ rc;
}

/*
** Retrieve a pointer to a static mutex or allocate a new dynamic one.
*/
sqlite3_mutex *sqlite3_mutex_alloc(ಪೂರ್ಣಾಂಕ id){
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOINIT
  ಆದರೆ( id<=SQLITE_MUTEX_RECURSIVE && sqlite3_initialize() ) ಹಿಂತಿರುಗಿ 0;
  ಆದರೆ( id>SQLITE_MUTEX_RECURSIVE && sqlite3MutexInit() ) ಹಿಂತಿರುಗಿ 0;
#ಆದರೆ_ಅಂತ್ಯ
  ಪ್ರತಿಜ್ಞಾ( sqlite3GlobalConfig.mutex.xMutexAlloc );
  ಹಿಂತಿರುಗಿ sqlite3GlobalConfig.mutex.xMutexAlloc(id);
}

sqlite3_mutex *sqlite3MutexAlloc(ಪೂರ್ಣಾಂಕ id){
  ಆದರೆ( !sqlite3GlobalConfig.bCoreMutex ){
    ಹಿಂತಿರುಗಿ 0;
  }
  ಪ್ರತಿಜ್ಞಾ( GLOBAL(ಪೂರ್ಣಾಂಕ, mutexIsInit) );
  ಪ್ರತಿಜ್ಞಾ( sqlite3GlobalConfig.mutex.xMutexAlloc );
  ಹಿಂತಿರುಗಿ sqlite3GlobalConfig.mutex.xMutexAlloc(id);
}

/*
** Free a dynamic mutex.
*/
ಶೂನ್ಯ sqlite3_mutex_free(sqlite3_mutex *p){
  ಆದರೆ( p ){
    ಪ್ರತಿಜ್ಞಾ( sqlite3GlobalConfig.mutex.xMutexFree );
    sqlite3GlobalConfig.mutex.xMutexFree(p);
  }
}

/*
** Obtain the mutex p. If some other thread already has the mutex, block
** until it can be obtained.
*/
ಶೂನ್ಯ sqlite3_mutex_enter(sqlite3_mutex *p){
  ಆದರೆ( p ){
    ಪ್ರತಿಜ್ಞಾ( sqlite3GlobalConfig.mutex.xMutexEnter );
    sqlite3GlobalConfig.mutex.xMutexEnter(p);
  }
}

/*
** Obtain the mutex p. If successful, return SQLITE_OK. Otherwise, if another
** thread holds the mutex and it cannot be obtained, return SQLITE_BUSY.
*/
ಪೂರ್ಣಾಂಕ sqlite3_mutex_try(sqlite3_mutex *p){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  ಆದರೆ( p ){
    ಪ್ರತಿಜ್ಞಾ( sqlite3GlobalConfig.mutex.xMutexTry );
    ಹಿಂತಿರುಗಿ sqlite3GlobalConfig.mutex.xMutexTry(p);
  }
  ಹಿಂತಿರುಗಿ rc;
}

/*
** The sqlite3_mutex_leave() routine exits a mutex that was previously
** entered by the same thread.  The behavior is undefined if the mutex 
** is not currently entered. If a NULL pointer is passed as an argument
** this function is a no-op.
*/
ಶೂನ್ಯ sqlite3_mutex_leave(sqlite3_mutex *p){
  ಆದರೆ( p ){
    ಪ್ರತಿಜ್ಞಾ( sqlite3GlobalConfig.mutex.xMutexLeave );
    sqlite3GlobalConfig.mutex.xMutexLeave(p);
  }
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NDEBUG
/*
** The sqlite3_mutex_held() and sqlite3_mutex_notheld() routine are
** intended for use inside assert() statements.
**
** Because these routines raise false-positive alerts in TSAN, disable
** them (make them always return 1) when compiling with TSAN.
*/
ಪೂರ್ಣಾಂಕ sqlite3_mutex_held(sqlite3_mutex *p){
# ಆದರೆ defined(__has_feature)
#   ಆದರೆ __has_feature(thread_sanitizer)
      p = 0;
#   endif
# endif
  ಪ್ರತಿಜ್ಞಾ( p==0 || sqlite3GlobalConfig.mutex.xMutexHeld );
  ಹಿಂತಿರುಗಿ p==0 || sqlite3GlobalConfig.mutex.xMutexHeld(p);
}
ಪೂರ್ಣಾಂಕ sqlite3_mutex_notheld(sqlite3_mutex *p){
# ಆದರೆ defined(__has_feature)
#   ಆದರೆ __has_feature(thread_sanitizer)
      p = 0;
#   endif
# endif
  ಪ್ರತಿಜ್ಞಾ( p==0 || sqlite3GlobalConfig.mutex.xMutexNotheld );
  ಹಿಂತಿರುಗಿ p==0 || sqlite3GlobalConfig.mutex.xMutexNotheld(p);
}
#ಆದರೆ_ಅಂತ್ಯ /* NDEBUG */

#ಆದರೆ_ಅಂತ್ಯ /* !defined(SQLITE_MUTEX_OMIT) */
