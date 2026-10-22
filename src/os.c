/*
** 2005 November 29
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
******************************************************************************
**
** This file contains OS interface code that is common to all
** architectures.
*/
#ضمن "sqliteInt.h"

/*
** If we compile with the SQLITE_TEST macro set, then the following block
** of code will give us the ability to simulate a disk I/O error.  This
** is used for testing the I/O recovery logic.
*/
#ܐܢ defined(SQLITE_TEST)
صحيح sqlite3_io_error_hit = 0;            /* Total number of I/O Errors */
صحيح sqlite3_io_error_hardhit = 0;        /* Number of non-benign errors */
صحيح sqlite3_io_error_pending = 0;        /* Count down to first I/O error */
صحيح sqlite3_io_error_persist = 0;        /* True if I/O errors persist */
صحيح sqlite3_io_error_benign = 0;         /* True if errors are benign */
صحيح sqlite3_diskfull_pending = 0;
صحيح sqlite3_diskfull = 0;
#endif /* defined(SQLITE_TEST) */

/*
** When testing, also keep a count of the number of open files.
*/
#ܐܢ defined(SQLITE_TEST)
صحيح sqlite3_open_file_count = 0;
#endif /* defined(SQLITE_TEST) */

/*
** The default SQLite sqlite3_vfs implementations do not allocate
** memory (actually, os_unix.c allocates a small amount of memory
** from within OsOpen()), but some third-party implementations may.
** So we test the effects of a malloc() failing and the sqlite3OsXXX()
** function returning SQLITE_IOERR_NOMEM using the DO_OS_MALLOC_TEST macro.
**
** The following functions are instrumented for malloc() failure
** testing:
**
**     sqlite3OsRead()
**     sqlite3OsWrite()
**     sqlite3OsSync()
**     sqlite3OsFileSize()
**     sqlite3OsLock()
**     sqlite3OsCheckReservedLock()
**     sqlite3OsFileControl()
**     sqlite3OsShmMap()
**     sqlite3OsOpen()
**     sqlite3OsDelete()
**     sqlite3OsAccess()
**     sqlite3OsFullPathname()
**
*/
#ܐܢ defined(SQLITE_TEST)
صحيح sqlite3_memdebug_vfs_oom_test = 1;
  #define DO_OS_MALLOC_TEST(x)                                       \
  ܐܢ (sqlite3_memdebug_vfs_oom_test && (!x || !sqlite3JournalIsInMemory(x))) { \
    فراغ *pTstAlloc = sqlite3Malloc(10);                             \
    ܐܢ (!pTstAlloc) ܦܢܐ SQLITE_IOERR_NOMEM_BKPT;                  \
    sqlite3_free(pTstAlloc);                                         \
  }
#ܐܚܪܢܐ
  #define DO_OS_MALLOC_TEST(x)
#endif

/*
** The following routines are convenience wrappers around methods
** of the sqlite3_file object.  This is mostly just syntactic sugar. All
** of this would be completely automatic if SQLite were coded using
** C++ instead of plain old C.
*/
فراغ sqlite3OsClose(sqlite3_file *pId){
  ܐܢ( pId->pMethods ){
    pId->pMethods->xClose(pId);
    pId->pMethods = 0;
  }
}
صحيح sqlite3OsRead(sqlite3_file *id, فراغ *pBuf, صحيح amt, i64 offset){
  DO_OS_MALLOC_TEST(id);
  ܦܢܐ id->pMethods->xRead(id, pBuf, amt, offset);
}
صحيح sqlite3OsWrite(sqlite3_file *id, ثابت فراغ *pBuf, صحيح amt, i64 offset){
  DO_OS_MALLOC_TEST(id);
  ܦܢܐ id->pMethods->xWrite(id, pBuf, amt, offset);
}
صحيح sqlite3OsTruncate(sqlite3_file *id, i64 size){
  ܦܢܐ id->pMethods->xTruncate(id, size);
}
صحيح sqlite3OsSync(sqlite3_file *id, صحيح flags){
  DO_OS_MALLOC_TEST(id);
  ܦܢܐ flags ? id->pMethods->xSync(id, flags) : SQLITE_OK;
}
صحيح sqlite3OsFileSize(sqlite3_file *id, i64 *pSize){
  DO_OS_MALLOC_TEST(id);
  ܦܢܐ id->pMethods->xFileSize(id, pSize);
}
صحيح sqlite3OsLock(sqlite3_file *id, صحيح lockType){
  DO_OS_MALLOC_TEST(id);
  أكد( lockType>=SQLITE_LOCK_SHARED && lockType<=SQLITE_LOCK_EXCLUSIVE );
  ܦܢܐ id->pMethods->xLock(id, lockType);
}
صحيح sqlite3OsUnlock(sqlite3_file *id, صحيح lockType){
  أكد( lockType==SQLITE_LOCK_NONE || lockType==SQLITE_LOCK_SHARED );
  ܦܢܐ id->pMethods->xUnlock(id, lockType);
}
صحيح sqlite3OsCheckReservedLock(sqlite3_file *id, صحيح *pResOut){
  DO_OS_MALLOC_TEST(id);
  ܦܢܐ id->pMethods->xCheckReservedLock(id, pResOut);
}

/*
** Use sqlite3OsFileControl() when we are doing something that might fail
** and we need to know about the failures.  Use sqlite3OsFileControlHint()
** when simply tossing information over the wall to the VFS and we do not
** really care if the VFS receives and understands the information since it
** is only a hint and can be safely ignored.  The sqlite3OsFileControlHint()
** routine has no return value since the return value would be meaningless.
*/
صحيح sqlite3OsFileControl(sqlite3_file *id, صحيح op, فراغ *pArg){
  ܐܢ( id->pMethods==0 ) ܦܢܐ SQLITE_NOTFOUND;
#ifdef SQLITE_TEST
  ܐܢ( op!=SQLITE_FCNTL_COMMIT_PHASETWO
   && op!=SQLITE_FCNTL_LOCK_TIMEOUT
   && op!=SQLITE_FCNTL_CKPT_DONE
   && op!=SQLITE_FCNTL_CKPT_START
  ){
    /* Faults are not injected into COMMIT_PHASETWO because, assuming SQLite
    ** is using a regular VFS, it is called after the corresponding
    ** transaction has been committed. Injecting a fault at this point
    ** confuses the test scripts - the COMMIT command returns SQLITE_NOMEM
    ** but the transaction is committed anyway.
    **
    ** The core must call OsFileControl() though, not OsFileControlHint(),
    ** as if a custom VFS (e.g. zipvfs) returns an error here, it probably
    ** means the commit really has failed and an error should be returned
    ** to the user.
    **
    ** The CKPT_DONE and CKPT_START file-controls are write-only signals
    ** to the cksumvfs.  Their return code is meaningless and is ignored
    ** by the SQLite core, so there is no point in simulating OOMs for them.
    */
    DO_OS_MALLOC_TEST(id);
  }
#endif
  ܦܢܐ id->pMethods->xFileControl(id, op, pArg);
}
فراغ sqlite3OsFileControlHint(sqlite3_file *id, صحيح op, فراغ *pArg){
  ܐܢ( id->pMethods ) (فراغ)id->pMethods->xFileControl(id, op, pArg);
}

صحيح sqlite3OsSectorSize(sqlite3_file *id){
  صحيح (*xSectorSize)(sqlite3_file*) = id->pMethods->xSectorSize;
  ܦܢܐ (xSectorSize ? xSectorSize(id) : SQLITE_DEFAULT_SECTOR_SIZE);
}
صحيح sqlite3OsDeviceCharacteristics(sqlite3_file *id){
  ܐܢ( NEVER(id->pMethods==0) ) ܦܢܐ 0;
  ܦܢܐ id->pMethods->xDeviceCharacteristics(id);
}
#ifndef SQLITE_OMIT_WAL
صحيح sqlite3OsShmLock(sqlite3_file *id, صحيح offset, صحيح n, صحيح flags){
  ܦܢܐ id->pMethods->xShmLock(id, offset, n, flags);
}
فراغ sqlite3OsShmBarrier(sqlite3_file *id){
  id->pMethods->xShmBarrier(id);
}
صحيح sqlite3OsShmUnmap(sqlite3_file *id, صحيح deleteFlag){
  ܦܢܐ id->pMethods->xShmUnmap(id, deleteFlag);
}
صحيح sqlite3OsShmMap(
  sqlite3_file *id,               /* Database file handle */
  صحيح iPage,
  صحيح pgsz,
  صحيح bExtend,                    /* True to extend file if necessary */
  فراغ متطاير **pp              /* OUT: Pointer to mapping */
){
  DO_OS_MALLOC_TEST(id);
  ܦܢܐ id->pMethods->xShmMap(id, iPage, pgsz, bExtend, pp);
}
#endif /* SQLITE_OMIT_WAL */

#ܐܢ SQLITE_MAX_MMAP_SIZE>0
/* The real implementation of xFetch and xUnfetch */
صحيح sqlite3OsFetch(sqlite3_file *id, i64 iOff, صحيح iAmt, فراغ **pp){
  DO_OS_MALLOC_TEST(id);
  ܦܢܐ id->pMethods->xFetch(id, iOff, iAmt, pp);
}
صحيح sqlite3OsUnfetch(sqlite3_file *id, i64 iOff, فراغ *p){
  ܦܢܐ id->pMethods->xUnfetch(id, iOff, p);
}
#ܐܚܪܢܐ
/* No-op stubs to use when memory-mapped I/O is disabled */
صحيح sqlite3OsFetch(sqlite3_file *id, i64 iOff, صحيح iAmt, فراغ **pp){
  *pp = 0;
  ܦܢܐ SQLITE_OK;
}
صحيح sqlite3OsUnfetch(sqlite3_file *id, i64 iOff, فراغ *p){
  ܦܢܐ SQLITE_OK;
}
#endif

/*
** The next group of routines are convenience wrappers around the
** VFS methods.
*/
صحيح sqlite3OsOpen(
  sqlite3_vfs *pVfs,
  ثابت حرف *zPath,
  sqlite3_file *pFile,
  صحيح flags,
  صحيح *pFlagsOut
){
  صحيح rc;
  DO_OS_MALLOC_TEST(0);
  /* 0x87f7f is a mask of SQLITE_OPEN_ flags that are valid to be passed
  ** down into the VFS layer.  Some SQLITE_OPEN_ flags (for example,
  ** SQLITE_OPEN_FULLMUTEX or SQLITE_OPEN_SHAREDCACHE) are blocked before
  ** reaching the VFS. */
  أكد( zPath || (flags & SQLITE_OPEN_EXCLUSIVE) );
  rc = pVfs->xOpen(pVfs, zPath, pFile, flags & 0x1087f7f, pFlagsOut);
  أكد( rc==SQLITE_OK || pFile->pMethods==0 );
  ܦܢܐ rc;
}
صحيح sqlite3OsDelete(sqlite3_vfs *pVfs, ثابت حرف *zPath, صحيح dirSync){
  DO_OS_MALLOC_TEST(0);
  أكد( dirSync==0 || dirSync==1 );
  ܦܢܐ pVfs->xDelete!=0 ? pVfs->xDelete(pVfs, zPath, dirSync) : SQLITE_OK;
}
صحيح sqlite3OsAccess(
  sqlite3_vfs *pVfs,
  ثابت حرف *zPath,
  صحيح flags,
  صحيح *pResOut
){
  DO_OS_MALLOC_TEST(0);
  ܦܢܐ pVfs->xAccess(pVfs, zPath, flags, pResOut);
}
صحيح sqlite3OsFullPathname(
  sqlite3_vfs *pVfs,
  ثابت حرف *zPath,
  صحيح nPathOut,
  حرف *zPathOut
){
  DO_OS_MALLOC_TEST(0);
  zPathOut[0] = 0;
  ܦܢܐ pVfs->xFullPathname(pVfs, zPath, nPathOut, zPathOut);
}
#ifndef SQLITE_OMIT_LOAD_EXTENSION
فراغ *sqlite3OsDlOpen(sqlite3_vfs *pVfs, ثابت حرف *zPath){
  أكد( zPath!=0 );
  أكد( strlen(zPath)<=SQLITE_MAX_PATHLEN );  /* tag-20210611-1 */
  ܦܢܐ pVfs->xDlOpen(pVfs, zPath);
}
فراغ sqlite3OsDlError(sqlite3_vfs *pVfs, صحيح nByte, حرف *zBufOut){
  pVfs->xDlError(pVfs, nByte, zBufOut);
}
فراغ (*sqlite3OsDlSym(sqlite3_vfs *pVfs, فراغ *pHdle, ثابت حرف *zSym))(فراغ){
  ܦܢܐ pVfs->xDlSym(pVfs, pHdle, zSym);
}
فراغ sqlite3OsDlClose(sqlite3_vfs *pVfs, فراغ *pHandle){
  pVfs->xDlClose(pVfs, pHandle);
}
#endif /* SQLITE_OMIT_LOAD_EXTENSION */
صحيح sqlite3OsRandomness(sqlite3_vfs *pVfs, صحيح nByte, حرف *zBufOut){
  ܐܢ( sqlite3Config.iPrngSeed ){
    memset(zBufOut, 0, nByte);
    ܐܢ( ALWAYS(nByte>(signed)حجم(unsigned)) ) nByte = حجم(unsigned صحيح);
    memcpy(zBufOut, &sqlite3Config.iPrngSeed, nByte);
    ܦܢܐ SQLITE_OK;
  }ܐܚܪܢܐ{
    ܦܢܐ pVfs->xRandomness(pVfs, nByte, zBufOut);
  }

}
صحيح sqlite3OsSleep(sqlite3_vfs *pVfs, صحيح nMicro){
  ܦܢܐ pVfs->xSleep(pVfs, nMicro);
}
صحيح sqlite3OsGetLastError(sqlite3_vfs *pVfs){
  ܦܢܐ pVfs->xGetLastError ? pVfs->xGetLastError(pVfs, 0, 0) : 0;
}
صحيح sqlite3OsCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *pTimeOut){
  صحيح rc;
  /* IMPLEMENTATION-OF: R-49045-42493 SQLite will use the xCurrentTimeInt64()
  ** method to get the current date and time if that method is available
  ** (if iVersion is 2 or greater and the function pointer is not NULL) and
  ** will fall back to xCurrentTime() if xCurrentTimeInt64() is
  ** unavailable.
  */
  ܐܢ( pVfs->iVersion>=2 && pVfs->xCurrentTimeInt64 ){
    rc = pVfs->xCurrentTimeInt64(pVfs, pTimeOut);
  }ܐܚܪܢܐ{
    مزدوج r;
    rc = pVfs->xCurrentTime(pVfs, &r);
    *pTimeOut = sqlite3RealToI64(r*86400000.0);
  }
  ܦܢܐ rc;
}

صحيح sqlite3OsOpenMalloc(
  sqlite3_vfs *pVfs,
  ثابت حرف *zFile,
  sqlite3_file **ppFile,
  صحيح flags,
  صحيح *pOutFlags
){
  صحيح rc;
  sqlite3_file *pFile;
  pFile = (sqlite3_file *)sqlite3MallocZero(pVfs->szOsFile);
  ܐܢ( pFile ){
    rc = sqlite3OsOpen(pVfs, zFile, pFile, flags, pOutFlags);
    ܐܢ( rc!=SQLITE_OK ){
      sqlite3_free(pFile);
      *ppFile = 0;
    }ܐܚܪܢܐ{
      *ppFile = pFile;
    }
  }ܐܚܪܢܐ{
    *ppFile = 0;
    rc = SQLITE_NOMEM_BKPT;
  }
  أكد( *ppFile!=0 || rc!=SQLITE_OK );
  ܦܢܐ rc;
}
فراغ sqlite3OsCloseFree(sqlite3_file *pFile){
  أكد( pFile );
  sqlite3OsClose(pFile);
  sqlite3_free(pFile);
}

/*
** This function is a wrapper around the OS specific implementation of
** sqlite3_os_init(). The purpose of the wrapper is to provide the
** ability to simulate a malloc failure, so that the handling of an
** error in sqlite3_os_init() by the upper layers can be tested.
*/
صحيح sqlite3OsInit(فراغ){
  فراغ *p = sqlite3_malloc(10);
  ܐܢ( p==0 ) ܦܢܐ SQLITE_NOMEM_BKPT;
  sqlite3_free(p);
  ܦܢܐ sqlite3_os_init();
}

/*
** The list of all registered VFS implementations.
*/
ساكن sqlite3_vfs * SQLITE_WSD vfsList = 0;
#define vfsList GLOBAL(sqlite3_vfs *, vfsList)

/*
** Locate a VFS by name.  If no name is given, simply return the
** first VFS on the list.
*/
sqlite3_vfs *sqlite3_vfs_find(ثابت حرف *zVfs){
  sqlite3_vfs *pVfs = 0;
#ܐܢ SQLITE_THREADSAFE
  sqlite3_mutex *mutex;
#endif
#ifndef SQLITE_OMIT_AUTOINIT
  صحيح rc = sqlite3_initialize();
  ܐܢ( rc ) ܦܢܐ 0;
#endif
#ܐܢ SQLITE_THREADSAFE
  mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
#endif
  sqlite3_mutex_enter(mutex);
  لكل(pVfs = vfsList; pVfs; pVfs=pVfs->pNext){
    ܐܢ( zVfs==0 ) اكسر;
    ܐܢ( strcmp(zVfs, pVfs->zName)==0 ) اكسر;
  }
  sqlite3_mutex_leave(mutex);
  ܦܢܐ pVfs;
}

/*
** Unlink a VFS from the linked list
*/
ساكن فراغ vfsUnlink(sqlite3_vfs *pVfs){
  أكد( sqlite3_mutex_held(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN)) );
  ܐܢ( pVfs==0 ){
    /* No-op */
  }ܐܚܪܢܐ ܐܢ( vfsList==pVfs ){
    vfsList = pVfs->pNext;
  }ܐܚܪܢܐ ܐܢ( vfsList ){
    sqlite3_vfs *p = vfsList;
    ܟܡܐ( p->pNext && p->pNext!=pVfs ){
      p = p->pNext;
    }
    ܐܢ( p->pNext==pVfs ){
      p->pNext = pVfs->pNext;
    }
  }
}

/*
** Register a VFS with the system.  It is harmless to register the same
** VFS multiple times.  The new VFS becomes the default if makeDflt is
** true.
*/
صحيح sqlite3_vfs_register(sqlite3_vfs *pVfs, صحيح makeDflt){
  MUTEX_LOGIC(sqlite3_mutex *mutex;)
#ifndef SQLITE_OMIT_AUTOINIT
  صحيح rc = sqlite3_initialize();
  ܐܢ( rc ) ܦܢܐ rc;
#endif
#ifdef SQLITE_ENABLE_API_ARMOR
  ܐܢ( pVfs==0 ) ܦܢܐ SQLITE_MISUSE_BKPT;
#endif

  MUTEX_LOGIC( mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  sqlite3_mutex_enter(mutex);
  vfsUnlink(pVfs);
  ܐܢ( makeDflt || vfsList==0 ){
    pVfs->pNext = vfsList;
    vfsList = pVfs;
  }ܐܚܪܢܐ{
    pVfs->pNext = vfsList->pNext;
    vfsList->pNext = pVfs;
  }
  أكد(vfsList);
  sqlite3_mutex_leave(mutex);
  ܦܢܐ SQLITE_OK;
}

/*
** Unregister a VFS so that it is no longer accessible.
*/
صحيح sqlite3_vfs_unregister(sqlite3_vfs *pVfs){
  MUTEX_LOGIC(sqlite3_mutex *mutex;)
#ifndef SQLITE_OMIT_AUTOINIT
  صحيح rc = sqlite3_initialize();
  ܐܢ( rc ) ܦܢܐ rc;
#endif
  MUTEX_LOGIC( mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  sqlite3_mutex_enter(mutex);
  vfsUnlink(pVfs);
  sqlite3_mutex_leave(mutex);
  ܦܢܐ SQLITE_OK;
}
