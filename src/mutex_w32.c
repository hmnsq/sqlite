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
#شامل "sqliteInt.h"

#اگر SQLITE_OS_WIN
/*
** Include code that is common to all os_*.c files
*/
#شامل "os_common.h"

/*
** Include the header file for the Windows VFS.
*/
#شامل "os_win.h"
#اگر_ختم

/*
** The code in this file is only used if we are compiling multithreaded
** on a Win32 system.
*/
#اگر_تعریف_شدہ SQLITE_MUTEX_W32

#اگر MSVC_نسخہ>0
# define ALIGN128 __declspec(align(128))
#ورنہ
# define ALIGN128
#اگر_ختم

#اگر_تعریف_شدہ _MSC_VER
#ہدایت warning(ڈالو)
#ہدایت warning(disable: 4324)
#اگر_ختم /* _MSC_VER */

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
ساخت sqlite3_mutex { 
  اتحاد {
    CRITICAL_SECTION cs;     /* The CRITICAL_SECTION mutex.  id==1 */
    SRWLOCK srwl;            /* The Slim reader/writer lock.  id!=1 */
  } u;
  صحیح id;                    /* Mutex type */
#اگر_تعریف_شدہ SQLITE_DEBUG
  متلون صحیح nRef;         /* Number of entrances */
  متلون DWORD owner;      /* Thread holding this mutex */
  متلون LONG trace;       /* True to trace changes */
#اگر_ختم
};
#اگر_تعریف_شدہ _MSC_VER
#ہدایت warning(نکالو)
#اگر_ختم /* _MSC_VER */

#اگر_تعریف_شدہ SQLITE_DEBUG
/*
** The sqlite3_mutex_held() and sqlite3_mutex_notheld() routine are
** intended for use only inside assert() statements.
*/
ساکن صحیح winMutexHeld(sqlite3_mutex *p){
  واپس p->nRef!=0 && p->owner==GetCurrentThreadId();
}

ساکن صحیح winMutexNotheld(sqlite3_mutex *p){
  واپس p->nRef==0 || p->owner!=GetCurrentThreadId();
}
#اگر_ختم

/*
** Try to provide a memory barrier operation, needed for initialization
** and also for the xShmBarrier method of the VFS in cases when SQLite is
** compiled without mutexes (SQLITE_THREADSAFE=0).
*/
خالی sqlite3_حافظہ_Barrier(خالی){
#اگر defined(SQLITE_حافظہ_BARRIER)
  SQLITE_حافظہ_BARRIER;
#وگرنہ_اگر defined(__GNUC__)
  __sync_synchronize();
#وگرنہ_اگر MSVC_نسخہ>=1400
  _ReadWriteBarrier();
#وگرنہ_اگر defined(حافظہ_Barrier)
  حافظہ_Barrier();
#اگر_ختم
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
ساکن ALIGN128 اتحاد staticMutexes {
  sqlite3_mutex m;
  حرف spacer[128];
} aWindowsMutex[12];
ساکن صحیح winMutex_isInit = 0;

/* As the winMutexInit() and winMutexEnd() functions are called as part
** of the sqlite3_initialize() and sqlite3_shutdown() processing, the
** "interlocked" magic used here is probably not strictly necessary.
*/
ساکن LONG متلون win_Mutex_تالا = 0;

خالی sqlite3_win32_sleep(DWORD milliseconds); /* os_win.c */

ساکن صحیح winMutexInit(خالی){
  /* The first to increment to 1 does actual initialization */
  اگر( InterlockedCompareExchange(&win_Mutex_تالا, 1, 0)==0 ){
    صحیح i;
    برائے(i=0; i<Array_سائز(aWindowsMutex); i++){
      sqlite3_mutex *p = &aWindowsMutex[i].m;
      p->id = i+2;
      InitializeSRWLock(&p->u.srwl);
#اگر_تعریف_شدہ SQLITE_DEBUG
      p->nRef = 0;
      p->owner = 0;
      p->trace = 0;
#اگر_ختم
    }
    winMutex_isInit = 1;
  }ورنہ{
    /* Another thread is (in the process of) initializing the static
    ** mutexes */
    جب( !winMutex_isInit ){
      sqlite3_win32_sleep(1);
    }
  }
  واپس SQLITE_OK;
}

ساکن صحیح win_Mutex_انتہا(خالی){
  /* The first to decrement to 0 does actual shutdown
  ** (which should be the last to shutdown.) */
  اگر( InterlockedCompareExchange(&win_Mutex_تالا, 0, 1)==1 ){
    اگر( winMutex_isInit==1 ){
      winMutex_isInit = 0;
    }
  }
  واپس SQLITE_OK;
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
ساکن sqlite3_mutex *winMutexAlloc(صحیح iType){
  sqlite3_mutex *p;

  انتخاب( iType ){
    حالت SQLITE_MUTEX_FAST:
    حالت SQLITE_MUTEX_RECURSIVE: {
      p = sqlite3MallocZero( حجم(*p) );
      اگر( p ){
        p->id = iType;
#اگر_تعریف_شدہ SQLITE_DEBUG
#اگر_تعریف_شدہ SQLITE_WIN32_MUTEX_TRACE_DYNAMIC
        p->trace = 1;
#اگر_ختم
#اگر_ختم
        اگر( iType==SQLITE_MUTEX_RECURSIVE ){
          InitializeCriticalSection(&p->u.cs);
        }ورنہ{
          InitializeSRWLock(&p->u.srwl);
        }
      }
      توڑو;
    }
    طےشدہ: {
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
      اگر( iType-2<0 || iType-2>=Array_سائز(aWindowsMutex) ){
        (خالی)SQLITE_MISUSE_BKPT;
        واپس 0;
      }
#اگر_ختم
      p = &aWindowsMutex[iType-2].m;
#اگر_تعریف_شدہ SQLITE_DEBUG
#اگر_تعریف_شدہ SQLITE_WIN32_MUTEX_TRACE_STATIC
      InterlockedCompareExchange(&p->trace, 1, 0);
#اگر_ختم
#اگر_ختم
      توڑو;
    }
  }
  أكد( p==0 || p->id==iType );
  واپس p;
}


/*
** This routine deallocates a previously
** allocated mutex.  SQLite is careful to deallocate every
** mutex that it allocates.
*/
ساکن خالی winMutexFree(sqlite3_mutex *p){
  أكد( p );
  أكد( p->nRef==0 && p->owner==0 );
  اگر( p->id==SQLITE_MUTEX_FAST ){
    sqlite3_free(p);
  }ورنہ اگر( p->id==SQLITE_MUTEX_RECURSIVE ){
    DeleteCriticalSection(&p->u.cs);
    sqlite3_free(p);
  }ورنہ{
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
    (خالی)SQLITE_MISUSE_BKPT;
#اگر_ختم
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
ساکن خالی winMutexEnter(sqlite3_mutex *p){
  أكد( p );
#اگر_تعریف_شدہ SQLITE_DEBUG
  أكد( p->id==SQLITE_MUTEX_RECURSIVE || winMutexNotheld(p) );
#اگر_ختم
  أكد( winMutex_isInit==1 );
  اگر( p->id==SQLITE_MUTEX_RECURSIVE ){
    EnterCriticalSection(&p->u.cs);
  }ورنہ{
    AcquireSRWLockExclusive(&p->u.srwl);
  }
#اگر_تعریف_شدہ SQLITE_DEBUG
  أكد( p->nRef>0 || p->owner==0 );
  p->owner = GetCurrentThreadId();
  p->nRef++;
  اگر( p->trace ){
    OSTRACE(("ENTER-MUTEX tid=%lu, mutex(%d)=%p (%d), nRef=%d\n",
             GetCurrentThreadId(), p->id, p, p->trace, p->nRef));
  }
#اگر_ختم
}

ساکن صحیح winMutexTry(sqlite3_mutex *p){
  صحیح rc = SQLITE_BUSY;
  أكد( p );
  أكد( p->id==SQLITE_MUTEX_RECURSIVE || winMutexNotheld(p) );
  /*
  ** The sqlite3_mutex_try() routine is seldom used, and when it is
  ** used it is merely an optimization.  So it is OK for it to always
  ** fail.
  */
  اگر( p->id==SQLITE_MUTEX_RECURSIVE ){
    rc = TryEnterCriticalSection(&p->u.cs);
  }ورنہ{
    rc = TryAcquireSRWLockExclusive(&p->u.srwl);
  }
  اگر( rc ){
#اگر_تعریف_شدہ SQLITE_DEBUG
    p->owner = GetCurrentThreadId();
    p->nRef++;
#اگر_ختم
    rc = SQLITE_OK;
  }ورنہ{
    rc = SQLITE_BUSY;
  }
#اگر_تعریف_شدہ SQLITE_DEBUG
  اگر( p->trace ){
    OSTRACE((
       "TRY-MUTEX tid=%lu, mutex(%d)=%p (%d), owner=%lu, nRef=%d, rc=%s\n",
       GetCurrentThreadId(), p->id, p, p->trace, p->owner, p->nRef,
       sqlite3ErrName(rc)));
  }
#اگر_ختم
  واپس rc;
}

/*
** The sqlite3_mutex_leave() routine exits a mutex that was
** previously entered by the same thread.  The behavior
** is undefined if the mutex is not currently entered or
** is not currently allocated.  SQLite will never do either.
*/
ساکن خالی winMutexLeave(sqlite3_mutex *p){
  أكد( p );
#اگر_تعریف_شدہ SQLITE_DEBUG
  أكد( p->nRef>0 );
  أكد( p->owner==GetCurrentThreadId() );
  p->nRef--;
  اگر( p->nRef==0 ) p->owner = 0;
  أكد( p->nRef==0 || p->id==SQLITE_MUTEX_RECURSIVE );
#اگر_ختم
  أكد( winMutex_isInit==1 );
  اگر( p->id==SQLITE_MUTEX_RECURSIVE ){
    LeaveCriticalSection(&p->u.cs);
  }ورنہ{
    ReleaseSRWLockExclusive(&p->u.srwl);
  }
#اگر_تعریف_شدہ SQLITE_DEBUG
  اگر( p->trace ){
    OSTRACE(("LEAVE-MUTEX tid=%lu, mutex(%d)=%p (%d), nRef=%d\n",
             GetCurrentThreadId(), p->id, p, p->trace, p->nRef));
  }
#اگر_ختم
}

sqlite3_mutex_methods ثابت *sqlite3DefaultMutex(خالی){
  ساکن ثابت sqlite3_mutex_methods sMutex = {
    winMutexInit,
    win_Mutex_انتہا,
    winMutexAlloc,
    winMutexFree,
    winMutexEnter,
    winMutexTry,
    winMutexLeave,
#اگر_تعریف_شدہ SQLITE_DEBUG
    winMutexHeld,
    winMutexNotheld
#ورنہ
    0,
    0
#اگر_ختم
  };
  واپس &sMutex;
}

#اگر_ختم /* SQLITE_MUTEX_W32 */
