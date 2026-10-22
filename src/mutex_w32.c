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
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"

#ಯದಿ SQLITE_OS_WIN
/*
** Include code that is common to all os_*.c files
*/
#ಅನ್ತರ್ಭಾವ "os_common.h"

/*
** Include the header file for the Windows VFS.
*/
#ಅನ್ತರ್ಭಾವ "os_win.h"
#endif

/*
** The code in this file is only used if we are compiling multithreaded
** on a Win32 system.
*/
#ifdef SQLITE_MUTEX_W32

#ಯದಿ MSVC_VERSION>0
# define ALIGN128 __declspec(align(128))
#ಅನ್ಯಥಾ
# define ALIGN128
#endif

#ifdef _MSC_VER
#pragma warning(ತಳ್ಳು)
#pragma warning(disable: 4324)
#endif /* _MSC_VER */

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
ಸಂರಚನಾ sqlite3_mutex { 
  ಸಙ್ಘ {
    CRITICAL_SECTION cs;     /* The CRITICAL_SECTION mutex.  id==1 */
    SRWLOCK srwl;            /* The Slim reader/writer lock.  id!=1 */
  } u;
  ಪೂರ್ಣಾಙ್ಕ id;                    /* Mutex type */
#ifdef SQLITE_DEBUG
  ಚಞ್ಚಲ ಪೂರ್ಣಾಙ್ಕ nRef;         /* Number of entrances */
  ಚಞ್ಚಲ DWORD owner;      /* Thread holding this mutex */
  ಚಞ್ಚಲ LONG trace;       /* True to trace changes */
#endif
};
#ifdef _MSC_VER
#pragma warning(ತೆಗೆ)
#endif /* _MSC_VER */

#ifdef SQLITE_DEBUG
/*
** The sqlite3_mutex_held() and sqlite3_mutex_notheld() routine are
** intended for use only inside assert() statements.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ winMutexHeld(sqlite3_mutex *p){
  ಪ್ರತ್ಯಾಗಮ p->nRef!=0 && p->owner==GetCurrentThreadId();
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ winMutexNotheld(sqlite3_mutex *p){
  ಪ್ರತ್ಯಾಗಮ p->nRef==0 || p->owner!=GetCurrentThreadId();
}
#endif

/*
** Try to provide a memory barrier operation, needed for initialization
** and also for the xShmBarrier method of the VFS in cases when SQLite is
** compiled without mutexes (SQLITE_THREADSAFE=0).
*/
ಶೂನ್ಯ sqlite3MemoryBarrier(ಶೂನ್ಯ){
#ಯದಿ defined(SQLITE_MEMORY_BARRIER)
  SQLITE_MEMORY_BARRIER;
#ನೋಚೇದ್ಯದಿ defined(__GNUC__)
  __sync_synchronize();
#ನೋಚೇದ್ಯದಿ MSVC_VERSION>=1400
  _ReadWriteBarrier();
#ನೋಚೇದ್ಯದಿ defined(MemoryBarrier)
  MemoryBarrier();
#endif
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
ಸ್ಥಿರ ALIGN128 ಸಙ್ಘ staticMutexes {
  sqlite3_mutex m;
  ವರ್ಣ spacer[128];
} aWindowsMutex[12];
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ winMutex_isInit = 0;

/* As the winMutexInit() and winMutexEnd() functions are called as part
** of the sqlite3_initialize() and sqlite3_shutdown() processing, the
** "interlocked" magic used here is probably not strictly necessary.
*/
ಸ್ಥಿರ LONG ಚಞ್ಚಲ winMutex_lock = 0;

ಶೂನ್ಯ sqlite3_win32_sleep(DWORD milliseconds); /* os_win.c */

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ winMutexInit(ಶೂನ್ಯ){
  /* The first to increment to 1 does actual initialization */
  ಯದಿ( InterlockedCompareExchange(&winMutex_lock, 1, 0)==0 ){
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿ(i=0; i<ArraySize(aWindowsMutex); i++){
      sqlite3_mutex *p = &aWindowsMutex[i].m;
      p->id = i+2;
      InitializeSRWLock(&p->u.srwl);
#ifdef SQLITE_DEBUG
      p->nRef = 0;
      p->owner = 0;
      p->trace = 0;
#endif
    }
    winMutex_isInit = 1;
  }ಅನ್ಯಥಾ{
    /* Another thread is (in the process of) initializing the static
    ** mutexes */
    ಯಾವತ್( !winMutex_isInit ){
      sqlite3_win32_sleep(1);
    }
  }
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ winMutexEnd(ಶೂನ್ಯ){
  /* The first to decrement to 0 does actual shutdown
  ** (which should be the last to shutdown.) */
  ಯದಿ( InterlockedCompareExchange(&winMutex_lock, 0, 1)==1 ){
    ಯದಿ( winMutex_isInit==1 ){
      winMutex_isInit = 0;
    }
  }
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
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
ಸ್ಥಿರ sqlite3_mutex *winMutexAlloc(ಪೂರ್ಣಾಙ್ಕ iType){
  sqlite3_mutex *p;

  ವಿಕಲ್ಪ( iType ){
    ಪಕ್ಷ SQLITE_MUTEX_FAST:
    ಪಕ್ಷ SQLITE_MUTEX_RECURSIVE: {
      p = sqlite3MallocZero( ಪರಿಮಾಣ(*p) );
      ಯದಿ( p ){
        p->id = iType;
#ifdef SQLITE_DEBUG
#ifdef SQLITE_WIN32_MUTEX_TRACE_DYNAMIC
        p->trace = 1;
#endif
#endif
        ಯದಿ( iType==SQLITE_MUTEX_RECURSIVE ){
          InitializeCriticalSection(&p->u.cs);
        }ಅನ್ಯಥಾ{
          InitializeSRWLock(&p->u.srwl);
        }
      }
      ಭಙ್ಗ;
    }
    ಸ್ವಭಾವ: {
#ifdef SQLITE_ENABLE_API_ARMOR
      ಯದಿ( iType-2<0 || iType-2>=ArraySize(aWindowsMutex) ){
        (ಶೂನ್ಯ)SQLITE_MISUSE_BKPT;
        ಪ್ರತ್ಯಾಗಮ 0;
      }
#endif
      p = &aWindowsMutex[iType-2].m;
#ifdef SQLITE_DEBUG
#ifdef SQLITE_WIN32_MUTEX_TRACE_STATIC
      InterlockedCompareExchange(&p->trace, 1, 0);
#endif
#endif
      ಭಙ್ಗ;
    }
  }
  ಪ್ರತಿಜ್ಞಾ( p==0 || p->id==iType );
  ಪ್ರತ್ಯಾಗಮ p;
}


/*
** This routine deallocates a previously
** allocated mutex.  SQLite is careful to deallocate every
** mutex that it allocates.
*/
ಸ್ಥಿರ ಶೂನ್ಯ winMutexFree(sqlite3_mutex *p){
  ಪ್ರತಿಜ್ಞಾ( p );
  ಪ್ರತಿಜ್ಞಾ( p->nRef==0 && p->owner==0 );
  ಯದಿ( p->id==SQLITE_MUTEX_FAST ){
    sqlite3_free(p);
  }ಅನ್ಯಥಾ ಯದಿ( p->id==SQLITE_MUTEX_RECURSIVE ){
    DeleteCriticalSection(&p->u.cs);
    sqlite3_free(p);
  }ಅನ್ಯಥಾ{
#ifdef SQLITE_ENABLE_API_ARMOR
    (ಶೂನ್ಯ)SQLITE_MISUSE_BKPT;
#endif
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
ಸ್ಥಿರ ಶೂನ್ಯ winMutexEnter(sqlite3_mutex *p){
  ಪ್ರತಿಜ್ಞಾ( p );
#ifdef SQLITE_DEBUG
  ಪ್ರತಿಜ್ಞಾ( p->id==SQLITE_MUTEX_RECURSIVE || winMutexNotheld(p) );
#endif
  ಪ್ರತಿಜ್ಞಾ( winMutex_isInit==1 );
  ಯದಿ( p->id==SQLITE_MUTEX_RECURSIVE ){
    EnterCriticalSection(&p->u.cs);
  }ಅನ್ಯಥಾ{
    AcquireSRWLockExclusive(&p->u.srwl);
  }
#ifdef SQLITE_DEBUG
  ಪ್ರತಿಜ್ಞಾ( p->nRef>0 || p->owner==0 );
  p->owner = GetCurrentThreadId();
  p->nRef++;
  ಯದಿ( p->trace ){
    OSTRACE(("ENTER-MUTEX tid=%lu, mutex(%d)=%p (%d), nRef=%d\n",
             GetCurrentThreadId(), p->id, p, p->trace, p->nRef));
  }
#endif
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ winMutexTry(sqlite3_mutex *p){
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_BUSY;
  ಪ್ರತಿಜ್ಞಾ( p );
  ಪ್ರತಿಜ್ಞಾ( p->id==SQLITE_MUTEX_RECURSIVE || winMutexNotheld(p) );
  /*
  ** The sqlite3_mutex_try() routine is seldom used, and when it is
  ** used it is merely an optimization.  So it is OK for it to always
  ** fail.
  */
  ಯದಿ( p->id==SQLITE_MUTEX_RECURSIVE ){
    rc = TryEnterCriticalSection(&p->u.cs);
  }ಅನ್ಯಥಾ{
    rc = TryAcquireSRWLockExclusive(&p->u.srwl);
  }
  ಯದಿ( rc ){
#ifdef SQLITE_DEBUG
    p->owner = GetCurrentThreadId();
    p->nRef++;
#endif
    rc = SQLITE_OK;
  }ಅನ್ಯಥಾ{
    rc = SQLITE_BUSY;
  }
#ifdef SQLITE_DEBUG
  ಯದಿ( p->trace ){
    OSTRACE((
       "TRY-MUTEX tid=%lu, mutex(%d)=%p (%d), owner=%lu, nRef=%d, rc=%s\n",
       GetCurrentThreadId(), p->id, p, p->trace, p->owner, p->nRef,
       sqlite3ErrName(rc)));
  }
#endif
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** The sqlite3_mutex_leave() routine exits a mutex that was
** previously entered by the same thread.  The behavior
** is undefined if the mutex is not currently entered or
** is not currently allocated.  SQLite will never do either.
*/
ಸ್ಥಿರ ಶೂನ್ಯ winMutexLeave(sqlite3_mutex *p){
  ಪ್ರತಿಜ್ಞಾ( p );
#ifdef SQLITE_DEBUG
  ಪ್ರತಿಜ್ಞಾ( p->nRef>0 );
  ಪ್ರತಿಜ್ಞಾ( p->owner==GetCurrentThreadId() );
  p->nRef--;
  ಯದಿ( p->nRef==0 ) p->owner = 0;
  ಪ್ರತಿಜ್ಞಾ( p->nRef==0 || p->id==SQLITE_MUTEX_RECURSIVE );
#endif
  ಪ್ರತಿಜ್ಞಾ( winMutex_isInit==1 );
  ಯದಿ( p->id==SQLITE_MUTEX_RECURSIVE ){
    LeaveCriticalSection(&p->u.cs);
  }ಅನ್ಯಥಾ{
    ReleaseSRWLockExclusive(&p->u.srwl);
  }
#ifdef SQLITE_DEBUG
  ಯದಿ( p->trace ){
    OSTRACE(("LEAVE-MUTEX tid=%lu, mutex(%d)=%p (%d), nRef=%d\n",
             GetCurrentThreadId(), p->id, p, p->trace, p->nRef));
  }
#endif
}

sqlite3_mutex_methods ಧ್ರುವ *sqlite3DefaultMutex(ಶೂನ್ಯ){
  ಸ್ಥಿರ ಧ್ರುವ sqlite3_mutex_methods sMutex = {
    winMutexInit,
    winMutexEnd,
    winMutexAlloc,
    winMutexFree,
    winMutexEnter,
    winMutexTry,
    winMutexLeave,
#ifdef SQLITE_DEBUG
    winMutexHeld,
    winMutexNotheld
#ಅನ್ಯಥಾ
    0,
    0
#endif
  };
  ಪ್ರತ್ಯಾಗಮ &sMutex;
}

#endif /* SQLITE_MUTEX_W32 */
