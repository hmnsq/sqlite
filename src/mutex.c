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

#nếu defined(SQLITE_DEBUG) && !defined(SQLITE_MUTEX_OMIT)
/*
** For debugging purposes, record when the mutex subsystem is initialized
** and uninitialized so that we can assert() if there is an attempt to
** allocate a mutex while the system is uninitialized.
*/
static SQLITE_WSD số_nguyên mutexIsInit = 0;
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
typedef struct CheckMutex CheckMutex;
struct CheckMutex {
  số_nguyên iType;
  sqlite3_mutex *mutex;
};

#define SQLITE_MUTEX_WARNONCONTENTION  (-1)

/* 
** Pointer to real mutex methods object used by the CheckMutex
** implementation. Set by checkMutexInit(). 
*/
static SQLITE_WSD const sqlite3_mutex_methods *pGlobalMutexMethods;

#ifdef SQLITE_DEBUG
static số_nguyên checkMutexHeld(sqlite3_mutex *p){
  trả_về pGlobalMutexMethods->xMutexHeld(((CheckMutex*)p)->mutex);
}
static số_nguyên checkMutexNotheld(sqlite3_mutex *p){
  trả_về pGlobalMutexMethods->xMutexNotheld(((CheckMutex*)p)->mutex);
}
#endif

/*
** Initialize and deinitialize the mutex subsystem.
*/
static số_nguyên checkMutexInit(void){ 
  pGlobalMutexMethods = sqlite3DefaultMutex();
  trả_về pGlobalMutexMethods->xMutexInit(); 
}
static số_nguyên checkMutexEnd(void){ 
  số_nguyên rc = pGlobalMutexMethods->xMutexEnd(); 
  pGlobalMutexMethods = 0;
  trả_về rc;
}

/*
** Allocate a mutex.
*/
static sqlite3_mutex *checkMutexAlloc(số_nguyên iType){
  static CheckMutex staticMutexes[] = {
    {2, 0}, {3, 0}, {4, 0}, {5, 0},
    {6, 0}, {7, 0}, {8, 0}, {9, 0},
    {10, 0}, {11, 0}, {12, 0}, {13, 0}
  };
  CheckMutex *p = 0;

  assert( SQLITE_MUTEX_RECURSIVE==1 && SQLITE_MUTEX_FAST==0 );
  nếu( iType<2 ){
    p = sqlite3MallocZero(sizeof(CheckMutex));
    nếu( p==0 ) trả_về 0;
    p->iType = iType;
  }không_thì{
#ifdef SQLITE_ENABLE_API_ARMOR
    nếu( iType-2>=ArraySize(staticMutexes) ){
      (void)SQLITE_MISUSE_BKPT;
      trả_về 0;
    }
#endif
    p = &staticMutexes[iType-2];
  }

  nếu( p->mutex==0 ){
    p->mutex = pGlobalMutexMethods->xMutexAlloc(iType);
    nếu( p->mutex==0 ){
      nếu( iType<2 ){
        sqlite3_free(p);
      }
      p = 0;
    }
  }

  trả_về (sqlite3_mutex*)p;
}

/*
** Free a mutex.
*/
static void checkMutexFree(sqlite3_mutex *p){
  assert( SQLITE_MUTEX_RECURSIVE<2 );
  assert( SQLITE_MUTEX_FAST<2 );
  assert( SQLITE_MUTEX_WARNONCONTENTION<2 );

#ifdef SQLITE_ENABLE_API_ARMOR
  nếu( ((CheckMutex*)p)->iType<2 )
#endif
  {
    CheckMutex *pCheck = (CheckMutex*)p;
    pGlobalMutexMethods->xMutexFree(pCheck->mutex);
    sqlite3_free(pCheck);
  }
#ifdef SQLITE_ENABLE_API_ARMOR
  không_thì{
    (void)SQLITE_MISUSE_BKPT;
  }
#endif
}

/*
** Enter the mutex.
*/
static void checkMutexEnter(sqlite3_mutex *p){
  CheckMutex *pCheck = (CheckMutex*)p;
  nếu( pCheck->iType==SQLITE_MUTEX_WARNONCONTENTION ){
    nếu( SQLITE_OK==pGlobalMutexMethods->xMutexTry(pCheck->mutex) ){
      trả_về;
    }
    sqlite3_log(SQLITE_MISUSE, 
        "illegal multi-threaded access to database connection"
    );
#nếu SQLITE_THREAD_MISUSE_ABORT
    abort();
#endif
  }
  pGlobalMutexMethods->xMutexEnter(pCheck->mutex);
}

/*
** Enter the mutex (do not block).
*/
static số_nguyên checkMutexTry(sqlite3_mutex *p){
  CheckMutex *pCheck = (CheckMutex*)p;
  trả_về pGlobalMutexMethods->xMutexTry(pCheck->mutex);
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
#không_thì
    0,
    0
#endif
  };
  trả_về &sMutex;
}

/*
** Mark the SQLITE_MUTEX_RECURSIVE mutex passed as the only argument as
** one on which there should be no contention.
*/
void sqlite3MutexWarnOnContention(sqlite3_mutex *p){
  nếu( sqlite3GlobalConfig.mutex.xMutexAlloc==checkMutexAlloc ){
    CheckMutex *pCheck = (CheckMutex*)p;
    assert( pCheck->iType==SQLITE_MUTEX_RECURSIVE );
    pCheck->iType = SQLITE_MUTEX_WARNONCONTENTION;
  }
}
#endif   /* ifdef SQLITE_THREAD_MISUSE_WARNINGS */

/*
** Initialize the mutex system.
*/
số_nguyên sqlite3MutexInit(void){ 
  số_nguyên rc = SQLITE_OK;
  nếu( !sqlite3GlobalConfig.mutex.xMutexAlloc ){
    /* If the xMutexAlloc method has not been set, then the user did not
    ** install a mutex implementation via sqlite3_config() prior to 
    ** sqlite3_initialize() being called. This block copies pointers to
    ** the default implementation into the sqlite3GlobalConfig structure.
    */
    sqlite3_mutex_methods const *pFrom;
    sqlite3_mutex_methods *pTo = &sqlite3GlobalConfig.mutex;

    nếu( sqlite3GlobalConfig.bCoreMutex ){
#ifdef SQLITE_THREAD_MISUSE_WARNINGS
      pFrom = multiThreadedCheckMutex();
#không_thì
      pFrom = sqlite3DefaultMutex();
#endif
    }không_thì{
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
  }không_thì{
    /* Another thread may have just installed the default methods above.
    ** This barrier pairs with the one prior to setting xMutexAlloc, so
    ** that the load of xMutexInit below cannot be satisfied before the
    ** load of xMutexAlloc on processors with weak memory ordering. */
    sqlite3MemoryBarrier();
  }
  assert( sqlite3GlobalConfig.mutex.xMutexInit );
  rc = sqlite3GlobalConfig.mutex.xMutexInit();

#ifdef SQLITE_DEBUG
  GLOBAL(số_nguyên, mutexIsInit) = 1;
#endif

  sqlite3MemoryBarrier();
  trả_về rc;
}

/*
** Shutdown the mutex system. This call frees resources allocated by
** sqlite3MutexInit().
*/
số_nguyên sqlite3MutexEnd(void){
  số_nguyên rc = SQLITE_OK;
  nếu( sqlite3GlobalConfig.mutex.xMutexEnd ){
    rc = sqlite3GlobalConfig.mutex.xMutexEnd();
  }

#ifdef SQLITE_DEBUG
  GLOBAL(số_nguyên, mutexIsInit) = 0;
#endif

  trả_về rc;
}

/*
** Retrieve a pointer to a static mutex or allocate a new dynamic one.
*/
sqlite3_mutex *sqlite3_mutex_alloc(số_nguyên id){
#ifndef SQLITE_OMIT_AUTOINIT
  nếu( id<=SQLITE_MUTEX_RECURSIVE && sqlite3_initialize() ) trả_về 0;
  nếu( id>SQLITE_MUTEX_RECURSIVE && sqlite3MutexInit() ) trả_về 0;
#endif
  assert( sqlite3GlobalConfig.mutex.xMutexAlloc );
  trả_về sqlite3GlobalConfig.mutex.xMutexAlloc(id);
}

sqlite3_mutex *sqlite3MutexAlloc(số_nguyên id){
  nếu( !sqlite3GlobalConfig.bCoreMutex ){
    trả_về 0;
  }
  assert( GLOBAL(số_nguyên, mutexIsInit) );
  assert( sqlite3GlobalConfig.mutex.xMutexAlloc );
  trả_về sqlite3GlobalConfig.mutex.xMutexAlloc(id);
}

/*
** Free a dynamic mutex.
*/
void sqlite3_mutex_free(sqlite3_mutex *p){
  nếu( p ){
    assert( sqlite3GlobalConfig.mutex.xMutexFree );
    sqlite3GlobalConfig.mutex.xMutexFree(p);
  }
}

/*
** Obtain the mutex p. If some other thread already has the mutex, block
** until it can be obtained.
*/
void sqlite3_mutex_enter(sqlite3_mutex *p){
  nếu( p ){
    assert( sqlite3GlobalConfig.mutex.xMutexEnter );
    sqlite3GlobalConfig.mutex.xMutexEnter(p);
  }
}

/*
** Obtain the mutex p. If successful, return SQLITE_OK. Otherwise, if another
** thread holds the mutex and it cannot be obtained, return SQLITE_BUSY.
*/
số_nguyên sqlite3_mutex_try(sqlite3_mutex *p){
  số_nguyên rc = SQLITE_OK;
  nếu( p ){
    assert( sqlite3GlobalConfig.mutex.xMutexTry );
    trả_về sqlite3GlobalConfig.mutex.xMutexTry(p);
  }
  trả_về rc;
}

/*
** The sqlite3_mutex_leave() routine exits a mutex that was previously
** entered by the same thread.  The behavior is undefined if the mutex 
** is not currently entered. If a NULL pointer is passed as an argument
** this function is a no-op.
*/
void sqlite3_mutex_leave(sqlite3_mutex *p){
  nếu( p ){
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
số_nguyên sqlite3_mutex_held(sqlite3_mutex *p){
# nếu defined(__has_feature)
#   nếu __has_feature(thread_sanitizer)
      p = 0;
#   endif
# endif
  assert( p==0 || sqlite3GlobalConfig.mutex.xMutexHeld );
  trả_về p==0 || sqlite3GlobalConfig.mutex.xMutexHeld(p);
}
số_nguyên sqlite3_mutex_notheld(sqlite3_mutex *p){
# nếu defined(__has_feature)
#   nếu __has_feature(thread_sanitizer)
      p = 0;
#   endif
# endif
  assert( p==0 || sqlite3GlobalConfig.mutex.xMutexNotheld );
  trả_về p==0 || sqlite3GlobalConfig.mutex.xMutexNotheld(p);
}
#endif /* NDEBUG */

#endif /* !defined(SQLITE_MUTEX_OMIT) */
