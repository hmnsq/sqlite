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
#అన్తర్భావ "sqliteInt.h"

/*
** If we compile with the SQLITE_TEST macro set, then the following block
** of code will give us the ability to simulate a disk I/O error.  This
** is used for testing the I/O recovery logic.
*/
#ఒకవేళ defined(SQLITE_TEST)
పూర్ణాంకం sqlite3_io_error_తగిలింది = 0;            /* Total number of I/O Errors */
పూర్ణాంకం sqlite3_io_error_hardhit = 0;        /* Number of non-benign errors */
పూర్ణాంకం sqlite3_io_error_pending = 0;        /* Count down to first I/O error */
పూర్ణాంకం sqlite3_io_error_persist = 0;        /* True if I/O errors persist */
పూర్ణాంకం sqlite3_io_error_benign = 0;         /* True if errors are benign */
పూర్ణాంకం sqlite3_diskfull_pending = 0;
పూర్ణాంకం sqlite3_diskfull = 0;
#అయితే_ముగింపు /* defined(SQLITE_TEST) */

/*
** When testing, also keep a count of the number of open files.
*/
#ఒకవేళ defined(SQLITE_TEST)
పూర్ణాంకం sqlite3_open_file_లెక్క = 0;
#అయితే_ముగింపు /* defined(SQLITE_TEST) */

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
#ఒకవేళ defined(SQLITE_TEST)
పూర్ణాంకం sqlite3_memdebug_vfs_oom_test = 1;
  #నిర్వచించు DO_OS_MALLOC_TEST(x)                                       \
  ఒకవేళ (sqlite3_memdebug_vfs_oom_test && (!x || !sqlite3_Journal_Is_లోపల_జ్ఞాపకం(x))) { \
    శూన్య *pTstAlloc = sqlite3Malloc(10);                             \
    ఒకవేళ (!pTstAlloc) తిరిగి SQLITE_IOERR_NOMEM_BKPT;                  \
    sqlite3_free(pTstAlloc);                                         \
  }
#లేకపోతే
  #నిర్వచించు DO_OS_MALLOC_TEST(x)
#అయితే_ముగింపు

/*
** The following routines are convenience wrappers around methods
** of the sqlite3_file object.  This is mostly just syntactic sugar. All
** of this would be completely automatic if SQLite were coded using
** C++ instead of plain old C.
*/
శూన్య sqlite3OsClose(sqlite3_file *pId){
  ఒకవేళ( pId->pMethods ){
    pId->pMethods->xClose(pId);
    pId->pMethods = 0;
  }
}
పూర్ణాంకం sqlite3OsRead(sqlite3_file *id, శూన్య *pBuf, పూర్ణాంకం amt, i64 స్థానభ్రంశం){
  DO_OS_MALLOC_TEST(id);
  తిరిగి id->pMethods->xRead(id, pBuf, amt, స్థానభ్రంశం);
}
పూర్ణాంకం sqlite3OsWrite(sqlite3_file *id, స్థిరరాశి శూన్య *pBuf, పూర్ణాంకం amt, i64 స్థానభ్రంశం){
  DO_OS_MALLOC_TEST(id);
  తిరిగి id->pMethods->xWrite(id, pBuf, amt, స్థానభ్రంశం);
}
పూర్ణాంకం sqlite3OsTruncate(sqlite3_file *id, i64 కొలత){
  తిరిగి id->pMethods->xTruncate(id, కొలత);
}
పూర్ణాంకం sqlite3OsSync(sqlite3_file *id, పూర్ణాంకం flags){
  DO_OS_MALLOC_TEST(id);
  తిరిగి flags ? id->pMethods->xSync(id, flags) : SQLITE_OK;
}
పూర్ణాంకం sqlite3_Os_File_కొలత(sqlite3_file *id, i64 *p_కొలత){
  DO_OS_MALLOC_TEST(id);
  తిరిగి id->pMethods->x_File_కొలత(id, p_కొలత);
}
పూర్ణాంకం sqlite3_Os_తాళం(sqlite3_file *id, పూర్ణాంకం తాళం_Type){
  DO_OS_MALLOC_TEST(id);
  ప్రతిజ్ఞా( తాళం_Type>=SQLITE_తాళం_SHARED && తాళం_Type<=SQLITE_తాళం_EXCLUSIVE );
  తిరిగి id->pMethods->x_తాళం(id, తాళం_Type);
}
పూర్ణాంకం sqlite3OsUnlock(sqlite3_file *id, పూర్ణాంకం తాళం_Type){
  ప్రతిజ్ఞా( తాళం_Type==SQLITE_తాళం_NONE || తాళం_Type==SQLITE_తాళం_SHARED );
  తిరిగి id->pMethods->xUnlock(id, తాళం_Type);
}
పూర్ణాంకం sqlite3_Os_Check_Reserved_తాళం(sqlite3_file *id, పూర్ణాంకం *p_Res_బహిర్గతం){
  DO_OS_MALLOC_TEST(id);
  తిరిగి id->pMethods->x_Check_Reserved_తాళం(id, p_Res_బహిర్గతం);
}

/*
** Use sqlite3OsFileControl() when we are doing something that might fail
** and we need to know about the failures.  Use sqlite3OsFileControlHint()
** when simply tossing information over the wall to the VFS and we do not
** really care if the VFS receives and understands the information since it
** is only a hint and can be safely ignored.  The sqlite3OsFileControlHint()
** routine has no return value since the return value would be meaningless.
*/
పూర్ణాంకం sqlite3OsFileControl(sqlite3_file *id, పూర్ణాంకం op, శూన్య *pArg){
  ఒకవేళ( id->pMethods==0 ) తిరిగి SQLITE_NOTFOUND;
#నిర్వచితమైతే SQLITE_TEST
  ఒకవేళ( op!=SQLITE_FCNTL_COMMIT_PHASETWO
   && op!=SQLITE_FCNTL_తాళం_TIMEOUT
   && op!=SQLITE_FCNTL_CKPT_పూర్తి
   && op!=SQLITE_FCNTL_CKPT_ప్రారంభం
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
#అయితే_ముగింపు
  తిరిగి id->pMethods->xFileControl(id, op, pArg);
}
శూన్య sqlite3OsFileControlHint(sqlite3_file *id, పూర్ణాంకం op, శూన్య *pArg){
  ఒకవేళ( id->pMethods ) (శూన్య)id->pMethods->xFileControl(id, op, pArg);
}

పూర్ణాంకం sqlite3_Os_Sector_కొలత(sqlite3_file *id){
  పూర్ణాంకం (*x_Sector_కొలత)(sqlite3_file*) = id->pMethods->x_Sector_కొలత;
  తిరిగి (x_Sector_కొలత ? x_Sector_కొలత(id) : SQLITE_DEFAULT_SECTOR_కొలత);
}
పూర్ణాంకం sqlite3OsDeviceCharacteristics(sqlite3_file *id){
  ఒకవేళ( NEVER(id->pMethods==0) ) తిరిగి 0;
  తిరిగి id->pMethods->xDeviceCharacteristics(id);
}
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WAL
పూర్ణాంకం sqlite3_Os_Shm_తాళం(sqlite3_file *id, పూర్ణాంకం స్థానభ్రంశం, పూర్ణాంకం n, పూర్ణాంకం flags){
  తిరిగి id->pMethods->x_Shm_తాళం(id, స్థానభ్రంశం, n, flags);
}
శూన్య sqlite3OsShmBarrier(sqlite3_file *id){
  id->pMethods->xShmBarrier(id);
}
పూర్ణాంకం sqlite3OsShmUnmap(sqlite3_file *id, పూర్ణాంకం deleteFlag){
  తిరిగి id->pMethods->xShmUnmap(id, deleteFlag);
}
పూర్ణాంకం sqlite3OsShmMap(
  sqlite3_file *id,               /* Database file handle */
  పూర్ణాంకం i_పుట,
  పూర్ణాంకం pgsz,
  పూర్ణాంకం bExtend,                    /* True to extend file if necessary */
  శూన్య అస్థిర **pp              /* OUT: Pointer to mapping */
){
  DO_OS_MALLOC_TEST(id);
  తిరిగి id->pMethods->xShmMap(id, i_పుట, pgsz, bExtend, pp);
}
#అయితే_ముగింపు /* SQLITE_OMIT_WAL */

#ఒకవేళ SQLITE_MAX_MMAP_కొలత>0
/* The real implementation of xFetch and xUnfetch */
పూర్ణాంకం sqlite3OsFetch(sqlite3_file *id, i64 iOff, పూర్ణాంకం iAmt, శూన్య **pp){
  DO_OS_MALLOC_TEST(id);
  తిరిగి id->pMethods->xFetch(id, iOff, iAmt, pp);
}
పూర్ణాంకం sqlite3OsUnfetch(sqlite3_file *id, i64 iOff, శూన్య *p){
  తిరిగి id->pMethods->xUnfetch(id, iOff, p);
}
#లేకపోతే
/* No-op stubs to use when memory-mapped I/O is disabled */
పూర్ణాంకం sqlite3OsFetch(sqlite3_file *id, i64 iOff, పూర్ణాంకం iAmt, శూన్య **pp){
  *pp = 0;
  తిరిగి SQLITE_OK;
}
పూర్ణాంకం sqlite3OsUnfetch(sqlite3_file *id, i64 iOff, శూన్య *p){
  తిరిగి SQLITE_OK;
}
#అయితే_ముగింపు

/*
** The next group of routines are convenience wrappers around the
** VFS methods.
*/
పూర్ణాంకం sqlite3OsOpen(
  sqlite3_vfs *pVfs,
  స్థిరరాశి అక్షరం *z_మార్గం,
  sqlite3_file *pFile,
  పూర్ణాంకం flags,
  పూర్ణాంకం *p_Flags_బహిర్గతం
){
  పూర్ణాంకం rc;
  DO_OS_MALLOC_TEST(0);
  /* 0x87f7f is a mask of SQLITE_OPEN_ flags that are valid to be passed
  ** down into the VFS layer.  Some SQLITE_OPEN_ flags (for example,
  ** SQLITE_OPEN_FULLMUTEX or SQLITE_OPEN_SHAREDCACHE) are blocked before
  ** reaching the VFS. */
  ప్రతిజ్ఞా( z_మార్గం || (flags & SQLITE_OPEN_EXCLUSIVE) );
  rc = pVfs->xOpen(pVfs, z_మార్గం, pFile, flags & 0x1087f7f, p_Flags_బహిర్గతం);
  ప్రతిజ్ఞా( rc==SQLITE_OK || pFile->pMethods==0 );
  తిరిగి rc;
}
పూర్ణాంకం sqlite3OsDelete(sqlite3_vfs *pVfs, స్థిరరాశి అక్షరం *z_మార్గం, పూర్ణాంకం dirSync){
  DO_OS_MALLOC_TEST(0);
  ప్రతిజ్ఞా( dirSync==0 || dirSync==1 );
  తిరిగి pVfs->xDelete!=0 ? pVfs->xDelete(pVfs, z_మార్గం, dirSync) : SQLITE_OK;
}
పూర్ణాంకం sqlite3OsAccess(
  sqlite3_vfs *pVfs,
  స్థిరరాశి అక్షరం *z_మార్గం,
  పూర్ణాంకం flags,
  పూర్ణాంకం *p_Res_బహిర్గతం
){
  DO_OS_MALLOC_TEST(0);
  తిరిగి pVfs->xAccess(pVfs, z_మార్గం, flags, p_Res_బహిర్గతం);
}
పూర్ణాంకం sqlite3_Os_నిండిన_Pathname(
  sqlite3_vfs *pVfs,
  స్థిరరాశి అక్షరం *z_మార్గం,
  పూర్ణాంకం n_మార్గం_బహిర్గతం,
  అక్షరం *z_మార్గం_బహిర్గతం
){
  DO_OS_MALLOC_TEST(0);
  z_మార్గం_బహిర్గతం[0] = 0;
  తిరిగి pVfs->x_నిండిన_Pathname(pVfs, z_మార్గం, n_మార్గం_బహిర్గతం, z_మార్గం_బహిర్గతం);
}
#నిర్వచితం_కాకపోతే SQLITE_OMIT_ఎక్కించు_EXTENSION
శూన్య *sqlite3OsDlOpen(sqlite3_vfs *pVfs, స్థిరరాశి అక్షరం *z_మార్గం){
  ప్రతిజ్ఞా( z_మార్గం!=0 );
  ప్రతిజ్ఞా( అక్షరశ్రేణి_పొడవు(z_మార్గం)<=SQLITE_MAX_PATHLEN );  /* tag-20210611-1 */
  తిరిగి pVfs->xDlOpen(pVfs, z_మార్గం);
}
శూన్య sqlite3OsDlError(sqlite3_vfs *pVfs, పూర్ణాంకం nByte, అక్షరం *z_Buf_బహిర్గతం){
  pVfs->xDlError(pVfs, nByte, z_Buf_బహిర్గతం);
}
శూన్య (*sqlite3OsDlSym(sqlite3_vfs *pVfs, శూన్య *pHdle, స్థిరరాశి అక్షరం *zSym))(శూన్య){
  తిరిగి pVfs->xDlSym(pVfs, pHdle, zSym);
}
శూన్య sqlite3OsDlClose(sqlite3_vfs *pVfs, శూన్య *pHandle){
  pVfs->xDlClose(pVfs, pHandle);
}
#అయితే_ముగింపు /* SQLITE_OMIT_LOAD_EXTENSION */
పూర్ణాంకం sqlite3OsRandomness(sqlite3_vfs *pVfs, పూర్ణాంకం nByte, అక్షరం *z_Buf_బహిర్గతం){
  ఒకవేళ( sqlite3Config.i_Prng_విత్తనం ){
    memset(z_Buf_బహిర్గతం, 0, nByte);
    ఒకవేళ( ALWAYS(nByte>(గుర్తుగల)పరిమాణం(గుర్తులేని)) ) nByte = పరిమాణం(గుర్తులేని పూర్ణాంకం);
    జ్ఞాపకం_నకలు(z_Buf_బహిర్గతం, &sqlite3Config.i_Prng_విత్తనం, nByte);
    తిరిగి SQLITE_OK;
  }లేకపోతే{
    తిరిగి pVfs->xRandomness(pVfs, nByte, z_Buf_బహిర్గతం);
  }

}
పూర్ణాంకం sqlite3OsSleep(sqlite3_vfs *pVfs, పూర్ణాంకం nMicro){
  తిరిగి pVfs->xSleep(pVfs, nMicro);
}
పూర్ణాంకం sqlite3_Os_Get_చివరి_Error(sqlite3_vfs *pVfs){
  తిరిగి pVfs->x_Get_చివరి_Error ? pVfs->x_Get_చివరి_Error(pVfs, 0, 0) : 0;
}
పూర్ణాంకం sqlite3_Os_Current_సమయం_Int64(sqlite3_vfs *pVfs, sqlite3_int64 *p_సమయం_బహిర్గతం){
  పూర్ణాంకం rc;
  /* IMPLEMENTATION-OF: R-49045-42493 SQLite will use the xCurrentTimeInt64()
  ** method to get the current date and time if that method is available
  ** (if iVersion is 2 or greater and the function pointer is not NULL) and
  ** will fall back to xCurrentTime() if xCurrentTimeInt64() is
  ** unavailable.
  */
  ఒకవేళ( pVfs->i_సంస్కరణ>=2 && pVfs->x_Current_సమయం_Int64 ){
    rc = pVfs->x_Current_సమయం_Int64(pVfs, p_సమయం_బహిర్గతం);
  }లేకపోతే{
    ద్విగుణ r;
    rc = pVfs->x_Current_సమయం(pVfs, &r);
    *p_సమయం_బహిర్గతం = sqlite3_Real_గమ్యం_I64(r*86400000.0);
  }
  తిరిగి rc;
}

పూర్ణాంకం sqlite3OsOpenMalloc(
  sqlite3_vfs *pVfs,
  స్థిరరాశి అక్షరం *zFile,
  sqlite3_file **ppFile,
  పూర్ణాంకం flags,
  పూర్ణాంకం *p_బహిర్గతం_Flags
){
  పూర్ణాంకం rc;
  sqlite3_file *pFile;
  pFile = (sqlite3_file *)sqlite3MallocZero(pVfs->szOsFile);
  ఒకవేళ( pFile ){
    rc = sqlite3OsOpen(pVfs, zFile, pFile, flags, p_బహిర్గతం_Flags);
    ఒకవేళ( rc!=SQLITE_OK ){
      sqlite3_free(pFile);
      *ppFile = 0;
    }లేకపోతే{
      *ppFile = pFile;
    }
  }లేకపోతే{
    *ppFile = 0;
    rc = SQLITE_NOMEM_BKPT;
  }
  ప్రతిజ్ఞా( *ppFile!=0 || rc!=SQLITE_OK );
  తిరిగి rc;
}
శూన్య sqlite3OsCloseFree(sqlite3_file *pFile){
  ప్రతిజ్ఞా( pFile );
  sqlite3OsClose(pFile);
  sqlite3_free(pFile);
}

/*
** This function is a wrapper around the OS specific implementation of
** sqlite3_os_init(). The purpose of the wrapper is to provide the
** ability to simulate a malloc failure, so that the handling of an
** error in sqlite3_os_init() by the upper layers can be tested.
*/
పూర్ణాంకం sqlite3OsInit(శూన్య){
  శూన్య *p = sqlite3_malloc(10);
  ఒకవేళ( p==0 ) తిరిగి SQLITE_NOMEM_BKPT;
  sqlite3_free(p);
  తిరిగి sqlite3_os_init();
}

/*
** The list of all registered VFS implementations.
*/
స్థిర sqlite3_vfs * SQLITE_WSD vfsList = 0;
#నిర్వచించు vfsList GLOBAL(sqlite3_vfs *, vfsList)

/*
** Locate a VFS by name.  If no name is given, simply return the
** first VFS on the list.
*/
sqlite3_vfs *sqlite3_vfs_కనుగొను(స్థిరరాశి అక్షరం *zVfs){
  sqlite3_vfs *pVfs = 0;
#ఒకవేళ SQLITE_THREADSAFE
  sqlite3_mutex *mutex;
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOINIT
  పూర్ణాంకం rc = sqlite3_initialize();
  ఒకవేళ( rc ) తిరిగి 0;
#అయితే_ముగింపు
#ఒకవేళ SQLITE_THREADSAFE
  mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
#అయితే_ముగింపు
  sqlite3_mutex_enter(mutex);
  కొరకు(pVfs = vfsList; pVfs; pVfs=pVfs->p_తదుపరి){
    ఒకవేళ( zVfs==0 ) ఆపు;
    ఒకవేళ( strcmp(zVfs, pVfs->zName)==0 ) ఆపు;
  }
  sqlite3_mutex_leave(mutex);
  తిరిగి pVfs;
}

/*
** Unlink a VFS from the linked list
*/
స్థిర శూన్య vfsUnlink(sqlite3_vfs *pVfs){
  ప్రతిజ్ఞా( sqlite3_mutex_held(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN)) );
  ఒకవేళ( pVfs==0 ){
    /* No-op */
  }లేకపోతే ఒకవేళ( vfsList==pVfs ){
    vfsList = pVfs->p_తదుపరి;
  }లేకపోతే ఒకవేళ( vfsList ){
    sqlite3_vfs *p = vfsList;
    అయితే( p->p_తదుపరి && p->p_తదుపరి!=pVfs ){
      p = p->p_తదుపరి;
    }
    ఒకవేళ( p->p_తదుపరి==pVfs ){
      p->p_తదుపరి = pVfs->p_తదుపరి;
    }
  }
}

/*
** Register a VFS with the system.  It is harmless to register the same
** VFS multiple times.  The new VFS becomes the default if makeDflt is
** true.
*/
పూర్ణాంకం sqlite3_vfs_రిజిస్టరు(sqlite3_vfs *pVfs, పూర్ణాంకం makeDflt){
  MUTEX_LOGIC(sqlite3_mutex *mutex;)
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOINIT
  పూర్ణాంకం rc = sqlite3_initialize();
  ఒకవేళ( rc ) తిరిగి rc;
#అయితే_ముగింపు
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( pVfs==0 ) తిరిగి SQLITE_MISUSE_BKPT;
#అయితే_ముగింపు

  MUTEX_LOGIC( mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  sqlite3_mutex_enter(mutex);
  vfsUnlink(pVfs);
  ఒకవేళ( makeDflt || vfsList==0 ){
    pVfs->p_తదుపరి = vfsList;
    vfsList = pVfs;
  }లేకపోతే{
    pVfs->p_తదుపరి = vfsList->p_తదుపరి;
    vfsList->p_తదుపరి = pVfs;
  }
  ప్రతిజ్ఞా(vfsList);
  sqlite3_mutex_leave(mutex);
  తిరిగి SQLITE_OK;
}

/*
** Unregister a VFS so that it is no longer accessible.
*/
పూర్ణాంకం sqlite3_vfs_unregister(sqlite3_vfs *pVfs){
  MUTEX_LOGIC(sqlite3_mutex *mutex;)
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOINIT
  పూర్ణాంకం rc = sqlite3_initialize();
  ఒకవేళ( rc ) తిరిగి rc;
#అయితే_ముగింపు
  MUTEX_LOGIC( mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  sqlite3_mutex_enter(mutex);
  vfsUnlink(pVfs);
  sqlite3_mutex_leave(mutex);
  తిరిగి SQLITE_OK;
}
