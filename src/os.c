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
#ضمن "sqliteInt.h"

/*
** If we compile with the SQLITE_TEST macro set, then the following block
** of code will give us the ability to simulate a disk I/O error.  This
** is used for testing the I/O recovery logic.
*/
#إذا defined(SQLITE_TEST)
صحيح sqlite3_io_error_ڪاميابي = 0;            /* Total number of I/O Errors */
صحيح sqlite3_io_error_hardhit = 0;        /* Number of non-benign errors */
صحيح sqlite3_io_error_pending = 0;        /* Count down to first I/O error */
صحيح sqlite3_io_error_persist = 0;        /* True if I/O errors persist */
صحيح sqlite3_io_error_benign = 0;         /* True if errors are benign */
صحيح sqlite3_diskfull_pending = 0;
صحيح sqlite3_diskfull = 0;
#جيڪڏهن_پورو /* defined(SQLITE_TEST) */

/*
** When testing, also keep a count of the number of open files.
*/
#إذا defined(SQLITE_TEST)
صحيح sqlite3_open_file_ڳڻپ = 0;
#جيڪڏهن_پورو /* defined(SQLITE_TEST) */

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
#إذا defined(SQLITE_TEST)
صحيح sqlite3_memdebug_vfs_oom_test = 1;
  #وصف_ڪريو DO_OS_MALLOC_TEST(x)                                       \
  إذا (sqlite3_memdebug_vfs_oom_test && (!x || !sqlite3_Journal_Is_اندريون_يادگيري(x))) { \
    فراغ *pTstAlloc = sqlite3Malloc(10);                             \
    إذا (!pTstAlloc) أرجع SQLITE_IOERR_NOMEM_BKPT;                  \
    sqlite3_free(pTstAlloc);                                         \
  }
#وإلا
  #وصف_ڪريو DO_OS_MALLOC_TEST(x)
#جيڪڏهن_پورو

/*
** The following routines are convenience wrappers around methods
** of the sqlite3_file object.  This is mostly just syntactic sugar. All
** of this would be completely automatic if SQLite were coded using
** C++ instead of plain old C.
*/
فراغ sqlite3OsClose(sqlite3_file *pId){
  إذا( pId->pMethods ){
    pId->pMethods->xClose(pId);
    pId->pMethods = 0;
  }
}
صحيح sqlite3OsRead(sqlite3_file *id, فراغ *pBuf, صحيح amt, i64 وٿي){
  DO_OS_MALLOC_TEST(id);
  أرجع id->pMethods->xRead(id, pBuf, amt, وٿي);
}
صحيح sqlite3OsWrite(sqlite3_file *id, ثابت فراغ *pBuf, صحيح amt, i64 وٿي){
  DO_OS_MALLOC_TEST(id);
  أرجع id->pMethods->xWrite(id, pBuf, amt, وٿي);
}
صحيح sqlite3OsTruncate(sqlite3_file *id, i64 سائيز){
  أرجع id->pMethods->xTruncate(id, سائيز);
}
صحيح sqlite3OsSync(sqlite3_file *id, صحيح flags){
  DO_OS_MALLOC_TEST(id);
  أرجع flags ? id->pMethods->xSync(id, flags) : SQLITE_OK;
}
صحيح sqlite3_Os_File_سائيز(sqlite3_file *id, i64 *p_سائيز){
  DO_OS_MALLOC_TEST(id);
  أرجع id->pMethods->x_File_سائيز(id, p_سائيز);
}
صحيح sqlite3_Os_تالو(sqlite3_file *id, صحيح تالو_Type){
  DO_OS_MALLOC_TEST(id);
  أكد( تالو_Type>=SQLITE_تالو_SHARED && تالو_Type<=SQLITE_تالو_EXCLUSIVE );
  أرجع id->pMethods->x_تالو(id, تالو_Type);
}
صحيح sqlite3OsUnlock(sqlite3_file *id, صحيح تالو_Type){
  أكد( تالو_Type==SQLITE_تالو_NONE || تالو_Type==SQLITE_تالو_SHARED );
  أرجع id->pMethods->xUnlock(id, تالو_Type);
}
صحيح sqlite3_Os_Check_Reserved_تالو(sqlite3_file *id, صحيح *p_Res_ٻاهريون){
  DO_OS_MALLOC_TEST(id);
  أرجع id->pMethods->x_Check_Reserved_تالو(id, p_Res_ٻاهريون);
}

/*
** Use sqlite3OsFileControl() when we are doing something that might fail
** and we need to know about the failures.  Use sqlite3OsFileControlHint()
** when simply tossing information over the wall to the VFS and we do not
** really care if the VFS receives and understands the information since it
** is only a hint and can be safely ignored.  The sqlite3OsFileControlHint()
** routine has no return value since the return value would be meaningless.
*/
صحيح sqlite3OsFileControl(sqlite3_file *id, صحيح op, فراغ *pArg){
  إذا( id->pMethods==0 ) أرجع SQLITE_NOTFOUND;
#جيڪڏهن_وصف_ٿيل SQLITE_TEST
  إذا( op!=SQLITE_FCNTL_COMMIT_PHASETWO
   && op!=SQLITE_FCNTL_تالو_TIMEOUT
   && op!=SQLITE_FCNTL_CKPT_پورو
   && op!=SQLITE_FCNTL_CKPT_شروعات
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
#جيڪڏهن_پورو
  أرجع id->pMethods->xFileControl(id, op, pArg);
}
فراغ sqlite3OsFileControlHint(sqlite3_file *id, صحيح op, فراغ *pArg){
  إذا( id->pMethods ) (فراغ)id->pMethods->xFileControl(id, op, pArg);
}

صحيح sqlite3_Os_Sector_سائيز(sqlite3_file *id){
  صحيح (*x_Sector_سائيز)(sqlite3_file*) = id->pMethods->x_Sector_سائيز;
  أرجع (x_Sector_سائيز ? x_Sector_سائيز(id) : SQLITE_DEFAULT_SECTOR_سائيز);
}
صحيح sqlite3OsDeviceCharacteristics(sqlite3_file *id){
  إذا( NEVER(id->pMethods==0) ) أرجع 0;
  أرجع id->pMethods->xDeviceCharacteristics(id);
}
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_WAL
صحيح sqlite3_Os_Shm_تالو(sqlite3_file *id, صحيح وٿي, صحيح n, صحيح flags){
  أرجع id->pMethods->x_Shm_تالو(id, وٿي, n, flags);
}
فراغ sqlite3OsShmBarrier(sqlite3_file *id){
  id->pMethods->xShmBarrier(id);
}
صحيح sqlite3OsShmUnmap(sqlite3_file *id, صحيح deleteFlag){
  أرجع id->pMethods->xShmUnmap(id, deleteFlag);
}
صحيح sqlite3OsShmMap(
  sqlite3_file *id,               /* Database file handle */
  صحيح i_صفحو,
  صحيح pgsz,
  صحيح bExtend,                    /* True to extend file if necessary */
  فراغ متطاير **pp              /* OUT: Pointer to mapping */
){
  DO_OS_MALLOC_TEST(id);
  أرجع id->pMethods->xShmMap(id, i_صفحو, pgsz, bExtend, pp);
}
#جيڪڏهن_پورو /* SQLITE_OMIT_WAL */

#إذا SQLITE_MAX_MMAP_سائيز>0
/* The real implementation of xFetch and xUnfetch */
صحيح sqlite3OsFetch(sqlite3_file *id, i64 iOff, صحيح iAmt, فراغ **pp){
  DO_OS_MALLOC_TEST(id);
  أرجع id->pMethods->xFetch(id, iOff, iAmt, pp);
}
صحيح sqlite3OsUnfetch(sqlite3_file *id, i64 iOff, فراغ *p){
  أرجع id->pMethods->xUnfetch(id, iOff, p);
}
#وإلا
/* No-op stubs to use when memory-mapped I/O is disabled */
صحيح sqlite3OsFetch(sqlite3_file *id, i64 iOff, صحيح iAmt, فراغ **pp){
  *pp = 0;
  أرجع SQLITE_OK;
}
صحيح sqlite3OsUnfetch(sqlite3_file *id, i64 iOff, فراغ *p){
  أرجع SQLITE_OK;
}
#جيڪڏهن_پورو

/*
** The next group of routines are convenience wrappers around the
** VFS methods.
*/
صحيح sqlite3OsOpen(
  sqlite3_vfs *pVfs,
  ثابت حرف *z_رستو,
  sqlite3_file *pFile,
  صحيح flags,
  صحيح *p_Flags_ٻاهريون
){
  صحيح rc;
  DO_OS_MALLOC_TEST(0);
  /* 0x87f7f is a mask of SQLITE_OPEN_ flags that are valid to be passed
  ** down into the VFS layer.  Some SQLITE_OPEN_ flags (for example,
  ** SQLITE_OPEN_FULLMUTEX or SQLITE_OPEN_SHAREDCACHE) are blocked before
  ** reaching the VFS. */
  أكد( z_رستو || (flags & SQLITE_OPEN_EXCLUSIVE) );
  rc = pVfs->xOpen(pVfs, z_رستو, pFile, flags & 0x1087f7f, p_Flags_ٻاهريون);
  أكد( rc==SQLITE_OK || pFile->pMethods==0 );
  أرجع rc;
}
صحيح sqlite3OsDelete(sqlite3_vfs *pVfs, ثابت حرف *z_رستو, صحيح dirSync){
  DO_OS_MALLOC_TEST(0);
  أكد( dirSync==0 || dirSync==1 );
  أرجع pVfs->xDelete!=0 ? pVfs->xDelete(pVfs, z_رستو, dirSync) : SQLITE_OK;
}
صحيح sqlite3OsAccess(
  sqlite3_vfs *pVfs,
  ثابت حرف *z_رستو,
  صحيح flags,
  صحيح *p_Res_ٻاهريون
){
  DO_OS_MALLOC_TEST(0);
  أرجع pVfs->xAccess(pVfs, z_رستو, flags, p_Res_ٻاهريون);
}
صحيح sqlite3_Os_ڀريل_Pathname(
  sqlite3_vfs *pVfs,
  ثابت حرف *z_رستو,
  صحيح n_رستو_ٻاهريون,
  حرف *z_رستو_ٻاهريون
){
  DO_OS_MALLOC_TEST(0);
  z_رستو_ٻاهريون[0] = 0;
  أرجع pVfs->x_ڀريل_Pathname(pVfs, z_رستو, n_رستو_ٻاهريون, z_رستو_ٻاهريون);
}
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_ڀريو_EXTENSION
فراغ *sqlite3OsDlOpen(sqlite3_vfs *pVfs, ثابت حرف *z_رستو){
  أكد( z_رستو!=0 );
  أكد( لڙي_ڊيگهه(z_رستو)<=SQLITE_MAX_PATHLEN );  /* tag-20210611-1 */
  أرجع pVfs->xDlOpen(pVfs, z_رستو);
}
فراغ sqlite3OsDlError(sqlite3_vfs *pVfs, صحيح nByte, حرف *z_Buf_ٻاهريون){
  pVfs->xDlError(pVfs, nByte, z_Buf_ٻاهريون);
}
فراغ (*sqlite3OsDlSym(sqlite3_vfs *pVfs, فراغ *pHdle, ثابت حرف *zSym))(فراغ){
  أرجع pVfs->xDlSym(pVfs, pHdle, zSym);
}
فراغ sqlite3OsDlClose(sqlite3_vfs *pVfs, فراغ *pHandle){
  pVfs->xDlClose(pVfs, pHandle);
}
#جيڪڏهن_پورو /* SQLITE_OMIT_LOAD_EXTENSION */
صحيح sqlite3OsRandomness(sqlite3_vfs *pVfs, صحيح nByte, حرف *z_Buf_ٻاهريون){
  إذا( sqlite3Config.i_Prng_ٻج ){
    memset(z_Buf_ٻاهريون, 0, nByte);
    إذا( ALWAYS(nByte>(نشانوارو)حجم(بنانشان)) ) nByte = حجم(بنانشان صحيح);
    يادگيري_نقل(z_Buf_ٻاهريون, &sqlite3Config.i_Prng_ٻج, nByte);
    أرجع SQLITE_OK;
  }وإلا{
    أرجع pVfs->xRandomness(pVfs, nByte, z_Buf_ٻاهريون);
  }

}
صحيح sqlite3OsSleep(sqlite3_vfs *pVfs, صحيح nMicro){
  أرجع pVfs->xSleep(pVfs, nMicro);
}
صحيح sqlite3_Os_Get_آخري_Error(sqlite3_vfs *pVfs){
  أرجع pVfs->x_Get_آخري_Error ? pVfs->x_Get_آخري_Error(pVfs, 0, 0) : 0;
}
صحيح sqlite3_Os_Current_وقت_Int64(sqlite3_vfs *pVfs, sqlite3_int64 *p_وقت_ٻاهريون){
  صحيح rc;
  /* IMPLEMENTATION-OF: R-49045-42493 SQLite will use the xCurrentTimeInt64()
  ** method to get the current date and time if that method is available
  ** (if iVersion is 2 or greater and the function pointer is not NULL) and
  ** will fall back to xCurrentTime() if xCurrentTimeInt64() is
  ** unavailable.
  */
  إذا( pVfs->i_نسخو>=2 && pVfs->x_Current_وقت_Int64 ){
    rc = pVfs->x_Current_وقت_Int64(pVfs, p_وقت_ٻاهريون);
  }وإلا{
    مزدوج r;
    rc = pVfs->x_Current_وقت(pVfs, &r);
    *p_وقت_ٻاهريون = sqlite3_Real_منزل_I64(r*86400000.0);
  }
  أرجع rc;
}

صحيح sqlite3OsOpenMalloc(
  sqlite3_vfs *pVfs,
  ثابت حرف *zFile,
  sqlite3_file **ppFile,
  صحيح flags,
  صحيح *p_ٻاهريون_Flags
){
  صحيح rc;
  sqlite3_file *pFile;
  pFile = (sqlite3_file *)sqlite3MallocZero(pVfs->szOsFile);
  إذا( pFile ){
    rc = sqlite3OsOpen(pVfs, zFile, pFile, flags, p_ٻاهريون_Flags);
    إذا( rc!=SQLITE_OK ){
      sqlite3_free(pFile);
      *ppFile = 0;
    }وإلا{
      *ppFile = pFile;
    }
  }وإلا{
    *ppFile = 0;
    rc = SQLITE_NOMEM_BKPT;
  }
  أكد( *ppFile!=0 || rc!=SQLITE_OK );
  أرجع rc;
}
فراغ sqlite3OsCloseFree(sqlite3_file *pFile){
  أكد( pFile );
  sqlite3OsClose(pFile);
  sqlite3_free(pFile);
}

/*
** This function is a wrapper around the OS specific implementation of
** sqlite3_os_init(). The purpose of the wrapper is to provide the
** ability to simulate a malloc failure, so that the handling of an
** error in sqlite3_os_init() by the upper layers can be tested.
*/
صحيح sqlite3OsInit(فراغ){
  فراغ *p = sqlite3_malloc(10);
  إذا( p==0 ) أرجع SQLITE_NOMEM_BKPT;
  sqlite3_free(p);
  أرجع sqlite3_os_init();
}

/*
** The list of all registered VFS implementations.
*/
ساكن sqlite3_vfs * SQLITE_WSD vfsList = 0;
#وصف_ڪريو vfsList GLOBAL(sqlite3_vfs *, vfsList)

/*
** Locate a VFS by name.  If no name is given, simply return the
** first VFS on the list.
*/
sqlite3_vfs *sqlite3_vfs_ڳوليو(ثابت حرف *zVfs){
  sqlite3_vfs *pVfs = 0;
#إذا SQLITE_THREADSAFE
  sqlite3_mutex *mutex;
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_AUTOINIT
  صحيح rc = sqlite3_initialize();
  إذا( rc ) أرجع 0;
#جيڪڏهن_پورو
#إذا SQLITE_THREADSAFE
  mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
#جيڪڏهن_پورو
  sqlite3_mutex_enter(mutex);
  لكل(pVfs = vfsList; pVfs; pVfs=pVfs->p_ايندڙ){
    إذا( zVfs==0 ) اكسر;
    إذا( strcmp(zVfs, pVfs->zName)==0 ) اكسر;
  }
  sqlite3_mutex_leave(mutex);
  أرجع pVfs;
}

/*
** Unlink a VFS from the linked list
*/
ساكن فراغ vfsUnlink(sqlite3_vfs *pVfs){
  أكد( sqlite3_mutex_held(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN)) );
  إذا( pVfs==0 ){
    /* No-op */
  }وإلا إذا( vfsList==pVfs ){
    vfsList = pVfs->p_ايندڙ;
  }وإلا إذا( vfsList ){
    sqlite3_vfs *p = vfsList;
    طالما( p->p_ايندڙ && p->p_ايندڙ!=pVfs ){
      p = p->p_ايندڙ;
    }
    إذا( p->p_ايندڙ==pVfs ){
      p->p_ايندڙ = pVfs->p_ايندڙ;
    }
  }
}

/*
** Register a VFS with the system.  It is harmless to register the same
** VFS multiple times.  The new VFS becomes the default if makeDflt is
** true.
*/
صحيح sqlite3_vfs_رجسٽر(sqlite3_vfs *pVfs, صحيح makeDflt){
  MUTEX_LOGIC(sqlite3_mutex *mutex;)
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_AUTOINIT
  صحيح rc = sqlite3_initialize();
  إذا( rc ) أرجع rc;
#جيڪڏهن_پورو
#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_API_ARMOR
  إذا( pVfs==0 ) أرجع SQLITE_MISUSE_BKPT;
#جيڪڏهن_پورو

  MUTEX_LOGIC( mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  sqlite3_mutex_enter(mutex);
  vfsUnlink(pVfs);
  إذا( makeDflt || vfsList==0 ){
    pVfs->p_ايندڙ = vfsList;
    vfsList = pVfs;
  }وإلا{
    pVfs->p_ايندڙ = vfsList->p_ايندڙ;
    vfsList->p_ايندڙ = pVfs;
  }
  أكد(vfsList);
  sqlite3_mutex_leave(mutex);
  أرجع SQLITE_OK;
}

/*
** Unregister a VFS so that it is no longer accessible.
*/
صحيح sqlite3_vfs_unregister(sqlite3_vfs *pVfs){
  MUTEX_LOGIC(sqlite3_mutex *mutex;)
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_AUTOINIT
  صحيح rc = sqlite3_initialize();
  إذا( rc ) أرجع rc;
#جيڪڏهن_پورو
  MUTEX_LOGIC( mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  sqlite3_mutex_enter(mutex);
  vfsUnlink(pVfs);
  sqlite3_mutex_leave(mutex);
  أرجع SQLITE_OK;
}
