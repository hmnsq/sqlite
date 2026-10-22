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
#ଅନ୍ତର୍ଭାଵ "sqliteInt.h"
#ଅନ୍ତର୍ଭାଵ <stdarg.h>

/*
** Attempt to release up to n bytes of non-essential memory currently
** held by SQLite. An example of non-essential memory is memory used to
** cache database pages that are not currently in use.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_release_memory(ପୂର୍ଣ୍ଣସଂଖ୍ୟା n){
#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
  ଫେରନ୍ତୁ sqlite3PcacheReleaseMemory(n);
#ଅନ୍ୟଥା
  /* IMPLEMENTATION-OF: R-34391-24921 The sqlite3_release_memory() routine
  ** is a no-op returning zero if SQLite is not compiled with
  ** SQLITE_ENABLE_MEMORY_MANAGEMENT. */
  UNUSED_PARAMETER(n);
  ଫେରନ୍ତୁ 0;
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
ସ୍ଥିର SQLITE_WSD ସଂରଚନା Mem0Global {
  sqlite3_mutex *mutex;         /* Mutex to serialize access */
  sqlite3_int64 alarmThreshold; /* The soft heap limit */
  sqlite3_int64 hardLimit;      /* The hard upper bound on memory */

  /*
  ** True if heap is nearly "full" where "full" is defined by the
  ** sqlite3_soft_heap_limit() setting.
  */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nearlyFull;
} mem0 = { 0, SQLITE_MAX_MEMORY, SQLITE_MAX_MEMORY, 0 };

#define mem0 GLOBAL(ସଂରଚନା Mem0Global, mem0)

/*
** Return the memory allocator mutex. sqlite3_status() needs it.
*/
sqlite3_mutex *sqlite3MallocMutex(ଶୂନ୍ୟ){
  ଫେରନ୍ତୁ mem0.mutex;
}

#ifndef SQLITE_OMIT_DEPRECATED
/*
** Deprecated external interface.  It used to set an alarm callback
** that was invoked when memory usage grew too large.  Now it is a
** no-op.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_memory_alarm(
  ଶୂନ୍ୟ(*xCallback)(ଶୂନ୍ୟ *pArg, sqlite3_int64 used,ପୂର୍ଣ୍ଣସଂଖ୍ୟା N),
  ଶୂନ୍ୟ *pArg,
  sqlite3_int64 iThreshold
){
  (ଶୂନ୍ୟ)xCallback;
  (ଶୂନ୍ୟ)pArg;
  (ଶୂନ୍ୟ)iThreshold;
  ଫେରନ୍ତୁ SQLITE_OK;
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
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = sqlite3_initialize();
  ଯଦି( rc ) ଫେରନ୍ତୁ -1;
#endif
  sqlite3_mutex_enter(mem0.mutex);
  priorLimit = mem0.alarmThreshold;
  ଯଦି( n<0 ){
    sqlite3_mutex_leave(mem0.mutex);
    ଫେରନ୍ତୁ priorLimit;
  }
  ଯଦି( mem0.hardLimit>0 && (n>mem0.hardLimit || n==0) ){
    n = mem0.hardLimit;
  }
  mem0.alarmThreshold = n;
  nUsed = sqlite3_Status_ମୂଲ୍ୟ(SQLITE_STATUS_MEMORY_USED);
  AtomicStore(&mem0.nearlyFull, n>0 && n<=nUsed);
  sqlite3_mutex_leave(mem0.mutex);
  excess = sqlite3_memory_used() - n;
  ଯଦି( excess>0 ) sqlite3_release_memory((ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(excess & 0x7fffffff));
  ଫେରନ୍ତୁ priorLimit;
}
ଶୂନ୍ୟ sqlite3_soft_heap_limit(ପୂର୍ଣ୍ଣସଂଖ୍ୟା n){
  ଯଦି( n<0 ) n = 0;
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
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = sqlite3_initialize();
  ଯଦି( rc ) ଫେରନ୍ତୁ -1;
#endif
  sqlite3_mutex_enter(mem0.mutex);
  priorLimit = mem0.hardLimit;
  ଯଦି( n>=0 ){
    mem0.hardLimit = n;
    ଯଦି( n<mem0.alarmThreshold || mem0.alarmThreshold==0 ){
      mem0.alarmThreshold = n;
    }
  }
  sqlite3_mutex_leave(mem0.mutex);
  ଫେରନ୍ତୁ priorLimit;
}


/*
** Initialize the memory allocation subsystem.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3MallocInit(ଶୂନ୍ୟ){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ଯଦି( sqlite3GlobalConfig.m.xMalloc==0 ){
    sqlite3MemSetDefault();
  }
  mem0.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM);
  ଯଦି( sqlite3GlobalConfig.pPage==0 || sqlite3GlobalConfig.szPage<512
      || sqlite3GlobalConfig.nPage<=0 ){
    sqlite3GlobalConfig.pPage = 0;
    sqlite3GlobalConfig.szPage = 0;
  }
  rc = sqlite3GlobalConfig.m.xInit(sqlite3GlobalConfig.m.pAppData);
  ଯଦି( rc!=SQLITE_OK ) memset(&mem0, 0, ଆକାର(mem0));
  ଫେରନ୍ତୁ rc;
}

/*
** Return true if the heap is currently under memory pressure - in other
** words if the amount of heap used is close to the limit set by
** sqlite3_soft_heap_limit().
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3HeapNearlyFull(ଶୂନ୍ୟ){
  ଫେରନ୍ତୁ AtomicLoad(&mem0.nearlyFull);
}

/*
** Deinitialize the memory allocation subsystem.
*/
ଶୂନ୍ୟ sqlite3MallocEnd(ଶୂନ୍ୟ){
  ଯଦି( sqlite3GlobalConfig.m.xShutdown ){
    sqlite3GlobalConfig.m.xShutdown(sqlite3GlobalConfig.m.pAppData);
  }
  memset(&mem0, 0, ଆକାର(mem0));
}

/*
** Return the amount of memory currently checked out.
*/
sqlite3_int64 sqlite3_memory_used(ଶୂନ୍ୟ){
  sqlite3_int64 res, mx;
  sqlite3_status64(SQLITE_STATUS_MEMORY_USED, &res, &mx, 0);
  ଫେରନ୍ତୁ res;
}

/*
** Return the maximum amount of memory that has ever been
** checked out since either the beginning of this process
** or since the most recent reset.
*/
sqlite3_int64 sqlite3_memory_highwater(ପୂର୍ଣ୍ଣସଂଖ୍ୟା resetFlag){
  sqlite3_int64 res, mx;
  sqlite3_status64(SQLITE_STATUS_MEMORY_USED, &res, &mx, resetFlag);
  ଫେରନ୍ତୁ mx;
}

/*
** Trigger the alarm 
*/
ସ୍ଥିର ଶୂନ୍ୟ sqlite3MallocAlarm(ପୂର୍ଣ୍ଣସଂଖ୍ୟା nByte){
  ଯଦି( mem0.alarmThreshold<=0 ) ଫେରନ୍ତୁ;
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
ସ୍ଥିର SQLITE_NOINLINE ଶୂନ୍ୟ test_oom_breakpoint(u64 n){
  ସ୍ଥିର u64 nOomFault = 0;
  nOomFault += n;
  /* The assert() is never reached in a human lifetime.  It  is here mostly
  ** to prevent code optimizers from optimizing out this function. */
  ପ୍ରତିଜ୍ଞା( (nOomFault>>32) < 0xffffffff );
}
#ଅନ୍ୟଥା
# define test_oom_breakpoint(X)   /* No-op for production builds */
#endif

/*
** Do a memory allocation with statistics and alarms.  Assume the
** lock is already held.
*/
ସ୍ଥିର ଶୂନ୍ୟ mallocWithAlarm(ପୂର୍ଣ୍ଣସଂଖ୍ୟା n, ଶୂନ୍ୟ **pp){
  ଶୂନ୍ୟ *p;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nFull;
  ପ୍ରତିଜ୍ଞା( sqlite3_mutex_held(mem0.mutex) );
  ପ୍ରତିଜ୍ଞା( n>0 );

  /* In Firefox (circa 2017-02-08), xRoundup() is remapped to an internal
  ** implementation of malloc_good_size(), which must be called in debug
  ** mode and specifically when the DMD "Dark Matter Detector" is enabled
  ** or else a crash results.  Hence, do not attempt to optimize out the
  ** following xRoundup() call. */
  nFull = sqlite3GlobalConfig.m.xRoundup(n);

  sqlite3StatusHighwater(SQLITE_STATUS_MALLOC_SIZE, n);
  ଯଦି( mem0.alarmThreshold>0 ){
    sqlite3_int64 nUsed = sqlite3_Status_ମୂଲ୍ୟ(SQLITE_STATUS_MEMORY_USED);
    ଯଦି( nUsed >= mem0.alarmThreshold - nFull ){
      AtomicStore(&mem0.nearlyFull, 1);
      sqlite3MallocAlarm(nFull);
      ଯଦି( mem0.hardLimit ){
        nUsed = sqlite3_Status_ମୂଲ୍ୟ(SQLITE_STATUS_MEMORY_USED);
        ଯଦି( nUsed >= mem0.hardLimit - nFull ){
          test_oom_breakpoint(1);
          *pp = 0;
          ଫେରନ୍ତୁ;
        }
      }
    }ଅନ୍ୟଥା{
      AtomicStore(&mem0.nearlyFull, 0);
    }
  }
  p = sqlite3GlobalConfig.m.xMalloc(nFull);
#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
  ଯଦି( p==0 && mem0.alarmThreshold>0 ){
    sqlite3MallocAlarm(nFull);
    p = sqlite3GlobalConfig.m.xMalloc(nFull);
  }
#endif
  ଯଦି( p ){
    nFull = sqlite3MallocSize(p);
    sqlite3StatusUp(SQLITE_STATUS_MEMORY_USED, nFull);
    sqlite3StatusUp(SQLITE_STATUS_MALLOC_ଗଣନା, 1);
  }
  *pp = p;
}

/*
** Allocate memory.  This routine is like sqlite3_malloc() except that it
** assumes the memory subsystem has already been initialized.
*/
ଶୂନ୍ୟ *sqlite3Malloc(u64 n){
  ଶୂନ୍ୟ *p;
  ଯଦି( n==0 || n>SQLITE_MAX_ALLOCATION_SIZE ){
    p = 0;
  }ଅନ୍ୟଥା ଯଦି( sqlite3GlobalConfig.bMemstat ){
    sqlite3_mutex_enter(mem0.mutex);
    mallocWithAlarm((ପୂର୍ଣ୍ଣସଂଖ୍ୟା)n, &p);
    sqlite3_mutex_leave(mem0.mutex);
  }ଅନ୍ୟଥା{
    p = sqlite3GlobalConfig.m.xMalloc((ପୂର୍ଣ୍ଣସଂଖ୍ୟା)n);
  }
  ପ୍ରତିଜ୍ଞା( EIGHT_BYTE_ALIGNMENT(p) );  /* IMP: R-11148-40995 */
  ଫେରନ୍ତୁ p;
}

/*
** This version of the memory allocation is for use by the application.
** First make sure the memory subsystem is initialized, then do the
** allocation.
*/
ଶୂନ୍ୟ *sqlite3_malloc(ପୂର୍ଣ୍ଣସଂଖ୍ୟା n){
#ifndef SQLITE_OMIT_AUTOINIT
  ଯଦି( sqlite3_initialize() ) ଫେରନ୍ତୁ 0;
#endif
  ଫେରନ୍ତୁ n<=0 ? 0 : sqlite3Malloc(n);
}
ଶୂନ୍ୟ *sqlite3_malloc64(sqlite3_uint64 n){
#ifndef SQLITE_OMIT_AUTOINIT
  ଯଦି( sqlite3_initialize() ) ଫେରନ୍ତୁ 0;
#endif
  ଫେରନ୍ତୁ sqlite3Malloc(n);
}

/*
** TRUE if p is a lookaside memory allocation from db
*/
#ifndef SQLITE_OMIT_LOOKASIDE
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା isLookaside(sqlite3 *db, ଅଚଳ ଶୂନ୍ୟ *p){
  ଫେରନ୍ତୁ SQLITE_WITHIN(p, db->lookaside.pStart, db->lookaside.pTrueEnd);
}
#ଅନ୍ୟଥା
#define isLookaside(A,B) 0
#endif

/*
** Return the size of a memory allocation previously obtained from
** sqlite3Malloc() or sqlite3_malloc().
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3MallocSize(ଅଚଳ ଶୂନ୍ୟ *p){
  ପ୍ରତିଜ୍ଞା( sqlite3MemdebugHasType(p, MEMTYPE_HEAP) );
  ଫେରନ୍ତୁ sqlite3GlobalConfig.m.xSize((ଶୂନ୍ୟ*)p);
}
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା lookasideMallocSize(sqlite3 *db, ଅଚଳ ଶୂନ୍ୟ *p){
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE    
  ଫେରନ୍ତୁ p<db->lookaside.p_ମଧ୍ୟ ? db->lookaside.szTrue : LOOKASIDE_SMALL;
#ଅନ୍ୟଥା
  ଫେରନ୍ତୁ db->lookaside.szTrue;
#endif  
}
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3DbMallocSize(sqlite3 *db, ଅଚଳ ଶୂନ୍ୟ *p){
  ପ୍ରତିଜ୍ଞା( p!=0 );
#ifdef SQLITE_DEBUG
  ଯଦି( db==0 ){
    ପ୍ରତିଜ୍ଞା( sqlite3MemdebugNoType(p, (u8)~MEMTYPE_HEAP) );
    ପ୍ରତିଜ୍ଞା( sqlite3MemdebugHasType(p, MEMTYPE_HEAP) );
  }ଅନ୍ୟଥା ଯଦି( !isLookaside(db,p) ){
    ପ୍ରତିଜ୍ଞା( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
    ପ୍ରତିଜ୍ଞା( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  }
#endif
  ଯଦି( db ){
    ଯଦି( ((uptr)p)<(uptr)(db->lookaside.pTrueEnd) ){
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
      ଯଦି( ((uptr)p)>=(uptr)(db->lookaside.p_ମଧ୍ୟ) ){
        ପ୍ରତିଜ୍ଞା( sqlite3_mutex_held(db->mutex) );
        ଫେରନ୍ତୁ LOOKASIDE_SMALL;
      }
#endif
      ଯଦି( ((uptr)p)>=(uptr)(db->lookaside.pStart) ){
        ପ୍ରତିଜ୍ଞା( sqlite3_mutex_held(db->mutex) );
        ଫେରନ୍ତୁ db->lookaside.szTrue;
      }
    }
  }
  ଫେରନ୍ତୁ sqlite3GlobalConfig.m.xSize((ଶୂନ୍ୟ*)p);
}
sqlite3_uint64 sqlite3_msize(ଶୂନ୍ୟ *p){
  ପ୍ରତିଜ୍ଞା( sqlite3MemdebugNoType(p, (u8)~MEMTYPE_HEAP) );
  ପ୍ରତିଜ୍ଞା( sqlite3MemdebugHasType(p, MEMTYPE_HEAP) );
  ଫେରନ୍ତୁ p ? sqlite3GlobalConfig.m.xSize(p) : 0;
}

/*
** Free memory previously obtained from sqlite3Malloc().
*/
ଶୂନ୍ୟ sqlite3_free(ଶୂନ୍ୟ *p){
  ଯଦି( p==0 ) ଫେରନ୍ତୁ;  /* IMP: R-49053-54554 */
  ପ୍ରତିଜ୍ଞା( sqlite3MemdebugHasType(p, MEMTYPE_HEAP) );
  ପ୍ରତିଜ୍ଞା( sqlite3MemdebugNoType(p, (u8)~MEMTYPE_HEAP) );
  ଯଦି( sqlite3GlobalConfig.bMemstat ){
    sqlite3_mutex_enter(mem0.mutex);
    sqlite3StatusDown(SQLITE_STATUS_MEMORY_USED, sqlite3MallocSize(p));
    sqlite3StatusDown(SQLITE_STATUS_MALLOC_ଗଣନା, 1);
    sqlite3GlobalConfig.m.xFree(p);
    sqlite3_mutex_leave(mem0.mutex);
  }ଅନ୍ୟଥା{
    sqlite3GlobalConfig.m.xFree(p);
  }
}

/*
** Add the size of memory allocation "p" to the count in
** *db->pnBytesFreed.
*/
ସ୍ଥିର SQLITE_NOINLINE ଶୂନ୍ୟ measureAllocationSize(sqlite3 *db, ଶୂନ୍ୟ *p){
  *db->pnBytesFreed += sqlite3DbMallocSize(db,p);
}

/*
** Free memory that might be associated with a particular database
** connection.  Calling sqlite3DbFree(D,X) for X==0 is a harmless no-op.
** The sqlite3DbFreeNN(D,X) version requires that X be non-NULL.
*/
ଶୂନ୍ୟ sqlite3DbFreeNN(sqlite3 *db, ଶୂନ୍ୟ *p){
  ପ୍ରତିଜ୍ଞା( db==0 || sqlite3_mutex_held(db->mutex) );
  ପ୍ରତିଜ୍ଞା( p!=0 );
  ଯଦି( db ){
    ଯଦି( ((uptr)p)<(uptr)(db->lookaside.pEnd) ){
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
      ଯଦି( ((uptr)p)>=(uptr)(db->lookaside.p_ମଧ୍ୟ) ){
        LookasideSlot *pBuf = (LookasideSlot*)p;
        ପ୍ରତିଜ୍ଞା( db->pnBytesFreed==0 );
#ifdef SQLITE_DEBUG
        memset(p, 0xaa, LOOKASIDE_SMALL);  /* Trash freed content */
#endif
        pBuf->pNext = db->lookaside.pSmallFree;
        db->lookaside.pSmallFree = pBuf;
        ଫେରନ୍ତୁ;
      }
#endif /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
      ଯଦି( ((uptr)p)>=(uptr)(db->lookaside.pStart) ){
        LookasideSlot *pBuf = (LookasideSlot*)p;
        ପ୍ରତିଜ୍ଞା( db->pnBytesFreed==0 );
#ifdef SQLITE_DEBUG
        memset(p, 0xaa, db->lookaside.szTrue);  /* Trash freed content */
#endif
        pBuf->pNext = db->lookaside.pFree;
        db->lookaside.pFree = pBuf;
        ଫେରନ୍ତୁ;
      }
    }
    ଯଦି( db->pnBytesFreed ){
      measureAllocationSize(db, p);
      ଫେରନ୍ତୁ;
    }
  }
  ପ୍ରତିଜ୍ଞା( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  ପ୍ରତିଜ୍ଞା( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  ପ୍ରତିଜ୍ଞା( db!=0 || sqlite3MemdebugNoType(p, MEMTYPE_LOOKASIDE) );
  sqlite3MemdebugSetType(p, MEMTYPE_HEAP);
  sqlite3_free(p);
}
ଶୂନ୍ୟ sqlite3DbNNFreeNN(sqlite3 *db, ଶୂନ୍ୟ *p){
  ପ୍ରତିଜ୍ଞା( db!=0 );
  ପ୍ରତିଜ୍ଞା( sqlite3_mutex_held(db->mutex) );
  ପ୍ରତିଜ୍ଞା( p!=0 );
  ଯଦି( ((uptr)p)<(uptr)(db->lookaside.pEnd) ){
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
    ଯଦି( ((uptr)p)>=(uptr)(db->lookaside.p_ମଧ୍ୟ) ){
      LookasideSlot *pBuf = (LookasideSlot*)p;
      ପ୍ରତିଜ୍ଞା( db->pnBytesFreed==0 );
#ifdef SQLITE_DEBUG
      memset(p, 0xaa, LOOKASIDE_SMALL);  /* Trash freed content */
#endif
      pBuf->pNext = db->lookaside.pSmallFree;
      db->lookaside.pSmallFree = pBuf;
      ଫେରନ୍ତୁ;
    }
#endif /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
    ଯଦି( ((uptr)p)>=(uptr)(db->lookaside.pStart) ){
      LookasideSlot *pBuf = (LookasideSlot*)p;
      ପ୍ରତିଜ୍ଞା( db->pnBytesFreed==0 );
#ifdef SQLITE_DEBUG
      memset(p, 0xaa, db->lookaside.szTrue);  /* Trash freed content */
#endif
      pBuf->pNext = db->lookaside.pFree;
      db->lookaside.pFree = pBuf;
      ଫେରନ୍ତୁ;
    }
  }
  ଯଦି( db->pnBytesFreed ){
    measureAllocationSize(db, p);
    ଫେରନ୍ତୁ;
  }
  ପ୍ରତିଜ୍ଞା( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  ପ୍ରତିଜ୍ଞା( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  sqlite3MemdebugSetType(p, MEMTYPE_HEAP);
  sqlite3_free(p);
}
ଶୂନ୍ୟ sqlite3DbFree(sqlite3 *db, ଶୂନ୍ୟ *p){
  ପ୍ରତିଜ୍ଞା( db==0 || sqlite3_mutex_held(db->mutex) );
  ଯଦି( p ) sqlite3DbFreeNN(db, p);
}

/*
** Change the size of an existing memory allocation
*/
ଶୂନ୍ୟ *sqlite3Realloc(ଶୂନ୍ୟ *pOld, u64 nBytes){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nOld, nNew, nDiff;
  ଶୂନ୍ୟ *pNew;
  ପ୍ରତିଜ୍ଞା( sqlite3MemdebugHasType(pOld, MEMTYPE_HEAP) );
  ପ୍ରତିଜ୍ଞା( sqlite3MemdebugNoType(pOld, (u8)~MEMTYPE_HEAP) );
  ଯଦି( pOld==0 ){
    ଫେରନ୍ତୁ sqlite3Malloc(nBytes); /* IMP: R-04300-56712 */
  }
  ଯଦି( nBytes==0 ){
    sqlite3_free(pOld); /* IMP: R-26507-47431 */
    ଫେରନ୍ତୁ 0;
  }
  ଯଦି( nBytes>SQLITE_MAX_ALLOCATION_SIZE ){
    ଫେରନ୍ତୁ 0;
  }
  nOld = sqlite3MallocSize(pOld);
  /* IMPLEMENTATION-OF: R-46199-30249 SQLite guarantees that the second
  ** argument to xRealloc is always a value returned by a prior call to
  ** xRoundup. */
  nNew = sqlite3GlobalConfig.m.xRoundup((ପୂର୍ଣ୍ଣସଂଖ୍ୟା)nBytes);
  ଯଦି( nOld==nNew ){
    pNew = pOld;
  }ଅନ୍ୟଥା ଯଦି( sqlite3GlobalConfig.bMemstat ){
    sqlite3_int64 nUsed;
    sqlite3_mutex_enter(mem0.mutex);
    sqlite3StatusHighwater(SQLITE_STATUS_MALLOC_SIZE, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)nBytes);
    nDiff = nNew - nOld;
    ଯଦି( nDiff>0 && (nUsed = sqlite3_Status_ମୂଲ୍ୟ(SQLITE_STATUS_MEMORY_USED)) >= 
          mem0.alarmThreshold-nDiff ){
      sqlite3MallocAlarm(nDiff);
      ଯଦି( mem0.hardLimit>0 && nUsed >= mem0.hardLimit - nDiff ){
        sqlite3_mutex_leave(mem0.mutex);
        test_oom_breakpoint(1);
        ଫେରନ୍ତୁ 0;
      }
    }
    pNew = sqlite3GlobalConfig.m.xRealloc(pOld, nNew);
#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
    ଯଦି( pNew==0 && mem0.alarmThreshold>0 ){
      sqlite3MallocAlarm((ପୂର୍ଣ୍ଣସଂଖ୍ୟା)nBytes);
      pNew = sqlite3GlobalConfig.m.xRealloc(pOld, nNew);
    }
#endif
    ଯଦି( pNew ){
      nNew = sqlite3MallocSize(pNew);
      sqlite3StatusUp(SQLITE_STATUS_MEMORY_USED, nNew-nOld);
    }
    sqlite3_mutex_leave(mem0.mutex);
  }ଅନ୍ୟଥା{
    pNew = sqlite3GlobalConfig.m.xRealloc(pOld, nNew);
  }
  ପ୍ରତିଜ୍ଞା( EIGHT_BYTE_ALIGNMENT(pNew) ); /* IMP: R-11148-40995 */
  ଫେରନ୍ତୁ pNew;
}

/*
** The public interface to sqlite3Realloc.  Make sure that the memory
** subsystem is initialized prior to invoking sqliteRealloc.
*/
ଶୂନ୍ୟ *sqlite3_realloc(ଶୂନ୍ୟ *pOld, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n){
#ifndef SQLITE_OMIT_AUTOINIT
  ଯଦି( sqlite3_initialize() ) ଫେରନ୍ତୁ 0;
#endif
  ଯଦି( n<0 ) n = 0;  /* IMP: R-26507-47431 */
  ଫେରନ୍ତୁ sqlite3Realloc(pOld, n);
}
ଶୂନ୍ୟ *sqlite3_realloc64(ଶୂନ୍ୟ *pOld, sqlite3_uint64 n){
#ifndef SQLITE_OMIT_AUTOINIT
  ଯଦି( sqlite3_initialize() ) ଫେରନ୍ତୁ 0;
#endif
  ଫେରନ୍ତୁ sqlite3Realloc(pOld, n);
}


/*
** Allocate and zero memory.
*/ 
ଶୂନ୍ୟ *sqlite3MallocZero(u64 n){
  ଶୂନ୍ୟ *p = sqlite3Malloc(n);
  ଯଦି( p ){
    memset(p, 0, (size_t)n);
  }
  ଫେରନ୍ତୁ p;
}

/*
** Allocate and zero memory.  If the allocation fails, make
** the mallocFailed flag in the connection pointer.
*/
ଶୂନ୍ୟ *sqlite3DbMallocZero(sqlite3 *db, u64 n){
  ଶୂନ୍ୟ *p;
  testcase( db==0 );
  p = sqlite3DbMallocRaw(db, n);
  ଯଦି( p ) memset(p, 0, (size_t)n);
  ଫେରନ୍ତୁ p;
}


/* Finish the work of sqlite3DbMallocRawNN for the unusual and
** slower case when the allocation cannot be fulfilled using lookaside.
*/
ସ୍ଥିର SQLITE_NOINLINE ଶୂନ୍ୟ *dbMallocRawFinish(sqlite3 *db, u64 n){
  ଶୂନ୍ୟ *p;
  ପ୍ରତିଜ୍ଞା( db!=0 );
  p = sqlite3Malloc(n);
  ଯଦି( !p ) sqlite3OomFault(db);
  sqlite3MemdebugSetType(p, 
         (db->lookaside.bDisable==0) ? MEMTYPE_LOOKASIDE : MEMTYPE_HEAP);
  ଫେରନ୍ତୁ p;
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
ଶୂନ୍ୟ *sqlite3DbMallocRaw(sqlite3 *db, u64 n){
  ଶୂନ୍ୟ *p;
  ଯଦି( db ) ଫେରନ୍ତୁ sqlite3DbMallocRawNN(db, n);
  p = sqlite3Malloc(n);
  sqlite3MemdebugSetType(p, MEMTYPE_HEAP);
  ଫେରନ୍ତୁ p;
}
ଶୂନ୍ୟ *sqlite3DbMallocRawNN(sqlite3 *db, u64 n){
#ifndef SQLITE_OMIT_LOOKASIDE
  LookasideSlot *pBuf;
  ପ୍ରତିଜ୍ଞା( db!=0 );
  ପ୍ରତିଜ୍ଞା( sqlite3_mutex_held(db->mutex) );
  ପ୍ରତିଜ୍ଞା( db->pnBytesFreed==0 );
  ଯଦି( n>db->lookaside.sz ){
    ଯଦି( !db->lookaside.bDisable ){
      db->lookaside.anStat[1]++;      
    }ଅନ୍ୟଥା ଯଦି( db->mallocFailed ){
      ଫେରନ୍ତୁ 0;
    }
    ଫେରନ୍ତୁ dbMallocRawFinish(db, n);
  }
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
  ଯଦି( n<=LOOKASIDE_SMALL ){
    ଯଦି( (pBuf = db->lookaside.pSmallFree)!=0 ){
      db->lookaside.pSmallFree = pBuf->pNext;
      db->lookaside.anStat[0]++;
      ଫେରନ୍ତୁ (ଶୂନ୍ୟ*)pBuf;
    }ଅନ୍ୟଥା ଯଦି( (pBuf = db->lookaside.pSmallInit)!=0 ){
      db->lookaside.pSmallInit = pBuf->pNext;
      db->lookaside.anStat[0]++;
      ଫେରନ୍ତୁ (ଶୂନ୍ୟ*)pBuf;
    }
  }
#endif
  ଯଦି( (pBuf = db->lookaside.pFree)!=0 ){
    db->lookaside.pFree = pBuf->pNext;
    db->lookaside.anStat[0]++;
    ଫେରନ୍ତୁ (ଶୂନ୍ୟ*)pBuf;
  }ଅନ୍ୟଥା ଯଦି( (pBuf = db->lookaside.pInit)!=0 ){
    db->lookaside.pInit = pBuf->pNext;
    db->lookaside.anStat[0]++;
    ଫେରନ୍ତୁ (ଶୂନ୍ୟ*)pBuf;
  }ଅନ୍ୟଥା{
    db->lookaside.anStat[2]++;
  }
#ଅନ୍ୟଥା
  ପ୍ରତିଜ୍ଞା( db!=0 );
  ପ୍ରତିଜ୍ଞା( sqlite3_mutex_held(db->mutex) );
  ପ୍ରତିଜ୍ଞା( db->pnBytesFreed==0 );
  ଯଦି( db->mallocFailed ){
    ଫେରନ୍ତୁ 0;
  }
#endif
  ଫେରନ୍ତୁ dbMallocRawFinish(db, n);
}

/* Forward declaration */
ସ୍ଥିର SQLITE_NOINLINE ଶୂନ୍ୟ *dbReallocFinish(sqlite3 *db, ଶୂନ୍ୟ *p, u64 n);

/*
** Resize the block of memory pointed to by p to n bytes. If the
** resize fails, set the mallocFailed flag in the connection object.
*/
ଶୂନ୍ୟ *sqlite3DbRealloc(sqlite3 *db, ଶୂନ୍ୟ *p, u64 n){
  ପ୍ରତିଜ୍ଞା( db!=0 );
  ଯଦି( p==0 ) ଫେରନ୍ତୁ sqlite3DbMallocRawNN(db, n);
  ପ୍ରତିଜ୍ଞା( sqlite3_mutex_held(db->mutex) );
  ଯଦି( ((uptr)p)<(uptr)db->lookaside.pEnd ){
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
    ଯଦି( ((uptr)p)>=(uptr)db->lookaside.p_ମଧ୍ୟ ){
      ଯଦି( n<=LOOKASIDE_SMALL ) ଫେରନ୍ତୁ p;
    }ଅନ୍ୟଥା
#endif
    ଯଦି( ((uptr)p)>=(uptr)db->lookaside.pStart ){
      ଯଦି( n<=db->lookaside.szTrue ) ଫେରନ୍ତୁ p;
    }
  }
  ଫେରନ୍ତୁ dbReallocFinish(db, p, n);
}
ସ୍ଥିର SQLITE_NOINLINE ଶୂନ୍ୟ *dbReallocFinish(sqlite3 *db, ଶୂନ୍ୟ *p, u64 n){
  ଶୂନ୍ୟ *pNew = 0;
  ପ୍ରତିଜ୍ଞା( db!=0 );
  ପ୍ରତିଜ୍ଞା( p!=0 );
  ଯଦି( db->mallocFailed==0 ){
    ଯଦି( isLookaside(db, p) ){
      pNew = sqlite3DbMallocRawNN(db, n);
      ଯଦି( pNew ){
        memcpy(pNew, p, lookasideMallocSize(db, p));
        sqlite3DbFree(db, p);
      }
    }ଅନ୍ୟଥା{
      ପ୍ରତିଜ୍ଞା( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
      ପ୍ରତିଜ୍ଞା( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
      sqlite3MemdebugSetType(p, MEMTYPE_HEAP);
      pNew = sqlite3Realloc(p, n);
      ଯଦି( !pNew ){
        sqlite3OomFault(db);
      }
      sqlite3MemdebugSetType(pNew,
            (db->lookaside.bDisable==0 ? MEMTYPE_LOOKASIDE : MEMTYPE_HEAP));
    }
  }
  ଫେରନ୍ତୁ pNew;
}

/*
** Attempt to reallocate p.  If the reallocation fails, then free p
** and set the mallocFailed flag in the database connection.
*/
ଶୂନ୍ୟ *sqlite3DbReallocOrFree(sqlite3 *db, ଶୂନ୍ୟ *p, u64 n){
  ଶୂନ୍ୟ *pNew;
  pNew = sqlite3DbRealloc(db, p, n);
  ଯଦି( !pNew ){
    sqlite3DbFree(db, p);
  }
  ଫେରନ୍ତୁ pNew;
}

/*
** Make a copy of a string in memory obtained from sqliteMalloc(). These 
** functions call sqlite3MallocRaw() directly instead of sqliteMalloc(). This
** is because when memory debugging is turned on, these two functions are 
** called via macros that record the current file and line number in the
** ThreadData structure.
*/
ଅକ୍ଷର *sqlite3DbStrDup(sqlite3 *db, ଅଚଳ ଅକ୍ଷର *z){
  ଅକ୍ଷର *zNew;
  size_t n;
  ଯଦି( z==0 ){
    ଫେରନ୍ତୁ 0;
  }
  n = strlen(z) + 1;
  zNew = sqlite3DbMallocRaw(db, n);
  ଯଦି( zNew ){
    memcpy(zNew, z, n);
  }
  ଫେରନ୍ତୁ zNew;
}
ଅକ୍ଷର *sqlite3DbStrNDup(sqlite3 *db, ଅଚଳ ଅକ୍ଷର *z, u64 n){
  ଅକ୍ଷର *zNew;
  ପ୍ରତିଜ୍ଞା( db!=0 );
  ପ୍ରତିଜ୍ଞା( z!=0 || n==0 );
  ପ୍ରତିଜ୍ଞା( (n&0x7fffffff)==n );
  zNew = z ? sqlite3DbMallocRawNN(db, n+1) : 0;
  ଯଦି( zNew ){
    memcpy(zNew, z, (size_t)n);
    zNew[n] = 0;
  }
  ଫେରନ୍ତୁ zNew;
}

/*
** The text between zStart and zEnd represents a phrase within a larger
** SQL statement.  Make a copy of this phrase in space obtained form
** sqlite3DbMalloc().  Omit leading and trailing whitespace.
*/
ଅକ୍ଷର *sqlite3DbSpanDup(sqlite3 *db, ଅଚଳ ଅକ୍ଷର *zStart, ଅଚଳ ଅକ୍ଷର *zEnd){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n;
#ifdef SQLITE_DEBUG
  /* Because of the way the parser works, the span is guaranteed to contain
  ** at least one non-space character */
  ପାଇଁ(n=0; sqlite3Isspace(zStart[n]); n++){ ପ୍ରତିଜ୍ଞା( &zStart[n]<zEnd ); }
#endif
  ଯେତେବେଳେ( sqlite3Isspace(zStart[0]) ) zStart++;
  n = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(zEnd - zStart);
  ଯେତେବେଳେ( sqlite3Isspace(zStart[n-1]) ) n--;
  ଫେରନ୍ତୁ sqlite3DbStrNDup(db, zStart, n);
}

/*
** Free any prior content in *pz and replace it with a copy of zNew.
*/
ଶୂନ୍ୟ sqlite3SetString(ଅକ୍ଷର **pz, sqlite3 *db, ଅଚଳ ଅକ୍ଷର *zNew){
  ଅକ୍ଷର *z = sqlite3DbStrDup(db, zNew);
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
ଶୂନ୍ୟ *sqlite3OomFault(sqlite3 *db){
  ଯଦି( db->mallocFailed==0 && db->bBenignMalloc==0 ){
    db->mallocFailed = 1;
    ଯଦି( db->nVdbeExec>0 ){
      AtomicStore(&db->u1.isInterrupted, 1);
    }
    DisableLookaside;
    ଯଦି( db->pParse ){
      Parse *pParse;
      sqlite3_Error_ସନ୍ଦେଶ(db->pParse, "out of memory");
      db->pParse->rc = SQLITE_NOMEM_BKPT;
      ପାଇଁ(pParse=db->pParse->pOuterParse; pParse; pParse = pParse->pOuterParse){
        pParse->nErr++;
        pParse->rc = SQLITE_NOMEM;
      } 
    }
  }
  ଫେରନ୍ତୁ 0;
}

/*
** This routine reactivates the memory allocator and clears the
** db->mallocFailed flag as necessary.
**
** The memory allocator is not restarted if there are running
** VDBEs.
*/
ଶୂନ୍ୟ sqlite3OomClear(sqlite3 *db){
  ଯଦି( db->mallocFailed && db->nVdbeExec==0 ){
    db->mallocFailed = 0;
    AtomicStore(&db->u1.isInterrupted, 0);
    ପ୍ରତିଜ୍ଞା( db->lookaside.bDisable>0 );
    EnableLookaside;
  }
}

/*
** Take actions at the end of an API call to deal with error codes.
*/
ସ୍ଥିର SQLITE_NOINLINE ପୂର୍ଣ୍ଣସଂଖ୍ୟା apiHandleError(sqlite3 *db, ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc){
  ଯଦି( db->mallocFailed || rc==SQLITE_IOERR_NOMEM ){
    sqlite3OomClear(db);
    sqlite3Error(db, SQLITE_NOMEM);
    ଫେରନ୍ତୁ SQLITE_NOMEM_BKPT;
  }
  ଫେରନ୍ତୁ rc & db->errMask;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3ApiExit(sqlite3* db, ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc){
  /* If the db handle must hold the connection handle mutex here.
  ** Otherwise the read (and possible write) of db->mallocFailed 
  ** is unsafe, as is the call to sqlite3Error().
  */
  ପ୍ରତିଜ୍ଞା( db!=0 );
  ପ୍ରତିଜ୍ଞା( sqlite3_mutex_held(db->mutex) );
  ଯଦି( db->mallocFailed || rc ){
    ଫେରନ୍ତୁ apiHandleError(db, rc);
  }
  ଫେରନ୍ତୁ 0;
}
