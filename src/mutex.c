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

#གལ་སྲིད defined(SQLITE_DEBUG) && !defined(SQLITE_MUTEX_OMIT)
/*
** For debugging purposes, record when the mutex subsystem is initialized
** and uninitialized so that we can assert() if there is an attempt to
** allocate a mutex while the system is uninitialized.
*/
static SQLITE_WSD ཧྲིལ mutexIsInit = 0;
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
  ཧྲིལ iType;
  sqlite3_mutex *mutex;
};

#define SQLITE_MUTEX_WARNONCONTENTION  (-1)

/* 
** Pointer to real mutex methods object used by the CheckMutex
** implementation. Set by checkMutexInit(). 
*/
static SQLITE_WSD const sqlite3_mutex_methods *pGlobalMutexMethods;

#ifdef SQLITE_DEBUG
static ཧྲིལ checkMutexHeld(sqlite3_mutex *p){
  ལོག pGlobalMutexMethods->xMutexHeld(((CheckMutex*)p)->mutex);
}
static ཧྲིལ checkMutexNotheld(sqlite3_mutex *p){
  ལོག pGlobalMutexMethods->xMutexNotheld(((CheckMutex*)p)->mutex);
}
#endif

/*
** Initialize and deinitialize the mutex subsystem.
*/
static ཧྲིལ checkMutexInit(void){ 
  pGlobalMutexMethods = sqlite3DefaultMutex();
  ལོག pGlobalMutexMethods->xMutexInit(); 
}
static ཧྲིལ checkMutexEnd(void){ 
  ཧྲིལ rc = pGlobalMutexMethods->xMutexEnd(); 
  pGlobalMutexMethods = 0;
  ལོག rc;
}

/*
** Allocate a mutex.
*/
static sqlite3_mutex *checkMutexAlloc(ཧྲིལ iType){
  static CheckMutex staticMutexes[] = {
    {2, 0}, {3, 0}, {4, 0}, {5, 0},
    {6, 0}, {7, 0}, {8, 0}, {9, 0},
    {10, 0}, {11, 0}, {12, 0}, {13, 0}
  };
  CheckMutex *p = 0;

  assert( SQLITE_MUTEX_RECURSIVE==1 && SQLITE_MUTEX_FAST==0 );
  གལ་སྲིད( iType<2 ){
    p = sqlite3MallocZero(parimaa_na(CheckMutex));
    གལ་སྲིད( p==0 ) ལོག 0;
    p->iType = iType;
  }དེ་མིན{
#ifdef SQLITE_ENABLE_API_ARMOR
    གལ་སྲིད( iType-2>=ArraySize(staticMutexes) ){
      (void)SQLITE_MISUSE_BKPT;
      ལོག 0;
    }
#endif
    p = &staticMutexes[iType-2];
  }

  གལ་སྲིད( p->mutex==0 ){
    p->mutex = pGlobalMutexMethods->xMutexAlloc(iType);
    གལ་སྲིད( p->mutex==0 ){
      གལ་སྲིད( iType<2 ){
        sqlite3_free(p);
      }
      p = 0;
    }
  }

  ལོག (sqlite3_mutex*)p;
}

/*
** Free a mutex.
*/
static void checkMutexFree(sqlite3_mutex *p){
  assert( SQLITE_MUTEX_RECURSIVE<2 );
  assert( SQLITE_MUTEX_FAST<2 );
  assert( SQLITE_MUTEX_WARNONCONTENTION<2 );

#ifdef SQLITE_ENABLE_API_ARMOR
  གལ་སྲིད( ((CheckMutex*)p)->iType<2 )
#endif
  {
    CheckMutex *pCheck = (CheckMutex*)p;
    pGlobalMutexMethods->xMutexFree(pCheck->mutex);
    sqlite3_free(pCheck);
  }
#ifdef SQLITE_ENABLE_API_ARMOR
  དེ་མིན{
    (void)SQLITE_MISUSE_BKPT;
  }
#endif
}

/*
** Enter the mutex.
*/
static void checkMutexEnter(sqlite3_mutex *p){
  CheckMutex *pCheck = (CheckMutex*)p;
  གལ་སྲིད( pCheck->iType==SQLITE_MUTEX_WARNONCONTENTION ){
    གལ་སྲིད( SQLITE_OK==pGlobalMutexMethods->xMutexTry(pCheck->mutex) ){
      ལོག;
    }
    sqlite3_log(SQLITE_MISUSE, 
        "illegal multi-threaded access to database connection"
    );
#གལ་སྲིད SQLITE_THREAD_MISUSE_ABORT
    abort();
#endif
  }
  pGlobalMutexMethods->xMutexEnter(pCheck->mutex);
}

/*
** Enter the mutex (do not block).
*/
static ཧྲིལ checkMutexTry(sqlite3_mutex *p){
  CheckMutex *pCheck = (CheckMutex*)p;
  ལོག pGlobalMutexMethods->xMutexTry(pCheck->mutex);
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
#དེ་མིན
    0,
    0
#endif
  };
  ལོག &sMutex;
}

/*
** Mark the SQLITE_MUTEX_RECURSIVE mutex passed as the only argument as
** one on which there should be no contention.
*/
void sqlite3MutexWarnOnContention(sqlite3_mutex *p){
  གལ་སྲིད( sqlite3GlobalConfig.mutex.xMutexAlloc==checkMutexAlloc ){
    CheckMutex *pCheck = (CheckMutex*)p;
    assert( pCheck->iType==SQLITE_MUTEX_RECURSIVE );
    pCheck->iType = SQLITE_MUTEX_WARNONCONTENTION;
  }
}
#endif   /* ifdef SQLITE_THREAD_MISUSE_WARNINGS */

/*
** Initialize the mutex system.
*/
ཧྲིལ sqlite3MutexInit(void){ 
  ཧྲིལ rc = SQLITE_OK;
  གལ་སྲིད( !sqlite3GlobalConfig.mutex.xMutexAlloc ){
    /* If the xMutexAlloc method has not been set, then the user did not
    ** install a mutex implementation via sqlite3_config() prior to 
    ** sqlite3_initialize() being called. This block copies pointers to
    ** the default implementation into the sqlite3GlobalConfig structure.
    */
    sqlite3_mutex_methods const *pFrom;
    sqlite3_mutex_methods *pTo = &sqlite3GlobalConfig.mutex;

    གལ་སྲིད( sqlite3GlobalConfig.bCoreMutex ){
#ifdef SQLITE_THREAD_MISUSE_WARNINGS
      pFrom = multiThreadedCheckMutex();
#དེ་མིན
      pFrom = sqlite3DefaultMutex();
#endif
    }དེ་མིན{
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
  }དེ་མིན{
    /* Another thread may have just installed the default methods above.
    ** This barrier pairs with the one prior to setting xMutexAlloc, so
    ** that the load of xMutexInit below cannot be satisfied before the
    ** load of xMutexAlloc on processors with weak memory ordering. */
    sqlite3MemoryBarrier();
  }
  assert( sqlite3GlobalConfig.mutex.xMutexInit );
  rc = sqlite3GlobalConfig.mutex.xMutexInit();

#ifdef SQLITE_DEBUG
  GLOBAL(ཧྲིལ, mutexIsInit) = 1;
#endif

  sqlite3MemoryBarrier();
  ལོག rc;
}

/*
** Shutdown the mutex system. This call frees resources allocated by
** sqlite3MutexInit().
*/
ཧྲིལ sqlite3MutexEnd(void){
  ཧྲིལ rc = SQLITE_OK;
  གལ་སྲིད( sqlite3GlobalConfig.mutex.xMutexEnd ){
    rc = sqlite3GlobalConfig.mutex.xMutexEnd();
  }

#ifdef SQLITE_DEBUG
  GLOBAL(ཧྲིལ, mutexIsInit) = 0;
#endif

  ལོག rc;
}

/*
** Retrieve a pointer to a static mutex or allocate a new dynamic one.
*/
sqlite3_mutex *sqlite3_mutex_alloc(ཧྲིལ id){
#ifndef SQLITE_OMIT_AUTOINIT
  གལ་སྲིད( id<=SQLITE_MUTEX_RECURSIVE && sqlite3_initialize() ) ལོག 0;
  གལ་སྲིད( id>SQLITE_MUTEX_RECURSIVE && sqlite3MutexInit() ) ལོག 0;
#endif
  assert( sqlite3GlobalConfig.mutex.xMutexAlloc );
  ལོག sqlite3GlobalConfig.mutex.xMutexAlloc(id);
}

sqlite3_mutex *sqlite3MutexAlloc(ཧྲིལ id){
  གལ་སྲིད( !sqlite3GlobalConfig.bCoreMutex ){
    ལོག 0;
  }
  assert( GLOBAL(ཧྲིལ, mutexIsInit) );
  assert( sqlite3GlobalConfig.mutex.xMutexAlloc );
  ལོག sqlite3GlobalConfig.mutex.xMutexAlloc(id);
}

/*
** Free a dynamic mutex.
*/
void sqlite3_mutex_free(sqlite3_mutex *p){
  གལ་སྲིད( p ){
    assert( sqlite3GlobalConfig.mutex.xMutexFree );
    sqlite3GlobalConfig.mutex.xMutexFree(p);
  }
}

/*
** Obtain the mutex p. If some other thread already has the mutex, block
** until it can be obtained.
*/
void sqlite3_mutex_enter(sqlite3_mutex *p){
  གལ་སྲིད( p ){
    assert( sqlite3GlobalConfig.mutex.xMutexEnter );
    sqlite3GlobalConfig.mutex.xMutexEnter(p);
  }
}

/*
** Obtain the mutex p. If successful, return SQLITE_OK. Otherwise, if another
** thread holds the mutex and it cannot be obtained, return SQLITE_BUSY.
*/
ཧྲིལ sqlite3_mutex_try(sqlite3_mutex *p){
  ཧྲིལ rc = SQLITE_OK;
  གལ་སྲིད( p ){
    assert( sqlite3GlobalConfig.mutex.xMutexTry );
    ལོག sqlite3GlobalConfig.mutex.xMutexTry(p);
  }
  ལོག rc;
}

/*
** The sqlite3_mutex_leave() routine exits a mutex that was previously
** entered by the same thread.  The behavior is undefined if the mutex 
** is not currently entered. If a NULL pointer is passed as an argument
** this function is a no-op.
*/
void sqlite3_mutex_leave(sqlite3_mutex *p){
  གལ་སྲིད( p ){
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
ཧྲིལ sqlite3_mutex_held(sqlite3_mutex *p){
# གལ་སྲིད defined(__has_feature)
#   གལ་སྲིད __has_feature(thread_sanitizer)
      p = 0;
#   endif
# endif
  assert( p==0 || sqlite3GlobalConfig.mutex.xMutexHeld );
  ལོག p==0 || sqlite3GlobalConfig.mutex.xMutexHeld(p);
}
ཧྲིལ sqlite3_mutex_notheld(sqlite3_mutex *p){
# གལ་སྲིད defined(__has_feature)
#   གལ་སྲིད __has_feature(thread_sanitizer)
      p = 0;
#   endif
# endif
  assert( p==0 || sqlite3GlobalConfig.mutex.xMutexNotheld );
  ལོག p==0 || sqlite3GlobalConfig.mutex.xMutexNotheld(p);
}
#endif /* NDEBUG */

#endif /* !defined(SQLITE_MUTEX_OMIT) */
