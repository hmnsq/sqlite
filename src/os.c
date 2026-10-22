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
#অন্তর্ভাৱ "sqliteInt.h"

/*
** If we compile with the SQLITE_TEST macro set, then the following block
** of code will give us the ability to simulate a disk I/O error.  This
** is used for testing the I/O recovery logic.
*/
#যদি defined(SQLITE_TEST)
পূর্ণাঙ্ক sqlite3_io_error_hit = 0;            /* Total number of I/O Errors */
পূর্ণাঙ্ক sqlite3_io_error_hardhit = 0;        /* Number of non-benign errors */
পূর্ণাঙ্ক sqlite3_io_error_pending = 0;        /* Count down to first I/O error */
পূর্ণাঙ্ক sqlite3_io_error_persist = 0;        /* True if I/O errors persist */
পূর্ণাঙ্ক sqlite3_io_error_benign = 0;         /* True if errors are benign */
পূর্ণাঙ্ক sqlite3_diskfull_pending = 0;
পূর্ণাঙ্ক sqlite3_diskfull = 0;
#endif /* defined(SQLITE_TEST) */

/*
** When testing, also keep a count of the number of open files.
*/
#যদি defined(SQLITE_TEST)
পূর্ণাঙ্ক sqlite3_open_file_গুনতি = 0;
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
#যদি defined(SQLITE_TEST)
পূর্ণাঙ্ক sqlite3_memdebug_vfs_oom_test = 1;
  #define DO_OS_MALLOC_TEST(x)                                       \
  যদি (sqlite3_memdebug_vfs_oom_test && (!x || !sqlite3JournalIsInMemory(x))) { \
    শূন্য *pTstAlloc = sqlite3Malloc(10);                             \
    যদি (!pTstAlloc) ঘূৰাই SQLITE_IOERR_NOMEM_BKPT;                  \
    sqlite3_free(pTstAlloc);                                         \
  }
#অন্যথা
  #define DO_OS_MALLOC_TEST(x)
#endif

/*
** The following routines are convenience wrappers around methods
** of the sqlite3_file object.  This is mostly just syntactic sugar. All
** of this would be completely automatic if SQLite were coded using
** C++ instead of plain old C.
*/
শূন্য sqlite3OsClose(sqlite3_file *pId){
  যদি( pId->pMethods ){
    pId->pMethods->xClose(pId);
    pId->pMethods = 0;
  }
}
পূর্ণাঙ্ক sqlite3OsRead(sqlite3_file *id, শূন্য *pBuf, পূর্ণাঙ্ক amt, i64 offset){
  DO_OS_MALLOC_TEST(id);
  ঘূৰাই id->pMethods->xRead(id, pBuf, amt, offset);
}
পূর্ণাঙ্ক sqlite3OsWrite(sqlite3_file *id, ধ্রুৱ শূন্য *pBuf, পূর্ণাঙ্ক amt, i64 offset){
  DO_OS_MALLOC_TEST(id);
  ঘূৰাই id->pMethods->xWrite(id, pBuf, amt, offset);
}
পূর্ণাঙ্ক sqlite3OsTruncate(sqlite3_file *id, i64 size){
  ঘূৰাই id->pMethods->xTruncate(id, size);
}
পূর্ণাঙ্ক sqlite3OsSync(sqlite3_file *id, পূর্ণাঙ্ক flags){
  DO_OS_MALLOC_TEST(id);
  ঘূৰাই flags ? id->pMethods->xSync(id, flags) : SQLITE_OK;
}
পূর্ণাঙ্ক sqlite3OsFileSize(sqlite3_file *id, i64 *pSize){
  DO_OS_MALLOC_TEST(id);
  ঘূৰাই id->pMethods->xFileSize(id, pSize);
}
পূর্ণাঙ্ক sqlite3OsLock(sqlite3_file *id, পূর্ণাঙ্ক lockType){
  DO_OS_MALLOC_TEST(id);
  প্রতিজ্ঞা( lockType>=SQLITE_LOCK_SHARED && lockType<=SQLITE_LOCK_EXCLUSIVE );
  ঘূৰাই id->pMethods->xLock(id, lockType);
}
পূর্ণাঙ্ক sqlite3OsUnlock(sqlite3_file *id, পূর্ণাঙ্ক lockType){
  প্রতিজ্ঞা( lockType==SQLITE_LOCK_NONE || lockType==SQLITE_LOCK_SHARED );
  ঘূৰাই id->pMethods->xUnlock(id, lockType);
}
পূর্ণাঙ্ক sqlite3OsCheckReservedLock(sqlite3_file *id, পূর্ণাঙ্ক *pResOut){
  DO_OS_MALLOC_TEST(id);
  ঘূৰাই id->pMethods->xCheckReservedLock(id, pResOut);
}

/*
** Use sqlite3OsFileControl() when we are doing something that might fail
** and we need to know about the failures.  Use sqlite3OsFileControlHint()
** when simply tossing information over the wall to the VFS and we do not
** really care if the VFS receives and understands the information since it
** is only a hint and can be safely ignored.  The sqlite3OsFileControlHint()
** routine has no return value since the return value would be meaningless.
*/
পূর্ণাঙ্ক sqlite3OsFileControl(sqlite3_file *id, পূর্ণাঙ্ক op, শূন্য *pArg){
  যদি( id->pMethods==0 ) ঘূৰাই SQLITE_NOTFOUND;
#ifdef SQLITE_TEST
  যদি( op!=SQLITE_FCNTL_COMMIT_PHASETWO
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
  ঘূৰাই id->pMethods->xFileControl(id, op, pArg);
}
শূন্য sqlite3OsFileControlHint(sqlite3_file *id, পূর্ণাঙ্ক op, শূন্য *pArg){
  যদি( id->pMethods ) (শূন্য)id->pMethods->xFileControl(id, op, pArg);
}

পূর্ণাঙ্ক sqlite3OsSectorSize(sqlite3_file *id){
  পূর্ণাঙ্ক (*xSectorSize)(sqlite3_file*) = id->pMethods->xSectorSize;
  ঘূৰাই (xSectorSize ? xSectorSize(id) : SQLITE_DEFAULT_SECTOR_SIZE);
}
পূর্ণাঙ্ক sqlite3OsDeviceCharacteristics(sqlite3_file *id){
  যদি( NEVER(id->pMethods==0) ) ঘূৰাই 0;
  ঘূৰাই id->pMethods->xDeviceCharacteristics(id);
}
#ifndef SQLITE_OMIT_WAL
পূর্ণাঙ্ক sqlite3OsShmLock(sqlite3_file *id, পূর্ণাঙ্ক offset, পূর্ণাঙ্ক n, পূর্ণাঙ্ক flags){
  ঘূৰাই id->pMethods->xShmLock(id, offset, n, flags);
}
শূন্য sqlite3OsShmBarrier(sqlite3_file *id){
  id->pMethods->xShmBarrier(id);
}
পূর্ণাঙ্ক sqlite3OsShmUnmap(sqlite3_file *id, পূর্ণাঙ্ক deleteFlag){
  ঘূৰাই id->pMethods->xShmUnmap(id, deleteFlag);
}
পূর্ণাঙ্ক sqlite3OsShmMap(
  sqlite3_file *id,               /* Database file handle */
  পূর্ণাঙ্ক iPage,
  পূর্ণাঙ্ক pgsz,
  পূর্ণাঙ্ক bExtend,                    /* True to extend file if necessary */
  শূন্য চঞ্চল **pp              /* OUT: Pointer to mapping */
){
  DO_OS_MALLOC_TEST(id);
  ঘূৰাই id->pMethods->xShmMap(id, iPage, pgsz, bExtend, pp);
}
#endif /* SQLITE_OMIT_WAL */

#যদি SQLITE_MAX_MMAP_SIZE>0
/* The real implementation of xFetch and xUnfetch */
পূর্ণাঙ্ক sqlite3OsFetch(sqlite3_file *id, i64 iOff, পূর্ণাঙ্ক iAmt, শূন্য **pp){
  DO_OS_MALLOC_TEST(id);
  ঘূৰাই id->pMethods->xFetch(id, iOff, iAmt, pp);
}
পূর্ণাঙ্ক sqlite3OsUnfetch(sqlite3_file *id, i64 iOff, শূন্য *p){
  ঘূৰাই id->pMethods->xUnfetch(id, iOff, p);
}
#অন্যথা
/* No-op stubs to use when memory-mapped I/O is disabled */
পূর্ণাঙ্ক sqlite3OsFetch(sqlite3_file *id, i64 iOff, পূর্ণাঙ্ক iAmt, শূন্য **pp){
  *pp = 0;
  ঘূৰাই SQLITE_OK;
}
পূর্ণাঙ্ক sqlite3OsUnfetch(sqlite3_file *id, i64 iOff, শূন্য *p){
  ঘূৰাই SQLITE_OK;
}
#endif

/*
** The next group of routines are convenience wrappers around the
** VFS methods.
*/
পূর্ণাঙ্ক sqlite3OsOpen(
  sqlite3_vfs *pVfs,
  ধ্রুৱ ৱর্ণ *zPath,
  sqlite3_file *pFile,
  পূর্ণাঙ্ক flags,
  পূর্ণাঙ্ক *pFlagsOut
){
  পূর্ণাঙ্ক rc;
  DO_OS_MALLOC_TEST(0);
  /* 0x87f7f is a mask of SQLITE_OPEN_ flags that are valid to be passed
  ** down into the VFS layer.  Some SQLITE_OPEN_ flags (for example,
  ** SQLITE_OPEN_FULLMUTEX or SQLITE_OPEN_SHAREDCACHE) are blocked before
  ** reaching the VFS. */
  প্রতিজ্ঞা( zPath || (flags & SQLITE_OPEN_EXCLUSIVE) );
  rc = pVfs->xOpen(pVfs, zPath, pFile, flags & 0x1087f7f, pFlagsOut);
  প্রতিজ্ঞা( rc==SQLITE_OK || pFile->pMethods==0 );
  ঘূৰাই rc;
}
পূর্ণাঙ্ক sqlite3OsDelete(sqlite3_vfs *pVfs, ধ্রুৱ ৱর্ণ *zPath, পূর্ণাঙ্ক dirSync){
  DO_OS_MALLOC_TEST(0);
  প্রতিজ্ঞা( dirSync==0 || dirSync==1 );
  ঘূৰাই pVfs->xDelete!=0 ? pVfs->xDelete(pVfs, zPath, dirSync) : SQLITE_OK;
}
পূর্ণাঙ্ক sqlite3OsAccess(
  sqlite3_vfs *pVfs,
  ধ্রুৱ ৱর্ণ *zPath,
  পূর্ণাঙ্ক flags,
  পূর্ণাঙ্ক *pResOut
){
  DO_OS_MALLOC_TEST(0);
  ঘূৰাই pVfs->xAccess(pVfs, zPath, flags, pResOut);
}
পূর্ণাঙ্ক sqlite3OsFullPathname(
  sqlite3_vfs *pVfs,
  ধ্রুৱ ৱর্ণ *zPath,
  পূর্ণাঙ্ক nPathOut,
  ৱর্ণ *zPathOut
){
  DO_OS_MALLOC_TEST(0);
  zPathOut[0] = 0;
  ঘূৰাই pVfs->xFullPathname(pVfs, zPath, nPathOut, zPathOut);
}
#ifndef SQLITE_OMIT_LOAD_EXTENSION
শূন্য *sqlite3OsDlOpen(sqlite3_vfs *pVfs, ধ্রুৱ ৱর্ণ *zPath){
  প্রতিজ্ঞা( zPath!=0 );
  প্রতিজ্ঞা( strlen(zPath)<=SQLITE_MAX_PATHLEN );  /* tag-20210611-1 */
  ঘূৰাই pVfs->xDlOpen(pVfs, zPath);
}
শূন্য sqlite3OsDlError(sqlite3_vfs *pVfs, পূর্ণাঙ্ক nByte, ৱর্ণ *zBufOut){
  pVfs->xDlError(pVfs, nByte, zBufOut);
}
শূন্য (*sqlite3OsDlSym(sqlite3_vfs *pVfs, শূন্য *pHdle, ধ্রুৱ ৱর্ণ *zSym))(শূন্য){
  ঘূৰাই pVfs->xDlSym(pVfs, pHdle, zSym);
}
শূন্য sqlite3OsDlClose(sqlite3_vfs *pVfs, শূন্য *pHandle){
  pVfs->xDlClose(pVfs, pHandle);
}
#endif /* SQLITE_OMIT_LOAD_EXTENSION */
পূর্ণাঙ্ক sqlite3OsRandomness(sqlite3_vfs *pVfs, পূর্ণাঙ্ক nByte, ৱর্ণ *zBufOut){
  যদি( sqlite3Config.iPrngSeed ){
    memset(zBufOut, 0, nByte);
    যদি( ALWAYS(nByte>(signed)পরিমাণ(unsigned)) ) nByte = পরিমাণ(unsigned পূর্ণাঙ্ক);
    memcpy(zBufOut, &sqlite3Config.iPrngSeed, nByte);
    ঘূৰাই SQLITE_OK;
  }অন্যথা{
    ঘূৰাই pVfs->xRandomness(pVfs, nByte, zBufOut);
  }

}
পূর্ণাঙ্ক sqlite3OsSleep(sqlite3_vfs *pVfs, পূর্ণাঙ্ক nMicro){
  ঘূৰাই pVfs->xSleep(pVfs, nMicro);
}
পূর্ণাঙ্ক sqlite3OsGetLastError(sqlite3_vfs *pVfs){
  ঘূৰাই pVfs->xGetLastError ? pVfs->xGetLastError(pVfs, 0, 0) : 0;
}
পূর্ণাঙ্ক sqlite3OsCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *pTimeOut){
  পূর্ণাঙ্ক rc;
  /* IMPLEMENTATION-OF: R-49045-42493 SQLite will use the xCurrentTimeInt64()
  ** method to get the current date and time if that method is available
  ** (if iVersion is 2 or greater and the function pointer is not NULL) and
  ** will fall back to xCurrentTime() if xCurrentTimeInt64() is
  ** unavailable.
  */
  যদি( pVfs->iVersion>=2 && pVfs->xCurrentTimeInt64 ){
    rc = pVfs->xCurrentTimeInt64(pVfs, pTimeOut);
  }অন্যথা{
    দ্ৱিগুণ r;
    rc = pVfs->xCurrentTime(pVfs, &r);
    *pTimeOut = sqlite3RealToI64(r*86400000.0);
  }
  ঘূৰাই rc;
}

পূর্ণাঙ্ক sqlite3OsOpenMalloc(
  sqlite3_vfs *pVfs,
  ধ্রুৱ ৱর্ণ *zFile,
  sqlite3_file **ppFile,
  পূর্ণাঙ্ক flags,
  পূর্ণাঙ্ক *pOutFlags
){
  পূর্ণাঙ্ক rc;
  sqlite3_file *pFile;
  pFile = (sqlite3_file *)sqlite3MallocZero(pVfs->szOsFile);
  যদি( pFile ){
    rc = sqlite3OsOpen(pVfs, zFile, pFile, flags, pOutFlags);
    যদি( rc!=SQLITE_OK ){
      sqlite3_free(pFile);
      *ppFile = 0;
    }অন্যথা{
      *ppFile = pFile;
    }
  }অন্যথা{
    *ppFile = 0;
    rc = SQLITE_NOMEM_BKPT;
  }
  প্রতিজ্ঞা( *ppFile!=0 || rc!=SQLITE_OK );
  ঘূৰাই rc;
}
শূন্য sqlite3OsCloseFree(sqlite3_file *pFile){
  প্রতিজ্ঞা( pFile );
  sqlite3OsClose(pFile);
  sqlite3_free(pFile);
}

/*
** This function is a wrapper around the OS specific implementation of
** sqlite3_os_init(). The purpose of the wrapper is to provide the
** ability to simulate a malloc failure, so that the handling of an
** error in sqlite3_os_init() by the upper layers can be tested.
*/
পূর্ণাঙ্ক sqlite3OsInit(শূন্য){
  শূন্য *p = sqlite3_malloc(10);
  যদি( p==0 ) ঘূৰাই SQLITE_NOMEM_BKPT;
  sqlite3_free(p);
  ঘূৰাই sqlite3_os_init();
}

/*
** The list of all registered VFS implementations.
*/
স্থির sqlite3_vfs * SQLITE_WSD vfsList = 0;
#define vfsList GLOBAL(sqlite3_vfs *, vfsList)

/*
** Locate a VFS by name.  If no name is given, simply return the
** first VFS on the list.
*/
sqlite3_vfs *sqlite3_vfs_find(ধ্রুৱ ৱর্ণ *zVfs){
  sqlite3_vfs *pVfs = 0;
#যদি SQLITE_THREADSAFE
  sqlite3_mutex *mutex;
#endif
#ifndef SQLITE_OMIT_AUTOINIT
  পূর্ণাঙ্ক rc = sqlite3_initialize();
  যদি( rc ) ঘূৰাই 0;
#endif
#যদি SQLITE_THREADSAFE
  mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
#endif
  sqlite3_mutex_enter(mutex);
  বাবে(pVfs = vfsList; pVfs; pVfs=pVfs->pNext){
    যদি( zVfs==0 ) ভঙ্গ;
    যদি( strcmp(zVfs, pVfs->zName)==0 ) ভঙ্গ;
  }
  sqlite3_mutex_leave(mutex);
  ঘূৰাই pVfs;
}

/*
** Unlink a VFS from the linked list
*/
স্থির শূন্য vfsUnlink(sqlite3_vfs *pVfs){
  প্রতিজ্ঞা( sqlite3_mutex_held(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN)) );
  যদি( pVfs==0 ){
    /* No-op */
  }অন্যথা যদি( vfsList==pVfs ){
    vfsList = pVfs->pNext;
  }অন্যথা যদি( vfsList ){
    sqlite3_vfs *p = vfsList;
    যেতিয়ালৈ( p->pNext && p->pNext!=pVfs ){
      p = p->pNext;
    }
    যদি( p->pNext==pVfs ){
      p->pNext = pVfs->pNext;
    }
  }
}

/*
** Register a VFS with the system.  It is harmless to register the same
** VFS multiple times.  The new VFS becomes the default if makeDflt is
** true.
*/
পূর্ণাঙ্ক sqlite3_vfs_register(sqlite3_vfs *pVfs, পূর্ণাঙ্ক makeDflt){
  MUTEX_LOGIC(sqlite3_mutex *mutex;)
#ifndef SQLITE_OMIT_AUTOINIT
  পূর্ণাঙ্ক rc = sqlite3_initialize();
  যদি( rc ) ঘূৰাই rc;
#endif
#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( pVfs==0 ) ঘূৰাই SQLITE_MISUSE_BKPT;
#endif

  MUTEX_LOGIC( mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  sqlite3_mutex_enter(mutex);
  vfsUnlink(pVfs);
  যদি( makeDflt || vfsList==0 ){
    pVfs->pNext = vfsList;
    vfsList = pVfs;
  }অন্যথা{
    pVfs->pNext = vfsList->pNext;
    vfsList->pNext = pVfs;
  }
  প্রতিজ্ঞা(vfsList);
  sqlite3_mutex_leave(mutex);
  ঘূৰাই SQLITE_OK;
}

/*
** Unregister a VFS so that it is no longer accessible.
*/
পূর্ণাঙ্ক sqlite3_vfs_unregister(sqlite3_vfs *pVfs){
  MUTEX_LOGIC(sqlite3_mutex *mutex;)
#ifndef SQLITE_OMIT_AUTOINIT
  পূর্ণাঙ্ক rc = sqlite3_initialize();
  যদি( rc ) ঘূৰাই rc;
#endif
  MUTEX_LOGIC( mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  sqlite3_mutex_enter(mutex);
  vfsUnlink(pVfs);
  sqlite3_mutex_leave(mutex);
  ঘূৰাই SQLITE_OK;
}
