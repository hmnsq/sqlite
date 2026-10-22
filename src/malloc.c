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
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"
#ಅನ್ತರ್ಭಾವ <stdarg.h>

/*
** Attempt to release up to n bytes of non-essential memory currently
** held by SQLite. An example of non-essential memory is memory used to
** cache database pages that are not currently in use.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_release_memory(ಪೂರ್ಣಾಙ್ಕ n){
#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
  ಪ್ರತ್ಯಾಗಮ sqlite3PcacheReleaseMemory(n);
#ಅನ್ಯಥಾ
  /* IMPLEMENTATION-OF: R-34391-24921 The sqlite3_release_memory() routine
  ** is a no-op returning zero if SQLite is not compiled with
  ** SQLITE_ENABLE_MEMORY_MANAGEMENT. */
  UNUSED_PARAMETER(n);
  ಪ್ರತ್ಯಾಗಮ 0;
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
ಸ್ಥಿರ SQLITE_WSD ಸಂರಚನಾ Mem0Global {
  sqlite3_mutex *mutex;         /* Mutex to serialize access */
  sqlite3_int64 alarmThreshold; /* The soft heap limit */
  sqlite3_int64 hardLimit;      /* The hard upper bound on memory */

  /*
  ** True if heap is nearly "full" where "full" is defined by the
  ** sqlite3_soft_heap_limit() setting.
  */
  ಪೂರ್ಣಾಙ್ಕ nearlyFull;
} mem0 = { 0, SQLITE_MAX_MEMORY, SQLITE_MAX_MEMORY, 0 };

#define mem0 GLOBAL(ಸಂರಚನಾ Mem0Global, mem0)

/*
** Return the memory allocator mutex. sqlite3_status() needs it.
*/
sqlite3_mutex *sqlite3MallocMutex(ಶೂನ್ಯ){
  ಪ್ರತ್ಯಾಗಮ mem0.mutex;
}

#ifndef SQLITE_OMIT_DEPRECATED
/*
** Deprecated external interface.  It used to set an alarm callback
** that was invoked when memory usage grew too large.  Now it is a
** no-op.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_memory_alarm(
  ಶೂನ್ಯ(*xCallback)(ಶೂನ್ಯ *pArg, sqlite3_int64 used,ಪೂರ್ಣಾಙ್ಕ N),
  ಶೂನ್ಯ *pArg,
  sqlite3_int64 iThreshold
){
  (ಶೂನ್ಯ)xCallback;
  (ಶೂನ್ಯ)pArg;
  (ಶೂನ್ಯ)iThreshold;
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
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
  ಪೂರ್ಣಾಙ್ಕ rc = sqlite3_initialize();
  ಯದಿ( rc ) ಪ್ರತ್ಯಾಗಮ -1;
#endif
  sqlite3_mutex_enter(mem0.mutex);
  priorLimit = mem0.alarmThreshold;
  ಯದಿ( n<0 ){
    sqlite3_mutex_leave(mem0.mutex);
    ಪ್ರತ್ಯಾಗಮ priorLimit;
  }
  ಯದಿ( mem0.hardLimit>0 && (n>mem0.hardLimit || n==0) ){
    n = mem0.hardLimit;
  }
  mem0.alarmThreshold = n;
  nUsed = sqlite3_Status_ಮೌಲ್ಯ(SQLITE_STATUS_MEMORY_USED);
  AtomicStore(&mem0.nearlyFull, n>0 && n<=nUsed);
  sqlite3_mutex_leave(mem0.mutex);
  excess = sqlite3_memory_used() - n;
  ಯದಿ( excess>0 ) sqlite3_release_memory((ಪೂರ್ಣಾಙ್ಕ)(excess & 0x7fffffff));
  ಪ್ರತ್ಯಾಗಮ priorLimit;
}
ಶೂನ್ಯ sqlite3_soft_heap_limit(ಪೂರ್ಣಾಙ್ಕ n){
  ಯದಿ( n<0 ) n = 0;
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
  ಪೂರ್ಣಾಙ್ಕ rc = sqlite3_initialize();
  ಯದಿ( rc ) ಪ್ರತ್ಯಾಗಮ -1;
#endif
  sqlite3_mutex_enter(mem0.mutex);
  priorLimit = mem0.hardLimit;
  ಯದಿ( n>=0 ){
    mem0.hardLimit = n;
    ಯದಿ( n<mem0.alarmThreshold || mem0.alarmThreshold==0 ){
      mem0.alarmThreshold = n;
    }
  }
  sqlite3_mutex_leave(mem0.mutex);
  ಪ್ರತ್ಯಾಗಮ priorLimit;
}


/*
** Initialize the memory allocation subsystem.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3MallocInit(ಶೂನ್ಯ){
  ಪೂರ್ಣಾಙ್ಕ rc;
  ಯದಿ( sqlite3GlobalConfig.m.xMalloc==0 ){
    sqlite3MemSetDefault();
  }
  mem0.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM);
  ಯದಿ( sqlite3GlobalConfig.pPage==0 || sqlite3GlobalConfig.szPage<512
      || sqlite3GlobalConfig.nPage<=0 ){
    sqlite3GlobalConfig.pPage = 0;
    sqlite3GlobalConfig.szPage = 0;
  }
  rc = sqlite3GlobalConfig.m.xInit(sqlite3GlobalConfig.m.pAppData);
  ಯದಿ( rc!=SQLITE_OK ) memset(&mem0, 0, ಪರಿಮಾಣ(mem0));
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Return true if the heap is currently under memory pressure - in other
** words if the amount of heap used is close to the limit set by
** sqlite3_soft_heap_limit().
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3HeapNearlyFull(ಶೂನ್ಯ){
  ಪ್ರತ್ಯಾಗಮ AtomicLoad(&mem0.nearlyFull);
}

/*
** Deinitialize the memory allocation subsystem.
*/
ಶೂನ್ಯ sqlite3MallocEnd(ಶೂನ್ಯ){
  ಯದಿ( sqlite3GlobalConfig.m.xShutdown ){
    sqlite3GlobalConfig.m.xShutdown(sqlite3GlobalConfig.m.pAppData);
  }
  memset(&mem0, 0, ಪರಿಮಾಣ(mem0));
}

/*
** Return the amount of memory currently checked out.
*/
sqlite3_int64 sqlite3_memory_used(ಶೂನ್ಯ){
  sqlite3_int64 res, mx;
  sqlite3_status64(SQLITE_STATUS_MEMORY_USED, &res, &mx, 0);
  ಪ್ರತ್ಯಾಗಮ res;
}

/*
** Return the maximum amount of memory that has ever been
** checked out since either the beginning of this process
** or since the most recent reset.
*/
sqlite3_int64 sqlite3_memory_highwater(ಪೂರ್ಣಾಙ್ಕ resetFlag){
  sqlite3_int64 res, mx;
  sqlite3_status64(SQLITE_STATUS_MEMORY_USED, &res, &mx, resetFlag);
  ಪ್ರತ್ಯಾಗಮ mx;
}

/*
** Trigger the alarm 
*/
ಸ್ಥಿರ ಶೂನ್ಯ sqlite3MallocAlarm(ಪೂರ್ಣಾಙ್ಕ nByte){
  ಯದಿ( mem0.alarmThreshold<=0 ) ಪ್ರತ್ಯಾಗಮ;
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
ಸ್ಥಿರ SQLITE_NOINLINE ಶೂನ್ಯ test_oom_breakpoint(u64 n){
  ಸ್ಥಿರ u64 nOomFault = 0;
  nOomFault += n;
  /* The assert() is never reached in a human lifetime.  It  is here mostly
  ** to prevent code optimizers from optimizing out this function. */
  ಪ್ರತಿಜ್ಞಾ( (nOomFault>>32) < 0xffffffff );
}
#ಅನ್ಯಥಾ
# define test_oom_breakpoint(X)   /* No-op for production builds */
#endif

/*
** Do a memory allocation with statistics and alarms.  Assume the
** lock is already held.
*/
ಸ್ಥಿರ ಶೂನ್ಯ mallocWithAlarm(ಪೂರ್ಣಾಙ್ಕ n, ಶೂನ್ಯ **pp){
  ಶೂನ್ಯ *p;
  ಪೂರ್ಣಾಙ್ಕ nFull;
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(mem0.mutex) );
  ಪ್ರತಿಜ್ಞಾ( n>0 );

  /* In Firefox (circa 2017-02-08), xRoundup() is remapped to an internal
  ** implementation of malloc_good_size(), which must be called in debug
  ** mode and specifically when the DMD "Dark Matter Detector" is enabled
  ** or else a crash results.  Hence, do not attempt to optimize out the
  ** following xRoundup() call. */
  nFull = sqlite3GlobalConfig.m.xRoundup(n);

  sqlite3StatusHighwater(SQLITE_STATUS_MALLOC_SIZE, n);
  ಯದಿ( mem0.alarmThreshold>0 ){
    sqlite3_int64 nUsed = sqlite3_Status_ಮೌಲ್ಯ(SQLITE_STATUS_MEMORY_USED);
    ಯದಿ( nUsed >= mem0.alarmThreshold - nFull ){
      AtomicStore(&mem0.nearlyFull, 1);
      sqlite3MallocAlarm(nFull);
      ಯದಿ( mem0.hardLimit ){
        nUsed = sqlite3_Status_ಮೌಲ್ಯ(SQLITE_STATUS_MEMORY_USED);
        ಯದಿ( nUsed >= mem0.hardLimit - nFull ){
          test_oom_breakpoint(1);
          *pp = 0;
          ಪ್ರತ್ಯಾಗಮ;
        }
      }
    }ಅನ್ಯಥಾ{
      AtomicStore(&mem0.nearlyFull, 0);
    }
  }
  p = sqlite3GlobalConfig.m.xMalloc(nFull);
#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
  ಯದಿ( p==0 && mem0.alarmThreshold>0 ){
    sqlite3MallocAlarm(nFull);
    p = sqlite3GlobalConfig.m.xMalloc(nFull);
  }
#endif
  ಯದಿ( p ){
    nFull = sqlite3MallocSize(p);
    sqlite3StatusUp(SQLITE_STATUS_MEMORY_USED, nFull);
    sqlite3StatusUp(SQLITE_STATUS_MALLOC_ಎಣಿಕೆ, 1);
  }
  *pp = p;
}

/*
** Allocate memory.  This routine is like sqlite3_malloc() except that it
** assumes the memory subsystem has already been initialized.
*/
ಶೂನ್ಯ *sqlite3Malloc(u64 n){
  ಶೂನ್ಯ *p;
  ಯದಿ( n==0 || n>SQLITE_MAX_ALLOCATION_SIZE ){
    p = 0;
  }ಅನ್ಯಥಾ ಯದಿ( sqlite3GlobalConfig.bMemstat ){
    sqlite3_mutex_enter(mem0.mutex);
    mallocWithAlarm((ಪೂರ್ಣಾಙ್ಕ)n, &p);
    sqlite3_mutex_leave(mem0.mutex);
  }ಅನ್ಯಥಾ{
    p = sqlite3GlobalConfig.m.xMalloc((ಪೂರ್ಣಾಙ್ಕ)n);
  }
  ಪ್ರತಿಜ್ಞಾ( EIGHT_BYTE_ALIGNMENT(p) );  /* IMP: R-11148-40995 */
  ಪ್ರತ್ಯಾಗಮ p;
}

/*
** This version of the memory allocation is for use by the application.
** First make sure the memory subsystem is initialized, then do the
** allocation.
*/
ಶೂನ್ಯ *sqlite3_malloc(ಪೂರ್ಣಾಙ್ಕ n){
#ifndef SQLITE_OMIT_AUTOINIT
  ಯದಿ( sqlite3_initialize() ) ಪ್ರತ್ಯಾಗಮ 0;
#endif
  ಪ್ರತ್ಯಾಗಮ n<=0 ? 0 : sqlite3Malloc(n);
}
ಶೂನ್ಯ *sqlite3_malloc64(sqlite3_uint64 n){
#ifndef SQLITE_OMIT_AUTOINIT
  ಯದಿ( sqlite3_initialize() ) ಪ್ರತ್ಯಾಗಮ 0;
#endif
  ಪ್ರತ್ಯಾಗಮ sqlite3Malloc(n);
}

/*
** TRUE if p is a lookaside memory allocation from db
*/
#ifndef SQLITE_OMIT_LOOKASIDE
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ isLookaside(sqlite3 *db, ಧ್ರುವ ಶೂನ್ಯ *p){
  ಪ್ರತ್ಯಾಗಮ SQLITE_WITHIN(p, db->lookaside.pStart, db->lookaside.pTrueEnd);
}
#ಅನ್ಯಥಾ
#define isLookaside(A,B) 0
#endif

/*
** Return the size of a memory allocation previously obtained from
** sqlite3Malloc() or sqlite3_malloc().
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3MallocSize(ಧ್ರುವ ಶೂನ್ಯ *p){
  ಪ್ರತಿಜ್ಞಾ( sqlite3MemdebugHasType(p, MEMTYPE_HEAP) );
  ಪ್ರತ್ಯಾಗಮ sqlite3GlobalConfig.m.xSize((ಶೂನ್ಯ*)p);
}
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ lookasideMallocSize(sqlite3 *db, ಧ್ರುವ ಶೂನ್ಯ *p){
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE    
  ಪ್ರತ್ಯಾಗಮ p<db->lookaside.p_ಮಧ್ಯ ? db->lookaside.szTrue : LOOKASIDE_SMALL;
#ಅನ್ಯಥಾ
  ಪ್ರತ್ಯಾಗಮ db->lookaside.szTrue;
#endif  
}
ಪೂರ್ಣಾಙ್ಕ sqlite3DbMallocSize(sqlite3 *db, ಧ್ರುವ ಶೂನ್ಯ *p){
  ಪ್ರತಿಜ್ಞಾ( p!=0 );
#ifdef SQLITE_DEBUG
  ಯದಿ( db==0 ){
    ಪ್ರತಿಜ್ಞಾ( sqlite3MemdebugNoType(p, (u8)~MEMTYPE_HEAP) );
    ಪ್ರತಿಜ್ಞಾ( sqlite3MemdebugHasType(p, MEMTYPE_HEAP) );
  }ಅನ್ಯಥಾ ಯದಿ( !isLookaside(db,p) ){
    ಪ್ರತಿಜ್ಞಾ( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
    ಪ್ರತಿಜ್ಞಾ( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  }
#endif
  ಯದಿ( db ){
    ಯದಿ( ((uptr)p)<(uptr)(db->lookaside.pTrueEnd) ){
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
      ಯದಿ( ((uptr)p)>=(uptr)(db->lookaside.p_ಮಧ್ಯ) ){
        ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(db->mutex) );
        ಪ್ರತ್ಯಾಗಮ LOOKASIDE_SMALL;
      }
#endif
      ಯದಿ( ((uptr)p)>=(uptr)(db->lookaside.pStart) ){
        ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(db->mutex) );
        ಪ್ರತ್ಯಾಗಮ db->lookaside.szTrue;
      }
    }
  }
  ಪ್ರತ್ಯಾಗಮ sqlite3GlobalConfig.m.xSize((ಶೂನ್ಯ*)p);
}
sqlite3_uint64 sqlite3_msize(ಶೂನ್ಯ *p){
  ಪ್ರತಿಜ್ಞಾ( sqlite3MemdebugNoType(p, (u8)~MEMTYPE_HEAP) );
  ಪ್ರತಿಜ್ಞಾ( sqlite3MemdebugHasType(p, MEMTYPE_HEAP) );
  ಪ್ರತ್ಯಾಗಮ p ? sqlite3GlobalConfig.m.xSize(p) : 0;
}

/*
** Free memory previously obtained from sqlite3Malloc().
*/
ಶೂನ್ಯ sqlite3_free(ಶೂನ್ಯ *p){
  ಯದಿ( p==0 ) ಪ್ರತ್ಯಾಗಮ;  /* IMP: R-49053-54554 */
  ಪ್ರತಿಜ್ಞಾ( sqlite3MemdebugHasType(p, MEMTYPE_HEAP) );
  ಪ್ರತಿಜ್ಞಾ( sqlite3MemdebugNoType(p, (u8)~MEMTYPE_HEAP) );
  ಯದಿ( sqlite3GlobalConfig.bMemstat ){
    sqlite3_mutex_enter(mem0.mutex);
    sqlite3StatusDown(SQLITE_STATUS_MEMORY_USED, sqlite3MallocSize(p));
    sqlite3StatusDown(SQLITE_STATUS_MALLOC_ಎಣಿಕೆ, 1);
    sqlite3GlobalConfig.m.xFree(p);
    sqlite3_mutex_leave(mem0.mutex);
  }ಅನ್ಯಥಾ{
    sqlite3GlobalConfig.m.xFree(p);
  }
}

/*
** Add the size of memory allocation "p" to the count in
** *db->pnBytesFreed.
*/
ಸ್ಥಿರ SQLITE_NOINLINE ಶೂನ್ಯ measureAllocationSize(sqlite3 *db, ಶೂನ್ಯ *p){
  *db->pnBytesFreed += sqlite3DbMallocSize(db,p);
}

/*
** Free memory that might be associated with a particular database
** connection.  Calling sqlite3DbFree(D,X) for X==0 is a harmless no-op.
** The sqlite3DbFreeNN(D,X) version requires that X be non-NULL.
*/
ಶೂನ್ಯ sqlite3DbFreeNN(sqlite3 *db, ಶೂನ್ಯ *p){
  ಪ್ರತಿಜ್ಞಾ( db==0 || sqlite3_mutex_held(db->mutex) );
  ಪ್ರತಿಜ್ಞಾ( p!=0 );
  ಯದಿ( db ){
    ಯದಿ( ((uptr)p)<(uptr)(db->lookaside.pEnd) ){
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
      ಯದಿ( ((uptr)p)>=(uptr)(db->lookaside.p_ಮಧ್ಯ) ){
        LookasideSlot *pBuf = (LookasideSlot*)p;
        ಪ್ರತಿಜ್ಞಾ( db->pnBytesFreed==0 );
#ifdef SQLITE_DEBUG
        memset(p, 0xaa, LOOKASIDE_SMALL);  /* Trash freed content */
#endif
        pBuf->pNext = db->lookaside.pSmallFree;
        db->lookaside.pSmallFree = pBuf;
        ಪ್ರತ್ಯಾಗಮ;
      }
#endif /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
      ಯದಿ( ((uptr)p)>=(uptr)(db->lookaside.pStart) ){
        LookasideSlot *pBuf = (LookasideSlot*)p;
        ಪ್ರತಿಜ್ಞಾ( db->pnBytesFreed==0 );
#ifdef SQLITE_DEBUG
        memset(p, 0xaa, db->lookaside.szTrue);  /* Trash freed content */
#endif
        pBuf->pNext = db->lookaside.pFree;
        db->lookaside.pFree = pBuf;
        ಪ್ರತ್ಯಾಗಮ;
      }
    }
    ಯದಿ( db->pnBytesFreed ){
      measureAllocationSize(db, p);
      ಪ್ರತ್ಯಾಗಮ;
    }
  }
  ಪ್ರತಿಜ್ಞಾ( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  ಪ್ರತಿಜ್ಞಾ( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  ಪ್ರತಿಜ್ಞಾ( db!=0 || sqlite3MemdebugNoType(p, MEMTYPE_LOOKASIDE) );
  sqlite3MemdebugSetType(p, MEMTYPE_HEAP);
  sqlite3_free(p);
}
ಶೂನ್ಯ sqlite3DbNNFreeNN(sqlite3 *db, ಶೂನ್ಯ *p){
  ಪ್ರತಿಜ್ಞಾ( db!=0 );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(db->mutex) );
  ಪ್ರತಿಜ್ಞಾ( p!=0 );
  ಯದಿ( ((uptr)p)<(uptr)(db->lookaside.pEnd) ){
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
    ಯದಿ( ((uptr)p)>=(uptr)(db->lookaside.p_ಮಧ್ಯ) ){
      LookasideSlot *pBuf = (LookasideSlot*)p;
      ಪ್ರತಿಜ್ಞಾ( db->pnBytesFreed==0 );
#ifdef SQLITE_DEBUG
      memset(p, 0xaa, LOOKASIDE_SMALL);  /* Trash freed content */
#endif
      pBuf->pNext = db->lookaside.pSmallFree;
      db->lookaside.pSmallFree = pBuf;
      ಪ್ರತ್ಯಾಗಮ;
    }
#endif /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
    ಯದಿ( ((uptr)p)>=(uptr)(db->lookaside.pStart) ){
      LookasideSlot *pBuf = (LookasideSlot*)p;
      ಪ್ರತಿಜ್ಞಾ( db->pnBytesFreed==0 );
#ifdef SQLITE_DEBUG
      memset(p, 0xaa, db->lookaside.szTrue);  /* Trash freed content */
#endif
      pBuf->pNext = db->lookaside.pFree;
      db->lookaside.pFree = pBuf;
      ಪ್ರತ್ಯಾಗಮ;
    }
  }
  ಯದಿ( db->pnBytesFreed ){
    measureAllocationSize(db, p);
    ಪ್ರತ್ಯಾಗಮ;
  }
  ಪ್ರತಿಜ್ಞಾ( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  ಪ್ರತಿಜ್ಞಾ( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  sqlite3MemdebugSetType(p, MEMTYPE_HEAP);
  sqlite3_free(p);
}
ಶೂನ್ಯ sqlite3DbFree(sqlite3 *db, ಶೂನ್ಯ *p){
  ಪ್ರತಿಜ್ಞಾ( db==0 || sqlite3_mutex_held(db->mutex) );
  ಯದಿ( p ) sqlite3DbFreeNN(db, p);
}

/*
** Change the size of an existing memory allocation
*/
ಶೂನ್ಯ *sqlite3Realloc(ಶೂನ್ಯ *pOld, u64 nBytes){
  ಪೂರ್ಣಾಙ್ಕ nOld, nNew, nDiff;
  ಶೂನ್ಯ *pNew;
  ಪ್ರತಿಜ್ಞಾ( sqlite3MemdebugHasType(pOld, MEMTYPE_HEAP) );
  ಪ್ರತಿಜ್ಞಾ( sqlite3MemdebugNoType(pOld, (u8)~MEMTYPE_HEAP) );
  ಯದಿ( pOld==0 ){
    ಪ್ರತ್ಯಾಗಮ sqlite3Malloc(nBytes); /* IMP: R-04300-56712 */
  }
  ಯದಿ( nBytes==0 ){
    sqlite3_free(pOld); /* IMP: R-26507-47431 */
    ಪ್ರತ್ಯಾಗಮ 0;
  }
  ಯದಿ( nBytes>SQLITE_MAX_ALLOCATION_SIZE ){
    ಪ್ರತ್ಯಾಗಮ 0;
  }
  nOld = sqlite3MallocSize(pOld);
  /* IMPLEMENTATION-OF: R-46199-30249 SQLite guarantees that the second
  ** argument to xRealloc is always a value returned by a prior call to
  ** xRoundup. */
  nNew = sqlite3GlobalConfig.m.xRoundup((ಪೂರ್ಣಾಙ್ಕ)nBytes);
  ಯದಿ( nOld==nNew ){
    pNew = pOld;
  }ಅನ್ಯಥಾ ಯದಿ( sqlite3GlobalConfig.bMemstat ){
    sqlite3_int64 nUsed;
    sqlite3_mutex_enter(mem0.mutex);
    sqlite3StatusHighwater(SQLITE_STATUS_MALLOC_SIZE, (ಪೂರ್ಣಾಙ್ಕ)nBytes);
    nDiff = nNew - nOld;
    ಯದಿ( nDiff>0 && (nUsed = sqlite3_Status_ಮೌಲ್ಯ(SQLITE_STATUS_MEMORY_USED)) >= 
          mem0.alarmThreshold-nDiff ){
      sqlite3MallocAlarm(nDiff);
      ಯದಿ( mem0.hardLimit>0 && nUsed >= mem0.hardLimit - nDiff ){
        sqlite3_mutex_leave(mem0.mutex);
        test_oom_breakpoint(1);
        ಪ್ರತ್ಯಾಗಮ 0;
      }
    }
    pNew = sqlite3GlobalConfig.m.xRealloc(pOld, nNew);
#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
    ಯದಿ( pNew==0 && mem0.alarmThreshold>0 ){
      sqlite3MallocAlarm((ಪೂರ್ಣಾಙ್ಕ)nBytes);
      pNew = sqlite3GlobalConfig.m.xRealloc(pOld, nNew);
    }
#endif
    ಯದಿ( pNew ){
      nNew = sqlite3MallocSize(pNew);
      sqlite3StatusUp(SQLITE_STATUS_MEMORY_USED, nNew-nOld);
    }
    sqlite3_mutex_leave(mem0.mutex);
  }ಅನ್ಯಥಾ{
    pNew = sqlite3GlobalConfig.m.xRealloc(pOld, nNew);
  }
  ಪ್ರತಿಜ್ಞಾ( EIGHT_BYTE_ALIGNMENT(pNew) ); /* IMP: R-11148-40995 */
  ಪ್ರತ್ಯಾಗಮ pNew;
}

/*
** The public interface to sqlite3Realloc.  Make sure that the memory
** subsystem is initialized prior to invoking sqliteRealloc.
*/
ಶೂನ್ಯ *sqlite3_realloc(ಶೂನ್ಯ *pOld, ಪೂರ್ಣಾಙ್ಕ n){
#ifndef SQLITE_OMIT_AUTOINIT
  ಯದಿ( sqlite3_initialize() ) ಪ್ರತ್ಯಾಗಮ 0;
#endif
  ಯದಿ( n<0 ) n = 0;  /* IMP: R-26507-47431 */
  ಪ್ರತ್ಯಾಗಮ sqlite3Realloc(pOld, n);
}
ಶೂನ್ಯ *sqlite3_realloc64(ಶೂನ್ಯ *pOld, sqlite3_uint64 n){
#ifndef SQLITE_OMIT_AUTOINIT
  ಯದಿ( sqlite3_initialize() ) ಪ್ರತ್ಯಾಗಮ 0;
#endif
  ಪ್ರತ್ಯಾಗಮ sqlite3Realloc(pOld, n);
}


/*
** Allocate and zero memory.
*/ 
ಶೂನ್ಯ *sqlite3MallocZero(u64 n){
  ಶೂನ್ಯ *p = sqlite3Malloc(n);
  ಯದಿ( p ){
    memset(p, 0, (size_t)n);
  }
  ಪ್ರತ್ಯಾಗಮ p;
}

/*
** Allocate and zero memory.  If the allocation fails, make
** the mallocFailed flag in the connection pointer.
*/
ಶೂನ್ಯ *sqlite3DbMallocZero(sqlite3 *db, u64 n){
  ಶೂನ್ಯ *p;
  testcase( db==0 );
  p = sqlite3DbMallocRaw(db, n);
  ಯದಿ( p ) memset(p, 0, (size_t)n);
  ಪ್ರತ್ಯಾಗಮ p;
}


/* Finish the work of sqlite3DbMallocRawNN for the unusual and
** slower case when the allocation cannot be fulfilled using lookaside.
*/
ಸ್ಥಿರ SQLITE_NOINLINE ಶೂನ್ಯ *dbMallocRawFinish(sqlite3 *db, u64 n){
  ಶೂನ್ಯ *p;
  ಪ್ರತಿಜ್ಞಾ( db!=0 );
  p = sqlite3Malloc(n);
  ಯದಿ( !p ) sqlite3OomFault(db);
  sqlite3MemdebugSetType(p, 
         (db->lookaside.bDisable==0) ? MEMTYPE_LOOKASIDE : MEMTYPE_HEAP);
  ಪ್ರತ್ಯಾಗಮ p;
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
ಶೂನ್ಯ *sqlite3DbMallocRaw(sqlite3 *db, u64 n){
  ಶೂನ್ಯ *p;
  ಯದಿ( db ) ಪ್ರತ್ಯಾಗಮ sqlite3DbMallocRawNN(db, n);
  p = sqlite3Malloc(n);
  sqlite3MemdebugSetType(p, MEMTYPE_HEAP);
  ಪ್ರತ್ಯಾಗಮ p;
}
ಶೂನ್ಯ *sqlite3DbMallocRawNN(sqlite3 *db, u64 n){
#ifndef SQLITE_OMIT_LOOKASIDE
  LookasideSlot *pBuf;
  ಪ್ರತಿಜ್ಞಾ( db!=0 );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(db->mutex) );
  ಪ್ರತಿಜ್ಞಾ( db->pnBytesFreed==0 );
  ಯದಿ( n>db->lookaside.sz ){
    ಯದಿ( !db->lookaside.bDisable ){
      db->lookaside.anStat[1]++;      
    }ಅನ್ಯಥಾ ಯದಿ( db->mallocFailed ){
      ಪ್ರತ್ಯಾಗಮ 0;
    }
    ಪ್ರತ್ಯಾಗಮ dbMallocRawFinish(db, n);
  }
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
  ಯದಿ( n<=LOOKASIDE_SMALL ){
    ಯದಿ( (pBuf = db->lookaside.pSmallFree)!=0 ){
      db->lookaside.pSmallFree = pBuf->pNext;
      db->lookaside.anStat[0]++;
      ಪ್ರತ್ಯಾಗಮ (ಶೂನ್ಯ*)pBuf;
    }ಅನ್ಯಥಾ ಯದಿ( (pBuf = db->lookaside.pSmallInit)!=0 ){
      db->lookaside.pSmallInit = pBuf->pNext;
      db->lookaside.anStat[0]++;
      ಪ್ರತ್ಯಾಗಮ (ಶೂನ್ಯ*)pBuf;
    }
  }
#endif
  ಯದಿ( (pBuf = db->lookaside.pFree)!=0 ){
    db->lookaside.pFree = pBuf->pNext;
    db->lookaside.anStat[0]++;
    ಪ್ರತ್ಯಾಗಮ (ಶೂನ್ಯ*)pBuf;
  }ಅನ್ಯಥಾ ಯದಿ( (pBuf = db->lookaside.pInit)!=0 ){
    db->lookaside.pInit = pBuf->pNext;
    db->lookaside.anStat[0]++;
    ಪ್ರತ್ಯಾಗಮ (ಶೂನ್ಯ*)pBuf;
  }ಅನ್ಯಥಾ{
    db->lookaside.anStat[2]++;
  }
#ಅನ್ಯಥಾ
  ಪ್ರತಿಜ್ಞಾ( db!=0 );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(db->mutex) );
  ಪ್ರತಿಜ್ಞಾ( db->pnBytesFreed==0 );
  ಯದಿ( db->mallocFailed ){
    ಪ್ರತ್ಯಾಗಮ 0;
  }
#endif
  ಪ್ರತ್ಯಾಗಮ dbMallocRawFinish(db, n);
}

/* Forward declaration */
ಸ್ಥಿರ SQLITE_NOINLINE ಶೂನ್ಯ *dbReallocFinish(sqlite3 *db, ಶೂನ್ಯ *p, u64 n);

/*
** Resize the block of memory pointed to by p to n bytes. If the
** resize fails, set the mallocFailed flag in the connection object.
*/
ಶೂನ್ಯ *sqlite3DbRealloc(sqlite3 *db, ಶೂನ್ಯ *p, u64 n){
  ಪ್ರತಿಜ್ಞಾ( db!=0 );
  ಯದಿ( p==0 ) ಪ್ರತ್ಯಾಗಮ sqlite3DbMallocRawNN(db, n);
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(db->mutex) );
  ಯದಿ( ((uptr)p)<(uptr)db->lookaside.pEnd ){
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
    ಯದಿ( ((uptr)p)>=(uptr)db->lookaside.p_ಮಧ್ಯ ){
      ಯದಿ( n<=LOOKASIDE_SMALL ) ಪ್ರತ್ಯಾಗಮ p;
    }ಅನ್ಯಥಾ
#endif
    ಯದಿ( ((uptr)p)>=(uptr)db->lookaside.pStart ){
      ಯದಿ( n<=db->lookaside.szTrue ) ಪ್ರತ್ಯಾಗಮ p;
    }
  }
  ಪ್ರತ್ಯಾಗಮ dbReallocFinish(db, p, n);
}
ಸ್ಥಿರ SQLITE_NOINLINE ಶೂನ್ಯ *dbReallocFinish(sqlite3 *db, ಶೂನ್ಯ *p, u64 n){
  ಶೂನ್ಯ *pNew = 0;
  ಪ್ರತಿಜ್ಞಾ( db!=0 );
  ಪ್ರತಿಜ್ಞಾ( p!=0 );
  ಯದಿ( db->mallocFailed==0 ){
    ಯದಿ( isLookaside(db, p) ){
      pNew = sqlite3DbMallocRawNN(db, n);
      ಯದಿ( pNew ){
        memcpy(pNew, p, lookasideMallocSize(db, p));
        sqlite3DbFree(db, p);
      }
    }ಅನ್ಯಥಾ{
      ಪ್ರತಿಜ್ಞಾ( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
      ಪ್ರತಿಜ್ಞಾ( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
      sqlite3MemdebugSetType(p, MEMTYPE_HEAP);
      pNew = sqlite3Realloc(p, n);
      ಯದಿ( !pNew ){
        sqlite3OomFault(db);
      }
      sqlite3MemdebugSetType(pNew,
            (db->lookaside.bDisable==0 ? MEMTYPE_LOOKASIDE : MEMTYPE_HEAP));
    }
  }
  ಪ್ರತ್ಯಾಗಮ pNew;
}

/*
** Attempt to reallocate p.  If the reallocation fails, then free p
** and set the mallocFailed flag in the database connection.
*/
ಶೂನ್ಯ *sqlite3DbReallocOrFree(sqlite3 *db, ಶೂನ್ಯ *p, u64 n){
  ಶೂನ್ಯ *pNew;
  pNew = sqlite3DbRealloc(db, p, n);
  ಯದಿ( !pNew ){
    sqlite3DbFree(db, p);
  }
  ಪ್ರತ್ಯಾಗಮ pNew;
}

/*
** Make a copy of a string in memory obtained from sqliteMalloc(). These 
** functions call sqlite3MallocRaw() directly instead of sqliteMalloc(). This
** is because when memory debugging is turned on, these two functions are 
** called via macros that record the current file and line number in the
** ThreadData structure.
*/
ವರ್ಣ *sqlite3DbStrDup(sqlite3 *db, ಧ್ರುವ ವರ್ಣ *z){
  ವರ್ಣ *zNew;
  size_t n;
  ಯದಿ( z==0 ){
    ಪ್ರತ್ಯಾಗಮ 0;
  }
  n = strlen(z) + 1;
  zNew = sqlite3DbMallocRaw(db, n);
  ಯದಿ( zNew ){
    memcpy(zNew, z, n);
  }
  ಪ್ರತ್ಯಾಗಮ zNew;
}
ವರ್ಣ *sqlite3DbStrNDup(sqlite3 *db, ಧ್ರುವ ವರ್ಣ *z, u64 n){
  ವರ್ಣ *zNew;
  ಪ್ರತಿಜ್ಞಾ( db!=0 );
  ಪ್ರತಿಜ್ಞಾ( z!=0 || n==0 );
  ಪ್ರತಿಜ್ಞಾ( (n&0x7fffffff)==n );
  zNew = z ? sqlite3DbMallocRawNN(db, n+1) : 0;
  ಯದಿ( zNew ){
    memcpy(zNew, z, (size_t)n);
    zNew[n] = 0;
  }
  ಪ್ರತ್ಯಾಗಮ zNew;
}

/*
** The text between zStart and zEnd represents a phrase within a larger
** SQL statement.  Make a copy of this phrase in space obtained form
** sqlite3DbMalloc().  Omit leading and trailing whitespace.
*/
ವರ್ಣ *sqlite3DbSpanDup(sqlite3 *db, ಧ್ರುವ ವರ್ಣ *zStart, ಧ್ರುವ ವರ್ಣ *zEnd){
  ಪೂರ್ಣಾಙ್ಕ n;
#ifdef SQLITE_DEBUG
  /* Because of the way the parser works, the span is guaranteed to contain
  ** at least one non-space character */
  ಪ್ರತಿ(n=0; sqlite3Isspace(zStart[n]); n++){ ಪ್ರತಿಜ್ಞಾ( &zStart[n]<zEnd ); }
#endif
  ಯಾವತ್( sqlite3Isspace(zStart[0]) ) zStart++;
  n = (ಪೂರ್ಣಾಙ್ಕ)(zEnd - zStart);
  ಯಾವತ್( sqlite3Isspace(zStart[n-1]) ) n--;
  ಪ್ರತ್ಯಾಗಮ sqlite3DbStrNDup(db, zStart, n);
}

/*
** Free any prior content in *pz and replace it with a copy of zNew.
*/
ಶೂನ್ಯ sqlite3SetString(ವರ್ಣ **pz, sqlite3 *db, ಧ್ರುವ ವರ್ಣ *zNew){
  ವರ್ಣ *z = sqlite3DbStrDup(db, zNew);
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
ಶೂನ್ಯ *sqlite3OomFault(sqlite3 *db){
  ಯದಿ( db->mallocFailed==0 && db->bBenignMalloc==0 ){
    db->mallocFailed = 1;
    ಯದಿ( db->nVdbeExec>0 ){
      AtomicStore(&db->u1.isInterrupted, 1);
    }
    DisableLookaside;
    ಯದಿ( db->pParse ){
      Parse *pParse;
      sqlite3_Error_ಸಂದೇಶ(db->pParse, "out of memory");
      db->pParse->rc = SQLITE_NOMEM_BKPT;
      ಪ್ರತಿ(pParse=db->pParse->pOuterParse; pParse; pParse = pParse->pOuterParse){
        pParse->nErr++;
        pParse->rc = SQLITE_NOMEM;
      } 
    }
  }
  ಪ್ರತ್ಯಾಗಮ 0;
}

/*
** This routine reactivates the memory allocator and clears the
** db->mallocFailed flag as necessary.
**
** The memory allocator is not restarted if there are running
** VDBEs.
*/
ಶೂನ್ಯ sqlite3OomClear(sqlite3 *db){
  ಯದಿ( db->mallocFailed && db->nVdbeExec==0 ){
    db->mallocFailed = 0;
    AtomicStore(&db->u1.isInterrupted, 0);
    ಪ್ರತಿಜ್ಞಾ( db->lookaside.bDisable>0 );
    EnableLookaside;
  }
}

/*
** Take actions at the end of an API call to deal with error codes.
*/
ಸ್ಥಿರ SQLITE_NOINLINE ಪೂರ್ಣಾಙ್ಕ apiHandleError(sqlite3 *db, ಪೂರ್ಣಾಙ್ಕ rc){
  ಯದಿ( db->mallocFailed || rc==SQLITE_IOERR_NOMEM ){
    sqlite3OomClear(db);
    sqlite3Error(db, SQLITE_NOMEM);
    ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM_BKPT;
  }
  ಪ್ರತ್ಯಾಗಮ rc & db->errMask;
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
ಪೂರ್ಣಾಙ್ಕ sqlite3ApiExit(sqlite3* db, ಪೂರ್ಣಾಙ್ಕ rc){
  /* If the db handle must hold the connection handle mutex here.
  ** Otherwise the read (and possible write) of db->mallocFailed 
  ** is unsafe, as is the call to sqlite3Error().
  */
  ಪ್ರತಿಜ್ಞಾ( db!=0 );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(db->mutex) );
  ಯದಿ( db->mallocFailed || rc ){
    ಪ್ರತ್ಯಾಗಮ apiHandleError(db, rc);
  }
  ಪ್ರತ್ಯಾಗಮ 0;
}
