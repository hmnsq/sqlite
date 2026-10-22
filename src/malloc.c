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
#include "sqliteInt.h"
#include <stdarg.h>

/*
** Attempt to release up to n bytes of non-essential memory currently
** held by SQLite. An example of non-essential memory is memory used to
** cache database pages that are not currently in use.
*/
ціле sqlite3_release_memory(ціле n){
#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
  повернути sqlite3PcacheReleaseMemory(n);
#інакше
  /* IMPLEMENTATION-OF: R-34391-24921 The sqlite3_release_memory() routine
  ** is a no-op returning zero if SQLite is not compiled with
  ** SQLITE_ENABLE_MEMORY_MANAGEMENT. */
  UNUSED_PARAMETER(n);
  повернути 0;
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
static SQLITE_WSD struct Mem0Global {
  sqlite3_mutex *mutex;         /* Mutex to serialize access */
  sqlite3_int64 alarmThreshold; /* The soft heap limit */
  sqlite3_int64 hardLimit;      /* The hard upper bound on memory */

  /*
  ** True if heap is nearly "full" where "full" is defined by the
  ** sqlite3_soft_heap_limit() setting.
  */
  ціле nearlyFull;
} mem0 = { 0, SQLITE_MAX_MEMORY, SQLITE_MAX_MEMORY, 0 };

#define mem0 GLOBAL(struct Mem0Global, mem0)

/*
** Return the memory allocator mutex. sqlite3_status() needs it.
*/
sqlite3_mutex *sqlite3MallocMutex(void){
  повернути mem0.mutex;
}

#ifndef SQLITE_OMIT_DEPRECATED
/*
** Deprecated external interface.  It used to set an alarm callback
** that was invoked when memory usage grew too large.  Now it is a
** no-op.
*/
ціле sqlite3_memory_alarm(
  void(*xCallback)(void *pArg, sqlite3_int64 used,ціле N),
  void *pArg,
  sqlite3_int64 iThreshold
){
  (void)xCallback;
  (void)pArg;
  (void)iThreshold;
  повернути SQLITE_OK;
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
  ціле rc = sqlite3_initialize();
  якщо( rc ) повернути -1;
#endif
  sqlite3_mutex_enter(mem0.mutex);
  priorLimit = mem0.alarmThreshold;
  якщо( n<0 ){
    sqlite3_mutex_leave(mem0.mutex);
    повернути priorLimit;
  }
  якщо( mem0.hardLimit>0 && (n>mem0.hardLimit || n==0) ){
    n = mem0.hardLimit;
  }
  mem0.alarmThreshold = n;
  nUsed = sqlite3StatusValue(SQLITE_STATUS_MEMORY_USED);
  AtomicStore(&mem0.nearlyFull, n>0 && n<=nUsed);
  sqlite3_mutex_leave(mem0.mutex);
  excess = sqlite3_memory_used() - n;
  якщо( excess>0 ) sqlite3_release_memory((ціле)(excess & 0x7fffffff));
  повернути priorLimit;
}
void sqlite3_soft_heap_limit(ціле n){
  якщо( n<0 ) n = 0;
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
  ціле rc = sqlite3_initialize();
  якщо( rc ) повернути -1;
#endif
  sqlite3_mutex_enter(mem0.mutex);
  priorLimit = mem0.hardLimit;
  якщо( n>=0 ){
    mem0.hardLimit = n;
    якщо( n<mem0.alarmThreshold || mem0.alarmThreshold==0 ){
      mem0.alarmThreshold = n;
    }
  }
  sqlite3_mutex_leave(mem0.mutex);
  повернути priorLimit;
}


/*
** Initialize the memory allocation subsystem.
*/
ціле sqlite3MallocInit(void){
  ціле rc;
  якщо( sqlite3GlobalConfig.m.xMalloc==0 ){
    sqlite3MemSetDefault();
  }
  mem0.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM);
  якщо( sqlite3GlobalConfig.pPage==0 || sqlite3GlobalConfig.szPage<512
      || sqlite3GlobalConfig.nPage<=0 ){
    sqlite3GlobalConfig.pPage = 0;
    sqlite3GlobalConfig.szPage = 0;
  }
  rc = sqlite3GlobalConfig.m.xInit(sqlite3GlobalConfig.m.pAppData);
  якщо( rc!=SQLITE_OK ) memset(&mem0, 0, sizeof(mem0));
  повернути rc;
}

/*
** Return true if the heap is currently under memory pressure - in other
** words if the amount of heap used is close to the limit set by
** sqlite3_soft_heap_limit().
*/
ціле sqlite3HeapNearlyFull(void){
  повернути AtomicLoad(&mem0.nearlyFull);
}

/*
** Deinitialize the memory allocation subsystem.
*/
void sqlite3MallocEnd(void){
  якщо( sqlite3GlobalConfig.m.xShutdown ){
    sqlite3GlobalConfig.m.xShutdown(sqlite3GlobalConfig.m.pAppData);
  }
  memset(&mem0, 0, sizeof(mem0));
}

/*
** Return the amount of memory currently checked out.
*/
sqlite3_int64 sqlite3_memory_used(void){
  sqlite3_int64 res, mx;
  sqlite3_status64(SQLITE_STATUS_MEMORY_USED, &res, &mx, 0);
  повернути res;
}

/*
** Return the maximum amount of memory that has ever been
** checked out since either the beginning of this process
** or since the most recent reset.
*/
sqlite3_int64 sqlite3_memory_highwater(ціле resetFlag){
  sqlite3_int64 res, mx;
  sqlite3_status64(SQLITE_STATUS_MEMORY_USED, &res, &mx, resetFlag);
  повернути mx;
}

/*
** Trigger the alarm 
*/
static void sqlite3MallocAlarm(ціле nByte){
  якщо( mem0.alarmThreshold<=0 ) повернути;
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
static SQLITE_NOINLINE void test_oom_breakpoint(u64 n){
  static u64 nOomFault = 0;
  nOomFault += n;
  /* The assert() is never reached in a human lifetime.  It  is here mostly
  ** to prevent code optimizers from optimizing out this function. */
  assert( (nOomFault>>32) < 0xffffffff );
}
#інакше
# define test_oom_breakpoint(X)   /* No-op for production builds */
#endif

/*
** Do a memory allocation with statistics and alarms.  Assume the
** lock is already held.
*/
static void mallocWithAlarm(ціле n, void **pp){
  void *p;
  ціле nFull;
  assert( sqlite3_mutex_held(mem0.mutex) );
  assert( n>0 );

  /* In Firefox (circa 2017-02-08), xRoundup() is remapped to an internal
  ** implementation of malloc_good_size(), which must be called in debug
  ** mode and specifically when the DMD "Dark Matter Detector" is enabled
  ** or else a crash results.  Hence, do not attempt to optimize out the
  ** following xRoundup() call. */
  nFull = sqlite3GlobalConfig.m.xRoundup(n);

  sqlite3StatusHighwater(SQLITE_STATUS_MALLOC_SIZE, n);
  якщо( mem0.alarmThreshold>0 ){
    sqlite3_int64 nUsed = sqlite3StatusValue(SQLITE_STATUS_MEMORY_USED);
    якщо( nUsed >= mem0.alarmThreshold - nFull ){
      AtomicStore(&mem0.nearlyFull, 1);
      sqlite3MallocAlarm(nFull);
      якщо( mem0.hardLimit ){
        nUsed = sqlite3StatusValue(SQLITE_STATUS_MEMORY_USED);
        якщо( nUsed >= mem0.hardLimit - nFull ){
          test_oom_breakpoint(1);
          *pp = 0;
          повернути;
        }
      }
    }інакше{
      AtomicStore(&mem0.nearlyFull, 0);
    }
  }
  p = sqlite3GlobalConfig.m.xMalloc(nFull);
#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
  якщо( p==0 && mem0.alarmThreshold>0 ){
    sqlite3MallocAlarm(nFull);
    p = sqlite3GlobalConfig.m.xMalloc(nFull);
  }
#endif
  якщо( p ){
    nFull = sqlite3MallocSize(p);
    sqlite3StatusUp(SQLITE_STATUS_MEMORY_USED, nFull);
    sqlite3StatusUp(SQLITE_STATUS_MALLOC_COUNT, 1);
  }
  *pp = p;
}

/*
** Allocate memory.  This routine is like sqlite3_malloc() except that it
** assumes the memory subsystem has already been initialized.
*/
void *sqlite3Malloc(u64 n){
  void *p;
  якщо( n==0 || n>SQLITE_MAX_ALLOCATION_SIZE ){
    p = 0;
  }інакше якщо( sqlite3GlobalConfig.bMemstat ){
    sqlite3_mutex_enter(mem0.mutex);
    mallocWithAlarm((ціле)n, &p);
    sqlite3_mutex_leave(mem0.mutex);
  }інакше{
    p = sqlite3GlobalConfig.m.xMalloc((ціле)n);
  }
  assert( EIGHT_BYTE_ALIGNMENT(p) );  /* IMP: R-11148-40995 */
  повернути p;
}

/*
** This version of the memory allocation is for use by the application.
** First make sure the memory subsystem is initialized, then do the
** allocation.
*/
void *sqlite3_malloc(ціле n){
#ifndef SQLITE_OMIT_AUTOINIT
  якщо( sqlite3_initialize() ) повернути 0;
#endif
  повернути n<=0 ? 0 : sqlite3Malloc(n);
}
void *sqlite3_malloc64(sqlite3_uint64 n){
#ifndef SQLITE_OMIT_AUTOINIT
  якщо( sqlite3_initialize() ) повернути 0;
#endif
  повернути sqlite3Malloc(n);
}

/*
** TRUE if p is a lookaside memory allocation from db
*/
#ifndef SQLITE_OMIT_LOOKASIDE
static ціле isLookaside(sqlite3 *db, const void *p){
  повернути SQLITE_WITHIN(p, db->lookaside.pStart, db->lookaside.pTrueEnd);
}
#інакше
#define isLookaside(A,B) 0
#endif

/*
** Return the size of a memory allocation previously obtained from
** sqlite3Malloc() or sqlite3_malloc().
*/
ціле sqlite3MallocSize(const void *p){
  assert( sqlite3MemdebugHasType(p, MEMTYPE_HEAP) );
  повернути sqlite3GlobalConfig.m.xSize((void*)p);
}
static ціле lookasideMallocSize(sqlite3 *db, const void *p){
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE    
  повернути p<db->lookaside.pMiddle ? db->lookaside.szTrue : LOOKASIDE_SMALL;
#інакше
  повернути db->lookaside.szTrue;
#endif  
}
ціле sqlite3DbMallocSize(sqlite3 *db, const void *p){
  assert( p!=0 );
#ifdef SQLITE_DEBUG
  якщо( db==0 ){
    assert( sqlite3MemdebugNoType(p, (u8)~MEMTYPE_HEAP) );
    assert( sqlite3MemdebugHasType(p, MEMTYPE_HEAP) );
  }інакше якщо( !isLookaside(db,p) ){
    assert( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
    assert( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  }
#endif
  якщо( db ){
    якщо( ((uptr)p)<(uptr)(db->lookaside.pTrueEnd) ){
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
      якщо( ((uptr)p)>=(uptr)(db->lookaside.pMiddle) ){
        assert( sqlite3_mutex_held(db->mutex) );
        повернути LOOKASIDE_SMALL;
      }
#endif
      якщо( ((uptr)p)>=(uptr)(db->lookaside.pStart) ){
        assert( sqlite3_mutex_held(db->mutex) );
        повернути db->lookaside.szTrue;
      }
    }
  }
  повернути sqlite3GlobalConfig.m.xSize((void*)p);
}
sqlite3_uint64 sqlite3_msize(void *p){
  assert( sqlite3MemdebugNoType(p, (u8)~MEMTYPE_HEAP) );
  assert( sqlite3MemdebugHasType(p, MEMTYPE_HEAP) );
  повернути p ? sqlite3GlobalConfig.m.xSize(p) : 0;
}

/*
** Free memory previously obtained from sqlite3Malloc().
*/
void sqlite3_free(void *p){
  якщо( p==0 ) повернути;  /* IMP: R-49053-54554 */
  assert( sqlite3MemdebugHasType(p, MEMTYPE_HEAP) );
  assert( sqlite3MemdebugNoType(p, (u8)~MEMTYPE_HEAP) );
  якщо( sqlite3GlobalConfig.bMemstat ){
    sqlite3_mutex_enter(mem0.mutex);
    sqlite3StatusDown(SQLITE_STATUS_MEMORY_USED, sqlite3MallocSize(p));
    sqlite3StatusDown(SQLITE_STATUS_MALLOC_COUNT, 1);
    sqlite3GlobalConfig.m.xFree(p);
    sqlite3_mutex_leave(mem0.mutex);
  }інакше{
    sqlite3GlobalConfig.m.xFree(p);
  }
}

/*
** Add the size of memory allocation "p" to the count in
** *db->pnBytesFreed.
*/
static SQLITE_NOINLINE void measureAllocationSize(sqlite3 *db, void *p){
  *db->pnBytesFreed += sqlite3DbMallocSize(db,p);
}

/*
** Free memory that might be associated with a particular database
** connection.  Calling sqlite3DbFree(D,X) for X==0 is a harmless no-op.
** The sqlite3DbFreeNN(D,X) version requires that X be non-NULL.
*/
void sqlite3DbFreeNN(sqlite3 *db, void *p){
  assert( db==0 || sqlite3_mutex_held(db->mutex) );
  assert( p!=0 );
  якщо( db ){
    якщо( ((uptr)p)<(uptr)(db->lookaside.pEnd) ){
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
      якщо( ((uptr)p)>=(uptr)(db->lookaside.pMiddle) ){
        LookasideSlot *pBuf = (LookasideSlot*)p;
        assert( db->pnBytesFreed==0 );
#ifdef SQLITE_DEBUG
        memset(p, 0xaa, LOOKASIDE_SMALL);  /* Trash freed content */
#endif
        pBuf->pNext = db->lookaside.pSmallFree;
        db->lookaside.pSmallFree = pBuf;
        повернути;
      }
#endif /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
      якщо( ((uptr)p)>=(uptr)(db->lookaside.pStart) ){
        LookasideSlot *pBuf = (LookasideSlot*)p;
        assert( db->pnBytesFreed==0 );
#ifdef SQLITE_DEBUG
        memset(p, 0xaa, db->lookaside.szTrue);  /* Trash freed content */
#endif
        pBuf->pNext = db->lookaside.pFree;
        db->lookaside.pFree = pBuf;
        повернути;
      }
    }
    якщо( db->pnBytesFreed ){
      measureAllocationSize(db, p);
      повернути;
    }
  }
  assert( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  assert( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  assert( db!=0 || sqlite3MemdebugNoType(p, MEMTYPE_LOOKASIDE) );
  sqlite3MemdebugSetType(p, MEMTYPE_HEAP);
  sqlite3_free(p);
}
void sqlite3DbNNFreeNN(sqlite3 *db, void *p){
  assert( db!=0 );
  assert( sqlite3_mutex_held(db->mutex) );
  assert( p!=0 );
  якщо( ((uptr)p)<(uptr)(db->lookaside.pEnd) ){
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
    якщо( ((uptr)p)>=(uptr)(db->lookaside.pMiddle) ){
      LookasideSlot *pBuf = (LookasideSlot*)p;
      assert( db->pnBytesFreed==0 );
#ifdef SQLITE_DEBUG
      memset(p, 0xaa, LOOKASIDE_SMALL);  /* Trash freed content */
#endif
      pBuf->pNext = db->lookaside.pSmallFree;
      db->lookaside.pSmallFree = pBuf;
      повернути;
    }
#endif /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
    якщо( ((uptr)p)>=(uptr)(db->lookaside.pStart) ){
      LookasideSlot *pBuf = (LookasideSlot*)p;
      assert( db->pnBytesFreed==0 );
#ifdef SQLITE_DEBUG
      memset(p, 0xaa, db->lookaside.szTrue);  /* Trash freed content */
#endif
      pBuf->pNext = db->lookaside.pFree;
      db->lookaside.pFree = pBuf;
      повернути;
    }
  }
  якщо( db->pnBytesFreed ){
    measureAllocationSize(db, p);
    повернути;
  }
  assert( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  assert( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  sqlite3MemdebugSetType(p, MEMTYPE_HEAP);
  sqlite3_free(p);
}
void sqlite3DbFree(sqlite3 *db, void *p){
  assert( db==0 || sqlite3_mutex_held(db->mutex) );
  якщо( p ) sqlite3DbFreeNN(db, p);
}

/*
** Change the size of an existing memory allocation
*/
void *sqlite3Realloc(void *pOld, u64 nBytes){
  ціле nOld, nNew, nDiff;
  void *pNew;
  assert( sqlite3MemdebugHasType(pOld, MEMTYPE_HEAP) );
  assert( sqlite3MemdebugNoType(pOld, (u8)~MEMTYPE_HEAP) );
  якщо( pOld==0 ){
    повернути sqlite3Malloc(nBytes); /* IMP: R-04300-56712 */
  }
  якщо( nBytes==0 ){
    sqlite3_free(pOld); /* IMP: R-26507-47431 */
    повернути 0;
  }
  якщо( nBytes>SQLITE_MAX_ALLOCATION_SIZE ){
    повернути 0;
  }
  nOld = sqlite3MallocSize(pOld);
  /* IMPLEMENTATION-OF: R-46199-30249 SQLite guarantees that the second
  ** argument to xRealloc is always a value returned by a prior call to
  ** xRoundup. */
  nNew = sqlite3GlobalConfig.m.xRoundup((ціле)nBytes);
  якщо( nOld==nNew ){
    pNew = pOld;
  }інакше якщо( sqlite3GlobalConfig.bMemstat ){
    sqlite3_int64 nUsed;
    sqlite3_mutex_enter(mem0.mutex);
    sqlite3StatusHighwater(SQLITE_STATUS_MALLOC_SIZE, (ціле)nBytes);
    nDiff = nNew - nOld;
    якщо( nDiff>0 && (nUsed = sqlite3StatusValue(SQLITE_STATUS_MEMORY_USED)) >= 
          mem0.alarmThreshold-nDiff ){
      sqlite3MallocAlarm(nDiff);
      якщо( mem0.hardLimit>0 && nUsed >= mem0.hardLimit - nDiff ){
        sqlite3_mutex_leave(mem0.mutex);
        test_oom_breakpoint(1);
        повернути 0;
      }
    }
    pNew = sqlite3GlobalConfig.m.xRealloc(pOld, nNew);
#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
    якщо( pNew==0 && mem0.alarmThreshold>0 ){
      sqlite3MallocAlarm((ціле)nBytes);
      pNew = sqlite3GlobalConfig.m.xRealloc(pOld, nNew);
    }
#endif
    якщо( pNew ){
      nNew = sqlite3MallocSize(pNew);
      sqlite3StatusUp(SQLITE_STATUS_MEMORY_USED, nNew-nOld);
    }
    sqlite3_mutex_leave(mem0.mutex);
  }інакше{
    pNew = sqlite3GlobalConfig.m.xRealloc(pOld, nNew);
  }
  assert( EIGHT_BYTE_ALIGNMENT(pNew) ); /* IMP: R-11148-40995 */
  повернути pNew;
}

/*
** The public interface to sqlite3Realloc.  Make sure that the memory
** subsystem is initialized prior to invoking sqliteRealloc.
*/
void *sqlite3_realloc(void *pOld, ціле n){
#ifndef SQLITE_OMIT_AUTOINIT
  якщо( sqlite3_initialize() ) повернути 0;
#endif
  якщо( n<0 ) n = 0;  /* IMP: R-26507-47431 */
  повернути sqlite3Realloc(pOld, n);
}
void *sqlite3_realloc64(void *pOld, sqlite3_uint64 n){
#ifndef SQLITE_OMIT_AUTOINIT
  якщо( sqlite3_initialize() ) повернути 0;
#endif
  повернути sqlite3Realloc(pOld, n);
}


/*
** Allocate and zero memory.
*/ 
void *sqlite3MallocZero(u64 n){
  void *p = sqlite3Malloc(n);
  якщо( p ){
    memset(p, 0, (size_t)n);
  }
  повернути p;
}

/*
** Allocate and zero memory.  If the allocation fails, make
** the mallocFailed flag in the connection pointer.
*/
void *sqlite3DbMallocZero(sqlite3 *db, u64 n){
  void *p;
  testcase( db==0 );
  p = sqlite3DbMallocRaw(db, n);
  якщо( p ) memset(p, 0, (size_t)n);
  повернути p;
}


/* Finish the work of sqlite3DbMallocRawNN for the unusual and
** slower case when the allocation cannot be fulfilled using lookaside.
*/
static SQLITE_NOINLINE void *dbMallocRawFinish(sqlite3 *db, u64 n){
  void *p;
  assert( db!=0 );
  p = sqlite3Malloc(n);
  якщо( !p ) sqlite3OomFault(db);
  sqlite3MemdebugSetType(p, 
         (db->lookaside.bDisable==0) ? MEMTYPE_LOOKASIDE : MEMTYPE_HEAP);
  повернути p;
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
void *sqlite3DbMallocRaw(sqlite3 *db, u64 n){
  void *p;
  якщо( db ) повернути sqlite3DbMallocRawNN(db, n);
  p = sqlite3Malloc(n);
  sqlite3MemdebugSetType(p, MEMTYPE_HEAP);
  повернути p;
}
void *sqlite3DbMallocRawNN(sqlite3 *db, u64 n){
#ifndef SQLITE_OMIT_LOOKASIDE
  LookasideSlot *pBuf;
  assert( db!=0 );
  assert( sqlite3_mutex_held(db->mutex) );
  assert( db->pnBytesFreed==0 );
  якщо( n>db->lookaside.sz ){
    якщо( !db->lookaside.bDisable ){
      db->lookaside.anStat[1]++;      
    }інакше якщо( db->mallocFailed ){
      повернути 0;
    }
    повернути dbMallocRawFinish(db, n);
  }
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
  якщо( n<=LOOKASIDE_SMALL ){
    якщо( (pBuf = db->lookaside.pSmallFree)!=0 ){
      db->lookaside.pSmallFree = pBuf->pNext;
      db->lookaside.anStat[0]++;
      повернути (void*)pBuf;
    }інакше якщо( (pBuf = db->lookaside.pSmallInit)!=0 ){
      db->lookaside.pSmallInit = pBuf->pNext;
      db->lookaside.anStat[0]++;
      повернути (void*)pBuf;
    }
  }
#endif
  якщо( (pBuf = db->lookaside.pFree)!=0 ){
    db->lookaside.pFree = pBuf->pNext;
    db->lookaside.anStat[0]++;
    повернути (void*)pBuf;
  }інакше якщо( (pBuf = db->lookaside.pInit)!=0 ){
    db->lookaside.pInit = pBuf->pNext;
    db->lookaside.anStat[0]++;
    повернути (void*)pBuf;
  }інакше{
    db->lookaside.anStat[2]++;
  }
#інакше
  assert( db!=0 );
  assert( sqlite3_mutex_held(db->mutex) );
  assert( db->pnBytesFreed==0 );
  якщо( db->mallocFailed ){
    повернути 0;
  }
#endif
  повернути dbMallocRawFinish(db, n);
}

/* Forward declaration */
static SQLITE_NOINLINE void *dbReallocFinish(sqlite3 *db, void *p, u64 n);

/*
** Resize the block of memory pointed to by p to n bytes. If the
** resize fails, set the mallocFailed flag in the connection object.
*/
void *sqlite3DbRealloc(sqlite3 *db, void *p, u64 n){
  assert( db!=0 );
  якщо( p==0 ) повернути sqlite3DbMallocRawNN(db, n);
  assert( sqlite3_mutex_held(db->mutex) );
  якщо( ((uptr)p)<(uptr)db->lookaside.pEnd ){
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
    якщо( ((uptr)p)>=(uptr)db->lookaside.pMiddle ){
      якщо( n<=LOOKASIDE_SMALL ) повернути p;
    }інакше
#endif
    якщо( ((uptr)p)>=(uptr)db->lookaside.pStart ){
      якщо( n<=db->lookaside.szTrue ) повернути p;
    }
  }
  повернути dbReallocFinish(db, p, n);
}
static SQLITE_NOINLINE void *dbReallocFinish(sqlite3 *db, void *p, u64 n){
  void *pNew = 0;
  assert( db!=0 );
  assert( p!=0 );
  якщо( db->mallocFailed==0 ){
    якщо( isLookaside(db, p) ){
      pNew = sqlite3DbMallocRawNN(db, n);
      якщо( pNew ){
        memcpy(pNew, p, lookasideMallocSize(db, p));
        sqlite3DbFree(db, p);
      }
    }інакше{
      assert( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
      assert( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
      sqlite3MemdebugSetType(p, MEMTYPE_HEAP);
      pNew = sqlite3Realloc(p, n);
      якщо( !pNew ){
        sqlite3OomFault(db);
      }
      sqlite3MemdebugSetType(pNew,
            (db->lookaside.bDisable==0 ? MEMTYPE_LOOKASIDE : MEMTYPE_HEAP));
    }
  }
  повернути pNew;
}

/*
** Attempt to reallocate p.  If the reallocation fails, then free p
** and set the mallocFailed flag in the database connection.
*/
void *sqlite3DbReallocOrFree(sqlite3 *db, void *p, u64 n){
  void *pNew;
  pNew = sqlite3DbRealloc(db, p, n);
  якщо( !pNew ){
    sqlite3DbFree(db, p);
  }
  повернути pNew;
}

/*
** Make a copy of a string in memory obtained from sqliteMalloc(). These 
** functions call sqlite3MallocRaw() directly instead of sqliteMalloc(). This
** is because when memory debugging is turned on, these two functions are 
** called via macros that record the current file and line number in the
** ThreadData structure.
*/
char *sqlite3DbStrDup(sqlite3 *db, const char *z){
  char *zNew;
  size_t n;
  якщо( z==0 ){
    повернути 0;
  }
  n = strlen(z) + 1;
  zNew = sqlite3DbMallocRaw(db, n);
  якщо( zNew ){
    memcpy(zNew, z, n);
  }
  повернути zNew;
}
char *sqlite3DbStrNDup(sqlite3 *db, const char *z, u64 n){
  char *zNew;
  assert( db!=0 );
  assert( z!=0 || n==0 );
  assert( (n&0x7fffffff)==n );
  zNew = z ? sqlite3DbMallocRawNN(db, n+1) : 0;
  якщо( zNew ){
    memcpy(zNew, z, (size_t)n);
    zNew[n] = 0;
  }
  повернути zNew;
}

/*
** The text between zStart and zEnd represents a phrase within a larger
** SQL statement.  Make a copy of this phrase in space obtained form
** sqlite3DbMalloc().  Omit leading and trailing whitespace.
*/
char *sqlite3DbSpanDup(sqlite3 *db, const char *zStart, const char *zEnd){
  ціле n;
#ifdef SQLITE_DEBUG
  /* Because of the way the parser works, the span is guaranteed to contain
  ** at least one non-space character */
  для(n=0; sqlite3Isspace(zStart[n]); n++){ assert( &zStart[n]<zEnd ); }
#endif
  поки( sqlite3Isspace(zStart[0]) ) zStart++;
  n = (ціле)(zEnd - zStart);
  поки( sqlite3Isspace(zStart[n-1]) ) n--;
  повернути sqlite3DbStrNDup(db, zStart, n);
}

/*
** Free any prior content in *pz and replace it with a copy of zNew.
*/
void sqlite3SetString(char **pz, sqlite3 *db, const char *zNew){
  char *z = sqlite3DbStrDup(db, zNew);
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
void *sqlite3OomFault(sqlite3 *db){
  якщо( db->mallocFailed==0 && db->bBenignMalloc==0 ){
    db->mallocFailed = 1;
    якщо( db->nVdbeExec>0 ){
      AtomicStore(&db->u1.isInterrupted, 1);
    }
    DisableLookaside;
    якщо( db->pParse ){
      Parse *pParse;
      sqlite3ErrorMsg(db->pParse, "out of memory");
      db->pParse->rc = SQLITE_NOMEM_BKPT;
      для(pParse=db->pParse->pOuterParse; pParse; pParse = pParse->pOuterParse){
        pParse->nErr++;
        pParse->rc = SQLITE_NOMEM;
      } 
    }
  }
  повернути 0;
}

/*
** This routine reactivates the memory allocator and clears the
** db->mallocFailed flag as necessary.
**
** The memory allocator is not restarted if there are running
** VDBEs.
*/
void sqlite3OomClear(sqlite3 *db){
  якщо( db->mallocFailed && db->nVdbeExec==0 ){
    db->mallocFailed = 0;
    AtomicStore(&db->u1.isInterrupted, 0);
    assert( db->lookaside.bDisable>0 );
    EnableLookaside;
  }
}

/*
** Take actions at the end of an API call to deal with error codes.
*/
static SQLITE_NOINLINE ціле apiHandleError(sqlite3 *db, ціле rc){
  якщо( db->mallocFailed || rc==SQLITE_IOERR_NOMEM ){
    sqlite3OomClear(db);
    sqlite3Error(db, SQLITE_NOMEM);
    повернути SQLITE_NOMEM_BKPT;
  }
  повернути rc & db->errMask;
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
ціле sqlite3ApiExit(sqlite3* db, ціле rc){
  /* If the db handle must hold the connection handle mutex here.
  ** Otherwise the read (and possible write) of db->mallocFailed 
  ** is unsafe, as is the call to sqlite3Error().
  */
  assert( db!=0 );
  assert( sqlite3_mutex_held(db->mutex) );
  якщо( db->mallocFailed || rc ){
    повернути apiHandleError(db, rc);
  }
  повернути 0;
}
