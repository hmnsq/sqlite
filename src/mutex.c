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

#إذا defined(SQLITE_DEBUG) && !defined(SQLITE_MUTEX_OMIT)
/*
** For debugging purposes, record when the mutex subsystem is initialized
** and uninitialized so that we can assert() if there is an attempt to
** allocate a mutex while the system is uninitialized.
*/
ساكن SQLITE_WSD صحيح mutexIsInit = 0;
#جے_انت /* SQLITE_DEBUG && !defined(SQLITE_MUTEX_OMIT) */


#جے_بنا_تعریف SQLITE_MUTEX_OMIT

#جے_تعریف_شدہ SQLITE_THREAD_MISUSE_WARNINGS
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

#تعریف_کرو SQLITE_MUTEX_WARNONCONTENTION  (-1)

/* 
** Pointer to real mutex methods object used by the CheckMutex
** implementation. Set by checkMutexInit(). 
*/
ساكن SQLITE_WSD ثابت sqlite3_mutex_methods *pGlobalMutexMethods;

#جے_تعریف_شدہ SQLITE_DEBUG
ساكن صحيح checkMutexHeld(sqlite3_mutex *p){
  أرجع pGlobalMutexMethods->xMutexHeld(((CheckMutex*)p)->mutex);
}
ساكن صحيح checkMutexNotheld(sqlite3_mutex *p){
  أرجع pGlobalMutexMethods->xMutexNotheld(((CheckMutex*)p)->mutex);
}
#جے_انت

/*
** Initialize and deinitialize the mutex subsystem.
*/
ساكن صحيح checkMutexInit(فراغ){ 
  pGlobalMutexMethods = sqlite3DefaultMutex();
  أرجع pGlobalMutexMethods->xMutexInit(); 
}
ساكن صحيح check_Mutex_سرا(فراغ){ 
  صحيح rc = pGlobalMutexMethods->x_Mutex_سرا(); 
  pGlobalMutexMethods = 0;
  أرجع rc;
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
  إذا( iType<2 ){
    p = sqlite3MallocZero(حجم(CheckMutex));
    إذا( p==0 ) أرجع 0;
    p->iType = iType;
  }وإلا{
#جے_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
    إذا( iType-2>=Array_ناپ(staticMutexes) ){
      (فراغ)SQLITE_MISUSE_BKPT;
      أرجع 0;
    }
#جے_انت
    p = &staticMutexes[iType-2];
  }

  إذا( p->mutex==0 ){
    p->mutex = pGlobalMutexMethods->xMutexAlloc(iType);
    إذا( p->mutex==0 ){
      إذا( iType<2 ){
        sqlite3_free(p);
      }
      p = 0;
    }
  }

  أرجع (sqlite3_mutex*)p;
}

/*
** Free a mutex.
*/
ساكن فراغ checkMutexFree(sqlite3_mutex *p){
  أكد( SQLITE_MUTEX_RECURSIVE<2 );
  أكد( SQLITE_MUTEX_FAST<2 );
  أكد( SQLITE_MUTEX_WARNONCONTENTION<2 );

#جے_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  إذا( ((CheckMutex*)p)->iType<2 )
#جے_انت
  {
    CheckMutex *pCheck = (CheckMutex*)p;
    pGlobalMutexMethods->xMutexFree(pCheck->mutex);
    sqlite3_free(pCheck);
  }
#جے_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  وإلا{
    (فراغ)SQLITE_MISUSE_BKPT;
  }
#جے_انت
}

/*
** Enter the mutex.
*/
ساكن فراغ checkMutexEnter(sqlite3_mutex *p){
  CheckMutex *pCheck = (CheckMutex*)p;
  إذا( pCheck->iType==SQLITE_MUTEX_WARNONCONTENTION ){
    إذا( SQLITE_OK==pGlobalMutexMethods->xMutexTry(pCheck->mutex) ){
      أرجع;
    }
    sqlite3_log(SQLITE_MISUSE, 
        "illegal multi-threaded access to database connection"
    );
#إذا SQLITE_THREAD_MISUSE_ABORT
    abort();
#جے_انت
  }
  pGlobalMutexMethods->xMutexEnter(pCheck->mutex);
}

/*
** Enter the mutex (do not block).
*/
ساكن صحيح checkMutexTry(sqlite3_mutex *p){
  CheckMutex *pCheck = (CheckMutex*)p;
  أرجع pGlobalMutexMethods->xMutexTry(pCheck->mutex);
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
    check_Mutex_سرا,
    checkMutexAlloc,
    checkMutexFree,
    checkMutexEnter,
    checkMutexTry,
    checkMutexLeave,
#جے_تعریف_شدہ SQLITE_DEBUG
    checkMutexHeld,
    checkMutexNotheld
#وإلا
    0,
    0
#جے_انت
  };
  أرجع &sMutex;
}

/*
** Mark the SQLITE_MUTEX_RECURSIVE mutex passed as the only argument as
** one on which there should be no contention.
*/
فراغ sqlite3MutexWarnOnContention(sqlite3_mutex *p){
  إذا( sqlite3GlobalConfig.mutex.xMutexAlloc==checkMutexAlloc ){
    CheckMutex *pCheck = (CheckMutex*)p;
    أكد( pCheck->iType==SQLITE_MUTEX_RECURSIVE );
    pCheck->iType = SQLITE_MUTEX_WARNONCONTENTION;
  }
}
#جے_انت   /* ifdef SQLITE_THREAD_MISUSE_WARNINGS */

/*
** Initialize the mutex system.
*/
صحيح sqlite3MutexInit(فراغ){ 
  صحيح rc = SQLITE_OK;
  إذا( !sqlite3GlobalConfig.mutex.xMutexAlloc ){
    /* If the xMutexAlloc method has not been set, then the user did not
    ** install a mutex implementation via sqlite3_config() prior to 
    ** sqlite3_initialize() being called. This block copies pointers to
    ** the default implementation into the sqlite3GlobalConfig structure.
    */
    sqlite3_mutex_methods ثابت *pFrom;
    sqlite3_mutex_methods *p_منزل = &sqlite3GlobalConfig.mutex;

    إذا( sqlite3GlobalConfig.bCoreMutex ){
#جے_تعریف_شدہ SQLITE_THREAD_MISUSE_WARNINGS
      pFrom = multiThreadedCheckMutex();
#وإلا
      pFrom = sqlite3DefaultMutex();
#جے_انت
    }وإلا{
      pFrom = sqlite3NoopMutex();
    }
    p_منزل->xMutexInit = pFrom->xMutexInit;
    p_منزل->x_Mutex_سرا = pFrom->x_Mutex_سرا;
    p_منزل->xMutexFree = pFrom->xMutexFree;
    p_منزل->xMutexEnter = pFrom->xMutexEnter;
    p_منزل->xMutexTry = pFrom->xMutexTry;
    p_منزل->xMutexLeave = pFrom->xMutexLeave;
    p_منزل->xMutexHeld = pFrom->xMutexHeld;
    p_منزل->xMutexNotheld = pFrom->xMutexNotheld;
    sqlite3_حافظہ_Barrier();
    p_منزل->xMutexAlloc = pFrom->xMutexAlloc;
  }وإلا{
    /* Another thread may have just installed the default methods above.
    ** This barrier pairs with the one prior to setting xMutexAlloc, so
    ** that the load of xMutexInit below cannot be satisfied before the
    ** load of xMutexAlloc on processors with weak memory ordering. */
    sqlite3_حافظہ_Barrier();
  }
  أكد( sqlite3GlobalConfig.mutex.xMutexInit );
  rc = sqlite3GlobalConfig.mutex.xMutexInit();

#جے_تعریف_شدہ SQLITE_DEBUG
  GLOBAL(صحيح, mutexIsInit) = 1;
#جے_انت

  sqlite3_حافظہ_Barrier();
  أرجع rc;
}

/*
** Shutdown the mutex system. This call frees resources allocated by
** sqlite3MutexInit().
*/
صحيح sqlite3_Mutex_سرا(فراغ){
  صحيح rc = SQLITE_OK;
  إذا( sqlite3GlobalConfig.mutex.x_Mutex_سرا ){
    rc = sqlite3GlobalConfig.mutex.x_Mutex_سرا();
  }

#جے_تعریف_شدہ SQLITE_DEBUG
  GLOBAL(صحيح, mutexIsInit) = 0;
#جے_انت

  أرجع rc;
}

/*
** Retrieve a pointer to a static mutex or allocate a new dynamic one.
*/
sqlite3_mutex *sqlite3_mutex_alloc(صحيح id){
#جے_بنا_تعریف SQLITE_OMIT_AUTOINIT
  إذا( id<=SQLITE_MUTEX_RECURSIVE && sqlite3_initialize() ) أرجع 0;
  إذا( id>SQLITE_MUTEX_RECURSIVE && sqlite3MutexInit() ) أرجع 0;
#جے_انت
  أكد( sqlite3GlobalConfig.mutex.xMutexAlloc );
  أرجع sqlite3GlobalConfig.mutex.xMutexAlloc(id);
}

sqlite3_mutex *sqlite3MutexAlloc(صحيح id){
  إذا( !sqlite3GlobalConfig.bCoreMutex ){
    أرجع 0;
  }
  أكد( GLOBAL(صحيح, mutexIsInit) );
  أكد( sqlite3GlobalConfig.mutex.xMutexAlloc );
  أرجع sqlite3GlobalConfig.mutex.xMutexAlloc(id);
}

/*
** Free a dynamic mutex.
*/
فراغ sqlite3_mutex_free(sqlite3_mutex *p){
  إذا( p ){
    أكد( sqlite3GlobalConfig.mutex.xMutexFree );
    sqlite3GlobalConfig.mutex.xMutexFree(p);
  }
}

/*
** Obtain the mutex p. If some other thread already has the mutex, block
** until it can be obtained.
*/
فراغ sqlite3_mutex_enter(sqlite3_mutex *p){
  إذا( p ){
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
  إذا( p ){
    أكد( sqlite3GlobalConfig.mutex.xMutexTry );
    أرجع sqlite3GlobalConfig.mutex.xMutexTry(p);
  }
  أرجع rc;
}

/*
** The sqlite3_mutex_leave() routine exits a mutex that was previously
** entered by the same thread.  The behavior is undefined if the mutex 
** is not currently entered. If a NULL pointer is passed as an argument
** this function is a no-op.
*/
فراغ sqlite3_mutex_leave(sqlite3_mutex *p){
  إذا( p ){
    أكد( sqlite3GlobalConfig.mutex.xMutexLeave );
    sqlite3GlobalConfig.mutex.xMutexLeave(p);
  }
}

#جے_بنا_تعریف NDEBUG
/*
** The sqlite3_mutex_held() and sqlite3_mutex_notheld() routine are
** intended for use inside assert() statements.
**
** Because these routines raise false-positive alerts in TSAN, disable
** them (make them always return 1) when compiling with TSAN.
*/
صحيح sqlite3_mutex_held(sqlite3_mutex *p){
# إذا defined(__has_feature)
#   إذا __has_feature(thread_sanitizer)
      p = 0;
#   endif
# endif
  أكد( p==0 || sqlite3GlobalConfig.mutex.xMutexHeld );
  أرجع p==0 || sqlite3GlobalConfig.mutex.xMutexHeld(p);
}
صحيح sqlite3_mutex_notheld(sqlite3_mutex *p){
# إذا defined(__has_feature)
#   إذا __has_feature(thread_sanitizer)
      p = 0;
#   endif
# endif
  أكد( p==0 || sqlite3GlobalConfig.mutex.xMutexNotheld );
  أرجع p==0 || sqlite3GlobalConfig.mutex.xMutexNotheld(p);
}
#جے_انت /* NDEBUG */

#جے_انت /* !defined(SQLITE_MUTEX_OMIT) */
