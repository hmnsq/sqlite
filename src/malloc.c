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
#അന്തര്ഭാവ "sqliteInt.h"
#അന്തര്ഭാവ <stdarg.h>

/*
** Attempt to release up to n bytes of non-essential memory currently
** held by SQLite. An example of non-essential memory is memory used to
** cache database pages that are not currently in use.
*/
പൂർണ്ണസംഖ്യ sqlite3_release_memory(പൂർണ്ണസംഖ്യ n){
#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
  തിരികെ sqlite3PcacheReleaseMemory(n);
#അല്ലെങ്കിൽ
  /* IMPLEMENTATION-OF: R-34391-24921 The sqlite3_release_memory() routine
  ** is a no-op returning zero if SQLite is not compiled with
  ** SQLITE_ENABLE_MEMORY_MANAGEMENT. */
  UNUSED_PARAMETER(n);
  തിരികെ 0;
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
സ്ഥിതം SQLITE_WSD ഘടന Mem0Global {
  sqlite3_mutex *mutex;         /* Mutex to serialize access */
  sqlite3_int64 alarmThreshold; /* The soft heap limit */
  sqlite3_int64 hardLimit;      /* The hard upper bound on memory */

  /*
  ** True if heap is nearly "full" where "full" is defined by the
  ** sqlite3_soft_heap_limit() setting.
  */
  പൂർണ്ണസംഖ്യ nearlyFull;
} mem0 = { 0, SQLITE_MAX_MEMORY, SQLITE_MAX_MEMORY, 0 };

#define mem0 GLOBAL(ഘടന Mem0Global, mem0)

/*
** Return the memory allocator mutex. sqlite3_status() needs it.
*/
sqlite3_mutex *sqlite3MallocMutex(ശൂന്യം){
  തിരികെ mem0.mutex;
}

#ifndef SQLITE_OMIT_DEPRECATED
/*
** Deprecated external interface.  It used to set an alarm callback
** that was invoked when memory usage grew too large.  Now it is a
** no-op.
*/
പൂർണ്ണസംഖ്യ sqlite3_memory_alarm(
  ശൂന്യം(*xCallback)(ശൂന്യം *pArg, sqlite3_int64 used,പൂർണ്ണസംഖ്യ N),
  ശൂന്യം *pArg,
  sqlite3_int64 iThreshold
){
  (ശൂന്യം)xCallback;
  (ശൂന്യം)pArg;
  (ശൂന്യം)iThreshold;
  തിരികെ SQLITE_OK;
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
  പൂർണ്ണസംഖ്യ rc = sqlite3_initialize();
  എങ്കിൽ( rc ) തിരികെ -1;
#endif
  sqlite3_mutex_enter(mem0.mutex);
  priorLimit = mem0.alarmThreshold;
  എങ്കിൽ( n<0 ){
    sqlite3_mutex_leave(mem0.mutex);
    തിരികെ priorLimit;
  }
  എങ്കിൽ( mem0.hardLimit>0 && (n>mem0.hardLimit || n==0) ){
    n = mem0.hardLimit;
  }
  mem0.alarmThreshold = n;
  nUsed = sqlite3_Status_മൂല്യം(SQLITE_STATUS_MEMORY_USED);
  AtomicStore(&mem0.nearlyFull, n>0 && n<=nUsed);
  sqlite3_mutex_leave(mem0.mutex);
  excess = sqlite3_memory_used() - n;
  എങ്കിൽ( excess>0 ) sqlite3_release_memory((പൂർണ്ണസംഖ്യ)(excess & 0x7fffffff));
  തിരികെ priorLimit;
}
ശൂന്യം sqlite3_soft_heap_limit(പൂർണ്ണസംഖ്യ n){
  എങ്കിൽ( n<0 ) n = 0;
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
  പൂർണ്ണസംഖ്യ rc = sqlite3_initialize();
  എങ്കിൽ( rc ) തിരികെ -1;
#endif
  sqlite3_mutex_enter(mem0.mutex);
  priorLimit = mem0.hardLimit;
  എങ്കിൽ( n>=0 ){
    mem0.hardLimit = n;
    എങ്കിൽ( n<mem0.alarmThreshold || mem0.alarmThreshold==0 ){
      mem0.alarmThreshold = n;
    }
  }
  sqlite3_mutex_leave(mem0.mutex);
  തിരികെ priorLimit;
}


/*
** Initialize the memory allocation subsystem.
*/
പൂർണ്ണസംഖ്യ sqlite3MallocInit(ശൂന്യം){
  പൂർണ്ണസംഖ്യ rc;
  എങ്കിൽ( sqlite3GlobalConfig.m.xMalloc==0 ){
    sqlite3MemSetDefault();
  }
  mem0.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM);
  എങ്കിൽ( sqlite3GlobalConfig.pPage==0 || sqlite3GlobalConfig.szPage<512
      || sqlite3GlobalConfig.nPage<=0 ){
    sqlite3GlobalConfig.pPage = 0;
    sqlite3GlobalConfig.szPage = 0;
  }
  rc = sqlite3GlobalConfig.m.xInit(sqlite3GlobalConfig.m.pAppData);
  എങ്കിൽ( rc!=SQLITE_OK ) memset(&mem0, 0, വലുപ്പം(mem0));
  തിരികെ rc;
}

/*
** Return true if the heap is currently under memory pressure - in other
** words if the amount of heap used is close to the limit set by
** sqlite3_soft_heap_limit().
*/
പൂർണ്ണസംഖ്യ sqlite3HeapNearlyFull(ശൂന്യം){
  തിരികെ AtomicLoad(&mem0.nearlyFull);
}

/*
** Deinitialize the memory allocation subsystem.
*/
ശൂന്യം sqlite3MallocEnd(ശൂന്യം){
  എങ്കിൽ( sqlite3GlobalConfig.m.xShutdown ){
    sqlite3GlobalConfig.m.xShutdown(sqlite3GlobalConfig.m.pAppData);
  }
  memset(&mem0, 0, വലുപ്പം(mem0));
}

/*
** Return the amount of memory currently checked out.
*/
sqlite3_int64 sqlite3_memory_used(ശൂന്യം){
  sqlite3_int64 res, mx;
  sqlite3_status64(SQLITE_STATUS_MEMORY_USED, &res, &mx, 0);
  തിരികെ res;
}

/*
** Return the maximum amount of memory that has ever been
** checked out since either the beginning of this process
** or since the most recent reset.
*/
sqlite3_int64 sqlite3_memory_highwater(പൂർണ്ണസംഖ്യ resetFlag){
  sqlite3_int64 res, mx;
  sqlite3_status64(SQLITE_STATUS_MEMORY_USED, &res, &mx, resetFlag);
  തിരികെ mx;
}

/*
** Trigger the alarm 
*/
സ്ഥിതം ശൂന്യം sqlite3MallocAlarm(പൂർണ്ണസംഖ്യ nByte){
  എങ്കിൽ( mem0.alarmThreshold<=0 ) തിരികെ;
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
സ്ഥിതം SQLITE_NOINLINE ശൂന്യം test_oom_breakpoint(u64 n){
  സ്ഥിതം u64 nOomFault = 0;
  nOomFault += n;
  /* The assert() is never reached in a human lifetime.  It  is here mostly
  ** to prevent code optimizers from optimizing out this function. */
  പ്രതിജ്ഞാ( (nOomFault>>32) < 0xffffffff );
}
#അല്ലെങ്കിൽ
# define test_oom_breakpoint(X)   /* No-op for production builds */
#endif

/*
** Do a memory allocation with statistics and alarms.  Assume the
** lock is already held.
*/
സ്ഥിതം ശൂന്യം mallocWithAlarm(പൂർണ്ണസംഖ്യ n, ശൂന്യം **pp){
  ശൂന്യം *p;
  പൂർണ്ണസംഖ്യ nFull;
  പ്രതിജ്ഞാ( sqlite3_mutex_held(mem0.mutex) );
  പ്രതിജ്ഞാ( n>0 );

  /* In Firefox (circa 2017-02-08), xRoundup() is remapped to an internal
  ** implementation of malloc_good_size(), which must be called in debug
  ** mode and specifically when the DMD "Dark Matter Detector" is enabled
  ** or else a crash results.  Hence, do not attempt to optimize out the
  ** following xRoundup() call. */
  nFull = sqlite3GlobalConfig.m.xRoundup(n);

  sqlite3StatusHighwater(SQLITE_STATUS_MALLOC_SIZE, n);
  എങ്കിൽ( mem0.alarmThreshold>0 ){
    sqlite3_int64 nUsed = sqlite3_Status_മൂല്യം(SQLITE_STATUS_MEMORY_USED);
    എങ്കിൽ( nUsed >= mem0.alarmThreshold - nFull ){
      AtomicStore(&mem0.nearlyFull, 1);
      sqlite3MallocAlarm(nFull);
      എങ്കിൽ( mem0.hardLimit ){
        nUsed = sqlite3_Status_മൂല്യം(SQLITE_STATUS_MEMORY_USED);
        എങ്കിൽ( nUsed >= mem0.hardLimit - nFull ){
          test_oom_breakpoint(1);
          *pp = 0;
          തിരികെ;
        }
      }
    }അല്ലെങ്കിൽ{
      AtomicStore(&mem0.nearlyFull, 0);
    }
  }
  p = sqlite3GlobalConfig.m.xMalloc(nFull);
#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
  എങ്കിൽ( p==0 && mem0.alarmThreshold>0 ){
    sqlite3MallocAlarm(nFull);
    p = sqlite3GlobalConfig.m.xMalloc(nFull);
  }
#endif
  എങ്കിൽ( p ){
    nFull = sqlite3MallocSize(p);
    sqlite3StatusUp(SQLITE_STATUS_MEMORY_USED, nFull);
    sqlite3StatusUp(SQLITE_STATUS_MALLOC_എണ്ണം, 1);
  }
  *pp = p;
}

/*
** Allocate memory.  This routine is like sqlite3_malloc() except that it
** assumes the memory subsystem has already been initialized.
*/
ശൂന്യം *sqlite3Malloc(u64 n){
  ശൂന്യം *p;
  എങ്കിൽ( n==0 || n>SQLITE_MAX_ALLOCATION_SIZE ){
    p = 0;
  }അല്ലെങ്കിൽ എങ്കിൽ( sqlite3GlobalConfig.bMemstat ){
    sqlite3_mutex_enter(mem0.mutex);
    mallocWithAlarm((പൂർണ്ണസംഖ്യ)n, &p);
    sqlite3_mutex_leave(mem0.mutex);
  }അല്ലെങ്കിൽ{
    p = sqlite3GlobalConfig.m.xMalloc((പൂർണ്ണസംഖ്യ)n);
  }
  പ്രതിജ്ഞാ( EIGHT_BYTE_ALIGNMENT(p) );  /* IMP: R-11148-40995 */
  തിരികെ p;
}

/*
** This version of the memory allocation is for use by the application.
** First make sure the memory subsystem is initialized, then do the
** allocation.
*/
ശൂന്യം *sqlite3_malloc(പൂർണ്ണസംഖ്യ n){
#ifndef SQLITE_OMIT_AUTOINIT
  എങ്കിൽ( sqlite3_initialize() ) തിരികെ 0;
#endif
  തിരികെ n<=0 ? 0 : sqlite3Malloc(n);
}
ശൂന്യം *sqlite3_malloc64(sqlite3_uint64 n){
#ifndef SQLITE_OMIT_AUTOINIT
  എങ്കിൽ( sqlite3_initialize() ) തിരികെ 0;
#endif
  തിരികെ sqlite3Malloc(n);
}

/*
** TRUE if p is a lookaside memory allocation from db
*/
#ifndef SQLITE_OMIT_LOOKASIDE
സ്ഥിതം പൂർണ്ണസംഖ്യ isLookaside(sqlite3 *db, സ്ഥിരം ശൂന്യം *p){
  തിരികെ SQLITE_WITHIN(p, db->lookaside.pStart, db->lookaside.pTrueEnd);
}
#അല്ലെങ്കിൽ
#define isLookaside(A,B) 0
#endif

/*
** Return the size of a memory allocation previously obtained from
** sqlite3Malloc() or sqlite3_malloc().
*/
പൂർണ്ണസംഖ്യ sqlite3MallocSize(സ്ഥിരം ശൂന്യം *p){
  പ്രതിജ്ഞാ( sqlite3MemdebugHasType(p, MEMTYPE_HEAP) );
  തിരികെ sqlite3GlobalConfig.m.xSize((ശൂന്യം*)p);
}
സ്ഥിതം പൂർണ്ണസംഖ്യ lookasideMallocSize(sqlite3 *db, സ്ഥിരം ശൂന്യം *p){
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE    
  തിരികെ p<db->lookaside.p_നടുവ് ? db->lookaside.szTrue : LOOKASIDE_SMALL;
#അല്ലെങ്കിൽ
  തിരികെ db->lookaside.szTrue;
#endif  
}
പൂർണ്ണസംഖ്യ sqlite3DbMallocSize(sqlite3 *db, സ്ഥിരം ശൂന്യം *p){
  പ്രതിജ്ഞാ( p!=0 );
#ifdef SQLITE_DEBUG
  എങ്കിൽ( db==0 ){
    പ്രതിജ്ഞാ( sqlite3MemdebugNoType(p, (u8)~MEMTYPE_HEAP) );
    പ്രതിജ്ഞാ( sqlite3MemdebugHasType(p, MEMTYPE_HEAP) );
  }അല്ലെങ്കിൽ എങ്കിൽ( !isLookaside(db,p) ){
    പ്രതിജ്ഞാ( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
    പ്രതിജ്ഞാ( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  }
#endif
  എങ്കിൽ( db ){
    എങ്കിൽ( ((uptr)p)<(uptr)(db->lookaside.pTrueEnd) ){
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
      എങ്കിൽ( ((uptr)p)>=(uptr)(db->lookaside.p_നടുവ്) ){
        പ്രതിജ്ഞാ( sqlite3_mutex_held(db->mutex) );
        തിരികെ LOOKASIDE_SMALL;
      }
#endif
      എങ്കിൽ( ((uptr)p)>=(uptr)(db->lookaside.pStart) ){
        പ്രതിജ്ഞാ( sqlite3_mutex_held(db->mutex) );
        തിരികെ db->lookaside.szTrue;
      }
    }
  }
  തിരികെ sqlite3GlobalConfig.m.xSize((ശൂന്യം*)p);
}
sqlite3_uint64 sqlite3_msize(ശൂന്യം *p){
  പ്രതിജ്ഞാ( sqlite3MemdebugNoType(p, (u8)~MEMTYPE_HEAP) );
  പ്രതിജ്ഞാ( sqlite3MemdebugHasType(p, MEMTYPE_HEAP) );
  തിരികെ p ? sqlite3GlobalConfig.m.xSize(p) : 0;
}

/*
** Free memory previously obtained from sqlite3Malloc().
*/
ശൂന്യം sqlite3_free(ശൂന്യം *p){
  എങ്കിൽ( p==0 ) തിരികെ;  /* IMP: R-49053-54554 */
  പ്രതിജ്ഞാ( sqlite3MemdebugHasType(p, MEMTYPE_HEAP) );
  പ്രതിജ്ഞാ( sqlite3MemdebugNoType(p, (u8)~MEMTYPE_HEAP) );
  എങ്കിൽ( sqlite3GlobalConfig.bMemstat ){
    sqlite3_mutex_enter(mem0.mutex);
    sqlite3StatusDown(SQLITE_STATUS_MEMORY_USED, sqlite3MallocSize(p));
    sqlite3StatusDown(SQLITE_STATUS_MALLOC_എണ്ണം, 1);
    sqlite3GlobalConfig.m.xFree(p);
    sqlite3_mutex_leave(mem0.mutex);
  }അല്ലെങ്കിൽ{
    sqlite3GlobalConfig.m.xFree(p);
  }
}

/*
** Add the size of memory allocation "p" to the count in
** *db->pnBytesFreed.
*/
സ്ഥിതം SQLITE_NOINLINE ശൂന്യം measureAllocationSize(sqlite3 *db, ശൂന്യം *p){
  *db->pnBytesFreed += sqlite3DbMallocSize(db,p);
}

/*
** Free memory that might be associated with a particular database
** connection.  Calling sqlite3DbFree(D,X) for X==0 is a harmless no-op.
** The sqlite3DbFreeNN(D,X) version requires that X be non-NULL.
*/
ശൂന്യം sqlite3DbFreeNN(sqlite3 *db, ശൂന്യം *p){
  പ്രതിജ്ഞാ( db==0 || sqlite3_mutex_held(db->mutex) );
  പ്രതിജ്ഞാ( p!=0 );
  എങ്കിൽ( db ){
    എങ്കിൽ( ((uptr)p)<(uptr)(db->lookaside.pEnd) ){
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
      എങ്കിൽ( ((uptr)p)>=(uptr)(db->lookaside.p_നടുവ്) ){
        LookasideSlot *pBuf = (LookasideSlot*)p;
        പ്രതിജ്ഞാ( db->pnBytesFreed==0 );
#ifdef SQLITE_DEBUG
        memset(p, 0xaa, LOOKASIDE_SMALL);  /* Trash freed content */
#endif
        pBuf->pNext = db->lookaside.pSmallFree;
        db->lookaside.pSmallFree = pBuf;
        തിരികെ;
      }
#endif /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
      എങ്കിൽ( ((uptr)p)>=(uptr)(db->lookaside.pStart) ){
        LookasideSlot *pBuf = (LookasideSlot*)p;
        പ്രതിജ്ഞാ( db->pnBytesFreed==0 );
#ifdef SQLITE_DEBUG
        memset(p, 0xaa, db->lookaside.szTrue);  /* Trash freed content */
#endif
        pBuf->pNext = db->lookaside.pFree;
        db->lookaside.pFree = pBuf;
        തിരികെ;
      }
    }
    എങ്കിൽ( db->pnBytesFreed ){
      measureAllocationSize(db, p);
      തിരികെ;
    }
  }
  പ്രതിജ്ഞാ( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  പ്രതിജ്ഞാ( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  പ്രതിജ്ഞാ( db!=0 || sqlite3MemdebugNoType(p, MEMTYPE_LOOKASIDE) );
  sqlite3MemdebugSetType(p, MEMTYPE_HEAP);
  sqlite3_free(p);
}
ശൂന്യം sqlite3DbNNFreeNN(sqlite3 *db, ശൂന്യം *p){
  പ്രതിജ്ഞാ( db!=0 );
  പ്രതിജ്ഞാ( sqlite3_mutex_held(db->mutex) );
  പ്രതിജ്ഞാ( p!=0 );
  എങ്കിൽ( ((uptr)p)<(uptr)(db->lookaside.pEnd) ){
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
    എങ്കിൽ( ((uptr)p)>=(uptr)(db->lookaside.p_നടുവ്) ){
      LookasideSlot *pBuf = (LookasideSlot*)p;
      പ്രതിജ്ഞാ( db->pnBytesFreed==0 );
#ifdef SQLITE_DEBUG
      memset(p, 0xaa, LOOKASIDE_SMALL);  /* Trash freed content */
#endif
      pBuf->pNext = db->lookaside.pSmallFree;
      db->lookaside.pSmallFree = pBuf;
      തിരികെ;
    }
#endif /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
    എങ്കിൽ( ((uptr)p)>=(uptr)(db->lookaside.pStart) ){
      LookasideSlot *pBuf = (LookasideSlot*)p;
      പ്രതിജ്ഞാ( db->pnBytesFreed==0 );
#ifdef SQLITE_DEBUG
      memset(p, 0xaa, db->lookaside.szTrue);  /* Trash freed content */
#endif
      pBuf->pNext = db->lookaside.pFree;
      db->lookaside.pFree = pBuf;
      തിരികെ;
    }
  }
  എങ്കിൽ( db->pnBytesFreed ){
    measureAllocationSize(db, p);
    തിരികെ;
  }
  പ്രതിജ്ഞാ( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  പ്രതിജ്ഞാ( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  sqlite3MemdebugSetType(p, MEMTYPE_HEAP);
  sqlite3_free(p);
}
ശൂന്യം sqlite3DbFree(sqlite3 *db, ശൂന്യം *p){
  പ്രതിജ്ഞാ( db==0 || sqlite3_mutex_held(db->mutex) );
  എങ്കിൽ( p ) sqlite3DbFreeNN(db, p);
}

/*
** Change the size of an existing memory allocation
*/
ശൂന്യം *sqlite3Realloc(ശൂന്യം *pOld, u64 nBytes){
  പൂർണ്ണസംഖ്യ nOld, nNew, nDiff;
  ശൂന്യം *pNew;
  പ്രതിജ്ഞാ( sqlite3MemdebugHasType(pOld, MEMTYPE_HEAP) );
  പ്രതിജ്ഞാ( sqlite3MemdebugNoType(pOld, (u8)~MEMTYPE_HEAP) );
  എങ്കിൽ( pOld==0 ){
    തിരികെ sqlite3Malloc(nBytes); /* IMP: R-04300-56712 */
  }
  എങ്കിൽ( nBytes==0 ){
    sqlite3_free(pOld); /* IMP: R-26507-47431 */
    തിരികെ 0;
  }
  എങ്കിൽ( nBytes>SQLITE_MAX_ALLOCATION_SIZE ){
    തിരികെ 0;
  }
  nOld = sqlite3MallocSize(pOld);
  /* IMPLEMENTATION-OF: R-46199-30249 SQLite guarantees that the second
  ** argument to xRealloc is always a value returned by a prior call to
  ** xRoundup. */
  nNew = sqlite3GlobalConfig.m.xRoundup((പൂർണ്ണസംഖ്യ)nBytes);
  എങ്കിൽ( nOld==nNew ){
    pNew = pOld;
  }അല്ലെങ്കിൽ എങ്കിൽ( sqlite3GlobalConfig.bMemstat ){
    sqlite3_int64 nUsed;
    sqlite3_mutex_enter(mem0.mutex);
    sqlite3StatusHighwater(SQLITE_STATUS_MALLOC_SIZE, (പൂർണ്ണസംഖ്യ)nBytes);
    nDiff = nNew - nOld;
    എങ്കിൽ( nDiff>0 && (nUsed = sqlite3_Status_മൂല്യം(SQLITE_STATUS_MEMORY_USED)) >= 
          mem0.alarmThreshold-nDiff ){
      sqlite3MallocAlarm(nDiff);
      എങ്കിൽ( mem0.hardLimit>0 && nUsed >= mem0.hardLimit - nDiff ){
        sqlite3_mutex_leave(mem0.mutex);
        test_oom_breakpoint(1);
        തിരികെ 0;
      }
    }
    pNew = sqlite3GlobalConfig.m.xRealloc(pOld, nNew);
#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
    എങ്കിൽ( pNew==0 && mem0.alarmThreshold>0 ){
      sqlite3MallocAlarm((പൂർണ്ണസംഖ്യ)nBytes);
      pNew = sqlite3GlobalConfig.m.xRealloc(pOld, nNew);
    }
#endif
    എങ്കിൽ( pNew ){
      nNew = sqlite3MallocSize(pNew);
      sqlite3StatusUp(SQLITE_STATUS_MEMORY_USED, nNew-nOld);
    }
    sqlite3_mutex_leave(mem0.mutex);
  }അല്ലെങ്കിൽ{
    pNew = sqlite3GlobalConfig.m.xRealloc(pOld, nNew);
  }
  പ്രതിജ്ഞാ( EIGHT_BYTE_ALIGNMENT(pNew) ); /* IMP: R-11148-40995 */
  തിരികെ pNew;
}

/*
** The public interface to sqlite3Realloc.  Make sure that the memory
** subsystem is initialized prior to invoking sqliteRealloc.
*/
ശൂന്യം *sqlite3_realloc(ശൂന്യം *pOld, പൂർണ്ണസംഖ്യ n){
#ifndef SQLITE_OMIT_AUTOINIT
  എങ്കിൽ( sqlite3_initialize() ) തിരികെ 0;
#endif
  എങ്കിൽ( n<0 ) n = 0;  /* IMP: R-26507-47431 */
  തിരികെ sqlite3Realloc(pOld, n);
}
ശൂന്യം *sqlite3_realloc64(ശൂന്യം *pOld, sqlite3_uint64 n){
#ifndef SQLITE_OMIT_AUTOINIT
  എങ്കിൽ( sqlite3_initialize() ) തിരികെ 0;
#endif
  തിരികെ sqlite3Realloc(pOld, n);
}


/*
** Allocate and zero memory.
*/ 
ശൂന്യം *sqlite3MallocZero(u64 n){
  ശൂന്യം *p = sqlite3Malloc(n);
  എങ്കിൽ( p ){
    memset(p, 0, (size_t)n);
  }
  തിരികെ p;
}

/*
** Allocate and zero memory.  If the allocation fails, make
** the mallocFailed flag in the connection pointer.
*/
ശൂന്യം *sqlite3DbMallocZero(sqlite3 *db, u64 n){
  ശൂന്യം *p;
  testcase( db==0 );
  p = sqlite3DbMallocRaw(db, n);
  എങ്കിൽ( p ) memset(p, 0, (size_t)n);
  തിരികെ p;
}


/* Finish the work of sqlite3DbMallocRawNN for the unusual and
** slower case when the allocation cannot be fulfilled using lookaside.
*/
സ്ഥിതം SQLITE_NOINLINE ശൂന്യം *dbMallocRawFinish(sqlite3 *db, u64 n){
  ശൂന്യം *p;
  പ്രതിജ്ഞാ( db!=0 );
  p = sqlite3Malloc(n);
  എങ്കിൽ( !p ) sqlite3OomFault(db);
  sqlite3MemdebugSetType(p, 
         (db->lookaside.bDisable==0) ? MEMTYPE_LOOKASIDE : MEMTYPE_HEAP);
  തിരികെ p;
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
ശൂന്യം *sqlite3DbMallocRaw(sqlite3 *db, u64 n){
  ശൂന്യം *p;
  എങ്കിൽ( db ) തിരികെ sqlite3DbMallocRawNN(db, n);
  p = sqlite3Malloc(n);
  sqlite3MemdebugSetType(p, MEMTYPE_HEAP);
  തിരികെ p;
}
ശൂന്യം *sqlite3DbMallocRawNN(sqlite3 *db, u64 n){
#ifndef SQLITE_OMIT_LOOKASIDE
  LookasideSlot *pBuf;
  പ്രതിജ്ഞാ( db!=0 );
  പ്രതിജ്ഞാ( sqlite3_mutex_held(db->mutex) );
  പ്രതിജ്ഞാ( db->pnBytesFreed==0 );
  എങ്കിൽ( n>db->lookaside.sz ){
    എങ്കിൽ( !db->lookaside.bDisable ){
      db->lookaside.anStat[1]++;      
    }അല്ലെങ്കിൽ എങ്കിൽ( db->mallocFailed ){
      തിരികെ 0;
    }
    തിരികെ dbMallocRawFinish(db, n);
  }
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
  എങ്കിൽ( n<=LOOKASIDE_SMALL ){
    എങ്കിൽ( (pBuf = db->lookaside.pSmallFree)!=0 ){
      db->lookaside.pSmallFree = pBuf->pNext;
      db->lookaside.anStat[0]++;
      തിരികെ (ശൂന്യം*)pBuf;
    }അല്ലെങ്കിൽ എങ്കിൽ( (pBuf = db->lookaside.pSmallInit)!=0 ){
      db->lookaside.pSmallInit = pBuf->pNext;
      db->lookaside.anStat[0]++;
      തിരികെ (ശൂന്യം*)pBuf;
    }
  }
#endif
  എങ്കിൽ( (pBuf = db->lookaside.pFree)!=0 ){
    db->lookaside.pFree = pBuf->pNext;
    db->lookaside.anStat[0]++;
    തിരികെ (ശൂന്യം*)pBuf;
  }അല്ലെങ്കിൽ എങ്കിൽ( (pBuf = db->lookaside.pInit)!=0 ){
    db->lookaside.pInit = pBuf->pNext;
    db->lookaside.anStat[0]++;
    തിരികെ (ശൂന്യം*)pBuf;
  }അല്ലെങ്കിൽ{
    db->lookaside.anStat[2]++;
  }
#അല്ലെങ്കിൽ
  പ്രതിജ്ഞാ( db!=0 );
  പ്രതിജ്ഞാ( sqlite3_mutex_held(db->mutex) );
  പ്രതിജ്ഞാ( db->pnBytesFreed==0 );
  എങ്കിൽ( db->mallocFailed ){
    തിരികെ 0;
  }
#endif
  തിരികെ dbMallocRawFinish(db, n);
}

/* Forward declaration */
സ്ഥിതം SQLITE_NOINLINE ശൂന്യം *dbReallocFinish(sqlite3 *db, ശൂന്യം *p, u64 n);

/*
** Resize the block of memory pointed to by p to n bytes. If the
** resize fails, set the mallocFailed flag in the connection object.
*/
ശൂന്യം *sqlite3DbRealloc(sqlite3 *db, ശൂന്യം *p, u64 n){
  പ്രതിജ്ഞാ( db!=0 );
  എങ്കിൽ( p==0 ) തിരികെ sqlite3DbMallocRawNN(db, n);
  പ്രതിജ്ഞാ( sqlite3_mutex_held(db->mutex) );
  എങ്കിൽ( ((uptr)p)<(uptr)db->lookaside.pEnd ){
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
    എങ്കിൽ( ((uptr)p)>=(uptr)db->lookaside.p_നടുവ് ){
      എങ്കിൽ( n<=LOOKASIDE_SMALL ) തിരികെ p;
    }അല്ലെങ്കിൽ
#endif
    എങ്കിൽ( ((uptr)p)>=(uptr)db->lookaside.pStart ){
      എങ്കിൽ( n<=db->lookaside.szTrue ) തിരികെ p;
    }
  }
  തിരികെ dbReallocFinish(db, p, n);
}
സ്ഥിതം SQLITE_NOINLINE ശൂന്യം *dbReallocFinish(sqlite3 *db, ശൂന്യം *p, u64 n){
  ശൂന്യം *pNew = 0;
  പ്രതിജ്ഞാ( db!=0 );
  പ്രതിജ്ഞാ( p!=0 );
  എങ്കിൽ( db->mallocFailed==0 ){
    എങ്കിൽ( isLookaside(db, p) ){
      pNew = sqlite3DbMallocRawNN(db, n);
      എങ്കിൽ( pNew ){
        memcpy(pNew, p, lookasideMallocSize(db, p));
        sqlite3DbFree(db, p);
      }
    }അല്ലെങ്കിൽ{
      പ്രതിജ്ഞാ( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
      പ്രതിജ്ഞാ( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
      sqlite3MemdebugSetType(p, MEMTYPE_HEAP);
      pNew = sqlite3Realloc(p, n);
      എങ്കിൽ( !pNew ){
        sqlite3OomFault(db);
      }
      sqlite3MemdebugSetType(pNew,
            (db->lookaside.bDisable==0 ? MEMTYPE_LOOKASIDE : MEMTYPE_HEAP));
    }
  }
  തിരികെ pNew;
}

/*
** Attempt to reallocate p.  If the reallocation fails, then free p
** and set the mallocFailed flag in the database connection.
*/
ശൂന്യം *sqlite3DbReallocOrFree(sqlite3 *db, ശൂന്യം *p, u64 n){
  ശൂന്യം *pNew;
  pNew = sqlite3DbRealloc(db, p, n);
  എങ്കിൽ( !pNew ){
    sqlite3DbFree(db, p);
  }
  തിരികെ pNew;
}

/*
** Make a copy of a string in memory obtained from sqliteMalloc(). These 
** functions call sqlite3MallocRaw() directly instead of sqliteMalloc(). This
** is because when memory debugging is turned on, these two functions are 
** called via macros that record the current file and line number in the
** ThreadData structure.
*/
അക്ഷരം *sqlite3DbStrDup(sqlite3 *db, സ്ഥിരം അക്ഷരം *z){
  അക്ഷരം *zNew;
  size_t n;
  എങ്കിൽ( z==0 ){
    തിരികെ 0;
  }
  n = strlen(z) + 1;
  zNew = sqlite3DbMallocRaw(db, n);
  എങ്കിൽ( zNew ){
    memcpy(zNew, z, n);
  }
  തിരികെ zNew;
}
അക്ഷരം *sqlite3DbStrNDup(sqlite3 *db, സ്ഥിരം അക്ഷരം *z, u64 n){
  അക്ഷരം *zNew;
  പ്രതിജ്ഞാ( db!=0 );
  പ്രതിജ്ഞാ( z!=0 || n==0 );
  പ്രതിജ്ഞാ( (n&0x7fffffff)==n );
  zNew = z ? sqlite3DbMallocRawNN(db, n+1) : 0;
  എങ്കിൽ( zNew ){
    memcpy(zNew, z, (size_t)n);
    zNew[n] = 0;
  }
  തിരികെ zNew;
}

/*
** The text between zStart and zEnd represents a phrase within a larger
** SQL statement.  Make a copy of this phrase in space obtained form
** sqlite3DbMalloc().  Omit leading and trailing whitespace.
*/
അക്ഷരം *sqlite3DbSpanDup(sqlite3 *db, സ്ഥിരം അക്ഷരം *zStart, സ്ഥിരം അക്ഷരം *zEnd){
  പൂർണ്ണസംഖ്യ n;
#ifdef SQLITE_DEBUG
  /* Because of the way the parser works, the span is guaranteed to contain
  ** at least one non-space character */
  വേണ്ടി(n=0; sqlite3Isspace(zStart[n]); n++){ പ്രതിജ്ഞാ( &zStart[n]<zEnd ); }
#endif
  ആകുമ്പോൾ( sqlite3Isspace(zStart[0]) ) zStart++;
  n = (പൂർണ്ണസംഖ്യ)(zEnd - zStart);
  ആകുമ്പോൾ( sqlite3Isspace(zStart[n-1]) ) n--;
  തിരികെ sqlite3DbStrNDup(db, zStart, n);
}

/*
** Free any prior content in *pz and replace it with a copy of zNew.
*/
ശൂന്യം sqlite3SetString(അക്ഷരം **pz, sqlite3 *db, സ്ഥിരം അക്ഷരം *zNew){
  അക്ഷരം *z = sqlite3DbStrDup(db, zNew);
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
ശൂന്യം *sqlite3OomFault(sqlite3 *db){
  എങ്കിൽ( db->mallocFailed==0 && db->bBenignMalloc==0 ){
    db->mallocFailed = 1;
    എങ്കിൽ( db->nVdbeExec>0 ){
      AtomicStore(&db->u1.isInterrupted, 1);
    }
    DisableLookaside;
    എങ്കിൽ( db->pParse ){
      Parse *pParse;
      sqlite3_Error_സന്ദേശം(db->pParse, "out of memory");
      db->pParse->rc = SQLITE_NOMEM_BKPT;
      വേണ്ടി(pParse=db->pParse->pOuterParse; pParse; pParse = pParse->pOuterParse){
        pParse->nErr++;
        pParse->rc = SQLITE_NOMEM;
      } 
    }
  }
  തിരികെ 0;
}

/*
** This routine reactivates the memory allocator and clears the
** db->mallocFailed flag as necessary.
**
** The memory allocator is not restarted if there are running
** VDBEs.
*/
ശൂന്യം sqlite3OomClear(sqlite3 *db){
  എങ്കിൽ( db->mallocFailed && db->nVdbeExec==0 ){
    db->mallocFailed = 0;
    AtomicStore(&db->u1.isInterrupted, 0);
    പ്രതിജ്ഞാ( db->lookaside.bDisable>0 );
    EnableLookaside;
  }
}

/*
** Take actions at the end of an API call to deal with error codes.
*/
സ്ഥിതം SQLITE_NOINLINE പൂർണ്ണസംഖ്യ apiHandleError(sqlite3 *db, പൂർണ്ണസംഖ്യ rc){
  എങ്കിൽ( db->mallocFailed || rc==SQLITE_IOERR_NOMEM ){
    sqlite3OomClear(db);
    sqlite3Error(db, SQLITE_NOMEM);
    തിരികെ SQLITE_NOMEM_BKPT;
  }
  തിരികെ rc & db->errMask;
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
പൂർണ്ണസംഖ്യ sqlite3ApiExit(sqlite3* db, പൂർണ്ണസംഖ്യ rc){
  /* If the db handle must hold the connection handle mutex here.
  ** Otherwise the read (and possible write) of db->mallocFailed 
  ** is unsafe, as is the call to sqlite3Error().
  */
  പ്രതിജ്ഞാ( db!=0 );
  പ്രതിജ്ഞാ( sqlite3_mutex_held(db->mutex) );
  എങ്കിൽ( db->mallocFailed || rc ){
    തിരികെ apiHandleError(db, rc);
  }
  തിരികെ 0;
}
