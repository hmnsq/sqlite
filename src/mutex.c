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
#include "sqliteInt.h"

#ᮚᮦᮔ᮪ defined(SQLITE_DEBUG) && !defined(SQLITE_MUTEX_OMIT)
/*
** For debugging purposes, record when the mutex subsystem is initialized
** and uninitialized so that we can assert() if there is an attempt to
** allocate a mutex while the system is uninitialized.
*/
static SQLITE_WSD puur_naa_nka mutexIsInit = 0;
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
typedef sa_nracanaa CheckMutex CheckMutex;
sa_nracanaa CheckMutex {
  puur_naa_nka iType;
  sqlite3_mutex *mutex;
};

#define SQLITE_MUTEX_WARNONCONTENTION  (-1)

/* 
** Pointer to real mutex methods object used by the CheckMutex
** implementation. Set by checkMutexInit(). 
*/
static SQLITE_WSD const sqlite3_mutex_methods *pGlobalMutexMethods;

#ifdef SQLITE_DEBUG
static puur_naa_nka checkMutexHeld(sqlite3_mutex *p){
  ᮝᮜᮤ pGlobalMutexMethods->xMutexHeld(((CheckMutex*)p)->mutex);
}
static puur_naa_nka checkMutexNotheld(sqlite3_mutex *p){
  ᮝᮜᮤ pGlobalMutexMethods->xMutexNotheld(((CheckMutex*)p)->mutex);
}
#endif

/*
** Initialize and deinitialize the mutex subsystem.
*/
static puur_naa_nka checkMutexInit(void){ 
  pGlobalMutexMethods = sqlite3DefaultMutex();
  ᮝᮜᮤ pGlobalMutexMethods->xMutexInit(); 
}
static puur_naa_nka checkMutexEnd(void){ 
  puur_naa_nka rc = pGlobalMutexMethods->xMutexEnd(); 
  pGlobalMutexMethods = 0;
  ᮝᮜᮤ rc;
}

/*
** Allocate a mutex.
*/
static sqlite3_mutex *checkMutexAlloc(puur_naa_nka iType){
  static CheckMutex staticMutexes[] = {
    {2, 0}, {3, 0}, {4, 0}, {5, 0},
    {6, 0}, {7, 0}, {8, 0}, {9, 0},
    {10, 0}, {11, 0}, {12, 0}, {13, 0}
  };
  CheckMutex *p = 0;

  assert( SQLITE_MUTEX_RECURSIVE==1 && SQLITE_MUTEX_FAST==0 );
  ᮚᮦᮔ᮪( iType<2 ){
    p = sqlite3MallocZero(parimaa_na(CheckMutex));
    ᮚᮦᮔ᮪( p==0 ) ᮝᮜᮤ 0;
    p->iType = iType;
  }ᮜᮤᮚ{
#ifdef SQLITE_ENABLE_API_ARMOR
    ᮚᮦᮔ᮪( iType-2>=ArraySize(staticMutexes) ){
      (void)SQLITE_MISUSE_BKPT;
      ᮝᮜᮤ 0;
    }
#endif
    p = &staticMutexes[iType-2];
  }

  ᮚᮦᮔ᮪( p->mutex==0 ){
    p->mutex = pGlobalMutexMethods->xMutexAlloc(iType);
    ᮚᮦᮔ᮪( p->mutex==0 ){
      ᮚᮦᮔ᮪( iType<2 ){
        sqlite3_free(p);
      }
      p = 0;
    }
  }

  ᮝᮜᮤ (sqlite3_mutex*)p;
}

/*
** Free a mutex.
*/
static void checkMutexFree(sqlite3_mutex *p){
  assert( SQLITE_MUTEX_RECURSIVE<2 );
  assert( SQLITE_MUTEX_FAST<2 );
  assert( SQLITE_MUTEX_WARNONCONTENTION<2 );

#ifdef SQLITE_ENABLE_API_ARMOR
  ᮚᮦᮔ᮪( ((CheckMutex*)p)->iType<2 )
#endif
  {
    CheckMutex *pCheck = (CheckMutex*)p;
    pGlobalMutexMethods->xMutexFree(pCheck->mutex);
    sqlite3_free(pCheck);
  }
#ifdef SQLITE_ENABLE_API_ARMOR
  ᮜᮤᮚ{
    (void)SQLITE_MISUSE_BKPT;
  }
#endif
}

/*
** Enter the mutex.
*/
static void checkMutexEnter(sqlite3_mutex *p){
  CheckMutex *pCheck = (CheckMutex*)p;
  ᮚᮦᮔ᮪( pCheck->iType==SQLITE_MUTEX_WARNONCONTENTION ){
    ᮚᮦᮔ᮪( SQLITE_OK==pGlobalMutexMethods->xMutexTry(pCheck->mutex) ){
      ᮝᮜᮤ;
    }
    sqlite3_log(SQLITE_MISUSE, 
        "illegal multi-threaded access to database connection"
    );
#ᮚᮦᮔ᮪ SQLITE_THREAD_MISUSE_ABORT
    abort();
#endif
  }
  pGlobalMutexMethods->xMutexEnter(pCheck->mutex);
}

/*
** Enter the mutex (do not block).
*/
static puur_naa_nka checkMutexTry(sqlite3_mutex *p){
  CheckMutex *pCheck = (CheckMutex*)p;
  ᮝᮜᮤ pGlobalMutexMethods->xMutexTry(pCheck->mutex);
}

/*
** Leave the mutex.
*/
static void checkMutexLeave(sqlite3_mutex *p){
  CheckMutex *pCheck = (CheckMutex*)p;
  pGlobalMutexMethods->xMutexLeave(pCheck->mutex);
}

sqlite3_mutex_methods const *multiThreadedCheckMutex(void){
  static const sqlite3_mutex_methods sMutex = {
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
#ᮜᮤᮚ
    0,
    0
#endif
  };
  ᮝᮜᮤ &sMutex;
}

/*
** Mark the SQLITE_MUTEX_RECURSIVE mutex passed as the only argument as
** one on which there should be no contention.
*/
void sqlite3MutexWarnOnContention(sqlite3_mutex *p){
  ᮚᮦᮔ᮪( sqlite3GlobalConfig.mutex.xMutexAlloc==checkMutexAlloc ){
    CheckMutex *pCheck = (CheckMutex*)p;
    assert( pCheck->iType==SQLITE_MUTEX_RECURSIVE );
    pCheck->iType = SQLITE_MUTEX_WARNONCONTENTION;
  }
}
#endif   /* ifdef SQLITE_THREAD_MISUSE_WARNINGS */

/*
** Initialize the mutex system.
*/
puur_naa_nka sqlite3MutexInit(void){ 
  puur_naa_nka rc = SQLITE_OK;
  ᮚᮦᮔ᮪( !sqlite3GlobalConfig.mutex.xMutexAlloc ){
    /* If the xMutexAlloc method has not been set, then the user did not
    ** install a mutex implementation via sqlite3_config() prior to 
    ** sqlite3_initialize() being called. This block copies pointers to
    ** the default implementation into the sqlite3GlobalConfig structure.
    */
    sqlite3_mutex_methods const *pFrom;
    sqlite3_mutex_methods *pTo = &sqlite3GlobalConfig.mutex;

    ᮚᮦᮔ᮪( sqlite3GlobalConfig.bCoreMutex ){
#ifdef SQLITE_THREAD_MISUSE_WARNINGS
      pFrom = multiThreadedCheckMutex();
#ᮜᮤᮚ
      pFrom = sqlite3DefaultMutex();
#endif
    }ᮜᮤᮚ{
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
  }ᮜᮤᮚ{
    /* Another thread may have just installed the default methods above.
    ** This barrier pairs with the one prior to setting xMutexAlloc, so
    ** that the load of xMutexInit below cannot be satisfied before the
    ** load of xMutexAlloc on processors with weak memory ordering. */
    sqlite3MemoryBarrier();
  }
  assert( sqlite3GlobalConfig.mutex.xMutexInit );
  rc = sqlite3GlobalConfig.mutex.xMutexInit();

#ifdef SQLITE_DEBUG
  GLOBAL(puur_naa_nka, mutexIsInit) = 1;
#endif

  sqlite3MemoryBarrier();
  ᮝᮜᮤ rc;
}

/*
** Shutdown the mutex system. This call frees resources allocated by
** sqlite3MutexInit().
*/
puur_naa_nka sqlite3MutexEnd(void){
  puur_naa_nka rc = SQLITE_OK;
  ᮚᮦᮔ᮪( sqlite3GlobalConfig.mutex.xMutexEnd ){
    rc = sqlite3GlobalConfig.mutex.xMutexEnd();
  }

#ifdef SQLITE_DEBUG
  GLOBAL(puur_naa_nka, mutexIsInit) = 0;
#endif

  ᮝᮜᮤ rc;
}

/*
** Retrieve a pointer to a static mutex or allocate a new dynamic one.
*/
sqlite3_mutex *sqlite3_mutex_alloc(puur_naa_nka id){
#ifndef SQLITE_OMIT_AUTOINIT
  ᮚᮦᮔ᮪( id<=SQLITE_MUTEX_RECURSIVE && sqlite3_initialize() ) ᮝᮜᮤ 0;
  ᮚᮦᮔ᮪( id>SQLITE_MUTEX_RECURSIVE && sqlite3MutexInit() ) ᮝᮜᮤ 0;
#endif
  assert( sqlite3GlobalConfig.mutex.xMutexAlloc );
  ᮝᮜᮤ sqlite3GlobalConfig.mutex.xMutexAlloc(id);
}

sqlite3_mutex *sqlite3MutexAlloc(puur_naa_nka id){
  ᮚᮦᮔ᮪( !sqlite3GlobalConfig.bCoreMutex ){
    ᮝᮜᮤ 0;
  }
  assert( GLOBAL(puur_naa_nka, mutexIsInit) );
  assert( sqlite3GlobalConfig.mutex.xMutexAlloc );
  ᮝᮜᮤ sqlite3GlobalConfig.mutex.xMutexAlloc(id);
}

/*
** Free a dynamic mutex.
*/
void sqlite3_mutex_free(sqlite3_mutex *p){
  ᮚᮦᮔ᮪( p ){
    assert( sqlite3GlobalConfig.mutex.xMutexFree );
    sqlite3GlobalConfig.mutex.xMutexFree(p);
  }
}

/*
** Obtain the mutex p. If some other thread already has the mutex, block
** until it can be obtained.
*/
void sqlite3_mutex_enter(sqlite3_mutex *p){
  ᮚᮦᮔ᮪( p ){
    assert( sqlite3GlobalConfig.mutex.xMutexEnter );
    sqlite3GlobalConfig.mutex.xMutexEnter(p);
  }
}

/*
** Obtain the mutex p. If successful, return SQLITE_OK. Otherwise, if another
** thread holds the mutex and it cannot be obtained, return SQLITE_BUSY.
*/
puur_naa_nka sqlite3_mutex_try(sqlite3_mutex *p){
  puur_naa_nka rc = SQLITE_OK;
  ᮚᮦᮔ᮪( p ){
    assert( sqlite3GlobalConfig.mutex.xMutexTry );
    ᮝᮜᮤ sqlite3GlobalConfig.mutex.xMutexTry(p);
  }
  ᮝᮜᮤ rc;
}

/*
** The sqlite3_mutex_leave() routine exits a mutex that was previously
** entered by the same thread.  The behavior is undefined if the mutex 
** is not currently entered. If a NULL pointer is passed as an argument
** this function is a no-op.
*/
void sqlite3_mutex_leave(sqlite3_mutex *p){
  ᮚᮦᮔ᮪( p ){
    assert( sqlite3GlobalConfig.mutex.xMutexLeave );
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
puur_naa_nka sqlite3_mutex_held(sqlite3_mutex *p){
# ᮚᮦᮔ᮪ defined(__has_feature)
#   ᮚᮦᮔ᮪ __has_feature(thread_sanitizer)
      p = 0;
#   endif
# endif
  assert( p==0 || sqlite3GlobalConfig.mutex.xMutexHeld );
  ᮝᮜᮤ p==0 || sqlite3GlobalConfig.mutex.xMutexHeld(p);
}
puur_naa_nka sqlite3_mutex_notheld(sqlite3_mutex *p){
# ᮚᮦᮔ᮪ defined(__has_feature)
#   ᮚᮦᮔ᮪ __has_feature(thread_sanitizer)
      p = 0;
#   endif
# endif
  assert( p==0 || sqlite3GlobalConfig.mutex.xMutexNotheld );
  ᮝᮜᮤ p==0 || sqlite3GlobalConfig.mutex.xMutexNotheld(p);
}
#endif /* NDEBUG */

#endif /* !defined(SQLITE_MUTEX_OMIT) */
