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
#အကယ်၍ defined(SQLITE_TEST)
ကိန်းပြည့် sqlite3_io_error_hit = 0;            /* Total number of I/O Errors */
ကိန်းပြည့် sqlite3_io_error_hardhit = 0;        /* Number of non-benign errors */
ကိန်းပြည့် sqlite3_io_error_pending = 0;        /* Count down to first I/O error */
ကိန်းပြည့် sqlite3_io_error_persist = 0;        /* True if I/O errors persist */
ကိန်းပြည့် sqlite3_io_error_benign = 0;         /* True if errors are benign */
ကိန်းပြည့် sqlite3_diskfull_pending = 0;
ကိန်းပြည့် sqlite3_diskfull = 0;
#endif /* defined(SQLITE_TEST) */

/*
** When testing, also keep a count of the number of open files.
*/
#အကယ်၍ defined(SQLITE_TEST)
ကိန်းပြည့် sqlite3_open_file_count = 0;
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
#အကယ်၍ defined(SQLITE_TEST)
ကိန်းပြည့် sqlite3_memdebug_vfs_oom_test = 1;
  #define DO_OS_MALLOC_TEST(x)                                       \
  အကယ်၍ (sqlite3_memdebug_vfs_oom_test && (!x || !sqlite3JournalIsInMemory(x))) { \
    void *pTstAlloc = sqlite3Malloc(10);                             \
    အကယ်၍ (!pTstAlloc) ပြန်ပေး SQLITE_IOERR_NOMEM_BKPT;                  \
    sqlite3_free(pTstAlloc);                                         \
  }
#မဟုတ်ပါက
  #define DO_OS_MALLOC_TEST(x)
#endif

/*
** The following routines are convenience wrappers around methods
** of the sqlite3_file object.  This is mostly just syntactic sugar. All
** of this would be completely automatic if SQLite were coded using
** C++ instead of plain old C.
*/
void sqlite3OsClose(sqlite3_file *pId){
  အကယ်၍( pId->pMethods ){
    pId->pMethods->xClose(pId);
    pId->pMethods = 0;
  }
}
ကိန်းပြည့် sqlite3OsRead(sqlite3_file *id, void *pBuf, ကိန်းပြည့် amt, i64 offset){
  DO_OS_MALLOC_TEST(id);
  ပြန်ပေး id->pMethods->xRead(id, pBuf, amt, offset);
}
ကိန်းပြည့် sqlite3OsWrite(sqlite3_file *id, const void *pBuf, ကိန်းပြည့် amt, i64 offset){
  DO_OS_MALLOC_TEST(id);
  ပြန်ပေး id->pMethods->xWrite(id, pBuf, amt, offset);
}
ကိန်းပြည့် sqlite3OsTruncate(sqlite3_file *id, i64 size){
  ပြန်ပေး id->pMethods->xTruncate(id, size);
}
ကိန်းပြည့် sqlite3OsSync(sqlite3_file *id, ကိန်းပြည့် flags){
  DO_OS_MALLOC_TEST(id);
  ပြန်ပေး flags ? id->pMethods->xSync(id, flags) : SQLITE_OK;
}
ကိန်းပြည့် sqlite3OsFileSize(sqlite3_file *id, i64 *pSize){
  DO_OS_MALLOC_TEST(id);
  ပြန်ပေး id->pMethods->xFileSize(id, pSize);
}
ကိန်းပြည့် sqlite3OsLock(sqlite3_file *id, ကိန်းပြည့် lockType){
  DO_OS_MALLOC_TEST(id);
  assert( lockType>=SQLITE_LOCK_SHARED && lockType<=SQLITE_LOCK_EXCLUSIVE );
  ပြန်ပေး id->pMethods->xLock(id, lockType);
}
ကိန်းပြည့် sqlite3OsUnlock(sqlite3_file *id, ကိန်းပြည့် lockType){
  assert( lockType==SQLITE_LOCK_NONE || lockType==SQLITE_LOCK_SHARED );
  ပြန်ပေး id->pMethods->xUnlock(id, lockType);
}
ကိန်းပြည့် sqlite3OsCheckReservedLock(sqlite3_file *id, ကိန်းပြည့် *pResOut){
  DO_OS_MALLOC_TEST(id);
  ပြန်ပေး id->pMethods->xCheckReservedLock(id, pResOut);
}

/*
** Use sqlite3OsFileControl() when we are doing something that might fail
** and we need to know about the failures.  Use sqlite3OsFileControlHint()
** when simply tossing information over the wall to the VFS and we do not
** really care if the VFS receives and understands the information since it
** is only a hint and can be safely ignored.  The sqlite3OsFileControlHint()
** routine has no return value since the return value would be meaningless.
*/
ကိန်းပြည့် sqlite3OsFileControl(sqlite3_file *id, ကိန်းပြည့် op, void *pArg){
  အကယ်၍( id->pMethods==0 ) ပြန်ပေး SQLITE_NOTFOUND;
#ifdef SQLITE_TEST
  အကယ်၍( op!=SQLITE_FCNTL_COMMIT_PHASETWO
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
  ပြန်ပေး id->pMethods->xFileControl(id, op, pArg);
}
void sqlite3OsFileControlHint(sqlite3_file *id, ကိန်းပြည့် op, void *pArg){
  အကယ်၍( id->pMethods ) (void)id->pMethods->xFileControl(id, op, pArg);
}

ကိန်းပြည့် sqlite3OsSectorSize(sqlite3_file *id){
  ကိန်းပြည့် (*xSectorSize)(sqlite3_file*) = id->pMethods->xSectorSize;
  ပြန်ပေး (xSectorSize ? xSectorSize(id) : SQLITE_DEFAULT_SECTOR_SIZE);
}
ကိန်းပြည့် sqlite3OsDeviceCharacteristics(sqlite3_file *id){
  အကယ်၍( NEVER(id->pMethods==0) ) ပြန်ပေး 0;
  ပြန်ပေး id->pMethods->xDeviceCharacteristics(id);
}
#ifndef SQLITE_OMIT_WAL
ကိန်းပြည့် sqlite3OsShmLock(sqlite3_file *id, ကိန်းပြည့် offset, ကိန်းပြည့် n, ကိန်းပြည့် flags){
  ပြန်ပေး id->pMethods->xShmLock(id, offset, n, flags);
}
void sqlite3OsShmBarrier(sqlite3_file *id){
  id->pMethods->xShmBarrier(id);
}
ကိန်းပြည့် sqlite3OsShmUnmap(sqlite3_file *id, ကိန်းပြည့် deleteFlag){
  ပြန်ပေး id->pMethods->xShmUnmap(id, deleteFlag);
}
ကိန်းပြည့် sqlite3OsShmMap(
  sqlite3_file *id,               /* Database file handle */
  ကိန်းပြည့် iPage,
  ကိန်းပြည့် pgsz,
  ကိန်းပြည့် bExtend,                    /* True to extend file if necessary */
  void ca_ncala **pp              /* OUT: Pointer to mapping */
){
  DO_OS_MALLOC_TEST(id);
  ပြန်ပေး id->pMethods->xShmMap(id, iPage, pgsz, bExtend, pp);
}
#endif /* SQLITE_OMIT_WAL */

#အကယ်၍ SQLITE_MAX_MMAP_SIZE>0
/* The real implementation of xFetch and xUnfetch */
ကိန်းပြည့် sqlite3OsFetch(sqlite3_file *id, i64 iOff, ကိန်းပြည့် iAmt, void **pp){
  DO_OS_MALLOC_TEST(id);
  ပြန်ပေး id->pMethods->xFetch(id, iOff, iAmt, pp);
}
ကိန်းပြည့် sqlite3OsUnfetch(sqlite3_file *id, i64 iOff, void *p){
  ပြန်ပေး id->pMethods->xUnfetch(id, iOff, p);
}
#မဟုတ်ပါက
/* No-op stubs to use when memory-mapped I/O is disabled */
ကိန်းပြည့် sqlite3OsFetch(sqlite3_file *id, i64 iOff, ကိန်းပြည့် iAmt, void **pp){
  *pp = 0;
  ပြန်ပေး SQLITE_OK;
}
ကိန်းပြည့် sqlite3OsUnfetch(sqlite3_file *id, i64 iOff, void *p){
  ပြန်ပေး SQLITE_OK;
}
#endif

/*
** The next group of routines are convenience wrappers around the
** VFS methods.
*/
ကိန်းပြည့် sqlite3OsOpen(
  sqlite3_vfs *pVfs,
  const var_na *zPath,
  sqlite3_file *pFile,
  ကိန်းပြည့် flags,
  ကိန်းပြည့် *pFlagsOut
){
  ကိန်းပြည့် rc;
  DO_OS_MALLOC_TEST(0);
  /* 0x87f7f is a mask of SQLITE_OPEN_ flags that are valid to be passed
  ** down into the VFS layer.  Some SQLITE_OPEN_ flags (for example,
  ** SQLITE_OPEN_FULLMUTEX or SQLITE_OPEN_SHAREDCACHE) are blocked before
  ** reaching the VFS. */
  assert( zPath || (flags & SQLITE_OPEN_EXCLUSIVE) );
  rc = pVfs->xOpen(pVfs, zPath, pFile, flags & 0x1087f7f, pFlagsOut);
  assert( rc==SQLITE_OK || pFile->pMethods==0 );
  ပြန်ပေး rc;
}
ကိန်းပြည့် sqlite3OsDelete(sqlite3_vfs *pVfs, const var_na *zPath, ကိန်းပြည့် dirSync){
  DO_OS_MALLOC_TEST(0);
  assert( dirSync==0 || dirSync==1 );
  ပြန်ပေး pVfs->xDelete!=0 ? pVfs->xDelete(pVfs, zPath, dirSync) : SQLITE_OK;
}
ကိန်းပြည့် sqlite3OsAccess(
  sqlite3_vfs *pVfs,
  const var_na *zPath,
  ကိန်းပြည့် flags,
  ကိန်းပြည့် *pResOut
){
  DO_OS_MALLOC_TEST(0);
  ပြန်ပေး pVfs->xAccess(pVfs, zPath, flags, pResOut);
}
ကိန်းပြည့် sqlite3OsFullPathname(
  sqlite3_vfs *pVfs,
  const var_na *zPath,
  ကိန်းပြည့် nPathOut,
  var_na *zPathOut
){
  DO_OS_MALLOC_TEST(0);
  zPathOut[0] = 0;
  ပြန်ပေး pVfs->xFullPathname(pVfs, zPath, nPathOut, zPathOut);
}
#ifndef SQLITE_OMIT_LOAD_EXTENSION
void *sqlite3OsDlOpen(sqlite3_vfs *pVfs, const var_na *zPath){
  assert( zPath!=0 );
  assert( strlen(zPath)<=SQLITE_MAX_PATHLEN );  /* tag-20210611-1 */
  ပြန်ပေး pVfs->xDlOpen(pVfs, zPath);
}
void sqlite3OsDlError(sqlite3_vfs *pVfs, ကိန်းပြည့် nByte, var_na *zBufOut){
  pVfs->xDlError(pVfs, nByte, zBufOut);
}
void (*sqlite3OsDlSym(sqlite3_vfs *pVfs, void *pHdle, const var_na *zSym))(void){
  ပြန်ပေး pVfs->xDlSym(pVfs, pHdle, zSym);
}
void sqlite3OsDlClose(sqlite3_vfs *pVfs, void *pHandle){
  pVfs->xDlClose(pVfs, pHandle);
}
#endif /* SQLITE_OMIT_LOAD_EXTENSION */
ကိန်းပြည့် sqlite3OsRandomness(sqlite3_vfs *pVfs, ကိန်းပြည့် nByte, var_na *zBufOut){
  အကယ်၍( sqlite3Config.iPrngSeed ){
    memset(zBufOut, 0, nByte);
    အကယ်၍( ALWAYS(nByte>(signed)parimaa_na(unsigned)) ) nByte = parimaa_na(unsigned ကိန်းပြည့်);
    memcpy(zBufOut, &sqlite3Config.iPrngSeed, nByte);
    ပြန်ပေး SQLITE_OK;
  }မဟုတ်ပါက{
    ပြန်ပေး pVfs->xRandomness(pVfs, nByte, zBufOut);
  }

}
ကိန်းပြည့် sqlite3OsSleep(sqlite3_vfs *pVfs, ကိန်းပြည့် nMicro){
  ပြန်ပေး pVfs->xSleep(pVfs, nMicro);
}
ကိန်းပြည့် sqlite3OsGetLastError(sqlite3_vfs *pVfs){
  ပြန်ပေး pVfs->xGetLastError ? pVfs->xGetLastError(pVfs, 0, 0) : 0;
}
ကိန်းပြည့် sqlite3OsCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *pTimeOut){
  ကိန်းပြည့် rc;
  /* IMPLEMENTATION-OF: R-49045-42493 SQLite will use the xCurrentTimeInt64()
  ** method to get the current date and time if that method is available
  ** (if iVersion is 2 or greater and the function pointer is not NULL) and
  ** will fall back to xCurrentTime() if xCurrentTimeInt64() is
  ** unavailable.
  */
  အကယ်၍( pVfs->iVersion>=2 && pVfs->xCurrentTimeInt64 ){
    rc = pVfs->xCurrentTimeInt64(pVfs, pTimeOut);
  }မဟုတ်ပါက{
    dviguu_na r;
    rc = pVfs->xCurrentTime(pVfs, &r);
    *pTimeOut = sqlite3RealToI64(r*86400000.0);
  }
  ပြန်ပေး rc;
}

ကိန်းပြည့် sqlite3OsOpenMalloc(
  sqlite3_vfs *pVfs,
  const var_na *zFile,
  sqlite3_file **ppFile,
  ကိန်းပြည့် flags,
  ကိန်းပြည့် *pOutFlags
){
  ကိန်းပြည့် rc;
  sqlite3_file *pFile;
  pFile = (sqlite3_file *)sqlite3MallocZero(pVfs->szOsFile);
  အကယ်၍( pFile ){
    rc = sqlite3OsOpen(pVfs, zFile, pFile, flags, pOutFlags);
    အကယ်၍( rc!=SQLITE_OK ){
      sqlite3_free(pFile);
      *ppFile = 0;
    }မဟုတ်ပါက{
      *ppFile = pFile;
    }
  }မဟုတ်ပါက{
    *ppFile = 0;
    rc = SQLITE_NOMEM_BKPT;
  }
  assert( *ppFile!=0 || rc!=SQLITE_OK );
  ပြန်ပေး rc;
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
ကိန်းပြည့် sqlite3OsInit(void){
  void *p = sqlite3_malloc(10);
  အကယ်၍( p==0 ) ပြန်ပေး SQLITE_NOMEM_BKPT;
  sqlite3_free(p);
  ပြန်ပေး sqlite3_os_init();
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
#အကယ်၍ SQLITE_THREADSAFE
  sqlite3_mutex *mutex;
#endif
#ifndef SQLITE_OMIT_AUTOINIT
  ကိန်းပြည့် rc = sqlite3_initialize();
  အကယ်၍( rc ) ပြန်ပေး 0;
#endif
#အကယ်၍ SQLITE_THREADSAFE
  mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
#endif
  sqlite3_mutex_enter(mutex);
  အတွက်(pVfs = vfsList; pVfs; pVfs=pVfs->pNext){
    အကယ်၍( zVfs==0 ) bha_nga;
    အကယ်၍( strcmp(zVfs, pVfs->zName)==0 ) bha_nga;
  }
  sqlite3_mutex_leave(mutex);
  ပြန်ပေး pVfs;
}

/*
** Unlink a VFS from the linked list
*/
static void vfsUnlink(sqlite3_vfs *pVfs){
  assert( sqlite3_mutex_held(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN)) );
  အကယ်၍( pVfs==0 ){
    /* No-op */
  }မဟုတ်ပါက အကယ်၍( vfsList==pVfs ){
    vfsList = pVfs->pNext;
  }မဟုတ်ပါက အကယ်၍( vfsList ){
    sqlite3_vfs *p = vfsList;
    နေစဉ်( p->pNext && p->pNext!=pVfs ){
      p = p->pNext;
    }
    အကယ်၍( p->pNext==pVfs ){
      p->pNext = pVfs->pNext;
    }
  }
}

/*
** Register a VFS with the system.  It is harmless to register the same
** VFS multiple times.  The new VFS becomes the default if makeDflt is
** true.
*/
ကိန်းပြည့် sqlite3_vfs_register(sqlite3_vfs *pVfs, ကိန်းပြည့် makeDflt){
  MUTEX_LOGIC(sqlite3_mutex *mutex;)
#ifndef SQLITE_OMIT_AUTOINIT
  ကိန်းပြည့် rc = sqlite3_initialize();
  အကယ်၍( rc ) ပြန်ပေး rc;
#endif
#ifdef SQLITE_ENABLE_API_ARMOR
  အကယ်၍( pVfs==0 ) ပြန်ပေး SQLITE_MISUSE_BKPT;
#endif

  MUTEX_LOGIC( mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  sqlite3_mutex_enter(mutex);
  vfsUnlink(pVfs);
  အကယ်၍( makeDflt || vfsList==0 ){
    pVfs->pNext = vfsList;
    vfsList = pVfs;
  }မဟုတ်ပါက{
    pVfs->pNext = vfsList->pNext;
    vfsList->pNext = pVfs;
  }
  assert(vfsList);
  sqlite3_mutex_leave(mutex);
  ပြန်ပေး SQLITE_OK;
}

/*
** Unregister a VFS so that it is no longer accessible.
*/
ကိန်းပြည့် sqlite3_vfs_unregister(sqlite3_vfs *pVfs){
  MUTEX_LOGIC(sqlite3_mutex *mutex;)
#ifndef SQLITE_OMIT_AUTOINIT
  ကိန်းပြည့် rc = sqlite3_initialize();
  အကယ်၍( rc ) ပြန်ပေး rc;
#endif
  MUTEX_LOGIC( mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  sqlite3_mutex_enter(mutex);
  vfsUnlink(pVfs);
  sqlite3_mutex_leave(mutex);
  ပြန်ပေး SQLITE_OK;
}
