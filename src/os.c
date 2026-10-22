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
#समावेश "sqliteInt.h"

/*
** If we compile with the SQLITE_TEST macro set, then the following block
** of code will give us the ability to simulate a disk I/O error.  This
** is used for testing the I/O recovery logic.
*/
#अगर defined(SQLITE_TEST)
पूर्णांक sqlite3_io_error_hit = 0;            /* Total number of I/O Errors */
पूर्णांक sqlite3_io_error_hardhit = 0;        /* Number of non-benign errors */
पूर्णांक sqlite3_io_error_pending = 0;        /* Count down to first I/O error */
पूर्णांक sqlite3_io_error_persist = 0;        /* True if I/O errors persist */
पूर्णांक sqlite3_io_error_benign = 0;         /* True if errors are benign */
पूर्णांक sqlite3_diskfull_pending = 0;
पूर्णांक sqlite3_diskfull = 0;
#पूर्ण_अगर /* defined(SQLITE_TEST) */

/*
** When testing, also keep a count of the number of open files.
*/
#अगर defined(SQLITE_TEST)
पूर्णांक sqlite3_open_file_गिनती = 0;
#पूर्ण_अगर /* defined(SQLITE_TEST) */

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
#अगर defined(SQLITE_TEST)
पूर्णांक sqlite3_memdebug_vfs_oom_test = 1;
  #घोषणा DO_OS_MALLOC_TEST(x)                                       \
  अगर (sqlite3_memdebug_vfs_oom_test && (!x || !sqlite3JournalIsInMemory(x))) { \
    व्योम *pTstAlloc = sqlite3Malloc(10);                             \
    अगर (!pTstAlloc) वापस SQLITE_IOERR_NOMEM_BKPT;                  \
    sqlite3_free(pTstAlloc);                                         \
  }
#अन्यथा
  #घोषणा DO_OS_MALLOC_TEST(x)
#पूर्ण_अगर

/*
** The following routines are convenience wrappers around methods
** of the sqlite3_file object.  This is mostly just syntactic sugar. All
** of this would be completely automatic if SQLite were coded using
** C++ instead of plain old C.
*/
व्योम sqlite3OsClose(sqlite3_file *pId){
  अगर( pId->pMethods ){
    pId->pMethods->xClose(pId);
    pId->pMethods = 0;
  }
}
पूर्णांक sqlite3OsRead(sqlite3_file *id, व्योम *pBuf, पूर्णांक amt, i64 offset){
  DO_OS_MALLOC_TEST(id);
  वापस id->pMethods->xRead(id, pBuf, amt, offset);
}
पूर्णांक sqlite3OsWrite(sqlite3_file *id, स्थिर व्योम *pBuf, पूर्णांक amt, i64 offset){
  DO_OS_MALLOC_TEST(id);
  वापस id->pMethods->xWrite(id, pBuf, amt, offset);
}
पूर्णांक sqlite3OsTruncate(sqlite3_file *id, i64 आकार){
  वापस id->pMethods->xTruncate(id, आकार);
}
पूर्णांक sqlite3OsSync(sqlite3_file *id, पूर्णांक flags){
  DO_OS_MALLOC_TEST(id);
  वापस flags ? id->pMethods->xSync(id, flags) : SQLITE_OK;
}
पूर्णांक sqlite3_Os_File_आकार(sqlite3_file *id, i64 *p_आकार){
  DO_OS_MALLOC_TEST(id);
  वापस id->pMethods->x_File_आकार(id, p_आकार);
}
पूर्णांक sqlite3OsLock(sqlite3_file *id, पूर्णांक lockType){
  DO_OS_MALLOC_TEST(id);
  निश्चित( lockType>=SQLITE_LOCK_SHARED && lockType<=SQLITE_LOCK_EXCLUSIVE );
  वापस id->pMethods->xLock(id, lockType);
}
पूर्णांक sqlite3OsUnlock(sqlite3_file *id, पूर्णांक lockType){
  निश्चित( lockType==SQLITE_LOCK_NONE || lockType==SQLITE_LOCK_SHARED );
  वापस id->pMethods->xUnlock(id, lockType);
}
पूर्णांक sqlite3OsCheckReservedLock(sqlite3_file *id, पूर्णांक *pResOut){
  DO_OS_MALLOC_TEST(id);
  वापस id->pMethods->xCheckReservedLock(id, pResOut);
}

/*
** Use sqlite3OsFileControl() when we are doing something that might fail
** and we need to know about the failures.  Use sqlite3OsFileControlHint()
** when simply tossing information over the wall to the VFS and we do not
** really care if the VFS receives and understands the information since it
** is only a hint and can be safely ignored.  The sqlite3OsFileControlHint()
** routine has no return value since the return value would be meaningless.
*/
पूर्णांक sqlite3OsFileControl(sqlite3_file *id, पूर्णांक op, व्योम *pArg){
  अगर( id->pMethods==0 ) वापस SQLITE_NOTFOUND;
#अगर_घोषित SQLITE_TEST
  अगर( op!=SQLITE_FCNTL_COMMIT_PHASETWO
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
#पूर्ण_अगर
  वापस id->pMethods->xFileControl(id, op, pArg);
}
व्योम sqlite3OsFileControlHint(sqlite3_file *id, पूर्णांक op, व्योम *pArg){
  अगर( id->pMethods ) (व्योम)id->pMethods->xFileControl(id, op, pArg);
}

पूर्णांक sqlite3_Os_Sector_आकार(sqlite3_file *id){
  पूर्णांक (*x_Sector_आकार)(sqlite3_file*) = id->pMethods->x_Sector_आकार;
  वापस (x_Sector_आकार ? x_Sector_आकार(id) : SQLITE_DEFAULT_SECTOR_आकार);
}
पूर्णांक sqlite3OsDeviceCharacteristics(sqlite3_file *id){
  अगर( NEVER(id->pMethods==0) ) वापस 0;
  वापस id->pMethods->xDeviceCharacteristics(id);
}
#अगर_अघोषित SQLITE_OMIT_WAL
पूर्णांक sqlite3OsShmLock(sqlite3_file *id, पूर्णांक offset, पूर्णांक n, पूर्णांक flags){
  वापस id->pMethods->xShmLock(id, offset, n, flags);
}
व्योम sqlite3OsShmBarrier(sqlite3_file *id){
  id->pMethods->xShmBarrier(id);
}
पूर्णांक sqlite3OsShmUnmap(sqlite3_file *id, पूर्णांक deleteFlag){
  वापस id->pMethods->xShmUnmap(id, deleteFlag);
}
पूर्णांक sqlite3OsShmMap(
  sqlite3_file *id,               /* Database file handle */
  पूर्णांक iPage,
  पूर्णांक pgsz,
  पूर्णांक bExtend,                    /* True to extend file if necessary */
  व्योम अस्थिर **pp              /* OUT: Pointer to mapping */
){
  DO_OS_MALLOC_TEST(id);
  वापस id->pMethods->xShmMap(id, iPage, pgsz, bExtend, pp);
}
#पूर्ण_अगर /* SQLITE_OMIT_WAL */

#अगर SQLITE_MAX_MMAP_आकार>0
/* The real implementation of xFetch and xUnfetch */
पूर्णांक sqlite3OsFetch(sqlite3_file *id, i64 iOff, पूर्णांक iAmt, व्योम **pp){
  DO_OS_MALLOC_TEST(id);
  वापस id->pMethods->xFetch(id, iOff, iAmt, pp);
}
पूर्णांक sqlite3OsUnfetch(sqlite3_file *id, i64 iOff, व्योम *p){
  वापस id->pMethods->xUnfetch(id, iOff, p);
}
#अन्यथा
/* No-op stubs to use when memory-mapped I/O is disabled */
पूर्णांक sqlite3OsFetch(sqlite3_file *id, i64 iOff, पूर्णांक iAmt, व्योम **pp){
  *pp = 0;
  वापस SQLITE_OK;
}
पूर्णांक sqlite3OsUnfetch(sqlite3_file *id, i64 iOff, व्योम *p){
  वापस SQLITE_OK;
}
#पूर्ण_अगर

/*
** The next group of routines are convenience wrappers around the
** VFS methods.
*/
पूर्णांक sqlite3OsOpen(
  sqlite3_vfs *pVfs,
  स्थिर अक्षर *zPath,
  sqlite3_file *pFile,
  पूर्णांक flags,
  पूर्णांक *pFlagsOut
){
  पूर्णांक rc;
  DO_OS_MALLOC_TEST(0);
  /* 0x87f7f is a mask of SQLITE_OPEN_ flags that are valid to be passed
  ** down into the VFS layer.  Some SQLITE_OPEN_ flags (for example,
  ** SQLITE_OPEN_FULLMUTEX or SQLITE_OPEN_SHAREDCACHE) are blocked before
  ** reaching the VFS. */
  निश्चित( zPath || (flags & SQLITE_OPEN_EXCLUSIVE) );
  rc = pVfs->xOpen(pVfs, zPath, pFile, flags & 0x1087f7f, pFlagsOut);
  निश्चित( rc==SQLITE_OK || pFile->pMethods==0 );
  वापस rc;
}
पूर्णांक sqlite3OsDelete(sqlite3_vfs *pVfs, स्थिर अक्षर *zPath, पूर्णांक dirSync){
  DO_OS_MALLOC_TEST(0);
  निश्चित( dirSync==0 || dirSync==1 );
  वापस pVfs->xDelete!=0 ? pVfs->xDelete(pVfs, zPath, dirSync) : SQLITE_OK;
}
पूर्णांक sqlite3OsAccess(
  sqlite3_vfs *pVfs,
  स्थिर अक्षर *zPath,
  पूर्णांक flags,
  पूर्णांक *pResOut
){
  DO_OS_MALLOC_TEST(0);
  वापस pVfs->xAccess(pVfs, zPath, flags, pResOut);
}
पूर्णांक sqlite3OsFullPathname(
  sqlite3_vfs *pVfs,
  स्थिर अक्षर *zPath,
  पूर्णांक nPathOut,
  अक्षर *zPathOut
){
  DO_OS_MALLOC_TEST(0);
  zPathOut[0] = 0;
  वापस pVfs->xFullPathname(pVfs, zPath, nPathOut, zPathOut);
}
#अगर_अघोषित SQLITE_OMIT_LOAD_EXTENSION
व्योम *sqlite3OsDlOpen(sqlite3_vfs *pVfs, स्थिर अक्षर *zPath){
  निश्चित( zPath!=0 );
  निश्चित( म_माप(zPath)<=SQLITE_MAX_PATHLEN );  /* tag-20210611-1 */
  वापस pVfs->xDlOpen(pVfs, zPath);
}
व्योम sqlite3OsDlError(sqlite3_vfs *pVfs, पूर्णांक nByte, अक्षर *zBufOut){
  pVfs->xDlError(pVfs, nByte, zBufOut);
}
व्योम (*sqlite3OsDlSym(sqlite3_vfs *pVfs, व्योम *pHdle, स्थिर अक्षर *zSym))(व्योम){
  वापस pVfs->xDlSym(pVfs, pHdle, zSym);
}
व्योम sqlite3OsDlClose(sqlite3_vfs *pVfs, व्योम *pHandle){
  pVfs->xDlClose(pVfs, pHandle);
}
#पूर्ण_अगर /* SQLITE_OMIT_LOAD_EXTENSION */
पूर्णांक sqlite3OsRandomness(sqlite3_vfs *pVfs, पूर्णांक nByte, अक्षर *zBufOut){
  अगर( sqlite3Config.iPrngSeed ){
    स_रखो(zBufOut, 0, nByte);
    अगर( ALWAYS(nByte>(चिन्हित)माप(अचिन्हित)) ) nByte = माप(अचिन्हित पूर्णांक);
    स_नकल(zBufOut, &sqlite3Config.iPrngSeed, nByte);
    वापस SQLITE_OK;
  }अन्यथा{
    वापस pVfs->xRandomness(pVfs, nByte, zBufOut);
  }

}
पूर्णांक sqlite3OsSleep(sqlite3_vfs *pVfs, पूर्णांक nMicro){
  वापस pVfs->xSleep(pVfs, nMicro);
}
पूर्णांक sqlite3OsGetLastError(sqlite3_vfs *pVfs){
  वापस pVfs->xGetLastError ? pVfs->xGetLastError(pVfs, 0, 0) : 0;
}
पूर्णांक sqlite3OsCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *pTimeOut){
  पूर्णांक rc;
  /* IMPLEMENTATION-OF: R-49045-42493 SQLite will use the xCurrentTimeInt64()
  ** method to get the current date and time if that method is available
  ** (if iVersion is 2 or greater and the function pointer is not NULL) and
  ** will fall back to xCurrentTime() if xCurrentTimeInt64() is
  ** unavailable.
  */
  अगर( pVfs->iVersion>=2 && pVfs->xCurrentTimeInt64 ){
    rc = pVfs->xCurrentTimeInt64(pVfs, pTimeOut);
  }अन्यथा{
    द्विगुन r;
    rc = pVfs->xCurrentTime(pVfs, &r);
    *pTimeOut = sqlite3RealToI64(r*86400000.0);
  }
  वापस rc;
}

पूर्णांक sqlite3OsOpenMalloc(
  sqlite3_vfs *pVfs,
  स्थिर अक्षर *zFile,
  sqlite3_file **ppFile,
  पूर्णांक flags,
  पूर्णांक *pOutFlags
){
  पूर्णांक rc;
  sqlite3_file *pFile;
  pFile = (sqlite3_file *)sqlite3MallocZero(pVfs->szOsFile);
  अगर( pFile ){
    rc = sqlite3OsOpen(pVfs, zFile, pFile, flags, pOutFlags);
    अगर( rc!=SQLITE_OK ){
      sqlite3_free(pFile);
      *ppFile = 0;
    }अन्यथा{
      *ppFile = pFile;
    }
  }अन्यथा{
    *ppFile = 0;
    rc = SQLITE_NOMEM_BKPT;
  }
  निश्चित( *ppFile!=0 || rc!=SQLITE_OK );
  वापस rc;
}
व्योम sqlite3OsCloseFree(sqlite3_file *pFile){
  निश्चित( pFile );
  sqlite3OsClose(pFile);
  sqlite3_free(pFile);
}

/*
** This function is a wrapper around the OS specific implementation of
** sqlite3_os_init(). The purpose of the wrapper is to provide the
** ability to simulate a malloc failure, so that the handling of an
** error in sqlite3_os_init() by the upper layers can be tested.
*/
पूर्णांक sqlite3OsInit(व्योम){
  व्योम *p = sqlite3_malloc(10);
  अगर( p==0 ) वापस SQLITE_NOMEM_BKPT;
  sqlite3_free(p);
  वापस sqlite3_os_init();
}

/*
** The list of all registered VFS implementations.
*/
अटल sqlite3_vfs * SQLITE_WSD vfsList = 0;
#घोषणा vfsList GLOBAL(sqlite3_vfs *, vfsList)

/*
** Locate a VFS by name.  If no name is given, simply return the
** first VFS on the list.
*/
sqlite3_vfs *sqlite3_vfs_find(स्थिर अक्षर *zVfs){
  sqlite3_vfs *pVfs = 0;
#अगर SQLITE_THREADSAFE
  sqlite3_mutex *mutex;
#पूर्ण_अगर
#अगर_अघोषित SQLITE_OMIT_AUTOINIT
  पूर्णांक rc = sqlite3_initialize();
  अगर( rc ) वापस 0;
#पूर्ण_अगर
#अगर SQLITE_THREADSAFE
  mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
#पूर्ण_अगर
  sqlite3_mutex_enter(mutex);
  क्रम(pVfs = vfsList; pVfs; pVfs=pVfs->p_अग्र){
    अगर( zVfs==0 ) अवरोध;
    अगर( म_भेद(zVfs, pVfs->zName)==0 ) अवरोध;
  }
  sqlite3_mutex_leave(mutex);
  वापस pVfs;
}

/*
** Unlink a VFS from the linked list
*/
अटल व्योम vfsUnlink(sqlite3_vfs *pVfs){
  निश्चित( sqlite3_mutex_held(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN)) );
  अगर( pVfs==0 ){
    /* No-op */
  }अन्यथा अगर( vfsList==pVfs ){
    vfsList = pVfs->p_अग्र;
  }अन्यथा अगर( vfsList ){
    sqlite3_vfs *p = vfsList;
    जबतक( p->p_अग्र && p->p_अग्र!=pVfs ){
      p = p->p_अग्र;
    }
    अगर( p->p_अग्र==pVfs ){
      p->p_अग्र = pVfs->p_अग्र;
    }
  }
}

/*
** Register a VFS with the system.  It is harmless to register the same
** VFS multiple times.  The new VFS becomes the default if makeDflt is
** true.
*/
पूर्णांक sqlite3_vfs_register(sqlite3_vfs *pVfs, पूर्णांक makeDflt){
  MUTEX_LOGIC(sqlite3_mutex *mutex;)
#अगर_अघोषित SQLITE_OMIT_AUTOINIT
  पूर्णांक rc = sqlite3_initialize();
  अगर( rc ) वापस rc;
#पूर्ण_अगर
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( pVfs==0 ) वापस SQLITE_MISUSE_BKPT;
#पूर्ण_अगर

  MUTEX_LOGIC( mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  sqlite3_mutex_enter(mutex);
  vfsUnlink(pVfs);
  अगर( makeDflt || vfsList==0 ){
    pVfs->p_अग्र = vfsList;
    vfsList = pVfs;
  }अन्यथा{
    pVfs->p_अग्र = vfsList->p_अग्र;
    vfsList->p_अग्र = pVfs;
  }
  निश्चित(vfsList);
  sqlite3_mutex_leave(mutex);
  वापस SQLITE_OK;
}

/*
** Unregister a VFS so that it is no longer accessible.
*/
पूर्णांक sqlite3_vfs_unregister(sqlite3_vfs *pVfs){
  MUTEX_LOGIC(sqlite3_mutex *mutex;)
#अगर_अघोषित SQLITE_OMIT_AUTOINIT
  पूर्णांक rc = sqlite3_initialize();
  अगर( rc ) वापस rc;
#पूर्ण_अगर
  MUTEX_LOGIC( mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  sqlite3_mutex_enter(mutex);
  vfsUnlink(pVfs);
  sqlite3_mutex_leave(mutex);
  वापस SQLITE_OK;
}
