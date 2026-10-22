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
#অন্তর্ভাৱ "sqliteInt.h"
#অন্তর্ভাৱ <stdarg.h>

/*
** Attempt to release up to n bytes of non-essential memory currently
** held by SQLite. An example of non-essential memory is memory used to
** cache database pages that are not currently in use.
*/
পূর্ণাঙ্ক sqlite3_release_memory(পূর্ণাঙ্ক n){
#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
  ঘূৰাই sqlite3PcacheReleaseMemory(n);
#অন্যথা
  /* IMPLEMENTATION-OF: R-34391-24921 The sqlite3_release_memory() routine
  ** is a no-op returning zero if SQLite is not compiled with
  ** SQLITE_ENABLE_MEMORY_MANAGEMENT. */
  UNUSED_PARAMETER(n);
  ঘূৰাই 0;
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
স্থির SQLITE_WSD সংরচনা Mem0Global {
  sqlite3_mutex *mutex;         /* Mutex to serialize access */
  sqlite3_int64 alarmThreshold; /* The soft heap limit */
  sqlite3_int64 hardLimit;      /* The hard upper bound on memory */

  /*
  ** True if heap is nearly "full" where "full" is defined by the
  ** sqlite3_soft_heap_limit() setting.
  */
  পূর্ণাঙ্ক nearlyFull;
} mem0 = { 0, SQLITE_MAX_MEMORY, SQLITE_MAX_MEMORY, 0 };

#define mem0 GLOBAL(সংরচনা Mem0Global, mem0)

/*
** Return the memory allocator mutex. sqlite3_status() needs it.
*/
sqlite3_mutex *sqlite3MallocMutex(শূন্য){
  ঘূৰাই mem0.mutex;
}

#ifndef SQLITE_OMIT_DEPRECATED
/*
** Deprecated external interface.  It used to set an alarm callback
** that was invoked when memory usage grew too large.  Now it is a
** no-op.
*/
পূর্ণাঙ্ক sqlite3_memory_alarm(
  শূন্য(*xCallback)(শূন্য *pArg, sqlite3_int64 used,পূর্ণাঙ্ক N),
  শূন্য *pArg,
  sqlite3_int64 iThreshold
){
  (শূন্য)xCallback;
  (শূন্য)pArg;
  (শূন্য)iThreshold;
  ঘূৰাই SQLITE_OK;
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
  পূর্ণাঙ্ক rc = sqlite3_initialize();
  যদি( rc ) ঘূৰাই -1;
#endif
  sqlite3_mutex_enter(mem0.mutex);
  priorLimit = mem0.alarmThreshold;
  যদি( n<0 ){
    sqlite3_mutex_leave(mem0.mutex);
    ঘূৰাই priorLimit;
  }
  যদি( mem0.hardLimit>0 && (n>mem0.hardLimit || n==0) ){
    n = mem0.hardLimit;
  }
  mem0.alarmThreshold = n;
  nUsed = sqlite3_Status_মান(SQLITE_STATUS_MEMORY_USED);
  AtomicStore(&mem0.nearlyFull, n>0 && n<=nUsed);
  sqlite3_mutex_leave(mem0.mutex);
  excess = sqlite3_memory_used() - n;
  যদি( excess>0 ) sqlite3_release_memory((পূর্ণাঙ্ক)(excess & 0x7fffffff));
  ঘূৰাই priorLimit;
}
শূন্য sqlite3_soft_heap_limit(পূর্ণাঙ্ক n){
  যদি( n<0 ) n = 0;
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
  পূর্ণাঙ্ক rc = sqlite3_initialize();
  যদি( rc ) ঘূৰাই -1;
#endif
  sqlite3_mutex_enter(mem0.mutex);
  priorLimit = mem0.hardLimit;
  যদি( n>=0 ){
    mem0.hardLimit = n;
    যদি( n<mem0.alarmThreshold || mem0.alarmThreshold==0 ){
      mem0.alarmThreshold = n;
    }
  }
  sqlite3_mutex_leave(mem0.mutex);
  ঘূৰাই priorLimit;
}


/*
** Initialize the memory allocation subsystem.
*/
পূর্ণাঙ্ক sqlite3MallocInit(শূন্য){
  পূর্ণাঙ্ক rc;
  যদি( sqlite3GlobalConfig.m.xMalloc==0 ){
    sqlite3MemSetDefault();
  }
  mem0.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM);
  যদি( sqlite3GlobalConfig.pPage==0 || sqlite3GlobalConfig.szPage<512
      || sqlite3GlobalConfig.nPage<=0 ){
    sqlite3GlobalConfig.pPage = 0;
    sqlite3GlobalConfig.szPage = 0;
  }
  rc = sqlite3GlobalConfig.m.xInit(sqlite3GlobalConfig.m.pAppData);
  যদি( rc!=SQLITE_OK ) memset(&mem0, 0, পরিমাণ(mem0));
  ঘূৰাই rc;
}

/*
** Return true if the heap is currently under memory pressure - in other
** words if the amount of heap used is close to the limit set by
** sqlite3_soft_heap_limit().
*/
পূর্ণাঙ্ক sqlite3HeapNearlyFull(শূন্য){
  ঘূৰাই AtomicLoad(&mem0.nearlyFull);
}

/*
** Deinitialize the memory allocation subsystem.
*/
শূন্য sqlite3MallocEnd(শূন্য){
  যদি( sqlite3GlobalConfig.m.xShutdown ){
    sqlite3GlobalConfig.m.xShutdown(sqlite3GlobalConfig.m.pAppData);
  }
  memset(&mem0, 0, পরিমাণ(mem0));
}

/*
** Return the amount of memory currently checked out.
*/
sqlite3_int64 sqlite3_memory_used(শূন্য){
  sqlite3_int64 res, mx;
  sqlite3_status64(SQLITE_STATUS_MEMORY_USED, &res, &mx, 0);
  ঘূৰাই res;
}

/*
** Return the maximum amount of memory that has ever been
** checked out since either the beginning of this process
** or since the most recent reset.
*/
sqlite3_int64 sqlite3_memory_highwater(পূর্ণাঙ্ক resetFlag){
  sqlite3_int64 res, mx;
  sqlite3_status64(SQLITE_STATUS_MEMORY_USED, &res, &mx, resetFlag);
  ঘূৰাই mx;
}

/*
** Trigger the alarm 
*/
স্থির শূন্য sqlite3MallocAlarm(পূর্ণাঙ্ক nByte){
  যদি( mem0.alarmThreshold<=0 ) ঘূৰাই;
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
স্থির SQLITE_NOINLINE শূন্য test_oom_breakpoint(u64 n){
  স্থির u64 nOomFault = 0;
  nOomFault += n;
  /* The assert() is never reached in a human lifetime.  It  is here mostly
  ** to prevent code optimizers from optimizing out this function. */
  প্রতিজ্ঞা( (nOomFault>>32) < 0xffffffff );
}
#অন্যথা
# define test_oom_breakpoint(X)   /* No-op for production builds */
#endif

/*
** Do a memory allocation with statistics and alarms.  Assume the
** lock is already held.
*/
স্থির শূন্য mallocWithAlarm(পূর্ণাঙ্ক n, শূন্য **pp){
  শূন্য *p;
  পূর্ণাঙ্ক nFull;
  প্রতিজ্ঞা( sqlite3_mutex_held(mem0.mutex) );
  প্রতিজ্ঞা( n>0 );

  /* In Firefox (circa 2017-02-08), xRoundup() is remapped to an internal
  ** implementation of malloc_good_size(), which must be called in debug
  ** mode and specifically when the DMD "Dark Matter Detector" is enabled
  ** or else a crash results.  Hence, do not attempt to optimize out the
  ** following xRoundup() call. */
  nFull = sqlite3GlobalConfig.m.xRoundup(n);

  sqlite3StatusHighwater(SQLITE_STATUS_MALLOC_SIZE, n);
  যদি( mem0.alarmThreshold>0 ){
    sqlite3_int64 nUsed = sqlite3_Status_মান(SQLITE_STATUS_MEMORY_USED);
    যদি( nUsed >= mem0.alarmThreshold - nFull ){
      AtomicStore(&mem0.nearlyFull, 1);
      sqlite3MallocAlarm(nFull);
      যদি( mem0.hardLimit ){
        nUsed = sqlite3_Status_মান(SQLITE_STATUS_MEMORY_USED);
        যদি( nUsed >= mem0.hardLimit - nFull ){
          test_oom_breakpoint(1);
          *pp = 0;
          ঘূৰাই;
        }
      }
    }অন্যথা{
      AtomicStore(&mem0.nearlyFull, 0);
    }
  }
  p = sqlite3GlobalConfig.m.xMalloc(nFull);
#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
  যদি( p==0 && mem0.alarmThreshold>0 ){
    sqlite3MallocAlarm(nFull);
    p = sqlite3GlobalConfig.m.xMalloc(nFull);
  }
#endif
  যদি( p ){
    nFull = sqlite3MallocSize(p);
    sqlite3StatusUp(SQLITE_STATUS_MEMORY_USED, nFull);
    sqlite3StatusUp(SQLITE_STATUS_MALLOC_গুনতি, 1);
  }
  *pp = p;
}

/*
** Allocate memory.  This routine is like sqlite3_malloc() except that it
** assumes the memory subsystem has already been initialized.
*/
শূন্য *sqlite3Malloc(u64 n){
  শূন্য *p;
  যদি( n==0 || n>SQLITE_MAX_ALLOCATION_SIZE ){
    p = 0;
  }অন্যথা যদি( sqlite3GlobalConfig.bMemstat ){
    sqlite3_mutex_enter(mem0.mutex);
    mallocWithAlarm((পূর্ণাঙ্ক)n, &p);
    sqlite3_mutex_leave(mem0.mutex);
  }অন্যথা{
    p = sqlite3GlobalConfig.m.xMalloc((পূর্ণাঙ্ক)n);
  }
  প্রতিজ্ঞা( EIGHT_BYTE_ALIGNMENT(p) );  /* IMP: R-11148-40995 */
  ঘূৰাই p;
}

/*
** This version of the memory allocation is for use by the application.
** First make sure the memory subsystem is initialized, then do the
** allocation.
*/
শূন্য *sqlite3_malloc(পূর্ণাঙ্ক n){
#ifndef SQLITE_OMIT_AUTOINIT
  যদি( sqlite3_initialize() ) ঘূৰাই 0;
#endif
  ঘূৰাই n<=0 ? 0 : sqlite3Malloc(n);
}
শূন্য *sqlite3_malloc64(sqlite3_uint64 n){
#ifndef SQLITE_OMIT_AUTOINIT
  যদি( sqlite3_initialize() ) ঘূৰাই 0;
#endif
  ঘূৰাই sqlite3Malloc(n);
}

/*
** TRUE if p is a lookaside memory allocation from db
*/
#ifndef SQLITE_OMIT_LOOKASIDE
স্থির পূর্ণাঙ্ক isLookaside(sqlite3 *db, ধ্রুৱ শূন্য *p){
  ঘূৰাই SQLITE_WITHIN(p, db->lookaside.pStart, db->lookaside.pTrueEnd);
}
#অন্যথা
#define isLookaside(A,B) 0
#endif

/*
** Return the size of a memory allocation previously obtained from
** sqlite3Malloc() or sqlite3_malloc().
*/
পূর্ণাঙ্ক sqlite3MallocSize(ধ্রুৱ শূন্য *p){
  প্রতিজ্ঞা( sqlite3MemdebugHasType(p, MEMTYPE_HEAP) );
  ঘূৰাই sqlite3GlobalConfig.m.xSize((শূন্য*)p);
}
স্থির পূর্ণাঙ্ক lookasideMallocSize(sqlite3 *db, ধ্রুৱ শূন্য *p){
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE    
  ঘূৰাই p<db->lookaside.p_মধ্য ? db->lookaside.szTrue : LOOKASIDE_SMALL;
#অন্যথা
  ঘূৰাই db->lookaside.szTrue;
#endif  
}
পূর্ণাঙ্ক sqlite3DbMallocSize(sqlite3 *db, ধ্রুৱ শূন্য *p){
  প্রতিজ্ঞা( p!=0 );
#ifdef SQLITE_DEBUG
  যদি( db==0 ){
    প্রতিজ্ঞা( sqlite3MemdebugNoType(p, (u8)~MEMTYPE_HEAP) );
    প্রতিজ্ঞা( sqlite3MemdebugHasType(p, MEMTYPE_HEAP) );
  }অন্যথা যদি( !isLookaside(db,p) ){
    প্রতিজ্ঞা( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
    প্রতিজ্ঞা( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  }
#endif
  যদি( db ){
    যদি( ((uptr)p)<(uptr)(db->lookaside.pTrueEnd) ){
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
      যদি( ((uptr)p)>=(uptr)(db->lookaside.p_মধ্য) ){
        প্রতিজ্ঞা( sqlite3_mutex_held(db->mutex) );
        ঘূৰাই LOOKASIDE_SMALL;
      }
#endif
      যদি( ((uptr)p)>=(uptr)(db->lookaside.pStart) ){
        প্রতিজ্ঞা( sqlite3_mutex_held(db->mutex) );
        ঘূৰাই db->lookaside.szTrue;
      }
    }
  }
  ঘূৰাই sqlite3GlobalConfig.m.xSize((শূন্য*)p);
}
sqlite3_uint64 sqlite3_msize(শূন্য *p){
  প্রতিজ্ঞা( sqlite3MemdebugNoType(p, (u8)~MEMTYPE_HEAP) );
  প্রতিজ্ঞা( sqlite3MemdebugHasType(p, MEMTYPE_HEAP) );
  ঘূৰাই p ? sqlite3GlobalConfig.m.xSize(p) : 0;
}

/*
** Free memory previously obtained from sqlite3Malloc().
*/
শূন্য sqlite3_free(শূন্য *p){
  যদি( p==0 ) ঘূৰাই;  /* IMP: R-49053-54554 */
  প্রতিজ্ঞা( sqlite3MemdebugHasType(p, MEMTYPE_HEAP) );
  প্রতিজ্ঞা( sqlite3MemdebugNoType(p, (u8)~MEMTYPE_HEAP) );
  যদি( sqlite3GlobalConfig.bMemstat ){
    sqlite3_mutex_enter(mem0.mutex);
    sqlite3StatusDown(SQLITE_STATUS_MEMORY_USED, sqlite3MallocSize(p));
    sqlite3StatusDown(SQLITE_STATUS_MALLOC_গুনতি, 1);
    sqlite3GlobalConfig.m.xFree(p);
    sqlite3_mutex_leave(mem0.mutex);
  }অন্যথা{
    sqlite3GlobalConfig.m.xFree(p);
  }
}

/*
** Add the size of memory allocation "p" to the count in
** *db->pnBytesFreed.
*/
স্থির SQLITE_NOINLINE শূন্য measureAllocationSize(sqlite3 *db, শূন্য *p){
  *db->pnBytesFreed += sqlite3DbMallocSize(db,p);
}

/*
** Free memory that might be associated with a particular database
** connection.  Calling sqlite3DbFree(D,X) for X==0 is a harmless no-op.
** The sqlite3DbFreeNN(D,X) version requires that X be non-NULL.
*/
শূন্য sqlite3DbFreeNN(sqlite3 *db, শূন্য *p){
  প্রতিজ্ঞা( db==0 || sqlite3_mutex_held(db->mutex) );
  প্রতিজ্ঞা( p!=0 );
  যদি( db ){
    যদি( ((uptr)p)<(uptr)(db->lookaside.pEnd) ){
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
      যদি( ((uptr)p)>=(uptr)(db->lookaside.p_মধ্য) ){
        LookasideSlot *pBuf = (LookasideSlot*)p;
        প্রতিজ্ঞা( db->pnBytesFreed==0 );
#ifdef SQLITE_DEBUG
        memset(p, 0xaa, LOOKASIDE_SMALL);  /* Trash freed content */
#endif
        pBuf->pNext = db->lookaside.pSmallFree;
        db->lookaside.pSmallFree = pBuf;
        ঘূৰাই;
      }
#endif /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
      যদি( ((uptr)p)>=(uptr)(db->lookaside.pStart) ){
        LookasideSlot *pBuf = (LookasideSlot*)p;
        প্রতিজ্ঞা( db->pnBytesFreed==0 );
#ifdef SQLITE_DEBUG
        memset(p, 0xaa, db->lookaside.szTrue);  /* Trash freed content */
#endif
        pBuf->pNext = db->lookaside.pFree;
        db->lookaside.pFree = pBuf;
        ঘূৰাই;
      }
    }
    যদি( db->pnBytesFreed ){
      measureAllocationSize(db, p);
      ঘূৰাই;
    }
  }
  প্রতিজ্ঞা( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  প্রতিজ্ঞা( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  প্রতিজ্ঞা( db!=0 || sqlite3MemdebugNoType(p, MEMTYPE_LOOKASIDE) );
  sqlite3MemdebugSetType(p, MEMTYPE_HEAP);
  sqlite3_free(p);
}
শূন্য sqlite3DbNNFreeNN(sqlite3 *db, শূন্য *p){
  প্রতিজ্ঞা( db!=0 );
  প্রতিজ্ঞা( sqlite3_mutex_held(db->mutex) );
  প্রতিজ্ঞা( p!=0 );
  যদি( ((uptr)p)<(uptr)(db->lookaside.pEnd) ){
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
    যদি( ((uptr)p)>=(uptr)(db->lookaside.p_মধ্য) ){
      LookasideSlot *pBuf = (LookasideSlot*)p;
      প্রতিজ্ঞা( db->pnBytesFreed==0 );
#ifdef SQLITE_DEBUG
      memset(p, 0xaa, LOOKASIDE_SMALL);  /* Trash freed content */
#endif
      pBuf->pNext = db->lookaside.pSmallFree;
      db->lookaside.pSmallFree = pBuf;
      ঘূৰাই;
    }
#endif /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
    যদি( ((uptr)p)>=(uptr)(db->lookaside.pStart) ){
      LookasideSlot *pBuf = (LookasideSlot*)p;
      প্রতিজ্ঞা( db->pnBytesFreed==0 );
#ifdef SQLITE_DEBUG
      memset(p, 0xaa, db->lookaside.szTrue);  /* Trash freed content */
#endif
      pBuf->pNext = db->lookaside.pFree;
      db->lookaside.pFree = pBuf;
      ঘূৰাই;
    }
  }
  যদি( db->pnBytesFreed ){
    measureAllocationSize(db, p);
    ঘূৰাই;
  }
  প্রতিজ্ঞা( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  প্রতিজ্ঞা( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  sqlite3MemdebugSetType(p, MEMTYPE_HEAP);
  sqlite3_free(p);
}
শূন্য sqlite3DbFree(sqlite3 *db, শূন্য *p){
  প্রতিজ্ঞা( db==0 || sqlite3_mutex_held(db->mutex) );
  যদি( p ) sqlite3DbFreeNN(db, p);
}

/*
** Change the size of an existing memory allocation
*/
শূন্য *sqlite3Realloc(শূন্য *pOld, u64 nBytes){
  পূর্ণাঙ্ক nOld, nNew, nDiff;
  শূন্য *pNew;
  প্রতিজ্ঞা( sqlite3MemdebugHasType(pOld, MEMTYPE_HEAP) );
  প্রতিজ্ঞা( sqlite3MemdebugNoType(pOld, (u8)~MEMTYPE_HEAP) );
  যদি( pOld==0 ){
    ঘূৰাই sqlite3Malloc(nBytes); /* IMP: R-04300-56712 */
  }
  যদি( nBytes==0 ){
    sqlite3_free(pOld); /* IMP: R-26507-47431 */
    ঘূৰাই 0;
  }
  যদি( nBytes>SQLITE_MAX_ALLOCATION_SIZE ){
    ঘূৰাই 0;
  }
  nOld = sqlite3MallocSize(pOld);
  /* IMPLEMENTATION-OF: R-46199-30249 SQLite guarantees that the second
  ** argument to xRealloc is always a value returned by a prior call to
  ** xRoundup. */
  nNew = sqlite3GlobalConfig.m.xRoundup((পূর্ণাঙ্ক)nBytes);
  যদি( nOld==nNew ){
    pNew = pOld;
  }অন্যথা যদি( sqlite3GlobalConfig.bMemstat ){
    sqlite3_int64 nUsed;
    sqlite3_mutex_enter(mem0.mutex);
    sqlite3StatusHighwater(SQLITE_STATUS_MALLOC_SIZE, (পূর্ণাঙ্ক)nBytes);
    nDiff = nNew - nOld;
    যদি( nDiff>0 && (nUsed = sqlite3_Status_মান(SQLITE_STATUS_MEMORY_USED)) >= 
          mem0.alarmThreshold-nDiff ){
      sqlite3MallocAlarm(nDiff);
      যদি( mem0.hardLimit>0 && nUsed >= mem0.hardLimit - nDiff ){
        sqlite3_mutex_leave(mem0.mutex);
        test_oom_breakpoint(1);
        ঘূৰাই 0;
      }
    }
    pNew = sqlite3GlobalConfig.m.xRealloc(pOld, nNew);
#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
    যদি( pNew==0 && mem0.alarmThreshold>0 ){
      sqlite3MallocAlarm((পূর্ণাঙ্ক)nBytes);
      pNew = sqlite3GlobalConfig.m.xRealloc(pOld, nNew);
    }
#endif
    যদি( pNew ){
      nNew = sqlite3MallocSize(pNew);
      sqlite3StatusUp(SQLITE_STATUS_MEMORY_USED, nNew-nOld);
    }
    sqlite3_mutex_leave(mem0.mutex);
  }অন্যথা{
    pNew = sqlite3GlobalConfig.m.xRealloc(pOld, nNew);
  }
  প্রতিজ্ঞা( EIGHT_BYTE_ALIGNMENT(pNew) ); /* IMP: R-11148-40995 */
  ঘূৰাই pNew;
}

/*
** The public interface to sqlite3Realloc.  Make sure that the memory
** subsystem is initialized prior to invoking sqliteRealloc.
*/
শূন্য *sqlite3_realloc(শূন্য *pOld, পূর্ণাঙ্ক n){
#ifndef SQLITE_OMIT_AUTOINIT
  যদি( sqlite3_initialize() ) ঘূৰাই 0;
#endif
  যদি( n<0 ) n = 0;  /* IMP: R-26507-47431 */
  ঘূৰাই sqlite3Realloc(pOld, n);
}
শূন্য *sqlite3_realloc64(শূন্য *pOld, sqlite3_uint64 n){
#ifndef SQLITE_OMIT_AUTOINIT
  যদি( sqlite3_initialize() ) ঘূৰাই 0;
#endif
  ঘূৰাই sqlite3Realloc(pOld, n);
}


/*
** Allocate and zero memory.
*/ 
শূন্য *sqlite3MallocZero(u64 n){
  শূন্য *p = sqlite3Malloc(n);
  যদি( p ){
    memset(p, 0, (size_t)n);
  }
  ঘূৰাই p;
}

/*
** Allocate and zero memory.  If the allocation fails, make
** the mallocFailed flag in the connection pointer.
*/
শূন্য *sqlite3DbMallocZero(sqlite3 *db, u64 n){
  শূন্য *p;
  testcase( db==0 );
  p = sqlite3DbMallocRaw(db, n);
  যদি( p ) memset(p, 0, (size_t)n);
  ঘূৰাই p;
}


/* Finish the work of sqlite3DbMallocRawNN for the unusual and
** slower case when the allocation cannot be fulfilled using lookaside.
*/
স্থির SQLITE_NOINLINE শূন্য *dbMallocRawFinish(sqlite3 *db, u64 n){
  শূন্য *p;
  প্রতিজ্ঞা( db!=0 );
  p = sqlite3Malloc(n);
  যদি( !p ) sqlite3OomFault(db);
  sqlite3MemdebugSetType(p, 
         (db->lookaside.bDisable==0) ? MEMTYPE_LOOKASIDE : MEMTYPE_HEAP);
  ঘূৰাই p;
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
শূন্য *sqlite3DbMallocRaw(sqlite3 *db, u64 n){
  শূন্য *p;
  যদি( db ) ঘূৰাই sqlite3DbMallocRawNN(db, n);
  p = sqlite3Malloc(n);
  sqlite3MemdebugSetType(p, MEMTYPE_HEAP);
  ঘূৰাই p;
}
শূন্য *sqlite3DbMallocRawNN(sqlite3 *db, u64 n){
#ifndef SQLITE_OMIT_LOOKASIDE
  LookasideSlot *pBuf;
  প্রতিজ্ঞা( db!=0 );
  প্রতিজ্ঞা( sqlite3_mutex_held(db->mutex) );
  প্রতিজ্ঞা( db->pnBytesFreed==0 );
  যদি( n>db->lookaside.sz ){
    যদি( !db->lookaside.bDisable ){
      db->lookaside.anStat[1]++;      
    }অন্যথা যদি( db->mallocFailed ){
      ঘূৰাই 0;
    }
    ঘূৰাই dbMallocRawFinish(db, n);
  }
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
  যদি( n<=LOOKASIDE_SMALL ){
    যদি( (pBuf = db->lookaside.pSmallFree)!=0 ){
      db->lookaside.pSmallFree = pBuf->pNext;
      db->lookaside.anStat[0]++;
      ঘূৰাই (শূন্য*)pBuf;
    }অন্যথা যদি( (pBuf = db->lookaside.pSmallInit)!=0 ){
      db->lookaside.pSmallInit = pBuf->pNext;
      db->lookaside.anStat[0]++;
      ঘূৰাই (শূন্য*)pBuf;
    }
  }
#endif
  যদি( (pBuf = db->lookaside.pFree)!=0 ){
    db->lookaside.pFree = pBuf->pNext;
    db->lookaside.anStat[0]++;
    ঘূৰাই (শূন্য*)pBuf;
  }অন্যথা যদি( (pBuf = db->lookaside.pInit)!=0 ){
    db->lookaside.pInit = pBuf->pNext;
    db->lookaside.anStat[0]++;
    ঘূৰাই (শূন্য*)pBuf;
  }অন্যথা{
    db->lookaside.anStat[2]++;
  }
#অন্যথা
  প্রতিজ্ঞা( db!=0 );
  প্রতিজ্ঞা( sqlite3_mutex_held(db->mutex) );
  প্রতিজ্ঞা( db->pnBytesFreed==0 );
  যদি( db->mallocFailed ){
    ঘূৰাই 0;
  }
#endif
  ঘূৰাই dbMallocRawFinish(db, n);
}

/* Forward declaration */
স্থির SQLITE_NOINLINE শূন্য *dbReallocFinish(sqlite3 *db, শূন্য *p, u64 n);

/*
** Resize the block of memory pointed to by p to n bytes. If the
** resize fails, set the mallocFailed flag in the connection object.
*/
শূন্য *sqlite3DbRealloc(sqlite3 *db, শূন্য *p, u64 n){
  প্রতিজ্ঞা( db!=0 );
  যদি( p==0 ) ঘূৰাই sqlite3DbMallocRawNN(db, n);
  প্রতিজ্ঞা( sqlite3_mutex_held(db->mutex) );
  যদি( ((uptr)p)<(uptr)db->lookaside.pEnd ){
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
    যদি( ((uptr)p)>=(uptr)db->lookaside.p_মধ্য ){
      যদি( n<=LOOKASIDE_SMALL ) ঘূৰাই p;
    }অন্যথা
#endif
    যদি( ((uptr)p)>=(uptr)db->lookaside.pStart ){
      যদি( n<=db->lookaside.szTrue ) ঘূৰাই p;
    }
  }
  ঘূৰাই dbReallocFinish(db, p, n);
}
স্থির SQLITE_NOINLINE শূন্য *dbReallocFinish(sqlite3 *db, শূন্য *p, u64 n){
  শূন্য *pNew = 0;
  প্রতিজ্ঞা( db!=0 );
  প্রতিজ্ঞা( p!=0 );
  যদি( db->mallocFailed==0 ){
    যদি( isLookaside(db, p) ){
      pNew = sqlite3DbMallocRawNN(db, n);
      যদি( pNew ){
        memcpy(pNew, p, lookasideMallocSize(db, p));
        sqlite3DbFree(db, p);
      }
    }অন্যথা{
      প্রতিজ্ঞা( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
      প্রতিজ্ঞা( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
      sqlite3MemdebugSetType(p, MEMTYPE_HEAP);
      pNew = sqlite3Realloc(p, n);
      যদি( !pNew ){
        sqlite3OomFault(db);
      }
      sqlite3MemdebugSetType(pNew,
            (db->lookaside.bDisable==0 ? MEMTYPE_LOOKASIDE : MEMTYPE_HEAP));
    }
  }
  ঘূৰাই pNew;
}

/*
** Attempt to reallocate p.  If the reallocation fails, then free p
** and set the mallocFailed flag in the database connection.
*/
শূন্য *sqlite3DbReallocOrFree(sqlite3 *db, শূন্য *p, u64 n){
  শূন্য *pNew;
  pNew = sqlite3DbRealloc(db, p, n);
  যদি( !pNew ){
    sqlite3DbFree(db, p);
  }
  ঘূৰাই pNew;
}

/*
** Make a copy of a string in memory obtained from sqliteMalloc(). These 
** functions call sqlite3MallocRaw() directly instead of sqliteMalloc(). This
** is because when memory debugging is turned on, these two functions are 
** called via macros that record the current file and line number in the
** ThreadData structure.
*/
ৱর্ণ *sqlite3DbStrDup(sqlite3 *db, ধ্রুৱ ৱর্ণ *z){
  ৱর্ণ *zNew;
  size_t n;
  যদি( z==0 ){
    ঘূৰাই 0;
  }
  n = strlen(z) + 1;
  zNew = sqlite3DbMallocRaw(db, n);
  যদি( zNew ){
    memcpy(zNew, z, n);
  }
  ঘূৰাই zNew;
}
ৱর্ণ *sqlite3DbStrNDup(sqlite3 *db, ধ্রুৱ ৱর্ণ *z, u64 n){
  ৱর্ণ *zNew;
  প্রতিজ্ঞা( db!=0 );
  প্রতিজ্ঞা( z!=0 || n==0 );
  প্রতিজ্ঞা( (n&0x7fffffff)==n );
  zNew = z ? sqlite3DbMallocRawNN(db, n+1) : 0;
  যদি( zNew ){
    memcpy(zNew, z, (size_t)n);
    zNew[n] = 0;
  }
  ঘূৰাই zNew;
}

/*
** The text between zStart and zEnd represents a phrase within a larger
** SQL statement.  Make a copy of this phrase in space obtained form
** sqlite3DbMalloc().  Omit leading and trailing whitespace.
*/
ৱর্ণ *sqlite3DbSpanDup(sqlite3 *db, ধ্রুৱ ৱর্ণ *zStart, ধ্রুৱ ৱর্ণ *zEnd){
  পূর্ণাঙ্ক n;
#ifdef SQLITE_DEBUG
  /* Because of the way the parser works, the span is guaranteed to contain
  ** at least one non-space character */
  বাবে(n=0; sqlite3Isspace(zStart[n]); n++){ প্রতিজ্ঞা( &zStart[n]<zEnd ); }
#endif
  যেতিয়ালৈ( sqlite3Isspace(zStart[0]) ) zStart++;
  n = (পূর্ণাঙ্ক)(zEnd - zStart);
  যেতিয়ালৈ( sqlite3Isspace(zStart[n-1]) ) n--;
  ঘূৰাই sqlite3DbStrNDup(db, zStart, n);
}

/*
** Free any prior content in *pz and replace it with a copy of zNew.
*/
শূন্য sqlite3SetString(ৱর্ণ **pz, sqlite3 *db, ধ্রুৱ ৱর্ণ *zNew){
  ৱর্ণ *z = sqlite3DbStrDup(db, zNew);
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
শূন্য *sqlite3OomFault(sqlite3 *db){
  যদি( db->mallocFailed==0 && db->bBenignMalloc==0 ){
    db->mallocFailed = 1;
    যদি( db->nVdbeExec>0 ){
      AtomicStore(&db->u1.isInterrupted, 1);
    }
    DisableLookaside;
    যদি( db->pParse ){
      Parse *pParse;
      sqlite3_Error_বাৰ্তা(db->pParse, "out of memory");
      db->pParse->rc = SQLITE_NOMEM_BKPT;
      বাবে(pParse=db->pParse->pOuterParse; pParse; pParse = pParse->pOuterParse){
        pParse->nErr++;
        pParse->rc = SQLITE_NOMEM;
      } 
    }
  }
  ঘূৰাই 0;
}

/*
** This routine reactivates the memory allocator and clears the
** db->mallocFailed flag as necessary.
**
** The memory allocator is not restarted if there are running
** VDBEs.
*/
শূন্য sqlite3OomClear(sqlite3 *db){
  যদি( db->mallocFailed && db->nVdbeExec==0 ){
    db->mallocFailed = 0;
    AtomicStore(&db->u1.isInterrupted, 0);
    প্রতিজ্ঞা( db->lookaside.bDisable>0 );
    EnableLookaside;
  }
}

/*
** Take actions at the end of an API call to deal with error codes.
*/
স্থির SQLITE_NOINLINE পূর্ণাঙ্ক apiHandleError(sqlite3 *db, পূর্ণাঙ্ক rc){
  যদি( db->mallocFailed || rc==SQLITE_IOERR_NOMEM ){
    sqlite3OomClear(db);
    sqlite3Error(db, SQLITE_NOMEM);
    ঘূৰাই SQLITE_NOMEM_BKPT;
  }
  ঘূৰাই rc & db->errMask;
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
পূর্ণাঙ্ক sqlite3ApiExit(sqlite3* db, পূর্ণাঙ্ক rc){
  /* If the db handle must hold the connection handle mutex here.
  ** Otherwise the read (and possible write) of db->mallocFailed 
  ** is unsafe, as is the call to sqlite3Error().
  */
  প্রতিজ্ঞা( db!=0 );
  প্রতিজ্ঞা( sqlite3_mutex_held(db->mutex) );
  যদি( db->mallocFailed || rc ){
    ঘূৰাই apiHandleError(db, rc);
  }
  ঘূৰাই 0;
}
