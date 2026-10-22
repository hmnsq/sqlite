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
#அந்தர்பாவ "sqliteInt.h"

#எனில் defined(SQLITE_DEBUG) && !defined(SQLITE_MUTEX_OMIT)
/*
** For debugging purposes, record when the mutex subsystem is initialized
** and uninitialized so that we can assert() if there is an attempt to
** allocate a mutex while the system is uninitialized.
*/
நிலையான SQLITE_WSD முழு mutexIsInit = 0;
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
வகைவரை கட்டமைப்பு CheckMutex CheckMutex;
கட்டமைப்பு CheckMutex {
  முழு iType;
  sqlite3_mutex *mutex;
};

#define SQLITE_MUTEX_WARNONCONTENTION  (-1)

/* 
** Pointer to real mutex methods object used by the CheckMutex
** implementation. Set by checkMutexInit(). 
*/
நிலையான SQLITE_WSD மாறிலி sqlite3_mutex_methods *pGlobalMutexMethods;

#ifdef SQLITE_DEBUG
நிலையான முழு checkMutexHeld(sqlite3_mutex *p){
  திரும்பு pGlobalMutexMethods->xMutexHeld(((CheckMutex*)p)->mutex);
}
நிலையான முழு checkMutexNotheld(sqlite3_mutex *p){
  திரும்பு pGlobalMutexMethods->xMutexNotheld(((CheckMutex*)p)->mutex);
}
#endif

/*
** Initialize and deinitialize the mutex subsystem.
*/
நிலையான முழு checkMutexInit(வெறுமை){ 
  pGlobalMutexMethods = sqlite3DefaultMutex();
  திரும்பு pGlobalMutexMethods->xMutexInit(); 
}
நிலையான முழு checkMutexEnd(வெறுமை){ 
  முழு rc = pGlobalMutexMethods->xMutexEnd(); 
  pGlobalMutexMethods = 0;
  திரும்பு rc;
}

/*
** Allocate a mutex.
*/
நிலையான sqlite3_mutex *checkMutexAlloc(முழு iType){
  நிலையான CheckMutex staticMutexes[] = {
    {2, 0}, {3, 0}, {4, 0}, {5, 0},
    {6, 0}, {7, 0}, {8, 0}, {9, 0},
    {10, 0}, {11, 0}, {12, 0}, {13, 0}
  };
  CheckMutex *p = 0;

  ப்ரதிஜ்ஞா( SQLITE_MUTEX_RECURSIVE==1 && SQLITE_MUTEX_FAST==0 );
  எனில்( iType<2 ){
    p = sqlite3MallocZero(அளவு(CheckMutex));
    எனில்( p==0 ) திரும்பு 0;
    p->iType = iType;
  }இல்லையெனில்{
#ifdef SQLITE_ENABLE_API_ARMOR
    எனில்( iType-2>=ArraySize(staticMutexes) ){
      (வெறுமை)SQLITE_MISUSE_BKPT;
      திரும்பு 0;
    }
#endif
    p = &staticMutexes[iType-2];
  }

  எனில்( p->mutex==0 ){
    p->mutex = pGlobalMutexMethods->xMutexAlloc(iType);
    எனில்( p->mutex==0 ){
      எனில்( iType<2 ){
        sqlite3_free(p);
      }
      p = 0;
    }
  }

  திரும்பு (sqlite3_mutex*)p;
}

/*
** Free a mutex.
*/
நிலையான வெறுமை checkMutexFree(sqlite3_mutex *p){
  ப்ரதிஜ்ஞா( SQLITE_MUTEX_RECURSIVE<2 );
  ப்ரதிஜ்ஞா( SQLITE_MUTEX_FAST<2 );
  ப்ரதிஜ்ஞா( SQLITE_MUTEX_WARNONCONTENTION<2 );

#ifdef SQLITE_ENABLE_API_ARMOR
  எனில்( ((CheckMutex*)p)->iType<2 )
#endif
  {
    CheckMutex *pCheck = (CheckMutex*)p;
    pGlobalMutexMethods->xMutexFree(pCheck->mutex);
    sqlite3_free(pCheck);
  }
#ifdef SQLITE_ENABLE_API_ARMOR
  இல்லையெனில்{
    (வெறுமை)SQLITE_MISUSE_BKPT;
  }
#endif
}

/*
** Enter the mutex.
*/
நிலையான வெறுமை checkMutexEnter(sqlite3_mutex *p){
  CheckMutex *pCheck = (CheckMutex*)p;
  எனில்( pCheck->iType==SQLITE_MUTEX_WARNONCONTENTION ){
    எனில்( SQLITE_OK==pGlobalMutexMethods->xMutexTry(pCheck->mutex) ){
      திரும்பு;
    }
    sqlite3_log(SQLITE_MISUSE, 
        "illegal multi-threaded access to database connection"
    );
#எனில் SQLITE_THREAD_MISUSE_ABORT
    abort();
#endif
  }
  pGlobalMutexMethods->xMutexEnter(pCheck->mutex);
}

/*
** Enter the mutex (do not block).
*/
நிலையான முழு checkMutexTry(sqlite3_mutex *p){
  CheckMutex *pCheck = (CheckMutex*)p;
  திரும்பு pGlobalMutexMethods->xMutexTry(pCheck->mutex);
}

/*
** Leave the mutex.
*/
நிலையான வெறுமை checkMutexLeave(sqlite3_mutex *p){
  CheckMutex *pCheck = (CheckMutex*)p;
  pGlobalMutexMethods->xMutexLeave(pCheck->mutex);
}

sqlite3_mutex_methods மாறிலி *multiThreadedCheckMutex(வெறுமை){
  நிலையான மாறிலி sqlite3_mutex_methods sMutex = {
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
#இல்லையெனில்
    0,
    0
#endif
  };
  திரும்பு &sMutex;
}

/*
** Mark the SQLITE_MUTEX_RECURSIVE mutex passed as the only argument as
** one on which there should be no contention.
*/
வெறுமை sqlite3MutexWarnOnContention(sqlite3_mutex *p){
  எனில்( sqlite3GlobalConfig.mutex.xMutexAlloc==checkMutexAlloc ){
    CheckMutex *pCheck = (CheckMutex*)p;
    ப்ரதிஜ்ஞா( pCheck->iType==SQLITE_MUTEX_RECURSIVE );
    pCheck->iType = SQLITE_MUTEX_WARNONCONTENTION;
  }
}
#endif   /* ifdef SQLITE_THREAD_MISUSE_WARNINGS */

/*
** Initialize the mutex system.
*/
முழு sqlite3MutexInit(வெறுமை){ 
  முழு rc = SQLITE_OK;
  எனில்( !sqlite3GlobalConfig.mutex.xMutexAlloc ){
    /* If the xMutexAlloc method has not been set, then the user did not
    ** install a mutex implementation via sqlite3_config() prior to 
    ** sqlite3_initialize() being called. This block copies pointers to
    ** the default implementation into the sqlite3GlobalConfig structure.
    */
    sqlite3_mutex_methods மாறிலி *pFrom;
    sqlite3_mutex_methods *pTo = &sqlite3GlobalConfig.mutex;

    எனில்( sqlite3GlobalConfig.bCoreMutex ){
#ifdef SQLITE_THREAD_MISUSE_WARNINGS
      pFrom = multiThreadedCheckMutex();
#இல்லையெனில்
      pFrom = sqlite3DefaultMutex();
#endif
    }இல்லையெனில்{
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
  }இல்லையெனில்{
    /* Another thread may have just installed the default methods above.
    ** This barrier pairs with the one prior to setting xMutexAlloc, so
    ** that the load of xMutexInit below cannot be satisfied before the
    ** load of xMutexAlloc on processors with weak memory ordering. */
    sqlite3MemoryBarrier();
  }
  ப்ரதிஜ்ஞா( sqlite3GlobalConfig.mutex.xMutexInit );
  rc = sqlite3GlobalConfig.mutex.xMutexInit();

#ifdef SQLITE_DEBUG
  GLOBAL(முழு, mutexIsInit) = 1;
#endif

  sqlite3MemoryBarrier();
  திரும்பு rc;
}

/*
** Shutdown the mutex system. This call frees resources allocated by
** sqlite3MutexInit().
*/
முழு sqlite3MutexEnd(வெறுமை){
  முழு rc = SQLITE_OK;
  எனில்( sqlite3GlobalConfig.mutex.xMutexEnd ){
    rc = sqlite3GlobalConfig.mutex.xMutexEnd();
  }

#ifdef SQLITE_DEBUG
  GLOBAL(முழு, mutexIsInit) = 0;
#endif

  திரும்பு rc;
}

/*
** Retrieve a pointer to a static mutex or allocate a new dynamic one.
*/
sqlite3_mutex *sqlite3_mutex_alloc(முழு id){
#ifndef SQLITE_OMIT_AUTOINIT
  எனில்( id<=SQLITE_MUTEX_RECURSIVE && sqlite3_initialize() ) திரும்பு 0;
  எனில்( id>SQLITE_MUTEX_RECURSIVE && sqlite3MutexInit() ) திரும்பு 0;
#endif
  ப்ரதிஜ்ஞா( sqlite3GlobalConfig.mutex.xMutexAlloc );
  திரும்பு sqlite3GlobalConfig.mutex.xMutexAlloc(id);
}

sqlite3_mutex *sqlite3MutexAlloc(முழு id){
  எனில்( !sqlite3GlobalConfig.bCoreMutex ){
    திரும்பு 0;
  }
  ப்ரதிஜ்ஞா( GLOBAL(முழு, mutexIsInit) );
  ப்ரதிஜ்ஞா( sqlite3GlobalConfig.mutex.xMutexAlloc );
  திரும்பு sqlite3GlobalConfig.mutex.xMutexAlloc(id);
}

/*
** Free a dynamic mutex.
*/
வெறுமை sqlite3_mutex_free(sqlite3_mutex *p){
  எனில்( p ){
    ப்ரதிஜ்ஞா( sqlite3GlobalConfig.mutex.xMutexFree );
    sqlite3GlobalConfig.mutex.xMutexFree(p);
  }
}

/*
** Obtain the mutex p. If some other thread already has the mutex, block
** until it can be obtained.
*/
வெறுமை sqlite3_mutex_enter(sqlite3_mutex *p){
  எனில்( p ){
    ப்ரதிஜ்ஞா( sqlite3GlobalConfig.mutex.xMutexEnter );
    sqlite3GlobalConfig.mutex.xMutexEnter(p);
  }
}

/*
** Obtain the mutex p. If successful, return SQLITE_OK. Otherwise, if another
** thread holds the mutex and it cannot be obtained, return SQLITE_BUSY.
*/
முழு sqlite3_mutex_try(sqlite3_mutex *p){
  முழு rc = SQLITE_OK;
  எனில்( p ){
    ப்ரதிஜ்ஞா( sqlite3GlobalConfig.mutex.xMutexTry );
    திரும்பு sqlite3GlobalConfig.mutex.xMutexTry(p);
  }
  திரும்பு rc;
}

/*
** The sqlite3_mutex_leave() routine exits a mutex that was previously
** entered by the same thread.  The behavior is undefined if the mutex 
** is not currently entered. If a NULL pointer is passed as an argument
** this function is a no-op.
*/
வெறுமை sqlite3_mutex_leave(sqlite3_mutex *p){
  எனில்( p ){
    ப்ரதிஜ்ஞா( sqlite3GlobalConfig.mutex.xMutexLeave );
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
முழு sqlite3_mutex_held(sqlite3_mutex *p){
# எனில் defined(__has_feature)
#   எனில் __has_feature(thread_sanitizer)
      p = 0;
#   endif
# endif
  ப்ரதிஜ்ஞா( p==0 || sqlite3GlobalConfig.mutex.xMutexHeld );
  திரும்பு p==0 || sqlite3GlobalConfig.mutex.xMutexHeld(p);
}
முழு sqlite3_mutex_notheld(sqlite3_mutex *p){
# எனில் defined(__has_feature)
#   எனில் __has_feature(thread_sanitizer)
      p = 0;
#   endif
# endif
  ப்ரதிஜ்ஞா( p==0 || sqlite3GlobalConfig.mutex.xMutexNotheld );
  திரும்பு p==0 || sqlite3GlobalConfig.mutex.xMutexNotheld(p);
}
#endif /* NDEBUG */

#endif /* !defined(SQLITE_MUTEX_OMIT) */
