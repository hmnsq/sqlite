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
#समावेश "sqliteInt.h"

#अगर_घोषित SQLITE_ENABLE_FTS3
# include "fts3.h"
#पूर्ण_अगर
#अगर_घोषित SQLITE_ENABLE_RTREE
# include "rtree.h"
#पूर्ण_अगर
#अगर defined(SQLITE_ENABLE_ICU) || defined(SQLITE_ENABLE_ICU_COLLATIONS)
# include "sqliteicu.h"
#पूर्ण_अगर

/*
** This is an extension initializer that is a no-op and always
** succeeds, except that it fails if the fault-simulation is set
** to 500.
*/
अटल पूर्णांक sqlite3TestExtInit(sqlite3 *db){
  (व्योम)db;
  वापस sqlite3FaultSim(500);
}


/*
** Forward declarations of external module initializer functions
** for modules that need them.
*/
#अगर_घोषित SQLITE_ENABLE_FTS5
पूर्णांक sqlite3Fts5Init(sqlite3*);
#पूर्ण_अगर
#अगर_घोषित SQLITE_ENABLE_STMTVTAB
पूर्णांक sqlite3StmtVtabInit(sqlite3*);
#पूर्ण_अगर
#अगर_घोषित SQLITE_EXTRA_AUTOEXT
पूर्णांक SQLITE_EXTRA_AUTOEXT(sqlite3*);
#पूर्ण_अगर
/*
** An array of pointers to extension initializer functions for
** built-in extensions.
*/
अटल पूर्णांक (*स्थिर sqlite3BuiltinExtensions[])(sqlite3*) = {
#अगर_घोषित SQLITE_ENABLE_FTS3
  sqlite3Fts3Init,
#पूर्ण_अगर
#अगर_घोषित SQLITE_ENABLE_FTS5
  sqlite3Fts5Init,
#पूर्ण_अगर
#अगर defined(SQLITE_ENABLE_ICU) || defined(SQLITE_ENABLE_ICU_COLLATIONS)
  sqlite3IcuInit,
#पूर्ण_अगर
#अगर_घोषित SQLITE_ENABLE_RTREE
  sqlite3RtreeInit,
#पूर्ण_अगर
#अगर_घोषित SQLITE_ENABLE_DBPAGE_VTAB
  sqlite3DbpageRegister,
#पूर्ण_अगर
#अगर_घोषित SQLITE_ENABLE_DBSTAT_VTAB
  sqlite3DbstatRegister,
#पूर्ण_अगर
  sqlite3TestExtInit,
#अगर_घोषित SQLITE_ENABLE_STMTVTAB
  sqlite3StmtVtabInit,
#पूर्ण_अगर
#अगर_घोषित SQLITE_ENABLE_BYTECODE_VTAB
  sqlite3VdbeBytecodeVtabInit,
#पूर्ण_अगर
#अगर_घोषित SQLITE_EXTRA_AUTOEXT
  SQLITE_EXTRA_AUTOEXT,
#पूर्ण_अगर
};

#अगर_अघोषित SQLITE_AMALGAMATION
/* IMPLEMENTATION-OF: R-46656-45156 The sqlite3_version[] string constant
** contains the text of SQLITE_VERSION macro.
*/
स्थिर अक्षर sqlite3_version[] = SQLITE_VERSION;
#पूर्ण_अगर

/* IMPLEMENTATION-OF: R-53536-42575 The sqlite3_libversion() function returns
** a pointer to the to the sqlite3_version[] string constant.
*/
स्थिर अक्षर *sqlite3_libversion(व्योम){ वापस sqlite3_version; }

/* IMPLEMENTATION-OF: R-25063-23286 The sqlite3_sourceid() function returns a
** pointer to a string constant whose value is the same as the
** SQLITE_SOURCE_ID C preprocessor macro. Except if SQLite is built using
** an edited copy of the amalgamation, then the last four characters of
** the hash might be different from SQLITE_SOURCE_ID.
*/
स्थिर अक्षर *sqlite3_sourceid(व्योम){ वापस SQLITE_SOURCE_ID; }

/* IMPLEMENTATION-OF: R-35210-63508 The sqlite3_libversion_number() function
** returns an integer equal to SQLITE_VERSION_NUMBER.
*/
पूर्णांक sqlite3_libversion_संख्या(व्योम){ वापस SQLITE_VERSION_संख्या; }

/* IMPLEMENTATION-OF: R-20790-14025 The sqlite3_threadsafe() function returns
** zero if and only if SQLite was compiled with mutexing code omitted due to
** the SQLITE_THREADSAFE compile-time option being set to 0.
*/
पूर्णांक sqlite3_threadsafe(व्योम){ वापस SQLITE_THREADSAFE; }

/*
** When compiling the test fixture or with debugging enabled (on Win32),
** this variable being set to non-zero will cause OSTRACE macros to emit
** extra diagnostic information.
*/
#अगर_घोषित SQLITE_HAVE_OS_TRACE
# ifndef SQLITE_DEBUG_OS_TRACE
#   define SQLITE_DEBUG_OS_TRACE 0
# endif
  पूर्णांक sqlite3OSTrace = SQLITE_DEBUG_OS_TRACE;
#पूर्ण_अगर

#अगर !defined(SQLITE_OMIT_TRACE) && defined(SQLITE_ENABLE_IOTRACE)
/*
** If the following function pointer is not NULL and if
** SQLITE_ENABLE_IOTRACE is enabled, then messages describing
** I/O active are written using this function.  These messages
** are intended for debugging activity only.
*/
SQLITE_API व्योम (SQLITE_CDECL *sqlite3IoTrace)(स्थिर अक्षर*, ...) = 0;
#पूर्ण_अगर

/*
** If the following global variable points to a string which is the
** name of a directory, then that directory will be used to store
** temporary files.
**
** See also the "PRAGMA temp_store_directory" SQL command.
*/
अक्षर *sqlite3_अस्थायी_directory = 0;

/*
** If the following global variable points to a string which is the
** name of a directory, then that directory will be used to store
** all database files specified with a relative pathname.
**
** See also the "PRAGMA data_store_directory" SQL command.
*/
अक्षर *sqlite3_तथ्य_directory = 0;

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
अटल SQLITE_NOINLINE पूर्णांक sqlite3Initialize(व्योम){
  MUTEX_LOGIC( sqlite3_mutex *pMainMtx; )   /* The main static mutex */
  पूर्णांक rc;                                   /* Result code */
#अगर_घोषित SQLITE_EXTRA_INIT
  पूर्णांक bRunExtraInit = 0;                    /* Extra initialization needed */
#पूर्ण_अगर

  /* If the following assert() fails on some obscure processor/compiler
  ** combination to warn that SQLite has been mis-compiled.  If you hit
  ** this assert(), that means you need to recompile with the
  ** -DSQLITE_PTRSIZE=n compile-time option to set the correct pointer size.
  */
  निश्चित( SQLITE_PTRSIZE==माप(अक्षर*) );

  /* Make sure the mutex subsystem is initialized.  If unable to
  ** initialize the mutex subsystem, return early with the error.
  ** If the system is so sick that we are unable to allocate a mutex,
  ** there is not much SQLite is going to be able to do.
  **
  ** The mutex subsystem must take care of serializing its own
  ** initialization.
  */
  rc = sqlite3MutexInit();
  अगर( rc ) वापस rc;

  /* Initialize the malloc() system and the recursive pInitMutex mutex.
  ** This operation is protected by the STATIC_MAIN mutex.  Note that
  ** MutexAlloc() is called for a static mutex prior to initializing the
  ** malloc subsystem - this implies that the allocation of a static
  ** mutex must not require support from the malloc subsystem.
  */
  MUTEX_LOGIC( pMainMtx = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  sqlite3_mutex_enter(pMainMtx);
  sqlite3GlobalConfig.isMutexInit = 1;
  अगर( !sqlite3GlobalConfig.isMallocInit ){
    rc = sqlite3MallocInit();
  }
  अगर( rc==SQLITE_OK ){
    sqlite3GlobalConfig.isMallocInit = 1;
    अगर( !sqlite3GlobalConfig.pInitMutex ){
      sqlite3GlobalConfig.pInitMutex =
           sqlite3MutexAlloc(SQLITE_MUTEX_RECURSIVE);
      अगर( sqlite3GlobalConfig.bCoreMutex && !sqlite3GlobalConfig.pInitMutex ){
        rc = SQLITE_NOMEM_BKPT;
      }
    }
  }
  अगर( rc==SQLITE_OK ){
    sqlite3GlobalConfig.nRefInitMutex++;
  }
  sqlite3_mutex_leave(pMainMtx);

  /* If rc is not SQLITE_OK at this point, then either the malloc
  ** subsystem could not be initialized or the system failed to allocate
  ** the pInitMutex mutex. Return an error in either case.  */
  अगर( rc!=SQLITE_OK ){
    वापस rc;
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
  अगर( sqlite3GlobalConfig.isInit==0 && sqlite3GlobalConfig.inProgress==0 ){
    sqlite3GlobalConfig.inProgress = 1;
#अगर_घोषित SQLITE_ENABLE_SQLLOG
    {
      बाह्य व्योम sqlite3_init_sqllog(व्योम);
      sqlite3_init_sqllog();
    }
#पूर्ण_अगर
    स_रखो(&sqlite3BuiltinFunctions, 0, माप(sqlite3BuiltinFunctions));
    sqlite3RegisterBuiltinFunctions();
    अगर( sqlite3GlobalConfig.isPCacheInit==0 ){
      rc = sqlite3PcacheInitialize();
    }
    अगर( rc==SQLITE_OK ){
      sqlite3GlobalConfig.isPCacheInit = 1;
      rc = sqlite3OsInit();
    }
#अगर_अघोषित SQLITE_OMIT_DESERIALIZE
    अगर( rc==SQLITE_OK ){
      rc = sqlite3MemdbInit();
    }
#पूर्ण_अगर
    अगर( rc==SQLITE_OK ){
      sqlite3_PCache_बफर_Setup( sqlite3GlobalConfig.pPage,
          sqlite3GlobalConfig.szPage, sqlite3GlobalConfig.nPage);
#अगर_घोषित SQLITE_EXTRA_INIT_MUTEXED
      {
        पूर्णांक SQLITE_EXTRA_INIT_MUTEXED(स्थिर अक्षर*);
        rc = SQLITE_EXTRA_INIT_MUTEXED(0);
      }
#पूर्ण_अगर
    }
    अगर( rc==SQLITE_OK ){
      sqlite3MemoryBarrier();
      sqlite3GlobalConfig.isInit = 1;
#अगर_घोषित SQLITE_EXTRA_INIT
      bRunExtraInit = 1;
#पूर्ण_अगर
    }
    sqlite3GlobalConfig.inProgress = 0;
  }
  sqlite3_mutex_leave(sqlite3GlobalConfig.pInitMutex);

  /* Go back under the static mutex and clean up the recursive
  ** mutex to prevent a resource leak.
  */
  sqlite3_mutex_enter(pMainMtx);
  sqlite3GlobalConfig.nRefInitMutex--;
  अगर( sqlite3GlobalConfig.nRefInitMutex<=0 ){
    निश्चित( sqlite3GlobalConfig.nRefInitMutex==0 );
    sqlite3_mutex_free(sqlite3GlobalConfig.pInitMutex);
    sqlite3GlobalConfig.pInitMutex = 0;
  }
  sqlite3_mutex_leave(pMainMtx);

  /* The following is just a sanity check to make sure SQLite has
  ** been compiled correctly.  It is important to run this code, but
  ** we don't want to run it too often and soak up CPU cycles for no
  ** reason.  So we run it once during initialization.
  */
#अगर_अघोषित न_संशोधन
#अगर_अघोषित SQLITE_OMIT_FLOATING_POINT
  /* This section of code's only "output" is via assert() statements. */
  अगर( rc==SQLITE_OK ){
    u64 x = (((u64)1)<<63)-1;
    द्विगुन y;
    निश्चित(माप(x)==8);
    निश्चित(माप(x)==माप(y));
    स_नकल(&y, &x, 8);
    निश्चित( sqlite3IsNaN(y) );
  }
#पूर्ण_अगर
#पूर्ण_अगर

  /* Do extra initialization steps requested by the SQLITE_EXTRA_INIT
  ** compile-time option.
  */
#अगर_घोषित SQLITE_EXTRA_INIT
  अगर( bRunExtraInit ){
    पूर्णांक SQLITE_EXTRA_INIT(स्थिर अक्षर*);
    rc = SQLITE_EXTRA_INIT(0);
  }
#पूर्ण_अगर
  वापस rc;
}
SQLITE_API पूर्णांक sqlite3_initialize(व्योम){
  /* If this build does not support writable static data (WSD) natively
  ** then we have to invoke the (application-supplied) WSD initialization
  ** routine before doing anything else. */
#अगर_घोषित SQLITE_OMIT_WSD
  पूर्णांक rc = sqlite3_wsd_init(4096, 24);
  अगर( rc!=SQLITE_OK ){
    वापस rc;
  }
#पूर्ण_अगर

  अगर( sqlite3GlobalConfig.isInit ){
    /* SQLite has already been initialized.  Fast early-out. */
    sqlite3MemoryBarrier();
    वापस SQLITE_OK;
  }अन्यथा{
    /* Invoke sqlite3Initialize() to do the actual work. */
    वापस sqlite3Initialize();
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
पूर्णांक sqlite3_shutdown(व्योम){
#अगर_घोषित SQLITE_OMIT_WSD
  पूर्णांक rc = sqlite3_wsd_init(4096, 24);
  अगर( rc!=SQLITE_OK ){
    वापस rc;
  }
#पूर्ण_अगर

  अगर( sqlite3GlobalConfig.isInit ){
#अगर_घोषित SQLITE_EXTRA_SHUTDOWN
    व्योम SQLITE_EXTRA_SHUTDOWN(व्योम);
    SQLITE_EXTRA_SHUTDOWN();
#पूर्ण_अगर
    sqlite3_os_end();
    sqlite3_reset_auto_extension();
    sqlite3GlobalConfig.isInit = 0;
  }
  अगर( sqlite3GlobalConfig.isPCacheInit ){
    sqlite3PcacheShutdown();
    sqlite3GlobalConfig.isPCacheInit = 0;
  }
  अगर( sqlite3GlobalConfig.isMallocInit ){
    sqlite3MallocEnd();
    sqlite3GlobalConfig.isMallocInit = 0;

#अगर_अघोषित SQLITE_OMIT_SHUTDOWN_DIRECTORIES
    /* The heap subsystem has now been shutdown and these values are supposed
    ** to be NULL or point to memory that was obtained from sqlite3_malloc(),
    ** which would rely on that heap subsystem; therefore, make sure these
    ** values cannot refer to heap memory that was just invalidated when the
    ** heap subsystem was shutdown.  This is only done if the current call to
    ** this function resulted in the heap subsystem actually being shutdown.
    */
    sqlite3_तथ्य_directory = 0;
    sqlite3_अस्थायी_directory = 0;
#पूर्ण_अगर
  }
  अगर( sqlite3GlobalConfig.isMutexInit ){
    sqlite3MutexEnd();
    sqlite3GlobalConfig.isMutexInit = 0;
  }

  वापस SQLITE_OK;
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
पूर्णांक sqlite3_config(पूर्णांक op, ...){
  बहु_सूची ap;
  पूर्णांक rc = SQLITE_OK;

  /* sqlite3_config() normally returns SQLITE_MISUSE if it is invoked while
  ** the SQLite library is in use.  Except, a few selected opcodes
  ** are allowed.
  */
  अगर( sqlite3GlobalConfig.isInit ){
    अटल स्थिर u64 mAnytimeConfigOption = 0
       | MASKBIT64( SQLITE_CONFIG_LOG )
       | MASKBIT64( SQLITE_CONFIG_PCACHE_HDRSZ )
    ;
    अगर( op<0 || op>63 || (MASKBIT64(op) & mAnytimeConfigOption)==0 ){
      वापस SQLITE_MISUSE_BKPT;
    }
    testcase( op==SQLITE_CONFIG_LOG );
    testcase( op==SQLITE_CONFIG_PCACHE_HDRSZ );
  }

  बहु_शुरू(ap, op);
  चयन( op ){

    /* Mutex configuration options are only available in a threadsafe
    ** compile.
    */
#अगर defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE>0  /* IMP: R-54466-46756 */
    हाल SQLITE_CONFIG_SINGLETHREAD: {
      /* EVIDENCE-OF: R-02748-19096 This option sets the threading mode to
      ** Single-thread. */
      sqlite3GlobalConfig.bCoreMutex = 0;  /* Disable mutex on core */
      sqlite3GlobalConfig.bFullMutex = 0;  /* Disable mutex on connections */
      अवरोध;
    }
#पूर्ण_अगर
#अगर defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE>0 /* IMP: R-20520-54086 */
    हाल SQLITE_CONFIG_MULTITHREAD: {
      /* EVIDENCE-OF: R-14374-42468 This option sets the threading mode to
      ** Multi-thread. */
      sqlite3GlobalConfig.bCoreMutex = 1;  /* Enable mutex on core */
      sqlite3GlobalConfig.bFullMutex = 0;  /* Disable mutex on connections */
      अवरोध;
    }
#पूर्ण_अगर
#अगर defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE>0 /* IMP: R-59593-21810 */
    हाल SQLITE_CONFIG_SERIALIZED: {
      /* EVIDENCE-OF: R-41220-51800 This option sets the threading mode to
      ** Serialized. */
      sqlite3GlobalConfig.bCoreMutex = 1;  /* Enable mutex on core */
      sqlite3GlobalConfig.bFullMutex = 1;  /* Enable mutex on connections */
      अवरोध;
    }
#पूर्ण_अगर
#अगर defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE>0 /* IMP: R-63666-48755 */
    हाल SQLITE_CONFIG_MUTEX: {
      /* Specify an alternative mutex implementation */
      sqlite3GlobalConfig.mutex = *बहु_तर्क(ap, sqlite3_mutex_methods*);
      अवरोध;
    }
#पूर्ण_अगर
#अगर defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE>0 /* IMP: R-14450-37597 */
    हाल SQLITE_CONFIG_GETMUTEX: {
      /* Retrieve the current mutex implementation */
      *बहु_तर्क(ap, sqlite3_mutex_methods*) = sqlite3GlobalConfig.mutex;
      अवरोध;
    }
#पूर्ण_अगर

    हाल SQLITE_CONFIG_MALLOC: {
      /* EVIDENCE-OF: R-55594-21030 The SQLITE_CONFIG_MALLOC option takes a
      ** single argument which is a pointer to an instance of the
      ** sqlite3_mem_methods structure. The argument specifies alternative
      ** low-level memory allocation routines to be used in place of the memory
      ** allocation routines built into SQLite. */
      sqlite3GlobalConfig.m = *बहु_तर्क(ap, sqlite3_mem_methods*);
      अवरोध;
    }
    हाल SQLITE_CONFIG_GETMALLOC: {
      /* EVIDENCE-OF: R-51213-46414 The SQLITE_CONFIG_GETMALLOC option takes a
      ** single argument which is a pointer to an instance of the
      ** sqlite3_mem_methods structure. The sqlite3_mem_methods structure is
      ** filled with the currently defined memory allocation routines. */
      अगर( sqlite3GlobalConfig.m.xMalloc==0 ) sqlite3MemSetDefault();
      *बहु_तर्क(ap, sqlite3_mem_methods*) = sqlite3GlobalConfig.m;
      अवरोध;
    }
    हाल SQLITE_CONFIG_MEMSTATUS: {
      निश्चित( !sqlite3GlobalConfig.isInit );  /* Cannot change at runtime */
      /* EVIDENCE-OF: R-61275-35157 The SQLITE_CONFIG_MEMSTATUS option takes
      ** single argument of type int, interpreted as a boolean, which enables
      ** or disables the collection of memory allocation statistics. */
      sqlite3GlobalConfig.bMemstat = बहु_तर्क(ap, पूर्णांक);
      अवरोध;
    }
    हाल SQLITE_CONFIG_SMALL_MALLOC: {
      sqlite3GlobalConfig.bSmallMalloc = बहु_तर्क(ap, पूर्णांक)!=0;
      अवरोध;
    }
    हाल SQLITE_CONFIG_PAGECACHE: {
      /* EVIDENCE-OF: R-18761-36601 There are three arguments to
      ** SQLITE_CONFIG_PAGECACHE: A pointer to 8-byte aligned memory (pMem),
      ** the size of each page cache line (sz), and the number of cache lines
      ** (N). */
      sqlite3GlobalConfig.pPage = बहु_तर्क(ap, व्योम*);
      sqlite3GlobalConfig.szPage = बहु_तर्क(ap, पूर्णांक);
      sqlite3GlobalConfig.nPage = बहु_तर्क(ap, पूर्णांक);
      अवरोध;
    }
    हाल SQLITE_CONFIG_PCACHE_HDRSZ: {
      /* EVIDENCE-OF: R-39100-27317 The SQLITE_CONFIG_PCACHE_HDRSZ option takes
      ** a single parameter which is a pointer to an integer and writes into
      ** that integer the number of extra bytes per page required for each page
      ** in SQLITE_CONFIG_PAGECACHE. */
      *बहु_तर्क(ap, पूर्णांक*) =
          sqlite3_Header_आकार_Btree() +
          sqlite3_Header_आकार_Pcache() +
          sqlite3_Header_आकार_Pcache1();
      अवरोध;
    }

    हाल SQLITE_CONFIG_PCACHE: {
      /* no-op */
      अवरोध;
    }
    हाल SQLITE_CONFIG_GETPCACHE: {
      /* now an error */
      rc = SQLITE_ERROR;
      अवरोध;
    }

    हाल SQLITE_CONFIG_PCACHE2: {
      /* EVIDENCE-OF: R-63325-48378 The SQLITE_CONFIG_PCACHE2 option takes a
      ** single argument which is a pointer to an sqlite3_pcache_methods2
      ** object. This object specifies the interface to a custom page cache
      ** implementation. */
      sqlite3GlobalConfig.pcache2 = *बहु_तर्क(ap, sqlite3_pcache_methods2*);
      अवरोध;
    }
    हाल SQLITE_CONFIG_GETPCACHE2: {
      /* EVIDENCE-OF: R-22035-46182 The SQLITE_CONFIG_GETPCACHE2 option takes a
      ** single argument which is a pointer to an sqlite3_pcache_methods2
      ** object. SQLite copies of the current page cache implementation into
      ** that object. */
      अगर( sqlite3GlobalConfig.pcache2.xInit==0 ){
        sqlite3PCacheSetDefault();
      }
      *बहु_तर्क(ap, sqlite3_pcache_methods2*) = sqlite3GlobalConfig.pcache2;
      अवरोध;
    }

/* EVIDENCE-OF: R-06626-12911 The SQLITE_CONFIG_HEAP option is only
** available if SQLite is compiled with either SQLITE_ENABLE_MEMSYS3 or
** SQLITE_ENABLE_MEMSYS5 and returns SQLITE_ERROR if invoked otherwise. */
#अगर defined(SQLITE_ENABLE_MEMSYS3) || defined(SQLITE_ENABLE_MEMSYS5)
    हाल SQLITE_CONFIG_HEAP: {
      /* EVIDENCE-OF: R-19854-42126 There are three arguments to
      ** SQLITE_CONFIG_HEAP: An 8-byte aligned pointer to the memory, the
      ** number of bytes in the memory buffer, and the minimum allocation size.
      */
      sqlite3GlobalConfig.pHeap = बहु_तर्क(ap, व्योम*);
      sqlite3GlobalConfig.nHeap = बहु_तर्क(ap, पूर्णांक);
      sqlite3GlobalConfig.mnReq = बहु_तर्क(ap, पूर्णांक);

      अगर( sqlite3GlobalConfig.mnReq<1 ){
        sqlite3GlobalConfig.mnReq = 1;
      }अन्यथा अगर( sqlite3GlobalConfig.mnReq>(1<<12) ){
        /* cap min request size at 2^12 */
        sqlite3GlobalConfig.mnReq = (1<<12);
      }

      अगर( sqlite3GlobalConfig.pHeap==0 ){
        /* EVIDENCE-OF: R-49920-60189 If the first pointer (the memory pointer)
        ** is NULL, then SQLite reverts to using its default memory allocator
        ** (the system malloc() implementation), undoing any prior invocation of
        ** SQLITE_CONFIG_MALLOC.
        **
        ** Setting sqlite3GlobalConfig.m to all zeros will cause malloc to
        ** revert to its default implementation when sqlite3_initialize() is run
        */
        स_रखो(&sqlite3GlobalConfig.m, 0, माप(sqlite3GlobalConfig.m));
      }अन्यथा{
        /* EVIDENCE-OF: R-61006-08918 If the memory pointer is not NULL then the
        ** alternative memory allocator is engaged to handle all of SQLites
        ** memory allocation needs. */
#अगर_घोषित SQLITE_ENABLE_MEMSYS3
        sqlite3GlobalConfig.m = *sqlite3MemGetMemsys3();
#पूर्ण_अगर
#अगर_घोषित SQLITE_ENABLE_MEMSYS5
        sqlite3GlobalConfig.m = *sqlite3MemGetMemsys5();
#पूर्ण_अगर
      }
      अवरोध;
    }
#पूर्ण_अगर

    हाल SQLITE_CONFIG_LOOKASIDE: {
      sqlite3GlobalConfig.szLookaside = बहु_तर्क(ap, पूर्णांक);
      sqlite3GlobalConfig.nLookaside = बहु_तर्क(ap, पूर्णांक);
      अवरोध;
    }
   
    /* Record a pointer to the logger function and its first argument.
    ** The default is NULL.  Logging is disabled if the function pointer is
    ** NULL.
    */
    हाल SQLITE_CONFIG_LOG: {
      /* MSVC is picky about pulling func ptrs from va lists.
      ** http://support.microsoft.com/kb/47961
      ** sqlite3GlobalConfig.xLog = va_arg(ap, void(*)(void*,int,const char*));
      */
      प्रकार व्योम(*LOGFUNC_t)(व्योम*,पूर्णांक,स्थिर अक्षर*);
      LOGFUNC_t xLog = बहु_तर्क(ap, LOGFUNC_t);
      व्योम *pLogArg = बहु_तर्क(ap, व्योम*);
      AtomicStore(&sqlite3GlobalConfig.xLog, xLog);
      AtomicStore(&sqlite3GlobalConfig.pLogArg, pLogArg);
      अवरोध;
    }

    /* EVIDENCE-OF: R-55548-33817 The compile-time setting for URI filenames
    ** can be changed at start-time using the
    ** sqlite3_config(SQLITE_CONFIG_URI,1) or
    ** sqlite3_config(SQLITE_CONFIG_URI,0) configuration calls.
    */
    हाल SQLITE_CONFIG_URI: {
      /* EVIDENCE-OF: R-25451-61125 The SQLITE_CONFIG_URI option takes a single
      ** argument of type int. If non-zero, then URI handling is globally
      ** enabled. If the parameter is zero, then URI handling is globally
      ** disabled. */
      पूर्णांक bOpenUri = बहु_तर्क(ap, पूर्णांक);
      AtomicStore(&sqlite3GlobalConfig.bOpenUri, bOpenUri);
      अवरोध;
    }

    हाल SQLITE_CONFIG_COVERING_सूचकांक_SCAN: {
      /* EVIDENCE-OF: R-36592-02772 The SQLITE_CONFIG_COVERING_INDEX_SCAN
      ** option takes a single integer argument which is interpreted as a
      ** boolean in order to enable or disable the use of covering indices for
      ** full table scans in the query optimizer. */
      sqlite3GlobalConfig.bUseCis = बहु_तर्क(ap, पूर्णांक);
      अवरोध;
    }

#अगर_घोषित SQLITE_ENABLE_SQLLOG
    हाल SQLITE_CONFIG_SQLLOG: {
      प्रकार व्योम(*SQLLOGFUNC_t)(व्योम*, sqlite3*, स्थिर अक्षर*, पूर्णांक);
      sqlite3GlobalConfig.xSqllog = बहु_तर्क(ap, SQLLOGFUNC_t);
      sqlite3GlobalConfig.pSqllogArg = बहु_तर्क(ap, व्योम *);
      अवरोध;
    }
#पूर्ण_अगर

    हाल SQLITE_CONFIG_MMAP_आकार: {
      /* EVIDENCE-OF: R-58063-38258 SQLITE_CONFIG_MMAP_SIZE takes two 64-bit
      ** integer (sqlite3_int64) values that are the default mmap size limit
      ** (the default setting for PRAGMA mmap_size) and the maximum allowed
      ** mmap size limit. */
      sqlite3_int64 szMmap = बहु_तर्क(ap, sqlite3_int64);
      sqlite3_int64 mxMmap = बहु_तर्क(ap, sqlite3_int64);
      /* EVIDENCE-OF: R-53367-43190 If either argument to this option is
      ** negative, then that argument is changed to its compile-time default.
      **
      ** EVIDENCE-OF: R-34993-45031 The maximum allowed mmap size will be
      ** silently truncated if necessary so that it does not exceed the
      ** compile-time maximum mmap size set by the SQLITE_MAX_MMAP_SIZE
      ** compile-time option.
      */
      अगर( mxMmap<0 || mxMmap>SQLITE_MAX_MMAP_आकार ){
        mxMmap = SQLITE_MAX_MMAP_आकार;
      }
      अगर( szMmap<0 ) szMmap = SQLITE_DEFAULT_MMAP_आकार;
      अगर( szMmap>mxMmap) szMmap = mxMmap;
      sqlite3GlobalConfig.mxMmap = mxMmap;
      sqlite3GlobalConfig.szMmap = szMmap;
      अवरोध;
    }

#अगर SQLITE_OS_WIN && defined(SQLITE_WIN32_MALLOC) /* IMP: R-04780-55815 */
    हाल SQLITE_CONFIG_WIN32_HEAPSIZE: {
      /* EVIDENCE-OF: R-34926-03360 SQLITE_CONFIG_WIN32_HEAPSIZE takes a 32-bit
      ** unsigned integer value that specifies the maximum size of the created
      ** heap. */
      sqlite3GlobalConfig.nHeap = बहु_तर्क(ap, पूर्णांक);
      अवरोध;
    }
#पूर्ण_अगर

    हाल SQLITE_CONFIG_PMASZ: {
      sqlite3GlobalConfig.szPma = बहु_तर्क(ap, अचिन्हित पूर्णांक);
      अवरोध;
    }

    हाल SQLITE_CONFIG_STMTJRNL_SPILL: {
      sqlite3GlobalConfig.nStmtSpill = बहु_तर्क(ap, पूर्णांक);
      अवरोध;
    }

#अगर_घोषित SQLITE_ENABLE_SORTER_REFERENCES
    हाल SQLITE_CONFIG_SORTERREF_आकार: {
      पूर्णांक iVal = बहु_तर्क(ap, पूर्णांक);
      अगर( iVal<0 ){
        iVal = SQLITE_DEFAULT_SORTERREF_आकार;
      }
      sqlite3GlobalConfig.szSorterRef = (u32)iVal;
      अवरोध;
    }
#पूर्ण_अगर /* SQLITE_ENABLE_SORTER_REFERENCES */

#अगर_अघोषित SQLITE_OMIT_DESERIALIZE
    हाल SQLITE_CONFIG_MEMDB_MAXSIZE: {
      sqlite3GlobalConfig.mx_Memdb_आकार = बहु_तर्क(ap, sqlite3_int64);
      अवरोध;
    }
#पूर्ण_अगर /* SQLITE_OMIT_DESERIALIZE */

    हाल SQLITE_CONFIG_ROWID_IN_VIEW: {
      पूर्णांक *pVal = बहु_तर्क(ap,पूर्णांक*);
#अगर_घोषित SQLITE_ALLOW_ROWID_IN_VIEW
      अगर( 0==*pVal ) sqlite3GlobalConfig.mNoVisibleRowid = TF_NoVisibleRowid;
      अगर( 1==*pVal ) sqlite3GlobalConfig.mNoVisibleRowid = 0;
      *pVal = (sqlite3GlobalConfig.mNoVisibleRowid==0);
#अन्यथा
      *pVal = 0;
#पूर्ण_अगर
      अवरोध;
    }

    शेष: {
      rc = SQLITE_ERROR;
      अवरोध;
    }
  }
  बहु_पूर्ण(ap);
  वापस rc;
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
अटल पूर्णांक setupLookaside(
  sqlite3 *db,    /* Database connection being configured */
  व्योम *pBuf,     /* Memory to use for lookaside.  May be NULL */
  पूर्णांक sz,         /* Desired size of each lookaside memory slot */
  पूर्णांक cnt         /* Number of slots to allocate */
){
#अगर_अघोषित SQLITE_OMIT_LOOKASIDE
  व्योम *pStart;          /* Start of the lookaside buffer */
  sqlite3_int64 szAlloc; /* Total space set aside for lookaside memory */
  पूर्णांक nBig;              /* Number of full-size slots */
  पूर्णांक nSm;               /* Number smaller LOOKASIDE_SMALL-byte slots */
 
  अगर( sqlite3LookasideUsed(db,0)>0 ){
    वापस SQLITE_BUSY;
  }
  /* Free any existing lookaside buffer for this handle before
  ** allocating a new one so we don't have to have space for
  ** both at the same time.
  */
  अगर( db->lookaside.bMalloced ){
    sqlite3_free(db->lookaside.pStart);
  }
  /* The size of a lookaside slot after ROUNDDOWN8 needs to be larger
  ** than a pointer and small enough to fit in a u16.
  */
  sz = ROUNDDOWN8(sz);
  अगर( sz<=(पूर्णांक)माप(LookasideSlot*) ) sz = 0;
  अगर( sz>65528 ) sz = 65528;
  /* Count must be at least 1 to be useful, but not so large as to use
  ** more than 0x7fff0000 total bytes for lookaside. */
  अगर( cnt<1 ) cnt = 0;
  अगर( sz>0 && cnt>(0x7fff0000/sz) ) cnt = 0x7fff0000/sz;
  szAlloc = (i64)sz*(i64)cnt;
  अगर( szAlloc==0 ){
    sz = 0;
    pStart = 0;
  }अन्यथा अगर( pBuf==0 ){
    sqlite3BeginBenignMalloc();
    pStart = sqlite3Malloc( szAlloc );
    sqlite3EndBenignMalloc();
    अगर( pStart ) szAlloc = sqlite3_Malloc_आकार(pStart);
  }अन्यथा{
    pStart = pBuf;
  }
#अगर_अघोषित SQLITE_OMIT_TWOSIZE_LOOKASIDE
  अगर( sz>=LOOKASIDE_SMALL*3 ){
    nBig = szAlloc/(3*LOOKASIDE_SMALL+sz);
    nSm = (szAlloc - (i64)sz*(i64)nBig)/LOOKASIDE_SMALL;
  }अन्यथा अगर( sz>=LOOKASIDE_SMALL*2 ){
    nBig = szAlloc/(LOOKASIDE_SMALL+sz);
    nSm = (szAlloc - (i64)sz*(i64)nBig)/LOOKASIDE_SMALL;
  }अन्यथा
#पूर्ण_अगर /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
  अगर( sz>0 ){
    nBig = szAlloc/sz;
    nSm = 0;
  }अन्यथा{
    nBig = nSm = 0;
  }
  db->lookaside.pStart = pStart;
  db->lookaside.pInit = 0;
  db->lookaside.pFree = 0;
  db->lookaside.sz = (u16)sz;
  db->lookaside.szTrue = (u16)sz;
  अगर( pStart ){
    पूर्णांक i;
    LookasideSlot *p;
    निश्चित( sz > (पूर्णांक)माप(LookasideSlot*) );
    p = (LookasideSlot*)pStart;
    क्रम(i=0; i<nBig; i++){
      p->p_अग्र = db->lookaside.pInit;
      db->lookaside.pInit = p;
      p = (LookasideSlot*)&((u8*)p)[sz];
    }
#अगर_अघोषित SQLITE_OMIT_TWOSIZE_LOOKASIDE
    db->lookaside.pSmallInit = 0;
    db->lookaside.pSmallFree = 0;
    db->lookaside.p_मध्य = p;
    क्रम(i=0; i<nSm; i++){
      p->p_अग्र = db->lookaside.pSmallInit;
      db->lookaside.pSmallInit = p;
      p = (LookasideSlot*)&((u8*)p)[LOOKASIDE_SMALL];
    }
#पूर्ण_अगर /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
    निश्चित( ((uptr)p)<=szAlloc + (uptr)pStart );
    db->lookaside.pEnd = p;
    db->lookaside.bDisable = 0;
    db->lookaside.bMalloced = pBuf==0 ?1:0;
    db->lookaside.nSlot = nBig+nSm;
  }अन्यथा{
    db->lookaside.pStart = 0;
#अगर_अघोषित SQLITE_OMIT_TWOSIZE_LOOKASIDE
    db->lookaside.pSmallInit = 0;
    db->lookaside.pSmallFree = 0;
    db->lookaside.p_मध्य = 0;
#पूर्ण_अगर /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
    db->lookaside.pEnd = 0;
    db->lookaside.bDisable = 1;
    db->lookaside.sz = 0;
    db->lookaside.bMalloced = 0;
    db->lookaside.nSlot = 0;
  }
  db->lookaside.pTrueEnd = db->lookaside.pEnd;
  निश्चित( sqlite3LookasideUsed(db,0)==0 );
#पूर्ण_अगर /* SQLITE_OMIT_LOOKASIDE */
  वापस SQLITE_OK;
}

/*
** Return the mutex associated with a database connection.
*/
sqlite3_mutex *sqlite3_db_mutex(sqlite3 *db){
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !sqlite3SafetyCheckOk(db) ){
    (व्योम)SQLITE_MISUSE_BKPT;
    वापस 0;
  }
#पूर्ण_अगर
  वापस db->mutex;
}

/*
** Free up as much memory as we can from the given database
** connection.
*/
पूर्णांक sqlite3_db_release_memory(sqlite3 *db){
  पूर्णांक i;

#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !sqlite3SafetyCheckOk(db) ) वापस SQLITE_MISUSE_BKPT;
#पूर्ण_अगर
  sqlite3_mutex_enter(db->mutex);
  sqlite3BtreeEnterAll(db);
  क्रम(i=0; i<db->nDb; i++){
    Btree *pBt = db->aDb[i].pBt;
    अगर( pBt ){
      Pager *pPager = sqlite3BtreePager(pBt);
      sqlite3PagerShrink(pPager);
    }
  }
  sqlite3BtreeLeaveAll(db);
  sqlite3_mutex_leave(db->mutex);
  वापस SQLITE_OK;
}

/*
** Flush any dirty pages in the pager-cache for any attached database
** to disk.
*/
पूर्णांक sqlite3_db_cacheflush(sqlite3 *db){
  पूर्णांक i;
  पूर्णांक rc = SQLITE_OK;
  पूर्णांक bSeenBusy = 0;

#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !sqlite3SafetyCheckOk(db) ) वापस SQLITE_MISUSE_BKPT;
#पूर्ण_अगर
  sqlite3_mutex_enter(db->mutex);
  sqlite3BtreeEnterAll(db);
  क्रम(i=0; rc==SQLITE_OK && i<db->nDb; i++){
    Btree *pBt = db->aDb[i].pBt;
    अगर( pBt && sqlite3BtreeTxnState(pBt)==SQLITE_TXN_WRITE ){
      Pager *pPager = sqlite3BtreePager(pBt);
      rc = sqlite3PagerFlush(pPager);
      अगर( rc==SQLITE_BUSY ){
        bSeenBusy = 1;
        rc = SQLITE_OK;
      }
    }
  }
  sqlite3BtreeLeaveAll(db);
  sqlite3_mutex_leave(db->mutex);
  वापस ((rc==SQLITE_OK && bSeenBusy) ? SQLITE_BUSY : rc);
}

/*
** Configuration settings for an individual database connection
*/
पूर्णांक sqlite3_db_config(sqlite3 *db, पूर्णांक op, ...){
  बहु_सूची ap;
  पूर्णांक rc;

#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !sqlite3SafetyCheckOk(db) ) वापस SQLITE_MISUSE_BKPT;
#पूर्ण_अगर
  sqlite3_mutex_enter(db->mutex);
  बहु_शुरू(ap, op);
  चयन( op ){
    हाल SQLITE_DBCONFIG_MAINDBNAME: {
      /* IMP: R-06824-28531 */
      /* IMP: R-36257-52125 */
      db->aDb[0].zDbSName = बहु_तर्क(ap,अक्षर*);
      rc = SQLITE_OK;
      अवरोध;
    }
    हाल SQLITE_DBCONFIG_LOOKASIDE: {
      व्योम *pBuf = बहु_तर्क(ap, व्योम*); /* IMP: R-26835-10964 */
      पूर्णांक sz = बहु_तर्क(ap, पूर्णांक);       /* IMP: R-47871-25994 */
      पूर्णांक cnt = बहु_तर्क(ap, पूर्णांक);      /* IMP: R-04460-53386 */
      rc = setupLookaside(db, pBuf, sz, cnt);
      अवरोध;
    }
    हाल SQLITE_DBCONFIG_FP_DIGITS: {
      पूर्णांक nIn = बहु_तर्क(ap, पूर्णांक);
      पूर्णांक *pOut = बहु_तर्क(ap, पूर्णांक*);
      अगर( nIn>3 && nIn<24 ) db->nFpDigit = (u8)nIn;
      अगर( pOut ) *pOut = db->nFpDigit;
      rc = SQLITE_OK;
      अवरोध;
    }
    शेष: {
      अटल स्थिर काष्ठा {
        पूर्णांक op;      /* The opcode */
        u64 mask;    /* Mask of the bit in sqlite3.flags to set/clear */
      } aFlagOp[] = {
        { SQLITE_DBCONFIG_ENABLE_FKEY,           SQLITE_Foreign_कुंजियाँ    },
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
      अचिन्हित पूर्णांक i;
      rc = SQLITE_ERROR; /* IMP: R-42790-23372 */
      क्रम(i=0; i<Array_आकार(aFlagOp); i++){
        अगर( aFlagOp[i].op==op ){
          पूर्णांक onoff = बहु_तर्क(ap, पूर्णांक);
          पूर्णांक *pRes = बहु_तर्क(ap, पूर्णांक*);
          u64 oldFlags = db->flags;
          अगर( onoff>0 ){
            db->flags |= aFlagOp[i].mask;
          }अन्यथा अगर( onoff==0 ){
            db->flags &= ~(u64)aFlagOp[i].mask;
          }
          अगर( oldFlags!=db->flags ){
            sqlite3ExpirePreparedStatements(db, 0);
          }
          अगर( pRes ){
            *pRes = (db->flags & aFlagOp[i].mask)!=0;
          }
          rc = SQLITE_OK;
          अवरोध;
        }
      }
      अवरोध;
    }
  }
  बहु_पूर्ण(ap);
  sqlite3_mutex_leave(db->mutex);
  वापस rc;
}

/*
** This is the default collating function named "BINARY" which is always
** available.
*/
पूर्णांक sqlite3_द्विआधारी_Compare(
  व्योम *NotUsed,
  पूर्णांक nKey1, स्थिर व्योम *pKey1,
  पूर्णांक nKey2, स्थिर व्योम *pKey2
){
  पूर्णांक rc, n;
  UNUSED_PARAMETER(NotUsed);
  n = nKey1<nKey2 ? nKey1 : nKey2;
  /* EVIDENCE-OF: R-65033-28449 The built-in BINARY collation compares
  ** strings byte by byte using the memcmp() function from the standard C
  ** library. */
  निश्चित( pKey1 && pKey2 );
  rc = स_भेद(pKey1, pKey2, n);
  अगर( rc==0 ){
    rc = nKey1 - nKey2;
  }
  वापस rc;
}

/*
** This is the collating function named "RTRIM" which is always
** available.  Ignore trailing spaces.
*/
अटल पूर्णांक rtrimCollFunc(
  व्योम *pUser,
  पूर्णांक nKey1, स्थिर व्योम *pKey1,
  पूर्णांक nKey2, स्थिर व्योम *pKey2
){
  स्थिर u8 *pK1 = (स्थिर u8*)pKey1;
  स्थिर u8 *pK2 = (स्थिर u8*)pKey2;
  जबतक( nKey1 && pK1[nKey1-1]==' ' ) nKey1--;
  जबतक( nKey2 && pK2[nKey2-1]==' ' ) nKey2--;
  वापस sqlite3_द्विआधारी_Compare(pUser, nKey1, pKey1, nKey2, pKey2);
}

/*
** Return true if CollSeq is the default built-in BINARY.
*/
#अगर !SQLITE_USES_INLINE
पूर्णांक sqlite3_Is_द्विआधारी(स्थिर CollSeq *p){
  निश्चित( p==0 || p->xCmp!=sqlite3_द्विआधारी_Compare
       || म_भेद(p->zName,"BINARY")==0 );
  वापस p==0 || p->xCmp==sqlite3_द्विआधारी_Compare;
}
#पूर्ण_अगर

/*
** Another built-in collating sequence: NOCASE.
**
** This collating sequence is intended to be used for "case independent
** comparison". SQLite's knowledge of upper and lower case equivalents
** extends only to the 26 characters used in the English language.
**
** At the moment there is only a UTF-8 implementation.
*/
अटल पूर्णांक nocaseCollatingFunc(
  व्योम *NotUsed,
  पूर्णांक nKey1, स्थिर व्योम *pKey1,
  पूर्णांक nKey2, स्थिर व्योम *pKey2
){
  पूर्णांक r = sqlite3StrNICmp(
      (स्थिर अक्षर *)pKey1, (स्थिर अक्षर *)pKey2, (nKey1<nKey2)?nKey1:nKey2);
  UNUSED_PARAMETER(NotUsed);
  अगर( 0==r ){
    r = nKey1-nKey2;
  }
  वापस r;
}

/*
** Return the ROWID of the most recent insert
*/
sqlite_int64 sqlite3_last_जोड़ो_rowid(sqlite3 *db){
  i64 iRet;
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !sqlite3SafetyCheckOk(db) ){
    (व्योम)SQLITE_MISUSE_BKPT;
    वापस 0;
  }
#पूर्ण_अगर
  sqlite3_mutex_enter(db->mutex);
  iRet = db->lastRowid;
  sqlite3_mutex_leave(db->mutex);
  वापस iRet;
}

/*
** Set the value returned by the sqlite3_last_insert_rowid() API function.
*/
व्योम sqlite3_set_last_जोड़ो_rowid(sqlite3 *db, sqlite3_int64 iRowid){
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !sqlite3SafetyCheckOk(db) ){
    (व्योम)SQLITE_MISUSE_BKPT;
    वापस;
  }
#पूर्ण_अगर
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
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !sqlite3SafetyCheckOk(db) ){
    (व्योम)SQLITE_MISUSE_BKPT;
    वापस 0;
  }
#पूर्ण_अगर
  sqlite3_mutex_enter(db->mutex);
  iRet = db->nChange;
  sqlite3_mutex_leave(db->mutex);
  वापस iRet;
}
पूर्णांक sqlite3_changes(sqlite3 *db){
  वापस (पूर्णांक)sqlite3_changes64(db);
}

/*
** Return the number of changes since the database handle was opened.
*/
sqlite3_int64 sqlite3_योग_changes64(sqlite3 *db){
  i64 iRet;
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !sqlite3SafetyCheckOk(db) ){
    (व्योम)SQLITE_MISUSE_BKPT;
    वापस 0;
  }
#पूर्ण_अगर
  sqlite3_mutex_enter(db->mutex);
  iRet = db->n_योग_Change;
  sqlite3_mutex_leave(db->mutex);
  वापस iRet;
}
पूर्णांक sqlite3_योग_changes(sqlite3 *db){
  वापस (पूर्णांक)sqlite3_योग_changes64(db);
}

/*
** Close all open savepoints. This function only manipulates fields of the
** database handle object, it does not close any savepoints that may be open
** at the b-tree/pager level.
*/
व्योम sqlite3CloseSavepoints(sqlite3 *db){
  जबतक( db->pSavepoint ){
    Savepoint *pTmp = db->pSavepoint;
    db->pSavepoint = pTmp->p_अग्र;
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
अटल व्योम functionDestroy(sqlite3 *db, FuncDef *p){
  FuncDestructor *pDestructor;
  निश्चित( (p->funcFlags & SQLITE_FUNC_BUILTIN)==0 );
  pDestructor = p->u.pDestructor;
  अगर( pDestructor ){
    pDestructor->nRef--;
    अगर( pDestructor->nRef==0 ){
      pDestructor->xDestroy(pDestructor->p_User_तथ्य);
      sqlite3DbFree(db, pDestructor);
    }
  }
}

/*
** Disconnect all sqlite3_vtab objects that belong to database connection
** db. This is called when db is being closed.
*/
अटल व्योम disconnectAllVtab(sqlite3 *db){
#अगर_अघोषित SQLITE_OMIT_VIRTUALTABLE
  पूर्णांक i;
  HashElem *p;
  sqlite3BtreeEnterAll(db);
  क्रम(i=0; i<db->nDb; i++){
    Schema *pSchema = db->aDb[i].pSchema;
    अगर( pSchema ){
      क्रम(p=sqliteHashFirst(&pSchema->tblHash); p; p=sqlite_Hash_अग्र(p)){
        Table *pTab = (Table *)sqlite_Hash_तथ्य(p);
        अगर( IsVirtual(pTab) ) sqlite3VtabDisconnect(db, pTab);
      }
    }
  }
  क्रम(p=sqliteHashFirst(&db->aModule); p; p=sqlite_Hash_अग्र(p)){
    Module *pMod = (Module *)sqlite_Hash_तथ्य(p);
    अगर( pMod->pEpoTab ){
      sqlite3VtabDisconnect(db, pMod->pEpoTab);
    }
  }
  sqlite3VtabUnlockList(db);
  sqlite3BtreeLeaveAll(db);
#अन्यथा
  UNUSED_PARAMETER(db);
#पूर्ण_अगर
}

/*
** Return TRUE if database connection db has unfinalized prepared
** statements or unfinished sqlite3_backup objects. 
*/
अटल पूर्णांक connectionIsBusy(sqlite3 *db){
  पूर्णांक j;
  निश्चित( sqlite3_mutex_held(db->mutex) );
  अगर( db->pVdbe ) वापस 1;
  क्रम(j=0; j<db->nDb; j++){
    Btree *pBt = db->aDb[j].pBt;
    अगर( pBt && sqlite3BtreeIsInBackup(pBt) ) वापस 1;
  }
  वापस 0;
}

/*
** Close an existing SQLite database
*/
अटल पूर्णांक sqlite3Close(sqlite3 *db, पूर्णांक forceZombie){
  अगर( !db ){
    /* EVIDENCE-OF: R-63257-11740 Calling sqlite3_close() or
    ** sqlite3_close_v2() with a NULL pointer argument is a harmless no-op. */
    वापस SQLITE_OK;
  }
  अगर( !sqlite3SafetyCheckSickOrOk(db) ){
    वापस SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(db->mutex);
  अगर( db->mTrace & SQLITE_TRACE_CLOSE ){
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
  अगर( !forceZombie && connectionIsBusy(db) ){
    sqlite3_Error_With_संदेश(db, SQLITE_BUSY, "unable to close due to unfinalized "
       "statements or unfinished backups");
    sqlite3_mutex_leave(db->mutex);
    वापस SQLITE_BUSY;
  }

#अगर_घोषित SQLITE_ENABLE_SQLLOG
  अगर( sqlite3GlobalConfig.xSqllog ){
    /* Closing the handle. Fourth parameter is passed the value 2. */
    sqlite3GlobalConfig.xSqllog(sqlite3GlobalConfig.pSqllogArg, db, 0, 2);
  }
#पूर्ण_अगर

  जबतक( db->p_Db_तथ्य ){
    Db_Client_तथ्य *p = db->p_Db_तथ्य;
    db->p_Db_तथ्य = p->p_अग्र;
    निश्चित( p->p_तथ्य!=0 );
    अगर( p->xDestructor ) p->xDestructor(p->p_तथ्य);
    sqlite3_free(p);
  }

  /* Convert the connection into a zombie and then close it.
  */
  db->eOpenState = SQLITE_STATE_ZOMBIE;
  sqlite3LeaveMutexAndCloseZombie(db);
  वापस SQLITE_OK;
}

/*
** Return the transaction state for a single databse, or the maximum
** transaction state over all attached databases if zSchema is null.
*/
पूर्णांक sqlite3_txn_state(sqlite3 *db, स्थिर अक्षर *zSchema){
  पूर्णांक iDb, nDb;
  पूर्णांक iTxn = -1;
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !sqlite3SafetyCheckOk(db) ){
    (व्योम)SQLITE_MISUSE_BKPT;
    वापस -1;
  }
#पूर्ण_अगर
  sqlite3_mutex_enter(db->mutex);
  अगर( zSchema ){
    nDb = iDb = sqlite3FindDbName(db, zSchema);
    अगर( iDb<0 ) nDb--;
  }अन्यथा{
    iDb = 0;
    nDb = db->nDb-1;
  }
  क्रम(; iDb<=nDb; iDb++){
    Btree *pBt = db->aDb[iDb].pBt;
    पूर्णांक x = pBt!=0 ? sqlite3BtreeTxnState(pBt) : SQLITE_TXN_NONE;
    अगर( x>iTxn ) iTxn = x;
  }
  sqlite3_mutex_leave(db->mutex);
  वापस iTxn;
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
पूर्णांक sqlite3_close(sqlite3 *db){ वापस sqlite3Close(db,0); }
पूर्णांक sqlite3_close_v2(sqlite3 *db){ वापस sqlite3Close(db,1); }


/*
** Close the mutex on database connection db.
**
** Furthermore, if database connection db is a zombie (meaning that there
** has been a prior call to sqlite3_close(db) or sqlite3_close_v2(db)) and
** every sqlite3_stmt has now been finalized and every sqlite3_backup has
** finished, then free all resources.
*/
व्योम sqlite3LeaveMutexAndCloseZombie(sqlite3 *db){
  HashElem *i;                    /* Hash table iterator */
  पूर्णांक j;

  /* If there are outstanding sqlite3_stmt or sqlite3_backup objects
  ** or if the connection has not yet been closed by sqlite3_close_v2(),
  ** then just leave the mutex and return.
  */
  अगर( db->eOpenState!=SQLITE_STATE_ZOMBIE || connectionIsBusy(db) ){
    sqlite3_mutex_leave(db->mutex);
    वापस;
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
  क्रम(j=0; j<db->nDb; j++){
    काष्ठा Db *pDb = &db->aDb[j];
    अगर( pDb->pBt ){
      sqlite3BtreeClose(pDb->pBt);
      pDb->pBt = 0;
      अगर( j!=1 ){
        pDb->pSchema = 0;
      }
    }
  }
  /* Clear the TEMP schema separately and last */
  अगर( db->aDb[1].pSchema ){
    sqlite3SchemaClear(db->aDb[1].pSchema);
    निश्चित( db->aDb[1].pSchema->trigHash.गिनती==0 );
  }
  sqlite3VtabUnlockList(db);

  /* Free up the array of auxiliary databases */
  sqlite3CollapseDatabaseArray(db);
  निश्चित( db->nDb<=2 );
  निश्चित( db->aDb==db->aDbStatic );

  /* Tell the code in notify.c that the connection no longer holds any
  ** locks and does not require any further unlock-notify callbacks.
  */
  sqlite3ConnectionClosed(db);

  क्रम(i=sqliteHashFirst(&db->aFunc); i; i=sqlite_Hash_अग्र(i)){
    FuncDef *p_अग्र, *p;
    p = sqlite_Hash_तथ्य(i);
    करो{
      functionDestroy(db, p);
      p_अग्र = p->p_अग्र;
      sqlite3DbFree(db, p);
      p = p_अग्र;
    }जबतक( p );
  }
  sqlite3HashClear(&db->aFunc);
  क्रम(i=sqliteHashFirst(&db->aCollSeq); i; i=sqlite_Hash_अग्र(i)){
    CollSeq *pColl = (CollSeq *)sqlite_Hash_तथ्य(i);
    /* Invoke any destructors registered for collation sequence user data. */
    क्रम(j=0; j<3; j++){
      अगर( pColl[j].xDel ){
        pColl[j].xDel(pColl[j].pUser);
      }
    }
    sqlite3DbFree(db, pColl);
  }
  sqlite3HashClear(&db->aCollSeq);
#अगर_अघोषित SQLITE_OMIT_VIRTUALTABLE
  क्रम(i=sqliteHashFirst(&db->aModule); i; i=sqlite_Hash_अग्र(i)){
    Module *pMod = (Module *)sqlite_Hash_तथ्य(i);
    sqlite3VtabEponymousTableClear(db, pMod);
    sqlite3VtabModuleUnref(db, pMod);
  }
  sqlite3HashClear(&db->aModule);
#पूर्ण_अगर

  sqlite3Error(db, SQLITE_OK); /* Deallocates any cached error strings. */
  sqlite3_मूल्य_Free(db->pErr);
  sqlite3CloseExtensions(db);

  db->eOpenState = SQLITE_STATE_ERROR;

  /* The temp-database schema is allocated differently from the other schema
  ** objects (using sqliteMalloc() directly, instead of sqlite3BtreeSchema()).
  ** So it needs to be freed here. Todo: Why not roll the temp schema into
  ** the same sqliteMalloc() as the one that allocates the database
  ** structure?
  */
  sqlite3DbFree(db, db->aDb[1].pSchema);
  अगर( db->xAutovacDestr ){
    db->xAutovacDestr(db->pAutovacPagesArg);
  }
  sqlite3_mutex_leave(db->mutex);
  db->eOpenState = SQLITE_STATE_CLOSED;
  sqlite3_mutex_free(db->mutex);
  निश्चित( sqlite3LookasideUsed(db,0)==0 );
  अगर( db->lookaside.bMalloced ){
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
व्योम sqlite3RollbackAll(sqlite3 *db, पूर्णांक tripCode){
  पूर्णांक i;
  पूर्णांक inTrans = 0;
  पूर्णांक schemaChange;
  निश्चित( sqlite3_mutex_held(db->mutex) );
  sqlite3BeginBenignMalloc();

  /* Obtain all b-tree mutexes before making any calls to BtreeRollback().
  ** This is important in case the transaction being rolled back has
  ** modified the database schema. If the b-tree mutexes are not taken
  ** here, then another shared-cache connection might sneak in between
  ** the database rollback and schema reset, which can cause false
  ** corruption reports in some cases.  */
  sqlite3BtreeEnterAll(db);
  schemaChange = (db->mDbFlags & DBFLAG_SchemaChange)!=0 && db->init.busy==0;

  क्रम(i=0; i<db->nDb; i++){
    Btree *p = db->aDb[i].pBt;
    अगर( p ){
      अगर( sqlite3BtreeTxnState(p)==SQLITE_TXN_WRITE ){
        inTrans = 1;
      }
      sqlite3BtreeRollback(p, tripCode, !schemaChange);
    }
  }
  sqlite3VtabRollback(db);
  sqlite3EndBenignMalloc();

  अगर( schemaChange ){
    sqlite3ExpirePreparedStatements(db, 0);
    sqlite3ResetAllSchemasOfConnection(db);
  }
  sqlite3BtreeLeaveAll(db);

  /* Any deferred constraint violations have now been resolved. */
  db->nDeferredCons = 0;
  db->nDeferredImmCons = 0;
  db->flags &= ~(u64)(SQLITE_DeferFKs|SQLITE_CorruptRdOnly);

  /* If one has been configured, invoke the rollback-hook callback */
  अगर( db->xRollbackCallback && (inTrans || !db->autoCommit) ){
    db->xRollbackCallback(db->pRollbackArg);
  }
}

/*
** Return a static string containing the name corresponding to the error code
** specified in the argument.
*/
#अगर defined(SQLITE_NEED_ERR_NAME)
स्थिर अक्षर *sqlite3ErrName(पूर्णांक rc){
  स्थिर अक्षर *zName = 0;
  पूर्णांक i, origRc = rc;
  क्रम(i=0; i<2 && zName==0; i++, rc &= 0xff){
    चयन( rc ){
      हाल SQLITE_OK:                 zName = "SQLITE_OK";                अवरोध;
      हाल SQLITE_ERROR:              zName = "SQLITE_ERROR";             अवरोध;
      हाल SQLITE_ERROR_SNAPSHOT:     zName = "SQLITE_ERROR_SNAPSHOT";    अवरोध;
      हाल SQLITE_ERROR_RETRY:        zName = "SQLITE_ERROR_RETRY";       अवरोध;
      हाल SQLITE_ERROR_MISSING_COLLSEQ:
                                zName = "SQLITE_ERROR_MISSING_COLLSEQ";   अवरोध;
      हाल SQLITE_INTERNAL:           zName = "SQLITE_INTERNAL";          अवरोध;
      हाल SQLITE_PERM:               zName = "SQLITE_PERM";              अवरोध;
      हाल SQLITE_ABORT:              zName = "SQLITE_ABORT";             अवरोध;
      हाल SQLITE_ABORT_ROLLBACK:     zName = "SQLITE_ABORT_ROLLBACK";    अवरोध;
      हाल SQLITE_BUSY:               zName = "SQLITE_BUSY";              अवरोध;
      हाल SQLITE_BUSY_RECOVERY:      zName = "SQLITE_BUSY_RECOVERY";     अवरोध;
      हाल SQLITE_BUSY_SNAPSHOT:      zName = "SQLITE_BUSY_SNAPSHOT";     अवरोध;
      हाल SQLITE_LOCKED:             zName = "SQLITE_LOCKED";            अवरोध;
      हाल SQLITE_LOCKED_SHAREDCACHE: zName = "SQLITE_LOCKED_SHAREDCACHE";अवरोध;
      हाल SQLITE_NOMEM:              zName = "SQLITE_NOMEM";             अवरोध;
      हाल SQLITE_READONLY:           zName = "SQLITE_READONLY";          अवरोध;
      हाल SQLITE_READONLY_RECOVERY:  zName = "SQLITE_READONLY_RECOVERY"; अवरोध;
      हाल SQLITE_READONLY_CANTINIT:  zName = "SQLITE_READONLY_CANTINIT"; अवरोध;
      हाल SQLITE_READONLY_ROLLBACK:  zName = "SQLITE_READONLY_ROLLBACK"; अवरोध;
      हाल SQLITE_READONLY_DBMOVED:   zName = "SQLITE_READONLY_DBMOVED";  अवरोध;
      हाल SQLITE_READONLY_DIRECTORY: zName = "SQLITE_READONLY_DIRECTORY";अवरोध;
      हाल SQLITE_INTERRUPT:          zName = "SQLITE_INTERRUPT";         अवरोध;
      हाल SQLITE_IOERR:              zName = "SQLITE_IOERR";             अवरोध;
      हाल SQLITE_IOERR_READ:         zName = "SQLITE_IOERR_READ";        अवरोध;
      हाल SQLITE_IOERR_SHORT_READ:   zName = "SQLITE_IOERR_SHORT_READ";  अवरोध;
      हाल SQLITE_IOERR_WRITE:        zName = "SQLITE_IOERR_WRITE";       अवरोध;
      हाल SQLITE_IOERR_FSYNC:        zName = "SQLITE_IOERR_FSYNC";       अवरोध;
      हाल SQLITE_IOERR_DIR_FSYNC:    zName = "SQLITE_IOERR_DIR_FSYNC";   अवरोध;
      हाल SQLITE_IOERR_TRUNCATE:     zName = "SQLITE_IOERR_TRUNCATE";    अवरोध;
      हाल SQLITE_IOERR_FSTAT:        zName = "SQLITE_IOERR_FSTAT";       अवरोध;
      हाल SQLITE_IOERR_UNLOCK:       zName = "SQLITE_IOERR_UNLOCK";      अवरोध;
      हाल SQLITE_IOERR_RDLOCK:       zName = "SQLITE_IOERR_RDLOCK";      अवरोध;
      हाल SQLITE_IOERR_DELETE:       zName = "SQLITE_IOERR_DELETE";      अवरोध;
      हाल SQLITE_IOERR_NOMEM:        zName = "SQLITE_IOERR_NOMEM";       अवरोध;
      हाल SQLITE_IOERR_ACCESS:       zName = "SQLITE_IOERR_ACCESS";      अवरोध;
      हाल SQLITE_IOERR_CHECKRESERVEDLOCK:
                                zName = "SQLITE_IOERR_CHECKRESERVEDLOCK"; अवरोध;
      हाल SQLITE_IOERR_LOCK:         zName = "SQLITE_IOERR_LOCK";        अवरोध;
      हाल SQLITE_IOERR_CLOSE:        zName = "SQLITE_IOERR_CLOSE";       अवरोध;
      हाल SQLITE_IOERR_DIR_CLOSE:    zName = "SQLITE_IOERR_DIR_CLOSE";   अवरोध;
      हाल SQLITE_IOERR_SHMOPEN:      zName = "SQLITE_IOERR_SHMOPEN";     अवरोध;
      हाल SQLITE_IOERR_SHMSIZE:      zName = "SQLITE_IOERR_SHMSIZE";     अवरोध;
      हाल SQLITE_IOERR_SHMLOCK:      zName = "SQLITE_IOERR_SHMLOCK";     अवरोध;
      हाल SQLITE_IOERR_SHMMAP:       zName = "SQLITE_IOERR_SHMMAP";      अवरोध;
      हाल SQLITE_IOERR_SEEK:         zName = "SQLITE_IOERR_SEEK";        अवरोध;
      हाल SQLITE_IOERR_DELETE_NOENT: zName = "SQLITE_IOERR_DELETE_NOENT";अवरोध;
      हाल SQLITE_IOERR_MMAP:         zName = "SQLITE_IOERR_MMAP";        अवरोध;
      हाल SQLITE_IOERR_GETTEMPPATH:  zName = "SQLITE_IOERR_GETTEMPPATH"; अवरोध;
      हाल SQLITE_IOERR_CONVPATH:     zName = "SQLITE_IOERR_CONVPATH";    अवरोध;
      हाल SQLITE_CORRUPT:            zName = "SQLITE_CORRUPT";           अवरोध;
      हाल SQLITE_CORRUPT_VTAB:       zName = "SQLITE_CORRUPT_VTAB";      अवरोध;
      हाल SQLITE_NOTFOUND:           zName = "SQLITE_NOTFOUND";          अवरोध;
      हाल SQLITE_FULL:               zName = "SQLITE_FULL";              अवरोध;
      हाल SQLITE_CANTOPEN:           zName = "SQLITE_CANTOPEN";          अवरोध;
      हाल SQLITE_CANTOPEN_NOTEMPDIR: zName = "SQLITE_CANTOPEN_NOTEMPDIR";अवरोध;
      हाल SQLITE_CANTOPEN_ISDIR:     zName = "SQLITE_CANTOPEN_ISDIR";    अवरोध;
      हाल SQLITE_CANTOPEN_FULLPATH:  zName = "SQLITE_CANTOPEN_FULLPATH"; अवरोध;
      हाल SQLITE_CANTOPEN_CONVPATH:  zName = "SQLITE_CANTOPEN_CONVPATH"; अवरोध;
      हाल SQLITE_CANTOPEN_SYMLINK:   zName = "SQLITE_CANTOPEN_SYMLINK";  अवरोध;
      हाल SQLITE_PROTOCOL:           zName = "SQLITE_PROTOCOL";          अवरोध;
      हाल SQLITE_EMPTY:              zName = "SQLITE_EMPTY";             अवरोध;
      हाल SQLITE_SCHEMA:             zName = "SQLITE_SCHEMA";            अवरोध;
      हाल SQLITE_TOOBIG:             zName = "SQLITE_TOOBIG";            अवरोध;
      हाल SQLITE_CONSTRAINT:         zName = "SQLITE_CONSTRAINT";        अवरोध;
      हाल SQLITE_CONSTRAINT_UNIQUE:  zName = "SQLITE_CONSTRAINT_UNIQUE"; अवरोध;
      हाल SQLITE_CONSTRAINT_TRIGGER: zName = "SQLITE_CONSTRAINT_TRIGGER";अवरोध;
      हाल SQLITE_CONSTRAINT_FOREIGNKEY:
                                zName = "SQLITE_CONSTRAINT_FOREIGNKEY";   अवरोध;
      हाल SQLITE_CONSTRAINT_CHECK:   zName = "SQLITE_CONSTRAINT_CHECK";  अवरोध;
      हाल SQLITE_CONSTRAINT_PRIMARYKEY:
                                zName = "SQLITE_CONSTRAINT_PRIMARYKEY";   अवरोध;
      हाल SQLITE_CONSTRAINT_NOTNULL: zName = "SQLITE_CONSTRAINT_NOTNULL";अवरोध;
      हाल SQLITE_CONSTRAINT_COMMITHOOK:
                                zName = "SQLITE_CONSTRAINT_COMMITHOOK";   अवरोध;
      हाल SQLITE_CONSTRAINT_VTAB:    zName = "SQLITE_CONSTRAINT_VTAB";   अवरोध;
      हाल SQLITE_CONSTRAINT_FUNCTION:
                                zName = "SQLITE_CONSTRAINT_FUNCTION";     अवरोध;
      हाल SQLITE_CONSTRAINT_ROWID:   zName = "SQLITE_CONSTRAINT_ROWID";  अवरोध;
      हाल SQLITE_MISMATCH:           zName = "SQLITE_MISMATCH";          अवरोध;
      हाल SQLITE_MISUSE:             zName = "SQLITE_MISUSE";            अवरोध;
      हाल SQLITE_NOLFS:              zName = "SQLITE_NOLFS";             अवरोध;
      हाल SQLITE_AUTH:               zName = "SQLITE_AUTH";              अवरोध;
      हाल SQLITE_FORMAT:             zName = "SQLITE_FORMAT";            अवरोध;
      हाल SQLITE_RANGE:              zName = "SQLITE_RANGE";             अवरोध;
      हाल SQLITE_NOTADB:             zName = "SQLITE_NOTADB";            अवरोध;
      हाल SQLITE_ROW:                zName = "SQLITE_ROW";               अवरोध;
      हाल SQLITE_NOTICE:             zName = "SQLITE_NOTICE";            अवरोध;
      हाल SQLITE_NOTICE_RECOVER_WAL: zName = "SQLITE_NOTICE_RECOVER_WAL";अवरोध;
      हाल SQLITE_NOTICE_RECOVER_ROLLBACK:
                                zName = "SQLITE_NOTICE_RECOVER_ROLLBACK"; अवरोध;
      हाल SQLITE_NOTICE_RBU:         zName = "SQLITE_NOTICE_RBU"; अवरोध;
      हाल SQLITE_WARNING:            zName = "SQLITE_WARNING";           अवरोध;
      हाल SQLITE_WARNING_AUTOINDEX:  zName = "SQLITE_WARNING_AUTOINDEX"; अवरोध;
      हाल SQLITE_DONE:               zName = "SQLITE_DONE";              अवरोध;
    }
  }
  अगर( zName==0 ){
    अटल अक्षर zBuf[50];
    sqlite3_snprintf(माप(zBuf), zBuf, "SQLITE_UNKNOWN(%d)", origRc);
    zName = zBuf;
  }
  वापस zName;
}
#पूर्ण_अगर

/*
** Return a static string that describes the kind of error specified in the
** argument.
*/
स्थिर अक्षर *sqlite3ErrStr(पूर्णांक rc){
  अटल स्थिर अक्षर* स्थिर a_संदेश[] = {
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
#अगर_घोषित SQLITE_DISABLE_LFS
    /* SQLITE_NOLFS       */ "large file support is disabled",
#अन्यथा
    /* SQLITE_NOLFS       */ 0,
#पूर्ण_अगर
    /* SQLITE_AUTH        */ "authorization denied",
    /* SQLITE_FORMAT      */ 0,
    /* SQLITE_RANGE       */ "column index out of range",
    /* SQLITE_NOTADB      */ "file is not a database",
    /* SQLITE_NOTICE      */ "notification message",
    /* SQLITE_WARNING     */ "warning message",
  };
  स्थिर अक्षर *zErr = "unknown error";
  चयन( rc ){
    हाल SQLITE_ABORT_ROLLBACK: {
      zErr = "abort due to ROLLBACK";
      अवरोध;
    }
    हाल SQLITE_ROW: {
      zErr = "another row available";
      अवरोध;
    }
    हाल SQLITE_DONE: {
      zErr = "no more rows available";
      अवरोध;
    }
    शेष: {
      rc &= 0xff;
      अगर( ALWAYS(rc>=0) && rc<Array_आकार(a_संदेश) && a_संदेश[rc]!=0 ){
        zErr = a_संदेश[rc];
      }
      अवरोध;
    }
  }
  वापस zErr;
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
अटल पूर्णांक sqliteDefaultBusyCallback(
  व्योम *ptr,               /* Database connection */
  पूर्णांक गिनती                /* Number of times table has been busy */
){
#अगर SQLITE_OS_WIN || !defined(HAVE_NANOSLEEP) || HAVE_NANOSLEEP
  /* This case is for systems that have support for sleeping for fractions of
  ** a second.  Examples:  All windows systems, unix systems with nanosleep() */
  अटल स्थिर u8 delays[] =
     { 1, 2, 5, 10, 15, 20, 25, 25,  25,  50,  50, 100 };
  अटल स्थिर u8 totals[] =
     { 0, 1, 3,  8, 18, 33, 53, 78, 103, 128, 178, 228 };
# define NDELAY Array_आकार(delays)
  sqlite3 *db = (sqlite3 *)ptr;
  पूर्णांक tmout = db->busyTimeout;
  पूर्णांक delay, prior;

  निश्चित( गिनती>=0 );
  अगर( गिनती < NDELAY ){
    delay = delays[गिनती];
    prior = totals[गिनती];
  }अन्यथा{
    delay = delays[NDELAY-1];
    prior = totals[NDELAY-1] + delay*(गिनती-(NDELAY-1));
  }
  अगर( prior + delay > tmout ){
    delay = tmout - prior;
    अगर( delay<=0 ) वापस 0;
  }
  sqlite3OsSleep(db->pVfs, delay*1000);
  वापस 1;
#अन्यथा
  /* This case for unix systems that lack usleep() support.  Sleeping
  ** must be done in increments of whole seconds */
  sqlite3 *db = (sqlite3 *)ptr;
  पूर्णांक tmout = ((sqlite3 *)ptr)->busyTimeout;
  अगर( (गिनती+1)*1000 > tmout ){
    वापस 0;
  }
  sqlite3OsSleep(db->pVfs, 1000000);
  वापस 1;
#पूर्ण_अगर
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
पूर्णांक sqlite3InvokeBusyHandler(BusyHandler *p){
  पूर्णांक rc;
  अगर( p->xBusyHandler==0 || p->nBusy<0 ) वापस 0;
  rc = p->xBusyHandler(p->pBusyArg, p->nBusy);
  अगर( rc==0 ){
    p->nBusy = -1;
  }अन्यथा{
    p->nBusy++;
  }
  वापस rc;
}

/*
** This routine sets the busy callback for an Sqlite database to the
** given callback function with the given argument.
*/
पूर्णांक sqlite3_busy_handler(
  sqlite3 *db,
  पूर्णांक (*xBusy)(व्योम*,पूर्णांक),
  व्योम *pArg
){
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !sqlite3SafetyCheckOk(db) ) वापस SQLITE_MISUSE_BKPT;
#पूर्ण_अगर
  sqlite3_mutex_enter(db->mutex);
  db->busyHandler.xBusyHandler = xBusy;
  db->busyHandler.pBusyArg = pArg;
  db->busyHandler.nBusy = 0;
  db->busyTimeout = 0;
#अगर_घोषित SQLITE_ENABLE_SETLK_TIMEOUT
  db->setlkTimeout = 0;
#पूर्ण_अगर
  sqlite3_mutex_leave(db->mutex);
  वापस SQLITE_OK;
}

#अगर_अघोषित SQLITE_OMIT_PROGRESS_CALLBACK
/*
** This routine sets the progress callback for an Sqlite database to the
** given callback function with the given argument. The progress callback will
** be invoked every nOps opcodes.
*/
व्योम sqlite3_progress_handler(
  sqlite3 *db,
  पूर्णांक nOps,
  पूर्णांक (*xProgress)(व्योम*),
  व्योम *pArg
){
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !sqlite3SafetyCheckOk(db) ){
    (व्योम)SQLITE_MISUSE_BKPT;
    वापस;
  }
#पूर्ण_अगर
  sqlite3_mutex_enter(db->mutex);
  अगर( nOps>0 ){
    db->xProgress = xProgress;
    db->nProgressOps = (अचिन्हित)nOps;
    db->pProgressArg = pArg;
  }अन्यथा{
    db->xProgress = 0;
    db->nProgressOps = 0;
    db->pProgressArg = 0;
  }
  sqlite3_mutex_leave(db->mutex);
}
#पूर्ण_अगर


/*
** This routine installs a default busy handler that waits for the
** specified number of milliseconds before returning 0.
*/
पूर्णांक sqlite3_busy_timeout(sqlite3 *db, पूर्णांक ms){
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !sqlite3SafetyCheckOk(db) ) वापस SQLITE_MISUSE_BKPT;
#पूर्ण_अगर
  sqlite3_mutex_enter(db->mutex);
  अगर( ms>0 ){
    sqlite3_busy_handler(db, (पूर्णांक(*)(व्योम*,पूर्णांक))sqliteDefaultBusyCallback,
                             (व्योम*)db);
    db->busyTimeout = ms;
#अगर_घोषित SQLITE_ENABLE_SETLK_TIMEOUT
    db->setlkTimeout = ms;
#पूर्ण_अगर
  }अन्यथा{
    sqlite3_busy_handler(db, 0, 0);
  }
  sqlite3_mutex_leave(db->mutex);
  वापस SQLITE_OK;
}

/*
** Set the setlk timeout value.
*/
पूर्णांक sqlite3_setlk_timeout(sqlite3 *db, पूर्णांक ms, पूर्णांक flags){
#अगर_घोषित SQLITE_ENABLE_SETLK_TIMEOUT
  पूर्णांक iDb;
  पूर्णांक bBOC = ((flags & SQLITE_SETLK_BLOCK_ON_CONNECT) ? 1 : 0);
#पूर्ण_अगर
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !sqlite3SafetyCheckOk(db) ) वापस SQLITE_MISUSE_BKPT;
#पूर्ण_अगर
  अगर( ms<-1 ) वापस SQLITE_RANGE;
#अगर_घोषित SQLITE_ENABLE_SETLK_TIMEOUT
  sqlite3_mutex_enter(db->mutex);
  db->setlkTimeout = ms;
  db->setlkFlags = flags;
  sqlite3BtreeEnterAll(db);
  क्रम(iDb=0; iDb<db->nDb; iDb++){
    Btree *pBt = db->aDb[iDb].pBt;
    अगर( pBt ){
      sqlite3_file *fd = sqlite3PagerFile(sqlite3BtreePager(pBt));
      sqlite3OsFileControlHint(fd, SQLITE_FCNTL_BLOCK_ON_CONNECT, (व्योम*)&bBOC);
    }
  }
  sqlite3BtreeLeaveAll(db);
  sqlite3_mutex_leave(db->mutex);
#पूर्ण_अगर
#अगर !defined(SQLITE_ENABLE_API_ARMOR) && !defined(SQLITE_ENABLE_SETLK_TIMEOUT)
  UNUSED_PARAMETER(db);
  UNUSED_PARAMETER(flags);
#पूर्ण_अगर
  वापस SQLITE_OK;
}

/*
** Cause any pending operation to stop at its earliest opportunity.
*/
व्योम sqlite3_interrupt(sqlite3 *db){
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !sqlite3SafetyCheckOk(db)
   && (db==0 || db->eOpenState!=SQLITE_STATE_ZOMBIE)
  ){
    (व्योम)SQLITE_MISUSE_BKPT;
    वापस;
  }
#पूर्ण_अगर
  AtomicStore(&db->u1.isInterrupted, 1);
}

/*
** Return true or false depending on whether or not an interrupt is
** pending on connection db.
*/
पूर्णांक sqlite3_is_interrupted(sqlite3 *db){
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !sqlite3SafetyCheckOk(db)
   && (db==0 || db->eOpenState!=SQLITE_STATE_ZOMBIE)
  ){
    (व्योम)SQLITE_MISUSE_BKPT;
    वापस 0;
  }
#पूर्ण_अगर
  वापस AtomicLoad(&db->u1.isInterrupted)!=0;
}

/*
** This function is exactly the same as sqlite3_create_function(), except
** that it is designed to be called by internal code. The difference is
** that if a malloc() fails in sqlite3_create_function(), an error code
** is returned and the mallocFailed flag cleared.
*/
पूर्णांक sqlite3CreateFunc(
  sqlite3 *db,
  स्थिर अक्षर *zFunctionName,
  पूर्णांक nArg,
  पूर्णांक enc,
  व्योम *p_User_तथ्य,
  व्योम (*xSFunc)(sqlite3_context*,पूर्णांक,sqlite3_मूल्य **),
  व्योम (*xStep)(sqlite3_context*,पूर्णांक,sqlite3_मूल्य **),
  व्योम (*xFinal)(sqlite3_context*),
  व्योम (*x_मूल्य)(sqlite3_context*),
  व्योम (*xInverse)(sqlite3_context*,पूर्णांक,sqlite3_मूल्य **),
  FuncDestructor *pDestructor
){
  FuncDef *p;
  पूर्णांक extraFlags;

  निश्चित( sqlite3_mutex_held(db->mutex) );
  निश्चित( x_मूल्य==0 || xSFunc==0 );
  अगर( zFunctionName==0                /* Must have a valid name */
   || (xSFunc!=0 && xFinal!=0)        /* Not both xSFunc and xFinal */
   || ((xFinal==0)!=(xStep==0))       /* Both or neither of xFinal and xStep */
   || ((x_मूल्य==0)!=(xInverse==0))    /* Both or neither of xValue, xInverse */
   || (nArg<-1 || nArg>SQLITE_MAX_FUNCTION_ARG)
   || (255<sqlite3Strlen30(zFunctionName))
  ){
    वापस SQLITE_MISUSE_BKPT;
  }

  निश्चित( SQLITE_FUNC_CONSTANT==SQLITE_DETERMINISTIC );
  निश्चित( SQLITE_FUNC_DIRECT==SQLITE_DIRECTONLY );
  extraFlags = enc &  (SQLITE_DETERMINISTIC|SQLITE_DIRECTONLY|
                       SQLITE_SUBTYPE|SQLITE_INNOCUOUS|
                       SQLITE_परिणाम_SUBTYPE|SQLITE_SELFORDER1);
  enc &= (SQLITE_FUNC_ENCMASK|SQLITE_ANY);

  /* The SQLITE_INNOCUOUS flag is the same bit as SQLITE_FUNC_UNSAFE.  But
  ** the meaning is inverted.  So flip the bit. */
  निश्चित( SQLITE_FUNC_UNSAFE==SQLITE_INNOCUOUS );
  extraFlags ^= SQLITE_FUNC_UNSAFE;  /* tag-20230109-1 */

 
#अगर_अघोषित SQLITE_OMIT_UTF16
  /* If SQLITE_UTF16 is specified as the encoding type, transform this
  ** to one of SQLITE_UTF16LE or SQLITE_UTF16BE using the
  ** SQLITE_UTF16NATIVE macro. SQLITE_UTF16 is not used internally.
  **
  ** If SQLITE_ANY is specified, add three versions of the function
  ** to the hash table.
  */
  चयन( enc ){
    हाल SQLITE_UTF16:
      enc = SQLITE_UTF16NATIVE;
      अवरोध;
    हाल SQLITE_ANY: {
      पूर्णांक rc;
      rc = sqlite3CreateFunc(db, zFunctionName, nArg,
           (SQLITE_UTF8|extraFlags)^SQLITE_FUNC_UNSAFE, /* tag-20230109-1 */
           p_User_तथ्य, xSFunc, xStep, xFinal, x_मूल्य, xInverse, pDestructor);
      अगर( rc==SQLITE_OK ){
        rc = sqlite3CreateFunc(db, zFunctionName, nArg,
             (SQLITE_UTF16LE|extraFlags)^SQLITE_FUNC_UNSAFE, /* tag-20230109-1*/
             p_User_तथ्य, xSFunc, xStep, xFinal, x_मूल्य, xInverse, pDestructor);
      }
      अगर( rc!=SQLITE_OK ){
        वापस rc;
      }
      enc = SQLITE_UTF16BE;
      अवरोध;
    }
    हाल SQLITE_UTF8:
    हाल SQLITE_UTF16LE:
    हाल SQLITE_UTF16BE:
      अवरोध;
    शेष:
      enc = SQLITE_UTF8;
      अवरोध;
  }
#अन्यथा
  enc = SQLITE_UTF8;
#पूर्ण_अगर
 
  /* Check if an existing function is being overridden or deleted. If so,
  ** and there are active VMs, then return SQLITE_BUSY. If a function
  ** is being overridden/deleted but there are no active VMs, allow the
  ** operation to continue but invalidate all precompiled statements.
  */
  p = sqlite3FindFunction(db, zFunctionName, nArg, (u8)enc, 0);
  अगर( p && (p->funcFlags & SQLITE_FUNC_ENCMASK)==(u32)enc && p->nArg==nArg ){
    अगर( db->nVdbeActive ){
      sqlite3_Error_With_संदेश(db, SQLITE_BUSY,
        "unable to delete/modify user-function due to active statements");
      निश्चित( !db->mallocFailed || db->nVdbeExec>0 );
      वापस SQLITE_BUSY;
    }अन्यथा{
      sqlite3ExpirePreparedStatements(db, 0);
    }
  }अन्यथा अगर( xSFunc==0 && xFinal==0 ){
    /* Trying to delete a function that does not exist.  This is a no-op.
    ** https://sqlite.org/forum/forumpost/726219164b */
    वापस SQLITE_OK;
  }

  p = sqlite3FindFunction(db, zFunctionName, nArg, (u8)enc, 1);
  निश्चित(p || db->mallocFailed);
  अगर( !p ){
    वापस SQLITE_NOMEM_BKPT;
  }

  /* If an older version of the function with a configured destructor is
  ** being replaced invoke the destructor function here. */
  functionDestroy(db, p);

  अगर( pDestructor ){
    pDestructor->nRef++;
  }
  p->u.pDestructor = pDestructor;
  p->funcFlags = (p->funcFlags & SQLITE_FUNC_ENCMASK) | extraFlags;
  testcase( p->funcFlags & SQLITE_DETERMINISTIC );
  testcase( p->funcFlags & SQLITE_DIRECTONLY );
  p->xSFunc = xSFunc ? xSFunc : xStep;
  p->xFinalize = xFinal;
  p->x_मूल्य = x_मूल्य;
  p->xInverse = xInverse;
  p->p_User_तथ्य = p_User_तथ्य;
  p->nArg = (u16)nArg;
  वापस SQLITE_OK;
}

/*
** Worker function used by utf-8 APIs that create new functions:
**
**    sqlite3_create_function()
**    sqlite3_create_function_v2()
**    sqlite3_create_window_function()
*/
अटल पूर्णांक createFunctionApi(
  sqlite3 *db,
  स्थिर अक्षर *zFunc,
  पूर्णांक nArg,
  पूर्णांक enc,
  व्योम *p,
  व्योम (*xSFunc)(sqlite3_context*,पूर्णांक,sqlite3_मूल्य**),
  व्योम (*xStep)(sqlite3_context*,पूर्णांक,sqlite3_मूल्य**),
  व्योम (*xFinal)(sqlite3_context*),
  व्योम (*x_मूल्य)(sqlite3_context*),
  व्योम (*xInverse)(sqlite3_context*,पूर्णांक,sqlite3_मूल्य**),
  व्योम(*xDestroy)(व्योम*)
){
  पूर्णांक rc = SQLITE_ERROR;
  FuncDestructor *pArg = 0;

#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !sqlite3SafetyCheckOk(db) ){
    वापस SQLITE_MISUSE_BKPT;
  }
#पूर्ण_अगर
  sqlite3_mutex_enter(db->mutex);
  अगर( xDestroy ){
    pArg = (FuncDestructor *)sqlite3Malloc(माप(FuncDestructor));
    अगर( !pArg ){
      sqlite3OomFault(db);
      xDestroy(p);
      जाओ out;
    }
    pArg->nRef = 0;
    pArg->xDestroy = xDestroy;
    pArg->p_User_तथ्य = p;
  }
  rc = sqlite3CreateFunc(db, zFunc, nArg, enc, p,
      xSFunc, xStep, xFinal, x_मूल्य, xInverse, pArg
  );
  अगर( pArg && pArg->nRef==0 ){
    निश्चित( rc!=SQLITE_OK || (xStep==0 && xFinal==0) );
    xDestroy(p);
    sqlite3_free(pArg);
  }

 out:
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  वापस rc;
}

/*
** Create new user functions.
*/
पूर्णांक sqlite3_create_function(
  sqlite3 *db,
  स्थिर अक्षर *zFunc,
  पूर्णांक nArg,
  पूर्णांक enc,
  व्योम *p,
  व्योम (*xSFunc)(sqlite3_context*,पूर्णांक,sqlite3_मूल्य **),
  व्योम (*xStep)(sqlite3_context*,पूर्णांक,sqlite3_मूल्य **),
  व्योम (*xFinal)(sqlite3_context*)
){
  वापस createFunctionApi(db, zFunc, nArg, enc, p, xSFunc, xStep,
                                    xFinal, 0, 0, 0);
}
पूर्णांक sqlite3_create_function_v2(
  sqlite3 *db,
  स्थिर अक्षर *zFunc,
  पूर्णांक nArg,
  पूर्णांक enc,
  व्योम *p,
  व्योम (*xSFunc)(sqlite3_context*,पूर्णांक,sqlite3_मूल्य **),
  व्योम (*xStep)(sqlite3_context*,पूर्णांक,sqlite3_मूल्य **),
  व्योम (*xFinal)(sqlite3_context*),
  व्योम (*xDestroy)(व्योम *)
){
  वापस createFunctionApi(db, zFunc, nArg, enc, p, xSFunc, xStep,
                                    xFinal, 0, 0, xDestroy);
}
पूर्णांक sqlite3_create_window_function(
  sqlite3 *db,
  स्थिर अक्षर *zFunc,
  पूर्णांक nArg,
  पूर्णांक enc,
  व्योम *p,
  व्योम (*xStep)(sqlite3_context*,पूर्णांक,sqlite3_मूल्य **),
  व्योम (*xFinal)(sqlite3_context*),
  व्योम (*x_मूल्य)(sqlite3_context*),
  व्योम (*xInverse)(sqlite3_context*,पूर्णांक,sqlite3_मूल्य **),
  व्योम (*xDestroy)(व्योम *)
){
  वापस createFunctionApi(db, zFunc, nArg, enc, p, 0, xStep,
                                    xFinal, x_मूल्य, xInverse, xDestroy);
}

#अगर_अघोषित SQLITE_OMIT_UTF16
पूर्णांक sqlite3_create_function16(
  sqlite3 *db,
  स्थिर व्योम *zFunctionName,
  पूर्णांक nArg,
  पूर्णांक eTextRep,
  व्योम *p,
  व्योम (*xSFunc)(sqlite3_context*,पूर्णांक,sqlite3_मूल्य**),
  व्योम (*xStep)(sqlite3_context*,पूर्णांक,sqlite3_मूल्य**),
  व्योम (*xFinal)(sqlite3_context*)
){
  पूर्णांक rc;
  अक्षर *zFunc8;

#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !sqlite3SafetyCheckOk(db) || zFunctionName==0 ) वापस SQLITE_MISUSE_BKPT;
#पूर्ण_अगर
  sqlite3_mutex_enter(db->mutex);
  निश्चित( !db->mallocFailed );
  zFunc8 = sqlite3Utf16to8(db, zFunctionName, -1, SQLITE_UTF16NATIVE);
  rc = sqlite3CreateFunc(db, zFunc8, nArg, eTextRep, p, xSFunc,xStep,xFinal,0,0,0);
  sqlite3DbFree(db, zFunc8);
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  वापस rc;
}
#पूर्ण_अगर


/*
** The following is the implementation of an SQL function that always
** fails with an error message stating that the function is used in the
** wrong context.  The sqlite3_overload_function() API might construct
** SQL function that use this routine so that the functions will exist
** for name resolution but are actually overloaded by the xFindFunction
** method of virtual tables.
*/
अटल व्योम sqlite3InvalidFunction(
  sqlite3_context *context,  /* The function calling context */
  पूर्णांक NotUsed,               /* Number of arguments to the function */
  sqlite3_मूल्य **NotUsed2   /* Value of each argument */
){
  स्थिर अक्षर *zName = (स्थिर अक्षर*)sqlite3_user_तथ्य(context);
  अक्षर *zErr;
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  zErr = sqlite3_mprintf(
      "unable to use function %s in the requested context", zName);
  sqlite3_परिणाम_error(context, zErr, -1);
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
पूर्णांक sqlite3_overload_function(
  sqlite3 *db,
  स्थिर अक्षर *zName,
  पूर्णांक nArg
){
  पूर्णांक rc;
  अक्षर *zCopy;

#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !sqlite3SafetyCheckOk(db) || zName==0 || nArg<-2 ){
    वापस SQLITE_MISUSE_BKPT;
  }
#पूर्ण_अगर
  sqlite3_mutex_enter(db->mutex);
  rc = sqlite3FindFunction(db, zName, nArg, SQLITE_UTF8, 0)!=0;
  sqlite3_mutex_leave(db->mutex);
  अगर( rc ) वापस SQLITE_OK;
  zCopy = sqlite3_mprintf("%s", zName);
  अगर( zCopy==0 ) वापस SQLITE_NOMEM;
  वापस sqlite3_create_function_v2(db, zName, nArg, SQLITE_UTF8,
                           zCopy, sqlite3InvalidFunction, 0, 0, sqlite3_free);
}

#अगर_अघोषित SQLITE_OMIT_TRACE
/*
** Register a trace function.  The pArg from the previously registered trace
** is returned. 
**
** A NULL trace function means that no tracing is executes.  A non-NULL
** trace is a pointer to a function that is invoked at the start of each
** SQL statement.
*/
#अगर_अघोषित SQLITE_OMIT_DEPRECATED
व्योम *sqlite3_trace(sqlite3 *db, व्योम(*xTrace)(व्योम*,स्थिर अक्षर*), व्योम *pArg){
  व्योम *pOld;

#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !sqlite3SafetyCheckOk(db) ){
    (व्योम)SQLITE_MISUSE_BKPT;
    वापस 0;
  }
#पूर्ण_अगर
  sqlite3_mutex_enter(db->mutex);
  pOld = db->pTraceArg;
  db->mTrace = xTrace ? SQLITE_TRACE_LEGACY : 0;
  db->trace.xLegacy = xTrace;
  db->pTraceArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  वापस pOld;
}
#पूर्ण_अगर /* SQLITE_OMIT_DEPRECATED */

/* Register a trace callback using the version-2 interface.
*/
पूर्णांक sqlite3_trace_v2(
  sqlite3 *db,                               /* Trace this connection */
  अचिन्हित mTrace,                           /* Mask of events to be traced */
  पूर्णांक(*xTrace)(अचिन्हित,व्योम*,व्योम*,व्योम*),  /* Callback to invoke */
  व्योम *pArg                                 /* Context */
){
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !sqlite3SafetyCheckOk(db) ){
    वापस SQLITE_MISUSE_BKPT;
  }
#पूर्ण_अगर
  sqlite3_mutex_enter(db->mutex);
  अगर( mTrace==0 ) xTrace = 0;
  अगर( xTrace==0 ) mTrace = 0;
  db->mTrace = mTrace;
  db->trace.xV2 = xTrace;
  db->pTraceArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  वापस SQLITE_OK;
}

#अगर_अघोषित SQLITE_OMIT_DEPRECATED
/*
** Register a profile function.  The pArg from the previously registered
** profile function is returned. 
**
** A NULL profile function means that no profiling is executes.  A non-NULL
** profile is a pointer to a function that is invoked at the conclusion of
** each SQL statement that is run.
*/
व्योम *sqlite3_profile(
  sqlite3 *db,
  व्योम (*xProfile)(व्योम*,स्थिर अक्षर*,sqlite_uint64),
  व्योम *pArg
){
  व्योम *pOld;

#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !sqlite3SafetyCheckOk(db) ){
    (व्योम)SQLITE_MISUSE_BKPT;
    वापस 0;
  }
#पूर्ण_अगर
  sqlite3_mutex_enter(db->mutex);
  pOld = db->pProfileArg;
  db->xProfile = xProfile;
  db->pProfileArg = pArg;
  db->mTrace &= SQLITE_TRACE_NONLEGACY_MASK;
  अगर( db->xProfile ) db->mTrace |= SQLITE_TRACE_XPROFILE;
  sqlite3_mutex_leave(db->mutex);
  वापस pOld;
}
#पूर्ण_अगर /* SQLITE_OMIT_DEPRECATED */
#पूर्ण_अगर /* SQLITE_OMIT_TRACE */

/*
** Register a function to be invoked when a transaction commits.
** If the invoked function returns non-zero, then the commit becomes a
** rollback.
*/
व्योम *sqlite3_commit_hook(
  sqlite3 *db,              /* Attach the hook to this database */
  पूर्णांक (*xCallback)(व्योम*),  /* Function to invoke on each commit */
  व्योम *pArg                /* Argument to the function */
){
  व्योम *pOld;

#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !sqlite3SafetyCheckOk(db) ){
    (व्योम)SQLITE_MISUSE_BKPT;
    वापस 0;
  }
#पूर्ण_अगर
  sqlite3_mutex_enter(db->mutex);
  pOld = db->pCommitArg;
  db->xCommitCallback = xCallback;
  db->pCommitArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  वापस pOld;
}

/*
** Register a callback to be invoked each time a row is updated,
** inserted or deleted using this database connection.
*/
व्योम *sqlite3_update_hook(
  sqlite3 *db,              /* Attach the hook to this database */
  व्योम (*xCallback)(व्योम*,पूर्णांक,अक्षर स्थिर *,अक्षर स्थिर *,sqlite_int64),
  व्योम *pArg                /* Argument to the function */
){
  व्योम *pRet;

#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !sqlite3SafetyCheckOk(db) ){
    (व्योम)SQLITE_MISUSE_BKPT;
    वापस 0;
  }
#पूर्ण_अगर
  sqlite3_mutex_enter(db->mutex);
  pRet = db->pUpdateArg;
  db->xUpdateCallback = xCallback;
  db->pUpdateArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  वापस pRet;
}

/*
** Register a callback to be invoked each time a transaction is rolled
** back by this database connection.
*/
व्योम *sqlite3_rollback_hook(
  sqlite3 *db,              /* Attach the hook to this database */
  व्योम (*xCallback)(व्योम*), /* Callback function */
  व्योम *pArg                /* Argument to the function */
){
  व्योम *pRet;

#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !sqlite3SafetyCheckOk(db) ){
    (व्योम)SQLITE_MISUSE_BKPT;
    वापस 0;
  }
#पूर्ण_अगर
  sqlite3_mutex_enter(db->mutex);
  pRet = db->pRollbackArg;
  db->xRollbackCallback = xCallback;
  db->pRollbackArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  वापस pRet;
}

#अगर_घोषित SQLITE_ENABLE_PREUPDATE_HOOK
/*
** Register a callback to be invoked each time a row is updated,
** inserted or deleted using this database connection.
*/
व्योम *sqlite3_preupdate_hook(
  sqlite3 *db,              /* Attach the hook to this database */
  व्योम(*xCallback)(         /* Callback function */
    व्योम*,sqlite3*,पूर्णांक,अक्षर स्थिर*,अक्षर स्थिर*,sqlite3_int64,sqlite3_int64),
  व्योम *pArg                /* First callback argument */
){
  व्योम *pRet;

#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( db==0 ){
    वापस 0;
  }
#पूर्ण_अगर
  sqlite3_mutex_enter(db->mutex);
  pRet = db->pPreUpdateArg;
  db->xPreUpdateCallback = xCallback;
  db->pPreUpdateArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  वापस pRet;
}
#पूर्ण_अगर /* SQLITE_ENABLE_PREUPDATE_HOOK */

/*
** Register a function to be invoked prior to each autovacuum that
** determines the number of pages to vacuum.
*/
पूर्णांक sqlite3_autovacuum_pages(
  sqlite3 *db,                 /* Attach the hook to this database */
  अचिन्हित पूर्णांक (*xCallback)(व्योम*,स्थिर अक्षर*,u32,u32,u32),
  व्योम *pArg,                  /* Argument to the function */
  व्योम (*xDestructor)(व्योम*)   /* Destructor for pArg */
){
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !sqlite3SafetyCheckOk(db) ){
    अगर( xDestructor ) xDestructor(pArg);
    वापस SQLITE_MISUSE_BKPT;
  }
#पूर्ण_अगर
  sqlite3_mutex_enter(db->mutex);
  अगर( db->xAutovacDestr ){
    db->xAutovacDestr(db->pAutovacPagesArg);
  }
  db->xAutovacPages = xCallback;
  db->pAutovacPagesArg = pArg;
  db->xAutovacDestr = xDestructor;
  sqlite3_mutex_leave(db->mutex);
  वापस SQLITE_OK;
}


#अगर_अघोषित SQLITE_OMIT_WAL
/*
** The sqlite3_wal_hook() callback registered by sqlite3_wal_autocheckpoint().
** Invoke sqlite3_wal_checkpoint if the number of frames in the log file
** is greater than sqlite3.pWalArg cast to an integer (the value configured by
** wal_autocheckpoint()).
*/
पूर्णांक sqlite3WalDefaultHook(
  व्योम *p_Client_तथ्य,     /* Argument */
  sqlite3 *db,           /* Connection */
  स्थिर अक्षर *zDb,       /* Database */
  पूर्णांक nFrame             /* Size of WAL */
){
  अगर( nFrame>=SQLITE_PTR_TO_INT(p_Client_तथ्य) ){
    sqlite3BeginBenignMalloc();
    sqlite3_wal_checkpoint(db, zDb);
    sqlite3EndBenignMalloc();
  }
  वापस SQLITE_OK;
}
#पूर्ण_अगर /* SQLITE_OMIT_WAL */

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
पूर्णांक sqlite3_wal_autocheckpoint(sqlite3 *db, पूर्णांक nFrame){
#अगर_घोषित SQLITE_OMIT_WAL
  UNUSED_PARAMETER(db);
  UNUSED_PARAMETER(nFrame);
#अन्यथा
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !sqlite3SafetyCheckOk(db) ) वापस SQLITE_MISUSE_BKPT;
#पूर्ण_अगर
  अगर( nFrame>0 ){
    sqlite3_wal_hook(db, sqlite3WalDefaultHook, SQLITE_INT_TO_PTR(nFrame));
  }अन्यथा{
    sqlite3_wal_hook(db, 0, 0);
  }
#पूर्ण_अगर
  वापस SQLITE_OK;
}

/*
** Register a callback to be invoked each time a transaction is written
** into the write-ahead-log by this database connection.
*/
व्योम *sqlite3_wal_hook(
  sqlite3 *db,                    /* Attach the hook to this db handle */
  पूर्णांक(*xCallback)(व्योम *, sqlite3*, स्थिर अक्षर*, पूर्णांक),
  व्योम *pArg                      /* First argument passed to xCallback() */
){
#अगर_अघोषित SQLITE_OMIT_WAL
  व्योम *pRet;
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !sqlite3SafetyCheckOk(db) ){
    (व्योम)SQLITE_MISUSE_BKPT;
    वापस 0;
  }
#पूर्ण_अगर
  sqlite3_mutex_enter(db->mutex);
  pRet = db->pWalArg;
  db->xWalCallback = xCallback;
  db->pWalArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  वापस pRet;
#अन्यथा
  UNUSED_PARAMETER(db);
  UNUSED_PARAMETER(xCallback);
  UNUSED_PARAMETER(pArg);
  वापस 0;
#पूर्ण_अगर
}

/*
** Checkpoint database zDb.
*/
पूर्णांक sqlite3_wal_checkpoint_v2(
  sqlite3 *db,                    /* Database handle */
  स्थिर अक्षर *zDb,                /* Name of attached database (or NULL) */
  पूर्णांक eMode,                      /* SQLITE_CHECKPOINT_* value */
  पूर्णांक *pnLog,                     /* OUT: Size of WAL log in frames */
  पूर्णांक *pnCkpt                     /* OUT: Total number of frames checkpointed */
){
#अगर_घोषित SQLITE_OMIT_WAL
  UNUSED_PARAMETER(db);
  UNUSED_PARAMETER(zDb);
  UNUSED_PARAMETER(eMode);
  UNUSED_PARAMETER(pnLog);
  UNUSED_PARAMETER(pnCkpt);
  वापस SQLITE_OK;
#अन्यथा
  पूर्णांक rc;                         /* Return code */
  पूर्णांक iDb;                        /* Schema to checkpoint */

#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !sqlite3SafetyCheckOk(db) ) वापस SQLITE_MISUSE_BKPT;
#पूर्ण_अगर

  /* Initialize the output variables to -1 in case an error occurs. */
  अगर( pnLog ) *pnLog = -1;
  अगर( pnCkpt ) *pnCkpt = -1;

  निश्चित( SQLITE_CHECKPOINT_NOOP==-1 );
  निश्चित( SQLITE_CHECKPOINT_PASSIVE==0 );
  निश्चित( SQLITE_CHECKPOINT_FULL==1 );
  निश्चित( SQLITE_CHECKPOINT_RESTART==2 );
  निश्चित( SQLITE_CHECKPOINT_TRUNCATE==3 );
  अगर( eMode<SQLITE_CHECKPOINT_NOOP || eMode>SQLITE_CHECKPOINT_TRUNCATE ){
    /* EVIDENCE-OF: R-03996-12088 The M parameter must be a valid checkpoint
    ** mode: */
    वापस SQLITE_MISUSE_BKPT;
  }

  sqlite3_mutex_enter(db->mutex);
  अगर( zDb && zDb[0] ){
    iDb = sqlite3FindDbName(db, zDb);
  }अन्यथा{
    iDb = SQLITE_MAX_DB;   /* This means process all schemas */
  }
  अगर( iDb<0 ){
    rc = SQLITE_ERROR;
    sqlite3_Error_With_संदेश(db, SQLITE_ERROR, "unknown database: %s", zDb);
  }अन्यथा{
    db->busyHandler.nBusy = 0;
    rc = sqlite3Checkpoint(db, iDb, eMode, pnLog, pnCkpt);
    sqlite3Error(db, rc);
  }
  rc = sqlite3ApiExit(db, rc);

  /* If there are no active statements, clear the interrupt flag at this
  ** point.  */
  अगर( db->nVdbeActive==0 ){
    AtomicStore(&db->u1.isInterrupted, 0);
  }

  sqlite3_mutex_leave(db->mutex);
  वापस rc;
#पूर्ण_अगर
}


/*
** Checkpoint database zDb. If zDb is NULL, or if the buffer zDb points
** to contains a zero-length string, all attached databases are
** checkpointed.
*/
पूर्णांक sqlite3_wal_checkpoint(sqlite3 *db, स्थिर अक्षर *zDb){
  /* EVIDENCE-OF: R-41613-20553 The sqlite3_wal_checkpoint(D,X) is equivalent to
  ** sqlite3_wal_checkpoint_v2(D,X,SQLITE_CHECKPOINT_PASSIVE,0,0). */
  वापस sqlite3_wal_checkpoint_v2(db,zDb,SQLITE_CHECKPOINT_PASSIVE,0,0);
}

#अगर_अघोषित SQLITE_OMIT_WAL
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
पूर्णांक sqlite3Checkpoint(sqlite3 *db, पूर्णांक iDb, पूर्णांक eMode, पूर्णांक *pnLog, पूर्णांक *pnCkpt){
  पूर्णांक rc = SQLITE_OK;             /* Return code */
  पूर्णांक i;                          /* Used to iterate through attached dbs */
  पूर्णांक bBusy = 0;                  /* True if SQLITE_BUSY has been encountered */

  निश्चित( sqlite3_mutex_held(db->mutex) );
  निश्चित( !pnLog || *pnLog==-1 );
  निश्चित( !pnCkpt || *pnCkpt==-1 );
  testcase( iDb==SQLITE_MAX_ATTACHED ); /* See forum post a006d86f72 */
  testcase( iDb==SQLITE_MAX_DB );

  क्रम(i=0; i<db->nDb && rc==SQLITE_OK; i++){
    अगर( i==iDb || iDb==SQLITE_MAX_DB ){
      rc = sqlite3BtreeCheckpoint(db->aDb[i].pBt, eMode, pnLog, pnCkpt);
      pnLog = 0;
      pnCkpt = 0;
      अगर( rc==SQLITE_BUSY ){
        bBusy = 1;
        rc = SQLITE_OK;
      }
    }
  }

  वापस (rc==SQLITE_OK && bBusy) ? SQLITE_BUSY : rc;
}
#पूर्ण_अगर /* SQLITE_OMIT_WAL */

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
पूर्णांक sqlite3_अस्थायी_In_Memory(स्थिर sqlite3 *db){
#अगर SQLITE_अस्थायी_STORE==1
  वापस ( db->अस्थायी_store==2 );
#पूर्ण_अगर
#अगर SQLITE_अस्थायी_STORE==2
  वापस ( db->अस्थायी_store!=1 );
#पूर्ण_अगर
#अगर SQLITE_अस्थायी_STORE==3
  UNUSED_PARAMETER(db);
  वापस 1;
#पूर्ण_अगर
#अगर SQLITE_अस्थायी_STORE<1 || SQLITE_अस्थायी_STORE>3
  UNUSED_PARAMETER(db);
  वापस 0;
#पूर्ण_अगर
}

/*
** Return UTF-8 encoded English language explanation of the most recent
** error.
*/
स्थिर अक्षर *sqlite3_errmsg(sqlite3 *db){
  स्थिर अक्षर *z;
  अगर( !db ){
    वापस sqlite3ErrStr(SQLITE_NOMEM_BKPT);
  }
  अगर( !sqlite3SafetyCheckSickOrOk(db) ){
    वापस sqlite3ErrStr(SQLITE_MISUSE_BKPT);
  }
  sqlite3_mutex_enter(db->mutex);
  अगर( db->mallocFailed ){
    z = sqlite3ErrStr(SQLITE_NOMEM_BKPT);
  }अन्यथा{
    testcase( db->pErr==0 );
    z = db->errCode ? (अक्षर*)sqlite3_मूल्य_text(db->pErr) : 0;
    निश्चित( !db->mallocFailed );
    अगर( z==0 ){
      z = sqlite3ErrStr(db->errCode);
    }
  }
  sqlite3_mutex_leave(db->mutex);
  वापस z;
}

/*
** Set the error code and error message associated with the database handle.
**
** This routine is intended to be called by outside extensions (ex: the
** Session extension). Internal logic should invoke sqlite3Error() or
** sqlite3ErrorWithMsg() directly.
*/
पूर्णांक sqlite3_set_errmsg(sqlite3 *db, पूर्णांक errcode, स्थिर अक्षर *z_संदेश){
  पूर्णांक rc = SQLITE_OK;
  अगर( !sqlite3SafetyCheckOk(db) ){
    वापस SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(db->mutex);
  अगर( z_संदेश ){
    sqlite3_Error_With_संदेश(db, errcode, "%s", z_संदेश);
  }अन्यथा{
    sqlite3Error(db, errcode);
  }
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  वापस rc;
}

/*
** Return the byte offset of the most recent error
*/
पूर्णांक sqlite3_error_offset(sqlite3 *db){
  पूर्णांक iOffset = -1;
  अगर( db && sqlite3SafetyCheckSickOrOk(db) ){
    sqlite3_mutex_enter(db->mutex);
    अगर( db->errCode ){
      iOffset = db->errByteOffset;
    }
    sqlite3_mutex_leave(db->mutex);
  }
  वापस iOffset;
}

#अगर_अघोषित SQLITE_OMIT_UTF16
/*
** Return UTF-16 encoded English language explanation of the most recent
** error.
*/
स्थिर व्योम *sqlite3_errmsg16(sqlite3 *db){
  अटल स्थिर u16 outOfMem[] = {
    'o', 'u', 't', ' ', 'o', 'f', ' ', 'm', 'e', 'm', 'o', 'r', 'y', 0
  };
  अटल स्थिर u16 misuse[] = {
    'b', 'a', 'd', ' ', 'p', 'a', 'r', 'a', 'm', 'e', 't', 'e', 'r', ' ',
    'o', 'r', ' ', 'o', 't', 'h', 'e', 'r', ' ', 'A', 'P', 'I', ' ',
    'm', 'i', 's', 'u', 's', 'e', 0
  };

  स्थिर व्योम *z;
  अगर( !db ){
    वापस (व्योम *)outOfMem;
  }
  अगर( !sqlite3SafetyCheckSickOrOk(db) ){
    वापस (व्योम *)misuse;
  }
  sqlite3_mutex_enter(db->mutex);
  अगर( db->mallocFailed ){
    z = (व्योम *)outOfMem;
  }अन्यथा{
    z = sqlite3_मूल्य_text16(db->pErr);
    अगर( z==0 ){
      sqlite3_Error_With_संदेश(db, db->errCode, sqlite3ErrStr(db->errCode));
      z = sqlite3_मूल्य_text16(db->pErr);
    }
    /* A malloc() may have failed within the call to sqlite3_value_text16()
    ** above. If this is the case, then the db->mallocFailed flag needs to
    ** be cleared before returning. Do this directly, instead of via
    ** sqlite3ApiExit(), to avoid setting the database handle error message.
    */
    sqlite3OomClear(db);
  }
  sqlite3_mutex_leave(db->mutex);
  वापस z;
}
#पूर्ण_अगर /* SQLITE_OMIT_UTF16 */

/*
** Return the most recent error code generated by an SQLite routine. If NULL is
** passed to this function, we assume a malloc() failed during sqlite3_open().
*/
पूर्णांक sqlite3_errcode(sqlite3 *db){
  पूर्णांक iRet;
  अगर( !db ) वापस SQLITE_NOMEM_BKPT;
  अगर( !sqlite3SafetyCheckSickOrOk(db) ){
    वापस SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(db->mutex);
  अगर( db->mallocFailed ){
    iRet = SQLITE_NOMEM_BKPT;
  }अन्यथा{
    iRet = db->errCode & db->errMask;
  }
  sqlite3_mutex_leave(db->mutex);
  वापस iRet;
}
पूर्णांक sqlite3_extended_errcode(sqlite3 *db){
  पूर्णांक iRet;
  अगर( !db ) वापस SQLITE_NOMEM_BKPT;
  अगर( !sqlite3SafetyCheckSickOrOk(db) ){
    वापस SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(db->mutex);
  अगर( db->mallocFailed ){
    iRet = SQLITE_NOMEM_BKPT;
  }अन्यथा{
    iRet = db->errCode;
  }
  sqlite3_mutex_leave(db->mutex);
  वापस iRet;
}
पूर्णांक sqlite3_system_errno(sqlite3 *db){
  पूर्णांक iRet = 0;
  अगर( db ){
    sqlite3_mutex_enter(db->mutex);
    iRet = db->iSysErrno;
    sqlite3_mutex_leave(db->mutex);
  }
  वापस iRet;
} 

/*
** Return a string that describes the kind of error specified in the
** argument.  For now, this simply calls the internal sqlite3ErrStr()
** function.
*/
स्थिर अक्षर *sqlite3_errstr(पूर्णांक rc){
  वापस sqlite3ErrStr(rc);
}

/*
** Create a new collating function for database "db".  The name is zName
** and the encoding is enc.
*/
अटल पूर्णांक createCollation(
  sqlite3* db,
  स्थिर अक्षर *zName,
  u8 enc,
  व्योम* pCtx,
  पूर्णांक(*xCompare)(व्योम*,पूर्णांक,स्थिर व्योम*,पूर्णांक,स्थिर व्योम*),
  व्योम(*xDel)(व्योम*)
){
  CollSeq *pColl;
  पूर्णांक enc2;
 
  निश्चित( sqlite3_mutex_held(db->mutex) );

  /* If SQLITE_UTF16 is specified as the encoding type, transform this
  ** to one of SQLITE_UTF16LE or SQLITE_UTF16BE using the
  ** SQLITE_UTF16NATIVE macro. SQLITE_UTF16 is not used internally.
  */
  enc2 = enc;
  testcase( enc2==SQLITE_UTF16 );
  testcase( enc2==SQLITE_UTF16_ALIGNED );
  अगर( enc2==SQLITE_UTF16 || enc2==SQLITE_UTF16_ALIGNED ){
    enc2 = SQLITE_UTF16NATIVE;
  }
  अगर( enc2<SQLITE_UTF8 || enc2>SQLITE_UTF16BE ){
    वापस SQLITE_MISUSE_BKPT;
  }

  /* Check if this call is removing or replacing an existing collation
  ** sequence. If so, and there are active VMs, return busy. If there
  ** are no active VMs, invalidate any pre-compiled statements.
  */
  pColl = sqlite3FindCollSeq(db, (u8)enc2, zName, 0);
  अगर( pColl && pColl->xCmp ){
    अगर( db->nVdbeActive ){
      sqlite3_Error_With_संदेश(db, SQLITE_BUSY,
        "unable to delete/modify collation sequence due to active statements");
      वापस SQLITE_BUSY;
    }
    sqlite3ExpirePreparedStatements(db, 0);

    /* If collation sequence pColl was created directly by a call to
    ** sqlite3_create_collation, and not generated by synthCollSeq(),
    ** then any copies made by synthCollSeq() need to be invalidated.
    ** Also, collation destructor - CollSeq.xDel() - function may need
    ** to be called.
    */
    अगर( (pColl->enc & ~SQLITE_UTF16_ALIGNED)==enc2 ){
      CollSeq *aColl = sqlite3HashFind(&db->aCollSeq, zName);
      पूर्णांक j;
      क्रम(j=0; j<3; j++){
        CollSeq *p = &aColl[j];
        अगर( p->enc==pColl->enc ){
          अगर( p->xDel ){
            p->xDel(p->pUser);
          }
          p->xCmp = 0;
        }
      }
    }
  }

  pColl = sqlite3FindCollSeq(db, (u8)enc2, zName, 1);
  अगर( pColl==0 ) वापस SQLITE_NOMEM_BKPT;
  pColl->xCmp = xCompare;
  pColl->pUser = pCtx;
  pColl->xDel = xDel;
  pColl->enc = (u8)(enc2 | (enc & SQLITE_UTF16_ALIGNED));
  sqlite3Error(db, SQLITE_OK);
  वापस SQLITE_OK;
}


/*
** This array defines hard upper bounds on limit values.  The
** initializer must be kept in sync with the SQLITE_LIMIT_*
** #defines in sqlite3.h.
*/
अटल स्थिर पूर्णांक aHardLimit[] = {
  SQLITE_MAX_लंबाई,
  SQLITE_MAX_SQL_लंबाई,
  SQLITE_MAX_COLUMN,
  SQLITE_MAX_व्यंजक_DEPTH,
  SQLITE_MAX_COMPOUND_SELECT,
  SQLITE_MAX_VDBE_OP,
  SQLITE_MAX_FUNCTION_ARG,
  SQLITE_MAX_ATTACHED,
  SQLITE_MAX_LIKE_PATTERN_लंबाई,
  SQLITE_MAX_VARIABLE_संख्या,      /* IMP: R-38091-32352 */
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
#अगर SQLITE_MAX_लंबाई<100
# error SQLITE_MAX_लंबाई must be at least 100
#पूर्ण_अगर
#अगर SQLITE_MAX_SQL_लंबाई<100
# error SQLITE_MAX_SQL_लंबाई must be at least 100
#पूर्ण_अगर
#अगर SQLITE_MAX_SQL_लंबाई>SQLITE_MAX_लंबाई
# error SQLITE_MAX_SQL_लंबाई must not be greater than SQLITE_MAX_लंबाई
#पूर्ण_अगर
#अगर SQLITE_MAX_SQL_लंबाई>2147482624     /* 1024 less than 2^31 */
# error SQLITE_MAX_SQL_लंबाई must not be greater than 2147482624
#पूर्ण_अगर
#अगर SQLITE_MAX_COMPOUND_SELECT<2
# error SQLITE_MAX_COMPOUND_SELECT must be at least 2
#पूर्ण_अगर
#अगर SQLITE_MAX_VDBE_OP<40
# error SQLITE_MAX_VDBE_OP must be at least 40
#पूर्ण_अगर
#अगर SQLITE_MAX_FUNCTION_ARG<0 || SQLITE_MAX_FUNCTION_ARG>32767
# error SQLITE_MAX_FUNCTION_ARG must be between 0 and 32767
#पूर्ण_अगर
#अगर SQLITE_MAX_ATTACHED<0 || SQLITE_MAX_ATTACHED>125
# error SQLITE_MAX_ATTACHED must be between 0 and 125
#पूर्ण_अगर
#अगर SQLITE_MAX_LIKE_PATTERN_लंबाई<1
# error SQLITE_MAX_LIKE_PATTERN_लंबाई must be at least 1
#पूर्ण_अगर
#अगर SQLITE_MAX_COLUMN>32767
# error SQLITE_MAX_COLUMN must not exceed 32767
#पूर्ण_अगर
#अगर SQLITE_MAX_TRIGGER_DEPTH<1
# error SQLITE_MAX_TRIGGER_DEPTH must be at least 1
#पूर्ण_अगर
#अगर SQLITE_MAX_WORKER_THREADS<0 || SQLITE_MAX_WORKER_THREADS>50
# error SQLITE_MAX_WORKER_THREADS must be between 0 and 50
#पूर्ण_अगर
#अगर SQLITE_MAX_TRIGGER_STEPS<0 || SQLITE_MAX_TRIGGER_STEPS>65535
# error SQLITE_MAX_TRIGGER_STEPS must be between 0 and 65535
#पूर्ण_अगर


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
पूर्णांक sqlite3_limit(sqlite3 *db, पूर्णांक limitId, पूर्णांक newLimit){
  पूर्णांक oldLimit;

#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !sqlite3SafetyCheckOk(db) ){
    (व्योम)SQLITE_MISUSE_BKPT;
    वापस -1;
  }
#पूर्ण_अगर

  /* EVIDENCE-OF: R-30189-54097 For each limit category SQLITE_LIMIT_NAME
  ** there is a hard upper bound set at compile-time by a C preprocessor
  ** macro called SQLITE_MAX_NAME. (The "_LIMIT_" in the name is changed to
  ** "_MAX_".)
  */
  निश्चित( aHardLimit[SQLITE_LIMIT_लंबाई]==SQLITE_MAX_लंबाई );
  निश्चित( aHardLimit[SQLITE_LIMIT_SQL_लंबाई]==SQLITE_MAX_SQL_लंबाई );
  निश्चित( aHardLimit[SQLITE_LIMIT_COLUMN]==SQLITE_MAX_COLUMN );
  निश्चित( aHardLimit[SQLITE_LIMIT_व्यंजक_DEPTH]==SQLITE_MAX_व्यंजक_DEPTH );
  निश्चित( aHardLimit[SQLITE_LIMIT_PARSER_DEPTH]==SQLITE_MAX_PARSER_DEPTH );
  निश्चित( aHardLimit[SQLITE_LIMIT_COMPOUND_SELECT]==SQLITE_MAX_COMPOUND_SELECT);
  निश्चित( aHardLimit[SQLITE_LIMIT_VDBE_OP]==SQLITE_MAX_VDBE_OP );
  निश्चित( aHardLimit[SQLITE_LIMIT_FUNCTION_ARG]==SQLITE_MAX_FUNCTION_ARG );
  निश्चित( aHardLimit[SQLITE_LIMIT_ATTACHED]==SQLITE_MAX_ATTACHED );
  निश्चित( aHardLimit[SQLITE_LIMIT_LIKE_PATTERN_लंबाई]==
                                               SQLITE_MAX_LIKE_PATTERN_लंबाई );
  निश्चित( aHardLimit[SQLITE_LIMIT_VARIABLE_संख्या]==SQLITE_MAX_VARIABLE_संख्या);
  निश्चित( aHardLimit[SQLITE_LIMIT_TRIGGER_DEPTH]==SQLITE_MAX_TRIGGER_DEPTH );
  निश्चित( aHardLimit[SQLITE_LIMIT_WORKER_THREADS]==SQLITE_MAX_WORKER_THREADS );
  निश्चित( aHardLimit[SQLITE_LIMIT_SCHEMA]==SQLITE_MAX_SCHEMA );
  निश्चित( aHardLimit[SQLITE_LIMIT_TRIGGER_STEPS]==SQLITE_MAX_TRIGGER_STEPS );
  निश्चित( aHardLimit[SQLITE_LIMIT_CTES]==SQLITE_MAX_CTES );
  निश्चित( SQLITE_LIMIT_CTES==(SQLITE_N_LIMIT-1) );


  अगर( limitId<0 || limitId>=SQLITE_N_LIMIT ){
    वापस -1;
  }
  sqlite3_mutex_enter(db->mutex);
  oldLimit = db->aLimit[limitId];
  अगर( newLimit>=0 ){                   /* IMP: R-52476-28732 */
    अगर( newLimit>aHardLimit[limitId] ){
      newLimit = aHardLimit[limitId];  /* IMP: R-51463-25634 */
    }अन्यथा अगर( newLimit<SQLITE_MIN_लंबाई && limitId==SQLITE_LIMIT_लंबाई ){
      newLimit = SQLITE_MIN_लंबाई;
    }
    db->aLimit[limitId] = newLimit;
  }
  sqlite3_mutex_leave(db->mutex);
  वापस oldLimit;                     /* IMP: R-53341-35419 */
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
पूर्णांक sqlite3ParseUri(
  स्थिर अक्षर *zDefaultVfs,        /* VFS to use if no "vfs=xxx" query option */
  स्थिर अक्षर *zUri,               /* Nul-terminated URI to parse */
  अचिन्हित पूर्णांक *pFlags,           /* IN/OUT: SQLITE_OPEN_XXX flags */
  sqlite3_vfs **ppVfs,            /* OUT: VFS to use */
  अक्षर **pzFile,                  /* OUT: Filename component of URI */
  अक्षर **pz_Err_संदेश                 /* OUT: Error message (if rc!=SQLITE_OK) */
){
  पूर्णांक rc = SQLITE_OK;
  अचिन्हित पूर्णांक flags = *pFlags;
  स्थिर अक्षर *zVfs = zDefaultVfs;
  अक्षर *zFile;
  अक्षर c;
  i64 nUri = म_माप(zUri);

  निश्चित( *pz_Err_संदेश==0 );

  अगर( ((flags & SQLITE_OPEN_URI)                     /* IMP: R-48725-32206 */
       || AtomicLoad(&sqlite3GlobalConfig.bOpenUri)) /* IMP: R-51689-46548 */
   && nUri>=5 && स_भेद(zUri, "file:", 5)==0         /* IMP: R-57884-37496 */
  ){
    अक्षर *zOpt;
    पूर्णांक eState;                   /* Parser state when parsing URI */
    i64 iIn;                      /* Input character index */
    i64 iOut = 0;                 /* Output character index */
    u64 nByte = nUri+8;           /* Bytes of space to allocate */

    /* Make sure the SQLITE_OPEN_URI flag is set to indicate to the VFS xOpen
    ** method that there may be extra parameters following the file-name.  */
    flags |= SQLITE_OPEN_URI;

    क्रम(iIn=0; iIn<nUri; iIn++) nByte += (zUri[iIn]=='&');
    zFile = sqlite3_malloc64(nByte);
    अगर( !zFile ) वापस SQLITE_NOMEM_BKPT;

    स_रखो(zFile, 0, 4);  /* 4-byte of 0x00 is the start of DB name marker */
    zFile += 4;

    iIn = 5;
#अगर_घोषित SQLITE_ALLOW_URI_AUTHORITY
    अगर( म_भेदन(zUri+5, "///", 3)==0 ){
      iIn = 7;
      /* The following condition causes URIs with five leading / characters
      ** like file://///host/path to be converted into UNCs like //host/path.
      ** The correct URI for that UNC has only two or four leading / characters
      ** file://host/path or file:////host/path.  But 5 leading slashes is a
      ** common error, we are told, so we handle it as a special case. */
      अगर( म_भेदन(zUri+7, "///", 3)==0 ){ iIn++; }
    }अन्यथा अगर( म_भेदन(zUri+5, "//localhost/", 12)==0 ){
      iIn = 16;
    }
#अन्यथा
    /* Discard the scheme and authority segments of the URI. */
    अगर( zUri[5]=='/' && zUri[6]=='/' ){
      iIn = 7;
      जबतक( zUri[iIn] && zUri[iIn]!='/' ) iIn++;
      अगर( iIn!=7 && (iIn!=16 || स_भेद("localhost", &zUri[7], 9)) ){
        *pz_Err_संदेश = sqlite3_mprintf("invalid uri authority: %.*s",
            (पूर्णांक)(iIn-7), &zUri[7]);
        rc = SQLITE_ERROR;
        जाओ parse_uri_out;
      }
    }
#पूर्ण_अगर

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
    जबतक( (c = zUri[iIn])!=0 && c!='#' ){
      iIn++;
      अगर( c=='%'
       && sqlite3Isxdigit(zUri[iIn])
       && sqlite3Isxdigit(zUri[iIn+1])
      ){
        पूर्णांक octet = (sqlite3HexToInt(zUri[iIn++]) << 4);
        octet += sqlite3HexToInt(zUri[iIn++]);

        निश्चित( octet>=0 && octet<256 );
        अगर( octet==0 ){
#अगर_अघोषित SQLITE_ENABLE_URI_00_ERROR
          /* This branch is taken when "%00" appears within the URI. In this
          ** case we ignore all text in the remainder of the path, name or
          ** value currently being parsed. So ignore the current character
          ** and skip to the next "?", "=" or "&", as appropriate. */
          जबतक( (c = zUri[iIn])!=0 && c!='#'
              && (eState!=0 || c!='?')
              && (eState!=1 || (c!='=' && c!='&'))
              && (eState!=2 || c!='&')
          ){
            iIn++;
          }
          जारी;
#अन्यथा
          /* If ENABLE_URI_00_ERROR is defined, "%00" in a URI is an error. */
          *pz_Err_संदेश = sqlite3_mprintf("unexpected %%00 in uri");
          rc = SQLITE_ERROR;
          जाओ parse_uri_out;
#पूर्ण_अगर
        }
        c = octet;
      }अन्यथा अगर( eState==1 && (c=='&' || c=='=') ){
        अगर( zFile[iOut-1]==0 ){
          /* An empty option name. Ignore this option altogether. */
          जबतक( zUri[iIn] && zUri[iIn]!='#' && zUri[iIn-1]!='&' ) iIn++;
          जारी;
        }
        अगर( c=='&' ){
          zFile[iOut++] = '\0';
        }अन्यथा{
          eState = 2;
        }
        c = 0;
      }अन्यथा अगर( (eState==0 && c=='?') || (eState==2 && c=='&') ){
        c = 0;
        eState = 1;
      }
      zFile[iOut++] = c;
    }
    अगर( eState==1 ) zFile[iOut++] = '\0';
    स_रखो(zFile+iOut, 0, 4); /* end-of-options + empty journal filenames */

    /* Check if there were any options specified that should be interpreted
    ** here. Options that are interpreted here include "vfs" and those that
    ** correspond to flags that may be passed to the sqlite3_open_v2()
    ** method. */
    zOpt = &zFile[म_माप(zFile)+1];
    जबतक( zOpt[0] ){
      i64 nOpt = म_माप(zOpt);
      अक्षर *zVal = &zOpt[nOpt+1];
      i64 nVal = म_माप(zVal);

      अगर( nOpt==3 && स_भेद("vfs", zOpt, 3)==0 ){
        zVfs = zVal;
      }अन्यथा{
        काष्ठा OpenMode {
          स्थिर अक्षर *z;
          पूर्णांक mode;
        } *aMode = 0;
        अक्षर *zModeType = 0;
        पूर्णांक mask = 0;
        पूर्णांक limit = 0;

        अगर( nOpt==5 && स_भेद("cache", zOpt, 5)==0 ){
          अटल काष्ठा OpenMode aCacheMode[] = {
            { "shared",  SQLITE_OPEN_SHAREDCACHE },
            { "private", SQLITE_OPEN_PRIVATECACHE },
            { 0, 0 }
          };

          mask = SQLITE_OPEN_SHAREDCACHE|SQLITE_OPEN_PRIVATECACHE;
          aMode = aCacheMode;
          limit = mask;
          zModeType = "cache";
        }
        अगर( nOpt==4 && स_भेद("mode", zOpt, 4)==0 ){
          अटल काष्ठा OpenMode aOpenMode[] = {
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

        अगर( aMode ){
          पूर्णांक i;
          पूर्णांक mode = 0;
          क्रम(i=0; aMode[i].z; i++){
            स्थिर अक्षर *z = aMode[i].z;
            अगर( nVal==(i64)म_माप(z) && 0==स_भेद(zVal, z, nVal) ){
              mode = aMode[i].mode;
              अवरोध;
            }
          }
          अगर( mode==0 ){
            *pz_Err_संदेश = sqlite3_mprintf("no such %s mode: %s", zModeType, zVal);
            rc = SQLITE_ERROR;
            जाओ parse_uri_out;
          }
          अगर( (mode & ~SQLITE_OPEN_MEMORY)>limit ){
            *pz_Err_संदेश = sqlite3_mprintf("%s mode not allowed: %s",
                                        zModeType, zVal);
            rc = SQLITE_PERM;
            जाओ parse_uri_out;
          }
          flags = (flags & ~mask) | mode;
        }
      }

      zOpt = &zVal[nVal+1];
    }

  }अन्यथा{
    zFile = sqlite3_malloc64(nUri+8);
    अगर( !zFile ) वापस SQLITE_NOMEM_BKPT;
    स_रखो(zFile, 0, 4);
    zFile += 4;
    अगर( nUri ){
      स_नकल(zFile, zUri, nUri);
    }
    स_रखो(zFile+nUri, 0, 4);
    flags &= ~SQLITE_OPEN_URI;
  }

  *ppVfs = sqlite3_vfs_find(zVfs);
  अगर( *ppVfs==0 ){
    *pz_Err_संदेश = sqlite3_mprintf("no such vfs: %s", zVfs);
    rc = SQLITE_ERROR;
  }
 parse_uri_out:
  अगर( rc!=SQLITE_OK ){
    sqlite3_free_filename(zFile);
    zFile = 0;
  }
  *pFlags = flags;
  *pzFile = zFile;
  वापस rc;
}

/*
** This routine does the core work of extracting URI parameters from a
** database filename for the sqlite3_uri_parameter() interface.
*/
अटल स्थिर अक्षर *uriParameter(स्थिर अक्षर *zFilename, स्थिर अक्षर *zParam){
  zFilename += sqlite3Strlen30(zFilename) + 1;
  जबतक( ALWAYS(zFilename!=0) && zFilename[0] ){
    पूर्णांक x = म_भेद(zFilename, zParam);
    zFilename += sqlite3Strlen30(zFilename) + 1;
    अगर( x==0 ) वापस zFilename;
    zFilename += sqlite3Strlen30(zFilename) + 1;
  }
  वापस 0;
}



/*
** This routine does the work of opening a database on behalf of
** sqlite3_open() and sqlite3_open16(). The database filename "zFilename" 
** is UTF-8 encoded.
*/
अटल पूर्णांक openDatabase(
  स्थिर अक्षर *zFilename, /* Database filename UTF-8 encoded */
  sqlite3 **ppDb,        /* OUT: Returned database handle */
  अचिन्हित पूर्णांक flags,    /* Operational flags */
  स्थिर अक्षर *zVfs       /* Name of the VFS to use */
){
  sqlite3 *db;                    /* Store allocated handle here */
  पूर्णांक rc;                         /* Return code */
  पूर्णांक isThreadsafe;               /* True for threadsafe connections */
  अक्षर *zOpen = 0;                /* Filename argument to pass to BtreeOpen() */
  अक्षर *z_Err_संदेश = 0;              /* Error message from sqlite3ParseUri() */
  पूर्णांक i;                          /* Loop counter */

#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( ppDb==0 ) वापस SQLITE_MISUSE_BKPT;
#पूर्ण_अगर
  *ppDb = 0;
#अगर_अघोषित SQLITE_OMIT_AUTOINIT
  rc = sqlite3_initialize();
  अगर( rc ) वापस rc;
#पूर्ण_अगर

  अगर( sqlite3GlobalConfig.bCoreMutex==0 ){
    isThreadsafe = 0;
  }अन्यथा अगर( flags & SQLITE_OPEN_NOMUTEX ){
    isThreadsafe = 0;
  }अन्यथा अगर( flags & SQLITE_OPEN_FULLMUTEX ){
    isThreadsafe = 1;
  }अन्यथा{
    isThreadsafe = sqlite3GlobalConfig.bFullMutex;
  }

  अगर( flags & SQLITE_OPEN_PRIVATECACHE ){
    flags &= ~SQLITE_OPEN_SHAREDCACHE;
  }अन्यथा अगर( sqlite3GlobalConfig.sharedCacheEnabled ){
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
               SQLITE_OPEN_अस्थायी_DB |
               SQLITE_OPEN_TRANSIENT_DB |
               SQLITE_OPEN_MAIN_JOURNAL |
               SQLITE_OPEN_अस्थायी_JOURNAL |
               SQLITE_OPEN_SUBJOURNAL |
               SQLITE_OPEN_SUPER_JOURNAL |
               SQLITE_OPEN_NOMUTEX |
               SQLITE_OPEN_FULLMUTEX |
               SQLITE_OPEN_WAL
             );

  /* Allocate the sqlite data structure */
  db = sqlite3MallocZero( माप(sqlite3) );
  अगर( db==0 ) जाओ opendb_out;
  अगर( isThreadsafe
#अगर defined(SQLITE_THREAD_MISUSE_WARNINGS)
   || sqlite3GlobalConfig.bCoreMutex
#पूर्ण_अगर
  ){
    db->mutex = sqlite3MutexAlloc(SQLITE_MUTEX_RECURSIVE);
    अगर( db->mutex==0 ){
      sqlite3_free(db);
      db = 0;
      जाओ opendb_out;
    }
    अगर( isThreadsafe==0 ){
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

  निश्चित( माप(db->aLimit)==माप(aHardLimit) );
  स_नकल(db->aLimit, aHardLimit, माप(db->aLimit));
  db->aLimit[SQLITE_LIMIT_WORKER_THREADS] = SQLITE_DEFAULT_WORKER_THREADS;
  db->autoCommit = 1;
  db->अग्र_Autovac = -1;
  db->szMmap = sqlite3GlobalConfig.szMmap;
  db->अग्र_Pagesize = 0;
  db->init.azInit = sqlite3StdType; /* Any array of string ptrs will do */
#अगर_घोषित SQLITE_ENABLE_SORTER_MMAP
  /* Beginning with version 3.37.0, using the VFS xFetch() API to memory-map
  ** the temporary files used to do external sorts (see code in vdbesort.c)
  ** is disabled. It can still be used either by defining
  ** SQLITE_ENABLE_SORTER_MMAP at compile time or by using the
  ** SQLITE_TESTCTRL_SORTER_MMAP test-control at runtime. */
  db->nMaxSorterMmap = 0x7FFFFFFF;
#पूर्ण_अगर
  db->flags |= SQLITE_ShortColNames
                 | SQLITE_EnableTrigger
                 | SQLITE_EnableView
                 | SQLITE_CacheSpill
                 | SQLITE_AttachCreate
                 | SQLITE_AttachWrite
                 | SQLITE_Comments
#अगर !defined(SQLITE_TRUSTED_SCHEMA) || SQLITE_TRUSTED_SCHEMA+0!=0
                 | SQLITE_TrustedSchema
#पूर्ण_अगर
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
#अगर !defined(SQLITE_DQS)
# define SQLITE_DQS 3
#पूर्ण_अगर
#अगर (SQLITE_DQS&1)==1
                 | SQLITE_DqsDML
#पूर्ण_अगर
#अगर (SQLITE_DQS&2)==2
                 | SQLITE_DqsDDL
#पूर्ण_अगर

#अगर !defined(SQLITE_DEFAULT_AUTOMATIC_सूचकांक) || SQLITE_DEFAULT_AUTOMATIC_सूचकांक
                 | SQLITE_Auto_सूचकांक
#पूर्ण_अगर
#अगर SQLITE_DEFAULT_CKPTFULLFSYNC
                 | SQLITE_CkptFullFSync
#पूर्ण_अगर
#अगर SQLITE_DEFAULT_FILE_FORMAT<4
                 | SQLITE_LegacyFileFmt
#पूर्ण_अगर
#अगर_घोषित SQLITE_ENABLE_LOAD_EXTENSION
                 | SQLITE_LoadExtension
#पूर्ण_अगर
#अगर SQLITE_DEFAULT_RECURSIVE_TRIGGERS
                 | SQLITE_RecTriggers
#पूर्ण_अगर
#अगर defined(SQLITE_DEFAULT_FOREIGN_कुंजियाँ) && SQLITE_DEFAULT_FOREIGN_कुंजियाँ
                 | SQLITE_Foreign_कुंजियाँ
#पूर्ण_अगर
#अगर defined(SQLITE_REVERSE_UNORDERED_SELECTS)
                 | SQLITE_ReverseOrder
#पूर्ण_अगर
#अगर defined(SQLITE_ENABLE_OVERSIZE_CELL_CHECK)
                 | SQLITE_Cell_आकार_Ck
#पूर्ण_अगर
#अगर defined(SQLITE_ENABLE_FTS3_TOKENIZER)
                 | SQLITE_Fts3Tokenizer
#पूर्ण_अगर
#अगर defined(SQLITE_ENABLE_QPSG)
                 | SQLITE_EnableQPSG
#पूर्ण_अगर
#अगर defined(SQLITE_DEFAULT_DEFENSIVE)
                 | SQLITE_Defensive
#पूर्ण_अगर
#अगर defined(SQLITE_DEFAULT_LEGACY_ALTER_TABLE)
                 | SQLITE_LegacyAlter
#पूर्ण_अगर
#अगर defined(SQLITE_ENABLE_STMT_SCANSTATUS)
                 | SQLITE_StmtScanStatus
#पूर्ण_अगर
      ;
  sqlite3HashInit(&db->aCollSeq);
#अगर_अघोषित SQLITE_OMIT_VIRTUALTABLE
  sqlite3HashInit(&db->aModule);
#पूर्ण_अगर

  /* Add the default collation sequence BINARY. BINARY works for both UTF-8
  ** and UTF-16, so add a version for each to avoid any unnecessary
  ** conversions. The only error that can occur here is a malloc() failure.
  **
  ** EVIDENCE-OF: R-52786-44878 SQLite defines three built-in collating
  ** functions:
  */
  createCollation(db, sqlite3_Str_द्विआधारी, SQLITE_UTF8, 0,
                  sqlite3_द्विआधारी_Compare, 0);
  createCollation(db, sqlite3_Str_द्विआधारी, SQLITE_UTF16BE, 0,
                  sqlite3_द्विआधारी_Compare, 0);
  createCollation(db, sqlite3_Str_द्विआधारी, SQLITE_UTF16LE, 0,
                  sqlite3_द्विआधारी_Compare, 0);
  createCollation(db, "NOCASE", SQLITE_UTF8, 0, nocaseCollatingFunc, 0);
  createCollation(db, "RTRIM", SQLITE_UTF8, 0, rtrimCollFunc, 0);
  अगर( db->mallocFailed ){
    जाओ opendb_out;
  }

#अगर SQLITE_OS_UNIX && defined(SQLITE_OS_KV_OPTIONAL)
  /* Process magic filenames ":localStorage:" and ":sessionStorage:" */
  अगर( zFilename && zFilename[0]==':' ){
    अगर( म_भेद(zFilename, ":localStorage:")==0 ){
      zFilename = "file:local?vfs=kvvfs";
      flags |= SQLITE_OPEN_URI;
    }अन्यथा अगर( म_भेद(zFilename, ":sessionStorage:")==0 ){
      zFilename = "file:session?vfs=kvvfs";
      flags |= SQLITE_OPEN_URI;
    }
  }
#पूर्ण_अगर /* SQLITE_OS_UNIX && defined(SQLITE_OS_KV_OPTIONAL) */

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
  निश्चित( SQLITE_OPEN_READONLY  == 0x01 );
  निश्चित( SQLITE_OPEN_READWRITE == 0x02 );
  निश्चित( SQLITE_OPEN_CREATE    == 0x04 );
  testcase( (1<<(flags&7))==0x02 ); /* READONLY */
  testcase( (1<<(flags&7))==0x04 ); /* READWRITE */
  testcase( (1<<(flags&7))==0x40 ); /* READWRITE | CREATE */
  अगर( ((1<<(flags&7)) & 0x46)==0 ){
    rc = SQLITE_MISUSE_BKPT;  /* IMP: R-18321-05872 */
  }अन्यथा{
    अगर( zFilename==0 ) zFilename = ":memory:";
    rc = sqlite3ParseUri(zVfs, zFilename, &flags, &db->pVfs, &zOpen, &z_Err_संदेश);
  }
  अगर( rc!=SQLITE_OK ){
    अगर( rc==SQLITE_NOMEM ) sqlite3OomFault(db);
    sqlite3_Error_With_संदेश(db, rc, z_Err_संदेश ? "%s" : 0, z_Err_संदेश);
    sqlite3_free(z_Err_संदेश);
    जाओ opendb_out;
  }
  निश्चित( db->pVfs!=0 );
#अगर SQLITE_OS_KV || defined(SQLITE_OS_KV_OPTIONAL)
  अगर( sqlite3_stricmp(db->pVfs->zName, "kvvfs")==0 ){
    db->अस्थायी_store = 2;
  }
#पूर्ण_अगर

  /* Open the backend database driver */
  rc = sqlite3BtreeOpen(db->pVfs, zOpen, db, &db->aDb[0].pBt, 0,
                        flags | SQLITE_OPEN_MAIN_DB);
  अगर( rc!=SQLITE_OK ){
    अगर( rc==SQLITE_IOERR_NOMEM ){
      rc = SQLITE_NOMEM_BKPT;
    }
    sqlite3Error(db, rc);
    जाओ opendb_out;
  }
  sqlite3BtreeEnter(db->aDb[0].pBt);
  db->aDb[0].pSchema = sqlite3SchemaGet(db, db->aDb[0].pBt);
  अगर( !db->mallocFailed ){
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
  अगर( db->mallocFailed ){
    जाओ opendb_out;
  }

  /* Register all built-in functions, but do not attempt to read the
  ** database schema yet. This is delayed until the first time the database
  ** is accessed.
  */
  sqlite3Error(db, SQLITE_OK);
  sqlite3RegisterPerConnectionBuiltinFunctions(db);
  rc = sqlite3_errcode(db);


  /* Load compiled-in extensions */
  क्रम(i=0; rc==SQLITE_OK && i<Array_आकार(sqlite3BuiltinExtensions); i++){
    rc = sqlite3BuiltinExtensions[i](db);
  }

  /* Load automatic extensions - extensions that have been registered
  ** using the sqlite3_automatic_extension() API.
  */
  अगर( rc==SQLITE_OK ){
    sqlite3AutoLoadExtensions(db);
    rc = sqlite3_errcode(db);
    अगर( rc!=SQLITE_OK ){
      जाओ opendb_out;
    }
  }

#अगर_घोषित SQLITE_ENABLE_INTERNAL_FUNCTIONS
  /* Testing use only!!! The -DSQLITE_ENABLE_INTERNAL_FUNCTIONS=1 compile-time
  ** option gives access to internal functions by default. 
  ** Testing use only!!! */
  db->mDbFlags |= DBFLAG_InternalFunc;
#पूर्ण_अगर

  /* -DSQLITE_DEFAULT_LOCKING_MODE=1 makes EXCLUSIVE the default locking
  ** mode.  -DSQLITE_DEFAULT_LOCKING_MODE=0 make NORMAL the default locking
  ** mode.  Doing nothing at all also makes NORMAL the default.
  */
#अगर_घोषित SQLITE_DEFAULT_LOCKING_MODE
  db->dfltLockMode = SQLITE_DEFAULT_LOCKING_MODE;
  sqlite3PagerLockingMode(sqlite3BtreePager(db->aDb[0].pBt),
                          SQLITE_DEFAULT_LOCKING_MODE);
#पूर्ण_अगर

  अगर( rc ) sqlite3Error(db, rc);

  /* Enable the lookaside-malloc subsystem */
  setupLookaside(db, 0, sqlite3GlobalConfig.szLookaside,
                        sqlite3GlobalConfig.nLookaside);

  sqlite3_wal_autocheckpoint(db, SQLITE_DEFAULT_WAL_AUTOCHECKPOINT);

opendb_out:
  अगर( db ){
    निश्चित( db->mutex!=0 || isThreadsafe==0
           || sqlite3GlobalConfig.bFullMutex==0 );
    sqlite3_mutex_leave(db->mutex);
  }
  rc = sqlite3_errcode(db);
  निश्चित( db!=0 || (rc&0xff)==SQLITE_NOMEM );
  अगर( (rc&0xff)==SQLITE_NOMEM ){
    sqlite3_close(db);
    db = 0;
  }अन्यथा अगर( rc!=SQLITE_OK ){
    db->eOpenState = SQLITE_STATE_SICK;
  }
  *ppDb = db;
#अगर_घोषित SQLITE_ENABLE_SQLLOG
  अगर( sqlite3GlobalConfig.xSqllog ){
    /* Opening a db handle. Fourth parameter is passed 0. */
    व्योम *pArg = sqlite3GlobalConfig.pSqllogArg;
    sqlite3GlobalConfig.xSqllog(pArg, db, zFilename, 0);
  }
#पूर्ण_अगर
  sqlite3_free_filename(zOpen);
  वापस rc;
}


/*
** Open a new database handle.
*/
पूर्णांक sqlite3_open(
  स्थिर अक्षर *zFilename,
  sqlite3 **ppDb
){
  वापस openDatabase(zFilename, ppDb,
                      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, 0);
}
पूर्णांक sqlite3_open_v2(
  स्थिर अक्षर *filename,   /* Database filename (UTF-8) */
  sqlite3 **ppDb,         /* OUT: SQLite db handle */
  पूर्णांक flags,              /* Flags */
  स्थिर अक्षर *zVfs        /* Name of VFS module to use */
){
  वापस openDatabase(filename, ppDb, (अचिन्हित पूर्णांक)flags, zVfs);
}

#अगर_अघोषित SQLITE_OMIT_UTF16
/*
** Open a new database handle.
*/
पूर्णांक sqlite3_open16(
  स्थिर व्योम *zFilename,
  sqlite3 **ppDb
){
  अक्षर स्थिर *zFilename8;   /* zFilename encoded in UTF-8 instead of UTF-16 */
  sqlite3_मूल्य *pVal;
  पूर्णांक rc;

#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( ppDb==0 ) वापस SQLITE_MISUSE_BKPT;
#पूर्ण_अगर
  *ppDb = 0;
#अगर_अघोषित SQLITE_OMIT_AUTOINIT
  rc = sqlite3_initialize();
  अगर( rc ) वापस rc;
#पूर्ण_अगर
  अगर( zFilename==0 ) zFilename = "\000\000";
  pVal = sqlite3_मूल्य_New(0);
  sqlite3_मूल्य_Set_Str(pVal, -1, zFilename, SQLITE_UTF16NATIVE, SQLITE_STATIC);
  zFilename8 = sqlite3_मूल्य_Text(pVal, SQLITE_UTF8);
  अगर( zFilename8 ){
    rc = openDatabase(zFilename8, ppDb,
                      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, 0);
    निश्चित( *ppDb || rc==SQLITE_NOMEM );
    अगर( rc==SQLITE_OK && !DbHasProperty(*ppDb, 0, DB_SchemaLoaded) ){
      SCHEMA_ENC(*ppDb) = ENC(*ppDb) = SQLITE_UTF16NATIVE;
    }
  }अन्यथा{
    rc = SQLITE_NOMEM_BKPT;
  }
  sqlite3_मूल्य_Free(pVal);

  वापस rc & 0xff;
}
#पूर्ण_अगर /* SQLITE_OMIT_UTF16 */

/*
** Register a new collation sequence with the database handle db.
*/
पूर्णांक sqlite3_create_collation(
  sqlite3* db,
  स्थिर अक्षर *zName,
  पूर्णांक enc,
  व्योम* pCtx,
  पूर्णांक(*xCompare)(व्योम*,पूर्णांक,स्थिर व्योम*,पूर्णांक,स्थिर व्योम*)
){
  वापस sqlite3_create_collation_v2(db, zName, enc, pCtx, xCompare, 0);
}

/*
** Register a new collation sequence with the database handle db.
*/
पूर्णांक sqlite3_create_collation_v2(
  sqlite3* db,
  स्थिर अक्षर *zName,
  पूर्णांक enc,
  व्योम* pCtx,
  पूर्णांक(*xCompare)(व्योम*,पूर्णांक,स्थिर व्योम*,पूर्णांक,स्थिर व्योम*),
  व्योम(*xDel)(व्योम*)
){
  पूर्णांक rc;

#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !sqlite3SafetyCheckOk(db) || zName==0 ) वापस SQLITE_MISUSE_BKPT;
#पूर्ण_अगर
  sqlite3_mutex_enter(db->mutex);
  निश्चित( !db->mallocFailed );
  rc = createCollation(db, zName, (u8)enc, pCtx, xCompare, xDel);
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  वापस rc;
}

#अगर_अघोषित SQLITE_OMIT_UTF16
/*
** Register a new collation sequence with the database handle db.
*/
पूर्णांक sqlite3_create_collation16(
  sqlite3* db,
  स्थिर व्योम *zName,
  पूर्णांक enc,
  व्योम* pCtx,
  पूर्णांक(*xCompare)(व्योम*,पूर्णांक,स्थिर व्योम*,पूर्णांक,स्थिर व्योम*)
){
  पूर्णांक rc = SQLITE_OK;
  अक्षर *zName8;

#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !sqlite3SafetyCheckOk(db) || zName==0 ) वापस SQLITE_MISUSE_BKPT;
#पूर्ण_अगर
  sqlite3_mutex_enter(db->mutex);
  निश्चित( !db->mallocFailed );
  zName8 = sqlite3Utf16to8(db, zName, -1, SQLITE_UTF16NATIVE);
  अगर( zName8 ){
    rc = createCollation(db, zName8, (u8)enc, pCtx, xCompare, 0);
    sqlite3DbFree(db, zName8);
  }
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  वापस rc;
}
#पूर्ण_अगर /* SQLITE_OMIT_UTF16 */

/*
** Register a collation sequence factory callback with the database handle
** db. Replace any previously installed collation sequence factory.
*/
पूर्णांक sqlite3_collation_needed(
  sqlite3 *db,
  व्योम *pCollNeededArg,
  व्योम(*xCollNeeded)(व्योम*,sqlite3*,पूर्णांक eTextRep,स्थिर अक्षर*)
){
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !sqlite3SafetyCheckOk(db) ) वापस SQLITE_MISUSE_BKPT;
#पूर्ण_अगर
  sqlite3_mutex_enter(db->mutex);
  db->xCollNeeded = xCollNeeded;
  db->xCollNeeded16 = 0;
  db->pCollNeededArg = pCollNeededArg;
  sqlite3_mutex_leave(db->mutex);
  वापस SQLITE_OK;
}

#अगर_अघोषित SQLITE_OMIT_UTF16
/*
** Register a collation sequence factory callback with the database handle
** db. Replace any previously installed collation sequence factory.
*/
पूर्णांक sqlite3_collation_needed16(
  sqlite3 *db,
  व्योम *pCollNeededArg,
  व्योम(*xCollNeeded16)(व्योम*,sqlite3*,पूर्णांक eTextRep,स्थिर व्योम*)
){
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !sqlite3SafetyCheckOk(db) ) वापस SQLITE_MISUSE_BKPT;
#पूर्ण_अगर
  sqlite3_mutex_enter(db->mutex);
  db->xCollNeeded = 0;
  db->xCollNeeded16 = xCollNeeded16;
  db->pCollNeededArg = pCollNeededArg;
  sqlite3_mutex_leave(db->mutex);
  वापस SQLITE_OK;
}
#पूर्ण_अगर /* SQLITE_OMIT_UTF16 */

/*
** Find existing client data.
*/
व्योम *sqlite3_get_clientdata(sqlite3 *db, स्थिर अक्षर *zName){
  Db_Client_तथ्य *p;
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !zName || !sqlite3SafetyCheckOk(db) ){
    (व्योम)SQLITE_MISUSE_BKPT;
    वापस 0;
  }
#पूर्ण_अगर
  sqlite3_mutex_enter(db->mutex);
  क्रम(p=db->p_Db_तथ्य; p; p=p->p_अग्र){
    अगर( म_भेद(p->zName, zName)==0 ){
      व्योम *p_परिणाम = p->p_तथ्य;
      sqlite3_mutex_leave(db->mutex);
      वापस p_परिणाम;
    }
  }
  sqlite3_mutex_leave(db->mutex);
  वापस 0;
}

/*
** Add new client data to a database connection.
*/
पूर्णांक sqlite3_set_clientdata(
  sqlite3 *db,                   /* Attach client data to this connection */
  स्थिर अक्षर *zName,             /* Name of the client data */
  व्योम *p_तथ्य,                   /* The client data itself */
  व्योम (*xDestructor)(व्योम*)     /* Destructor */
){
  Db_Client_तथ्य *p, **pp;
  sqlite3_mutex_enter(db->mutex);
  pp = &db->p_Db_तथ्य;
  क्रम(p=db->p_Db_तथ्य; p && म_भेद(p->zName,zName); p=p->p_अग्र){
    pp = &p->p_अग्र;
  }
  अगर( p ){
    निश्चित( p->p_तथ्य!=0 );
    अगर( p->xDestructor ) p->xDestructor(p->p_तथ्य);
    अगर( p_तथ्य==0 ){
      *pp = p->p_अग्र;
      sqlite3_free(p);
      sqlite3_mutex_leave(db->mutex);
      वापस SQLITE_OK;
    }
  }अन्यथा अगर( p_तथ्य==0 ){
    sqlite3_mutex_leave(db->mutex);
    वापस SQLITE_OK;
  }अन्यथा{
    माप_प्रकार n = म_माप(zName);
    p = sqlite3_malloc64( SZ_DBCLIENTDATA(n+1) );
    अगर( p==0 ){
      अगर( xDestructor ) xDestructor(p_तथ्य);
      sqlite3_mutex_leave(db->mutex);
      वापस SQLITE_NOMEM;
    }
    स_नकल(p->zName, zName, n+1);
    p->p_अग्र = db->p_Db_तथ्य;
    db->p_Db_तथ्य = p;
  }
  p->p_तथ्य = p_तथ्य;
  p->xDestructor = xDestructor;
  sqlite3_mutex_leave(db->mutex);
  वापस SQLITE_OK;
}


#अगर_अघोषित SQLITE_OMIT_DEPRECATED
/*
** This function is now an anachronism. It used to be used to recover from a
** malloc() failure, but SQLite now does this automatically.
*/
पूर्णांक sqlite3_global_recover(व्योम){
  वापस SQLITE_OK;
}
#पूर्ण_अगर

/*
** Test to see whether or not the database connection is in autocommit
** mode.  Return TRUE if it is and FALSE if not.  Autocommit mode is on
** by default.  Autocommit is disabled by a BEGIN statement and reenabled
** by the next COMMIT or ROLLBACK.
*/
पूर्णांक sqlite3_get_autocommit(sqlite3 *db){
  पूर्णांक iRet;
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !sqlite3SafetyCheckOk(db) ){
    (व्योम)SQLITE_MISUSE_BKPT;
    वापस 0;
  }
#पूर्ण_अगर
  sqlite3_mutex_enter(db->mutex);
  iRet = db->autoCommit;
  sqlite3_mutex_leave(db->mutex);
  वापस iRet;
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
पूर्णांक sqlite3ReportError(पूर्णांक iErr, पूर्णांक lineno, स्थिर अक्षर *zType){
  sqlite3_log(iErr, "%s at line %d of [%.10s]",
              zType, lineno, 20+sqlite3_sourceid());
  वापस iErr;
}
पूर्णांक sqlite3CorruptError(पूर्णांक lineno){
  testcase( sqlite3GlobalConfig.xLog!=0 );
  वापस sqlite3ReportError(SQLITE_CORRUPT, lineno, "database corruption");
}
पूर्णांक sqlite3MisuseError(पूर्णांक lineno){
  testcase( sqlite3GlobalConfig.xLog!=0 );
  वापस sqlite3ReportError(SQLITE_MISUSE, lineno, "misuse");
}
पूर्णांक sqlite3CantopenError(पूर्णांक lineno){
  testcase( sqlite3GlobalConfig.xLog!=0 );
  वापस sqlite3ReportError(SQLITE_CANTOPEN, lineno, "cannot open file");
}
#अगर defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_CORRUPT_PGNO)
पूर्णांक sqlite3CorruptPgnoError(पूर्णांक lineno, Pgno pgno){
  अक्षर z_संदेश[100];
  sqlite3_snprintf(माप(z_संदेश), z_संदेश, "database corruption page %d", pgno);
  testcase( sqlite3GlobalConfig.xLog!=0 );
  वापस sqlite3ReportError(SQLITE_CORRUPT, lineno, z_संदेश);
}
#पूर्ण_अगर
#अगर_घोषित SQLITE_DEBUG
पूर्णांक sqlite3NomemError(पूर्णांक lineno){
  testcase( sqlite3GlobalConfig.xLog!=0 );
  वापस sqlite3ReportError(SQLITE_NOMEM, lineno, "OOM");
}
पूर्णांक sqlite3IoerrnomemError(पूर्णांक lineno){
  testcase( sqlite3GlobalConfig.xLog!=0 );
  वापस sqlite3ReportError(SQLITE_IOERR_NOMEM, lineno, "I/O OOM error");
}
#पूर्ण_अगर

#अगर_अघोषित SQLITE_OMIT_DEPRECATED
/*
** This is a convenience routine that makes sure that all thread-specific
** data for this thread has been deallocated.
**
** SQLite no longer uses thread-specific data so this routine is now a
** no-op.  It is retained for historical compatibility.
*/
व्योम sqlite3_thread_cleanup(व्योम){
}
#पूर्ण_अगर

/*
** Return meta information about a specific column of a database table.
** See comment in sqlite3.h (sqlite.h.in) for details.
*/
पूर्णांक sqlite3_table_column_metadata(
  sqlite3 *db,                /* Connection handle */
  स्थिर अक्षर *zDbName,        /* Database name or NULL */
  स्थिर अक्षर *zTableName,     /* Table name */
  स्थिर अक्षर *zColumnName,    /* Column name */
  अक्षर स्थिर **pz_तथ्य_Type,    /* OUTPUT: Declared data type */
  अक्षर स्थिर **pzCollSeq,     /* OUTPUT: Collation sequence name */
  पूर्णांक *pNotNull,              /* OUTPUT: True if NOT NULL constraint exists */
  पूर्णांक *p_Primary_कुंजी,           /* OUTPUT: True if column part of PK */
  पूर्णांक *pAutoinc               /* OUTPUT: True if column is auto-increment */
){
  पूर्णांक rc;
  अक्षर *z_Err_संदेश = 0;
  Table *pTab = 0;
  Column *pCol = 0;
  पूर्णांक iCol = 0;
  अक्षर स्थिर *z_तथ्य_Type = 0;
  अक्षर स्थिर *zCollSeq = 0;
  पूर्णांक notnull = 0;
  पूर्णांक primarykey = 0;
  पूर्णांक autoinc = 0;


#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !sqlite3SafetyCheckOk(db) || zTableName==0 ){
    वापस SQLITE_MISUSE_BKPT;
  }
#पूर्ण_अगर

  /* Ensure the database schema has been loaded */
  sqlite3_mutex_enter(db->mutex);
  sqlite3BtreeEnterAll(db);
  rc = sqlite3Init(db, &z_Err_संदेश);
  अगर( SQLITE_OK!=rc ){
    जाओ error_out;
  }

  /* Locate the table in question */
  pTab = sqlite3FindTable(db, zTableName, zDbName);
  अगर( !pTab || IsView(pTab) ){
    pTab = 0;
    जाओ error_out;
  }

  /* Find the column for which info is requested */
  अगर( zColumnName==0 ){
    /* Query for existence of table only */
  }अन्यथा{
    iCol = sqlite3_Column_सूचकांक(pTab, zColumnName);
    अगर( iCol>=0 ){
      pCol = &pTab->aCol[iCol];
    }अन्यथा{
      अगर( HasRowid(pTab) && sqlite3IsRowid(zColumnName) ){
        iCol = pTab->iPKey;
        pCol = iCol>=0 ? &pTab->aCol[iCol] : 0;
      }अन्यथा{
        pTab = 0;
        जाओ error_out;
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
  अगर( pCol ){
    z_तथ्य_Type = sqlite3ColumnType(pCol,0);
    zCollSeq = sqlite3ColumnColl(pCol);
    notnull = pCol->notNull!=0;
    primarykey  = (pCol->colFlags & COLFLAG_PRIMKEY)!=0;
    autoinc = pTab->iPKey==iCol && (pTab->tabFlags & TF_Autoincrement)!=0;
  }अन्यथा{
    z_तथ्य_Type = "INTEGER";
    primarykey = 1;
  }
  अगर( !zCollSeq ){
    zCollSeq = sqlite3_Str_द्विआधारी;
  }

error_out:
  sqlite3BtreeLeaveAll(db);

  /* Whether the function call succeeded or failed, set the output parameters
  ** to whatever their local counterparts contain. If an error did occur,
  ** this has the effect of zeroing all output parameters.
  */
  अगर( pz_तथ्य_Type ) *pz_तथ्य_Type = z_तथ्य_Type;
  अगर( pzCollSeq ) *pzCollSeq = zCollSeq;
  अगर( pNotNull ) *pNotNull = notnull;
  अगर( p_Primary_कुंजी ) *p_Primary_कुंजी = primarykey;
  अगर( pAutoinc ) *pAutoinc = autoinc;

  अगर( SQLITE_OK==rc && !pTab ){
    sqlite3DbFree(db, z_Err_संदेश);
    z_Err_संदेश = sqlite3MPrintf(db, "no such table column: %s.%s", zTableName,
        zColumnName);
    rc = SQLITE_ERROR;
  }
  sqlite3_Error_With_संदेश(db, rc, (z_Err_संदेश?"%s":0), z_Err_संदेश);
  sqlite3DbFree(db, z_Err_संदेश);
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  वापस rc;
}

/*
** Sleep for a little while.  Return the amount of time slept.
*/
पूर्णांक sqlite3_sleep(पूर्णांक ms){
  sqlite3_vfs *pVfs;
  पूर्णांक rc;
  pVfs = sqlite3_vfs_find(0);
  अगर( pVfs==0 ) वापस 0;

  /* This function works in milliseconds, but the underlying OsSleep()
  ** API uses microseconds. Hence the 1000's.
  */
  rc = (sqlite3OsSleep(pVfs, ms<0 ? 0 : 1000*ms)/1000);
  वापस rc;
}

/*
** Enable or disable the extended result codes.
*/
पूर्णांक sqlite3_extended_परिणाम_codes(sqlite3 *db, पूर्णांक onoff){
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !sqlite3SafetyCheckOk(db) ) वापस SQLITE_MISUSE_BKPT;
#पूर्ण_अगर
  sqlite3_mutex_enter(db->mutex);
  db->errMask = onoff ? 0xffffffff : 0xff;
  sqlite3_mutex_leave(db->mutex);
  वापस SQLITE_OK;
}

/*
** Invoke the xFileControl method on a particular database.
*/
पूर्णांक sqlite3_file_control(sqlite3 *db, स्थिर अक्षर *zDbName, पूर्णांक op, व्योम *pArg){
  पूर्णांक rc = SQLITE_ERROR;
  Btree *pBtree;

#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !sqlite3SafetyCheckOk(db) ) वापस SQLITE_MISUSE_BKPT;
#पूर्ण_अगर
  sqlite3_mutex_enter(db->mutex);
  pBtree = sqlite3DbNameToBtree(db, zDbName);
  अगर( pBtree ){
    Pager *pPager;
    sqlite3_file *fd;
    sqlite3BtreeEnter(pBtree);
    pPager = sqlite3BtreePager(pBtree);
    निश्चित( pPager!=0 );
    fd = sqlite3PagerFile(pPager);
    निश्चित( fd!=0 );
    अगर( op==SQLITE_FCNTL_FILE_POINTER ){
      *(sqlite3_file**)pArg = fd;
      rc = SQLITE_OK;
    }अन्यथा अगर( op==SQLITE_FCNTL_VFS_POINTER ){
      *(sqlite3_vfs**)pArg = sqlite3PagerVfs(pPager);
      rc = SQLITE_OK;
    }अन्यथा अगर( op==SQLITE_FCNTL_JOURNAL_POINTER ){
      *(sqlite3_file**)pArg = sqlite3PagerJrnlFile(pPager);
      rc = SQLITE_OK;
    }अन्यथा अगर( op==SQLITE_FCNTL_तथ्य_VERSION ){
      *(अचिन्हित पूर्णांक*)pArg = sqlite3_Pager_तथ्य_Version(pPager);
      rc = SQLITE_OK;
    }अन्यथा अगर( op==SQLITE_FCNTL_RESERVE_BYTES ){
      पूर्णांक iNew = *(पूर्णांक*)pArg;
      *(पूर्णांक*)pArg = sqlite3BtreeGetRequestedReserve(pBtree);
      अगर( iNew>=0 && iNew<=255 ){
        sqlite3_Btree_Set_Page_आकार(pBtree, 0, iNew, 0);
      }
      rc = SQLITE_OK;
    }अन्यथा अगर( op==SQLITE_FCNTL_RESET_CACHE ){
      sqlite3BtreeClearCache(pBtree);
      rc = SQLITE_OK;
    }अन्यथा{
      पूर्णांक nSave = db->busyHandler.nBusy;
      rc = sqlite3OsFileControl(fd, op, pArg);
      db->busyHandler.nBusy = nSave;
    }
    sqlite3BtreeLeave(pBtree);
  }
  sqlite3_mutex_leave(db->mutex);
  वापस rc;
}

/*
** Interface to the testing logic.
*/
पूर्णांक sqlite3_test_control(पूर्णांक op, ...){
  पूर्णांक rc = 0;
#अगर_घोषित SQLITE_UNTESTABLE
  UNUSED_PARAMETER(op);
#अन्यथा
  बहु_सूची ap;
  बहु_शुरू(ap, op);
  चयन( op ){

    /*
    ** Save the current state of the PRNG.
    */
    हाल SQLITE_TESTCTRL_PRNG_SAVE: {
      sqlite3PrngSaveState();
      अवरोध;
    }

    /*
    ** Restore the state of the PRNG to the last state saved using
    ** PRNG_SAVE.  If PRNG_SAVE has never before been called, then
    ** this verb acts like PRNG_RESET.
    */
    हाल SQLITE_TESTCTRL_PRNG_RESTORE: {
      sqlite3PrngRestoreState();
      अवरोध;
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
#अगर_अघोषित SQLITE_OMIT_WSD
    हाल SQLITE_TESTCTRL_PRNG_SEED: {
      पूर्णांक x = बहु_तर्क(ap, पूर्णांक);
      पूर्णांक y;
      sqlite3 *db = बहु_तर्क(ap, sqlite3*);
      निश्चित( db==0 || db->aDb[0].pSchema!=0 );
      अगर( db && (y = db->aDb[0].pSchema->schema_cookie)!=0 ){ x = y; }
      sqlite3Config.iPrngSeed = x;
      sqlite3_randomness(0,0);
      अवरोध;
    }
#पूर्ण_अगर

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
    हाल SQLITE_TESTCTRL_FK_NO_ACTION: {
      sqlite3 *db = बहु_तर्क(ap, sqlite3*);
      पूर्णांक b = बहु_तर्क(ap, पूर्णांक);
      अगर( b ){
        db->flags |= SQLITE_FkNoAction;
      }अन्यथा{
        db->flags &= ~SQLITE_FkNoAction;
      }
      अवरोध;
    }

    /*
    **  sqlite3_test_control(BITVEC_TEST, size, program)
    **
    ** Run a test against a Bitvec object of size.  The program argument
    ** is an array of integers that defines the test.  Return -1 on a
    ** memory allocation error, 0 on success, or non-zero for an error.
    ** See the sqlite3BitvecBuiltinTest() for additional information.
    */
    हाल SQLITE_TESTCTRL_BITVEC_TEST: {
      पूर्णांक sz = बहु_तर्क(ap, पूर्णांक);
      पूर्णांक *aProg = बहु_तर्क(ap, पूर्णांक*);
      rc = sqlite3BitvecBuiltinTest(sz, aProg);
      अवरोध;
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
    हाल SQLITE_TESTCTRL_FAULT_INSTALL: {
      /* A bug in MSVC prevents it from understanding pointers to functions
      ** types in the second argument to va_arg().  Work around the problem
      ** using a typedef.
      ** http://support.microsoft.com/kb/47961  <-- dead hyperlink
      ** Search at http://web.archive.org/ to find the 2015-03-16 archive
      ** of the link above to see the original text.
      ** sqlite3GlobalConfig.xTestCallback = va_arg(ap, int(*)(int));
      */
      प्रकार पूर्णांक(*sqlite3FaultFuncType)(पूर्णांक);
      sqlite3GlobalConfig.xTestCallback = बहु_तर्क(ap, sqlite3FaultFuncType);
      rc = sqlite3FaultSim(0);
      अवरोध;
    }

    /*
    **  sqlite3_test_control(BENIGN_MALLOC_HOOKS, xBegin, xEnd)
    **
    ** Register hooks to call to indicate which malloc() failures
    ** are benign.
    */
    हाल SQLITE_TESTCTRL_BENIGN_MALLOC_HOOKS: {
      प्रकार व्योम (*void_function)(व्योम);
      void_function xBenignBegin;
      void_function xBenignEnd;
      xBenignBegin = बहु_तर्क(ap, void_function);
      xBenignEnd = बहु_तर्क(ap, void_function);
      sqlite3BenignMallocHooks(xBenignBegin, xBenignEnd);
      अवरोध;
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
    हाल SQLITE_TESTCTRL_PENDING_BYTE: {
      rc = PENDING_BYTE;
#अगर_अघोषित SQLITE_OMIT_WSD
      {
        अचिन्हित पूर्णांक newVal = बहु_तर्क(ap, अचिन्हित पूर्णांक);
        अगर( newVal ) sqlite3PendingByte = newVal;
      }
#पूर्ण_अगर
      अवरोध;
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
    हाल SQLITE_TESTCTRL_ASSERT: {
      अस्थिर पूर्णांक x = 0;
      निश्चित( /*side-effects-ok*/ (x = बहु_तर्क(ap,पूर्णांक))!=0 );
      rc = x;
#अगर defined(SQLITE_DEBUG)
      /* Invoke these debugging routines so that the compiler does not
      ** issue "defined but not used" warnings. */
      अगर( x==9999 ){
        sqlite3_Show_व्यंजक(0);
        sqlite3_Show_व्यंजक_List(0);
        sqlite3ShowIdList(0);
        sqlite3ShowSrcList(0);
        sqlite3ShowWith(0);
        sqlite3ShowUpsert(0);
#अगर_अघोषित SQLITE_OMIT_TRIGGER
        sqlite3ShowTriggerStep(0);
        sqlite3ShowTriggerStepList(0);
        sqlite3ShowTrigger(0);
        sqlite3ShowTriggerList(0);
#पूर्ण_अगर
#अगर_अघोषित SQLITE_OMIT_WINDOWFUNC
        sqlite3ShowWindow(0);
        sqlite3ShowWinFunc(0);
#पूर्ण_अगर
        sqlite3ShowSelect(0);
      }
#पूर्ण_अगर
      अवरोध;
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
    हाल SQLITE_TESTCTRL_ALWAYS: {
      पूर्णांक x = बहु_तर्क(ap,पूर्णांक);
      rc = x ? ALWAYS(x) : 0;
      अवरोध;
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
    हाल SQLITE_TESTCTRL_BYTEORDER: {
      rc = SQLITE_BYTEORDER*100 + SQLITE_LITTLEENDIAN*10 + SQLITE_BIGENDIAN;
      अवरोध;
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
    हाल SQLITE_TESTCTRL_OPTIMIZATIONS: {
      sqlite3 *db = बहु_तर्क(ap, sqlite3*);
      db->dbOptFlags = बहु_तर्क(ap, u32);
      अवरोध;
    }

    /*  sqlite3_test_control(SQLITE_TESTCTRL_GETOPT, sqlite3 *db, int *N)
    **
    ** Write the current optimization settings into *N.  A zero bit means that
    ** the optimization is on, and a 1 bit means that the optimization is off.
    */
    हाल SQLITE_TESTCTRL_GETOPT: {
      sqlite3 *db = बहु_तर्क(ap, sqlite3*);
      पूर्णांक *pN = बहु_तर्क(ap, पूर्णांक*);
      *pN = db->dbOptFlags;
      अवरोध;
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
    हाल SQLITE_TESTCTRL_LOCALTIME_FAULT: {
      sqlite3GlobalConfig.bLocaltimeFault = बहु_तर्क(ap, पूर्णांक);
      अगर( sqlite3GlobalConfig.bLocaltimeFault==2 ){
        प्रकार पूर्णांक(*sqlite3LocaltimeType)(स्थिर व्योम*,व्योम*);
        sqlite3GlobalConfig.xAltLocaltime = बहु_तर्क(ap, sqlite3LocaltimeType);
      }अन्यथा{
        sqlite3GlobalConfig.xAltLocaltime = 0;
      }
      अवरोध;
    }

    /*   sqlite3_test_control(SQLITE_TESTCTRL_INTERNAL_FUNCTIONS, sqlite3*);
    **
    ** Toggle the ability to use internal functions on or off for
    ** the database connection given in the argument.
    */
    हाल SQLITE_TESTCTRL_INTERNAL_FUNCTIONS: {
      sqlite3 *db = बहु_तर्क(ap, sqlite3*);
      db->mDbFlags ^= DBFLAG_InternalFunc;
      अवरोध;
    }

    /*   sqlite3_test_control(SQLITE_TESTCTRL_NEVER_CORRUPT, int);
    **
    ** Set or clear a flag that indicates that the database file is always well-
    ** formed and never corrupt.  This flag is clear by default, indicating that
    ** database files might have arbitrary corruption.  Setting the flag during
    ** testing causes certain assert() statements in the code to be activated
    ** that demonstrate invariants on well-formed database files.
    */
    हाल SQLITE_TESTCTRL_NEVER_CORRUPT: {
      sqlite3GlobalConfig.neverCorrupt = बहु_तर्क(ap, पूर्णांक);
      अवरोध;
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
    हाल SQLITE_TESTCTRL_EXTRA_SCHEMA_CHECKS: {
      sqlite3GlobalConfig.bExtraSchemaChecks = बहु_तर्क(ap, पूर्णांक);
      अवरोध;
    }

    /* Set the threshold at which OP_Once counters reset back to zero.
    ** By default this is 0x7ffffffe (over 2 billion), but that value is
    ** too big to test in a reasonable amount of time, so this control is
    ** provided to set a small and easily reachable reset value.
    */
    हाल SQLITE_TESTCTRL_ONCE_RESET_THRESHOLD: {
      sqlite3GlobalConfig.iOnceResetThreshold = बहु_तर्क(ap, पूर्णांक);
      अवरोध;
    }

    /*   sqlite3_test_control(SQLITE_TESTCTRL_VDBE_COVERAGE, xCallback, ptr);
    **
    ** Set the VDBE coverage callback function to xCallback with context
    ** pointer ptr.
    */
    हाल SQLITE_TESTCTRL_VDBE_COVERAGE: {
#अगर_घोषित SQLITE_VDBE_COVERAGE
      प्रकार व्योम (*branch_callback)(व्योम*,अचिन्हित पूर्णांक,
                                      अचिन्हित अक्षर,अचिन्हित अक्षर);
      sqlite3GlobalConfig.xVdbeBranch = बहु_तर्क(ap,branch_callback);
      sqlite3GlobalConfig.pVdbeBranchArg = बहु_तर्क(ap,व्योम*);
#पूर्ण_अगर
      अवरोध;
    }

    /*   sqlite3_test_control(SQLITE_TESTCTRL_SORTER_MMAP, db, nMax); */
    हाल SQLITE_TESTCTRL_SORTER_MMAP: {
      sqlite3 *db = बहु_तर्क(ap, sqlite3*);
      db->nMaxSorterMmap = बहु_तर्क(ap, पूर्णांक);
      अवरोध;
    }

    /*   sqlite3_test_control(SQLITE_TESTCTRL_ISINIT);
    **
    ** Return SQLITE_OK if SQLite has been initialized and SQLITE_ERROR if
    ** not.
    */
    हाल SQLITE_TESTCTRL_ISINIT: {
      अगर( sqlite3GlobalConfig.isInit==0 ) rc = SQLITE_ERROR;
      अवरोध;
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
    हाल SQLITE_TESTCTRL_IMPOSTER: {
      sqlite3 *db = बहु_तर्क(ap, sqlite3*);
      पूर्णांक iDb;
      sqlite3_mutex_enter(db->mutex);
      iDb = sqlite3FindDbName(db, बहु_तर्क(ap,स्थिर अक्षर*));
      अगर( iDb>=0 ){
        db->init.iDb = iDb;
        db->init.busy = db->init.imposterTable = बहु_तर्क(ap,पूर्णांक);
        db->init.newTnum = बहु_तर्क(ap,पूर्णांक);
        अगर( db->init.busy==0 && db->init.newTnum>0 ){
          sqlite3ResetAllSchemasOfConnection(db);
        }
      }
      sqlite3_mutex_leave(db->mutex);
      अवरोध;
    }

#अगर defined(YYCOVERAGE)
    /*  sqlite3_test_control(SQLITE_TESTCTRL_PARSER_COVERAGE, FILE *out)
    **
    ** This test control (only available when SQLite is compiled with
    ** -DYYCOVERAGE) writes a report onto "out" that shows all
    ** state/lookahead combinations in the parser state machine
    ** which are never exercised.  If any state is missed, make the
    ** return code SQLITE_ERROR.
    */
    हाल SQLITE_TESTCTRL_PARSER_COVERAGE: {
      खाता *out = बहु_तर्क(ap, खाता*);
      अगर( sqlite3ParserCoverage(out) ) rc = SQLITE_ERROR;
      अवरोध;
    }
#पूर्ण_अगर /* defined(YYCOVERAGE) */

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
    हाल SQLITE_TESTCTRL_परिणाम_INTREAL: {
      sqlite3_context *pCtx = बहु_तर्क(ap, sqlite3_context*);
      sqlite3_परिणाम_Int_Real(pCtx);
      अवरोध;
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
    हाल SQLITE_TESTCTRL_SEEK_गिनती: {
      sqlite3 *db = बहु_तर्क(ap, sqlite3*);
      u64 *pn = बहु_तर्क(ap, sqlite3_uint64*);
      *pn = sqlite3_Btree_Seek_गिनती(db->aDb->pBt);
      (व्योम)db;  /* Silence harmless unused variable warning */
      अवरोध;
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
    हाल SQLITE_TESTCTRL_TRACEFLAGS: {
       पूर्णांक opTrace = बहु_तर्क(ap, पूर्णांक);
       u32 *ptr = बहु_तर्क(ap, u32*);
       चयन( opTrace ){
         हाल 0:   *ptr = sqlite3_वृक्ष_Trace;      अवरोध;
         हाल 1:   sqlite3_वृक्ष_Trace = *ptr;      अवरोध;
         हाल 2:   *ptr = sqlite3WhereTrace;     अवरोध;
         हाल 3:   sqlite3WhereTrace = *ptr;     अवरोध;
       }
       अवरोध;
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
    हाल SQLITE_TESTCTRL_LOGEST: {
      द्विगुन rIn = बहु_तर्क(ap, द्विगुन);
      LogEst rLogEst = sqlite3LogEstFromDouble(rIn);
      पूर्णांक *pI1 = बहु_तर्क(ap,पूर्णांक*);
      u64 *pU64 = बहु_तर्क(ap,u64*);
      पूर्णांक *pI2 = बहु_तर्क(ap,पूर्णांक*);
      *pI1 = rLogEst;
      *pU64 = sqlite3LogEstToInt(rLogEst);
      *pI2 = sqlite3LogEst(*pU64);
      अवरोध;
    }

    /* sqlite3_test_control(SQLITE_TESTCTRL_ATOF, const char *z, double *p);
    **
    ** Test access to the sqlite3AtoF() routine.
    */
    हाल SQLITE_TESTCTRL_ATOF: {
      स्थिर अक्षर *z = बहु_तर्क(ap,स्थिर अक्षर*);
      द्विगुन *pR = बहु_तर्क(ap,द्विगुन*);
      rc = sqlite3AtoF(z,pR);
      अवरोध;
    }

#अगर defined(SQLITE_DEBUG) && !defined(SQLITE_OMIT_WSD)
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
    हाल SQLITE_TESTCTRL_TUNE: {
      पूर्णांक id = बहु_तर्क(ap, पूर्णांक);
      पूर्णांक *pi_मूल्य = बहु_तर्क(ap, पूर्णांक*);
      अगर( id>0 && id<=SQLITE_NTUNE ){
        Tuning(id) = *pi_मूल्य;
      }अन्यथा अगर( id<0 && id>=-SQLITE_NTUNE ){
        *pi_मूल्य = Tuning(-id);
      }अन्यथा{
        rc = SQLITE_NOTFOUND;
      }
      अवरोध;
    }
#पूर्ण_अगर

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
    हाल SQLITE_TESTCTRL_JSON_SELFCHECK: {
#अगर defined(SQLITE_DEBUG) && !defined(SQLITE_OMIT_WSD)
      पूर्णांक *pOnOff = बहु_तर्क(ap, पूर्णांक*);
      अगर( *pOnOff<0 ){
        *pOnOff = sqlite3Config.bJsonSelfcheck;
      }अन्यथा{
        sqlite3Config.bJsonSelfcheck = (u8)((*pOnOff)&0xff);
      }
#पूर्ण_अगर
      अवरोध;
    }
  }
  बहु_पूर्ण(ap);
#पूर्ण_अगर /* SQLITE_UNTESTABLE */
  वापस rc;
}

/*
** The Pager stores the Database filename, Journal filename, and WAL filename
** consecutively in memory, in that order.  The database filename is prefixed
** by four zero bytes.  Locate the start of the database filename by searching
** backwards for the first byte following four consecutive zero bytes.
**
** This only works if the filename passed in was obtained from the Pager.
*/
अटल स्थिर अक्षर *databaseName(स्थिर अक्षर *zName){
  जबतक( zName[-1]!=0 || zName[-2]!=0 || zName[-3]!=0 || zName[-4]!=0 ){
    zName--;
  }
  वापस zName;
}

/*
** Append text z[] to the end of p[].  Return a pointer to the first
** character after then zero terminator on the new text in p[].
*/
अटल अक्षर *appendText(अक्षर *p, स्थिर अक्षर *z){
  माप_प्रकार n = म_माप(z);
  स_नकल(p, z, n+1);
  वापस p+n+1;
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
स्थिर अक्षर *sqlite3_create_filename(
  स्थिर अक्षर *zDatabase,
  स्थिर अक्षर *zJournal,
  स्थिर अक्षर *zWal,
  पूर्णांक nParam,
  स्थिर अक्षर **azParam
){
  sqlite3_int64 nByte;
  पूर्णांक i;
  अक्षर *p_परिणाम, *p;
  nByte = म_माप(zDatabase) + म_माप(zJournal) + म_माप(zWal) + 10;
  क्रम(i=0; i<nParam*2; i++){
    nByte += म_माप(azParam[i])+1;
  }
  p_परिणाम = p = sqlite3_malloc64( nByte );
  अगर( p==0 ) वापस 0;
  स_रखो(p, 0, 4);
  p += 4;
  p = appendText(p, zDatabase);
  क्रम(i=0; i<nParam*2; i++){
    p = appendText(p, azParam[i]);
  }
  *(p++) = 0;
  p = appendText(p, zJournal);
  p = appendText(p, zWal);
  *(p++) = 0;
  *(p++) = 0;
  निश्चित( (sqlite3_int64)(p - p_परिणाम)==nByte );
  वापस p_परिणाम + 4;
}

/*
** Free memory obtained from sqlite3_create_filename().  It is a severe
** error to call this routine with any parameter other than a pointer
** previously obtained from sqlite3_create_filename() or a NULL pointer.
*/
व्योम sqlite3_free_filename(स्थिर अक्षर *p){
  अगर( p==0 ) वापस;
  p = databaseName(p);
  sqlite3_free((अक्षर*)p - 4);
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
स्थिर अक्षर *sqlite3_uri_parameter(स्थिर अक्षर *zFilename, स्थिर अक्षर *zParam){
  अगर( zFilename==0 || zParam==0 ) वापस 0;
  zFilename = databaseName(zFilename);
  वापस uriParameter(zFilename, zParam);
}

/*
** Return a pointer to the name of Nth query parameter of the filename.
*/
स्थिर अक्षर *sqlite3_uri_कुंजी(स्थिर अक्षर *zFilename, पूर्णांक N){
  अगर( zFilename==0 || N<0 ) वापस 0;
  zFilename = databaseName(zFilename);
  zFilename += sqlite3Strlen30(zFilename) + 1;
  जबतक( ALWAYS(zFilename) && zFilename[0] && (N--)>0 ){
    zFilename += sqlite3Strlen30(zFilename) + 1;
    zFilename += sqlite3Strlen30(zFilename) + 1;
  }
  वापस zFilename[0] ? zFilename : 0;
}

/*
** Return a boolean value for a query parameter.
*/
पूर्णांक sqlite3_uri_boolean(स्थिर अक्षर *zFilename, स्थिर अक्षर *zParam, पूर्णांक bDflt){
  स्थिर अक्षर *z = sqlite3_uri_parameter(zFilename, zParam);
  bDflt = bDflt!=0;
  वापस z ? sqlite3GetBoolean(z, bDflt) : bDflt;
}

/*
** Return a 64-bit integer value for a query parameter.
*/
sqlite3_int64 sqlite3_uri_int64(
  स्थिर अक्षर *zFilename,    /* Filename as passed to xOpen */
  स्थिर अक्षर *zParam,       /* URI parameter sought */
  sqlite3_int64 bDflt       /* return if parameter is missing */
){
  स्थिर अक्षर *z = sqlite3_uri_parameter(zFilename, zParam);
  sqlite3_int64 v;
  अगर( z && sqlite3DecOrHexToI64(z, &v)==0 ){
    bDflt = v;
  }
  वापस bDflt;
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
स्थिर अक्षर *sqlite3_filename_database(स्थिर अक्षर *zFilename){
  अगर( zFilename==0 ) वापस 0;
  वापस databaseName(zFilename);
}
स्थिर अक्षर *sqlite3_filename_journal(स्थिर अक्षर *zFilename){
  अगर( zFilename==0 ) वापस 0;
  zFilename = databaseName(zFilename);
  zFilename += sqlite3Strlen30(zFilename) + 1;
  जबतक( ALWAYS(zFilename) && zFilename[0] ){
    zFilename += sqlite3Strlen30(zFilename) + 1;
    zFilename += sqlite3Strlen30(zFilename) + 1;
  }
  वापस zFilename + 1;
}
स्थिर अक्षर *sqlite3_filename_wal(स्थिर अक्षर *zFilename){
#अगर_घोषित SQLITE_OMIT_WAL
  UNUSED_PARAMETER(zFilename);
  वापस 0;
#अन्यथा
  zFilename = sqlite3_filename_journal(zFilename);
  अगर( zFilename ) zFilename += sqlite3Strlen30(zFilename) + 1;
  वापस zFilename;
#पूर्ण_अगर
}

/*
** Return the Btree pointer identified by zDbName.  Return NULL if not found.
*/
Btree *sqlite3DbNameToBtree(sqlite3 *db, स्थिर अक्षर *zDbName){
  पूर्णांक iDb = zDbName ? sqlite3FindDbName(db, zDbName) : 0;
  वापस iDb<0 ? 0 : db->aDb[iDb].pBt;
}

/*
** Return the name of the N-th database schema.  Return NULL if N is out
** of range.
*/
स्थिर अक्षर *sqlite3_db_name(sqlite3 *db, पूर्णांक N){
  स्थिर अक्षर *zRet = 0;
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !sqlite3SafetyCheckOk(db) ){
    (व्योम)SQLITE_MISUSE_BKPT;
    वापस 0;
  }
#पूर्ण_अगर
  sqlite3_mutex_enter(db->mutex);
  अगर( N>=0 && N<db->nDb ){
    zRet = db->aDb[N].zDbSName;
  }
  sqlite3_mutex_leave(db->mutex);
  वापस zRet;
}

/*
** Return the filename of the database associated with a database
** connection.
*/
स्थिर अक्षर *sqlite3_db_filename(sqlite3 *db, स्थिर अक्षर *zDbName){
  Btree *pBt;
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !sqlite3SafetyCheckOk(db) ){
    (व्योम)SQLITE_MISUSE_BKPT;
    वापस 0;
  }
#पूर्ण_अगर
  pBt = sqlite3DbNameToBtree(db, zDbName);
  वापस pBt ? sqlite3BtreeGetFilename(pBt) : 0;
}

/*
** Return 1 if database is read-only or 0 if read/write.  Return -1 if
** no such database exists.
*/
पूर्णांक sqlite3_db_readonly(sqlite3 *db, स्थिर अक्षर *zDbName){
  Btree *pBt;
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !sqlite3SafetyCheckOk(db) ){
    (व्योम)SQLITE_MISUSE_BKPT;
    वापस -1;
  }
#पूर्ण_अगर
  pBt = sqlite3DbNameToBtree(db, zDbName);
  वापस pBt ? sqlite3BtreeIsReadonly(pBt) : -1;
}

#अगर_घोषित SQLITE_ENABLE_SNAPSHOT
/*
** Obtain a snapshot handle for the snapshot of database zDb currently
** being read by handle db.
*/
पूर्णांक sqlite3_snapshot_get(
  sqlite3 *db,
  स्थिर अक्षर *zDb,
  sqlite3_snapshot **ppSnapshot
){
  पूर्णांक rc = SQLITE_ERROR;
#अगर_अघोषित SQLITE_OMIT_WAL

#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !sqlite3SafetyCheckOk(db) ){
    वापस SQLITE_MISUSE_BKPT;
  }
#पूर्ण_अगर
  sqlite3_mutex_enter(db->mutex);

  अगर( db->autoCommit==0 ){
    पूर्णांक iDb = sqlite3FindDbName(db, zDb);
    अगर( iDb==0 || iDb>1 ){
      Btree *pBt = db->aDb[iDb].pBt;
      अगर( SQLITE_TXN_WRITE!=sqlite3BtreeTxnState(pBt) ){
        Pager *pPager = sqlite3BtreePager(pBt);
        i64 dummy = 0;
        sqlite3PagerSnapshotOpen(pPager, (sqlite3_snapshot*)&dummy);
        rc = sqlite3BtreeBeginTrans(pBt, 0, 0);
        sqlite3PagerSnapshotOpen(pPager, 0);
        अगर( rc==SQLITE_OK ){
          rc = sqlite3PagerSnapshotGet(sqlite3BtreePager(pBt), ppSnapshot);
        }
      }
    }
  }

  sqlite3_mutex_leave(db->mutex);
#पूर्ण_अगर   /* SQLITE_OMIT_WAL */
  वापस rc;
}

/*
** Open a read-transaction on the snapshot identified by pSnapshot.
*/
पूर्णांक sqlite3_snapshot_open(
  sqlite3 *db,
  स्थिर अक्षर *zDb,
  sqlite3_snapshot *pSnapshot
){
  पूर्णांक rc = SQLITE_ERROR;
#अगर_अघोषित SQLITE_OMIT_WAL

#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !sqlite3SafetyCheckOk(db) ){
    वापस SQLITE_MISUSE_BKPT;
  }
#पूर्ण_अगर
  sqlite3_mutex_enter(db->mutex);
  अगर( db->autoCommit==0 ){
    पूर्णांक iDb;
    iDb = sqlite3FindDbName(db, zDb);
    अगर( iDb==0 || iDb>1 ){
      Btree *pBt = db->aDb[iDb].pBt;
      अगर( sqlite3BtreeTxnState(pBt)!=SQLITE_TXN_WRITE ){
        Pager *pPager = sqlite3BtreePager(pBt);
        पूर्णांक bUnlock = 0;
        अगर( sqlite3BtreeTxnState(pBt)!=SQLITE_TXN_NONE ){
          अगर( db->nVdbeActive==0 ){
            rc = sqlite3PagerSnapshotCheck(pPager, pSnapshot);
            अगर( rc==SQLITE_OK ){
              bUnlock = 1;
              rc = sqlite3BtreeCommit(pBt);
            }
          }
        }अन्यथा{
          rc = SQLITE_OK;
        }
        अगर( rc==SQLITE_OK ){
          rc = sqlite3PagerSnapshotOpen(pPager, pSnapshot);
        }
        अगर( rc==SQLITE_OK ){
          rc = sqlite3BtreeBeginTrans(pBt, 0, 0);
          sqlite3PagerSnapshotOpen(pPager, 0);
        }
        अगर( bUnlock ){
          sqlite3PagerSnapshotUnlock(pPager);
        }
      }
    }
  }

  sqlite3_mutex_leave(db->mutex);
#पूर्ण_अगर   /* SQLITE_OMIT_WAL */
  वापस rc;
}

/*
** Recover as many snapshots as possible from the wal file associated with
** schema zDb of database db.
*/
पूर्णांक sqlite3_snapshot_recover(sqlite3 *db, स्थिर अक्षर *zDb){
  पूर्णांक rc = SQLITE_ERROR;
#अगर_अघोषित SQLITE_OMIT_WAL
  पूर्णांक iDb;

#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !sqlite3SafetyCheckOk(db) ){
    वापस SQLITE_MISUSE_BKPT;
  }
#पूर्ण_अगर

  sqlite3_mutex_enter(db->mutex);
  iDb = sqlite3FindDbName(db, zDb);
  अगर( iDb==0 || iDb>1 ){
    Btree *pBt = db->aDb[iDb].pBt;
    अगर( SQLITE_TXN_NONE==sqlite3BtreeTxnState(pBt) ){
      rc = sqlite3BtreeBeginTrans(pBt, 0, 0);
      अगर( rc==SQLITE_OK ){
        rc = sqlite3PagerSnapshotRecover(sqlite3BtreePager(pBt));
        sqlite3BtreeCommit(pBt);
      }
    }
  }
  sqlite3_mutex_leave(db->mutex);
#पूर्ण_अगर   /* SQLITE_OMIT_WAL */
  वापस rc;
}

/*
** Free a snapshot handle obtained from sqlite3_snapshot_get().
*/
व्योम sqlite3_snapshot_free(sqlite3_snapshot *pSnapshot){
  sqlite3_free(pSnapshot);
}
#पूर्ण_अगर /* SQLITE_ENABLE_SNAPSHOT */

#अगर_अघोषित SQLITE_OMIT_COMPILEOPTION_DIAGS
/*
** Given the name of a compile-time option, return true if that option
** was used and false if not.
**
** The name can optionally begin with "SQLITE_" but the "SQLITE_" prefix
** is not required for a match.
*/
पूर्णांक sqlite3_compileoption_used(स्थिर अक्षर *zOptName){
  पूर्णांक i, n;
  पूर्णांक nOpt;
  स्थिर अक्षर **azCompileOpt;

#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( zOptName==0 ){
    (व्योम)SQLITE_MISUSE_BKPT;
    वापस 0;
  }
#पूर्ण_अगर

  azCompileOpt = sqlite3CompileOptions(&nOpt);

  अगर( sqlite3StrNICmp(zOptName, "SQLITE_", 7)==0 ) zOptName += 7;
  n = sqlite3Strlen30(zOptName);

  /* Since nOpt is normally in single digits, a linear search is
  ** adequate. No need for a binary search. */
  क्रम(i=0; i<nOpt; i++){
    अगर( sqlite3StrNICmp(zOptName, azCompileOpt[i], n)==0
     && sqlite3IsIdChar((अचिन्हित अक्षर)azCompileOpt[i][n])==0
    ){
      वापस 1;
    }
  }
  वापस 0;
}

/*
** Return the N-th compile-time option string.  If N is out of range,
** return a NULL pointer.
*/
स्थिर अक्षर *sqlite3_compileoption_get(पूर्णांक N){
  पूर्णांक nOpt;
  स्थिर अक्षर **azCompileOpt;
  azCompileOpt = sqlite3CompileOptions(&nOpt);
  अगर( N>=0 && N<nOpt ){
    वापस azCompileOpt[N];
  }
  वापस 0;
}
#पूर्ण_अगर /* SQLITE_OMIT_COMPILEOPTION_DIAGS */
