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
#അന്തര്ഭാവ "sqliteInt.h"

/*
** If we compile with the SQLITE_TEST macro set, then the following block
** of code will give us the ability to simulate a disk I/O error.  This
** is used for testing the I/O recovery logic.
*/
#എങ്കിൽ defined(SQLITE_TEST)
പൂർണ്ണസംഖ്യ sqlite3_io_error_hit = 0;            /* Total number of I/O Errors */
പൂർണ്ണസംഖ്യ sqlite3_io_error_hardhit = 0;        /* Number of non-benign errors */
പൂർണ്ണസംഖ്യ sqlite3_io_error_pending = 0;        /* Count down to first I/O error */
പൂർണ്ണസംഖ്യ sqlite3_io_error_persist = 0;        /* True if I/O errors persist */
പൂർണ്ണസംഖ്യ sqlite3_io_error_benign = 0;         /* True if errors are benign */
പൂർണ്ണസംഖ്യ sqlite3_diskfull_pending = 0;
പൂർണ്ണസംഖ്യ sqlite3_diskfull = 0;
#endif /* defined(SQLITE_TEST) */

/*
** When testing, also keep a count of the number of open files.
*/
#എങ്കിൽ defined(SQLITE_TEST)
പൂർണ്ണസംഖ്യ sqlite3_open_file_എണ്ണം = 0;
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
#എങ്കിൽ defined(SQLITE_TEST)
പൂർണ്ണസംഖ്യ sqlite3_memdebug_vfs_oom_test = 1;
  #define DO_OS_MALLOC_TEST(x)                                       \
  എങ്കിൽ (sqlite3_memdebug_vfs_oom_test && (!x || !sqlite3JournalIsInMemory(x))) { \
    ശൂന്യം *pTstAlloc = sqlite3Malloc(10);                             \
    എങ്കിൽ (!pTstAlloc) തിരികെ SQLITE_IOERR_NOMEM_BKPT;                  \
    sqlite3_free(pTstAlloc);                                         \
  }
#അല്ലെങ്കിൽ
  #define DO_OS_MALLOC_TEST(x)
#endif

/*
** The following routines are convenience wrappers around methods
** of the sqlite3_file object.  This is mostly just syntactic sugar. All
** of this would be completely automatic if SQLite were coded using
** C++ instead of plain old C.
*/
ശൂന്യം sqlite3OsClose(sqlite3_file *pId){
  എങ്കിൽ( pId->pMethods ){
    pId->pMethods->xClose(pId);
    pId->pMethods = 0;
  }
}
പൂർണ്ണസംഖ്യ sqlite3OsRead(sqlite3_file *id, ശൂന്യം *pBuf, പൂർണ്ണസംഖ്യ amt, i64 offset){
  DO_OS_MALLOC_TEST(id);
  തിരികെ id->pMethods->xRead(id, pBuf, amt, offset);
}
പൂർണ്ണസംഖ്യ sqlite3OsWrite(sqlite3_file *id, സ്ഥിരം ശൂന്യം *pBuf, പൂർണ്ണസംഖ്യ amt, i64 offset){
  DO_OS_MALLOC_TEST(id);
  തിരികെ id->pMethods->xWrite(id, pBuf, amt, offset);
}
പൂർണ്ണസംഖ്യ sqlite3OsTruncate(sqlite3_file *id, i64 size){
  തിരികെ id->pMethods->xTruncate(id, size);
}
പൂർണ്ണസംഖ്യ sqlite3OsSync(sqlite3_file *id, പൂർണ്ണസംഖ്യ flags){
  DO_OS_MALLOC_TEST(id);
  തിരികെ flags ? id->pMethods->xSync(id, flags) : SQLITE_OK;
}
പൂർണ്ണസംഖ്യ sqlite3OsFileSize(sqlite3_file *id, i64 *pSize){
  DO_OS_MALLOC_TEST(id);
  തിരികെ id->pMethods->xFileSize(id, pSize);
}
പൂർണ്ണസംഖ്യ sqlite3OsLock(sqlite3_file *id, പൂർണ്ണസംഖ്യ lockType){
  DO_OS_MALLOC_TEST(id);
  പ്രതിജ്ഞാ( lockType>=SQLITE_LOCK_SHARED && lockType<=SQLITE_LOCK_EXCLUSIVE );
  തിരികെ id->pMethods->xLock(id, lockType);
}
പൂർണ്ണസംഖ്യ sqlite3OsUnlock(sqlite3_file *id, പൂർണ്ണസംഖ്യ lockType){
  പ്രതിജ്ഞാ( lockType==SQLITE_LOCK_NONE || lockType==SQLITE_LOCK_SHARED );
  തിരികെ id->pMethods->xUnlock(id, lockType);
}
പൂർണ്ണസംഖ്യ sqlite3OsCheckReservedLock(sqlite3_file *id, പൂർണ്ണസംഖ്യ *pResOut){
  DO_OS_MALLOC_TEST(id);
  തിരികെ id->pMethods->xCheckReservedLock(id, pResOut);
}

/*
** Use sqlite3OsFileControl() when we are doing something that might fail
** and we need to know about the failures.  Use sqlite3OsFileControlHint()
** when simply tossing information over the wall to the VFS and we do not
** really care if the VFS receives and understands the information since it
** is only a hint and can be safely ignored.  The sqlite3OsFileControlHint()
** routine has no return value since the return value would be meaningless.
*/
പൂർണ്ണസംഖ്യ sqlite3OsFileControl(sqlite3_file *id, പൂർണ്ണസംഖ്യ op, ശൂന്യം *pArg){
  എങ്കിൽ( id->pMethods==0 ) തിരികെ SQLITE_NOTFOUND;
#ifdef SQLITE_TEST
  എങ്കിൽ( op!=SQLITE_FCNTL_COMMIT_PHASETWO
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
  തിരികെ id->pMethods->xFileControl(id, op, pArg);
}
ശൂന്യം sqlite3OsFileControlHint(sqlite3_file *id, പൂർണ്ണസംഖ്യ op, ശൂന്യം *pArg){
  എങ്കിൽ( id->pMethods ) (ശൂന്യം)id->pMethods->xFileControl(id, op, pArg);
}

പൂർണ്ണസംഖ്യ sqlite3OsSectorSize(sqlite3_file *id){
  പൂർണ്ണസംഖ്യ (*xSectorSize)(sqlite3_file*) = id->pMethods->xSectorSize;
  തിരികെ (xSectorSize ? xSectorSize(id) : SQLITE_DEFAULT_SECTOR_SIZE);
}
പൂർണ്ണസംഖ്യ sqlite3OsDeviceCharacteristics(sqlite3_file *id){
  എങ്കിൽ( NEVER(id->pMethods==0) ) തിരികെ 0;
  തിരികെ id->pMethods->xDeviceCharacteristics(id);
}
#ifndef SQLITE_OMIT_WAL
പൂർണ്ണസംഖ്യ sqlite3OsShmLock(sqlite3_file *id, പൂർണ്ണസംഖ്യ offset, പൂർണ്ണസംഖ്യ n, പൂർണ്ണസംഖ്യ flags){
  തിരികെ id->pMethods->xShmLock(id, offset, n, flags);
}
ശൂന്യം sqlite3OsShmBarrier(sqlite3_file *id){
  id->pMethods->xShmBarrier(id);
}
പൂർണ്ണസംഖ്യ sqlite3OsShmUnmap(sqlite3_file *id, പൂർണ്ണസംഖ്യ deleteFlag){
  തിരികെ id->pMethods->xShmUnmap(id, deleteFlag);
}
പൂർണ്ണസംഖ്യ sqlite3OsShmMap(
  sqlite3_file *id,               /* Database file handle */
  പൂർണ്ണസംഖ്യ iPage,
  പൂർണ്ണസംഖ്യ pgsz,
  പൂർണ്ണസംഖ്യ bExtend,                    /* True to extend file if necessary */
  ശൂന്യം അസ്ഥിര **pp              /* OUT: Pointer to mapping */
){
  DO_OS_MALLOC_TEST(id);
  തിരികെ id->pMethods->xShmMap(id, iPage, pgsz, bExtend, pp);
}
#endif /* SQLITE_OMIT_WAL */

#എങ്കിൽ SQLITE_MAX_MMAP_SIZE>0
/* The real implementation of xFetch and xUnfetch */
പൂർണ്ണസംഖ്യ sqlite3OsFetch(sqlite3_file *id, i64 iOff, പൂർണ്ണസംഖ്യ iAmt, ശൂന്യം **pp){
  DO_OS_MALLOC_TEST(id);
  തിരികെ id->pMethods->xFetch(id, iOff, iAmt, pp);
}
പൂർണ്ണസംഖ്യ sqlite3OsUnfetch(sqlite3_file *id, i64 iOff, ശൂന്യം *p){
  തിരികെ id->pMethods->xUnfetch(id, iOff, p);
}
#അല്ലെങ്കിൽ
/* No-op stubs to use when memory-mapped I/O is disabled */
പൂർണ്ണസംഖ്യ sqlite3OsFetch(sqlite3_file *id, i64 iOff, പൂർണ്ണസംഖ്യ iAmt, ശൂന്യം **pp){
  *pp = 0;
  തിരികെ SQLITE_OK;
}
പൂർണ്ണസംഖ്യ sqlite3OsUnfetch(sqlite3_file *id, i64 iOff, ശൂന്യം *p){
  തിരികെ SQLITE_OK;
}
#endif

/*
** The next group of routines are convenience wrappers around the
** VFS methods.
*/
പൂർണ്ണസംഖ്യ sqlite3OsOpen(
  sqlite3_vfs *pVfs,
  സ്ഥിരം അക്ഷരം *zPath,
  sqlite3_file *pFile,
  പൂർണ്ണസംഖ്യ flags,
  പൂർണ്ണസംഖ്യ *pFlagsOut
){
  പൂർണ്ണസംഖ്യ rc;
  DO_OS_MALLOC_TEST(0);
  /* 0x87f7f is a mask of SQLITE_OPEN_ flags that are valid to be passed
  ** down into the VFS layer.  Some SQLITE_OPEN_ flags (for example,
  ** SQLITE_OPEN_FULLMUTEX or SQLITE_OPEN_SHAREDCACHE) are blocked before
  ** reaching the VFS. */
  പ്രതിജ്ഞാ( zPath || (flags & SQLITE_OPEN_EXCLUSIVE) );
  rc = pVfs->xOpen(pVfs, zPath, pFile, flags & 0x1087f7f, pFlagsOut);
  പ്രതിജ്ഞാ( rc==SQLITE_OK || pFile->pMethods==0 );
  തിരികെ rc;
}
പൂർണ്ണസംഖ്യ sqlite3OsDelete(sqlite3_vfs *pVfs, സ്ഥിരം അക്ഷരം *zPath, പൂർണ്ണസംഖ്യ dirSync){
  DO_OS_MALLOC_TEST(0);
  പ്രതിജ്ഞാ( dirSync==0 || dirSync==1 );
  തിരികെ pVfs->xDelete!=0 ? pVfs->xDelete(pVfs, zPath, dirSync) : SQLITE_OK;
}
പൂർണ്ണസംഖ്യ sqlite3OsAccess(
  sqlite3_vfs *pVfs,
  സ്ഥിരം അക്ഷരം *zPath,
  പൂർണ്ണസംഖ്യ flags,
  പൂർണ്ണസംഖ്യ *pResOut
){
  DO_OS_MALLOC_TEST(0);
  തിരികെ pVfs->xAccess(pVfs, zPath, flags, pResOut);
}
പൂർണ്ണസംഖ്യ sqlite3OsFullPathname(
  sqlite3_vfs *pVfs,
  സ്ഥിരം അക്ഷരം *zPath,
  പൂർണ്ണസംഖ്യ nPathOut,
  അക്ഷരം *zPathOut
){
  DO_OS_MALLOC_TEST(0);
  zPathOut[0] = 0;
  തിരികെ pVfs->xFullPathname(pVfs, zPath, nPathOut, zPathOut);
}
#ifndef SQLITE_OMIT_LOAD_EXTENSION
ശൂന്യം *sqlite3OsDlOpen(sqlite3_vfs *pVfs, സ്ഥിരം അക്ഷരം *zPath){
  പ്രതിജ്ഞാ( zPath!=0 );
  പ്രതിജ്ഞാ( strlen(zPath)<=SQLITE_MAX_PATHLEN );  /* tag-20210611-1 */
  തിരികെ pVfs->xDlOpen(pVfs, zPath);
}
ശൂന്യം sqlite3OsDlError(sqlite3_vfs *pVfs, പൂർണ്ണസംഖ്യ nByte, അക്ഷരം *zBufOut){
  pVfs->xDlError(pVfs, nByte, zBufOut);
}
ശൂന്യം (*sqlite3OsDlSym(sqlite3_vfs *pVfs, ശൂന്യം *pHdle, സ്ഥിരം അക്ഷരം *zSym))(ശൂന്യം){
  തിരികെ pVfs->xDlSym(pVfs, pHdle, zSym);
}
ശൂന്യം sqlite3OsDlClose(sqlite3_vfs *pVfs, ശൂന്യം *pHandle){
  pVfs->xDlClose(pVfs, pHandle);
}
#endif /* SQLITE_OMIT_LOAD_EXTENSION */
പൂർണ്ണസംഖ്യ sqlite3OsRandomness(sqlite3_vfs *pVfs, പൂർണ്ണസംഖ്യ nByte, അക്ഷരം *zBufOut){
  എങ്കിൽ( sqlite3Config.iPrngSeed ){
    memset(zBufOut, 0, nByte);
    എങ്കിൽ( ALWAYS(nByte>(signed)വലുപ്പം(unsigned)) ) nByte = വലുപ്പം(unsigned പൂർണ്ണസംഖ്യ);
    memcpy(zBufOut, &sqlite3Config.iPrngSeed, nByte);
    തിരികെ SQLITE_OK;
  }അല്ലെങ്കിൽ{
    തിരികെ pVfs->xRandomness(pVfs, nByte, zBufOut);
  }

}
പൂർണ്ണസംഖ്യ sqlite3OsSleep(sqlite3_vfs *pVfs, പൂർണ്ണസംഖ്യ nMicro){
  തിരികെ pVfs->xSleep(pVfs, nMicro);
}
പൂർണ്ണസംഖ്യ sqlite3OsGetLastError(sqlite3_vfs *pVfs){
  തിരികെ pVfs->xGetLastError ? pVfs->xGetLastError(pVfs, 0, 0) : 0;
}
പൂർണ്ണസംഖ്യ sqlite3OsCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *pTimeOut){
  പൂർണ്ണസംഖ്യ rc;
  /* IMPLEMENTATION-OF: R-49045-42493 SQLite will use the xCurrentTimeInt64()
  ** method to get the current date and time if that method is available
  ** (if iVersion is 2 or greater and the function pointer is not NULL) and
  ** will fall back to xCurrentTime() if xCurrentTimeInt64() is
  ** unavailable.
  */
  എങ്കിൽ( pVfs->iVersion>=2 && pVfs->xCurrentTimeInt64 ){
    rc = pVfs->xCurrentTimeInt64(pVfs, pTimeOut);
  }അല്ലെങ്കിൽ{
    ഇരട്ട r;
    rc = pVfs->xCurrentTime(pVfs, &r);
    *pTimeOut = sqlite3RealToI64(r*86400000.0);
  }
  തിരികെ rc;
}

പൂർണ്ണസംഖ്യ sqlite3OsOpenMalloc(
  sqlite3_vfs *pVfs,
  സ്ഥിരം അക്ഷരം *zFile,
  sqlite3_file **ppFile,
  പൂർണ്ണസംഖ്യ flags,
  പൂർണ്ണസംഖ്യ *pOutFlags
){
  പൂർണ്ണസംഖ്യ rc;
  sqlite3_file *pFile;
  pFile = (sqlite3_file *)sqlite3MallocZero(pVfs->szOsFile);
  എങ്കിൽ( pFile ){
    rc = sqlite3OsOpen(pVfs, zFile, pFile, flags, pOutFlags);
    എങ്കിൽ( rc!=SQLITE_OK ){
      sqlite3_free(pFile);
      *ppFile = 0;
    }അല്ലെങ്കിൽ{
      *ppFile = pFile;
    }
  }അല്ലെങ്കിൽ{
    *ppFile = 0;
    rc = SQLITE_NOMEM_BKPT;
  }
  പ്രതിജ്ഞാ( *ppFile!=0 || rc!=SQLITE_OK );
  തിരികെ rc;
}
ശൂന്യം sqlite3OsCloseFree(sqlite3_file *pFile){
  പ്രതിജ്ഞാ( pFile );
  sqlite3OsClose(pFile);
  sqlite3_free(pFile);
}

/*
** This function is a wrapper around the OS specific implementation of
** sqlite3_os_init(). The purpose of the wrapper is to provide the
** ability to simulate a malloc failure, so that the handling of an
** error in sqlite3_os_init() by the upper layers can be tested.
*/
പൂർണ്ണസംഖ്യ sqlite3OsInit(ശൂന്യം){
  ശൂന്യം *p = sqlite3_malloc(10);
  എങ്കിൽ( p==0 ) തിരികെ SQLITE_NOMEM_BKPT;
  sqlite3_free(p);
  തിരികെ sqlite3_os_init();
}

/*
** The list of all registered VFS implementations.
*/
സ്ഥിതം sqlite3_vfs * SQLITE_WSD vfsList = 0;
#define vfsList GLOBAL(sqlite3_vfs *, vfsList)

/*
** Locate a VFS by name.  If no name is given, simply return the
** first VFS on the list.
*/
sqlite3_vfs *sqlite3_vfs_find(സ്ഥിരം അക്ഷരം *zVfs){
  sqlite3_vfs *pVfs = 0;
#എങ്കിൽ SQLITE_THREADSAFE
  sqlite3_mutex *mutex;
#endif
#ifndef SQLITE_OMIT_AUTOINIT
  പൂർണ്ണസംഖ്യ rc = sqlite3_initialize();
  എങ്കിൽ( rc ) തിരികെ 0;
#endif
#എങ്കിൽ SQLITE_THREADSAFE
  mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
#endif
  sqlite3_mutex_enter(mutex);
  വേണ്ടി(pVfs = vfsList; pVfs; pVfs=pVfs->pNext){
    എങ്കിൽ( zVfs==0 ) നിർത്തുക;
    എങ്കിൽ( strcmp(zVfs, pVfs->zName)==0 ) നിർത്തുക;
  }
  sqlite3_mutex_leave(mutex);
  തിരികെ pVfs;
}

/*
** Unlink a VFS from the linked list
*/
സ്ഥിതം ശൂന്യം vfsUnlink(sqlite3_vfs *pVfs){
  പ്രതിജ്ഞാ( sqlite3_mutex_held(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN)) );
  എങ്കിൽ( pVfs==0 ){
    /* No-op */
  }അല്ലെങ്കിൽ എങ്കിൽ( vfsList==pVfs ){
    vfsList = pVfs->pNext;
  }അല്ലെങ്കിൽ എങ്കിൽ( vfsList ){
    sqlite3_vfs *p = vfsList;
    ആകുമ്പോൾ( p->pNext && p->pNext!=pVfs ){
      p = p->pNext;
    }
    എങ്കിൽ( p->pNext==pVfs ){
      p->pNext = pVfs->pNext;
    }
  }
}

/*
** Register a VFS with the system.  It is harmless to register the same
** VFS multiple times.  The new VFS becomes the default if makeDflt is
** true.
*/
പൂർണ്ണസംഖ്യ sqlite3_vfs_register(sqlite3_vfs *pVfs, പൂർണ്ണസംഖ്യ makeDflt){
  MUTEX_LOGIC(sqlite3_mutex *mutex;)
#ifndef SQLITE_OMIT_AUTOINIT
  പൂർണ്ണസംഖ്യ rc = sqlite3_initialize();
  എങ്കിൽ( rc ) തിരികെ rc;
#endif
#ifdef SQLITE_ENABLE_API_ARMOR
  എങ്കിൽ( pVfs==0 ) തിരികെ SQLITE_MISUSE_BKPT;
#endif

  MUTEX_LOGIC( mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  sqlite3_mutex_enter(mutex);
  vfsUnlink(pVfs);
  എങ്കിൽ( makeDflt || vfsList==0 ){
    pVfs->pNext = vfsList;
    vfsList = pVfs;
  }അല്ലെങ്കിൽ{
    pVfs->pNext = vfsList->pNext;
    vfsList->pNext = pVfs;
  }
  പ്രതിജ്ഞാ(vfsList);
  sqlite3_mutex_leave(mutex);
  തിരികെ SQLITE_OK;
}

/*
** Unregister a VFS so that it is no longer accessible.
*/
പൂർണ്ണസംഖ്യ sqlite3_vfs_unregister(sqlite3_vfs *pVfs){
  MUTEX_LOGIC(sqlite3_mutex *mutex;)
#ifndef SQLITE_OMIT_AUTOINIT
  പൂർണ്ണസംഖ്യ rc = sqlite3_initialize();
  എങ്കിൽ( rc ) തിരികെ rc;
#endif
  MUTEX_LOGIC( mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  sqlite3_mutex_enter(mutex);
  vfsUnlink(pVfs);
  sqlite3_mutex_leave(mutex);
  തിരികെ SQLITE_OK;
}
