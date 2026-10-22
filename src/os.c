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
#ଅନ୍ତର୍ଭାଵ "sqliteInt.h"

/*
** If we compile with the SQLITE_TEST macro set, then the following block
** of code will give us the ability to simulate a disk I/O error.  This
** is used for testing the I/O recovery logic.
*/
#ଯଦି defined(SQLITE_TEST)
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_io_error_hit = 0;            /* Total number of I/O Errors */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_io_error_hardhit = 0;        /* Number of non-benign errors */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_io_error_pending = 0;        /* Count down to first I/O error */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_io_error_persist = 0;        /* True if I/O errors persist */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_io_error_benign = 0;         /* True if errors are benign */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_diskfull_pending = 0;
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_diskfull = 0;
#endif /* defined(SQLITE_TEST) */

/*
** When testing, also keep a count of the number of open files.
*/
#ଯଦି defined(SQLITE_TEST)
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_open_file_ଗଣନା = 0;
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
#ଯଦି defined(SQLITE_TEST)
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_memdebug_vfs_oom_test = 1;
  #define DO_OS_MALLOC_TEST(x)                                       \
  ଯଦି (sqlite3_memdebug_vfs_oom_test && (!x || !sqlite3JournalIsInMemory(x))) { \
    ଶୂନ୍ୟ *pTstAlloc = sqlite3Malloc(10);                             \
    ଯଦି (!pTstAlloc) ଫେରନ୍ତୁ SQLITE_IOERR_NOMEM_BKPT;                  \
    sqlite3_free(pTstAlloc);                                         \
  }
#ଅନ୍ୟଥା
  #define DO_OS_MALLOC_TEST(x)
#endif

/*
** The following routines are convenience wrappers around methods
** of the sqlite3_file object.  This is mostly just syntactic sugar. All
** of this would be completely automatic if SQLite were coded using
** C++ instead of plain old C.
*/
ଶୂନ୍ୟ sqlite3OsClose(sqlite3_file *pId){
  ଯଦି( pId->pMethods ){
    pId->pMethods->xClose(pId);
    pId->pMethods = 0;
  }
}
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3OsRead(sqlite3_file *id, ଶୂନ୍ୟ *pBuf, ପୂର୍ଣ୍ଣସଂଖ୍ୟା amt, i64 offset){
  DO_OS_MALLOC_TEST(id);
  ଫେରନ୍ତୁ id->pMethods->xRead(id, pBuf, amt, offset);
}
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3OsWrite(sqlite3_file *id, ଅଚଳ ଶୂନ୍ୟ *pBuf, ପୂର୍ଣ୍ଣସଂଖ୍ୟା amt, i64 offset){
  DO_OS_MALLOC_TEST(id);
  ଫେରନ୍ତୁ id->pMethods->xWrite(id, pBuf, amt, offset);
}
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3OsTruncate(sqlite3_file *id, i64 size){
  ଫେରନ୍ତୁ id->pMethods->xTruncate(id, size);
}
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3OsSync(sqlite3_file *id, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags){
  DO_OS_MALLOC_TEST(id);
  ଫେରନ୍ତୁ flags ? id->pMethods->xSync(id, flags) : SQLITE_OK;
}
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3OsFileSize(sqlite3_file *id, i64 *pSize){
  DO_OS_MALLOC_TEST(id);
  ଫେରନ୍ତୁ id->pMethods->xFileSize(id, pSize);
}
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3OsLock(sqlite3_file *id, ପୂର୍ଣ୍ଣସଂଖ୍ୟା lockType){
  DO_OS_MALLOC_TEST(id);
  ପ୍ରତିଜ୍ଞା( lockType>=SQLITE_LOCK_SHARED && lockType<=SQLITE_LOCK_EXCLUSIVE );
  ଫେରନ୍ତୁ id->pMethods->xLock(id, lockType);
}
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3OsUnlock(sqlite3_file *id, ପୂର୍ଣ୍ଣସଂଖ୍ୟା lockType){
  ପ୍ରତିଜ୍ଞା( lockType==SQLITE_LOCK_NONE || lockType==SQLITE_LOCK_SHARED );
  ଫେରନ୍ତୁ id->pMethods->xUnlock(id, lockType);
}
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3OsCheckReservedLock(sqlite3_file *id, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pResOut){
  DO_OS_MALLOC_TEST(id);
  ଫେରନ୍ତୁ id->pMethods->xCheckReservedLock(id, pResOut);
}

/*
** Use sqlite3OsFileControl() when we are doing something that might fail
** and we need to know about the failures.  Use sqlite3OsFileControlHint()
** when simply tossing information over the wall to the VFS and we do not
** really care if the VFS receives and understands the information since it
** is only a hint and can be safely ignored.  The sqlite3OsFileControlHint()
** routine has no return value since the return value would be meaningless.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3OsFileControl(sqlite3_file *id, ପୂର୍ଣ୍ଣସଂଖ୍ୟା op, ଶୂନ୍ୟ *pArg){
  ଯଦି( id->pMethods==0 ) ଫେରନ୍ତୁ SQLITE_NOTFOUND;
#ifdef SQLITE_TEST
  ଯଦି( op!=SQLITE_FCNTL_COMMIT_PHASETWO
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
  ଫେରନ୍ତୁ id->pMethods->xFileControl(id, op, pArg);
}
ଶୂନ୍ୟ sqlite3OsFileControlHint(sqlite3_file *id, ପୂର୍ଣ୍ଣସଂଖ୍ୟା op, ଶୂନ୍ୟ *pArg){
  ଯଦି( id->pMethods ) (ଶୂନ୍ୟ)id->pMethods->xFileControl(id, op, pArg);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3OsSectorSize(sqlite3_file *id){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*xSectorSize)(sqlite3_file*) = id->pMethods->xSectorSize;
  ଫେରନ୍ତୁ (xSectorSize ? xSectorSize(id) : SQLITE_DEFAULT_SECTOR_SIZE);
}
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3OsDeviceCharacteristics(sqlite3_file *id){
  ଯଦି( NEVER(id->pMethods==0) ) ଫେରନ୍ତୁ 0;
  ଫେରନ୍ତୁ id->pMethods->xDeviceCharacteristics(id);
}
#ifndef SQLITE_OMIT_WAL
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3OsShmLock(sqlite3_file *id, ପୂର୍ଣ୍ଣସଂଖ୍ୟା offset, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags){
  ଫେରନ୍ତୁ id->pMethods->xShmLock(id, offset, n, flags);
}
ଶୂନ୍ୟ sqlite3OsShmBarrier(sqlite3_file *id){
  id->pMethods->xShmBarrier(id);
}
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3OsShmUnmap(sqlite3_file *id, ପୂର୍ଣ୍ଣସଂଖ୍ୟା deleteFlag){
  ଫେରନ୍ତୁ id->pMethods->xShmUnmap(id, deleteFlag);
}
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3OsShmMap(
  sqlite3_file *id,               /* Database file handle */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iPage,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା pgsz,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା bExtend,                    /* True to extend file if necessary */
  ଶୂନ୍ୟ ଅସ୍ଥିର **pp              /* OUT: Pointer to mapping */
){
  DO_OS_MALLOC_TEST(id);
  ଫେରନ୍ତୁ id->pMethods->xShmMap(id, iPage, pgsz, bExtend, pp);
}
#endif /* SQLITE_OMIT_WAL */

#ଯଦି SQLITE_MAX_MMAP_SIZE>0
/* The real implementation of xFetch and xUnfetch */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3OsFetch(sqlite3_file *id, i64 iOff, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iAmt, ଶୂନ୍ୟ **pp){
  DO_OS_MALLOC_TEST(id);
  ଫେରନ୍ତୁ id->pMethods->xFetch(id, iOff, iAmt, pp);
}
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3OsUnfetch(sqlite3_file *id, i64 iOff, ଶୂନ୍ୟ *p){
  ଫେରନ୍ତୁ id->pMethods->xUnfetch(id, iOff, p);
}
#ଅନ୍ୟଥା
/* No-op stubs to use when memory-mapped I/O is disabled */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3OsFetch(sqlite3_file *id, i64 iOff, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iAmt, ଶୂନ୍ୟ **pp){
  *pp = 0;
  ଫେରନ୍ତୁ SQLITE_OK;
}
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3OsUnfetch(sqlite3_file *id, i64 iOff, ଶୂନ୍ୟ *p){
  ଫେରନ୍ତୁ SQLITE_OK;
}
#endif

/*
** The next group of routines are convenience wrappers around the
** VFS methods.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3OsOpen(
  sqlite3_vfs *pVfs,
  ଅଚଳ ଅକ୍ଷର *zPath,
  sqlite3_file *pFile,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pFlagsOut
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  DO_OS_MALLOC_TEST(0);
  /* 0x87f7f is a mask of SQLITE_OPEN_ flags that are valid to be passed
  ** down into the VFS layer.  Some SQLITE_OPEN_ flags (for example,
  ** SQLITE_OPEN_FULLMUTEX or SQLITE_OPEN_SHAREDCACHE) are blocked before
  ** reaching the VFS. */
  ପ୍ରତିଜ୍ଞା( zPath || (flags & SQLITE_OPEN_EXCLUSIVE) );
  rc = pVfs->xOpen(pVfs, zPath, pFile, flags & 0x1087f7f, pFlagsOut);
  ପ୍ରତିଜ୍ଞା( rc==SQLITE_OK || pFile->pMethods==0 );
  ଫେରନ୍ତୁ rc;
}
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3OsDelete(sqlite3_vfs *pVfs, ଅଚଳ ଅକ୍ଷର *zPath, ପୂର୍ଣ୍ଣସଂଖ୍ୟା dirSync){
  DO_OS_MALLOC_TEST(0);
  ପ୍ରତିଜ୍ଞା( dirSync==0 || dirSync==1 );
  ଫେରନ୍ତୁ pVfs->xDelete!=0 ? pVfs->xDelete(pVfs, zPath, dirSync) : SQLITE_OK;
}
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3OsAccess(
  sqlite3_vfs *pVfs,
  ଅଚଳ ଅକ୍ଷର *zPath,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pResOut
){
  DO_OS_MALLOC_TEST(0);
  ଫେରନ୍ତୁ pVfs->xAccess(pVfs, zPath, flags, pResOut);
}
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3OsFullPathname(
  sqlite3_vfs *pVfs,
  ଅଚଳ ଅକ୍ଷର *zPath,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nPathOut,
  ଅକ୍ଷର *zPathOut
){
  DO_OS_MALLOC_TEST(0);
  zPathOut[0] = 0;
  ଫେରନ୍ତୁ pVfs->xFullPathname(pVfs, zPath, nPathOut, zPathOut);
}
#ifndef SQLITE_OMIT_LOAD_EXTENSION
ଶୂନ୍ୟ *sqlite3OsDlOpen(sqlite3_vfs *pVfs, ଅଚଳ ଅକ୍ଷର *zPath){
  ପ୍ରତିଜ୍ଞା( zPath!=0 );
  ପ୍ରତିଜ୍ଞା( strlen(zPath)<=SQLITE_MAX_PATHLEN );  /* tag-20210611-1 */
  ଫେରନ୍ତୁ pVfs->xDlOpen(pVfs, zPath);
}
ଶୂନ୍ୟ sqlite3OsDlError(sqlite3_vfs *pVfs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nByte, ଅକ୍ଷର *zBufOut){
  pVfs->xDlError(pVfs, nByte, zBufOut);
}
ଶୂନ୍ୟ (*sqlite3OsDlSym(sqlite3_vfs *pVfs, ଶୂନ୍ୟ *pHdle, ଅଚଳ ଅକ୍ଷର *zSym))(ଶୂନ୍ୟ){
  ଫେରନ୍ତୁ pVfs->xDlSym(pVfs, pHdle, zSym);
}
ଶୂନ୍ୟ sqlite3OsDlClose(sqlite3_vfs *pVfs, ଶୂନ୍ୟ *pHandle){
  pVfs->xDlClose(pVfs, pHandle);
}
#endif /* SQLITE_OMIT_LOAD_EXTENSION */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3OsRandomness(sqlite3_vfs *pVfs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nByte, ଅକ୍ଷର *zBufOut){
  ଯଦି( sqlite3Config.iPrngSeed ){
    memset(zBufOut, 0, nByte);
    ଯଦି( ALWAYS(nByte>(signed)ଆକାର(unsigned)) ) nByte = ଆକାର(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
    memcpy(zBufOut, &sqlite3Config.iPrngSeed, nByte);
    ଫେରନ୍ତୁ SQLITE_OK;
  }ଅନ୍ୟଥା{
    ଫେରନ୍ତୁ pVfs->xRandomness(pVfs, nByte, zBufOut);
  }

}
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3OsSleep(sqlite3_vfs *pVfs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nMicro){
  ଫେରନ୍ତୁ pVfs->xSleep(pVfs, nMicro);
}
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3OsGetLastError(sqlite3_vfs *pVfs){
  ଫେରନ୍ତୁ pVfs->xGetLastError ? pVfs->xGetLastError(pVfs, 0, 0) : 0;
}
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3OsCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *pTimeOut){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  /* IMPLEMENTATION-OF: R-49045-42493 SQLite will use the xCurrentTimeInt64()
  ** method to get the current date and time if that method is available
  ** (if iVersion is 2 or greater and the function pointer is not NULL) and
  ** will fall back to xCurrentTime() if xCurrentTimeInt64() is
  ** unavailable.
  */
  ଯଦି( pVfs->iVersion>=2 && pVfs->xCurrentTimeInt64 ){
    rc = pVfs->xCurrentTimeInt64(pVfs, pTimeOut);
  }ଅନ୍ୟଥା{
    ଦ୍ୱିଗୁଣ r;
    rc = pVfs->xCurrentTime(pVfs, &r);
    *pTimeOut = sqlite3RealToI64(r*86400000.0);
  }
  ଫେରନ୍ତୁ rc;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3OsOpenMalloc(
  sqlite3_vfs *pVfs,
  ଅଚଳ ଅକ୍ଷର *zFile,
  sqlite3_file **ppFile,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pOutFlags
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  sqlite3_file *pFile;
  pFile = (sqlite3_file *)sqlite3MallocZero(pVfs->szOsFile);
  ଯଦି( pFile ){
    rc = sqlite3OsOpen(pVfs, zFile, pFile, flags, pOutFlags);
    ଯଦି( rc!=SQLITE_OK ){
      sqlite3_free(pFile);
      *ppFile = 0;
    }ଅନ୍ୟଥା{
      *ppFile = pFile;
    }
  }ଅନ୍ୟଥା{
    *ppFile = 0;
    rc = SQLITE_NOMEM_BKPT;
  }
  ପ୍ରତିଜ୍ଞା( *ppFile!=0 || rc!=SQLITE_OK );
  ଫେରନ୍ତୁ rc;
}
ଶୂନ୍ୟ sqlite3OsCloseFree(sqlite3_file *pFile){
  ପ୍ରତିଜ୍ଞା( pFile );
  sqlite3OsClose(pFile);
  sqlite3_free(pFile);
}

/*
** This function is a wrapper around the OS specific implementation of
** sqlite3_os_init(). The purpose of the wrapper is to provide the
** ability to simulate a malloc failure, so that the handling of an
** error in sqlite3_os_init() by the upper layers can be tested.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3OsInit(ଶୂନ୍ୟ){
  ଶୂନ୍ୟ *p = sqlite3_malloc(10);
  ଯଦି( p==0 ) ଫେରନ୍ତୁ SQLITE_NOMEM_BKPT;
  sqlite3_free(p);
  ଫେରନ୍ତୁ sqlite3_os_init();
}

/*
** The list of all registered VFS implementations.
*/
ସ୍ଥିର sqlite3_vfs * SQLITE_WSD vfsList = 0;
#define vfsList GLOBAL(sqlite3_vfs *, vfsList)

/*
** Locate a VFS by name.  If no name is given, simply return the
** first VFS on the list.
*/
sqlite3_vfs *sqlite3_vfs_find(ଅଚଳ ଅକ୍ଷର *zVfs){
  sqlite3_vfs *pVfs = 0;
#ଯଦି SQLITE_THREADSAFE
  sqlite3_mutex *mutex;
#endif
#ifndef SQLITE_OMIT_AUTOINIT
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = sqlite3_initialize();
  ଯଦି( rc ) ଫେରନ୍ତୁ 0;
#endif
#ଯଦି SQLITE_THREADSAFE
  mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
#endif
  sqlite3_mutex_enter(mutex);
  ପାଇଁ(pVfs = vfsList; pVfs; pVfs=pVfs->pNext){
    ଯଦି( zVfs==0 ) ଭାଙ୍ଗ;
    ଯଦି( strcmp(zVfs, pVfs->zName)==0 ) ଭାଙ୍ଗ;
  }
  sqlite3_mutex_leave(mutex);
  ଫେରନ୍ତୁ pVfs;
}

/*
** Unlink a VFS from the linked list
*/
ସ୍ଥିର ଶୂନ୍ୟ vfsUnlink(sqlite3_vfs *pVfs){
  ପ୍ରତିଜ୍ଞା( sqlite3_mutex_held(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN)) );
  ଯଦି( pVfs==0 ){
    /* No-op */
  }ଅନ୍ୟଥା ଯଦି( vfsList==pVfs ){
    vfsList = pVfs->pNext;
  }ଅନ୍ୟଥା ଯଦି( vfsList ){
    sqlite3_vfs *p = vfsList;
    ଯେତେବେଳେ( p->pNext && p->pNext!=pVfs ){
      p = p->pNext;
    }
    ଯଦି( p->pNext==pVfs ){
      p->pNext = pVfs->pNext;
    }
  }
}

/*
** Register a VFS with the system.  It is harmless to register the same
** VFS multiple times.  The new VFS becomes the default if makeDflt is
** true.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_vfs_register(sqlite3_vfs *pVfs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା makeDflt){
  MUTEX_LOGIC(sqlite3_mutex *mutex;)
#ifndef SQLITE_OMIT_AUTOINIT
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = sqlite3_initialize();
  ଯଦି( rc ) ଫେରନ୍ତୁ rc;
#endif
#ifdef SQLITE_ENABLE_API_ARMOR
  ଯଦି( pVfs==0 ) ଫେରନ୍ତୁ SQLITE_MISUSE_BKPT;
#endif

  MUTEX_LOGIC( mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  sqlite3_mutex_enter(mutex);
  vfsUnlink(pVfs);
  ଯଦି( makeDflt || vfsList==0 ){
    pVfs->pNext = vfsList;
    vfsList = pVfs;
  }ଅନ୍ୟଥା{
    pVfs->pNext = vfsList->pNext;
    vfsList->pNext = pVfs;
  }
  ପ୍ରତିଜ୍ଞା(vfsList);
  sqlite3_mutex_leave(mutex);
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Unregister a VFS so that it is no longer accessible.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_vfs_unregister(sqlite3_vfs *pVfs){
  MUTEX_LOGIC(sqlite3_mutex *mutex;)
#ifndef SQLITE_OMIT_AUTOINIT
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = sqlite3_initialize();
  ଯଦି( rc ) ଫେରନ୍ତୁ rc;
#endif
  MUTEX_LOGIC( mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  sqlite3_mutex_enter(mutex);
  vfsUnlink(pVfs);
  sqlite3_mutex_leave(mutex);
  ଫେରନ୍ତୁ SQLITE_OK;
}
