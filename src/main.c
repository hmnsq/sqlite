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
#include "sqliteInt.h"

#ifdef SQLITE_ENABLE_FTS3
# include "fts3.h"
#endif
#ifdef SQLITE_ENABLE_RTREE
# include "rtree.h"
#endif
#如果 defined(SQLITE_ENABLE_ICU) || defined(SQLITE_ENABLE_ICU_COLLATIONS)
# include "sqliteicu.h"
#endif

/*
** This is an extension initializer that is a no-op and always
** succeeds, except that it fails if the fault-simulation is set
** to 500.
*/
static 整数 sqlite3TestExtInit(sqlite3 *db){
  (void)db;
  返回 sqlite3FaultSim(500);
}


/*
** Forward declarations of external module initializer functions
** for modules that need them.
*/
#ifdef SQLITE_ENABLE_FTS5
整数 sqlite3Fts5Init(sqlite3*);
#endif
#ifdef SQLITE_ENABLE_STMTVTAB
整数 sqlite3StmtVtabInit(sqlite3*);
#endif
#ifdef SQLITE_EXTRA_AUTOEXT
整数 SQLITE_EXTRA_AUTOEXT(sqlite3*);
#endif
/*
** An array of pointers to extension initializer functions for
** built-in extensions.
*/
static 整数 (*const sqlite3BuiltinExtensions[])(sqlite3*) = {
#ifdef SQLITE_ENABLE_FTS3
  sqlite3Fts3Init,
#endif
#ifdef SQLITE_ENABLE_FTS5
  sqlite3Fts5Init,
#endif
#如果 defined(SQLITE_ENABLE_ICU) || defined(SQLITE_ENABLE_ICU_COLLATIONS)
  sqlite3IcuInit,
#endif
#ifdef SQLITE_ENABLE_RTREE
  sqlite3RtreeInit,
#endif
#ifdef SQLITE_ENABLE_DBPAGE_VTAB
  sqlite3DbpageRegister,
#endif
#ifdef SQLITE_ENABLE_DBSTAT_VTAB
  sqlite3DbstatRegister,
#endif
  sqlite3TestExtInit,
#ifdef SQLITE_ENABLE_STMTVTAB
  sqlite3StmtVtabInit,
#endif
#ifdef SQLITE_ENABLE_BYTECODE_VTAB
  sqlite3VdbeBytecodeVtabInit,
#endif
#ifdef SQLITE_EXTRA_AUTOEXT
  SQLITE_EXTRA_AUTOEXT,
#endif
};

#ifndef SQLITE_AMALGAMATION
/* IMPLEMENTATION-OF: R-46656-45156 The sqlite3_version[] string constant
** contains the text of SQLITE_VERSION macro.
*/
const char sqlite3_version[] = SQLITE_VERSION;
#endif

/* IMPLEMENTATION-OF: R-53536-42575 The sqlite3_libversion() function returns
** a pointer to the to the sqlite3_version[] string constant.
*/
const char *sqlite3_libversion(void){ 返回 sqlite3_version; }

/* IMPLEMENTATION-OF: R-25063-23286 The sqlite3_sourceid() function returns a
** pointer to a string constant whose value is the same as the
** SQLITE_SOURCE_ID C preprocessor macro. Except if SQLite is built using
** an edited copy of the amalgamation, then the last four characters of
** the hash might be different from SQLITE_SOURCE_ID.
*/
const char *sqlite3_sourceid(void){ 返回 SQLITE_SOURCE_ID; }

/* IMPLEMENTATION-OF: R-35210-63508 The sqlite3_libversion_number() function
** returns an integer equal to SQLITE_VERSION_NUMBER.
*/
整数 sqlite3_libversion_number(void){ 返回 SQLITE_VERSION_NUMBER; }

/* IMPLEMENTATION-OF: R-20790-14025 The sqlite3_threadsafe() function returns
** zero if and only if SQLite was compiled with mutexing code omitted due to
** the SQLITE_THREADSAFE compile-time option being set to 0.
*/
整数 sqlite3_threadsafe(void){ 返回 SQLITE_THREADSAFE; }

/*
** When compiling the test fixture or with debugging enabled (on Win32),
** this variable being set to non-zero will cause OSTRACE macros to emit
** extra diagnostic information.
*/
#ifdef SQLITE_HAVE_OS_TRACE
# ifndef SQLITE_DEBUG_OS_TRACE
#   define SQLITE_DEBUG_OS_TRACE 0
# endif
  整数 sqlite3OSTrace = SQLITE_DEBUG_OS_TRACE;
#endif

#如果 !defined(SQLITE_OMIT_TRACE) && defined(SQLITE_ENABLE_IOTRACE)
/*
** If the following function pointer is not NULL and if
** SQLITE_ENABLE_IOTRACE is enabled, then messages describing
** I/O active are written using this function.  These messages
** are intended for debugging activity only.
*/
SQLITE_API void (SQLITE_CDECL *sqlite3IoTrace)(const char*, ...) = 0;
#endif

/*
** If the following global variable points to a string which is the
** name of a directory, then that directory will be used to store
** temporary files.
**
** See also the "PRAGMA temp_store_directory" SQL command.
*/
char *sqlite3_temp_directory = 0;

/*
** If the following global variable points to a string which is the
** name of a directory, then that directory will be used to store
** all database files specified with a relative pathname.
**
** See also the "PRAGMA data_store_directory" SQL command.
*/
char *sqlite3_data_directory = 0;

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
static SQLITE_NOINLINE 整数 sqlite3Initialize(void){
  MUTEX_LOGIC( sqlite3_mutex *pMainMtx; )   /* The main static mutex */
  整数 rc;                                   /* Result code */
#ifdef SQLITE_EXTRA_INIT
  整数 bRunExtraInit = 0;                    /* Extra initialization needed */
#endif

  /* If the following assert() fails on some obscure processor/compiler
  ** combination to warn that SQLite has been mis-compiled.  If you hit
  ** this assert(), that means you need to recompile with the
  ** -DSQLITE_PTRSIZE=n compile-time option to set the correct pointer size.
  */
  assert( SQLITE_PTRSIZE==sizeof(char*) );

  /* Make sure the mutex subsystem is initialized.  If unable to
  ** initialize the mutex subsystem, return early with the error.
  ** If the system is so sick that we are unable to allocate a mutex,
  ** there is not much SQLite is going to be able to do.
  **
  ** The mutex subsystem must take care of serializing its own
  ** initialization.
  */
  rc = sqlite3MutexInit();
  如果( rc ) 返回 rc;

  /* Initialize the malloc() system and the recursive pInitMutex mutex.
  ** This operation is protected by the STATIC_MAIN mutex.  Note that
  ** MutexAlloc() is called for a static mutex prior to initializing the
  ** malloc subsystem - this implies that the allocation of a static
  ** mutex must not require support from the malloc subsystem.
  */
  MUTEX_LOGIC( pMainMtx = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  sqlite3_mutex_enter(pMainMtx);
  sqlite3GlobalConfig.isMutexInit = 1;
  如果( !sqlite3GlobalConfig.isMallocInit ){
    rc = sqlite3MallocInit();
  }
  如果( rc==SQLITE_OK ){
    sqlite3GlobalConfig.isMallocInit = 1;
    如果( !sqlite3GlobalConfig.pInitMutex ){
      sqlite3GlobalConfig.pInitMutex =
           sqlite3MutexAlloc(SQLITE_MUTEX_RECURSIVE);
      如果( sqlite3GlobalConfig.bCoreMutex && !sqlite3GlobalConfig.pInitMutex ){
        rc = SQLITE_NOMEM_BKPT;
      }
    }
  }
  如果( rc==SQLITE_OK ){
    sqlite3GlobalConfig.nRefInitMutex++;
  }
  sqlite3_mutex_leave(pMainMtx);

  /* If rc is not SQLITE_OK at this point, then either the malloc
  ** subsystem could not be initialized or the system failed to allocate
  ** the pInitMutex mutex. Return an error in either case.  */
  如果( rc!=SQLITE_OK ){
    返回 rc;
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
  如果( sqlite3GlobalConfig.isInit==0 && sqlite3GlobalConfig.inProgress==0 ){
    sqlite3GlobalConfig.inProgress = 1;
#ifdef SQLITE_ENABLE_SQLLOG
    {
      extern void sqlite3_init_sqllog(void);
      sqlite3_init_sqllog();
    }
#endif
    memset(&sqlite3BuiltinFunctions, 0, sizeof(sqlite3BuiltinFunctions));
    sqlite3RegisterBuiltinFunctions();
    如果( sqlite3GlobalConfig.isPCacheInit==0 ){
      rc = sqlite3PcacheInitialize();
    }
    如果( rc==SQLITE_OK ){
      sqlite3GlobalConfig.isPCacheInit = 1;
      rc = sqlite3OsInit();
    }
#ifndef SQLITE_OMIT_DESERIALIZE
    如果( rc==SQLITE_OK ){
      rc = sqlite3MemdbInit();
    }
#endif
    如果( rc==SQLITE_OK ){
      sqlite3PCacheBufferSetup( sqlite3GlobalConfig.pPage,
          sqlite3GlobalConfig.szPage, sqlite3GlobalConfig.nPage);
#ifdef SQLITE_EXTRA_INIT_MUTEXED
      {
        整数 SQLITE_EXTRA_INIT_MUTEXED(const char*);
        rc = SQLITE_EXTRA_INIT_MUTEXED(0);
      }
#endif
    }
    如果( rc==SQLITE_OK ){
      sqlite3MemoryBarrier();
      sqlite3GlobalConfig.isInit = 1;
#ifdef SQLITE_EXTRA_INIT
      bRunExtraInit = 1;
#endif
    }
    sqlite3GlobalConfig.inProgress = 0;
  }
  sqlite3_mutex_leave(sqlite3GlobalConfig.pInitMutex);

  /* Go back under the static mutex and clean up the recursive
  ** mutex to prevent a resource leak.
  */
  sqlite3_mutex_enter(pMainMtx);
  sqlite3GlobalConfig.nRefInitMutex--;
  如果( sqlite3GlobalConfig.nRefInitMutex<=0 ){
    assert( sqlite3GlobalConfig.nRefInitMutex==0 );
    sqlite3_mutex_free(sqlite3GlobalConfig.pInitMutex);
    sqlite3GlobalConfig.pInitMutex = 0;
  }
  sqlite3_mutex_leave(pMainMtx);

  /* The following is just a sanity check to make sure SQLite has
  ** been compiled correctly.  It is important to run this code, but
  ** we don't want to run it too often and soak up CPU cycles for no
  ** reason.  So we run it once during initialization.
  */
#ifndef NDEBUG
#ifndef SQLITE_OMIT_FLOATING_POINT
  /* This section of code's only "output" is via assert() statements. */
  如果( rc==SQLITE_OK ){
    u64 x = (((u64)1)<<63)-1;
    double y;
    assert(sizeof(x)==8);
    assert(sizeof(x)==sizeof(y));
    memcpy(&y, &x, 8);
    assert( sqlite3IsNaN(y) );
  }
#endif
#endif

  /* Do extra initialization steps requested by the SQLITE_EXTRA_INIT
  ** compile-time option.
  */
#ifdef SQLITE_EXTRA_INIT
  如果( bRunExtraInit ){
    整数 SQLITE_EXTRA_INIT(const char*);
    rc = SQLITE_EXTRA_INIT(0);
  }
#endif
  返回 rc;
}
SQLITE_API 整数 sqlite3_initialize(void){
  /* If this build does not support writable static data (WSD) natively
  ** then we have to invoke the (application-supplied) WSD initialization
  ** routine before doing anything else. */
#ifdef SQLITE_OMIT_WSD
  整数 rc = sqlite3_wsd_init(4096, 24);
  如果( rc!=SQLITE_OK ){
    返回 rc;
  }
#endif

  如果( sqlite3GlobalConfig.isInit ){
    /* SQLite has already been initialized.  Fast early-out. */
    sqlite3MemoryBarrier();
    返回 SQLITE_OK;
  }否则{
    /* Invoke sqlite3Initialize() to do the actual work. */
    返回 sqlite3Initialize();
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
整数 sqlite3_shutdown(void){
#ifdef SQLITE_OMIT_WSD
  整数 rc = sqlite3_wsd_init(4096, 24);
  如果( rc!=SQLITE_OK ){
    返回 rc;
  }
#endif

  如果( sqlite3GlobalConfig.isInit ){
#ifdef SQLITE_EXTRA_SHUTDOWN
    void SQLITE_EXTRA_SHUTDOWN(void);
    SQLITE_EXTRA_SHUTDOWN();
#endif
    sqlite3_os_end();
    sqlite3_reset_auto_extension();
    sqlite3GlobalConfig.isInit = 0;
  }
  如果( sqlite3GlobalConfig.isPCacheInit ){
    sqlite3PcacheShutdown();
    sqlite3GlobalConfig.isPCacheInit = 0;
  }
  如果( sqlite3GlobalConfig.isMallocInit ){
    sqlite3MallocEnd();
    sqlite3GlobalConfig.isMallocInit = 0;

#ifndef SQLITE_OMIT_SHUTDOWN_DIRECTORIES
    /* The heap subsystem has now been shutdown and these values are supposed
    ** to be NULL or point to memory that was obtained from sqlite3_malloc(),
    ** which would rely on that heap subsystem; therefore, make sure these
    ** values cannot refer to heap memory that was just invalidated when the
    ** heap subsystem was shutdown.  This is only done if the current call to
    ** this function resulted in the heap subsystem actually being shutdown.
    */
    sqlite3_data_directory = 0;
    sqlite3_temp_directory = 0;
#endif
  }
  如果( sqlite3GlobalConfig.isMutexInit ){
    sqlite3MutexEnd();
    sqlite3GlobalConfig.isMutexInit = 0;
  }

  返回 SQLITE_OK;
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
整数 sqlite3_config(整数 op, ...){
  va_list ap;
  整数 rc = SQLITE_OK;

  /* sqlite3_config() normally returns SQLITE_MISUSE if it is invoked while
  ** the SQLite library is in use.  Except, a few selected opcodes
  ** are allowed.
  */
  如果( sqlite3GlobalConfig.isInit ){
    static const u64 mAnytimeConfigOption = 0
       | MASKBIT64( SQLITE_CONFIG_LOG )
       | MASKBIT64( SQLITE_CONFIG_PCACHE_HDRSZ )
    ;
    如果( op<0 || op>63 || (MASKBIT64(op) & mAnytimeConfigOption)==0 ){
      返回 SQLITE_MISUSE_BKPT;
    }
    testcase( op==SQLITE_CONFIG_LOG );
    testcase( op==SQLITE_CONFIG_PCACHE_HDRSZ );
  }

  va_start(ap, op);
  switch( op ){

    /* Mutex configuration options are only available in a threadsafe
    ** compile.
    */
#如果 defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE>0  /* IMP: R-54466-46756 */
    case SQLITE_CONFIG_SINGLETHREAD: {
      /* EVIDENCE-OF: R-02748-19096 This option sets the threading mode to
      ** Single-thread. */
      sqlite3GlobalConfig.bCoreMutex = 0;  /* Disable mutex on core */
      sqlite3GlobalConfig.bFullMutex = 0;  /* Disable mutex on connections */
      break;
    }
#endif
#如果 defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE>0 /* IMP: R-20520-54086 */
    case SQLITE_CONFIG_MULTITHREAD: {
      /* EVIDENCE-OF: R-14374-42468 This option sets the threading mode to
      ** Multi-thread. */
      sqlite3GlobalConfig.bCoreMutex = 1;  /* Enable mutex on core */
      sqlite3GlobalConfig.bFullMutex = 0;  /* Disable mutex on connections */
      break;
    }
#endif
#如果 defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE>0 /* IMP: R-59593-21810 */
    case SQLITE_CONFIG_SERIALIZED: {
      /* EVIDENCE-OF: R-41220-51800 This option sets the threading mode to
      ** Serialized. */
      sqlite3GlobalConfig.bCoreMutex = 1;  /* Enable mutex on core */
      sqlite3GlobalConfig.bFullMutex = 1;  /* Enable mutex on connections */
      break;
    }
#endif
#如果 defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE>0 /* IMP: R-63666-48755 */
    case SQLITE_CONFIG_MUTEX: {
      /* Specify an alternative mutex implementation */
      sqlite3GlobalConfig.mutex = *va_arg(ap, sqlite3_mutex_methods*);
      break;
    }
#endif
#如果 defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE>0 /* IMP: R-14450-37597 */
    case SQLITE_CONFIG_GETMUTEX: {
      /* Retrieve the current mutex implementation */
      *va_arg(ap, sqlite3_mutex_methods*) = sqlite3GlobalConfig.mutex;
      break;
    }
#endif

    case SQLITE_CONFIG_MALLOC: {
      /* EVIDENCE-OF: R-55594-21030 The SQLITE_CONFIG_MALLOC option takes a
      ** single argument which is a pointer to an instance of the
      ** sqlite3_mem_methods structure. The argument specifies alternative
      ** low-level memory allocation routines to be used in place of the memory
      ** allocation routines built into SQLite. */
      sqlite3GlobalConfig.m = *va_arg(ap, sqlite3_mem_methods*);
      break;
    }
    case SQLITE_CONFIG_GETMALLOC: {
      /* EVIDENCE-OF: R-51213-46414 The SQLITE_CONFIG_GETMALLOC option takes a
      ** single argument which is a pointer to an instance of the
      ** sqlite3_mem_methods structure. The sqlite3_mem_methods structure is
      ** filled with the currently defined memory allocation routines. */
      如果( sqlite3GlobalConfig.m.xMalloc==0 ) sqlite3MemSetDefault();
      *va_arg(ap, sqlite3_mem_methods*) = sqlite3GlobalConfig.m;
      break;
    }
    case SQLITE_CONFIG_MEMSTATUS: {
      assert( !sqlite3GlobalConfig.isInit );  /* Cannot change at runtime */
      /* EVIDENCE-OF: R-61275-35157 The SQLITE_CONFIG_MEMSTATUS option takes
      ** single argument of type int, interpreted as a boolean, which enables
      ** or disables the collection of memory allocation statistics. */
      sqlite3GlobalConfig.bMemstat = va_arg(ap, 整数);
      break;
    }
    case SQLITE_CONFIG_SMALL_MALLOC: {
      sqlite3GlobalConfig.bSmallMalloc = va_arg(ap, 整数)!=0;
      break;
    }
    case SQLITE_CONFIG_PAGECACHE: {
      /* EVIDENCE-OF: R-18761-36601 There are three arguments to
      ** SQLITE_CONFIG_PAGECACHE: A pointer to 8-byte aligned memory (pMem),
      ** the size of each page cache line (sz), and the number of cache lines
      ** (N). */
      sqlite3GlobalConfig.pPage = va_arg(ap, void*);
      sqlite3GlobalConfig.szPage = va_arg(ap, 整数);
      sqlite3GlobalConfig.nPage = va_arg(ap, 整数);
      break;
    }
    case SQLITE_CONFIG_PCACHE_HDRSZ: {
      /* EVIDENCE-OF: R-39100-27317 The SQLITE_CONFIG_PCACHE_HDRSZ option takes
      ** a single parameter which is a pointer to an integer and writes into
      ** that integer the number of extra bytes per page required for each page
      ** in SQLITE_CONFIG_PAGECACHE. */
      *va_arg(ap, 整数*) =
          sqlite3HeaderSizeBtree() +
          sqlite3HeaderSizePcache() +
          sqlite3HeaderSizePcache1();
      break;
    }

    case SQLITE_CONFIG_PCACHE: {
      /* no-op */
      break;
    }
    case SQLITE_CONFIG_GETPCACHE: {
      /* now an error */
      rc = SQLITE_ERROR;
      break;
    }

    case SQLITE_CONFIG_PCACHE2: {
      /* EVIDENCE-OF: R-63325-48378 The SQLITE_CONFIG_PCACHE2 option takes a
      ** single argument which is a pointer to an sqlite3_pcache_methods2
      ** object. This object specifies the interface to a custom page cache
      ** implementation. */
      sqlite3GlobalConfig.pcache2 = *va_arg(ap, sqlite3_pcache_methods2*);
      break;
    }
    case SQLITE_CONFIG_GETPCACHE2: {
      /* EVIDENCE-OF: R-22035-46182 The SQLITE_CONFIG_GETPCACHE2 option takes a
      ** single argument which is a pointer to an sqlite3_pcache_methods2
      ** object. SQLite copies of the current page cache implementation into
      ** that object. */
      如果( sqlite3GlobalConfig.pcache2.xInit==0 ){
        sqlite3PCacheSetDefault();
      }
      *va_arg(ap, sqlite3_pcache_methods2*) = sqlite3GlobalConfig.pcache2;
      break;
    }

/* EVIDENCE-OF: R-06626-12911 The SQLITE_CONFIG_HEAP option is only
** available if SQLite is compiled with either SQLITE_ENABLE_MEMSYS3 or
** SQLITE_ENABLE_MEMSYS5 and returns SQLITE_ERROR if invoked otherwise. */
#如果 defined(SQLITE_ENABLE_MEMSYS3) || defined(SQLITE_ENABLE_MEMSYS5)
    case SQLITE_CONFIG_HEAP: {
      /* EVIDENCE-OF: R-19854-42126 There are three arguments to
      ** SQLITE_CONFIG_HEAP: An 8-byte aligned pointer to the memory, the
      ** number of bytes in the memory buffer, and the minimum allocation size.
      */
      sqlite3GlobalConfig.pHeap = va_arg(ap, void*);
      sqlite3GlobalConfig.nHeap = va_arg(ap, 整数);
      sqlite3GlobalConfig.mnReq = va_arg(ap, 整数);

      如果( sqlite3GlobalConfig.mnReq<1 ){
        sqlite3GlobalConfig.mnReq = 1;
      }否则 如果( sqlite3GlobalConfig.mnReq>(1<<12) ){
        /* cap min request size at 2^12 */
        sqlite3GlobalConfig.mnReq = (1<<12);
      }

      如果( sqlite3GlobalConfig.pHeap==0 ){
        /* EVIDENCE-OF: R-49920-60189 If the first pointer (the memory pointer)
        ** is NULL, then SQLite reverts to using its default memory allocator
        ** (the system malloc() implementation), undoing any prior invocation of
        ** SQLITE_CONFIG_MALLOC.
        **
        ** Setting sqlite3GlobalConfig.m to all zeros will cause malloc to
        ** revert to its default implementation when sqlite3_initialize() is run
        */
        memset(&sqlite3GlobalConfig.m, 0, sizeof(sqlite3GlobalConfig.m));
      }否则{
        /* EVIDENCE-OF: R-61006-08918 If the memory pointer is not NULL then the
        ** alternative memory allocator is engaged to handle all of SQLites
        ** memory allocation needs. */
#ifdef SQLITE_ENABLE_MEMSYS3
        sqlite3GlobalConfig.m = *sqlite3MemGetMemsys3();
#endif
#ifdef SQLITE_ENABLE_MEMSYS5
        sqlite3GlobalConfig.m = *sqlite3MemGetMemsys5();
#endif
      }
      break;
    }
#endif

    case SQLITE_CONFIG_LOOKASIDE: {
      sqlite3GlobalConfig.szLookaside = va_arg(ap, 整数);
      sqlite3GlobalConfig.nLookaside = va_arg(ap, 整数);
      break;
    }
   
    /* Record a pointer to the logger function and its first argument.
    ** The default is NULL.  Logging is disabled if the function pointer is
    ** NULL.
    */
    case SQLITE_CONFIG_LOG: {
      /* MSVC is picky about pulling func ptrs from va lists.
      ** http://support.microsoft.com/kb/47961
      ** sqlite3GlobalConfig.xLog = va_arg(ap, void(*)(void*,int,const char*));
      */
      typedef void(*LOGFUNC_t)(void*,整数,const char*);
      LOGFUNC_t xLog = va_arg(ap, LOGFUNC_t);
      void *pLogArg = va_arg(ap, void*);
      AtomicStore(&sqlite3GlobalConfig.xLog, xLog);
      AtomicStore(&sqlite3GlobalConfig.pLogArg, pLogArg);
      break;
    }

    /* EVIDENCE-OF: R-55548-33817 The compile-time setting for URI filenames
    ** can be changed at start-time using the
    ** sqlite3_config(SQLITE_CONFIG_URI,1) or
    ** sqlite3_config(SQLITE_CONFIG_URI,0) configuration calls.
    */
    case SQLITE_CONFIG_URI: {
      /* EVIDENCE-OF: R-25451-61125 The SQLITE_CONFIG_URI option takes a single
      ** argument of type int. If non-zero, then URI handling is globally
      ** enabled. If the parameter is zero, then URI handling is globally
      ** disabled. */
      整数 bOpenUri = va_arg(ap, 整数);
      AtomicStore(&sqlite3GlobalConfig.bOpenUri, bOpenUri);
      break;
    }

    case SQLITE_CONFIG_COVERING_INDEX_SCAN: {
      /* EVIDENCE-OF: R-36592-02772 The SQLITE_CONFIG_COVERING_INDEX_SCAN
      ** option takes a single integer argument which is interpreted as a
      ** boolean in order to enable or disable the use of covering indices for
      ** full table scans in the query optimizer. */
      sqlite3GlobalConfig.bUseCis = va_arg(ap, 整数);
      break;
    }

#ifdef SQLITE_ENABLE_SQLLOG
    case SQLITE_CONFIG_SQLLOG: {
      typedef void(*SQLLOGFUNC_t)(void*, sqlite3*, const char*, 整数);
      sqlite3GlobalConfig.xSqllog = va_arg(ap, SQLLOGFUNC_t);
      sqlite3GlobalConfig.pSqllogArg = va_arg(ap, void *);
      break;
    }
#endif

    case SQLITE_CONFIG_MMAP_SIZE: {
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
      如果( mxMmap<0 || mxMmap>SQLITE_MAX_MMAP_SIZE ){
        mxMmap = SQLITE_MAX_MMAP_SIZE;
      }
      如果( szMmap<0 ) szMmap = SQLITE_DEFAULT_MMAP_SIZE;
      如果( szMmap>mxMmap) szMmap = mxMmap;
      sqlite3GlobalConfig.mxMmap = mxMmap;
      sqlite3GlobalConfig.szMmap = szMmap;
      break;
    }

#如果 SQLITE_OS_WIN && defined(SQLITE_WIN32_MALLOC) /* IMP: R-04780-55815 */
    case SQLITE_CONFIG_WIN32_HEAPSIZE: {
      /* EVIDENCE-OF: R-34926-03360 SQLITE_CONFIG_WIN32_HEAPSIZE takes a 32-bit
      ** unsigned integer value that specifies the maximum size of the created
      ** heap. */
      sqlite3GlobalConfig.nHeap = va_arg(ap, 整数);
      break;
    }
#endif

    case SQLITE_CONFIG_PMASZ: {
      sqlite3GlobalConfig.szPma = va_arg(ap, unsigned 整数);
      break;
    }

    case SQLITE_CONFIG_STMTJRNL_SPILL: {
      sqlite3GlobalConfig.nStmtSpill = va_arg(ap, 整数);
      break;
    }

#ifdef SQLITE_ENABLE_SORTER_REFERENCES
    case SQLITE_CONFIG_SORTERREF_SIZE: {
      整数 iVal = va_arg(ap, 整数);
      如果( iVal<0 ){
        iVal = SQLITE_DEFAULT_SORTERREF_SIZE;
      }
      sqlite3GlobalConfig.szSorterRef = (u32)iVal;
      break;
    }
#endif /* SQLITE_ENABLE_SORTER_REFERENCES */

#ifndef SQLITE_OMIT_DESERIALIZE
    case SQLITE_CONFIG_MEMDB_MAXSIZE: {
      sqlite3GlobalConfig.mxMemdbSize = va_arg(ap, sqlite3_int64);
      break;
    }
#endif /* SQLITE_OMIT_DESERIALIZE */

    case SQLITE_CONFIG_ROWID_IN_VIEW: {
      整数 *pVal = va_arg(ap,整数*);
#ifdef SQLITE_ALLOW_ROWID_IN_VIEW
      如果( 0==*pVal ) sqlite3GlobalConfig.mNoVisibleRowid = TF_NoVisibleRowid;
      如果( 1==*pVal ) sqlite3GlobalConfig.mNoVisibleRowid = 0;
      *pVal = (sqlite3GlobalConfig.mNoVisibleRowid==0);
#否则
      *pVal = 0;
#endif
      break;
    }

    default: {
      rc = SQLITE_ERROR;
      break;
    }
  }
  va_end(ap);
  返回 rc;
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
static 整数 setupLookaside(
  sqlite3 *db,    /* Database connection being configured */
  void *pBuf,     /* Memory to use for lookaside.  May be NULL */
  整数 sz,         /* Desired size of each lookaside memory slot */
  整数 cnt         /* Number of slots to allocate */
){
#ifndef SQLITE_OMIT_LOOKASIDE
  void *pStart;          /* Start of the lookaside buffer */
  sqlite3_int64 szAlloc; /* Total space set aside for lookaside memory */
  整数 nBig;              /* Number of full-size slots */
  整数 nSm;               /* Number smaller LOOKASIDE_SMALL-byte slots */
 
  如果( sqlite3LookasideUsed(db,0)>0 ){
    返回 SQLITE_BUSY;
  }
  /* Free any existing lookaside buffer for this handle before
  ** allocating a new one so we don't have to have space for
  ** both at the same time.
  */
  如果( db->lookaside.bMalloced ){
    sqlite3_free(db->lookaside.pStart);
  }
  /* The size of a lookaside slot after ROUNDDOWN8 needs to be larger
  ** than a pointer and small enough to fit in a u16.
  */
  sz = ROUNDDOWN8(sz);
  如果( sz<=(整数)sizeof(LookasideSlot*) ) sz = 0;
  如果( sz>65528 ) sz = 65528;
  /* Count must be at least 1 to be useful, but not so large as to use
  ** more than 0x7fff0000 total bytes for lookaside. */
  如果( cnt<1 ) cnt = 0;
  如果( sz>0 && cnt>(0x7fff0000/sz) ) cnt = 0x7fff0000/sz;
  szAlloc = (i64)sz*(i64)cnt;
  如果( szAlloc==0 ){
    sz = 0;
    pStart = 0;
  }否则 如果( pBuf==0 ){
    sqlite3BeginBenignMalloc();
    pStart = sqlite3Malloc( szAlloc );
    sqlite3EndBenignMalloc();
    如果( pStart ) szAlloc = sqlite3MallocSize(pStart);
  }否则{
    pStart = pBuf;
  }
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
  如果( sz>=LOOKASIDE_SMALL*3 ){
    nBig = szAlloc/(3*LOOKASIDE_SMALL+sz);
    nSm = (szAlloc - (i64)sz*(i64)nBig)/LOOKASIDE_SMALL;
  }否则 如果( sz>=LOOKASIDE_SMALL*2 ){
    nBig = szAlloc/(LOOKASIDE_SMALL+sz);
    nSm = (szAlloc - (i64)sz*(i64)nBig)/LOOKASIDE_SMALL;
  }否则
#endif /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
  如果( sz>0 ){
    nBig = szAlloc/sz;
    nSm = 0;
  }否则{
    nBig = nSm = 0;
  }
  db->lookaside.pStart = pStart;
  db->lookaside.pInit = 0;
  db->lookaside.pFree = 0;
  db->lookaside.sz = (u16)sz;
  db->lookaside.szTrue = (u16)sz;
  如果( pStart ){
    整数 i;
    LookasideSlot *p;
    assert( sz > (整数)sizeof(LookasideSlot*) );
    p = (LookasideSlot*)pStart;
    为(i=0; i<nBig; i++){
      p->pNext = db->lookaside.pInit;
      db->lookaside.pInit = p;
      p = (LookasideSlot*)&((u8*)p)[sz];
    }
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
    db->lookaside.pSmallInit = 0;
    db->lookaside.pSmallFree = 0;
    db->lookaside.pMiddle = p;
    为(i=0; i<nSm; i++){
      p->pNext = db->lookaside.pSmallInit;
      db->lookaside.pSmallInit = p;
      p = (LookasideSlot*)&((u8*)p)[LOOKASIDE_SMALL];
    }
#endif /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
    assert( ((uptr)p)<=szAlloc + (uptr)pStart );
    db->lookaside.pEnd = p;
    db->lookaside.bDisable = 0;
    db->lookaside.bMalloced = pBuf==0 ?1:0;
    db->lookaside.nSlot = nBig+nSm;
  }否则{
    db->lookaside.pStart = 0;
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
    db->lookaside.pSmallInit = 0;
    db->lookaside.pSmallFree = 0;
    db->lookaside.pMiddle = 0;
#endif /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
    db->lookaside.pEnd = 0;
    db->lookaside.bDisable = 1;
    db->lookaside.sz = 0;
    db->lookaside.bMalloced = 0;
    db->lookaside.nSlot = 0;
  }
  db->lookaside.pTrueEnd = db->lookaside.pEnd;
  assert( sqlite3LookasideUsed(db,0)==0 );
#endif /* SQLITE_OMIT_LOOKASIDE */
  返回 SQLITE_OK;
}

/*
** Return the mutex associated with a database connection.
*/
sqlite3_mutex *sqlite3_db_mutex(sqlite3 *db){
#ifdef SQLITE_ENABLE_API_ARMOR
  如果( !sqlite3SafetyCheckOk(db) ){
    (void)SQLITE_MISUSE_BKPT;
    返回 0;
  }
#endif
  返回 db->mutex;
}

/*
** Free up as much memory as we can from the given database
** connection.
*/
整数 sqlite3_db_release_memory(sqlite3 *db){
  整数 i;

#ifdef SQLITE_ENABLE_API_ARMOR
  如果( !sqlite3SafetyCheckOk(db) ) 返回 SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(db->mutex);
  sqlite3BtreeEnterAll(db);
  为(i=0; i<db->nDb; i++){
    Btree *pBt = db->aDb[i].pBt;
    如果( pBt ){
      Pager *pPager = sqlite3BtreePager(pBt);
      sqlite3PagerShrink(pPager);
    }
  }
  sqlite3BtreeLeaveAll(db);
  sqlite3_mutex_leave(db->mutex);
  返回 SQLITE_OK;
}

/*
** Flush any dirty pages in the pager-cache for any attached database
** to disk.
*/
整数 sqlite3_db_cacheflush(sqlite3 *db){
  整数 i;
  整数 rc = SQLITE_OK;
  整数 bSeenBusy = 0;

#ifdef SQLITE_ENABLE_API_ARMOR
  如果( !sqlite3SafetyCheckOk(db) ) 返回 SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(db->mutex);
  sqlite3BtreeEnterAll(db);
  为(i=0; rc==SQLITE_OK && i<db->nDb; i++){
    Btree *pBt = db->aDb[i].pBt;
    如果( pBt && sqlite3BtreeTxnState(pBt)==SQLITE_TXN_WRITE ){
      Pager *pPager = sqlite3BtreePager(pBt);
      rc = sqlite3PagerFlush(pPager);
      如果( rc==SQLITE_BUSY ){
        bSeenBusy = 1;
        rc = SQLITE_OK;
      }
    }
  }
  sqlite3BtreeLeaveAll(db);
  sqlite3_mutex_leave(db->mutex);
  返回 ((rc==SQLITE_OK && bSeenBusy) ? SQLITE_BUSY : rc);
}

/*
** Configuration settings for an individual database connection
*/
整数 sqlite3_db_config(sqlite3 *db, 整数 op, ...){
  va_list ap;
  整数 rc;

#ifdef SQLITE_ENABLE_API_ARMOR
  如果( !sqlite3SafetyCheckOk(db) ) 返回 SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(db->mutex);
  va_start(ap, op);
  switch( op ){
    case SQLITE_DBCONFIG_MAINDBNAME: {
      /* IMP: R-06824-28531 */
      /* IMP: R-36257-52125 */
      db->aDb[0].zDbSName = va_arg(ap,char*);
      rc = SQLITE_OK;
      break;
    }
    case SQLITE_DBCONFIG_LOOKASIDE: {
      void *pBuf = va_arg(ap, void*); /* IMP: R-26835-10964 */
      整数 sz = va_arg(ap, 整数);       /* IMP: R-47871-25994 */
      整数 cnt = va_arg(ap, 整数);      /* IMP: R-04460-53386 */
      rc = setupLookaside(db, pBuf, sz, cnt);
      break;
    }
    case SQLITE_DBCONFIG_FP_DIGITS: {
      整数 nIn = va_arg(ap, 整数);
      整数 *pOut = va_arg(ap, 整数*);
      如果( nIn>3 && nIn<24 ) db->nFpDigit = (u8)nIn;
      如果( pOut ) *pOut = db->nFpDigit;
      rc = SQLITE_OK;
      break;
    }
    default: {
      static const struct {
        整数 op;      /* The opcode */
        u64 mask;    /* Mask of the bit in sqlite3.flags to set/clear */
      } aFlagOp[] = {
        { SQLITE_DBCONFIG_ENABLE_FKEY,           SQLITE_ForeignKeys    },
        { SQLITE_DBCONFIG_ENABLE_TRIGGER,        SQLITE_EnableTrigger  },
        { SQLITE_DBCONFIG_ENABLE_VIEW,           SQLITE_EnableView     },
        { SQLITE_DBCONFIG_ENABLE_FTS3_TOKENIZER, SQLITE_Fts3Tokenizer  },
        { SQLITE_DBCONFIG_ENABLE_LOAD_EXTENSION, SQLITE_LoadExtension  },
        { SQLITE_DBCONFIG_NO_CKPT_ON_CLOSE,      SQLITE_NoCkptOnClose  },
        { SQLITE_DBCONFIG_ENABLE_QPSG,           SQLITE_EnableQPSG     },
        { SQLITE_DBCONFIG_TRIGGER_EQP,           SQLITE_TriggerEQP     },
        { SQLITE_DBCONFIG_RESET_DATABASE,        SQLITE_ResetDatabase  },
        { SQLITE_DBCONFIG_DEFENSIVE,             SQLITE_Defensive      },
        { SQLITE_DBCONFIG_WRITABLE_SCHEMA,       SQLITE_WriteSchema|
                                                 SQLITE_NoSchemaError  },
        { SQLITE_DBCONFIG_LEGACY_ALTER_TABLE,    SQLITE_LegacyAlter    },
        { SQLITE_DBCONFIG_DQS_DDL,               SQLITE_DqsDDL         },
        { SQLITE_DBCONFIG_DQS_DML,               SQLITE_DqsDML         },
        { SQLITE_DBCONFIG_LEGACY_FILE_FORMAT,    SQLITE_LegacyFileFmt  },
        { SQLITE_DBCONFIG_TRUSTED_SCHEMA,        SQLITE_TrustedSchema  },
        { SQLITE_DBCONFIG_STMT_SCANSTATUS,       SQLITE_StmtScanStatus },
        { SQLITE_DBCONFIG_REVERSE_SCANORDER,     SQLITE_ReverseOrder   },
        { SQLITE_DBCONFIG_ENABLE_ATTACH_CREATE,  SQLITE_AttachCreate   },
        { SQLITE_DBCONFIG_ENABLE_ATTACH_WRITE,   SQLITE_AttachWrite    },
        { SQLITE_DBCONFIG_ENABLE_COMMENTS,       SQLITE_Comments       },
      };
      unsigned 整数 i;
      rc = SQLITE_ERROR; /* IMP: R-42790-23372 */
      为(i=0; i<ArraySize(aFlagOp); i++){
        如果( aFlagOp[i].op==op ){
          整数 onoff = va_arg(ap, 整数);
          整数 *pRes = va_arg(ap, 整数*);
          u64 oldFlags = db->flags;
          如果( onoff>0 ){
            db->flags |= aFlagOp[i].mask;
          }否则 如果( onoff==0 ){
            db->flags &= ~(u64)aFlagOp[i].mask;
          }
          如果( oldFlags!=db->flags ){
            sqlite3ExpirePreparedStatements(db, 0);
          }
          如果( pRes ){
            *pRes = (db->flags & aFlagOp[i].mask)!=0;
          }
          rc = SQLITE_OK;
          break;
        }
      }
      break;
    }
  }
  va_end(ap);
  sqlite3_mutex_leave(db->mutex);
  返回 rc;
}

/*
** This is the default collating function named "BINARY" which is always
** available.
*/
整数 sqlite3BinaryCompare(
  void *NotUsed,
  整数 nKey1, const void *pKey1,
  整数 nKey2, const void *pKey2
){
  整数 rc, n;
  UNUSED_PARAMETER(NotUsed);
  n = nKey1<nKey2 ? nKey1 : nKey2;
  /* EVIDENCE-OF: R-65033-28449 The built-in BINARY collation compares
  ** strings byte by byte using the memcmp() function from the standard C
  ** library. */
  assert( pKey1 && pKey2 );
  rc = memcmp(pKey1, pKey2, n);
  如果( rc==0 ){
    rc = nKey1 - nKey2;
  }
  返回 rc;
}

/*
** This is the collating function named "RTRIM" which is always
** available.  Ignore trailing spaces.
*/
static 整数 rtrimCollFunc(
  void *pUser,
  整数 nKey1, const void *pKey1,
  整数 nKey2, const void *pKey2
){
  const u8 *pK1 = (const u8*)pKey1;
  const u8 *pK2 = (const u8*)pKey2;
  当( nKey1 && pK1[nKey1-1]==' ' ) nKey1--;
  当( nKey2 && pK2[nKey2-1]==' ' ) nKey2--;
  返回 sqlite3BinaryCompare(pUser, nKey1, pKey1, nKey2, pKey2);
}

/*
** Return true if CollSeq is the default built-in BINARY.
*/
#如果 !SQLITE_USES_INLINE
整数 sqlite3IsBinary(const CollSeq *p){
  assert( p==0 || p->xCmp!=sqlite3BinaryCompare
       || strcmp(p->zName,"BINARY")==0 );
  返回 p==0 || p->xCmp==sqlite3BinaryCompare;
}
#endif

/*
** Another built-in collating sequence: NOCASE.
**
** This collating sequence is intended to be used for "case independent
** comparison". SQLite's knowledge of upper and lower case equivalents
** extends only to the 26 characters used in the English language.
**
** At the moment there is only a UTF-8 implementation.
*/
static 整数 nocaseCollatingFunc(
  void *NotUsed,
  整数 nKey1, const void *pKey1,
  整数 nKey2, const void *pKey2
){
  整数 r = sqlite3StrNICmp(
      (const char *)pKey1, (const char *)pKey2, (nKey1<nKey2)?nKey1:nKey2);
  UNUSED_PARAMETER(NotUsed);
  如果( 0==r ){
    r = nKey1-nKey2;
  }
  返回 r;
}

/*
** Return the ROWID of the most recent insert
*/
sqlite_int64 sqlite3_last_insert_rowid(sqlite3 *db){
  i64 iRet;
#ifdef SQLITE_ENABLE_API_ARMOR
  如果( !sqlite3SafetyCheckOk(db) ){
    (void)SQLITE_MISUSE_BKPT;
    返回 0;
  }
#endif
  sqlite3_mutex_enter(db->mutex);
  iRet = db->lastRowid;
  sqlite3_mutex_leave(db->mutex);
  返回 iRet;
}

/*
** Set the value returned by the sqlite3_last_insert_rowid() API function.
*/
void sqlite3_set_last_insert_rowid(sqlite3 *db, sqlite3_int64 iRowid){
#ifdef SQLITE_ENABLE_API_ARMOR
  如果( !sqlite3SafetyCheckOk(db) ){
    (void)SQLITE_MISUSE_BKPT;
    返回;
  }
#endif
  sqlite3_mutex_enter(db->mutex);
  db->lastRowid = iRowid;
  sqlite3_mutex_leave(db->mutex);
}

/*
** Return the number of changes in the most recently executed DML
** statement.
*/
sqlite3_int64 sqlite3_changes64(sqlite3 *db){
  i64 iRet;
#ifdef SQLITE_ENABLE_API_ARMOR
  如果( !sqlite3SafetyCheckOk(db) ){
    (void)SQLITE_MISUSE_BKPT;
    返回 0;
  }
#endif
  sqlite3_mutex_enter(db->mutex);
  iRet = db->nChange;
  sqlite3_mutex_leave(db->mutex);
  返回 iRet;
}
整数 sqlite3_changes(sqlite3 *db){
  返回 (整数)sqlite3_changes64(db);
}

/*
** Return the number of changes since the database handle was opened.
*/
sqlite3_int64 sqlite3_total_changes64(sqlite3 *db){
  i64 iRet;
#ifdef SQLITE_ENABLE_API_ARMOR
  如果( !sqlite3SafetyCheckOk(db) ){
    (void)SQLITE_MISUSE_BKPT;
    返回 0;
  }
#endif
  sqlite3_mutex_enter(db->mutex);
  iRet = db->nTotalChange;
  sqlite3_mutex_leave(db->mutex);
  返回 iRet;
}
整数 sqlite3_total_changes(sqlite3 *db){
  返回 (整数)sqlite3_total_changes64(db);
}

/*
** Close all open savepoints. This function only manipulates fields of the
** database handle object, it does not close any savepoints that may be open
** at the b-tree/pager level.
*/
void sqlite3CloseSavepoints(sqlite3 *db){
  当( db->pSavepoint ){
    Savepoint *pTmp = db->pSavepoint;
    db->pSavepoint = pTmp->pNext;
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
static void functionDestroy(sqlite3 *db, FuncDef *p){
  FuncDestructor *pDestructor;
  assert( (p->funcFlags & SQLITE_FUNC_BUILTIN)==0 );
  pDestructor = p->u.pDestructor;
  如果( pDestructor ){
    pDestructor->nRef--;
    如果( pDestructor->nRef==0 ){
      pDestructor->xDestroy(pDestructor->pUserData);
      sqlite3DbFree(db, pDestructor);
    }
  }
}

/*
** Disconnect all sqlite3_vtab objects that belong to database connection
** db. This is called when db is being closed.
*/
static void disconnectAllVtab(sqlite3 *db){
#ifndef SQLITE_OMIT_VIRTUALTABLE
  整数 i;
  HashElem *p;
  sqlite3BtreeEnterAll(db);
  为(i=0; i<db->nDb; i++){
    Schema *pSchema = db->aDb[i].pSchema;
    如果( pSchema ){
      为(p=sqliteHashFirst(&pSchema->tblHash); p; p=sqliteHashNext(p)){
        Table *pTab = (Table *)sqliteHashData(p);
        如果( IsVirtual(pTab) ) sqlite3VtabDisconnect(db, pTab);
      }
    }
  }
  为(p=sqliteHashFirst(&db->aModule); p; p=sqliteHashNext(p)){
    Module *pMod = (Module *)sqliteHashData(p);
    如果( pMod->pEpoTab ){
      sqlite3VtabDisconnect(db, pMod->pEpoTab);
    }
  }
  sqlite3VtabUnlockList(db);
  sqlite3BtreeLeaveAll(db);
#否则
  UNUSED_PARAMETER(db);
#endif
}

/*
** Return TRUE if database connection db has unfinalized prepared
** statements or unfinished sqlite3_backup objects. 
*/
static 整数 connectionIsBusy(sqlite3 *db){
  整数 j;
  assert( sqlite3_mutex_held(db->mutex) );
  如果( db->pVdbe ) 返回 1;
  为(j=0; j<db->nDb; j++){
    Btree *pBt = db->aDb[j].pBt;
    如果( pBt && sqlite3BtreeIsInBackup(pBt) ) 返回 1;
  }
  返回 0;
}

/*
** Close an existing SQLite database
*/
static 整数 sqlite3Close(sqlite3 *db, 整数 forceZombie){
  如果( !db ){
    /* EVIDENCE-OF: R-63257-11740 Calling sqlite3_close() or
    ** sqlite3_close_v2() with a NULL pointer argument is a harmless no-op. */
    返回 SQLITE_OK;
  }
  如果( !sqlite3SafetyCheckSickOrOk(db) ){
    返回 SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(db->mutex);
  如果( db->mTrace & SQLITE_TRACE_CLOSE ){
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
  如果( !forceZombie && connectionIsBusy(db) ){
    sqlite3ErrorWithMsg(db, SQLITE_BUSY, "unable to close due to unfinalized "
       "statements or unfinished backups");
    sqlite3_mutex_leave(db->mutex);
    返回 SQLITE_BUSY;
  }

#ifdef SQLITE_ENABLE_SQLLOG
  如果( sqlite3GlobalConfig.xSqllog ){
    /* Closing the handle. Fourth parameter is passed the value 2. */
    sqlite3GlobalConfig.xSqllog(sqlite3GlobalConfig.pSqllogArg, db, 0, 2);
  }
#endif

  当( db->pDbData ){
    DbClientData *p = db->pDbData;
    db->pDbData = p->pNext;
    assert( p->pData!=0 );
    如果( p->xDestructor ) p->xDestructor(p->pData);
    sqlite3_free(p);
  }

  /* Convert the connection into a zombie and then close it.
  */
  db->eOpenState = SQLITE_STATE_ZOMBIE;
  sqlite3LeaveMutexAndCloseZombie(db);
  返回 SQLITE_OK;
}

/*
** Return the transaction state for a single databse, or the maximum
** transaction state over all attached databases if zSchema is null.
*/
整数 sqlite3_txn_state(sqlite3 *db, const char *zSchema){
  整数 iDb, nDb;
  整数 iTxn = -1;
#ifdef SQLITE_ENABLE_API_ARMOR
  如果( !sqlite3SafetyCheckOk(db) ){
    (void)SQLITE_MISUSE_BKPT;
    返回 -1;
  }
#endif
  sqlite3_mutex_enter(db->mutex);
  如果( zSchema ){
    nDb = iDb = sqlite3FindDbName(db, zSchema);
    如果( iDb<0 ) nDb--;
  }否则{
    iDb = 0;
    nDb = db->nDb-1;
  }
  为(; iDb<=nDb; iDb++){
    Btree *pBt = db->aDb[iDb].pBt;
    整数 x = pBt!=0 ? sqlite3BtreeTxnState(pBt) : SQLITE_TXN_NONE;
    如果( x>iTxn ) iTxn = x;
  }
  sqlite3_mutex_leave(db->mutex);
  返回 iTxn;
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
整数 sqlite3_close(sqlite3 *db){ 返回 sqlite3Close(db,0); }
整数 sqlite3_close_v2(sqlite3 *db){ 返回 sqlite3Close(db,1); }


/*
** Close the mutex on database connection db.
**
** Furthermore, if database connection db is a zombie (meaning that there
** has been a prior call to sqlite3_close(db) or sqlite3_close_v2(db)) and
** every sqlite3_stmt has now been finalized and every sqlite3_backup has
** finished, then free all resources.
*/
void sqlite3LeaveMutexAndCloseZombie(sqlite3 *db){
  HashElem *i;                    /* Hash table iterator */
  整数 j;

  /* If there are outstanding sqlite3_stmt or sqlite3_backup objects
  ** or if the connection has not yet been closed by sqlite3_close_v2(),
  ** then just leave the mutex and return.
  */
  如果( db->eOpenState!=SQLITE_STATE_ZOMBIE || connectionIsBusy(db) ){
    sqlite3_mutex_leave(db->mutex);
    返回;
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
  为(j=0; j<db->nDb; j++){
    struct Db *pDb = &db->aDb[j];
    如果( pDb->pBt ){
      sqlite3BtreeClose(pDb->pBt);
      pDb->pBt = 0;
      如果( j!=1 ){
        pDb->pSchema = 0;
      }
    }
  }
  /* Clear the TEMP schema separately and last */
  如果( db->aDb[1].pSchema ){
    sqlite3SchemaClear(db->aDb[1].pSchema);
    assert( db->aDb[1].pSchema->trigHash.count==0 );
  }
  sqlite3VtabUnlockList(db);

  /* Free up the array of auxiliary databases */
  sqlite3CollapseDatabaseArray(db);
  assert( db->nDb<=2 );
  assert( db->aDb==db->aDbStatic );

  /* Tell the code in notify.c that the connection no longer holds any
  ** locks and does not require any further unlock-notify callbacks.
  */
  sqlite3ConnectionClosed(db);

  为(i=sqliteHashFirst(&db->aFunc); i; i=sqliteHashNext(i)){
    FuncDef *pNext, *p;
    p = sqliteHashData(i);
    do{
      functionDestroy(db, p);
      pNext = p->pNext;
      sqlite3DbFree(db, p);
      p = pNext;
    }当( p );
  }
  sqlite3HashClear(&db->aFunc);
  为(i=sqliteHashFirst(&db->aCollSeq); i; i=sqliteHashNext(i)){
    CollSeq *pColl = (CollSeq *)sqliteHashData(i);
    /* Invoke any destructors registered for collation sequence user data. */
    为(j=0; j<3; j++){
      如果( pColl[j].xDel ){
        pColl[j].xDel(pColl[j].pUser);
      }
    }
    sqlite3DbFree(db, pColl);
  }
  sqlite3HashClear(&db->aCollSeq);
#ifndef SQLITE_OMIT_VIRTUALTABLE
  为(i=sqliteHashFirst(&db->aModule); i; i=sqliteHashNext(i)){
    Module *pMod = (Module *)sqliteHashData(i);
    sqlite3VtabEponymousTableClear(db, pMod);
    sqlite3VtabModuleUnref(db, pMod);
  }
  sqlite3HashClear(&db->aModule);
#endif

  sqlite3Error(db, SQLITE_OK); /* Deallocates any cached error strings. */
  sqlite3ValueFree(db->pErr);
  sqlite3CloseExtensions(db);

  db->eOpenState = SQLITE_STATE_ERROR;

  /* The temp-database schema is allocated differently from the other schema
  ** objects (using sqliteMalloc() directly, instead of sqlite3BtreeSchema()).
  ** So it needs to be freed here. Todo: Why not roll the temp schema into
  ** the same sqliteMalloc() as the one that allocates the database
  ** structure?
  */
  sqlite3DbFree(db, db->aDb[1].pSchema);
  如果( db->xAutovacDestr ){
    db->xAutovacDestr(db->pAutovacPagesArg);
  }
  sqlite3_mutex_leave(db->mutex);
  db->eOpenState = SQLITE_STATE_CLOSED;
  sqlite3_mutex_free(db->mutex);
  assert( sqlite3LookasideUsed(db,0)==0 );
  如果( db->lookaside.bMalloced ){
    sqlite3_free(db->lookaside.pStart);
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
void sqlite3RollbackAll(sqlite3 *db, 整数 tripCode){
  整数 i;
  整数 inTrans = 0;
  整数 schemaChange;
  assert( sqlite3_mutex_held(db->mutex) );
  sqlite3BeginBenignMalloc();

  /* Obtain all b-tree mutexes before making any calls to BtreeRollback().
  ** This is important in case the transaction being rolled back has
  ** modified the database schema. If the b-tree mutexes are not taken
  ** here, then another shared-cache connection might sneak in between
  ** the database rollback and schema reset, which can cause false
  ** corruption reports in some cases.  */
  sqlite3BtreeEnterAll(db);
  schemaChange = (db->mDbFlags & DBFLAG_SchemaChange)!=0 && db->init.busy==0;

  为(i=0; i<db->nDb; i++){
    Btree *p = db->aDb[i].pBt;
    如果( p ){
      如果( sqlite3BtreeTxnState(p)==SQLITE_TXN_WRITE ){
        inTrans = 1;
      }
      sqlite3BtreeRollback(p, tripCode, !schemaChange);
    }
  }
  sqlite3VtabRollback(db);
  sqlite3EndBenignMalloc();

  如果( schemaChange ){
    sqlite3ExpirePreparedStatements(db, 0);
    sqlite3ResetAllSchemasOfConnection(db);
  }
  sqlite3BtreeLeaveAll(db);

  /* Any deferred constraint violations have now been resolved. */
  db->nDeferredCons = 0;
  db->nDeferredImmCons = 0;
  db->flags &= ~(u64)(SQLITE_DeferFKs|SQLITE_CorruptRdOnly);

  /* If one has been configured, invoke the rollback-hook callback */
  如果( db->xRollbackCallback && (inTrans || !db->autoCommit) ){
    db->xRollbackCallback(db->pRollbackArg);
  }
}

/*
** Return a static string containing the name corresponding to the error code
** specified in the argument.
*/
#如果 defined(SQLITE_NEED_ERR_NAME)
const char *sqlite3ErrName(整数 rc){
  const char *zName = 0;
  整数 i, origRc = rc;
  为(i=0; i<2 && zName==0; i++, rc &= 0xff){
    switch( rc ){
      case SQLITE_OK:                 zName = "SQLITE_OK";                break;
      case SQLITE_ERROR:              zName = "SQLITE_ERROR";             break;
      case SQLITE_ERROR_SNAPSHOT:     zName = "SQLITE_ERROR_SNAPSHOT";    break;
      case SQLITE_ERROR_RETRY:        zName = "SQLITE_ERROR_RETRY";       break;
      case SQLITE_ERROR_MISSING_COLLSEQ:
                                zName = "SQLITE_ERROR_MISSING_COLLSEQ";   break;
      case SQLITE_INTERNAL:           zName = "SQLITE_INTERNAL";          break;
      case SQLITE_PERM:               zName = "SQLITE_PERM";              break;
      case SQLITE_ABORT:              zName = "SQLITE_ABORT";             break;
      case SQLITE_ABORT_ROLLBACK:     zName = "SQLITE_ABORT_ROLLBACK";    break;
      case SQLITE_BUSY:               zName = "SQLITE_BUSY";              break;
      case SQLITE_BUSY_RECOVERY:      zName = "SQLITE_BUSY_RECOVERY";     break;
      case SQLITE_BUSY_SNAPSHOT:      zName = "SQLITE_BUSY_SNAPSHOT";     break;
      case SQLITE_LOCKED:             zName = "SQLITE_LOCKED";            break;
      case SQLITE_LOCKED_SHAREDCACHE: zName = "SQLITE_LOCKED_SHAREDCACHE";break;
      case SQLITE_NOMEM:              zName = "SQLITE_NOMEM";             break;
      case SQLITE_READONLY:           zName = "SQLITE_READONLY";          break;
      case SQLITE_READONLY_RECOVERY:  zName = "SQLITE_READONLY_RECOVERY"; break;
      case SQLITE_READONLY_CANTINIT:  zName = "SQLITE_READONLY_CANTINIT"; break;
      case SQLITE_READONLY_ROLLBACK:  zName = "SQLITE_READONLY_ROLLBACK"; break;
      case SQLITE_READONLY_DBMOVED:   zName = "SQLITE_READONLY_DBMOVED";  break;
      case SQLITE_READONLY_DIRECTORY: zName = "SQLITE_READONLY_DIRECTORY";break;
      case SQLITE_INTERRUPT:          zName = "SQLITE_INTERRUPT";         break;
      case SQLITE_IOERR:              zName = "SQLITE_IOERR";             break;
      case SQLITE_IOERR_READ:         zName = "SQLITE_IOERR_READ";        break;
      case SQLITE_IOERR_SHORT_READ:   zName = "SQLITE_IOERR_SHORT_READ";  break;
      case SQLITE_IOERR_WRITE:        zName = "SQLITE_IOERR_WRITE";       break;
      case SQLITE_IOERR_FSYNC:        zName = "SQLITE_IOERR_FSYNC";       break;
      case SQLITE_IOERR_DIR_FSYNC:    zName = "SQLITE_IOERR_DIR_FSYNC";   break;
      case SQLITE_IOERR_TRUNCATE:     zName = "SQLITE_IOERR_TRUNCATE";    break;
      case SQLITE_IOERR_FSTAT:        zName = "SQLITE_IOERR_FSTAT";       break;
      case SQLITE_IOERR_UNLOCK:       zName = "SQLITE_IOERR_UNLOCK";      break;
      case SQLITE_IOERR_RDLOCK:       zName = "SQLITE_IOERR_RDLOCK";      break;
      case SQLITE_IOERR_DELETE:       zName = "SQLITE_IOERR_DELETE";      break;
      case SQLITE_IOERR_NOMEM:        zName = "SQLITE_IOERR_NOMEM";       break;
      case SQLITE_IOERR_ACCESS:       zName = "SQLITE_IOERR_ACCESS";      break;
      case SQLITE_IOERR_CHECKRESERVEDLOCK:
                                zName = "SQLITE_IOERR_CHECKRESERVEDLOCK"; break;
      case SQLITE_IOERR_LOCK:         zName = "SQLITE_IOERR_LOCK";        break;
      case SQLITE_IOERR_CLOSE:        zName = "SQLITE_IOERR_CLOSE";       break;
      case SQLITE_IOERR_DIR_CLOSE:    zName = "SQLITE_IOERR_DIR_CLOSE";   break;
      case SQLITE_IOERR_SHMOPEN:      zName = "SQLITE_IOERR_SHMOPEN";     break;
      case SQLITE_IOERR_SHMSIZE:      zName = "SQLITE_IOERR_SHMSIZE";     break;
      case SQLITE_IOERR_SHMLOCK:      zName = "SQLITE_IOERR_SHMLOCK";     break;
      case SQLITE_IOERR_SHMMAP:       zName = "SQLITE_IOERR_SHMMAP";      break;
      case SQLITE_IOERR_SEEK:         zName = "SQLITE_IOERR_SEEK";        break;
      case SQLITE_IOERR_DELETE_NOENT: zName = "SQLITE_IOERR_DELETE_NOENT";break;
      case SQLITE_IOERR_MMAP:         zName = "SQLITE_IOERR_MMAP";        break;
      case SQLITE_IOERR_GETTEMPPATH:  zName = "SQLITE_IOERR_GETTEMPPATH"; break;
      case SQLITE_IOERR_CONVPATH:     zName = "SQLITE_IOERR_CONVPATH";    break;
      case SQLITE_CORRUPT:            zName = "SQLITE_CORRUPT";           break;
      case SQLITE_CORRUPT_VTAB:       zName = "SQLITE_CORRUPT_VTAB";      break;
      case SQLITE_NOTFOUND:           zName = "SQLITE_NOTFOUND";          break;
      case SQLITE_FULL:               zName = "SQLITE_FULL";              break;
      case SQLITE_CANTOPEN:           zName = "SQLITE_CANTOPEN";          break;
      case SQLITE_CANTOPEN_NOTEMPDIR: zName = "SQLITE_CANTOPEN_NOTEMPDIR";break;
      case SQLITE_CANTOPEN_ISDIR:     zName = "SQLITE_CANTOPEN_ISDIR";    break;
      case SQLITE_CANTOPEN_FULLPATH:  zName = "SQLITE_CANTOPEN_FULLPATH"; break;
      case SQLITE_CANTOPEN_CONVPATH:  zName = "SQLITE_CANTOPEN_CONVPATH"; break;
      case SQLITE_CANTOPEN_SYMLINK:   zName = "SQLITE_CANTOPEN_SYMLINK";  break;
      case SQLITE_PROTOCOL:           zName = "SQLITE_PROTOCOL";          break;
      case SQLITE_EMPTY:              zName = "SQLITE_EMPTY";             break;
      case SQLITE_SCHEMA:             zName = "SQLITE_SCHEMA";            break;
      case SQLITE_TOOBIG:             zName = "SQLITE_TOOBIG";            break;
      case SQLITE_CONSTRAINT:         zName = "SQLITE_CONSTRAINT";        break;
      case SQLITE_CONSTRAINT_UNIQUE:  zName = "SQLITE_CONSTRAINT_UNIQUE"; break;
      case SQLITE_CONSTRAINT_TRIGGER: zName = "SQLITE_CONSTRAINT_TRIGGER";break;
      case SQLITE_CONSTRAINT_FOREIGNKEY:
                                zName = "SQLITE_CONSTRAINT_FOREIGNKEY";   break;
      case SQLITE_CONSTRAINT_CHECK:   zName = "SQLITE_CONSTRAINT_CHECK";  break;
      case SQLITE_CONSTRAINT_PRIMARYKEY:
                                zName = "SQLITE_CONSTRAINT_PRIMARYKEY";   break;
      case SQLITE_CONSTRAINT_NOTNULL: zName = "SQLITE_CONSTRAINT_NOTNULL";break;
      case SQLITE_CONSTRAINT_COMMITHOOK:
                                zName = "SQLITE_CONSTRAINT_COMMITHOOK";   break;
      case SQLITE_CONSTRAINT_VTAB:    zName = "SQLITE_CONSTRAINT_VTAB";   break;
      case SQLITE_CONSTRAINT_FUNCTION:
                                zName = "SQLITE_CONSTRAINT_FUNCTION";     break;
      case SQLITE_CONSTRAINT_ROWID:   zName = "SQLITE_CONSTRAINT_ROWID";  break;
      case SQLITE_MISMATCH:           zName = "SQLITE_MISMATCH";          break;
      case SQLITE_MISUSE:             zName = "SQLITE_MISUSE";            break;
      case SQLITE_NOLFS:              zName = "SQLITE_NOLFS";             break;
      case SQLITE_AUTH:               zName = "SQLITE_AUTH";              break;
      case SQLITE_FORMAT:             zName = "SQLITE_FORMAT";            break;
      case SQLITE_RANGE:              zName = "SQLITE_RANGE";             break;
      case SQLITE_NOTADB:             zName = "SQLITE_NOTADB";            break;
      case SQLITE_ROW:                zName = "SQLITE_ROW";               break;
      case SQLITE_NOTICE:             zName = "SQLITE_NOTICE";            break;
      case SQLITE_NOTICE_RECOVER_WAL: zName = "SQLITE_NOTICE_RECOVER_WAL";break;
      case SQLITE_NOTICE_RECOVER_ROLLBACK:
                                zName = "SQLITE_NOTICE_RECOVER_ROLLBACK"; break;
      case SQLITE_NOTICE_RBU:         zName = "SQLITE_NOTICE_RBU"; break;
      case SQLITE_WARNING:            zName = "SQLITE_WARNING";           break;
      case SQLITE_WARNING_AUTOINDEX:  zName = "SQLITE_WARNING_AUTOINDEX"; break;
      case SQLITE_DONE:               zName = "SQLITE_DONE";              break;
    }
  }
  如果( zName==0 ){
    static char zBuf[50];
    sqlite3_snprintf(sizeof(zBuf), zBuf, "SQLITE_UNKNOWN(%d)", origRc);
    zName = zBuf;
  }
  返回 zName;
}
#endif

/*
** Return a static string that describes the kind of error specified in the
** argument.
*/
const char *sqlite3ErrStr(整数 rc){
  static const char* const aMsg[] = {
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
#ifdef SQLITE_DISABLE_LFS
    /* SQLITE_NOLFS       */ "large file support is disabled",
#否则
    /* SQLITE_NOLFS       */ 0,
#endif
    /* SQLITE_AUTH        */ "authorization denied",
    /* SQLITE_FORMAT      */ 0,
    /* SQLITE_RANGE       */ "column index out of range",
    /* SQLITE_NOTADB      */ "file is not a database",
    /* SQLITE_NOTICE      */ "notification message",
    /* SQLITE_WARNING     */ "warning message",
  };
  const char *zErr = "unknown error";
  switch( rc ){
    case SQLITE_ABORT_ROLLBACK: {
      zErr = "abort due to ROLLBACK";
      break;
    }
    case SQLITE_ROW: {
      zErr = "another row available";
      break;
    }
    case SQLITE_DONE: {
      zErr = "no more rows available";
      break;
    }
    default: {
      rc &= 0xff;
      如果( ALWAYS(rc>=0) && rc<ArraySize(aMsg) && aMsg[rc]!=0 ){
        zErr = aMsg[rc];
      }
      break;
    }
  }
  返回 zErr;
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
static 整数 sqliteDefaultBusyCallback(
  void *ptr,               /* Database connection */
  整数 count                /* Number of times table has been busy */
){
#如果 SQLITE_OS_WIN || !defined(HAVE_NANOSLEEP) || HAVE_NANOSLEEP
  /* This case is for systems that have support for sleeping for fractions of
  ** a second.  Examples:  All windows systems, unix systems with nanosleep() */
  static const u8 delays[] =
     { 1, 2, 5, 10, 15, 20, 25, 25,  25,  50,  50, 100 };
  static const u8 totals[] =
     { 0, 1, 3,  8, 18, 33, 53, 78, 103, 128, 178, 228 };
# define NDELAY ArraySize(delays)
  sqlite3 *db = (sqlite3 *)ptr;
  整数 tmout = db->busyTimeout;
  整数 delay, prior;

  assert( count>=0 );
  如果( count < NDELAY ){
    delay = delays[count];
    prior = totals[count];
  }否则{
    delay = delays[NDELAY-1];
    prior = totals[NDELAY-1] + delay*(count-(NDELAY-1));
  }
  如果( prior + delay > tmout ){
    delay = tmout - prior;
    如果( delay<=0 ) 返回 0;
  }
  sqlite3OsSleep(db->pVfs, delay*1000);
  返回 1;
#否则
  /* This case for unix systems that lack usleep() support.  Sleeping
  ** must be done in increments of whole seconds */
  sqlite3 *db = (sqlite3 *)ptr;
  整数 tmout = ((sqlite3 *)ptr)->busyTimeout;
  如果( (count+1)*1000 > tmout ){
    返回 0;
  }
  sqlite3OsSleep(db->pVfs, 1000000);
  返回 1;
#endif
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
整数 sqlite3InvokeBusyHandler(BusyHandler *p){
  整数 rc;
  如果( p->xBusyHandler==0 || p->nBusy<0 ) 返回 0;
  rc = p->xBusyHandler(p->pBusyArg, p->nBusy);
  如果( rc==0 ){
    p->nBusy = -1;
  }否则{
    p->nBusy++;
  }
  返回 rc;
}

/*
** This routine sets the busy callback for an Sqlite database to the
** given callback function with the given argument.
*/
整数 sqlite3_busy_handler(
  sqlite3 *db,
  整数 (*xBusy)(void*,整数),
  void *pArg
){
#ifdef SQLITE_ENABLE_API_ARMOR
  如果( !sqlite3SafetyCheckOk(db) ) 返回 SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(db->mutex);
  db->busyHandler.xBusyHandler = xBusy;
  db->busyHandler.pBusyArg = pArg;
  db->busyHandler.nBusy = 0;
  db->busyTimeout = 0;
#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
  db->setlkTimeout = 0;
#endif
  sqlite3_mutex_leave(db->mutex);
  返回 SQLITE_OK;
}

#ifndef SQLITE_OMIT_PROGRESS_CALLBACK
/*
** This routine sets the progress callback for an Sqlite database to the
** given callback function with the given argument. The progress callback will
** be invoked every nOps opcodes.
*/
void sqlite3_progress_handler(
  sqlite3 *db,
  整数 nOps,
  整数 (*xProgress)(void*),
  void *pArg
){
#ifdef SQLITE_ENABLE_API_ARMOR
  如果( !sqlite3SafetyCheckOk(db) ){
    (void)SQLITE_MISUSE_BKPT;
    返回;
  }
#endif
  sqlite3_mutex_enter(db->mutex);
  如果( nOps>0 ){
    db->xProgress = xProgress;
    db->nProgressOps = (unsigned)nOps;
    db->pProgressArg = pArg;
  }否则{
    db->xProgress = 0;
    db->nProgressOps = 0;
    db->pProgressArg = 0;
  }
  sqlite3_mutex_leave(db->mutex);
}
#endif


/*
** This routine installs a default busy handler that waits for the
** specified number of milliseconds before returning 0.
*/
整数 sqlite3_busy_timeout(sqlite3 *db, 整数 ms){
#ifdef SQLITE_ENABLE_API_ARMOR
  如果( !sqlite3SafetyCheckOk(db) ) 返回 SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(db->mutex);
  如果( ms>0 ){
    sqlite3_busy_handler(db, (整数(*)(void*,整数))sqliteDefaultBusyCallback,
                             (void*)db);
    db->busyTimeout = ms;
#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
    db->setlkTimeout = ms;
#endif
  }否则{
    sqlite3_busy_handler(db, 0, 0);
  }
  sqlite3_mutex_leave(db->mutex);
  返回 SQLITE_OK;
}

/*
** Set the setlk timeout value.
*/
整数 sqlite3_setlk_timeout(sqlite3 *db, 整数 ms, 整数 flags){
#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
  整数 iDb;
  整数 bBOC = ((flags & SQLITE_SETLK_BLOCK_ON_CONNECT) ? 1 : 0);
#endif
#ifdef SQLITE_ENABLE_API_ARMOR
  如果( !sqlite3SafetyCheckOk(db) ) 返回 SQLITE_MISUSE_BKPT;
#endif
  如果( ms<-1 ) 返回 SQLITE_RANGE;
#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
  sqlite3_mutex_enter(db->mutex);
  db->setlkTimeout = ms;
  db->setlkFlags = flags;
  sqlite3BtreeEnterAll(db);
  为(iDb=0; iDb<db->nDb; iDb++){
    Btree *pBt = db->aDb[iDb].pBt;
    如果( pBt ){
      sqlite3_file *fd = sqlite3PagerFile(sqlite3BtreePager(pBt));
      sqlite3OsFileControlHint(fd, SQLITE_FCNTL_BLOCK_ON_CONNECT, (void*)&bBOC);
    }
  }
  sqlite3BtreeLeaveAll(db);
  sqlite3_mutex_leave(db->mutex);
#endif
#如果 !defined(SQLITE_ENABLE_API_ARMOR) && !defined(SQLITE_ENABLE_SETLK_TIMEOUT)
  UNUSED_PARAMETER(db);
  UNUSED_PARAMETER(flags);
#endif
  返回 SQLITE_OK;
}

/*
** Cause any pending operation to stop at its earliest opportunity.
*/
void sqlite3_interrupt(sqlite3 *db){
#ifdef SQLITE_ENABLE_API_ARMOR
  如果( !sqlite3SafetyCheckOk(db)
   && (db==0 || db->eOpenState!=SQLITE_STATE_ZOMBIE)
  ){
    (void)SQLITE_MISUSE_BKPT;
    返回;
  }
#endif
  AtomicStore(&db->u1.isInterrupted, 1);
}

/*
** Return true or false depending on whether or not an interrupt is
** pending on connection db.
*/
整数 sqlite3_is_interrupted(sqlite3 *db){
#ifdef SQLITE_ENABLE_API_ARMOR
  如果( !sqlite3SafetyCheckOk(db)
   && (db==0 || db->eOpenState!=SQLITE_STATE_ZOMBIE)
  ){
    (void)SQLITE_MISUSE_BKPT;
    返回 0;
  }
#endif
  返回 AtomicLoad(&db->u1.isInterrupted)!=0;
}

/*
** This function is exactly the same as sqlite3_create_function(), except
** that it is designed to be called by internal code. The difference is
** that if a malloc() fails in sqlite3_create_function(), an error code
** is returned and the mallocFailed flag cleared.
*/
整数 sqlite3CreateFunc(
  sqlite3 *db,
  const char *zFunctionName,
  整数 nArg,
  整数 enc,
  void *pUserData,
  void (*xSFunc)(sqlite3_context*,整数,sqlite3_value **),
  void (*xStep)(sqlite3_context*,整数,sqlite3_value **),
  void (*xFinal)(sqlite3_context*),
  void (*xValue)(sqlite3_context*),
  void (*xInverse)(sqlite3_context*,整数,sqlite3_value **),
  FuncDestructor *pDestructor
){
  FuncDef *p;
  整数 extraFlags;

  assert( sqlite3_mutex_held(db->mutex) );
  assert( xValue==0 || xSFunc==0 );
  如果( zFunctionName==0                /* Must have a valid name */
   || (xSFunc!=0 && xFinal!=0)        /* Not both xSFunc and xFinal */
   || ((xFinal==0)!=(xStep==0))       /* Both or neither of xFinal and xStep */
   || ((xValue==0)!=(xInverse==0))    /* Both or neither of xValue, xInverse */
   || (nArg<-1 || nArg>SQLITE_MAX_FUNCTION_ARG)
   || (255<sqlite3Strlen30(zFunctionName))
  ){
    返回 SQLITE_MISUSE_BKPT;
  }

  assert( SQLITE_FUNC_CONSTANT==SQLITE_DETERMINISTIC );
  assert( SQLITE_FUNC_DIRECT==SQLITE_DIRECTONLY );
  extraFlags = enc &  (SQLITE_DETERMINISTIC|SQLITE_DIRECTONLY|
                       SQLITE_SUBTYPE|SQLITE_INNOCUOUS|
                       SQLITE_RESULT_SUBTYPE|SQLITE_SELFORDER1);
  enc &= (SQLITE_FUNC_ENCMASK|SQLITE_ANY);

  /* The SQLITE_INNOCUOUS flag is the same bit as SQLITE_FUNC_UNSAFE.  But
  ** the meaning is inverted.  So flip the bit. */
  assert( SQLITE_FUNC_UNSAFE==SQLITE_INNOCUOUS );
  extraFlags ^= SQLITE_FUNC_UNSAFE;  /* tag-20230109-1 */

 
#ifndef SQLITE_OMIT_UTF16
  /* If SQLITE_UTF16 is specified as the encoding type, transform this
  ** to one of SQLITE_UTF16LE or SQLITE_UTF16BE using the
  ** SQLITE_UTF16NATIVE macro. SQLITE_UTF16 is not used internally.
  **
  ** If SQLITE_ANY is specified, add three versions of the function
  ** to the hash table.
  */
  switch( enc ){
    case SQLITE_UTF16:
      enc = SQLITE_UTF16NATIVE;
      break;
    case SQLITE_ANY: {
      整数 rc;
      rc = sqlite3CreateFunc(db, zFunctionName, nArg,
           (SQLITE_UTF8|extraFlags)^SQLITE_FUNC_UNSAFE, /* tag-20230109-1 */
           pUserData, xSFunc, xStep, xFinal, xValue, xInverse, pDestructor);
      如果( rc==SQLITE_OK ){
        rc = sqlite3CreateFunc(db, zFunctionName, nArg,
             (SQLITE_UTF16LE|extraFlags)^SQLITE_FUNC_UNSAFE, /* tag-20230109-1*/
             pUserData, xSFunc, xStep, xFinal, xValue, xInverse, pDestructor);
      }
      如果( rc!=SQLITE_OK ){
        返回 rc;
      }
      enc = SQLITE_UTF16BE;
      break;
    }
    case SQLITE_UTF8:
    case SQLITE_UTF16LE:
    case SQLITE_UTF16BE:
      break;
    default:
      enc = SQLITE_UTF8;
      break;
  }
#否则
  enc = SQLITE_UTF8;
#endif
 
  /* Check if an existing function is being overridden or deleted. If so,
  ** and there are active VMs, then return SQLITE_BUSY. If a function
  ** is being overridden/deleted but there are no active VMs, allow the
  ** operation to continue but invalidate all precompiled statements.
  */
  p = sqlite3FindFunction(db, zFunctionName, nArg, (u8)enc, 0);
  如果( p && (p->funcFlags & SQLITE_FUNC_ENCMASK)==(u32)enc && p->nArg==nArg ){
    如果( db->nVdbeActive ){
      sqlite3ErrorWithMsg(db, SQLITE_BUSY,
        "unable to delete/modify user-function due to active statements");
      assert( !db->mallocFailed || db->nVdbeExec>0 );
      返回 SQLITE_BUSY;
    }否则{
      sqlite3ExpirePreparedStatements(db, 0);
    }
  }否则 如果( xSFunc==0 && xFinal==0 ){
    /* Trying to delete a function that does not exist.  This is a no-op.
    ** https://sqlite.org/forum/forumpost/726219164b */
    返回 SQLITE_OK;
  }

  p = sqlite3FindFunction(db, zFunctionName, nArg, (u8)enc, 1);
  assert(p || db->mallocFailed);
  如果( !p ){
    返回 SQLITE_NOMEM_BKPT;
  }

  /* If an older version of the function with a configured destructor is
  ** being replaced invoke the destructor function here. */
  functionDestroy(db, p);

  如果( pDestructor ){
    pDestructor->nRef++;
  }
  p->u.pDestructor = pDestructor;
  p->funcFlags = (p->funcFlags & SQLITE_FUNC_ENCMASK) | extraFlags;
  testcase( p->funcFlags & SQLITE_DETERMINISTIC );
  testcase( p->funcFlags & SQLITE_DIRECTONLY );
  p->xSFunc = xSFunc ? xSFunc : xStep;
  p->xFinalize = xFinal;
  p->xValue = xValue;
  p->xInverse = xInverse;
  p->pUserData = pUserData;
  p->nArg = (u16)nArg;
  返回 SQLITE_OK;
}

/*
** Worker function used by utf-8 APIs that create new functions:
**
**    sqlite3_create_function()
**    sqlite3_create_function_v2()
**    sqlite3_create_window_function()
*/
static 整数 createFunctionApi(
  sqlite3 *db,
  const char *zFunc,
  整数 nArg,
  整数 enc,
  void *p,
  void (*xSFunc)(sqlite3_context*,整数,sqlite3_value**),
  void (*xStep)(sqlite3_context*,整数,sqlite3_value**),
  void (*xFinal)(sqlite3_context*),
  void (*xValue)(sqlite3_context*),
  void (*xInverse)(sqlite3_context*,整数,sqlite3_value**),
  void(*xDestroy)(void*)
){
  整数 rc = SQLITE_ERROR;
  FuncDestructor *pArg = 0;

#ifdef SQLITE_ENABLE_API_ARMOR
  如果( !sqlite3SafetyCheckOk(db) ){
    返回 SQLITE_MISUSE_BKPT;
  }
#endif
  sqlite3_mutex_enter(db->mutex);
  如果( xDestroy ){
    pArg = (FuncDestructor *)sqlite3Malloc(sizeof(FuncDestructor));
    如果( !pArg ){
      sqlite3OomFault(db);
      xDestroy(p);
      goto out;
    }
    pArg->nRef = 0;
    pArg->xDestroy = xDestroy;
    pArg->pUserData = p;
  }
  rc = sqlite3CreateFunc(db, zFunc, nArg, enc, p,
      xSFunc, xStep, xFinal, xValue, xInverse, pArg
  );
  如果( pArg && pArg->nRef==0 ){
    assert( rc!=SQLITE_OK || (xStep==0 && xFinal==0) );
    xDestroy(p);
    sqlite3_free(pArg);
  }

 out:
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  返回 rc;
}

/*
** Create new user functions.
*/
整数 sqlite3_create_function(
  sqlite3 *db,
  const char *zFunc,
  整数 nArg,
  整数 enc,
  void *p,
  void (*xSFunc)(sqlite3_context*,整数,sqlite3_value **),
  void (*xStep)(sqlite3_context*,整数,sqlite3_value **),
  void (*xFinal)(sqlite3_context*)
){
  返回 createFunctionApi(db, zFunc, nArg, enc, p, xSFunc, xStep,
                                    xFinal, 0, 0, 0);
}
整数 sqlite3_create_function_v2(
  sqlite3 *db,
  const char *zFunc,
  整数 nArg,
  整数 enc,
  void *p,
  void (*xSFunc)(sqlite3_context*,整数,sqlite3_value **),
  void (*xStep)(sqlite3_context*,整数,sqlite3_value **),
  void (*xFinal)(sqlite3_context*),
  void (*xDestroy)(void *)
){
  返回 createFunctionApi(db, zFunc, nArg, enc, p, xSFunc, xStep,
                                    xFinal, 0, 0, xDestroy);
}
整数 sqlite3_create_window_function(
  sqlite3 *db,
  const char *zFunc,
  整数 nArg,
  整数 enc,
  void *p,
  void (*xStep)(sqlite3_context*,整数,sqlite3_value **),
  void (*xFinal)(sqlite3_context*),
  void (*xValue)(sqlite3_context*),
  void (*xInverse)(sqlite3_context*,整数,sqlite3_value **),
  void (*xDestroy)(void *)
){
  返回 createFunctionApi(db, zFunc, nArg, enc, p, 0, xStep,
                                    xFinal, xValue, xInverse, xDestroy);
}

#ifndef SQLITE_OMIT_UTF16
整数 sqlite3_create_function16(
  sqlite3 *db,
  const void *zFunctionName,
  整数 nArg,
  整数 eTextRep,
  void *p,
  void (*xSFunc)(sqlite3_context*,整数,sqlite3_value**),
  void (*xStep)(sqlite3_context*,整数,sqlite3_value**),
  void (*xFinal)(sqlite3_context*)
){
  整数 rc;
  char *zFunc8;

#ifdef SQLITE_ENABLE_API_ARMOR
  如果( !sqlite3SafetyCheckOk(db) || zFunctionName==0 ) 返回 SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(db->mutex);
  assert( !db->mallocFailed );
  zFunc8 = sqlite3Utf16to8(db, zFunctionName, -1, SQLITE_UTF16NATIVE);
  rc = sqlite3CreateFunc(db, zFunc8, nArg, eTextRep, p, xSFunc,xStep,xFinal,0,0,0);
  sqlite3DbFree(db, zFunc8);
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  返回 rc;
}
#endif


/*
** The following is the implementation of an SQL function that always
** fails with an error message stating that the function is used in the
** wrong context.  The sqlite3_overload_function() API might construct
** SQL function that use this routine so that the functions will exist
** for name resolution but are actually overloaded by the xFindFunction
** method of virtual tables.
*/
static void sqlite3InvalidFunction(
  sqlite3_context *context,  /* The function calling context */
  整数 NotUsed,               /* Number of arguments to the function */
  sqlite3_value **NotUsed2   /* Value of each argument */
){
  const char *zName = (const char*)sqlite3_user_data(context);
  char *zErr;
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  zErr = sqlite3_mprintf(
      "unable to use function %s in the requested context", zName);
  sqlite3_result_error(context, zErr, -1);
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
整数 sqlite3_overload_function(
  sqlite3 *db,
  const char *zName,
  整数 nArg
){
  整数 rc;
  char *zCopy;

#ifdef SQLITE_ENABLE_API_ARMOR
  如果( !sqlite3SafetyCheckOk(db) || zName==0 || nArg<-2 ){
    返回 SQLITE_MISUSE_BKPT;
  }
#endif
  sqlite3_mutex_enter(db->mutex);
  rc = sqlite3FindFunction(db, zName, nArg, SQLITE_UTF8, 0)!=0;
  sqlite3_mutex_leave(db->mutex);
  如果( rc ) 返回 SQLITE_OK;
  zCopy = sqlite3_mprintf("%s", zName);
  如果( zCopy==0 ) 返回 SQLITE_NOMEM;
  返回 sqlite3_create_function_v2(db, zName, nArg, SQLITE_UTF8,
                           zCopy, sqlite3InvalidFunction, 0, 0, sqlite3_free);
}

#ifndef SQLITE_OMIT_TRACE
/*
** Register a trace function.  The pArg from the previously registered trace
** is returned. 
**
** A NULL trace function means that no tracing is executes.  A non-NULL
** trace is a pointer to a function that is invoked at the start of each
** SQL statement.
*/
#ifndef SQLITE_OMIT_DEPRECATED
void *sqlite3_trace(sqlite3 *db, void(*xTrace)(void*,const char*), void *pArg){
  void *pOld;

#ifdef SQLITE_ENABLE_API_ARMOR
  如果( !sqlite3SafetyCheckOk(db) ){
    (void)SQLITE_MISUSE_BKPT;
    返回 0;
  }
#endif
  sqlite3_mutex_enter(db->mutex);
  pOld = db->pTraceArg;
  db->mTrace = xTrace ? SQLITE_TRACE_LEGACY : 0;
  db->trace.xLegacy = xTrace;
  db->pTraceArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  返回 pOld;
}
#endif /* SQLITE_OMIT_DEPRECATED */

/* Register a trace callback using the version-2 interface.
*/
整数 sqlite3_trace_v2(
  sqlite3 *db,                               /* Trace this connection */
  unsigned mTrace,                           /* Mask of events to be traced */
  整数(*xTrace)(unsigned,void*,void*,void*),  /* Callback to invoke */
  void *pArg                                 /* Context */
){
#ifdef SQLITE_ENABLE_API_ARMOR
  如果( !sqlite3SafetyCheckOk(db) ){
    返回 SQLITE_MISUSE_BKPT;
  }
#endif
  sqlite3_mutex_enter(db->mutex);
  如果( mTrace==0 ) xTrace = 0;
  如果( xTrace==0 ) mTrace = 0;
  db->mTrace = mTrace;
  db->trace.xV2 = xTrace;
  db->pTraceArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  返回 SQLITE_OK;
}

#ifndef SQLITE_OMIT_DEPRECATED
/*
** Register a profile function.  The pArg from the previously registered
** profile function is returned. 
**
** A NULL profile function means that no profiling is executes.  A non-NULL
** profile is a pointer to a function that is invoked at the conclusion of
** each SQL statement that is run.
*/
void *sqlite3_profile(
  sqlite3 *db,
  void (*xProfile)(void*,const char*,sqlite_uint64),
  void *pArg
){
  void *pOld;

#ifdef SQLITE_ENABLE_API_ARMOR
  如果( !sqlite3SafetyCheckOk(db) ){
    (void)SQLITE_MISUSE_BKPT;
    返回 0;
  }
#endif
  sqlite3_mutex_enter(db->mutex);
  pOld = db->pProfileArg;
  db->xProfile = xProfile;
  db->pProfileArg = pArg;
  db->mTrace &= SQLITE_TRACE_NONLEGACY_MASK;
  如果( db->xProfile ) db->mTrace |= SQLITE_TRACE_XPROFILE;
  sqlite3_mutex_leave(db->mutex);
  返回 pOld;
}
#endif /* SQLITE_OMIT_DEPRECATED */
#endif /* SQLITE_OMIT_TRACE */

/*
** Register a function to be invoked when a transaction commits.
** If the invoked function returns non-zero, then the commit becomes a
** rollback.
*/
void *sqlite3_commit_hook(
  sqlite3 *db,              /* Attach the hook to this database */
  整数 (*xCallback)(void*),  /* Function to invoke on each commit */
  void *pArg                /* Argument to the function */
){
  void *pOld;

#ifdef SQLITE_ENABLE_API_ARMOR
  如果( !sqlite3SafetyCheckOk(db) ){
    (void)SQLITE_MISUSE_BKPT;
    返回 0;
  }
#endif
  sqlite3_mutex_enter(db->mutex);
  pOld = db->pCommitArg;
  db->xCommitCallback = xCallback;
  db->pCommitArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  返回 pOld;
}

/*
** Register a callback to be invoked each time a row is updated,
** inserted or deleted using this database connection.
*/
void *sqlite3_update_hook(
  sqlite3 *db,              /* Attach the hook to this database */
  void (*xCallback)(void*,整数,char const *,char const *,sqlite_int64),
  void *pArg                /* Argument to the function */
){
  void *pRet;

#ifdef SQLITE_ENABLE_API_ARMOR
  如果( !sqlite3SafetyCheckOk(db) ){
    (void)SQLITE_MISUSE_BKPT;
    返回 0;
  }
#endif
  sqlite3_mutex_enter(db->mutex);
  pRet = db->pUpdateArg;
  db->xUpdateCallback = xCallback;
  db->pUpdateArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  返回 pRet;
}

/*
** Register a callback to be invoked each time a transaction is rolled
** back by this database connection.
*/
void *sqlite3_rollback_hook(
  sqlite3 *db,              /* Attach the hook to this database */
  void (*xCallback)(void*), /* Callback function */
  void *pArg                /* Argument to the function */
){
  void *pRet;

#ifdef SQLITE_ENABLE_API_ARMOR
  如果( !sqlite3SafetyCheckOk(db) ){
    (void)SQLITE_MISUSE_BKPT;
    返回 0;
  }
#endif
  sqlite3_mutex_enter(db->mutex);
  pRet = db->pRollbackArg;
  db->xRollbackCallback = xCallback;
  db->pRollbackArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  返回 pRet;
}

#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
/*
** Register a callback to be invoked each time a row is updated,
** inserted or deleted using this database connection.
*/
void *sqlite3_preupdate_hook(
  sqlite3 *db,              /* Attach the hook to this database */
  void(*xCallback)(         /* Callback function */
    void*,sqlite3*,整数,char const*,char const*,sqlite3_int64,sqlite3_int64),
  void *pArg                /* First callback argument */
){
  void *pRet;

#ifdef SQLITE_ENABLE_API_ARMOR
  如果( db==0 ){
    返回 0;
  }
#endif
  sqlite3_mutex_enter(db->mutex);
  pRet = db->pPreUpdateArg;
  db->xPreUpdateCallback = xCallback;
  db->pPreUpdateArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  返回 pRet;
}
#endif /* SQLITE_ENABLE_PREUPDATE_HOOK */

/*
** Register a function to be invoked prior to each autovacuum that
** determines the number of pages to vacuum.
*/
整数 sqlite3_autovacuum_pages(
  sqlite3 *db,                 /* Attach the hook to this database */
  unsigned 整数 (*xCallback)(void*,const char*,u32,u32,u32),
  void *pArg,                  /* Argument to the function */
  void (*xDestructor)(void*)   /* Destructor for pArg */
){
#ifdef SQLITE_ENABLE_API_ARMOR
  如果( !sqlite3SafetyCheckOk(db) ){
    如果( xDestructor ) xDestructor(pArg);
    返回 SQLITE_MISUSE_BKPT;
  }
#endif
  sqlite3_mutex_enter(db->mutex);
  如果( db->xAutovacDestr ){
    db->xAutovacDestr(db->pAutovacPagesArg);
  }
  db->xAutovacPages = xCallback;
  db->pAutovacPagesArg = pArg;
  db->xAutovacDestr = xDestructor;
  sqlite3_mutex_leave(db->mutex);
  返回 SQLITE_OK;
}


#ifndef SQLITE_OMIT_WAL
/*
** The sqlite3_wal_hook() callback registered by sqlite3_wal_autocheckpoint().
** Invoke sqlite3_wal_checkpoint if the number of frames in the log file
** is greater than sqlite3.pWalArg cast to an integer (the value configured by
** wal_autocheckpoint()).
*/
整数 sqlite3WalDefaultHook(
  void *pClientData,     /* Argument */
  sqlite3 *db,           /* Connection */
  const char *zDb,       /* Database */
  整数 nFrame             /* Size of WAL */
){
  如果( nFrame>=SQLITE_PTR_TO_INT(pClientData) ){
    sqlite3BeginBenignMalloc();
    sqlite3_wal_checkpoint(db, zDb);
    sqlite3EndBenignMalloc();
  }
  返回 SQLITE_OK;
}
#endif /* SQLITE_OMIT_WAL */

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
整数 sqlite3_wal_autocheckpoint(sqlite3 *db, 整数 nFrame){
#ifdef SQLITE_OMIT_WAL
  UNUSED_PARAMETER(db);
  UNUSED_PARAMETER(nFrame);
#否则
#ifdef SQLITE_ENABLE_API_ARMOR
  如果( !sqlite3SafetyCheckOk(db) ) 返回 SQLITE_MISUSE_BKPT;
#endif
  如果( nFrame>0 ){
    sqlite3_wal_hook(db, sqlite3WalDefaultHook, SQLITE_INT_TO_PTR(nFrame));
  }否则{
    sqlite3_wal_hook(db, 0, 0);
  }
#endif
  返回 SQLITE_OK;
}

/*
** Register a callback to be invoked each time a transaction is written
** into the write-ahead-log by this database connection.
*/
void *sqlite3_wal_hook(
  sqlite3 *db,                    /* Attach the hook to this db handle */
  整数(*xCallback)(void *, sqlite3*, const char*, 整数),
  void *pArg                      /* First argument passed to xCallback() */
){
#ifndef SQLITE_OMIT_WAL
  void *pRet;
#ifdef SQLITE_ENABLE_API_ARMOR
  如果( !sqlite3SafetyCheckOk(db) ){
    (void)SQLITE_MISUSE_BKPT;
    返回 0;
  }
#endif
  sqlite3_mutex_enter(db->mutex);
  pRet = db->pWalArg;
  db->xWalCallback = xCallback;
  db->pWalArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  返回 pRet;
#否则
  UNUSED_PARAMETER(db);
  UNUSED_PARAMETER(xCallback);
  UNUSED_PARAMETER(pArg);
  返回 0;
#endif
}

/*
** Checkpoint database zDb.
*/
整数 sqlite3_wal_checkpoint_v2(
  sqlite3 *db,                    /* Database handle */
  const char *zDb,                /* Name of attached database (or NULL) */
  整数 eMode,                      /* SQLITE_CHECKPOINT_* value */
  整数 *pnLog,                     /* OUT: Size of WAL log in frames */
  整数 *pnCkpt                     /* OUT: Total number of frames checkpointed */
){
#ifdef SQLITE_OMIT_WAL
  UNUSED_PARAMETER(db);
  UNUSED_PARAMETER(zDb);
  UNUSED_PARAMETER(eMode);
  UNUSED_PARAMETER(pnLog);
  UNUSED_PARAMETER(pnCkpt);
  返回 SQLITE_OK;
#否则
  整数 rc;                         /* Return code */
  整数 iDb;                        /* Schema to checkpoint */

#ifdef SQLITE_ENABLE_API_ARMOR
  如果( !sqlite3SafetyCheckOk(db) ) 返回 SQLITE_MISUSE_BKPT;
#endif

  /* Initialize the output variables to -1 in case an error occurs. */
  如果( pnLog ) *pnLog = -1;
  如果( pnCkpt ) *pnCkpt = -1;

  assert( SQLITE_CHECKPOINT_NOOP==-1 );
  assert( SQLITE_CHECKPOINT_PASSIVE==0 );
  assert( SQLITE_CHECKPOINT_FULL==1 );
  assert( SQLITE_CHECKPOINT_RESTART==2 );
  assert( SQLITE_CHECKPOINT_TRUNCATE==3 );
  如果( eMode<SQLITE_CHECKPOINT_NOOP || eMode>SQLITE_CHECKPOINT_TRUNCATE ){
    /* EVIDENCE-OF: R-03996-12088 The M parameter must be a valid checkpoint
    ** mode: */
    返回 SQLITE_MISUSE_BKPT;
  }

  sqlite3_mutex_enter(db->mutex);
  如果( zDb && zDb[0] ){
    iDb = sqlite3FindDbName(db, zDb);
  }否则{
    iDb = SQLITE_MAX_DB;   /* This means process all schemas */
  }
  如果( iDb<0 ){
    rc = SQLITE_ERROR;
    sqlite3ErrorWithMsg(db, SQLITE_ERROR, "unknown database: %s", zDb);
  }否则{
    db->busyHandler.nBusy = 0;
    rc = sqlite3Checkpoint(db, iDb, eMode, pnLog, pnCkpt);
    sqlite3Error(db, rc);
  }
  rc = sqlite3ApiExit(db, rc);

  /* If there are no active statements, clear the interrupt flag at this
  ** point.  */
  如果( db->nVdbeActive==0 ){
    AtomicStore(&db->u1.isInterrupted, 0);
  }

  sqlite3_mutex_leave(db->mutex);
  返回 rc;
#endif
}


/*
** Checkpoint database zDb. If zDb is NULL, or if the buffer zDb points
** to contains a zero-length string, all attached databases are
** checkpointed.
*/
整数 sqlite3_wal_checkpoint(sqlite3 *db, const char *zDb){
  /* EVIDENCE-OF: R-41613-20553 The sqlite3_wal_checkpoint(D,X) is equivalent to
  ** sqlite3_wal_checkpoint_v2(D,X,SQLITE_CHECKPOINT_PASSIVE,0,0). */
  返回 sqlite3_wal_checkpoint_v2(db,zDb,SQLITE_CHECKPOINT_PASSIVE,0,0);
}

#ifndef SQLITE_OMIT_WAL
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
整数 sqlite3Checkpoint(sqlite3 *db, 整数 iDb, 整数 eMode, 整数 *pnLog, 整数 *pnCkpt){
  整数 rc = SQLITE_OK;             /* Return code */
  整数 i;                          /* Used to iterate through attached dbs */
  整数 bBusy = 0;                  /* True if SQLITE_BUSY has been encountered */

  assert( sqlite3_mutex_held(db->mutex) );
  assert( !pnLog || *pnLog==-1 );
  assert( !pnCkpt || *pnCkpt==-1 );
  testcase( iDb==SQLITE_MAX_ATTACHED ); /* See forum post a006d86f72 */
  testcase( iDb==SQLITE_MAX_DB );

  为(i=0; i<db->nDb && rc==SQLITE_OK; i++){
    如果( i==iDb || iDb==SQLITE_MAX_DB ){
      rc = sqlite3BtreeCheckpoint(db->aDb[i].pBt, eMode, pnLog, pnCkpt);
      pnLog = 0;
      pnCkpt = 0;
      如果( rc==SQLITE_BUSY ){
        bBusy = 1;
        rc = SQLITE_OK;
      }
    }
  }

  返回 (rc==SQLITE_OK && bBusy) ? SQLITE_BUSY : rc;
}
#endif /* SQLITE_OMIT_WAL */

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
整数 sqlite3TempInMemory(const sqlite3 *db){
#如果 SQLITE_TEMP_STORE==1
  返回 ( db->temp_store==2 );
#endif
#如果 SQLITE_TEMP_STORE==2
  返回 ( db->temp_store!=1 );
#endif
#如果 SQLITE_TEMP_STORE==3
  UNUSED_PARAMETER(db);
  返回 1;
#endif
#如果 SQLITE_TEMP_STORE<1 || SQLITE_TEMP_STORE>3
  UNUSED_PARAMETER(db);
  返回 0;
#endif
}

/*
** Return UTF-8 encoded English language explanation of the most recent
** error.
*/
const char *sqlite3_errmsg(sqlite3 *db){
  const char *z;
  如果( !db ){
    返回 sqlite3ErrStr(SQLITE_NOMEM_BKPT);
  }
  如果( !sqlite3SafetyCheckSickOrOk(db) ){
    返回 sqlite3ErrStr(SQLITE_MISUSE_BKPT);
  }
  sqlite3_mutex_enter(db->mutex);
  如果( db->mallocFailed ){
    z = sqlite3ErrStr(SQLITE_NOMEM_BKPT);
  }否则{
    testcase( db->pErr==0 );
    z = db->errCode ? (char*)sqlite3_value_text(db->pErr) : 0;
    assert( !db->mallocFailed );
    如果( z==0 ){
      z = sqlite3ErrStr(db->errCode);
    }
  }
  sqlite3_mutex_leave(db->mutex);
  返回 z;
}

/*
** Set the error code and error message associated with the database handle.
**
** This routine is intended to be called by outside extensions (ex: the
** Session extension). Internal logic should invoke sqlite3Error() or
** sqlite3ErrorWithMsg() directly.
*/
整数 sqlite3_set_errmsg(sqlite3 *db, 整数 errcode, const char *zMsg){
  整数 rc = SQLITE_OK;
  如果( !sqlite3SafetyCheckOk(db) ){
    返回 SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(db->mutex);
  如果( zMsg ){
    sqlite3ErrorWithMsg(db, errcode, "%s", zMsg);
  }否则{
    sqlite3Error(db, errcode);
  }
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  返回 rc;
}

/*
** Return the byte offset of the most recent error
*/
整数 sqlite3_error_offset(sqlite3 *db){
  整数 iOffset = -1;
  如果( db && sqlite3SafetyCheckSickOrOk(db) ){
    sqlite3_mutex_enter(db->mutex);
    如果( db->errCode ){
      iOffset = db->errByteOffset;
    }
    sqlite3_mutex_leave(db->mutex);
  }
  返回 iOffset;
}

#ifndef SQLITE_OMIT_UTF16
/*
** Return UTF-16 encoded English language explanation of the most recent
** error.
*/
const void *sqlite3_errmsg16(sqlite3 *db){
  static const u16 outOfMem[] = {
    'o', 'u', 't', ' ', 'o', 'f', ' ', 'm', 'e', 'm', 'o', 'r', 'y', 0
  };
  static const u16 misuse[] = {
    'b', 'a', 'd', ' ', 'p', 'a', 'r', 'a', 'm', 'e', 't', 'e', 'r', ' ',
    'o', 'r', ' ', 'o', 't', 'h', 'e', 'r', ' ', 'A', 'P', 'I', ' ',
    'm', 'i', 's', 'u', 's', 'e', 0
  };

  const void *z;
  如果( !db ){
    返回 (void *)outOfMem;
  }
  如果( !sqlite3SafetyCheckSickOrOk(db) ){
    返回 (void *)misuse;
  }
  sqlite3_mutex_enter(db->mutex);
  如果( db->mallocFailed ){
    z = (void *)outOfMem;
  }否则{
    z = sqlite3_value_text16(db->pErr);
    如果( z==0 ){
      sqlite3ErrorWithMsg(db, db->errCode, sqlite3ErrStr(db->errCode));
      z = sqlite3_value_text16(db->pErr);
    }
    /* A malloc() may have failed within the call to sqlite3_value_text16()
    ** above. If this is the case, then the db->mallocFailed flag needs to
    ** be cleared before returning. Do this directly, instead of via
    ** sqlite3ApiExit(), to avoid setting the database handle error message.
    */
    sqlite3OomClear(db);
  }
  sqlite3_mutex_leave(db->mutex);
  返回 z;
}
#endif /* SQLITE_OMIT_UTF16 */

/*
** Return the most recent error code generated by an SQLite routine. If NULL is
** passed to this function, we assume a malloc() failed during sqlite3_open().
*/
整数 sqlite3_errcode(sqlite3 *db){
  整数 iRet;
  如果( !db ) 返回 SQLITE_NOMEM_BKPT;
  如果( !sqlite3SafetyCheckSickOrOk(db) ){
    返回 SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(db->mutex);
  如果( db->mallocFailed ){
    iRet = SQLITE_NOMEM_BKPT;
  }否则{
    iRet = db->errCode & db->errMask;
  }
  sqlite3_mutex_leave(db->mutex);
  返回 iRet;
}
整数 sqlite3_extended_errcode(sqlite3 *db){
  整数 iRet;
  如果( !db ) 返回 SQLITE_NOMEM_BKPT;
  如果( !sqlite3SafetyCheckSickOrOk(db) ){
    返回 SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(db->mutex);
  如果( db->mallocFailed ){
    iRet = SQLITE_NOMEM_BKPT;
  }否则{
    iRet = db->errCode;
  }
  sqlite3_mutex_leave(db->mutex);
  返回 iRet;
}
整数 sqlite3_system_errno(sqlite3 *db){
  整数 iRet = 0;
  如果( db ){
    sqlite3_mutex_enter(db->mutex);
    iRet = db->iSysErrno;
    sqlite3_mutex_leave(db->mutex);
  }
  返回 iRet;
} 

/*
** Return a string that describes the kind of error specified in the
** argument.  For now, this simply calls the internal sqlite3ErrStr()
** function.
*/
const char *sqlite3_errstr(整数 rc){
  返回 sqlite3ErrStr(rc);
}

/*
** Create a new collating function for database "db".  The name is zName
** and the encoding is enc.
*/
static 整数 createCollation(
  sqlite3* db,
  const char *zName,
  u8 enc,
  void* pCtx,
  整数(*xCompare)(void*,整数,const void*,整数,const void*),
  void(*xDel)(void*)
){
  CollSeq *pColl;
  整数 enc2;
 
  assert( sqlite3_mutex_held(db->mutex) );

  /* If SQLITE_UTF16 is specified as the encoding type, transform this
  ** to one of SQLITE_UTF16LE or SQLITE_UTF16BE using the
  ** SQLITE_UTF16NATIVE macro. SQLITE_UTF16 is not used internally.
  */
  enc2 = enc;
  testcase( enc2==SQLITE_UTF16 );
  testcase( enc2==SQLITE_UTF16_ALIGNED );
  如果( enc2==SQLITE_UTF16 || enc2==SQLITE_UTF16_ALIGNED ){
    enc2 = SQLITE_UTF16NATIVE;
  }
  如果( enc2<SQLITE_UTF8 || enc2>SQLITE_UTF16BE ){
    返回 SQLITE_MISUSE_BKPT;
  }

  /* Check if this call is removing or replacing an existing collation
  ** sequence. If so, and there are active VMs, return busy. If there
  ** are no active VMs, invalidate any pre-compiled statements.
  */
  pColl = sqlite3FindCollSeq(db, (u8)enc2, zName, 0);
  如果( pColl && pColl->xCmp ){
    如果( db->nVdbeActive ){
      sqlite3ErrorWithMsg(db, SQLITE_BUSY,
        "unable to delete/modify collation sequence due to active statements");
      返回 SQLITE_BUSY;
    }
    sqlite3ExpirePreparedStatements(db, 0);

    /* If collation sequence pColl was created directly by a call to
    ** sqlite3_create_collation, and not generated by synthCollSeq(),
    ** then any copies made by synthCollSeq() need to be invalidated.
    ** Also, collation destructor - CollSeq.xDel() - function may need
    ** to be called.
    */
    如果( (pColl->enc & ~SQLITE_UTF16_ALIGNED)==enc2 ){
      CollSeq *aColl = sqlite3HashFind(&db->aCollSeq, zName);
      整数 j;
      为(j=0; j<3; j++){
        CollSeq *p = &aColl[j];
        如果( p->enc==pColl->enc ){
          如果( p->xDel ){
            p->xDel(p->pUser);
          }
          p->xCmp = 0;
        }
      }
    }
  }

  pColl = sqlite3FindCollSeq(db, (u8)enc2, zName, 1);
  如果( pColl==0 ) 返回 SQLITE_NOMEM_BKPT;
  pColl->xCmp = xCompare;
  pColl->pUser = pCtx;
  pColl->xDel = xDel;
  pColl->enc = (u8)(enc2 | (enc & SQLITE_UTF16_ALIGNED));
  sqlite3Error(db, SQLITE_OK);
  返回 SQLITE_OK;
}


/*
** This array defines hard upper bounds on limit values.  The
** initializer must be kept in sync with the SQLITE_LIMIT_*
** #defines in sqlite3.h.
*/
static const 整数 aHardLimit[] = {
  SQLITE_MAX_LENGTH,
  SQLITE_MAX_SQL_LENGTH,
  SQLITE_MAX_COLUMN,
  SQLITE_MAX_EXPR_DEPTH,
  SQLITE_MAX_COMPOUND_SELECT,
  SQLITE_MAX_VDBE_OP,
  SQLITE_MAX_FUNCTION_ARG,
  SQLITE_MAX_ATTACHED,
  SQLITE_MAX_LIKE_PATTERN_LENGTH,
  SQLITE_MAX_VARIABLE_NUMBER,      /* IMP: R-38091-32352 */
  SQLITE_MAX_TRIGGER_DEPTH,
  SQLITE_MAX_WORKER_THREADS,
  SQLITE_MAX_PARSER_DEPTH,
  SQLITE_MAX_SCHEMA,
  SQLITE_MAX_TRIGGER_STEPS,
  SQLITE_MAX_CTES,
};

/*
** Make sure the hard limits are set to reasonable values
*/
#如果 SQLITE_MAX_LENGTH<100
# error SQLITE_MAX_LENGTH must be at least 100
#endif
#如果 SQLITE_MAX_SQL_LENGTH<100
# error SQLITE_MAX_SQL_LENGTH must be at least 100
#endif
#如果 SQLITE_MAX_SQL_LENGTH>SQLITE_MAX_LENGTH
# error SQLITE_MAX_SQL_LENGTH must not be greater than SQLITE_MAX_LENGTH
#endif
#如果 SQLITE_MAX_SQL_LENGTH>2147482624     /* 1024 less than 2^31 */
# error SQLITE_MAX_SQL_LENGTH must not be greater than 2147482624
#endif
#如果 SQLITE_MAX_COMPOUND_SELECT<2
# error SQLITE_MAX_COMPOUND_SELECT must be at least 2
#endif
#如果 SQLITE_MAX_VDBE_OP<40
# error SQLITE_MAX_VDBE_OP must be at least 40
#endif
#如果 SQLITE_MAX_FUNCTION_ARG<0 || SQLITE_MAX_FUNCTION_ARG>32767
# error SQLITE_MAX_FUNCTION_ARG must be between 0 and 32767
#endif
#如果 SQLITE_MAX_ATTACHED<0 || SQLITE_MAX_ATTACHED>125
# error SQLITE_MAX_ATTACHED must be between 0 and 125
#endif
#如果 SQLITE_MAX_LIKE_PATTERN_LENGTH<1
# error SQLITE_MAX_LIKE_PATTERN_LENGTH must be at least 1
#endif
#如果 SQLITE_MAX_COLUMN>32767
# error SQLITE_MAX_COLUMN must not exceed 32767
#endif
#如果 SQLITE_MAX_TRIGGER_DEPTH<1
# error SQLITE_MAX_TRIGGER_DEPTH must be at least 1
#endif
#如果 SQLITE_MAX_WORKER_THREADS<0 || SQLITE_MAX_WORKER_THREADS>50
# error SQLITE_MAX_WORKER_THREADS must be between 0 and 50
#endif
#如果 SQLITE_MAX_TRIGGER_STEPS<0 || SQLITE_MAX_TRIGGER_STEPS>65535
# error SQLITE_MAX_TRIGGER_STEPS must be between 0 and 65535
#endif


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
整数 sqlite3_limit(sqlite3 *db, 整数 limitId, 整数 newLimit){
  整数 oldLimit;

#ifdef SQLITE_ENABLE_API_ARMOR
  如果( !sqlite3SafetyCheckOk(db) ){
    (void)SQLITE_MISUSE_BKPT;
    返回 -1;
  }
#endif

  /* EVIDENCE-OF: R-30189-54097 For each limit category SQLITE_LIMIT_NAME
  ** there is a hard upper bound set at compile-time by a C preprocessor
  ** macro called SQLITE_MAX_NAME. (The "_LIMIT_" in the name is changed to
  ** "_MAX_".)
  */
  assert( aHardLimit[SQLITE_LIMIT_LENGTH]==SQLITE_MAX_LENGTH );
  assert( aHardLimit[SQLITE_LIMIT_SQL_LENGTH]==SQLITE_MAX_SQL_LENGTH );
  assert( aHardLimit[SQLITE_LIMIT_COLUMN]==SQLITE_MAX_COLUMN );
  assert( aHardLimit[SQLITE_LIMIT_EXPR_DEPTH]==SQLITE_MAX_EXPR_DEPTH );
  assert( aHardLimit[SQLITE_LIMIT_PARSER_DEPTH]==SQLITE_MAX_PARSER_DEPTH );
  assert( aHardLimit[SQLITE_LIMIT_COMPOUND_SELECT]==SQLITE_MAX_COMPOUND_SELECT);
  assert( aHardLimit[SQLITE_LIMIT_VDBE_OP]==SQLITE_MAX_VDBE_OP );
  assert( aHardLimit[SQLITE_LIMIT_FUNCTION_ARG]==SQLITE_MAX_FUNCTION_ARG );
  assert( aHardLimit[SQLITE_LIMIT_ATTACHED]==SQLITE_MAX_ATTACHED );
  assert( aHardLimit[SQLITE_LIMIT_LIKE_PATTERN_LENGTH]==
                                               SQLITE_MAX_LIKE_PATTERN_LENGTH );
  assert( aHardLimit[SQLITE_LIMIT_VARIABLE_NUMBER]==SQLITE_MAX_VARIABLE_NUMBER);
  assert( aHardLimit[SQLITE_LIMIT_TRIGGER_DEPTH]==SQLITE_MAX_TRIGGER_DEPTH );
  assert( aHardLimit[SQLITE_LIMIT_WORKER_THREADS]==SQLITE_MAX_WORKER_THREADS );
  assert( aHardLimit[SQLITE_LIMIT_SCHEMA]==SQLITE_MAX_SCHEMA );
  assert( aHardLimit[SQLITE_LIMIT_TRIGGER_STEPS]==SQLITE_MAX_TRIGGER_STEPS );
  assert( aHardLimit[SQLITE_LIMIT_CTES]==SQLITE_MAX_CTES );
  assert( SQLITE_LIMIT_CTES==(SQLITE_N_LIMIT-1) );


  如果( limitId<0 || limitId>=SQLITE_N_LIMIT ){
    返回 -1;
  }
  sqlite3_mutex_enter(db->mutex);
  oldLimit = db->aLimit[limitId];
  如果( newLimit>=0 ){                   /* IMP: R-52476-28732 */
    如果( newLimit>aHardLimit[limitId] ){
      newLimit = aHardLimit[limitId];  /* IMP: R-51463-25634 */
    }否则 如果( newLimit<SQLITE_MIN_LENGTH && limitId==SQLITE_LIMIT_LENGTH ){
      newLimit = SQLITE_MIN_LENGTH;
    }
    db->aLimit[limitId] = newLimit;
  }
  sqlite3_mutex_leave(db->mutex);
  返回 oldLimit;                     /* IMP: R-53341-35419 */
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
整数 sqlite3ParseUri(
  const char *zDefaultVfs,        /* VFS to use if no "vfs=xxx" query option */
  const char *zUri,               /* Nul-terminated URI to parse */
  unsigned 整数 *pFlags,           /* IN/OUT: SQLITE_OPEN_XXX flags */
  sqlite3_vfs **ppVfs,            /* OUT: VFS to use */
  char **pzFile,                  /* OUT: Filename component of URI */
  char **pzErrMsg                 /* OUT: Error message (if rc!=SQLITE_OK) */
){
  整数 rc = SQLITE_OK;
  unsigned 整数 flags = *pFlags;
  const char *zVfs = zDefaultVfs;
  char *zFile;
  char c;
  i64 nUri = strlen(zUri);

  assert( *pzErrMsg==0 );

  如果( ((flags & SQLITE_OPEN_URI)                     /* IMP: R-48725-32206 */
       || AtomicLoad(&sqlite3GlobalConfig.bOpenUri)) /* IMP: R-51689-46548 */
   && nUri>=5 && memcmp(zUri, "file:", 5)==0         /* IMP: R-57884-37496 */
  ){
    char *zOpt;
    整数 eState;                   /* Parser state when parsing URI */
    i64 iIn;                      /* Input character index */
    i64 iOut = 0;                 /* Output character index */
    u64 nByte = nUri+8;           /* Bytes of space to allocate */

    /* Make sure the SQLITE_OPEN_URI flag is set to indicate to the VFS xOpen
    ** method that there may be extra parameters following the file-name.  */
    flags |= SQLITE_OPEN_URI;

    为(iIn=0; iIn<nUri; iIn++) nByte += (zUri[iIn]=='&');
    zFile = sqlite3_malloc64(nByte);
    如果( !zFile ) 返回 SQLITE_NOMEM_BKPT;

    memset(zFile, 0, 4);  /* 4-byte of 0x00 is the start of DB name marker */
    zFile += 4;

    iIn = 5;
#ifdef SQLITE_ALLOW_URI_AUTHORITY
    如果( strncmp(zUri+5, "///", 3)==0 ){
      iIn = 7;
      /* The following condition causes URIs with five leading / characters
      ** like file://///host/path to be converted into UNCs like //host/path.
      ** The correct URI for that UNC has only two or four leading / characters
      ** file://host/path or file:////host/path.  But 5 leading slashes is a
      ** common error, we are told, so we handle it as a special case. */
      如果( strncmp(zUri+7, "///", 3)==0 ){ iIn++; }
    }否则 如果( strncmp(zUri+5, "//localhost/", 12)==0 ){
      iIn = 16;
    }
#否则
    /* Discard the scheme and authority segments of the URI. */
    如果( zUri[5]=='/' && zUri[6]=='/' ){
      iIn = 7;
      当( zUri[iIn] && zUri[iIn]!='/' ) iIn++;
      如果( iIn!=7 && (iIn!=16 || memcmp("localhost", &zUri[7], 9)) ){
        *pzErrMsg = sqlite3_mprintf("invalid uri authority: %.*s",
            (整数)(iIn-7), &zUri[7]);
        rc = SQLITE_ERROR;
        goto parse_uri_out;
      }
    }
#endif

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
    当( (c = zUri[iIn])!=0 && c!='#' ){
      iIn++;
      如果( c=='%'
       && sqlite3Isxdigit(zUri[iIn])
       && sqlite3Isxdigit(zUri[iIn+1])
      ){
        整数 octet = (sqlite3HexToInt(zUri[iIn++]) << 4);
        octet += sqlite3HexToInt(zUri[iIn++]);

        assert( octet>=0 && octet<256 );
        如果( octet==0 ){
#ifndef SQLITE_ENABLE_URI_00_ERROR
          /* This branch is taken when "%00" appears within the URI. In this
          ** case we ignore all text in the remainder of the path, name or
          ** value currently being parsed. So ignore the current character
          ** and skip to the next "?", "=" or "&", as appropriate. */
          当( (c = zUri[iIn])!=0 && c!='#'
              && (eState!=0 || c!='?')
              && (eState!=1 || (c!='=' && c!='&'))
              && (eState!=2 || c!='&')
          ){
            iIn++;
          }
          continue;
#否则
          /* If ENABLE_URI_00_ERROR is defined, "%00" in a URI is an error. */
          *pzErrMsg = sqlite3_mprintf("unexpected %%00 in uri");
          rc = SQLITE_ERROR;
          goto parse_uri_out;
#endif
        }
        c = octet;
      }否则 如果( eState==1 && (c=='&' || c=='=') ){
        如果( zFile[iOut-1]==0 ){
          /* An empty option name. Ignore this option altogether. */
          当( zUri[iIn] && zUri[iIn]!='#' && zUri[iIn-1]!='&' ) iIn++;
          continue;
        }
        如果( c=='&' ){
          zFile[iOut++] = '\0';
        }否则{
          eState = 2;
        }
        c = 0;
      }否则 如果( (eState==0 && c=='?') || (eState==2 && c=='&') ){
        c = 0;
        eState = 1;
      }
      zFile[iOut++] = c;
    }
    如果( eState==1 ) zFile[iOut++] = '\0';
    memset(zFile+iOut, 0, 4); /* end-of-options + empty journal filenames */

    /* Check if there were any options specified that should be interpreted
    ** here. Options that are interpreted here include "vfs" and those that
    ** correspond to flags that may be passed to the sqlite3_open_v2()
    ** method. */
    zOpt = &zFile[strlen(zFile)+1];
    当( zOpt[0] ){
      i64 nOpt = strlen(zOpt);
      char *zVal = &zOpt[nOpt+1];
      i64 nVal = strlen(zVal);

      如果( nOpt==3 && memcmp("vfs", zOpt, 3)==0 ){
        zVfs = zVal;
      }否则{
        struct OpenMode {
          const char *z;
          整数 mode;
        } *aMode = 0;
        char *zModeType = 0;
        整数 mask = 0;
        整数 limit = 0;

        如果( nOpt==5 && memcmp("cache", zOpt, 5)==0 ){
          static struct OpenMode aCacheMode[] = {
            { "shared",  SQLITE_OPEN_SHAREDCACHE },
            { "private", SQLITE_OPEN_PRIVATECACHE },
            { 0, 0 }
          };

          mask = SQLITE_OPEN_SHAREDCACHE|SQLITE_OPEN_PRIVATECACHE;
          aMode = aCacheMode;
          limit = mask;
          zModeType = "cache";
        }
        如果( nOpt==4 && memcmp("mode", zOpt, 4)==0 ){
          static struct OpenMode aOpenMode[] = {
            { "ro",  SQLITE_OPEN_READONLY },
            { "rw",  SQLITE_OPEN_READWRITE },
            { "rwc", SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE },
            { "memory", SQLITE_OPEN_MEMORY },
            { 0, 0 }
          };

          mask = SQLITE_OPEN_READONLY | SQLITE_OPEN_READWRITE
                   | SQLITE_OPEN_CREATE | SQLITE_OPEN_MEMORY;
          aMode = aOpenMode;
          limit = mask & flags;
          zModeType = "access";
        }

        如果( aMode ){
          整数 i;
          整数 mode = 0;
          为(i=0; aMode[i].z; i++){
            const char *z = aMode[i].z;
            如果( nVal==(i64)strlen(z) && 0==memcmp(zVal, z, nVal) ){
              mode = aMode[i].mode;
              break;
            }
          }
          如果( mode==0 ){
            *pzErrMsg = sqlite3_mprintf("no such %s mode: %s", zModeType, zVal);
            rc = SQLITE_ERROR;
            goto parse_uri_out;
          }
          如果( (mode & ~SQLITE_OPEN_MEMORY)>limit ){
            *pzErrMsg = sqlite3_mprintf("%s mode not allowed: %s",
                                        zModeType, zVal);
            rc = SQLITE_PERM;
            goto parse_uri_out;
          }
          flags = (flags & ~mask) | mode;
        }
      }

      zOpt = &zVal[nVal+1];
    }

  }否则{
    zFile = sqlite3_malloc64(nUri+8);
    如果( !zFile ) 返回 SQLITE_NOMEM_BKPT;
    memset(zFile, 0, 4);
    zFile += 4;
    如果( nUri ){
      memcpy(zFile, zUri, nUri);
    }
    memset(zFile+nUri, 0, 4);
    flags &= ~SQLITE_OPEN_URI;
  }

  *ppVfs = sqlite3_vfs_find(zVfs);
  如果( *ppVfs==0 ){
    *pzErrMsg = sqlite3_mprintf("no such vfs: %s", zVfs);
    rc = SQLITE_ERROR;
  }
 parse_uri_out:
  如果( rc!=SQLITE_OK ){
    sqlite3_free_filename(zFile);
    zFile = 0;
  }
  *pFlags = flags;
  *pzFile = zFile;
  返回 rc;
}

/*
** This routine does the core work of extracting URI parameters from a
** database filename for the sqlite3_uri_parameter() interface.
*/
static const char *uriParameter(const char *zFilename, const char *zParam){
  zFilename += sqlite3Strlen30(zFilename) + 1;
  当( ALWAYS(zFilename!=0) && zFilename[0] ){
    整数 x = strcmp(zFilename, zParam);
    zFilename += sqlite3Strlen30(zFilename) + 1;
    如果( x==0 ) 返回 zFilename;
    zFilename += sqlite3Strlen30(zFilename) + 1;
  }
  返回 0;
}



/*
** This routine does the work of opening a database on behalf of
** sqlite3_open() and sqlite3_open16(). The database filename "zFilename" 
** is UTF-8 encoded.
*/
static 整数 openDatabase(
  const char *zFilename, /* Database filename UTF-8 encoded */
  sqlite3 **ppDb,        /* OUT: Returned database handle */
  unsigned 整数 flags,    /* Operational flags */
  const char *zVfs       /* Name of the VFS to use */
){
  sqlite3 *db;                    /* Store allocated handle here */
  整数 rc;                         /* Return code */
  整数 isThreadsafe;               /* True for threadsafe connections */
  char *zOpen = 0;                /* Filename argument to pass to BtreeOpen() */
  char *zErrMsg = 0;              /* Error message from sqlite3ParseUri() */
  整数 i;                          /* Loop counter */

#ifdef SQLITE_ENABLE_API_ARMOR
  如果( ppDb==0 ) 返回 SQLITE_MISUSE_BKPT;
#endif
  *ppDb = 0;
#ifndef SQLITE_OMIT_AUTOINIT
  rc = sqlite3_initialize();
  如果( rc ) 返回 rc;
#endif

  如果( sqlite3GlobalConfig.bCoreMutex==0 ){
    isThreadsafe = 0;
  }否则 如果( flags & SQLITE_OPEN_NOMUTEX ){
    isThreadsafe = 0;
  }否则 如果( flags & SQLITE_OPEN_FULLMUTEX ){
    isThreadsafe = 1;
  }否则{
    isThreadsafe = sqlite3GlobalConfig.bFullMutex;
  }

  如果( flags & SQLITE_OPEN_PRIVATECACHE ){
    flags &= ~SQLITE_OPEN_SHAREDCACHE;
  }否则 如果( sqlite3GlobalConfig.sharedCacheEnabled ){
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
               SQLITE_OPEN_TEMP_DB |
               SQLITE_OPEN_TRANSIENT_DB |
               SQLITE_OPEN_MAIN_JOURNAL |
               SQLITE_OPEN_TEMP_JOURNAL |
               SQLITE_OPEN_SUBJOURNAL |
               SQLITE_OPEN_SUPER_JOURNAL |
               SQLITE_OPEN_NOMUTEX |
               SQLITE_OPEN_FULLMUTEX |
               SQLITE_OPEN_WAL
             );

  /* Allocate the sqlite data structure */
  db = sqlite3MallocZero( sizeof(sqlite3) );
  如果( db==0 ) goto opendb_out;
  如果( isThreadsafe
#如果 defined(SQLITE_THREAD_MISUSE_WARNINGS)
   || sqlite3GlobalConfig.bCoreMutex
#endif
  ){
    db->mutex = sqlite3MutexAlloc(SQLITE_MUTEX_RECURSIVE);
    如果( db->mutex==0 ){
      sqlite3_free(db);
      db = 0;
      goto opendb_out;
    }
    如果( isThreadsafe==0 ){
      sqlite3MutexWarnOnContention(db->mutex);
    }
  }
  sqlite3_mutex_enter(db->mutex);
  db->errMask = (flags & SQLITE_OPEN_EXRESCODE)!=0 ? 0xffffffff : 0xff;
  db->nDb = 2;
  db->eOpenState = SQLITE_STATE_BUSY;
  db->aDb = db->aDbStatic;
  db->lookaside.bDisable = 1;
  db->lookaside.sz = 0;
  db->nFpDigit = 17;

  assert( sizeof(db->aLimit)==sizeof(aHardLimit) );
  memcpy(db->aLimit, aHardLimit, sizeof(db->aLimit));
  db->aLimit[SQLITE_LIMIT_WORKER_THREADS] = SQLITE_DEFAULT_WORKER_THREADS;
  db->autoCommit = 1;
  db->nextAutovac = -1;
  db->szMmap = sqlite3GlobalConfig.szMmap;
  db->nextPagesize = 0;
  db->init.azInit = sqlite3StdType; /* Any array of string ptrs will do */
#ifdef SQLITE_ENABLE_SORTER_MMAP
  /* Beginning with version 3.37.0, using the VFS xFetch() API to memory-map
  ** the temporary files used to do external sorts (see code in vdbesort.c)
  ** is disabled. It can still be used either by defining
  ** SQLITE_ENABLE_SORTER_MMAP at compile time or by using the
  ** SQLITE_TESTCTRL_SORTER_MMAP test-control at runtime. */
  db->nMaxSorterMmap = 0x7FFFFFFF;
#endif
  db->flags |= SQLITE_ShortColNames
                 | SQLITE_EnableTrigger
                 | SQLITE_EnableView
                 | SQLITE_CacheSpill
                 | SQLITE_AttachCreate
                 | SQLITE_AttachWrite
                 | SQLITE_Comments
#如果 !defined(SQLITE_TRUSTED_SCHEMA) || SQLITE_TRUSTED_SCHEMA+0!=0
                 | SQLITE_TrustedSchema
#endif
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
#如果 !defined(SQLITE_DQS)
# define SQLITE_DQS 3
#endif
#如果 (SQLITE_DQS&1)==1
                 | SQLITE_DqsDML
#endif
#如果 (SQLITE_DQS&2)==2
                 | SQLITE_DqsDDL
#endif

#如果 !defined(SQLITE_DEFAULT_AUTOMATIC_INDEX) || SQLITE_DEFAULT_AUTOMATIC_INDEX
                 | SQLITE_AutoIndex
#endif
#如果 SQLITE_DEFAULT_CKPTFULLFSYNC
                 | SQLITE_CkptFullFSync
#endif
#如果 SQLITE_DEFAULT_FILE_FORMAT<4
                 | SQLITE_LegacyFileFmt
#endif
#ifdef SQLITE_ENABLE_LOAD_EXTENSION
                 | SQLITE_LoadExtension
#endif
#如果 SQLITE_DEFAULT_RECURSIVE_TRIGGERS
                 | SQLITE_RecTriggers
#endif
#如果 defined(SQLITE_DEFAULT_FOREIGN_KEYS) && SQLITE_DEFAULT_FOREIGN_KEYS
                 | SQLITE_ForeignKeys
#endif
#如果 defined(SQLITE_REVERSE_UNORDERED_SELECTS)
                 | SQLITE_ReverseOrder
#endif
#如果 defined(SQLITE_ENABLE_OVERSIZE_CELL_CHECK)
                 | SQLITE_CellSizeCk
#endif
#如果 defined(SQLITE_ENABLE_FTS3_TOKENIZER)
                 | SQLITE_Fts3Tokenizer
#endif
#如果 defined(SQLITE_ENABLE_QPSG)
                 | SQLITE_EnableQPSG
#endif
#如果 defined(SQLITE_DEFAULT_DEFENSIVE)
                 | SQLITE_Defensive
#endif
#如果 defined(SQLITE_DEFAULT_LEGACY_ALTER_TABLE)
                 | SQLITE_LegacyAlter
#endif
#如果 defined(SQLITE_ENABLE_STMT_SCANSTATUS)
                 | SQLITE_StmtScanStatus
#endif
      ;
  sqlite3HashInit(&db->aCollSeq);
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3HashInit(&db->aModule);
#endif

  /* Add the default collation sequence BINARY. BINARY works for both UTF-8
  ** and UTF-16, so add a version for each to avoid any unnecessary
  ** conversions. The only error that can occur here is a malloc() failure.
  **
  ** EVIDENCE-OF: R-52786-44878 SQLite defines three built-in collating
  ** functions:
  */
  createCollation(db, sqlite3StrBINARY, SQLITE_UTF8, 0,
                  sqlite3BinaryCompare, 0);
  createCollation(db, sqlite3StrBINARY, SQLITE_UTF16BE, 0,
                  sqlite3BinaryCompare, 0);
  createCollation(db, sqlite3StrBINARY, SQLITE_UTF16LE, 0,
                  sqlite3BinaryCompare, 0);
  createCollation(db, "NOCASE", SQLITE_UTF8, 0, nocaseCollatingFunc, 0);
  createCollation(db, "RTRIM", SQLITE_UTF8, 0, rtrimCollFunc, 0);
  如果( db->mallocFailed ){
    goto opendb_out;
  }

#如果 SQLITE_OS_UNIX && defined(SQLITE_OS_KV_OPTIONAL)
  /* Process magic filenames ":localStorage:" and ":sessionStorage:" */
  如果( zFilename && zFilename[0]==':' ){
    如果( strcmp(zFilename, ":localStorage:")==0 ){
      zFilename = "file:local?vfs=kvvfs";
      flags |= SQLITE_OPEN_URI;
    }否则 如果( strcmp(zFilename, ":sessionStorage:")==0 ){
      zFilename = "file:session?vfs=kvvfs";
      flags |= SQLITE_OPEN_URI;
    }
  }
#endif /* SQLITE_OS_UNIX && defined(SQLITE_OS_KV_OPTIONAL) */

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
  assert( SQLITE_OPEN_READONLY  == 0x01 );
  assert( SQLITE_OPEN_READWRITE == 0x02 );
  assert( SQLITE_OPEN_CREATE    == 0x04 );
  testcase( (1<<(flags&7))==0x02 ); /* READONLY */
  testcase( (1<<(flags&7))==0x04 ); /* READWRITE */
  testcase( (1<<(flags&7))==0x40 ); /* READWRITE | CREATE */
  如果( ((1<<(flags&7)) & 0x46)==0 ){
    rc = SQLITE_MISUSE_BKPT;  /* IMP: R-18321-05872 */
  }否则{
    如果( zFilename==0 ) zFilename = ":memory:";
    rc = sqlite3ParseUri(zVfs, zFilename, &flags, &db->pVfs, &zOpen, &zErrMsg);
  }
  如果( rc!=SQLITE_OK ){
    如果( rc==SQLITE_NOMEM ) sqlite3OomFault(db);
    sqlite3ErrorWithMsg(db, rc, zErrMsg ? "%s" : 0, zErrMsg);
    sqlite3_free(zErrMsg);
    goto opendb_out;
  }
  assert( db->pVfs!=0 );
#如果 SQLITE_OS_KV || defined(SQLITE_OS_KV_OPTIONAL)
  如果( sqlite3_stricmp(db->pVfs->zName, "kvvfs")==0 ){
    db->temp_store = 2;
  }
#endif

  /* Open the backend database driver */
  rc = sqlite3BtreeOpen(db->pVfs, zOpen, db, &db->aDb[0].pBt, 0,
                        flags | SQLITE_OPEN_MAIN_DB);
  如果( rc!=SQLITE_OK ){
    如果( rc==SQLITE_IOERR_NOMEM ){
      rc = SQLITE_NOMEM_BKPT;
    }
    sqlite3Error(db, rc);
    goto opendb_out;
  }
  sqlite3BtreeEnter(db->aDb[0].pBt);
  db->aDb[0].pSchema = sqlite3SchemaGet(db, db->aDb[0].pBt);
  如果( !db->mallocFailed ){
    sqlite3SetTextEncoding(db, SCHEMA_ENC(db));
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
  如果( db->mallocFailed ){
    goto opendb_out;
  }

  /* Register all built-in functions, but do not attempt to read the
  ** database schema yet. This is delayed until the first time the database
  ** is accessed.
  */
  sqlite3Error(db, SQLITE_OK);
  sqlite3RegisterPerConnectionBuiltinFunctions(db);
  rc = sqlite3_errcode(db);


  /* Load compiled-in extensions */
  为(i=0; rc==SQLITE_OK && i<ArraySize(sqlite3BuiltinExtensions); i++){
    rc = sqlite3BuiltinExtensions[i](db);
  }

  /* Load automatic extensions - extensions that have been registered
  ** using the sqlite3_automatic_extension() API.
  */
  如果( rc==SQLITE_OK ){
    sqlite3AutoLoadExtensions(db);
    rc = sqlite3_errcode(db);
    如果( rc!=SQLITE_OK ){
      goto opendb_out;
    }
  }

#ifdef SQLITE_ENABLE_INTERNAL_FUNCTIONS
  /* Testing use only!!! The -DSQLITE_ENABLE_INTERNAL_FUNCTIONS=1 compile-time
  ** option gives access to internal functions by default. 
  ** Testing use only!!! */
  db->mDbFlags |= DBFLAG_InternalFunc;
#endif

  /* -DSQLITE_DEFAULT_LOCKING_MODE=1 makes EXCLUSIVE the default locking
  ** mode.  -DSQLITE_DEFAULT_LOCKING_MODE=0 make NORMAL the default locking
  ** mode.  Doing nothing at all also makes NORMAL the default.
  */
#ifdef SQLITE_DEFAULT_LOCKING_MODE
  db->dfltLockMode = SQLITE_DEFAULT_LOCKING_MODE;
  sqlite3PagerLockingMode(sqlite3BtreePager(db->aDb[0].pBt),
                          SQLITE_DEFAULT_LOCKING_MODE);
#endif

  如果( rc ) sqlite3Error(db, rc);

  /* Enable the lookaside-malloc subsystem */
  setupLookaside(db, 0, sqlite3GlobalConfig.szLookaside,
                        sqlite3GlobalConfig.nLookaside);

  sqlite3_wal_autocheckpoint(db, SQLITE_DEFAULT_WAL_AUTOCHECKPOINT);

opendb_out:
  如果( db ){
    assert( db->mutex!=0 || isThreadsafe==0
           || sqlite3GlobalConfig.bFullMutex==0 );
    sqlite3_mutex_leave(db->mutex);
  }
  rc = sqlite3_errcode(db);
  assert( db!=0 || (rc&0xff)==SQLITE_NOMEM );
  如果( (rc&0xff)==SQLITE_NOMEM ){
    sqlite3_close(db);
    db = 0;
  }否则 如果( rc!=SQLITE_OK ){
    db->eOpenState = SQLITE_STATE_SICK;
  }
  *ppDb = db;
#ifdef SQLITE_ENABLE_SQLLOG
  如果( sqlite3GlobalConfig.xSqllog ){
    /* Opening a db handle. Fourth parameter is passed 0. */
    void *pArg = sqlite3GlobalConfig.pSqllogArg;
    sqlite3GlobalConfig.xSqllog(pArg, db, zFilename, 0);
  }
#endif
  sqlite3_free_filename(zOpen);
  返回 rc;
}


/*
** Open a new database handle.
*/
整数 sqlite3_open(
  const char *zFilename,
  sqlite3 **ppDb
){
  返回 openDatabase(zFilename, ppDb,
                      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, 0);
}
整数 sqlite3_open_v2(
  const char *filename,   /* Database filename (UTF-8) */
  sqlite3 **ppDb,         /* OUT: SQLite db handle */
  整数 flags,              /* Flags */
  const char *zVfs        /* Name of VFS module to use */
){
  返回 openDatabase(filename, ppDb, (unsigned 整数)flags, zVfs);
}

#ifndef SQLITE_OMIT_UTF16
/*
** Open a new database handle.
*/
整数 sqlite3_open16(
  const void *zFilename,
  sqlite3 **ppDb
){
  char const *zFilename8;   /* zFilename encoded in UTF-8 instead of UTF-16 */
  sqlite3_value *pVal;
  整数 rc;

#ifdef SQLITE_ENABLE_API_ARMOR
  如果( ppDb==0 ) 返回 SQLITE_MISUSE_BKPT;
#endif
  *ppDb = 0;
#ifndef SQLITE_OMIT_AUTOINIT
  rc = sqlite3_initialize();
  如果( rc ) 返回 rc;
#endif
  如果( zFilename==0 ) zFilename = "\000\000";
  pVal = sqlite3ValueNew(0);
  sqlite3ValueSetStr(pVal, -1, zFilename, SQLITE_UTF16NATIVE, SQLITE_STATIC);
  zFilename8 = sqlite3ValueText(pVal, SQLITE_UTF8);
  如果( zFilename8 ){
    rc = openDatabase(zFilename8, ppDb,
                      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, 0);
    assert( *ppDb || rc==SQLITE_NOMEM );
    如果( rc==SQLITE_OK && !DbHasProperty(*ppDb, 0, DB_SchemaLoaded) ){
      SCHEMA_ENC(*ppDb) = ENC(*ppDb) = SQLITE_UTF16NATIVE;
    }
  }否则{
    rc = SQLITE_NOMEM_BKPT;
  }
  sqlite3ValueFree(pVal);

  返回 rc & 0xff;
}
#endif /* SQLITE_OMIT_UTF16 */

/*
** Register a new collation sequence with the database handle db.
*/
整数 sqlite3_create_collation(
  sqlite3* db,
  const char *zName,
  整数 enc,
  void* pCtx,
  整数(*xCompare)(void*,整数,const void*,整数,const void*)
){
  返回 sqlite3_create_collation_v2(db, zName, enc, pCtx, xCompare, 0);
}

/*
** Register a new collation sequence with the database handle db.
*/
整数 sqlite3_create_collation_v2(
  sqlite3* db,
  const char *zName,
  整数 enc,
  void* pCtx,
  整数(*xCompare)(void*,整数,const void*,整数,const void*),
  void(*xDel)(void*)
){
  整数 rc;

#ifdef SQLITE_ENABLE_API_ARMOR
  如果( !sqlite3SafetyCheckOk(db) || zName==0 ) 返回 SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(db->mutex);
  assert( !db->mallocFailed );
  rc = createCollation(db, zName, (u8)enc, pCtx, xCompare, xDel);
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  返回 rc;
}

#ifndef SQLITE_OMIT_UTF16
/*
** Register a new collation sequence with the database handle db.
*/
整数 sqlite3_create_collation16(
  sqlite3* db,
  const void *zName,
  整数 enc,
  void* pCtx,
  整数(*xCompare)(void*,整数,const void*,整数,const void*)
){
  整数 rc = SQLITE_OK;
  char *zName8;

#ifdef SQLITE_ENABLE_API_ARMOR
  如果( !sqlite3SafetyCheckOk(db) || zName==0 ) 返回 SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(db->mutex);
  assert( !db->mallocFailed );
  zName8 = sqlite3Utf16to8(db, zName, -1, SQLITE_UTF16NATIVE);
  如果( zName8 ){
    rc = createCollation(db, zName8, (u8)enc, pCtx, xCompare, 0);
    sqlite3DbFree(db, zName8);
  }
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  返回 rc;
}
#endif /* SQLITE_OMIT_UTF16 */

/*
** Register a collation sequence factory callback with the database handle
** db. Replace any previously installed collation sequence factory.
*/
整数 sqlite3_collation_needed(
  sqlite3 *db,
  void *pCollNeededArg,
  void(*xCollNeeded)(void*,sqlite3*,整数 eTextRep,const char*)
){
#ifdef SQLITE_ENABLE_API_ARMOR
  如果( !sqlite3SafetyCheckOk(db) ) 返回 SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(db->mutex);
  db->xCollNeeded = xCollNeeded;
  db->xCollNeeded16 = 0;
  db->pCollNeededArg = pCollNeededArg;
  sqlite3_mutex_leave(db->mutex);
  返回 SQLITE_OK;
}

#ifndef SQLITE_OMIT_UTF16
/*
** Register a collation sequence factory callback with the database handle
** db. Replace any previously installed collation sequence factory.
*/
整数 sqlite3_collation_needed16(
  sqlite3 *db,
  void *pCollNeededArg,
  void(*xCollNeeded16)(void*,sqlite3*,整数 eTextRep,const void*)
){
#ifdef SQLITE_ENABLE_API_ARMOR
  如果( !sqlite3SafetyCheckOk(db) ) 返回 SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(db->mutex);
  db->xCollNeeded = 0;
  db->xCollNeeded16 = xCollNeeded16;
  db->pCollNeededArg = pCollNeededArg;
  sqlite3_mutex_leave(db->mutex);
  返回 SQLITE_OK;
}
#endif /* SQLITE_OMIT_UTF16 */

/*
** Find existing client data.
*/
void *sqlite3_get_clientdata(sqlite3 *db, const char *zName){
  DbClientData *p;
#ifdef SQLITE_ENABLE_API_ARMOR
  如果( !zName || !sqlite3SafetyCheckOk(db) ){
    (void)SQLITE_MISUSE_BKPT;
    返回 0;
  }
#endif
  sqlite3_mutex_enter(db->mutex);
  为(p=db->pDbData; p; p=p->pNext){
    如果( strcmp(p->zName, zName)==0 ){
      void *pResult = p->pData;
      sqlite3_mutex_leave(db->mutex);
      返回 pResult;
    }
  }
  sqlite3_mutex_leave(db->mutex);
  返回 0;
}

/*
** Add new client data to a database connection.
*/
整数 sqlite3_set_clientdata(
  sqlite3 *db,                   /* Attach client data to this connection */
  const char *zName,             /* Name of the client data */
  void *pData,                   /* The client data itself */
  void (*xDestructor)(void*)     /* Destructor */
){
  DbClientData *p, **pp;
  sqlite3_mutex_enter(db->mutex);
  pp = &db->pDbData;
  为(p=db->pDbData; p && strcmp(p->zName,zName); p=p->pNext){
    pp = &p->pNext;
  }
  如果( p ){
    assert( p->pData!=0 );
    如果( p->xDestructor ) p->xDestructor(p->pData);
    如果( pData==0 ){
      *pp = p->pNext;
      sqlite3_free(p);
      sqlite3_mutex_leave(db->mutex);
      返回 SQLITE_OK;
    }
  }否则 如果( pData==0 ){
    sqlite3_mutex_leave(db->mutex);
    返回 SQLITE_OK;
  }否则{
    size_t n = strlen(zName);
    p = sqlite3_malloc64( SZ_DBCLIENTDATA(n+1) );
    如果( p==0 ){
      如果( xDestructor ) xDestructor(pData);
      sqlite3_mutex_leave(db->mutex);
      返回 SQLITE_NOMEM;
    }
    memcpy(p->zName, zName, n+1);
    p->pNext = db->pDbData;
    db->pDbData = p;
  }
  p->pData = pData;
  p->xDestructor = xDestructor;
  sqlite3_mutex_leave(db->mutex);
  返回 SQLITE_OK;
}


#ifndef SQLITE_OMIT_DEPRECATED
/*
** This function is now an anachronism. It used to be used to recover from a
** malloc() failure, but SQLite now does this automatically.
*/
整数 sqlite3_global_recover(void){
  返回 SQLITE_OK;
}
#endif

/*
** Test to see whether or not the database connection is in autocommit
** mode.  Return TRUE if it is and FALSE if not.  Autocommit mode is on
** by default.  Autocommit is disabled by a BEGIN statement and reenabled
** by the next COMMIT or ROLLBACK.
*/
整数 sqlite3_get_autocommit(sqlite3 *db){
  整数 iRet;
#ifdef SQLITE_ENABLE_API_ARMOR
  如果( !sqlite3SafetyCheckOk(db) ){
    (void)SQLITE_MISUSE_BKPT;
    返回 0;
  }
#endif
  sqlite3_mutex_enter(db->mutex);
  iRet = db->autoCommit;
  sqlite3_mutex_leave(db->mutex);
  返回 iRet;
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
整数 sqlite3ReportError(整数 iErr, 整数 lineno, const char *zType){
  sqlite3_log(iErr, "%s at line %d of [%.10s]",
              zType, lineno, 20+sqlite3_sourceid());
  返回 iErr;
}
整数 sqlite3CorruptError(整数 lineno){
  testcase( sqlite3GlobalConfig.xLog!=0 );
  返回 sqlite3ReportError(SQLITE_CORRUPT, lineno, "database corruption");
}
整数 sqlite3MisuseError(整数 lineno){
  testcase( sqlite3GlobalConfig.xLog!=0 );
  返回 sqlite3ReportError(SQLITE_MISUSE, lineno, "misuse");
}
整数 sqlite3CantopenError(整数 lineno){
  testcase( sqlite3GlobalConfig.xLog!=0 );
  返回 sqlite3ReportError(SQLITE_CANTOPEN, lineno, "cannot open file");
}
#如果 defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_CORRUPT_PGNO)
整数 sqlite3CorruptPgnoError(整数 lineno, Pgno pgno){
  char zMsg[100];
  sqlite3_snprintf(sizeof(zMsg), zMsg, "database corruption page %d", pgno);
  testcase( sqlite3GlobalConfig.xLog!=0 );
  返回 sqlite3ReportError(SQLITE_CORRUPT, lineno, zMsg);
}
#endif
#ifdef SQLITE_DEBUG
整数 sqlite3NomemError(整数 lineno){
  testcase( sqlite3GlobalConfig.xLog!=0 );
  返回 sqlite3ReportError(SQLITE_NOMEM, lineno, "OOM");
}
整数 sqlite3IoerrnomemError(整数 lineno){
  testcase( sqlite3GlobalConfig.xLog!=0 );
  返回 sqlite3ReportError(SQLITE_IOERR_NOMEM, lineno, "I/O OOM error");
}
#endif

#ifndef SQLITE_OMIT_DEPRECATED
/*
** This is a convenience routine that makes sure that all thread-specific
** data for this thread has been deallocated.
**
** SQLite no longer uses thread-specific data so this routine is now a
** no-op.  It is retained for historical compatibility.
*/
void sqlite3_thread_cleanup(void){
}
#endif

/*
** Return meta information about a specific column of a database table.
** See comment in sqlite3.h (sqlite.h.in) for details.
*/
整数 sqlite3_table_column_metadata(
  sqlite3 *db,                /* Connection handle */
  const char *zDbName,        /* Database name or NULL */
  const char *zTableName,     /* Table name */
  const char *zColumnName,    /* Column name */
  char const **pzDataType,    /* OUTPUT: Declared data type */
  char const **pzCollSeq,     /* OUTPUT: Collation sequence name */
  整数 *pNotNull,              /* OUTPUT: True if NOT NULL constraint exists */
  整数 *pPrimaryKey,           /* OUTPUT: True if column part of PK */
  整数 *pAutoinc               /* OUTPUT: True if column is auto-increment */
){
  整数 rc;
  char *zErrMsg = 0;
  Table *pTab = 0;
  Column *pCol = 0;
  整数 iCol = 0;
  char const *zDataType = 0;
  char const *zCollSeq = 0;
  整数 notnull = 0;
  整数 primarykey = 0;
  整数 autoinc = 0;


#ifdef SQLITE_ENABLE_API_ARMOR
  如果( !sqlite3SafetyCheckOk(db) || zTableName==0 ){
    返回 SQLITE_MISUSE_BKPT;
  }
#endif

  /* Ensure the database schema has been loaded */
  sqlite3_mutex_enter(db->mutex);
  sqlite3BtreeEnterAll(db);
  rc = sqlite3Init(db, &zErrMsg);
  如果( SQLITE_OK!=rc ){
    goto error_out;
  }

  /* Locate the table in question */
  pTab = sqlite3FindTable(db, zTableName, zDbName);
  如果( !pTab || IsView(pTab) ){
    pTab = 0;
    goto error_out;
  }

  /* Find the column for which info is requested */
  如果( zColumnName==0 ){
    /* Query for existence of table only */
  }否则{
    iCol = sqlite3ColumnIndex(pTab, zColumnName);
    如果( iCol>=0 ){
      pCol = &pTab->aCol[iCol];
    }否则{
      如果( HasRowid(pTab) && sqlite3IsRowid(zColumnName) ){
        iCol = pTab->iPKey;
        pCol = iCol>=0 ? &pTab->aCol[iCol] : 0;
      }否则{
        pTab = 0;
        goto error_out;
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
  如果( pCol ){
    zDataType = sqlite3ColumnType(pCol,0);
    zCollSeq = sqlite3ColumnColl(pCol);
    notnull = pCol->notNull!=0;
    primarykey  = (pCol->colFlags & COLFLAG_PRIMKEY)!=0;
    autoinc = pTab->iPKey==iCol && (pTab->tabFlags & TF_Autoincrement)!=0;
  }否则{
    zDataType = "INTEGER";
    primarykey = 1;
  }
  如果( !zCollSeq ){
    zCollSeq = sqlite3StrBINARY;
  }

error_out:
  sqlite3BtreeLeaveAll(db);

  /* Whether the function call succeeded or failed, set the output parameters
  ** to whatever their local counterparts contain. If an error did occur,
  ** this has the effect of zeroing all output parameters.
  */
  如果( pzDataType ) *pzDataType = zDataType;
  如果( pzCollSeq ) *pzCollSeq = zCollSeq;
  如果( pNotNull ) *pNotNull = notnull;
  如果( pPrimaryKey ) *pPrimaryKey = primarykey;
  如果( pAutoinc ) *pAutoinc = autoinc;

  如果( SQLITE_OK==rc && !pTab ){
    sqlite3DbFree(db, zErrMsg);
    zErrMsg = sqlite3MPrintf(db, "no such table column: %s.%s", zTableName,
        zColumnName);
    rc = SQLITE_ERROR;
  }
  sqlite3ErrorWithMsg(db, rc, (zErrMsg?"%s":0), zErrMsg);
  sqlite3DbFree(db, zErrMsg);
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  返回 rc;
}

/*
** Sleep for a little while.  Return the amount of time slept.
*/
整数 sqlite3_sleep(整数 ms){
  sqlite3_vfs *pVfs;
  整数 rc;
  pVfs = sqlite3_vfs_find(0);
  如果( pVfs==0 ) 返回 0;

  /* This function works in milliseconds, but the underlying OsSleep()
  ** API uses microseconds. Hence the 1000's.
  */
  rc = (sqlite3OsSleep(pVfs, ms<0 ? 0 : 1000*ms)/1000);
  返回 rc;
}

/*
** Enable or disable the extended result codes.
*/
整数 sqlite3_extended_result_codes(sqlite3 *db, 整数 onoff){
#ifdef SQLITE_ENABLE_API_ARMOR
  如果( !sqlite3SafetyCheckOk(db) ) 返回 SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(db->mutex);
  db->errMask = onoff ? 0xffffffff : 0xff;
  sqlite3_mutex_leave(db->mutex);
  返回 SQLITE_OK;
}

/*
** Invoke the xFileControl method on a particular database.
*/
整数 sqlite3_file_control(sqlite3 *db, const char *zDbName, 整数 op, void *pArg){
  整数 rc = SQLITE_ERROR;
  Btree *pBtree;

#ifdef SQLITE_ENABLE_API_ARMOR
  如果( !sqlite3SafetyCheckOk(db) ) 返回 SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(db->mutex);
  pBtree = sqlite3DbNameToBtree(db, zDbName);
  如果( pBtree ){
    Pager *pPager;
    sqlite3_file *fd;
    sqlite3BtreeEnter(pBtree);
    pPager = sqlite3BtreePager(pBtree);
    assert( pPager!=0 );
    fd = sqlite3PagerFile(pPager);
    assert( fd!=0 );
    如果( op==SQLITE_FCNTL_FILE_POINTER ){
      *(sqlite3_file**)pArg = fd;
      rc = SQLITE_OK;
    }否则 如果( op==SQLITE_FCNTL_VFS_POINTER ){
      *(sqlite3_vfs**)pArg = sqlite3PagerVfs(pPager);
      rc = SQLITE_OK;
    }否则 如果( op==SQLITE_FCNTL_JOURNAL_POINTER ){
      *(sqlite3_file**)pArg = sqlite3PagerJrnlFile(pPager);
      rc = SQLITE_OK;
    }否则 如果( op==SQLITE_FCNTL_DATA_VERSION ){
      *(unsigned 整数*)pArg = sqlite3PagerDataVersion(pPager);
      rc = SQLITE_OK;
    }否则 如果( op==SQLITE_FCNTL_RESERVE_BYTES ){
      整数 iNew = *(整数*)pArg;
      *(整数*)pArg = sqlite3BtreeGetRequestedReserve(pBtree);
      如果( iNew>=0 && iNew<=255 ){
        sqlite3BtreeSetPageSize(pBtree, 0, iNew, 0);
      }
      rc = SQLITE_OK;
    }否则 如果( op==SQLITE_FCNTL_RESET_CACHE ){
      sqlite3BtreeClearCache(pBtree);
      rc = SQLITE_OK;
    }否则{
      整数 nSave = db->busyHandler.nBusy;
      rc = sqlite3OsFileControl(fd, op, pArg);
      db->busyHandler.nBusy = nSave;
    }
    sqlite3BtreeLeave(pBtree);
  }
  sqlite3_mutex_leave(db->mutex);
  返回 rc;
}

/*
** Interface to the testing logic.
*/
整数 sqlite3_test_control(整数 op, ...){
  整数 rc = 0;
#ifdef SQLITE_UNTESTABLE
  UNUSED_PARAMETER(op);
#否则
  va_list ap;
  va_start(ap, op);
  switch( op ){

    /*
    ** Save the current state of the PRNG.
    */
    case SQLITE_TESTCTRL_PRNG_SAVE: {
      sqlite3PrngSaveState();
      break;
    }

    /*
    ** Restore the state of the PRNG to the last state saved using
    ** PRNG_SAVE.  If PRNG_SAVE has never before been called, then
    ** this verb acts like PRNG_RESET.
    */
    case SQLITE_TESTCTRL_PRNG_RESTORE: {
      sqlite3PrngRestoreState();
      break;
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
#ifndef SQLITE_OMIT_WSD
    case SQLITE_TESTCTRL_PRNG_SEED: {
      整数 x = va_arg(ap, 整数);
      整数 y;
      sqlite3 *db = va_arg(ap, sqlite3*);
      assert( db==0 || db->aDb[0].pSchema!=0 );
      如果( db && (y = db->aDb[0].pSchema->schema_cookie)!=0 ){ x = y; }
      sqlite3Config.iPrngSeed = x;
      sqlite3_randomness(0,0);
      break;
    }
#endif

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
    case SQLITE_TESTCTRL_FK_NO_ACTION: {
      sqlite3 *db = va_arg(ap, sqlite3*);
      整数 b = va_arg(ap, 整数);
      如果( b ){
        db->flags |= SQLITE_FkNoAction;
      }否则{
        db->flags &= ~SQLITE_FkNoAction;
      }
      break;
    }

    /*
    **  sqlite3_test_control(BITVEC_TEST, size, program)
    **
    ** Run a test against a Bitvec object of size.  The program argument
    ** is an array of integers that defines the test.  Return -1 on a
    ** memory allocation error, 0 on success, or non-zero for an error.
    ** See the sqlite3BitvecBuiltinTest() for additional information.
    */
    case SQLITE_TESTCTRL_BITVEC_TEST: {
      整数 sz = va_arg(ap, 整数);
      整数 *aProg = va_arg(ap, 整数*);
      rc = sqlite3BitvecBuiltinTest(sz, aProg);
      break;
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
    case SQLITE_TESTCTRL_FAULT_INSTALL: {
      /* A bug in MSVC prevents it from understanding pointers to functions
      ** types in the second argument to va_arg().  Work around the problem
      ** using a typedef.
      ** http://support.microsoft.com/kb/47961  <-- dead hyperlink
      ** Search at http://web.archive.org/ to find the 2015-03-16 archive
      ** of the link above to see the original text.
      ** sqlite3GlobalConfig.xTestCallback = va_arg(ap, int(*)(int));
      */
      typedef 整数(*sqlite3FaultFuncType)(整数);
      sqlite3GlobalConfig.xTestCallback = va_arg(ap, sqlite3FaultFuncType);
      rc = sqlite3FaultSim(0);
      break;
    }

    /*
    **  sqlite3_test_control(BENIGN_MALLOC_HOOKS, xBegin, xEnd)
    **
    ** Register hooks to call to indicate which malloc() failures
    ** are benign.
    */
    case SQLITE_TESTCTRL_BENIGN_MALLOC_HOOKS: {
      typedef void (*void_function)(void);
      void_function xBenignBegin;
      void_function xBenignEnd;
      xBenignBegin = va_arg(ap, void_function);
      xBenignEnd = va_arg(ap, void_function);
      sqlite3BenignMallocHooks(xBenignBegin, xBenignEnd);
      break;
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
    case SQLITE_TESTCTRL_PENDING_BYTE: {
      rc = PENDING_BYTE;
#ifndef SQLITE_OMIT_WSD
      {
        unsigned 整数 newVal = va_arg(ap, unsigned 整数);
        如果( newVal ) sqlite3PendingByte = newVal;
      }
#endif
      break;
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
    case SQLITE_TESTCTRL_ASSERT: {
      volatile 整数 x = 0;
      assert( /*side-effects-ok*/ (x = va_arg(ap,整数))!=0 );
      rc = x;
#如果 defined(SQLITE_DEBUG)
      /* Invoke these debugging routines so that the compiler does not
      ** issue "defined but not used" warnings. */
      如果( x==9999 ){
        sqlite3ShowExpr(0);
        sqlite3ShowExprList(0);
        sqlite3ShowIdList(0);
        sqlite3ShowSrcList(0);
        sqlite3ShowWith(0);
        sqlite3ShowUpsert(0);
#ifndef SQLITE_OMIT_TRIGGER
        sqlite3ShowTriggerStep(0);
        sqlite3ShowTriggerStepList(0);
        sqlite3ShowTrigger(0);
        sqlite3ShowTriggerList(0);
#endif
#ifndef SQLITE_OMIT_WINDOWFUNC
        sqlite3ShowWindow(0);
        sqlite3ShowWinFunc(0);
#endif
        sqlite3ShowSelect(0);
      }
#endif
      break;
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
    case SQLITE_TESTCTRL_ALWAYS: {
      整数 x = va_arg(ap,整数);
      rc = x ? ALWAYS(x) : 0;
      break;
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
    case SQLITE_TESTCTRL_BYTEORDER: {
      rc = SQLITE_BYTEORDER*100 + SQLITE_LITTLEENDIAN*10 + SQLITE_BIGENDIAN;
      break;
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
    case SQLITE_TESTCTRL_OPTIMIZATIONS: {
      sqlite3 *db = va_arg(ap, sqlite3*);
      db->dbOptFlags = va_arg(ap, u32);
      break;
    }

    /*  sqlite3_test_control(SQLITE_TESTCTRL_GETOPT, sqlite3 *db, int *N)
    **
    ** Write the current optimization settings into *N.  A zero bit means that
    ** the optimization is on, and a 1 bit means that the optimization is off.
    */
    case SQLITE_TESTCTRL_GETOPT: {
      sqlite3 *db = va_arg(ap, sqlite3*);
      整数 *pN = va_arg(ap, 整数*);
      *pN = db->dbOptFlags;
      break;
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
    case SQLITE_TESTCTRL_LOCALTIME_FAULT: {
      sqlite3GlobalConfig.bLocaltimeFault = va_arg(ap, 整数);
      如果( sqlite3GlobalConfig.bLocaltimeFault==2 ){
        typedef 整数(*sqlite3LocaltimeType)(const void*,void*);
        sqlite3GlobalConfig.xAltLocaltime = va_arg(ap, sqlite3LocaltimeType);
      }否则{
        sqlite3GlobalConfig.xAltLocaltime = 0;
      }
      break;
    }

    /*   sqlite3_test_control(SQLITE_TESTCTRL_INTERNAL_FUNCTIONS, sqlite3*);
    **
    ** Toggle the ability to use internal functions on or off for
    ** the database connection given in the argument.
    */
    case SQLITE_TESTCTRL_INTERNAL_FUNCTIONS: {
      sqlite3 *db = va_arg(ap, sqlite3*);
      db->mDbFlags ^= DBFLAG_InternalFunc;
      break;
    }

    /*   sqlite3_test_control(SQLITE_TESTCTRL_NEVER_CORRUPT, int);
    **
    ** Set or clear a flag that indicates that the database file is always well-
    ** formed and never corrupt.  This flag is clear by default, indicating that
    ** database files might have arbitrary corruption.  Setting the flag during
    ** testing causes certain assert() statements in the code to be activated
    ** that demonstrate invariants on well-formed database files.
    */
    case SQLITE_TESTCTRL_NEVER_CORRUPT: {
      sqlite3GlobalConfig.neverCorrupt = va_arg(ap, 整数);
      break;
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
    case SQLITE_TESTCTRL_EXTRA_SCHEMA_CHECKS: {
      sqlite3GlobalConfig.bExtraSchemaChecks = va_arg(ap, 整数);
      break;
    }

    /* Set the threshold at which OP_Once counters reset back to zero.
    ** By default this is 0x7ffffffe (over 2 billion), but that value is
    ** too big to test in a reasonable amount of time, so this control is
    ** provided to set a small and easily reachable reset value.
    */
    case SQLITE_TESTCTRL_ONCE_RESET_THRESHOLD: {
      sqlite3GlobalConfig.iOnceResetThreshold = va_arg(ap, 整数);
      break;
    }

    /*   sqlite3_test_control(SQLITE_TESTCTRL_VDBE_COVERAGE, xCallback, ptr);
    **
    ** Set the VDBE coverage callback function to xCallback with context
    ** pointer ptr.
    */
    case SQLITE_TESTCTRL_VDBE_COVERAGE: {
#ifdef SQLITE_VDBE_COVERAGE
      typedef void (*branch_callback)(void*,unsigned 整数,
                                      unsigned char,unsigned char);
      sqlite3GlobalConfig.xVdbeBranch = va_arg(ap,branch_callback);
      sqlite3GlobalConfig.pVdbeBranchArg = va_arg(ap,void*);
#endif
      break;
    }

    /*   sqlite3_test_control(SQLITE_TESTCTRL_SORTER_MMAP, db, nMax); */
    case SQLITE_TESTCTRL_SORTER_MMAP: {
      sqlite3 *db = va_arg(ap, sqlite3*);
      db->nMaxSorterMmap = va_arg(ap, 整数);
      break;
    }

    /*   sqlite3_test_control(SQLITE_TESTCTRL_ISINIT);
    **
    ** Return SQLITE_OK if SQLite has been initialized and SQLITE_ERROR if
    ** not.
    */
    case SQLITE_TESTCTRL_ISINIT: {
      如果( sqlite3GlobalConfig.isInit==0 ) rc = SQLITE_ERROR;
      break;
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
    case SQLITE_TESTCTRL_IMPOSTER: {
      sqlite3 *db = va_arg(ap, sqlite3*);
      整数 iDb;
      sqlite3_mutex_enter(db->mutex);
      iDb = sqlite3FindDbName(db, va_arg(ap,const char*));
      如果( iDb>=0 ){
        db->init.iDb = iDb;
        db->init.busy = db->init.imposterTable = va_arg(ap,整数);
        db->init.newTnum = va_arg(ap,整数);
        如果( db->init.busy==0 && db->init.newTnum>0 ){
          sqlite3ResetAllSchemasOfConnection(db);
        }
      }
      sqlite3_mutex_leave(db->mutex);
      break;
    }

#如果 defined(YYCOVERAGE)
    /*  sqlite3_test_control(SQLITE_TESTCTRL_PARSER_COVERAGE, FILE *out)
    **
    ** This test control (only available when SQLite is compiled with
    ** -DYYCOVERAGE) writes a report onto "out" that shows all
    ** state/lookahead combinations in the parser state machine
    ** which are never exercised.  If any state is missed, make the
    ** return code SQLITE_ERROR.
    */
    case SQLITE_TESTCTRL_PARSER_COVERAGE: {
      FILE *out = va_arg(ap, FILE*);
      如果( sqlite3ParserCoverage(out) ) rc = SQLITE_ERROR;
      break;
    }
#endif /* defined(YYCOVERAGE) */

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
    case SQLITE_TESTCTRL_RESULT_INTREAL: {
      sqlite3_context *pCtx = va_arg(ap, sqlite3_context*);
      sqlite3ResultIntReal(pCtx);
      break;
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
    case SQLITE_TESTCTRL_SEEK_COUNT: {
      sqlite3 *db = va_arg(ap, sqlite3*);
      u64 *pn = va_arg(ap, sqlite3_uint64*);
      *pn = sqlite3BtreeSeekCount(db->aDb->pBt);
      (void)db;  /* Silence harmless unused variable warning */
      break;
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
    case SQLITE_TESTCTRL_TRACEFLAGS: {
       整数 opTrace = va_arg(ap, 整数);
       u32 *ptr = va_arg(ap, u32*);
       switch( opTrace ){
         case 0:   *ptr = sqlite3TreeTrace;      break;
         case 1:   sqlite3TreeTrace = *ptr;      break;
         case 2:   *ptr = sqlite3WhereTrace;     break;
         case 3:   sqlite3WhereTrace = *ptr;     break;
       }
       break;
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
    case SQLITE_TESTCTRL_LOGEST: {
      double rIn = va_arg(ap, double);
      LogEst rLogEst = sqlite3LogEstFromDouble(rIn);
      整数 *pI1 = va_arg(ap,整数*);
      u64 *pU64 = va_arg(ap,u64*);
      整数 *pI2 = va_arg(ap,整数*);
      *pI1 = rLogEst;
      *pU64 = sqlite3LogEstToInt(rLogEst);
      *pI2 = sqlite3LogEst(*pU64);
      break;
    }

    /* sqlite3_test_control(SQLITE_TESTCTRL_ATOF, const char *z, double *p);
    **
    ** Test access to the sqlite3AtoF() routine.
    */
    case SQLITE_TESTCTRL_ATOF: {
      const char *z = va_arg(ap,const char*);
      double *pR = va_arg(ap,double*);
      rc = sqlite3AtoF(z,pR);
      break;
    }

#如果 defined(SQLITE_DEBUG) && !defined(SQLITE_OMIT_WSD)
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
    case SQLITE_TESTCTRL_TUNE: {
      整数 id = va_arg(ap, 整数);
      整数 *piValue = va_arg(ap, 整数*);
      如果( id>0 && id<=SQLITE_NTUNE ){
        Tuning(id) = *piValue;
      }否则 如果( id<0 && id>=-SQLITE_NTUNE ){
        *piValue = Tuning(-id);
      }否则{
        rc = SQLITE_NOTFOUND;
      }
      break;
    }
#endif

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
    case SQLITE_TESTCTRL_JSON_SELFCHECK: {
#如果 defined(SQLITE_DEBUG) && !defined(SQLITE_OMIT_WSD)
      整数 *pOnOff = va_arg(ap, 整数*);
      如果( *pOnOff<0 ){
        *pOnOff = sqlite3Config.bJsonSelfcheck;
      }否则{
        sqlite3Config.bJsonSelfcheck = (u8)((*pOnOff)&0xff);
      }
#endif
      break;
    }
  }
  va_end(ap);
#endif /* SQLITE_UNTESTABLE */
  返回 rc;
}

/*
** The Pager stores the Database filename, Journal filename, and WAL filename
** consecutively in memory, in that order.  The database filename is prefixed
** by four zero bytes.  Locate the start of the database filename by searching
** backwards for the first byte following four consecutive zero bytes.
**
** This only works if the filename passed in was obtained from the Pager.
*/
static const char *databaseName(const char *zName){
  当( zName[-1]!=0 || zName[-2]!=0 || zName[-3]!=0 || zName[-4]!=0 ){
    zName--;
  }
  返回 zName;
}

/*
** Append text z[] to the end of p[].  Return a pointer to the first
** character after then zero terminator on the new text in p[].
*/
static char *appendText(char *p, const char *z){
  size_t n = strlen(z);
  memcpy(p, z, n+1);
  返回 p+n+1;
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
const char *sqlite3_create_filename(
  const char *zDatabase,
  const char *zJournal,
  const char *zWal,
  整数 nParam,
  const char **azParam
){
  sqlite3_int64 nByte;
  整数 i;
  char *pResult, *p;
  nByte = strlen(zDatabase) + strlen(zJournal) + strlen(zWal) + 10;
  为(i=0; i<nParam*2; i++){
    nByte += strlen(azParam[i])+1;
  }
  pResult = p = sqlite3_malloc64( nByte );
  如果( p==0 ) 返回 0;
  memset(p, 0, 4);
  p += 4;
  p = appendText(p, zDatabase);
  为(i=0; i<nParam*2; i++){
    p = appendText(p, azParam[i]);
  }
  *(p++) = 0;
  p = appendText(p, zJournal);
  p = appendText(p, zWal);
  *(p++) = 0;
  *(p++) = 0;
  assert( (sqlite3_int64)(p - pResult)==nByte );
  返回 pResult + 4;
}

/*
** Free memory obtained from sqlite3_create_filename().  It is a severe
** error to call this routine with any parameter other than a pointer
** previously obtained from sqlite3_create_filename() or a NULL pointer.
*/
void sqlite3_free_filename(const char *p){
  如果( p==0 ) 返回;
  p = databaseName(p);
  sqlite3_free((char*)p - 4);
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
const char *sqlite3_uri_parameter(const char *zFilename, const char *zParam){
  如果( zFilename==0 || zParam==0 ) 返回 0;
  zFilename = databaseName(zFilename);
  返回 uriParameter(zFilename, zParam);
}

/*
** Return a pointer to the name of Nth query parameter of the filename.
*/
const char *sqlite3_uri_key(const char *zFilename, 整数 N){
  如果( zFilename==0 || N<0 ) 返回 0;
  zFilename = databaseName(zFilename);
  zFilename += sqlite3Strlen30(zFilename) + 1;
  当( ALWAYS(zFilename) && zFilename[0] && (N--)>0 ){
    zFilename += sqlite3Strlen30(zFilename) + 1;
    zFilename += sqlite3Strlen30(zFilename) + 1;
  }
  返回 zFilename[0] ? zFilename : 0;
}

/*
** Return a boolean value for a query parameter.
*/
整数 sqlite3_uri_boolean(const char *zFilename, const char *zParam, 整数 bDflt){
  const char *z = sqlite3_uri_parameter(zFilename, zParam);
  bDflt = bDflt!=0;
  返回 z ? sqlite3GetBoolean(z, bDflt) : bDflt;
}

/*
** Return a 64-bit integer value for a query parameter.
*/
sqlite3_int64 sqlite3_uri_int64(
  const char *zFilename,    /* Filename as passed to xOpen */
  const char *zParam,       /* URI parameter sought */
  sqlite3_int64 bDflt       /* return if parameter is missing */
){
  const char *z = sqlite3_uri_parameter(zFilename, zParam);
  sqlite3_int64 v;
  如果( z && sqlite3DecOrHexToI64(z, &v)==0 ){
    bDflt = v;
  }
  返回 bDflt;
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
const char *sqlite3_filename_database(const char *zFilename){
  如果( zFilename==0 ) 返回 0;
  返回 databaseName(zFilename);
}
const char *sqlite3_filename_journal(const char *zFilename){
  如果( zFilename==0 ) 返回 0;
  zFilename = databaseName(zFilename);
  zFilename += sqlite3Strlen30(zFilename) + 1;
  当( ALWAYS(zFilename) && zFilename[0] ){
    zFilename += sqlite3Strlen30(zFilename) + 1;
    zFilename += sqlite3Strlen30(zFilename) + 1;
  }
  返回 zFilename + 1;
}
const char *sqlite3_filename_wal(const char *zFilename){
#ifdef SQLITE_OMIT_WAL
  UNUSED_PARAMETER(zFilename);
  返回 0;
#否则
  zFilename = sqlite3_filename_journal(zFilename);
  如果( zFilename ) zFilename += sqlite3Strlen30(zFilename) + 1;
  返回 zFilename;
#endif
}

/*
** Return the Btree pointer identified by zDbName.  Return NULL if not found.
*/
Btree *sqlite3DbNameToBtree(sqlite3 *db, const char *zDbName){
  整数 iDb = zDbName ? sqlite3FindDbName(db, zDbName) : 0;
  返回 iDb<0 ? 0 : db->aDb[iDb].pBt;
}

/*
** Return the name of the N-th database schema.  Return NULL if N is out
** of range.
*/
const char *sqlite3_db_name(sqlite3 *db, 整数 N){
  const char *zRet = 0;
#ifdef SQLITE_ENABLE_API_ARMOR
  如果( !sqlite3SafetyCheckOk(db) ){
    (void)SQLITE_MISUSE_BKPT;
    返回 0;
  }
#endif
  sqlite3_mutex_enter(db->mutex);
  如果( N>=0 && N<db->nDb ){
    zRet = db->aDb[N].zDbSName;
  }
  sqlite3_mutex_leave(db->mutex);
  返回 zRet;
}

/*
** Return the filename of the database associated with a database
** connection.
*/
const char *sqlite3_db_filename(sqlite3 *db, const char *zDbName){
  Btree *pBt;
#ifdef SQLITE_ENABLE_API_ARMOR
  如果( !sqlite3SafetyCheckOk(db) ){
    (void)SQLITE_MISUSE_BKPT;
    返回 0;
  }
#endif
  pBt = sqlite3DbNameToBtree(db, zDbName);
  返回 pBt ? sqlite3BtreeGetFilename(pBt) : 0;
}

/*
** Return 1 if database is read-only or 0 if read/write.  Return -1 if
** no such database exists.
*/
整数 sqlite3_db_readonly(sqlite3 *db, const char *zDbName){
  Btree *pBt;
#ifdef SQLITE_ENABLE_API_ARMOR
  如果( !sqlite3SafetyCheckOk(db) ){
    (void)SQLITE_MISUSE_BKPT;
    返回 -1;
  }
#endif
  pBt = sqlite3DbNameToBtree(db, zDbName);
  返回 pBt ? sqlite3BtreeIsReadonly(pBt) : -1;
}

#ifdef SQLITE_ENABLE_SNAPSHOT
/*
** Obtain a snapshot handle for the snapshot of database zDb currently
** being read by handle db.
*/
整数 sqlite3_snapshot_get(
  sqlite3 *db,
  const char *zDb,
  sqlite3_snapshot **ppSnapshot
){
  整数 rc = SQLITE_ERROR;
#ifndef SQLITE_OMIT_WAL

#ifdef SQLITE_ENABLE_API_ARMOR
  如果( !sqlite3SafetyCheckOk(db) ){
    返回 SQLITE_MISUSE_BKPT;
  }
#endif
  sqlite3_mutex_enter(db->mutex);

  如果( db->autoCommit==0 ){
    整数 iDb = sqlite3FindDbName(db, zDb);
    如果( iDb==0 || iDb>1 ){
      Btree *pBt = db->aDb[iDb].pBt;
      如果( SQLITE_TXN_WRITE!=sqlite3BtreeTxnState(pBt) ){
        Pager *pPager = sqlite3BtreePager(pBt);
        i64 dummy = 0;
        sqlite3PagerSnapshotOpen(pPager, (sqlite3_snapshot*)&dummy);
        rc = sqlite3BtreeBeginTrans(pBt, 0, 0);
        sqlite3PagerSnapshotOpen(pPager, 0);
        如果( rc==SQLITE_OK ){
          rc = sqlite3PagerSnapshotGet(sqlite3BtreePager(pBt), ppSnapshot);
        }
      }
    }
  }

  sqlite3_mutex_leave(db->mutex);
#endif   /* SQLITE_OMIT_WAL */
  返回 rc;
}

/*
** Open a read-transaction on the snapshot identified by pSnapshot.
*/
整数 sqlite3_snapshot_open(
  sqlite3 *db,
  const char *zDb,
  sqlite3_snapshot *pSnapshot
){
  整数 rc = SQLITE_ERROR;
#ifndef SQLITE_OMIT_WAL

#ifdef SQLITE_ENABLE_API_ARMOR
  如果( !sqlite3SafetyCheckOk(db) ){
    返回 SQLITE_MISUSE_BKPT;
  }
#endif
  sqlite3_mutex_enter(db->mutex);
  如果( db->autoCommit==0 ){
    整数 iDb;
    iDb = sqlite3FindDbName(db, zDb);
    如果( iDb==0 || iDb>1 ){
      Btree *pBt = db->aDb[iDb].pBt;
      如果( sqlite3BtreeTxnState(pBt)!=SQLITE_TXN_WRITE ){
        Pager *pPager = sqlite3BtreePager(pBt);
        整数 bUnlock = 0;
        如果( sqlite3BtreeTxnState(pBt)!=SQLITE_TXN_NONE ){
          如果( db->nVdbeActive==0 ){
            rc = sqlite3PagerSnapshotCheck(pPager, pSnapshot);
            如果( rc==SQLITE_OK ){
              bUnlock = 1;
              rc = sqlite3BtreeCommit(pBt);
            }
          }
        }否则{
          rc = SQLITE_OK;
        }
        如果( rc==SQLITE_OK ){
          rc = sqlite3PagerSnapshotOpen(pPager, pSnapshot);
        }
        如果( rc==SQLITE_OK ){
          rc = sqlite3BtreeBeginTrans(pBt, 0, 0);
          sqlite3PagerSnapshotOpen(pPager, 0);
        }
        如果( bUnlock ){
          sqlite3PagerSnapshotUnlock(pPager);
        }
      }
    }
  }

  sqlite3_mutex_leave(db->mutex);
#endif   /* SQLITE_OMIT_WAL */
  返回 rc;
}

/*
** Recover as many snapshots as possible from the wal file associated with
** schema zDb of database db.
*/
整数 sqlite3_snapshot_recover(sqlite3 *db, const char *zDb){
  整数 rc = SQLITE_ERROR;
#ifndef SQLITE_OMIT_WAL
  整数 iDb;

#ifdef SQLITE_ENABLE_API_ARMOR
  如果( !sqlite3SafetyCheckOk(db) ){
    返回 SQLITE_MISUSE_BKPT;
  }
#endif

  sqlite3_mutex_enter(db->mutex);
  iDb = sqlite3FindDbName(db, zDb);
  如果( iDb==0 || iDb>1 ){
    Btree *pBt = db->aDb[iDb].pBt;
    如果( SQLITE_TXN_NONE==sqlite3BtreeTxnState(pBt) ){
      rc = sqlite3BtreeBeginTrans(pBt, 0, 0);
      如果( rc==SQLITE_OK ){
        rc = sqlite3PagerSnapshotRecover(sqlite3BtreePager(pBt));
        sqlite3BtreeCommit(pBt);
      }
    }
  }
  sqlite3_mutex_leave(db->mutex);
#endif   /* SQLITE_OMIT_WAL */
  返回 rc;
}

/*
** Free a snapshot handle obtained from sqlite3_snapshot_get().
*/
void sqlite3_snapshot_free(sqlite3_snapshot *pSnapshot){
  sqlite3_free(pSnapshot);
}
#endif /* SQLITE_ENABLE_SNAPSHOT */

#ifndef SQLITE_OMIT_COMPILEOPTION_DIAGS
/*
** Given the name of a compile-time option, return true if that option
** was used and false if not.
**
** The name can optionally begin with "SQLITE_" but the "SQLITE_" prefix
** is not required for a match.
*/
整数 sqlite3_compileoption_used(const char *zOptName){
  整数 i, n;
  整数 nOpt;
  const char **azCompileOpt;

#ifdef SQLITE_ENABLE_API_ARMOR
  如果( zOptName==0 ){
    (void)SQLITE_MISUSE_BKPT;
    返回 0;
  }
#endif

  azCompileOpt = sqlite3CompileOptions(&nOpt);

  如果( sqlite3StrNICmp(zOptName, "SQLITE_", 7)==0 ) zOptName += 7;
  n = sqlite3Strlen30(zOptName);

  /* Since nOpt is normally in single digits, a linear search is
  ** adequate. No need for a binary search. */
  为(i=0; i<nOpt; i++){
    如果( sqlite3StrNICmp(zOptName, azCompileOpt[i], n)==0
     && sqlite3IsIdChar((unsigned char)azCompileOpt[i][n])==0
    ){
      返回 1;
    }
  }
  返回 0;
}

/*
** Return the N-th compile-time option string.  If N is out of range,
** return a NULL pointer.
*/
const char *sqlite3_compileoption_get(整数 N){
  整数 nOpt;
  const char **azCompileOpt;
  azCompileOpt = sqlite3CompileOptions(&nOpt);
  如果( N>=0 && N<nOpt ){
    返回 azCompileOpt[N];
  }
  返回 0;
}
#endif /* SQLITE_OMIT_COMPILEOPTION_DIAGS */
