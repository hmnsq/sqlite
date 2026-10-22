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
#如果 defined(SQLITE_TEST)
整数 sqlite3_io_error_hit = 0;            /* Total number of I/O Errors */
整数 sqlite3_io_error_hardhit = 0;        /* Number of non-benign errors */
整数 sqlite3_io_error_pending = 0;        /* Count down to first I/O error */
整数 sqlite3_io_error_persist = 0;        /* True if I/O errors persist */
整数 sqlite3_io_error_benign = 0;         /* True if errors are benign */
整数 sqlite3_diskfull_pending = 0;
整数 sqlite3_diskfull = 0;
#endif /* defined(SQLITE_TEST) */

/*
** When testing, also keep a count of the number of open files.
*/
#如果 defined(SQLITE_TEST)
整数 sqlite3_open_file_count = 0;
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
#如果 defined(SQLITE_TEST)
整数 sqlite3_memdebug_vfs_oom_test = 1;
  #define DO_OS_MALLOC_TEST(x)                                       \
  如果 (sqlite3_memdebug_vfs_oom_test && (!x || !sqlite3JournalIsInMemory(x))) { \
    void *pTstAlloc = sqlite3Malloc(10);                             \
    如果 (!pTstAlloc) 返回 SQLITE_IOERR_NOMEM_BKPT;                  \
    sqlite3_free(pTstAlloc);                                         \
  }
#否则
  #define DO_OS_MALLOC_TEST(x)
#endif

/*
** The following routines are convenience wrappers around methods
** of the sqlite3_file object.  This is mostly just syntactic sugar. All
** of this would be completely automatic if SQLite were coded using
** C++ instead of plain old C.
*/
void sqlite3OsClose(sqlite3_file *pId){
  如果( pId->pMethods ){
    pId->pMethods->xClose(pId);
    pId->pMethods = 0;
  }
}
整数 sqlite3OsRead(sqlite3_file *id, void *pBuf, 整数 amt, i64 offset){
  DO_OS_MALLOC_TEST(id);
  返回 id->pMethods->xRead(id, pBuf, amt, offset);
}
整数 sqlite3OsWrite(sqlite3_file *id, const void *pBuf, 整数 amt, i64 offset){
  DO_OS_MALLOC_TEST(id);
  返回 id->pMethods->xWrite(id, pBuf, amt, offset);
}
整数 sqlite3OsTruncate(sqlite3_file *id, i64 size){
  返回 id->pMethods->xTruncate(id, size);
}
整数 sqlite3OsSync(sqlite3_file *id, 整数 flags){
  DO_OS_MALLOC_TEST(id);
  返回 flags ? id->pMethods->xSync(id, flags) : SQLITE_OK;
}
整数 sqlite3OsFileSize(sqlite3_file *id, i64 *pSize){
  DO_OS_MALLOC_TEST(id);
  返回 id->pMethods->xFileSize(id, pSize);
}
整数 sqlite3OsLock(sqlite3_file *id, 整数 lockType){
  DO_OS_MALLOC_TEST(id);
  assert( lockType>=SQLITE_LOCK_SHARED && lockType<=SQLITE_LOCK_EXCLUSIVE );
  返回 id->pMethods->xLock(id, lockType);
}
整数 sqlite3OsUnlock(sqlite3_file *id, 整数 lockType){
  assert( lockType==SQLITE_LOCK_NONE || lockType==SQLITE_LOCK_SHARED );
  返回 id->pMethods->xUnlock(id, lockType);
}
整数 sqlite3OsCheckReservedLock(sqlite3_file *id, 整数 *pResOut){
  DO_OS_MALLOC_TEST(id);
  返回 id->pMethods->xCheckReservedLock(id, pResOut);
}

/*
** Use sqlite3OsFileControl() when we are doing something that might fail
** and we need to know about the failures.  Use sqlite3OsFileControlHint()
** when simply tossing information over the wall to the VFS and we do not
** really care if the VFS receives and understands the information since it
** is only a hint and can be safely ignored.  The sqlite3OsFileControlHint()
** routine has no return value since the return value would be meaningless.
*/
整数 sqlite3OsFileControl(sqlite3_file *id, 整数 op, void *pArg){
  如果( id->pMethods==0 ) 返回 SQLITE_NOTFOUND;
#ifdef SQLITE_TEST
  如果( op!=SQLITE_FCNTL_COMMIT_PHASETWO
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
  返回 id->pMethods->xFileControl(id, op, pArg);
}
void sqlite3OsFileControlHint(sqlite3_file *id, 整数 op, void *pArg){
  如果( id->pMethods ) (void)id->pMethods->xFileControl(id, op, pArg);
}

整数 sqlite3OsSectorSize(sqlite3_file *id){
  整数 (*xSectorSize)(sqlite3_file*) = id->pMethods->xSectorSize;
  返回 (xSectorSize ? xSectorSize(id) : SQLITE_DEFAULT_SECTOR_SIZE);
}
整数 sqlite3OsDeviceCharacteristics(sqlite3_file *id){
  如果( NEVER(id->pMethods==0) ) 返回 0;
  返回 id->pMethods->xDeviceCharacteristics(id);
}
#ifndef SQLITE_OMIT_WAL
整数 sqlite3OsShmLock(sqlite3_file *id, 整数 offset, 整数 n, 整数 flags){
  返回 id->pMethods->xShmLock(id, offset, n, flags);
}
void sqlite3OsShmBarrier(sqlite3_file *id){
  id->pMethods->xShmBarrier(id);
}
整数 sqlite3OsShmUnmap(sqlite3_file *id, 整数 deleteFlag){
  返回 id->pMethods->xShmUnmap(id, deleteFlag);
}
整数 sqlite3OsShmMap(
  sqlite3_file *id,               /* Database file handle */
  整数 iPage,
  整数 pgsz,
  整数 bExtend,                    /* True to extend file if necessary */
  void volatile **pp              /* OUT: Pointer to mapping */
){
  DO_OS_MALLOC_TEST(id);
  返回 id->pMethods->xShmMap(id, iPage, pgsz, bExtend, pp);
}
#endif /* SQLITE_OMIT_WAL */

#如果 SQLITE_MAX_MMAP_SIZE>0
/* The real implementation of xFetch and xUnfetch */
整数 sqlite3OsFetch(sqlite3_file *id, i64 iOff, 整数 iAmt, void **pp){
  DO_OS_MALLOC_TEST(id);
  返回 id->pMethods->xFetch(id, iOff, iAmt, pp);
}
整数 sqlite3OsUnfetch(sqlite3_file *id, i64 iOff, void *p){
  返回 id->pMethods->xUnfetch(id, iOff, p);
}
#否则
/* No-op stubs to use when memory-mapped I/O is disabled */
整数 sqlite3OsFetch(sqlite3_file *id, i64 iOff, 整数 iAmt, void **pp){
  *pp = 0;
  返回 SQLITE_OK;
}
整数 sqlite3OsUnfetch(sqlite3_file *id, i64 iOff, void *p){
  返回 SQLITE_OK;
}
#endif

/*
** The next group of routines are convenience wrappers around the
** VFS methods.
*/
整数 sqlite3OsOpen(
  sqlite3_vfs *pVfs,
  const char *zPath,
  sqlite3_file *pFile,
  整数 flags,
  整数 *pFlagsOut
){
  整数 rc;
  DO_OS_MALLOC_TEST(0);
  /* 0x87f7f is a mask of SQLITE_OPEN_ flags that are valid to be passed
  ** down into the VFS layer.  Some SQLITE_OPEN_ flags (for example,
  ** SQLITE_OPEN_FULLMUTEX or SQLITE_OPEN_SHAREDCACHE) are blocked before
  ** reaching the VFS. */
  assert( zPath || (flags & SQLITE_OPEN_EXCLUSIVE) );
  rc = pVfs->xOpen(pVfs, zPath, pFile, flags & 0x1087f7f, pFlagsOut);
  assert( rc==SQLITE_OK || pFile->pMethods==0 );
  返回 rc;
}
整数 sqlite3OsDelete(sqlite3_vfs *pVfs, const char *zPath, 整数 dirSync){
  DO_OS_MALLOC_TEST(0);
  assert( dirSync==0 || dirSync==1 );
  返回 pVfs->xDelete!=0 ? pVfs->xDelete(pVfs, zPath, dirSync) : SQLITE_OK;
}
整数 sqlite3OsAccess(
  sqlite3_vfs *pVfs,
  const char *zPath,
  整数 flags,
  整数 *pResOut
){
  DO_OS_MALLOC_TEST(0);
  返回 pVfs->xAccess(pVfs, zPath, flags, pResOut);
}
整数 sqlite3OsFullPathname(
  sqlite3_vfs *pVfs,
  const char *zPath,
  整数 nPathOut,
  char *zPathOut
){
  DO_OS_MALLOC_TEST(0);
  zPathOut[0] = 0;
  返回 pVfs->xFullPathname(pVfs, zPath, nPathOut, zPathOut);
}
#ifndef SQLITE_OMIT_LOAD_EXTENSION
void *sqlite3OsDlOpen(sqlite3_vfs *pVfs, const char *zPath){
  assert( zPath!=0 );
  assert( strlen(zPath)<=SQLITE_MAX_PATHLEN );  /* tag-20210611-1 */
  返回 pVfs->xDlOpen(pVfs, zPath);
}
void sqlite3OsDlError(sqlite3_vfs *pVfs, 整数 nByte, char *zBufOut){
  pVfs->xDlError(pVfs, nByte, zBufOut);
}
void (*sqlite3OsDlSym(sqlite3_vfs *pVfs, void *pHdle, const char *zSym))(void){
  返回 pVfs->xDlSym(pVfs, pHdle, zSym);
}
void sqlite3OsDlClose(sqlite3_vfs *pVfs, void *pHandle){
  pVfs->xDlClose(pVfs, pHandle);
}
#endif /* SQLITE_OMIT_LOAD_EXTENSION */
整数 sqlite3OsRandomness(sqlite3_vfs *pVfs, 整数 nByte, char *zBufOut){
  如果( sqlite3Config.iPrngSeed ){
    memset(zBufOut, 0, nByte);
    如果( ALWAYS(nByte>(signed)sizeof(unsigned)) ) nByte = sizeof(unsigned 整数);
    memcpy(zBufOut, &sqlite3Config.iPrngSeed, nByte);
    返回 SQLITE_OK;
  }否则{
    返回 pVfs->xRandomness(pVfs, nByte, zBufOut);
  }

}
整数 sqlite3OsSleep(sqlite3_vfs *pVfs, 整数 nMicro){
  返回 pVfs->xSleep(pVfs, nMicro);
}
整数 sqlite3OsGetLastError(sqlite3_vfs *pVfs){
  返回 pVfs->xGetLastError ? pVfs->xGetLastError(pVfs, 0, 0) : 0;
}
整数 sqlite3OsCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *pTimeOut){
  整数 rc;
  /* IMPLEMENTATION-OF: R-49045-42493 SQLite will use the xCurrentTimeInt64()
  ** method to get the current date and time if that method is available
  ** (if iVersion is 2 or greater and the function pointer is not NULL) and
  ** will fall back to xCurrentTime() if xCurrentTimeInt64() is
  ** unavailable.
  */
  如果( pVfs->iVersion>=2 && pVfs->xCurrentTimeInt64 ){
    rc = pVfs->xCurrentTimeInt64(pVfs, pTimeOut);
  }否则{
    double r;
    rc = pVfs->xCurrentTime(pVfs, &r);
    *pTimeOut = sqlite3RealToI64(r*86400000.0);
  }
  返回 rc;
}

整数 sqlite3OsOpenMalloc(
  sqlite3_vfs *pVfs,
  const char *zFile,
  sqlite3_file **ppFile,
  整数 flags,
  整数 *pOutFlags
){
  整数 rc;
  sqlite3_file *pFile;
  pFile = (sqlite3_file *)sqlite3MallocZero(pVfs->szOsFile);
  如果( pFile ){
    rc = sqlite3OsOpen(pVfs, zFile, pFile, flags, pOutFlags);
    如果( rc!=SQLITE_OK ){
      sqlite3_free(pFile);
      *ppFile = 0;
    }否则{
      *ppFile = pFile;
    }
  }否则{
    *ppFile = 0;
    rc = SQLITE_NOMEM_BKPT;
  }
  assert( *ppFile!=0 || rc!=SQLITE_OK );
  返回 rc;
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
整数 sqlite3OsInit(void){
  void *p = sqlite3_malloc(10);
  如果( p==0 ) 返回 SQLITE_NOMEM_BKPT;
  sqlite3_free(p);
  返回 sqlite3_os_init();
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
#如果 SQLITE_THREADSAFE
  sqlite3_mutex *mutex;
#endif
#ifndef SQLITE_OMIT_AUTOINIT
  整数 rc = sqlite3_initialize();
  如果( rc ) 返回 0;
#endif
#如果 SQLITE_THREADSAFE
  mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
#endif
  sqlite3_mutex_enter(mutex);
  为(pVfs = vfsList; pVfs; pVfs=pVfs->pNext){
    如果( zVfs==0 ) break;
    如果( strcmp(zVfs, pVfs->zName)==0 ) break;
  }
  sqlite3_mutex_leave(mutex);
  返回 pVfs;
}

/*
** Unlink a VFS from the linked list
*/
static void vfsUnlink(sqlite3_vfs *pVfs){
  assert( sqlite3_mutex_held(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN)) );
  如果( pVfs==0 ){
    /* No-op */
  }否则 如果( vfsList==pVfs ){
    vfsList = pVfs->pNext;
  }否则 如果( vfsList ){
    sqlite3_vfs *p = vfsList;
    当( p->pNext && p->pNext!=pVfs ){
      p = p->pNext;
    }
    如果( p->pNext==pVfs ){
      p->pNext = pVfs->pNext;
    }
  }
}

/*
** Register a VFS with the system.  It is harmless to register the same
** VFS multiple times.  The new VFS becomes the default if makeDflt is
** true.
*/
整数 sqlite3_vfs_register(sqlite3_vfs *pVfs, 整数 makeDflt){
  MUTEX_LOGIC(sqlite3_mutex *mutex;)
#ifndef SQLITE_OMIT_AUTOINIT
  整数 rc = sqlite3_initialize();
  如果( rc ) 返回 rc;
#endif
#ifdef SQLITE_ENABLE_API_ARMOR
  如果( pVfs==0 ) 返回 SQLITE_MISUSE_BKPT;
#endif

  MUTEX_LOGIC( mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  sqlite3_mutex_enter(mutex);
  vfsUnlink(pVfs);
  如果( makeDflt || vfsList==0 ){
    pVfs->pNext = vfsList;
    vfsList = pVfs;
  }否则{
    pVfs->pNext = vfsList->pNext;
    vfsList->pNext = pVfs;
  }
  assert(vfsList);
  sqlite3_mutex_leave(mutex);
  返回 SQLITE_OK;
}

/*
** Unregister a VFS so that it is no longer accessible.
*/
整数 sqlite3_vfs_unregister(sqlite3_vfs *pVfs){
  MUTEX_LOGIC(sqlite3_mutex *mutex;)
#ifndef SQLITE_OMIT_AUTOINIT
  整数 rc = sqlite3_initialize();
  如果( rc ) 返回 rc;
#endif
  MUTEX_LOGIC( mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  sqlite3_mutex_enter(mutex);
  vfsUnlink(pVfs);
  sqlite3_mutex_leave(mutex);
  返回 SQLITE_OK;
}
