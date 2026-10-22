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
#ضمن "sqliteInt.h"
#ضمن <stdarg.h>

/*
** Attempt to release up to n bytes of non-essential memory currently
** held by SQLite. An example of non-essential memory is memory used to
** cache database pages that are not currently in use.
*/
صحيح sqlite3_release_يادگيري(صحيح n){
#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_يادگيري_MANAGEMENT
  أرجع sqlite3_Pcache_Release_يادگيري(n);
#وإلا
  /* IMPLEMENTATION-OF: R-34391-24921 The sqlite3_release_memory() routine
  ** is a no-op returning zero if SQLite is not compiled with
  ** SQLITE_ENABLE_MEMORY_MANAGEMENT. */
  اڻ_استعمال_ٿيل_PARAMETER(n);
  أرجع 0;
#جيڪڏهن_پورو
}

/*
** Default value of the hard heap limit.  0 means "no limit".
*/
#جيڪڏهن_وصف_نه_ٿيل SQLITE_MAX_يادگيري
# define SQLITE_MAX_يادگيري 0
#جيڪڏهن_پورو

/*
** State information local to the memory allocation subsystem.
*/
ساكن SQLITE_WSD بنية Mem0Global {
  sqlite3_mutex *mutex;         /* Mutex to serialize access */
  sqlite3_int64 alarm_حد; /* The soft heap limit */
  sqlite3_int64 hard_حد_بندي;      /* The hard upper bound on memory */

  /*
  ** True if heap is nearly "full" where "full" is defined by the
  ** sqlite3_soft_heap_limit() setting.
  */
  صحيح nearly_ڀريل;
} mem0 = { 0, SQLITE_MAX_يادگيري, SQLITE_MAX_يادگيري, 0 };

#وصف_ڪريو mem0 GLOBAL(بنية Mem0Global, mem0)

/*
** Return the memory allocator mutex. sqlite3_status() needs it.
*/
sqlite3_mutex *sqlite3MallocMutex(فراغ){
  أرجع mem0.mutex;
}

#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_DEPRECATED
/*
** Deprecated external interface.  It used to set an alarm callback
** that was invoked when memory usage grew too large.  Now it is a
** no-op.
*/
صحيح sqlite3_يادگيري_alarm(
  فراغ(*xCallback)(فراغ *pArg, sqlite3_int64 استعمال_ٿيل,صحيح N),
  فراغ *pArg,
  sqlite3_int64 i_حد
){
  (فراغ)xCallback;
  (فراغ)pArg;
  (فراغ)i_حد;
  أرجع SQLITE_OK;
}
#جيڪڏهن_پورو

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
sqlite3_int64 sqlite3_soft_ڍير_limit64(sqlite3_int64 n){
  sqlite3_int64 اڳواٽ_حد_بندي;
  sqlite3_int64 excess;
  sqlite3_int64 n_استعمال_ٿيل;
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_AUTOINIT
  صحيح rc = sqlite3_initialize();
  إذا( rc ) أرجع -1;
#جيڪڏهن_پورو
  sqlite3_mutex_enter(mem0.mutex);
  اڳواٽ_حد_بندي = mem0.alarm_حد;
  إذا( n<0 ){
    sqlite3_mutex_leave(mem0.mutex);
    أرجع اڳواٽ_حد_بندي;
  }
  إذا( mem0.hard_حد_بندي>0 && (n>mem0.hard_حد_بندي || n==0) ){
    n = mem0.hard_حد_بندي;
  }
  mem0.alarm_حد = n;
  n_استعمال_ٿيل = sqlite3_حالت_قدر(SQLITE_حالت_يادگيري_استعمال_ٿيل);
  AtomicStore(&mem0.nearly_ڀريل, n>0 && n<=n_استعمال_ٿيل);
  sqlite3_mutex_leave(mem0.mutex);
  excess = sqlite3_يادگيري_استعمال_ٿيل() - n;
  إذا( excess>0 ) sqlite3_release_يادگيري((صحيح)(excess & 0x7fffffff));
  أرجع اڳواٽ_حد_بندي;
}
فراغ sqlite3_soft_ڍير_حد_بندي(صحيح n){
  إذا( n<0 ) n = 0;
  sqlite3_soft_ڍير_limit64(n);
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
sqlite3_int64 sqlite3_hard_ڍير_limit64(sqlite3_int64 n){
  sqlite3_int64 اڳواٽ_حد_بندي;
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_AUTOINIT
  صحيح rc = sqlite3_initialize();
  إذا( rc ) أرجع -1;
#جيڪڏهن_پورو
  sqlite3_mutex_enter(mem0.mutex);
  اڳواٽ_حد_بندي = mem0.hard_حد_بندي;
  إذا( n>=0 ){
    mem0.hard_حد_بندي = n;
    إذا( n<mem0.alarm_حد || mem0.alarm_حد==0 ){
      mem0.alarm_حد = n;
    }
  }
  sqlite3_mutex_leave(mem0.mutex);
  أرجع اڳواٽ_حد_بندي;
}


/*
** Initialize the memory allocation subsystem.
*/
صحيح sqlite3MallocInit(فراغ){
  صحيح rc;
  إذا( sqlite3GlobalConfig.m.xMalloc==0 ){
    sqlite3_Mem_سيٽ_Default();
  }
  mem0.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM);
  إذا( sqlite3GlobalConfig.p_صفحو==0 || sqlite3GlobalConfig.sz_صفحو<512
      || sqlite3GlobalConfig.n_صفحو<=0 ){
    sqlite3GlobalConfig.p_صفحو = 0;
    sqlite3GlobalConfig.sz_صفحو = 0;
  }
  rc = sqlite3GlobalConfig.m.xInit(sqlite3GlobalConfig.m.p_App_معلومات);
  إذا( rc!=SQLITE_OK ) memset(&mem0, 0, حجم(mem0));
  أرجع rc;
}

/*
** Return true if the heap is currently under memory pressure - in other
** words if the amount of heap used is close to the limit set by
** sqlite3_soft_heap_limit().
*/
صحيح sqlite3_ڍير_Nearly_ڀريل(فراغ){
  أرجع Atomic_ڀريو(&mem0.nearly_ڀريل);
}

/*
** Deinitialize the memory allocation subsystem.
*/
فراغ sqlite3_Malloc_پڇاڙي(فراغ){
  إذا( sqlite3GlobalConfig.m.xShutdown ){
    sqlite3GlobalConfig.m.xShutdown(sqlite3GlobalConfig.m.p_App_معلومات);
  }
  memset(&mem0, 0, حجم(mem0));
}

/*
** Return the amount of memory currently checked out.
*/
sqlite3_int64 sqlite3_يادگيري_استعمال_ٿيل(فراغ){
  sqlite3_int64 res, mx;
  sqlite3_status64(SQLITE_حالت_يادگيري_استعمال_ٿيل, &res, &mx, 0);
  أرجع res;
}

/*
** Return the maximum amount of memory that has ever been
** checked out since either the beginning of this process
** or since the most recent reset.
*/
sqlite3_int64 sqlite3_يادگيري_highwater(صحيح resetFlag){
  sqlite3_int64 res, mx;
  sqlite3_status64(SQLITE_حالت_يادگيري_استعمال_ٿيل, &res, &mx, resetFlag);
  أرجع mx;
}

/*
** Trigger the alarm 
*/
ساكن فراغ sqlite3MallocAlarm(صحيح nByte){
  إذا( mem0.alarm_حد<=0 ) أرجع;
  sqlite3_mutex_leave(mem0.mutex);
  sqlite3_release_يادگيري(nByte);
  sqlite3_mutex_enter(mem0.mutex);
}

#جيڪڏهن_وصف_ٿيل SQLITE_DEBUG
/*
** This routine is called whenever an out-of-memory condition is seen,
** It's only purpose to to serve as a breakpoint for gdb or similar
** code debuggers when working on out-of-memory conditions, for example
** caused by PRAGMA hard_heap_limit=N.
*/
ساكن SQLITE_NOINLINE فراغ test_oom_breakpoint(u64 n){
  ساكن u64 nOomFault = 0;
  nOomFault += n;
  /* The assert() is never reached in a human lifetime.  It  is here mostly
  ** to prevent code optimizers from optimizing out this function. */
  أكد( (nOomFault>>32) < 0xffffffff );
}
#وإلا
# define test_oom_breakpoint(X)   /* No-op for production builds */
#جيڪڏهن_پورو

/*
** Do a memory allocation with statistics and alarms.  Assume the
** lock is already held.
*/
ساكن فراغ mallocWithAlarm(صحيح n, فراغ **pp){
  فراغ *p;
  صحيح n_ڀريل;
  أكد( sqlite3_mutex_held(mem0.mutex) );
  أكد( n>0 );

  /* In Firefox (circa 2017-02-08), xRoundup() is remapped to an internal
  ** implementation of malloc_good_size(), which must be called in debug
  ** mode and specifically when the DMD "Dark Matter Detector" is enabled
  ** or else a crash results.  Hence, do not attempt to optimize out the
  ** following xRoundup() call. */
  n_ڀريل = sqlite3GlobalConfig.m.xRoundup(n);

  sqlite3_حالت_Highwater(SQLITE_حالت_MALLOC_سائيز, n);
  إذا( mem0.alarm_حد>0 ){
    sqlite3_int64 n_استعمال_ٿيل = sqlite3_حالت_قدر(SQLITE_حالت_يادگيري_استعمال_ٿيل);
    إذا( n_استعمال_ٿيل >= mem0.alarm_حد - n_ڀريل ){
      AtomicStore(&mem0.nearly_ڀريل, 1);
      sqlite3MallocAlarm(n_ڀريل);
      إذا( mem0.hard_حد_بندي ){
        n_استعمال_ٿيل = sqlite3_حالت_قدر(SQLITE_حالت_يادگيري_استعمال_ٿيل);
        إذا( n_استعمال_ٿيل >= mem0.hard_حد_بندي - n_ڀريل ){
          test_oom_breakpoint(1);
          *pp = 0;
          أرجع;
        }
      }
    }وإلا{
      AtomicStore(&mem0.nearly_ڀريل, 0);
    }
  }
  p = sqlite3GlobalConfig.m.xMalloc(n_ڀريل);
#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_يادگيري_MANAGEMENT
  إذا( p==0 && mem0.alarm_حد>0 ){
    sqlite3MallocAlarm(n_ڀريل);
    p = sqlite3GlobalConfig.m.xMalloc(n_ڀريل);
  }
#جيڪڏهن_پورو
  إذا( p ){
    n_ڀريل = sqlite3_Malloc_سائيز(p);
    sqlite3_حالت_مٿي(SQLITE_حالت_يادگيري_استعمال_ٿيل, n_ڀريل);
    sqlite3_حالت_مٿي(SQLITE_حالت_MALLOC_ڳڻپ, 1);
  }
  *pp = p;
}

/*
** Allocate memory.  This routine is like sqlite3_malloc() except that it
** assumes the memory subsystem has already been initialized.
*/
فراغ *sqlite3Malloc(u64 n){
  فراغ *p;
  إذا( n==0 || n>SQLITE_MAX_ورهاست_سائيز ){
    p = 0;
  }وإلا إذا( sqlite3GlobalConfig.bMemstat ){
    sqlite3_mutex_enter(mem0.mutex);
    mallocWithAlarm((صحيح)n, &p);
    sqlite3_mutex_leave(mem0.mutex);
  }وإلا{
    p = sqlite3GlobalConfig.m.xMalloc((صحيح)n);
  }
  أكد( EIGHT_BYTE_ALIGNMENT(p) );  /* IMP: R-11148-40995 */
  أرجع p;
}

/*
** This version of the memory allocation is for use by the application.
** First make sure the memory subsystem is initialized, then do the
** allocation.
*/
فراغ *sqlite3_malloc(صحيح n){
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_AUTOINIT
  إذا( sqlite3_initialize() ) أرجع 0;
#جيڪڏهن_پورو
  أرجع n<=0 ? 0 : sqlite3Malloc(n);
}
فراغ *sqlite3_malloc64(sqlite3_uint64 n){
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_AUTOINIT
  إذا( sqlite3_initialize() ) أرجع 0;
#جيڪڏهن_پورو
  أرجع sqlite3Malloc(n);
}

/*
** TRUE if p is a lookaside memory allocation from db
*/
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_LOOKASIDE
ساكن صحيح isLookaside(sqlite3 *db, ثابت فراغ *p){
  أرجع SQLITE_WITHIN(p, db->lookaside.p_شروعات, db->lookaside.p_True_پڇاڙي);
}
#وإلا
#وصف_ڪريو isLookaside(A,B) 0
#جيڪڏهن_پورو

/*
** Return the size of a memory allocation previously obtained from
** sqlite3Malloc() or sqlite3_malloc().
*/
صحيح sqlite3_Malloc_سائيز(ثابت فراغ *p){
  أكد( sqlite3MemdebugHasType(p, MEMTYPE_ڍير) );
  أرجع sqlite3GlobalConfig.m.x_سائيز((فراغ*)p);
}
ساكن صحيح lookaside_Malloc_سائيز(sqlite3 *db, ثابت فراغ *p){
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_TWOSIZE_LOOKASIDE    
  أرجع p<db->lookaside.p_وچ ? db->lookaside.szTrue : LOOKASIDE_SMALL;
#وإلا
  أرجع db->lookaside.szTrue;
#جيڪڏهن_پورو  
}
صحيح sqlite3_Db_Malloc_سائيز(sqlite3 *db, ثابت فراغ *p){
  أكد( p!=0 );
#جيڪڏهن_وصف_ٿيل SQLITE_DEBUG
  إذا( db==0 ){
    أكد( sqlite3MemdebugNoType(p, (u8)~MEMTYPE_ڍير) );
    أكد( sqlite3MemdebugHasType(p, MEMTYPE_ڍير) );
  }وإلا إذا( !isLookaside(db,p) ){
    أكد( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_ڍير)) );
    أكد( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_ڍير)) );
  }
#جيڪڏهن_پورو
  إذا( db ){
    إذا( ((uptr)p)<(uptr)(db->lookaside.p_True_پڇاڙي) ){
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_TWOSIZE_LOOKASIDE
      إذا( ((uptr)p)>=(uptr)(db->lookaside.p_وچ) ){
        أكد( sqlite3_mutex_held(db->mutex) );
        أرجع LOOKASIDE_SMALL;
      }
#جيڪڏهن_پورو
      إذا( ((uptr)p)>=(uptr)(db->lookaside.p_شروعات) ){
        أكد( sqlite3_mutex_held(db->mutex) );
        أرجع db->lookaside.szTrue;
      }
    }
  }
  أرجع sqlite3GlobalConfig.m.x_سائيز((فراغ*)p);
}
sqlite3_uint64 sqlite3_msize(فراغ *p){
  أكد( sqlite3MemdebugNoType(p, (u8)~MEMTYPE_ڍير) );
  أكد( sqlite3MemdebugHasType(p, MEMTYPE_ڍير) );
  أرجع p ? sqlite3GlobalConfig.m.x_سائيز(p) : 0;
}

/*
** Free memory previously obtained from sqlite3Malloc().
*/
فراغ sqlite3_free(فراغ *p){
  إذا( p==0 ) أرجع;  /* IMP: R-49053-54554 */
  أكد( sqlite3MemdebugHasType(p, MEMTYPE_ڍير) );
  أكد( sqlite3MemdebugNoType(p, (u8)~MEMTYPE_ڍير) );
  إذا( sqlite3GlobalConfig.bMemstat ){
    sqlite3_mutex_enter(mem0.mutex);
    sqlite3_حالت_هيٺ_ڏانهن(SQLITE_حالت_يادگيري_استعمال_ٿيل, sqlite3_Malloc_سائيز(p));
    sqlite3_حالت_هيٺ_ڏانهن(SQLITE_حالت_MALLOC_ڳڻپ, 1);
    sqlite3GlobalConfig.m.xFree(p);
    sqlite3_mutex_leave(mem0.mutex);
  }وإلا{
    sqlite3GlobalConfig.m.xFree(p);
  }
}

/*
** Add the size of memory allocation "p" to the count in
** *db->pnBytesFreed.
*/
ساكن SQLITE_NOINLINE فراغ measure_ورهاست_سائيز(sqlite3 *db, فراغ *p){
  *db->pn_بائيٽون_Freed += sqlite3_Db_Malloc_سائيز(db,p);
}

/*
** Free memory that might be associated with a particular database
** connection.  Calling sqlite3DbFree(D,X) for X==0 is a harmless no-op.
** The sqlite3DbFreeNN(D,X) version requires that X be non-NULL.
*/
فراغ sqlite3DbFreeNN(sqlite3 *db, فراغ *p){
  أكد( db==0 || sqlite3_mutex_held(db->mutex) );
  أكد( p!=0 );
  إذا( db ){
    إذا( ((uptr)p)<(uptr)(db->lookaside.p_پڇاڙي) ){
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_TWOSIZE_LOOKASIDE
      إذا( ((uptr)p)>=(uptr)(db->lookaside.p_وچ) ){
        Lookaside_جڳهه *pBuf = (Lookaside_جڳهه*)p;
        أكد( db->pn_بائيٽون_Freed==0 );
#جيڪڏهن_وصف_ٿيل SQLITE_DEBUG
        memset(p, 0xaa, LOOKASIDE_SMALL);  /* Trash freed content */
#جيڪڏهن_پورو
        pBuf->p_ايندڙ = db->lookaside.pSmallFree;
        db->lookaside.pSmallFree = pBuf;
        أرجع;
      }
#جيڪڏهن_پورو /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
      إذا( ((uptr)p)>=(uptr)(db->lookaside.p_شروعات) ){
        Lookaside_جڳهه *pBuf = (Lookaside_جڳهه*)p;
        أكد( db->pn_بائيٽون_Freed==0 );
#جيڪڏهن_وصف_ٿيل SQLITE_DEBUG
        memset(p, 0xaa, db->lookaside.szTrue);  /* Trash freed content */
#جيڪڏهن_پورو
        pBuf->p_ايندڙ = db->lookaside.pFree;
        db->lookaside.pFree = pBuf;
        أرجع;
      }
    }
    إذا( db->pn_بائيٽون_Freed ){
      measure_ورهاست_سائيز(db, p);
      أرجع;
    }
  }
  أكد( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_ڍير)) );
  أكد( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_ڍير)) );
  أكد( db!=0 || sqlite3MemdebugNoType(p, MEMTYPE_LOOKASIDE) );
  sqlite3_Memdebug_سيٽ_Type(p, MEMTYPE_ڍير);
  sqlite3_free(p);
}
فراغ sqlite3DbNNFreeNN(sqlite3 *db, فراغ *p){
  أكد( db!=0 );
  أكد( sqlite3_mutex_held(db->mutex) );
  أكد( p!=0 );
  إذا( ((uptr)p)<(uptr)(db->lookaside.p_پڇاڙي) ){
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_TWOSIZE_LOOKASIDE
    إذا( ((uptr)p)>=(uptr)(db->lookaside.p_وچ) ){
      Lookaside_جڳهه *pBuf = (Lookaside_جڳهه*)p;
      أكد( db->pn_بائيٽون_Freed==0 );
#جيڪڏهن_وصف_ٿيل SQLITE_DEBUG
      memset(p, 0xaa, LOOKASIDE_SMALL);  /* Trash freed content */
#جيڪڏهن_پورو
      pBuf->p_ايندڙ = db->lookaside.pSmallFree;
      db->lookaside.pSmallFree = pBuf;
      أرجع;
    }
#جيڪڏهن_پورو /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
    إذا( ((uptr)p)>=(uptr)(db->lookaside.p_شروعات) ){
      Lookaside_جڳهه *pBuf = (Lookaside_جڳهه*)p;
      أكد( db->pn_بائيٽون_Freed==0 );
#جيڪڏهن_وصف_ٿيل SQLITE_DEBUG
      memset(p, 0xaa, db->lookaside.szTrue);  /* Trash freed content */
#جيڪڏهن_پورو
      pBuf->p_ايندڙ = db->lookaside.pFree;
      db->lookaside.pFree = pBuf;
      أرجع;
    }
  }
  إذا( db->pn_بائيٽون_Freed ){
    measure_ورهاست_سائيز(db, p);
    أرجع;
  }
  أكد( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_ڍير)) );
  أكد( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_ڍير)) );
  sqlite3_Memdebug_سيٽ_Type(p, MEMTYPE_ڍير);
  sqlite3_free(p);
}
فراغ sqlite3DbFree(sqlite3 *db, فراغ *p){
  أكد( db==0 || sqlite3_mutex_held(db->mutex) );
  إذا( p ) sqlite3DbFreeNN(db, p);
}

/*
** Change the size of an existing memory allocation
*/
فراغ *sqlite3Realloc(فراغ *pOld, u64 n_بائيٽون){
  صحيح nOld, nNew, nDiff;
  فراغ *pNew;
  أكد( sqlite3MemdebugHasType(pOld, MEMTYPE_ڍير) );
  أكد( sqlite3MemdebugNoType(pOld, (u8)~MEMTYPE_ڍير) );
  إذا( pOld==0 ){
    أرجع sqlite3Malloc(n_بائيٽون); /* IMP: R-04300-56712 */
  }
  إذا( n_بائيٽون==0 ){
    sqlite3_free(pOld); /* IMP: R-26507-47431 */
    أرجع 0;
  }
  إذا( n_بائيٽون>SQLITE_MAX_ورهاست_سائيز ){
    أرجع 0;
  }
  nOld = sqlite3_Malloc_سائيز(pOld);
  /* IMPLEMENTATION-OF: R-46199-30249 SQLite guarantees that the second
  ** argument to xRealloc is always a value returned by a prior call to
  ** xRoundup. */
  nNew = sqlite3GlobalConfig.m.xRoundup((صحيح)n_بائيٽون);
  إذا( nOld==nNew ){
    pNew = pOld;
  }وإلا إذا( sqlite3GlobalConfig.bMemstat ){
    sqlite3_int64 n_استعمال_ٿيل;
    sqlite3_mutex_enter(mem0.mutex);
    sqlite3_حالت_Highwater(SQLITE_حالت_MALLOC_سائيز, (صحيح)n_بائيٽون);
    nDiff = nNew - nOld;
    إذا( nDiff>0 && (n_استعمال_ٿيل = sqlite3_حالت_قدر(SQLITE_حالت_يادگيري_استعمال_ٿيل)) >= 
          mem0.alarm_حد-nDiff ){
      sqlite3MallocAlarm(nDiff);
      إذا( mem0.hard_حد_بندي>0 && n_استعمال_ٿيل >= mem0.hard_حد_بندي - nDiff ){
        sqlite3_mutex_leave(mem0.mutex);
        test_oom_breakpoint(1);
        أرجع 0;
      }
    }
    pNew = sqlite3GlobalConfig.m.xRealloc(pOld, nNew);
#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_يادگيري_MANAGEMENT
    إذا( pNew==0 && mem0.alarm_حد>0 ){
      sqlite3MallocAlarm((صحيح)n_بائيٽون);
      pNew = sqlite3GlobalConfig.m.xRealloc(pOld, nNew);
    }
#جيڪڏهن_پورو
    إذا( pNew ){
      nNew = sqlite3_Malloc_سائيز(pNew);
      sqlite3_حالت_مٿي(SQLITE_حالت_يادگيري_استعمال_ٿيل, nNew-nOld);
    }
    sqlite3_mutex_leave(mem0.mutex);
  }وإلا{
    pNew = sqlite3GlobalConfig.m.xRealloc(pOld, nNew);
  }
  أكد( EIGHT_BYTE_ALIGNMENT(pNew) ); /* IMP: R-11148-40995 */
  أرجع pNew;
}

/*
** The public interface to sqlite3Realloc.  Make sure that the memory
** subsystem is initialized prior to invoking sqliteRealloc.
*/
فراغ *sqlite3_realloc(فراغ *pOld, صحيح n){
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_AUTOINIT
  إذا( sqlite3_initialize() ) أرجع 0;
#جيڪڏهن_پورو
  إذا( n<0 ) n = 0;  /* IMP: R-26507-47431 */
  أرجع sqlite3Realloc(pOld, n);
}
فراغ *sqlite3_realloc64(فراغ *pOld, sqlite3_uint64 n){
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_AUTOINIT
  إذا( sqlite3_initialize() ) أرجع 0;
#جيڪڏهن_پورو
  أرجع sqlite3Realloc(pOld, n);
}


/*
** Allocate and zero memory.
*/ 
فراغ *sqlite3MallocZero(u64 n){
  فراغ *p = sqlite3Malloc(n);
  إذا( p ){
    memset(p, 0, (size_t)n);
  }
  أرجع p;
}

/*
** Allocate and zero memory.  If the allocation fails, make
** the mallocFailed flag in the connection pointer.
*/
فراغ *sqlite3DbMallocZero(sqlite3 *db, u64 n){
  فراغ *p;
  testcase( db==0 );
  p = sqlite3DbMallocRaw(db, n);
  إذا( p ) memset(p, 0, (size_t)n);
  أرجع p;
}


/* Finish the work of sqlite3DbMallocRawNN for the unusual and
** slower case when the allocation cannot be fulfilled using lookaside.
*/
ساكن SQLITE_NOINLINE فراغ *db_Malloc_Raw_پڄاڻي(sqlite3 *db, u64 n){
  فراغ *p;
  أكد( db!=0 );
  p = sqlite3Malloc(n);
  إذا( !p ) sqlite3OomFault(db);
  sqlite3_Memdebug_سيٽ_Type(p, 
         (db->lookaside.bDisable==0) ? MEMTYPE_LOOKASIDE : MEMTYPE_ڍير);
  أرجع p;
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
فراغ *sqlite3DbMallocRaw(sqlite3 *db, u64 n){
  فراغ *p;
  إذا( db ) أرجع sqlite3DbMallocRawNN(db, n);
  p = sqlite3Malloc(n);
  sqlite3_Memdebug_سيٽ_Type(p, MEMTYPE_ڍير);
  أرجع p;
}
فراغ *sqlite3DbMallocRawNN(sqlite3 *db, u64 n){
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_LOOKASIDE
  Lookaside_جڳهه *pBuf;
  أكد( db!=0 );
  أكد( sqlite3_mutex_held(db->mutex) );
  أكد( db->pn_بائيٽون_Freed==0 );
  إذا( n>db->lookaside.sz ){
    إذا( !db->lookaside.bDisable ){
      db->lookaside.anStat[1]++;      
    }وإلا إذا( db->mallocFailed ){
      أرجع 0;
    }
    أرجع db_Malloc_Raw_پڄاڻي(db, n);
  }
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_TWOSIZE_LOOKASIDE
  إذا( n<=LOOKASIDE_SMALL ){
    إذا( (pBuf = db->lookaside.pSmallFree)!=0 ){
      db->lookaside.pSmallFree = pBuf->p_ايندڙ;
      db->lookaside.anStat[0]++;
      أرجع (فراغ*)pBuf;
    }وإلا إذا( (pBuf = db->lookaside.pSmallInit)!=0 ){
      db->lookaside.pSmallInit = pBuf->p_ايندڙ;
      db->lookaside.anStat[0]++;
      أرجع (فراغ*)pBuf;
    }
  }
#جيڪڏهن_پورو
  إذا( (pBuf = db->lookaside.pFree)!=0 ){
    db->lookaside.pFree = pBuf->p_ايندڙ;
    db->lookaside.anStat[0]++;
    أرجع (فراغ*)pBuf;
  }وإلا إذا( (pBuf = db->lookaside.pInit)!=0 ){
    db->lookaside.pInit = pBuf->p_ايندڙ;
    db->lookaside.anStat[0]++;
    أرجع (فراغ*)pBuf;
  }وإلا{
    db->lookaside.anStat[2]++;
  }
#وإلا
  أكد( db!=0 );
  أكد( sqlite3_mutex_held(db->mutex) );
  أكد( db->pn_بائيٽون_Freed==0 );
  إذا( db->mallocFailed ){
    أرجع 0;
  }
#جيڪڏهن_پورو
  أرجع db_Malloc_Raw_پڄاڻي(db, n);
}

/* Forward declaration */
ساكن SQLITE_NOINLINE فراغ *db_Realloc_پڄاڻي(sqlite3 *db, فراغ *p, u64 n);

/*
** Resize the block of memory pointed to by p to n bytes. If the
** resize fails, set the mallocFailed flag in the connection object.
*/
فراغ *sqlite3DbRealloc(sqlite3 *db, فراغ *p, u64 n){
  أكد( db!=0 );
  إذا( p==0 ) أرجع sqlite3DbMallocRawNN(db, n);
  أكد( sqlite3_mutex_held(db->mutex) );
  إذا( ((uptr)p)<(uptr)db->lookaside.p_پڇاڙي ){
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_TWOSIZE_LOOKASIDE
    إذا( ((uptr)p)>=(uptr)db->lookaside.p_وچ ){
      إذا( n<=LOOKASIDE_SMALL ) أرجع p;
    }وإلا
#جيڪڏهن_پورو
    إذا( ((uptr)p)>=(uptr)db->lookaside.p_شروعات ){
      إذا( n<=db->lookaside.szTrue ) أرجع p;
    }
  }
  أرجع db_Realloc_پڄاڻي(db, p, n);
}
ساكن SQLITE_NOINLINE فراغ *db_Realloc_پڄاڻي(sqlite3 *db, فراغ *p, u64 n){
  فراغ *pNew = 0;
  أكد( db!=0 );
  أكد( p!=0 );
  إذا( db->mallocFailed==0 ){
    إذا( isLookaside(db, p) ){
      pNew = sqlite3DbMallocRawNN(db, n);
      إذا( pNew ){
        يادگيري_نقل(pNew, p, lookaside_Malloc_سائيز(db, p));
        sqlite3DbFree(db, p);
      }
    }وإلا{
      أكد( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_ڍير)) );
      أكد( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_ڍير)) );
      sqlite3_Memdebug_سيٽ_Type(p, MEMTYPE_ڍير);
      pNew = sqlite3Realloc(p, n);
      إذا( !pNew ){
        sqlite3OomFault(db);
      }
      sqlite3_Memdebug_سيٽ_Type(pNew,
            (db->lookaside.bDisable==0 ? MEMTYPE_LOOKASIDE : MEMTYPE_ڍير));
    }
  }
  أرجع pNew;
}

/*
** Attempt to reallocate p.  If the reallocation fails, then free p
** and set the mallocFailed flag in the database connection.
*/
فراغ *sqlite3DbReallocOrFree(sqlite3 *db, فراغ *p, u64 n){
  فراغ *pNew;
  pNew = sqlite3DbRealloc(db, p, n);
  إذا( !pNew ){
    sqlite3DbFree(db, p);
  }
  أرجع pNew;
}

/*
** Make a copy of a string in memory obtained from sqliteMalloc(). These 
** functions call sqlite3MallocRaw() directly instead of sqliteMalloc(). This
** is because when memory debugging is turned on, these two functions are 
** called via macros that record the current file and line number in the
** ThreadData structure.
*/
حرف *sqlite3DbStrDup(sqlite3 *db, ثابت حرف *z){
  حرف *zNew;
  size_t n;
  إذا( z==0 ){
    أرجع 0;
  }
  n = لڙي_ڊيگهه(z) + 1;
  zNew = sqlite3DbMallocRaw(db, n);
  إذا( zNew ){
    يادگيري_نقل(zNew, z, n);
  }
  أرجع zNew;
}
حرف *sqlite3DbStrNDup(sqlite3 *db, ثابت حرف *z, u64 n){
  حرف *zNew;
  أكد( db!=0 );
  أكد( z!=0 || n==0 );
  أكد( (n&0x7fffffff)==n );
  zNew = z ? sqlite3DbMallocRawNN(db, n+1) : 0;
  إذا( zNew ){
    يادگيري_نقل(zNew, z, (size_t)n);
    zNew[n] = 0;
  }
  أرجع zNew;
}

/*
** The text between zStart and zEnd represents a phrase within a larger
** SQL statement.  Make a copy of this phrase in space obtained form
** sqlite3DbMalloc().  Omit leading and trailing whitespace.
*/
حرف *sqlite3_Db_وسعت_Dup(sqlite3 *db, ثابت حرف *z_شروعات, ثابت حرف *z_پڇاڙي){
  صحيح n;
#جيڪڏهن_وصف_ٿيل SQLITE_DEBUG
  /* Because of the way the parser works, the span is guaranteed to contain
  ** at least one non-space character */
  لكل(n=0; sqlite3Isspace(z_شروعات[n]); n++){ أكد( &z_شروعات[n]<z_پڇاڙي ); }
#جيڪڏهن_پورو
  طالما( sqlite3Isspace(z_شروعات[0]) ) z_شروعات++;
  n = (صحيح)(z_پڇاڙي - z_شروعات);
  طالما( sqlite3Isspace(z_شروعات[n-1]) ) n--;
  أرجع sqlite3DbStrNDup(db, z_شروعات, n);
}

/*
** Free any prior content in *pz and replace it with a copy of zNew.
*/
فراغ sqlite3_سيٽ_String(حرف **pz, sqlite3 *db, ثابت حرف *zNew){
  حرف *z = sqlite3DbStrDup(db, zNew);
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
فراغ *sqlite3OomFault(sqlite3 *db){
  إذا( db->mallocFailed==0 && db->bBenignMalloc==0 ){
    db->mallocFailed = 1;
    إذا( db->nVdbeExec>0 ){
      AtomicStore(&db->u1.isInterrupted, 1);
    }
    DisableLookaside;
    إذا( db->pParse ){
      Parse *pParse;
      sqlite3_Error_نياپو(db->pParse, "out of memory");
      db->pParse->rc = SQLITE_NOMEM_BKPT;
      لكل(pParse=db->pParse->pOuterParse; pParse; pParse = pParse->pOuterParse){
        pParse->nErr++;
        pParse->rc = SQLITE_NOMEM;
      } 
    }
  }
  أرجع 0;
}

/*
** This routine reactivates the memory allocator and clears the
** db->mallocFailed flag as necessary.
**
** The memory allocator is not restarted if there are running
** VDBEs.
*/
فراغ sqlite3OomClear(sqlite3 *db){
  إذا( db->mallocFailed && db->nVdbeExec==0 ){
    db->mallocFailed = 0;
    AtomicStore(&db->u1.isInterrupted, 0);
    أكد( db->lookaside.bDisable>0 );
    EnableLookaside;
  }
}

/*
** Take actions at the end of an API call to deal with error codes.
*/
ساكن SQLITE_NOINLINE صحيح apiHandleError(sqlite3 *db, صحيح rc){
  إذا( db->mallocFailed || rc==SQLITE_IOERR_NOMEM ){
    sqlite3OomClear(db);
    sqlite3Error(db, SQLITE_NOMEM);
    أرجع SQLITE_NOMEM_BKPT;
  }
  أرجع rc & db->err_نقاب;
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
صحيح sqlite3ApiExit(sqlite3* db, صحيح rc){
  /* If the db handle must hold the connection handle mutex here.
  ** Otherwise the read (and possible write) of db->mallocFailed 
  ** is unsafe, as is the call to sqlite3Error().
  */
  أكد( db!=0 );
  أكد( sqlite3_mutex_held(db->mutex) );
  إذا( db->mallocFailed || rc ){
    أرجع apiHandleError(db, rc);
  }
  أرجع 0;
}
