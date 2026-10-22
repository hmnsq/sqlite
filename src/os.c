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
#nếu defined(SQLITE_TEST)
số_nguyên sqlite3_io_error_hit = 0;            /* Total number of I/O Errors */
số_nguyên sqlite3_io_error_hardhit = 0;        /* Number of non-benign errors */
số_nguyên sqlite3_io_error_pending = 0;        /* Count down to first I/O error */
số_nguyên sqlite3_io_error_persist = 0;        /* True if I/O errors persist */
số_nguyên sqlite3_io_error_benign = 0;         /* True if errors are benign */
số_nguyên sqlite3_diskfull_pending = 0;
số_nguyên sqlite3_diskfull = 0;
#endif /* defined(SQLITE_TEST) */

/*
** When testing, also keep a count of the number of open files.
*/
#nếu defined(SQLITE_TEST)
số_nguyên sqlite3_open_file_count = 0;
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
#nếu defined(SQLITE_TEST)
số_nguyên sqlite3_memdebug_vfs_oom_test = 1;
  #define DO_OS_MALLOC_TEST(x)                                       \
  nếu (sqlite3_memdebug_vfs_oom_test && (!x || !sqlite3JournalIsInMemory(x))) { \
    void *pTstAlloc = sqlite3Malloc(10);                             \
    nếu (!pTstAlloc) trả_về SQLITE_IOERR_NOMEM_BKPT;                  \
    sqlite3_free(pTstAlloc);                                         \
  }
#không_thì
  #define DO_OS_MALLOC_TEST(x)
#endif

/*
** The following routines are convenience wrappers around methods
** of the sqlite3_file object.  This is mostly just syntactic sugar. All
** of this would be completely automatic if SQLite were coded using
** C++ instead of plain old C.
*/
void sqlite3OsClose(sqlite3_file *pId){
  nếu( pId->pMethods ){
    pId->pMethods->xClose(pId);
    pId->pMethods = 0;
  }
}
số_nguyên sqlite3OsRead(sqlite3_file *id, void *pBuf, số_nguyên amt, i64 offset){
  DO_OS_MALLOC_TEST(id);
  trả_về id->pMethods->xRead(id, pBuf, amt, offset);
}
số_nguyên sqlite3OsWrite(sqlite3_file *id, const void *pBuf, số_nguyên amt, i64 offset){
  DO_OS_MALLOC_TEST(id);
  trả_về id->pMethods->xWrite(id, pBuf, amt, offset);
}
số_nguyên sqlite3OsTruncate(sqlite3_file *id, i64 size){
  trả_về id->pMethods->xTruncate(id, size);
}
số_nguyên sqlite3OsSync(sqlite3_file *id, số_nguyên flags){
  DO_OS_MALLOC_TEST(id);
  trả_về flags ? id->pMethods->xSync(id, flags) : SQLITE_OK;
}
số_nguyên sqlite3OsFileSize(sqlite3_file *id, i64 *pSize){
  DO_OS_MALLOC_TEST(id);
  trả_về id->pMethods->xFileSize(id, pSize);
}
số_nguyên sqlite3OsLock(sqlite3_file *id, số_nguyên lockType){
  DO_OS_MALLOC_TEST(id);
  assert( lockType>=SQLITE_LOCK_SHARED && lockType<=SQLITE_LOCK_EXCLUSIVE );
  trả_về id->pMethods->xLock(id, lockType);
}
số_nguyên sqlite3OsUnlock(sqlite3_file *id, số_nguyên lockType){
  assert( lockType==SQLITE_LOCK_NONE || lockType==SQLITE_LOCK_SHARED );
  trả_về id->pMethods->xUnlock(id, lockType);
}
số_nguyên sqlite3OsCheckReservedLock(sqlite3_file *id, số_nguyên *pResOut){
  DO_OS_MALLOC_TEST(id);
  trả_về id->pMethods->xCheckReservedLock(id, pResOut);
}

/*
** Use sqlite3OsFileControl() when we are doing something that might fail
** and we need to know about the failures.  Use sqlite3OsFileControlHint()
** when simply tossing information over the wall to the VFS and we do not
** really care if the VFS receives and understands the information since it
** is only a hint and can be safely ignored.  The sqlite3OsFileControlHint()
** routine has no return value since the return value would be meaningless.
*/
số_nguyên sqlite3OsFileControl(sqlite3_file *id, số_nguyên op, void *pArg){
  nếu( id->pMethods==0 ) trả_về SQLITE_NOTFOUND;
#ifdef SQLITE_TEST
  nếu( op!=SQLITE_FCNTL_COMMIT_PHASETWO
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
  trả_về id->pMethods->xFileControl(id, op, pArg);
}
void sqlite3OsFileControlHint(sqlite3_file *id, số_nguyên op, void *pArg){
  nếu( id->pMethods ) (void)id->pMethods->xFileControl(id, op, pArg);
}

số_nguyên sqlite3OsSectorSize(sqlite3_file *id){
  số_nguyên (*xSectorSize)(sqlite3_file*) = id->pMethods->xSectorSize;
  trả_về (xSectorSize ? xSectorSize(id) : SQLITE_DEFAULT_SECTOR_SIZE);
}
số_nguyên sqlite3OsDeviceCharacteristics(sqlite3_file *id){
  nếu( NEVER(id->pMethods==0) ) trả_về 0;
  trả_về id->pMethods->xDeviceCharacteristics(id);
}
#ifndef SQLITE_OMIT_WAL
số_nguyên sqlite3OsShmLock(sqlite3_file *id, số_nguyên offset, số_nguyên n, số_nguyên flags){
  trả_về id->pMethods->xShmLock(id, offset, n, flags);
}
void sqlite3OsShmBarrier(sqlite3_file *id){
  id->pMethods->xShmBarrier(id);
}
số_nguyên sqlite3OsShmUnmap(sqlite3_file *id, số_nguyên deleteFlag){
  trả_về id->pMethods->xShmUnmap(id, deleteFlag);
}
số_nguyên sqlite3OsShmMap(
  sqlite3_file *id,               /* Database file handle */
  số_nguyên iPage,
  số_nguyên pgsz,
  số_nguyên bExtend,                    /* True to extend file if necessary */
  void volatile **pp              /* OUT: Pointer to mapping */
){
  DO_OS_MALLOC_TEST(id);
  trả_về id->pMethods->xShmMap(id, iPage, pgsz, bExtend, pp);
}
#endif /* SQLITE_OMIT_WAL */

#nếu SQLITE_MAX_MMAP_SIZE>0
/* The real implementation of xFetch and xUnfetch */
số_nguyên sqlite3OsFetch(sqlite3_file *id, i64 iOff, số_nguyên iAmt, void **pp){
  DO_OS_MALLOC_TEST(id);
  trả_về id->pMethods->xFetch(id, iOff, iAmt, pp);
}
số_nguyên sqlite3OsUnfetch(sqlite3_file *id, i64 iOff, void *p){
  trả_về id->pMethods->xUnfetch(id, iOff, p);
}
#không_thì
/* No-op stubs to use when memory-mapped I/O is disabled */
số_nguyên sqlite3OsFetch(sqlite3_file *id, i64 iOff, số_nguyên iAmt, void **pp){
  *pp = 0;
  trả_về SQLITE_OK;
}
số_nguyên sqlite3OsUnfetch(sqlite3_file *id, i64 iOff, void *p){
  trả_về SQLITE_OK;
}
#endif

/*
** The next group of routines are convenience wrappers around the
** VFS methods.
*/
số_nguyên sqlite3OsOpen(
  sqlite3_vfs *pVfs,
  const char *zPath,
  sqlite3_file *pFile,
  số_nguyên flags,
  số_nguyên *pFlagsOut
){
  số_nguyên rc;
  DO_OS_MALLOC_TEST(0);
  /* 0x87f7f is a mask of SQLITE_OPEN_ flags that are valid to be passed
  ** down into the VFS layer.  Some SQLITE_OPEN_ flags (for example,
  ** SQLITE_OPEN_FULLMUTEX or SQLITE_OPEN_SHAREDCACHE) are blocked before
  ** reaching the VFS. */
  assert( zPath || (flags & SQLITE_OPEN_EXCLUSIVE) );
  rc = pVfs->xOpen(pVfs, zPath, pFile, flags & 0x1087f7f, pFlagsOut);
  assert( rc==SQLITE_OK || pFile->pMethods==0 );
  trả_về rc;
}
số_nguyên sqlite3OsDelete(sqlite3_vfs *pVfs, const char *zPath, số_nguyên dirSync){
  DO_OS_MALLOC_TEST(0);
  assert( dirSync==0 || dirSync==1 );
  trả_về pVfs->xDelete!=0 ? pVfs->xDelete(pVfs, zPath, dirSync) : SQLITE_OK;
}
số_nguyên sqlite3OsAccess(
  sqlite3_vfs *pVfs,
  const char *zPath,
  số_nguyên flags,
  số_nguyên *pResOut
){
  DO_OS_MALLOC_TEST(0);
  trả_về pVfs->xAccess(pVfs, zPath, flags, pResOut);
}
số_nguyên sqlite3OsFullPathname(
  sqlite3_vfs *pVfs,
  const char *zPath,
  số_nguyên nPathOut,
  char *zPathOut
){
  DO_OS_MALLOC_TEST(0);
  zPathOut[0] = 0;
  trả_về pVfs->xFullPathname(pVfs, zPath, nPathOut, zPathOut);
}
#ifndef SQLITE_OMIT_LOAD_EXTENSION
void *sqlite3OsDlOpen(sqlite3_vfs *pVfs, const char *zPath){
  assert( zPath!=0 );
  assert( strlen(zPath)<=SQLITE_MAX_PATHLEN );  /* tag-20210611-1 */
  trả_về pVfs->xDlOpen(pVfs, zPath);
}
void sqlite3OsDlError(sqlite3_vfs *pVfs, số_nguyên nByte, char *zBufOut){
  pVfs->xDlError(pVfs, nByte, zBufOut);
}
void (*sqlite3OsDlSym(sqlite3_vfs *pVfs, void *pHdle, const char *zSym))(void){
  trả_về pVfs->xDlSym(pVfs, pHdle, zSym);
}
void sqlite3OsDlClose(sqlite3_vfs *pVfs, void *pHandle){
  pVfs->xDlClose(pVfs, pHandle);
}
#endif /* SQLITE_OMIT_LOAD_EXTENSION */
số_nguyên sqlite3OsRandomness(sqlite3_vfs *pVfs, số_nguyên nByte, char *zBufOut){
  nếu( sqlite3Config.iPrngSeed ){
    memset(zBufOut, 0, nByte);
    nếu( ALWAYS(nByte>(signed)sizeof(unsigned)) ) nByte = sizeof(unsigned số_nguyên);
    memcpy(zBufOut, &sqlite3Config.iPrngSeed, nByte);
    trả_về SQLITE_OK;
  }không_thì{
    trả_về pVfs->xRandomness(pVfs, nByte, zBufOut);
  }

}
số_nguyên sqlite3OsSleep(sqlite3_vfs *pVfs, số_nguyên nMicro){
  trả_về pVfs->xSleep(pVfs, nMicro);
}
số_nguyên sqlite3OsGetLastError(sqlite3_vfs *pVfs){
  trả_về pVfs->xGetLastError ? pVfs->xGetLastError(pVfs, 0, 0) : 0;
}
số_nguyên sqlite3OsCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *pTimeOut){
  số_nguyên rc;
  /* IMPLEMENTATION-OF: R-49045-42493 SQLite will use the xCurrentTimeInt64()
  ** method to get the current date and time if that method is available
  ** (if iVersion is 2 or greater and the function pointer is not NULL) and
  ** will fall back to xCurrentTime() if xCurrentTimeInt64() is
  ** unavailable.
  */
  nếu( pVfs->iVersion>=2 && pVfs->xCurrentTimeInt64 ){
    rc = pVfs->xCurrentTimeInt64(pVfs, pTimeOut);
  }không_thì{
    double r;
    rc = pVfs->xCurrentTime(pVfs, &r);
    *pTimeOut = sqlite3RealToI64(r*86400000.0);
  }
  trả_về rc;
}

số_nguyên sqlite3OsOpenMalloc(
  sqlite3_vfs *pVfs,
  const char *zFile,
  sqlite3_file **ppFile,
  số_nguyên flags,
  số_nguyên *pOutFlags
){
  số_nguyên rc;
  sqlite3_file *pFile;
  pFile = (sqlite3_file *)sqlite3MallocZero(pVfs->szOsFile);
  nếu( pFile ){
    rc = sqlite3OsOpen(pVfs, zFile, pFile, flags, pOutFlags);
    nếu( rc!=SQLITE_OK ){
      sqlite3_free(pFile);
      *ppFile = 0;
    }không_thì{
      *ppFile = pFile;
    }
  }không_thì{
    *ppFile = 0;
    rc = SQLITE_NOMEM_BKPT;
  }
  assert( *ppFile!=0 || rc!=SQLITE_OK );
  trả_về rc;
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
số_nguyên sqlite3OsInit(void){
  void *p = sqlite3_malloc(10);
  nếu( p==0 ) trả_về SQLITE_NOMEM_BKPT;
  sqlite3_free(p);
  trả_về sqlite3_os_init();
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
#nếu SQLITE_THREADSAFE
  sqlite3_mutex *mutex;
#endif
#ifndef SQLITE_OMIT_AUTOINIT
  số_nguyên rc = sqlite3_initialize();
  nếu( rc ) trả_về 0;
#endif
#nếu SQLITE_THREADSAFE
  mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
#endif
  sqlite3_mutex_enter(mutex);
  cho(pVfs = vfsList; pVfs; pVfs=pVfs->pNext){
    nếu( zVfs==0 ) break;
    nếu( strcmp(zVfs, pVfs->zName)==0 ) break;
  }
  sqlite3_mutex_leave(mutex);
  trả_về pVfs;
}

/*
** Unlink a VFS from the linked list
*/
static void vfsUnlink(sqlite3_vfs *pVfs){
  assert( sqlite3_mutex_held(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN)) );
  nếu( pVfs==0 ){
    /* No-op */
  }không_thì nếu( vfsList==pVfs ){
    vfsList = pVfs->pNext;
  }không_thì nếu( vfsList ){
    sqlite3_vfs *p = vfsList;
    trong_khi( p->pNext && p->pNext!=pVfs ){
      p = p->pNext;
    }
    nếu( p->pNext==pVfs ){
      p->pNext = pVfs->pNext;
    }
  }
}

/*
** Register a VFS with the system.  It is harmless to register the same
** VFS multiple times.  The new VFS becomes the default if makeDflt is
** true.
*/
số_nguyên sqlite3_vfs_register(sqlite3_vfs *pVfs, số_nguyên makeDflt){
  MUTEX_LOGIC(sqlite3_mutex *mutex;)
#ifndef SQLITE_OMIT_AUTOINIT
  số_nguyên rc = sqlite3_initialize();
  nếu( rc ) trả_về rc;
#endif
#ifdef SQLITE_ENABLE_API_ARMOR
  nếu( pVfs==0 ) trả_về SQLITE_MISUSE_BKPT;
#endif

  MUTEX_LOGIC( mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  sqlite3_mutex_enter(mutex);
  vfsUnlink(pVfs);
  nếu( makeDflt || vfsList==0 ){
    pVfs->pNext = vfsList;
    vfsList = pVfs;
  }không_thì{
    pVfs->pNext = vfsList->pNext;
    vfsList->pNext = pVfs;
  }
  assert(vfsList);
  sqlite3_mutex_leave(mutex);
  trả_về SQLITE_OK;
}

/*
** Unregister a VFS so that it is no longer accessible.
*/
số_nguyên sqlite3_vfs_unregister(sqlite3_vfs *pVfs){
  MUTEX_LOGIC(sqlite3_mutex *mutex;)
#ifndef SQLITE_OMIT_AUTOINIT
  số_nguyên rc = sqlite3_initialize();
  nếu( rc ) trả_về rc;
#endif
  MUTEX_LOGIC( mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  sqlite3_mutex_enter(mutex);
  vfsUnlink(pVfs);
  sqlite3_mutex_leave(mutex);
  trả_về SQLITE_OK;
}
