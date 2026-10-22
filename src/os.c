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
#અન્તર્ભાવ "sqliteInt.h"

/*
** If we compile with the SQLITE_TEST macro set, then the following block
** of code will give us the ability to simulate a disk I/O error.  This
** is used for testing the I/O recovery logic.
*/
#જો defined(SQLITE_TEST)
પૂર્ણાંક sqlite3_io_error_hit = 0;            /* Total number of I/O Errors */
પૂર્ણાંક sqlite3_io_error_hardhit = 0;        /* Number of non-benign errors */
પૂર્ણાંક sqlite3_io_error_pending = 0;        /* Count down to first I/O error */
પૂર્ણાંક sqlite3_io_error_persist = 0;        /* True if I/O errors persist */
પૂર્ણાંક sqlite3_io_error_benign = 0;         /* True if errors are benign */
પૂર્ણાંક sqlite3_diskfull_pending = 0;
પૂર્ણાંક sqlite3_diskfull = 0;
#endif /* defined(SQLITE_TEST) */

/*
** When testing, also keep a count of the number of open files.
*/
#જો defined(SQLITE_TEST)
પૂર્ણાંક sqlite3_open_file_ગણતરી = 0;
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
#જો defined(SQLITE_TEST)
પૂર્ણાંક sqlite3_memdebug_vfs_oom_test = 1;
  #define DO_OS_MALLOC_TEST(x)                                       \
  જો (sqlite3_memdebug_vfs_oom_test && (!x || !sqlite3JournalIsInMemory(x))) { \
    શૂન્ય *pTstAlloc = sqlite3Malloc(10);                             \
    જો (!pTstAlloc) પાછા SQLITE_IOERR_NOMEM_BKPT;                  \
    sqlite3_free(pTstAlloc);                                         \
  }
#નહીંતર
  #define DO_OS_MALLOC_TEST(x)
#endif

/*
** The following routines are convenience wrappers around methods
** of the sqlite3_file object.  This is mostly just syntactic sugar. All
** of this would be completely automatic if SQLite were coded using
** C++ instead of plain old C.
*/
શૂન્ય sqlite3OsClose(sqlite3_file *pId){
  જો( pId->pMethods ){
    pId->pMethods->xClose(pId);
    pId->pMethods = 0;
  }
}
પૂર્ણાંક sqlite3OsRead(sqlite3_file *id, શૂન્ય *pBuf, પૂર્ણાંક amt, i64 offset){
  DO_OS_MALLOC_TEST(id);
  પાછા id->pMethods->xRead(id, pBuf, amt, offset);
}
પૂર્ણાંક sqlite3OsWrite(sqlite3_file *id, અચલ શૂન્ય *pBuf, પૂર્ણાંક amt, i64 offset){
  DO_OS_MALLOC_TEST(id);
  પાછા id->pMethods->xWrite(id, pBuf, amt, offset);
}
પૂર્ણાંક sqlite3OsTruncate(sqlite3_file *id, i64 size){
  પાછા id->pMethods->xTruncate(id, size);
}
પૂર્ણાંક sqlite3OsSync(sqlite3_file *id, પૂર્ણાંક flags){
  DO_OS_MALLOC_TEST(id);
  પાછા flags ? id->pMethods->xSync(id, flags) : SQLITE_OK;
}
પૂર્ણાંક sqlite3OsFileSize(sqlite3_file *id, i64 *pSize){
  DO_OS_MALLOC_TEST(id);
  પાછા id->pMethods->xFileSize(id, pSize);
}
પૂર્ણાંક sqlite3OsLock(sqlite3_file *id, પૂર્ણાંક lockType){
  DO_OS_MALLOC_TEST(id);
  પ્રતિજ્ઞા( lockType>=SQLITE_LOCK_SHARED && lockType<=SQLITE_LOCK_EXCLUSIVE );
  પાછા id->pMethods->xLock(id, lockType);
}
પૂર્ણાંક sqlite3OsUnlock(sqlite3_file *id, પૂર્ણાંક lockType){
  પ્રતિજ્ઞા( lockType==SQLITE_LOCK_NONE || lockType==SQLITE_LOCK_SHARED );
  પાછા id->pMethods->xUnlock(id, lockType);
}
પૂર્ણાંક sqlite3OsCheckReservedLock(sqlite3_file *id, પૂર્ણાંક *pResOut){
  DO_OS_MALLOC_TEST(id);
  પાછા id->pMethods->xCheckReservedLock(id, pResOut);
}

/*
** Use sqlite3OsFileControl() when we are doing something that might fail
** and we need to know about the failures.  Use sqlite3OsFileControlHint()
** when simply tossing information over the wall to the VFS and we do not
** really care if the VFS receives and understands the information since it
** is only a hint and can be safely ignored.  The sqlite3OsFileControlHint()
** routine has no return value since the return value would be meaningless.
*/
પૂર્ણાંક sqlite3OsFileControl(sqlite3_file *id, પૂર્ણાંક op, શૂન્ય *pArg){
  જો( id->pMethods==0 ) પાછા SQLITE_NOTFOUND;
#ifdef SQLITE_TEST
  જો( op!=SQLITE_FCNTL_COMMIT_PHASETWO
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
  પાછા id->pMethods->xFileControl(id, op, pArg);
}
શૂન્ય sqlite3OsFileControlHint(sqlite3_file *id, પૂર્ણાંક op, શૂન્ય *pArg){
  જો( id->pMethods ) (શૂન્ય)id->pMethods->xFileControl(id, op, pArg);
}

પૂર્ણાંક sqlite3OsSectorSize(sqlite3_file *id){
  પૂર્ણાંક (*xSectorSize)(sqlite3_file*) = id->pMethods->xSectorSize;
  પાછા (xSectorSize ? xSectorSize(id) : SQLITE_DEFAULT_SECTOR_SIZE);
}
પૂર્ણાંક sqlite3OsDeviceCharacteristics(sqlite3_file *id){
  જો( NEVER(id->pMethods==0) ) પાછા 0;
  પાછા id->pMethods->xDeviceCharacteristics(id);
}
#ifndef SQLITE_OMIT_WAL
પૂર્ણાંક sqlite3OsShmLock(sqlite3_file *id, પૂર્ણાંક offset, પૂર્ણાંક n, પૂર્ણાંક flags){
  પાછા id->pMethods->xShmLock(id, offset, n, flags);
}
શૂન્ય sqlite3OsShmBarrier(sqlite3_file *id){
  id->pMethods->xShmBarrier(id);
}
પૂર્ણાંક sqlite3OsShmUnmap(sqlite3_file *id, પૂર્ણાંક deleteFlag){
  પાછા id->pMethods->xShmUnmap(id, deleteFlag);
}
પૂર્ણાંક sqlite3OsShmMap(
  sqlite3_file *id,               /* Database file handle */
  પૂર્ણાંક iPage,
  પૂર્ણાંક pgsz,
  પૂર્ણાંક bExtend,                    /* True to extend file if necessary */
  શૂન્ય અસ્થિર **pp              /* OUT: Pointer to mapping */
){
  DO_OS_MALLOC_TEST(id);
  પાછા id->pMethods->xShmMap(id, iPage, pgsz, bExtend, pp);
}
#endif /* SQLITE_OMIT_WAL */

#જો SQLITE_MAX_MMAP_SIZE>0
/* The real implementation of xFetch and xUnfetch */
પૂર્ણાંક sqlite3OsFetch(sqlite3_file *id, i64 iOff, પૂર્ણાંક iAmt, શૂન્ય **pp){
  DO_OS_MALLOC_TEST(id);
  પાછા id->pMethods->xFetch(id, iOff, iAmt, pp);
}
પૂર્ણાંક sqlite3OsUnfetch(sqlite3_file *id, i64 iOff, શૂન્ય *p){
  પાછા id->pMethods->xUnfetch(id, iOff, p);
}
#નહીંતર
/* No-op stubs to use when memory-mapped I/O is disabled */
પૂર્ણાંક sqlite3OsFetch(sqlite3_file *id, i64 iOff, પૂર્ણાંક iAmt, શૂન્ય **pp){
  *pp = 0;
  પાછા SQLITE_OK;
}
પૂર્ણાંક sqlite3OsUnfetch(sqlite3_file *id, i64 iOff, શૂન્ય *p){
  પાછા SQLITE_OK;
}
#endif

/*
** The next group of routines are convenience wrappers around the
** VFS methods.
*/
પૂર્ણાંક sqlite3OsOpen(
  sqlite3_vfs *pVfs,
  અચલ અક્ષર *zPath,
  sqlite3_file *pFile,
  પૂર્ણાંક flags,
  પૂર્ણાંક *pFlagsOut
){
  પૂર્ણાંક rc;
  DO_OS_MALLOC_TEST(0);
  /* 0x87f7f is a mask of SQLITE_OPEN_ flags that are valid to be passed
  ** down into the VFS layer.  Some SQLITE_OPEN_ flags (for example,
  ** SQLITE_OPEN_FULLMUTEX or SQLITE_OPEN_SHAREDCACHE) are blocked before
  ** reaching the VFS. */
  પ્રતિજ્ઞા( zPath || (flags & SQLITE_OPEN_EXCLUSIVE) );
  rc = pVfs->xOpen(pVfs, zPath, pFile, flags & 0x1087f7f, pFlagsOut);
  પ્રતિજ્ઞા( rc==SQLITE_OK || pFile->pMethods==0 );
  પાછા rc;
}
પૂર્ણાંક sqlite3OsDelete(sqlite3_vfs *pVfs, અચલ અક્ષર *zPath, પૂર્ણાંક dirSync){
  DO_OS_MALLOC_TEST(0);
  પ્રતિજ્ઞા( dirSync==0 || dirSync==1 );
  પાછા pVfs->xDelete!=0 ? pVfs->xDelete(pVfs, zPath, dirSync) : SQLITE_OK;
}
પૂર્ણાંક sqlite3OsAccess(
  sqlite3_vfs *pVfs,
  અચલ અક્ષર *zPath,
  પૂર્ણાંક flags,
  પૂર્ણાંક *pResOut
){
  DO_OS_MALLOC_TEST(0);
  પાછા pVfs->xAccess(pVfs, zPath, flags, pResOut);
}
પૂર્ણાંક sqlite3OsFullPathname(
  sqlite3_vfs *pVfs,
  અચલ અક્ષર *zPath,
  પૂર્ણાંક nPathOut,
  અક્ષર *zPathOut
){
  DO_OS_MALLOC_TEST(0);
  zPathOut[0] = 0;
  પાછા pVfs->xFullPathname(pVfs, zPath, nPathOut, zPathOut);
}
#ifndef SQLITE_OMIT_LOAD_EXTENSION
શૂન્ય *sqlite3OsDlOpen(sqlite3_vfs *pVfs, અચલ અક્ષર *zPath){
  પ્રતિજ્ઞા( zPath!=0 );
  પ્રતિજ્ઞા( strlen(zPath)<=SQLITE_MAX_PATHLEN );  /* tag-20210611-1 */
  પાછા pVfs->xDlOpen(pVfs, zPath);
}
શૂન્ય sqlite3OsDlError(sqlite3_vfs *pVfs, પૂર્ણાંક nByte, અક્ષર *zBufOut){
  pVfs->xDlError(pVfs, nByte, zBufOut);
}
શૂન્ય (*sqlite3OsDlSym(sqlite3_vfs *pVfs, શૂન્ય *pHdle, અચલ અક્ષર *zSym))(શૂન્ય){
  પાછા pVfs->xDlSym(pVfs, pHdle, zSym);
}
શૂન્ય sqlite3OsDlClose(sqlite3_vfs *pVfs, શૂન્ય *pHandle){
  pVfs->xDlClose(pVfs, pHandle);
}
#endif /* SQLITE_OMIT_LOAD_EXTENSION */
પૂર્ણાંક sqlite3OsRandomness(sqlite3_vfs *pVfs, પૂર્ણાંક nByte, અક્ષર *zBufOut){
  જો( sqlite3Config.iPrngSeed ){
    memset(zBufOut, 0, nByte);
    જો( ALWAYS(nByte>(signed)કદ(unsigned)) ) nByte = કદ(unsigned પૂર્ણાંક);
    memcpy(zBufOut, &sqlite3Config.iPrngSeed, nByte);
    પાછા SQLITE_OK;
  }નહીંતર{
    પાછા pVfs->xRandomness(pVfs, nByte, zBufOut);
  }

}
પૂર્ણાંક sqlite3OsSleep(sqlite3_vfs *pVfs, પૂર્ણાંક nMicro){
  પાછા pVfs->xSleep(pVfs, nMicro);
}
પૂર્ણાંક sqlite3OsGetLastError(sqlite3_vfs *pVfs){
  પાછા pVfs->xGetLastError ? pVfs->xGetLastError(pVfs, 0, 0) : 0;
}
પૂર્ણાંક sqlite3OsCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *pTimeOut){
  પૂર્ણાંક rc;
  /* IMPLEMENTATION-OF: R-49045-42493 SQLite will use the xCurrentTimeInt64()
  ** method to get the current date and time if that method is available
  ** (if iVersion is 2 or greater and the function pointer is not NULL) and
  ** will fall back to xCurrentTime() if xCurrentTimeInt64() is
  ** unavailable.
  */
  જો( pVfs->iVersion>=2 && pVfs->xCurrentTimeInt64 ){
    rc = pVfs->xCurrentTimeInt64(pVfs, pTimeOut);
  }નહીંતર{
    દ્વિગુણ r;
    rc = pVfs->xCurrentTime(pVfs, &r);
    *pTimeOut = sqlite3RealToI64(r*86400000.0);
  }
  પાછા rc;
}

પૂર્ણાંક sqlite3OsOpenMalloc(
  sqlite3_vfs *pVfs,
  અચલ અક્ષર *zFile,
  sqlite3_file **ppFile,
  પૂર્ણાંક flags,
  પૂર્ણાંક *pOutFlags
){
  પૂર્ણાંક rc;
  sqlite3_file *pFile;
  pFile = (sqlite3_file *)sqlite3MallocZero(pVfs->szOsFile);
  જો( pFile ){
    rc = sqlite3OsOpen(pVfs, zFile, pFile, flags, pOutFlags);
    જો( rc!=SQLITE_OK ){
      sqlite3_free(pFile);
      *ppFile = 0;
    }નહીંતર{
      *ppFile = pFile;
    }
  }નહીંતર{
    *ppFile = 0;
    rc = SQLITE_NOMEM_BKPT;
  }
  પ્રતિજ્ઞા( *ppFile!=0 || rc!=SQLITE_OK );
  પાછા rc;
}
શૂન્ય sqlite3OsCloseFree(sqlite3_file *pFile){
  પ્રતિજ્ઞા( pFile );
  sqlite3OsClose(pFile);
  sqlite3_free(pFile);
}

/*
** This function is a wrapper around the OS specific implementation of
** sqlite3_os_init(). The purpose of the wrapper is to provide the
** ability to simulate a malloc failure, so that the handling of an
** error in sqlite3_os_init() by the upper layers can be tested.
*/
પૂર્ણાંક sqlite3OsInit(શૂન્ય){
  શૂન્ય *p = sqlite3_malloc(10);
  જો( p==0 ) પાછા SQLITE_NOMEM_BKPT;
  sqlite3_free(p);
  પાછા sqlite3_os_init();
}

/*
** The list of all registered VFS implementations.
*/
સ્થિર sqlite3_vfs * SQLITE_WSD vfsList = 0;
#define vfsList GLOBAL(sqlite3_vfs *, vfsList)

/*
** Locate a VFS by name.  If no name is given, simply return the
** first VFS on the list.
*/
sqlite3_vfs *sqlite3_vfs_find(અચલ અક્ષર *zVfs){
  sqlite3_vfs *pVfs = 0;
#જો SQLITE_THREADSAFE
  sqlite3_mutex *mutex;
#endif
#ifndef SQLITE_OMIT_AUTOINIT
  પૂર્ણાંક rc = sqlite3_initialize();
  જો( rc ) પાછા 0;
#endif
#જો SQLITE_THREADSAFE
  mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
#endif
  sqlite3_mutex_enter(mutex);
  માટે(pVfs = vfsList; pVfs; pVfs=pVfs->pNext){
    જો( zVfs==0 ) તોડો;
    જો( strcmp(zVfs, pVfs->zName)==0 ) તોડો;
  }
  sqlite3_mutex_leave(mutex);
  પાછા pVfs;
}

/*
** Unlink a VFS from the linked list
*/
સ્થિર શૂન્ય vfsUnlink(sqlite3_vfs *pVfs){
  પ્રતિજ્ઞા( sqlite3_mutex_held(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN)) );
  જો( pVfs==0 ){
    /* No-op */
  }નહીંતર જો( vfsList==pVfs ){
    vfsList = pVfs->pNext;
  }નહીંતર જો( vfsList ){
    sqlite3_vfs *p = vfsList;
    જ્યારે( p->pNext && p->pNext!=pVfs ){
      p = p->pNext;
    }
    જો( p->pNext==pVfs ){
      p->pNext = pVfs->pNext;
    }
  }
}

/*
** Register a VFS with the system.  It is harmless to register the same
** VFS multiple times.  The new VFS becomes the default if makeDflt is
** true.
*/
પૂર્ણાંક sqlite3_vfs_register(sqlite3_vfs *pVfs, પૂર્ણાંક makeDflt){
  MUTEX_LOGIC(sqlite3_mutex *mutex;)
#ifndef SQLITE_OMIT_AUTOINIT
  પૂર્ણાંક rc = sqlite3_initialize();
  જો( rc ) પાછા rc;
#endif
#ifdef SQLITE_ENABLE_API_ARMOR
  જો( pVfs==0 ) પાછા SQLITE_MISUSE_BKPT;
#endif

  MUTEX_LOGIC( mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  sqlite3_mutex_enter(mutex);
  vfsUnlink(pVfs);
  જો( makeDflt || vfsList==0 ){
    pVfs->pNext = vfsList;
    vfsList = pVfs;
  }નહીંતર{
    pVfs->pNext = vfsList->pNext;
    vfsList->pNext = pVfs;
  }
  પ્રતિજ્ઞા(vfsList);
  sqlite3_mutex_leave(mutex);
  પાછા SQLITE_OK;
}

/*
** Unregister a VFS so that it is no longer accessible.
*/
પૂર્ણાંક sqlite3_vfs_unregister(sqlite3_vfs *pVfs){
  MUTEX_LOGIC(sqlite3_mutex *mutex;)
#ifndef SQLITE_OMIT_AUTOINIT
  પૂર્ણાંક rc = sqlite3_initialize();
  જો( rc ) પાછા rc;
#endif
  MUTEX_LOGIC( mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  sqlite3_mutex_enter(mutex);
  vfsUnlink(pVfs);
  sqlite3_mutex_leave(mutex);
  પાછા SQLITE_OK;
}
