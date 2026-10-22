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
#अन्तर्भाव "sqliteInt.h"

#यदि SQLITE_OS_WIN
/*
** Include code that is common to all os_*.c files
*/
#अन्तर्भाव "os_common.h"

/*
** Include the header file for the Windows VFS.
*/
#अन्तर्भाव "os_win.h"
#यदि_अन्तः

/*
** The code in this file is only used if we are compiling multithreaded
** on a Win32 system.
*/
#यदि_परिभाषितम् SQLITE_MUTEX_W32

#यदि MSVC_संस्करणम्>0
# define ALIGN128 __declspec(align(128))
#अन्यथा
# define ALIGN128
#यदि_अन्तः

#यदि_परिभाषितम् _MSC_VER
#निर्देशः warning(प्रक्षिप)
#निर्देशः warning(disable: 4324)
#यदि_अन्तः /* _MSC_VER */

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
संरचना sqlite3_mutex { 
  सङ्घ {
    CRITICAL_SECTION cs;     /* The CRITICAL_SECTION mutex.  id==1 */
    SRWLOCK srwl;            /* The Slim reader/writer lock.  id!=1 */
  } u;
  पूर्णाङ्क id;                    /* Mutex type */
#यदि_परिभाषितम् SQLITE_DEBUG
  चञ्चल पूर्णाङ्क nRef;         /* Number of entrances */
  चञ्चल DWORD owner;      /* Thread holding this mutex */
  चञ्चल LONG trace;       /* True to trace changes */
#यदि_अन्तः
};
#यदि_परिभाषितम् _MSC_VER
#निर्देशः warning(निष्कासय)
#यदि_अन्तः /* _MSC_VER */

#यदि_परिभाषितम् SQLITE_DEBUG
/*
** The sqlite3_mutex_held() and sqlite3_mutex_notheld() routine are
** intended for use only inside assert() statements.
*/
स्थिर पूर्णाङ्क winMutexHeld(sqlite3_mutex *p){
  प्रत्यागम p->nRef!=0 && p->owner==GetCurrentThreadId();
}

स्थिर पूर्णाङ्क winMutexNotheld(sqlite3_mutex *p){
  प्रत्यागम p->nRef==0 || p->owner!=GetCurrentThreadId();
}
#यदि_अन्तः

/*
** Try to provide a memory barrier operation, needed for initialization
** and also for the xShmBarrier method of the VFS in cases when SQLite is
** compiled without mutexes (SQLITE_THREADSAFE=0).
*/
शून्य sqlite3_स्मृतिः_Barrier(शून्य){
#यदि defined(SQLITE_स्मृतिः_BARRIER)
  SQLITE_स्मृतिः_BARRIER;
#नोचेद्यदि defined(__GNUC__)
  __sync_synchronize();
#नोचेद्यदि MSVC_संस्करणम्>=1400
  _ReadWriteBarrier();
#नोचेद्यदि defined(स्मृतिः_Barrier)
  स्मृतिः_Barrier();
#यदि_अन्तः
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
स्थिर ALIGN128 सङ्घ staticMutexes {
  sqlite3_mutex m;
  वर्ण spacer[128];
} aWindowsMutex[12];
स्थिर पूर्णाङ्क winMutex_isInit = 0;

/* As the winMutexInit() and winMutexEnd() functions are called as part
** of the sqlite3_initialize() and sqlite3_shutdown() processing, the
** "interlocked" magic used here is probably not strictly necessary.
*/
स्थिर LONG चञ्चल win_Mutex_तालकम् = 0;

शून्य sqlite3_win32_sleep(DWORD milliseconds); /* os_win.c */

स्थिर पूर्णाङ्क winMutexInit(शून्य){
  /* The first to increment to 1 does actual initialization */
  यदि( InterlockedCompareExchange(&win_Mutex_तालकम्, 1, 0)==0 ){
    पूर्णाङ्क i;
    प्रति(i=0; i<Array_आकार(aWindowsMutex); i++){
      sqlite3_mutex *p = &aWindowsMutex[i].m;
      p->id = i+2;
      InitializeSRWLock(&p->u.srwl);
#यदि_परिभाषितम् SQLITE_DEBUG
      p->nRef = 0;
      p->owner = 0;
      p->trace = 0;
#यदि_अन्तः
    }
    winMutex_isInit = 1;
  }अन्यथा{
    /* Another thread is (in the process of) initializing the static
    ** mutexes */
    यावत्( !winMutex_isInit ){
      sqlite3_win32_sleep(1);
    }
  }
  प्रत्यागम SQLITE_OK;
}

स्थिर पूर्णाङ्क win_Mutex_अन्तः(शून्य){
  /* The first to decrement to 0 does actual shutdown
  ** (which should be the last to shutdown.) */
  यदि( InterlockedCompareExchange(&win_Mutex_तालकम्, 0, 1)==1 ){
    यदि( winMutex_isInit==1 ){
      winMutex_isInit = 0;
    }
  }
  प्रत्यागम SQLITE_OK;
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
स्थिर sqlite3_mutex *winMutexAlloc(पूर्णाङ्क iType){
  sqlite3_mutex *p;

  विकल्प( iType ){
    पक्ष SQLITE_MUTEX_FAST:
    पक्ष SQLITE_MUTEX_RECURSIVE: {
      p = sqlite3MallocZero( परिमाण(*p) );
      यदि( p ){
        p->id = iType;
#यदि_परिभाषितम् SQLITE_DEBUG
#यदि_परिभाषितम् SQLITE_WIN32_MUTEX_TRACE_DYNAMIC
        p->trace = 1;
#यदि_अन्तः
#यदि_अन्तः
        यदि( iType==SQLITE_MUTEX_RECURSIVE ){
          InitializeCriticalSection(&p->u.cs);
        }अन्यथा{
          InitializeSRWLock(&p->u.srwl);
        }
      }
      भङ्ग;
    }
    स्वभाव: {
#यदि_परिभाषितम् SQLITE_ENABLE_API_ARMOR
      यदि( iType-2<0 || iType-2>=Array_आकार(aWindowsMutex) ){
        (शून्य)SQLITE_MISUSE_BKPT;
        प्रत्यागम 0;
      }
#यदि_अन्तः
      p = &aWindowsMutex[iType-2].m;
#यदि_परिभाषितम् SQLITE_DEBUG
#यदि_परिभाषितम् SQLITE_WIN32_MUTEX_TRACE_STATIC
      InterlockedCompareExchange(&p->trace, 1, 0);
#यदि_अन्तः
#यदि_अन्तः
      भङ्ग;
    }
  }
  प्रतिज्ञा( p==0 || p->id==iType );
  प्रत्यागम p;
}


/*
** This routine deallocates a previously
** allocated mutex.  SQLite is careful to deallocate every
** mutex that it allocates.
*/
स्थिर शून्य winMutexFree(sqlite3_mutex *p){
  प्रतिज्ञा( p );
  प्रतिज्ञा( p->nRef==0 && p->owner==0 );
  यदि( p->id==SQLITE_MUTEX_FAST ){
    sqlite3_free(p);
  }अन्यथा यदि( p->id==SQLITE_MUTEX_RECURSIVE ){
    DeleteCriticalSection(&p->u.cs);
    sqlite3_free(p);
  }अन्यथा{
#यदि_परिभाषितम् SQLITE_ENABLE_API_ARMOR
    (शून्य)SQLITE_MISUSE_BKPT;
#यदि_अन्तः
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
स्थिर शून्य winMutexEnter(sqlite3_mutex *p){
  प्रतिज्ञा( p );
#यदि_परिभाषितम् SQLITE_DEBUG
  प्रतिज्ञा( p->id==SQLITE_MUTEX_RECURSIVE || winMutexNotheld(p) );
#यदि_अन्तः
  प्रतिज्ञा( winMutex_isInit==1 );
  यदि( p->id==SQLITE_MUTEX_RECURSIVE ){
    EnterCriticalSection(&p->u.cs);
  }अन्यथा{
    AcquireSRWLockExclusive(&p->u.srwl);
  }
#यदि_परिभाषितम् SQLITE_DEBUG
  प्रतिज्ञा( p->nRef>0 || p->owner==0 );
  p->owner = GetCurrentThreadId();
  p->nRef++;
  यदि( p->trace ){
    OSTRACE(("ENTER-MUTEX tid=%lu, mutex(%d)=%p (%d), nRef=%d\n",
             GetCurrentThreadId(), p->id, p, p->trace, p->nRef));
  }
#यदि_अन्तः
}

स्थिर पूर्णाङ्क winMutexTry(sqlite3_mutex *p){
  पूर्णाङ्क rc = SQLITE_BUSY;
  प्रतिज्ञा( p );
  प्रतिज्ञा( p->id==SQLITE_MUTEX_RECURSIVE || winMutexNotheld(p) );
  /*
  ** The sqlite3_mutex_try() routine is seldom used, and when it is
  ** used it is merely an optimization.  So it is OK for it to always
  ** fail.
  */
  यदि( p->id==SQLITE_MUTEX_RECURSIVE ){
    rc = TryEnterCriticalSection(&p->u.cs);
  }अन्यथा{
    rc = TryAcquireSRWLockExclusive(&p->u.srwl);
  }
  यदि( rc ){
#यदि_परिभाषितम् SQLITE_DEBUG
    p->owner = GetCurrentThreadId();
    p->nRef++;
#यदि_अन्तः
    rc = SQLITE_OK;
  }अन्यथा{
    rc = SQLITE_BUSY;
  }
#यदि_परिभाषितम् SQLITE_DEBUG
  यदि( p->trace ){
    OSTRACE((
       "TRY-MUTEX tid=%lu, mutex(%d)=%p (%d), owner=%lu, nRef=%d, rc=%s\n",
       GetCurrentThreadId(), p->id, p, p->trace, p->owner, p->nRef,
       sqlite3ErrName(rc)));
  }
#यदि_अन्तः
  प्रत्यागम rc;
}

/*
** The sqlite3_mutex_leave() routine exits a mutex that was
** previously entered by the same thread.  The behavior
** is undefined if the mutex is not currently entered or
** is not currently allocated.  SQLite will never do either.
*/
स्थिर शून्य winMutexLeave(sqlite3_mutex *p){
  प्रतिज्ञा( p );
#यदि_परिभाषितम् SQLITE_DEBUG
  प्रतिज्ञा( p->nRef>0 );
  प्रतिज्ञा( p->owner==GetCurrentThreadId() );
  p->nRef--;
  यदि( p->nRef==0 ) p->owner = 0;
  प्रतिज्ञा( p->nRef==0 || p->id==SQLITE_MUTEX_RECURSIVE );
#यदि_अन्तः
  प्रतिज्ञा( winMutex_isInit==1 );
  यदि( p->id==SQLITE_MUTEX_RECURSIVE ){
    LeaveCriticalSection(&p->u.cs);
  }अन्यथा{
    ReleaseSRWLockExclusive(&p->u.srwl);
  }
#यदि_परिभाषितम् SQLITE_DEBUG
  यदि( p->trace ){
    OSTRACE(("LEAVE-MUTEX tid=%lu, mutex(%d)=%p (%d), nRef=%d\n",
             GetCurrentThreadId(), p->id, p, p->trace, p->nRef));
  }
#यदि_अन्तः
}

sqlite3_mutex_methods ध्रुव *sqlite3DefaultMutex(शून्य){
  स्थिर ध्रुव sqlite3_mutex_methods sMutex = {
    winMutexInit,
    win_Mutex_अन्तः,
    winMutexAlloc,
    winMutexFree,
    winMutexEnter,
    winMutexTry,
    winMutexLeave,
#यदि_परिभाषितम् SQLITE_DEBUG
    winMutexHeld,
    winMutexNotheld
#अन्यथा
    0,
    0
#यदि_अन्तः
  };
  प्रत्यागम &sMutex;
}

#यदि_अन्तः /* SQLITE_MUTEX_W32 */
