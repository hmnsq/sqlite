/*
** 2001 September 15
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
**
** Memory allocation functions used throughout sqlite.
*/
#अन्तर्भाव "sqliteInt.h"
#अन्तर्भाव <stdarg.h>

/*
** Attempt to release up to n bytes of non-essential memory currently
** held by SQLite. An example of non-essential memory is memory used to
** cache database pages that are not currently in use.
*/
पूर्णाङ्क sqlite3_release_memory(पूर्णाङ्क n){
#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
  पटिनिवत्त sqlite3PcacheReleaseMemory(n);
#अञ्ञथा
  /* IMPLEMENTATION-OF: R-34391-24921 The sqlite3_release_memory() routine
  ** is a no-op returning zero if SQLite is not compiled with
  ** SQLITE_ENABLE_MEMORY_MANAGEMENT. */
  UNUSED_PARAMETER(n);
  पटिनिवत्त 0;
#endif
}

/*
** Default value of the hard heap limit.  0 means "no limit".
*/
#ifndef SQLITE_MAX_MEMORY
# define SQLITE_MAX_MEMORY 0
#endif

/*
** State information local to the memory allocation subsystem.
*/
स्थिर SQLITE_WSD संरचना Mem0Global {
  sqlite3_mutex *mutex;         /* Mutex to serialize access */
  sqlite3_int64 alarmThreshold; /* The soft heap limit */
  sqlite3_int64 hardLimit;      /* The hard upper bound on memory */

  /*
  ** True if heap is nearly "full" where "full" is defined by the
  ** sqlite3_soft_heap_limit() setting.
  */
  पूर्णाङ्क nearlyFull;
} mem0 = { 0, SQLITE_MAX_MEMORY, SQLITE_MAX_MEMORY, 0 };

#define mem0 GLOBAL(संरचना Mem0Global, mem0)

/*
** Return the memory allocator mutex. sqlite3_status() needs it.
*/
sqlite3_mutex *sqlite3MallocMutex(शून्य){
  पटिनिवत्त mem0.mutex;
}

#ifndef SQLITE_OMIT_DEPRECATED
/*
** Deprecated external interface.  It used to set an alarm callback
** that was invoked when memory usage grew too large.  Now it is a
** no-op.
*/
पूर्णाङ्क sqlite3_memory_alarm(
  शून्य(*xCallback)(शून्य *pArg, sqlite3_int64 used,पूर्णाङ्क N),
  शून्य *pArg,
  sqlite3_int64 iThreshold
){
  (शून्य)xCallback;
  (शून्य)pArg;
  (शून्य)iThreshold;
  पटिनिवत्त SQLITE_OK;
}
#endif

/*
** Set the soft heap-size limit for the library.  An argument of
** zero disables the limit.  A negative argument is a no-op used to
** obtain the return value.
**
** The return value is the value of the heap limit just before this
** interface was called.
**
** If the hard heap limit is enabled, then the soft heap limit cannot
** be disabled nor raised above the hard heap limit.
*/
sqlite3_int64 sqlite3_soft_heap_limit64(sqlite3_int64 n){
  sqlite3_int64 priorLimit;
  sqlite3_int64 excess;
  sqlite3_int64 nUsed;
#ifndef SQLITE_OMIT_AUTOINIT
  पूर्णाङ्क rc = sqlite3_initialize();
  सचे( rc ) पटिनिवत्त -1;
#endif
  sqlite3_mutex_enter(mem0.mutex);
  priorLimit = mem0.alarmThreshold;
  सचे( n<0 ){
    sqlite3_mutex_leave(mem0.mutex);
    पटिनिवत्त priorLimit;
  }
  सचे( mem0.hardLimit>0 && (n>mem0.hardLimit || n==0) ){
    n = mem0.hardLimit;
  }
  mem0.alarmThreshold = n;
  nUsed = sqlite3_Status_मानम्(SQLITE_STATUS_MEMORY_USED);
  AtomicStore(&mem0.nearlyFull, n>0 && n<=nUsed);
  sqlite3_mutex_leave(mem0.mutex);
  excess = sqlite3_memory_used() - n;
  सचे( excess>0 ) sqlite3_release_memory((पूर्णाङ्क)(excess & 0x7fffffff));
  पटिनिवत्त priorLimit;
}
शून्य sqlite3_soft_heap_limit(पूर्णाङ्क n){
  सचे( n<0 ) n = 0;
  sqlite3_soft_heap_limit64(n);
}

/*
** Set the hard heap-size limit for the library. An argument of zero
** disables the hard heap limit.  A negative argument is a no-op used
** to obtain the return value without affecting the hard heap limit.
**
** The return value is the value of the hard heap limit just prior to
** calling this interface.
**
** Setting the hard heap limit will also activate the soft heap limit
** and constrain the soft heap limit to be no more than the hard heap
** limit.
*/
sqlite3_int64 sqlite3_hard_heap_limit64(sqlite3_int64 n){
  sqlite3_int64 priorLimit;
#ifndef SQLITE_OMIT_AUTOINIT
  पूर्णाङ्क rc = sqlite3_initialize();
  सचे( rc ) पटिनिवत्त -1;
#endif
  sqlite3_mutex_enter(mem0.mutex);
  priorLimit = mem0.hardLimit;
  सचे( n>=0 ){
    mem0.hardLimit = n;
    सचे( n<mem0.alarmThreshold || mem0.alarmThreshold==0 ){
      mem0.alarmThreshold = n;
    }
  }
  sqlite3_mutex_leave(mem0.mutex);
  पटिनिवत्त priorLimit;
}


/*
** Initialize the memory allocation subsystem.
*/
पूर्णाङ्क sqlite3MallocInit(शून्य){
  पूर्णाङ्क rc;
  सचे( sqlite3GlobalConfig.m.xMalloc==0 ){
    sqlite3MemSetDefault();
  }
  mem0.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM);
  सचे( sqlite3GlobalConfig.pPage==0 || sqlite3GlobalConfig.szPage<512
      || sqlite3GlobalConfig.nPage<=0 ){
    sqlite3GlobalConfig.pPage = 0;
    sqlite3GlobalConfig.szPage = 0;
  }
  rc = sqlite3GlobalConfig.m.xInit(sqlite3GlobalConfig.m.pAppData);
  सचे( rc!=SQLITE_OK ) memset(&mem0, 0, परिमाण(mem0));
  पटिनिवत्त rc;
}

/*
** Return true if the heap is currently under memory pressure - in other
** words if the amount of heap used is close to the limit set by
** sqlite3_soft_heap_limit().
*/
पूर्णाङ्क sqlite3HeapNearlyFull(शून्य){
  पटिनिवत्त AtomicLoad(&mem0.nearlyFull);
}

/*
** Deinitialize the memory allocation subsystem.
*/
शून्य sqlite3MallocEnd(शून्य){
  सचे( sqlite3GlobalConfig.m.xShutdown ){
    sqlite3GlobalConfig.m.xShutdown(sqlite3GlobalConfig.m.pAppData);
  }
  memset(&mem0, 0, परिमाण(mem0));
}

/*
** Return the amount of memory currently checked out.
*/
sqlite3_int64 sqlite3_memory_used(शून्य){
  sqlite3_int64 res, mx;
  sqlite3_status64(SQLITE_STATUS_MEMORY_USED, &res, &mx, 0);
  पटिनिवत्त res;
}

/*
** Return the maximum amount of memory that has ever been
** checked out since either the beginning of this process
** or since the most recent reset.
*/
sqlite3_int64 sqlite3_memory_highwater(पूर्णाङ्क resetFlag){
  sqlite3_int64 res, mx;
  sqlite3_status64(SQLITE_STATUS_MEMORY_USED, &res, &mx, resetFlag);
  पटिनिवत्त mx;
}

/*
** Trigger the alarm 
*/
स्थिर शून्य sqlite3MallocAlarm(पूर्णाङ्क nByte){
  सचे( mem0.alarmThreshold<=0 ) पटिनिवत्त;
  sqlite3_mutex_leave(mem0.mutex);
  sqlite3_release_memory(nByte);
  sqlite3_mutex_enter(mem0.mutex);
}

#ifdef SQLITE_DEBUG
/*
** This routine is called whenever an out-of-memory condition is seen,
** It's only purpose to to serve as a breakpoint for gdb or similar
** code debuggers when working on out-of-memory conditions, for example
** caused by PRAGMA hard_heap_limit=N.
*/
स्थिर SQLITE_NOINLINE शून्य test_oom_breakpoint(u64 n){
  स्थिर u64 nOomFault = 0;
  nOomFault += n;
  /* The assert() is never reached in a human lifetime.  It  is here mostly
  ** to prevent code optimizers from optimizing out this function. */
  प्रतिज्ञा( (nOomFault>>32) < 0xffffffff );
}
#अञ्ञथा
# define test_oom_breakpoint(X)   /* No-op for production builds */
#endif

/*
** Do a memory allocation with statistics and alarms.  Assume the
** lock is already held.
*/
स्थिर शून्य mallocWithAlarm(पूर्णाङ्क n, शून्य **pp){
  शून्य *p;
  पूर्णाङ्क nFull;
  प्रतिज्ञा( sqlite3_mutex_held(mem0.mutex) );
  प्रतिज्ञा( n>0 );

  /* In Firefox (circa 2017-02-08), xRoundup() is remapped to an internal
  ** implementation of malloc_good_size(), which must be called in debug
  ** mode and specifically when the DMD "Dark Matter Detector" is enabled
  ** or else a crash results.  Hence, do not attempt to optimize out the
  ** following xRoundup() call. */
  nFull = sqlite3GlobalConfig.m.xRoundup(n);

  sqlite3StatusHighwater(SQLITE_STATUS_MALLOC_SIZE, n);
  सचे( mem0.alarmThreshold>0 ){
    sqlite3_int64 nUsed = sqlite3_Status_मानम्(SQLITE_STATUS_MEMORY_USED);
    सचे( nUsed >= mem0.alarmThreshold - nFull ){
      AtomicStore(&mem0.nearlyFull, 1);
      sqlite3MallocAlarm(nFull);
      सचे( mem0.hardLimit ){
        nUsed = sqlite3_Status_मानम्(SQLITE_STATUS_MEMORY_USED);
        सचे( nUsed >= mem0.hardLimit - nFull ){
          test_oom_breakpoint(1);
          *pp = 0;
          पटिनिवत्त;
        }
      }
    }अञ्ञथा{
      AtomicStore(&mem0.nearlyFull, 0);
    }
  }
  p = sqlite3GlobalConfig.m.xMalloc(nFull);
#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
  सचे( p==0 && mem0.alarmThreshold>0 ){
    sqlite3MallocAlarm(nFull);
    p = sqlite3GlobalConfig.m.xMalloc(nFull);
  }
#endif
  सचे( p ){
    nFull = sqlite3MallocSize(p);
    sqlite3StatusUp(SQLITE_STATUS_MEMORY_USED, nFull);
    sqlite3StatusUp(SQLITE_STATUS_MALLOC_गणनम्, 1);
  }
  *pp = p;
}

/*
** Allocate memory.  This routine is like sqlite3_malloc() except that it
** assumes the memory subsystem has already been initialized.
*/
शून्य *sqlite3Malloc(u64 n){
  शून्य *p;
  सचे( n==0 || n>SQLITE_MAX_ALLOCATION_SIZE ){
    p = 0;
  }अञ्ञथा सचे( sqlite3GlobalConfig.bMemstat ){
    sqlite3_mutex_enter(mem0.mutex);
    mallocWithAlarm((पूर्णाङ्क)n, &p);
    sqlite3_mutex_leave(mem0.mutex);
  }अञ्ञथा{
    p = sqlite3GlobalConfig.m.xMalloc((पूर्णाङ्क)n);
  }
  प्रतिज्ञा( EIGHT_BYTE_ALIGNMENT(p) );  /* IMP: R-11148-40995 */
  पटिनिवत्त p;
}

/*
** This version of the memory allocation is for use by the application.
** First make sure the memory subsystem is initialized, then do the
** allocation.
*/
शून्य *sqlite3_malloc(पूर्णाङ्क n){
#ifndef SQLITE_OMIT_AUTOINIT
  सचे( sqlite3_initialize() ) पटिनिवत्त 0;
#endif
  पटिनिवत्त n<=0 ? 0 : sqlite3Malloc(n);
}
शून्य *sqlite3_malloc64(sqlite3_uint64 n){
#ifndef SQLITE_OMIT_AUTOINIT
  सचे( sqlite3_initialize() ) पटिनिवत्त 0;
#endif
  पटिनिवत्त sqlite3Malloc(n);
}

/*
** TRUE if p is a lookaside memory allocation from db
*/
#ifndef SQLITE_OMIT_LOOKASIDE
स्थिर पूर्णाङ्क isLookaside(sqlite3 *db, ध्रुव शून्य *p){
  पटिनिवत्त SQLITE_WITHIN(p, db->lookaside.pStart, db->lookaside.pTrueEnd);
}
#अञ्ञथा
#define isLookaside(A,B) 0
#endif

/*
** Return the size of a memory allocation previously obtained from
** sqlite3Malloc() or sqlite3_malloc().
*/
पूर्णाङ्क sqlite3MallocSize(ध्रुव शून्य *p){
  प्रतिज्ञा( sqlite3MemdebugHasType(p, MEMTYPE_HEAP) );
  पटिनिवत्त sqlite3GlobalConfig.m.xSize((शून्य*)p);
}
स्थिर पूर्णाङ्क lookasideMallocSize(sqlite3 *db, ध्रुव शून्य *p){
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE    
  पटिनिवत्त p<db->lookaside.p_मध्यः ? db->lookaside.szTrue : LOOKASIDE_SMALL;
#अञ्ञथा
  पटिनिवत्त db->lookaside.szTrue;
#endif  
}
पूर्णाङ्क sqlite3DbMallocSize(sqlite3 *db, ध्रुव शून्य *p){
  प्रतिज्ञा( p!=0 );
#ifdef SQLITE_DEBUG
  सचे( db==0 ){
    प्रतिज्ञा( sqlite3MemdebugNoType(p, (u8)~MEMTYPE_HEAP) );
    प्रतिज्ञा( sqlite3MemdebugHasType(p, MEMTYPE_HEAP) );
  }अञ्ञथा सचे( !isLookaside(db,p) ){
    प्रतिज्ञा( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
    प्रतिज्ञा( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  }
#endif
  सचे( db ){
    सचे( ((uptr)p)<(uptr)(db->lookaside.pTrueEnd) ){
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
      सचे( ((uptr)p)>=(uptr)(db->lookaside.p_मध्यः) ){
        प्रतिज्ञा( sqlite3_mutex_held(db->mutex) );
        पटिनिवत्त LOOKASIDE_SMALL;
      }
#endif
      सचे( ((uptr)p)>=(uptr)(db->lookaside.pStart) ){
        प्रतिज्ञा( sqlite3_mutex_held(db->mutex) );
        पटिनिवत्त db->lookaside.szTrue;
      }
    }
  }
  पटिनिवत्त sqlite3GlobalConfig.m.xSize((शून्य*)p);
}
sqlite3_uint64 sqlite3_msize(शून्य *p){
  प्रतिज्ञा( sqlite3MemdebugNoType(p, (u8)~MEMTYPE_HEAP) );
  प्रतिज्ञा( sqlite3MemdebugHasType(p, MEMTYPE_HEAP) );
  पटिनिवत्त p ? sqlite3GlobalConfig.m.xSize(p) : 0;
}

/*
** Free memory previously obtained from sqlite3Malloc().
*/
शून्य sqlite3_free(शून्य *p){
  सचे( p==0 ) पटिनिवत्त;  /* IMP: R-49053-54554 */
  प्रतिज्ञा( sqlite3MemdebugHasType(p, MEMTYPE_HEAP) );
  प्रतिज्ञा( sqlite3MemdebugNoType(p, (u8)~MEMTYPE_HEAP) );
  सचे( sqlite3GlobalConfig.bMemstat ){
    sqlite3_mutex_enter(mem0.mutex);
    sqlite3StatusDown(SQLITE_STATUS_MEMORY_USED, sqlite3MallocSize(p));
    sqlite3StatusDown(SQLITE_STATUS_MALLOC_गणनम्, 1);
    sqlite3GlobalConfig.m.xFree(p);
    sqlite3_mutex_leave(mem0.mutex);
  }अञ्ञथा{
    sqlite3GlobalConfig.m.xFree(p);
  }
}

/*
** Add the size of memory allocation "p" to the count in
** *db->pnBytesFreed.
*/
स्थिर SQLITE_NOINLINE शून्य measureAllocationSize(sqlite3 *db, शून्य *p){
  *db->pnBytesFreed += sqlite3DbMallocSize(db,p);
}

/*
** Free memory that might be associated with a particular database
** connection.  Calling sqlite3DbFree(D,X) for X==0 is a harmless no-op.
** The sqlite3DbFreeNN(D,X) version requires that X be non-NULL.
*/
शून्य sqlite3DbFreeNN(sqlite3 *db, शून्य *p){
  प्रतिज्ञा( db==0 || sqlite3_mutex_held(db->mutex) );
  प्रतिज्ञा( p!=0 );
  सचे( db ){
    सचे( ((uptr)p)<(uptr)(db->lookaside.pEnd) ){
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
      सचे( ((uptr)p)>=(uptr)(db->lookaside.p_मध्यः) ){
        LookasideSlot *pBuf = (LookasideSlot*)p;
        प्रतिज्ञा( db->pnBytesFreed==0 );
#ifdef SQLITE_DEBUG
        memset(p, 0xaa, LOOKASIDE_SMALL);  /* Trash freed content */
#endif
        pBuf->pNext = db->lookaside.pSmallFree;
        db->lookaside.pSmallFree = pBuf;
        पटिनिवत्त;
      }
#endif /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
      सचे( ((uptr)p)>=(uptr)(db->lookaside.pStart) ){
        LookasideSlot *pBuf = (LookasideSlot*)p;
        प्रतिज्ञा( db->pnBytesFreed==0 );
#ifdef SQLITE_DEBUG
        memset(p, 0xaa, db->lookaside.szTrue);  /* Trash freed content */
#endif
        pBuf->pNext = db->lookaside.pFree;
        db->lookaside.pFree = pBuf;
        पटिनिवत्त;
      }
    }
    सचे( db->pnBytesFreed ){
      measureAllocationSize(db, p);
      पटिनिवत्त;
    }
  }
  प्रतिज्ञा( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  प्रतिज्ञा( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  प्रतिज्ञा( db!=0 || sqlite3MemdebugNoType(p, MEMTYPE_LOOKASIDE) );
  sqlite3MemdebugSetType(p, MEMTYPE_HEAP);
  sqlite3_free(p);
}
शून्य sqlite3DbNNFreeNN(sqlite3 *db, शून्य *p){
  प्रतिज्ञा( db!=0 );
  प्रतिज्ञा( sqlite3_mutex_held(db->mutex) );
  प्रतिज्ञा( p!=0 );
  सचे( ((uptr)p)<(uptr)(db->lookaside.pEnd) ){
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
    सचे( ((uptr)p)>=(uptr)(db->lookaside.p_मध्यः) ){
      LookasideSlot *pBuf = (LookasideSlot*)p;
      प्रतिज्ञा( db->pnBytesFreed==0 );
#ifdef SQLITE_DEBUG
      memset(p, 0xaa, LOOKASIDE_SMALL);  /* Trash freed content */
#endif
      pBuf->pNext = db->lookaside.pSmallFree;
      db->lookaside.pSmallFree = pBuf;
      पटिनिवत्त;
    }
#endif /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
    सचे( ((uptr)p)>=(uptr)(db->lookaside.pStart) ){
      LookasideSlot *pBuf = (LookasideSlot*)p;
      प्रतिज्ञा( db->pnBytesFreed==0 );
#ifdef SQLITE_DEBUG
      memset(p, 0xaa, db->lookaside.szTrue);  /* Trash freed content */
#endif
      pBuf->pNext = db->lookaside.pFree;
      db->lookaside.pFree = pBuf;
      पटिनिवत्त;
    }
  }
  सचे( db->pnBytesFreed ){
    measureAllocationSize(db, p);
    पटिनिवत्त;
  }
  प्रतिज्ञा( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  प्रतिज्ञा( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  sqlite3MemdebugSetType(p, MEMTYPE_HEAP);
  sqlite3_free(p);
}
शून्य sqlite3DbFree(sqlite3 *db, शून्य *p){
  प्रतिज्ञा( db==0 || sqlite3_mutex_held(db->mutex) );
  सचे( p ) sqlite3DbFreeNN(db, p);
}

/*
** Change the size of an existing memory allocation
*/
शून्य *sqlite3Realloc(शून्य *pOld, u64 nBytes){
  पूर्णाङ्क nOld, nNew, nDiff;
  शून्य *pNew;
  प्रतिज्ञा( sqlite3MemdebugHasType(pOld, MEMTYPE_HEAP) );
  प्रतिज्ञा( sqlite3MemdebugNoType(pOld, (u8)~MEMTYPE_HEAP) );
  सचे( pOld==0 ){
    पटिनिवत्त sqlite3Malloc(nBytes); /* IMP: R-04300-56712 */
  }
  सचे( nBytes==0 ){
    sqlite3_free(pOld); /* IMP: R-26507-47431 */
    पटिनिवत्त 0;
  }
  सचे( nBytes>SQLITE_MAX_ALLOCATION_SIZE ){
    पटिनिवत्त 0;
  }
  nOld = sqlite3MallocSize(pOld);
  /* IMPLEMENTATION-OF: R-46199-30249 SQLite guarantees that the second
  ** argument to xRealloc is always a value returned by a prior call to
  ** xRoundup. */
  nNew = sqlite3GlobalConfig.m.xRoundup((पूर्णाङ्क)nBytes);
  सचे( nOld==nNew ){
    pNew = pOld;
  }अञ्ञथा सचे( sqlite3GlobalConfig.bMemstat ){
    sqlite3_int64 nUsed;
    sqlite3_mutex_enter(mem0.mutex);
    sqlite3StatusHighwater(SQLITE_STATUS_MALLOC_SIZE, (पूर्णाङ्क)nBytes);
    nDiff = nNew - nOld;
    सचे( nDiff>0 && (nUsed = sqlite3_Status_मानम्(SQLITE_STATUS_MEMORY_USED)) >= 
          mem0.alarmThreshold-nDiff ){
      sqlite3MallocAlarm(nDiff);
      सचे( mem0.hardLimit>0 && nUsed >= mem0.hardLimit - nDiff ){
        sqlite3_mutex_leave(mem0.mutex);
        test_oom_breakpoint(1);
        पटिनिवत्त 0;
      }
    }
    pNew = sqlite3GlobalConfig.m.xRealloc(pOld, nNew);
#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
    सचे( pNew==0 && mem0.alarmThreshold>0 ){
      sqlite3MallocAlarm((पूर्णाङ्क)nBytes);
      pNew = sqlite3GlobalConfig.m.xRealloc(pOld, nNew);
    }
#endif
    सचे( pNew ){
      nNew = sqlite3MallocSize(pNew);
      sqlite3StatusUp(SQLITE_STATUS_MEMORY_USED, nNew-nOld);
    }
    sqlite3_mutex_leave(mem0.mutex);
  }अञ्ञथा{
    pNew = sqlite3GlobalConfig.m.xRealloc(pOld, nNew);
  }
  प्रतिज्ञा( EIGHT_BYTE_ALIGNMENT(pNew) ); /* IMP: R-11148-40995 */
  पटिनिवत्त pNew;
}

/*
** The public interface to sqlite3Realloc.  Make sure that the memory
** subsystem is initialized prior to invoking sqliteRealloc.
*/
शून्य *sqlite3_realloc(शून्य *pOld, पूर्णाङ्क n){
#ifndef SQLITE_OMIT_AUTOINIT
  सचे( sqlite3_initialize() ) पटिनिवत्त 0;
#endif
  सचे( n<0 ) n = 0;  /* IMP: R-26507-47431 */
  पटिनिवत्त sqlite3Realloc(pOld, n);
}
शून्य *sqlite3_realloc64(शून्य *pOld, sqlite3_uint64 n){
#ifndef SQLITE_OMIT_AUTOINIT
  सचे( sqlite3_initialize() ) पटिनिवत्त 0;
#endif
  पटिनिवत्त sqlite3Realloc(pOld, n);
}


/*
** Allocate and zero memory.
*/ 
शून्य *sqlite3MallocZero(u64 n){
  शून्य *p = sqlite3Malloc(n);
  सचे( p ){
    memset(p, 0, (size_t)n);
  }
  पटिनिवत्त p;
}

/*
** Allocate and zero memory.  If the allocation fails, make
** the mallocFailed flag in the connection pointer.
*/
शून्य *sqlite3DbMallocZero(sqlite3 *db, u64 n){
  शून्य *p;
  testcase( db==0 );
  p = sqlite3DbMallocRaw(db, n);
  सचे( p ) memset(p, 0, (size_t)n);
  पटिनिवत्त p;
}


/* Finish the work of sqlite3DbMallocRawNN for the unusual and
** slower case when the allocation cannot be fulfilled using lookaside.
*/
स्थिर SQLITE_NOINLINE शून्य *dbMallocRawFinish(sqlite3 *db, u64 n){
  शून्य *p;
  प्रतिज्ञा( db!=0 );
  p = sqlite3Malloc(n);
  सचे( !p ) sqlite3OomFault(db);
  sqlite3MemdebugSetType(p, 
         (db->lookaside.bDisable==0) ? MEMTYPE_LOOKASIDE : MEMTYPE_HEAP);
  पटिनिवत्त p;
}

/*
** Allocate memory, either lookaside (if possible) or heap.  
** If the allocation fails, set the mallocFailed flag in
** the connection pointer.
**
** If db!=0 and db->mallocFailed is true (indicating a prior malloc
** failure on the same database connection) then always return 0.
** Hence for a particular database connection, once malloc starts
** failing, it fails consistently until mallocFailed is reset.
** This is an important assumption.  There are many places in the
** code that do things like this:
**
**         int *a = (int*)sqlite3DbMallocRaw(db, 100);
**         int *b = (int*)sqlite3DbMallocRaw(db, 200);
**         if( b ) a[10] = 9;
**
** In other words, if a subsequent malloc (ex: "b") worked, it is assumed
** that all prior mallocs (ex: "a") worked too.
**
** The sqlite3MallocRawNN() variant guarantees that the "db" parameter is
** not a NULL pointer.
*/
शून्य *sqlite3DbMallocRaw(sqlite3 *db, u64 n){
  शून्य *p;
  सचे( db ) पटिनिवत्त sqlite3DbMallocRawNN(db, n);
  p = sqlite3Malloc(n);
  sqlite3MemdebugSetType(p, MEMTYPE_HEAP);
  पटिनिवत्त p;
}
शून्य *sqlite3DbMallocRawNN(sqlite3 *db, u64 n){
#ifndef SQLITE_OMIT_LOOKASIDE
  LookasideSlot *pBuf;
  प्रतिज्ञा( db!=0 );
  प्रतिज्ञा( sqlite3_mutex_held(db->mutex) );
  प्रतिज्ञा( db->pnBytesFreed==0 );
  सचे( n>db->lookaside.sz ){
    सचे( !db->lookaside.bDisable ){
      db->lookaside.anStat[1]++;      
    }अञ्ञथा सचे( db->mallocFailed ){
      पटिनिवत्त 0;
    }
    पटिनिवत्त dbMallocRawFinish(db, n);
  }
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
  सचे( n<=LOOKASIDE_SMALL ){
    सचे( (pBuf = db->lookaside.pSmallFree)!=0 ){
      db->lookaside.pSmallFree = pBuf->pNext;
      db->lookaside.anStat[0]++;
      पटिनिवत्त (शून्य*)pBuf;
    }अञ्ञथा सचे( (pBuf = db->lookaside.pSmallInit)!=0 ){
      db->lookaside.pSmallInit = pBuf->pNext;
      db->lookaside.anStat[0]++;
      पटिनिवत्त (शून्य*)pBuf;
    }
  }
#endif
  सचे( (pBuf = db->lookaside.pFree)!=0 ){
    db->lookaside.pFree = pBuf->pNext;
    db->lookaside.anStat[0]++;
    पटिनिवत्त (शून्य*)pBuf;
  }अञ्ञथा सचे( (pBuf = db->lookaside.pInit)!=0 ){
    db->lookaside.pInit = pBuf->pNext;
    db->lookaside.anStat[0]++;
    पटिनिवत्त (शून्य*)pBuf;
  }अञ्ञथा{
    db->lookaside.anStat[2]++;
  }
#अञ्ञथा
  प्रतिज्ञा( db!=0 );
  प्रतिज्ञा( sqlite3_mutex_held(db->mutex) );
  प्रतिज्ञा( db->pnBytesFreed==0 );
  सचे( db->mallocFailed ){
    पटिनिवत्त 0;
  }
#endif
  पटिनिवत्त dbMallocRawFinish(db, n);
}

/* Forward declaration */
स्थिर SQLITE_NOINLINE शून्य *dbReallocFinish(sqlite3 *db, शून्य *p, u64 n);

/*
** Resize the block of memory pointed to by p to n bytes. If the
** resize fails, set the mallocFailed flag in the connection object.
*/
शून्य *sqlite3DbRealloc(sqlite3 *db, शून्य *p, u64 n){
  प्रतिज्ञा( db!=0 );
  सचे( p==0 ) पटिनिवत्त sqlite3DbMallocRawNN(db, n);
  प्रतिज्ञा( sqlite3_mutex_held(db->mutex) );
  सचे( ((uptr)p)<(uptr)db->lookaside.pEnd ){
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
    सचे( ((uptr)p)>=(uptr)db->lookaside.p_मध्यः ){
      सचे( n<=LOOKASIDE_SMALL ) पटिनिवत्त p;
    }अञ्ञथा
#endif
    सचे( ((uptr)p)>=(uptr)db->lookaside.pStart ){
      सचे( n<=db->lookaside.szTrue ) पटिनिवत्त p;
    }
  }
  पटिनिवत्त dbReallocFinish(db, p, n);
}
स्थिर SQLITE_NOINLINE शून्य *dbReallocFinish(sqlite3 *db, शून्य *p, u64 n){
  शून्य *pNew = 0;
  प्रतिज्ञा( db!=0 );
  प्रतिज्ञा( p!=0 );
  सचे( db->mallocFailed==0 ){
    सचे( isLookaside(db, p) ){
      pNew = sqlite3DbMallocRawNN(db, n);
      सचे( pNew ){
        memcpy(pNew, p, lookasideMallocSize(db, p));
        sqlite3DbFree(db, p);
      }
    }अञ्ञथा{
      प्रतिज्ञा( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
      प्रतिज्ञा( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
      sqlite3MemdebugSetType(p, MEMTYPE_HEAP);
      pNew = sqlite3Realloc(p, n);
      सचे( !pNew ){
        sqlite3OomFault(db);
      }
      sqlite3MemdebugSetType(pNew,
            (db->lookaside.bDisable==0 ? MEMTYPE_LOOKASIDE : MEMTYPE_HEAP));
    }
  }
  पटिनिवत्त pNew;
}

/*
** Attempt to reallocate p.  If the reallocation fails, then free p
** and set the mallocFailed flag in the database connection.
*/
शून्य *sqlite3DbReallocOrFree(sqlite3 *db, शून्य *p, u64 n){
  शून्य *pNew;
  pNew = sqlite3DbRealloc(db, p, n);
  सचे( !pNew ){
    sqlite3DbFree(db, p);
  }
  पटिनिवत्त pNew;
}

/*
** Make a copy of a string in memory obtained from sqliteMalloc(). These 
** functions call sqlite3MallocRaw() directly instead of sqliteMalloc(). This
** is because when memory debugging is turned on, these two functions are 
** called via macros that record the current file and line number in the
** ThreadData structure.
*/
वर्ण *sqlite3DbStrDup(sqlite3 *db, ध्रुव वर्ण *z){
  वर्ण *zNew;
  size_t n;
  सचे( z==0 ){
    पटिनिवत्त 0;
  }
  n = strlen(z) + 1;
  zNew = sqlite3DbMallocRaw(db, n);
  सचे( zNew ){
    memcpy(zNew, z, n);
  }
  पटिनिवत्त zNew;
}
वर्ण *sqlite3DbStrNDup(sqlite3 *db, ध्रुव वर्ण *z, u64 n){
  वर्ण *zNew;
  प्रतिज्ञा( db!=0 );
  प्रतिज्ञा( z!=0 || n==0 );
  प्रतिज्ञा( (n&0x7fffffff)==n );
  zNew = z ? sqlite3DbMallocRawNN(db, n+1) : 0;
  सचे( zNew ){
    memcpy(zNew, z, (size_t)n);
    zNew[n] = 0;
  }
  पटिनिवत्त zNew;
}

/*
** The text between zStart and zEnd represents a phrase within a larger
** SQL statement.  Make a copy of this phrase in space obtained form
** sqlite3DbMalloc().  Omit leading and trailing whitespace.
*/
वर्ण *sqlite3DbSpanDup(sqlite3 *db, ध्रुव वर्ण *zStart, ध्रुव वर्ण *zEnd){
  पूर्णाङ्क n;
#ifdef SQLITE_DEBUG
  /* Because of the way the parser works, the span is guaranteed to contain
  ** at least one non-space character */
  अत्थाय(n=0; sqlite3Isspace(zStart[n]); n++){ प्रतिज्ञा( &zStart[n]<zEnd ); }
#endif
  याव( sqlite3Isspace(zStart[0]) ) zStart++;
  n = (पूर्णाङ्क)(zEnd - zStart);
  याव( sqlite3Isspace(zStart[n-1]) ) n--;
  पटिनिवत्त sqlite3DbStrNDup(db, zStart, n);
}

/*
** Free any prior content in *pz and replace it with a copy of zNew.
*/
शून्य sqlite3SetString(वर्ण **pz, sqlite3 *db, ध्रुव वर्ण *zNew){
  वर्ण *z = sqlite3DbStrDup(db, zNew);
  sqlite3DbFree(db, *pz);
  *pz = z;
}

/*
** Call this routine to record the fact that an OOM (out-of-memory) error
** has happened.  This routine will set db->mallocFailed, and also
** temporarily disable the lookaside memory allocator and interrupt
** any running VDBEs.
**
** Always return a NULL pointer so that this routine can be invoked using
**
**      return sqlite3OomFault(db);
**
** and thereby avoid unnecessary stack frame allocations for the overwhelmingly
** common case where no OOM occurs.
*/
शून्य *sqlite3OomFault(sqlite3 *db){
  सचे( db->mallocFailed==0 && db->bBenignMalloc==0 ){
    db->mallocFailed = 1;
    सचे( db->nVdbeExec>0 ){
      AtomicStore(&db->u1.isInterrupted, 1);
    }
    DisableLookaside;
    सचे( db->pParse ){
      Parse *pParse;
      sqlite3_Error_सन्देशः(db->pParse, "out of memory");
      db->pParse->rc = SQLITE_NOMEM_BKPT;
      अत्थाय(pParse=db->pParse->pOuterParse; pParse; pParse = pParse->pOuterParse){
        pParse->nErr++;
        pParse->rc = SQLITE_NOMEM;
      } 
    }
  }
  पटिनिवत्त 0;
}

/*
** This routine reactivates the memory allocator and clears the
** db->mallocFailed flag as necessary.
**
** The memory allocator is not restarted if there are running
** VDBEs.
*/
शून्य sqlite3OomClear(sqlite3 *db){
  सचे( db->mallocFailed && db->nVdbeExec==0 ){
    db->mallocFailed = 0;
    AtomicStore(&db->u1.isInterrupted, 0);
    प्रतिज्ञा( db->lookaside.bDisable>0 );
    EnableLookaside;
  }
}

/*
** Take actions at the end of an API call to deal with error codes.
*/
स्थिर SQLITE_NOINLINE पूर्णाङ्क apiHandleError(sqlite3 *db, पूर्णाङ्क rc){
  सचे( db->mallocFailed || rc==SQLITE_IOERR_NOMEM ){
    sqlite3OomClear(db);
    sqlite3Error(db, SQLITE_NOMEM);
    पटिनिवत्त SQLITE_NOMEM_BKPT;
  }
  पटिनिवत्त rc & db->errMask;
}

/*
** This function must be called before exiting any API function (i.e. 
** returning control to the user) that has called sqlite3_malloc or
** sqlite3_realloc.
**
** The returned value is normally a copy of the second argument to this
** function. However, if a malloc() failure has occurred since the previous
** invocation SQLITE_NOMEM is returned instead. 
**
** If an OOM as occurred, then the connection error-code (the value
** returned by sqlite3_errcode()) is set to SQLITE_NOMEM.
*/
पूर्णाङ्क sqlite3ApiExit(sqlite3* db, पूर्णाङ्क rc){
  /* If the db handle must hold the connection handle mutex here.
  ** Otherwise the read (and possible write) of db->mallocFailed 
  ** is unsafe, as is the call to sqlite3Error().
  */
  प्रतिज्ञा( db!=0 );
  प्रतिज्ञा( sqlite3_mutex_held(db->mutex) );
  सचे( db->mallocFailed || rc ){
    पटिनिवत्त apiHandleError(db, rc);
  }
  पटिनिवत्त 0;
}
