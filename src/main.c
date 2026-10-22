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
** Main file for the SQLite library.  The routines in this file
** implement the programmer interface to the library.  Routines in
** other files are for internal use by SQLite and should not be
** accessed by users of the library.
*/
#شامل "sqliteInt.h"

#اگر_تعریف_شدہ SQLITE_ENABLE_FTS3
# include "fts3.h"
#اگر_ختم
#اگر_تعریف_شدہ SQLITE_ENABLE_RTREE
# include "rtree.h"
#اگر_ختم
#اگر defined(SQLITE_ENABLE_ICU) || defined(SQLITE_ENABLE_ICU_COLLATIONS)
# include "sqliteicu.h"
#اگر_ختم

/*
** This is an extension initializer that is a no-op and always
** succeeds, except that it fails if the fault-simulation is set
** to 500.
*/
ساکن صحیح sqlite3TestExtInit(sqlite3 *db){
  (خالی)db;
  واپس sqlite3FaultSim(500);
}


/*
** Forward declarations of external module initializer functions
** for modules that need them.
*/
#اگر_تعریف_شدہ SQLITE_ENABLE_FTS5
صحیح sqlite3Fts5Init(sqlite3*);
#اگر_ختم
#اگر_تعریف_شدہ SQLITE_ENABLE_STMTVTAB
صحیح sqlite3StmtVtabInit(sqlite3*);
#اگر_ختم
#اگر_تعریف_شدہ SQLITE_EXTRA_AUTOEXT
صحیح SQLITE_EXTRA_AUTOEXT(sqlite3*);
#اگر_ختم
/*
** An array of pointers to extension initializer functions for
** built-in extensions.
*/
ساکن صحیح (*ثابت sqlite3BuiltinExtensions[])(sqlite3*) = {
#اگر_تعریف_شدہ SQLITE_ENABLE_FTS3
  sqlite3Fts3Init,
#اگر_ختم
#اگر_تعریف_شدہ SQLITE_ENABLE_FTS5
  sqlite3Fts5Init,
#اگر_ختم
#اگر defined(SQLITE_ENABLE_ICU) || defined(SQLITE_ENABLE_ICU_COLLATIONS)
  sqlite3IcuInit,
#اگر_ختم
#اگر_تعریف_شدہ SQLITE_ENABLE_RTREE
  sqlite3RtreeInit,
#اگر_ختم
#اگر_تعریف_شدہ SQLITE_ENABLE_DBPAGE_VTAB
  sqlite3_Dbpage_رجسٹر,
#اگر_ختم
#اگر_تعریف_شدہ SQLITE_ENABLE_DBSTAT_VTAB
  sqlite3_Dbstat_رجسٹر,
#اگر_ختم
  sqlite3TestExtInit,
#اگر_تعریف_شدہ SQLITE_ENABLE_STMTVTAB
  sqlite3StmtVtabInit,
#اگر_ختم
#اگر_تعریف_شدہ SQLITE_ENABLE_BYTECODE_VTAB
  sqlite3VdbeBytecodeVtabInit,
#اگر_ختم
#اگر_تعریف_شدہ SQLITE_EXTRA_AUTOEXT
  SQLITE_EXTRA_AUTOEXT,
#اگر_ختم
};

#اگر_غیر_تعریف_شدہ SQLITE_AMALGAMATION
/* IMPLEMENTATION-OF: R-46656-45156 The sqlite3_version[] string constant
** contains the text of SQLITE_VERSION macro.
*/
ثابت حرف sqlite3_نسخہ[] = SQLITE_نسخہ;
#اگر_ختم

/* IMPLEMENTATION-OF: R-53536-42575 The sqlite3_libversion() function returns
** a pointer to the to the sqlite3_version[] string constant.
*/
ثابت حرف *sqlite3_libversion(خالی){ واپس sqlite3_نسخہ; }

/* IMPLEMENTATION-OF: R-25063-23286 The sqlite3_sourceid() function returns a
** pointer to a string constant whose value is the same as the
** SQLITE_SOURCE_ID C preprocessor macro. Except if SQLite is built using
** an edited copy of the amalgamation, then the last four characters of
** the hash might be different from SQLITE_SOURCE_ID.
*/
ثابت حرف *sqlite3_sourceid(خالی){ واپس SQLITE_ماخذ_ID; }

/* IMPLEMENTATION-OF: R-35210-63508 The sqlite3_libversion_number() function
** returns an integer equal to SQLITE_VERSION_NUMBER.
*/
صحیح sqlite3_libversion_نمبر(خالی){ واپس SQLITE_نسخہ_نمبر; }

/* IMPLEMENTATION-OF: R-20790-14025 The sqlite3_threadsafe() function returns
** zero if and only if SQLite was compiled with mutexing code omitted due to
** the SQLITE_THREADSAFE compile-time option being set to 0.
*/
صحیح sqlite3_threadsafe(خالی){ واپس SQLITE_THREADSAFE; }

/*
** When compiling the test fixture or with debugging enabled (on Win32),
** this variable being set to non-zero will cause OSTRACE macros to emit
** extra diagnostic information.
*/
#اگر_تعریف_شدہ SQLITE_HAVE_OS_TRACE
# ifndef SQLITE_DEBUG_OS_TRACE
#   define SQLITE_DEBUG_OS_TRACE 0
# endif
  صحیح sqlite3OSTrace = SQLITE_DEBUG_OS_TRACE;
#اگر_ختم

#اگر !defined(SQLITE_OMIT_TRACE) && defined(SQLITE_ENABLE_IOTRACE)
/*
** If the following function pointer is not NULL and if
** SQLITE_ENABLE_IOTRACE is enabled, then messages describing
** I/O active are written using this function.  These messages
** are intended for debugging activity only.
*/
SQLITE_API خالی (SQLITE_CDECL *sqlite3IoTrace)(ثابت حرف*, ...) = 0;
#اگر_ختم

/*
** If the following global variable points to a string which is the
** name of a directory, then that directory will be used to store
** temporary files.
**
** See also the "PRAGMA temp_store_directory" SQL command.
*/
حرف *sqlite3_عارضی_directory = 0;

/*
** If the following global variable points to a string which is the
** name of a directory, then that directory will be used to store
** all database files specified with a relative pathname.
**
** See also the "PRAGMA data_store_directory" SQL command.
*/
حرف *sqlite3_ڈیٹا_directory = 0;

/*
** Initialize SQLite. 
**
** The sqlite3_initialize() routine must be called to initialize the
** memory allocation, VFS, and mutex subsystems prior to doing any
** serious work.  As long as you do not compile with SQLITE_OMIT_AUTOINIT
** this routine will be called automatically by key routines such as
** sqlite3_open(). 
**
** This routine is a no-op except on its very first call for the process,
** or for the first call after a call to sqlite3_shutdown.  Most calls
** to sqlite3_initialize() are, in fact, no-ops.  For that reason, the
** routine is broken into two pieces:
**
**     sqlite3Initialize()     Does the actual work of initialization
**
**     sqlite3_initialize()    Checks to see if initialization is needed
**                             and invokes sqlite3Initialize() if it is.
**
** The sqlite3_initialize() interface is called frequently, but
** sqlite3Initialize() runs rarely.  The function is broken up this way
** to avoid wasting CPU cycles with unnecessary stack setup for local
** variables in cases where it is not needed.
**
** The first thread to call sqlite3_initialize() runs the initialization to
** completion. If subsequent threads call sqlite3_initialize() before the first
** thread has finished the initialization process, then the subsequent
** threads must block until the first thread finishes with the initialization.
**
** The first thread might call this routine recursively.  Recursive
** calls to this routine should not block, of course.  Otherwise the
** initialization process would never complete.
**
** Let X be the first thread to enter sqlite3_initialize().  Let Y be some
** other thread.  While the initial invocation of sqlite3_initialize() by X
** is incomplete, it is required that:
**
**    *  Calls to sqlite3_initialize() from Y must block until the outer-most
**       call by X completes.
**
**    *  Recursive calls to sqlite3_initialize() from thread X return
**       immediately without blocking.
*/
ساکن SQLITE_NOINLINE صحیح sqlite3Initialize(خالی){
  MUTEX_LOGIC( sqlite3_mutex *pMainMtx; )   /* The main static mutex */
  صحیح rc;                                   /* Result code */
#اگر_تعریف_شدہ SQLITE_EXTRA_INIT
  صحیح b_تسلسل_Extra_Init = 0;                    /* Extra initialization needed */
#اگر_ختم

  /* If the following assert() fails on some obscure processor/compiler
  ** combination to warn that SQLite has been mis-compiled.  If you hit
  ** this assert(), that means you need to recompile with the
  ** -DSQLITE_PTRSIZE=n compile-time option to set the correct pointer size.
  */
  أكد( SQLITE_PTRSIZE==حجم(حرف*) );

  /* Make sure the mutex subsystem is initialized.  If unable to
  ** initialize the mutex subsystem, return early with the error.
  ** If the system is so sick that we are unable to allocate a mutex,
  ** there is not much SQLite is going to be able to do.
  **
  ** The mutex subsystem must take care of serializing its own
  ** initialization.
  */
  rc = sqlite3MutexInit();
  اگر( rc ) واپس rc;

  /* Initialize the malloc() system and the recursive pInitMutex mutex.
  ** This operation is protected by the STATIC_MAIN mutex.  Note that
  ** MutexAlloc() is called for a static mutex prior to initializing the
  ** malloc subsystem - this implies that the allocation of a static
  ** mutex must not require support from the malloc subsystem.
  */
  MUTEX_LOGIC( pMainMtx = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  sqlite3_mutex_enter(pMainMtx);
  sqlite3GlobalConfig.isMutexInit = 1;
  اگر( !sqlite3GlobalConfig.isMallocInit ){
    rc = sqlite3MallocInit();
  }
  اگر( rc==SQLITE_OK ){
    sqlite3GlobalConfig.isMallocInit = 1;
    اگر( !sqlite3GlobalConfig.pInitMutex ){
      sqlite3GlobalConfig.pInitMutex =
           sqlite3MutexAlloc(SQLITE_MUTEX_RECURSIVE);
      اگر( sqlite3GlobalConfig.bCoreMutex && !sqlite3GlobalConfig.pInitMutex ){
        rc = SQLITE_NOMEM_BKPT;
      }
    }
  }
  اگر( rc==SQLITE_OK ){
    sqlite3GlobalConfig.nRefInitMutex++;
  }
  sqlite3_mutex_leave(pMainMtx);

  /* If rc is not SQLITE_OK at this point, then either the malloc
  ** subsystem could not be initialized or the system failed to allocate
  ** the pInitMutex mutex. Return an error in either case.  */
  اگر( rc!=SQLITE_OK ){
    واپس rc;
  }

  /* Do the rest of the initialization under the recursive mutex so
  ** that we will be able to handle recursive calls into
  ** sqlite3_initialize().  The recursive calls normally come through
  ** sqlite3_os_init() when it invokes sqlite3_vfs_register(), but other
  ** recursive calls might also be possible.
  **
  ** IMPLEMENTATION-OF: R-00140-37445 SQLite automatically serializes calls
  ** to the xInit method, so the xInit method need not be threadsafe.
  **
  ** The following mutex is what serializes access to the appdef pcache xInit
  ** methods.  The sqlite3_pcache_methods.xInit() all is embedded in the
  ** call to sqlite3PcacheInitialize().
  */
  sqlite3_mutex_enter(sqlite3GlobalConfig.pInitMutex);
  اگر( sqlite3GlobalConfig.isInit==0 && sqlite3GlobalConfig.اندر_پیش_رفت==0 ){
    sqlite3GlobalConfig.اندر_پیش_رفت = 1;
#اگر_تعریف_شدہ SQLITE_ENABLE_SQLLOG
    {
      خارجی خالی sqlite3_init_sqllog(خالی);
      sqlite3_init_sqllog();
    }
#اگر_ختم
    memset(&sqlite3BuiltinFunctions, 0, حجم(sqlite3BuiltinFunctions));
    sqlite3_رجسٹر_Builtin_Functions();
    اگر( sqlite3GlobalConfig.isPCacheInit==0 ){
      rc = sqlite3PcacheInitialize();
    }
    اگر( rc==SQLITE_OK ){
      sqlite3GlobalConfig.isPCacheInit = 1;
      rc = sqlite3OsInit();
    }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_DESERIALIZE
    اگر( rc==SQLITE_OK ){
      rc = sqlite3MemdbInit();
    }
#اگر_ختم
    اگر( rc==SQLITE_OK ){
      sqlite3_PCache_بفر_تیاری( sqlite3GlobalConfig.p_صفحہ,
          sqlite3GlobalConfig.sz_صفحہ, sqlite3GlobalConfig.n_صفحہ);
#اگر_تعریف_شدہ SQLITE_EXTRA_INIT_MUTEXED
      {
        صحیح SQLITE_EXTRA_INIT_MUTEXED(ثابت حرف*);
        rc = SQLITE_EXTRA_INIT_MUTEXED(0);
      }
#اگر_ختم
    }
    اگر( rc==SQLITE_OK ){
      sqlite3_حافظہ_Barrier();
      sqlite3GlobalConfig.isInit = 1;
#اگر_تعریف_شدہ SQLITE_EXTRA_INIT
      b_تسلسل_Extra_Init = 1;
#اگر_ختم
    }
    sqlite3GlobalConfig.اندر_پیش_رفت = 0;
  }
  sqlite3_mutex_leave(sqlite3GlobalConfig.pInitMutex);

  /* Go back under the static mutex and clean up the recursive
  ** mutex to prevent a resource leak.
  */
  sqlite3_mutex_enter(pMainMtx);
  sqlite3GlobalConfig.nRefInitMutex--;
  اگر( sqlite3GlobalConfig.nRefInitMutex<=0 ){
    أكد( sqlite3GlobalConfig.nRefInitMutex==0 );
    sqlite3_mutex_free(sqlite3GlobalConfig.pInitMutex);
    sqlite3GlobalConfig.pInitMutex = 0;
  }
  sqlite3_mutex_leave(pMainMtx);

  /* The following is just a sanity check to make sure SQLite has
  ** been compiled correctly.  It is important to run this code, but
  ** we don't want to run it too often and soak up CPU cycles for no
  ** reason.  So we run it once during initialization.
  */
#اگر_غیر_تعریف_شدہ NDEBUG
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_FLOATING_نقطہ
  /* This section of code's only "output" is via assert() statements. */
  اگر( rc==SQLITE_OK ){
    u64 x = (((u64)1)<<63)-1;
    دوہرا y;
    أكد(حجم(x)==8);
    أكد(حجم(x)==حجم(y));
    حافظہ_نقل(&y, &x, 8);
    أكد( sqlite3IsNaN(y) );
  }
#اگر_ختم
#اگر_ختم

  /* Do extra initialization steps requested by the SQLITE_EXTRA_INIT
  ** compile-time option.
  */
#اگر_تعریف_شدہ SQLITE_EXTRA_INIT
  اگر( b_تسلسل_Extra_Init ){
    صحیح SQLITE_EXTRA_INIT(ثابت حرف*);
    rc = SQLITE_EXTRA_INIT(0);
  }
#اگر_ختم
  واپس rc;
}
SQLITE_API صحیح sqlite3_initialize(خالی){
  /* If this build does not support writable static data (WSD) natively
  ** then we have to invoke the (application-supplied) WSD initialization
  ** routine before doing anything else. */
#اگر_تعریف_شدہ SQLITE_OMIT_WSD
  صحیح rc = sqlite3_wsd_init(4096, 24);
  اگر( rc!=SQLITE_OK ){
    واپس rc;
  }
#اگر_ختم

  اگر( sqlite3GlobalConfig.isInit ){
    /* SQLite has already been initialized.  Fast early-out. */
    sqlite3_حافظہ_Barrier();
    واپس SQLITE_OK;
  }ورنہ{
    /* Invoke sqlite3Initialize() to do the actual work. */
    واپس sqlite3Initialize();
  }
}

/*
** Undo the effects of sqlite3_initialize().  Must not be called while
** there are outstanding database connections or memory allocations or
** while any part of SQLite is otherwise in use in any thread.  This
** routine is not threadsafe.  But it is safe to invoke this routine
** on when SQLite is already shut down.  If SQLite is already shut down
** when this routine is invoked, then this routine is a harmless no-op.
*/
صحیح sqlite3_shutdown(خالی){
#اگر_تعریف_شدہ SQLITE_OMIT_WSD
  صحیح rc = sqlite3_wsd_init(4096, 24);
  اگر( rc!=SQLITE_OK ){
    واپس rc;
  }
#اگر_ختم

  اگر( sqlite3GlobalConfig.isInit ){
#اگر_تعریف_شدہ SQLITE_EXTRA_SHUTDOWN
    خالی SQLITE_EXTRA_SHUTDOWN(خالی);
    SQLITE_EXTRA_SHUTDOWN();
#اگر_ختم
    sqlite3_os_انتہا();
    sqlite3_reset_auto_extension();
    sqlite3GlobalConfig.isInit = 0;
  }
  اگر( sqlite3GlobalConfig.isPCacheInit ){
    sqlite3PcacheShutdown();
    sqlite3GlobalConfig.isPCacheInit = 0;
  }
  اگر( sqlite3GlobalConfig.isMallocInit ){
    sqlite3_Malloc_انتہا();
    sqlite3GlobalConfig.isMallocInit = 0;

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_SHUTDOWN_DIRECTORIES
    /* The heap subsystem has now been shutdown and these values are supposed
    ** to be NULL or point to memory that was obtained from sqlite3_malloc(),
    ** which would rely on that heap subsystem; therefore, make sure these
    ** values cannot refer to heap memory that was just invalidated when the
    ** heap subsystem was shutdown.  This is only done if the current call to
    ** this function resulted in the heap subsystem actually being shutdown.
    */
    sqlite3_ڈیٹا_directory = 0;
    sqlite3_عارضی_directory = 0;
#اگر_ختم
  }
  اگر( sqlite3GlobalConfig.isMutexInit ){
    sqlite3_Mutex_انتہا();
    sqlite3GlobalConfig.isMutexInit = 0;
  }

  واپس SQLITE_OK;
}

/*
** This API allows applications to modify the global configuration of
** the SQLite library at run-time.
**
** This routine should only be called when there are no outstanding
** database connections or memory allocations.  This routine is not
** threadsafe.  Failure to heed these warnings can lead to unpredictable
** behavior.
*/
صحیح sqlite3_config(صحیح op, ...){
  va_list ap;
  صحیح rc = SQLITE_OK;

  /* sqlite3_config() normally returns SQLITE_MISUSE if it is invoked while
  ** the SQLite library is in use.  Except, a few selected opcodes
  ** are allowed.
  */
  اگر( sqlite3GlobalConfig.isInit ){
    ساکن ثابت u64 mAnytimeConfigOption = 0
       | MASKBIT64( SQLITE_CONFIG_LOG )
       | MASKBIT64( SQLITE_CONFIG_PCACHE_HDRSZ )
    ;
    اگر( op<0 || op>63 || (MASKBIT64(op) & mAnytimeConfigOption)==0 ){
      واپس SQLITE_MISUSE_BKPT;
    }
    testcase( op==SQLITE_CONFIG_LOG );
    testcase( op==SQLITE_CONFIG_PCACHE_HDRSZ );
  }

  va_start(ap, op);
  انتخاب( op ){

    /* Mutex configuration options are only available in a threadsafe
    ** compile.
    */
#اگر defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE>0  /* IMP: R-54466-46756 */
    حالت SQLITE_CONFIG_SINGLETHREAD: {
      /* EVIDENCE-OF: R-02748-19096 This option sets the threading mode to
      ** Single-thread. */
      sqlite3GlobalConfig.bCoreMutex = 0;  /* Disable mutex on core */
      sqlite3GlobalConfig.b_بھرا_Mutex = 0;  /* Disable mutex on connections */
      توڑو;
    }
#اگر_ختم
#اگر defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE>0 /* IMP: R-20520-54086 */
    حالت SQLITE_CONFIG_MULTITHREAD: {
      /* EVIDENCE-OF: R-14374-42468 This option sets the threading mode to
      ** Multi-thread. */
      sqlite3GlobalConfig.bCoreMutex = 1;  /* Enable mutex on core */
      sqlite3GlobalConfig.b_بھرا_Mutex = 0;  /* Disable mutex on connections */
      توڑو;
    }
#اگر_ختم
#اگر defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE>0 /* IMP: R-59593-21810 */
    حالت SQLITE_CONFIG_SERIALIZED: {
      /* EVIDENCE-OF: R-41220-51800 This option sets the threading mode to
      ** Serialized. */
      sqlite3GlobalConfig.bCoreMutex = 1;  /* Enable mutex on core */
      sqlite3GlobalConfig.b_بھرا_Mutex = 1;  /* Enable mutex on connections */
      توڑو;
    }
#اگر_ختم
#اگر defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE>0 /* IMP: R-63666-48755 */
    حالت SQLITE_CONFIG_MUTEX: {
      /* Specify an alternative mutex implementation */
      sqlite3GlobalConfig.mutex = *va_arg(ap, sqlite3_mutex_methods*);
      توڑو;
    }
#اگر_ختم
#اگر defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE>0 /* IMP: R-14450-37597 */
    حالت SQLITE_CONFIG_GETMUTEX: {
      /* Retrieve the current mutex implementation */
      *va_arg(ap, sqlite3_mutex_methods*) = sqlite3GlobalConfig.mutex;
      توڑو;
    }
#اگر_ختم

    حالت SQLITE_CONFIG_MALLOC: {
      /* EVIDENCE-OF: R-55594-21030 The SQLITE_CONFIG_MALLOC option takes a
      ** single argument which is a pointer to an instance of the
      ** sqlite3_mem_methods structure. The argument specifies alternative
      ** low-level memory allocation routines to be used in place of the memory
      ** allocation routines built into SQLite. */
      sqlite3GlobalConfig.m = *va_arg(ap, sqlite3_mem_methods*);
      توڑو;
    }
    حالت SQLITE_CONFIG_GETMALLOC: {
      /* EVIDENCE-OF: R-51213-46414 The SQLITE_CONFIG_GETMALLOC option takes a
      ** single argument which is a pointer to an instance of the
      ** sqlite3_mem_methods structure. The sqlite3_mem_methods structure is
      ** filled with the currently defined memory allocation routines. */
      اگر( sqlite3GlobalConfig.m.xMalloc==0 ) sqlite3_Mem_سیٹ_Default();
      *va_arg(ap, sqlite3_mem_methods*) = sqlite3GlobalConfig.m;
      توڑو;
    }
    حالت SQLITE_CONFIG_MEMSTATUS: {
      أكد( !sqlite3GlobalConfig.isInit );  /* Cannot change at runtime */
      /* EVIDENCE-OF: R-61275-35157 The SQLITE_CONFIG_MEMSTATUS option takes
      ** single argument of type int, interpreted as a boolean, which enables
      ** or disables the collection of memory allocation statistics. */
      sqlite3GlobalConfig.bMemstat = va_arg(ap, صحیح);
      توڑو;
    }
    حالت SQLITE_CONFIG_SMALL_MALLOC: {
      sqlite3GlobalConfig.bSmallMalloc = va_arg(ap, صحیح)!=0;
      توڑو;
    }
    حالت SQLITE_CONFIG_PAGECACHE: {
      /* EVIDENCE-OF: R-18761-36601 There are three arguments to
      ** SQLITE_CONFIG_PAGECACHE: A pointer to 8-byte aligned memory (pMem),
      ** the size of each page cache line (sz), and the number of cache lines
      ** (N). */
      sqlite3GlobalConfig.p_صفحہ = va_arg(ap, خالی*);
      sqlite3GlobalConfig.sz_صفحہ = va_arg(ap, صحیح);
      sqlite3GlobalConfig.n_صفحہ = va_arg(ap, صحیح);
      توڑو;
    }
    حالت SQLITE_CONFIG_PCACHE_HDRSZ: {
      /* EVIDENCE-OF: R-39100-27317 The SQLITE_CONFIG_PCACHE_HDRSZ option takes
      ** a single parameter which is a pointer to an integer and writes into
      ** that integer the number of extra bytes per page required for each page
      ** in SQLITE_CONFIG_PAGECACHE. */
      *va_arg(ap, صحیح*) =
          sqlite3_سرخی_سائز_Btree() +
          sqlite3_سرخی_سائز_Pcache() +
          sqlite3_سرخی_سائز_Pcache1();
      توڑو;
    }

    حالت SQLITE_CONFIG_PCACHE: {
      /* no-op */
      توڑو;
    }
    حالت SQLITE_CONFIG_GETPCACHE: {
      /* now an error */
      rc = SQLITE_ERROR;
      توڑو;
    }

    حالت SQLITE_CONFIG_PCACHE2: {
      /* EVIDENCE-OF: R-63325-48378 The SQLITE_CONFIG_PCACHE2 option takes a
      ** single argument which is a pointer to an sqlite3_pcache_methods2
      ** object. This object specifies the interface to a custom page cache
      ** implementation. */
      sqlite3GlobalConfig.pcache2 = *va_arg(ap, sqlite3_pcache_methods2*);
      توڑو;
    }
    حالت SQLITE_CONFIG_GETPCACHE2: {
      /* EVIDENCE-OF: R-22035-46182 The SQLITE_CONFIG_GETPCACHE2 option takes a
      ** single argument which is a pointer to an sqlite3_pcache_methods2
      ** object. SQLite copies of the current page cache implementation into
      ** that object. */
      اگر( sqlite3GlobalConfig.pcache2.xInit==0 ){
        sqlite3_PCache_سیٹ_Default();
      }
      *va_arg(ap, sqlite3_pcache_methods2*) = sqlite3GlobalConfig.pcache2;
      توڑو;
    }

/* EVIDENCE-OF: R-06626-12911 The SQLITE_CONFIG_HEAP option is only
** available if SQLite is compiled with either SQLITE_ENABLE_MEMSYS3 or
** SQLITE_ENABLE_MEMSYS5 and returns SQLITE_ERROR if invoked otherwise. */
#اگر defined(SQLITE_ENABLE_MEMSYS3) || defined(SQLITE_ENABLE_MEMSYS5)
    حالت SQLITE_CONFIG_ڈھیر: {
      /* EVIDENCE-OF: R-19854-42126 There are three arguments to
      ** SQLITE_CONFIG_HEAP: An 8-byte aligned pointer to the memory, the
      ** number of bytes in the memory buffer, and the minimum allocation size.
      */
      sqlite3GlobalConfig.p_ڈھیر = va_arg(ap, خالی*);
      sqlite3GlobalConfig.n_ڈھیر = va_arg(ap, صحیح);
      sqlite3GlobalConfig.mnReq = va_arg(ap, صحیح);

      اگر( sqlite3GlobalConfig.mnReq<1 ){
        sqlite3GlobalConfig.mnReq = 1;
      }ورنہ اگر( sqlite3GlobalConfig.mnReq>(1<<12) ){
        /* cap min request size at 2^12 */
        sqlite3GlobalConfig.mnReq = (1<<12);
      }

      اگر( sqlite3GlobalConfig.p_ڈھیر==0 ){
        /* EVIDENCE-OF: R-49920-60189 If the first pointer (the memory pointer)
        ** is NULL, then SQLite reverts to using its default memory allocator
        ** (the system malloc() implementation), undoing any prior invocation of
        ** SQLITE_CONFIG_MALLOC.
        **
        ** Setting sqlite3GlobalConfig.m to all zeros will cause malloc to
        ** revert to its default implementation when sqlite3_initialize() is run
        */
        memset(&sqlite3GlobalConfig.m, 0, حجم(sqlite3GlobalConfig.m));
      }ورنہ{
        /* EVIDENCE-OF: R-61006-08918 If the memory pointer is not NULL then the
        ** alternative memory allocator is engaged to handle all of SQLites
        ** memory allocation needs. */
#اگر_تعریف_شدہ SQLITE_ENABLE_MEMSYS3
        sqlite3GlobalConfig.m = *sqlite3MemGetMemsys3();
#اگر_ختم
#اگر_تعریف_شدہ SQLITE_ENABLE_MEMSYS5
        sqlite3GlobalConfig.m = *sqlite3MemGetMemsys5();
#اگر_ختم
      }
      توڑو;
    }
#اگر_ختم

    حالت SQLITE_CONFIG_LOOKASIDE: {
      sqlite3GlobalConfig.szLookaside = va_arg(ap, صحیح);
      sqlite3GlobalConfig.nLookaside = va_arg(ap, صحیح);
      توڑو;
    }
   
    /* Record a pointer to the logger function and its first argument.
    ** The default is NULL.  Logging is disabled if the function pointer is
    ** NULL.
    */
    حالت SQLITE_CONFIG_LOG: {
      /* MSVC is picky about pulling func ptrs from va lists.
      ** http://support.microsoft.com/kb/47961
      ** sqlite3GlobalConfig.xLog = va_arg(ap, void(*)(void*,int,const char*));
      */
      قسمتعریف خالی(*LOGFUNC_t)(خالی*,صحیح,ثابت حرف*);
      LOGFUNC_t xLog = va_arg(ap, LOGFUNC_t);
      خالی *pLogArg = va_arg(ap, خالی*);
      AtomicStore(&sqlite3GlobalConfig.xLog, xLog);
      AtomicStore(&sqlite3GlobalConfig.pLogArg, pLogArg);
      توڑو;
    }

    /* EVIDENCE-OF: R-55548-33817 The compile-time setting for URI filenames
    ** can be changed at start-time using the
    ** sqlite3_config(SQLITE_CONFIG_URI,1) or
    ** sqlite3_config(SQLITE_CONFIG_URI,0) configuration calls.
    */
    حالت SQLITE_CONFIG_URI: {
      /* EVIDENCE-OF: R-25451-61125 The SQLITE_CONFIG_URI option takes a single
      ** argument of type int. If non-zero, then URI handling is globally
      ** enabled. If the parameter is zero, then URI handling is globally
      ** disabled. */
      صحیح bOpenUri = va_arg(ap, صحیح);
      AtomicStore(&sqlite3GlobalConfig.bOpenUri, bOpenUri);
      توڑو;
    }

    حالت SQLITE_CONFIG_COVERING_اشاریہ_جائزہ: {
      /* EVIDENCE-OF: R-36592-02772 The SQLITE_CONFIG_COVERING_INDEX_SCAN
      ** option takes a single integer argument which is interpreted as a
      ** boolean in order to enable or disable the use of covering indices for
      ** full table scans in the query optimizer. */
      sqlite3GlobalConfig.bUseCis = va_arg(ap, صحیح);
      توڑو;
    }

#اگر_تعریف_شدہ SQLITE_ENABLE_SQLLOG
    حالت SQLITE_CONFIG_SQLLOG: {
      قسمتعریف خالی(*SQLLOGFUNC_t)(خالی*, sqlite3*, ثابت حرف*, صحیح);
      sqlite3GlobalConfig.xSqllog = va_arg(ap, SQLLOGFUNC_t);
      sqlite3GlobalConfig.pSqllogArg = va_arg(ap, خالی *);
      توڑو;
    }
#اگر_ختم

    حالت SQLITE_CONFIG_MMAP_سائز: {
      /* EVIDENCE-OF: R-58063-38258 SQLITE_CONFIG_MMAP_SIZE takes two 64-bit
      ** integer (sqlite3_int64) values that are the default mmap size limit
      ** (the default setting for PRAGMA mmap_size) and the maximum allowed
      ** mmap size limit. */
      sqlite3_int64 szMmap = va_arg(ap, sqlite3_int64);
      sqlite3_int64 mxMmap = va_arg(ap, sqlite3_int64);
      /* EVIDENCE-OF: R-53367-43190 If either argument to this option is
      ** negative, then that argument is changed to its compile-time default.
      **
      ** EVIDENCE-OF: R-34993-45031 The maximum allowed mmap size will be
      ** silently truncated if necessary so that it does not exceed the
      ** compile-time maximum mmap size set by the SQLITE_MAX_MMAP_SIZE
      ** compile-time option.
      */
      اگر( mxMmap<0 || mxMmap>SQLITE_MAX_MMAP_سائز ){
        mxMmap = SQLITE_MAX_MMAP_سائز;
      }
      اگر( szMmap<0 ) szMmap = SQLITE_DEFAULT_MMAP_سائز;
      اگر( szMmap>mxMmap) szMmap = mxMmap;
      sqlite3GlobalConfig.mxMmap = mxMmap;
      sqlite3GlobalConfig.szMmap = szMmap;
      توڑو;
    }

#اگر SQLITE_OS_WIN && defined(SQLITE_WIN32_MALLOC) /* IMP: R-04780-55815 */
    حالت SQLITE_CONFIG_WIN32_HEAPSIZE: {
      /* EVIDENCE-OF: R-34926-03360 SQLITE_CONFIG_WIN32_HEAPSIZE takes a 32-bit
      ** unsigned integer value that specifies the maximum size of the created
      ** heap. */
      sqlite3GlobalConfig.n_ڈھیر = va_arg(ap, صحیح);
      توڑو;
    }
#اگر_ختم

    حالت SQLITE_CONFIG_PMASZ: {
      sqlite3GlobalConfig.szPma = va_arg(ap, غیرعلامتی صحیح);
      توڑو;
    }

    حالت SQLITE_CONFIG_STMTJRNL_SPILL: {
      sqlite3GlobalConfig.nStmtSpill = va_arg(ap, صحیح);
      توڑو;
    }

#اگر_تعریف_شدہ SQLITE_ENABLE_SORTER_REFERENCES
    حالت SQLITE_CONFIG_SORTERREF_سائز: {
      صحیح iVal = va_arg(ap, صحیح);
      اگر( iVal<0 ){
        iVal = SQLITE_DEFAULT_SORTERREF_سائز;
      }
      sqlite3GlobalConfig.szSorterRef = (u32)iVal;
      توڑو;
    }
#اگر_ختم /* SQLITE_ENABLE_SORTER_REFERENCES */

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_DESERIALIZE
    حالت SQLITE_CONFIG_MEMDB_MAXSIZE: {
      sqlite3GlobalConfig.mx_Memdb_سائز = va_arg(ap, sqlite3_int64);
      توڑو;
    }
#اگر_ختم /* SQLITE_OMIT_DESERIALIZE */

    حالت SQLITE_CONFIG_ROWID_اندر_VIEW: {
      صحیح *pVal = va_arg(ap,صحیح*);
#اگر_تعریف_شدہ SQLITE_ALLOW_ROWID_اندر_VIEW
      اگر( 0==*pVal ) sqlite3GlobalConfig.mNoVisibleRowid = TF_NoVisibleRowid;
      اگر( 1==*pVal ) sqlite3GlobalConfig.mNoVisibleRowid = 0;
      *pVal = (sqlite3GlobalConfig.mNoVisibleRowid==0);
#ورنہ
      *pVal = 0;
#اگر_ختم
      توڑو;
    }

    طےشدہ: {
      rc = SQLITE_ERROR;
      توڑو;
    }
  }
  va_end(ap);
  واپس rc;
}

/*
** Set up the lookaside buffers for a database connection.
** Return SQLITE_OK on success. 
** If lookaside is already active, return SQLITE_BUSY.
**
** The sz parameter is the number of bytes in each lookaside slot.
** The cnt parameter is the number of slots.  If pBuf is NULL the
** space for the lookaside memory is obtained from sqlite3_malloc()
** or similar.  If pBuf is not NULL then it is sz*cnt bytes of memory
** to use for the lookaside memory.
*/
ساکن صحیح تیاری_Lookaside(
  sqlite3 *db,    /* Database connection being configured */
  خالی *pBuf,     /* Memory to use for lookaside.  May be NULL */
  صحیح sz,         /* Desired size of each lookaside memory slot */
  صحیح cnt         /* Number of slots to allocate */
){
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_LOOKASIDE
  خالی *p_آغاز;          /* Start of the lookaside buffer */
  sqlite3_int64 szAlloc; /* Total space set aside for lookaside memory */
  صحیح nBig;              /* Number of full-size slots */
  صحیح nSm;               /* Number smaller LOOKASIDE_SMALL-byte slots */
 
  اگر( sqlite3_Lookaside_مستعمل(db,0)>0 ){
    واپس SQLITE_BUSY;
  }
  /* Free any existing lookaside buffer for this handle before
  ** allocating a new one so we don't have to have space for
  ** both at the same time.
  */
  اگر( db->lookaside.bMalloced ){
    sqlite3_free(db->lookaside.p_آغاز);
  }
  /* The size of a lookaside slot after ROUNDDOWN8 needs to be larger
  ** than a pointer and small enough to fit in a u16.
  */
  sz = ROUNDDOWN8(sz);
  اگر( sz<=(صحیح)حجم(Lookaside_خانہ*) ) sz = 0;
  اگر( sz>65528 ) sz = 65528;
  /* Count must be at least 1 to be useful, but not so large as to use
  ** more than 0x7fff0000 total bytes for lookaside. */
  اگر( cnt<1 ) cnt = 0;
  اگر( sz>0 && cnt>(0x7fff0000/sz) ) cnt = 0x7fff0000/sz;
  szAlloc = (i64)sz*(i64)cnt;
  اگر( szAlloc==0 ){
    sz = 0;
    p_آغاز = 0;
  }ورنہ اگر( pBuf==0 ){
    sqlite3BeginBenignMalloc();
    p_آغاز = sqlite3Malloc( szAlloc );
    sqlite3_انتہا_Benign_Malloc();
    اگر( p_آغاز ) szAlloc = sqlite3_Malloc_سائز(p_آغاز);
  }ورنہ{
    p_آغاز = pBuf;
  }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_TWOSIZE_LOOKASIDE
  اگر( sz>=LOOKASIDE_SMALL*3 ){
    nBig = szAlloc/(3*LOOKASIDE_SMALL+sz);
    nSm = (szAlloc - (i64)sz*(i64)nBig)/LOOKASIDE_SMALL;
  }ورنہ اگر( sz>=LOOKASIDE_SMALL*2 ){
    nBig = szAlloc/(LOOKASIDE_SMALL+sz);
    nSm = (szAlloc - (i64)sz*(i64)nBig)/LOOKASIDE_SMALL;
  }ورنہ
#اگر_ختم /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
  اگر( sz>0 ){
    nBig = szAlloc/sz;
    nSm = 0;
  }ورنہ{
    nBig = nSm = 0;
  }
  db->lookaside.p_آغاز = p_آغاز;
  db->lookaside.pInit = 0;
  db->lookaside.pFree = 0;
  db->lookaside.sz = (u16)sz;
  db->lookaside.szTrue = (u16)sz;
  اگر( p_آغاز ){
    صحیح i;
    Lookaside_خانہ *p;
    أكد( sz > (صحیح)حجم(Lookaside_خانہ*) );
    p = (Lookaside_خانہ*)p_آغاز;
    برائے(i=0; i<nBig; i++){
      p->p_اگلا = db->lookaside.pInit;
      db->lookaside.pInit = p;
      p = (Lookaside_خانہ*)&((u8*)p)[sz];
    }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_TWOSIZE_LOOKASIDE
    db->lookaside.pSmallInit = 0;
    db->lookaside.pSmallFree = 0;
    db->lookaside.p_درمیان = p;
    برائے(i=0; i<nSm; i++){
      p->p_اگلا = db->lookaside.pSmallInit;
      db->lookaside.pSmallInit = p;
      p = (Lookaside_خانہ*)&((u8*)p)[LOOKASIDE_SMALL];
    }
#اگر_ختم /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
    أكد( ((uptr)p)<=szAlloc + (uptr)p_آغاز );
    db->lookaside.p_انتہا = p;
    db->lookaside.bDisable = 0;
    db->lookaside.bMalloced = pBuf==0 ?1:0;
    db->lookaside.n_خانہ = nBig+nSm;
  }ورنہ{
    db->lookaside.p_آغاز = 0;
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_TWOSIZE_LOOKASIDE
    db->lookaside.pSmallInit = 0;
    db->lookaside.pSmallFree = 0;
    db->lookaside.p_درمیان = 0;
#اگر_ختم /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
    db->lookaside.p_انتہا = 0;
    db->lookaside.bDisable = 1;
    db->lookaside.sz = 0;
    db->lookaside.bMalloced = 0;
    db->lookaside.n_خانہ = 0;
  }
  db->lookaside.p_True_انتہا = db->lookaside.p_انتہا;
  أكد( sqlite3_Lookaside_مستعمل(db,0)==0 );
#اگر_ختم /* SQLITE_OMIT_LOOKASIDE */
  واپس SQLITE_OK;
}

/*
** Return the mutex associated with a database connection.
*/
sqlite3_mutex *sqlite3_db_mutex(sqlite3 *db){
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db) ){
    (خالی)SQLITE_MISUSE_BKPT;
    واپس 0;
  }
#اگر_ختم
  واپس db->mutex;
}

/*
** Free up as much memory as we can from the given database
** connection.
*/
صحیح sqlite3_db_release_حافظہ(sqlite3 *db){
  صحیح i;

#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db) ) واپس SQLITE_MISUSE_BKPT;
#اگر_ختم
  sqlite3_mutex_enter(db->mutex);
  sqlite3BtreeEnterAll(db);
  برائے(i=0; i<db->nDb; i++){
    Btree *pBt = db->aDb[i].pBt;
    اگر( pBt ){
      Pager *pPager = sqlite3BtreePager(pBt);
      sqlite3PagerShrink(pPager);
    }
  }
  sqlite3BtreeLeaveAll(db);
  sqlite3_mutex_leave(db->mutex);
  واپس SQLITE_OK;
}

/*
** Flush any dirty pages in the pager-cache for any attached database
** to disk.
*/
صحیح sqlite3_db_cacheflush(sqlite3 *db){
  صحیح i;
  صحیح rc = SQLITE_OK;
  صحیح b_نظر_آیا_Busy = 0;

#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db) ) واپس SQLITE_MISUSE_BKPT;
#اگر_ختم
  sqlite3_mutex_enter(db->mutex);
  sqlite3BtreeEnterAll(db);
  برائے(i=0; rc==SQLITE_OK && i<db->nDb; i++){
    Btree *pBt = db->aDb[i].pBt;
    اگر( pBt && sqlite3BtreeTxnState(pBt)==SQLITE_TXN_WRITE ){
      Pager *pPager = sqlite3BtreePager(pBt);
      rc = sqlite3PagerFlush(pPager);
      اگر( rc==SQLITE_BUSY ){
        b_نظر_آیا_Busy = 1;
        rc = SQLITE_OK;
      }
    }
  }
  sqlite3BtreeLeaveAll(db);
  sqlite3_mutex_leave(db->mutex);
  واپس ((rc==SQLITE_OK && b_نظر_آیا_Busy) ? SQLITE_BUSY : rc);
}

/*
** Configuration settings for an individual database connection
*/
صحیح sqlite3_db_config(sqlite3 *db, صحیح op, ...){
  va_list ap;
  صحیح rc;

#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db) ) واپس SQLITE_MISUSE_BKPT;
#اگر_ختم
  sqlite3_mutex_enter(db->mutex);
  va_start(ap, op);
  انتخاب( op ){
    حالت SQLITE_DBCONFIG_MAINDBNAME: {
      /* IMP: R-06824-28531 */
      /* IMP: R-36257-52125 */
      db->aDb[0].zDbSName = va_arg(ap,حرف*);
      rc = SQLITE_OK;
      توڑو;
    }
    حالت SQLITE_DBCONFIG_LOOKASIDE: {
      خالی *pBuf = va_arg(ap, خالی*); /* IMP: R-26835-10964 */
      صحیح sz = va_arg(ap, صحیح);       /* IMP: R-47871-25994 */
      صحیح cnt = va_arg(ap, صحیح);      /* IMP: R-04460-53386 */
      rc = تیاری_Lookaside(db, pBuf, sz, cnt);
      توڑو;
    }
    حالت SQLITE_DBCONFIG_FP_DIGITS: {
      صحیح n_اندر = va_arg(ap, صحیح);
      صحیح *p_برآمد = va_arg(ap, صحیح*);
      اگر( n_اندر>3 && n_اندر<24 ) db->nFpDigit = (u8)n_اندر;
      اگر( p_برآمد ) *p_برآمد = db->nFpDigit;
      rc = SQLITE_OK;
      توڑو;
    }
    طےشدہ: {
      ساکن ثابت ساخت {
        صحیح op;      /* The opcode */
        u64 نقاب;    /* Mask of the bit in sqlite3.flags to set/clear */
      } aFlagOp[] = {
        { SQLITE_DBCONFIG_ENABLE_FKEY,           SQLITE_Foreign_کلیدیں    },
        { SQLITE_DBCONFIG_ENABLE_TRIGGER,        SQLITE_EnableTrigger  },
        { SQLITE_DBCONFIG_ENABLE_VIEW,           SQLITE_EnableView     },
        { SQLITE_DBCONFIG_ENABLE_FTS3_TOKENIZER, SQLITE_Fts3Tokenizer  },
        { SQLITE_DBCONFIG_ENABLE_بھرو_EXTENSION, SQLITE_بھرو_Extension  },
        { SQLITE_DBCONFIG_NO_CKPT_ON_CLOSE,      SQLITE_NoCkptOnClose  },
        { SQLITE_DBCONFIG_ENABLE_QPSG,           SQLITE_EnableQPSG     },
        { SQLITE_DBCONFIG_TRIGGER_EQP,           SQLITE_TriggerEQP     },
        { SQLITE_DBCONFIG_RESET_DATABASE,        SQLITE_ResetDatabase  },
        { SQLITE_DBCONFIG_DEFENSIVE,             SQLITE_Defensive      },
        { SQLITE_DBCONFIG_WRITABLE_SCHEMA,       SQLITE_WriteSchema|
                                                 SQLITE_NoSchemaError  },
        { SQLITE_DBCONFIG_LEGACY_ALTER_جدول,    SQLITE_LegacyAlter    },
        { SQLITE_DBCONFIG_DQS_DDL,               SQLITE_DqsDDL         },
        { SQLITE_DBCONFIG_DQS_DML,               SQLITE_DqsDML         },
        { SQLITE_DBCONFIG_LEGACY_FILE_FORMAT,    SQLITE_LegacyFileFmt  },
        { SQLITE_DBCONFIG_TRUSTED_SCHEMA,        SQLITE_TrustedSchema  },
        { SQLITE_DBCONFIG_STMT_SCANSTATUS,       SQLITE_Stmt_جائزہ_حالت },
        { SQLITE_DBCONFIG_الٹ_SCANORDER,     SQLITE_الٹ_ترتیب   },
        { SQLITE_DBCONFIG_ENABLE_ATTACH_CREATE,  SQLITE_AttachCreate   },
        { SQLITE_DBCONFIG_ENABLE_ATTACH_WRITE,   SQLITE_AttachWrite    },
        { SQLITE_DBCONFIG_ENABLE_COMMENTS,       SQLITE_Comments       },
      };
      غیرعلامتی صحیح i;
      rc = SQLITE_ERROR; /* IMP: R-42790-23372 */
      برائے(i=0; i<Array_سائز(aFlagOp); i++){
        اگر( aFlagOp[i].op==op ){
          صحیح onoff = va_arg(ap, صحیح);
          صحیح *pRes = va_arg(ap, صحیح*);
          u64 oldFlags = db->flags;
          اگر( onoff>0 ){
            db->flags |= aFlagOp[i].نقاب;
          }ورنہ اگر( onoff==0 ){
            db->flags &= ~(u64)aFlagOp[i].نقاب;
          }
          اگر( oldFlags!=db->flags ){
            sqlite3ExpirePreparedStatements(db, 0);
          }
          اگر( pRes ){
            *pRes = (db->flags & aFlagOp[i].نقاب)!=0;
          }
          rc = SQLITE_OK;
          توڑو;
        }
      }
      توڑو;
    }
  }
  va_end(ap);
  sqlite3_mutex_leave(db->mutex);
  واپس rc;
}

/*
** This is the default collating function named "BINARY" which is always
** available.
*/
صحیح sqlite3_ثنائی_Compare(
  خالی *Not_مستعمل,
  صحیح nKey1, ثابت خالی *pKey1,
  صحیح nKey2, ثابت خالی *pKey2
){
  صحیح rc, n;
  غیر_مستعمل_PARAMETER(Not_مستعمل);
  n = nKey1<nKey2 ? nKey1 : nKey2;
  /* EVIDENCE-OF: R-65033-28449 The built-in BINARY collation compares
  ** strings byte by byte using the memcmp() function from the standard C
  ** library. */
  أكد( pKey1 && pKey2 );
  rc = memcmp(pKey1, pKey2, n);
  اگر( rc==0 ){
    rc = nKey1 - nKey2;
  }
  واپس rc;
}

/*
** This is the collating function named "RTRIM" which is always
** available.  Ignore trailing spaces.
*/
ساکن صحیح rtrimCollFunc(
  خالی *pUser,
  صحیح nKey1, ثابت خالی *pKey1,
  صحیح nKey2, ثابت خالی *pKey2
){
  ثابت u8 *pK1 = (ثابت u8*)pKey1;
  ثابت u8 *pK2 = (ثابت u8*)pKey2;
  جب( nKey1 && pK1[nKey1-1]==' ' ) nKey1--;
  جب( nKey2 && pK2[nKey2-1]==' ' ) nKey2--;
  واپس sqlite3_ثنائی_Compare(pUser, nKey1, pKey1, nKey2, pKey2);
}

/*
** Return true if CollSeq is the default built-in BINARY.
*/
#اگر !SQLITE_استعمال_کرتا_INLINE
صحیح sqlite3_Is_ثنائی(ثابت CollSeq *p){
  أكد( p==0 || p->xCmp!=sqlite3_ثنائی_Compare
       || strcmp(p->zName,"BINARY")==0 );
  واپس p==0 || p->xCmp==sqlite3_ثنائی_Compare;
}
#اگر_ختم

/*
** Another built-in collating sequence: NOCASE.
**
** This collating sequence is intended to be used for "case independent
** comparison". SQLite's knowledge of upper and lower case equivalents
** extends only to the 26 characters used in the English language.
**
** At the moment there is only a UTF-8 implementation.
*/
ساکن صحیح nocaseCollatingFunc(
  خالی *Not_مستعمل,
  صحیح nKey1, ثابت خالی *pKey1,
  صحیح nKey2, ثابت خالی *pKey2
){
  صحیح r = sqlite3StrNICmp(
      (ثابت حرف *)pKey1, (ثابت حرف *)pKey2, (nKey1<nKey2)?nKey1:nKey2);
  غیر_مستعمل_PARAMETER(Not_مستعمل);
  اگر( 0==r ){
    r = nKey1-nKey2;
  }
  واپس r;
}

/*
** Return the ROWID of the most recent insert
*/
sqlite_int64 sqlite3_آخری_داخل_کرو_rowid(sqlite3 *db){
  i64 iRet;
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db) ){
    (خالی)SQLITE_MISUSE_BKPT;
    واپس 0;
  }
#اگر_ختم
  sqlite3_mutex_enter(db->mutex);
  iRet = db->آخری_Rowid;
  sqlite3_mutex_leave(db->mutex);
  واپس iRet;
}

/*
** Set the value returned by the sqlite3_last_insert_rowid() API function.
*/
خالی sqlite3_سیٹ_آخری_داخل_کرو_rowid(sqlite3 *db, sqlite3_int64 iRowid){
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db) ){
    (خالی)SQLITE_MISUSE_BKPT;
    واپس;
  }
#اگر_ختم
  sqlite3_mutex_enter(db->mutex);
  db->آخری_Rowid = iRowid;
  sqlite3_mutex_leave(db->mutex);
}

/*
** Return the number of changes in the most recently executed DML
** statement.
*/
sqlite3_int64 sqlite3_changes64(sqlite3 *db){
  i64 iRet;
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db) ){
    (خالی)SQLITE_MISUSE_BKPT;
    واپس 0;
  }
#اگر_ختم
  sqlite3_mutex_enter(db->mutex);
  iRet = db->nChange;
  sqlite3_mutex_leave(db->mutex);
  واپس iRet;
}
صحیح sqlite3_changes(sqlite3 *db){
  واپس (صحیح)sqlite3_changes64(db);
}

/*
** Return the number of changes since the database handle was opened.
*/
sqlite3_int64 sqlite3_کل_changes64(sqlite3 *db){
  i64 iRet;
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db) ){
    (خالی)SQLITE_MISUSE_BKPT;
    واپس 0;
  }
#اگر_ختم
  sqlite3_mutex_enter(db->mutex);
  iRet = db->n_کل_Change;
  sqlite3_mutex_leave(db->mutex);
  واپس iRet;
}
صحیح sqlite3_کل_changes(sqlite3 *db){
  واپس (صحیح)sqlite3_کل_changes64(db);
}

/*
** Close all open savepoints. This function only manipulates fields of the
** database handle object, it does not close any savepoints that may be open
** at the b-tree/pager level.
*/
خالی sqlite3CloseSavepoints(sqlite3 *db){
  جب( db->pSavepoint ){
    Savepoint *pTmp = db->pSavepoint;
    db->pSavepoint = pTmp->p_اگلا;
    sqlite3DbFree(db, pTmp);
  }
  db->nSavepoint = 0;
  db->nStatement = 0;
  db->isTransactionSavepoint = 0;
}

/*
** Invoke the destructor function associated with FuncDef p, if any. Except,
** if this is not the last copy of the function, do not invoke it. Multiple
** copies of a single function are created when create_function() is called
** with SQLITE_ANY as the encoding.
*/
ساکن خالی functionDestroy(sqlite3 *db, FuncDef *p){
  FuncDestructor *pDestructor;
  أكد( (p->funcFlags & SQLITE_FUNC_BUILTIN)==0 );
  pDestructor = p->u.pDestructor;
  اگر( pDestructor ){
    pDestructor->nRef--;
    اگر( pDestructor->nRef==0 ){
      pDestructor->xDestroy(pDestructor->p_User_ڈیٹا);
      sqlite3DbFree(db, pDestructor);
    }
  }
}

/*
** Disconnect all sqlite3_vtab objects that belong to database connection
** db. This is called when db is being closed.
*/
ساکن خالی disconnectAllVtab(sqlite3 *db){
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_VIRTUALTABLE
  صحیح i;
  HashElem *p;
  sqlite3BtreeEnterAll(db);
  برائے(i=0; i<db->nDb; i++){
    Schema *pSchema = db->aDb[i].pSchema;
    اگر( pSchema ){
      برائے(p=sqlite_Hash_پہلا(&pSchema->tblHash); p; p=sqlite_Hash_اگلا(p)){
        جدول *pTab = (جدول *)sqlite_Hash_ڈیٹا(p);
        اگر( IsVirtual(pTab) ) sqlite3VtabDisconnect(db, pTab);
      }
    }
  }
  برائے(p=sqlite_Hash_پہلا(&db->aModule); p; p=sqlite_Hash_اگلا(p)){
    Module *pMod = (Module *)sqlite_Hash_ڈیٹا(p);
    اگر( pMod->pEpoTab ){
      sqlite3VtabDisconnect(db, pMod->pEpoTab);
    }
  }
  sqlite3VtabUnlockList(db);
  sqlite3BtreeLeaveAll(db);
#ورنہ
  غیر_مستعمل_PARAMETER(db);
#اگر_ختم
}

/*
** Return TRUE if database connection db has unfinalized prepared
** statements or unfinished sqlite3_backup objects. 
*/
ساکن صحیح connectionIsBusy(sqlite3 *db){
  صحیح j;
  أكد( sqlite3_mutex_held(db->mutex) );
  اگر( db->pVdbe ) واپس 1;
  برائے(j=0; j<db->nDb; j++){
    Btree *pBt = db->aDb[j].pBt;
    اگر( pBt && sqlite3_Btree_Is_اندر_Backup(pBt) ) واپس 1;
  }
  واپس 0;
}

/*
** Close an existing SQLite database
*/
ساکن صحیح sqlite3Close(sqlite3 *db, صحیح forceZombie){
  اگر( !db ){
    /* EVIDENCE-OF: R-63257-11740 Calling sqlite3_close() or
    ** sqlite3_close_v2() with a NULL pointer argument is a harmless no-op. */
    واپس SQLITE_OK;
  }
  اگر( !sqlite3SafetyCheckSickOrOk(db) ){
    واپس SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(db->mutex);
  اگر( db->mTrace & SQLITE_TRACE_CLOSE ){
    db->trace.xV2(SQLITE_TRACE_CLOSE, db->pTraceArg, db, 0);
  }

  /* Force xDisconnect calls on all virtual tables */
  disconnectAllVtab(db);

  /* If a transaction is open, the disconnectAllVtab() call above
  ** will not have called the xDisconnect() method on any virtual
  ** tables in the db->aVTrans[] array. The following sqlite3VtabRollback()
  ** call will do so. We need to do this before the check for active
  ** SQL statements below, as the v-table implementation may be storing
  ** some prepared statements internally.
  */
  sqlite3VtabRollback(db);

  /* Legacy behavior (sqlite3_close() behavior) is to return
  ** SQLITE_BUSY if the connection can not be closed immediately.
  */
  اگر( !forceZombie && connectionIsBusy(db) ){
    sqlite3_Error_With_پیغام(db, SQLITE_BUSY, "unable to close due to unfinalized "
       "statements or unfinished backups");
    sqlite3_mutex_leave(db->mutex);
    واپس SQLITE_BUSY;
  }

#اگر_تعریف_شدہ SQLITE_ENABLE_SQLLOG
  اگر( sqlite3GlobalConfig.xSqllog ){
    /* Closing the handle. Fourth parameter is passed the value 2. */
    sqlite3GlobalConfig.xSqllog(sqlite3GlobalConfig.pSqllogArg, db, 0, 2);
  }
#اگر_ختم

  جب( db->p_Db_ڈیٹا ){
    Db_Client_ڈیٹا *p = db->p_Db_ڈیٹا;
    db->p_Db_ڈیٹا = p->p_اگلا;
    أكد( p->p_ڈیٹا!=0 );
    اگر( p->xDestructor ) p->xDestructor(p->p_ڈیٹا);
    sqlite3_free(p);
  }

  /* Convert the connection into a zombie and then close it.
  */
  db->eOpenState = SQLITE_STATE_ZOMBIE;
  sqlite3LeaveMutexAndCloseZombie(db);
  واپس SQLITE_OK;
}

/*
** Return the transaction state for a single databse, or the maximum
** transaction state over all attached databases if zSchema is null.
*/
صحیح sqlite3_txn_state(sqlite3 *db, ثابت حرف *zSchema){
  صحیح iDb, nDb;
  صحیح iTxn = -1;
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db) ){
    (خالی)SQLITE_MISUSE_BKPT;
    واپس -1;
  }
#اگر_ختم
  sqlite3_mutex_enter(db->mutex);
  اگر( zSchema ){
    nDb = iDb = sqlite3_ڈھونڈو_Db_Name(db, zSchema);
    اگر( iDb<0 ) nDb--;
  }ورنہ{
    iDb = 0;
    nDb = db->nDb-1;
  }
  برائے(; iDb<=nDb; iDb++){
    Btree *pBt = db->aDb[iDb].pBt;
    صحیح x = pBt!=0 ? sqlite3BtreeTxnState(pBt) : SQLITE_TXN_NONE;
    اگر( x>iTxn ) iTxn = x;
  }
  sqlite3_mutex_leave(db->mutex);
  واپس iTxn;
}

/*
** Two variations on the public interface for closing a database
** connection. The sqlite3_close() version returns SQLITE_BUSY and
** leaves the connection open if there are unfinalized prepared
** statements or unfinished sqlite3_backups.  The sqlite3_close_v2()
** version forces the connection to become a zombie if there are
** unclosed resources, and arranges for deallocation when the last
** prepare statement or sqlite3_backup closes.
*/
صحیح sqlite3_close(sqlite3 *db){ واپس sqlite3Close(db,0); }
صحیح sqlite3_close_v2(sqlite3 *db){ واپس sqlite3Close(db,1); }


/*
** Close the mutex on database connection db.
**
** Furthermore, if database connection db is a zombie (meaning that there
** has been a prior call to sqlite3_close(db) or sqlite3_close_v2(db)) and
** every sqlite3_stmt has now been finalized and every sqlite3_backup has
** finished, then free all resources.
*/
خالی sqlite3LeaveMutexAndCloseZombie(sqlite3 *db){
  HashElem *i;                    /* Hash table iterator */
  صحیح j;

  /* If there are outstanding sqlite3_stmt or sqlite3_backup objects
  ** or if the connection has not yet been closed by sqlite3_close_v2(),
  ** then just leave the mutex and return.
  */
  اگر( db->eOpenState!=SQLITE_STATE_ZOMBIE || connectionIsBusy(db) ){
    sqlite3_mutex_leave(db->mutex);
    واپس;
  }

  /* If we reach this point, it means that the database connection has
  ** closed all sqlite3_stmt and sqlite3_backup objects and has been
  ** passed to sqlite3_close (meaning that it is a zombie).  Therefore,
  ** go ahead and free all resources.
  */

  /* If a transaction is open, roll it back. This also ensures that if
  ** any database schemas have been modified by an uncommitted transaction
  ** they are reset. And that the required b-tree mutex is held to make
  ** the pager rollback and schema reset an atomic operation. */
  sqlite3RollbackAll(db, SQLITE_OK);

  /* Free any outstanding Savepoint structures. */
  sqlite3CloseSavepoints(db);

  /* Close all database connections */
  برائے(j=0; j<db->nDb; j++){
    ساخت Db *pDb = &db->aDb[j];
    اگر( pDb->pBt ){
      sqlite3BtreeClose(pDb->pBt);
      pDb->pBt = 0;
      اگر( j!=1 ){
        pDb->pSchema = 0;
      }
    }
  }
  /* Clear the TEMP schema separately and last */
  اگر( db->aDb[1].pSchema ){
    sqlite3SchemaClear(db->aDb[1].pSchema);
    أكد( db->aDb[1].pSchema->trigHash.گنتی==0 );
  }
  sqlite3VtabUnlockList(db);

  /* Free up the array of auxiliary databases */
  sqlite3CollapseDatabaseArray(db);
  أكد( db->nDb<=2 );
  أكد( db->aDb==db->aDbStatic );

  /* Tell the code in notify.c that the connection no longer holds any
  ** locks and does not require any further unlock-notify callbacks.
  */
  sqlite3ConnectionClosed(db);

  برائے(i=sqlite_Hash_پہلا(&db->aFunc); i; i=sqlite_Hash_اگلا(i)){
    FuncDef *p_اگلا, *p;
    p = sqlite_Hash_ڈیٹا(i);
    کرو{
      functionDestroy(db, p);
      p_اگلا = p->p_اگلا;
      sqlite3DbFree(db, p);
      p = p_اگلا;
    }جب( p );
  }
  sqlite3HashClear(&db->aFunc);
  برائے(i=sqlite_Hash_پہلا(&db->aCollSeq); i; i=sqlite_Hash_اگلا(i)){
    CollSeq *pColl = (CollSeq *)sqlite_Hash_ڈیٹا(i);
    /* Invoke any destructors registered for collation sequence user data. */
    برائے(j=0; j<3; j++){
      اگر( pColl[j].xDel ){
        pColl[j].xDel(pColl[j].pUser);
      }
    }
    sqlite3DbFree(db, pColl);
  }
  sqlite3HashClear(&db->aCollSeq);
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_VIRTUALTABLE
  برائے(i=sqlite_Hash_پہلا(&db->aModule); i; i=sqlite_Hash_اگلا(i)){
    Module *pMod = (Module *)sqlite_Hash_ڈیٹا(i);
    sqlite3_Vtab_Eponymous_جدول_Clear(db, pMod);
    sqlite3VtabModuleUnref(db, pMod);
  }
  sqlite3HashClear(&db->aModule);
#اگر_ختم

  sqlite3Error(db, SQLITE_OK); /* Deallocates any cached error strings. */
  sqlite3_قدر_Free(db->pErr);
  sqlite3CloseExtensions(db);

  db->eOpenState = SQLITE_STATE_ERROR;

  /* The temp-database schema is allocated differently from the other schema
  ** objects (using sqliteMalloc() directly, instead of sqlite3BtreeSchema()).
  ** So it needs to be freed here. Todo: Why not roll the temp schema into
  ** the same sqliteMalloc() as the one that allocates the database
  ** structure?
  */
  sqlite3DbFree(db, db->aDb[1].pSchema);
  اگر( db->xAutovacDestr ){
    db->xAutovacDestr(db->p_Autovac_صفحات_Arg);
  }
  sqlite3_mutex_leave(db->mutex);
  db->eOpenState = SQLITE_STATE_CLOSED;
  sqlite3_mutex_free(db->mutex);
  أكد( sqlite3_Lookaside_مستعمل(db,0)==0 );
  اگر( db->lookaside.bMalloced ){
    sqlite3_free(db->lookaside.p_آغاز);
  }
  sqlite3_free(db);
}

/*
** Rollback all database files.  If tripCode is not SQLITE_OK, then
** any write cursors are invalidated ("tripped" - as in "tripping a circuit
** breaker") and made to return tripCode if there are any further
** attempts to use that cursor.  Read cursors remain open and valid
** but are "saved" in case the table pages are moved around.
*/
خالی sqlite3RollbackAll(sqlite3 *db, صحیح trip_رمز){
  صحیح i;
  صحیح اندر_Trans = 0;
  صحیح schemaChange;
  أكد( sqlite3_mutex_held(db->mutex) );
  sqlite3BeginBenignMalloc();

  /* Obtain all b-tree mutexes before making any calls to BtreeRollback().
  ** This is important in case the transaction being rolled back has
  ** modified the database schema. If the b-tree mutexes are not taken
  ** here, then another shared-cache connection might sneak in between
  ** the database rollback and schema reset, which can cause false
  ** corruption reports in some cases.  */
  sqlite3BtreeEnterAll(db);
  schemaChange = (db->mDbFlags & DBFLAG_SchemaChange)!=0 && db->init.busy==0;

  برائے(i=0; i<db->nDb; i++){
    Btree *p = db->aDb[i].pBt;
    اگر( p ){
      اگر( sqlite3BtreeTxnState(p)==SQLITE_TXN_WRITE ){
        اندر_Trans = 1;
      }
      sqlite3BtreeRollback(p, trip_رمز, !schemaChange);
    }
  }
  sqlite3VtabRollback(db);
  sqlite3_انتہا_Benign_Malloc();

  اگر( schemaChange ){
    sqlite3ExpirePreparedStatements(db, 0);
    sqlite3_Reset_All_Schemas_کا_Connection(db);
  }
  sqlite3BtreeLeaveAll(db);

  /* Any deferred constraint violations have now been resolved. */
  db->nDeferredCons = 0;
  db->nDeferredImmCons = 0;
  db->flags &= ~(u64)(SQLITE_DeferFKs|SQLITE_CorruptRdOnly);

  /* If one has been configured, invoke the rollback-hook callback */
  اگر( db->xRollbackCallback && (اندر_Trans || !db->autoCommit) ){
    db->xRollbackCallback(db->pRollbackArg);
  }
}

/*
** Return a static string containing the name corresponding to the error code
** specified in the argument.
*/
#اگر defined(SQLITE_NEED_ERR_NAME)
ثابت حرف *sqlite3ErrName(صحیح rc){
  ثابت حرف *zName = 0;
  صحیح i, origRc = rc;
  برائے(i=0; i<2 && zName==0; i++, rc &= 0xff){
    انتخاب( rc ){
      حالت SQLITE_OK:                 zName = "SQLITE_OK";                توڑو;
      حالت SQLITE_ERROR:              zName = "SQLITE_ERROR";             توڑو;
      حالت SQLITE_ERROR_SNAPSHOT:     zName = "SQLITE_ERROR_SNAPSHOT";    توڑو;
      حالت SQLITE_ERROR_RETRY:        zName = "SQLITE_ERROR_RETRY";       توڑو;
      حالت SQLITE_ERROR_MISSING_COLLSEQ:
                                zName = "SQLITE_ERROR_MISSING_COLLSEQ";   توڑو;
      حالت SQLITE_اندرونی:           zName = "SQLITE_INTERNAL";          توڑو;
      حالت SQLITE_PERM:               zName = "SQLITE_PERM";              توڑو;
      حالت SQLITE_ABORT:              zName = "SQLITE_ABORT";             توڑو;
      حالت SQLITE_ABORT_ROLLBACK:     zName = "SQLITE_ABORT_ROLLBACK";    توڑو;
      حالت SQLITE_BUSY:               zName = "SQLITE_BUSY";              توڑو;
      حالت SQLITE_BUSY_RECOVERY:      zName = "SQLITE_BUSY_RECOVERY";     توڑو;
      حالت SQLITE_BUSY_SNAPSHOT:      zName = "SQLITE_BUSY_SNAPSHOT";     توڑو;
      حالت SQLITE_LOCKED:             zName = "SQLITE_LOCKED";            توڑو;
      حالت SQLITE_LOCKED_SHAREDCACHE: zName = "SQLITE_LOCKED_SHAREDCACHE";توڑو;
      حالت SQLITE_NOMEM:              zName = "SQLITE_NOMEM";             توڑو;
      حالت SQLITE_READONLY:           zName = "SQLITE_READONLY";          توڑو;
      حالت SQLITE_READONLY_RECOVERY:  zName = "SQLITE_READONLY_RECOVERY"; توڑو;
      حالت SQLITE_READONLY_CANTINIT:  zName = "SQLITE_READONLY_CANTINIT"; توڑو;
      حالت SQLITE_READONLY_ROLLBACK:  zName = "SQLITE_READONLY_ROLLBACK"; توڑو;
      حالت SQLITE_READONLY_DBMOVED:   zName = "SQLITE_READONLY_DBMOVED";  توڑو;
      حالت SQLITE_READONLY_DIRECTORY: zName = "SQLITE_READONLY_DIRECTORY";توڑو;
      حالت SQLITE_مداخلت:          zName = "SQLITE_INTERRUPT";         توڑو;
      حالت SQLITE_IOERR:              zName = "SQLITE_IOERR";             توڑو;
      حالت SQLITE_IOERR_READ:         zName = "SQLITE_IOERR_READ";        توڑو;
      حالت SQLITE_IOERR_SHORT_READ:   zName = "SQLITE_IOERR_SHORT_READ";  توڑو;
      حالت SQLITE_IOERR_WRITE:        zName = "SQLITE_IOERR_WRITE";       توڑو;
      حالت SQLITE_IOERR_FSYNC:        zName = "SQLITE_IOERR_FSYNC";       توڑو;
      حالت SQLITE_IOERR_DIR_FSYNC:    zName = "SQLITE_IOERR_DIR_FSYNC";   توڑو;
      حالت SQLITE_IOERR_TRUNCATE:     zName = "SQLITE_IOERR_TRUNCATE";    توڑو;
      حالت SQLITE_IOERR_FSTAT:        zName = "SQLITE_IOERR_FSTAT";       توڑو;
      حالت SQLITE_IOERR_UNLOCK:       zName = "SQLITE_IOERR_UNLOCK";      توڑو;
      حالت SQLITE_IOERR_RDLOCK:       zName = "SQLITE_IOERR_RDLOCK";      توڑو;
      حالت SQLITE_IOERR_DELETE:       zName = "SQLITE_IOERR_DELETE";      توڑو;
      حالت SQLITE_IOERR_NOMEM:        zName = "SQLITE_IOERR_NOMEM";       توڑو;
      حالت SQLITE_IOERR_ACCESS:       zName = "SQLITE_IOERR_ACCESS";      توڑو;
      حالت SQLITE_IOERR_CHECKRESERVEDLOCK:
                                zName = "SQLITE_IOERR_CHECKRESERVEDLOCK"; توڑو;
      حالت SQLITE_IOERR_تالا:         zName = "SQLITE_IOERR_LOCK";        توڑو;
      حالت SQLITE_IOERR_CLOSE:        zName = "SQLITE_IOERR_CLOSE";       توڑو;
      حالت SQLITE_IOERR_DIR_CLOSE:    zName = "SQLITE_IOERR_DIR_CLOSE";   توڑو;
      حالت SQLITE_IOERR_SHMOPEN:      zName = "SQLITE_IOERR_SHMOPEN";     توڑو;
      حالت SQLITE_IOERR_SHMSIZE:      zName = "SQLITE_IOERR_SHMSIZE";     توڑو;
      حالت SQLITE_IOERR_SHMLOCK:      zName = "SQLITE_IOERR_SHMLOCK";     توڑو;
      حالت SQLITE_IOERR_SHMMAP:       zName = "SQLITE_IOERR_SHMMAP";      توڑو;
      حالت SQLITE_IOERR_SEEK:         zName = "SQLITE_IOERR_SEEK";        توڑو;
      حالت SQLITE_IOERR_DELETE_NOENT: zName = "SQLITE_IOERR_DELETE_NOENT";توڑو;
      حالت SQLITE_IOERR_MMAP:         zName = "SQLITE_IOERR_MMAP";        توڑو;
      حالت SQLITE_IOERR_GETTEMPPATH:  zName = "SQLITE_IOERR_GETTEMPPATH"; توڑو;
      حالت SQLITE_IOERR_CONVPATH:     zName = "SQLITE_IOERR_CONVPATH";    توڑو;
      حالت SQLITE_CORRUPT:            zName = "SQLITE_CORRUPT";           توڑو;
      حالت SQLITE_CORRUPT_VTAB:       zName = "SQLITE_CORRUPT_VTAB";      توڑو;
      حالت SQLITE_NOTFOUND:           zName = "SQLITE_NOTFOUND";          توڑو;
      حالت SQLITE_بھرا:               zName = "SQLITE_FULL";              توڑو;
      حالت SQLITE_CANTOPEN:           zName = "SQLITE_CANTOPEN";          توڑو;
      حالت SQLITE_CANTOPEN_NOTEMPDIR: zName = "SQLITE_CANTOPEN_NOTEMPDIR";توڑو;
      حالت SQLITE_CANTOPEN_ISDIR:     zName = "SQLITE_CANTOPEN_ISDIR";    توڑو;
      حالت SQLITE_CANTOPEN_FULLPATH:  zName = "SQLITE_CANTOPEN_FULLPATH"; توڑو;
      حالت SQLITE_CANTOPEN_CONVPATH:  zName = "SQLITE_CANTOPEN_CONVPATH"; توڑو;
      حالت SQLITE_CANTOPEN_SYMLINK:   zName = "SQLITE_CANTOPEN_SYMLINK";  توڑو;
      حالت SQLITE_PROTOCOL:           zName = "SQLITE_PROTOCOL";          توڑو;
      حالت SQLITE_خالی_خانے:              zName = "SQLITE_EMPTY";             توڑو;
      حالت SQLITE_SCHEMA:             zName = "SQLITE_SCHEMA";            توڑو;
      حالت SQLITE_TOOBIG:             zName = "SQLITE_TOOBIG";            توڑو;
      حالت SQLITE_CONSTRAINT:         zName = "SQLITE_CONSTRAINT";        توڑو;
      حالت SQLITE_CONSTRAINT_UNIQUE:  zName = "SQLITE_CONSTRAINT_UNIQUE"; توڑو;
      حالت SQLITE_CONSTRAINT_TRIGGER: zName = "SQLITE_CONSTRAINT_TRIGGER";توڑو;
      حالت SQLITE_CONSTRAINT_FOREIGNKEY:
                                zName = "SQLITE_CONSTRAINT_FOREIGNKEY";   توڑو;
      حالت SQLITE_CONSTRAINT_CHECK:   zName = "SQLITE_CONSTRAINT_CHECK";  توڑو;
      حالت SQLITE_CONSTRAINT_PRIMARYKEY:
                                zName = "SQLITE_CONSTRAINT_PRIMARYKEY";   توڑو;
      حالت SQLITE_CONSTRAINT_NOTNULL: zName = "SQLITE_CONSTRAINT_NOTNULL";توڑو;
      حالت SQLITE_CONSTRAINT_COMMITHOOK:
                                zName = "SQLITE_CONSTRAINT_COMMITHOOK";   توڑو;
      حالت SQLITE_CONSTRAINT_VTAB:    zName = "SQLITE_CONSTRAINT_VTAB";   توڑو;
      حالت SQLITE_CONSTRAINT_FUNCTION:
                                zName = "SQLITE_CONSTRAINT_FUNCTION";     توڑو;
      حالت SQLITE_CONSTRAINT_ROWID:   zName = "SQLITE_CONSTRAINT_ROWID";  توڑو;
      حالت SQLITE_MISMATCH:           zName = "SQLITE_MISMATCH";          توڑو;
      حالت SQLITE_MISUSE:             zName = "SQLITE_MISUSE";            توڑو;
      حالت SQLITE_NOLFS:              zName = "SQLITE_NOLFS";             توڑو;
      حالت SQLITE_AUTH:               zName = "SQLITE_AUTH";              توڑو;
      حالت SQLITE_FORMAT:             zName = "SQLITE_FORMAT";            توڑو;
      حالت SQLITE_RANGE:              zName = "SQLITE_RANGE";             توڑو;
      حالت SQLITE_NOTADB:             zName = "SQLITE_NOTADB";            توڑو;
      حالت SQLITE_صف:                zName = "SQLITE_ROW";               توڑو;
      حالت SQLITE_NOTICE:             zName = "SQLITE_NOTICE";            توڑو;
      حالت SQLITE_NOTICE_RECOVER_WAL: zName = "SQLITE_NOTICE_RECOVER_WAL";توڑو;
      حالت SQLITE_NOTICE_RECOVER_ROLLBACK:
                                zName = "SQLITE_NOTICE_RECOVER_ROLLBACK"; توڑو;
      حالت SQLITE_NOTICE_RBU:         zName = "SQLITE_NOTICE_RBU"; توڑو;
      حالت SQLITE_WARNING:            zName = "SQLITE_WARNING";           توڑو;
      حالت SQLITE_WARNING_AUTOINDEX:  zName = "SQLITE_WARNING_AUTOINDEX"; توڑو;
      حالت SQLITE_مکمل:               zName = "SQLITE_DONE";              توڑو;
    }
  }
  اگر( zName==0 ){
    ساکن حرف zBuf[50];
    sqlite3_snprintf(حجم(zBuf), zBuf, "SQLITE_UNKNOWN(%d)", origRc);
    zName = zBuf;
  }
  واپس zName;
}
#اگر_ختم

/*
** Return a static string that describes the kind of error specified in the
** argument.
*/
ثابت حرف *sqlite3ErrStr(صحیح rc){
  ساکن ثابت حرف* ثابت a_پیغام[] = {
    /* SQLITE_OK          */ "not an error",
    /* SQLITE_ERROR       */ "SQL logic error",
    /* SQLITE_INTERNAL    */ 0,
    /* SQLITE_PERM        */ "access permission denied",
    /* SQLITE_ABORT       */ "query aborted",
    /* SQLITE_BUSY        */ "database is locked",
    /* SQLITE_LOCKED      */ "database table is locked",
    /* SQLITE_NOMEM       */ "out of memory",
    /* SQLITE_READONLY    */ "attempt to write a readonly database",
    /* SQLITE_INTERRUPT   */ "interrupted",
    /* SQLITE_IOERR       */ "disk I/O error",
    /* SQLITE_CORRUPT     */ "database disk image is malformed",
    /* SQLITE_NOTFOUND    */ "unknown operation",
    /* SQLITE_FULL        */ "database or disk is full",
    /* SQLITE_CANTOPEN    */ "unable to open database file",
    /* SQLITE_PROTOCOL    */ "locking protocol",
    /* SQLITE_EMPTY       */ 0,
    /* SQLITE_SCHEMA      */ "database schema has changed",
    /* SQLITE_TOOBIG      */ "string or blob too big",
    /* SQLITE_CONSTRAINT  */ "constraint failed",
    /* SQLITE_MISMATCH    */ "datatype mismatch",
    /* SQLITE_MISUSE      */ "bad parameter or other API misuse",
#اگر_تعریف_شدہ SQLITE_DISABLE_LFS
    /* SQLITE_NOLFS       */ "large file support is disabled",
#ورنہ
    /* SQLITE_NOLFS       */ 0,
#اگر_ختم
    /* SQLITE_AUTH        */ "authorization denied",
    /* SQLITE_FORMAT      */ 0,
    /* SQLITE_RANGE       */ "column index out of range",
    /* SQLITE_NOTADB      */ "file is not a database",
    /* SQLITE_NOTICE      */ "notification message",
    /* SQLITE_WARNING     */ "warning message",
  };
  ثابت حرف *zErr = "unknown error";
  انتخاب( rc ){
    حالت SQLITE_ABORT_ROLLBACK: {
      zErr = "abort due to ROLLBACK";
      توڑو;
    }
    حالت SQLITE_صف: {
      zErr = "another row available";
      توڑو;
    }
    حالت SQLITE_مکمل: {
      zErr = "no more rows available";
      توڑو;
    }
    طےشدہ: {
      rc &= 0xff;
      اگر( ALWAYS(rc>=0) && rc<Array_سائز(a_پیغام) && a_پیغام[rc]!=0 ){
        zErr = a_پیغام[rc];
      }
      توڑو;
    }
  }
  واپس zErr;
}

/*
** This routine implements a busy callback that sleeps and tries
** again until a timeout value is reached.  The timeout value is
** an integer number of milliseconds passed in as the first
** argument.
**
** Return non-zero to retry the lock.  Return zero to stop trying
** and cause SQLite to return SQLITE_BUSY.
*/
ساکن صحیح sqliteDefaultBusyCallback(
  خالی *ptr,               /* Database connection */
  صحیح گنتی                /* Number of times table has been busy */
){
#اگر SQLITE_OS_WIN || !defined(HAVE_NANOSLEEP) || HAVE_NANOSLEEP
  /* This case is for systems that have support for sleeping for fractions of
  ** a second.  Examples:  All windows systems, unix systems with nanosleep() */
  ساکن ثابت u8 delays[] =
     { 1, 2, 5, 10, 15, 20, 25, 25,  25,  50,  50, 100 };
  ساکن ثابت u8 totals[] =
     { 0, 1, 3,  8, 18, 33, 53, 78, 103, 128, 178, 228 };
# define NDELAY Array_سائز(delays)
  sqlite3 *db = (sqlite3 *)ptr;
  صحیح tmout = db->busyTimeout;
  صحیح delay, پیشگی;

  أكد( گنتی>=0 );
  اگر( گنتی < NDELAY ){
    delay = delays[گنتی];
    پیشگی = totals[گنتی];
  }ورنہ{
    delay = delays[NDELAY-1];
    پیشگی = totals[NDELAY-1] + delay*(گنتی-(NDELAY-1));
  }
  اگر( پیشگی + delay > tmout ){
    delay = tmout - پیشگی;
    اگر( delay<=0 ) واپس 0;
  }
  sqlite3OsSleep(db->pVfs, delay*1000);
  واپس 1;
#ورنہ
  /* This case for unix systems that lack usleep() support.  Sleeping
  ** must be done in increments of whole seconds */
  sqlite3 *db = (sqlite3 *)ptr;
  صحیح tmout = ((sqlite3 *)ptr)->busyTimeout;
  اگر( (گنتی+1)*1000 > tmout ){
    واپس 0;
  }
  sqlite3OsSleep(db->pVfs, 1000000);
  واپس 1;
#اگر_ختم
}

/*
** Invoke the given busy handler.
**
** This routine is called when an operation failed to acquire a
** lock on VFS file pFile.
**
** If this routine returns non-zero, the lock is retried.  If it
** returns 0, the operation aborts with an SQLITE_BUSY error.
*/
صحیح sqlite3InvokeBusyHandler(BusyHandler *p){
  صحیح rc;
  اگر( p->xBusyHandler==0 || p->nBusy<0 ) واپس 0;
  rc = p->xBusyHandler(p->pBusyArg, p->nBusy);
  اگر( rc==0 ){
    p->nBusy = -1;
  }ورنہ{
    p->nBusy++;
  }
  واپس rc;
}

/*
** This routine sets the busy callback for an Sqlite database to the
** given callback function with the given argument.
*/
صحیح sqlite3_busy_handler(
  sqlite3 *db,
  صحیح (*xBusy)(خالی*,صحیح),
  خالی *pArg
){
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db) ) واپس SQLITE_MISUSE_BKPT;
#اگر_ختم
  sqlite3_mutex_enter(db->mutex);
  db->busyHandler.xBusyHandler = xBusy;
  db->busyHandler.pBusyArg = pArg;
  db->busyHandler.nBusy = 0;
  db->busyTimeout = 0;
#اگر_تعریف_شدہ SQLITE_ENABLE_SETLK_TIMEOUT
  db->setlkTimeout = 0;
#اگر_ختم
  sqlite3_mutex_leave(db->mutex);
  واپس SQLITE_OK;
}

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_پیش_رفت_CALLBACK
/*
** This routine sets the progress callback for an Sqlite database to the
** given callback function with the given argument. The progress callback will
** be invoked every nOps opcodes.
*/
خالی sqlite3_پیش_رفت_handler(
  sqlite3 *db,
  صحیح nOps,
  صحیح (*x_پیش_رفت)(خالی*),
  خالی *pArg
){
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db) ){
    (خالی)SQLITE_MISUSE_BKPT;
    واپس;
  }
#اگر_ختم
  sqlite3_mutex_enter(db->mutex);
  اگر( nOps>0 ){
    db->x_پیش_رفت = x_پیش_رفت;
    db->n_پیش_رفت_Ops = (غیرعلامتی)nOps;
    db->p_پیش_رفت_Arg = pArg;
  }ورنہ{
    db->x_پیش_رفت = 0;
    db->n_پیش_رفت_Ops = 0;
    db->p_پیش_رفت_Arg = 0;
  }
  sqlite3_mutex_leave(db->mutex);
}
#اگر_ختم


/*
** This routine installs a default busy handler that waits for the
** specified number of milliseconds before returning 0.
*/
صحیح sqlite3_busy_timeout(sqlite3 *db, صحیح ms){
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db) ) واپس SQLITE_MISUSE_BKPT;
#اگر_ختم
  sqlite3_mutex_enter(db->mutex);
  اگر( ms>0 ){
    sqlite3_busy_handler(db, (صحیح(*)(خالی*,صحیح))sqliteDefaultBusyCallback,
                             (خالی*)db);
    db->busyTimeout = ms;
#اگر_تعریف_شدہ SQLITE_ENABLE_SETLK_TIMEOUT
    db->setlkTimeout = ms;
#اگر_ختم
  }ورنہ{
    sqlite3_busy_handler(db, 0, 0);
  }
  sqlite3_mutex_leave(db->mutex);
  واپس SQLITE_OK;
}

/*
** Set the setlk timeout value.
*/
صحیح sqlite3_setlk_timeout(sqlite3 *db, صحیح ms, صحیح flags){
#اگر_تعریف_شدہ SQLITE_ENABLE_SETLK_TIMEOUT
  صحیح iDb;
  صحیح bBOC = ((flags & SQLITE_SETLK_بلاک_ON_CONNECT) ? 1 : 0);
#اگر_ختم
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db) ) واپس SQLITE_MISUSE_BKPT;
#اگر_ختم
  اگر( ms<-1 ) واپس SQLITE_RANGE;
#اگر_تعریف_شدہ SQLITE_ENABLE_SETLK_TIMEOUT
  sqlite3_mutex_enter(db->mutex);
  db->setlkTimeout = ms;
  db->setlkFlags = flags;
  sqlite3BtreeEnterAll(db);
  برائے(iDb=0; iDb<db->nDb; iDb++){
    Btree *pBt = db->aDb[iDb].pBt;
    اگر( pBt ){
      sqlite3_file *fd = sqlite3PagerFile(sqlite3BtreePager(pBt));
      sqlite3OsFileControlHint(fd, SQLITE_FCNTL_بلاک_ON_CONNECT, (خالی*)&bBOC);
    }
  }
  sqlite3BtreeLeaveAll(db);
  sqlite3_mutex_leave(db->mutex);
#اگر_ختم
#اگر !defined(SQLITE_ENABLE_API_ARMOR) && !defined(SQLITE_ENABLE_SETLK_TIMEOUT)
  غیر_مستعمل_PARAMETER(db);
  غیر_مستعمل_PARAMETER(flags);
#اگر_ختم
  واپس SQLITE_OK;
}

/*
** Cause any pending operation to stop at its earliest opportunity.
*/
خالی sqlite3_مداخلت(sqlite3 *db){
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db)
   && (db==0 || db->eOpenState!=SQLITE_STATE_ZOMBIE)
  ){
    (خالی)SQLITE_MISUSE_BKPT;
    واپس;
  }
#اگر_ختم
  AtomicStore(&db->u1.isInterrupted, 1);
}

/*
** Return true or false depending on whether or not an interrupt is
** pending on connection db.
*/
صحیح sqlite3_is_interrupted(sqlite3 *db){
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db)
   && (db==0 || db->eOpenState!=SQLITE_STATE_ZOMBIE)
  ){
    (خالی)SQLITE_MISUSE_BKPT;
    واپس 0;
  }
#اگر_ختم
  واپس Atomic_بھرو(&db->u1.isInterrupted)!=0;
}

/*
** This function is exactly the same as sqlite3_create_function(), except
** that it is designed to be called by internal code. The difference is
** that if a malloc() fails in sqlite3_create_function(), an error code
** is returned and the mallocFailed flag cleared.
*/
صحیح sqlite3CreateFunc(
  sqlite3 *db,
  ثابت حرف *zFunctionName,
  صحیح nArg,
  صحیح enc,
  خالی *p_User_ڈیٹا,
  خالی (*xSFunc)(sqlite3_context*,صحیح,sqlite3_قدر **),
  خالی (*x_قدم)(sqlite3_context*,صحیح,sqlite3_قدر **),
  خالی (*xFinal)(sqlite3_context*),
  خالی (*x_قدر)(sqlite3_context*),
  خالی (*xInverse)(sqlite3_context*,صحیح,sqlite3_قدر **),
  FuncDestructor *pDestructor
){
  FuncDef *p;
  صحیح extraFlags;

  أكد( sqlite3_mutex_held(db->mutex) );
  أكد( x_قدر==0 || xSFunc==0 );
  اگر( zFunctionName==0                /* Must have a valid name */
   || (xSFunc!=0 && xFinal!=0)        /* Not both xSFunc and xFinal */
   || ((xFinal==0)!=(x_قدم==0))       /* Both or neither of xFinal and xStep */
   || ((x_قدر==0)!=(xInverse==0))    /* Both or neither of xValue, xInverse */
   || (nArg<-1 || nArg>SQLITE_MAX_FUNCTION_ARG)
   || (255<sqlite3Strlen30(zFunctionName))
  ){
    واپس SQLITE_MISUSE_BKPT;
  }

  أكد( SQLITE_FUNC_CONSTANT==SQLITE_DETERMINISTIC );
  أكد( SQLITE_FUNC_براہ_راست==SQLITE_DIRECTONLY );
  extraFlags = enc &  (SQLITE_DETERMINISTIC|SQLITE_DIRECTONLY|
                       SQLITE_SUBTYPE|SQLITE_INNOCUOUS|
                       SQLITE_نتیجہ_SUBTYPE|SQLITE_SELFORDER1);
  enc &= (SQLITE_FUNC_ENCMASK|SQLITE_ANY);

  /* The SQLITE_INNOCUOUS flag is the same bit as SQLITE_FUNC_UNSAFE.  But
  ** the meaning is inverted.  So flip the bit. */
  أكد( SQLITE_FUNC_UNSAFE==SQLITE_INNOCUOUS );
  extraFlags ^= SQLITE_FUNC_UNSAFE;  /* tag-20230109-1 */

 
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_UTF16
  /* If SQLITE_UTF16 is specified as the encoding type, transform this
  ** to one of SQLITE_UTF16LE or SQLITE_UTF16BE using the
  ** SQLITE_UTF16NATIVE macro. SQLITE_UTF16 is not used internally.
  **
  ** If SQLITE_ANY is specified, add three versions of the function
  ** to the hash table.
  */
  انتخاب( enc ){
    حالت SQLITE_UTF16:
      enc = SQLITE_UTF16NATIVE;
      توڑو;
    حالت SQLITE_ANY: {
      صحیح rc;
      rc = sqlite3CreateFunc(db, zFunctionName, nArg,
           (SQLITE_UTF8|extraFlags)^SQLITE_FUNC_UNSAFE, /* tag-20230109-1 */
           p_User_ڈیٹا, xSFunc, x_قدم, xFinal, x_قدر, xInverse, pDestructor);
      اگر( rc==SQLITE_OK ){
        rc = sqlite3CreateFunc(db, zFunctionName, nArg,
             (SQLITE_UTF16LE|extraFlags)^SQLITE_FUNC_UNSAFE, /* tag-20230109-1*/
             p_User_ڈیٹا, xSFunc, x_قدم, xFinal, x_قدر, xInverse, pDestructor);
      }
      اگر( rc!=SQLITE_OK ){
        واپس rc;
      }
      enc = SQLITE_UTF16BE;
      توڑو;
    }
    حالت SQLITE_UTF8:
    حالت SQLITE_UTF16LE:
    حالت SQLITE_UTF16BE:
      توڑو;
    طےشدہ:
      enc = SQLITE_UTF8;
      توڑو;
  }
#ورنہ
  enc = SQLITE_UTF8;
#اگر_ختم
 
  /* Check if an existing function is being overridden or deleted. If so,
  ** and there are active VMs, then return SQLITE_BUSY. If a function
  ** is being overridden/deleted but there are no active VMs, allow the
  ** operation to continue but invalidate all precompiled statements.
  */
  p = sqlite3_ڈھونڈو_Function(db, zFunctionName, nArg, (u8)enc, 0);
  اگر( p && (p->funcFlags & SQLITE_FUNC_ENCMASK)==(u32)enc && p->nArg==nArg ){
    اگر( db->n_Vdbe_فعال ){
      sqlite3_Error_With_پیغام(db, SQLITE_BUSY,
        "unable to delete/modify user-function due to active statements");
      أكد( !db->mallocFailed || db->nVdbeExec>0 );
      واپس SQLITE_BUSY;
    }ورنہ{
      sqlite3ExpirePreparedStatements(db, 0);
    }
  }ورنہ اگر( xSFunc==0 && xFinal==0 ){
    /* Trying to delete a function that does not exist.  This is a no-op.
    ** https://sqlite.org/forum/forumpost/726219164b */
    واپس SQLITE_OK;
  }

  p = sqlite3_ڈھونڈو_Function(db, zFunctionName, nArg, (u8)enc, 1);
  أكد(p || db->mallocFailed);
  اگر( !p ){
    واپس SQLITE_NOMEM_BKPT;
  }

  /* If an older version of the function with a configured destructor is
  ** being replaced invoke the destructor function here. */
  functionDestroy(db, p);

  اگر( pDestructor ){
    pDestructor->nRef++;
  }
  p->u.pDestructor = pDestructor;
  p->funcFlags = (p->funcFlags & SQLITE_FUNC_ENCMASK) | extraFlags;
  testcase( p->funcFlags & SQLITE_DETERMINISTIC );
  testcase( p->funcFlags & SQLITE_DIRECTONLY );
  p->xSFunc = xSFunc ? xSFunc : x_قدم;
  p->xFinalize = xFinal;
  p->x_قدر = x_قدر;
  p->xInverse = xInverse;
  p->p_User_ڈیٹا = p_User_ڈیٹا;
  p->nArg = (u16)nArg;
  واپس SQLITE_OK;
}

/*
** Worker function used by utf-8 APIs that create new functions:
**
**    sqlite3_create_function()
**    sqlite3_create_function_v2()
**    sqlite3_create_window_function()
*/
ساکن صحیح createFunctionApi(
  sqlite3 *db,
  ثابت حرف *zFunc,
  صحیح nArg,
  صحیح enc,
  خالی *p,
  خالی (*xSFunc)(sqlite3_context*,صحیح,sqlite3_قدر**),
  خالی (*x_قدم)(sqlite3_context*,صحیح,sqlite3_قدر**),
  خالی (*xFinal)(sqlite3_context*),
  خالی (*x_قدر)(sqlite3_context*),
  خالی (*xInverse)(sqlite3_context*,صحیح,sqlite3_قدر**),
  خالی(*xDestroy)(خالی*)
){
  صحیح rc = SQLITE_ERROR;
  FuncDestructor *pArg = 0;

#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db) ){
    واپس SQLITE_MISUSE_BKPT;
  }
#اگر_ختم
  sqlite3_mutex_enter(db->mutex);
  اگر( xDestroy ){
    pArg = (FuncDestructor *)sqlite3Malloc(حجم(FuncDestructor));
    اگر( !pArg ){
      sqlite3OomFault(db);
      xDestroy(p);
      جاؤ برآمد;
    }
    pArg->nRef = 0;
    pArg->xDestroy = xDestroy;
    pArg->p_User_ڈیٹا = p;
  }
  rc = sqlite3CreateFunc(db, zFunc, nArg, enc, p,
      xSFunc, x_قدم, xFinal, x_قدر, xInverse, pArg
  );
  اگر( pArg && pArg->nRef==0 ){
    أكد( rc!=SQLITE_OK || (x_قدم==0 && xFinal==0) );
    xDestroy(p);
    sqlite3_free(pArg);
  }

 برآمد:
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  واپس rc;
}

/*
** Create new user functions.
*/
صحیح sqlite3_create_function(
  sqlite3 *db,
  ثابت حرف *zFunc,
  صحیح nArg,
  صحیح enc,
  خالی *p,
  خالی (*xSFunc)(sqlite3_context*,صحیح,sqlite3_قدر **),
  خالی (*x_قدم)(sqlite3_context*,صحیح,sqlite3_قدر **),
  خالی (*xFinal)(sqlite3_context*)
){
  واپس createFunctionApi(db, zFunc, nArg, enc, p, xSFunc, x_قدم,
                                    xFinal, 0, 0, 0);
}
صحیح sqlite3_create_function_v2(
  sqlite3 *db,
  ثابت حرف *zFunc,
  صحیح nArg,
  صحیح enc,
  خالی *p,
  خالی (*xSFunc)(sqlite3_context*,صحیح,sqlite3_قدر **),
  خالی (*x_قدم)(sqlite3_context*,صحیح,sqlite3_قدر **),
  خالی (*xFinal)(sqlite3_context*),
  خالی (*xDestroy)(خالی *)
){
  واپس createFunctionApi(db, zFunc, nArg, enc, p, xSFunc, x_قدم,
                                    xFinal, 0, 0, xDestroy);
}
صحیح sqlite3_create_کھڑکی_function(
  sqlite3 *db,
  ثابت حرف *zFunc,
  صحیح nArg,
  صحیح enc,
  خالی *p,
  خالی (*x_قدم)(sqlite3_context*,صحیح,sqlite3_قدر **),
  خالی (*xFinal)(sqlite3_context*),
  خالی (*x_قدر)(sqlite3_context*),
  خالی (*xInverse)(sqlite3_context*,صحیح,sqlite3_قدر **),
  خالی (*xDestroy)(خالی *)
){
  واپس createFunctionApi(db, zFunc, nArg, enc, p, 0, x_قدم,
                                    xFinal, x_قدر, xInverse, xDestroy);
}

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_UTF16
صحیح sqlite3_create_function16(
  sqlite3 *db,
  ثابت خالی *zFunctionName,
  صحیح nArg,
  صحیح e_متن_Rep,
  خالی *p,
  خالی (*xSFunc)(sqlite3_context*,صحیح,sqlite3_قدر**),
  خالی (*x_قدم)(sqlite3_context*,صحیح,sqlite3_قدر**),
  خالی (*xFinal)(sqlite3_context*)
){
  صحیح rc;
  حرف *zFunc8;

#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db) || zFunctionName==0 ) واپس SQLITE_MISUSE_BKPT;
#اگر_ختم
  sqlite3_mutex_enter(db->mutex);
  أكد( !db->mallocFailed );
  zFunc8 = sqlite3Utf16to8(db, zFunctionName, -1, SQLITE_UTF16NATIVE);
  rc = sqlite3CreateFunc(db, zFunc8, nArg, e_متن_Rep, p, xSFunc,x_قدم,xFinal,0,0,0);
  sqlite3DbFree(db, zFunc8);
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  واپس rc;
}
#اگر_ختم


/*
** The following is the implementation of an SQL function that always
** fails with an error message stating that the function is used in the
** wrong context.  The sqlite3_overload_function() API might construct
** SQL function that use this routine so that the functions will exist
** for name resolution but are actually overloaded by the xFindFunction
** method of virtual tables.
*/
ساکن خالی sqlite3InvalidFunction(
  sqlite3_context *context,  /* The function calling context */
  صحیح Not_مستعمل,               /* Number of arguments to the function */
  sqlite3_قدر **NotUsed2   /* Value of each argument */
){
  ثابت حرف *zName = (ثابت حرف*)sqlite3_user_ڈیٹا(context);
  حرف *zErr;
  غیر_مستعمل_PARAMETER2(Not_مستعمل, NotUsed2);
  zErr = sqlite3_mprintf(
      "unable to use function %s in the requested context", zName);
  sqlite3_نتیجہ_error(context, zErr, -1);
  sqlite3_free(zErr);
}

/*
** Declare that a function has been overloaded by a virtual table.
**
** If the function already exists as a regular global function, then
** this routine is a no-op.  If the function does not exist, then create
** a new one that always throws a run-time error. 
**
** When virtual tables intend to provide an overloaded function, they
** should call this routine to make sure the global function exists.
** A global function must exist in order for name resolution to work
** properly.
*/
صحیح sqlite3_overload_function(
  sqlite3 *db,
  ثابت حرف *zName,
  صحیح nArg
){
  صحیح rc;
  حرف *zCopy;

#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db) || zName==0 || nArg<-2 ){
    واپس SQLITE_MISUSE_BKPT;
  }
#اگر_ختم
  sqlite3_mutex_enter(db->mutex);
  rc = sqlite3_ڈھونڈو_Function(db, zName, nArg, SQLITE_UTF8, 0)!=0;
  sqlite3_mutex_leave(db->mutex);
  اگر( rc ) واپس SQLITE_OK;
  zCopy = sqlite3_mprintf("%s", zName);
  اگر( zCopy==0 ) واپس SQLITE_NOMEM;
  واپس sqlite3_create_function_v2(db, zName, nArg, SQLITE_UTF8,
                           zCopy, sqlite3InvalidFunction, 0, 0, sqlite3_free);
}

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_TRACE
/*
** Register a trace function.  The pArg from the previously registered trace
** is returned. 
**
** A NULL trace function means that no tracing is executes.  A non-NULL
** trace is a pointer to a function that is invoked at the start of each
** SQL statement.
*/
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_DEPRECATED
خالی *sqlite3_trace(sqlite3 *db, خالی(*xTrace)(خالی*,ثابت حرف*), خالی *pArg){
  خالی *pOld;

#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db) ){
    (خالی)SQLITE_MISUSE_BKPT;
    واپس 0;
  }
#اگر_ختم
  sqlite3_mutex_enter(db->mutex);
  pOld = db->pTraceArg;
  db->mTrace = xTrace ? SQLITE_TRACE_LEGACY : 0;
  db->trace.xLegacy = xTrace;
  db->pTraceArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  واپس pOld;
}
#اگر_ختم /* SQLITE_OMIT_DEPRECATED */

/* Register a trace callback using the version-2 interface.
*/
صحیح sqlite3_trace_v2(
  sqlite3 *db,                               /* Trace this connection */
  غیرعلامتی mTrace,                           /* Mask of events to be traced */
  صحیح(*xTrace)(غیرعلامتی,خالی*,خالی*,خالی*),  /* Callback to invoke */
  خالی *pArg                                 /* Context */
){
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db) ){
    واپس SQLITE_MISUSE_BKPT;
  }
#اگر_ختم
  sqlite3_mutex_enter(db->mutex);
  اگر( mTrace==0 ) xTrace = 0;
  اگر( xTrace==0 ) mTrace = 0;
  db->mTrace = mTrace;
  db->trace.xV2 = xTrace;
  db->pTraceArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  واپس SQLITE_OK;
}

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_DEPRECATED
/*
** Register a profile function.  The pArg from the previously registered
** profile function is returned. 
**
** A NULL profile function means that no profiling is executes.  A non-NULL
** profile is a pointer to a function that is invoked at the conclusion of
** each SQL statement that is run.
*/
خالی *sqlite3_profile(
  sqlite3 *db,
  خالی (*xProfile)(خالی*,ثابت حرف*,sqlite_uint64),
  خالی *pArg
){
  خالی *pOld;

#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db) ){
    (خالی)SQLITE_MISUSE_BKPT;
    واپس 0;
  }
#اگر_ختم
  sqlite3_mutex_enter(db->mutex);
  pOld = db->pProfileArg;
  db->xProfile = xProfile;
  db->pProfileArg = pArg;
  db->mTrace &= SQLITE_TRACE_NONLEGACY_نقاب;
  اگر( db->xProfile ) db->mTrace |= SQLITE_TRACE_XPROFILE;
  sqlite3_mutex_leave(db->mutex);
  واپس pOld;
}
#اگر_ختم /* SQLITE_OMIT_DEPRECATED */
#اگر_ختم /* SQLITE_OMIT_TRACE */

/*
** Register a function to be invoked when a transaction commits.
** If the invoked function returns non-zero, then the commit becomes a
** rollback.
*/
خالی *sqlite3_commit_hook(
  sqlite3 *db,              /* Attach the hook to this database */
  صحیح (*xCallback)(خالی*),  /* Function to invoke on each commit */
  خالی *pArg                /* Argument to the function */
){
  خالی *pOld;

#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db) ){
    (خالی)SQLITE_MISUSE_BKPT;
    واپس 0;
  }
#اگر_ختم
  sqlite3_mutex_enter(db->mutex);
  pOld = db->pCommitArg;
  db->xCommitCallback = xCallback;
  db->pCommitArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  واپس pOld;
}

/*
** Register a callback to be invoked each time a row is updated,
** inserted or deleted using this database connection.
*/
خالی *sqlite3_update_hook(
  sqlite3 *db,              /* Attach the hook to this database */
  خالی (*xCallback)(خالی*,صحیح,حرف ثابت *,حرف ثابت *,sqlite_int64),
  خالی *pArg                /* Argument to the function */
){
  خالی *pRet;

#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db) ){
    (خالی)SQLITE_MISUSE_BKPT;
    واپس 0;
  }
#اگر_ختم
  sqlite3_mutex_enter(db->mutex);
  pRet = db->pUpdateArg;
  db->xUpdateCallback = xCallback;
  db->pUpdateArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  واپس pRet;
}

/*
** Register a callback to be invoked each time a transaction is rolled
** back by this database connection.
*/
خالی *sqlite3_rollback_hook(
  sqlite3 *db,              /* Attach the hook to this database */
  خالی (*xCallback)(خالی*), /* Callback function */
  خالی *pArg                /* Argument to the function */
){
  خالی *pRet;

#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db) ){
    (خالی)SQLITE_MISUSE_BKPT;
    واپس 0;
  }
#اگر_ختم
  sqlite3_mutex_enter(db->mutex);
  pRet = db->pRollbackArg;
  db->xRollbackCallback = xCallback;
  db->pRollbackArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  واپس pRet;
}

#اگر_تعریف_شدہ SQLITE_ENABLE_PREUPDATE_HOOK
/*
** Register a callback to be invoked each time a row is updated,
** inserted or deleted using this database connection.
*/
خالی *sqlite3_preupdate_hook(
  sqlite3 *db,              /* Attach the hook to this database */
  خالی(*xCallback)(         /* Callback function */
    خالی*,sqlite3*,صحیح,حرف ثابت*,حرف ثابت*,sqlite3_int64,sqlite3_int64),
  خالی *pArg                /* First callback argument */
){
  خالی *pRet;

#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( db==0 ){
    واپس 0;
  }
#اگر_ختم
  sqlite3_mutex_enter(db->mutex);
  pRet = db->pPreUpdateArg;
  db->xPreUpdateCallback = xCallback;
  db->pPreUpdateArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  واپس pRet;
}
#اگر_ختم /* SQLITE_ENABLE_PREUPDATE_HOOK */

/*
** Register a function to be invoked prior to each autovacuum that
** determines the number of pages to vacuum.
*/
صحیح sqlite3_autovacuum_صفحات(
  sqlite3 *db,                 /* Attach the hook to this database */
  غیرعلامتی صحیح (*xCallback)(خالی*,ثابت حرف*,u32,u32,u32),
  خالی *pArg,                  /* Argument to the function */
  خالی (*xDestructor)(خالی*)   /* Destructor for pArg */
){
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db) ){
    اگر( xDestructor ) xDestructor(pArg);
    واپس SQLITE_MISUSE_BKPT;
  }
#اگر_ختم
  sqlite3_mutex_enter(db->mutex);
  اگر( db->xAutovacDestr ){
    db->xAutovacDestr(db->p_Autovac_صفحات_Arg);
  }
  db->x_Autovac_صفحات = xCallback;
  db->p_Autovac_صفحات_Arg = pArg;
  db->xAutovacDestr = xDestructor;
  sqlite3_mutex_leave(db->mutex);
  واپس SQLITE_OK;
}


#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WAL
/*
** The sqlite3_wal_hook() callback registered by sqlite3_wal_autocheckpoint().
** Invoke sqlite3_wal_checkpoint if the number of frames in the log file
** is greater than sqlite3.pWalArg cast to an integer (the value configured by
** wal_autocheckpoint()).
*/
صحیح sqlite3WalDefaultHook(
  خالی *p_Client_ڈیٹا,     /*دلیل*/
  sqlite3 *db,           /* Connection */
  ثابت حرف *zDb,       /* Database */
  صحیح n_چوکھٹا             /* Size of WAL */
){
  اگر( n_چوکھٹا>=SQLITE_PTR_مقصد_INT(p_Client_ڈیٹا) ){
    sqlite3BeginBenignMalloc();
    sqlite3_wal_checkpoint(db, zDb);
    sqlite3_انتہا_Benign_Malloc();
  }
  واپس SQLITE_OK;
}
#اگر_ختم /* SQLITE_OMIT_WAL */

/*
** Configure an sqlite3_wal_hook() callback to automatically checkpoint
** a database after committing a transaction if there are nFrame or
** more frames in the log file. Passing zero or a negative value as the
** nFrame parameter disables automatic checkpoints entirely.
**
** The callback registered by this function replaces any existing callback
** registered using sqlite3_wal_hook(). Likewise, registering a callback
** using sqlite3_wal_hook() disables the automatic checkpoint mechanism
** configured by this function.
*/
صحیح sqlite3_wal_autocheckpoint(sqlite3 *db, صحیح n_چوکھٹا){
#اگر_تعریف_شدہ SQLITE_OMIT_WAL
  غیر_مستعمل_PARAMETER(db);
  غیر_مستعمل_PARAMETER(n_چوکھٹا);
#ورنہ
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db) ) واپس SQLITE_MISUSE_BKPT;
#اگر_ختم
  اگر( n_چوکھٹا>0 ){
    sqlite3_wal_hook(db, sqlite3WalDefaultHook, SQLITE_INT_مقصد_PTR(n_چوکھٹا));
  }ورنہ{
    sqlite3_wal_hook(db, 0, 0);
  }
#اگر_ختم
  واپس SQLITE_OK;
}

/*
** Register a callback to be invoked each time a transaction is written
** into the write-ahead-log by this database connection.
*/
خالی *sqlite3_wal_hook(
  sqlite3 *db,                    /* Attach the hook to this db handle */
  صحیح(*xCallback)(خالی *, sqlite3*, ثابت حرف*, صحیح),
  خالی *pArg                      /* First argument passed to xCallback() */
){
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WAL
  خالی *pRet;
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db) ){
    (خالی)SQLITE_MISUSE_BKPT;
    واپس 0;
  }
#اگر_ختم
  sqlite3_mutex_enter(db->mutex);
  pRet = db->pWalArg;
  db->xWalCallback = xCallback;
  db->pWalArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  واپس pRet;
#ورنہ
  غیر_مستعمل_PARAMETER(db);
  غیر_مستعمل_PARAMETER(xCallback);
  غیر_مستعمل_PARAMETER(pArg);
  واپس 0;
#اگر_ختم
}

/*
** Checkpoint database zDb.
*/
صحیح sqlite3_wal_checkpoint_v2(
  sqlite3 *db,                    /* Database handle */
  ثابت حرف *zDb,                /* Name of attached database (or NULL) */
  صحیح e_منوال,                      /* SQLITE_CHECKPOINT_* value */
  صحیح *pnLog,                     /* OUT: Size of WAL log in frames */
  صحیح *pnCkpt                     /* OUT: Total number of frames checkpointed */
){
#اگر_تعریف_شدہ SQLITE_OMIT_WAL
  غیر_مستعمل_PARAMETER(db);
  غیر_مستعمل_PARAMETER(zDb);
  غیر_مستعمل_PARAMETER(e_منوال);
  غیر_مستعمل_PARAMETER(pnLog);
  غیر_مستعمل_PARAMETER(pnCkpt);
  واپس SQLITE_OK;
#ورنہ
  صحیح rc;                         /* Return code */
  صحیح iDb;                        /* Schema to checkpoint */

#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db) ) واپس SQLITE_MISUSE_BKPT;
#اگر_ختم

  /* Initialize the output variables to -1 in case an error occurs. */
  اگر( pnLog ) *pnLog = -1;
  اگر( pnCkpt ) *pnCkpt = -1;

  أكد( SQLITE_CHECKPOINT_NOOP==-1 );
  أكد( SQLITE_CHECKPOINT_PASSIVE==0 );
  أكد( SQLITE_CHECKPOINT_بھرا==1 );
  أكد( SQLITE_CHECKPOINT_RESTART==2 );
  أكد( SQLITE_CHECKPOINT_TRUNCATE==3 );
  اگر( e_منوال<SQLITE_CHECKPOINT_NOOP || e_منوال>SQLITE_CHECKPOINT_TRUNCATE ){
    /* EVIDENCE-OF: R-03996-12088 The M parameter must be a valid checkpoint
    ** mode: */
    واپس SQLITE_MISUSE_BKPT;
  }

  sqlite3_mutex_enter(db->mutex);
  اگر( zDb && zDb[0] ){
    iDb = sqlite3_ڈھونڈو_Db_Name(db, zDb);
  }ورنہ{
    iDb = SQLITE_MAX_DB;   /* This means process all schemas */
  }
  اگر( iDb<0 ){
    rc = SQLITE_ERROR;
    sqlite3_Error_With_پیغام(db, SQLITE_ERROR, "unknown database: %s", zDb);
  }ورنہ{
    db->busyHandler.nBusy = 0;
    rc = sqlite3Checkpoint(db, iDb, e_منوال, pnLog, pnCkpt);
    sqlite3Error(db, rc);
  }
  rc = sqlite3ApiExit(db, rc);

  /* If there are no active statements, clear the interrupt flag at this
  ** point.  */
  اگر( db->n_Vdbe_فعال==0 ){
    AtomicStore(&db->u1.isInterrupted, 0);
  }

  sqlite3_mutex_leave(db->mutex);
  واپس rc;
#اگر_ختم
}


/*
** Checkpoint database zDb. If zDb is NULL, or if the buffer zDb points
** to contains a zero-length string, all attached databases are
** checkpointed.
*/
صحیح sqlite3_wal_checkpoint(sqlite3 *db, ثابت حرف *zDb){
  /* EVIDENCE-OF: R-41613-20553 The sqlite3_wal_checkpoint(D,X) is equivalent to
  ** sqlite3_wal_checkpoint_v2(D,X,SQLITE_CHECKPOINT_PASSIVE,0,0). */
  واپس sqlite3_wal_checkpoint_v2(db,zDb,SQLITE_CHECKPOINT_PASSIVE,0,0);
}

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WAL
/*
** Run a checkpoint on database iDb. This is a no-op if database iDb is
** not currently open in WAL mode.
**
** If a transaction is open on the database being checkpointed, this
** function returns SQLITE_LOCKED and a checkpoint is not attempted. If
** an error occurs while running the checkpoint, an SQLite error code is
** returned (i.e. SQLITE_IOERR). Otherwise, SQLITE_OK.
**
** The mutex on database handle db should be held by the caller. The mutex
** associated with the specific b-tree being checkpointed is taken by
** this function while the checkpoint is running.
**
** If iDb is passed SQLITE_MAX_DB then all attached databases are
** checkpointed. If an error is encountered it is returned immediately -
** no attempt is made to checkpoint any remaining databases.
**
** Parameter eMode is one of SQLITE_CHECKPOINT_PASSIVE, FULL, RESTART
** or TRUNCATE.
*/
صحیح sqlite3Checkpoint(sqlite3 *db, صحیح iDb, صحیح e_منوال, صحیح *pnLog, صحیح *pnCkpt){
  صحیح rc = SQLITE_OK;             /* Return code */
  صحیح i;                          /* Used to iterate through attached dbs */
  صحیح bBusy = 0;                  /* True if SQLITE_BUSY has been encountered */

  أكد( sqlite3_mutex_held(db->mutex) );
  أكد( !pnLog || *pnLog==-1 );
  أكد( !pnCkpt || *pnCkpt==-1 );
  testcase( iDb==SQLITE_MAX_ATTACHED ); /* See forum post a006d86f72 */
  testcase( iDb==SQLITE_MAX_DB );

  برائے(i=0; i<db->nDb && rc==SQLITE_OK; i++){
    اگر( i==iDb || iDb==SQLITE_MAX_DB ){
      rc = sqlite3BtreeCheckpoint(db->aDb[i].pBt, e_منوال, pnLog, pnCkpt);
      pnLog = 0;
      pnCkpt = 0;
      اگر( rc==SQLITE_BUSY ){
        bBusy = 1;
        rc = SQLITE_OK;
      }
    }
  }

  واپس (rc==SQLITE_OK && bBusy) ? SQLITE_BUSY : rc;
}
#اگر_ختم /* SQLITE_OMIT_WAL */

/*
** This function returns true if main-memory should be used instead of
** a temporary file for transient pager files and statement journals.
** The value returned depends on the value of db->temp_store (runtime
** parameter) and the compile time value of SQLITE_TEMP_STORE. The
** following table describes the relationship between these two values
** and this functions return value.
**
**   SQLITE_TEMP_STORE     db->temp_store     Location of temporary database
**   -----------------     --------------     ------------------------------
**   0                     any                file      (return 0)
**   1                     1                  file      (return 0)
**   1                     2                  memory    (return 1)
**   1                     0                  file      (return 0)
**   2                     1                  file      (return 0)
**   2                     2                  memory    (return 1)
**   2                     0                  memory    (return 1)
**   3                     any                memory    (return 1)
*/
صحیح sqlite3_عارضی_اندر_حافظہ(ثابت sqlite3 *db){
#اگر SQLITE_عارضی_STORE==1
  واپس ( db->عارضی_store==2 );
#اگر_ختم
#اگر SQLITE_عارضی_STORE==2
  واپس ( db->عارضی_store!=1 );
#اگر_ختم
#اگر SQLITE_عارضی_STORE==3
  غیر_مستعمل_PARAMETER(db);
  واپس 1;
#اگر_ختم
#اگر SQLITE_عارضی_STORE<1 || SQLITE_عارضی_STORE>3
  غیر_مستعمل_PARAMETER(db);
  واپس 0;
#اگر_ختم
}

/*
** Return UTF-8 encoded English language explanation of the most recent
** error.
*/
ثابت حرف *sqlite3_errmsg(sqlite3 *db){
  ثابت حرف *z;
  اگر( !db ){
    واپس sqlite3ErrStr(SQLITE_NOMEM_BKPT);
  }
  اگر( !sqlite3SafetyCheckSickOrOk(db) ){
    واپس sqlite3ErrStr(SQLITE_MISUSE_BKPT);
  }
  sqlite3_mutex_enter(db->mutex);
  اگر( db->mallocFailed ){
    z = sqlite3ErrStr(SQLITE_NOMEM_BKPT);
  }ورنہ{
    testcase( db->pErr==0 );
    z = db->err_رمز ? (حرف*)sqlite3_قدر_متن(db->pErr) : 0;
    أكد( !db->mallocFailed );
    اگر( z==0 ){
      z = sqlite3ErrStr(db->err_رمز);
    }
  }
  sqlite3_mutex_leave(db->mutex);
  واپس z;
}

/*
** Set the error code and error message associated with the database handle.
**
** This routine is intended to be called by outside extensions (ex: the
** Session extension). Internal logic should invoke sqlite3Error() or
** sqlite3ErrorWithMsg() directly.
*/
صحیح sqlite3_سیٹ_errmsg(sqlite3 *db, صحیح errcode, ثابت حرف *z_پیغام){
  صحیح rc = SQLITE_OK;
  اگر( !sqlite3SafetyCheckOk(db) ){
    واپس SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(db->mutex);
  اگر( z_پیغام ){
    sqlite3_Error_With_پیغام(db, errcode, "%s", z_پیغام);
  }ورنہ{
    sqlite3Error(db, errcode);
  }
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  واپس rc;
}

/*
** Return the byte offset of the most recent error
*/
صحیح sqlite3_error_انحراف(sqlite3 *db){
  صحیح i_انحراف = -1;
  اگر( db && sqlite3SafetyCheckSickOrOk(db) ){
    sqlite3_mutex_enter(db->mutex);
    اگر( db->err_رمز ){
      i_انحراف = db->err_Byte_انحراف;
    }
    sqlite3_mutex_leave(db->mutex);
  }
  واپس i_انحراف;
}

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_UTF16
/*
** Return UTF-16 encoded English language explanation of the most recent
** error.
*/
ثابت خالی *sqlite3_errmsg16(sqlite3 *db){
  ساکن ثابت u16 برآمد_کا_Mem[] = {
    'o', 'u', 't', ' ', 'o', 'f', ' ', 'm', 'e', 'm', 'o', 'r', 'y', 0
  };
  ساکن ثابت u16 misuse[] = {
    'b', 'a', 'd', ' ', 'p', 'a', 'r', 'a', 'm', 'e', 't', 'e', 'r', ' ',
    'o', 'r', ' ', 'o', 't', 'h', 'e', 'r', ' ', 'A', 'P', 'I', ' ',
    'm', 'i', 's', 'u', 's', 'e', 0
  };

  ثابت خالی *z;
  اگر( !db ){
    واپس (خالی *)برآمد_کا_Mem;
  }
  اگر( !sqlite3SafetyCheckSickOrOk(db) ){
    واپس (خالی *)misuse;
  }
  sqlite3_mutex_enter(db->mutex);
  اگر( db->mallocFailed ){
    z = (خالی *)برآمد_کا_Mem;
  }ورنہ{
    z = sqlite3_قدر_text16(db->pErr);
    اگر( z==0 ){
      sqlite3_Error_With_پیغام(db, db->err_رمز, sqlite3ErrStr(db->err_رمز));
      z = sqlite3_قدر_text16(db->pErr);
    }
    /* A malloc() may have failed within the call to sqlite3_value_text16()
    ** above. If this is the case, then the db->mallocFailed flag needs to
    ** be cleared before returning. Do this directly, instead of via
    ** sqlite3ApiExit(), to avoid setting the database handle error message.
    */
    sqlite3OomClear(db);
  }
  sqlite3_mutex_leave(db->mutex);
  واپس z;
}
#اگر_ختم /* SQLITE_OMIT_UTF16 */

/*
** Return the most recent error code generated by an SQLite routine. If NULL is
** passed to this function, we assume a malloc() failed during sqlite3_open().
*/
صحیح sqlite3_errcode(sqlite3 *db){
  صحیح iRet;
  اگر( !db ) واپس SQLITE_NOMEM_BKPT;
  اگر( !sqlite3SafetyCheckSickOrOk(db) ){
    واپس SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(db->mutex);
  اگر( db->mallocFailed ){
    iRet = SQLITE_NOMEM_BKPT;
  }ورنہ{
    iRet = db->err_رمز & db->err_نقاب;
  }
  sqlite3_mutex_leave(db->mutex);
  واپس iRet;
}
صحیح sqlite3_extended_errcode(sqlite3 *db){
  صحیح iRet;
  اگر( !db ) واپس SQLITE_NOMEM_BKPT;
  اگر( !sqlite3SafetyCheckSickOrOk(db) ){
    واپس SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(db->mutex);
  اگر( db->mallocFailed ){
    iRet = SQLITE_NOMEM_BKPT;
  }ورنہ{
    iRet = db->err_رمز;
  }
  sqlite3_mutex_leave(db->mutex);
  واپس iRet;
}
صحیح sqlite3_system_errno(sqlite3 *db){
  صحیح iRet = 0;
  اگر( db ){
    sqlite3_mutex_enter(db->mutex);
    iRet = db->iSysErrno;
    sqlite3_mutex_leave(db->mutex);
  }
  واپس iRet;
} 

/*
** Return a string that describes the kind of error specified in the
** argument.  For now, this simply calls the internal sqlite3ErrStr()
** function.
*/
ثابت حرف *sqlite3_errstr(صحیح rc){
  واپس sqlite3ErrStr(rc);
}

/*
** Create a new collating function for database "db".  The name is zName
** and the encoding is enc.
*/
ساکن صحیح createCollation(
  sqlite3* db,
  ثابت حرف *zName,
  u8 enc,
  خالی* pCtx,
  صحیح(*xCompare)(خالی*,صحیح,ثابت خالی*,صحیح,ثابت خالی*),
  خالی(*xDel)(خالی*)
){
  CollSeq *pColl;
  صحیح enc2;
 
  أكد( sqlite3_mutex_held(db->mutex) );

  /* If SQLITE_UTF16 is specified as the encoding type, transform this
  ** to one of SQLITE_UTF16LE or SQLITE_UTF16BE using the
  ** SQLITE_UTF16NATIVE macro. SQLITE_UTF16 is not used internally.
  */
  enc2 = enc;
  testcase( enc2==SQLITE_UTF16 );
  testcase( enc2==SQLITE_UTF16_ALIGNED );
  اگر( enc2==SQLITE_UTF16 || enc2==SQLITE_UTF16_ALIGNED ){
    enc2 = SQLITE_UTF16NATIVE;
  }
  اگر( enc2<SQLITE_UTF8 || enc2>SQLITE_UTF16BE ){
    واپس SQLITE_MISUSE_BKPT;
  }

  /* Check if this call is removing or replacing an existing collation
  ** sequence. If so, and there are active VMs, return busy. If there
  ** are no active VMs, invalidate any pre-compiled statements.
  */
  pColl = sqlite3_ڈھونڈو_Coll_Seq(db, (u8)enc2, zName, 0);
  اگر( pColl && pColl->xCmp ){
    اگر( db->n_Vdbe_فعال ){
      sqlite3_Error_With_پیغام(db, SQLITE_BUSY,
        "unable to delete/modify collation sequence due to active statements");
      واپس SQLITE_BUSY;
    }
    sqlite3ExpirePreparedStatements(db, 0);

    /* If collation sequence pColl was created directly by a call to
    ** sqlite3_create_collation, and not generated by synthCollSeq(),
    ** then any copies made by synthCollSeq() need to be invalidated.
    ** Also, collation destructor - CollSeq.xDel() - function may need
    ** to be called.
    */
    اگر( (pColl->enc & ~SQLITE_UTF16_ALIGNED)==enc2 ){
      CollSeq *aColl = sqlite3_Hash_ڈھونڈو(&db->aCollSeq, zName);
      صحیح j;
      برائے(j=0; j<3; j++){
        CollSeq *p = &aColl[j];
        اگر( p->enc==pColl->enc ){
          اگر( p->xDel ){
            p->xDel(p->pUser);
          }
          p->xCmp = 0;
        }
      }
    }
  }

  pColl = sqlite3_ڈھونڈو_Coll_Seq(db, (u8)enc2, zName, 1);
  اگر( pColl==0 ) واپس SQLITE_NOMEM_BKPT;
  pColl->xCmp = xCompare;
  pColl->pUser = pCtx;
  pColl->xDel = xDel;
  pColl->enc = (u8)(enc2 | (enc & SQLITE_UTF16_ALIGNED));
  sqlite3Error(db, SQLITE_OK);
  واپس SQLITE_OK;
}


/*
** This array defines hard upper bounds on limit values.  The
** initializer must be kept in sync with the SQLITE_LIMIT_*
** #defines in sqlite3.h.
*/
ساکن ثابت صحیح a_Hard_حد_بندی[] = {
  SQLITE_MAX_لمبائی,
  SQLITE_MAX_SQL_لمبائی,
  SQLITE_MAX_عمود,
  SQLITE_MAX_اظہاریہ_گہرائی,
  SQLITE_MAX_COMPOUND_SELECT,
  SQLITE_MAX_VDBE_OP,
  SQLITE_MAX_FUNCTION_ARG,
  SQLITE_MAX_ATTACHED,
  SQLITE_MAX_LIKE_نمونہ_لمبائی,
  SQLITE_MAX_VARIABLE_نمبر,      /* IMP: R-38091-32352 */
  SQLITE_MAX_TRIGGER_گہرائی,
  SQLITE_MAX_کارکن_دھاگے,
  SQLITE_MAX_PARSER_گہرائی,
  SQLITE_MAX_SCHEMA,
  SQLITE_MAX_TRIGGER_اقدام,
  SQLITE_MAX_CTES,
};

/*
** Make sure the hard limits are set to reasonable values
*/
#اگر SQLITE_MAX_لمبائی<100
# error SQLITE_MAX_لمبائی must be at کم_سے_کم 100
#اگر_ختم
#اگر SQLITE_MAX_SQL_لمبائی<100
# error SQLITE_MAX_SQL_لمبائی must be at کم_سے_کم 100
#اگر_ختم
#اگر SQLITE_MAX_SQL_لمبائی>SQLITE_MAX_لمبائی
# error SQLITE_MAX_SQL_لمبائی must not be greater than SQLITE_MAX_لمبائی
#اگر_ختم
#اگر SQLITE_MAX_SQL_لمبائی>2147482624     /* 1024 less than 2^31 */
# error SQLITE_MAX_SQL_لمبائی must not be greater than 2147482624
#اگر_ختم
#اگر SQLITE_MAX_COMPOUND_SELECT<2
# error SQLITE_MAX_COMPOUND_SELECT must be at کم_سے_کم 2
#اگر_ختم
#اگر SQLITE_MAX_VDBE_OP<40
# error SQLITE_MAX_VDBE_OP must be at کم_سے_کم 40
#اگر_ختم
#اگر SQLITE_MAX_FUNCTION_ARG<0 || SQLITE_MAX_FUNCTION_ARG>32767
# error SQLITE_MAX_FUNCTION_ARG must be between 0 and 32767
#اگر_ختم
#اگر SQLITE_MAX_ATTACHED<0 || SQLITE_MAX_ATTACHED>125
# error SQLITE_MAX_ATTACHED must be between 0 and 125
#اگر_ختم
#اگر SQLITE_MAX_LIKE_نمونہ_لمبائی<1
# error SQLITE_MAX_LIKE_نمونہ_لمبائی must be at کم_سے_کم 1
#اگر_ختم
#اگر SQLITE_MAX_عمود>32767
# error SQLITE_MAX_عمود must not exceed 32767
#اگر_ختم
#اگر SQLITE_MAX_TRIGGER_گہرائی<1
# error SQLITE_MAX_TRIGGER_گہرائی must be at کم_سے_کم 1
#اگر_ختم
#اگر SQLITE_MAX_کارکن_دھاگے<0 || SQLITE_MAX_کارکن_دھاگے>50
# error SQLITE_MAX_کارکن_دھاگے must be between 0 and 50
#اگر_ختم
#اگر SQLITE_MAX_TRIGGER_اقدام<0 || SQLITE_MAX_TRIGGER_اقدام>65535
# error SQLITE_MAX_TRIGGER_اقدام must be between 0 and 65535
#اگر_ختم


/*
** Change the value of a limit.  Report the old value.
** If an invalid limit index is supplied, report -1.
** Make no changes but still report the old value if the
** new limit is negative.
**
** A new lower limit does not shrink existing constructs.
** It merely prevents new constructs that exceed the limit
** from forming.
*/
صحیح sqlite3_حد_بندی(sqlite3 *db, صحیح حد_بندی_Id, صحیح new_حد_بندی){
  صحیح old_حد_بندی;

#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db) ){
    (خالی)SQLITE_MISUSE_BKPT;
    واپس -1;
  }
#اگر_ختم

  /* EVIDENCE-OF: R-30189-54097 For each limit category SQLITE_LIMIT_NAME
  ** there is a hard upper bound set at compile-time by a C preprocessor
  ** macro called SQLITE_MAX_NAME. (The "_LIMIT_" in the name is changed to
  ** "_MAX_".)
  */
  أكد( a_Hard_حد_بندی[SQLITE_حد_بندی_لمبائی]==SQLITE_MAX_لمبائی );
  أكد( a_Hard_حد_بندی[SQLITE_حد_بندی_SQL_لمبائی]==SQLITE_MAX_SQL_لمبائی );
  أكد( a_Hard_حد_بندی[SQLITE_حد_بندی_عمود]==SQLITE_MAX_عمود );
  أكد( a_Hard_حد_بندی[SQLITE_حد_بندی_اظہاریہ_گہرائی]==SQLITE_MAX_اظہاریہ_گہرائی );
  أكد( a_Hard_حد_بندی[SQLITE_حد_بندی_PARSER_گہرائی]==SQLITE_MAX_PARSER_گہرائی );
  أكد( a_Hard_حد_بندی[SQLITE_حد_بندی_COMPOUND_SELECT]==SQLITE_MAX_COMPOUND_SELECT);
  أكد( a_Hard_حد_بندی[SQLITE_حد_بندی_VDBE_OP]==SQLITE_MAX_VDBE_OP );
  أكد( a_Hard_حد_بندی[SQLITE_حد_بندی_FUNCTION_ARG]==SQLITE_MAX_FUNCTION_ARG );
  أكد( a_Hard_حد_بندی[SQLITE_حد_بندی_ATTACHED]==SQLITE_MAX_ATTACHED );
  أكد( a_Hard_حد_بندی[SQLITE_حد_بندی_LIKE_نمونہ_لمبائی]==
                                               SQLITE_MAX_LIKE_نمونہ_لمبائی );
  أكد( a_Hard_حد_بندی[SQLITE_حد_بندی_VARIABLE_نمبر]==SQLITE_MAX_VARIABLE_نمبر);
  أكد( a_Hard_حد_بندی[SQLITE_حد_بندی_TRIGGER_گہرائی]==SQLITE_MAX_TRIGGER_گہرائی );
  أكد( a_Hard_حد_بندی[SQLITE_حد_بندی_کارکن_دھاگے]==SQLITE_MAX_کارکن_دھاگے );
  أكد( a_Hard_حد_بندی[SQLITE_حد_بندی_SCHEMA]==SQLITE_MAX_SCHEMA );
  أكد( a_Hard_حد_بندی[SQLITE_حد_بندی_TRIGGER_اقدام]==SQLITE_MAX_TRIGGER_اقدام );
  أكد( a_Hard_حد_بندی[SQLITE_حد_بندی_CTES]==SQLITE_MAX_CTES );
  أكد( SQLITE_حد_بندی_CTES==(SQLITE_N_حد_بندی-1) );


  اگر( حد_بندی_Id<0 || حد_بندی_Id>=SQLITE_N_حد_بندی ){
    واپس -1;
  }
  sqlite3_mutex_enter(db->mutex);
  old_حد_بندی = db->a_حد_بندی[حد_بندی_Id];
  اگر( new_حد_بندی>=0 ){                   /* IMP: R-52476-28732 */
    اگر( new_حد_بندی>a_Hard_حد_بندی[حد_بندی_Id] ){
      new_حد_بندی = a_Hard_حد_بندی[حد_بندی_Id];  /* IMP: R-51463-25634 */
    }ورنہ اگر( new_حد_بندی<SQLITE_MIN_لمبائی && حد_بندی_Id==SQLITE_حد_بندی_لمبائی ){
      new_حد_بندی = SQLITE_MIN_لمبائی;
    }
    db->a_حد_بندی[حد_بندی_Id] = new_حد_بندی;
  }
  sqlite3_mutex_leave(db->mutex);
  واپس old_حد_بندی;                     /* IMP: R-53341-35419 */
}

/*
** This function is used to parse both URIs and non-URI filenames passed by the
** user to API functions sqlite3_open() or sqlite3_open_v2(), and for database
** URIs specified as part of ATTACH statements.
**
** The first argument to this function is the name of the VFS to use (or
** a NULL to signify the default VFS) if the URI does not contain a "vfs=xxx"
** query parameter. The second argument contains the URI (or non-URI filename)
** itself. When this function is called the *pFlags variable should contain
** the default flags to open the database handle with. The value stored in
** *pFlags may be updated before returning if the URI filename contains
** "cache=xxx" or "mode=xxx" query parameters.
**
** If successful, SQLITE_OK is returned. In this case *ppVfs is set to point to
** the VFS that should be used to open the database file. *pzFile is set to
** point to a buffer containing the name of the file to open.  The value
** stored in *pzFile is a database name acceptable to sqlite3_uri_parameter()
** and is in the same format as names created using sqlite3_create_filename().
** The caller must invoke sqlite3_free_filename() (not sqlite3_free()!) on
** the value returned in *pzFile to avoid a memory leak.
**
** If an error occurs, then an SQLite error code is returned and *pzErrMsg
** may be set to point to a buffer containing an English language error
** message. It is the responsibility of the caller to eventually release
** this buffer by calling sqlite3_free().
*/
صحیح sqlite3ParseUri(
  ثابت حرف *zDefaultVfs,        /* VFS to use if no "vfs=xxx" query option */
  ثابت حرف *zUri,               /* Nul-terminated URI to parse */
  غیرعلامتی صحیح *pFlags,           /* IN/OUT: SQLITE_OPEN_XXX flags */
  sqlite3_vfs **ppVfs,            /* OUT: VFS to use */
  حرف **pzFile,                  /* OUT: Filename component of URI */
  حرف **pz_Err_پیغام                 /* OUT: Error message (if rc!=SQLITE_OK) */
){
  صحیح rc = SQLITE_OK;
  غیرعلامتی صحیح flags = *pFlags;
  ثابت حرف *zVfs = zDefaultVfs;
  حرف *zFile;
  حرف c;
  i64 nUri = لڑی_لمبائی(zUri);

  أكد( *pz_Err_پیغام==0 );

  اگر( ((flags & SQLITE_OPEN_URI)                     /* IMP: R-48725-32206 */
       || Atomic_بھرو(&sqlite3GlobalConfig.bOpenUri)) /* IMP: R-51689-46548 */
   && nUri>=5 && memcmp(zUri, "file:", 5)==0         /* IMP: R-57884-37496 */
  ){
    حرف *zOpt;
    صحیح eState;                   /* Parser state when parsing URI */
    i64 i_اندر;                      /* Input character index */
    i64 i_برآمد = 0;                 /* Output character index */
    u64 nByte = nUri+8;           /* Bytes of space to allocate */

    /* Make sure the SQLITE_OPEN_URI flag is set to indicate to the VFS xOpen
    ** method that there may be extra parameters following the file-name.  */
    flags |= SQLITE_OPEN_URI;

    برائے(i_اندر=0; i_اندر<nUri; i_اندر++) nByte += (zUri[i_اندر]=='&');
    zFile = sqlite3_malloc64(nByte);
    اگر( !zFile ) واپس SQLITE_NOMEM_BKPT;

    memset(zFile, 0, 4);  /* 4-byte of 0x00 is the start of DB name marker */
    zFile += 4;

    i_اندر = 5;
#اگر_تعریف_شدہ SQLITE_ALLOW_URI_AUTHORITY
    اگر( لڑی_موازنہ(zUri+5, "///", 3)==0 ){
      i_اندر = 7;
      /* The following condition causes URIs with five leading / characters
      ** like file://///host/path to be converted into UNCs like //host/path.
      ** The correct URI for that UNC has only two or four leading / characters
      ** file://host/path or file:////host/path.  But 5 leading slashes is a
      ** common error, we are told, so we handle it as a special case. */
      اگر( لڑی_موازنہ(zUri+7, "///", 3)==0 ){ i_اندر++; }
    }ورنہ اگر( لڑی_موازنہ(zUri+5, "//localhost/", 12)==0 ){
      i_اندر = 16;
    }
#ورنہ
    /* Discard the scheme and authority segments of the URI. */
    اگر( zUri[5]=='/' && zUri[6]=='/' ){
      i_اندر = 7;
      جب( zUri[i_اندر] && zUri[i_اندر]!='/' ) i_اندر++;
      اگر( i_اندر!=7 && (i_اندر!=16 || memcmp("localhost", &zUri[7], 9)) ){
        *pz_Err_پیغام = sqlite3_mprintf("invalid uri authority: %.*s",
            (صحیح)(i_اندر-7), &zUri[7]);
        rc = SQLITE_ERROR;
        جاؤ parse_uri_برآمد;
      }
    }
#اگر_ختم

    /* Copy the filename and any query parameters into the zFile buffer.
    ** Decode %HH escape codes along the way.
    **
    ** Within this loop, variable eState may be set to 0, 1 or 2, depending
    ** on the parsing context. As follows:
    **
    **   0: Parsing file-name.
    **   1: Parsing name section of a name=value query parameter.
    **   2: Parsing value section of a name=value query parameter.
    */
    eState = 0;
    جب( (c = zUri[i_اندر])!=0 && c!='#' ){
      i_اندر++;
      اگر( c=='%'
       && sqlite3Isxdigit(zUri[i_اندر])
       && sqlite3Isxdigit(zUri[i_اندر+1])
      ){
        صحیح octet = (sqlite3_Hex_مقصد_Int(zUri[i_اندر++]) << 4);
        octet += sqlite3_Hex_مقصد_Int(zUri[i_اندر++]);

        أكد( octet>=0 && octet<256 );
        اگر( octet==0 ){
#اگر_غیر_تعریف_شدہ SQLITE_ENABLE_URI_00_ERROR
          /* This branch is taken when "%00" appears within the URI. In this
          ** case we ignore all text in the remainder of the path, name or
          ** value currently being parsed. So ignore the current character
          ** and skip to the next "?", "=" or "&", as appropriate. */
          جب( (c = zUri[i_اندر])!=0 && c!='#'
              && (eState!=0 || c!='?')
              && (eState!=1 || (c!='=' && c!='&'))
              && (eState!=2 || c!='&')
          ){
            i_اندر++;
          }
          جاری;
#ورنہ
          /* If ENABLE_URI_00_ERROR is defined, "%00" in a URI is an error. */
          *pz_Err_پیغام = sqlite3_mprintf("unexpected %%00 in uri");
          rc = SQLITE_ERROR;
          جاؤ parse_uri_برآمد;
#اگر_ختم
        }
        c = octet;
      }ورنہ اگر( eState==1 && (c=='&' || c=='=') ){
        اگر( zFile[i_برآمد-1]==0 ){
          /* An empty option name. Ignore this option altogether. */
          جب( zUri[i_اندر] && zUri[i_اندر]!='#' && zUri[i_اندر-1]!='&' ) i_اندر++;
          جاری;
        }
        اگر( c=='&' ){
          zFile[i_برآمد++] = '\0';
        }ورنہ{
          eState = 2;
        }
        c = 0;
      }ورنہ اگر( (eState==0 && c=='?') || (eState==2 && c=='&') ){
        c = 0;
        eState = 1;
      }
      zFile[i_برآمد++] = c;
    }
    اگر( eState==1 ) zFile[i_برآمد++] = '\0';
    memset(zFile+i_برآمد, 0, 4); /* end-of-options + empty journal filenames */

    /* Check if there were any options specified that should be interpreted
    ** here. Options that are interpreted here include "vfs" and those that
    ** correspond to flags that may be passed to the sqlite3_open_v2()
    ** method. */
    zOpt = &zFile[لڑی_لمبائی(zFile)+1];
    جب( zOpt[0] ){
      i64 nOpt = لڑی_لمبائی(zOpt);
      حرف *zVal = &zOpt[nOpt+1];
      i64 nVal = لڑی_لمبائی(zVal);

      اگر( nOpt==3 && memcmp("vfs", zOpt, 3)==0 ){
        zVfs = zVal;
      }ورنہ{
        ساخت Open_منوال {
          ثابت حرف *z;
          صحیح منوال;
        } *a_منوال = 0;
        حرف *z_منوال_Type = 0;
        صحیح نقاب = 0;
        صحیح حد_بندی = 0;

        اگر( nOpt==5 && memcmp("cache", zOpt, 5)==0 ){
          ساکن ساخت Open_منوال a_Cache_منوال[] = {
            { "shared",  SQLITE_OPEN_SHAREDCACHE },
            { "private", SQLITE_OPEN_PRIVATECACHE },
            { 0, 0 }
          };

          نقاب = SQLITE_OPEN_SHAREDCACHE|SQLITE_OPEN_PRIVATECACHE;
          a_منوال = a_Cache_منوال;
          حد_بندی = نقاب;
          z_منوال_Type = "cache";
        }
        اگر( nOpt==4 && memcmp("mode", zOpt, 4)==0 ){
          ساکن ساخت Open_منوال a_Open_منوال[] = {
            { "ro",  SQLITE_OPEN_READONLY },
            { "rw",  SQLITE_OPEN_READWRITE },
            { "rwc", SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE },
            { "memory", SQLITE_OPEN_حافظہ },
            { 0, 0 }
          };

          نقاب = SQLITE_OPEN_READONLY | SQLITE_OPEN_READWRITE
                   | SQLITE_OPEN_CREATE | SQLITE_OPEN_حافظہ;
          a_منوال = a_Open_منوال;
          حد_بندی = نقاب & flags;
          z_منوال_Type = "access";
        }

        اگر( a_منوال ){
          صحیح i;
          صحیح منوال = 0;
          برائے(i=0; a_منوال[i].z; i++){
            ثابت حرف *z = a_منوال[i].z;
            اگر( nVal==(i64)لڑی_لمبائی(z) && 0==memcmp(zVal, z, nVal) ){
              منوال = a_منوال[i].منوال;
              توڑو;
            }
          }
          اگر( منوال==0 ){
            *pz_Err_پیغام = sqlite3_mprintf("no such %s mode: %s", z_منوال_Type, zVal);
            rc = SQLITE_ERROR;
            جاؤ parse_uri_برآمد;
          }
          اگر( (منوال & ~SQLITE_OPEN_حافظہ)>حد_بندی ){
            *pz_Err_پیغام = sqlite3_mprintf("%s mode not allowed: %s",
                                        z_منوال_Type, zVal);
            rc = SQLITE_PERM;
            جاؤ parse_uri_برآمد;
          }
          flags = (flags & ~نقاب) | منوال;
        }
      }

      zOpt = &zVal[nVal+1];
    }

  }ورنہ{
    zFile = sqlite3_malloc64(nUri+8);
    اگر( !zFile ) واپس SQLITE_NOMEM_BKPT;
    memset(zFile, 0, 4);
    zFile += 4;
    اگر( nUri ){
      حافظہ_نقل(zFile, zUri, nUri);
    }
    memset(zFile+nUri, 0, 4);
    flags &= ~SQLITE_OPEN_URI;
  }

  *ppVfs = sqlite3_vfs_ڈھونڈو(zVfs);
  اگر( *ppVfs==0 ){
    *pz_Err_پیغام = sqlite3_mprintf("no such vfs: %s", zVfs);
    rc = SQLITE_ERROR;
  }
 parse_uri_برآمد:
  اگر( rc!=SQLITE_OK ){
    sqlite3_free_filename(zFile);
    zFile = 0;
  }
  *pFlags = flags;
  *pzFile = zFile;
  واپس rc;
}

/*
** This routine does the core work of extracting URI parameters from a
** database filename for the sqlite3_uri_parameter() interface.
*/
ساکن ثابت حرف *uriParameter(ثابت حرف *zFilename, ثابت حرف *zParam){
  zFilename += sqlite3Strlen30(zFilename) + 1;
  جب( ALWAYS(zFilename!=0) && zFilename[0] ){
    صحیح x = strcmp(zFilename, zParam);
    zFilename += sqlite3Strlen30(zFilename) + 1;
    اگر( x==0 ) واپس zFilename;
    zFilename += sqlite3Strlen30(zFilename) + 1;
  }
  واپس 0;
}



/*
** This routine does the work of opening a database on behalf of
** sqlite3_open() and sqlite3_open16(). The database filename "zFilename" 
** is UTF-8 encoded.
*/
ساکن صحیح openDatabase(
  ثابت حرف *zFilename, /* Database filename UTF-8 encoded */
  sqlite3 **ppDb,        /* OUT: Returned database handle */
  غیرعلامتی صحیح flags,    /* Operational flags */
  ثابت حرف *zVfs       /* Name of the VFS to use */
){
  sqlite3 *db;                    /* Store allocated handle here */
  صحیح rc;                         /* Return code */
  صحیح isThreadsafe;               /* True for threadsafe connections */
  حرف *zOpen = 0;                /* Filename argument to pass to BtreeOpen() */
  حرف *z_Err_پیغام = 0;              /* Error message from sqlite3ParseUri() */
  صحیح i;                          /* Loop counter */

#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( ppDb==0 ) واپس SQLITE_MISUSE_BKPT;
#اگر_ختم
  *ppDb = 0;
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOINIT
  rc = sqlite3_initialize();
  اگر( rc ) واپس rc;
#اگر_ختم

  اگر( sqlite3GlobalConfig.bCoreMutex==0 ){
    isThreadsafe = 0;
  }ورنہ اگر( flags & SQLITE_OPEN_NOMUTEX ){
    isThreadsafe = 0;
  }ورنہ اگر( flags & SQLITE_OPEN_FULLMUTEX ){
    isThreadsafe = 1;
  }ورنہ{
    isThreadsafe = sqlite3GlobalConfig.b_بھرا_Mutex;
  }

  اگر( flags & SQLITE_OPEN_PRIVATECACHE ){
    flags &= ~SQLITE_OPEN_SHAREDCACHE;
  }ورنہ اگر( sqlite3GlobalConfig.sharedCacheEnabled ){
    flags |= SQLITE_OPEN_SHAREDCACHE;
  }

  /* Remove harmful bits from the flags parameter
  **
  ** The SQLITE_OPEN_NOMUTEX and SQLITE_OPEN_FULLMUTEX flags were
  ** dealt with in the previous code block.  Besides these, the only
  ** valid input flags for sqlite3_open_v2() are SQLITE_OPEN_READONLY,
  ** SQLITE_OPEN_READWRITE, SQLITE_OPEN_CREATE, SQLITE_OPEN_SHAREDCACHE,
  ** SQLITE_OPEN_PRIVATECACHE, SQLITE_OPEN_EXRESCODE, and some reserved
  ** bits.  Silently mask off all other flags.
  */
  flags &=  ~( SQLITE_OPEN_DELETEONCLOSE |
               SQLITE_OPEN_EXCLUSIVE |
               SQLITE_OPEN_MAIN_DB |
               SQLITE_OPEN_عارضی_DB |
               SQLITE_OPEN_TRANSIENT_DB |
               SQLITE_OPEN_MAIN_JOURNAL |
               SQLITE_OPEN_عارضی_JOURNAL |
               SQLITE_OPEN_SUBJOURNAL |
               SQLITE_OPEN_SUPER_JOURNAL |
               SQLITE_OPEN_NOMUTEX |
               SQLITE_OPEN_FULLMUTEX |
               SQLITE_OPEN_WAL
             );

  /* Allocate the sqlite data structure */
  db = sqlite3MallocZero( حجم(sqlite3) );
  اگر( db==0 ) جاؤ opendb_برآمد;
  اگر( isThreadsafe
#اگر defined(SQLITE_THREAD_MISUSE_WARNINGS)
   || sqlite3GlobalConfig.bCoreMutex
#اگر_ختم
  ){
    db->mutex = sqlite3MutexAlloc(SQLITE_MUTEX_RECURSIVE);
    اگر( db->mutex==0 ){
      sqlite3_free(db);
      db = 0;
      جاؤ opendb_برآمد;
    }
    اگر( isThreadsafe==0 ){
      sqlite3MutexWarnOnContention(db->mutex);
    }
  }
  sqlite3_mutex_enter(db->mutex);
  db->err_نقاب = (flags & SQLITE_OPEN_EXRESCODE)!=0 ? 0xffffffff : 0xff;
  db->nDb = 2;
  db->eOpenState = SQLITE_STATE_BUSY;
  db->aDb = db->aDbStatic;
  db->lookaside.bDisable = 1;
  db->lookaside.sz = 0;
  db->nFpDigit = 17;

  أكد( حجم(db->a_حد_بندی)==حجم(a_Hard_حد_بندی) );
  حافظہ_نقل(db->a_حد_بندی, a_Hard_حد_بندی, حجم(db->a_حد_بندی));
  db->a_حد_بندی[SQLITE_حد_بندی_کارکن_دھاگے] = SQLITE_DEFAULT_کارکن_دھاگے;
  db->autoCommit = 1;
  db->اگلا_Autovac = -1;
  db->szMmap = sqlite3GlobalConfig.szMmap;
  db->اگلا_Pagesize = 0;
  db->init.azInit = sqlite3StdType; /* Any array of string ptrs will do */
#اگر_تعریف_شدہ SQLITE_ENABLE_SORTER_MMAP
  /* Beginning with version 3.37.0, using the VFS xFetch() API to memory-map
  ** the temporary files used to do external sorts (see code in vdbesort.c)
  ** is disabled. It can still be used either by defining
  ** SQLITE_ENABLE_SORTER_MMAP at compile time or by using the
  ** SQLITE_TESTCTRL_SORTER_MMAP test-control at runtime. */
  db->nMaxSorterMmap = 0x7FFFFFFF;
#اگر_ختم
  db->flags |= SQLITE_Short_ستون_نام
                 | SQLITE_EnableTrigger
                 | SQLITE_EnableView
                 | SQLITE_CacheSpill
                 | SQLITE_AttachCreate
                 | SQLITE_AttachWrite
                 | SQLITE_Comments
#اگر !defined(SQLITE_TRUSTED_SCHEMA) || SQLITE_TRUSTED_SCHEMA+0!=0
                 | SQLITE_TrustedSchema
#اگر_ختم
/* The SQLITE_DQS compile-time option determines the default settings
** for SQLITE_DBCONFIG_DQS_DDL and SQLITE_DBCONFIG_DQS_DML.
**
**    SQLITE_DQS     SQLITE_DBCONFIG_DQS_DDL    SQLITE_DBCONFIG_DQS_DML
**    ----------     -----------------------    -----------------------
**     undefined               on                          on
**         3                   on                          on
**         2                   on                         off
**         1                  off                          on
**         0                  off                         off
**
** Legacy behavior is 3 (double-quoted string literals are allowed anywhere)
** and so that is the default.  But developers are encouraged to use
** -DSQLITE_DQS=0 (best) or -DSQLITE_DQS=1 (second choice) if possible.
*/
#اگر !defined(SQLITE_DQS)
# define SQLITE_DQS 3
#اگر_ختم
#اگر (SQLITE_DQS&1)==1
                 | SQLITE_DqsDML
#اگر_ختم
#اگر (SQLITE_DQS&2)==2
                 | SQLITE_DqsDDL
#اگر_ختم

#اگر !defined(SQLITE_DEFAULT_AUTOMATIC_اشاریہ) || SQLITE_DEFAULT_AUTOMATIC_اشاریہ
                 | SQLITE_Auto_اشاریہ
#اگر_ختم
#اگر SQLITE_DEFAULT_CKPTFULLFSYNC
                 | SQLITE_Ckpt_بھرا_FSync
#اگر_ختم
#اگر SQLITE_DEFAULT_FILE_FORMAT<4
                 | SQLITE_LegacyFileFmt
#اگر_ختم
#اگر_تعریف_شدہ SQLITE_ENABLE_بھرو_EXTENSION
                 | SQLITE_بھرو_Extension
#اگر_ختم
#اگر SQLITE_DEFAULT_RECURSIVE_TRIGGERS
                 | SQLITE_RecTriggers
#اگر_ختم
#اگر defined(SQLITE_DEFAULT_FOREIGN_کلیدیں) && SQLITE_DEFAULT_FOREIGN_کلیدیں
                 | SQLITE_Foreign_کلیدیں
#اگر_ختم
#اگر defined(SQLITE_الٹ_UNORDERED_SELECTS)
                 | SQLITE_الٹ_ترتیب
#اگر_ختم
#اگر defined(SQLITE_ENABLE_OVERSIZE_CELL_CHECK)
                 | SQLITE_Cell_سائز_Ck
#اگر_ختم
#اگر defined(SQLITE_ENABLE_FTS3_TOKENIZER)
                 | SQLITE_Fts3Tokenizer
#اگر_ختم
#اگر defined(SQLITE_ENABLE_QPSG)
                 | SQLITE_EnableQPSG
#اگر_ختم
#اگر defined(SQLITE_DEFAULT_DEFENSIVE)
                 | SQLITE_Defensive
#اگر_ختم
#اگر defined(SQLITE_DEFAULT_LEGACY_ALTER_جدول)
                 | SQLITE_LegacyAlter
#اگر_ختم
#اگر defined(SQLITE_ENABLE_STMT_SCANSTATUS)
                 | SQLITE_Stmt_جائزہ_حالت
#اگر_ختم
      ;
  sqlite3HashInit(&db->aCollSeq);
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_VIRTUALTABLE
  sqlite3HashInit(&db->aModule);
#اگر_ختم

  /* Add the default collation sequence BINARY. BINARY works for both UTF-8
  ** and UTF-16, so add a version for each to avoid any unnecessary
  ** conversions. The only error that can occur here is a malloc() failure.
  **
  ** EVIDENCE-OF: R-52786-44878 SQLite defines three built-in collating
  ** functions:
  */
  createCollation(db, sqlite3_Str_ثنائی, SQLITE_UTF8, 0,
                  sqlite3_ثنائی_Compare, 0);
  createCollation(db, sqlite3_Str_ثنائی, SQLITE_UTF16BE, 0,
                  sqlite3_ثنائی_Compare, 0);
  createCollation(db, sqlite3_Str_ثنائی, SQLITE_UTF16LE, 0,
                  sqlite3_ثنائی_Compare, 0);
  createCollation(db, "NOCASE", SQLITE_UTF8, 0, nocaseCollatingFunc, 0);
  createCollation(db, "RTRIM", SQLITE_UTF8, 0, rtrimCollFunc, 0);
  اگر( db->mallocFailed ){
    جاؤ opendb_برآمد;
  }

#اگر SQLITE_OS_UNIX && defined(SQLITE_OS_KV_OPTIONAL)
  /* Process magic filenames ":localStorage:" and ":sessionStorage:" */
  اگر( zFilename && zFilename[0]==':' ){
    اگر( strcmp(zFilename, ":localStorage:")==0 ){
      zFilename = "file:local?vfs=kvvfs";
      flags |= SQLITE_OPEN_URI;
    }ورنہ اگر( strcmp(zFilename, ":sessionStorage:")==0 ){
      zFilename = "file:session?vfs=kvvfs";
      flags |= SQLITE_OPEN_URI;
    }
  }
#اگر_ختم /* SQLITE_OS_UNIX && defined(SQLITE_OS_KV_OPTIONAL) */

  /* Parse the filename/URI argument
  **
  ** Only allow sensible combinations of bits in the flags argument. 
  ** Throw an error if any non-sense combination is used.  If we
  ** do not block illegal combinations here, it could trigger
  ** assert() statements in deeper layers.  Sensible combinations
  ** are:
  **
  **  1:  SQLITE_OPEN_READONLY
  **  2:  SQLITE_OPEN_READWRITE
  **  6:  SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE
  */
  db->openFlags = flags;
  أكد( SQLITE_OPEN_READONLY  == 0x01 );
  أكد( SQLITE_OPEN_READWRITE == 0x02 );
  أكد( SQLITE_OPEN_CREATE    == 0x04 );
  testcase( (1<<(flags&7))==0x02 ); /* READONLY */
  testcase( (1<<(flags&7))==0x04 ); /* READWRITE */
  testcase( (1<<(flags&7))==0x40 ); /* READWRITE | CREATE */
  اگر( ((1<<(flags&7)) & 0x46)==0 ){
    rc = SQLITE_MISUSE_BKPT;  /* IMP: R-18321-05872 */
  }ورنہ{
    اگر( zFilename==0 ) zFilename = ":memory:";
    rc = sqlite3ParseUri(zVfs, zFilename, &flags, &db->pVfs, &zOpen, &z_Err_پیغام);
  }
  اگر( rc!=SQLITE_OK ){
    اگر( rc==SQLITE_NOMEM ) sqlite3OomFault(db);
    sqlite3_Error_With_پیغام(db, rc, z_Err_پیغام ? "%s" : 0, z_Err_پیغام);
    sqlite3_free(z_Err_پیغام);
    جاؤ opendb_برآمد;
  }
  أكد( db->pVfs!=0 );
#اگر SQLITE_OS_KV || defined(SQLITE_OS_KV_OPTIONAL)
  اگر( sqlite3_stricmp(db->pVfs->zName, "kvvfs")==0 ){
    db->عارضی_store = 2;
  }
#اگر_ختم

  /* Open the backend database driver */
  rc = sqlite3BtreeOpen(db->pVfs, zOpen, db, &db->aDb[0].pBt, 0,
                        flags | SQLITE_OPEN_MAIN_DB);
  اگر( rc!=SQLITE_OK ){
    اگر( rc==SQLITE_IOERR_NOMEM ){
      rc = SQLITE_NOMEM_BKPT;
    }
    sqlite3Error(db, rc);
    جاؤ opendb_برآمد;
  }
  sqlite3BtreeEnter(db->aDb[0].pBt);
  db->aDb[0].pSchema = sqlite3SchemaGet(db, db->aDb[0].pBt);
  اگر( !db->mallocFailed ){
    sqlite3_سیٹ_متن_Encoding(db, SCHEMA_ENC(db));
  }
  sqlite3BtreeLeave(db->aDb[0].pBt);
  db->aDb[1].pSchema = sqlite3SchemaGet(db, 0);

  /* The default safety_level for the main database is FULL; for the temp
  ** database it is OFF. This matches the pager layer defaults. 
  */
  db->aDb[0].zDbSName = "main";
  db->aDb[0].safety_level = SQLITE_DEFAULT_SYNCHRONOUS+1;
  db->aDb[1].zDbSName = "temp";
  db->aDb[1].safety_level = PAGER_SYNCHRONOUS_OFF;

  db->eOpenState = SQLITE_STATE_OPEN;
  اگر( db->mallocFailed ){
    جاؤ opendb_برآمد;
  }

  /* Register all built-in functions, but do not attempt to read the
  ** database schema yet. This is delayed until the first time the database
  ** is accessed.
  */
  sqlite3Error(db, SQLITE_OK);
  sqlite3_رجسٹر_فی_Connection_Builtin_Functions(db);
  rc = sqlite3_errcode(db);


  /* Load compiled-in extensions */
  برائے(i=0; rc==SQLITE_OK && i<Array_سائز(sqlite3BuiltinExtensions); i++){
    rc = sqlite3BuiltinExtensions[i](db);
  }

  /* Load automatic extensions - extensions that have been registered
  ** using the sqlite3_automatic_extension() API.
  */
  اگر( rc==SQLITE_OK ){
    sqlite3_Auto_بھرو_Extensions(db);
    rc = sqlite3_errcode(db);
    اگر( rc!=SQLITE_OK ){
      جاؤ opendb_برآمد;
    }
  }

#اگر_تعریف_شدہ SQLITE_ENABLE_اندرونی_FUNCTIONS
  /* Testing use only!!! The -DSQLITE_ENABLE_INTERNAL_FUNCTIONS=1 compile-time
  ** option gives access to internal functions by default. 
  ** Testing use only!!! */
  db->mDbFlags |= DBFLAG_اندرونی_Func;
#اگر_ختم

  /* -DSQLITE_DEFAULT_LOCKING_MODE=1 makes EXCLUSIVE the default locking
  ** mode.  -DSQLITE_DEFAULT_LOCKING_MODE=0 make NORMAL the default locking
  ** mode.  Doing nothing at all also makes NORMAL the default.
  */
#اگر_تعریف_شدہ SQLITE_DEFAULT_LOCKING_منوال
  db->dflt_تالا_منوال = SQLITE_DEFAULT_LOCKING_منوال;
  sqlite3_Pager_Locking_منوال(sqlite3BtreePager(db->aDb[0].pBt),
                          SQLITE_DEFAULT_LOCKING_منوال);
#اگر_ختم

  اگر( rc ) sqlite3Error(db, rc);

  /* Enable the lookaside-malloc subsystem */
  تیاری_Lookaside(db, 0, sqlite3GlobalConfig.szLookaside,
                        sqlite3GlobalConfig.nLookaside);

  sqlite3_wal_autocheckpoint(db, SQLITE_DEFAULT_WAL_AUTOCHECKPOINT);

opendb_برآمد:
  اگر( db ){
    أكد( db->mutex!=0 || isThreadsafe==0
           || sqlite3GlobalConfig.b_بھرا_Mutex==0 );
    sqlite3_mutex_leave(db->mutex);
  }
  rc = sqlite3_errcode(db);
  أكد( db!=0 || (rc&0xff)==SQLITE_NOMEM );
  اگر( (rc&0xff)==SQLITE_NOMEM ){
    sqlite3_close(db);
    db = 0;
  }ورنہ اگر( rc!=SQLITE_OK ){
    db->eOpenState = SQLITE_STATE_SICK;
  }
  *ppDb = db;
#اگر_تعریف_شدہ SQLITE_ENABLE_SQLLOG
  اگر( sqlite3GlobalConfig.xSqllog ){
    /* Opening a db handle. Fourth parameter is passed 0. */
    خالی *pArg = sqlite3GlobalConfig.pSqllogArg;
    sqlite3GlobalConfig.xSqllog(pArg, db, zFilename, 0);
  }
#اگر_ختم
  sqlite3_free_filename(zOpen);
  واپس rc;
}


/*
** Open a new database handle.
*/
صحیح sqlite3_open(
  ثابت حرف *zFilename,
  sqlite3 **ppDb
){
  واپس openDatabase(zFilename, ppDb,
                      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, 0);
}
صحیح sqlite3_open_v2(
  ثابت حرف *filename,   /* Database filename (UTF-8) */
  sqlite3 **ppDb,         /* OUT: SQLite db handle */
  صحیح flags,              /* Flags */
  ثابت حرف *zVfs        /* Name of VFS module to use */
){
  واپس openDatabase(filename, ppDb, (غیرعلامتی صحیح)flags, zVfs);
}

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_UTF16
/*
** Open a new database handle.
*/
صحیح sqlite3_open16(
  ثابت خالی *zFilename,
  sqlite3 **ppDb
){
  حرف ثابت *zFilename8;   /* zFilename encoded in UTF-8 instead of UTF-16 */
  sqlite3_قدر *pVal;
  صحیح rc;

#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( ppDb==0 ) واپس SQLITE_MISUSE_BKPT;
#اگر_ختم
  *ppDb = 0;
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOINIT
  rc = sqlite3_initialize();
  اگر( rc ) واپس rc;
#اگر_ختم
  اگر( zFilename==0 ) zFilename = "\000\000";
  pVal = sqlite3_قدر_New(0);
  sqlite3_قدر_سیٹ_Str(pVal, -1, zFilename, SQLITE_UTF16NATIVE, SQLITE_STATIC);
  zFilename8 = sqlite3_قدر_متن_2(pVal, SQLITE_UTF8);
  اگر( zFilename8 ){
    rc = openDatabase(zFilename8, ppDb,
                      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, 0);
    أكد( *ppDb || rc==SQLITE_NOMEM );
    اگر( rc==SQLITE_OK && !DbHasProperty(*ppDb, 0, DB_SchemaLoaded) ){
      SCHEMA_ENC(*ppDb) = ENC(*ppDb) = SQLITE_UTF16NATIVE;
    }
  }ورنہ{
    rc = SQLITE_NOMEM_BKPT;
  }
  sqlite3_قدر_Free(pVal);

  واپس rc & 0xff;
}
#اگر_ختم /* SQLITE_OMIT_UTF16 */

/*
** Register a new collation sequence with the database handle db.
*/
صحیح sqlite3_create_collation(
  sqlite3* db,
  ثابت حرف *zName,
  صحیح enc,
  خالی* pCtx,
  صحیح(*xCompare)(خالی*,صحیح,ثابت خالی*,صحیح,ثابت خالی*)
){
  واپس sqlite3_create_collation_v2(db, zName, enc, pCtx, xCompare, 0);
}

/*
** Register a new collation sequence with the database handle db.
*/
صحیح sqlite3_create_collation_v2(
  sqlite3* db,
  ثابت حرف *zName,
  صحیح enc,
  خالی* pCtx,
  صحیح(*xCompare)(خالی*,صحیح,ثابت خالی*,صحیح,ثابت خالی*),
  خالی(*xDel)(خالی*)
){
  صحیح rc;

#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db) || zName==0 ) واپس SQLITE_MISUSE_BKPT;
#اگر_ختم
  sqlite3_mutex_enter(db->mutex);
  أكد( !db->mallocFailed );
  rc = createCollation(db, zName, (u8)enc, pCtx, xCompare, xDel);
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  واپس rc;
}

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_UTF16
/*
** Register a new collation sequence with the database handle db.
*/
صحیح sqlite3_create_collation16(
  sqlite3* db,
  ثابت خالی *zName,
  صحیح enc,
  خالی* pCtx,
  صحیح(*xCompare)(خالی*,صحیح,ثابت خالی*,صحیح,ثابت خالی*)
){
  صحیح rc = SQLITE_OK;
  حرف *zName8;

#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db) || zName==0 ) واپس SQLITE_MISUSE_BKPT;
#اگر_ختم
  sqlite3_mutex_enter(db->mutex);
  أكد( !db->mallocFailed );
  zName8 = sqlite3Utf16to8(db, zName, -1, SQLITE_UTF16NATIVE);
  اگر( zName8 ){
    rc = createCollation(db, zName8, (u8)enc, pCtx, xCompare, 0);
    sqlite3DbFree(db, zName8);
  }
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  واپس rc;
}
#اگر_ختم /* SQLITE_OMIT_UTF16 */

/*
** Register a collation sequence factory callback with the database handle
** db. Replace any previously installed collation sequence factory.
*/
صحیح sqlite3_collation_needed(
  sqlite3 *db,
  خالی *pCollNeededArg,
  خالی(*xCollNeeded)(خالی*,sqlite3*,صحیح e_متن_Rep,ثابت حرف*)
){
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db) ) واپس SQLITE_MISUSE_BKPT;
#اگر_ختم
  sqlite3_mutex_enter(db->mutex);
  db->xCollNeeded = xCollNeeded;
  db->xCollNeeded16 = 0;
  db->pCollNeededArg = pCollNeededArg;
  sqlite3_mutex_leave(db->mutex);
  واپس SQLITE_OK;
}

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_UTF16
/*
** Register a collation sequence factory callback with the database handle
** db. Replace any previously installed collation sequence factory.
*/
صحیح sqlite3_collation_needed16(
  sqlite3 *db,
  خالی *pCollNeededArg,
  خالی(*xCollNeeded16)(خالی*,sqlite3*,صحیح e_متن_Rep,ثابت خالی*)
){
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db) ) واپس SQLITE_MISUSE_BKPT;
#اگر_ختم
  sqlite3_mutex_enter(db->mutex);
  db->xCollNeeded = 0;
  db->xCollNeeded16 = xCollNeeded16;
  db->pCollNeededArg = pCollNeededArg;
  sqlite3_mutex_leave(db->mutex);
  واپس SQLITE_OK;
}
#اگر_ختم /* SQLITE_OMIT_UTF16 */

/*
** Find existing client data.
*/
خالی *sqlite3_get_clientdata(sqlite3 *db, ثابت حرف *zName){
  Db_Client_ڈیٹا *p;
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !zName || !sqlite3SafetyCheckOk(db) ){
    (خالی)SQLITE_MISUSE_BKPT;
    واپس 0;
  }
#اگر_ختم
  sqlite3_mutex_enter(db->mutex);
  برائے(p=db->p_Db_ڈیٹا; p; p=p->p_اگلا){
    اگر( strcmp(p->zName, zName)==0 ){
      خالی *p_نتیجہ = p->p_ڈیٹا;
      sqlite3_mutex_leave(db->mutex);
      واپس p_نتیجہ;
    }
  }
  sqlite3_mutex_leave(db->mutex);
  واپس 0;
}

/*
** Add new client data to a database connection.
*/
صحیح sqlite3_سیٹ_clientdata(
  sqlite3 *db,                   /* Attach client data to this connection */
  ثابت حرف *zName,             /* Name of the client data */
  خالی *p_ڈیٹا,                   /* The client data itself */
  خالی (*xDestructor)(خالی*)     /* Destructor */
){
  Db_Client_ڈیٹا *p, **pp;
  sqlite3_mutex_enter(db->mutex);
  pp = &db->p_Db_ڈیٹا;
  برائے(p=db->p_Db_ڈیٹا; p && strcmp(p->zName,zName); p=p->p_اگلا){
    pp = &p->p_اگلا;
  }
  اگر( p ){
    أكد( p->p_ڈیٹا!=0 );
    اگر( p->xDestructor ) p->xDestructor(p->p_ڈیٹا);
    اگر( p_ڈیٹا==0 ){
      *pp = p->p_اگلا;
      sqlite3_free(p);
      sqlite3_mutex_leave(db->mutex);
      واپس SQLITE_OK;
    }
  }ورنہ اگر( p_ڈیٹا==0 ){
    sqlite3_mutex_leave(db->mutex);
    واپس SQLITE_OK;
  }ورنہ{
    size_t n = لڑی_لمبائی(zName);
    p = sqlite3_malloc64( SZ_DBCLIENTDATA(n+1) );
    اگر( p==0 ){
      اگر( xDestructor ) xDestructor(p_ڈیٹا);
      sqlite3_mutex_leave(db->mutex);
      واپس SQLITE_NOMEM;
    }
    حافظہ_نقل(p->zName, zName, n+1);
    p->p_اگلا = db->p_Db_ڈیٹا;
    db->p_Db_ڈیٹا = p;
  }
  p->p_ڈیٹا = p_ڈیٹا;
  p->xDestructor = xDestructor;
  sqlite3_mutex_leave(db->mutex);
  واپس SQLITE_OK;
}


#اگر_غیر_تعریف_شدہ SQLITE_OMIT_DEPRECATED
/*
** This function is now an anachronism. It used to be used to recover from a
** malloc() failure, but SQLite now does this automatically.
*/
صحیح sqlite3_global_recover(خالی){
  واپس SQLITE_OK;
}
#اگر_ختم

/*
** Test to see whether or not the database connection is in autocommit
** mode.  Return TRUE if it is and FALSE if not.  Autocommit mode is on
** by default.  Autocommit is disabled by a BEGIN statement and reenabled
** by the next COMMIT or ROLLBACK.
*/
صحیح sqlite3_get_autocommit(sqlite3 *db){
  صحیح iRet;
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db) ){
    (خالی)SQLITE_MISUSE_BKPT;
    واپس 0;
  }
#اگر_ختم
  sqlite3_mutex_enter(db->mutex);
  iRet = db->autoCommit;
  sqlite3_mutex_leave(db->mutex);
  واپس iRet;
}

/*
** The following routines are substitutes for constants SQLITE_CORRUPT,
** SQLITE_MISUSE, SQLITE_CANTOPEN, SQLITE_NOMEM and possibly other error
** constants.  They serve two purposes:
**
**   1.  Serve as a convenient place to set a breakpoint in a debugger
**       to detect when version error conditions occurs.
**
**   2.  Invoke sqlite3_log() to provide the source code location where
**       a low-level error is first detected.
*/
صحیح sqlite3ReportError(صحیح iErr, صحیح lineno, ثابت حرف *zType){
  sqlite3_log(iErr, "%s at line %d of [%.10s]",
              zType, lineno, 20+sqlite3_sourceid());
  واپس iErr;
}
صحیح sqlite3CorruptError(صحیح lineno){
  testcase( sqlite3GlobalConfig.xLog!=0 );
  واپس sqlite3ReportError(SQLITE_CORRUPT, lineno, "database corruption");
}
صحیح sqlite3MisuseError(صحیح lineno){
  testcase( sqlite3GlobalConfig.xLog!=0 );
  واپس sqlite3ReportError(SQLITE_MISUSE, lineno, "misuse");
}
صحیح sqlite3CantopenError(صحیح lineno){
  testcase( sqlite3GlobalConfig.xLog!=0 );
  واپس sqlite3ReportError(SQLITE_CANTOPEN, lineno, "cannot open file");
}
#اگر defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_CORRUPT_PGNO)
صحیح sqlite3CorruptPgnoError(صحیح lineno, Pgno pgno){
  حرف z_پیغام[100];
  sqlite3_snprintf(حجم(z_پیغام), z_پیغام, "database corruption page %d", pgno);
  testcase( sqlite3GlobalConfig.xLog!=0 );
  واپس sqlite3ReportError(SQLITE_CORRUPT, lineno, z_پیغام);
}
#اگر_ختم
#اگر_تعریف_شدہ SQLITE_DEBUG
صحیح sqlite3NomemError(صحیح lineno){
  testcase( sqlite3GlobalConfig.xLog!=0 );
  واپس sqlite3ReportError(SQLITE_NOMEM, lineno, "OOM");
}
صحیح sqlite3IoerrnomemError(صحیح lineno){
  testcase( sqlite3GlobalConfig.xLog!=0 );
  واپس sqlite3ReportError(SQLITE_IOERR_NOMEM, lineno, "I/O OOM error");
}
#اگر_ختم

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_DEPRECATED
/*
** This is a convenience routine that makes sure that all thread-specific
** data for this thread has been deallocated.
**
** SQLite no longer uses thread-specific data so this routine is now a
** no-op.  It is retained for historical compatibility.
*/
خالی sqlite3_thread_cleanup(خالی){
}
#اگر_ختم

/*
** Return meta information about a specific column of a database table.
** See comment in sqlite3.h (sqlite.h.in) for details.
*/
صحیح sqlite3_جدول_عمود_metadata(
  sqlite3 *db,                /* Connection handle */
  ثابت حرف *zDbName,        /* Database name or NULL */
  ثابت حرف *z_جدول_Name,     /* Table name */
  ثابت حرف *z_عمود_Name,    /* Column name */
  حرف ثابت **pz_ڈیٹا_Type,    /* OUTPUT: Declared data type */
  حرف ثابت **pzCollSeq,     /* OUTPUT: Collation sequence name */
  صحیح *pNotNull,              /* OUTPUT: True if NOT NULL constraint exists */
  صحیح *p_Primary_کلید,           /* OUTPUT: True if column part of PK */
  صحیح *pAutoinc               /* OUTPUT: True if column is auto-increment */
){
  صحیح rc;
  حرف *z_Err_پیغام = 0;
  جدول *pTab = 0;
  عمود *p_ستون = 0;
  صحیح i_ستون = 0;
  حرف ثابت *z_ڈیٹا_Type = 0;
  حرف ثابت *zCollSeq = 0;
  صحیح notnull = 0;
  صحیح primarykey = 0;
  صحیح autoinc = 0;


#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db) || z_جدول_Name==0 ){
    واپس SQLITE_MISUSE_BKPT;
  }
#اگر_ختم

  /* Ensure the database schema has been loaded */
  sqlite3_mutex_enter(db->mutex);
  sqlite3BtreeEnterAll(db);
  rc = sqlite3Init(db, &z_Err_پیغام);
  اگر( SQLITE_OK!=rc ){
    جاؤ error_برآمد;
  }

  /* Locate the table in question */
  pTab = sqlite3_ڈھونڈو_جدول(db, z_جدول_Name, zDbName);
  اگر( !pTab || IsView(pTab) ){
    pTab = 0;
    جاؤ error_برآمد;
  }

  /* Find the column for which info is requested */
  اگر( z_عمود_Name==0 ){
    /* Query for existence of table only */
  }ورنہ{
    i_ستون = sqlite3_عمود_اشاریہ(pTab, z_عمود_Name);
    اگر( i_ستون>=0 ){
      p_ستون = &pTab->a_ستون[i_ستون];
    }ورنہ{
      اگر( HasRowid(pTab) && sqlite3IsRowid(z_عمود_Name) ){
        i_ستون = pTab->iPKey;
        p_ستون = i_ستون>=0 ? &pTab->a_ستون[i_ستون] : 0;
      }ورنہ{
        pTab = 0;
        جاؤ error_برآمد;
      }
    }
  }

  /* The following block stores the meta information that will be returned
  ** to the caller in local variables zDataType, zCollSeq, notnull, primarykey
  ** and autoinc. At this point there are two possibilities:
  **
  **     1. The specified column name was rowid", "oid" or "_rowid_"
  **        and there is no explicitly declared IPK column.
  **
  **     2. The table is not a view and the column name identified an
  **        explicitly declared column. Copy meta information from *pCol.
  */
  اگر( p_ستون ){
    z_ڈیٹا_Type = sqlite3_عمود_Type(p_ستون,0);
    zCollSeq = sqlite3_عمود_Coll(p_ستون);
    notnull = p_ستون->notNull!=0;
    primarykey  = (p_ستون->ستون_Flags & COLFLAG_PRIMKEY)!=0;
    autoinc = pTab->iPKey==i_ستون && (pTab->tabFlags & TF_Autoincrement)!=0;
  }ورنہ{
    z_ڈیٹا_Type = "INTEGER";
    primarykey = 1;
  }
  اگر( !zCollSeq ){
    zCollSeq = sqlite3_Str_ثنائی;
  }

error_برآمد:
  sqlite3BtreeLeaveAll(db);

  /* Whether the function call succeeded or failed, set the output parameters
  ** to whatever their local counterparts contain. If an error did occur,
  ** this has the effect of zeroing all output parameters.
  */
  اگر( pz_ڈیٹا_Type ) *pz_ڈیٹا_Type = z_ڈیٹا_Type;
  اگر( pzCollSeq ) *pzCollSeq = zCollSeq;
  اگر( pNotNull ) *pNotNull = notnull;
  اگر( p_Primary_کلید ) *p_Primary_کلید = primarykey;
  اگر( pAutoinc ) *pAutoinc = autoinc;

  اگر( SQLITE_OK==rc && !pTab ){
    sqlite3DbFree(db, z_Err_پیغام);
    z_Err_پیغام = sqlite3MPrintf(db, "no such table column: %s.%s", z_جدول_Name,
        z_عمود_Name);
    rc = SQLITE_ERROR;
  }
  sqlite3_Error_With_پیغام(db, rc, (z_Err_پیغام?"%s":0), z_Err_پیغام);
  sqlite3DbFree(db, z_Err_پیغام);
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  واپس rc;
}

/*
** Sleep for a little while.  Return the amount of time slept.
*/
صحیح sqlite3_sleep(صحیح ms){
  sqlite3_vfs *pVfs;
  صحیح rc;
  pVfs = sqlite3_vfs_ڈھونڈو(0);
  اگر( pVfs==0 ) واپس 0;

  /* This function works in milliseconds, but the underlying OsSleep()
  ** API uses microseconds. Hence the 1000's.
  */
  rc = (sqlite3OsSleep(pVfs, ms<0 ? 0 : 1000*ms)/1000);
  واپس rc;
}

/*
** Enable or disable the extended result codes.
*/
صحیح sqlite3_extended_نتیجہ_codes(sqlite3 *db, صحیح onoff){
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db) ) واپس SQLITE_MISUSE_BKPT;
#اگر_ختم
  sqlite3_mutex_enter(db->mutex);
  db->err_نقاب = onoff ? 0xffffffff : 0xff;
  sqlite3_mutex_leave(db->mutex);
  واپس SQLITE_OK;
}

/*
** Invoke the xFileControl method on a particular database.
*/
صحیح sqlite3_file_control(sqlite3 *db, ثابت حرف *zDbName, صحیح op, خالی *pArg){
  صحیح rc = SQLITE_ERROR;
  Btree *pBtree;

#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db) ) واپس SQLITE_MISUSE_BKPT;
#اگر_ختم
  sqlite3_mutex_enter(db->mutex);
  pBtree = sqlite3_Db_Name_مقصد_Btree(db, zDbName);
  اگر( pBtree ){
    Pager *pPager;
    sqlite3_file *fd;
    sqlite3BtreeEnter(pBtree);
    pPager = sqlite3BtreePager(pBtree);
    أكد( pPager!=0 );
    fd = sqlite3PagerFile(pPager);
    أكد( fd!=0 );
    اگر( op==SQLITE_FCNTL_FILE_اشارہ_گر ){
      *(sqlite3_file**)pArg = fd;
      rc = SQLITE_OK;
    }ورنہ اگر( op==SQLITE_FCNTL_VFS_اشارہ_گر ){
      *(sqlite3_vfs**)pArg = sqlite3PagerVfs(pPager);
      rc = SQLITE_OK;
    }ورنہ اگر( op==SQLITE_FCNTL_JOURNAL_اشارہ_گر ){
      *(sqlite3_file**)pArg = sqlite3PagerJrnlFile(pPager);
      rc = SQLITE_OK;
    }ورنہ اگر( op==SQLITE_FCNTL_ڈیٹا_نسخہ ){
      *(غیرعلامتی صحیح*)pArg = sqlite3_Pager_ڈیٹا_نسخہ(pPager);
      rc = SQLITE_OK;
    }ورنہ اگر( op==SQLITE_FCNTL_RESERVE_بائٹیں ){
      صحیح iNew = *(صحیح*)pArg;
      *(صحیح*)pArg = sqlite3BtreeGetRequestedReserve(pBtree);
      اگر( iNew>=0 && iNew<=255 ){
        sqlite3_Btree_سیٹ_صفحہ_سائز(pBtree, 0, iNew, 0);
      }
      rc = SQLITE_OK;
    }ورنہ اگر( op==SQLITE_FCNTL_RESET_CACHE ){
      sqlite3BtreeClearCache(pBtree);
      rc = SQLITE_OK;
    }ورنہ{
      صحیح nSave = db->busyHandler.nBusy;
      rc = sqlite3OsFileControl(fd, op, pArg);
      db->busyHandler.nBusy = nSave;
    }
    sqlite3BtreeLeave(pBtree);
  }
  sqlite3_mutex_leave(db->mutex);
  واپس rc;
}

/*
** Interface to the testing logic.
*/
صحیح sqlite3_test_control(صحیح op, ...){
  صحیح rc = 0;
#اگر_تعریف_شدہ SQLITE_UNTESTABLE
  غیر_مستعمل_PARAMETER(op);
#ورنہ
  va_list ap;
  va_start(ap, op);
  انتخاب( op ){

    /*
    ** Save the current state of the PRNG.
    */
    حالت SQLITE_TESTCTRL_PRNG_SAVE: {
      sqlite3PrngSaveState();
      توڑو;
    }

    /*
    ** Restore the state of the PRNG to the last state saved using
    ** PRNG_SAVE.  If PRNG_SAVE has never before been called, then
    ** this verb acts like PRNG_RESET.
    */
    حالت SQLITE_TESTCTRL_PRNG_RESTORE: {
      sqlite3PrngRestoreState();
      توڑو;
    }

    /*  sqlite3_test_control(SQLITE_TESTCTRL_PRNG_SEED, int x, sqlite3 *db);
    **
    ** Control the seed for the pseudo-random number generator (PRNG) that
    ** is built into SQLite.  Cases:
    **
    **    x!=0 && db!=0       Seed the PRNG to the current value of the
    **                        schema cookie in the main database for db, or
    **                        x if the schema cookie is zero.  This case
    **                        is convenient to use with database fuzzers
    **                        as it allows the fuzzer some control over the
    **                        the PRNG seed.
    **
    **    x!=0 && db==0       Seed the PRNG to the value of x.
    **
    **    x==0 && db==0       Revert to default behavior of using the
    **                        xRandomness method on the primary VFS.
    **
    ** This test-control also resets the PRNG so that the new seed will
    ** be used for the next call to sqlite3_randomness().
    */
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WSD
    حالت SQLITE_TESTCTRL_PRNG_بیج: {
      صحیح x = va_arg(ap, صحیح);
      صحیح y;
      sqlite3 *db = va_arg(ap, sqlite3*);
      أكد( db==0 || db->aDb[0].pSchema!=0 );
      اگر( db && (y = db->aDb[0].pSchema->schema_cookie)!=0 ){ x = y; }
      sqlite3Config.i_Prng_بیج = x;
      sqlite3_randomness(0,0);
      توڑو;
    }
#اگر_ختم

    /*  sqlite3_test_control(SQLITE_TESTCTRL_FK_NO_ACTION, sqlite3 *db, int b);
    **
    ** If b is true, then activate the SQLITE_FkNoAction setting.  If b is
    ** false then clear that setting.  If the SQLITE_FkNoAction setting is
    ** enabled, all foreign key ON DELETE and ON UPDATE actions behave as if
    ** they were NO ACTION, regardless of how they are defined.
    **
    ** NB:  One must usually run "PRAGMA writable_schema=RESET" after
    ** using this test-control, before it will take full effect.  failing
    ** to reset the schema can result in some unexpected behavior.
    */
    حالت SQLITE_TESTCTRL_FK_NO_ACTION: {
      sqlite3 *db = va_arg(ap, sqlite3*);
      صحیح b = va_arg(ap, صحیح);
      اگر( b ){
        db->flags |= SQLITE_FkNoAction;
      }ورنہ{
        db->flags &= ~SQLITE_FkNoAction;
      }
      توڑو;
    }

    /*
    **  sqlite3_test_control(BITVEC_TEST, size, program)
    **
    ** Run a test against a Bitvec object of size.  The program argument
    ** is an array of integers that defines the test.  Return -1 on a
    ** memory allocation error, 0 on success, or non-zero for an error.
    ** See the sqlite3BitvecBuiltinTest() for additional information.
    */
    حالت SQLITE_TESTCTRL_BITVEC_TEST: {
      صحیح sz = va_arg(ap, صحیح);
      صحیح *aProg = va_arg(ap, صحیح*);
      rc = sqlite3BitvecBuiltinTest(sz, aProg);
      توڑو;
    }

    /*
    **  sqlite3_test_control(FAULT_INSTALL, xCallback)
    **
    ** Arrange to invoke xCallback() whenever sqlite3FaultSim() is called,
    ** if xCallback is not NULL.
    **
    ** As a test of the fault simulator mechanism itself, sqlite3FaultSim(0)
    ** is called immediately after installing the new callback and the return
    ** value from sqlite3FaultSim(0) becomes the return from
    ** sqlite3_test_control().
    */
    حالت SQLITE_TESTCTRL_FAULT_INSTALL: {
      /* A bug in MSVC prevents it from understanding pointers to functions
      ** types in the second argument to va_arg().  Work around the problem
      ** using a typedef.
      ** http://support.microsoft.com/kb/47961  <-- dead hyperlink
      ** Search at http://web.archive.org/ to find the 2015-03-16 archive
      ** of the link above to see the original text.
      ** sqlite3GlobalConfig.xTestCallback = va_arg(ap, int(*)(int));
      */
      قسمتعریف صحیح(*sqlite3FaultFuncType)(صحیح);
      sqlite3GlobalConfig.xTestCallback = va_arg(ap, sqlite3FaultFuncType);
      rc = sqlite3FaultSim(0);
      توڑو;
    }

    /*
    **  sqlite3_test_control(BENIGN_MALLOC_HOOKS, xBegin, xEnd)
    **
    ** Register hooks to call to indicate which malloc() failures
    ** are benign.
    */
    حالت SQLITE_TESTCTRL_BENIGN_MALLOC_HOOKS: {
      قسمتعریف خالی (*void_function)(خالی);
      void_function xBenignBegin;
      void_function x_Benign_انتہا;
      xBenignBegin = va_arg(ap, void_function);
      x_Benign_انتہا = va_arg(ap, void_function);
      sqlite3BenignMallocHooks(xBenignBegin, x_Benign_انتہا);
      توڑو;
    }

    /*
    **  sqlite3_test_control(SQLITE_TESTCTRL_PENDING_BYTE, unsigned int X)
    **
    ** Set the PENDING byte to the value in the argument, if X>0.
    ** Make no changes if X==0.  Return the value of the pending byte
    ** as it existing before this routine was called.
    **
    ** IMPORTANT:  Changing the PENDING byte from 0x40000000 results in
    ** an incompatible database file format.  Changing the PENDING byte
    ** while any database connection is open results in undefined and
    ** deleterious behavior.
    */
    حالت SQLITE_TESTCTRL_PENDING_BYTE: {
      rc = PENDING_BYTE;
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WSD
      {
        غیرعلامتی صحیح newVal = va_arg(ap, غیرعلامتی صحیح);
        اگر( newVal ) sqlite3PendingByte = newVal;
      }
#اگر_ختم
      توڑو;
    }

    /*
    **  sqlite3_test_control(SQLITE_TESTCTRL_ASSERT, int X)
    **
    ** This action provides a run-time test to see whether or not
    ** assert() was enabled at compile-time.  If X is true and assert()
    ** is enabled, then the return value is true.  If X is true and
    ** assert() is disabled, then the return value is zero.  If X is
    ** false and assert() is enabled, then the assertion fires and the
    ** process aborts.  If X is false and assert() is disabled, then the
    ** return value is zero.
    */
    حالت SQLITE_TESTCTRL_ASSERT: {
      متلون صحیح x = 0;
      أكد( /*side-effects-ok*/ (x = va_arg(ap,صحیح))!=0 );
      rc = x;
#اگر defined(SQLITE_DEBUG)
      /* Invoke these debugging routines so that the compiler does not
      ** issue "defined but not used" warnings. */
      اگر( x==9999 ){
        sqlite3_Show_اظہاریہ(0);
        sqlite3_Show_اظہاریہ_List(0);
        sqlite3ShowIdList(0);
        sqlite3ShowSrcList(0);
        sqlite3ShowWith(0);
        sqlite3ShowUpsert(0);
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_TRIGGER
        sqlite3_Show_Trigger_قدم(0);
        sqlite3_Show_Trigger_قدم_List(0);
        sqlite3ShowTrigger(0);
        sqlite3ShowTriggerList(0);
#اگر_ختم
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
        sqlite3_Show_کھڑکی(0);
        sqlite3ShowWinFunc(0);
#اگر_ختم
        sqlite3ShowSelect(0);
      }
#اگر_ختم
      توڑو;
    }


    /*
    **  sqlite3_test_control(SQLITE_TESTCTRL_ALWAYS, int X)
    **
    ** This action provides a run-time test to see how the ALWAYS and
    ** NEVER macros were defined at compile-time.
    **
    ** The return value is ALWAYS(X) if X is true, or 0 if X is false.
    **
    ** The recommended test is X==2.  If the return value is 2, that means
    ** ALWAYS() and NEVER() are both no-op pass-through macros, which is the
    ** default setting.  If the return value is 1, then ALWAYS() is either
    ** hard-coded to true or else it asserts if its argument is false.
    ** The first behavior (hard-coded to true) is the case if
    ** SQLITE_TESTCTRL_ASSERT shows that assert() is disabled and the second
    ** behavior (assert if the argument to ALWAYS() is false) is the case if
    ** SQLITE_TESTCTRL_ASSERT shows that assert() is enabled.
    **
    ** The run-time test procedure might look something like this:
    **
    **    if( sqlite3_test_control(SQLITE_TESTCTRL_ALWAYS, 2)==2 ){
    **      // ALWAYS() and NEVER() are no-op pass-through macros
    **    }else if( sqlite3_test_control(SQLITE_TESTCTRL_ASSERT, 1) ){
    **      // ALWAYS(x) asserts that x is true. NEVER(x) asserts x is false.
    **    }else{
    **      // ALWAYS(x) is a constant 1.  NEVER(x) is a constant 0.
    **    }
    */
    حالت SQLITE_TESTCTRL_ALWAYS: {
      صحیح x = va_arg(ap,صحیح);
      rc = x ? ALWAYS(x) : 0;
      توڑو;
    }

    /*
    **   sqlite3_test_control(SQLITE_TESTCTRL_BYTEORDER);
    **
    ** The integer returned reveals the byte-order of the computer on which
    ** SQLite is running:
    **
    **       1     big-endian,    determined at run-time
    **      10     little-endian, determined at run-time
    **  432101     big-endian,    determined at compile-time
    **  123410     little-endian, determined at compile-time
    */
    حالت SQLITE_TESTCTRL_BYTEORDER: {
      rc = SQLITE_BYTEORDER*100 + SQLITE_LITTLEENDIAN*10 + SQLITE_BIGENDIAN;
      توڑو;
    }

    /*  sqlite3_test_control(SQLITE_TESTCTRL_OPTIMIZATIONS, sqlite3 *db, int N)
    **
    ** Enable or disable various optimizations for testing purposes.  The
    ** argument N is a bitmask of optimizations to be disabled.  For normal
    ** operation N should be 0.  The idea is that a test program (like the
    ** SQL Logic Test or SLT test module) can run the same SQL multiple times
    ** with various optimizations disabled to verify that the same answer
    ** is obtained in every case.
    */
    حالت SQLITE_TESTCTRL_OPTIMIZATIONS: {
      sqlite3 *db = va_arg(ap, sqlite3*);
      db->dbOptFlags = va_arg(ap, u32);
      توڑو;
    }

    /*  sqlite3_test_control(SQLITE_TESTCTRL_GETOPT, sqlite3 *db, int *N)
    **
    ** Write the current optimization settings into *N.  A zero bit means that
    ** the optimization is on, and a 1 bit means that the optimization is off.
    */
    حالت SQLITE_TESTCTRL_GETOPT: {
      sqlite3 *db = va_arg(ap, sqlite3*);
      صحیح *pN = va_arg(ap, صحیح*);
      *pN = db->dbOptFlags;
      توڑو;
    }

    /*   sqlite3_test_control(SQLITE_TESTCTRL_LOCALTIME_FAULT, onoff, xAlt);
    **
    ** If parameter onoff is 1, subsequent calls to localtime() fail.
    ** If 2, then invoke xAlt() instead of localtime().  If 0, normal
    ** processing.
    **
    ** xAlt arguments are void pointers, but they really want to be:
    **
    **    int xAlt(const time_t*, struct tm*);
    **
    ** xAlt should write results in to struct tm object of its 2nd argument
    ** and return zero on success, or return non-zero on failure.
    */
    حالت SQLITE_TESTCTRL_LOCALTIME_FAULT: {
      sqlite3GlobalConfig.bLocaltimeFault = va_arg(ap, صحیح);
      اگر( sqlite3GlobalConfig.bLocaltimeFault==2 ){
        قسمتعریف صحیح(*sqlite3LocaltimeType)(ثابت خالی*,خالی*);
        sqlite3GlobalConfig.xAltLocaltime = va_arg(ap, sqlite3LocaltimeType);
      }ورنہ{
        sqlite3GlobalConfig.xAltLocaltime = 0;
      }
      توڑو;
    }

    /*   sqlite3_test_control(SQLITE_TESTCTRL_INTERNAL_FUNCTIONS, sqlite3*);
    **
    ** Toggle the ability to use internal functions on or off for
    ** the database connection given in the argument.
    */
    حالت SQLITE_TESTCTRL_اندرونی_FUNCTIONS: {
      sqlite3 *db = va_arg(ap, sqlite3*);
      db->mDbFlags ^= DBFLAG_اندرونی_Func;
      توڑو;
    }

    /*   sqlite3_test_control(SQLITE_TESTCTRL_NEVER_CORRUPT, int);
    **
    ** Set or clear a flag that indicates that the database file is always well-
    ** formed and never corrupt.  This flag is clear by default, indicating that
    ** database files might have arbitrary corruption.  Setting the flag during
    ** testing causes certain assert() statements in the code to be activated
    ** that demonstrate invariants on well-formed database files.
    */
    حالت SQLITE_TESTCTRL_NEVER_CORRUPT: {
      sqlite3GlobalConfig.neverCorrupt = va_arg(ap, صحیح);
      توڑو;
    }

    /*   sqlite3_test_control(SQLITE_TESTCTRL_EXTRA_SCHEMA_CHECKS, int);
    **
    ** Set or clear a flag that causes SQLite to verify that type, name,
    ** and tbl_name fields of the sqlite_schema table.  This is normally
    ** on, but it is sometimes useful to turn it off for testing.
    **
    ** 2020-07-22:  Disabling EXTRA_SCHEMA_CHECKS also disables the
    ** verification of rootpage numbers when parsing the schema.  This
    ** is useful to make it easier to reach strange internal error states
    ** during testing.  The EXTRA_SCHEMA_CHECKS setting is always enabled
    ** in production.
    */
    حالت SQLITE_TESTCTRL_EXTRA_SCHEMA_CHECKS: {
      sqlite3GlobalConfig.bExtraSchemaChecks = va_arg(ap, صحیح);
      توڑو;
    }

    /* Set the threshold at which OP_Once counters reset back to zero.
    ** By default this is 0x7ffffffe (over 2 billion), but that value is
    ** too big to test in a reasonable amount of time, so this control is
    ** provided to set a small and easily reachable reset value.
    */
    حالت SQLITE_TESTCTRL_ONCE_RESET_حد: {
      sqlite3GlobalConfig.i_Once_Reset_حد = va_arg(ap, صحیح);
      توڑو;
    }

    /*   sqlite3_test_control(SQLITE_TESTCTRL_VDBE_COVERAGE, xCallback, ptr);
    **
    ** Set the VDBE coverage callback function to xCallback with context
    ** pointer ptr.
    */
    حالت SQLITE_TESTCTRL_VDBE_COVERAGE: {
#اگر_تعریف_شدہ SQLITE_VDBE_COVERAGE
      قسمتعریف خالی (*branch_callback)(خالی*,غیرعلامتی صحیح,
                                      غیرعلامتی حرف,غیرعلامتی حرف);
      sqlite3GlobalConfig.xVdbeBranch = va_arg(ap,branch_callback);
      sqlite3GlobalConfig.pVdbeBranchArg = va_arg(ap,خالی*);
#اگر_ختم
      توڑو;
    }

    /*   sqlite3_test_control(SQLITE_TESTCTRL_SORTER_MMAP, db, nMax); */
    حالت SQLITE_TESTCTRL_SORTER_MMAP: {
      sqlite3 *db = va_arg(ap, sqlite3*);
      db->nMaxSorterMmap = va_arg(ap, صحیح);
      توڑو;
    }

    /*   sqlite3_test_control(SQLITE_TESTCTRL_ISINIT);
    **
    ** Return SQLITE_OK if SQLite has been initialized and SQLITE_ERROR if
    ** not.
    */
    حالت SQLITE_TESTCTRL_ISINIT: {
      اگر( sqlite3GlobalConfig.isInit==0 ) rc = SQLITE_ERROR;
      توڑو;
    }

    /*  sqlite3_test_control(SQLITE_TESTCTRL_IMPOSTER, db, dbName, mode, tnum);
    **
    ** This test control is used to create imposter tables.  "db" is a pointer
    ** to the database connection.  dbName is the database name (ex: "main" or
    ** "temp") which will receive the imposter.  "mode" turns imposter mode on
    ** or off.  mode==0 means imposter mode is off.  mode==1 means imposter mode
    ** is on.  mode==2 means imposter mode is on but results in an imposter
    ** table that is read-only unless writable_schema is on.  "tnum" is the
    ** root page of the b-tree to which the imposter table should connect.
    **
    ** Enable imposter mode only when the schema has already been parsed.  Then
    ** run a single CREATE TABLE statement to construct the imposter table in
    ** the parsed schema.  Then turn imposter mode back off again.
    **
    ** If onOff==0 and tnum>0 then reset the schema for all databases, causing
    ** the schema to be reparsed the next time it is needed.  This has the
    ** effect of erasing all imposter tables.
    */
    حالت SQLITE_TESTCTRL_IMPOSTER: {
      sqlite3 *db = va_arg(ap, sqlite3*);
      صحیح iDb;
      sqlite3_mutex_enter(db->mutex);
      iDb = sqlite3_ڈھونڈو_Db_Name(db, va_arg(ap,ثابت حرف*));
      اگر( iDb>=0 ){
        db->init.iDb = iDb;
        db->init.busy = db->init.imposter_جدول = va_arg(ap,صحیح);
        db->init.newTnum = va_arg(ap,صحیح);
        اگر( db->init.busy==0 && db->init.newTnum>0 ){
          sqlite3_Reset_All_Schemas_کا_Connection(db);
        }
      }
      sqlite3_mutex_leave(db->mutex);
      توڑو;
    }

#اگر defined(YYCOVERAGE)
    /*  sqlite3_test_control(SQLITE_TESTCTRL_PARSER_COVERAGE, FILE *out)
    **
    ** This test control (only available when SQLite is compiled with
    ** -DYYCOVERAGE) writes a report onto "out" that shows all
    ** state/lookahead combinations in the parser state machine
    ** which are never exercised.  If any state is missed, make the
    ** return code SQLITE_ERROR.
    */
    حالت SQLITE_TESTCTRL_PARSER_COVERAGE: {
      FILE *برآمد = va_arg(ap, FILE*);
      اگر( sqlite3ParserCoverage(برآمد) ) rc = SQLITE_ERROR;
      توڑو;
    }
#اگر_ختم /* defined(YYCOVERAGE) */

    /*  sqlite3_test_control(SQLITE_TESTCTRL_RESULT_INTREAL, sqlite3_context*);
    **
    ** This test-control causes the most recent sqlite3_result_int64() value
    ** to be interpreted as a MEM_IntReal instead of as an MEM_Int.  Normally,
    ** MEM_IntReal values only arise during an INSERT operation of integer
    ** values into a REAL column, so they can be challenging to test.  This
    ** test-control enables us to write an intreal() SQL function that can
    ** inject an intreal() value at arbitrary places in an SQL statement,
    ** for testing purposes.
    */
    حالت SQLITE_TESTCTRL_نتیجہ_INTREAL: {
      sqlite3_context *pCtx = va_arg(ap, sqlite3_context*);
      sqlite3_نتیجہ_Int_Real(pCtx);
      توڑو;
    }

    /*  sqlite3_test_control(SQLITE_TESTCTRL_SEEK_COUNT,
    **    sqlite3 *db,    // Database connection
    **    u64 *pnSeek     // Write seek count here
    **  );
    **
    ** This test-control queries the seek-counter on the "main" database
    ** file.  The seek-counter is written into *pnSeek and is then reset.
    ** The seek-count is only available if compiled with SQLITE_DEBUG.
    */
    حالت SQLITE_TESTCTRL_SEEK_گنتی: {
      sqlite3 *db = va_arg(ap, sqlite3*);
      u64 *pn = va_arg(ap, sqlite3_uint64*);
      *pn = sqlite3_Btree_Seek_گنتی(db->aDb->pBt);
      (خالی)db;  /* Silence harmless unused variable warning */
      توڑو;
    }

    /*  sqlite3_test_control(SQLITE_TESTCTRL_TRACEFLAGS, op, ptr)
    **
    **  "ptr" is a pointer to a u32. 
    **
    **   op==0       Store the current sqlite3TreeTrace in *ptr
    **   op==1       Set sqlite3TreeTrace to the value *ptr
    **   op==2       Store the current sqlite3WhereTrace in *ptr
    **   op==3       Set sqlite3WhereTrace to the value *ptr
    */
    حالت SQLITE_TESTCTRL_TRACEFLAGS: {
       صحیح opTrace = va_arg(ap, صحیح);
       u32 *ptr = va_arg(ap, u32*);
       انتخاب( opTrace ){
         حالت 0:   *ptr = sqlite3_درخت_Trace;      توڑو;
         حالت 1:   sqlite3_درخت_Trace = *ptr;      توڑو;
         حالت 2:   *ptr = sqlite3WhereTrace;     توڑو;
         حالت 3:   sqlite3WhereTrace = *ptr;     توڑو;
       }
       توڑو;
    }

    /* sqlite3_test_control(SQLITE_TESTCTRL_LOGEST,
    **      double fIn,     // Input value
    **      int *pLogEst,   // sqlite3LogEstFromDouble(fIn)
    **      u64 *pInt,      // sqlite3LogEstToInt(*pLogEst)
    **      int *pLogEst2   // sqlite3LogEst(*pInt)
    ** );
    **
    ** Test access for the LogEst conversion routines.
    */
    حالت SQLITE_TESTCTRL_LOGEST: {
      دوہرا r_اندر = va_arg(ap, دوہرا);
      LogEst rLogEst = sqlite3LogEstFromDouble(r_اندر);
      صحیح *pI1 = va_arg(ap,صحیح*);
      u64 *pU64 = va_arg(ap,u64*);
      صحیح *pI2 = va_arg(ap,صحیح*);
      *pI1 = rLogEst;
      *pU64 = sqlite3_Log_Est_مقصد_Int(rLogEst);
      *pI2 = sqlite3LogEst(*pU64);
      توڑو;
    }

    /* sqlite3_test_control(SQLITE_TESTCTRL_ATOF, const char *z, double *p);
    **
    ** Test access to the sqlite3AtoF() routine.
    */
    حالت SQLITE_TESTCTRL_ATOF: {
      ثابت حرف *z = va_arg(ap,ثابت حرف*);
      دوہرا *pR = va_arg(ap,دوہرا*);
      rc = sqlite3AtoF(z,pR);
      توڑو;
    }

#اگر defined(SQLITE_DEBUG) && !defined(SQLITE_OMIT_WSD)
    /* sqlite3_test_control(SQLITE_TESTCTRL_TUNE, id, *piValue)
    **
    ** If "id" is an integer between 1 and SQLITE_NTUNE then set the value
    ** of the id-th tuning parameter to *piValue.  If "id" is between -1
    ** and -SQLITE_NTUNE, then write the current value of the (-id)-th
    ** tuning parameter into *piValue.
    **
    ** Tuning parameters are for use during transient development builds,
    ** to help find the best values for constants in the query planner.
    ** Access tuning parameters using the Tuning(ID) macro.  Set the
    ** parameters in the CLI using ".testctrl tune ID VALUE".
    **
    ** Transient use only.  Tuning parameters should not be used in
    ** checked-in code.
    */
    حالت SQLITE_TESTCTRL_TUNE: {
      صحیح id = va_arg(ap, صحیح);
      صحیح *pi_قدر = va_arg(ap, صحیح*);
      اگر( id>0 && id<=SQLITE_NTUNE ){
        Tuning(id) = *pi_قدر;
      }ورنہ اگر( id<0 && id>=-SQLITE_NTUNE ){
        *pi_قدر = Tuning(-id);
      }ورنہ{
        rc = SQLITE_NOTFOUND;
      }
      توڑو;
    }
#اگر_ختم

    /* sqlite3_test_control(SQLITE_TESTCTRL_JSON_SELFCHECK, &onOff);
    **
    ** Activate or deactivate validation of JSONB that is generated from
    ** text.  Off by default, as the validation is slow.  Validation is
    ** only available if compiled using SQLITE_DEBUG.
    **
    ** If onOff is initially 1, then turn it on.  If onOff is initially
    ** off, turn it off.  If onOff is initially -1, then change onOff
    ** to be the current setting.
    */
    حالت SQLITE_TESTCTRL_JSON_SELFCHECK: {
#اگر defined(SQLITE_DEBUG) && !defined(SQLITE_OMIT_WSD)
      صحیح *pOnOff = va_arg(ap, صحیح*);
      اگر( *pOnOff<0 ){
        *pOnOff = sqlite3Config.bJsonSelfcheck;
      }ورنہ{
        sqlite3Config.bJsonSelfcheck = (u8)((*pOnOff)&0xff);
      }
#اگر_ختم
      توڑو;
    }
  }
  va_end(ap);
#اگر_ختم /* SQLITE_UNTESTABLE */
  واپس rc;
}

/*
** The Pager stores the Database filename, Journal filename, and WAL filename
** consecutively in memory, in that order.  The database filename is prefixed
** by four zero bytes.  Locate the start of the database filename by searching
** backwards for the first byte following four consecutive zero bytes.
**
** This only works if the filename passed in was obtained from the Pager.
*/
ساکن ثابت حرف *databaseName(ثابت حرف *zName){
  جب( zName[-1]!=0 || zName[-2]!=0 || zName[-3]!=0 || zName[-4]!=0 ){
    zName--;
  }
  واپس zName;
}

/*
** Append text z[] to the end of p[].  Return a pointer to the first
** character after then zero terminator on the new text in p[].
*/
ساکن حرف *append_متن(حرف *p, ثابت حرف *z){
  size_t n = لڑی_لمبائی(z);
  حافظہ_نقل(p, z, n+1);
  واپس p+n+1;
}

/*
** Allocate memory to hold names for a database, journal file, WAL file,
** and query parameters.  The pointer returned is valid for use by
** sqlite3_filename_database() and sqlite3_uri_parameter() and related
** functions.
**
** Memory layout must be compatible with that generated by the pager
** and expected by sqlite3_uri_parameter() and databaseName().
*/
ثابت حرف *sqlite3_create_filename(
  ثابت حرف *zDatabase,
  ثابت حرف *zJournal,
  ثابت حرف *zWal,
  صحیح nParam,
  ثابت حرف **azParam
){
  sqlite3_int64 nByte;
  صحیح i;
  حرف *p_نتیجہ, *p;
  nByte = لڑی_لمبائی(zDatabase) + لڑی_لمبائی(zJournal) + لڑی_لمبائی(zWal) + 10;
  برائے(i=0; i<nParam*2; i++){
    nByte += لڑی_لمبائی(azParam[i])+1;
  }
  p_نتیجہ = p = sqlite3_malloc64( nByte );
  اگر( p==0 ) واپس 0;
  memset(p, 0, 4);
  p += 4;
  p = append_متن(p, zDatabase);
  برائے(i=0; i<nParam*2; i++){
    p = append_متن(p, azParam[i]);
  }
  *(p++) = 0;
  p = append_متن(p, zJournal);
  p = append_متن(p, zWal);
  *(p++) = 0;
  *(p++) = 0;
  أكد( (sqlite3_int64)(p - p_نتیجہ)==nByte );
  واپس p_نتیجہ + 4;
}

/*
** Free memory obtained from sqlite3_create_filename().  It is a severe
** error to call this routine with any parameter other than a pointer
** previously obtained from sqlite3_create_filename() or a NULL pointer.
*/
خالی sqlite3_free_filename(ثابت حرف *p){
  اگر( p==0 ) واپس;
  p = databaseName(p);
  sqlite3_free((حرف*)p - 4);
}


/*
** This is a utility routine, useful to VFS implementations, that checks
** to see if a database file was a URI that contained a specific query
** parameter, and if so obtains the value of the query parameter.
**
** The zFilename argument is the filename pointer passed into the xOpen()
** method of a VFS implementation.  The zParam argument is the name of the
** query parameter we seek.  This routine returns the value of the zParam
** parameter if it exists.  If the parameter does not exist, this routine
** returns a NULL pointer.
*/
ثابت حرف *sqlite3_uri_parameter(ثابت حرف *zFilename, ثابت حرف *zParam){
  اگر( zFilename==0 || zParam==0 ) واپس 0;
  zFilename = databaseName(zFilename);
  واپس uriParameter(zFilename, zParam);
}

/*
** Return a pointer to the name of Nth query parameter of the filename.
*/
ثابت حرف *sqlite3_uri_کلید(ثابت حرف *zFilename, صحیح N){
  اگر( zFilename==0 || N<0 ) واپس 0;
  zFilename = databaseName(zFilename);
  zFilename += sqlite3Strlen30(zFilename) + 1;
  جب( ALWAYS(zFilename) && zFilename[0] && (N--)>0 ){
    zFilename += sqlite3Strlen30(zFilename) + 1;
    zFilename += sqlite3Strlen30(zFilename) + 1;
  }
  واپس zFilename[0] ? zFilename : 0;
}

/*
** Return a boolean value for a query parameter.
*/
صحیح sqlite3_uri_boolean(ثابت حرف *zFilename, ثابت حرف *zParam, صحیح bDflt){
  ثابت حرف *z = sqlite3_uri_parameter(zFilename, zParam);
  bDflt = bDflt!=0;
  واپس z ? sqlite3GetBoolean(z, bDflt) : bDflt;
}

/*
** Return a 64-bit integer value for a query parameter.
*/
sqlite3_int64 sqlite3_uri_int64(
  ثابت حرف *zFilename,    /* Filename as passed to xOpen */
  ثابت حرف *zParam,       /* URI parameter sought */
  sqlite3_int64 bDflt       /* return if parameter is missing */
){
  ثابت حرف *z = sqlite3_uri_parameter(zFilename, zParam);
  sqlite3_int64 v;
  اگر( z && sqlite3_Dec_Or_Hex_مقصد_I64(z, &v)==0 ){
    bDflt = v;
  }
  واپس bDflt;
}

/*
** Translate a filename that was handed to a VFS routine into the corresponding
** database, journal, or WAL file.
**
** It is an error to pass this routine a filename string that was not
** passed into the VFS from the SQLite core.  Doing so is similar to
** passing free() a pointer that was not obtained from malloc() - it is
** an error that we cannot easily detect but that will likely cause memory
** corruption.
*/
ثابت حرف *sqlite3_filename_database(ثابت حرف *zFilename){
  اگر( zFilename==0 ) واپس 0;
  واپس databaseName(zFilename);
}
ثابت حرف *sqlite3_filename_journal(ثابت حرف *zFilename){
  اگر( zFilename==0 ) واپس 0;
  zFilename = databaseName(zFilename);
  zFilename += sqlite3Strlen30(zFilename) + 1;
  جب( ALWAYS(zFilename) && zFilename[0] ){
    zFilename += sqlite3Strlen30(zFilename) + 1;
    zFilename += sqlite3Strlen30(zFilename) + 1;
  }
  واپس zFilename + 1;
}
ثابت حرف *sqlite3_filename_wal(ثابت حرف *zFilename){
#اگر_تعریف_شدہ SQLITE_OMIT_WAL
  غیر_مستعمل_PARAMETER(zFilename);
  واپس 0;
#ورنہ
  zFilename = sqlite3_filename_journal(zFilename);
  اگر( zFilename ) zFilename += sqlite3Strlen30(zFilename) + 1;
  واپس zFilename;
#اگر_ختم
}

/*
** Return the Btree pointer identified by zDbName.  Return NULL if not found.
*/
Btree *sqlite3_Db_Name_مقصد_Btree(sqlite3 *db, ثابت حرف *zDbName){
  صحیح iDb = zDbName ? sqlite3_ڈھونڈو_Db_Name(db, zDbName) : 0;
  واپس iDb<0 ? 0 : db->aDb[iDb].pBt;
}

/*
** Return the name of the N-th database schema.  Return NULL if N is out
** of range.
*/
ثابت حرف *sqlite3_db_name(sqlite3 *db, صحیح N){
  ثابت حرف *zRet = 0;
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db) ){
    (خالی)SQLITE_MISUSE_BKPT;
    واپس 0;
  }
#اگر_ختم
  sqlite3_mutex_enter(db->mutex);
  اگر( N>=0 && N<db->nDb ){
    zRet = db->aDb[N].zDbSName;
  }
  sqlite3_mutex_leave(db->mutex);
  واپس zRet;
}

/*
** Return the filename of the database associated with a database
** connection.
*/
ثابت حرف *sqlite3_db_filename(sqlite3 *db, ثابت حرف *zDbName){
  Btree *pBt;
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db) ){
    (خالی)SQLITE_MISUSE_BKPT;
    واپس 0;
  }
#اگر_ختم
  pBt = sqlite3_Db_Name_مقصد_Btree(db, zDbName);
  واپس pBt ? sqlite3BtreeGetFilename(pBt) : 0;
}

/*
** Return 1 if database is read-only or 0 if read/write.  Return -1 if
** no such database exists.
*/
صحیح sqlite3_db_readonly(sqlite3 *db, ثابت حرف *zDbName){
  Btree *pBt;
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db) ){
    (خالی)SQLITE_MISUSE_BKPT;
    واپس -1;
  }
#اگر_ختم
  pBt = sqlite3_Db_Name_مقصد_Btree(db, zDbName);
  واپس pBt ? sqlite3BtreeIsReadonly(pBt) : -1;
}

#اگر_تعریف_شدہ SQLITE_ENABLE_SNAPSHOT
/*
** Obtain a snapshot handle for the snapshot of database zDb currently
** being read by handle db.
*/
صحیح sqlite3_snapshot_get(
  sqlite3 *db,
  ثابت حرف *zDb,
  sqlite3_snapshot **ppSnapshot
){
  صحیح rc = SQLITE_ERROR;
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WAL

#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db) ){
    واپس SQLITE_MISUSE_BKPT;
  }
#اگر_ختم
  sqlite3_mutex_enter(db->mutex);

  اگر( db->autoCommit==0 ){
    صحیح iDb = sqlite3_ڈھونڈو_Db_Name(db, zDb);
    اگر( iDb==0 || iDb>1 ){
      Btree *pBt = db->aDb[iDb].pBt;
      اگر( SQLITE_TXN_WRITE!=sqlite3BtreeTxnState(pBt) ){
        Pager *pPager = sqlite3BtreePager(pBt);
        i64 dummy = 0;
        sqlite3PagerSnapshotOpen(pPager, (sqlite3_snapshot*)&dummy);
        rc = sqlite3BtreeBeginTrans(pBt, 0, 0);
        sqlite3PagerSnapshotOpen(pPager, 0);
        اگر( rc==SQLITE_OK ){
          rc = sqlite3PagerSnapshotGet(sqlite3BtreePager(pBt), ppSnapshot);
        }
      }
    }
  }

  sqlite3_mutex_leave(db->mutex);
#اگر_ختم   /* SQLITE_OMIT_WAL */
  واپس rc;
}

/*
** Open a read-transaction on the snapshot identified by pSnapshot.
*/
صحیح sqlite3_snapshot_open(
  sqlite3 *db,
  ثابت حرف *zDb,
  sqlite3_snapshot *pSnapshot
){
  صحیح rc = SQLITE_ERROR;
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WAL

#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db) ){
    واپس SQLITE_MISUSE_BKPT;
  }
#اگر_ختم
  sqlite3_mutex_enter(db->mutex);
  اگر( db->autoCommit==0 ){
    صحیح iDb;
    iDb = sqlite3_ڈھونڈو_Db_Name(db, zDb);
    اگر( iDb==0 || iDb>1 ){
      Btree *pBt = db->aDb[iDb].pBt;
      اگر( sqlite3BtreeTxnState(pBt)!=SQLITE_TXN_WRITE ){
        Pager *pPager = sqlite3BtreePager(pBt);
        صحیح bUnlock = 0;
        اگر( sqlite3BtreeTxnState(pBt)!=SQLITE_TXN_NONE ){
          اگر( db->n_Vdbe_فعال==0 ){
            rc = sqlite3PagerSnapshotCheck(pPager, pSnapshot);
            اگر( rc==SQLITE_OK ){
              bUnlock = 1;
              rc = sqlite3BtreeCommit(pBt);
            }
          }
        }ورنہ{
          rc = SQLITE_OK;
        }
        اگر( rc==SQLITE_OK ){
          rc = sqlite3PagerSnapshotOpen(pPager, pSnapshot);
        }
        اگر( rc==SQLITE_OK ){
          rc = sqlite3BtreeBeginTrans(pBt, 0, 0);
          sqlite3PagerSnapshotOpen(pPager, 0);
        }
        اگر( bUnlock ){
          sqlite3PagerSnapshotUnlock(pPager);
        }
      }
    }
  }

  sqlite3_mutex_leave(db->mutex);
#اگر_ختم   /* SQLITE_OMIT_WAL */
  واپس rc;
}

/*
** Recover as many snapshots as possible from the wal file associated with
** schema zDb of database db.
*/
صحیح sqlite3_snapshot_recover(sqlite3 *db, ثابت حرف *zDb){
  صحیح rc = SQLITE_ERROR;
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WAL
  صحیح iDb;

#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db) ){
    واپس SQLITE_MISUSE_BKPT;
  }
#اگر_ختم

  sqlite3_mutex_enter(db->mutex);
  iDb = sqlite3_ڈھونڈو_Db_Name(db, zDb);
  اگر( iDb==0 || iDb>1 ){
    Btree *pBt = db->aDb[iDb].pBt;
    اگر( SQLITE_TXN_NONE==sqlite3BtreeTxnState(pBt) ){
      rc = sqlite3BtreeBeginTrans(pBt, 0, 0);
      اگر( rc==SQLITE_OK ){
        rc = sqlite3PagerSnapshotRecover(sqlite3BtreePager(pBt));
        sqlite3BtreeCommit(pBt);
      }
    }
  }
  sqlite3_mutex_leave(db->mutex);
#اگر_ختم   /* SQLITE_OMIT_WAL */
  واپس rc;
}

/*
** Free a snapshot handle obtained from sqlite3_snapshot_get().
*/
خالی sqlite3_snapshot_free(sqlite3_snapshot *pSnapshot){
  sqlite3_free(pSnapshot);
}
#اگر_ختم /* SQLITE_ENABLE_SNAPSHOT */

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_COMPILEOPTION_DIAGS
/*
** Given the name of a compile-time option, return true if that option
** was used and false if not.
**
** The name can optionally begin with "SQLITE_" but the "SQLITE_" prefix
** is not required for a match.
*/
صحیح sqlite3_compileoption_مستعمل(ثابت حرف *zOptName){
  صحیح i, n;
  صحیح nOpt;
  ثابت حرف **azCompileOpt;

#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( zOptName==0 ){
    (خالی)SQLITE_MISUSE_BKPT;
    واپس 0;
  }
#اگر_ختم

  azCompileOpt = sqlite3CompileOptions(&nOpt);

  اگر( sqlite3StrNICmp(zOptName, "SQLITE_", 7)==0 ) zOptName += 7;
  n = sqlite3Strlen30(zOptName);

  /* Since nOpt is normally in single digits, a linear search is
  ** adequate. No need for a binary search. */
  برائے(i=0; i<nOpt; i++){
    اگر( sqlite3StrNICmp(zOptName, azCompileOpt[i], n)==0
     && sqlite3IsIdChar((غیرعلامتی حرف)azCompileOpt[i][n])==0
    ){
      واپس 1;
    }
  }
  واپس 0;
}

/*
** Return the N-th compile-time option string.  If N is out of range,
** return a NULL pointer.
*/
ثابت حرف *sqlite3_compileoption_get(صحیح N){
  صحیح nOpt;
  ثابت حرف **azCompileOpt;
  azCompileOpt = sqlite3CompileOptions(&nOpt);
  اگر( N>=0 && N<nOpt ){
    واپس azCompileOpt[N];
  }
  واپس 0;
}
#اگر_ختم /* SQLITE_OMIT_COMPILEOPTION_DIAGS */
