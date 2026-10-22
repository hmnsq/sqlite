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
पूर्णाङ्क sqlite3_release_स्मृतिः(पूर्णाङ्क n){
#यदि_परिभाषितम् SQLITE_ENABLE_स्मृतिः_MANAGEMENT
  प्रत्यागम sqlite3_Pcache_Release_स्मृतिः(n);
#अन्यथा
  /* IMPLEMENTATION-OF: R-34391-24921 The sqlite3_release_memory() routine
  ** is a no-op returning zero if SQLite is not compiled with
  ** SQLITE_ENABLE_MEMORY_MANAGEMENT. */
  अप्रयुक्तम्_PARAMETER(n);
  प्रत्यागम 0;
#यदि_अन्तः
}

/*
** Default value of the hard heap limit.  0 means "no limit".
*/
#यदि_अपरिभाषितम् SQLITE_MAX_स्मृतिः
# define SQLITE_MAX_स्मृतिः 0
#यदि_अन्तः

/*
** State information local to the memory allocation subsystem.
*/
स्थिर SQLITE_WSD संरचना Mem0Global {
  sqlite3_mutex *mutex;         /* Mutex to serialize access */
  sqlite3_int64 alarm_देहली; /* The soft heap limit */
  sqlite3_int64 hard_सीमा;      /* The hard upper bound on memory */

  /*
  ** True if heap is nearly "full" where "full" is defined by the
  ** sqlite3_soft_heap_limit() setting.
  */
  पूर्णाङ्क nearly_पूर्णम्;
} mem0 = { 0, SQLITE_MAX_स्मृतिः, SQLITE_MAX_स्मृतिः, 0 };

#परिभाषय mem0 GLOBAL(संरचना Mem0Global, mem0)

/*
** Return the memory allocator mutex. sqlite3_status() needs it.
*/
sqlite3_mutex *sqlite3MallocMutex(शून्य){
  प्रत्यागम mem0.mutex;
}

#यदि_अपरिभाषितम् SQLITE_OMIT_DEPRECATED
/*
** Deprecated external interface.  It used to set an alarm callback
** that was invoked when memory usage grew too large.  Now it is a
** no-op.
*/
पूर्णाङ्क sqlite3_स्मृतिः_alarm(
  शून्य(*xCallback)(शून्य *pArg, sqlite3_int64 प्रयुक्तम्,पूर्णाङ्क N),
  शून्य *pArg,
  sqlite3_int64 i_देहली
){
  (शून्य)xCallback;
  (शून्य)pArg;
  (शून्य)i_देहली;
  प्रत्यागम SQLITE_OK;
}
#यदि_अन्तः

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
  sqlite3_int64 प्राक्_सीमा;
  sqlite3_int64 excess;
  sqlite3_int64 n_प्रयुक्तम्;
#यदि_अपरिभाषितम् SQLITE_OMIT_AUTOINIT
  पूर्णाङ्क rc = sqlite3_initialize();
  यदि( rc ) प्रत्यागम -1;
#यदि_अन्तः
  sqlite3_mutex_enter(mem0.mutex);
  प्राक्_सीमा = mem0.alarm_देहली;
  यदि( n<0 ){
    sqlite3_mutex_leave(mem0.mutex);
    प्रत्यागम प्राक्_सीमा;
  }
  यदि( mem0.hard_सीमा>0 && (n>mem0.hard_सीमा || n==0) ){
    n = mem0.hard_सीमा;
  }
  mem0.alarm_देहली = n;
  n_प्रयुक्तम् = sqlite3_स्थितिः_मानम्(SQLITE_स्थितिः_स्मृतिः_प्रयुक्तम्);
  AtomicStore(&mem0.nearly_पूर्णम्, n>0 && n<=n_प्रयुक्तम्);
  sqlite3_mutex_leave(mem0.mutex);
  excess = sqlite3_स्मृतिः_प्रयुक्तम्() - n;
  यदि( excess>0 ) sqlite3_release_स्मृतिः((पूर्णाङ्क)(excess & 0x7fffffff));
  प्रत्यागम प्राक्_सीमा;
}
शून्य sqlite3_soft_heap_सीमा(पूर्णाङ्क n){
  यदि( n<0 ) n = 0;
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
  sqlite3_int64 प्राक्_सीमा;
#यदि_अपरिभाषितम् SQLITE_OMIT_AUTOINIT
  पूर्णाङ्क rc = sqlite3_initialize();
  यदि( rc ) प्रत्यागम -1;
#यदि_अन्तः
  sqlite3_mutex_enter(mem0.mutex);
  प्राक्_सीमा = mem0.hard_सीमा;
  यदि( n>=0 ){
    mem0.hard_सीमा = n;
    यदि( n<mem0.alarm_देहली || mem0.alarm_देहली==0 ){
      mem0.alarm_देहली = n;
    }
  }
  sqlite3_mutex_leave(mem0.mutex);
  प्रत्यागम प्राक्_सीमा;
}


/*
** Initialize the memory allocation subsystem.
*/
पूर्णाङ्क sqlite3MallocInit(शून्य){
  पूर्णाङ्क rc;
  यदि( sqlite3GlobalConfig.m.xMalloc==0 ){
    sqlite3_Mem_समुच्चयः_Default();
  }
  mem0.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM);
  यदि( sqlite3GlobalConfig.pPage==0 || sqlite3GlobalConfig.szPage<512
      || sqlite3GlobalConfig.nPage<=0 ){
    sqlite3GlobalConfig.pPage = 0;
    sqlite3GlobalConfig.szPage = 0;
  }
  rc = sqlite3GlobalConfig.m.xInit(sqlite3GlobalConfig.m.p_App_दत्त);
  यदि( rc!=SQLITE_OK ) स्मृतिं_पूरय(&mem0, 0, परिमाण(mem0));
  प्रत्यागम rc;
}

/*
** Return true if the heap is currently under memory pressure - in other
** words if the amount of heap used is close to the limit set by
** sqlite3_soft_heap_limit().
*/
पूर्णाङ्क sqlite3_Heap_Nearly_पूर्णम्(शून्य){
  प्रत्यागम Atomic_आरोपय(&mem0.nearly_पूर्णम्);
}

/*
** Deinitialize the memory allocation subsystem.
*/
शून्य sqlite3_Malloc_अन्तः(शून्य){
  यदि( sqlite3GlobalConfig.m.xShutdown ){
    sqlite3GlobalConfig.m.xShutdown(sqlite3GlobalConfig.m.p_App_दत्त);
  }
  स्मृतिं_पूरय(&mem0, 0, परिमाण(mem0));
}

/*
** Return the amount of memory currently checked out.
*/
sqlite3_int64 sqlite3_स्मृतिः_प्रयुक्तम्(शून्य){
  sqlite3_int64 res, mx;
  sqlite3_status64(SQLITE_स्थितिः_स्मृतिः_प्रयुक्तम्, &res, &mx, 0);
  प्रत्यागम res;
}

/*
** Return the maximum amount of memory that has ever been
** checked out since either the beginning of this process
** or since the most recent reset.
*/
sqlite3_int64 sqlite3_स्मृतिः_highwater(पूर्णाङ्क resetFlag){
  sqlite3_int64 res, mx;
  sqlite3_status64(SQLITE_स्थितिः_स्मृतिः_प्रयुक्तम्, &res, &mx, resetFlag);
  प्रत्यागम mx;
}

/*
** Trigger the alarm 
*/
स्थिर शून्य sqlite3MallocAlarm(पूर्णाङ्क nByte){
  यदि( mem0.alarm_देहली<=0 ) प्रत्यागम;
  sqlite3_mutex_leave(mem0.mutex);
  sqlite3_release_स्मृतिः(nByte);
  sqlite3_mutex_enter(mem0.mutex);
}

#यदि_परिभाषितम् SQLITE_DEBUG
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
#यदि_अन्तः

/*
** Do a memory allocation with statistics and alarms.  Assume the
** lock is already held.
*/
स्थिर शून्य mallocWithAlarm(पूर्णाङ्क n, शून्य **pp){
  शून्य *p;
  पूर्णाङ्क n_पूर्णम्;
  प्रतिज्ञा( sqlite3_mutex_held(mem0.mutex) );
  प्रतिज्ञा( n>0 );

  /* In Firefox (circa 2017-02-08), xRoundup() is remapped to an internal
  ** implementation of malloc_good_size(), which must be called in debug
  ** mode and specifically when the DMD "Dark Matter Detector" is enabled
  ** or else a crash results.  Hence, do not attempt to optimize out the
  ** following xRoundup() call. */
  n_पूर्णम् = sqlite3GlobalConfig.m.xRoundup(n);

  sqlite3_स्थितिः_Highwater(SQLITE_स्थितिः_MALLOC_आकार, n);
  यदि( mem0.alarm_देहली>0 ){
    sqlite3_int64 n_प्रयुक्तम् = sqlite3_स्थितिः_मानम्(SQLITE_स्थितिः_स्मृतिः_प्रयुक्तम्);
    यदि( n_प्रयुक्तम् >= mem0.alarm_देहली - n_पूर्णम् ){
      AtomicStore(&mem0.nearly_पूर्णम्, 1);
      sqlite3MallocAlarm(n_पूर्णम्);
      यदि( mem0.hard_सीमा ){
        n_प्रयुक्तम् = sqlite3_स्थितिः_मानम्(SQLITE_स्थितिः_स्मृतिः_प्रयुक्तम्);
        यदि( n_प्रयुक्तम् >= mem0.hard_सीमा - n_पूर्णम् ){
          test_oom_breakpoint(1);
          *pp = 0;
          प्रत्यागम;
        }
      }
    }अन्यथा{
      AtomicStore(&mem0.nearly_पूर्णम्, 0);
    }
  }
  p = sqlite3GlobalConfig.m.xMalloc(n_पूर्णम्);
#यदि_परिभाषितम् SQLITE_ENABLE_स्मृतिः_MANAGEMENT
  यदि( p==0 && mem0.alarm_देहली>0 ){
    sqlite3MallocAlarm(n_पूर्णम्);
    p = sqlite3GlobalConfig.m.xMalloc(n_पूर्णम्);
  }
#यदि_अन्तः
  यदि( p ){
    n_पूर्णम् = sqlite3_Malloc_आकार(p);
    sqlite3_स्थितिः_ऊर्ध्वम्(SQLITE_स्थितिः_स्मृतिः_प्रयुक्तम्, n_पूर्णम्);
    sqlite3_स्थितिः_ऊर्ध्वम्(SQLITE_स्थितिः_MALLOC_गणनम्, 1);
  }
  *pp = p;
}

/*
** Allocate memory.  This routine is like sqlite3_malloc() except that it
** assumes the memory subsystem has already been initialized.
*/
शून्य *sqlite3Malloc(u64 n){
  शून्य *p;
  यदि( n==0 || n>SQLITE_MAX_आवण्टनम्_आकार ){
    p = 0;
  }अन्यथा यदि( sqlite3GlobalConfig.bMemstat ){
    sqlite3_mutex_enter(mem0.mutex);
    mallocWithAlarm((पूर्णाङ्क)n, &p);
    sqlite3_mutex_leave(mem0.mutex);
  }अन्यथा{
    p = sqlite3GlobalConfig.m.xMalloc((पूर्णाङ्क)n);
  }
  प्रतिज्ञा( EIGHT_BYTE_ALIGNMENT(p) );  /* IMP: R-11148-40995 */
  प्रत्यागम p;
}

/*
** This version of the memory allocation is for use by the application.
** First make sure the memory subsystem is initialized, then do the
** allocation.
*/
शून्य *sqlite3_malloc(पूर्णाङ्क n){
#यदि_अपरिभाषितम् SQLITE_OMIT_AUTOINIT
  यदि( sqlite3_initialize() ) प्रत्यागम 0;
#यदि_अन्तः
  प्रत्यागम n<=0 ? 0 : sqlite3Malloc(n);
}
शून्य *sqlite3_malloc64(sqlite3_uint64 n){
#यदि_अपरिभाषितम् SQLITE_OMIT_AUTOINIT
  यदि( sqlite3_initialize() ) प्रत्यागम 0;
#यदि_अन्तः
  प्रत्यागम sqlite3Malloc(n);
}

/*
** TRUE if p is a lookaside memory allocation from db
*/
#यदि_अपरिभाषितम् SQLITE_OMIT_LOOKASIDE
स्थिर पूर्णाङ्क isLookaside(sqlite3 *db, ध्रुव शून्य *p){
  प्रत्यागम SQLITE_WITHIN(p, db->lookaside.pStart, db->lookaside.p_True_अन्तः);
}
#अन्यथा
#परिभाषय isLookaside(A,B) 0
#यदि_अन्तः

/*
** Return the size of a memory allocation previously obtained from
** sqlite3Malloc() or sqlite3_malloc().
*/
पूर्णाङ्क sqlite3_Malloc_आकार(ध्रुव शून्य *p){
  प्रतिज्ञा( sqlite3MemdebugHasType(p, MEMTYPE_HEAP) );
  प्रत्यागम sqlite3GlobalConfig.m.x_आकार((शून्य*)p);
}
स्थिर पूर्णाङ्क lookaside_Malloc_आकार(sqlite3 *db, ध्रुव शून्य *p){
#यदि_अपरिभाषितम् SQLITE_OMIT_TWOSIZE_LOOKASIDE    
  प्रत्यागम p<db->lookaside.p_मध्यः ? db->lookaside.szTrue : LOOKASIDE_SMALL;
#अन्यथा
  प्रत्यागम db->lookaside.szTrue;
#यदि_अन्तः  
}
पूर्णाङ्क sqlite3_Db_Malloc_आकार(sqlite3 *db, ध्रुव शून्य *p){
  प्रतिज्ञा( p!=0 );
#यदि_परिभाषितम् SQLITE_DEBUG
  यदि( db==0 ){
    प्रतिज्ञा( sqlite3MemdebugNoType(p, (u8)~MEMTYPE_HEAP) );
    प्रतिज्ञा( sqlite3MemdebugHasType(p, MEMTYPE_HEAP) );
  }अन्यथा यदि( !isLookaside(db,p) ){
    प्रतिज्ञा( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
    प्रतिज्ञा( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  }
#यदि_अन्तः
  यदि( db ){
    यदि( ((uptr)p)<(uptr)(db->lookaside.p_True_अन्तः) ){
#यदि_अपरिभाषितम् SQLITE_OMIT_TWOSIZE_LOOKASIDE
      यदि( ((uptr)p)>=(uptr)(db->lookaside.p_मध्यः) ){
        प्रतिज्ञा( sqlite3_mutex_held(db->mutex) );
        प्रत्यागम LOOKASIDE_SMALL;
      }
#यदि_अन्तः
      यदि( ((uptr)p)>=(uptr)(db->lookaside.pStart) ){
        प्रतिज्ञा( sqlite3_mutex_held(db->mutex) );
        प्रत्यागम db->lookaside.szTrue;
      }
    }
  }
  प्रत्यागम sqlite3GlobalConfig.m.x_आकार((शून्य*)p);
}
sqlite3_uint64 sqlite3_msize(शून्य *p){
  प्रतिज्ञा( sqlite3MemdebugNoType(p, (u8)~MEMTYPE_HEAP) );
  प्रतिज्ञा( sqlite3MemdebugHasType(p, MEMTYPE_HEAP) );
  प्रत्यागम p ? sqlite3GlobalConfig.m.x_आकार(p) : 0;
}

/*
** Free memory previously obtained from sqlite3Malloc().
*/
शून्य sqlite3_free(शून्य *p){
  यदि( p==0 ) प्रत्यागम;  /* IMP: R-49053-54554 */
  प्रतिज्ञा( sqlite3MemdebugHasType(p, MEMTYPE_HEAP) );
  प्रतिज्ञा( sqlite3MemdebugNoType(p, (u8)~MEMTYPE_HEAP) );
  यदि( sqlite3GlobalConfig.bMemstat ){
    sqlite3_mutex_enter(mem0.mutex);
    sqlite3_स्थितिः_अधस्तात्(SQLITE_स्थितिः_स्मृतिः_प्रयुक्तम्, sqlite3_Malloc_आकार(p));
    sqlite3_स्थितिः_अधस्तात्(SQLITE_स्थितिः_MALLOC_गणनम्, 1);
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
स्थिर SQLITE_NOINLINE शून्य measure_आवण्टनम्_आकार(sqlite3 *db, शून्य *p){
  *db->pn_अष्टकानि_Freed += sqlite3_Db_Malloc_आकार(db,p);
}

/*
** Free memory that might be associated with a particular database
** connection.  Calling sqlite3DbFree(D,X) for X==0 is a harmless no-op.
** The sqlite3DbFreeNN(D,X) version requires that X be non-NULL.
*/
शून्य sqlite3DbFreeNN(sqlite3 *db, शून्य *p){
  प्रतिज्ञा( db==0 || sqlite3_mutex_held(db->mutex) );
  प्रतिज्ञा( p!=0 );
  यदि( db ){
    यदि( ((uptr)p)<(uptr)(db->lookaside.p_अन्तः) ){
#यदि_अपरिभाषितम् SQLITE_OMIT_TWOSIZE_LOOKASIDE
      यदि( ((uptr)p)>=(uptr)(db->lookaside.p_मध्यः) ){
        LookasideSlot *pBuf = (LookasideSlot*)p;
        प्रतिज्ञा( db->pn_अष्टकानि_Freed==0 );
#यदि_परिभाषितम् SQLITE_DEBUG
        स्मृतिं_पूरय(p, 0xaa, LOOKASIDE_SMALL);  /* Trash freed content */
#यदि_अन्तः
        pBuf->p_अग्र = db->lookaside.pSmallFree;
        db->lookaside.pSmallFree = pBuf;
        प्रत्यागम;
      }
#यदि_अन्तः /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
      यदि( ((uptr)p)>=(uptr)(db->lookaside.pStart) ){
        LookasideSlot *pBuf = (LookasideSlot*)p;
        प्रतिज्ञा( db->pn_अष्टकानि_Freed==0 );
#यदि_परिभाषितम् SQLITE_DEBUG
        स्मृतिं_पूरय(p, 0xaa, db->lookaside.szTrue);  /* Trash freed content */
#यदि_अन्तः
        pBuf->p_अग्र = db->lookaside.pFree;
        db->lookaside.pFree = pBuf;
        प्रत्यागम;
      }
    }
    यदि( db->pn_अष्टकानि_Freed ){
      measure_आवण्टनम्_आकार(db, p);
      प्रत्यागम;
    }
  }
  प्रतिज्ञा( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  प्रतिज्ञा( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  प्रतिज्ञा( db!=0 || sqlite3MemdebugNoType(p, MEMTYPE_LOOKASIDE) );
  sqlite3_Memdebug_समुच्चयः_Type(p, MEMTYPE_HEAP);
  sqlite3_free(p);
}
शून्य sqlite3DbNNFreeNN(sqlite3 *db, शून्य *p){
  प्रतिज्ञा( db!=0 );
  प्रतिज्ञा( sqlite3_mutex_held(db->mutex) );
  प्रतिज्ञा( p!=0 );
  यदि( ((uptr)p)<(uptr)(db->lookaside.p_अन्तः) ){
#यदि_अपरिभाषितम् SQLITE_OMIT_TWOSIZE_LOOKASIDE
    यदि( ((uptr)p)>=(uptr)(db->lookaside.p_मध्यः) ){
      LookasideSlot *pBuf = (LookasideSlot*)p;
      प्रतिज्ञा( db->pn_अष्टकानि_Freed==0 );
#यदि_परिभाषितम् SQLITE_DEBUG
      स्मृतिं_पूरय(p, 0xaa, LOOKASIDE_SMALL);  /* Trash freed content */
#यदि_अन्तः
      pBuf->p_अग्र = db->lookaside.pSmallFree;
      db->lookaside.pSmallFree = pBuf;
      प्रत्यागम;
    }
#यदि_अन्तः /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
    यदि( ((uptr)p)>=(uptr)(db->lookaside.pStart) ){
      LookasideSlot *pBuf = (LookasideSlot*)p;
      प्रतिज्ञा( db->pn_अष्टकानि_Freed==0 );
#यदि_परिभाषितम् SQLITE_DEBUG
      स्मृतिं_पूरय(p, 0xaa, db->lookaside.szTrue);  /* Trash freed content */
#यदि_अन्तः
      pBuf->p_अग्र = db->lookaside.pFree;
      db->lookaside.pFree = pBuf;
      प्रत्यागम;
    }
  }
  यदि( db->pn_अष्टकानि_Freed ){
    measure_आवण्टनम्_आकार(db, p);
    प्रत्यागम;
  }
  प्रतिज्ञा( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  प्रतिज्ञा( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
  sqlite3_Memdebug_समुच्चयः_Type(p, MEMTYPE_HEAP);
  sqlite3_free(p);
}
शून्य sqlite3DbFree(sqlite3 *db, शून्य *p){
  प्रतिज्ञा( db==0 || sqlite3_mutex_held(db->mutex) );
  यदि( p ) sqlite3DbFreeNN(db, p);
}

/*
** Change the size of an existing memory allocation
*/
शून्य *sqlite3Realloc(शून्य *pOld, u64 n_अष्टकानि){
  पूर्णाङ्क nOld, nNew, nDiff;
  शून्य *pNew;
  प्रतिज्ञा( sqlite3MemdebugHasType(pOld, MEMTYPE_HEAP) );
  प्रतिज्ञा( sqlite3MemdebugNoType(pOld, (u8)~MEMTYPE_HEAP) );
  यदि( pOld==0 ){
    प्रत्यागम sqlite3Malloc(n_अष्टकानि); /* IMP: R-04300-56712 */
  }
  यदि( n_अष्टकानि==0 ){
    sqlite3_free(pOld); /* IMP: R-26507-47431 */
    प्रत्यागम 0;
  }
  यदि( n_अष्टकानि>SQLITE_MAX_आवण्टनम्_आकार ){
    प्रत्यागम 0;
  }
  nOld = sqlite3_Malloc_आकार(pOld);
  /* IMPLEMENTATION-OF: R-46199-30249 SQLite guarantees that the second
  ** argument to xRealloc is always a value returned by a prior call to
  ** xRoundup. */
  nNew = sqlite3GlobalConfig.m.xRoundup((पूर्णाङ्क)n_अष्टकानि);
  यदि( nOld==nNew ){
    pNew = pOld;
  }अन्यथा यदि( sqlite3GlobalConfig.bMemstat ){
    sqlite3_int64 n_प्रयुक्तम्;
    sqlite3_mutex_enter(mem0.mutex);
    sqlite3_स्थितिः_Highwater(SQLITE_स्थितिः_MALLOC_आकार, (पूर्णाङ्क)n_अष्टकानि);
    nDiff = nNew - nOld;
    यदि( nDiff>0 && (n_प्रयुक्तम् = sqlite3_स्थितिः_मानम्(SQLITE_स्थितिः_स्मृतिः_प्रयुक्तम्)) >= 
          mem0.alarm_देहली-nDiff ){
      sqlite3MallocAlarm(nDiff);
      यदि( mem0.hard_सीमा>0 && n_प्रयुक्तम् >= mem0.hard_सीमा - nDiff ){
        sqlite3_mutex_leave(mem0.mutex);
        test_oom_breakpoint(1);
        प्रत्यागम 0;
      }
    }
    pNew = sqlite3GlobalConfig.m.xRealloc(pOld, nNew);
#यदि_परिभाषितम् SQLITE_ENABLE_स्मृतिः_MANAGEMENT
    यदि( pNew==0 && mem0.alarm_देहली>0 ){
      sqlite3MallocAlarm((पूर्णाङ्क)n_अष्टकानि);
      pNew = sqlite3GlobalConfig.m.xRealloc(pOld, nNew);
    }
#यदि_अन्तः
    यदि( pNew ){
      nNew = sqlite3_Malloc_आकार(pNew);
      sqlite3_स्थितिः_ऊर्ध्वम्(SQLITE_स्थितिः_स्मृतिः_प्रयुक्तम्, nNew-nOld);
    }
    sqlite3_mutex_leave(mem0.mutex);
  }अन्यथा{
    pNew = sqlite3GlobalConfig.m.xRealloc(pOld, nNew);
  }
  प्रतिज्ञा( EIGHT_BYTE_ALIGNMENT(pNew) ); /* IMP: R-11148-40995 */
  प्रत्यागम pNew;
}

/*
** The public interface to sqlite3Realloc.  Make sure that the memory
** subsystem is initialized prior to invoking sqliteRealloc.
*/
शून्य *sqlite3_realloc(शून्य *pOld, पूर्णाङ्क n){
#यदि_अपरिभाषितम् SQLITE_OMIT_AUTOINIT
  यदि( sqlite3_initialize() ) प्रत्यागम 0;
#यदि_अन्तः
  यदि( n<0 ) n = 0;  /* IMP: R-26507-47431 */
  प्रत्यागम sqlite3Realloc(pOld, n);
}
शून्य *sqlite3_realloc64(शून्य *pOld, sqlite3_uint64 n){
#यदि_अपरिभाषितम् SQLITE_OMIT_AUTOINIT
  यदि( sqlite3_initialize() ) प्रत्यागम 0;
#यदि_अन्तः
  प्रत्यागम sqlite3Realloc(pOld, n);
}


/*
** Allocate and zero memory.
*/ 
शून्य *sqlite3MallocZero(u64 n){
  शून्य *p = sqlite3Malloc(n);
  यदि( p ){
    स्मृतिं_पूरय(p, 0, (size_t)n);
  }
  प्रत्यागम p;
}

/*
** Allocate and zero memory.  If the allocation fails, make
** the mallocFailed flag in the connection pointer.
*/
शून्य *sqlite3DbMallocZero(sqlite3 *db, u64 n){
  शून्य *p;
  testcase( db==0 );
  p = sqlite3DbMallocRaw(db, n);
  यदि( p ) स्मृतिं_पूरय(p, 0, (size_t)n);
  प्रत्यागम p;
}


/* Finish the work of sqlite3DbMallocRawNN for the unusual and
** slower case when the allocation cannot be fulfilled using lookaside.
*/
स्थिर SQLITE_NOINLINE शून्य *dbMallocRawFinish(sqlite3 *db, u64 n){
  शून्य *p;
  प्रतिज्ञा( db!=0 );
  p = sqlite3Malloc(n);
  यदि( !p ) sqlite3OomFault(db);
  sqlite3_Memdebug_समुच्चयः_Type(p, 
         (db->lookaside.bDisable==0) ? MEMTYPE_LOOKASIDE : MEMTYPE_HEAP);
  प्रत्यागम p;
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
  यदि( db ) प्रत्यागम sqlite3DbMallocRawNN(db, n);
  p = sqlite3Malloc(n);
  sqlite3_Memdebug_समुच्चयः_Type(p, MEMTYPE_HEAP);
  प्रत्यागम p;
}
शून्य *sqlite3DbMallocRawNN(sqlite3 *db, u64 n){
#यदि_अपरिभाषितम् SQLITE_OMIT_LOOKASIDE
  LookasideSlot *pBuf;
  प्रतिज्ञा( db!=0 );
  प्रतिज्ञा( sqlite3_mutex_held(db->mutex) );
  प्रतिज्ञा( db->pn_अष्टकानि_Freed==0 );
  यदि( n>db->lookaside.sz ){
    यदि( !db->lookaside.bDisable ){
      db->lookaside.anStat[1]++;      
    }अन्यथा यदि( db->mallocFailed ){
      प्रत्यागम 0;
    }
    प्रत्यागम dbMallocRawFinish(db, n);
  }
#यदि_अपरिभाषितम् SQLITE_OMIT_TWOSIZE_LOOKASIDE
  यदि( n<=LOOKASIDE_SMALL ){
    यदि( (pBuf = db->lookaside.pSmallFree)!=0 ){
      db->lookaside.pSmallFree = pBuf->p_अग्र;
      db->lookaside.anStat[0]++;
      प्रत्यागम (शून्य*)pBuf;
    }अन्यथा यदि( (pBuf = db->lookaside.pSmallInit)!=0 ){
      db->lookaside.pSmallInit = pBuf->p_अग्र;
      db->lookaside.anStat[0]++;
      प्रत्यागम (शून्य*)pBuf;
    }
  }
#यदि_अन्तः
  यदि( (pBuf = db->lookaside.pFree)!=0 ){
    db->lookaside.pFree = pBuf->p_अग्र;
    db->lookaside.anStat[0]++;
    प्रत्यागम (शून्य*)pBuf;
  }अन्यथा यदि( (pBuf = db->lookaside.pInit)!=0 ){
    db->lookaside.pInit = pBuf->p_अग्र;
    db->lookaside.anStat[0]++;
    प्रत्यागम (शून्य*)pBuf;
  }अन्यथा{
    db->lookaside.anStat[2]++;
  }
#अन्यथा
  प्रतिज्ञा( db!=0 );
  प्रतिज्ञा( sqlite3_mutex_held(db->mutex) );
  प्रतिज्ञा( db->pn_अष्टकानि_Freed==0 );
  यदि( db->mallocFailed ){
    प्रत्यागम 0;
  }
#यदि_अन्तः
  प्रत्यागम dbMallocRawFinish(db, n);
}

/* Forward declaration */
स्थिर SQLITE_NOINLINE शून्य *dbReallocFinish(sqlite3 *db, शून्य *p, u64 n);

/*
** Resize the block of memory pointed to by p to n bytes. If the
** resize fails, set the mallocFailed flag in the connection object.
*/
शून्य *sqlite3DbRealloc(sqlite3 *db, शून्य *p, u64 n){
  प्रतिज्ञा( db!=0 );
  यदि( p==0 ) प्रत्यागम sqlite3DbMallocRawNN(db, n);
  प्रतिज्ञा( sqlite3_mutex_held(db->mutex) );
  यदि( ((uptr)p)<(uptr)db->lookaside.p_अन्तः ){
#यदि_अपरिभाषितम् SQLITE_OMIT_TWOSIZE_LOOKASIDE
    यदि( ((uptr)p)>=(uptr)db->lookaside.p_मध्यः ){
      यदि( n<=LOOKASIDE_SMALL ) प्रत्यागम p;
    }अन्यथा
#यदि_अन्तः
    यदि( ((uptr)p)>=(uptr)db->lookaside.pStart ){
      यदि( n<=db->lookaside.szTrue ) प्रत्यागम p;
    }
  }
  प्रत्यागम dbReallocFinish(db, p, n);
}
स्थिर SQLITE_NOINLINE शून्य *dbReallocFinish(sqlite3 *db, शून्य *p, u64 n){
  शून्य *pNew = 0;
  प्रतिज्ञा( db!=0 );
  प्रतिज्ञा( p!=0 );
  यदि( db->mallocFailed==0 ){
    यदि( isLookaside(db, p) ){
      pNew = sqlite3DbMallocRawNN(db, n);
      यदि( pNew ){
        स्मृतिं_प्रतिलिख(pNew, p, lookaside_Malloc_आकार(db, p));
        sqlite3DbFree(db, p);
      }
    }अन्यथा{
      प्रतिज्ञा( sqlite3MemdebugHasType(p, (MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
      प्रतिज्ञा( sqlite3MemdebugNoType(p, (u8)~(MEMTYPE_LOOKASIDE|MEMTYPE_HEAP)) );
      sqlite3_Memdebug_समुच्चयः_Type(p, MEMTYPE_HEAP);
      pNew = sqlite3Realloc(p, n);
      यदि( !pNew ){
        sqlite3OomFault(db);
      }
      sqlite3_Memdebug_समुच्चयः_Type(pNew,
            (db->lookaside.bDisable==0 ? MEMTYPE_LOOKASIDE : MEMTYPE_HEAP));
    }
  }
  प्रत्यागम pNew;
}

/*
** Attempt to reallocate p.  If the reallocation fails, then free p
** and set the mallocFailed flag in the database connection.
*/
शून्य *sqlite3DbReallocOrFree(sqlite3 *db, शून्य *p, u64 n){
  शून्य *pNew;
  pNew = sqlite3DbRealloc(db, p, n);
  यदि( !pNew ){
    sqlite3DbFree(db, p);
  }
  प्रत्यागम pNew;
}

/*
** Make a copy of a string in memory obtained from sqliteMalloc(). These 
** functions call sqlite3MallocRaw() directly instead of sqliteMalloc(). This
** is because when memory debugging is turned on, these two functions are 
** called via macros that record the current file and line number in the
** ThreadData structure.
*/
वर्ण *sqlite3DbStrDup(sqlite3 *db, ध्रुव वर्ण *z){
  वर्ण *zNew;
  size_t n;
  यदि( z==0 ){
    प्रत्यागम 0;
  }
  n = वाक्यदैर्घ्यम्(z) + 1;
  zNew = sqlite3DbMallocRaw(db, n);
  यदि( zNew ){
    स्मृतिं_प्रतिलिख(zNew, z, n);
  }
  प्रत्यागम zNew;
}
वर्ण *sqlite3DbStrNDup(sqlite3 *db, ध्रुव वर्ण *z, u64 n){
  वर्ण *zNew;
  प्रतिज्ञा( db!=0 );
  प्रतिज्ञा( z!=0 || n==0 );
  प्रतिज्ञा( (n&0x7fffffff)==n );
  zNew = z ? sqlite3DbMallocRawNN(db, n+1) : 0;
  यदि( zNew ){
    स्मृतिं_प्रतिलिख(zNew, z, (size_t)n);
    zNew[n] = 0;
  }
  प्रत्यागम zNew;
}

/*
** The text between zStart and zEnd represents a phrase within a larger
** SQL statement.  Make a copy of this phrase in space obtained form
** sqlite3DbMalloc().  Omit leading and trailing whitespace.
*/
वर्ण *sqlite3_Db_विस्तृतिः_Dup(sqlite3 *db, ध्रुव वर्ण *zStart, ध्रुव वर्ण *z_अन्तः){
  पूर्णाङ्क n;
#यदि_परिभाषितम् SQLITE_DEBUG
  /* Because of the way the parser works, the span is guaranteed to contain
  ** at least one non-space character */
  प्रति(n=0; sqlite3Isspace(zStart[n]); n++){ प्रतिज्ञा( &zStart[n]<z_अन्तः ); }
#यदि_अन्तः
  यावत्( sqlite3Isspace(zStart[0]) ) zStart++;
  n = (पूर्णाङ्क)(z_अन्तः - zStart);
  यावत्( sqlite3Isspace(zStart[n-1]) ) n--;
  प्रत्यागम sqlite3DbStrNDup(db, zStart, n);
}

/*
** Free any prior content in *pz and replace it with a copy of zNew.
*/
शून्य sqlite3_समुच्चयः_String(वर्ण **pz, sqlite3 *db, ध्रुव वर्ण *zNew){
  वर्ण *z = sqlite3DbStrDup(db, zNew);
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
  यदि( db->mallocFailed==0 && db->bBenignMalloc==0 ){
    db->mallocFailed = 1;
    यदि( db->nVdbeExec>0 ){
      AtomicStore(&db->u1.isInterrupted, 1);
    }
    DisableLookaside;
    यदि( db->pParse ){
      Parse *pParse;
      sqlite3_Error_सन्देशः(db->pParse, "out of memory");
      db->pParse->rc = SQLITE_NOMEM_BKPT;
      प्रति(pParse=db->pParse->pOuterParse; pParse; pParse = pParse->pOuterParse){
        pParse->nErr++;
        pParse->rc = SQLITE_NOMEM;
      } 
    }
  }
  प्रत्यागम 0;
}

/*
** This routine reactivates the memory allocator and clears the
** db->mallocFailed flag as necessary.
**
** The memory allocator is not restarted if there are running
** VDBEs.
*/
शून्य sqlite3OomClear(sqlite3 *db){
  यदि( db->mallocFailed && db->nVdbeExec==0 ){
    db->mallocFailed = 0;
    AtomicStore(&db->u1.isInterrupted, 0);
    प्रतिज्ञा( db->lookaside.bDisable>0 );
    EnableLookaside;
  }
}

/*
** Take actions at the end of an API call to deal with error codes.
*/
स्थिर SQLITE_NOINLINE पूर्णाङ्क apiHandleError(sqlite3 *db, पूर्णाङ्क rc){
  यदि( db->mallocFailed || rc==SQLITE_IOERR_NOMEM ){
    sqlite3OomClear(db);
    sqlite3Error(db, SQLITE_NOMEM);
    प्रत्यागम SQLITE_NOMEM_BKPT;
  }
  प्रत्यागम rc & db->err_आवरणम्;
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
पूर्णाङ्क sqlite3ApiExit(sqlite3* db, पूर्णाङ्क rc){
  /* If the db handle must hold the connection handle mutex here.
  ** Otherwise the read (and possible write) of db->mallocFailed 
  ** is unsafe, as is the call to sqlite3Error().
  */
  प्रतिज्ञा( db!=0 );
  प्रतिज्ञा( sqlite3_mutex_held(db->mutex) );
  यदि( db->mallocFailed || rc ){
    प्रत्यागम apiHandleError(db, rc);
  }
  प्रत्यागम 0;
}
