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
#include "sqliteInt.h"

/*
** If we compile with the SQLITE_TEST macro set, then the following block
** of code will give us the ability to simulate a disk I/O error.  This
** is used for testing the I/O recovery logic.
*/
#නම් defined(SQLITE_TEST)
පූර්ණ_සංඛ්‍යා sqlite3_io_error_hit = 0;            /* Total number of I/O Errors */
පූර්ණ_සංඛ්‍යා sqlite3_io_error_hardhit = 0;        /* Number of non-benign errors */
පූර්ණ_සංඛ්‍යා sqlite3_io_error_pending = 0;        /* Count down to first I/O error */
පූර්ණ_සංඛ්‍යා sqlite3_io_error_persist = 0;        /* True if I/O errors persist */
පූර්ණ_සංඛ්‍යා sqlite3_io_error_benign = 0;         /* True if errors are benign */
පූර්ණ_සංඛ්‍යා sqlite3_diskfull_pending = 0;
පූර්ණ_සංඛ්‍යා sqlite3_diskfull = 0;
#endif /* defined(SQLITE_TEST) */

/*
** When testing, also keep a count of the number of open files.
*/
#නම් defined(SQLITE_TEST)
පූර්ණ_සංඛ්‍යා sqlite3_open_file_count = 0;
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
#නම් defined(SQLITE_TEST)
පූර්ණ_සංඛ්‍යා sqlite3_memdebug_vfs_oom_test = 1;
  #define DO_OS_MALLOC_TEST(x)                                       \
  නම් (sqlite3_memdebug_vfs_oom_test && (!x || !sqlite3JournalIsInMemory(x))) { \
    void *pTstAlloc = sqlite3Malloc(10);                             \
    නම් (!pTstAlloc) ආපසු SQLITE_IOERR_NOMEM_BKPT;                  \
    sqlite3_free(pTstAlloc);                                         \
  }
#නැතහොත්
  #define DO_OS_MALLOC_TEST(x)
#endif

/*
** The following routines are convenience wrappers around methods
** of the sqlite3_file object.  This is mostly just syntactic sugar. All
** of this would be completely automatic if SQLite were coded using
** C++ instead of plain old C.
*/
void sqlite3OsClose(sqlite3_file *pId){
  නම්( pId->pMethods ){
    pId->pMethods->xClose(pId);
    pId->pMethods = 0;
  }
}
පූර්ණ_සංඛ්‍යා sqlite3OsRead(sqlite3_file *id, void *pBuf, පූර්ණ_සංඛ්‍යා amt, i64 offset){
  DO_OS_MALLOC_TEST(id);
  ආපසු id->pMethods->xRead(id, pBuf, amt, offset);
}
පූර්ණ_සංඛ්‍යා sqlite3OsWrite(sqlite3_file *id, const void *pBuf, පූර්ණ_සංඛ්‍යා amt, i64 offset){
  DO_OS_MALLOC_TEST(id);
  ආපසු id->pMethods->xWrite(id, pBuf, amt, offset);
}
පූර්ණ_සංඛ්‍යා sqlite3OsTruncate(sqlite3_file *id, i64 size){
  ආපසු id->pMethods->xTruncate(id, size);
}
පූර්ණ_සංඛ්‍යා sqlite3OsSync(sqlite3_file *id, පූර්ණ_සංඛ්‍යා flags){
  DO_OS_MALLOC_TEST(id);
  ආපසු flags ? id->pMethods->xSync(id, flags) : SQLITE_OK;
}
පූර්ණ_සංඛ්‍යා sqlite3OsFileSize(sqlite3_file *id, i64 *pSize){
  DO_OS_MALLOC_TEST(id);
  ආපසු id->pMethods->xFileSize(id, pSize);
}
පූර්ණ_සංඛ්‍යා sqlite3OsLock(sqlite3_file *id, පූර්ණ_සංඛ්‍යා lockType){
  DO_OS_MALLOC_TEST(id);
  assert( lockType>=SQLITE_LOCK_SHARED && lockType<=SQLITE_LOCK_EXCLUSIVE );
  ආපසු id->pMethods->xLock(id, lockType);
}
පූර්ණ_සංඛ්‍යා sqlite3OsUnlock(sqlite3_file *id, පූර්ණ_සංඛ්‍යා lockType){
  assert( lockType==SQLITE_LOCK_NONE || lockType==SQLITE_LOCK_SHARED );
  ආපසු id->pMethods->xUnlock(id, lockType);
}
පූර්ණ_සංඛ්‍යා sqlite3OsCheckReservedLock(sqlite3_file *id, පූර්ණ_සංඛ්‍යා *pResOut){
  DO_OS_MALLOC_TEST(id);
  ආපසු id->pMethods->xCheckReservedLock(id, pResOut);
}

/*
** Use sqlite3OsFileControl() when we are doing something that might fail
** and we need to know about the failures.  Use sqlite3OsFileControlHint()
** when simply tossing information over the wall to the VFS and we do not
** really care if the VFS receives and understands the information since it
** is only a hint and can be safely ignored.  The sqlite3OsFileControlHint()
** routine has no return value since the return value would be meaningless.
*/
පූර්ණ_සංඛ්‍යා sqlite3OsFileControl(sqlite3_file *id, පූර්ණ_සංඛ්‍යා op, void *pArg){
  නම්( id->pMethods==0 ) ආපසු SQLITE_NOTFOUND;
#ifdef SQLITE_TEST
  නම්( op!=SQLITE_FCNTL_COMMIT_PHASETWO
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
  ආපසු id->pMethods->xFileControl(id, op, pArg);
}
void sqlite3OsFileControlHint(sqlite3_file *id, පූර්ණ_සංඛ්‍යා op, void *pArg){
  නම්( id->pMethods ) (void)id->pMethods->xFileControl(id, op, pArg);
}

පූර්ණ_සංඛ්‍යා sqlite3OsSectorSize(sqlite3_file *id){
  පූර්ණ_සංඛ්‍යා (*xSectorSize)(sqlite3_file*) = id->pMethods->xSectorSize;
  ආපසු (xSectorSize ? xSectorSize(id) : SQLITE_DEFAULT_SECTOR_SIZE);
}
පූර්ණ_සංඛ්‍යා sqlite3OsDeviceCharacteristics(sqlite3_file *id){
  නම්( NEVER(id->pMethods==0) ) ආපසු 0;
  ආපසු id->pMethods->xDeviceCharacteristics(id);
}
#ifndef SQLITE_OMIT_WAL
පූර්ණ_සංඛ්‍යා sqlite3OsShmLock(sqlite3_file *id, පූර්ණ_සංඛ්‍යා offset, පූර්ණ_සංඛ්‍යා n, පූර්ණ_සංඛ්‍යා flags){
  ආපසු id->pMethods->xShmLock(id, offset, n, flags);
}
void sqlite3OsShmBarrier(sqlite3_file *id){
  id->pMethods->xShmBarrier(id);
}
පූර්ණ_සංඛ්‍යා sqlite3OsShmUnmap(sqlite3_file *id, පූර්ණ_සංඛ්‍යා deleteFlag){
  ආපසු id->pMethods->xShmUnmap(id, deleteFlag);
}
පූර්ණ_සංඛ්‍යා sqlite3OsShmMap(
  sqlite3_file *id,               /* Database file handle */
  පූර්ණ_සංඛ්‍යා iPage,
  පූර්ණ_සංඛ්‍යා pgsz,
  පූර්ණ_සංඛ්‍යා bExtend,                    /* True to extend file if necessary */
  void ca_ncala **pp              /* OUT: Pointer to mapping */
){
  DO_OS_MALLOC_TEST(id);
  ආපසු id->pMethods->xShmMap(id, iPage, pgsz, bExtend, pp);
}
#endif /* SQLITE_OMIT_WAL */

#නම් SQLITE_MAX_MMAP_SIZE>0
/* The real implementation of xFetch and xUnfetch */
පූර්ණ_සංඛ්‍යා sqlite3OsFetch(sqlite3_file *id, i64 iOff, පූර්ණ_සංඛ්‍යා iAmt, void **pp){
  DO_OS_MALLOC_TEST(id);
  ආපසු id->pMethods->xFetch(id, iOff, iAmt, pp);
}
පූර්ණ_සංඛ්‍යා sqlite3OsUnfetch(sqlite3_file *id, i64 iOff, void *p){
  ආපසු id->pMethods->xUnfetch(id, iOff, p);
}
#නැතහොත්
/* No-op stubs to use when memory-mapped I/O is disabled */
පූර්ණ_සංඛ්‍යා sqlite3OsFetch(sqlite3_file *id, i64 iOff, පූර්ණ_සංඛ්‍යා iAmt, void **pp){
  *pp = 0;
  ආපසු SQLITE_OK;
}
පූර්ණ_සංඛ්‍යා sqlite3OsUnfetch(sqlite3_file *id, i64 iOff, void *p){
  ආපසු SQLITE_OK;
}
#endif

/*
** The next group of routines are convenience wrappers around the
** VFS methods.
*/
පූර්ණ_සංඛ්‍යා sqlite3OsOpen(
  sqlite3_vfs *pVfs,
  const var_na *zPath,
  sqlite3_file *pFile,
  පූර්ණ_සංඛ්‍යා flags,
  පූර්ණ_සංඛ්‍යා *pFlagsOut
){
  පූර්ණ_සංඛ්‍යා rc;
  DO_OS_MALLOC_TEST(0);
  /* 0x87f7f is a mask of SQLITE_OPEN_ flags that are valid to be passed
  ** down into the VFS layer.  Some SQLITE_OPEN_ flags (for example,
  ** SQLITE_OPEN_FULLMUTEX or SQLITE_OPEN_SHAREDCACHE) are blocked before
  ** reaching the VFS. */
  assert( zPath || (flags & SQLITE_OPEN_EXCLUSIVE) );
  rc = pVfs->xOpen(pVfs, zPath, pFile, flags & 0x1087f7f, pFlagsOut);
  assert( rc==SQLITE_OK || pFile->pMethods==0 );
  ආපසු rc;
}
පූර්ණ_සංඛ්‍යා sqlite3OsDelete(sqlite3_vfs *pVfs, const var_na *zPath, පූර්ණ_සංඛ්‍යා dirSync){
  DO_OS_MALLOC_TEST(0);
  assert( dirSync==0 || dirSync==1 );
  ආපසු pVfs->xDelete!=0 ? pVfs->xDelete(pVfs, zPath, dirSync) : SQLITE_OK;
}
පූර්ණ_සංඛ්‍යා sqlite3OsAccess(
  sqlite3_vfs *pVfs,
  const var_na *zPath,
  පූර්ණ_සංඛ්‍යා flags,
  පූර්ණ_සංඛ්‍යා *pResOut
){
  DO_OS_MALLOC_TEST(0);
  ආපසු pVfs->xAccess(pVfs, zPath, flags, pResOut);
}
පූර්ණ_සංඛ්‍යා sqlite3OsFullPathname(
  sqlite3_vfs *pVfs,
  const var_na *zPath,
  පූර්ණ_සංඛ්‍යා nPathOut,
  var_na *zPathOut
){
  DO_OS_MALLOC_TEST(0);
  zPathOut[0] = 0;
  ආපසු pVfs->xFullPathname(pVfs, zPath, nPathOut, zPathOut);
}
#ifndef SQLITE_OMIT_LOAD_EXTENSION
void *sqlite3OsDlOpen(sqlite3_vfs *pVfs, const var_na *zPath){
  assert( zPath!=0 );
  assert( strlen(zPath)<=SQLITE_MAX_PATHLEN );  /* tag-20210611-1 */
  ආපසු pVfs->xDlOpen(pVfs, zPath);
}
void sqlite3OsDlError(sqlite3_vfs *pVfs, පූර්ණ_සංඛ්‍යා nByte, var_na *zBufOut){
  pVfs->xDlError(pVfs, nByte, zBufOut);
}
void (*sqlite3OsDlSym(sqlite3_vfs *pVfs, void *pHdle, const var_na *zSym))(void){
  ආපසු pVfs->xDlSym(pVfs, pHdle, zSym);
}
void sqlite3OsDlClose(sqlite3_vfs *pVfs, void *pHandle){
  pVfs->xDlClose(pVfs, pHandle);
}
#endif /* SQLITE_OMIT_LOAD_EXTENSION */
පූර්ණ_සංඛ්‍යා sqlite3OsRandomness(sqlite3_vfs *pVfs, පූර්ණ_සංඛ්‍යා nByte, var_na *zBufOut){
  නම්( sqlite3Config.iPrngSeed ){
    memset(zBufOut, 0, nByte);
    නම්( ALWAYS(nByte>(signed)parimaa_na(unsigned)) ) nByte = parimaa_na(unsigned පූර්ණ_සංඛ්‍යා);
    memcpy(zBufOut, &sqlite3Config.iPrngSeed, nByte);
    ආපසු SQLITE_OK;
  }නැතහොත්{
    ආපසු pVfs->xRandomness(pVfs, nByte, zBufOut);
  }

}
පූර්ණ_සංඛ්‍යා sqlite3OsSleep(sqlite3_vfs *pVfs, පූර්ණ_සංඛ්‍යා nMicro){
  ආපසු pVfs->xSleep(pVfs, nMicro);
}
පූර්ණ_සංඛ්‍යා sqlite3OsGetLastError(sqlite3_vfs *pVfs){
  ආපසු pVfs->xGetLastError ? pVfs->xGetLastError(pVfs, 0, 0) : 0;
}
පූර්ණ_සංඛ්‍යා sqlite3OsCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *pTimeOut){
  පූර්ණ_සංඛ්‍යා rc;
  /* IMPLEMENTATION-OF: R-49045-42493 SQLite will use the xCurrentTimeInt64()
  ** method to get the current date and time if that method is available
  ** (if iVersion is 2 or greater and the function pointer is not NULL) and
  ** will fall back to xCurrentTime() if xCurrentTimeInt64() is
  ** unavailable.
  */
  නම්( pVfs->iVersion>=2 && pVfs->xCurrentTimeInt64 ){
    rc = pVfs->xCurrentTimeInt64(pVfs, pTimeOut);
  }නැතහොත්{
    dviguu_na r;
    rc = pVfs->xCurrentTime(pVfs, &r);
    *pTimeOut = sqlite3RealToI64(r*86400000.0);
  }
  ආපසු rc;
}

පූර්ණ_සංඛ්‍යා sqlite3OsOpenMalloc(
  sqlite3_vfs *pVfs,
  const var_na *zFile,
  sqlite3_file **ppFile,
  පූර්ණ_සංඛ්‍යා flags,
  පූර්ණ_සංඛ්‍යා *pOutFlags
){
  පූර්ණ_සංඛ්‍යා rc;
  sqlite3_file *pFile;
  pFile = (sqlite3_file *)sqlite3MallocZero(pVfs->szOsFile);
  නම්( pFile ){
    rc = sqlite3OsOpen(pVfs, zFile, pFile, flags, pOutFlags);
    නම්( rc!=SQLITE_OK ){
      sqlite3_free(pFile);
      *ppFile = 0;
    }නැතහොත්{
      *ppFile = pFile;
    }
  }නැතහොත්{
    *ppFile = 0;
    rc = SQLITE_NOMEM_BKPT;
  }
  assert( *ppFile!=0 || rc!=SQLITE_OK );
  ආපසු rc;
}
void sqlite3OsCloseFree(sqlite3_file *pFile){
  assert( pFile );
  sqlite3OsClose(pFile);
  sqlite3_free(pFile);
}

/*
** This function is a wrapper around the OS specific implementation of
** sqlite3_os_init(). The purpose of the wrapper is to provide the
** ability to simulate a malloc failure, so that the handling of an
** error in sqlite3_os_init() by the upper layers can be tested.
*/
පූර්ණ_සංඛ්‍යා sqlite3OsInit(void){
  void *p = sqlite3_malloc(10);
  නම්( p==0 ) ආපසු SQLITE_NOMEM_BKPT;
  sqlite3_free(p);
  ආපසු sqlite3_os_init();
}

/*
** The list of all registered VFS implementations.
*/
static sqlite3_vfs * SQLITE_WSD vfsList = 0;
#define vfsList GLOBAL(sqlite3_vfs *, vfsList)

/*
** Locate a VFS by name.  If no name is given, simply return the
** first VFS on the list.
*/
sqlite3_vfs *sqlite3_vfs_find(const var_na *zVfs){
  sqlite3_vfs *pVfs = 0;
#නම් SQLITE_THREADSAFE
  sqlite3_mutex *mutex;
#endif
#ifndef SQLITE_OMIT_AUTOINIT
  පූර්ණ_සංඛ්‍යා rc = sqlite3_initialize();
  නම්( rc ) ආපසු 0;
#endif
#නම් SQLITE_THREADSAFE
  mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
#endif
  sqlite3_mutex_enter(mutex);
  සඳහා(pVfs = vfsList; pVfs; pVfs=pVfs->pNext){
    නම්( zVfs==0 ) bha_nga;
    නම්( strcmp(zVfs, pVfs->zName)==0 ) bha_nga;
  }
  sqlite3_mutex_leave(mutex);
  ආපසු pVfs;
}

/*
** Unlink a VFS from the linked list
*/
static void vfsUnlink(sqlite3_vfs *pVfs){
  assert( sqlite3_mutex_held(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN)) );
  නම්( pVfs==0 ){
    /* No-op */
  }නැතහොත් නම්( vfsList==pVfs ){
    vfsList = pVfs->pNext;
  }නැතහොත් නම්( vfsList ){
    sqlite3_vfs *p = vfsList;
    අතරතුර( p->pNext && p->pNext!=pVfs ){
      p = p->pNext;
    }
    නම්( p->pNext==pVfs ){
      p->pNext = pVfs->pNext;
    }
  }
}

/*
** Register a VFS with the system.  It is harmless to register the same
** VFS multiple times.  The new VFS becomes the default if makeDflt is
** true.
*/
පූර්ණ_සංඛ්‍යා sqlite3_vfs_register(sqlite3_vfs *pVfs, පූර්ණ_සංඛ්‍යා makeDflt){
  MUTEX_LOGIC(sqlite3_mutex *mutex;)
#ifndef SQLITE_OMIT_AUTOINIT
  පූර්ණ_සංඛ්‍යා rc = sqlite3_initialize();
  නම්( rc ) ආපසු rc;
#endif
#ifdef SQLITE_ENABLE_API_ARMOR
  නම්( pVfs==0 ) ආපසු SQLITE_MISUSE_BKPT;
#endif

  MUTEX_LOGIC( mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  sqlite3_mutex_enter(mutex);
  vfsUnlink(pVfs);
  නම්( makeDflt || vfsList==0 ){
    pVfs->pNext = vfsList;
    vfsList = pVfs;
  }නැතහොත්{
    pVfs->pNext = vfsList->pNext;
    vfsList->pNext = pVfs;
  }
  assert(vfsList);
  sqlite3_mutex_leave(mutex);
  ආපසු SQLITE_OK;
}

/*
** Unregister a VFS so that it is no longer accessible.
*/
පූර්ණ_සංඛ්‍යා sqlite3_vfs_unregister(sqlite3_vfs *pVfs){
  MUTEX_LOGIC(sqlite3_mutex *mutex;)
#ifndef SQLITE_OMIT_AUTOINIT
  පූර්ණ_සංඛ්‍යා rc = sqlite3_initialize();
  නම්( rc ) ආපසු rc;
#endif
  MUTEX_LOGIC( mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  sqlite3_mutex_enter(mutex);
  vfsUnlink(pVfs);
  sqlite3_mutex_leave(mutex);
  ආපසු SQLITE_OK;
}
