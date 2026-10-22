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
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"

/*
** If we compile with the SQLITE_TEST macro set, then the following block
** of code will give us the ability to simulate a disk I/O error.  This
** is used for testing the I/O recovery logic.
*/
#ಆದರೆ defined(SQLITE_TEST)
ಪೂರ್ಣಾಂಕ sqlite3_io_error_ದೊರಕಿತು = 0;            /* Total number of I/O Errors */
ಪೂರ್ಣಾಂಕ sqlite3_io_error_hardhit = 0;        /* Number of non-benign errors */
ಪೂರ್ಣಾಂಕ sqlite3_io_error_pending = 0;        /* Count down to first I/O error */
ಪೂರ್ಣಾಂಕ sqlite3_io_error_persist = 0;        /* True if I/O errors persist */
ಪೂರ್ಣಾಂಕ sqlite3_io_error_benign = 0;         /* True if errors are benign */
ಪೂರ್ಣಾಂಕ sqlite3_diskfull_pending = 0;
ಪೂರ್ಣಾಂಕ sqlite3_diskfull = 0;
#ಆದರೆ_ಅಂತ್ಯ /* defined(SQLITE_TEST) */

/*
** When testing, also keep a count of the number of open files.
*/
#ಆದರೆ defined(SQLITE_TEST)
ಪೂರ್ಣಾಂಕ sqlite3_open_file_ಎಣಿಕೆ = 0;
#ಆದರೆ_ಅಂತ್ಯ /* defined(SQLITE_TEST) */

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
#ಆದರೆ defined(SQLITE_TEST)
ಪೂರ್ಣಾಂಕ sqlite3_memdebug_vfs_oom_test = 1;
  #ವ್ಯಾಖ್ಯಾನಿಸು DO_OS_MALLOC_TEST(x)                                       \
  ಆದರೆ (sqlite3_memdebug_vfs_oom_test && (!x || !sqlite3_Journal_Is_ಒಳ_ಸ್ಮರಣೆ(x))) { \
    ಶೂನ್ಯ *pTstAlloc = sqlite3Malloc(10);                             \
    ಆದರೆ (!pTstAlloc) ಹಿಂತಿರುಗಿ SQLITE_IOERR_NOMEM_BKPT;                  \
    sqlite3_free(pTstAlloc);                                         \
  }
#ಇಲ್ಲದಿದ್ದರೆ
  #ವ್ಯಾಖ್ಯಾನಿಸು DO_OS_MALLOC_TEST(x)
#ಆದರೆ_ಅಂತ್ಯ

/*
** The following routines are convenience wrappers around methods
** of the sqlite3_file object.  This is mostly just syntactic sugar. All
** of this would be completely automatic if SQLite were coded using
** C++ instead of plain old C.
*/
ಶೂನ್ಯ sqlite3OsClose(sqlite3_file *pId){
  ಆದರೆ( pId->pMethods ){
    pId->pMethods->xClose(pId);
    pId->pMethods = 0;
  }
}
ಪೂರ್ಣಾಂಕ sqlite3OsRead(sqlite3_file *id, ಶೂನ್ಯ *pBuf, ಪೂರ್ಣಾಂಕ amt, i64 ಸ್ಥಾನಾಂತರ){
  DO_OS_MALLOC_TEST(id);
  ಹಿಂತಿರುಗಿ id->pMethods->xRead(id, pBuf, amt, ಸ್ಥಾನಾಂತರ);
}
ಪೂರ್ಣಾಂಕ sqlite3OsWrite(sqlite3_file *id, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *pBuf, ಪೂರ್ಣಾಂಕ amt, i64 ಸ್ಥಾನಾಂತರ){
  DO_OS_MALLOC_TEST(id);
  ಹಿಂತಿರುಗಿ id->pMethods->xWrite(id, pBuf, amt, ಸ್ಥಾನಾಂತರ);
}
ಪೂರ್ಣಾಂಕ sqlite3OsTruncate(sqlite3_file *id, i64 ಗಾತ್ರ_2){
  ಹಿಂತಿರುಗಿ id->pMethods->xTruncate(id, ಗಾತ್ರ_2);
}
ಪೂರ್ಣಾಂಕ sqlite3OsSync(sqlite3_file *id, ಪೂರ್ಣಾಂಕ flags){
  DO_OS_MALLOC_TEST(id);
  ಹಿಂತಿರುಗಿ flags ? id->pMethods->xSync(id, flags) : SQLITE_OK;
}
ಪೂರ್ಣಾಂಕ sqlite3_Os_File_ಗಾತ್ರ(sqlite3_file *id, i64 *p_ಗಾತ್ರ){
  DO_OS_MALLOC_TEST(id);
  ಹಿಂತಿರುಗಿ id->pMethods->x_File_ಗಾತ್ರ(id, p_ಗಾತ್ರ);
}
ಪೂರ್ಣಾಂಕ sqlite3_Os_ಬೀಗ(sqlite3_file *id, ಪೂರ್ಣಾಂಕ ಬೀಗ_Type){
  DO_OS_MALLOC_TEST(id);
  ಪ್ರತಿಜ್ಞಾ( ಬೀಗ_Type>=SQLITE_ಬೀಗ_SHARED && ಬೀಗ_Type<=SQLITE_ಬೀಗ_EXCLUSIVE );
  ಹಿಂತಿರುಗಿ id->pMethods->x_ಬೀಗ(id, ಬೀಗ_Type);
}
ಪೂರ್ಣಾಂಕ sqlite3OsUnlock(sqlite3_file *id, ಪೂರ್ಣಾಂಕ ಬೀಗ_Type){
  ಪ್ರತಿಜ್ಞಾ( ಬೀಗ_Type==SQLITE_ಬೀಗ_NONE || ಬೀಗ_Type==SQLITE_ಬೀಗ_SHARED );
  ಹಿಂತಿರುಗಿ id->pMethods->xUnlock(id, ಬೀಗ_Type);
}
ಪೂರ್ಣಾಂಕ sqlite3_Os_Check_Reserved_ಬೀಗ(sqlite3_file *id, ಪೂರ್ಣಾಂಕ *p_Res_ಹೊರಹೋಗು){
  DO_OS_MALLOC_TEST(id);
  ಹಿಂತಿರುಗಿ id->pMethods->x_Check_Reserved_ಬೀಗ(id, p_Res_ಹೊರಹೋಗು);
}

/*
** Use sqlite3OsFileControl() when we are doing something that might fail
** and we need to know about the failures.  Use sqlite3OsFileControlHint()
** when simply tossing information over the wall to the VFS and we do not
** really care if the VFS receives and understands the information since it
** is only a hint and can be safely ignored.  The sqlite3OsFileControlHint()
** routine has no return value since the return value would be meaningless.
*/
ಪೂರ್ಣಾಂಕ sqlite3OsFileControl(sqlite3_file *id, ಪೂರ್ಣಾಂಕ op, ಶೂನ್ಯ *pArg){
  ಆದರೆ( id->pMethods==0 ) ಹಿಂತಿರುಗಿ SQLITE_NOTFOUND;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_TEST
  ಆದರೆ( op!=SQLITE_FCNTL_COMMIT_PHASETWO
   && op!=SQLITE_FCNTL_ಬೀಗ_TIMEOUT
   && op!=SQLITE_FCNTL_CKPT_ಮುಗಿದ
   && op!=SQLITE_FCNTL_CKPT_ಆರಂಭ
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
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ id->pMethods->xFileControl(id, op, pArg);
}
ಶೂನ್ಯ sqlite3OsFileControlHint(sqlite3_file *id, ಪೂರ್ಣಾಂಕ op, ಶೂನ್ಯ *pArg){
  ಆದರೆ( id->pMethods ) (ಶೂನ್ಯ)id->pMethods->xFileControl(id, op, pArg);
}

ಪೂರ್ಣಾಂಕ sqlite3_Os_Sector_ಗಾತ್ರ(sqlite3_file *id){
  ಪೂರ್ಣಾಂಕ (*x_Sector_ಗಾತ್ರ)(sqlite3_file*) = id->pMethods->x_Sector_ಗಾತ್ರ;
  ಹಿಂತಿರುಗಿ (x_Sector_ಗಾತ್ರ ? x_Sector_ಗಾತ್ರ(id) : SQLITE_DEFAULT_SECTOR_ಗಾತ್ರ);
}
ಪೂರ್ಣಾಂಕ sqlite3OsDeviceCharacteristics(sqlite3_file *id){
  ಆದರೆ( NEVER(id->pMethods==0) ) ಹಿಂತಿರುಗಿ 0;
  ಹಿಂತಿರುಗಿ id->pMethods->xDeviceCharacteristics(id);
}
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WAL
ಪೂರ್ಣಾಂಕ sqlite3_Os_Shm_ಬೀಗ(sqlite3_file *id, ಪೂರ್ಣಾಂಕ ಸ್ಥಾನಾಂತರ, ಪೂರ್ಣಾಂಕ n, ಪೂರ್ಣಾಂಕ flags){
  ಹಿಂತಿರುಗಿ id->pMethods->x_Shm_ಬೀಗ(id, ಸ್ಥಾನಾಂತರ, n, flags);
}
ಶೂನ್ಯ sqlite3OsShmBarrier(sqlite3_file *id){
  id->pMethods->xShmBarrier(id);
}
ಪೂರ್ಣಾಂಕ sqlite3OsShmUnmap(sqlite3_file *id, ಪೂರ್ಣಾಂಕ deleteFlag){
  ಹಿಂತಿರುಗಿ id->pMethods->xShmUnmap(id, deleteFlag);
}
ಪೂರ್ಣಾಂಕ sqlite3OsShmMap(
  sqlite3_file *id,               /* Database file handle */
  ಪೂರ್ಣಾಂಕ i_ಪುಟ,
  ಪೂರ್ಣಾಂಕ pgsz,
  ಪೂರ್ಣಾಂಕ bExtend,                    /* True to extend file if necessary */
  ಶೂನ್ಯ ಅಸ್ಥಿರ **pp              /* OUT: Pointer to mapping */
){
  DO_OS_MALLOC_TEST(id);
  ಹಿಂತಿರುಗಿ id->pMethods->xShmMap(id, i_ಪುಟ, pgsz, bExtend, pp);
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_WAL */

#ಆದರೆ SQLITE_MAX_MMAP_ಗಾತ್ರ>0
/* The real implementation of xFetch and xUnfetch */
ಪೂರ್ಣಾಂಕ sqlite3OsFetch(sqlite3_file *id, i64 iOff, ಪೂರ್ಣಾಂಕ iAmt, ಶೂನ್ಯ **pp){
  DO_OS_MALLOC_TEST(id);
  ಹಿಂತಿರುಗಿ id->pMethods->xFetch(id, iOff, iAmt, pp);
}
ಪೂರ್ಣಾಂಕ sqlite3OsUnfetch(sqlite3_file *id, i64 iOff, ಶೂನ್ಯ *p){
  ಹಿಂತಿರುಗಿ id->pMethods->xUnfetch(id, iOff, p);
}
#ಇಲ್ಲದಿದ್ದರೆ
/* No-op stubs to use when memory-mapped I/O is disabled */
ಪೂರ್ಣಾಂಕ sqlite3OsFetch(sqlite3_file *id, i64 iOff, ಪೂರ್ಣಾಂಕ iAmt, ಶೂನ್ಯ **pp){
  *pp = 0;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}
ಪೂರ್ಣಾಂಕ sqlite3OsUnfetch(sqlite3_file *id, i64 iOff, ಶೂನ್ಯ *p){
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}
#ಆದರೆ_ಅಂತ್ಯ

/*
** The next group of routines are convenience wrappers around the
** VFS methods.
*/
ಪೂರ್ಣಾಂಕ sqlite3OsOpen(
  sqlite3_vfs *pVfs,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮಾರ್ಗ,
  sqlite3_file *pFile,
  ಪೂರ್ಣಾಂಕ flags,
  ಪೂರ್ಣಾಂಕ *p_Flags_ಹೊರಹೋಗು
){
  ಪೂರ್ಣಾಂಕ rc;
  DO_OS_MALLOC_TEST(0);
  /* 0x87f7f is a mask of SQLITE_OPEN_ flags that are valid to be passed
  ** down into the VFS layer.  Some SQLITE_OPEN_ flags (for example,
  ** SQLITE_OPEN_FULLMUTEX or SQLITE_OPEN_SHAREDCACHE) are blocked before
  ** reaching the VFS. */
  ಪ್ರತಿಜ್ಞಾ( z_ಮಾರ್ಗ || (flags & SQLITE_OPEN_EXCLUSIVE) );
  rc = pVfs->xOpen(pVfs, z_ಮಾರ್ಗ, pFile, flags & 0x1087f7f, p_Flags_ಹೊರಹೋಗು);
  ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_OK || pFile->pMethods==0 );
  ಹಿಂತಿರುಗಿ rc;
}
ಪೂರ್ಣಾಂಕ sqlite3OsDelete(sqlite3_vfs *pVfs, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮಾರ್ಗ, ಪೂರ್ಣಾಂಕ dirSync){
  DO_OS_MALLOC_TEST(0);
  ಪ್ರತಿಜ್ಞಾ( dirSync==0 || dirSync==1 );
  ಹಿಂತಿರುಗಿ pVfs->xDelete!=0 ? pVfs->xDelete(pVfs, z_ಮಾರ್ಗ, dirSync) : SQLITE_OK;
}
ಪೂರ್ಣಾಂಕ sqlite3OsAccess(
  sqlite3_vfs *pVfs,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮಾರ್ಗ,
  ಪೂರ್ಣಾಂಕ flags,
  ಪೂರ್ಣಾಂಕ *p_Res_ಹೊರಹೋಗು
){
  DO_OS_MALLOC_TEST(0);
  ಹಿಂತಿರುಗಿ pVfs->xAccess(pVfs, z_ಮಾರ್ಗ, flags, p_Res_ಹೊರಹೋಗು);
}
ಪೂರ್ಣಾಂಕ sqlite3_Os_ತುಂಬಿದ_Pathname(
  sqlite3_vfs *pVfs,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮಾರ್ಗ,
  ಪೂರ್ಣಾಂಕ n_ಮಾರ್ಗ_ಹೊರಹೋಗು,
  ಅಕ್ಷರ *z_ಮಾರ್ಗ_ಹೊರಹೋಗು
){
  DO_OS_MALLOC_TEST(0);
  z_ಮಾರ್ಗ_ಹೊರಹೋಗು[0] = 0;
  ಹಿಂತಿರುಗಿ pVfs->x_ತುಂಬಿದ_Pathname(pVfs, z_ಮಾರ್ಗ, n_ಮಾರ್ಗ_ಹೊರಹೋಗು, z_ಮಾರ್ಗ_ಹೊರಹೋಗು);
}
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_ತುಂಬು_EXTENSION
ಶೂನ್ಯ *sqlite3OsDlOpen(sqlite3_vfs *pVfs, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮಾರ್ಗ){
  ಪ್ರತಿಜ್ಞಾ( z_ಮಾರ್ಗ!=0 );
  ಪ್ರತಿಜ್ಞಾ( ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(z_ಮಾರ್ಗ)<=SQLITE_MAX_PATHLEN );  /* tag-20210611-1 */
  ಹಿಂತಿರುಗಿ pVfs->xDlOpen(pVfs, z_ಮಾರ್ಗ);
}
ಶೂನ್ಯ sqlite3OsDlError(sqlite3_vfs *pVfs, ಪೂರ್ಣಾಂಕ nByte, ಅಕ್ಷರ *z_Buf_ಹೊರಹೋಗು){
  pVfs->xDlError(pVfs, nByte, z_Buf_ಹೊರಹೋಗು);
}
ಶೂನ್ಯ (*sqlite3OsDlSym(sqlite3_vfs *pVfs, ಶೂನ್ಯ *pHdle, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSym))(ಶೂನ್ಯ){
  ಹಿಂತಿರುಗಿ pVfs->xDlSym(pVfs, pHdle, zSym);
}
ಶೂನ್ಯ sqlite3OsDlClose(sqlite3_vfs *pVfs, ಶೂನ್ಯ *pHandle){
  pVfs->xDlClose(pVfs, pHandle);
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_LOAD_EXTENSION */
ಪೂರ್ಣಾಂಕ sqlite3OsRandomness(sqlite3_vfs *pVfs, ಪೂರ್ಣಾಂಕ nByte, ಅಕ್ಷರ *z_Buf_ಹೊರಹೋಗು){
  ಆದರೆ( sqlite3Config.i_Prng_ಬೀಜ ){
    memset(z_Buf_ಹೊರಹೋಗು, 0, nByte);
    ಆದರೆ( ALWAYS(nByte>(ಚಿಹ್ನೆಯುಳ್ಳ)ಗಾತ್ರ(ಚಿಹ್ನೆರಹಿತ)) ) nByte = ಗಾತ್ರ(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ);
    ಸ್ಮರಣೆ_ನಕಲು(z_Buf_ಹೊರಹೋಗು, &sqlite3Config.i_Prng_ಬೀಜ, nByte);
    ಹಿಂತಿರುಗಿ SQLITE_OK;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಹಿಂತಿರುಗಿ pVfs->xRandomness(pVfs, nByte, z_Buf_ಹೊರಹೋಗು);
  }

}
ಪೂರ್ಣಾಂಕ sqlite3OsSleep(sqlite3_vfs *pVfs, ಪೂರ್ಣಾಂಕ nMicro){
  ಹಿಂತಿರುಗಿ pVfs->xSleep(pVfs, nMicro);
}
ಪೂರ್ಣಾಂಕ sqlite3_Os_Get_ಕೊನೆಯ_Error(sqlite3_vfs *pVfs){
  ಹಿಂತಿರುಗಿ pVfs->x_Get_ಕೊನೆಯ_Error ? pVfs->x_Get_ಕೊನೆಯ_Error(pVfs, 0, 0) : 0;
}
ಪೂರ್ಣಾಂಕ sqlite3_Os_Current_ಸಮಯ_Int64(sqlite3_vfs *pVfs, sqlite3_int64 *p_ಸಮಯ_ಹೊರಹೋಗು){
  ಪೂರ್ಣಾಂಕ rc;
  /* IMPLEMENTATION-OF: R-49045-42493 SQLite will use the xCurrentTimeInt64()
  ** method to get the current date and time if that method is available
  ** (if iVersion is 2 or greater and the function pointer is not NULL) and
  ** will fall back to xCurrentTime() if xCurrentTimeInt64() is
  ** unavailable.
  */
  ಆದರೆ( pVfs->i_ಆವೃತ್ತಿ>=2 && pVfs->x_Current_ಸಮಯ_Int64 ){
    rc = pVfs->x_Current_ಸಮಯ_Int64(pVfs, p_ಸಮಯ_ಹೊರಹೋಗು);
  }ಇಲ್ಲದಿದ್ದರೆ{
    ದ್ವಿಗುಣ r;
    rc = pVfs->x_Current_ಸಮಯ(pVfs, &r);
    *p_ಸಮಯ_ಹೊರಹೋಗು = sqlite3_Real_ಗಮ್ಯ_I64(r*86400000.0);
  }
  ಹಿಂತಿರುಗಿ rc;
}

ಪೂರ್ಣಾಂಕ sqlite3OsOpenMalloc(
  sqlite3_vfs *pVfs,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFile,
  sqlite3_file **ppFile,
  ಪೂರ್ಣಾಂಕ flags,
  ಪೂರ್ಣಾಂಕ *p_ಹೊರಹೋಗು_Flags
){
  ಪೂರ್ಣಾಂಕ rc;
  sqlite3_file *pFile;
  pFile = (sqlite3_file *)sqlite3MallocZero(pVfs->szOsFile);
  ಆದರೆ( pFile ){
    rc = sqlite3OsOpen(pVfs, zFile, pFile, flags, p_ಹೊರಹೋಗು_Flags);
    ಆದರೆ( rc!=SQLITE_OK ){
      sqlite3_free(pFile);
      *ppFile = 0;
    }ಇಲ್ಲದಿದ್ದರೆ{
      *ppFile = pFile;
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    *ppFile = 0;
    rc = SQLITE_NOMEM_BKPT;
  }
  ಪ್ರತಿಜ್ಞಾ( *ppFile!=0 || rc!=SQLITE_OK );
  ಹಿಂತಿರುಗಿ rc;
}
ಶೂನ್ಯ sqlite3OsCloseFree(sqlite3_file *pFile){
  ಪ್ರತಿಜ್ಞಾ( pFile );
  sqlite3OsClose(pFile);
  sqlite3_free(pFile);
}

/*
** This function is a wrapper around the OS specific implementation of
** sqlite3_os_init(). The purpose of the wrapper is to provide the
** ability to simulate a malloc failure, so that the handling of an
** error in sqlite3_os_init() by the upper layers can be tested.
*/
ಪೂರ್ಣಾಂಕ sqlite3OsInit(ಶೂನ್ಯ){
  ಶೂನ್ಯ *p = sqlite3_malloc(10);
  ಆದರೆ( p==0 ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
  sqlite3_free(p);
  ಹಿಂತಿರುಗಿ sqlite3_os_init();
}

/*
** The list of all registered VFS implementations.
*/
ಸ್ಥಿರ sqlite3_vfs * SQLITE_WSD vfsList = 0;
#ವ್ಯಾಖ್ಯಾನಿಸು vfsList GLOBAL(sqlite3_vfs *, vfsList)

/*
** Locate a VFS by name.  If no name is given, simply return the
** first VFS on the list.
*/
sqlite3_vfs *sqlite3_vfs_ಕಂಡುಹಿಡಿ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zVfs){
  sqlite3_vfs *pVfs = 0;
#ಆದರೆ SQLITE_THREADSAFE
  sqlite3_mutex *mutex;
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOINIT
  ಪೂರ್ಣಾಂಕ rc = sqlite3_initialize();
  ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ 0;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ SQLITE_THREADSAFE
  mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_mutex_enter(mutex);
  ಗಾಗಿ(pVfs = vfsList; pVfs; pVfs=pVfs->p_ಮುಂದಿನ){
    ಆದರೆ( zVfs==0 ) ನಿಲ್ಲಿಸು;
    ಆದರೆ( strcmp(zVfs, pVfs->zName)==0 ) ನಿಲ್ಲಿಸು;
  }
  sqlite3_mutex_leave(mutex);
  ಹಿಂತಿರುಗಿ pVfs;
}

/*
** Unlink a VFS from the linked list
*/
ಸ್ಥಿರ ಶೂನ್ಯ vfsUnlink(sqlite3_vfs *pVfs){
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN)) );
  ಆದರೆ( pVfs==0 ){
    /* No-op */
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( vfsList==pVfs ){
    vfsList = pVfs->p_ಮುಂದಿನ;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( vfsList ){
    sqlite3_vfs *p = vfsList;
    ಆಗ( p->p_ಮುಂದಿನ && p->p_ಮುಂದಿನ!=pVfs ){
      p = p->p_ಮುಂದಿನ;
    }
    ಆದರೆ( p->p_ಮುಂದಿನ==pVfs ){
      p->p_ಮುಂದಿನ = pVfs->p_ಮುಂದಿನ;
    }
  }
}

/*
** Register a VFS with the system.  It is harmless to register the same
** VFS multiple times.  The new VFS becomes the default if makeDflt is
** true.
*/
ಪೂರ್ಣಾಂಕ sqlite3_vfs_ರಿಜಿಸ್ಟರ್(sqlite3_vfs *pVfs, ಪೂರ್ಣಾಂಕ makeDflt){
  MUTEX_LOGIC(sqlite3_mutex *mutex;)
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOINIT
  ಪೂರ್ಣಾಂಕ rc = sqlite3_initialize();
  ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ rc;
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( pVfs==0 ) ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
#ಆದರೆ_ಅಂತ್ಯ

  MUTEX_LOGIC( mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  sqlite3_mutex_enter(mutex);
  vfsUnlink(pVfs);
  ಆದರೆ( makeDflt || vfsList==0 ){
    pVfs->p_ಮುಂದಿನ = vfsList;
    vfsList = pVfs;
  }ಇಲ್ಲದಿದ್ದರೆ{
    pVfs->p_ಮುಂದಿನ = vfsList->p_ಮುಂದಿನ;
    vfsList->p_ಮುಂದಿನ = pVfs;
  }
  ಪ್ರತಿಜ್ಞಾ(vfsList);
  sqlite3_mutex_leave(mutex);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Unregister a VFS so that it is no longer accessible.
*/
ಪೂರ್ಣಾಂಕ sqlite3_vfs_unregister(sqlite3_vfs *pVfs){
  MUTEX_LOGIC(sqlite3_mutex *mutex;)
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOINIT
  ಪೂರ್ಣಾಂಕ rc = sqlite3_initialize();
  ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ rc;
#ಆದರೆ_ಅಂತ್ಯ
  MUTEX_LOGIC( mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  sqlite3_mutex_enter(mutex);
  vfsUnlink(pVfs);
  sqlite3_mutex_leave(mutex);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}
