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
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_FTS3
# include "fts3.h"
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_RTREE
# include "rtree.h"
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(SQLITE_ENABLE_ICU) || defined(SQLITE_ENABLE_ICU_COLLATIONS)
# include "sqliteicu.h"
#ಆದರೆ_ಅಂತ್ಯ

/*
** This is an extension initializer that is a no-op and always
** succeeds, except that it fails if the fault-simulation is set
** to 500.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ sqlite3TestExtInit(sqlite3 *db){
  (ಶೂನ್ಯ)db;
  ಹಿಂತಿರುಗಿ sqlite3FaultSim(500);
}


/*
** Forward declarations of external module initializer functions
** for modules that need them.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_FTS5
ಪೂರ್ಣಾಂಕ sqlite3Fts5Init(sqlite3*);
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STMTVTAB
ಪೂರ್ಣಾಂಕ sqlite3StmtVtabInit(sqlite3*);
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_EXTRA_AUTOEXT
ಪೂರ್ಣಾಂಕ SQLITE_EXTRA_AUTOEXT(sqlite3*);
#ಆದರೆ_ಅಂತ್ಯ
/*
** An array of pointers to extension initializer functions for
** built-in extensions.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ (*ಸ್ಥಿರಾಂಕ sqlite3BuiltinExtensions[])(sqlite3*) = {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_FTS3
  sqlite3Fts3Init,
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_FTS5
  sqlite3Fts5Init,
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(SQLITE_ENABLE_ICU) || defined(SQLITE_ENABLE_ICU_COLLATIONS)
  sqlite3IcuInit,
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_RTREE
  sqlite3RtreeInit,
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_DBPAGE_VTAB
  sqlite3_Dbpage_ರಿಜಿಸ್ಟರ್,
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_DBSTAT_VTAB
  sqlite3_Dbstat_ರಿಜಿಸ್ಟರ್,
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3TestExtInit,
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STMTVTAB
  sqlite3StmtVtabInit,
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_BYTECODE_VTAB
  sqlite3VdbeBytecodeVtabInit,
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_EXTRA_AUTOEXT
  SQLITE_EXTRA_AUTOEXT,
#ಆದರೆ_ಅಂತ್ಯ
};

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_AMALGAMATION
/* IMPLEMENTATION-OF: R-46656-45156 The sqlite3_version[] string constant
** contains the text of SQLITE_VERSION macro.
*/
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ sqlite3_ಆವೃತ್ತಿ[] = SQLITE_ಆವೃತ್ತಿ;
#ಆದರೆ_ಅಂತ್ಯ

/* IMPLEMENTATION-OF: R-53536-42575 The sqlite3_libversion() function returns
** a pointer to the to the sqlite3_version[] string constant.
*/
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3_libversion(ಶೂನ್ಯ){ ಹಿಂತಿರುಗಿ sqlite3_ಆವೃತ್ತಿ; }

/* IMPLEMENTATION-OF: R-25063-23286 The sqlite3_sourceid() function returns a
** pointer to a string constant whose value is the same as the
** SQLITE_SOURCE_ID C preprocessor macro. Except if SQLite is built using
** an edited copy of the amalgamation, then the last four characters of
** the hash might be different from SQLITE_SOURCE_ID.
*/
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3_sourceid(ಶೂನ್ಯ){ ಹಿಂತಿರುಗಿ SQLITE_ಮೂಲ_ID; }

/* IMPLEMENTATION-OF: R-35210-63508 The sqlite3_libversion_number() function
** returns an integer equal to SQLITE_VERSION_NUMBER.
*/
ಪೂರ್ಣಾಂಕ sqlite3_libversion_ಸಂಖ್ಯೆ(ಶೂನ್ಯ){ ಹಿಂತಿರುಗಿ SQLITE_ಆವೃತ್ತಿ_ಸಂಖ್ಯೆ; }

/* IMPLEMENTATION-OF: R-20790-14025 The sqlite3_threadsafe() function returns
** zero if and only if SQLite was compiled with mutexing code omitted due to
** the SQLITE_THREADSAFE compile-time option being set to 0.
*/
ಪೂರ್ಣಾಂಕ sqlite3_threadsafe(ಶೂನ್ಯ){ ಹಿಂತಿರುಗಿ SQLITE_THREADSAFE; }

/*
** When compiling the test fixture or with debugging enabled (on Win32),
** this variable being set to non-zero will cause OSTRACE macros to emit
** extra diagnostic information.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_HAVE_OS_TRACE
# ifndef SQLITE_DEBUG_OS_TRACE
#   define SQLITE_DEBUG_OS_TRACE 0
# endif
  ಪೂರ್ಣಾಂಕ sqlite3OSTrace = SQLITE_DEBUG_OS_TRACE;
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ !defined(SQLITE_OMIT_TRACE) && defined(SQLITE_ENABLE_IOTRACE)
/*
** If the following function pointer is not NULL and if
** SQLITE_ENABLE_IOTRACE is enabled, then messages describing
** I/O active are written using this function.  These messages
** are intended for debugging activity only.
*/
SQLITE_API ಶೂನ್ಯ (SQLITE_CDECL *sqlite3IoTrace)(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*, ...) = 0;
#ಆದರೆ_ಅಂತ್ಯ

/*
** If the following global variable points to a string which is the
** name of a directory, then that directory will be used to store
** temporary files.
**
** See also the "PRAGMA temp_store_directory" SQL command.
*/
ಅಕ್ಷರ *sqlite3_ತಾತ್ಕಾಲಿಕ_directory = 0;

/*
** If the following global variable points to a string which is the
** name of a directory, then that directory will be used to store
** all database files specified with a relative pathname.
**
** See also the "PRAGMA data_store_directory" SQL command.
*/
ಅಕ್ಷರ *sqlite3_ದತ್ತಾಂಶ_directory = 0;

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
ಸ್ಥಿರ SQLITE_NOINLINE ಪೂರ್ಣಾಂಕ sqlite3Initialize(ಶೂನ್ಯ){
  MUTEX_LOGIC( sqlite3_mutex *pMainMtx; )   /* The main static mutex */
  ಪೂರ್ಣಾಂಕ rc;                                   /* Result code */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_EXTRA_INIT
  ಪೂರ್ಣಾಂಕ b_ಸತತ_Extra_Init = 0;                    /* Extra initialization needed */
#ಆದರೆ_ಅಂತ್ಯ

  /* If the following assert() fails on some obscure processor/compiler
  ** combination to warn that SQLite has been mis-compiled.  If you hit
  ** this assert(), that means you need to recompile with the
  ** -DSQLITE_PTRSIZE=n compile-time option to set the correct pointer size.
  */
  ಪ್ರತಿಜ್ಞಾ( SQLITE_PTRSIZE==ಗಾತ್ರ(ಅಕ್ಷರ*) );

  /* Make sure the mutex subsystem is initialized.  If unable to
  ** initialize the mutex subsystem, return early with the error.
  ** If the system is so sick that we are unable to allocate a mutex,
  ** there is not much SQLite is going to be able to do.
  **
  ** The mutex subsystem must take care of serializing its own
  ** initialization.
  */
  rc = sqlite3MutexInit();
  ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ rc;

  /* Initialize the malloc() system and the recursive pInitMutex mutex.
  ** This operation is protected by the STATIC_MAIN mutex.  Note that
  ** MutexAlloc() is called for a static mutex prior to initializing the
  ** malloc subsystem - this implies that the allocation of a static
  ** mutex must not require support from the malloc subsystem.
  */
  MUTEX_LOGIC( pMainMtx = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  sqlite3_mutex_enter(pMainMtx);
  sqlite3GlobalConfig.isMutexInit = 1;
  ಆದರೆ( !sqlite3GlobalConfig.isMallocInit ){
    rc = sqlite3MallocInit();
  }
  ಆದರೆ( rc==SQLITE_OK ){
    sqlite3GlobalConfig.isMallocInit = 1;
    ಆದರೆ( !sqlite3GlobalConfig.pInitMutex ){
      sqlite3GlobalConfig.pInitMutex =
           sqlite3MutexAlloc(SQLITE_MUTEX_RECURSIVE);
      ಆದರೆ( sqlite3GlobalConfig.bCoreMutex && !sqlite3GlobalConfig.pInitMutex ){
        rc = SQLITE_NOMEM_BKPT;
      }
    }
  }
  ಆದರೆ( rc==SQLITE_OK ){
    sqlite3GlobalConfig.nRefInitMutex++;
  }
  sqlite3_mutex_leave(pMainMtx);

  /* If rc is not SQLITE_OK at this point, then either the malloc
  ** subsystem could not be initialized or the system failed to allocate
  ** the pInitMutex mutex. Return an error in either case.  */
  ಆದರೆ( rc!=SQLITE_OK ){
    ಹಿಂತಿರುಗಿ rc;
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
  ಆದರೆ( sqlite3GlobalConfig.isInit==0 && sqlite3GlobalConfig.ಒಳ_ಪ್ರಗತಿ==0 ){
    sqlite3GlobalConfig.ಒಳ_ಪ್ರಗತಿ = 1;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_SQLLOG
    {
      ಬಾಹ್ಯ ಶೂನ್ಯ sqlite3_init_sqllog(ಶೂನ್ಯ);
      sqlite3_init_sqllog();
    }
#ಆದರೆ_ಅಂತ್ಯ
    memset(&sqlite3BuiltinFunctions, 0, ಗಾತ್ರ(sqlite3BuiltinFunctions));
    sqlite3_ರಿಜಿಸ್ಟರ್_Builtin_Functions();
    ಆದರೆ( sqlite3GlobalConfig.isPCacheInit==0 ){
      rc = sqlite3PcacheInitialize();
    }
    ಆದರೆ( rc==SQLITE_OK ){
      sqlite3GlobalConfig.isPCacheInit = 1;
      rc = sqlite3OsInit();
    }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_DESERIALIZE
    ಆದರೆ( rc==SQLITE_OK ){
      rc = sqlite3MemdbInit();
    }
#ಆದರೆ_ಅಂತ್ಯ
    ಆದರೆ( rc==SQLITE_OK ){
      sqlite3_PCache_ಬಫರ್_ಸಿದ್ಧತೆ( sqlite3GlobalConfig.p_ಪುಟ,
          sqlite3GlobalConfig.sz_ಪುಟ, sqlite3GlobalConfig.n_ಪುಟ);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_EXTRA_INIT_MUTEXED
      {
        ಪೂರ್ಣಾಂಕ SQLITE_EXTRA_INIT_MUTEXED(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
        rc = SQLITE_EXTRA_INIT_MUTEXED(0);
      }
#ಆದರೆ_ಅಂತ್ಯ
    }
    ಆದರೆ( rc==SQLITE_OK ){
      sqlite3_ಸ್ಮರಣೆ_Barrier();
      sqlite3GlobalConfig.isInit = 1;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_EXTRA_INIT
      b_ಸತತ_Extra_Init = 1;
#ಆದರೆ_ಅಂತ್ಯ
    }
    sqlite3GlobalConfig.ಒಳ_ಪ್ರಗತಿ = 0;
  }
  sqlite3_mutex_leave(sqlite3GlobalConfig.pInitMutex);

  /* Go back under the static mutex and clean up the recursive
  ** mutex to prevent a resource leak.
  */
  sqlite3_mutex_enter(pMainMtx);
  sqlite3GlobalConfig.nRefInitMutex--;
  ಆದರೆ( sqlite3GlobalConfig.nRefInitMutex<=0 ){
    ಪ್ರತಿಜ್ಞಾ( sqlite3GlobalConfig.nRefInitMutex==0 );
    sqlite3_mutex_free(sqlite3GlobalConfig.pInitMutex);
    sqlite3GlobalConfig.pInitMutex = 0;
  }
  sqlite3_mutex_leave(pMainMtx);

  /* The following is just a sanity check to make sure SQLite has
  ** been compiled correctly.  It is important to run this code, but
  ** we don't want to run it too often and soak up CPU cycles for no
  ** reason.  So we run it once during initialization.
  */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NDEBUG
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_FLOATING_ಬಿಂದು
  /* This section of code's only "output" is via assert() statements. */
  ಆದರೆ( rc==SQLITE_OK ){
    u64 x = (((u64)1)<<63)-1;
    ದ್ವಿಗುಣ y;
    ಪ್ರತಿಜ್ಞಾ(ಗಾತ್ರ(x)==8);
    ಪ್ರತಿಜ್ಞಾ(ಗಾತ್ರ(x)==ಗಾತ್ರ(y));
    ಸ್ಮರಣೆ_ನಕಲು(&y, &x, 8);
    ಪ್ರತಿಜ್ಞಾ( sqlite3IsNaN(y) );
  }
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ

  /* Do extra initialization steps requested by the SQLITE_EXTRA_INIT
  ** compile-time option.
  */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_EXTRA_INIT
  ಆದರೆ( b_ಸತತ_Extra_Init ){
    ಪೂರ್ಣಾಂಕ SQLITE_EXTRA_INIT(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
    rc = SQLITE_EXTRA_INIT(0);
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ rc;
}
SQLITE_API ಪೂರ್ಣಾಂಕ sqlite3_initialize(ಶೂನ್ಯ){
  /* If this build does not support writable static data (WSD) natively
  ** then we have to invoke the (application-supplied) WSD initialization
  ** routine before doing anything else. */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_OMIT_WSD
  ಪೂರ್ಣಾಂಕ rc = sqlite3_wsd_init(4096, 24);
  ಆದರೆ( rc!=SQLITE_OK ){
    ಹಿಂತಿರುಗಿ rc;
  }
#ಆದರೆ_ಅಂತ್ಯ

  ಆದರೆ( sqlite3GlobalConfig.isInit ){
    /* SQLite has already been initialized.  Fast early-out. */
    sqlite3_ಸ್ಮರಣೆ_Barrier();
    ಹಿಂತಿರುಗಿ SQLITE_OK;
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* Invoke sqlite3Initialize() to do the actual work. */
    ಹಿಂತಿರುಗಿ sqlite3Initialize();
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
ಪೂರ್ಣಾಂಕ sqlite3_shutdown(ಶೂನ್ಯ){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_OMIT_WSD
  ಪೂರ್ಣಾಂಕ rc = sqlite3_wsd_init(4096, 24);
  ಆದರೆ( rc!=SQLITE_OK ){
    ಹಿಂತಿರುಗಿ rc;
  }
#ಆದರೆ_ಅಂತ್ಯ

  ಆದರೆ( sqlite3GlobalConfig.isInit ){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_EXTRA_SHUTDOWN
    ಶೂನ್ಯ SQLITE_EXTRA_SHUTDOWN(ಶೂನ್ಯ);
    SQLITE_EXTRA_SHUTDOWN();
#ಆದರೆ_ಅಂತ್ಯ
    sqlite3_os_ಕೊನೆ();
    sqlite3_reset_auto_extension();
    sqlite3GlobalConfig.isInit = 0;
  }
  ಆದರೆ( sqlite3GlobalConfig.isPCacheInit ){
    sqlite3PcacheShutdown();
    sqlite3GlobalConfig.isPCacheInit = 0;
  }
  ಆದರೆ( sqlite3GlobalConfig.isMallocInit ){
    sqlite3_Malloc_ಕೊನೆ();
    sqlite3GlobalConfig.isMallocInit = 0;

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_SHUTDOWN_DIRECTORIES
    /* The heap subsystem has now been shutdown and these values are supposed
    ** to be NULL or point to memory that was obtained from sqlite3_malloc(),
    ** which would rely on that heap subsystem; therefore, make sure these
    ** values cannot refer to heap memory that was just invalidated when the
    ** heap subsystem was shutdown.  This is only done if the current call to
    ** this function resulted in the heap subsystem actually being shutdown.
    */
    sqlite3_ದತ್ತಾಂಶ_directory = 0;
    sqlite3_ತಾತ್ಕಾಲಿಕ_directory = 0;
#ಆದರೆ_ಅಂತ್ಯ
  }
  ಆದರೆ( sqlite3GlobalConfig.isMutexInit ){
    sqlite3_Mutex_ಕೊನೆ();
    sqlite3GlobalConfig.isMutexInit = 0;
  }

  ಹಿಂತಿರುಗಿ SQLITE_OK;
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
ಪೂರ್ಣಾಂಕ sqlite3_config(ಪೂರ್ಣಾಂಕ op, ...){
  va_list ap;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;

  /* sqlite3_config() normally returns SQLITE_MISUSE if it is invoked while
  ** the SQLite library is in use.  Except, a few selected opcodes
  ** are allowed.
  */
  ಆದರೆ( sqlite3GlobalConfig.isInit ){
    ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ u64 mAnytimeConfigOption = 0
       | MASKBIT64( SQLITE_CONFIG_LOG )
       | MASKBIT64( SQLITE_CONFIG_PCACHE_HDRSZ )
    ;
    ಆದರೆ( op<0 || op>63 || (MASKBIT64(op) & mAnytimeConfigOption)==0 ){
      ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
    }
    testcase( op==SQLITE_CONFIG_LOG );
    testcase( op==SQLITE_CONFIG_PCACHE_HDRSZ );
  }

  va_start(ap, op);
  ಆಯ್ಕೆ( op ){

    /* Mutex configuration options are only available in a threadsafe
    ** compile.
    */
#ಆದರೆ defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE>0  /* IMP: R-54466-46756 */
    ಸ್ಥಿತಿ SQLITE_CONFIG_SINGLETHREAD: {
      /* EVIDENCE-OF: R-02748-19096 This option sets the threading mode to
      ** Single-thread. */
      sqlite3GlobalConfig.bCoreMutex = 0;  /* Disable mutex on core */
      sqlite3GlobalConfig.b_ತುಂಬಿದ_Mutex = 0;  /* Disable mutex on connections */
      ನಿಲ್ಲಿಸು;
    }
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE>0 /* IMP: R-20520-54086 */
    ಸ್ಥಿತಿ SQLITE_CONFIG_MULTITHREAD: {
      /* EVIDENCE-OF: R-14374-42468 This option sets the threading mode to
      ** Multi-thread. */
      sqlite3GlobalConfig.bCoreMutex = 1;  /* Enable mutex on core */
      sqlite3GlobalConfig.b_ತುಂಬಿದ_Mutex = 0;  /* Disable mutex on connections */
      ನಿಲ್ಲಿಸು;
    }
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE>0 /* IMP: R-59593-21810 */
    ಸ್ಥಿತಿ SQLITE_CONFIG_SERIALIZED: {
      /* EVIDENCE-OF: R-41220-51800 This option sets the threading mode to
      ** Serialized. */
      sqlite3GlobalConfig.bCoreMutex = 1;  /* Enable mutex on core */
      sqlite3GlobalConfig.b_ತುಂಬಿದ_Mutex = 1;  /* Enable mutex on connections */
      ನಿಲ್ಲಿಸು;
    }
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE>0 /* IMP: R-63666-48755 */
    ಸ್ಥಿತಿ SQLITE_CONFIG_MUTEX: {
      /* Specify an alternative mutex implementation */
      sqlite3GlobalConfig.mutex = *va_arg(ap, sqlite3_mutex_methods*);
      ನಿಲ್ಲಿಸು;
    }
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE>0 /* IMP: R-14450-37597 */
    ಸ್ಥಿತಿ SQLITE_CONFIG_GETMUTEX: {
      /* Retrieve the current mutex implementation */
      *va_arg(ap, sqlite3_mutex_methods*) = sqlite3GlobalConfig.mutex;
      ನಿಲ್ಲಿಸು;
    }
#ಆದರೆ_ಅಂತ್ಯ

    ಸ್ಥಿತಿ SQLITE_CONFIG_MALLOC: {
      /* EVIDENCE-OF: R-55594-21030 The SQLITE_CONFIG_MALLOC option takes a
      ** single argument which is a pointer to an instance of the
      ** sqlite3_mem_methods structure. The argument specifies alternative
      ** low-level memory allocation routines to be used in place of the memory
      ** allocation routines built into SQLite. */
      sqlite3GlobalConfig.m = *va_arg(ap, sqlite3_mem_methods*);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ SQLITE_CONFIG_GETMALLOC: {
      /* EVIDENCE-OF: R-51213-46414 The SQLITE_CONFIG_GETMALLOC option takes a
      ** single argument which is a pointer to an instance of the
      ** sqlite3_mem_methods structure. The sqlite3_mem_methods structure is
      ** filled with the currently defined memory allocation routines. */
      ಆದರೆ( sqlite3GlobalConfig.m.xMalloc==0 ) sqlite3_Mem_ಗಣ_Default();
      *va_arg(ap, sqlite3_mem_methods*) = sqlite3GlobalConfig.m;
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ SQLITE_CONFIG_MEMSTATUS: {
      ಪ್ರತಿಜ್ಞಾ( !sqlite3GlobalConfig.isInit );  /* Cannot change at runtime */
      /* EVIDENCE-OF: R-61275-35157 The SQLITE_CONFIG_MEMSTATUS option takes
      ** single argument of type int, interpreted as a boolean, which enables
      ** or disables the collection of memory allocation statistics. */
      sqlite3GlobalConfig.bMemstat = va_arg(ap, ಪೂರ್ಣಾಂಕ);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ SQLITE_CONFIG_SMALL_MALLOC: {
      sqlite3GlobalConfig.bSmallMalloc = va_arg(ap, ಪೂರ್ಣಾಂಕ)!=0;
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ SQLITE_CONFIG_PAGECACHE: {
      /* EVIDENCE-OF: R-18761-36601 There are three arguments to
      ** SQLITE_CONFIG_PAGECACHE: A pointer to 8-byte aligned memory (pMem),
      ** the size of each page cache line (sz), and the number of cache lines
      ** (N). */
      sqlite3GlobalConfig.p_ಪುಟ = va_arg(ap, ಶೂನ್ಯ*);
      sqlite3GlobalConfig.sz_ಪುಟ = va_arg(ap, ಪೂರ್ಣಾಂಕ);
      sqlite3GlobalConfig.n_ಪುಟ = va_arg(ap, ಪೂರ್ಣಾಂಕ);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ SQLITE_CONFIG_PCACHE_HDRSZ: {
      /* EVIDENCE-OF: R-39100-27317 The SQLITE_CONFIG_PCACHE_HDRSZ option takes
      ** a single parameter which is a pointer to an integer and writes into
      ** that integer the number of extra bytes per page required for each page
      ** in SQLITE_CONFIG_PAGECACHE. */
      *va_arg(ap, ಪೂರ್ಣಾಂಕ*) =
          sqlite3_ಶೀರ್ಷಿಕೆ_ಗಾತ್ರ_Btree() +
          sqlite3_ಶೀರ್ಷಿಕೆ_ಗಾತ್ರ_Pcache() +
          sqlite3_ಶೀರ್ಷಿಕೆ_ಗಾತ್ರ_Pcache1();
      ನಿಲ್ಲಿಸು;
    }

    ಸ್ಥಿತಿ SQLITE_CONFIG_PCACHE: {
      /* no-op */
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ SQLITE_CONFIG_GETPCACHE: {
      /* now an error */
      rc = SQLITE_ERROR;
      ನಿಲ್ಲಿಸು;
    }

    ಸ್ಥಿತಿ SQLITE_CONFIG_PCACHE2: {
      /* EVIDENCE-OF: R-63325-48378 The SQLITE_CONFIG_PCACHE2 option takes a
      ** single argument which is a pointer to an sqlite3_pcache_methods2
      ** object. This object specifies the interface to a custom page cache
      ** implementation. */
      sqlite3GlobalConfig.pcache2 = *va_arg(ap, sqlite3_pcache_methods2*);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ SQLITE_CONFIG_GETPCACHE2: {
      /* EVIDENCE-OF: R-22035-46182 The SQLITE_CONFIG_GETPCACHE2 option takes a
      ** single argument which is a pointer to an sqlite3_pcache_methods2
      ** object. SQLite copies of the current page cache implementation into
      ** that object. */
      ಆದರೆ( sqlite3GlobalConfig.pcache2.xInit==0 ){
        sqlite3_PCache_ಗಣ_Default();
      }
      *va_arg(ap, sqlite3_pcache_methods2*) = sqlite3GlobalConfig.pcache2;
      ನಿಲ್ಲಿಸು;
    }

/* EVIDENCE-OF: R-06626-12911 The SQLITE_CONFIG_HEAP option is only
** available if SQLite is compiled with either SQLITE_ENABLE_MEMSYS3 or
** SQLITE_ENABLE_MEMSYS5 and returns SQLITE_ERROR if invoked otherwise. */
#ಆದರೆ defined(SQLITE_ENABLE_MEMSYS3) || defined(SQLITE_ENABLE_MEMSYS5)
    ಸ್ಥಿತಿ SQLITE_CONFIG_ರಾಶಿ: {
      /* EVIDENCE-OF: R-19854-42126 There are three arguments to
      ** SQLITE_CONFIG_HEAP: An 8-byte aligned pointer to the memory, the
      ** number of bytes in the memory buffer, and the minimum allocation size.
      */
      sqlite3GlobalConfig.p_ರಾಶಿ = va_arg(ap, ಶೂನ್ಯ*);
      sqlite3GlobalConfig.n_ರಾಶಿ = va_arg(ap, ಪೂರ್ಣಾಂಕ);
      sqlite3GlobalConfig.mnReq = va_arg(ap, ಪೂರ್ಣಾಂಕ);

      ಆದರೆ( sqlite3GlobalConfig.mnReq<1 ){
        sqlite3GlobalConfig.mnReq = 1;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( sqlite3GlobalConfig.mnReq>(1<<12) ){
        /* cap min request size at 2^12 */
        sqlite3GlobalConfig.mnReq = (1<<12);
      }

      ಆದರೆ( sqlite3GlobalConfig.p_ರಾಶಿ==0 ){
        /* EVIDENCE-OF: R-49920-60189 If the first pointer (the memory pointer)
        ** is NULL, then SQLite reverts to using its default memory allocator
        ** (the system malloc() implementation), undoing any prior invocation of
        ** SQLITE_CONFIG_MALLOC.
        **
        ** Setting sqlite3GlobalConfig.m to all zeros will cause malloc to
        ** revert to its default implementation when sqlite3_initialize() is run
        */
        memset(&sqlite3GlobalConfig.m, 0, ಗಾತ್ರ(sqlite3GlobalConfig.m));
      }ಇಲ್ಲದಿದ್ದರೆ{
        /* EVIDENCE-OF: R-61006-08918 If the memory pointer is not NULL then the
        ** alternative memory allocator is engaged to handle all of SQLites
        ** memory allocation needs. */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_MEMSYS3
        sqlite3GlobalConfig.m = *sqlite3MemGetMemsys3();
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_MEMSYS5
        sqlite3GlobalConfig.m = *sqlite3MemGetMemsys5();
#ಆದರೆ_ಅಂತ್ಯ
      }
      ನಿಲ್ಲಿಸು;
    }
#ಆದರೆ_ಅಂತ್ಯ

    ಸ್ಥಿತಿ SQLITE_CONFIG_LOOKASIDE: {
      sqlite3GlobalConfig.szLookaside = va_arg(ap, ಪೂರ್ಣಾಂಕ);
      sqlite3GlobalConfig.nLookaside = va_arg(ap, ಪೂರ್ಣಾಂಕ);
      ನಿಲ್ಲಿಸು;
    }
   
    /* Record a pointer to the logger function and its first argument.
    ** The default is NULL.  Logging is disabled if the function pointer is
    ** NULL.
    */
    ಸ್ಥಿತಿ SQLITE_CONFIG_LOG: {
      /* MSVC is picky about pulling func ptrs from va lists.
      ** http://support.microsoft.com/kb/47961
      ** sqlite3GlobalConfig.xLog = va_arg(ap, void(*)(void*,int,const char*));
      */
      ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಶೂನ್ಯ(*LOGFUNC_t)(ಶೂನ್ಯ*,ಪೂರ್ಣಾಂಕ,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
      LOGFUNC_t xLog = va_arg(ap, LOGFUNC_t);
      ಶೂನ್ಯ *pLogArg = va_arg(ap, ಶೂನ್ಯ*);
      AtomicStore(&sqlite3GlobalConfig.xLog, xLog);
      AtomicStore(&sqlite3GlobalConfig.pLogArg, pLogArg);
      ನಿಲ್ಲಿಸು;
    }

    /* EVIDENCE-OF: R-55548-33817 The compile-time setting for URI filenames
    ** can be changed at start-time using the
    ** sqlite3_config(SQLITE_CONFIG_URI,1) or
    ** sqlite3_config(SQLITE_CONFIG_URI,0) configuration calls.
    */
    ಸ್ಥಿತಿ SQLITE_CONFIG_URI: {
      /* EVIDENCE-OF: R-25451-61125 The SQLITE_CONFIG_URI option takes a single
      ** argument of type int. If non-zero, then URI handling is globally
      ** enabled. If the parameter is zero, then URI handling is globally
      ** disabled. */
      ಪೂರ್ಣಾಂಕ bOpenUri = va_arg(ap, ಪೂರ್ಣಾಂಕ);
      AtomicStore(&sqlite3GlobalConfig.bOpenUri, bOpenUri);
      ನಿಲ್ಲಿಸು;
    }

    ಸ್ಥಿತಿ SQLITE_CONFIG_COVERING_ಸೂಚ್ಯಂಕ_ಪರಿಶೀಲನೆ: {
      /* EVIDENCE-OF: R-36592-02772 The SQLITE_CONFIG_COVERING_INDEX_SCAN
      ** option takes a single integer argument which is interpreted as a
      ** boolean in order to enable or disable the use of covering indices for
      ** full table scans in the query optimizer. */
      sqlite3GlobalConfig.bUseCis = va_arg(ap, ಪೂರ್ಣಾಂಕ);
      ನಿಲ್ಲಿಸು;
    }

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_SQLLOG
    ಸ್ಥಿತಿ SQLITE_CONFIG_SQLLOG: {
      ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಶೂನ್ಯ(*SQLLOGFUNC_t)(ಶೂನ್ಯ*, sqlite3*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*, ಪೂರ್ಣಾಂಕ);
      sqlite3GlobalConfig.xSqllog = va_arg(ap, SQLLOGFUNC_t);
      sqlite3GlobalConfig.pSqllogArg = va_arg(ap, ಶೂನ್ಯ *);
      ನಿಲ್ಲಿಸು;
    }
#ಆದರೆ_ಅಂತ್ಯ

    ಸ್ಥಿತಿ SQLITE_CONFIG_MMAP_ಗಾತ್ರ: {
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
      ಆದರೆ( mxMmap<0 || mxMmap>SQLITE_MAX_MMAP_ಗಾತ್ರ ){
        mxMmap = SQLITE_MAX_MMAP_ಗಾತ್ರ;
      }
      ಆದರೆ( szMmap<0 ) szMmap = SQLITE_DEFAULT_MMAP_ಗಾತ್ರ;
      ಆದರೆ( szMmap>mxMmap) szMmap = mxMmap;
      sqlite3GlobalConfig.mxMmap = mxMmap;
      sqlite3GlobalConfig.szMmap = szMmap;
      ನಿಲ್ಲಿಸು;
    }

#ಆದರೆ SQLITE_OS_WIN && defined(SQLITE_WIN32_MALLOC) /* IMP: R-04780-55815 */
    ಸ್ಥಿತಿ SQLITE_CONFIG_WIN32_HEAPSIZE: {
      /* EVIDENCE-OF: R-34926-03360 SQLITE_CONFIG_WIN32_HEAPSIZE takes a 32-bit
      ** unsigned integer value that specifies the maximum size of the created
      ** heap. */
      sqlite3GlobalConfig.n_ರಾಶಿ = va_arg(ap, ಪೂರ್ಣಾಂಕ);
      ನಿಲ್ಲಿಸು;
    }
#ಆದರೆ_ಅಂತ್ಯ

    ಸ್ಥಿತಿ SQLITE_CONFIG_PMASZ: {
      sqlite3GlobalConfig.szPma = va_arg(ap, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ);
      ನಿಲ್ಲಿಸು;
    }

    ಸ್ಥಿತಿ SQLITE_CONFIG_STMTJRNL_SPILL: {
      sqlite3GlobalConfig.nStmtSpill = va_arg(ap, ಪೂರ್ಣಾಂಕ);
      ನಿಲ್ಲಿಸು;
    }

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_SORTER_REFERENCES
    ಸ್ಥಿತಿ SQLITE_CONFIG_SORTERREF_ಗಾತ್ರ: {
      ಪೂರ್ಣಾಂಕ iVal = va_arg(ap, ಪೂರ್ಣಾಂಕ);
      ಆದರೆ( iVal<0 ){
        iVal = SQLITE_DEFAULT_SORTERREF_ಗಾತ್ರ;
      }
      sqlite3GlobalConfig.szSorterRef = (u32)iVal;
      ನಿಲ್ಲಿಸು;
    }
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_ENABLE_SORTER_REFERENCES */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_DESERIALIZE
    ಸ್ಥಿತಿ SQLITE_CONFIG_MEMDB_MAXSIZE: {
      sqlite3GlobalConfig.mx_Memdb_ಗಾತ್ರ = va_arg(ap, sqlite3_int64);
      ನಿಲ್ಲಿಸು;
    }
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_DESERIALIZE */

    ಸ್ಥಿತಿ SQLITE_CONFIG_ROWID_ಒಳ_VIEW: {
      ಪೂರ್ಣಾಂಕ *pVal = va_arg(ap,ಪೂರ್ಣಾಂಕ*);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ALLOW_ROWID_ಒಳ_VIEW
      ಆದರೆ( 0==*pVal ) sqlite3GlobalConfig.mNoVisibleRowid = TF_NoVisibleRowid;
      ಆದರೆ( 1==*pVal ) sqlite3GlobalConfig.mNoVisibleRowid = 0;
      *pVal = (sqlite3GlobalConfig.mNoVisibleRowid==0);
#ಇಲ್ಲದಿದ್ದರೆ
      *pVal = 0;
#ಆದರೆ_ಅಂತ್ಯ
      ನಿಲ್ಲಿಸು;
    }

    ಪೂರ್ವನಿಯೋಜಿತ: {
      rc = SQLITE_ERROR;
      ನಿಲ್ಲಿಸು;
    }
  }
  va_end(ap);
  ಹಿಂತಿರುಗಿ rc;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಸಿದ್ಧತೆ_Lookaside(
  sqlite3 *db,    /* Database connection being configured */
  ಶೂನ್ಯ *pBuf,     /* Memory to use for lookaside.  May be NULL */
  ಪೂರ್ಣಾಂಕ sz,         /* Desired size of each lookaside memory slot */
  ಪೂರ್ಣಾಂಕ cnt         /* Number of slots to allocate */
){
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_LOOKASIDE
  ಶೂನ್ಯ *p_ಆರಂಭ;          /* Start of the lookaside buffer */
  sqlite3_int64 szAlloc; /* Total space set aside for lookaside memory */
  ಪೂರ್ಣಾಂಕ nBig;              /* Number of full-size slots */
  ಪೂರ್ಣಾಂಕ nSm;               /* Number smaller LOOKASIDE_SMALL-byte slots */
 
  ಆದರೆ( sqlite3_Lookaside_ಬಳಸಿದ(db,0)>0 ){
    ಹಿಂತಿರುಗಿ SQLITE_BUSY;
  }
  /* Free any existing lookaside buffer for this handle before
  ** allocating a new one so we don't have to have space for
  ** both at the same time.
  */
  ಆದರೆ( db->lookaside.bMalloced ){
    sqlite3_free(db->lookaside.p_ಆರಂಭ);
  }
  /* The size of a lookaside slot after ROUNDDOWN8 needs to be larger
  ** than a pointer and small enough to fit in a u16.
  */
  sz = ROUNDDOWN8(sz);
  ಆದರೆ( sz<=(ಪೂರ್ಣಾಂಕ)ಗಾತ್ರ(Lookaside_ಸ್ಥಾನ*) ) sz = 0;
  ಆದರೆ( sz>65528 ) sz = 65528;
  /* Count must be at least 1 to be useful, but not so large as to use
  ** more than 0x7fff0000 total bytes for lookaside. */
  ಆದರೆ( cnt<1 ) cnt = 0;
  ಆದರೆ( sz>0 && cnt>(0x7fff0000/sz) ) cnt = 0x7fff0000/sz;
  szAlloc = (i64)sz*(i64)cnt;
  ಆದರೆ( szAlloc==0 ){
    sz = 0;
    p_ಆರಂಭ = 0;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pBuf==0 ){
    sqlite3BeginBenignMalloc();
    p_ಆರಂಭ = sqlite3Malloc( szAlloc );
    sqlite3_ಕೊನೆ_Benign_Malloc();
    ಆದರೆ( p_ಆರಂಭ ) szAlloc = sqlite3_Malloc_ಗಾತ್ರ(p_ಆರಂಭ);
  }ಇಲ್ಲದಿದ್ದರೆ{
    p_ಆರಂಭ = pBuf;
  }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_TWOSIZE_LOOKASIDE
  ಆದರೆ( sz>=LOOKASIDE_SMALL*3 ){
    nBig = szAlloc/(3*LOOKASIDE_SMALL+sz);
    nSm = (szAlloc - (i64)sz*(i64)nBig)/LOOKASIDE_SMALL;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( sz>=LOOKASIDE_SMALL*2 ){
    nBig = szAlloc/(LOOKASIDE_SMALL+sz);
    nSm = (szAlloc - (i64)sz*(i64)nBig)/LOOKASIDE_SMALL;
  }ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
  ಆದರೆ( sz>0 ){
    nBig = szAlloc/sz;
    nSm = 0;
  }ಇಲ್ಲದಿದ್ದರೆ{
    nBig = nSm = 0;
  }
  db->lookaside.p_ಆರಂಭ = p_ಆರಂಭ;
  db->lookaside.pInit = 0;
  db->lookaside.pFree = 0;
  db->lookaside.sz = (u16)sz;
  db->lookaside.szTrue = (u16)sz;
  ಆದರೆ( p_ಆರಂಭ ){
    ಪೂರ್ಣಾಂಕ i;
    Lookaside_ಸ್ಥಾನ *p;
    ಪ್ರತಿಜ್ಞಾ( sz > (ಪೂರ್ಣಾಂಕ)ಗಾತ್ರ(Lookaside_ಸ್ಥಾನ*) );
    p = (Lookaside_ಸ್ಥಾನ*)p_ಆರಂಭ;
    ಗಾಗಿ(i=0; i<nBig; i++){
      p->p_ಮುಂದಿನ = db->lookaside.pInit;
      db->lookaside.pInit = p;
      p = (Lookaside_ಸ್ಥಾನ*)&((u8*)p)[sz];
    }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_TWOSIZE_LOOKASIDE
    db->lookaside.pSmallInit = 0;
    db->lookaside.pSmallFree = 0;
    db->lookaside.p_ಮಧ್ಯ = p;
    ಗಾಗಿ(i=0; i<nSm; i++){
      p->p_ಮುಂದಿನ = db->lookaside.pSmallInit;
      db->lookaside.pSmallInit = p;
      p = (Lookaside_ಸ್ಥಾನ*)&((u8*)p)[LOOKASIDE_SMALL];
    }
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
    ಪ್ರತಿಜ್ಞಾ( ((uptr)p)<=szAlloc + (uptr)p_ಆರಂಭ );
    db->lookaside.p_ಕೊನೆ = p;
    db->lookaside.bDisable = 0;
    db->lookaside.bMalloced = pBuf==0 ?1:0;
    db->lookaside.n_ಸ್ಥಾನ = nBig+nSm;
  }ಇಲ್ಲದಿದ್ದರೆ{
    db->lookaside.p_ಆರಂಭ = 0;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_TWOSIZE_LOOKASIDE
    db->lookaside.pSmallInit = 0;
    db->lookaside.pSmallFree = 0;
    db->lookaside.p_ಮಧ್ಯ = 0;
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
    db->lookaside.p_ಕೊನೆ = 0;
    db->lookaside.bDisable = 1;
    db->lookaside.sz = 0;
    db->lookaside.bMalloced = 0;
    db->lookaside.n_ಸ್ಥಾನ = 0;
  }
  db->lookaside.p_True_ಕೊನೆ = db->lookaside.p_ಕೊನೆ;
  ಪ್ರತಿಜ್ಞಾ( sqlite3_Lookaside_ಬಳಸಿದ(db,0)==0 );
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_LOOKASIDE */
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Return the mutex associated with a database connection.
*/
sqlite3_mutex *sqlite3_db_mutex(sqlite3 *db){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !sqlite3SafetyCheckOk(db) ){
    (ಶೂನ್ಯ)SQLITE_MISUSE_BKPT;
    ಹಿಂತಿರುಗಿ 0;
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ db->mutex;
}

/*
** Free up as much memory as we can from the given database
** connection.
*/
ಪೂರ್ಣಾಂಕ sqlite3_db_release_ಸ್ಮರಣೆ(sqlite3 *db){
  ಪೂರ್ಣಾಂಕ i;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !sqlite3SafetyCheckOk(db) ) ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_mutex_enter(db->mutex);
  sqlite3BtreeEnterAll(db);
  ಗಾಗಿ(i=0; i<db->nDb; i++){
    Btree *pBt = db->aDb[i].pBt;
    ಆದರೆ( pBt ){
      Pager *pPager = sqlite3BtreePager(pBt);
      sqlite3PagerShrink(pPager);
    }
  }
  sqlite3BtreeLeaveAll(db);
  sqlite3_mutex_leave(db->mutex);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Flush any dirty pages in the pager-cache for any attached database
** to disk.
*/
ಪೂರ್ಣಾಂಕ sqlite3_db_cacheflush(sqlite3 *db){
  ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  ಪೂರ್ಣಾಂಕ b_ನೋಡಿದ_Busy = 0;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !sqlite3SafetyCheckOk(db) ) ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_mutex_enter(db->mutex);
  sqlite3BtreeEnterAll(db);
  ಗಾಗಿ(i=0; rc==SQLITE_OK && i<db->nDb; i++){
    Btree *pBt = db->aDb[i].pBt;
    ಆದರೆ( pBt && sqlite3BtreeTxnState(pBt)==SQLITE_TXN_WRITE ){
      Pager *pPager = sqlite3BtreePager(pBt);
      rc = sqlite3PagerFlush(pPager);
      ಆದರೆ( rc==SQLITE_BUSY ){
        b_ನೋಡಿದ_Busy = 1;
        rc = SQLITE_OK;
      }
    }
  }
  sqlite3BtreeLeaveAll(db);
  sqlite3_mutex_leave(db->mutex);
  ಹಿಂತಿರುಗಿ ((rc==SQLITE_OK && b_ನೋಡಿದ_Busy) ? SQLITE_BUSY : rc);
}

/*
** Configuration settings for an individual database connection
*/
ಪೂರ್ಣಾಂಕ sqlite3_db_config(sqlite3 *db, ಪೂರ್ಣಾಂಕ op, ...){
  va_list ap;
  ಪೂರ್ಣಾಂಕ rc;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !sqlite3SafetyCheckOk(db) ) ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_mutex_enter(db->mutex);
  va_start(ap, op);
  ಆಯ್ಕೆ( op ){
    ಸ್ಥಿತಿ SQLITE_DBCONFIG_MAINDBNAME: {
      /* IMP: R-06824-28531 */
      /* IMP: R-36257-52125 */
      db->aDb[0].zDbSName = va_arg(ap,ಅಕ್ಷರ*);
      rc = SQLITE_OK;
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ SQLITE_DBCONFIG_LOOKASIDE: {
      ಶೂನ್ಯ *pBuf = va_arg(ap, ಶೂನ್ಯ*); /* IMP: R-26835-10964 */
      ಪೂರ್ಣಾಂಕ sz = va_arg(ap, ಪೂರ್ಣಾಂಕ);       /* IMP: R-47871-25994 */
      ಪೂರ್ಣಾಂಕ cnt = va_arg(ap, ಪೂರ್ಣಾಂಕ);      /* IMP: R-04460-53386 */
      rc = ಸಿದ್ಧತೆ_Lookaside(db, pBuf, sz, cnt);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ SQLITE_DBCONFIG_FP_DIGITS: {
      ಪೂರ್ಣಾಂಕ n_ಒಳ = va_arg(ap, ಪೂರ್ಣಾಂಕ);
      ಪೂರ್ಣಾಂಕ *p_ಹೊರಹೋಗು = va_arg(ap, ಪೂರ್ಣಾಂಕ*);
      ಆದರೆ( n_ಒಳ>3 && n_ಒಳ<24 ) db->nFpDigit = (u8)n_ಒಳ;
      ಆದರೆ( p_ಹೊರಹೋಗು ) *p_ಹೊರಹೋಗು = db->nFpDigit;
      rc = SQLITE_OK;
      ನಿಲ್ಲಿಸು;
    }
    ಪೂರ್ವನಿಯೋಜಿತ: {
      ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ {
        ಪೂರ್ಣಾಂಕ op;      /* The opcode */
        u64 ಮುಖವಾಡ;    /* Mask of the bit in sqlite3.flags to set/clear */
      } aFlagOp[] = {
        { SQLITE_DBCONFIG_ENABLE_FKEY,           SQLITE_Foreign_ಕೀಲಿಗಳು    },
        { SQLITE_DBCONFIG_ENABLE_TRIGGER,        SQLITE_EnableTrigger  },
        { SQLITE_DBCONFIG_ENABLE_VIEW,           SQLITE_EnableView     },
        { SQLITE_DBCONFIG_ENABLE_FTS3_TOKENIZER, SQLITE_Fts3Tokenizer  },
        { SQLITE_DBCONFIG_ENABLE_ತುಂಬು_EXTENSION, SQLITE_ತುಂಬು_Extension  },
        { SQLITE_DBCONFIG_NO_CKPT_ON_CLOSE,      SQLITE_NoCkptOnClose  },
        { SQLITE_DBCONFIG_ENABLE_QPSG,           SQLITE_EnableQPSG     },
        { SQLITE_DBCONFIG_TRIGGER_EQP,           SQLITE_TriggerEQP     },
        { SQLITE_DBCONFIG_RESET_DATABASE,        SQLITE_ResetDatabase  },
        { SQLITE_DBCONFIG_DEFENSIVE,             SQLITE_Defensive      },
        { SQLITE_DBCONFIG_WRITABLE_SCHEMA,       SQLITE_WriteSchema|
                                                 SQLITE_NoSchemaError  },
        { SQLITE_DBCONFIG_LEGACY_ALTER_ಕೋಷ್ಟಕ,    SQLITE_LegacyAlter    },
        { SQLITE_DBCONFIG_DQS_DDL,               SQLITE_DqsDDL         },
        { SQLITE_DBCONFIG_DQS_DML,               SQLITE_DqsDML         },
        { SQLITE_DBCONFIG_LEGACY_FILE_FORMAT,    SQLITE_LegacyFileFmt  },
        { SQLITE_DBCONFIG_TRUSTED_SCHEMA,        SQLITE_TrustedSchema  },
        { SQLITE_DBCONFIG_STMT_SCANSTATUS,       SQLITE_Stmt_ಪರಿಶೀಲನೆ_ಸ್ಥಿತಿ },
        { SQLITE_DBCONFIG_ಹಿಮ್ಮುಖ_SCANORDER,     SQLITE_ಹಿಮ್ಮುಖ_ಕ್ರಮ   },
        { SQLITE_DBCONFIG_ENABLE_ATTACH_CREATE,  SQLITE_AttachCreate   },
        { SQLITE_DBCONFIG_ENABLE_ATTACH_WRITE,   SQLITE_AttachWrite    },
        { SQLITE_DBCONFIG_ENABLE_COMMENTS,       SQLITE_Comments       },
      };
      ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;
      rc = SQLITE_ERROR; /* IMP: R-42790-23372 */
      ಗಾಗಿ(i=0; i<Array_ಗಾತ್ರ(aFlagOp); i++){
        ಆದರೆ( aFlagOp[i].op==op ){
          ಪೂರ್ಣಾಂಕ onoff = va_arg(ap, ಪೂರ್ಣಾಂಕ);
          ಪೂರ್ಣಾಂಕ *pRes = va_arg(ap, ಪೂರ್ಣಾಂಕ*);
          u64 oldFlags = db->flags;
          ಆದರೆ( onoff>0 ){
            db->flags |= aFlagOp[i].ಮುಖವಾಡ;
          }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( onoff==0 ){
            db->flags &= ~(u64)aFlagOp[i].ಮುಖವಾಡ;
          }
          ಆದರೆ( oldFlags!=db->flags ){
            sqlite3ExpirePreparedStatements(db, 0);
          }
          ಆದರೆ( pRes ){
            *pRes = (db->flags & aFlagOp[i].ಮುಖವಾಡ)!=0;
          }
          rc = SQLITE_OK;
          ನಿಲ್ಲಿಸು;
        }
      }
      ನಿಲ್ಲಿಸು;
    }
  }
  va_end(ap);
  sqlite3_mutex_leave(db->mutex);
  ಹಿಂತಿರುಗಿ rc;
}

/*
** This is the default collating function named "BINARY" which is always
** available.
*/
ಪೂರ್ಣಾಂಕ sqlite3_ದ್ವಿಮಾನ_Compare(
  ಶೂನ್ಯ *Not_ಬಳಸಿದ,
  ಪೂರ್ಣಾಂಕ nKey1, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *pKey1,
  ಪೂರ್ಣಾಂಕ nKey2, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *pKey2
){
  ಪೂರ್ಣಾಂಕ rc, n;
  ಬಳಸದ_PARAMETER(Not_ಬಳಸಿದ);
  n = nKey1<nKey2 ? nKey1 : nKey2;
  /* EVIDENCE-OF: R-65033-28449 The built-in BINARY collation compares
  ** strings byte by byte using the memcmp() function from the standard C
  ** library. */
  ಪ್ರತಿಜ್ಞಾ( pKey1 && pKey2 );
  rc = memcmp(pKey1, pKey2, n);
  ಆದರೆ( rc==0 ){
    rc = nKey1 - nKey2;
  }
  ಹಿಂತಿರುಗಿ rc;
}

/*
** This is the collating function named "RTRIM" which is always
** available.  Ignore trailing spaces.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ rtrimCollFunc(
  ಶೂನ್ಯ *pUser,
  ಪೂರ್ಣಾಂಕ nKey1, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *pKey1,
  ಪೂರ್ಣಾಂಕ nKey2, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *pKey2
){
  ಸ್ಥಿರಾಂಕ u8 *pK1 = (ಸ್ಥಿರಾಂಕ u8*)pKey1;
  ಸ್ಥಿರಾಂಕ u8 *pK2 = (ಸ್ಥಿರಾಂಕ u8*)pKey2;
  ಆಗ( nKey1 && pK1[nKey1-1]==' ' ) nKey1--;
  ಆಗ( nKey2 && pK2[nKey2-1]==' ' ) nKey2--;
  ಹಿಂತಿರುಗಿ sqlite3_ದ್ವಿಮಾನ_Compare(pUser, nKey1, pKey1, nKey2, pKey2);
}

/*
** Return true if CollSeq is the default built-in BINARY.
*/
#ಆದರೆ !SQLITE_ಬಳಸುತ್ತದೆ_INLINE
ಪೂರ್ಣಾಂಕ sqlite3_Is_ದ್ವಿಮಾನ(ಸ್ಥಿರಾಂಕ CollSeq *p){
  ಪ್ರತಿಜ್ಞಾ( p==0 || p->xCmp!=sqlite3_ದ್ವಿಮಾನ_Compare
       || strcmp(p->zName,"BINARY")==0 );
  ಹಿಂತಿರುಗಿ p==0 || p->xCmp==sqlite3_ದ್ವಿಮಾನ_Compare;
}
#ಆದರೆ_ಅಂತ್ಯ

/*
** Another built-in collating sequence: NOCASE.
**
** This collating sequence is intended to be used for "case independent
** comparison". SQLite's knowledge of upper and lower case equivalents
** extends only to the 26 characters used in the English language.
**
** At the moment there is only a UTF-8 implementation.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ nocaseCollatingFunc(
  ಶೂನ್ಯ *Not_ಬಳಸಿದ,
  ಪೂರ್ಣಾಂಕ nKey1, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *pKey1,
  ಪೂರ್ಣಾಂಕ nKey2, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *pKey2
){
  ಪೂರ್ಣಾಂಕ r = sqlite3StrNICmp(
      (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *)pKey1, (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *)pKey2, (nKey1<nKey2)?nKey1:nKey2);
  ಬಳಸದ_PARAMETER(Not_ಬಳಸಿದ);
  ಆದರೆ( 0==r ){
    r = nKey1-nKey2;
  }
  ಹಿಂತಿರುಗಿ r;
}

/*
** Return the ROWID of the most recent insert
*/
sqlite_int64 sqlite3_ಕೊನೆಯ_ಸೇರಿಸು_rowid(sqlite3 *db){
  i64 iRet;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !sqlite3SafetyCheckOk(db) ){
    (ಶೂನ್ಯ)SQLITE_MISUSE_BKPT;
    ಹಿಂತಿರುಗಿ 0;
  }
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_mutex_enter(db->mutex);
  iRet = db->ಕೊನೆಯ_Rowid;
  sqlite3_mutex_leave(db->mutex);
  ಹಿಂತಿರುಗಿ iRet;
}

/*
** Set the value returned by the sqlite3_last_insert_rowid() API function.
*/
ಶೂನ್ಯ sqlite3_ಗಣ_ಕೊನೆಯ_ಸೇರಿಸು_rowid(sqlite3 *db, sqlite3_int64 iRowid){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !sqlite3SafetyCheckOk(db) ){
    (ಶೂನ್ಯ)SQLITE_MISUSE_BKPT;
    ಹಿಂತಿರುಗಿ;
  }
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_mutex_enter(db->mutex);
  db->ಕೊನೆಯ_Rowid = iRowid;
  sqlite3_mutex_leave(db->mutex);
}

/*
** Return the number of changes in the most recently executed DML
** statement.
*/
sqlite3_int64 sqlite3_changes64(sqlite3 *db){
  i64 iRet;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !sqlite3SafetyCheckOk(db) ){
    (ಶೂನ್ಯ)SQLITE_MISUSE_BKPT;
    ಹಿಂತಿರುಗಿ 0;
  }
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_mutex_enter(db->mutex);
  iRet = db->nChange;
  sqlite3_mutex_leave(db->mutex);
  ಹಿಂತಿರುಗಿ iRet;
}
ಪೂರ್ಣಾಂಕ sqlite3_changes(sqlite3 *db){
  ಹಿಂತಿರುಗಿ (ಪೂರ್ಣಾಂಕ)sqlite3_changes64(db);
}

/*
** Return the number of changes since the database handle was opened.
*/
sqlite3_int64 sqlite3_ಒಟ್ಟು_changes64(sqlite3 *db){
  i64 iRet;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !sqlite3SafetyCheckOk(db) ){
    (ಶೂನ್ಯ)SQLITE_MISUSE_BKPT;
    ಹಿಂತಿರುಗಿ 0;
  }
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_mutex_enter(db->mutex);
  iRet = db->n_ಒಟ್ಟು_Change;
  sqlite3_mutex_leave(db->mutex);
  ಹಿಂತಿರುಗಿ iRet;
}
ಪೂರ್ಣಾಂಕ sqlite3_ಒಟ್ಟು_changes(sqlite3 *db){
  ಹಿಂತಿರುಗಿ (ಪೂರ್ಣಾಂಕ)sqlite3_ಒಟ್ಟು_changes64(db);
}

/*
** Close all open savepoints. This function only manipulates fields of the
** database handle object, it does not close any savepoints that may be open
** at the b-tree/pager level.
*/
ಶೂನ್ಯ sqlite3CloseSavepoints(sqlite3 *db){
  ಆಗ( db->pSavepoint ){
    Savepoint *pTmp = db->pSavepoint;
    db->pSavepoint = pTmp->p_ಮುಂದಿನ;
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
ಸ್ಥಿರ ಶೂನ್ಯ functionDestroy(sqlite3 *db, FuncDef *p){
  FuncDestructor *pDestructor;
  ಪ್ರತಿಜ್ಞಾ( (p->funcFlags & SQLITE_FUNC_BUILTIN)==0 );
  pDestructor = p->u.pDestructor;
  ಆದರೆ( pDestructor ){
    pDestructor->nRef--;
    ಆದರೆ( pDestructor->nRef==0 ){
      pDestructor->xDestroy(pDestructor->p_User_ದತ್ತಾಂಶ);
      sqlite3DbFree(db, pDestructor);
    }
  }
}

/*
** Disconnect all sqlite3_vtab objects that belong to database connection
** db. This is called when db is being closed.
*/
ಸ್ಥಿರ ಶೂನ್ಯ disconnectAllVtab(sqlite3 *db){
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
  ಪೂರ್ಣಾಂಕ i;
  HashElem *p;
  sqlite3BtreeEnterAll(db);
  ಗಾಗಿ(i=0; i<db->nDb; i++){
    Schema *pSchema = db->aDb[i].pSchema;
    ಆದರೆ( pSchema ){
      ಗಾಗಿ(p=sqlite_Hash_ಮೊದಲ(&pSchema->tblHash); p; p=sqlite_Hash_ಮುಂದಿನ(p)){
        ಕೋಷ್ಟಕ *pTab = (ಕೋಷ್ಟಕ *)sqlite_Hash_ದತ್ತಾಂಶ(p);
        ಆದರೆ( IsVirtual(pTab) ) sqlite3VtabDisconnect(db, pTab);
      }
    }
  }
  ಗಾಗಿ(p=sqlite_Hash_ಮೊದಲ(&db->aModule); p; p=sqlite_Hash_ಮುಂದಿನ(p)){
    Module *pMod = (Module *)sqlite_Hash_ದತ್ತಾಂಶ(p);
    ಆದರೆ( pMod->pEpoTab ){
      sqlite3VtabDisconnect(db, pMod->pEpoTab);
    }
  }
  sqlite3VtabUnlockList(db);
  sqlite3BtreeLeaveAll(db);
#ಇಲ್ಲದಿದ್ದರೆ
  ಬಳಸದ_PARAMETER(db);
#ಆದರೆ_ಅಂತ್ಯ
}

/*
** Return TRUE if database connection db has unfinalized prepared
** statements or unfinished sqlite3_backup objects. 
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ connectionIsBusy(sqlite3 *db){
  ಪೂರ್ಣಾಂಕ j;
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(db->mutex) );
  ಆದರೆ( db->pVdbe ) ಹಿಂತಿರುಗಿ 1;
  ಗಾಗಿ(j=0; j<db->nDb; j++){
    Btree *pBt = db->aDb[j].pBt;
    ಆದರೆ( pBt && sqlite3_Btree_Is_ಒಳ_Backup(pBt) ) ಹಿಂತಿರುಗಿ 1;
  }
  ಹಿಂತಿರುಗಿ 0;
}

/*
** Close an existing SQLite database
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ sqlite3Close(sqlite3 *db, ಪೂರ್ಣಾಂಕ forceZombie){
  ಆದರೆ( !db ){
    /* EVIDENCE-OF: R-63257-11740 Calling sqlite3_close() or
    ** sqlite3_close_v2() with a NULL pointer argument is a harmless no-op. */
    ಹಿಂತಿರುಗಿ SQLITE_OK;
  }
  ಆದರೆ( !sqlite3SafetyCheckSickOrOk(db) ){
    ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(db->mutex);
  ಆದರೆ( db->mTrace & SQLITE_TRACE_CLOSE ){
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
  ಆದರೆ( !forceZombie && connectionIsBusy(db) ){
    sqlite3_Error_With_ಸಂದೇಶ(db, SQLITE_BUSY, "unable to close due to unfinalized "
       "statements or unfinished backups");
    sqlite3_mutex_leave(db->mutex);
    ಹಿಂತಿರುಗಿ SQLITE_BUSY;
  }

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_SQLLOG
  ಆದರೆ( sqlite3GlobalConfig.xSqllog ){
    /* Closing the handle. Fourth parameter is passed the value 2. */
    sqlite3GlobalConfig.xSqllog(sqlite3GlobalConfig.pSqllogArg, db, 0, 2);
  }
#ಆದರೆ_ಅಂತ್ಯ

  ಆಗ( db->p_Db_ದತ್ತಾಂಶ ){
    Db_Client_ದತ್ತಾಂಶ *p = db->p_Db_ದತ್ತಾಂಶ;
    db->p_Db_ದತ್ತಾಂಶ = p->p_ಮುಂದಿನ;
    ಪ್ರತಿಜ್ಞಾ( p->p_ದತ್ತಾಂಶ!=0 );
    ಆದರೆ( p->xDestructor ) p->xDestructor(p->p_ದತ್ತಾಂಶ);
    sqlite3_free(p);
  }

  /* Convert the connection into a zombie and then close it.
  */
  db->eOpenState = SQLITE_STATE_ZOMBIE;
  sqlite3LeaveMutexAndCloseZombie(db);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Return the transaction state for a single databse, or the maximum
** transaction state over all attached databases if zSchema is null.
*/
ಪೂರ್ಣಾಂಕ sqlite3_txn_state(sqlite3 *db, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSchema){
  ಪೂರ್ಣಾಂಕ iDb, nDb;
  ಪೂರ್ಣಾಂಕ iTxn = -1;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !sqlite3SafetyCheckOk(db) ){
    (ಶೂನ್ಯ)SQLITE_MISUSE_BKPT;
    ಹಿಂತಿರುಗಿ -1;
  }
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_mutex_enter(db->mutex);
  ಆದರೆ( zSchema ){
    nDb = iDb = sqlite3_ಕಂಡುಹಿಡಿ_Db_Name(db, zSchema);
    ಆದರೆ( iDb<0 ) nDb--;
  }ಇಲ್ಲದಿದ್ದರೆ{
    iDb = 0;
    nDb = db->nDb-1;
  }
  ಗಾಗಿ(; iDb<=nDb; iDb++){
    Btree *pBt = db->aDb[iDb].pBt;
    ಪೂರ್ಣಾಂಕ x = pBt!=0 ? sqlite3BtreeTxnState(pBt) : SQLITE_TXN_NONE;
    ಆದರೆ( x>iTxn ) iTxn = x;
  }
  sqlite3_mutex_leave(db->mutex);
  ಹಿಂತಿರುಗಿ iTxn;
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
ಪೂರ್ಣಾಂಕ sqlite3_close(sqlite3 *db){ ಹಿಂತಿರುಗಿ sqlite3Close(db,0); }
ಪೂರ್ಣಾಂಕ sqlite3_close_v2(sqlite3 *db){ ಹಿಂತಿರುಗಿ sqlite3Close(db,1); }


/*
** Close the mutex on database connection db.
**
** Furthermore, if database connection db is a zombie (meaning that there
** has been a prior call to sqlite3_close(db) or sqlite3_close_v2(db)) and
** every sqlite3_stmt has now been finalized and every sqlite3_backup has
** finished, then free all resources.
*/
ಶೂನ್ಯ sqlite3LeaveMutexAndCloseZombie(sqlite3 *db){
  HashElem *i;                    /* Hash table iterator */
  ಪೂರ್ಣಾಂಕ j;

  /* If there are outstanding sqlite3_stmt or sqlite3_backup objects
  ** or if the connection has not yet been closed by sqlite3_close_v2(),
  ** then just leave the mutex and return.
  */
  ಆದರೆ( db->eOpenState!=SQLITE_STATE_ZOMBIE || connectionIsBusy(db) ){
    sqlite3_mutex_leave(db->mutex);
    ಹಿಂತಿರುಗಿ;
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
  ಗಾಗಿ(j=0; j<db->nDb; j++){
    ರಚನೆ Db *pDb = &db->aDb[j];
    ಆದರೆ( pDb->pBt ){
      sqlite3BtreeClose(pDb->pBt);
      pDb->pBt = 0;
      ಆದರೆ( j!=1 ){
        pDb->pSchema = 0;
      }
    }
  }
  /* Clear the TEMP schema separately and last */
  ಆದರೆ( db->aDb[1].pSchema ){
    sqlite3SchemaClear(db->aDb[1].pSchema);
    ಪ್ರತಿಜ್ಞಾ( db->aDb[1].pSchema->trigHash.ಎಣಿಕೆ_2==0 );
  }
  sqlite3VtabUnlockList(db);

  /* Free up the array of auxiliary databases */
  sqlite3CollapseDatabaseArray(db);
  ಪ್ರತಿಜ್ಞಾ( db->nDb<=2 );
  ಪ್ರತಿಜ್ಞಾ( db->aDb==db->aDbStatic );

  /* Tell the code in notify.c that the connection no longer holds any
  ** locks and does not require any further unlock-notify callbacks.
  */
  sqlite3ConnectionClosed(db);

  ಗಾಗಿ(i=sqlite_Hash_ಮೊದಲ(&db->aFunc); i; i=sqlite_Hash_ಮುಂದಿನ(i)){
    FuncDef *p_ಮುಂದಿನ, *p;
    p = sqlite_Hash_ದತ್ತಾಂಶ(i);
    ಮಾಡು{
      functionDestroy(db, p);
      p_ಮುಂದಿನ = p->p_ಮುಂದಿನ;
      sqlite3DbFree(db, p);
      p = p_ಮುಂದಿನ;
    }ಆಗ( p );
  }
  sqlite3HashClear(&db->aFunc);
  ಗಾಗಿ(i=sqlite_Hash_ಮೊದಲ(&db->aCollSeq); i; i=sqlite_Hash_ಮುಂದಿನ(i)){
    CollSeq *pColl = (CollSeq *)sqlite_Hash_ದತ್ತಾಂಶ(i);
    /* Invoke any destructors registered for collation sequence user data. */
    ಗಾಗಿ(j=0; j<3; j++){
      ಆದರೆ( pColl[j].xDel ){
        pColl[j].xDel(pColl[j].pUser);
      }
    }
    sqlite3DbFree(db, pColl);
  }
  sqlite3HashClear(&db->aCollSeq);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
  ಗಾಗಿ(i=sqlite_Hash_ಮೊದಲ(&db->aModule); i; i=sqlite_Hash_ಮುಂದಿನ(i)){
    Module *pMod = (Module *)sqlite_Hash_ದತ್ತಾಂಶ(i);
    sqlite3_Vtab_Eponymous_ಕೋಷ್ಟಕ_Clear(db, pMod);
    sqlite3VtabModuleUnref(db, pMod);
  }
  sqlite3HashClear(&db->aModule);
#ಆದರೆ_ಅಂತ್ಯ

  sqlite3Error(db, SQLITE_OK); /* Deallocates any cached error strings. */
  sqlite3_ಮೌಲ್ಯ_Free(db->pErr);
  sqlite3CloseExtensions(db);

  db->eOpenState = SQLITE_STATE_ERROR;

  /* The temp-database schema is allocated differently from the other schema
  ** objects (using sqliteMalloc() directly, instead of sqlite3BtreeSchema()).
  ** So it needs to be freed here. Todo: Why not roll the temp schema into
  ** the same sqliteMalloc() as the one that allocates the database
  ** structure?
  */
  sqlite3DbFree(db, db->aDb[1].pSchema);
  ಆದರೆ( db->xAutovacDestr ){
    db->xAutovacDestr(db->p_Autovac_ಪುಟಗಳು_Arg);
  }
  sqlite3_mutex_leave(db->mutex);
  db->eOpenState = SQLITE_STATE_CLOSED;
  sqlite3_mutex_free(db->mutex);
  ಪ್ರತಿಜ್ಞಾ( sqlite3_Lookaside_ಬಳಸಿದ(db,0)==0 );
  ಆದರೆ( db->lookaside.bMalloced ){
    sqlite3_free(db->lookaside.p_ಆರಂಭ);
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
ಶೂನ್ಯ sqlite3RollbackAll(sqlite3 *db, ಪೂರ್ಣಾಂಕ trip_ಸಂಕೇತ){
  ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ ಒಳ_Trans = 0;
  ಪೂರ್ಣಾಂಕ schemaChange;
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(db->mutex) );
  sqlite3BeginBenignMalloc();

  /* Obtain all b-tree mutexes before making any calls to BtreeRollback().
  ** This is important in case the transaction being rolled back has
  ** modified the database schema. If the b-tree mutexes are not taken
  ** here, then another shared-cache connection might sneak in between
  ** the database rollback and schema reset, which can cause false
  ** corruption reports in some cases.  */
  sqlite3BtreeEnterAll(db);
  schemaChange = (db->mDbFlags & DBFLAG_SchemaChange)!=0 && db->init.busy==0;

  ಗಾಗಿ(i=0; i<db->nDb; i++){
    Btree *p = db->aDb[i].pBt;
    ಆದರೆ( p ){
      ಆದರೆ( sqlite3BtreeTxnState(p)==SQLITE_TXN_WRITE ){
        ಒಳ_Trans = 1;
      }
      sqlite3BtreeRollback(p, trip_ಸಂಕೇತ, !schemaChange);
    }
  }
  sqlite3VtabRollback(db);
  sqlite3_ಕೊನೆ_Benign_Malloc();

  ಆದರೆ( schemaChange ){
    sqlite3ExpirePreparedStatements(db, 0);
    sqlite3_Reset_All_Schemas_ದ_Connection(db);
  }
  sqlite3BtreeLeaveAll(db);

  /* Any deferred constraint violations have now been resolved. */
  db->nDeferredCons = 0;
  db->nDeferredImmCons = 0;
  db->flags &= ~(u64)(SQLITE_DeferFKs|SQLITE_CorruptRdOnly);

  /* If one has been configured, invoke the rollback-hook callback */
  ಆದರೆ( db->xRollbackCallback && (ಒಳ_Trans || !db->autoCommit) ){
    db->xRollbackCallback(db->pRollbackArg);
  }
}

/*
** Return a static string containing the name corresponding to the error code
** specified in the argument.
*/
#ಆದರೆ defined(SQLITE_NEED_ERR_NAME)
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3ErrName(ಪೂರ್ಣಾಂಕ rc){
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName = 0;
  ಪೂರ್ಣಾಂಕ i, origRc = rc;
  ಗಾಗಿ(i=0; i<2 && zName==0; i++, rc &= 0xff){
    ಆಯ್ಕೆ( rc ){
      ಸ್ಥಿತಿ SQLITE_OK:                 zName = "SQLITE_OK";                ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_ERROR:              zName = "SQLITE_ERROR";             ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_ERROR_SNAPSHOT:     zName = "SQLITE_ERROR_SNAPSHOT";    ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_ERROR_RETRY:        zName = "SQLITE_ERROR_RETRY";       ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_ERROR_MISSING_COLLSEQ:
                                zName = "SQLITE_ERROR_MISSING_COLLSEQ";   ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_ಆಂತರಿಕ:           zName = "SQLITE_INTERNAL";          ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_PERM:               zName = "SQLITE_PERM";              ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_ABORT:              zName = "SQLITE_ABORT";             ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_ABORT_ROLLBACK:     zName = "SQLITE_ABORT_ROLLBACK";    ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_BUSY:               zName = "SQLITE_BUSY";              ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_BUSY_RECOVERY:      zName = "SQLITE_BUSY_RECOVERY";     ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_BUSY_SNAPSHOT:      zName = "SQLITE_BUSY_SNAPSHOT";     ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_LOCKED:             zName = "SQLITE_LOCKED";            ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_LOCKED_SHAREDCACHE: zName = "SQLITE_LOCKED_SHAREDCACHE";ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_NOMEM:              zName = "SQLITE_NOMEM";             ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_READONLY:           zName = "SQLITE_READONLY";          ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_READONLY_RECOVERY:  zName = "SQLITE_READONLY_RECOVERY"; ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_READONLY_CANTINIT:  zName = "SQLITE_READONLY_CANTINIT"; ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_READONLY_ROLLBACK:  zName = "SQLITE_READONLY_ROLLBACK"; ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_READONLY_DBMOVED:   zName = "SQLITE_READONLY_DBMOVED";  ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_READONLY_DIRECTORY: zName = "SQLITE_READONLY_DIRECTORY";ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_ಅಡಚಣೆ:          zName = "SQLITE_INTERRUPT";         ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_IOERR:              zName = "SQLITE_IOERR";             ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_IOERR_READ:         zName = "SQLITE_IOERR_READ";        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_IOERR_SHORT_READ:   zName = "SQLITE_IOERR_SHORT_READ";  ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_IOERR_WRITE:        zName = "SQLITE_IOERR_WRITE";       ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_IOERR_FSYNC:        zName = "SQLITE_IOERR_FSYNC";       ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_IOERR_DIR_FSYNC:    zName = "SQLITE_IOERR_DIR_FSYNC";   ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_IOERR_TRUNCATE:     zName = "SQLITE_IOERR_TRUNCATE";    ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_IOERR_FSTAT:        zName = "SQLITE_IOERR_FSTAT";       ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_IOERR_UNLOCK:       zName = "SQLITE_IOERR_UNLOCK";      ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_IOERR_RDLOCK:       zName = "SQLITE_IOERR_RDLOCK";      ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_IOERR_DELETE:       zName = "SQLITE_IOERR_DELETE";      ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_IOERR_NOMEM:        zName = "SQLITE_IOERR_NOMEM";       ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_IOERR_ACCESS:       zName = "SQLITE_IOERR_ACCESS";      ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_IOERR_CHECKRESERVEDLOCK:
                                zName = "SQLITE_IOERR_CHECKRESERVEDLOCK"; ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_IOERR_ಬೀಗ:         zName = "SQLITE_IOERR_LOCK";        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_IOERR_CLOSE:        zName = "SQLITE_IOERR_CLOSE";       ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_IOERR_DIR_CLOSE:    zName = "SQLITE_IOERR_DIR_CLOSE";   ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_IOERR_SHMOPEN:      zName = "SQLITE_IOERR_SHMOPEN";     ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_IOERR_SHMSIZE:      zName = "SQLITE_IOERR_SHMSIZE";     ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_IOERR_SHMLOCK:      zName = "SQLITE_IOERR_SHMLOCK";     ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_IOERR_SHMMAP:       zName = "SQLITE_IOERR_SHMMAP";      ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_IOERR_SEEK:         zName = "SQLITE_IOERR_SEEK";        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_IOERR_DELETE_NOENT: zName = "SQLITE_IOERR_DELETE_NOENT";ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_IOERR_MMAP:         zName = "SQLITE_IOERR_MMAP";        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_IOERR_GETTEMPPATH:  zName = "SQLITE_IOERR_GETTEMPPATH"; ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_IOERR_CONVPATH:     zName = "SQLITE_IOERR_CONVPATH";    ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_CORRUPT:            zName = "SQLITE_CORRUPT";           ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_CORRUPT_VTAB:       zName = "SQLITE_CORRUPT_VTAB";      ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_NOTFOUND:           zName = "SQLITE_NOTFOUND";          ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_ತುಂಬಿದ:               zName = "SQLITE_FULL";              ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_CANTOPEN:           zName = "SQLITE_CANTOPEN";          ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_CANTOPEN_NOTEMPDIR: zName = "SQLITE_CANTOPEN_NOTEMPDIR";ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_CANTOPEN_ISDIR:     zName = "SQLITE_CANTOPEN_ISDIR";    ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_CANTOPEN_FULLPATH:  zName = "SQLITE_CANTOPEN_FULLPATH"; ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_CANTOPEN_CONVPATH:  zName = "SQLITE_CANTOPEN_CONVPATH"; ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_CANTOPEN_SYMLINK:   zName = "SQLITE_CANTOPEN_SYMLINK";  ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_PROTOCOL:           zName = "SQLITE_PROTOCOL";          ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_ಖಾಲಿಗಳು:              zName = "SQLITE_EMPTY";             ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_SCHEMA:             zName = "SQLITE_SCHEMA";            ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_TOOBIG:             zName = "SQLITE_TOOBIG";            ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_CONSTRAINT:         zName = "SQLITE_CONSTRAINT";        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_CONSTRAINT_UNIQUE:  zName = "SQLITE_CONSTRAINT_UNIQUE"; ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_CONSTRAINT_TRIGGER: zName = "SQLITE_CONSTRAINT_TRIGGER";ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_CONSTRAINT_FOREIGNKEY:
                                zName = "SQLITE_CONSTRAINT_FOREIGNKEY";   ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_CONSTRAINT_CHECK:   zName = "SQLITE_CONSTRAINT_CHECK";  ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_CONSTRAINT_PRIMARYKEY:
                                zName = "SQLITE_CONSTRAINT_PRIMARYKEY";   ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_CONSTRAINT_NOTNULL: zName = "SQLITE_CONSTRAINT_NOTNULL";ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_CONSTRAINT_COMMITHOOK:
                                zName = "SQLITE_CONSTRAINT_COMMITHOOK";   ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_CONSTRAINT_VTAB:    zName = "SQLITE_CONSTRAINT_VTAB";   ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_CONSTRAINT_FUNCTION:
                                zName = "SQLITE_CONSTRAINT_FUNCTION";     ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_CONSTRAINT_ROWID:   zName = "SQLITE_CONSTRAINT_ROWID";  ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_MISMATCH:           zName = "SQLITE_MISMATCH";          ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_MISUSE:             zName = "SQLITE_MISUSE";            ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_NOLFS:              zName = "SQLITE_NOLFS";             ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_AUTH:               zName = "SQLITE_AUTH";              ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_FORMAT:             zName = "SQLITE_FORMAT";            ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_RANGE:              zName = "SQLITE_RANGE";             ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_NOTADB:             zName = "SQLITE_NOTADB";            ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_ಅಡ್ಡಸಾಲು:                zName = "SQLITE_ROW";               ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_NOTICE:             zName = "SQLITE_NOTICE";            ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_NOTICE_RECOVER_WAL: zName = "SQLITE_NOTICE_RECOVER_WAL";ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_NOTICE_RECOVER_ROLLBACK:
                                zName = "SQLITE_NOTICE_RECOVER_ROLLBACK"; ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_NOTICE_RBU:         zName = "SQLITE_NOTICE_RBU"; ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_WARNING:            zName = "SQLITE_WARNING";           ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_WARNING_AUTOINDEX:  zName = "SQLITE_WARNING_AUTOINDEX"; ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_ಮುಗಿದ:               zName = "SQLITE_DONE";              ನಿಲ್ಲಿಸು;
    }
  }
  ಆದರೆ( zName==0 ){
    ಸ್ಥಿರ ಅಕ್ಷರ zBuf[50];
    sqlite3_snprintf(ಗಾತ್ರ(zBuf), zBuf, "SQLITE_UNKNOWN(%d)", origRc);
    zName = zBuf;
  }
  ಹಿಂತಿರುಗಿ zName;
}
#ಆದರೆ_ಅಂತ್ಯ

/*
** Return a static string that describes the kind of error specified in the
** argument.
*/
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3ErrStr(ಪೂರ್ಣಾಂಕ rc){
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* ಸ್ಥಿರಾಂಕ a_ಸಂದೇಶ[] = {
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
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DISABLE_LFS
    /* SQLITE_NOLFS       */ "large file support is disabled",
#ಇಲ್ಲದಿದ್ದರೆ
    /* SQLITE_NOLFS       */ 0,
#ಆದರೆ_ಅಂತ್ಯ
    /* SQLITE_AUTH        */ "authorization denied",
    /* SQLITE_FORMAT      */ 0,
    /* SQLITE_RANGE       */ "column index out of range",
    /* SQLITE_NOTADB      */ "file is not a database",
    /* SQLITE_NOTICE      */ "notification message",
    /* SQLITE_WARNING     */ "warning message",
  };
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zErr = "unknown error";
  ಆಯ್ಕೆ( rc ){
    ಸ್ಥಿತಿ SQLITE_ABORT_ROLLBACK: {
      zErr = "abort due to ROLLBACK";
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ SQLITE_ಅಡ್ಡಸಾಲು: {
      zErr = "another row available";
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ SQLITE_ಮುಗಿದ: {
      zErr = "no more rows available";
      ನಿಲ್ಲಿಸು;
    }
    ಪೂರ್ವನಿಯೋಜಿತ: {
      rc &= 0xff;
      ಆದರೆ( ALWAYS(rc>=0) && rc<Array_ಗಾತ್ರ(a_ಸಂದೇಶ) && a_ಸಂದೇಶ[rc]!=0 ){
        zErr = a_ಸಂದೇಶ[rc];
      }
      ನಿಲ್ಲಿಸು;
    }
  }
  ಹಿಂತಿರುಗಿ zErr;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ sqliteDefaultBusyCallback(
  ಶೂನ್ಯ *ptr,               /* Database connection */
  ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2                /* Number of times table has been busy */
){
#ಆದರೆ SQLITE_OS_WIN || !defined(HAVE_NANOSLEEP) || HAVE_NANOSLEEP
  /* This case is for systems that have support for sleeping for fractions of
  ** a second.  Examples:  All windows systems, unix systems with nanosleep() */
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ u8 delays[] =
     { 1, 2, 5, 10, 15, 20, 25, 25,  25,  50,  50, 100 };
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ u8 totals[] =
     { 0, 1, 3,  8, 18, 33, 53, 78, 103, 128, 178, 228 };
# define NDELAY Array_ಗಾತ್ರ(delays)
  sqlite3 *db = (sqlite3 *)ptr;
  ಪೂರ್ಣಾಂಕ tmout = db->busyTimeout;
  ಪೂರ್ಣಾಂಕ delay, ಪೂರ್ವ;

  ಪ್ರತಿಜ್ಞಾ( ಎಣಿಕೆ_2>=0 );
  ಆದರೆ( ಎಣಿಕೆ_2 < NDELAY ){
    delay = delays[ಎಣಿಕೆ_2];
    ಪೂರ್ವ = totals[ಎಣಿಕೆ_2];
  }ಇಲ್ಲದಿದ್ದರೆ{
    delay = delays[NDELAY-1];
    ಪೂರ್ವ = totals[NDELAY-1] + delay*(ಎಣಿಕೆ_2-(NDELAY-1));
  }
  ಆದರೆ( ಪೂರ್ವ + delay > tmout ){
    delay = tmout - ಪೂರ್ವ;
    ಆದರೆ( delay<=0 ) ಹಿಂತಿರುಗಿ 0;
  }
  sqlite3OsSleep(db->pVfs, delay*1000);
  ಹಿಂತಿರುಗಿ 1;
#ಇಲ್ಲದಿದ್ದರೆ
  /* This case for unix systems that lack usleep() support.  Sleeping
  ** must be done in increments of whole seconds */
  sqlite3 *db = (sqlite3 *)ptr;
  ಪೂರ್ಣಾಂಕ tmout = ((sqlite3 *)ptr)->busyTimeout;
  ಆದರೆ( (ಎಣಿಕೆ_2+1)*1000 > tmout ){
    ಹಿಂತಿರುಗಿ 0;
  }
  sqlite3OsSleep(db->pVfs, 1000000);
  ಹಿಂತಿರುಗಿ 1;
#ಆದರೆ_ಅಂತ್ಯ
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
ಪೂರ್ಣಾಂಕ sqlite3InvokeBusyHandler(BusyHandler *p){
  ಪೂರ್ಣಾಂಕ rc;
  ಆದರೆ( p->xBusyHandler==0 || p->nBusy<0 ) ಹಿಂತಿರುಗಿ 0;
  rc = p->xBusyHandler(p->pBusyArg, p->nBusy);
  ಆದರೆ( rc==0 ){
    p->nBusy = -1;
  }ಇಲ್ಲದಿದ್ದರೆ{
    p->nBusy++;
  }
  ಹಿಂತಿರುಗಿ rc;
}

/*
** This routine sets the busy callback for an Sqlite database to the
** given callback function with the given argument.
*/
ಪೂರ್ಣಾಂಕ sqlite3_busy_handler(
  sqlite3 *db,
  ಪೂರ್ಣಾಂಕ (*xBusy)(ಶೂನ್ಯ*,ಪೂರ್ಣಾಂಕ),
  ಶೂನ್ಯ *pArg
){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !sqlite3SafetyCheckOk(db) ) ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_mutex_enter(db->mutex);
  db->busyHandler.xBusyHandler = xBusy;
  db->busyHandler.pBusyArg = pArg;
  db->busyHandler.nBusy = 0;
  db->busyTimeout = 0;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_SETLK_TIMEOUT
  db->setlkTimeout = 0;
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_mutex_leave(db->mutex);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_ಪ್ರಗತಿ_CALLBACK
/*
** This routine sets the progress callback for an Sqlite database to the
** given callback function with the given argument. The progress callback will
** be invoked every nOps opcodes.
*/
ಶೂನ್ಯ sqlite3_ಪ್ರಗತಿ_handler(
  sqlite3 *db,
  ಪೂರ್ಣಾಂಕ nOps,
  ಪೂರ್ಣಾಂಕ (*x_ಪ್ರಗತಿ)(ಶೂನ್ಯ*),
  ಶೂನ್ಯ *pArg
){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !sqlite3SafetyCheckOk(db) ){
    (ಶೂನ್ಯ)SQLITE_MISUSE_BKPT;
    ಹಿಂತಿರುಗಿ;
  }
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_mutex_enter(db->mutex);
  ಆದರೆ( nOps>0 ){
    db->x_ಪ್ರಗತಿ = x_ಪ್ರಗತಿ;
    db->n_ಪ್ರಗತಿ_Ops = (ಚಿಹ್ನೆರಹಿತ)nOps;
    db->p_ಪ್ರಗತಿ_Arg = pArg;
  }ಇಲ್ಲದಿದ್ದರೆ{
    db->x_ಪ್ರಗತಿ = 0;
    db->n_ಪ್ರಗತಿ_Ops = 0;
    db->p_ಪ್ರಗತಿ_Arg = 0;
  }
  sqlite3_mutex_leave(db->mutex);
}
#ಆದರೆ_ಅಂತ್ಯ


/*
** This routine installs a default busy handler that waits for the
** specified number of milliseconds before returning 0.
*/
ಪೂರ್ಣಾಂಕ sqlite3_busy_timeout(sqlite3 *db, ಪೂರ್ಣಾಂಕ ms){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !sqlite3SafetyCheckOk(db) ) ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_mutex_enter(db->mutex);
  ಆದರೆ( ms>0 ){
    sqlite3_busy_handler(db, (ಪೂರ್ಣಾಂಕ(*)(ಶೂನ್ಯ*,ಪೂರ್ಣಾಂಕ))sqliteDefaultBusyCallback,
                             (ಶೂನ್ಯ*)db);
    db->busyTimeout = ms;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_SETLK_TIMEOUT
    db->setlkTimeout = ms;
#ಆದರೆ_ಅಂತ್ಯ
  }ಇಲ್ಲದಿದ್ದರೆ{
    sqlite3_busy_handler(db, 0, 0);
  }
  sqlite3_mutex_leave(db->mutex);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Set the setlk timeout value.
*/
ಪೂರ್ಣಾಂಕ sqlite3_setlk_timeout(sqlite3 *db, ಪೂರ್ಣಾಂಕ ms, ಪೂರ್ಣಾಂಕ flags){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_SETLK_TIMEOUT
  ಪೂರ್ಣಾಂಕ iDb;
  ಪೂರ್ಣಾಂಕ bBOC = ((flags & SQLITE_SETLK_ಖಂಡ_ON_CONNECT) ? 1 : 0);
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !sqlite3SafetyCheckOk(db) ) ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ( ms<-1 ) ಹಿಂತಿರುಗಿ SQLITE_RANGE;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_SETLK_TIMEOUT
  sqlite3_mutex_enter(db->mutex);
  db->setlkTimeout = ms;
  db->setlkFlags = flags;
  sqlite3BtreeEnterAll(db);
  ಗಾಗಿ(iDb=0; iDb<db->nDb; iDb++){
    Btree *pBt = db->aDb[iDb].pBt;
    ಆದರೆ( pBt ){
      sqlite3_file *fd = sqlite3PagerFile(sqlite3BtreePager(pBt));
      sqlite3OsFileControlHint(fd, SQLITE_FCNTL_ಖಂಡ_ON_CONNECT, (ಶೂನ್ಯ*)&bBOC);
    }
  }
  sqlite3BtreeLeaveAll(db);
  sqlite3_mutex_leave(db->mutex);
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ !defined(SQLITE_ENABLE_API_ARMOR) && !defined(SQLITE_ENABLE_SETLK_TIMEOUT)
  ಬಳಸದ_PARAMETER(db);
  ಬಳಸದ_PARAMETER(flags);
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Cause any pending operation to stop at its earliest opportunity.
*/
ಶೂನ್ಯ sqlite3_ಅಡಚಣೆ(sqlite3 *db){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !sqlite3SafetyCheckOk(db)
   && (db==0 || db->eOpenState!=SQLITE_STATE_ZOMBIE)
  ){
    (ಶೂನ್ಯ)SQLITE_MISUSE_BKPT;
    ಹಿಂತಿರುಗಿ;
  }
#ಆದರೆ_ಅಂತ್ಯ
  AtomicStore(&db->u1.isInterrupted, 1);
}

/*
** Return true or false depending on whether or not an interrupt is
** pending on connection db.
*/
ಪೂರ್ಣಾಂಕ sqlite3_is_interrupted(sqlite3 *db){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !sqlite3SafetyCheckOk(db)
   && (db==0 || db->eOpenState!=SQLITE_STATE_ZOMBIE)
  ){
    (ಶೂನ್ಯ)SQLITE_MISUSE_BKPT;
    ಹಿಂತಿರುಗಿ 0;
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ Atomic_ತುಂಬು(&db->u1.isInterrupted)!=0;
}

/*
** This function is exactly the same as sqlite3_create_function(), except
** that it is designed to be called by internal code. The difference is
** that if a malloc() fails in sqlite3_create_function(), an error code
** is returned and the mallocFailed flag cleared.
*/
ಪೂರ್ಣಾಂಕ sqlite3CreateFunc(
  sqlite3 *db,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFunctionName,
  ಪೂರ್ಣಾಂಕ nArg,
  ಪೂರ್ಣಾಂಕ enc,
  ಶೂನ್ಯ *p_User_ದತ್ತಾಂಶ,
  ಶೂನ್ಯ (*xSFunc)(sqlite3_context*,ಪೂರ್ಣಾಂಕ,sqlite3_ಮೌಲ್ಯ **),
  ಶೂನ್ಯ (*x_ಹೆಜ್ಜೆ)(sqlite3_context*,ಪೂರ್ಣಾಂಕ,sqlite3_ಮೌಲ್ಯ **),
  ಶೂನ್ಯ (*xFinal)(sqlite3_context*),
  ಶೂನ್ಯ (*x_ಮೌಲ್ಯ)(sqlite3_context*),
  ಶೂನ್ಯ (*xInverse)(sqlite3_context*,ಪೂರ್ಣಾಂಕ,sqlite3_ಮೌಲ್ಯ **),
  FuncDestructor *pDestructor
){
  FuncDef *p;
  ಪೂರ್ಣಾಂಕ extraFlags;

  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(db->mutex) );
  ಪ್ರತಿಜ್ಞಾ( x_ಮೌಲ್ಯ==0 || xSFunc==0 );
  ಆದರೆ( zFunctionName==0                /* Must have a valid name */
   || (xSFunc!=0 && xFinal!=0)        /* Not both xSFunc and xFinal */
   || ((xFinal==0)!=(x_ಹೆಜ್ಜೆ==0))       /* Both or neither of xFinal and xStep */
   || ((x_ಮೌಲ್ಯ==0)!=(xInverse==0))    /* Both or neither of xValue, xInverse */
   || (nArg<-1 || nArg>SQLITE_MAX_FUNCTION_ARG)
   || (255<sqlite3Strlen30(zFunctionName))
  ){
    ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
  }

  ಪ್ರತಿಜ್ಞಾ( SQLITE_FUNC_CONSTANT==SQLITE_DETERMINISTIC );
  ಪ್ರತಿಜ್ಞಾ( SQLITE_FUNC_ನೇರ==SQLITE_DIRECTONLY );
  extraFlags = enc &  (SQLITE_DETERMINISTIC|SQLITE_DIRECTONLY|
                       SQLITE_SUBTYPE|SQLITE_INNOCUOUS|
                       SQLITE_ಫಲಿತಾಂಶ_SUBTYPE|SQLITE_SELFORDER1);
  enc &= (SQLITE_FUNC_ENCMASK|SQLITE_ANY);

  /* The SQLITE_INNOCUOUS flag is the same bit as SQLITE_FUNC_UNSAFE.  But
  ** the meaning is inverted.  So flip the bit. */
  ಪ್ರತಿಜ್ಞಾ( SQLITE_FUNC_UNSAFE==SQLITE_INNOCUOUS );
  extraFlags ^= SQLITE_FUNC_UNSAFE;  /* tag-20230109-1 */

 
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_UTF16
  /* If SQLITE_UTF16 is specified as the encoding type, transform this
  ** to one of SQLITE_UTF16LE or SQLITE_UTF16BE using the
  ** SQLITE_UTF16NATIVE macro. SQLITE_UTF16 is not used internally.
  **
  ** If SQLITE_ANY is specified, add three versions of the function
  ** to the hash table.
  */
  ಆಯ್ಕೆ( enc ){
    ಸ್ಥಿತಿ SQLITE_UTF16:
      enc = SQLITE_UTF16NATIVE;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ SQLITE_ANY: {
      ಪೂರ್ಣಾಂಕ rc;
      rc = sqlite3CreateFunc(db, zFunctionName, nArg,
           (SQLITE_UTF8|extraFlags)^SQLITE_FUNC_UNSAFE, /* tag-20230109-1 */
           p_User_ದತ್ತಾಂಶ, xSFunc, x_ಹೆಜ್ಜೆ, xFinal, x_ಮೌಲ್ಯ, xInverse, pDestructor);
      ಆದರೆ( rc==SQLITE_OK ){
        rc = sqlite3CreateFunc(db, zFunctionName, nArg,
             (SQLITE_UTF16LE|extraFlags)^SQLITE_FUNC_UNSAFE, /* tag-20230109-1*/
             p_User_ದತ್ತಾಂಶ, xSFunc, x_ಹೆಜ್ಜೆ, xFinal, x_ಮೌಲ್ಯ, xInverse, pDestructor);
      }
      ಆದರೆ( rc!=SQLITE_OK ){
        ಹಿಂತಿರುಗಿ rc;
      }
      enc = SQLITE_UTF16BE;
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ SQLITE_UTF8:
    ಸ್ಥಿತಿ SQLITE_UTF16LE:
    ಸ್ಥಿತಿ SQLITE_UTF16BE:
      ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:
      enc = SQLITE_UTF8;
      ನಿಲ್ಲಿಸು;
  }
#ಇಲ್ಲದಿದ್ದರೆ
  enc = SQLITE_UTF8;
#ಆದರೆ_ಅಂತ್ಯ
 
  /* Check if an existing function is being overridden or deleted. If so,
  ** and there are active VMs, then return SQLITE_BUSY. If a function
  ** is being overridden/deleted but there are no active VMs, allow the
  ** operation to continue but invalidate all precompiled statements.
  */
  p = sqlite3_ಕಂಡುಹಿಡಿ_Function(db, zFunctionName, nArg, (u8)enc, 0);
  ಆದರೆ( p && (p->funcFlags & SQLITE_FUNC_ENCMASK)==(u32)enc && p->nArg==nArg ){
    ಆದರೆ( db->n_Vdbe_ಸಕ್ರಿಯ ){
      sqlite3_Error_With_ಸಂದೇಶ(db, SQLITE_BUSY,
        "unable to delete/modify user-function due to active statements");
      ಪ್ರತಿಜ್ಞಾ( !db->mallocFailed || db->nVdbeExec>0 );
      ಹಿಂತಿರುಗಿ SQLITE_BUSY;
    }ಇಲ್ಲದಿದ್ದರೆ{
      sqlite3ExpirePreparedStatements(db, 0);
    }
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( xSFunc==0 && xFinal==0 ){
    /* Trying to delete a function that does not exist.  This is a no-op.
    ** https://sqlite.org/forum/forumpost/726219164b */
    ಹಿಂತಿರುಗಿ SQLITE_OK;
  }

  p = sqlite3_ಕಂಡುಹಿಡಿ_Function(db, zFunctionName, nArg, (u8)enc, 1);
  ಪ್ರತಿಜ್ಞಾ(p || db->mallocFailed);
  ಆದರೆ( !p ){
    ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
  }

  /* If an older version of the function with a configured destructor is
  ** being replaced invoke the destructor function here. */
  functionDestroy(db, p);

  ಆದರೆ( pDestructor ){
    pDestructor->nRef++;
  }
  p->u.pDestructor = pDestructor;
  p->funcFlags = (p->funcFlags & SQLITE_FUNC_ENCMASK) | extraFlags;
  testcase( p->funcFlags & SQLITE_DETERMINISTIC );
  testcase( p->funcFlags & SQLITE_DIRECTONLY );
  p->xSFunc = xSFunc ? xSFunc : x_ಹೆಜ್ಜೆ;
  p->xFinalize = xFinal;
  p->x_ಮೌಲ್ಯ = x_ಮೌಲ್ಯ;
  p->xInverse = xInverse;
  p->p_User_ದತ್ತಾಂಶ = p_User_ದತ್ತಾಂಶ;
  p->nArg = (u16)nArg;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Worker function used by utf-8 APIs that create new functions:
**
**    sqlite3_create_function()
**    sqlite3_create_function_v2()
**    sqlite3_create_window_function()
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ createFunctionApi(
  sqlite3 *db,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFunc,
  ಪೂರ್ಣಾಂಕ nArg,
  ಪೂರ್ಣಾಂಕ enc,
  ಶೂನ್ಯ *p,
  ಶೂನ್ಯ (*xSFunc)(sqlite3_context*,ಪೂರ್ಣಾಂಕ,sqlite3_ಮೌಲ್ಯ**),
  ಶೂನ್ಯ (*x_ಹೆಜ್ಜೆ)(sqlite3_context*,ಪೂರ್ಣಾಂಕ,sqlite3_ಮೌಲ್ಯ**),
  ಶೂನ್ಯ (*xFinal)(sqlite3_context*),
  ಶೂನ್ಯ (*x_ಮೌಲ್ಯ)(sqlite3_context*),
  ಶೂನ್ಯ (*xInverse)(sqlite3_context*,ಪೂರ್ಣಾಂಕ,sqlite3_ಮೌಲ್ಯ**),
  ಶೂನ್ಯ(*xDestroy)(ಶೂನ್ಯ*)
){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_ERROR;
  FuncDestructor *pArg = 0;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !sqlite3SafetyCheckOk(db) ){
    ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
  }
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_mutex_enter(db->mutex);
  ಆದರೆ( xDestroy ){
    pArg = (FuncDestructor *)sqlite3Malloc(ಗಾತ್ರ(FuncDestructor));
    ಆದರೆ( !pArg ){
      sqlite3OomFault(db);
      xDestroy(p);
      ಹೋಗು ಹೊರಹೋಗು;
    }
    pArg->nRef = 0;
    pArg->xDestroy = xDestroy;
    pArg->p_User_ದತ್ತಾಂಶ = p;
  }
  rc = sqlite3CreateFunc(db, zFunc, nArg, enc, p,
      xSFunc, x_ಹೆಜ್ಜೆ, xFinal, x_ಮೌಲ್ಯ, xInverse, pArg
  );
  ಆದರೆ( pArg && pArg->nRef==0 ){
    ಪ್ರತಿಜ್ಞಾ( rc!=SQLITE_OK || (x_ಹೆಜ್ಜೆ==0 && xFinal==0) );
    xDestroy(p);
    sqlite3_free(pArg);
  }

 ಹೊರಹೋಗು:
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Create new user functions.
*/
ಪೂರ್ಣಾಂಕ sqlite3_create_function(
  sqlite3 *db,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFunc,
  ಪೂರ್ಣಾಂಕ nArg,
  ಪೂರ್ಣಾಂಕ enc,
  ಶೂನ್ಯ *p,
  ಶೂನ್ಯ (*xSFunc)(sqlite3_context*,ಪೂರ್ಣಾಂಕ,sqlite3_ಮೌಲ್ಯ **),
  ಶೂನ್ಯ (*x_ಹೆಜ್ಜೆ)(sqlite3_context*,ಪೂರ್ಣಾಂಕ,sqlite3_ಮೌಲ್ಯ **),
  ಶೂನ್ಯ (*xFinal)(sqlite3_context*)
){
  ಹಿಂತಿರುಗಿ createFunctionApi(db, zFunc, nArg, enc, p, xSFunc, x_ಹೆಜ್ಜೆ,
                                    xFinal, 0, 0, 0);
}
ಪೂರ್ಣಾಂಕ sqlite3_create_function_v2(
  sqlite3 *db,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFunc,
  ಪೂರ್ಣಾಂಕ nArg,
  ಪೂರ್ಣಾಂಕ enc,
  ಶೂನ್ಯ *p,
  ಶೂನ್ಯ (*xSFunc)(sqlite3_context*,ಪೂರ್ಣಾಂಕ,sqlite3_ಮೌಲ್ಯ **),
  ಶೂನ್ಯ (*x_ಹೆಜ್ಜೆ)(sqlite3_context*,ಪೂರ್ಣಾಂಕ,sqlite3_ಮೌಲ್ಯ **),
  ಶೂನ್ಯ (*xFinal)(sqlite3_context*),
  ಶೂನ್ಯ (*xDestroy)(ಶೂನ್ಯ *)
){
  ಹಿಂತಿರುಗಿ createFunctionApi(db, zFunc, nArg, enc, p, xSFunc, x_ಹೆಜ್ಜೆ,
                                    xFinal, 0, 0, xDestroy);
}
ಪೂರ್ಣಾಂಕ sqlite3_create_ಕಿಟಕಿ_function(
  sqlite3 *db,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFunc,
  ಪೂರ್ಣಾಂಕ nArg,
  ಪೂರ್ಣಾಂಕ enc,
  ಶೂನ್ಯ *p,
  ಶೂನ್ಯ (*x_ಹೆಜ್ಜೆ)(sqlite3_context*,ಪೂರ್ಣಾಂಕ,sqlite3_ಮೌಲ್ಯ **),
  ಶೂನ್ಯ (*xFinal)(sqlite3_context*),
  ಶೂನ್ಯ (*x_ಮೌಲ್ಯ)(sqlite3_context*),
  ಶೂನ್ಯ (*xInverse)(sqlite3_context*,ಪೂರ್ಣಾಂಕ,sqlite3_ಮೌಲ್ಯ **),
  ಶೂನ್ಯ (*xDestroy)(ಶೂನ್ಯ *)
){
  ಹಿಂತಿರುಗಿ createFunctionApi(db, zFunc, nArg, enc, p, 0, x_ಹೆಜ್ಜೆ,
                                    xFinal, x_ಮೌಲ್ಯ, xInverse, xDestroy);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_UTF16
ಪೂರ್ಣಾಂಕ sqlite3_create_function16(
  sqlite3 *db,
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *zFunctionName,
  ಪೂರ್ಣಾಂಕ nArg,
  ಪೂರ್ಣಾಂಕ e_ಪಠ್ಯ_Rep,
  ಶೂನ್ಯ *p,
  ಶೂನ್ಯ (*xSFunc)(sqlite3_context*,ಪೂರ್ಣಾಂಕ,sqlite3_ಮೌಲ್ಯ**),
  ಶೂನ್ಯ (*x_ಹೆಜ್ಜೆ)(sqlite3_context*,ಪೂರ್ಣಾಂಕ,sqlite3_ಮೌಲ್ಯ**),
  ಶೂನ್ಯ (*xFinal)(sqlite3_context*)
){
  ಪೂರ್ಣಾಂಕ rc;
  ಅಕ್ಷರ *zFunc8;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !sqlite3SafetyCheckOk(db) || zFunctionName==0 ) ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_mutex_enter(db->mutex);
  ಪ್ರತಿಜ್ಞಾ( !db->mallocFailed );
  zFunc8 = sqlite3Utf16to8(db, zFunctionName, -1, SQLITE_UTF16NATIVE);
  rc = sqlite3CreateFunc(db, zFunc8, nArg, e_ಪಠ್ಯ_Rep, p, xSFunc,x_ಹೆಜ್ಜೆ,xFinal,0,0,0);
  sqlite3DbFree(db, zFunc8);
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  ಹಿಂತಿರುಗಿ rc;
}
#ಆದರೆ_ಅಂತ್ಯ


/*
** The following is the implementation of an SQL function that always
** fails with an error message stating that the function is used in the
** wrong context.  The sqlite3_overload_function() API might construct
** SQL function that use this routine so that the functions will exist
** for name resolution but are actually overloaded by the xFindFunction
** method of virtual tables.
*/
ಸ್ಥಿರ ಶೂನ್ಯ sqlite3InvalidFunction(
  sqlite3_context *context,  /* The function calling context */
  ಪೂರ್ಣಾಂಕ Not_ಬಳಸಿದ,               /* Number of arguments to the function */
  sqlite3_ಮೌಲ್ಯ **NotUsed2   /* Value of each argument */
){
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_user_ದತ್ತಾಂಶ(context);
  ಅಕ್ಷರ *zErr;
  ಬಳಸದ_PARAMETER2(Not_ಬಳಸಿದ, NotUsed2);
  zErr = sqlite3_mprintf(
      "unable to use function %s in the requested context", zName);
  sqlite3_ಫಲಿತಾಂಶ_error(context, zErr, -1);
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
ಪೂರ್ಣಾಂಕ sqlite3_overload_function(
  sqlite3 *db,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName,
  ಪೂರ್ಣಾಂಕ nArg
){
  ಪೂರ್ಣಾಂಕ rc;
  ಅಕ್ಷರ *zCopy;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !sqlite3SafetyCheckOk(db) || zName==0 || nArg<-2 ){
    ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
  }
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_mutex_enter(db->mutex);
  rc = sqlite3_ಕಂಡುಹಿಡಿ_Function(db, zName, nArg, SQLITE_UTF8, 0)!=0;
  sqlite3_mutex_leave(db->mutex);
  ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ SQLITE_OK;
  zCopy = sqlite3_mprintf("%s", zName);
  ಆದರೆ( zCopy==0 ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
  ಹಿಂತಿರುಗಿ sqlite3_create_function_v2(db, zName, nArg, SQLITE_UTF8,
                           zCopy, sqlite3InvalidFunction, 0, 0, sqlite3_free);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_TRACE
/*
** Register a trace function.  The pArg from the previously registered trace
** is returned. 
**
** A NULL trace function means that no tracing is executes.  A non-NULL
** trace is a pointer to a function that is invoked at the start of each
** SQL statement.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_DEPRECATED
ಶೂನ್ಯ *sqlite3_trace(sqlite3 *db, ಶೂನ್ಯ(*xTrace)(ಶೂನ್ಯ*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*), ಶೂನ್ಯ *pArg){
  ಶೂನ್ಯ *pOld;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !sqlite3SafetyCheckOk(db) ){
    (ಶೂನ್ಯ)SQLITE_MISUSE_BKPT;
    ಹಿಂತಿರುಗಿ 0;
  }
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_mutex_enter(db->mutex);
  pOld = db->pTraceArg;
  db->mTrace = xTrace ? SQLITE_TRACE_LEGACY : 0;
  db->trace.xLegacy = xTrace;
  db->pTraceArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  ಹಿಂತಿರುಗಿ pOld;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_DEPRECATED */

/* Register a trace callback using the version-2 interface.
*/
ಪೂರ್ಣಾಂಕ sqlite3_trace_v2(
  sqlite3 *db,                               /* Trace this connection */
  ಚಿಹ್ನೆರಹಿತ mTrace,                           /* Mask of events to be traced */
  ಪೂರ್ಣಾಂಕ(*xTrace)(ಚಿಹ್ನೆರಹಿತ,ಶೂನ್ಯ*,ಶೂನ್ಯ*,ಶೂನ್ಯ*),  /* Callback to invoke */
  ಶೂನ್ಯ *pArg                                 /* Context */
){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !sqlite3SafetyCheckOk(db) ){
    ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
  }
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_mutex_enter(db->mutex);
  ಆದರೆ( mTrace==0 ) xTrace = 0;
  ಆದರೆ( xTrace==0 ) mTrace = 0;
  db->mTrace = mTrace;
  db->trace.xV2 = xTrace;
  db->pTraceArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_DEPRECATED
/*
** Register a profile function.  The pArg from the previously registered
** profile function is returned. 
**
** A NULL profile function means that no profiling is executes.  A non-NULL
** profile is a pointer to a function that is invoked at the conclusion of
** each SQL statement that is run.
*/
ಶೂನ್ಯ *sqlite3_profile(
  sqlite3 *db,
  ಶೂನ್ಯ (*xProfile)(ಶೂನ್ಯ*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,sqlite_uint64),
  ಶೂನ್ಯ *pArg
){
  ಶೂನ್ಯ *pOld;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !sqlite3SafetyCheckOk(db) ){
    (ಶೂನ್ಯ)SQLITE_MISUSE_BKPT;
    ಹಿಂತಿರುಗಿ 0;
  }
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_mutex_enter(db->mutex);
  pOld = db->pProfileArg;
  db->xProfile = xProfile;
  db->pProfileArg = pArg;
  db->mTrace &= SQLITE_TRACE_NONLEGACY_ಮುಖವಾಡ;
  ಆದರೆ( db->xProfile ) db->mTrace |= SQLITE_TRACE_XPROFILE;
  sqlite3_mutex_leave(db->mutex);
  ಹಿಂತಿರುಗಿ pOld;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_DEPRECATED */
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_TRACE */

/*
** Register a function to be invoked when a transaction commits.
** If the invoked function returns non-zero, then the commit becomes a
** rollback.
*/
ಶೂನ್ಯ *sqlite3_commit_hook(
  sqlite3 *db,              /* Attach the hook to this database */
  ಪೂರ್ಣಾಂಕ (*xCallback)(ಶೂನ್ಯ*),  /* Function to invoke on each commit */
  ಶೂನ್ಯ *pArg                /* Argument to the function */
){
  ಶೂನ್ಯ *pOld;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !sqlite3SafetyCheckOk(db) ){
    (ಶೂನ್ಯ)SQLITE_MISUSE_BKPT;
    ಹಿಂತಿರುಗಿ 0;
  }
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_mutex_enter(db->mutex);
  pOld = db->pCommitArg;
  db->xCommitCallback = xCallback;
  db->pCommitArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  ಹಿಂತಿರುಗಿ pOld;
}

/*
** Register a callback to be invoked each time a row is updated,
** inserted or deleted using this database connection.
*/
ಶೂನ್ಯ *sqlite3_update_hook(
  sqlite3 *db,              /* Attach the hook to this database */
  ಶೂನ್ಯ (*xCallback)(ಶೂನ್ಯ*,ಪೂರ್ಣಾಂಕ,ಅಕ್ಷರ ಸ್ಥಿರಾಂಕ *,ಅಕ್ಷರ ಸ್ಥಿರಾಂಕ *,sqlite_int64),
  ಶೂನ್ಯ *pArg                /* Argument to the function */
){
  ಶೂನ್ಯ *pRet;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !sqlite3SafetyCheckOk(db) ){
    (ಶೂನ್ಯ)SQLITE_MISUSE_BKPT;
    ಹಿಂತಿರುಗಿ 0;
  }
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_mutex_enter(db->mutex);
  pRet = db->pUpdateArg;
  db->xUpdateCallback = xCallback;
  db->pUpdateArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  ಹಿಂತಿರುಗಿ pRet;
}

/*
** Register a callback to be invoked each time a transaction is rolled
** back by this database connection.
*/
ಶೂನ್ಯ *sqlite3_rollback_hook(
  sqlite3 *db,              /* Attach the hook to this database */
  ಶೂನ್ಯ (*xCallback)(ಶೂನ್ಯ*), /* Callback function */
  ಶೂನ್ಯ *pArg                /* Argument to the function */
){
  ಶೂನ್ಯ *pRet;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !sqlite3SafetyCheckOk(db) ){
    (ಶೂನ್ಯ)SQLITE_MISUSE_BKPT;
    ಹಿಂತಿರುಗಿ 0;
  }
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_mutex_enter(db->mutex);
  pRet = db->pRollbackArg;
  db->xRollbackCallback = xCallback;
  db->pRollbackArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  ಹಿಂತಿರುಗಿ pRet;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_PREUPDATE_HOOK
/*
** Register a callback to be invoked each time a row is updated,
** inserted or deleted using this database connection.
*/
ಶೂನ್ಯ *sqlite3_preupdate_hook(
  sqlite3 *db,              /* Attach the hook to this database */
  ಶೂನ್ಯ(*xCallback)(         /* Callback function */
    ಶೂನ್ಯ*,sqlite3*,ಪೂರ್ಣಾಂಕ,ಅಕ್ಷರ ಸ್ಥಿರಾಂಕ*,ಅಕ್ಷರ ಸ್ಥಿರಾಂಕ*,sqlite3_int64,sqlite3_int64),
  ಶೂನ್ಯ *pArg                /* First callback argument */
){
  ಶೂನ್ಯ *pRet;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( db==0 ){
    ಹಿಂತಿರುಗಿ 0;
  }
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_mutex_enter(db->mutex);
  pRet = db->pPreUpdateArg;
  db->xPreUpdateCallback = xCallback;
  db->pPreUpdateArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  ಹಿಂತಿರುಗಿ pRet;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_ENABLE_PREUPDATE_HOOK */

/*
** Register a function to be invoked prior to each autovacuum that
** determines the number of pages to vacuum.
*/
ಪೂರ್ಣಾಂಕ sqlite3_autovacuum_ಪುಟಗಳು(
  sqlite3 *db,                 /* Attach the hook to this database */
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ (*xCallback)(ಶೂನ್ಯ*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,u32,u32,u32),
  ಶೂನ್ಯ *pArg,                  /* Argument to the function */
  ಶೂನ್ಯ (*xDestructor)(ಶೂನ್ಯ*)   /* Destructor for pArg */
){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !sqlite3SafetyCheckOk(db) ){
    ಆದರೆ( xDestructor ) xDestructor(pArg);
    ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
  }
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_mutex_enter(db->mutex);
  ಆದರೆ( db->xAutovacDestr ){
    db->xAutovacDestr(db->p_Autovac_ಪುಟಗಳು_Arg);
  }
  db->x_Autovac_ಪುಟಗಳು = xCallback;
  db->p_Autovac_ಪುಟಗಳು_Arg = pArg;
  db->xAutovacDestr = xDestructor;
  sqlite3_mutex_leave(db->mutex);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}


#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WAL
/*
** The sqlite3_wal_hook() callback registered by sqlite3_wal_autocheckpoint().
** Invoke sqlite3_wal_checkpoint if the number of frames in the log file
** is greater than sqlite3.pWalArg cast to an integer (the value configured by
** wal_autocheckpoint()).
*/
ಪೂರ್ಣಾಂಕ sqlite3WalDefaultHook(
  ಶೂನ್ಯ *p_Client_ದತ್ತಾಂಶ,     /*ವಾದ*/
  sqlite3 *db,           /* Connection */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDb,       /* Database */
  ಪೂರ್ಣಾಂಕ n_ಚೌಕಟ್ಟು             /* Size of WAL */
){
  ಆದರೆ( n_ಚೌಕಟ್ಟು>=SQLITE_PTR_ಗಮ್ಯ_INT(p_Client_ದತ್ತಾಂಶ) ){
    sqlite3BeginBenignMalloc();
    sqlite3_wal_checkpoint(db, zDb);
    sqlite3_ಕೊನೆ_Benign_Malloc();
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_WAL */

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
ಪೂರ್ಣಾಂಕ sqlite3_wal_autocheckpoint(sqlite3 *db, ಪೂರ್ಣಾಂಕ n_ಚೌಕಟ್ಟು){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_OMIT_WAL
  ಬಳಸದ_PARAMETER(db);
  ಬಳಸದ_PARAMETER(n_ಚೌಕಟ್ಟು);
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !sqlite3SafetyCheckOk(db) ) ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ( n_ಚೌಕಟ್ಟು>0 ){
    sqlite3_wal_hook(db, sqlite3WalDefaultHook, SQLITE_INT_ಗಮ್ಯ_PTR(n_ಚೌಕಟ್ಟು));
  }ಇಲ್ಲದಿದ್ದರೆ{
    sqlite3_wal_hook(db, 0, 0);
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Register a callback to be invoked each time a transaction is written
** into the write-ahead-log by this database connection.
*/
ಶೂನ್ಯ *sqlite3_wal_hook(
  sqlite3 *db,                    /* Attach the hook to this db handle */
  ಪೂರ್ಣಾಂಕ(*xCallback)(ಶೂನ್ಯ *, sqlite3*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*, ಪೂರ್ಣಾಂಕ),
  ಶೂನ್ಯ *pArg                      /* First argument passed to xCallback() */
){
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WAL
  ಶೂನ್ಯ *pRet;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !sqlite3SafetyCheckOk(db) ){
    (ಶೂನ್ಯ)SQLITE_MISUSE_BKPT;
    ಹಿಂತಿರುಗಿ 0;
  }
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_mutex_enter(db->mutex);
  pRet = db->pWalArg;
  db->xWalCallback = xCallback;
  db->pWalArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  ಹಿಂತಿರುಗಿ pRet;
#ಇಲ್ಲದಿದ್ದರೆ
  ಬಳಸದ_PARAMETER(db);
  ಬಳಸದ_PARAMETER(xCallback);
  ಬಳಸದ_PARAMETER(pArg);
  ಹಿಂತಿರುಗಿ 0;
#ಆದರೆ_ಅಂತ್ಯ
}

/*
** Checkpoint database zDb.
*/
ಪೂರ್ಣಾಂಕ sqlite3_wal_checkpoint_v2(
  sqlite3 *db,                    /* Database handle */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDb,                /* Name of attached database (or NULL) */
  ಪೂರ್ಣಾಂಕ e_ಬಹುಲಕ,                      /* SQLITE_CHECKPOINT_* value */
  ಪೂರ್ಣಾಂಕ *pnLog,                     /* OUT: Size of WAL log in frames */
  ಪೂರ್ಣಾಂಕ *pnCkpt                     /* OUT: Total number of frames checkpointed */
){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_OMIT_WAL
  ಬಳಸದ_PARAMETER(db);
  ಬಳಸದ_PARAMETER(zDb);
  ಬಳಸದ_PARAMETER(e_ಬಹುಲಕ);
  ಬಳಸದ_PARAMETER(pnLog);
  ಬಳಸದ_PARAMETER(pnCkpt);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
#ಇಲ್ಲದಿದ್ದರೆ
  ಪೂರ್ಣಾಂಕ rc;                         /* Return code */
  ಪೂರ್ಣಾಂಕ iDb;                        /* Schema to checkpoint */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !sqlite3SafetyCheckOk(db) ) ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
#ಆದರೆ_ಅಂತ್ಯ

  /* Initialize the output variables to -1 in case an error occurs. */
  ಆದರೆ( pnLog ) *pnLog = -1;
  ಆದರೆ( pnCkpt ) *pnCkpt = -1;

  ಪ್ರತಿಜ್ಞಾ( SQLITE_CHECKPOINT_NOOP==-1 );
  ಪ್ರತಿಜ್ಞಾ( SQLITE_CHECKPOINT_PASSIVE==0 );
  ಪ್ರತಿಜ್ಞಾ( SQLITE_CHECKPOINT_ತುಂಬಿದ==1 );
  ಪ್ರತಿಜ್ಞಾ( SQLITE_CHECKPOINT_RESTART==2 );
  ಪ್ರತಿಜ್ಞಾ( SQLITE_CHECKPOINT_TRUNCATE==3 );
  ಆದರೆ( e_ಬಹುಲಕ<SQLITE_CHECKPOINT_NOOP || e_ಬಹುಲಕ>SQLITE_CHECKPOINT_TRUNCATE ){
    /* EVIDENCE-OF: R-03996-12088 The M parameter must be a valid checkpoint
    ** mode: */
    ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
  }

  sqlite3_mutex_enter(db->mutex);
  ಆದರೆ( zDb && zDb[0] ){
    iDb = sqlite3_ಕಂಡುಹಿಡಿ_Db_Name(db, zDb);
  }ಇಲ್ಲದಿದ್ದರೆ{
    iDb = SQLITE_MAX_DB;   /* This means process all schemas */
  }
  ಆದರೆ( iDb<0 ){
    rc = SQLITE_ERROR;
    sqlite3_Error_With_ಸಂದೇಶ(db, SQLITE_ERROR, "unknown database: %s", zDb);
  }ಇಲ್ಲದಿದ್ದರೆ{
    db->busyHandler.nBusy = 0;
    rc = sqlite3Checkpoint(db, iDb, e_ಬಹುಲಕ, pnLog, pnCkpt);
    sqlite3Error(db, rc);
  }
  rc = sqlite3ApiExit(db, rc);

  /* If there are no active statements, clear the interrupt flag at this
  ** point.  */
  ಆದರೆ( db->n_Vdbe_ಸಕ್ರಿಯ==0 ){
    AtomicStore(&db->u1.isInterrupted, 0);
  }

  sqlite3_mutex_leave(db->mutex);
  ಹಿಂತಿರುಗಿ rc;
#ಆದರೆ_ಅಂತ್ಯ
}


/*
** Checkpoint database zDb. If zDb is NULL, or if the buffer zDb points
** to contains a zero-length string, all attached databases are
** checkpointed.
*/
ಪೂರ್ಣಾಂಕ sqlite3_wal_checkpoint(sqlite3 *db, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDb){
  /* EVIDENCE-OF: R-41613-20553 The sqlite3_wal_checkpoint(D,X) is equivalent to
  ** sqlite3_wal_checkpoint_v2(D,X,SQLITE_CHECKPOINT_PASSIVE,0,0). */
  ಹಿಂತಿರುಗಿ sqlite3_wal_checkpoint_v2(db,zDb,SQLITE_CHECKPOINT_PASSIVE,0,0);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WAL
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
ಪೂರ್ಣಾಂಕ sqlite3Checkpoint(sqlite3 *db, ಪೂರ್ಣಾಂಕ iDb, ಪೂರ್ಣಾಂಕ e_ಬಹುಲಕ, ಪೂರ್ಣಾಂಕ *pnLog, ಪೂರ್ಣಾಂಕ *pnCkpt){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;             /* Return code */
  ಪೂರ್ಣಾಂಕ i;                          /* Used to iterate through attached dbs */
  ಪೂರ್ಣಾಂಕ bBusy = 0;                  /* True if SQLITE_BUSY has been encountered */

  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(db->mutex) );
  ಪ್ರತಿಜ್ಞಾ( !pnLog || *pnLog==-1 );
  ಪ್ರತಿಜ್ಞಾ( !pnCkpt || *pnCkpt==-1 );
  testcase( iDb==SQLITE_MAX_ATTACHED ); /* See forum post a006d86f72 */
  testcase( iDb==SQLITE_MAX_DB );

  ಗಾಗಿ(i=0; i<db->nDb && rc==SQLITE_OK; i++){
    ಆದರೆ( i==iDb || iDb==SQLITE_MAX_DB ){
      rc = sqlite3BtreeCheckpoint(db->aDb[i].pBt, e_ಬಹುಲಕ, pnLog, pnCkpt);
      pnLog = 0;
      pnCkpt = 0;
      ಆದರೆ( rc==SQLITE_BUSY ){
        bBusy = 1;
        rc = SQLITE_OK;
      }
    }
  }

  ಹಿಂತಿರುಗಿ (rc==SQLITE_OK && bBusy) ? SQLITE_BUSY : rc;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_WAL */

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
ಪೂರ್ಣಾಂಕ sqlite3_ತಾತ್ಕಾಲಿಕ_ಒಳ_ಸ್ಮರಣೆ(ಸ್ಥಿರಾಂಕ sqlite3 *db){
#ಆದರೆ SQLITE_ತಾತ್ಕಾಲಿಕ_STORE==1
  ಹಿಂತಿರುಗಿ ( db->ತಾತ್ಕಾಲಿಕ_store==2 );
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ SQLITE_ತಾತ್ಕಾಲಿಕ_STORE==2
  ಹಿಂತಿರುಗಿ ( db->ತಾತ್ಕಾಲಿಕ_store!=1 );
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ SQLITE_ತಾತ್ಕಾಲಿಕ_STORE==3
  ಬಳಸದ_PARAMETER(db);
  ಹಿಂತಿರುಗಿ 1;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ SQLITE_ತಾತ್ಕಾಲಿಕ_STORE<1 || SQLITE_ತಾತ್ಕಾಲಿಕ_STORE>3
  ಬಳಸದ_PARAMETER(db);
  ಹಿಂತಿರುಗಿ 0;
#ಆದರೆ_ಅಂತ್ಯ
}

/*
** Return UTF-8 encoded English language explanation of the most recent
** error.
*/
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3_errmsg(sqlite3 *db){
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z;
  ಆದರೆ( !db ){
    ಹಿಂತಿರುಗಿ sqlite3ErrStr(SQLITE_NOMEM_BKPT);
  }
  ಆದರೆ( !sqlite3SafetyCheckSickOrOk(db) ){
    ಹಿಂತಿರುಗಿ sqlite3ErrStr(SQLITE_MISUSE_BKPT);
  }
  sqlite3_mutex_enter(db->mutex);
  ಆದರೆ( db->mallocFailed ){
    z = sqlite3ErrStr(SQLITE_NOMEM_BKPT);
  }ಇಲ್ಲದಿದ್ದರೆ{
    testcase( db->pErr==0 );
    z = db->err_ಸಂಕೇತ ? (ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(db->pErr) : 0;
    ಪ್ರತಿಜ್ಞಾ( !db->mallocFailed );
    ಆದರೆ( z==0 ){
      z = sqlite3ErrStr(db->err_ಸಂಕೇತ);
    }
  }
  sqlite3_mutex_leave(db->mutex);
  ಹಿಂತಿರುಗಿ z;
}

/*
** Set the error code and error message associated with the database handle.
**
** This routine is intended to be called by outside extensions (ex: the
** Session extension). Internal logic should invoke sqlite3Error() or
** sqlite3ErrorWithMsg() directly.
*/
ಪೂರ್ಣಾಂಕ sqlite3_ಗಣ_errmsg(sqlite3 *db, ಪೂರ್ಣಾಂಕ errcode, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಸಂದೇಶ){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  ಆದರೆ( !sqlite3SafetyCheckOk(db) ){
    ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(db->mutex);
  ಆದರೆ( z_ಸಂದೇಶ ){
    sqlite3_Error_With_ಸಂದೇಶ(db, errcode, "%s", z_ಸಂದೇಶ);
  }ಇಲ್ಲದಿದ್ದರೆ{
    sqlite3Error(db, errcode);
  }
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Return the byte offset of the most recent error
*/
ಪೂರ್ಣಾಂಕ sqlite3_error_ಸ್ಥಾನಾಂತರ(sqlite3 *db){
  ಪೂರ್ಣಾಂಕ i_ಸ್ಥಾನಾಂತರ = -1;
  ಆದರೆ( db && sqlite3SafetyCheckSickOrOk(db) ){
    sqlite3_mutex_enter(db->mutex);
    ಆದರೆ( db->err_ಸಂಕೇತ ){
      i_ಸ್ಥಾನಾಂತರ = db->err_Byte_ಸ್ಥಾನಾಂತರ;
    }
    sqlite3_mutex_leave(db->mutex);
  }
  ಹಿಂತಿರುಗಿ i_ಸ್ಥಾನಾಂತರ;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_UTF16
/*
** Return UTF-16 encoded English language explanation of the most recent
** error.
*/
ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *sqlite3_errmsg16(sqlite3 *db){
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ u16 ಹೊರಹೋಗು_ದ_Mem[] = {
    'o', 'u', 't', ' ', 'o', 'f', ' ', 'm', 'e', 'm', 'o', 'r', 'y', 0
  };
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ u16 misuse[] = {
    'b', 'a', 'd', ' ', 'p', 'a', 'r', 'a', 'm', 'e', 't', 'e', 'r', ' ',
    'o', 'r', ' ', 'o', 't', 'h', 'e', 'r', ' ', 'A', 'P', 'I', ' ',
    'm', 'i', 's', 'u', 's', 'e', 0
  };

  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *z;
  ಆದರೆ( !db ){
    ಹಿಂತಿರುಗಿ (ಶೂನ್ಯ *)ಹೊರಹೋಗು_ದ_Mem;
  }
  ಆದರೆ( !sqlite3SafetyCheckSickOrOk(db) ){
    ಹಿಂತಿರುಗಿ (ಶೂನ್ಯ *)misuse;
  }
  sqlite3_mutex_enter(db->mutex);
  ಆದರೆ( db->mallocFailed ){
    z = (ಶೂನ್ಯ *)ಹೊರಹೋಗು_ದ_Mem;
  }ಇಲ್ಲದಿದ್ದರೆ{
    z = sqlite3_ಮೌಲ್ಯ_text16(db->pErr);
    ಆದರೆ( z==0 ){
      sqlite3_Error_With_ಸಂದೇಶ(db, db->err_ಸಂಕೇತ, sqlite3ErrStr(db->err_ಸಂಕೇತ));
      z = sqlite3_ಮೌಲ್ಯ_text16(db->pErr);
    }
    /* A malloc() may have failed within the call to sqlite3_value_text16()
    ** above. If this is the case, then the db->mallocFailed flag needs to
    ** be cleared before returning. Do this directly, instead of via
    ** sqlite3ApiExit(), to avoid setting the database handle error message.
    */
    sqlite3OomClear(db);
  }
  sqlite3_mutex_leave(db->mutex);
  ಹಿಂತಿರುಗಿ z;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_UTF16 */

/*
** Return the most recent error code generated by an SQLite routine. If NULL is
** passed to this function, we assume a malloc() failed during sqlite3_open().
*/
ಪೂರ್ಣಾಂಕ sqlite3_errcode(sqlite3 *db){
  ಪೂರ್ಣಾಂಕ iRet;
  ಆದರೆ( !db ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
  ಆದರೆ( !sqlite3SafetyCheckSickOrOk(db) ){
    ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(db->mutex);
  ಆದರೆ( db->mallocFailed ){
    iRet = SQLITE_NOMEM_BKPT;
  }ಇಲ್ಲದಿದ್ದರೆ{
    iRet = db->err_ಸಂಕೇತ & db->err_ಮುಖವಾಡ;
  }
  sqlite3_mutex_leave(db->mutex);
  ಹಿಂತಿರುಗಿ iRet;
}
ಪೂರ್ಣಾಂಕ sqlite3_extended_errcode(sqlite3 *db){
  ಪೂರ್ಣಾಂಕ iRet;
  ಆದರೆ( !db ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
  ಆದರೆ( !sqlite3SafetyCheckSickOrOk(db) ){
    ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(db->mutex);
  ಆದರೆ( db->mallocFailed ){
    iRet = SQLITE_NOMEM_BKPT;
  }ಇಲ್ಲದಿದ್ದರೆ{
    iRet = db->err_ಸಂಕೇತ;
  }
  sqlite3_mutex_leave(db->mutex);
  ಹಿಂತಿರುಗಿ iRet;
}
ಪೂರ್ಣಾಂಕ sqlite3_system_errno(sqlite3 *db){
  ಪೂರ್ಣಾಂಕ iRet = 0;
  ಆದರೆ( db ){
    sqlite3_mutex_enter(db->mutex);
    iRet = db->iSysErrno;
    sqlite3_mutex_leave(db->mutex);
  }
  ಹಿಂತಿರುಗಿ iRet;
} 

/*
** Return a string that describes the kind of error specified in the
** argument.  For now, this simply calls the internal sqlite3ErrStr()
** function.
*/
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3_errstr(ಪೂರ್ಣಾಂಕ rc){
  ಹಿಂತಿರುಗಿ sqlite3ErrStr(rc);
}

/*
** Create a new collating function for database "db".  The name is zName
** and the encoding is enc.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ createCollation(
  sqlite3* db,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName,
  u8 enc,
  ಶೂನ್ಯ* pCtx,
  ಪೂರ್ಣಾಂಕ(*xCompare)(ಶೂನ್ಯ*,ಪೂರ್ಣಾಂಕ,ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*,ಪೂರ್ಣಾಂಕ,ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*),
  ಶೂನ್ಯ(*xDel)(ಶೂನ್ಯ*)
){
  CollSeq *pColl;
  ಪೂರ್ಣಾಂಕ enc2;
 
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(db->mutex) );

  /* If SQLITE_UTF16 is specified as the encoding type, transform this
  ** to one of SQLITE_UTF16LE or SQLITE_UTF16BE using the
  ** SQLITE_UTF16NATIVE macro. SQLITE_UTF16 is not used internally.
  */
  enc2 = enc;
  testcase( enc2==SQLITE_UTF16 );
  testcase( enc2==SQLITE_UTF16_ALIGNED );
  ಆದರೆ( enc2==SQLITE_UTF16 || enc2==SQLITE_UTF16_ALIGNED ){
    enc2 = SQLITE_UTF16NATIVE;
  }
  ಆದರೆ( enc2<SQLITE_UTF8 || enc2>SQLITE_UTF16BE ){
    ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
  }

  /* Check if this call is removing or replacing an existing collation
  ** sequence. If so, and there are active VMs, return busy. If there
  ** are no active VMs, invalidate any pre-compiled statements.
  */
  pColl = sqlite3_ಕಂಡುಹಿಡಿ_Coll_Seq(db, (u8)enc2, zName, 0);
  ಆದರೆ( pColl && pColl->xCmp ){
    ಆದರೆ( db->n_Vdbe_ಸಕ್ರಿಯ ){
      sqlite3_Error_With_ಸಂದೇಶ(db, SQLITE_BUSY,
        "unable to delete/modify collation sequence due to active statements");
      ಹಿಂತಿರುಗಿ SQLITE_BUSY;
    }
    sqlite3ExpirePreparedStatements(db, 0);

    /* If collation sequence pColl was created directly by a call to
    ** sqlite3_create_collation, and not generated by synthCollSeq(),
    ** then any copies made by synthCollSeq() need to be invalidated.
    ** Also, collation destructor - CollSeq.xDel() - function may need
    ** to be called.
    */
    ಆದರೆ( (pColl->enc & ~SQLITE_UTF16_ALIGNED)==enc2 ){
      CollSeq *aColl = sqlite3_Hash_ಕಂಡುಹಿಡಿ(&db->aCollSeq, zName);
      ಪೂರ್ಣಾಂಕ j;
      ಗಾಗಿ(j=0; j<3; j++){
        CollSeq *p = &aColl[j];
        ಆದರೆ( p->enc==pColl->enc ){
          ಆದರೆ( p->xDel ){
            p->xDel(p->pUser);
          }
          p->xCmp = 0;
        }
      }
    }
  }

  pColl = sqlite3_ಕಂಡುಹಿಡಿ_Coll_Seq(db, (u8)enc2, zName, 1);
  ಆದರೆ( pColl==0 ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
  pColl->xCmp = xCompare;
  pColl->pUser = pCtx;
  pColl->xDel = xDel;
  pColl->enc = (u8)(enc2 | (enc & SQLITE_UTF16_ALIGNED));
  sqlite3Error(db, SQLITE_OK);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}


/*
** This array defines hard upper bounds on limit values.  The
** initializer must be kept in sync with the SQLITE_LIMIT_*
** #defines in sqlite3.h.
*/
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ a_Hard_ಎಲ್ಲೆ[] = {
  SQLITE_MAX_ಉದ್ದಳತೆ,
  SQLITE_MAX_SQL_ಉದ್ದಳತೆ,
  SQLITE_MAX_ಸ್ತಂಭ,
  SQLITE_MAX_ಅಭಿವ್ಯಕ್ತಿ_ಆಳ,
  SQLITE_MAX_COMPOUND_SELECT,
  SQLITE_MAX_VDBE_OP,
  SQLITE_MAX_FUNCTION_ARG,
  SQLITE_MAX_ATTACHED,
  SQLITE_MAX_LIKE_ನಮೂನೆ_ಉದ್ದಳತೆ,
  SQLITE_MAX_VARIABLE_ಸಂಖ್ಯೆ,      /* IMP: R-38091-32352 */
  SQLITE_MAX_TRIGGER_ಆಳ,
  SQLITE_MAX_ಕೆಲಸಗಾರ_ಎಳೆಗಳು,
  SQLITE_MAX_PARSER_ಆಳ,
  SQLITE_MAX_SCHEMA,
  SQLITE_MAX_TRIGGER_ಹೆಜ್ಜೆಗಳು,
  SQLITE_MAX_CTES,
};

/*
** Make sure the hard limits are set to reasonable values
*/
#ಆದರೆ SQLITE_MAX_ಉದ್ದಳತೆ<100
# error SQLITE_MAX_ಉದ್ದಳತೆ must be at ಕನಿಷ್ಠ 100
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ SQLITE_MAX_SQL_ಉದ್ದಳತೆ<100
# error SQLITE_MAX_SQL_ಉದ್ದಳತೆ must be at ಕನಿಷ್ಠ 100
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ SQLITE_MAX_SQL_ಉದ್ದಳತೆ>SQLITE_MAX_ಉದ್ದಳತೆ
# error SQLITE_MAX_SQL_ಉದ್ದಳತೆ must not be greater than SQLITE_MAX_ಉದ್ದಳತೆ
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ SQLITE_MAX_SQL_ಉದ್ದಳತೆ>2147482624     /* 1024 less than 2^31 */
# error SQLITE_MAX_SQL_ಉದ್ದಳತೆ must not be greater than 2147482624
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ SQLITE_MAX_COMPOUND_SELECT<2
# error SQLITE_MAX_COMPOUND_SELECT must be at ಕನಿಷ್ಠ 2
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ SQLITE_MAX_VDBE_OP<40
# error SQLITE_MAX_VDBE_OP must be at ಕನಿಷ್ಠ 40
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ SQLITE_MAX_FUNCTION_ARG<0 || SQLITE_MAX_FUNCTION_ARG>32767
# error SQLITE_MAX_FUNCTION_ARG must be between 0 and 32767
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ SQLITE_MAX_ATTACHED<0 || SQLITE_MAX_ATTACHED>125
# error SQLITE_MAX_ATTACHED must be between 0 and 125
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ SQLITE_MAX_LIKE_ನಮೂನೆ_ಉದ್ದಳತೆ<1
# error SQLITE_MAX_LIKE_ನಮೂನೆ_ಉದ್ದಳತೆ must be at ಕನಿಷ್ಠ 1
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ SQLITE_MAX_ಸ್ತಂಭ>32767
# error SQLITE_MAX_ಸ್ತಂಭ must not exceed 32767
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ SQLITE_MAX_TRIGGER_ಆಳ<1
# error SQLITE_MAX_TRIGGER_ಆಳ must be at ಕನಿಷ್ಠ 1
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ SQLITE_MAX_ಕೆಲಸಗಾರ_ಎಳೆಗಳು<0 || SQLITE_MAX_ಕೆಲಸಗಾರ_ಎಳೆಗಳು>50
# error SQLITE_MAX_ಕೆಲಸಗಾರ_ಎಳೆಗಳು must be between 0 and 50
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ SQLITE_MAX_TRIGGER_ಹೆಜ್ಜೆಗಳು<0 || SQLITE_MAX_TRIGGER_ಹೆಜ್ಜೆಗಳು>65535
# error SQLITE_MAX_TRIGGER_ಹೆಜ್ಜೆಗಳು must be between 0 and 65535
#ಆದರೆ_ಅಂತ್ಯ


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
ಪೂರ್ಣಾಂಕ sqlite3_ಎಲ್ಲೆ(sqlite3 *db, ಪೂರ್ಣಾಂಕ ಎಲ್ಲೆ_Id, ಪೂರ್ಣಾಂಕ new_ಎಲ್ಲೆ){
  ಪೂರ್ಣಾಂಕ old_ಎಲ್ಲೆ;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !sqlite3SafetyCheckOk(db) ){
    (ಶೂನ್ಯ)SQLITE_MISUSE_BKPT;
    ಹಿಂತಿರುಗಿ -1;
  }
#ಆದರೆ_ಅಂತ್ಯ

  /* EVIDENCE-OF: R-30189-54097 For each limit category SQLITE_LIMIT_NAME
  ** there is a hard upper bound set at compile-time by a C preprocessor
  ** macro called SQLITE_MAX_NAME. (The "_LIMIT_" in the name is changed to
  ** "_MAX_".)
  */
  ಪ್ರತಿಜ್ಞಾ( a_Hard_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_ಉದ್ದಳತೆ]==SQLITE_MAX_ಉದ್ದಳತೆ );
  ಪ್ರತಿಜ್ಞಾ( a_Hard_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_SQL_ಉದ್ದಳತೆ]==SQLITE_MAX_SQL_ಉದ್ದಳತೆ );
  ಪ್ರತಿಜ್ಞಾ( a_Hard_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_ಸ್ತಂಭ]==SQLITE_MAX_ಸ್ತಂಭ );
  ಪ್ರತಿಜ್ಞಾ( a_Hard_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_ಅಭಿವ್ಯಕ್ತಿ_ಆಳ]==SQLITE_MAX_ಅಭಿವ್ಯಕ್ತಿ_ಆಳ );
  ಪ್ರತಿಜ್ಞಾ( a_Hard_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_PARSER_ಆಳ]==SQLITE_MAX_PARSER_ಆಳ );
  ಪ್ರತಿಜ್ಞಾ( a_Hard_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_COMPOUND_SELECT]==SQLITE_MAX_COMPOUND_SELECT);
  ಪ್ರತಿಜ್ಞಾ( a_Hard_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_VDBE_OP]==SQLITE_MAX_VDBE_OP );
  ಪ್ರತಿಜ್ಞಾ( a_Hard_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_FUNCTION_ARG]==SQLITE_MAX_FUNCTION_ARG );
  ಪ್ರತಿಜ್ಞಾ( a_Hard_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_ATTACHED]==SQLITE_MAX_ATTACHED );
  ಪ್ರತಿಜ್ಞಾ( a_Hard_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_LIKE_ನಮೂನೆ_ಉದ್ದಳತೆ]==
                                               SQLITE_MAX_LIKE_ನಮೂನೆ_ಉದ್ದಳತೆ );
  ಪ್ರತಿಜ್ಞಾ( a_Hard_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_VARIABLE_ಸಂಖ್ಯೆ]==SQLITE_MAX_VARIABLE_ಸಂಖ್ಯೆ);
  ಪ್ರತಿಜ್ಞಾ( a_Hard_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_TRIGGER_ಆಳ]==SQLITE_MAX_TRIGGER_ಆಳ );
  ಪ್ರತಿಜ್ಞಾ( a_Hard_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_ಕೆಲಸಗಾರ_ಎಳೆಗಳು]==SQLITE_MAX_ಕೆಲಸಗಾರ_ಎಳೆಗಳು );
  ಪ್ರತಿಜ್ಞಾ( a_Hard_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_SCHEMA]==SQLITE_MAX_SCHEMA );
  ಪ್ರತಿಜ್ಞಾ( a_Hard_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_TRIGGER_ಹೆಜ್ಜೆಗಳು]==SQLITE_MAX_TRIGGER_ಹೆಜ್ಜೆಗಳು );
  ಪ್ರತಿಜ್ಞಾ( a_Hard_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_CTES]==SQLITE_MAX_CTES );
  ಪ್ರತಿಜ್ಞಾ( SQLITE_ಎಲ್ಲೆ_CTES==(SQLITE_N_ಎಲ್ಲೆ-1) );


  ಆದರೆ( ಎಲ್ಲೆ_Id<0 || ಎಲ್ಲೆ_Id>=SQLITE_N_ಎಲ್ಲೆ ){
    ಹಿಂತಿರುಗಿ -1;
  }
  sqlite3_mutex_enter(db->mutex);
  old_ಎಲ್ಲೆ = db->a_ಎಲ್ಲೆ[ಎಲ್ಲೆ_Id];
  ಆದರೆ( new_ಎಲ್ಲೆ>=0 ){                   /* IMP: R-52476-28732 */
    ಆದರೆ( new_ಎಲ್ಲೆ>a_Hard_ಎಲ್ಲೆ[ಎಲ್ಲೆ_Id] ){
      new_ಎಲ್ಲೆ = a_Hard_ಎಲ್ಲೆ[ಎಲ್ಲೆ_Id];  /* IMP: R-51463-25634 */
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( new_ಎಲ್ಲೆ<SQLITE_MIN_ಉದ್ದಳತೆ && ಎಲ್ಲೆ_Id==SQLITE_ಎಲ್ಲೆ_ಉದ್ದಳತೆ ){
      new_ಎಲ್ಲೆ = SQLITE_MIN_ಉದ್ದಳತೆ;
    }
    db->a_ಎಲ್ಲೆ[ಎಲ್ಲೆ_Id] = new_ಎಲ್ಲೆ;
  }
  sqlite3_mutex_leave(db->mutex);
  ಹಿಂತಿರುಗಿ old_ಎಲ್ಲೆ;                     /* IMP: R-53341-35419 */
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
ಪೂರ್ಣಾಂಕ sqlite3ParseUri(
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDefaultVfs,        /* VFS to use if no "vfs=xxx" query option */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zUri,               /* Nul-terminated URI to parse */
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *pFlags,           /* IN/OUT: SQLITE_OPEN_XXX flags */
  sqlite3_vfs **ppVfs,            /* OUT: VFS to use */
  ಅಕ್ಷರ **pzFile,                  /* OUT: Filename component of URI */
  ಅಕ್ಷರ **pz_Err_ಸಂದೇಶ                 /* OUT: Error message (if rc!=SQLITE_OK) */
){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ flags = *pFlags;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zVfs = zDefaultVfs;
  ಅಕ್ಷರ *zFile;
  ಅಕ್ಷರ c;
  i64 nUri = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(zUri);

  ಪ್ರತಿಜ್ಞಾ( *pz_Err_ಸಂದೇಶ==0 );

  ಆದರೆ( ((flags & SQLITE_OPEN_URI)                     /* IMP: R-48725-32206 */
       || Atomic_ತುಂಬು(&sqlite3GlobalConfig.bOpenUri)) /* IMP: R-51689-46548 */
   && nUri>=5 && memcmp(zUri, "file:", 5)==0         /* IMP: R-57884-37496 */
  ){
    ಅಕ್ಷರ *zOpt;
    ಪೂರ್ಣಾಂಕ eState;                   /* Parser state when parsing URI */
    i64 i_ಒಳ;                      /* Input character index */
    i64 i_ಹೊರಹೋಗು = 0;                 /* Output character index */
    u64 nByte = nUri+8;           /* Bytes of space to allocate */

    /* Make sure the SQLITE_OPEN_URI flag is set to indicate to the VFS xOpen
    ** method that there may be extra parameters following the file-name.  */
    flags |= SQLITE_OPEN_URI;

    ಗಾಗಿ(i_ಒಳ=0; i_ಒಳ<nUri; i_ಒಳ++) nByte += (zUri[i_ಒಳ]=='&');
    zFile = sqlite3_malloc64(nByte);
    ಆದರೆ( !zFile ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;

    memset(zFile, 0, 4);  /* 4-byte of 0x00 is the start of DB name marker */
    zFile += 4;

    i_ಒಳ = 5;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ALLOW_URI_AUTHORITY
    ಆದರೆ( ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(zUri+5, "///", 3)==0 ){
      i_ಒಳ = 7;
      /* The following condition causes URIs with five leading / characters
      ** like file://///host/path to be converted into UNCs like //host/path.
      ** The correct URI for that UNC has only two or four leading / characters
      ** file://host/path or file:////host/path.  But 5 leading slashes is a
      ** common error, we are told, so we handle it as a special case. */
      ಆದರೆ( ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(zUri+7, "///", 3)==0 ){ i_ಒಳ++; }
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(zUri+5, "//localhost/", 12)==0 ){
      i_ಒಳ = 16;
    }
#ಇಲ್ಲದಿದ್ದರೆ
    /* Discard the scheme and authority segments of the URI. */
    ಆದರೆ( zUri[5]=='/' && zUri[6]=='/' ){
      i_ಒಳ = 7;
      ಆಗ( zUri[i_ಒಳ] && zUri[i_ಒಳ]!='/' ) i_ಒಳ++;
      ಆದರೆ( i_ಒಳ!=7 && (i_ಒಳ!=16 || memcmp("localhost", &zUri[7], 9)) ){
        *pz_Err_ಸಂದೇಶ = sqlite3_mprintf("invalid uri authority: %.*s",
            (ಪೂರ್ಣಾಂಕ)(i_ಒಳ-7), &zUri[7]);
        rc = SQLITE_ERROR;
        ಹೋಗು parse_uri_ಹೊರಹೋಗು;
      }
    }
#ಆದರೆ_ಅಂತ್ಯ

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
    ಆಗ( (c = zUri[i_ಒಳ])!=0 && c!='#' ){
      i_ಒಳ++;
      ಆದರೆ( c=='%'
       && sqlite3Isxdigit(zUri[i_ಒಳ])
       && sqlite3Isxdigit(zUri[i_ಒಳ+1])
      ){
        ಪೂರ್ಣಾಂಕ octet = (sqlite3_Hex_ಗಮ್ಯ_Int(zUri[i_ಒಳ++]) << 4);
        octet += sqlite3_Hex_ಗಮ್ಯ_Int(zUri[i_ಒಳ++]);

        ಪ್ರತಿಜ್ಞಾ( octet>=0 && octet<256 );
        ಆದರೆ( octet==0 ){
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_ENABLE_URI_00_ERROR
          /* This branch is taken when "%00" appears within the URI. In this
          ** case we ignore all text in the remainder of the path, name or
          ** value currently being parsed. So ignore the current character
          ** and skip to the next "?", "=" or "&", as appropriate. */
          ಆಗ( (c = zUri[i_ಒಳ])!=0 && c!='#'
              && (eState!=0 || c!='?')
              && (eState!=1 || (c!='=' && c!='&'))
              && (eState!=2 || c!='&')
          ){
            i_ಒಳ++;
          }
          ಮುಂದುವರಿ;
#ಇಲ್ಲದಿದ್ದರೆ
          /* If ENABLE_URI_00_ERROR is defined, "%00" in a URI is an error. */
          *pz_Err_ಸಂದೇಶ = sqlite3_mprintf("unexpected %%00 in uri");
          rc = SQLITE_ERROR;
          ಹೋಗು parse_uri_ಹೊರಹೋಗು;
#ಆದರೆ_ಅಂತ್ಯ
        }
        c = octet;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( eState==1 && (c=='&' || c=='=') ){
        ಆದರೆ( zFile[i_ಹೊರಹೋಗು-1]==0 ){
          /* An empty option name. Ignore this option altogether. */
          ಆಗ( zUri[i_ಒಳ] && zUri[i_ಒಳ]!='#' && zUri[i_ಒಳ-1]!='&' ) i_ಒಳ++;
          ಮುಂದುವರಿ;
        }
        ಆದರೆ( c=='&' ){
          zFile[i_ಹೊರಹೋಗು++] = '\0';
        }ಇಲ್ಲದಿದ್ದರೆ{
          eState = 2;
        }
        c = 0;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( (eState==0 && c=='?') || (eState==2 && c=='&') ){
        c = 0;
        eState = 1;
      }
      zFile[i_ಹೊರಹೋಗು++] = c;
    }
    ಆದರೆ( eState==1 ) zFile[i_ಹೊರಹೋಗು++] = '\0';
    memset(zFile+i_ಹೊರಹೋಗು, 0, 4); /* end-of-options + empty journal filenames */

    /* Check if there were any options specified that should be interpreted
    ** here. Options that are interpreted here include "vfs" and those that
    ** correspond to flags that may be passed to the sqlite3_open_v2()
    ** method. */
    zOpt = &zFile[ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(zFile)+1];
    ಆಗ( zOpt[0] ){
      i64 nOpt = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(zOpt);
      ಅಕ್ಷರ *zVal = &zOpt[nOpt+1];
      i64 nVal = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(zVal);

      ಆದರೆ( nOpt==3 && memcmp("vfs", zOpt, 3)==0 ){
        zVfs = zVal;
      }ಇಲ್ಲದಿದ್ದರೆ{
        ರಚನೆ Open_ಬಹುಲಕ {
          ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z;
          ಪೂರ್ಣಾಂಕ ಬಹುಲಕ;
        } *a_ಬಹುಲಕ = 0;
        ಅಕ್ಷರ *z_ಬಹುಲಕ_Type = 0;
        ಪೂರ್ಣಾಂಕ ಮುಖವಾಡ = 0;
        ಪೂರ್ಣಾಂಕ ಎಲ್ಲೆ = 0;

        ಆದರೆ( nOpt==5 && memcmp("cache", zOpt, 5)==0 ){
          ಸ್ಥಿರ ರಚನೆ Open_ಬಹುಲಕ a_Cache_ಬಹುಲಕ[] = {
            { "shared",  SQLITE_OPEN_SHAREDCACHE },
            { "private", SQLITE_OPEN_PRIVATECACHE },
            { 0, 0 }
          };

          ಮುಖವಾಡ = SQLITE_OPEN_SHAREDCACHE|SQLITE_OPEN_PRIVATECACHE;
          a_ಬಹುಲಕ = a_Cache_ಬಹುಲಕ;
          ಎಲ್ಲೆ = ಮುಖವಾಡ;
          z_ಬಹುಲಕ_Type = "cache";
        }
        ಆದರೆ( nOpt==4 && memcmp("mode", zOpt, 4)==0 ){
          ಸ್ಥಿರ ರಚನೆ Open_ಬಹುಲಕ a_Open_ಬಹುಲಕ[] = {
            { "ro",  SQLITE_OPEN_READONLY },
            { "rw",  SQLITE_OPEN_READWRITE },
            { "rwc", SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE },
            { "memory", SQLITE_OPEN_ಸ್ಮರಣೆ },
            { 0, 0 }
          };

          ಮುಖವಾಡ = SQLITE_OPEN_READONLY | SQLITE_OPEN_READWRITE
                   | SQLITE_OPEN_CREATE | SQLITE_OPEN_ಸ್ಮರಣೆ;
          a_ಬಹುಲಕ = a_Open_ಬಹುಲಕ;
          ಎಲ್ಲೆ = ಮುಖವಾಡ & flags;
          z_ಬಹುಲಕ_Type = "access";
        }

        ಆದರೆ( a_ಬಹುಲಕ ){
          ಪೂರ್ಣಾಂಕ i;
          ಪೂರ್ಣಾಂಕ ಬಹುಲಕ = 0;
          ಗಾಗಿ(i=0; a_ಬಹುಲಕ[i].z; i++){
            ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z = a_ಬಹುಲಕ[i].z;
            ಆದರೆ( nVal==(i64)ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(z) && 0==memcmp(zVal, z, nVal) ){
              ಬಹುಲಕ = a_ಬಹುಲಕ[i].ಬಹುಲಕ;
              ನಿಲ್ಲಿಸು;
            }
          }
          ಆದರೆ( ಬಹುಲಕ==0 ){
            *pz_Err_ಸಂದೇಶ = sqlite3_mprintf("no such %s mode: %s", z_ಬಹುಲಕ_Type, zVal);
            rc = SQLITE_ERROR;
            ಹೋಗು parse_uri_ಹೊರಹೋಗು;
          }
          ಆದರೆ( (ಬಹುಲಕ & ~SQLITE_OPEN_ಸ್ಮರಣೆ)>ಎಲ್ಲೆ ){
            *pz_Err_ಸಂದೇಶ = sqlite3_mprintf("%s mode not allowed: %s",
                                        z_ಬಹುಲಕ_Type, zVal);
            rc = SQLITE_PERM;
            ಹೋಗು parse_uri_ಹೊರಹೋಗು;
          }
          flags = (flags & ~ಮುಖವಾಡ) | ಬಹುಲಕ;
        }
      }

      zOpt = &zVal[nVal+1];
    }

  }ಇಲ್ಲದಿದ್ದರೆ{
    zFile = sqlite3_malloc64(nUri+8);
    ಆದರೆ( !zFile ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
    memset(zFile, 0, 4);
    zFile += 4;
    ಆದರೆ( nUri ){
      ಸ್ಮರಣೆ_ನಕಲು(zFile, zUri, nUri);
    }
    memset(zFile+nUri, 0, 4);
    flags &= ~SQLITE_OPEN_URI;
  }

  *ppVfs = sqlite3_vfs_ಕಂಡುಹಿಡಿ(zVfs);
  ಆದರೆ( *ppVfs==0 ){
    *pz_Err_ಸಂದೇಶ = sqlite3_mprintf("no such vfs: %s", zVfs);
    rc = SQLITE_ERROR;
  }
 parse_uri_ಹೊರಹೋಗು:
  ಆದರೆ( rc!=SQLITE_OK ){
    sqlite3_free_filename(zFile);
    zFile = 0;
  }
  *pFlags = flags;
  *pzFile = zFile;
  ಹಿಂತಿರುಗಿ rc;
}

/*
** This routine does the core work of extracting URI parameters from a
** database filename for the sqlite3_uri_parameter() interface.
*/
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *uriParameter(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFilename, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zParam){
  zFilename += sqlite3Strlen30(zFilename) + 1;
  ಆಗ( ALWAYS(zFilename!=0) && zFilename[0] ){
    ಪೂರ್ಣಾಂಕ x = strcmp(zFilename, zParam);
    zFilename += sqlite3Strlen30(zFilename) + 1;
    ಆದರೆ( x==0 ) ಹಿಂತಿರುಗಿ zFilename;
    zFilename += sqlite3Strlen30(zFilename) + 1;
  }
  ಹಿಂತಿರುಗಿ 0;
}



/*
** This routine does the work of opening a database on behalf of
** sqlite3_open() and sqlite3_open16(). The database filename "zFilename" 
** is UTF-8 encoded.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ openDatabase(
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFilename, /* Database filename UTF-8 encoded */
  sqlite3 **ppDb,        /* OUT: Returned database handle */
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ flags,    /* Operational flags */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zVfs       /* Name of the VFS to use */
){
  sqlite3 *db;                    /* Store allocated handle here */
  ಪೂರ್ಣಾಂಕ rc;                         /* Return code */
  ಪೂರ್ಣಾಂಕ isThreadsafe;               /* True for threadsafe connections */
  ಅಕ್ಷರ *zOpen = 0;                /* Filename argument to pass to BtreeOpen() */
  ಅಕ್ಷರ *z_Err_ಸಂದೇಶ = 0;              /* Error message from sqlite3ParseUri() */
  ಪೂರ್ಣಾಂಕ i;                          /* Loop counter */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( ppDb==0 ) ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
#ಆದರೆ_ಅಂತ್ಯ
  *ppDb = 0;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOINIT
  rc = sqlite3_initialize();
  ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ rc;
#ಆದರೆ_ಅಂತ್ಯ

  ಆದರೆ( sqlite3GlobalConfig.bCoreMutex==0 ){
    isThreadsafe = 0;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( flags & SQLITE_OPEN_NOMUTEX ){
    isThreadsafe = 0;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( flags & SQLITE_OPEN_FULLMUTEX ){
    isThreadsafe = 1;
  }ಇಲ್ಲದಿದ್ದರೆ{
    isThreadsafe = sqlite3GlobalConfig.b_ತುಂಬಿದ_Mutex;
  }

  ಆದರೆ( flags & SQLITE_OPEN_PRIVATECACHE ){
    flags &= ~SQLITE_OPEN_SHAREDCACHE;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( sqlite3GlobalConfig.sharedCacheEnabled ){
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
               SQLITE_OPEN_ತಾತ್ಕಾಲಿಕ_DB |
               SQLITE_OPEN_TRANSIENT_DB |
               SQLITE_OPEN_MAIN_JOURNAL |
               SQLITE_OPEN_ತಾತ್ಕಾಲಿಕ_JOURNAL |
               SQLITE_OPEN_SUBJOURNAL |
               SQLITE_OPEN_SUPER_JOURNAL |
               SQLITE_OPEN_NOMUTEX |
               SQLITE_OPEN_FULLMUTEX |
               SQLITE_OPEN_WAL
             );

  /* Allocate the sqlite data structure */
  db = sqlite3MallocZero( ಗಾತ್ರ(sqlite3) );
  ಆದರೆ( db==0 ) ಹೋಗು opendb_ಹೊರಹೋಗು;
  ಆದರೆ( isThreadsafe
#ಆದರೆ defined(SQLITE_THREAD_MISUSE_WARNINGS)
   || sqlite3GlobalConfig.bCoreMutex
#ಆದರೆ_ಅಂತ್ಯ
  ){
    db->mutex = sqlite3MutexAlloc(SQLITE_MUTEX_RECURSIVE);
    ಆದರೆ( db->mutex==0 ){
      sqlite3_free(db);
      db = 0;
      ಹೋಗು opendb_ಹೊರಹೋಗು;
    }
    ಆದರೆ( isThreadsafe==0 ){
      sqlite3MutexWarnOnContention(db->mutex);
    }
  }
  sqlite3_mutex_enter(db->mutex);
  db->err_ಮುಖವಾಡ = (flags & SQLITE_OPEN_EXRESCODE)!=0 ? 0xffffffff : 0xff;
  db->nDb = 2;
  db->eOpenState = SQLITE_STATE_BUSY;
  db->aDb = db->aDbStatic;
  db->lookaside.bDisable = 1;
  db->lookaside.sz = 0;
  db->nFpDigit = 17;

  ಪ್ರತಿಜ್ಞಾ( ಗಾತ್ರ(db->a_ಎಲ್ಲೆ)==ಗಾತ್ರ(a_Hard_ಎಲ್ಲೆ) );
  ಸ್ಮರಣೆ_ನಕಲು(db->a_ಎಲ್ಲೆ, a_Hard_ಎಲ್ಲೆ, ಗಾತ್ರ(db->a_ಎಲ್ಲೆ));
  db->a_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_ಕೆಲಸಗಾರ_ಎಳೆಗಳು] = SQLITE_DEFAULT_ಕೆಲಸಗಾರ_ಎಳೆಗಳು;
  db->autoCommit = 1;
  db->ಮುಂದಿನ_Autovac = -1;
  db->szMmap = sqlite3GlobalConfig.szMmap;
  db->ಮುಂದಿನ_Pagesize = 0;
  db->init.azInit = sqlite3StdType; /* Any array of string ptrs will do */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_SORTER_MMAP
  /* Beginning with version 3.37.0, using the VFS xFetch() API to memory-map
  ** the temporary files used to do external sorts (see code in vdbesort.c)
  ** is disabled. It can still be used either by defining
  ** SQLITE_ENABLE_SORTER_MMAP at compile time or by using the
  ** SQLITE_TESTCTRL_SORTER_MMAP test-control at runtime. */
  db->nMaxSorterMmap = 0x7FFFFFFF;
#ಆದರೆ_ಅಂತ್ಯ
  db->flags |= SQLITE_Short_ಲಂಬಸಾಲು_ಹೆಸರುಗಳು
                 | SQLITE_EnableTrigger
                 | SQLITE_EnableView
                 | SQLITE_CacheSpill
                 | SQLITE_AttachCreate
                 | SQLITE_AttachWrite
                 | SQLITE_Comments
#ಆದರೆ !defined(SQLITE_TRUSTED_SCHEMA) || SQLITE_TRUSTED_SCHEMA+0!=0
                 | SQLITE_TrustedSchema
#ಆದರೆ_ಅಂತ್ಯ
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
#ಆದರೆ !defined(SQLITE_DQS)
# define SQLITE_DQS 3
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ (SQLITE_DQS&1)==1
                 | SQLITE_DqsDML
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ (SQLITE_DQS&2)==2
                 | SQLITE_DqsDDL
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ !defined(SQLITE_DEFAULT_AUTOMATIC_ಸೂಚ್ಯಂಕ) || SQLITE_DEFAULT_AUTOMATIC_ಸೂಚ್ಯಂಕ
                 | SQLITE_Auto_ಸೂಚ್ಯಂಕ
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ SQLITE_DEFAULT_CKPTFULLFSYNC
                 | SQLITE_Ckpt_ತುಂಬಿದ_FSync
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ SQLITE_DEFAULT_FILE_FORMAT<4
                 | SQLITE_LegacyFileFmt
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_ತುಂಬು_EXTENSION
                 | SQLITE_ತುಂಬು_Extension
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ SQLITE_DEFAULT_RECURSIVE_TRIGGERS
                 | SQLITE_RecTriggers
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(SQLITE_DEFAULT_FOREIGN_ಕೀಲಿಗಳು) && SQLITE_DEFAULT_FOREIGN_ಕೀಲಿಗಳು
                 | SQLITE_Foreign_ಕೀಲಿಗಳು
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(SQLITE_ಹಿಮ್ಮುಖ_UNORDERED_SELECTS)
                 | SQLITE_ಹಿಮ್ಮುಖ_ಕ್ರಮ
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(SQLITE_ENABLE_OVERSIZE_CELL_CHECK)
                 | SQLITE_Cell_ಗಾತ್ರ_Ck
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(SQLITE_ENABLE_FTS3_TOKENIZER)
                 | SQLITE_Fts3Tokenizer
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(SQLITE_ENABLE_QPSG)
                 | SQLITE_EnableQPSG
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(SQLITE_DEFAULT_DEFENSIVE)
                 | SQLITE_Defensive
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(SQLITE_DEFAULT_LEGACY_ALTER_ಕೋಷ್ಟಕ)
                 | SQLITE_LegacyAlter
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(SQLITE_ENABLE_STMT_SCANSTATUS)
                 | SQLITE_Stmt_ಪರಿಶೀಲನೆ_ಸ್ಥಿತಿ
#ಆದರೆ_ಅಂತ್ಯ
      ;
  sqlite3HashInit(&db->aCollSeq);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
  sqlite3HashInit(&db->aModule);
#ಆದರೆ_ಅಂತ್ಯ

  /* Add the default collation sequence BINARY. BINARY works for both UTF-8
  ** and UTF-16, so add a version for each to avoid any unnecessary
  ** conversions. The only error that can occur here is a malloc() failure.
  **
  ** EVIDENCE-OF: R-52786-44878 SQLite defines three built-in collating
  ** functions:
  */
  createCollation(db, sqlite3_Str_ದ್ವಿಮಾನ, SQLITE_UTF8, 0,
                  sqlite3_ದ್ವಿಮಾನ_Compare, 0);
  createCollation(db, sqlite3_Str_ದ್ವಿಮಾನ, SQLITE_UTF16BE, 0,
                  sqlite3_ದ್ವಿಮಾನ_Compare, 0);
  createCollation(db, sqlite3_Str_ದ್ವಿಮಾನ, SQLITE_UTF16LE, 0,
                  sqlite3_ದ್ವಿಮಾನ_Compare, 0);
  createCollation(db, "NOCASE", SQLITE_UTF8, 0, nocaseCollatingFunc, 0);
  createCollation(db, "RTRIM", SQLITE_UTF8, 0, rtrimCollFunc, 0);
  ಆದರೆ( db->mallocFailed ){
    ಹೋಗು opendb_ಹೊರಹೋಗು;
  }

#ಆದರೆ SQLITE_OS_UNIX && defined(SQLITE_OS_KV_OPTIONAL)
  /* Process magic filenames ":localStorage:" and ":sessionStorage:" */
  ಆದರೆ( zFilename && zFilename[0]==':' ){
    ಆದರೆ( strcmp(zFilename, ":localStorage:")==0 ){
      zFilename = "file:local?vfs=kvvfs";
      flags |= SQLITE_OPEN_URI;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( strcmp(zFilename, ":sessionStorage:")==0 ){
      zFilename = "file:session?vfs=kvvfs";
      flags |= SQLITE_OPEN_URI;
    }
  }
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OS_UNIX && defined(SQLITE_OS_KV_OPTIONAL) */

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
  ಪ್ರತಿಜ್ಞಾ( SQLITE_OPEN_READONLY  == 0x01 );
  ಪ್ರತಿಜ್ಞಾ( SQLITE_OPEN_READWRITE == 0x02 );
  ಪ್ರತಿಜ್ಞಾ( SQLITE_OPEN_CREATE    == 0x04 );
  testcase( (1<<(flags&7))==0x02 ); /* READONLY */
  testcase( (1<<(flags&7))==0x04 ); /* READWRITE */
  testcase( (1<<(flags&7))==0x40 ); /* READWRITE | CREATE */
  ಆದರೆ( ((1<<(flags&7)) & 0x46)==0 ){
    rc = SQLITE_MISUSE_BKPT;  /* IMP: R-18321-05872 */
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಆದರೆ( zFilename==0 ) zFilename = ":memory:";
    rc = sqlite3ParseUri(zVfs, zFilename, &flags, &db->pVfs, &zOpen, &z_Err_ಸಂದೇಶ);
  }
  ಆದರೆ( rc!=SQLITE_OK ){
    ಆದರೆ( rc==SQLITE_NOMEM ) sqlite3OomFault(db);
    sqlite3_Error_With_ಸಂದೇಶ(db, rc, z_Err_ಸಂದೇಶ ? "%s" : 0, z_Err_ಸಂದೇಶ);
    sqlite3_free(z_Err_ಸಂದೇಶ);
    ಹೋಗು opendb_ಹೊರಹೋಗು;
  }
  ಪ್ರತಿಜ್ಞಾ( db->pVfs!=0 );
#ಆದರೆ SQLITE_OS_KV || defined(SQLITE_OS_KV_OPTIONAL)
  ಆದರೆ( sqlite3_stricmp(db->pVfs->zName, "kvvfs")==0 ){
    db->ತಾತ್ಕಾಲಿಕ_store = 2;
  }
#ಆದರೆ_ಅಂತ್ಯ

  /* Open the backend database driver */
  rc = sqlite3BtreeOpen(db->pVfs, zOpen, db, &db->aDb[0].pBt, 0,
                        flags | SQLITE_OPEN_MAIN_DB);
  ಆದರೆ( rc!=SQLITE_OK ){
    ಆದರೆ( rc==SQLITE_IOERR_NOMEM ){
      rc = SQLITE_NOMEM_BKPT;
    }
    sqlite3Error(db, rc);
    ಹೋಗು opendb_ಹೊರಹೋಗು;
  }
  sqlite3BtreeEnter(db->aDb[0].pBt);
  db->aDb[0].pSchema = sqlite3SchemaGet(db, db->aDb[0].pBt);
  ಆದರೆ( !db->mallocFailed ){
    sqlite3_ಗಣ_ಪಠ್ಯ_Encoding(db, SCHEMA_ENC(db));
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
  ಆದರೆ( db->mallocFailed ){
    ಹೋಗು opendb_ಹೊರಹೋಗು;
  }

  /* Register all built-in functions, but do not attempt to read the
  ** database schema yet. This is delayed until the first time the database
  ** is accessed.
  */
  sqlite3Error(db, SQLITE_OK);
  sqlite3_ರಿಜಿಸ್ಟರ್_ಪ್ರತಿ_Connection_Builtin_Functions(db);
  rc = sqlite3_errcode(db);


  /* Load compiled-in extensions */
  ಗಾಗಿ(i=0; rc==SQLITE_OK && i<Array_ಗಾತ್ರ(sqlite3BuiltinExtensions); i++){
    rc = sqlite3BuiltinExtensions[i](db);
  }

  /* Load automatic extensions - extensions that have been registered
  ** using the sqlite3_automatic_extension() API.
  */
  ಆದರೆ( rc==SQLITE_OK ){
    sqlite3_Auto_ತುಂಬು_Extensions(db);
    rc = sqlite3_errcode(db);
    ಆದರೆ( rc!=SQLITE_OK ){
      ಹೋಗು opendb_ಹೊರಹೋಗು;
    }
  }

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_ಆಂತರಿಕ_FUNCTIONS
  /* Testing use only!!! The -DSQLITE_ENABLE_INTERNAL_FUNCTIONS=1 compile-time
  ** option gives access to internal functions by default. 
  ** Testing use only!!! */
  db->mDbFlags |= DBFLAG_ಆಂತರಿಕ_Func;
#ಆದರೆ_ಅಂತ್ಯ

  /* -DSQLITE_DEFAULT_LOCKING_MODE=1 makes EXCLUSIVE the default locking
  ** mode.  -DSQLITE_DEFAULT_LOCKING_MODE=0 make NORMAL the default locking
  ** mode.  Doing nothing at all also makes NORMAL the default.
  */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEFAULT_LOCKING_ಬಹುಲಕ
  db->dflt_ಬೀಗ_ಬಹುಲಕ = SQLITE_DEFAULT_LOCKING_ಬಹುಲಕ;
  sqlite3_Pager_Locking_ಬಹುಲಕ(sqlite3BtreePager(db->aDb[0].pBt),
                          SQLITE_DEFAULT_LOCKING_ಬಹುಲಕ);
#ಆದರೆ_ಅಂತ್ಯ

  ಆದರೆ( rc ) sqlite3Error(db, rc);

  /* Enable the lookaside-malloc subsystem */
  ಸಿದ್ಧತೆ_Lookaside(db, 0, sqlite3GlobalConfig.szLookaside,
                        sqlite3GlobalConfig.nLookaside);

  sqlite3_wal_autocheckpoint(db, SQLITE_DEFAULT_WAL_AUTOCHECKPOINT);

opendb_ಹೊರಹೋಗು:
  ಆದರೆ( db ){
    ಪ್ರತಿಜ್ಞಾ( db->mutex!=0 || isThreadsafe==0
           || sqlite3GlobalConfig.b_ತುಂಬಿದ_Mutex==0 );
    sqlite3_mutex_leave(db->mutex);
  }
  rc = sqlite3_errcode(db);
  ಪ್ರತಿಜ್ಞಾ( db!=0 || (rc&0xff)==SQLITE_NOMEM );
  ಆದರೆ( (rc&0xff)==SQLITE_NOMEM ){
    sqlite3_close(db);
    db = 0;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( rc!=SQLITE_OK ){
    db->eOpenState = SQLITE_STATE_SICK;
  }
  *ppDb = db;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_SQLLOG
  ಆದರೆ( sqlite3GlobalConfig.xSqllog ){
    /* Opening a db handle. Fourth parameter is passed 0. */
    ಶೂನ್ಯ *pArg = sqlite3GlobalConfig.pSqllogArg;
    sqlite3GlobalConfig.xSqllog(pArg, db, zFilename, 0);
  }
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_free_filename(zOpen);
  ಹಿಂತಿರುಗಿ rc;
}


/*
** Open a new database handle.
*/
ಪೂರ್ಣಾಂಕ sqlite3_open(
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFilename,
  sqlite3 **ppDb
){
  ಹಿಂತಿರುಗಿ openDatabase(zFilename, ppDb,
                      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, 0);
}
ಪೂರ್ಣಾಂಕ sqlite3_open_v2(
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *filename,   /* Database filename (UTF-8) */
  sqlite3 **ppDb,         /* OUT: SQLite db handle */
  ಪೂರ್ಣಾಂಕ flags,              /* Flags */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zVfs        /* Name of VFS module to use */
){
  ಹಿಂತಿರುಗಿ openDatabase(filename, ppDb, (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)flags, zVfs);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_UTF16
/*
** Open a new database handle.
*/
ಪೂರ್ಣಾಂಕ sqlite3_open16(
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *zFilename,
  sqlite3 **ppDb
){
  ಅಕ್ಷರ ಸ್ಥಿರಾಂಕ *zFilename8;   /* zFilename encoded in UTF-8 instead of UTF-16 */
  sqlite3_ಮೌಲ್ಯ *pVal;
  ಪೂರ್ಣಾಂಕ rc;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( ppDb==0 ) ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
#ಆದರೆ_ಅಂತ್ಯ
  *ppDb = 0;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOINIT
  rc = sqlite3_initialize();
  ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ rc;
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ( zFilename==0 ) zFilename = "\000\000";
  pVal = sqlite3_ಮೌಲ್ಯ_New(0);
  sqlite3_ಮೌಲ್ಯ_ಗಣ_Str(pVal, -1, zFilename, SQLITE_UTF16NATIVE, SQLITE_STATIC);
  zFilename8 = sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ_2(pVal, SQLITE_UTF8);
  ಆದರೆ( zFilename8 ){
    rc = openDatabase(zFilename8, ppDb,
                      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, 0);
    ಪ್ರತಿಜ್ಞಾ( *ppDb || rc==SQLITE_NOMEM );
    ಆದರೆ( rc==SQLITE_OK && !DbHasProperty(*ppDb, 0, DB_SchemaLoaded) ){
      SCHEMA_ENC(*ppDb) = ENC(*ppDb) = SQLITE_UTF16NATIVE;
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    rc = SQLITE_NOMEM_BKPT;
  }
  sqlite3_ಮೌಲ್ಯ_Free(pVal);

  ಹಿಂತಿರುಗಿ rc & 0xff;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_UTF16 */

/*
** Register a new collation sequence with the database handle db.
*/
ಪೂರ್ಣಾಂಕ sqlite3_create_collation(
  sqlite3* db,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName,
  ಪೂರ್ಣಾಂಕ enc,
  ಶೂನ್ಯ* pCtx,
  ಪೂರ್ಣಾಂಕ(*xCompare)(ಶೂನ್ಯ*,ಪೂರ್ಣಾಂಕ,ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*,ಪೂರ್ಣಾಂಕ,ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*)
){
  ಹಿಂತಿರುಗಿ sqlite3_create_collation_v2(db, zName, enc, pCtx, xCompare, 0);
}

/*
** Register a new collation sequence with the database handle db.
*/
ಪೂರ್ಣಾಂಕ sqlite3_create_collation_v2(
  sqlite3* db,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName,
  ಪೂರ್ಣಾಂಕ enc,
  ಶೂನ್ಯ* pCtx,
  ಪೂರ್ಣಾಂಕ(*xCompare)(ಶೂನ್ಯ*,ಪೂರ್ಣಾಂಕ,ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*,ಪೂರ್ಣಾಂಕ,ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*),
  ಶೂನ್ಯ(*xDel)(ಶೂನ್ಯ*)
){
  ಪೂರ್ಣಾಂಕ rc;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !sqlite3SafetyCheckOk(db) || zName==0 ) ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_mutex_enter(db->mutex);
  ಪ್ರತಿಜ್ಞಾ( !db->mallocFailed );
  rc = createCollation(db, zName, (u8)enc, pCtx, xCompare, xDel);
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  ಹಿಂತಿರುಗಿ rc;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_UTF16
/*
** Register a new collation sequence with the database handle db.
*/
ಪೂರ್ಣಾಂಕ sqlite3_create_collation16(
  sqlite3* db,
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *zName,
  ಪೂರ್ಣಾಂಕ enc,
  ಶೂನ್ಯ* pCtx,
  ಪೂರ್ಣಾಂಕ(*xCompare)(ಶೂನ್ಯ*,ಪೂರ್ಣಾಂಕ,ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*,ಪೂರ್ಣಾಂಕ,ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*)
){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  ಅಕ್ಷರ *zName8;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !sqlite3SafetyCheckOk(db) || zName==0 ) ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_mutex_enter(db->mutex);
  ಪ್ರತಿಜ್ಞಾ( !db->mallocFailed );
  zName8 = sqlite3Utf16to8(db, zName, -1, SQLITE_UTF16NATIVE);
  ಆದರೆ( zName8 ){
    rc = createCollation(db, zName8, (u8)enc, pCtx, xCompare, 0);
    sqlite3DbFree(db, zName8);
  }
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  ಹಿಂತಿರುಗಿ rc;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_UTF16 */

/*
** Register a collation sequence factory callback with the database handle
** db. Replace any previously installed collation sequence factory.
*/
ಪೂರ್ಣಾಂಕ sqlite3_collation_needed(
  sqlite3 *db,
  ಶೂನ್ಯ *pCollNeededArg,
  ಶೂನ್ಯ(*xCollNeeded)(ಶೂನ್ಯ*,sqlite3*,ಪೂರ್ಣಾಂಕ e_ಪಠ್ಯ_Rep,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)
){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !sqlite3SafetyCheckOk(db) ) ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_mutex_enter(db->mutex);
  db->xCollNeeded = xCollNeeded;
  db->xCollNeeded16 = 0;
  db->pCollNeededArg = pCollNeededArg;
  sqlite3_mutex_leave(db->mutex);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_UTF16
/*
** Register a collation sequence factory callback with the database handle
** db. Replace any previously installed collation sequence factory.
*/
ಪೂರ್ಣಾಂಕ sqlite3_collation_needed16(
  sqlite3 *db,
  ಶೂನ್ಯ *pCollNeededArg,
  ಶೂನ್ಯ(*xCollNeeded16)(ಶೂನ್ಯ*,sqlite3*,ಪೂರ್ಣಾಂಕ e_ಪಠ್ಯ_Rep,ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*)
){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !sqlite3SafetyCheckOk(db) ) ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_mutex_enter(db->mutex);
  db->xCollNeeded = 0;
  db->xCollNeeded16 = xCollNeeded16;
  db->pCollNeededArg = pCollNeededArg;
  sqlite3_mutex_leave(db->mutex);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_UTF16 */

/*
** Find existing client data.
*/
ಶೂನ್ಯ *sqlite3_get_clientdata(sqlite3 *db, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName){
  Db_Client_ದತ್ತಾಂಶ *p;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !zName || !sqlite3SafetyCheckOk(db) ){
    (ಶೂನ್ಯ)SQLITE_MISUSE_BKPT;
    ಹಿಂತಿರುಗಿ 0;
  }
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_mutex_enter(db->mutex);
  ಗಾಗಿ(p=db->p_Db_ದತ್ತಾಂಶ; p; p=p->p_ಮುಂದಿನ){
    ಆದರೆ( strcmp(p->zName, zName)==0 ){
      ಶೂನ್ಯ *p_ಫಲಿತಾಂಶ = p->p_ದತ್ತಾಂಶ;
      sqlite3_mutex_leave(db->mutex);
      ಹಿಂತಿರುಗಿ p_ಫಲಿತಾಂಶ;
    }
  }
  sqlite3_mutex_leave(db->mutex);
  ಹಿಂತಿರುಗಿ 0;
}

/*
** Add new client data to a database connection.
*/
ಪೂರ್ಣಾಂಕ sqlite3_ಗಣ_clientdata(
  sqlite3 *db,                   /* Attach client data to this connection */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName,             /* Name of the client data */
  ಶೂನ್ಯ *p_ದತ್ತಾಂಶ,                   /* The client data itself */
  ಶೂನ್ಯ (*xDestructor)(ಶೂನ್ಯ*)     /* Destructor */
){
  Db_Client_ದತ್ತಾಂಶ *p, **pp;
  sqlite3_mutex_enter(db->mutex);
  pp = &db->p_Db_ದತ್ತಾಂಶ;
  ಗಾಗಿ(p=db->p_Db_ದತ್ತಾಂಶ; p && strcmp(p->zName,zName); p=p->p_ಮುಂದಿನ){
    pp = &p->p_ಮುಂದಿನ;
  }
  ಆದರೆ( p ){
    ಪ್ರತಿಜ್ಞಾ( p->p_ದತ್ತಾಂಶ!=0 );
    ಆದರೆ( p->xDestructor ) p->xDestructor(p->p_ದತ್ತಾಂಶ);
    ಆದರೆ( p_ದತ್ತಾಂಶ==0 ){
      *pp = p->p_ಮುಂದಿನ;
      sqlite3_free(p);
      sqlite3_mutex_leave(db->mutex);
      ಹಿಂತಿರುಗಿ SQLITE_OK;
    }
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p_ದತ್ತಾಂಶ==0 ){
    sqlite3_mutex_leave(db->mutex);
    ಹಿಂತಿರುಗಿ SQLITE_OK;
  }ಇಲ್ಲದಿದ್ದರೆ{
    size_t n = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(zName);
    p = sqlite3_malloc64( SZ_DBCLIENTDATA(n+1) );
    ಆದರೆ( p==0 ){
      ಆದರೆ( xDestructor ) xDestructor(p_ದತ್ತಾಂಶ);
      sqlite3_mutex_leave(db->mutex);
      ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
    }
    ಸ್ಮರಣೆ_ನಕಲು(p->zName, zName, n+1);
    p->p_ಮುಂದಿನ = db->p_Db_ದತ್ತಾಂಶ;
    db->p_Db_ದತ್ತಾಂಶ = p;
  }
  p->p_ದತ್ತಾಂಶ = p_ದತ್ತಾಂಶ;
  p->xDestructor = xDestructor;
  sqlite3_mutex_leave(db->mutex);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}


#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_DEPRECATED
/*
** This function is now an anachronism. It used to be used to recover from a
** malloc() failure, but SQLite now does this automatically.
*/
ಪೂರ್ಣಾಂಕ sqlite3_global_recover(ಶೂನ್ಯ){
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}
#ಆದರೆ_ಅಂತ್ಯ

/*
** Test to see whether or not the database connection is in autocommit
** mode.  Return TRUE if it is and FALSE if not.  Autocommit mode is on
** by default.  Autocommit is disabled by a BEGIN statement and reenabled
** by the next COMMIT or ROLLBACK.
*/
ಪೂರ್ಣಾಂಕ sqlite3_get_autocommit(sqlite3 *db){
  ಪೂರ್ಣಾಂಕ iRet;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !sqlite3SafetyCheckOk(db) ){
    (ಶೂನ್ಯ)SQLITE_MISUSE_BKPT;
    ಹಿಂತಿರುಗಿ 0;
  }
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_mutex_enter(db->mutex);
  iRet = db->autoCommit;
  sqlite3_mutex_leave(db->mutex);
  ಹಿಂತಿರುಗಿ iRet;
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
ಪೂರ್ಣಾಂಕ sqlite3ReportError(ಪೂರ್ಣಾಂಕ iErr, ಪೂರ್ಣಾಂಕ lineno, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zType){
  sqlite3_log(iErr, "%s at line %d of [%.10s]",
              zType, lineno, 20+sqlite3_sourceid());
  ಹಿಂತಿರುಗಿ iErr;
}
ಪೂರ್ಣಾಂಕ sqlite3CorruptError(ಪೂರ್ಣಾಂಕ lineno){
  testcase( sqlite3GlobalConfig.xLog!=0 );
  ಹಿಂತಿರುಗಿ sqlite3ReportError(SQLITE_CORRUPT, lineno, "database corruption");
}
ಪೂರ್ಣಾಂಕ sqlite3MisuseError(ಪೂರ್ಣಾಂಕ lineno){
  testcase( sqlite3GlobalConfig.xLog!=0 );
  ಹಿಂತಿರುಗಿ sqlite3ReportError(SQLITE_MISUSE, lineno, "misuse");
}
ಪೂರ್ಣಾಂಕ sqlite3CantopenError(ಪೂರ್ಣಾಂಕ lineno){
  testcase( sqlite3GlobalConfig.xLog!=0 );
  ಹಿಂತಿರುಗಿ sqlite3ReportError(SQLITE_CANTOPEN, lineno, "cannot open file");
}
#ಆದರೆ defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_CORRUPT_PGNO)
ಪೂರ್ಣಾಂಕ sqlite3CorruptPgnoError(ಪೂರ್ಣಾಂಕ lineno, Pgno pgno){
  ಅಕ್ಷರ z_ಸಂದೇಶ[100];
  sqlite3_snprintf(ಗಾತ್ರ(z_ಸಂದೇಶ), z_ಸಂದೇಶ, "database corruption page %d", pgno);
  testcase( sqlite3GlobalConfig.xLog!=0 );
  ಹಿಂತಿರುಗಿ sqlite3ReportError(SQLITE_CORRUPT, lineno, z_ಸಂದೇಶ);
}
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
ಪೂರ್ಣಾಂಕ sqlite3NomemError(ಪೂರ್ಣಾಂಕ lineno){
  testcase( sqlite3GlobalConfig.xLog!=0 );
  ಹಿಂತಿರುಗಿ sqlite3ReportError(SQLITE_NOMEM, lineno, "OOM");
}
ಪೂರ್ಣಾಂಕ sqlite3IoerrnomemError(ಪೂರ್ಣಾಂಕ lineno){
  testcase( sqlite3GlobalConfig.xLog!=0 );
  ಹಿಂತಿರುಗಿ sqlite3ReportError(SQLITE_IOERR_NOMEM, lineno, "I/O OOM error");
}
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_DEPRECATED
/*
** This is a convenience routine that makes sure that all thread-specific
** data for this thread has been deallocated.
**
** SQLite no longer uses thread-specific data so this routine is now a
** no-op.  It is retained for historical compatibility.
*/
ಶೂನ್ಯ sqlite3_thread_cleanup(ಶೂನ್ಯ){
}
#ಆದರೆ_ಅಂತ್ಯ

/*
** Return meta information about a specific column of a database table.
** See comment in sqlite3.h (sqlite.h.in) for details.
*/
ಪೂರ್ಣಾಂಕ sqlite3_ಕೋಷ್ಟಕ_ಸ್ತಂಭ_metadata(
  sqlite3 *db,                /* Connection handle */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDbName,        /* Database name or NULL */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಕೋಷ್ಟಕ_Name,     /* Table name */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಸ್ತಂಭ_Name,    /* Column name */
  ಅಕ್ಷರ ಸ್ಥಿರಾಂಕ **pz_ದತ್ತಾಂಶ_Type,    /* OUTPUT: Declared data type */
  ಅಕ್ಷರ ಸ್ಥಿರಾಂಕ **pzCollSeq,     /* OUTPUT: Collation sequence name */
  ಪೂರ್ಣಾಂಕ *pNotNull,              /* OUTPUT: True if NOT NULL constraint exists */
  ಪೂರ್ಣಾಂಕ *p_Primary_ಕೀಲಿ,           /* OUTPUT: True if column part of PK */
  ಪೂರ್ಣಾಂಕ *pAutoinc               /* OUTPUT: True if column is auto-increment */
){
  ಪೂರ್ಣಾಂಕ rc;
  ಅಕ್ಷರ *z_Err_ಸಂದೇಶ = 0;
  ಕೋಷ್ಟಕ *pTab = 0;
  ಸ್ತಂಭ *p_ಲಂಬಸಾಲು = 0;
  ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು = 0;
  ಅಕ್ಷರ ಸ್ಥಿರಾಂಕ *z_ದತ್ತಾಂಶ_Type = 0;
  ಅಕ್ಷರ ಸ್ಥಿರಾಂಕ *zCollSeq = 0;
  ಪೂರ್ಣಾಂಕ notnull = 0;
  ಪೂರ್ಣಾಂಕ primarykey = 0;
  ಪೂರ್ಣಾಂಕ autoinc = 0;


#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !sqlite3SafetyCheckOk(db) || z_ಕೋಷ್ಟಕ_Name==0 ){
    ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
  }
#ಆದರೆ_ಅಂತ್ಯ

  /* Ensure the database schema has been loaded */
  sqlite3_mutex_enter(db->mutex);
  sqlite3BtreeEnterAll(db);
  rc = sqlite3Init(db, &z_Err_ಸಂದೇಶ);
  ಆದರೆ( SQLITE_OK!=rc ){
    ಹೋಗು error_ಹೊರಹೋಗು;
  }

  /* Locate the table in question */
  pTab = sqlite3_ಕಂಡುಹಿಡಿ_ಕೋಷ್ಟಕ(db, z_ಕೋಷ್ಟಕ_Name, zDbName);
  ಆದರೆ( !pTab || IsView(pTab) ){
    pTab = 0;
    ಹೋಗು error_ಹೊರಹೋಗು;
  }

  /* Find the column for which info is requested */
  ಆದರೆ( z_ಸ್ತಂಭ_Name==0 ){
    /* Query for existence of table only */
  }ಇಲ್ಲದಿದ್ದರೆ{
    i_ಲಂಬಸಾಲು = sqlite3_ಸ್ತಂಭ_ಸೂಚ್ಯಂಕ(pTab, z_ಸ್ತಂಭ_Name);
    ಆದರೆ( i_ಲಂಬಸಾಲು>=0 ){
      p_ಲಂಬಸಾಲು = &pTab->a_ಲಂಬಸಾಲು[i_ಲಂಬಸಾಲು];
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಆದರೆ( HasRowid(pTab) && sqlite3IsRowid(z_ಸ್ತಂಭ_Name) ){
        i_ಲಂಬಸಾಲು = pTab->iPKey;
        p_ಲಂಬಸಾಲು = i_ಲಂಬಸಾಲು>=0 ? &pTab->a_ಲಂಬಸಾಲು[i_ಲಂಬಸಾಲು] : 0;
      }ಇಲ್ಲದಿದ್ದರೆ{
        pTab = 0;
        ಹೋಗು error_ಹೊರಹೋಗು;
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
  ಆದರೆ( p_ಲಂಬಸಾಲು ){
    z_ದತ್ತಾಂಶ_Type = sqlite3_ಸ್ತಂಭ_Type(p_ಲಂಬಸಾಲು,0);
    zCollSeq = sqlite3_ಸ್ತಂಭ_Coll(p_ಲಂಬಸಾಲು);
    notnull = p_ಲಂಬಸಾಲು->notNull!=0;
    primarykey  = (p_ಲಂಬಸಾಲು->ಲಂಬಸಾಲು_Flags & COLFLAG_PRIMKEY)!=0;
    autoinc = pTab->iPKey==i_ಲಂಬಸಾಲು && (pTab->tabFlags & TF_Autoincrement)!=0;
  }ಇಲ್ಲದಿದ್ದರೆ{
    z_ದತ್ತಾಂಶ_Type = "INTEGER";
    primarykey = 1;
  }
  ಆದರೆ( !zCollSeq ){
    zCollSeq = sqlite3_Str_ದ್ವಿಮಾನ;
  }

error_ಹೊರಹೋಗು:
  sqlite3BtreeLeaveAll(db);

  /* Whether the function call succeeded or failed, set the output parameters
  ** to whatever their local counterparts contain. If an error did occur,
  ** this has the effect of zeroing all output parameters.
  */
  ಆದರೆ( pz_ದತ್ತಾಂಶ_Type ) *pz_ದತ್ತಾಂಶ_Type = z_ದತ್ತಾಂಶ_Type;
  ಆದರೆ( pzCollSeq ) *pzCollSeq = zCollSeq;
  ಆದರೆ( pNotNull ) *pNotNull = notnull;
  ಆದರೆ( p_Primary_ಕೀಲಿ ) *p_Primary_ಕೀಲಿ = primarykey;
  ಆದರೆ( pAutoinc ) *pAutoinc = autoinc;

  ಆದರೆ( SQLITE_OK==rc && !pTab ){
    sqlite3DbFree(db, z_Err_ಸಂದೇಶ);
    z_Err_ಸಂದೇಶ = sqlite3MPrintf(db, "no such table column: %s.%s", z_ಕೋಷ್ಟಕ_Name,
        z_ಸ್ತಂಭ_Name);
    rc = SQLITE_ERROR;
  }
  sqlite3_Error_With_ಸಂದೇಶ(db, rc, (z_Err_ಸಂದೇಶ?"%s":0), z_Err_ಸಂದೇಶ);
  sqlite3DbFree(db, z_Err_ಸಂದೇಶ);
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Sleep for a little while.  Return the amount of time slept.
*/
ಪೂರ್ಣಾಂಕ sqlite3_sleep(ಪೂರ್ಣಾಂಕ ms){
  sqlite3_vfs *pVfs;
  ಪೂರ್ಣಾಂಕ rc;
  pVfs = sqlite3_vfs_ಕಂಡುಹಿಡಿ(0);
  ಆದರೆ( pVfs==0 ) ಹಿಂತಿರುಗಿ 0;

  /* This function works in milliseconds, but the underlying OsSleep()
  ** API uses microseconds. Hence the 1000's.
  */
  rc = (sqlite3OsSleep(pVfs, ms<0 ? 0 : 1000*ms)/1000);
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Enable or disable the extended result codes.
*/
ಪೂರ್ಣಾಂಕ sqlite3_extended_ಫಲಿತಾಂಶ_codes(sqlite3 *db, ಪೂರ್ಣಾಂಕ onoff){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !sqlite3SafetyCheckOk(db) ) ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_mutex_enter(db->mutex);
  db->err_ಮುಖವಾಡ = onoff ? 0xffffffff : 0xff;
  sqlite3_mutex_leave(db->mutex);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Invoke the xFileControl method on a particular database.
*/
ಪೂರ್ಣಾಂಕ sqlite3_file_control(sqlite3 *db, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDbName, ಪೂರ್ಣಾಂಕ op, ಶೂನ್ಯ *pArg){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_ERROR;
  Btree *pBtree;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !sqlite3SafetyCheckOk(db) ) ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_mutex_enter(db->mutex);
  pBtree = sqlite3_Db_Name_ಗಮ್ಯ_Btree(db, zDbName);
  ಆದರೆ( pBtree ){
    Pager *pPager;
    sqlite3_file *fd;
    sqlite3BtreeEnter(pBtree);
    pPager = sqlite3BtreePager(pBtree);
    ಪ್ರತಿಜ್ಞಾ( pPager!=0 );
    fd = sqlite3PagerFile(pPager);
    ಪ್ರತಿಜ್ಞಾ( fd!=0 );
    ಆದರೆ( op==SQLITE_FCNTL_FILE_ಸೂಚಕ ){
      *(sqlite3_file**)pArg = fd;
      rc = SQLITE_OK;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( op==SQLITE_FCNTL_VFS_ಸೂಚಕ ){
      *(sqlite3_vfs**)pArg = sqlite3PagerVfs(pPager);
      rc = SQLITE_OK;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( op==SQLITE_FCNTL_JOURNAL_ಸೂಚಕ ){
      *(sqlite3_file**)pArg = sqlite3PagerJrnlFile(pPager);
      rc = SQLITE_OK;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( op==SQLITE_FCNTL_ದತ್ತಾಂಶ_ಆವೃತ್ತಿ ){
      *(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ*)pArg = sqlite3_Pager_ದತ್ತಾಂಶ_ಆವೃತ್ತಿ(pPager);
      rc = SQLITE_OK;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( op==SQLITE_FCNTL_RESERVE_ಬೈಟುಗಳು ){
      ಪೂರ್ಣಾಂಕ iNew = *(ಪೂರ್ಣಾಂಕ*)pArg;
      *(ಪೂರ್ಣಾಂಕ*)pArg = sqlite3BtreeGetRequestedReserve(pBtree);
      ಆದರೆ( iNew>=0 && iNew<=255 ){
        sqlite3_Btree_ಗಣ_ಪುಟ_ಗಾತ್ರ(pBtree, 0, iNew, 0);
      }
      rc = SQLITE_OK;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( op==SQLITE_FCNTL_RESET_CACHE ){
      sqlite3BtreeClearCache(pBtree);
      rc = SQLITE_OK;
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಪೂರ್ಣಾಂಕ nSave = db->busyHandler.nBusy;
      rc = sqlite3OsFileControl(fd, op, pArg);
      db->busyHandler.nBusy = nSave;
    }
    sqlite3BtreeLeave(pBtree);
  }
  sqlite3_mutex_leave(db->mutex);
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Interface to the testing logic.
*/
ಪೂರ್ಣಾಂಕ sqlite3_test_control(ಪೂರ್ಣಾಂಕ op, ...){
  ಪೂರ್ಣಾಂಕ rc = 0;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_UNTESTABLE
  ಬಳಸದ_PARAMETER(op);
#ಇಲ್ಲದಿದ್ದರೆ
  va_list ap;
  va_start(ap, op);
  ಆಯ್ಕೆ( op ){

    /*
    ** Save the current state of the PRNG.
    */
    ಸ್ಥಿತಿ SQLITE_TESTCTRL_PRNG_SAVE: {
      sqlite3PrngSaveState();
      ನಿಲ್ಲಿಸು;
    }

    /*
    ** Restore the state of the PRNG to the last state saved using
    ** PRNG_SAVE.  If PRNG_SAVE has never before been called, then
    ** this verb acts like PRNG_RESET.
    */
    ಸ್ಥಿತಿ SQLITE_TESTCTRL_PRNG_RESTORE: {
      sqlite3PrngRestoreState();
      ನಿಲ್ಲಿಸು;
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
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WSD
    ಸ್ಥಿತಿ SQLITE_TESTCTRL_PRNG_ಬೀಜ: {
      ಪೂರ್ಣಾಂಕ x = va_arg(ap, ಪೂರ್ಣಾಂಕ);
      ಪೂರ್ಣಾಂಕ y;
      sqlite3 *db = va_arg(ap, sqlite3*);
      ಪ್ರತಿಜ್ಞಾ( db==0 || db->aDb[0].pSchema!=0 );
      ಆದರೆ( db && (y = db->aDb[0].pSchema->schema_cookie)!=0 ){ x = y; }
      sqlite3Config.i_Prng_ಬೀಜ = x;
      sqlite3_randomness(0,0);
      ನಿಲ್ಲಿಸು;
    }
#ಆದರೆ_ಅಂತ್ಯ

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
    ಸ್ಥಿತಿ SQLITE_TESTCTRL_FK_NO_ACTION: {
      sqlite3 *db = va_arg(ap, sqlite3*);
      ಪೂರ್ಣಾಂಕ b = va_arg(ap, ಪೂರ್ಣಾಂಕ);
      ಆದರೆ( b ){
        db->flags |= SQLITE_FkNoAction;
      }ಇಲ್ಲದಿದ್ದರೆ{
        db->flags &= ~SQLITE_FkNoAction;
      }
      ನಿಲ್ಲಿಸು;
    }

    /*
    **  sqlite3_test_control(BITVEC_TEST, size, program)
    **
    ** Run a test against a Bitvec object of size.  The program argument
    ** is an array of integers that defines the test.  Return -1 on a
    ** memory allocation error, 0 on success, or non-zero for an error.
    ** See the sqlite3BitvecBuiltinTest() for additional information.
    */
    ಸ್ಥಿತಿ SQLITE_TESTCTRL_BITVEC_TEST: {
      ಪೂರ್ಣಾಂಕ sz = va_arg(ap, ಪೂರ್ಣಾಂಕ);
      ಪೂರ್ಣಾಂಕ *aProg = va_arg(ap, ಪೂರ್ಣಾಂಕ*);
      rc = sqlite3BitvecBuiltinTest(sz, aProg);
      ನಿಲ್ಲಿಸು;
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
    ಸ್ಥಿತಿ SQLITE_TESTCTRL_FAULT_INSTALL: {
      /* A bug in MSVC prevents it from understanding pointers to functions
      ** types in the second argument to va_arg().  Work around the problem
      ** using a typedef.
      ** http://support.microsoft.com/kb/47961  <-- dead hyperlink
      ** Search at http://web.archive.org/ to find the 2015-03-16 archive
      ** of the link above to see the original text.
      ** sqlite3GlobalConfig.xTestCallback = va_arg(ap, int(*)(int));
      */
      ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಪೂರ್ಣಾಂಕ(*sqlite3FaultFuncType)(ಪೂರ್ಣಾಂಕ);
      sqlite3GlobalConfig.xTestCallback = va_arg(ap, sqlite3FaultFuncType);
      rc = sqlite3FaultSim(0);
      ನಿಲ್ಲಿಸು;
    }

    /*
    **  sqlite3_test_control(BENIGN_MALLOC_HOOKS, xBegin, xEnd)
    **
    ** Register hooks to call to indicate which malloc() failures
    ** are benign.
    */
    ಸ್ಥಿತಿ SQLITE_TESTCTRL_BENIGN_MALLOC_HOOKS: {
      ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಶೂನ್ಯ (*void_function)(ಶೂನ್ಯ);
      void_function xBenignBegin;
      void_function x_Benign_ಕೊನೆ;
      xBenignBegin = va_arg(ap, void_function);
      x_Benign_ಕೊನೆ = va_arg(ap, void_function);
      sqlite3BenignMallocHooks(xBenignBegin, x_Benign_ಕೊನೆ);
      ನಿಲ್ಲಿಸು;
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
    ಸ್ಥಿತಿ SQLITE_TESTCTRL_PENDING_BYTE: {
      rc = PENDING_BYTE;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WSD
      {
        ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ newVal = va_arg(ap, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ);
        ಆದರೆ( newVal ) sqlite3PendingByte = newVal;
      }
#ಆದರೆ_ಅಂತ್ಯ
      ನಿಲ್ಲಿಸು;
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
    ಸ್ಥಿತಿ SQLITE_TESTCTRL_ASSERT: {
      ಅಸ್ಥಿರ ಪೂರ್ಣಾಂಕ x = 0;
      ಪ್ರತಿಜ್ಞಾ( /*side-effects-ok*/ (x = va_arg(ap,ಪೂರ್ಣಾಂಕ))!=0 );
      rc = x;
#ಆದರೆ defined(SQLITE_DEBUG)
      /* Invoke these debugging routines so that the compiler does not
      ** issue "defined but not used" warnings. */
      ಆದರೆ( x==9999 ){
        sqlite3_Show_ಅಭಿವ್ಯಕ್ತಿ(0);
        sqlite3_Show_ಅಭಿವ್ಯಕ್ತಿ_List(0);
        sqlite3ShowIdList(0);
        sqlite3ShowSrcList(0);
        sqlite3ShowWith(0);
        sqlite3ShowUpsert(0);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_TRIGGER
        sqlite3_Show_Trigger_ಹೆಜ್ಜೆ(0);
        sqlite3_Show_Trigger_ಹೆಜ್ಜೆ_List(0);
        sqlite3ShowTrigger(0);
        sqlite3ShowTriggerList(0);
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
        sqlite3_Show_ಕಿಟಕಿ(0);
        sqlite3ShowWinFunc(0);
#ಆದರೆ_ಅಂತ್ಯ
        sqlite3ShowSelect(0);
      }
#ಆದರೆ_ಅಂತ್ಯ
      ನಿಲ್ಲಿಸು;
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
    ಸ್ಥಿತಿ SQLITE_TESTCTRL_ALWAYS: {
      ಪೂರ್ಣಾಂಕ x = va_arg(ap,ಪೂರ್ಣಾಂಕ);
      rc = x ? ALWAYS(x) : 0;
      ನಿಲ್ಲಿಸು;
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
    ಸ್ಥಿತಿ SQLITE_TESTCTRL_BYTEORDER: {
      rc = SQLITE_BYTEORDER*100 + SQLITE_LITTLEENDIAN*10 + SQLITE_BIGENDIAN;
      ನಿಲ್ಲಿಸು;
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
    ಸ್ಥಿತಿ SQLITE_TESTCTRL_OPTIMIZATIONS: {
      sqlite3 *db = va_arg(ap, sqlite3*);
      db->dbOptFlags = va_arg(ap, u32);
      ನಿಲ್ಲಿಸು;
    }

    /*  sqlite3_test_control(SQLITE_TESTCTRL_GETOPT, sqlite3 *db, int *N)
    **
    ** Write the current optimization settings into *N.  A zero bit means that
    ** the optimization is on, and a 1 bit means that the optimization is off.
    */
    ಸ್ಥಿತಿ SQLITE_TESTCTRL_GETOPT: {
      sqlite3 *db = va_arg(ap, sqlite3*);
      ಪೂರ್ಣಾಂಕ *pN = va_arg(ap, ಪೂರ್ಣಾಂಕ*);
      *pN = db->dbOptFlags;
      ನಿಲ್ಲಿಸು;
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
    ಸ್ಥಿತಿ SQLITE_TESTCTRL_LOCALTIME_FAULT: {
      sqlite3GlobalConfig.bLocaltimeFault = va_arg(ap, ಪೂರ್ಣಾಂಕ);
      ಆದರೆ( sqlite3GlobalConfig.bLocaltimeFault==2 ){
        ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಪೂರ್ಣಾಂಕ(*sqlite3LocaltimeType)(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*,ಶೂನ್ಯ*);
        sqlite3GlobalConfig.xAltLocaltime = va_arg(ap, sqlite3LocaltimeType);
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3GlobalConfig.xAltLocaltime = 0;
      }
      ನಿಲ್ಲಿಸು;
    }

    /*   sqlite3_test_control(SQLITE_TESTCTRL_INTERNAL_FUNCTIONS, sqlite3*);
    **
    ** Toggle the ability to use internal functions on or off for
    ** the database connection given in the argument.
    */
    ಸ್ಥಿತಿ SQLITE_TESTCTRL_ಆಂತರಿಕ_FUNCTIONS: {
      sqlite3 *db = va_arg(ap, sqlite3*);
      db->mDbFlags ^= DBFLAG_ಆಂತರಿಕ_Func;
      ನಿಲ್ಲಿಸು;
    }

    /*   sqlite3_test_control(SQLITE_TESTCTRL_NEVER_CORRUPT, int);
    **
    ** Set or clear a flag that indicates that the database file is always well-
    ** formed and never corrupt.  This flag is clear by default, indicating that
    ** database files might have arbitrary corruption.  Setting the flag during
    ** testing causes certain assert() statements in the code to be activated
    ** that demonstrate invariants on well-formed database files.
    */
    ಸ್ಥಿತಿ SQLITE_TESTCTRL_NEVER_CORRUPT: {
      sqlite3GlobalConfig.neverCorrupt = va_arg(ap, ಪೂರ್ಣಾಂಕ);
      ನಿಲ್ಲಿಸು;
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
    ಸ್ಥಿತಿ SQLITE_TESTCTRL_EXTRA_SCHEMA_CHECKS: {
      sqlite3GlobalConfig.bExtraSchemaChecks = va_arg(ap, ಪೂರ್ಣಾಂಕ);
      ನಿಲ್ಲಿಸು;
    }

    /* Set the threshold at which OP_Once counters reset back to zero.
    ** By default this is 0x7ffffffe (over 2 billion), but that value is
    ** too big to test in a reasonable amount of time, so this control is
    ** provided to set a small and easily reachable reset value.
    */
    ಸ್ಥಿತಿ SQLITE_TESTCTRL_ONCE_RESET_ಮಿತಿ: {
      sqlite3GlobalConfig.i_Once_Reset_ಮಿತಿ = va_arg(ap, ಪೂರ್ಣಾಂಕ);
      ನಿಲ್ಲಿಸು;
    }

    /*   sqlite3_test_control(SQLITE_TESTCTRL_VDBE_COVERAGE, xCallback, ptr);
    **
    ** Set the VDBE coverage callback function to xCallback with context
    ** pointer ptr.
    */
    ಸ್ಥಿತಿ SQLITE_TESTCTRL_VDBE_COVERAGE: {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_VDBE_COVERAGE
      ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಶೂನ್ಯ (*branch_callback)(ಶೂನ್ಯ*,ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ,
                                      ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ,ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ);
      sqlite3GlobalConfig.xVdbeBranch = va_arg(ap,branch_callback);
      sqlite3GlobalConfig.pVdbeBranchArg = va_arg(ap,ಶೂನ್ಯ*);
#ಆದರೆ_ಅಂತ್ಯ
      ನಿಲ್ಲಿಸು;
    }

    /*   sqlite3_test_control(SQLITE_TESTCTRL_SORTER_MMAP, db, nMax); */
    ಸ್ಥಿತಿ SQLITE_TESTCTRL_SORTER_MMAP: {
      sqlite3 *db = va_arg(ap, sqlite3*);
      db->nMaxSorterMmap = va_arg(ap, ಪೂರ್ಣಾಂಕ);
      ನಿಲ್ಲಿಸು;
    }

    /*   sqlite3_test_control(SQLITE_TESTCTRL_ISINIT);
    **
    ** Return SQLITE_OK if SQLite has been initialized and SQLITE_ERROR if
    ** not.
    */
    ಸ್ಥಿತಿ SQLITE_TESTCTRL_ISINIT: {
      ಆದರೆ( sqlite3GlobalConfig.isInit==0 ) rc = SQLITE_ERROR;
      ನಿಲ್ಲಿಸು;
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
    ಸ್ಥಿತಿ SQLITE_TESTCTRL_IMPOSTER: {
      sqlite3 *db = va_arg(ap, sqlite3*);
      ಪೂರ್ಣಾಂಕ iDb;
      sqlite3_mutex_enter(db->mutex);
      iDb = sqlite3_ಕಂಡುಹಿಡಿ_Db_Name(db, va_arg(ap,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*));
      ಆದರೆ( iDb>=0 ){
        db->init.iDb = iDb;
        db->init.busy = db->init.imposter_ಕೋಷ್ಟಕ = va_arg(ap,ಪೂರ್ಣಾಂಕ);
        db->init.newTnum = va_arg(ap,ಪೂರ್ಣಾಂಕ);
        ಆದರೆ( db->init.busy==0 && db->init.newTnum>0 ){
          sqlite3_Reset_All_Schemas_ದ_Connection(db);
        }
      }
      sqlite3_mutex_leave(db->mutex);
      ನಿಲ್ಲಿಸು;
    }

#ಆದರೆ defined(YYCOVERAGE)
    /*  sqlite3_test_control(SQLITE_TESTCTRL_PARSER_COVERAGE, FILE *out)
    **
    ** This test control (only available when SQLite is compiled with
    ** -DYYCOVERAGE) writes a report onto "out" that shows all
    ** state/lookahead combinations in the parser state machine
    ** which are never exercised.  If any state is missed, make the
    ** return code SQLITE_ERROR.
    */
    ಸ್ಥಿತಿ SQLITE_TESTCTRL_PARSER_COVERAGE: {
      FILE *ಹೊರಹೋಗು = va_arg(ap, FILE*);
      ಆದರೆ( sqlite3ParserCoverage(ಹೊರಹೋಗು) ) rc = SQLITE_ERROR;
      ನಿಲ್ಲಿಸು;
    }
#ಆದರೆ_ಅಂತ್ಯ /* defined(YYCOVERAGE) */

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
    ಸ್ಥಿತಿ SQLITE_TESTCTRL_ಫಲಿತಾಂಶ_INTREAL: {
      sqlite3_context *pCtx = va_arg(ap, sqlite3_context*);
      sqlite3_ಫಲಿತಾಂಶ_Int_Real(pCtx);
      ನಿಲ್ಲಿಸು;
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
    ಸ್ಥಿತಿ SQLITE_TESTCTRL_SEEK_ಎಣಿಕೆ: {
      sqlite3 *db = va_arg(ap, sqlite3*);
      u64 *pn = va_arg(ap, sqlite3_uint64*);
      *pn = sqlite3_Btree_Seek_ಎಣಿಕೆ(db->aDb->pBt);
      (ಶೂನ್ಯ)db;  /* Silence harmless unused variable warning */
      ನಿಲ್ಲಿಸು;
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
    ಸ್ಥಿತಿ SQLITE_TESTCTRL_TRACEFLAGS: {
       ಪೂರ್ಣಾಂಕ opTrace = va_arg(ap, ಪೂರ್ಣಾಂಕ);
       u32 *ptr = va_arg(ap, u32*);
       ಆಯ್ಕೆ( opTrace ){
         ಸ್ಥಿತಿ 0:   *ptr = sqlite3_ವೃಕ್ಷ_Trace;      ನಿಲ್ಲಿಸು;
         ಸ್ಥಿತಿ 1:   sqlite3_ವೃಕ್ಷ_Trace = *ptr;      ನಿಲ್ಲಿಸು;
         ಸ್ಥಿತಿ 2:   *ptr = sqlite3WhereTrace;     ನಿಲ್ಲಿಸು;
         ಸ್ಥಿತಿ 3:   sqlite3WhereTrace = *ptr;     ನಿಲ್ಲಿಸು;
       }
       ನಿಲ್ಲಿಸು;
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
    ಸ್ಥಿತಿ SQLITE_TESTCTRL_LOGEST: {
      ದ್ವಿಗುಣ r_ಒಳ = va_arg(ap, ದ್ವಿಗುಣ);
      LogEst rLogEst = sqlite3LogEstFromDouble(r_ಒಳ);
      ಪೂರ್ಣಾಂಕ *pI1 = va_arg(ap,ಪೂರ್ಣಾಂಕ*);
      u64 *pU64 = va_arg(ap,u64*);
      ಪೂರ್ಣಾಂಕ *pI2 = va_arg(ap,ಪೂರ್ಣಾಂಕ*);
      *pI1 = rLogEst;
      *pU64 = sqlite3_Log_Est_ಗಮ್ಯ_Int(rLogEst);
      *pI2 = sqlite3LogEst(*pU64);
      ನಿಲ್ಲಿಸು;
    }

    /* sqlite3_test_control(SQLITE_TESTCTRL_ATOF, const char *z, double *p);
    **
    ** Test access to the sqlite3AtoF() routine.
    */
    ಸ್ಥಿತಿ SQLITE_TESTCTRL_ATOF: {
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z = va_arg(ap,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
      ದ್ವಿಗುಣ *pR = va_arg(ap,ದ್ವಿಗುಣ*);
      rc = sqlite3AtoF(z,pR);
      ನಿಲ್ಲಿಸು;
    }

#ಆದರೆ defined(SQLITE_DEBUG) && !defined(SQLITE_OMIT_WSD)
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
    ಸ್ಥಿತಿ SQLITE_TESTCTRL_TUNE: {
      ಪೂರ್ಣಾಂಕ id = va_arg(ap, ಪೂರ್ಣಾಂಕ);
      ಪೂರ್ಣಾಂಕ *pi_ಮೌಲ್ಯ = va_arg(ap, ಪೂರ್ಣಾಂಕ*);
      ಆದರೆ( id>0 && id<=SQLITE_NTUNE ){
        Tuning(id) = *pi_ಮೌಲ್ಯ;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( id<0 && id>=-SQLITE_NTUNE ){
        *pi_ಮೌಲ್ಯ = Tuning(-id);
      }ಇಲ್ಲದಿದ್ದರೆ{
        rc = SQLITE_NOTFOUND;
      }
      ನಿಲ್ಲಿಸು;
    }
#ಆದರೆ_ಅಂತ್ಯ

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
    ಸ್ಥಿತಿ SQLITE_TESTCTRL_JSON_SELFCHECK: {
#ಆದರೆ defined(SQLITE_DEBUG) && !defined(SQLITE_OMIT_WSD)
      ಪೂರ್ಣಾಂಕ *pOnOff = va_arg(ap, ಪೂರ್ಣಾಂಕ*);
      ಆದರೆ( *pOnOff<0 ){
        *pOnOff = sqlite3Config.bJsonSelfcheck;
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3Config.bJsonSelfcheck = (u8)((*pOnOff)&0xff);
      }
#ಆದರೆ_ಅಂತ್ಯ
      ನಿಲ್ಲಿಸು;
    }
  }
  va_end(ap);
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_UNTESTABLE */
  ಹಿಂತಿರುಗಿ rc;
}

/*
** The Pager stores the Database filename, Journal filename, and WAL filename
** consecutively in memory, in that order.  The database filename is prefixed
** by four zero bytes.  Locate the start of the database filename by searching
** backwards for the first byte following four consecutive zero bytes.
**
** This only works if the filename passed in was obtained from the Pager.
*/
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *databaseName(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName){
  ಆಗ( zName[-1]!=0 || zName[-2]!=0 || zName[-3]!=0 || zName[-4]!=0 ){
    zName--;
  }
  ಹಿಂತಿರುಗಿ zName;
}

/*
** Append text z[] to the end of p[].  Return a pointer to the first
** character after then zero terminator on the new text in p[].
*/
ಸ್ಥಿರ ಅಕ್ಷರ *append_ಪಠ್ಯ(ಅಕ್ಷರ *p, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z){
  size_t n = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(z);
  ಸ್ಮರಣೆ_ನಕಲು(p, z, n+1);
  ಹಿಂತಿರುಗಿ p+n+1;
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
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3_create_filename(
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDatabase,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zJournal,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zWal,
  ಪೂರ್ಣಾಂಕ nParam,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **azParam
){
  sqlite3_int64 nByte;
  ಪೂರ್ಣಾಂಕ i;
  ಅಕ್ಷರ *p_ಫಲಿತಾಂಶ, *p;
  nByte = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(zDatabase) + ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(zJournal) + ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(zWal) + 10;
  ಗಾಗಿ(i=0; i<nParam*2; i++){
    nByte += ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(azParam[i])+1;
  }
  p_ಫಲಿತಾಂಶ = p = sqlite3_malloc64( nByte );
  ಆದರೆ( p==0 ) ಹಿಂತಿರುಗಿ 0;
  memset(p, 0, 4);
  p += 4;
  p = append_ಪಠ್ಯ(p, zDatabase);
  ಗಾಗಿ(i=0; i<nParam*2; i++){
    p = append_ಪಠ್ಯ(p, azParam[i]);
  }
  *(p++) = 0;
  p = append_ಪಠ್ಯ(p, zJournal);
  p = append_ಪಠ್ಯ(p, zWal);
  *(p++) = 0;
  *(p++) = 0;
  ಪ್ರತಿಜ್ಞಾ( (sqlite3_int64)(p - p_ಫಲಿತಾಂಶ)==nByte );
  ಹಿಂತಿರುಗಿ p_ಫಲಿತಾಂಶ + 4;
}

/*
** Free memory obtained from sqlite3_create_filename().  It is a severe
** error to call this routine with any parameter other than a pointer
** previously obtained from sqlite3_create_filename() or a NULL pointer.
*/
ಶೂನ್ಯ sqlite3_free_filename(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p){
  ಆದರೆ( p==0 ) ಹಿಂತಿರುಗಿ;
  p = databaseName(p);
  sqlite3_free((ಅಕ್ಷರ*)p - 4);
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
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3_uri_parameter(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFilename, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zParam){
  ಆದರೆ( zFilename==0 || zParam==0 ) ಹಿಂತಿರುಗಿ 0;
  zFilename = databaseName(zFilename);
  ಹಿಂತಿರುಗಿ uriParameter(zFilename, zParam);
}

/*
** Return a pointer to the name of Nth query parameter of the filename.
*/
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3_uri_ಕೀಲಿ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFilename, ಪೂರ್ಣಾಂಕ N){
  ಆದರೆ( zFilename==0 || N<0 ) ಹಿಂತಿರುಗಿ 0;
  zFilename = databaseName(zFilename);
  zFilename += sqlite3Strlen30(zFilename) + 1;
  ಆಗ( ALWAYS(zFilename) && zFilename[0] && (N--)>0 ){
    zFilename += sqlite3Strlen30(zFilename) + 1;
    zFilename += sqlite3Strlen30(zFilename) + 1;
  }
  ಹಿಂತಿರುಗಿ zFilename[0] ? zFilename : 0;
}

/*
** Return a boolean value for a query parameter.
*/
ಪೂರ್ಣಾಂಕ sqlite3_uri_boolean(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFilename, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zParam, ಪೂರ್ಣಾಂಕ bDflt){
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z = sqlite3_uri_parameter(zFilename, zParam);
  bDflt = bDflt!=0;
  ಹಿಂತಿರುಗಿ z ? sqlite3GetBoolean(z, bDflt) : bDflt;
}

/*
** Return a 64-bit integer value for a query parameter.
*/
sqlite3_int64 sqlite3_uri_int64(
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFilename,    /* Filename as passed to xOpen */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zParam,       /* URI parameter sought */
  sqlite3_int64 bDflt       /* return if parameter is missing */
){
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z = sqlite3_uri_parameter(zFilename, zParam);
  sqlite3_int64 v;
  ಆದರೆ( z && sqlite3_Dec_Or_Hex_ಗಮ್ಯ_I64(z, &v)==0 ){
    bDflt = v;
  }
  ಹಿಂತಿರುಗಿ bDflt;
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
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3_filename_database(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFilename){
  ಆದರೆ( zFilename==0 ) ಹಿಂತಿರುಗಿ 0;
  ಹಿಂತಿರುಗಿ databaseName(zFilename);
}
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3_filename_journal(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFilename){
  ಆದರೆ( zFilename==0 ) ಹಿಂತಿರುಗಿ 0;
  zFilename = databaseName(zFilename);
  zFilename += sqlite3Strlen30(zFilename) + 1;
  ಆಗ( ALWAYS(zFilename) && zFilename[0] ){
    zFilename += sqlite3Strlen30(zFilename) + 1;
    zFilename += sqlite3Strlen30(zFilename) + 1;
  }
  ಹಿಂತಿರುಗಿ zFilename + 1;
}
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3_filename_wal(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFilename){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_OMIT_WAL
  ಬಳಸದ_PARAMETER(zFilename);
  ಹಿಂತಿರುಗಿ 0;
#ಇಲ್ಲದಿದ್ದರೆ
  zFilename = sqlite3_filename_journal(zFilename);
  ಆದರೆ( zFilename ) zFilename += sqlite3Strlen30(zFilename) + 1;
  ಹಿಂತಿರುಗಿ zFilename;
#ಆದರೆ_ಅಂತ್ಯ
}

/*
** Return the Btree pointer identified by zDbName.  Return NULL if not found.
*/
Btree *sqlite3_Db_Name_ಗಮ್ಯ_Btree(sqlite3 *db, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDbName){
  ಪೂರ್ಣಾಂಕ iDb = zDbName ? sqlite3_ಕಂಡುಹಿಡಿ_Db_Name(db, zDbName) : 0;
  ಹಿಂತಿರುಗಿ iDb<0 ? 0 : db->aDb[iDb].pBt;
}

/*
** Return the name of the N-th database schema.  Return NULL if N is out
** of range.
*/
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3_db_name(sqlite3 *db, ಪೂರ್ಣಾಂಕ N){
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zRet = 0;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !sqlite3SafetyCheckOk(db) ){
    (ಶೂನ್ಯ)SQLITE_MISUSE_BKPT;
    ಹಿಂತಿರುಗಿ 0;
  }
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_mutex_enter(db->mutex);
  ಆದರೆ( N>=0 && N<db->nDb ){
    zRet = db->aDb[N].zDbSName;
  }
  sqlite3_mutex_leave(db->mutex);
  ಹಿಂತಿರುಗಿ zRet;
}

/*
** Return the filename of the database associated with a database
** connection.
*/
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3_db_filename(sqlite3 *db, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDbName){
  Btree *pBt;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !sqlite3SafetyCheckOk(db) ){
    (ಶೂನ್ಯ)SQLITE_MISUSE_BKPT;
    ಹಿಂತಿರುಗಿ 0;
  }
#ಆದರೆ_ಅಂತ್ಯ
  pBt = sqlite3_Db_Name_ಗಮ್ಯ_Btree(db, zDbName);
  ಹಿಂತಿರುಗಿ pBt ? sqlite3BtreeGetFilename(pBt) : 0;
}

/*
** Return 1 if database is read-only or 0 if read/write.  Return -1 if
** no such database exists.
*/
ಪೂರ್ಣಾಂಕ sqlite3_db_readonly(sqlite3 *db, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDbName){
  Btree *pBt;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !sqlite3SafetyCheckOk(db) ){
    (ಶೂನ್ಯ)SQLITE_MISUSE_BKPT;
    ಹಿಂತಿರುಗಿ -1;
  }
#ಆದರೆ_ಅಂತ್ಯ
  pBt = sqlite3_Db_Name_ಗಮ್ಯ_Btree(db, zDbName);
  ಹಿಂತಿರುಗಿ pBt ? sqlite3BtreeIsReadonly(pBt) : -1;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_SNAPSHOT
/*
** Obtain a snapshot handle for the snapshot of database zDb currently
** being read by handle db.
*/
ಪೂರ್ಣಾಂಕ sqlite3_snapshot_get(
  sqlite3 *db,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDb,
  sqlite3_snapshot **ppSnapshot
){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_ERROR;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WAL

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !sqlite3SafetyCheckOk(db) ){
    ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
  }
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_mutex_enter(db->mutex);

  ಆದರೆ( db->autoCommit==0 ){
    ಪೂರ್ಣಾಂಕ iDb = sqlite3_ಕಂಡುಹಿಡಿ_Db_Name(db, zDb);
    ಆದರೆ( iDb==0 || iDb>1 ){
      Btree *pBt = db->aDb[iDb].pBt;
      ಆದರೆ( SQLITE_TXN_WRITE!=sqlite3BtreeTxnState(pBt) ){
        Pager *pPager = sqlite3BtreePager(pBt);
        i64 dummy = 0;
        sqlite3PagerSnapshotOpen(pPager, (sqlite3_snapshot*)&dummy);
        rc = sqlite3BtreeBeginTrans(pBt, 0, 0);
        sqlite3PagerSnapshotOpen(pPager, 0);
        ಆದರೆ( rc==SQLITE_OK ){
          rc = sqlite3PagerSnapshotGet(sqlite3BtreePager(pBt), ppSnapshot);
        }
      }
    }
  }

  sqlite3_mutex_leave(db->mutex);
#ಆದರೆ_ಅಂತ್ಯ   /* SQLITE_OMIT_WAL */
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Open a read-transaction on the snapshot identified by pSnapshot.
*/
ಪೂರ್ಣಾಂಕ sqlite3_snapshot_open(
  sqlite3 *db,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDb,
  sqlite3_snapshot *pSnapshot
){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_ERROR;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WAL

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !sqlite3SafetyCheckOk(db) ){
    ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
  }
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_mutex_enter(db->mutex);
  ಆದರೆ( db->autoCommit==0 ){
    ಪೂರ್ಣಾಂಕ iDb;
    iDb = sqlite3_ಕಂಡುಹಿಡಿ_Db_Name(db, zDb);
    ಆದರೆ( iDb==0 || iDb>1 ){
      Btree *pBt = db->aDb[iDb].pBt;
      ಆದರೆ( sqlite3BtreeTxnState(pBt)!=SQLITE_TXN_WRITE ){
        Pager *pPager = sqlite3BtreePager(pBt);
        ಪೂರ್ಣಾಂಕ bUnlock = 0;
        ಆದರೆ( sqlite3BtreeTxnState(pBt)!=SQLITE_TXN_NONE ){
          ಆದರೆ( db->n_Vdbe_ಸಕ್ರಿಯ==0 ){
            rc = sqlite3PagerSnapshotCheck(pPager, pSnapshot);
            ಆದರೆ( rc==SQLITE_OK ){
              bUnlock = 1;
              rc = sqlite3BtreeCommit(pBt);
            }
          }
        }ಇಲ್ಲದಿದ್ದರೆ{
          rc = SQLITE_OK;
        }
        ಆದರೆ( rc==SQLITE_OK ){
          rc = sqlite3PagerSnapshotOpen(pPager, pSnapshot);
        }
        ಆದರೆ( rc==SQLITE_OK ){
          rc = sqlite3BtreeBeginTrans(pBt, 0, 0);
          sqlite3PagerSnapshotOpen(pPager, 0);
        }
        ಆದರೆ( bUnlock ){
          sqlite3PagerSnapshotUnlock(pPager);
        }
      }
    }
  }

  sqlite3_mutex_leave(db->mutex);
#ಆದರೆ_ಅಂತ್ಯ   /* SQLITE_OMIT_WAL */
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Recover as many snapshots as possible from the wal file associated with
** schema zDb of database db.
*/
ಪೂರ್ಣಾಂಕ sqlite3_snapshot_recover(sqlite3 *db, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDb){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_ERROR;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WAL
  ಪೂರ್ಣಾಂಕ iDb;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !sqlite3SafetyCheckOk(db) ){
    ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
  }
#ಆದರೆ_ಅಂತ್ಯ

  sqlite3_mutex_enter(db->mutex);
  iDb = sqlite3_ಕಂಡುಹಿಡಿ_Db_Name(db, zDb);
  ಆದರೆ( iDb==0 || iDb>1 ){
    Btree *pBt = db->aDb[iDb].pBt;
    ಆದರೆ( SQLITE_TXN_NONE==sqlite3BtreeTxnState(pBt) ){
      rc = sqlite3BtreeBeginTrans(pBt, 0, 0);
      ಆದರೆ( rc==SQLITE_OK ){
        rc = sqlite3PagerSnapshotRecover(sqlite3BtreePager(pBt));
        sqlite3BtreeCommit(pBt);
      }
    }
  }
  sqlite3_mutex_leave(db->mutex);
#ಆದರೆ_ಅಂತ್ಯ   /* SQLITE_OMIT_WAL */
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Free a snapshot handle obtained from sqlite3_snapshot_get().
*/
ಶೂನ್ಯ sqlite3_snapshot_free(sqlite3_snapshot *pSnapshot){
  sqlite3_free(pSnapshot);
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_ENABLE_SNAPSHOT */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_COMPILEOPTION_DIAGS
/*
** Given the name of a compile-time option, return true if that option
** was used and false if not.
**
** The name can optionally begin with "SQLITE_" but the "SQLITE_" prefix
** is not required for a match.
*/
ಪೂರ್ಣಾಂಕ sqlite3_compileoption_ಬಳಸಿದ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zOptName){
  ಪೂರ್ಣಾಂಕ i, n;
  ಪೂರ್ಣಾಂಕ nOpt;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **azCompileOpt;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( zOptName==0 ){
    (ಶೂನ್ಯ)SQLITE_MISUSE_BKPT;
    ಹಿಂತಿರುಗಿ 0;
  }
#ಆದರೆ_ಅಂತ್ಯ

  azCompileOpt = sqlite3CompileOptions(&nOpt);

  ಆದರೆ( sqlite3StrNICmp(zOptName, "SQLITE_", 7)==0 ) zOptName += 7;
  n = sqlite3Strlen30(zOptName);

  /* Since nOpt is normally in single digits, a linear search is
  ** adequate. No need for a binary search. */
  ಗಾಗಿ(i=0; i<nOpt; i++){
    ಆದರೆ( sqlite3StrNICmp(zOptName, azCompileOpt[i], n)==0
     && sqlite3IsIdChar((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)azCompileOpt[i][n])==0
    ){
      ಹಿಂತಿರುಗಿ 1;
    }
  }
  ಹಿಂತಿರುಗಿ 0;
}

/*
** Return the N-th compile-time option string.  If N is out of range,
** return a NULL pointer.
*/
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3_compileoption_get(ಪೂರ್ಣಾಂಕ N){
  ಪೂರ್ಣಾಂಕ nOpt;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **azCompileOpt;
  azCompileOpt = sqlite3CompileOptions(&nOpt);
  ಆದರೆ( N>=0 && N<nOpt ){
    ಹಿಂತಿರುಗಿ azCompileOpt[N];
  }
  ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_COMPILEOPTION_DIAGS */
