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
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"

/*
** If we compile with the SQLITE_TEST macro set, then the following block
** of code will give us the ability to simulate a disk I/O error.  This
** is used for testing the I/O recovery logic.
*/
#ಯದಿ defined(SQLITE_TEST)
ಪೂರ್ಣಾಙ್ಕ sqlite3_io_error_hit = 0;            /* Total number of I/O Errors */
ಪೂರ್ಣಾಙ್ಕ sqlite3_io_error_hardhit = 0;        /* Number of non-benign errors */
ಪೂರ್ಣಾಙ್ಕ sqlite3_io_error_pending = 0;        /* Count down to first I/O error */
ಪೂರ್ಣಾಙ್ಕ sqlite3_io_error_persist = 0;        /* True if I/O errors persist */
ಪೂರ್ಣಾಙ್ಕ sqlite3_io_error_benign = 0;         /* True if errors are benign */
ಪೂರ್ಣಾಙ್ಕ sqlite3_diskfull_pending = 0;
ಪೂರ್ಣಾಙ್ಕ sqlite3_diskfull = 0;
#endif /* defined(SQLITE_TEST) */

/*
** When testing, also keep a count of the number of open files.
*/
#ಯದಿ defined(SQLITE_TEST)
ಪೂರ್ಣಾಙ್ಕ sqlite3_open_file_ಎಣಿಕೆ = 0;
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
#ಯದಿ defined(SQLITE_TEST)
ಪೂರ್ಣಾಙ್ಕ sqlite3_memdebug_vfs_oom_test = 1;
  #define DO_OS_MALLOC_TEST(x)                                       \
  ಯದಿ (sqlite3_memdebug_vfs_oom_test && (!x || !sqlite3JournalIsInMemory(x))) { \
    ಶೂನ್ಯ *pTstAlloc = sqlite3Malloc(10);                             \
    ಯದಿ (!pTstAlloc) ಪ್ರತ್ಯಾಗಮ SQLITE_IOERR_NOMEM_BKPT;                  \
    sqlite3_free(pTstAlloc);                                         \
  }
#ಅನ್ಯಥಾ
  #define DO_OS_MALLOC_TEST(x)
#endif

/*
** The following routines are convenience wrappers around methods
** of the sqlite3_file object.  This is mostly just syntactic sugar. All
** of this would be completely automatic if SQLite were coded using
** C++ instead of plain old C.
*/
ಶೂನ್ಯ sqlite3OsClose(sqlite3_file *pId){
  ಯದಿ( pId->pMethods ){
    pId->pMethods->xClose(pId);
    pId->pMethods = 0;
  }
}
ಪೂರ್ಣಾಙ್ಕ sqlite3OsRead(sqlite3_file *id, ಶೂನ್ಯ *pBuf, ಪೂರ್ಣಾಙ್ಕ amt, i64 offset){
  DO_OS_MALLOC_TEST(id);
  ಪ್ರತ್ಯಾಗಮ id->pMethods->xRead(id, pBuf, amt, offset);
}
ಪೂರ್ಣಾಙ್ಕ sqlite3OsWrite(sqlite3_file *id, ಧ್ರುವ ಶೂನ್ಯ *pBuf, ಪೂರ್ಣಾಙ್ಕ amt, i64 offset){
  DO_OS_MALLOC_TEST(id);
  ಪ್ರತ್ಯಾಗಮ id->pMethods->xWrite(id, pBuf, amt, offset);
}
ಪೂರ್ಣಾಙ್ಕ sqlite3OsTruncate(sqlite3_file *id, i64 size){
  ಪ್ರತ್ಯಾಗಮ id->pMethods->xTruncate(id, size);
}
ಪೂರ್ಣಾಙ್ಕ sqlite3OsSync(sqlite3_file *id, ಪೂರ್ಣಾಙ್ಕ flags){
  DO_OS_MALLOC_TEST(id);
  ಪ್ರತ್ಯಾಗಮ flags ? id->pMethods->xSync(id, flags) : SQLITE_OK;
}
ಪೂರ್ಣಾಙ್ಕ sqlite3OsFileSize(sqlite3_file *id, i64 *pSize){
  DO_OS_MALLOC_TEST(id);
  ಪ್ರತ್ಯಾಗಮ id->pMethods->xFileSize(id, pSize);
}
ಪೂರ್ಣಾಙ್ಕ sqlite3OsLock(sqlite3_file *id, ಪೂರ್ಣಾಙ್ಕ lockType){
  DO_OS_MALLOC_TEST(id);
  ಪ್ರತಿಜ್ಞಾ( lockType>=SQLITE_LOCK_SHARED && lockType<=SQLITE_LOCK_EXCLUSIVE );
  ಪ್ರತ್ಯಾಗಮ id->pMethods->xLock(id, lockType);
}
ಪೂರ್ಣಾಙ್ಕ sqlite3OsUnlock(sqlite3_file *id, ಪೂರ್ಣಾಙ್ಕ lockType){
  ಪ್ರತಿಜ್ಞಾ( lockType==SQLITE_LOCK_NONE || lockType==SQLITE_LOCK_SHARED );
  ಪ್ರತ್ಯಾಗಮ id->pMethods->xUnlock(id, lockType);
}
ಪೂರ್ಣಾಙ್ಕ sqlite3OsCheckReservedLock(sqlite3_file *id, ಪೂರ್ಣಾಙ್ಕ *pResOut){
  DO_OS_MALLOC_TEST(id);
  ಪ್ರತ್ಯಾಗಮ id->pMethods->xCheckReservedLock(id, pResOut);
}

/*
** Use sqlite3OsFileControl() when we are doing something that might fail
** and we need to know about the failures.  Use sqlite3OsFileControlHint()
** when simply tossing information over the wall to the VFS and we do not
** really care if the VFS receives and understands the information since it
** is only a hint and can be safely ignored.  The sqlite3OsFileControlHint()
** routine has no return value since the return value would be meaningless.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3OsFileControl(sqlite3_file *id, ಪೂರ್ಣಾಙ್ಕ op, ಶೂನ್ಯ *pArg){
  ಯದಿ( id->pMethods==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_NOTFOUND;
#ifdef SQLITE_TEST
  ಯದಿ( op!=SQLITE_FCNTL_COMMIT_PHASETWO
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
  ಪ್ರತ್ಯಾಗಮ id->pMethods->xFileControl(id, op, pArg);
}
ಶೂನ್ಯ sqlite3OsFileControlHint(sqlite3_file *id, ಪೂರ್ಣಾಙ್ಕ op, ಶೂನ್ಯ *pArg){
  ಯದಿ( id->pMethods ) (ಶೂನ್ಯ)id->pMethods->xFileControl(id, op, pArg);
}

ಪೂರ್ಣಾಙ್ಕ sqlite3OsSectorSize(sqlite3_file *id){
  ಪೂರ್ಣಾಙ್ಕ (*xSectorSize)(sqlite3_file*) = id->pMethods->xSectorSize;
  ಪ್ರತ್ಯಾಗಮ (xSectorSize ? xSectorSize(id) : SQLITE_DEFAULT_SECTOR_SIZE);
}
ಪೂರ್ಣಾಙ್ಕ sqlite3OsDeviceCharacteristics(sqlite3_file *id){
  ಯದಿ( NEVER(id->pMethods==0) ) ಪ್ರತ್ಯಾಗಮ 0;
  ಪ್ರತ್ಯಾಗಮ id->pMethods->xDeviceCharacteristics(id);
}
#ifndef SQLITE_OMIT_WAL
ಪೂರ್ಣಾಙ್ಕ sqlite3OsShmLock(sqlite3_file *id, ಪೂರ್ಣಾಙ್ಕ offset, ಪೂರ್ಣಾಙ್ಕ n, ಪೂರ್ಣಾಙ್ಕ flags){
  ಪ್ರತ್ಯಾಗಮ id->pMethods->xShmLock(id, offset, n, flags);
}
ಶೂನ್ಯ sqlite3OsShmBarrier(sqlite3_file *id){
  id->pMethods->xShmBarrier(id);
}
ಪೂರ್ಣಾಙ್ಕ sqlite3OsShmUnmap(sqlite3_file *id, ಪೂರ್ಣಾಙ್ಕ deleteFlag){
  ಪ್ರತ್ಯಾಗಮ id->pMethods->xShmUnmap(id, deleteFlag);
}
ಪೂರ್ಣಾಙ್ಕ sqlite3OsShmMap(
  sqlite3_file *id,               /* Database file handle */
  ಪೂರ್ಣಾಙ್ಕ iPage,
  ಪೂರ್ಣಾಙ್ಕ pgsz,
  ಪೂರ್ಣಾಙ್ಕ bExtend,                    /* True to extend file if necessary */
  ಶೂನ್ಯ ಚಞ್ಚಲ **pp              /* OUT: Pointer to mapping */
){
  DO_OS_MALLOC_TEST(id);
  ಪ್ರತ್ಯಾಗಮ id->pMethods->xShmMap(id, iPage, pgsz, bExtend, pp);
}
#endif /* SQLITE_OMIT_WAL */

#ಯದಿ SQLITE_MAX_MMAP_SIZE>0
/* The real implementation of xFetch and xUnfetch */
ಪೂರ್ಣಾಙ್ಕ sqlite3OsFetch(sqlite3_file *id, i64 iOff, ಪೂರ್ಣಾಙ್ಕ iAmt, ಶೂನ್ಯ **pp){
  DO_OS_MALLOC_TEST(id);
  ಪ್ರತ್ಯಾಗಮ id->pMethods->xFetch(id, iOff, iAmt, pp);
}
ಪೂರ್ಣಾಙ್ಕ sqlite3OsUnfetch(sqlite3_file *id, i64 iOff, ಶೂನ್ಯ *p){
  ಪ್ರತ್ಯಾಗಮ id->pMethods->xUnfetch(id, iOff, p);
}
#ಅನ್ಯಥಾ
/* No-op stubs to use when memory-mapped I/O is disabled */
ಪೂರ್ಣಾಙ್ಕ sqlite3OsFetch(sqlite3_file *id, i64 iOff, ಪೂರ್ಣಾಙ್ಕ iAmt, ಶೂನ್ಯ **pp){
  *pp = 0;
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}
ಪೂರ್ಣಾಙ್ಕ sqlite3OsUnfetch(sqlite3_file *id, i64 iOff, ಶೂನ್ಯ *p){
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}
#endif

/*
** The next group of routines are convenience wrappers around the
** VFS methods.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3OsOpen(
  sqlite3_vfs *pVfs,
  ಧ್ರುವ ವರ್ಣ *zPath,
  sqlite3_file *pFile,
  ಪೂರ್ಣಾಙ್ಕ flags,
  ಪೂರ್ಣಾಙ್ಕ *pFlagsOut
){
  ಪೂರ್ಣಾಙ್ಕ rc;
  DO_OS_MALLOC_TEST(0);
  /* 0x87f7f is a mask of SQLITE_OPEN_ flags that are valid to be passed
  ** down into the VFS layer.  Some SQLITE_OPEN_ flags (for example,
  ** SQLITE_OPEN_FULLMUTEX or SQLITE_OPEN_SHAREDCACHE) are blocked before
  ** reaching the VFS. */
  ಪ್ರತಿಜ್ಞಾ( zPath || (flags & SQLITE_OPEN_EXCLUSIVE) );
  rc = pVfs->xOpen(pVfs, zPath, pFile, flags & 0x1087f7f, pFlagsOut);
  ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_OK || pFile->pMethods==0 );
  ಪ್ರತ್ಯಾಗಮ rc;
}
ಪೂರ್ಣಾಙ್ಕ sqlite3OsDelete(sqlite3_vfs *pVfs, ಧ್ರುವ ವರ್ಣ *zPath, ಪೂರ್ಣಾಙ್ಕ dirSync){
  DO_OS_MALLOC_TEST(0);
  ಪ್ರತಿಜ್ಞಾ( dirSync==0 || dirSync==1 );
  ಪ್ರತ್ಯಾಗಮ pVfs->xDelete!=0 ? pVfs->xDelete(pVfs, zPath, dirSync) : SQLITE_OK;
}
ಪೂರ್ಣಾಙ್ಕ sqlite3OsAccess(
  sqlite3_vfs *pVfs,
  ಧ್ರುವ ವರ್ಣ *zPath,
  ಪೂರ್ಣಾಙ್ಕ flags,
  ಪೂರ್ಣಾಙ್ಕ *pResOut
){
  DO_OS_MALLOC_TEST(0);
  ಪ್ರತ್ಯಾಗಮ pVfs->xAccess(pVfs, zPath, flags, pResOut);
}
ಪೂರ್ಣಾಙ್ಕ sqlite3OsFullPathname(
  sqlite3_vfs *pVfs,
  ಧ್ರುವ ವರ್ಣ *zPath,
  ಪೂರ್ಣಾಙ್ಕ nPathOut,
  ವರ್ಣ *zPathOut
){
  DO_OS_MALLOC_TEST(0);
  zPathOut[0] = 0;
  ಪ್ರತ್ಯಾಗಮ pVfs->xFullPathname(pVfs, zPath, nPathOut, zPathOut);
}
#ifndef SQLITE_OMIT_LOAD_EXTENSION
ಶೂನ್ಯ *sqlite3OsDlOpen(sqlite3_vfs *pVfs, ಧ್ರುವ ವರ್ಣ *zPath){
  ಪ್ರತಿಜ್ಞಾ( zPath!=0 );
  ಪ್ರತಿಜ್ಞಾ( strlen(zPath)<=SQLITE_MAX_PATHLEN );  /* tag-20210611-1 */
  ಪ್ರತ್ಯಾಗಮ pVfs->xDlOpen(pVfs, zPath);
}
ಶೂನ್ಯ sqlite3OsDlError(sqlite3_vfs *pVfs, ಪೂರ್ಣಾಙ್ಕ nByte, ವರ್ಣ *zBufOut){
  pVfs->xDlError(pVfs, nByte, zBufOut);
}
ಶೂನ್ಯ (*sqlite3OsDlSym(sqlite3_vfs *pVfs, ಶೂನ್ಯ *pHdle, ಧ್ರುವ ವರ್ಣ *zSym))(ಶೂನ್ಯ){
  ಪ್ರತ್ಯಾಗಮ pVfs->xDlSym(pVfs, pHdle, zSym);
}
ಶೂನ್ಯ sqlite3OsDlClose(sqlite3_vfs *pVfs, ಶೂನ್ಯ *pHandle){
  pVfs->xDlClose(pVfs, pHandle);
}
#endif /* SQLITE_OMIT_LOAD_EXTENSION */
ಪೂರ್ಣಾಙ್ಕ sqlite3OsRandomness(sqlite3_vfs *pVfs, ಪೂರ್ಣಾಙ್ಕ nByte, ವರ್ಣ *zBufOut){
  ಯದಿ( sqlite3Config.iPrngSeed ){
    memset(zBufOut, 0, nByte);
    ಯದಿ( ALWAYS(nByte>(signed)ಪರಿಮಾಣ(unsigned)) ) nByte = ಪರಿಮಾಣ(unsigned ಪೂರ್ಣಾಙ್ಕ);
    memcpy(zBufOut, &sqlite3Config.iPrngSeed, nByte);
    ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
  }ಅನ್ಯಥಾ{
    ಪ್ರತ್ಯಾಗಮ pVfs->xRandomness(pVfs, nByte, zBufOut);
  }

}
ಪೂರ್ಣಾಙ್ಕ sqlite3OsSleep(sqlite3_vfs *pVfs, ಪೂರ್ಣಾಙ್ಕ nMicro){
  ಪ್ರತ್ಯಾಗಮ pVfs->xSleep(pVfs, nMicro);
}
ಪೂರ್ಣಾಙ್ಕ sqlite3OsGetLastError(sqlite3_vfs *pVfs){
  ಪ್ರತ್ಯಾಗಮ pVfs->xGetLastError ? pVfs->xGetLastError(pVfs, 0, 0) : 0;
}
ಪೂರ್ಣಾಙ್ಕ sqlite3OsCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *pTimeOut){
  ಪೂರ್ಣಾಙ್ಕ rc;
  /* IMPLEMENTATION-OF: R-49045-42493 SQLite will use the xCurrentTimeInt64()
  ** method to get the current date and time if that method is available
  ** (if iVersion is 2 or greater and the function pointer is not NULL) and
  ** will fall back to xCurrentTime() if xCurrentTimeInt64() is
  ** unavailable.
  */
  ಯದಿ( pVfs->iVersion>=2 && pVfs->xCurrentTimeInt64 ){
    rc = pVfs->xCurrentTimeInt64(pVfs, pTimeOut);
  }ಅನ್ಯಥಾ{
    ದ್ವಿಗುಣ r;
    rc = pVfs->xCurrentTime(pVfs, &r);
    *pTimeOut = sqlite3RealToI64(r*86400000.0);
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}

ಪೂರ್ಣಾಙ್ಕ sqlite3OsOpenMalloc(
  sqlite3_vfs *pVfs,
  ಧ್ರುವ ವರ್ಣ *zFile,
  sqlite3_file **ppFile,
  ಪೂರ್ಣಾಙ್ಕ flags,
  ಪೂರ್ಣಾಙ್ಕ *pOutFlags
){
  ಪೂರ್ಣಾಙ್ಕ rc;
  sqlite3_file *pFile;
  pFile = (sqlite3_file *)sqlite3MallocZero(pVfs->szOsFile);
  ಯದಿ( pFile ){
    rc = sqlite3OsOpen(pVfs, zFile, pFile, flags, pOutFlags);
    ಯದಿ( rc!=SQLITE_OK ){
      sqlite3_free(pFile);
      *ppFile = 0;
    }ಅನ್ಯಥಾ{
      *ppFile = pFile;
    }
  }ಅನ್ಯಥಾ{
    *ppFile = 0;
    rc = SQLITE_NOMEM_BKPT;
  }
  ಪ್ರತಿಜ್ಞಾ( *ppFile!=0 || rc!=SQLITE_OK );
  ಪ್ರತ್ಯಾಗಮ rc;
}
ಶೂನ್ಯ sqlite3OsCloseFree(sqlite3_file *pFile){
  ಪ್ರತಿಜ್ಞಾ( pFile );
  sqlite3OsClose(pFile);
  sqlite3_free(pFile);
}

/*
** This function is a wrapper around the OS specific implementation of
** sqlite3_os_init(). The purpose of the wrapper is to provide the
** ability to simulate a malloc failure, so that the handling of an
** error in sqlite3_os_init() by the upper layers can be tested.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3OsInit(ಶೂನ್ಯ){
  ಶೂನ್ಯ *p = sqlite3_malloc(10);
  ಯದಿ( p==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM_BKPT;
  sqlite3_free(p);
  ಪ್ರತ್ಯಾಗಮ sqlite3_os_init();
}

/*
** The list of all registered VFS implementations.
*/
ಸ್ಥಿರ sqlite3_vfs * SQLITE_WSD vfsList = 0;
#define vfsList GLOBAL(sqlite3_vfs *, vfsList)

/*
** Locate a VFS by name.  If no name is given, simply return the
** first VFS on the list.
*/
sqlite3_vfs *sqlite3_vfs_find(ಧ್ರುವ ವರ್ಣ *zVfs){
  sqlite3_vfs *pVfs = 0;
#ಯದಿ SQLITE_THREADSAFE
  sqlite3_mutex *mutex;
#endif
#ifndef SQLITE_OMIT_AUTOINIT
  ಪೂರ್ಣಾಙ್ಕ rc = sqlite3_initialize();
  ಯದಿ( rc ) ಪ್ರತ್ಯಾಗಮ 0;
#endif
#ಯದಿ SQLITE_THREADSAFE
  mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
#endif
  sqlite3_mutex_enter(mutex);
  ಪ್ರತಿ(pVfs = vfsList; pVfs; pVfs=pVfs->pNext){
    ಯದಿ( zVfs==0 ) ಭಙ್ಗ;
    ಯದಿ( strcmp(zVfs, pVfs->zName)==0 ) ಭಙ್ಗ;
  }
  sqlite3_mutex_leave(mutex);
  ಪ್ರತ್ಯಾಗಮ pVfs;
}

/*
** Unlink a VFS from the linked list
*/
ಸ್ಥಿರ ಶೂನ್ಯ vfsUnlink(sqlite3_vfs *pVfs){
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN)) );
  ಯದಿ( pVfs==0 ){
    /* No-op */
  }ಅನ್ಯಥಾ ಯದಿ( vfsList==pVfs ){
    vfsList = pVfs->pNext;
  }ಅನ್ಯಥಾ ಯದಿ( vfsList ){
    sqlite3_vfs *p = vfsList;
    ಯಾವತ್( p->pNext && p->pNext!=pVfs ){
      p = p->pNext;
    }
    ಯದಿ( p->pNext==pVfs ){
      p->pNext = pVfs->pNext;
    }
  }
}

/*
** Register a VFS with the system.  It is harmless to register the same
** VFS multiple times.  The new VFS becomes the default if makeDflt is
** true.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_vfs_register(sqlite3_vfs *pVfs, ಪೂರ್ಣಾಙ್ಕ makeDflt){
  MUTEX_LOGIC(sqlite3_mutex *mutex;)
#ifndef SQLITE_OMIT_AUTOINIT
  ಪೂರ್ಣಾಙ್ಕ rc = sqlite3_initialize();
  ಯದಿ( rc ) ಪ್ರತ್ಯಾಗಮ rc;
#endif
#ifdef SQLITE_ENABLE_API_ARMOR
  ಯದಿ( pVfs==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_MISUSE_BKPT;
#endif

  MUTEX_LOGIC( mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  sqlite3_mutex_enter(mutex);
  vfsUnlink(pVfs);
  ಯದಿ( makeDflt || vfsList==0 ){
    pVfs->pNext = vfsList;
    vfsList = pVfs;
  }ಅನ್ಯಥಾ{
    pVfs->pNext = vfsList->pNext;
    vfsList->pNext = pVfs;
  }
  ಪ್ರತಿಜ್ಞಾ(vfsList);
  sqlite3_mutex_leave(mutex);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Unregister a VFS so that it is no longer accessible.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_vfs_unregister(sqlite3_vfs *pVfs){
  MUTEX_LOGIC(sqlite3_mutex *mutex;)
#ifndef SQLITE_OMIT_AUTOINIT
  ಪೂರ್ಣಾಙ್ಕ rc = sqlite3_initialize();
  ಯದಿ( rc ) ಪ್ರತ್ಯಾಗಮ rc;
#endif
  MUTEX_LOGIC( mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  sqlite3_mutex_enter(mutex);
  vfsUnlink(pVfs);
  sqlite3_mutex_leave(mutex);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}
