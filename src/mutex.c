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
#समावेश "sqliteInt.h"

#अगर defined(SQLITE_DEBUG) && !defined(SQLITE_MUTEX_OMIT)
/*
** For debugging purposes, record when the mutex subsystem is initialized
** and uninitialized so that we can assert() if there is an attempt to
** allocate a mutex while the system is uninitialized.
*/
अटल SQLITE_WSD पूर्णांक mutexIsInit = 0;
#पूर्ण_अगर /* SQLITE_DEBUG && !defined(SQLITE_MUTEX_OMIT) */


#अगर_अघोषित SQLITE_MUTEX_OMIT

#अगर_घोषित SQLITE_THREAD_MISUSE_WARNINGS
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
प्रकार काष्ठा CheckMutex CheckMutex;
काष्ठा CheckMutex {
  पूर्णांक iType;
  sqlite3_mutex *mutex;
};

#घोषणा SQLITE_MUTEX_WARNONCONTENTION  (-1)

/* 
** Pointer to real mutex methods object used by the CheckMutex
** implementation. Set by checkMutexInit(). 
*/
अटल SQLITE_WSD स्थिर sqlite3_mutex_methods *pGlobalMutexMethods;

#अगर_घोषित SQLITE_DEBUG
अटल पूर्णांक checkMutexHeld(sqlite3_mutex *p){
  वापस pGlobalMutexMethods->xMutexHeld(((CheckMutex*)p)->mutex);
}
अटल पूर्णांक checkMutexNotheld(sqlite3_mutex *p){
  वापस pGlobalMutexMethods->xMutexNotheld(((CheckMutex*)p)->mutex);
}
#पूर्ण_अगर

/*
** Initialize and deinitialize the mutex subsystem.
*/
अटल पूर्णांक checkMutexInit(व्योम){ 
  pGlobalMutexMethods = sqlite3DefaultMutex();
  वापस pGlobalMutexMethods->xMutexInit(); 
}
अटल पूर्णांक checkMutexEnd(व्योम){ 
  पूर्णांक rc = pGlobalMutexMethods->xMutexEnd(); 
  pGlobalMutexMethods = 0;
  वापस rc;
}

/*
** Allocate a mutex.
*/
अटल sqlite3_mutex *checkMutexAlloc(पूर्णांक iType){
  अटल CheckMutex staticMutexes[] = {
    {2, 0}, {3, 0}, {4, 0}, {5, 0},
    {6, 0}, {7, 0}, {8, 0}, {9, 0},
    {10, 0}, {11, 0}, {12, 0}, {13, 0}
  };
  CheckMutex *p = 0;

  निश्चित( SQLITE_MUTEX_RECURSIVE==1 && SQLITE_MUTEX_FAST==0 );
  अगर( iType<2 ){
    p = sqlite3MallocZero(माप(CheckMutex));
    अगर( p==0 ) वापस 0;
    p->iType = iType;
  }अन्यथा{
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
    अगर( iType-2>=Array_आकार(staticMutexes) ){
      (व्योम)SQLITE_MISUSE_BKPT;
      वापस 0;
    }
#पूर्ण_अगर
    p = &staticMutexes[iType-2];
  }

  अगर( p->mutex==0 ){
    p->mutex = pGlobalMutexMethods->xMutexAlloc(iType);
    अगर( p->mutex==0 ){
      अगर( iType<2 ){
        sqlite3_free(p);
      }
      p = 0;
    }
  }

  वापस (sqlite3_mutex*)p;
}

/*
** Free a mutex.
*/
अटल व्योम checkMutexFree(sqlite3_mutex *p){
  निश्चित( SQLITE_MUTEX_RECURSIVE<2 );
  निश्चित( SQLITE_MUTEX_FAST<2 );
  निश्चित( SQLITE_MUTEX_WARNONCONTENTION<2 );

#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( ((CheckMutex*)p)->iType<2 )
#पूर्ण_अगर
  {
    CheckMutex *pCheck = (CheckMutex*)p;
    pGlobalMutexMethods->xMutexFree(pCheck->mutex);
    sqlite3_free(pCheck);
  }
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अन्यथा{
    (व्योम)SQLITE_MISUSE_BKPT;
  }
#पूर्ण_अगर
}

/*
** Enter the mutex.
*/
अटल व्योम checkMutexEnter(sqlite3_mutex *p){
  CheckMutex *pCheck = (CheckMutex*)p;
  अगर( pCheck->iType==SQLITE_MUTEX_WARNONCONTENTION ){
    अगर( SQLITE_OK==pGlobalMutexMethods->xMutexTry(pCheck->mutex) ){
      वापस;
    }
    sqlite3_log(SQLITE_MISUSE, 
        "illegal multi-threaded access to database connection"
    );
#अगर SQLITE_THREAD_MISUSE_ABORT
    पात();
#पूर्ण_अगर
  }
  pGlobalMutexMethods->xMutexEnter(pCheck->mutex);
}

/*
** Enter the mutex (do not block).
*/
अटल पूर्णांक checkMutexTry(sqlite3_mutex *p){
  CheckMutex *pCheck = (CheckMutex*)p;
  वापस pGlobalMutexMethods->xMutexTry(pCheck->mutex);
}

/*
** Leave the mutex.
*/
अटल व्योम checkMutexLeave(sqlite3_mutex *p){
  CheckMutex *pCheck = (CheckMutex*)p;
  pGlobalMutexMethods->xMutexLeave(pCheck->mutex);
}

sqlite3_mutex_methods स्थिर *multiThreadedCheckMutex(व्योम){
  अटल स्थिर sqlite3_mutex_methods sMutex = {
    checkMutexInit,
    checkMutexEnd,
    checkMutexAlloc,
    checkMutexFree,
    checkMutexEnter,
    checkMutexTry,
    checkMutexLeave,
#अगर_घोषित SQLITE_DEBUG
    checkMutexHeld,
    checkMutexNotheld
#अन्यथा
    0,
    0
#पूर्ण_अगर
  };
  वापस &sMutex;
}

/*
** Mark the SQLITE_MUTEX_RECURSIVE mutex passed as the only argument as
** one on which there should be no contention.
*/
व्योम sqlite3MutexWarnOnContention(sqlite3_mutex *p){
  अगर( sqlite3GlobalConfig.mutex.xMutexAlloc==checkMutexAlloc ){
    CheckMutex *pCheck = (CheckMutex*)p;
    निश्चित( pCheck->iType==SQLITE_MUTEX_RECURSIVE );
    pCheck->iType = SQLITE_MUTEX_WARNONCONTENTION;
  }
}
#पूर्ण_अगर   /* ifdef SQLITE_THREAD_MISUSE_WARNINGS */

/*
** Initialize the mutex system.
*/
पूर्णांक sqlite3MutexInit(व्योम){ 
  पूर्णांक rc = SQLITE_OK;
  अगर( !sqlite3GlobalConfig.mutex.xMutexAlloc ){
    /* If the xMutexAlloc method has not been set, then the user did not
    ** install a mutex implementation via sqlite3_config() prior to 
    ** sqlite3_initialize() being called. This block copies pointers to
    ** the default implementation into the sqlite3GlobalConfig structure.
    */
    sqlite3_mutex_methods स्थिर *pFrom;
    sqlite3_mutex_methods *pTo = &sqlite3GlobalConfig.mutex;

    अगर( sqlite3GlobalConfig.bCoreMutex ){
#अगर_घोषित SQLITE_THREAD_MISUSE_WARNINGS
      pFrom = multiThreadedCheckMutex();
#अन्यथा
      pFrom = sqlite3DefaultMutex();
#पूर्ण_अगर
    }अन्यथा{
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
  }अन्यथा{
    /* Another thread may have just installed the default methods above.
    ** This barrier pairs with the one prior to setting xMutexAlloc, so
    ** that the load of xMutexInit below cannot be satisfied before the
    ** load of xMutexAlloc on processors with weak memory ordering. */
    sqlite3MemoryBarrier();
  }
  निश्चित( sqlite3GlobalConfig.mutex.xMutexInit );
  rc = sqlite3GlobalConfig.mutex.xMutexInit();

#अगर_घोषित SQLITE_DEBUG
  GLOBAL(पूर्णांक, mutexIsInit) = 1;
#पूर्ण_अगर

  sqlite3MemoryBarrier();
  वापस rc;
}

/*
** Shutdown the mutex system. This call frees resources allocated by
** sqlite3MutexInit().
*/
पूर्णांक sqlite3MutexEnd(व्योम){
  पूर्णांक rc = SQLITE_OK;
  अगर( sqlite3GlobalConfig.mutex.xMutexEnd ){
    rc = sqlite3GlobalConfig.mutex.xMutexEnd();
  }

#अगर_घोषित SQLITE_DEBUG
  GLOBAL(पूर्णांक, mutexIsInit) = 0;
#पूर्ण_अगर

  वापस rc;
}

/*
** Retrieve a pointer to a static mutex or allocate a new dynamic one.
*/
sqlite3_mutex *sqlite3_mutex_alloc(पूर्णांक id){
#अगर_अघोषित SQLITE_OMIT_AUTOINIT
  अगर( id<=SQLITE_MUTEX_RECURSIVE && sqlite3_initialize() ) वापस 0;
  अगर( id>SQLITE_MUTEX_RECURSIVE && sqlite3MutexInit() ) वापस 0;
#पूर्ण_अगर
  निश्चित( sqlite3GlobalConfig.mutex.xMutexAlloc );
  वापस sqlite3GlobalConfig.mutex.xMutexAlloc(id);
}

sqlite3_mutex *sqlite3MutexAlloc(पूर्णांक id){
  अगर( !sqlite3GlobalConfig.bCoreMutex ){
    वापस 0;
  }
  निश्चित( GLOBAL(पूर्णांक, mutexIsInit) );
  निश्चित( sqlite3GlobalConfig.mutex.xMutexAlloc );
  वापस sqlite3GlobalConfig.mutex.xMutexAlloc(id);
}

/*
** Free a dynamic mutex.
*/
व्योम sqlite3_mutex_free(sqlite3_mutex *p){
  अगर( p ){
    निश्चित( sqlite3GlobalConfig.mutex.xMutexFree );
    sqlite3GlobalConfig.mutex.xMutexFree(p);
  }
}

/*
** Obtain the mutex p. If some other thread already has the mutex, block
** until it can be obtained.
*/
व्योम sqlite3_mutex_enter(sqlite3_mutex *p){
  अगर( p ){
    निश्चित( sqlite3GlobalConfig.mutex.xMutexEnter );
    sqlite3GlobalConfig.mutex.xMutexEnter(p);
  }
}

/*
** Obtain the mutex p. If successful, return SQLITE_OK. Otherwise, if another
** thread holds the mutex and it cannot be obtained, return SQLITE_BUSY.
*/
पूर्णांक sqlite3_mutex_try(sqlite3_mutex *p){
  पूर्णांक rc = SQLITE_OK;
  अगर( p ){
    निश्चित( sqlite3GlobalConfig.mutex.xMutexTry );
    वापस sqlite3GlobalConfig.mutex.xMutexTry(p);
  }
  वापस rc;
}

/*
** The sqlite3_mutex_leave() routine exits a mutex that was previously
** entered by the same thread.  The behavior is undefined if the mutex 
** is not currently entered. If a NULL pointer is passed as an argument
** this function is a no-op.
*/
व्योम sqlite3_mutex_leave(sqlite3_mutex *p){
  अगर( p ){
    निश्चित( sqlite3GlobalConfig.mutex.xMutexLeave );
    sqlite3GlobalConfig.mutex.xMutexLeave(p);
  }
}

#अगर_अघोषित न_संशोधन
/*
** The sqlite3_mutex_held() and sqlite3_mutex_notheld() routine are
** intended for use inside assert() statements.
**
** Because these routines raise false-positive alerts in TSAN, disable
** them (make them always return 1) when compiling with TSAN.
*/
पूर्णांक sqlite3_mutex_held(sqlite3_mutex *p){
# अगर defined(__has_feature)
#   अगर __has_feature(thread_sanitizer)
      p = 0;
#   endif
# endif
  निश्चित( p==0 || sqlite3GlobalConfig.mutex.xMutexHeld );
  वापस p==0 || sqlite3GlobalConfig.mutex.xMutexHeld(p);
}
पूर्णांक sqlite3_mutex_notheld(sqlite3_mutex *p){
# अगर defined(__has_feature)
#   अगर __has_feature(thread_sanitizer)
      p = 0;
#   endif
# endif
  निश्चित( p==0 || sqlite3GlobalConfig.mutex.xMutexNotheld );
  वापस p==0 || sqlite3GlobalConfig.mutex.xMutexNotheld(p);
}
#पूर्ण_अगर /* NDEBUG */

#पूर्ण_अगर /* !defined(SQLITE_MUTEX_OMIT) */
