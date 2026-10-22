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
#якщо defined(SQLITE_TEST)
ціле sqlite3_io_error_hit = 0;            /* Total number of I/O Errors */
ціле sqlite3_io_error_hardhit = 0;        /* Number of non-benign errors */
ціле sqlite3_io_error_pending = 0;        /* Count down to first I/O error */
ціле sqlite3_io_error_persist = 0;        /* True if I/O errors persist */
ціле sqlite3_io_error_benign = 0;         /* True if errors are benign */
ціле sqlite3_diskfull_pending = 0;
ціле sqlite3_diskfull = 0;
#endif /* defined(SQLITE_TEST) */

/*
** When testing, also keep a count of the number of open files.
*/
#якщо defined(SQLITE_TEST)
ціле sqlite3_open_file_count = 0;
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
#якщо defined(SQLITE_TEST)
ціле sqlite3_memdebug_vfs_oom_test = 1;
  #define DO_OS_MALLOC_TEST(x)                                       \
  якщо (sqlite3_memdebug_vfs_oom_test && (!x || !sqlite3JournalIsInMemory(x))) { \
    void *pTstAlloc = sqlite3Malloc(10);                             \
    якщо (!pTstAlloc) повернути SQLITE_IOERR_NOMEM_BKPT;                  \
    sqlite3_free(pTstAlloc);                                         \
  }
#інакше
  #define DO_OS_MALLOC_TEST(x)
#endif

/*
** The following routines are convenience wrappers around methods
** of the sqlite3_file object.  This is mostly just syntactic sugar. All
** of this would be completely automatic if SQLite were coded using
** C++ instead of plain old C.
*/
void sqlite3OsClose(sqlite3_file *pId){
  якщо( pId->pMethods ){
    pId->pMethods->xClose(pId);
    pId->pMethods = 0;
  }
}
ціле sqlite3OsRead(sqlite3_file *id, void *pBuf, ціле amt, i64 offset){
  DO_OS_MALLOC_TEST(id);
  повернути id->pMethods->xRead(id, pBuf, amt, offset);
}
ціле sqlite3OsWrite(sqlite3_file *id, const void *pBuf, ціле amt, i64 offset){
  DO_OS_MALLOC_TEST(id);
  повернути id->pMethods->xWrite(id, pBuf, amt, offset);
}
ціле sqlite3OsTruncate(sqlite3_file *id, i64 size){
  повернути id->pMethods->xTruncate(id, size);
}
ціле sqlite3OsSync(sqlite3_file *id, ціле flags){
  DO_OS_MALLOC_TEST(id);
  повернути flags ? id->pMethods->xSync(id, flags) : SQLITE_OK;
}
ціле sqlite3OsFileSize(sqlite3_file *id, i64 *pSize){
  DO_OS_MALLOC_TEST(id);
  повернути id->pMethods->xFileSize(id, pSize);
}
ціле sqlite3OsLock(sqlite3_file *id, ціле lockType){
  DO_OS_MALLOC_TEST(id);
  assert( lockType>=SQLITE_LOCK_SHARED && lockType<=SQLITE_LOCK_EXCLUSIVE );
  повернути id->pMethods->xLock(id, lockType);
}
ціле sqlite3OsUnlock(sqlite3_file *id, ціле lockType){
  assert( lockType==SQLITE_LOCK_NONE || lockType==SQLITE_LOCK_SHARED );
  повернути id->pMethods->xUnlock(id, lockType);
}
ціле sqlite3OsCheckReservedLock(sqlite3_file *id, ціле *pResOut){
  DO_OS_MALLOC_TEST(id);
  повернути id->pMethods->xCheckReservedLock(id, pResOut);
}

/*
** Use sqlite3OsFileControl() when we are doing something that might fail
** and we need to know about the failures.  Use sqlite3OsFileControlHint()
** when simply tossing information over the wall to the VFS and we do not
** really care if the VFS receives and understands the information since it
** is only a hint and can be safely ignored.  The sqlite3OsFileControlHint()
** routine has no return value since the return value would be meaningless.
*/
ціле sqlite3OsFileControl(sqlite3_file *id, ціле op, void *pArg){
  якщо( id->pMethods==0 ) повернути SQLITE_NOTFOUND;
#ifdef SQLITE_TEST
  якщо( op!=SQLITE_FCNTL_COMMIT_PHASETWO
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
  повернути id->pMethods->xFileControl(id, op, pArg);
}
void sqlite3OsFileControlHint(sqlite3_file *id, ціле op, void *pArg){
  якщо( id->pMethods ) (void)id->pMethods->xFileControl(id, op, pArg);
}

ціле sqlite3OsSectorSize(sqlite3_file *id){
  ціле (*xSectorSize)(sqlite3_file*) = id->pMethods->xSectorSize;
  повернути (xSectorSize ? xSectorSize(id) : SQLITE_DEFAULT_SECTOR_SIZE);
}
ціле sqlite3OsDeviceCharacteristics(sqlite3_file *id){
  якщо( NEVER(id->pMethods==0) ) повернути 0;
  повернути id->pMethods->xDeviceCharacteristics(id);
}
#ifndef SQLITE_OMIT_WAL
ціле sqlite3OsShmLock(sqlite3_file *id, ціле offset, ціле n, ціле flags){
  повернути id->pMethods->xShmLock(id, offset, n, flags);
}
void sqlite3OsShmBarrier(sqlite3_file *id){
  id->pMethods->xShmBarrier(id);
}
ціле sqlite3OsShmUnmap(sqlite3_file *id, ціле deleteFlag){
  повернути id->pMethods->xShmUnmap(id, deleteFlag);
}
ціле sqlite3OsShmMap(
  sqlite3_file *id,               /* Database file handle */
  ціле iPage,
  ціле pgsz,
  ціле bExtend,                    /* True to extend file if necessary */
  void volatile **pp              /* OUT: Pointer to mapping */
){
  DO_OS_MALLOC_TEST(id);
  повернути id->pMethods->xShmMap(id, iPage, pgsz, bExtend, pp);
}
#endif /* SQLITE_OMIT_WAL */

#якщо SQLITE_MAX_MMAP_SIZE>0
/* The real implementation of xFetch and xUnfetch */
ціле sqlite3OsFetch(sqlite3_file *id, i64 iOff, ціле iAmt, void **pp){
  DO_OS_MALLOC_TEST(id);
  повернути id->pMethods->xFetch(id, iOff, iAmt, pp);
}
ціле sqlite3OsUnfetch(sqlite3_file *id, i64 iOff, void *p){
  повернути id->pMethods->xUnfetch(id, iOff, p);
}
#інакше
/* No-op stubs to use when memory-mapped I/O is disabled */
ціле sqlite3OsFetch(sqlite3_file *id, i64 iOff, ціле iAmt, void **pp){
  *pp = 0;
  повернути SQLITE_OK;
}
ціле sqlite3OsUnfetch(sqlite3_file *id, i64 iOff, void *p){
  повернути SQLITE_OK;
}
#endif

/*
** The next group of routines are convenience wrappers around the
** VFS methods.
*/
ціле sqlite3OsOpen(
  sqlite3_vfs *pVfs,
  const char *zPath,
  sqlite3_file *pFile,
  ціле flags,
  ціле *pFlagsOut
){
  ціле rc;
  DO_OS_MALLOC_TEST(0);
  /* 0x87f7f is a mask of SQLITE_OPEN_ flags that are valid to be passed
  ** down into the VFS layer.  Some SQLITE_OPEN_ flags (for example,
  ** SQLITE_OPEN_FULLMUTEX or SQLITE_OPEN_SHAREDCACHE) are blocked before
  ** reaching the VFS. */
  assert( zPath || (flags & SQLITE_OPEN_EXCLUSIVE) );
  rc = pVfs->xOpen(pVfs, zPath, pFile, flags & 0x1087f7f, pFlagsOut);
  assert( rc==SQLITE_OK || pFile->pMethods==0 );
  повернути rc;
}
ціле sqlite3OsDelete(sqlite3_vfs *pVfs, const char *zPath, ціле dirSync){
  DO_OS_MALLOC_TEST(0);
  assert( dirSync==0 || dirSync==1 );
  повернути pVfs->xDelete!=0 ? pVfs->xDelete(pVfs, zPath, dirSync) : SQLITE_OK;
}
ціле sqlite3OsAccess(
  sqlite3_vfs *pVfs,
  const char *zPath,
  ціле flags,
  ціле *pResOut
){
  DO_OS_MALLOC_TEST(0);
  повернути pVfs->xAccess(pVfs, zPath, flags, pResOut);
}
ціле sqlite3OsFullPathname(
  sqlite3_vfs *pVfs,
  const char *zPath,
  ціле nPathOut,
  char *zPathOut
){
  DO_OS_MALLOC_TEST(0);
  zPathOut[0] = 0;
  повернути pVfs->xFullPathname(pVfs, zPath, nPathOut, zPathOut);
}
#ifndef SQLITE_OMIT_LOAD_EXTENSION
void *sqlite3OsDlOpen(sqlite3_vfs *pVfs, const char *zPath){
  assert( zPath!=0 );
  assert( strlen(zPath)<=SQLITE_MAX_PATHLEN );  /* tag-20210611-1 */
  повернути pVfs->xDlOpen(pVfs, zPath);
}
void sqlite3OsDlError(sqlite3_vfs *pVfs, ціле nByte, char *zBufOut){
  pVfs->xDlError(pVfs, nByte, zBufOut);
}
void (*sqlite3OsDlSym(sqlite3_vfs *pVfs, void *pHdle, const char *zSym))(void){
  повернути pVfs->xDlSym(pVfs, pHdle, zSym);
}
void sqlite3OsDlClose(sqlite3_vfs *pVfs, void *pHandle){
  pVfs->xDlClose(pVfs, pHandle);
}
#endif /* SQLITE_OMIT_LOAD_EXTENSION */
ціле sqlite3OsRandomness(sqlite3_vfs *pVfs, ціле nByte, char *zBufOut){
  якщо( sqlite3Config.iPrngSeed ){
    memset(zBufOut, 0, nByte);
    якщо( ALWAYS(nByte>(signed)sizeof(unsigned)) ) nByte = sizeof(unsigned ціле);
    memcpy(zBufOut, &sqlite3Config.iPrngSeed, nByte);
    повернути SQLITE_OK;
  }інакше{
    повернути pVfs->xRandomness(pVfs, nByte, zBufOut);
  }

}
ціле sqlite3OsSleep(sqlite3_vfs *pVfs, ціле nMicro){
  повернути pVfs->xSleep(pVfs, nMicro);
}
ціле sqlite3OsGetLastError(sqlite3_vfs *pVfs){
  повернути pVfs->xGetLastError ? pVfs->xGetLastError(pVfs, 0, 0) : 0;
}
ціле sqlite3OsCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *pTimeOut){
  ціле rc;
  /* IMPLEMENTATION-OF: R-49045-42493 SQLite will use the xCurrentTimeInt64()
  ** method to get the current date and time if that method is available
  ** (if iVersion is 2 or greater and the function pointer is not NULL) and
  ** will fall back to xCurrentTime() if xCurrentTimeInt64() is
  ** unavailable.
  */
  якщо( pVfs->iVersion>=2 && pVfs->xCurrentTimeInt64 ){
    rc = pVfs->xCurrentTimeInt64(pVfs, pTimeOut);
  }інакше{
    double r;
    rc = pVfs->xCurrentTime(pVfs, &r);
    *pTimeOut = sqlite3RealToI64(r*86400000.0);
  }
  повернути rc;
}

ціле sqlite3OsOpenMalloc(
  sqlite3_vfs *pVfs,
  const char *zFile,
  sqlite3_file **ppFile,
  ціле flags,
  ціле *pOutFlags
){
  ціле rc;
  sqlite3_file *pFile;
  pFile = (sqlite3_file *)sqlite3MallocZero(pVfs->szOsFile);
  якщо( pFile ){
    rc = sqlite3OsOpen(pVfs, zFile, pFile, flags, pOutFlags);
    якщо( rc!=SQLITE_OK ){
      sqlite3_free(pFile);
      *ppFile = 0;
    }інакше{
      *ppFile = pFile;
    }
  }інакше{
    *ppFile = 0;
    rc = SQLITE_NOMEM_BKPT;
  }
  assert( *ppFile!=0 || rc!=SQLITE_OK );
  повернути rc;
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
ціле sqlite3OsInit(void){
  void *p = sqlite3_malloc(10);
  якщо( p==0 ) повернути SQLITE_NOMEM_BKPT;
  sqlite3_free(p);
  повернути sqlite3_os_init();
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
sqlite3_vfs *sqlite3_vfs_find(const char *zVfs){
  sqlite3_vfs *pVfs = 0;
#якщо SQLITE_THREADSAFE
  sqlite3_mutex *mutex;
#endif
#ifndef SQLITE_OMIT_AUTOINIT
  ціле rc = sqlite3_initialize();
  якщо( rc ) повернути 0;
#endif
#якщо SQLITE_THREADSAFE
  mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
#endif
  sqlite3_mutex_enter(mutex);
  для(pVfs = vfsList; pVfs; pVfs=pVfs->pNext){
    якщо( zVfs==0 ) break;
    якщо( strcmp(zVfs, pVfs->zName)==0 ) break;
  }
  sqlite3_mutex_leave(mutex);
  повернути pVfs;
}

/*
** Unlink a VFS from the linked list
*/
static void vfsUnlink(sqlite3_vfs *pVfs){
  assert( sqlite3_mutex_held(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN)) );
  якщо( pVfs==0 ){
    /* No-op */
  }інакше якщо( vfsList==pVfs ){
    vfsList = pVfs->pNext;
  }інакше якщо( vfsList ){
    sqlite3_vfs *p = vfsList;
    поки( p->pNext && p->pNext!=pVfs ){
      p = p->pNext;
    }
    якщо( p->pNext==pVfs ){
      p->pNext = pVfs->pNext;
    }
  }
}

/*
** Register a VFS with the system.  It is harmless to register the same
** VFS multiple times.  The new VFS becomes the default if makeDflt is
** true.
*/
ціле sqlite3_vfs_register(sqlite3_vfs *pVfs, ціле makeDflt){
  MUTEX_LOGIC(sqlite3_mutex *mutex;)
#ifndef SQLITE_OMIT_AUTOINIT
  ціле rc = sqlite3_initialize();
  якщо( rc ) повернути rc;
#endif
#ifdef SQLITE_ENABLE_API_ARMOR
  якщо( pVfs==0 ) повернути SQLITE_MISUSE_BKPT;
#endif

  MUTEX_LOGIC( mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  sqlite3_mutex_enter(mutex);
  vfsUnlink(pVfs);
  якщо( makeDflt || vfsList==0 ){
    pVfs->pNext = vfsList;
    vfsList = pVfs;
  }інакше{
    pVfs->pNext = vfsList->pNext;
    vfsList->pNext = pVfs;
  }
  assert(vfsList);
  sqlite3_mutex_leave(mutex);
  повернути SQLITE_OK;
}

/*
** Unregister a VFS so that it is no longer accessible.
*/
ціле sqlite3_vfs_unregister(sqlite3_vfs *pVfs){
  MUTEX_LOGIC(sqlite3_mutex *mutex;)
#ifndef SQLITE_OMIT_AUTOINIT
  ціле rc = sqlite3_initialize();
  якщо( rc ) повернути rc;
#endif
  MUTEX_LOGIC( mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  sqlite3_mutex_enter(mutex);
  vfsUnlink(pVfs);
  sqlite3_mutex_leave(mutex);
  повернути SQLITE_OK;
}
