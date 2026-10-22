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
** This file contains the C functions that implement mutexes for Win32.
*/
#समावेश "sqliteInt.h"

#अगर SQLITE_OS_WIN
/*
** Include code that is common to all os_*.c files
*/
#समावेश "os_common.h"

/*
** Include the header file for the Windows VFS.
*/
#समावेश "os_win.h"
#पूर्ण_अगर

/*
** The code in this file is only used if we are compiling multithreaded
** on a Win32 system.
*/
#अगर_घोषित SQLITE_MUTEX_W32

#अगर MSVC_VERSION>0
# define ALIGN128 __declspec(align(128))
#अन्यथा
# define ALIGN128
#पूर्ण_अगर

#अगर_घोषित _MSC_VER
#आशय warning(धकेलो)
#आशय warning(disable: 4324)
#पूर्ण_अगर /* _MSC_VER */

/*
** Each SQLite mutex is an instance of the following structure.
**
** The ALIGN128 macro attempts to force 128-byte alignment on mutexes,
** so that adjacent mutex objects are always on different cache lines
** in the CPU.  This is CPU-dependent, of course, but 128-byte alignment
** seems to work well for all contemporary processors.  Experiments show
** that 64 works just as well most of the time, but the internet says
** that 128-byte alignment works better.
*/
काष्ठा sqlite3_mutex { 
  जोड़ {
    CRITICAL_SECTION cs;     /* The CRITICAL_SECTION mutex.  id==1 */
    SRWLOCK srwl;            /* The Slim reader/writer lock.  id!=1 */
  } u;
  पूर्णांक id;                    /* Mutex type */
#अगर_घोषित SQLITE_DEBUG
  अस्थिर पूर्णांक nRef;         /* Number of entrances */
  अस्थिर DWORD owner;      /* Thread holding this mutex */
  अस्थिर LONG trace;       /* True to trace changes */
#पूर्ण_अगर
};
#अगर_घोषित _MSC_VER
#आशय warning(निकालो)
#पूर्ण_अगर /* _MSC_VER */

#अगर_घोषित SQLITE_DEBUG
/*
** The sqlite3_mutex_held() and sqlite3_mutex_notheld() routine are
** intended for use only inside assert() statements.
*/
अटल पूर्णांक winMutexHeld(sqlite3_mutex *p){
  वापस p->nRef!=0 && p->owner==GetCurrentThreadId();
}

अटल पूर्णांक winMutexNotheld(sqlite3_mutex *p){
  वापस p->nRef==0 || p->owner!=GetCurrentThreadId();
}
#पूर्ण_अगर

/*
** Try to provide a memory barrier operation, needed for initialization
** and also for the xShmBarrier method of the VFS in cases when SQLite is
** compiled without mutexes (SQLITE_THREADSAFE=0).
*/
व्योम sqlite3MemoryBarrier(व्योम){
#अगर defined(SQLITE_MEMORY_BARRIER)
  SQLITE_MEMORY_BARRIER;
#या_अगर defined(__GNUC__)
  __sync_synchronize();
#या_अगर MSVC_VERSION>=1400
  _ReadWriteBarrier();
#या_अगर defined(MemoryBarrier)
  MemoryBarrier();
#पूर्ण_अगर
}

/*
** Static mutexes.
**
** The ALIGN128 macro on the static mutex array forces 128-byte alignment
** on the mutexes so that adjacent mutex objects are always on different
** cache lines in the CPU.  This is CPU-dependent, of course, but 128-byte
** alignment seems to work well for all contemporary processors.
** Experiments show that 64 works just as well most of the time, but
** the internet says that 128-byte alignment works better.
*/
अटल ALIGN128 जोड़ staticMutexes {
  sqlite3_mutex m;
  अक्षर spacer[128];
} aWindowsMutex[12];
अटल पूर्णांक winMutex_isInit = 0;

/* As the winMutexInit() and winMutexEnd() functions are called as part
** of the sqlite3_initialize() and sqlite3_shutdown() processing, the
** "interlocked" magic used here is probably not strictly necessary.
*/
अटल LONG अस्थिर winMutex_lock = 0;

व्योम sqlite3_win32_sleep(DWORD milliseconds); /* os_win.c */

अटल पूर्णांक winMutexInit(व्योम){
  /* The first to increment to 1 does actual initialization */
  अगर( InterlockedCompareExchange(&winMutex_lock, 1, 0)==0 ){
    पूर्णांक i;
    क्रम(i=0; i<Array_आकार(aWindowsMutex); i++){
      sqlite3_mutex *p = &aWindowsMutex[i].m;
      p->id = i+2;
      InitializeSRWLock(&p->u.srwl);
#अगर_घोषित SQLITE_DEBUG
      p->nRef = 0;
      p->owner = 0;
      p->trace = 0;
#पूर्ण_अगर
    }
    winMutex_isInit = 1;
  }अन्यथा{
    /* Another thread is (in the process of) initializing the static
    ** mutexes */
    जबतक( !winMutex_isInit ){
      sqlite3_win32_sleep(1);
    }
  }
  वापस SQLITE_OK;
}

अटल पूर्णांक winMutexEnd(व्योम){
  /* The first to decrement to 0 does actual shutdown
  ** (which should be the last to shutdown.) */
  अगर( InterlockedCompareExchange(&winMutex_lock, 0, 1)==1 ){
    अगर( winMutex_isInit==1 ){
      winMutex_isInit = 0;
    }
  }
  वापस SQLITE_OK;
}

/*
** The sqlite3_mutex_alloc() routine allocates a new
** mutex and returns a pointer to it.  If it returns NULL
** that means that a mutex could not be allocated.  SQLite
** will unwind its stack and return an error.  The argument
** to sqlite3_mutex_alloc() is one of these integer constants:
**
** <ul>
** <li>  SQLITE_MUTEX_FAST
** <li>  SQLITE_MUTEX_RECURSIVE
** <li>  SQLITE_MUTEX_STATIC_MAIN
** <li>  SQLITE_MUTEX_STATIC_MEM
** <li>  SQLITE_MUTEX_STATIC_OPEN
** <li>  SQLITE_MUTEX_STATIC_PRNG
** <li>  SQLITE_MUTEX_STATIC_LRU
** <li>  SQLITE_MUTEX_STATIC_PMEM
** <li>  SQLITE_MUTEX_STATIC_APP1
** <li>  SQLITE_MUTEX_STATIC_APP2
** <li>  SQLITE_MUTEX_STATIC_APP3
** <li>  SQLITE_MUTEX_STATIC_VFS1
** <li>  SQLITE_MUTEX_STATIC_VFS2
** <li>  SQLITE_MUTEX_STATIC_VFS3
** </ul>
**
** The first two constants cause sqlite3_mutex_alloc() to create
** a new mutex.  The new mutex is recursive when SQLITE_MUTEX_RECURSIVE
** is used but not necessarily so when SQLITE_MUTEX_FAST is used.
** The mutex implementation does not need to make a distinction
** between SQLITE_MUTEX_RECURSIVE and SQLITE_MUTEX_FAST if it does
** not want to.  But SQLite will only request a recursive mutex in
** cases where it really needs one.  If a faster non-recursive mutex
** implementation is available on the host platform, the mutex subsystem
** might return such a mutex in response to SQLITE_MUTEX_FAST.
**
** The other allowed parameters to sqlite3_mutex_alloc() each return
** a pointer to a static preexisting mutex.  Six static mutexes are
** used by the current version of SQLite.  Future versions of SQLite
** may add additional static mutexes.  Static mutexes are for internal
** use by SQLite only.  Applications that use SQLite mutexes should
** use only the dynamic mutexes returned by SQLITE_MUTEX_FAST or
** SQLITE_MUTEX_RECURSIVE.
**
** Note that if one of the dynamic mutex parameters (SQLITE_MUTEX_FAST
** or SQLITE_MUTEX_RECURSIVE) is used then sqlite3_mutex_alloc()
** returns a different mutex on every call.  But for the static
** mutex types, the same mutex is returned on every call that has
** the same type number.
*/
अटल sqlite3_mutex *winMutexAlloc(पूर्णांक iType){
  sqlite3_mutex *p;

  चयन( iType ){
    हाल SQLITE_MUTEX_FAST:
    हाल SQLITE_MUTEX_RECURSIVE: {
      p = sqlite3MallocZero( माप(*p) );
      अगर( p ){
        p->id = iType;
#अगर_घोषित SQLITE_DEBUG
#अगर_घोषित SQLITE_WIN32_MUTEX_TRACE_DYNAMIC
        p->trace = 1;
#पूर्ण_अगर
#पूर्ण_अगर
        अगर( iType==SQLITE_MUTEX_RECURSIVE ){
          InitializeCriticalSection(&p->u.cs);
        }अन्यथा{
          InitializeSRWLock(&p->u.srwl);
        }
      }
      अवरोध;
    }
    शेष: {
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
      अगर( iType-2<0 || iType-2>=Array_आकार(aWindowsMutex) ){
        (व्योम)SQLITE_MISUSE_BKPT;
        वापस 0;
      }
#पूर्ण_अगर
      p = &aWindowsMutex[iType-2].m;
#अगर_घोषित SQLITE_DEBUG
#अगर_घोषित SQLITE_WIN32_MUTEX_TRACE_STATIC
      InterlockedCompareExchange(&p->trace, 1, 0);
#पूर्ण_अगर
#पूर्ण_अगर
      अवरोध;
    }
  }
  निश्चित( p==0 || p->id==iType );
  वापस p;
}


/*
** This routine deallocates a previously
** allocated mutex.  SQLite is careful to deallocate every
** mutex that it allocates.
*/
अटल व्योम winMutexFree(sqlite3_mutex *p){
  निश्चित( p );
  निश्चित( p->nRef==0 && p->owner==0 );
  अगर( p->id==SQLITE_MUTEX_FAST ){
    sqlite3_free(p);
  }अन्यथा अगर( p->id==SQLITE_MUTEX_RECURSIVE ){
    DeleteCriticalSection(&p->u.cs);
    sqlite3_free(p);
  }अन्यथा{
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
    (व्योम)SQLITE_MISUSE_BKPT;
#पूर्ण_अगर
  }
}

/*
** The sqlite3_mutex_enter() and sqlite3_mutex_try() routines attempt
** to enter a mutex.  If another thread is already within the mutex,
** sqlite3_mutex_enter() will block and sqlite3_mutex_try() will return
** SQLITE_BUSY.  The sqlite3_mutex_try() interface returns SQLITE_OK
** upon successful entry.  Mutexes created using SQLITE_MUTEX_RECURSIVE can
** be entered multiple times by the same thread.  In such cases the,
** mutex must be exited an equal number of times before another thread
** can enter.  If the same thread tries to enter any other kind of mutex
** more than once, the behavior is undefined.
*/
अटल व्योम winMutexEnter(sqlite3_mutex *p){
  निश्चित( p );
#अगर_घोषित SQLITE_DEBUG
  निश्चित( p->id==SQLITE_MUTEX_RECURSIVE || winMutexNotheld(p) );
#पूर्ण_अगर
  निश्चित( winMutex_isInit==1 );
  अगर( p->id==SQLITE_MUTEX_RECURSIVE ){
    EnterCriticalSection(&p->u.cs);
  }अन्यथा{
    AcquireSRWLockExclusive(&p->u.srwl);
  }
#अगर_घोषित SQLITE_DEBUG
  निश्चित( p->nRef>0 || p->owner==0 );
  p->owner = GetCurrentThreadId();
  p->nRef++;
  अगर( p->trace ){
    OSTRACE(("ENTER-MUTEX tid=%lu, mutex(%d)=%p (%d), nRef=%d\n",
             GetCurrentThreadId(), p->id, p, p->trace, p->nRef));
  }
#पूर्ण_अगर
}

अटल पूर्णांक winMutexTry(sqlite3_mutex *p){
  पूर्णांक rc = SQLITE_BUSY;
  निश्चित( p );
  निश्चित( p->id==SQLITE_MUTEX_RECURSIVE || winMutexNotheld(p) );
  /*
  ** The sqlite3_mutex_try() routine is seldom used, and when it is
  ** used it is merely an optimization.  So it is OK for it to always
  ** fail.
  */
  अगर( p->id==SQLITE_MUTEX_RECURSIVE ){
    rc = TryEnterCriticalSection(&p->u.cs);
  }अन्यथा{
    rc = TryAcquireSRWLockExclusive(&p->u.srwl);
  }
  अगर( rc ){
#अगर_घोषित SQLITE_DEBUG
    p->owner = GetCurrentThreadId();
    p->nRef++;
#पूर्ण_अगर
    rc = SQLITE_OK;
  }अन्यथा{
    rc = SQLITE_BUSY;
  }
#अगर_घोषित SQLITE_DEBUG
  अगर( p->trace ){
    OSTRACE((
       "TRY-MUTEX tid=%lu, mutex(%d)=%p (%d), owner=%lu, nRef=%d, rc=%s\n",
       GetCurrentThreadId(), p->id, p, p->trace, p->owner, p->nRef,
       sqlite3ErrName(rc)));
  }
#पूर्ण_अगर
  वापस rc;
}

/*
** The sqlite3_mutex_leave() routine exits a mutex that was
** previously entered by the same thread.  The behavior
** is undefined if the mutex is not currently entered or
** is not currently allocated.  SQLite will never do either.
*/
अटल व्योम winMutexLeave(sqlite3_mutex *p){
  निश्चित( p );
#अगर_घोषित SQLITE_DEBUG
  निश्चित( p->nRef>0 );
  निश्चित( p->owner==GetCurrentThreadId() );
  p->nRef--;
  अगर( p->nRef==0 ) p->owner = 0;
  निश्चित( p->nRef==0 || p->id==SQLITE_MUTEX_RECURSIVE );
#पूर्ण_अगर
  निश्चित( winMutex_isInit==1 );
  अगर( p->id==SQLITE_MUTEX_RECURSIVE ){
    LeaveCriticalSection(&p->u.cs);
  }अन्यथा{
    ReleaseSRWLockExclusive(&p->u.srwl);
  }
#अगर_घोषित SQLITE_DEBUG
  अगर( p->trace ){
    OSTRACE(("LEAVE-MUTEX tid=%lu, mutex(%d)=%p (%d), nRef=%d\n",
             GetCurrentThreadId(), p->id, p, p->trace, p->nRef));
  }
#पूर्ण_अगर
}

sqlite3_mutex_methods स्थिर *sqlite3DefaultMutex(व्योम){
  अटल स्थिर sqlite3_mutex_methods sMutex = {
    winMutexInit,
    winMutexEnd,
    winMutexAlloc,
    winMutexFree,
    winMutexEnter,
    winMutexTry,
    winMutexLeave,
#अगर_घोषित SQLITE_DEBUG
    winMutexHeld,
    winMutexNotheld
#अन्यथा
    0,
    0
#पूर्ण_अगर
  };
  वापस &sMutex;
}

#पूर्ण_अगर /* SQLITE_MUTEX_W32 */
