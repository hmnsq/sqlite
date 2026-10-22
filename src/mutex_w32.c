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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"

#ਜੇ SQLITE_OS_WIN
/*
** Include code that is common to all os_*.c files
*/
#ਅਨ੍ਤਰ੍ਭਾਵ "os_common.h"

/*
** Include the header file for the Windows VFS.
*/
#ਅਨ੍ਤਰ੍ਭਾਵ "os_win.h"
#ਜੇ_ਅੰਤ

/*
** The code in this file is only used if we are compiling multithreaded
** on a Win32 system.
*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_MUTEX_W32

#ਜੇ MSVC_ਸੰਸਕਰਣ>0
# define ALIGN128 __declspec(align(128))
#ਨਹੀਂਤਾਂ
# define ALIGN128
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _MSC_VER
#ਹਦਾਇਤ warning(ਧੱਕੋ)
#ਹਦਾਇਤ warning(disable: 4324)
#ਜੇ_ਅੰਤ /* _MSC_VER */

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
ਬਣਤਰ sqlite3_mutex { 
  ਸੰਘ {
    CRITICAL_SECTION cs;     /* The CRITICAL_SECTION mutex.  id==1 */
    SRWLOCK srwl;            /* The Slim reader/writer lock.  id!=1 */
  } u;
  ਪੂਰਨ id;                    /* Mutex type */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  ਅਸਥਿਰ ਪੂਰਨ nRef;         /* Number of entrances */
  ਅਸਥਿਰ DWORD owner;      /* Thread holding this mutex */
  ਅਸਥਿਰ LONG trace;       /* True to trace changes */
#ਜੇ_ਅੰਤ
};
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _MSC_VER
#ਹਦਾਇਤ warning(ਕੱਢੋ)
#ਜੇ_ਅੰਤ /* _MSC_VER */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
/*
** The sqlite3_mutex_held() and sqlite3_mutex_notheld() routine are
** intended for use only inside assert() statements.
*/
ਸਥਿਰ ਪੂਰਨ winMutexHeld(sqlite3_mutex *p){
  ਵਾਪਸ p->nRef!=0 && p->owner==GetCurrentThreadId();
}

ਸਥਿਰ ਪੂਰਨ winMutexNotheld(sqlite3_mutex *p){
  ਵਾਪਸ p->nRef==0 || p->owner!=GetCurrentThreadId();
}
#ਜੇ_ਅੰਤ

/*
** Try to provide a memory barrier operation, needed for initialization
** and also for the xShmBarrier method of the VFS in cases when SQLite is
** compiled without mutexes (SQLITE_THREADSAFE=0).
*/
ਖਾਲੀ sqlite3_ਸਮ੍ਰਿਤੀ_Barrier(ਖਾਲੀ){
#ਜੇ defined(SQLITE_ਸਮ੍ਰਿਤੀ_BARRIER)
  SQLITE_ਸਮ੍ਰਿਤੀ_BARRIER;
#ਜਾਂਜੇ defined(__GNUC__)
  __sync_synchronize();
#ਜਾਂਜੇ MSVC_ਸੰਸਕਰਣ>=1400
  _ReadWriteBarrier();
#ਜਾਂਜੇ defined(ਸਮ੍ਰਿਤੀ_Barrier)
  ਸਮ੍ਰਿਤੀ_Barrier();
#ਜੇ_ਅੰਤ
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
ਸਥਿਰ ALIGN128 ਸੰਘ staticMutexes {
  sqlite3_mutex m;
  ਅੱਖਰ spacer[128];
} aWindowsMutex[12];
ਸਥਿਰ ਪੂਰਨ winMutex_isInit = 0;

/* As the winMutexInit() and winMutexEnd() functions are called as part
** of the sqlite3_initialize() and sqlite3_shutdown() processing, the
** "interlocked" magic used here is probably not strictly necessary.
*/
ਸਥਿਰ LONG ਅਸਥਿਰ win_Mutex_ਤਾਲਾ = 0;

ਖਾਲੀ sqlite3_win32_sleep(DWORD milliseconds); /* os_win.c */

ਸਥਿਰ ਪੂਰਨ winMutexInit(ਖਾਲੀ){
  /* The first to increment to 1 does actual initialization */
  ਜੇ( InterlockedCompareExchange(&win_Mutex_ਤਾਲਾ, 1, 0)==0 ){
    ਪੂਰਨ i;
    ਲਈ(i=0; i<Array_ਮਾਪ(aWindowsMutex); i++){
      sqlite3_mutex *p = &aWindowsMutex[i].m;
      p->id = i+2;
      InitializeSRWLock(&p->u.srwl);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
      p->nRef = 0;
      p->owner = 0;
      p->trace = 0;
#ਜੇ_ਅੰਤ
    }
    winMutex_isInit = 1;
  }ਨਹੀਂਤਾਂ{
    /* Another thread is (in the process of) initializing the static
    ** mutexes */
    ਜਦੋਂ( !winMutex_isInit ){
      sqlite3_win32_sleep(1);
    }
  }
  ਵਾਪਸ SQLITE_OK;
}

ਸਥਿਰ ਪੂਰਨ win_Mutex_ਸਿਰਾ(ਖਾਲੀ){
  /* The first to decrement to 0 does actual shutdown
  ** (which should be the last to shutdown.) */
  ਜੇ( InterlockedCompareExchange(&win_Mutex_ਤਾਲਾ, 0, 1)==1 ){
    ਜੇ( winMutex_isInit==1 ){
      winMutex_isInit = 0;
    }
  }
  ਵਾਪਸ SQLITE_OK;
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
ਸਥਿਰ sqlite3_mutex *winMutexAlloc(ਪੂਰਨ iType){
  sqlite3_mutex *p;

  ਚੋਣ( iType ){
    ਹਾਲਤ SQLITE_MUTEX_FAST:
    ਹਾਲਤ SQLITE_MUTEX_RECURSIVE: {
      p = sqlite3MallocZero( ਆਕਾਰ(*p) );
      ਜੇ( p ){
        p->id = iType;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_WIN32_MUTEX_TRACE_DYNAMIC
        p->trace = 1;
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
        ਜੇ( iType==SQLITE_MUTEX_RECURSIVE ){
          InitializeCriticalSection(&p->u.cs);
        }ਨਹੀਂਤਾਂ{
          InitializeSRWLock(&p->u.srwl);
        }
      }
      ਰੋਕੋ;
    }
    ਮੂਲ: {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
      ਜੇ( iType-2<0 || iType-2>=Array_ਮਾਪ(aWindowsMutex) ){
        (ਖਾਲੀ)SQLITE_MISUSE_BKPT;
        ਵਾਪਸ 0;
      }
#ਜੇ_ਅੰਤ
      p = &aWindowsMutex[iType-2].m;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_WIN32_MUTEX_TRACE_STATIC
      InterlockedCompareExchange(&p->trace, 1, 0);
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
      ਰੋਕੋ;
    }
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( p==0 || p->id==iType );
  ਵਾਪਸ p;
}


/*
** This routine deallocates a previously
** allocated mutex.  SQLite is careful to deallocate every
** mutex that it allocates.
*/
ਸਥਿਰ ਖਾਲੀ winMutexFree(sqlite3_mutex *p){
  ਪ੍ਰਤਿਜ੍ਞਾ( p );
  ਪ੍ਰਤਿਜ੍ਞਾ( p->nRef==0 && p->owner==0 );
  ਜੇ( p->id==SQLITE_MUTEX_FAST ){
    sqlite3_free(p);
  }ਨਹੀਂਤਾਂ ਜੇ( p->id==SQLITE_MUTEX_RECURSIVE ){
    DeleteCriticalSection(&p->u.cs);
    sqlite3_free(p);
  }ਨਹੀਂਤਾਂ{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
    (ਖਾਲੀ)SQLITE_MISUSE_BKPT;
#ਜੇ_ਅੰਤ
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
ਸਥਿਰ ਖਾਲੀ winMutexEnter(sqlite3_mutex *p){
  ਪ੍ਰਤਿਜ੍ਞਾ( p );
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  ਪ੍ਰਤਿਜ੍ਞਾ( p->id==SQLITE_MUTEX_RECURSIVE || winMutexNotheld(p) );
#ਜੇ_ਅੰਤ
  ਪ੍ਰਤਿਜ੍ਞਾ( winMutex_isInit==1 );
  ਜੇ( p->id==SQLITE_MUTEX_RECURSIVE ){
    EnterCriticalSection(&p->u.cs);
  }ਨਹੀਂਤਾਂ{
    AcquireSRWLockExclusive(&p->u.srwl);
  }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  ਪ੍ਰਤਿਜ੍ਞਾ( p->nRef>0 || p->owner==0 );
  p->owner = GetCurrentThreadId();
  p->nRef++;
  ਜੇ( p->trace ){
    OSTRACE(("ENTER-MUTEX tid=%lu, mutex(%d)=%p (%d), nRef=%d\n",
             GetCurrentThreadId(), p->id, p, p->trace, p->nRef));
  }
#ਜੇ_ਅੰਤ
}

ਸਥਿਰ ਪੂਰਨ winMutexTry(sqlite3_mutex *p){
  ਪੂਰਨ rc = SQLITE_BUSY;
  ਪ੍ਰਤਿਜ੍ਞਾ( p );
  ਪ੍ਰਤਿਜ੍ਞਾ( p->id==SQLITE_MUTEX_RECURSIVE || winMutexNotheld(p) );
  /*
  ** The sqlite3_mutex_try() routine is seldom used, and when it is
  ** used it is merely an optimization.  So it is OK for it to always
  ** fail.
  */
  ਜੇ( p->id==SQLITE_MUTEX_RECURSIVE ){
    rc = TryEnterCriticalSection(&p->u.cs);
  }ਨਹੀਂਤਾਂ{
    rc = TryAcquireSRWLockExclusive(&p->u.srwl);
  }
  ਜੇ( rc ){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
    p->owner = GetCurrentThreadId();
    p->nRef++;
#ਜੇ_ਅੰਤ
    rc = SQLITE_OK;
  }ਨਹੀਂਤਾਂ{
    rc = SQLITE_BUSY;
  }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  ਜੇ( p->trace ){
    OSTRACE((
       "TRY-MUTEX tid=%lu, mutex(%d)=%p (%d), owner=%lu, nRef=%d, rc=%s\n",
       GetCurrentThreadId(), p->id, p, p->trace, p->owner, p->nRef,
       sqlite3ErrName(rc)));
  }
#ਜੇ_ਅੰਤ
  ਵਾਪਸ rc;
}

/*
** The sqlite3_mutex_leave() routine exits a mutex that was
** previously entered by the same thread.  The behavior
** is undefined if the mutex is not currently entered or
** is not currently allocated.  SQLite will never do either.
*/
ਸਥਿਰ ਖਾਲੀ winMutexLeave(sqlite3_mutex *p){
  ਪ੍ਰਤਿਜ੍ਞਾ( p );
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  ਪ੍ਰਤਿਜ੍ਞਾ( p->nRef>0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p->owner==GetCurrentThreadId() );
  p->nRef--;
  ਜੇ( p->nRef==0 ) p->owner = 0;
  ਪ੍ਰਤਿਜ੍ਞਾ( p->nRef==0 || p->id==SQLITE_MUTEX_RECURSIVE );
#ਜੇ_ਅੰਤ
  ਪ੍ਰਤਿਜ੍ਞਾ( winMutex_isInit==1 );
  ਜੇ( p->id==SQLITE_MUTEX_RECURSIVE ){
    LeaveCriticalSection(&p->u.cs);
  }ਨਹੀਂਤਾਂ{
    ReleaseSRWLockExclusive(&p->u.srwl);
  }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  ਜੇ( p->trace ){
    OSTRACE(("LEAVE-MUTEX tid=%lu, mutex(%d)=%p (%d), nRef=%d\n",
             GetCurrentThreadId(), p->id, p, p->trace, p->nRef));
  }
#ਜੇ_ਅੰਤ
}

sqlite3_mutex_methods ਅਚਲ *sqlite3DefaultMutex(ਖਾਲੀ){
  ਸਥਿਰ ਅਚਲ sqlite3_mutex_methods sMutex = {
    winMutexInit,
    win_Mutex_ਸਿਰਾ,
    winMutexAlloc,
    winMutexFree,
    winMutexEnter,
    winMutexTry,
    winMutexLeave,
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
    winMutexHeld,
    winMutexNotheld
#ਨਹੀਂਤਾਂ
    0,
    0
#ਜੇ_ਅੰਤ
  };
  ਵਾਪਸ &sMutex;
}

#ਜੇ_ਅੰਤ /* SQLITE_MUTEX_W32 */
