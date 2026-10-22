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
#شامل "sqliteInt.h"
#شامل <stdarg.h>

/*
** Attempt to release up to n bytes of non-essential memory currently
** held by SQLite. An example of non-essential memory is memory used to
** cache database pages that are not currently in use.
*/
صحیح sqlite3_release_حافظہ(صحیح n){
#اگر_تعریف_شدہ SQLITE_ENABLE_حافظہ_MANAGEMENT
  واپس sqlite3_Pcache_Release_حافظہ(n);
#ورنہ
  /* IMPLEMENTATION-OF: R-34391-24921 The sqlite3_release_memory() routine
  ** is a no-op returning zero if SQLite is not compiled with
  ** SQLITE_ENABLE_MEMORY_MANAGEMENT. */
  غیر_مستعمل_PARAMETER(n);
  واپس 0;
#اگر_ختم
}

/*
** Default value of the hard heap limit.  0 means "no limit".
*/
#اگر_غیر_تعریف_شدہ SQLITE_MAX_حافظہ
# define SQLITE_MAX_حافظہ 0
#اگر_ختم

/*
** State information local to the memory allocation subsystem.
*/
ساکن SQLITE_WSD ساخت Mem0Global {
  sqlite3_mutex *mutex;         /* Mutex to serialize access */
  sqlite3_int64 alarm_حد; /* The soft heap limit */
  sqlite3_int64 hard_حد_بندی;      /* The hard upper bound on memory */

  /*
  ** True if heap is nearly "full" where "full" is defined by the
  ** sqlite3_soft_heap_limit() setting.
  */
  صحیح nearly_بھرا;
} mem0 = { 0, SQLITE_MAX_حافظہ, SQLITE_MAX_حافظہ, 0 };

#تعریف_کرو mem0 GLOBAL(ساخت Mem0Global, mem0)

/*
** Return the memory allocator mutex. sqlite3_status() needs it.
*/
sqlite3_mutex *sqlite3MallocMutex(خالی){
  واپس mem0.mutex;
}

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_DEPRECATED
/*
** Deprecated external interface.  It used to set an alarm callback
** that was invoked when memory usage grew too large.  Now it is a
** no-op.
*/
صحیح sqlite3_حافظہ_alarm(
  خالی(*xCallback)(خالی *pArg, sqlite3_int64 مستعمل,صحیح N),
  خالی *pArg,
  sqlite3_int64 i_حد
){
  (خالی)xCallback;
  (خالی)pArg;
  (خالی)i_حد;
  واپس SQLITE_OK;
}
#اگر_ختم

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
sqlite3_int64 sqlite3_soft_ڈھیر_limit64(sqlite3_int64 n){
  sqlite3_int64 پیشگی_حد_بندی;
  sqlite3_int64 excess;
  sqlite3_int64 n_مستعمل;
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOINIT
  صحیح rc = sqlite3_initialize();
  اگر( rc ) واپس -1;
#اگر_ختم
  sqlite3_mutex_enter(mem0.mutex);
  پیشگی_حد_بندی = mem0.alarm_حد;
  اگر( n<0 ){
    sqlite3_mutex_leave(mem0.mutex);
    واپس پیشگی_حد_بندی;
  }
  اگر( mem0.hard_حد_بندی>0 && (n>mem0.hard_حد_بندی || n==0) ){
    n = mem0.hard_حد_بندی;
  }
  mem0.alarm_حد = n;
  n_مستعمل = sqlite3_حالت_قدر(SQLITE_حالت_حافظہ_مستعمل);
  AtomicStore(&mem0.nearly_بھرا, n>0 && n<=n_مستعمل);
  sqlite3_mutex_leave(mem0.mutex);
  excess = sqlite3_حافظہ_مستعمل() - n;
  اگر( excess>0 ) sqlite3_release_حافظہ((صحیح)(excess & 0x7fffffff));
  واپس پیشگی_حد_بندی;
}
خالی sqlite3_soft_ڈھیر_حد_بندی(صحیح n){
  اگر( n<0 ) n = 0;
  sqlite3_soft_ڈھیر_limit64(n);
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
sqlite3_int64 sqlite3_hard_ڈھیر_limit64(sqlite3_int64 n){
  sqlite3_int64 پیشگی_حد_بندی;
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOINIT
  صحیح rc = sqlite3_initialize();
  اگر( rc ) واپس -1;
#اگر_ختم
  sqlite3_mutex_enter(mem0.mutex);
  پیشگی_حد_بندی = mem0.hard_حد_بندی;
  اگر( n>=0 ){
    mem0.hard_حد_بندی = n;
    اگر( n<mem0.alarm_حد || mem0.alarm_حد==0 ){
      mem0.alarm_حد = n;
    }
  }
  sqlite3_mutex_leave(mem0.mutex);
  واپس پیشگی_حد_بندی;
}


/*
** Initialize the memory allocation subsystem.
*/
صحیح sqlite3MallocInit(خالی){
  صحیح rc;
  اگر( sqlite3GlobalConfig.m.xMalloc==0 ){
    sqlite3_Mem_سیٹ_Default();
  }
  mem0.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM);
  اگر( sqlite3GlobalConfig.p_صفحہ==0 || sqlite3GlobalConfig.sz_صفحہ<512
      || sqlite3GlobalConfig.n_صفحہ<=0 ){
    sqlite3GlobalConfig.p_صفحہ = 0;
    sqlite3GlobalConfig.sz_صفحہ = 0;
  }
  rc = sqlite3GlobalConfig.m.xInit(sqlite3GlobalConfig.m.p_App_ڈیٹا);
  اگر( rc!=SQLITE_OK ) memset(&mem0, 0, حجم(mem0));
  واپس rc;
}

/*
** Return true if the heap is currently under memory pressure - in other
** words if the amount of heap used is close to the limit set by
** sqlite3_soft_heap_limit().
*/
صحیح sqlite3_ڈھیر_Nearly_بھرا(خالی){
  واپس Atomic_بھرو(&mem0.nearly_بھرا);
}

/*
** Deinitialize the memory allocation subsystem.
*/
خالی sqlite3_Malloc_انتہا(خالی){
  اگر( sqlite3GlobalConfig.m.xShutdown ){
    sqlite3GlobalConfig.m.xShutdown(sqlite3GlobalConfig.m.p_App_ڈیٹا);
  }
  memset(&mem0, 0, حجم(mem0));
}

/*
** Return the amount of memory currently checked out.
*/
sqlite3_int64 sqlite3_حافظہ_مستعمل(خالی){
  sqlite3_int64 res, mx;
  sqlite3_status64(SQLITE_حالت_حافظہ_مستعمل, &res, &mx, 0);
  واپس res;
}

/*
** Return the maximum amount of memory that has ever been
** checked out since either the beginning of this process
** or since the most recent reset.
*/
sqlite3_int64 sqlite3_حافظہ_highwater(صحیح resetFlag){
  sqlite3_int64 res, mx;
  sqlite3_status64(SQLITE_حالت_حافظہ_مستعمل, &res, &mx, resetFlag);
  واپس mx;
}

/*
** Trigger the alarm 
*/
ساکن خالی sqlite3MallocAlarm(صحیح nByte){
  اگر( mem0.alarm_حد<=0 ) واپس;
  sqlite3_mutex_leave(mem0.mutex);
  sqlite3_release_حافظہ(nByte);
  sqlite3_mutex_enter(mem0.mutex);
}

#اگر_تعریف_شدہ SQLITE_DEBUG
/*
** This routine is called whenever an out-of-memory condition is seen,
** It's only purpose to to serve as a breakpoint for gdb or similar
** code debuggers when working on out-of-memory conditions, for example
** caused by PRAGMA hard_heap_limit=N.
*/
ساکن SQLITE_NOINLINE خالی test_oom_breakpoint(u64 n){
  ساکن u64 nOomFault = 0;
  nOomFault += n;
  /* The assert() is never reached in a human lifetime.  It  is here mostly
  ** to prevent code optimizers from optimizing out this function. */
  أكد( (nOomFault>>32) < 0xffffffff );
}
#ورنہ
# define test_oom_breakpoint(X)   /* No-op for production builds */
#اگر_ختم

/*
** Do a memory allocation with statistics and alarms.  Assume the
** lock is already held.
*/
ساکن خالی mallocWithAlarm(صحیح n, خالی **pp){
  خالی *p;
  صحیح n_بھرا;
  أكد( sqlite3_mutex_held(mem0.mutex) );
  أكد( n>0 );

  /* In Firefox (circa 2017-02-08), xRoundup() is remapped to an internal
  ** implementation of malloc_good_size(), which must be called in debug
  ** mode and specifically when the DMD "Dark Matter Detector" is enabled
  ** or else a crash results.  Hence, do not attempt to optimize out the
  ** following xRoundup() call. */
  n_بھرا = sqlite3GlobalConfig.m.xRoundup(n);

  sqlite3_حالت_Highwater(SQLITE_حالت_MALLOC_سائز, n);
  اگر( mem0.alarm_حد>0 ){
    sqlite3_int64 n_مستعمل = sqlite3_حالت_قدر(SQLITE_حالت_حافظہ_مستعمل);
    اگر( n_مستعمل >= mem0.alarm_حد - n_بھرا ){
      AtomicStore(&mem0.nearly_بھرا, 1);
      sqlite3MallocAlarm(n_بھرا);
      اگر( mem0.hard_حد_بندی ){
        n_مستعمل = sqlite3_حالت_قدر(SQLITE_حالت_حافظہ_مستعمل);
        اگر( n_مستعمل >= mem0.hard_حد_بندی - n_بھرا ){
          test_oom_breakpoint(1);
          *pp = 0;
          واپس;
        }
      }
    }ورنہ{
      AtomicStore(&mem0.nearly_بھرا, 0);
    }
  }
  p = sqlite3GlobalConfig.m.xMalloc(n_بھرا);
#اگر_تعریف_شدہ SQLITE_ENABLE_حافظہ_MANAGEMENT
  اگر( p==0 && mem0.alarm_حد>0 ){
    sqlite3MallocAlarm(n_بھرا);
    p = sqlite3GlobalConfig.m.xMalloc(n_بھرا);
  }
#اگر_ختم
  اگر( p ){
    n_بھرا = sqlite3_Malloc_سائز(p);
    sqlite3_حالت_اوپر(SQLITE_حالت_حافظہ_مستعمل, n_بھرا);
    sqlite3_حالت_اوپر(SQLITE_حالت_MALLOC_گنتی, 1);
  }
  *pp = p;
}

/*
** Allocate memory.  This routine is like sqlite3_malloc() except that it
** assumes the memory subsystem has already been initialized.
*/
خالی *sqlite3Malloc(u64 n){
  خالی *p;
  اگر( n==0 || n>SQLITE_MAX_تخصیص_سائز ){
    p = 0;
  }ورنہ اگر( sqlite3GlobalConfig.bMemstat ){
    sqlite3_mutex_enter(mem0.mutex);
    mallocWithAlarm((صحیح)n, &p);
    sqlite3_mutex_leave(mem0.mutex);
  }ورنہ{
    p = sqlite3GlobalConfig.m.xMalloc((صحیح)n);
  }
  أكد( EIGHT_BYTE_ALIGNMENT(p) );  /* IMP: R-11148-40995 */
  واپس p;
}

/*
** This version of the memory allocation is for use by the application.
** First make sure the memory subsystem is initialized, then do the
** allocation.
*/
خالی *sqlite3_malloc(صحیح n){
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOINIT
  اگر( sqlite3_initialize() ) واپس 0;
#اگر_ختم
  واپس n<=0 ? 0 : sqlite3Malloc(n);
}
خالی *sqlite3_malloc64(sqlite3_uint64 n){
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOINIT
  اگر( sqlite3_initialize() ) واپس 0;
#اگر_ختم
  واپس sqlite3Malloc(n);
}

/*
** TRUE if p is a lookaside memory allocation from db
*/
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_LOOKASIDE
ساکن صحیح isLookaside(sqlite3 *db, ثابت خالی *p){
  واپس SQLITE_WITHIN(p, db->lookaside.p_آغاز, db->lookaside.p_True_انتہا);
}
#ورنہ
#تعریف_کرو isLookaside(A,B) 0
#اگر_ختم

/*
** Return the size of a memory allocation previously obtained from
** sqlite3Malloc() or sqlite3_malloc().
*/
صحیح sqlite3_Malloc_سائز(ثابت خالی *p){
  أكد( sqlite3MemdebugHasType(p, MEMTYPE_ڈھیر) );
  واپس sqlite3GlobalConfig.m.x_سائز((خالی*)p);
}
ساکن صحیح lookaside_Malloc_سائز(sqlite3 *db, ثابت خالی *p){
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_TWOSIZE_LOOKASIDE    
  واپس p<db->lookaside.p_درمیان ? db->lookaside.szTrue : LOOKASIDE_SMALL;
#ورنہ
  واپس db->lookaside.szTrue;
#اگر_ختم  
}
صحیح sqlite3_Db_Malloc_سائز(sqlite3 *db, ثابت خالی *p){
  أكد( p!=0 );
#اگر_تعریف_شدہ SQLITE_DEBUG
  اگر( db==0 ){
    أكد( sqlite3MemdebugNoType(p, (u8)~MEMTYPE_ڈھیر) );
    أكد( sqlite3MemdebugHasType(p, MEMTYPE_ڈھیر) );
  }ورنہ اگر( !isLookaside(db,p) ){
    أكد( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_ڈھیر)) );
    أكد( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_ڈھیر)) );
  }
#اگر_ختم
  اگر( db ){
    اگر( ((uptr)p)<(uptr)(db->lookaside.p_True_انتہا) ){
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_TWOSIZE_LOOKASIDE
      اگر( ((uptr)p)>=(uptr)(db->lookaside.p_درمیان) ){
        أكد( sqlite3_mutex_held(db->mutex) );
        واپس LOOKASIDE_SMALL;
      }
#اگر_ختم
      اگر( ((uptr)p)>=(uptr)(db->lookaside.p_آغاز) ){
        أكد( sqlite3_mutex_held(db->mutex) );
        واپس db->lookaside.szTrue;
      }
    }
  }
  واپس sqlite3GlobalConfig.m.x_سائز((خالی*)p);
}
sqlite3_uint64 sqlite3_msize(خالی *p){
  أكد( sqlite3MemdebugNoType(p, (u8)~MEMTYPE_ڈھیر) );
  أكد( sqlite3MemdebugHasType(p, MEMTYPE_ڈھیر) );
  واپس p ? sqlite3GlobalConfig.m.x_سائز(p) : 0;
}

/*
** Free memory previously obtained from sqlite3Malloc().
*/
خالی sqlite3_free(خالی *p){
  اگر( p==0 ) واپس;  /* IMP: R-49053-54554 */
  أكد( sqlite3MemdebugHasType(p, MEMTYPE_ڈھیر) );
  أكد( sqlite3MemdebugNoType(p, (u8)~MEMTYPE_ڈھیر) );
  اگر( sqlite3GlobalConfig.bMemstat ){
    sqlite3_mutex_enter(mem0.mutex);
    sqlite3_حالت_نیچے_کو(SQLITE_حالت_حافظہ_مستعمل, sqlite3_Malloc_سائز(p));
    sqlite3_حالت_نیچے_کو(SQLITE_حالت_MALLOC_گنتی, 1);
    sqlite3GlobalConfig.m.xFree(p);
    sqlite3_mutex_leave(mem0.mutex);
  }ورنہ{
    sqlite3GlobalConfig.m.xFree(p);
  }
}

/*
** Add the size of memory allocation "p" to the count in
** *db->pnBytesFreed.
*/
ساکن SQLITE_NOINLINE خالی measure_تخصیص_سائز(sqlite3 *db, خالی *p){
  *db->pn_بائٹیں_Freed += sqlite3_Db_Malloc_سائز(db,p);
}

/*
** Free memory that might be associated with a particular database
** connection.  Calling sqlite3DbFree(D,X) for X==0 is a harmless no-op.
** The sqlite3DbFreeNN(D,X) version requires that X be non-NULL.
*/
خالی sqlite3DbFreeNN(sqlite3 *db, خالی *p){
  أكد( db==0 || sqlite3_mutex_held(db->mutex) );
  أكد( p!=0 );
  اگر( db ){
    اگر( ((uptr)p)<(uptr)(db->lookaside.p_انتہا) ){
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_TWOSIZE_LOOKASIDE
      اگر( ((uptr)p)>=(uptr)(db->lookaside.p_درمیان) ){
        Lookaside_خانہ *pBuf = (Lookaside_خانہ*)p;
        أكد( db->pn_بائٹیں_Freed==0 );
#اگر_تعریف_شدہ SQLITE_DEBUG
        memset(p, 0xaa, LOOKASIDE_SMALL);  /* Trash freed content */
#اگر_ختم
        pBuf->p_اگلا = db->lookaside.pSmallFree;
        db->lookaside.pSmallFree = pBuf;
        واپس;
      }
#اگر_ختم /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
      اگر( ((uptr)p)>=(uptr)(db->lookaside.p_آغاز) ){
        Lookaside_خانہ *pBuf = (Lookaside_خانہ*)p;
        أكد( db->pn_بائٹیں_Freed==0 );
#اگر_تعریف_شدہ SQLITE_DEBUG
        memset(p, 0xaa, db->lookaside.szTrue);  /* Trash freed content */
#اگر_ختم
        pBuf->p_اگلا = db->lookaside.pFree;
        db->lookaside.pFree = pBuf;
        واپس;
      }
    }
    اگر( db->pn_بائٹیں_Freed ){
      measure_تخصیص_سائز(db, p);
      واپس;
    }
  }
  أكد( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_ڈھیر)) );
  أكد( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_ڈھیر)) );
  أكد( db!=0 || sqlite3MemdebugNoType(p, MEMTYPE_LOOKASIDE) );
  sqlite3_Memdebug_سیٹ_Type(p, MEMTYPE_ڈھیر);
  sqlite3_free(p);
}
خالی sqlite3DbNNFreeNN(sqlite3 *db, خالی *p){
  أكد( db!=0 );
  أكد( sqlite3_mutex_held(db->mutex) );
  أكد( p!=0 );
  اگر( ((uptr)p)<(uptr)(db->lookaside.p_انتہا) ){
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_TWOSIZE_LOOKASIDE
    اگر( ((uptr)p)>=(uptr)(db->lookaside.p_درمیان) ){
      Lookaside_خانہ *pBuf = (Lookaside_خانہ*)p;
      أكد( db->pn_بائٹیں_Freed==0 );
#اگر_تعریف_شدہ SQLITE_DEBUG
      memset(p, 0xaa, LOOKASIDE_SMALL);  /* Trash freed content */
#اگر_ختم
      pBuf->p_اگلا = db->lookaside.pSmallFree;
      db->lookaside.pSmallFree = pBuf;
      واپس;
    }
#اگر_ختم /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
    اگر( ((uptr)p)>=(uptr)(db->lookaside.p_آغاز) ){
      Lookaside_خانہ *pBuf = (Lookaside_خانہ*)p;
      أكد( db->pn_بائٹیں_Freed==0 );
#اگر_تعریف_شدہ SQLITE_DEBUG
      memset(p, 0xaa, db->lookaside.szTrue);  /* Trash freed content */
#اگر_ختم
      pBuf->p_اگلا = db->lookaside.pFree;
      db->lookaside.pFree = pBuf;
      واپس;
    }
  }
  اگر( db->pn_بائٹیں_Freed ){
    measure_تخصیص_سائز(db, p);
    واپس;
  }
  أكد( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_ڈھیر)) );
  أكد( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_ڈھیر)) );
  sqlite3_Memdebug_سیٹ_Type(p, MEMTYPE_ڈھیر);
  sqlite3_free(p);
}
خالی sqlite3DbFree(sqlite3 *db, خالی *p){
  أكد( db==0 || sqlite3_mutex_held(db->mutex) );
  اگر( p ) sqlite3DbFreeNN(db, p);
}

/*
** Change the size of an existing memory allocation
*/
خالی *sqlite3Realloc(خالی *pOld, u64 n_بائٹیں){
  صحیح nOld, nNew, nDiff;
  خالی *pNew;
  أكد( sqlite3MemdebugHasType(pOld, MEMTYPE_ڈھیر) );
  أكد( sqlite3MemdebugNoType(pOld, (u8)~MEMTYPE_ڈھیر) );
  اگر( pOld==0 ){
    واپس sqlite3Malloc(n_بائٹیں); /* IMP: R-04300-56712 */
  }
  اگر( n_بائٹیں==0 ){
    sqlite3_free(pOld); /* IMP: R-26507-47431 */
    واپس 0;
  }
  اگر( n_بائٹیں>SQLITE_MAX_تخصیص_سائز ){
    واپس 0;
  }
  nOld = sqlite3_Malloc_سائز(pOld);
  /* IMPLEMENTATION-OF: R-46199-30249 SQLite guarantees that the second
  ** argument to xRealloc is always a value returned by a prior call to
  ** xRoundup. */
  nNew = sqlite3GlobalConfig.m.xRoundup((صحیح)n_بائٹیں);
  اگر( nOld==nNew ){
    pNew = pOld;
  }ورنہ اگر( sqlite3GlobalConfig.bMemstat ){
    sqlite3_int64 n_مستعمل;
    sqlite3_mutex_enter(mem0.mutex);
    sqlite3_حالت_Highwater(SQLITE_حالت_MALLOC_سائز, (صحیح)n_بائٹیں);
    nDiff = nNew - nOld;
    اگر( nDiff>0 && (n_مستعمل = sqlite3_حالت_قدر(SQLITE_حالت_حافظہ_مستعمل)) >= 
          mem0.alarm_حد-nDiff ){
      sqlite3MallocAlarm(nDiff);
      اگر( mem0.hard_حد_بندی>0 && n_مستعمل >= mem0.hard_حد_بندی - nDiff ){
        sqlite3_mutex_leave(mem0.mutex);
        test_oom_breakpoint(1);
        واپس 0;
      }
    }
    pNew = sqlite3GlobalConfig.m.xRealloc(pOld, nNew);
#اگر_تعریف_شدہ SQLITE_ENABLE_حافظہ_MANAGEMENT
    اگر( pNew==0 && mem0.alarm_حد>0 ){
      sqlite3MallocAlarm((صحیح)n_بائٹیں);
      pNew = sqlite3GlobalConfig.m.xRealloc(pOld, nNew);
    }
#اگر_ختم
    اگر( pNew ){
      nNew = sqlite3_Malloc_سائز(pNew);
      sqlite3_حالت_اوپر(SQLITE_حالت_حافظہ_مستعمل, nNew-nOld);
    }
    sqlite3_mutex_leave(mem0.mutex);
  }ورنہ{
    pNew = sqlite3GlobalConfig.m.xRealloc(pOld, nNew);
  }
  أكد( EIGHT_BYTE_ALIGNMENT(pNew) ); /* IMP: R-11148-40995 */
  واپس pNew;
}

/*
** The public interface to sqlite3Realloc.  Make sure that the memory
** subsystem is initialized prior to invoking sqliteRealloc.
*/
خالی *sqlite3_realloc(خالی *pOld, صحیح n){
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOINIT
  اگر( sqlite3_initialize() ) واپس 0;
#اگر_ختم
  اگر( n<0 ) n = 0;  /* IMP: R-26507-47431 */
  واپس sqlite3Realloc(pOld, n);
}
خالی *sqlite3_realloc64(خالی *pOld, sqlite3_uint64 n){
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOINIT
  اگر( sqlite3_initialize() ) واپس 0;
#اگر_ختم
  واپس sqlite3Realloc(pOld, n);
}


/*
** Allocate and zero memory.
*/ 
خالی *sqlite3MallocZero(u64 n){
  خالی *p = sqlite3Malloc(n);
  اگر( p ){
    memset(p, 0, (size_t)n);
  }
  واپس p;
}

/*
** Allocate and zero memory.  If the allocation fails, make
** the mallocFailed flag in the connection pointer.
*/
خالی *sqlite3DbMallocZero(sqlite3 *db, u64 n){
  خالی *p;
  testcase( db==0 );
  p = sqlite3DbMallocRaw(db, n);
  اگر( p ) memset(p, 0, (size_t)n);
  واپس p;
}


/* Finish the work of sqlite3DbMallocRawNN for the unusual and
** slower case when the allocation cannot be fulfilled using lookaside.
*/
ساکن SQLITE_NOINLINE خالی *db_Malloc_Raw_اختتام(sqlite3 *db, u64 n){
  خالی *p;
  أكد( db!=0 );
  p = sqlite3Malloc(n);
  اگر( !p ) sqlite3OomFault(db);
  sqlite3_Memdebug_سیٹ_Type(p, 
         (db->lookaside.bDisable==0) ? MEMTYPE_LOOKASIDE : MEMTYPE_ڈھیر);
  واپس p;
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
خالی *sqlite3DbMallocRaw(sqlite3 *db, u64 n){
  خالی *p;
  اگر( db ) واپس sqlite3DbMallocRawNN(db, n);
  p = sqlite3Malloc(n);
  sqlite3_Memdebug_سیٹ_Type(p, MEMTYPE_ڈھیر);
  واپس p;
}
خالی *sqlite3DbMallocRawNN(sqlite3 *db, u64 n){
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_LOOKASIDE
  Lookaside_خانہ *pBuf;
  أكد( db!=0 );
  أكد( sqlite3_mutex_held(db->mutex) );
  أكد( db->pn_بائٹیں_Freed==0 );
  اگر( n>db->lookaside.sz ){
    اگر( !db->lookaside.bDisable ){
      db->lookaside.anStat[1]++;      
    }ورنہ اگر( db->mallocFailed ){
      واپس 0;
    }
    واپس db_Malloc_Raw_اختتام(db, n);
  }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_TWOSIZE_LOOKASIDE
  اگر( n<=LOOKASIDE_SMALL ){
    اگر( (pBuf = db->lookaside.pSmallFree)!=0 ){
      db->lookaside.pSmallFree = pBuf->p_اگلا;
      db->lookaside.anStat[0]++;
      واپس (خالی*)pBuf;
    }ورنہ اگر( (pBuf = db->lookaside.pSmallInit)!=0 ){
      db->lookaside.pSmallInit = pBuf->p_اگلا;
      db->lookaside.anStat[0]++;
      واپس (خالی*)pBuf;
    }
  }
#اگر_ختم
  اگر( (pBuf = db->lookaside.pFree)!=0 ){
    db->lookaside.pFree = pBuf->p_اگلا;
    db->lookaside.anStat[0]++;
    واپس (خالی*)pBuf;
  }ورنہ اگر( (pBuf = db->lookaside.pInit)!=0 ){
    db->lookaside.pInit = pBuf->p_اگلا;
    db->lookaside.anStat[0]++;
    واپس (خالی*)pBuf;
  }ورنہ{
    db->lookaside.anStat[2]++;
  }
#ورنہ
  أكد( db!=0 );
  أكد( sqlite3_mutex_held(db->mutex) );
  أكد( db->pn_بائٹیں_Freed==0 );
  اگر( db->mallocFailed ){
    واپس 0;
  }
#اگر_ختم
  واپس db_Malloc_Raw_اختتام(db, n);
}

/* Forward declaration */
ساکن SQLITE_NOINLINE خالی *db_Realloc_اختتام(sqlite3 *db, خالی *p, u64 n);

/*
** Resize the block of memory pointed to by p to n bytes. If the
** resize fails, set the mallocFailed flag in the connection object.
*/
خالی *sqlite3DbRealloc(sqlite3 *db, خالی *p, u64 n){
  أكد( db!=0 );
  اگر( p==0 ) واپس sqlite3DbMallocRawNN(db, n);
  أكد( sqlite3_mutex_held(db->mutex) );
  اگر( ((uptr)p)<(uptr)db->lookaside.p_انتہا ){
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_TWOSIZE_LOOKASIDE
    اگر( ((uptr)p)>=(uptr)db->lookaside.p_درمیان ){
      اگر( n<=LOOKASIDE_SMALL ) واپس p;
    }ورنہ
#اگر_ختم
    اگر( ((uptr)p)>=(uptr)db->lookaside.p_آغاز ){
      اگر( n<=db->lookaside.szTrue ) واپس p;
    }
  }
  واپس db_Realloc_اختتام(db, p, n);
}
ساکن SQLITE_NOINLINE خالی *db_Realloc_اختتام(sqlite3 *db, خالی *p, u64 n){
  خالی *pNew = 0;
  أكد( db!=0 );
  أكد( p!=0 );
  اگر( db->mallocFailed==0 ){
    اگر( isLookaside(db, p) ){
      pNew = sqlite3DbMallocRawNN(db, n);
      اگر( pNew ){
        حافظہ_نقل(pNew, p, lookaside_Malloc_سائز(db, p));
        sqlite3DbFree(db, p);
      }
    }ورنہ{
      أكد( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_ڈھیر)) );
      أكد( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_ڈھیر)) );
      sqlite3_Memdebug_سیٹ_Type(p, MEMTYPE_ڈھیر);
      pNew = sqlite3Realloc(p, n);
      اگر( !pNew ){
        sqlite3OomFault(db);
      }
      sqlite3_Memdebug_سیٹ_Type(pNew,
            (db->lookaside.bDisable==0 ? MEMTYPE_LOOKASIDE : MEMTYPE_ڈھیر));
    }
  }
  واپس pNew;
}

/*
** Attempt to reallocate p.  If the reallocation fails, then free p
** and set the mallocFailed flag in the database connection.
*/
خالی *sqlite3DbReallocOrFree(sqlite3 *db, خالی *p, u64 n){
  خالی *pNew;
  pNew = sqlite3DbRealloc(db, p, n);
  اگر( !pNew ){
    sqlite3DbFree(db, p);
  }
  واپس pNew;
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
  اگر( z==0 ){
    واپس 0;
  }
  n = لڑی_لمبائی(z) + 1;
  zNew = sqlite3DbMallocRaw(db, n);
  اگر( zNew ){
    حافظہ_نقل(zNew, z, n);
  }
  واپس zNew;
}
حرف *sqlite3DbStrNDup(sqlite3 *db, ثابت حرف *z, u64 n){
  حرف *zNew;
  أكد( db!=0 );
  أكد( z!=0 || n==0 );
  أكد( (n&0x7fffffff)==n );
  zNew = z ? sqlite3DbMallocRawNN(db, n+1) : 0;
  اگر( zNew ){
    حافظہ_نقل(zNew, z, (size_t)n);
    zNew[n] = 0;
  }
  واپس zNew;
}

/*
** The text between zStart and zEnd represents a phrase within a larger
** SQL statement.  Make a copy of this phrase in space obtained form
** sqlite3DbMalloc().  Omit leading and trailing whitespace.
*/
حرف *sqlite3_Db_وسعت_Dup(sqlite3 *db, ثابت حرف *z_آغاز, ثابت حرف *z_انتہا){
  صحیح n;
#اگر_تعریف_شدہ SQLITE_DEBUG
  /* Because of the way the parser works, the span is guaranteed to contain
  ** at least one non-space character */
  برائے(n=0; sqlite3Isspace(z_آغاز[n]); n++){ أكد( &z_آغاز[n]<z_انتہا ); }
#اگر_ختم
  جب( sqlite3Isspace(z_آغاز[0]) ) z_آغاز++;
  n = (صحیح)(z_انتہا - z_آغاز);
  جب( sqlite3Isspace(z_آغاز[n-1]) ) n--;
  واپس sqlite3DbStrNDup(db, z_آغاز, n);
}

/*
** Free any prior content in *pz and replace it with a copy of zNew.
*/
خالی sqlite3_سیٹ_String(حرف **pz, sqlite3 *db, ثابت حرف *zNew){
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
خالی *sqlite3OomFault(sqlite3 *db){
  اگر( db->mallocFailed==0 && db->bBenignMalloc==0 ){
    db->mallocFailed = 1;
    اگر( db->nVdbeExec>0 ){
      AtomicStore(&db->u1.isInterrupted, 1);
    }
    DisableLookaside;
    اگر( db->pParse ){
      Parse *pParse;
      sqlite3_Error_پیغام(db->pParse, "out of memory");
      db->pParse->rc = SQLITE_NOMEM_BKPT;
      برائے(pParse=db->pParse->pOuterParse; pParse; pParse = pParse->pOuterParse){
        pParse->nErr++;
        pParse->rc = SQLITE_NOMEM;
      } 
    }
  }
  واپس 0;
}

/*
** This routine reactivates the memory allocator and clears the
** db->mallocFailed flag as necessary.
**
** The memory allocator is not restarted if there are running
** VDBEs.
*/
خالی sqlite3OomClear(sqlite3 *db){
  اگر( db->mallocFailed && db->nVdbeExec==0 ){
    db->mallocFailed = 0;
    AtomicStore(&db->u1.isInterrupted, 0);
    أكد( db->lookaside.bDisable>0 );
    EnableLookaside;
  }
}

/*
** Take actions at the end of an API call to deal with error codes.
*/
ساکن SQLITE_NOINLINE صحیح apiHandleError(sqlite3 *db, صحیح rc){
  اگر( db->mallocFailed || rc==SQLITE_IOERR_NOMEM ){
    sqlite3OomClear(db);
    sqlite3Error(db, SQLITE_NOMEM);
    واپس SQLITE_NOMEM_BKPT;
  }
  واپس rc & db->err_نقاب;
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
صحیح sqlite3ApiExit(sqlite3* db, صحیح rc){
  /* If the db handle must hold the connection handle mutex here.
  ** Otherwise the read (and possible write) of db->mallocFailed 
  ** is unsafe, as is the call to sqlite3Error().
  */
  أكد( db!=0 );
  أكد( sqlite3_mutex_held(db->mutex) );
  اگر( db->mallocFailed || rc ){
    واپس apiHandleError(db, rc);
  }
  واپس 0;
}
