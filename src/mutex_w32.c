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
#అన్తర్భావ "sqliteInt.h"

#ఒకవేళ SQLITE_OS_WIN
/*
** Include code that is common to all os_*.c files
*/
#అన్తర్భావ "os_common.h"

/*
** Include the header file for the Windows VFS.
*/
#అన్తర్భావ "os_win.h"
#అయితే_ముగింపు

/*
** The code in this file is only used if we are compiling multithreaded
** on a Win32 system.
*/
#నిర్వచితమైతే SQLITE_MUTEX_W32

#ఒకవేళ MSVC_సంస్కరణ>0
# define ALIGN128 __declspec(align(128))
#లేకపోతే
# define ALIGN128
#అయితే_ముగింపు

#నిర్వచితమైతే _MSC_VER
#సూచన warning(నెట్టు)
#సూచన warning(disable: 4324)
#అయితే_ముగింపు /* _MSC_VER */

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
నిర్మాణం sqlite3_mutex { 
  సంఘం {
    CRITICAL_SECTION cs;     /* The CRITICAL_SECTION mutex.  id==1 */
    SRWLOCK srwl;            /* The Slim reader/writer lock.  id!=1 */
  } u;
  పూర్ణాంకం id;                    /* Mutex type */
#నిర్వచితమైతే SQLITE_DEBUG
  అస్థిర పూర్ణాంకం nRef;         /* Number of entrances */
  అస్థిర DWORD owner;      /* Thread holding this mutex */
  అస్థిర LONG trace;       /* True to trace changes */
#అయితే_ముగింపు
};
#నిర్వచితమైతే _MSC_VER
#సూచన warning(తీయి)
#అయితే_ముగింపు /* _MSC_VER */

#నిర్వచితమైతే SQLITE_DEBUG
/*
** The sqlite3_mutex_held() and sqlite3_mutex_notheld() routine are
** intended for use only inside assert() statements.
*/
స్థిర పూర్ణాంకం winMutexHeld(sqlite3_mutex *p){
  తిరిగి p->nRef!=0 && p->owner==GetCurrentThreadId();
}

స్థిర పూర్ణాంకం winMutexNotheld(sqlite3_mutex *p){
  తిరిగి p->nRef==0 || p->owner!=GetCurrentThreadId();
}
#అయితే_ముగింపు

/*
** Try to provide a memory barrier operation, needed for initialization
** and also for the xShmBarrier method of the VFS in cases when SQLite is
** compiled without mutexes (SQLITE_THREADSAFE=0).
*/
శూన్య sqlite3_జ్ఞాపకం_Barrier(శూన్య){
#ఒకవేళ defined(SQLITE_జ్ఞాపకం_BARRIER)
  SQLITE_జ్ఞాపకం_BARRIER;
#లేదావేళ defined(__GNUC__)
  __sync_synchronize();
#లేదావేళ MSVC_సంస్కరణ>=1400
  _ReadWriteBarrier();
#లేదావేళ defined(జ్ఞాపకం_Barrier)
  జ్ఞాపకం_Barrier();
#అయితే_ముగింపు
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
స్థిర ALIGN128 సంఘం staticMutexes {
  sqlite3_mutex m;
  అక్షరం spacer[128];
} aWindowsMutex[12];
స్థిర పూర్ణాంకం winMutex_isInit = 0;

/* As the winMutexInit() and winMutexEnd() functions are called as part
** of the sqlite3_initialize() and sqlite3_shutdown() processing, the
** "interlocked" magic used here is probably not strictly necessary.
*/
స్థిర LONG అస్థిర win_Mutex_తాళం = 0;

శూన్య sqlite3_win32_sleep(DWORD milliseconds); /* os_win.c */

స్థిర పూర్ణాంకం winMutexInit(శూన్య){
  /* The first to increment to 1 does actual initialization */
  ఒకవేళ( InterlockedCompareExchange(&win_Mutex_తాళం, 1, 0)==0 ){
    పూర్ణాంకం i;
    కొరకు(i=0; i<Array_కొలత(aWindowsMutex); i++){
      sqlite3_mutex *p = &aWindowsMutex[i].m;
      p->id = i+2;
      InitializeSRWLock(&p->u.srwl);
#నిర్వచితమైతే SQLITE_DEBUG
      p->nRef = 0;
      p->owner = 0;
      p->trace = 0;
#అయితే_ముగింపు
    }
    winMutex_isInit = 1;
  }లేకపోతే{
    /* Another thread is (in the process of) initializing the static
    ** mutexes */
    అయితే( !winMutex_isInit ){
      sqlite3_win32_sleep(1);
    }
  }
  తిరిగి SQLITE_OK;
}

స్థిర పూర్ణాంకం win_Mutex_చివర(శూన్య){
  /* The first to decrement to 0 does actual shutdown
  ** (which should be the last to shutdown.) */
  ఒకవేళ( InterlockedCompareExchange(&win_Mutex_తాళం, 0, 1)==1 ){
    ఒకవేళ( winMutex_isInit==1 ){
      winMutex_isInit = 0;
    }
  }
  తిరిగి SQLITE_OK;
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
స్థిర sqlite3_mutex *winMutexAlloc(పూర్ణాంకం iType){
  sqlite3_mutex *p;

  ఎంపిక( iType ){
    స్థితి SQLITE_MUTEX_FAST:
    స్థితి SQLITE_MUTEX_RECURSIVE: {
      p = sqlite3MallocZero( పరిమాణం(*p) );
      ఒకవేళ( p ){
        p->id = iType;
#నిర్వచితమైతే SQLITE_DEBUG
#నిర్వచితమైతే SQLITE_WIN32_MUTEX_TRACE_DYNAMIC
        p->trace = 1;
#అయితే_ముగింపు
#అయితే_ముగింపు
        ఒకవేళ( iType==SQLITE_MUTEX_RECURSIVE ){
          InitializeCriticalSection(&p->u.cs);
        }లేకపోతే{
          InitializeSRWLock(&p->u.srwl);
        }
      }
      ఆపు;
    }
    అప్రమేయ: {
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
      ఒకవేళ( iType-2<0 || iType-2>=Array_కొలత(aWindowsMutex) ){
        (శూన్య)SQLITE_MISUSE_BKPT;
        తిరిగి 0;
      }
#అయితే_ముగింపు
      p = &aWindowsMutex[iType-2].m;
#నిర్వచితమైతే SQLITE_DEBUG
#నిర్వచితమైతే SQLITE_WIN32_MUTEX_TRACE_STATIC
      InterlockedCompareExchange(&p->trace, 1, 0);
#అయితే_ముగింపు
#అయితే_ముగింపు
      ఆపు;
    }
  }
  ప్రతిజ్ఞా( p==0 || p->id==iType );
  తిరిగి p;
}


/*
** This routine deallocates a previously
** allocated mutex.  SQLite is careful to deallocate every
** mutex that it allocates.
*/
స్థిర శూన్య winMutexFree(sqlite3_mutex *p){
  ప్రతిజ్ఞా( p );
  ప్రతిజ్ఞా( p->nRef==0 && p->owner==0 );
  ఒకవేళ( p->id==SQLITE_MUTEX_FAST ){
    sqlite3_free(p);
  }లేకపోతే ఒకవేళ( p->id==SQLITE_MUTEX_RECURSIVE ){
    DeleteCriticalSection(&p->u.cs);
    sqlite3_free(p);
  }లేకపోతే{
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
    (శూన్య)SQLITE_MISUSE_BKPT;
#అయితే_ముగింపు
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
స్థిర శూన్య winMutexEnter(sqlite3_mutex *p){
  ప్రతిజ్ఞా( p );
#నిర్వచితమైతే SQLITE_DEBUG
  ప్రతిజ్ఞా( p->id==SQLITE_MUTEX_RECURSIVE || winMutexNotheld(p) );
#అయితే_ముగింపు
  ప్రతిజ్ఞా( winMutex_isInit==1 );
  ఒకవేళ( p->id==SQLITE_MUTEX_RECURSIVE ){
    EnterCriticalSection(&p->u.cs);
  }లేకపోతే{
    AcquireSRWLockExclusive(&p->u.srwl);
  }
#నిర్వచితమైతే SQLITE_DEBUG
  ప్రతిజ్ఞా( p->nRef>0 || p->owner==0 );
  p->owner = GetCurrentThreadId();
  p->nRef++;
  ఒకవేళ( p->trace ){
    OSTRACE(("ENTER-MUTEX tid=%lu, mutex(%d)=%p (%d), nRef=%d\n",
             GetCurrentThreadId(), p->id, p, p->trace, p->nRef));
  }
#అయితే_ముగింపు
}

స్థిర పూర్ణాంకం winMutexTry(sqlite3_mutex *p){
  పూర్ణాంకం rc = SQLITE_BUSY;
  ప్రతిజ్ఞా( p );
  ప్రతిజ్ఞా( p->id==SQLITE_MUTEX_RECURSIVE || winMutexNotheld(p) );
  /*
  ** The sqlite3_mutex_try() routine is seldom used, and when it is
  ** used it is merely an optimization.  So it is OK for it to always
  ** fail.
  */
  ఒకవేళ( p->id==SQLITE_MUTEX_RECURSIVE ){
    rc = TryEnterCriticalSection(&p->u.cs);
  }లేకపోతే{
    rc = TryAcquireSRWLockExclusive(&p->u.srwl);
  }
  ఒకవేళ( rc ){
#నిర్వచితమైతే SQLITE_DEBUG
    p->owner = GetCurrentThreadId();
    p->nRef++;
#అయితే_ముగింపు
    rc = SQLITE_OK;
  }లేకపోతే{
    rc = SQLITE_BUSY;
  }
#నిర్వచితమైతే SQLITE_DEBUG
  ఒకవేళ( p->trace ){
    OSTRACE((
       "TRY-MUTEX tid=%lu, mutex(%d)=%p (%d), owner=%lu, nRef=%d, rc=%s\n",
       GetCurrentThreadId(), p->id, p, p->trace, p->owner, p->nRef,
       sqlite3ErrName(rc)));
  }
#అయితే_ముగింపు
  తిరిగి rc;
}

/*
** The sqlite3_mutex_leave() routine exits a mutex that was
** previously entered by the same thread.  The behavior
** is undefined if the mutex is not currently entered or
** is not currently allocated.  SQLite will never do either.
*/
స్థిర శూన్య winMutexLeave(sqlite3_mutex *p){
  ప్రతిజ్ఞా( p );
#నిర్వచితమైతే SQLITE_DEBUG
  ప్రతిజ్ఞా( p->nRef>0 );
  ప్రతిజ్ఞా( p->owner==GetCurrentThreadId() );
  p->nRef--;
  ఒకవేళ( p->nRef==0 ) p->owner = 0;
  ప్రతిజ్ఞా( p->nRef==0 || p->id==SQLITE_MUTEX_RECURSIVE );
#అయితే_ముగింపు
  ప్రతిజ్ఞా( winMutex_isInit==1 );
  ఒకవేళ( p->id==SQLITE_MUTEX_RECURSIVE ){
    LeaveCriticalSection(&p->u.cs);
  }లేకపోతే{
    ReleaseSRWLockExclusive(&p->u.srwl);
  }
#నిర్వచితమైతే SQLITE_DEBUG
  ఒకవేళ( p->trace ){
    OSTRACE(("LEAVE-MUTEX tid=%lu, mutex(%d)=%p (%d), nRef=%d\n",
             GetCurrentThreadId(), p->id, p, p->trace, p->nRef));
  }
#అయితే_ముగింపు
}

sqlite3_mutex_methods స్థిరరాశి *sqlite3DefaultMutex(శూన్య){
  స్థిర స్థిరరాశి sqlite3_mutex_methods sMutex = {
    winMutexInit,
    win_Mutex_చివర,
    winMutexAlloc,
    winMutexFree,
    winMutexEnter,
    winMutexTry,
    winMutexLeave,
#నిర్వచితమైతే SQLITE_DEBUG
    winMutexHeld,
    winMutexNotheld
#లేకపోతే
    0,
    0
#అయితే_ముగింపు
  };
  తిరిగి &sMutex;
}

#అయితే_ముగింపు /* SQLITE_MUTEX_W32 */
