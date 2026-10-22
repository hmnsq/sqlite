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
#అన్తర్భావ "sqliteInt.h"
#అన్తర్భావ <stdarg.h>

/*
** Attempt to release up to n bytes of non-essential memory currently
** held by SQLite. An example of non-essential memory is memory used to
** cache database pages that are not currently in use.
*/
పూర్ణాంకం sqlite3_release_జ్ఞాపకం(పూర్ణాంకం n){
#నిర్వచితమైతే SQLITE_ENABLE_జ్ఞాపకం_MANAGEMENT
  తిరిగి sqlite3_Pcache_Release_జ్ఞాపకం(n);
#లేకపోతే
  /* IMPLEMENTATION-OF: R-34391-24921 The sqlite3_release_memory() routine
  ** is a no-op returning zero if SQLite is not compiled with
  ** SQLITE_ENABLE_MEMORY_MANAGEMENT. */
  వాడని_PARAMETER(n);
  తిరిగి 0;
#అయితే_ముగింపు
}

/*
** Default value of the hard heap limit.  0 means "no limit".
*/
#నిర్వచితం_కాకపోతే SQLITE_MAX_జ్ఞాపకం
# define SQLITE_MAX_జ్ఞాపకం 0
#అయితే_ముగింపు

/*
** State information local to the memory allocation subsystem.
*/
స్థిర SQLITE_WSD నిర్మాణం Mem0Global {
  sqlite3_mutex *mutex;         /* Mutex to serialize access */
  sqlite3_int64 alarm_పరిమితి; /* The soft heap limit */
  sqlite3_int64 hard_హద్దు;      /* The hard upper bound on memory */

  /*
  ** True if heap is nearly "full" where "full" is defined by the
  ** sqlite3_soft_heap_limit() setting.
  */
  పూర్ణాంకం nearly_నిండిన;
} mem0 = { 0, SQLITE_MAX_జ్ఞాపకం, SQLITE_MAX_జ్ఞాపకం, 0 };

#నిర్వచించు mem0 GLOBAL(నిర్మాణం Mem0Global, mem0)

/*
** Return the memory allocator mutex. sqlite3_status() needs it.
*/
sqlite3_mutex *sqlite3MallocMutex(శూన్య){
  తిరిగి mem0.mutex;
}

#నిర్వచితం_కాకపోతే SQLITE_OMIT_DEPRECATED
/*
** Deprecated external interface.  It used to set an alarm callback
** that was invoked when memory usage grew too large.  Now it is a
** no-op.
*/
పూర్ణాంకం sqlite3_జ్ఞాపకం_alarm(
  శూన్య(*xCallback)(శూన్య *pArg, sqlite3_int64 వాడిన,పూర్ణాంకం N),
  శూన్య *pArg,
  sqlite3_int64 i_పరిమితి
){
  (శూన్య)xCallback;
  (శూన్య)pArg;
  (శూన్య)i_పరిమితి;
  తిరిగి SQLITE_OK;
}
#అయితే_ముగింపు

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
sqlite3_int64 sqlite3_soft_కుప్ప_limit64(sqlite3_int64 n){
  sqlite3_int64 పూర్వ_హద్దు;
  sqlite3_int64 excess;
  sqlite3_int64 n_వాడిన;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOINIT
  పూర్ణాంకం rc = sqlite3_initialize();
  ఒకవేళ( rc ) తిరిగి -1;
#అయితే_ముగింపు
  sqlite3_mutex_enter(mem0.mutex);
  పూర్వ_హద్దు = mem0.alarm_పరిమితి;
  ఒకవేళ( n<0 ){
    sqlite3_mutex_leave(mem0.mutex);
    తిరిగి పూర్వ_హద్దు;
  }
  ఒకవేళ( mem0.hard_హద్దు>0 && (n>mem0.hard_హద్దు || n==0) ){
    n = mem0.hard_హద్దు;
  }
  mem0.alarm_పరిమితి = n;
  n_వాడిన = sqlite3_స్థితి_విలువ(SQLITE_స్థితి_జ్ఞాపకం_వాడిన);
  AtomicStore(&mem0.nearly_నిండిన, n>0 && n<=n_వాడిన);
  sqlite3_mutex_leave(mem0.mutex);
  excess = sqlite3_జ్ఞాపకం_వాడిన() - n;
  ఒకవేళ( excess>0 ) sqlite3_release_జ్ఞాపకం((పూర్ణాంకం)(excess & 0x7fffffff));
  తిరిగి పూర్వ_హద్దు;
}
శూన్య sqlite3_soft_కుప్ప_హద్దు(పూర్ణాంకం n){
  ఒకవేళ( n<0 ) n = 0;
  sqlite3_soft_కుప్ప_limit64(n);
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
sqlite3_int64 sqlite3_hard_కుప్ప_limit64(sqlite3_int64 n){
  sqlite3_int64 పూర్వ_హద్దు;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOINIT
  పూర్ణాంకం rc = sqlite3_initialize();
  ఒకవేళ( rc ) తిరిగి -1;
#అయితే_ముగింపు
  sqlite3_mutex_enter(mem0.mutex);
  పూర్వ_హద్దు = mem0.hard_హద్దు;
  ఒకవేళ( n>=0 ){
    mem0.hard_హద్దు = n;
    ఒకవేళ( n<mem0.alarm_పరిమితి || mem0.alarm_పరిమితి==0 ){
      mem0.alarm_పరిమితి = n;
    }
  }
  sqlite3_mutex_leave(mem0.mutex);
  తిరిగి పూర్వ_హద్దు;
}


/*
** Initialize the memory allocation subsystem.
*/
పూర్ణాంకం sqlite3MallocInit(శూన్య){
  పూర్ణాంకం rc;
  ఒకవేళ( sqlite3GlobalConfig.m.xMalloc==0 ){
    sqlite3_Mem_సమితి_Default();
  }
  mem0.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM);
  ఒకవేళ( sqlite3GlobalConfig.p_పుట==0 || sqlite3GlobalConfig.sz_పుట<512
      || sqlite3GlobalConfig.n_పుట<=0 ){
    sqlite3GlobalConfig.p_పుట = 0;
    sqlite3GlobalConfig.sz_పుట = 0;
  }
  rc = sqlite3GlobalConfig.m.xInit(sqlite3GlobalConfig.m.p_App_దత్తాంశం);
  ఒకవేళ( rc!=SQLITE_OK ) memset(&mem0, 0, పరిమాణం(mem0));
  తిరిగి rc;
}

/*
** Return true if the heap is currently under memory pressure - in other
** words if the amount of heap used is close to the limit set by
** sqlite3_soft_heap_limit().
*/
పూర్ణాంకం sqlite3_కుప్ప_Nearly_నిండిన(శూన్య){
  తిరిగి Atomic_ఎక్కించు(&mem0.nearly_నిండిన);
}

/*
** Deinitialize the memory allocation subsystem.
*/
శూన్య sqlite3_Malloc_చివర(శూన్య){
  ఒకవేళ( sqlite3GlobalConfig.m.xShutdown ){
    sqlite3GlobalConfig.m.xShutdown(sqlite3GlobalConfig.m.p_App_దత్తాంశం);
  }
  memset(&mem0, 0, పరిమాణం(mem0));
}

/*
** Return the amount of memory currently checked out.
*/
sqlite3_int64 sqlite3_జ్ఞాపకం_వాడిన(శూన్య){
  sqlite3_int64 res, mx;
  sqlite3_status64(SQLITE_స్థితి_జ్ఞాపకం_వాడిన, &res, &mx, 0);
  తిరిగి res;
}

/*
** Return the maximum amount of memory that has ever been
** checked out since either the beginning of this process
** or since the most recent reset.
*/
sqlite3_int64 sqlite3_జ్ఞాపకం_highwater(పూర్ణాంకం resetFlag){
  sqlite3_int64 res, mx;
  sqlite3_status64(SQLITE_స్థితి_జ్ఞాపకం_వాడిన, &res, &mx, resetFlag);
  తిరిగి mx;
}

/*
** Trigger the alarm 
*/
స్థిర శూన్య sqlite3MallocAlarm(పూర్ణాంకం nByte){
  ఒకవేళ( mem0.alarm_పరిమితి<=0 ) తిరిగి;
  sqlite3_mutex_leave(mem0.mutex);
  sqlite3_release_జ్ఞాపకం(nByte);
  sqlite3_mutex_enter(mem0.mutex);
}

#నిర్వచితమైతే SQLITE_DEBUG
/*
** This routine is called whenever an out-of-memory condition is seen,
** It's only purpose to to serve as a breakpoint for gdb or similar
** code debuggers when working on out-of-memory conditions, for example
** caused by PRAGMA hard_heap_limit=N.
*/
స్థిర SQLITE_NOINLINE శూన్య test_oom_breakpoint(u64 n){
  స్థిర u64 nOomFault = 0;
  nOomFault += n;
  /* The assert() is never reached in a human lifetime.  It  is here mostly
  ** to prevent code optimizers from optimizing out this function. */
  ప్రతిజ్ఞా( (nOomFault>>32) < 0xffffffff );
}
#లేకపోతే
# define test_oom_breakpoint(X)   /* No-op for production builds */
#అయితే_ముగింపు

/*
** Do a memory allocation with statistics and alarms.  Assume the
** lock is already held.
*/
స్థిర శూన్య mallocWithAlarm(పూర్ణాంకం n, శూన్య **pp){
  శూన్య *p;
  పూర్ణాంకం n_నిండిన;
  ప్రతిజ్ఞా( sqlite3_mutex_held(mem0.mutex) );
  ప్రతిజ్ఞా( n>0 );

  /* In Firefox (circa 2017-02-08), xRoundup() is remapped to an internal
  ** implementation of malloc_good_size(), which must be called in debug
  ** mode and specifically when the DMD "Dark Matter Detector" is enabled
  ** or else a crash results.  Hence, do not attempt to optimize out the
  ** following xRoundup() call. */
  n_నిండిన = sqlite3GlobalConfig.m.xRoundup(n);

  sqlite3_స్థితి_Highwater(SQLITE_స్థితి_MALLOC_కొలత, n);
  ఒకవేళ( mem0.alarm_పరిమితి>0 ){
    sqlite3_int64 n_వాడిన = sqlite3_స్థితి_విలువ(SQLITE_స్థితి_జ్ఞాపకం_వాడిన);
    ఒకవేళ( n_వాడిన >= mem0.alarm_పరిమితి - n_నిండిన ){
      AtomicStore(&mem0.nearly_నిండిన, 1);
      sqlite3MallocAlarm(n_నిండిన);
      ఒకవేళ( mem0.hard_హద్దు ){
        n_వాడిన = sqlite3_స్థితి_విలువ(SQLITE_స్థితి_జ్ఞాపకం_వాడిన);
        ఒకవేళ( n_వాడిన >= mem0.hard_హద్దు - n_నిండిన ){
          test_oom_breakpoint(1);
          *pp = 0;
          తిరిగి;
        }
      }
    }లేకపోతే{
      AtomicStore(&mem0.nearly_నిండిన, 0);
    }
  }
  p = sqlite3GlobalConfig.m.xMalloc(n_నిండిన);
#నిర్వచితమైతే SQLITE_ENABLE_జ్ఞాపకం_MANAGEMENT
  ఒకవేళ( p==0 && mem0.alarm_పరిమితి>0 ){
    sqlite3MallocAlarm(n_నిండిన);
    p = sqlite3GlobalConfig.m.xMalloc(n_నిండిన);
  }
#అయితే_ముగింపు
  ఒకవేళ( p ){
    n_నిండిన = sqlite3_Malloc_కొలత(p);
    sqlite3_స్థితి_పైకి(SQLITE_స్థితి_జ్ఞాపకం_వాడిన, n_నిండిన);
    sqlite3_స్థితి_పైకి(SQLITE_స్థితి_MALLOC_లెక్క, 1);
  }
  *pp = p;
}

/*
** Allocate memory.  This routine is like sqlite3_malloc() except that it
** assumes the memory subsystem has already been initialized.
*/
శూన్య *sqlite3Malloc(u64 n){
  శూన్య *p;
  ఒకవేళ( n==0 || n>SQLITE_MAX_కేటాయింపు_కొలత ){
    p = 0;
  }లేకపోతే ఒకవేళ( sqlite3GlobalConfig.bMemstat ){
    sqlite3_mutex_enter(mem0.mutex);
    mallocWithAlarm((పూర్ణాంకం)n, &p);
    sqlite3_mutex_leave(mem0.mutex);
  }లేకపోతే{
    p = sqlite3GlobalConfig.m.xMalloc((పూర్ణాంకం)n);
  }
  ప్రతిజ్ఞా( EIGHT_BYTE_ALIGNMENT(p) );  /* IMP: R-11148-40995 */
  తిరిగి p;
}

/*
** This version of the memory allocation is for use by the application.
** First make sure the memory subsystem is initialized, then do the
** allocation.
*/
శూన్య *sqlite3_malloc(పూర్ణాంకం n){
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOINIT
  ఒకవేళ( sqlite3_initialize() ) తిరిగి 0;
#అయితే_ముగింపు
  తిరిగి n<=0 ? 0 : sqlite3Malloc(n);
}
శూన్య *sqlite3_malloc64(sqlite3_uint64 n){
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOINIT
  ఒకవేళ( sqlite3_initialize() ) తిరిగి 0;
#అయితే_ముగింపు
  తిరిగి sqlite3Malloc(n);
}

/*
** TRUE if p is a lookaside memory allocation from db
*/
#నిర్వచితం_కాకపోతే SQLITE_OMIT_LOOKASIDE
స్థిర పూర్ణాంకం isLookaside(sqlite3 *db, స్థిరరాశి శూన్య *p){
  తిరిగి SQLITE_WITHIN(p, db->lookaside.p_ప్రారంభం, db->lookaside.p_True_చివర);
}
#లేకపోతే
#నిర్వచించు isLookaside(A,B) 0
#అయితే_ముగింపు

/*
** Return the size of a memory allocation previously obtained from
** sqlite3Malloc() or sqlite3_malloc().
*/
పూర్ణాంకం sqlite3_Malloc_కొలత(స్థిరరాశి శూన్య *p){
  ప్రతిజ్ఞా( sqlite3MemdebugHasType(p, MEMTYPE_కుప్ప) );
  తిరిగి sqlite3GlobalConfig.m.x_కొలత((శూన్య*)p);
}
స్థిర పూర్ణాంకం lookaside_Malloc_కొలత(sqlite3 *db, స్థిరరాశి శూన్య *p){
#నిర్వచితం_కాకపోతే SQLITE_OMIT_TWOSIZE_LOOKASIDE    
  తిరిగి p<db->lookaside.p_మధ్య ? db->lookaside.szTrue : LOOKASIDE_SMALL;
#లేకపోతే
  తిరిగి db->lookaside.szTrue;
#అయితే_ముగింపు  
}
పూర్ణాంకం sqlite3_Db_Malloc_కొలత(sqlite3 *db, స్థిరరాశి శూన్య *p){
  ప్రతిజ్ఞా( p!=0 );
#నిర్వచితమైతే SQLITE_DEBUG
  ఒకవేళ( db==0 ){
    ప్రతిజ్ఞా( sqlite3MemdebugNoType(p, (u8)~MEMTYPE_కుప్ప) );
    ప్రతిజ్ఞా( sqlite3MemdebugHasType(p, MEMTYPE_కుప్ప) );
  }లేకపోతే ఒకవేళ( !isLookaside(db,p) ){
    ప్రతిజ్ఞా( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_కుప్ప)) );
    ప్రతిజ్ఞా( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_కుప్ప)) );
  }
#అయితే_ముగింపు
  ఒకవేళ( db ){
    ఒకవేళ( ((uptr)p)<(uptr)(db->lookaside.p_True_చివర) ){
#నిర్వచితం_కాకపోతే SQLITE_OMIT_TWOSIZE_LOOKASIDE
      ఒకవేళ( ((uptr)p)>=(uptr)(db->lookaside.p_మధ్య) ){
        ప్రతిజ్ఞా( sqlite3_mutex_held(db->mutex) );
        తిరిగి LOOKASIDE_SMALL;
      }
#అయితే_ముగింపు
      ఒకవేళ( ((uptr)p)>=(uptr)(db->lookaside.p_ప్రారంభం) ){
        ప్రతిజ్ఞా( sqlite3_mutex_held(db->mutex) );
        తిరిగి db->lookaside.szTrue;
      }
    }
  }
  తిరిగి sqlite3GlobalConfig.m.x_కొలత((శూన్య*)p);
}
sqlite3_uint64 sqlite3_msize(శూన్య *p){
  ప్రతిజ్ఞా( sqlite3MemdebugNoType(p, (u8)~MEMTYPE_కుప్ప) );
  ప్రతిజ్ఞా( sqlite3MemdebugHasType(p, MEMTYPE_కుప్ప) );
  తిరిగి p ? sqlite3GlobalConfig.m.x_కొలత(p) : 0;
}

/*
** Free memory previously obtained from sqlite3Malloc().
*/
శూన్య sqlite3_free(శూన్య *p){
  ఒకవేళ( p==0 ) తిరిగి;  /* IMP: R-49053-54554 */
  ప్రతిజ్ఞా( sqlite3MemdebugHasType(p, MEMTYPE_కుప్ప) );
  ప్రతిజ్ఞా( sqlite3MemdebugNoType(p, (u8)~MEMTYPE_కుప్ప) );
  ఒకవేళ( sqlite3GlobalConfig.bMemstat ){
    sqlite3_mutex_enter(mem0.mutex);
    sqlite3_స్థితి_కిందికి(SQLITE_స్థితి_జ్ఞాపకం_వాడిన, sqlite3_Malloc_కొలత(p));
    sqlite3_స్థితి_కిందికి(SQLITE_స్థితి_MALLOC_లెక్క, 1);
    sqlite3GlobalConfig.m.xFree(p);
    sqlite3_mutex_leave(mem0.mutex);
  }లేకపోతే{
    sqlite3GlobalConfig.m.xFree(p);
  }
}

/*
** Add the size of memory allocation "p" to the count in
** *db->pnBytesFreed.
*/
స్థిర SQLITE_NOINLINE శూన్య measure_కేటాయింపు_కొలత(sqlite3 *db, శూన్య *p){
  *db->pn_బైట్లు_Freed += sqlite3_Db_Malloc_కొలత(db,p);
}

/*
** Free memory that might be associated with a particular database
** connection.  Calling sqlite3DbFree(D,X) for X==0 is a harmless no-op.
** The sqlite3DbFreeNN(D,X) version requires that X be non-NULL.
*/
శూన్య sqlite3DbFreeNN(sqlite3 *db, శూన్య *p){
  ప్రతిజ్ఞా( db==0 || sqlite3_mutex_held(db->mutex) );
  ప్రతిజ్ఞా( p!=0 );
  ఒకవేళ( db ){
    ఒకవేళ( ((uptr)p)<(uptr)(db->lookaside.p_చివర) ){
#నిర్వచితం_కాకపోతే SQLITE_OMIT_TWOSIZE_LOOKASIDE
      ఒకవేళ( ((uptr)p)>=(uptr)(db->lookaside.p_మధ్య) ){
        Lookaside_స్థానం *pBuf = (Lookaside_స్థానం*)p;
        ప్రతిజ్ఞా( db->pn_బైట్లు_Freed==0 );
#నిర్వచితమైతే SQLITE_DEBUG
        memset(p, 0xaa, LOOKASIDE_SMALL);  /* Trash freed content */
#అయితే_ముగింపు
        pBuf->p_తదుపరి = db->lookaside.pSmallFree;
        db->lookaside.pSmallFree = pBuf;
        తిరిగి;
      }
#అయితే_ముగింపు /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
      ఒకవేళ( ((uptr)p)>=(uptr)(db->lookaside.p_ప్రారంభం) ){
        Lookaside_స్థానం *pBuf = (Lookaside_స్థానం*)p;
        ప్రతిజ్ఞా( db->pn_బైట్లు_Freed==0 );
#నిర్వచితమైతే SQLITE_DEBUG
        memset(p, 0xaa, db->lookaside.szTrue);  /* Trash freed content */
#అయితే_ముగింపు
        pBuf->p_తదుపరి = db->lookaside.pFree;
        db->lookaside.pFree = pBuf;
        తిరిగి;
      }
    }
    ఒకవేళ( db->pn_బైట్లు_Freed ){
      measure_కేటాయింపు_కొలత(db, p);
      తిరిగి;
    }
  }
  ప్రతిజ్ఞా( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_కుప్ప)) );
  ప్రతిజ్ఞా( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_కుప్ప)) );
  ప్రతిజ్ఞా( db!=0 || sqlite3MemdebugNoType(p, MEMTYPE_LOOKASIDE) );
  sqlite3_Memdebug_సమితి_Type(p, MEMTYPE_కుప్ప);
  sqlite3_free(p);
}
శూన్య sqlite3DbNNFreeNN(sqlite3 *db, శూన్య *p){
  ప్రతిజ్ఞా( db!=0 );
  ప్రతిజ్ఞా( sqlite3_mutex_held(db->mutex) );
  ప్రతిజ్ఞా( p!=0 );
  ఒకవేళ( ((uptr)p)<(uptr)(db->lookaside.p_చివర) ){
#నిర్వచితం_కాకపోతే SQLITE_OMIT_TWOSIZE_LOOKASIDE
    ఒకవేళ( ((uptr)p)>=(uptr)(db->lookaside.p_మధ్య) ){
      Lookaside_స్థానం *pBuf = (Lookaside_స్థానం*)p;
      ప్రతిజ్ఞా( db->pn_బైట్లు_Freed==0 );
#నిర్వచితమైతే SQLITE_DEBUG
      memset(p, 0xaa, LOOKASIDE_SMALL);  /* Trash freed content */
#అయితే_ముగింపు
      pBuf->p_తదుపరి = db->lookaside.pSmallFree;
      db->lookaside.pSmallFree = pBuf;
      తిరిగి;
    }
#అయితే_ముగింపు /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
    ఒకవేళ( ((uptr)p)>=(uptr)(db->lookaside.p_ప్రారంభం) ){
      Lookaside_స్థానం *pBuf = (Lookaside_స్థానం*)p;
      ప్రతిజ్ఞా( db->pn_బైట్లు_Freed==0 );
#నిర్వచితమైతే SQLITE_DEBUG
      memset(p, 0xaa, db->lookaside.szTrue);  /* Trash freed content */
#అయితే_ముగింపు
      pBuf->p_తదుపరి = db->lookaside.pFree;
      db->lookaside.pFree = pBuf;
      తిరిగి;
    }
  }
  ఒకవేళ( db->pn_బైట్లు_Freed ){
    measure_కేటాయింపు_కొలత(db, p);
    తిరిగి;
  }
  ప్రతిజ్ఞా( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_కుప్ప)) );
  ప్రతిజ్ఞా( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_కుప్ప)) );
  sqlite3_Memdebug_సమితి_Type(p, MEMTYPE_కుప్ప);
  sqlite3_free(p);
}
శూన్య sqlite3DbFree(sqlite3 *db, శూన్య *p){
  ప్రతిజ్ఞా( db==0 || sqlite3_mutex_held(db->mutex) );
  ఒకవేళ( p ) sqlite3DbFreeNN(db, p);
}

/*
** Change the size of an existing memory allocation
*/
శూన్య *sqlite3Realloc(శూన్య *pOld, u64 n_బైట్లు){
  పూర్ణాంకం nOld, nNew, nDiff;
  శూన్య *pNew;
  ప్రతిజ్ఞా( sqlite3MemdebugHasType(pOld, MEMTYPE_కుప్ప) );
  ప్రతిజ్ఞా( sqlite3MemdebugNoType(pOld, (u8)~MEMTYPE_కుప్ప) );
  ఒకవేళ( pOld==0 ){
    తిరిగి sqlite3Malloc(n_బైట్లు); /* IMP: R-04300-56712 */
  }
  ఒకవేళ( n_బైట్లు==0 ){
    sqlite3_free(pOld); /* IMP: R-26507-47431 */
    తిరిగి 0;
  }
  ఒకవేళ( n_బైట్లు>SQLITE_MAX_కేటాయింపు_కొలత ){
    తిరిగి 0;
  }
  nOld = sqlite3_Malloc_కొలత(pOld);
  /* IMPLEMENTATION-OF: R-46199-30249 SQLite guarantees that the second
  ** argument to xRealloc is always a value returned by a prior call to
  ** xRoundup. */
  nNew = sqlite3GlobalConfig.m.xRoundup((పూర్ణాంకం)n_బైట్లు);
  ఒకవేళ( nOld==nNew ){
    pNew = pOld;
  }లేకపోతే ఒకవేళ( sqlite3GlobalConfig.bMemstat ){
    sqlite3_int64 n_వాడిన;
    sqlite3_mutex_enter(mem0.mutex);
    sqlite3_స్థితి_Highwater(SQLITE_స్థితి_MALLOC_కొలత, (పూర్ణాంకం)n_బైట్లు);
    nDiff = nNew - nOld;
    ఒకవేళ( nDiff>0 && (n_వాడిన = sqlite3_స్థితి_విలువ(SQLITE_స్థితి_జ్ఞాపకం_వాడిన)) >= 
          mem0.alarm_పరిమితి-nDiff ){
      sqlite3MallocAlarm(nDiff);
      ఒకవేళ( mem0.hard_హద్దు>0 && n_వాడిన >= mem0.hard_హద్దు - nDiff ){
        sqlite3_mutex_leave(mem0.mutex);
        test_oom_breakpoint(1);
        తిరిగి 0;
      }
    }
    pNew = sqlite3GlobalConfig.m.xRealloc(pOld, nNew);
#నిర్వచితమైతే SQLITE_ENABLE_జ్ఞాపకం_MANAGEMENT
    ఒకవేళ( pNew==0 && mem0.alarm_పరిమితి>0 ){
      sqlite3MallocAlarm((పూర్ణాంకం)n_బైట్లు);
      pNew = sqlite3GlobalConfig.m.xRealloc(pOld, nNew);
    }
#అయితే_ముగింపు
    ఒకవేళ( pNew ){
      nNew = sqlite3_Malloc_కొలత(pNew);
      sqlite3_స్థితి_పైకి(SQLITE_స్థితి_జ్ఞాపకం_వాడిన, nNew-nOld);
    }
    sqlite3_mutex_leave(mem0.mutex);
  }లేకపోతే{
    pNew = sqlite3GlobalConfig.m.xRealloc(pOld, nNew);
  }
  ప్రతిజ్ఞా( EIGHT_BYTE_ALIGNMENT(pNew) ); /* IMP: R-11148-40995 */
  తిరిగి pNew;
}

/*
** The public interface to sqlite3Realloc.  Make sure that the memory
** subsystem is initialized prior to invoking sqliteRealloc.
*/
శూన్య *sqlite3_realloc(శూన్య *pOld, పూర్ణాంకం n){
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOINIT
  ఒకవేళ( sqlite3_initialize() ) తిరిగి 0;
#అయితే_ముగింపు
  ఒకవేళ( n<0 ) n = 0;  /* IMP: R-26507-47431 */
  తిరిగి sqlite3Realloc(pOld, n);
}
శూన్య *sqlite3_realloc64(శూన్య *pOld, sqlite3_uint64 n){
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOINIT
  ఒకవేళ( sqlite3_initialize() ) తిరిగి 0;
#అయితే_ముగింపు
  తిరిగి sqlite3Realloc(pOld, n);
}


/*
** Allocate and zero memory.
*/ 
శూన్య *sqlite3MallocZero(u64 n){
  శూన్య *p = sqlite3Malloc(n);
  ఒకవేళ( p ){
    memset(p, 0, (size_t)n);
  }
  తిరిగి p;
}

/*
** Allocate and zero memory.  If the allocation fails, make
** the mallocFailed flag in the connection pointer.
*/
శూన్య *sqlite3DbMallocZero(sqlite3 *db, u64 n){
  శూన్య *p;
  testcase( db==0 );
  p = sqlite3DbMallocRaw(db, n);
  ఒకవేళ( p ) memset(p, 0, (size_t)n);
  తిరిగి p;
}


/* Finish the work of sqlite3DbMallocRawNN for the unusual and
** slower case when the allocation cannot be fulfilled using lookaside.
*/
స్థిర SQLITE_NOINLINE శూన్య *db_Malloc_Raw_ముగింపు(sqlite3 *db, u64 n){
  శూన్య *p;
  ప్రతిజ్ఞా( db!=0 );
  p = sqlite3Malloc(n);
  ఒకవేళ( !p ) sqlite3OomFault(db);
  sqlite3_Memdebug_సమితి_Type(p, 
         (db->lookaside.bDisable==0) ? MEMTYPE_LOOKASIDE : MEMTYPE_కుప్ప);
  తిరిగి p;
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
శూన్య *sqlite3DbMallocRaw(sqlite3 *db, u64 n){
  శూన్య *p;
  ఒకవేళ( db ) తిరిగి sqlite3DbMallocRawNN(db, n);
  p = sqlite3Malloc(n);
  sqlite3_Memdebug_సమితి_Type(p, MEMTYPE_కుప్ప);
  తిరిగి p;
}
శూన్య *sqlite3DbMallocRawNN(sqlite3 *db, u64 n){
#నిర్వచితం_కాకపోతే SQLITE_OMIT_LOOKASIDE
  Lookaside_స్థానం *pBuf;
  ప్రతిజ్ఞా( db!=0 );
  ప్రతిజ్ఞా( sqlite3_mutex_held(db->mutex) );
  ప్రతిజ్ఞా( db->pn_బైట్లు_Freed==0 );
  ఒకవేళ( n>db->lookaside.sz ){
    ఒకవేళ( !db->lookaside.bDisable ){
      db->lookaside.anStat[1]++;      
    }లేకపోతే ఒకవేళ( db->mallocFailed ){
      తిరిగి 0;
    }
    తిరిగి db_Malloc_Raw_ముగింపు(db, n);
  }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_TWOSIZE_LOOKASIDE
  ఒకవేళ( n<=LOOKASIDE_SMALL ){
    ఒకవేళ( (pBuf = db->lookaside.pSmallFree)!=0 ){
      db->lookaside.pSmallFree = pBuf->p_తదుపరి;
      db->lookaside.anStat[0]++;
      తిరిగి (శూన్య*)pBuf;
    }లేకపోతే ఒకవేళ( (pBuf = db->lookaside.pSmallInit)!=0 ){
      db->lookaside.pSmallInit = pBuf->p_తదుపరి;
      db->lookaside.anStat[0]++;
      తిరిగి (శూన్య*)pBuf;
    }
  }
#అయితే_ముగింపు
  ఒకవేళ( (pBuf = db->lookaside.pFree)!=0 ){
    db->lookaside.pFree = pBuf->p_తదుపరి;
    db->lookaside.anStat[0]++;
    తిరిగి (శూన్య*)pBuf;
  }లేకపోతే ఒకవేళ( (pBuf = db->lookaside.pInit)!=0 ){
    db->lookaside.pInit = pBuf->p_తదుపరి;
    db->lookaside.anStat[0]++;
    తిరిగి (శూన్య*)pBuf;
  }లేకపోతే{
    db->lookaside.anStat[2]++;
  }
#లేకపోతే
  ప్రతిజ్ఞా( db!=0 );
  ప్రతిజ్ఞా( sqlite3_mutex_held(db->mutex) );
  ప్రతిజ్ఞా( db->pn_బైట్లు_Freed==0 );
  ఒకవేళ( db->mallocFailed ){
    తిరిగి 0;
  }
#అయితే_ముగింపు
  తిరిగి db_Malloc_Raw_ముగింపు(db, n);
}

/* Forward declaration */
స్థిర SQLITE_NOINLINE శూన్య *db_Realloc_ముగింపు(sqlite3 *db, శూన్య *p, u64 n);

/*
** Resize the block of memory pointed to by p to n bytes. If the
** resize fails, set the mallocFailed flag in the connection object.
*/
శూన్య *sqlite3DbRealloc(sqlite3 *db, శూన్య *p, u64 n){
  ప్రతిజ్ఞా( db!=0 );
  ఒకవేళ( p==0 ) తిరిగి sqlite3DbMallocRawNN(db, n);
  ప్రతిజ్ఞా( sqlite3_mutex_held(db->mutex) );
  ఒకవేళ( ((uptr)p)<(uptr)db->lookaside.p_చివర ){
#నిర్వచితం_కాకపోతే SQLITE_OMIT_TWOSIZE_LOOKASIDE
    ఒకవేళ( ((uptr)p)>=(uptr)db->lookaside.p_మధ్య ){
      ఒకవేళ( n<=LOOKASIDE_SMALL ) తిరిగి p;
    }లేకపోతే
#అయితే_ముగింపు
    ఒకవేళ( ((uptr)p)>=(uptr)db->lookaside.p_ప్రారంభం ){
      ఒకవేళ( n<=db->lookaside.szTrue ) తిరిగి p;
    }
  }
  తిరిగి db_Realloc_ముగింపు(db, p, n);
}
స్థిర SQLITE_NOINLINE శూన్య *db_Realloc_ముగింపు(sqlite3 *db, శూన్య *p, u64 n){
  శూన్య *pNew = 0;
  ప్రతిజ్ఞా( db!=0 );
  ప్రతిజ్ఞా( p!=0 );
  ఒకవేళ( db->mallocFailed==0 ){
    ఒకవేళ( isLookaside(db, p) ){
      pNew = sqlite3DbMallocRawNN(db, n);
      ఒకవేళ( pNew ){
        జ్ఞాపకం_నకలు(pNew, p, lookaside_Malloc_కొలత(db, p));
        sqlite3DbFree(db, p);
      }
    }లేకపోతే{
      ప్రతిజ్ఞా( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_కుప్ప)) );
      ప్రతిజ్ఞా( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_కుప్ప)) );
      sqlite3_Memdebug_సమితి_Type(p, MEMTYPE_కుప్ప);
      pNew = sqlite3Realloc(p, n);
      ఒకవేళ( !pNew ){
        sqlite3OomFault(db);
      }
      sqlite3_Memdebug_సమితి_Type(pNew,
            (db->lookaside.bDisable==0 ? MEMTYPE_LOOKASIDE : MEMTYPE_కుప్ప));
    }
  }
  తిరిగి pNew;
}

/*
** Attempt to reallocate p.  If the reallocation fails, then free p
** and set the mallocFailed flag in the database connection.
*/
శూన్య *sqlite3DbReallocOrFree(sqlite3 *db, శూన్య *p, u64 n){
  శూన్య *pNew;
  pNew = sqlite3DbRealloc(db, p, n);
  ఒకవేళ( !pNew ){
    sqlite3DbFree(db, p);
  }
  తిరిగి pNew;
}

/*
** Make a copy of a string in memory obtained from sqliteMalloc(). These 
** functions call sqlite3MallocRaw() directly instead of sqliteMalloc(). This
** is because when memory debugging is turned on, these two functions are 
** called via macros that record the current file and line number in the
** ThreadData structure.
*/
అక్షరం *sqlite3DbStrDup(sqlite3 *db, స్థిరరాశి అక్షరం *z){
  అక్షరం *zNew;
  size_t n;
  ఒకవేళ( z==0 ){
    తిరిగి 0;
  }
  n = అక్షరశ్రేణి_పొడవు(z) + 1;
  zNew = sqlite3DbMallocRaw(db, n);
  ఒకవేళ( zNew ){
    జ్ఞాపకం_నకలు(zNew, z, n);
  }
  తిరిగి zNew;
}
అక్షరం *sqlite3DbStrNDup(sqlite3 *db, స్థిరరాశి అక్షరం *z, u64 n){
  అక్షరం *zNew;
  ప్రతిజ్ఞా( db!=0 );
  ప్రతిజ్ఞా( z!=0 || n==0 );
  ప్రతిజ్ఞా( (n&0x7fffffff)==n );
  zNew = z ? sqlite3DbMallocRawNN(db, n+1) : 0;
  ఒకవేళ( zNew ){
    జ్ఞాపకం_నకలు(zNew, z, (size_t)n);
    zNew[n] = 0;
  }
  తిరిగి zNew;
}

/*
** The text between zStart and zEnd represents a phrase within a larger
** SQL statement.  Make a copy of this phrase in space obtained form
** sqlite3DbMalloc().  Omit leading and trailing whitespace.
*/
అక్షరం *sqlite3_Db_పరిధి_Dup(sqlite3 *db, స్థిరరాశి అక్షరం *z_ప్రారంభం, స్థిరరాశి అక్షరం *z_చివర){
  పూర్ణాంకం n;
#నిర్వచితమైతే SQLITE_DEBUG
  /* Because of the way the parser works, the span is guaranteed to contain
  ** at least one non-space character */
  కొరకు(n=0; sqlite3Isspace(z_ప్రారంభం[n]); n++){ ప్రతిజ్ఞా( &z_ప్రారంభం[n]<z_చివర ); }
#అయితే_ముగింపు
  అయితే( sqlite3Isspace(z_ప్రారంభం[0]) ) z_ప్రారంభం++;
  n = (పూర్ణాంకం)(z_చివర - z_ప్రారంభం);
  అయితే( sqlite3Isspace(z_ప్రారంభం[n-1]) ) n--;
  తిరిగి sqlite3DbStrNDup(db, z_ప్రారంభం, n);
}

/*
** Free any prior content in *pz and replace it with a copy of zNew.
*/
శూన్య sqlite3_సమితి_String(అక్షరం **pz, sqlite3 *db, స్థిరరాశి అక్షరం *zNew){
  అక్షరం *z = sqlite3DbStrDup(db, zNew);
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
శూన్య *sqlite3OomFault(sqlite3 *db){
  ఒకవేళ( db->mallocFailed==0 && db->bBenignMalloc==0 ){
    db->mallocFailed = 1;
    ఒకవేళ( db->nVdbeExec>0 ){
      AtomicStore(&db->u1.isInterrupted, 1);
    }
    DisableLookaside;
    ఒకవేళ( db->pParse ){
      Parse *pParse;
      sqlite3_Error_సందేశం(db->pParse, "out of memory");
      db->pParse->rc = SQLITE_NOMEM_BKPT;
      కొరకు(pParse=db->pParse->pOuterParse; pParse; pParse = pParse->pOuterParse){
        pParse->nErr++;
        pParse->rc = SQLITE_NOMEM;
      } 
    }
  }
  తిరిగి 0;
}

/*
** This routine reactivates the memory allocator and clears the
** db->mallocFailed flag as necessary.
**
** The memory allocator is not restarted if there are running
** VDBEs.
*/
శూన్య sqlite3OomClear(sqlite3 *db){
  ఒకవేళ( db->mallocFailed && db->nVdbeExec==0 ){
    db->mallocFailed = 0;
    AtomicStore(&db->u1.isInterrupted, 0);
    ప్రతిజ్ఞా( db->lookaside.bDisable>0 );
    EnableLookaside;
  }
}

/*
** Take actions at the end of an API call to deal with error codes.
*/
స్థిర SQLITE_NOINLINE పూర్ణాంకం apiHandleError(sqlite3 *db, పూర్ణాంకం rc){
  ఒకవేళ( db->mallocFailed || rc==SQLITE_IOERR_NOMEM ){
    sqlite3OomClear(db);
    sqlite3Error(db, SQLITE_NOMEM);
    తిరిగి SQLITE_NOMEM_BKPT;
  }
  తిరిగి rc & db->err_ముసుగు;
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
పూర్ణాంకం sqlite3ApiExit(sqlite3* db, పూర్ణాంకం rc){
  /* If the db handle must hold the connection handle mutex here.
  ** Otherwise the read (and possible write) of db->mallocFailed 
  ** is unsafe, as is the call to sqlite3Error().
  */
  ప్రతిజ్ఞా( db!=0 );
  ప్రతిజ్ఞా( sqlite3_mutex_held(db->mutex) );
  ఒకవేళ( db->mallocFailed || rc ){
    తిరిగి apiHandleError(db, rc);
  }
  తిరిగి 0;
}
