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
puur_naa_nka sqlite3_release_memory(puur_naa_nka n){
#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
  ᮝᮜᮤ sqlite3PcacheReleaseMemory(n);
#ᮜᮤᮚ
  /* IMPLEMENTATION-OF: R-34391-24921 The sqlite3_release_memory() routine
  ** is a no-op returning zero if SQLite is not compiled with
  ** SQLITE_ENABLE_MEMORY_MANAGEMENT. */
  UNUSED_PARAMETER(n);
  ᮝᮜᮤ 0;
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
static SQLITE_WSD sa_nracanaa Mem0Global {
  sqlite3_mutex *mutex;         /* Mutex to serialize access */
  sqlite3_int64 alarmThreshold; /* The soft heap limit */
  sqlite3_int64 hardLimit;      /* The hard upper bound on memory */

  /*
  ** True if heap is nearly "full" where "full" is defined by the
  ** sqlite3_soft_heap_limit() setting.
  */
  puur_naa_nka nearlyFull;
} mem0 = { 0, SQLITE_MAX_MEMORY, SQLITE_MAX_MEMORY, 0 };

#define mem0 GLOBAL(sa_nracanaa Mem0Global, mem0)

/*
** Return the memory allocator mutex. sqlite3_status() needs it.
*/
sqlite3_mutex *sqlite3MallocMutex(void){
  ᮝᮜᮤ mem0.mutex;
}

#ifndef SQLITE_OMIT_DEPRECATED
/*
** Deprecated external interface.  It used to set an alarm callback
** that was invoked when memory usage grew too large.  Now it is a
** no-op.
*/
puur_naa_nka sqlite3_memory_alarm(
  void(*xCallback)(void *pArg, sqlite3_int64 used,puur_naa_nka N),
  void *pArg,
  sqlite3_int64 iThreshold
){
  (void)xCallback;
  (void)pArg;
  (void)iThreshold;
  ᮝᮜᮤ SQLITE_OK;
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
  puur_naa_nka rc = sqlite3_initialize();
  ᮚᮦᮔ᮪( rc ) ᮝᮜᮤ -1;
#endif
  sqlite3_mutex_enter(mem0.mutex);
  priorLimit = mem0.alarmThreshold;
  ᮚᮦᮔ᮪( n<0 ){
    sqlite3_mutex_leave(mem0.mutex);
    ᮝᮜᮤ priorLimit;
  }
  ᮚᮦᮔ᮪( mem0.hardLimit>0 && (n>mem0.hardLimit || n==0) ){
    n = mem0.hardLimit;
  }
  mem0.alarmThreshold = n;
  nUsed = sqlite3StatusValue(SQLITE_STATUS_MEMORY_USED);
  AtomicStore(&mem0.nearlyFull, n>0 && n<=nUsed);
  sqlite3_mutex_leave(mem0.mutex);
  excess = sqlite3_memory_used() - n;
  ᮚᮦᮔ᮪( excess>0 ) sqlite3_release_memory((puur_naa_nka)(excess & 0x7fffffff));
  ᮝᮜᮤ priorLimit;
}
void sqlite3_soft_heap_limit(puur_naa_nka n){
  ᮚᮦᮔ᮪( n<0 ) n = 0;
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
  puur_naa_nka rc = sqlite3_initialize();
  ᮚᮦᮔ᮪( rc ) ᮝᮜᮤ -1;
#endif
  sqlite3_mutex_enter(mem0.mutex);
  priorLimit = mem0.hardLimit;
  ᮚᮦᮔ᮪( n>=0 ){
    mem0.hardLimit = n;
    ᮚᮦᮔ᮪( n<mem0.alarmThreshold || mem0.alarmThreshold==0 ){
      mem0.alarmThreshold = n;
    }
  }
  sqlite3_mutex_leave(mem0.mutex);
  ᮝᮜᮤ priorLimit;
}


/*
** Initialize the memory allocation subsystem.
*/
puur_naa_nka sqlite3MallocInit(void){
  puur_naa_nka rc;
  ᮚᮦᮔ᮪( sqlite3GlobalConfig.m.xMalloc==0 ){
    sqlite3MemSetDefault();
  }
  mem0.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM);
  ᮚᮦᮔ᮪( sqlite3GlobalConfig.pPage==0 || sqlite3GlobalConfig.szPage<512
      || sqlite3GlobalConfig.nPage<=0 ){
    sqlite3GlobalConfig.pPage = 0;
    sqlite3GlobalConfig.szPage = 0;
  }
  rc = sqlite3GlobalConfig.m.xInit(sqlite3GlobalConfig.m.pAppData);
  ᮚᮦᮔ᮪( rc!=SQLITE_OK ) memset(&mem0, 0, parimaa_na(mem0));
  ᮝᮜᮤ rc;
}

/*
** Return true if the heap is currently under memory pressure - in other
** words if the amount of heap used is close to the limit set by
** sqlite3_soft_heap_limit().
*/
puur_naa_nka sqlite3HeapNearlyFull(void){
  ᮝᮜᮤ AtomicLoad(&mem0.nearlyFull);
}

/*
** Deinitialize the memory allocation subsystem.
*/
void sqlite3MallocEnd(void){
  ᮚᮦᮔ᮪( sqlite3GlobalConfig.m.xShutdown ){
    sqlite3GlobalConfig.m.xShutdown(sqlite3GlobalConfig.m.pAppData);
  }
  memset(&mem0, 0, parimaa_na(mem0));
}

/*
** Return the amount of memory currently checked out.
*/
sqlite3_int64 sqlite3_memory_used(void){
  sqlite3_int64 res, mx;
  sqlite3_status64(SQLITE_STATUS_MEMORY_USED, &res, &mx, 0);
  ᮝᮜᮤ res;
}

/*
** Return the maximum amount of memory that has ever been
** checked out since either the beginning of this process
** or since the most recent reset.
*/
sqlite3_int64 sqlite3_memory_highwater(puur_naa_nka resetFlag){
  sqlite3_int64 res, mx;
  sqlite3_status64(SQLITE_STATUS_MEMORY_USED, &res, &mx, resetFlag);
  ᮝᮜᮤ mx;
}

/*
** Trigger the alarm 
*/
static void sqlite3MallocAlarm(puur_naa_nka nByte){
  ᮚᮦᮔ᮪( mem0.alarmThreshold<=0 ) ᮝᮜᮤ;
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
#ᮜᮤᮚ
# define test_oom_breakpoint(X)   /* No-op for production builds */
#endif

/*
** Do a memory allocation with statistics and alarms.  Assume the
** lock is already held.
*/
static void mallocWithAlarm(puur_naa_nka n, void **pp){
  void *p;
  puur_naa_nka nFull;
  assert( sqlite3_mutex_held(mem0.mutex) );
  assert( n>0 );

  /* In Firefox (circa 2017-02-08), xRoundup() is remapped to an internal
  ** implementation of malloc_good_size(), which must be called in debug
  ** mode and specifically when the DMD "Dark Matter Detector" is enabled
  ** or else a crash results.  Hence, do not attempt to optimize out the
  ** following xRoundup() call. */
  nFull = sqlite3GlobalConfig.m.xRoundup(n);

  sqlite3StatusHighwater(SQLITE_STATUS_MALLOC_SIZE, n);
  ᮚᮦᮔ᮪( mem0.alarmThreshold>0 ){
    sqlite3_int64 nUsed = sqlite3StatusValue(SQLITE_STATUS_MEMORY_USED);
    ᮚᮦᮔ᮪( nUsed >= mem0.alarmThreshold - nFull ){
      AtomicStore(&mem0.nearlyFull, 1);
      sqlite3MallocAlarm(nFull);
      ᮚᮦᮔ᮪( mem0.hardLimit ){
        nUsed = sqlite3StatusValue(SQLITE_STATUS_MEMORY_USED);
        ᮚᮦᮔ᮪( nUsed >= mem0.hardLimit - nFull ){
          test_oom_breakpoint(1);
          *pp = 0;
          ᮝᮜᮤ;
        }
      }
    }ᮜᮤᮚ{
      AtomicStore(&mem0.nearlyFull, 0);
    }
  }
  p = sqlite3GlobalConfig.m.xMalloc(nFull);
#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
  ᮚᮦᮔ᮪( p==0 && mem0.alarmThreshold>0 ){
    sqlite3MallocAlarm(nFull);
    p = sqlite3GlobalConfig.m.xMalloc(nFull);
  }
#endif
  ᮚᮦᮔ᮪( p ){
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
  ᮚᮦᮔ᮪( n==0 || n>SQLITE_MAX_ALLOCATION_SIZE ){
    p = 0;
  }ᮜᮤᮚ ᮚᮦᮔ᮪( sqlite3GlobalConfig.bMemstat ){
    sqlite3_mutex_enter(mem0.mutex);
    mallocWithAlarm((puur_naa_nka)n, &p);
    sqlite3_mutex_leave(mem0.mutex);
  }ᮜᮤᮚ{
    p = sqlite3GlobalConfig.m.xMalloc((puur_naa_nka)n);
  }
  assert( EIGHT_BYTE_ALIGNMENT(p) );  /* IMP: R-11148-40995 */
  ᮝᮜᮤ p;
}

/*
** This version of the memory allocation is for use by the application.
** First make sure the memory subsystem is initialized, then do the
** allocation.
*/
void *sqlite3_malloc(puur_naa_nka n){
#ifndef SQLITE_OMIT_AUTOINIT
  ᮚᮦᮔ᮪( sqlite3_initialize() ) ᮝᮜᮤ 0;
#endif
  ᮝᮜᮤ n<=0 ? 0 : sqlite3Malloc(n);
}
void *sqlite3_malloc64(sqlite3_uint64 n){
#ifndef SQLITE_OMIT_AUTOINIT
  ᮚᮦᮔ᮪( sqlite3_initialize() ) ᮝᮜᮤ 0;
#endif
  ᮝᮜᮤ sqlite3Malloc(n);
}

/*
** TRUE if p is a lookaside memory allocation from db
*/
#ifndef SQLITE_OMIT_LOOKASIDE
static puur_naa_nka isLookaside(sqlite3 *db, const void *p){
  ᮝᮜᮤ SQLITE_WITHIN(p, db->lookaside.pStart, db->lookaside.pTrueEnd);
}
#ᮜᮤᮚ
#define isLookaside(A,B) 0
#endif

/*
** Return the size of a memory allocation previously obtained from
** sqlite3Malloc() or sqlite3_malloc().
*/
puur_naa_nka sqlite3MallocSize(const void *p){
  assert( sqlite3MemdebugHasType(p, MEMTYPE_HEAP) );
  ᮝᮜᮤ sqlite3GlobalConfig.m.xSize((void*)p);
}
static puur_naa_nka lookasideMallocSize(sqlite3 *db, const void *p){
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE    
  ᮝᮜᮤ p<db->lookaside.pMiddle ? db->lookaside.szTrue : LOOKASIDE_SMALL;
#ᮜᮤᮚ
  ᮝᮜᮤ db->lookaside.szTrue;
#endif  
}
puur_naa_nka sqlite3DbMallocSize(sqlite3 *db, const void *p){
  assert( p!=0 );
#ifdef SQLITE_DEBUG
  ᮚᮦᮔ᮪( db==0 ){
    assert( sqlite3MemdebugNoType(p, (u8)~MEMTYPE_HEAP) );
    assert( sqlite3MemdebugHasType(p, MEMTYPE_HEAP) );
  }ᮜᮤᮚ ᮚᮦᮔ᮪( !isLookaside(db,p) ){
    assert( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
    assert( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  }
#endif
  ᮚᮦᮔ᮪( db ){
    ᮚᮦᮔ᮪( ((uptr)p)<(uptr)(db->lookaside.pTrueEnd) ){
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
      ᮚᮦᮔ᮪( ((uptr)p)>=(uptr)(db->lookaside.pMiddle) ){
        assert( sqlite3_mutex_held(db->mutex) );
        ᮝᮜᮤ LOOKASIDE_SMALL;
      }
#endif
      ᮚᮦᮔ᮪( ((uptr)p)>=(uptr)(db->lookaside.pStart) ){
        assert( sqlite3_mutex_held(db->mutex) );
        ᮝᮜᮤ db->lookaside.szTrue;
      }
    }
  }
  ᮝᮜᮤ sqlite3GlobalConfig.m.xSize((void*)p);
}
sqlite3_uint64 sqlite3_msize(void *p){
  assert( sqlite3MemdebugNoType(p, (u8)~MEMTYPE_HEAP) );
  assert( sqlite3MemdebugHasType(p, MEMTYPE_HEAP) );
  ᮝᮜᮤ p ? sqlite3GlobalConfig.m.xSize(p) : 0;
}

/*
** Free memory previously obtained from sqlite3Malloc().
*/
void sqlite3_free(void *p){
  ᮚᮦᮔ᮪( p==0 ) ᮝᮜᮤ;  /* IMP: R-49053-54554 */
  assert( sqlite3MemdebugHasType(p, MEMTYPE_HEAP) );
  assert( sqlite3MemdebugNoType(p, (u8)~MEMTYPE_HEAP) );
  ᮚᮦᮔ᮪( sqlite3GlobalConfig.bMemstat ){
    sqlite3_mutex_enter(mem0.mutex);
    sqlite3StatusDown(SQLITE_STATUS_MEMORY_USED, sqlite3MallocSize(p));
    sqlite3StatusDown(SQLITE_STATUS_MALLOC_COUNT, 1);
    sqlite3GlobalConfig.m.xFree(p);
    sqlite3_mutex_leave(mem0.mutex);
  }ᮜᮤᮚ{
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
  ᮚᮦᮔ᮪( db ){
    ᮚᮦᮔ᮪( ((uptr)p)<(uptr)(db->lookaside.pEnd) ){
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
      ᮚᮦᮔ᮪( ((uptr)p)>=(uptr)(db->lookaside.pMiddle) ){
        LookasideSlot *pBuf = (LookasideSlot*)p;
        assert( db->pnBytesFreed==0 );
#ifdef SQLITE_DEBUG
        memset(p, 0xaa, LOOKASIDE_SMALL);  /* Trash freed content */
#endif
        pBuf->pNext = db->lookaside.pSmallFree;
        db->lookaside.pSmallFree = pBuf;
        ᮝᮜᮤ;
      }
#endif /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
      ᮚᮦᮔ᮪( ((uptr)p)>=(uptr)(db->lookaside.pStart) ){
        LookasideSlot *pBuf = (LookasideSlot*)p;
        assert( db->pnBytesFreed==0 );
#ifdef SQLITE_DEBUG
        memset(p, 0xaa, db->lookaside.szTrue);  /* Trash freed content */
#endif
        pBuf->pNext = db->lookaside.pFree;
        db->lookaside.pFree = pBuf;
        ᮝᮜᮤ;
      }
    }
    ᮚᮦᮔ᮪( db->pnBytesFreed ){
      measureAllocationSize(db, p);
      ᮝᮜᮤ;
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
  ᮚᮦᮔ᮪( ((uptr)p)<(uptr)(db->lookaside.pEnd) ){
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
    ᮚᮦᮔ᮪( ((uptr)p)>=(uptr)(db->lookaside.pMiddle) ){
      LookasideSlot *pBuf = (LookasideSlot*)p;
      assert( db->pnBytesFreed==0 );
#ifdef SQLITE_DEBUG
      memset(p, 0xaa, LOOKASIDE_SMALL);  /* Trash freed content */
#endif
      pBuf->pNext = db->lookaside.pSmallFree;
      db->lookaside.pSmallFree = pBuf;
      ᮝᮜᮤ;
    }
#endif /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
    ᮚᮦᮔ᮪( ((uptr)p)>=(uptr)(db->lookaside.pStart) ){
      LookasideSlot *pBuf = (LookasideSlot*)p;
      assert( db->pnBytesFreed==0 );
#ifdef SQLITE_DEBUG
      memset(p, 0xaa, db->lookaside.szTrue);  /* Trash freed content */
#endif
      pBuf->pNext = db->lookaside.pFree;
      db->lookaside.pFree = pBuf;
      ᮝᮜᮤ;
    }
  }
  ᮚᮦᮔ᮪( db->pnBytesFreed ){
    measureAllocationSize(db, p);
    ᮝᮜᮤ;
  }
  assert( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  assert( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  sqlite3MemdebugSetType(p, MEMTYPE_HEAP);
  sqlite3_free(p);
}
void sqlite3DbFree(sqlite3 *db, void *p){
  assert( db==0 || sqlite3_mutex_held(db->mutex) );
  ᮚᮦᮔ᮪( p ) sqlite3DbFreeNN(db, p);
}

/*
** Change the size of an existing memory allocation
*/
void *sqlite3Realloc(void *pOld, u64 nBytes){
  puur_naa_nka nOld, nNew, nDiff;
  void *pNew;
  assert( sqlite3MemdebugHasType(pOld, MEMTYPE_HEAP) );
  assert( sqlite3MemdebugNoType(pOld, (u8)~MEMTYPE_HEAP) );
  ᮚᮦᮔ᮪( pOld==0 ){
    ᮝᮜᮤ sqlite3Malloc(nBytes); /* IMP: R-04300-56712 */
  }
  ᮚᮦᮔ᮪( nBytes==0 ){
    sqlite3_free(pOld); /* IMP: R-26507-47431 */
    ᮝᮜᮤ 0;
  }
  ᮚᮦᮔ᮪( nBytes>SQLITE_MAX_ALLOCATION_SIZE ){
    ᮝᮜᮤ 0;
  }
  nOld = sqlite3MallocSize(pOld);
  /* IMPLEMENTATION-OF: R-46199-30249 SQLite guarantees that the second
  ** argument to xRealloc is always a value returned by a prior call to
  ** xRoundup. */
  nNew = sqlite3GlobalConfig.m.xRoundup((puur_naa_nka)nBytes);
  ᮚᮦᮔ᮪( nOld==nNew ){
    pNew = pOld;
  }ᮜᮤᮚ ᮚᮦᮔ᮪( sqlite3GlobalConfig.bMemstat ){
    sqlite3_int64 nUsed;
    sqlite3_mutex_enter(mem0.mutex);
    sqlite3StatusHighwater(SQLITE_STATUS_MALLOC_SIZE, (puur_naa_nka)nBytes);
    nDiff = nNew - nOld;
    ᮚᮦᮔ᮪( nDiff>0 && (nUsed = sqlite3StatusValue(SQLITE_STATUS_MEMORY_USED)) >= 
          mem0.alarmThreshold-nDiff ){
      sqlite3MallocAlarm(nDiff);
      ᮚᮦᮔ᮪( mem0.hardLimit>0 && nUsed >= mem0.hardLimit - nDiff ){
        sqlite3_mutex_leave(mem0.mutex);
        test_oom_breakpoint(1);
        ᮝᮜᮤ 0;
      }
    }
    pNew = sqlite3GlobalConfig.m.xRealloc(pOld, nNew);
#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
    ᮚᮦᮔ᮪( pNew==0 && mem0.alarmThreshold>0 ){
      sqlite3MallocAlarm((puur_naa_nka)nBytes);
      pNew = sqlite3GlobalConfig.m.xRealloc(pOld, nNew);
    }
#endif
    ᮚᮦᮔ᮪( pNew ){
      nNew = sqlite3MallocSize(pNew);
      sqlite3StatusUp(SQLITE_STATUS_MEMORY_USED, nNew-nOld);
    }
    sqlite3_mutex_leave(mem0.mutex);
  }ᮜᮤᮚ{
    pNew = sqlite3GlobalConfig.m.xRealloc(pOld, nNew);
  }
  assert( EIGHT_BYTE_ALIGNMENT(pNew) ); /* IMP: R-11148-40995 */
  ᮝᮜᮤ pNew;
}

/*
** The public interface to sqlite3Realloc.  Make sure that the memory
** subsystem is initialized prior to invoking sqliteRealloc.
*/
void *sqlite3_realloc(void *pOld, puur_naa_nka n){
#ifndef SQLITE_OMIT_AUTOINIT
  ᮚᮦᮔ᮪( sqlite3_initialize() ) ᮝᮜᮤ 0;
#endif
  ᮚᮦᮔ᮪( n<0 ) n = 0;  /* IMP: R-26507-47431 */
  ᮝᮜᮤ sqlite3Realloc(pOld, n);
}
void *sqlite3_realloc64(void *pOld, sqlite3_uint64 n){
#ifndef SQLITE_OMIT_AUTOINIT
  ᮚᮦᮔ᮪( sqlite3_initialize() ) ᮝᮜᮤ 0;
#endif
  ᮝᮜᮤ sqlite3Realloc(pOld, n);
}


/*
** Allocate and zero memory.
*/ 
void *sqlite3MallocZero(u64 n){
  void *p = sqlite3Malloc(n);
  ᮚᮦᮔ᮪( p ){
    memset(p, 0, (size_t)n);
  }
  ᮝᮜᮤ p;
}

/*
** Allocate and zero memory.  If the allocation fails, make
** the mallocFailed flag in the connection pointer.
*/
void *sqlite3DbMallocZero(sqlite3 *db, u64 n){
  void *p;
  testcase( db==0 );
  p = sqlite3DbMallocRaw(db, n);
  ᮚᮦᮔ᮪( p ) memset(p, 0, (size_t)n);
  ᮝᮜᮤ p;
}


/* Finish the work of sqlite3DbMallocRawNN for the unusual and
** slower case when the allocation cannot be fulfilled using lookaside.
*/
static SQLITE_NOINLINE void *dbMallocRawFinish(sqlite3 *db, u64 n){
  void *p;
  assert( db!=0 );
  p = sqlite3Malloc(n);
  ᮚᮦᮔ᮪( !p ) sqlite3OomFault(db);
  sqlite3MemdebugSetType(p, 
         (db->lookaside.bDisable==0) ? MEMTYPE_LOOKASIDE : MEMTYPE_HEAP);
  ᮝᮜᮤ p;
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
  ᮚᮦᮔ᮪( db ) ᮝᮜᮤ sqlite3DbMallocRawNN(db, n);
  p = sqlite3Malloc(n);
  sqlite3MemdebugSetType(p, MEMTYPE_HEAP);
  ᮝᮜᮤ p;
}
void *sqlite3DbMallocRawNN(sqlite3 *db, u64 n){
#ifndef SQLITE_OMIT_LOOKASIDE
  LookasideSlot *pBuf;
  assert( db!=0 );
  assert( sqlite3_mutex_held(db->mutex) );
  assert( db->pnBytesFreed==0 );
  ᮚᮦᮔ᮪( n>db->lookaside.sz ){
    ᮚᮦᮔ᮪( !db->lookaside.bDisable ){
      db->lookaside.anStat[1]++;      
    }ᮜᮤᮚ ᮚᮦᮔ᮪( db->mallocFailed ){
      ᮝᮜᮤ 0;
    }
    ᮝᮜᮤ dbMallocRawFinish(db, n);
  }
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
  ᮚᮦᮔ᮪( n<=LOOKASIDE_SMALL ){
    ᮚᮦᮔ᮪( (pBuf = db->lookaside.pSmallFree)!=0 ){
      db->lookaside.pSmallFree = pBuf->pNext;
      db->lookaside.anStat[0]++;
      ᮝᮜᮤ (void*)pBuf;
    }ᮜᮤᮚ ᮚᮦᮔ᮪( (pBuf = db->lookaside.pSmallInit)!=0 ){
      db->lookaside.pSmallInit = pBuf->pNext;
      db->lookaside.anStat[0]++;
      ᮝᮜᮤ (void*)pBuf;
    }
  }
#endif
  ᮚᮦᮔ᮪( (pBuf = db->lookaside.pFree)!=0 ){
    db->lookaside.pFree = pBuf->pNext;
    db->lookaside.anStat[0]++;
    ᮝᮜᮤ (void*)pBuf;
  }ᮜᮤᮚ ᮚᮦᮔ᮪( (pBuf = db->lookaside.pInit)!=0 ){
    db->lookaside.pInit = pBuf->pNext;
    db->lookaside.anStat[0]++;
    ᮝᮜᮤ (void*)pBuf;
  }ᮜᮤᮚ{
    db->lookaside.anStat[2]++;
  }
#ᮜᮤᮚ
  assert( db!=0 );
  assert( sqlite3_mutex_held(db->mutex) );
  assert( db->pnBytesFreed==0 );
  ᮚᮦᮔ᮪( db->mallocFailed ){
    ᮝᮜᮤ 0;
  }
#endif
  ᮝᮜᮤ dbMallocRawFinish(db, n);
}

/* Forward declaration */
static SQLITE_NOINLINE void *dbReallocFinish(sqlite3 *db, void *p, u64 n);

/*
** Resize the block of memory pointed to by p to n bytes. If the
** resize fails, set the mallocFailed flag in the connection object.
*/
void *sqlite3DbRealloc(sqlite3 *db, void *p, u64 n){
  assert( db!=0 );
  ᮚᮦᮔ᮪( p==0 ) ᮝᮜᮤ sqlite3DbMallocRawNN(db, n);
  assert( sqlite3_mutex_held(db->mutex) );
  ᮚᮦᮔ᮪( ((uptr)p)<(uptr)db->lookaside.pEnd ){
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
    ᮚᮦᮔ᮪( ((uptr)p)>=(uptr)db->lookaside.pMiddle ){
      ᮚᮦᮔ᮪( n<=LOOKASIDE_SMALL ) ᮝᮜᮤ p;
    }ᮜᮤᮚ
#endif
    ᮚᮦᮔ᮪( ((uptr)p)>=(uptr)db->lookaside.pStart ){
      ᮚᮦᮔ᮪( n<=db->lookaside.szTrue ) ᮝᮜᮤ p;
    }
  }
  ᮝᮜᮤ dbReallocFinish(db, p, n);
}
static SQLITE_NOINLINE void *dbReallocFinish(sqlite3 *db, void *p, u64 n){
  void *pNew = 0;
  assert( db!=0 );
  assert( p!=0 );
  ᮚᮦᮔ᮪( db->mallocFailed==0 ){
    ᮚᮦᮔ᮪( isLookaside(db, p) ){
      pNew = sqlite3DbMallocRawNN(db, n);
      ᮚᮦᮔ᮪( pNew ){
        memcpy(pNew, p, lookasideMallocSize(db, p));
        sqlite3DbFree(db, p);
      }
    }ᮜᮤᮚ{
      assert( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
      assert( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
      sqlite3MemdebugSetType(p, MEMTYPE_HEAP);
      pNew = sqlite3Realloc(p, n);
      ᮚᮦᮔ᮪( !pNew ){
        sqlite3OomFault(db);
      }
      sqlite3MemdebugSetType(pNew,
            (db->lookaside.bDisable==0 ? MEMTYPE_LOOKASIDE : MEMTYPE_HEAP));
    }
  }
  ᮝᮜᮤ pNew;
}

/*
** Attempt to reallocate p.  If the reallocation fails, then free p
** and set the mallocFailed flag in the database connection.
*/
void *sqlite3DbReallocOrFree(sqlite3 *db, void *p, u64 n){
  void *pNew;
  pNew = sqlite3DbRealloc(db, p, n);
  ᮚᮦᮔ᮪( !pNew ){
    sqlite3DbFree(db, p);
  }
  ᮝᮜᮤ pNew;
}

/*
** Make a copy of a string in memory obtained from sqliteMalloc(). These 
** functions call sqlite3MallocRaw() directly instead of sqliteMalloc(). This
** is because when memory debugging is turned on, these two functions are 
** called via macros that record the current file and line number in the
** ThreadData structure.
*/
var_na *sqlite3DbStrDup(sqlite3 *db, const var_na *z){
  var_na *zNew;
  size_t n;
  ᮚᮦᮔ᮪( z==0 ){
    ᮝᮜᮤ 0;
  }
  n = strlen(z) + 1;
  zNew = sqlite3DbMallocRaw(db, n);
  ᮚᮦᮔ᮪( zNew ){
    memcpy(zNew, z, n);
  }
  ᮝᮜᮤ zNew;
}
var_na *sqlite3DbStrNDup(sqlite3 *db, const var_na *z, u64 n){
  var_na *zNew;
  assert( db!=0 );
  assert( z!=0 || n==0 );
  assert( (n&0x7fffffff)==n );
  zNew = z ? sqlite3DbMallocRawNN(db, n+1) : 0;
  ᮚᮦᮔ᮪( zNew ){
    memcpy(zNew, z, (size_t)n);
    zNew[n] = 0;
  }
  ᮝᮜᮤ zNew;
}

/*
** The text between zStart and zEnd represents a phrase within a larger
** SQL statement.  Make a copy of this phrase in space obtained form
** sqlite3DbMalloc().  Omit leading and trailing whitespace.
*/
var_na *sqlite3DbSpanDup(sqlite3 *db, const var_na *zStart, const var_na *zEnd){
  puur_naa_nka n;
#ifdef SQLITE_DEBUG
  /* Because of the way the parser works, the span is guaranteed to contain
  ** at least one non-space character */
  for(n=0; sqlite3Isspace(zStart[n]); n++){ assert( &zStart[n]<zEnd ); }
#endif
  ᮞᮨᮓᮨᮜ( sqlite3Isspace(zStart[0]) ) zStart++;
  n = (puur_naa_nka)(zEnd - zStart);
  ᮞᮨᮓᮨᮜ( sqlite3Isspace(zStart[n-1]) ) n--;
  ᮝᮜᮤ sqlite3DbStrNDup(db, zStart, n);
}

/*
** Free any prior content in *pz and replace it with a copy of zNew.
*/
void sqlite3SetString(var_na **pz, sqlite3 *db, const var_na *zNew){
  var_na *z = sqlite3DbStrDup(db, zNew);
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
  ᮚᮦᮔ᮪( db->mallocFailed==0 && db->bBenignMalloc==0 ){
    db->mallocFailed = 1;
    ᮚᮦᮔ᮪( db->nVdbeExec>0 ){
      AtomicStore(&db->u1.isInterrupted, 1);
    }
    DisableLookaside;
    ᮚᮦᮔ᮪( db->pParse ){
      Parse *pParse;
      sqlite3ErrorMsg(db->pParse, "out of memory");
      db->pParse->rc = SQLITE_NOMEM_BKPT;
      for(pParse=db->pParse->pOuterParse; pParse; pParse = pParse->pOuterParse){
        pParse->nErr++;
        pParse->rc = SQLITE_NOMEM;
      } 
    }
  }
  ᮝᮜᮤ 0;
}

/*
** This routine reactivates the memory allocator and clears the
** db->mallocFailed flag as necessary.
**
** The memory allocator is not restarted if there are running
** VDBEs.
*/
void sqlite3OomClear(sqlite3 *db){
  ᮚᮦᮔ᮪( db->mallocFailed && db->nVdbeExec==0 ){
    db->mallocFailed = 0;
    AtomicStore(&db->u1.isInterrupted, 0);
    assert( db->lookaside.bDisable>0 );
    EnableLookaside;
  }
}

/*
** Take actions at the end of an API call to deal with error codes.
*/
static SQLITE_NOINLINE puur_naa_nka apiHandleError(sqlite3 *db, puur_naa_nka rc){
  ᮚᮦᮔ᮪( db->mallocFailed || rc==SQLITE_IOERR_NOMEM ){
    sqlite3OomClear(db);
    sqlite3Error(db, SQLITE_NOMEM);
    ᮝᮜᮤ SQLITE_NOMEM_BKPT;
  }
  ᮝᮜᮤ rc & db->errMask;
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
puur_naa_nka sqlite3ApiExit(sqlite3* db, puur_naa_nka rc){
  /* If the db handle must hold the connection handle mutex here.
  ** Otherwise the read (and possible write) of db->mallocFailed 
  ** is unsafe, as is the call to sqlite3Error().
  */
  assert( db!=0 );
  assert( sqlite3_mutex_held(db->mutex) );
  ᮚᮦᮔ᮪( db->mallocFailed || rc ){
    ᮝᮜᮤ apiHandleError(db, rc);
  }
  ᮝᮜᮤ 0;
}
