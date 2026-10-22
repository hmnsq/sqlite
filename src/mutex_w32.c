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

#ಆದರೆ SQLITE_OS_WIN
/*
** Include code that is common to all os_*.c files
*/
#ಅನ್ತರ್ಭಾವ "os_common.h"

/*
** Include the header file for the Windows VFS.
*/
#ಅನ್ತರ್ಭಾವ "os_win.h"
#ಆದರೆ_ಅಂತ್ಯ

/*
** The code in this file is only used if we are compiling multithreaded
** on a Win32 system.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_MUTEX_W32

#ಆದರೆ MSVC_ಆವೃತ್ತಿ>0
# define ALIGN128 __declspec(align(128))
#ಇಲ್ಲದಿದ್ದರೆ
# define ALIGN128
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _MSC_VER
#ಸೂಚನೆ warning(ತಳ್ಳು)
#ಸೂಚನೆ warning(disable: 4324)
#ಆದರೆ_ಅಂತ್ಯ /* _MSC_VER */

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
ರಚನೆ sqlite3_mutex { 
  ಒಕ್ಕೂಟ {
    CRITICAL_SECTION cs;     /* The CRITICAL_SECTION mutex.  id==1 */
    SRWLOCK srwl;            /* The Slim reader/writer lock.  id!=1 */
  } u;
  ಪೂರ್ಣಾಂಕ id;                    /* Mutex type */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  ಅಸ್ಥಿರ ಪೂರ್ಣಾಂಕ nRef;         /* Number of entrances */
  ಅಸ್ಥಿರ DWORD owner;      /* Thread holding this mutex */
  ಅಸ್ಥಿರ LONG trace;       /* True to trace changes */
#ಆದರೆ_ಅಂತ್ಯ
};
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _MSC_VER
#ಸೂಚನೆ warning(ತೆಗೆ)
#ಆದರೆ_ಅಂತ್ಯ /* _MSC_VER */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
/*
** The sqlite3_mutex_held() and sqlite3_mutex_notheld() routine are
** intended for use only inside assert() statements.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ winMutexHeld(sqlite3_mutex *p){
  ಹಿಂತಿರುಗಿ p->nRef!=0 && p->owner==GetCurrentThreadId();
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ winMutexNotheld(sqlite3_mutex *p){
  ಹಿಂತಿರುಗಿ p->nRef==0 || p->owner!=GetCurrentThreadId();
}
#ಆದರೆ_ಅಂತ್ಯ

/*
** Try to provide a memory barrier operation, needed for initialization
** and also for the xShmBarrier method of the VFS in cases when SQLite is
** compiled without mutexes (SQLITE_THREADSAFE=0).
*/
ಶೂನ್ಯ sqlite3_ಸ್ಮರಣೆ_Barrier(ಶೂನ್ಯ){
#ಆದರೆ defined(SQLITE_ಸ್ಮರಣೆ_BARRIER)
  SQLITE_ಸ್ಮರಣೆ_BARRIER;
#ಅಥವಾಆದರೆ defined(__GNUC__)
  __sync_synchronize();
#ಅಥವಾಆದರೆ MSVC_ಆವೃತ್ತಿ>=1400
  _ReadWriteBarrier();
#ಅಥವಾಆದರೆ defined(ಸ್ಮರಣೆ_Barrier)
  ಸ್ಮರಣೆ_Barrier();
#ಆದರೆ_ಅಂತ್ಯ
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
ಸ್ಥಿರ ALIGN128 ಒಕ್ಕೂಟ staticMutexes {
  sqlite3_mutex m;
  ಅಕ್ಷರ spacer[128];
} aWindowsMutex[12];
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ winMutex_isInit = 0;

/* As the winMutexInit() and winMutexEnd() functions are called as part
** of the sqlite3_initialize() and sqlite3_shutdown() processing, the
** "interlocked" magic used here is probably not strictly necessary.
*/
ಸ್ಥಿರ LONG ಅಸ್ಥಿರ win_Mutex_ಬೀಗ = 0;

ಶೂನ್ಯ sqlite3_win32_sleep(DWORD milliseconds); /* os_win.c */

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ winMutexInit(ಶೂನ್ಯ){
  /* The first to increment to 1 does actual initialization */
  ಆದರೆ( InterlockedCompareExchange(&win_Mutex_ಬೀಗ, 1, 0)==0 ){
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ(i=0; i<Array_ಗಾತ್ರ(aWindowsMutex); i++){
      sqlite3_mutex *p = &aWindowsMutex[i].m;
      p->id = i+2;
      InitializeSRWLock(&p->u.srwl);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
      p->nRef = 0;
      p->owner = 0;
      p->trace = 0;
#ಆದರೆ_ಅಂತ್ಯ
    }
    winMutex_isInit = 1;
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* Another thread is (in the process of) initializing the static
    ** mutexes */
    ಆಗ( !winMutex_isInit ){
      sqlite3_win32_sleep(1);
    }
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ win_Mutex_ಕೊನೆ(ಶೂನ್ಯ){
  /* The first to decrement to 0 does actual shutdown
  ** (which should be the last to shutdown.) */
  ಆದರೆ( InterlockedCompareExchange(&win_Mutex_ಬೀಗ, 0, 1)==1 ){
    ಆದರೆ( winMutex_isInit==1 ){
      winMutex_isInit = 0;
    }
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
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
ಸ್ಥಿರ sqlite3_mutex *winMutexAlloc(ಪೂರ್ಣಾಂಕ iType){
  sqlite3_mutex *p;

  ಆಯ್ಕೆ( iType ){
    ಸ್ಥಿತಿ SQLITE_MUTEX_FAST:
    ಸ್ಥಿತಿ SQLITE_MUTEX_RECURSIVE: {
      p = sqlite3MallocZero( ಗಾತ್ರ(*p) );
      ಆದರೆ( p ){
        p->id = iType;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_WIN32_MUTEX_TRACE_DYNAMIC
        p->trace = 1;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
        ಆದರೆ( iType==SQLITE_MUTEX_RECURSIVE ){
          InitializeCriticalSection(&p->u.cs);
        }ಇಲ್ಲದಿದ್ದರೆ{
          InitializeSRWLock(&p->u.srwl);
        }
      }
      ನಿಲ್ಲಿಸು;
    }
    ಪೂರ್ವನಿಯೋಜಿತ: {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
      ಆದರೆ( iType-2<0 || iType-2>=Array_ಗಾತ್ರ(aWindowsMutex) ){
        (ಶೂನ್ಯ)SQLITE_MISUSE_BKPT;
        ಹಿಂತಿರುಗಿ 0;
      }
#ಆದರೆ_ಅಂತ್ಯ
      p = &aWindowsMutex[iType-2].m;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_WIN32_MUTEX_TRACE_STATIC
      InterlockedCompareExchange(&p->trace, 1, 0);
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
      ನಿಲ್ಲಿಸು;
    }
  }
  ಪ್ರತಿಜ್ಞಾ( p==0 || p->id==iType );
  ಹಿಂತಿರುಗಿ p;
}


/*
** This routine deallocates a previously
** allocated mutex.  SQLite is careful to deallocate every
** mutex that it allocates.
*/
ಸ್ಥಿರ ಶೂನ್ಯ winMutexFree(sqlite3_mutex *p){
  ಪ್ರತಿಜ್ಞಾ( p );
  ಪ್ರತಿಜ್ಞಾ( p->nRef==0 && p->owner==0 );
  ಆದರೆ( p->id==SQLITE_MUTEX_FAST ){
    sqlite3_free(p);
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p->id==SQLITE_MUTEX_RECURSIVE ){
    DeleteCriticalSection(&p->u.cs);
    sqlite3_free(p);
  }ಇಲ್ಲದಿದ್ದರೆ{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
    (ಶೂನ್ಯ)SQLITE_MISUSE_BKPT;
#ಆದರೆ_ಅಂತ್ಯ
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
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  ಪ್ರತಿಜ್ಞಾ( p->id==SQLITE_MUTEX_RECURSIVE || winMutexNotheld(p) );
#ಆದರೆ_ಅಂತ್ಯ
  ಪ್ರತಿಜ್ಞಾ( winMutex_isInit==1 );
  ಆದರೆ( p->id==SQLITE_MUTEX_RECURSIVE ){
    EnterCriticalSection(&p->u.cs);
  }ಇಲ್ಲದಿದ್ದರೆ{
    AcquireSRWLockExclusive(&p->u.srwl);
  }
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  ಪ್ರತಿಜ್ಞಾ( p->nRef>0 || p->owner==0 );
  p->owner = GetCurrentThreadId();
  p->nRef++;
  ಆದರೆ( p->trace ){
    OSTRACE(("ENTER-MUTEX tid=%lu, mutex(%d)=%p (%d), nRef=%d\n",
             GetCurrentThreadId(), p->id, p, p->trace, p->nRef));
  }
#ಆದರೆ_ಅಂತ್ಯ
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ winMutexTry(sqlite3_mutex *p){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_BUSY;
  ಪ್ರತಿಜ್ಞಾ( p );
  ಪ್ರತಿಜ್ಞಾ( p->id==SQLITE_MUTEX_RECURSIVE || winMutexNotheld(p) );
  /*
  ** The sqlite3_mutex_try() routine is seldom used, and when it is
  ** used it is merely an optimization.  So it is OK for it to always
  ** fail.
  */
  ಆದರೆ( p->id==SQLITE_MUTEX_RECURSIVE ){
    rc = TryEnterCriticalSection(&p->u.cs);
  }ಇಲ್ಲದಿದ್ದರೆ{
    rc = TryAcquireSRWLockExclusive(&p->u.srwl);
  }
  ಆದರೆ( rc ){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
    p->owner = GetCurrentThreadId();
    p->nRef++;
#ಆದರೆ_ಅಂತ್ಯ
    rc = SQLITE_OK;
  }ಇಲ್ಲದಿದ್ದರೆ{
    rc = SQLITE_BUSY;
  }
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  ಆದರೆ( p->trace ){
    OSTRACE((
       "TRY-MUTEX tid=%lu, mutex(%d)=%p (%d), owner=%lu, nRef=%d, rc=%s\n",
       GetCurrentThreadId(), p->id, p, p->trace, p->owner, p->nRef,
       sqlite3ErrName(rc)));
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ rc;
}

/*
** The sqlite3_mutex_leave() routine exits a mutex that was
** previously entered by the same thread.  The behavior
** is undefined if the mutex is not currently entered or
** is not currently allocated.  SQLite will never do either.
*/
ಸ್ಥಿರ ಶೂನ್ಯ winMutexLeave(sqlite3_mutex *p){
  ಪ್ರತಿಜ್ಞಾ( p );
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  ಪ್ರತಿಜ್ಞಾ( p->nRef>0 );
  ಪ್ರತಿಜ್ಞಾ( p->owner==GetCurrentThreadId() );
  p->nRef--;
  ಆದರೆ( p->nRef==0 ) p->owner = 0;
  ಪ್ರತಿಜ್ಞಾ( p->nRef==0 || p->id==SQLITE_MUTEX_RECURSIVE );
#ಆದರೆ_ಅಂತ್ಯ
  ಪ್ರತಿಜ್ಞಾ( winMutex_isInit==1 );
  ಆದರೆ( p->id==SQLITE_MUTEX_RECURSIVE ){
    LeaveCriticalSection(&p->u.cs);
  }ಇಲ್ಲದಿದ್ದರೆ{
    ReleaseSRWLockExclusive(&p->u.srwl);
  }
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  ಆದರೆ( p->trace ){
    OSTRACE(("LEAVE-MUTEX tid=%lu, mutex(%d)=%p (%d), nRef=%d\n",
             GetCurrentThreadId(), p->id, p, p->trace, p->nRef));
  }
#ಆದರೆ_ಅಂತ್ಯ
}

sqlite3_mutex_methods ಸ್ಥಿರಾಂಕ *sqlite3DefaultMutex(ಶೂನ್ಯ){
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ sqlite3_mutex_methods sMutex = {
    winMutexInit,
    win_Mutex_ಕೊನೆ,
    winMutexAlloc,
    winMutexFree,
    winMutexEnter,
    winMutexTry,
    winMutexLeave,
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
    winMutexHeld,
    winMutexNotheld
#ಇಲ್ಲದಿದ್ದರೆ
    0,
    0
#ಆದರೆ_ಅಂತ್ಯ
  };
  ಹಿಂತಿರುಗಿ &sMutex;
}

#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_MUTEX_W32 */
