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
정수 sqlite3_release_memory(정수 n){
#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
  반환 sqlite3PcacheReleaseMemory(n);
#아니면
  /* IMPLEMENTATION-OF: R-34391-24921 The sqlite3_release_memory() routine
  ** is a no-op returning zero if SQLite is not compiled with
  ** SQLITE_ENABLE_MEMORY_MANAGEMENT. */
  UNUSED_PARAMETER(n);
  반환 0;
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
  정수 nearlyFull;
} mem0 = { 0, SQLITE_MAX_MEMORY, SQLITE_MAX_MEMORY, 0 };

#define mem0 GLOBAL(struct Mem0Global, mem0)

/*
** Return the memory allocator mutex. sqlite3_status() needs it.
*/
sqlite3_mutex *sqlite3MallocMutex(void){
  반환 mem0.mutex;
}

#ifndef SQLITE_OMIT_DEPRECATED
/*
** Deprecated external interface.  It used to set an alarm callback
** that was invoked when memory usage grew too large.  Now it is a
** no-op.
*/
정수 sqlite3_memory_alarm(
  void(*xCallback)(void *pArg, sqlite3_int64 used,정수 N),
  void *pArg,
  sqlite3_int64 iThreshold
){
  (void)xCallback;
  (void)pArg;
  (void)iThreshold;
  반환 SQLITE_OK;
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
  정수 rc = sqlite3_initialize();
  만약( rc ) 반환 -1;
#endif
  sqlite3_mutex_enter(mem0.mutex);
  priorLimit = mem0.alarmThreshold;
  만약( n<0 ){
    sqlite3_mutex_leave(mem0.mutex);
    반환 priorLimit;
  }
  만약( mem0.hardLimit>0 && (n>mem0.hardLimit || n==0) ){
    n = mem0.hardLimit;
  }
  mem0.alarmThreshold = n;
  nUsed = sqlite3StatusValue(SQLITE_STATUS_MEMORY_USED);
  AtomicStore(&mem0.nearlyFull, n>0 && n<=nUsed);
  sqlite3_mutex_leave(mem0.mutex);
  excess = sqlite3_memory_used() - n;
  만약( excess>0 ) sqlite3_release_memory((정수)(excess & 0x7fffffff));
  반환 priorLimit;
}
void sqlite3_soft_heap_limit(정수 n){
  만약( n<0 ) n = 0;
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
  정수 rc = sqlite3_initialize();
  만약( rc ) 반환 -1;
#endif
  sqlite3_mutex_enter(mem0.mutex);
  priorLimit = mem0.hardLimit;
  만약( n>=0 ){
    mem0.hardLimit = n;
    만약( n<mem0.alarmThreshold || mem0.alarmThreshold==0 ){
      mem0.alarmThreshold = n;
    }
  }
  sqlite3_mutex_leave(mem0.mutex);
  반환 priorLimit;
}


/*
** Initialize the memory allocation subsystem.
*/
정수 sqlite3MallocInit(void){
  정수 rc;
  만약( sqlite3GlobalConfig.m.xMalloc==0 ){
    sqlite3MemSetDefault();
  }
  mem0.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM);
  만약( sqlite3GlobalConfig.pPage==0 || sqlite3GlobalConfig.szPage<512
      || sqlite3GlobalConfig.nPage<=0 ){
    sqlite3GlobalConfig.pPage = 0;
    sqlite3GlobalConfig.szPage = 0;
  }
  rc = sqlite3GlobalConfig.m.xInit(sqlite3GlobalConfig.m.pAppData);
  만약( rc!=SQLITE_OK ) memset(&mem0, 0, sizeof(mem0));
  반환 rc;
}

/*
** Return true if the heap is currently under memory pressure - in other
** words if the amount of heap used is close to the limit set by
** sqlite3_soft_heap_limit().
*/
정수 sqlite3HeapNearlyFull(void){
  반환 AtomicLoad(&mem0.nearlyFull);
}

/*
** Deinitialize the memory allocation subsystem.
*/
void sqlite3MallocEnd(void){
  만약( sqlite3GlobalConfig.m.xShutdown ){
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
  반환 res;
}

/*
** Return the maximum amount of memory that has ever been
** checked out since either the beginning of this process
** or since the most recent reset.
*/
sqlite3_int64 sqlite3_memory_highwater(정수 resetFlag){
  sqlite3_int64 res, mx;
  sqlite3_status64(SQLITE_STATUS_MEMORY_USED, &res, &mx, resetFlag);
  반환 mx;
}

/*
** Trigger the alarm 
*/
static void sqlite3MallocAlarm(정수 nByte){
  만약( mem0.alarmThreshold<=0 ) 반환;
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
#아니면
# define test_oom_breakpoint(X)   /* No-op for production builds */
#endif

/*
** Do a memory allocation with statistics and alarms.  Assume the
** lock is already held.
*/
static void mallocWithAlarm(정수 n, void **pp){
  void *p;
  정수 nFull;
  assert( sqlite3_mutex_held(mem0.mutex) );
  assert( n>0 );

  /* In Firefox (circa 2017-02-08), xRoundup() is remapped to an internal
  ** implementation of malloc_good_size(), which must be called in debug
  ** mode and specifically when the DMD "Dark Matter Detector" is enabled
  ** or else a crash results.  Hence, do not attempt to optimize out the
  ** following xRoundup() call. */
  nFull = sqlite3GlobalConfig.m.xRoundup(n);

  sqlite3StatusHighwater(SQLITE_STATUS_MALLOC_SIZE, n);
  만약( mem0.alarmThreshold>0 ){
    sqlite3_int64 nUsed = sqlite3StatusValue(SQLITE_STATUS_MEMORY_USED);
    만약( nUsed >= mem0.alarmThreshold - nFull ){
      AtomicStore(&mem0.nearlyFull, 1);
      sqlite3MallocAlarm(nFull);
      만약( mem0.hardLimit ){
        nUsed = sqlite3StatusValue(SQLITE_STATUS_MEMORY_USED);
        만약( nUsed >= mem0.hardLimit - nFull ){
          test_oom_breakpoint(1);
          *pp = 0;
          반환;
        }
      }
    }아니면{
      AtomicStore(&mem0.nearlyFull, 0);
    }
  }
  p = sqlite3GlobalConfig.m.xMalloc(nFull);
#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
  만약( p==0 && mem0.alarmThreshold>0 ){
    sqlite3MallocAlarm(nFull);
    p = sqlite3GlobalConfig.m.xMalloc(nFull);
  }
#endif
  만약( p ){
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
  만약( n==0 || n>SQLITE_MAX_ALLOCATION_SIZE ){
    p = 0;
  }아니면 만약( sqlite3GlobalConfig.bMemstat ){
    sqlite3_mutex_enter(mem0.mutex);
    mallocWithAlarm((정수)n, &p);
    sqlite3_mutex_leave(mem0.mutex);
  }아니면{
    p = sqlite3GlobalConfig.m.xMalloc((정수)n);
  }
  assert( EIGHT_BYTE_ALIGNMENT(p) );  /* IMP: R-11148-40995 */
  반환 p;
}

/*
** This version of the memory allocation is for use by the application.
** First make sure the memory subsystem is initialized, then do the
** allocation.
*/
void *sqlite3_malloc(정수 n){
#ifndef SQLITE_OMIT_AUTOINIT
  만약( sqlite3_initialize() ) 반환 0;
#endif
  반환 n<=0 ? 0 : sqlite3Malloc(n);
}
void *sqlite3_malloc64(sqlite3_uint64 n){
#ifndef SQLITE_OMIT_AUTOINIT
  만약( sqlite3_initialize() ) 반환 0;
#endif
  반환 sqlite3Malloc(n);
}

/*
** TRUE if p is a lookaside memory allocation from db
*/
#ifndef SQLITE_OMIT_LOOKASIDE
static 정수 isLookaside(sqlite3 *db, const void *p){
  반환 SQLITE_WITHIN(p, db->lookaside.pStart, db->lookaside.pTrueEnd);
}
#아니면
#define isLookaside(A,B) 0
#endif

/*
** Return the size of a memory allocation previously obtained from
** sqlite3Malloc() or sqlite3_malloc().
*/
정수 sqlite3MallocSize(const void *p){
  assert( sqlite3MemdebugHasType(p, MEMTYPE_HEAP) );
  반환 sqlite3GlobalConfig.m.xSize((void*)p);
}
static 정수 lookasideMallocSize(sqlite3 *db, const void *p){
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE    
  반환 p<db->lookaside.pMiddle ? db->lookaside.szTrue : LOOKASIDE_SMALL;
#아니면
  반환 db->lookaside.szTrue;
#endif  
}
정수 sqlite3DbMallocSize(sqlite3 *db, const void *p){
  assert( p!=0 );
#ifdef SQLITE_DEBUG
  만약( db==0 ){
    assert( sqlite3MemdebugNoType(p, (u8)~MEMTYPE_HEAP) );
    assert( sqlite3MemdebugHasType(p, MEMTYPE_HEAP) );
  }아니면 만약( !isLookaside(db,p) ){
    assert( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
    assert( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  }
#endif
  만약( db ){
    만약( ((uptr)p)<(uptr)(db->lookaside.pTrueEnd) ){
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
      만약( ((uptr)p)>=(uptr)(db->lookaside.pMiddle) ){
        assert( sqlite3_mutex_held(db->mutex) );
        반환 LOOKASIDE_SMALL;
      }
#endif
      만약( ((uptr)p)>=(uptr)(db->lookaside.pStart) ){
        assert( sqlite3_mutex_held(db->mutex) );
        반환 db->lookaside.szTrue;
      }
    }
  }
  반환 sqlite3GlobalConfig.m.xSize((void*)p);
}
sqlite3_uint64 sqlite3_msize(void *p){
  assert( sqlite3MemdebugNoType(p, (u8)~MEMTYPE_HEAP) );
  assert( sqlite3MemdebugHasType(p, MEMTYPE_HEAP) );
  반환 p ? sqlite3GlobalConfig.m.xSize(p) : 0;
}

/*
** Free memory previously obtained from sqlite3Malloc().
*/
void sqlite3_free(void *p){
  만약( p==0 ) 반환;  /* IMP: R-49053-54554 */
  assert( sqlite3MemdebugHasType(p, MEMTYPE_HEAP) );
  assert( sqlite3MemdebugNoType(p, (u8)~MEMTYPE_HEAP) );
  만약( sqlite3GlobalConfig.bMemstat ){
    sqlite3_mutex_enter(mem0.mutex);
    sqlite3StatusDown(SQLITE_STATUS_MEMORY_USED, sqlite3MallocSize(p));
    sqlite3StatusDown(SQLITE_STATUS_MALLOC_COUNT, 1);
    sqlite3GlobalConfig.m.xFree(p);
    sqlite3_mutex_leave(mem0.mutex);
  }아니면{
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
  만약( db ){
    만약( ((uptr)p)<(uptr)(db->lookaside.pEnd) ){
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
      만약( ((uptr)p)>=(uptr)(db->lookaside.pMiddle) ){
        LookasideSlot *pBuf = (LookasideSlot*)p;
        assert( db->pnBytesFreed==0 );
#ifdef SQLITE_DEBUG
        memset(p, 0xaa, LOOKASIDE_SMALL);  /* Trash freed content */
#endif
        pBuf->pNext = db->lookaside.pSmallFree;
        db->lookaside.pSmallFree = pBuf;
        반환;
      }
#endif /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
      만약( ((uptr)p)>=(uptr)(db->lookaside.pStart) ){
        LookasideSlot *pBuf = (LookasideSlot*)p;
        assert( db->pnBytesFreed==0 );
#ifdef SQLITE_DEBUG
        memset(p, 0xaa, db->lookaside.szTrue);  /* Trash freed content */
#endif
        pBuf->pNext = db->lookaside.pFree;
        db->lookaside.pFree = pBuf;
        반환;
      }
    }
    만약( db->pnBytesFreed ){
      measureAllocationSize(db, p);
      반환;
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
  만약( ((uptr)p)<(uptr)(db->lookaside.pEnd) ){
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
    만약( ((uptr)p)>=(uptr)(db->lookaside.pMiddle) ){
      LookasideSlot *pBuf = (LookasideSlot*)p;
      assert( db->pnBytesFreed==0 );
#ifdef SQLITE_DEBUG
      memset(p, 0xaa, LOOKASIDE_SMALL);  /* Trash freed content */
#endif
      pBuf->pNext = db->lookaside.pSmallFree;
      db->lookaside.pSmallFree = pBuf;
      반환;
    }
#endif /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
    만약( ((uptr)p)>=(uptr)(db->lookaside.pStart) ){
      LookasideSlot *pBuf = (LookasideSlot*)p;
      assert( db->pnBytesFreed==0 );
#ifdef SQLITE_DEBUG
      memset(p, 0xaa, db->lookaside.szTrue);  /* Trash freed content */
#endif
      pBuf->pNext = db->lookaside.pFree;
      db->lookaside.pFree = pBuf;
      반환;
    }
  }
  만약( db->pnBytesFreed ){
    measureAllocationSize(db, p);
    반환;
  }
  assert( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  assert( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  sqlite3MemdebugSetType(p, MEMTYPE_HEAP);
  sqlite3_free(p);
}
void sqlite3DbFree(sqlite3 *db, void *p){
  assert( db==0 || sqlite3_mutex_held(db->mutex) );
  만약( p ) sqlite3DbFreeNN(db, p);
}

/*
** Change the size of an existing memory allocation
*/
void *sqlite3Realloc(void *pOld, u64 nBytes){
  정수 nOld, nNew, nDiff;
  void *pNew;
  assert( sqlite3MemdebugHasType(pOld, MEMTYPE_HEAP) );
  assert( sqlite3MemdebugNoType(pOld, (u8)~MEMTYPE_HEAP) );
  만약( pOld==0 ){
    반환 sqlite3Malloc(nBytes); /* IMP: R-04300-56712 */
  }
  만약( nBytes==0 ){
    sqlite3_free(pOld); /* IMP: R-26507-47431 */
    반환 0;
  }
  만약( nBytes>SQLITE_MAX_ALLOCATION_SIZE ){
    반환 0;
  }
  nOld = sqlite3MallocSize(pOld);
  /* IMPLEMENTATION-OF: R-46199-30249 SQLite guarantees that the second
  ** argument to xRealloc is always a value returned by a prior call to
  ** xRoundup. */
  nNew = sqlite3GlobalConfig.m.xRoundup((정수)nBytes);
  만약( nOld==nNew ){
    pNew = pOld;
  }아니면 만약( sqlite3GlobalConfig.bMemstat ){
    sqlite3_int64 nUsed;
    sqlite3_mutex_enter(mem0.mutex);
    sqlite3StatusHighwater(SQLITE_STATUS_MALLOC_SIZE, (정수)nBytes);
    nDiff = nNew - nOld;
    만약( nDiff>0 && (nUsed = sqlite3StatusValue(SQLITE_STATUS_MEMORY_USED)) >= 
          mem0.alarmThreshold-nDiff ){
      sqlite3MallocAlarm(nDiff);
      만약( mem0.hardLimit>0 && nUsed >= mem0.hardLimit - nDiff ){
        sqlite3_mutex_leave(mem0.mutex);
        test_oom_breakpoint(1);
        반환 0;
      }
    }
    pNew = sqlite3GlobalConfig.m.xRealloc(pOld, nNew);
#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
    만약( pNew==0 && mem0.alarmThreshold>0 ){
      sqlite3MallocAlarm((정수)nBytes);
      pNew = sqlite3GlobalConfig.m.xRealloc(pOld, nNew);
    }
#endif
    만약( pNew ){
      nNew = sqlite3MallocSize(pNew);
      sqlite3StatusUp(SQLITE_STATUS_MEMORY_USED, nNew-nOld);
    }
    sqlite3_mutex_leave(mem0.mutex);
  }아니면{
    pNew = sqlite3GlobalConfig.m.xRealloc(pOld, nNew);
  }
  assert( EIGHT_BYTE_ALIGNMENT(pNew) ); /* IMP: R-11148-40995 */
  반환 pNew;
}

/*
** The public interface to sqlite3Realloc.  Make sure that the memory
** subsystem is initialized prior to invoking sqliteRealloc.
*/
void *sqlite3_realloc(void *pOld, 정수 n){
#ifndef SQLITE_OMIT_AUTOINIT
  만약( sqlite3_initialize() ) 반환 0;
#endif
  만약( n<0 ) n = 0;  /* IMP: R-26507-47431 */
  반환 sqlite3Realloc(pOld, n);
}
void *sqlite3_realloc64(void *pOld, sqlite3_uint64 n){
#ifndef SQLITE_OMIT_AUTOINIT
  만약( sqlite3_initialize() ) 반환 0;
#endif
  반환 sqlite3Realloc(pOld, n);
}


/*
** Allocate and zero memory.
*/ 
void *sqlite3MallocZero(u64 n){
  void *p = sqlite3Malloc(n);
  만약( p ){
    memset(p, 0, (size_t)n);
  }
  반환 p;
}

/*
** Allocate and zero memory.  If the allocation fails, make
** the mallocFailed flag in the connection pointer.
*/
void *sqlite3DbMallocZero(sqlite3 *db, u64 n){
  void *p;
  testcase( db==0 );
  p = sqlite3DbMallocRaw(db, n);
  만약( p ) memset(p, 0, (size_t)n);
  반환 p;
}


/* Finish the work of sqlite3DbMallocRawNN for the unusual and
** slower case when the allocation cannot be fulfilled using lookaside.
*/
static SQLITE_NOINLINE void *dbMallocRawFinish(sqlite3 *db, u64 n){
  void *p;
  assert( db!=0 );
  p = sqlite3Malloc(n);
  만약( !p ) sqlite3OomFault(db);
  sqlite3MemdebugSetType(p, 
         (db->lookaside.bDisable==0) ? MEMTYPE_LOOKASIDE : MEMTYPE_HEAP);
  반환 p;
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
  만약( db ) 반환 sqlite3DbMallocRawNN(db, n);
  p = sqlite3Malloc(n);
  sqlite3MemdebugSetType(p, MEMTYPE_HEAP);
  반환 p;
}
void *sqlite3DbMallocRawNN(sqlite3 *db, u64 n){
#ifndef SQLITE_OMIT_LOOKASIDE
  LookasideSlot *pBuf;
  assert( db!=0 );
  assert( sqlite3_mutex_held(db->mutex) );
  assert( db->pnBytesFreed==0 );
  만약( n>db->lookaside.sz ){
    만약( !db->lookaside.bDisable ){
      db->lookaside.anStat[1]++;      
    }아니면 만약( db->mallocFailed ){
      반환 0;
    }
    반환 dbMallocRawFinish(db, n);
  }
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
  만약( n<=LOOKASIDE_SMALL ){
    만약( (pBuf = db->lookaside.pSmallFree)!=0 ){
      db->lookaside.pSmallFree = pBuf->pNext;
      db->lookaside.anStat[0]++;
      반환 (void*)pBuf;
    }아니면 만약( (pBuf = db->lookaside.pSmallInit)!=0 ){
      db->lookaside.pSmallInit = pBuf->pNext;
      db->lookaside.anStat[0]++;
      반환 (void*)pBuf;
    }
  }
#endif
  만약( (pBuf = db->lookaside.pFree)!=0 ){
    db->lookaside.pFree = pBuf->pNext;
    db->lookaside.anStat[0]++;
    반환 (void*)pBuf;
  }아니면 만약( (pBuf = db->lookaside.pInit)!=0 ){
    db->lookaside.pInit = pBuf->pNext;
    db->lookaside.anStat[0]++;
    반환 (void*)pBuf;
  }아니면{
    db->lookaside.anStat[2]++;
  }
#아니면
  assert( db!=0 );
  assert( sqlite3_mutex_held(db->mutex) );
  assert( db->pnBytesFreed==0 );
  만약( db->mallocFailed ){
    반환 0;
  }
#endif
  반환 dbMallocRawFinish(db, n);
}

/* Forward declaration */
static SQLITE_NOINLINE void *dbReallocFinish(sqlite3 *db, void *p, u64 n);

/*
** Resize the block of memory pointed to by p to n bytes. If the
** resize fails, set the mallocFailed flag in the connection object.
*/
void *sqlite3DbRealloc(sqlite3 *db, void *p, u64 n){
  assert( db!=0 );
  만약( p==0 ) 반환 sqlite3DbMallocRawNN(db, n);
  assert( sqlite3_mutex_held(db->mutex) );
  만약( ((uptr)p)<(uptr)db->lookaside.pEnd ){
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
    만약( ((uptr)p)>=(uptr)db->lookaside.pMiddle ){
      만약( n<=LOOKASIDE_SMALL ) 반환 p;
    }아니면
#endif
    만약( ((uptr)p)>=(uptr)db->lookaside.pStart ){
      만약( n<=db->lookaside.szTrue ) 반환 p;
    }
  }
  반환 dbReallocFinish(db, p, n);
}
static SQLITE_NOINLINE void *dbReallocFinish(sqlite3 *db, void *p, u64 n){
  void *pNew = 0;
  assert( db!=0 );
  assert( p!=0 );
  만약( db->mallocFailed==0 ){
    만약( isLookaside(db, p) ){
      pNew = sqlite3DbMallocRawNN(db, n);
      만약( pNew ){
        memcpy(pNew, p, lookasideMallocSize(db, p));
        sqlite3DbFree(db, p);
      }
    }아니면{
      assert( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
      assert( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
      sqlite3MemdebugSetType(p, MEMTYPE_HEAP);
      pNew = sqlite3Realloc(p, n);
      만약( !pNew ){
        sqlite3OomFault(db);
      }
      sqlite3MemdebugSetType(pNew,
            (db->lookaside.bDisable==0 ? MEMTYPE_LOOKASIDE : MEMTYPE_HEAP));
    }
  }
  반환 pNew;
}

/*
** Attempt to reallocate p.  If the reallocation fails, then free p
** and set the mallocFailed flag in the database connection.
*/
void *sqlite3DbReallocOrFree(sqlite3 *db, void *p, u64 n){
  void *pNew;
  pNew = sqlite3DbRealloc(db, p, n);
  만약( !pNew ){
    sqlite3DbFree(db, p);
  }
  반환 pNew;
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
  만약( z==0 ){
    반환 0;
  }
  n = strlen(z) + 1;
  zNew = sqlite3DbMallocRaw(db, n);
  만약( zNew ){
    memcpy(zNew, z, n);
  }
  반환 zNew;
}
char *sqlite3DbStrNDup(sqlite3 *db, const char *z, u64 n){
  char *zNew;
  assert( db!=0 );
  assert( z!=0 || n==0 );
  assert( (n&0x7fffffff)==n );
  zNew = z ? sqlite3DbMallocRawNN(db, n+1) : 0;
  만약( zNew ){
    memcpy(zNew, z, (size_t)n);
    zNew[n] = 0;
  }
  반환 zNew;
}

/*
** The text between zStart and zEnd represents a phrase within a larger
** SQL statement.  Make a copy of this phrase in space obtained form
** sqlite3DbMalloc().  Omit leading and trailing whitespace.
*/
char *sqlite3DbSpanDup(sqlite3 *db, const char *zStart, const char *zEnd){
  정수 n;
#ifdef SQLITE_DEBUG
  /* Because of the way the parser works, the span is guaranteed to contain
  ** at least one non-space character */
  위해(n=0; sqlite3Isspace(zStart[n]); n++){ assert( &zStart[n]<zEnd ); }
#endif
  동안( sqlite3Isspace(zStart[0]) ) zStart++;
  n = (정수)(zEnd - zStart);
  동안( sqlite3Isspace(zStart[n-1]) ) n--;
  반환 sqlite3DbStrNDup(db, zStart, n);
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
  만약( db->mallocFailed==0 && db->bBenignMalloc==0 ){
    db->mallocFailed = 1;
    만약( db->nVdbeExec>0 ){
      AtomicStore(&db->u1.isInterrupted, 1);
    }
    DisableLookaside;
    만약( db->pParse ){
      Parse *pParse;
      sqlite3ErrorMsg(db->pParse, "out of memory");
      db->pParse->rc = SQLITE_NOMEM_BKPT;
      위해(pParse=db->pParse->pOuterParse; pParse; pParse = pParse->pOuterParse){
        pParse->nErr++;
        pParse->rc = SQLITE_NOMEM;
      } 
    }
  }
  반환 0;
}

/*
** This routine reactivates the memory allocator and clears the
** db->mallocFailed flag as necessary.
**
** The memory allocator is not restarted if there are running
** VDBEs.
*/
void sqlite3OomClear(sqlite3 *db){
  만약( db->mallocFailed && db->nVdbeExec==0 ){
    db->mallocFailed = 0;
    AtomicStore(&db->u1.isInterrupted, 0);
    assert( db->lookaside.bDisable>0 );
    EnableLookaside;
  }
}

/*
** Take actions at the end of an API call to deal with error codes.
*/
static SQLITE_NOINLINE 정수 apiHandleError(sqlite3 *db, 정수 rc){
  만약( db->mallocFailed || rc==SQLITE_IOERR_NOMEM ){
    sqlite3OomClear(db);
    sqlite3Error(db, SQLITE_NOMEM);
    반환 SQLITE_NOMEM_BKPT;
  }
  반환 rc & db->errMask;
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
정수 sqlite3ApiExit(sqlite3* db, 정수 rc){
  /* If the db handle must hold the connection handle mutex here.
  ** Otherwise the read (and possible write) of db->mallocFailed 
  ** is unsafe, as is the call to sqlite3Error().
  */
  assert( db!=0 );
  assert( sqlite3_mutex_held(db->mutex) );
  만약( db->mallocFailed || rc ){
    반환 apiHandleError(db, rc);
  }
  반환 0;
}
