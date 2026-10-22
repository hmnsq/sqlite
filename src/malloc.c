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
#अन्तर्भाव "sqliteInt.h"
#अन्तर्भाव <stdarg.h>

/*
** Attempt to release up to n bytes of non-essential memory currently
** held by SQLite. An example of non-essential memory is memory used to
** cache database pages that are not currently in use.
*/
पूर्णांक sqlite3_release_memory(पूर्णांक n){
#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
  परत sqlite3PcacheReleaseMemory(n);
#अन्यथा
  /* IMPLEMENTATION-OF: R-34391-24921 The sqlite3_release_memory() routine
  ** is a no-op returning zero if SQLite is not compiled with
  ** SQLITE_ENABLE_MEMORY_MANAGEMENT. */
  UNUSED_PARAMETER(n);
  परत 0;
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
स्थिर SQLITE_WSD संरचना Mem0Global {
  sqlite3_mutex *mutex;         /* Mutex to serialize access */
  sqlite3_int64 alarmThreshold; /* The soft heap limit */
  sqlite3_int64 hardLimit;      /* The hard upper bound on memory */

  /*
  ** True if heap is nearly "full" where "full" is defined by the
  ** sqlite3_soft_heap_limit() setting.
  */
  पूर्णांक nearlyFull;
} mem0 = { 0, SQLITE_MAX_MEMORY, SQLITE_MAX_MEMORY, 0 };

#define mem0 GLOBAL(संरचना Mem0Global, mem0)

/*
** Return the memory allocator mutex. sqlite3_status() needs it.
*/
sqlite3_mutex *sqlite3MallocMutex(शून्य){
  परत mem0.mutex;
}

#ifndef SQLITE_OMIT_DEPRECATED
/*
** Deprecated external interface.  It used to set an alarm callback
** that was invoked when memory usage grew too large.  Now it is a
** no-op.
*/
पूर्णांक sqlite3_memory_alarm(
  शून्य(*xCallback)(शून्य *pArg, sqlite3_int64 used,पूर्णांक N),
  शून्य *pArg,
  sqlite3_int64 iThreshold
){
  (शून्य)xCallback;
  (शून्य)pArg;
  (शून्य)iThreshold;
  परत SQLITE_OK;
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
  पूर्णांक rc = sqlite3_initialize();
  जर( rc ) परत -1;
#endif
  sqlite3_mutex_enter(mem0.mutex);
  priorLimit = mem0.alarmThreshold;
  जर( n<0 ){
    sqlite3_mutex_leave(mem0.mutex);
    परत priorLimit;
  }
  जर( mem0.hardLimit>0 && (n>mem0.hardLimit || n==0) ){
    n = mem0.hardLimit;
  }
  mem0.alarmThreshold = n;
  nUsed = sqlite3_Status_मूल्य(SQLITE_STATUS_MEMORY_USED);
  AtomicStore(&mem0.nearlyFull, n>0 && n<=nUsed);
  sqlite3_mutex_leave(mem0.mutex);
  excess = sqlite3_memory_used() - n;
  जर( excess>0 ) sqlite3_release_memory((पूर्णांक)(excess & 0x7fffffff));
  परत priorLimit;
}
शून्य sqlite3_soft_heap_limit(पूर्णांक n){
  जर( n<0 ) n = 0;
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
  पूर्णांक rc = sqlite3_initialize();
  जर( rc ) परत -1;
#endif
  sqlite3_mutex_enter(mem0.mutex);
  priorLimit = mem0.hardLimit;
  जर( n>=0 ){
    mem0.hardLimit = n;
    जर( n<mem0.alarmThreshold || mem0.alarmThreshold==0 ){
      mem0.alarmThreshold = n;
    }
  }
  sqlite3_mutex_leave(mem0.mutex);
  परत priorLimit;
}


/*
** Initialize the memory allocation subsystem.
*/
पूर्णांक sqlite3MallocInit(शून्य){
  पूर्णांक rc;
  जर( sqlite3GlobalConfig.m.xMalloc==0 ){
    sqlite3MemSetDefault();
  }
  mem0.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM);
  जर( sqlite3GlobalConfig.pPage==0 || sqlite3GlobalConfig.szPage<512
      || sqlite3GlobalConfig.nPage<=0 ){
    sqlite3GlobalConfig.pPage = 0;
    sqlite3GlobalConfig.szPage = 0;
  }
  rc = sqlite3GlobalConfig.m.xInit(sqlite3GlobalConfig.m.pAppData);
  जर( rc!=SQLITE_OK ) स_रखो(&mem0, 0, आकार(mem0));
  परत rc;
}

/*
** Return true if the heap is currently under memory pressure - in other
** words if the amount of heap used is close to the limit set by
** sqlite3_soft_heap_limit().
*/
पूर्णांक sqlite3HeapNearlyFull(शून्य){
  परत AtomicLoad(&mem0.nearlyFull);
}

/*
** Deinitialize the memory allocation subsystem.
*/
शून्य sqlite3MallocEnd(शून्य){
  जर( sqlite3GlobalConfig.m.xShutdown ){
    sqlite3GlobalConfig.m.xShutdown(sqlite3GlobalConfig.m.pAppData);
  }
  स_रखो(&mem0, 0, आकार(mem0));
}

/*
** Return the amount of memory currently checked out.
*/
sqlite3_int64 sqlite3_memory_used(शून्य){
  sqlite3_int64 res, mx;
  sqlite3_status64(SQLITE_STATUS_MEMORY_USED, &res, &mx, 0);
  परत res;
}

/*
** Return the maximum amount of memory that has ever been
** checked out since either the beginning of this process
** or since the most recent reset.
*/
sqlite3_int64 sqlite3_memory_highwater(पूर्णांक resetFlag){
  sqlite3_int64 res, mx;
  sqlite3_status64(SQLITE_STATUS_MEMORY_USED, &res, &mx, resetFlag);
  परत mx;
}

/*
** Trigger the alarm 
*/
स्थिर शून्य sqlite3MallocAlarm(पूर्णांक nByte){
  जर( mem0.alarmThreshold<=0 ) परत;
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
स्थिर SQLITE_NOINLINE शून्य test_oom_breakpoint(u64 n){
  स्थिर u64 nOomFault = 0;
  nOomFault += n;
  /* The assert() is never reached in a human lifetime.  It  is here mostly
  ** to prevent code optimizers from optimizing out this function. */
  प्रतिज्ञा( (nOomFault>>32) < 0xffffffff );
}
#अन्यथा
# define test_oom_breakpoint(X)   /* No-op for production builds */
#endif

/*
** Do a memory allocation with statistics and alarms.  Assume the
** lock is already held.
*/
स्थिर शून्य mallocWithAlarm(पूर्णांक n, शून्य **pp){
  शून्य *p;
  पूर्णांक nFull;
  प्रतिज्ञा( sqlite3_mutex_held(mem0.mutex) );
  प्रतिज्ञा( n>0 );

  /* In Firefox (circa 2017-02-08), xRoundup() is remapped to an internal
  ** implementation of malloc_good_size(), which must be called in debug
  ** mode and specifically when the DMD "Dark Matter Detector" is enabled
  ** or else a crash results.  Hence, do not attempt to optimize out the
  ** following xRoundup() call. */
  nFull = sqlite3GlobalConfig.m.xRoundup(n);

  sqlite3StatusHighwater(SQLITE_STATUS_MALLOC_SIZE, n);
  जर( mem0.alarmThreshold>0 ){
    sqlite3_int64 nUsed = sqlite3_Status_मूल्य(SQLITE_STATUS_MEMORY_USED);
    जर( nUsed >= mem0.alarmThreshold - nFull ){
      AtomicStore(&mem0.nearlyFull, 1);
      sqlite3MallocAlarm(nFull);
      जर( mem0.hardLimit ){
        nUsed = sqlite3_Status_मूल्य(SQLITE_STATUS_MEMORY_USED);
        जर( nUsed >= mem0.hardLimit - nFull ){
          test_oom_breakpoint(1);
          *pp = 0;
          परत;
        }
      }
    }अन्यथा{
      AtomicStore(&mem0.nearlyFull, 0);
    }
  }
  p = sqlite3GlobalConfig.m.xMalloc(nFull);
#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
  जर( p==0 && mem0.alarmThreshold>0 ){
    sqlite3MallocAlarm(nFull);
    p = sqlite3GlobalConfig.m.xMalloc(nFull);
  }
#endif
  जर( p ){
    nFull = sqlite3MallocSize(p);
    sqlite3StatusUp(SQLITE_STATUS_MEMORY_USED, nFull);
    sqlite3StatusUp(SQLITE_STATUS_MALLOC_मोजणी, 1);
  }
  *pp = p;
}

/*
** Allocate memory.  This routine is like sqlite3_malloc() except that it
** assumes the memory subsystem has already been initialized.
*/
शून्य *sqlite3Malloc(u64 n){
  शून्य *p;
  जर( n==0 || n>SQLITE_MAX_ALLOCATION_SIZE ){
    p = 0;
  }अन्यथा जर( sqlite3GlobalConfig.bMemstat ){
    sqlite3_mutex_enter(mem0.mutex);
    mallocWithAlarm((पूर्णांक)n, &p);
    sqlite3_mutex_leave(mem0.mutex);
  }अन्यथा{
    p = sqlite3GlobalConfig.m.xMalloc((पूर्णांक)n);
  }
  प्रतिज्ञा( EIGHT_BYTE_ALIGNMENT(p) );  /* IMP: R-11148-40995 */
  परत p;
}

/*
** This version of the memory allocation is for use by the application.
** First make sure the memory subsystem is initialized, then do the
** allocation.
*/
शून्य *sqlite3_malloc(पूर्णांक n){
#ifndef SQLITE_OMIT_AUTOINIT
  जर( sqlite3_initialize() ) परत 0;
#endif
  परत n<=0 ? 0 : sqlite3Malloc(n);
}
शून्य *sqlite3_malloc64(sqlite3_uint64 n){
#ifndef SQLITE_OMIT_AUTOINIT
  जर( sqlite3_initialize() ) परत 0;
#endif
  परत sqlite3Malloc(n);
}

/*
** TRUE if p is a lookaside memory allocation from db
*/
#ifndef SQLITE_OMIT_LOOKASIDE
स्थिर पूर्णांक isLookaside(sqlite3 *db, स्थिरांक शून्य *p){
  परत SQLITE_WITHIN(p, db->lookaside.pStart, db->lookaside.pTrueEnd);
}
#अन्यथा
#define isLookaside(A,B) 0
#endif

/*
** Return the size of a memory allocation previously obtained from
** sqlite3Malloc() or sqlite3_malloc().
*/
पूर्णांक sqlite3MallocSize(स्थिरांक शून्य *p){
  प्रतिज्ञा( sqlite3MemdebugHasType(p, MEMTYPE_HEAP) );
  परत sqlite3GlobalConfig.m.xSize((शून्य*)p);
}
स्थिर पूर्णांक lookasideMallocSize(sqlite3 *db, स्थिरांक शून्य *p){
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE    
  परत p<db->lookaside.p_मधला ? db->lookaside.szTrue : LOOKASIDE_SMALL;
#अन्यथा
  परत db->lookaside.szTrue;
#endif  
}
पूर्णांक sqlite3DbMallocSize(sqlite3 *db, स्थिरांक शून्य *p){
  प्रतिज्ञा( p!=0 );
#ifdef SQLITE_DEBUG
  जर( db==0 ){
    प्रतिज्ञा( sqlite3MemdebugNoType(p, (u8)~MEMTYPE_HEAP) );
    प्रतिज्ञा( sqlite3MemdebugHasType(p, MEMTYPE_HEAP) );
  }अन्यथा जर( !isLookaside(db,p) ){
    प्रतिज्ञा( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
    प्रतिज्ञा( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  }
#endif
  जर( db ){
    जर( ((uptr)p)<(uptr)(db->lookaside.pTrueEnd) ){
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
      जर( ((uptr)p)>=(uptr)(db->lookaside.p_मधला) ){
        प्रतिज्ञा( sqlite3_mutex_held(db->mutex) );
        परत LOOKASIDE_SMALL;
      }
#endif
      जर( ((uptr)p)>=(uptr)(db->lookaside.pStart) ){
        प्रतिज्ञा( sqlite3_mutex_held(db->mutex) );
        परत db->lookaside.szTrue;
      }
    }
  }
  परत sqlite3GlobalConfig.m.xSize((शून्य*)p);
}
sqlite3_uint64 sqlite3_msize(शून्य *p){
  प्रतिज्ञा( sqlite3MemdebugNoType(p, (u8)~MEMTYPE_HEAP) );
  प्रतिज्ञा( sqlite3MemdebugHasType(p, MEMTYPE_HEAP) );
  परत p ? sqlite3GlobalConfig.m.xSize(p) : 0;
}

/*
** Free memory previously obtained from sqlite3Malloc().
*/
शून्य sqlite3_free(शून्य *p){
  जर( p==0 ) परत;  /* IMP: R-49053-54554 */
  प्रतिज्ञा( sqlite3MemdebugHasType(p, MEMTYPE_HEAP) );
  प्रतिज्ञा( sqlite3MemdebugNoType(p, (u8)~MEMTYPE_HEAP) );
  जर( sqlite3GlobalConfig.bMemstat ){
    sqlite3_mutex_enter(mem0.mutex);
    sqlite3StatusDown(SQLITE_STATUS_MEMORY_USED, sqlite3MallocSize(p));
    sqlite3StatusDown(SQLITE_STATUS_MALLOC_मोजणी, 1);
    sqlite3GlobalConfig.m.xFree(p);
    sqlite3_mutex_leave(mem0.mutex);
  }अन्यथा{
    sqlite3GlobalConfig.m.xFree(p);
  }
}

/*
** Add the size of memory allocation "p" to the count in
** *db->pnBytesFreed.
*/
स्थिर SQLITE_NOINLINE शून्य measureAllocationSize(sqlite3 *db, शून्य *p){
  *db->pnBytesFreed += sqlite3DbMallocSize(db,p);
}

/*
** Free memory that might be associated with a particular database
** connection.  Calling sqlite3DbFree(D,X) for X==0 is a harmless no-op.
** The sqlite3DbFreeNN(D,X) version requires that X be non-NULL.
*/
शून्य sqlite3DbFreeNN(sqlite3 *db, शून्य *p){
  प्रतिज्ञा( db==0 || sqlite3_mutex_held(db->mutex) );
  प्रतिज्ञा( p!=0 );
  जर( db ){
    जर( ((uptr)p)<(uptr)(db->lookaside.pEnd) ){
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
      जर( ((uptr)p)>=(uptr)(db->lookaside.p_मधला) ){
        LookasideSlot *pBuf = (LookasideSlot*)p;
        प्रतिज्ञा( db->pnBytesFreed==0 );
#ifdef SQLITE_DEBUG
        स_रखो(p, 0xaa, LOOKASIDE_SMALL);  /* Trash freed content */
#endif
        pBuf->pNext = db->lookaside.pSmallFree;
        db->lookaside.pSmallFree = pBuf;
        परत;
      }
#endif /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
      जर( ((uptr)p)>=(uptr)(db->lookaside.pStart) ){
        LookasideSlot *pBuf = (LookasideSlot*)p;
        प्रतिज्ञा( db->pnBytesFreed==0 );
#ifdef SQLITE_DEBUG
        स_रखो(p, 0xaa, db->lookaside.szTrue);  /* Trash freed content */
#endif
        pBuf->pNext = db->lookaside.pFree;
        db->lookaside.pFree = pBuf;
        परत;
      }
    }
    जर( db->pnBytesFreed ){
      measureAllocationSize(db, p);
      परत;
    }
  }
  प्रतिज्ञा( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  प्रतिज्ञा( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  प्रतिज्ञा( db!=0 || sqlite3MemdebugNoType(p, MEMTYPE_LOOKASIDE) );
  sqlite3MemdebugSetType(p, MEMTYPE_HEAP);
  sqlite3_free(p);
}
शून्य sqlite3DbNNFreeNN(sqlite3 *db, शून्य *p){
  प्रतिज्ञा( db!=0 );
  प्रतिज्ञा( sqlite3_mutex_held(db->mutex) );
  प्रतिज्ञा( p!=0 );
  जर( ((uptr)p)<(uptr)(db->lookaside.pEnd) ){
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
    जर( ((uptr)p)>=(uptr)(db->lookaside.p_मधला) ){
      LookasideSlot *pBuf = (LookasideSlot*)p;
      प्रतिज्ञा( db->pnBytesFreed==0 );
#ifdef SQLITE_DEBUG
      स_रखो(p, 0xaa, LOOKASIDE_SMALL);  /* Trash freed content */
#endif
      pBuf->pNext = db->lookaside.pSmallFree;
      db->lookaside.pSmallFree = pBuf;
      परत;
    }
#endif /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
    जर( ((uptr)p)>=(uptr)(db->lookaside.pStart) ){
      LookasideSlot *pBuf = (LookasideSlot*)p;
      प्रतिज्ञा( db->pnBytesFreed==0 );
#ifdef SQLITE_DEBUG
      स_रखो(p, 0xaa, db->lookaside.szTrue);  /* Trash freed content */
#endif
      pBuf->pNext = db->lookaside.pFree;
      db->lookaside.pFree = pBuf;
      परत;
    }
  }
  जर( db->pnBytesFreed ){
    measureAllocationSize(db, p);
    परत;
  }
  प्रतिज्ञा( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  प्रतिज्ञा( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  sqlite3MemdebugSetType(p, MEMTYPE_HEAP);
  sqlite3_free(p);
}
शून्य sqlite3DbFree(sqlite3 *db, शून्य *p){
  प्रतिज्ञा( db==0 || sqlite3_mutex_held(db->mutex) );
  जर( p ) sqlite3DbFreeNN(db, p);
}

/*
** Change the size of an existing memory allocation
*/
शून्य *sqlite3Realloc(शून्य *pOld, u64 nBytes){
  पूर्णांक nOld, nNew, nDiff;
  शून्य *pNew;
  प्रतिज्ञा( sqlite3MemdebugHasType(pOld, MEMTYPE_HEAP) );
  प्रतिज्ञा( sqlite3MemdebugNoType(pOld, (u8)~MEMTYPE_HEAP) );
  जर( pOld==0 ){
    परत sqlite3Malloc(nBytes); /* IMP: R-04300-56712 */
  }
  जर( nBytes==0 ){
    sqlite3_free(pOld); /* IMP: R-26507-47431 */
    परत 0;
  }
  जर( nBytes>SQLITE_MAX_ALLOCATION_SIZE ){
    परत 0;
  }
  nOld = sqlite3MallocSize(pOld);
  /* IMPLEMENTATION-OF: R-46199-30249 SQLite guarantees that the second
  ** argument to xRealloc is always a value returned by a prior call to
  ** xRoundup. */
  nNew = sqlite3GlobalConfig.m.xRoundup((पूर्णांक)nBytes);
  जर( nOld==nNew ){
    pNew = pOld;
  }अन्यथा जर( sqlite3GlobalConfig.bMemstat ){
    sqlite3_int64 nUsed;
    sqlite3_mutex_enter(mem0.mutex);
    sqlite3StatusHighwater(SQLITE_STATUS_MALLOC_SIZE, (पूर्णांक)nBytes);
    nDiff = nNew - nOld;
    जर( nDiff>0 && (nUsed = sqlite3_Status_मूल्य(SQLITE_STATUS_MEMORY_USED)) >= 
          mem0.alarmThreshold-nDiff ){
      sqlite3MallocAlarm(nDiff);
      जर( mem0.hardLimit>0 && nUsed >= mem0.hardLimit - nDiff ){
        sqlite3_mutex_leave(mem0.mutex);
        test_oom_breakpoint(1);
        परत 0;
      }
    }
    pNew = sqlite3GlobalConfig.m.xRealloc(pOld, nNew);
#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
    जर( pNew==0 && mem0.alarmThreshold>0 ){
      sqlite3MallocAlarm((पूर्णांक)nBytes);
      pNew = sqlite3GlobalConfig.m.xRealloc(pOld, nNew);
    }
#endif
    जर( pNew ){
      nNew = sqlite3MallocSize(pNew);
      sqlite3StatusUp(SQLITE_STATUS_MEMORY_USED, nNew-nOld);
    }
    sqlite3_mutex_leave(mem0.mutex);
  }अन्यथा{
    pNew = sqlite3GlobalConfig.m.xRealloc(pOld, nNew);
  }
  प्रतिज्ञा( EIGHT_BYTE_ALIGNMENT(pNew) ); /* IMP: R-11148-40995 */
  परत pNew;
}

/*
** The public interface to sqlite3Realloc.  Make sure that the memory
** subsystem is initialized prior to invoking sqliteRealloc.
*/
शून्य *sqlite3_realloc(शून्य *pOld, पूर्णांक n){
#ifndef SQLITE_OMIT_AUTOINIT
  जर( sqlite3_initialize() ) परत 0;
#endif
  जर( n<0 ) n = 0;  /* IMP: R-26507-47431 */
  परत sqlite3Realloc(pOld, n);
}
शून्य *sqlite3_realloc64(शून्य *pOld, sqlite3_uint64 n){
#ifndef SQLITE_OMIT_AUTOINIT
  जर( sqlite3_initialize() ) परत 0;
#endif
  परत sqlite3Realloc(pOld, n);
}


/*
** Allocate and zero memory.
*/ 
शून्य *sqlite3MallocZero(u64 n){
  शून्य *p = sqlite3Malloc(n);
  जर( p ){
    स_रखो(p, 0, (माप_प्रकार)n);
  }
  परत p;
}

/*
** Allocate and zero memory.  If the allocation fails, make
** the mallocFailed flag in the connection pointer.
*/
शून्य *sqlite3DbMallocZero(sqlite3 *db, u64 n){
  शून्य *p;
  testcase( db==0 );
  p = sqlite3DbMallocRaw(db, n);
  जर( p ) स_रखो(p, 0, (माप_प्रकार)n);
  परत p;
}


/* Finish the work of sqlite3DbMallocRawNN for the unusual and
** slower case when the allocation cannot be fulfilled using lookaside.
*/
स्थिर SQLITE_NOINLINE शून्य *dbMallocRawFinish(sqlite3 *db, u64 n){
  शून्य *p;
  प्रतिज्ञा( db!=0 );
  p = sqlite3Malloc(n);
  जर( !p ) sqlite3OomFault(db);
  sqlite3MemdebugSetType(p, 
         (db->lookaside.bDisable==0) ? MEMTYPE_LOOKASIDE : MEMTYPE_HEAP);
  परत p;
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
शून्य *sqlite3DbMallocRaw(sqlite3 *db, u64 n){
  शून्य *p;
  जर( db ) परत sqlite3DbMallocRawNN(db, n);
  p = sqlite3Malloc(n);
  sqlite3MemdebugSetType(p, MEMTYPE_HEAP);
  परत p;
}
शून्य *sqlite3DbMallocRawNN(sqlite3 *db, u64 n){
#ifndef SQLITE_OMIT_LOOKASIDE
  LookasideSlot *pBuf;
  प्रतिज्ञा( db!=0 );
  प्रतिज्ञा( sqlite3_mutex_held(db->mutex) );
  प्रतिज्ञा( db->pnBytesFreed==0 );
  जर( n>db->lookaside.sz ){
    जर( !db->lookaside.bDisable ){
      db->lookaside.anStat[1]++;      
    }अन्यथा जर( db->mallocFailed ){
      परत 0;
    }
    परत dbMallocRawFinish(db, n);
  }
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
  जर( n<=LOOKASIDE_SMALL ){
    जर( (pBuf = db->lookaside.pSmallFree)!=0 ){
      db->lookaside.pSmallFree = pBuf->pNext;
      db->lookaside.anStat[0]++;
      परत (शून्य*)pBuf;
    }अन्यथा जर( (pBuf = db->lookaside.pSmallInit)!=0 ){
      db->lookaside.pSmallInit = pBuf->pNext;
      db->lookaside.anStat[0]++;
      परत (शून्य*)pBuf;
    }
  }
#endif
  जर( (pBuf = db->lookaside.pFree)!=0 ){
    db->lookaside.pFree = pBuf->pNext;
    db->lookaside.anStat[0]++;
    परत (शून्य*)pBuf;
  }अन्यथा जर( (pBuf = db->lookaside.pInit)!=0 ){
    db->lookaside.pInit = pBuf->pNext;
    db->lookaside.anStat[0]++;
    परत (शून्य*)pBuf;
  }अन्यथा{
    db->lookaside.anStat[2]++;
  }
#अन्यथा
  प्रतिज्ञा( db!=0 );
  प्रतिज्ञा( sqlite3_mutex_held(db->mutex) );
  प्रतिज्ञा( db->pnBytesFreed==0 );
  जर( db->mallocFailed ){
    परत 0;
  }
#endif
  परत dbMallocRawFinish(db, n);
}

/* Forward declaration */
स्थिर SQLITE_NOINLINE शून्य *dbReallocFinish(sqlite3 *db, शून्य *p, u64 n);

/*
** Resize the block of memory pointed to by p to n bytes. If the
** resize fails, set the mallocFailed flag in the connection object.
*/
शून्य *sqlite3DbRealloc(sqlite3 *db, शून्य *p, u64 n){
  प्रतिज्ञा( db!=0 );
  जर( p==0 ) परत sqlite3DbMallocRawNN(db, n);
  प्रतिज्ञा( sqlite3_mutex_held(db->mutex) );
  जर( ((uptr)p)<(uptr)db->lookaside.pEnd ){
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
    जर( ((uptr)p)>=(uptr)db->lookaside.p_मधला ){
      जर( n<=LOOKASIDE_SMALL ) परत p;
    }अन्यथा
#endif
    जर( ((uptr)p)>=(uptr)db->lookaside.pStart ){
      जर( n<=db->lookaside.szTrue ) परत p;
    }
  }
  परत dbReallocFinish(db, p, n);
}
स्थिर SQLITE_NOINLINE शून्य *dbReallocFinish(sqlite3 *db, शून्य *p, u64 n){
  शून्य *pNew = 0;
  प्रतिज्ञा( db!=0 );
  प्रतिज्ञा( p!=0 );
  जर( db->mallocFailed==0 ){
    जर( isLookaside(db, p) ){
      pNew = sqlite3DbMallocRawNN(db, n);
      जर( pNew ){
        स_नकल(pNew, p, lookasideMallocSize(db, p));
        sqlite3DbFree(db, p);
      }
    }अन्यथा{
      प्रतिज्ञा( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
      प्रतिज्ञा( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
      sqlite3MemdebugSetType(p, MEMTYPE_HEAP);
      pNew = sqlite3Realloc(p, n);
      जर( !pNew ){
        sqlite3OomFault(db);
      }
      sqlite3MemdebugSetType(pNew,
            (db->lookaside.bDisable==0 ? MEMTYPE_LOOKASIDE : MEMTYPE_HEAP));
    }
  }
  परत pNew;
}

/*
** Attempt to reallocate p.  If the reallocation fails, then free p
** and set the mallocFailed flag in the database connection.
*/
शून्य *sqlite3DbReallocOrFree(sqlite3 *db, शून्य *p, u64 n){
  शून्य *pNew;
  pNew = sqlite3DbRealloc(db, p, n);
  जर( !pNew ){
    sqlite3DbFree(db, p);
  }
  परत pNew;
}

/*
** Make a copy of a string in memory obtained from sqliteMalloc(). These 
** functions call sqlite3MallocRaw() directly instead of sqliteMalloc(). This
** is because when memory debugging is turned on, these two functions are 
** called via macros that record the current file and line number in the
** ThreadData structure.
*/
अक्षर *sqlite3DbStrDup(sqlite3 *db, स्थिरांक अक्षर *z){
  अक्षर *zNew;
  माप_प्रकार n;
  जर( z==0 ){
    परत 0;
  }
  n = म_माप(z) + 1;
  zNew = sqlite3DbMallocRaw(db, n);
  जर( zNew ){
    स_नकल(zNew, z, n);
  }
  परत zNew;
}
अक्षर *sqlite3DbStrNDup(sqlite3 *db, स्थिरांक अक्षर *z, u64 n){
  अक्षर *zNew;
  प्रतिज्ञा( db!=0 );
  प्रतिज्ञा( z!=0 || n==0 );
  प्रतिज्ञा( (n&0x7fffffff)==n );
  zNew = z ? sqlite3DbMallocRawNN(db, n+1) : 0;
  जर( zNew ){
    स_नकल(zNew, z, (माप_प्रकार)n);
    zNew[n] = 0;
  }
  परत zNew;
}

/*
** The text between zStart and zEnd represents a phrase within a larger
** SQL statement.  Make a copy of this phrase in space obtained form
** sqlite3DbMalloc().  Omit leading and trailing whitespace.
*/
अक्षर *sqlite3DbSpanDup(sqlite3 *db, स्थिरांक अक्षर *zStart, स्थिरांक अक्षर *zEnd){
  पूर्णांक n;
#ifdef SQLITE_DEBUG
  /* Because of the way the parser works, the span is guaranteed to contain
  ** at least one non-space character */
  साठी(n=0; sqlite3Isspace(zStart[n]); n++){ प्रतिज्ञा( &zStart[n]<zEnd ); }
#endif
  पर्यंत( sqlite3Isspace(zStart[0]) ) zStart++;
  n = (पूर्णांक)(zEnd - zStart);
  पर्यंत( sqlite3Isspace(zStart[n-1]) ) n--;
  परत sqlite3DbStrNDup(db, zStart, n);
}

/*
** Free any prior content in *pz and replace it with a copy of zNew.
*/
शून्य sqlite3SetString(अक्षर **pz, sqlite3 *db, स्थिरांक अक्षर *zNew){
  अक्षर *z = sqlite3DbStrDup(db, zNew);
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
शून्य *sqlite3OomFault(sqlite3 *db){
  जर( db->mallocFailed==0 && db->bBenignMalloc==0 ){
    db->mallocFailed = 1;
    जर( db->nVdbeExec>0 ){
      AtomicStore(&db->u1.isInterrupted, 1);
    }
    DisableLookaside;
    जर( db->pParse ){
      Parse *pParse;
      sqlite3_Error_संदेश(db->pParse, "out of memory");
      db->pParse->rc = SQLITE_NOMEM_BKPT;
      साठी(pParse=db->pParse->pOuterParse; pParse; pParse = pParse->pOuterParse){
        pParse->nErr++;
        pParse->rc = SQLITE_NOMEM;
      } 
    }
  }
  परत 0;
}

/*
** This routine reactivates the memory allocator and clears the
** db->mallocFailed flag as necessary.
**
** The memory allocator is not restarted if there are running
** VDBEs.
*/
शून्य sqlite3OomClear(sqlite3 *db){
  जर( db->mallocFailed && db->nVdbeExec==0 ){
    db->mallocFailed = 0;
    AtomicStore(&db->u1.isInterrupted, 0);
    प्रतिज्ञा( db->lookaside.bDisable>0 );
    EnableLookaside;
  }
}

/*
** Take actions at the end of an API call to deal with error codes.
*/
स्थिर SQLITE_NOINLINE पूर्णांक apiHandleError(sqlite3 *db, पूर्णांक rc){
  जर( db->mallocFailed || rc==SQLITE_IOERR_NOMEM ){
    sqlite3OomClear(db);
    sqlite3Error(db, SQLITE_NOMEM);
    परत SQLITE_NOMEM_BKPT;
  }
  परत rc & db->errMask;
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
पूर्णांक sqlite3ApiExit(sqlite3* db, पूर्णांक rc){
  /* If the db handle must hold the connection handle mutex here.
  ** Otherwise the read (and possible write) of db->mallocFailed 
  ** is unsafe, as is the call to sqlite3Error().
  */
  प्रतिज्ञा( db!=0 );
  प्रतिज्ञा( sqlite3_mutex_held(db->mutex) );
  जर( db->mallocFailed || rc ){
    परत apiHandleError(db, rc);
  }
  परत 0;
}
