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
#समावेश "sqliteInt.h"
#समावेश <stdarg.h>

/*
** Attempt to release up to n bytes of non-essential memory currently
** held by SQLite. An example of non-essential memory is memory used to
** cache database pages that are not currently in use.
*/
पूर्णांक sqlite3_release_memory(पूर्णांक n){
#अगर_घोषित SQLITE_ENABLE_MEMORY_MANAGEMENT
  वापस sqlite3PcacheReleaseMemory(n);
#अन्यथा
  /* IMPLEMENTATION-OF: R-34391-24921 The sqlite3_release_memory() routine
  ** is a no-op returning zero if SQLite is not compiled with
  ** SQLITE_ENABLE_MEMORY_MANAGEMENT. */
  UNUSED_PARAMETER(n);
  वापस 0;
#पूर्ण_अगर
}

/*
** Default value of the hard heap limit.  0 means "no limit".
*/
#अगर_अघोषित SQLITE_MAX_MEMORY
# define SQLITE_MAX_MEMORY 0
#पूर्ण_अगर

/*
** State information local to the memory allocation subsystem.
*/
अटल SQLITE_WSD काष्ठा Mem0Global {
  sqlite3_mutex *mutex;         /* Mutex to serialize access */
  sqlite3_int64 alarmThreshold; /* The soft heap limit */
  sqlite3_int64 hardLimit;      /* The hard upper bound on memory */

  /*
  ** True if heap is nearly "full" where "full" is defined by the
  ** sqlite3_soft_heap_limit() setting.
  */
  पूर्णांक nearlyFull;
} mem0 = { 0, SQLITE_MAX_MEMORY, SQLITE_MAX_MEMORY, 0 };

#घोषणा mem0 GLOBAL(काष्ठा Mem0Global, mem0)

/*
** Return the memory allocator mutex. sqlite3_status() needs it.
*/
sqlite3_mutex *sqlite3MallocMutex(व्योम){
  वापस mem0.mutex;
}

#अगर_अघोषित SQLITE_OMIT_DEPRECATED
/*
** Deprecated external interface.  It used to set an alarm callback
** that was invoked when memory usage grew too large.  Now it is a
** no-op.
*/
पूर्णांक sqlite3_memory_alarm(
  व्योम(*xCallback)(व्योम *pArg, sqlite3_int64 used,पूर्णांक N),
  व्योम *pArg,
  sqlite3_int64 iThreshold
){
  (व्योम)xCallback;
  (व्योम)pArg;
  (व्योम)iThreshold;
  वापस SQLITE_OK;
}
#पूर्ण_अगर

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
#अगर_अघोषित SQLITE_OMIT_AUTOINIT
  पूर्णांक rc = sqlite3_initialize();
  अगर( rc ) वापस -1;
#पूर्ण_अगर
  sqlite3_mutex_enter(mem0.mutex);
  priorLimit = mem0.alarmThreshold;
  अगर( n<0 ){
    sqlite3_mutex_leave(mem0.mutex);
    वापस priorLimit;
  }
  अगर( mem0.hardLimit>0 && (n>mem0.hardLimit || n==0) ){
    n = mem0.hardLimit;
  }
  mem0.alarmThreshold = n;
  nUsed = sqlite3_Status_मूल्य(SQLITE_STATUS_MEMORY_USED);
  AtomicStore(&mem0.nearlyFull, n>0 && n<=nUsed);
  sqlite3_mutex_leave(mem0.mutex);
  excess = sqlite3_memory_used() - n;
  अगर( excess>0 ) sqlite3_release_memory((पूर्णांक)(excess & 0x7fffffff));
  वापस priorLimit;
}
व्योम sqlite3_soft_heap_limit(पूर्णांक n){
  अगर( n<0 ) n = 0;
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
#अगर_अघोषित SQLITE_OMIT_AUTOINIT
  पूर्णांक rc = sqlite3_initialize();
  अगर( rc ) वापस -1;
#पूर्ण_अगर
  sqlite3_mutex_enter(mem0.mutex);
  priorLimit = mem0.hardLimit;
  अगर( n>=0 ){
    mem0.hardLimit = n;
    अगर( n<mem0.alarmThreshold || mem0.alarmThreshold==0 ){
      mem0.alarmThreshold = n;
    }
  }
  sqlite3_mutex_leave(mem0.mutex);
  वापस priorLimit;
}


/*
** Initialize the memory allocation subsystem.
*/
पूर्णांक sqlite3MallocInit(व्योम){
  पूर्णांक rc;
  अगर( sqlite3GlobalConfig.m.xMalloc==0 ){
    sqlite3MemSetDefault();
  }
  mem0.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM);
  अगर( sqlite3GlobalConfig.pPage==0 || sqlite3GlobalConfig.szPage<512
      || sqlite3GlobalConfig.nPage<=0 ){
    sqlite3GlobalConfig.pPage = 0;
    sqlite3GlobalConfig.szPage = 0;
  }
  rc = sqlite3GlobalConfig.m.xInit(sqlite3GlobalConfig.m.p_App_तथ्य);
  अगर( rc!=SQLITE_OK ) स_रखो(&mem0, 0, माप(mem0));
  वापस rc;
}

/*
** Return true if the heap is currently under memory pressure - in other
** words if the amount of heap used is close to the limit set by
** sqlite3_soft_heap_limit().
*/
पूर्णांक sqlite3HeapNearlyFull(व्योम){
  वापस AtomicLoad(&mem0.nearlyFull);
}

/*
** Deinitialize the memory allocation subsystem.
*/
व्योम sqlite3MallocEnd(व्योम){
  अगर( sqlite3GlobalConfig.m.xShutdown ){
    sqlite3GlobalConfig.m.xShutdown(sqlite3GlobalConfig.m.p_App_तथ्य);
  }
  स_रखो(&mem0, 0, माप(mem0));
}

/*
** Return the amount of memory currently checked out.
*/
sqlite3_int64 sqlite3_memory_used(व्योम){
  sqlite3_int64 res, mx;
  sqlite3_status64(SQLITE_STATUS_MEMORY_USED, &res, &mx, 0);
  वापस res;
}

/*
** Return the maximum amount of memory that has ever been
** checked out since either the beginning of this process
** or since the most recent reset.
*/
sqlite3_int64 sqlite3_memory_highwater(पूर्णांक resetFlag){
  sqlite3_int64 res, mx;
  sqlite3_status64(SQLITE_STATUS_MEMORY_USED, &res, &mx, resetFlag);
  वापस mx;
}

/*
** Trigger the alarm 
*/
अटल व्योम sqlite3MallocAlarm(पूर्णांक nByte){
  अगर( mem0.alarmThreshold<=0 ) वापस;
  sqlite3_mutex_leave(mem0.mutex);
  sqlite3_release_memory(nByte);
  sqlite3_mutex_enter(mem0.mutex);
}

#अगर_घोषित SQLITE_DEBUG
/*
** This routine is called whenever an out-of-memory condition is seen,
** It's only purpose to to serve as a breakpoint for gdb or similar
** code debuggers when working on out-of-memory conditions, for example
** caused by PRAGMA hard_heap_limit=N.
*/
अटल SQLITE_NOINLINE व्योम test_oom_breakpoint(u64 n){
  अटल u64 nOomFault = 0;
  nOomFault += n;
  /* The assert() is never reached in a human lifetime.  It  is here mostly
  ** to prevent code optimizers from optimizing out this function. */
  निश्चित( (nOomFault>>32) < 0xffffffff );
}
#अन्यथा
# define test_oom_breakpoint(X)   /* No-op for production builds */
#पूर्ण_अगर

/*
** Do a memory allocation with statistics and alarms.  Assume the
** lock is already held.
*/
अटल व्योम mallocWithAlarm(पूर्णांक n, व्योम **pp){
  व्योम *p;
  पूर्णांक nFull;
  निश्चित( sqlite3_mutex_held(mem0.mutex) );
  निश्चित( n>0 );

  /* In Firefox (circa 2017-02-08), xRoundup() is remapped to an internal
  ** implementation of malloc_good_size(), which must be called in debug
  ** mode and specifically when the DMD "Dark Matter Detector" is enabled
  ** or else a crash results.  Hence, do not attempt to optimize out the
  ** following xRoundup() call. */
  nFull = sqlite3GlobalConfig.m.xRoundup(n);

  sqlite3StatusHighwater(SQLITE_STATUS_MALLOC_आकार, n);
  अगर( mem0.alarmThreshold>0 ){
    sqlite3_int64 nUsed = sqlite3_Status_मूल्य(SQLITE_STATUS_MEMORY_USED);
    अगर( nUsed >= mem0.alarmThreshold - nFull ){
      AtomicStore(&mem0.nearlyFull, 1);
      sqlite3MallocAlarm(nFull);
      अगर( mem0.hardLimit ){
        nUsed = sqlite3_Status_मूल्य(SQLITE_STATUS_MEMORY_USED);
        अगर( nUsed >= mem0.hardLimit - nFull ){
          test_oom_breakpoint(1);
          *pp = 0;
          वापस;
        }
      }
    }अन्यथा{
      AtomicStore(&mem0.nearlyFull, 0);
    }
  }
  p = sqlite3GlobalConfig.m.xMalloc(nFull);
#अगर_घोषित SQLITE_ENABLE_MEMORY_MANAGEMENT
  अगर( p==0 && mem0.alarmThreshold>0 ){
    sqlite3MallocAlarm(nFull);
    p = sqlite3GlobalConfig.m.xMalloc(nFull);
  }
#पूर्ण_अगर
  अगर( p ){
    nFull = sqlite3_Malloc_आकार(p);
    sqlite3StatusUp(SQLITE_STATUS_MEMORY_USED, nFull);
    sqlite3StatusUp(SQLITE_STATUS_MALLOC_गिनती, 1);
  }
  *pp = p;
}

/*
** Allocate memory.  This routine is like sqlite3_malloc() except that it
** assumes the memory subsystem has already been initialized.
*/
व्योम *sqlite3Malloc(u64 n){
  व्योम *p;
  अगर( n==0 || n>SQLITE_MAX_ALLOCATION_आकार ){
    p = 0;
  }अन्यथा अगर( sqlite3GlobalConfig.bMemstat ){
    sqlite3_mutex_enter(mem0.mutex);
    mallocWithAlarm((पूर्णांक)n, &p);
    sqlite3_mutex_leave(mem0.mutex);
  }अन्यथा{
    p = sqlite3GlobalConfig.m.xMalloc((पूर्णांक)n);
  }
  निश्चित( EIGHT_BYTE_ALIGNMENT(p) );  /* IMP: R-11148-40995 */
  वापस p;
}

/*
** This version of the memory allocation is for use by the application.
** First make sure the memory subsystem is initialized, then do the
** allocation.
*/
व्योम *sqlite3_malloc(पूर्णांक n){
#अगर_अघोषित SQLITE_OMIT_AUTOINIT
  अगर( sqlite3_initialize() ) वापस 0;
#पूर्ण_अगर
  वापस n<=0 ? 0 : sqlite3Malloc(n);
}
व्योम *sqlite3_malloc64(sqlite3_uint64 n){
#अगर_अघोषित SQLITE_OMIT_AUTOINIT
  अगर( sqlite3_initialize() ) वापस 0;
#पूर्ण_अगर
  वापस sqlite3Malloc(n);
}

/*
** TRUE if p is a lookaside memory allocation from db
*/
#अगर_अघोषित SQLITE_OMIT_LOOKASIDE
अटल पूर्णांक isLookaside(sqlite3 *db, स्थिर व्योम *p){
  वापस SQLITE_WITHIN(p, db->lookaside.pStart, db->lookaside.pTrueEnd);
}
#अन्यथा
#घोषणा isLookaside(A,B) 0
#पूर्ण_अगर

/*
** Return the size of a memory allocation previously obtained from
** sqlite3Malloc() or sqlite3_malloc().
*/
पूर्णांक sqlite3_Malloc_आकार(स्थिर व्योम *p){
  निश्चित( sqlite3MemdebugHasType(p, MEMTYPE_HEAP) );
  वापस sqlite3GlobalConfig.m.x_आकार((व्योम*)p);
}
अटल पूर्णांक lookaside_Malloc_आकार(sqlite3 *db, स्थिर व्योम *p){
#अगर_अघोषित SQLITE_OMIT_TWOSIZE_LOOKASIDE    
  वापस p<db->lookaside.p_मध्य ? db->lookaside.szTrue : LOOKASIDE_SMALL;
#अन्यथा
  वापस db->lookaside.szTrue;
#पूर्ण_अगर  
}
पूर्णांक sqlite3_Db_Malloc_आकार(sqlite3 *db, स्थिर व्योम *p){
  निश्चित( p!=0 );
#अगर_घोषित SQLITE_DEBUG
  अगर( db==0 ){
    निश्चित( sqlite3MemdebugNoType(p, (u8)~MEMTYPE_HEAP) );
    निश्चित( sqlite3MemdebugHasType(p, MEMTYPE_HEAP) );
  }अन्यथा अगर( !isLookaside(db,p) ){
    निश्चित( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
    निश्चित( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  }
#पूर्ण_अगर
  अगर( db ){
    अगर( ((uptr)p)<(uptr)(db->lookaside.pTrueEnd) ){
#अगर_अघोषित SQLITE_OMIT_TWOSIZE_LOOKASIDE
      अगर( ((uptr)p)>=(uptr)(db->lookaside.p_मध्य) ){
        निश्चित( sqlite3_mutex_held(db->mutex) );
        वापस LOOKASIDE_SMALL;
      }
#पूर्ण_अगर
      अगर( ((uptr)p)>=(uptr)(db->lookaside.pStart) ){
        निश्चित( sqlite3_mutex_held(db->mutex) );
        वापस db->lookaside.szTrue;
      }
    }
  }
  वापस sqlite3GlobalConfig.m.x_आकार((व्योम*)p);
}
sqlite3_uint64 sqlite3_msize(व्योम *p){
  निश्चित( sqlite3MemdebugNoType(p, (u8)~MEMTYPE_HEAP) );
  निश्चित( sqlite3MemdebugHasType(p, MEMTYPE_HEAP) );
  वापस p ? sqlite3GlobalConfig.m.x_आकार(p) : 0;
}

/*
** Free memory previously obtained from sqlite3Malloc().
*/
व्योम sqlite3_free(व्योम *p){
  अगर( p==0 ) वापस;  /* IMP: R-49053-54554 */
  निश्चित( sqlite3MemdebugHasType(p, MEMTYPE_HEAP) );
  निश्चित( sqlite3MemdebugNoType(p, (u8)~MEMTYPE_HEAP) );
  अगर( sqlite3GlobalConfig.bMemstat ){
    sqlite3_mutex_enter(mem0.mutex);
    sqlite3StatusDown(SQLITE_STATUS_MEMORY_USED, sqlite3_Malloc_आकार(p));
    sqlite3StatusDown(SQLITE_STATUS_MALLOC_गिनती, 1);
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
अटल SQLITE_NOINLINE व्योम measure_Allocation_आकार(sqlite3 *db, व्योम *p){
  *db->pnBytesFreed += sqlite3_Db_Malloc_आकार(db,p);
}

/*
** Free memory that might be associated with a particular database
** connection.  Calling sqlite3DbFree(D,X) for X==0 is a harmless no-op.
** The sqlite3DbFreeNN(D,X) version requires that X be non-NULL.
*/
व्योम sqlite3DbFreeNN(sqlite3 *db, व्योम *p){
  निश्चित( db==0 || sqlite3_mutex_held(db->mutex) );
  निश्चित( p!=0 );
  अगर( db ){
    अगर( ((uptr)p)<(uptr)(db->lookaside.pEnd) ){
#अगर_अघोषित SQLITE_OMIT_TWOSIZE_LOOKASIDE
      अगर( ((uptr)p)>=(uptr)(db->lookaside.p_मध्य) ){
        LookasideSlot *pBuf = (LookasideSlot*)p;
        निश्चित( db->pnBytesFreed==0 );
#अगर_घोषित SQLITE_DEBUG
        स_रखो(p, 0xaa, LOOKASIDE_SMALL);  /* Trash freed content */
#पूर्ण_अगर
        pBuf->p_अग्र = db->lookaside.pSmallFree;
        db->lookaside.pSmallFree = pBuf;
        वापस;
      }
#पूर्ण_अगर /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
      अगर( ((uptr)p)>=(uptr)(db->lookaside.pStart) ){
        LookasideSlot *pBuf = (LookasideSlot*)p;
        निश्चित( db->pnBytesFreed==0 );
#अगर_घोषित SQLITE_DEBUG
        स_रखो(p, 0xaa, db->lookaside.szTrue);  /* Trash freed content */
#पूर्ण_अगर
        pBuf->p_अग्र = db->lookaside.pFree;
        db->lookaside.pFree = pBuf;
        वापस;
      }
    }
    अगर( db->pnBytesFreed ){
      measure_Allocation_आकार(db, p);
      वापस;
    }
  }
  निश्चित( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  निश्चित( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  निश्चित( db!=0 || sqlite3MemdebugNoType(p, MEMTYPE_LOOKASIDE) );
  sqlite3MemdebugSetType(p, MEMTYPE_HEAP);
  sqlite3_free(p);
}
व्योम sqlite3DbNNFreeNN(sqlite3 *db, व्योम *p){
  निश्चित( db!=0 );
  निश्चित( sqlite3_mutex_held(db->mutex) );
  निश्चित( p!=0 );
  अगर( ((uptr)p)<(uptr)(db->lookaside.pEnd) ){
#अगर_अघोषित SQLITE_OMIT_TWOSIZE_LOOKASIDE
    अगर( ((uptr)p)>=(uptr)(db->lookaside.p_मध्य) ){
      LookasideSlot *pBuf = (LookasideSlot*)p;
      निश्चित( db->pnBytesFreed==0 );
#अगर_घोषित SQLITE_DEBUG
      स_रखो(p, 0xaa, LOOKASIDE_SMALL);  /* Trash freed content */
#पूर्ण_अगर
      pBuf->p_अग्र = db->lookaside.pSmallFree;
      db->lookaside.pSmallFree = pBuf;
      वापस;
    }
#पूर्ण_अगर /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
    अगर( ((uptr)p)>=(uptr)(db->lookaside.pStart) ){
      LookasideSlot *pBuf = (LookasideSlot*)p;
      निश्चित( db->pnBytesFreed==0 );
#अगर_घोषित SQLITE_DEBUG
      स_रखो(p, 0xaa, db->lookaside.szTrue);  /* Trash freed content */
#पूर्ण_अगर
      pBuf->p_अग्र = db->lookaside.pFree;
      db->lookaside.pFree = pBuf;
      वापस;
    }
  }
  अगर( db->pnBytesFreed ){
    measure_Allocation_आकार(db, p);
    वापस;
  }
  निश्चित( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  निश्चित( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  sqlite3MemdebugSetType(p, MEMTYPE_HEAP);
  sqlite3_free(p);
}
व्योम sqlite3DbFree(sqlite3 *db, व्योम *p){
  निश्चित( db==0 || sqlite3_mutex_held(db->mutex) );
  अगर( p ) sqlite3DbFreeNN(db, p);
}

/*
** Change the size of an existing memory allocation
*/
व्योम *sqlite3Realloc(व्योम *pOld, u64 nBytes){
  पूर्णांक nOld, nNew, nDiff;
  व्योम *pNew;
  निश्चित( sqlite3MemdebugHasType(pOld, MEMTYPE_HEAP) );
  निश्चित( sqlite3MemdebugNoType(pOld, (u8)~MEMTYPE_HEAP) );
  अगर( pOld==0 ){
    वापस sqlite3Malloc(nBytes); /* IMP: R-04300-56712 */
  }
  अगर( nBytes==0 ){
    sqlite3_free(pOld); /* IMP: R-26507-47431 */
    वापस 0;
  }
  अगर( nBytes>SQLITE_MAX_ALLOCATION_आकार ){
    वापस 0;
  }
  nOld = sqlite3_Malloc_आकार(pOld);
  /* IMPLEMENTATION-OF: R-46199-30249 SQLite guarantees that the second
  ** argument to xRealloc is always a value returned by a prior call to
  ** xRoundup. */
  nNew = sqlite3GlobalConfig.m.xRoundup((पूर्णांक)nBytes);
  अगर( nOld==nNew ){
    pNew = pOld;
  }अन्यथा अगर( sqlite3GlobalConfig.bMemstat ){
    sqlite3_int64 nUsed;
    sqlite3_mutex_enter(mem0.mutex);
    sqlite3StatusHighwater(SQLITE_STATUS_MALLOC_आकार, (पूर्णांक)nBytes);
    nDiff = nNew - nOld;
    अगर( nDiff>0 && (nUsed = sqlite3_Status_मूल्य(SQLITE_STATUS_MEMORY_USED)) >= 
          mem0.alarmThreshold-nDiff ){
      sqlite3MallocAlarm(nDiff);
      अगर( mem0.hardLimit>0 && nUsed >= mem0.hardLimit - nDiff ){
        sqlite3_mutex_leave(mem0.mutex);
        test_oom_breakpoint(1);
        वापस 0;
      }
    }
    pNew = sqlite3GlobalConfig.m.xRealloc(pOld, nNew);
#अगर_घोषित SQLITE_ENABLE_MEMORY_MANAGEMENT
    अगर( pNew==0 && mem0.alarmThreshold>0 ){
      sqlite3MallocAlarm((पूर्णांक)nBytes);
      pNew = sqlite3GlobalConfig.m.xRealloc(pOld, nNew);
    }
#पूर्ण_अगर
    अगर( pNew ){
      nNew = sqlite3_Malloc_आकार(pNew);
      sqlite3StatusUp(SQLITE_STATUS_MEMORY_USED, nNew-nOld);
    }
    sqlite3_mutex_leave(mem0.mutex);
  }अन्यथा{
    pNew = sqlite3GlobalConfig.m.xRealloc(pOld, nNew);
  }
  निश्चित( EIGHT_BYTE_ALIGNMENT(pNew) ); /* IMP: R-11148-40995 */
  वापस pNew;
}

/*
** The public interface to sqlite3Realloc.  Make sure that the memory
** subsystem is initialized prior to invoking sqliteRealloc.
*/
व्योम *sqlite3_realloc(व्योम *pOld, पूर्णांक n){
#अगर_अघोषित SQLITE_OMIT_AUTOINIT
  अगर( sqlite3_initialize() ) वापस 0;
#पूर्ण_अगर
  अगर( n<0 ) n = 0;  /* IMP: R-26507-47431 */
  वापस sqlite3Realloc(pOld, n);
}
व्योम *sqlite3_realloc64(व्योम *pOld, sqlite3_uint64 n){
#अगर_अघोषित SQLITE_OMIT_AUTOINIT
  अगर( sqlite3_initialize() ) वापस 0;
#पूर्ण_अगर
  वापस sqlite3Realloc(pOld, n);
}


/*
** Allocate and zero memory.
*/ 
व्योम *sqlite3MallocZero(u64 n){
  व्योम *p = sqlite3Malloc(n);
  अगर( p ){
    स_रखो(p, 0, (माप_प्रकार)n);
  }
  वापस p;
}

/*
** Allocate and zero memory.  If the allocation fails, make
** the mallocFailed flag in the connection pointer.
*/
व्योम *sqlite3DbMallocZero(sqlite3 *db, u64 n){
  व्योम *p;
  testcase( db==0 );
  p = sqlite3DbMallocRaw(db, n);
  अगर( p ) स_रखो(p, 0, (माप_प्रकार)n);
  वापस p;
}


/* Finish the work of sqlite3DbMallocRawNN for the unusual and
** slower case when the allocation cannot be fulfilled using lookaside.
*/
अटल SQLITE_NOINLINE व्योम *dbMallocRawFinish(sqlite3 *db, u64 n){
  व्योम *p;
  निश्चित( db!=0 );
  p = sqlite3Malloc(n);
  अगर( !p ) sqlite3OomFault(db);
  sqlite3MemdebugSetType(p, 
         (db->lookaside.bDisable==0) ? MEMTYPE_LOOKASIDE : MEMTYPE_HEAP);
  वापस p;
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
व्योम *sqlite3DbMallocRaw(sqlite3 *db, u64 n){
  व्योम *p;
  अगर( db ) वापस sqlite3DbMallocRawNN(db, n);
  p = sqlite3Malloc(n);
  sqlite3MemdebugSetType(p, MEMTYPE_HEAP);
  वापस p;
}
व्योम *sqlite3DbMallocRawNN(sqlite3 *db, u64 n){
#अगर_अघोषित SQLITE_OMIT_LOOKASIDE
  LookasideSlot *pBuf;
  निश्चित( db!=0 );
  निश्चित( sqlite3_mutex_held(db->mutex) );
  निश्चित( db->pnBytesFreed==0 );
  अगर( n>db->lookaside.sz ){
    अगर( !db->lookaside.bDisable ){
      db->lookaside.anStat[1]++;      
    }अन्यथा अगर( db->mallocFailed ){
      वापस 0;
    }
    वापस dbMallocRawFinish(db, n);
  }
#अगर_अघोषित SQLITE_OMIT_TWOSIZE_LOOKASIDE
  अगर( n<=LOOKASIDE_SMALL ){
    अगर( (pBuf = db->lookaside.pSmallFree)!=0 ){
      db->lookaside.pSmallFree = pBuf->p_अग्र;
      db->lookaside.anStat[0]++;
      वापस (व्योम*)pBuf;
    }अन्यथा अगर( (pBuf = db->lookaside.pSmallInit)!=0 ){
      db->lookaside.pSmallInit = pBuf->p_अग्र;
      db->lookaside.anStat[0]++;
      वापस (व्योम*)pBuf;
    }
  }
#पूर्ण_अगर
  अगर( (pBuf = db->lookaside.pFree)!=0 ){
    db->lookaside.pFree = pBuf->p_अग्र;
    db->lookaside.anStat[0]++;
    वापस (व्योम*)pBuf;
  }अन्यथा अगर( (pBuf = db->lookaside.pInit)!=0 ){
    db->lookaside.pInit = pBuf->p_अग्र;
    db->lookaside.anStat[0]++;
    वापस (व्योम*)pBuf;
  }अन्यथा{
    db->lookaside.anStat[2]++;
  }
#अन्यथा
  निश्चित( db!=0 );
  निश्चित( sqlite3_mutex_held(db->mutex) );
  निश्चित( db->pnBytesFreed==0 );
  अगर( db->mallocFailed ){
    वापस 0;
  }
#पूर्ण_अगर
  वापस dbMallocRawFinish(db, n);
}

/* Forward declaration */
अटल SQLITE_NOINLINE व्योम *dbReallocFinish(sqlite3 *db, व्योम *p, u64 n);

/*
** Resize the block of memory pointed to by p to n bytes. If the
** resize fails, set the mallocFailed flag in the connection object.
*/
व्योम *sqlite3DbRealloc(sqlite3 *db, व्योम *p, u64 n){
  निश्चित( db!=0 );
  अगर( p==0 ) वापस sqlite3DbMallocRawNN(db, n);
  निश्चित( sqlite3_mutex_held(db->mutex) );
  अगर( ((uptr)p)<(uptr)db->lookaside.pEnd ){
#अगर_अघोषित SQLITE_OMIT_TWOSIZE_LOOKASIDE
    अगर( ((uptr)p)>=(uptr)db->lookaside.p_मध्य ){
      अगर( n<=LOOKASIDE_SMALL ) वापस p;
    }अन्यथा
#पूर्ण_अगर
    अगर( ((uptr)p)>=(uptr)db->lookaside.pStart ){
      अगर( n<=db->lookaside.szTrue ) वापस p;
    }
  }
  वापस dbReallocFinish(db, p, n);
}
अटल SQLITE_NOINLINE व्योम *dbReallocFinish(sqlite3 *db, व्योम *p, u64 n){
  व्योम *pNew = 0;
  निश्चित( db!=0 );
  निश्चित( p!=0 );
  अगर( db->mallocFailed==0 ){
    अगर( isLookaside(db, p) ){
      pNew = sqlite3DbMallocRawNN(db, n);
      अगर( pNew ){
        स_नकल(pNew, p, lookaside_Malloc_आकार(db, p));
        sqlite3DbFree(db, p);
      }
    }अन्यथा{
      निश्चित( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
      निश्चित( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
      sqlite3MemdebugSetType(p, MEMTYPE_HEAP);
      pNew = sqlite3Realloc(p, n);
      अगर( !pNew ){
        sqlite3OomFault(db);
      }
      sqlite3MemdebugSetType(pNew,
            (db->lookaside.bDisable==0 ? MEMTYPE_LOOKASIDE : MEMTYPE_HEAP));
    }
  }
  वापस pNew;
}

/*
** Attempt to reallocate p.  If the reallocation fails, then free p
** and set the mallocFailed flag in the database connection.
*/
व्योम *sqlite3DbReallocOrFree(sqlite3 *db, व्योम *p, u64 n){
  व्योम *pNew;
  pNew = sqlite3DbRealloc(db, p, n);
  अगर( !pNew ){
    sqlite3DbFree(db, p);
  }
  वापस pNew;
}

/*
** Make a copy of a string in memory obtained from sqliteMalloc(). These 
** functions call sqlite3MallocRaw() directly instead of sqliteMalloc(). This
** is because when memory debugging is turned on, these two functions are 
** called via macros that record the current file and line number in the
** ThreadData structure.
*/
अक्षर *sqlite3DbStrDup(sqlite3 *db, स्थिर अक्षर *z){
  अक्षर *zNew;
  माप_प्रकार n;
  अगर( z==0 ){
    वापस 0;
  }
  n = म_माप(z) + 1;
  zNew = sqlite3DbMallocRaw(db, n);
  अगर( zNew ){
    स_नकल(zNew, z, n);
  }
  वापस zNew;
}
अक्षर *sqlite3DbStrNDup(sqlite3 *db, स्थिर अक्षर *z, u64 n){
  अक्षर *zNew;
  निश्चित( db!=0 );
  निश्चित( z!=0 || n==0 );
  निश्चित( (n&0x7fffffff)==n );
  zNew = z ? sqlite3DbMallocRawNN(db, n+1) : 0;
  अगर( zNew ){
    स_नकल(zNew, z, (माप_प्रकार)n);
    zNew[n] = 0;
  }
  वापस zNew;
}

/*
** The text between zStart and zEnd represents a phrase within a larger
** SQL statement.  Make a copy of this phrase in space obtained form
** sqlite3DbMalloc().  Omit leading and trailing whitespace.
*/
अक्षर *sqlite3DbSpanDup(sqlite3 *db, स्थिर अक्षर *zStart, स्थिर अक्षर *zEnd){
  पूर्णांक n;
#अगर_घोषित SQLITE_DEBUG
  /* Because of the way the parser works, the span is guaranteed to contain
  ** at least one non-space character */
  क्रम(n=0; sqlite3Isspace(zStart[n]); n++){ निश्चित( &zStart[n]<zEnd ); }
#पूर्ण_अगर
  जबतक( sqlite3Isspace(zStart[0]) ) zStart++;
  n = (पूर्णांक)(zEnd - zStart);
  जबतक( sqlite3Isspace(zStart[n-1]) ) n--;
  वापस sqlite3DbStrNDup(db, zStart, n);
}

/*
** Free any prior content in *pz and replace it with a copy of zNew.
*/
व्योम sqlite3SetString(अक्षर **pz, sqlite3 *db, स्थिर अक्षर *zNew){
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
व्योम *sqlite3OomFault(sqlite3 *db){
  अगर( db->mallocFailed==0 && db->bBenignMalloc==0 ){
    db->mallocFailed = 1;
    अगर( db->nVdbeExec>0 ){
      AtomicStore(&db->u1.isInterrupted, 1);
    }
    DisableLookaside;
    अगर( db->pParse ){
      Parse *pParse;
      sqlite3_Error_संदेश(db->pParse, "out of memory");
      db->pParse->rc = SQLITE_NOMEM_BKPT;
      क्रम(pParse=db->pParse->pOuterParse; pParse; pParse = pParse->pOuterParse){
        pParse->nErr++;
        pParse->rc = SQLITE_NOMEM;
      } 
    }
  }
  वापस 0;
}

/*
** This routine reactivates the memory allocator and clears the
** db->mallocFailed flag as necessary.
**
** The memory allocator is not restarted if there are running
** VDBEs.
*/
व्योम sqlite3OomClear(sqlite3 *db){
  अगर( db->mallocFailed && db->nVdbeExec==0 ){
    db->mallocFailed = 0;
    AtomicStore(&db->u1.isInterrupted, 0);
    निश्चित( db->lookaside.bDisable>0 );
    EnableLookaside;
  }
}

/*
** Take actions at the end of an API call to deal with error codes.
*/
अटल SQLITE_NOINLINE पूर्णांक apiHandleError(sqlite3 *db, पूर्णांक rc){
  अगर( db->mallocFailed || rc==SQLITE_IOERR_NOMEM ){
    sqlite3OomClear(db);
    sqlite3Error(db, SQLITE_NOMEM);
    वापस SQLITE_NOMEM_BKPT;
  }
  वापस rc & db->errMask;
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
  निश्चित( db!=0 );
  निश्चित( sqlite3_mutex_held(db->mutex) );
  अगर( db->mallocFailed || rc ){
    वापस apiHandleError(db, rc);
  }
  वापस 0;
}
