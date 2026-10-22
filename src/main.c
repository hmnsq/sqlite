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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_FTS3
# include "fts3.h"
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_RTREE
# include "rtree.h"
#ਜੇ_ਅੰਤ
#ਜੇ defined(SQLITE_ENABLE_ICU) || defined(SQLITE_ENABLE_ICU_COLLATIONS)
# include "sqliteicu.h"
#ਜੇ_ਅੰਤ

/*
** This is an extension initializer that is a no-op and always
** succeeds, except that it fails if the fault-simulation is set
** to 500.
*/
ਸਥਿਰ ਪੂਰਨ sqlite3TestExtInit(sqlite3 *db){
  (ਖਾਲੀ)db;
  ਵਾਪਸ sqlite3FaultSim(500);
}


/*
** Forward declarations of external module initializer functions
** for modules that need them.
*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_FTS5
ਪੂਰਨ sqlite3Fts5Init(sqlite3*);
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STMTVTAB
ਪੂਰਨ sqlite3StmtVtabInit(sqlite3*);
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_EXTRA_AUTOEXT
ਪੂਰਨ SQLITE_EXTRA_AUTOEXT(sqlite3*);
#ਜੇ_ਅੰਤ
/*
** An array of pointers to extension initializer functions for
** built-in extensions.
*/
ਸਥਿਰ ਪੂਰਨ (*ਅਚਲ sqlite3BuiltinExtensions[])(sqlite3*) = {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_FTS3
  sqlite3Fts3Init,
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_FTS5
  sqlite3Fts5Init,
#ਜੇ_ਅੰਤ
#ਜੇ defined(SQLITE_ENABLE_ICU) || defined(SQLITE_ENABLE_ICU_COLLATIONS)
  sqlite3IcuInit,
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_RTREE
  sqlite3RtreeInit,
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_DBPAGE_VTAB
  sqlite3_Dbpage_ਰਜਿਸਟਰ,
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_DBSTAT_VTAB
  sqlite3_Dbstat_ਰਜਿਸਟਰ,
#ਜੇ_ਅੰਤ
  sqlite3TestExtInit,
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STMTVTAB
  sqlite3StmtVtabInit,
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_BYTECODE_VTAB
  sqlite3VdbeBytecodeVtabInit,
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_EXTRA_AUTOEXT
  SQLITE_EXTRA_AUTOEXT,
#ਜੇ_ਅੰਤ
};

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_AMALGAMATION
/* IMPLEMENTATION-OF: R-46656-45156 The sqlite3_version[] string constant
** contains the text of SQLITE_VERSION macro.
*/
ਅਚਲ ਅੱਖਰ sqlite3_ਸੰਸਕਰਣ[] = SQLITE_ਸੰਸਕਰਣ;
#ਜੇ_ਅੰਤ

/* IMPLEMENTATION-OF: R-53536-42575 The sqlite3_libversion() function returns
** a pointer to the to the sqlite3_version[] string constant.
*/
ਅਚਲ ਅੱਖਰ *sqlite3_libversion(ਖਾਲੀ){ ਵਾਪਸ sqlite3_ਸੰਸਕਰਣ; }

/* IMPLEMENTATION-OF: R-25063-23286 The sqlite3_sourceid() function returns a
** pointer to a string constant whose value is the same as the
** SQLITE_SOURCE_ID C preprocessor macro. Except if SQLite is built using
** an edited copy of the amalgamation, then the last four characters of
** the hash might be different from SQLITE_SOURCE_ID.
*/
ਅਚਲ ਅੱਖਰ *sqlite3_sourceid(ਖਾਲੀ){ ਵਾਪਸ SQLITE_ਸਰੋਤ_ID; }

/* IMPLEMENTATION-OF: R-35210-63508 The sqlite3_libversion_number() function
** returns an integer equal to SQLITE_VERSION_NUMBER.
*/
ਪੂਰਨ sqlite3_libversion_ਸੰਖਿਆ(ਖਾਲੀ){ ਵਾਪਸ SQLITE_ਸੰਸਕਰਣ_ਸੰਖਿਆ; }

/* IMPLEMENTATION-OF: R-20790-14025 The sqlite3_threadsafe() function returns
** zero if and only if SQLite was compiled with mutexing code omitted due to
** the SQLITE_THREADSAFE compile-time option being set to 0.
*/
ਪੂਰਨ sqlite3_threadsafe(ਖਾਲੀ){ ਵਾਪਸ SQLITE_THREADSAFE; }

/*
** When compiling the test fixture or with debugging enabled (on Win32),
** this variable being set to non-zero will cause OSTRACE macros to emit
** extra diagnostic information.
*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_HAVE_OS_TRACE
# ifndef SQLITE_DEBUG_OS_TRACE
#   define SQLITE_DEBUG_OS_TRACE 0
# endif
  ਪੂਰਨ sqlite3OSTrace = SQLITE_DEBUG_OS_TRACE;
#ਜੇ_ਅੰਤ

#ਜੇ !defined(SQLITE_OMIT_TRACE) && defined(SQLITE_ENABLE_IOTRACE)
/*
** If the following function pointer is not NULL and if
** SQLITE_ENABLE_IOTRACE is enabled, then messages describing
** I/O active are written using this function.  These messages
** are intended for debugging activity only.
*/
SQLITE_API ਖਾਲੀ (SQLITE_CDECL *sqlite3IoTrace)(ਅਚਲ ਅੱਖਰ*, ...) = 0;
#ਜੇ_ਅੰਤ

/*
** If the following global variable points to a string which is the
** name of a directory, then that directory will be used to store
** temporary files.
**
** See also the "PRAGMA temp_store_directory" SQL command.
*/
ਅੱਖਰ *sqlite3_ਆਰਜ਼ੀ_directory = 0;

/*
** If the following global variable points to a string which is the
** name of a directory, then that directory will be used to store
** all database files specified with a relative pathname.
**
** See also the "PRAGMA data_store_directory" SQL command.
*/
ਅੱਖਰ *sqlite3_ਅੰਕੜੇ_directory = 0;

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
ਸਥਿਰ SQLITE_NOINLINE ਪੂਰਨ sqlite3Initialize(ਖਾਲੀ){
  MUTEX_LOGIC( sqlite3_mutex *pMainMtx; )   /* The main static mutex */
  ਪੂਰਨ rc;                                   /* Result code */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_EXTRA_INIT
  ਪੂਰਨ b_ਲੜੀਵਾਰ_Extra_Init = 0;                    /* Extra initialization needed */
#ਜੇ_ਅੰਤ

  /* If the following assert() fails on some obscure processor/compiler
  ** combination to warn that SQLite has been mis-compiled.  If you hit
  ** this assert(), that means you need to recompile with the
  ** -DSQLITE_PTRSIZE=n compile-time option to set the correct pointer size.
  */
  ਪ੍ਰਤਿਜ੍ਞਾ( SQLITE_PTRSIZE==ਆਕਾਰ(ਅੱਖਰ*) );

  /* Make sure the mutex subsystem is initialized.  If unable to
  ** initialize the mutex subsystem, return early with the error.
  ** If the system is so sick that we are unable to allocate a mutex,
  ** there is not much SQLite is going to be able to do.
  **
  ** The mutex subsystem must take care of serializing its own
  ** initialization.
  */
  rc = sqlite3MutexInit();
  ਜੇ( rc ) ਵਾਪਸ rc;

  /* Initialize the malloc() system and the recursive pInitMutex mutex.
  ** This operation is protected by the STATIC_MAIN mutex.  Note that
  ** MutexAlloc() is called for a static mutex prior to initializing the
  ** malloc subsystem - this implies that the allocation of a static
  ** mutex must not require support from the malloc subsystem.
  */
  MUTEX_LOGIC( pMainMtx = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  sqlite3_mutex_enter(pMainMtx);
  sqlite3GlobalConfig.isMutexInit = 1;
  ਜੇ( !sqlite3GlobalConfig.isMallocInit ){
    rc = sqlite3MallocInit();
  }
  ਜੇ( rc==SQLITE_OK ){
    sqlite3GlobalConfig.isMallocInit = 1;
    ਜੇ( !sqlite3GlobalConfig.pInitMutex ){
      sqlite3GlobalConfig.pInitMutex =
           sqlite3MutexAlloc(SQLITE_MUTEX_RECURSIVE);
      ਜੇ( sqlite3GlobalConfig.bCoreMutex && !sqlite3GlobalConfig.pInitMutex ){
        rc = SQLITE_NOMEM_BKPT;
      }
    }
  }
  ਜੇ( rc==SQLITE_OK ){
    sqlite3GlobalConfig.nRefInitMutex++;
  }
  sqlite3_mutex_leave(pMainMtx);

  /* If rc is not SQLITE_OK at this point, then either the malloc
  ** subsystem could not be initialized or the system failed to allocate
  ** the pInitMutex mutex. Return an error in either case.  */
  ਜੇ( rc!=SQLITE_OK ){
    ਵਾਪਸ rc;
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
  ਜੇ( sqlite3GlobalConfig.isInit==0 && sqlite3GlobalConfig.ਵਿੱਚ_ਤਰੱਕੀ==0 ){
    sqlite3GlobalConfig.ਵਿੱਚ_ਤਰੱਕੀ = 1;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_SQLLOG
    {
      ਬਾਹਰੀ ਖਾਲੀ sqlite3_init_sqllog(ਖਾਲੀ);
      sqlite3_init_sqllog();
    }
#ਜੇ_ਅੰਤ
    memset(&sqlite3BuiltinFunctions, 0, ਆਕਾਰ(sqlite3BuiltinFunctions));
    sqlite3_ਰਜਿਸਟਰ_Builtin_Functions();
    ਜੇ( sqlite3GlobalConfig.isPCacheInit==0 ){
      rc = sqlite3PcacheInitialize();
    }
    ਜੇ( rc==SQLITE_OK ){
      sqlite3GlobalConfig.isPCacheInit = 1;
      rc = sqlite3OsInit();
    }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_DESERIALIZE
    ਜੇ( rc==SQLITE_OK ){
      rc = sqlite3MemdbInit();
    }
#ਜੇ_ਅੰਤ
    ਜੇ( rc==SQLITE_OK ){
      sqlite3_PCache_ਬਫਰ_ਤਿਆਰੀ( sqlite3GlobalConfig.p_ਪੰਨਾ,
          sqlite3GlobalConfig.sz_ਪੰਨਾ, sqlite3GlobalConfig.n_ਪੰਨਾ);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_EXTRA_INIT_MUTEXED
      {
        ਪੂਰਨ SQLITE_EXTRA_INIT_MUTEXED(ਅਚਲ ਅੱਖਰ*);
        rc = SQLITE_EXTRA_INIT_MUTEXED(0);
      }
#ਜੇ_ਅੰਤ
    }
    ਜੇ( rc==SQLITE_OK ){
      sqlite3_ਸਮ੍ਰਿਤੀ_Barrier();
      sqlite3GlobalConfig.isInit = 1;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_EXTRA_INIT
      b_ਲੜੀਵਾਰ_Extra_Init = 1;
#ਜੇ_ਅੰਤ
    }
    sqlite3GlobalConfig.ਵਿੱਚ_ਤਰੱਕੀ = 0;
  }
  sqlite3_mutex_leave(sqlite3GlobalConfig.pInitMutex);

  /* Go back under the static mutex and clean up the recursive
  ** mutex to prevent a resource leak.
  */
  sqlite3_mutex_enter(pMainMtx);
  sqlite3GlobalConfig.nRefInitMutex--;
  ਜੇ( sqlite3GlobalConfig.nRefInitMutex<=0 ){
    ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3GlobalConfig.nRefInitMutex==0 );
    sqlite3_mutex_free(sqlite3GlobalConfig.pInitMutex);
    sqlite3GlobalConfig.pInitMutex = 0;
  }
  sqlite3_mutex_leave(pMainMtx);

  /* The following is just a sanity check to make sure SQLite has
  ** been compiled correctly.  It is important to run this code, but
  ** we don't want to run it too often and soak up CPU cycles for no
  ** reason.  So we run it once during initialization.
  */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NDEBUG
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_FLOATING_ਬਿੰਦੂ
  /* This section of code's only "output" is via assert() statements. */
  ਜੇ( rc==SQLITE_OK ){
    u64 x = (((u64)1)<<63)-1;
    ਦੁੱਗਣਾ y;
    ਪ੍ਰਤਿਜ੍ਞਾ(ਆਕਾਰ(x)==8);
    ਪ੍ਰਤਿਜ੍ਞਾ(ਆਕਾਰ(x)==ਆਕਾਰ(y));
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(&y, &x, 8);
    ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3IsNaN(y) );
  }
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ

  /* Do extra initialization steps requested by the SQLITE_EXTRA_INIT
  ** compile-time option.
  */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_EXTRA_INIT
  ਜੇ( b_ਲੜੀਵਾਰ_Extra_Init ){
    ਪੂਰਨ SQLITE_EXTRA_INIT(ਅਚਲ ਅੱਖਰ*);
    rc = SQLITE_EXTRA_INIT(0);
  }
#ਜੇ_ਅੰਤ
  ਵਾਪਸ rc;
}
SQLITE_API ਪੂਰਨ sqlite3_initialize(ਖਾਲੀ){
  /* If this build does not support writable static data (WSD) natively
  ** then we have to invoke the (application-supplied) WSD initialization
  ** routine before doing anything else. */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WSD
  ਪੂਰਨ rc = sqlite3_wsd_init(4096, 24);
  ਜੇ( rc!=SQLITE_OK ){
    ਵਾਪਸ rc;
  }
#ਜੇ_ਅੰਤ

  ਜੇ( sqlite3GlobalConfig.isInit ){
    /* SQLite has already been initialized.  Fast early-out. */
    sqlite3_ਸਮ੍ਰਿਤੀ_Barrier();
    ਵਾਪਸ SQLITE_OK;
  }ਨਹੀਂਤਾਂ{
    /* Invoke sqlite3Initialize() to do the actual work. */
    ਵਾਪਸ sqlite3Initialize();
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
ਪੂਰਨ sqlite3_shutdown(ਖਾਲੀ){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WSD
  ਪੂਰਨ rc = sqlite3_wsd_init(4096, 24);
  ਜੇ( rc!=SQLITE_OK ){
    ਵਾਪਸ rc;
  }
#ਜੇ_ਅੰਤ

  ਜੇ( sqlite3GlobalConfig.isInit ){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_EXTRA_SHUTDOWN
    ਖਾਲੀ SQLITE_EXTRA_SHUTDOWN(ਖਾਲੀ);
    SQLITE_EXTRA_SHUTDOWN();
#ਜੇ_ਅੰਤ
    sqlite3_os_ਸਿਰਾ();
    sqlite3_reset_auto_extension();
    sqlite3GlobalConfig.isInit = 0;
  }
  ਜੇ( sqlite3GlobalConfig.isPCacheInit ){
    sqlite3PcacheShutdown();
    sqlite3GlobalConfig.isPCacheInit = 0;
  }
  ਜੇ( sqlite3GlobalConfig.isMallocInit ){
    sqlite3_Malloc_ਸਿਰਾ();
    sqlite3GlobalConfig.isMallocInit = 0;

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_SHUTDOWN_DIRECTORIES
    /* The heap subsystem has now been shutdown and these values are supposed
    ** to be NULL or point to memory that was obtained from sqlite3_malloc(),
    ** which would rely on that heap subsystem; therefore, make sure these
    ** values cannot refer to heap memory that was just invalidated when the
    ** heap subsystem was shutdown.  This is only done if the current call to
    ** this function resulted in the heap subsystem actually being shutdown.
    */
    sqlite3_ਅੰਕੜੇ_directory = 0;
    sqlite3_ਆਰਜ਼ੀ_directory = 0;
#ਜੇ_ਅੰਤ
  }
  ਜੇ( sqlite3GlobalConfig.isMutexInit ){
    sqlite3_Mutex_ਸਿਰਾ();
    sqlite3GlobalConfig.isMutexInit = 0;
  }

  ਵਾਪਸ SQLITE_OK;
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
ਪੂਰਨ sqlite3_config(ਪੂਰਨ op, ...){
  va_list ap;
  ਪੂਰਨ rc = SQLITE_OK;

  /* sqlite3_config() normally returns SQLITE_MISUSE if it is invoked while
  ** the SQLite library is in use.  Except, a few selected opcodes
  ** are allowed.
  */
  ਜੇ( sqlite3GlobalConfig.isInit ){
    ਸਥਿਰ ਅਚਲ u64 mAnytimeConfigOption = 0
       | MASKBIT64( SQLITE_CONFIG_LOG )
       | MASKBIT64( SQLITE_CONFIG_PCACHE_HDRSZ )
    ;
    ਜੇ( op<0 || op>63 || (MASKBIT64(op) & mAnytimeConfigOption)==0 ){
      ਵਾਪਸ SQLITE_MISUSE_BKPT;
    }
    testcase( op==SQLITE_CONFIG_LOG );
    testcase( op==SQLITE_CONFIG_PCACHE_HDRSZ );
  }

  va_start(ap, op);
  ਚੋਣ( op ){

    /* Mutex configuration options are only available in a threadsafe
    ** compile.
    */
#ਜੇ defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE>0  /* IMP: R-54466-46756 */
    ਹਾਲਤ SQLITE_CONFIG_SINGLETHREAD: {
      /* EVIDENCE-OF: R-02748-19096 This option sets the threading mode to
      ** Single-thread. */
      sqlite3GlobalConfig.bCoreMutex = 0;  /* Disable mutex on core */
      sqlite3GlobalConfig.b_ਭਰਿਆ_Mutex = 0;  /* Disable mutex on connections */
      ਰੋਕੋ;
    }
#ਜੇ_ਅੰਤ
#ਜੇ defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE>0 /* IMP: R-20520-54086 */
    ਹਾਲਤ SQLITE_CONFIG_MULTITHREAD: {
      /* EVIDENCE-OF: R-14374-42468 This option sets the threading mode to
      ** Multi-thread. */
      sqlite3GlobalConfig.bCoreMutex = 1;  /* Enable mutex on core */
      sqlite3GlobalConfig.b_ਭਰਿਆ_Mutex = 0;  /* Disable mutex on connections */
      ਰੋਕੋ;
    }
#ਜੇ_ਅੰਤ
#ਜੇ defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE>0 /* IMP: R-59593-21810 */
    ਹਾਲਤ SQLITE_CONFIG_SERIALIZED: {
      /* EVIDENCE-OF: R-41220-51800 This option sets the threading mode to
      ** Serialized. */
      sqlite3GlobalConfig.bCoreMutex = 1;  /* Enable mutex on core */
      sqlite3GlobalConfig.b_ਭਰਿਆ_Mutex = 1;  /* Enable mutex on connections */
      ਰੋਕੋ;
    }
#ਜੇ_ਅੰਤ
#ਜੇ defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE>0 /* IMP: R-63666-48755 */
    ਹਾਲਤ SQLITE_CONFIG_MUTEX: {
      /* Specify an alternative mutex implementation */
      sqlite3GlobalConfig.mutex = *va_arg(ap, sqlite3_mutex_methods*);
      ਰੋਕੋ;
    }
#ਜੇ_ਅੰਤ
#ਜੇ defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE>0 /* IMP: R-14450-37597 */
    ਹਾਲਤ SQLITE_CONFIG_GETMUTEX: {
      /* Retrieve the current mutex implementation */
      *va_arg(ap, sqlite3_mutex_methods*) = sqlite3GlobalConfig.mutex;
      ਰੋਕੋ;
    }
#ਜੇ_ਅੰਤ

    ਹਾਲਤ SQLITE_CONFIG_MALLOC: {
      /* EVIDENCE-OF: R-55594-21030 The SQLITE_CONFIG_MALLOC option takes a
      ** single argument which is a pointer to an instance of the
      ** sqlite3_mem_methods structure. The argument specifies alternative
      ** low-level memory allocation routines to be used in place of the memory
      ** allocation routines built into SQLite. */
      sqlite3GlobalConfig.m = *va_arg(ap, sqlite3_mem_methods*);
      ਰੋਕੋ;
    }
    ਹਾਲਤ SQLITE_CONFIG_GETMALLOC: {
      /* EVIDENCE-OF: R-51213-46414 The SQLITE_CONFIG_GETMALLOC option takes a
      ** single argument which is a pointer to an instance of the
      ** sqlite3_mem_methods structure. The sqlite3_mem_methods structure is
      ** filled with the currently defined memory allocation routines. */
      ਜੇ( sqlite3GlobalConfig.m.xMalloc==0 ) sqlite3_Mem_ਸੈੱਟ_Default();
      *va_arg(ap, sqlite3_mem_methods*) = sqlite3GlobalConfig.m;
      ਰੋਕੋ;
    }
    ਹਾਲਤ SQLITE_CONFIG_MEMSTATUS: {
      ਪ੍ਰਤਿਜ੍ਞਾ( !sqlite3GlobalConfig.isInit );  /* Cannot change at runtime */
      /* EVIDENCE-OF: R-61275-35157 The SQLITE_CONFIG_MEMSTATUS option takes
      ** single argument of type int, interpreted as a boolean, which enables
      ** or disables the collection of memory allocation statistics. */
      sqlite3GlobalConfig.bMemstat = va_arg(ap, ਪੂਰਨ);
      ਰੋਕੋ;
    }
    ਹਾਲਤ SQLITE_CONFIG_SMALL_MALLOC: {
      sqlite3GlobalConfig.bSmallMalloc = va_arg(ap, ਪੂਰਨ)!=0;
      ਰੋਕੋ;
    }
    ਹਾਲਤ SQLITE_CONFIG_PAGECACHE: {
      /* EVIDENCE-OF: R-18761-36601 There are three arguments to
      ** SQLITE_CONFIG_PAGECACHE: A pointer to 8-byte aligned memory (pMem),
      ** the size of each page cache line (sz), and the number of cache lines
      ** (N). */
      sqlite3GlobalConfig.p_ਪੰਨਾ = va_arg(ap, ਖਾਲੀ*);
      sqlite3GlobalConfig.sz_ਪੰਨਾ = va_arg(ap, ਪੂਰਨ);
      sqlite3GlobalConfig.n_ਪੰਨਾ = va_arg(ap, ਪੂਰਨ);
      ਰੋਕੋ;
    }
    ਹਾਲਤ SQLITE_CONFIG_PCACHE_HDRSZ: {
      /* EVIDENCE-OF: R-39100-27317 The SQLITE_CONFIG_PCACHE_HDRSZ option takes
      ** a single parameter which is a pointer to an integer and writes into
      ** that integer the number of extra bytes per page required for each page
      ** in SQLITE_CONFIG_PAGECACHE. */
      *va_arg(ap, ਪੂਰਨ*) =
          sqlite3_ਸਿਰਲੇਖ_ਮਾਪ_Btree() +
          sqlite3_ਸਿਰਲੇਖ_ਮਾਪ_Pcache() +
          sqlite3_ਸਿਰਲੇਖ_ਮਾਪ_Pcache1();
      ਰੋਕੋ;
    }

    ਹਾਲਤ SQLITE_CONFIG_PCACHE: {
      /* no-op */
      ਰੋਕੋ;
    }
    ਹਾਲਤ SQLITE_CONFIG_GETPCACHE: {
      /* now an error */
      rc = SQLITE_ERROR;
      ਰੋਕੋ;
    }

    ਹਾਲਤ SQLITE_CONFIG_PCACHE2: {
      /* EVIDENCE-OF: R-63325-48378 The SQLITE_CONFIG_PCACHE2 option takes a
      ** single argument which is a pointer to an sqlite3_pcache_methods2
      ** object. This object specifies the interface to a custom page cache
      ** implementation. */
      sqlite3GlobalConfig.pcache2 = *va_arg(ap, sqlite3_pcache_methods2*);
      ਰੋਕੋ;
    }
    ਹਾਲਤ SQLITE_CONFIG_GETPCACHE2: {
      /* EVIDENCE-OF: R-22035-46182 The SQLITE_CONFIG_GETPCACHE2 option takes a
      ** single argument which is a pointer to an sqlite3_pcache_methods2
      ** object. SQLite copies of the current page cache implementation into
      ** that object. */
      ਜੇ( sqlite3GlobalConfig.pcache2.xInit==0 ){
        sqlite3_PCache_ਸੈੱਟ_Default();
      }
      *va_arg(ap, sqlite3_pcache_methods2*) = sqlite3GlobalConfig.pcache2;
      ਰੋਕੋ;
    }

/* EVIDENCE-OF: R-06626-12911 The SQLITE_CONFIG_HEAP option is only
** available if SQLite is compiled with either SQLITE_ENABLE_MEMSYS3 or
** SQLITE_ENABLE_MEMSYS5 and returns SQLITE_ERROR if invoked otherwise. */
#ਜੇ defined(SQLITE_ENABLE_MEMSYS3) || defined(SQLITE_ENABLE_MEMSYS5)
    ਹਾਲਤ SQLITE_CONFIG_ਢੇਰ: {
      /* EVIDENCE-OF: R-19854-42126 There are three arguments to
      ** SQLITE_CONFIG_HEAP: An 8-byte aligned pointer to the memory, the
      ** number of bytes in the memory buffer, and the minimum allocation size.
      */
      sqlite3GlobalConfig.p_ਢੇਰ = va_arg(ap, ਖਾਲੀ*);
      sqlite3GlobalConfig.n_ਢੇਰ = va_arg(ap, ਪੂਰਨ);
      sqlite3GlobalConfig.mnReq = va_arg(ap, ਪੂਰਨ);

      ਜੇ( sqlite3GlobalConfig.mnReq<1 ){
        sqlite3GlobalConfig.mnReq = 1;
      }ਨਹੀਂਤਾਂ ਜੇ( sqlite3GlobalConfig.mnReq>(1<<12) ){
        /* cap min request size at 2^12 */
        sqlite3GlobalConfig.mnReq = (1<<12);
      }

      ਜੇ( sqlite3GlobalConfig.p_ਢੇਰ==0 ){
        /* EVIDENCE-OF: R-49920-60189 If the first pointer (the memory pointer)
        ** is NULL, then SQLite reverts to using its default memory allocator
        ** (the system malloc() implementation), undoing any prior invocation of
        ** SQLITE_CONFIG_MALLOC.
        **
        ** Setting sqlite3GlobalConfig.m to all zeros will cause malloc to
        ** revert to its default implementation when sqlite3_initialize() is run
        */
        memset(&sqlite3GlobalConfig.m, 0, ਆਕਾਰ(sqlite3GlobalConfig.m));
      }ਨਹੀਂਤਾਂ{
        /* EVIDENCE-OF: R-61006-08918 If the memory pointer is not NULL then the
        ** alternative memory allocator is engaged to handle all of SQLites
        ** memory allocation needs. */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_MEMSYS3
        sqlite3GlobalConfig.m = *sqlite3MemGetMemsys3();
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_MEMSYS5
        sqlite3GlobalConfig.m = *sqlite3MemGetMemsys5();
#ਜੇ_ਅੰਤ
      }
      ਰੋਕੋ;
    }
#ਜੇ_ਅੰਤ

    ਹਾਲਤ SQLITE_CONFIG_LOOKASIDE: {
      sqlite3GlobalConfig.szLookaside = va_arg(ap, ਪੂਰਨ);
      sqlite3GlobalConfig.nLookaside = va_arg(ap, ਪੂਰਨ);
      ਰੋਕੋ;
    }
   
    /* Record a pointer to the logger function and its first argument.
    ** The default is NULL.  Logging is disabled if the function pointer is
    ** NULL.
    */
    ਹਾਲਤ SQLITE_CONFIG_LOG: {
      /* MSVC is picky about pulling func ptrs from va lists.
      ** http://support.microsoft.com/kb/47961
      ** sqlite3GlobalConfig.xLog = va_arg(ap, void(*)(void*,int,const char*));
      */
      ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਖਾਲੀ(*LOGFUNC_t)(ਖਾਲੀ*,ਪੂਰਨ,ਅਚਲ ਅੱਖਰ*);
      LOGFUNC_t xLog = va_arg(ap, LOGFUNC_t);
      ਖਾਲੀ *pLogArg = va_arg(ap, ਖਾਲੀ*);
      AtomicStore(&sqlite3GlobalConfig.xLog, xLog);
      AtomicStore(&sqlite3GlobalConfig.pLogArg, pLogArg);
      ਰੋਕੋ;
    }

    /* EVIDENCE-OF: R-55548-33817 The compile-time setting for URI filenames
    ** can be changed at start-time using the
    ** sqlite3_config(SQLITE_CONFIG_URI,1) or
    ** sqlite3_config(SQLITE_CONFIG_URI,0) configuration calls.
    */
    ਹਾਲਤ SQLITE_CONFIG_URI: {
      /* EVIDENCE-OF: R-25451-61125 The SQLITE_CONFIG_URI option takes a single
      ** argument of type int. If non-zero, then URI handling is globally
      ** enabled. If the parameter is zero, then URI handling is globally
      ** disabled. */
      ਪੂਰਨ bOpenUri = va_arg(ap, ਪੂਰਨ);
      AtomicStore(&sqlite3GlobalConfig.bOpenUri, bOpenUri);
      ਰੋਕੋ;
    }

    ਹਾਲਤ SQLITE_CONFIG_COVERING_ਸੂਚਕਾਂਕ_ਜਾਂਚ: {
      /* EVIDENCE-OF: R-36592-02772 The SQLITE_CONFIG_COVERING_INDEX_SCAN
      ** option takes a single integer argument which is interpreted as a
      ** boolean in order to enable or disable the use of covering indices for
      ** full table scans in the query optimizer. */
      sqlite3GlobalConfig.bUseCis = va_arg(ap, ਪੂਰਨ);
      ਰੋਕੋ;
    }

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_SQLLOG
    ਹਾਲਤ SQLITE_CONFIG_SQLLOG: {
      ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਖਾਲੀ(*SQLLOGFUNC_t)(ਖਾਲੀ*, sqlite3*, ਅਚਲ ਅੱਖਰ*, ਪੂਰਨ);
      sqlite3GlobalConfig.xSqllog = va_arg(ap, SQLLOGFUNC_t);
      sqlite3GlobalConfig.pSqllogArg = va_arg(ap, ਖਾਲੀ *);
      ਰੋਕੋ;
    }
#ਜੇ_ਅੰਤ

    ਹਾਲਤ SQLITE_CONFIG_MMAP_ਮਾਪ: {
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
      ਜੇ( mxMmap<0 || mxMmap>SQLITE_MAX_MMAP_ਮਾਪ ){
        mxMmap = SQLITE_MAX_MMAP_ਮਾਪ;
      }
      ਜੇ( szMmap<0 ) szMmap = SQLITE_DEFAULT_MMAP_ਮਾਪ;
      ਜੇ( szMmap>mxMmap) szMmap = mxMmap;
      sqlite3GlobalConfig.mxMmap = mxMmap;
      sqlite3GlobalConfig.szMmap = szMmap;
      ਰੋਕੋ;
    }

#ਜੇ SQLITE_OS_WIN && defined(SQLITE_WIN32_MALLOC) /* IMP: R-04780-55815 */
    ਹਾਲਤ SQLITE_CONFIG_WIN32_HEAPSIZE: {
      /* EVIDENCE-OF: R-34926-03360 SQLITE_CONFIG_WIN32_HEAPSIZE takes a 32-bit
      ** unsigned integer value that specifies the maximum size of the created
      ** heap. */
      sqlite3GlobalConfig.n_ਢੇਰ = va_arg(ap, ਪੂਰਨ);
      ਰੋਕੋ;
    }
#ਜੇ_ਅੰਤ

    ਹਾਲਤ SQLITE_CONFIG_PMASZ: {
      sqlite3GlobalConfig.szPma = va_arg(ap, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ);
      ਰੋਕੋ;
    }

    ਹਾਲਤ SQLITE_CONFIG_STMTJRNL_SPILL: {
      sqlite3GlobalConfig.nStmtSpill = va_arg(ap, ਪੂਰਨ);
      ਰੋਕੋ;
    }

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_SORTER_REFERENCES
    ਹਾਲਤ SQLITE_CONFIG_SORTERREF_ਮਾਪ: {
      ਪੂਰਨ iVal = va_arg(ap, ਪੂਰਨ);
      ਜੇ( iVal<0 ){
        iVal = SQLITE_DEFAULT_SORTERREF_ਮਾਪ;
      }
      sqlite3GlobalConfig.szSorterRef = (u32)iVal;
      ਰੋਕੋ;
    }
#ਜੇ_ਅੰਤ /* SQLITE_ENABLE_SORTER_REFERENCES */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_DESERIALIZE
    ਹਾਲਤ SQLITE_CONFIG_MEMDB_MAXSIZE: {
      sqlite3GlobalConfig.mx_Memdb_ਮਾਪ = va_arg(ap, sqlite3_int64);
      ਰੋਕੋ;
    }
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_DESERIALIZE */

    ਹਾਲਤ SQLITE_CONFIG_ROWID_ਵਿੱਚ_VIEW: {
      ਪੂਰਨ *pVal = va_arg(ap,ਪੂਰਨ*);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ALLOW_ROWID_ਵਿੱਚ_VIEW
      ਜੇ( 0==*pVal ) sqlite3GlobalConfig.mNoVisibleRowid = TF_NoVisibleRowid;
      ਜੇ( 1==*pVal ) sqlite3GlobalConfig.mNoVisibleRowid = 0;
      *pVal = (sqlite3GlobalConfig.mNoVisibleRowid==0);
#ਨਹੀਂਤਾਂ
      *pVal = 0;
#ਜੇ_ਅੰਤ
      ਰੋਕੋ;
    }

    ਮੂਲ: {
      rc = SQLITE_ERROR;
      ਰੋਕੋ;
    }
  }
  va_end(ap);
  ਵਾਪਸ rc;
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
ਸਥਿਰ ਪੂਰਨ ਤਿਆਰੀ_Lookaside(
  sqlite3 *db,    /* Database connection being configured */
  ਖਾਲੀ *pBuf,     /* Memory to use for lookaside.  May be NULL */
  ਪੂਰਨ sz,         /* Desired size of each lookaside memory slot */
  ਪੂਰਨ cnt         /* Number of slots to allocate */
){
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_LOOKASIDE
  ਖਾਲੀ *p_ਸ਼ੁਰੂ;          /* Start of the lookaside buffer */
  sqlite3_int64 szAlloc; /* Total space set aside for lookaside memory */
  ਪੂਰਨ nBig;              /* Number of full-size slots */
  ਪੂਰਨ nSm;               /* Number smaller LOOKASIDE_SMALL-byte slots */
 
  ਜੇ( sqlite3_Lookaside_ਵਰਤਿਆ(db,0)>0 ){
    ਵਾਪਸ SQLITE_BUSY;
  }
  /* Free any existing lookaside buffer for this handle before
  ** allocating a new one so we don't have to have space for
  ** both at the same time.
  */
  ਜੇ( db->lookaside.bMalloced ){
    sqlite3_free(db->lookaside.p_ਸ਼ੁਰੂ);
  }
  /* The size of a lookaside slot after ROUNDDOWN8 needs to be larger
  ** than a pointer and small enough to fit in a u16.
  */
  sz = ROUNDDOWN8(sz);
  ਜੇ( sz<=(ਪੂਰਨ)ਆਕਾਰ(Lookaside_ਥਾਂ*) ) sz = 0;
  ਜੇ( sz>65528 ) sz = 65528;
  /* Count must be at least 1 to be useful, but not so large as to use
  ** more than 0x7fff0000 total bytes for lookaside. */
  ਜੇ( cnt<1 ) cnt = 0;
  ਜੇ( sz>0 && cnt>(0x7fff0000/sz) ) cnt = 0x7fff0000/sz;
  szAlloc = (i64)sz*(i64)cnt;
  ਜੇ( szAlloc==0 ){
    sz = 0;
    p_ਸ਼ੁਰੂ = 0;
  }ਨਹੀਂਤਾਂ ਜੇ( pBuf==0 ){
    sqlite3BeginBenignMalloc();
    p_ਸ਼ੁਰੂ = sqlite3Malloc( szAlloc );
    sqlite3_ਸਿਰਾ_Benign_Malloc();
    ਜੇ( p_ਸ਼ੁਰੂ ) szAlloc = sqlite3_Malloc_ਮਾਪ(p_ਸ਼ੁਰੂ);
  }ਨਹੀਂਤਾਂ{
    p_ਸ਼ੁਰੂ = pBuf;
  }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_TWOSIZE_LOOKASIDE
  ਜੇ( sz>=LOOKASIDE_SMALL*3 ){
    nBig = szAlloc/(3*LOOKASIDE_SMALL+sz);
    nSm = (szAlloc - (i64)sz*(i64)nBig)/LOOKASIDE_SMALL;
  }ਨਹੀਂਤਾਂ ਜੇ( sz>=LOOKASIDE_SMALL*2 ){
    nBig = szAlloc/(LOOKASIDE_SMALL+sz);
    nSm = (szAlloc - (i64)sz*(i64)nBig)/LOOKASIDE_SMALL;
  }ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
  ਜੇ( sz>0 ){
    nBig = szAlloc/sz;
    nSm = 0;
  }ਨਹੀਂਤਾਂ{
    nBig = nSm = 0;
  }
  db->lookaside.p_ਸ਼ੁਰੂ = p_ਸ਼ੁਰੂ;
  db->lookaside.pInit = 0;
  db->lookaside.pFree = 0;
  db->lookaside.sz = (u16)sz;
  db->lookaside.szTrue = (u16)sz;
  ਜੇ( p_ਸ਼ੁਰੂ ){
    ਪੂਰਨ i;
    Lookaside_ਥਾਂ *p;
    ਪ੍ਰਤਿਜ੍ਞਾ( sz > (ਪੂਰਨ)ਆਕਾਰ(Lookaside_ਥਾਂ*) );
    p = (Lookaside_ਥਾਂ*)p_ਸ਼ੁਰੂ;
    ਲਈ(i=0; i<nBig; i++){
      p->p_ਅਗਲਾ = db->lookaside.pInit;
      db->lookaside.pInit = p;
      p = (Lookaside_ਥਾਂ*)&((u8*)p)[sz];
    }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_TWOSIZE_LOOKASIDE
    db->lookaside.pSmallInit = 0;
    db->lookaside.pSmallFree = 0;
    db->lookaside.p_ਵਿਚਕਾਰ = p;
    ਲਈ(i=0; i<nSm; i++){
      p->p_ਅਗਲਾ = db->lookaside.pSmallInit;
      db->lookaside.pSmallInit = p;
      p = (Lookaside_ਥਾਂ*)&((u8*)p)[LOOKASIDE_SMALL];
    }
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
    ਪ੍ਰਤਿਜ੍ਞਾ( ((uptr)p)<=szAlloc + (uptr)p_ਸ਼ੁਰੂ );
    db->lookaside.p_ਸਿਰਾ = p;
    db->lookaside.bDisable = 0;
    db->lookaside.bMalloced = pBuf==0 ?1:0;
    db->lookaside.n_ਥਾਂ = nBig+nSm;
  }ਨਹੀਂਤਾਂ{
    db->lookaside.p_ਸ਼ੁਰੂ = 0;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_TWOSIZE_LOOKASIDE
    db->lookaside.pSmallInit = 0;
    db->lookaside.pSmallFree = 0;
    db->lookaside.p_ਵਿਚਕਾਰ = 0;
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
    db->lookaside.p_ਸਿਰਾ = 0;
    db->lookaside.bDisable = 1;
    db->lookaside.sz = 0;
    db->lookaside.bMalloced = 0;
    db->lookaside.n_ਥਾਂ = 0;
  }
  db->lookaside.p_True_ਸਿਰਾ = db->lookaside.p_ਸਿਰਾ;
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_Lookaside_ਵਰਤਿਆ(db,0)==0 );
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_LOOKASIDE */
  ਵਾਪਸ SQLITE_OK;
}

/*
** Return the mutex associated with a database connection.
*/
sqlite3_mutex *sqlite3_db_mutex(sqlite3 *db){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !sqlite3SafetyCheckOk(db) ){
    (ਖਾਲੀ)SQLITE_MISUSE_BKPT;
    ਵਾਪਸ 0;
  }
#ਜੇ_ਅੰਤ
  ਵਾਪਸ db->mutex;
}

/*
** Free up as much memory as we can from the given database
** connection.
*/
ਪੂਰਨ sqlite3_db_release_ਸਮ੍ਰਿਤੀ(sqlite3 *db){
  ਪੂਰਨ i;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !sqlite3SafetyCheckOk(db) ) ਵਾਪਸ SQLITE_MISUSE_BKPT;
#ਜੇ_ਅੰਤ
  sqlite3_mutex_enter(db->mutex);
  sqlite3BtreeEnterAll(db);
  ਲਈ(i=0; i<db->nDb; i++){
    Btree *pBt = db->aDb[i].pBt;
    ਜੇ( pBt ){
      Pager *pPager = sqlite3BtreePager(pBt);
      sqlite3PagerShrink(pPager);
    }
  }
  sqlite3BtreeLeaveAll(db);
  sqlite3_mutex_leave(db->mutex);
  ਵਾਪਸ SQLITE_OK;
}

/*
** Flush any dirty pages in the pager-cache for any attached database
** to disk.
*/
ਪੂਰਨ sqlite3_db_cacheflush(sqlite3 *db){
  ਪੂਰਨ i;
  ਪੂਰਨ rc = SQLITE_OK;
  ਪੂਰਨ b_ਵੇਖਿਆ_Busy = 0;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !sqlite3SafetyCheckOk(db) ) ਵਾਪਸ SQLITE_MISUSE_BKPT;
#ਜੇ_ਅੰਤ
  sqlite3_mutex_enter(db->mutex);
  sqlite3BtreeEnterAll(db);
  ਲਈ(i=0; rc==SQLITE_OK && i<db->nDb; i++){
    Btree *pBt = db->aDb[i].pBt;
    ਜੇ( pBt && sqlite3BtreeTxnState(pBt)==SQLITE_TXN_WRITE ){
      Pager *pPager = sqlite3BtreePager(pBt);
      rc = sqlite3PagerFlush(pPager);
      ਜੇ( rc==SQLITE_BUSY ){
        b_ਵੇਖਿਆ_Busy = 1;
        rc = SQLITE_OK;
      }
    }
  }
  sqlite3BtreeLeaveAll(db);
  sqlite3_mutex_leave(db->mutex);
  ਵਾਪਸ ((rc==SQLITE_OK && b_ਵੇਖਿਆ_Busy) ? SQLITE_BUSY : rc);
}

/*
** Configuration settings for an individual database connection
*/
ਪੂਰਨ sqlite3_db_config(sqlite3 *db, ਪੂਰਨ op, ...){
  va_list ap;
  ਪੂਰਨ rc;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !sqlite3SafetyCheckOk(db) ) ਵਾਪਸ SQLITE_MISUSE_BKPT;
#ਜੇ_ਅੰਤ
  sqlite3_mutex_enter(db->mutex);
  va_start(ap, op);
  ਚੋਣ( op ){
    ਹਾਲਤ SQLITE_DBCONFIG_MAINDBNAME: {
      /* IMP: R-06824-28531 */
      /* IMP: R-36257-52125 */
      db->aDb[0].zDbSName = va_arg(ap,ਅੱਖਰ*);
      rc = SQLITE_OK;
      ਰੋਕੋ;
    }
    ਹਾਲਤ SQLITE_DBCONFIG_LOOKASIDE: {
      ਖਾਲੀ *pBuf = va_arg(ap, ਖਾਲੀ*); /* IMP: R-26835-10964 */
      ਪੂਰਨ sz = va_arg(ap, ਪੂਰਨ);       /* IMP: R-47871-25994 */
      ਪੂਰਨ cnt = va_arg(ap, ਪੂਰਨ);      /* IMP: R-04460-53386 */
      rc = ਤਿਆਰੀ_Lookaside(db, pBuf, sz, cnt);
      ਰੋਕੋ;
    }
    ਹਾਲਤ SQLITE_DBCONFIG_FP_DIGITS: {
      ਪੂਰਨ n_ਵਿੱਚ = va_arg(ap, ਪੂਰਨ);
      ਪੂਰਨ *p_ਬਾਹਰਲਾ = va_arg(ap, ਪੂਰਨ*);
      ਜੇ( n_ਵਿੱਚ>3 && n_ਵਿੱਚ<24 ) db->nFpDigit = (u8)n_ਵਿੱਚ;
      ਜੇ( p_ਬਾਹਰਲਾ ) *p_ਬਾਹਰਲਾ = db->nFpDigit;
      rc = SQLITE_OK;
      ਰੋਕੋ;
    }
    ਮੂਲ: {
      ਸਥਿਰ ਅਚਲ ਬਣਤਰ {
        ਪੂਰਨ op;      /* The opcode */
        u64 ਨਕਾਬ;    /* Mask of the bit in sqlite3.flags to set/clear */
      } aFlagOp[] = {
        { SQLITE_DBCONFIG_ENABLE_FKEY,           SQLITE_Foreign_ਕੁੰਜੀਆਂ    },
        { SQLITE_DBCONFIG_ENABLE_TRIGGER,        SQLITE_EnableTrigger  },
        { SQLITE_DBCONFIG_ENABLE_VIEW,           SQLITE_EnableView     },
        { SQLITE_DBCONFIG_ENABLE_FTS3_TOKENIZER, SQLITE_Fts3Tokenizer  },
        { SQLITE_DBCONFIG_ENABLE_ਲੱਦੋ_EXTENSION, SQLITE_ਲੱਦੋ_Extension  },
        { SQLITE_DBCONFIG_NO_CKPT_ON_CLOSE,      SQLITE_NoCkptOnClose  },
        { SQLITE_DBCONFIG_ENABLE_QPSG,           SQLITE_EnableQPSG     },
        { SQLITE_DBCONFIG_TRIGGER_EQP,           SQLITE_TriggerEQP     },
        { SQLITE_DBCONFIG_RESET_DATABASE,        SQLITE_ResetDatabase  },
        { SQLITE_DBCONFIG_DEFENSIVE,             SQLITE_Defensive      },
        { SQLITE_DBCONFIG_WRITABLE_SCHEMA,       SQLITE_WriteSchema|
                                                 SQLITE_NoSchemaError  },
        { SQLITE_DBCONFIG_LEGACY_ALTER_ਸਾਰਣੀ,    SQLITE_LegacyAlter    },
        { SQLITE_DBCONFIG_DQS_DDL,               SQLITE_DqsDDL         },
        { SQLITE_DBCONFIG_DQS_DML,               SQLITE_DqsDML         },
        { SQLITE_DBCONFIG_LEGACY_FILE_FORMAT,    SQLITE_LegacyFileFmt  },
        { SQLITE_DBCONFIG_TRUSTED_SCHEMA,        SQLITE_TrustedSchema  },
        { SQLITE_DBCONFIG_STMT_SCANSTATUS,       SQLITE_Stmt_ਜਾਂਚ_ਸਥਿਤੀ },
        { SQLITE_DBCONFIG_ਉਲਟ_SCANORDER,     SQLITE_ਉਲਟ_ਕ੍ਰਮ   },
        { SQLITE_DBCONFIG_ENABLE_ATTACH_CREATE,  SQLITE_AttachCreate   },
        { SQLITE_DBCONFIG_ENABLE_ATTACH_WRITE,   SQLITE_AttachWrite    },
        { SQLITE_DBCONFIG_ENABLE_COMMENTS,       SQLITE_Comments       },
      };
      ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
      rc = SQLITE_ERROR; /* IMP: R-42790-23372 */
      ਲਈ(i=0; i<Array_ਮਾਪ(aFlagOp); i++){
        ਜੇ( aFlagOp[i].op==op ){
          ਪੂਰਨ onoff = va_arg(ap, ਪੂਰਨ);
          ਪੂਰਨ *pRes = va_arg(ap, ਪੂਰਨ*);
          u64 oldFlags = db->flags;
          ਜੇ( onoff>0 ){
            db->flags |= aFlagOp[i].ਨਕਾਬ;
          }ਨਹੀਂਤਾਂ ਜੇ( onoff==0 ){
            db->flags &= ~(u64)aFlagOp[i].ਨਕਾਬ;
          }
          ਜੇ( oldFlags!=db->flags ){
            sqlite3ExpirePreparedStatements(db, 0);
          }
          ਜੇ( pRes ){
            *pRes = (db->flags & aFlagOp[i].ਨਕਾਬ)!=0;
          }
          rc = SQLITE_OK;
          ਰੋਕੋ;
        }
      }
      ਰੋਕੋ;
    }
  }
  va_end(ap);
  sqlite3_mutex_leave(db->mutex);
  ਵਾਪਸ rc;
}

/*
** This is the default collating function named "BINARY" which is always
** available.
*/
ਪੂਰਨ sqlite3_ਦੁਆਧਾਰੀ_Compare(
  ਖਾਲੀ *Not_ਵਰਤਿਆ,
  ਪੂਰਨ nKey1, ਅਚਲ ਖਾਲੀ *pKey1,
  ਪੂਰਨ nKey2, ਅਚਲ ਖਾਲੀ *pKey2
){
  ਪੂਰਨ rc, n;
  ਅਣਵਰਤਿਆ_PARAMETER(Not_ਵਰਤਿਆ);
  n = nKey1<nKey2 ? nKey1 : nKey2;
  /* EVIDENCE-OF: R-65033-28449 The built-in BINARY collation compares
  ** strings byte by byte using the memcmp() function from the standard C
  ** library. */
  ਪ੍ਰਤਿਜ੍ਞਾ( pKey1 && pKey2 );
  rc = memcmp(pKey1, pKey2, n);
  ਜੇ( rc==0 ){
    rc = nKey1 - nKey2;
  }
  ਵਾਪਸ rc;
}

/*
** This is the collating function named "RTRIM" which is always
** available.  Ignore trailing spaces.
*/
ਸਥਿਰ ਪੂਰਨ rtrimCollFunc(
  ਖਾਲੀ *pUser,
  ਪੂਰਨ nKey1, ਅਚਲ ਖਾਲੀ *pKey1,
  ਪੂਰਨ nKey2, ਅਚਲ ਖਾਲੀ *pKey2
){
  ਅਚਲ u8 *pK1 = (ਅਚਲ u8*)pKey1;
  ਅਚਲ u8 *pK2 = (ਅਚਲ u8*)pKey2;
  ਜਦੋਂ( nKey1 && pK1[nKey1-1]==' ' ) nKey1--;
  ਜਦੋਂ( nKey2 && pK2[nKey2-1]==' ' ) nKey2--;
  ਵਾਪਸ sqlite3_ਦੁਆਧਾਰੀ_Compare(pUser, nKey1, pKey1, nKey2, pKey2);
}

/*
** Return true if CollSeq is the default built-in BINARY.
*/
#ਜੇ !SQLITE_ਵਰਤਦਾ_INLINE
ਪੂਰਨ sqlite3_Is_ਦੁਆਧਾਰੀ(ਅਚਲ CollSeq *p){
  ਪ੍ਰਤਿਜ੍ਞਾ( p==0 || p->xCmp!=sqlite3_ਦੁਆਧਾਰੀ_Compare
       || strcmp(p->zName,"BINARY")==0 );
  ਵਾਪਸ p==0 || p->xCmp==sqlite3_ਦੁਆਧਾਰੀ_Compare;
}
#ਜੇ_ਅੰਤ

/*
** Another built-in collating sequence: NOCASE.
**
** This collating sequence is intended to be used for "case independent
** comparison". SQLite's knowledge of upper and lower case equivalents
** extends only to the 26 characters used in the English language.
**
** At the moment there is only a UTF-8 implementation.
*/
ਸਥਿਰ ਪੂਰਨ nocaseCollatingFunc(
  ਖਾਲੀ *Not_ਵਰਤਿਆ,
  ਪੂਰਨ nKey1, ਅਚਲ ਖਾਲੀ *pKey1,
  ਪੂਰਨ nKey2, ਅਚਲ ਖਾਲੀ *pKey2
){
  ਪੂਰਨ r = sqlite3StrNICmp(
      (ਅਚਲ ਅੱਖਰ *)pKey1, (ਅਚਲ ਅੱਖਰ *)pKey2, (nKey1<nKey2)?nKey1:nKey2);
  ਅਣਵਰਤਿਆ_PARAMETER(Not_ਵਰਤਿਆ);
  ਜੇ( 0==r ){
    r = nKey1-nKey2;
  }
  ਵਾਪਸ r;
}

/*
** Return the ROWID of the most recent insert
*/
sqlite_int64 sqlite3_ਆਖਰੀ_ਪਾਓ_rowid(sqlite3 *db){
  i64 iRet;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !sqlite3SafetyCheckOk(db) ){
    (ਖਾਲੀ)SQLITE_MISUSE_BKPT;
    ਵਾਪਸ 0;
  }
#ਜੇ_ਅੰਤ
  sqlite3_mutex_enter(db->mutex);
  iRet = db->ਆਖਰੀ_Rowid;
  sqlite3_mutex_leave(db->mutex);
  ਵਾਪਸ iRet;
}

/*
** Set the value returned by the sqlite3_last_insert_rowid() API function.
*/
ਖਾਲੀ sqlite3_ਸੈੱਟ_ਆਖਰੀ_ਪਾਓ_rowid(sqlite3 *db, sqlite3_int64 iRowid){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !sqlite3SafetyCheckOk(db) ){
    (ਖਾਲੀ)SQLITE_MISUSE_BKPT;
    ਵਾਪਸ;
  }
#ਜੇ_ਅੰਤ
  sqlite3_mutex_enter(db->mutex);
  db->ਆਖਰੀ_Rowid = iRowid;
  sqlite3_mutex_leave(db->mutex);
}

/*
** Return the number of changes in the most recently executed DML
** statement.
*/
sqlite3_int64 sqlite3_changes64(sqlite3 *db){
  i64 iRet;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !sqlite3SafetyCheckOk(db) ){
    (ਖਾਲੀ)SQLITE_MISUSE_BKPT;
    ਵਾਪਸ 0;
  }
#ਜੇ_ਅੰਤ
  sqlite3_mutex_enter(db->mutex);
  iRet = db->nChange;
  sqlite3_mutex_leave(db->mutex);
  ਵਾਪਸ iRet;
}
ਪੂਰਨ sqlite3_changes(sqlite3 *db){
  ਵਾਪਸ (ਪੂਰਨ)sqlite3_changes64(db);
}

/*
** Return the number of changes since the database handle was opened.
*/
sqlite3_int64 sqlite3_ਕੁੱਲ_changes64(sqlite3 *db){
  i64 iRet;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !sqlite3SafetyCheckOk(db) ){
    (ਖਾਲੀ)SQLITE_MISUSE_BKPT;
    ਵਾਪਸ 0;
  }
#ਜੇ_ਅੰਤ
  sqlite3_mutex_enter(db->mutex);
  iRet = db->n_ਕੁੱਲ_Change;
  sqlite3_mutex_leave(db->mutex);
  ਵਾਪਸ iRet;
}
ਪੂਰਨ sqlite3_ਕੁੱਲ_changes(sqlite3 *db){
  ਵਾਪਸ (ਪੂਰਨ)sqlite3_ਕੁੱਲ_changes64(db);
}

/*
** Close all open savepoints. This function only manipulates fields of the
** database handle object, it does not close any savepoints that may be open
** at the b-tree/pager level.
*/
ਖਾਲੀ sqlite3CloseSavepoints(sqlite3 *db){
  ਜਦੋਂ( db->pSavepoint ){
    Savepoint *pTmp = db->pSavepoint;
    db->pSavepoint = pTmp->p_ਅਗਲਾ;
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
ਸਥਿਰ ਖਾਲੀ functionDestroy(sqlite3 *db, FuncDef *p){
  FuncDestructor *pDestructor;
  ਪ੍ਰਤਿਜ੍ਞਾ( (p->funcFlags & SQLITE_FUNC_BUILTIN)==0 );
  pDestructor = p->u.pDestructor;
  ਜੇ( pDestructor ){
    pDestructor->nRef--;
    ਜੇ( pDestructor->nRef==0 ){
      pDestructor->xDestroy(pDestructor->p_User_ਅੰਕੜੇ);
      sqlite3DbFree(db, pDestructor);
    }
  }
}

/*
** Disconnect all sqlite3_vtab objects that belong to database connection
** db. This is called when db is being closed.
*/
ਸਥਿਰ ਖਾਲੀ disconnectAllVtab(sqlite3 *db){
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
  ਪੂਰਨ i;
  HashElem *p;
  sqlite3BtreeEnterAll(db);
  ਲਈ(i=0; i<db->nDb; i++){
    Schema *pSchema = db->aDb[i].pSchema;
    ਜੇ( pSchema ){
      ਲਈ(p=sqlite_Hash_ਪਹਿਲਾ(&pSchema->tblHash); p; p=sqlite_Hash_ਅਗਲਾ(p)){
        ਸਾਰਣੀ *pTab = (ਸਾਰਣੀ *)sqlite_Hash_ਅੰਕੜੇ(p);
        ਜੇ( IsVirtual(pTab) ) sqlite3VtabDisconnect(db, pTab);
      }
    }
  }
  ਲਈ(p=sqlite_Hash_ਪਹਿਲਾ(&db->aModule); p; p=sqlite_Hash_ਅਗਲਾ(p)){
    Module *pMod = (Module *)sqlite_Hash_ਅੰਕੜੇ(p);
    ਜੇ( pMod->pEpoTab ){
      sqlite3VtabDisconnect(db, pMod->pEpoTab);
    }
  }
  sqlite3VtabUnlockList(db);
  sqlite3BtreeLeaveAll(db);
#ਨਹੀਂਤਾਂ
  ਅਣਵਰਤਿਆ_PARAMETER(db);
#ਜੇ_ਅੰਤ
}

/*
** Return TRUE if database connection db has unfinalized prepared
** statements or unfinished sqlite3_backup objects. 
*/
ਸਥਿਰ ਪੂਰਨ connectionIsBusy(sqlite3 *db){
  ਪੂਰਨ j;
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(db->mutex) );
  ਜੇ( db->pVdbe ) ਵਾਪਸ 1;
  ਲਈ(j=0; j<db->nDb; j++){
    Btree *pBt = db->aDb[j].pBt;
    ਜੇ( pBt && sqlite3_Btree_Is_ਵਿੱਚ_Backup(pBt) ) ਵਾਪਸ 1;
  }
  ਵਾਪਸ 0;
}

/*
** Close an existing SQLite database
*/
ਸਥਿਰ ਪੂਰਨ sqlite3Close(sqlite3 *db, ਪੂਰਨ forceZombie){
  ਜੇ( !db ){
    /* EVIDENCE-OF: R-63257-11740 Calling sqlite3_close() or
    ** sqlite3_close_v2() with a NULL pointer argument is a harmless no-op. */
    ਵਾਪਸ SQLITE_OK;
  }
  ਜੇ( !sqlite3SafetyCheckSickOrOk(db) ){
    ਵਾਪਸ SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(db->mutex);
  ਜੇ( db->mTrace & SQLITE_TRACE_CLOSE ){
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
  ਜੇ( !forceZombie && connectionIsBusy(db) ){
    sqlite3_Error_With_ਸੁਨੇਹਾ(db, SQLITE_BUSY, "unable to close due to unfinalized "
       "statements or unfinished backups");
    sqlite3_mutex_leave(db->mutex);
    ਵਾਪਸ SQLITE_BUSY;
  }

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_SQLLOG
  ਜੇ( sqlite3GlobalConfig.xSqllog ){
    /* Closing the handle. Fourth parameter is passed the value 2. */
    sqlite3GlobalConfig.xSqllog(sqlite3GlobalConfig.pSqllogArg, db, 0, 2);
  }
#ਜੇ_ਅੰਤ

  ਜਦੋਂ( db->p_Db_ਅੰਕੜੇ ){
    Db_Client_ਅੰਕੜੇ *p = db->p_Db_ਅੰਕੜੇ;
    db->p_Db_ਅੰਕੜੇ = p->p_ਅਗਲਾ;
    ਪ੍ਰਤਿਜ੍ਞਾ( p->p_ਅੰਕੜੇ!=0 );
    ਜੇ( p->xDestructor ) p->xDestructor(p->p_ਅੰਕੜੇ);
    sqlite3_free(p);
  }

  /* Convert the connection into a zombie and then close it.
  */
  db->eOpenState = SQLITE_STATE_ZOMBIE;
  sqlite3LeaveMutexAndCloseZombie(db);
  ਵਾਪਸ SQLITE_OK;
}

/*
** Return the transaction state for a single databse, or the maximum
** transaction state over all attached databases if zSchema is null.
*/
ਪੂਰਨ sqlite3_txn_state(sqlite3 *db, ਅਚਲ ਅੱਖਰ *zSchema){
  ਪੂਰਨ iDb, nDb;
  ਪੂਰਨ iTxn = -1;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !sqlite3SafetyCheckOk(db) ){
    (ਖਾਲੀ)SQLITE_MISUSE_BKPT;
    ਵਾਪਸ -1;
  }
#ਜੇ_ਅੰਤ
  sqlite3_mutex_enter(db->mutex);
  ਜੇ( zSchema ){
    nDb = iDb = sqlite3_ਲੱਭੋ_Db_Name(db, zSchema);
    ਜੇ( iDb<0 ) nDb--;
  }ਨਹੀਂਤਾਂ{
    iDb = 0;
    nDb = db->nDb-1;
  }
  ਲਈ(; iDb<=nDb; iDb++){
    Btree *pBt = db->aDb[iDb].pBt;
    ਪੂਰਨ x = pBt!=0 ? sqlite3BtreeTxnState(pBt) : SQLITE_TXN_NONE;
    ਜੇ( x>iTxn ) iTxn = x;
  }
  sqlite3_mutex_leave(db->mutex);
  ਵਾਪਸ iTxn;
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
ਪੂਰਨ sqlite3_close(sqlite3 *db){ ਵਾਪਸ sqlite3Close(db,0); }
ਪੂਰਨ sqlite3_close_v2(sqlite3 *db){ ਵਾਪਸ sqlite3Close(db,1); }


/*
** Close the mutex on database connection db.
**
** Furthermore, if database connection db is a zombie (meaning that there
** has been a prior call to sqlite3_close(db) or sqlite3_close_v2(db)) and
** every sqlite3_stmt has now been finalized and every sqlite3_backup has
** finished, then free all resources.
*/
ਖਾਲੀ sqlite3LeaveMutexAndCloseZombie(sqlite3 *db){
  HashElem *i;                    /* Hash table iterator */
  ਪੂਰਨ j;

  /* If there are outstanding sqlite3_stmt or sqlite3_backup objects
  ** or if the connection has not yet been closed by sqlite3_close_v2(),
  ** then just leave the mutex and return.
  */
  ਜੇ( db->eOpenState!=SQLITE_STATE_ZOMBIE || connectionIsBusy(db) ){
    sqlite3_mutex_leave(db->mutex);
    ਵਾਪਸ;
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
  ਲਈ(j=0; j<db->nDb; j++){
    ਬਣਤਰ Db *pDb = &db->aDb[j];
    ਜੇ( pDb->pBt ){
      sqlite3BtreeClose(pDb->pBt);
      pDb->pBt = 0;
      ਜੇ( j!=1 ){
        pDb->pSchema = 0;
      }
    }
  }
  /* Clear the TEMP schema separately and last */
  ਜੇ( db->aDb[1].pSchema ){
    sqlite3SchemaClear(db->aDb[1].pSchema);
    ਪ੍ਰਤਿਜ੍ਞਾ( db->aDb[1].pSchema->trigHash.ਗਿਣਤੀ_2==0 );
  }
  sqlite3VtabUnlockList(db);

  /* Free up the array of auxiliary databases */
  sqlite3CollapseDatabaseArray(db);
  ਪ੍ਰਤਿਜ੍ਞਾ( db->nDb<=2 );
  ਪ੍ਰਤਿਜ੍ਞਾ( db->aDb==db->aDbStatic );

  /* Tell the code in notify.c that the connection no longer holds any
  ** locks and does not require any further unlock-notify callbacks.
  */
  sqlite3ConnectionClosed(db);

  ਲਈ(i=sqlite_Hash_ਪਹਿਲਾ(&db->aFunc); i; i=sqlite_Hash_ਅਗਲਾ(i)){
    FuncDef *p_ਅਗਲਾ, *p;
    p = sqlite_Hash_ਅੰਕੜੇ(i);
    ਕਰੋ{
      functionDestroy(db, p);
      p_ਅਗਲਾ = p->p_ਅਗਲਾ;
      sqlite3DbFree(db, p);
      p = p_ਅਗਲਾ;
    }ਜਦੋਂ( p );
  }
  sqlite3HashClear(&db->aFunc);
  ਲਈ(i=sqlite_Hash_ਪਹਿਲਾ(&db->aCollSeq); i; i=sqlite_Hash_ਅਗਲਾ(i)){
    CollSeq *pColl = (CollSeq *)sqlite_Hash_ਅੰਕੜੇ(i);
    /* Invoke any destructors registered for collation sequence user data. */
    ਲਈ(j=0; j<3; j++){
      ਜੇ( pColl[j].xDel ){
        pColl[j].xDel(pColl[j].pUser);
      }
    }
    sqlite3DbFree(db, pColl);
  }
  sqlite3HashClear(&db->aCollSeq);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
  ਲਈ(i=sqlite_Hash_ਪਹਿਲਾ(&db->aModule); i; i=sqlite_Hash_ਅਗਲਾ(i)){
    Module *pMod = (Module *)sqlite_Hash_ਅੰਕੜੇ(i);
    sqlite3_Vtab_Eponymous_ਸਾਰਣੀ_Clear(db, pMod);
    sqlite3VtabModuleUnref(db, pMod);
  }
  sqlite3HashClear(&db->aModule);
#ਜੇ_ਅੰਤ

  sqlite3Error(db, SQLITE_OK); /* Deallocates any cached error strings. */
  sqlite3_ਮੁੱਲ_Free(db->pErr);
  sqlite3CloseExtensions(db);

  db->eOpenState = SQLITE_STATE_ERROR;

  /* The temp-database schema is allocated differently from the other schema
  ** objects (using sqliteMalloc() directly, instead of sqlite3BtreeSchema()).
  ** So it needs to be freed here. Todo: Why not roll the temp schema into
  ** the same sqliteMalloc() as the one that allocates the database
  ** structure?
  */
  sqlite3DbFree(db, db->aDb[1].pSchema);
  ਜੇ( db->xAutovacDestr ){
    db->xAutovacDestr(db->p_Autovac_ਪੰਨੇ_Arg);
  }
  sqlite3_mutex_leave(db->mutex);
  db->eOpenState = SQLITE_STATE_CLOSED;
  sqlite3_mutex_free(db->mutex);
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_Lookaside_ਵਰਤਿਆ(db,0)==0 );
  ਜੇ( db->lookaside.bMalloced ){
    sqlite3_free(db->lookaside.p_ਸ਼ੁਰੂ);
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
ਖਾਲੀ sqlite3RollbackAll(sqlite3 *db, ਪੂਰਨ trip_ਕੋਡ){
  ਪੂਰਨ i;
  ਪੂਰਨ ਵਿੱਚ_Trans = 0;
  ਪੂਰਨ schemaChange;
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(db->mutex) );
  sqlite3BeginBenignMalloc();

  /* Obtain all b-tree mutexes before making any calls to BtreeRollback().
  ** This is important in case the transaction being rolled back has
  ** modified the database schema. If the b-tree mutexes are not taken
  ** here, then another shared-cache connection might sneak in between
  ** the database rollback and schema reset, which can cause false
  ** corruption reports in some cases.  */
  sqlite3BtreeEnterAll(db);
  schemaChange = (db->mDbFlags & DBFLAG_SchemaChange)!=0 && db->init.busy==0;

  ਲਈ(i=0; i<db->nDb; i++){
    Btree *p = db->aDb[i].pBt;
    ਜੇ( p ){
      ਜੇ( sqlite3BtreeTxnState(p)==SQLITE_TXN_WRITE ){
        ਵਿੱਚ_Trans = 1;
      }
      sqlite3BtreeRollback(p, trip_ਕੋਡ, !schemaChange);
    }
  }
  sqlite3VtabRollback(db);
  sqlite3_ਸਿਰਾ_Benign_Malloc();

  ਜੇ( schemaChange ){
    sqlite3ExpirePreparedStatements(db, 0);
    sqlite3_Reset_All_Schemas_ਦਾ_Connection(db);
  }
  sqlite3BtreeLeaveAll(db);

  /* Any deferred constraint violations have now been resolved. */
  db->nDeferredCons = 0;
  db->nDeferredImmCons = 0;
  db->flags &= ~(u64)(SQLITE_DeferFKs|SQLITE_CorruptRdOnly);

  /* If one has been configured, invoke the rollback-hook callback */
  ਜੇ( db->xRollbackCallback && (ਵਿੱਚ_Trans || !db->autoCommit) ){
    db->xRollbackCallback(db->pRollbackArg);
  }
}

/*
** Return a static string containing the name corresponding to the error code
** specified in the argument.
*/
#ਜੇ defined(SQLITE_NEED_ERR_NAME)
ਅਚਲ ਅੱਖਰ *sqlite3ErrName(ਪੂਰਨ rc){
  ਅਚਲ ਅੱਖਰ *zName = 0;
  ਪੂਰਨ i, origRc = rc;
  ਲਈ(i=0; i<2 && zName==0; i++, rc &= 0xff){
    ਚੋਣ( rc ){
      ਹਾਲਤ SQLITE_OK:                 zName = "SQLITE_OK";                ਰੋਕੋ;
      ਹਾਲਤ SQLITE_ERROR:              zName = "SQLITE_ERROR";             ਰੋਕੋ;
      ਹਾਲਤ SQLITE_ERROR_SNAPSHOT:     zName = "SQLITE_ERROR_SNAPSHOT";    ਰੋਕੋ;
      ਹਾਲਤ SQLITE_ERROR_RETRY:        zName = "SQLITE_ERROR_RETRY";       ਰੋਕੋ;
      ਹਾਲਤ SQLITE_ERROR_MISSING_COLLSEQ:
                                zName = "SQLITE_ERROR_MISSING_COLLSEQ";   ਰੋਕੋ;
      ਹਾਲਤ SQLITE_ਅੰਦਰੂਨੀ:           zName = "SQLITE_INTERNAL";          ਰੋਕੋ;
      ਹਾਲਤ SQLITE_PERM:               zName = "SQLITE_PERM";              ਰੋਕੋ;
      ਹਾਲਤ SQLITE_ABORT:              zName = "SQLITE_ABORT";             ਰੋਕੋ;
      ਹਾਲਤ SQLITE_ABORT_ROLLBACK:     zName = "SQLITE_ABORT_ROLLBACK";    ਰੋਕੋ;
      ਹਾਲਤ SQLITE_BUSY:               zName = "SQLITE_BUSY";              ਰੋਕੋ;
      ਹਾਲਤ SQLITE_BUSY_RECOVERY:      zName = "SQLITE_BUSY_RECOVERY";     ਰੋਕੋ;
      ਹਾਲਤ SQLITE_BUSY_SNAPSHOT:      zName = "SQLITE_BUSY_SNAPSHOT";     ਰੋਕੋ;
      ਹਾਲਤ SQLITE_LOCKED:             zName = "SQLITE_LOCKED";            ਰੋਕੋ;
      ਹਾਲਤ SQLITE_LOCKED_SHAREDCACHE: zName = "SQLITE_LOCKED_SHAREDCACHE";ਰੋਕੋ;
      ਹਾਲਤ SQLITE_NOMEM:              zName = "SQLITE_NOMEM";             ਰੋਕੋ;
      ਹਾਲਤ SQLITE_READONLY:           zName = "SQLITE_READONLY";          ਰੋਕੋ;
      ਹਾਲਤ SQLITE_READONLY_RECOVERY:  zName = "SQLITE_READONLY_RECOVERY"; ਰੋਕੋ;
      ਹਾਲਤ SQLITE_READONLY_CANTINIT:  zName = "SQLITE_READONLY_CANTINIT"; ਰੋਕੋ;
      ਹਾਲਤ SQLITE_READONLY_ROLLBACK:  zName = "SQLITE_READONLY_ROLLBACK"; ਰੋਕੋ;
      ਹਾਲਤ SQLITE_READONLY_DBMOVED:   zName = "SQLITE_READONLY_DBMOVED";  ਰੋਕੋ;
      ਹਾਲਤ SQLITE_READONLY_DIRECTORY: zName = "SQLITE_READONLY_DIRECTORY";ਰੋਕੋ;
      ਹਾਲਤ SQLITE_ਰੁਕਾਵਟ:          zName = "SQLITE_INTERRUPT";         ਰੋਕੋ;
      ਹਾਲਤ SQLITE_IOERR:              zName = "SQLITE_IOERR";             ਰੋਕੋ;
      ਹਾਲਤ SQLITE_IOERR_READ:         zName = "SQLITE_IOERR_READ";        ਰੋਕੋ;
      ਹਾਲਤ SQLITE_IOERR_SHORT_READ:   zName = "SQLITE_IOERR_SHORT_READ";  ਰੋਕੋ;
      ਹਾਲਤ SQLITE_IOERR_WRITE:        zName = "SQLITE_IOERR_WRITE";       ਰੋਕੋ;
      ਹਾਲਤ SQLITE_IOERR_FSYNC:        zName = "SQLITE_IOERR_FSYNC";       ਰੋਕੋ;
      ਹਾਲਤ SQLITE_IOERR_DIR_FSYNC:    zName = "SQLITE_IOERR_DIR_FSYNC";   ਰੋਕੋ;
      ਹਾਲਤ SQLITE_IOERR_TRUNCATE:     zName = "SQLITE_IOERR_TRUNCATE";    ਰੋਕੋ;
      ਹਾਲਤ SQLITE_IOERR_FSTAT:        zName = "SQLITE_IOERR_FSTAT";       ਰੋਕੋ;
      ਹਾਲਤ SQLITE_IOERR_UNLOCK:       zName = "SQLITE_IOERR_UNLOCK";      ਰੋਕੋ;
      ਹਾਲਤ SQLITE_IOERR_RDLOCK:       zName = "SQLITE_IOERR_RDLOCK";      ਰੋਕੋ;
      ਹਾਲਤ SQLITE_IOERR_DELETE:       zName = "SQLITE_IOERR_DELETE";      ਰੋਕੋ;
      ਹਾਲਤ SQLITE_IOERR_NOMEM:        zName = "SQLITE_IOERR_NOMEM";       ਰੋਕੋ;
      ਹਾਲਤ SQLITE_IOERR_ACCESS:       zName = "SQLITE_IOERR_ACCESS";      ਰੋਕੋ;
      ਹਾਲਤ SQLITE_IOERR_CHECKRESERVEDLOCK:
                                zName = "SQLITE_IOERR_CHECKRESERVEDLOCK"; ਰੋਕੋ;
      ਹਾਲਤ SQLITE_IOERR_ਤਾਲਾ:         zName = "SQLITE_IOERR_LOCK";        ਰੋਕੋ;
      ਹਾਲਤ SQLITE_IOERR_CLOSE:        zName = "SQLITE_IOERR_CLOSE";       ਰੋਕੋ;
      ਹਾਲਤ SQLITE_IOERR_DIR_CLOSE:    zName = "SQLITE_IOERR_DIR_CLOSE";   ਰੋਕੋ;
      ਹਾਲਤ SQLITE_IOERR_SHMOPEN:      zName = "SQLITE_IOERR_SHMOPEN";     ਰੋਕੋ;
      ਹਾਲਤ SQLITE_IOERR_SHMSIZE:      zName = "SQLITE_IOERR_SHMSIZE";     ਰੋਕੋ;
      ਹਾਲਤ SQLITE_IOERR_SHMLOCK:      zName = "SQLITE_IOERR_SHMLOCK";     ਰੋਕੋ;
      ਹਾਲਤ SQLITE_IOERR_SHMMAP:       zName = "SQLITE_IOERR_SHMMAP";      ਰੋਕੋ;
      ਹਾਲਤ SQLITE_IOERR_SEEK:         zName = "SQLITE_IOERR_SEEK";        ਰੋਕੋ;
      ਹਾਲਤ SQLITE_IOERR_DELETE_NOENT: zName = "SQLITE_IOERR_DELETE_NOENT";ਰੋਕੋ;
      ਹਾਲਤ SQLITE_IOERR_MMAP:         zName = "SQLITE_IOERR_MMAP";        ਰੋਕੋ;
      ਹਾਲਤ SQLITE_IOERR_GETTEMPPATH:  zName = "SQLITE_IOERR_GETTEMPPATH"; ਰੋਕੋ;
      ਹਾਲਤ SQLITE_IOERR_CONVPATH:     zName = "SQLITE_IOERR_CONVPATH";    ਰੋਕੋ;
      ਹਾਲਤ SQLITE_CORRUPT:            zName = "SQLITE_CORRUPT";           ਰੋਕੋ;
      ਹਾਲਤ SQLITE_CORRUPT_VTAB:       zName = "SQLITE_CORRUPT_VTAB";      ਰੋਕੋ;
      ਹਾਲਤ SQLITE_NOTFOUND:           zName = "SQLITE_NOTFOUND";          ਰੋਕੋ;
      ਹਾਲਤ SQLITE_ਭਰਿਆ:               zName = "SQLITE_FULL";              ਰੋਕੋ;
      ਹਾਲਤ SQLITE_CANTOPEN:           zName = "SQLITE_CANTOPEN";          ਰੋਕੋ;
      ਹਾਲਤ SQLITE_CANTOPEN_NOTEMPDIR: zName = "SQLITE_CANTOPEN_NOTEMPDIR";ਰੋਕੋ;
      ਹਾਲਤ SQLITE_CANTOPEN_ISDIR:     zName = "SQLITE_CANTOPEN_ISDIR";    ਰੋਕੋ;
      ਹਾਲਤ SQLITE_CANTOPEN_FULLPATH:  zName = "SQLITE_CANTOPEN_FULLPATH"; ਰੋਕੋ;
      ਹਾਲਤ SQLITE_CANTOPEN_CONVPATH:  zName = "SQLITE_CANTOPEN_CONVPATH"; ਰੋਕੋ;
      ਹਾਲਤ SQLITE_CANTOPEN_SYMLINK:   zName = "SQLITE_CANTOPEN_SYMLINK";  ਰੋਕੋ;
      ਹਾਲਤ SQLITE_PROTOCOL:           zName = "SQLITE_PROTOCOL";          ਰੋਕੋ;
      ਹਾਲਤ SQLITE_ਖਾਲੀ_ਥਾਵਾਂ:              zName = "SQLITE_EMPTY";             ਰੋਕੋ;
      ਹਾਲਤ SQLITE_SCHEMA:             zName = "SQLITE_SCHEMA";            ਰੋਕੋ;
      ਹਾਲਤ SQLITE_TOOBIG:             zName = "SQLITE_TOOBIG";            ਰੋਕੋ;
      ਹਾਲਤ SQLITE_CONSTRAINT:         zName = "SQLITE_CONSTRAINT";        ਰੋਕੋ;
      ਹਾਲਤ SQLITE_CONSTRAINT_UNIQUE:  zName = "SQLITE_CONSTRAINT_UNIQUE"; ਰੋਕੋ;
      ਹਾਲਤ SQLITE_CONSTRAINT_TRIGGER: zName = "SQLITE_CONSTRAINT_TRIGGER";ਰੋਕੋ;
      ਹਾਲਤ SQLITE_CONSTRAINT_FOREIGNKEY:
                                zName = "SQLITE_CONSTRAINT_FOREIGNKEY";   ਰੋਕੋ;
      ਹਾਲਤ SQLITE_CONSTRAINT_CHECK:   zName = "SQLITE_CONSTRAINT_CHECK";  ਰੋਕੋ;
      ਹਾਲਤ SQLITE_CONSTRAINT_PRIMARYKEY:
                                zName = "SQLITE_CONSTRAINT_PRIMARYKEY";   ਰੋਕੋ;
      ਹਾਲਤ SQLITE_CONSTRAINT_NOTNULL: zName = "SQLITE_CONSTRAINT_NOTNULL";ਰੋਕੋ;
      ਹਾਲਤ SQLITE_CONSTRAINT_COMMITHOOK:
                                zName = "SQLITE_CONSTRAINT_COMMITHOOK";   ਰੋਕੋ;
      ਹਾਲਤ SQLITE_CONSTRAINT_VTAB:    zName = "SQLITE_CONSTRAINT_VTAB";   ਰੋਕੋ;
      ਹਾਲਤ SQLITE_CONSTRAINT_FUNCTION:
                                zName = "SQLITE_CONSTRAINT_FUNCTION";     ਰੋਕੋ;
      ਹਾਲਤ SQLITE_CONSTRAINT_ROWID:   zName = "SQLITE_CONSTRAINT_ROWID";  ਰੋਕੋ;
      ਹਾਲਤ SQLITE_MISMATCH:           zName = "SQLITE_MISMATCH";          ਰੋਕੋ;
      ਹਾਲਤ SQLITE_MISUSE:             zName = "SQLITE_MISUSE";            ਰੋਕੋ;
      ਹਾਲਤ SQLITE_NOLFS:              zName = "SQLITE_NOLFS";             ਰੋਕੋ;
      ਹਾਲਤ SQLITE_AUTH:               zName = "SQLITE_AUTH";              ਰੋਕੋ;
      ਹਾਲਤ SQLITE_FORMAT:             zName = "SQLITE_FORMAT";            ਰੋਕੋ;
      ਹਾਲਤ SQLITE_RANGE:              zName = "SQLITE_RANGE";             ਰੋਕੋ;
      ਹਾਲਤ SQLITE_NOTADB:             zName = "SQLITE_NOTADB";            ਰੋਕੋ;
      ਹਾਲਤ SQLITE_ਪੰਕਤੀ:                zName = "SQLITE_ROW";               ਰੋਕੋ;
      ਹਾਲਤ SQLITE_NOTICE:             zName = "SQLITE_NOTICE";            ਰੋਕੋ;
      ਹਾਲਤ SQLITE_NOTICE_RECOVER_WAL: zName = "SQLITE_NOTICE_RECOVER_WAL";ਰੋਕੋ;
      ਹਾਲਤ SQLITE_NOTICE_RECOVER_ROLLBACK:
                                zName = "SQLITE_NOTICE_RECOVER_ROLLBACK"; ਰੋਕੋ;
      ਹਾਲਤ SQLITE_NOTICE_RBU:         zName = "SQLITE_NOTICE_RBU"; ਰੋਕੋ;
      ਹਾਲਤ SQLITE_WARNING:            zName = "SQLITE_WARNING";           ਰੋਕੋ;
      ਹਾਲਤ SQLITE_WARNING_AUTOINDEX:  zName = "SQLITE_WARNING_AUTOINDEX"; ਰੋਕੋ;
      ਹਾਲਤ SQLITE_ਪੂਰਾ:               zName = "SQLITE_DONE";              ਰੋਕੋ;
    }
  }
  ਜੇ( zName==0 ){
    ਸਥਿਰ ਅੱਖਰ zBuf[50];
    sqlite3_snprintf(ਆਕਾਰ(zBuf), zBuf, "SQLITE_UNKNOWN(%d)", origRc);
    zName = zBuf;
  }
  ਵਾਪਸ zName;
}
#ਜੇ_ਅੰਤ

/*
** Return a static string that describes the kind of error specified in the
** argument.
*/
ਅਚਲ ਅੱਖਰ *sqlite3ErrStr(ਪੂਰਨ rc){
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ* ਅਚਲ a_ਸੁਨੇਹਾ[] = {
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
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DISABLE_LFS
    /* SQLITE_NOLFS       */ "large file support is disabled",
#ਨਹੀਂਤਾਂ
    /* SQLITE_NOLFS       */ 0,
#ਜੇ_ਅੰਤ
    /* SQLITE_AUTH        */ "authorization denied",
    /* SQLITE_FORMAT      */ 0,
    /* SQLITE_RANGE       */ "column index out of range",
    /* SQLITE_NOTADB      */ "file is not a database",
    /* SQLITE_NOTICE      */ "notification message",
    /* SQLITE_WARNING     */ "warning message",
  };
  ਅਚਲ ਅੱਖਰ *zErr = "unknown error";
  ਚੋਣ( rc ){
    ਹਾਲਤ SQLITE_ABORT_ROLLBACK: {
      zErr = "abort due to ROLLBACK";
      ਰੋਕੋ;
    }
    ਹਾਲਤ SQLITE_ਪੰਕਤੀ: {
      zErr = "another row available";
      ਰੋਕੋ;
    }
    ਹਾਲਤ SQLITE_ਪੂਰਾ: {
      zErr = "no more rows available";
      ਰੋਕੋ;
    }
    ਮੂਲ: {
      rc &= 0xff;
      ਜੇ( ALWAYS(rc>=0) && rc<Array_ਮਾਪ(a_ਸੁਨੇਹਾ) && a_ਸੁਨੇਹਾ[rc]!=0 ){
        zErr = a_ਸੁਨੇਹਾ[rc];
      }
      ਰੋਕੋ;
    }
  }
  ਵਾਪਸ zErr;
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
ਸਥਿਰ ਪੂਰਨ sqliteDefaultBusyCallback(
  ਖਾਲੀ *ptr,               /* Database connection */
  ਪੂਰਨ ਗਿਣਤੀ_2                /* Number of times table has been busy */
){
#ਜੇ SQLITE_OS_WIN || !defined(HAVE_NANOSLEEP) || HAVE_NANOSLEEP
  /* This case is for systems that have support for sleeping for fractions of
  ** a second.  Examples:  All windows systems, unix systems with nanosleep() */
  ਸਥਿਰ ਅਚਲ u8 delays[] =
     { 1, 2, 5, 10, 15, 20, 25, 25,  25,  50,  50, 100 };
  ਸਥਿਰ ਅਚਲ u8 totals[] =
     { 0, 1, 3,  8, 18, 33, 53, 78, 103, 128, 178, 228 };
# define NDELAY Array_ਮਾਪ(delays)
  sqlite3 *db = (sqlite3 *)ptr;
  ਪੂਰਨ tmout = db->busyTimeout;
  ਪੂਰਨ delay, ਪੂਰਵ;

  ਪ੍ਰਤਿਜ੍ਞਾ( ਗਿਣਤੀ_2>=0 );
  ਜੇ( ਗਿਣਤੀ_2 < NDELAY ){
    delay = delays[ਗਿਣਤੀ_2];
    ਪੂਰਵ = totals[ਗਿਣਤੀ_2];
  }ਨਹੀਂਤਾਂ{
    delay = delays[NDELAY-1];
    ਪੂਰਵ = totals[NDELAY-1] + delay*(ਗਿਣਤੀ_2-(NDELAY-1));
  }
  ਜੇ( ਪੂਰਵ + delay > tmout ){
    delay = tmout - ਪੂਰਵ;
    ਜੇ( delay<=0 ) ਵਾਪਸ 0;
  }
  sqlite3OsSleep(db->pVfs, delay*1000);
  ਵਾਪਸ 1;
#ਨਹੀਂਤਾਂ
  /* This case for unix systems that lack usleep() support.  Sleeping
  ** must be done in increments of whole seconds */
  sqlite3 *db = (sqlite3 *)ptr;
  ਪੂਰਨ tmout = ((sqlite3 *)ptr)->busyTimeout;
  ਜੇ( (ਗਿਣਤੀ_2+1)*1000 > tmout ){
    ਵਾਪਸ 0;
  }
  sqlite3OsSleep(db->pVfs, 1000000);
  ਵਾਪਸ 1;
#ਜੇ_ਅੰਤ
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
ਪੂਰਨ sqlite3InvokeBusyHandler(BusyHandler *p){
  ਪੂਰਨ rc;
  ਜੇ( p->xBusyHandler==0 || p->nBusy<0 ) ਵਾਪਸ 0;
  rc = p->xBusyHandler(p->pBusyArg, p->nBusy);
  ਜੇ( rc==0 ){
    p->nBusy = -1;
  }ਨਹੀਂਤਾਂ{
    p->nBusy++;
  }
  ਵਾਪਸ rc;
}

/*
** This routine sets the busy callback for an Sqlite database to the
** given callback function with the given argument.
*/
ਪੂਰਨ sqlite3_busy_handler(
  sqlite3 *db,
  ਪੂਰਨ (*xBusy)(ਖਾਲੀ*,ਪੂਰਨ),
  ਖਾਲੀ *pArg
){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !sqlite3SafetyCheckOk(db) ) ਵਾਪਸ SQLITE_MISUSE_BKPT;
#ਜੇ_ਅੰਤ
  sqlite3_mutex_enter(db->mutex);
  db->busyHandler.xBusyHandler = xBusy;
  db->busyHandler.pBusyArg = pArg;
  db->busyHandler.nBusy = 0;
  db->busyTimeout = 0;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_SETLK_TIMEOUT
  db->setlkTimeout = 0;
#ਜੇ_ਅੰਤ
  sqlite3_mutex_leave(db->mutex);
  ਵਾਪਸ SQLITE_OK;
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_ਤਰੱਕੀ_CALLBACK
/*
** This routine sets the progress callback for an Sqlite database to the
** given callback function with the given argument. The progress callback will
** be invoked every nOps opcodes.
*/
ਖਾਲੀ sqlite3_ਤਰੱਕੀ_handler(
  sqlite3 *db,
  ਪੂਰਨ nOps,
  ਪੂਰਨ (*x_ਤਰੱਕੀ)(ਖਾਲੀ*),
  ਖਾਲੀ *pArg
){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !sqlite3SafetyCheckOk(db) ){
    (ਖਾਲੀ)SQLITE_MISUSE_BKPT;
    ਵਾਪਸ;
  }
#ਜੇ_ਅੰਤ
  sqlite3_mutex_enter(db->mutex);
  ਜੇ( nOps>0 ){
    db->x_ਤਰੱਕੀ = x_ਤਰੱਕੀ;
    db->n_ਤਰੱਕੀ_Ops = (ਬਿਨਾਨਿਸ਼ਾਨੀ)nOps;
    db->p_ਤਰੱਕੀ_Arg = pArg;
  }ਨਹੀਂਤਾਂ{
    db->x_ਤਰੱਕੀ = 0;
    db->n_ਤਰੱਕੀ_Ops = 0;
    db->p_ਤਰੱਕੀ_Arg = 0;
  }
  sqlite3_mutex_leave(db->mutex);
}
#ਜੇ_ਅੰਤ


/*
** This routine installs a default busy handler that waits for the
** specified number of milliseconds before returning 0.
*/
ਪੂਰਨ sqlite3_busy_timeout(sqlite3 *db, ਪੂਰਨ ms){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !sqlite3SafetyCheckOk(db) ) ਵਾਪਸ SQLITE_MISUSE_BKPT;
#ਜੇ_ਅੰਤ
  sqlite3_mutex_enter(db->mutex);
  ਜੇ( ms>0 ){
    sqlite3_busy_handler(db, (ਪੂਰਨ(*)(ਖਾਲੀ*,ਪੂਰਨ))sqliteDefaultBusyCallback,
                             (ਖਾਲੀ*)db);
    db->busyTimeout = ms;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_SETLK_TIMEOUT
    db->setlkTimeout = ms;
#ਜੇ_ਅੰਤ
  }ਨਹੀਂਤਾਂ{
    sqlite3_busy_handler(db, 0, 0);
  }
  sqlite3_mutex_leave(db->mutex);
  ਵਾਪਸ SQLITE_OK;
}

/*
** Set the setlk timeout value.
*/
ਪੂਰਨ sqlite3_setlk_timeout(sqlite3 *db, ਪੂਰਨ ms, ਪੂਰਨ flags){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_SETLK_TIMEOUT
  ਪੂਰਨ iDb;
  ਪੂਰਨ bBOC = ((flags & SQLITE_SETLK_ਖੰਡ_ON_CONNECT) ? 1 : 0);
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !sqlite3SafetyCheckOk(db) ) ਵਾਪਸ SQLITE_MISUSE_BKPT;
#ਜੇ_ਅੰਤ
  ਜੇ( ms<-1 ) ਵਾਪਸ SQLITE_RANGE;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_SETLK_TIMEOUT
  sqlite3_mutex_enter(db->mutex);
  db->setlkTimeout = ms;
  db->setlkFlags = flags;
  sqlite3BtreeEnterAll(db);
  ਲਈ(iDb=0; iDb<db->nDb; iDb++){
    Btree *pBt = db->aDb[iDb].pBt;
    ਜੇ( pBt ){
      sqlite3_file *fd = sqlite3PagerFile(sqlite3BtreePager(pBt));
      sqlite3OsFileControlHint(fd, SQLITE_FCNTL_ਖੰਡ_ON_CONNECT, (ਖਾਲੀ*)&bBOC);
    }
  }
  sqlite3BtreeLeaveAll(db);
  sqlite3_mutex_leave(db->mutex);
#ਜੇ_ਅੰਤ
#ਜੇ !defined(SQLITE_ENABLE_API_ARMOR) && !defined(SQLITE_ENABLE_SETLK_TIMEOUT)
  ਅਣਵਰਤਿਆ_PARAMETER(db);
  ਅਣਵਰਤਿਆ_PARAMETER(flags);
#ਜੇ_ਅੰਤ
  ਵਾਪਸ SQLITE_OK;
}

/*
** Cause any pending operation to stop at its earliest opportunity.
*/
ਖਾਲੀ sqlite3_ਰੁਕਾਵਟ(sqlite3 *db){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !sqlite3SafetyCheckOk(db)
   && (db==0 || db->eOpenState!=SQLITE_STATE_ZOMBIE)
  ){
    (ਖਾਲੀ)SQLITE_MISUSE_BKPT;
    ਵਾਪਸ;
  }
#ਜੇ_ਅੰਤ
  AtomicStore(&db->u1.isInterrupted, 1);
}

/*
** Return true or false depending on whether or not an interrupt is
** pending on connection db.
*/
ਪੂਰਨ sqlite3_is_interrupted(sqlite3 *db){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !sqlite3SafetyCheckOk(db)
   && (db==0 || db->eOpenState!=SQLITE_STATE_ZOMBIE)
  ){
    (ਖਾਲੀ)SQLITE_MISUSE_BKPT;
    ਵਾਪਸ 0;
  }
#ਜੇ_ਅੰਤ
  ਵਾਪਸ Atomic_ਲੱਦੋ(&db->u1.isInterrupted)!=0;
}

/*
** This function is exactly the same as sqlite3_create_function(), except
** that it is designed to be called by internal code. The difference is
** that if a malloc() fails in sqlite3_create_function(), an error code
** is returned and the mallocFailed flag cleared.
*/
ਪੂਰਨ sqlite3CreateFunc(
  sqlite3 *db,
  ਅਚਲ ਅੱਖਰ *zFunctionName,
  ਪੂਰਨ nArg,
  ਪੂਰਨ enc,
  ਖਾਲੀ *p_User_ਅੰਕੜੇ,
  ਖਾਲੀ (*xSFunc)(sqlite3_context*,ਪੂਰਨ,sqlite3_ਮੁੱਲ **),
  ਖਾਲੀ (*x_ਕਦਮ)(sqlite3_context*,ਪੂਰਨ,sqlite3_ਮੁੱਲ **),
  ਖਾਲੀ (*xFinal)(sqlite3_context*),
  ਖਾਲੀ (*x_ਮੁੱਲ)(sqlite3_context*),
  ਖਾਲੀ (*xInverse)(sqlite3_context*,ਪੂਰਨ,sqlite3_ਮੁੱਲ **),
  FuncDestructor *pDestructor
){
  FuncDef *p;
  ਪੂਰਨ extraFlags;

  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(db->mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( x_ਮੁੱਲ==0 || xSFunc==0 );
  ਜੇ( zFunctionName==0                /* Must have a valid name */
   || (xSFunc!=0 && xFinal!=0)        /* Not both xSFunc and xFinal */
   || ((xFinal==0)!=(x_ਕਦਮ==0))       /* Both or neither of xFinal and xStep */
   || ((x_ਮੁੱਲ==0)!=(xInverse==0))    /* Both or neither of xValue, xInverse */
   || (nArg<-1 || nArg>SQLITE_MAX_FUNCTION_ARG)
   || (255<sqlite3Strlen30(zFunctionName))
  ){
    ਵਾਪਸ SQLITE_MISUSE_BKPT;
  }

  ਪ੍ਰਤਿਜ੍ਞਾ( SQLITE_FUNC_CONSTANT==SQLITE_DETERMINISTIC );
  ਪ੍ਰਤਿਜ੍ਞਾ( SQLITE_FUNC_ਸਿੱਧਾ==SQLITE_DIRECTONLY );
  extraFlags = enc &  (SQLITE_DETERMINISTIC|SQLITE_DIRECTONLY|
                       SQLITE_SUBTYPE|SQLITE_INNOCUOUS|
                       SQLITE_ਨਤੀਜਾ_SUBTYPE|SQLITE_SELFORDER1);
  enc &= (SQLITE_FUNC_ENCMASK|SQLITE_ANY);

  /* The SQLITE_INNOCUOUS flag is the same bit as SQLITE_FUNC_UNSAFE.  But
  ** the meaning is inverted.  So flip the bit. */
  ਪ੍ਰਤਿਜ੍ਞਾ( SQLITE_FUNC_UNSAFE==SQLITE_INNOCUOUS );
  extraFlags ^= SQLITE_FUNC_UNSAFE;  /* tag-20230109-1 */

 
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_UTF16
  /* If SQLITE_UTF16 is specified as the encoding type, transform this
  ** to one of SQLITE_UTF16LE or SQLITE_UTF16BE using the
  ** SQLITE_UTF16NATIVE macro. SQLITE_UTF16 is not used internally.
  **
  ** If SQLITE_ANY is specified, add three versions of the function
  ** to the hash table.
  */
  ਚੋਣ( enc ){
    ਹਾਲਤ SQLITE_UTF16:
      enc = SQLITE_UTF16NATIVE;
      ਰੋਕੋ;
    ਹਾਲਤ SQLITE_ANY: {
      ਪੂਰਨ rc;
      rc = sqlite3CreateFunc(db, zFunctionName, nArg,
           (SQLITE_UTF8|extraFlags)^SQLITE_FUNC_UNSAFE, /* tag-20230109-1 */
           p_User_ਅੰਕੜੇ, xSFunc, x_ਕਦਮ, xFinal, x_ਮੁੱਲ, xInverse, pDestructor);
      ਜੇ( rc==SQLITE_OK ){
        rc = sqlite3CreateFunc(db, zFunctionName, nArg,
             (SQLITE_UTF16LE|extraFlags)^SQLITE_FUNC_UNSAFE, /* tag-20230109-1*/
             p_User_ਅੰਕੜੇ, xSFunc, x_ਕਦਮ, xFinal, x_ਮੁੱਲ, xInverse, pDestructor);
      }
      ਜੇ( rc!=SQLITE_OK ){
        ਵਾਪਸ rc;
      }
      enc = SQLITE_UTF16BE;
      ਰੋਕੋ;
    }
    ਹਾਲਤ SQLITE_UTF8:
    ਹਾਲਤ SQLITE_UTF16LE:
    ਹਾਲਤ SQLITE_UTF16BE:
      ਰੋਕੋ;
    ਮੂਲ:
      enc = SQLITE_UTF8;
      ਰੋਕੋ;
  }
#ਨਹੀਂਤਾਂ
  enc = SQLITE_UTF8;
#ਜੇ_ਅੰਤ
 
  /* Check if an existing function is being overridden or deleted. If so,
  ** and there are active VMs, then return SQLITE_BUSY. If a function
  ** is being overridden/deleted but there are no active VMs, allow the
  ** operation to continue but invalidate all precompiled statements.
  */
  p = sqlite3_ਲੱਭੋ_Function(db, zFunctionName, nArg, (u8)enc, 0);
  ਜੇ( p && (p->funcFlags & SQLITE_FUNC_ENCMASK)==(u32)enc && p->nArg==nArg ){
    ਜੇ( db->n_Vdbe_ਸਰਗਰਮ ){
      sqlite3_Error_With_ਸੁਨੇਹਾ(db, SQLITE_BUSY,
        "unable to delete/modify user-function due to active statements");
      ਪ੍ਰਤਿਜ੍ਞਾ( !db->mallocFailed || db->nVdbeExec>0 );
      ਵਾਪਸ SQLITE_BUSY;
    }ਨਹੀਂਤਾਂ{
      sqlite3ExpirePreparedStatements(db, 0);
    }
  }ਨਹੀਂਤਾਂ ਜੇ( xSFunc==0 && xFinal==0 ){
    /* Trying to delete a function that does not exist.  This is a no-op.
    ** https://sqlite.org/forum/forumpost/726219164b */
    ਵਾਪਸ SQLITE_OK;
  }

  p = sqlite3_ਲੱਭੋ_Function(db, zFunctionName, nArg, (u8)enc, 1);
  ਪ੍ਰਤਿਜ੍ਞਾ(p || db->mallocFailed);
  ਜੇ( !p ){
    ਵਾਪਸ SQLITE_NOMEM_BKPT;
  }

  /* If an older version of the function with a configured destructor is
  ** being replaced invoke the destructor function here. */
  functionDestroy(db, p);

  ਜੇ( pDestructor ){
    pDestructor->nRef++;
  }
  p->u.pDestructor = pDestructor;
  p->funcFlags = (p->funcFlags & SQLITE_FUNC_ENCMASK) | extraFlags;
  testcase( p->funcFlags & SQLITE_DETERMINISTIC );
  testcase( p->funcFlags & SQLITE_DIRECTONLY );
  p->xSFunc = xSFunc ? xSFunc : x_ਕਦਮ;
  p->xFinalize = xFinal;
  p->x_ਮੁੱਲ = x_ਮੁੱਲ;
  p->xInverse = xInverse;
  p->p_User_ਅੰਕੜੇ = p_User_ਅੰਕੜੇ;
  p->nArg = (u16)nArg;
  ਵਾਪਸ SQLITE_OK;
}

/*
** Worker function used by utf-8 APIs that create new functions:
**
**    sqlite3_create_function()
**    sqlite3_create_function_v2()
**    sqlite3_create_window_function()
*/
ਸਥਿਰ ਪੂਰਨ createFunctionApi(
  sqlite3 *db,
  ਅਚਲ ਅੱਖਰ *zFunc,
  ਪੂਰਨ nArg,
  ਪੂਰਨ enc,
  ਖਾਲੀ *p,
  ਖਾਲੀ (*xSFunc)(sqlite3_context*,ਪੂਰਨ,sqlite3_ਮੁੱਲ**),
  ਖਾਲੀ (*x_ਕਦਮ)(sqlite3_context*,ਪੂਰਨ,sqlite3_ਮੁੱਲ**),
  ਖਾਲੀ (*xFinal)(sqlite3_context*),
  ਖਾਲੀ (*x_ਮੁੱਲ)(sqlite3_context*),
  ਖਾਲੀ (*xInverse)(sqlite3_context*,ਪੂਰਨ,sqlite3_ਮੁੱਲ**),
  ਖਾਲੀ(*xDestroy)(ਖਾਲੀ*)
){
  ਪੂਰਨ rc = SQLITE_ERROR;
  FuncDestructor *pArg = 0;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !sqlite3SafetyCheckOk(db) ){
    ਵਾਪਸ SQLITE_MISUSE_BKPT;
  }
#ਜੇ_ਅੰਤ
  sqlite3_mutex_enter(db->mutex);
  ਜੇ( xDestroy ){
    pArg = (FuncDestructor *)sqlite3Malloc(ਆਕਾਰ(FuncDestructor));
    ਜੇ( !pArg ){
      sqlite3OomFault(db);
      xDestroy(p);
      ਜਾਓ ਬਾਹਰਲਾ;
    }
    pArg->nRef = 0;
    pArg->xDestroy = xDestroy;
    pArg->p_User_ਅੰਕੜੇ = p;
  }
  rc = sqlite3CreateFunc(db, zFunc, nArg, enc, p,
      xSFunc, x_ਕਦਮ, xFinal, x_ਮੁੱਲ, xInverse, pArg
  );
  ਜੇ( pArg && pArg->nRef==0 ){
    ਪ੍ਰਤਿਜ੍ਞਾ( rc!=SQLITE_OK || (x_ਕਦਮ==0 && xFinal==0) );
    xDestroy(p);
    sqlite3_free(pArg);
  }

 ਬਾਹਰਲਾ:
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  ਵਾਪਸ rc;
}

/*
** Create new user functions.
*/
ਪੂਰਨ sqlite3_create_function(
  sqlite3 *db,
  ਅਚਲ ਅੱਖਰ *zFunc,
  ਪੂਰਨ nArg,
  ਪੂਰਨ enc,
  ਖਾਲੀ *p,
  ਖਾਲੀ (*xSFunc)(sqlite3_context*,ਪੂਰਨ,sqlite3_ਮੁੱਲ **),
  ਖਾਲੀ (*x_ਕਦਮ)(sqlite3_context*,ਪੂਰਨ,sqlite3_ਮੁੱਲ **),
  ਖਾਲੀ (*xFinal)(sqlite3_context*)
){
  ਵਾਪਸ createFunctionApi(db, zFunc, nArg, enc, p, xSFunc, x_ਕਦਮ,
                                    xFinal, 0, 0, 0);
}
ਪੂਰਨ sqlite3_create_function_v2(
  sqlite3 *db,
  ਅਚਲ ਅੱਖਰ *zFunc,
  ਪੂਰਨ nArg,
  ਪੂਰਨ enc,
  ਖਾਲੀ *p,
  ਖਾਲੀ (*xSFunc)(sqlite3_context*,ਪੂਰਨ,sqlite3_ਮੁੱਲ **),
  ਖਾਲੀ (*x_ਕਦਮ)(sqlite3_context*,ਪੂਰਨ,sqlite3_ਮੁੱਲ **),
  ਖਾਲੀ (*xFinal)(sqlite3_context*),
  ਖਾਲੀ (*xDestroy)(ਖਾਲੀ *)
){
  ਵਾਪਸ createFunctionApi(db, zFunc, nArg, enc, p, xSFunc, x_ਕਦਮ,
                                    xFinal, 0, 0, xDestroy);
}
ਪੂਰਨ sqlite3_create_ਖਿੜਕੀ_function(
  sqlite3 *db,
  ਅਚਲ ਅੱਖਰ *zFunc,
  ਪੂਰਨ nArg,
  ਪੂਰਨ enc,
  ਖਾਲੀ *p,
  ਖਾਲੀ (*x_ਕਦਮ)(sqlite3_context*,ਪੂਰਨ,sqlite3_ਮੁੱਲ **),
  ਖਾਲੀ (*xFinal)(sqlite3_context*),
  ਖਾਲੀ (*x_ਮੁੱਲ)(sqlite3_context*),
  ਖਾਲੀ (*xInverse)(sqlite3_context*,ਪੂਰਨ,sqlite3_ਮੁੱਲ **),
  ਖਾਲੀ (*xDestroy)(ਖਾਲੀ *)
){
  ਵਾਪਸ createFunctionApi(db, zFunc, nArg, enc, p, 0, x_ਕਦਮ,
                                    xFinal, x_ਮੁੱਲ, xInverse, xDestroy);
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_UTF16
ਪੂਰਨ sqlite3_create_function16(
  sqlite3 *db,
  ਅਚਲ ਖਾਲੀ *zFunctionName,
  ਪੂਰਨ nArg,
  ਪੂਰਨ e_ਪਾਠ_Rep,
  ਖਾਲੀ *p,
  ਖਾਲੀ (*xSFunc)(sqlite3_context*,ਪੂਰਨ,sqlite3_ਮੁੱਲ**),
  ਖਾਲੀ (*x_ਕਦਮ)(sqlite3_context*,ਪੂਰਨ,sqlite3_ਮੁੱਲ**),
  ਖਾਲੀ (*xFinal)(sqlite3_context*)
){
  ਪੂਰਨ rc;
  ਅੱਖਰ *zFunc8;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !sqlite3SafetyCheckOk(db) || zFunctionName==0 ) ਵਾਪਸ SQLITE_MISUSE_BKPT;
#ਜੇ_ਅੰਤ
  sqlite3_mutex_enter(db->mutex);
  ਪ੍ਰਤਿਜ੍ਞਾ( !db->mallocFailed );
  zFunc8 = sqlite3Utf16to8(db, zFunctionName, -1, SQLITE_UTF16NATIVE);
  rc = sqlite3CreateFunc(db, zFunc8, nArg, e_ਪਾਠ_Rep, p, xSFunc,x_ਕਦਮ,xFinal,0,0,0);
  sqlite3DbFree(db, zFunc8);
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  ਵਾਪਸ rc;
}
#ਜੇ_ਅੰਤ


/*
** The following is the implementation of an SQL function that always
** fails with an error message stating that the function is used in the
** wrong context.  The sqlite3_overload_function() API might construct
** SQL function that use this routine so that the functions will exist
** for name resolution but are actually overloaded by the xFindFunction
** method of virtual tables.
*/
ਸਥਿਰ ਖਾਲੀ sqlite3InvalidFunction(
  sqlite3_context *context,  /* The function calling context */
  ਪੂਰਨ Not_ਵਰਤਿਆ,               /* Number of arguments to the function */
  sqlite3_ਮੁੱਲ **NotUsed2   /* Value of each argument */
){
  ਅਚਲ ਅੱਖਰ *zName = (ਅਚਲ ਅੱਖਰ*)sqlite3_user_ਅੰਕੜੇ(context);
  ਅੱਖਰ *zErr;
  ਅਣਵਰਤਿਆ_PARAMETER2(Not_ਵਰਤਿਆ, NotUsed2);
  zErr = sqlite3_mprintf(
      "unable to use function %s in the requested context", zName);
  sqlite3_ਨਤੀਜਾ_error(context, zErr, -1);
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
ਪੂਰਨ sqlite3_overload_function(
  sqlite3 *db,
  ਅਚਲ ਅੱਖਰ *zName,
  ਪੂਰਨ nArg
){
  ਪੂਰਨ rc;
  ਅੱਖਰ *zCopy;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !sqlite3SafetyCheckOk(db) || zName==0 || nArg<-2 ){
    ਵਾਪਸ SQLITE_MISUSE_BKPT;
  }
#ਜੇ_ਅੰਤ
  sqlite3_mutex_enter(db->mutex);
  rc = sqlite3_ਲੱਭੋ_Function(db, zName, nArg, SQLITE_UTF8, 0)!=0;
  sqlite3_mutex_leave(db->mutex);
  ਜੇ( rc ) ਵਾਪਸ SQLITE_OK;
  zCopy = sqlite3_mprintf("%s", zName);
  ਜੇ( zCopy==0 ) ਵਾਪਸ SQLITE_NOMEM;
  ਵਾਪਸ sqlite3_create_function_v2(db, zName, nArg, SQLITE_UTF8,
                           zCopy, sqlite3InvalidFunction, 0, 0, sqlite3_free);
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_TRACE
/*
** Register a trace function.  The pArg from the previously registered trace
** is returned. 
**
** A NULL trace function means that no tracing is executes.  A non-NULL
** trace is a pointer to a function that is invoked at the start of each
** SQL statement.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_DEPRECATED
ਖਾਲੀ *sqlite3_trace(sqlite3 *db, ਖਾਲੀ(*xTrace)(ਖਾਲੀ*,ਅਚਲ ਅੱਖਰ*), ਖਾਲੀ *pArg){
  ਖਾਲੀ *pOld;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !sqlite3SafetyCheckOk(db) ){
    (ਖਾਲੀ)SQLITE_MISUSE_BKPT;
    ਵਾਪਸ 0;
  }
#ਜੇ_ਅੰਤ
  sqlite3_mutex_enter(db->mutex);
  pOld = db->pTraceArg;
  db->mTrace = xTrace ? SQLITE_TRACE_LEGACY : 0;
  db->trace.xLegacy = xTrace;
  db->pTraceArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  ਵਾਪਸ pOld;
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_DEPRECATED */

/* Register a trace callback using the version-2 interface.
*/
ਪੂਰਨ sqlite3_trace_v2(
  sqlite3 *db,                               /* Trace this connection */
  ਬਿਨਾਨਿਸ਼ਾਨੀ mTrace,                           /* Mask of events to be traced */
  ਪੂਰਨ(*xTrace)(ਬਿਨਾਨਿਸ਼ਾਨੀ,ਖਾਲੀ*,ਖਾਲੀ*,ਖਾਲੀ*),  /* Callback to invoke */
  ਖਾਲੀ *pArg                                 /* Context */
){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !sqlite3SafetyCheckOk(db) ){
    ਵਾਪਸ SQLITE_MISUSE_BKPT;
  }
#ਜੇ_ਅੰਤ
  sqlite3_mutex_enter(db->mutex);
  ਜੇ( mTrace==0 ) xTrace = 0;
  ਜੇ( xTrace==0 ) mTrace = 0;
  db->mTrace = mTrace;
  db->trace.xV2 = xTrace;
  db->pTraceArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  ਵਾਪਸ SQLITE_OK;
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_DEPRECATED
/*
** Register a profile function.  The pArg from the previously registered
** profile function is returned. 
**
** A NULL profile function means that no profiling is executes.  A non-NULL
** profile is a pointer to a function that is invoked at the conclusion of
** each SQL statement that is run.
*/
ਖਾਲੀ *sqlite3_profile(
  sqlite3 *db,
  ਖਾਲੀ (*xProfile)(ਖਾਲੀ*,ਅਚਲ ਅੱਖਰ*,sqlite_uint64),
  ਖਾਲੀ *pArg
){
  ਖਾਲੀ *pOld;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !sqlite3SafetyCheckOk(db) ){
    (ਖਾਲੀ)SQLITE_MISUSE_BKPT;
    ਵਾਪਸ 0;
  }
#ਜੇ_ਅੰਤ
  sqlite3_mutex_enter(db->mutex);
  pOld = db->pProfileArg;
  db->xProfile = xProfile;
  db->pProfileArg = pArg;
  db->mTrace &= SQLITE_TRACE_NONLEGACY_ਨਕਾਬ;
  ਜੇ( db->xProfile ) db->mTrace |= SQLITE_TRACE_XPROFILE;
  sqlite3_mutex_leave(db->mutex);
  ਵਾਪਸ pOld;
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_DEPRECATED */
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_TRACE */

/*
** Register a function to be invoked when a transaction commits.
** If the invoked function returns non-zero, then the commit becomes a
** rollback.
*/
ਖਾਲੀ *sqlite3_commit_hook(
  sqlite3 *db,              /* Attach the hook to this database */
  ਪੂਰਨ (*xCallback)(ਖਾਲੀ*),  /* Function to invoke on each commit */
  ਖਾਲੀ *pArg                /* Argument to the function */
){
  ਖਾਲੀ *pOld;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !sqlite3SafetyCheckOk(db) ){
    (ਖਾਲੀ)SQLITE_MISUSE_BKPT;
    ਵਾਪਸ 0;
  }
#ਜੇ_ਅੰਤ
  sqlite3_mutex_enter(db->mutex);
  pOld = db->pCommitArg;
  db->xCommitCallback = xCallback;
  db->pCommitArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  ਵਾਪਸ pOld;
}

/*
** Register a callback to be invoked each time a row is updated,
** inserted or deleted using this database connection.
*/
ਖਾਲੀ *sqlite3_update_hook(
  sqlite3 *db,              /* Attach the hook to this database */
  ਖਾਲੀ (*xCallback)(ਖਾਲੀ*,ਪੂਰਨ,ਅੱਖਰ ਅਚਲ *,ਅੱਖਰ ਅਚਲ *,sqlite_int64),
  ਖਾਲੀ *pArg                /* Argument to the function */
){
  ਖਾਲੀ *pRet;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !sqlite3SafetyCheckOk(db) ){
    (ਖਾਲੀ)SQLITE_MISUSE_BKPT;
    ਵਾਪਸ 0;
  }
#ਜੇ_ਅੰਤ
  sqlite3_mutex_enter(db->mutex);
  pRet = db->pUpdateArg;
  db->xUpdateCallback = xCallback;
  db->pUpdateArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  ਵਾਪਸ pRet;
}

/*
** Register a callback to be invoked each time a transaction is rolled
** back by this database connection.
*/
ਖਾਲੀ *sqlite3_rollback_hook(
  sqlite3 *db,              /* Attach the hook to this database */
  ਖਾਲੀ (*xCallback)(ਖਾਲੀ*), /* Callback function */
  ਖਾਲੀ *pArg                /* Argument to the function */
){
  ਖਾਲੀ *pRet;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !sqlite3SafetyCheckOk(db) ){
    (ਖਾਲੀ)SQLITE_MISUSE_BKPT;
    ਵਾਪਸ 0;
  }
#ਜੇ_ਅੰਤ
  sqlite3_mutex_enter(db->mutex);
  pRet = db->pRollbackArg;
  db->xRollbackCallback = xCallback;
  db->pRollbackArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  ਵਾਪਸ pRet;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_PREUPDATE_HOOK
/*
** Register a callback to be invoked each time a row is updated,
** inserted or deleted using this database connection.
*/
ਖਾਲੀ *sqlite3_preupdate_hook(
  sqlite3 *db,              /* Attach the hook to this database */
  ਖਾਲੀ(*xCallback)(         /* Callback function */
    ਖਾਲੀ*,sqlite3*,ਪੂਰਨ,ਅੱਖਰ ਅਚਲ*,ਅੱਖਰ ਅਚਲ*,sqlite3_int64,sqlite3_int64),
  ਖਾਲੀ *pArg                /* First callback argument */
){
  ਖਾਲੀ *pRet;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( db==0 ){
    ਵਾਪਸ 0;
  }
#ਜੇ_ਅੰਤ
  sqlite3_mutex_enter(db->mutex);
  pRet = db->pPreUpdateArg;
  db->xPreUpdateCallback = xCallback;
  db->pPreUpdateArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  ਵਾਪਸ pRet;
}
#ਜੇ_ਅੰਤ /* SQLITE_ENABLE_PREUPDATE_HOOK */

/*
** Register a function to be invoked prior to each autovacuum that
** determines the number of pages to vacuum.
*/
ਪੂਰਨ sqlite3_autovacuum_ਪੰਨੇ(
  sqlite3 *db,                 /* Attach the hook to this database */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ (*xCallback)(ਖਾਲੀ*,ਅਚਲ ਅੱਖਰ*,u32,u32,u32),
  ਖਾਲੀ *pArg,                  /* Argument to the function */
  ਖਾਲੀ (*xDestructor)(ਖਾਲੀ*)   /* Destructor for pArg */
){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !sqlite3SafetyCheckOk(db) ){
    ਜੇ( xDestructor ) xDestructor(pArg);
    ਵਾਪਸ SQLITE_MISUSE_BKPT;
  }
#ਜੇ_ਅੰਤ
  sqlite3_mutex_enter(db->mutex);
  ਜੇ( db->xAutovacDestr ){
    db->xAutovacDestr(db->p_Autovac_ਪੰਨੇ_Arg);
  }
  db->x_Autovac_ਪੰਨੇ = xCallback;
  db->p_Autovac_ਪੰਨੇ_Arg = pArg;
  db->xAutovacDestr = xDestructor;
  sqlite3_mutex_leave(db->mutex);
  ਵਾਪਸ SQLITE_OK;
}


#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WAL
/*
** The sqlite3_wal_hook() callback registered by sqlite3_wal_autocheckpoint().
** Invoke sqlite3_wal_checkpoint if the number of frames in the log file
** is greater than sqlite3.pWalArg cast to an integer (the value configured by
** wal_autocheckpoint()).
*/
ਪੂਰਨ sqlite3WalDefaultHook(
  ਖਾਲੀ *p_Client_ਅੰਕੜੇ,     /*ਦਲੀਲ*/
  sqlite3 *db,           /* Connection */
  ਅਚਲ ਅੱਖਰ *zDb,       /* Database */
  ਪੂਰਨ n_ਚੌਖਟਾ             /* Size of WAL */
){
  ਜੇ( n_ਚੌਖਟਾ>=SQLITE_PTR_ਮੰਜ਼ਿਲ_INT(p_Client_ਅੰਕੜੇ) ){
    sqlite3BeginBenignMalloc();
    sqlite3_wal_checkpoint(db, zDb);
    sqlite3_ਸਿਰਾ_Benign_Malloc();
  }
  ਵਾਪਸ SQLITE_OK;
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_WAL */

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
ਪੂਰਨ sqlite3_wal_autocheckpoint(sqlite3 *db, ਪੂਰਨ n_ਚੌਖਟਾ){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WAL
  ਅਣਵਰਤਿਆ_PARAMETER(db);
  ਅਣਵਰਤਿਆ_PARAMETER(n_ਚੌਖਟਾ);
#ਨਹੀਂਤਾਂ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !sqlite3SafetyCheckOk(db) ) ਵਾਪਸ SQLITE_MISUSE_BKPT;
#ਜੇ_ਅੰਤ
  ਜੇ( n_ਚੌਖਟਾ>0 ){
    sqlite3_wal_hook(db, sqlite3WalDefaultHook, SQLITE_INT_ਮੰਜ਼ਿਲ_PTR(n_ਚੌਖਟਾ));
  }ਨਹੀਂਤਾਂ{
    sqlite3_wal_hook(db, 0, 0);
  }
#ਜੇ_ਅੰਤ
  ਵਾਪਸ SQLITE_OK;
}

/*
** Register a callback to be invoked each time a transaction is written
** into the write-ahead-log by this database connection.
*/
ਖਾਲੀ *sqlite3_wal_hook(
  sqlite3 *db,                    /* Attach the hook to this db handle */
  ਪੂਰਨ(*xCallback)(ਖਾਲੀ *, sqlite3*, ਅਚਲ ਅੱਖਰ*, ਪੂਰਨ),
  ਖਾਲੀ *pArg                      /* First argument passed to xCallback() */
){
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WAL
  ਖਾਲੀ *pRet;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !sqlite3SafetyCheckOk(db) ){
    (ਖਾਲੀ)SQLITE_MISUSE_BKPT;
    ਵਾਪਸ 0;
  }
#ਜੇ_ਅੰਤ
  sqlite3_mutex_enter(db->mutex);
  pRet = db->pWalArg;
  db->xWalCallback = xCallback;
  db->pWalArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  ਵਾਪਸ pRet;
#ਨਹੀਂਤਾਂ
  ਅਣਵਰਤਿਆ_PARAMETER(db);
  ਅਣਵਰਤਿਆ_PARAMETER(xCallback);
  ਅਣਵਰਤਿਆ_PARAMETER(pArg);
  ਵਾਪਸ 0;
#ਜੇ_ਅੰਤ
}

/*
** Checkpoint database zDb.
*/
ਪੂਰਨ sqlite3_wal_checkpoint_v2(
  sqlite3 *db,                    /* Database handle */
  ਅਚਲ ਅੱਖਰ *zDb,                /* Name of attached database (or NULL) */
  ਪੂਰਨ e_ਬਹੁਲਕ,                      /* SQLITE_CHECKPOINT_* value */
  ਪੂਰਨ *pnLog,                     /* OUT: Size of WAL log in frames */
  ਪੂਰਨ *pnCkpt                     /* OUT: Total number of frames checkpointed */
){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WAL
  ਅਣਵਰਤਿਆ_PARAMETER(db);
  ਅਣਵਰਤਿਆ_PARAMETER(zDb);
  ਅਣਵਰਤਿਆ_PARAMETER(e_ਬਹੁਲਕ);
  ਅਣਵਰਤਿਆ_PARAMETER(pnLog);
  ਅਣਵਰਤਿਆ_PARAMETER(pnCkpt);
  ਵਾਪਸ SQLITE_OK;
#ਨਹੀਂਤਾਂ
  ਪੂਰਨ rc;                         /* Return code */
  ਪੂਰਨ iDb;                        /* Schema to checkpoint */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !sqlite3SafetyCheckOk(db) ) ਵਾਪਸ SQLITE_MISUSE_BKPT;
#ਜੇ_ਅੰਤ

  /* Initialize the output variables to -1 in case an error occurs. */
  ਜੇ( pnLog ) *pnLog = -1;
  ਜੇ( pnCkpt ) *pnCkpt = -1;

  ਪ੍ਰਤਿਜ੍ਞਾ( SQLITE_CHECKPOINT_NOOP==-1 );
  ਪ੍ਰਤਿਜ੍ਞਾ( SQLITE_CHECKPOINT_PASSIVE==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( SQLITE_CHECKPOINT_ਭਰਿਆ==1 );
  ਪ੍ਰਤਿਜ੍ਞਾ( SQLITE_CHECKPOINT_RESTART==2 );
  ਪ੍ਰਤਿਜ੍ਞਾ( SQLITE_CHECKPOINT_TRUNCATE==3 );
  ਜੇ( e_ਬਹੁਲਕ<SQLITE_CHECKPOINT_NOOP || e_ਬਹੁਲਕ>SQLITE_CHECKPOINT_TRUNCATE ){
    /* EVIDENCE-OF: R-03996-12088 The M parameter must be a valid checkpoint
    ** mode: */
    ਵਾਪਸ SQLITE_MISUSE_BKPT;
  }

  sqlite3_mutex_enter(db->mutex);
  ਜੇ( zDb && zDb[0] ){
    iDb = sqlite3_ਲੱਭੋ_Db_Name(db, zDb);
  }ਨਹੀਂਤਾਂ{
    iDb = SQLITE_MAX_DB;   /* This means process all schemas */
  }
  ਜੇ( iDb<0 ){
    rc = SQLITE_ERROR;
    sqlite3_Error_With_ਸੁਨੇਹਾ(db, SQLITE_ERROR, "unknown database: %s", zDb);
  }ਨਹੀਂਤਾਂ{
    db->busyHandler.nBusy = 0;
    rc = sqlite3Checkpoint(db, iDb, e_ਬਹੁਲਕ, pnLog, pnCkpt);
    sqlite3Error(db, rc);
  }
  rc = sqlite3ApiExit(db, rc);

  /* If there are no active statements, clear the interrupt flag at this
  ** point.  */
  ਜੇ( db->n_Vdbe_ਸਰਗਰਮ==0 ){
    AtomicStore(&db->u1.isInterrupted, 0);
  }

  sqlite3_mutex_leave(db->mutex);
  ਵਾਪਸ rc;
#ਜੇ_ਅੰਤ
}


/*
** Checkpoint database zDb. If zDb is NULL, or if the buffer zDb points
** to contains a zero-length string, all attached databases are
** checkpointed.
*/
ਪੂਰਨ sqlite3_wal_checkpoint(sqlite3 *db, ਅਚਲ ਅੱਖਰ *zDb){
  /* EVIDENCE-OF: R-41613-20553 The sqlite3_wal_checkpoint(D,X) is equivalent to
  ** sqlite3_wal_checkpoint_v2(D,X,SQLITE_CHECKPOINT_PASSIVE,0,0). */
  ਵਾਪਸ sqlite3_wal_checkpoint_v2(db,zDb,SQLITE_CHECKPOINT_PASSIVE,0,0);
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WAL
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
ਪੂਰਨ sqlite3Checkpoint(sqlite3 *db, ਪੂਰਨ iDb, ਪੂਰਨ e_ਬਹੁਲਕ, ਪੂਰਨ *pnLog, ਪੂਰਨ *pnCkpt){
  ਪੂਰਨ rc = SQLITE_OK;             /* Return code */
  ਪੂਰਨ i;                          /* Used to iterate through attached dbs */
  ਪੂਰਨ bBusy = 0;                  /* True if SQLITE_BUSY has been encountered */

  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(db->mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( !pnLog || *pnLog==-1 );
  ਪ੍ਰਤਿਜ੍ਞਾ( !pnCkpt || *pnCkpt==-1 );
  testcase( iDb==SQLITE_MAX_ATTACHED ); /* See forum post a006d86f72 */
  testcase( iDb==SQLITE_MAX_DB );

  ਲਈ(i=0; i<db->nDb && rc==SQLITE_OK; i++){
    ਜੇ( i==iDb || iDb==SQLITE_MAX_DB ){
      rc = sqlite3BtreeCheckpoint(db->aDb[i].pBt, e_ਬਹੁਲਕ, pnLog, pnCkpt);
      pnLog = 0;
      pnCkpt = 0;
      ਜੇ( rc==SQLITE_BUSY ){
        bBusy = 1;
        rc = SQLITE_OK;
      }
    }
  }

  ਵਾਪਸ (rc==SQLITE_OK && bBusy) ? SQLITE_BUSY : rc;
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_WAL */

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
ਪੂਰਨ sqlite3_ਆਰਜ਼ੀ_ਵਿੱਚ_ਸਮ੍ਰਿਤੀ(ਅਚਲ sqlite3 *db){
#ਜੇ SQLITE_ਆਰਜ਼ੀ_STORE==1
  ਵਾਪਸ ( db->ਆਰਜ਼ੀ_store==2 );
#ਜੇ_ਅੰਤ
#ਜੇ SQLITE_ਆਰਜ਼ੀ_STORE==2
  ਵਾਪਸ ( db->ਆਰਜ਼ੀ_store!=1 );
#ਜੇ_ਅੰਤ
#ਜੇ SQLITE_ਆਰਜ਼ੀ_STORE==3
  ਅਣਵਰਤਿਆ_PARAMETER(db);
  ਵਾਪਸ 1;
#ਜੇ_ਅੰਤ
#ਜੇ SQLITE_ਆਰਜ਼ੀ_STORE<1 || SQLITE_ਆਰਜ਼ੀ_STORE>3
  ਅਣਵਰਤਿਆ_PARAMETER(db);
  ਵਾਪਸ 0;
#ਜੇ_ਅੰਤ
}

/*
** Return UTF-8 encoded English language explanation of the most recent
** error.
*/
ਅਚਲ ਅੱਖਰ *sqlite3_errmsg(sqlite3 *db){
  ਅਚਲ ਅੱਖਰ *z;
  ਜੇ( !db ){
    ਵਾਪਸ sqlite3ErrStr(SQLITE_NOMEM_BKPT);
  }
  ਜੇ( !sqlite3SafetyCheckSickOrOk(db) ){
    ਵਾਪਸ sqlite3ErrStr(SQLITE_MISUSE_BKPT);
  }
  sqlite3_mutex_enter(db->mutex);
  ਜੇ( db->mallocFailed ){
    z = sqlite3ErrStr(SQLITE_NOMEM_BKPT);
  }ਨਹੀਂਤਾਂ{
    testcase( db->pErr==0 );
    z = db->err_ਕੋਡ ? (ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(db->pErr) : 0;
    ਪ੍ਰਤਿਜ੍ਞਾ( !db->mallocFailed );
    ਜੇ( z==0 ){
      z = sqlite3ErrStr(db->err_ਕੋਡ);
    }
  }
  sqlite3_mutex_leave(db->mutex);
  ਵਾਪਸ z;
}

/*
** Set the error code and error message associated with the database handle.
**
** This routine is intended to be called by outside extensions (ex: the
** Session extension). Internal logic should invoke sqlite3Error() or
** sqlite3ErrorWithMsg() directly.
*/
ਪੂਰਨ sqlite3_ਸੈੱਟ_errmsg(sqlite3 *db, ਪੂਰਨ errcode, ਅਚਲ ਅੱਖਰ *z_ਸੁਨੇਹਾ){
  ਪੂਰਨ rc = SQLITE_OK;
  ਜੇ( !sqlite3SafetyCheckOk(db) ){
    ਵਾਪਸ SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(db->mutex);
  ਜੇ( z_ਸੁਨੇਹਾ ){
    sqlite3_Error_With_ਸੁਨੇਹਾ(db, errcode, "%s", z_ਸੁਨੇਹਾ);
  }ਨਹੀਂਤਾਂ{
    sqlite3Error(db, errcode);
  }
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  ਵਾਪਸ rc;
}

/*
** Return the byte offset of the most recent error
*/
ਪੂਰਨ sqlite3_error_ਵਿੱਥ(sqlite3 *db){
  ਪੂਰਨ i_ਵਿੱਥ = -1;
  ਜੇ( db && sqlite3SafetyCheckSickOrOk(db) ){
    sqlite3_mutex_enter(db->mutex);
    ਜੇ( db->err_ਕੋਡ ){
      i_ਵਿੱਥ = db->err_Byte_ਵਿੱਥ;
    }
    sqlite3_mutex_leave(db->mutex);
  }
  ਵਾਪਸ i_ਵਿੱਥ;
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_UTF16
/*
** Return UTF-16 encoded English language explanation of the most recent
** error.
*/
ਅਚਲ ਖਾਲੀ *sqlite3_errmsg16(sqlite3 *db){
  ਸਥਿਰ ਅਚਲ u16 ਬਾਹਰਲਾ_ਦਾ_Mem[] = {
    'o', 'u', 't', ' ', 'o', 'f', ' ', 'm', 'e', 'm', 'o', 'r', 'y', 0
  };
  ਸਥਿਰ ਅਚਲ u16 misuse[] = {
    'b', 'a', 'd', ' ', 'p', 'a', 'r', 'a', 'm', 'e', 't', 'e', 'r', ' ',
    'o', 'r', ' ', 'o', 't', 'h', 'e', 'r', ' ', 'A', 'P', 'I', ' ',
    'm', 'i', 's', 'u', 's', 'e', 0
  };

  ਅਚਲ ਖਾਲੀ *z;
  ਜੇ( !db ){
    ਵਾਪਸ (ਖਾਲੀ *)ਬਾਹਰਲਾ_ਦਾ_Mem;
  }
  ਜੇ( !sqlite3SafetyCheckSickOrOk(db) ){
    ਵਾਪਸ (ਖਾਲੀ *)misuse;
  }
  sqlite3_mutex_enter(db->mutex);
  ਜੇ( db->mallocFailed ){
    z = (ਖਾਲੀ *)ਬਾਹਰਲਾ_ਦਾ_Mem;
  }ਨਹੀਂਤਾਂ{
    z = sqlite3_ਮੁੱਲ_text16(db->pErr);
    ਜੇ( z==0 ){
      sqlite3_Error_With_ਸੁਨੇਹਾ(db, db->err_ਕੋਡ, sqlite3ErrStr(db->err_ਕੋਡ));
      z = sqlite3_ਮੁੱਲ_text16(db->pErr);
    }
    /* A malloc() may have failed within the call to sqlite3_value_text16()
    ** above. If this is the case, then the db->mallocFailed flag needs to
    ** be cleared before returning. Do this directly, instead of via
    ** sqlite3ApiExit(), to avoid setting the database handle error message.
    */
    sqlite3OomClear(db);
  }
  sqlite3_mutex_leave(db->mutex);
  ਵਾਪਸ z;
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_UTF16 */

/*
** Return the most recent error code generated by an SQLite routine. If NULL is
** passed to this function, we assume a malloc() failed during sqlite3_open().
*/
ਪੂਰਨ sqlite3_errcode(sqlite3 *db){
  ਪੂਰਨ iRet;
  ਜੇ( !db ) ਵਾਪਸ SQLITE_NOMEM_BKPT;
  ਜੇ( !sqlite3SafetyCheckSickOrOk(db) ){
    ਵਾਪਸ SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(db->mutex);
  ਜੇ( db->mallocFailed ){
    iRet = SQLITE_NOMEM_BKPT;
  }ਨਹੀਂਤਾਂ{
    iRet = db->err_ਕੋਡ & db->err_ਨਕਾਬ;
  }
  sqlite3_mutex_leave(db->mutex);
  ਵਾਪਸ iRet;
}
ਪੂਰਨ sqlite3_extended_errcode(sqlite3 *db){
  ਪੂਰਨ iRet;
  ਜੇ( !db ) ਵਾਪਸ SQLITE_NOMEM_BKPT;
  ਜੇ( !sqlite3SafetyCheckSickOrOk(db) ){
    ਵਾਪਸ SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(db->mutex);
  ਜੇ( db->mallocFailed ){
    iRet = SQLITE_NOMEM_BKPT;
  }ਨਹੀਂਤਾਂ{
    iRet = db->err_ਕੋਡ;
  }
  sqlite3_mutex_leave(db->mutex);
  ਵਾਪਸ iRet;
}
ਪੂਰਨ sqlite3_system_errno(sqlite3 *db){
  ਪੂਰਨ iRet = 0;
  ਜੇ( db ){
    sqlite3_mutex_enter(db->mutex);
    iRet = db->iSysErrno;
    sqlite3_mutex_leave(db->mutex);
  }
  ਵਾਪਸ iRet;
} 

/*
** Return a string that describes the kind of error specified in the
** argument.  For now, this simply calls the internal sqlite3ErrStr()
** function.
*/
ਅਚਲ ਅੱਖਰ *sqlite3_errstr(ਪੂਰਨ rc){
  ਵਾਪਸ sqlite3ErrStr(rc);
}

/*
** Create a new collating function for database "db".  The name is zName
** and the encoding is enc.
*/
ਸਥਿਰ ਪੂਰਨ createCollation(
  sqlite3* db,
  ਅਚਲ ਅੱਖਰ *zName,
  u8 enc,
  ਖਾਲੀ* pCtx,
  ਪੂਰਨ(*xCompare)(ਖਾਲੀ*,ਪੂਰਨ,ਅਚਲ ਖਾਲੀ*,ਪੂਰਨ,ਅਚਲ ਖਾਲੀ*),
  ਖਾਲੀ(*xDel)(ਖਾਲੀ*)
){
  CollSeq *pColl;
  ਪੂਰਨ enc2;
 
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(db->mutex) );

  /* If SQLITE_UTF16 is specified as the encoding type, transform this
  ** to one of SQLITE_UTF16LE or SQLITE_UTF16BE using the
  ** SQLITE_UTF16NATIVE macro. SQLITE_UTF16 is not used internally.
  */
  enc2 = enc;
  testcase( enc2==SQLITE_UTF16 );
  testcase( enc2==SQLITE_UTF16_ALIGNED );
  ਜੇ( enc2==SQLITE_UTF16 || enc2==SQLITE_UTF16_ALIGNED ){
    enc2 = SQLITE_UTF16NATIVE;
  }
  ਜੇ( enc2<SQLITE_UTF8 || enc2>SQLITE_UTF16BE ){
    ਵਾਪਸ SQLITE_MISUSE_BKPT;
  }

  /* Check if this call is removing or replacing an existing collation
  ** sequence. If so, and there are active VMs, return busy. If there
  ** are no active VMs, invalidate any pre-compiled statements.
  */
  pColl = sqlite3_ਲੱਭੋ_Coll_Seq(db, (u8)enc2, zName, 0);
  ਜੇ( pColl && pColl->xCmp ){
    ਜੇ( db->n_Vdbe_ਸਰਗਰਮ ){
      sqlite3_Error_With_ਸੁਨੇਹਾ(db, SQLITE_BUSY,
        "unable to delete/modify collation sequence due to active statements");
      ਵਾਪਸ SQLITE_BUSY;
    }
    sqlite3ExpirePreparedStatements(db, 0);

    /* If collation sequence pColl was created directly by a call to
    ** sqlite3_create_collation, and not generated by synthCollSeq(),
    ** then any copies made by synthCollSeq() need to be invalidated.
    ** Also, collation destructor - CollSeq.xDel() - function may need
    ** to be called.
    */
    ਜੇ( (pColl->enc & ~SQLITE_UTF16_ALIGNED)==enc2 ){
      CollSeq *aColl = sqlite3_Hash_ਲੱਭੋ(&db->aCollSeq, zName);
      ਪੂਰਨ j;
      ਲਈ(j=0; j<3; j++){
        CollSeq *p = &aColl[j];
        ਜੇ( p->enc==pColl->enc ){
          ਜੇ( p->xDel ){
            p->xDel(p->pUser);
          }
          p->xCmp = 0;
        }
      }
    }
  }

  pColl = sqlite3_ਲੱਭੋ_Coll_Seq(db, (u8)enc2, zName, 1);
  ਜੇ( pColl==0 ) ਵਾਪਸ SQLITE_NOMEM_BKPT;
  pColl->xCmp = xCompare;
  pColl->pUser = pCtx;
  pColl->xDel = xDel;
  pColl->enc = (u8)(enc2 | (enc & SQLITE_UTF16_ALIGNED));
  sqlite3Error(db, SQLITE_OK);
  ਵਾਪਸ SQLITE_OK;
}


/*
** This array defines hard upper bounds on limit values.  The
** initializer must be kept in sync with the SQLITE_LIMIT_*
** #defines in sqlite3.h.
*/
ਸਥਿਰ ਅਚਲ ਪੂਰਨ a_Hard_ਹੱਦਬੰਦੀ[] = {
  SQLITE_MAX_ਲੰਮਾਈ,
  SQLITE_MAX_SQL_ਲੰਮਾਈ,
  SQLITE_MAX_ਕਾਲਮ,
  SQLITE_MAX_ਵਿਅੰਜਕ_ਡੂੰਘਾਈ,
  SQLITE_MAX_COMPOUND_SELECT,
  SQLITE_MAX_VDBE_OP,
  SQLITE_MAX_FUNCTION_ARG,
  SQLITE_MAX_ATTACHED,
  SQLITE_MAX_LIKE_ਨਮੂਨਾ_ਲੰਮਾਈ,
  SQLITE_MAX_VARIABLE_ਸੰਖਿਆ,      /* IMP: R-38091-32352 */
  SQLITE_MAX_TRIGGER_ਡੂੰਘਾਈ,
  SQLITE_MAX_ਕਾਮਾ_ਧਾਗੇ,
  SQLITE_MAX_PARSER_ਡੂੰਘਾਈ,
  SQLITE_MAX_SCHEMA,
  SQLITE_MAX_TRIGGER_ਕਦਮਾਂ,
  SQLITE_MAX_CTES,
};

/*
** Make sure the hard limits are set to reasonable values
*/
#ਜੇ SQLITE_MAX_ਲੰਮਾਈ<100
# error SQLITE_MAX_ਲੰਮਾਈ must be at ਘੱਟ_ਤੋਂ_ਘੱਟ 100
#ਜੇ_ਅੰਤ
#ਜੇ SQLITE_MAX_SQL_ਲੰਮਾਈ<100
# error SQLITE_MAX_SQL_ਲੰਮਾਈ must be at ਘੱਟ_ਤੋਂ_ਘੱਟ 100
#ਜੇ_ਅੰਤ
#ਜੇ SQLITE_MAX_SQL_ਲੰਮਾਈ>SQLITE_MAX_ਲੰਮਾਈ
# error SQLITE_MAX_SQL_ਲੰਮਾਈ must not be greater than SQLITE_MAX_ਲੰਮਾਈ
#ਜੇ_ਅੰਤ
#ਜੇ SQLITE_MAX_SQL_ਲੰਮਾਈ>2147482624     /* 1024 less than 2^31 */
# error SQLITE_MAX_SQL_ਲੰਮਾਈ must not be greater than 2147482624
#ਜੇ_ਅੰਤ
#ਜੇ SQLITE_MAX_COMPOUND_SELECT<2
# error SQLITE_MAX_COMPOUND_SELECT must be at ਘੱਟ_ਤੋਂ_ਘੱਟ 2
#ਜੇ_ਅੰਤ
#ਜੇ SQLITE_MAX_VDBE_OP<40
# error SQLITE_MAX_VDBE_OP must be at ਘੱਟ_ਤੋਂ_ਘੱਟ 40
#ਜੇ_ਅੰਤ
#ਜੇ SQLITE_MAX_FUNCTION_ARG<0 || SQLITE_MAX_FUNCTION_ARG>32767
# error SQLITE_MAX_FUNCTION_ARG must be between 0 and 32767
#ਜੇ_ਅੰਤ
#ਜੇ SQLITE_MAX_ATTACHED<0 || SQLITE_MAX_ATTACHED>125
# error SQLITE_MAX_ATTACHED must be between 0 and 125
#ਜੇ_ਅੰਤ
#ਜੇ SQLITE_MAX_LIKE_ਨਮੂਨਾ_ਲੰਮਾਈ<1
# error SQLITE_MAX_LIKE_ਨਮੂਨਾ_ਲੰਮਾਈ must be at ਘੱਟ_ਤੋਂ_ਘੱਟ 1
#ਜੇ_ਅੰਤ
#ਜੇ SQLITE_MAX_ਕਾਲਮ>32767
# error SQLITE_MAX_ਕਾਲਮ must not exceed 32767
#ਜੇ_ਅੰਤ
#ਜੇ SQLITE_MAX_TRIGGER_ਡੂੰਘਾਈ<1
# error SQLITE_MAX_TRIGGER_ਡੂੰਘਾਈ must be at ਘੱਟ_ਤੋਂ_ਘੱਟ 1
#ਜੇ_ਅੰਤ
#ਜੇ SQLITE_MAX_ਕਾਮਾ_ਧਾਗੇ<0 || SQLITE_MAX_ਕਾਮਾ_ਧਾਗੇ>50
# error SQLITE_MAX_ਕਾਮਾ_ਧਾਗੇ must be between 0 and 50
#ਜੇ_ਅੰਤ
#ਜੇ SQLITE_MAX_TRIGGER_ਕਦਮਾਂ<0 || SQLITE_MAX_TRIGGER_ਕਦਮਾਂ>65535
# error SQLITE_MAX_TRIGGER_ਕਦਮਾਂ must be between 0 and 65535
#ਜੇ_ਅੰਤ


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
ਪੂਰਨ sqlite3_ਹੱਦਬੰਦੀ(sqlite3 *db, ਪੂਰਨ ਹੱਦਬੰਦੀ_Id, ਪੂਰਨ new_ਹੱਦਬੰਦੀ){
  ਪੂਰਨ old_ਹੱਦਬੰਦੀ;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !sqlite3SafetyCheckOk(db) ){
    (ਖਾਲੀ)SQLITE_MISUSE_BKPT;
    ਵਾਪਸ -1;
  }
#ਜੇ_ਅੰਤ

  /* EVIDENCE-OF: R-30189-54097 For each limit category SQLITE_LIMIT_NAME
  ** there is a hard upper bound set at compile-time by a C preprocessor
  ** macro called SQLITE_MAX_NAME. (The "_LIMIT_" in the name is changed to
  ** "_MAX_".)
  */
  ਪ੍ਰਤਿਜ੍ਞਾ( a_Hard_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_ਲੰਮਾਈ]==SQLITE_MAX_ਲੰਮਾਈ );
  ਪ੍ਰਤਿਜ੍ਞਾ( a_Hard_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_SQL_ਲੰਮਾਈ]==SQLITE_MAX_SQL_ਲੰਮਾਈ );
  ਪ੍ਰਤਿਜ੍ਞਾ( a_Hard_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_ਕਾਲਮ]==SQLITE_MAX_ਕਾਲਮ );
  ਪ੍ਰਤਿਜ੍ਞਾ( a_Hard_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_ਵਿਅੰਜਕ_ਡੂੰਘਾਈ]==SQLITE_MAX_ਵਿਅੰਜਕ_ਡੂੰਘਾਈ );
  ਪ੍ਰਤਿਜ੍ਞਾ( a_Hard_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_PARSER_ਡੂੰਘਾਈ]==SQLITE_MAX_PARSER_ਡੂੰਘਾਈ );
  ਪ੍ਰਤਿਜ੍ਞਾ( a_Hard_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_COMPOUND_SELECT]==SQLITE_MAX_COMPOUND_SELECT);
  ਪ੍ਰਤਿਜ੍ਞਾ( a_Hard_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_VDBE_OP]==SQLITE_MAX_VDBE_OP );
  ਪ੍ਰਤਿਜ੍ਞਾ( a_Hard_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_FUNCTION_ARG]==SQLITE_MAX_FUNCTION_ARG );
  ਪ੍ਰਤਿਜ੍ਞਾ( a_Hard_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_ATTACHED]==SQLITE_MAX_ATTACHED );
  ਪ੍ਰਤਿਜ੍ਞਾ( a_Hard_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_LIKE_ਨਮੂਨਾ_ਲੰਮਾਈ]==
                                               SQLITE_MAX_LIKE_ਨਮੂਨਾ_ਲੰਮਾਈ );
  ਪ੍ਰਤਿਜ੍ਞਾ( a_Hard_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_VARIABLE_ਸੰਖਿਆ]==SQLITE_MAX_VARIABLE_ਸੰਖਿਆ);
  ਪ੍ਰਤਿਜ੍ਞਾ( a_Hard_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_TRIGGER_ਡੂੰਘਾਈ]==SQLITE_MAX_TRIGGER_ਡੂੰਘਾਈ );
  ਪ੍ਰਤਿਜ੍ਞਾ( a_Hard_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_ਕਾਮਾ_ਧਾਗੇ]==SQLITE_MAX_ਕਾਮਾ_ਧਾਗੇ );
  ਪ੍ਰਤਿਜ੍ਞਾ( a_Hard_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_SCHEMA]==SQLITE_MAX_SCHEMA );
  ਪ੍ਰਤਿਜ੍ਞਾ( a_Hard_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_TRIGGER_ਕਦਮਾਂ]==SQLITE_MAX_TRIGGER_ਕਦਮਾਂ );
  ਪ੍ਰਤਿਜ੍ਞਾ( a_Hard_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_CTES]==SQLITE_MAX_CTES );
  ਪ੍ਰਤਿਜ੍ਞਾ( SQLITE_ਹੱਦਬੰਦੀ_CTES==(SQLITE_N_ਹੱਦਬੰਦੀ-1) );


  ਜੇ( ਹੱਦਬੰਦੀ_Id<0 || ਹੱਦਬੰਦੀ_Id>=SQLITE_N_ਹੱਦਬੰਦੀ ){
    ਵਾਪਸ -1;
  }
  sqlite3_mutex_enter(db->mutex);
  old_ਹੱਦਬੰਦੀ = db->a_ਹੱਦਬੰਦੀ[ਹੱਦਬੰਦੀ_Id];
  ਜੇ( new_ਹੱਦਬੰਦੀ>=0 ){                   /* IMP: R-52476-28732 */
    ਜੇ( new_ਹੱਦਬੰਦੀ>a_Hard_ਹੱਦਬੰਦੀ[ਹੱਦਬੰਦੀ_Id] ){
      new_ਹੱਦਬੰਦੀ = a_Hard_ਹੱਦਬੰਦੀ[ਹੱਦਬੰਦੀ_Id];  /* IMP: R-51463-25634 */
    }ਨਹੀਂਤਾਂ ਜੇ( new_ਹੱਦਬੰਦੀ<SQLITE_MIN_ਲੰਮਾਈ && ਹੱਦਬੰਦੀ_Id==SQLITE_ਹੱਦਬੰਦੀ_ਲੰਮਾਈ ){
      new_ਹੱਦਬੰਦੀ = SQLITE_MIN_ਲੰਮਾਈ;
    }
    db->a_ਹੱਦਬੰਦੀ[ਹੱਦਬੰਦੀ_Id] = new_ਹੱਦਬੰਦੀ;
  }
  sqlite3_mutex_leave(db->mutex);
  ਵਾਪਸ old_ਹੱਦਬੰਦੀ;                     /* IMP: R-53341-35419 */
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
ਪੂਰਨ sqlite3ParseUri(
  ਅਚਲ ਅੱਖਰ *zDefaultVfs,        /* VFS to use if no "vfs=xxx" query option */
  ਅਚਲ ਅੱਖਰ *zUri,               /* Nul-terminated URI to parse */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *pFlags,           /* IN/OUT: SQLITE_OPEN_XXX flags */
  sqlite3_vfs **ppVfs,            /* OUT: VFS to use */
  ਅੱਖਰ **pzFile,                  /* OUT: Filename component of URI */
  ਅੱਖਰ **pz_Err_ਸੁਨੇਹਾ                 /* OUT: Error message (if rc!=SQLITE_OK) */
){
  ਪੂਰਨ rc = SQLITE_OK;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ flags = *pFlags;
  ਅਚਲ ਅੱਖਰ *zVfs = zDefaultVfs;
  ਅੱਖਰ *zFile;
  ਅੱਖਰ c;
  i64 nUri = ਲੜੀ_ਲੰਬਾਈ(zUri);

  ਪ੍ਰਤਿਜ੍ਞਾ( *pz_Err_ਸੁਨੇਹਾ==0 );

  ਜੇ( ((flags & SQLITE_OPEN_URI)                     /* IMP: R-48725-32206 */
       || Atomic_ਲੱਦੋ(&sqlite3GlobalConfig.bOpenUri)) /* IMP: R-51689-46548 */
   && nUri>=5 && memcmp(zUri, "file:", 5)==0         /* IMP: R-57884-37496 */
  ){
    ਅੱਖਰ *zOpt;
    ਪੂਰਨ eState;                   /* Parser state when parsing URI */
    i64 i_ਵਿੱਚ;                      /* Input character index */
    i64 i_ਬਾਹਰਲਾ = 0;                 /* Output character index */
    u64 nByte = nUri+8;           /* Bytes of space to allocate */

    /* Make sure the SQLITE_OPEN_URI flag is set to indicate to the VFS xOpen
    ** method that there may be extra parameters following the file-name.  */
    flags |= SQLITE_OPEN_URI;

    ਲਈ(i_ਵਿੱਚ=0; i_ਵਿੱਚ<nUri; i_ਵਿੱਚ++) nByte += (zUri[i_ਵਿੱਚ]=='&');
    zFile = sqlite3_malloc64(nByte);
    ਜੇ( !zFile ) ਵਾਪਸ SQLITE_NOMEM_BKPT;

    memset(zFile, 0, 4);  /* 4-byte of 0x00 is the start of DB name marker */
    zFile += 4;

    i_ਵਿੱਚ = 5;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ALLOW_URI_AUTHORITY
    ਜੇ( ਲੜੀ_ਤੁਲਨਾ(zUri+5, "///", 3)==0 ){
      i_ਵਿੱਚ = 7;
      /* The following condition causes URIs with five leading / characters
      ** like file://///host/path to be converted into UNCs like //host/path.
      ** The correct URI for that UNC has only two or four leading / characters
      ** file://host/path or file:////host/path.  But 5 leading slashes is a
      ** common error, we are told, so we handle it as a special case. */
      ਜੇ( ਲੜੀ_ਤੁਲਨਾ(zUri+7, "///", 3)==0 ){ i_ਵਿੱਚ++; }
    }ਨਹੀਂਤਾਂ ਜੇ( ਲੜੀ_ਤੁਲਨਾ(zUri+5, "//localhost/", 12)==0 ){
      i_ਵਿੱਚ = 16;
    }
#ਨਹੀਂਤਾਂ
    /* Discard the scheme and authority segments of the URI. */
    ਜੇ( zUri[5]=='/' && zUri[6]=='/' ){
      i_ਵਿੱਚ = 7;
      ਜਦੋਂ( zUri[i_ਵਿੱਚ] && zUri[i_ਵਿੱਚ]!='/' ) i_ਵਿੱਚ++;
      ਜੇ( i_ਵਿੱਚ!=7 && (i_ਵਿੱਚ!=16 || memcmp("localhost", &zUri[7], 9)) ){
        *pz_Err_ਸੁਨੇਹਾ = sqlite3_mprintf("invalid uri authority: %.*s",
            (ਪੂਰਨ)(i_ਵਿੱਚ-7), &zUri[7]);
        rc = SQLITE_ERROR;
        ਜਾਓ parse_uri_ਬਾਹਰਲਾ;
      }
    }
#ਜੇ_ਅੰਤ

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
    ਜਦੋਂ( (c = zUri[i_ਵਿੱਚ])!=0 && c!='#' ){
      i_ਵਿੱਚ++;
      ਜੇ( c=='%'
       && sqlite3Isxdigit(zUri[i_ਵਿੱਚ])
       && sqlite3Isxdigit(zUri[i_ਵਿੱਚ+1])
      ){
        ਪੂਰਨ octet = (sqlite3_Hex_ਮੰਜ਼ਿਲ_Int(zUri[i_ਵਿੱਚ++]) << 4);
        octet += sqlite3_Hex_ਮੰਜ਼ਿਲ_Int(zUri[i_ਵਿੱਚ++]);

        ਪ੍ਰਤਿਜ੍ਞਾ( octet>=0 && octet<256 );
        ਜੇ( octet==0 ){
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_URI_00_ERROR
          /* This branch is taken when "%00" appears within the URI. In this
          ** case we ignore all text in the remainder of the path, name or
          ** value currently being parsed. So ignore the current character
          ** and skip to the next "?", "=" or "&", as appropriate. */
          ਜਦੋਂ( (c = zUri[i_ਵਿੱਚ])!=0 && c!='#'
              && (eState!=0 || c!='?')
              && (eState!=1 || (c!='=' && c!='&'))
              && (eState!=2 || c!='&')
          ){
            i_ਵਿੱਚ++;
          }
          ਜਾਰੀ;
#ਨਹੀਂਤਾਂ
          /* If ENABLE_URI_00_ERROR is defined, "%00" in a URI is an error. */
          *pz_Err_ਸੁਨੇਹਾ = sqlite3_mprintf("unexpected %%00 in uri");
          rc = SQLITE_ERROR;
          ਜਾਓ parse_uri_ਬਾਹਰਲਾ;
#ਜੇ_ਅੰਤ
        }
        c = octet;
      }ਨਹੀਂਤਾਂ ਜੇ( eState==1 && (c=='&' || c=='=') ){
        ਜੇ( zFile[i_ਬਾਹਰਲਾ-1]==0 ){
          /* An empty option name. Ignore this option altogether. */
          ਜਦੋਂ( zUri[i_ਵਿੱਚ] && zUri[i_ਵਿੱਚ]!='#' && zUri[i_ਵਿੱਚ-1]!='&' ) i_ਵਿੱਚ++;
          ਜਾਰੀ;
        }
        ਜੇ( c=='&' ){
          zFile[i_ਬਾਹਰਲਾ++] = '\0';
        }ਨਹੀਂਤਾਂ{
          eState = 2;
        }
        c = 0;
      }ਨਹੀਂਤਾਂ ਜੇ( (eState==0 && c=='?') || (eState==2 && c=='&') ){
        c = 0;
        eState = 1;
      }
      zFile[i_ਬਾਹਰਲਾ++] = c;
    }
    ਜੇ( eState==1 ) zFile[i_ਬਾਹਰਲਾ++] = '\0';
    memset(zFile+i_ਬਾਹਰਲਾ, 0, 4); /* end-of-options + empty journal filenames */

    /* Check if there were any options specified that should be interpreted
    ** here. Options that are interpreted here include "vfs" and those that
    ** correspond to flags that may be passed to the sqlite3_open_v2()
    ** method. */
    zOpt = &zFile[ਲੜੀ_ਲੰਬਾਈ(zFile)+1];
    ਜਦੋਂ( zOpt[0] ){
      i64 nOpt = ਲੜੀ_ਲੰਬਾਈ(zOpt);
      ਅੱਖਰ *zVal = &zOpt[nOpt+1];
      i64 nVal = ਲੜੀ_ਲੰਬਾਈ(zVal);

      ਜੇ( nOpt==3 && memcmp("vfs", zOpt, 3)==0 ){
        zVfs = zVal;
      }ਨਹੀਂਤਾਂ{
        ਬਣਤਰ Open_ਬਹੁਲਕ {
          ਅਚਲ ਅੱਖਰ *z;
          ਪੂਰਨ ਬਹੁਲਕ;
        } *a_ਬਹੁਲਕ = 0;
        ਅੱਖਰ *z_ਬਹੁਲਕ_Type = 0;
        ਪੂਰਨ ਨਕਾਬ = 0;
        ਪੂਰਨ ਹੱਦਬੰਦੀ = 0;

        ਜੇ( nOpt==5 && memcmp("cache", zOpt, 5)==0 ){
          ਸਥਿਰ ਬਣਤਰ Open_ਬਹੁਲਕ a_Cache_ਬਹੁਲਕ[] = {
            { "shared",  SQLITE_OPEN_SHAREDCACHE },
            { "private", SQLITE_OPEN_PRIVATECACHE },
            { 0, 0 }
          };

          ਨਕਾਬ = SQLITE_OPEN_SHAREDCACHE|SQLITE_OPEN_PRIVATECACHE;
          a_ਬਹੁਲਕ = a_Cache_ਬਹੁਲਕ;
          ਹੱਦਬੰਦੀ = ਨਕਾਬ;
          z_ਬਹੁਲਕ_Type = "cache";
        }
        ਜੇ( nOpt==4 && memcmp("mode", zOpt, 4)==0 ){
          ਸਥਿਰ ਬਣਤਰ Open_ਬਹੁਲਕ a_Open_ਬਹੁਲਕ[] = {
            { "ro",  SQLITE_OPEN_READONLY },
            { "rw",  SQLITE_OPEN_READWRITE },
            { "rwc", SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE },
            { "memory", SQLITE_OPEN_ਸਮ੍ਰਿਤੀ },
            { 0, 0 }
          };

          ਨਕਾਬ = SQLITE_OPEN_READONLY | SQLITE_OPEN_READWRITE
                   | SQLITE_OPEN_CREATE | SQLITE_OPEN_ਸਮ੍ਰਿਤੀ;
          a_ਬਹੁਲਕ = a_Open_ਬਹੁਲਕ;
          ਹੱਦਬੰਦੀ = ਨਕਾਬ & flags;
          z_ਬਹੁਲਕ_Type = "access";
        }

        ਜੇ( a_ਬਹੁਲਕ ){
          ਪੂਰਨ i;
          ਪੂਰਨ ਬਹੁਲਕ = 0;
          ਲਈ(i=0; a_ਬਹੁਲਕ[i].z; i++){
            ਅਚਲ ਅੱਖਰ *z = a_ਬਹੁਲਕ[i].z;
            ਜੇ( nVal==(i64)ਲੜੀ_ਲੰਬਾਈ(z) && 0==memcmp(zVal, z, nVal) ){
              ਬਹੁਲਕ = a_ਬਹੁਲਕ[i].ਬਹੁਲਕ;
              ਰੋਕੋ;
            }
          }
          ਜੇ( ਬਹੁਲਕ==0 ){
            *pz_Err_ਸੁਨੇਹਾ = sqlite3_mprintf("no such %s mode: %s", z_ਬਹੁਲਕ_Type, zVal);
            rc = SQLITE_ERROR;
            ਜਾਓ parse_uri_ਬਾਹਰਲਾ;
          }
          ਜੇ( (ਬਹੁਲਕ & ~SQLITE_OPEN_ਸਮ੍ਰਿਤੀ)>ਹੱਦਬੰਦੀ ){
            *pz_Err_ਸੁਨੇਹਾ = sqlite3_mprintf("%s mode not allowed: %s",
                                        z_ਬਹੁਲਕ_Type, zVal);
            rc = SQLITE_PERM;
            ਜਾਓ parse_uri_ਬਾਹਰਲਾ;
          }
          flags = (flags & ~ਨਕਾਬ) | ਬਹੁਲਕ;
        }
      }

      zOpt = &zVal[nVal+1];
    }

  }ਨਹੀਂਤਾਂ{
    zFile = sqlite3_malloc64(nUri+8);
    ਜੇ( !zFile ) ਵਾਪਸ SQLITE_NOMEM_BKPT;
    memset(zFile, 0, 4);
    zFile += 4;
    ਜੇ( nUri ){
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(zFile, zUri, nUri);
    }
    memset(zFile+nUri, 0, 4);
    flags &= ~SQLITE_OPEN_URI;
  }

  *ppVfs = sqlite3_vfs_ਲੱਭੋ(zVfs);
  ਜੇ( *ppVfs==0 ){
    *pz_Err_ਸੁਨੇਹਾ = sqlite3_mprintf("no such vfs: %s", zVfs);
    rc = SQLITE_ERROR;
  }
 parse_uri_ਬਾਹਰਲਾ:
  ਜੇ( rc!=SQLITE_OK ){
    sqlite3_free_filename(zFile);
    zFile = 0;
  }
  *pFlags = flags;
  *pzFile = zFile;
  ਵਾਪਸ rc;
}

/*
** This routine does the core work of extracting URI parameters from a
** database filename for the sqlite3_uri_parameter() interface.
*/
ਸਥਿਰ ਅਚਲ ਅੱਖਰ *uriParameter(ਅਚਲ ਅੱਖਰ *zFilename, ਅਚਲ ਅੱਖਰ *zParam){
  zFilename += sqlite3Strlen30(zFilename) + 1;
  ਜਦੋਂ( ALWAYS(zFilename!=0) && zFilename[0] ){
    ਪੂਰਨ x = strcmp(zFilename, zParam);
    zFilename += sqlite3Strlen30(zFilename) + 1;
    ਜੇ( x==0 ) ਵਾਪਸ zFilename;
    zFilename += sqlite3Strlen30(zFilename) + 1;
  }
  ਵਾਪਸ 0;
}



/*
** This routine does the work of opening a database on behalf of
** sqlite3_open() and sqlite3_open16(). The database filename "zFilename" 
** is UTF-8 encoded.
*/
ਸਥਿਰ ਪੂਰਨ openDatabase(
  ਅਚਲ ਅੱਖਰ *zFilename, /* Database filename UTF-8 encoded */
  sqlite3 **ppDb,        /* OUT: Returned database handle */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ flags,    /* Operational flags */
  ਅਚਲ ਅੱਖਰ *zVfs       /* Name of the VFS to use */
){
  sqlite3 *db;                    /* Store allocated handle here */
  ਪੂਰਨ rc;                         /* Return code */
  ਪੂਰਨ isThreadsafe;               /* True for threadsafe connections */
  ਅੱਖਰ *zOpen = 0;                /* Filename argument to pass to BtreeOpen() */
  ਅੱਖਰ *z_Err_ਸੁਨੇਹਾ = 0;              /* Error message from sqlite3ParseUri() */
  ਪੂਰਨ i;                          /* Loop counter */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( ppDb==0 ) ਵਾਪਸ SQLITE_MISUSE_BKPT;
#ਜੇ_ਅੰਤ
  *ppDb = 0;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOINIT
  rc = sqlite3_initialize();
  ਜੇ( rc ) ਵਾਪਸ rc;
#ਜੇ_ਅੰਤ

  ਜੇ( sqlite3GlobalConfig.bCoreMutex==0 ){
    isThreadsafe = 0;
  }ਨਹੀਂਤਾਂ ਜੇ( flags & SQLITE_OPEN_NOMUTEX ){
    isThreadsafe = 0;
  }ਨਹੀਂਤਾਂ ਜੇ( flags & SQLITE_OPEN_FULLMUTEX ){
    isThreadsafe = 1;
  }ਨਹੀਂਤਾਂ{
    isThreadsafe = sqlite3GlobalConfig.b_ਭਰਿਆ_Mutex;
  }

  ਜੇ( flags & SQLITE_OPEN_PRIVATECACHE ){
    flags &= ~SQLITE_OPEN_SHAREDCACHE;
  }ਨਹੀਂਤਾਂ ਜੇ( sqlite3GlobalConfig.sharedCacheEnabled ){
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
               SQLITE_OPEN_ਆਰਜ਼ੀ_DB |
               SQLITE_OPEN_TRANSIENT_DB |
               SQLITE_OPEN_MAIN_JOURNAL |
               SQLITE_OPEN_ਆਰਜ਼ੀ_JOURNAL |
               SQLITE_OPEN_SUBJOURNAL |
               SQLITE_OPEN_SUPER_JOURNAL |
               SQLITE_OPEN_NOMUTEX |
               SQLITE_OPEN_FULLMUTEX |
               SQLITE_OPEN_WAL
             );

  /* Allocate the sqlite data structure */
  db = sqlite3MallocZero( ਆਕਾਰ(sqlite3) );
  ਜੇ( db==0 ) ਜਾਓ opendb_ਬਾਹਰਲਾ;
  ਜੇ( isThreadsafe
#ਜੇ defined(SQLITE_THREAD_MISUSE_WARNINGS)
   || sqlite3GlobalConfig.bCoreMutex
#ਜੇ_ਅੰਤ
  ){
    db->mutex = sqlite3MutexAlloc(SQLITE_MUTEX_RECURSIVE);
    ਜੇ( db->mutex==0 ){
      sqlite3_free(db);
      db = 0;
      ਜਾਓ opendb_ਬਾਹਰਲਾ;
    }
    ਜੇ( isThreadsafe==0 ){
      sqlite3MutexWarnOnContention(db->mutex);
    }
  }
  sqlite3_mutex_enter(db->mutex);
  db->err_ਨਕਾਬ = (flags & SQLITE_OPEN_EXRESCODE)!=0 ? 0xffffffff : 0xff;
  db->nDb = 2;
  db->eOpenState = SQLITE_STATE_BUSY;
  db->aDb = db->aDbStatic;
  db->lookaside.bDisable = 1;
  db->lookaside.sz = 0;
  db->nFpDigit = 17;

  ਪ੍ਰਤਿਜ੍ਞਾ( ਆਕਾਰ(db->a_ਹੱਦਬੰਦੀ)==ਆਕਾਰ(a_Hard_ਹੱਦਬੰਦੀ) );
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(db->a_ਹੱਦਬੰਦੀ, a_Hard_ਹੱਦਬੰਦੀ, ਆਕਾਰ(db->a_ਹੱਦਬੰਦੀ));
  db->a_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_ਕਾਮਾ_ਧਾਗੇ] = SQLITE_DEFAULT_ਕਾਮਾ_ਧਾਗੇ;
  db->autoCommit = 1;
  db->ਅਗਲਾ_Autovac = -1;
  db->szMmap = sqlite3GlobalConfig.szMmap;
  db->ਅਗਲਾ_Pagesize = 0;
  db->init.azInit = sqlite3StdType; /* Any array of string ptrs will do */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_SORTER_MMAP
  /* Beginning with version 3.37.0, using the VFS xFetch() API to memory-map
  ** the temporary files used to do external sorts (see code in vdbesort.c)
  ** is disabled. It can still be used either by defining
  ** SQLITE_ENABLE_SORTER_MMAP at compile time or by using the
  ** SQLITE_TESTCTRL_SORTER_MMAP test-control at runtime. */
  db->nMaxSorterMmap = 0x7FFFFFFF;
#ਜੇ_ਅੰਤ
  db->flags |= SQLITE_Short_ਖਾਨਾ_ਨਾਂ
                 | SQLITE_EnableTrigger
                 | SQLITE_EnableView
                 | SQLITE_CacheSpill
                 | SQLITE_AttachCreate
                 | SQLITE_AttachWrite
                 | SQLITE_Comments
#ਜੇ !defined(SQLITE_TRUSTED_SCHEMA) || SQLITE_TRUSTED_SCHEMA+0!=0
                 | SQLITE_TrustedSchema
#ਜੇ_ਅੰਤ
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
#ਜੇ !defined(SQLITE_DQS)
# define SQLITE_DQS 3
#ਜੇ_ਅੰਤ
#ਜੇ (SQLITE_DQS&1)==1
                 | SQLITE_DqsDML
#ਜੇ_ਅੰਤ
#ਜੇ (SQLITE_DQS&2)==2
                 | SQLITE_DqsDDL
#ਜੇ_ਅੰਤ

#ਜੇ !defined(SQLITE_DEFAULT_AUTOMATIC_ਸੂਚਕਾਂਕ) || SQLITE_DEFAULT_AUTOMATIC_ਸੂਚਕਾਂਕ
                 | SQLITE_Auto_ਸੂਚਕਾਂਕ
#ਜੇ_ਅੰਤ
#ਜੇ SQLITE_DEFAULT_CKPTFULLFSYNC
                 | SQLITE_Ckpt_ਭਰਿਆ_FSync
#ਜੇ_ਅੰਤ
#ਜੇ SQLITE_DEFAULT_FILE_FORMAT<4
                 | SQLITE_LegacyFileFmt
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_ਲੱਦੋ_EXTENSION
                 | SQLITE_ਲੱਦੋ_Extension
#ਜੇ_ਅੰਤ
#ਜੇ SQLITE_DEFAULT_RECURSIVE_TRIGGERS
                 | SQLITE_RecTriggers
#ਜੇ_ਅੰਤ
#ਜੇ defined(SQLITE_DEFAULT_FOREIGN_ਕੁੰਜੀਆਂ) && SQLITE_DEFAULT_FOREIGN_ਕੁੰਜੀਆਂ
                 | SQLITE_Foreign_ਕੁੰਜੀਆਂ
#ਜੇ_ਅੰਤ
#ਜੇ defined(SQLITE_ਉਲਟ_UNORDERED_SELECTS)
                 | SQLITE_ਉਲਟ_ਕ੍ਰਮ
#ਜੇ_ਅੰਤ
#ਜੇ defined(SQLITE_ENABLE_OVERSIZE_CELL_CHECK)
                 | SQLITE_Cell_ਮਾਪ_Ck
#ਜੇ_ਅੰਤ
#ਜੇ defined(SQLITE_ENABLE_FTS3_TOKENIZER)
                 | SQLITE_Fts3Tokenizer
#ਜੇ_ਅੰਤ
#ਜੇ defined(SQLITE_ENABLE_QPSG)
                 | SQLITE_EnableQPSG
#ਜੇ_ਅੰਤ
#ਜੇ defined(SQLITE_DEFAULT_DEFENSIVE)
                 | SQLITE_Defensive
#ਜੇ_ਅੰਤ
#ਜੇ defined(SQLITE_DEFAULT_LEGACY_ALTER_ਸਾਰਣੀ)
                 | SQLITE_LegacyAlter
#ਜੇ_ਅੰਤ
#ਜੇ defined(SQLITE_ENABLE_STMT_SCANSTATUS)
                 | SQLITE_Stmt_ਜਾਂਚ_ਸਥਿਤੀ
#ਜੇ_ਅੰਤ
      ;
  sqlite3HashInit(&db->aCollSeq);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
  sqlite3HashInit(&db->aModule);
#ਜੇ_ਅੰਤ

  /* Add the default collation sequence BINARY. BINARY works for both UTF-8
  ** and UTF-16, so add a version for each to avoid any unnecessary
  ** conversions. The only error that can occur here is a malloc() failure.
  **
  ** EVIDENCE-OF: R-52786-44878 SQLite defines three built-in collating
  ** functions:
  */
  createCollation(db, sqlite3_Str_ਦੁਆਧਾਰੀ, SQLITE_UTF8, 0,
                  sqlite3_ਦੁਆਧਾਰੀ_Compare, 0);
  createCollation(db, sqlite3_Str_ਦੁਆਧਾਰੀ, SQLITE_UTF16BE, 0,
                  sqlite3_ਦੁਆਧਾਰੀ_Compare, 0);
  createCollation(db, sqlite3_Str_ਦੁਆਧਾਰੀ, SQLITE_UTF16LE, 0,
                  sqlite3_ਦੁਆਧਾਰੀ_Compare, 0);
  createCollation(db, "NOCASE", SQLITE_UTF8, 0, nocaseCollatingFunc, 0);
  createCollation(db, "RTRIM", SQLITE_UTF8, 0, rtrimCollFunc, 0);
  ਜੇ( db->mallocFailed ){
    ਜਾਓ opendb_ਬਾਹਰਲਾ;
  }

#ਜੇ SQLITE_OS_UNIX && defined(SQLITE_OS_KV_OPTIONAL)
  /* Process magic filenames ":localStorage:" and ":sessionStorage:" */
  ਜੇ( zFilename && zFilename[0]==':' ){
    ਜੇ( strcmp(zFilename, ":localStorage:")==0 ){
      zFilename = "file:local?vfs=kvvfs";
      flags |= SQLITE_OPEN_URI;
    }ਨਹੀਂਤਾਂ ਜੇ( strcmp(zFilename, ":sessionStorage:")==0 ){
      zFilename = "file:session?vfs=kvvfs";
      flags |= SQLITE_OPEN_URI;
    }
  }
#ਜੇ_ਅੰਤ /* SQLITE_OS_UNIX && defined(SQLITE_OS_KV_OPTIONAL) */

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
  ਪ੍ਰਤਿਜ੍ਞਾ( SQLITE_OPEN_READONLY  == 0x01 );
  ਪ੍ਰਤਿਜ੍ਞਾ( SQLITE_OPEN_READWRITE == 0x02 );
  ਪ੍ਰਤਿਜ੍ਞਾ( SQLITE_OPEN_CREATE    == 0x04 );
  testcase( (1<<(flags&7))==0x02 ); /* READONLY */
  testcase( (1<<(flags&7))==0x04 ); /* READWRITE */
  testcase( (1<<(flags&7))==0x40 ); /* READWRITE | CREATE */
  ਜੇ( ((1<<(flags&7)) & 0x46)==0 ){
    rc = SQLITE_MISUSE_BKPT;  /* IMP: R-18321-05872 */
  }ਨਹੀਂਤਾਂ{
    ਜੇ( zFilename==0 ) zFilename = ":memory:";
    rc = sqlite3ParseUri(zVfs, zFilename, &flags, &db->pVfs, &zOpen, &z_Err_ਸੁਨੇਹਾ);
  }
  ਜੇ( rc!=SQLITE_OK ){
    ਜੇ( rc==SQLITE_NOMEM ) sqlite3OomFault(db);
    sqlite3_Error_With_ਸੁਨੇਹਾ(db, rc, z_Err_ਸੁਨੇਹਾ ? "%s" : 0, z_Err_ਸੁਨੇਹਾ);
    sqlite3_free(z_Err_ਸੁਨੇਹਾ);
    ਜਾਓ opendb_ਬਾਹਰਲਾ;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( db->pVfs!=0 );
#ਜੇ SQLITE_OS_KV || defined(SQLITE_OS_KV_OPTIONAL)
  ਜੇ( sqlite3_stricmp(db->pVfs->zName, "kvvfs")==0 ){
    db->ਆਰਜ਼ੀ_store = 2;
  }
#ਜੇ_ਅੰਤ

  /* Open the backend database driver */
  rc = sqlite3BtreeOpen(db->pVfs, zOpen, db, &db->aDb[0].pBt, 0,
                        flags | SQLITE_OPEN_MAIN_DB);
  ਜੇ( rc!=SQLITE_OK ){
    ਜੇ( rc==SQLITE_IOERR_NOMEM ){
      rc = SQLITE_NOMEM_BKPT;
    }
    sqlite3Error(db, rc);
    ਜਾਓ opendb_ਬਾਹਰਲਾ;
  }
  sqlite3BtreeEnter(db->aDb[0].pBt);
  db->aDb[0].pSchema = sqlite3SchemaGet(db, db->aDb[0].pBt);
  ਜੇ( !db->mallocFailed ){
    sqlite3_ਸੈੱਟ_ਪਾਠ_Encoding(db, SCHEMA_ENC(db));
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
  ਜੇ( db->mallocFailed ){
    ਜਾਓ opendb_ਬਾਹਰਲਾ;
  }

  /* Register all built-in functions, but do not attempt to read the
  ** database schema yet. This is delayed until the first time the database
  ** is accessed.
  */
  sqlite3Error(db, SQLITE_OK);
  sqlite3_ਰਜਿਸਟਰ_ਪ੍ਰਤੀ_Connection_Builtin_Functions(db);
  rc = sqlite3_errcode(db);


  /* Load compiled-in extensions */
  ਲਈ(i=0; rc==SQLITE_OK && i<Array_ਮਾਪ(sqlite3BuiltinExtensions); i++){
    rc = sqlite3BuiltinExtensions[i](db);
  }

  /* Load automatic extensions - extensions that have been registered
  ** using the sqlite3_automatic_extension() API.
  */
  ਜੇ( rc==SQLITE_OK ){
    sqlite3_Auto_ਲੱਦੋ_Extensions(db);
    rc = sqlite3_errcode(db);
    ਜੇ( rc!=SQLITE_OK ){
      ਜਾਓ opendb_ਬਾਹਰਲਾ;
    }
  }

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_ਅੰਦਰੂਨੀ_FUNCTIONS
  /* Testing use only!!! The -DSQLITE_ENABLE_INTERNAL_FUNCTIONS=1 compile-time
  ** option gives access to internal functions by default. 
  ** Testing use only!!! */
  db->mDbFlags |= DBFLAG_ਅੰਦਰੂਨੀ_Func;
#ਜੇ_ਅੰਤ

  /* -DSQLITE_DEFAULT_LOCKING_MODE=1 makes EXCLUSIVE the default locking
  ** mode.  -DSQLITE_DEFAULT_LOCKING_MODE=0 make NORMAL the default locking
  ** mode.  Doing nothing at all also makes NORMAL the default.
  */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEFAULT_LOCKING_ਬਹੁਲਕ
  db->dflt_ਤਾਲਾ_ਬਹੁਲਕ = SQLITE_DEFAULT_LOCKING_ਬਹੁਲਕ;
  sqlite3_Pager_Locking_ਬਹੁਲਕ(sqlite3BtreePager(db->aDb[0].pBt),
                          SQLITE_DEFAULT_LOCKING_ਬਹੁਲਕ);
#ਜੇ_ਅੰਤ

  ਜੇ( rc ) sqlite3Error(db, rc);

  /* Enable the lookaside-malloc subsystem */
  ਤਿਆਰੀ_Lookaside(db, 0, sqlite3GlobalConfig.szLookaside,
                        sqlite3GlobalConfig.nLookaside);

  sqlite3_wal_autocheckpoint(db, SQLITE_DEFAULT_WAL_AUTOCHECKPOINT);

opendb_ਬਾਹਰਲਾ:
  ਜੇ( db ){
    ਪ੍ਰਤਿਜ੍ਞਾ( db->mutex!=0 || isThreadsafe==0
           || sqlite3GlobalConfig.b_ਭਰਿਆ_Mutex==0 );
    sqlite3_mutex_leave(db->mutex);
  }
  rc = sqlite3_errcode(db);
  ਪ੍ਰਤਿਜ੍ਞਾ( db!=0 || (rc&0xff)==SQLITE_NOMEM );
  ਜੇ( (rc&0xff)==SQLITE_NOMEM ){
    sqlite3_close(db);
    db = 0;
  }ਨਹੀਂਤਾਂ ਜੇ( rc!=SQLITE_OK ){
    db->eOpenState = SQLITE_STATE_SICK;
  }
  *ppDb = db;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_SQLLOG
  ਜੇ( sqlite3GlobalConfig.xSqllog ){
    /* Opening a db handle. Fourth parameter is passed 0. */
    ਖਾਲੀ *pArg = sqlite3GlobalConfig.pSqllogArg;
    sqlite3GlobalConfig.xSqllog(pArg, db, zFilename, 0);
  }
#ਜੇ_ਅੰਤ
  sqlite3_free_filename(zOpen);
  ਵਾਪਸ rc;
}


/*
** Open a new database handle.
*/
ਪੂਰਨ sqlite3_open(
  ਅਚਲ ਅੱਖਰ *zFilename,
  sqlite3 **ppDb
){
  ਵਾਪਸ openDatabase(zFilename, ppDb,
                      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, 0);
}
ਪੂਰਨ sqlite3_open_v2(
  ਅਚਲ ਅੱਖਰ *filename,   /* Database filename (UTF-8) */
  sqlite3 **ppDb,         /* OUT: SQLite db handle */
  ਪੂਰਨ flags,              /* Flags */
  ਅਚਲ ਅੱਖਰ *zVfs        /* Name of VFS module to use */
){
  ਵਾਪਸ openDatabase(filename, ppDb, (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)flags, zVfs);
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_UTF16
/*
** Open a new database handle.
*/
ਪੂਰਨ sqlite3_open16(
  ਅਚਲ ਖਾਲੀ *zFilename,
  sqlite3 **ppDb
){
  ਅੱਖਰ ਅਚਲ *zFilename8;   /* zFilename encoded in UTF-8 instead of UTF-16 */
  sqlite3_ਮੁੱਲ *pVal;
  ਪੂਰਨ rc;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( ppDb==0 ) ਵਾਪਸ SQLITE_MISUSE_BKPT;
#ਜੇ_ਅੰਤ
  *ppDb = 0;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOINIT
  rc = sqlite3_initialize();
  ਜੇ( rc ) ਵਾਪਸ rc;
#ਜੇ_ਅੰਤ
  ਜੇ( zFilename==0 ) zFilename = "\000\000";
  pVal = sqlite3_ਮੁੱਲ_New(0);
  sqlite3_ਮੁੱਲ_ਸੈੱਟ_Str(pVal, -1, zFilename, SQLITE_UTF16NATIVE, SQLITE_STATIC);
  zFilename8 = sqlite3_ਮੁੱਲ_ਪਾਠ_2(pVal, SQLITE_UTF8);
  ਜੇ( zFilename8 ){
    rc = openDatabase(zFilename8, ppDb,
                      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, 0);
    ਪ੍ਰਤਿਜ੍ਞਾ( *ppDb || rc==SQLITE_NOMEM );
    ਜੇ( rc==SQLITE_OK && !DbHasProperty(*ppDb, 0, DB_SchemaLoaded) ){
      SCHEMA_ENC(*ppDb) = ENC(*ppDb) = SQLITE_UTF16NATIVE;
    }
  }ਨਹੀਂਤਾਂ{
    rc = SQLITE_NOMEM_BKPT;
  }
  sqlite3_ਮੁੱਲ_Free(pVal);

  ਵਾਪਸ rc & 0xff;
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_UTF16 */

/*
** Register a new collation sequence with the database handle db.
*/
ਪੂਰਨ sqlite3_create_collation(
  sqlite3* db,
  ਅਚਲ ਅੱਖਰ *zName,
  ਪੂਰਨ enc,
  ਖਾਲੀ* pCtx,
  ਪੂਰਨ(*xCompare)(ਖਾਲੀ*,ਪੂਰਨ,ਅਚਲ ਖਾਲੀ*,ਪੂਰਨ,ਅਚਲ ਖਾਲੀ*)
){
  ਵਾਪਸ sqlite3_create_collation_v2(db, zName, enc, pCtx, xCompare, 0);
}

/*
** Register a new collation sequence with the database handle db.
*/
ਪੂਰਨ sqlite3_create_collation_v2(
  sqlite3* db,
  ਅਚਲ ਅੱਖਰ *zName,
  ਪੂਰਨ enc,
  ਖਾਲੀ* pCtx,
  ਪੂਰਨ(*xCompare)(ਖਾਲੀ*,ਪੂਰਨ,ਅਚਲ ਖਾਲੀ*,ਪੂਰਨ,ਅਚਲ ਖਾਲੀ*),
  ਖਾਲੀ(*xDel)(ਖਾਲੀ*)
){
  ਪੂਰਨ rc;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !sqlite3SafetyCheckOk(db) || zName==0 ) ਵਾਪਸ SQLITE_MISUSE_BKPT;
#ਜੇ_ਅੰਤ
  sqlite3_mutex_enter(db->mutex);
  ਪ੍ਰਤਿਜ੍ਞਾ( !db->mallocFailed );
  rc = createCollation(db, zName, (u8)enc, pCtx, xCompare, xDel);
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  ਵਾਪਸ rc;
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_UTF16
/*
** Register a new collation sequence with the database handle db.
*/
ਪੂਰਨ sqlite3_create_collation16(
  sqlite3* db,
  ਅਚਲ ਖਾਲੀ *zName,
  ਪੂਰਨ enc,
  ਖਾਲੀ* pCtx,
  ਪੂਰਨ(*xCompare)(ਖਾਲੀ*,ਪੂਰਨ,ਅਚਲ ਖਾਲੀ*,ਪੂਰਨ,ਅਚਲ ਖਾਲੀ*)
){
  ਪੂਰਨ rc = SQLITE_OK;
  ਅੱਖਰ *zName8;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !sqlite3SafetyCheckOk(db) || zName==0 ) ਵਾਪਸ SQLITE_MISUSE_BKPT;
#ਜੇ_ਅੰਤ
  sqlite3_mutex_enter(db->mutex);
  ਪ੍ਰਤਿਜ੍ਞਾ( !db->mallocFailed );
  zName8 = sqlite3Utf16to8(db, zName, -1, SQLITE_UTF16NATIVE);
  ਜੇ( zName8 ){
    rc = createCollation(db, zName8, (u8)enc, pCtx, xCompare, 0);
    sqlite3DbFree(db, zName8);
  }
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  ਵਾਪਸ rc;
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_UTF16 */

/*
** Register a collation sequence factory callback with the database handle
** db. Replace any previously installed collation sequence factory.
*/
ਪੂਰਨ sqlite3_collation_needed(
  sqlite3 *db,
  ਖਾਲੀ *pCollNeededArg,
  ਖਾਲੀ(*xCollNeeded)(ਖਾਲੀ*,sqlite3*,ਪੂਰਨ e_ਪਾਠ_Rep,ਅਚਲ ਅੱਖਰ*)
){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !sqlite3SafetyCheckOk(db) ) ਵਾਪਸ SQLITE_MISUSE_BKPT;
#ਜੇ_ਅੰਤ
  sqlite3_mutex_enter(db->mutex);
  db->xCollNeeded = xCollNeeded;
  db->xCollNeeded16 = 0;
  db->pCollNeededArg = pCollNeededArg;
  sqlite3_mutex_leave(db->mutex);
  ਵਾਪਸ SQLITE_OK;
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_UTF16
/*
** Register a collation sequence factory callback with the database handle
** db. Replace any previously installed collation sequence factory.
*/
ਪੂਰਨ sqlite3_collation_needed16(
  sqlite3 *db,
  ਖਾਲੀ *pCollNeededArg,
  ਖਾਲੀ(*xCollNeeded16)(ਖਾਲੀ*,sqlite3*,ਪੂਰਨ e_ਪਾਠ_Rep,ਅਚਲ ਖਾਲੀ*)
){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !sqlite3SafetyCheckOk(db) ) ਵਾਪਸ SQLITE_MISUSE_BKPT;
#ਜੇ_ਅੰਤ
  sqlite3_mutex_enter(db->mutex);
  db->xCollNeeded = 0;
  db->xCollNeeded16 = xCollNeeded16;
  db->pCollNeededArg = pCollNeededArg;
  sqlite3_mutex_leave(db->mutex);
  ਵਾਪਸ SQLITE_OK;
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_UTF16 */

/*
** Find existing client data.
*/
ਖਾਲੀ *sqlite3_get_clientdata(sqlite3 *db, ਅਚਲ ਅੱਖਰ *zName){
  Db_Client_ਅੰਕੜੇ *p;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !zName || !sqlite3SafetyCheckOk(db) ){
    (ਖਾਲੀ)SQLITE_MISUSE_BKPT;
    ਵਾਪਸ 0;
  }
#ਜੇ_ਅੰਤ
  sqlite3_mutex_enter(db->mutex);
  ਲਈ(p=db->p_Db_ਅੰਕੜੇ; p; p=p->p_ਅਗਲਾ){
    ਜੇ( strcmp(p->zName, zName)==0 ){
      ਖਾਲੀ *p_ਨਤੀਜਾ = p->p_ਅੰਕੜੇ;
      sqlite3_mutex_leave(db->mutex);
      ਵਾਪਸ p_ਨਤੀਜਾ;
    }
  }
  sqlite3_mutex_leave(db->mutex);
  ਵਾਪਸ 0;
}

/*
** Add new client data to a database connection.
*/
ਪੂਰਨ sqlite3_ਸੈੱਟ_clientdata(
  sqlite3 *db,                   /* Attach client data to this connection */
  ਅਚਲ ਅੱਖਰ *zName,             /* Name of the client data */
  ਖਾਲੀ *p_ਅੰਕੜੇ,                   /* The client data itself */
  ਖਾਲੀ (*xDestructor)(ਖਾਲੀ*)     /* Destructor */
){
  Db_Client_ਅੰਕੜੇ *p, **pp;
  sqlite3_mutex_enter(db->mutex);
  pp = &db->p_Db_ਅੰਕੜੇ;
  ਲਈ(p=db->p_Db_ਅੰਕੜੇ; p && strcmp(p->zName,zName); p=p->p_ਅਗਲਾ){
    pp = &p->p_ਅਗਲਾ;
  }
  ਜੇ( p ){
    ਪ੍ਰਤਿਜ੍ਞਾ( p->p_ਅੰਕੜੇ!=0 );
    ਜੇ( p->xDestructor ) p->xDestructor(p->p_ਅੰਕੜੇ);
    ਜੇ( p_ਅੰਕੜੇ==0 ){
      *pp = p->p_ਅਗਲਾ;
      sqlite3_free(p);
      sqlite3_mutex_leave(db->mutex);
      ਵਾਪਸ SQLITE_OK;
    }
  }ਨਹੀਂਤਾਂ ਜੇ( p_ਅੰਕੜੇ==0 ){
    sqlite3_mutex_leave(db->mutex);
    ਵਾਪਸ SQLITE_OK;
  }ਨਹੀਂਤਾਂ{
    size_t n = ਲੜੀ_ਲੰਬਾਈ(zName);
    p = sqlite3_malloc64( SZ_DBCLIENTDATA(n+1) );
    ਜੇ( p==0 ){
      ਜੇ( xDestructor ) xDestructor(p_ਅੰਕੜੇ);
      sqlite3_mutex_leave(db->mutex);
      ਵਾਪਸ SQLITE_NOMEM;
    }
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(p->zName, zName, n+1);
    p->p_ਅਗਲਾ = db->p_Db_ਅੰਕੜੇ;
    db->p_Db_ਅੰਕੜੇ = p;
  }
  p->p_ਅੰਕੜੇ = p_ਅੰਕੜੇ;
  p->xDestructor = xDestructor;
  sqlite3_mutex_leave(db->mutex);
  ਵਾਪਸ SQLITE_OK;
}


#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_DEPRECATED
/*
** This function is now an anachronism. It used to be used to recover from a
** malloc() failure, but SQLite now does this automatically.
*/
ਪੂਰਨ sqlite3_global_recover(ਖਾਲੀ){
  ਵਾਪਸ SQLITE_OK;
}
#ਜੇ_ਅੰਤ

/*
** Test to see whether or not the database connection is in autocommit
** mode.  Return TRUE if it is and FALSE if not.  Autocommit mode is on
** by default.  Autocommit is disabled by a BEGIN statement and reenabled
** by the next COMMIT or ROLLBACK.
*/
ਪੂਰਨ sqlite3_get_autocommit(sqlite3 *db){
  ਪੂਰਨ iRet;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !sqlite3SafetyCheckOk(db) ){
    (ਖਾਲੀ)SQLITE_MISUSE_BKPT;
    ਵਾਪਸ 0;
  }
#ਜੇ_ਅੰਤ
  sqlite3_mutex_enter(db->mutex);
  iRet = db->autoCommit;
  sqlite3_mutex_leave(db->mutex);
  ਵਾਪਸ iRet;
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
ਪੂਰਨ sqlite3ReportError(ਪੂਰਨ iErr, ਪੂਰਨ lineno, ਅਚਲ ਅੱਖਰ *zType){
  sqlite3_log(iErr, "%s at line %d of [%.10s]",
              zType, lineno, 20+sqlite3_sourceid());
  ਵਾਪਸ iErr;
}
ਪੂਰਨ sqlite3CorruptError(ਪੂਰਨ lineno){
  testcase( sqlite3GlobalConfig.xLog!=0 );
  ਵਾਪਸ sqlite3ReportError(SQLITE_CORRUPT, lineno, "database corruption");
}
ਪੂਰਨ sqlite3MisuseError(ਪੂਰਨ lineno){
  testcase( sqlite3GlobalConfig.xLog!=0 );
  ਵਾਪਸ sqlite3ReportError(SQLITE_MISUSE, lineno, "misuse");
}
ਪੂਰਨ sqlite3CantopenError(ਪੂਰਨ lineno){
  testcase( sqlite3GlobalConfig.xLog!=0 );
  ਵਾਪਸ sqlite3ReportError(SQLITE_CANTOPEN, lineno, "cannot open file");
}
#ਜੇ defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_CORRUPT_PGNO)
ਪੂਰਨ sqlite3CorruptPgnoError(ਪੂਰਨ lineno, Pgno pgno){
  ਅੱਖਰ z_ਸੁਨੇਹਾ[100];
  sqlite3_snprintf(ਆਕਾਰ(z_ਸੁਨੇਹਾ), z_ਸੁਨੇਹਾ, "database corruption page %d", pgno);
  testcase( sqlite3GlobalConfig.xLog!=0 );
  ਵਾਪਸ sqlite3ReportError(SQLITE_CORRUPT, lineno, z_ਸੁਨੇਹਾ);
}
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
ਪੂਰਨ sqlite3NomemError(ਪੂਰਨ lineno){
  testcase( sqlite3GlobalConfig.xLog!=0 );
  ਵਾਪਸ sqlite3ReportError(SQLITE_NOMEM, lineno, "OOM");
}
ਪੂਰਨ sqlite3IoerrnomemError(ਪੂਰਨ lineno){
  testcase( sqlite3GlobalConfig.xLog!=0 );
  ਵਾਪਸ sqlite3ReportError(SQLITE_IOERR_NOMEM, lineno, "I/O OOM error");
}
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_DEPRECATED
/*
** This is a convenience routine that makes sure that all thread-specific
** data for this thread has been deallocated.
**
** SQLite no longer uses thread-specific data so this routine is now a
** no-op.  It is retained for historical compatibility.
*/
ਖਾਲੀ sqlite3_thread_cleanup(ਖਾਲੀ){
}
#ਜੇ_ਅੰਤ

/*
** Return meta information about a specific column of a database table.
** See comment in sqlite3.h (sqlite.h.in) for details.
*/
ਪੂਰਨ sqlite3_ਸਾਰਣੀ_ਕਾਲਮ_metadata(
  sqlite3 *db,                /* Connection handle */
  ਅਚਲ ਅੱਖਰ *zDbName,        /* Database name or NULL */
  ਅਚਲ ਅੱਖਰ *z_ਸਾਰਣੀ_Name,     /* Table name */
  ਅਚਲ ਅੱਖਰ *z_ਕਾਲਮ_Name,    /* Column name */
  ਅੱਖਰ ਅਚਲ **pz_ਅੰਕੜੇ_Type,    /* OUTPUT: Declared data type */
  ਅੱਖਰ ਅਚਲ **pzCollSeq,     /* OUTPUT: Collation sequence name */
  ਪੂਰਨ *pNotNull,              /* OUTPUT: True if NOT NULL constraint exists */
  ਪੂਰਨ *p_Primary_ਕੁੰਜੀ,           /* OUTPUT: True if column part of PK */
  ਪੂਰਨ *pAutoinc               /* OUTPUT: True if column is auto-increment */
){
  ਪੂਰਨ rc;
  ਅੱਖਰ *z_Err_ਸੁਨੇਹਾ = 0;
  ਸਾਰਣੀ *pTab = 0;
  ਕਾਲਮ *p_ਖਾਨਾ = 0;
  ਪੂਰਨ i_ਖਾਨਾ = 0;
  ਅੱਖਰ ਅਚਲ *z_ਅੰਕੜੇ_Type = 0;
  ਅੱਖਰ ਅਚਲ *zCollSeq = 0;
  ਪੂਰਨ notnull = 0;
  ਪੂਰਨ primarykey = 0;
  ਪੂਰਨ autoinc = 0;


#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !sqlite3SafetyCheckOk(db) || z_ਸਾਰਣੀ_Name==0 ){
    ਵਾਪਸ SQLITE_MISUSE_BKPT;
  }
#ਜੇ_ਅੰਤ

  /* Ensure the database schema has been loaded */
  sqlite3_mutex_enter(db->mutex);
  sqlite3BtreeEnterAll(db);
  rc = sqlite3Init(db, &z_Err_ਸੁਨੇਹਾ);
  ਜੇ( SQLITE_OK!=rc ){
    ਜਾਓ error_ਬਾਹਰਲਾ;
  }

  /* Locate the table in question */
  pTab = sqlite3_ਲੱਭੋ_ਸਾਰਣੀ(db, z_ਸਾਰਣੀ_Name, zDbName);
  ਜੇ( !pTab || IsView(pTab) ){
    pTab = 0;
    ਜਾਓ error_ਬਾਹਰਲਾ;
  }

  /* Find the column for which info is requested */
  ਜੇ( z_ਕਾਲਮ_Name==0 ){
    /* Query for existence of table only */
  }ਨਹੀਂਤਾਂ{
    i_ਖਾਨਾ = sqlite3_ਕਾਲਮ_ਸੂਚਕਾਂਕ(pTab, z_ਕਾਲਮ_Name);
    ਜੇ( i_ਖਾਨਾ>=0 ){
      p_ਖਾਨਾ = &pTab->a_ਖਾਨਾ[i_ਖਾਨਾ];
    }ਨਹੀਂਤਾਂ{
      ਜੇ( HasRowid(pTab) && sqlite3IsRowid(z_ਕਾਲਮ_Name) ){
        i_ਖਾਨਾ = pTab->iPKey;
        p_ਖਾਨਾ = i_ਖਾਨਾ>=0 ? &pTab->a_ਖਾਨਾ[i_ਖਾਨਾ] : 0;
      }ਨਹੀਂਤਾਂ{
        pTab = 0;
        ਜਾਓ error_ਬਾਹਰਲਾ;
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
  ਜੇ( p_ਖਾਨਾ ){
    z_ਅੰਕੜੇ_Type = sqlite3_ਕਾਲਮ_Type(p_ਖਾਨਾ,0);
    zCollSeq = sqlite3_ਕਾਲਮ_Coll(p_ਖਾਨਾ);
    notnull = p_ਖਾਨਾ->notNull!=0;
    primarykey  = (p_ਖਾਨਾ->ਖਾਨਾ_Flags & COLFLAG_PRIMKEY)!=0;
    autoinc = pTab->iPKey==i_ਖਾਨਾ && (pTab->tabFlags & TF_Autoincrement)!=0;
  }ਨਹੀਂਤਾਂ{
    z_ਅੰਕੜੇ_Type = "INTEGER";
    primarykey = 1;
  }
  ਜੇ( !zCollSeq ){
    zCollSeq = sqlite3_Str_ਦੁਆਧਾਰੀ;
  }

error_ਬਾਹਰਲਾ:
  sqlite3BtreeLeaveAll(db);

  /* Whether the function call succeeded or failed, set the output parameters
  ** to whatever their local counterparts contain. If an error did occur,
  ** this has the effect of zeroing all output parameters.
  */
  ਜੇ( pz_ਅੰਕੜੇ_Type ) *pz_ਅੰਕੜੇ_Type = z_ਅੰਕੜੇ_Type;
  ਜੇ( pzCollSeq ) *pzCollSeq = zCollSeq;
  ਜੇ( pNotNull ) *pNotNull = notnull;
  ਜੇ( p_Primary_ਕੁੰਜੀ ) *p_Primary_ਕੁੰਜੀ = primarykey;
  ਜੇ( pAutoinc ) *pAutoinc = autoinc;

  ਜੇ( SQLITE_OK==rc && !pTab ){
    sqlite3DbFree(db, z_Err_ਸੁਨੇਹਾ);
    z_Err_ਸੁਨੇਹਾ = sqlite3MPrintf(db, "no such table column: %s.%s", z_ਸਾਰਣੀ_Name,
        z_ਕਾਲਮ_Name);
    rc = SQLITE_ERROR;
  }
  sqlite3_Error_With_ਸੁਨੇਹਾ(db, rc, (z_Err_ਸੁਨੇਹਾ?"%s":0), z_Err_ਸੁਨੇਹਾ);
  sqlite3DbFree(db, z_Err_ਸੁਨੇਹਾ);
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  ਵਾਪਸ rc;
}

/*
** Sleep for a little while.  Return the amount of time slept.
*/
ਪੂਰਨ sqlite3_sleep(ਪੂਰਨ ms){
  sqlite3_vfs *pVfs;
  ਪੂਰਨ rc;
  pVfs = sqlite3_vfs_ਲੱਭੋ(0);
  ਜੇ( pVfs==0 ) ਵਾਪਸ 0;

  /* This function works in milliseconds, but the underlying OsSleep()
  ** API uses microseconds. Hence the 1000's.
  */
  rc = (sqlite3OsSleep(pVfs, ms<0 ? 0 : 1000*ms)/1000);
  ਵਾਪਸ rc;
}

/*
** Enable or disable the extended result codes.
*/
ਪੂਰਨ sqlite3_extended_ਨਤੀਜਾ_codes(sqlite3 *db, ਪੂਰਨ onoff){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !sqlite3SafetyCheckOk(db) ) ਵਾਪਸ SQLITE_MISUSE_BKPT;
#ਜੇ_ਅੰਤ
  sqlite3_mutex_enter(db->mutex);
  db->err_ਨਕਾਬ = onoff ? 0xffffffff : 0xff;
  sqlite3_mutex_leave(db->mutex);
  ਵਾਪਸ SQLITE_OK;
}

/*
** Invoke the xFileControl method on a particular database.
*/
ਪੂਰਨ sqlite3_file_control(sqlite3 *db, ਅਚਲ ਅੱਖਰ *zDbName, ਪੂਰਨ op, ਖਾਲੀ *pArg){
  ਪੂਰਨ rc = SQLITE_ERROR;
  Btree *pBtree;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !sqlite3SafetyCheckOk(db) ) ਵਾਪਸ SQLITE_MISUSE_BKPT;
#ਜੇ_ਅੰਤ
  sqlite3_mutex_enter(db->mutex);
  pBtree = sqlite3_Db_Name_ਮੰਜ਼ਿਲ_Btree(db, zDbName);
  ਜੇ( pBtree ){
    Pager *pPager;
    sqlite3_file *fd;
    sqlite3BtreeEnter(pBtree);
    pPager = sqlite3BtreePager(pBtree);
    ਪ੍ਰਤਿਜ੍ਞਾ( pPager!=0 );
    fd = sqlite3PagerFile(pPager);
    ਪ੍ਰਤਿਜ੍ਞਾ( fd!=0 );
    ਜੇ( op==SQLITE_FCNTL_FILE_ਸੂਚਕ ){
      *(sqlite3_file**)pArg = fd;
      rc = SQLITE_OK;
    }ਨਹੀਂਤਾਂ ਜੇ( op==SQLITE_FCNTL_VFS_ਸੂਚਕ ){
      *(sqlite3_vfs**)pArg = sqlite3PagerVfs(pPager);
      rc = SQLITE_OK;
    }ਨਹੀਂਤਾਂ ਜੇ( op==SQLITE_FCNTL_JOURNAL_ਸੂਚਕ ){
      *(sqlite3_file**)pArg = sqlite3PagerJrnlFile(pPager);
      rc = SQLITE_OK;
    }ਨਹੀਂਤਾਂ ਜੇ( op==SQLITE_FCNTL_ਅੰਕੜੇ_ਸੰਸਕਰਣ ){
      *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ*)pArg = sqlite3_Pager_ਅੰਕੜੇ_ਸੰਸਕਰਣ(pPager);
      rc = SQLITE_OK;
    }ਨਹੀਂਤਾਂ ਜੇ( op==SQLITE_FCNTL_RESERVE_ਬਾਈਟਾਂ ){
      ਪੂਰਨ iNew = *(ਪੂਰਨ*)pArg;
      *(ਪੂਰਨ*)pArg = sqlite3BtreeGetRequestedReserve(pBtree);
      ਜੇ( iNew>=0 && iNew<=255 ){
        sqlite3_Btree_ਸੈੱਟ_ਪੰਨਾ_ਮਾਪ(pBtree, 0, iNew, 0);
      }
      rc = SQLITE_OK;
    }ਨਹੀਂਤਾਂ ਜੇ( op==SQLITE_FCNTL_RESET_CACHE ){
      sqlite3BtreeClearCache(pBtree);
      rc = SQLITE_OK;
    }ਨਹੀਂਤਾਂ{
      ਪੂਰਨ nSave = db->busyHandler.nBusy;
      rc = sqlite3OsFileControl(fd, op, pArg);
      db->busyHandler.nBusy = nSave;
    }
    sqlite3BtreeLeave(pBtree);
  }
  sqlite3_mutex_leave(db->mutex);
  ਵਾਪਸ rc;
}

/*
** Interface to the testing logic.
*/
ਪੂਰਨ sqlite3_test_control(ਪੂਰਨ op, ...){
  ਪੂਰਨ rc = 0;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_UNTESTABLE
  ਅਣਵਰਤਿਆ_PARAMETER(op);
#ਨਹੀਂਤਾਂ
  va_list ap;
  va_start(ap, op);
  ਚੋਣ( op ){

    /*
    ** Save the current state of the PRNG.
    */
    ਹਾਲਤ SQLITE_TESTCTRL_PRNG_SAVE: {
      sqlite3PrngSaveState();
      ਰੋਕੋ;
    }

    /*
    ** Restore the state of the PRNG to the last state saved using
    ** PRNG_SAVE.  If PRNG_SAVE has never before been called, then
    ** this verb acts like PRNG_RESET.
    */
    ਹਾਲਤ SQLITE_TESTCTRL_PRNG_RESTORE: {
      sqlite3PrngRestoreState();
      ਰੋਕੋ;
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
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WSD
    ਹਾਲਤ SQLITE_TESTCTRL_PRNG_ਬੀਜ: {
      ਪੂਰਨ x = va_arg(ap, ਪੂਰਨ);
      ਪੂਰਨ y;
      sqlite3 *db = va_arg(ap, sqlite3*);
      ਪ੍ਰਤਿਜ੍ਞਾ( db==0 || db->aDb[0].pSchema!=0 );
      ਜੇ( db && (y = db->aDb[0].pSchema->schema_cookie)!=0 ){ x = y; }
      sqlite3Config.i_Prng_ਬੀਜ = x;
      sqlite3_randomness(0,0);
      ਰੋਕੋ;
    }
#ਜੇ_ਅੰਤ

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
    ਹਾਲਤ SQLITE_TESTCTRL_FK_NO_ACTION: {
      sqlite3 *db = va_arg(ap, sqlite3*);
      ਪੂਰਨ b = va_arg(ap, ਪੂਰਨ);
      ਜੇ( b ){
        db->flags |= SQLITE_FkNoAction;
      }ਨਹੀਂਤਾਂ{
        db->flags &= ~SQLITE_FkNoAction;
      }
      ਰੋਕੋ;
    }

    /*
    **  sqlite3_test_control(BITVEC_TEST, size, program)
    **
    ** Run a test against a Bitvec object of size.  The program argument
    ** is an array of integers that defines the test.  Return -1 on a
    ** memory allocation error, 0 on success, or non-zero for an error.
    ** See the sqlite3BitvecBuiltinTest() for additional information.
    */
    ਹਾਲਤ SQLITE_TESTCTRL_BITVEC_TEST: {
      ਪੂਰਨ sz = va_arg(ap, ਪੂਰਨ);
      ਪੂਰਨ *aProg = va_arg(ap, ਪੂਰਨ*);
      rc = sqlite3BitvecBuiltinTest(sz, aProg);
      ਰੋਕੋ;
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
    ਹਾਲਤ SQLITE_TESTCTRL_FAULT_INSTALL: {
      /* A bug in MSVC prevents it from understanding pointers to functions
      ** types in the second argument to va_arg().  Work around the problem
      ** using a typedef.
      ** http://support.microsoft.com/kb/47961  <-- dead hyperlink
      ** Search at http://web.archive.org/ to find the 2015-03-16 archive
      ** of the link above to see the original text.
      ** sqlite3GlobalConfig.xTestCallback = va_arg(ap, int(*)(int));
      */
      ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਪੂਰਨ(*sqlite3FaultFuncType)(ਪੂਰਨ);
      sqlite3GlobalConfig.xTestCallback = va_arg(ap, sqlite3FaultFuncType);
      rc = sqlite3FaultSim(0);
      ਰੋਕੋ;
    }

    /*
    **  sqlite3_test_control(BENIGN_MALLOC_HOOKS, xBegin, xEnd)
    **
    ** Register hooks to call to indicate which malloc() failures
    ** are benign.
    */
    ਹਾਲਤ SQLITE_TESTCTRL_BENIGN_MALLOC_HOOKS: {
      ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਖਾਲੀ (*void_function)(ਖਾਲੀ);
      void_function xBenignBegin;
      void_function x_Benign_ਸਿਰਾ;
      xBenignBegin = va_arg(ap, void_function);
      x_Benign_ਸਿਰਾ = va_arg(ap, void_function);
      sqlite3BenignMallocHooks(xBenignBegin, x_Benign_ਸਿਰਾ);
      ਰੋਕੋ;
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
    ਹਾਲਤ SQLITE_TESTCTRL_PENDING_BYTE: {
      rc = PENDING_BYTE;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WSD
      {
        ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ newVal = va_arg(ap, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ);
        ਜੇ( newVal ) sqlite3PendingByte = newVal;
      }
#ਜੇ_ਅੰਤ
      ਰੋਕੋ;
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
    ਹਾਲਤ SQLITE_TESTCTRL_ASSERT: {
      ਅਸਥਿਰ ਪੂਰਨ x = 0;
      ਪ੍ਰਤਿਜ੍ਞਾ( /*side-effects-ok*/ (x = va_arg(ap,ਪੂਰਨ))!=0 );
      rc = x;
#ਜੇ defined(SQLITE_DEBUG)
      /* Invoke these debugging routines so that the compiler does not
      ** issue "defined but not used" warnings. */
      ਜੇ( x==9999 ){
        sqlite3_Show_ਵਿਅੰਜਕ(0);
        sqlite3_Show_ਵਿਅੰਜਕ_List(0);
        sqlite3ShowIdList(0);
        sqlite3ShowSrcList(0);
        sqlite3ShowWith(0);
        sqlite3ShowUpsert(0);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_TRIGGER
        sqlite3_Show_Trigger_ਕਦਮ(0);
        sqlite3_Show_Trigger_ਕਦਮ_List(0);
        sqlite3ShowTrigger(0);
        sqlite3ShowTriggerList(0);
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
        sqlite3_Show_ਖਿੜਕੀ(0);
        sqlite3ShowWinFunc(0);
#ਜੇ_ਅੰਤ
        sqlite3ShowSelect(0);
      }
#ਜੇ_ਅੰਤ
      ਰੋਕੋ;
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
    ਹਾਲਤ SQLITE_TESTCTRL_ALWAYS: {
      ਪੂਰਨ x = va_arg(ap,ਪੂਰਨ);
      rc = x ? ALWAYS(x) : 0;
      ਰੋਕੋ;
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
    ਹਾਲਤ SQLITE_TESTCTRL_BYTEORDER: {
      rc = SQLITE_BYTEORDER*100 + SQLITE_LITTLEENDIAN*10 + SQLITE_BIGENDIAN;
      ਰੋਕੋ;
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
    ਹਾਲਤ SQLITE_TESTCTRL_OPTIMIZATIONS: {
      sqlite3 *db = va_arg(ap, sqlite3*);
      db->dbOptFlags = va_arg(ap, u32);
      ਰੋਕੋ;
    }

    /*  sqlite3_test_control(SQLITE_TESTCTRL_GETOPT, sqlite3 *db, int *N)
    **
    ** Write the current optimization settings into *N.  A zero bit means that
    ** the optimization is on, and a 1 bit means that the optimization is off.
    */
    ਹਾਲਤ SQLITE_TESTCTRL_GETOPT: {
      sqlite3 *db = va_arg(ap, sqlite3*);
      ਪੂਰਨ *pN = va_arg(ap, ਪੂਰਨ*);
      *pN = db->dbOptFlags;
      ਰੋਕੋ;
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
    ਹਾਲਤ SQLITE_TESTCTRL_LOCALTIME_FAULT: {
      sqlite3GlobalConfig.bLocaltimeFault = va_arg(ap, ਪੂਰਨ);
      ਜੇ( sqlite3GlobalConfig.bLocaltimeFault==2 ){
        ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਪੂਰਨ(*sqlite3LocaltimeType)(ਅਚਲ ਖਾਲੀ*,ਖਾਲੀ*);
        sqlite3GlobalConfig.xAltLocaltime = va_arg(ap, sqlite3LocaltimeType);
      }ਨਹੀਂਤਾਂ{
        sqlite3GlobalConfig.xAltLocaltime = 0;
      }
      ਰੋਕੋ;
    }

    /*   sqlite3_test_control(SQLITE_TESTCTRL_INTERNAL_FUNCTIONS, sqlite3*);
    **
    ** Toggle the ability to use internal functions on or off for
    ** the database connection given in the argument.
    */
    ਹਾਲਤ SQLITE_TESTCTRL_ਅੰਦਰੂਨੀ_FUNCTIONS: {
      sqlite3 *db = va_arg(ap, sqlite3*);
      db->mDbFlags ^= DBFLAG_ਅੰਦਰੂਨੀ_Func;
      ਰੋਕੋ;
    }

    /*   sqlite3_test_control(SQLITE_TESTCTRL_NEVER_CORRUPT, int);
    **
    ** Set or clear a flag that indicates that the database file is always well-
    ** formed and never corrupt.  This flag is clear by default, indicating that
    ** database files might have arbitrary corruption.  Setting the flag during
    ** testing causes certain assert() statements in the code to be activated
    ** that demonstrate invariants on well-formed database files.
    */
    ਹਾਲਤ SQLITE_TESTCTRL_NEVER_CORRUPT: {
      sqlite3GlobalConfig.neverCorrupt = va_arg(ap, ਪੂਰਨ);
      ਰੋਕੋ;
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
    ਹਾਲਤ SQLITE_TESTCTRL_EXTRA_SCHEMA_CHECKS: {
      sqlite3GlobalConfig.bExtraSchemaChecks = va_arg(ap, ਪੂਰਨ);
      ਰੋਕੋ;
    }

    /* Set the threshold at which OP_Once counters reset back to zero.
    ** By default this is 0x7ffffffe (over 2 billion), but that value is
    ** too big to test in a reasonable amount of time, so this control is
    ** provided to set a small and easily reachable reset value.
    */
    ਹਾਲਤ SQLITE_TESTCTRL_ONCE_RESET_ਹੱਦ: {
      sqlite3GlobalConfig.i_Once_Reset_ਹੱਦ = va_arg(ap, ਪੂਰਨ);
      ਰੋਕੋ;
    }

    /*   sqlite3_test_control(SQLITE_TESTCTRL_VDBE_COVERAGE, xCallback, ptr);
    **
    ** Set the VDBE coverage callback function to xCallback with context
    ** pointer ptr.
    */
    ਹਾਲਤ SQLITE_TESTCTRL_VDBE_COVERAGE: {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_VDBE_COVERAGE
      ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਖਾਲੀ (*branch_callback)(ਖਾਲੀ*,ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ,
                                      ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ,ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ);
      sqlite3GlobalConfig.xVdbeBranch = va_arg(ap,branch_callback);
      sqlite3GlobalConfig.pVdbeBranchArg = va_arg(ap,ਖਾਲੀ*);
#ਜੇ_ਅੰਤ
      ਰੋਕੋ;
    }

    /*   sqlite3_test_control(SQLITE_TESTCTRL_SORTER_MMAP, db, nMax); */
    ਹਾਲਤ SQLITE_TESTCTRL_SORTER_MMAP: {
      sqlite3 *db = va_arg(ap, sqlite3*);
      db->nMaxSorterMmap = va_arg(ap, ਪੂਰਨ);
      ਰੋਕੋ;
    }

    /*   sqlite3_test_control(SQLITE_TESTCTRL_ISINIT);
    **
    ** Return SQLITE_OK if SQLite has been initialized and SQLITE_ERROR if
    ** not.
    */
    ਹਾਲਤ SQLITE_TESTCTRL_ISINIT: {
      ਜੇ( sqlite3GlobalConfig.isInit==0 ) rc = SQLITE_ERROR;
      ਰੋਕੋ;
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
    ਹਾਲਤ SQLITE_TESTCTRL_IMPOSTER: {
      sqlite3 *db = va_arg(ap, sqlite3*);
      ਪੂਰਨ iDb;
      sqlite3_mutex_enter(db->mutex);
      iDb = sqlite3_ਲੱਭੋ_Db_Name(db, va_arg(ap,ਅਚਲ ਅੱਖਰ*));
      ਜੇ( iDb>=0 ){
        db->init.iDb = iDb;
        db->init.busy = db->init.imposter_ਸਾਰਣੀ = va_arg(ap,ਪੂਰਨ);
        db->init.newTnum = va_arg(ap,ਪੂਰਨ);
        ਜੇ( db->init.busy==0 && db->init.newTnum>0 ){
          sqlite3_Reset_All_Schemas_ਦਾ_Connection(db);
        }
      }
      sqlite3_mutex_leave(db->mutex);
      ਰੋਕੋ;
    }

#ਜੇ defined(YYCOVERAGE)
    /*  sqlite3_test_control(SQLITE_TESTCTRL_PARSER_COVERAGE, FILE *out)
    **
    ** This test control (only available when SQLite is compiled with
    ** -DYYCOVERAGE) writes a report onto "out" that shows all
    ** state/lookahead combinations in the parser state machine
    ** which are never exercised.  If any state is missed, make the
    ** return code SQLITE_ERROR.
    */
    ਹਾਲਤ SQLITE_TESTCTRL_PARSER_COVERAGE: {
      FILE *ਬਾਹਰਲਾ = va_arg(ap, FILE*);
      ਜੇ( sqlite3ParserCoverage(ਬਾਹਰਲਾ) ) rc = SQLITE_ERROR;
      ਰੋਕੋ;
    }
#ਜੇ_ਅੰਤ /* defined(YYCOVERAGE) */

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
    ਹਾਲਤ SQLITE_TESTCTRL_ਨਤੀਜਾ_INTREAL: {
      sqlite3_context *pCtx = va_arg(ap, sqlite3_context*);
      sqlite3_ਨਤੀਜਾ_Int_Real(pCtx);
      ਰੋਕੋ;
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
    ਹਾਲਤ SQLITE_TESTCTRL_SEEK_ਗਿਣਤੀ: {
      sqlite3 *db = va_arg(ap, sqlite3*);
      u64 *pn = va_arg(ap, sqlite3_uint64*);
      *pn = sqlite3_Btree_Seek_ਗਿਣਤੀ(db->aDb->pBt);
      (ਖਾਲੀ)db;  /* Silence harmless unused variable warning */
      ਰੋਕੋ;
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
    ਹਾਲਤ SQLITE_TESTCTRL_TRACEFLAGS: {
       ਪੂਰਨ opTrace = va_arg(ap, ਪੂਰਨ);
       u32 *ptr = va_arg(ap, u32*);
       ਚੋਣ( opTrace ){
         ਹਾਲਤ 0:   *ptr = sqlite3_ਰੁੱਖ_Trace;      ਰੋਕੋ;
         ਹਾਲਤ 1:   sqlite3_ਰੁੱਖ_Trace = *ptr;      ਰੋਕੋ;
         ਹਾਲਤ 2:   *ptr = sqlite3WhereTrace;     ਰੋਕੋ;
         ਹਾਲਤ 3:   sqlite3WhereTrace = *ptr;     ਰੋਕੋ;
       }
       ਰੋਕੋ;
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
    ਹਾਲਤ SQLITE_TESTCTRL_LOGEST: {
      ਦੁੱਗਣਾ r_ਵਿੱਚ = va_arg(ap, ਦੁੱਗਣਾ);
      LogEst rLogEst = sqlite3LogEstFromDouble(r_ਵਿੱਚ);
      ਪੂਰਨ *pI1 = va_arg(ap,ਪੂਰਨ*);
      u64 *pU64 = va_arg(ap,u64*);
      ਪੂਰਨ *pI2 = va_arg(ap,ਪੂਰਨ*);
      *pI1 = rLogEst;
      *pU64 = sqlite3_Log_Est_ਮੰਜ਼ਿਲ_Int(rLogEst);
      *pI2 = sqlite3LogEst(*pU64);
      ਰੋਕੋ;
    }

    /* sqlite3_test_control(SQLITE_TESTCTRL_ATOF, const char *z, double *p);
    **
    ** Test access to the sqlite3AtoF() routine.
    */
    ਹਾਲਤ SQLITE_TESTCTRL_ATOF: {
      ਅਚਲ ਅੱਖਰ *z = va_arg(ap,ਅਚਲ ਅੱਖਰ*);
      ਦੁੱਗਣਾ *pR = va_arg(ap,ਦੁੱਗਣਾ*);
      rc = sqlite3AtoF(z,pR);
      ਰੋਕੋ;
    }

#ਜੇ defined(SQLITE_DEBUG) && !defined(SQLITE_OMIT_WSD)
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
    ਹਾਲਤ SQLITE_TESTCTRL_TUNE: {
      ਪੂਰਨ id = va_arg(ap, ਪੂਰਨ);
      ਪੂਰਨ *pi_ਮੁੱਲ = va_arg(ap, ਪੂਰਨ*);
      ਜੇ( id>0 && id<=SQLITE_NTUNE ){
        Tuning(id) = *pi_ਮੁੱਲ;
      }ਨਹੀਂਤਾਂ ਜੇ( id<0 && id>=-SQLITE_NTUNE ){
        *pi_ਮੁੱਲ = Tuning(-id);
      }ਨਹੀਂਤਾਂ{
        rc = SQLITE_NOTFOUND;
      }
      ਰੋਕੋ;
    }
#ਜੇ_ਅੰਤ

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
    ਹਾਲਤ SQLITE_TESTCTRL_JSON_SELFCHECK: {
#ਜੇ defined(SQLITE_DEBUG) && !defined(SQLITE_OMIT_WSD)
      ਪੂਰਨ *pOnOff = va_arg(ap, ਪੂਰਨ*);
      ਜੇ( *pOnOff<0 ){
        *pOnOff = sqlite3Config.bJsonSelfcheck;
      }ਨਹੀਂਤਾਂ{
        sqlite3Config.bJsonSelfcheck = (u8)((*pOnOff)&0xff);
      }
#ਜੇ_ਅੰਤ
      ਰੋਕੋ;
    }
  }
  va_end(ap);
#ਜੇ_ਅੰਤ /* SQLITE_UNTESTABLE */
  ਵਾਪਸ rc;
}

/*
** The Pager stores the Database filename, Journal filename, and WAL filename
** consecutively in memory, in that order.  The database filename is prefixed
** by four zero bytes.  Locate the start of the database filename by searching
** backwards for the first byte following four consecutive zero bytes.
**
** This only works if the filename passed in was obtained from the Pager.
*/
ਸਥਿਰ ਅਚਲ ਅੱਖਰ *databaseName(ਅਚਲ ਅੱਖਰ *zName){
  ਜਦੋਂ( zName[-1]!=0 || zName[-2]!=0 || zName[-3]!=0 || zName[-4]!=0 ){
    zName--;
  }
  ਵਾਪਸ zName;
}

/*
** Append text z[] to the end of p[].  Return a pointer to the first
** character after then zero terminator on the new text in p[].
*/
ਸਥਿਰ ਅੱਖਰ *append_ਪਾਠ(ਅੱਖਰ *p, ਅਚਲ ਅੱਖਰ *z){
  size_t n = ਲੜੀ_ਲੰਬਾਈ(z);
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(p, z, n+1);
  ਵਾਪਸ p+n+1;
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
ਅਚਲ ਅੱਖਰ *sqlite3_create_filename(
  ਅਚਲ ਅੱਖਰ *zDatabase,
  ਅਚਲ ਅੱਖਰ *zJournal,
  ਅਚਲ ਅੱਖਰ *zWal,
  ਪੂਰਨ nParam,
  ਅਚਲ ਅੱਖਰ **azParam
){
  sqlite3_int64 nByte;
  ਪੂਰਨ i;
  ਅੱਖਰ *p_ਨਤੀਜਾ, *p;
  nByte = ਲੜੀ_ਲੰਬਾਈ(zDatabase) + ਲੜੀ_ਲੰਬਾਈ(zJournal) + ਲੜੀ_ਲੰਬਾਈ(zWal) + 10;
  ਲਈ(i=0; i<nParam*2; i++){
    nByte += ਲੜੀ_ਲੰਬਾਈ(azParam[i])+1;
  }
  p_ਨਤੀਜਾ = p = sqlite3_malloc64( nByte );
  ਜੇ( p==0 ) ਵਾਪਸ 0;
  memset(p, 0, 4);
  p += 4;
  p = append_ਪਾਠ(p, zDatabase);
  ਲਈ(i=0; i<nParam*2; i++){
    p = append_ਪਾਠ(p, azParam[i]);
  }
  *(p++) = 0;
  p = append_ਪਾਠ(p, zJournal);
  p = append_ਪਾਠ(p, zWal);
  *(p++) = 0;
  *(p++) = 0;
  ਪ੍ਰਤਿਜ੍ਞਾ( (sqlite3_int64)(p - p_ਨਤੀਜਾ)==nByte );
  ਵਾਪਸ p_ਨਤੀਜਾ + 4;
}

/*
** Free memory obtained from sqlite3_create_filename().  It is a severe
** error to call this routine with any parameter other than a pointer
** previously obtained from sqlite3_create_filename() or a NULL pointer.
*/
ਖਾਲੀ sqlite3_free_filename(ਅਚਲ ਅੱਖਰ *p){
  ਜੇ( p==0 ) ਵਾਪਸ;
  p = databaseName(p);
  sqlite3_free((ਅੱਖਰ*)p - 4);
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
ਅਚਲ ਅੱਖਰ *sqlite3_uri_parameter(ਅਚਲ ਅੱਖਰ *zFilename, ਅਚਲ ਅੱਖਰ *zParam){
  ਜੇ( zFilename==0 || zParam==0 ) ਵਾਪਸ 0;
  zFilename = databaseName(zFilename);
  ਵਾਪਸ uriParameter(zFilename, zParam);
}

/*
** Return a pointer to the name of Nth query parameter of the filename.
*/
ਅਚਲ ਅੱਖਰ *sqlite3_uri_ਕੁੰਜੀ(ਅਚਲ ਅੱਖਰ *zFilename, ਪੂਰਨ N){
  ਜੇ( zFilename==0 || N<0 ) ਵਾਪਸ 0;
  zFilename = databaseName(zFilename);
  zFilename += sqlite3Strlen30(zFilename) + 1;
  ਜਦੋਂ( ALWAYS(zFilename) && zFilename[0] && (N--)>0 ){
    zFilename += sqlite3Strlen30(zFilename) + 1;
    zFilename += sqlite3Strlen30(zFilename) + 1;
  }
  ਵਾਪਸ zFilename[0] ? zFilename : 0;
}

/*
** Return a boolean value for a query parameter.
*/
ਪੂਰਨ sqlite3_uri_boolean(ਅਚਲ ਅੱਖਰ *zFilename, ਅਚਲ ਅੱਖਰ *zParam, ਪੂਰਨ bDflt){
  ਅਚਲ ਅੱਖਰ *z = sqlite3_uri_parameter(zFilename, zParam);
  bDflt = bDflt!=0;
  ਵਾਪਸ z ? sqlite3GetBoolean(z, bDflt) : bDflt;
}

/*
** Return a 64-bit integer value for a query parameter.
*/
sqlite3_int64 sqlite3_uri_int64(
  ਅਚਲ ਅੱਖਰ *zFilename,    /* Filename as passed to xOpen */
  ਅਚਲ ਅੱਖਰ *zParam,       /* URI parameter sought */
  sqlite3_int64 bDflt       /* return if parameter is missing */
){
  ਅਚਲ ਅੱਖਰ *z = sqlite3_uri_parameter(zFilename, zParam);
  sqlite3_int64 v;
  ਜੇ( z && sqlite3_Dec_Or_Hex_ਮੰਜ਼ਿਲ_I64(z, &v)==0 ){
    bDflt = v;
  }
  ਵਾਪਸ bDflt;
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
ਅਚਲ ਅੱਖਰ *sqlite3_filename_database(ਅਚਲ ਅੱਖਰ *zFilename){
  ਜੇ( zFilename==0 ) ਵਾਪਸ 0;
  ਵਾਪਸ databaseName(zFilename);
}
ਅਚਲ ਅੱਖਰ *sqlite3_filename_journal(ਅਚਲ ਅੱਖਰ *zFilename){
  ਜੇ( zFilename==0 ) ਵਾਪਸ 0;
  zFilename = databaseName(zFilename);
  zFilename += sqlite3Strlen30(zFilename) + 1;
  ਜਦੋਂ( ALWAYS(zFilename) && zFilename[0] ){
    zFilename += sqlite3Strlen30(zFilename) + 1;
    zFilename += sqlite3Strlen30(zFilename) + 1;
  }
  ਵਾਪਸ zFilename + 1;
}
ਅਚਲ ਅੱਖਰ *sqlite3_filename_wal(ਅਚਲ ਅੱਖਰ *zFilename){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WAL
  ਅਣਵਰਤਿਆ_PARAMETER(zFilename);
  ਵਾਪਸ 0;
#ਨਹੀਂਤਾਂ
  zFilename = sqlite3_filename_journal(zFilename);
  ਜੇ( zFilename ) zFilename += sqlite3Strlen30(zFilename) + 1;
  ਵਾਪਸ zFilename;
#ਜੇ_ਅੰਤ
}

/*
** Return the Btree pointer identified by zDbName.  Return NULL if not found.
*/
Btree *sqlite3_Db_Name_ਮੰਜ਼ਿਲ_Btree(sqlite3 *db, ਅਚਲ ਅੱਖਰ *zDbName){
  ਪੂਰਨ iDb = zDbName ? sqlite3_ਲੱਭੋ_Db_Name(db, zDbName) : 0;
  ਵਾਪਸ iDb<0 ? 0 : db->aDb[iDb].pBt;
}

/*
** Return the name of the N-th database schema.  Return NULL if N is out
** of range.
*/
ਅਚਲ ਅੱਖਰ *sqlite3_db_name(sqlite3 *db, ਪੂਰਨ N){
  ਅਚਲ ਅੱਖਰ *zRet = 0;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !sqlite3SafetyCheckOk(db) ){
    (ਖਾਲੀ)SQLITE_MISUSE_BKPT;
    ਵਾਪਸ 0;
  }
#ਜੇ_ਅੰਤ
  sqlite3_mutex_enter(db->mutex);
  ਜੇ( N>=0 && N<db->nDb ){
    zRet = db->aDb[N].zDbSName;
  }
  sqlite3_mutex_leave(db->mutex);
  ਵਾਪਸ zRet;
}

/*
** Return the filename of the database associated with a database
** connection.
*/
ਅਚਲ ਅੱਖਰ *sqlite3_db_filename(sqlite3 *db, ਅਚਲ ਅੱਖਰ *zDbName){
  Btree *pBt;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !sqlite3SafetyCheckOk(db) ){
    (ਖਾਲੀ)SQLITE_MISUSE_BKPT;
    ਵਾਪਸ 0;
  }
#ਜੇ_ਅੰਤ
  pBt = sqlite3_Db_Name_ਮੰਜ਼ਿਲ_Btree(db, zDbName);
  ਵਾਪਸ pBt ? sqlite3BtreeGetFilename(pBt) : 0;
}

/*
** Return 1 if database is read-only or 0 if read/write.  Return -1 if
** no such database exists.
*/
ਪੂਰਨ sqlite3_db_readonly(sqlite3 *db, ਅਚਲ ਅੱਖਰ *zDbName){
  Btree *pBt;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !sqlite3SafetyCheckOk(db) ){
    (ਖਾਲੀ)SQLITE_MISUSE_BKPT;
    ਵਾਪਸ -1;
  }
#ਜੇ_ਅੰਤ
  pBt = sqlite3_Db_Name_ਮੰਜ਼ਿਲ_Btree(db, zDbName);
  ਵਾਪਸ pBt ? sqlite3BtreeIsReadonly(pBt) : -1;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_SNAPSHOT
/*
** Obtain a snapshot handle for the snapshot of database zDb currently
** being read by handle db.
*/
ਪੂਰਨ sqlite3_snapshot_get(
  sqlite3 *db,
  ਅਚਲ ਅੱਖਰ *zDb,
  sqlite3_snapshot **ppSnapshot
){
  ਪੂਰਨ rc = SQLITE_ERROR;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WAL

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !sqlite3SafetyCheckOk(db) ){
    ਵਾਪਸ SQLITE_MISUSE_BKPT;
  }
#ਜੇ_ਅੰਤ
  sqlite3_mutex_enter(db->mutex);

  ਜੇ( db->autoCommit==0 ){
    ਪੂਰਨ iDb = sqlite3_ਲੱਭੋ_Db_Name(db, zDb);
    ਜੇ( iDb==0 || iDb>1 ){
      Btree *pBt = db->aDb[iDb].pBt;
      ਜੇ( SQLITE_TXN_WRITE!=sqlite3BtreeTxnState(pBt) ){
        Pager *pPager = sqlite3BtreePager(pBt);
        i64 dummy = 0;
        sqlite3PagerSnapshotOpen(pPager, (sqlite3_snapshot*)&dummy);
        rc = sqlite3BtreeBeginTrans(pBt, 0, 0);
        sqlite3PagerSnapshotOpen(pPager, 0);
        ਜੇ( rc==SQLITE_OK ){
          rc = sqlite3PagerSnapshotGet(sqlite3BtreePager(pBt), ppSnapshot);
        }
      }
    }
  }

  sqlite3_mutex_leave(db->mutex);
#ਜੇ_ਅੰਤ   /* SQLITE_OMIT_WAL */
  ਵਾਪਸ rc;
}

/*
** Open a read-transaction on the snapshot identified by pSnapshot.
*/
ਪੂਰਨ sqlite3_snapshot_open(
  sqlite3 *db,
  ਅਚਲ ਅੱਖਰ *zDb,
  sqlite3_snapshot *pSnapshot
){
  ਪੂਰਨ rc = SQLITE_ERROR;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WAL

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !sqlite3SafetyCheckOk(db) ){
    ਵਾਪਸ SQLITE_MISUSE_BKPT;
  }
#ਜੇ_ਅੰਤ
  sqlite3_mutex_enter(db->mutex);
  ਜੇ( db->autoCommit==0 ){
    ਪੂਰਨ iDb;
    iDb = sqlite3_ਲੱਭੋ_Db_Name(db, zDb);
    ਜੇ( iDb==0 || iDb>1 ){
      Btree *pBt = db->aDb[iDb].pBt;
      ਜੇ( sqlite3BtreeTxnState(pBt)!=SQLITE_TXN_WRITE ){
        Pager *pPager = sqlite3BtreePager(pBt);
        ਪੂਰਨ bUnlock = 0;
        ਜੇ( sqlite3BtreeTxnState(pBt)!=SQLITE_TXN_NONE ){
          ਜੇ( db->n_Vdbe_ਸਰਗਰਮ==0 ){
            rc = sqlite3PagerSnapshotCheck(pPager, pSnapshot);
            ਜੇ( rc==SQLITE_OK ){
              bUnlock = 1;
              rc = sqlite3BtreeCommit(pBt);
            }
          }
        }ਨਹੀਂਤਾਂ{
          rc = SQLITE_OK;
        }
        ਜੇ( rc==SQLITE_OK ){
          rc = sqlite3PagerSnapshotOpen(pPager, pSnapshot);
        }
        ਜੇ( rc==SQLITE_OK ){
          rc = sqlite3BtreeBeginTrans(pBt, 0, 0);
          sqlite3PagerSnapshotOpen(pPager, 0);
        }
        ਜੇ( bUnlock ){
          sqlite3PagerSnapshotUnlock(pPager);
        }
      }
    }
  }

  sqlite3_mutex_leave(db->mutex);
#ਜੇ_ਅੰਤ   /* SQLITE_OMIT_WAL */
  ਵਾਪਸ rc;
}

/*
** Recover as many snapshots as possible from the wal file associated with
** schema zDb of database db.
*/
ਪੂਰਨ sqlite3_snapshot_recover(sqlite3 *db, ਅਚਲ ਅੱਖਰ *zDb){
  ਪੂਰਨ rc = SQLITE_ERROR;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WAL
  ਪੂਰਨ iDb;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !sqlite3SafetyCheckOk(db) ){
    ਵਾਪਸ SQLITE_MISUSE_BKPT;
  }
#ਜੇ_ਅੰਤ

  sqlite3_mutex_enter(db->mutex);
  iDb = sqlite3_ਲੱਭੋ_Db_Name(db, zDb);
  ਜੇ( iDb==0 || iDb>1 ){
    Btree *pBt = db->aDb[iDb].pBt;
    ਜੇ( SQLITE_TXN_NONE==sqlite3BtreeTxnState(pBt) ){
      rc = sqlite3BtreeBeginTrans(pBt, 0, 0);
      ਜੇ( rc==SQLITE_OK ){
        rc = sqlite3PagerSnapshotRecover(sqlite3BtreePager(pBt));
        sqlite3BtreeCommit(pBt);
      }
    }
  }
  sqlite3_mutex_leave(db->mutex);
#ਜੇ_ਅੰਤ   /* SQLITE_OMIT_WAL */
  ਵਾਪਸ rc;
}

/*
** Free a snapshot handle obtained from sqlite3_snapshot_get().
*/
ਖਾਲੀ sqlite3_snapshot_free(sqlite3_snapshot *pSnapshot){
  sqlite3_free(pSnapshot);
}
#ਜੇ_ਅੰਤ /* SQLITE_ENABLE_SNAPSHOT */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_COMPILEOPTION_DIAGS
/*
** Given the name of a compile-time option, return true if that option
** was used and false if not.
**
** The name can optionally begin with "SQLITE_" but the "SQLITE_" prefix
** is not required for a match.
*/
ਪੂਰਨ sqlite3_compileoption_ਵਰਤਿਆ(ਅਚਲ ਅੱਖਰ *zOptName){
  ਪੂਰਨ i, n;
  ਪੂਰਨ nOpt;
  ਅਚਲ ਅੱਖਰ **azCompileOpt;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( zOptName==0 ){
    (ਖਾਲੀ)SQLITE_MISUSE_BKPT;
    ਵਾਪਸ 0;
  }
#ਜੇ_ਅੰਤ

  azCompileOpt = sqlite3CompileOptions(&nOpt);

  ਜੇ( sqlite3StrNICmp(zOptName, "SQLITE_", 7)==0 ) zOptName += 7;
  n = sqlite3Strlen30(zOptName);

  /* Since nOpt is normally in single digits, a linear search is
  ** adequate. No need for a binary search. */
  ਲਈ(i=0; i<nOpt; i++){
    ਜੇ( sqlite3StrNICmp(zOptName, azCompileOpt[i], n)==0
     && sqlite3IsIdChar((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)azCompileOpt[i][n])==0
    ){
      ਵਾਪਸ 1;
    }
  }
  ਵਾਪਸ 0;
}

/*
** Return the N-th compile-time option string.  If N is out of range,
** return a NULL pointer.
*/
ਅਚਲ ਅੱਖਰ *sqlite3_compileoption_get(ਪੂਰਨ N){
  ਪੂਰਨ nOpt;
  ਅਚਲ ਅੱਖਰ **azCompileOpt;
  azCompileOpt = sqlite3CompileOptions(&nOpt);
  ਜੇ( N>=0 && N<nOpt ){
    ਵਾਪਸ azCompileOpt[N];
  }
  ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_COMPILEOPTION_DIAGS */
