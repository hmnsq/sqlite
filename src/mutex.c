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
#అన్తర్భావ "sqliteInt.h"

#ఒకవేళ defined(SQLITE_DEBUG) && !defined(SQLITE_MUTEX_OMIT)
/*
** For debugging purposes, record when the mutex subsystem is initialized
** and uninitialized so that we can assert() if there is an attempt to
** allocate a mutex while the system is uninitialized.
*/
స్థిర SQLITE_WSD పూర్ణాంకం mutexIsInit = 0;
#అయితే_ముగింపు /* SQLITE_DEBUG && !defined(SQLITE_MUTEX_OMIT) */


#నిర్వచితం_కాకపోతే SQLITE_MUTEX_OMIT

#నిర్వచితమైతే SQLITE_THREAD_MISUSE_WARNINGS
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
రకనిర్వచనం నిర్మాణం CheckMutex CheckMutex;
నిర్మాణం CheckMutex {
  పూర్ణాంకం iType;
  sqlite3_mutex *mutex;
};

#నిర్వచించు SQLITE_MUTEX_WARNONCONTENTION  (-1)

/* 
** Pointer to real mutex methods object used by the CheckMutex
** implementation. Set by checkMutexInit(). 
*/
స్థిర SQLITE_WSD స్థిరరాశి sqlite3_mutex_methods *pGlobalMutexMethods;

#నిర్వచితమైతే SQLITE_DEBUG
స్థిర పూర్ణాంకం checkMutexHeld(sqlite3_mutex *p){
  తిరిగి pGlobalMutexMethods->xMutexHeld(((CheckMutex*)p)->mutex);
}
స్థిర పూర్ణాంకం checkMutexNotheld(sqlite3_mutex *p){
  తిరిగి pGlobalMutexMethods->xMutexNotheld(((CheckMutex*)p)->mutex);
}
#అయితే_ముగింపు

/*
** Initialize and deinitialize the mutex subsystem.
*/
స్థిర పూర్ణాంకం checkMutexInit(శూన్య){ 
  pGlobalMutexMethods = sqlite3DefaultMutex();
  తిరిగి pGlobalMutexMethods->xMutexInit(); 
}
స్థిర పూర్ణాంకం check_Mutex_చివర(శూన్య){ 
  పూర్ణాంకం rc = pGlobalMutexMethods->x_Mutex_చివర(); 
  pGlobalMutexMethods = 0;
  తిరిగి rc;
}

/*
** Allocate a mutex.
*/
స్థిర sqlite3_mutex *checkMutexAlloc(పూర్ణాంకం iType){
  స్థిర CheckMutex staticMutexes[] = {
    {2, 0}, {3, 0}, {4, 0}, {5, 0},
    {6, 0}, {7, 0}, {8, 0}, {9, 0},
    {10, 0}, {11, 0}, {12, 0}, {13, 0}
  };
  CheckMutex *p = 0;

  ప్రతిజ్ఞా( SQLITE_MUTEX_RECURSIVE==1 && SQLITE_MUTEX_FAST==0 );
  ఒకవేళ( iType<2 ){
    p = sqlite3MallocZero(పరిమాణం(CheckMutex));
    ఒకవేళ( p==0 ) తిరిగి 0;
    p->iType = iType;
  }లేకపోతే{
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
    ఒకవేళ( iType-2>=Array_కొలత(staticMutexes) ){
      (శూన్య)SQLITE_MISUSE_BKPT;
      తిరిగి 0;
    }
#అయితే_ముగింపు
    p = &staticMutexes[iType-2];
  }

  ఒకవేళ( p->mutex==0 ){
    p->mutex = pGlobalMutexMethods->xMutexAlloc(iType);
    ఒకవేళ( p->mutex==0 ){
      ఒకవేళ( iType<2 ){
        sqlite3_free(p);
      }
      p = 0;
    }
  }

  తిరిగి (sqlite3_mutex*)p;
}

/*
** Free a mutex.
*/
స్థిర శూన్య checkMutexFree(sqlite3_mutex *p){
  ప్రతిజ్ఞా( SQLITE_MUTEX_RECURSIVE<2 );
  ప్రతిజ్ఞా( SQLITE_MUTEX_FAST<2 );
  ప్రతిజ్ఞా( SQLITE_MUTEX_WARNONCONTENTION<2 );

#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( ((CheckMutex*)p)->iType<2 )
#అయితే_ముగింపు
  {
    CheckMutex *pCheck = (CheckMutex*)p;
    pGlobalMutexMethods->xMutexFree(pCheck->mutex);
    sqlite3_free(pCheck);
  }
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  లేకపోతే{
    (శూన్య)SQLITE_MISUSE_BKPT;
  }
#అయితే_ముగింపు
}

/*
** Enter the mutex.
*/
స్థిర శూన్య checkMutexEnter(sqlite3_mutex *p){
  CheckMutex *pCheck = (CheckMutex*)p;
  ఒకవేళ( pCheck->iType==SQLITE_MUTEX_WARNONCONTENTION ){
    ఒకవేళ( SQLITE_OK==pGlobalMutexMethods->xMutexTry(pCheck->mutex) ){
      తిరిగి;
    }
    sqlite3_log(SQLITE_MISUSE, 
        "illegal multi-threaded access to database connection"
    );
#ఒకవేళ SQLITE_THREAD_MISUSE_ABORT
    abort();
#అయితే_ముగింపు
  }
  pGlobalMutexMethods->xMutexEnter(pCheck->mutex);
}

/*
** Enter the mutex (do not block).
*/
స్థిర పూర్ణాంకం checkMutexTry(sqlite3_mutex *p){
  CheckMutex *pCheck = (CheckMutex*)p;
  తిరిగి pGlobalMutexMethods->xMutexTry(pCheck->mutex);
}

/*
** Leave the mutex.
*/
స్థిర శూన్య checkMutexLeave(sqlite3_mutex *p){
  CheckMutex *pCheck = (CheckMutex*)p;
  pGlobalMutexMethods->xMutexLeave(pCheck->mutex);
}

sqlite3_mutex_methods స్థిరరాశి *multiThreadedCheckMutex(శూన్య){
  స్థిర స్థిరరాశి sqlite3_mutex_methods sMutex = {
    checkMutexInit,
    check_Mutex_చివర,
    checkMutexAlloc,
    checkMutexFree,
    checkMutexEnter,
    checkMutexTry,
    checkMutexLeave,
#నిర్వచితమైతే SQLITE_DEBUG
    checkMutexHeld,
    checkMutexNotheld
#లేకపోతే
    0,
    0
#అయితే_ముగింపు
  };
  తిరిగి &sMutex;
}

/*
** Mark the SQLITE_MUTEX_RECURSIVE mutex passed as the only argument as
** one on which there should be no contention.
*/
శూన్య sqlite3MutexWarnOnContention(sqlite3_mutex *p){
  ఒకవేళ( sqlite3GlobalConfig.mutex.xMutexAlloc==checkMutexAlloc ){
    CheckMutex *pCheck = (CheckMutex*)p;
    ప్రతిజ్ఞా( pCheck->iType==SQLITE_MUTEX_RECURSIVE );
    pCheck->iType = SQLITE_MUTEX_WARNONCONTENTION;
  }
}
#అయితే_ముగింపు   /* ifdef SQLITE_THREAD_MISUSE_WARNINGS */

/*
** Initialize the mutex system.
*/
పూర్ణాంకం sqlite3MutexInit(శూన్య){ 
  పూర్ణాంకం rc = SQLITE_OK;
  ఒకవేళ( !sqlite3GlobalConfig.mutex.xMutexAlloc ){
    /* If the xMutexAlloc method has not been set, then the user did not
    ** install a mutex implementation via sqlite3_config() prior to 
    ** sqlite3_initialize() being called. This block copies pointers to
    ** the default implementation into the sqlite3GlobalConfig structure.
    */
    sqlite3_mutex_methods స్థిరరాశి *pFrom;
    sqlite3_mutex_methods *p_గమ్యం = &sqlite3GlobalConfig.mutex;

    ఒకవేళ( sqlite3GlobalConfig.bCoreMutex ){
#నిర్వచితమైతే SQLITE_THREAD_MISUSE_WARNINGS
      pFrom = multiThreadedCheckMutex();
#లేకపోతే
      pFrom = sqlite3DefaultMutex();
#అయితే_ముగింపు
    }లేకపోతే{
      pFrom = sqlite3NoopMutex();
    }
    p_గమ్యం->xMutexInit = pFrom->xMutexInit;
    p_గమ్యం->x_Mutex_చివర = pFrom->x_Mutex_చివర;
    p_గమ్యం->xMutexFree = pFrom->xMutexFree;
    p_గమ్యం->xMutexEnter = pFrom->xMutexEnter;
    p_గమ్యం->xMutexTry = pFrom->xMutexTry;
    p_గమ్యం->xMutexLeave = pFrom->xMutexLeave;
    p_గమ్యం->xMutexHeld = pFrom->xMutexHeld;
    p_గమ్యం->xMutexNotheld = pFrom->xMutexNotheld;
    sqlite3_జ్ఞాపకం_Barrier();
    p_గమ్యం->xMutexAlloc = pFrom->xMutexAlloc;
  }లేకపోతే{
    /* Another thread may have just installed the default methods above.
    ** This barrier pairs with the one prior to setting xMutexAlloc, so
    ** that the load of xMutexInit below cannot be satisfied before the
    ** load of xMutexAlloc on processors with weak memory ordering. */
    sqlite3_జ్ఞాపకం_Barrier();
  }
  ప్రతిజ్ఞా( sqlite3GlobalConfig.mutex.xMutexInit );
  rc = sqlite3GlobalConfig.mutex.xMutexInit();

#నిర్వచితమైతే SQLITE_DEBUG
  GLOBAL(పూర్ణాంకం, mutexIsInit) = 1;
#అయితే_ముగింపు

  sqlite3_జ్ఞాపకం_Barrier();
  తిరిగి rc;
}

/*
** Shutdown the mutex system. This call frees resources allocated by
** sqlite3MutexInit().
*/
పూర్ణాంకం sqlite3_Mutex_చివర(శూన్య){
  పూర్ణాంకం rc = SQLITE_OK;
  ఒకవేళ( sqlite3GlobalConfig.mutex.x_Mutex_చివర ){
    rc = sqlite3GlobalConfig.mutex.x_Mutex_చివర();
  }

#నిర్వచితమైతే SQLITE_DEBUG
  GLOBAL(పూర్ణాంకం, mutexIsInit) = 0;
#అయితే_ముగింపు

  తిరిగి rc;
}

/*
** Retrieve a pointer to a static mutex or allocate a new dynamic one.
*/
sqlite3_mutex *sqlite3_mutex_alloc(పూర్ణాంకం id){
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOINIT
  ఒకవేళ( id<=SQLITE_MUTEX_RECURSIVE && sqlite3_initialize() ) తిరిగి 0;
  ఒకవేళ( id>SQLITE_MUTEX_RECURSIVE && sqlite3MutexInit() ) తిరిగి 0;
#అయితే_ముగింపు
  ప్రతిజ్ఞా( sqlite3GlobalConfig.mutex.xMutexAlloc );
  తిరిగి sqlite3GlobalConfig.mutex.xMutexAlloc(id);
}

sqlite3_mutex *sqlite3MutexAlloc(పూర్ణాంకం id){
  ఒకవేళ( !sqlite3GlobalConfig.bCoreMutex ){
    తిరిగి 0;
  }
  ప్రతిజ్ఞా( GLOBAL(పూర్ణాంకం, mutexIsInit) );
  ప్రతిజ్ఞా( sqlite3GlobalConfig.mutex.xMutexAlloc );
  తిరిగి sqlite3GlobalConfig.mutex.xMutexAlloc(id);
}

/*
** Free a dynamic mutex.
*/
శూన్య sqlite3_mutex_free(sqlite3_mutex *p){
  ఒకవేళ( p ){
    ప్రతిజ్ఞా( sqlite3GlobalConfig.mutex.xMutexFree );
    sqlite3GlobalConfig.mutex.xMutexFree(p);
  }
}

/*
** Obtain the mutex p. If some other thread already has the mutex, block
** until it can be obtained.
*/
శూన్య sqlite3_mutex_enter(sqlite3_mutex *p){
  ఒకవేళ( p ){
    ప్రతిజ్ఞా( sqlite3GlobalConfig.mutex.xMutexEnter );
    sqlite3GlobalConfig.mutex.xMutexEnter(p);
  }
}

/*
** Obtain the mutex p. If successful, return SQLITE_OK. Otherwise, if another
** thread holds the mutex and it cannot be obtained, return SQLITE_BUSY.
*/
పూర్ణాంకం sqlite3_mutex_try(sqlite3_mutex *p){
  పూర్ణాంకం rc = SQLITE_OK;
  ఒకవేళ( p ){
    ప్రతిజ్ఞా( sqlite3GlobalConfig.mutex.xMutexTry );
    తిరిగి sqlite3GlobalConfig.mutex.xMutexTry(p);
  }
  తిరిగి rc;
}

/*
** The sqlite3_mutex_leave() routine exits a mutex that was previously
** entered by the same thread.  The behavior is undefined if the mutex 
** is not currently entered. If a NULL pointer is passed as an argument
** this function is a no-op.
*/
శూన్య sqlite3_mutex_leave(sqlite3_mutex *p){
  ఒకవేళ( p ){
    ప్రతిజ్ఞా( sqlite3GlobalConfig.mutex.xMutexLeave );
    sqlite3GlobalConfig.mutex.xMutexLeave(p);
  }
}

#నిర్వచితం_కాకపోతే NDEBUG
/*
** The sqlite3_mutex_held() and sqlite3_mutex_notheld() routine are
** intended for use inside assert() statements.
**
** Because these routines raise false-positive alerts in TSAN, disable
** them (make them always return 1) when compiling with TSAN.
*/
పూర్ణాంకం sqlite3_mutex_held(sqlite3_mutex *p){
# ఒకవేళ defined(__has_feature)
#   ఒకవేళ __has_feature(thread_sanitizer)
      p = 0;
#   endif
# endif
  ప్రతిజ్ఞా( p==0 || sqlite3GlobalConfig.mutex.xMutexHeld );
  తిరిగి p==0 || sqlite3GlobalConfig.mutex.xMutexHeld(p);
}
పూర్ణాంకం sqlite3_mutex_notheld(sqlite3_mutex *p){
# ఒకవేళ defined(__has_feature)
#   ఒకవేళ __has_feature(thread_sanitizer)
      p = 0;
#   endif
# endif
  ప్రతిజ్ఞా( p==0 || sqlite3GlobalConfig.mutex.xMutexNotheld );
  తిరిగి p==0 || sqlite3GlobalConfig.mutex.xMutexNotheld(p);
}
#అయితే_ముగింపు /* NDEBUG */

#అయితే_ముగింపు /* !defined(SQLITE_MUTEX_OMIT) */
