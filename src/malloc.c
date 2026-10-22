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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <stdarg.h>

/*
** Attempt to release up to n bytes of non-essential memory currently
** held by SQLite. An example of non-essential memory is memory used to
** cache database pages that are not currently in use.
*/
ਪੂਰਨ sqlite3_release_ਸਮ੍ਰਿਤੀ(ਪੂਰਨ n){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_ਸਮ੍ਰਿਤੀ_MANAGEMENT
  ਵਾਪਸ sqlite3_Pcache_Release_ਸਮ੍ਰਿਤੀ(n);
#ਨਹੀਂਤਾਂ
  /* IMPLEMENTATION-OF: R-34391-24921 The sqlite3_release_memory() routine
  ** is a no-op returning zero if SQLite is not compiled with
  ** SQLITE_ENABLE_MEMORY_MANAGEMENT. */
  ਅਣਵਰਤਿਆ_PARAMETER(n);
  ਵਾਪਸ 0;
#ਜੇ_ਅੰਤ
}

/*
** Default value of the hard heap limit.  0 means "no limit".
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_MAX_ਸਮ੍ਰਿਤੀ
# define SQLITE_MAX_ਸਮ੍ਰਿਤੀ 0
#ਜੇ_ਅੰਤ

/*
** State information local to the memory allocation subsystem.
*/
ਸਥਿਰ SQLITE_WSD ਬਣਤਰ Mem0Global {
  sqlite3_mutex *mutex;         /* Mutex to serialize access */
  sqlite3_int64 alarm_ਹੱਦ; /* The soft heap limit */
  sqlite3_int64 hard_ਹੱਦਬੰਦੀ;      /* The hard upper bound on memory */

  /*
  ** True if heap is nearly "full" where "full" is defined by the
  ** sqlite3_soft_heap_limit() setting.
  */
  ਪੂਰਨ nearly_ਭਰਿਆ;
} mem0 = { 0, SQLITE_MAX_ਸਮ੍ਰਿਤੀ, SQLITE_MAX_ਸਮ੍ਰਿਤੀ, 0 };

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ mem0 GLOBAL(ਬਣਤਰ Mem0Global, mem0)

/*
** Return the memory allocator mutex. sqlite3_status() needs it.
*/
sqlite3_mutex *sqlite3MallocMutex(ਖਾਲੀ){
  ਵਾਪਸ mem0.mutex;
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_DEPRECATED
/*
** Deprecated external interface.  It used to set an alarm callback
** that was invoked when memory usage grew too large.  Now it is a
** no-op.
*/
ਪੂਰਨ sqlite3_ਸਮ੍ਰਿਤੀ_alarm(
  ਖਾਲੀ(*xCallback)(ਖਾਲੀ *pArg, sqlite3_int64 ਵਰਤਿਆ,ਪੂਰਨ N),
  ਖਾਲੀ *pArg,
  sqlite3_int64 i_ਹੱਦ
){
  (ਖਾਲੀ)xCallback;
  (ਖਾਲੀ)pArg;
  (ਖਾਲੀ)i_ਹੱਦ;
  ਵਾਪਸ SQLITE_OK;
}
#ਜੇ_ਅੰਤ

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
sqlite3_int64 sqlite3_soft_ਢੇਰ_limit64(sqlite3_int64 n){
  sqlite3_int64 ਪੂਰਵ_ਹੱਦਬੰਦੀ;
  sqlite3_int64 excess;
  sqlite3_int64 n_ਵਰਤਿਆ;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOINIT
  ਪੂਰਨ rc = sqlite3_initialize();
  ਜੇ( rc ) ਵਾਪਸ -1;
#ਜੇ_ਅੰਤ
  sqlite3_mutex_enter(mem0.mutex);
  ਪੂਰਵ_ਹੱਦਬੰਦੀ = mem0.alarm_ਹੱਦ;
  ਜੇ( n<0 ){
    sqlite3_mutex_leave(mem0.mutex);
    ਵਾਪਸ ਪੂਰਵ_ਹੱਦਬੰਦੀ;
  }
  ਜੇ( mem0.hard_ਹੱਦਬੰਦੀ>0 && (n>mem0.hard_ਹੱਦਬੰਦੀ || n==0) ){
    n = mem0.hard_ਹੱਦਬੰਦੀ;
  }
  mem0.alarm_ਹੱਦ = n;
  n_ਵਰਤਿਆ = sqlite3_ਸਥਿਤੀ_ਮੁੱਲ(SQLITE_ਸਥਿਤੀ_ਸਮ੍ਰਿਤੀ_ਵਰਤਿਆ);
  AtomicStore(&mem0.nearly_ਭਰਿਆ, n>0 && n<=n_ਵਰਤਿਆ);
  sqlite3_mutex_leave(mem0.mutex);
  excess = sqlite3_ਸਮ੍ਰਿਤੀ_ਵਰਤਿਆ() - n;
  ਜੇ( excess>0 ) sqlite3_release_ਸਮ੍ਰਿਤੀ((ਪੂਰਨ)(excess & 0x7fffffff));
  ਵਾਪਸ ਪੂਰਵ_ਹੱਦਬੰਦੀ;
}
ਖਾਲੀ sqlite3_soft_ਢੇਰ_ਹੱਦਬੰਦੀ(ਪੂਰਨ n){
  ਜੇ( n<0 ) n = 0;
  sqlite3_soft_ਢੇਰ_limit64(n);
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
sqlite3_int64 sqlite3_hard_ਢੇਰ_limit64(sqlite3_int64 n){
  sqlite3_int64 ਪੂਰਵ_ਹੱਦਬੰਦੀ;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOINIT
  ਪੂਰਨ rc = sqlite3_initialize();
  ਜੇ( rc ) ਵਾਪਸ -1;
#ਜੇ_ਅੰਤ
  sqlite3_mutex_enter(mem0.mutex);
  ਪੂਰਵ_ਹੱਦਬੰਦੀ = mem0.hard_ਹੱਦਬੰਦੀ;
  ਜੇ( n>=0 ){
    mem0.hard_ਹੱਦਬੰਦੀ = n;
    ਜੇ( n<mem0.alarm_ਹੱਦ || mem0.alarm_ਹੱਦ==0 ){
      mem0.alarm_ਹੱਦ = n;
    }
  }
  sqlite3_mutex_leave(mem0.mutex);
  ਵਾਪਸ ਪੂਰਵ_ਹੱਦਬੰਦੀ;
}


/*
** Initialize the memory allocation subsystem.
*/
ਪੂਰਨ sqlite3MallocInit(ਖਾਲੀ){
  ਪੂਰਨ rc;
  ਜੇ( sqlite3GlobalConfig.m.xMalloc==0 ){
    sqlite3_Mem_ਸੈੱਟ_Default();
  }
  mem0.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM);
  ਜੇ( sqlite3GlobalConfig.p_ਪੰਨਾ==0 || sqlite3GlobalConfig.sz_ਪੰਨਾ<512
      || sqlite3GlobalConfig.n_ਪੰਨਾ<=0 ){
    sqlite3GlobalConfig.p_ਪੰਨਾ = 0;
    sqlite3GlobalConfig.sz_ਪੰਨਾ = 0;
  }
  rc = sqlite3GlobalConfig.m.xInit(sqlite3GlobalConfig.m.p_App_ਅੰਕੜੇ);
  ਜੇ( rc!=SQLITE_OK ) memset(&mem0, 0, ਆਕਾਰ(mem0));
  ਵਾਪਸ rc;
}

/*
** Return true if the heap is currently under memory pressure - in other
** words if the amount of heap used is close to the limit set by
** sqlite3_soft_heap_limit().
*/
ਪੂਰਨ sqlite3_ਢੇਰ_Nearly_ਭਰਿਆ(ਖਾਲੀ){
  ਵਾਪਸ Atomic_ਲੱਦੋ(&mem0.nearly_ਭਰਿਆ);
}

/*
** Deinitialize the memory allocation subsystem.
*/
ਖਾਲੀ sqlite3_Malloc_ਸਿਰਾ(ਖਾਲੀ){
  ਜੇ( sqlite3GlobalConfig.m.xShutdown ){
    sqlite3GlobalConfig.m.xShutdown(sqlite3GlobalConfig.m.p_App_ਅੰਕੜੇ);
  }
  memset(&mem0, 0, ਆਕਾਰ(mem0));
}

/*
** Return the amount of memory currently checked out.
*/
sqlite3_int64 sqlite3_ਸਮ੍ਰਿਤੀ_ਵਰਤਿਆ(ਖਾਲੀ){
  sqlite3_int64 res, mx;
  sqlite3_status64(SQLITE_ਸਥਿਤੀ_ਸਮ੍ਰਿਤੀ_ਵਰਤਿਆ, &res, &mx, 0);
  ਵਾਪਸ res;
}

/*
** Return the maximum amount of memory that has ever been
** checked out since either the beginning of this process
** or since the most recent reset.
*/
sqlite3_int64 sqlite3_ਸਮ੍ਰਿਤੀ_highwater(ਪੂਰਨ resetFlag){
  sqlite3_int64 res, mx;
  sqlite3_status64(SQLITE_ਸਥਿਤੀ_ਸਮ੍ਰਿਤੀ_ਵਰਤਿਆ, &res, &mx, resetFlag);
  ਵਾਪਸ mx;
}

/*
** Trigger the alarm 
*/
ਸਥਿਰ ਖਾਲੀ sqlite3MallocAlarm(ਪੂਰਨ nByte){
  ਜੇ( mem0.alarm_ਹੱਦ<=0 ) ਵਾਪਸ;
  sqlite3_mutex_leave(mem0.mutex);
  sqlite3_release_ਸਮ੍ਰਿਤੀ(nByte);
  sqlite3_mutex_enter(mem0.mutex);
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
/*
** This routine is called whenever an out-of-memory condition is seen,
** It's only purpose to to serve as a breakpoint for gdb or similar
** code debuggers when working on out-of-memory conditions, for example
** caused by PRAGMA hard_heap_limit=N.
*/
ਸਥਿਰ SQLITE_NOINLINE ਖਾਲੀ test_oom_breakpoint(u64 n){
  ਸਥਿਰ u64 nOomFault = 0;
  nOomFault += n;
  /* The assert() is never reached in a human lifetime.  It  is here mostly
  ** to prevent code optimizers from optimizing out this function. */
  ਪ੍ਰਤਿਜ੍ਞਾ( (nOomFault>>32) < 0xffffffff );
}
#ਨਹੀਂਤਾਂ
# define test_oom_breakpoint(X)   /* No-op for production builds */
#ਜੇ_ਅੰਤ

/*
** Do a memory allocation with statistics and alarms.  Assume the
** lock is already held.
*/
ਸਥਿਰ ਖਾਲੀ mallocWithAlarm(ਪੂਰਨ n, ਖਾਲੀ **pp){
  ਖਾਲੀ *p;
  ਪੂਰਨ n_ਭਰਿਆ;
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(mem0.mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( n>0 );

  /* In Firefox (circa 2017-02-08), xRoundup() is remapped to an internal
  ** implementation of malloc_good_size(), which must be called in debug
  ** mode and specifically when the DMD "Dark Matter Detector" is enabled
  ** or else a crash results.  Hence, do not attempt to optimize out the
  ** following xRoundup() call. */
  n_ਭਰਿਆ = sqlite3GlobalConfig.m.xRoundup(n);

  sqlite3_ਸਥਿਤੀ_Highwater(SQLITE_ਸਥਿਤੀ_MALLOC_ਮਾਪ, n);
  ਜੇ( mem0.alarm_ਹੱਦ>0 ){
    sqlite3_int64 n_ਵਰਤਿਆ = sqlite3_ਸਥਿਤੀ_ਮੁੱਲ(SQLITE_ਸਥਿਤੀ_ਸਮ੍ਰਿਤੀ_ਵਰਤਿਆ);
    ਜੇ( n_ਵਰਤਿਆ >= mem0.alarm_ਹੱਦ - n_ਭਰਿਆ ){
      AtomicStore(&mem0.nearly_ਭਰਿਆ, 1);
      sqlite3MallocAlarm(n_ਭਰਿਆ);
      ਜੇ( mem0.hard_ਹੱਦਬੰਦੀ ){
        n_ਵਰਤਿਆ = sqlite3_ਸਥਿਤੀ_ਮੁੱਲ(SQLITE_ਸਥਿਤੀ_ਸਮ੍ਰਿਤੀ_ਵਰਤਿਆ);
        ਜੇ( n_ਵਰਤਿਆ >= mem0.hard_ਹੱਦਬੰਦੀ - n_ਭਰਿਆ ){
          test_oom_breakpoint(1);
          *pp = 0;
          ਵਾਪਸ;
        }
      }
    }ਨਹੀਂਤਾਂ{
      AtomicStore(&mem0.nearly_ਭਰਿਆ, 0);
    }
  }
  p = sqlite3GlobalConfig.m.xMalloc(n_ਭਰਿਆ);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_ਸਮ੍ਰਿਤੀ_MANAGEMENT
  ਜੇ( p==0 && mem0.alarm_ਹੱਦ>0 ){
    sqlite3MallocAlarm(n_ਭਰਿਆ);
    p = sqlite3GlobalConfig.m.xMalloc(n_ਭਰਿਆ);
  }
#ਜੇ_ਅੰਤ
  ਜੇ( p ){
    n_ਭਰਿਆ = sqlite3_Malloc_ਮਾਪ(p);
    sqlite3_ਸਥਿਤੀ_ਉੱਪਰ(SQLITE_ਸਥਿਤੀ_ਸਮ੍ਰਿਤੀ_ਵਰਤਿਆ, n_ਭਰਿਆ);
    sqlite3_ਸਥਿਤੀ_ਉੱਪਰ(SQLITE_ਸਥਿਤੀ_MALLOC_ਗਿਣਤੀ, 1);
  }
  *pp = p;
}

/*
** Allocate memory.  This routine is like sqlite3_malloc() except that it
** assumes the memory subsystem has already been initialized.
*/
ਖਾਲੀ *sqlite3Malloc(u64 n){
  ਖਾਲੀ *p;
  ਜੇ( n==0 || n>SQLITE_MAX_ਵੰਡ_ਮਾਪ ){
    p = 0;
  }ਨਹੀਂਤਾਂ ਜੇ( sqlite3GlobalConfig.bMemstat ){
    sqlite3_mutex_enter(mem0.mutex);
    mallocWithAlarm((ਪੂਰਨ)n, &p);
    sqlite3_mutex_leave(mem0.mutex);
  }ਨਹੀਂਤਾਂ{
    p = sqlite3GlobalConfig.m.xMalloc((ਪੂਰਨ)n);
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( EIGHT_BYTE_ALIGNMENT(p) );  /* IMP: R-11148-40995 */
  ਵਾਪਸ p;
}

/*
** This version of the memory allocation is for use by the application.
** First make sure the memory subsystem is initialized, then do the
** allocation.
*/
ਖਾਲੀ *sqlite3_malloc(ਪੂਰਨ n){
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOINIT
  ਜੇ( sqlite3_initialize() ) ਵਾਪਸ 0;
#ਜੇ_ਅੰਤ
  ਵਾਪਸ n<=0 ? 0 : sqlite3Malloc(n);
}
ਖਾਲੀ *sqlite3_malloc64(sqlite3_uint64 n){
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOINIT
  ਜੇ( sqlite3_initialize() ) ਵਾਪਸ 0;
#ਜੇ_ਅੰਤ
  ਵਾਪਸ sqlite3Malloc(n);
}

/*
** TRUE if p is a lookaside memory allocation from db
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_LOOKASIDE
ਸਥਿਰ ਪੂਰਨ isLookaside(sqlite3 *db, ਅਚਲ ਖਾਲੀ *p){
  ਵਾਪਸ SQLITE_WITHIN(p, db->lookaside.p_ਸ਼ੁਰੂ, db->lookaside.p_True_ਸਿਰਾ);
}
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ isLookaside(A,B) 0
#ਜੇ_ਅੰਤ

/*
** Return the size of a memory allocation previously obtained from
** sqlite3Malloc() or sqlite3_malloc().
*/
ਪੂਰਨ sqlite3_Malloc_ਮਾਪ(ਅਚਲ ਖਾਲੀ *p){
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3MemdebugHasType(p, MEMTYPE_ਢੇਰ) );
  ਵਾਪਸ sqlite3GlobalConfig.m.x_ਮਾਪ((ਖਾਲੀ*)p);
}
ਸਥਿਰ ਪੂਰਨ lookaside_Malloc_ਮਾਪ(sqlite3 *db, ਅਚਲ ਖਾਲੀ *p){
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_TWOSIZE_LOOKASIDE    
  ਵਾਪਸ p<db->lookaside.p_ਵਿਚਕਾਰ ? db->lookaside.szTrue : LOOKASIDE_SMALL;
#ਨਹੀਂਤਾਂ
  ਵਾਪਸ db->lookaside.szTrue;
#ਜੇ_ਅੰਤ  
}
ਪੂਰਨ sqlite3_Db_Malloc_ਮਾਪ(sqlite3 *db, ਅਚਲ ਖਾਲੀ *p){
  ਪ੍ਰਤਿਜ੍ਞਾ( p!=0 );
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  ਜੇ( db==0 ){
    ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3MemdebugNoType(p, (u8)~MEMTYPE_ਢੇਰ) );
    ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3MemdebugHasType(p, MEMTYPE_ਢੇਰ) );
  }ਨਹੀਂਤਾਂ ਜੇ( !isLookaside(db,p) ){
    ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_ਢੇਰ)) );
    ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_ਢੇਰ)) );
  }
#ਜੇ_ਅੰਤ
  ਜੇ( db ){
    ਜੇ( ((uptr)p)<(uptr)(db->lookaside.p_True_ਸਿਰਾ) ){
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_TWOSIZE_LOOKASIDE
      ਜੇ( ((uptr)p)>=(uptr)(db->lookaside.p_ਵਿਚਕਾਰ) ){
        ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(db->mutex) );
        ਵਾਪਸ LOOKASIDE_SMALL;
      }
#ਜੇ_ਅੰਤ
      ਜੇ( ((uptr)p)>=(uptr)(db->lookaside.p_ਸ਼ੁਰੂ) ){
        ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(db->mutex) );
        ਵਾਪਸ db->lookaside.szTrue;
      }
    }
  }
  ਵਾਪਸ sqlite3GlobalConfig.m.x_ਮਾਪ((ਖਾਲੀ*)p);
}
sqlite3_uint64 sqlite3_msize(ਖਾਲੀ *p){
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3MemdebugNoType(p, (u8)~MEMTYPE_ਢੇਰ) );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3MemdebugHasType(p, MEMTYPE_ਢੇਰ) );
  ਵਾਪਸ p ? sqlite3GlobalConfig.m.x_ਮਾਪ(p) : 0;
}

/*
** Free memory previously obtained from sqlite3Malloc().
*/
ਖਾਲੀ sqlite3_free(ਖਾਲੀ *p){
  ਜੇ( p==0 ) ਵਾਪਸ;  /* IMP: R-49053-54554 */
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3MemdebugHasType(p, MEMTYPE_ਢੇਰ) );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3MemdebugNoType(p, (u8)~MEMTYPE_ਢੇਰ) );
  ਜੇ( sqlite3GlobalConfig.bMemstat ){
    sqlite3_mutex_enter(mem0.mutex);
    sqlite3_ਸਥਿਤੀ_ਥੱਲੇ(SQLITE_ਸਥਿਤੀ_ਸਮ੍ਰਿਤੀ_ਵਰਤਿਆ, sqlite3_Malloc_ਮਾਪ(p));
    sqlite3_ਸਥਿਤੀ_ਥੱਲੇ(SQLITE_ਸਥਿਤੀ_MALLOC_ਗਿਣਤੀ, 1);
    sqlite3GlobalConfig.m.xFree(p);
    sqlite3_mutex_leave(mem0.mutex);
  }ਨਹੀਂਤਾਂ{
    sqlite3GlobalConfig.m.xFree(p);
  }
}

/*
** Add the size of memory allocation "p" to the count in
** *db->pnBytesFreed.
*/
ਸਥਿਰ SQLITE_NOINLINE ਖਾਲੀ measure_ਵੰਡ_ਮਾਪ(sqlite3 *db, ਖਾਲੀ *p){
  *db->pn_ਬਾਈਟਾਂ_Freed += sqlite3_Db_Malloc_ਮਾਪ(db,p);
}

/*
** Free memory that might be associated with a particular database
** connection.  Calling sqlite3DbFree(D,X) for X==0 is a harmless no-op.
** The sqlite3DbFreeNN(D,X) version requires that X be non-NULL.
*/
ਖਾਲੀ sqlite3DbFreeNN(sqlite3 *db, ਖਾਲੀ *p){
  ਪ੍ਰਤਿਜ੍ਞਾ( db==0 || sqlite3_mutex_held(db->mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( p!=0 );
  ਜੇ( db ){
    ਜੇ( ((uptr)p)<(uptr)(db->lookaside.p_ਸਿਰਾ) ){
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_TWOSIZE_LOOKASIDE
      ਜੇ( ((uptr)p)>=(uptr)(db->lookaside.p_ਵਿਚਕਾਰ) ){
        Lookaside_ਥਾਂ *pBuf = (Lookaside_ਥਾਂ*)p;
        ਪ੍ਰਤਿਜ੍ਞਾ( db->pn_ਬਾਈਟਾਂ_Freed==0 );
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
        memset(p, 0xaa, LOOKASIDE_SMALL);  /* Trash freed content */
#ਜੇ_ਅੰਤ
        pBuf->p_ਅਗਲਾ = db->lookaside.pSmallFree;
        db->lookaside.pSmallFree = pBuf;
        ਵਾਪਸ;
      }
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
      ਜੇ( ((uptr)p)>=(uptr)(db->lookaside.p_ਸ਼ੁਰੂ) ){
        Lookaside_ਥਾਂ *pBuf = (Lookaside_ਥਾਂ*)p;
        ਪ੍ਰਤਿਜ੍ਞਾ( db->pn_ਬਾਈਟਾਂ_Freed==0 );
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
        memset(p, 0xaa, db->lookaside.szTrue);  /* Trash freed content */
#ਜੇ_ਅੰਤ
        pBuf->p_ਅਗਲਾ = db->lookaside.pFree;
        db->lookaside.pFree = pBuf;
        ਵਾਪਸ;
      }
    }
    ਜੇ( db->pn_ਬਾਈਟਾਂ_Freed ){
      measure_ਵੰਡ_ਮਾਪ(db, p);
      ਵਾਪਸ;
    }
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_ਢੇਰ)) );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_ਢੇਰ)) );
  ਪ੍ਰਤਿਜ੍ਞਾ( db!=0 || sqlite3MemdebugNoType(p, MEMTYPE_LOOKASIDE) );
  sqlite3_Memdebug_ਸੈੱਟ_Type(p, MEMTYPE_ਢੇਰ);
  sqlite3_free(p);
}
ਖਾਲੀ sqlite3DbNNFreeNN(sqlite3 *db, ਖਾਲੀ *p){
  ਪ੍ਰਤਿਜ੍ਞਾ( db!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(db->mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( p!=0 );
  ਜੇ( ((uptr)p)<(uptr)(db->lookaside.p_ਸਿਰਾ) ){
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_TWOSIZE_LOOKASIDE
    ਜੇ( ((uptr)p)>=(uptr)(db->lookaside.p_ਵਿਚਕਾਰ) ){
      Lookaside_ਥਾਂ *pBuf = (Lookaside_ਥਾਂ*)p;
      ਪ੍ਰਤਿਜ੍ਞਾ( db->pn_ਬਾਈਟਾਂ_Freed==0 );
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
      memset(p, 0xaa, LOOKASIDE_SMALL);  /* Trash freed content */
#ਜੇ_ਅੰਤ
      pBuf->p_ਅਗਲਾ = db->lookaside.pSmallFree;
      db->lookaside.pSmallFree = pBuf;
      ਵਾਪਸ;
    }
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
    ਜੇ( ((uptr)p)>=(uptr)(db->lookaside.p_ਸ਼ੁਰੂ) ){
      Lookaside_ਥਾਂ *pBuf = (Lookaside_ਥਾਂ*)p;
      ਪ੍ਰਤਿਜ੍ਞਾ( db->pn_ਬਾਈਟਾਂ_Freed==0 );
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
      memset(p, 0xaa, db->lookaside.szTrue);  /* Trash freed content */
#ਜੇ_ਅੰਤ
      pBuf->p_ਅਗਲਾ = db->lookaside.pFree;
      db->lookaside.pFree = pBuf;
      ਵਾਪਸ;
    }
  }
  ਜੇ( db->pn_ਬਾਈਟਾਂ_Freed ){
    measure_ਵੰਡ_ਮਾਪ(db, p);
    ਵਾਪਸ;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_ਢੇਰ)) );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_ਢੇਰ)) );
  sqlite3_Memdebug_ਸੈੱਟ_Type(p, MEMTYPE_ਢੇਰ);
  sqlite3_free(p);
}
ਖਾਲੀ sqlite3DbFree(sqlite3 *db, ਖਾਲੀ *p){
  ਪ੍ਰਤਿਜ੍ਞਾ( db==0 || sqlite3_mutex_held(db->mutex) );
  ਜੇ( p ) sqlite3DbFreeNN(db, p);
}

/*
** Change the size of an existing memory allocation
*/
ਖਾਲੀ *sqlite3Realloc(ਖਾਲੀ *pOld, u64 n_ਬਾਈਟਾਂ){
  ਪੂਰਨ nOld, nNew, nDiff;
  ਖਾਲੀ *pNew;
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3MemdebugHasType(pOld, MEMTYPE_ਢੇਰ) );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3MemdebugNoType(pOld, (u8)~MEMTYPE_ਢੇਰ) );
  ਜੇ( pOld==0 ){
    ਵਾਪਸ sqlite3Malloc(n_ਬਾਈਟਾਂ); /* IMP: R-04300-56712 */
  }
  ਜੇ( n_ਬਾਈਟਾਂ==0 ){
    sqlite3_free(pOld); /* IMP: R-26507-47431 */
    ਵਾਪਸ 0;
  }
  ਜੇ( n_ਬਾਈਟਾਂ>SQLITE_MAX_ਵੰਡ_ਮਾਪ ){
    ਵਾਪਸ 0;
  }
  nOld = sqlite3_Malloc_ਮਾਪ(pOld);
  /* IMPLEMENTATION-OF: R-46199-30249 SQLite guarantees that the second
  ** argument to xRealloc is always a value returned by a prior call to
  ** xRoundup. */
  nNew = sqlite3GlobalConfig.m.xRoundup((ਪੂਰਨ)n_ਬਾਈਟਾਂ);
  ਜੇ( nOld==nNew ){
    pNew = pOld;
  }ਨਹੀਂਤਾਂ ਜੇ( sqlite3GlobalConfig.bMemstat ){
    sqlite3_int64 n_ਵਰਤਿਆ;
    sqlite3_mutex_enter(mem0.mutex);
    sqlite3_ਸਥਿਤੀ_Highwater(SQLITE_ਸਥਿਤੀ_MALLOC_ਮਾਪ, (ਪੂਰਨ)n_ਬਾਈਟਾਂ);
    nDiff = nNew - nOld;
    ਜੇ( nDiff>0 && (n_ਵਰਤਿਆ = sqlite3_ਸਥਿਤੀ_ਮੁੱਲ(SQLITE_ਸਥਿਤੀ_ਸਮ੍ਰਿਤੀ_ਵਰਤਿਆ)) >= 
          mem0.alarm_ਹੱਦ-nDiff ){
      sqlite3MallocAlarm(nDiff);
      ਜੇ( mem0.hard_ਹੱਦਬੰਦੀ>0 && n_ਵਰਤਿਆ >= mem0.hard_ਹੱਦਬੰਦੀ - nDiff ){
        sqlite3_mutex_leave(mem0.mutex);
        test_oom_breakpoint(1);
        ਵਾਪਸ 0;
      }
    }
    pNew = sqlite3GlobalConfig.m.xRealloc(pOld, nNew);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_ਸਮ੍ਰਿਤੀ_MANAGEMENT
    ਜੇ( pNew==0 && mem0.alarm_ਹੱਦ>0 ){
      sqlite3MallocAlarm((ਪੂਰਨ)n_ਬਾਈਟਾਂ);
      pNew = sqlite3GlobalConfig.m.xRealloc(pOld, nNew);
    }
#ਜੇ_ਅੰਤ
    ਜੇ( pNew ){
      nNew = sqlite3_Malloc_ਮਾਪ(pNew);
      sqlite3_ਸਥਿਤੀ_ਉੱਪਰ(SQLITE_ਸਥਿਤੀ_ਸਮ੍ਰਿਤੀ_ਵਰਤਿਆ, nNew-nOld);
    }
    sqlite3_mutex_leave(mem0.mutex);
  }ਨਹੀਂਤਾਂ{
    pNew = sqlite3GlobalConfig.m.xRealloc(pOld, nNew);
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( EIGHT_BYTE_ALIGNMENT(pNew) ); /* IMP: R-11148-40995 */
  ਵਾਪਸ pNew;
}

/*
** The public interface to sqlite3Realloc.  Make sure that the memory
** subsystem is initialized prior to invoking sqliteRealloc.
*/
ਖਾਲੀ *sqlite3_realloc(ਖਾਲੀ *pOld, ਪੂਰਨ n){
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOINIT
  ਜੇ( sqlite3_initialize() ) ਵਾਪਸ 0;
#ਜੇ_ਅੰਤ
  ਜੇ( n<0 ) n = 0;  /* IMP: R-26507-47431 */
  ਵਾਪਸ sqlite3Realloc(pOld, n);
}
ਖਾਲੀ *sqlite3_realloc64(ਖਾਲੀ *pOld, sqlite3_uint64 n){
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOINIT
  ਜੇ( sqlite3_initialize() ) ਵਾਪਸ 0;
#ਜੇ_ਅੰਤ
  ਵਾਪਸ sqlite3Realloc(pOld, n);
}


/*
** Allocate and zero memory.
*/ 
ਖਾਲੀ *sqlite3MallocZero(u64 n){
  ਖਾਲੀ *p = sqlite3Malloc(n);
  ਜੇ( p ){
    memset(p, 0, (size_t)n);
  }
  ਵਾਪਸ p;
}

/*
** Allocate and zero memory.  If the allocation fails, make
** the mallocFailed flag in the connection pointer.
*/
ਖਾਲੀ *sqlite3DbMallocZero(sqlite3 *db, u64 n){
  ਖਾਲੀ *p;
  testcase( db==0 );
  p = sqlite3DbMallocRaw(db, n);
  ਜੇ( p ) memset(p, 0, (size_t)n);
  ਵਾਪਸ p;
}


/* Finish the work of sqlite3DbMallocRawNN for the unusual and
** slower case when the allocation cannot be fulfilled using lookaside.
*/
ਸਥਿਰ SQLITE_NOINLINE ਖਾਲੀ *db_Malloc_Raw_ਅੰਤ(sqlite3 *db, u64 n){
  ਖਾਲੀ *p;
  ਪ੍ਰਤਿਜ੍ਞਾ( db!=0 );
  p = sqlite3Malloc(n);
  ਜੇ( !p ) sqlite3OomFault(db);
  sqlite3_Memdebug_ਸੈੱਟ_Type(p, 
         (db->lookaside.bDisable==0) ? MEMTYPE_LOOKASIDE : MEMTYPE_ਢੇਰ);
  ਵਾਪਸ p;
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
ਖਾਲੀ *sqlite3DbMallocRaw(sqlite3 *db, u64 n){
  ਖਾਲੀ *p;
  ਜੇ( db ) ਵਾਪਸ sqlite3DbMallocRawNN(db, n);
  p = sqlite3Malloc(n);
  sqlite3_Memdebug_ਸੈੱਟ_Type(p, MEMTYPE_ਢੇਰ);
  ਵਾਪਸ p;
}
ਖਾਲੀ *sqlite3DbMallocRawNN(sqlite3 *db, u64 n){
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_LOOKASIDE
  Lookaside_ਥਾਂ *pBuf;
  ਪ੍ਰਤਿਜ੍ਞਾ( db!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(db->mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( db->pn_ਬਾਈਟਾਂ_Freed==0 );
  ਜੇ( n>db->lookaside.sz ){
    ਜੇ( !db->lookaside.bDisable ){
      db->lookaside.anStat[1]++;      
    }ਨਹੀਂਤਾਂ ਜੇ( db->mallocFailed ){
      ਵਾਪਸ 0;
    }
    ਵਾਪਸ db_Malloc_Raw_ਅੰਤ(db, n);
  }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_TWOSIZE_LOOKASIDE
  ਜੇ( n<=LOOKASIDE_SMALL ){
    ਜੇ( (pBuf = db->lookaside.pSmallFree)!=0 ){
      db->lookaside.pSmallFree = pBuf->p_ਅਗਲਾ;
      db->lookaside.anStat[0]++;
      ਵਾਪਸ (ਖਾਲੀ*)pBuf;
    }ਨਹੀਂਤਾਂ ਜੇ( (pBuf = db->lookaside.pSmallInit)!=0 ){
      db->lookaside.pSmallInit = pBuf->p_ਅਗਲਾ;
      db->lookaside.anStat[0]++;
      ਵਾਪਸ (ਖਾਲੀ*)pBuf;
    }
  }
#ਜੇ_ਅੰਤ
  ਜੇ( (pBuf = db->lookaside.pFree)!=0 ){
    db->lookaside.pFree = pBuf->p_ਅਗਲਾ;
    db->lookaside.anStat[0]++;
    ਵਾਪਸ (ਖਾਲੀ*)pBuf;
  }ਨਹੀਂਤਾਂ ਜੇ( (pBuf = db->lookaside.pInit)!=0 ){
    db->lookaside.pInit = pBuf->p_ਅਗਲਾ;
    db->lookaside.anStat[0]++;
    ਵਾਪਸ (ਖਾਲੀ*)pBuf;
  }ਨਹੀਂਤਾਂ{
    db->lookaside.anStat[2]++;
  }
#ਨਹੀਂਤਾਂ
  ਪ੍ਰਤਿਜ੍ਞਾ( db!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(db->mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( db->pn_ਬਾਈਟਾਂ_Freed==0 );
  ਜੇ( db->mallocFailed ){
    ਵਾਪਸ 0;
  }
#ਜੇ_ਅੰਤ
  ਵਾਪਸ db_Malloc_Raw_ਅੰਤ(db, n);
}

/* Forward declaration */
ਸਥਿਰ SQLITE_NOINLINE ਖਾਲੀ *db_Realloc_ਅੰਤ(sqlite3 *db, ਖਾਲੀ *p, u64 n);

/*
** Resize the block of memory pointed to by p to n bytes. If the
** resize fails, set the mallocFailed flag in the connection object.
*/
ਖਾਲੀ *sqlite3DbRealloc(sqlite3 *db, ਖਾਲੀ *p, u64 n){
  ਪ੍ਰਤਿਜ੍ਞਾ( db!=0 );
  ਜੇ( p==0 ) ਵਾਪਸ sqlite3DbMallocRawNN(db, n);
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(db->mutex) );
  ਜੇ( ((uptr)p)<(uptr)db->lookaside.p_ਸਿਰਾ ){
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_TWOSIZE_LOOKASIDE
    ਜੇ( ((uptr)p)>=(uptr)db->lookaside.p_ਵਿਚਕਾਰ ){
      ਜੇ( n<=LOOKASIDE_SMALL ) ਵਾਪਸ p;
    }ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
    ਜੇ( ((uptr)p)>=(uptr)db->lookaside.p_ਸ਼ੁਰੂ ){
      ਜੇ( n<=db->lookaside.szTrue ) ਵਾਪਸ p;
    }
  }
  ਵਾਪਸ db_Realloc_ਅੰਤ(db, p, n);
}
ਸਥਿਰ SQLITE_NOINLINE ਖਾਲੀ *db_Realloc_ਅੰਤ(sqlite3 *db, ਖਾਲੀ *p, u64 n){
  ਖਾਲੀ *pNew = 0;
  ਪ੍ਰਤਿਜ੍ਞਾ( db!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p!=0 );
  ਜੇ( db->mallocFailed==0 ){
    ਜੇ( isLookaside(db, p) ){
      pNew = sqlite3DbMallocRawNN(db, n);
      ਜੇ( pNew ){
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(pNew, p, lookaside_Malloc_ਮਾਪ(db, p));
        sqlite3DbFree(db, p);
      }
    }ਨਹੀਂਤਾਂ{
      ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_ਢੇਰ)) );
      ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_ਢੇਰ)) );
      sqlite3_Memdebug_ਸੈੱਟ_Type(p, MEMTYPE_ਢੇਰ);
      pNew = sqlite3Realloc(p, n);
      ਜੇ( !pNew ){
        sqlite3OomFault(db);
      }
      sqlite3_Memdebug_ਸੈੱਟ_Type(pNew,
            (db->lookaside.bDisable==0 ? MEMTYPE_LOOKASIDE : MEMTYPE_ਢੇਰ));
    }
  }
  ਵਾਪਸ pNew;
}

/*
** Attempt to reallocate p.  If the reallocation fails, then free p
** and set the mallocFailed flag in the database connection.
*/
ਖਾਲੀ *sqlite3DbReallocOrFree(sqlite3 *db, ਖਾਲੀ *p, u64 n){
  ਖਾਲੀ *pNew;
  pNew = sqlite3DbRealloc(db, p, n);
  ਜੇ( !pNew ){
    sqlite3DbFree(db, p);
  }
  ਵਾਪਸ pNew;
}

/*
** Make a copy of a string in memory obtained from sqliteMalloc(). These 
** functions call sqlite3MallocRaw() directly instead of sqliteMalloc(). This
** is because when memory debugging is turned on, these two functions are 
** called via macros that record the current file and line number in the
** ThreadData structure.
*/
ਅੱਖਰ *sqlite3DbStrDup(sqlite3 *db, ਅਚਲ ਅੱਖਰ *z){
  ਅੱਖਰ *zNew;
  size_t n;
  ਜੇ( z==0 ){
    ਵਾਪਸ 0;
  }
  n = ਲੜੀ_ਲੰਬਾਈ(z) + 1;
  zNew = sqlite3DbMallocRaw(db, n);
  ਜੇ( zNew ){
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(zNew, z, n);
  }
  ਵਾਪਸ zNew;
}
ਅੱਖਰ *sqlite3DbStrNDup(sqlite3 *db, ਅਚਲ ਅੱਖਰ *z, u64 n){
  ਅੱਖਰ *zNew;
  ਪ੍ਰਤਿਜ੍ਞਾ( db!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( z!=0 || n==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( (n&0x7fffffff)==n );
  zNew = z ? sqlite3DbMallocRawNN(db, n+1) : 0;
  ਜੇ( zNew ){
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(zNew, z, (size_t)n);
    zNew[n] = 0;
  }
  ਵਾਪਸ zNew;
}

/*
** The text between zStart and zEnd represents a phrase within a larger
** SQL statement.  Make a copy of this phrase in space obtained form
** sqlite3DbMalloc().  Omit leading and trailing whitespace.
*/
ਅੱਖਰ *sqlite3_Db_ਵਿਸਤਾਰ_Dup(sqlite3 *db, ਅਚਲ ਅੱਖਰ *z_ਸ਼ੁਰੂ, ਅਚਲ ਅੱਖਰ *z_ਸਿਰਾ){
  ਪੂਰਨ n;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  /* Because of the way the parser works, the span is guaranteed to contain
  ** at least one non-space character */
  ਲਈ(n=0; sqlite3Isspace(z_ਸ਼ੁਰੂ[n]); n++){ ਪ੍ਰਤਿਜ੍ਞਾ( &z_ਸ਼ੁਰੂ[n]<z_ਸਿਰਾ ); }
#ਜੇ_ਅੰਤ
  ਜਦੋਂ( sqlite3Isspace(z_ਸ਼ੁਰੂ[0]) ) z_ਸ਼ੁਰੂ++;
  n = (ਪੂਰਨ)(z_ਸਿਰਾ - z_ਸ਼ੁਰੂ);
  ਜਦੋਂ( sqlite3Isspace(z_ਸ਼ੁਰੂ[n-1]) ) n--;
  ਵਾਪਸ sqlite3DbStrNDup(db, z_ਸ਼ੁਰੂ, n);
}

/*
** Free any prior content in *pz and replace it with a copy of zNew.
*/
ਖਾਲੀ sqlite3_ਸੈੱਟ_String(ਅੱਖਰ **pz, sqlite3 *db, ਅਚਲ ਅੱਖਰ *zNew){
  ਅੱਖਰ *z = sqlite3DbStrDup(db, zNew);
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
ਖਾਲੀ *sqlite3OomFault(sqlite3 *db){
  ਜੇ( db->mallocFailed==0 && db->bBenignMalloc==0 ){
    db->mallocFailed = 1;
    ਜੇ( db->nVdbeExec>0 ){
      AtomicStore(&db->u1.isInterrupted, 1);
    }
    DisableLookaside;
    ਜੇ( db->pParse ){
      Parse *pParse;
      sqlite3_Error_ਸੁਨੇਹਾ(db->pParse, "out of memory");
      db->pParse->rc = SQLITE_NOMEM_BKPT;
      ਲਈ(pParse=db->pParse->pOuterParse; pParse; pParse = pParse->pOuterParse){
        pParse->nErr++;
        pParse->rc = SQLITE_NOMEM;
      } 
    }
  }
  ਵਾਪਸ 0;
}

/*
** This routine reactivates the memory allocator and clears the
** db->mallocFailed flag as necessary.
**
** The memory allocator is not restarted if there are running
** VDBEs.
*/
ਖਾਲੀ sqlite3OomClear(sqlite3 *db){
  ਜੇ( db->mallocFailed && db->nVdbeExec==0 ){
    db->mallocFailed = 0;
    AtomicStore(&db->u1.isInterrupted, 0);
    ਪ੍ਰਤਿਜ੍ਞਾ( db->lookaside.bDisable>0 );
    EnableLookaside;
  }
}

/*
** Take actions at the end of an API call to deal with error codes.
*/
ਸਥਿਰ SQLITE_NOINLINE ਪੂਰਨ apiHandleError(sqlite3 *db, ਪੂਰਨ rc){
  ਜੇ( db->mallocFailed || rc==SQLITE_IOERR_NOMEM ){
    sqlite3OomClear(db);
    sqlite3Error(db, SQLITE_NOMEM);
    ਵਾਪਸ SQLITE_NOMEM_BKPT;
  }
  ਵਾਪਸ rc & db->err_ਨਕਾਬ;
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
ਪੂਰਨ sqlite3ApiExit(sqlite3* db, ਪੂਰਨ rc){
  /* If the db handle must hold the connection handle mutex here.
  ** Otherwise the read (and possible write) of db->mallocFailed 
  ** is unsafe, as is the call to sqlite3Error().
  */
  ਪ੍ਰਤਿਜ੍ਞਾ( db!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(db->mutex) );
  ਜੇ( db->mallocFailed || rc ){
    ਵਾਪਸ apiHandleError(db, rc);
  }
  ਵਾਪਸ 0;
}
