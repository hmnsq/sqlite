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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"

/*
** If we compile with the SQLITE_TEST macro set, then the following block
** of code will give us the ability to simulate a disk I/O error.  This
** is used for testing the I/O recovery logic.
*/
#ਜੇ defined(SQLITE_TEST)
ਪੂਰਨ sqlite3_io_error_ਪ੍ਰਾਪਤੀ = 0;            /* Total number of I/O Errors */
ਪੂਰਨ sqlite3_io_error_hardhit = 0;        /* Number of non-benign errors */
ਪੂਰਨ sqlite3_io_error_pending = 0;        /* Count down to first I/O error */
ਪੂਰਨ sqlite3_io_error_persist = 0;        /* True if I/O errors persist */
ਪੂਰਨ sqlite3_io_error_benign = 0;         /* True if errors are benign */
ਪੂਰਨ sqlite3_diskfull_pending = 0;
ਪੂਰਨ sqlite3_diskfull = 0;
#ਜੇ_ਅੰਤ /* defined(SQLITE_TEST) */

/*
** When testing, also keep a count of the number of open files.
*/
#ਜੇ defined(SQLITE_TEST)
ਪੂਰਨ sqlite3_open_file_ਗਿਣਤੀ = 0;
#ਜੇ_ਅੰਤ /* defined(SQLITE_TEST) */

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
#ਜੇ defined(SQLITE_TEST)
ਪੂਰਨ sqlite3_memdebug_vfs_oom_test = 1;
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DO_OS_MALLOC_TEST(x)                                       \
  ਜੇ (sqlite3_memdebug_vfs_oom_test && (!x || !sqlite3_Journal_Is_ਵਿੱਚ_ਸਮ੍ਰਿਤੀ(x))) { \
    ਖਾਲੀ *pTstAlloc = sqlite3Malloc(10);                             \
    ਜੇ (!pTstAlloc) ਵਾਪਸ SQLITE_IOERR_NOMEM_BKPT;                  \
    sqlite3_free(pTstAlloc);                                         \
  }
#ਨਹੀਂਤਾਂ
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DO_OS_MALLOC_TEST(x)
#ਜੇ_ਅੰਤ

/*
** The following routines are convenience wrappers around methods
** of the sqlite3_file object.  This is mostly just syntactic sugar. All
** of this would be completely automatic if SQLite were coded using
** C++ instead of plain old C.
*/
ਖਾਲੀ sqlite3OsClose(sqlite3_file *pId){
  ਜੇ( pId->pMethods ){
    pId->pMethods->xClose(pId);
    pId->pMethods = 0;
  }
}
ਪੂਰਨ sqlite3OsRead(sqlite3_file *id, ਖਾਲੀ *pBuf, ਪੂਰਨ amt, i64 ਵਿੱਥ){
  DO_OS_MALLOC_TEST(id);
  ਵਾਪਸ id->pMethods->xRead(id, pBuf, amt, ਵਿੱਥ);
}
ਪੂਰਨ sqlite3OsWrite(sqlite3_file *id, ਅਚਲ ਖਾਲੀ *pBuf, ਪੂਰਨ amt, i64 ਵਿੱਥ){
  DO_OS_MALLOC_TEST(id);
  ਵਾਪਸ id->pMethods->xWrite(id, pBuf, amt, ਵਿੱਥ);
}
ਪੂਰਨ sqlite3OsTruncate(sqlite3_file *id, i64 ਮਾਪ){
  ਵਾਪਸ id->pMethods->xTruncate(id, ਮਾਪ);
}
ਪੂਰਨ sqlite3OsSync(sqlite3_file *id, ਪੂਰਨ flags){
  DO_OS_MALLOC_TEST(id);
  ਵਾਪਸ flags ? id->pMethods->xSync(id, flags) : SQLITE_OK;
}
ਪੂਰਨ sqlite3_Os_File_ਮਾਪ(sqlite3_file *id, i64 *p_ਮਾਪ){
  DO_OS_MALLOC_TEST(id);
  ਵਾਪਸ id->pMethods->x_File_ਮਾਪ(id, p_ਮਾਪ);
}
ਪੂਰਨ sqlite3_Os_ਤਾਲਾ(sqlite3_file *id, ਪੂਰਨ ਤਾਲਾ_Type){
  DO_OS_MALLOC_TEST(id);
  ਪ੍ਰਤਿਜ੍ਞਾ( ਤਾਲਾ_Type>=SQLITE_ਤਾਲਾ_SHARED && ਤਾਲਾ_Type<=SQLITE_ਤਾਲਾ_EXCLUSIVE );
  ਵਾਪਸ id->pMethods->x_ਤਾਲਾ(id, ਤਾਲਾ_Type);
}
ਪੂਰਨ sqlite3OsUnlock(sqlite3_file *id, ਪੂਰਨ ਤਾਲਾ_Type){
  ਪ੍ਰਤਿਜ੍ਞਾ( ਤਾਲਾ_Type==SQLITE_ਤਾਲਾ_NONE || ਤਾਲਾ_Type==SQLITE_ਤਾਲਾ_SHARED );
  ਵਾਪਸ id->pMethods->xUnlock(id, ਤਾਲਾ_Type);
}
ਪੂਰਨ sqlite3_Os_Check_Reserved_ਤਾਲਾ(sqlite3_file *id, ਪੂਰਨ *p_Res_ਬਾਹਰਲਾ){
  DO_OS_MALLOC_TEST(id);
  ਵਾਪਸ id->pMethods->x_Check_Reserved_ਤਾਲਾ(id, p_Res_ਬਾਹਰਲਾ);
}

/*
** Use sqlite3OsFileControl() when we are doing something that might fail
** and we need to know about the failures.  Use sqlite3OsFileControlHint()
** when simply tossing information over the wall to the VFS and we do not
** really care if the VFS receives and understands the information since it
** is only a hint and can be safely ignored.  The sqlite3OsFileControlHint()
** routine has no return value since the return value would be meaningless.
*/
ਪੂਰਨ sqlite3OsFileControl(sqlite3_file *id, ਪੂਰਨ op, ਖਾਲੀ *pArg){
  ਜੇ( id->pMethods==0 ) ਵਾਪਸ SQLITE_NOTFOUND;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_TEST
  ਜੇ( op!=SQLITE_FCNTL_COMMIT_PHASETWO
   && op!=SQLITE_FCNTL_ਤਾਲਾ_TIMEOUT
   && op!=SQLITE_FCNTL_CKPT_ਪੂਰਾ
   && op!=SQLITE_FCNTL_CKPT_ਸ਼ੁਰੂ
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
#ਜੇ_ਅੰਤ
  ਵਾਪਸ id->pMethods->xFileControl(id, op, pArg);
}
ਖਾਲੀ sqlite3OsFileControlHint(sqlite3_file *id, ਪੂਰਨ op, ਖਾਲੀ *pArg){
  ਜੇ( id->pMethods ) (ਖਾਲੀ)id->pMethods->xFileControl(id, op, pArg);
}

ਪੂਰਨ sqlite3_Os_Sector_ਮਾਪ(sqlite3_file *id){
  ਪੂਰਨ (*x_Sector_ਮਾਪ)(sqlite3_file*) = id->pMethods->x_Sector_ਮਾਪ;
  ਵਾਪਸ (x_Sector_ਮਾਪ ? x_Sector_ਮਾਪ(id) : SQLITE_DEFAULT_SECTOR_ਮਾਪ);
}
ਪੂਰਨ sqlite3OsDeviceCharacteristics(sqlite3_file *id){
  ਜੇ( NEVER(id->pMethods==0) ) ਵਾਪਸ 0;
  ਵਾਪਸ id->pMethods->xDeviceCharacteristics(id);
}
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WAL
ਪੂਰਨ sqlite3_Os_Shm_ਤਾਲਾ(sqlite3_file *id, ਪੂਰਨ ਵਿੱਥ, ਪੂਰਨ n, ਪੂਰਨ flags){
  ਵਾਪਸ id->pMethods->x_Shm_ਤਾਲਾ(id, ਵਿੱਥ, n, flags);
}
ਖਾਲੀ sqlite3OsShmBarrier(sqlite3_file *id){
  id->pMethods->xShmBarrier(id);
}
ਪੂਰਨ sqlite3OsShmUnmap(sqlite3_file *id, ਪੂਰਨ deleteFlag){
  ਵਾਪਸ id->pMethods->xShmUnmap(id, deleteFlag);
}
ਪੂਰਨ sqlite3OsShmMap(
  sqlite3_file *id,               /* Database file handle */
  ਪੂਰਨ i_ਪੰਨਾ,
  ਪੂਰਨ pgsz,
  ਪੂਰਨ bExtend,                    /* True to extend file if necessary */
  ਖਾਲੀ ਅਸਥਿਰ **pp              /* OUT: Pointer to mapping */
){
  DO_OS_MALLOC_TEST(id);
  ਵਾਪਸ id->pMethods->xShmMap(id, i_ਪੰਨਾ, pgsz, bExtend, pp);
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_WAL */

#ਜੇ SQLITE_MAX_MMAP_ਮਾਪ>0
/* The real implementation of xFetch and xUnfetch */
ਪੂਰਨ sqlite3OsFetch(sqlite3_file *id, i64 iOff, ਪੂਰਨ iAmt, ਖਾਲੀ **pp){
  DO_OS_MALLOC_TEST(id);
  ਵਾਪਸ id->pMethods->xFetch(id, iOff, iAmt, pp);
}
ਪੂਰਨ sqlite3OsUnfetch(sqlite3_file *id, i64 iOff, ਖਾਲੀ *p){
  ਵਾਪਸ id->pMethods->xUnfetch(id, iOff, p);
}
#ਨਹੀਂਤਾਂ
/* No-op stubs to use when memory-mapped I/O is disabled */
ਪੂਰਨ sqlite3OsFetch(sqlite3_file *id, i64 iOff, ਪੂਰਨ iAmt, ਖਾਲੀ **pp){
  *pp = 0;
  ਵਾਪਸ SQLITE_OK;
}
ਪੂਰਨ sqlite3OsUnfetch(sqlite3_file *id, i64 iOff, ਖਾਲੀ *p){
  ਵਾਪਸ SQLITE_OK;
}
#ਜੇ_ਅੰਤ

/*
** The next group of routines are convenience wrappers around the
** VFS methods.
*/
ਪੂਰਨ sqlite3OsOpen(
  sqlite3_vfs *pVfs,
  ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ,
  sqlite3_file *pFile,
  ਪੂਰਨ flags,
  ਪੂਰਨ *p_Flags_ਬਾਹਰਲਾ
){
  ਪੂਰਨ rc;
  DO_OS_MALLOC_TEST(0);
  /* 0x87f7f is a mask of SQLITE_OPEN_ flags that are valid to be passed
  ** down into the VFS layer.  Some SQLITE_OPEN_ flags (for example,
  ** SQLITE_OPEN_FULLMUTEX or SQLITE_OPEN_SHAREDCACHE) are blocked before
  ** reaching the VFS. */
  ਪ੍ਰਤਿਜ੍ਞਾ( z_ਰਸਤਾ || (flags & SQLITE_OPEN_EXCLUSIVE) );
  rc = pVfs->xOpen(pVfs, z_ਰਸਤਾ, pFile, flags & 0x1087f7f, p_Flags_ਬਾਹਰਲਾ);
  ਪ੍ਰਤਿਜ੍ਞਾ( rc==SQLITE_OK || pFile->pMethods==0 );
  ਵਾਪਸ rc;
}
ਪੂਰਨ sqlite3OsDelete(sqlite3_vfs *pVfs, ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ, ਪੂਰਨ dirSync){
  DO_OS_MALLOC_TEST(0);
  ਪ੍ਰਤਿਜ੍ਞਾ( dirSync==0 || dirSync==1 );
  ਵਾਪਸ pVfs->xDelete!=0 ? pVfs->xDelete(pVfs, z_ਰਸਤਾ, dirSync) : SQLITE_OK;
}
ਪੂਰਨ sqlite3OsAccess(
  sqlite3_vfs *pVfs,
  ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ,
  ਪੂਰਨ flags,
  ਪੂਰਨ *p_Res_ਬਾਹਰਲਾ
){
  DO_OS_MALLOC_TEST(0);
  ਵਾਪਸ pVfs->xAccess(pVfs, z_ਰਸਤਾ, flags, p_Res_ਬਾਹਰਲਾ);
}
ਪੂਰਨ sqlite3_Os_ਭਰਿਆ_Pathname(
  sqlite3_vfs *pVfs,
  ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ,
  ਪੂਰਨ n_ਰਸਤਾ_ਬਾਹਰਲਾ,
  ਅੱਖਰ *z_ਰਸਤਾ_ਬਾਹਰਲਾ
){
  DO_OS_MALLOC_TEST(0);
  z_ਰਸਤਾ_ਬਾਹਰਲਾ[0] = 0;
  ਵਾਪਸ pVfs->x_ਭਰਿਆ_Pathname(pVfs, z_ਰਸਤਾ, n_ਰਸਤਾ_ਬਾਹਰਲਾ, z_ਰਸਤਾ_ਬਾਹਰਲਾ);
}
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_ਲੱਦੋ_EXTENSION
ਖਾਲੀ *sqlite3OsDlOpen(sqlite3_vfs *pVfs, ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ){
  ਪ੍ਰਤਿਜ੍ਞਾ( z_ਰਸਤਾ!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( ਲੜੀ_ਲੰਬਾਈ(z_ਰਸਤਾ)<=SQLITE_MAX_PATHLEN );  /* tag-20210611-1 */
  ਵਾਪਸ pVfs->xDlOpen(pVfs, z_ਰਸਤਾ);
}
ਖਾਲੀ sqlite3OsDlError(sqlite3_vfs *pVfs, ਪੂਰਨ nByte, ਅੱਖਰ *z_Buf_ਬਾਹਰਲਾ){
  pVfs->xDlError(pVfs, nByte, z_Buf_ਬਾਹਰਲਾ);
}
ਖਾਲੀ (*sqlite3OsDlSym(sqlite3_vfs *pVfs, ਖਾਲੀ *pHdle, ਅਚਲ ਅੱਖਰ *zSym))(ਖਾਲੀ){
  ਵਾਪਸ pVfs->xDlSym(pVfs, pHdle, zSym);
}
ਖਾਲੀ sqlite3OsDlClose(sqlite3_vfs *pVfs, ਖਾਲੀ *pHandle){
  pVfs->xDlClose(pVfs, pHandle);
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_LOAD_EXTENSION */
ਪੂਰਨ sqlite3OsRandomness(sqlite3_vfs *pVfs, ਪੂਰਨ nByte, ਅੱਖਰ *z_Buf_ਬਾਹਰਲਾ){
  ਜੇ( sqlite3Config.i_Prng_ਬੀਜ ){
    memset(z_Buf_ਬਾਹਰਲਾ, 0, nByte);
    ਜੇ( ALWAYS(nByte>(ਨਿਸ਼ਾਨੀਵਾਲਾ)ਆਕਾਰ(ਬਿਨਾਨਿਸ਼ਾਨੀ)) ) nByte = ਆਕਾਰ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ);
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(z_Buf_ਬਾਹਰਲਾ, &sqlite3Config.i_Prng_ਬੀਜ, nByte);
    ਵਾਪਸ SQLITE_OK;
  }ਨਹੀਂਤਾਂ{
    ਵਾਪਸ pVfs->xRandomness(pVfs, nByte, z_Buf_ਬਾਹਰਲਾ);
  }

}
ਪੂਰਨ sqlite3OsSleep(sqlite3_vfs *pVfs, ਪੂਰਨ nMicro){
  ਵਾਪਸ pVfs->xSleep(pVfs, nMicro);
}
ਪੂਰਨ sqlite3_Os_Get_ਆਖਰੀ_Error(sqlite3_vfs *pVfs){
  ਵਾਪਸ pVfs->x_Get_ਆਖਰੀ_Error ? pVfs->x_Get_ਆਖਰੀ_Error(pVfs, 0, 0) : 0;
}
ਪੂਰਨ sqlite3_Os_Current_ਸਮਾਂ_Int64(sqlite3_vfs *pVfs, sqlite3_int64 *p_ਸਮਾਂ_ਬਾਹਰਲਾ){
  ਪੂਰਨ rc;
  /* IMPLEMENTATION-OF: R-49045-42493 SQLite will use the xCurrentTimeInt64()
  ** method to get the current date and time if that method is available
  ** (if iVersion is 2 or greater and the function pointer is not NULL) and
  ** will fall back to xCurrentTime() if xCurrentTimeInt64() is
  ** unavailable.
  */
  ਜੇ( pVfs->i_ਸੰਸਕਰਣ>=2 && pVfs->x_Current_ਸਮਾਂ_Int64 ){
    rc = pVfs->x_Current_ਸਮਾਂ_Int64(pVfs, p_ਸਮਾਂ_ਬਾਹਰਲਾ);
  }ਨਹੀਂਤਾਂ{
    ਦੁੱਗਣਾ r;
    rc = pVfs->x_Current_ਸਮਾਂ(pVfs, &r);
    *p_ਸਮਾਂ_ਬਾਹਰਲਾ = sqlite3_Real_ਮੰਜ਼ਿਲ_I64(r*86400000.0);
  }
  ਵਾਪਸ rc;
}

ਪੂਰਨ sqlite3OsOpenMalloc(
  sqlite3_vfs *pVfs,
  ਅਚਲ ਅੱਖਰ *zFile,
  sqlite3_file **ppFile,
  ਪੂਰਨ flags,
  ਪੂਰਨ *p_ਬਾਹਰਲਾ_Flags
){
  ਪੂਰਨ rc;
  sqlite3_file *pFile;
  pFile = (sqlite3_file *)sqlite3MallocZero(pVfs->szOsFile);
  ਜੇ( pFile ){
    rc = sqlite3OsOpen(pVfs, zFile, pFile, flags, p_ਬਾਹਰਲਾ_Flags);
    ਜੇ( rc!=SQLITE_OK ){
      sqlite3_free(pFile);
      *ppFile = 0;
    }ਨਹੀਂਤਾਂ{
      *ppFile = pFile;
    }
  }ਨਹੀਂਤਾਂ{
    *ppFile = 0;
    rc = SQLITE_NOMEM_BKPT;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( *ppFile!=0 || rc!=SQLITE_OK );
  ਵਾਪਸ rc;
}
ਖਾਲੀ sqlite3OsCloseFree(sqlite3_file *pFile){
  ਪ੍ਰਤਿਜ੍ਞਾ( pFile );
  sqlite3OsClose(pFile);
  sqlite3_free(pFile);
}

/*
** This function is a wrapper around the OS specific implementation of
** sqlite3_os_init(). The purpose of the wrapper is to provide the
** ability to simulate a malloc failure, so that the handling of an
** error in sqlite3_os_init() by the upper layers can be tested.
*/
ਪੂਰਨ sqlite3OsInit(ਖਾਲੀ){
  ਖਾਲੀ *p = sqlite3_malloc(10);
  ਜੇ( p==0 ) ਵਾਪਸ SQLITE_NOMEM_BKPT;
  sqlite3_free(p);
  ਵਾਪਸ sqlite3_os_init();
}

/*
** The list of all registered VFS implementations.
*/
ਸਥਿਰ sqlite3_vfs * SQLITE_WSD vfsList = 0;
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ vfsList GLOBAL(sqlite3_vfs *, vfsList)

/*
** Locate a VFS by name.  If no name is given, simply return the
** first VFS on the list.
*/
sqlite3_vfs *sqlite3_vfs_ਲੱਭੋ(ਅਚਲ ਅੱਖਰ *zVfs){
  sqlite3_vfs *pVfs = 0;
#ਜੇ SQLITE_THREADSAFE
  sqlite3_mutex *mutex;
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOINIT
  ਪੂਰਨ rc = sqlite3_initialize();
  ਜੇ( rc ) ਵਾਪਸ 0;
#ਜੇ_ਅੰਤ
#ਜੇ SQLITE_THREADSAFE
  mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
#ਜੇ_ਅੰਤ
  sqlite3_mutex_enter(mutex);
  ਲਈ(pVfs = vfsList; pVfs; pVfs=pVfs->p_ਅਗਲਾ){
    ਜੇ( zVfs==0 ) ਰੋਕੋ;
    ਜੇ( strcmp(zVfs, pVfs->zName)==0 ) ਰੋਕੋ;
  }
  sqlite3_mutex_leave(mutex);
  ਵਾਪਸ pVfs;
}

/*
** Unlink a VFS from the linked list
*/
ਸਥਿਰ ਖਾਲੀ vfsUnlink(sqlite3_vfs *pVfs){
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN)) );
  ਜੇ( pVfs==0 ){
    /* No-op */
  }ਨਹੀਂਤਾਂ ਜੇ( vfsList==pVfs ){
    vfsList = pVfs->p_ਅਗਲਾ;
  }ਨਹੀਂਤਾਂ ਜੇ( vfsList ){
    sqlite3_vfs *p = vfsList;
    ਜਦੋਂ( p->p_ਅਗਲਾ && p->p_ਅਗਲਾ!=pVfs ){
      p = p->p_ਅਗਲਾ;
    }
    ਜੇ( p->p_ਅਗਲਾ==pVfs ){
      p->p_ਅਗਲਾ = pVfs->p_ਅਗਲਾ;
    }
  }
}

/*
** Register a VFS with the system.  It is harmless to register the same
** VFS multiple times.  The new VFS becomes the default if makeDflt is
** true.
*/
ਪੂਰਨ sqlite3_vfs_ਰਜਿਸਟਰ(sqlite3_vfs *pVfs, ਪੂਰਨ makeDflt){
  MUTEX_LOGIC(sqlite3_mutex *mutex;)
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOINIT
  ਪੂਰਨ rc = sqlite3_initialize();
  ਜੇ( rc ) ਵਾਪਸ rc;
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( pVfs==0 ) ਵਾਪਸ SQLITE_MISUSE_BKPT;
#ਜੇ_ਅੰਤ

  MUTEX_LOGIC( mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  sqlite3_mutex_enter(mutex);
  vfsUnlink(pVfs);
  ਜੇ( makeDflt || vfsList==0 ){
    pVfs->p_ਅਗਲਾ = vfsList;
    vfsList = pVfs;
  }ਨਹੀਂਤਾਂ{
    pVfs->p_ਅਗਲਾ = vfsList->p_ਅਗਲਾ;
    vfsList->p_ਅਗਲਾ = pVfs;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ(vfsList);
  sqlite3_mutex_leave(mutex);
  ਵਾਪਸ SQLITE_OK;
}

/*
** Unregister a VFS so that it is no longer accessible.
*/
ਪੂਰਨ sqlite3_vfs_unregister(sqlite3_vfs *pVfs){
  MUTEX_LOGIC(sqlite3_mutex *mutex;)
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOINIT
  ਪੂਰਨ rc = sqlite3_initialize();
  ਜੇ( rc ) ਵਾਪਸ rc;
#ਜੇ_ਅੰਤ
  MUTEX_LOGIC( mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  sqlite3_mutex_enter(mutex);
  vfsUnlink(pVfs);
  sqlite3_mutex_leave(mutex);
  ਵਾਪਸ SQLITE_OK;
}
