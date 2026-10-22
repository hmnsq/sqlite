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
#अन्तर्भाव "sqliteInt.h"

/*
** If we compile with the SQLITE_TEST macro set, then the following block
** of code will give us the ability to simulate a disk I/O error.  This
** is used for testing the I/O recovery logic.
*/
#यदि defined(SQLITE_TEST)
पूर्णाङ्क sqlite3_io_error_hit = 0;            /* Total number of I/O Errors */
पूर्णाङ्क sqlite3_io_error_hardhit = 0;        /* Number of non-benign errors */
पूर्णाङ्क sqlite3_io_error_pending = 0;        /* Count down to first I/O error */
पूर्णाङ्क sqlite3_io_error_persist = 0;        /* True if I/O errors persist */
पूर्णाङ्क sqlite3_io_error_benign = 0;         /* True if errors are benign */
पूर्णाङ्क sqlite3_diskfull_pending = 0;
पूर्णाङ्क sqlite3_diskfull = 0;
#endif /* defined(SQLITE_TEST) */

/*
** When testing, also keep a count of the number of open files.
*/
#यदि defined(SQLITE_TEST)
पूर्णाङ्क sqlite3_open_file_गन्ती = 0;
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
#यदि defined(SQLITE_TEST)
पूर्णाङ्क sqlite3_memdebug_vfs_oom_test = 1;
  #define DO_OS_MALLOC_TEST(x)                                       \
  यदि (sqlite3_memdebug_vfs_oom_test && (!x || !sqlite3JournalIsInMemory(x))) { \
    शून्य *pTstAlloc = sqlite3Malloc(10);                             \
    यदि (!pTstAlloc) फर्काऊ SQLITE_IOERR_NOMEM_BKPT;                  \
    sqlite3_free(pTstAlloc);                                         \
  }
#अन्यथा
  #define DO_OS_MALLOC_TEST(x)
#endif

/*
** The following routines are convenience wrappers around methods
** of the sqlite3_file object.  This is mostly just syntactic sugar. All
** of this would be completely automatic if SQLite were coded using
** C++ instead of plain old C.
*/
शून्य sqlite3OsClose(sqlite3_file *pId){
  यदि( pId->pMethods ){
    pId->pMethods->xClose(pId);
    pId->pMethods = 0;
  }
}
पूर्णाङ्क sqlite3OsRead(sqlite3_file *id, शून्य *pBuf, पूर्णाङ्क amt, i64 offset){
  DO_OS_MALLOC_TEST(id);
  फर्काऊ id->pMethods->xRead(id, pBuf, amt, offset);
}
पूर्णाङ्क sqlite3OsWrite(sqlite3_file *id, अचर शून्य *pBuf, पूर्णाङ्क amt, i64 offset){
  DO_OS_MALLOC_TEST(id);
  फर्काऊ id->pMethods->xWrite(id, pBuf, amt, offset);
}
पूर्णाङ्क sqlite3OsTruncate(sqlite3_file *id, i64 size){
  फर्काऊ id->pMethods->xTruncate(id, size);
}
पूर्णाङ्क sqlite3OsSync(sqlite3_file *id, पूर्णाङ्क flags){
  DO_OS_MALLOC_TEST(id);
  फर्काऊ flags ? id->pMethods->xSync(id, flags) : SQLITE_OK;
}
पूर्णाङ्क sqlite3OsFileSize(sqlite3_file *id, i64 *pSize){
  DO_OS_MALLOC_TEST(id);
  फर्काऊ id->pMethods->xFileSize(id, pSize);
}
पूर्णाङ्क sqlite3OsLock(sqlite3_file *id, पूर्णाङ्क lockType){
  DO_OS_MALLOC_TEST(id);
  प्रतिज्ञा( lockType>=SQLITE_LOCK_SHARED && lockType<=SQLITE_LOCK_EXCLUSIVE );
  फर्काऊ id->pMethods->xLock(id, lockType);
}
पूर्णाङ्क sqlite3OsUnlock(sqlite3_file *id, पूर्णाङ्क lockType){
  प्रतिज्ञा( lockType==SQLITE_LOCK_NONE || lockType==SQLITE_LOCK_SHARED );
  फर्काऊ id->pMethods->xUnlock(id, lockType);
}
पूर्णाङ्क sqlite3OsCheckReservedLock(sqlite3_file *id, पूर्णाङ्क *pResOut){
  DO_OS_MALLOC_TEST(id);
  फर्काऊ id->pMethods->xCheckReservedLock(id, pResOut);
}

/*
** Use sqlite3OsFileControl() when we are doing something that might fail
** and we need to know about the failures.  Use sqlite3OsFileControlHint()
** when simply tossing information over the wall to the VFS and we do not
** really care if the VFS receives and understands the information since it
** is only a hint and can be safely ignored.  The sqlite3OsFileControlHint()
** routine has no return value since the return value would be meaningless.
*/
पूर्णाङ्क sqlite3OsFileControl(sqlite3_file *id, पूर्णाङ्क op, शून्य *pArg){
  यदि( id->pMethods==0 ) फर्काऊ SQLITE_NOTFOUND;
#ifdef SQLITE_TEST
  यदि( op!=SQLITE_FCNTL_COMMIT_PHASETWO
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
  फर्काऊ id->pMethods->xFileControl(id, op, pArg);
}
शून्य sqlite3OsFileControlHint(sqlite3_file *id, पूर्णाङ्क op, शून्य *pArg){
  यदि( id->pMethods ) (शून्य)id->pMethods->xFileControl(id, op, pArg);
}

पूर्णाङ्क sqlite3OsSectorSize(sqlite3_file *id){
  पूर्णाङ्क (*xSectorSize)(sqlite3_file*) = id->pMethods->xSectorSize;
  फर्काऊ (xSectorSize ? xSectorSize(id) : SQLITE_DEFAULT_SECTOR_SIZE);
}
पूर्णाङ्क sqlite3OsDeviceCharacteristics(sqlite3_file *id){
  यदि( NEVER(id->pMethods==0) ) फर्काऊ 0;
  फर्काऊ id->pMethods->xDeviceCharacteristics(id);
}
#ifndef SQLITE_OMIT_WAL
पूर्णाङ्क sqlite3OsShmLock(sqlite3_file *id, पूर्णाङ्क offset, पूर्णाङ्क n, पूर्णाङ्क flags){
  फर्काऊ id->pMethods->xShmLock(id, offset, n, flags);
}
शून्य sqlite3OsShmBarrier(sqlite3_file *id){
  id->pMethods->xShmBarrier(id);
}
पूर्णाङ्क sqlite3OsShmUnmap(sqlite3_file *id, पूर्णाङ्क deleteFlag){
  फर्काऊ id->pMethods->xShmUnmap(id, deleteFlag);
}
पूर्णाङ्क sqlite3OsShmMap(
  sqlite3_file *id,               /* Database file handle */
  पूर्णाङ्क iPage,
  पूर्णाङ्क pgsz,
  पूर्णाङ्क bExtend,                    /* True to extend file if necessary */
  शून्य चञ्चल **pp              /* OUT: Pointer to mapping */
){
  DO_OS_MALLOC_TEST(id);
  फर्काऊ id->pMethods->xShmMap(id, iPage, pgsz, bExtend, pp);
}
#endif /* SQLITE_OMIT_WAL */

#यदि SQLITE_MAX_MMAP_SIZE>0
/* The real implementation of xFetch and xUnfetch */
पूर्णाङ्क sqlite3OsFetch(sqlite3_file *id, i64 iOff, पूर्णाङ्क iAmt, शून्य **pp){
  DO_OS_MALLOC_TEST(id);
  फर्काऊ id->pMethods->xFetch(id, iOff, iAmt, pp);
}
पूर्णाङ्क sqlite3OsUnfetch(sqlite3_file *id, i64 iOff, शून्य *p){
  फर्काऊ id->pMethods->xUnfetch(id, iOff, p);
}
#अन्यथा
/* No-op stubs to use when memory-mapped I/O is disabled */
पूर्णाङ्क sqlite3OsFetch(sqlite3_file *id, i64 iOff, पूर्णाङ्क iAmt, शून्य **pp){
  *pp = 0;
  फर्काऊ SQLITE_OK;
}
पूर्णाङ्क sqlite3OsUnfetch(sqlite3_file *id, i64 iOff, शून्य *p){
  फर्काऊ SQLITE_OK;
}
#endif

/*
** The next group of routines are convenience wrappers around the
** VFS methods.
*/
पूर्णाङ्क sqlite3OsOpen(
  sqlite3_vfs *pVfs,
  अचर वर्ण *zPath,
  sqlite3_file *pFile,
  पूर्णाङ्क flags,
  पूर्णाङ्क *pFlagsOut
){
  पूर्णाङ्क rc;
  DO_OS_MALLOC_TEST(0);
  /* 0x87f7f is a mask of SQLITE_OPEN_ flags that are valid to be passed
  ** down into the VFS layer.  Some SQLITE_OPEN_ flags (for example,
  ** SQLITE_OPEN_FULLMUTEX or SQLITE_OPEN_SHAREDCACHE) are blocked before
  ** reaching the VFS. */
  प्रतिज्ञा( zPath || (flags & SQLITE_OPEN_EXCLUSIVE) );
  rc = pVfs->xOpen(pVfs, zPath, pFile, flags & 0x1087f7f, pFlagsOut);
  प्रतिज्ञा( rc==SQLITE_OK || pFile->pMethods==0 );
  फर्काऊ rc;
}
पूर्णाङ्क sqlite3OsDelete(sqlite3_vfs *pVfs, अचर वर्ण *zPath, पूर्णाङ्क dirSync){
  DO_OS_MALLOC_TEST(0);
  प्रतिज्ञा( dirSync==0 || dirSync==1 );
  फर्काऊ pVfs->xDelete!=0 ? pVfs->xDelete(pVfs, zPath, dirSync) : SQLITE_OK;
}
पूर्णाङ्क sqlite3OsAccess(
  sqlite3_vfs *pVfs,
  अचर वर्ण *zPath,
  पूर्णाङ्क flags,
  पूर्णाङ्क *pResOut
){
  DO_OS_MALLOC_TEST(0);
  फर्काऊ pVfs->xAccess(pVfs, zPath, flags, pResOut);
}
पूर्णाङ्क sqlite3OsFullPathname(
  sqlite3_vfs *pVfs,
  अचर वर्ण *zPath,
  पूर्णाङ्क nPathOut,
  वर्ण *zPathOut
){
  DO_OS_MALLOC_TEST(0);
  zPathOut[0] = 0;
  फर्काऊ pVfs->xFullPathname(pVfs, zPath, nPathOut, zPathOut);
}
#ifndef SQLITE_OMIT_LOAD_EXTENSION
शून्य *sqlite3OsDlOpen(sqlite3_vfs *pVfs, अचर वर्ण *zPath){
  प्रतिज्ञा( zPath!=0 );
  प्रतिज्ञा( म_माप(zPath)<=SQLITE_MAX_PATHLEN );  /* tag-20210611-1 */
  फर्काऊ pVfs->xDlOpen(pVfs, zPath);
}
शून्य sqlite3OsDlError(sqlite3_vfs *pVfs, पूर्णाङ्क nByte, वर्ण *zBufOut){
  pVfs->xDlError(pVfs, nByte, zBufOut);
}
शून्य (*sqlite3OsDlSym(sqlite3_vfs *pVfs, शून्य *pHdle, अचर वर्ण *zSym))(शून्य){
  फर्काऊ pVfs->xDlSym(pVfs, pHdle, zSym);
}
शून्य sqlite3OsDlClose(sqlite3_vfs *pVfs, शून्य *pHandle){
  pVfs->xDlClose(pVfs, pHandle);
}
#endif /* SQLITE_OMIT_LOAD_EXTENSION */
पूर्णाङ्क sqlite3OsRandomness(sqlite3_vfs *pVfs, पूर्णाङ्क nByte, वर्ण *zBufOut){
  यदि( sqlite3Config.iPrngSeed ){
    स_रखो(zBufOut, 0, nByte);
    यदि( ALWAYS(nByte>(चिन्हित)परिमाण(अचिन्हित)) ) nByte = परिमाण(अचिन्हित पूर्णाङ्क);
    स_नकल(zBufOut, &sqlite3Config.iPrngSeed, nByte);
    फर्काऊ SQLITE_OK;
  }अन्यथा{
    फर्काऊ pVfs->xRandomness(pVfs, nByte, zBufOut);
  }

}
पूर्णाङ्क sqlite3OsSleep(sqlite3_vfs *pVfs, पूर्णाङ्क nMicro){
  फर्काऊ pVfs->xSleep(pVfs, nMicro);
}
पूर्णाङ्क sqlite3OsGetLastError(sqlite3_vfs *pVfs){
  फर्काऊ pVfs->xGetLastError ? pVfs->xGetLastError(pVfs, 0, 0) : 0;
}
पूर्णाङ्क sqlite3OsCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *pTimeOut){
  पूर्णाङ्क rc;
  /* IMPLEMENTATION-OF: R-49045-42493 SQLite will use the xCurrentTimeInt64()
  ** method to get the current date and time if that method is available
  ** (if iVersion is 2 or greater and the function pointer is not NULL) and
  ** will fall back to xCurrentTime() if xCurrentTimeInt64() is
  ** unavailable.
  */
  यदि( pVfs->iVersion>=2 && pVfs->xCurrentTimeInt64 ){
    rc = pVfs->xCurrentTimeInt64(pVfs, pTimeOut);
  }अन्यथा{
    द्विगुण r;
    rc = pVfs->xCurrentTime(pVfs, &r);
    *pTimeOut = sqlite3RealToI64(r*86400000.0);
  }
  फर्काऊ rc;
}

पूर्णाङ्क sqlite3OsOpenMalloc(
  sqlite3_vfs *pVfs,
  अचर वर्ण *zFile,
  sqlite3_file **ppFile,
  पूर्णाङ्क flags,
  पूर्णाङ्क *pOutFlags
){
  पूर्णाङ्क rc;
  sqlite3_file *pFile;
  pFile = (sqlite3_file *)sqlite3MallocZero(pVfs->szOsFile);
  यदि( pFile ){
    rc = sqlite3OsOpen(pVfs, zFile, pFile, flags, pOutFlags);
    यदि( rc!=SQLITE_OK ){
      sqlite3_free(pFile);
      *ppFile = 0;
    }अन्यथा{
      *ppFile = pFile;
    }
  }अन्यथा{
    *ppFile = 0;
    rc = SQLITE_NOMEM_BKPT;
  }
  प्रतिज्ञा( *ppFile!=0 || rc!=SQLITE_OK );
  फर्काऊ rc;
}
शून्य sqlite3OsCloseFree(sqlite3_file *pFile){
  प्रतिज्ञा( pFile );
  sqlite3OsClose(pFile);
  sqlite3_free(pFile);
}

/*
** This function is a wrapper around the OS specific implementation of
** sqlite3_os_init(). The purpose of the wrapper is to provide the
** ability to simulate a malloc failure, so that the handling of an
** error in sqlite3_os_init() by the upper layers can be tested.
*/
पूर्णाङ्क sqlite3OsInit(शून्य){
  शून्य *p = sqlite3_malloc(10);
  यदि( p==0 ) फर्काऊ SQLITE_NOMEM_BKPT;
  sqlite3_free(p);
  फर्काऊ sqlite3_os_init();
}

/*
** The list of all registered VFS implementations.
*/
स्थिर sqlite3_vfs * SQLITE_WSD vfsList = 0;
#define vfsList GLOBAL(sqlite3_vfs *, vfsList)

/*
** Locate a VFS by name.  If no name is given, simply return the
** first VFS on the list.
*/
sqlite3_vfs *sqlite3_vfs_find(अचर वर्ण *zVfs){
  sqlite3_vfs *pVfs = 0;
#यदि SQLITE_THREADSAFE
  sqlite3_mutex *mutex;
#endif
#ifndef SQLITE_OMIT_AUTOINIT
  पूर्णाङ्क rc = sqlite3_initialize();
  यदि( rc ) फर्काऊ 0;
#endif
#यदि SQLITE_THREADSAFE
  mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
#endif
  sqlite3_mutex_enter(mutex);
  लागि(pVfs = vfsList; pVfs; pVfs=pVfs->pNext){
    यदि( zVfs==0 ) रोक;
    यदि( म_भेद(zVfs, pVfs->zName)==0 ) रोक;
  }
  sqlite3_mutex_leave(mutex);
  फर्काऊ pVfs;
}

/*
** Unlink a VFS from the linked list
*/
स्थिर शून्य vfsUnlink(sqlite3_vfs *pVfs){
  प्रतिज्ञा( sqlite3_mutex_held(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN)) );
  यदि( pVfs==0 ){
    /* No-op */
  }अन्यथा यदि( vfsList==pVfs ){
    vfsList = pVfs->pNext;
  }अन्यथा यदि( vfsList ){
    sqlite3_vfs *p = vfsList;
    जबसम्म( p->pNext && p->pNext!=pVfs ){
      p = p->pNext;
    }
    यदि( p->pNext==pVfs ){
      p->pNext = pVfs->pNext;
    }
  }
}

/*
** Register a VFS with the system.  It is harmless to register the same
** VFS multiple times.  The new VFS becomes the default if makeDflt is
** true.
*/
पूर्णाङ्क sqlite3_vfs_register(sqlite3_vfs *pVfs, पूर्णाङ्क makeDflt){
  MUTEX_LOGIC(sqlite3_mutex *mutex;)
#ifndef SQLITE_OMIT_AUTOINIT
  पूर्णाङ्क rc = sqlite3_initialize();
  यदि( rc ) फर्काऊ rc;
#endif
#ifdef SQLITE_ENABLE_API_ARMOR
  यदि( pVfs==0 ) फर्काऊ SQLITE_MISUSE_BKPT;
#endif

  MUTEX_LOGIC( mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  sqlite3_mutex_enter(mutex);
  vfsUnlink(pVfs);
  यदि( makeDflt || vfsList==0 ){
    pVfs->pNext = vfsList;
    vfsList = pVfs;
  }अन्यथा{
    pVfs->pNext = vfsList->pNext;
    vfsList->pNext = pVfs;
  }
  प्रतिज्ञा(vfsList);
  sqlite3_mutex_leave(mutex);
  फर्काऊ SQLITE_OK;
}

/*
** Unregister a VFS so that it is no longer accessible.
*/
पूर्णाङ्क sqlite3_vfs_unregister(sqlite3_vfs *pVfs){
  MUTEX_LOGIC(sqlite3_mutex *mutex;)
#ifndef SQLITE_OMIT_AUTOINIT
  पूर्णाङ्क rc = sqlite3_initialize();
  यदि( rc ) फर्काऊ rc;
#endif
  MUTEX_LOGIC( mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  sqlite3_mutex_enter(mutex);
  vfsUnlink(pVfs);
  sqlite3_mutex_leave(mutex);
  फर्काऊ SQLITE_OK;
}
