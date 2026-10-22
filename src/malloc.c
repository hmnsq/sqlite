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
#અન્તર્ભાવ "sqliteInt.h"
#અન્તર્ભાવ <stdarg.h>

/*
** Attempt to release up to n bytes of non-essential memory currently
** held by SQLite. An example of non-essential memory is memory used to
** cache database pages that are not currently in use.
*/
પૂર્ણાંક sqlite3_release_memory(પૂર્ણાંક n){
#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
  પાછા sqlite3PcacheReleaseMemory(n);
#નહીંતર
  /* IMPLEMENTATION-OF: R-34391-24921 The sqlite3_release_memory() routine
  ** is a no-op returning zero if SQLite is not compiled with
  ** SQLITE_ENABLE_MEMORY_MANAGEMENT. */
  UNUSED_PARAMETER(n);
  પાછા 0;
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
સ્થિર SQLITE_WSD સંરચના Mem0Global {
  sqlite3_mutex *mutex;         /* Mutex to serialize access */
  sqlite3_int64 alarmThreshold; /* The soft heap limit */
  sqlite3_int64 hardLimit;      /* The hard upper bound on memory */

  /*
  ** True if heap is nearly "full" where "full" is defined by the
  ** sqlite3_soft_heap_limit() setting.
  */
  પૂર્ણાંક nearlyFull;
} mem0 = { 0, SQLITE_MAX_MEMORY, SQLITE_MAX_MEMORY, 0 };

#define mem0 GLOBAL(સંરચના Mem0Global, mem0)

/*
** Return the memory allocator mutex. sqlite3_status() needs it.
*/
sqlite3_mutex *sqlite3MallocMutex(શૂન્ય){
  પાછા mem0.mutex;
}

#ifndef SQLITE_OMIT_DEPRECATED
/*
** Deprecated external interface.  It used to set an alarm callback
** that was invoked when memory usage grew too large.  Now it is a
** no-op.
*/
પૂર્ણાંક sqlite3_memory_alarm(
  શૂન્ય(*xCallback)(શૂન્ય *pArg, sqlite3_int64 used,પૂર્ણાંક N),
  શૂન્ય *pArg,
  sqlite3_int64 iThreshold
){
  (શૂન્ય)xCallback;
  (શૂન્ય)pArg;
  (શૂન્ય)iThreshold;
  પાછા SQLITE_OK;
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
  પૂર્ણાંક rc = sqlite3_initialize();
  જો( rc ) પાછા -1;
#endif
  sqlite3_mutex_enter(mem0.mutex);
  priorLimit = mem0.alarmThreshold;
  જો( n<0 ){
    sqlite3_mutex_leave(mem0.mutex);
    પાછા priorLimit;
  }
  જો( mem0.hardLimit>0 && (n>mem0.hardLimit || n==0) ){
    n = mem0.hardLimit;
  }
  mem0.alarmThreshold = n;
  nUsed = sqlite3_Status_કિંમત(SQLITE_STATUS_MEMORY_USED);
  AtomicStore(&mem0.nearlyFull, n>0 && n<=nUsed);
  sqlite3_mutex_leave(mem0.mutex);
  excess = sqlite3_memory_used() - n;
  જો( excess>0 ) sqlite3_release_memory((પૂર્ણાંક)(excess & 0x7fffffff));
  પાછા priorLimit;
}
શૂન્ય sqlite3_soft_heap_limit(પૂર્ણાંક n){
  જો( n<0 ) n = 0;
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
  પૂર્ણાંક rc = sqlite3_initialize();
  જો( rc ) પાછા -1;
#endif
  sqlite3_mutex_enter(mem0.mutex);
  priorLimit = mem0.hardLimit;
  જો( n>=0 ){
    mem0.hardLimit = n;
    જો( n<mem0.alarmThreshold || mem0.alarmThreshold==0 ){
      mem0.alarmThreshold = n;
    }
  }
  sqlite3_mutex_leave(mem0.mutex);
  પાછા priorLimit;
}


/*
** Initialize the memory allocation subsystem.
*/
પૂર્ણાંક sqlite3MallocInit(શૂન્ય){
  પૂર્ણાંક rc;
  જો( sqlite3GlobalConfig.m.xMalloc==0 ){
    sqlite3MemSetDefault();
  }
  mem0.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM);
  જો( sqlite3GlobalConfig.pPage==0 || sqlite3GlobalConfig.szPage<512
      || sqlite3GlobalConfig.nPage<=0 ){
    sqlite3GlobalConfig.pPage = 0;
    sqlite3GlobalConfig.szPage = 0;
  }
  rc = sqlite3GlobalConfig.m.xInit(sqlite3GlobalConfig.m.pAppData);
  જો( rc!=SQLITE_OK ) memset(&mem0, 0, કદ(mem0));
  પાછા rc;
}

/*
** Return true if the heap is currently under memory pressure - in other
** words if the amount of heap used is close to the limit set by
** sqlite3_soft_heap_limit().
*/
પૂર્ણાંક sqlite3HeapNearlyFull(શૂન્ય){
  પાછા AtomicLoad(&mem0.nearlyFull);
}

/*
** Deinitialize the memory allocation subsystem.
*/
શૂન્ય sqlite3MallocEnd(શૂન્ય){
  જો( sqlite3GlobalConfig.m.xShutdown ){
    sqlite3GlobalConfig.m.xShutdown(sqlite3GlobalConfig.m.pAppData);
  }
  memset(&mem0, 0, કદ(mem0));
}

/*
** Return the amount of memory currently checked out.
*/
sqlite3_int64 sqlite3_memory_used(શૂન્ય){
  sqlite3_int64 res, mx;
  sqlite3_status64(SQLITE_STATUS_MEMORY_USED, &res, &mx, 0);
  પાછા res;
}

/*
** Return the maximum amount of memory that has ever been
** checked out since either the beginning of this process
** or since the most recent reset.
*/
sqlite3_int64 sqlite3_memory_highwater(પૂર્ણાંક resetFlag){
  sqlite3_int64 res, mx;
  sqlite3_status64(SQLITE_STATUS_MEMORY_USED, &res, &mx, resetFlag);
  પાછા mx;
}

/*
** Trigger the alarm 
*/
સ્થિર શૂન્ય sqlite3MallocAlarm(પૂર્ણાંક nByte){
  જો( mem0.alarmThreshold<=0 ) પાછા;
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
સ્થિર SQLITE_NOINLINE શૂન્ય test_oom_breakpoint(u64 n){
  સ્થિર u64 nOomFault = 0;
  nOomFault += n;
  /* The assert() is never reached in a human lifetime.  It  is here mostly
  ** to prevent code optimizers from optimizing out this function. */
  પ્રતિજ્ઞા( (nOomFault>>32) < 0xffffffff );
}
#નહીંતર
# define test_oom_breakpoint(X)   /* No-op for production builds */
#endif

/*
** Do a memory allocation with statistics and alarms.  Assume the
** lock is already held.
*/
સ્થિર શૂન્ય mallocWithAlarm(પૂર્ણાંક n, શૂન્ય **pp){
  શૂન્ય *p;
  પૂર્ણાંક nFull;
  પ્રતિજ્ઞા( sqlite3_mutex_held(mem0.mutex) );
  પ્રતિજ્ઞા( n>0 );

  /* In Firefox (circa 2017-02-08), xRoundup() is remapped to an internal
  ** implementation of malloc_good_size(), which must be called in debug
  ** mode and specifically when the DMD "Dark Matter Detector" is enabled
  ** or else a crash results.  Hence, do not attempt to optimize out the
  ** following xRoundup() call. */
  nFull = sqlite3GlobalConfig.m.xRoundup(n);

  sqlite3StatusHighwater(SQLITE_STATUS_MALLOC_SIZE, n);
  જો( mem0.alarmThreshold>0 ){
    sqlite3_int64 nUsed = sqlite3_Status_કિંમત(SQLITE_STATUS_MEMORY_USED);
    જો( nUsed >= mem0.alarmThreshold - nFull ){
      AtomicStore(&mem0.nearlyFull, 1);
      sqlite3MallocAlarm(nFull);
      જો( mem0.hardLimit ){
        nUsed = sqlite3_Status_કિંમત(SQLITE_STATUS_MEMORY_USED);
        જો( nUsed >= mem0.hardLimit - nFull ){
          test_oom_breakpoint(1);
          *pp = 0;
          પાછા;
        }
      }
    }નહીંતર{
      AtomicStore(&mem0.nearlyFull, 0);
    }
  }
  p = sqlite3GlobalConfig.m.xMalloc(nFull);
#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
  જો( p==0 && mem0.alarmThreshold>0 ){
    sqlite3MallocAlarm(nFull);
    p = sqlite3GlobalConfig.m.xMalloc(nFull);
  }
#endif
  જો( p ){
    nFull = sqlite3MallocSize(p);
    sqlite3StatusUp(SQLITE_STATUS_MEMORY_USED, nFull);
    sqlite3StatusUp(SQLITE_STATUS_MALLOC_ગણતરી, 1);
  }
  *pp = p;
}

/*
** Allocate memory.  This routine is like sqlite3_malloc() except that it
** assumes the memory subsystem has already been initialized.
*/
શૂન્ય *sqlite3Malloc(u64 n){
  શૂન્ય *p;
  જો( n==0 || n>SQLITE_MAX_ALLOCATION_SIZE ){
    p = 0;
  }નહીંતર જો( sqlite3GlobalConfig.bMemstat ){
    sqlite3_mutex_enter(mem0.mutex);
    mallocWithAlarm((પૂર્ણાંક)n, &p);
    sqlite3_mutex_leave(mem0.mutex);
  }નહીંતર{
    p = sqlite3GlobalConfig.m.xMalloc((પૂર્ણાંક)n);
  }
  પ્રતિજ્ઞા( EIGHT_BYTE_ALIGNMENT(p) );  /* IMP: R-11148-40995 */
  પાછા p;
}

/*
** This version of the memory allocation is for use by the application.
** First make sure the memory subsystem is initialized, then do the
** allocation.
*/
શૂન્ય *sqlite3_malloc(પૂર્ણાંક n){
#ifndef SQLITE_OMIT_AUTOINIT
  જો( sqlite3_initialize() ) પાછા 0;
#endif
  પાછા n<=0 ? 0 : sqlite3Malloc(n);
}
શૂન્ય *sqlite3_malloc64(sqlite3_uint64 n){
#ifndef SQLITE_OMIT_AUTOINIT
  જો( sqlite3_initialize() ) પાછા 0;
#endif
  પાછા sqlite3Malloc(n);
}

/*
** TRUE if p is a lookaside memory allocation from db
*/
#ifndef SQLITE_OMIT_LOOKASIDE
સ્થિર પૂર્ણાંક isLookaside(sqlite3 *db, અચલ શૂન્ય *p){
  પાછા SQLITE_WITHIN(p, db->lookaside.pStart, db->lookaside.pTrueEnd);
}
#નહીંતર
#define isLookaside(A,B) 0
#endif

/*
** Return the size of a memory allocation previously obtained from
** sqlite3Malloc() or sqlite3_malloc().
*/
પૂર્ણાંક sqlite3MallocSize(અચલ શૂન્ય *p){
  પ્રતિજ્ઞા( sqlite3MemdebugHasType(p, MEMTYPE_HEAP) );
  પાછા sqlite3GlobalConfig.m.xSize((શૂન્ય*)p);
}
સ્થિર પૂર્ણાંક lookasideMallocSize(sqlite3 *db, અચલ શૂન્ય *p){
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE    
  પાછા p<db->lookaside.p_મધ્ય ? db->lookaside.szTrue : LOOKASIDE_SMALL;
#નહીંતર
  પાછા db->lookaside.szTrue;
#endif  
}
પૂર્ણાંક sqlite3DbMallocSize(sqlite3 *db, અચલ શૂન્ય *p){
  પ્રતિજ્ઞા( p!=0 );
#ifdef SQLITE_DEBUG
  જો( db==0 ){
    પ્રતિજ્ઞા( sqlite3MemdebugNoType(p, (u8)~MEMTYPE_HEAP) );
    પ્રતિજ્ઞા( sqlite3MemdebugHasType(p, MEMTYPE_HEAP) );
  }નહીંતર જો( !isLookaside(db,p) ){
    પ્રતિજ્ઞા( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
    પ્રતિજ્ઞા( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  }
#endif
  જો( db ){
    જો( ((uptr)p)<(uptr)(db->lookaside.pTrueEnd) ){
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
      જો( ((uptr)p)>=(uptr)(db->lookaside.p_મધ્ય) ){
        પ્રતિજ્ઞા( sqlite3_mutex_held(db->mutex) );
        પાછા LOOKASIDE_SMALL;
      }
#endif
      જો( ((uptr)p)>=(uptr)(db->lookaside.pStart) ){
        પ્રતિજ્ઞા( sqlite3_mutex_held(db->mutex) );
        પાછા db->lookaside.szTrue;
      }
    }
  }
  પાછા sqlite3GlobalConfig.m.xSize((શૂન્ય*)p);
}
sqlite3_uint64 sqlite3_msize(શૂન્ય *p){
  પ્રતિજ્ઞા( sqlite3MemdebugNoType(p, (u8)~MEMTYPE_HEAP) );
  પ્રતિજ્ઞા( sqlite3MemdebugHasType(p, MEMTYPE_HEAP) );
  પાછા p ? sqlite3GlobalConfig.m.xSize(p) : 0;
}

/*
** Free memory previously obtained from sqlite3Malloc().
*/
શૂન્ય sqlite3_free(શૂન્ય *p){
  જો( p==0 ) પાછા;  /* IMP: R-49053-54554 */
  પ્રતિજ્ઞા( sqlite3MemdebugHasType(p, MEMTYPE_HEAP) );
  પ્રતિજ્ઞા( sqlite3MemdebugNoType(p, (u8)~MEMTYPE_HEAP) );
  જો( sqlite3GlobalConfig.bMemstat ){
    sqlite3_mutex_enter(mem0.mutex);
    sqlite3StatusDown(SQLITE_STATUS_MEMORY_USED, sqlite3MallocSize(p));
    sqlite3StatusDown(SQLITE_STATUS_MALLOC_ગણતરી, 1);
    sqlite3GlobalConfig.m.xFree(p);
    sqlite3_mutex_leave(mem0.mutex);
  }નહીંતર{
    sqlite3GlobalConfig.m.xFree(p);
  }
}

/*
** Add the size of memory allocation "p" to the count in
** *db->pnBytesFreed.
*/
સ્થિર SQLITE_NOINLINE શૂન્ય measureAllocationSize(sqlite3 *db, શૂન્ય *p){
  *db->pnBytesFreed += sqlite3DbMallocSize(db,p);
}

/*
** Free memory that might be associated with a particular database
** connection.  Calling sqlite3DbFree(D,X) for X==0 is a harmless no-op.
** The sqlite3DbFreeNN(D,X) version requires that X be non-NULL.
*/
શૂન્ય sqlite3DbFreeNN(sqlite3 *db, શૂન્ય *p){
  પ્રતિજ્ઞા( db==0 || sqlite3_mutex_held(db->mutex) );
  પ્રતિજ્ઞા( p!=0 );
  જો( db ){
    જો( ((uptr)p)<(uptr)(db->lookaside.pEnd) ){
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
      જો( ((uptr)p)>=(uptr)(db->lookaside.p_મધ્ય) ){
        LookasideSlot *pBuf = (LookasideSlot*)p;
        પ્રતિજ્ઞા( db->pnBytesFreed==0 );
#ifdef SQLITE_DEBUG
        memset(p, 0xaa, LOOKASIDE_SMALL);  /* Trash freed content */
#endif
        pBuf->pNext = db->lookaside.pSmallFree;
        db->lookaside.pSmallFree = pBuf;
        પાછા;
      }
#endif /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
      જો( ((uptr)p)>=(uptr)(db->lookaside.pStart) ){
        LookasideSlot *pBuf = (LookasideSlot*)p;
        પ્રતિજ્ઞા( db->pnBytesFreed==0 );
#ifdef SQLITE_DEBUG
        memset(p, 0xaa, db->lookaside.szTrue);  /* Trash freed content */
#endif
        pBuf->pNext = db->lookaside.pFree;
        db->lookaside.pFree = pBuf;
        પાછા;
      }
    }
    જો( db->pnBytesFreed ){
      measureAllocationSize(db, p);
      પાછા;
    }
  }
  પ્રતિજ્ઞા( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  પ્રતિજ્ઞા( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  પ્રતિજ્ઞા( db!=0 || sqlite3MemdebugNoType(p, MEMTYPE_LOOKASIDE) );
  sqlite3MemdebugSetType(p, MEMTYPE_HEAP);
  sqlite3_free(p);
}
શૂન્ય sqlite3DbNNFreeNN(sqlite3 *db, શૂન્ય *p){
  પ્રતિજ્ઞા( db!=0 );
  પ્રતિજ્ઞા( sqlite3_mutex_held(db->mutex) );
  પ્રતિજ્ઞા( p!=0 );
  જો( ((uptr)p)<(uptr)(db->lookaside.pEnd) ){
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
    જો( ((uptr)p)>=(uptr)(db->lookaside.p_મધ્ય) ){
      LookasideSlot *pBuf = (LookasideSlot*)p;
      પ્રતિજ્ઞા( db->pnBytesFreed==0 );
#ifdef SQLITE_DEBUG
      memset(p, 0xaa, LOOKASIDE_SMALL);  /* Trash freed content */
#endif
      pBuf->pNext = db->lookaside.pSmallFree;
      db->lookaside.pSmallFree = pBuf;
      પાછા;
    }
#endif /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
    જો( ((uptr)p)>=(uptr)(db->lookaside.pStart) ){
      LookasideSlot *pBuf = (LookasideSlot*)p;
      પ્રતિજ્ઞા( db->pnBytesFreed==0 );
#ifdef SQLITE_DEBUG
      memset(p, 0xaa, db->lookaside.szTrue);  /* Trash freed content */
#endif
      pBuf->pNext = db->lookaside.pFree;
      db->lookaside.pFree = pBuf;
      પાછા;
    }
  }
  જો( db->pnBytesFreed ){
    measureAllocationSize(db, p);
    પાછા;
  }
  પ્રતિજ્ઞા( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  પ્રતિજ્ઞા( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  sqlite3MemdebugSetType(p, MEMTYPE_HEAP);
  sqlite3_free(p);
}
શૂન્ય sqlite3DbFree(sqlite3 *db, શૂન્ય *p){
  પ્રતિજ્ઞા( db==0 || sqlite3_mutex_held(db->mutex) );
  જો( p ) sqlite3DbFreeNN(db, p);
}

/*
** Change the size of an existing memory allocation
*/
શૂન્ય *sqlite3Realloc(શૂન્ય *pOld, u64 nBytes){
  પૂર્ણાંક nOld, nNew, nDiff;
  શૂન્ય *pNew;
  પ્રતિજ્ઞા( sqlite3MemdebugHasType(pOld, MEMTYPE_HEAP) );
  પ્રતિજ્ઞા( sqlite3MemdebugNoType(pOld, (u8)~MEMTYPE_HEAP) );
  જો( pOld==0 ){
    પાછા sqlite3Malloc(nBytes); /* IMP: R-04300-56712 */
  }
  જો( nBytes==0 ){
    sqlite3_free(pOld); /* IMP: R-26507-47431 */
    પાછા 0;
  }
  જો( nBytes>SQLITE_MAX_ALLOCATION_SIZE ){
    પાછા 0;
  }
  nOld = sqlite3MallocSize(pOld);
  /* IMPLEMENTATION-OF: R-46199-30249 SQLite guarantees that the second
  ** argument to xRealloc is always a value returned by a prior call to
  ** xRoundup. */
  nNew = sqlite3GlobalConfig.m.xRoundup((પૂર્ણાંક)nBytes);
  જો( nOld==nNew ){
    pNew = pOld;
  }નહીંતર જો( sqlite3GlobalConfig.bMemstat ){
    sqlite3_int64 nUsed;
    sqlite3_mutex_enter(mem0.mutex);
    sqlite3StatusHighwater(SQLITE_STATUS_MALLOC_SIZE, (પૂર્ણાંક)nBytes);
    nDiff = nNew - nOld;
    જો( nDiff>0 && (nUsed = sqlite3_Status_કિંમત(SQLITE_STATUS_MEMORY_USED)) >= 
          mem0.alarmThreshold-nDiff ){
      sqlite3MallocAlarm(nDiff);
      જો( mem0.hardLimit>0 && nUsed >= mem0.hardLimit - nDiff ){
        sqlite3_mutex_leave(mem0.mutex);
        test_oom_breakpoint(1);
        પાછા 0;
      }
    }
    pNew = sqlite3GlobalConfig.m.xRealloc(pOld, nNew);
#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
    જો( pNew==0 && mem0.alarmThreshold>0 ){
      sqlite3MallocAlarm((પૂર્ણાંક)nBytes);
      pNew = sqlite3GlobalConfig.m.xRealloc(pOld, nNew);
    }
#endif
    જો( pNew ){
      nNew = sqlite3MallocSize(pNew);
      sqlite3StatusUp(SQLITE_STATUS_MEMORY_USED, nNew-nOld);
    }
    sqlite3_mutex_leave(mem0.mutex);
  }નહીંતર{
    pNew = sqlite3GlobalConfig.m.xRealloc(pOld, nNew);
  }
  પ્રતિજ્ઞા( EIGHT_BYTE_ALIGNMENT(pNew) ); /* IMP: R-11148-40995 */
  પાછા pNew;
}

/*
** The public interface to sqlite3Realloc.  Make sure that the memory
** subsystem is initialized prior to invoking sqliteRealloc.
*/
શૂન્ય *sqlite3_realloc(શૂન્ય *pOld, પૂર્ણાંક n){
#ifndef SQLITE_OMIT_AUTOINIT
  જો( sqlite3_initialize() ) પાછા 0;
#endif
  જો( n<0 ) n = 0;  /* IMP: R-26507-47431 */
  પાછા sqlite3Realloc(pOld, n);
}
શૂન્ય *sqlite3_realloc64(શૂન્ય *pOld, sqlite3_uint64 n){
#ifndef SQLITE_OMIT_AUTOINIT
  જો( sqlite3_initialize() ) પાછા 0;
#endif
  પાછા sqlite3Realloc(pOld, n);
}


/*
** Allocate and zero memory.
*/ 
શૂન્ય *sqlite3MallocZero(u64 n){
  શૂન્ય *p = sqlite3Malloc(n);
  જો( p ){
    memset(p, 0, (size_t)n);
  }
  પાછા p;
}

/*
** Allocate and zero memory.  If the allocation fails, make
** the mallocFailed flag in the connection pointer.
*/
શૂન્ય *sqlite3DbMallocZero(sqlite3 *db, u64 n){
  શૂન્ય *p;
  testcase( db==0 );
  p = sqlite3DbMallocRaw(db, n);
  જો( p ) memset(p, 0, (size_t)n);
  પાછા p;
}


/* Finish the work of sqlite3DbMallocRawNN for the unusual and
** slower case when the allocation cannot be fulfilled using lookaside.
*/
સ્થિર SQLITE_NOINLINE શૂન્ય *dbMallocRawFinish(sqlite3 *db, u64 n){
  શૂન્ય *p;
  પ્રતિજ્ઞા( db!=0 );
  p = sqlite3Malloc(n);
  જો( !p ) sqlite3OomFault(db);
  sqlite3MemdebugSetType(p, 
         (db->lookaside.bDisable==0) ? MEMTYPE_LOOKASIDE : MEMTYPE_HEAP);
  પાછા p;
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
શૂન્ય *sqlite3DbMallocRaw(sqlite3 *db, u64 n){
  શૂન્ય *p;
  જો( db ) પાછા sqlite3DbMallocRawNN(db, n);
  p = sqlite3Malloc(n);
  sqlite3MemdebugSetType(p, MEMTYPE_HEAP);
  પાછા p;
}
શૂન્ય *sqlite3DbMallocRawNN(sqlite3 *db, u64 n){
#ifndef SQLITE_OMIT_LOOKASIDE
  LookasideSlot *pBuf;
  પ્રતિજ્ઞા( db!=0 );
  પ્રતિજ્ઞા( sqlite3_mutex_held(db->mutex) );
  પ્રતિજ્ઞા( db->pnBytesFreed==0 );
  જો( n>db->lookaside.sz ){
    જો( !db->lookaside.bDisable ){
      db->lookaside.anStat[1]++;      
    }નહીંતર જો( db->mallocFailed ){
      પાછા 0;
    }
    પાછા dbMallocRawFinish(db, n);
  }
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
  જો( n<=LOOKASIDE_SMALL ){
    જો( (pBuf = db->lookaside.pSmallFree)!=0 ){
      db->lookaside.pSmallFree = pBuf->pNext;
      db->lookaside.anStat[0]++;
      પાછા (શૂન્ય*)pBuf;
    }નહીંતર જો( (pBuf = db->lookaside.pSmallInit)!=0 ){
      db->lookaside.pSmallInit = pBuf->pNext;
      db->lookaside.anStat[0]++;
      પાછા (શૂન્ય*)pBuf;
    }
  }
#endif
  જો( (pBuf = db->lookaside.pFree)!=0 ){
    db->lookaside.pFree = pBuf->pNext;
    db->lookaside.anStat[0]++;
    પાછા (શૂન્ય*)pBuf;
  }નહીંતર જો( (pBuf = db->lookaside.pInit)!=0 ){
    db->lookaside.pInit = pBuf->pNext;
    db->lookaside.anStat[0]++;
    પાછા (શૂન્ય*)pBuf;
  }નહીંતર{
    db->lookaside.anStat[2]++;
  }
#નહીંતર
  પ્રતિજ્ઞા( db!=0 );
  પ્રતિજ્ઞા( sqlite3_mutex_held(db->mutex) );
  પ્રતિજ્ઞા( db->pnBytesFreed==0 );
  જો( db->mallocFailed ){
    પાછા 0;
  }
#endif
  પાછા dbMallocRawFinish(db, n);
}

/* Forward declaration */
સ્થિર SQLITE_NOINLINE શૂન્ય *dbReallocFinish(sqlite3 *db, શૂન્ય *p, u64 n);

/*
** Resize the block of memory pointed to by p to n bytes. If the
** resize fails, set the mallocFailed flag in the connection object.
*/
શૂન્ય *sqlite3DbRealloc(sqlite3 *db, શૂન્ય *p, u64 n){
  પ્રતિજ્ઞા( db!=0 );
  જો( p==0 ) પાછા sqlite3DbMallocRawNN(db, n);
  પ્રતિજ્ઞા( sqlite3_mutex_held(db->mutex) );
  જો( ((uptr)p)<(uptr)db->lookaside.pEnd ){
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
    જો( ((uptr)p)>=(uptr)db->lookaside.p_મધ્ય ){
      જો( n<=LOOKASIDE_SMALL ) પાછા p;
    }નહીંતર
#endif
    જો( ((uptr)p)>=(uptr)db->lookaside.pStart ){
      જો( n<=db->lookaside.szTrue ) પાછા p;
    }
  }
  પાછા dbReallocFinish(db, p, n);
}
સ્થિર SQLITE_NOINLINE શૂન્ય *dbReallocFinish(sqlite3 *db, શૂન્ય *p, u64 n){
  શૂન્ય *pNew = 0;
  પ્રતિજ્ઞા( db!=0 );
  પ્રતિજ્ઞા( p!=0 );
  જો( db->mallocFailed==0 ){
    જો( isLookaside(db, p) ){
      pNew = sqlite3DbMallocRawNN(db, n);
      જો( pNew ){
        memcpy(pNew, p, lookasideMallocSize(db, p));
        sqlite3DbFree(db, p);
      }
    }નહીંતર{
      પ્રતિજ્ઞા( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
      પ્રતિજ્ઞા( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
      sqlite3MemdebugSetType(p, MEMTYPE_HEAP);
      pNew = sqlite3Realloc(p, n);
      જો( !pNew ){
        sqlite3OomFault(db);
      }
      sqlite3MemdebugSetType(pNew,
            (db->lookaside.bDisable==0 ? MEMTYPE_LOOKASIDE : MEMTYPE_HEAP));
    }
  }
  પાછા pNew;
}

/*
** Attempt to reallocate p.  If the reallocation fails, then free p
** and set the mallocFailed flag in the database connection.
*/
શૂન્ય *sqlite3DbReallocOrFree(sqlite3 *db, શૂન્ય *p, u64 n){
  શૂન્ય *pNew;
  pNew = sqlite3DbRealloc(db, p, n);
  જો( !pNew ){
    sqlite3DbFree(db, p);
  }
  પાછા pNew;
}

/*
** Make a copy of a string in memory obtained from sqliteMalloc(). These 
** functions call sqlite3MallocRaw() directly instead of sqliteMalloc(). This
** is because when memory debugging is turned on, these two functions are 
** called via macros that record the current file and line number in the
** ThreadData structure.
*/
અક્ષર *sqlite3DbStrDup(sqlite3 *db, અચલ અક્ષર *z){
  અક્ષર *zNew;
  size_t n;
  જો( z==0 ){
    પાછા 0;
  }
  n = strlen(z) + 1;
  zNew = sqlite3DbMallocRaw(db, n);
  જો( zNew ){
    memcpy(zNew, z, n);
  }
  પાછા zNew;
}
અક્ષર *sqlite3DbStrNDup(sqlite3 *db, અચલ અક્ષર *z, u64 n){
  અક્ષર *zNew;
  પ્રતિજ્ઞા( db!=0 );
  પ્રતિજ્ઞા( z!=0 || n==0 );
  પ્રતિજ્ઞા( (n&0x7fffffff)==n );
  zNew = z ? sqlite3DbMallocRawNN(db, n+1) : 0;
  જો( zNew ){
    memcpy(zNew, z, (size_t)n);
    zNew[n] = 0;
  }
  પાછા zNew;
}

/*
** The text between zStart and zEnd represents a phrase within a larger
** SQL statement.  Make a copy of this phrase in space obtained form
** sqlite3DbMalloc().  Omit leading and trailing whitespace.
*/
અક્ષર *sqlite3DbSpanDup(sqlite3 *db, અચલ અક્ષર *zStart, અચલ અક્ષર *zEnd){
  પૂર્ણાંક n;
#ifdef SQLITE_DEBUG
  /* Because of the way the parser works, the span is guaranteed to contain
  ** at least one non-space character */
  માટે(n=0; sqlite3Isspace(zStart[n]); n++){ પ્રતિજ્ઞા( &zStart[n]<zEnd ); }
#endif
  જ્યારે( sqlite3Isspace(zStart[0]) ) zStart++;
  n = (પૂર્ણાંક)(zEnd - zStart);
  જ્યારે( sqlite3Isspace(zStart[n-1]) ) n--;
  પાછા sqlite3DbStrNDup(db, zStart, n);
}

/*
** Free any prior content in *pz and replace it with a copy of zNew.
*/
શૂન્ય sqlite3SetString(અક્ષર **pz, sqlite3 *db, અચલ અક્ષર *zNew){
  અક્ષર *z = sqlite3DbStrDup(db, zNew);
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
શૂન્ય *sqlite3OomFault(sqlite3 *db){
  જો( db->mallocFailed==0 && db->bBenignMalloc==0 ){
    db->mallocFailed = 1;
    જો( db->nVdbeExec>0 ){
      AtomicStore(&db->u1.isInterrupted, 1);
    }
    DisableLookaside;
    જો( db->pParse ){
      Parse *pParse;
      sqlite3_Error_સંદેશ(db->pParse, "out of memory");
      db->pParse->rc = SQLITE_NOMEM_BKPT;
      માટે(pParse=db->pParse->pOuterParse; pParse; pParse = pParse->pOuterParse){
        pParse->nErr++;
        pParse->rc = SQLITE_NOMEM;
      } 
    }
  }
  પાછા 0;
}

/*
** This routine reactivates the memory allocator and clears the
** db->mallocFailed flag as necessary.
**
** The memory allocator is not restarted if there are running
** VDBEs.
*/
શૂન્ય sqlite3OomClear(sqlite3 *db){
  જો( db->mallocFailed && db->nVdbeExec==0 ){
    db->mallocFailed = 0;
    AtomicStore(&db->u1.isInterrupted, 0);
    પ્રતિજ્ઞા( db->lookaside.bDisable>0 );
    EnableLookaside;
  }
}

/*
** Take actions at the end of an API call to deal with error codes.
*/
સ્થિર SQLITE_NOINLINE પૂર્ણાંક apiHandleError(sqlite3 *db, પૂર્ણાંક rc){
  જો( db->mallocFailed || rc==SQLITE_IOERR_NOMEM ){
    sqlite3OomClear(db);
    sqlite3Error(db, SQLITE_NOMEM);
    પાછા SQLITE_NOMEM_BKPT;
  }
  પાછા rc & db->errMask;
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
પૂર્ણાંક sqlite3ApiExit(sqlite3* db, પૂર્ણાંક rc){
  /* If the db handle must hold the connection handle mutex here.
  ** Otherwise the read (and possible write) of db->mallocFailed 
  ** is unsafe, as is the call to sqlite3Error().
  */
  પ્રતિજ્ઞા( db!=0 );
  પ્રતિજ્ઞા( sqlite3_mutex_held(db->mutex) );
  જો( db->mallocFailed || rc ){
    પાછા apiHandleError(db, rc);
  }
  પાછા 0;
}
