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
ساکن SQLITE_WSD صحیح mutexIsInit = 0;
#اگر_ختم /* SQLITE_DEBUG && !defined(SQLITE_MUTEX_OMIT) */


#اگر_غیر_تعریف_شدہ SQLITE_MUTEX_OMIT

#اگر_تعریف_شدہ SQLITE_THREAD_MISUSE_WARNINGS
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
قسمتعریف ساخت CheckMutex CheckMutex;
ساخت CheckMutex {
  صحیح iType;
  sqlite3_mutex *mutex;
};

#تعریف_کرو SQLITE_MUTEX_WARNONCONTENTION  (-1)

/* 
** Pointer to real mutex methods object used by the CheckMutex
** implementation. Set by checkMutexInit(). 
*/
ساکن SQLITE_WSD ثابت sqlite3_mutex_methods *pGlobalMutexMethods;

#اگر_تعریف_شدہ SQLITE_DEBUG
ساکن صحیح checkMutexHeld(sqlite3_mutex *p){
  واپس pGlobalMutexMethods->xMutexHeld(((CheckMutex*)p)->mutex);
}
ساکن صحیح checkMutexNotheld(sqlite3_mutex *p){
  واپس pGlobalMutexMethods->xMutexNotheld(((CheckMutex*)p)->mutex);
}
#اگر_ختم

/*
** Initialize and deinitialize the mutex subsystem.
*/
ساکن صحیح checkMutexInit(خالی){ 
  pGlobalMutexMethods = sqlite3DefaultMutex();
  واپس pGlobalMutexMethods->xMutexInit(); 
}
ساکن صحیح check_Mutex_انتہا(خالی){ 
  صحیح rc = pGlobalMutexMethods->x_Mutex_انتہا(); 
  pGlobalMutexMethods = 0;
  واپس rc;
}

/*
** Allocate a mutex.
*/
ساکن sqlite3_mutex *checkMutexAlloc(صحیح iType){
  ساکن CheckMutex staticMutexes[] = {
    {2, 0}, {3, 0}, {4, 0}, {5, 0},
    {6, 0}, {7, 0}, {8, 0}, {9, 0},
    {10, 0}, {11, 0}, {12, 0}, {13, 0}
  };
  CheckMutex *p = 0;

  أكد( SQLITE_MUTEX_RECURSIVE==1 && SQLITE_MUTEX_FAST==0 );
  اگر( iType<2 ){
    p = sqlite3MallocZero(حجم(CheckMutex));
    اگر( p==0 ) واپس 0;
    p->iType = iType;
  }ورنہ{
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
    اگر( iType-2>=Array_سائز(staticMutexes) ){
      (خالی)SQLITE_MISUSE_BKPT;
      واپس 0;
    }
#اگر_ختم
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

  واپس (sqlite3_mutex*)p;
}

/*
** Free a mutex.
*/
ساکن خالی checkMutexFree(sqlite3_mutex *p){
  أكد( SQLITE_MUTEX_RECURSIVE<2 );
  أكد( SQLITE_MUTEX_FAST<2 );
  أكد( SQLITE_MUTEX_WARNONCONTENTION<2 );

#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( ((CheckMutex*)p)->iType<2 )
#اگر_ختم
  {
    CheckMutex *pCheck = (CheckMutex*)p;
    pGlobalMutexMethods->xMutexFree(pCheck->mutex);
    sqlite3_free(pCheck);
  }
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  ورنہ{
    (خالی)SQLITE_MISUSE_BKPT;
  }
#اگر_ختم
}

/*
** Enter the mutex.
*/
ساکن خالی checkMutexEnter(sqlite3_mutex *p){
  CheckMutex *pCheck = (CheckMutex*)p;
  اگر( pCheck->iType==SQLITE_MUTEX_WARNONCONTENTION ){
    اگر( SQLITE_OK==pGlobalMutexMethods->xMutexTry(pCheck->mutex) ){
      واپس;
    }
    sqlite3_log(SQLITE_MISUSE, 
        "illegal multi-threaded access to database connection"
    );
#اگر SQLITE_THREAD_MISUSE_ABORT
    abort();
#اگر_ختم
  }
  pGlobalMutexMethods->xMutexEnter(pCheck->mutex);
}

/*
** Enter the mutex (do not block).
*/
ساکن صحیح checkMutexTry(sqlite3_mutex *p){
  CheckMutex *pCheck = (CheckMutex*)p;
  واپس pGlobalMutexMethods->xMutexTry(pCheck->mutex);
}

/*
** Leave the mutex.
*/
ساکن خالی checkMutexLeave(sqlite3_mutex *p){
  CheckMutex *pCheck = (CheckMutex*)p;
  pGlobalMutexMethods->xMutexLeave(pCheck->mutex);
}

sqlite3_mutex_methods ثابت *multiThreadedCheckMutex(خالی){
  ساکن ثابت sqlite3_mutex_methods sMutex = {
    checkMutexInit,
    check_Mutex_انتہا,
    checkMutexAlloc,
    checkMutexFree,
    checkMutexEnter,
    checkMutexTry,
    checkMutexLeave,
#اگر_تعریف_شدہ SQLITE_DEBUG
    checkMutexHeld,
    checkMutexNotheld
#ورنہ
    0,
    0
#اگر_ختم
  };
  واپس &sMutex;
}

/*
** Mark the SQLITE_MUTEX_RECURSIVE mutex passed as the only argument as
** one on which there should be no contention.
*/
خالی sqlite3MutexWarnOnContention(sqlite3_mutex *p){
  اگر( sqlite3GlobalConfig.mutex.xMutexAlloc==checkMutexAlloc ){
    CheckMutex *pCheck = (CheckMutex*)p;
    أكد( pCheck->iType==SQLITE_MUTEX_RECURSIVE );
    pCheck->iType = SQLITE_MUTEX_WARNONCONTENTION;
  }
}
#اگر_ختم   /* ifdef SQLITE_THREAD_MISUSE_WARNINGS */

/*
** Initialize the mutex system.
*/
صحیح sqlite3MutexInit(خالی){ 
  صحیح rc = SQLITE_OK;
  اگر( !sqlite3GlobalConfig.mutex.xMutexAlloc ){
    /* If the xMutexAlloc method has not been set, then the user did not
    ** install a mutex implementation via sqlite3_config() prior to 
    ** sqlite3_initialize() being called. This block copies pointers to
    ** the default implementation into the sqlite3GlobalConfig structure.
    */
    sqlite3_mutex_methods ثابت *pFrom;
    sqlite3_mutex_methods *p_مقصد = &sqlite3GlobalConfig.mutex;

    اگر( sqlite3GlobalConfig.bCoreMutex ){
#اگر_تعریف_شدہ SQLITE_THREAD_MISUSE_WARNINGS
      pFrom = multiThreadedCheckMutex();
#ورنہ
      pFrom = sqlite3DefaultMutex();
#اگر_ختم
    }ورنہ{
      pFrom = sqlite3NoopMutex();
    }
    p_مقصد->xMutexInit = pFrom->xMutexInit;
    p_مقصد->x_Mutex_انتہا = pFrom->x_Mutex_انتہا;
    p_مقصد->xMutexFree = pFrom->xMutexFree;
    p_مقصد->xMutexEnter = pFrom->xMutexEnter;
    p_مقصد->xMutexTry = pFrom->xMutexTry;
    p_مقصد->xMutexLeave = pFrom->xMutexLeave;
    p_مقصد->xMutexHeld = pFrom->xMutexHeld;
    p_مقصد->xMutexNotheld = pFrom->xMutexNotheld;
    sqlite3_حافظہ_Barrier();
    p_مقصد->xMutexAlloc = pFrom->xMutexAlloc;
  }ورنہ{
    /* Another thread may have just installed the default methods above.
    ** This barrier pairs with the one prior to setting xMutexAlloc, so
    ** that the load of xMutexInit below cannot be satisfied before the
    ** load of xMutexAlloc on processors with weak memory ordering. */
    sqlite3_حافظہ_Barrier();
  }
  أكد( sqlite3GlobalConfig.mutex.xMutexInit );
  rc = sqlite3GlobalConfig.mutex.xMutexInit();

#اگر_تعریف_شدہ SQLITE_DEBUG
  GLOBAL(صحیح, mutexIsInit) = 1;
#اگر_ختم

  sqlite3_حافظہ_Barrier();
  واپس rc;
}

/*
** Shutdown the mutex system. This call frees resources allocated by
** sqlite3MutexInit().
*/
صحیح sqlite3_Mutex_انتہا(خالی){
  صحیح rc = SQLITE_OK;
  اگر( sqlite3GlobalConfig.mutex.x_Mutex_انتہا ){
    rc = sqlite3GlobalConfig.mutex.x_Mutex_انتہا();
  }

#اگر_تعریف_شدہ SQLITE_DEBUG
  GLOBAL(صحیح, mutexIsInit) = 0;
#اگر_ختم

  واپس rc;
}

/*
** Retrieve a pointer to a static mutex or allocate a new dynamic one.
*/
sqlite3_mutex *sqlite3_mutex_alloc(صحیح id){
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOINIT
  اگر( id<=SQLITE_MUTEX_RECURSIVE && sqlite3_initialize() ) واپس 0;
  اگر( id>SQLITE_MUTEX_RECURSIVE && sqlite3MutexInit() ) واپس 0;
#اگر_ختم
  أكد( sqlite3GlobalConfig.mutex.xMutexAlloc );
  واپس sqlite3GlobalConfig.mutex.xMutexAlloc(id);
}

sqlite3_mutex *sqlite3MutexAlloc(صحیح id){
  اگر( !sqlite3GlobalConfig.bCoreMutex ){
    واپس 0;
  }
  أكد( GLOBAL(صحیح, mutexIsInit) );
  أكد( sqlite3GlobalConfig.mutex.xMutexAlloc );
  واپس sqlite3GlobalConfig.mutex.xMutexAlloc(id);
}

/*
** Free a dynamic mutex.
*/
خالی sqlite3_mutex_free(sqlite3_mutex *p){
  اگر( p ){
    أكد( sqlite3GlobalConfig.mutex.xMutexFree );
    sqlite3GlobalConfig.mutex.xMutexFree(p);
  }
}

/*
** Obtain the mutex p. If some other thread already has the mutex, block
** until it can be obtained.
*/
خالی sqlite3_mutex_enter(sqlite3_mutex *p){
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
    واپس sqlite3GlobalConfig.mutex.xMutexTry(p);
  }
  واپس rc;
}

/*
** The sqlite3_mutex_leave() routine exits a mutex that was previously
** entered by the same thread.  The behavior is undefined if the mutex 
** is not currently entered. If a NULL pointer is passed as an argument
** this function is a no-op.
*/
خالی sqlite3_mutex_leave(sqlite3_mutex *p){
  اگر( p ){
    أكد( sqlite3GlobalConfig.mutex.xMutexLeave );
    sqlite3GlobalConfig.mutex.xMutexLeave(p);
  }
}

#اگر_غیر_تعریف_شدہ NDEBUG
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
  واپس p==0 || sqlite3GlobalConfig.mutex.xMutexHeld(p);
}
صحیح sqlite3_mutex_notheld(sqlite3_mutex *p){
# اگر defined(__has_feature)
#   اگر __has_feature(thread_sanitizer)
      p = 0;
#   endif
# endif
  أكد( p==0 || sqlite3GlobalConfig.mutex.xMutexNotheld );
  واپس p==0 || sqlite3GlobalConfig.mutex.xMutexNotheld(p);
}
#اگر_ختم /* NDEBUG */

#اگر_ختم /* !defined(SQLITE_MUTEX_OMIT) */
