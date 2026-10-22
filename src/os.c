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
#அந்தர்பாவ "sqliteInt.h"

/*
** If we compile with the SQLITE_TEST macro set, then the following block
** of code will give us the ability to simulate a disk I/O error.  This
** is used for testing the I/O recovery logic.
*/
#எனில் defined(SQLITE_TEST)
முழு sqlite3_io_error_hit = 0;            /* Total number of I/O Errors */
முழு sqlite3_io_error_hardhit = 0;        /* Number of non-benign errors */
முழு sqlite3_io_error_pending = 0;        /* Count down to first I/O error */
முழு sqlite3_io_error_persist = 0;        /* True if I/O errors persist */
முழு sqlite3_io_error_benign = 0;         /* True if errors are benign */
முழு sqlite3_diskfull_pending = 0;
முழு sqlite3_diskfull = 0;
#endif /* defined(SQLITE_TEST) */

/*
** When testing, also keep a count of the number of open files.
*/
#எனில் defined(SQLITE_TEST)
முழு sqlite3_open_file_மொத்தம் = 0;
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
#எனில் defined(SQLITE_TEST)
முழு sqlite3_memdebug_vfs_oom_test = 1;
  #define DO_OS_MALLOC_TEST(x)                                       \
  எனில் (sqlite3_memdebug_vfs_oom_test && (!x || !sqlite3JournalIsInMemory(x))) { \
    வெறுமை *pTstAlloc = sqlite3Malloc(10);                             \
    எனில் (!pTstAlloc) திரும்பு SQLITE_IOERR_NOMEM_BKPT;                  \
    sqlite3_free(pTstAlloc);                                         \
  }
#இல்லையெனில்
  #define DO_OS_MALLOC_TEST(x)
#endif

/*
** The following routines are convenience wrappers around methods
** of the sqlite3_file object.  This is mostly just syntactic sugar. All
** of this would be completely automatic if SQLite were coded using
** C++ instead of plain old C.
*/
வெறுமை sqlite3OsClose(sqlite3_file *pId){
  எனில்( pId->pMethods ){
    pId->pMethods->xClose(pId);
    pId->pMethods = 0;
  }
}
முழு sqlite3OsRead(sqlite3_file *id, வெறுமை *pBuf, முழு amt, i64 offset){
  DO_OS_MALLOC_TEST(id);
  திரும்பு id->pMethods->xRead(id, pBuf, amt, offset);
}
முழு sqlite3OsWrite(sqlite3_file *id, மாறிலி வெறுமை *pBuf, முழு amt, i64 offset){
  DO_OS_MALLOC_TEST(id);
  திரும்பு id->pMethods->xWrite(id, pBuf, amt, offset);
}
முழு sqlite3OsTruncate(sqlite3_file *id, i64 size){
  திரும்பு id->pMethods->xTruncate(id, size);
}
முழு sqlite3OsSync(sqlite3_file *id, முழு flags){
  DO_OS_MALLOC_TEST(id);
  திரும்பு flags ? id->pMethods->xSync(id, flags) : SQLITE_OK;
}
முழு sqlite3OsFileSize(sqlite3_file *id, i64 *pSize){
  DO_OS_MALLOC_TEST(id);
  திரும்பு id->pMethods->xFileSize(id, pSize);
}
முழு sqlite3OsLock(sqlite3_file *id, முழு lockType){
  DO_OS_MALLOC_TEST(id);
  ப்ரதிஜ்ஞா( lockType>=SQLITE_LOCK_SHARED && lockType<=SQLITE_LOCK_EXCLUSIVE );
  திரும்பு id->pMethods->xLock(id, lockType);
}
முழு sqlite3OsUnlock(sqlite3_file *id, முழு lockType){
  ப்ரதிஜ்ஞா( lockType==SQLITE_LOCK_NONE || lockType==SQLITE_LOCK_SHARED );
  திரும்பு id->pMethods->xUnlock(id, lockType);
}
முழு sqlite3OsCheckReservedLock(sqlite3_file *id, முழு *pResOut){
  DO_OS_MALLOC_TEST(id);
  திரும்பு id->pMethods->xCheckReservedLock(id, pResOut);
}

/*
** Use sqlite3OsFileControl() when we are doing something that might fail
** and we need to know about the failures.  Use sqlite3OsFileControlHint()
** when simply tossing information over the wall to the VFS and we do not
** really care if the VFS receives and understands the information since it
** is only a hint and can be safely ignored.  The sqlite3OsFileControlHint()
** routine has no return value since the return value would be meaningless.
*/
முழு sqlite3OsFileControl(sqlite3_file *id, முழு op, வெறுமை *pArg){
  எனில்( id->pMethods==0 ) திரும்பு SQLITE_NOTFOUND;
#ifdef SQLITE_TEST
  எனில்( op!=SQLITE_FCNTL_COMMIT_PHASETWO
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
  திரும்பு id->pMethods->xFileControl(id, op, pArg);
}
வெறுமை sqlite3OsFileControlHint(sqlite3_file *id, முழு op, வெறுமை *pArg){
  எனில்( id->pMethods ) (வெறுமை)id->pMethods->xFileControl(id, op, pArg);
}

முழு sqlite3OsSectorSize(sqlite3_file *id){
  முழு (*xSectorSize)(sqlite3_file*) = id->pMethods->xSectorSize;
  திரும்பு (xSectorSize ? xSectorSize(id) : SQLITE_DEFAULT_SECTOR_SIZE);
}
முழு sqlite3OsDeviceCharacteristics(sqlite3_file *id){
  எனில்( NEVER(id->pMethods==0) ) திரும்பு 0;
  திரும்பு id->pMethods->xDeviceCharacteristics(id);
}
#ifndef SQLITE_OMIT_WAL
முழு sqlite3OsShmLock(sqlite3_file *id, முழு offset, முழு n, முழு flags){
  திரும்பு id->pMethods->xShmLock(id, offset, n, flags);
}
வெறுமை sqlite3OsShmBarrier(sqlite3_file *id){
  id->pMethods->xShmBarrier(id);
}
முழு sqlite3OsShmUnmap(sqlite3_file *id, முழு deleteFlag){
  திரும்பு id->pMethods->xShmUnmap(id, deleteFlag);
}
முழு sqlite3OsShmMap(
  sqlite3_file *id,               /* Database file handle */
  முழு iPage,
  முழு pgsz,
  முழு bExtend,                    /* True to extend file if necessary */
  வெறுமை நிலையற்ற **pp              /* OUT: Pointer to mapping */
){
  DO_OS_MALLOC_TEST(id);
  திரும்பு id->pMethods->xShmMap(id, iPage, pgsz, bExtend, pp);
}
#endif /* SQLITE_OMIT_WAL */

#எனில் SQLITE_MAX_MMAP_SIZE>0
/* The real implementation of xFetch and xUnfetch */
முழு sqlite3OsFetch(sqlite3_file *id, i64 iOff, முழு iAmt, வெறுமை **pp){
  DO_OS_MALLOC_TEST(id);
  திரும்பு id->pMethods->xFetch(id, iOff, iAmt, pp);
}
முழு sqlite3OsUnfetch(sqlite3_file *id, i64 iOff, வெறுமை *p){
  திரும்பு id->pMethods->xUnfetch(id, iOff, p);
}
#இல்லையெனில்
/* No-op stubs to use when memory-mapped I/O is disabled */
முழு sqlite3OsFetch(sqlite3_file *id, i64 iOff, முழு iAmt, வெறுமை **pp){
  *pp = 0;
  திரும்பு SQLITE_OK;
}
முழு sqlite3OsUnfetch(sqlite3_file *id, i64 iOff, வெறுமை *p){
  திரும்பு SQLITE_OK;
}
#endif

/*
** The next group of routines are convenience wrappers around the
** VFS methods.
*/
முழு sqlite3OsOpen(
  sqlite3_vfs *pVfs,
  மாறிலி எழுத்து *zPath,
  sqlite3_file *pFile,
  முழு flags,
  முழு *pFlagsOut
){
  முழு rc;
  DO_OS_MALLOC_TEST(0);
  /* 0x87f7f is a mask of SQLITE_OPEN_ flags that are valid to be passed
  ** down into the VFS layer.  Some SQLITE_OPEN_ flags (for example,
  ** SQLITE_OPEN_FULLMUTEX or SQLITE_OPEN_SHAREDCACHE) are blocked before
  ** reaching the VFS. */
  ப்ரதிஜ்ஞா( zPath || (flags & SQLITE_OPEN_EXCLUSIVE) );
  rc = pVfs->xOpen(pVfs, zPath, pFile, flags & 0x1087f7f, pFlagsOut);
  ப்ரதிஜ்ஞா( rc==SQLITE_OK || pFile->pMethods==0 );
  திரும்பு rc;
}
முழு sqlite3OsDelete(sqlite3_vfs *pVfs, மாறிலி எழுத்து *zPath, முழு dirSync){
  DO_OS_MALLOC_TEST(0);
  ப்ரதிஜ்ஞா( dirSync==0 || dirSync==1 );
  திரும்பு pVfs->xDelete!=0 ? pVfs->xDelete(pVfs, zPath, dirSync) : SQLITE_OK;
}
முழு sqlite3OsAccess(
  sqlite3_vfs *pVfs,
  மாறிலி எழுத்து *zPath,
  முழு flags,
  முழு *pResOut
){
  DO_OS_MALLOC_TEST(0);
  திரும்பு pVfs->xAccess(pVfs, zPath, flags, pResOut);
}
முழு sqlite3OsFullPathname(
  sqlite3_vfs *pVfs,
  மாறிலி எழுத்து *zPath,
  முழு nPathOut,
  எழுத்து *zPathOut
){
  DO_OS_MALLOC_TEST(0);
  zPathOut[0] = 0;
  திரும்பு pVfs->xFullPathname(pVfs, zPath, nPathOut, zPathOut);
}
#ifndef SQLITE_OMIT_LOAD_EXTENSION
வெறுமை *sqlite3OsDlOpen(sqlite3_vfs *pVfs, மாறிலி எழுத்து *zPath){
  ப்ரதிஜ்ஞா( zPath!=0 );
  ப்ரதிஜ்ஞா( strlen(zPath)<=SQLITE_MAX_PATHLEN );  /* tag-20210611-1 */
  திரும்பு pVfs->xDlOpen(pVfs, zPath);
}
வெறுமை sqlite3OsDlError(sqlite3_vfs *pVfs, முழு nByte, எழுத்து *zBufOut){
  pVfs->xDlError(pVfs, nByte, zBufOut);
}
வெறுமை (*sqlite3OsDlSym(sqlite3_vfs *pVfs, வெறுமை *pHdle, மாறிலி எழுத்து *zSym))(வெறுமை){
  திரும்பு pVfs->xDlSym(pVfs, pHdle, zSym);
}
வெறுமை sqlite3OsDlClose(sqlite3_vfs *pVfs, வெறுமை *pHandle){
  pVfs->xDlClose(pVfs, pHandle);
}
#endif /* SQLITE_OMIT_LOAD_EXTENSION */
முழு sqlite3OsRandomness(sqlite3_vfs *pVfs, முழு nByte, எழுத்து *zBufOut){
  எனில்( sqlite3Config.iPrngSeed ){
    memset(zBufOut, 0, nByte);
    எனில்( ALWAYS(nByte>(signed)அளவு(unsigned)) ) nByte = அளவு(unsigned முழு);
    memcpy(zBufOut, &sqlite3Config.iPrngSeed, nByte);
    திரும்பு SQLITE_OK;
  }இல்லையெனில்{
    திரும்பு pVfs->xRandomness(pVfs, nByte, zBufOut);
  }

}
முழு sqlite3OsSleep(sqlite3_vfs *pVfs, முழு nMicro){
  திரும்பு pVfs->xSleep(pVfs, nMicro);
}
முழு sqlite3OsGetLastError(sqlite3_vfs *pVfs){
  திரும்பு pVfs->xGetLastError ? pVfs->xGetLastError(pVfs, 0, 0) : 0;
}
முழு sqlite3OsCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *pTimeOut){
  முழு rc;
  /* IMPLEMENTATION-OF: R-49045-42493 SQLite will use the xCurrentTimeInt64()
  ** method to get the current date and time if that method is available
  ** (if iVersion is 2 or greater and the function pointer is not NULL) and
  ** will fall back to xCurrentTime() if xCurrentTimeInt64() is
  ** unavailable.
  */
  எனில்( pVfs->iVersion>=2 && pVfs->xCurrentTimeInt64 ){
    rc = pVfs->xCurrentTimeInt64(pVfs, pTimeOut);
  }இல்லையெனில்{
    இரட்டை r;
    rc = pVfs->xCurrentTime(pVfs, &r);
    *pTimeOut = sqlite3RealToI64(r*86400000.0);
  }
  திரும்பு rc;
}

முழு sqlite3OsOpenMalloc(
  sqlite3_vfs *pVfs,
  மாறிலி எழுத்து *zFile,
  sqlite3_file **ppFile,
  முழு flags,
  முழு *pOutFlags
){
  முழு rc;
  sqlite3_file *pFile;
  pFile = (sqlite3_file *)sqlite3MallocZero(pVfs->szOsFile);
  எனில்( pFile ){
    rc = sqlite3OsOpen(pVfs, zFile, pFile, flags, pOutFlags);
    எனில்( rc!=SQLITE_OK ){
      sqlite3_free(pFile);
      *ppFile = 0;
    }இல்லையெனில்{
      *ppFile = pFile;
    }
  }இல்லையெனில்{
    *ppFile = 0;
    rc = SQLITE_NOMEM_BKPT;
  }
  ப்ரதிஜ்ஞா( *ppFile!=0 || rc!=SQLITE_OK );
  திரும்பு rc;
}
வெறுமை sqlite3OsCloseFree(sqlite3_file *pFile){
  ப்ரதிஜ்ஞா( pFile );
  sqlite3OsClose(pFile);
  sqlite3_free(pFile);
}

/*
** This function is a wrapper around the OS specific implementation of
** sqlite3_os_init(). The purpose of the wrapper is to provide the
** ability to simulate a malloc failure, so that the handling of an
** error in sqlite3_os_init() by the upper layers can be tested.
*/
முழு sqlite3OsInit(வெறுமை){
  வெறுமை *p = sqlite3_malloc(10);
  எனில்( p==0 ) திரும்பு SQLITE_NOMEM_BKPT;
  sqlite3_free(p);
  திரும்பு sqlite3_os_init();
}

/*
** The list of all registered VFS implementations.
*/
நிலையான sqlite3_vfs * SQLITE_WSD vfsList = 0;
#define vfsList GLOBAL(sqlite3_vfs *, vfsList)

/*
** Locate a VFS by name.  If no name is given, simply return the
** first VFS on the list.
*/
sqlite3_vfs *sqlite3_vfs_find(மாறிலி எழுத்து *zVfs){
  sqlite3_vfs *pVfs = 0;
#எனில் SQLITE_THREADSAFE
  sqlite3_mutex *mutex;
#endif
#ifndef SQLITE_OMIT_AUTOINIT
  முழு rc = sqlite3_initialize();
  எனில்( rc ) திரும்பு 0;
#endif
#எனில் SQLITE_THREADSAFE
  mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
#endif
  sqlite3_mutex_enter(mutex);
  க்கு(pVfs = vfsList; pVfs; pVfs=pVfs->pNext){
    எனில்( zVfs==0 ) நிறுத்து;
    எனில்( strcmp(zVfs, pVfs->zName)==0 ) நிறுத்து;
  }
  sqlite3_mutex_leave(mutex);
  திரும்பு pVfs;
}

/*
** Unlink a VFS from the linked list
*/
நிலையான வெறுமை vfsUnlink(sqlite3_vfs *pVfs){
  ப்ரதிஜ்ஞா( sqlite3_mutex_held(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN)) );
  எனில்( pVfs==0 ){
    /* No-op */
  }இல்லையெனில் எனில்( vfsList==pVfs ){
    vfsList = pVfs->pNext;
  }இல்லையெனில் எனில்( vfsList ){
    sqlite3_vfs *p = vfsList;
    போது( p->pNext && p->pNext!=pVfs ){
      p = p->pNext;
    }
    எனில்( p->pNext==pVfs ){
      p->pNext = pVfs->pNext;
    }
  }
}

/*
** Register a VFS with the system.  It is harmless to register the same
** VFS multiple times.  The new VFS becomes the default if makeDflt is
** true.
*/
முழு sqlite3_vfs_register(sqlite3_vfs *pVfs, முழு makeDflt){
  MUTEX_LOGIC(sqlite3_mutex *mutex;)
#ifndef SQLITE_OMIT_AUTOINIT
  முழு rc = sqlite3_initialize();
  எனில்( rc ) திரும்பு rc;
#endif
#ifdef SQLITE_ENABLE_API_ARMOR
  எனில்( pVfs==0 ) திரும்பு SQLITE_MISUSE_BKPT;
#endif

  MUTEX_LOGIC( mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  sqlite3_mutex_enter(mutex);
  vfsUnlink(pVfs);
  எனில்( makeDflt || vfsList==0 ){
    pVfs->pNext = vfsList;
    vfsList = pVfs;
  }இல்லையெனில்{
    pVfs->pNext = vfsList->pNext;
    vfsList->pNext = pVfs;
  }
  ப்ரதிஜ்ஞா(vfsList);
  sqlite3_mutex_leave(mutex);
  திரும்பு SQLITE_OK;
}

/*
** Unregister a VFS so that it is no longer accessible.
*/
முழு sqlite3_vfs_unregister(sqlite3_vfs *pVfs){
  MUTEX_LOGIC(sqlite3_mutex *mutex;)
#ifndef SQLITE_OMIT_AUTOINIT
  முழு rc = sqlite3_initialize();
  எனில்( rc ) திரும்பு rc;
#endif
  MUTEX_LOGIC( mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  sqlite3_mutex_enter(mutex);
  vfsUnlink(pVfs);
  sqlite3_mutex_leave(mutex);
  திரும்பு SQLITE_OK;
}
