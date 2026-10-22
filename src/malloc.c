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
#அந்தர்பாவ "sqliteInt.h"
#அந்தர்பாவ <stdarg.h>

/*
** Attempt to release up to n bytes of non-essential memory currently
** held by SQLite. An example of non-essential memory is memory used to
** cache database pages that are not currently in use.
*/
முழு sqlite3_release_memory(முழு n){
#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
  திரும்பு sqlite3PcacheReleaseMemory(n);
#இல்லையெனில்
  /* IMPLEMENTATION-OF: R-34391-24921 The sqlite3_release_memory() routine
  ** is a no-op returning zero if SQLite is not compiled with
  ** SQLITE_ENABLE_MEMORY_MANAGEMENT. */
  UNUSED_PARAMETER(n);
  திரும்பு 0;
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
நிலையான SQLITE_WSD கட்டமைப்பு Mem0Global {
  sqlite3_mutex *mutex;         /* Mutex to serialize access */
  sqlite3_int64 alarmThreshold; /* The soft heap limit */
  sqlite3_int64 hardLimit;      /* The hard upper bound on memory */

  /*
  ** True if heap is nearly "full" where "full" is defined by the
  ** sqlite3_soft_heap_limit() setting.
  */
  முழு nearlyFull;
} mem0 = { 0, SQLITE_MAX_MEMORY, SQLITE_MAX_MEMORY, 0 };

#define mem0 GLOBAL(கட்டமைப்பு Mem0Global, mem0)

/*
** Return the memory allocator mutex. sqlite3_status() needs it.
*/
sqlite3_mutex *sqlite3MallocMutex(வெறுமை){
  திரும்பு mem0.mutex;
}

#ifndef SQLITE_OMIT_DEPRECATED
/*
** Deprecated external interface.  It used to set an alarm callback
** that was invoked when memory usage grew too large.  Now it is a
** no-op.
*/
முழு sqlite3_memory_alarm(
  வெறுமை(*xCallback)(வெறுமை *pArg, sqlite3_int64 used,முழு N),
  வெறுமை *pArg,
  sqlite3_int64 iThreshold
){
  (வெறுமை)xCallback;
  (வெறுமை)pArg;
  (வெறுமை)iThreshold;
  திரும்பு SQLITE_OK;
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
  முழு rc = sqlite3_initialize();
  எனில்( rc ) திரும்பு -1;
#endif
  sqlite3_mutex_enter(mem0.mutex);
  priorLimit = mem0.alarmThreshold;
  எனில்( n<0 ){
    sqlite3_mutex_leave(mem0.mutex);
    திரும்பு priorLimit;
  }
  எனில்( mem0.hardLimit>0 && (n>mem0.hardLimit || n==0) ){
    n = mem0.hardLimit;
  }
  mem0.alarmThreshold = n;
  nUsed = sqlite3_Status_மதிப்பு(SQLITE_STATUS_MEMORY_USED);
  AtomicStore(&mem0.nearlyFull, n>0 && n<=nUsed);
  sqlite3_mutex_leave(mem0.mutex);
  excess = sqlite3_memory_used() - n;
  எனில்( excess>0 ) sqlite3_release_memory((முழு)(excess & 0x7fffffff));
  திரும்பு priorLimit;
}
வெறுமை sqlite3_soft_heap_limit(முழு n){
  எனில்( n<0 ) n = 0;
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
  முழு rc = sqlite3_initialize();
  எனில்( rc ) திரும்பு -1;
#endif
  sqlite3_mutex_enter(mem0.mutex);
  priorLimit = mem0.hardLimit;
  எனில்( n>=0 ){
    mem0.hardLimit = n;
    எனில்( n<mem0.alarmThreshold || mem0.alarmThreshold==0 ){
      mem0.alarmThreshold = n;
    }
  }
  sqlite3_mutex_leave(mem0.mutex);
  திரும்பு priorLimit;
}


/*
** Initialize the memory allocation subsystem.
*/
முழு sqlite3MallocInit(வெறுமை){
  முழு rc;
  எனில்( sqlite3GlobalConfig.m.xMalloc==0 ){
    sqlite3MemSetDefault();
  }
  mem0.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM);
  எனில்( sqlite3GlobalConfig.pPage==0 || sqlite3GlobalConfig.szPage<512
      || sqlite3GlobalConfig.nPage<=0 ){
    sqlite3GlobalConfig.pPage = 0;
    sqlite3GlobalConfig.szPage = 0;
  }
  rc = sqlite3GlobalConfig.m.xInit(sqlite3GlobalConfig.m.pAppData);
  எனில்( rc!=SQLITE_OK ) memset(&mem0, 0, அளவு(mem0));
  திரும்பு rc;
}

/*
** Return true if the heap is currently under memory pressure - in other
** words if the amount of heap used is close to the limit set by
** sqlite3_soft_heap_limit().
*/
முழு sqlite3HeapNearlyFull(வெறுமை){
  திரும்பு AtomicLoad(&mem0.nearlyFull);
}

/*
** Deinitialize the memory allocation subsystem.
*/
வெறுமை sqlite3MallocEnd(வெறுமை){
  எனில்( sqlite3GlobalConfig.m.xShutdown ){
    sqlite3GlobalConfig.m.xShutdown(sqlite3GlobalConfig.m.pAppData);
  }
  memset(&mem0, 0, அளவு(mem0));
}

/*
** Return the amount of memory currently checked out.
*/
sqlite3_int64 sqlite3_memory_used(வெறுமை){
  sqlite3_int64 res, mx;
  sqlite3_status64(SQLITE_STATUS_MEMORY_USED, &res, &mx, 0);
  திரும்பு res;
}

/*
** Return the maximum amount of memory that has ever been
** checked out since either the beginning of this process
** or since the most recent reset.
*/
sqlite3_int64 sqlite3_memory_highwater(முழு resetFlag){
  sqlite3_int64 res, mx;
  sqlite3_status64(SQLITE_STATUS_MEMORY_USED, &res, &mx, resetFlag);
  திரும்பு mx;
}

/*
** Trigger the alarm 
*/
நிலையான வெறுமை sqlite3MallocAlarm(முழு nByte){
  எனில்( mem0.alarmThreshold<=0 ) திரும்பு;
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
நிலையான SQLITE_NOINLINE வெறுமை test_oom_breakpoint(u64 n){
  நிலையான u64 nOomFault = 0;
  nOomFault += n;
  /* The assert() is never reached in a human lifetime.  It  is here mostly
  ** to prevent code optimizers from optimizing out this function. */
  ப்ரதிஜ்ஞா( (nOomFault>>32) < 0xffffffff );
}
#இல்லையெனில்
# define test_oom_breakpoint(X)   /* No-op for production builds */
#endif

/*
** Do a memory allocation with statistics and alarms.  Assume the
** lock is already held.
*/
நிலையான வெறுமை mallocWithAlarm(முழு n, வெறுமை **pp){
  வெறுமை *p;
  முழு nFull;
  ப்ரதிஜ்ஞா( sqlite3_mutex_held(mem0.mutex) );
  ப்ரதிஜ்ஞா( n>0 );

  /* In Firefox (circa 2017-02-08), xRoundup() is remapped to an internal
  ** implementation of malloc_good_size(), which must be called in debug
  ** mode and specifically when the DMD "Dark Matter Detector" is enabled
  ** or else a crash results.  Hence, do not attempt to optimize out the
  ** following xRoundup() call. */
  nFull = sqlite3GlobalConfig.m.xRoundup(n);

  sqlite3StatusHighwater(SQLITE_STATUS_MALLOC_SIZE, n);
  எனில்( mem0.alarmThreshold>0 ){
    sqlite3_int64 nUsed = sqlite3_Status_மதிப்பு(SQLITE_STATUS_MEMORY_USED);
    எனில்( nUsed >= mem0.alarmThreshold - nFull ){
      AtomicStore(&mem0.nearlyFull, 1);
      sqlite3MallocAlarm(nFull);
      எனில்( mem0.hardLimit ){
        nUsed = sqlite3_Status_மதிப்பு(SQLITE_STATUS_MEMORY_USED);
        எனில்( nUsed >= mem0.hardLimit - nFull ){
          test_oom_breakpoint(1);
          *pp = 0;
          திரும்பு;
        }
      }
    }இல்லையெனில்{
      AtomicStore(&mem0.nearlyFull, 0);
    }
  }
  p = sqlite3GlobalConfig.m.xMalloc(nFull);
#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
  எனில்( p==0 && mem0.alarmThreshold>0 ){
    sqlite3MallocAlarm(nFull);
    p = sqlite3GlobalConfig.m.xMalloc(nFull);
  }
#endif
  எனில்( p ){
    nFull = sqlite3MallocSize(p);
    sqlite3StatusUp(SQLITE_STATUS_MEMORY_USED, nFull);
    sqlite3StatusUp(SQLITE_STATUS_MALLOC_மொத்தம், 1);
  }
  *pp = p;
}

/*
** Allocate memory.  This routine is like sqlite3_malloc() except that it
** assumes the memory subsystem has already been initialized.
*/
வெறுமை *sqlite3Malloc(u64 n){
  வெறுமை *p;
  எனில்( n==0 || n>SQLITE_MAX_ALLOCATION_SIZE ){
    p = 0;
  }இல்லையெனில் எனில்( sqlite3GlobalConfig.bMemstat ){
    sqlite3_mutex_enter(mem0.mutex);
    mallocWithAlarm((முழு)n, &p);
    sqlite3_mutex_leave(mem0.mutex);
  }இல்லையெனில்{
    p = sqlite3GlobalConfig.m.xMalloc((முழு)n);
  }
  ப்ரதிஜ்ஞா( EIGHT_BYTE_ALIGNMENT(p) );  /* IMP: R-11148-40995 */
  திரும்பு p;
}

/*
** This version of the memory allocation is for use by the application.
** First make sure the memory subsystem is initialized, then do the
** allocation.
*/
வெறுமை *sqlite3_malloc(முழு n){
#ifndef SQLITE_OMIT_AUTOINIT
  எனில்( sqlite3_initialize() ) திரும்பு 0;
#endif
  திரும்பு n<=0 ? 0 : sqlite3Malloc(n);
}
வெறுமை *sqlite3_malloc64(sqlite3_uint64 n){
#ifndef SQLITE_OMIT_AUTOINIT
  எனில்( sqlite3_initialize() ) திரும்பு 0;
#endif
  திரும்பு sqlite3Malloc(n);
}

/*
** TRUE if p is a lookaside memory allocation from db
*/
#ifndef SQLITE_OMIT_LOOKASIDE
நிலையான முழு isLookaside(sqlite3 *db, மாறிலி வெறுமை *p){
  திரும்பு SQLITE_WITHIN(p, db->lookaside.pStart, db->lookaside.pTrueEnd);
}
#இல்லையெனில்
#define isLookaside(A,B) 0
#endif

/*
** Return the size of a memory allocation previously obtained from
** sqlite3Malloc() or sqlite3_malloc().
*/
முழு sqlite3MallocSize(மாறிலி வெறுமை *p){
  ப்ரதிஜ்ஞா( sqlite3MemdebugHasType(p, MEMTYPE_HEAP) );
  திரும்பு sqlite3GlobalConfig.m.xSize((வெறுமை*)p);
}
நிலையான முழு lookasideMallocSize(sqlite3 *db, மாறிலி வெறுமை *p){
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE    
  திரும்பு p<db->lookaside.p_நடு ? db->lookaside.szTrue : LOOKASIDE_SMALL;
#இல்லையெனில்
  திரும்பு db->lookaside.szTrue;
#endif  
}
முழு sqlite3DbMallocSize(sqlite3 *db, மாறிலி வெறுமை *p){
  ப்ரதிஜ்ஞா( p!=0 );
#ifdef SQLITE_DEBUG
  எனில்( db==0 ){
    ப்ரதிஜ்ஞா( sqlite3MemdebugNoType(p, (u8)~MEMTYPE_HEAP) );
    ப்ரதிஜ்ஞா( sqlite3MemdebugHasType(p, MEMTYPE_HEAP) );
  }இல்லையெனில் எனில்( !isLookaside(db,p) ){
    ப்ரதிஜ்ஞா( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
    ப்ரதிஜ்ஞா( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  }
#endif
  எனில்( db ){
    எனில்( ((uptr)p)<(uptr)(db->lookaside.pTrueEnd) ){
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
      எனில்( ((uptr)p)>=(uptr)(db->lookaside.p_நடு) ){
        ப்ரதிஜ்ஞா( sqlite3_mutex_held(db->mutex) );
        திரும்பு LOOKASIDE_SMALL;
      }
#endif
      எனில்( ((uptr)p)>=(uptr)(db->lookaside.pStart) ){
        ப்ரதிஜ்ஞா( sqlite3_mutex_held(db->mutex) );
        திரும்பு db->lookaside.szTrue;
      }
    }
  }
  திரும்பு sqlite3GlobalConfig.m.xSize((வெறுமை*)p);
}
sqlite3_uint64 sqlite3_msize(வெறுமை *p){
  ப்ரதிஜ்ஞா( sqlite3MemdebugNoType(p, (u8)~MEMTYPE_HEAP) );
  ப்ரதிஜ்ஞா( sqlite3MemdebugHasType(p, MEMTYPE_HEAP) );
  திரும்பு p ? sqlite3GlobalConfig.m.xSize(p) : 0;
}

/*
** Free memory previously obtained from sqlite3Malloc().
*/
வெறுமை sqlite3_free(வெறுமை *p){
  எனில்( p==0 ) திரும்பு;  /* IMP: R-49053-54554 */
  ப்ரதிஜ்ஞா( sqlite3MemdebugHasType(p, MEMTYPE_HEAP) );
  ப்ரதிஜ்ஞா( sqlite3MemdebugNoType(p, (u8)~MEMTYPE_HEAP) );
  எனில்( sqlite3GlobalConfig.bMemstat ){
    sqlite3_mutex_enter(mem0.mutex);
    sqlite3StatusDown(SQLITE_STATUS_MEMORY_USED, sqlite3MallocSize(p));
    sqlite3StatusDown(SQLITE_STATUS_MALLOC_மொத்தம், 1);
    sqlite3GlobalConfig.m.xFree(p);
    sqlite3_mutex_leave(mem0.mutex);
  }இல்லையெனில்{
    sqlite3GlobalConfig.m.xFree(p);
  }
}

/*
** Add the size of memory allocation "p" to the count in
** *db->pnBytesFreed.
*/
நிலையான SQLITE_NOINLINE வெறுமை measureAllocationSize(sqlite3 *db, வெறுமை *p){
  *db->pnBytesFreed += sqlite3DbMallocSize(db,p);
}

/*
** Free memory that might be associated with a particular database
** connection.  Calling sqlite3DbFree(D,X) for X==0 is a harmless no-op.
** The sqlite3DbFreeNN(D,X) version requires that X be non-NULL.
*/
வெறுமை sqlite3DbFreeNN(sqlite3 *db, வெறுமை *p){
  ப்ரதிஜ்ஞா( db==0 || sqlite3_mutex_held(db->mutex) );
  ப்ரதிஜ்ஞா( p!=0 );
  எனில்( db ){
    எனில்( ((uptr)p)<(uptr)(db->lookaside.pEnd) ){
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
      எனில்( ((uptr)p)>=(uptr)(db->lookaside.p_நடு) ){
        LookasideSlot *pBuf = (LookasideSlot*)p;
        ப்ரதிஜ்ஞா( db->pnBytesFreed==0 );
#ifdef SQLITE_DEBUG
        memset(p, 0xaa, LOOKASIDE_SMALL);  /* Trash freed content */
#endif
        pBuf->pNext = db->lookaside.pSmallFree;
        db->lookaside.pSmallFree = pBuf;
        திரும்பு;
      }
#endif /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
      எனில்( ((uptr)p)>=(uptr)(db->lookaside.pStart) ){
        LookasideSlot *pBuf = (LookasideSlot*)p;
        ப்ரதிஜ்ஞா( db->pnBytesFreed==0 );
#ifdef SQLITE_DEBUG
        memset(p, 0xaa, db->lookaside.szTrue);  /* Trash freed content */
#endif
        pBuf->pNext = db->lookaside.pFree;
        db->lookaside.pFree = pBuf;
        திரும்பு;
      }
    }
    எனில்( db->pnBytesFreed ){
      measureAllocationSize(db, p);
      திரும்பு;
    }
  }
  ப்ரதிஜ்ஞா( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  ப்ரதிஜ்ஞா( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  ப்ரதிஜ்ஞா( db!=0 || sqlite3MemdebugNoType(p, MEMTYPE_LOOKASIDE) );
  sqlite3MemdebugSetType(p, MEMTYPE_HEAP);
  sqlite3_free(p);
}
வெறுமை sqlite3DbNNFreeNN(sqlite3 *db, வெறுமை *p){
  ப்ரதிஜ்ஞா( db!=0 );
  ப்ரதிஜ்ஞா( sqlite3_mutex_held(db->mutex) );
  ப்ரதிஜ்ஞா( p!=0 );
  எனில்( ((uptr)p)<(uptr)(db->lookaside.pEnd) ){
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
    எனில்( ((uptr)p)>=(uptr)(db->lookaside.p_நடு) ){
      LookasideSlot *pBuf = (LookasideSlot*)p;
      ப்ரதிஜ்ஞா( db->pnBytesFreed==0 );
#ifdef SQLITE_DEBUG
      memset(p, 0xaa, LOOKASIDE_SMALL);  /* Trash freed content */
#endif
      pBuf->pNext = db->lookaside.pSmallFree;
      db->lookaside.pSmallFree = pBuf;
      திரும்பு;
    }
#endif /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
    எனில்( ((uptr)p)>=(uptr)(db->lookaside.pStart) ){
      LookasideSlot *pBuf = (LookasideSlot*)p;
      ப்ரதிஜ்ஞா( db->pnBytesFreed==0 );
#ifdef SQLITE_DEBUG
      memset(p, 0xaa, db->lookaside.szTrue);  /* Trash freed content */
#endif
      pBuf->pNext = db->lookaside.pFree;
      db->lookaside.pFree = pBuf;
      திரும்பு;
    }
  }
  எனில்( db->pnBytesFreed ){
    measureAllocationSize(db, p);
    திரும்பு;
  }
  ப்ரதிஜ்ஞா( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  ப்ரதிஜ்ஞா( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  sqlite3MemdebugSetType(p, MEMTYPE_HEAP);
  sqlite3_free(p);
}
வெறுமை sqlite3DbFree(sqlite3 *db, வெறுமை *p){
  ப்ரதிஜ்ஞா( db==0 || sqlite3_mutex_held(db->mutex) );
  எனில்( p ) sqlite3DbFreeNN(db, p);
}

/*
** Change the size of an existing memory allocation
*/
வெறுமை *sqlite3Realloc(வெறுமை *pOld, u64 nBytes){
  முழு nOld, nNew, nDiff;
  வெறுமை *pNew;
  ப்ரதிஜ்ஞா( sqlite3MemdebugHasType(pOld, MEMTYPE_HEAP) );
  ப்ரதிஜ்ஞா( sqlite3MemdebugNoType(pOld, (u8)~MEMTYPE_HEAP) );
  எனில்( pOld==0 ){
    திரும்பு sqlite3Malloc(nBytes); /* IMP: R-04300-56712 */
  }
  எனில்( nBytes==0 ){
    sqlite3_free(pOld); /* IMP: R-26507-47431 */
    திரும்பு 0;
  }
  எனில்( nBytes>SQLITE_MAX_ALLOCATION_SIZE ){
    திரும்பு 0;
  }
  nOld = sqlite3MallocSize(pOld);
  /* IMPLEMENTATION-OF: R-46199-30249 SQLite guarantees that the second
  ** argument to xRealloc is always a value returned by a prior call to
  ** xRoundup. */
  nNew = sqlite3GlobalConfig.m.xRoundup((முழு)nBytes);
  எனில்( nOld==nNew ){
    pNew = pOld;
  }இல்லையெனில் எனில்( sqlite3GlobalConfig.bMemstat ){
    sqlite3_int64 nUsed;
    sqlite3_mutex_enter(mem0.mutex);
    sqlite3StatusHighwater(SQLITE_STATUS_MALLOC_SIZE, (முழு)nBytes);
    nDiff = nNew - nOld;
    எனில்( nDiff>0 && (nUsed = sqlite3_Status_மதிப்பு(SQLITE_STATUS_MEMORY_USED)) >= 
          mem0.alarmThreshold-nDiff ){
      sqlite3MallocAlarm(nDiff);
      எனில்( mem0.hardLimit>0 && nUsed >= mem0.hardLimit - nDiff ){
        sqlite3_mutex_leave(mem0.mutex);
        test_oom_breakpoint(1);
        திரும்பு 0;
      }
    }
    pNew = sqlite3GlobalConfig.m.xRealloc(pOld, nNew);
#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
    எனில்( pNew==0 && mem0.alarmThreshold>0 ){
      sqlite3MallocAlarm((முழு)nBytes);
      pNew = sqlite3GlobalConfig.m.xRealloc(pOld, nNew);
    }
#endif
    எனில்( pNew ){
      nNew = sqlite3MallocSize(pNew);
      sqlite3StatusUp(SQLITE_STATUS_MEMORY_USED, nNew-nOld);
    }
    sqlite3_mutex_leave(mem0.mutex);
  }இல்லையெனில்{
    pNew = sqlite3GlobalConfig.m.xRealloc(pOld, nNew);
  }
  ப்ரதிஜ்ஞா( EIGHT_BYTE_ALIGNMENT(pNew) ); /* IMP: R-11148-40995 */
  திரும்பு pNew;
}

/*
** The public interface to sqlite3Realloc.  Make sure that the memory
** subsystem is initialized prior to invoking sqliteRealloc.
*/
வெறுமை *sqlite3_realloc(வெறுமை *pOld, முழு n){
#ifndef SQLITE_OMIT_AUTOINIT
  எனில்( sqlite3_initialize() ) திரும்பு 0;
#endif
  எனில்( n<0 ) n = 0;  /* IMP: R-26507-47431 */
  திரும்பு sqlite3Realloc(pOld, n);
}
வெறுமை *sqlite3_realloc64(வெறுமை *pOld, sqlite3_uint64 n){
#ifndef SQLITE_OMIT_AUTOINIT
  எனில்( sqlite3_initialize() ) திரும்பு 0;
#endif
  திரும்பு sqlite3Realloc(pOld, n);
}


/*
** Allocate and zero memory.
*/ 
வெறுமை *sqlite3MallocZero(u64 n){
  வெறுமை *p = sqlite3Malloc(n);
  எனில்( p ){
    memset(p, 0, (size_t)n);
  }
  திரும்பு p;
}

/*
** Allocate and zero memory.  If the allocation fails, make
** the mallocFailed flag in the connection pointer.
*/
வெறுமை *sqlite3DbMallocZero(sqlite3 *db, u64 n){
  வெறுமை *p;
  testcase( db==0 );
  p = sqlite3DbMallocRaw(db, n);
  எனில்( p ) memset(p, 0, (size_t)n);
  திரும்பு p;
}


/* Finish the work of sqlite3DbMallocRawNN for the unusual and
** slower case when the allocation cannot be fulfilled using lookaside.
*/
நிலையான SQLITE_NOINLINE வெறுமை *dbMallocRawFinish(sqlite3 *db, u64 n){
  வெறுமை *p;
  ப்ரதிஜ்ஞா( db!=0 );
  p = sqlite3Malloc(n);
  எனில்( !p ) sqlite3OomFault(db);
  sqlite3MemdebugSetType(p, 
         (db->lookaside.bDisable==0) ? MEMTYPE_LOOKASIDE : MEMTYPE_HEAP);
  திரும்பு p;
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
வெறுமை *sqlite3DbMallocRaw(sqlite3 *db, u64 n){
  வெறுமை *p;
  எனில்( db ) திரும்பு sqlite3DbMallocRawNN(db, n);
  p = sqlite3Malloc(n);
  sqlite3MemdebugSetType(p, MEMTYPE_HEAP);
  திரும்பு p;
}
வெறுமை *sqlite3DbMallocRawNN(sqlite3 *db, u64 n){
#ifndef SQLITE_OMIT_LOOKASIDE
  LookasideSlot *pBuf;
  ப்ரதிஜ்ஞா( db!=0 );
  ப்ரதிஜ்ஞா( sqlite3_mutex_held(db->mutex) );
  ப்ரதிஜ்ஞா( db->pnBytesFreed==0 );
  எனில்( n>db->lookaside.sz ){
    எனில்( !db->lookaside.bDisable ){
      db->lookaside.anStat[1]++;      
    }இல்லையெனில் எனில்( db->mallocFailed ){
      திரும்பு 0;
    }
    திரும்பு dbMallocRawFinish(db, n);
  }
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
  எனில்( n<=LOOKASIDE_SMALL ){
    எனில்( (pBuf = db->lookaside.pSmallFree)!=0 ){
      db->lookaside.pSmallFree = pBuf->pNext;
      db->lookaside.anStat[0]++;
      திரும்பு (வெறுமை*)pBuf;
    }இல்லையெனில் எனில்( (pBuf = db->lookaside.pSmallInit)!=0 ){
      db->lookaside.pSmallInit = pBuf->pNext;
      db->lookaside.anStat[0]++;
      திரும்பு (வெறுமை*)pBuf;
    }
  }
#endif
  எனில்( (pBuf = db->lookaside.pFree)!=0 ){
    db->lookaside.pFree = pBuf->pNext;
    db->lookaside.anStat[0]++;
    திரும்பு (வெறுமை*)pBuf;
  }இல்லையெனில் எனில்( (pBuf = db->lookaside.pInit)!=0 ){
    db->lookaside.pInit = pBuf->pNext;
    db->lookaside.anStat[0]++;
    திரும்பு (வெறுமை*)pBuf;
  }இல்லையெனில்{
    db->lookaside.anStat[2]++;
  }
#இல்லையெனில்
  ப்ரதிஜ்ஞா( db!=0 );
  ப்ரதிஜ்ஞா( sqlite3_mutex_held(db->mutex) );
  ப்ரதிஜ்ஞா( db->pnBytesFreed==0 );
  எனில்( db->mallocFailed ){
    திரும்பு 0;
  }
#endif
  திரும்பு dbMallocRawFinish(db, n);
}

/* Forward declaration */
நிலையான SQLITE_NOINLINE வெறுமை *dbReallocFinish(sqlite3 *db, வெறுமை *p, u64 n);

/*
** Resize the block of memory pointed to by p to n bytes. If the
** resize fails, set the mallocFailed flag in the connection object.
*/
வெறுமை *sqlite3DbRealloc(sqlite3 *db, வெறுமை *p, u64 n){
  ப்ரதிஜ்ஞா( db!=0 );
  எனில்( p==0 ) திரும்பு sqlite3DbMallocRawNN(db, n);
  ப்ரதிஜ்ஞா( sqlite3_mutex_held(db->mutex) );
  எனில்( ((uptr)p)<(uptr)db->lookaside.pEnd ){
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
    எனில்( ((uptr)p)>=(uptr)db->lookaside.p_நடு ){
      எனில்( n<=LOOKASIDE_SMALL ) திரும்பு p;
    }இல்லையெனில்
#endif
    எனில்( ((uptr)p)>=(uptr)db->lookaside.pStart ){
      எனில்( n<=db->lookaside.szTrue ) திரும்பு p;
    }
  }
  திரும்பு dbReallocFinish(db, p, n);
}
நிலையான SQLITE_NOINLINE வெறுமை *dbReallocFinish(sqlite3 *db, வெறுமை *p, u64 n){
  வெறுமை *pNew = 0;
  ப்ரதிஜ்ஞா( db!=0 );
  ப்ரதிஜ்ஞா( p!=0 );
  எனில்( db->mallocFailed==0 ){
    எனில்( isLookaside(db, p) ){
      pNew = sqlite3DbMallocRawNN(db, n);
      எனில்( pNew ){
        memcpy(pNew, p, lookasideMallocSize(db, p));
        sqlite3DbFree(db, p);
      }
    }இல்லையெனில்{
      ப்ரதிஜ்ஞா( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
      ப்ரதிஜ்ஞா( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
      sqlite3MemdebugSetType(p, MEMTYPE_HEAP);
      pNew = sqlite3Realloc(p, n);
      எனில்( !pNew ){
        sqlite3OomFault(db);
      }
      sqlite3MemdebugSetType(pNew,
            (db->lookaside.bDisable==0 ? MEMTYPE_LOOKASIDE : MEMTYPE_HEAP));
    }
  }
  திரும்பு pNew;
}

/*
** Attempt to reallocate p.  If the reallocation fails, then free p
** and set the mallocFailed flag in the database connection.
*/
வெறுமை *sqlite3DbReallocOrFree(sqlite3 *db, வெறுமை *p, u64 n){
  வெறுமை *pNew;
  pNew = sqlite3DbRealloc(db, p, n);
  எனில்( !pNew ){
    sqlite3DbFree(db, p);
  }
  திரும்பு pNew;
}

/*
** Make a copy of a string in memory obtained from sqliteMalloc(). These 
** functions call sqlite3MallocRaw() directly instead of sqliteMalloc(). This
** is because when memory debugging is turned on, these two functions are 
** called via macros that record the current file and line number in the
** ThreadData structure.
*/
எழுத்து *sqlite3DbStrDup(sqlite3 *db, மாறிலி எழுத்து *z){
  எழுத்து *zNew;
  size_t n;
  எனில்( z==0 ){
    திரும்பு 0;
  }
  n = strlen(z) + 1;
  zNew = sqlite3DbMallocRaw(db, n);
  எனில்( zNew ){
    memcpy(zNew, z, n);
  }
  திரும்பு zNew;
}
எழுத்து *sqlite3DbStrNDup(sqlite3 *db, மாறிலி எழுத்து *z, u64 n){
  எழுத்து *zNew;
  ப்ரதிஜ்ஞா( db!=0 );
  ப்ரதிஜ்ஞா( z!=0 || n==0 );
  ப்ரதிஜ்ஞா( (n&0x7fffffff)==n );
  zNew = z ? sqlite3DbMallocRawNN(db, n+1) : 0;
  எனில்( zNew ){
    memcpy(zNew, z, (size_t)n);
    zNew[n] = 0;
  }
  திரும்பு zNew;
}

/*
** The text between zStart and zEnd represents a phrase within a larger
** SQL statement.  Make a copy of this phrase in space obtained form
** sqlite3DbMalloc().  Omit leading and trailing whitespace.
*/
எழுத்து *sqlite3DbSpanDup(sqlite3 *db, மாறிலி எழுத்து *zStart, மாறிலி எழுத்து *zEnd){
  முழு n;
#ifdef SQLITE_DEBUG
  /* Because of the way the parser works, the span is guaranteed to contain
  ** at least one non-space character */
  க்கு(n=0; sqlite3Isspace(zStart[n]); n++){ ப்ரதிஜ்ஞா( &zStart[n]<zEnd ); }
#endif
  போது( sqlite3Isspace(zStart[0]) ) zStart++;
  n = (முழு)(zEnd - zStart);
  போது( sqlite3Isspace(zStart[n-1]) ) n--;
  திரும்பு sqlite3DbStrNDup(db, zStart, n);
}

/*
** Free any prior content in *pz and replace it with a copy of zNew.
*/
வெறுமை sqlite3SetString(எழுத்து **pz, sqlite3 *db, மாறிலி எழுத்து *zNew){
  எழுத்து *z = sqlite3DbStrDup(db, zNew);
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
வெறுமை *sqlite3OomFault(sqlite3 *db){
  எனில்( db->mallocFailed==0 && db->bBenignMalloc==0 ){
    db->mallocFailed = 1;
    எனில்( db->nVdbeExec>0 ){
      AtomicStore(&db->u1.isInterrupted, 1);
    }
    DisableLookaside;
    எனில்( db->pParse ){
      Parse *pParse;
      sqlite3_Error_செய்தி(db->pParse, "out of memory");
      db->pParse->rc = SQLITE_NOMEM_BKPT;
      க்கு(pParse=db->pParse->pOuterParse; pParse; pParse = pParse->pOuterParse){
        pParse->nErr++;
        pParse->rc = SQLITE_NOMEM;
      } 
    }
  }
  திரும்பு 0;
}

/*
** This routine reactivates the memory allocator and clears the
** db->mallocFailed flag as necessary.
**
** The memory allocator is not restarted if there are running
** VDBEs.
*/
வெறுமை sqlite3OomClear(sqlite3 *db){
  எனில்( db->mallocFailed && db->nVdbeExec==0 ){
    db->mallocFailed = 0;
    AtomicStore(&db->u1.isInterrupted, 0);
    ப்ரதிஜ்ஞா( db->lookaside.bDisable>0 );
    EnableLookaside;
  }
}

/*
** Take actions at the end of an API call to deal with error codes.
*/
நிலையான SQLITE_NOINLINE முழு apiHandleError(sqlite3 *db, முழு rc){
  எனில்( db->mallocFailed || rc==SQLITE_IOERR_NOMEM ){
    sqlite3OomClear(db);
    sqlite3Error(db, SQLITE_NOMEM);
    திரும்பு SQLITE_NOMEM_BKPT;
  }
  திரும்பு rc & db->errMask;
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
முழு sqlite3ApiExit(sqlite3* db, முழு rc){
  /* If the db handle must hold the connection handle mutex here.
  ** Otherwise the read (and possible write) of db->mallocFailed 
  ** is unsafe, as is the call to sqlite3Error().
  */
  ப்ரதிஜ்ஞா( db!=0 );
  ப்ரதிஜ்ஞா( sqlite3_mutex_held(db->mutex) );
  எனில்( db->mallocFailed || rc ){
    திரும்பு apiHandleError(db, rc);
  }
  திரும்பு 0;
}
