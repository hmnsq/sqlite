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
số_nguyên sqlite3_release_memory(số_nguyên n){
#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
  trả_về sqlite3PcacheReleaseMemory(n);
#không_thì
  /* IMPLEMENTATION-OF: R-34391-24921 The sqlite3_release_memory() routine
  ** is a no-op returning zero if SQLite is not compiled with
  ** SQLITE_ENABLE_MEMORY_MANAGEMENT. */
  UNUSED_PARAMETER(n);
  trả_về 0;
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
  số_nguyên nearlyFull;
} mem0 = { 0, SQLITE_MAX_MEMORY, SQLITE_MAX_MEMORY, 0 };

#define mem0 GLOBAL(struct Mem0Global, mem0)

/*
** Return the memory allocator mutex. sqlite3_status() needs it.
*/
sqlite3_mutex *sqlite3MallocMutex(void){
  trả_về mem0.mutex;
}

#ifndef SQLITE_OMIT_DEPRECATED
/*
** Deprecated external interface.  It used to set an alarm callback
** that was invoked when memory usage grew too large.  Now it is a
** no-op.
*/
số_nguyên sqlite3_memory_alarm(
  void(*xCallback)(void *pArg, sqlite3_int64 used,số_nguyên N),
  void *pArg,
  sqlite3_int64 iThreshold
){
  (void)xCallback;
  (void)pArg;
  (void)iThreshold;
  trả_về SQLITE_OK;
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
  số_nguyên rc = sqlite3_initialize();
  nếu( rc ) trả_về -1;
#endif
  sqlite3_mutex_enter(mem0.mutex);
  priorLimit = mem0.alarmThreshold;
  nếu( n<0 ){
    sqlite3_mutex_leave(mem0.mutex);
    trả_về priorLimit;
  }
  nếu( mem0.hardLimit>0 && (n>mem0.hardLimit || n==0) ){
    n = mem0.hardLimit;
  }
  mem0.alarmThreshold = n;
  nUsed = sqlite3StatusValue(SQLITE_STATUS_MEMORY_USED);
  AtomicStore(&mem0.nearlyFull, n>0 && n<=nUsed);
  sqlite3_mutex_leave(mem0.mutex);
  excess = sqlite3_memory_used() - n;
  nếu( excess>0 ) sqlite3_release_memory((số_nguyên)(excess & 0x7fffffff));
  trả_về priorLimit;
}
void sqlite3_soft_heap_limit(số_nguyên n){
  nếu( n<0 ) n = 0;
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
  số_nguyên rc = sqlite3_initialize();
  nếu( rc ) trả_về -1;
#endif
  sqlite3_mutex_enter(mem0.mutex);
  priorLimit = mem0.hardLimit;
  nếu( n>=0 ){
    mem0.hardLimit = n;
    nếu( n<mem0.alarmThreshold || mem0.alarmThreshold==0 ){
      mem0.alarmThreshold = n;
    }
  }
  sqlite3_mutex_leave(mem0.mutex);
  trả_về priorLimit;
}


/*
** Initialize the memory allocation subsystem.
*/
số_nguyên sqlite3MallocInit(void){
  số_nguyên rc;
  nếu( sqlite3GlobalConfig.m.xMalloc==0 ){
    sqlite3MemSetDefault();
  }
  mem0.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM);
  nếu( sqlite3GlobalConfig.pPage==0 || sqlite3GlobalConfig.szPage<512
      || sqlite3GlobalConfig.nPage<=0 ){
    sqlite3GlobalConfig.pPage = 0;
    sqlite3GlobalConfig.szPage = 0;
  }
  rc = sqlite3GlobalConfig.m.xInit(sqlite3GlobalConfig.m.pAppData);
  nếu( rc!=SQLITE_OK ) memset(&mem0, 0, sizeof(mem0));
  trả_về rc;
}

/*
** Return true if the heap is currently under memory pressure - in other
** words if the amount of heap used is close to the limit set by
** sqlite3_soft_heap_limit().
*/
số_nguyên sqlite3HeapNearlyFull(void){
  trả_về AtomicLoad(&mem0.nearlyFull);
}

/*
** Deinitialize the memory allocation subsystem.
*/
void sqlite3MallocEnd(void){
  nếu( sqlite3GlobalConfig.m.xShutdown ){
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
  trả_về res;
}

/*
** Return the maximum amount of memory that has ever been
** checked out since either the beginning of this process
** or since the most recent reset.
*/
sqlite3_int64 sqlite3_memory_highwater(số_nguyên resetFlag){
  sqlite3_int64 res, mx;
  sqlite3_status64(SQLITE_STATUS_MEMORY_USED, &res, &mx, resetFlag);
  trả_về mx;
}

/*
** Trigger the alarm 
*/
static void sqlite3MallocAlarm(số_nguyên nByte){
  nếu( mem0.alarmThreshold<=0 ) trả_về;
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
#không_thì
# define test_oom_breakpoint(X)   /* No-op for production builds */
#endif

/*
** Do a memory allocation with statistics and alarms.  Assume the
** lock is already held.
*/
static void mallocWithAlarm(số_nguyên n, void **pp){
  void *p;
  số_nguyên nFull;
  assert( sqlite3_mutex_held(mem0.mutex) );
  assert( n>0 );

  /* In Firefox (circa 2017-02-08), xRoundup() is remapped to an internal
  ** implementation of malloc_good_size(), which must be called in debug
  ** mode and specifically when the DMD "Dark Matter Detector" is enabled
  ** or else a crash results.  Hence, do not attempt to optimize out the
  ** following xRoundup() call. */
  nFull = sqlite3GlobalConfig.m.xRoundup(n);

  sqlite3StatusHighwater(SQLITE_STATUS_MALLOC_SIZE, n);
  nếu( mem0.alarmThreshold>0 ){
    sqlite3_int64 nUsed = sqlite3StatusValue(SQLITE_STATUS_MEMORY_USED);
    nếu( nUsed >= mem0.alarmThreshold - nFull ){
      AtomicStore(&mem0.nearlyFull, 1);
      sqlite3MallocAlarm(nFull);
      nếu( mem0.hardLimit ){
        nUsed = sqlite3StatusValue(SQLITE_STATUS_MEMORY_USED);
        nếu( nUsed >= mem0.hardLimit - nFull ){
          test_oom_breakpoint(1);
          *pp = 0;
          trả_về;
        }
      }
    }không_thì{
      AtomicStore(&mem0.nearlyFull, 0);
    }
  }
  p = sqlite3GlobalConfig.m.xMalloc(nFull);
#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
  nếu( p==0 && mem0.alarmThreshold>0 ){
    sqlite3MallocAlarm(nFull);
    p = sqlite3GlobalConfig.m.xMalloc(nFull);
  }
#endif
  nếu( p ){
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
  nếu( n==0 || n>SQLITE_MAX_ALLOCATION_SIZE ){
    p = 0;
  }không_thì nếu( sqlite3GlobalConfig.bMemstat ){
    sqlite3_mutex_enter(mem0.mutex);
    mallocWithAlarm((số_nguyên)n, &p);
    sqlite3_mutex_leave(mem0.mutex);
  }không_thì{
    p = sqlite3GlobalConfig.m.xMalloc((số_nguyên)n);
  }
  assert( EIGHT_BYTE_ALIGNMENT(p) );  /* IMP: R-11148-40995 */
  trả_về p;
}

/*
** This version of the memory allocation is for use by the application.
** First make sure the memory subsystem is initialized, then do the
** allocation.
*/
void *sqlite3_malloc(số_nguyên n){
#ifndef SQLITE_OMIT_AUTOINIT
  nếu( sqlite3_initialize() ) trả_về 0;
#endif
  trả_về n<=0 ? 0 : sqlite3Malloc(n);
}
void *sqlite3_malloc64(sqlite3_uint64 n){
#ifndef SQLITE_OMIT_AUTOINIT
  nếu( sqlite3_initialize() ) trả_về 0;
#endif
  trả_về sqlite3Malloc(n);
}

/*
** TRUE if p is a lookaside memory allocation from db
*/
#ifndef SQLITE_OMIT_LOOKASIDE
static số_nguyên isLookaside(sqlite3 *db, const void *p){
  trả_về SQLITE_WITHIN(p, db->lookaside.pStart, db->lookaside.pTrueEnd);
}
#không_thì
#define isLookaside(A,B) 0
#endif

/*
** Return the size of a memory allocation previously obtained from
** sqlite3Malloc() or sqlite3_malloc().
*/
số_nguyên sqlite3MallocSize(const void *p){
  assert( sqlite3MemdebugHasType(p, MEMTYPE_HEAP) );
  trả_về sqlite3GlobalConfig.m.xSize((void*)p);
}
static số_nguyên lookasideMallocSize(sqlite3 *db, const void *p){
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE    
  trả_về p<db->lookaside.pMiddle ? db->lookaside.szTrue : LOOKASIDE_SMALL;
#không_thì
  trả_về db->lookaside.szTrue;
#endif  
}
số_nguyên sqlite3DbMallocSize(sqlite3 *db, const void *p){
  assert( p!=0 );
#ifdef SQLITE_DEBUG
  nếu( db==0 ){
    assert( sqlite3MemdebugNoType(p, (u8)~MEMTYPE_HEAP) );
    assert( sqlite3MemdebugHasType(p, MEMTYPE_HEAP) );
  }không_thì nếu( !isLookaside(db,p) ){
    assert( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
    assert( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  }
#endif
  nếu( db ){
    nếu( ((uptr)p)<(uptr)(db->lookaside.pTrueEnd) ){
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
      nếu( ((uptr)p)>=(uptr)(db->lookaside.pMiddle) ){
        assert( sqlite3_mutex_held(db->mutex) );
        trả_về LOOKASIDE_SMALL;
      }
#endif
      nếu( ((uptr)p)>=(uptr)(db->lookaside.pStart) ){
        assert( sqlite3_mutex_held(db->mutex) );
        trả_về db->lookaside.szTrue;
      }
    }
  }
  trả_về sqlite3GlobalConfig.m.xSize((void*)p);
}
sqlite3_uint64 sqlite3_msize(void *p){
  assert( sqlite3MemdebugNoType(p, (u8)~MEMTYPE_HEAP) );
  assert( sqlite3MemdebugHasType(p, MEMTYPE_HEAP) );
  trả_về p ? sqlite3GlobalConfig.m.xSize(p) : 0;
}

/*
** Free memory previously obtained from sqlite3Malloc().
*/
void sqlite3_free(void *p){
  nếu( p==0 ) trả_về;  /* IMP: R-49053-54554 */
  assert( sqlite3MemdebugHasType(p, MEMTYPE_HEAP) );
  assert( sqlite3MemdebugNoType(p, (u8)~MEMTYPE_HEAP) );
  nếu( sqlite3GlobalConfig.bMemstat ){
    sqlite3_mutex_enter(mem0.mutex);
    sqlite3StatusDown(SQLITE_STATUS_MEMORY_USED, sqlite3MallocSize(p));
    sqlite3StatusDown(SQLITE_STATUS_MALLOC_COUNT, 1);
    sqlite3GlobalConfig.m.xFree(p);
    sqlite3_mutex_leave(mem0.mutex);
  }không_thì{
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
  nếu( db ){
    nếu( ((uptr)p)<(uptr)(db->lookaside.pEnd) ){
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
      nếu( ((uptr)p)>=(uptr)(db->lookaside.pMiddle) ){
        LookasideSlot *pBuf = (LookasideSlot*)p;
        assert( db->pnBytesFreed==0 );
#ifdef SQLITE_DEBUG
        memset(p, 0xaa, LOOKASIDE_SMALL);  /* Trash freed content */
#endif
        pBuf->pNext = db->lookaside.pSmallFree;
        db->lookaside.pSmallFree = pBuf;
        trả_về;
      }
#endif /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
      nếu( ((uptr)p)>=(uptr)(db->lookaside.pStart) ){
        LookasideSlot *pBuf = (LookasideSlot*)p;
        assert( db->pnBytesFreed==0 );
#ifdef SQLITE_DEBUG
        memset(p, 0xaa, db->lookaside.szTrue);  /* Trash freed content */
#endif
        pBuf->pNext = db->lookaside.pFree;
        db->lookaside.pFree = pBuf;
        trả_về;
      }
    }
    nếu( db->pnBytesFreed ){
      measureAllocationSize(db, p);
      trả_về;
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
  nếu( ((uptr)p)<(uptr)(db->lookaside.pEnd) ){
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
    nếu( ((uptr)p)>=(uptr)(db->lookaside.pMiddle) ){
      LookasideSlot *pBuf = (LookasideSlot*)p;
      assert( db->pnBytesFreed==0 );
#ifdef SQLITE_DEBUG
      memset(p, 0xaa, LOOKASIDE_SMALL);  /* Trash freed content */
#endif
      pBuf->pNext = db->lookaside.pSmallFree;
      db->lookaside.pSmallFree = pBuf;
      trả_về;
    }
#endif /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
    nếu( ((uptr)p)>=(uptr)(db->lookaside.pStart) ){
      LookasideSlot *pBuf = (LookasideSlot*)p;
      assert( db->pnBytesFreed==0 );
#ifdef SQLITE_DEBUG
      memset(p, 0xaa, db->lookaside.szTrue);  /* Trash freed content */
#endif
      pBuf->pNext = db->lookaside.pFree;
      db->lookaside.pFree = pBuf;
      trả_về;
    }
  }
  nếu( db->pnBytesFreed ){
    measureAllocationSize(db, p);
    trả_về;
  }
  assert( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  assert( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  sqlite3MemdebugSetType(p, MEMTYPE_HEAP);
  sqlite3_free(p);
}
void sqlite3DbFree(sqlite3 *db, void *p){
  assert( db==0 || sqlite3_mutex_held(db->mutex) );
  nếu( p ) sqlite3DbFreeNN(db, p);
}

/*
** Change the size of an existing memory allocation
*/
void *sqlite3Realloc(void *pOld, u64 nBytes){
  số_nguyên nOld, nNew, nDiff;
  void *pNew;
  assert( sqlite3MemdebugHasType(pOld, MEMTYPE_HEAP) );
  assert( sqlite3MemdebugNoType(pOld, (u8)~MEMTYPE_HEAP) );
  nếu( pOld==0 ){
    trả_về sqlite3Malloc(nBytes); /* IMP: R-04300-56712 */
  }
  nếu( nBytes==0 ){
    sqlite3_free(pOld); /* IMP: R-26507-47431 */
    trả_về 0;
  }
  nếu( nBytes>SQLITE_MAX_ALLOCATION_SIZE ){
    trả_về 0;
  }
  nOld = sqlite3MallocSize(pOld);
  /* IMPLEMENTATION-OF: R-46199-30249 SQLite guarantees that the second
  ** argument to xRealloc is always a value returned by a prior call to
  ** xRoundup. */
  nNew = sqlite3GlobalConfig.m.xRoundup((số_nguyên)nBytes);
  nếu( nOld==nNew ){
    pNew = pOld;
  }không_thì nếu( sqlite3GlobalConfig.bMemstat ){
    sqlite3_int64 nUsed;
    sqlite3_mutex_enter(mem0.mutex);
    sqlite3StatusHighwater(SQLITE_STATUS_MALLOC_SIZE, (số_nguyên)nBytes);
    nDiff = nNew - nOld;
    nếu( nDiff>0 && (nUsed = sqlite3StatusValue(SQLITE_STATUS_MEMORY_USED)) >= 
          mem0.alarmThreshold-nDiff ){
      sqlite3MallocAlarm(nDiff);
      nếu( mem0.hardLimit>0 && nUsed >= mem0.hardLimit - nDiff ){
        sqlite3_mutex_leave(mem0.mutex);
        test_oom_breakpoint(1);
        trả_về 0;
      }
    }
    pNew = sqlite3GlobalConfig.m.xRealloc(pOld, nNew);
#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
    nếu( pNew==0 && mem0.alarmThreshold>0 ){
      sqlite3MallocAlarm((số_nguyên)nBytes);
      pNew = sqlite3GlobalConfig.m.xRealloc(pOld, nNew);
    }
#endif
    nếu( pNew ){
      nNew = sqlite3MallocSize(pNew);
      sqlite3StatusUp(SQLITE_STATUS_MEMORY_USED, nNew-nOld);
    }
    sqlite3_mutex_leave(mem0.mutex);
  }không_thì{
    pNew = sqlite3GlobalConfig.m.xRealloc(pOld, nNew);
  }
  assert( EIGHT_BYTE_ALIGNMENT(pNew) ); /* IMP: R-11148-40995 */
  trả_về pNew;
}

/*
** The public interface to sqlite3Realloc.  Make sure that the memory
** subsystem is initialized prior to invoking sqliteRealloc.
*/
void *sqlite3_realloc(void *pOld, số_nguyên n){
#ifndef SQLITE_OMIT_AUTOINIT
  nếu( sqlite3_initialize() ) trả_về 0;
#endif
  nếu( n<0 ) n = 0;  /* IMP: R-26507-47431 */
  trả_về sqlite3Realloc(pOld, n);
}
void *sqlite3_realloc64(void *pOld, sqlite3_uint64 n){
#ifndef SQLITE_OMIT_AUTOINIT
  nếu( sqlite3_initialize() ) trả_về 0;
#endif
  trả_về sqlite3Realloc(pOld, n);
}


/*
** Allocate and zero memory.
*/ 
void *sqlite3MallocZero(u64 n){
  void *p = sqlite3Malloc(n);
  nếu( p ){
    memset(p, 0, (size_t)n);
  }
  trả_về p;
}

/*
** Allocate and zero memory.  If the allocation fails, make
** the mallocFailed flag in the connection pointer.
*/
void *sqlite3DbMallocZero(sqlite3 *db, u64 n){
  void *p;
  testcase( db==0 );
  p = sqlite3DbMallocRaw(db, n);
  nếu( p ) memset(p, 0, (size_t)n);
  trả_về p;
}


/* Finish the work of sqlite3DbMallocRawNN for the unusual and
** slower case when the allocation cannot be fulfilled using lookaside.
*/
static SQLITE_NOINLINE void *dbMallocRawFinish(sqlite3 *db, u64 n){
  void *p;
  assert( db!=0 );
  p = sqlite3Malloc(n);
  nếu( !p ) sqlite3OomFault(db);
  sqlite3MemdebugSetType(p, 
         (db->lookaside.bDisable==0) ? MEMTYPE_LOOKASIDE : MEMTYPE_HEAP);
  trả_về p;
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
  nếu( db ) trả_về sqlite3DbMallocRawNN(db, n);
  p = sqlite3Malloc(n);
  sqlite3MemdebugSetType(p, MEMTYPE_HEAP);
  trả_về p;
}
void *sqlite3DbMallocRawNN(sqlite3 *db, u64 n){
#ifndef SQLITE_OMIT_LOOKASIDE
  LookasideSlot *pBuf;
  assert( db!=0 );
  assert( sqlite3_mutex_held(db->mutex) );
  assert( db->pnBytesFreed==0 );
  nếu( n>db->lookaside.sz ){
    nếu( !db->lookaside.bDisable ){
      db->lookaside.anStat[1]++;      
    }không_thì nếu( db->mallocFailed ){
      trả_về 0;
    }
    trả_về dbMallocRawFinish(db, n);
  }
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
  nếu( n<=LOOKASIDE_SMALL ){
    nếu( (pBuf = db->lookaside.pSmallFree)!=0 ){
      db->lookaside.pSmallFree = pBuf->pNext;
      db->lookaside.anStat[0]++;
      trả_về (void*)pBuf;
    }không_thì nếu( (pBuf = db->lookaside.pSmallInit)!=0 ){
      db->lookaside.pSmallInit = pBuf->pNext;
      db->lookaside.anStat[0]++;
      trả_về (void*)pBuf;
    }
  }
#endif
  nếu( (pBuf = db->lookaside.pFree)!=0 ){
    db->lookaside.pFree = pBuf->pNext;
    db->lookaside.anStat[0]++;
    trả_về (void*)pBuf;
  }không_thì nếu( (pBuf = db->lookaside.pInit)!=0 ){
    db->lookaside.pInit = pBuf->pNext;
    db->lookaside.anStat[0]++;
    trả_về (void*)pBuf;
  }không_thì{
    db->lookaside.anStat[2]++;
  }
#không_thì
  assert( db!=0 );
  assert( sqlite3_mutex_held(db->mutex) );
  assert( db->pnBytesFreed==0 );
  nếu( db->mallocFailed ){
    trả_về 0;
  }
#endif
  trả_về dbMallocRawFinish(db, n);
}

/* Forward declaration */
static SQLITE_NOINLINE void *dbReallocFinish(sqlite3 *db, void *p, u64 n);

/*
** Resize the block of memory pointed to by p to n bytes. If the
** resize fails, set the mallocFailed flag in the connection object.
*/
void *sqlite3DbRealloc(sqlite3 *db, void *p, u64 n){
  assert( db!=0 );
  nếu( p==0 ) trả_về sqlite3DbMallocRawNN(db, n);
  assert( sqlite3_mutex_held(db->mutex) );
  nếu( ((uptr)p)<(uptr)db->lookaside.pEnd ){
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
    nếu( ((uptr)p)>=(uptr)db->lookaside.pMiddle ){
      nếu( n<=LOOKASIDE_SMALL ) trả_về p;
    }không_thì
#endif
    nếu( ((uptr)p)>=(uptr)db->lookaside.pStart ){
      nếu( n<=db->lookaside.szTrue ) trả_về p;
    }
  }
  trả_về dbReallocFinish(db, p, n);
}
static SQLITE_NOINLINE void *dbReallocFinish(sqlite3 *db, void *p, u64 n){
  void *pNew = 0;
  assert( db!=0 );
  assert( p!=0 );
  nếu( db->mallocFailed==0 ){
    nếu( isLookaside(db, p) ){
      pNew = sqlite3DbMallocRawNN(db, n);
      nếu( pNew ){
        memcpy(pNew, p, lookasideMallocSize(db, p));
        sqlite3DbFree(db, p);
      }
    }không_thì{
      assert( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
      assert( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
      sqlite3MemdebugSetType(p, MEMTYPE_HEAP);
      pNew = sqlite3Realloc(p, n);
      nếu( !pNew ){
        sqlite3OomFault(db);
      }
      sqlite3MemdebugSetType(pNew,
            (db->lookaside.bDisable==0 ? MEMTYPE_LOOKASIDE : MEMTYPE_HEAP));
    }
  }
  trả_về pNew;
}

/*
** Attempt to reallocate p.  If the reallocation fails, then free p
** and set the mallocFailed flag in the database connection.
*/
void *sqlite3DbReallocOrFree(sqlite3 *db, void *p, u64 n){
  void *pNew;
  pNew = sqlite3DbRealloc(db, p, n);
  nếu( !pNew ){
    sqlite3DbFree(db, p);
  }
  trả_về pNew;
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
  nếu( z==0 ){
    trả_về 0;
  }
  n = strlen(z) + 1;
  zNew = sqlite3DbMallocRaw(db, n);
  nếu( zNew ){
    memcpy(zNew, z, n);
  }
  trả_về zNew;
}
char *sqlite3DbStrNDup(sqlite3 *db, const char *z, u64 n){
  char *zNew;
  assert( db!=0 );
  assert( z!=0 || n==0 );
  assert( (n&0x7fffffff)==n );
  zNew = z ? sqlite3DbMallocRawNN(db, n+1) : 0;
  nếu( zNew ){
    memcpy(zNew, z, (size_t)n);
    zNew[n] = 0;
  }
  trả_về zNew;
}

/*
** The text between zStart and zEnd represents a phrase within a larger
** SQL statement.  Make a copy of this phrase in space obtained form
** sqlite3DbMalloc().  Omit leading and trailing whitespace.
*/
char *sqlite3DbSpanDup(sqlite3 *db, const char *zStart, const char *zEnd){
  số_nguyên n;
#ifdef SQLITE_DEBUG
  /* Because of the way the parser works, the span is guaranteed to contain
  ** at least one non-space character */
  cho(n=0; sqlite3Isspace(zStart[n]); n++){ assert( &zStart[n]<zEnd ); }
#endif
  trong_khi( sqlite3Isspace(zStart[0]) ) zStart++;
  n = (số_nguyên)(zEnd - zStart);
  trong_khi( sqlite3Isspace(zStart[n-1]) ) n--;
  trả_về sqlite3DbStrNDup(db, zStart, n);
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
  nếu( db->mallocFailed==0 && db->bBenignMalloc==0 ){
    db->mallocFailed = 1;
    nếu( db->nVdbeExec>0 ){
      AtomicStore(&db->u1.isInterrupted, 1);
    }
    DisableLookaside;
    nếu( db->pParse ){
      Parse *pParse;
      sqlite3ErrorMsg(db->pParse, "out of memory");
      db->pParse->rc = SQLITE_NOMEM_BKPT;
      cho(pParse=db->pParse->pOuterParse; pParse; pParse = pParse->pOuterParse){
        pParse->nErr++;
        pParse->rc = SQLITE_NOMEM;
      } 
    }
  }
  trả_về 0;
}

/*
** This routine reactivates the memory allocator and clears the
** db->mallocFailed flag as necessary.
**
** The memory allocator is not restarted if there are running
** VDBEs.
*/
void sqlite3OomClear(sqlite3 *db){
  nếu( db->mallocFailed && db->nVdbeExec==0 ){
    db->mallocFailed = 0;
    AtomicStore(&db->u1.isInterrupted, 0);
    assert( db->lookaside.bDisable>0 );
    EnableLookaside;
  }
}

/*
** Take actions at the end of an API call to deal with error codes.
*/
static SQLITE_NOINLINE số_nguyên apiHandleError(sqlite3 *db, số_nguyên rc){
  nếu( db->mallocFailed || rc==SQLITE_IOERR_NOMEM ){
    sqlite3OomClear(db);
    sqlite3Error(db, SQLITE_NOMEM);
    trả_về SQLITE_NOMEM_BKPT;
  }
  trả_về rc & db->errMask;
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
số_nguyên sqlite3ApiExit(sqlite3* db, số_nguyên rc){
  /* If the db handle must hold the connection handle mutex here.
  ** Otherwise the read (and possible write) of db->mallocFailed 
  ** is unsafe, as is the call to sqlite3Error().
  */
  assert( db!=0 );
  assert( sqlite3_mutex_held(db->mutex) );
  nếu( db->mallocFailed || rc ){
    trả_về apiHandleError(db, rc);
  }
  trả_về 0;
}
