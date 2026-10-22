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
#شامل "sqliteInt.h"

/*
** If we compile with the SQLITE_TEST macro set, then the following block
** of code will give us the ability to simulate a disk I/O error.  This
** is used for testing the I/O recovery logic.
*/
#اگر defined(SQLITE_TEST)
صحیح sqlite3_io_error_کامیابی = 0;            /* Total number of I/O Errors */
صحیح sqlite3_io_error_hardhit = 0;        /* Number of non-benign errors */
صحیح sqlite3_io_error_pending = 0;        /* Count down to first I/O error */
صحیح sqlite3_io_error_persist = 0;        /* True if I/O errors persist */
صحیح sqlite3_io_error_benign = 0;         /* True if errors are benign */
صحیح sqlite3_diskfull_pending = 0;
صحیح sqlite3_diskfull = 0;
#اگر_ختم /* defined(SQLITE_TEST) */

/*
** When testing, also keep a count of the number of open files.
*/
#اگر defined(SQLITE_TEST)
صحیح sqlite3_open_file_گنتی = 0;
#اگر_ختم /* defined(SQLITE_TEST) */

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
#اگر defined(SQLITE_TEST)
صحیح sqlite3_memdebug_vfs_oom_test = 1;
  #تعریف_کرو DO_OS_MALLOC_TEST(x)                                       \
  اگر (sqlite3_memdebug_vfs_oom_test && (!x || !sqlite3_Journal_Is_اندر_حافظہ(x))) { \
    خالی *pTstAlloc = sqlite3Malloc(10);                             \
    اگر (!pTstAlloc) واپس SQLITE_IOERR_NOMEM_BKPT;                  \
    sqlite3_free(pTstAlloc);                                         \
  }
#ورنہ
  #تعریف_کرو DO_OS_MALLOC_TEST(x)
#اگر_ختم

/*
** The following routines are convenience wrappers around methods
** of the sqlite3_file object.  This is mostly just syntactic sugar. All
** of this would be completely automatic if SQLite were coded using
** C++ instead of plain old C.
*/
خالی sqlite3OsClose(sqlite3_file *pId){
  اگر( pId->pMethods ){
    pId->pMethods->xClose(pId);
    pId->pMethods = 0;
  }
}
صحیح sqlite3OsRead(sqlite3_file *id, خالی *pBuf, صحیح amt, i64 انحراف){
  DO_OS_MALLOC_TEST(id);
  واپس id->pMethods->xRead(id, pBuf, amt, انحراف);
}
صحیح sqlite3OsWrite(sqlite3_file *id, ثابت خالی *pBuf, صحیح amt, i64 انحراف){
  DO_OS_MALLOC_TEST(id);
  واپس id->pMethods->xWrite(id, pBuf, amt, انحراف);
}
صحیح sqlite3OsTruncate(sqlite3_file *id, i64 سائز){
  واپس id->pMethods->xTruncate(id, سائز);
}
صحیح sqlite3OsSync(sqlite3_file *id, صحیح flags){
  DO_OS_MALLOC_TEST(id);
  واپس flags ? id->pMethods->xSync(id, flags) : SQLITE_OK;
}
صحیح sqlite3_Os_File_سائز(sqlite3_file *id, i64 *p_سائز){
  DO_OS_MALLOC_TEST(id);
  واپس id->pMethods->x_File_سائز(id, p_سائز);
}
صحیح sqlite3_Os_تالا(sqlite3_file *id, صحیح تالا_Type){
  DO_OS_MALLOC_TEST(id);
  أكد( تالا_Type>=SQLITE_تالا_SHARED && تالا_Type<=SQLITE_تالا_EXCLUSIVE );
  واپس id->pMethods->x_تالا(id, تالا_Type);
}
صحیح sqlite3OsUnlock(sqlite3_file *id, صحیح تالا_Type){
  أكد( تالا_Type==SQLITE_تالا_NONE || تالا_Type==SQLITE_تالا_SHARED );
  واپس id->pMethods->xUnlock(id, تالا_Type);
}
صحیح sqlite3_Os_Check_Reserved_تالا(sqlite3_file *id, صحیح *p_Res_برآمد){
  DO_OS_MALLOC_TEST(id);
  واپس id->pMethods->x_Check_Reserved_تالا(id, p_Res_برآمد);
}

/*
** Use sqlite3OsFileControl() when we are doing something that might fail
** and we need to know about the failures.  Use sqlite3OsFileControlHint()
** when simply tossing information over the wall to the VFS and we do not
** really care if the VFS receives and understands the information since it
** is only a hint and can be safely ignored.  The sqlite3OsFileControlHint()
** routine has no return value since the return value would be meaningless.
*/
صحیح sqlite3OsFileControl(sqlite3_file *id, صحیح op, خالی *pArg){
  اگر( id->pMethods==0 ) واپس SQLITE_NOTFOUND;
#اگر_تعریف_شدہ SQLITE_TEST
  اگر( op!=SQLITE_FCNTL_COMMIT_PHASETWO
   && op!=SQLITE_FCNTL_تالا_TIMEOUT
   && op!=SQLITE_FCNTL_CKPT_مکمل
   && op!=SQLITE_FCNTL_CKPT_آغاز
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
#اگر_ختم
  واپس id->pMethods->xFileControl(id, op, pArg);
}
خالی sqlite3OsFileControlHint(sqlite3_file *id, صحیح op, خالی *pArg){
  اگر( id->pMethods ) (خالی)id->pMethods->xFileControl(id, op, pArg);
}

صحیح sqlite3_Os_Sector_سائز(sqlite3_file *id){
  صحیح (*x_Sector_سائز)(sqlite3_file*) = id->pMethods->x_Sector_سائز;
  واپس (x_Sector_سائز ? x_Sector_سائز(id) : SQLITE_DEFAULT_SECTOR_سائز);
}
صحیح sqlite3OsDeviceCharacteristics(sqlite3_file *id){
  اگر( NEVER(id->pMethods==0) ) واپس 0;
  واپس id->pMethods->xDeviceCharacteristics(id);
}
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WAL
صحیح sqlite3_Os_Shm_تالا(sqlite3_file *id, صحیح انحراف, صحیح n, صحیح flags){
  واپس id->pMethods->x_Shm_تالا(id, انحراف, n, flags);
}
خالی sqlite3OsShmBarrier(sqlite3_file *id){
  id->pMethods->xShmBarrier(id);
}
صحیح sqlite3OsShmUnmap(sqlite3_file *id, صحیح deleteFlag){
  واپس id->pMethods->xShmUnmap(id, deleteFlag);
}
صحیح sqlite3OsShmMap(
  sqlite3_file *id,               /* Database file handle */
  صحیح i_صفحہ,
  صحیح pgsz,
  صحیح bExtend,                    /* True to extend file if necessary */
  خالی متلون **pp              /* OUT: Pointer to mapping */
){
  DO_OS_MALLOC_TEST(id);
  واپس id->pMethods->xShmMap(id, i_صفحہ, pgsz, bExtend, pp);
}
#اگر_ختم /* SQLITE_OMIT_WAL */

#اگر SQLITE_MAX_MMAP_سائز>0
/* The real implementation of xFetch and xUnfetch */
صحیح sqlite3OsFetch(sqlite3_file *id, i64 iOff, صحیح iAmt, خالی **pp){
  DO_OS_MALLOC_TEST(id);
  واپس id->pMethods->xFetch(id, iOff, iAmt, pp);
}
صحیح sqlite3OsUnfetch(sqlite3_file *id, i64 iOff, خالی *p){
  واپس id->pMethods->xUnfetch(id, iOff, p);
}
#ورنہ
/* No-op stubs to use when memory-mapped I/O is disabled */
صحیح sqlite3OsFetch(sqlite3_file *id, i64 iOff, صحیح iAmt, خالی **pp){
  *pp = 0;
  واپس SQLITE_OK;
}
صحیح sqlite3OsUnfetch(sqlite3_file *id, i64 iOff, خالی *p){
  واپس SQLITE_OK;
}
#اگر_ختم

/*
** The next group of routines are convenience wrappers around the
** VFS methods.
*/
صحیح sqlite3OsOpen(
  sqlite3_vfs *pVfs,
  ثابت حرف *z_راستہ,
  sqlite3_file *pFile,
  صحیح flags,
  صحیح *p_Flags_برآمد
){
  صحیح rc;
  DO_OS_MALLOC_TEST(0);
  /* 0x87f7f is a mask of SQLITE_OPEN_ flags that are valid to be passed
  ** down into the VFS layer.  Some SQLITE_OPEN_ flags (for example,
  ** SQLITE_OPEN_FULLMUTEX or SQLITE_OPEN_SHAREDCACHE) are blocked before
  ** reaching the VFS. */
  أكد( z_راستہ || (flags & SQLITE_OPEN_EXCLUSIVE) );
  rc = pVfs->xOpen(pVfs, z_راستہ, pFile, flags & 0x1087f7f, p_Flags_برآمد);
  أكد( rc==SQLITE_OK || pFile->pMethods==0 );
  واپس rc;
}
صحیح sqlite3OsDelete(sqlite3_vfs *pVfs, ثابت حرف *z_راستہ, صحیح dirSync){
  DO_OS_MALLOC_TEST(0);
  أكد( dirSync==0 || dirSync==1 );
  واپس pVfs->xDelete!=0 ? pVfs->xDelete(pVfs, z_راستہ, dirSync) : SQLITE_OK;
}
صحیح sqlite3OsAccess(
  sqlite3_vfs *pVfs,
  ثابت حرف *z_راستہ,
  صحیح flags,
  صحیح *p_Res_برآمد
){
  DO_OS_MALLOC_TEST(0);
  واپس pVfs->xAccess(pVfs, z_راستہ, flags, p_Res_برآمد);
}
صحیح sqlite3_Os_بھرا_Pathname(
  sqlite3_vfs *pVfs,
  ثابت حرف *z_راستہ,
  صحیح n_راستہ_برآمد,
  حرف *z_راستہ_برآمد
){
  DO_OS_MALLOC_TEST(0);
  z_راستہ_برآمد[0] = 0;
  واپس pVfs->x_بھرا_Pathname(pVfs, z_راستہ, n_راستہ_برآمد, z_راستہ_برآمد);
}
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_بھرو_EXTENSION
خالی *sqlite3OsDlOpen(sqlite3_vfs *pVfs, ثابت حرف *z_راستہ){
  أكد( z_راستہ!=0 );
  أكد( لڑی_لمبائی(z_راستہ)<=SQLITE_MAX_PATHLEN );  /* tag-20210611-1 */
  واپس pVfs->xDlOpen(pVfs, z_راستہ);
}
خالی sqlite3OsDlError(sqlite3_vfs *pVfs, صحیح nByte, حرف *z_Buf_برآمد){
  pVfs->xDlError(pVfs, nByte, z_Buf_برآمد);
}
خالی (*sqlite3OsDlSym(sqlite3_vfs *pVfs, خالی *pHdle, ثابت حرف *zSym))(خالی){
  واپس pVfs->xDlSym(pVfs, pHdle, zSym);
}
خالی sqlite3OsDlClose(sqlite3_vfs *pVfs, خالی *pHandle){
  pVfs->xDlClose(pVfs, pHandle);
}
#اگر_ختم /* SQLITE_OMIT_LOAD_EXTENSION */
صحیح sqlite3OsRandomness(sqlite3_vfs *pVfs, صحیح nByte, حرف *z_Buf_برآمد){
  اگر( sqlite3Config.i_Prng_بیج ){
    memset(z_Buf_برآمد, 0, nByte);
    اگر( ALWAYS(nByte>(علامتی)حجم(غیرعلامتی)) ) nByte = حجم(غیرعلامتی صحیح);
    حافظہ_نقل(z_Buf_برآمد, &sqlite3Config.i_Prng_بیج, nByte);
    واپس SQLITE_OK;
  }ورنہ{
    واپس pVfs->xRandomness(pVfs, nByte, z_Buf_برآمد);
  }

}
صحیح sqlite3OsSleep(sqlite3_vfs *pVfs, صحیح nMicro){
  واپس pVfs->xSleep(pVfs, nMicro);
}
صحیح sqlite3_Os_Get_آخری_Error(sqlite3_vfs *pVfs){
  واپس pVfs->x_Get_آخری_Error ? pVfs->x_Get_آخری_Error(pVfs, 0, 0) : 0;
}
صحیح sqlite3_Os_Current_وقت_Int64(sqlite3_vfs *pVfs, sqlite3_int64 *p_وقت_برآمد){
  صحیح rc;
  /* IMPLEMENTATION-OF: R-49045-42493 SQLite will use the xCurrentTimeInt64()
  ** method to get the current date and time if that method is available
  ** (if iVersion is 2 or greater and the function pointer is not NULL) and
  ** will fall back to xCurrentTime() if xCurrentTimeInt64() is
  ** unavailable.
  */
  اگر( pVfs->i_نسخہ>=2 && pVfs->x_Current_وقت_Int64 ){
    rc = pVfs->x_Current_وقت_Int64(pVfs, p_وقت_برآمد);
  }ورنہ{
    دوہرا r;
    rc = pVfs->x_Current_وقت(pVfs, &r);
    *p_وقت_برآمد = sqlite3_Real_مقصد_I64(r*86400000.0);
  }
  واپس rc;
}

صحیح sqlite3OsOpenMalloc(
  sqlite3_vfs *pVfs,
  ثابت حرف *zFile,
  sqlite3_file **ppFile,
  صحیح flags,
  صحیح *p_برآمد_Flags
){
  صحیح rc;
  sqlite3_file *pFile;
  pFile = (sqlite3_file *)sqlite3MallocZero(pVfs->szOsFile);
  اگر( pFile ){
    rc = sqlite3OsOpen(pVfs, zFile, pFile, flags, p_برآمد_Flags);
    اگر( rc!=SQLITE_OK ){
      sqlite3_free(pFile);
      *ppFile = 0;
    }ورنہ{
      *ppFile = pFile;
    }
  }ورنہ{
    *ppFile = 0;
    rc = SQLITE_NOMEM_BKPT;
  }
  أكد( *ppFile!=0 || rc!=SQLITE_OK );
  واپس rc;
}
خالی sqlite3OsCloseFree(sqlite3_file *pFile){
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
صحیح sqlite3OsInit(خالی){
  خالی *p = sqlite3_malloc(10);
  اگر( p==0 ) واپس SQLITE_NOMEM_BKPT;
  sqlite3_free(p);
  واپس sqlite3_os_init();
}

/*
** The list of all registered VFS implementations.
*/
ساکن sqlite3_vfs * SQLITE_WSD vfsList = 0;
#تعریف_کرو vfsList GLOBAL(sqlite3_vfs *, vfsList)

/*
** Locate a VFS by name.  If no name is given, simply return the
** first VFS on the list.
*/
sqlite3_vfs *sqlite3_vfs_ڈھونڈو(ثابت حرف *zVfs){
  sqlite3_vfs *pVfs = 0;
#اگر SQLITE_THREADSAFE
  sqlite3_mutex *mutex;
#اگر_ختم
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOINIT
  صحیح rc = sqlite3_initialize();
  اگر( rc ) واپس 0;
#اگر_ختم
#اگر SQLITE_THREADSAFE
  mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
#اگر_ختم
  sqlite3_mutex_enter(mutex);
  برائے(pVfs = vfsList; pVfs; pVfs=pVfs->p_اگلا){
    اگر( zVfs==0 ) توڑو;
    اگر( strcmp(zVfs, pVfs->zName)==0 ) توڑو;
  }
  sqlite3_mutex_leave(mutex);
  واپس pVfs;
}

/*
** Unlink a VFS from the linked list
*/
ساکن خالی vfsUnlink(sqlite3_vfs *pVfs){
  أكد( sqlite3_mutex_held(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN)) );
  اگر( pVfs==0 ){
    /* No-op */
  }ورنہ اگر( vfsList==pVfs ){
    vfsList = pVfs->p_اگلا;
  }ورنہ اگر( vfsList ){
    sqlite3_vfs *p = vfsList;
    جب( p->p_اگلا && p->p_اگلا!=pVfs ){
      p = p->p_اگلا;
    }
    اگر( p->p_اگلا==pVfs ){
      p->p_اگلا = pVfs->p_اگلا;
    }
  }
}

/*
** Register a VFS with the system.  It is harmless to register the same
** VFS multiple times.  The new VFS becomes the default if makeDflt is
** true.
*/
صحیح sqlite3_vfs_رجسٹر(sqlite3_vfs *pVfs, صحیح makeDflt){
  MUTEX_LOGIC(sqlite3_mutex *mutex;)
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOINIT
  صحیح rc = sqlite3_initialize();
  اگر( rc ) واپس rc;
#اگر_ختم
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( pVfs==0 ) واپس SQLITE_MISUSE_BKPT;
#اگر_ختم

  MUTEX_LOGIC( mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  sqlite3_mutex_enter(mutex);
  vfsUnlink(pVfs);
  اگر( makeDflt || vfsList==0 ){
    pVfs->p_اگلا = vfsList;
    vfsList = pVfs;
  }ورنہ{
    pVfs->p_اگلا = vfsList->p_اگلا;
    vfsList->p_اگلا = pVfs;
  }
  أكد(vfsList);
  sqlite3_mutex_leave(mutex);
  واپس SQLITE_OK;
}

/*
** Unregister a VFS so that it is no longer accessible.
*/
صحیح sqlite3_vfs_unregister(sqlite3_vfs *pVfs){
  MUTEX_LOGIC(sqlite3_mutex *mutex;)
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOINIT
  صحیح rc = sqlite3_initialize();
  اگر( rc ) واپس rc;
#اگر_ختم
  MUTEX_LOGIC( mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  sqlite3_mutex_enter(mutex);
  vfsUnlink(pVfs);
  sqlite3_mutex_leave(mutex);
  واپس SQLITE_OK;
}
