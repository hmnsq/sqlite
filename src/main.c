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
#အကယ်၍ defined(SQLITE_ENABLE_ICU) || defined(SQLITE_ENABLE_ICU_COLLATIONS)
# include "sqliteicu.h"
#endif

/*
** This is an extension initializer that is a no-op and always
** succeeds, except that it fails if the fault-simulation is set
** to 500.
*/
static ကိန်းပြည့် sqlite3TestExtInit(sqlite3 *db){
  (void)db;
  ပြန်ပေး sqlite3FaultSim(500);
}


/*
** Forward declarations of external module initializer functions
** for modules that need them.
*/
#ifdef SQLITE_ENABLE_FTS5
ကိန်းပြည့် sqlite3Fts5Init(sqlite3*);
#endif
#ifdef SQLITE_ENABLE_STMTVTAB
ကိန်းပြည့် sqlite3StmtVtabInit(sqlite3*);
#endif
#ifdef SQLITE_EXTRA_AUTOEXT
ကိန်းပြည့် SQLITE_EXTRA_AUTOEXT(sqlite3*);
#endif
/*
** An array of pointers to extension initializer functions for
** built-in extensions.
*/
static ကိန်းပြည့် (*const sqlite3BuiltinExtensions[])(sqlite3*) = {
#ifdef SQLITE_ENABLE_FTS3
  sqlite3Fts3Init,
#endif
#ifdef SQLITE_ENABLE_FTS5
  sqlite3Fts5Init,
#endif
#အကယ်၍ defined(SQLITE_ENABLE_ICU) || defined(SQLITE_ENABLE_ICU_COLLATIONS)
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
const var_na sqlite3_version[] = SQLITE_VERSION;
#endif

/* IMPLEMENTATION-OF: R-53536-42575 The sqlite3_libversion() function returns
** a pointer to the to the sqlite3_version[] string constant.
*/
const var_na *sqlite3_libversion(void){ ပြန်ပေး sqlite3_version; }

/* IMPLEMENTATION-OF: R-25063-23286 The sqlite3_sourceid() function returns a
** pointer to a string constant whose value is the same as the
** SQLITE_SOURCE_ID C preprocessor macro. Except if SQLite is built using
** an edited copy of the amalgamation, then the last four characters of
** the hash might be different from SQLITE_SOURCE_ID.
*/
const var_na *sqlite3_sourceid(void){ ပြန်ပေး SQLITE_SOURCE_ID; }

/* IMPLEMENTATION-OF: R-35210-63508 The sqlite3_libversion_number() function
** returns an integer equal to SQLITE_VERSION_NUMBER.
*/
ကိန်းပြည့် sqlite3_libversion_number(void){ ပြန်ပေး SQLITE_VERSION_NUMBER; }

/* IMPLEMENTATION-OF: R-20790-14025 The sqlite3_threadsafe() function returns
** zero if and only if SQLite was compiled with mutexing code omitted due to
** the SQLITE_THREADSAFE compile-time option being set to 0.
*/
ကိန်းပြည့် sqlite3_threadsafe(void){ ပြန်ပေး SQLITE_THREADSAFE; }

/*
** When compiling the test fixture or with debugging enabled (on Win32),
** this variable being set to non-zero will cause OSTRACE macros to emit
** extra diagnostic information.
*/
#ifdef SQLITE_HAVE_OS_TRACE
# ifndef SQLITE_DEBUG_OS_TRACE
#   define SQLITE_DEBUG_OS_TRACE 0
# endif
  ကိန်းပြည့် sqlite3OSTrace = SQLITE_DEBUG_OS_TRACE;
#endif

#အကယ်၍ !defined(SQLITE_OMIT_TRACE) && defined(SQLITE_ENABLE_IOTRACE)
/*
** If the following function pointer is not NULL and if
** SQLITE_ENABLE_IOTRACE is enabled, then messages describing
** I/O active are written using this function.  These messages
** are intended for debugging activity only.
*/
SQLITE_API void (SQLITE_CDECL *sqlite3IoTrace)(const var_na*, ...) = 0;
#endif

/*
** If the following global variable points to a string which is the
** name of a directory, then that directory will be used to store
** temporary files.
**
** See also the "PRAGMA temp_store_directory" SQL command.
*/
var_na *sqlite3_temp_directory = 0;

/*
** If the following global variable points to a string which is the
** name of a directory, then that directory will be used to store
** all database files specified with a relative pathname.
**
** See also the "PRAGMA data_store_directory" SQL command.
*/
var_na *sqlite3_data_directory = 0;

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
static SQLITE_NOINLINE ကိန်းပြည့် sqlite3Initialize(void){
  MUTEX_LOGIC( sqlite3_mutex *pMainMtx; )   /* The main static mutex */
  ကိန်းပြည့် rc;                                   /* Result code */
#ifdef SQLITE_EXTRA_INIT
  ကိန်းပြည့် bRunExtraInit = 0;                    /* Extra initialization needed */
#endif

  /* If the following assert() fails on some obscure processor/compiler
  ** combination to warn that SQLite has been mis-compiled.  If you hit
  ** this assert(), that means you need to recompile with the
  ** -DSQLITE_PTRSIZE=n compile-time option to set the correct pointer size.
  */
  assert( SQLITE_PTRSIZE==parimaa_na(var_na*) );

  /* Make sure the mutex subsystem is initialized.  If unable to
  ** initialize the mutex subsystem, return early with the error.
  ** If the system is so sick that we are unable to allocate a mutex,
  ** there is not much SQLite is going to be able to do.
  **
  ** The mutex subsystem must take care of serializing its own
  ** initialization.
  */
  rc = sqlite3MutexInit();
  အကယ်၍( rc ) ပြန်ပေး rc;

  /* Initialize the malloc() system and the recursive pInitMutex mutex.
  ** This operation is protected by the STATIC_MAIN mutex.  Note that
  ** MutexAlloc() is called for a static mutex prior to initializing the
  ** malloc subsystem - this implies that the allocation of a static
  ** mutex must not require support from the malloc subsystem.
  */
  MUTEX_LOGIC( pMainMtx = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  sqlite3_mutex_enter(pMainMtx);
  sqlite3GlobalConfig.isMutexInit = 1;
  အကယ်၍( !sqlite3GlobalConfig.isMallocInit ){
    rc = sqlite3MallocInit();
  }
  အကယ်၍( rc==SQLITE_OK ){
    sqlite3GlobalConfig.isMallocInit = 1;
    အကယ်၍( !sqlite3GlobalConfig.pInitMutex ){
      sqlite3GlobalConfig.pInitMutex =
           sqlite3MutexAlloc(SQLITE_MUTEX_RECURSIVE);
      အကယ်၍( sqlite3GlobalConfig.bCoreMutex && !sqlite3GlobalConfig.pInitMutex ){
        rc = SQLITE_NOMEM_BKPT;
      }
    }
  }
  အကယ်၍( rc==SQLITE_OK ){
    sqlite3GlobalConfig.nRefInitMutex++;
  }
  sqlite3_mutex_leave(pMainMtx);

  /* If rc is not SQLITE_OK at this point, then either the malloc
  ** subsystem could not be initialized or the system failed to allocate
  ** the pInitMutex mutex. Return an error in either case.  */
  အကယ်၍( rc!=SQLITE_OK ){
    ပြန်ပေး rc;
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
  အကယ်၍( sqlite3GlobalConfig.isInit==0 && sqlite3GlobalConfig.inProgress==0 ){
    sqlite3GlobalConfig.inProgress = 1;
#ifdef SQLITE_ENABLE_SQLLOG
    {
      extern void sqlite3_init_sqllog(void);
      sqlite3_init_sqllog();
    }
#endif
    memset(&sqlite3BuiltinFunctions, 0, parimaa_na(sqlite3BuiltinFunctions));
    sqlite3RegisterBuiltinFunctions();
    အကယ်၍( sqlite3GlobalConfig.isPCacheInit==0 ){
      rc = sqlite3PcacheInitialize();
    }
    အကယ်၍( rc==SQLITE_OK ){
      sqlite3GlobalConfig.isPCacheInit = 1;
      rc = sqlite3OsInit();
    }
#ifndef SQLITE_OMIT_DESERIALIZE
    အကယ်၍( rc==SQLITE_OK ){
      rc = sqlite3MemdbInit();
    }
#endif
    အကယ်၍( rc==SQLITE_OK ){
      sqlite3PCacheBufferSetup( sqlite3GlobalConfig.pPage,
          sqlite3GlobalConfig.szPage, sqlite3GlobalConfig.nPage);
#ifdef SQLITE_EXTRA_INIT_MUTEXED
      {
        ကိန်းပြည့် SQLITE_EXTRA_INIT_MUTEXED(const var_na*);
        rc = SQLITE_EXTRA_INIT_MUTEXED(0);
      }
#endif
    }
    အကယ်၍( rc==SQLITE_OK ){
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
  အကယ်၍( sqlite3GlobalConfig.nRefInitMutex<=0 ){
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
  အကယ်၍( rc==SQLITE_OK ){
    u64 x = (((u64)1)<<63)-1;
    dviguu_na y;
    assert(parimaa_na(x)==8);
    assert(parimaa_na(x)==parimaa_na(y));
    memcpy(&y, &x, 8);
    assert( sqlite3IsNaN(y) );
  }
#endif
#endif

  /* Do extra initialization steps requested by the SQLITE_EXTRA_INIT
  ** compile-time option.
  */
#ifdef SQLITE_EXTRA_INIT
  အကယ်၍( bRunExtraInit ){
    ကိန်းပြည့် SQLITE_EXTRA_INIT(const var_na*);
    rc = SQLITE_EXTRA_INIT(0);
  }
#endif
  ပြန်ပေး rc;
}
SQLITE_API ကိန်းပြည့် sqlite3_initialize(void){
  /* If this build does not support writable static data (WSD) natively
  ** then we have to invoke the (application-supplied) WSD initialization
  ** routine before doing anything else. */
#ifdef SQLITE_OMIT_WSD
  ကိန်းပြည့် rc = sqlite3_wsd_init(4096, 24);
  အကယ်၍( rc!=SQLITE_OK ){
    ပြန်ပေး rc;
  }
#endif

  အကယ်၍( sqlite3GlobalConfig.isInit ){
    /* SQLite has already been initialized.  Fast early-out. */
    sqlite3MemoryBarrier();
    ပြန်ပေး SQLITE_OK;
  }မဟုတ်ပါက{
    /* Invoke sqlite3Initialize() to do the actual work. */
    ပြန်ပေး sqlite3Initialize();
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
ကိန်းပြည့် sqlite3_shutdown(void){
#ifdef SQLITE_OMIT_WSD
  ကိန်းပြည့် rc = sqlite3_wsd_init(4096, 24);
  အကယ်၍( rc!=SQLITE_OK ){
    ပြန်ပေး rc;
  }
#endif

  အကယ်၍( sqlite3GlobalConfig.isInit ){
#ifdef SQLITE_EXTRA_SHUTDOWN
    void SQLITE_EXTRA_SHUTDOWN(void);
    SQLITE_EXTRA_SHUTDOWN();
#endif
    sqlite3_os_end();
    sqlite3_reset_auto_extension();
    sqlite3GlobalConfig.isInit = 0;
  }
  အကယ်၍( sqlite3GlobalConfig.isPCacheInit ){
    sqlite3PcacheShutdown();
    sqlite3GlobalConfig.isPCacheInit = 0;
  }
  အကယ်၍( sqlite3GlobalConfig.isMallocInit ){
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
  အကယ်၍( sqlite3GlobalConfig.isMutexInit ){
    sqlite3MutexEnd();
    sqlite3GlobalConfig.isMutexInit = 0;
  }

  ပြန်ပေး SQLITE_OK;
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
ကိန်းပြည့် sqlite3_config(ကိန်းပြည့် op, ...){
  va_list ap;
  ကိန်းပြည့် rc = SQLITE_OK;

  /* sqlite3_config() normally returns SQLITE_MISUSE if it is invoked while
  ** the SQLite library is in use.  Except, a few selected opcodes
  ** are allowed.
  */
  အကယ်၍( sqlite3GlobalConfig.isInit ){
    static const u64 mAnytimeConfigOption = 0
       | MASKBIT64( SQLITE_CONFIG_LOG )
       | MASKBIT64( SQLITE_CONFIG_PCACHE_HDRSZ )
    ;
    အကယ်၍( op<0 || op>63 || (MASKBIT64(op) & mAnytimeConfigOption)==0 ){
      ပြန်ပေး SQLITE_MISUSE_BKPT;
    }
    testcase( op==SQLITE_CONFIG_LOG );
    testcase( op==SQLITE_CONFIG_PCACHE_HDRSZ );
  }

  va_start(ap, op);
  switch( op ){

    /* Mutex configuration options are only available in a threadsafe
    ** compile.
    */
#အကယ်၍ defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE>0  /* IMP: R-54466-46756 */
    pak_sa SQLITE_CONFIG_SINGLETHREAD: {
      /* EVIDENCE-OF: R-02748-19096 This option sets the threading mode to
      ** Single-thread. */
      sqlite3GlobalConfig.bCoreMutex = 0;  /* Disable mutex on core */
      sqlite3GlobalConfig.bFullMutex = 0;  /* Disable mutex on connections */
      bha_nga;
    }
#endif
#အကယ်၍ defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE>0 /* IMP: R-20520-54086 */
    pak_sa SQLITE_CONFIG_MULTITHREAD: {
      /* EVIDENCE-OF: R-14374-42468 This option sets the threading mode to
      ** Multi-thread. */
      sqlite3GlobalConfig.bCoreMutex = 1;  /* Enable mutex on core */
      sqlite3GlobalConfig.bFullMutex = 0;  /* Disable mutex on connections */
      bha_nga;
    }
#endif
#အကယ်၍ defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE>0 /* IMP: R-59593-21810 */
    pak_sa SQLITE_CONFIG_SERIALIZED: {
      /* EVIDENCE-OF: R-41220-51800 This option sets the threading mode to
      ** Serialized. */
      sqlite3GlobalConfig.bCoreMutex = 1;  /* Enable mutex on core */
      sqlite3GlobalConfig.bFullMutex = 1;  /* Enable mutex on connections */
      bha_nga;
    }
#endif
#အကယ်၍ defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE>0 /* IMP: R-63666-48755 */
    pak_sa SQLITE_CONFIG_MUTEX: {
      /* Specify an alternative mutex implementation */
      sqlite3GlobalConfig.mutex = *va_arg(ap, sqlite3_mutex_methods*);
      bha_nga;
    }
#endif
#အကယ်၍ defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE>0 /* IMP: R-14450-37597 */
    pak_sa SQLITE_CONFIG_GETMUTEX: {
      /* Retrieve the current mutex implementation */
      *va_arg(ap, sqlite3_mutex_methods*) = sqlite3GlobalConfig.mutex;
      bha_nga;
    }
#endif

    pak_sa SQLITE_CONFIG_MALLOC: {
      /* EVIDENCE-OF: R-55594-21030 The SQLITE_CONFIG_MALLOC option takes a
      ** single argument which is a pointer to an instance of the
      ** sqlite3_mem_methods structure. The argument specifies alternative
      ** low-level memory allocation routines to be used in place of the memory
      ** allocation routines built into SQLite. */
      sqlite3GlobalConfig.m = *va_arg(ap, sqlite3_mem_methods*);
      bha_nga;
    }
    pak_sa SQLITE_CONFIG_GETMALLOC: {
      /* EVIDENCE-OF: R-51213-46414 The SQLITE_CONFIG_GETMALLOC option takes a
      ** single argument which is a pointer to an instance of the
      ** sqlite3_mem_methods structure. The sqlite3_mem_methods structure is
      ** filled with the currently defined memory allocation routines. */
      အကယ်၍( sqlite3GlobalConfig.m.xMalloc==0 ) sqlite3MemSetDefault();
      *va_arg(ap, sqlite3_mem_methods*) = sqlite3GlobalConfig.m;
      bha_nga;
    }
    pak_sa SQLITE_CONFIG_MEMSTATUS: {
      assert( !sqlite3GlobalConfig.isInit );  /* Cannot change at runtime */
      /* EVIDENCE-OF: R-61275-35157 The SQLITE_CONFIG_MEMSTATUS option takes
      ** single argument of type int, interpreted as a boolean, which enables
      ** or disables the collection of memory allocation statistics. */
      sqlite3GlobalConfig.bMemstat = va_arg(ap, ကိန်းပြည့်);
      bha_nga;
    }
    pak_sa SQLITE_CONFIG_SMALL_MALLOC: {
      sqlite3GlobalConfig.bSmallMalloc = va_arg(ap, ကိန်းပြည့်)!=0;
      bha_nga;
    }
    pak_sa SQLITE_CONFIG_PAGECACHE: {
      /* EVIDENCE-OF: R-18761-36601 There are three arguments to
      ** SQLITE_CONFIG_PAGECACHE: A pointer to 8-byte aligned memory (pMem),
      ** the size of each page cache line (sz), and the number of cache lines
      ** (N). */
      sqlite3GlobalConfig.pPage = va_arg(ap, void*);
      sqlite3GlobalConfig.szPage = va_arg(ap, ကိန်းပြည့်);
      sqlite3GlobalConfig.nPage = va_arg(ap, ကိန်းပြည့်);
      bha_nga;
    }
    pak_sa SQLITE_CONFIG_PCACHE_HDRSZ: {
      /* EVIDENCE-OF: R-39100-27317 The SQLITE_CONFIG_PCACHE_HDRSZ option takes
      ** a single parameter which is a pointer to an integer and writes into
      ** that integer the number of extra bytes per page required for each page
      ** in SQLITE_CONFIG_PAGECACHE. */
      *va_arg(ap, ကိန်းပြည့်*) =
          sqlite3HeaderSizeBtree() +
          sqlite3HeaderSizePcache() +
          sqlite3HeaderSizePcache1();
      bha_nga;
    }

    pak_sa SQLITE_CONFIG_PCACHE: {
      /* no-op */
      bha_nga;
    }
    pak_sa SQLITE_CONFIG_GETPCACHE: {
      /* now an error */
      rc = SQLITE_ERROR;
      bha_nga;
    }

    pak_sa SQLITE_CONFIG_PCACHE2: {
      /* EVIDENCE-OF: R-63325-48378 The SQLITE_CONFIG_PCACHE2 option takes a
      ** single argument which is a pointer to an sqlite3_pcache_methods2
      ** object. This object specifies the interface to a custom page cache
      ** implementation. */
      sqlite3GlobalConfig.pcache2 = *va_arg(ap, sqlite3_pcache_methods2*);
      bha_nga;
    }
    pak_sa SQLITE_CONFIG_GETPCACHE2: {
      /* EVIDENCE-OF: R-22035-46182 The SQLITE_CONFIG_GETPCACHE2 option takes a
      ** single argument which is a pointer to an sqlite3_pcache_methods2
      ** object. SQLite copies of the current page cache implementation into
      ** that object. */
      အကယ်၍( sqlite3GlobalConfig.pcache2.xInit==0 ){
        sqlite3PCacheSetDefault();
      }
      *va_arg(ap, sqlite3_pcache_methods2*) = sqlite3GlobalConfig.pcache2;
      bha_nga;
    }

/* EVIDENCE-OF: R-06626-12911 The SQLITE_CONFIG_HEAP option is only
** available if SQLite is compiled with either SQLITE_ENABLE_MEMSYS3 or
** SQLITE_ENABLE_MEMSYS5 and returns SQLITE_ERROR if invoked otherwise. */
#အကယ်၍ defined(SQLITE_ENABLE_MEMSYS3) || defined(SQLITE_ENABLE_MEMSYS5)
    pak_sa SQLITE_CONFIG_HEAP: {
      /* EVIDENCE-OF: R-19854-42126 There are three arguments to
      ** SQLITE_CONFIG_HEAP: An 8-byte aligned pointer to the memory, the
      ** number of bytes in the memory buffer, and the minimum allocation size.
      */
      sqlite3GlobalConfig.pHeap = va_arg(ap, void*);
      sqlite3GlobalConfig.nHeap = va_arg(ap, ကိန်းပြည့်);
      sqlite3GlobalConfig.mnReq = va_arg(ap, ကိန်းပြည့်);

      အကယ်၍( sqlite3GlobalConfig.mnReq<1 ){
        sqlite3GlobalConfig.mnReq = 1;
      }မဟုတ်ပါက အကယ်၍( sqlite3GlobalConfig.mnReq>(1<<12) ){
        /* cap min request size at 2^12 */
        sqlite3GlobalConfig.mnReq = (1<<12);
      }

      အကယ်၍( sqlite3GlobalConfig.pHeap==0 ){
        /* EVIDENCE-OF: R-49920-60189 If the first pointer (the memory pointer)
        ** is NULL, then SQLite reverts to using its default memory allocator
        ** (the system malloc() implementation), undoing any prior invocation of
        ** SQLITE_CONFIG_MALLOC.
        **
        ** Setting sqlite3GlobalConfig.m to all zeros will cause malloc to
        ** revert to its default implementation when sqlite3_initialize() is run
        */
        memset(&sqlite3GlobalConfig.m, 0, parimaa_na(sqlite3GlobalConfig.m));
      }မဟုတ်ပါက{
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
      bha_nga;
    }
#endif

    pak_sa SQLITE_CONFIG_LOOKASIDE: {
      sqlite3GlobalConfig.szLookaside = va_arg(ap, ကိန်းပြည့်);
      sqlite3GlobalConfig.nLookaside = va_arg(ap, ကိန်းပြည့်);
      bha_nga;
    }
   
    /* Record a pointer to the logger function and its first argument.
    ** The default is NULL.  Logging is disabled if the function pointer is
    ** NULL.
    */
    pak_sa SQLITE_CONFIG_LOG: {
      /* MSVC is picky about pulling func ptrs from va lists.
      ** http://support.microsoft.com/kb/47961
      ** sqlite3GlobalConfig.xLog = va_arg(ap, void(*)(void*,int,const char*));
      */
      typedef void(*LOGFUNC_t)(void*,ကိန်းပြည့်,const var_na*);
      LOGFUNC_t xLog = va_arg(ap, LOGFUNC_t);
      void *pLogArg = va_arg(ap, void*);
      AtomicStore(&sqlite3GlobalConfig.xLog, xLog);
      AtomicStore(&sqlite3GlobalConfig.pLogArg, pLogArg);
      bha_nga;
    }

    /* EVIDENCE-OF: R-55548-33817 The compile-time setting for URI filenames
    ** can be changed at start-time using the
    ** sqlite3_config(SQLITE_CONFIG_URI,1) or
    ** sqlite3_config(SQLITE_CONFIG_URI,0) configuration calls.
    */
    pak_sa SQLITE_CONFIG_URI: {
      /* EVIDENCE-OF: R-25451-61125 The SQLITE_CONFIG_URI option takes a single
      ** argument of type int. If non-zero, then URI handling is globally
      ** enabled. If the parameter is zero, then URI handling is globally
      ** disabled. */
      ကိန်းပြည့် bOpenUri = va_arg(ap, ကိန်းပြည့်);
      AtomicStore(&sqlite3GlobalConfig.bOpenUri, bOpenUri);
      bha_nga;
    }

    pak_sa SQLITE_CONFIG_COVERING_INDEX_SCAN: {
      /* EVIDENCE-OF: R-36592-02772 The SQLITE_CONFIG_COVERING_INDEX_SCAN
      ** option takes a single integer argument which is interpreted as a
      ** boolean in order to enable or disable the use of covering indices for
      ** full table scans in the query optimizer. */
      sqlite3GlobalConfig.bUseCis = va_arg(ap, ကိန်းပြည့်);
      bha_nga;
    }

#ifdef SQLITE_ENABLE_SQLLOG
    pak_sa SQLITE_CONFIG_SQLLOG: {
      typedef void(*SQLLOGFUNC_t)(void*, sqlite3*, const var_na*, ကိန်းပြည့်);
      sqlite3GlobalConfig.xSqllog = va_arg(ap, SQLLOGFUNC_t);
      sqlite3GlobalConfig.pSqllogArg = va_arg(ap, void *);
      bha_nga;
    }
#endif

    pak_sa SQLITE_CONFIG_MMAP_SIZE: {
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
      အကယ်၍( mxMmap<0 || mxMmap>SQLITE_MAX_MMAP_SIZE ){
        mxMmap = SQLITE_MAX_MMAP_SIZE;
      }
      အကယ်၍( szMmap<0 ) szMmap = SQLITE_DEFAULT_MMAP_SIZE;
      အကယ်၍( szMmap>mxMmap) szMmap = mxMmap;
      sqlite3GlobalConfig.mxMmap = mxMmap;
      sqlite3GlobalConfig.szMmap = szMmap;
      bha_nga;
    }

#အကယ်၍ SQLITE_OS_WIN && defined(SQLITE_WIN32_MALLOC) /* IMP: R-04780-55815 */
    pak_sa SQLITE_CONFIG_WIN32_HEAPSIZE: {
      /* EVIDENCE-OF: R-34926-03360 SQLITE_CONFIG_WIN32_HEAPSIZE takes a 32-bit
      ** unsigned integer value that specifies the maximum size of the created
      ** heap. */
      sqlite3GlobalConfig.nHeap = va_arg(ap, ကိန်းပြည့်);
      bha_nga;
    }
#endif

    pak_sa SQLITE_CONFIG_PMASZ: {
      sqlite3GlobalConfig.szPma = va_arg(ap, unsigned ကိန်းပြည့်);
      bha_nga;
    }

    pak_sa SQLITE_CONFIG_STMTJRNL_SPILL: {
      sqlite3GlobalConfig.nStmtSpill = va_arg(ap, ကိန်းပြည့်);
      bha_nga;
    }

#ifdef SQLITE_ENABLE_SORTER_REFERENCES
    pak_sa SQLITE_CONFIG_SORTERREF_SIZE: {
      ကိန်းပြည့် iVal = va_arg(ap, ကိန်းပြည့်);
      အကယ်၍( iVal<0 ){
        iVal = SQLITE_DEFAULT_SORTERREF_SIZE;
      }
      sqlite3GlobalConfig.szSorterRef = (u32)iVal;
      bha_nga;
    }
#endif /* SQLITE_ENABLE_SORTER_REFERENCES */

#ifndef SQLITE_OMIT_DESERIALIZE
    pak_sa SQLITE_CONFIG_MEMDB_MAXSIZE: {
      sqlite3GlobalConfig.mxMemdbSize = va_arg(ap, sqlite3_int64);
      bha_nga;
    }
#endif /* SQLITE_OMIT_DESERIALIZE */

    pak_sa SQLITE_CONFIG_ROWID_IN_VIEW: {
      ကိန်းပြည့် *pVal = va_arg(ap,ကိန်းပြည့်*);
#ifdef SQLITE_ALLOW_ROWID_IN_VIEW
      အကယ်၍( 0==*pVal ) sqlite3GlobalConfig.mNoVisibleRowid = TF_NoVisibleRowid;
      အကယ်၍( 1==*pVal ) sqlite3GlobalConfig.mNoVisibleRowid = 0;
      *pVal = (sqlite3GlobalConfig.mNoVisibleRowid==0);
#မဟုတ်ပါက
      *pVal = 0;
#endif
      bha_nga;
    }

    default: {
      rc = SQLITE_ERROR;
      bha_nga;
    }
  }
  va_end(ap);
  ပြန်ပေး rc;
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
static ကိန်းပြည့် setupLookaside(
  sqlite3 *db,    /* Database connection being configured */
  void *pBuf,     /* Memory to use for lookaside.  May be NULL */
  ကိန်းပြည့် sz,         /* Desired size of each lookaside memory slot */
  ကိန်းပြည့် cnt         /* Number of slots to allocate */
){
#ifndef SQLITE_OMIT_LOOKASIDE
  void *pStart;          /* Start of the lookaside buffer */
  sqlite3_int64 szAlloc; /* Total space set aside for lookaside memory */
  ကိန်းပြည့် nBig;              /* Number of full-size slots */
  ကိန်းပြည့် nSm;               /* Number smaller LOOKASIDE_SMALL-byte slots */
 
  အကယ်၍( sqlite3LookasideUsed(db,0)>0 ){
    ပြန်ပေး SQLITE_BUSY;
  }
  /* Free any existing lookaside buffer for this handle before
  ** allocating a new one so we don't have to have space for
  ** both at the same time.
  */
  အကယ်၍( db->lookaside.bMalloced ){
    sqlite3_free(db->lookaside.pStart);
  }
  /* The size of a lookaside slot after ROUNDDOWN8 needs to be larger
  ** than a pointer and small enough to fit in a u16.
  */
  sz = ROUNDDOWN8(sz);
  အကယ်၍( sz<=(ကိန်းပြည့်)parimaa_na(LookasideSlot*) ) sz = 0;
  အကယ်၍( sz>65528 ) sz = 65528;
  /* Count must be at least 1 to be useful, but not so large as to use
  ** more than 0x7fff0000 total bytes for lookaside. */
  အကယ်၍( cnt<1 ) cnt = 0;
  အကယ်၍( sz>0 && cnt>(0x7fff0000/sz) ) cnt = 0x7fff0000/sz;
  szAlloc = (i64)sz*(i64)cnt;
  အကယ်၍( szAlloc==0 ){
    sz = 0;
    pStart = 0;
  }မဟုတ်ပါက အကယ်၍( pBuf==0 ){
    sqlite3BeginBenignMalloc();
    pStart = sqlite3Malloc( szAlloc );
    sqlite3EndBenignMalloc();
    အကယ်၍( pStart ) szAlloc = sqlite3MallocSize(pStart);
  }မဟုတ်ပါက{
    pStart = pBuf;
  }
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
  အကယ်၍( sz>=LOOKASIDE_SMALL*3 ){
    nBig = szAlloc/(3*LOOKASIDE_SMALL+sz);
    nSm = (szAlloc - (i64)sz*(i64)nBig)/LOOKASIDE_SMALL;
  }မဟုတ်ပါက အကယ်၍( sz>=LOOKASIDE_SMALL*2 ){
    nBig = szAlloc/(LOOKASIDE_SMALL+sz);
    nSm = (szAlloc - (i64)sz*(i64)nBig)/LOOKASIDE_SMALL;
  }မဟုတ်ပါက
#endif /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
  အကယ်၍( sz>0 ){
    nBig = szAlloc/sz;
    nSm = 0;
  }မဟုတ်ပါက{
    nBig = nSm = 0;
  }
  db->lookaside.pStart = pStart;
  db->lookaside.pInit = 0;
  db->lookaside.pFree = 0;
  db->lookaside.sz = (u16)sz;
  db->lookaside.szTrue = (u16)sz;
  အကယ်၍( pStart ){
    ကိန်းပြည့် i;
    LookasideSlot *p;
    assert( sz > (ကိန်းပြည့်)parimaa_na(LookasideSlot*) );
    p = (LookasideSlot*)pStart;
    အတွက်(i=0; i<nBig; i++){
      p->pNext = db->lookaside.pInit;
      db->lookaside.pInit = p;
      p = (LookasideSlot*)&((u8*)p)[sz];
    }
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
    db->lookaside.pSmallInit = 0;
    db->lookaside.pSmallFree = 0;
    db->lookaside.pMiddle = p;
    အတွက်(i=0; i<nSm; i++){
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
  }မဟုတ်ပါက{
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
  ပြန်ပေး SQLITE_OK;
}

/*
** Return the mutex associated with a database connection.
*/
sqlite3_mutex *sqlite3_db_mutex(sqlite3 *db){
#ifdef SQLITE_ENABLE_API_ARMOR
  အကယ်၍( !sqlite3SafetyCheckOk(db) ){
    (void)SQLITE_MISUSE_BKPT;
    ပြန်ပေး 0;
  }
#endif
  ပြန်ပေး db->mutex;
}

/*
** Free up as much memory as we can from the given database
** connection.
*/
ကိန်းပြည့် sqlite3_db_release_memory(sqlite3 *db){
  ကိန်းပြည့် i;

#ifdef SQLITE_ENABLE_API_ARMOR
  အကယ်၍( !sqlite3SafetyCheckOk(db) ) ပြန်ပေး SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(db->mutex);
  sqlite3BtreeEnterAll(db);
  အတွက်(i=0; i<db->nDb; i++){
    Btree *pBt = db->aDb[i].pBt;
    အကယ်၍( pBt ){
      Pager *pPager = sqlite3BtreePager(pBt);
      sqlite3PagerShrink(pPager);
    }
  }
  sqlite3BtreeLeaveAll(db);
  sqlite3_mutex_leave(db->mutex);
  ပြန်ပေး SQLITE_OK;
}

/*
** Flush any dirty pages in the pager-cache for any attached database
** to disk.
*/
ကိန်းပြည့် sqlite3_db_cacheflush(sqlite3 *db){
  ကိန်းပြည့် i;
  ကိန်းပြည့် rc = SQLITE_OK;
  ကိန်းပြည့် bSeenBusy = 0;

#ifdef SQLITE_ENABLE_API_ARMOR
  အကယ်၍( !sqlite3SafetyCheckOk(db) ) ပြန်ပေး SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(db->mutex);
  sqlite3BtreeEnterAll(db);
  အတွက်(i=0; rc==SQLITE_OK && i<db->nDb; i++){
    Btree *pBt = db->aDb[i].pBt;
    အကယ်၍( pBt && sqlite3BtreeTxnState(pBt)==SQLITE_TXN_WRITE ){
      Pager *pPager = sqlite3BtreePager(pBt);
      rc = sqlite3PagerFlush(pPager);
      အကယ်၍( rc==SQLITE_BUSY ){
        bSeenBusy = 1;
        rc = SQLITE_OK;
      }
    }
  }
  sqlite3BtreeLeaveAll(db);
  sqlite3_mutex_leave(db->mutex);
  ပြန်ပေး ((rc==SQLITE_OK && bSeenBusy) ? SQLITE_BUSY : rc);
}

/*
** Configuration settings for an individual database connection
*/
ကိန်းပြည့် sqlite3_db_config(sqlite3 *db, ကိန်းပြည့် op, ...){
  va_list ap;
  ကိန်းပြည့် rc;

#ifdef SQLITE_ENABLE_API_ARMOR
  အကယ်၍( !sqlite3SafetyCheckOk(db) ) ပြန်ပေး SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(db->mutex);
  va_start(ap, op);
  switch( op ){
    pak_sa SQLITE_DBCONFIG_MAINDBNAME: {
      /* IMP: R-06824-28531 */
      /* IMP: R-36257-52125 */
      db->aDb[0].zDbSName = va_arg(ap,var_na*);
      rc = SQLITE_OK;
      bha_nga;
    }
    pak_sa SQLITE_DBCONFIG_LOOKASIDE: {
      void *pBuf = va_arg(ap, void*); /* IMP: R-26835-10964 */
      ကိန်းပြည့် sz = va_arg(ap, ကိန်းပြည့်);       /* IMP: R-47871-25994 */
      ကိန်းပြည့် cnt = va_arg(ap, ကိန်းပြည့်);      /* IMP: R-04460-53386 */
      rc = setupLookaside(db, pBuf, sz, cnt);
      bha_nga;
    }
    pak_sa SQLITE_DBCONFIG_FP_DIGITS: {
      ကိန်းပြည့် nIn = va_arg(ap, ကိန်းပြည့်);
      ကိန်းပြည့် *pOut = va_arg(ap, ကိန်းပြည့်*);
      အကယ်၍( nIn>3 && nIn<24 ) db->nFpDigit = (u8)nIn;
      အကယ်၍( pOut ) *pOut = db->nFpDigit;
      rc = SQLITE_OK;
      bha_nga;
    }
    default: {
      static const sa_nracanaa {
        ကိန်းပြည့် op;      /* The opcode */
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
      unsigned ကိန်းပြည့် i;
      rc = SQLITE_ERROR; /* IMP: R-42790-23372 */
      အတွက်(i=0; i<ArraySize(aFlagOp); i++){
        အကယ်၍( aFlagOp[i].op==op ){
          ကိန်းပြည့် onoff = va_arg(ap, ကိန်းပြည့်);
          ကိန်းပြည့် *pRes = va_arg(ap, ကိန်းပြည့်*);
          u64 oldFlags = db->flags;
          အကယ်၍( onoff>0 ){
            db->flags |= aFlagOp[i].mask;
          }မဟုတ်ပါက အကယ်၍( onoff==0 ){
            db->flags &= ~(u64)aFlagOp[i].mask;
          }
          အကယ်၍( oldFlags!=db->flags ){
            sqlite3ExpirePreparedStatements(db, 0);
          }
          အကယ်၍( pRes ){
            *pRes = (db->flags & aFlagOp[i].mask)!=0;
          }
          rc = SQLITE_OK;
          bha_nga;
        }
      }
      bha_nga;
    }
  }
  va_end(ap);
  sqlite3_mutex_leave(db->mutex);
  ပြန်ပေး rc;
}

/*
** This is the default collating function named "BINARY" which is always
** available.
*/
ကိန်းပြည့် sqlite3BinaryCompare(
  void *NotUsed,
  ကိန်းပြည့် nKey1, const void *pKey1,
  ကိန်းပြည့် nKey2, const void *pKey2
){
  ကိန်းပြည့် rc, n;
  UNUSED_PARAMETER(NotUsed);
  n = nKey1<nKey2 ? nKey1 : nKey2;
  /* EVIDENCE-OF: R-65033-28449 The built-in BINARY collation compares
  ** strings byte by byte using the memcmp() function from the standard C
  ** library. */
  assert( pKey1 && pKey2 );
  rc = memcmp(pKey1, pKey2, n);
  အကယ်၍( rc==0 ){
    rc = nKey1 - nKey2;
  }
  ပြန်ပေး rc;
}

/*
** This is the collating function named "RTRIM" which is always
** available.  Ignore trailing spaces.
*/
static ကိန်းပြည့် rtrimCollFunc(
  void *pUser,
  ကိန်းပြည့် nKey1, const void *pKey1,
  ကိန်းပြည့် nKey2, const void *pKey2
){
  const u8 *pK1 = (const u8*)pKey1;
  const u8 *pK2 = (const u8*)pKey2;
  နေစဉ်( nKey1 && pK1[nKey1-1]==' ' ) nKey1--;
  နေစဉ်( nKey2 && pK2[nKey2-1]==' ' ) nKey2--;
  ပြန်ပေး sqlite3BinaryCompare(pUser, nKey1, pKey1, nKey2, pKey2);
}

/*
** Return true if CollSeq is the default built-in BINARY.
*/
#အကယ်၍ !SQLITE_USES_INLINE
ကိန်းပြည့် sqlite3IsBinary(const CollSeq *p){
  assert( p==0 || p->xCmp!=sqlite3BinaryCompare
       || strcmp(p->zName,"BINARY")==0 );
  ပြန်ပေး p==0 || p->xCmp==sqlite3BinaryCompare;
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
static ကိန်းပြည့် nocaseCollatingFunc(
  void *NotUsed,
  ကိန်းပြည့် nKey1, const void *pKey1,
  ကိန်းပြည့် nKey2, const void *pKey2
){
  ကိန်းပြည့် r = sqlite3StrNICmp(
      (const var_na *)pKey1, (const var_na *)pKey2, (nKey1<nKey2)?nKey1:nKey2);
  UNUSED_PARAMETER(NotUsed);
  အကယ်၍( 0==r ){
    r = nKey1-nKey2;
  }
  ပြန်ပေး r;
}

/*
** Return the ROWID of the most recent insert
*/
sqlite_int64 sqlite3_last_insert_rowid(sqlite3 *db){
  i64 iRet;
#ifdef SQLITE_ENABLE_API_ARMOR
  အကယ်၍( !sqlite3SafetyCheckOk(db) ){
    (void)SQLITE_MISUSE_BKPT;
    ပြန်ပေး 0;
  }
#endif
  sqlite3_mutex_enter(db->mutex);
  iRet = db->lastRowid;
  sqlite3_mutex_leave(db->mutex);
  ပြန်ပေး iRet;
}

/*
** Set the value returned by the sqlite3_last_insert_rowid() API function.
*/
void sqlite3_set_last_insert_rowid(sqlite3 *db, sqlite3_int64 iRowid){
#ifdef SQLITE_ENABLE_API_ARMOR
  အကယ်၍( !sqlite3SafetyCheckOk(db) ){
    (void)SQLITE_MISUSE_BKPT;
    ပြန်ပေး;
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
  အကယ်၍( !sqlite3SafetyCheckOk(db) ){
    (void)SQLITE_MISUSE_BKPT;
    ပြန်ပေး 0;
  }
#endif
  sqlite3_mutex_enter(db->mutex);
  iRet = db->nChange;
  sqlite3_mutex_leave(db->mutex);
  ပြန်ပေး iRet;
}
ကိန်းပြည့် sqlite3_changes(sqlite3 *db){
  ပြန်ပေး (ကိန်းပြည့်)sqlite3_changes64(db);
}

/*
** Return the number of changes since the database handle was opened.
*/
sqlite3_int64 sqlite3_total_changes64(sqlite3 *db){
  i64 iRet;
#ifdef SQLITE_ENABLE_API_ARMOR
  အကယ်၍( !sqlite3SafetyCheckOk(db) ){
    (void)SQLITE_MISUSE_BKPT;
    ပြန်ပေး 0;
  }
#endif
  sqlite3_mutex_enter(db->mutex);
  iRet = db->nTotalChange;
  sqlite3_mutex_leave(db->mutex);
  ပြန်ပေး iRet;
}
ကိန်းပြည့် sqlite3_total_changes(sqlite3 *db){
  ပြန်ပေး (ကိန်းပြည့်)sqlite3_total_changes64(db);
}

/*
** Close all open savepoints. This function only manipulates fields of the
** database handle object, it does not close any savepoints that may be open
** at the b-tree/pager level.
*/
void sqlite3CloseSavepoints(sqlite3 *db){
  နေစဉ်( db->pSavepoint ){
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
  အကယ်၍( pDestructor ){
    pDestructor->nRef--;
    အကယ်၍( pDestructor->nRef==0 ){
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
  ကိန်းပြည့် i;
  HashElem *p;
  sqlite3BtreeEnterAll(db);
  အတွက်(i=0; i<db->nDb; i++){
    Schema *pSchema = db->aDb[i].pSchema;
    အကယ်၍( pSchema ){
      အတွက်(p=sqliteHashFirst(&pSchema->tblHash); p; p=sqliteHashNext(p)){
        Table *pTab = (Table *)sqliteHashData(p);
        အကယ်၍( IsVirtual(pTab) ) sqlite3VtabDisconnect(db, pTab);
      }
    }
  }
  အတွက်(p=sqliteHashFirst(&db->aModule); p; p=sqliteHashNext(p)){
    Module *pMod = (Module *)sqliteHashData(p);
    အကယ်၍( pMod->pEpoTab ){
      sqlite3VtabDisconnect(db, pMod->pEpoTab);
    }
  }
  sqlite3VtabUnlockList(db);
  sqlite3BtreeLeaveAll(db);
#မဟုတ်ပါက
  UNUSED_PARAMETER(db);
#endif
}

/*
** Return TRUE if database connection db has unfinalized prepared
** statements or unfinished sqlite3_backup objects. 
*/
static ကိန်းပြည့် connectionIsBusy(sqlite3 *db){
  ကိန်းပြည့် j;
  assert( sqlite3_mutex_held(db->mutex) );
  အကယ်၍( db->pVdbe ) ပြန်ပေး 1;
  အတွက်(j=0; j<db->nDb; j++){
    Btree *pBt = db->aDb[j].pBt;
    အကယ်၍( pBt && sqlite3BtreeIsInBackup(pBt) ) ပြန်ပေး 1;
  }
  ပြန်ပေး 0;
}

/*
** Close an existing SQLite database
*/
static ကိန်းပြည့် sqlite3Close(sqlite3 *db, ကိန်းပြည့် forceZombie){
  အကယ်၍( !db ){
    /* EVIDENCE-OF: R-63257-11740 Calling sqlite3_close() or
    ** sqlite3_close_v2() with a NULL pointer argument is a harmless no-op. */
    ပြန်ပေး SQLITE_OK;
  }
  အကယ်၍( !sqlite3SafetyCheckSickOrOk(db) ){
    ပြန်ပေး SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(db->mutex);
  အကယ်၍( db->mTrace & SQLITE_TRACE_CLOSE ){
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
  အကယ်၍( !forceZombie && connectionIsBusy(db) ){
    sqlite3ErrorWithMsg(db, SQLITE_BUSY, "unable to close due to unfinalized "
       "statements or unfinished backups");
    sqlite3_mutex_leave(db->mutex);
    ပြန်ပေး SQLITE_BUSY;
  }

#ifdef SQLITE_ENABLE_SQLLOG
  အကယ်၍( sqlite3GlobalConfig.xSqllog ){
    /* Closing the handle. Fourth parameter is passed the value 2. */
    sqlite3GlobalConfig.xSqllog(sqlite3GlobalConfig.pSqllogArg, db, 0, 2);
  }
#endif

  နေစဉ်( db->pDbData ){
    DbClientData *p = db->pDbData;
    db->pDbData = p->pNext;
    assert( p->pData!=0 );
    အကယ်၍( p->xDestructor ) p->xDestructor(p->pData);
    sqlite3_free(p);
  }

  /* Convert the connection into a zombie and then close it.
  */
  db->eOpenState = SQLITE_STATE_ZOMBIE;
  sqlite3LeaveMutexAndCloseZombie(db);
  ပြန်ပေး SQLITE_OK;
}

/*
** Return the transaction state for a single databse, or the maximum
** transaction state over all attached databases if zSchema is null.
*/
ကိန်းပြည့် sqlite3_txn_state(sqlite3 *db, const var_na *zSchema){
  ကိန်းပြည့် iDb, nDb;
  ကိန်းပြည့် iTxn = -1;
#ifdef SQLITE_ENABLE_API_ARMOR
  အကယ်၍( !sqlite3SafetyCheckOk(db) ){
    (void)SQLITE_MISUSE_BKPT;
    ပြန်ပေး -1;
  }
#endif
  sqlite3_mutex_enter(db->mutex);
  အကယ်၍( zSchema ){
    nDb = iDb = sqlite3FindDbName(db, zSchema);
    အကယ်၍( iDb<0 ) nDb--;
  }မဟုတ်ပါက{
    iDb = 0;
    nDb = db->nDb-1;
  }
  အတွက်(; iDb<=nDb; iDb++){
    Btree *pBt = db->aDb[iDb].pBt;
    ကိန်းပြည့် x = pBt!=0 ? sqlite3BtreeTxnState(pBt) : SQLITE_TXN_NONE;
    အကယ်၍( x>iTxn ) iTxn = x;
  }
  sqlite3_mutex_leave(db->mutex);
  ပြန်ပေး iTxn;
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
ကိန်းပြည့် sqlite3_close(sqlite3 *db){ ပြန်ပေး sqlite3Close(db,0); }
ကိန်းပြည့် sqlite3_close_v2(sqlite3 *db){ ပြန်ပေး sqlite3Close(db,1); }


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
  ကိန်းပြည့် j;

  /* If there are outstanding sqlite3_stmt or sqlite3_backup objects
  ** or if the connection has not yet been closed by sqlite3_close_v2(),
  ** then just leave the mutex and return.
  */
  အကယ်၍( db->eOpenState!=SQLITE_STATE_ZOMBIE || connectionIsBusy(db) ){
    sqlite3_mutex_leave(db->mutex);
    ပြန်ပေး;
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
  အတွက်(j=0; j<db->nDb; j++){
    sa_nracanaa Db *pDb = &db->aDb[j];
    အကယ်၍( pDb->pBt ){
      sqlite3BtreeClose(pDb->pBt);
      pDb->pBt = 0;
      အကယ်၍( j!=1 ){
        pDb->pSchema = 0;
      }
    }
  }
  /* Clear the TEMP schema separately and last */
  အကယ်၍( db->aDb[1].pSchema ){
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

  အတွက်(i=sqliteHashFirst(&db->aFunc); i; i=sqliteHashNext(i)){
    FuncDef *pNext, *p;
    p = sqliteHashData(i);
    do{
      functionDestroy(db, p);
      pNext = p->pNext;
      sqlite3DbFree(db, p);
      p = pNext;
    }နေစဉ်( p );
  }
  sqlite3HashClear(&db->aFunc);
  အတွက်(i=sqliteHashFirst(&db->aCollSeq); i; i=sqliteHashNext(i)){
    CollSeq *pColl = (CollSeq *)sqliteHashData(i);
    /* Invoke any destructors registered for collation sequence user data. */
    အတွက်(j=0; j<3; j++){
      အကယ်၍( pColl[j].xDel ){
        pColl[j].xDel(pColl[j].pUser);
      }
    }
    sqlite3DbFree(db, pColl);
  }
  sqlite3HashClear(&db->aCollSeq);
#ifndef SQLITE_OMIT_VIRTUALTABLE
  အတွက်(i=sqliteHashFirst(&db->aModule); i; i=sqliteHashNext(i)){
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
  အကယ်၍( db->xAutovacDestr ){
    db->xAutovacDestr(db->pAutovacPagesArg);
  }
  sqlite3_mutex_leave(db->mutex);
  db->eOpenState = SQLITE_STATE_CLOSED;
  sqlite3_mutex_free(db->mutex);
  assert( sqlite3LookasideUsed(db,0)==0 );
  အကယ်၍( db->lookaside.bMalloced ){
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
void sqlite3RollbackAll(sqlite3 *db, ကိန်းပြည့် tripCode){
  ကိန်းပြည့် i;
  ကိန်းပြည့် inTrans = 0;
  ကိန်းပြည့် schemaChange;
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

  အတွက်(i=0; i<db->nDb; i++){
    Btree *p = db->aDb[i].pBt;
    အကယ်၍( p ){
      အကယ်၍( sqlite3BtreeTxnState(p)==SQLITE_TXN_WRITE ){
        inTrans = 1;
      }
      sqlite3BtreeRollback(p, tripCode, !schemaChange);
    }
  }
  sqlite3VtabRollback(db);
  sqlite3EndBenignMalloc();

  အကယ်၍( schemaChange ){
    sqlite3ExpirePreparedStatements(db, 0);
    sqlite3ResetAllSchemasOfConnection(db);
  }
  sqlite3BtreeLeaveAll(db);

  /* Any deferred constraint violations have now been resolved. */
  db->nDeferredCons = 0;
  db->nDeferredImmCons = 0;
  db->flags &= ~(u64)(SQLITE_DeferFKs|SQLITE_CorruptRdOnly);

  /* If one has been configured, invoke the rollback-hook callback */
  အကယ်၍( db->xRollbackCallback && (inTrans || !db->autoCommit) ){
    db->xRollbackCallback(db->pRollbackArg);
  }
}

/*
** Return a static string containing the name corresponding to the error code
** specified in the argument.
*/
#အကယ်၍ defined(SQLITE_NEED_ERR_NAME)
const var_na *sqlite3ErrName(ကိန်းပြည့် rc){
  const var_na *zName = 0;
  ကိန်းပြည့် i, origRc = rc;
  အတွက်(i=0; i<2 && zName==0; i++, rc &= 0xff){
    switch( rc ){
      pak_sa SQLITE_OK:                 zName = "SQLITE_OK";                bha_nga;
      pak_sa SQLITE_ERROR:              zName = "SQLITE_ERROR";             bha_nga;
      pak_sa SQLITE_ERROR_SNAPSHOT:     zName = "SQLITE_ERROR_SNAPSHOT";    bha_nga;
      pak_sa SQLITE_ERROR_RETRY:        zName = "SQLITE_ERROR_RETRY";       bha_nga;
      pak_sa SQLITE_ERROR_MISSING_COLLSEQ:
                                zName = "SQLITE_ERROR_MISSING_COLLSEQ";   bha_nga;
      pak_sa SQLITE_INTERNAL:           zName = "SQLITE_INTERNAL";          bha_nga;
      pak_sa SQLITE_PERM:               zName = "SQLITE_PERM";              bha_nga;
      pak_sa SQLITE_ABORT:              zName = "SQLITE_ABORT";             bha_nga;
      pak_sa SQLITE_ABORT_ROLLBACK:     zName = "SQLITE_ABORT_ROLLBACK";    bha_nga;
      pak_sa SQLITE_BUSY:               zName = "SQLITE_BUSY";              bha_nga;
      pak_sa SQLITE_BUSY_RECOVERY:      zName = "SQLITE_BUSY_RECOVERY";     bha_nga;
      pak_sa SQLITE_BUSY_SNAPSHOT:      zName = "SQLITE_BUSY_SNAPSHOT";     bha_nga;
      pak_sa SQLITE_LOCKED:             zName = "SQLITE_LOCKED";            bha_nga;
      pak_sa SQLITE_LOCKED_SHAREDCACHE: zName = "SQLITE_LOCKED_SHAREDCACHE";bha_nga;
      pak_sa SQLITE_NOMEM:              zName = "SQLITE_NOMEM";             bha_nga;
      pak_sa SQLITE_READONLY:           zName = "SQLITE_READONLY";          bha_nga;
      pak_sa SQLITE_READONLY_RECOVERY:  zName = "SQLITE_READONLY_RECOVERY"; bha_nga;
      pak_sa SQLITE_READONLY_CANTINIT:  zName = "SQLITE_READONLY_CANTINIT"; bha_nga;
      pak_sa SQLITE_READONLY_ROLLBACK:  zName = "SQLITE_READONLY_ROLLBACK"; bha_nga;
      pak_sa SQLITE_READONLY_DBMOVED:   zName = "SQLITE_READONLY_DBMOVED";  bha_nga;
      pak_sa SQLITE_READONLY_DIRECTORY: zName = "SQLITE_READONLY_DIRECTORY";bha_nga;
      pak_sa SQLITE_INTERRUPT:          zName = "SQLITE_INTERRUPT";         bha_nga;
      pak_sa SQLITE_IOERR:              zName = "SQLITE_IOERR";             bha_nga;
      pak_sa SQLITE_IOERR_READ:         zName = "SQLITE_IOERR_READ";        bha_nga;
      pak_sa SQLITE_IOERR_SHORT_READ:   zName = "SQLITE_IOERR_SHORT_READ";  bha_nga;
      pak_sa SQLITE_IOERR_WRITE:        zName = "SQLITE_IOERR_WRITE";       bha_nga;
      pak_sa SQLITE_IOERR_FSYNC:        zName = "SQLITE_IOERR_FSYNC";       bha_nga;
      pak_sa SQLITE_IOERR_DIR_FSYNC:    zName = "SQLITE_IOERR_DIR_FSYNC";   bha_nga;
      pak_sa SQLITE_IOERR_TRUNCATE:     zName = "SQLITE_IOERR_TRUNCATE";    bha_nga;
      pak_sa SQLITE_IOERR_FSTAT:        zName = "SQLITE_IOERR_FSTAT";       bha_nga;
      pak_sa SQLITE_IOERR_UNLOCK:       zName = "SQLITE_IOERR_UNLOCK";      bha_nga;
      pak_sa SQLITE_IOERR_RDLOCK:       zName = "SQLITE_IOERR_RDLOCK";      bha_nga;
      pak_sa SQLITE_IOERR_DELETE:       zName = "SQLITE_IOERR_DELETE";      bha_nga;
      pak_sa SQLITE_IOERR_NOMEM:        zName = "SQLITE_IOERR_NOMEM";       bha_nga;
      pak_sa SQLITE_IOERR_ACCESS:       zName = "SQLITE_IOERR_ACCESS";      bha_nga;
      pak_sa SQLITE_IOERR_CHECKRESERVEDLOCK:
                                zName = "SQLITE_IOERR_CHECKRESERVEDLOCK"; bha_nga;
      pak_sa SQLITE_IOERR_LOCK:         zName = "SQLITE_IOERR_LOCK";        bha_nga;
      pak_sa SQLITE_IOERR_CLOSE:        zName = "SQLITE_IOERR_CLOSE";       bha_nga;
      pak_sa SQLITE_IOERR_DIR_CLOSE:    zName = "SQLITE_IOERR_DIR_CLOSE";   bha_nga;
      pak_sa SQLITE_IOERR_SHMOPEN:      zName = "SQLITE_IOERR_SHMOPEN";     bha_nga;
      pak_sa SQLITE_IOERR_SHMSIZE:      zName = "SQLITE_IOERR_SHMSIZE";     bha_nga;
      pak_sa SQLITE_IOERR_SHMLOCK:      zName = "SQLITE_IOERR_SHMLOCK";     bha_nga;
      pak_sa SQLITE_IOERR_SHMMAP:       zName = "SQLITE_IOERR_SHMMAP";      bha_nga;
      pak_sa SQLITE_IOERR_SEEK:         zName = "SQLITE_IOERR_SEEK";        bha_nga;
      pak_sa SQLITE_IOERR_DELETE_NOENT: zName = "SQLITE_IOERR_DELETE_NOENT";bha_nga;
      pak_sa SQLITE_IOERR_MMAP:         zName = "SQLITE_IOERR_MMAP";        bha_nga;
      pak_sa SQLITE_IOERR_GETTEMPPATH:  zName = "SQLITE_IOERR_GETTEMPPATH"; bha_nga;
      pak_sa SQLITE_IOERR_CONVPATH:     zName = "SQLITE_IOERR_CONVPATH";    bha_nga;
      pak_sa SQLITE_CORRUPT:            zName = "SQLITE_CORRUPT";           bha_nga;
      pak_sa SQLITE_CORRUPT_VTAB:       zName = "SQLITE_CORRUPT_VTAB";      bha_nga;
      pak_sa SQLITE_NOTFOUND:           zName = "SQLITE_NOTFOUND";          bha_nga;
      pak_sa SQLITE_FULL:               zName = "SQLITE_FULL";              bha_nga;
      pak_sa SQLITE_CANTOPEN:           zName = "SQLITE_CANTOPEN";          bha_nga;
      pak_sa SQLITE_CANTOPEN_NOTEMPDIR: zName = "SQLITE_CANTOPEN_NOTEMPDIR";bha_nga;
      pak_sa SQLITE_CANTOPEN_ISDIR:     zName = "SQLITE_CANTOPEN_ISDIR";    bha_nga;
      pak_sa SQLITE_CANTOPEN_FULLPATH:  zName = "SQLITE_CANTOPEN_FULLPATH"; bha_nga;
      pak_sa SQLITE_CANTOPEN_CONVPATH:  zName = "SQLITE_CANTOPEN_CONVPATH"; bha_nga;
      pak_sa SQLITE_CANTOPEN_SYMLINK:   zName = "SQLITE_CANTOPEN_SYMLINK";  bha_nga;
      pak_sa SQLITE_PROTOCOL:           zName = "SQLITE_PROTOCOL";          bha_nga;
      pak_sa SQLITE_EMPTY:              zName = "SQLITE_EMPTY";             bha_nga;
      pak_sa SQLITE_SCHEMA:             zName = "SQLITE_SCHEMA";            bha_nga;
      pak_sa SQLITE_TOOBIG:             zName = "SQLITE_TOOBIG";            bha_nga;
      pak_sa SQLITE_CONSTRAINT:         zName = "SQLITE_CONSTRAINT";        bha_nga;
      pak_sa SQLITE_CONSTRAINT_UNIQUE:  zName = "SQLITE_CONSTRAINT_UNIQUE"; bha_nga;
      pak_sa SQLITE_CONSTRAINT_TRIGGER: zName = "SQLITE_CONSTRAINT_TRIGGER";bha_nga;
      pak_sa SQLITE_CONSTRAINT_FOREIGNKEY:
                                zName = "SQLITE_CONSTRAINT_FOREIGNKEY";   bha_nga;
      pak_sa SQLITE_CONSTRAINT_CHECK:   zName = "SQLITE_CONSTRAINT_CHECK";  bha_nga;
      pak_sa SQLITE_CONSTRAINT_PRIMARYKEY:
                                zName = "SQLITE_CONSTRAINT_PRIMARYKEY";   bha_nga;
      pak_sa SQLITE_CONSTRAINT_NOTNULL: zName = "SQLITE_CONSTRAINT_NOTNULL";bha_nga;
      pak_sa SQLITE_CONSTRAINT_COMMITHOOK:
                                zName = "SQLITE_CONSTRAINT_COMMITHOOK";   bha_nga;
      pak_sa SQLITE_CONSTRAINT_VTAB:    zName = "SQLITE_CONSTRAINT_VTAB";   bha_nga;
      pak_sa SQLITE_CONSTRAINT_FUNCTION:
                                zName = "SQLITE_CONSTRAINT_FUNCTION";     bha_nga;
      pak_sa SQLITE_CONSTRAINT_ROWID:   zName = "SQLITE_CONSTRAINT_ROWID";  bha_nga;
      pak_sa SQLITE_MISMATCH:           zName = "SQLITE_MISMATCH";          bha_nga;
      pak_sa SQLITE_MISUSE:             zName = "SQLITE_MISUSE";            bha_nga;
      pak_sa SQLITE_NOLFS:              zName = "SQLITE_NOLFS";             bha_nga;
      pak_sa SQLITE_AUTH:               zName = "SQLITE_AUTH";              bha_nga;
      pak_sa SQLITE_FORMAT:             zName = "SQLITE_FORMAT";            bha_nga;
      pak_sa SQLITE_RANGE:              zName = "SQLITE_RANGE";             bha_nga;
      pak_sa SQLITE_NOTADB:             zName = "SQLITE_NOTADB";            bha_nga;
      pak_sa SQLITE_ROW:                zName = "SQLITE_ROW";               bha_nga;
      pak_sa SQLITE_NOTICE:             zName = "SQLITE_NOTICE";            bha_nga;
      pak_sa SQLITE_NOTICE_RECOVER_WAL: zName = "SQLITE_NOTICE_RECOVER_WAL";bha_nga;
      pak_sa SQLITE_NOTICE_RECOVER_ROLLBACK:
                                zName = "SQLITE_NOTICE_RECOVER_ROLLBACK"; bha_nga;
      pak_sa SQLITE_NOTICE_RBU:         zName = "SQLITE_NOTICE_RBU"; bha_nga;
      pak_sa SQLITE_WARNING:            zName = "SQLITE_WARNING";           bha_nga;
      pak_sa SQLITE_WARNING_AUTOINDEX:  zName = "SQLITE_WARNING_AUTOINDEX"; bha_nga;
      pak_sa SQLITE_DONE:               zName = "SQLITE_DONE";              bha_nga;
    }
  }
  အကယ်၍( zName==0 ){
    static var_na zBuf[50];
    sqlite3_snprintf(parimaa_na(zBuf), zBuf, "SQLITE_UNKNOWN(%d)", origRc);
    zName = zBuf;
  }
  ပြန်ပေး zName;
}
#endif

/*
** Return a static string that describes the kind of error specified in the
** argument.
*/
const var_na *sqlite3ErrStr(ကိန်းပြည့် rc){
  static const var_na* const aMsg[] = {
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
#မဟုတ်ပါက
    /* SQLITE_NOLFS       */ 0,
#endif
    /* SQLITE_AUTH        */ "authorization denied",
    /* SQLITE_FORMAT      */ 0,
    /* SQLITE_RANGE       */ "column index out of range",
    /* SQLITE_NOTADB      */ "file is not a database",
    /* SQLITE_NOTICE      */ "notification message",
    /* SQLITE_WARNING     */ "warning message",
  };
  const var_na *zErr = "unknown error";
  switch( rc ){
    pak_sa SQLITE_ABORT_ROLLBACK: {
      zErr = "abort due to ROLLBACK";
      bha_nga;
    }
    pak_sa SQLITE_ROW: {
      zErr = "another row available";
      bha_nga;
    }
    pak_sa SQLITE_DONE: {
      zErr = "no more rows available";
      bha_nga;
    }
    default: {
      rc &= 0xff;
      အကယ်၍( ALWAYS(rc>=0) && rc<ArraySize(aMsg) && aMsg[rc]!=0 ){
        zErr = aMsg[rc];
      }
      bha_nga;
    }
  }
  ပြန်ပေး zErr;
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
static ကိန်းပြည့် sqliteDefaultBusyCallback(
  void *ptr,               /* Database connection */
  ကိန်းပြည့် count                /* Number of times table has been busy */
){
#အကယ်၍ SQLITE_OS_WIN || !defined(HAVE_NANOSLEEP) || HAVE_NANOSLEEP
  /* This case is for systems that have support for sleeping for fractions of
  ** a second.  Examples:  All windows systems, unix systems with nanosleep() */
  static const u8 delays[] =
     { 1, 2, 5, 10, 15, 20, 25, 25,  25,  50,  50, 100 };
  static const u8 totals[] =
     { 0, 1, 3,  8, 18, 33, 53, 78, 103, 128, 178, 228 };
# define NDELAY ArraySize(delays)
  sqlite3 *db = (sqlite3 *)ptr;
  ကိန်းပြည့် tmout = db->busyTimeout;
  ကိန်းပြည့် delay, prior;

  assert( count>=0 );
  အကယ်၍( count < NDELAY ){
    delay = delays[count];
    prior = totals[count];
  }မဟုတ်ပါက{
    delay = delays[NDELAY-1];
    prior = totals[NDELAY-1] + delay*(count-(NDELAY-1));
  }
  အကယ်၍( prior + delay > tmout ){
    delay = tmout - prior;
    အကယ်၍( delay<=0 ) ပြန်ပေး 0;
  }
  sqlite3OsSleep(db->pVfs, delay*1000);
  ပြန်ပေး 1;
#မဟုတ်ပါက
  /* This case for unix systems that lack usleep() support.  Sleeping
  ** must be done in increments of whole seconds */
  sqlite3 *db = (sqlite3 *)ptr;
  ကိန်းပြည့် tmout = ((sqlite3 *)ptr)->busyTimeout;
  အကယ်၍( (count+1)*1000 > tmout ){
    ပြန်ပေး 0;
  }
  sqlite3OsSleep(db->pVfs, 1000000);
  ပြန်ပေး 1;
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
ကိန်းပြည့် sqlite3InvokeBusyHandler(BusyHandler *p){
  ကိန်းပြည့် rc;
  အကယ်၍( p->xBusyHandler==0 || p->nBusy<0 ) ပြန်ပေး 0;
  rc = p->xBusyHandler(p->pBusyArg, p->nBusy);
  အကယ်၍( rc==0 ){
    p->nBusy = -1;
  }မဟုတ်ပါက{
    p->nBusy++;
  }
  ပြန်ပေး rc;
}

/*
** This routine sets the busy callback for an Sqlite database to the
** given callback function with the given argument.
*/
ကိန်းပြည့် sqlite3_busy_handler(
  sqlite3 *db,
  ကိန်းပြည့် (*xBusy)(void*,ကိန်းပြည့်),
  void *pArg
){
#ifdef SQLITE_ENABLE_API_ARMOR
  အကယ်၍( !sqlite3SafetyCheckOk(db) ) ပြန်ပေး SQLITE_MISUSE_BKPT;
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
  ပြန်ပေး SQLITE_OK;
}

#ifndef SQLITE_OMIT_PROGRESS_CALLBACK
/*
** This routine sets the progress callback for an Sqlite database to the
** given callback function with the given argument. The progress callback will
** be invoked every nOps opcodes.
*/
void sqlite3_progress_handler(
  sqlite3 *db,
  ကိန်းပြည့် nOps,
  ကိန်းပြည့် (*xProgress)(void*),
  void *pArg
){
#ifdef SQLITE_ENABLE_API_ARMOR
  အကယ်၍( !sqlite3SafetyCheckOk(db) ){
    (void)SQLITE_MISUSE_BKPT;
    ပြန်ပေး;
  }
#endif
  sqlite3_mutex_enter(db->mutex);
  အကယ်၍( nOps>0 ){
    db->xProgress = xProgress;
    db->nProgressOps = (unsigned)nOps;
    db->pProgressArg = pArg;
  }မဟုတ်ပါက{
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
ကိန်းပြည့် sqlite3_busy_timeout(sqlite3 *db, ကိန်းပြည့် ms){
#ifdef SQLITE_ENABLE_API_ARMOR
  အကယ်၍( !sqlite3SafetyCheckOk(db) ) ပြန်ပေး SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(db->mutex);
  အကယ်၍( ms>0 ){
    sqlite3_busy_handler(db, (ကိန်းပြည့်(*)(void*,ကိန်းပြည့်))sqliteDefaultBusyCallback,
                             (void*)db);
    db->busyTimeout = ms;
#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
    db->setlkTimeout = ms;
#endif
  }မဟုတ်ပါက{
    sqlite3_busy_handler(db, 0, 0);
  }
  sqlite3_mutex_leave(db->mutex);
  ပြန်ပေး SQLITE_OK;
}

/*
** Set the setlk timeout value.
*/
ကိန်းပြည့် sqlite3_setlk_timeout(sqlite3 *db, ကိန်းပြည့် ms, ကိန်းပြည့် flags){
#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
  ကိန်းပြည့် iDb;
  ကိန်းပြည့် bBOC = ((flags & SQLITE_SETLK_BLOCK_ON_CONNECT) ? 1 : 0);
#endif
#ifdef SQLITE_ENABLE_API_ARMOR
  အကယ်၍( !sqlite3SafetyCheckOk(db) ) ပြန်ပေး SQLITE_MISUSE_BKPT;
#endif
  အကယ်၍( ms<-1 ) ပြန်ပေး SQLITE_RANGE;
#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
  sqlite3_mutex_enter(db->mutex);
  db->setlkTimeout = ms;
  db->setlkFlags = flags;
  sqlite3BtreeEnterAll(db);
  အတွက်(iDb=0; iDb<db->nDb; iDb++){
    Btree *pBt = db->aDb[iDb].pBt;
    အကယ်၍( pBt ){
      sqlite3_file *fd = sqlite3PagerFile(sqlite3BtreePager(pBt));
      sqlite3OsFileControlHint(fd, SQLITE_FCNTL_BLOCK_ON_CONNECT, (void*)&bBOC);
    }
  }
  sqlite3BtreeLeaveAll(db);
  sqlite3_mutex_leave(db->mutex);
#endif
#အကယ်၍ !defined(SQLITE_ENABLE_API_ARMOR) && !defined(SQLITE_ENABLE_SETLK_TIMEOUT)
  UNUSED_PARAMETER(db);
  UNUSED_PARAMETER(flags);
#endif
  ပြန်ပေး SQLITE_OK;
}

/*
** Cause any pending operation to stop at its earliest opportunity.
*/
void sqlite3_interrupt(sqlite3 *db){
#ifdef SQLITE_ENABLE_API_ARMOR
  အကယ်၍( !sqlite3SafetyCheckOk(db)
   && (db==0 || db->eOpenState!=SQLITE_STATE_ZOMBIE)
  ){
    (void)SQLITE_MISUSE_BKPT;
    ပြန်ပေး;
  }
#endif
  AtomicStore(&db->u1.isInterrupted, 1);
}

/*
** Return true or false depending on whether or not an interrupt is
** pending on connection db.
*/
ကိန်းပြည့် sqlite3_is_interrupted(sqlite3 *db){
#ifdef SQLITE_ENABLE_API_ARMOR
  အကယ်၍( !sqlite3SafetyCheckOk(db)
   && (db==0 || db->eOpenState!=SQLITE_STATE_ZOMBIE)
  ){
    (void)SQLITE_MISUSE_BKPT;
    ပြန်ပေး 0;
  }
#endif
  ပြန်ပေး AtomicLoad(&db->u1.isInterrupted)!=0;
}

/*
** This function is exactly the same as sqlite3_create_function(), except
** that it is designed to be called by internal code. The difference is
** that if a malloc() fails in sqlite3_create_function(), an error code
** is returned and the mallocFailed flag cleared.
*/
ကိန်းပြည့် sqlite3CreateFunc(
  sqlite3 *db,
  const var_na *zFunctionName,
  ကိန်းပြည့် nArg,
  ကိန်းပြည့် enc,
  void *pUserData,
  void (*xSFunc)(sqlite3_context*,ကိန်းပြည့်,sqlite3_value **),
  void (*xStep)(sqlite3_context*,ကိန်းပြည့်,sqlite3_value **),
  void (*xFinal)(sqlite3_context*),
  void (*xValue)(sqlite3_context*),
  void (*xInverse)(sqlite3_context*,ကိန်းပြည့်,sqlite3_value **),
  FuncDestructor *pDestructor
){
  FuncDef *p;
  ကိန်းပြည့် extraFlags;

  assert( sqlite3_mutex_held(db->mutex) );
  assert( xValue==0 || xSFunc==0 );
  အကယ်၍( zFunctionName==0                /* Must have a valid name */
   || (xSFunc!=0 && xFinal!=0)        /* Not both xSFunc and xFinal */
   || ((xFinal==0)!=(xStep==0))       /* Both or neither of xFinal and xStep */
   || ((xValue==0)!=(xInverse==0))    /* Both or neither of xValue, xInverse */
   || (nArg<-1 || nArg>SQLITE_MAX_FUNCTION_ARG)
   || (255<sqlite3Strlen30(zFunctionName))
  ){
    ပြန်ပေး SQLITE_MISUSE_BKPT;
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
    pak_sa SQLITE_UTF16:
      enc = SQLITE_UTF16NATIVE;
      bha_nga;
    pak_sa SQLITE_ANY: {
      ကိန်းပြည့် rc;
      rc = sqlite3CreateFunc(db, zFunctionName, nArg,
           (SQLITE_UTF8|extraFlags)^SQLITE_FUNC_UNSAFE, /* tag-20230109-1 */
           pUserData, xSFunc, xStep, xFinal, xValue, xInverse, pDestructor);
      အကယ်၍( rc==SQLITE_OK ){
        rc = sqlite3CreateFunc(db, zFunctionName, nArg,
             (SQLITE_UTF16LE|extraFlags)^SQLITE_FUNC_UNSAFE, /* tag-20230109-1*/
             pUserData, xSFunc, xStep, xFinal, xValue, xInverse, pDestructor);
      }
      အကယ်၍( rc!=SQLITE_OK ){
        ပြန်ပေး rc;
      }
      enc = SQLITE_UTF16BE;
      bha_nga;
    }
    pak_sa SQLITE_UTF8:
    pak_sa SQLITE_UTF16LE:
    pak_sa SQLITE_UTF16BE:
      bha_nga;
    default:
      enc = SQLITE_UTF8;
      bha_nga;
  }
#မဟုတ်ပါက
  enc = SQLITE_UTF8;
#endif
 
  /* Check if an existing function is being overridden or deleted. If so,
  ** and there are active VMs, then return SQLITE_BUSY. If a function
  ** is being overridden/deleted but there are no active VMs, allow the
  ** operation to continue but invalidate all precompiled statements.
  */
  p = sqlite3FindFunction(db, zFunctionName, nArg, (u8)enc, 0);
  အကယ်၍( p && (p->funcFlags & SQLITE_FUNC_ENCMASK)==(u32)enc && p->nArg==nArg ){
    အကယ်၍( db->nVdbeActive ){
      sqlite3ErrorWithMsg(db, SQLITE_BUSY,
        "unable to delete/modify user-function due to active statements");
      assert( !db->mallocFailed || db->nVdbeExec>0 );
      ပြန်ပေး SQLITE_BUSY;
    }မဟုတ်ပါက{
      sqlite3ExpirePreparedStatements(db, 0);
    }
  }မဟုတ်ပါက အကယ်၍( xSFunc==0 && xFinal==0 ){
    /* Trying to delete a function that does not exist.  This is a no-op.
    ** https://sqlite.org/forum/forumpost/726219164b */
    ပြန်ပေး SQLITE_OK;
  }

  p = sqlite3FindFunction(db, zFunctionName, nArg, (u8)enc, 1);
  assert(p || db->mallocFailed);
  အကယ်၍( !p ){
    ပြန်ပေး SQLITE_NOMEM_BKPT;
  }

  /* If an older version of the function with a configured destructor is
  ** being replaced invoke the destructor function here. */
  functionDestroy(db, p);

  အကယ်၍( pDestructor ){
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
  ပြန်ပေး SQLITE_OK;
}

/*
** Worker function used by utf-8 APIs that create new functions:
**
**    sqlite3_create_function()
**    sqlite3_create_function_v2()
**    sqlite3_create_window_function()
*/
static ကိန်းပြည့် createFunctionApi(
  sqlite3 *db,
  const var_na *zFunc,
  ကိန်းပြည့် nArg,
  ကိန်းပြည့် enc,
  void *p,
  void (*xSFunc)(sqlite3_context*,ကိန်းပြည့်,sqlite3_value**),
  void (*xStep)(sqlite3_context*,ကိန်းပြည့်,sqlite3_value**),
  void (*xFinal)(sqlite3_context*),
  void (*xValue)(sqlite3_context*),
  void (*xInverse)(sqlite3_context*,ကိန်းပြည့်,sqlite3_value**),
  void(*xDestroy)(void*)
){
  ကိန်းပြည့် rc = SQLITE_ERROR;
  FuncDestructor *pArg = 0;

#ifdef SQLITE_ENABLE_API_ARMOR
  အကယ်၍( !sqlite3SafetyCheckOk(db) ){
    ပြန်ပေး SQLITE_MISUSE_BKPT;
  }
#endif
  sqlite3_mutex_enter(db->mutex);
  အကယ်၍( xDestroy ){
    pArg = (FuncDestructor *)sqlite3Malloc(parimaa_na(FuncDestructor));
    အကယ်၍( !pArg ){
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
  အကယ်၍( pArg && pArg->nRef==0 ){
    assert( rc!=SQLITE_OK || (xStep==0 && xFinal==0) );
    xDestroy(p);
    sqlite3_free(pArg);
  }

 out:
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  ပြန်ပေး rc;
}

/*
** Create new user functions.
*/
ကိန်းပြည့် sqlite3_create_function(
  sqlite3 *db,
  const var_na *zFunc,
  ကိန်းပြည့် nArg,
  ကိန်းပြည့် enc,
  void *p,
  void (*xSFunc)(sqlite3_context*,ကိန်းပြည့်,sqlite3_value **),
  void (*xStep)(sqlite3_context*,ကိန်းပြည့်,sqlite3_value **),
  void (*xFinal)(sqlite3_context*)
){
  ပြန်ပေး createFunctionApi(db, zFunc, nArg, enc, p, xSFunc, xStep,
                                    xFinal, 0, 0, 0);
}
ကိန်းပြည့် sqlite3_create_function_v2(
  sqlite3 *db,
  const var_na *zFunc,
  ကိန်းပြည့် nArg,
  ကိန်းပြည့် enc,
  void *p,
  void (*xSFunc)(sqlite3_context*,ကိန်းပြည့်,sqlite3_value **),
  void (*xStep)(sqlite3_context*,ကိန်းပြည့်,sqlite3_value **),
  void (*xFinal)(sqlite3_context*),
  void (*xDestroy)(void *)
){
  ပြန်ပေး createFunctionApi(db, zFunc, nArg, enc, p, xSFunc, xStep,
                                    xFinal, 0, 0, xDestroy);
}
ကိန်းပြည့် sqlite3_create_window_function(
  sqlite3 *db,
  const var_na *zFunc,
  ကိန်းပြည့် nArg,
  ကိန်းပြည့် enc,
  void *p,
  void (*xStep)(sqlite3_context*,ကိန်းပြည့်,sqlite3_value **),
  void (*xFinal)(sqlite3_context*),
  void (*xValue)(sqlite3_context*),
  void (*xInverse)(sqlite3_context*,ကိန်းပြည့်,sqlite3_value **),
  void (*xDestroy)(void *)
){
  ပြန်ပေး createFunctionApi(db, zFunc, nArg, enc, p, 0, xStep,
                                    xFinal, xValue, xInverse, xDestroy);
}

#ifndef SQLITE_OMIT_UTF16
ကိန်းပြည့် sqlite3_create_function16(
  sqlite3 *db,
  const void *zFunctionName,
  ကိန်းပြည့် nArg,
  ကိန်းပြည့် eTextRep,
  void *p,
  void (*xSFunc)(sqlite3_context*,ကိန်းပြည့်,sqlite3_value**),
  void (*xStep)(sqlite3_context*,ကိန်းပြည့်,sqlite3_value**),
  void (*xFinal)(sqlite3_context*)
){
  ကိန်းပြည့် rc;
  var_na *zFunc8;

#ifdef SQLITE_ENABLE_API_ARMOR
  အကယ်၍( !sqlite3SafetyCheckOk(db) || zFunctionName==0 ) ပြန်ပေး SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(db->mutex);
  assert( !db->mallocFailed );
  zFunc8 = sqlite3Utf16to8(db, zFunctionName, -1, SQLITE_UTF16NATIVE);
  rc = sqlite3CreateFunc(db, zFunc8, nArg, eTextRep, p, xSFunc,xStep,xFinal,0,0,0);
  sqlite3DbFree(db, zFunc8);
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  ပြန်ပေး rc;
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
  ကိန်းပြည့် NotUsed,               /* Number of arguments to the function */
  sqlite3_value **NotUsed2   /* Value of each argument */
){
  const var_na *zName = (const var_na*)sqlite3_user_data(context);
  var_na *zErr;
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
ကိန်းပြည့် sqlite3_overload_function(
  sqlite3 *db,
  const var_na *zName,
  ကိန်းပြည့် nArg
){
  ကိန်းပြည့် rc;
  var_na *zCopy;

#ifdef SQLITE_ENABLE_API_ARMOR
  အကယ်၍( !sqlite3SafetyCheckOk(db) || zName==0 || nArg<-2 ){
    ပြန်ပေး SQLITE_MISUSE_BKPT;
  }
#endif
  sqlite3_mutex_enter(db->mutex);
  rc = sqlite3FindFunction(db, zName, nArg, SQLITE_UTF8, 0)!=0;
  sqlite3_mutex_leave(db->mutex);
  အကယ်၍( rc ) ပြန်ပေး SQLITE_OK;
  zCopy = sqlite3_mprintf("%s", zName);
  အကယ်၍( zCopy==0 ) ပြန်ပေး SQLITE_NOMEM;
  ပြန်ပေး sqlite3_create_function_v2(db, zName, nArg, SQLITE_UTF8,
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
void *sqlite3_trace(sqlite3 *db, void(*xTrace)(void*,const var_na*), void *pArg){
  void *pOld;

#ifdef SQLITE_ENABLE_API_ARMOR
  အကယ်၍( !sqlite3SafetyCheckOk(db) ){
    (void)SQLITE_MISUSE_BKPT;
    ပြန်ပေး 0;
  }
#endif
  sqlite3_mutex_enter(db->mutex);
  pOld = db->pTraceArg;
  db->mTrace = xTrace ? SQLITE_TRACE_LEGACY : 0;
  db->trace.xLegacy = xTrace;
  db->pTraceArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  ပြန်ပေး pOld;
}
#endif /* SQLITE_OMIT_DEPRECATED */

/* Register a trace callback using the version-2 interface.
*/
ကိန်းပြည့် sqlite3_trace_v2(
  sqlite3 *db,                               /* Trace this connection */
  unsigned mTrace,                           /* Mask of events to be traced */
  ကိန်းပြည့်(*xTrace)(unsigned,void*,void*,void*),  /* Callback to invoke */
  void *pArg                                 /* Context */
){
#ifdef SQLITE_ENABLE_API_ARMOR
  အကယ်၍( !sqlite3SafetyCheckOk(db) ){
    ပြန်ပေး SQLITE_MISUSE_BKPT;
  }
#endif
  sqlite3_mutex_enter(db->mutex);
  အကယ်၍( mTrace==0 ) xTrace = 0;
  အကယ်၍( xTrace==0 ) mTrace = 0;
  db->mTrace = mTrace;
  db->trace.xV2 = xTrace;
  db->pTraceArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  ပြန်ပေး SQLITE_OK;
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
  void (*xProfile)(void*,const var_na*,sqlite_uint64),
  void *pArg
){
  void *pOld;

#ifdef SQLITE_ENABLE_API_ARMOR
  အကယ်၍( !sqlite3SafetyCheckOk(db) ){
    (void)SQLITE_MISUSE_BKPT;
    ပြန်ပေး 0;
  }
#endif
  sqlite3_mutex_enter(db->mutex);
  pOld = db->pProfileArg;
  db->xProfile = xProfile;
  db->pProfileArg = pArg;
  db->mTrace &= SQLITE_TRACE_NONLEGACY_MASK;
  အကယ်၍( db->xProfile ) db->mTrace |= SQLITE_TRACE_XPROFILE;
  sqlite3_mutex_leave(db->mutex);
  ပြန်ပေး pOld;
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
  ကိန်းပြည့် (*xCallback)(void*),  /* Function to invoke on each commit */
  void *pArg                /* Argument to the function */
){
  void *pOld;

#ifdef SQLITE_ENABLE_API_ARMOR
  အကယ်၍( !sqlite3SafetyCheckOk(db) ){
    (void)SQLITE_MISUSE_BKPT;
    ပြန်ပေး 0;
  }
#endif
  sqlite3_mutex_enter(db->mutex);
  pOld = db->pCommitArg;
  db->xCommitCallback = xCallback;
  db->pCommitArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  ပြန်ပေး pOld;
}

/*
** Register a callback to be invoked each time a row is updated,
** inserted or deleted using this database connection.
*/
void *sqlite3_update_hook(
  sqlite3 *db,              /* Attach the hook to this database */
  void (*xCallback)(void*,ကိန်းပြည့်,var_na const *,var_na const *,sqlite_int64),
  void *pArg                /* Argument to the function */
){
  void *pRet;

#ifdef SQLITE_ENABLE_API_ARMOR
  အကယ်၍( !sqlite3SafetyCheckOk(db) ){
    (void)SQLITE_MISUSE_BKPT;
    ပြန်ပေး 0;
  }
#endif
  sqlite3_mutex_enter(db->mutex);
  pRet = db->pUpdateArg;
  db->xUpdateCallback = xCallback;
  db->pUpdateArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  ပြန်ပေး pRet;
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
  အကယ်၍( !sqlite3SafetyCheckOk(db) ){
    (void)SQLITE_MISUSE_BKPT;
    ပြန်ပေး 0;
  }
#endif
  sqlite3_mutex_enter(db->mutex);
  pRet = db->pRollbackArg;
  db->xRollbackCallback = xCallback;
  db->pRollbackArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  ပြန်ပေး pRet;
}

#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
/*
** Register a callback to be invoked each time a row is updated,
** inserted or deleted using this database connection.
*/
void *sqlite3_preupdate_hook(
  sqlite3 *db,              /* Attach the hook to this database */
  void(*xCallback)(         /* Callback function */
    void*,sqlite3*,ကိန်းပြည့်,var_na const*,var_na const*,sqlite3_int64,sqlite3_int64),
  void *pArg                /* First callback argument */
){
  void *pRet;

#ifdef SQLITE_ENABLE_API_ARMOR
  အကယ်၍( db==0 ){
    ပြန်ပေး 0;
  }
#endif
  sqlite3_mutex_enter(db->mutex);
  pRet = db->pPreUpdateArg;
  db->xPreUpdateCallback = xCallback;
  db->pPreUpdateArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  ပြန်ပေး pRet;
}
#endif /* SQLITE_ENABLE_PREUPDATE_HOOK */

/*
** Register a function to be invoked prior to each autovacuum that
** determines the number of pages to vacuum.
*/
ကိန်းပြည့် sqlite3_autovacuum_pages(
  sqlite3 *db,                 /* Attach the hook to this database */
  unsigned ကိန်းပြည့် (*xCallback)(void*,const var_na*,u32,u32,u32),
  void *pArg,                  /* Argument to the function */
  void (*xDestructor)(void*)   /* Destructor for pArg */
){
#ifdef SQLITE_ENABLE_API_ARMOR
  အကယ်၍( !sqlite3SafetyCheckOk(db) ){
    အကယ်၍( xDestructor ) xDestructor(pArg);
    ပြန်ပေး SQLITE_MISUSE_BKPT;
  }
#endif
  sqlite3_mutex_enter(db->mutex);
  အကယ်၍( db->xAutovacDestr ){
    db->xAutovacDestr(db->pAutovacPagesArg);
  }
  db->xAutovacPages = xCallback;
  db->pAutovacPagesArg = pArg;
  db->xAutovacDestr = xDestructor;
  sqlite3_mutex_leave(db->mutex);
  ပြန်ပေး SQLITE_OK;
}


#ifndef SQLITE_OMIT_WAL
/*
** The sqlite3_wal_hook() callback registered by sqlite3_wal_autocheckpoint().
** Invoke sqlite3_wal_checkpoint if the number of frames in the log file
** is greater than sqlite3.pWalArg cast to an integer (the value configured by
** wal_autocheckpoint()).
*/
ကိန်းပြည့် sqlite3WalDefaultHook(
  void *pClientData,     /* Argument */
  sqlite3 *db,           /* Connection */
  const var_na *zDb,       /* Database */
  ကိန်းပြည့် nFrame             /* Size of WAL */
){
  အကယ်၍( nFrame>=SQLITE_PTR_TO_INT(pClientData) ){
    sqlite3BeginBenignMalloc();
    sqlite3_wal_checkpoint(db, zDb);
    sqlite3EndBenignMalloc();
  }
  ပြန်ပေး SQLITE_OK;
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
ကိန်းပြည့် sqlite3_wal_autocheckpoint(sqlite3 *db, ကိန်းပြည့် nFrame){
#ifdef SQLITE_OMIT_WAL
  UNUSED_PARAMETER(db);
  UNUSED_PARAMETER(nFrame);
#မဟုတ်ပါက
#ifdef SQLITE_ENABLE_API_ARMOR
  အကယ်၍( !sqlite3SafetyCheckOk(db) ) ပြန်ပေး SQLITE_MISUSE_BKPT;
#endif
  အကယ်၍( nFrame>0 ){
    sqlite3_wal_hook(db, sqlite3WalDefaultHook, SQLITE_INT_TO_PTR(nFrame));
  }မဟုတ်ပါက{
    sqlite3_wal_hook(db, 0, 0);
  }
#endif
  ပြန်ပေး SQLITE_OK;
}

/*
** Register a callback to be invoked each time a transaction is written
** into the write-ahead-log by this database connection.
*/
void *sqlite3_wal_hook(
  sqlite3 *db,                    /* Attach the hook to this db handle */
  ကိန်းပြည့်(*xCallback)(void *, sqlite3*, const var_na*, ကိန်းပြည့်),
  void *pArg                      /* First argument passed to xCallback() */
){
#ifndef SQLITE_OMIT_WAL
  void *pRet;
#ifdef SQLITE_ENABLE_API_ARMOR
  အကယ်၍( !sqlite3SafetyCheckOk(db) ){
    (void)SQLITE_MISUSE_BKPT;
    ပြန်ပေး 0;
  }
#endif
  sqlite3_mutex_enter(db->mutex);
  pRet = db->pWalArg;
  db->xWalCallback = xCallback;
  db->pWalArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  ပြန်ပေး pRet;
#မဟုတ်ပါက
  UNUSED_PARAMETER(db);
  UNUSED_PARAMETER(xCallback);
  UNUSED_PARAMETER(pArg);
  ပြန်ပေး 0;
#endif
}

/*
** Checkpoint database zDb.
*/
ကိန်းပြည့် sqlite3_wal_checkpoint_v2(
  sqlite3 *db,                    /* Database handle */
  const var_na *zDb,                /* Name of attached database (or NULL) */
  ကိန်းပြည့် eMode,                      /* SQLITE_CHECKPOINT_* value */
  ကိန်းပြည့် *pnLog,                     /* OUT: Size of WAL log in frames */
  ကိန်းပြည့် *pnCkpt                     /* OUT: Total number of frames checkpointed */
){
#ifdef SQLITE_OMIT_WAL
  UNUSED_PARAMETER(db);
  UNUSED_PARAMETER(zDb);
  UNUSED_PARAMETER(eMode);
  UNUSED_PARAMETER(pnLog);
  UNUSED_PARAMETER(pnCkpt);
  ပြန်ပေး SQLITE_OK;
#မဟုတ်ပါက
  ကိန်းပြည့် rc;                         /* Return code */
  ကိန်းပြည့် iDb;                        /* Schema to checkpoint */

#ifdef SQLITE_ENABLE_API_ARMOR
  အကယ်၍( !sqlite3SafetyCheckOk(db) ) ပြန်ပေး SQLITE_MISUSE_BKPT;
#endif

  /* Initialize the output variables to -1 in case an error occurs. */
  အကယ်၍( pnLog ) *pnLog = -1;
  အကယ်၍( pnCkpt ) *pnCkpt = -1;

  assert( SQLITE_CHECKPOINT_NOOP==-1 );
  assert( SQLITE_CHECKPOINT_PASSIVE==0 );
  assert( SQLITE_CHECKPOINT_FULL==1 );
  assert( SQLITE_CHECKPOINT_RESTART==2 );
  assert( SQLITE_CHECKPOINT_TRUNCATE==3 );
  အကယ်၍( eMode<SQLITE_CHECKPOINT_NOOP || eMode>SQLITE_CHECKPOINT_TRUNCATE ){
    /* EVIDENCE-OF: R-03996-12088 The M parameter must be a valid checkpoint
    ** mode: */
    ပြန်ပေး SQLITE_MISUSE_BKPT;
  }

  sqlite3_mutex_enter(db->mutex);
  အကယ်၍( zDb && zDb[0] ){
    iDb = sqlite3FindDbName(db, zDb);
  }မဟုတ်ပါက{
    iDb = SQLITE_MAX_DB;   /* This means process all schemas */
  }
  အကယ်၍( iDb<0 ){
    rc = SQLITE_ERROR;
    sqlite3ErrorWithMsg(db, SQLITE_ERROR, "unknown database: %s", zDb);
  }မဟုတ်ပါက{
    db->busyHandler.nBusy = 0;
    rc = sqlite3Checkpoint(db, iDb, eMode, pnLog, pnCkpt);
    sqlite3Error(db, rc);
  }
  rc = sqlite3ApiExit(db, rc);

  /* If there are no active statements, clear the interrupt flag at this
  ** point.  */
  အကယ်၍( db->nVdbeActive==0 ){
    AtomicStore(&db->u1.isInterrupted, 0);
  }

  sqlite3_mutex_leave(db->mutex);
  ပြန်ပေး rc;
#endif
}


/*
** Checkpoint database zDb. If zDb is NULL, or if the buffer zDb points
** to contains a zero-length string, all attached databases are
** checkpointed.
*/
ကိန်းပြည့် sqlite3_wal_checkpoint(sqlite3 *db, const var_na *zDb){
  /* EVIDENCE-OF: R-41613-20553 The sqlite3_wal_checkpoint(D,X) is equivalent to
  ** sqlite3_wal_checkpoint_v2(D,X,SQLITE_CHECKPOINT_PASSIVE,0,0). */
  ပြန်ပေး sqlite3_wal_checkpoint_v2(db,zDb,SQLITE_CHECKPOINT_PASSIVE,0,0);
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
ကိန်းပြည့် sqlite3Checkpoint(sqlite3 *db, ကိန်းပြည့် iDb, ကိန်းပြည့် eMode, ကိန်းပြည့် *pnLog, ကိန်းပြည့် *pnCkpt){
  ကိန်းပြည့် rc = SQLITE_OK;             /* Return code */
  ကိန်းပြည့် i;                          /* Used to iterate through attached dbs */
  ကိန်းပြည့် bBusy = 0;                  /* True if SQLITE_BUSY has been encountered */

  assert( sqlite3_mutex_held(db->mutex) );
  assert( !pnLog || *pnLog==-1 );
  assert( !pnCkpt || *pnCkpt==-1 );
  testcase( iDb==SQLITE_MAX_ATTACHED ); /* See forum post a006d86f72 */
  testcase( iDb==SQLITE_MAX_DB );

  အတွက်(i=0; i<db->nDb && rc==SQLITE_OK; i++){
    အကယ်၍( i==iDb || iDb==SQLITE_MAX_DB ){
      rc = sqlite3BtreeCheckpoint(db->aDb[i].pBt, eMode, pnLog, pnCkpt);
      pnLog = 0;
      pnCkpt = 0;
      အကယ်၍( rc==SQLITE_BUSY ){
        bBusy = 1;
        rc = SQLITE_OK;
      }
    }
  }

  ပြန်ပေး (rc==SQLITE_OK && bBusy) ? SQLITE_BUSY : rc;
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
ကိန်းပြည့် sqlite3TempInMemory(const sqlite3 *db){
#အကယ်၍ SQLITE_TEMP_STORE==1
  ပြန်ပေး ( db->temp_store==2 );
#endif
#အကယ်၍ SQLITE_TEMP_STORE==2
  ပြန်ပေး ( db->temp_store!=1 );
#endif
#အကယ်၍ SQLITE_TEMP_STORE==3
  UNUSED_PARAMETER(db);
  ပြန်ပေး 1;
#endif
#အကယ်၍ SQLITE_TEMP_STORE<1 || SQLITE_TEMP_STORE>3
  UNUSED_PARAMETER(db);
  ပြန်ပေး 0;
#endif
}

/*
** Return UTF-8 encoded English language explanation of the most recent
** error.
*/
const var_na *sqlite3_errmsg(sqlite3 *db){
  const var_na *z;
  အကယ်၍( !db ){
    ပြန်ပေး sqlite3ErrStr(SQLITE_NOMEM_BKPT);
  }
  အကယ်၍( !sqlite3SafetyCheckSickOrOk(db) ){
    ပြန်ပေး sqlite3ErrStr(SQLITE_MISUSE_BKPT);
  }
  sqlite3_mutex_enter(db->mutex);
  အကယ်၍( db->mallocFailed ){
    z = sqlite3ErrStr(SQLITE_NOMEM_BKPT);
  }မဟုတ်ပါက{
    testcase( db->pErr==0 );
    z = db->errCode ? (var_na*)sqlite3_value_text(db->pErr) : 0;
    assert( !db->mallocFailed );
    အကယ်၍( z==0 ){
      z = sqlite3ErrStr(db->errCode);
    }
  }
  sqlite3_mutex_leave(db->mutex);
  ပြန်ပေး z;
}

/*
** Set the error code and error message associated with the database handle.
**
** This routine is intended to be called by outside extensions (ex: the
** Session extension). Internal logic should invoke sqlite3Error() or
** sqlite3ErrorWithMsg() directly.
*/
ကိန်းပြည့် sqlite3_set_errmsg(sqlite3 *db, ကိန်းပြည့် errcode, const var_na *zMsg){
  ကိန်းပြည့် rc = SQLITE_OK;
  အကယ်၍( !sqlite3SafetyCheckOk(db) ){
    ပြန်ပေး SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(db->mutex);
  အကယ်၍( zMsg ){
    sqlite3ErrorWithMsg(db, errcode, "%s", zMsg);
  }မဟုတ်ပါက{
    sqlite3Error(db, errcode);
  }
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  ပြန်ပေး rc;
}

/*
** Return the byte offset of the most recent error
*/
ကိန်းပြည့် sqlite3_error_offset(sqlite3 *db){
  ကိန်းပြည့် iOffset = -1;
  အကယ်၍( db && sqlite3SafetyCheckSickOrOk(db) ){
    sqlite3_mutex_enter(db->mutex);
    အကယ်၍( db->errCode ){
      iOffset = db->errByteOffset;
    }
    sqlite3_mutex_leave(db->mutex);
  }
  ပြန်ပေး iOffset;
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
  အကယ်၍( !db ){
    ပြန်ပေး (void *)outOfMem;
  }
  အကယ်၍( !sqlite3SafetyCheckSickOrOk(db) ){
    ပြန်ပေး (void *)misuse;
  }
  sqlite3_mutex_enter(db->mutex);
  အကယ်၍( db->mallocFailed ){
    z = (void *)outOfMem;
  }မဟုတ်ပါက{
    z = sqlite3_value_text16(db->pErr);
    အကယ်၍( z==0 ){
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
  ပြန်ပေး z;
}
#endif /* SQLITE_OMIT_UTF16 */

/*
** Return the most recent error code generated by an SQLite routine. If NULL is
** passed to this function, we assume a malloc() failed during sqlite3_open().
*/
ကိန်းပြည့် sqlite3_errcode(sqlite3 *db){
  ကိန်းပြည့် iRet;
  အကယ်၍( !db ) ပြန်ပေး SQLITE_NOMEM_BKPT;
  အကယ်၍( !sqlite3SafetyCheckSickOrOk(db) ){
    ပြန်ပေး SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(db->mutex);
  အကယ်၍( db->mallocFailed ){
    iRet = SQLITE_NOMEM_BKPT;
  }မဟုတ်ပါက{
    iRet = db->errCode & db->errMask;
  }
  sqlite3_mutex_leave(db->mutex);
  ပြန်ပေး iRet;
}
ကိန်းပြည့် sqlite3_extended_errcode(sqlite3 *db){
  ကိန်းပြည့် iRet;
  အကယ်၍( !db ) ပြန်ပေး SQLITE_NOMEM_BKPT;
  အကယ်၍( !sqlite3SafetyCheckSickOrOk(db) ){
    ပြန်ပေး SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(db->mutex);
  အကယ်၍( db->mallocFailed ){
    iRet = SQLITE_NOMEM_BKPT;
  }မဟုတ်ပါက{
    iRet = db->errCode;
  }
  sqlite3_mutex_leave(db->mutex);
  ပြန်ပေး iRet;
}
ကိန်းပြည့် sqlite3_system_errno(sqlite3 *db){
  ကိန်းပြည့် iRet = 0;
  အကယ်၍( db ){
    sqlite3_mutex_enter(db->mutex);
    iRet = db->iSysErrno;
    sqlite3_mutex_leave(db->mutex);
  }
  ပြန်ပေး iRet;
} 

/*
** Return a string that describes the kind of error specified in the
** argument.  For now, this simply calls the internal sqlite3ErrStr()
** function.
*/
const var_na *sqlite3_errstr(ကိန်းပြည့် rc){
  ပြန်ပေး sqlite3ErrStr(rc);
}

/*
** Create a new collating function for database "db".  The name is zName
** and the encoding is enc.
*/
static ကိန်းပြည့် createCollation(
  sqlite3* db,
  const var_na *zName,
  u8 enc,
  void* pCtx,
  ကိန်းပြည့်(*xCompare)(void*,ကိန်းပြည့်,const void*,ကိန်းပြည့်,const void*),
  void(*xDel)(void*)
){
  CollSeq *pColl;
  ကိန်းပြည့် enc2;
 
  assert( sqlite3_mutex_held(db->mutex) );

  /* If SQLITE_UTF16 is specified as the encoding type, transform this
  ** to one of SQLITE_UTF16LE or SQLITE_UTF16BE using the
  ** SQLITE_UTF16NATIVE macro. SQLITE_UTF16 is not used internally.
  */
  enc2 = enc;
  testcase( enc2==SQLITE_UTF16 );
  testcase( enc2==SQLITE_UTF16_ALIGNED );
  အကယ်၍( enc2==SQLITE_UTF16 || enc2==SQLITE_UTF16_ALIGNED ){
    enc2 = SQLITE_UTF16NATIVE;
  }
  အကယ်၍( enc2<SQLITE_UTF8 || enc2>SQLITE_UTF16BE ){
    ပြန်ပေး SQLITE_MISUSE_BKPT;
  }

  /* Check if this call is removing or replacing an existing collation
  ** sequence. If so, and there are active VMs, return busy. If there
  ** are no active VMs, invalidate any pre-compiled statements.
  */
  pColl = sqlite3FindCollSeq(db, (u8)enc2, zName, 0);
  အကယ်၍( pColl && pColl->xCmp ){
    အကယ်၍( db->nVdbeActive ){
      sqlite3ErrorWithMsg(db, SQLITE_BUSY,
        "unable to delete/modify collation sequence due to active statements");
      ပြန်ပေး SQLITE_BUSY;
    }
    sqlite3ExpirePreparedStatements(db, 0);

    /* If collation sequence pColl was created directly by a call to
    ** sqlite3_create_collation, and not generated by synthCollSeq(),
    ** then any copies made by synthCollSeq() need to be invalidated.
    ** Also, collation destructor - CollSeq.xDel() - function may need
    ** to be called.
    */
    အကယ်၍( (pColl->enc & ~SQLITE_UTF16_ALIGNED)==enc2 ){
      CollSeq *aColl = sqlite3HashFind(&db->aCollSeq, zName);
      ကိန်းပြည့် j;
      အတွက်(j=0; j<3; j++){
        CollSeq *p = &aColl[j];
        အကယ်၍( p->enc==pColl->enc ){
          အကယ်၍( p->xDel ){
            p->xDel(p->pUser);
          }
          p->xCmp = 0;
        }
      }
    }
  }

  pColl = sqlite3FindCollSeq(db, (u8)enc2, zName, 1);
  အကယ်၍( pColl==0 ) ပြန်ပေး SQLITE_NOMEM_BKPT;
  pColl->xCmp = xCompare;
  pColl->pUser = pCtx;
  pColl->xDel = xDel;
  pColl->enc = (u8)(enc2 | (enc & SQLITE_UTF16_ALIGNED));
  sqlite3Error(db, SQLITE_OK);
  ပြန်ပေး SQLITE_OK;
}


/*
** This array defines hard upper bounds on limit values.  The
** initializer must be kept in sync with the SQLITE_LIMIT_*
** #defines in sqlite3.h.
*/
static const ကိန်းပြည့် aHardLimit[] = {
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
#အကယ်၍ SQLITE_MAX_LENGTH<100
# error SQLITE_MAX_LENGTH must be at least 100
#endif
#အကယ်၍ SQLITE_MAX_SQL_LENGTH<100
# error SQLITE_MAX_SQL_LENGTH must be at least 100
#endif
#အကယ်၍ SQLITE_MAX_SQL_LENGTH>SQLITE_MAX_LENGTH
# error SQLITE_MAX_SQL_LENGTH must not be greater than SQLITE_MAX_LENGTH
#endif
#အကယ်၍ SQLITE_MAX_SQL_LENGTH>2147482624     /* 1024 less than 2^31 */
# error SQLITE_MAX_SQL_LENGTH must not be greater than 2147482624
#endif
#အကယ်၍ SQLITE_MAX_COMPOUND_SELECT<2
# error SQLITE_MAX_COMPOUND_SELECT must be at least 2
#endif
#အကယ်၍ SQLITE_MAX_VDBE_OP<40
# error SQLITE_MAX_VDBE_OP must be at least 40
#endif
#အကယ်၍ SQLITE_MAX_FUNCTION_ARG<0 || SQLITE_MAX_FUNCTION_ARG>32767
# error SQLITE_MAX_FUNCTION_ARG must be between 0 and 32767
#endif
#အကယ်၍ SQLITE_MAX_ATTACHED<0 || SQLITE_MAX_ATTACHED>125
# error SQLITE_MAX_ATTACHED must be between 0 and 125
#endif
#အကယ်၍ SQLITE_MAX_LIKE_PATTERN_LENGTH<1
# error SQLITE_MAX_LIKE_PATTERN_LENGTH must be at least 1
#endif
#အကယ်၍ SQLITE_MAX_COLUMN>32767
# error SQLITE_MAX_COLUMN must not exceed 32767
#endif
#အကယ်၍ SQLITE_MAX_TRIGGER_DEPTH<1
# error SQLITE_MAX_TRIGGER_DEPTH must be at least 1
#endif
#အကယ်၍ SQLITE_MAX_WORKER_THREADS<0 || SQLITE_MAX_WORKER_THREADS>50
# error SQLITE_MAX_WORKER_THREADS must be between 0 and 50
#endif
#အကယ်၍ SQLITE_MAX_TRIGGER_STEPS<0 || SQLITE_MAX_TRIGGER_STEPS>65535
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
ကိန်းပြည့် sqlite3_limit(sqlite3 *db, ကိန်းပြည့် limitId, ကိန်းပြည့် newLimit){
  ကိန်းပြည့် oldLimit;

#ifdef SQLITE_ENABLE_API_ARMOR
  အကယ်၍( !sqlite3SafetyCheckOk(db) ){
    (void)SQLITE_MISUSE_BKPT;
    ပြန်ပေး -1;
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


  အကယ်၍( limitId<0 || limitId>=SQLITE_N_LIMIT ){
    ပြန်ပေး -1;
  }
  sqlite3_mutex_enter(db->mutex);
  oldLimit = db->aLimit[limitId];
  အကယ်၍( newLimit>=0 ){                   /* IMP: R-52476-28732 */
    အကယ်၍( newLimit>aHardLimit[limitId] ){
      newLimit = aHardLimit[limitId];  /* IMP: R-51463-25634 */
    }မဟုတ်ပါက အကယ်၍( newLimit<SQLITE_MIN_LENGTH && limitId==SQLITE_LIMIT_LENGTH ){
      newLimit = SQLITE_MIN_LENGTH;
    }
    db->aLimit[limitId] = newLimit;
  }
  sqlite3_mutex_leave(db->mutex);
  ပြန်ပေး oldLimit;                     /* IMP: R-53341-35419 */
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
ကိန်းပြည့် sqlite3ParseUri(
  const var_na *zDefaultVfs,        /* VFS to use if no "vfs=xxx" query option */
  const var_na *zUri,               /* Nul-terminated URI to parse */
  unsigned ကိန်းပြည့် *pFlags,           /* IN/OUT: SQLITE_OPEN_XXX flags */
  sqlite3_vfs **ppVfs,            /* OUT: VFS to use */
  var_na **pzFile,                  /* OUT: Filename component of URI */
  var_na **pzErrMsg                 /* OUT: Error message (if rc!=SQLITE_OK) */
){
  ကိန်းပြည့် rc = SQLITE_OK;
  unsigned ကိန်းပြည့် flags = *pFlags;
  const var_na *zVfs = zDefaultVfs;
  var_na *zFile;
  var_na c;
  i64 nUri = strlen(zUri);

  assert( *pzErrMsg==0 );

  အကယ်၍( ((flags & SQLITE_OPEN_URI)                     /* IMP: R-48725-32206 */
       || AtomicLoad(&sqlite3GlobalConfig.bOpenUri)) /* IMP: R-51689-46548 */
   && nUri>=5 && memcmp(zUri, "file:", 5)==0         /* IMP: R-57884-37496 */
  ){
    var_na *zOpt;
    ကိန်းပြည့် eState;                   /* Parser state when parsing URI */
    i64 iIn;                      /* Input character index */
    i64 iOut = 0;                 /* Output character index */
    u64 nByte = nUri+8;           /* Bytes of space to allocate */

    /* Make sure the SQLITE_OPEN_URI flag is set to indicate to the VFS xOpen
    ** method that there may be extra parameters following the file-name.  */
    flags |= SQLITE_OPEN_URI;

    အတွက်(iIn=0; iIn<nUri; iIn++) nByte += (zUri[iIn]=='&');
    zFile = sqlite3_malloc64(nByte);
    အကယ်၍( !zFile ) ပြန်ပေး SQLITE_NOMEM_BKPT;

    memset(zFile, 0, 4);  /* 4-byte of 0x00 is the start of DB name marker */
    zFile += 4;

    iIn = 5;
#ifdef SQLITE_ALLOW_URI_AUTHORITY
    အကယ်၍( strncmp(zUri+5, "///", 3)==0 ){
      iIn = 7;
      /* The following condition causes URIs with five leading / characters
      ** like file://///host/path to be converted into UNCs like //host/path.
      ** The correct URI for that UNC has only two or four leading / characters
      ** file://host/path or file:////host/path.  But 5 leading slashes is a
      ** common error, we are told, so we handle it as a special case. */
      အကယ်၍( strncmp(zUri+7, "///", 3)==0 ){ iIn++; }
    }မဟုတ်ပါက အကယ်၍( strncmp(zUri+5, "//localhost/", 12)==0 ){
      iIn = 16;
    }
#မဟုတ်ပါက
    /* Discard the scheme and authority segments of the URI. */
    အကယ်၍( zUri[5]=='/' && zUri[6]=='/' ){
      iIn = 7;
      နေစဉ်( zUri[iIn] && zUri[iIn]!='/' ) iIn++;
      အကယ်၍( iIn!=7 && (iIn!=16 || memcmp("localhost", &zUri[7], 9)) ){
        *pzErrMsg = sqlite3_mprintf("invalid uri authority: %.*s",
            (ကိန်းပြည့်)(iIn-7), &zUri[7]);
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
    နေစဉ်( (c = zUri[iIn])!=0 && c!='#' ){
      iIn++;
      အကယ်၍( c=='%'
       && sqlite3Isxdigit(zUri[iIn])
       && sqlite3Isxdigit(zUri[iIn+1])
      ){
        ကိန်းပြည့် octet = (sqlite3HexToInt(zUri[iIn++]) << 4);
        octet += sqlite3HexToInt(zUri[iIn++]);

        assert( octet>=0 && octet<256 );
        အကယ်၍( octet==0 ){
#ifndef SQLITE_ENABLE_URI_00_ERROR
          /* This branch is taken when "%00" appears within the URI. In this
          ** case we ignore all text in the remainder of the path, name or
          ** value currently being parsed. So ignore the current character
          ** and skip to the next "?", "=" or "&", as appropriate. */
          နေစဉ်( (c = zUri[iIn])!=0 && c!='#'
              && (eState!=0 || c!='?')
              && (eState!=1 || (c!='=' && c!='&'))
              && (eState!=2 || c!='&')
          ){
            iIn++;
          }
          continue;
#မဟုတ်ပါက
          /* If ENABLE_URI_00_ERROR is defined, "%00" in a URI is an error. */
          *pzErrMsg = sqlite3_mprintf("unexpected %%00 in uri");
          rc = SQLITE_ERROR;
          goto parse_uri_out;
#endif
        }
        c = octet;
      }မဟုတ်ပါက အကယ်၍( eState==1 && (c=='&' || c=='=') ){
        အကယ်၍( zFile[iOut-1]==0 ){
          /* An empty option name. Ignore this option altogether. */
          နေစဉ်( zUri[iIn] && zUri[iIn]!='#' && zUri[iIn-1]!='&' ) iIn++;
          continue;
        }
        အကယ်၍( c=='&' ){
          zFile[iOut++] = '\0';
        }မဟုတ်ပါက{
          eState = 2;
        }
        c = 0;
      }မဟုတ်ပါက အကယ်၍( (eState==0 && c=='?') || (eState==2 && c=='&') ){
        c = 0;
        eState = 1;
      }
      zFile[iOut++] = c;
    }
    အကယ်၍( eState==1 ) zFile[iOut++] = '\0';
    memset(zFile+iOut, 0, 4); /* end-of-options + empty journal filenames */

    /* Check if there were any options specified that should be interpreted
    ** here. Options that are interpreted here include "vfs" and those that
    ** correspond to flags that may be passed to the sqlite3_open_v2()
    ** method. */
    zOpt = &zFile[strlen(zFile)+1];
    နေစဉ်( zOpt[0] ){
      i64 nOpt = strlen(zOpt);
      var_na *zVal = &zOpt[nOpt+1];
      i64 nVal = strlen(zVal);

      အကယ်၍( nOpt==3 && memcmp("vfs", zOpt, 3)==0 ){
        zVfs = zVal;
      }မဟုတ်ပါက{
        sa_nracanaa OpenMode {
          const var_na *z;
          ကိန်းပြည့် mode;
        } *aMode = 0;
        var_na *zModeType = 0;
        ကိန်းပြည့် mask = 0;
        ကိန်းပြည့် limit = 0;

        အကယ်၍( nOpt==5 && memcmp("cache", zOpt, 5)==0 ){
          static sa_nracanaa OpenMode aCacheMode[] = {
            { "shared",  SQLITE_OPEN_SHAREDCACHE },
            { "private", SQLITE_OPEN_PRIVATECACHE },
            { 0, 0 }
          };

          mask = SQLITE_OPEN_SHAREDCACHE|SQLITE_OPEN_PRIVATECACHE;
          aMode = aCacheMode;
          limit = mask;
          zModeType = "cache";
        }
        အကယ်၍( nOpt==4 && memcmp("mode", zOpt, 4)==0 ){
          static sa_nracanaa OpenMode aOpenMode[] = {
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

        အကယ်၍( aMode ){
          ကိန်းပြည့် i;
          ကိန်းပြည့် mode = 0;
          အတွက်(i=0; aMode[i].z; i++){
            const var_na *z = aMode[i].z;
            အကယ်၍( nVal==(i64)strlen(z) && 0==memcmp(zVal, z, nVal) ){
              mode = aMode[i].mode;
              bha_nga;
            }
          }
          အကယ်၍( mode==0 ){
            *pzErrMsg = sqlite3_mprintf("no such %s mode: %s", zModeType, zVal);
            rc = SQLITE_ERROR;
            goto parse_uri_out;
          }
          အကယ်၍( (mode & ~SQLITE_OPEN_MEMORY)>limit ){
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

  }မဟုတ်ပါက{
    zFile = sqlite3_malloc64(nUri+8);
    အကယ်၍( !zFile ) ပြန်ပေး SQLITE_NOMEM_BKPT;
    memset(zFile, 0, 4);
    zFile += 4;
    အကယ်၍( nUri ){
      memcpy(zFile, zUri, nUri);
    }
    memset(zFile+nUri, 0, 4);
    flags &= ~SQLITE_OPEN_URI;
  }

  *ppVfs = sqlite3_vfs_find(zVfs);
  အကယ်၍( *ppVfs==0 ){
    *pzErrMsg = sqlite3_mprintf("no such vfs: %s", zVfs);
    rc = SQLITE_ERROR;
  }
 parse_uri_out:
  အကယ်၍( rc!=SQLITE_OK ){
    sqlite3_free_filename(zFile);
    zFile = 0;
  }
  *pFlags = flags;
  *pzFile = zFile;
  ပြန်ပေး rc;
}

/*
** This routine does the core work of extracting URI parameters from a
** database filename for the sqlite3_uri_parameter() interface.
*/
static const var_na *uriParameter(const var_na *zFilename, const var_na *zParam){
  zFilename += sqlite3Strlen30(zFilename) + 1;
  နေစဉ်( ALWAYS(zFilename!=0) && zFilename[0] ){
    ကိန်းပြည့် x = strcmp(zFilename, zParam);
    zFilename += sqlite3Strlen30(zFilename) + 1;
    အကယ်၍( x==0 ) ပြန်ပေး zFilename;
    zFilename += sqlite3Strlen30(zFilename) + 1;
  }
  ပြန်ပေး 0;
}



/*
** This routine does the work of opening a database on behalf of
** sqlite3_open() and sqlite3_open16(). The database filename "zFilename" 
** is UTF-8 encoded.
*/
static ကိန်းပြည့် openDatabase(
  const var_na *zFilename, /* Database filename UTF-8 encoded */
  sqlite3 **ppDb,        /* OUT: Returned database handle */
  unsigned ကိန်းပြည့် flags,    /* Operational flags */
  const var_na *zVfs       /* Name of the VFS to use */
){
  sqlite3 *db;                    /* Store allocated handle here */
  ကိန်းပြည့် rc;                         /* Return code */
  ကိန်းပြည့် isThreadsafe;               /* True for threadsafe connections */
  var_na *zOpen = 0;                /* Filename argument to pass to BtreeOpen() */
  var_na *zErrMsg = 0;              /* Error message from sqlite3ParseUri() */
  ကိန်းပြည့် i;                          /* Loop counter */

#ifdef SQLITE_ENABLE_API_ARMOR
  အကယ်၍( ppDb==0 ) ပြန်ပေး SQLITE_MISUSE_BKPT;
#endif
  *ppDb = 0;
#ifndef SQLITE_OMIT_AUTOINIT
  rc = sqlite3_initialize();
  အကယ်၍( rc ) ပြန်ပေး rc;
#endif

  အကယ်၍( sqlite3GlobalConfig.bCoreMutex==0 ){
    isThreadsafe = 0;
  }မဟုတ်ပါက အကယ်၍( flags & SQLITE_OPEN_NOMUTEX ){
    isThreadsafe = 0;
  }မဟုတ်ပါက အကယ်၍( flags & SQLITE_OPEN_FULLMUTEX ){
    isThreadsafe = 1;
  }မဟုတ်ပါက{
    isThreadsafe = sqlite3GlobalConfig.bFullMutex;
  }

  အကယ်၍( flags & SQLITE_OPEN_PRIVATECACHE ){
    flags &= ~SQLITE_OPEN_SHAREDCACHE;
  }မဟုတ်ပါက အကယ်၍( sqlite3GlobalConfig.sharedCacheEnabled ){
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
  db = sqlite3MallocZero( parimaa_na(sqlite3) );
  အကယ်၍( db==0 ) goto opendb_out;
  အကယ်၍( isThreadsafe
#အကယ်၍ defined(SQLITE_THREAD_MISUSE_WARNINGS)
   || sqlite3GlobalConfig.bCoreMutex
#endif
  ){
    db->mutex = sqlite3MutexAlloc(SQLITE_MUTEX_RECURSIVE);
    အကယ်၍( db->mutex==0 ){
      sqlite3_free(db);
      db = 0;
      goto opendb_out;
    }
    အကယ်၍( isThreadsafe==0 ){
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

  assert( parimaa_na(db->aLimit)==parimaa_na(aHardLimit) );
  memcpy(db->aLimit, aHardLimit, parimaa_na(db->aLimit));
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
#အကယ်၍ !defined(SQLITE_TRUSTED_SCHEMA) || SQLITE_TRUSTED_SCHEMA+0!=0
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
#အကယ်၍ !defined(SQLITE_DQS)
# define SQLITE_DQS 3
#endif
#အကယ်၍ (SQLITE_DQS&1)==1
                 | SQLITE_DqsDML
#endif
#အကယ်၍ (SQLITE_DQS&2)==2
                 | SQLITE_DqsDDL
#endif

#အကယ်၍ !defined(SQLITE_DEFAULT_AUTOMATIC_INDEX) || SQLITE_DEFAULT_AUTOMATIC_INDEX
                 | SQLITE_AutoIndex
#endif
#အကယ်၍ SQLITE_DEFAULT_CKPTFULLFSYNC
                 | SQLITE_CkptFullFSync
#endif
#အကယ်၍ SQLITE_DEFAULT_FILE_FORMAT<4
                 | SQLITE_LegacyFileFmt
#endif
#ifdef SQLITE_ENABLE_LOAD_EXTENSION
                 | SQLITE_LoadExtension
#endif
#အကယ်၍ SQLITE_DEFAULT_RECURSIVE_TRIGGERS
                 | SQLITE_RecTriggers
#endif
#အကယ်၍ defined(SQLITE_DEFAULT_FOREIGN_KEYS) && SQLITE_DEFAULT_FOREIGN_KEYS
                 | SQLITE_ForeignKeys
#endif
#အကယ်၍ defined(SQLITE_REVERSE_UNORDERED_SELECTS)
                 | SQLITE_ReverseOrder
#endif
#အကယ်၍ defined(SQLITE_ENABLE_OVERSIZE_CELL_CHECK)
                 | SQLITE_CellSizeCk
#endif
#အကယ်၍ defined(SQLITE_ENABLE_FTS3_TOKENIZER)
                 | SQLITE_Fts3Tokenizer
#endif
#အကယ်၍ defined(SQLITE_ENABLE_QPSG)
                 | SQLITE_EnableQPSG
#endif
#အကယ်၍ defined(SQLITE_DEFAULT_DEFENSIVE)
                 | SQLITE_Defensive
#endif
#အကယ်၍ defined(SQLITE_DEFAULT_LEGACY_ALTER_TABLE)
                 | SQLITE_LegacyAlter
#endif
#အကယ်၍ defined(SQLITE_ENABLE_STMT_SCANSTATUS)
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
  အကယ်၍( db->mallocFailed ){
    goto opendb_out;
  }

#အကယ်၍ SQLITE_OS_UNIX && defined(SQLITE_OS_KV_OPTIONAL)
  /* Process magic filenames ":localStorage:" and ":sessionStorage:" */
  အကယ်၍( zFilename && zFilename[0]==':' ){
    အကယ်၍( strcmp(zFilename, ":localStorage:")==0 ){
      zFilename = "file:local?vfs=kvvfs";
      flags |= SQLITE_OPEN_URI;
    }မဟုတ်ပါက အကယ်၍( strcmp(zFilename, ":sessionStorage:")==0 ){
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
  အကယ်၍( ((1<<(flags&7)) & 0x46)==0 ){
    rc = SQLITE_MISUSE_BKPT;  /* IMP: R-18321-05872 */
  }မဟုတ်ပါက{
    အကယ်၍( zFilename==0 ) zFilename = ":memory:";
    rc = sqlite3ParseUri(zVfs, zFilename, &flags, &db->pVfs, &zOpen, &zErrMsg);
  }
  အကယ်၍( rc!=SQLITE_OK ){
    အကယ်၍( rc==SQLITE_NOMEM ) sqlite3OomFault(db);
    sqlite3ErrorWithMsg(db, rc, zErrMsg ? "%s" : 0, zErrMsg);
    sqlite3_free(zErrMsg);
    goto opendb_out;
  }
  assert( db->pVfs!=0 );
#အကယ်၍ SQLITE_OS_KV || defined(SQLITE_OS_KV_OPTIONAL)
  အကယ်၍( sqlite3_stricmp(db->pVfs->zName, "kvvfs")==0 ){
    db->temp_store = 2;
  }
#endif

  /* Open the backend database driver */
  rc = sqlite3BtreeOpen(db->pVfs, zOpen, db, &db->aDb[0].pBt, 0,
                        flags | SQLITE_OPEN_MAIN_DB);
  အကယ်၍( rc!=SQLITE_OK ){
    အကယ်၍( rc==SQLITE_IOERR_NOMEM ){
      rc = SQLITE_NOMEM_BKPT;
    }
    sqlite3Error(db, rc);
    goto opendb_out;
  }
  sqlite3BtreeEnter(db->aDb[0].pBt);
  db->aDb[0].pSchema = sqlite3SchemaGet(db, db->aDb[0].pBt);
  အကယ်၍( !db->mallocFailed ){
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
  အကယ်၍( db->mallocFailed ){
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
  အတွက်(i=0; rc==SQLITE_OK && i<ArraySize(sqlite3BuiltinExtensions); i++){
    rc = sqlite3BuiltinExtensions[i](db);
  }

  /* Load automatic extensions - extensions that have been registered
  ** using the sqlite3_automatic_extension() API.
  */
  အကယ်၍( rc==SQLITE_OK ){
    sqlite3AutoLoadExtensions(db);
    rc = sqlite3_errcode(db);
    အကယ်၍( rc!=SQLITE_OK ){
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

  အကယ်၍( rc ) sqlite3Error(db, rc);

  /* Enable the lookaside-malloc subsystem */
  setupLookaside(db, 0, sqlite3GlobalConfig.szLookaside,
                        sqlite3GlobalConfig.nLookaside);

  sqlite3_wal_autocheckpoint(db, SQLITE_DEFAULT_WAL_AUTOCHECKPOINT);

opendb_out:
  အကယ်၍( db ){
    assert( db->mutex!=0 || isThreadsafe==0
           || sqlite3GlobalConfig.bFullMutex==0 );
    sqlite3_mutex_leave(db->mutex);
  }
  rc = sqlite3_errcode(db);
  assert( db!=0 || (rc&0xff)==SQLITE_NOMEM );
  အကယ်၍( (rc&0xff)==SQLITE_NOMEM ){
    sqlite3_close(db);
    db = 0;
  }မဟုတ်ပါက အကယ်၍( rc!=SQLITE_OK ){
    db->eOpenState = SQLITE_STATE_SICK;
  }
  *ppDb = db;
#ifdef SQLITE_ENABLE_SQLLOG
  အကယ်၍( sqlite3GlobalConfig.xSqllog ){
    /* Opening a db handle. Fourth parameter is passed 0. */
    void *pArg = sqlite3GlobalConfig.pSqllogArg;
    sqlite3GlobalConfig.xSqllog(pArg, db, zFilename, 0);
  }
#endif
  sqlite3_free_filename(zOpen);
  ပြန်ပေး rc;
}


/*
** Open a new database handle.
*/
ကိန်းပြည့် sqlite3_open(
  const var_na *zFilename,
  sqlite3 **ppDb
){
  ပြန်ပေး openDatabase(zFilename, ppDb,
                      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, 0);
}
ကိန်းပြည့် sqlite3_open_v2(
  const var_na *filename,   /* Database filename (UTF-8) */
  sqlite3 **ppDb,         /* OUT: SQLite db handle */
  ကိန်းပြည့် flags,              /* Flags */
  const var_na *zVfs        /* Name of VFS module to use */
){
  ပြန်ပေး openDatabase(filename, ppDb, (unsigned ကိန်းပြည့်)flags, zVfs);
}

#ifndef SQLITE_OMIT_UTF16
/*
** Open a new database handle.
*/
ကိန်းပြည့် sqlite3_open16(
  const void *zFilename,
  sqlite3 **ppDb
){
  var_na const *zFilename8;   /* zFilename encoded in UTF-8 instead of UTF-16 */
  sqlite3_value *pVal;
  ကိန်းပြည့် rc;

#ifdef SQLITE_ENABLE_API_ARMOR
  အကယ်၍( ppDb==0 ) ပြန်ပေး SQLITE_MISUSE_BKPT;
#endif
  *ppDb = 0;
#ifndef SQLITE_OMIT_AUTOINIT
  rc = sqlite3_initialize();
  အကယ်၍( rc ) ပြန်ပေး rc;
#endif
  အကယ်၍( zFilename==0 ) zFilename = "\000\000";
  pVal = sqlite3ValueNew(0);
  sqlite3ValueSetStr(pVal, -1, zFilename, SQLITE_UTF16NATIVE, SQLITE_STATIC);
  zFilename8 = sqlite3ValueText(pVal, SQLITE_UTF8);
  အကယ်၍( zFilename8 ){
    rc = openDatabase(zFilename8, ppDb,
                      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, 0);
    assert( *ppDb || rc==SQLITE_NOMEM );
    အကယ်၍( rc==SQLITE_OK && !DbHasProperty(*ppDb, 0, DB_SchemaLoaded) ){
      SCHEMA_ENC(*ppDb) = ENC(*ppDb) = SQLITE_UTF16NATIVE;
    }
  }မဟုတ်ပါက{
    rc = SQLITE_NOMEM_BKPT;
  }
  sqlite3ValueFree(pVal);

  ပြန်ပေး rc & 0xff;
}
#endif /* SQLITE_OMIT_UTF16 */

/*
** Register a new collation sequence with the database handle db.
*/
ကိန်းပြည့် sqlite3_create_collation(
  sqlite3* db,
  const var_na *zName,
  ကိန်းပြည့် enc,
  void* pCtx,
  ကိန်းပြည့်(*xCompare)(void*,ကိန်းပြည့်,const void*,ကိန်းပြည့်,const void*)
){
  ပြန်ပေး sqlite3_create_collation_v2(db, zName, enc, pCtx, xCompare, 0);
}

/*
** Register a new collation sequence with the database handle db.
*/
ကိန်းပြည့် sqlite3_create_collation_v2(
  sqlite3* db,
  const var_na *zName,
  ကိန်းပြည့် enc,
  void* pCtx,
  ကိန်းပြည့်(*xCompare)(void*,ကိန်းပြည့်,const void*,ကိန်းပြည့်,const void*),
  void(*xDel)(void*)
){
  ကိန်းပြည့် rc;

#ifdef SQLITE_ENABLE_API_ARMOR
  အကယ်၍( !sqlite3SafetyCheckOk(db) || zName==0 ) ပြန်ပေး SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(db->mutex);
  assert( !db->mallocFailed );
  rc = createCollation(db, zName, (u8)enc, pCtx, xCompare, xDel);
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  ပြန်ပေး rc;
}

#ifndef SQLITE_OMIT_UTF16
/*
** Register a new collation sequence with the database handle db.
*/
ကိန်းပြည့် sqlite3_create_collation16(
  sqlite3* db,
  const void *zName,
  ကိန်းပြည့် enc,
  void* pCtx,
  ကိန်းပြည့်(*xCompare)(void*,ကိန်းပြည့်,const void*,ကိန်းပြည့်,const void*)
){
  ကိန်းပြည့် rc = SQLITE_OK;
  var_na *zName8;

#ifdef SQLITE_ENABLE_API_ARMOR
  အကယ်၍( !sqlite3SafetyCheckOk(db) || zName==0 ) ပြန်ပေး SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(db->mutex);
  assert( !db->mallocFailed );
  zName8 = sqlite3Utf16to8(db, zName, -1, SQLITE_UTF16NATIVE);
  အကယ်၍( zName8 ){
    rc = createCollation(db, zName8, (u8)enc, pCtx, xCompare, 0);
    sqlite3DbFree(db, zName8);
  }
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  ပြန်ပေး rc;
}
#endif /* SQLITE_OMIT_UTF16 */

/*
** Register a collation sequence factory callback with the database handle
** db. Replace any previously installed collation sequence factory.
*/
ကိန်းပြည့် sqlite3_collation_needed(
  sqlite3 *db,
  void *pCollNeededArg,
  void(*xCollNeeded)(void*,sqlite3*,ကိန်းပြည့် eTextRep,const var_na*)
){
#ifdef SQLITE_ENABLE_API_ARMOR
  အကယ်၍( !sqlite3SafetyCheckOk(db) ) ပြန်ပေး SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(db->mutex);
  db->xCollNeeded = xCollNeeded;
  db->xCollNeeded16 = 0;
  db->pCollNeededArg = pCollNeededArg;
  sqlite3_mutex_leave(db->mutex);
  ပြန်ပေး SQLITE_OK;
}

#ifndef SQLITE_OMIT_UTF16
/*
** Register a collation sequence factory callback with the database handle
** db. Replace any previously installed collation sequence factory.
*/
ကိန်းပြည့် sqlite3_collation_needed16(
  sqlite3 *db,
  void *pCollNeededArg,
  void(*xCollNeeded16)(void*,sqlite3*,ကိန်းပြည့် eTextRep,const void*)
){
#ifdef SQLITE_ENABLE_API_ARMOR
  အကယ်၍( !sqlite3SafetyCheckOk(db) ) ပြန်ပေး SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(db->mutex);
  db->xCollNeeded = 0;
  db->xCollNeeded16 = xCollNeeded16;
  db->pCollNeededArg = pCollNeededArg;
  sqlite3_mutex_leave(db->mutex);
  ပြန်ပေး SQLITE_OK;
}
#endif /* SQLITE_OMIT_UTF16 */

/*
** Find existing client data.
*/
void *sqlite3_get_clientdata(sqlite3 *db, const var_na *zName){
  DbClientData *p;
#ifdef SQLITE_ENABLE_API_ARMOR
  အကယ်၍( !zName || !sqlite3SafetyCheckOk(db) ){
    (void)SQLITE_MISUSE_BKPT;
    ပြန်ပေး 0;
  }
#endif
  sqlite3_mutex_enter(db->mutex);
  အတွက်(p=db->pDbData; p; p=p->pNext){
    အကယ်၍( strcmp(p->zName, zName)==0 ){
      void *pResult = p->pData;
      sqlite3_mutex_leave(db->mutex);
      ပြန်ပေး pResult;
    }
  }
  sqlite3_mutex_leave(db->mutex);
  ပြန်ပေး 0;
}

/*
** Add new client data to a database connection.
*/
ကိန်းပြည့် sqlite3_set_clientdata(
  sqlite3 *db,                   /* Attach client data to this connection */
  const var_na *zName,             /* Name of the client data */
  void *pData,                   /* The client data itself */
  void (*xDestructor)(void*)     /* Destructor */
){
  DbClientData *p, **pp;
  sqlite3_mutex_enter(db->mutex);
  pp = &db->pDbData;
  အတွက်(p=db->pDbData; p && strcmp(p->zName,zName); p=p->pNext){
    pp = &p->pNext;
  }
  အကယ်၍( p ){
    assert( p->pData!=0 );
    အကယ်၍( p->xDestructor ) p->xDestructor(p->pData);
    အကယ်၍( pData==0 ){
      *pp = p->pNext;
      sqlite3_free(p);
      sqlite3_mutex_leave(db->mutex);
      ပြန်ပေး SQLITE_OK;
    }
  }မဟုတ်ပါက အကယ်၍( pData==0 ){
    sqlite3_mutex_leave(db->mutex);
    ပြန်ပေး SQLITE_OK;
  }မဟုတ်ပါက{
    size_t n = strlen(zName);
    p = sqlite3_malloc64( SZ_DBCLIENTDATA(n+1) );
    အကယ်၍( p==0 ){
      အကယ်၍( xDestructor ) xDestructor(pData);
      sqlite3_mutex_leave(db->mutex);
      ပြန်ပေး SQLITE_NOMEM;
    }
    memcpy(p->zName, zName, n+1);
    p->pNext = db->pDbData;
    db->pDbData = p;
  }
  p->pData = pData;
  p->xDestructor = xDestructor;
  sqlite3_mutex_leave(db->mutex);
  ပြန်ပေး SQLITE_OK;
}


#ifndef SQLITE_OMIT_DEPRECATED
/*
** This function is now an anachronism. It used to be used to recover from a
** malloc() failure, but SQLite now does this automatically.
*/
ကိန်းပြည့် sqlite3_global_recover(void){
  ပြန်ပေး SQLITE_OK;
}
#endif

/*
** Test to see whether or not the database connection is in autocommit
** mode.  Return TRUE if it is and FALSE if not.  Autocommit mode is on
** by default.  Autocommit is disabled by a BEGIN statement and reenabled
** by the next COMMIT or ROLLBACK.
*/
ကိန်းပြည့် sqlite3_get_autocommit(sqlite3 *db){
  ကိန်းပြည့် iRet;
#ifdef SQLITE_ENABLE_API_ARMOR
  အကယ်၍( !sqlite3SafetyCheckOk(db) ){
    (void)SQLITE_MISUSE_BKPT;
    ပြန်ပေး 0;
  }
#endif
  sqlite3_mutex_enter(db->mutex);
  iRet = db->autoCommit;
  sqlite3_mutex_leave(db->mutex);
  ပြန်ပေး iRet;
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
ကိန်းပြည့် sqlite3ReportError(ကိန်းပြည့် iErr, ကိန်းပြည့် lineno, const var_na *zType){
  sqlite3_log(iErr, "%s at line %d of [%.10s]",
              zType, lineno, 20+sqlite3_sourceid());
  ပြန်ပေး iErr;
}
ကိန်းပြည့် sqlite3CorruptError(ကိန်းပြည့် lineno){
  testcase( sqlite3GlobalConfig.xLog!=0 );
  ပြန်ပေး sqlite3ReportError(SQLITE_CORRUPT, lineno, "database corruption");
}
ကိန်းပြည့် sqlite3MisuseError(ကိန်းပြည့် lineno){
  testcase( sqlite3GlobalConfig.xLog!=0 );
  ပြန်ပေး sqlite3ReportError(SQLITE_MISUSE, lineno, "misuse");
}
ကိန်းပြည့် sqlite3CantopenError(ကိန်းပြည့် lineno){
  testcase( sqlite3GlobalConfig.xLog!=0 );
  ပြန်ပေး sqlite3ReportError(SQLITE_CANTOPEN, lineno, "cannot open file");
}
#အကယ်၍ defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_CORRUPT_PGNO)
ကိန်းပြည့် sqlite3CorruptPgnoError(ကိန်းပြည့် lineno, Pgno pgno){
  var_na zMsg[100];
  sqlite3_snprintf(parimaa_na(zMsg), zMsg, "database corruption page %d", pgno);
  testcase( sqlite3GlobalConfig.xLog!=0 );
  ပြန်ပေး sqlite3ReportError(SQLITE_CORRUPT, lineno, zMsg);
}
#endif
#ifdef SQLITE_DEBUG
ကိန်းပြည့် sqlite3NomemError(ကိန်းပြည့် lineno){
  testcase( sqlite3GlobalConfig.xLog!=0 );
  ပြန်ပေး sqlite3ReportError(SQLITE_NOMEM, lineno, "OOM");
}
ကိန်းပြည့် sqlite3IoerrnomemError(ကိန်းပြည့် lineno){
  testcase( sqlite3GlobalConfig.xLog!=0 );
  ပြန်ပေး sqlite3ReportError(SQLITE_IOERR_NOMEM, lineno, "I/O OOM error");
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
ကိန်းပြည့် sqlite3_table_column_metadata(
  sqlite3 *db,                /* Connection handle */
  const var_na *zDbName,        /* Database name or NULL */
  const var_na *zTableName,     /* Table name */
  const var_na *zColumnName,    /* Column name */
  var_na const **pzDataType,    /* OUTPUT: Declared data type */
  var_na const **pzCollSeq,     /* OUTPUT: Collation sequence name */
  ကိန်းပြည့် *pNotNull,              /* OUTPUT: True if NOT NULL constraint exists */
  ကိန်းပြည့် *pPrimaryKey,           /* OUTPUT: True if column part of PK */
  ကိန်းပြည့် *pAutoinc               /* OUTPUT: True if column is auto-increment */
){
  ကိန်းပြည့် rc;
  var_na *zErrMsg = 0;
  Table *pTab = 0;
  Column *pCol = 0;
  ကိန်းပြည့် iCol = 0;
  var_na const *zDataType = 0;
  var_na const *zCollSeq = 0;
  ကိန်းပြည့် notnull = 0;
  ကိန်းပြည့် primarykey = 0;
  ကိန်းပြည့် autoinc = 0;


#ifdef SQLITE_ENABLE_API_ARMOR
  အကယ်၍( !sqlite3SafetyCheckOk(db) || zTableName==0 ){
    ပြန်ပေး SQLITE_MISUSE_BKPT;
  }
#endif

  /* Ensure the database schema has been loaded */
  sqlite3_mutex_enter(db->mutex);
  sqlite3BtreeEnterAll(db);
  rc = sqlite3Init(db, &zErrMsg);
  အကယ်၍( SQLITE_OK!=rc ){
    goto error_out;
  }

  /* Locate the table in question */
  pTab = sqlite3FindTable(db, zTableName, zDbName);
  အကယ်၍( !pTab || IsView(pTab) ){
    pTab = 0;
    goto error_out;
  }

  /* Find the column for which info is requested */
  အကယ်၍( zColumnName==0 ){
    /* Query for existence of table only */
  }မဟုတ်ပါက{
    iCol = sqlite3ColumnIndex(pTab, zColumnName);
    အကယ်၍( iCol>=0 ){
      pCol = &pTab->aCol[iCol];
    }မဟုတ်ပါက{
      အကယ်၍( HasRowid(pTab) && sqlite3IsRowid(zColumnName) ){
        iCol = pTab->iPKey;
        pCol = iCol>=0 ? &pTab->aCol[iCol] : 0;
      }မဟုတ်ပါက{
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
  အကယ်၍( pCol ){
    zDataType = sqlite3ColumnType(pCol,0);
    zCollSeq = sqlite3ColumnColl(pCol);
    notnull = pCol->notNull!=0;
    primarykey  = (pCol->colFlags & COLFLAG_PRIMKEY)!=0;
    autoinc = pTab->iPKey==iCol && (pTab->tabFlags & TF_Autoincrement)!=0;
  }မဟုတ်ပါက{
    zDataType = "INTEGER";
    primarykey = 1;
  }
  အကယ်၍( !zCollSeq ){
    zCollSeq = sqlite3StrBINARY;
  }

error_out:
  sqlite3BtreeLeaveAll(db);

  /* Whether the function call succeeded or failed, set the output parameters
  ** to whatever their local counterparts contain. If an error did occur,
  ** this has the effect of zeroing all output parameters.
  */
  အကယ်၍( pzDataType ) *pzDataType = zDataType;
  အကယ်၍( pzCollSeq ) *pzCollSeq = zCollSeq;
  အကယ်၍( pNotNull ) *pNotNull = notnull;
  အကယ်၍( pPrimaryKey ) *pPrimaryKey = primarykey;
  အကယ်၍( pAutoinc ) *pAutoinc = autoinc;

  အကယ်၍( SQLITE_OK==rc && !pTab ){
    sqlite3DbFree(db, zErrMsg);
    zErrMsg = sqlite3MPrintf(db, "no such table column: %s.%s", zTableName,
        zColumnName);
    rc = SQLITE_ERROR;
  }
  sqlite3ErrorWithMsg(db, rc, (zErrMsg?"%s":0), zErrMsg);
  sqlite3DbFree(db, zErrMsg);
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  ပြန်ပေး rc;
}

/*
** Sleep for a little while.  Return the amount of time slept.
*/
ကိန်းပြည့် sqlite3_sleep(ကိန်းပြည့် ms){
  sqlite3_vfs *pVfs;
  ကိန်းပြည့် rc;
  pVfs = sqlite3_vfs_find(0);
  အကယ်၍( pVfs==0 ) ပြန်ပေး 0;

  /* This function works in milliseconds, but the underlying OsSleep()
  ** API uses microseconds. Hence the 1000's.
  */
  rc = (sqlite3OsSleep(pVfs, ms<0 ? 0 : 1000*ms)/1000);
  ပြန်ပေး rc;
}

/*
** Enable or disable the extended result codes.
*/
ကိန်းပြည့် sqlite3_extended_result_codes(sqlite3 *db, ကိန်းပြည့် onoff){
#ifdef SQLITE_ENABLE_API_ARMOR
  အကယ်၍( !sqlite3SafetyCheckOk(db) ) ပြန်ပေး SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(db->mutex);
  db->errMask = onoff ? 0xffffffff : 0xff;
  sqlite3_mutex_leave(db->mutex);
  ပြန်ပေး SQLITE_OK;
}

/*
** Invoke the xFileControl method on a particular database.
*/
ကိန်းပြည့် sqlite3_file_control(sqlite3 *db, const var_na *zDbName, ကိန်းပြည့် op, void *pArg){
  ကိန်းပြည့် rc = SQLITE_ERROR;
  Btree *pBtree;

#ifdef SQLITE_ENABLE_API_ARMOR
  အကယ်၍( !sqlite3SafetyCheckOk(db) ) ပြန်ပေး SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(db->mutex);
  pBtree = sqlite3DbNameToBtree(db, zDbName);
  အကယ်၍( pBtree ){
    Pager *pPager;
    sqlite3_file *fd;
    sqlite3BtreeEnter(pBtree);
    pPager = sqlite3BtreePager(pBtree);
    assert( pPager!=0 );
    fd = sqlite3PagerFile(pPager);
    assert( fd!=0 );
    အကယ်၍( op==SQLITE_FCNTL_FILE_POINTER ){
      *(sqlite3_file**)pArg = fd;
      rc = SQLITE_OK;
    }မဟုတ်ပါက အကယ်၍( op==SQLITE_FCNTL_VFS_POINTER ){
      *(sqlite3_vfs**)pArg = sqlite3PagerVfs(pPager);
      rc = SQLITE_OK;
    }မဟုတ်ပါက အကယ်၍( op==SQLITE_FCNTL_JOURNAL_POINTER ){
      *(sqlite3_file**)pArg = sqlite3PagerJrnlFile(pPager);
      rc = SQLITE_OK;
    }မဟုတ်ပါက အကယ်၍( op==SQLITE_FCNTL_DATA_VERSION ){
      *(unsigned ကိန်းပြည့်*)pArg = sqlite3PagerDataVersion(pPager);
      rc = SQLITE_OK;
    }မဟုတ်ပါက အကယ်၍( op==SQLITE_FCNTL_RESERVE_BYTES ){
      ကိန်းပြည့် iNew = *(ကိန်းပြည့်*)pArg;
      *(ကိန်းပြည့်*)pArg = sqlite3BtreeGetRequestedReserve(pBtree);
      အကယ်၍( iNew>=0 && iNew<=255 ){
        sqlite3BtreeSetPageSize(pBtree, 0, iNew, 0);
      }
      rc = SQLITE_OK;
    }မဟုတ်ပါက အကယ်၍( op==SQLITE_FCNTL_RESET_CACHE ){
      sqlite3BtreeClearCache(pBtree);
      rc = SQLITE_OK;
    }မဟုတ်ပါက{
      ကိန်းပြည့် nSave = db->busyHandler.nBusy;
      rc = sqlite3OsFileControl(fd, op, pArg);
      db->busyHandler.nBusy = nSave;
    }
    sqlite3BtreeLeave(pBtree);
  }
  sqlite3_mutex_leave(db->mutex);
  ပြန်ပေး rc;
}

/*
** Interface to the testing logic.
*/
ကိန်းပြည့် sqlite3_test_control(ကိန်းပြည့် op, ...){
  ကိန်းပြည့် rc = 0;
#ifdef SQLITE_UNTESTABLE
  UNUSED_PARAMETER(op);
#မဟုတ်ပါက
  va_list ap;
  va_start(ap, op);
  switch( op ){

    /*
    ** Save the current state of the PRNG.
    */
    pak_sa SQLITE_TESTCTRL_PRNG_SAVE: {
      sqlite3PrngSaveState();
      bha_nga;
    }

    /*
    ** Restore the state of the PRNG to the last state saved using
    ** PRNG_SAVE.  If PRNG_SAVE has never before been called, then
    ** this verb acts like PRNG_RESET.
    */
    pak_sa SQLITE_TESTCTRL_PRNG_RESTORE: {
      sqlite3PrngRestoreState();
      bha_nga;
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
    pak_sa SQLITE_TESTCTRL_PRNG_SEED: {
      ကိန်းပြည့် x = va_arg(ap, ကိန်းပြည့်);
      ကိန်းပြည့် y;
      sqlite3 *db = va_arg(ap, sqlite3*);
      assert( db==0 || db->aDb[0].pSchema!=0 );
      အကယ်၍( db && (y = db->aDb[0].pSchema->schema_cookie)!=0 ){ x = y; }
      sqlite3Config.iPrngSeed = x;
      sqlite3_randomness(0,0);
      bha_nga;
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
    pak_sa SQLITE_TESTCTRL_FK_NO_ACTION: {
      sqlite3 *db = va_arg(ap, sqlite3*);
      ကိန်းပြည့် b = va_arg(ap, ကိန်းပြည့်);
      အကယ်၍( b ){
        db->flags |= SQLITE_FkNoAction;
      }မဟုတ်ပါက{
        db->flags &= ~SQLITE_FkNoAction;
      }
      bha_nga;
    }

    /*
    **  sqlite3_test_control(BITVEC_TEST, size, program)
    **
    ** Run a test against a Bitvec object of size.  The program argument
    ** is an array of integers that defines the test.  Return -1 on a
    ** memory allocation error, 0 on success, or non-zero for an error.
    ** See the sqlite3BitvecBuiltinTest() for additional information.
    */
    pak_sa SQLITE_TESTCTRL_BITVEC_TEST: {
      ကိန်းပြည့် sz = va_arg(ap, ကိန်းပြည့်);
      ကိန်းပြည့် *aProg = va_arg(ap, ကိန်းပြည့်*);
      rc = sqlite3BitvecBuiltinTest(sz, aProg);
      bha_nga;
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
    pak_sa SQLITE_TESTCTRL_FAULT_INSTALL: {
      /* A bug in MSVC prevents it from understanding pointers to functions
      ** types in the second argument to va_arg().  Work around the problem
      ** using a typedef.
      ** http://support.microsoft.com/kb/47961  <-- dead hyperlink
      ** Search at http://web.archive.org/ to find the 2015-03-16 archive
      ** of the link above to see the original text.
      ** sqlite3GlobalConfig.xTestCallback = va_arg(ap, int(*)(int));
      */
      typedef ကိန်းပြည့်(*sqlite3FaultFuncType)(ကိန်းပြည့်);
      sqlite3GlobalConfig.xTestCallback = va_arg(ap, sqlite3FaultFuncType);
      rc = sqlite3FaultSim(0);
      bha_nga;
    }

    /*
    **  sqlite3_test_control(BENIGN_MALLOC_HOOKS, xBegin, xEnd)
    **
    ** Register hooks to call to indicate which malloc() failures
    ** are benign.
    */
    pak_sa SQLITE_TESTCTRL_BENIGN_MALLOC_HOOKS: {
      typedef void (*void_function)(void);
      void_function xBenignBegin;
      void_function xBenignEnd;
      xBenignBegin = va_arg(ap, void_function);
      xBenignEnd = va_arg(ap, void_function);
      sqlite3BenignMallocHooks(xBenignBegin, xBenignEnd);
      bha_nga;
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
    pak_sa SQLITE_TESTCTRL_PENDING_BYTE: {
      rc = PENDING_BYTE;
#ifndef SQLITE_OMIT_WSD
      {
        unsigned ကိန်းပြည့် newVal = va_arg(ap, unsigned ကိန်းပြည့်);
        အကယ်၍( newVal ) sqlite3PendingByte = newVal;
      }
#endif
      bha_nga;
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
    pak_sa SQLITE_TESTCTRL_ASSERT: {
      ca_ncala ကိန်းပြည့် x = 0;
      assert( /*side-effects-ok*/ (x = va_arg(ap,ကိန်းပြည့်))!=0 );
      rc = x;
#အကယ်၍ defined(SQLITE_DEBUG)
      /* Invoke these debugging routines so that the compiler does not
      ** issue "defined but not used" warnings. */
      အကယ်၍( x==9999 ){
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
      bha_nga;
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
    pak_sa SQLITE_TESTCTRL_ALWAYS: {
      ကိန်းပြည့် x = va_arg(ap,ကိန်းပြည့်);
      rc = x ? ALWAYS(x) : 0;
      bha_nga;
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
    pak_sa SQLITE_TESTCTRL_BYTEORDER: {
      rc = SQLITE_BYTEORDER*100 + SQLITE_LITTLEENDIAN*10 + SQLITE_BIGENDIAN;
      bha_nga;
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
    pak_sa SQLITE_TESTCTRL_OPTIMIZATIONS: {
      sqlite3 *db = va_arg(ap, sqlite3*);
      db->dbOptFlags = va_arg(ap, u32);
      bha_nga;
    }

    /*  sqlite3_test_control(SQLITE_TESTCTRL_GETOPT, sqlite3 *db, int *N)
    **
    ** Write the current optimization settings into *N.  A zero bit means that
    ** the optimization is on, and a 1 bit means that the optimization is off.
    */
    pak_sa SQLITE_TESTCTRL_GETOPT: {
      sqlite3 *db = va_arg(ap, sqlite3*);
      ကိန်းပြည့် *pN = va_arg(ap, ကိန်းပြည့်*);
      *pN = db->dbOptFlags;
      bha_nga;
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
    pak_sa SQLITE_TESTCTRL_LOCALTIME_FAULT: {
      sqlite3GlobalConfig.bLocaltimeFault = va_arg(ap, ကိန်းပြည့်);
      အကယ်၍( sqlite3GlobalConfig.bLocaltimeFault==2 ){
        typedef ကိန်းပြည့်(*sqlite3LocaltimeType)(const void*,void*);
        sqlite3GlobalConfig.xAltLocaltime = va_arg(ap, sqlite3LocaltimeType);
      }မဟုတ်ပါက{
        sqlite3GlobalConfig.xAltLocaltime = 0;
      }
      bha_nga;
    }

    /*   sqlite3_test_control(SQLITE_TESTCTRL_INTERNAL_FUNCTIONS, sqlite3*);
    **
    ** Toggle the ability to use internal functions on or off for
    ** the database connection given in the argument.
    */
    pak_sa SQLITE_TESTCTRL_INTERNAL_FUNCTIONS: {
      sqlite3 *db = va_arg(ap, sqlite3*);
      db->mDbFlags ^= DBFLAG_InternalFunc;
      bha_nga;
    }

    /*   sqlite3_test_control(SQLITE_TESTCTRL_NEVER_CORRUPT, int);
    **
    ** Set or clear a flag that indicates that the database file is always well-
    ** formed and never corrupt.  This flag is clear by default, indicating that
    ** database files might have arbitrary corruption.  Setting the flag during
    ** testing causes certain assert() statements in the code to be activated
    ** that demonstrate invariants on well-formed database files.
    */
    pak_sa SQLITE_TESTCTRL_NEVER_CORRUPT: {
      sqlite3GlobalConfig.neverCorrupt = va_arg(ap, ကိန်းပြည့်);
      bha_nga;
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
    pak_sa SQLITE_TESTCTRL_EXTRA_SCHEMA_CHECKS: {
      sqlite3GlobalConfig.bExtraSchemaChecks = va_arg(ap, ကိန်းပြည့်);
      bha_nga;
    }

    /* Set the threshold at which OP_Once counters reset back to zero.
    ** By default this is 0x7ffffffe (over 2 billion), but that value is
    ** too big to test in a reasonable amount of time, so this control is
    ** provided to set a small and easily reachable reset value.
    */
    pak_sa SQLITE_TESTCTRL_ONCE_RESET_THRESHOLD: {
      sqlite3GlobalConfig.iOnceResetThreshold = va_arg(ap, ကိန်းပြည့်);
      bha_nga;
    }

    /*   sqlite3_test_control(SQLITE_TESTCTRL_VDBE_COVERAGE, xCallback, ptr);
    **
    ** Set the VDBE coverage callback function to xCallback with context
    ** pointer ptr.
    */
    pak_sa SQLITE_TESTCTRL_VDBE_COVERAGE: {
#ifdef SQLITE_VDBE_COVERAGE
      typedef void (*branch_callback)(void*,unsigned ကိန်းပြည့်,
                                      unsigned var_na,unsigned var_na);
      sqlite3GlobalConfig.xVdbeBranch = va_arg(ap,branch_callback);
      sqlite3GlobalConfig.pVdbeBranchArg = va_arg(ap,void*);
#endif
      bha_nga;
    }

    /*   sqlite3_test_control(SQLITE_TESTCTRL_SORTER_MMAP, db, nMax); */
    pak_sa SQLITE_TESTCTRL_SORTER_MMAP: {
      sqlite3 *db = va_arg(ap, sqlite3*);
      db->nMaxSorterMmap = va_arg(ap, ကိန်းပြည့်);
      bha_nga;
    }

    /*   sqlite3_test_control(SQLITE_TESTCTRL_ISINIT);
    **
    ** Return SQLITE_OK if SQLite has been initialized and SQLITE_ERROR if
    ** not.
    */
    pak_sa SQLITE_TESTCTRL_ISINIT: {
      အကယ်၍( sqlite3GlobalConfig.isInit==0 ) rc = SQLITE_ERROR;
      bha_nga;
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
    pak_sa SQLITE_TESTCTRL_IMPOSTER: {
      sqlite3 *db = va_arg(ap, sqlite3*);
      ကိန်းပြည့် iDb;
      sqlite3_mutex_enter(db->mutex);
      iDb = sqlite3FindDbName(db, va_arg(ap,const var_na*));
      အကယ်၍( iDb>=0 ){
        db->init.iDb = iDb;
        db->init.busy = db->init.imposterTable = va_arg(ap,ကိန်းပြည့်);
        db->init.newTnum = va_arg(ap,ကိန်းပြည့်);
        အကယ်၍( db->init.busy==0 && db->init.newTnum>0 ){
          sqlite3ResetAllSchemasOfConnection(db);
        }
      }
      sqlite3_mutex_leave(db->mutex);
      bha_nga;
    }

#အကယ်၍ defined(YYCOVERAGE)
    /*  sqlite3_test_control(SQLITE_TESTCTRL_PARSER_COVERAGE, FILE *out)
    **
    ** This test control (only available when SQLite is compiled with
    ** -DYYCOVERAGE) writes a report onto "out" that shows all
    ** state/lookahead combinations in the parser state machine
    ** which are never exercised.  If any state is missed, make the
    ** return code SQLITE_ERROR.
    */
    pak_sa SQLITE_TESTCTRL_PARSER_COVERAGE: {
      FILE *out = va_arg(ap, FILE*);
      အကယ်၍( sqlite3ParserCoverage(out) ) rc = SQLITE_ERROR;
      bha_nga;
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
    pak_sa SQLITE_TESTCTRL_RESULT_INTREAL: {
      sqlite3_context *pCtx = va_arg(ap, sqlite3_context*);
      sqlite3ResultIntReal(pCtx);
      bha_nga;
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
    pak_sa SQLITE_TESTCTRL_SEEK_COUNT: {
      sqlite3 *db = va_arg(ap, sqlite3*);
      u64 *pn = va_arg(ap, sqlite3_uint64*);
      *pn = sqlite3BtreeSeekCount(db->aDb->pBt);
      (void)db;  /* Silence harmless unused variable warning */
      bha_nga;
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
    pak_sa SQLITE_TESTCTRL_TRACEFLAGS: {
       ကိန်းပြည့် opTrace = va_arg(ap, ကိန်းပြည့်);
       u32 *ptr = va_arg(ap, u32*);
       switch( opTrace ){
         pak_sa 0:   *ptr = sqlite3TreeTrace;      bha_nga;
         pak_sa 1:   sqlite3TreeTrace = *ptr;      bha_nga;
         pak_sa 2:   *ptr = sqlite3WhereTrace;     bha_nga;
         pak_sa 3:   sqlite3WhereTrace = *ptr;     bha_nga;
       }
       bha_nga;
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
    pak_sa SQLITE_TESTCTRL_LOGEST: {
      dviguu_na rIn = va_arg(ap, dviguu_na);
      LogEst rLogEst = sqlite3LogEstFromDouble(rIn);
      ကိန်းပြည့် *pI1 = va_arg(ap,ကိန်းပြည့်*);
      u64 *pU64 = va_arg(ap,u64*);
      ကိန်းပြည့် *pI2 = va_arg(ap,ကိန်းပြည့်*);
      *pI1 = rLogEst;
      *pU64 = sqlite3LogEstToInt(rLogEst);
      *pI2 = sqlite3LogEst(*pU64);
      bha_nga;
    }

    /* sqlite3_test_control(SQLITE_TESTCTRL_ATOF, const char *z, double *p);
    **
    ** Test access to the sqlite3AtoF() routine.
    */
    pak_sa SQLITE_TESTCTRL_ATOF: {
      const var_na *z = va_arg(ap,const var_na*);
      dviguu_na *pR = va_arg(ap,dviguu_na*);
      rc = sqlite3AtoF(z,pR);
      bha_nga;
    }

#အကယ်၍ defined(SQLITE_DEBUG) && !defined(SQLITE_OMIT_WSD)
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
    pak_sa SQLITE_TESTCTRL_TUNE: {
      ကိန်းပြည့် id = va_arg(ap, ကိန်းပြည့်);
      ကိန်းပြည့် *piValue = va_arg(ap, ကိန်းပြည့်*);
      အကယ်၍( id>0 && id<=SQLITE_NTUNE ){
        Tuning(id) = *piValue;
      }မဟုတ်ပါက အကယ်၍( id<0 && id>=-SQLITE_NTUNE ){
        *piValue = Tuning(-id);
      }မဟုတ်ပါက{
        rc = SQLITE_NOTFOUND;
      }
      bha_nga;
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
    pak_sa SQLITE_TESTCTRL_JSON_SELFCHECK: {
#အကယ်၍ defined(SQLITE_DEBUG) && !defined(SQLITE_OMIT_WSD)
      ကိန်းပြည့် *pOnOff = va_arg(ap, ကိန်းပြည့်*);
      အကယ်၍( *pOnOff<0 ){
        *pOnOff = sqlite3Config.bJsonSelfcheck;
      }မဟုတ်ပါက{
        sqlite3Config.bJsonSelfcheck = (u8)((*pOnOff)&0xff);
      }
#endif
      bha_nga;
    }
  }
  va_end(ap);
#endif /* SQLITE_UNTESTABLE */
  ပြန်ပေး rc;
}

/*
** The Pager stores the Database filename, Journal filename, and WAL filename
** consecutively in memory, in that order.  The database filename is prefixed
** by four zero bytes.  Locate the start of the database filename by searching
** backwards for the first byte following four consecutive zero bytes.
**
** This only works if the filename passed in was obtained from the Pager.
*/
static const var_na *databaseName(const var_na *zName){
  နေစဉ်( zName[-1]!=0 || zName[-2]!=0 || zName[-3]!=0 || zName[-4]!=0 ){
    zName--;
  }
  ပြန်ပေး zName;
}

/*
** Append text z[] to the end of p[].  Return a pointer to the first
** character after then zero terminator on the new text in p[].
*/
static var_na *appendText(var_na *p, const var_na *z){
  size_t n = strlen(z);
  memcpy(p, z, n+1);
  ပြန်ပေး p+n+1;
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
const var_na *sqlite3_create_filename(
  const var_na *zDatabase,
  const var_na *zJournal,
  const var_na *zWal,
  ကိန်းပြည့် nParam,
  const var_na **azParam
){
  sqlite3_int64 nByte;
  ကိန်းပြည့် i;
  var_na *pResult, *p;
  nByte = strlen(zDatabase) + strlen(zJournal) + strlen(zWal) + 10;
  အတွက်(i=0; i<nParam*2; i++){
    nByte += strlen(azParam[i])+1;
  }
  pResult = p = sqlite3_malloc64( nByte );
  အကယ်၍( p==0 ) ပြန်ပေး 0;
  memset(p, 0, 4);
  p += 4;
  p = appendText(p, zDatabase);
  အတွက်(i=0; i<nParam*2; i++){
    p = appendText(p, azParam[i]);
  }
  *(p++) = 0;
  p = appendText(p, zJournal);
  p = appendText(p, zWal);
  *(p++) = 0;
  *(p++) = 0;
  assert( (sqlite3_int64)(p - pResult)==nByte );
  ပြန်ပေး pResult + 4;
}

/*
** Free memory obtained from sqlite3_create_filename().  It is a severe
** error to call this routine with any parameter other than a pointer
** previously obtained from sqlite3_create_filename() or a NULL pointer.
*/
void sqlite3_free_filename(const var_na *p){
  အကယ်၍( p==0 ) ပြန်ပေး;
  p = databaseName(p);
  sqlite3_free((var_na*)p - 4);
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
const var_na *sqlite3_uri_parameter(const var_na *zFilename, const var_na *zParam){
  အကယ်၍( zFilename==0 || zParam==0 ) ပြန်ပေး 0;
  zFilename = databaseName(zFilename);
  ပြန်ပေး uriParameter(zFilename, zParam);
}

/*
** Return a pointer to the name of Nth query parameter of the filename.
*/
const var_na *sqlite3_uri_key(const var_na *zFilename, ကိန်းပြည့် N){
  အကယ်၍( zFilename==0 || N<0 ) ပြန်ပေး 0;
  zFilename = databaseName(zFilename);
  zFilename += sqlite3Strlen30(zFilename) + 1;
  နေစဉ်( ALWAYS(zFilename) && zFilename[0] && (N--)>0 ){
    zFilename += sqlite3Strlen30(zFilename) + 1;
    zFilename += sqlite3Strlen30(zFilename) + 1;
  }
  ပြန်ပေး zFilename[0] ? zFilename : 0;
}

/*
** Return a boolean value for a query parameter.
*/
ကိန်းပြည့် sqlite3_uri_boolean(const var_na *zFilename, const var_na *zParam, ကိန်းပြည့် bDflt){
  const var_na *z = sqlite3_uri_parameter(zFilename, zParam);
  bDflt = bDflt!=0;
  ပြန်ပေး z ? sqlite3GetBoolean(z, bDflt) : bDflt;
}

/*
** Return a 64-bit integer value for a query parameter.
*/
sqlite3_int64 sqlite3_uri_int64(
  const var_na *zFilename,    /* Filename as passed to xOpen */
  const var_na *zParam,       /* URI parameter sought */
  sqlite3_int64 bDflt       /* return if parameter is missing */
){
  const var_na *z = sqlite3_uri_parameter(zFilename, zParam);
  sqlite3_int64 v;
  အကယ်၍( z && sqlite3DecOrHexToI64(z, &v)==0 ){
    bDflt = v;
  }
  ပြန်ပေး bDflt;
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
const var_na *sqlite3_filename_database(const var_na *zFilename){
  အကယ်၍( zFilename==0 ) ပြန်ပေး 0;
  ပြန်ပေး databaseName(zFilename);
}
const var_na *sqlite3_filename_journal(const var_na *zFilename){
  အကယ်၍( zFilename==0 ) ပြန်ပေး 0;
  zFilename = databaseName(zFilename);
  zFilename += sqlite3Strlen30(zFilename) + 1;
  နေစဉ်( ALWAYS(zFilename) && zFilename[0] ){
    zFilename += sqlite3Strlen30(zFilename) + 1;
    zFilename += sqlite3Strlen30(zFilename) + 1;
  }
  ပြန်ပေး zFilename + 1;
}
const var_na *sqlite3_filename_wal(const var_na *zFilename){
#ifdef SQLITE_OMIT_WAL
  UNUSED_PARAMETER(zFilename);
  ပြန်ပေး 0;
#မဟုတ်ပါက
  zFilename = sqlite3_filename_journal(zFilename);
  အကယ်၍( zFilename ) zFilename += sqlite3Strlen30(zFilename) + 1;
  ပြန်ပေး zFilename;
#endif
}

/*
** Return the Btree pointer identified by zDbName.  Return NULL if not found.
*/
Btree *sqlite3DbNameToBtree(sqlite3 *db, const var_na *zDbName){
  ကိန်းပြည့် iDb = zDbName ? sqlite3FindDbName(db, zDbName) : 0;
  ပြန်ပေး iDb<0 ? 0 : db->aDb[iDb].pBt;
}

/*
** Return the name of the N-th database schema.  Return NULL if N is out
** of range.
*/
const var_na *sqlite3_db_name(sqlite3 *db, ကိန်းပြည့် N){
  const var_na *zRet = 0;
#ifdef SQLITE_ENABLE_API_ARMOR
  အကယ်၍( !sqlite3SafetyCheckOk(db) ){
    (void)SQLITE_MISUSE_BKPT;
    ပြန်ပေး 0;
  }
#endif
  sqlite3_mutex_enter(db->mutex);
  အကယ်၍( N>=0 && N<db->nDb ){
    zRet = db->aDb[N].zDbSName;
  }
  sqlite3_mutex_leave(db->mutex);
  ပြန်ပေး zRet;
}

/*
** Return the filename of the database associated with a database
** connection.
*/
const var_na *sqlite3_db_filename(sqlite3 *db, const var_na *zDbName){
  Btree *pBt;
#ifdef SQLITE_ENABLE_API_ARMOR
  အကယ်၍( !sqlite3SafetyCheckOk(db) ){
    (void)SQLITE_MISUSE_BKPT;
    ပြန်ပေး 0;
  }
#endif
  pBt = sqlite3DbNameToBtree(db, zDbName);
  ပြန်ပေး pBt ? sqlite3BtreeGetFilename(pBt) : 0;
}

/*
** Return 1 if database is read-only or 0 if read/write.  Return -1 if
** no such database exists.
*/
ကိန်းပြည့် sqlite3_db_readonly(sqlite3 *db, const var_na *zDbName){
  Btree *pBt;
#ifdef SQLITE_ENABLE_API_ARMOR
  အကယ်၍( !sqlite3SafetyCheckOk(db) ){
    (void)SQLITE_MISUSE_BKPT;
    ပြန်ပေး -1;
  }
#endif
  pBt = sqlite3DbNameToBtree(db, zDbName);
  ပြန်ပေး pBt ? sqlite3BtreeIsReadonly(pBt) : -1;
}

#ifdef SQLITE_ENABLE_SNAPSHOT
/*
** Obtain a snapshot handle for the snapshot of database zDb currently
** being read by handle db.
*/
ကိန်းပြည့် sqlite3_snapshot_get(
  sqlite3 *db,
  const var_na *zDb,
  sqlite3_snapshot **ppSnapshot
){
  ကိန်းပြည့် rc = SQLITE_ERROR;
#ifndef SQLITE_OMIT_WAL

#ifdef SQLITE_ENABLE_API_ARMOR
  အကယ်၍( !sqlite3SafetyCheckOk(db) ){
    ပြန်ပေး SQLITE_MISUSE_BKPT;
  }
#endif
  sqlite3_mutex_enter(db->mutex);

  အကယ်၍( db->autoCommit==0 ){
    ကိန်းပြည့် iDb = sqlite3FindDbName(db, zDb);
    အကယ်၍( iDb==0 || iDb>1 ){
      Btree *pBt = db->aDb[iDb].pBt;
      အကယ်၍( SQLITE_TXN_WRITE!=sqlite3BtreeTxnState(pBt) ){
        Pager *pPager = sqlite3BtreePager(pBt);
        i64 dummy = 0;
        sqlite3PagerSnapshotOpen(pPager, (sqlite3_snapshot*)&dummy);
        rc = sqlite3BtreeBeginTrans(pBt, 0, 0);
        sqlite3PagerSnapshotOpen(pPager, 0);
        အကယ်၍( rc==SQLITE_OK ){
          rc = sqlite3PagerSnapshotGet(sqlite3BtreePager(pBt), ppSnapshot);
        }
      }
    }
  }

  sqlite3_mutex_leave(db->mutex);
#endif   /* SQLITE_OMIT_WAL */
  ပြန်ပေး rc;
}

/*
** Open a read-transaction on the snapshot identified by pSnapshot.
*/
ကိန်းပြည့် sqlite3_snapshot_open(
  sqlite3 *db,
  const var_na *zDb,
  sqlite3_snapshot *pSnapshot
){
  ကိန်းပြည့် rc = SQLITE_ERROR;
#ifndef SQLITE_OMIT_WAL

#ifdef SQLITE_ENABLE_API_ARMOR
  အကယ်၍( !sqlite3SafetyCheckOk(db) ){
    ပြန်ပေး SQLITE_MISUSE_BKPT;
  }
#endif
  sqlite3_mutex_enter(db->mutex);
  အကယ်၍( db->autoCommit==0 ){
    ကိန်းပြည့် iDb;
    iDb = sqlite3FindDbName(db, zDb);
    အကယ်၍( iDb==0 || iDb>1 ){
      Btree *pBt = db->aDb[iDb].pBt;
      အကယ်၍( sqlite3BtreeTxnState(pBt)!=SQLITE_TXN_WRITE ){
        Pager *pPager = sqlite3BtreePager(pBt);
        ကိန်းပြည့် bUnlock = 0;
        အကယ်၍( sqlite3BtreeTxnState(pBt)!=SQLITE_TXN_NONE ){
          အကယ်၍( db->nVdbeActive==0 ){
            rc = sqlite3PagerSnapshotCheck(pPager, pSnapshot);
            အကယ်၍( rc==SQLITE_OK ){
              bUnlock = 1;
              rc = sqlite3BtreeCommit(pBt);
            }
          }
        }မဟုတ်ပါက{
          rc = SQLITE_OK;
        }
        အကယ်၍( rc==SQLITE_OK ){
          rc = sqlite3PagerSnapshotOpen(pPager, pSnapshot);
        }
        အကယ်၍( rc==SQLITE_OK ){
          rc = sqlite3BtreeBeginTrans(pBt, 0, 0);
          sqlite3PagerSnapshotOpen(pPager, 0);
        }
        အကယ်၍( bUnlock ){
          sqlite3PagerSnapshotUnlock(pPager);
        }
      }
    }
  }

  sqlite3_mutex_leave(db->mutex);
#endif   /* SQLITE_OMIT_WAL */
  ပြန်ပေး rc;
}

/*
** Recover as many snapshots as possible from the wal file associated with
** schema zDb of database db.
*/
ကိန်းပြည့် sqlite3_snapshot_recover(sqlite3 *db, const var_na *zDb){
  ကိန်းပြည့် rc = SQLITE_ERROR;
#ifndef SQLITE_OMIT_WAL
  ကိန်းပြည့် iDb;

#ifdef SQLITE_ENABLE_API_ARMOR
  အကယ်၍( !sqlite3SafetyCheckOk(db) ){
    ပြန်ပေး SQLITE_MISUSE_BKPT;
  }
#endif

  sqlite3_mutex_enter(db->mutex);
  iDb = sqlite3FindDbName(db, zDb);
  အကယ်၍( iDb==0 || iDb>1 ){
    Btree *pBt = db->aDb[iDb].pBt;
    အကယ်၍( SQLITE_TXN_NONE==sqlite3BtreeTxnState(pBt) ){
      rc = sqlite3BtreeBeginTrans(pBt, 0, 0);
      အကယ်၍( rc==SQLITE_OK ){
        rc = sqlite3PagerSnapshotRecover(sqlite3BtreePager(pBt));
        sqlite3BtreeCommit(pBt);
      }
    }
  }
  sqlite3_mutex_leave(db->mutex);
#endif   /* SQLITE_OMIT_WAL */
  ပြန်ပေး rc;
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
ကိန်းပြည့် sqlite3_compileoption_used(const var_na *zOptName){
  ကိန်းပြည့် i, n;
  ကိန်းပြည့် nOpt;
  const var_na **azCompileOpt;

#ifdef SQLITE_ENABLE_API_ARMOR
  အကယ်၍( zOptName==0 ){
    (void)SQLITE_MISUSE_BKPT;
    ပြန်ပေး 0;
  }
#endif

  azCompileOpt = sqlite3CompileOptions(&nOpt);

  အကယ်၍( sqlite3StrNICmp(zOptName, "SQLITE_", 7)==0 ) zOptName += 7;
  n = sqlite3Strlen30(zOptName);

  /* Since nOpt is normally in single digits, a linear search is
  ** adequate. No need for a binary search. */
  အတွက်(i=0; i<nOpt; i++){
    အကယ်၍( sqlite3StrNICmp(zOptName, azCompileOpt[i], n)==0
     && sqlite3IsIdChar((unsigned var_na)azCompileOpt[i][n])==0
    ){
      ပြန်ပေး 1;
    }
  }
  ပြန်ပေး 0;
}

/*
** Return the N-th compile-time option string.  If N is out of range,
** return a NULL pointer.
*/
const var_na *sqlite3_compileoption_get(ကိန်းပြည့် N){
  ကိန်းပြည့် nOpt;
  const var_na **azCompileOpt;
  azCompileOpt = sqlite3CompileOptions(&nOpt);
  အကယ်၍( N>=0 && N<nOpt ){
    ပြန်ပေး azCompileOpt[N];
  }
  ပြန်ပေး 0;
}
#endif /* SQLITE_OMIT_COMPILEOPTION_DIAGS */
