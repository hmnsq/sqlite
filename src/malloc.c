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
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"
#ಅನ್ತರ್ಭಾವ <stdarg.h>

/*
** Attempt to release up to n bytes of non-essential memory currently
** held by SQLite. An example of non-essential memory is memory used to
** cache database pages that are not currently in use.
*/
ಪೂರ್ಣಾಂಕ sqlite3_release_ಸ್ಮರಣೆ(ಪೂರ್ಣಾಂಕ n){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_ಸ್ಮರಣೆ_MANAGEMENT
  ಹಿಂತಿರುಗಿ sqlite3_Pcache_Release_ಸ್ಮರಣೆ(n);
#ಇಲ್ಲದಿದ್ದರೆ
  /* IMPLEMENTATION-OF: R-34391-24921 The sqlite3_release_memory() routine
  ** is a no-op returning zero if SQLite is not compiled with
  ** SQLITE_ENABLE_MEMORY_MANAGEMENT. */
  ಬಳಸದ_PARAMETER(n);
  ಹಿಂತಿರುಗಿ 0;
#ಆದರೆ_ಅಂತ್ಯ
}

/*
** Default value of the hard heap limit.  0 means "no limit".
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_MAX_ಸ್ಮರಣೆ
# define SQLITE_MAX_ಸ್ಮರಣೆ 0
#ಆದರೆ_ಅಂತ್ಯ

/*
** State information local to the memory allocation subsystem.
*/
ಸ್ಥಿರ SQLITE_WSD ರಚನೆ Mem0Global {
  sqlite3_mutex *mutex;         /* Mutex to serialize access */
  sqlite3_int64 alarm_ಮಿತಿ; /* The soft heap limit */
  sqlite3_int64 hard_ಎಲ್ಲೆ;      /* The hard upper bound on memory */

  /*
  ** True if heap is nearly "full" where "full" is defined by the
  ** sqlite3_soft_heap_limit() setting.
  */
  ಪೂರ್ಣಾಂಕ nearly_ತುಂಬಿದ;
} mem0 = { 0, SQLITE_MAX_ಸ್ಮರಣೆ, SQLITE_MAX_ಸ್ಮರಣೆ, 0 };

#ವ್ಯಾಖ್ಯಾನಿಸು mem0 GLOBAL(ರಚನೆ Mem0Global, mem0)

/*
** Return the memory allocator mutex. sqlite3_status() needs it.
*/
sqlite3_mutex *sqlite3MallocMutex(ಶೂನ್ಯ){
  ಹಿಂತಿರುಗಿ mem0.mutex;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_DEPRECATED
/*
** Deprecated external interface.  It used to set an alarm callback
** that was invoked when memory usage grew too large.  Now it is a
** no-op.
*/
ಪೂರ್ಣಾಂಕ sqlite3_ಸ್ಮರಣೆ_alarm(
  ಶೂನ್ಯ(*xCallback)(ಶೂನ್ಯ *pArg, sqlite3_int64 ಬಳಸಿದ,ಪೂರ್ಣಾಂಕ N),
  ಶೂನ್ಯ *pArg,
  sqlite3_int64 i_ಮಿತಿ
){
  (ಶೂನ್ಯ)xCallback;
  (ಶೂನ್ಯ)pArg;
  (ಶೂನ್ಯ)i_ಮಿತಿ;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}
#ಆದರೆ_ಅಂತ್ಯ

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
sqlite3_int64 sqlite3_soft_ರಾಶಿ_limit64(sqlite3_int64 n){
  sqlite3_int64 ಪೂರ್ವ_ಎಲ್ಲೆ;
  sqlite3_int64 excess;
  sqlite3_int64 n_ಬಳಸಿದ;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOINIT
  ಪೂರ್ಣಾಂಕ rc = sqlite3_initialize();
  ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ -1;
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_mutex_enter(mem0.mutex);
  ಪೂರ್ವ_ಎಲ್ಲೆ = mem0.alarm_ಮಿತಿ;
  ಆದರೆ( n<0 ){
    sqlite3_mutex_leave(mem0.mutex);
    ಹಿಂತಿರುಗಿ ಪೂರ್ವ_ಎಲ್ಲೆ;
  }
  ಆದರೆ( mem0.hard_ಎಲ್ಲೆ>0 && (n>mem0.hard_ಎಲ್ಲೆ || n==0) ){
    n = mem0.hard_ಎಲ್ಲೆ;
  }
  mem0.alarm_ಮಿತಿ = n;
  n_ಬಳಸಿದ = sqlite3_ಸ್ಥಿತಿ_ಮೌಲ್ಯ(SQLITE_ಸ್ಥಿತಿ_ಸ್ಮರಣೆ_ಬಳಸಿದ);
  AtomicStore(&mem0.nearly_ತುಂಬಿದ, n>0 && n<=n_ಬಳಸಿದ);
  sqlite3_mutex_leave(mem0.mutex);
  excess = sqlite3_ಸ್ಮರಣೆ_ಬಳಸಿದ() - n;
  ಆದರೆ( excess>0 ) sqlite3_release_ಸ್ಮರಣೆ((ಪೂರ್ಣಾಂಕ)(excess & 0x7fffffff));
  ಹಿಂತಿರುಗಿ ಪೂರ್ವ_ಎಲ್ಲೆ;
}
ಶೂನ್ಯ sqlite3_soft_ರಾಶಿ_ಎಲ್ಲೆ(ಪೂರ್ಣಾಂಕ n){
  ಆದರೆ( n<0 ) n = 0;
  sqlite3_soft_ರಾಶಿ_limit64(n);
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
sqlite3_int64 sqlite3_hard_ರಾಶಿ_limit64(sqlite3_int64 n){
  sqlite3_int64 ಪೂರ್ವ_ಎಲ್ಲೆ;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOINIT
  ಪೂರ್ಣಾಂಕ rc = sqlite3_initialize();
  ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ -1;
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_mutex_enter(mem0.mutex);
  ಪೂರ್ವ_ಎಲ್ಲೆ = mem0.hard_ಎಲ್ಲೆ;
  ಆದರೆ( n>=0 ){
    mem0.hard_ಎಲ್ಲೆ = n;
    ಆದರೆ( n<mem0.alarm_ಮಿತಿ || mem0.alarm_ಮಿತಿ==0 ){
      mem0.alarm_ಮಿತಿ = n;
    }
  }
  sqlite3_mutex_leave(mem0.mutex);
  ಹಿಂತಿರುಗಿ ಪೂರ್ವ_ಎಲ್ಲೆ;
}


/*
** Initialize the memory allocation subsystem.
*/
ಪೂರ್ಣಾಂಕ sqlite3MallocInit(ಶೂನ್ಯ){
  ಪೂರ್ಣಾಂಕ rc;
  ಆದರೆ( sqlite3GlobalConfig.m.xMalloc==0 ){
    sqlite3_Mem_ಗಣ_Default();
  }
  mem0.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM);
  ಆದರೆ( sqlite3GlobalConfig.p_ಪುಟ==0 || sqlite3GlobalConfig.sz_ಪುಟ<512
      || sqlite3GlobalConfig.n_ಪುಟ<=0 ){
    sqlite3GlobalConfig.p_ಪುಟ = 0;
    sqlite3GlobalConfig.sz_ಪುಟ = 0;
  }
  rc = sqlite3GlobalConfig.m.xInit(sqlite3GlobalConfig.m.p_App_ದತ್ತಾಂಶ);
  ಆದರೆ( rc!=SQLITE_OK ) memset(&mem0, 0, ಗಾತ್ರ(mem0));
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Return true if the heap is currently under memory pressure - in other
** words if the amount of heap used is close to the limit set by
** sqlite3_soft_heap_limit().
*/
ಪೂರ್ಣಾಂಕ sqlite3_ರಾಶಿ_Nearly_ತುಂಬಿದ(ಶೂನ್ಯ){
  ಹಿಂತಿರುಗಿ Atomic_ತುಂಬು(&mem0.nearly_ತುಂಬಿದ);
}

/*
** Deinitialize the memory allocation subsystem.
*/
ಶೂನ್ಯ sqlite3_Malloc_ಕೊನೆ(ಶೂನ್ಯ){
  ಆದರೆ( sqlite3GlobalConfig.m.xShutdown ){
    sqlite3GlobalConfig.m.xShutdown(sqlite3GlobalConfig.m.p_App_ದತ್ತಾಂಶ);
  }
  memset(&mem0, 0, ಗಾತ್ರ(mem0));
}

/*
** Return the amount of memory currently checked out.
*/
sqlite3_int64 sqlite3_ಸ್ಮರಣೆ_ಬಳಸಿದ(ಶೂನ್ಯ){
  sqlite3_int64 res, mx;
  sqlite3_status64(SQLITE_ಸ್ಥಿತಿ_ಸ್ಮರಣೆ_ಬಳಸಿದ, &res, &mx, 0);
  ಹಿಂತಿರುಗಿ res;
}

/*
** Return the maximum amount of memory that has ever been
** checked out since either the beginning of this process
** or since the most recent reset.
*/
sqlite3_int64 sqlite3_ಸ್ಮರಣೆ_highwater(ಪೂರ್ಣಾಂಕ resetFlag){
  sqlite3_int64 res, mx;
  sqlite3_status64(SQLITE_ಸ್ಥಿತಿ_ಸ್ಮರಣೆ_ಬಳಸಿದ, &res, &mx, resetFlag);
  ಹಿಂತಿರುಗಿ mx;
}

/*
** Trigger the alarm 
*/
ಸ್ಥಿರ ಶೂನ್ಯ sqlite3MallocAlarm(ಪೂರ್ಣಾಂಕ nByte){
  ಆದರೆ( mem0.alarm_ಮಿತಿ<=0 ) ಹಿಂತಿರುಗಿ;
  sqlite3_mutex_leave(mem0.mutex);
  sqlite3_release_ಸ್ಮರಣೆ(nByte);
  sqlite3_mutex_enter(mem0.mutex);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
/*
** This routine is called whenever an out-of-memory condition is seen,
** It's only purpose to to serve as a breakpoint for gdb or similar
** code debuggers when working on out-of-memory conditions, for example
** caused by PRAGMA hard_heap_limit=N.
*/
ಸ್ಥಿರ SQLITE_NOINLINE ಶೂನ್ಯ test_oom_breakpoint(u64 n){
  ಸ್ಥಿರ u64 nOomFault = 0;
  nOomFault += n;
  /* The assert() is never reached in a human lifetime.  It  is here mostly
  ** to prevent code optimizers from optimizing out this function. */
  ಪ್ರತಿಜ್ಞಾ( (nOomFault>>32) < 0xffffffff );
}
#ಇಲ್ಲದಿದ್ದರೆ
# define test_oom_breakpoint(X)   /* No-op for production builds */
#ಆದರೆ_ಅಂತ್ಯ

/*
** Do a memory allocation with statistics and alarms.  Assume the
** lock is already held.
*/
ಸ್ಥಿರ ಶೂನ್ಯ mallocWithAlarm(ಪೂರ್ಣಾಂಕ n, ಶೂನ್ಯ **pp){
  ಶೂನ್ಯ *p;
  ಪೂರ್ಣಾಂಕ n_ತುಂಬಿದ;
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(mem0.mutex) );
  ಪ್ರತಿಜ್ಞಾ( n>0 );

  /* In Firefox (circa 2017-02-08), xRoundup() is remapped to an internal
  ** implementation of malloc_good_size(), which must be called in debug
  ** mode and specifically when the DMD "Dark Matter Detector" is enabled
  ** or else a crash results.  Hence, do not attempt to optimize out the
  ** following xRoundup() call. */
  n_ತುಂಬಿದ = sqlite3GlobalConfig.m.xRoundup(n);

  sqlite3_ಸ್ಥಿತಿ_Highwater(SQLITE_ಸ್ಥಿತಿ_MALLOC_ಗಾತ್ರ, n);
  ಆದರೆ( mem0.alarm_ಮಿತಿ>0 ){
    sqlite3_int64 n_ಬಳಸಿದ = sqlite3_ಸ್ಥಿತಿ_ಮೌಲ್ಯ(SQLITE_ಸ್ಥಿತಿ_ಸ್ಮರಣೆ_ಬಳಸಿದ);
    ಆದರೆ( n_ಬಳಸಿದ >= mem0.alarm_ಮಿತಿ - n_ತುಂಬಿದ ){
      AtomicStore(&mem0.nearly_ತುಂಬಿದ, 1);
      sqlite3MallocAlarm(n_ತುಂಬಿದ);
      ಆದರೆ( mem0.hard_ಎಲ್ಲೆ ){
        n_ಬಳಸಿದ = sqlite3_ಸ್ಥಿತಿ_ಮೌಲ್ಯ(SQLITE_ಸ್ಥಿತಿ_ಸ್ಮರಣೆ_ಬಳಸಿದ);
        ಆದರೆ( n_ಬಳಸಿದ >= mem0.hard_ಎಲ್ಲೆ - n_ತುಂಬಿದ ){
          test_oom_breakpoint(1);
          *pp = 0;
          ಹಿಂತಿರುಗಿ;
        }
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      AtomicStore(&mem0.nearly_ತುಂಬಿದ, 0);
    }
  }
  p = sqlite3GlobalConfig.m.xMalloc(n_ತುಂಬಿದ);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_ಸ್ಮರಣೆ_MANAGEMENT
  ಆದರೆ( p==0 && mem0.alarm_ಮಿತಿ>0 ){
    sqlite3MallocAlarm(n_ತುಂಬಿದ);
    p = sqlite3GlobalConfig.m.xMalloc(n_ತುಂಬಿದ);
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ( p ){
    n_ತುಂಬಿದ = sqlite3_Malloc_ಗಾತ್ರ(p);
    sqlite3_ಸ್ಥಿತಿ_ಮೇಲೆ(SQLITE_ಸ್ಥಿತಿ_ಸ್ಮರಣೆ_ಬಳಸಿದ, n_ತುಂಬಿದ);
    sqlite3_ಸ್ಥಿತಿ_ಮೇಲೆ(SQLITE_ಸ್ಥಿತಿ_MALLOC_ಎಣಿಕೆ, 1);
  }
  *pp = p;
}

/*
** Allocate memory.  This routine is like sqlite3_malloc() except that it
** assumes the memory subsystem has already been initialized.
*/
ಶೂನ್ಯ *sqlite3Malloc(u64 n){
  ಶೂನ್ಯ *p;
  ಆದರೆ( n==0 || n>SQLITE_MAX_ಹಂಚಿಕೆ_ಗಾತ್ರ ){
    p = 0;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( sqlite3GlobalConfig.bMemstat ){
    sqlite3_mutex_enter(mem0.mutex);
    mallocWithAlarm((ಪೂರ್ಣಾಂಕ)n, &p);
    sqlite3_mutex_leave(mem0.mutex);
  }ಇಲ್ಲದಿದ್ದರೆ{
    p = sqlite3GlobalConfig.m.xMalloc((ಪೂರ್ಣಾಂಕ)n);
  }
  ಪ್ರತಿಜ್ಞಾ( EIGHT_BYTE_ALIGNMENT(p) );  /* IMP: R-11148-40995 */
  ಹಿಂತಿರುಗಿ p;
}

/*
** This version of the memory allocation is for use by the application.
** First make sure the memory subsystem is initialized, then do the
** allocation.
*/
ಶೂನ್ಯ *sqlite3_malloc(ಪೂರ್ಣಾಂಕ n){
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOINIT
  ಆದರೆ( sqlite3_initialize() ) ಹಿಂತಿರುಗಿ 0;
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ n<=0 ? 0 : sqlite3Malloc(n);
}
ಶೂನ್ಯ *sqlite3_malloc64(sqlite3_uint64 n){
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOINIT
  ಆದರೆ( sqlite3_initialize() ) ಹಿಂತಿರುಗಿ 0;
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ sqlite3Malloc(n);
}

/*
** TRUE if p is a lookaside memory allocation from db
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_LOOKASIDE
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ isLookaside(sqlite3 *db, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *p){
  ಹಿಂತಿರುಗಿ SQLITE_WITHIN(p, db->lookaside.p_ಆರಂಭ, db->lookaside.p_True_ಕೊನೆ);
}
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು isLookaside(A,B) 0
#ಆದರೆ_ಅಂತ್ಯ

/*
** Return the size of a memory allocation previously obtained from
** sqlite3Malloc() or sqlite3_malloc().
*/
ಪೂರ್ಣಾಂಕ sqlite3_Malloc_ಗಾತ್ರ(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *p){
  ಪ್ರತಿಜ್ಞಾ( sqlite3MemdebugHasType(p, MEMTYPE_ರಾಶಿ) );
  ಹಿಂತಿರುಗಿ sqlite3GlobalConfig.m.x_ಗಾತ್ರ((ಶೂನ್ಯ*)p);
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ lookaside_Malloc_ಗಾತ್ರ(sqlite3 *db, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *p){
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_TWOSIZE_LOOKASIDE    
  ಹಿಂತಿರುಗಿ p<db->lookaside.p_ಮಧ್ಯ ? db->lookaside.szTrue : LOOKASIDE_SMALL;
#ಇಲ್ಲದಿದ್ದರೆ
  ಹಿಂತಿರುಗಿ db->lookaside.szTrue;
#ಆದರೆ_ಅಂತ್ಯ  
}
ಪೂರ್ಣಾಂಕ sqlite3_Db_Malloc_ಗಾತ್ರ(sqlite3 *db, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *p){
  ಪ್ರತಿಜ್ಞಾ( p!=0 );
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  ಆದರೆ( db==0 ){
    ಪ್ರತಿಜ್ಞಾ( sqlite3MemdebugNoType(p, (u8)~MEMTYPE_ರಾಶಿ) );
    ಪ್ರತಿಜ್ಞಾ( sqlite3MemdebugHasType(p, MEMTYPE_ರಾಶಿ) );
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( !isLookaside(db,p) ){
    ಪ್ರತಿಜ್ಞಾ( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_ರಾಶಿ)) );
    ಪ್ರತಿಜ್ಞಾ( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_ರಾಶಿ)) );
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ( db ){
    ಆದರೆ( ((uptr)p)<(uptr)(db->lookaside.p_True_ಕೊನೆ) ){
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_TWOSIZE_LOOKASIDE
      ಆದರೆ( ((uptr)p)>=(uptr)(db->lookaside.p_ಮಧ್ಯ) ){
        ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(db->mutex) );
        ಹಿಂತಿರುಗಿ LOOKASIDE_SMALL;
      }
#ಆದರೆ_ಅಂತ್ಯ
      ಆದರೆ( ((uptr)p)>=(uptr)(db->lookaside.p_ಆರಂಭ) ){
        ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(db->mutex) );
        ಹಿಂತಿರುಗಿ db->lookaside.szTrue;
      }
    }
  }
  ಹಿಂತಿರುಗಿ sqlite3GlobalConfig.m.x_ಗಾತ್ರ((ಶೂನ್ಯ*)p);
}
sqlite3_uint64 sqlite3_msize(ಶೂನ್ಯ *p){
  ಪ್ರತಿಜ್ಞಾ( sqlite3MemdebugNoType(p, (u8)~MEMTYPE_ರಾಶಿ) );
  ಪ್ರತಿಜ್ಞಾ( sqlite3MemdebugHasType(p, MEMTYPE_ರಾಶಿ) );
  ಹಿಂತಿರುಗಿ p ? sqlite3GlobalConfig.m.x_ಗಾತ್ರ(p) : 0;
}

/*
** Free memory previously obtained from sqlite3Malloc().
*/
ಶೂನ್ಯ sqlite3_free(ಶೂನ್ಯ *p){
  ಆದರೆ( p==0 ) ಹಿಂತಿರುಗಿ;  /* IMP: R-49053-54554 */
  ಪ್ರತಿಜ್ಞಾ( sqlite3MemdebugHasType(p, MEMTYPE_ರಾಶಿ) );
  ಪ್ರತಿಜ್ಞಾ( sqlite3MemdebugNoType(p, (u8)~MEMTYPE_ರಾಶಿ) );
  ಆದರೆ( sqlite3GlobalConfig.bMemstat ){
    sqlite3_mutex_enter(mem0.mutex);
    sqlite3_ಸ್ಥಿತಿ_ಕೆಳಕ್ಕೆ(SQLITE_ಸ್ಥಿತಿ_ಸ್ಮರಣೆ_ಬಳಸಿದ, sqlite3_Malloc_ಗಾತ್ರ(p));
    sqlite3_ಸ್ಥಿತಿ_ಕೆಳಕ್ಕೆ(SQLITE_ಸ್ಥಿತಿ_MALLOC_ಎಣಿಕೆ, 1);
    sqlite3GlobalConfig.m.xFree(p);
    sqlite3_mutex_leave(mem0.mutex);
  }ಇಲ್ಲದಿದ್ದರೆ{
    sqlite3GlobalConfig.m.xFree(p);
  }
}

/*
** Add the size of memory allocation "p" to the count in
** *db->pnBytesFreed.
*/
ಸ್ಥಿರ SQLITE_NOINLINE ಶೂನ್ಯ measure_ಹಂಚಿಕೆ_ಗಾತ್ರ(sqlite3 *db, ಶೂನ್ಯ *p){
  *db->pn_ಬೈಟುಗಳು_Freed += sqlite3_Db_Malloc_ಗಾತ್ರ(db,p);
}

/*
** Free memory that might be associated with a particular database
** connection.  Calling sqlite3DbFree(D,X) for X==0 is a harmless no-op.
** The sqlite3DbFreeNN(D,X) version requires that X be non-NULL.
*/
ಶೂನ್ಯ sqlite3DbFreeNN(sqlite3 *db, ಶೂನ್ಯ *p){
  ಪ್ರತಿಜ್ಞಾ( db==0 || sqlite3_mutex_held(db->mutex) );
  ಪ್ರತಿಜ್ಞಾ( p!=0 );
  ಆದರೆ( db ){
    ಆದರೆ( ((uptr)p)<(uptr)(db->lookaside.p_ಕೊನೆ) ){
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_TWOSIZE_LOOKASIDE
      ಆದರೆ( ((uptr)p)>=(uptr)(db->lookaside.p_ಮಧ್ಯ) ){
        Lookaside_ಸ್ಥಾನ *pBuf = (Lookaside_ಸ್ಥಾನ*)p;
        ಪ್ರತಿಜ್ಞಾ( db->pn_ಬೈಟುಗಳು_Freed==0 );
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
        memset(p, 0xaa, LOOKASIDE_SMALL);  /* Trash freed content */
#ಆದರೆ_ಅಂತ್ಯ
        pBuf->p_ಮುಂದಿನ = db->lookaside.pSmallFree;
        db->lookaside.pSmallFree = pBuf;
        ಹಿಂತಿರುಗಿ;
      }
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
      ಆದರೆ( ((uptr)p)>=(uptr)(db->lookaside.p_ಆರಂಭ) ){
        Lookaside_ಸ್ಥಾನ *pBuf = (Lookaside_ಸ್ಥಾನ*)p;
        ಪ್ರತಿಜ್ಞಾ( db->pn_ಬೈಟುಗಳು_Freed==0 );
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
        memset(p, 0xaa, db->lookaside.szTrue);  /* Trash freed content */
#ಆದರೆ_ಅಂತ್ಯ
        pBuf->p_ಮುಂದಿನ = db->lookaside.pFree;
        db->lookaside.pFree = pBuf;
        ಹಿಂತಿರುಗಿ;
      }
    }
    ಆದರೆ( db->pn_ಬೈಟುಗಳು_Freed ){
      measure_ಹಂಚಿಕೆ_ಗಾತ್ರ(db, p);
      ಹಿಂತಿರುಗಿ;
    }
  }
  ಪ್ರತಿಜ್ಞಾ( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_ರಾಶಿ)) );
  ಪ್ರತಿಜ್ಞಾ( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_ರಾಶಿ)) );
  ಪ್ರತಿಜ್ಞಾ( db!=0 || sqlite3MemdebugNoType(p, MEMTYPE_LOOKASIDE) );
  sqlite3_Memdebug_ಗಣ_Type(p, MEMTYPE_ರಾಶಿ);
  sqlite3_free(p);
}
ಶೂನ್ಯ sqlite3DbNNFreeNN(sqlite3 *db, ಶೂನ್ಯ *p){
  ಪ್ರತಿಜ್ಞಾ( db!=0 );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(db->mutex) );
  ಪ್ರತಿಜ್ಞಾ( p!=0 );
  ಆದರೆ( ((uptr)p)<(uptr)(db->lookaside.p_ಕೊನೆ) ){
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_TWOSIZE_LOOKASIDE
    ಆದರೆ( ((uptr)p)>=(uptr)(db->lookaside.p_ಮಧ್ಯ) ){
      Lookaside_ಸ್ಥಾನ *pBuf = (Lookaside_ಸ್ಥಾನ*)p;
      ಪ್ರತಿಜ್ಞಾ( db->pn_ಬೈಟುಗಳು_Freed==0 );
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
      memset(p, 0xaa, LOOKASIDE_SMALL);  /* Trash freed content */
#ಆದರೆ_ಅಂತ್ಯ
      pBuf->p_ಮುಂದಿನ = db->lookaside.pSmallFree;
      db->lookaside.pSmallFree = pBuf;
      ಹಿಂತಿರುಗಿ;
    }
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
    ಆದರೆ( ((uptr)p)>=(uptr)(db->lookaside.p_ಆರಂಭ) ){
      Lookaside_ಸ್ಥಾನ *pBuf = (Lookaside_ಸ್ಥಾನ*)p;
      ಪ್ರತಿಜ್ಞಾ( db->pn_ಬೈಟುಗಳು_Freed==0 );
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
      memset(p, 0xaa, db->lookaside.szTrue);  /* Trash freed content */
#ಆದರೆ_ಅಂತ್ಯ
      pBuf->p_ಮುಂದಿನ = db->lookaside.pFree;
      db->lookaside.pFree = pBuf;
      ಹಿಂತಿರುಗಿ;
    }
  }
  ಆದರೆ( db->pn_ಬೈಟುಗಳು_Freed ){
    measure_ಹಂಚಿಕೆ_ಗಾತ್ರ(db, p);
    ಹಿಂತಿರುಗಿ;
  }
  ಪ್ರತಿಜ್ಞಾ( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_ರಾಶಿ)) );
  ಪ್ರತಿಜ್ಞಾ( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_ರಾಶಿ)) );
  sqlite3_Memdebug_ಗಣ_Type(p, MEMTYPE_ರಾಶಿ);
  sqlite3_free(p);
}
ಶೂನ್ಯ sqlite3DbFree(sqlite3 *db, ಶೂನ್ಯ *p){
  ಪ್ರತಿಜ್ಞಾ( db==0 || sqlite3_mutex_held(db->mutex) );
  ಆದರೆ( p ) sqlite3DbFreeNN(db, p);
}

/*
** Change the size of an existing memory allocation
*/
ಶೂನ್ಯ *sqlite3Realloc(ಶೂನ್ಯ *pOld, u64 n_ಬೈಟುಗಳು){
  ಪೂರ್ಣಾಂಕ nOld, nNew, nDiff;
  ಶೂನ್ಯ *pNew;
  ಪ್ರತಿಜ್ಞಾ( sqlite3MemdebugHasType(pOld, MEMTYPE_ರಾಶಿ) );
  ಪ್ರತಿಜ್ಞಾ( sqlite3MemdebugNoType(pOld, (u8)~MEMTYPE_ರಾಶಿ) );
  ಆದರೆ( pOld==0 ){
    ಹಿಂತಿರುಗಿ sqlite3Malloc(n_ಬೈಟುಗಳು); /* IMP: R-04300-56712 */
  }
  ಆದರೆ( n_ಬೈಟುಗಳು==0 ){
    sqlite3_free(pOld); /* IMP: R-26507-47431 */
    ಹಿಂತಿರುಗಿ 0;
  }
  ಆದರೆ( n_ಬೈಟುಗಳು>SQLITE_MAX_ಹಂಚಿಕೆ_ಗಾತ್ರ ){
    ಹಿಂತಿರುಗಿ 0;
  }
  nOld = sqlite3_Malloc_ಗಾತ್ರ(pOld);
  /* IMPLEMENTATION-OF: R-46199-30249 SQLite guarantees that the second
  ** argument to xRealloc is always a value returned by a prior call to
  ** xRoundup. */
  nNew = sqlite3GlobalConfig.m.xRoundup((ಪೂರ್ಣಾಂಕ)n_ಬೈಟುಗಳು);
  ಆದರೆ( nOld==nNew ){
    pNew = pOld;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( sqlite3GlobalConfig.bMemstat ){
    sqlite3_int64 n_ಬಳಸಿದ;
    sqlite3_mutex_enter(mem0.mutex);
    sqlite3_ಸ್ಥಿತಿ_Highwater(SQLITE_ಸ್ಥಿತಿ_MALLOC_ಗಾತ್ರ, (ಪೂರ್ಣಾಂಕ)n_ಬೈಟುಗಳು);
    nDiff = nNew - nOld;
    ಆದರೆ( nDiff>0 && (n_ಬಳಸಿದ = sqlite3_ಸ್ಥಿತಿ_ಮೌಲ್ಯ(SQLITE_ಸ್ಥಿತಿ_ಸ್ಮರಣೆ_ಬಳಸಿದ)) >= 
          mem0.alarm_ಮಿತಿ-nDiff ){
      sqlite3MallocAlarm(nDiff);
      ಆದರೆ( mem0.hard_ಎಲ್ಲೆ>0 && n_ಬಳಸಿದ >= mem0.hard_ಎಲ್ಲೆ - nDiff ){
        sqlite3_mutex_leave(mem0.mutex);
        test_oom_breakpoint(1);
        ಹಿಂತಿರುಗಿ 0;
      }
    }
    pNew = sqlite3GlobalConfig.m.xRealloc(pOld, nNew);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_ಸ್ಮರಣೆ_MANAGEMENT
    ಆದರೆ( pNew==0 && mem0.alarm_ಮಿತಿ>0 ){
      sqlite3MallocAlarm((ಪೂರ್ಣಾಂಕ)n_ಬೈಟುಗಳು);
      pNew = sqlite3GlobalConfig.m.xRealloc(pOld, nNew);
    }
#ಆದರೆ_ಅಂತ್ಯ
    ಆದರೆ( pNew ){
      nNew = sqlite3_Malloc_ಗಾತ್ರ(pNew);
      sqlite3_ಸ್ಥಿತಿ_ಮೇಲೆ(SQLITE_ಸ್ಥಿತಿ_ಸ್ಮರಣೆ_ಬಳಸಿದ, nNew-nOld);
    }
    sqlite3_mutex_leave(mem0.mutex);
  }ಇಲ್ಲದಿದ್ದರೆ{
    pNew = sqlite3GlobalConfig.m.xRealloc(pOld, nNew);
  }
  ಪ್ರತಿಜ್ಞಾ( EIGHT_BYTE_ALIGNMENT(pNew) ); /* IMP: R-11148-40995 */
  ಹಿಂತಿರುಗಿ pNew;
}

/*
** The public interface to sqlite3Realloc.  Make sure that the memory
** subsystem is initialized prior to invoking sqliteRealloc.
*/
ಶೂನ್ಯ *sqlite3_realloc(ಶೂನ್ಯ *pOld, ಪೂರ್ಣಾಂಕ n){
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOINIT
  ಆದರೆ( sqlite3_initialize() ) ಹಿಂತಿರುಗಿ 0;
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ( n<0 ) n = 0;  /* IMP: R-26507-47431 */
  ಹಿಂತಿರುಗಿ sqlite3Realloc(pOld, n);
}
ಶೂನ್ಯ *sqlite3_realloc64(ಶೂನ್ಯ *pOld, sqlite3_uint64 n){
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOINIT
  ಆದರೆ( sqlite3_initialize() ) ಹಿಂತಿರುಗಿ 0;
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ sqlite3Realloc(pOld, n);
}


/*
** Allocate and zero memory.
*/ 
ಶೂನ್ಯ *sqlite3MallocZero(u64 n){
  ಶೂನ್ಯ *p = sqlite3Malloc(n);
  ಆದರೆ( p ){
    memset(p, 0, (size_t)n);
  }
  ಹಿಂತಿರುಗಿ p;
}

/*
** Allocate and zero memory.  If the allocation fails, make
** the mallocFailed flag in the connection pointer.
*/
ಶೂನ್ಯ *sqlite3DbMallocZero(sqlite3 *db, u64 n){
  ಶೂನ್ಯ *p;
  testcase( db==0 );
  p = sqlite3DbMallocRaw(db, n);
  ಆದರೆ( p ) memset(p, 0, (size_t)n);
  ಹಿಂತಿರುಗಿ p;
}


/* Finish the work of sqlite3DbMallocRawNN for the unusual and
** slower case when the allocation cannot be fulfilled using lookaside.
*/
ಸ್ಥಿರ SQLITE_NOINLINE ಶೂನ್ಯ *db_Malloc_Raw_ಮುಕ್ತಾಯ(sqlite3 *db, u64 n){
  ಶೂನ್ಯ *p;
  ಪ್ರತಿಜ್ಞಾ( db!=0 );
  p = sqlite3Malloc(n);
  ಆದರೆ( !p ) sqlite3OomFault(db);
  sqlite3_Memdebug_ಗಣ_Type(p, 
         (db->lookaside.bDisable==0) ? MEMTYPE_LOOKASIDE : MEMTYPE_ರಾಶಿ);
  ಹಿಂತಿರುಗಿ p;
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
ಶೂನ್ಯ *sqlite3DbMallocRaw(sqlite3 *db, u64 n){
  ಶೂನ್ಯ *p;
  ಆದರೆ( db ) ಹಿಂತಿರುಗಿ sqlite3DbMallocRawNN(db, n);
  p = sqlite3Malloc(n);
  sqlite3_Memdebug_ಗಣ_Type(p, MEMTYPE_ರಾಶಿ);
  ಹಿಂತಿರುಗಿ p;
}
ಶೂನ್ಯ *sqlite3DbMallocRawNN(sqlite3 *db, u64 n){
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_LOOKASIDE
  Lookaside_ಸ್ಥಾನ *pBuf;
  ಪ್ರತಿಜ್ಞಾ( db!=0 );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(db->mutex) );
  ಪ್ರತಿಜ್ಞಾ( db->pn_ಬೈಟುಗಳು_Freed==0 );
  ಆದರೆ( n>db->lookaside.sz ){
    ಆದರೆ( !db->lookaside.bDisable ){
      db->lookaside.anStat[1]++;      
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( db->mallocFailed ){
      ಹಿಂತಿರುಗಿ 0;
    }
    ಹಿಂತಿರುಗಿ db_Malloc_Raw_ಮುಕ್ತಾಯ(db, n);
  }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_TWOSIZE_LOOKASIDE
  ಆದರೆ( n<=LOOKASIDE_SMALL ){
    ಆದರೆ( (pBuf = db->lookaside.pSmallFree)!=0 ){
      db->lookaside.pSmallFree = pBuf->p_ಮುಂದಿನ;
      db->lookaside.anStat[0]++;
      ಹಿಂತಿರುಗಿ (ಶೂನ್ಯ*)pBuf;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( (pBuf = db->lookaside.pSmallInit)!=0 ){
      db->lookaside.pSmallInit = pBuf->p_ಮುಂದಿನ;
      db->lookaside.anStat[0]++;
      ಹಿಂತಿರುಗಿ (ಶೂನ್ಯ*)pBuf;
    }
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ( (pBuf = db->lookaside.pFree)!=0 ){
    db->lookaside.pFree = pBuf->p_ಮುಂದಿನ;
    db->lookaside.anStat[0]++;
    ಹಿಂತಿರುಗಿ (ಶೂನ್ಯ*)pBuf;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( (pBuf = db->lookaside.pInit)!=0 ){
    db->lookaside.pInit = pBuf->p_ಮುಂದಿನ;
    db->lookaside.anStat[0]++;
    ಹಿಂತಿರುಗಿ (ಶೂನ್ಯ*)pBuf;
  }ಇಲ್ಲದಿದ್ದರೆ{
    db->lookaside.anStat[2]++;
  }
#ಇಲ್ಲದಿದ್ದರೆ
  ಪ್ರತಿಜ್ಞಾ( db!=0 );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(db->mutex) );
  ಪ್ರತಿಜ್ಞಾ( db->pn_ಬೈಟುಗಳು_Freed==0 );
  ಆದರೆ( db->mallocFailed ){
    ಹಿಂತಿರುಗಿ 0;
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ db_Malloc_Raw_ಮುಕ್ತಾಯ(db, n);
}

/* Forward declaration */
ಸ್ಥಿರ SQLITE_NOINLINE ಶೂನ್ಯ *db_Realloc_ಮುಕ್ತಾಯ(sqlite3 *db, ಶೂನ್ಯ *p, u64 n);

/*
** Resize the block of memory pointed to by p to n bytes. If the
** resize fails, set the mallocFailed flag in the connection object.
*/
ಶೂನ್ಯ *sqlite3DbRealloc(sqlite3 *db, ಶೂನ್ಯ *p, u64 n){
  ಪ್ರತಿಜ್ಞಾ( db!=0 );
  ಆದರೆ( p==0 ) ಹಿಂತಿರುಗಿ sqlite3DbMallocRawNN(db, n);
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(db->mutex) );
  ಆದರೆ( ((uptr)p)<(uptr)db->lookaside.p_ಕೊನೆ ){
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_TWOSIZE_LOOKASIDE
    ಆದರೆ( ((uptr)p)>=(uptr)db->lookaside.p_ಮಧ್ಯ ){
      ಆದರೆ( n<=LOOKASIDE_SMALL ) ಹಿಂತಿರುಗಿ p;
    }ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
    ಆದರೆ( ((uptr)p)>=(uptr)db->lookaside.p_ಆರಂಭ ){
      ಆದರೆ( n<=db->lookaside.szTrue ) ಹಿಂತಿರುಗಿ p;
    }
  }
  ಹಿಂತಿರುಗಿ db_Realloc_ಮುಕ್ತಾಯ(db, p, n);
}
ಸ್ಥಿರ SQLITE_NOINLINE ಶೂನ್ಯ *db_Realloc_ಮುಕ್ತಾಯ(sqlite3 *db, ಶೂನ್ಯ *p, u64 n){
  ಶೂನ್ಯ *pNew = 0;
  ಪ್ರತಿಜ್ಞಾ( db!=0 );
  ಪ್ರತಿಜ್ಞಾ( p!=0 );
  ಆದರೆ( db->mallocFailed==0 ){
    ಆದರೆ( isLookaside(db, p) ){
      pNew = sqlite3DbMallocRawNN(db, n);
      ಆದರೆ( pNew ){
        ಸ್ಮರಣೆ_ನಕಲು(pNew, p, lookaside_Malloc_ಗಾತ್ರ(db, p));
        sqlite3DbFree(db, p);
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಪ್ರತಿಜ್ಞಾ( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_ರಾಶಿ)) );
      ಪ್ರತಿಜ್ಞಾ( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_ರಾಶಿ)) );
      sqlite3_Memdebug_ಗಣ_Type(p, MEMTYPE_ರಾಶಿ);
      pNew = sqlite3Realloc(p, n);
      ಆದರೆ( !pNew ){
        sqlite3OomFault(db);
      }
      sqlite3_Memdebug_ಗಣ_Type(pNew,
            (db->lookaside.bDisable==0 ? MEMTYPE_LOOKASIDE : MEMTYPE_ರಾಶಿ));
    }
  }
  ಹಿಂತಿರುಗಿ pNew;
}

/*
** Attempt to reallocate p.  If the reallocation fails, then free p
** and set the mallocFailed flag in the database connection.
*/
ಶೂನ್ಯ *sqlite3DbReallocOrFree(sqlite3 *db, ಶೂನ್ಯ *p, u64 n){
  ಶೂನ್ಯ *pNew;
  pNew = sqlite3DbRealloc(db, p, n);
  ಆದರೆ( !pNew ){
    sqlite3DbFree(db, p);
  }
  ಹಿಂತಿರುಗಿ pNew;
}

/*
** Make a copy of a string in memory obtained from sqliteMalloc(). These 
** functions call sqlite3MallocRaw() directly instead of sqliteMalloc(). This
** is because when memory debugging is turned on, these two functions are 
** called via macros that record the current file and line number in the
** ThreadData structure.
*/
ಅಕ್ಷರ *sqlite3DbStrDup(sqlite3 *db, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z){
  ಅಕ್ಷರ *zNew;
  size_t n;
  ಆದರೆ( z==0 ){
    ಹಿಂತಿರುಗಿ 0;
  }
  n = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(z) + 1;
  zNew = sqlite3DbMallocRaw(db, n);
  ಆದರೆ( zNew ){
    ಸ್ಮರಣೆ_ನಕಲು(zNew, z, n);
  }
  ಹಿಂತಿರುಗಿ zNew;
}
ಅಕ್ಷರ *sqlite3DbStrNDup(sqlite3 *db, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z, u64 n){
  ಅಕ್ಷರ *zNew;
  ಪ್ರತಿಜ್ಞಾ( db!=0 );
  ಪ್ರತಿಜ್ಞಾ( z!=0 || n==0 );
  ಪ್ರತಿಜ್ಞಾ( (n&0x7fffffff)==n );
  zNew = z ? sqlite3DbMallocRawNN(db, n+1) : 0;
  ಆದರೆ( zNew ){
    ಸ್ಮರಣೆ_ನಕಲು(zNew, z, (size_t)n);
    zNew[n] = 0;
  }
  ಹಿಂತಿರುಗಿ zNew;
}

/*
** The text between zStart and zEnd represents a phrase within a larger
** SQL statement.  Make a copy of this phrase in space obtained form
** sqlite3DbMalloc().  Omit leading and trailing whitespace.
*/
ಅಕ್ಷರ *sqlite3_Db_ಹರವು_Dup(sqlite3 *db, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಆರಂಭ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಕೊನೆ){
  ಪೂರ್ಣಾಂಕ n;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  /* Because of the way the parser works, the span is guaranteed to contain
  ** at least one non-space character */
  ಗಾಗಿ(n=0; sqlite3Isspace(z_ಆರಂಭ[n]); n++){ ಪ್ರತಿಜ್ಞಾ( &z_ಆರಂಭ[n]<z_ಕೊನೆ ); }
#ಆದರೆ_ಅಂತ್ಯ
  ಆಗ( sqlite3Isspace(z_ಆರಂಭ[0]) ) z_ಆರಂಭ++;
  n = (ಪೂರ್ಣಾಂಕ)(z_ಕೊನೆ - z_ಆರಂಭ);
  ಆಗ( sqlite3Isspace(z_ಆರಂಭ[n-1]) ) n--;
  ಹಿಂತಿರುಗಿ sqlite3DbStrNDup(db, z_ಆರಂಭ, n);
}

/*
** Free any prior content in *pz and replace it with a copy of zNew.
*/
ಶೂನ್ಯ sqlite3_ಗಣ_String(ಅಕ್ಷರ **pz, sqlite3 *db, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zNew){
  ಅಕ್ಷರ *z = sqlite3DbStrDup(db, zNew);
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
ಶೂನ್ಯ *sqlite3OomFault(sqlite3 *db){
  ಆದರೆ( db->mallocFailed==0 && db->bBenignMalloc==0 ){
    db->mallocFailed = 1;
    ಆದರೆ( db->nVdbeExec>0 ){
      AtomicStore(&db->u1.isInterrupted, 1);
    }
    DisableLookaside;
    ಆದರೆ( db->pParse ){
      Parse *pParse;
      sqlite3_Error_ಸಂದೇಶ(db->pParse, "out of memory");
      db->pParse->rc = SQLITE_NOMEM_BKPT;
      ಗಾಗಿ(pParse=db->pParse->pOuterParse; pParse; pParse = pParse->pOuterParse){
        pParse->nErr++;
        pParse->rc = SQLITE_NOMEM;
      } 
    }
  }
  ಹಿಂತಿರುಗಿ 0;
}

/*
** This routine reactivates the memory allocator and clears the
** db->mallocFailed flag as necessary.
**
** The memory allocator is not restarted if there are running
** VDBEs.
*/
ಶೂನ್ಯ sqlite3OomClear(sqlite3 *db){
  ಆದರೆ( db->mallocFailed && db->nVdbeExec==0 ){
    db->mallocFailed = 0;
    AtomicStore(&db->u1.isInterrupted, 0);
    ಪ್ರತಿಜ್ಞಾ( db->lookaside.bDisable>0 );
    EnableLookaside;
  }
}

/*
** Take actions at the end of an API call to deal with error codes.
*/
ಸ್ಥಿರ SQLITE_NOINLINE ಪೂರ್ಣಾಂಕ apiHandleError(sqlite3 *db, ಪೂರ್ಣಾಂಕ rc){
  ಆದರೆ( db->mallocFailed || rc==SQLITE_IOERR_NOMEM ){
    sqlite3OomClear(db);
    sqlite3Error(db, SQLITE_NOMEM);
    ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
  }
  ಹಿಂತಿರುಗಿ rc & db->err_ಮುಖವಾಡ;
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
ಪೂರ್ಣಾಂಕ sqlite3ApiExit(sqlite3* db, ಪೂರ್ಣಾಂಕ rc){
  /* If the db handle must hold the connection handle mutex here.
  ** Otherwise the read (and possible write) of db->mallocFailed 
  ** is unsafe, as is the call to sqlite3Error().
  */
  ಪ್ರತಿಜ್ಞಾ( db!=0 );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(db->mutex) );
  ಆದರೆ( db->mallocFailed || rc ){
    ಹಿಂತಿರುಗಿ apiHandleError(db, rc);
  }
  ಹಿಂತಿರುಗಿ 0;
}
