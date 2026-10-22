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
#అన్తర్భావ "sqliteInt.h"

#నిర్వచితమైతే SQLITE_ENABLE_FTS3
# include "fts3.h"
#అయితే_ముగింపు
#నిర్వచితమైతే SQLITE_ENABLE_RTREE
# include "rtree.h"
#అయితే_ముగింపు
#ఒకవేళ defined(SQLITE_ENABLE_ICU) || defined(SQLITE_ENABLE_ICU_COLLATIONS)
# include "sqliteicu.h"
#అయితే_ముగింపు

/*
** This is an extension initializer that is a no-op and always
** succeeds, except that it fails if the fault-simulation is set
** to 500.
*/
స్థిర పూర్ణాంకం sqlite3TestExtInit(sqlite3 *db){
  (శూన్య)db;
  తిరిగి sqlite3FaultSim(500);
}


/*
** Forward declarations of external module initializer functions
** for modules that need them.
*/
#నిర్వచితమైతే SQLITE_ENABLE_FTS5
పూర్ణాంకం sqlite3Fts5Init(sqlite3*);
#అయితే_ముగింపు
#నిర్వచితమైతే SQLITE_ENABLE_STMTVTAB
పూర్ణాంకం sqlite3StmtVtabInit(sqlite3*);
#అయితే_ముగింపు
#నిర్వచితమైతే SQLITE_EXTRA_AUTOEXT
పూర్ణాంకం SQLITE_EXTRA_AUTOEXT(sqlite3*);
#అయితే_ముగింపు
/*
** An array of pointers to extension initializer functions for
** built-in extensions.
*/
స్థిర పూర్ణాంకం (*స్థిరరాశి sqlite3BuiltinExtensions[])(sqlite3*) = {
#నిర్వచితమైతే SQLITE_ENABLE_FTS3
  sqlite3Fts3Init,
#అయితే_ముగింపు
#నిర్వచితమైతే SQLITE_ENABLE_FTS5
  sqlite3Fts5Init,
#అయితే_ముగింపు
#ఒకవేళ defined(SQLITE_ENABLE_ICU) || defined(SQLITE_ENABLE_ICU_COLLATIONS)
  sqlite3IcuInit,
#అయితే_ముగింపు
#నిర్వచితమైతే SQLITE_ENABLE_RTREE
  sqlite3RtreeInit,
#అయితే_ముగింపు
#నిర్వచితమైతే SQLITE_ENABLE_DBPAGE_VTAB
  sqlite3_Dbpage_రిజిస్టరు,
#అయితే_ముగింపు
#నిర్వచితమైతే SQLITE_ENABLE_DBSTAT_VTAB
  sqlite3_Dbstat_రిజిస్టరు,
#అయితే_ముగింపు
  sqlite3TestExtInit,
#నిర్వచితమైతే SQLITE_ENABLE_STMTVTAB
  sqlite3StmtVtabInit,
#అయితే_ముగింపు
#నిర్వచితమైతే SQLITE_ENABLE_BYTECODE_VTAB
  sqlite3VdbeBytecodeVtabInit,
#అయితే_ముగింపు
#నిర్వచితమైతే SQLITE_EXTRA_AUTOEXT
  SQLITE_EXTRA_AUTOEXT,
#అయితే_ముగింపు
};

#నిర్వచితం_కాకపోతే SQLITE_AMALGAMATION
/* IMPLEMENTATION-OF: R-46656-45156 The sqlite3_version[] string constant
** contains the text of SQLITE_VERSION macro.
*/
స్థిరరాశి అక్షరం sqlite3_సంస్కరణ[] = SQLITE_సంస్కరణ;
#అయితే_ముగింపు

/* IMPLEMENTATION-OF: R-53536-42575 The sqlite3_libversion() function returns
** a pointer to the to the sqlite3_version[] string constant.
*/
స్థిరరాశి అక్షరం *sqlite3_libversion(శూన్య){ తిరిగి sqlite3_సంస్కరణ; }

/* IMPLEMENTATION-OF: R-25063-23286 The sqlite3_sourceid() function returns a
** pointer to a string constant whose value is the same as the
** SQLITE_SOURCE_ID C preprocessor macro. Except if SQLite is built using
** an edited copy of the amalgamation, then the last four characters of
** the hash might be different from SQLITE_SOURCE_ID.
*/
స్థిరరాశి అక్షరం *sqlite3_sourceid(శూన్య){ తిరిగి SQLITE_ఆధారం_ID; }

/* IMPLEMENTATION-OF: R-35210-63508 The sqlite3_libversion_number() function
** returns an integer equal to SQLITE_VERSION_NUMBER.
*/
పూర్ణాంకం sqlite3_libversion_సంఖ్య(శూన్య){ తిరిగి SQLITE_సంస్కరణ_సంఖ్య; }

/* IMPLEMENTATION-OF: R-20790-14025 The sqlite3_threadsafe() function returns
** zero if and only if SQLite was compiled with mutexing code omitted due to
** the SQLITE_THREADSAFE compile-time option being set to 0.
*/
పూర్ణాంకం sqlite3_threadsafe(శూన్య){ తిరిగి SQLITE_THREADSAFE; }

/*
** When compiling the test fixture or with debugging enabled (on Win32),
** this variable being set to non-zero will cause OSTRACE macros to emit
** extra diagnostic information.
*/
#నిర్వచితమైతే SQLITE_HAVE_OS_TRACE
# ifndef SQLITE_DEBUG_OS_TRACE
#   define SQLITE_DEBUG_OS_TRACE 0
# endif
  పూర్ణాంకం sqlite3OSTrace = SQLITE_DEBUG_OS_TRACE;
#అయితే_ముగింపు

#ఒకవేళ !defined(SQLITE_OMIT_TRACE) && defined(SQLITE_ENABLE_IOTRACE)
/*
** If the following function pointer is not NULL and if
** SQLITE_ENABLE_IOTRACE is enabled, then messages describing
** I/O active are written using this function.  These messages
** are intended for debugging activity only.
*/
SQLITE_API శూన్య (SQLITE_CDECL *sqlite3IoTrace)(స్థిరరాశి అక్షరం*, ...) = 0;
#అయితే_ముగింపు

/*
** If the following global variable points to a string which is the
** name of a directory, then that directory will be used to store
** temporary files.
**
** See also the "PRAGMA temp_store_directory" SQL command.
*/
అక్షరం *sqlite3_తాత్కాలిక_directory = 0;

/*
** If the following global variable points to a string which is the
** name of a directory, then that directory will be used to store
** all database files specified with a relative pathname.
**
** See also the "PRAGMA data_store_directory" SQL command.
*/
అక్షరం *sqlite3_దత్తాంశం_directory = 0;

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
స్థిర SQLITE_NOINLINE పూర్ణాంకం sqlite3Initialize(శూన్య){
  MUTEX_LOGIC( sqlite3_mutex *pMainMtx; )   /* The main static mutex */
  పూర్ణాంకం rc;                                   /* Result code */
#నిర్వచితమైతే SQLITE_EXTRA_INIT
  పూర్ణాంకం b_నిరంతరం_Extra_Init = 0;                    /* Extra initialization needed */
#అయితే_ముగింపు

  /* If the following assert() fails on some obscure processor/compiler
  ** combination to warn that SQLite has been mis-compiled.  If you hit
  ** this assert(), that means you need to recompile with the
  ** -DSQLITE_PTRSIZE=n compile-time option to set the correct pointer size.
  */
  ప్రతిజ్ఞా( SQLITE_PTRSIZE==పరిమాణం(అక్షరం*) );

  /* Make sure the mutex subsystem is initialized.  If unable to
  ** initialize the mutex subsystem, return early with the error.
  ** If the system is so sick that we are unable to allocate a mutex,
  ** there is not much SQLite is going to be able to do.
  **
  ** The mutex subsystem must take care of serializing its own
  ** initialization.
  */
  rc = sqlite3MutexInit();
  ఒకవేళ( rc ) తిరిగి rc;

  /* Initialize the malloc() system and the recursive pInitMutex mutex.
  ** This operation is protected by the STATIC_MAIN mutex.  Note that
  ** MutexAlloc() is called for a static mutex prior to initializing the
  ** malloc subsystem - this implies that the allocation of a static
  ** mutex must not require support from the malloc subsystem.
  */
  MUTEX_LOGIC( pMainMtx = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  sqlite3_mutex_enter(pMainMtx);
  sqlite3GlobalConfig.isMutexInit = 1;
  ఒకవేళ( !sqlite3GlobalConfig.isMallocInit ){
    rc = sqlite3MallocInit();
  }
  ఒకవేళ( rc==SQLITE_OK ){
    sqlite3GlobalConfig.isMallocInit = 1;
    ఒకవేళ( !sqlite3GlobalConfig.pInitMutex ){
      sqlite3GlobalConfig.pInitMutex =
           sqlite3MutexAlloc(SQLITE_MUTEX_RECURSIVE);
      ఒకవేళ( sqlite3GlobalConfig.bCoreMutex && !sqlite3GlobalConfig.pInitMutex ){
        rc = SQLITE_NOMEM_BKPT;
      }
    }
  }
  ఒకవేళ( rc==SQLITE_OK ){
    sqlite3GlobalConfig.nRefInitMutex++;
  }
  sqlite3_mutex_leave(pMainMtx);

  /* If rc is not SQLITE_OK at this point, then either the malloc
  ** subsystem could not be initialized or the system failed to allocate
  ** the pInitMutex mutex. Return an error in either case.  */
  ఒకవేళ( rc!=SQLITE_OK ){
    తిరిగి rc;
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
  ఒకవేళ( sqlite3GlobalConfig.isInit==0 && sqlite3GlobalConfig.లోపల_పురోగతి==0 ){
    sqlite3GlobalConfig.లోపల_పురోగతి = 1;
#నిర్వచితమైతే SQLITE_ENABLE_SQLLOG
    {
      బాహ్య శూన్య sqlite3_init_sqllog(శూన్య);
      sqlite3_init_sqllog();
    }
#అయితే_ముగింపు
    memset(&sqlite3BuiltinFunctions, 0, పరిమాణం(sqlite3BuiltinFunctions));
    sqlite3_రిజిస్టరు_Builtin_Functions();
    ఒకవేళ( sqlite3GlobalConfig.isPCacheInit==0 ){
      rc = sqlite3PcacheInitialize();
    }
    ఒకవేళ( rc==SQLITE_OK ){
      sqlite3GlobalConfig.isPCacheInit = 1;
      rc = sqlite3OsInit();
    }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_DESERIALIZE
    ఒకవేళ( rc==SQLITE_OK ){
      rc = sqlite3MemdbInit();
    }
#అయితే_ముగింపు
    ఒకవేళ( rc==SQLITE_OK ){
      sqlite3_PCache_బఫరు_అమరిక( sqlite3GlobalConfig.p_పుట,
          sqlite3GlobalConfig.sz_పుట, sqlite3GlobalConfig.n_పుట);
#నిర్వచితమైతే SQLITE_EXTRA_INIT_MUTEXED
      {
        పూర్ణాంకం SQLITE_EXTRA_INIT_MUTEXED(స్థిరరాశి అక్షరం*);
        rc = SQLITE_EXTRA_INIT_MUTEXED(0);
      }
#అయితే_ముగింపు
    }
    ఒకవేళ( rc==SQLITE_OK ){
      sqlite3_జ్ఞాపకం_Barrier();
      sqlite3GlobalConfig.isInit = 1;
#నిర్వచితమైతే SQLITE_EXTRA_INIT
      b_నిరంతరం_Extra_Init = 1;
#అయితే_ముగింపు
    }
    sqlite3GlobalConfig.లోపల_పురోగతి = 0;
  }
  sqlite3_mutex_leave(sqlite3GlobalConfig.pInitMutex);

  /* Go back under the static mutex and clean up the recursive
  ** mutex to prevent a resource leak.
  */
  sqlite3_mutex_enter(pMainMtx);
  sqlite3GlobalConfig.nRefInitMutex--;
  ఒకవేళ( sqlite3GlobalConfig.nRefInitMutex<=0 ){
    ప్రతిజ్ఞా( sqlite3GlobalConfig.nRefInitMutex==0 );
    sqlite3_mutex_free(sqlite3GlobalConfig.pInitMutex);
    sqlite3GlobalConfig.pInitMutex = 0;
  }
  sqlite3_mutex_leave(pMainMtx);

  /* The following is just a sanity check to make sure SQLite has
  ** been compiled correctly.  It is important to run this code, but
  ** we don't want to run it too often and soak up CPU cycles for no
  ** reason.  So we run it once during initialization.
  */
#నిర్వచితం_కాకపోతే NDEBUG
#నిర్వచితం_కాకపోతే SQLITE_OMIT_FLOATING_బిందువు
  /* This section of code's only "output" is via assert() statements. */
  ఒకవేళ( rc==SQLITE_OK ){
    u64 x = (((u64)1)<<63)-1;
    ద్విగుణ y;
    ప్రతిజ్ఞా(పరిమాణం(x)==8);
    ప్రతిజ్ఞా(పరిమాణం(x)==పరిమాణం(y));
    జ్ఞాపకం_నకలు(&y, &x, 8);
    ప్రతిజ్ఞా( sqlite3IsNaN(y) );
  }
#అయితే_ముగింపు
#అయితే_ముగింపు

  /* Do extra initialization steps requested by the SQLITE_EXTRA_INIT
  ** compile-time option.
  */
#నిర్వచితమైతే SQLITE_EXTRA_INIT
  ఒకవేళ( b_నిరంతరం_Extra_Init ){
    పూర్ణాంకం SQLITE_EXTRA_INIT(స్థిరరాశి అక్షరం*);
    rc = SQLITE_EXTRA_INIT(0);
  }
#అయితే_ముగింపు
  తిరిగి rc;
}
SQLITE_API పూర్ణాంకం sqlite3_initialize(శూన్య){
  /* If this build does not support writable static data (WSD) natively
  ** then we have to invoke the (application-supplied) WSD initialization
  ** routine before doing anything else. */
#నిర్వచితమైతే SQLITE_OMIT_WSD
  పూర్ణాంకం rc = sqlite3_wsd_init(4096, 24);
  ఒకవేళ( rc!=SQLITE_OK ){
    తిరిగి rc;
  }
#అయితే_ముగింపు

  ఒకవేళ( sqlite3GlobalConfig.isInit ){
    /* SQLite has already been initialized.  Fast early-out. */
    sqlite3_జ్ఞాపకం_Barrier();
    తిరిగి SQLITE_OK;
  }లేకపోతే{
    /* Invoke sqlite3Initialize() to do the actual work. */
    తిరిగి sqlite3Initialize();
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
పూర్ణాంకం sqlite3_shutdown(శూన్య){
#నిర్వచితమైతే SQLITE_OMIT_WSD
  పూర్ణాంకం rc = sqlite3_wsd_init(4096, 24);
  ఒకవేళ( rc!=SQLITE_OK ){
    తిరిగి rc;
  }
#అయితే_ముగింపు

  ఒకవేళ( sqlite3GlobalConfig.isInit ){
#నిర్వచితమైతే SQLITE_EXTRA_SHUTDOWN
    శూన్య SQLITE_EXTRA_SHUTDOWN(శూన్య);
    SQLITE_EXTRA_SHUTDOWN();
#అయితే_ముగింపు
    sqlite3_os_చివర();
    sqlite3_reset_auto_extension();
    sqlite3GlobalConfig.isInit = 0;
  }
  ఒకవేళ( sqlite3GlobalConfig.isPCacheInit ){
    sqlite3PcacheShutdown();
    sqlite3GlobalConfig.isPCacheInit = 0;
  }
  ఒకవేళ( sqlite3GlobalConfig.isMallocInit ){
    sqlite3_Malloc_చివర();
    sqlite3GlobalConfig.isMallocInit = 0;

#నిర్వచితం_కాకపోతే SQLITE_OMIT_SHUTDOWN_DIRECTORIES
    /* The heap subsystem has now been shutdown and these values are supposed
    ** to be NULL or point to memory that was obtained from sqlite3_malloc(),
    ** which would rely on that heap subsystem; therefore, make sure these
    ** values cannot refer to heap memory that was just invalidated when the
    ** heap subsystem was shutdown.  This is only done if the current call to
    ** this function resulted in the heap subsystem actually being shutdown.
    */
    sqlite3_దత్తాంశం_directory = 0;
    sqlite3_తాత్కాలిక_directory = 0;
#అయితే_ముగింపు
  }
  ఒకవేళ( sqlite3GlobalConfig.isMutexInit ){
    sqlite3_Mutex_చివర();
    sqlite3GlobalConfig.isMutexInit = 0;
  }

  తిరిగి SQLITE_OK;
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
పూర్ణాంకం sqlite3_config(పూర్ణాంకం op, ...){
  va_list ap;
  పూర్ణాంకం rc = SQLITE_OK;

  /* sqlite3_config() normally returns SQLITE_MISUSE if it is invoked while
  ** the SQLite library is in use.  Except, a few selected opcodes
  ** are allowed.
  */
  ఒకవేళ( sqlite3GlobalConfig.isInit ){
    స్థిర స్థిరరాశి u64 mAnytimeConfigOption = 0
       | MASKBIT64( SQLITE_CONFIG_LOG )
       | MASKBIT64( SQLITE_CONFIG_PCACHE_HDRSZ )
    ;
    ఒకవేళ( op<0 || op>63 || (MASKBIT64(op) & mAnytimeConfigOption)==0 ){
      తిరిగి SQLITE_MISUSE_BKPT;
    }
    testcase( op==SQLITE_CONFIG_LOG );
    testcase( op==SQLITE_CONFIG_PCACHE_HDRSZ );
  }

  va_start(ap, op);
  ఎంపిక( op ){

    /* Mutex configuration options are only available in a threadsafe
    ** compile.
    */
#ఒకవేళ defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE>0  /* IMP: R-54466-46756 */
    స్థితి SQLITE_CONFIG_SINGLETHREAD: {
      /* EVIDENCE-OF: R-02748-19096 This option sets the threading mode to
      ** Single-thread. */
      sqlite3GlobalConfig.bCoreMutex = 0;  /* Disable mutex on core */
      sqlite3GlobalConfig.b_నిండిన_Mutex = 0;  /* Disable mutex on connections */
      ఆపు;
    }
#అయితే_ముగింపు
#ఒకవేళ defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE>0 /* IMP: R-20520-54086 */
    స్థితి SQLITE_CONFIG_MULTITHREAD: {
      /* EVIDENCE-OF: R-14374-42468 This option sets the threading mode to
      ** Multi-thread. */
      sqlite3GlobalConfig.bCoreMutex = 1;  /* Enable mutex on core */
      sqlite3GlobalConfig.b_నిండిన_Mutex = 0;  /* Disable mutex on connections */
      ఆపు;
    }
#అయితే_ముగింపు
#ఒకవేళ defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE>0 /* IMP: R-59593-21810 */
    స్థితి SQLITE_CONFIG_SERIALIZED: {
      /* EVIDENCE-OF: R-41220-51800 This option sets the threading mode to
      ** Serialized. */
      sqlite3GlobalConfig.bCoreMutex = 1;  /* Enable mutex on core */
      sqlite3GlobalConfig.b_నిండిన_Mutex = 1;  /* Enable mutex on connections */
      ఆపు;
    }
#అయితే_ముగింపు
#ఒకవేళ defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE>0 /* IMP: R-63666-48755 */
    స్థితి SQLITE_CONFIG_MUTEX: {
      /* Specify an alternative mutex implementation */
      sqlite3GlobalConfig.mutex = *va_arg(ap, sqlite3_mutex_methods*);
      ఆపు;
    }
#అయితే_ముగింపు
#ఒకవేళ defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE>0 /* IMP: R-14450-37597 */
    స్థితి SQLITE_CONFIG_GETMUTEX: {
      /* Retrieve the current mutex implementation */
      *va_arg(ap, sqlite3_mutex_methods*) = sqlite3GlobalConfig.mutex;
      ఆపు;
    }
#అయితే_ముగింపు

    స్థితి SQLITE_CONFIG_MALLOC: {
      /* EVIDENCE-OF: R-55594-21030 The SQLITE_CONFIG_MALLOC option takes a
      ** single argument which is a pointer to an instance of the
      ** sqlite3_mem_methods structure. The argument specifies alternative
      ** low-level memory allocation routines to be used in place of the memory
      ** allocation routines built into SQLite. */
      sqlite3GlobalConfig.m = *va_arg(ap, sqlite3_mem_methods*);
      ఆపు;
    }
    స్థితి SQLITE_CONFIG_GETMALLOC: {
      /* EVIDENCE-OF: R-51213-46414 The SQLITE_CONFIG_GETMALLOC option takes a
      ** single argument which is a pointer to an instance of the
      ** sqlite3_mem_methods structure. The sqlite3_mem_methods structure is
      ** filled with the currently defined memory allocation routines. */
      ఒకవేళ( sqlite3GlobalConfig.m.xMalloc==0 ) sqlite3_Mem_సమితి_Default();
      *va_arg(ap, sqlite3_mem_methods*) = sqlite3GlobalConfig.m;
      ఆపు;
    }
    స్థితి SQLITE_CONFIG_MEMSTATUS: {
      ప్రతిజ్ఞా( !sqlite3GlobalConfig.isInit );  /* Cannot change at runtime */
      /* EVIDENCE-OF: R-61275-35157 The SQLITE_CONFIG_MEMSTATUS option takes
      ** single argument of type int, interpreted as a boolean, which enables
      ** or disables the collection of memory allocation statistics. */
      sqlite3GlobalConfig.bMemstat = va_arg(ap, పూర్ణాంకం);
      ఆపు;
    }
    స్థితి SQLITE_CONFIG_SMALL_MALLOC: {
      sqlite3GlobalConfig.bSmallMalloc = va_arg(ap, పూర్ణాంకం)!=0;
      ఆపు;
    }
    స్థితి SQLITE_CONFIG_PAGECACHE: {
      /* EVIDENCE-OF: R-18761-36601 There are three arguments to
      ** SQLITE_CONFIG_PAGECACHE: A pointer to 8-byte aligned memory (pMem),
      ** the size of each page cache line (sz), and the number of cache lines
      ** (N). */
      sqlite3GlobalConfig.p_పుట = va_arg(ap, శూన్య*);
      sqlite3GlobalConfig.sz_పుట = va_arg(ap, పూర్ణాంకం);
      sqlite3GlobalConfig.n_పుట = va_arg(ap, పూర్ణాంకం);
      ఆపు;
    }
    స్థితి SQLITE_CONFIG_PCACHE_HDRSZ: {
      /* EVIDENCE-OF: R-39100-27317 The SQLITE_CONFIG_PCACHE_HDRSZ option takes
      ** a single parameter which is a pointer to an integer and writes into
      ** that integer the number of extra bytes per page required for each page
      ** in SQLITE_CONFIG_PAGECACHE. */
      *va_arg(ap, పూర్ణాంకం*) =
          sqlite3_శీర్షిక_కొలత_Btree() +
          sqlite3_శీర్షిక_కొలత_Pcache() +
          sqlite3_శీర్షిక_కొలత_Pcache1();
      ఆపు;
    }

    స్థితి SQLITE_CONFIG_PCACHE: {
      /* no-op */
      ఆపు;
    }
    స్థితి SQLITE_CONFIG_GETPCACHE: {
      /* now an error */
      rc = SQLITE_ERROR;
      ఆపు;
    }

    స్థితి SQLITE_CONFIG_PCACHE2: {
      /* EVIDENCE-OF: R-63325-48378 The SQLITE_CONFIG_PCACHE2 option takes a
      ** single argument which is a pointer to an sqlite3_pcache_methods2
      ** object. This object specifies the interface to a custom page cache
      ** implementation. */
      sqlite3GlobalConfig.pcache2 = *va_arg(ap, sqlite3_pcache_methods2*);
      ఆపు;
    }
    స్థితి SQLITE_CONFIG_GETPCACHE2: {
      /* EVIDENCE-OF: R-22035-46182 The SQLITE_CONFIG_GETPCACHE2 option takes a
      ** single argument which is a pointer to an sqlite3_pcache_methods2
      ** object. SQLite copies of the current page cache implementation into
      ** that object. */
      ఒకవేళ( sqlite3GlobalConfig.pcache2.xInit==0 ){
        sqlite3_PCache_సమితి_Default();
      }
      *va_arg(ap, sqlite3_pcache_methods2*) = sqlite3GlobalConfig.pcache2;
      ఆపు;
    }

/* EVIDENCE-OF: R-06626-12911 The SQLITE_CONFIG_HEAP option is only
** available if SQLite is compiled with either SQLITE_ENABLE_MEMSYS3 or
** SQLITE_ENABLE_MEMSYS5 and returns SQLITE_ERROR if invoked otherwise. */
#ఒకవేళ defined(SQLITE_ENABLE_MEMSYS3) || defined(SQLITE_ENABLE_MEMSYS5)
    స్థితి SQLITE_CONFIG_కుప్ప: {
      /* EVIDENCE-OF: R-19854-42126 There are three arguments to
      ** SQLITE_CONFIG_HEAP: An 8-byte aligned pointer to the memory, the
      ** number of bytes in the memory buffer, and the minimum allocation size.
      */
      sqlite3GlobalConfig.p_కుప్ప = va_arg(ap, శూన్య*);
      sqlite3GlobalConfig.n_కుప్ప = va_arg(ap, పూర్ణాంకం);
      sqlite3GlobalConfig.mnReq = va_arg(ap, పూర్ణాంకం);

      ఒకవేళ( sqlite3GlobalConfig.mnReq<1 ){
        sqlite3GlobalConfig.mnReq = 1;
      }లేకపోతే ఒకవేళ( sqlite3GlobalConfig.mnReq>(1<<12) ){
        /* cap min request size at 2^12 */
        sqlite3GlobalConfig.mnReq = (1<<12);
      }

      ఒకవేళ( sqlite3GlobalConfig.p_కుప్ప==0 ){
        /* EVIDENCE-OF: R-49920-60189 If the first pointer (the memory pointer)
        ** is NULL, then SQLite reverts to using its default memory allocator
        ** (the system malloc() implementation), undoing any prior invocation of
        ** SQLITE_CONFIG_MALLOC.
        **
        ** Setting sqlite3GlobalConfig.m to all zeros will cause malloc to
        ** revert to its default implementation when sqlite3_initialize() is run
        */
        memset(&sqlite3GlobalConfig.m, 0, పరిమాణం(sqlite3GlobalConfig.m));
      }లేకపోతే{
        /* EVIDENCE-OF: R-61006-08918 If the memory pointer is not NULL then the
        ** alternative memory allocator is engaged to handle all of SQLites
        ** memory allocation needs. */
#నిర్వచితమైతే SQLITE_ENABLE_MEMSYS3
        sqlite3GlobalConfig.m = *sqlite3MemGetMemsys3();
#అయితే_ముగింపు
#నిర్వచితమైతే SQLITE_ENABLE_MEMSYS5
        sqlite3GlobalConfig.m = *sqlite3MemGetMemsys5();
#అయితే_ముగింపు
      }
      ఆపు;
    }
#అయితే_ముగింపు

    స్థితి SQLITE_CONFIG_LOOKASIDE: {
      sqlite3GlobalConfig.szLookaside = va_arg(ap, పూర్ణాంకం);
      sqlite3GlobalConfig.nLookaside = va_arg(ap, పూర్ణాంకం);
      ఆపు;
    }
   
    /* Record a pointer to the logger function and its first argument.
    ** The default is NULL.  Logging is disabled if the function pointer is
    ** NULL.
    */
    స్థితి SQLITE_CONFIG_LOG: {
      /* MSVC is picky about pulling func ptrs from va lists.
      ** http://support.microsoft.com/kb/47961
      ** sqlite3GlobalConfig.xLog = va_arg(ap, void(*)(void*,int,const char*));
      */
      రకనిర్వచనం శూన్య(*LOGFUNC_t)(శూన్య*,పూర్ణాంకం,స్థిరరాశి అక్షరం*);
      LOGFUNC_t xLog = va_arg(ap, LOGFUNC_t);
      శూన్య *pLogArg = va_arg(ap, శూన్య*);
      AtomicStore(&sqlite3GlobalConfig.xLog, xLog);
      AtomicStore(&sqlite3GlobalConfig.pLogArg, pLogArg);
      ఆపు;
    }

    /* EVIDENCE-OF: R-55548-33817 The compile-time setting for URI filenames
    ** can be changed at start-time using the
    ** sqlite3_config(SQLITE_CONFIG_URI,1) or
    ** sqlite3_config(SQLITE_CONFIG_URI,0) configuration calls.
    */
    స్థితి SQLITE_CONFIG_URI: {
      /* EVIDENCE-OF: R-25451-61125 The SQLITE_CONFIG_URI option takes a single
      ** argument of type int. If non-zero, then URI handling is globally
      ** enabled. If the parameter is zero, then URI handling is globally
      ** disabled. */
      పూర్ణాంకం bOpenUri = va_arg(ap, పూర్ణాంకం);
      AtomicStore(&sqlite3GlobalConfig.bOpenUri, bOpenUri);
      ఆపు;
    }

    స్థితి SQLITE_CONFIG_COVERING_సూచీ_పరిశీలన: {
      /* EVIDENCE-OF: R-36592-02772 The SQLITE_CONFIG_COVERING_INDEX_SCAN
      ** option takes a single integer argument which is interpreted as a
      ** boolean in order to enable or disable the use of covering indices for
      ** full table scans in the query optimizer. */
      sqlite3GlobalConfig.bUseCis = va_arg(ap, పూర్ణాంకం);
      ఆపు;
    }

#నిర్వచితమైతే SQLITE_ENABLE_SQLLOG
    స్థితి SQLITE_CONFIG_SQLLOG: {
      రకనిర్వచనం శూన్య(*SQLLOGFUNC_t)(శూన్య*, sqlite3*, స్థిరరాశి అక్షరం*, పూర్ణాంకం);
      sqlite3GlobalConfig.xSqllog = va_arg(ap, SQLLOGFUNC_t);
      sqlite3GlobalConfig.pSqllogArg = va_arg(ap, శూన్య *);
      ఆపు;
    }
#అయితే_ముగింపు

    స్థితి SQLITE_CONFIG_MMAP_కొలత: {
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
      ఒకవేళ( mxMmap<0 || mxMmap>SQLITE_MAX_MMAP_కొలత ){
        mxMmap = SQLITE_MAX_MMAP_కొలత;
      }
      ఒకవేళ( szMmap<0 ) szMmap = SQLITE_DEFAULT_MMAP_కొలత;
      ఒకవేళ( szMmap>mxMmap) szMmap = mxMmap;
      sqlite3GlobalConfig.mxMmap = mxMmap;
      sqlite3GlobalConfig.szMmap = szMmap;
      ఆపు;
    }

#ఒకవేళ SQLITE_OS_WIN && defined(SQLITE_WIN32_MALLOC) /* IMP: R-04780-55815 */
    స్థితి SQLITE_CONFIG_WIN32_HEAPSIZE: {
      /* EVIDENCE-OF: R-34926-03360 SQLITE_CONFIG_WIN32_HEAPSIZE takes a 32-bit
      ** unsigned integer value that specifies the maximum size of the created
      ** heap. */
      sqlite3GlobalConfig.n_కుప్ప = va_arg(ap, పూర్ణాంకం);
      ఆపు;
    }
#అయితే_ముగింపు

    స్థితి SQLITE_CONFIG_PMASZ: {
      sqlite3GlobalConfig.szPma = va_arg(ap, గుర్తులేని పూర్ణాంకం);
      ఆపు;
    }

    స్థితి SQLITE_CONFIG_STMTJRNL_SPILL: {
      sqlite3GlobalConfig.nStmtSpill = va_arg(ap, పూర్ణాంకం);
      ఆపు;
    }

#నిర్వచితమైతే SQLITE_ENABLE_SORTER_REFERENCES
    స్థితి SQLITE_CONFIG_SORTERREF_కొలత: {
      పూర్ణాంకం iVal = va_arg(ap, పూర్ణాంకం);
      ఒకవేళ( iVal<0 ){
        iVal = SQLITE_DEFAULT_SORTERREF_కొలత;
      }
      sqlite3GlobalConfig.szSorterRef = (u32)iVal;
      ఆపు;
    }
#అయితే_ముగింపు /* SQLITE_ENABLE_SORTER_REFERENCES */

#నిర్వచితం_కాకపోతే SQLITE_OMIT_DESERIALIZE
    స్థితి SQLITE_CONFIG_MEMDB_MAXSIZE: {
      sqlite3GlobalConfig.mx_Memdb_కొలత = va_arg(ap, sqlite3_int64);
      ఆపు;
    }
#అయితే_ముగింపు /* SQLITE_OMIT_DESERIALIZE */

    స్థితి SQLITE_CONFIG_ROWID_లోపల_VIEW: {
      పూర్ణాంకం *pVal = va_arg(ap,పూర్ణాంకం*);
#నిర్వచితమైతే SQLITE_ALLOW_ROWID_లోపల_VIEW
      ఒకవేళ( 0==*pVal ) sqlite3GlobalConfig.mNoVisibleRowid = TF_NoVisibleRowid;
      ఒకవేళ( 1==*pVal ) sqlite3GlobalConfig.mNoVisibleRowid = 0;
      *pVal = (sqlite3GlobalConfig.mNoVisibleRowid==0);
#లేకపోతే
      *pVal = 0;
#అయితే_ముగింపు
      ఆపు;
    }

    అప్రమేయ: {
      rc = SQLITE_ERROR;
      ఆపు;
    }
  }
  va_end(ap);
  తిరిగి rc;
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
స్థిర పూర్ణాంకం అమరిక_Lookaside(
  sqlite3 *db,    /* Database connection being configured */
  శూన్య *pBuf,     /* Memory to use for lookaside.  May be NULL */
  పూర్ణాంకం sz,         /* Desired size of each lookaside memory slot */
  పూర్ణాంకం cnt         /* Number of slots to allocate */
){
#నిర్వచితం_కాకపోతే SQLITE_OMIT_LOOKASIDE
  శూన్య *p_ప్రారంభం;          /* Start of the lookaside buffer */
  sqlite3_int64 szAlloc; /* Total space set aside for lookaside memory */
  పూర్ణాంకం nBig;              /* Number of full-size slots */
  పూర్ణాంకం nSm;               /* Number smaller LOOKASIDE_SMALL-byte slots */
 
  ఒకవేళ( sqlite3_Lookaside_వాడిన(db,0)>0 ){
    తిరిగి SQLITE_BUSY;
  }
  /* Free any existing lookaside buffer for this handle before
  ** allocating a new one so we don't have to have space for
  ** both at the same time.
  */
  ఒకవేళ( db->lookaside.bMalloced ){
    sqlite3_free(db->lookaside.p_ప్రారంభం);
  }
  /* The size of a lookaside slot after ROUNDDOWN8 needs to be larger
  ** than a pointer and small enough to fit in a u16.
  */
  sz = ROUNDDOWN8(sz);
  ఒకవేళ( sz<=(పూర్ణాంకం)పరిమాణం(Lookaside_స్థానం*) ) sz = 0;
  ఒకవేళ( sz>65528 ) sz = 65528;
  /* Count must be at least 1 to be useful, but not so large as to use
  ** more than 0x7fff0000 total bytes for lookaside. */
  ఒకవేళ( cnt<1 ) cnt = 0;
  ఒకవేళ( sz>0 && cnt>(0x7fff0000/sz) ) cnt = 0x7fff0000/sz;
  szAlloc = (i64)sz*(i64)cnt;
  ఒకవేళ( szAlloc==0 ){
    sz = 0;
    p_ప్రారంభం = 0;
  }లేకపోతే ఒకవేళ( pBuf==0 ){
    sqlite3BeginBenignMalloc();
    p_ప్రారంభం = sqlite3Malloc( szAlloc );
    sqlite3_చివర_Benign_Malloc();
    ఒకవేళ( p_ప్రారంభం ) szAlloc = sqlite3_Malloc_కొలత(p_ప్రారంభం);
  }లేకపోతే{
    p_ప్రారంభం = pBuf;
  }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_TWOSIZE_LOOKASIDE
  ఒకవేళ( sz>=LOOKASIDE_SMALL*3 ){
    nBig = szAlloc/(3*LOOKASIDE_SMALL+sz);
    nSm = (szAlloc - (i64)sz*(i64)nBig)/LOOKASIDE_SMALL;
  }లేకపోతే ఒకవేళ( sz>=LOOKASIDE_SMALL*2 ){
    nBig = szAlloc/(LOOKASIDE_SMALL+sz);
    nSm = (szAlloc - (i64)sz*(i64)nBig)/LOOKASIDE_SMALL;
  }లేకపోతే
#అయితే_ముగింపు /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
  ఒకవేళ( sz>0 ){
    nBig = szAlloc/sz;
    nSm = 0;
  }లేకపోతే{
    nBig = nSm = 0;
  }
  db->lookaside.p_ప్రారంభం = p_ప్రారంభం;
  db->lookaside.pInit = 0;
  db->lookaside.pFree = 0;
  db->lookaside.sz = (u16)sz;
  db->lookaside.szTrue = (u16)sz;
  ఒకవేళ( p_ప్రారంభం ){
    పూర్ణాంకం i;
    Lookaside_స్థానం *p;
    ప్రతిజ్ఞా( sz > (పూర్ణాంకం)పరిమాణం(Lookaside_స్థానం*) );
    p = (Lookaside_స్థానం*)p_ప్రారంభం;
    కొరకు(i=0; i<nBig; i++){
      p->p_తదుపరి = db->lookaside.pInit;
      db->lookaside.pInit = p;
      p = (Lookaside_స్థానం*)&((u8*)p)[sz];
    }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_TWOSIZE_LOOKASIDE
    db->lookaside.pSmallInit = 0;
    db->lookaside.pSmallFree = 0;
    db->lookaside.p_మధ్య = p;
    కొరకు(i=0; i<nSm; i++){
      p->p_తదుపరి = db->lookaside.pSmallInit;
      db->lookaside.pSmallInit = p;
      p = (Lookaside_స్థానం*)&((u8*)p)[LOOKASIDE_SMALL];
    }
#అయితే_ముగింపు /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
    ప్రతిజ్ఞా( ((uptr)p)<=szAlloc + (uptr)p_ప్రారంభం );
    db->lookaside.p_చివర = p;
    db->lookaside.bDisable = 0;
    db->lookaside.bMalloced = pBuf==0 ?1:0;
    db->lookaside.n_స్థానం = nBig+nSm;
  }లేకపోతే{
    db->lookaside.p_ప్రారంభం = 0;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_TWOSIZE_LOOKASIDE
    db->lookaside.pSmallInit = 0;
    db->lookaside.pSmallFree = 0;
    db->lookaside.p_మధ్య = 0;
#అయితే_ముగింపు /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
    db->lookaside.p_చివర = 0;
    db->lookaside.bDisable = 1;
    db->lookaside.sz = 0;
    db->lookaside.bMalloced = 0;
    db->lookaside.n_స్థానం = 0;
  }
  db->lookaside.p_True_చివర = db->lookaside.p_చివర;
  ప్రతిజ్ఞా( sqlite3_Lookaside_వాడిన(db,0)==0 );
#అయితే_ముగింపు /* SQLITE_OMIT_LOOKASIDE */
  తిరిగి SQLITE_OK;
}

/*
** Return the mutex associated with a database connection.
*/
sqlite3_mutex *sqlite3_db_mutex(sqlite3 *db){
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !sqlite3SafetyCheckOk(db) ){
    (శూన్య)SQLITE_MISUSE_BKPT;
    తిరిగి 0;
  }
#అయితే_ముగింపు
  తిరిగి db->mutex;
}

/*
** Free up as much memory as we can from the given database
** connection.
*/
పూర్ణాంకం sqlite3_db_release_జ్ఞాపకం(sqlite3 *db){
  పూర్ణాంకం i;

#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !sqlite3SafetyCheckOk(db) ) తిరిగి SQLITE_MISUSE_BKPT;
#అయితే_ముగింపు
  sqlite3_mutex_enter(db->mutex);
  sqlite3BtreeEnterAll(db);
  కొరకు(i=0; i<db->nDb; i++){
    Btree *pBt = db->aDb[i].pBt;
    ఒకవేళ( pBt ){
      Pager *pPager = sqlite3BtreePager(pBt);
      sqlite3PagerShrink(pPager);
    }
  }
  sqlite3BtreeLeaveAll(db);
  sqlite3_mutex_leave(db->mutex);
  తిరిగి SQLITE_OK;
}

/*
** Flush any dirty pages in the pager-cache for any attached database
** to disk.
*/
పూర్ణాంకం sqlite3_db_cacheflush(sqlite3 *db){
  పూర్ణాంకం i;
  పూర్ణాంకం rc = SQLITE_OK;
  పూర్ణాంకం b_చూసిన_Busy = 0;

#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !sqlite3SafetyCheckOk(db) ) తిరిగి SQLITE_MISUSE_BKPT;
#అయితే_ముగింపు
  sqlite3_mutex_enter(db->mutex);
  sqlite3BtreeEnterAll(db);
  కొరకు(i=0; rc==SQLITE_OK && i<db->nDb; i++){
    Btree *pBt = db->aDb[i].pBt;
    ఒకవేళ( pBt && sqlite3BtreeTxnState(pBt)==SQLITE_TXN_WRITE ){
      Pager *pPager = sqlite3BtreePager(pBt);
      rc = sqlite3PagerFlush(pPager);
      ఒకవేళ( rc==SQLITE_BUSY ){
        b_చూసిన_Busy = 1;
        rc = SQLITE_OK;
      }
    }
  }
  sqlite3BtreeLeaveAll(db);
  sqlite3_mutex_leave(db->mutex);
  తిరిగి ((rc==SQLITE_OK && b_చూసిన_Busy) ? SQLITE_BUSY : rc);
}

/*
** Configuration settings for an individual database connection
*/
పూర్ణాంకం sqlite3_db_config(sqlite3 *db, పూర్ణాంకం op, ...){
  va_list ap;
  పూర్ణాంకం rc;

#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !sqlite3SafetyCheckOk(db) ) తిరిగి SQLITE_MISUSE_BKPT;
#అయితే_ముగింపు
  sqlite3_mutex_enter(db->mutex);
  va_start(ap, op);
  ఎంపిక( op ){
    స్థితి SQLITE_DBCONFIG_MAINDBNAME: {
      /* IMP: R-06824-28531 */
      /* IMP: R-36257-52125 */
      db->aDb[0].zDbSName = va_arg(ap,అక్షరం*);
      rc = SQLITE_OK;
      ఆపు;
    }
    స్థితి SQLITE_DBCONFIG_LOOKASIDE: {
      శూన్య *pBuf = va_arg(ap, శూన్య*); /* IMP: R-26835-10964 */
      పూర్ణాంకం sz = va_arg(ap, పూర్ణాంకం);       /* IMP: R-47871-25994 */
      పూర్ణాంకం cnt = va_arg(ap, పూర్ణాంకం);      /* IMP: R-04460-53386 */
      rc = అమరిక_Lookaside(db, pBuf, sz, cnt);
      ఆపు;
    }
    స్థితి SQLITE_DBCONFIG_FP_DIGITS: {
      పూర్ణాంకం n_లోపల = va_arg(ap, పూర్ణాంకం);
      పూర్ణాంకం *p_బహిర్గతం = va_arg(ap, పూర్ణాంకం*);
      ఒకవేళ( n_లోపల>3 && n_లోపల<24 ) db->nFpDigit = (u8)n_లోపల;
      ఒకవేళ( p_బహిర్గతం ) *p_బహిర్గతం = db->nFpDigit;
      rc = SQLITE_OK;
      ఆపు;
    }
    అప్రమేయ: {
      స్థిర స్థిరరాశి నిర్మాణం {
        పూర్ణాంకం op;      /* The opcode */
        u64 ముసుగు;    /* Mask of the bit in sqlite3.flags to set/clear */
      } aFlagOp[] = {
        { SQLITE_DBCONFIG_ENABLE_FKEY,           SQLITE_Foreign_కీలకాలు    },
        { SQLITE_DBCONFIG_ENABLE_TRIGGER,        SQLITE_EnableTrigger  },
        { SQLITE_DBCONFIG_ENABLE_VIEW,           SQLITE_EnableView     },
        { SQLITE_DBCONFIG_ENABLE_FTS3_TOKENIZER, SQLITE_Fts3Tokenizer  },
        { SQLITE_DBCONFIG_ENABLE_ఎక్కించు_EXTENSION, SQLITE_ఎక్కించు_Extension  },
        { SQLITE_DBCONFIG_NO_CKPT_ON_CLOSE,      SQLITE_NoCkptOnClose  },
        { SQLITE_DBCONFIG_ENABLE_QPSG,           SQLITE_EnableQPSG     },
        { SQLITE_DBCONFIG_TRIGGER_EQP,           SQLITE_TriggerEQP     },
        { SQLITE_DBCONFIG_RESET_DATABASE,        SQLITE_ResetDatabase  },
        { SQLITE_DBCONFIG_DEFENSIVE,             SQLITE_Defensive      },
        { SQLITE_DBCONFIG_WRITABLE_SCHEMA,       SQLITE_WriteSchema|
                                                 SQLITE_NoSchemaError  },
        { SQLITE_DBCONFIG_LEGACY_ALTER_పట్టిక,    SQLITE_LegacyAlter    },
        { SQLITE_DBCONFIG_DQS_DDL,               SQLITE_DqsDDL         },
        { SQLITE_DBCONFIG_DQS_DML,               SQLITE_DqsDML         },
        { SQLITE_DBCONFIG_LEGACY_FILE_FORMAT,    SQLITE_LegacyFileFmt  },
        { SQLITE_DBCONFIG_TRUSTED_SCHEMA,        SQLITE_TrustedSchema  },
        { SQLITE_DBCONFIG_STMT_SCANSTATUS,       SQLITE_Stmt_పరిశీలన_స్థితి },
        { SQLITE_DBCONFIG_తిరగేయి_SCANORDER,     SQLITE_తిరగేయి_క్రమం   },
        { SQLITE_DBCONFIG_ENABLE_ATTACH_CREATE,  SQLITE_AttachCreate   },
        { SQLITE_DBCONFIG_ENABLE_ATTACH_WRITE,   SQLITE_AttachWrite    },
        { SQLITE_DBCONFIG_ENABLE_COMMENTS,       SQLITE_Comments       },
      };
      గుర్తులేని పూర్ణాంకం i;
      rc = SQLITE_ERROR; /* IMP: R-42790-23372 */
      కొరకు(i=0; i<Array_కొలత(aFlagOp); i++){
        ఒకవేళ( aFlagOp[i].op==op ){
          పూర్ణాంకం onoff = va_arg(ap, పూర్ణాంకం);
          పూర్ణాంకం *pRes = va_arg(ap, పూర్ణాంకం*);
          u64 oldFlags = db->flags;
          ఒకవేళ( onoff>0 ){
            db->flags |= aFlagOp[i].ముసుగు;
          }లేకపోతే ఒకవేళ( onoff==0 ){
            db->flags &= ~(u64)aFlagOp[i].ముసుగు;
          }
          ఒకవేళ( oldFlags!=db->flags ){
            sqlite3ExpirePreparedStatements(db, 0);
          }
          ఒకవేళ( pRes ){
            *pRes = (db->flags & aFlagOp[i].ముసుగు)!=0;
          }
          rc = SQLITE_OK;
          ఆపు;
        }
      }
      ఆపు;
    }
  }
  va_end(ap);
  sqlite3_mutex_leave(db->mutex);
  తిరిగి rc;
}

/*
** This is the default collating function named "BINARY" which is always
** available.
*/
పూర్ణాంకం sqlite3_ద్విమాన_Compare(
  శూన్య *Not_వాడిన,
  పూర్ణాంకం nKey1, స్థిరరాశి శూన్య *pKey1,
  పూర్ణాంకం nKey2, స్థిరరాశి శూన్య *pKey2
){
  పూర్ణాంకం rc, n;
  వాడని_PARAMETER(Not_వాడిన);
  n = nKey1<nKey2 ? nKey1 : nKey2;
  /* EVIDENCE-OF: R-65033-28449 The built-in BINARY collation compares
  ** strings byte by byte using the memcmp() function from the standard C
  ** library. */
  ప్రతిజ్ఞా( pKey1 && pKey2 );
  rc = memcmp(pKey1, pKey2, n);
  ఒకవేళ( rc==0 ){
    rc = nKey1 - nKey2;
  }
  తిరిగి rc;
}

/*
** This is the collating function named "RTRIM" which is always
** available.  Ignore trailing spaces.
*/
స్థిర పూర్ణాంకం rtrimCollFunc(
  శూన్య *pUser,
  పూర్ణాంకం nKey1, స్థిరరాశి శూన్య *pKey1,
  పూర్ణాంకం nKey2, స్థిరరాశి శూన్య *pKey2
){
  స్థిరరాశి u8 *pK1 = (స్థిరరాశి u8*)pKey1;
  స్థిరరాశి u8 *pK2 = (స్థిరరాశి u8*)pKey2;
  అయితే( nKey1 && pK1[nKey1-1]==' ' ) nKey1--;
  అయితే( nKey2 && pK2[nKey2-1]==' ' ) nKey2--;
  తిరిగి sqlite3_ద్విమాన_Compare(pUser, nKey1, pKey1, nKey2, pKey2);
}

/*
** Return true if CollSeq is the default built-in BINARY.
*/
#ఒకవేళ !SQLITE_ఉపయోగిస్తుంది_INLINE
పూర్ణాంకం sqlite3_Is_ద్విమాన(స్థిరరాశి CollSeq *p){
  ప్రతిజ్ఞా( p==0 || p->xCmp!=sqlite3_ద్విమాన_Compare
       || strcmp(p->zName,"BINARY")==0 );
  తిరిగి p==0 || p->xCmp==sqlite3_ద్విమాన_Compare;
}
#అయితే_ముగింపు

/*
** Another built-in collating sequence: NOCASE.
**
** This collating sequence is intended to be used for "case independent
** comparison". SQLite's knowledge of upper and lower case equivalents
** extends only to the 26 characters used in the English language.
**
** At the moment there is only a UTF-8 implementation.
*/
స్థిర పూర్ణాంకం nocaseCollatingFunc(
  శూన్య *Not_వాడిన,
  పూర్ణాంకం nKey1, స్థిరరాశి శూన్య *pKey1,
  పూర్ణాంకం nKey2, స్థిరరాశి శూన్య *pKey2
){
  పూర్ణాంకం r = sqlite3StrNICmp(
      (స్థిరరాశి అక్షరం *)pKey1, (స్థిరరాశి అక్షరం *)pKey2, (nKey1<nKey2)?nKey1:nKey2);
  వాడని_PARAMETER(Not_వాడిన);
  ఒకవేళ( 0==r ){
    r = nKey1-nKey2;
  }
  తిరిగి r;
}

/*
** Return the ROWID of the most recent insert
*/
sqlite_int64 sqlite3_చివరి_చొప్పించు_rowid(sqlite3 *db){
  i64 iRet;
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !sqlite3SafetyCheckOk(db) ){
    (శూన్య)SQLITE_MISUSE_BKPT;
    తిరిగి 0;
  }
#అయితే_ముగింపు
  sqlite3_mutex_enter(db->mutex);
  iRet = db->చివరి_Rowid;
  sqlite3_mutex_leave(db->mutex);
  తిరిగి iRet;
}

/*
** Set the value returned by the sqlite3_last_insert_rowid() API function.
*/
శూన్య sqlite3_సమితి_చివరి_చొప్పించు_rowid(sqlite3 *db, sqlite3_int64 iRowid){
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !sqlite3SafetyCheckOk(db) ){
    (శూన్య)SQLITE_MISUSE_BKPT;
    తిరిగి;
  }
#అయితే_ముగింపు
  sqlite3_mutex_enter(db->mutex);
  db->చివరి_Rowid = iRowid;
  sqlite3_mutex_leave(db->mutex);
}

/*
** Return the number of changes in the most recently executed DML
** statement.
*/
sqlite3_int64 sqlite3_changes64(sqlite3 *db){
  i64 iRet;
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !sqlite3SafetyCheckOk(db) ){
    (శూన్య)SQLITE_MISUSE_BKPT;
    తిరిగి 0;
  }
#అయితే_ముగింపు
  sqlite3_mutex_enter(db->mutex);
  iRet = db->nChange;
  sqlite3_mutex_leave(db->mutex);
  తిరిగి iRet;
}
పూర్ణాంకం sqlite3_changes(sqlite3 *db){
  తిరిగి (పూర్ణాంకం)sqlite3_changes64(db);
}

/*
** Return the number of changes since the database handle was opened.
*/
sqlite3_int64 sqlite3_మొత్తం_changes64(sqlite3 *db){
  i64 iRet;
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !sqlite3SafetyCheckOk(db) ){
    (శూన్య)SQLITE_MISUSE_BKPT;
    తిరిగి 0;
  }
#అయితే_ముగింపు
  sqlite3_mutex_enter(db->mutex);
  iRet = db->n_మొత్తం_Change;
  sqlite3_mutex_leave(db->mutex);
  తిరిగి iRet;
}
పూర్ణాంకం sqlite3_మొత్తం_changes(sqlite3 *db){
  తిరిగి (పూర్ణాంకం)sqlite3_మొత్తం_changes64(db);
}

/*
** Close all open savepoints. This function only manipulates fields of the
** database handle object, it does not close any savepoints that may be open
** at the b-tree/pager level.
*/
శూన్య sqlite3CloseSavepoints(sqlite3 *db){
  అయితే( db->pSavepoint ){
    Savepoint *pTmp = db->pSavepoint;
    db->pSavepoint = pTmp->p_తదుపరి;
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
స్థిర శూన్య functionDestroy(sqlite3 *db, FuncDef *p){
  FuncDestructor *pDestructor;
  ప్రతిజ్ఞా( (p->funcFlags & SQLITE_FUNC_BUILTIN)==0 );
  pDestructor = p->u.pDestructor;
  ఒకవేళ( pDestructor ){
    pDestructor->nRef--;
    ఒకవేళ( pDestructor->nRef==0 ){
      pDestructor->xDestroy(pDestructor->p_User_దత్తాంశం);
      sqlite3DbFree(db, pDestructor);
    }
  }
}

/*
** Disconnect all sqlite3_vtab objects that belong to database connection
** db. This is called when db is being closed.
*/
స్థిర శూన్య disconnectAllVtab(sqlite3 *db){
#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
  పూర్ణాంకం i;
  HashElem *p;
  sqlite3BtreeEnterAll(db);
  కొరకు(i=0; i<db->nDb; i++){
    Schema *pSchema = db->aDb[i].pSchema;
    ఒకవేళ( pSchema ){
      కొరకు(p=sqlite_Hash_మొదటి(&pSchema->tblHash); p; p=sqlite_Hash_తదుపరి(p)){
        పట్టిక *pTab = (పట్టిక *)sqlite_Hash_దత్తాంశం(p);
        ఒకవేళ( IsVirtual(pTab) ) sqlite3VtabDisconnect(db, pTab);
      }
    }
  }
  కొరకు(p=sqlite_Hash_మొదటి(&db->aModule); p; p=sqlite_Hash_తదుపరి(p)){
    Module *pMod = (Module *)sqlite_Hash_దత్తాంశం(p);
    ఒకవేళ( pMod->pEpoTab ){
      sqlite3VtabDisconnect(db, pMod->pEpoTab);
    }
  }
  sqlite3VtabUnlockList(db);
  sqlite3BtreeLeaveAll(db);
#లేకపోతే
  వాడని_PARAMETER(db);
#అయితే_ముగింపు
}

/*
** Return TRUE if database connection db has unfinalized prepared
** statements or unfinished sqlite3_backup objects. 
*/
స్థిర పూర్ణాంకం connectionIsBusy(sqlite3 *db){
  పూర్ణాంకం j;
  ప్రతిజ్ఞా( sqlite3_mutex_held(db->mutex) );
  ఒకవేళ( db->pVdbe ) తిరిగి 1;
  కొరకు(j=0; j<db->nDb; j++){
    Btree *pBt = db->aDb[j].pBt;
    ఒకవేళ( pBt && sqlite3_Btree_Is_లోపల_Backup(pBt) ) తిరిగి 1;
  }
  తిరిగి 0;
}

/*
** Close an existing SQLite database
*/
స్థిర పూర్ణాంకం sqlite3Close(sqlite3 *db, పూర్ణాంకం forceZombie){
  ఒకవేళ( !db ){
    /* EVIDENCE-OF: R-63257-11740 Calling sqlite3_close() or
    ** sqlite3_close_v2() with a NULL pointer argument is a harmless no-op. */
    తిరిగి SQLITE_OK;
  }
  ఒకవేళ( !sqlite3SafetyCheckSickOrOk(db) ){
    తిరిగి SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(db->mutex);
  ఒకవేళ( db->mTrace & SQLITE_TRACE_CLOSE ){
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
  ఒకవేళ( !forceZombie && connectionIsBusy(db) ){
    sqlite3_Error_With_సందేశం(db, SQLITE_BUSY, "unable to close due to unfinalized "
       "statements or unfinished backups");
    sqlite3_mutex_leave(db->mutex);
    తిరిగి SQLITE_BUSY;
  }

#నిర్వచితమైతే SQLITE_ENABLE_SQLLOG
  ఒకవేళ( sqlite3GlobalConfig.xSqllog ){
    /* Closing the handle. Fourth parameter is passed the value 2. */
    sqlite3GlobalConfig.xSqllog(sqlite3GlobalConfig.pSqllogArg, db, 0, 2);
  }
#అయితే_ముగింపు

  అయితే( db->p_Db_దత్తాంశం ){
    Db_Client_దత్తాంశం *p = db->p_Db_దత్తాంశం;
    db->p_Db_దత్తాంశం = p->p_తదుపరి;
    ప్రతిజ్ఞా( p->p_దత్తాంశం!=0 );
    ఒకవేళ( p->xDestructor ) p->xDestructor(p->p_దత్తాంశం);
    sqlite3_free(p);
  }

  /* Convert the connection into a zombie and then close it.
  */
  db->eOpenState = SQLITE_STATE_ZOMBIE;
  sqlite3LeaveMutexAndCloseZombie(db);
  తిరిగి SQLITE_OK;
}

/*
** Return the transaction state for a single databse, or the maximum
** transaction state over all attached databases if zSchema is null.
*/
పూర్ణాంకం sqlite3_txn_state(sqlite3 *db, స్థిరరాశి అక్షరం *zSchema){
  పూర్ణాంకం iDb, nDb;
  పూర్ణాంకం iTxn = -1;
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !sqlite3SafetyCheckOk(db) ){
    (శూన్య)SQLITE_MISUSE_BKPT;
    తిరిగి -1;
  }
#అయితే_ముగింపు
  sqlite3_mutex_enter(db->mutex);
  ఒకవేళ( zSchema ){
    nDb = iDb = sqlite3_కనుగొను_Db_Name(db, zSchema);
    ఒకవేళ( iDb<0 ) nDb--;
  }లేకపోతే{
    iDb = 0;
    nDb = db->nDb-1;
  }
  కొరకు(; iDb<=nDb; iDb++){
    Btree *pBt = db->aDb[iDb].pBt;
    పూర్ణాంకం x = pBt!=0 ? sqlite3BtreeTxnState(pBt) : SQLITE_TXN_NONE;
    ఒకవేళ( x>iTxn ) iTxn = x;
  }
  sqlite3_mutex_leave(db->mutex);
  తిరిగి iTxn;
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
పూర్ణాంకం sqlite3_close(sqlite3 *db){ తిరిగి sqlite3Close(db,0); }
పూర్ణాంకం sqlite3_close_v2(sqlite3 *db){ తిరిగి sqlite3Close(db,1); }


/*
** Close the mutex on database connection db.
**
** Furthermore, if database connection db is a zombie (meaning that there
** has been a prior call to sqlite3_close(db) or sqlite3_close_v2(db)) and
** every sqlite3_stmt has now been finalized and every sqlite3_backup has
** finished, then free all resources.
*/
శూన్య sqlite3LeaveMutexAndCloseZombie(sqlite3 *db){
  HashElem *i;                    /* Hash table iterator */
  పూర్ణాంకం j;

  /* If there are outstanding sqlite3_stmt or sqlite3_backup objects
  ** or if the connection has not yet been closed by sqlite3_close_v2(),
  ** then just leave the mutex and return.
  */
  ఒకవేళ( db->eOpenState!=SQLITE_STATE_ZOMBIE || connectionIsBusy(db) ){
    sqlite3_mutex_leave(db->mutex);
    తిరిగి;
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
  కొరకు(j=0; j<db->nDb; j++){
    నిర్మాణం Db *pDb = &db->aDb[j];
    ఒకవేళ( pDb->pBt ){
      sqlite3BtreeClose(pDb->pBt);
      pDb->pBt = 0;
      ఒకవేళ( j!=1 ){
        pDb->pSchema = 0;
      }
    }
  }
  /* Clear the TEMP schema separately and last */
  ఒకవేళ( db->aDb[1].pSchema ){
    sqlite3SchemaClear(db->aDb[1].pSchema);
    ప్రతిజ్ఞా( db->aDb[1].pSchema->trigHash.లెక్క==0 );
  }
  sqlite3VtabUnlockList(db);

  /* Free up the array of auxiliary databases */
  sqlite3CollapseDatabaseArray(db);
  ప్రతిజ్ఞా( db->nDb<=2 );
  ప్రతిజ్ఞా( db->aDb==db->aDbStatic );

  /* Tell the code in notify.c that the connection no longer holds any
  ** locks and does not require any further unlock-notify callbacks.
  */
  sqlite3ConnectionClosed(db);

  కొరకు(i=sqlite_Hash_మొదటి(&db->aFunc); i; i=sqlite_Hash_తదుపరి(i)){
    FuncDef *p_తదుపరి, *p;
    p = sqlite_Hash_దత్తాంశం(i);
    చేయి{
      functionDestroy(db, p);
      p_తదుపరి = p->p_తదుపరి;
      sqlite3DbFree(db, p);
      p = p_తదుపరి;
    }అయితే( p );
  }
  sqlite3HashClear(&db->aFunc);
  కొరకు(i=sqlite_Hash_మొదటి(&db->aCollSeq); i; i=sqlite_Hash_తదుపరి(i)){
    CollSeq *pColl = (CollSeq *)sqlite_Hash_దత్తాంశం(i);
    /* Invoke any destructors registered for collation sequence user data. */
    కొరకు(j=0; j<3; j++){
      ఒకవేళ( pColl[j].xDel ){
        pColl[j].xDel(pColl[j].pUser);
      }
    }
    sqlite3DbFree(db, pColl);
  }
  sqlite3HashClear(&db->aCollSeq);
#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
  కొరకు(i=sqlite_Hash_మొదటి(&db->aModule); i; i=sqlite_Hash_తదుపరి(i)){
    Module *pMod = (Module *)sqlite_Hash_దత్తాంశం(i);
    sqlite3_Vtab_Eponymous_పట్టిక_Clear(db, pMod);
    sqlite3VtabModuleUnref(db, pMod);
  }
  sqlite3HashClear(&db->aModule);
#అయితే_ముగింపు

  sqlite3Error(db, SQLITE_OK); /* Deallocates any cached error strings. */
  sqlite3_విలువ_Free(db->pErr);
  sqlite3CloseExtensions(db);

  db->eOpenState = SQLITE_STATE_ERROR;

  /* The temp-database schema is allocated differently from the other schema
  ** objects (using sqliteMalloc() directly, instead of sqlite3BtreeSchema()).
  ** So it needs to be freed here. Todo: Why not roll the temp schema into
  ** the same sqliteMalloc() as the one that allocates the database
  ** structure?
  */
  sqlite3DbFree(db, db->aDb[1].pSchema);
  ఒకవేళ( db->xAutovacDestr ){
    db->xAutovacDestr(db->p_Autovac_పుటలు_Arg);
  }
  sqlite3_mutex_leave(db->mutex);
  db->eOpenState = SQLITE_STATE_CLOSED;
  sqlite3_mutex_free(db->mutex);
  ప్రతిజ్ఞా( sqlite3_Lookaside_వాడిన(db,0)==0 );
  ఒకవేళ( db->lookaside.bMalloced ){
    sqlite3_free(db->lookaside.p_ప్రారంభం);
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
శూన్య sqlite3RollbackAll(sqlite3 *db, పూర్ణాంకం trip_సంకేతం){
  పూర్ణాంకం i;
  పూర్ణాంకం లోపల_Trans = 0;
  పూర్ణాంకం schemaChange;
  ప్రతిజ్ఞా( sqlite3_mutex_held(db->mutex) );
  sqlite3BeginBenignMalloc();

  /* Obtain all b-tree mutexes before making any calls to BtreeRollback().
  ** This is important in case the transaction being rolled back has
  ** modified the database schema. If the b-tree mutexes are not taken
  ** here, then another shared-cache connection might sneak in between
  ** the database rollback and schema reset, which can cause false
  ** corruption reports in some cases.  */
  sqlite3BtreeEnterAll(db);
  schemaChange = (db->mDbFlags & DBFLAG_SchemaChange)!=0 && db->init.busy==0;

  కొరకు(i=0; i<db->nDb; i++){
    Btree *p = db->aDb[i].pBt;
    ఒకవేళ( p ){
      ఒకవేళ( sqlite3BtreeTxnState(p)==SQLITE_TXN_WRITE ){
        లోపల_Trans = 1;
      }
      sqlite3BtreeRollback(p, trip_సంకేతం, !schemaChange);
    }
  }
  sqlite3VtabRollback(db);
  sqlite3_చివర_Benign_Malloc();

  ఒకవేళ( schemaChange ){
    sqlite3ExpirePreparedStatements(db, 0);
    sqlite3_Reset_All_Schemas_యొక్క_Connection(db);
  }
  sqlite3BtreeLeaveAll(db);

  /* Any deferred constraint violations have now been resolved. */
  db->nDeferredCons = 0;
  db->nDeferredImmCons = 0;
  db->flags &= ~(u64)(SQLITE_DeferFKs|SQLITE_CorruptRdOnly);

  /* If one has been configured, invoke the rollback-hook callback */
  ఒకవేళ( db->xRollbackCallback && (లోపల_Trans || !db->autoCommit) ){
    db->xRollbackCallback(db->pRollbackArg);
  }
}

/*
** Return a static string containing the name corresponding to the error code
** specified in the argument.
*/
#ఒకవేళ defined(SQLITE_NEED_ERR_NAME)
స్థిరరాశి అక్షరం *sqlite3ErrName(పూర్ణాంకం rc){
  స్థిరరాశి అక్షరం *zName = 0;
  పూర్ణాంకం i, origRc = rc;
  కొరకు(i=0; i<2 && zName==0; i++, rc &= 0xff){
    ఎంపిక( rc ){
      స్థితి SQLITE_OK:                 zName = "SQLITE_OK";                ఆపు;
      స్థితి SQLITE_ERROR:              zName = "SQLITE_ERROR";             ఆపు;
      స్థితి SQLITE_ERROR_SNAPSHOT:     zName = "SQLITE_ERROR_SNAPSHOT";    ఆపు;
      స్థితి SQLITE_ERROR_RETRY:        zName = "SQLITE_ERROR_RETRY";       ఆపు;
      స్థితి SQLITE_ERROR_MISSING_COLLSEQ:
                                zName = "SQLITE_ERROR_MISSING_COLLSEQ";   ఆపు;
      స్థితి SQLITE_అంతర్గత:           zName = "SQLITE_INTERNAL";          ఆపు;
      స్థితి SQLITE_PERM:               zName = "SQLITE_PERM";              ఆపు;
      స్థితి SQLITE_ABORT:              zName = "SQLITE_ABORT";             ఆపు;
      స్థితి SQLITE_ABORT_ROLLBACK:     zName = "SQLITE_ABORT_ROLLBACK";    ఆపు;
      స్థితి SQLITE_BUSY:               zName = "SQLITE_BUSY";              ఆపు;
      స్థితి SQLITE_BUSY_RECOVERY:      zName = "SQLITE_BUSY_RECOVERY";     ఆపు;
      స్థితి SQLITE_BUSY_SNAPSHOT:      zName = "SQLITE_BUSY_SNAPSHOT";     ఆపు;
      స్థితి SQLITE_LOCKED:             zName = "SQLITE_LOCKED";            ఆపు;
      స్థితి SQLITE_LOCKED_SHAREDCACHE: zName = "SQLITE_LOCKED_SHAREDCACHE";ఆపు;
      స్థితి SQLITE_NOMEM:              zName = "SQLITE_NOMEM";             ఆపు;
      స్థితి SQLITE_READONLY:           zName = "SQLITE_READONLY";          ఆపు;
      స్థితి SQLITE_READONLY_RECOVERY:  zName = "SQLITE_READONLY_RECOVERY"; ఆపు;
      స్థితి SQLITE_READONLY_CANTINIT:  zName = "SQLITE_READONLY_CANTINIT"; ఆపు;
      స్థితి SQLITE_READONLY_ROLLBACK:  zName = "SQLITE_READONLY_ROLLBACK"; ఆపు;
      స్థితి SQLITE_READONLY_DBMOVED:   zName = "SQLITE_READONLY_DBMOVED";  ఆపు;
      స్థితి SQLITE_READONLY_DIRECTORY: zName = "SQLITE_READONLY_DIRECTORY";ఆపు;
      స్థితి SQLITE_అంతరాయం:          zName = "SQLITE_INTERRUPT";         ఆపు;
      స్థితి SQLITE_IOERR:              zName = "SQLITE_IOERR";             ఆపు;
      స్థితి SQLITE_IOERR_READ:         zName = "SQLITE_IOERR_READ";        ఆపు;
      స్థితి SQLITE_IOERR_SHORT_READ:   zName = "SQLITE_IOERR_SHORT_READ";  ఆపు;
      స్థితి SQLITE_IOERR_WRITE:        zName = "SQLITE_IOERR_WRITE";       ఆపు;
      స్థితి SQLITE_IOERR_FSYNC:        zName = "SQLITE_IOERR_FSYNC";       ఆపు;
      స్థితి SQLITE_IOERR_DIR_FSYNC:    zName = "SQLITE_IOERR_DIR_FSYNC";   ఆపు;
      స్థితి SQLITE_IOERR_TRUNCATE:     zName = "SQLITE_IOERR_TRUNCATE";    ఆపు;
      స్థితి SQLITE_IOERR_FSTAT:        zName = "SQLITE_IOERR_FSTAT";       ఆపు;
      స్థితి SQLITE_IOERR_UNLOCK:       zName = "SQLITE_IOERR_UNLOCK";      ఆపు;
      స్థితి SQLITE_IOERR_RDLOCK:       zName = "SQLITE_IOERR_RDLOCK";      ఆపు;
      స్థితి SQLITE_IOERR_DELETE:       zName = "SQLITE_IOERR_DELETE";      ఆపు;
      స్థితి SQLITE_IOERR_NOMEM:        zName = "SQLITE_IOERR_NOMEM";       ఆపు;
      స్థితి SQLITE_IOERR_ACCESS:       zName = "SQLITE_IOERR_ACCESS";      ఆపు;
      స్థితి SQLITE_IOERR_CHECKRESERVEDLOCK:
                                zName = "SQLITE_IOERR_CHECKRESERVEDLOCK"; ఆపు;
      స్థితి SQLITE_IOERR_తాళం:         zName = "SQLITE_IOERR_LOCK";        ఆపు;
      స్థితి SQLITE_IOERR_CLOSE:        zName = "SQLITE_IOERR_CLOSE";       ఆపు;
      స్థితి SQLITE_IOERR_DIR_CLOSE:    zName = "SQLITE_IOERR_DIR_CLOSE";   ఆపు;
      స్థితి SQLITE_IOERR_SHMOPEN:      zName = "SQLITE_IOERR_SHMOPEN";     ఆపు;
      స్థితి SQLITE_IOERR_SHMSIZE:      zName = "SQLITE_IOERR_SHMSIZE";     ఆపు;
      స్థితి SQLITE_IOERR_SHMLOCK:      zName = "SQLITE_IOERR_SHMLOCK";     ఆపు;
      స్థితి SQLITE_IOERR_SHMMAP:       zName = "SQLITE_IOERR_SHMMAP";      ఆపు;
      స్థితి SQLITE_IOERR_SEEK:         zName = "SQLITE_IOERR_SEEK";        ఆపు;
      స్థితి SQLITE_IOERR_DELETE_NOENT: zName = "SQLITE_IOERR_DELETE_NOENT";ఆపు;
      స్థితి SQLITE_IOERR_MMAP:         zName = "SQLITE_IOERR_MMAP";        ఆపు;
      స్థితి SQLITE_IOERR_GETTEMPPATH:  zName = "SQLITE_IOERR_GETTEMPPATH"; ఆపు;
      స్థితి SQLITE_IOERR_CONVPATH:     zName = "SQLITE_IOERR_CONVPATH";    ఆపు;
      స్థితి SQLITE_CORRUPT:            zName = "SQLITE_CORRUPT";           ఆపు;
      స్థితి SQLITE_CORRUPT_VTAB:       zName = "SQLITE_CORRUPT_VTAB";      ఆపు;
      స్థితి SQLITE_NOTFOUND:           zName = "SQLITE_NOTFOUND";          ఆపు;
      స్థితి SQLITE_నిండిన:               zName = "SQLITE_FULL";              ఆపు;
      స్థితి SQLITE_CANTOPEN:           zName = "SQLITE_CANTOPEN";          ఆపు;
      స్థితి SQLITE_CANTOPEN_NOTEMPDIR: zName = "SQLITE_CANTOPEN_NOTEMPDIR";ఆపు;
      స్థితి SQLITE_CANTOPEN_ISDIR:     zName = "SQLITE_CANTOPEN_ISDIR";    ఆపు;
      స్థితి SQLITE_CANTOPEN_FULLPATH:  zName = "SQLITE_CANTOPEN_FULLPATH"; ఆపు;
      స్థితి SQLITE_CANTOPEN_CONVPATH:  zName = "SQLITE_CANTOPEN_CONVPATH"; ఆపు;
      స్థితి SQLITE_CANTOPEN_SYMLINK:   zName = "SQLITE_CANTOPEN_SYMLINK";  ఆపు;
      స్థితి SQLITE_PROTOCOL:           zName = "SQLITE_PROTOCOL";          ఆపు;
      స్థితి SQLITE_ఖాళీలు:              zName = "SQLITE_EMPTY";             ఆపు;
      స్థితి SQLITE_SCHEMA:             zName = "SQLITE_SCHEMA";            ఆపు;
      స్థితి SQLITE_TOOBIG:             zName = "SQLITE_TOOBIG";            ఆపు;
      స్థితి SQLITE_CONSTRAINT:         zName = "SQLITE_CONSTRAINT";        ఆపు;
      స్థితి SQLITE_CONSTRAINT_UNIQUE:  zName = "SQLITE_CONSTRAINT_UNIQUE"; ఆపు;
      స్థితి SQLITE_CONSTRAINT_TRIGGER: zName = "SQLITE_CONSTRAINT_TRIGGER";ఆపు;
      స్థితి SQLITE_CONSTRAINT_FOREIGNKEY:
                                zName = "SQLITE_CONSTRAINT_FOREIGNKEY";   ఆపు;
      స్థితి SQLITE_CONSTRAINT_CHECK:   zName = "SQLITE_CONSTRAINT_CHECK";  ఆపు;
      స్థితి SQLITE_CONSTRAINT_PRIMARYKEY:
                                zName = "SQLITE_CONSTRAINT_PRIMARYKEY";   ఆపు;
      స్థితి SQLITE_CONSTRAINT_NOTNULL: zName = "SQLITE_CONSTRAINT_NOTNULL";ఆపు;
      స్థితి SQLITE_CONSTRAINT_COMMITHOOK:
                                zName = "SQLITE_CONSTRAINT_COMMITHOOK";   ఆపు;
      స్థితి SQLITE_CONSTRAINT_VTAB:    zName = "SQLITE_CONSTRAINT_VTAB";   ఆపు;
      స్థితి SQLITE_CONSTRAINT_FUNCTION:
                                zName = "SQLITE_CONSTRAINT_FUNCTION";     ఆపు;
      స్థితి SQLITE_CONSTRAINT_ROWID:   zName = "SQLITE_CONSTRAINT_ROWID";  ఆపు;
      స్థితి SQLITE_MISMATCH:           zName = "SQLITE_MISMATCH";          ఆపు;
      స్థితి SQLITE_MISUSE:             zName = "SQLITE_MISUSE";            ఆపు;
      స్థితి SQLITE_NOLFS:              zName = "SQLITE_NOLFS";             ఆపు;
      స్థితి SQLITE_AUTH:               zName = "SQLITE_AUTH";              ఆపు;
      స్థితి SQLITE_FORMAT:             zName = "SQLITE_FORMAT";            ఆపు;
      స్థితి SQLITE_RANGE:              zName = "SQLITE_RANGE";             ఆపు;
      స్థితి SQLITE_NOTADB:             zName = "SQLITE_NOTADB";            ఆపు;
      స్థితి SQLITE_అడ్డువరుస:                zName = "SQLITE_ROW";               ఆపు;
      స్థితి SQLITE_NOTICE:             zName = "SQLITE_NOTICE";            ఆపు;
      స్థితి SQLITE_NOTICE_RECOVER_WAL: zName = "SQLITE_NOTICE_RECOVER_WAL";ఆపు;
      స్థితి SQLITE_NOTICE_RECOVER_ROLLBACK:
                                zName = "SQLITE_NOTICE_RECOVER_ROLLBACK"; ఆపు;
      స్థితి SQLITE_NOTICE_RBU:         zName = "SQLITE_NOTICE_RBU"; ఆపు;
      స్థితి SQLITE_WARNING:            zName = "SQLITE_WARNING";           ఆపు;
      స్థితి SQLITE_WARNING_AUTOINDEX:  zName = "SQLITE_WARNING_AUTOINDEX"; ఆపు;
      స్థితి SQLITE_పూర్తి:               zName = "SQLITE_DONE";              ఆపు;
    }
  }
  ఒకవేళ( zName==0 ){
    స్థిర అక్షరం zBuf[50];
    sqlite3_snprintf(పరిమాణం(zBuf), zBuf, "SQLITE_UNKNOWN(%d)", origRc);
    zName = zBuf;
  }
  తిరిగి zName;
}
#అయితే_ముగింపు

/*
** Return a static string that describes the kind of error specified in the
** argument.
*/
స్థిరరాశి అక్షరం *sqlite3ErrStr(పూర్ణాంకం rc){
  స్థిర స్థిరరాశి అక్షరం* స్థిరరాశి a_సందేశం[] = {
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
#నిర్వచితమైతే SQLITE_DISABLE_LFS
    /* SQLITE_NOLFS       */ "large file support is disabled",
#లేకపోతే
    /* SQLITE_NOLFS       */ 0,
#అయితే_ముగింపు
    /* SQLITE_AUTH        */ "authorization denied",
    /* SQLITE_FORMAT      */ 0,
    /* SQLITE_RANGE       */ "column index out of range",
    /* SQLITE_NOTADB      */ "file is not a database",
    /* SQLITE_NOTICE      */ "notification message",
    /* SQLITE_WARNING     */ "warning message",
  };
  స్థిరరాశి అక్షరం *zErr = "unknown error";
  ఎంపిక( rc ){
    స్థితి SQLITE_ABORT_ROLLBACK: {
      zErr = "abort due to ROLLBACK";
      ఆపు;
    }
    స్థితి SQLITE_అడ్డువరుస: {
      zErr = "another row available";
      ఆపు;
    }
    స్థితి SQLITE_పూర్తి: {
      zErr = "no more rows available";
      ఆపు;
    }
    అప్రమేయ: {
      rc &= 0xff;
      ఒకవేళ( ALWAYS(rc>=0) && rc<Array_కొలత(a_సందేశం) && a_సందేశం[rc]!=0 ){
        zErr = a_సందేశం[rc];
      }
      ఆపు;
    }
  }
  తిరిగి zErr;
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
స్థిర పూర్ణాంకం sqliteDefaultBusyCallback(
  శూన్య *ptr,               /* Database connection */
  పూర్ణాంకం లెక్క                /* Number of times table has been busy */
){
#ఒకవేళ SQLITE_OS_WIN || !defined(HAVE_NANOSLEEP) || HAVE_NANOSLEEP
  /* This case is for systems that have support for sleeping for fractions of
  ** a second.  Examples:  All windows systems, unix systems with nanosleep() */
  స్థిర స్థిరరాశి u8 delays[] =
     { 1, 2, 5, 10, 15, 20, 25, 25,  25,  50,  50, 100 };
  స్థిర స్థిరరాశి u8 totals[] =
     { 0, 1, 3,  8, 18, 33, 53, 78, 103, 128, 178, 228 };
# define NDELAY Array_కొలత(delays)
  sqlite3 *db = (sqlite3 *)ptr;
  పూర్ణాంకం tmout = db->busyTimeout;
  పూర్ణాంకం delay, పూర్వ;

  ప్రతిజ్ఞా( లెక్క>=0 );
  ఒకవేళ( లెక్క < NDELAY ){
    delay = delays[లెక్క];
    పూర్వ = totals[లెక్క];
  }లేకపోతే{
    delay = delays[NDELAY-1];
    పూర్వ = totals[NDELAY-1] + delay*(లెక్క-(NDELAY-1));
  }
  ఒకవేళ( పూర్వ + delay > tmout ){
    delay = tmout - పూర్వ;
    ఒకవేళ( delay<=0 ) తిరిగి 0;
  }
  sqlite3OsSleep(db->pVfs, delay*1000);
  తిరిగి 1;
#లేకపోతే
  /* This case for unix systems that lack usleep() support.  Sleeping
  ** must be done in increments of whole seconds */
  sqlite3 *db = (sqlite3 *)ptr;
  పూర్ణాంకం tmout = ((sqlite3 *)ptr)->busyTimeout;
  ఒకవేళ( (లెక్క+1)*1000 > tmout ){
    తిరిగి 0;
  }
  sqlite3OsSleep(db->pVfs, 1000000);
  తిరిగి 1;
#అయితే_ముగింపు
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
పూర్ణాంకం sqlite3InvokeBusyHandler(BusyHandler *p){
  పూర్ణాంకం rc;
  ఒకవేళ( p->xBusyHandler==0 || p->nBusy<0 ) తిరిగి 0;
  rc = p->xBusyHandler(p->pBusyArg, p->nBusy);
  ఒకవేళ( rc==0 ){
    p->nBusy = -1;
  }లేకపోతే{
    p->nBusy++;
  }
  తిరిగి rc;
}

/*
** This routine sets the busy callback for an Sqlite database to the
** given callback function with the given argument.
*/
పూర్ణాంకం sqlite3_busy_handler(
  sqlite3 *db,
  పూర్ణాంకం (*xBusy)(శూన్య*,పూర్ణాంకం),
  శూన్య *pArg
){
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !sqlite3SafetyCheckOk(db) ) తిరిగి SQLITE_MISUSE_BKPT;
#అయితే_ముగింపు
  sqlite3_mutex_enter(db->mutex);
  db->busyHandler.xBusyHandler = xBusy;
  db->busyHandler.pBusyArg = pArg;
  db->busyHandler.nBusy = 0;
  db->busyTimeout = 0;
#నిర్వచితమైతే SQLITE_ENABLE_SETLK_TIMEOUT
  db->setlkTimeout = 0;
#అయితే_ముగింపు
  sqlite3_mutex_leave(db->mutex);
  తిరిగి SQLITE_OK;
}

#నిర్వచితం_కాకపోతే SQLITE_OMIT_పురోగతి_CALLBACK
/*
** This routine sets the progress callback for an Sqlite database to the
** given callback function with the given argument. The progress callback will
** be invoked every nOps opcodes.
*/
శూన్య sqlite3_పురోగతి_handler(
  sqlite3 *db,
  పూర్ణాంకం nOps,
  పూర్ణాంకం (*x_పురోగతి)(శూన్య*),
  శూన్య *pArg
){
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !sqlite3SafetyCheckOk(db) ){
    (శూన్య)SQLITE_MISUSE_BKPT;
    తిరిగి;
  }
#అయితే_ముగింపు
  sqlite3_mutex_enter(db->mutex);
  ఒకవేళ( nOps>0 ){
    db->x_పురోగతి = x_పురోగతి;
    db->n_పురోగతి_Ops = (గుర్తులేని)nOps;
    db->p_పురోగతి_Arg = pArg;
  }లేకపోతే{
    db->x_పురోగతి = 0;
    db->n_పురోగతి_Ops = 0;
    db->p_పురోగతి_Arg = 0;
  }
  sqlite3_mutex_leave(db->mutex);
}
#అయితే_ముగింపు


/*
** This routine installs a default busy handler that waits for the
** specified number of milliseconds before returning 0.
*/
పూర్ణాంకం sqlite3_busy_timeout(sqlite3 *db, పూర్ణాంకం ms){
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !sqlite3SafetyCheckOk(db) ) తిరిగి SQLITE_MISUSE_BKPT;
#అయితే_ముగింపు
  sqlite3_mutex_enter(db->mutex);
  ఒకవేళ( ms>0 ){
    sqlite3_busy_handler(db, (పూర్ణాంకం(*)(శూన్య*,పూర్ణాంకం))sqliteDefaultBusyCallback,
                             (శూన్య*)db);
    db->busyTimeout = ms;
#నిర్వచితమైతే SQLITE_ENABLE_SETLK_TIMEOUT
    db->setlkTimeout = ms;
#అయితే_ముగింపు
  }లేకపోతే{
    sqlite3_busy_handler(db, 0, 0);
  }
  sqlite3_mutex_leave(db->mutex);
  తిరిగి SQLITE_OK;
}

/*
** Set the setlk timeout value.
*/
పూర్ణాంకం sqlite3_setlk_timeout(sqlite3 *db, పూర్ణాంకం ms, పూర్ణాంకం flags){
#నిర్వచితమైతే SQLITE_ENABLE_SETLK_TIMEOUT
  పూర్ణాంకం iDb;
  పూర్ణాంకం bBOC = ((flags & SQLITE_SETLK_ఖండం_ON_CONNECT) ? 1 : 0);
#అయితే_ముగింపు
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !sqlite3SafetyCheckOk(db) ) తిరిగి SQLITE_MISUSE_BKPT;
#అయితే_ముగింపు
  ఒకవేళ( ms<-1 ) తిరిగి SQLITE_RANGE;
#నిర్వచితమైతే SQLITE_ENABLE_SETLK_TIMEOUT
  sqlite3_mutex_enter(db->mutex);
  db->setlkTimeout = ms;
  db->setlkFlags = flags;
  sqlite3BtreeEnterAll(db);
  కొరకు(iDb=0; iDb<db->nDb; iDb++){
    Btree *pBt = db->aDb[iDb].pBt;
    ఒకవేళ( pBt ){
      sqlite3_file *fd = sqlite3PagerFile(sqlite3BtreePager(pBt));
      sqlite3OsFileControlHint(fd, SQLITE_FCNTL_ఖండం_ON_CONNECT, (శూన్య*)&bBOC);
    }
  }
  sqlite3BtreeLeaveAll(db);
  sqlite3_mutex_leave(db->mutex);
#అయితే_ముగింపు
#ఒకవేళ !defined(SQLITE_ENABLE_API_ARMOR) && !defined(SQLITE_ENABLE_SETLK_TIMEOUT)
  వాడని_PARAMETER(db);
  వాడని_PARAMETER(flags);
#అయితే_ముగింపు
  తిరిగి SQLITE_OK;
}

/*
** Cause any pending operation to stop at its earliest opportunity.
*/
శూన్య sqlite3_అంతరాయం(sqlite3 *db){
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !sqlite3SafetyCheckOk(db)
   && (db==0 || db->eOpenState!=SQLITE_STATE_ZOMBIE)
  ){
    (శూన్య)SQLITE_MISUSE_BKPT;
    తిరిగి;
  }
#అయితే_ముగింపు
  AtomicStore(&db->u1.isInterrupted, 1);
}

/*
** Return true or false depending on whether or not an interrupt is
** pending on connection db.
*/
పూర్ణాంకం sqlite3_is_interrupted(sqlite3 *db){
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !sqlite3SafetyCheckOk(db)
   && (db==0 || db->eOpenState!=SQLITE_STATE_ZOMBIE)
  ){
    (శూన్య)SQLITE_MISUSE_BKPT;
    తిరిగి 0;
  }
#అయితే_ముగింపు
  తిరిగి Atomic_ఎక్కించు(&db->u1.isInterrupted)!=0;
}

/*
** This function is exactly the same as sqlite3_create_function(), except
** that it is designed to be called by internal code. The difference is
** that if a malloc() fails in sqlite3_create_function(), an error code
** is returned and the mallocFailed flag cleared.
*/
పూర్ణాంకం sqlite3CreateFunc(
  sqlite3 *db,
  స్థిరరాశి అక్షరం *zFunctionName,
  పూర్ణాంకం nArg,
  పూర్ణాంకం enc,
  శూన్య *p_User_దత్తాంశం,
  శూన్య (*xSFunc)(sqlite3_context*,పూర్ణాంకం,sqlite3_విలువ **),
  శూన్య (*x_అడుగు)(sqlite3_context*,పూర్ణాంకం,sqlite3_విలువ **),
  శూన్య (*xFinal)(sqlite3_context*),
  శూన్య (*x_విలువ)(sqlite3_context*),
  శూన్య (*xInverse)(sqlite3_context*,పూర్ణాంకం,sqlite3_విలువ **),
  FuncDestructor *pDestructor
){
  FuncDef *p;
  పూర్ణాంకం extraFlags;

  ప్రతిజ్ఞా( sqlite3_mutex_held(db->mutex) );
  ప్రతిజ్ఞా( x_విలువ==0 || xSFunc==0 );
  ఒకవేళ( zFunctionName==0                /* Must have a valid name */
   || (xSFunc!=0 && xFinal!=0)        /* Not both xSFunc and xFinal */
   || ((xFinal==0)!=(x_అడుగు==0))       /* Both or neither of xFinal and xStep */
   || ((x_విలువ==0)!=(xInverse==0))    /* Both or neither of xValue, xInverse */
   || (nArg<-1 || nArg>SQLITE_MAX_FUNCTION_ARG)
   || (255<sqlite3Strlen30(zFunctionName))
  ){
    తిరిగి SQLITE_MISUSE_BKPT;
  }

  ప్రతిజ్ఞా( SQLITE_FUNC_CONSTANT==SQLITE_DETERMINISTIC );
  ప్రతిజ్ఞా( SQLITE_FUNC_ప్రత్యక్ష==SQLITE_DIRECTONLY );
  extraFlags = enc &  (SQLITE_DETERMINISTIC|SQLITE_DIRECTONLY|
                       SQLITE_SUBTYPE|SQLITE_INNOCUOUS|
                       SQLITE_ఫలితం_SUBTYPE|SQLITE_SELFORDER1);
  enc &= (SQLITE_FUNC_ENCMASK|SQLITE_ANY);

  /* The SQLITE_INNOCUOUS flag is the same bit as SQLITE_FUNC_UNSAFE.  But
  ** the meaning is inverted.  So flip the bit. */
  ప్రతిజ్ఞా( SQLITE_FUNC_UNSAFE==SQLITE_INNOCUOUS );
  extraFlags ^= SQLITE_FUNC_UNSAFE;  /* tag-20230109-1 */

 
#నిర్వచితం_కాకపోతే SQLITE_OMIT_UTF16
  /* If SQLITE_UTF16 is specified as the encoding type, transform this
  ** to one of SQLITE_UTF16LE or SQLITE_UTF16BE using the
  ** SQLITE_UTF16NATIVE macro. SQLITE_UTF16 is not used internally.
  **
  ** If SQLITE_ANY is specified, add three versions of the function
  ** to the hash table.
  */
  ఎంపిక( enc ){
    స్థితి SQLITE_UTF16:
      enc = SQLITE_UTF16NATIVE;
      ఆపు;
    స్థితి SQLITE_ANY: {
      పూర్ణాంకం rc;
      rc = sqlite3CreateFunc(db, zFunctionName, nArg,
           (SQLITE_UTF8|extraFlags)^SQLITE_FUNC_UNSAFE, /* tag-20230109-1 */
           p_User_దత్తాంశం, xSFunc, x_అడుగు, xFinal, x_విలువ, xInverse, pDestructor);
      ఒకవేళ( rc==SQLITE_OK ){
        rc = sqlite3CreateFunc(db, zFunctionName, nArg,
             (SQLITE_UTF16LE|extraFlags)^SQLITE_FUNC_UNSAFE, /* tag-20230109-1*/
             p_User_దత్తాంశం, xSFunc, x_అడుగు, xFinal, x_విలువ, xInverse, pDestructor);
      }
      ఒకవేళ( rc!=SQLITE_OK ){
        తిరిగి rc;
      }
      enc = SQLITE_UTF16BE;
      ఆపు;
    }
    స్థితి SQLITE_UTF8:
    స్థితి SQLITE_UTF16LE:
    స్థితి SQLITE_UTF16BE:
      ఆపు;
    అప్రమేయ:
      enc = SQLITE_UTF8;
      ఆపు;
  }
#లేకపోతే
  enc = SQLITE_UTF8;
#అయితే_ముగింపు
 
  /* Check if an existing function is being overridden or deleted. If so,
  ** and there are active VMs, then return SQLITE_BUSY. If a function
  ** is being overridden/deleted but there are no active VMs, allow the
  ** operation to continue but invalidate all precompiled statements.
  */
  p = sqlite3_కనుగొను_Function(db, zFunctionName, nArg, (u8)enc, 0);
  ఒకవేళ( p && (p->funcFlags & SQLITE_FUNC_ENCMASK)==(u32)enc && p->nArg==nArg ){
    ఒకవేళ( db->n_Vdbe_చురుకైన ){
      sqlite3_Error_With_సందేశం(db, SQLITE_BUSY,
        "unable to delete/modify user-function due to active statements");
      ప్రతిజ్ఞా( !db->mallocFailed || db->nVdbeExec>0 );
      తిరిగి SQLITE_BUSY;
    }లేకపోతే{
      sqlite3ExpirePreparedStatements(db, 0);
    }
  }లేకపోతే ఒకవేళ( xSFunc==0 && xFinal==0 ){
    /* Trying to delete a function that does not exist.  This is a no-op.
    ** https://sqlite.org/forum/forumpost/726219164b */
    తిరిగి SQLITE_OK;
  }

  p = sqlite3_కనుగొను_Function(db, zFunctionName, nArg, (u8)enc, 1);
  ప్రతిజ్ఞా(p || db->mallocFailed);
  ఒకవేళ( !p ){
    తిరిగి SQLITE_NOMEM_BKPT;
  }

  /* If an older version of the function with a configured destructor is
  ** being replaced invoke the destructor function here. */
  functionDestroy(db, p);

  ఒకవేళ( pDestructor ){
    pDestructor->nRef++;
  }
  p->u.pDestructor = pDestructor;
  p->funcFlags = (p->funcFlags & SQLITE_FUNC_ENCMASK) | extraFlags;
  testcase( p->funcFlags & SQLITE_DETERMINISTIC );
  testcase( p->funcFlags & SQLITE_DIRECTONLY );
  p->xSFunc = xSFunc ? xSFunc : x_అడుగు;
  p->xFinalize = xFinal;
  p->x_విలువ = x_విలువ;
  p->xInverse = xInverse;
  p->p_User_దత్తాంశం = p_User_దత్తాంశం;
  p->nArg = (u16)nArg;
  తిరిగి SQLITE_OK;
}

/*
** Worker function used by utf-8 APIs that create new functions:
**
**    sqlite3_create_function()
**    sqlite3_create_function_v2()
**    sqlite3_create_window_function()
*/
స్థిర పూర్ణాంకం createFunctionApi(
  sqlite3 *db,
  స్థిరరాశి అక్షరం *zFunc,
  పూర్ణాంకం nArg,
  పూర్ణాంకం enc,
  శూన్య *p,
  శూన్య (*xSFunc)(sqlite3_context*,పూర్ణాంకం,sqlite3_విలువ**),
  శూన్య (*x_అడుగు)(sqlite3_context*,పూర్ణాంకం,sqlite3_విలువ**),
  శూన్య (*xFinal)(sqlite3_context*),
  శూన్య (*x_విలువ)(sqlite3_context*),
  శూన్య (*xInverse)(sqlite3_context*,పూర్ణాంకం,sqlite3_విలువ**),
  శూన్య(*xDestroy)(శూన్య*)
){
  పూర్ణాంకం rc = SQLITE_ERROR;
  FuncDestructor *pArg = 0;

#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !sqlite3SafetyCheckOk(db) ){
    తిరిగి SQLITE_MISUSE_BKPT;
  }
#అయితే_ముగింపు
  sqlite3_mutex_enter(db->mutex);
  ఒకవేళ( xDestroy ){
    pArg = (FuncDestructor *)sqlite3Malloc(పరిమాణం(FuncDestructor));
    ఒకవేళ( !pArg ){
      sqlite3OomFault(db);
      xDestroy(p);
      వెళ్ళు బహిర్గతం;
    }
    pArg->nRef = 0;
    pArg->xDestroy = xDestroy;
    pArg->p_User_దత్తాంశం = p;
  }
  rc = sqlite3CreateFunc(db, zFunc, nArg, enc, p,
      xSFunc, x_అడుగు, xFinal, x_విలువ, xInverse, pArg
  );
  ఒకవేళ( pArg && pArg->nRef==0 ){
    ప్రతిజ్ఞా( rc!=SQLITE_OK || (x_అడుగు==0 && xFinal==0) );
    xDestroy(p);
    sqlite3_free(pArg);
  }

 బహిర్గతం:
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  తిరిగి rc;
}

/*
** Create new user functions.
*/
పూర్ణాంకం sqlite3_create_function(
  sqlite3 *db,
  స్థిరరాశి అక్షరం *zFunc,
  పూర్ణాంకం nArg,
  పూర్ణాంకం enc,
  శూన్య *p,
  శూన్య (*xSFunc)(sqlite3_context*,పూర్ణాంకం,sqlite3_విలువ **),
  శూన్య (*x_అడుగు)(sqlite3_context*,పూర్ణాంకం,sqlite3_విలువ **),
  శూన్య (*xFinal)(sqlite3_context*)
){
  తిరిగి createFunctionApi(db, zFunc, nArg, enc, p, xSFunc, x_అడుగు,
                                    xFinal, 0, 0, 0);
}
పూర్ణాంకం sqlite3_create_function_v2(
  sqlite3 *db,
  స్థిరరాశి అక్షరం *zFunc,
  పూర్ణాంకం nArg,
  పూర్ణాంకం enc,
  శూన్య *p,
  శూన్య (*xSFunc)(sqlite3_context*,పూర్ణాంకం,sqlite3_విలువ **),
  శూన్య (*x_అడుగు)(sqlite3_context*,పూర్ణాంకం,sqlite3_విలువ **),
  శూన్య (*xFinal)(sqlite3_context*),
  శూన్య (*xDestroy)(శూన్య *)
){
  తిరిగి createFunctionApi(db, zFunc, nArg, enc, p, xSFunc, x_అడుగు,
                                    xFinal, 0, 0, xDestroy);
}
పూర్ణాంకం sqlite3_create_కిటికీ_function(
  sqlite3 *db,
  స్థిరరాశి అక్షరం *zFunc,
  పూర్ణాంకం nArg,
  పూర్ణాంకం enc,
  శూన్య *p,
  శూన్య (*x_అడుగు)(sqlite3_context*,పూర్ణాంకం,sqlite3_విలువ **),
  శూన్య (*xFinal)(sqlite3_context*),
  శూన్య (*x_విలువ)(sqlite3_context*),
  శూన్య (*xInverse)(sqlite3_context*,పూర్ణాంకం,sqlite3_విలువ **),
  శూన్య (*xDestroy)(శూన్య *)
){
  తిరిగి createFunctionApi(db, zFunc, nArg, enc, p, 0, x_అడుగు,
                                    xFinal, x_విలువ, xInverse, xDestroy);
}

#నిర్వచితం_కాకపోతే SQLITE_OMIT_UTF16
పూర్ణాంకం sqlite3_create_function16(
  sqlite3 *db,
  స్థిరరాశి శూన్య *zFunctionName,
  పూర్ణాంకం nArg,
  పూర్ణాంకం e_పాఠ్యం_Rep,
  శూన్య *p,
  శూన్య (*xSFunc)(sqlite3_context*,పూర్ణాంకం,sqlite3_విలువ**),
  శూన్య (*x_అడుగు)(sqlite3_context*,పూర్ణాంకం,sqlite3_విలువ**),
  శూన్య (*xFinal)(sqlite3_context*)
){
  పూర్ణాంకం rc;
  అక్షరం *zFunc8;

#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !sqlite3SafetyCheckOk(db) || zFunctionName==0 ) తిరిగి SQLITE_MISUSE_BKPT;
#అయితే_ముగింపు
  sqlite3_mutex_enter(db->mutex);
  ప్రతిజ్ఞా( !db->mallocFailed );
  zFunc8 = sqlite3Utf16to8(db, zFunctionName, -1, SQLITE_UTF16NATIVE);
  rc = sqlite3CreateFunc(db, zFunc8, nArg, e_పాఠ్యం_Rep, p, xSFunc,x_అడుగు,xFinal,0,0,0);
  sqlite3DbFree(db, zFunc8);
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  తిరిగి rc;
}
#అయితే_ముగింపు


/*
** The following is the implementation of an SQL function that always
** fails with an error message stating that the function is used in the
** wrong context.  The sqlite3_overload_function() API might construct
** SQL function that use this routine so that the functions will exist
** for name resolution but are actually overloaded by the xFindFunction
** method of virtual tables.
*/
స్థిర శూన్య sqlite3InvalidFunction(
  sqlite3_context *context,  /* The function calling context */
  పూర్ణాంకం Not_వాడిన,               /* Number of arguments to the function */
  sqlite3_విలువ **NotUsed2   /* Value of each argument */
){
  స్థిరరాశి అక్షరం *zName = (స్థిరరాశి అక్షరం*)sqlite3_user_దత్తాంశం(context);
  అక్షరం *zErr;
  వాడని_PARAMETER2(Not_వాడిన, NotUsed2);
  zErr = sqlite3_mprintf(
      "unable to use function %s in the requested context", zName);
  sqlite3_ఫలితం_error(context, zErr, -1);
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
పూర్ణాంకం sqlite3_overload_function(
  sqlite3 *db,
  స్థిరరాశి అక్షరం *zName,
  పూర్ణాంకం nArg
){
  పూర్ణాంకం rc;
  అక్షరం *zCopy;

#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !sqlite3SafetyCheckOk(db) || zName==0 || nArg<-2 ){
    తిరిగి SQLITE_MISUSE_BKPT;
  }
#అయితే_ముగింపు
  sqlite3_mutex_enter(db->mutex);
  rc = sqlite3_కనుగొను_Function(db, zName, nArg, SQLITE_UTF8, 0)!=0;
  sqlite3_mutex_leave(db->mutex);
  ఒకవేళ( rc ) తిరిగి SQLITE_OK;
  zCopy = sqlite3_mprintf("%s", zName);
  ఒకవేళ( zCopy==0 ) తిరిగి SQLITE_NOMEM;
  తిరిగి sqlite3_create_function_v2(db, zName, nArg, SQLITE_UTF8,
                           zCopy, sqlite3InvalidFunction, 0, 0, sqlite3_free);
}

#నిర్వచితం_కాకపోతే SQLITE_OMIT_TRACE
/*
** Register a trace function.  The pArg from the previously registered trace
** is returned. 
**
** A NULL trace function means that no tracing is executes.  A non-NULL
** trace is a pointer to a function that is invoked at the start of each
** SQL statement.
*/
#నిర్వచితం_కాకపోతే SQLITE_OMIT_DEPRECATED
శూన్య *sqlite3_trace(sqlite3 *db, శూన్య(*xTrace)(శూన్య*,స్థిరరాశి అక్షరం*), శూన్య *pArg){
  శూన్య *pOld;

#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !sqlite3SafetyCheckOk(db) ){
    (శూన్య)SQLITE_MISUSE_BKPT;
    తిరిగి 0;
  }
#అయితే_ముగింపు
  sqlite3_mutex_enter(db->mutex);
  pOld = db->pTraceArg;
  db->mTrace = xTrace ? SQLITE_TRACE_LEGACY : 0;
  db->trace.xLegacy = xTrace;
  db->pTraceArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  తిరిగి pOld;
}
#అయితే_ముగింపు /* SQLITE_OMIT_DEPRECATED */

/* Register a trace callback using the version-2 interface.
*/
పూర్ణాంకం sqlite3_trace_v2(
  sqlite3 *db,                               /* Trace this connection */
  గుర్తులేని mTrace,                           /* Mask of events to be traced */
  పూర్ణాంకం(*xTrace)(గుర్తులేని,శూన్య*,శూన్య*,శూన్య*),  /* Callback to invoke */
  శూన్య *pArg                                 /* Context */
){
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !sqlite3SafetyCheckOk(db) ){
    తిరిగి SQLITE_MISUSE_BKPT;
  }
#అయితే_ముగింపు
  sqlite3_mutex_enter(db->mutex);
  ఒకవేళ( mTrace==0 ) xTrace = 0;
  ఒకవేళ( xTrace==0 ) mTrace = 0;
  db->mTrace = mTrace;
  db->trace.xV2 = xTrace;
  db->pTraceArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  తిరిగి SQLITE_OK;
}

#నిర్వచితం_కాకపోతే SQLITE_OMIT_DEPRECATED
/*
** Register a profile function.  The pArg from the previously registered
** profile function is returned. 
**
** A NULL profile function means that no profiling is executes.  A non-NULL
** profile is a pointer to a function that is invoked at the conclusion of
** each SQL statement that is run.
*/
శూన్య *sqlite3_profile(
  sqlite3 *db,
  శూన్య (*xProfile)(శూన్య*,స్థిరరాశి అక్షరం*,sqlite_uint64),
  శూన్య *pArg
){
  శూన్య *pOld;

#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !sqlite3SafetyCheckOk(db) ){
    (శూన్య)SQLITE_MISUSE_BKPT;
    తిరిగి 0;
  }
#అయితే_ముగింపు
  sqlite3_mutex_enter(db->mutex);
  pOld = db->pProfileArg;
  db->xProfile = xProfile;
  db->pProfileArg = pArg;
  db->mTrace &= SQLITE_TRACE_NONLEGACY_ముసుగు;
  ఒకవేళ( db->xProfile ) db->mTrace |= SQLITE_TRACE_XPROFILE;
  sqlite3_mutex_leave(db->mutex);
  తిరిగి pOld;
}
#అయితే_ముగింపు /* SQLITE_OMIT_DEPRECATED */
#అయితే_ముగింపు /* SQLITE_OMIT_TRACE */

/*
** Register a function to be invoked when a transaction commits.
** If the invoked function returns non-zero, then the commit becomes a
** rollback.
*/
శూన్య *sqlite3_commit_hook(
  sqlite3 *db,              /* Attach the hook to this database */
  పూర్ణాంకం (*xCallback)(శూన్య*),  /* Function to invoke on each commit */
  శూన్య *pArg                /* Argument to the function */
){
  శూన్య *pOld;

#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !sqlite3SafetyCheckOk(db) ){
    (శూన్య)SQLITE_MISUSE_BKPT;
    తిరిగి 0;
  }
#అయితే_ముగింపు
  sqlite3_mutex_enter(db->mutex);
  pOld = db->pCommitArg;
  db->xCommitCallback = xCallback;
  db->pCommitArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  తిరిగి pOld;
}

/*
** Register a callback to be invoked each time a row is updated,
** inserted or deleted using this database connection.
*/
శూన్య *sqlite3_update_hook(
  sqlite3 *db,              /* Attach the hook to this database */
  శూన్య (*xCallback)(శూన్య*,పూర్ణాంకం,అక్షరం స్థిరరాశి *,అక్షరం స్థిరరాశి *,sqlite_int64),
  శూన్య *pArg                /* Argument to the function */
){
  శూన్య *pRet;

#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !sqlite3SafetyCheckOk(db) ){
    (శూన్య)SQLITE_MISUSE_BKPT;
    తిరిగి 0;
  }
#అయితే_ముగింపు
  sqlite3_mutex_enter(db->mutex);
  pRet = db->pUpdateArg;
  db->xUpdateCallback = xCallback;
  db->pUpdateArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  తిరిగి pRet;
}

/*
** Register a callback to be invoked each time a transaction is rolled
** back by this database connection.
*/
శూన్య *sqlite3_rollback_hook(
  sqlite3 *db,              /* Attach the hook to this database */
  శూన్య (*xCallback)(శూన్య*), /* Callback function */
  శూన్య *pArg                /* Argument to the function */
){
  శూన్య *pRet;

#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !sqlite3SafetyCheckOk(db) ){
    (శూన్య)SQLITE_MISUSE_BKPT;
    తిరిగి 0;
  }
#అయితే_ముగింపు
  sqlite3_mutex_enter(db->mutex);
  pRet = db->pRollbackArg;
  db->xRollbackCallback = xCallback;
  db->pRollbackArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  తిరిగి pRet;
}

#నిర్వచితమైతే SQLITE_ENABLE_PREUPDATE_HOOK
/*
** Register a callback to be invoked each time a row is updated,
** inserted or deleted using this database connection.
*/
శూన్య *sqlite3_preupdate_hook(
  sqlite3 *db,              /* Attach the hook to this database */
  శూన్య(*xCallback)(         /* Callback function */
    శూన్య*,sqlite3*,పూర్ణాంకం,అక్షరం స్థిరరాశి*,అక్షరం స్థిరరాశి*,sqlite3_int64,sqlite3_int64),
  శూన్య *pArg                /* First callback argument */
){
  శూన్య *pRet;

#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( db==0 ){
    తిరిగి 0;
  }
#అయితే_ముగింపు
  sqlite3_mutex_enter(db->mutex);
  pRet = db->pPreUpdateArg;
  db->xPreUpdateCallback = xCallback;
  db->pPreUpdateArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  తిరిగి pRet;
}
#అయితే_ముగింపు /* SQLITE_ENABLE_PREUPDATE_HOOK */

/*
** Register a function to be invoked prior to each autovacuum that
** determines the number of pages to vacuum.
*/
పూర్ణాంకం sqlite3_autovacuum_పుటలు(
  sqlite3 *db,                 /* Attach the hook to this database */
  గుర్తులేని పూర్ణాంకం (*xCallback)(శూన్య*,స్థిరరాశి అక్షరం*,u32,u32,u32),
  శూన్య *pArg,                  /* Argument to the function */
  శూన్య (*xDestructor)(శూన్య*)   /* Destructor for pArg */
){
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !sqlite3SafetyCheckOk(db) ){
    ఒకవేళ( xDestructor ) xDestructor(pArg);
    తిరిగి SQLITE_MISUSE_BKPT;
  }
#అయితే_ముగింపు
  sqlite3_mutex_enter(db->mutex);
  ఒకవేళ( db->xAutovacDestr ){
    db->xAutovacDestr(db->p_Autovac_పుటలు_Arg);
  }
  db->x_Autovac_పుటలు = xCallback;
  db->p_Autovac_పుటలు_Arg = pArg;
  db->xAutovacDestr = xDestructor;
  sqlite3_mutex_leave(db->mutex);
  తిరిగి SQLITE_OK;
}


#నిర్వచితం_కాకపోతే SQLITE_OMIT_WAL
/*
** The sqlite3_wal_hook() callback registered by sqlite3_wal_autocheckpoint().
** Invoke sqlite3_wal_checkpoint if the number of frames in the log file
** is greater than sqlite3.pWalArg cast to an integer (the value configured by
** wal_autocheckpoint()).
*/
పూర్ణాంకం sqlite3WalDefaultHook(
  శూన్య *p_Client_దత్తాంశం,     /*వాదం*/
  sqlite3 *db,           /* Connection */
  స్థిరరాశి అక్షరం *zDb,       /* Database */
  పూర్ణాంకం n_చట్రం             /* Size of WAL */
){
  ఒకవేళ( n_చట్రం>=SQLITE_PTR_గమ్యం_INT(p_Client_దత్తాంశం) ){
    sqlite3BeginBenignMalloc();
    sqlite3_wal_checkpoint(db, zDb);
    sqlite3_చివర_Benign_Malloc();
  }
  తిరిగి SQLITE_OK;
}
#అయితే_ముగింపు /* SQLITE_OMIT_WAL */

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
పూర్ణాంకం sqlite3_wal_autocheckpoint(sqlite3 *db, పూర్ణాంకం n_చట్రం){
#నిర్వచితమైతే SQLITE_OMIT_WAL
  వాడని_PARAMETER(db);
  వాడని_PARAMETER(n_చట్రం);
#లేకపోతే
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !sqlite3SafetyCheckOk(db) ) తిరిగి SQLITE_MISUSE_BKPT;
#అయితే_ముగింపు
  ఒకవేళ( n_చట్రం>0 ){
    sqlite3_wal_hook(db, sqlite3WalDefaultHook, SQLITE_INT_గమ్యం_PTR(n_చట్రం));
  }లేకపోతే{
    sqlite3_wal_hook(db, 0, 0);
  }
#అయితే_ముగింపు
  తిరిగి SQLITE_OK;
}

/*
** Register a callback to be invoked each time a transaction is written
** into the write-ahead-log by this database connection.
*/
శూన్య *sqlite3_wal_hook(
  sqlite3 *db,                    /* Attach the hook to this db handle */
  పూర్ణాంకం(*xCallback)(శూన్య *, sqlite3*, స్థిరరాశి అక్షరం*, పూర్ణాంకం),
  శూన్య *pArg                      /* First argument passed to xCallback() */
){
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WAL
  శూన్య *pRet;
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !sqlite3SafetyCheckOk(db) ){
    (శూన్య)SQLITE_MISUSE_BKPT;
    తిరిగి 0;
  }
#అయితే_ముగింపు
  sqlite3_mutex_enter(db->mutex);
  pRet = db->pWalArg;
  db->xWalCallback = xCallback;
  db->pWalArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  తిరిగి pRet;
#లేకపోతే
  వాడని_PARAMETER(db);
  వాడని_PARAMETER(xCallback);
  వాడని_PARAMETER(pArg);
  తిరిగి 0;
#అయితే_ముగింపు
}

/*
** Checkpoint database zDb.
*/
పూర్ణాంకం sqlite3_wal_checkpoint_v2(
  sqlite3 *db,                    /* Database handle */
  స్థిరరాశి అక్షరం *zDb,                /* Name of attached database (or NULL) */
  పూర్ణాంకం e_బహుళకం,                      /* SQLITE_CHECKPOINT_* value */
  పూర్ణాంకం *pnLog,                     /* OUT: Size of WAL log in frames */
  పూర్ణాంకం *pnCkpt                     /* OUT: Total number of frames checkpointed */
){
#నిర్వచితమైతే SQLITE_OMIT_WAL
  వాడని_PARAMETER(db);
  వాడని_PARAMETER(zDb);
  వాడని_PARAMETER(e_బహుళకం);
  వాడని_PARAMETER(pnLog);
  వాడని_PARAMETER(pnCkpt);
  తిరిగి SQLITE_OK;
#లేకపోతే
  పూర్ణాంకం rc;                         /* Return code */
  పూర్ణాంకం iDb;                        /* Schema to checkpoint */

#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !sqlite3SafetyCheckOk(db) ) తిరిగి SQLITE_MISUSE_BKPT;
#అయితే_ముగింపు

  /* Initialize the output variables to -1 in case an error occurs. */
  ఒకవేళ( pnLog ) *pnLog = -1;
  ఒకవేళ( pnCkpt ) *pnCkpt = -1;

  ప్రతిజ్ఞా( SQLITE_CHECKPOINT_NOOP==-1 );
  ప్రతిజ్ఞా( SQLITE_CHECKPOINT_PASSIVE==0 );
  ప్రతిజ్ఞా( SQLITE_CHECKPOINT_నిండిన==1 );
  ప్రతిజ్ఞా( SQLITE_CHECKPOINT_RESTART==2 );
  ప్రతిజ్ఞా( SQLITE_CHECKPOINT_TRUNCATE==3 );
  ఒకవేళ( e_బహుళకం<SQLITE_CHECKPOINT_NOOP || e_బహుళకం>SQLITE_CHECKPOINT_TRUNCATE ){
    /* EVIDENCE-OF: R-03996-12088 The M parameter must be a valid checkpoint
    ** mode: */
    తిరిగి SQLITE_MISUSE_BKPT;
  }

  sqlite3_mutex_enter(db->mutex);
  ఒకవేళ( zDb && zDb[0] ){
    iDb = sqlite3_కనుగొను_Db_Name(db, zDb);
  }లేకపోతే{
    iDb = SQLITE_MAX_DB;   /* This means process all schemas */
  }
  ఒకవేళ( iDb<0 ){
    rc = SQLITE_ERROR;
    sqlite3_Error_With_సందేశం(db, SQLITE_ERROR, "unknown database: %s", zDb);
  }లేకపోతే{
    db->busyHandler.nBusy = 0;
    rc = sqlite3Checkpoint(db, iDb, e_బహుళకం, pnLog, pnCkpt);
    sqlite3Error(db, rc);
  }
  rc = sqlite3ApiExit(db, rc);

  /* If there are no active statements, clear the interrupt flag at this
  ** point.  */
  ఒకవేళ( db->n_Vdbe_చురుకైన==0 ){
    AtomicStore(&db->u1.isInterrupted, 0);
  }

  sqlite3_mutex_leave(db->mutex);
  తిరిగి rc;
#అయితే_ముగింపు
}


/*
** Checkpoint database zDb. If zDb is NULL, or if the buffer zDb points
** to contains a zero-length string, all attached databases are
** checkpointed.
*/
పూర్ణాంకం sqlite3_wal_checkpoint(sqlite3 *db, స్థిరరాశి అక్షరం *zDb){
  /* EVIDENCE-OF: R-41613-20553 The sqlite3_wal_checkpoint(D,X) is equivalent to
  ** sqlite3_wal_checkpoint_v2(D,X,SQLITE_CHECKPOINT_PASSIVE,0,0). */
  తిరిగి sqlite3_wal_checkpoint_v2(db,zDb,SQLITE_CHECKPOINT_PASSIVE,0,0);
}

#నిర్వచితం_కాకపోతే SQLITE_OMIT_WAL
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
పూర్ణాంకం sqlite3Checkpoint(sqlite3 *db, పూర్ణాంకం iDb, పూర్ణాంకం e_బహుళకం, పూర్ణాంకం *pnLog, పూర్ణాంకం *pnCkpt){
  పూర్ణాంకం rc = SQLITE_OK;             /* Return code */
  పూర్ణాంకం i;                          /* Used to iterate through attached dbs */
  పూర్ణాంకం bBusy = 0;                  /* True if SQLITE_BUSY has been encountered */

  ప్రతిజ్ఞా( sqlite3_mutex_held(db->mutex) );
  ప్రతిజ్ఞా( !pnLog || *pnLog==-1 );
  ప్రతిజ్ఞా( !pnCkpt || *pnCkpt==-1 );
  testcase( iDb==SQLITE_MAX_ATTACHED ); /* See forum post a006d86f72 */
  testcase( iDb==SQLITE_MAX_DB );

  కొరకు(i=0; i<db->nDb && rc==SQLITE_OK; i++){
    ఒకవేళ( i==iDb || iDb==SQLITE_MAX_DB ){
      rc = sqlite3BtreeCheckpoint(db->aDb[i].pBt, e_బహుళకం, pnLog, pnCkpt);
      pnLog = 0;
      pnCkpt = 0;
      ఒకవేళ( rc==SQLITE_BUSY ){
        bBusy = 1;
        rc = SQLITE_OK;
      }
    }
  }

  తిరిగి (rc==SQLITE_OK && bBusy) ? SQLITE_BUSY : rc;
}
#అయితే_ముగింపు /* SQLITE_OMIT_WAL */

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
పూర్ణాంకం sqlite3_తాత్కాలిక_లోపల_జ్ఞాపకం(స్థిరరాశి sqlite3 *db){
#ఒకవేళ SQLITE_తాత్కాలిక_STORE==1
  తిరిగి ( db->తాత్కాలిక_store==2 );
#అయితే_ముగింపు
#ఒకవేళ SQLITE_తాత్కాలిక_STORE==2
  తిరిగి ( db->తాత్కాలిక_store!=1 );
#అయితే_ముగింపు
#ఒకవేళ SQLITE_తాత్కాలిక_STORE==3
  వాడని_PARAMETER(db);
  తిరిగి 1;
#అయితే_ముగింపు
#ఒకవేళ SQLITE_తాత్కాలిక_STORE<1 || SQLITE_తాత్కాలిక_STORE>3
  వాడని_PARAMETER(db);
  తిరిగి 0;
#అయితే_ముగింపు
}

/*
** Return UTF-8 encoded English language explanation of the most recent
** error.
*/
స్థిరరాశి అక్షరం *sqlite3_errmsg(sqlite3 *db){
  స్థిరరాశి అక్షరం *z;
  ఒకవేళ( !db ){
    తిరిగి sqlite3ErrStr(SQLITE_NOMEM_BKPT);
  }
  ఒకవేళ( !sqlite3SafetyCheckSickOrOk(db) ){
    తిరిగి sqlite3ErrStr(SQLITE_MISUSE_BKPT);
  }
  sqlite3_mutex_enter(db->mutex);
  ఒకవేళ( db->mallocFailed ){
    z = sqlite3ErrStr(SQLITE_NOMEM_BKPT);
  }లేకపోతే{
    testcase( db->pErr==0 );
    z = db->err_సంకేతం ? (అక్షరం*)sqlite3_విలువ_పాఠ్యం(db->pErr) : 0;
    ప్రతిజ్ఞా( !db->mallocFailed );
    ఒకవేళ( z==0 ){
      z = sqlite3ErrStr(db->err_సంకేతం);
    }
  }
  sqlite3_mutex_leave(db->mutex);
  తిరిగి z;
}

/*
** Set the error code and error message associated with the database handle.
**
** This routine is intended to be called by outside extensions (ex: the
** Session extension). Internal logic should invoke sqlite3Error() or
** sqlite3ErrorWithMsg() directly.
*/
పూర్ణాంకం sqlite3_సమితి_errmsg(sqlite3 *db, పూర్ణాంకం errcode, స్థిరరాశి అక్షరం *z_సందేశం){
  పూర్ణాంకం rc = SQLITE_OK;
  ఒకవేళ( !sqlite3SafetyCheckOk(db) ){
    తిరిగి SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(db->mutex);
  ఒకవేళ( z_సందేశం ){
    sqlite3_Error_With_సందేశం(db, errcode, "%s", z_సందేశం);
  }లేకపోతే{
    sqlite3Error(db, errcode);
  }
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  తిరిగి rc;
}

/*
** Return the byte offset of the most recent error
*/
పూర్ణాంకం sqlite3_error_స్థానభ్రంశం(sqlite3 *db){
  పూర్ణాంకం i_స్థానభ్రంశం = -1;
  ఒకవేళ( db && sqlite3SafetyCheckSickOrOk(db) ){
    sqlite3_mutex_enter(db->mutex);
    ఒకవేళ( db->err_సంకేతం ){
      i_స్థానభ్రంశం = db->err_Byte_స్థానభ్రంశం;
    }
    sqlite3_mutex_leave(db->mutex);
  }
  తిరిగి i_స్థానభ్రంశం;
}

#నిర్వచితం_కాకపోతే SQLITE_OMIT_UTF16
/*
** Return UTF-16 encoded English language explanation of the most recent
** error.
*/
స్థిరరాశి శూన్య *sqlite3_errmsg16(sqlite3 *db){
  స్థిర స్థిరరాశి u16 బహిర్గతం_యొక్క_Mem[] = {
    'o', 'u', 't', ' ', 'o', 'f', ' ', 'm', 'e', 'm', 'o', 'r', 'y', 0
  };
  స్థిర స్థిరరాశి u16 misuse[] = {
    'b', 'a', 'd', ' ', 'p', 'a', 'r', 'a', 'm', 'e', 't', 'e', 'r', ' ',
    'o', 'r', ' ', 'o', 't', 'h', 'e', 'r', ' ', 'A', 'P', 'I', ' ',
    'm', 'i', 's', 'u', 's', 'e', 0
  };

  స్థిరరాశి శూన్య *z;
  ఒకవేళ( !db ){
    తిరిగి (శూన్య *)బహిర్గతం_యొక్క_Mem;
  }
  ఒకవేళ( !sqlite3SafetyCheckSickOrOk(db) ){
    తిరిగి (శూన్య *)misuse;
  }
  sqlite3_mutex_enter(db->mutex);
  ఒకవేళ( db->mallocFailed ){
    z = (శూన్య *)బహిర్గతం_యొక్క_Mem;
  }లేకపోతే{
    z = sqlite3_విలువ_text16(db->pErr);
    ఒకవేళ( z==0 ){
      sqlite3_Error_With_సందేశం(db, db->err_సంకేతం, sqlite3ErrStr(db->err_సంకేతం));
      z = sqlite3_విలువ_text16(db->pErr);
    }
    /* A malloc() may have failed within the call to sqlite3_value_text16()
    ** above. If this is the case, then the db->mallocFailed flag needs to
    ** be cleared before returning. Do this directly, instead of via
    ** sqlite3ApiExit(), to avoid setting the database handle error message.
    */
    sqlite3OomClear(db);
  }
  sqlite3_mutex_leave(db->mutex);
  తిరిగి z;
}
#అయితే_ముగింపు /* SQLITE_OMIT_UTF16 */

/*
** Return the most recent error code generated by an SQLite routine. If NULL is
** passed to this function, we assume a malloc() failed during sqlite3_open().
*/
పూర్ణాంకం sqlite3_errcode(sqlite3 *db){
  పూర్ణాంకం iRet;
  ఒకవేళ( !db ) తిరిగి SQLITE_NOMEM_BKPT;
  ఒకవేళ( !sqlite3SafetyCheckSickOrOk(db) ){
    తిరిగి SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(db->mutex);
  ఒకవేళ( db->mallocFailed ){
    iRet = SQLITE_NOMEM_BKPT;
  }లేకపోతే{
    iRet = db->err_సంకేతం & db->err_ముసుగు;
  }
  sqlite3_mutex_leave(db->mutex);
  తిరిగి iRet;
}
పూర్ణాంకం sqlite3_extended_errcode(sqlite3 *db){
  పూర్ణాంకం iRet;
  ఒకవేళ( !db ) తిరిగి SQLITE_NOMEM_BKPT;
  ఒకవేళ( !sqlite3SafetyCheckSickOrOk(db) ){
    తిరిగి SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(db->mutex);
  ఒకవేళ( db->mallocFailed ){
    iRet = SQLITE_NOMEM_BKPT;
  }లేకపోతే{
    iRet = db->err_సంకేతం;
  }
  sqlite3_mutex_leave(db->mutex);
  తిరిగి iRet;
}
పూర్ణాంకం sqlite3_system_errno(sqlite3 *db){
  పూర్ణాంకం iRet = 0;
  ఒకవేళ( db ){
    sqlite3_mutex_enter(db->mutex);
    iRet = db->iSysErrno;
    sqlite3_mutex_leave(db->mutex);
  }
  తిరిగి iRet;
} 

/*
** Return a string that describes the kind of error specified in the
** argument.  For now, this simply calls the internal sqlite3ErrStr()
** function.
*/
స్థిరరాశి అక్షరం *sqlite3_errstr(పూర్ణాంకం rc){
  తిరిగి sqlite3ErrStr(rc);
}

/*
** Create a new collating function for database "db".  The name is zName
** and the encoding is enc.
*/
స్థిర పూర్ణాంకం createCollation(
  sqlite3* db,
  స్థిరరాశి అక్షరం *zName,
  u8 enc,
  శూన్య* pCtx,
  పూర్ణాంకం(*xCompare)(శూన్య*,పూర్ణాంకం,స్థిరరాశి శూన్య*,పూర్ణాంకం,స్థిరరాశి శూన్య*),
  శూన్య(*xDel)(శూన్య*)
){
  CollSeq *pColl;
  పూర్ణాంకం enc2;
 
  ప్రతిజ్ఞా( sqlite3_mutex_held(db->mutex) );

  /* If SQLITE_UTF16 is specified as the encoding type, transform this
  ** to one of SQLITE_UTF16LE or SQLITE_UTF16BE using the
  ** SQLITE_UTF16NATIVE macro. SQLITE_UTF16 is not used internally.
  */
  enc2 = enc;
  testcase( enc2==SQLITE_UTF16 );
  testcase( enc2==SQLITE_UTF16_ALIGNED );
  ఒకవేళ( enc2==SQLITE_UTF16 || enc2==SQLITE_UTF16_ALIGNED ){
    enc2 = SQLITE_UTF16NATIVE;
  }
  ఒకవేళ( enc2<SQLITE_UTF8 || enc2>SQLITE_UTF16BE ){
    తిరిగి SQLITE_MISUSE_BKPT;
  }

  /* Check if this call is removing or replacing an existing collation
  ** sequence. If so, and there are active VMs, return busy. If there
  ** are no active VMs, invalidate any pre-compiled statements.
  */
  pColl = sqlite3_కనుగొను_Coll_Seq(db, (u8)enc2, zName, 0);
  ఒకవేళ( pColl && pColl->xCmp ){
    ఒకవేళ( db->n_Vdbe_చురుకైన ){
      sqlite3_Error_With_సందేశం(db, SQLITE_BUSY,
        "unable to delete/modify collation sequence due to active statements");
      తిరిగి SQLITE_BUSY;
    }
    sqlite3ExpirePreparedStatements(db, 0);

    /* If collation sequence pColl was created directly by a call to
    ** sqlite3_create_collation, and not generated by synthCollSeq(),
    ** then any copies made by synthCollSeq() need to be invalidated.
    ** Also, collation destructor - CollSeq.xDel() - function may need
    ** to be called.
    */
    ఒకవేళ( (pColl->enc & ~SQLITE_UTF16_ALIGNED)==enc2 ){
      CollSeq *aColl = sqlite3_Hash_కనుగొను(&db->aCollSeq, zName);
      పూర్ణాంకం j;
      కొరకు(j=0; j<3; j++){
        CollSeq *p = &aColl[j];
        ఒకవేళ( p->enc==pColl->enc ){
          ఒకవేళ( p->xDel ){
            p->xDel(p->pUser);
          }
          p->xCmp = 0;
        }
      }
    }
  }

  pColl = sqlite3_కనుగొను_Coll_Seq(db, (u8)enc2, zName, 1);
  ఒకవేళ( pColl==0 ) తిరిగి SQLITE_NOMEM_BKPT;
  pColl->xCmp = xCompare;
  pColl->pUser = pCtx;
  pColl->xDel = xDel;
  pColl->enc = (u8)(enc2 | (enc & SQLITE_UTF16_ALIGNED));
  sqlite3Error(db, SQLITE_OK);
  తిరిగి SQLITE_OK;
}


/*
** This array defines hard upper bounds on limit values.  The
** initializer must be kept in sync with the SQLITE_LIMIT_*
** #defines in sqlite3.h.
*/
స్థిర స్థిరరాశి పూర్ణాంకం a_Hard_హద్దు[] = {
  SQLITE_MAX_నిడివి,
  SQLITE_MAX_SQL_నిడివి,
  SQLITE_MAX_స్తంభం,
  SQLITE_MAX_సమాసం_లోతు,
  SQLITE_MAX_COMPOUND_SELECT,
  SQLITE_MAX_VDBE_OP,
  SQLITE_MAX_FUNCTION_ARG,
  SQLITE_MAX_ATTACHED,
  SQLITE_MAX_LIKE_నమూనా_నిడివి,
  SQLITE_MAX_VARIABLE_సంఖ్య,      /* IMP: R-38091-32352 */
  SQLITE_MAX_TRIGGER_లోతు,
  SQLITE_MAX_కార్మికుడు_దారాలు,
  SQLITE_MAX_PARSER_లోతు,
  SQLITE_MAX_SCHEMA,
  SQLITE_MAX_TRIGGER_అడుగులు,
  SQLITE_MAX_CTES,
};

/*
** Make sure the hard limits are set to reasonable values
*/
#ఒకవేళ SQLITE_MAX_నిడివి<100
# error SQLITE_MAX_నిడివి must be at కనిష్ఠ 100
#అయితే_ముగింపు
#ఒకవేళ SQLITE_MAX_SQL_నిడివి<100
# error SQLITE_MAX_SQL_నిడివి must be at కనిష్ఠ 100
#అయితే_ముగింపు
#ఒకవేళ SQLITE_MAX_SQL_నిడివి>SQLITE_MAX_నిడివి
# error SQLITE_MAX_SQL_నిడివి must not be greater than SQLITE_MAX_నిడివి
#అయితే_ముగింపు
#ఒకవేళ SQLITE_MAX_SQL_నిడివి>2147482624     /* 1024 less than 2^31 */
# error SQLITE_MAX_SQL_నిడివి must not be greater than 2147482624
#అయితే_ముగింపు
#ఒకవేళ SQLITE_MAX_COMPOUND_SELECT<2
# error SQLITE_MAX_COMPOUND_SELECT must be at కనిష్ఠ 2
#అయితే_ముగింపు
#ఒకవేళ SQLITE_MAX_VDBE_OP<40
# error SQLITE_MAX_VDBE_OP must be at కనిష్ఠ 40
#అయితే_ముగింపు
#ఒకవేళ SQLITE_MAX_FUNCTION_ARG<0 || SQLITE_MAX_FUNCTION_ARG>32767
# error SQLITE_MAX_FUNCTION_ARG must be between 0 and 32767
#అయితే_ముగింపు
#ఒకవేళ SQLITE_MAX_ATTACHED<0 || SQLITE_MAX_ATTACHED>125
# error SQLITE_MAX_ATTACHED must be between 0 and 125
#అయితే_ముగింపు
#ఒకవేళ SQLITE_MAX_LIKE_నమూనా_నిడివి<1
# error SQLITE_MAX_LIKE_నమూనా_నిడివి must be at కనిష్ఠ 1
#అయితే_ముగింపు
#ఒకవేళ SQLITE_MAX_స్తంభం>32767
# error SQLITE_MAX_స్తంభం must not exceed 32767
#అయితే_ముగింపు
#ఒకవేళ SQLITE_MAX_TRIGGER_లోతు<1
# error SQLITE_MAX_TRIGGER_లోతు must be at కనిష్ఠ 1
#అయితే_ముగింపు
#ఒకవేళ SQLITE_MAX_కార్మికుడు_దారాలు<0 || SQLITE_MAX_కార్మికుడు_దారాలు>50
# error SQLITE_MAX_కార్మికుడు_దారాలు must be between 0 and 50
#అయితే_ముగింపు
#ఒకవేళ SQLITE_MAX_TRIGGER_అడుగులు<0 || SQLITE_MAX_TRIGGER_అడుగులు>65535
# error SQLITE_MAX_TRIGGER_అడుగులు must be between 0 and 65535
#అయితే_ముగింపు


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
పూర్ణాంకం sqlite3_హద్దు(sqlite3 *db, పూర్ణాంకం హద్దు_Id, పూర్ణాంకం new_హద్దు){
  పూర్ణాంకం old_హద్దు;

#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !sqlite3SafetyCheckOk(db) ){
    (శూన్య)SQLITE_MISUSE_BKPT;
    తిరిగి -1;
  }
#అయితే_ముగింపు

  /* EVIDENCE-OF: R-30189-54097 For each limit category SQLITE_LIMIT_NAME
  ** there is a hard upper bound set at compile-time by a C preprocessor
  ** macro called SQLITE_MAX_NAME. (The "_LIMIT_" in the name is changed to
  ** "_MAX_".)
  */
  ప్రతిజ్ఞా( a_Hard_హద్దు[SQLITE_హద్దు_నిడివి]==SQLITE_MAX_నిడివి );
  ప్రతిజ్ఞా( a_Hard_హద్దు[SQLITE_హద్దు_SQL_నిడివి]==SQLITE_MAX_SQL_నిడివి );
  ప్రతిజ్ఞా( a_Hard_హద్దు[SQLITE_హద్దు_స్తంభం]==SQLITE_MAX_స్తంభం );
  ప్రతిజ్ఞా( a_Hard_హద్దు[SQLITE_హద్దు_సమాసం_లోతు]==SQLITE_MAX_సమాసం_లోతు );
  ప్రతిజ్ఞా( a_Hard_హద్దు[SQLITE_హద్దు_PARSER_లోతు]==SQLITE_MAX_PARSER_లోతు );
  ప్రతిజ్ఞా( a_Hard_హద్దు[SQLITE_హద్దు_COMPOUND_SELECT]==SQLITE_MAX_COMPOUND_SELECT);
  ప్రతిజ్ఞా( a_Hard_హద్దు[SQLITE_హద్దు_VDBE_OP]==SQLITE_MAX_VDBE_OP );
  ప్రతిజ్ఞా( a_Hard_హద్దు[SQLITE_హద్దు_FUNCTION_ARG]==SQLITE_MAX_FUNCTION_ARG );
  ప్రతిజ్ఞా( a_Hard_హద్దు[SQLITE_హద్దు_ATTACHED]==SQLITE_MAX_ATTACHED );
  ప్రతిజ్ఞా( a_Hard_హద్దు[SQLITE_హద్దు_LIKE_నమూనా_నిడివి]==
                                               SQLITE_MAX_LIKE_నమూనా_నిడివి );
  ప్రతిజ్ఞా( a_Hard_హద్దు[SQLITE_హద్దు_VARIABLE_సంఖ్య]==SQLITE_MAX_VARIABLE_సంఖ్య);
  ప్రతిజ్ఞా( a_Hard_హద్దు[SQLITE_హద్దు_TRIGGER_లోతు]==SQLITE_MAX_TRIGGER_లోతు );
  ప్రతిజ్ఞా( a_Hard_హద్దు[SQLITE_హద్దు_కార్మికుడు_దారాలు]==SQLITE_MAX_కార్మికుడు_దారాలు );
  ప్రతిజ్ఞా( a_Hard_హద్దు[SQLITE_హద్దు_SCHEMA]==SQLITE_MAX_SCHEMA );
  ప్రతిజ్ఞా( a_Hard_హద్దు[SQLITE_హద్దు_TRIGGER_అడుగులు]==SQLITE_MAX_TRIGGER_అడుగులు );
  ప్రతిజ్ఞా( a_Hard_హద్దు[SQLITE_హద్దు_CTES]==SQLITE_MAX_CTES );
  ప్రతిజ్ఞా( SQLITE_హద్దు_CTES==(SQLITE_N_హద్దు-1) );


  ఒకవేళ( హద్దు_Id<0 || హద్దు_Id>=SQLITE_N_హద్దు ){
    తిరిగి -1;
  }
  sqlite3_mutex_enter(db->mutex);
  old_హద్దు = db->a_హద్దు[హద్దు_Id];
  ఒకవేళ( new_హద్దు>=0 ){                   /* IMP: R-52476-28732 */
    ఒకవేళ( new_హద్దు>a_Hard_హద్దు[హద్దు_Id] ){
      new_హద్దు = a_Hard_హద్దు[హద్దు_Id];  /* IMP: R-51463-25634 */
    }లేకపోతే ఒకవేళ( new_హద్దు<SQLITE_MIN_నిడివి && హద్దు_Id==SQLITE_హద్దు_నిడివి ){
      new_హద్దు = SQLITE_MIN_నిడివి;
    }
    db->a_హద్దు[హద్దు_Id] = new_హద్దు;
  }
  sqlite3_mutex_leave(db->mutex);
  తిరిగి old_హద్దు;                     /* IMP: R-53341-35419 */
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
పూర్ణాంకం sqlite3ParseUri(
  స్థిరరాశి అక్షరం *zDefaultVfs,        /* VFS to use if no "vfs=xxx" query option */
  స్థిరరాశి అక్షరం *zUri,               /* Nul-terminated URI to parse */
  గుర్తులేని పూర్ణాంకం *pFlags,           /* IN/OUT: SQLITE_OPEN_XXX flags */
  sqlite3_vfs **ppVfs,            /* OUT: VFS to use */
  అక్షరం **pzFile,                  /* OUT: Filename component of URI */
  అక్షరం **pz_Err_సందేశం                 /* OUT: Error message (if rc!=SQLITE_OK) */
){
  పూర్ణాంకం rc = SQLITE_OK;
  గుర్తులేని పూర్ణాంకం flags = *pFlags;
  స్థిరరాశి అక్షరం *zVfs = zDefaultVfs;
  అక్షరం *zFile;
  అక్షరం c;
  i64 nUri = అక్షరశ్రేణి_పొడవు(zUri);

  ప్రతిజ్ఞా( *pz_Err_సందేశం==0 );

  ఒకవేళ( ((flags & SQLITE_OPEN_URI)                     /* IMP: R-48725-32206 */
       || Atomic_ఎక్కించు(&sqlite3GlobalConfig.bOpenUri)) /* IMP: R-51689-46548 */
   && nUri>=5 && memcmp(zUri, "file:", 5)==0         /* IMP: R-57884-37496 */
  ){
    అక్షరం *zOpt;
    పూర్ణాంకం eState;                   /* Parser state when parsing URI */
    i64 i_లోపల;                      /* Input character index */
    i64 i_బహిర్గతం = 0;                 /* Output character index */
    u64 nByte = nUri+8;           /* Bytes of space to allocate */

    /* Make sure the SQLITE_OPEN_URI flag is set to indicate to the VFS xOpen
    ** method that there may be extra parameters following the file-name.  */
    flags |= SQLITE_OPEN_URI;

    కొరకు(i_లోపల=0; i_లోపల<nUri; i_లోపల++) nByte += (zUri[i_లోపల]=='&');
    zFile = sqlite3_malloc64(nByte);
    ఒకవేళ( !zFile ) తిరిగి SQLITE_NOMEM_BKPT;

    memset(zFile, 0, 4);  /* 4-byte of 0x00 is the start of DB name marker */
    zFile += 4;

    i_లోపల = 5;
#నిర్వచితమైతే SQLITE_ALLOW_URI_AUTHORITY
    ఒకవేళ( అక్షరశ్రేణి_పోలిక(zUri+5, "///", 3)==0 ){
      i_లోపల = 7;
      /* The following condition causes URIs with five leading / characters
      ** like file://///host/path to be converted into UNCs like //host/path.
      ** The correct URI for that UNC has only two or four leading / characters
      ** file://host/path or file:////host/path.  But 5 leading slashes is a
      ** common error, we are told, so we handle it as a special case. */
      ఒకవేళ( అక్షరశ్రేణి_పోలిక(zUri+7, "///", 3)==0 ){ i_లోపల++; }
    }లేకపోతే ఒకవేళ( అక్షరశ్రేణి_పోలిక(zUri+5, "//localhost/", 12)==0 ){
      i_లోపల = 16;
    }
#లేకపోతే
    /* Discard the scheme and authority segments of the URI. */
    ఒకవేళ( zUri[5]=='/' && zUri[6]=='/' ){
      i_లోపల = 7;
      అయితే( zUri[i_లోపల] && zUri[i_లోపల]!='/' ) i_లోపల++;
      ఒకవేళ( i_లోపల!=7 && (i_లోపల!=16 || memcmp("localhost", &zUri[7], 9)) ){
        *pz_Err_సందేశం = sqlite3_mprintf("invalid uri authority: %.*s",
            (పూర్ణాంకం)(i_లోపల-7), &zUri[7]);
        rc = SQLITE_ERROR;
        వెళ్ళు parse_uri_బహిర్గతం;
      }
    }
#అయితే_ముగింపు

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
    అయితే( (c = zUri[i_లోపల])!=0 && c!='#' ){
      i_లోపల++;
      ఒకవేళ( c=='%'
       && sqlite3Isxdigit(zUri[i_లోపల])
       && sqlite3Isxdigit(zUri[i_లోపల+1])
      ){
        పూర్ణాంకం octet = (sqlite3_Hex_గమ్యం_Int(zUri[i_లోపల++]) << 4);
        octet += sqlite3_Hex_గమ్యం_Int(zUri[i_లోపల++]);

        ప్రతిజ్ఞా( octet>=0 && octet<256 );
        ఒకవేళ( octet==0 ){
#నిర్వచితం_కాకపోతే SQLITE_ENABLE_URI_00_ERROR
          /* This branch is taken when "%00" appears within the URI. In this
          ** case we ignore all text in the remainder of the path, name or
          ** value currently being parsed. So ignore the current character
          ** and skip to the next "?", "=" or "&", as appropriate. */
          అయితే( (c = zUri[i_లోపల])!=0 && c!='#'
              && (eState!=0 || c!='?')
              && (eState!=1 || (c!='=' && c!='&'))
              && (eState!=2 || c!='&')
          ){
            i_లోపల++;
          }
          కొనసాగు;
#లేకపోతే
          /* If ENABLE_URI_00_ERROR is defined, "%00" in a URI is an error. */
          *pz_Err_సందేశం = sqlite3_mprintf("unexpected %%00 in uri");
          rc = SQLITE_ERROR;
          వెళ్ళు parse_uri_బహిర్గతం;
#అయితే_ముగింపు
        }
        c = octet;
      }లేకపోతే ఒకవేళ( eState==1 && (c=='&' || c=='=') ){
        ఒకవేళ( zFile[i_బహిర్గతం-1]==0 ){
          /* An empty option name. Ignore this option altogether. */
          అయితే( zUri[i_లోపల] && zUri[i_లోపల]!='#' && zUri[i_లోపల-1]!='&' ) i_లోపల++;
          కొనసాగు;
        }
        ఒకవేళ( c=='&' ){
          zFile[i_బహిర్గతం++] = '\0';
        }లేకపోతే{
          eState = 2;
        }
        c = 0;
      }లేకపోతే ఒకవేళ( (eState==0 && c=='?') || (eState==2 && c=='&') ){
        c = 0;
        eState = 1;
      }
      zFile[i_బహిర్గతం++] = c;
    }
    ఒకవేళ( eState==1 ) zFile[i_బహిర్గతం++] = '\0';
    memset(zFile+i_బహిర్గతం, 0, 4); /* end-of-options + empty journal filenames */

    /* Check if there were any options specified that should be interpreted
    ** here. Options that are interpreted here include "vfs" and those that
    ** correspond to flags that may be passed to the sqlite3_open_v2()
    ** method. */
    zOpt = &zFile[అక్షరశ్రేణి_పొడవు(zFile)+1];
    అయితే( zOpt[0] ){
      i64 nOpt = అక్షరశ్రేణి_పొడవు(zOpt);
      అక్షరం *zVal = &zOpt[nOpt+1];
      i64 nVal = అక్షరశ్రేణి_పొడవు(zVal);

      ఒకవేళ( nOpt==3 && memcmp("vfs", zOpt, 3)==0 ){
        zVfs = zVal;
      }లేకపోతే{
        నిర్మాణం Open_బహుళకం {
          స్థిరరాశి అక్షరం *z;
          పూర్ణాంకం బహుళకం;
        } *a_బహుళకం = 0;
        అక్షరం *z_బహుళకం_Type = 0;
        పూర్ణాంకం ముసుగు = 0;
        పూర్ణాంకం హద్దు = 0;

        ఒకవేళ( nOpt==5 && memcmp("cache", zOpt, 5)==0 ){
          స్థిర నిర్మాణం Open_బహుళకం a_Cache_బహుళకం[] = {
            { "shared",  SQLITE_OPEN_SHAREDCACHE },
            { "private", SQLITE_OPEN_PRIVATECACHE },
            { 0, 0 }
          };

          ముసుగు = SQLITE_OPEN_SHAREDCACHE|SQLITE_OPEN_PRIVATECACHE;
          a_బహుళకం = a_Cache_బహుళకం;
          హద్దు = ముసుగు;
          z_బహుళకం_Type = "cache";
        }
        ఒకవేళ( nOpt==4 && memcmp("mode", zOpt, 4)==0 ){
          స్థిర నిర్మాణం Open_బహుళకం a_Open_బహుళకం[] = {
            { "ro",  SQLITE_OPEN_READONLY },
            { "rw",  SQLITE_OPEN_READWRITE },
            { "rwc", SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE },
            { "memory", SQLITE_OPEN_జ్ఞాపకం },
            { 0, 0 }
          };

          ముసుగు = SQLITE_OPEN_READONLY | SQLITE_OPEN_READWRITE
                   | SQLITE_OPEN_CREATE | SQLITE_OPEN_జ్ఞాపకం;
          a_బహుళకం = a_Open_బహుళకం;
          హద్దు = ముసుగు & flags;
          z_బహుళకం_Type = "access";
        }

        ఒకవేళ( a_బహుళకం ){
          పూర్ణాంకం i;
          పూర్ణాంకం బహుళకం = 0;
          కొరకు(i=0; a_బహుళకం[i].z; i++){
            స్థిరరాశి అక్షరం *z = a_బహుళకం[i].z;
            ఒకవేళ( nVal==(i64)అక్షరశ్రేణి_పొడవు(z) && 0==memcmp(zVal, z, nVal) ){
              బహుళకం = a_బహుళకం[i].బహుళకం;
              ఆపు;
            }
          }
          ఒకవేళ( బహుళకం==0 ){
            *pz_Err_సందేశం = sqlite3_mprintf("no such %s mode: %s", z_బహుళకం_Type, zVal);
            rc = SQLITE_ERROR;
            వెళ్ళు parse_uri_బహిర్గతం;
          }
          ఒకవేళ( (బహుళకం & ~SQLITE_OPEN_జ్ఞాపకం)>హద్దు ){
            *pz_Err_సందేశం = sqlite3_mprintf("%s mode not allowed: %s",
                                        z_బహుళకం_Type, zVal);
            rc = SQLITE_PERM;
            వెళ్ళు parse_uri_బహిర్గతం;
          }
          flags = (flags & ~ముసుగు) | బహుళకం;
        }
      }

      zOpt = &zVal[nVal+1];
    }

  }లేకపోతే{
    zFile = sqlite3_malloc64(nUri+8);
    ఒకవేళ( !zFile ) తిరిగి SQLITE_NOMEM_BKPT;
    memset(zFile, 0, 4);
    zFile += 4;
    ఒకవేళ( nUri ){
      జ్ఞాపకం_నకలు(zFile, zUri, nUri);
    }
    memset(zFile+nUri, 0, 4);
    flags &= ~SQLITE_OPEN_URI;
  }

  *ppVfs = sqlite3_vfs_కనుగొను(zVfs);
  ఒకవేళ( *ppVfs==0 ){
    *pz_Err_సందేశం = sqlite3_mprintf("no such vfs: %s", zVfs);
    rc = SQLITE_ERROR;
  }
 parse_uri_బహిర్గతం:
  ఒకవేళ( rc!=SQLITE_OK ){
    sqlite3_free_filename(zFile);
    zFile = 0;
  }
  *pFlags = flags;
  *pzFile = zFile;
  తిరిగి rc;
}

/*
** This routine does the core work of extracting URI parameters from a
** database filename for the sqlite3_uri_parameter() interface.
*/
స్థిర స్థిరరాశి అక్షరం *uriParameter(స్థిరరాశి అక్షరం *zFilename, స్థిరరాశి అక్షరం *zParam){
  zFilename += sqlite3Strlen30(zFilename) + 1;
  అయితే( ALWAYS(zFilename!=0) && zFilename[0] ){
    పూర్ణాంకం x = strcmp(zFilename, zParam);
    zFilename += sqlite3Strlen30(zFilename) + 1;
    ఒకవేళ( x==0 ) తిరిగి zFilename;
    zFilename += sqlite3Strlen30(zFilename) + 1;
  }
  తిరిగి 0;
}



/*
** This routine does the work of opening a database on behalf of
** sqlite3_open() and sqlite3_open16(). The database filename "zFilename" 
** is UTF-8 encoded.
*/
స్థిర పూర్ణాంకం openDatabase(
  స్థిరరాశి అక్షరం *zFilename, /* Database filename UTF-8 encoded */
  sqlite3 **ppDb,        /* OUT: Returned database handle */
  గుర్తులేని పూర్ణాంకం flags,    /* Operational flags */
  స్థిరరాశి అక్షరం *zVfs       /* Name of the VFS to use */
){
  sqlite3 *db;                    /* Store allocated handle here */
  పూర్ణాంకం rc;                         /* Return code */
  పూర్ణాంకం isThreadsafe;               /* True for threadsafe connections */
  అక్షరం *zOpen = 0;                /* Filename argument to pass to BtreeOpen() */
  అక్షరం *z_Err_సందేశం = 0;              /* Error message from sqlite3ParseUri() */
  పూర్ణాంకం i;                          /* Loop counter */

#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( ppDb==0 ) తిరిగి SQLITE_MISUSE_BKPT;
#అయితే_ముగింపు
  *ppDb = 0;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOINIT
  rc = sqlite3_initialize();
  ఒకవేళ( rc ) తిరిగి rc;
#అయితే_ముగింపు

  ఒకవేళ( sqlite3GlobalConfig.bCoreMutex==0 ){
    isThreadsafe = 0;
  }లేకపోతే ఒకవేళ( flags & SQLITE_OPEN_NOMUTEX ){
    isThreadsafe = 0;
  }లేకపోతే ఒకవేళ( flags & SQLITE_OPEN_FULLMUTEX ){
    isThreadsafe = 1;
  }లేకపోతే{
    isThreadsafe = sqlite3GlobalConfig.b_నిండిన_Mutex;
  }

  ఒకవేళ( flags & SQLITE_OPEN_PRIVATECACHE ){
    flags &= ~SQLITE_OPEN_SHAREDCACHE;
  }లేకపోతే ఒకవేళ( sqlite3GlobalConfig.sharedCacheEnabled ){
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
               SQLITE_OPEN_తాత్కాలిక_DB |
               SQLITE_OPEN_TRANSIENT_DB |
               SQLITE_OPEN_MAIN_JOURNAL |
               SQLITE_OPEN_తాత్కాలిక_JOURNAL |
               SQLITE_OPEN_SUBJOURNAL |
               SQLITE_OPEN_SUPER_JOURNAL |
               SQLITE_OPEN_NOMUTEX |
               SQLITE_OPEN_FULLMUTEX |
               SQLITE_OPEN_WAL
             );

  /* Allocate the sqlite data structure */
  db = sqlite3MallocZero( పరిమాణం(sqlite3) );
  ఒకవేళ( db==0 ) వెళ్ళు opendb_బహిర్గతం;
  ఒకవేళ( isThreadsafe
#ఒకవేళ defined(SQLITE_THREAD_MISUSE_WARNINGS)
   || sqlite3GlobalConfig.bCoreMutex
#అయితే_ముగింపు
  ){
    db->mutex = sqlite3MutexAlloc(SQLITE_MUTEX_RECURSIVE);
    ఒకవేళ( db->mutex==0 ){
      sqlite3_free(db);
      db = 0;
      వెళ్ళు opendb_బహిర్గతం;
    }
    ఒకవేళ( isThreadsafe==0 ){
      sqlite3MutexWarnOnContention(db->mutex);
    }
  }
  sqlite3_mutex_enter(db->mutex);
  db->err_ముసుగు = (flags & SQLITE_OPEN_EXRESCODE)!=0 ? 0xffffffff : 0xff;
  db->nDb = 2;
  db->eOpenState = SQLITE_STATE_BUSY;
  db->aDb = db->aDbStatic;
  db->lookaside.bDisable = 1;
  db->lookaside.sz = 0;
  db->nFpDigit = 17;

  ప్రతిజ్ఞా( పరిమాణం(db->a_హద్దు)==పరిమాణం(a_Hard_హద్దు) );
  జ్ఞాపకం_నకలు(db->a_హద్దు, a_Hard_హద్దు, పరిమాణం(db->a_హద్దు));
  db->a_హద్దు[SQLITE_హద్దు_కార్మికుడు_దారాలు] = SQLITE_DEFAULT_కార్మికుడు_దారాలు;
  db->autoCommit = 1;
  db->తదుపరి_Autovac = -1;
  db->szMmap = sqlite3GlobalConfig.szMmap;
  db->తదుపరి_Pagesize = 0;
  db->init.azInit = sqlite3StdType; /* Any array of string ptrs will do */
#నిర్వచితమైతే SQLITE_ENABLE_SORTER_MMAP
  /* Beginning with version 3.37.0, using the VFS xFetch() API to memory-map
  ** the temporary files used to do external sorts (see code in vdbesort.c)
  ** is disabled. It can still be used either by defining
  ** SQLITE_ENABLE_SORTER_MMAP at compile time or by using the
  ** SQLITE_TESTCTRL_SORTER_MMAP test-control at runtime. */
  db->nMaxSorterMmap = 0x7FFFFFFF;
#అయితే_ముగింపు
  db->flags |= SQLITE_Short_నిలువు_పేర్లు
                 | SQLITE_EnableTrigger
                 | SQLITE_EnableView
                 | SQLITE_CacheSpill
                 | SQLITE_AttachCreate
                 | SQLITE_AttachWrite
                 | SQLITE_Comments
#ఒకవేళ !defined(SQLITE_TRUSTED_SCHEMA) || SQLITE_TRUSTED_SCHEMA+0!=0
                 | SQLITE_TrustedSchema
#అయితే_ముగింపు
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
#ఒకవేళ !defined(SQLITE_DQS)
# define SQLITE_DQS 3
#అయితే_ముగింపు
#ఒకవేళ (SQLITE_DQS&1)==1
                 | SQLITE_DqsDML
#అయితే_ముగింపు
#ఒకవేళ (SQLITE_DQS&2)==2
                 | SQLITE_DqsDDL
#అయితే_ముగింపు

#ఒకవేళ !defined(SQLITE_DEFAULT_AUTOMATIC_సూచీ) || SQLITE_DEFAULT_AUTOMATIC_సూచీ
                 | SQLITE_Auto_సూచీ
#అయితే_ముగింపు
#ఒకవేళ SQLITE_DEFAULT_CKPTFULLFSYNC
                 | SQLITE_Ckpt_నిండిన_FSync
#అయితే_ముగింపు
#ఒకవేళ SQLITE_DEFAULT_FILE_FORMAT<4
                 | SQLITE_LegacyFileFmt
#అయితే_ముగింపు
#నిర్వచితమైతే SQLITE_ENABLE_ఎక్కించు_EXTENSION
                 | SQLITE_ఎక్కించు_Extension
#అయితే_ముగింపు
#ఒకవేళ SQLITE_DEFAULT_RECURSIVE_TRIGGERS
                 | SQLITE_RecTriggers
#అయితే_ముగింపు
#ఒకవేళ defined(SQLITE_DEFAULT_FOREIGN_కీలకాలు) && SQLITE_DEFAULT_FOREIGN_కీలకాలు
                 | SQLITE_Foreign_కీలకాలు
#అయితే_ముగింపు
#ఒకవేళ defined(SQLITE_తిరగేయి_UNORDERED_SELECTS)
                 | SQLITE_తిరగేయి_క్రమం
#అయితే_ముగింపు
#ఒకవేళ defined(SQLITE_ENABLE_OVERSIZE_CELL_CHECK)
                 | SQLITE_Cell_కొలత_Ck
#అయితే_ముగింపు
#ఒకవేళ defined(SQLITE_ENABLE_FTS3_TOKENIZER)
                 | SQLITE_Fts3Tokenizer
#అయితే_ముగింపు
#ఒకవేళ defined(SQLITE_ENABLE_QPSG)
                 | SQLITE_EnableQPSG
#అయితే_ముగింపు
#ఒకవేళ defined(SQLITE_DEFAULT_DEFENSIVE)
                 | SQLITE_Defensive
#అయితే_ముగింపు
#ఒకవేళ defined(SQLITE_DEFAULT_LEGACY_ALTER_పట్టిక)
                 | SQLITE_LegacyAlter
#అయితే_ముగింపు
#ఒకవేళ defined(SQLITE_ENABLE_STMT_SCANSTATUS)
                 | SQLITE_Stmt_పరిశీలన_స్థితి
#అయితే_ముగింపు
      ;
  sqlite3HashInit(&db->aCollSeq);
#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
  sqlite3HashInit(&db->aModule);
#అయితే_ముగింపు

  /* Add the default collation sequence BINARY. BINARY works for both UTF-8
  ** and UTF-16, so add a version for each to avoid any unnecessary
  ** conversions. The only error that can occur here is a malloc() failure.
  **
  ** EVIDENCE-OF: R-52786-44878 SQLite defines three built-in collating
  ** functions:
  */
  createCollation(db, sqlite3_Str_ద్విమాన, SQLITE_UTF8, 0,
                  sqlite3_ద్విమాన_Compare, 0);
  createCollation(db, sqlite3_Str_ద్విమాన, SQLITE_UTF16BE, 0,
                  sqlite3_ద్విమాన_Compare, 0);
  createCollation(db, sqlite3_Str_ద్విమాన, SQLITE_UTF16LE, 0,
                  sqlite3_ద్విమాన_Compare, 0);
  createCollation(db, "NOCASE", SQLITE_UTF8, 0, nocaseCollatingFunc, 0);
  createCollation(db, "RTRIM", SQLITE_UTF8, 0, rtrimCollFunc, 0);
  ఒకవేళ( db->mallocFailed ){
    వెళ్ళు opendb_బహిర్గతం;
  }

#ఒకవేళ SQLITE_OS_UNIX && defined(SQLITE_OS_KV_OPTIONAL)
  /* Process magic filenames ":localStorage:" and ":sessionStorage:" */
  ఒకవేళ( zFilename && zFilename[0]==':' ){
    ఒకవేళ( strcmp(zFilename, ":localStorage:")==0 ){
      zFilename = "file:local?vfs=kvvfs";
      flags |= SQLITE_OPEN_URI;
    }లేకపోతే ఒకవేళ( strcmp(zFilename, ":sessionStorage:")==0 ){
      zFilename = "file:session?vfs=kvvfs";
      flags |= SQLITE_OPEN_URI;
    }
  }
#అయితే_ముగింపు /* SQLITE_OS_UNIX && defined(SQLITE_OS_KV_OPTIONAL) */

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
  ప్రతిజ్ఞా( SQLITE_OPEN_READONLY  == 0x01 );
  ప్రతిజ్ఞా( SQLITE_OPEN_READWRITE == 0x02 );
  ప్రతిజ్ఞా( SQLITE_OPEN_CREATE    == 0x04 );
  testcase( (1<<(flags&7))==0x02 ); /* READONLY */
  testcase( (1<<(flags&7))==0x04 ); /* READWRITE */
  testcase( (1<<(flags&7))==0x40 ); /* READWRITE | CREATE */
  ఒకవేళ( ((1<<(flags&7)) & 0x46)==0 ){
    rc = SQLITE_MISUSE_BKPT;  /* IMP: R-18321-05872 */
  }లేకపోతే{
    ఒకవేళ( zFilename==0 ) zFilename = ":memory:";
    rc = sqlite3ParseUri(zVfs, zFilename, &flags, &db->pVfs, &zOpen, &z_Err_సందేశం);
  }
  ఒకవేళ( rc!=SQLITE_OK ){
    ఒకవేళ( rc==SQLITE_NOMEM ) sqlite3OomFault(db);
    sqlite3_Error_With_సందేశం(db, rc, z_Err_సందేశం ? "%s" : 0, z_Err_సందేశం);
    sqlite3_free(z_Err_సందేశం);
    వెళ్ళు opendb_బహిర్గతం;
  }
  ప్రతిజ్ఞా( db->pVfs!=0 );
#ఒకవేళ SQLITE_OS_KV || defined(SQLITE_OS_KV_OPTIONAL)
  ఒకవేళ( sqlite3_stricmp(db->pVfs->zName, "kvvfs")==0 ){
    db->తాత్కాలిక_store = 2;
  }
#అయితే_ముగింపు

  /* Open the backend database driver */
  rc = sqlite3BtreeOpen(db->pVfs, zOpen, db, &db->aDb[0].pBt, 0,
                        flags | SQLITE_OPEN_MAIN_DB);
  ఒకవేళ( rc!=SQLITE_OK ){
    ఒకవేళ( rc==SQLITE_IOERR_NOMEM ){
      rc = SQLITE_NOMEM_BKPT;
    }
    sqlite3Error(db, rc);
    వెళ్ళు opendb_బహిర్గతం;
  }
  sqlite3BtreeEnter(db->aDb[0].pBt);
  db->aDb[0].pSchema = sqlite3SchemaGet(db, db->aDb[0].pBt);
  ఒకవేళ( !db->mallocFailed ){
    sqlite3_సమితి_పాఠ్యం_Encoding(db, SCHEMA_ENC(db));
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
  ఒకవేళ( db->mallocFailed ){
    వెళ్ళు opendb_బహిర్గతం;
  }

  /* Register all built-in functions, but do not attempt to read the
  ** database schema yet. This is delayed until the first time the database
  ** is accessed.
  */
  sqlite3Error(db, SQLITE_OK);
  sqlite3_రిజిస్టరు_చొప్పున_Connection_Builtin_Functions(db);
  rc = sqlite3_errcode(db);


  /* Load compiled-in extensions */
  కొరకు(i=0; rc==SQLITE_OK && i<Array_కొలత(sqlite3BuiltinExtensions); i++){
    rc = sqlite3BuiltinExtensions[i](db);
  }

  /* Load automatic extensions - extensions that have been registered
  ** using the sqlite3_automatic_extension() API.
  */
  ఒకవేళ( rc==SQLITE_OK ){
    sqlite3_Auto_ఎక్కించు_Extensions(db);
    rc = sqlite3_errcode(db);
    ఒకవేళ( rc!=SQLITE_OK ){
      వెళ్ళు opendb_బహిర్గతం;
    }
  }

#నిర్వచితమైతే SQLITE_ENABLE_అంతర్గత_FUNCTIONS
  /* Testing use only!!! The -DSQLITE_ENABLE_INTERNAL_FUNCTIONS=1 compile-time
  ** option gives access to internal functions by default. 
  ** Testing use only!!! */
  db->mDbFlags |= DBFLAG_అంతర్గత_Func;
#అయితే_ముగింపు

  /* -DSQLITE_DEFAULT_LOCKING_MODE=1 makes EXCLUSIVE the default locking
  ** mode.  -DSQLITE_DEFAULT_LOCKING_MODE=0 make NORMAL the default locking
  ** mode.  Doing nothing at all also makes NORMAL the default.
  */
#నిర్వచితమైతే SQLITE_DEFAULT_LOCKING_బహుళకం
  db->dflt_తాళం_బహుళకం = SQLITE_DEFAULT_LOCKING_బహుళకం;
  sqlite3_Pager_Locking_బహుళకం(sqlite3BtreePager(db->aDb[0].pBt),
                          SQLITE_DEFAULT_LOCKING_బహుళకం);
#అయితే_ముగింపు

  ఒకవేళ( rc ) sqlite3Error(db, rc);

  /* Enable the lookaside-malloc subsystem */
  అమరిక_Lookaside(db, 0, sqlite3GlobalConfig.szLookaside,
                        sqlite3GlobalConfig.nLookaside);

  sqlite3_wal_autocheckpoint(db, SQLITE_DEFAULT_WAL_AUTOCHECKPOINT);

opendb_బహిర్గతం:
  ఒకవేళ( db ){
    ప్రతిజ్ఞా( db->mutex!=0 || isThreadsafe==0
           || sqlite3GlobalConfig.b_నిండిన_Mutex==0 );
    sqlite3_mutex_leave(db->mutex);
  }
  rc = sqlite3_errcode(db);
  ప్రతిజ్ఞా( db!=0 || (rc&0xff)==SQLITE_NOMEM );
  ఒకవేళ( (rc&0xff)==SQLITE_NOMEM ){
    sqlite3_close(db);
    db = 0;
  }లేకపోతే ఒకవేళ( rc!=SQLITE_OK ){
    db->eOpenState = SQLITE_STATE_SICK;
  }
  *ppDb = db;
#నిర్వచితమైతే SQLITE_ENABLE_SQLLOG
  ఒకవేళ( sqlite3GlobalConfig.xSqllog ){
    /* Opening a db handle. Fourth parameter is passed 0. */
    శూన్య *pArg = sqlite3GlobalConfig.pSqllogArg;
    sqlite3GlobalConfig.xSqllog(pArg, db, zFilename, 0);
  }
#అయితే_ముగింపు
  sqlite3_free_filename(zOpen);
  తిరిగి rc;
}


/*
** Open a new database handle.
*/
పూర్ణాంకం sqlite3_open(
  స్థిరరాశి అక్షరం *zFilename,
  sqlite3 **ppDb
){
  తిరిగి openDatabase(zFilename, ppDb,
                      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, 0);
}
పూర్ణాంకం sqlite3_open_v2(
  స్థిరరాశి అక్షరం *filename,   /* Database filename (UTF-8) */
  sqlite3 **ppDb,         /* OUT: SQLite db handle */
  పూర్ణాంకం flags,              /* Flags */
  స్థిరరాశి అక్షరం *zVfs        /* Name of VFS module to use */
){
  తిరిగి openDatabase(filename, ppDb, (గుర్తులేని పూర్ణాంకం)flags, zVfs);
}

#నిర్వచితం_కాకపోతే SQLITE_OMIT_UTF16
/*
** Open a new database handle.
*/
పూర్ణాంకం sqlite3_open16(
  స్థిరరాశి శూన్య *zFilename,
  sqlite3 **ppDb
){
  అక్షరం స్థిరరాశి *zFilename8;   /* zFilename encoded in UTF-8 instead of UTF-16 */
  sqlite3_విలువ *pVal;
  పూర్ణాంకం rc;

#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( ppDb==0 ) తిరిగి SQLITE_MISUSE_BKPT;
#అయితే_ముగింపు
  *ppDb = 0;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOINIT
  rc = sqlite3_initialize();
  ఒకవేళ( rc ) తిరిగి rc;
#అయితే_ముగింపు
  ఒకవేళ( zFilename==0 ) zFilename = "\000\000";
  pVal = sqlite3_విలువ_New(0);
  sqlite3_విలువ_సమితి_Str(pVal, -1, zFilename, SQLITE_UTF16NATIVE, SQLITE_STATIC);
  zFilename8 = sqlite3_విలువ_పాఠ్యం_2(pVal, SQLITE_UTF8);
  ఒకవేళ( zFilename8 ){
    rc = openDatabase(zFilename8, ppDb,
                      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, 0);
    ప్రతిజ్ఞా( *ppDb || rc==SQLITE_NOMEM );
    ఒకవేళ( rc==SQLITE_OK && !DbHasProperty(*ppDb, 0, DB_SchemaLoaded) ){
      SCHEMA_ENC(*ppDb) = ENC(*ppDb) = SQLITE_UTF16NATIVE;
    }
  }లేకపోతే{
    rc = SQLITE_NOMEM_BKPT;
  }
  sqlite3_విలువ_Free(pVal);

  తిరిగి rc & 0xff;
}
#అయితే_ముగింపు /* SQLITE_OMIT_UTF16 */

/*
** Register a new collation sequence with the database handle db.
*/
పూర్ణాంకం sqlite3_create_collation(
  sqlite3* db,
  స్థిరరాశి అక్షరం *zName,
  పూర్ణాంకం enc,
  శూన్య* pCtx,
  పూర్ణాంకం(*xCompare)(శూన్య*,పూర్ణాంకం,స్థిరరాశి శూన్య*,పూర్ణాంకం,స్థిరరాశి శూన్య*)
){
  తిరిగి sqlite3_create_collation_v2(db, zName, enc, pCtx, xCompare, 0);
}

/*
** Register a new collation sequence with the database handle db.
*/
పూర్ణాంకం sqlite3_create_collation_v2(
  sqlite3* db,
  స్థిరరాశి అక్షరం *zName,
  పూర్ణాంకం enc,
  శూన్య* pCtx,
  పూర్ణాంకం(*xCompare)(శూన్య*,పూర్ణాంకం,స్థిరరాశి శూన్య*,పూర్ణాంకం,స్థిరరాశి శూన్య*),
  శూన్య(*xDel)(శూన్య*)
){
  పూర్ణాంకం rc;

#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !sqlite3SafetyCheckOk(db) || zName==0 ) తిరిగి SQLITE_MISUSE_BKPT;
#అయితే_ముగింపు
  sqlite3_mutex_enter(db->mutex);
  ప్రతిజ్ఞా( !db->mallocFailed );
  rc = createCollation(db, zName, (u8)enc, pCtx, xCompare, xDel);
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  తిరిగి rc;
}

#నిర్వచితం_కాకపోతే SQLITE_OMIT_UTF16
/*
** Register a new collation sequence with the database handle db.
*/
పూర్ణాంకం sqlite3_create_collation16(
  sqlite3* db,
  స్థిరరాశి శూన్య *zName,
  పూర్ణాంకం enc,
  శూన్య* pCtx,
  పూర్ణాంకం(*xCompare)(శూన్య*,పూర్ణాంకం,స్థిరరాశి శూన్య*,పూర్ణాంకం,స్థిరరాశి శూన్య*)
){
  పూర్ణాంకం rc = SQLITE_OK;
  అక్షరం *zName8;

#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !sqlite3SafetyCheckOk(db) || zName==0 ) తిరిగి SQLITE_MISUSE_BKPT;
#అయితే_ముగింపు
  sqlite3_mutex_enter(db->mutex);
  ప్రతిజ్ఞా( !db->mallocFailed );
  zName8 = sqlite3Utf16to8(db, zName, -1, SQLITE_UTF16NATIVE);
  ఒకవేళ( zName8 ){
    rc = createCollation(db, zName8, (u8)enc, pCtx, xCompare, 0);
    sqlite3DbFree(db, zName8);
  }
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  తిరిగి rc;
}
#అయితే_ముగింపు /* SQLITE_OMIT_UTF16 */

/*
** Register a collation sequence factory callback with the database handle
** db. Replace any previously installed collation sequence factory.
*/
పూర్ణాంకం sqlite3_collation_needed(
  sqlite3 *db,
  శూన్య *pCollNeededArg,
  శూన్య(*xCollNeeded)(శూన్య*,sqlite3*,పూర్ణాంకం e_పాఠ్యం_Rep,స్థిరరాశి అక్షరం*)
){
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !sqlite3SafetyCheckOk(db) ) తిరిగి SQLITE_MISUSE_BKPT;
#అయితే_ముగింపు
  sqlite3_mutex_enter(db->mutex);
  db->xCollNeeded = xCollNeeded;
  db->xCollNeeded16 = 0;
  db->pCollNeededArg = pCollNeededArg;
  sqlite3_mutex_leave(db->mutex);
  తిరిగి SQLITE_OK;
}

#నిర్వచితం_కాకపోతే SQLITE_OMIT_UTF16
/*
** Register a collation sequence factory callback with the database handle
** db. Replace any previously installed collation sequence factory.
*/
పూర్ణాంకం sqlite3_collation_needed16(
  sqlite3 *db,
  శూన్య *pCollNeededArg,
  శూన్య(*xCollNeeded16)(శూన్య*,sqlite3*,పూర్ణాంకం e_పాఠ్యం_Rep,స్థిరరాశి శూన్య*)
){
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !sqlite3SafetyCheckOk(db) ) తిరిగి SQLITE_MISUSE_BKPT;
#అయితే_ముగింపు
  sqlite3_mutex_enter(db->mutex);
  db->xCollNeeded = 0;
  db->xCollNeeded16 = xCollNeeded16;
  db->pCollNeededArg = pCollNeededArg;
  sqlite3_mutex_leave(db->mutex);
  తిరిగి SQLITE_OK;
}
#అయితే_ముగింపు /* SQLITE_OMIT_UTF16 */

/*
** Find existing client data.
*/
శూన్య *sqlite3_get_clientdata(sqlite3 *db, స్థిరరాశి అక్షరం *zName){
  Db_Client_దత్తాంశం *p;
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !zName || !sqlite3SafetyCheckOk(db) ){
    (శూన్య)SQLITE_MISUSE_BKPT;
    తిరిగి 0;
  }
#అయితే_ముగింపు
  sqlite3_mutex_enter(db->mutex);
  కొరకు(p=db->p_Db_దత్తాంశం; p; p=p->p_తదుపరి){
    ఒకవేళ( strcmp(p->zName, zName)==0 ){
      శూన్య *p_ఫలితం = p->p_దత్తాంశం;
      sqlite3_mutex_leave(db->mutex);
      తిరిగి p_ఫలితం;
    }
  }
  sqlite3_mutex_leave(db->mutex);
  తిరిగి 0;
}

/*
** Add new client data to a database connection.
*/
పూర్ణాంకం sqlite3_సమితి_clientdata(
  sqlite3 *db,                   /* Attach client data to this connection */
  స్థిరరాశి అక్షరం *zName,             /* Name of the client data */
  శూన్య *p_దత్తాంశం,                   /* The client data itself */
  శూన్య (*xDestructor)(శూన్య*)     /* Destructor */
){
  Db_Client_దత్తాంశం *p, **pp;
  sqlite3_mutex_enter(db->mutex);
  pp = &db->p_Db_దత్తాంశం;
  కొరకు(p=db->p_Db_దత్తాంశం; p && strcmp(p->zName,zName); p=p->p_తదుపరి){
    pp = &p->p_తదుపరి;
  }
  ఒకవేళ( p ){
    ప్రతిజ్ఞా( p->p_దత్తాంశం!=0 );
    ఒకవేళ( p->xDestructor ) p->xDestructor(p->p_దత్తాంశం);
    ఒకవేళ( p_దత్తాంశం==0 ){
      *pp = p->p_తదుపరి;
      sqlite3_free(p);
      sqlite3_mutex_leave(db->mutex);
      తిరిగి SQLITE_OK;
    }
  }లేకపోతే ఒకవేళ( p_దత్తాంశం==0 ){
    sqlite3_mutex_leave(db->mutex);
    తిరిగి SQLITE_OK;
  }లేకపోతే{
    size_t n = అక్షరశ్రేణి_పొడవు(zName);
    p = sqlite3_malloc64( SZ_DBCLIENTDATA(n+1) );
    ఒకవేళ( p==0 ){
      ఒకవేళ( xDestructor ) xDestructor(p_దత్తాంశం);
      sqlite3_mutex_leave(db->mutex);
      తిరిగి SQLITE_NOMEM;
    }
    జ్ఞాపకం_నకలు(p->zName, zName, n+1);
    p->p_తదుపరి = db->p_Db_దత్తాంశం;
    db->p_Db_దత్తాంశం = p;
  }
  p->p_దత్తాంశం = p_దత్తాంశం;
  p->xDestructor = xDestructor;
  sqlite3_mutex_leave(db->mutex);
  తిరిగి SQLITE_OK;
}


#నిర్వచితం_కాకపోతే SQLITE_OMIT_DEPRECATED
/*
** This function is now an anachronism. It used to be used to recover from a
** malloc() failure, but SQLite now does this automatically.
*/
పూర్ణాంకం sqlite3_global_recover(శూన్య){
  తిరిగి SQLITE_OK;
}
#అయితే_ముగింపు

/*
** Test to see whether or not the database connection is in autocommit
** mode.  Return TRUE if it is and FALSE if not.  Autocommit mode is on
** by default.  Autocommit is disabled by a BEGIN statement and reenabled
** by the next COMMIT or ROLLBACK.
*/
పూర్ణాంకం sqlite3_get_autocommit(sqlite3 *db){
  పూర్ణాంకం iRet;
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !sqlite3SafetyCheckOk(db) ){
    (శూన్య)SQLITE_MISUSE_BKPT;
    తిరిగి 0;
  }
#అయితే_ముగింపు
  sqlite3_mutex_enter(db->mutex);
  iRet = db->autoCommit;
  sqlite3_mutex_leave(db->mutex);
  తిరిగి iRet;
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
పూర్ణాంకం sqlite3ReportError(పూర్ణాంకం iErr, పూర్ణాంకం lineno, స్థిరరాశి అక్షరం *zType){
  sqlite3_log(iErr, "%s at line %d of [%.10s]",
              zType, lineno, 20+sqlite3_sourceid());
  తిరిగి iErr;
}
పూర్ణాంకం sqlite3CorruptError(పూర్ణాంకం lineno){
  testcase( sqlite3GlobalConfig.xLog!=0 );
  తిరిగి sqlite3ReportError(SQLITE_CORRUPT, lineno, "database corruption");
}
పూర్ణాంకం sqlite3MisuseError(పూర్ణాంకం lineno){
  testcase( sqlite3GlobalConfig.xLog!=0 );
  తిరిగి sqlite3ReportError(SQLITE_MISUSE, lineno, "misuse");
}
పూర్ణాంకం sqlite3CantopenError(పూర్ణాంకం lineno){
  testcase( sqlite3GlobalConfig.xLog!=0 );
  తిరిగి sqlite3ReportError(SQLITE_CANTOPEN, lineno, "cannot open file");
}
#ఒకవేళ defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_CORRUPT_PGNO)
పూర్ణాంకం sqlite3CorruptPgnoError(పూర్ణాంకం lineno, Pgno pgno){
  అక్షరం z_సందేశం[100];
  sqlite3_snprintf(పరిమాణం(z_సందేశం), z_సందేశం, "database corruption page %d", pgno);
  testcase( sqlite3GlobalConfig.xLog!=0 );
  తిరిగి sqlite3ReportError(SQLITE_CORRUPT, lineno, z_సందేశం);
}
#అయితే_ముగింపు
#నిర్వచితమైతే SQLITE_DEBUG
పూర్ణాంకం sqlite3NomemError(పూర్ణాంకం lineno){
  testcase( sqlite3GlobalConfig.xLog!=0 );
  తిరిగి sqlite3ReportError(SQLITE_NOMEM, lineno, "OOM");
}
పూర్ణాంకం sqlite3IoerrnomemError(పూర్ణాంకం lineno){
  testcase( sqlite3GlobalConfig.xLog!=0 );
  తిరిగి sqlite3ReportError(SQLITE_IOERR_NOMEM, lineno, "I/O OOM error");
}
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే SQLITE_OMIT_DEPRECATED
/*
** This is a convenience routine that makes sure that all thread-specific
** data for this thread has been deallocated.
**
** SQLite no longer uses thread-specific data so this routine is now a
** no-op.  It is retained for historical compatibility.
*/
శూన్య sqlite3_thread_cleanup(శూన్య){
}
#అయితే_ముగింపు

/*
** Return meta information about a specific column of a database table.
** See comment in sqlite3.h (sqlite.h.in) for details.
*/
పూర్ణాంకం sqlite3_పట్టిక_స్తంభం_metadata(
  sqlite3 *db,                /* Connection handle */
  స్థిరరాశి అక్షరం *zDbName,        /* Database name or NULL */
  స్థిరరాశి అక్షరం *z_పట్టిక_Name,     /* Table name */
  స్థిరరాశి అక్షరం *z_స్తంభం_Name,    /* Column name */
  అక్షరం స్థిరరాశి **pz_దత్తాంశం_Type,    /* OUTPUT: Declared data type */
  అక్షరం స్థిరరాశి **pzCollSeq,     /* OUTPUT: Collation sequence name */
  పూర్ణాంకం *pNotNull,              /* OUTPUT: True if NOT NULL constraint exists */
  పూర్ణాంకం *p_Primary_కీలకం,           /* OUTPUT: True if column part of PK */
  పూర్ణాంకం *pAutoinc               /* OUTPUT: True if column is auto-increment */
){
  పూర్ణాంకం rc;
  అక్షరం *z_Err_సందేశం = 0;
  పట్టిక *pTab = 0;
  స్తంభం *p_నిలువు = 0;
  పూర్ణాంకం i_నిలువు = 0;
  అక్షరం స్థిరరాశి *z_దత్తాంశం_Type = 0;
  అక్షరం స్థిరరాశి *zCollSeq = 0;
  పూర్ణాంకం notnull = 0;
  పూర్ణాంకం primarykey = 0;
  పూర్ణాంకం autoinc = 0;


#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !sqlite3SafetyCheckOk(db) || z_పట్టిక_Name==0 ){
    తిరిగి SQLITE_MISUSE_BKPT;
  }
#అయితే_ముగింపు

  /* Ensure the database schema has been loaded */
  sqlite3_mutex_enter(db->mutex);
  sqlite3BtreeEnterAll(db);
  rc = sqlite3Init(db, &z_Err_సందేశం);
  ఒకవేళ( SQLITE_OK!=rc ){
    వెళ్ళు error_బహిర్గతం;
  }

  /* Locate the table in question */
  pTab = sqlite3_కనుగొను_పట్టిక(db, z_పట్టిక_Name, zDbName);
  ఒకవేళ( !pTab || IsView(pTab) ){
    pTab = 0;
    వెళ్ళు error_బహిర్గతం;
  }

  /* Find the column for which info is requested */
  ఒకవేళ( z_స్తంభం_Name==0 ){
    /* Query for existence of table only */
  }లేకపోతే{
    i_నిలువు = sqlite3_స్తంభం_సూచీ(pTab, z_స్తంభం_Name);
    ఒకవేళ( i_నిలువు>=0 ){
      p_నిలువు = &pTab->a_నిలువు[i_నిలువు];
    }లేకపోతే{
      ఒకవేళ( HasRowid(pTab) && sqlite3IsRowid(z_స్తంభం_Name) ){
        i_నిలువు = pTab->iPKey;
        p_నిలువు = i_నిలువు>=0 ? &pTab->a_నిలువు[i_నిలువు] : 0;
      }లేకపోతే{
        pTab = 0;
        వెళ్ళు error_బహిర్గతం;
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
  ఒకవేళ( p_నిలువు ){
    z_దత్తాంశం_Type = sqlite3_స్తంభం_Type(p_నిలువు,0);
    zCollSeq = sqlite3_స్తంభం_Coll(p_నిలువు);
    notnull = p_నిలువు->notNull!=0;
    primarykey  = (p_నిలువు->నిలువు_Flags & COLFLAG_PRIMKEY)!=0;
    autoinc = pTab->iPKey==i_నిలువు && (pTab->tabFlags & TF_Autoincrement)!=0;
  }లేకపోతే{
    z_దత్తాంశం_Type = "INTEGER";
    primarykey = 1;
  }
  ఒకవేళ( !zCollSeq ){
    zCollSeq = sqlite3_Str_ద్విమాన;
  }

error_బహిర్గతం:
  sqlite3BtreeLeaveAll(db);

  /* Whether the function call succeeded or failed, set the output parameters
  ** to whatever their local counterparts contain. If an error did occur,
  ** this has the effect of zeroing all output parameters.
  */
  ఒకవేళ( pz_దత్తాంశం_Type ) *pz_దత్తాంశం_Type = z_దత్తాంశం_Type;
  ఒకవేళ( pzCollSeq ) *pzCollSeq = zCollSeq;
  ఒకవేళ( pNotNull ) *pNotNull = notnull;
  ఒకవేళ( p_Primary_కీలకం ) *p_Primary_కీలకం = primarykey;
  ఒకవేళ( pAutoinc ) *pAutoinc = autoinc;

  ఒకవేళ( SQLITE_OK==rc && !pTab ){
    sqlite3DbFree(db, z_Err_సందేశం);
    z_Err_సందేశం = sqlite3MPrintf(db, "no such table column: %s.%s", z_పట్టిక_Name,
        z_స్తంభం_Name);
    rc = SQLITE_ERROR;
  }
  sqlite3_Error_With_సందేశం(db, rc, (z_Err_సందేశం?"%s":0), z_Err_సందేశం);
  sqlite3DbFree(db, z_Err_సందేశం);
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  తిరిగి rc;
}

/*
** Sleep for a little while.  Return the amount of time slept.
*/
పూర్ణాంకం sqlite3_sleep(పూర్ణాంకం ms){
  sqlite3_vfs *pVfs;
  పూర్ణాంకం rc;
  pVfs = sqlite3_vfs_కనుగొను(0);
  ఒకవేళ( pVfs==0 ) తిరిగి 0;

  /* This function works in milliseconds, but the underlying OsSleep()
  ** API uses microseconds. Hence the 1000's.
  */
  rc = (sqlite3OsSleep(pVfs, ms<0 ? 0 : 1000*ms)/1000);
  తిరిగి rc;
}

/*
** Enable or disable the extended result codes.
*/
పూర్ణాంకం sqlite3_extended_ఫలితం_codes(sqlite3 *db, పూర్ణాంకం onoff){
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !sqlite3SafetyCheckOk(db) ) తిరిగి SQLITE_MISUSE_BKPT;
#అయితే_ముగింపు
  sqlite3_mutex_enter(db->mutex);
  db->err_ముసుగు = onoff ? 0xffffffff : 0xff;
  sqlite3_mutex_leave(db->mutex);
  తిరిగి SQLITE_OK;
}

/*
** Invoke the xFileControl method on a particular database.
*/
పూర్ణాంకం sqlite3_file_control(sqlite3 *db, స్థిరరాశి అక్షరం *zDbName, పూర్ణాంకం op, శూన్య *pArg){
  పూర్ణాంకం rc = SQLITE_ERROR;
  Btree *pBtree;

#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !sqlite3SafetyCheckOk(db) ) తిరిగి SQLITE_MISUSE_BKPT;
#అయితే_ముగింపు
  sqlite3_mutex_enter(db->mutex);
  pBtree = sqlite3_Db_Name_గమ్యం_Btree(db, zDbName);
  ఒకవేళ( pBtree ){
    Pager *pPager;
    sqlite3_file *fd;
    sqlite3BtreeEnter(pBtree);
    pPager = sqlite3BtreePager(pBtree);
    ప్రతిజ్ఞా( pPager!=0 );
    fd = sqlite3PagerFile(pPager);
    ప్రతిజ్ఞా( fd!=0 );
    ఒకవేళ( op==SQLITE_FCNTL_FILE_సూచిక ){
      *(sqlite3_file**)pArg = fd;
      rc = SQLITE_OK;
    }లేకపోతే ఒకవేళ( op==SQLITE_FCNTL_VFS_సూచిక ){
      *(sqlite3_vfs**)pArg = sqlite3PagerVfs(pPager);
      rc = SQLITE_OK;
    }లేకపోతే ఒకవేళ( op==SQLITE_FCNTL_JOURNAL_సూచిక ){
      *(sqlite3_file**)pArg = sqlite3PagerJrnlFile(pPager);
      rc = SQLITE_OK;
    }లేకపోతే ఒకవేళ( op==SQLITE_FCNTL_దత్తాంశం_సంస్కరణ ){
      *(గుర్తులేని పూర్ణాంకం*)pArg = sqlite3_Pager_దత్తాంశం_సంస్కరణ(pPager);
      rc = SQLITE_OK;
    }లేకపోతే ఒకవేళ( op==SQLITE_FCNTL_RESERVE_బైట్లు ){
      పూర్ణాంకం iNew = *(పూర్ణాంకం*)pArg;
      *(పూర్ణాంకం*)pArg = sqlite3BtreeGetRequestedReserve(pBtree);
      ఒకవేళ( iNew>=0 && iNew<=255 ){
        sqlite3_Btree_సమితి_పుట_కొలత(pBtree, 0, iNew, 0);
      }
      rc = SQLITE_OK;
    }లేకపోతే ఒకవేళ( op==SQLITE_FCNTL_RESET_CACHE ){
      sqlite3BtreeClearCache(pBtree);
      rc = SQLITE_OK;
    }లేకపోతే{
      పూర్ణాంకం nSave = db->busyHandler.nBusy;
      rc = sqlite3OsFileControl(fd, op, pArg);
      db->busyHandler.nBusy = nSave;
    }
    sqlite3BtreeLeave(pBtree);
  }
  sqlite3_mutex_leave(db->mutex);
  తిరిగి rc;
}

/*
** Interface to the testing logic.
*/
పూర్ణాంకం sqlite3_test_control(పూర్ణాంకం op, ...){
  పూర్ణాంకం rc = 0;
#నిర్వచితమైతే SQLITE_UNTESTABLE
  వాడని_PARAMETER(op);
#లేకపోతే
  va_list ap;
  va_start(ap, op);
  ఎంపిక( op ){

    /*
    ** Save the current state of the PRNG.
    */
    స్థితి SQLITE_TESTCTRL_PRNG_SAVE: {
      sqlite3PrngSaveState();
      ఆపు;
    }

    /*
    ** Restore the state of the PRNG to the last state saved using
    ** PRNG_SAVE.  If PRNG_SAVE has never before been called, then
    ** this verb acts like PRNG_RESET.
    */
    స్థితి SQLITE_TESTCTRL_PRNG_RESTORE: {
      sqlite3PrngRestoreState();
      ఆపు;
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
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WSD
    స్థితి SQLITE_TESTCTRL_PRNG_విత్తనం: {
      పూర్ణాంకం x = va_arg(ap, పూర్ణాంకం);
      పూర్ణాంకం y;
      sqlite3 *db = va_arg(ap, sqlite3*);
      ప్రతిజ్ఞా( db==0 || db->aDb[0].pSchema!=0 );
      ఒకవేళ( db && (y = db->aDb[0].pSchema->schema_cookie)!=0 ){ x = y; }
      sqlite3Config.i_Prng_విత్తనం = x;
      sqlite3_randomness(0,0);
      ఆపు;
    }
#అయితే_ముగింపు

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
    స్థితి SQLITE_TESTCTRL_FK_NO_ACTION: {
      sqlite3 *db = va_arg(ap, sqlite3*);
      పూర్ణాంకం b = va_arg(ap, పూర్ణాంకం);
      ఒకవేళ( b ){
        db->flags |= SQLITE_FkNoAction;
      }లేకపోతే{
        db->flags &= ~SQLITE_FkNoAction;
      }
      ఆపు;
    }

    /*
    **  sqlite3_test_control(BITVEC_TEST, size, program)
    **
    ** Run a test against a Bitvec object of size.  The program argument
    ** is an array of integers that defines the test.  Return -1 on a
    ** memory allocation error, 0 on success, or non-zero for an error.
    ** See the sqlite3BitvecBuiltinTest() for additional information.
    */
    స్థితి SQLITE_TESTCTRL_BITVEC_TEST: {
      పూర్ణాంకం sz = va_arg(ap, పూర్ణాంకం);
      పూర్ణాంకం *aProg = va_arg(ap, పూర్ణాంకం*);
      rc = sqlite3BitvecBuiltinTest(sz, aProg);
      ఆపు;
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
    స్థితి SQLITE_TESTCTRL_FAULT_INSTALL: {
      /* A bug in MSVC prevents it from understanding pointers to functions
      ** types in the second argument to va_arg().  Work around the problem
      ** using a typedef.
      ** http://support.microsoft.com/kb/47961  <-- dead hyperlink
      ** Search at http://web.archive.org/ to find the 2015-03-16 archive
      ** of the link above to see the original text.
      ** sqlite3GlobalConfig.xTestCallback = va_arg(ap, int(*)(int));
      */
      రకనిర్వచనం పూర్ణాంకం(*sqlite3FaultFuncType)(పూర్ణాంకం);
      sqlite3GlobalConfig.xTestCallback = va_arg(ap, sqlite3FaultFuncType);
      rc = sqlite3FaultSim(0);
      ఆపు;
    }

    /*
    **  sqlite3_test_control(BENIGN_MALLOC_HOOKS, xBegin, xEnd)
    **
    ** Register hooks to call to indicate which malloc() failures
    ** are benign.
    */
    స్థితి SQLITE_TESTCTRL_BENIGN_MALLOC_HOOKS: {
      రకనిర్వచనం శూన్య (*void_function)(శూన్య);
      void_function xBenignBegin;
      void_function x_Benign_చివర;
      xBenignBegin = va_arg(ap, void_function);
      x_Benign_చివర = va_arg(ap, void_function);
      sqlite3BenignMallocHooks(xBenignBegin, x_Benign_చివర);
      ఆపు;
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
    స్థితి SQLITE_TESTCTRL_PENDING_BYTE: {
      rc = PENDING_BYTE;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WSD
      {
        గుర్తులేని పూర్ణాంకం newVal = va_arg(ap, గుర్తులేని పూర్ణాంకం);
        ఒకవేళ( newVal ) sqlite3PendingByte = newVal;
      }
#అయితే_ముగింపు
      ఆపు;
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
    స్థితి SQLITE_TESTCTRL_ASSERT: {
      అస్థిర పూర్ణాంకం x = 0;
      ప్రతిజ్ఞా( /*side-effects-ok*/ (x = va_arg(ap,పూర్ణాంకం))!=0 );
      rc = x;
#ఒకవేళ defined(SQLITE_DEBUG)
      /* Invoke these debugging routines so that the compiler does not
      ** issue "defined but not used" warnings. */
      ఒకవేళ( x==9999 ){
        sqlite3_Show_సమాసం(0);
        sqlite3_Show_సమాసం_List(0);
        sqlite3ShowIdList(0);
        sqlite3ShowSrcList(0);
        sqlite3ShowWith(0);
        sqlite3ShowUpsert(0);
#నిర్వచితం_కాకపోతే SQLITE_OMIT_TRIGGER
        sqlite3_Show_Trigger_అడుగు(0);
        sqlite3_Show_Trigger_అడుగు_List(0);
        sqlite3ShowTrigger(0);
        sqlite3ShowTriggerList(0);
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
        sqlite3_Show_కిటికీ(0);
        sqlite3ShowWinFunc(0);
#అయితే_ముగింపు
        sqlite3ShowSelect(0);
      }
#అయితే_ముగింపు
      ఆపు;
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
    స్థితి SQLITE_TESTCTRL_ALWAYS: {
      పూర్ణాంకం x = va_arg(ap,పూర్ణాంకం);
      rc = x ? ALWAYS(x) : 0;
      ఆపు;
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
    స్థితి SQLITE_TESTCTRL_BYTEORDER: {
      rc = SQLITE_BYTEORDER*100 + SQLITE_LITTLEENDIAN*10 + SQLITE_BIGENDIAN;
      ఆపు;
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
    స్థితి SQLITE_TESTCTRL_OPTIMIZATIONS: {
      sqlite3 *db = va_arg(ap, sqlite3*);
      db->dbOptFlags = va_arg(ap, u32);
      ఆపు;
    }

    /*  sqlite3_test_control(SQLITE_TESTCTRL_GETOPT, sqlite3 *db, int *N)
    **
    ** Write the current optimization settings into *N.  A zero bit means that
    ** the optimization is on, and a 1 bit means that the optimization is off.
    */
    స్థితి SQLITE_TESTCTRL_GETOPT: {
      sqlite3 *db = va_arg(ap, sqlite3*);
      పూర్ణాంకం *pN = va_arg(ap, పూర్ణాంకం*);
      *pN = db->dbOptFlags;
      ఆపు;
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
    స్థితి SQLITE_TESTCTRL_LOCALTIME_FAULT: {
      sqlite3GlobalConfig.bLocaltimeFault = va_arg(ap, పూర్ణాంకం);
      ఒకవేళ( sqlite3GlobalConfig.bLocaltimeFault==2 ){
        రకనిర్వచనం పూర్ణాంకం(*sqlite3LocaltimeType)(స్థిరరాశి శూన్య*,శూన్య*);
        sqlite3GlobalConfig.xAltLocaltime = va_arg(ap, sqlite3LocaltimeType);
      }లేకపోతే{
        sqlite3GlobalConfig.xAltLocaltime = 0;
      }
      ఆపు;
    }

    /*   sqlite3_test_control(SQLITE_TESTCTRL_INTERNAL_FUNCTIONS, sqlite3*);
    **
    ** Toggle the ability to use internal functions on or off for
    ** the database connection given in the argument.
    */
    స్థితి SQLITE_TESTCTRL_అంతర్గత_FUNCTIONS: {
      sqlite3 *db = va_arg(ap, sqlite3*);
      db->mDbFlags ^= DBFLAG_అంతర్గత_Func;
      ఆపు;
    }

    /*   sqlite3_test_control(SQLITE_TESTCTRL_NEVER_CORRUPT, int);
    **
    ** Set or clear a flag that indicates that the database file is always well-
    ** formed and never corrupt.  This flag is clear by default, indicating that
    ** database files might have arbitrary corruption.  Setting the flag during
    ** testing causes certain assert() statements in the code to be activated
    ** that demonstrate invariants on well-formed database files.
    */
    స్థితి SQLITE_TESTCTRL_NEVER_CORRUPT: {
      sqlite3GlobalConfig.neverCorrupt = va_arg(ap, పూర్ణాంకం);
      ఆపు;
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
    స్థితి SQLITE_TESTCTRL_EXTRA_SCHEMA_CHECKS: {
      sqlite3GlobalConfig.bExtraSchemaChecks = va_arg(ap, పూర్ణాంకం);
      ఆపు;
    }

    /* Set the threshold at which OP_Once counters reset back to zero.
    ** By default this is 0x7ffffffe (over 2 billion), but that value is
    ** too big to test in a reasonable amount of time, so this control is
    ** provided to set a small and easily reachable reset value.
    */
    స్థితి SQLITE_TESTCTRL_ONCE_RESET_పరిమితి: {
      sqlite3GlobalConfig.i_Once_Reset_పరిమితి = va_arg(ap, పూర్ణాంకం);
      ఆపు;
    }

    /*   sqlite3_test_control(SQLITE_TESTCTRL_VDBE_COVERAGE, xCallback, ptr);
    **
    ** Set the VDBE coverage callback function to xCallback with context
    ** pointer ptr.
    */
    స్థితి SQLITE_TESTCTRL_VDBE_COVERAGE: {
#నిర్వచితమైతే SQLITE_VDBE_COVERAGE
      రకనిర్వచనం శూన్య (*branch_callback)(శూన్య*,గుర్తులేని పూర్ణాంకం,
                                      గుర్తులేని అక్షరం,గుర్తులేని అక్షరం);
      sqlite3GlobalConfig.xVdbeBranch = va_arg(ap,branch_callback);
      sqlite3GlobalConfig.pVdbeBranchArg = va_arg(ap,శూన్య*);
#అయితే_ముగింపు
      ఆపు;
    }

    /*   sqlite3_test_control(SQLITE_TESTCTRL_SORTER_MMAP, db, nMax); */
    స్థితి SQLITE_TESTCTRL_SORTER_MMAP: {
      sqlite3 *db = va_arg(ap, sqlite3*);
      db->nMaxSorterMmap = va_arg(ap, పూర్ణాంకం);
      ఆపు;
    }

    /*   sqlite3_test_control(SQLITE_TESTCTRL_ISINIT);
    **
    ** Return SQLITE_OK if SQLite has been initialized and SQLITE_ERROR if
    ** not.
    */
    స్థితి SQLITE_TESTCTRL_ISINIT: {
      ఒకవేళ( sqlite3GlobalConfig.isInit==0 ) rc = SQLITE_ERROR;
      ఆపు;
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
    స్థితి SQLITE_TESTCTRL_IMPOSTER: {
      sqlite3 *db = va_arg(ap, sqlite3*);
      పూర్ణాంకం iDb;
      sqlite3_mutex_enter(db->mutex);
      iDb = sqlite3_కనుగొను_Db_Name(db, va_arg(ap,స్థిరరాశి అక్షరం*));
      ఒకవేళ( iDb>=0 ){
        db->init.iDb = iDb;
        db->init.busy = db->init.imposter_పట్టిక = va_arg(ap,పూర్ణాంకం);
        db->init.newTnum = va_arg(ap,పూర్ణాంకం);
        ఒకవేళ( db->init.busy==0 && db->init.newTnum>0 ){
          sqlite3_Reset_All_Schemas_యొక్క_Connection(db);
        }
      }
      sqlite3_mutex_leave(db->mutex);
      ఆపు;
    }

#ఒకవేళ defined(YYCOVERAGE)
    /*  sqlite3_test_control(SQLITE_TESTCTRL_PARSER_COVERAGE, FILE *out)
    **
    ** This test control (only available when SQLite is compiled with
    ** -DYYCOVERAGE) writes a report onto "out" that shows all
    ** state/lookahead combinations in the parser state machine
    ** which are never exercised.  If any state is missed, make the
    ** return code SQLITE_ERROR.
    */
    స్థితి SQLITE_TESTCTRL_PARSER_COVERAGE: {
      FILE *బహిర్గతం = va_arg(ap, FILE*);
      ఒకవేళ( sqlite3ParserCoverage(బహిర్గతం) ) rc = SQLITE_ERROR;
      ఆపు;
    }
#అయితే_ముగింపు /* defined(YYCOVERAGE) */

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
    స్థితి SQLITE_TESTCTRL_ఫలితం_INTREAL: {
      sqlite3_context *pCtx = va_arg(ap, sqlite3_context*);
      sqlite3_ఫలితం_Int_Real(pCtx);
      ఆపు;
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
    స్థితి SQLITE_TESTCTRL_SEEK_లెక్క: {
      sqlite3 *db = va_arg(ap, sqlite3*);
      u64 *pn = va_arg(ap, sqlite3_uint64*);
      *pn = sqlite3_Btree_Seek_లెక్క(db->aDb->pBt);
      (శూన్య)db;  /* Silence harmless unused variable warning */
      ఆపు;
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
    స్థితి SQLITE_TESTCTRL_TRACEFLAGS: {
       పూర్ణాంకం opTrace = va_arg(ap, పూర్ణాంకం);
       u32 *ptr = va_arg(ap, u32*);
       ఎంపిక( opTrace ){
         స్థితి 0:   *ptr = sqlite3_వృక్షం_Trace;      ఆపు;
         స్థితి 1:   sqlite3_వృక్షం_Trace = *ptr;      ఆపు;
         స్థితి 2:   *ptr = sqlite3WhereTrace;     ఆపు;
         స్థితి 3:   sqlite3WhereTrace = *ptr;     ఆపు;
       }
       ఆపు;
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
    స్థితి SQLITE_TESTCTRL_LOGEST: {
      ద్విగుణ r_లోపల = va_arg(ap, ద్విగుణ);
      LogEst rLogEst = sqlite3LogEstFromDouble(r_లోపల);
      పూర్ణాంకం *pI1 = va_arg(ap,పూర్ణాంకం*);
      u64 *pU64 = va_arg(ap,u64*);
      పూర్ణాంకం *pI2 = va_arg(ap,పూర్ణాంకం*);
      *pI1 = rLogEst;
      *pU64 = sqlite3_Log_Est_గమ్యం_Int(rLogEst);
      *pI2 = sqlite3LogEst(*pU64);
      ఆపు;
    }

    /* sqlite3_test_control(SQLITE_TESTCTRL_ATOF, const char *z, double *p);
    **
    ** Test access to the sqlite3AtoF() routine.
    */
    స్థితి SQLITE_TESTCTRL_ATOF: {
      స్థిరరాశి అక్షరం *z = va_arg(ap,స్థిరరాశి అక్షరం*);
      ద్విగుణ *pR = va_arg(ap,ద్విగుణ*);
      rc = sqlite3AtoF(z,pR);
      ఆపు;
    }

#ఒకవేళ defined(SQLITE_DEBUG) && !defined(SQLITE_OMIT_WSD)
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
    స్థితి SQLITE_TESTCTRL_TUNE: {
      పూర్ణాంకం id = va_arg(ap, పూర్ణాంకం);
      పూర్ణాంకం *pi_విలువ = va_arg(ap, పూర్ణాంకం*);
      ఒకవేళ( id>0 && id<=SQLITE_NTUNE ){
        Tuning(id) = *pi_విలువ;
      }లేకపోతే ఒకవేళ( id<0 && id>=-SQLITE_NTUNE ){
        *pi_విలువ = Tuning(-id);
      }లేకపోతే{
        rc = SQLITE_NOTFOUND;
      }
      ఆపు;
    }
#అయితే_ముగింపు

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
    స్థితి SQLITE_TESTCTRL_JSON_SELFCHECK: {
#ఒకవేళ defined(SQLITE_DEBUG) && !defined(SQLITE_OMIT_WSD)
      పూర్ణాంకం *pOnOff = va_arg(ap, పూర్ణాంకం*);
      ఒకవేళ( *pOnOff<0 ){
        *pOnOff = sqlite3Config.bJsonSelfcheck;
      }లేకపోతే{
        sqlite3Config.bJsonSelfcheck = (u8)((*pOnOff)&0xff);
      }
#అయితే_ముగింపు
      ఆపు;
    }
  }
  va_end(ap);
#అయితే_ముగింపు /* SQLITE_UNTESTABLE */
  తిరిగి rc;
}

/*
** The Pager stores the Database filename, Journal filename, and WAL filename
** consecutively in memory, in that order.  The database filename is prefixed
** by four zero bytes.  Locate the start of the database filename by searching
** backwards for the first byte following four consecutive zero bytes.
**
** This only works if the filename passed in was obtained from the Pager.
*/
స్థిర స్థిరరాశి అక్షరం *databaseName(స్థిరరాశి అక్షరం *zName){
  అయితే( zName[-1]!=0 || zName[-2]!=0 || zName[-3]!=0 || zName[-4]!=0 ){
    zName--;
  }
  తిరిగి zName;
}

/*
** Append text z[] to the end of p[].  Return a pointer to the first
** character after then zero terminator on the new text in p[].
*/
స్థిర అక్షరం *append_పాఠ్యం(అక్షరం *p, స్థిరరాశి అక్షరం *z){
  size_t n = అక్షరశ్రేణి_పొడవు(z);
  జ్ఞాపకం_నకలు(p, z, n+1);
  తిరిగి p+n+1;
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
స్థిరరాశి అక్షరం *sqlite3_create_filename(
  స్థిరరాశి అక్షరం *zDatabase,
  స్థిరరాశి అక్షరం *zJournal,
  స్థిరరాశి అక్షరం *zWal,
  పూర్ణాంకం nParam,
  స్థిరరాశి అక్షరం **azParam
){
  sqlite3_int64 nByte;
  పూర్ణాంకం i;
  అక్షరం *p_ఫలితం, *p;
  nByte = అక్షరశ్రేణి_పొడవు(zDatabase) + అక్షరశ్రేణి_పొడవు(zJournal) + అక్షరశ్రేణి_పొడవు(zWal) + 10;
  కొరకు(i=0; i<nParam*2; i++){
    nByte += అక్షరశ్రేణి_పొడవు(azParam[i])+1;
  }
  p_ఫలితం = p = sqlite3_malloc64( nByte );
  ఒకవేళ( p==0 ) తిరిగి 0;
  memset(p, 0, 4);
  p += 4;
  p = append_పాఠ్యం(p, zDatabase);
  కొరకు(i=0; i<nParam*2; i++){
    p = append_పాఠ్యం(p, azParam[i]);
  }
  *(p++) = 0;
  p = append_పాఠ్యం(p, zJournal);
  p = append_పాఠ్యం(p, zWal);
  *(p++) = 0;
  *(p++) = 0;
  ప్రతిజ్ఞా( (sqlite3_int64)(p - p_ఫలితం)==nByte );
  తిరిగి p_ఫలితం + 4;
}

/*
** Free memory obtained from sqlite3_create_filename().  It is a severe
** error to call this routine with any parameter other than a pointer
** previously obtained from sqlite3_create_filename() or a NULL pointer.
*/
శూన్య sqlite3_free_filename(స్థిరరాశి అక్షరం *p){
  ఒకవేళ( p==0 ) తిరిగి;
  p = databaseName(p);
  sqlite3_free((అక్షరం*)p - 4);
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
స్థిరరాశి అక్షరం *sqlite3_uri_parameter(స్థిరరాశి అక్షరం *zFilename, స్థిరరాశి అక్షరం *zParam){
  ఒకవేళ( zFilename==0 || zParam==0 ) తిరిగి 0;
  zFilename = databaseName(zFilename);
  తిరిగి uriParameter(zFilename, zParam);
}

/*
** Return a pointer to the name of Nth query parameter of the filename.
*/
స్థిరరాశి అక్షరం *sqlite3_uri_కీలకం(స్థిరరాశి అక్షరం *zFilename, పూర్ణాంకం N){
  ఒకవేళ( zFilename==0 || N<0 ) తిరిగి 0;
  zFilename = databaseName(zFilename);
  zFilename += sqlite3Strlen30(zFilename) + 1;
  అయితే( ALWAYS(zFilename) && zFilename[0] && (N--)>0 ){
    zFilename += sqlite3Strlen30(zFilename) + 1;
    zFilename += sqlite3Strlen30(zFilename) + 1;
  }
  తిరిగి zFilename[0] ? zFilename : 0;
}

/*
** Return a boolean value for a query parameter.
*/
పూర్ణాంకం sqlite3_uri_boolean(స్థిరరాశి అక్షరం *zFilename, స్థిరరాశి అక్షరం *zParam, పూర్ణాంకం bDflt){
  స్థిరరాశి అక్షరం *z = sqlite3_uri_parameter(zFilename, zParam);
  bDflt = bDflt!=0;
  తిరిగి z ? sqlite3GetBoolean(z, bDflt) : bDflt;
}

/*
** Return a 64-bit integer value for a query parameter.
*/
sqlite3_int64 sqlite3_uri_int64(
  స్థిరరాశి అక్షరం *zFilename,    /* Filename as passed to xOpen */
  స్థిరరాశి అక్షరం *zParam,       /* URI parameter sought */
  sqlite3_int64 bDflt       /* return if parameter is missing */
){
  స్థిరరాశి అక్షరం *z = sqlite3_uri_parameter(zFilename, zParam);
  sqlite3_int64 v;
  ఒకవేళ( z && sqlite3_Dec_Or_Hex_గమ్యం_I64(z, &v)==0 ){
    bDflt = v;
  }
  తిరిగి bDflt;
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
స్థిరరాశి అక్షరం *sqlite3_filename_database(స్థిరరాశి అక్షరం *zFilename){
  ఒకవేళ( zFilename==0 ) తిరిగి 0;
  తిరిగి databaseName(zFilename);
}
స్థిరరాశి అక్షరం *sqlite3_filename_journal(స్థిరరాశి అక్షరం *zFilename){
  ఒకవేళ( zFilename==0 ) తిరిగి 0;
  zFilename = databaseName(zFilename);
  zFilename += sqlite3Strlen30(zFilename) + 1;
  అయితే( ALWAYS(zFilename) && zFilename[0] ){
    zFilename += sqlite3Strlen30(zFilename) + 1;
    zFilename += sqlite3Strlen30(zFilename) + 1;
  }
  తిరిగి zFilename + 1;
}
స్థిరరాశి అక్షరం *sqlite3_filename_wal(స్థిరరాశి అక్షరం *zFilename){
#నిర్వచితమైతే SQLITE_OMIT_WAL
  వాడని_PARAMETER(zFilename);
  తిరిగి 0;
#లేకపోతే
  zFilename = sqlite3_filename_journal(zFilename);
  ఒకవేళ( zFilename ) zFilename += sqlite3Strlen30(zFilename) + 1;
  తిరిగి zFilename;
#అయితే_ముగింపు
}

/*
** Return the Btree pointer identified by zDbName.  Return NULL if not found.
*/
Btree *sqlite3_Db_Name_గమ్యం_Btree(sqlite3 *db, స్థిరరాశి అక్షరం *zDbName){
  పూర్ణాంకం iDb = zDbName ? sqlite3_కనుగొను_Db_Name(db, zDbName) : 0;
  తిరిగి iDb<0 ? 0 : db->aDb[iDb].pBt;
}

/*
** Return the name of the N-th database schema.  Return NULL if N is out
** of range.
*/
స్థిరరాశి అక్షరం *sqlite3_db_name(sqlite3 *db, పూర్ణాంకం N){
  స్థిరరాశి అక్షరం *zRet = 0;
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !sqlite3SafetyCheckOk(db) ){
    (శూన్య)SQLITE_MISUSE_BKPT;
    తిరిగి 0;
  }
#అయితే_ముగింపు
  sqlite3_mutex_enter(db->mutex);
  ఒకవేళ( N>=0 && N<db->nDb ){
    zRet = db->aDb[N].zDbSName;
  }
  sqlite3_mutex_leave(db->mutex);
  తిరిగి zRet;
}

/*
** Return the filename of the database associated with a database
** connection.
*/
స్థిరరాశి అక్షరం *sqlite3_db_filename(sqlite3 *db, స్థిరరాశి అక్షరం *zDbName){
  Btree *pBt;
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !sqlite3SafetyCheckOk(db) ){
    (శూన్య)SQLITE_MISUSE_BKPT;
    తిరిగి 0;
  }
#అయితే_ముగింపు
  pBt = sqlite3_Db_Name_గమ్యం_Btree(db, zDbName);
  తిరిగి pBt ? sqlite3BtreeGetFilename(pBt) : 0;
}

/*
** Return 1 if database is read-only or 0 if read/write.  Return -1 if
** no such database exists.
*/
పూర్ణాంకం sqlite3_db_readonly(sqlite3 *db, స్థిరరాశి అక్షరం *zDbName){
  Btree *pBt;
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !sqlite3SafetyCheckOk(db) ){
    (శూన్య)SQLITE_MISUSE_BKPT;
    తిరిగి -1;
  }
#అయితే_ముగింపు
  pBt = sqlite3_Db_Name_గమ్యం_Btree(db, zDbName);
  తిరిగి pBt ? sqlite3BtreeIsReadonly(pBt) : -1;
}

#నిర్వచితమైతే SQLITE_ENABLE_SNAPSHOT
/*
** Obtain a snapshot handle for the snapshot of database zDb currently
** being read by handle db.
*/
పూర్ణాంకం sqlite3_snapshot_get(
  sqlite3 *db,
  స్థిరరాశి అక్షరం *zDb,
  sqlite3_snapshot **ppSnapshot
){
  పూర్ణాంకం rc = SQLITE_ERROR;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WAL

#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !sqlite3SafetyCheckOk(db) ){
    తిరిగి SQLITE_MISUSE_BKPT;
  }
#అయితే_ముగింపు
  sqlite3_mutex_enter(db->mutex);

  ఒకవేళ( db->autoCommit==0 ){
    పూర్ణాంకం iDb = sqlite3_కనుగొను_Db_Name(db, zDb);
    ఒకవేళ( iDb==0 || iDb>1 ){
      Btree *pBt = db->aDb[iDb].pBt;
      ఒకవేళ( SQLITE_TXN_WRITE!=sqlite3BtreeTxnState(pBt) ){
        Pager *pPager = sqlite3BtreePager(pBt);
        i64 dummy = 0;
        sqlite3PagerSnapshotOpen(pPager, (sqlite3_snapshot*)&dummy);
        rc = sqlite3BtreeBeginTrans(pBt, 0, 0);
        sqlite3PagerSnapshotOpen(pPager, 0);
        ఒకవేళ( rc==SQLITE_OK ){
          rc = sqlite3PagerSnapshotGet(sqlite3BtreePager(pBt), ppSnapshot);
        }
      }
    }
  }

  sqlite3_mutex_leave(db->mutex);
#అయితే_ముగింపు   /* SQLITE_OMIT_WAL */
  తిరిగి rc;
}

/*
** Open a read-transaction on the snapshot identified by pSnapshot.
*/
పూర్ణాంకం sqlite3_snapshot_open(
  sqlite3 *db,
  స్థిరరాశి అక్షరం *zDb,
  sqlite3_snapshot *pSnapshot
){
  పూర్ణాంకం rc = SQLITE_ERROR;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WAL

#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !sqlite3SafetyCheckOk(db) ){
    తిరిగి SQLITE_MISUSE_BKPT;
  }
#అయితే_ముగింపు
  sqlite3_mutex_enter(db->mutex);
  ఒకవేళ( db->autoCommit==0 ){
    పూర్ణాంకం iDb;
    iDb = sqlite3_కనుగొను_Db_Name(db, zDb);
    ఒకవేళ( iDb==0 || iDb>1 ){
      Btree *pBt = db->aDb[iDb].pBt;
      ఒకవేళ( sqlite3BtreeTxnState(pBt)!=SQLITE_TXN_WRITE ){
        Pager *pPager = sqlite3BtreePager(pBt);
        పూర్ణాంకం bUnlock = 0;
        ఒకవేళ( sqlite3BtreeTxnState(pBt)!=SQLITE_TXN_NONE ){
          ఒకవేళ( db->n_Vdbe_చురుకైన==0 ){
            rc = sqlite3PagerSnapshotCheck(pPager, pSnapshot);
            ఒకవేళ( rc==SQLITE_OK ){
              bUnlock = 1;
              rc = sqlite3BtreeCommit(pBt);
            }
          }
        }లేకపోతే{
          rc = SQLITE_OK;
        }
        ఒకవేళ( rc==SQLITE_OK ){
          rc = sqlite3PagerSnapshotOpen(pPager, pSnapshot);
        }
        ఒకవేళ( rc==SQLITE_OK ){
          rc = sqlite3BtreeBeginTrans(pBt, 0, 0);
          sqlite3PagerSnapshotOpen(pPager, 0);
        }
        ఒకవేళ( bUnlock ){
          sqlite3PagerSnapshotUnlock(pPager);
        }
      }
    }
  }

  sqlite3_mutex_leave(db->mutex);
#అయితే_ముగింపు   /* SQLITE_OMIT_WAL */
  తిరిగి rc;
}

/*
** Recover as many snapshots as possible from the wal file associated with
** schema zDb of database db.
*/
పూర్ణాంకం sqlite3_snapshot_recover(sqlite3 *db, స్థిరరాశి అక్షరం *zDb){
  పూర్ణాంకం rc = SQLITE_ERROR;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WAL
  పూర్ణాంకం iDb;

#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !sqlite3SafetyCheckOk(db) ){
    తిరిగి SQLITE_MISUSE_BKPT;
  }
#అయితే_ముగింపు

  sqlite3_mutex_enter(db->mutex);
  iDb = sqlite3_కనుగొను_Db_Name(db, zDb);
  ఒకవేళ( iDb==0 || iDb>1 ){
    Btree *pBt = db->aDb[iDb].pBt;
    ఒకవేళ( SQLITE_TXN_NONE==sqlite3BtreeTxnState(pBt) ){
      rc = sqlite3BtreeBeginTrans(pBt, 0, 0);
      ఒకవేళ( rc==SQLITE_OK ){
        rc = sqlite3PagerSnapshotRecover(sqlite3BtreePager(pBt));
        sqlite3BtreeCommit(pBt);
      }
    }
  }
  sqlite3_mutex_leave(db->mutex);
#అయితే_ముగింపు   /* SQLITE_OMIT_WAL */
  తిరిగి rc;
}

/*
** Free a snapshot handle obtained from sqlite3_snapshot_get().
*/
శూన్య sqlite3_snapshot_free(sqlite3_snapshot *pSnapshot){
  sqlite3_free(pSnapshot);
}
#అయితే_ముగింపు /* SQLITE_ENABLE_SNAPSHOT */

#నిర్వచితం_కాకపోతే SQLITE_OMIT_COMPILEOPTION_DIAGS
/*
** Given the name of a compile-time option, return true if that option
** was used and false if not.
**
** The name can optionally begin with "SQLITE_" but the "SQLITE_" prefix
** is not required for a match.
*/
పూర్ణాంకం sqlite3_compileoption_వాడిన(స్థిరరాశి అక్షరం *zOptName){
  పూర్ణాంకం i, n;
  పూర్ణాంకం nOpt;
  స్థిరరాశి అక్షరం **azCompileOpt;

#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( zOptName==0 ){
    (శూన్య)SQLITE_MISUSE_BKPT;
    తిరిగి 0;
  }
#అయితే_ముగింపు

  azCompileOpt = sqlite3CompileOptions(&nOpt);

  ఒకవేళ( sqlite3StrNICmp(zOptName, "SQLITE_", 7)==0 ) zOptName += 7;
  n = sqlite3Strlen30(zOptName);

  /* Since nOpt is normally in single digits, a linear search is
  ** adequate. No need for a binary search. */
  కొరకు(i=0; i<nOpt; i++){
    ఒకవేళ( sqlite3StrNICmp(zOptName, azCompileOpt[i], n)==0
     && sqlite3IsIdChar((గుర్తులేని అక్షరం)azCompileOpt[i][n])==0
    ){
      తిరిగి 1;
    }
  }
  తిరిగి 0;
}

/*
** Return the N-th compile-time option string.  If N is out of range,
** return a NULL pointer.
*/
స్థిరరాశి అక్షరం *sqlite3_compileoption_get(పూర్ణాంకం N){
  పూర్ణాంకం nOpt;
  స్థిరరాశి అక్షరం **azCompileOpt;
  azCompileOpt = sqlite3CompileOptions(&nOpt);
  ఒకవేళ( N>=0 && N<nOpt ){
    తిరిగి azCompileOpt[N];
  }
  తిరిగి 0;
}
#అయితే_ముగింపు /* SQLITE_OMIT_COMPILEOPTION_DIAGS */
