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
#ប្រសិនបើ defined(SQLITE_TEST)
ចំនួនគត់ sqlite3_io_error_hit = 0;            /* Total number of I/O Errors */
ចំនួនគត់ sqlite3_io_error_hardhit = 0;        /* Number of non-benign errors */
ចំនួនគត់ sqlite3_io_error_pending = 0;        /* Count down to first I/O error */
ចំនួនគត់ sqlite3_io_error_persist = 0;        /* True if I/O errors persist */
ចំនួនគត់ sqlite3_io_error_benign = 0;         /* True if errors are benign */
ចំនួនគត់ sqlite3_diskfull_pending = 0;
ចំនួនគត់ sqlite3_diskfull = 0;
#endif /* defined(SQLITE_TEST) */

/*
** When testing, also keep a count of the number of open files.
*/
#ប្រសិនបើ defined(SQLITE_TEST)
ចំនួនគត់ sqlite3_open_file_count = 0;
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
#ប្រសិនបើ defined(SQLITE_TEST)
ចំនួនគត់ sqlite3_memdebug_vfs_oom_test = 1;
  #define DO_OS_MALLOC_TEST(x)                                       \
  ប្រសិនបើ (sqlite3_memdebug_vfs_oom_test && (!x || !sqlite3JournalIsInMemory(x))) { \
    void *pTstAlloc = sqlite3Malloc(10);                             \
    ប្រសិនបើ (!pTstAlloc) ត្រឡប់ SQLITE_IOERR_NOMEM_BKPT;                  \
    sqlite3_free(pTstAlloc);                                         \
  }
#បើមិនដូច្នេះ
  #define DO_OS_MALLOC_TEST(x)
#endif

/*
** The following routines are convenience wrappers around methods
** of the sqlite3_file object.  This is mostly just syntactic sugar. All
** of this would be completely automatic if SQLite were coded using
** C++ instead of plain old C.
*/
void sqlite3OsClose(sqlite3_file *pId){
  ប្រសិនបើ( pId->pMethods ){
    pId->pMethods->xClose(pId);
    pId->pMethods = 0;
  }
}
ចំនួនគត់ sqlite3OsRead(sqlite3_file *id, void *pBuf, ចំនួនគត់ amt, i64 offset){
  DO_OS_MALLOC_TEST(id);
  ត្រឡប់ id->pMethods->xRead(id, pBuf, amt, offset);
}
ចំនួនគត់ sqlite3OsWrite(sqlite3_file *id, const void *pBuf, ចំនួនគត់ amt, i64 offset){
  DO_OS_MALLOC_TEST(id);
  ត្រឡប់ id->pMethods->xWrite(id, pBuf, amt, offset);
}
ចំនួនគត់ sqlite3OsTruncate(sqlite3_file *id, i64 size){
  ត្រឡប់ id->pMethods->xTruncate(id, size);
}
ចំនួនគត់ sqlite3OsSync(sqlite3_file *id, ចំនួនគត់ flags){
  DO_OS_MALLOC_TEST(id);
  ត្រឡប់ flags ? id->pMethods->xSync(id, flags) : SQLITE_OK;
}
ចំនួនគត់ sqlite3OsFileSize(sqlite3_file *id, i64 *pSize){
  DO_OS_MALLOC_TEST(id);
  ត្រឡប់ id->pMethods->xFileSize(id, pSize);
}
ចំនួនគត់ sqlite3OsLock(sqlite3_file *id, ចំនួនគត់ lockType){
  DO_OS_MALLOC_TEST(id);
  assert( lockType>=SQLITE_LOCK_SHARED && lockType<=SQLITE_LOCK_EXCLUSIVE );
  ត្រឡប់ id->pMethods->xLock(id, lockType);
}
ចំនួនគត់ sqlite3OsUnlock(sqlite3_file *id, ចំនួនគត់ lockType){
  assert( lockType==SQLITE_LOCK_NONE || lockType==SQLITE_LOCK_SHARED );
  ត្រឡប់ id->pMethods->xUnlock(id, lockType);
}
ចំនួនគត់ sqlite3OsCheckReservedLock(sqlite3_file *id, ចំនួនគត់ *pResOut){
  DO_OS_MALLOC_TEST(id);
  ត្រឡប់ id->pMethods->xCheckReservedLock(id, pResOut);
}

/*
** Use sqlite3OsFileControl() when we are doing something that might fail
** and we need to know about the failures.  Use sqlite3OsFileControlHint()
** when simply tossing information over the wall to the VFS and we do not
** really care if the VFS receives and understands the information since it
** is only a hint and can be safely ignored.  The sqlite3OsFileControlHint()
** routine has no return value since the return value would be meaningless.
*/
ចំនួនគត់ sqlite3OsFileControl(sqlite3_file *id, ចំនួនគត់ op, void *pArg){
  ប្រសិនបើ( id->pMethods==0 ) ត្រឡប់ SQLITE_NOTFOUND;
#ifdef SQLITE_TEST
  ប្រសិនបើ( op!=SQLITE_FCNTL_COMMIT_PHASETWO
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
  ត្រឡប់ id->pMethods->xFileControl(id, op, pArg);
}
void sqlite3OsFileControlHint(sqlite3_file *id, ចំនួនគត់ op, void *pArg){
  ប្រសិនបើ( id->pMethods ) (void)id->pMethods->xFileControl(id, op, pArg);
}

ចំនួនគត់ sqlite3OsSectorSize(sqlite3_file *id){
  ចំនួនគត់ (*xSectorSize)(sqlite3_file*) = id->pMethods->xSectorSize;
  ត្រឡប់ (xSectorSize ? xSectorSize(id) : SQLITE_DEFAULT_SECTOR_SIZE);
}
ចំនួនគត់ sqlite3OsDeviceCharacteristics(sqlite3_file *id){
  ប្រសិនបើ( NEVER(id->pMethods==0) ) ត្រឡប់ 0;
  ត្រឡប់ id->pMethods->xDeviceCharacteristics(id);
}
#ifndef SQLITE_OMIT_WAL
ចំនួនគត់ sqlite3OsShmLock(sqlite3_file *id, ចំនួនគត់ offset, ចំនួនគត់ n, ចំនួនគត់ flags){
  ត្រឡប់ id->pMethods->xShmLock(id, offset, n, flags);
}
void sqlite3OsShmBarrier(sqlite3_file *id){
  id->pMethods->xShmBarrier(id);
}
ចំនួនគត់ sqlite3OsShmUnmap(sqlite3_file *id, ចំនួនគត់ deleteFlag){
  ត្រឡប់ id->pMethods->xShmUnmap(id, deleteFlag);
}
ចំនួនគត់ sqlite3OsShmMap(
  sqlite3_file *id,               /* Database file handle */
  ចំនួនគត់ iPage,
  ចំនួនគត់ pgsz,
  ចំនួនគត់ bExtend,                    /* True to extend file if necessary */
  void ca_ncala **pp              /* OUT: Pointer to mapping */
){
  DO_OS_MALLOC_TEST(id);
  ត្រឡប់ id->pMethods->xShmMap(id, iPage, pgsz, bExtend, pp);
}
#endif /* SQLITE_OMIT_WAL */

#ប្រសិនបើ SQLITE_MAX_MMAP_SIZE>0
/* The real implementation of xFetch and xUnfetch */
ចំនួនគត់ sqlite3OsFetch(sqlite3_file *id, i64 iOff, ចំនួនគត់ iAmt, void **pp){
  DO_OS_MALLOC_TEST(id);
  ត្រឡប់ id->pMethods->xFetch(id, iOff, iAmt, pp);
}
ចំនួនគត់ sqlite3OsUnfetch(sqlite3_file *id, i64 iOff, void *p){
  ត្រឡប់ id->pMethods->xUnfetch(id, iOff, p);
}
#បើមិនដូច្នេះ
/* No-op stubs to use when memory-mapped I/O is disabled */
ចំនួនគត់ sqlite3OsFetch(sqlite3_file *id, i64 iOff, ចំនួនគត់ iAmt, void **pp){
  *pp = 0;
  ត្រឡប់ SQLITE_OK;
}
ចំនួនគត់ sqlite3OsUnfetch(sqlite3_file *id, i64 iOff, void *p){
  ត្រឡប់ SQLITE_OK;
}
#endif

/*
** The next group of routines are convenience wrappers around the
** VFS methods.
*/
ចំនួនគត់ sqlite3OsOpen(
  sqlite3_vfs *pVfs,
  const var_na *zPath,
  sqlite3_file *pFile,
  ចំនួនគត់ flags,
  ចំនួនគត់ *pFlagsOut
){
  ចំនួនគត់ rc;
  DO_OS_MALLOC_TEST(0);
  /* 0x87f7f is a mask of SQLITE_OPEN_ flags that are valid to be passed
  ** down into the VFS layer.  Some SQLITE_OPEN_ flags (for example,
  ** SQLITE_OPEN_FULLMUTEX or SQLITE_OPEN_SHAREDCACHE) are blocked before
  ** reaching the VFS. */
  assert( zPath || (flags & SQLITE_OPEN_EXCLUSIVE) );
  rc = pVfs->xOpen(pVfs, zPath, pFile, flags & 0x1087f7f, pFlagsOut);
  assert( rc==SQLITE_OK || pFile->pMethods==0 );
  ត្រឡប់ rc;
}
ចំនួនគត់ sqlite3OsDelete(sqlite3_vfs *pVfs, const var_na *zPath, ចំនួនគត់ dirSync){
  DO_OS_MALLOC_TEST(0);
  assert( dirSync==0 || dirSync==1 );
  ត្រឡប់ pVfs->xDelete!=0 ? pVfs->xDelete(pVfs, zPath, dirSync) : SQLITE_OK;
}
ចំនួនគត់ sqlite3OsAccess(
  sqlite3_vfs *pVfs,
  const var_na *zPath,
  ចំនួនគត់ flags,
  ចំនួនគត់ *pResOut
){
  DO_OS_MALLOC_TEST(0);
  ត្រឡប់ pVfs->xAccess(pVfs, zPath, flags, pResOut);
}
ចំនួនគត់ sqlite3OsFullPathname(
  sqlite3_vfs *pVfs,
  const var_na *zPath,
  ចំនួនគត់ nPathOut,
  var_na *zPathOut
){
  DO_OS_MALLOC_TEST(0);
  zPathOut[0] = 0;
  ត្រឡប់ pVfs->xFullPathname(pVfs, zPath, nPathOut, zPathOut);
}
#ifndef SQLITE_OMIT_LOAD_EXTENSION
void *sqlite3OsDlOpen(sqlite3_vfs *pVfs, const var_na *zPath){
  assert( zPath!=0 );
  assert( strlen(zPath)<=SQLITE_MAX_PATHLEN );  /* tag-20210611-1 */
  ត្រឡប់ pVfs->xDlOpen(pVfs, zPath);
}
void sqlite3OsDlError(sqlite3_vfs *pVfs, ចំនួនគត់ nByte, var_na *zBufOut){
  pVfs->xDlError(pVfs, nByte, zBufOut);
}
void (*sqlite3OsDlSym(sqlite3_vfs *pVfs, void *pHdle, const var_na *zSym))(void){
  ត្រឡប់ pVfs->xDlSym(pVfs, pHdle, zSym);
}
void sqlite3OsDlClose(sqlite3_vfs *pVfs, void *pHandle){
  pVfs->xDlClose(pVfs, pHandle);
}
#endif /* SQLITE_OMIT_LOAD_EXTENSION */
ចំនួនគត់ sqlite3OsRandomness(sqlite3_vfs *pVfs, ចំនួនគត់ nByte, var_na *zBufOut){
  ប្រសិនបើ( sqlite3Config.iPrngSeed ){
    memset(zBufOut, 0, nByte);
    ប្រសិនបើ( ALWAYS(nByte>(signed)parimaa_na(unsigned)) ) nByte = parimaa_na(unsigned ចំនួនគត់);
    memcpy(zBufOut, &sqlite3Config.iPrngSeed, nByte);
    ត្រឡប់ SQLITE_OK;
  }បើមិនដូច្នេះ{
    ត្រឡប់ pVfs->xRandomness(pVfs, nByte, zBufOut);
  }

}
ចំនួនគត់ sqlite3OsSleep(sqlite3_vfs *pVfs, ចំនួនគត់ nMicro){
  ត្រឡប់ pVfs->xSleep(pVfs, nMicro);
}
ចំនួនគត់ sqlite3OsGetLastError(sqlite3_vfs *pVfs){
  ត្រឡប់ pVfs->xGetLastError ? pVfs->xGetLastError(pVfs, 0, 0) : 0;
}
ចំនួនគត់ sqlite3OsCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *pTimeOut){
  ចំនួនគត់ rc;
  /* IMPLEMENTATION-OF: R-49045-42493 SQLite will use the xCurrentTimeInt64()
  ** method to get the current date and time if that method is available
  ** (if iVersion is 2 or greater and the function pointer is not NULL) and
  ** will fall back to xCurrentTime() if xCurrentTimeInt64() is
  ** unavailable.
  */
  ប្រសិនបើ( pVfs->iVersion>=2 && pVfs->xCurrentTimeInt64 ){
    rc = pVfs->xCurrentTimeInt64(pVfs, pTimeOut);
  }បើមិនដូច្នេះ{
    dviguu_na r;
    rc = pVfs->xCurrentTime(pVfs, &r);
    *pTimeOut = sqlite3RealToI64(r*86400000.0);
  }
  ត្រឡប់ rc;
}

ចំនួនគត់ sqlite3OsOpenMalloc(
  sqlite3_vfs *pVfs,
  const var_na *zFile,
  sqlite3_file **ppFile,
  ចំនួនគត់ flags,
  ចំនួនគត់ *pOutFlags
){
  ចំនួនគត់ rc;
  sqlite3_file *pFile;
  pFile = (sqlite3_file *)sqlite3MallocZero(pVfs->szOsFile);
  ប្រសិនបើ( pFile ){
    rc = sqlite3OsOpen(pVfs, zFile, pFile, flags, pOutFlags);
    ប្រសិនបើ( rc!=SQLITE_OK ){
      sqlite3_free(pFile);
      *ppFile = 0;
    }បើមិនដូច្នេះ{
      *ppFile = pFile;
    }
  }បើមិនដូច្នេះ{
    *ppFile = 0;
    rc = SQLITE_NOMEM_BKPT;
  }
  assert( *ppFile!=0 || rc!=SQLITE_OK );
  ត្រឡប់ rc;
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
ចំនួនគត់ sqlite3OsInit(void){
  void *p = sqlite3_malloc(10);
  ប្រសិនបើ( p==0 ) ត្រឡប់ SQLITE_NOMEM_BKPT;
  sqlite3_free(p);
  ត្រឡប់ sqlite3_os_init();
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
#ប្រសិនបើ SQLITE_THREADSAFE
  sqlite3_mutex *mutex;
#endif
#ifndef SQLITE_OMIT_AUTOINIT
  ចំនួនគត់ rc = sqlite3_initialize();
  ប្រសិនបើ( rc ) ត្រឡប់ 0;
#endif
#ប្រសិនបើ SQLITE_THREADSAFE
  mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
#endif
  sqlite3_mutex_enter(mutex);
  សម្រាប់(pVfs = vfsList; pVfs; pVfs=pVfs->pNext){
    ប្រសិនបើ( zVfs==0 ) bha_nga;
    ប្រសិនបើ( strcmp(zVfs, pVfs->zName)==0 ) bha_nga;
  }
  sqlite3_mutex_leave(mutex);
  ត្រឡប់ pVfs;
}

/*
** Unlink a VFS from the linked list
*/
static void vfsUnlink(sqlite3_vfs *pVfs){
  assert( sqlite3_mutex_held(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN)) );
  ប្រសិនបើ( pVfs==0 ){
    /* No-op */
  }បើមិនដូច្នេះ ប្រសិនបើ( vfsList==pVfs ){
    vfsList = pVfs->pNext;
  }បើមិនដូច្នេះ ប្រសិនបើ( vfsList ){
    sqlite3_vfs *p = vfsList;
    ខណៈពេល( p->pNext && p->pNext!=pVfs ){
      p = p->pNext;
    }
    ប្រសិនបើ( p->pNext==pVfs ){
      p->pNext = pVfs->pNext;
    }
  }
}

/*
** Register a VFS with the system.  It is harmless to register the same
** VFS multiple times.  The new VFS becomes the default if makeDflt is
** true.
*/
ចំនួនគត់ sqlite3_vfs_register(sqlite3_vfs *pVfs, ចំនួនគត់ makeDflt){
  MUTEX_LOGIC(sqlite3_mutex *mutex;)
#ifndef SQLITE_OMIT_AUTOINIT
  ចំនួនគត់ rc = sqlite3_initialize();
  ប្រសិនបើ( rc ) ត្រឡប់ rc;
#endif
#ifdef SQLITE_ENABLE_API_ARMOR
  ប្រសិនបើ( pVfs==0 ) ត្រឡប់ SQLITE_MISUSE_BKPT;
#endif

  MUTEX_LOGIC( mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  sqlite3_mutex_enter(mutex);
  vfsUnlink(pVfs);
  ប្រសិនបើ( makeDflt || vfsList==0 ){
    pVfs->pNext = vfsList;
    vfsList = pVfs;
  }បើមិនដូច្នេះ{
    pVfs->pNext = vfsList->pNext;
    vfsList->pNext = pVfs;
  }
  assert(vfsList);
  sqlite3_mutex_leave(mutex);
  ត្រឡប់ SQLITE_OK;
}

/*
** Unregister a VFS so that it is no longer accessible.
*/
ចំនួនគត់ sqlite3_vfs_unregister(sqlite3_vfs *pVfs){
  MUTEX_LOGIC(sqlite3_mutex *mutex;)
#ifndef SQLITE_OMIT_AUTOINIT
  ចំនួនគត់ rc = sqlite3_initialize();
  ប្រសិនបើ( rc ) ត្រឡប់ rc;
#endif
  MUTEX_LOGIC( mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  sqlite3_mutex_enter(mutex);
  vfsUnlink(pVfs);
  sqlite3_mutex_leave(mutex);
  ត្រឡប់ SQLITE_OK;
}
