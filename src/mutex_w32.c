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
#ضمن "sqliteInt.h"

#إذا SQLITE_OS_WIN
/*
** Include code that is common to all os_*.c files
*/
#ضمن "os_common.h"

/*
** Include the header file for the Windows VFS.
*/
#ضمن "os_win.h"
#د_که_پای

/*
** The code in this file is only used if we are compiling multithreaded
** on a Win32 system.
*/
#که_تعریف_شوی SQLITE_MUTEX_W32

#إذا MSVC_نسخه>0
# define ALIGN128 __declspec(align(128))
#وإلا
# define ALIGN128
#د_که_پای

#که_تعریف_شوی _MSC_VER
#لارښوونه warning(ټېل)
#لارښوونه warning(disable: 4324)
#د_که_پای /* _MSC_VER */

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
بنية sqlite3_mutex { 
  اتحاد {
    CRITICAL_SECTION cs;     /* The CRITICAL_SECTION mutex.  id==1 */
    SRWLOCK srwl;            /* The Slim reader/writer lock.  id!=1 */
  } u;
  صحيح id;                    /* Mutex type */
#که_تعریف_شوی SQLITE_DEBUG
  متطاير صحيح nRef;         /* Number of entrances */
  متطاير DWORD owner;      /* Thread holding this mutex */
  متطاير LONG trace;       /* True to trace changes */
#د_که_پای
};
#که_تعریف_شوی _MSC_VER
#لارښوونه warning(راوباسه)
#د_که_پای /* _MSC_VER */

#که_تعریف_شوی SQLITE_DEBUG
/*
** The sqlite3_mutex_held() and sqlite3_mutex_notheld() routine are
** intended for use only inside assert() statements.
*/
ساكن صحيح winMutexHeld(sqlite3_mutex *p){
  أرجع p->nRef!=0 && p->owner==GetCurrentThreadId();
}

ساكن صحيح winMutexNotheld(sqlite3_mutex *p){
  أرجع p->nRef==0 || p->owner!=GetCurrentThreadId();
}
#د_که_پای

/*
** Try to provide a memory barrier operation, needed for initialization
** and also for the xShmBarrier method of the VFS in cases when SQLite is
** compiled without mutexes (SQLITE_THREADSAFE=0).
*/
فراغ sqlite3_حافظه_Barrier(فراغ){
#إذا defined(SQLITE_حافظه_BARRIER)
  SQLITE_حافظه_BARRIER;
#وإلاإذا defined(__GNUC__)
  __sync_synchronize();
#وإلاإذا MSVC_نسخه>=1400
  _ReadWriteBarrier();
#وإلاإذا defined(حافظه_Barrier)
  حافظه_Barrier();
#د_که_پای
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
ساكن ALIGN128 اتحاد staticMutexes {
  sqlite3_mutex m;
  حرف spacer[128];
} aWindowsMutex[12];
ساكن صحيح winMutex_isInit = 0;

/* As the winMutexInit() and winMutexEnd() functions are called as part
** of the sqlite3_initialize() and sqlite3_shutdown() processing, the
** "interlocked" magic used here is probably not strictly necessary.
*/
ساكن LONG متطاير win_Mutex_قلف = 0;

فراغ sqlite3_win32_sleep(DWORD milliseconds); /* os_win.c */

ساكن صحيح winMutexInit(فراغ){
  /* The first to increment to 1 does actual initialization */
  إذا( InterlockedCompareExchange(&win_Mutex_قلف, 1, 0)==0 ){
    صحيح i;
    لكل(i=0; i<Array_اندازه(aWindowsMutex); i++){
      sqlite3_mutex *p = &aWindowsMutex[i].m;
      p->id = i+2;
      InitializeSRWLock(&p->u.srwl);
#که_تعریف_شوی SQLITE_DEBUG
      p->nRef = 0;
      p->owner = 0;
      p->trace = 0;
#د_که_پای
    }
    winMutex_isInit = 1;
  }وإلا{
    /* Another thread is (in the process of) initializing the static
    ** mutexes */
    طالما( !winMutex_isInit ){
      sqlite3_win32_sleep(1);
    }
  }
  أرجع SQLITE_OK;
}

ساكن صحيح win_Mutex_پای_ټکی(فراغ){
  /* The first to decrement to 0 does actual shutdown
  ** (which should be the last to shutdown.) */
  إذا( InterlockedCompareExchange(&win_Mutex_قلف, 0, 1)==1 ){
    إذا( winMutex_isInit==1 ){
      winMutex_isInit = 0;
    }
  }
  أرجع SQLITE_OK;
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
ساكن sqlite3_mutex *winMutexAlloc(صحيح iType){
  sqlite3_mutex *p;

  بدل( iType ){
    حالة SQLITE_MUTEX_FAST:
    حالة SQLITE_MUTEX_RECURSIVE: {
      p = sqlite3MallocZero( حجم(*p) );
      إذا( p ){
        p->id = iType;
#که_تعریف_شوی SQLITE_DEBUG
#که_تعریف_شوی SQLITE_WIN32_MUTEX_TRACE_DYNAMIC
        p->trace = 1;
#د_که_پای
#د_که_پای
        إذا( iType==SQLITE_MUTEX_RECURSIVE ){
          InitializeCriticalSection(&p->u.cs);
        }وإلا{
          InitializeSRWLock(&p->u.srwl);
        }
      }
      اكسر;
    }
    افتراضي: {
#که_تعریف_شوی SQLITE_ENABLE_API_ARMOR
      إذا( iType-2<0 || iType-2>=Array_اندازه(aWindowsMutex) ){
        (فراغ)SQLITE_MISUSE_BKPT;
        أرجع 0;
      }
#د_که_پای
      p = &aWindowsMutex[iType-2].m;
#که_تعریف_شوی SQLITE_DEBUG
#که_تعریف_شوی SQLITE_WIN32_MUTEX_TRACE_STATIC
      InterlockedCompareExchange(&p->trace, 1, 0);
#د_که_پای
#د_که_پای
      اكسر;
    }
  }
  أكد( p==0 || p->id==iType );
  أرجع p;
}


/*
** This routine deallocates a previously
** allocated mutex.  SQLite is careful to deallocate every
** mutex that it allocates.
*/
ساكن فراغ winMutexFree(sqlite3_mutex *p){
  أكد( p );
  أكد( p->nRef==0 && p->owner==0 );
  إذا( p->id==SQLITE_MUTEX_FAST ){
    sqlite3_free(p);
  }وإلا إذا( p->id==SQLITE_MUTEX_RECURSIVE ){
    DeleteCriticalSection(&p->u.cs);
    sqlite3_free(p);
  }وإلا{
#که_تعریف_شوی SQLITE_ENABLE_API_ARMOR
    (فراغ)SQLITE_MISUSE_BKPT;
#د_که_پای
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
ساكن فراغ winMutexEnter(sqlite3_mutex *p){
  أكد( p );
#که_تعریف_شوی SQLITE_DEBUG
  أكد( p->id==SQLITE_MUTEX_RECURSIVE || winMutexNotheld(p) );
#د_که_پای
  أكد( winMutex_isInit==1 );
  إذا( p->id==SQLITE_MUTEX_RECURSIVE ){
    EnterCriticalSection(&p->u.cs);
  }وإلا{
    AcquireSRWLockExclusive(&p->u.srwl);
  }
#که_تعریف_شوی SQLITE_DEBUG
  أكد( p->nRef>0 || p->owner==0 );
  p->owner = GetCurrentThreadId();
  p->nRef++;
  إذا( p->trace ){
    OSTRACE(("ENTER-MUTEX tid=%lu, mutex(%d)=%p (%d), nRef=%d\n",
             GetCurrentThreadId(), p->id, p, p->trace, p->nRef));
  }
#د_که_پای
}

ساكن صحيح winMutexTry(sqlite3_mutex *p){
  صحيح rc = SQLITE_BUSY;
  أكد( p );
  أكد( p->id==SQLITE_MUTEX_RECURSIVE || winMutexNotheld(p) );
  /*
  ** The sqlite3_mutex_try() routine is seldom used, and when it is
  ** used it is merely an optimization.  So it is OK for it to always
  ** fail.
  */
  إذا( p->id==SQLITE_MUTEX_RECURSIVE ){
    rc = TryEnterCriticalSection(&p->u.cs);
  }وإلا{
    rc = TryAcquireSRWLockExclusive(&p->u.srwl);
  }
  إذا( rc ){
#که_تعریف_شوی SQLITE_DEBUG
    p->owner = GetCurrentThreadId();
    p->nRef++;
#د_که_پای
    rc = SQLITE_OK;
  }وإلا{
    rc = SQLITE_BUSY;
  }
#که_تعریف_شوی SQLITE_DEBUG
  إذا( p->trace ){
    OSTRACE((
       "TRY-MUTEX tid=%lu, mutex(%d)=%p (%d), owner=%lu, nRef=%d, rc=%s\n",
       GetCurrentThreadId(), p->id, p, p->trace, p->owner, p->nRef,
       sqlite3ErrName(rc)));
  }
#د_که_پای
  أرجع rc;
}

/*
** The sqlite3_mutex_leave() routine exits a mutex that was
** previously entered by the same thread.  The behavior
** is undefined if the mutex is not currently entered or
** is not currently allocated.  SQLite will never do either.
*/
ساكن فراغ winMutexLeave(sqlite3_mutex *p){
  أكد( p );
#که_تعریف_شوی SQLITE_DEBUG
  أكد( p->nRef>0 );
  أكد( p->owner==GetCurrentThreadId() );
  p->nRef--;
  إذا( p->nRef==0 ) p->owner = 0;
  أكد( p->nRef==0 || p->id==SQLITE_MUTEX_RECURSIVE );
#د_که_پای
  أكد( winMutex_isInit==1 );
  إذا( p->id==SQLITE_MUTEX_RECURSIVE ){
    LeaveCriticalSection(&p->u.cs);
  }وإلا{
    ReleaseSRWLockExclusive(&p->u.srwl);
  }
#که_تعریف_شوی SQLITE_DEBUG
  إذا( p->trace ){
    OSTRACE(("LEAVE-MUTEX tid=%lu, mutex(%d)=%p (%d), nRef=%d\n",
             GetCurrentThreadId(), p->id, p, p->trace, p->nRef));
  }
#د_که_پای
}

sqlite3_mutex_methods ثابت *sqlite3DefaultMutex(فراغ){
  ساكن ثابت sqlite3_mutex_methods sMutex = {
    winMutexInit,
    win_Mutex_پای_ټکی,
    winMutexAlloc,
    winMutexFree,
    winMutexEnter,
    winMutexTry,
    winMutexLeave,
#که_تعریف_شوی SQLITE_DEBUG
    winMutexHeld,
    winMutexNotheld
#وإلا
    0,
    0
#د_که_پای
  };
  أرجع &sMutex;
}

#د_که_پای /* SQLITE_MUTEX_W32 */
