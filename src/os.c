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
#شامل "sqliteInt.h"

/*
** If we compile with the SQLITE_TEST macro set, then the following block
** of code will give us the ability to simulate a disk I/O error.  This
** is used for testing the I/O recovery logic.
*/
#اگر defined(SQLITE_TEST)
صحیح sqlite3_io_error_hit = 0;            /* Total number of I/O Errors */
صحیح sqlite3_io_error_hardhit = 0;        /* Number of non-benign errors */
صحیح sqlite3_io_error_pending = 0;        /* Count down to first I/O error */
صحیح sqlite3_io_error_persist = 0;        /* True if I/O errors persist */
صحیح sqlite3_io_error_benign = 0;         /* True if errors are benign */
صحیح sqlite3_diskfull_pending = 0;
صحیح sqlite3_diskfull = 0;
#endif /* defined(SQLITE_TEST) */

/*
** When testing, also keep a count of the number of open files.
*/
#اگر defined(SQLITE_TEST)
صحیح sqlite3_open_file_شمار = 0;
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
#اگر defined(SQLITE_TEST)
صحیح sqlite3_memdebug_vfs_oom_test = 1;
  #define DO_OS_MALLOC_TEST(x)                                       \
  اگر (sqlite3_memdebug_vfs_oom_test && (!x || !sqlite3JournalIsInMemory(x))) { \
    تهی *pTstAlloc = sqlite3Malloc(10);                             \
    اگر (!pTstAlloc) بازگشت SQLITE_IOERR_NOMEM_BKPT;                  \
    sqlite3_free(pTstAlloc);                                         \
  }
#وگرنه
  #define DO_OS_MALLOC_TEST(x)
#endif

/*
** The following routines are convenience wrappers around methods
** of the sqlite3_file object.  This is mostly just syntactic sugar. All
** of this would be completely automatic if SQLite were coded using
** C++ instead of plain old C.
*/
تهی sqlite3OsClose(sqlite3_file *pId){
  اگر( pId->pMethods ){
    pId->pMethods->xClose(pId);
    pId->pMethods = 0;
  }
}
صحیح sqlite3OsRead(sqlite3_file *id, تهی *pBuf, صحیح amt, i64 offset){
  DO_OS_MALLOC_TEST(id);
  بازگشت id->pMethods->xRead(id, pBuf, amt, offset);
}
صحیح sqlite3OsWrite(sqlite3_file *id, ثابت تهی *pBuf, صحیح amt, i64 offset){
  DO_OS_MALLOC_TEST(id);
  بازگشت id->pMethods->xWrite(id, pBuf, amt, offset);
}
صحیح sqlite3OsTruncate(sqlite3_file *id, i64 size){
  بازگشت id->pMethods->xTruncate(id, size);
}
صحیح sqlite3OsSync(sqlite3_file *id, صحیح flags){
  DO_OS_MALLOC_TEST(id);
  بازگشت flags ? id->pMethods->xSync(id, flags) : SQLITE_OK;
}
صحیح sqlite3OsFileSize(sqlite3_file *id, i64 *pSize){
  DO_OS_MALLOC_TEST(id);
  بازگشت id->pMethods->xFileSize(id, pSize);
}
صحیح sqlite3OsLock(sqlite3_file *id, صحیح lockType){
  DO_OS_MALLOC_TEST(id);
  أكد( lockType>=SQLITE_LOCK_SHARED && lockType<=SQLITE_LOCK_EXCLUSIVE );
  بازگشت id->pMethods->xLock(id, lockType);
}
صحیح sqlite3OsUnlock(sqlite3_file *id, صحیح lockType){
  أكد( lockType==SQLITE_LOCK_NONE || lockType==SQLITE_LOCK_SHARED );
  بازگشت id->pMethods->xUnlock(id, lockType);
}
صحیح sqlite3OsCheckReservedLock(sqlite3_file *id, صحیح *pResOut){
  DO_OS_MALLOC_TEST(id);
  بازگشت id->pMethods->xCheckReservedLock(id, pResOut);
}

/*
** Use sqlite3OsFileControl() when we are doing something that might fail
** and we need to know about the failures.  Use sqlite3OsFileControlHint()
** when simply tossing information over the wall to the VFS and we do not
** really care if the VFS receives and understands the information since it
** is only a hint and can be safely ignored.  The sqlite3OsFileControlHint()
** routine has no return value since the return value would be meaningless.
*/
صحیح sqlite3OsFileControl(sqlite3_file *id, صحیح op, تهی *pArg){
  اگر( id->pMethods==0 ) بازگشت SQLITE_NOTFOUND;
#ifdef SQLITE_TEST
  اگر( op!=SQLITE_FCNTL_COMMIT_PHASETWO
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
  بازگشت id->pMethods->xFileControl(id, op, pArg);
}
تهی sqlite3OsFileControlHint(sqlite3_file *id, صحیح op, تهی *pArg){
  اگر( id->pMethods ) (تهی)id->pMethods->xFileControl(id, op, pArg);
}

صحیح sqlite3OsSectorSize(sqlite3_file *id){
  صحیح (*xSectorSize)(sqlite3_file*) = id->pMethods->xSectorSize;
  بازگشت (xSectorSize ? xSectorSize(id) : SQLITE_DEFAULT_SECTOR_SIZE);
}
صحیح sqlite3OsDeviceCharacteristics(sqlite3_file *id){
  اگر( NEVER(id->pMethods==0) ) بازگشت 0;
  بازگشت id->pMethods->xDeviceCharacteristics(id);
}
#ifndef SQLITE_OMIT_WAL
صحیح sqlite3OsShmLock(sqlite3_file *id, صحیح offset, صحیح n, صحیح flags){
  بازگشت id->pMethods->xShmLock(id, offset, n, flags);
}
تهی sqlite3OsShmBarrier(sqlite3_file *id){
  id->pMethods->xShmBarrier(id);
}
صحیح sqlite3OsShmUnmap(sqlite3_file *id, صحیح deleteFlag){
  بازگشت id->pMethods->xShmUnmap(id, deleteFlag);
}
صحیح sqlite3OsShmMap(
  sqlite3_file *id,               /* Database file handle */
  صحیح iPage,
  صحیح pgsz,
  صحیح bExtend,                    /* True to extend file if necessary */
  تهی فرار **pp              /* OUT: Pointer to mapping */
){
  DO_OS_MALLOC_TEST(id);
  بازگشت id->pMethods->xShmMap(id, iPage, pgsz, bExtend, pp);
}
#endif /* SQLITE_OMIT_WAL */

#اگر SQLITE_MAX_MMAP_SIZE>0
/* The real implementation of xFetch and xUnfetch */
صحیح sqlite3OsFetch(sqlite3_file *id, i64 iOff, صحیح iAmt, تهی **pp){
  DO_OS_MALLOC_TEST(id);
  بازگشت id->pMethods->xFetch(id, iOff, iAmt, pp);
}
صحیح sqlite3OsUnfetch(sqlite3_file *id, i64 iOff, تهی *p){
  بازگشت id->pMethods->xUnfetch(id, iOff, p);
}
#وگرنه
/* No-op stubs to use when memory-mapped I/O is disabled */
صحیح sqlite3OsFetch(sqlite3_file *id, i64 iOff, صحیح iAmt, تهی **pp){
  *pp = 0;
  بازگشت SQLITE_OK;
}
صحیح sqlite3OsUnfetch(sqlite3_file *id, i64 iOff, تهی *p){
  بازگشت SQLITE_OK;
}
#endif

/*
** The next group of routines are convenience wrappers around the
** VFS methods.
*/
صحیح sqlite3OsOpen(
  sqlite3_vfs *pVfs,
  ثابت نویسه *zPath,
  sqlite3_file *pFile,
  صحیح flags,
  صحیح *pFlagsOut
){
  صحیح rc;
  DO_OS_MALLOC_TEST(0);
  /* 0x87f7f is a mask of SQLITE_OPEN_ flags that are valid to be passed
  ** down into the VFS layer.  Some SQLITE_OPEN_ flags (for example,
  ** SQLITE_OPEN_FULLMUTEX or SQLITE_OPEN_SHAREDCACHE) are blocked before
  ** reaching the VFS. */
  أكد( zPath || (flags & SQLITE_OPEN_EXCLUSIVE) );
  rc = pVfs->xOpen(pVfs, zPath, pFile, flags & 0x1087f7f, pFlagsOut);
  أكد( rc==SQLITE_OK || pFile->pMethods==0 );
  بازگشت rc;
}
صحیح sqlite3OsDelete(sqlite3_vfs *pVfs, ثابت نویسه *zPath, صحیح dirSync){
  DO_OS_MALLOC_TEST(0);
  أكد( dirSync==0 || dirSync==1 );
  بازگشت pVfs->xDelete!=0 ? pVfs->xDelete(pVfs, zPath, dirSync) : SQLITE_OK;
}
صحیح sqlite3OsAccess(
  sqlite3_vfs *pVfs,
  ثابت نویسه *zPath,
  صحیح flags,
  صحیح *pResOut
){
  DO_OS_MALLOC_TEST(0);
  بازگشت pVfs->xAccess(pVfs, zPath, flags, pResOut);
}
صحیح sqlite3OsFullPathname(
  sqlite3_vfs *pVfs,
  ثابت نویسه *zPath,
  صحیح nPathOut,
  نویسه *zPathOut
){
  DO_OS_MALLOC_TEST(0);
  zPathOut[0] = 0;
  بازگشت pVfs->xFullPathname(pVfs, zPath, nPathOut, zPathOut);
}
#ifndef SQLITE_OMIT_LOAD_EXTENSION
تهی *sqlite3OsDlOpen(sqlite3_vfs *pVfs, ثابت نویسه *zPath){
  أكد( zPath!=0 );
  أكد( strlen(zPath)<=SQLITE_MAX_PATHLEN );  /* tag-20210611-1 */
  بازگشت pVfs->xDlOpen(pVfs, zPath);
}
تهی sqlite3OsDlError(sqlite3_vfs *pVfs, صحیح nByte, نویسه *zBufOut){
  pVfs->xDlError(pVfs, nByte, zBufOut);
}
تهی (*sqlite3OsDlSym(sqlite3_vfs *pVfs, تهی *pHdle, ثابت نویسه *zSym))(تهی){
  بازگشت pVfs->xDlSym(pVfs, pHdle, zSym);
}
تهی sqlite3OsDlClose(sqlite3_vfs *pVfs, تهی *pHandle){
  pVfs->xDlClose(pVfs, pHandle);
}
#endif /* SQLITE_OMIT_LOAD_EXTENSION */
صحیح sqlite3OsRandomness(sqlite3_vfs *pVfs, صحیح nByte, نویسه *zBufOut){
  اگر( sqlite3Config.iPrngSeed ){
    memset(zBufOut, 0, nByte);
    اگر( ALWAYS(nByte>(signed)اندازه(unsigned)) ) nByte = اندازه(unsigned صحیح);
    memcpy(zBufOut, &sqlite3Config.iPrngSeed, nByte);
    بازگشت SQLITE_OK;
  }وگرنه{
    بازگشت pVfs->xRandomness(pVfs, nByte, zBufOut);
  }

}
صحیح sqlite3OsSleep(sqlite3_vfs *pVfs, صحیح nMicro){
  بازگشت pVfs->xSleep(pVfs, nMicro);
}
صحیح sqlite3OsGetLastError(sqlite3_vfs *pVfs){
  بازگشت pVfs->xGetLastError ? pVfs->xGetLastError(pVfs, 0, 0) : 0;
}
صحیح sqlite3OsCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *pTimeOut){
  صحیح rc;
  /* IMPLEMENTATION-OF: R-49045-42493 SQLite will use the xCurrentTimeInt64()
  ** method to get the current date and time if that method is available
  ** (if iVersion is 2 or greater and the function pointer is not NULL) and
  ** will fall back to xCurrentTime() if xCurrentTimeInt64() is
  ** unavailable.
  */
  اگر( pVfs->iVersion>=2 && pVfs->xCurrentTimeInt64 ){
    rc = pVfs->xCurrentTimeInt64(pVfs, pTimeOut);
  }وگرنه{
    دوگانه r;
    rc = pVfs->xCurrentTime(pVfs, &r);
    *pTimeOut = sqlite3RealToI64(r*86400000.0);
  }
  بازگشت rc;
}

صحیح sqlite3OsOpenMalloc(
  sqlite3_vfs *pVfs,
  ثابت نویسه *zFile,
  sqlite3_file **ppFile,
  صحیح flags,
  صحیح *pOutFlags
){
  صحیح rc;
  sqlite3_file *pFile;
  pFile = (sqlite3_file *)sqlite3MallocZero(pVfs->szOsFile);
  اگر( pFile ){
    rc = sqlite3OsOpen(pVfs, zFile, pFile, flags, pOutFlags);
    اگر( rc!=SQLITE_OK ){
      sqlite3_free(pFile);
      *ppFile = 0;
    }وگرنه{
      *ppFile = pFile;
    }
  }وگرنه{
    *ppFile = 0;
    rc = SQLITE_NOMEM_BKPT;
  }
  أكد( *ppFile!=0 || rc!=SQLITE_OK );
  بازگشت rc;
}
تهی sqlite3OsCloseFree(sqlite3_file *pFile){
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
صحیح sqlite3OsInit(تهی){
  تهی *p = sqlite3_malloc(10);
  اگر( p==0 ) بازگشت SQLITE_NOMEM_BKPT;
  sqlite3_free(p);
  بازگشت sqlite3_os_init();
}

/*
** The list of all registered VFS implementations.
*/
ایستا sqlite3_vfs * SQLITE_WSD vfsList = 0;
#define vfsList GLOBAL(sqlite3_vfs *, vfsList)

/*
** Locate a VFS by name.  If no name is given, simply return the
** first VFS on the list.
*/
sqlite3_vfs *sqlite3_vfs_find(ثابت نویسه *zVfs){
  sqlite3_vfs *pVfs = 0;
#اگر SQLITE_THREADSAFE
  sqlite3_mutex *mutex;
#endif
#ifndef SQLITE_OMIT_AUTOINIT
  صحیح rc = sqlite3_initialize();
  اگر( rc ) بازگشت 0;
#endif
#اگر SQLITE_THREADSAFE
  mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
#endif
  sqlite3_mutex_enter(mutex);
  برای(pVfs = vfsList; pVfs; pVfs=pVfs->pNext){
    اگر( zVfs==0 ) بشکن;
    اگر( strcmp(zVfs, pVfs->zName)==0 ) بشکن;
  }
  sqlite3_mutex_leave(mutex);
  بازگشت pVfs;
}

/*
** Unlink a VFS from the linked list
*/
ایستا تهی vfsUnlink(sqlite3_vfs *pVfs){
  أكد( sqlite3_mutex_held(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN)) );
  اگر( pVfs==0 ){
    /* No-op */
  }وگرنه اگر( vfsList==pVfs ){
    vfsList = pVfs->pNext;
  }وگرنه اگر( vfsList ){
    sqlite3_vfs *p = vfsList;
    وقتی( p->pNext && p->pNext!=pVfs ){
      p = p->pNext;
    }
    اگر( p->pNext==pVfs ){
      p->pNext = pVfs->pNext;
    }
  }
}

/*
** Register a VFS with the system.  It is harmless to register the same
** VFS multiple times.  The new VFS becomes the default if makeDflt is
** true.
*/
صحیح sqlite3_vfs_register(sqlite3_vfs *pVfs, صحیح makeDflt){
  MUTEX_LOGIC(sqlite3_mutex *mutex;)
#ifndef SQLITE_OMIT_AUTOINIT
  صحیح rc = sqlite3_initialize();
  اگر( rc ) بازگشت rc;
#endif
#ifdef SQLITE_ENABLE_API_ARMOR
  اگر( pVfs==0 ) بازگشت SQLITE_MISUSE_BKPT;
#endif

  MUTEX_LOGIC( mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  sqlite3_mutex_enter(mutex);
  vfsUnlink(pVfs);
  اگر( makeDflt || vfsList==0 ){
    pVfs->pNext = vfsList;
    vfsList = pVfs;
  }وگرنه{
    pVfs->pNext = vfsList->pNext;
    vfsList->pNext = pVfs;
  }
  أكد(vfsList);
  sqlite3_mutex_leave(mutex);
  بازگشت SQLITE_OK;
}

/*
** Unregister a VFS so that it is no longer accessible.
*/
صحیح sqlite3_vfs_unregister(sqlite3_vfs *pVfs){
  MUTEX_LOGIC(sqlite3_mutex *mutex;)
#ifndef SQLITE_OMIT_AUTOINIT
  صحیح rc = sqlite3_initialize();
  اگر( rc ) بازگشت rc;
#endif
  MUTEX_LOGIC( mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  sqlite3_mutex_enter(mutex);
  vfsUnlink(pVfs);
  sqlite3_mutex_leave(mutex);
  بازگشت SQLITE_OK;
}
