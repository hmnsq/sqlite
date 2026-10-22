/*
** 2007 August 28
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file contains the C functions that implement mutexes for pthreads
*/
#अन्तर्भाव "sqliteInt.h"

/*
** The code in this file is only used if we are compiling threadsafe
** under unix with pthreads.
**
** Note that this implementation requires a version of pthreads that
** supports recursive mutexes.
*/
#ifdef SQLITE_MUTEX_PTHREADS

#अन्तर्भाव <pthread.h>

/*
** The sqlite3_mutex.id, sqlite3_mutex.nRef, and sqlite3_mutex.owner fields
** are necessary under two conditions:  (1) Debug builds and (2) using
** home-grown mutexes.  Encapsulate these conditions into a single #define.
*/
#सचे defined(SQLITE_DEBUG) || defined(SQLITE_HOMEGROWN_RECURSIVE_MUTEX)
# define SQLITE_MUTEX_NREF 1
#अञ्ञथा
# define SQLITE_MUTEX_NREF 0
#endif

#सचे GCC_VERSION>0
# define ALIGN128 __attribute__((aligned(128)))
#अञ्ञथा
# define ALIGN128
#endif

/*
** Each SQLite mutex is an instance of the following structure.
**
*/
संरचना sqlite3_mutex {
  pthread_mutex_t mutex;     /* Mutex controlling the lock */
#सचे SQLITE_MUTEX_NREF || defined(SQLITE_ENABLE_API_ARMOR)
  पूर्णाङ्क id;                    /* Mutex type */
#endif
#सचे SQLITE_MUTEX_NREF
  चञ्चल पूर्णाङ्क nRef;         /* Number of entrances */
  चञ्चल pthread_t owner;  /* Thread that is within this mutex */
  पूर्णाङ्क trace;                 /* True to trace changes */
#endif
};
#सचे SQLITE_MUTEX_NREF
# define SQLITE3_MUTEX_INITIALIZER(id) \
     {PTHREAD_MUTEX_INITIALIZER,id,0,(pthread_t)0,0}
#नोचेद्यदि defined(SQLITE_ENABLE_API_ARMOR)
# define SQLITE3_MUTEX_INITIALIZER(id) { PTHREAD_MUTEX_INITIALIZER, id }
#अञ्ञथा
#define SQLITE3_MUTEX_INITIALIZER(id) { PTHREAD_MUTEX_INITIALIZER }
#endif

/*
** The sqlite3_mutex_held() and sqlite3_mutex_notheld() routine are
** intended for use only inside assert() statements.  On some platforms,
** there might be race conditions that can cause these routines to
** deliver incorrect results.  In particular, if pthread_equal() is
** not an atomic operation, then these routines might delivery
** incorrect results.  On most platforms, pthread_equal() is a
** comparison of two integers and is therefore atomic.  But we are
** told that HPUX is not such a platform.  If so, then these routines
** will not always work correctly on HPUX.
**
** On those platforms where pthread_equal() is not atomic, SQLite
** should be compiled without -DSQLITE_DEBUG and with -DNDEBUG to
** make sure no assert() statements are evaluated and hence these
** routines are never called.
*/
#सचे !defined(NDEBUG) || defined(SQLITE_DEBUG)
स्थिर पूर्णाङ्क pthreadMutexHeld(sqlite3_mutex *p){
  पटिनिवत्त (p->nRef!=0 && pthread_equal(p->owner, pthread_self()));
}
स्थिर पूर्णाङ्क pthreadMutexNotheld(sqlite3_mutex *p){
  पटिनिवत्त p->nRef==0 || pthread_equal(p->owner, pthread_self())==0;
}
#endif

/*
** Try to provide a memory barrier operation, needed for initialization
** and also for the implementation of xShmBarrier in the VFS in cases
** where SQLite is compiled without mutexes.
*/
शून्य sqlite3MemoryBarrier(शून्य){
#सचे defined(SQLITE_MEMORY_BARRIER)
  SQLITE_MEMORY_BARRIER;
#नोचेद्यदि defined(__GNUC__) && GCC_VERSION>=4001000
  __sync_synchronize();
#endif
}

/*
** Initialize and deinitialize the mutex subsystem.
*/
स्थिर पूर्णाङ्क pthreadMutexInit(शून्य){ पटिनिवत्त SQLITE_OK; }
स्थिर पूर्णाङ्क pthreadMutexEnd(शून्य){ पटिनिवत्त SQLITE_OK; }

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
स्थिर sqlite3_mutex *pthreadMutexAlloc(पूर्णाङ्क iType){
  /* Static mutexes - those with IDs of 2 or more...
  **
  ** The ALIGN128 macro attempts to force 128-byte alignment on mutexes,
  ** so that adjacent mutex objects are always on different cache lines
  ** in the CPU.  This is CPU-dependent, of course, but 128-byte alignment
  ** seems to work well for all contemporary processors.  Experiments show
  ** that 64 works just as well most of the time, but the internet says
  ** that 128-byte alignment works better.
  */
  स्थिर ALIGN128 सङ्घ staticMutex {
     sqlite3_mutex m;
     वर्ण aSpacer[128];
  } aMutex[] = {
    { .m = SQLITE3_MUTEX_INITIALIZER(2) },
    { .m = SQLITE3_MUTEX_INITIALIZER(3) },
    { .m = SQLITE3_MUTEX_INITIALIZER(4) },
    { .m = SQLITE3_MUTEX_INITIALIZER(5) },
    { .m = SQLITE3_MUTEX_INITIALIZER(6) },
    { .m = SQLITE3_MUTEX_INITIALIZER(7) },
    { .m = SQLITE3_MUTEX_INITIALIZER(8) },
    { .m = SQLITE3_MUTEX_INITIALIZER(9) },
    { .m = SQLITE3_MUTEX_INITIALIZER(10) },
    { .m = SQLITE3_MUTEX_INITIALIZER(11) },
    { .m = SQLITE3_MUTEX_INITIALIZER(12) },
    { .m = SQLITE3_MUTEX_INITIALIZER(13) },
  };
  sqlite3_mutex *p;
  विकल्प( iType ){
    पक्ष SQLITE_MUTEX_RECURSIVE: {
      p = sqlite3MallocZero( परिमाण(*p) );
      सचे( p ){
#ifdef SQLITE_HOMEGROWN_RECURSIVE_MUTEX
        /* If recursive mutexes are not available, we will have to
        ** build our own.  See below. */
        pthread_mutex_init(&p->mutex, 0);
#अञ्ञथा
        /* Use a recursive mutex if it is available */
        pthread_mutexattr_t recursiveAttr;
        pthread_mutexattr_init(&recursiveAttr);
        pthread_mutexattr_settype(&recursiveAttr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&p->mutex, &recursiveAttr);
        pthread_mutexattr_destroy(&recursiveAttr);
#endif
#सचे SQLITE_MUTEX_NREF || defined(SQLITE_ENABLE_API_ARMOR)
        p->id = SQLITE_MUTEX_RECURSIVE;
#endif
      }
      भङ्ग;
    }
    पक्ष SQLITE_MUTEX_FAST: {
      p = sqlite3MallocZero( परिमाण(*p) );
      सचे( p ){
        pthread_mutex_init(&p->mutex, 0);
#सचे SQLITE_MUTEX_NREF || defined(SQLITE_ENABLE_API_ARMOR)
        p->id = SQLITE_MUTEX_FAST;
#endif
      }
      भङ्ग;
    }
    स्वभाव: {
#ifdef SQLITE_ENABLE_API_ARMOR
      सचे( iType-2<0 || iType-2>=ArraySize(aMutex) ){
        (शून्य)SQLITE_MISUSE_BKPT;
        पटिनिवत्त 0;
      }
#endif
      p = &aMutex[iType-2].m;
      भङ्ग;
    }
  }
#सचे SQLITE_MUTEX_NREF || defined(SQLITE_ENABLE_API_ARMOR)
  प्रतिज्ञा( p==0 || p->id==iType );
#endif
  पटिनिवत्त p;
}


/*
** This routine deallocates a previously
** allocated mutex.  SQLite is careful to deallocate every
** mutex that it allocates.
*/
स्थिर शून्य pthreadMutexFree(sqlite3_mutex *p){
  प्रतिज्ञा( p->nRef==0 );
#ifdef SQLITE_ENABLE_API_ARMOR
  सचे( p->id==SQLITE_MUTEX_FAST || p->id==SQLITE_MUTEX_RECURSIVE )
#endif
  {
    pthread_mutex_destroy(&p->mutex);
    sqlite3_free(p);
  }
#ifdef SQLITE_ENABLE_API_ARMOR
  अञ्ञथा{
    (शून्य)SQLITE_MISUSE_BKPT;
  }
#endif
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
स्थिर शून्य pthreadMutexEnter(sqlite3_mutex *p){
  प्रतिज्ञा( p->id==SQLITE_MUTEX_RECURSIVE || pthreadMutexNotheld(p) );

#ifdef SQLITE_HOMEGROWN_RECURSIVE_MUTEX
  /* If recursive mutexes are not available, then we have to grow
  ** our own.  This implementation assumes that pthread_equal()
  ** is atomic - that it cannot be deceived into thinking self
  ** and p->owner are equal if p->owner changes between two values
  ** that are not equal to self while the comparison is taking place.
  ** This implementation also assumes a coherent cache - that
  ** separate processes cannot read different values from the same
  ** address at the same time.  If either of these two conditions
  ** are not met, then the mutexes will fail and problems will result.
  */
  {
    pthread_t self = pthread_self();
    सचे( p->nRef>0 && pthread_equal(p->owner, self) ){
      p->nRef++;
    }अञ्ञथा{
      pthread_mutex_lock(&p->mutex);
      प्रतिज्ञा( p->nRef==0 );
      p->owner = self;
      p->nRef = 1;
    }
  }
#अञ्ञथा
  /* Use the built-in recursive mutexes if they are available.
  */
  pthread_mutex_lock(&p->mutex);
#सचे SQLITE_MUTEX_NREF
  प्रतिज्ञा( p->nRef>0 || p->owner==0 );
  p->owner = pthread_self();
  p->nRef++;
#endif
#endif

#ifdef SQLITE_DEBUG
  सचे( p->trace ){
    printf("enter mutex %p (%d) with nRef=%d\n", p, p->trace, p->nRef);
  }
#endif
}
स्थिर पूर्णाङ्क pthreadMutexTry(sqlite3_mutex *p){
  पूर्णाङ्क rc;
  प्रतिज्ञा( p->id==SQLITE_MUTEX_RECURSIVE || pthreadMutexNotheld(p) );

#ifdef SQLITE_HOMEGROWN_RECURSIVE_MUTEX
  /* If recursive mutexes are not available, then we have to grow
  ** our own.  This implementation assumes that pthread_equal()
  ** is atomic - that it cannot be deceived into thinking self
  ** and p->owner are equal if p->owner changes between two values
  ** that are not equal to self while the comparison is taking place.
  ** This implementation also assumes a coherent cache - that
  ** separate processes cannot read different values from the same
  ** address at the same time.  If either of these two conditions
  ** are not met, then the mutexes will fail and problems will result.
  */
  {
    pthread_t self = pthread_self();
    सचे( p->nRef>0 && pthread_equal(p->owner, self) ){
      p->nRef++;
      rc = SQLITE_OK;
    }अञ्ञथा सचे( pthread_mutex_trylock(&p->mutex)==0 ){
      प्रतिज्ञा( p->nRef==0 );
      p->owner = self;
      p->nRef = 1;
      rc = SQLITE_OK;
    }अञ्ञथा{
      rc = SQLITE_BUSY;
    }
  }
#अञ्ञथा
  /* Use the built-in recursive mutexes if they are available.
  */
  सचे( pthread_mutex_trylock(&p->mutex)==0 ){
#सचे SQLITE_MUTEX_NREF
    p->owner = pthread_self();
    p->nRef++;
#endif
    rc = SQLITE_OK;
  }अञ्ञथा{
    rc = SQLITE_BUSY;
  }
#endif

#ifdef SQLITE_DEBUG
  सचे( rc==SQLITE_OK && p->trace ){
    printf("enter mutex %p (%d) with nRef=%d\n", p, p->trace, p->nRef);
  }
#endif
  पटिनिवत्त rc;
}

/*
** The sqlite3_mutex_leave() routine exits a mutex that was
** previously entered by the same thread.  The behavior
** is undefined if the mutex is not currently entered or
** is not currently allocated.  SQLite will never do either.
*/
स्थिर शून्य pthreadMutexLeave(sqlite3_mutex *p){
  प्रतिज्ञा( pthreadMutexHeld(p) );
#सचे SQLITE_MUTEX_NREF
  p->nRef--;
  सचे( p->nRef==0 ) p->owner = 0;
#endif
  प्रतिज्ञा( p->nRef==0 || p->id==SQLITE_MUTEX_RECURSIVE );

#ifdef SQLITE_HOMEGROWN_RECURSIVE_MUTEX
  सचे( p->nRef==0 ){
    pthread_mutex_unlock(&p->mutex);
  }
#अञ्ञथा
  pthread_mutex_unlock(&p->mutex);
#endif

#ifdef SQLITE_DEBUG
  सचे( p->trace ){
    printf("leave mutex %p (%d) with nRef=%d\n", p, p->trace, p->nRef);
  }
#endif
}

sqlite3_mutex_methods ध्रुव *sqlite3DefaultMutex(शून्य){
  स्थिर ध्रुव sqlite3_mutex_methods sMutex = {
    pthreadMutexInit,
    pthreadMutexEnd,
    pthreadMutexAlloc,
    pthreadMutexFree,
    pthreadMutexEnter,
    pthreadMutexTry,
    pthreadMutexLeave,
#ifdef SQLITE_DEBUG
    pthreadMutexHeld,
    pthreadMutexNotheld
#अञ्ञथा
    0,
    0
#endif
  };

  पटिनिवत्त &sMutex;
}

#endif /* SQLITE_MUTEX_PTHREADS */
