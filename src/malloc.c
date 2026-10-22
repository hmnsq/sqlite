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
صحيح sqlite3_release_حافظه(صحيح n){
#اگر_تعریف_شده SQLITE_ENABLE_حافظه_MANAGEMENT
  أرجع sqlite3_Pcache_Release_حافظه(n);
#وإلا
  /* IMPLEMENTATION-OF: R-34391-24921 The sqlite3_release_memory() routine
  ** is a no-op returning zero if SQLite is not compiled with
  ** SQLITE_ENABLE_MEMORY_MANAGEMENT. */
  استفادهنشده_PARAMETER(n);
  أرجع 0;
#پایان_اگر
}

/*
** Default value of the hard heap limit.  0 means "no limit".
*/
#اگر_تعریف_نشده SQLITE_MAX_حافظه
# define SQLITE_MAX_حافظه 0
#پایان_اگر

/*
** State information local to the memory allocation subsystem.
*/
ساكن SQLITE_WSD بنية Mem0Global {
  sqlite3_mutex *mutex;         /* Mutex to serialize access */
  sqlite3_int64 alarm_آستانه; /* The soft heap limit */
  sqlite3_int64 hard_حد;      /* The hard upper bound on memory */

  /*
  ** True if heap is nearly "full" where "full" is defined by the
  ** sqlite3_soft_heap_limit() setting.
  */
  صحيح nearly_پر;
} mem0 = { 0, SQLITE_MAX_حافظه, SQLITE_MAX_حافظه, 0 };

#تعریف_کن mem0 GLOBAL(بنية Mem0Global, mem0)

/*
** Return the memory allocator mutex. sqlite3_status() needs it.
*/
sqlite3_mutex *sqlite3MallocMutex(فراغ){
  أرجع mem0.mutex;
}

#اگر_تعریف_نشده SQLITE_OMIT_DEPRECATED
/*
** Deprecated external interface.  It used to set an alarm callback
** that was invoked when memory usage grew too large.  Now it is a
** no-op.
*/
صحيح sqlite3_حافظه_alarm(
  فراغ(*xCallback)(فراغ *pArg, sqlite3_int64 استفاده_شده,صحيح N),
  فراغ *pArg,
  sqlite3_int64 i_آستانه
){
  (فراغ)xCallback;
  (فراغ)pArg;
  (فراغ)i_آستانه;
  أرجع SQLITE_OK;
}
#پایان_اگر

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
sqlite3_int64 sqlite3_soft_انباشته_limit64(sqlite3_int64 n){
  sqlite3_int64 پیشین_حد;
  sqlite3_int64 excess;
  sqlite3_int64 n_استفاده_شده;
#اگر_تعریف_نشده SQLITE_OMIT_AUTOINIT
  صحيح rc = sqlite3_initialize();
  إذا( rc ) أرجع -1;
#پایان_اگر
  sqlite3_mutex_enter(mem0.mutex);
  پیشین_حد = mem0.alarm_آستانه;
  إذا( n<0 ){
    sqlite3_mutex_leave(mem0.mutex);
    أرجع پیشین_حد;
  }
  إذا( mem0.hard_حد>0 && (n>mem0.hard_حد || n==0) ){
    n = mem0.hard_حد;
  }
  mem0.alarm_آستانه = n;
  n_استفاده_شده = sqlite3_وضعیت_مقدار(SQLITE_وضعیت_حافظه_استفاده_شده);
  AtomicStore(&mem0.nearly_پر, n>0 && n<=n_استفاده_شده);
  sqlite3_mutex_leave(mem0.mutex);
  excess = sqlite3_حافظه_استفاده_شده() - n;
  إذا( excess>0 ) sqlite3_release_حافظه((صحيح)(excess & 0x7fffffff));
  أرجع پیشین_حد;
}
فراغ sqlite3_soft_انباشته_حد(صحيح n){
  إذا( n<0 ) n = 0;
  sqlite3_soft_انباشته_limit64(n);
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
sqlite3_int64 sqlite3_hard_انباشته_limit64(sqlite3_int64 n){
  sqlite3_int64 پیشین_حد;
#اگر_تعریف_نشده SQLITE_OMIT_AUTOINIT
  صحيح rc = sqlite3_initialize();
  إذا( rc ) أرجع -1;
#پایان_اگر
  sqlite3_mutex_enter(mem0.mutex);
  پیشین_حد = mem0.hard_حد;
  إذا( n>=0 ){
    mem0.hard_حد = n;
    إذا( n<mem0.alarm_آستانه || mem0.alarm_آستانه==0 ){
      mem0.alarm_آستانه = n;
    }
  }
  sqlite3_mutex_leave(mem0.mutex);
  أرجع پیشین_حد;
}


/*
** Initialize the memory allocation subsystem.
*/
صحيح sqlite3MallocInit(فراغ){
  صحيح rc;
  إذا( sqlite3GlobalConfig.m.xMalloc==0 ){
    sqlite3_Mem_مجموعه_Default();
  }
  mem0.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM);
  إذا( sqlite3GlobalConfig.p_صفحه==0 || sqlite3GlobalConfig.sz_صفحه<512
      || sqlite3GlobalConfig.n_صفحه<=0 ){
    sqlite3GlobalConfig.p_صفحه = 0;
    sqlite3GlobalConfig.sz_صفحه = 0;
  }
  rc = sqlite3GlobalConfig.m.xInit(sqlite3GlobalConfig.m.p_App_داده);
  إذا( rc!=SQLITE_OK ) memset(&mem0, 0, حجم(mem0));
  أرجع rc;
}

/*
** Return true if the heap is currently under memory pressure - in other
** words if the amount of heap used is close to the limit set by
** sqlite3_soft_heap_limit().
*/
صحيح sqlite3_انباشته_Nearly_پر(فراغ){
  أرجع Atomic_بارگیری(&mem0.nearly_پر);
}

/*
** Deinitialize the memory allocation subsystem.
*/
فراغ sqlite3_Malloc_انتها(فراغ){
  إذا( sqlite3GlobalConfig.m.xShutdown ){
    sqlite3GlobalConfig.m.xShutdown(sqlite3GlobalConfig.m.p_App_داده);
  }
  memset(&mem0, 0, حجم(mem0));
}

/*
** Return the amount of memory currently checked out.
*/
sqlite3_int64 sqlite3_حافظه_استفاده_شده(فراغ){
  sqlite3_int64 res, mx;
  sqlite3_status64(SQLITE_وضعیت_حافظه_استفاده_شده, &res, &mx, 0);
  أرجع res;
}

/*
** Return the maximum amount of memory that has ever been
** checked out since either the beginning of this process
** or since the most recent reset.
*/
sqlite3_int64 sqlite3_حافظه_highwater(صحيح resetFlag){
  sqlite3_int64 res, mx;
  sqlite3_status64(SQLITE_وضعیت_حافظه_استفاده_شده, &res, &mx, resetFlag);
  أرجع mx;
}

/*
** Trigger the alarm 
*/
ساكن فراغ sqlite3MallocAlarm(صحيح nByte){
  إذا( mem0.alarm_آستانه<=0 ) أرجع;
  sqlite3_mutex_leave(mem0.mutex);
  sqlite3_release_حافظه(nByte);
  sqlite3_mutex_enter(mem0.mutex);
}

#اگر_تعریف_شده SQLITE_DEBUG
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
#پایان_اگر

/*
** Do a memory allocation with statistics and alarms.  Assume the
** lock is already held.
*/
ساكن فراغ mallocWithAlarm(صحيح n, فراغ **pp){
  فراغ *p;
  صحيح n_پر;
  أكد( sqlite3_mutex_held(mem0.mutex) );
  أكد( n>0 );

  /* In Firefox (circa 2017-02-08), xRoundup() is remapped to an internal
  ** implementation of malloc_good_size(), which must be called in debug
  ** mode and specifically when the DMD "Dark Matter Detector" is enabled
  ** or else a crash results.  Hence, do not attempt to optimize out the
  ** following xRoundup() call. */
  n_پر = sqlite3GlobalConfig.m.xRoundup(n);

  sqlite3_وضعیت_Highwater(SQLITE_وضعیت_MALLOC_اندازه, n);
  إذا( mem0.alarm_آستانه>0 ){
    sqlite3_int64 n_استفاده_شده = sqlite3_وضعیت_مقدار(SQLITE_وضعیت_حافظه_استفاده_شده);
    إذا( n_استفاده_شده >= mem0.alarm_آستانه - n_پر ){
      AtomicStore(&mem0.nearly_پر, 1);
      sqlite3MallocAlarm(n_پر);
      إذا( mem0.hard_حد ){
        n_استفاده_شده = sqlite3_وضعیت_مقدار(SQLITE_وضعیت_حافظه_استفاده_شده);
        إذا( n_استفاده_شده >= mem0.hard_حد - n_پر ){
          test_oom_breakpoint(1);
          *pp = 0;
          أرجع;
        }
      }
    }وإلا{
      AtomicStore(&mem0.nearly_پر, 0);
    }
  }
  p = sqlite3GlobalConfig.m.xMalloc(n_پر);
#اگر_تعریف_شده SQLITE_ENABLE_حافظه_MANAGEMENT
  إذا( p==0 && mem0.alarm_آستانه>0 ){
    sqlite3MallocAlarm(n_پر);
    p = sqlite3GlobalConfig.m.xMalloc(n_پر);
  }
#پایان_اگر
  إذا( p ){
    n_پر = sqlite3_Malloc_اندازه(p);
    sqlite3_وضعیت_به_بالا(SQLITE_وضعیت_حافظه_استفاده_شده, n_پر);
    sqlite3_وضعیت_به_بالا(SQLITE_وضعیت_MALLOC_شمار, 1);
  }
  *pp = p;
}

/*
** Allocate memory.  This routine is like sqlite3_malloc() except that it
** assumes the memory subsystem has already been initialized.
*/
فراغ *sqlite3Malloc(u64 n){
  فراغ *p;
  إذا( n==0 || n>SQLITE_MAX_تخصیص_اندازه ){
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
#اگر_تعریف_نشده SQLITE_OMIT_AUTOINIT
  إذا( sqlite3_initialize() ) أرجع 0;
#پایان_اگر
  أرجع n<=0 ? 0 : sqlite3Malloc(n);
}
فراغ *sqlite3_malloc64(sqlite3_uint64 n){
#اگر_تعریف_نشده SQLITE_OMIT_AUTOINIT
  إذا( sqlite3_initialize() ) أرجع 0;
#پایان_اگر
  أرجع sqlite3Malloc(n);
}

/*
** TRUE if p is a lookaside memory allocation from db
*/
#اگر_تعریف_نشده SQLITE_OMIT_LOOKASIDE
ساكن صحيح isLookaside(sqlite3 *db, ثابت فراغ *p){
  أرجع SQLITE_WITHIN(p, db->lookaside.p_شروع, db->lookaside.p_True_انتها);
}
#وإلا
#تعریف_کن isLookaside(A,B) 0
#پایان_اگر

/*
** Return the size of a memory allocation previously obtained from
** sqlite3Malloc() or sqlite3_malloc().
*/
صحيح sqlite3_Malloc_اندازه(ثابت فراغ *p){
  أكد( sqlite3MemdebugHasType(p, MEMTYPE_انباشته) );
  أرجع sqlite3GlobalConfig.m.x_اندازه((فراغ*)p);
}
ساكن صحيح lookaside_Malloc_اندازه(sqlite3 *db, ثابت فراغ *p){
#اگر_تعریف_نشده SQLITE_OMIT_TWOSIZE_LOOKASIDE    
  أرجع p<db->lookaside.p_میانه ? db->lookaside.szTrue : LOOKASIDE_SMALL;
#وإلا
  أرجع db->lookaside.szTrue;
#پایان_اگر  
}
صحيح sqlite3_Db_Malloc_اندازه(sqlite3 *db, ثابت فراغ *p){
  أكد( p!=0 );
#اگر_تعریف_شده SQLITE_DEBUG
  إذا( db==0 ){
    أكد( sqlite3MemdebugNoType(p, (u8)~MEMTYPE_انباشته) );
    أكد( sqlite3MemdebugHasType(p, MEMTYPE_انباشته) );
  }وإلا إذا( !isLookaside(db,p) ){
    أكد( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_انباشته)) );
    أكد( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_انباشته)) );
  }
#پایان_اگر
  إذا( db ){
    إذا( ((uptr)p)<(uptr)(db->lookaside.p_True_انتها) ){
#اگر_تعریف_نشده SQLITE_OMIT_TWOSIZE_LOOKASIDE
      إذا( ((uptr)p)>=(uptr)(db->lookaside.p_میانه) ){
        أكد( sqlite3_mutex_held(db->mutex) );
        أرجع LOOKASIDE_SMALL;
      }
#پایان_اگر
      إذا( ((uptr)p)>=(uptr)(db->lookaside.p_شروع) ){
        أكد( sqlite3_mutex_held(db->mutex) );
        أرجع db->lookaside.szTrue;
      }
    }
  }
  أرجع sqlite3GlobalConfig.m.x_اندازه((فراغ*)p);
}
sqlite3_uint64 sqlite3_msize(فراغ *p){
  أكد( sqlite3MemdebugNoType(p, (u8)~MEMTYPE_انباشته) );
  أكد( sqlite3MemdebugHasType(p, MEMTYPE_انباشته) );
  أرجع p ? sqlite3GlobalConfig.m.x_اندازه(p) : 0;
}

/*
** Free memory previously obtained from sqlite3Malloc().
*/
فراغ sqlite3_free(فراغ *p){
  إذا( p==0 ) أرجع;  /* IMP: R-49053-54554 */
  أكد( sqlite3MemdebugHasType(p, MEMTYPE_انباشته) );
  أكد( sqlite3MemdebugNoType(p, (u8)~MEMTYPE_انباشته) );
  إذا( sqlite3GlobalConfig.bMemstat ){
    sqlite3_mutex_enter(mem0.mutex);
    sqlite3_وضعیت_پایین(SQLITE_وضعیت_حافظه_استفاده_شده, sqlite3_Malloc_اندازه(p));
    sqlite3_وضعیت_پایین(SQLITE_وضعیت_MALLOC_شمار, 1);
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
ساكن SQLITE_NOINLINE فراغ measure_تخصیص_اندازه(sqlite3 *db, فراغ *p){
  *db->pn_بایتها_Freed += sqlite3_Db_Malloc_اندازه(db,p);
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
    إذا( ((uptr)p)<(uptr)(db->lookaside.p_انتها) ){
#اگر_تعریف_نشده SQLITE_OMIT_TWOSIZE_LOOKASIDE
      إذا( ((uptr)p)>=(uptr)(db->lookaside.p_میانه) ){
        Lookaside_جایگاه *pBuf = (Lookaside_جایگاه*)p;
        أكد( db->pn_بایتها_Freed==0 );
#اگر_تعریف_شده SQLITE_DEBUG
        memset(p, 0xaa, LOOKASIDE_SMALL);  /* Trash freed content */
#پایان_اگر
        pBuf->p_بعدی = db->lookaside.pSmallFree;
        db->lookaside.pSmallFree = pBuf;
        أرجع;
      }
#پایان_اگر /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
      إذا( ((uptr)p)>=(uptr)(db->lookaside.p_شروع) ){
        Lookaside_جایگاه *pBuf = (Lookaside_جایگاه*)p;
        أكد( db->pn_بایتها_Freed==0 );
#اگر_تعریف_شده SQLITE_DEBUG
        memset(p, 0xaa, db->lookaside.szTrue);  /* Trash freed content */
#پایان_اگر
        pBuf->p_بعدی = db->lookaside.pFree;
        db->lookaside.pFree = pBuf;
        أرجع;
      }
    }
    إذا( db->pn_بایتها_Freed ){
      measure_تخصیص_اندازه(db, p);
      أرجع;
    }
  }
  أكد( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_انباشته)) );
  أكد( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_انباشته)) );
  أكد( db!=0 || sqlite3MemdebugNoType(p, MEMTYPE_LOOKASIDE) );
  sqlite3_Memdebug_مجموعه_Type(p, MEMTYPE_انباشته);
  sqlite3_free(p);
}
فراغ sqlite3DbNNFreeNN(sqlite3 *db, فراغ *p){
  أكد( db!=0 );
  أكد( sqlite3_mutex_held(db->mutex) );
  أكد( p!=0 );
  إذا( ((uptr)p)<(uptr)(db->lookaside.p_انتها) ){
#اگر_تعریف_نشده SQLITE_OMIT_TWOSIZE_LOOKASIDE
    إذا( ((uptr)p)>=(uptr)(db->lookaside.p_میانه) ){
      Lookaside_جایگاه *pBuf = (Lookaside_جایگاه*)p;
      أكد( db->pn_بایتها_Freed==0 );
#اگر_تعریف_شده SQLITE_DEBUG
      memset(p, 0xaa, LOOKASIDE_SMALL);  /* Trash freed content */
#پایان_اگر
      pBuf->p_بعدی = db->lookaside.pSmallFree;
      db->lookaside.pSmallFree = pBuf;
      أرجع;
    }
#پایان_اگر /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
    إذا( ((uptr)p)>=(uptr)(db->lookaside.p_شروع) ){
      Lookaside_جایگاه *pBuf = (Lookaside_جایگاه*)p;
      أكد( db->pn_بایتها_Freed==0 );
#اگر_تعریف_شده SQLITE_DEBUG
      memset(p, 0xaa, db->lookaside.szTrue);  /* Trash freed content */
#پایان_اگر
      pBuf->p_بعدی = db->lookaside.pFree;
      db->lookaside.pFree = pBuf;
      أرجع;
    }
  }
  إذا( db->pn_بایتها_Freed ){
    measure_تخصیص_اندازه(db, p);
    أرجع;
  }
  أكد( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_انباشته)) );
  أكد( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_انباشته)) );
  sqlite3_Memdebug_مجموعه_Type(p, MEMTYPE_انباشته);
  sqlite3_free(p);
}
فراغ sqlite3DbFree(sqlite3 *db, فراغ *p){
  أكد( db==0 || sqlite3_mutex_held(db->mutex) );
  إذا( p ) sqlite3DbFreeNN(db, p);
}

/*
** Change the size of an existing memory allocation
*/
فراغ *sqlite3Realloc(فراغ *pOld, u64 n_بایتها){
  صحيح nOld, nNew, nDiff;
  فراغ *pNew;
  أكد( sqlite3MemdebugHasType(pOld, MEMTYPE_انباشته) );
  أكد( sqlite3MemdebugNoType(pOld, (u8)~MEMTYPE_انباشته) );
  إذا( pOld==0 ){
    أرجع sqlite3Malloc(n_بایتها); /* IMP: R-04300-56712 */
  }
  إذا( n_بایتها==0 ){
    sqlite3_free(pOld); /* IMP: R-26507-47431 */
    أرجع 0;
  }
  إذا( n_بایتها>SQLITE_MAX_تخصیص_اندازه ){
    أرجع 0;
  }
  nOld = sqlite3_Malloc_اندازه(pOld);
  /* IMPLEMENTATION-OF: R-46199-30249 SQLite guarantees that the second
  ** argument to xRealloc is always a value returned by a prior call to
  ** xRoundup. */
  nNew = sqlite3GlobalConfig.m.xRoundup((صحيح)n_بایتها);
  إذا( nOld==nNew ){
    pNew = pOld;
  }وإلا إذا( sqlite3GlobalConfig.bMemstat ){
    sqlite3_int64 n_استفاده_شده;
    sqlite3_mutex_enter(mem0.mutex);
    sqlite3_وضعیت_Highwater(SQLITE_وضعیت_MALLOC_اندازه, (صحيح)n_بایتها);
    nDiff = nNew - nOld;
    إذا( nDiff>0 && (n_استفاده_شده = sqlite3_وضعیت_مقدار(SQLITE_وضعیت_حافظه_استفاده_شده)) >= 
          mem0.alarm_آستانه-nDiff ){
      sqlite3MallocAlarm(nDiff);
      إذا( mem0.hard_حد>0 && n_استفاده_شده >= mem0.hard_حد - nDiff ){
        sqlite3_mutex_leave(mem0.mutex);
        test_oom_breakpoint(1);
        أرجع 0;
      }
    }
    pNew = sqlite3GlobalConfig.m.xRealloc(pOld, nNew);
#اگر_تعریف_شده SQLITE_ENABLE_حافظه_MANAGEMENT
    إذا( pNew==0 && mem0.alarm_آستانه>0 ){
      sqlite3MallocAlarm((صحيح)n_بایتها);
      pNew = sqlite3GlobalConfig.m.xRealloc(pOld, nNew);
    }
#پایان_اگر
    إذا( pNew ){
      nNew = sqlite3_Malloc_اندازه(pNew);
      sqlite3_وضعیت_به_بالا(SQLITE_وضعیت_حافظه_استفاده_شده, nNew-nOld);
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
#اگر_تعریف_نشده SQLITE_OMIT_AUTOINIT
  إذا( sqlite3_initialize() ) أرجع 0;
#پایان_اگر
  إذا( n<0 ) n = 0;  /* IMP: R-26507-47431 */
  أرجع sqlite3Realloc(pOld, n);
}
فراغ *sqlite3_realloc64(فراغ *pOld, sqlite3_uint64 n){
#اگر_تعریف_نشده SQLITE_OMIT_AUTOINIT
  إذا( sqlite3_initialize() ) أرجع 0;
#پایان_اگر
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
ساكن SQLITE_NOINLINE فراغ *db_Malloc_Raw_پایان(sqlite3 *db, u64 n){
  فراغ *p;
  أكد( db!=0 );
  p = sqlite3Malloc(n);
  إذا( !p ) sqlite3OomFault(db);
  sqlite3_Memdebug_مجموعه_Type(p, 
         (db->lookaside.bDisable==0) ? MEMTYPE_LOOKASIDE : MEMTYPE_انباشته);
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
  sqlite3_Memdebug_مجموعه_Type(p, MEMTYPE_انباشته);
  أرجع p;
}
فراغ *sqlite3DbMallocRawNN(sqlite3 *db, u64 n){
#اگر_تعریف_نشده SQLITE_OMIT_LOOKASIDE
  Lookaside_جایگاه *pBuf;
  أكد( db!=0 );
  أكد( sqlite3_mutex_held(db->mutex) );
  أكد( db->pn_بایتها_Freed==0 );
  إذا( n>db->lookaside.sz ){
    إذا( !db->lookaside.bDisable ){
      db->lookaside.anStat[1]++;      
    }وإلا إذا( db->mallocFailed ){
      أرجع 0;
    }
    أرجع db_Malloc_Raw_پایان(db, n);
  }
#اگر_تعریف_نشده SQLITE_OMIT_TWOSIZE_LOOKASIDE
  إذا( n<=LOOKASIDE_SMALL ){
    إذا( (pBuf = db->lookaside.pSmallFree)!=0 ){
      db->lookaside.pSmallFree = pBuf->p_بعدی;
      db->lookaside.anStat[0]++;
      أرجع (فراغ*)pBuf;
    }وإلا إذا( (pBuf = db->lookaside.pSmallInit)!=0 ){
      db->lookaside.pSmallInit = pBuf->p_بعدی;
      db->lookaside.anStat[0]++;
      أرجع (فراغ*)pBuf;
    }
  }
#پایان_اگر
  إذا( (pBuf = db->lookaside.pFree)!=0 ){
    db->lookaside.pFree = pBuf->p_بعدی;
    db->lookaside.anStat[0]++;
    أرجع (فراغ*)pBuf;
  }وإلا إذا( (pBuf = db->lookaside.pInit)!=0 ){
    db->lookaside.pInit = pBuf->p_بعدی;
    db->lookaside.anStat[0]++;
    أرجع (فراغ*)pBuf;
  }وإلا{
    db->lookaside.anStat[2]++;
  }
#وإلا
  أكد( db!=0 );
  أكد( sqlite3_mutex_held(db->mutex) );
  أكد( db->pn_بایتها_Freed==0 );
  إذا( db->mallocFailed ){
    أرجع 0;
  }
#پایان_اگر
  أرجع db_Malloc_Raw_پایان(db, n);
}

/* Forward declaration */
ساكن SQLITE_NOINLINE فراغ *db_Realloc_پایان(sqlite3 *db, فراغ *p, u64 n);

/*
** Resize the block of memory pointed to by p to n bytes. If the
** resize fails, set the mallocFailed flag in the connection object.
*/
فراغ *sqlite3DbRealloc(sqlite3 *db, فراغ *p, u64 n){
  أكد( db!=0 );
  إذا( p==0 ) أرجع sqlite3DbMallocRawNN(db, n);
  أكد( sqlite3_mutex_held(db->mutex) );
  إذا( ((uptr)p)<(uptr)db->lookaside.p_انتها ){
#اگر_تعریف_نشده SQLITE_OMIT_TWOSIZE_LOOKASIDE
    إذا( ((uptr)p)>=(uptr)db->lookaside.p_میانه ){
      إذا( n<=LOOKASIDE_SMALL ) أرجع p;
    }وإلا
#پایان_اگر
    إذا( ((uptr)p)>=(uptr)db->lookaside.p_شروع ){
      إذا( n<=db->lookaside.szTrue ) أرجع p;
    }
  }
  أرجع db_Realloc_پایان(db, p, n);
}
ساكن SQLITE_NOINLINE فراغ *db_Realloc_پایان(sqlite3 *db, فراغ *p, u64 n){
  فراغ *pNew = 0;
  أكد( db!=0 );
  أكد( p!=0 );
  إذا( db->mallocFailed==0 ){
    إذا( isLookaside(db, p) ){
      pNew = sqlite3DbMallocRawNN(db, n);
      إذا( pNew ){
        کپی_حافظه(pNew, p, lookaside_Malloc_اندازه(db, p));
        sqlite3DbFree(db, p);
      }
    }وإلا{
      أكد( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_انباشته)) );
      أكد( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_انباشته)) );
      sqlite3_Memdebug_مجموعه_Type(p, MEMTYPE_انباشته);
      pNew = sqlite3Realloc(p, n);
      إذا( !pNew ){
        sqlite3OomFault(db);
      }
      sqlite3_Memdebug_مجموعه_Type(pNew,
            (db->lookaside.bDisable==0 ? MEMTYPE_LOOKASIDE : MEMTYPE_انباشته));
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
  n = طول_رشته(z) + 1;
  zNew = sqlite3DbMallocRaw(db, n);
  إذا( zNew ){
    کپی_حافظه(zNew, z, n);
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
    کپی_حافظه(zNew, z, (size_t)n);
    zNew[n] = 0;
  }
  أرجع zNew;
}

/*
** The text between zStart and zEnd represents a phrase within a larger
** SQL statement.  Make a copy of this phrase in space obtained form
** sqlite3DbMalloc().  Omit leading and trailing whitespace.
*/
حرف *sqlite3_Db_بازه_Dup(sqlite3 *db, ثابت حرف *z_شروع, ثابت حرف *z_انتها){
  صحيح n;
#اگر_تعریف_شده SQLITE_DEBUG
  /* Because of the way the parser works, the span is guaranteed to contain
  ** at least one non-space character */
  لكل(n=0; sqlite3Isspace(z_شروع[n]); n++){ أكد( &z_شروع[n]<z_انتها ); }
#پایان_اگر
  طالما( sqlite3Isspace(z_شروع[0]) ) z_شروع++;
  n = (صحيح)(z_انتها - z_شروع);
  طالما( sqlite3Isspace(z_شروع[n-1]) ) n--;
  أرجع sqlite3DbStrNDup(db, z_شروع, n);
}

/*
** Free any prior content in *pz and replace it with a copy of zNew.
*/
فراغ sqlite3_مجموعه_String(حرف **pz, sqlite3 *db, ثابت حرف *zNew){
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
      sqlite3_Error_پیام(db->pParse, "out of memory");
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
  أرجع rc & db->err_ماسک;
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
