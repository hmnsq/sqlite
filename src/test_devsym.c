/*
** 2008 Jan 22
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
** This file contains code that modified the OS layer in order to simulate
** different device types (by overriding the return values of the 
** xDeviceCharacteristics() and xSectorSize() methods).
*/
#ఒకవేళ SQLITE_TEST          /* This file is used for testing only */

#అన్తర్భావ "sqlite3.h"
#అన్తర్భావ "sqliteInt.h"

/*
** Maximum pathname length supported by the devsym backend.
*/
#నిర్వచించు DEVSYM_MAX_PATHNAME 512

/*
** Name used to identify this VFS.
*/
#నిర్వచించు DEVSYM_VFS_NAME "devsym"
#నిర్వచించు WRITECRASH_NAME "writecrash"

రకనిర్వచనం నిర్మాణం devsym_file devsym_file;
నిర్మాణం devsym_file {
  sqlite3_file పునాది;
  sqlite3_file *pReal;
};

/*
** Method declarations for devsym_file.
*/
స్థిర పూర్ణాంకం devsymClose(sqlite3_file*);
స్థిర పూర్ణాంకం devsymRead(sqlite3_file*, శూన్య*, పూర్ణాంకం iAmt, sqlite3_int64 iOfst);
స్థిర పూర్ణాంకం devsymWrite(sqlite3_file*,స్థిరరాశి శూన్య*,పూర్ణాంకం iAmt, sqlite3_int64 iOfst);
స్థిర పూర్ణాంకం devsymTruncate(sqlite3_file*, sqlite3_int64 కొలత);
స్థిర పూర్ణాంకం devsymSync(sqlite3_file*, పూర్ణాంకం flags);
స్థిర పూర్ణాంకం devsym_File_కొలత(sqlite3_file*, sqlite3_int64 *p_కొలత);
స్థిర పూర్ణాంకం devsym_తాళం(sqlite3_file*, పూర్ణాంకం);
స్థిర పూర్ణాంకం devsymUnlock(sqlite3_file*, పూర్ణాంకం);
స్థిర పూర్ణాంకం devsym_Check_Reserved_తాళం(sqlite3_file*, పూర్ణాంకం *);
స్థిర పూర్ణాంకం devsymFileControl(sqlite3_file*, పూర్ణాంకం op, శూన్య *pArg);
స్థిర పూర్ణాంకం devsym_Sector_కొలత(sqlite3_file*);
స్థిర పూర్ణాంకం devsymDeviceCharacteristics(sqlite3_file*);
స్థిర పూర్ణాంకం devsym_Shm_తాళం(sqlite3_file*,పూర్ణాంకం,పూర్ణాంకం,పూర్ణాంకం);
స్థిర పూర్ణాంకం devsymShmMap(sqlite3_file*,పూర్ణాంకం,పూర్ణాంకం,పూర్ణాంకం, శూన్య అస్థిర **);
స్థిర శూన్య devsymShmBarrier(sqlite3_file*);
స్థిర పూర్ణాంకం devsymShmUnmap(sqlite3_file*,పూర్ణాంకం);

/*
** Method declarations for devsym_vfs.
*/
స్థిర పూర్ణాంకం devsymOpen(sqlite3_vfs*, స్థిరరాశి అక్షరం *, sqlite3_file*, పూర్ణాంకం , పూర్ణాంకం *);
స్థిర పూర్ణాంకం devsymDelete(sqlite3_vfs*, స్థిరరాశి అక్షరం *zName, పూర్ణాంకం syncDir);
స్థిర పూర్ణాంకం devsymAccess(sqlite3_vfs*, స్థిరరాశి అక్షరం *zName, పూర్ణాంకం flags, పూర్ణాంకం *);
స్థిర పూర్ణాంకం devsym_నిండిన_Pathname(sqlite3_vfs*, స్థిరరాశి అక్షరం *zName, పూర్ణాంకం, అక్షరం *z_బహిర్గతం);
#నిర్వచితం_కాకపోతే SQLITE_OMIT_ఎక్కించు_EXTENSION
స్థిర శూన్య *devsymDlOpen(sqlite3_vfs*, స్థిరరాశి అక్షరం *zFilename);
స్థిర శూన్య devsymDlError(sqlite3_vfs*, పూర్ణాంకం nByte, అక్షరం *z_Err_సందేశం);
స్థిర శూన్య (*devsymDlSym(sqlite3_vfs*,శూన్య*, స్థిరరాశి అక్షరం *z_చిహ్నం))(శూన్య);
స్థిర శూన్య devsymDlClose(sqlite3_vfs*, శూన్య*);
#అయితే_ముగింపు /* SQLITE_OMIT_LOAD_EXTENSION */
స్థిర పూర్ణాంకం devsymRandomness(sqlite3_vfs*, పూర్ణాంకం nByte, అక్షరం *z_బహిర్గతం);
స్థిర పూర్ణాంకం devsymSleep(sqlite3_vfs*, పూర్ణాంకం microseconds);
స్థిర పూర్ణాంకం devsym_Current_సమయం(sqlite3_vfs*, ద్విగుణ*);

నిర్మాణం DevsymGlobal {
  sqlite3_vfs *pVfs;
  పూర్ణాంకం iDeviceChar;
  పూర్ణాంకం i_Sector_కొలత;
  పూర్ణాంకం nWriteCrash;
};
నిర్మాణం DevsymGlobal g = {0, 0, 512, 0};

/*
** Close an devsym-file.
*/
స్థిర పూర్ణాంకం devsymClose(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  sqlite3OsClose(p->pReal);
  తిరిగి SQLITE_OK;
}

/*
** Read data from an devsym-file.
*/
స్థిర పూర్ణాంకం devsymRead(
  sqlite3_file *pFile, 
  శూన్య *zBuf, 
  పూర్ణాంకం iAmt, 
  sqlite_int64 iOfst
){
  devsym_file *p = (devsym_file *)pFile;
  తిరిగి sqlite3OsRead(p->pReal, zBuf, iAmt, iOfst);
}

/*
** Write data to an devsym-file.
*/
స్థిర పూర్ణాంకం devsymWrite(
  sqlite3_file *pFile, 
  స్థిరరాశి శూన్య *zBuf, 
  పూర్ణాంకం iAmt, 
  sqlite_int64 iOfst
){
  devsym_file *p = (devsym_file *)pFile;
  తిరిగి sqlite3OsWrite(p->pReal, zBuf, iAmt, iOfst);
}

/*
** Truncate an devsym-file.
*/
స్థిర పూర్ణాంకం devsymTruncate(sqlite3_file *pFile, sqlite_int64 కొలత){
  devsym_file *p = (devsym_file *)pFile;
  తిరిగి sqlite3OsTruncate(p->pReal, కొలత);
}

/*
** Sync an devsym-file.
*/
స్థిర పూర్ణాంకం devsymSync(sqlite3_file *pFile, పూర్ణాంకం flags){
  devsym_file *p = (devsym_file *)pFile;
  తిరిగి sqlite3OsSync(p->pReal, flags);
}

/*
** Return the current file-size of an devsym-file.
*/
స్థిర పూర్ణాంకం devsym_File_కొలత(sqlite3_file *pFile, sqlite_int64 *p_కొలత){
  devsym_file *p = (devsym_file *)pFile;
  తిరిగి sqlite3_Os_File_కొలత(p->pReal, p_కొలత);
}

/*
** Lock an devsym-file.
*/
స్థిర పూర్ణాంకం devsym_తాళం(sqlite3_file *pFile, పూర్ణాంకం e_తాళం){
  devsym_file *p = (devsym_file *)pFile;
  తిరిగి sqlite3_Os_తాళం(p->pReal, e_తాళం);
}

/*
** Unlock an devsym-file.
*/
స్థిర పూర్ణాంకం devsymUnlock(sqlite3_file *pFile, పూర్ణాంకం e_తాళం){
  devsym_file *p = (devsym_file *)pFile;
  తిరిగి sqlite3OsUnlock(p->pReal, e_తాళం);
}

/*
** Check if another file-handle holds a RESERVED lock on an devsym-file.
*/
స్థిర పూర్ణాంకం devsym_Check_Reserved_తాళం(sqlite3_file *pFile, పూర్ణాంకం *p_Res_బహిర్గతం){
  devsym_file *p = (devsym_file *)pFile;
  తిరిగి sqlite3_Os_Check_Reserved_తాళం(p->pReal, p_Res_బహిర్గతం);
}

/*
** File control method. For custom operations on an devsym-file.
*/
స్థిర పూర్ణాంకం devsymFileControl(sqlite3_file *pFile, పూర్ణాంకం op, శూన్య *pArg){
  devsym_file *p = (devsym_file *)pFile;
  తిరిగి sqlite3OsFileControl(p->pReal, op, pArg);
}

/*
** Return the sector-size in bytes for an devsym-file.
*/
స్థిర పూర్ణాంకం devsym_Sector_కొలత(sqlite3_file *pFile){
  తిరిగి g.i_Sector_కొలత;
}

/*
** Return the device characteristic flags supported by an devsym-file.
*/
స్థిర పూర్ణాంకం devsymDeviceCharacteristics(sqlite3_file *pFile){
  తిరిగి g.iDeviceChar;
}

/*
** Shared-memory methods are all pass-throughs.
*/
స్థిర పూర్ణాంకం devsym_Shm_తాళం(sqlite3_file *pFile, పూర్ణాంకం ofst, పూర్ణాంకం n, పూర్ణాంకం flags){
  devsym_file *p = (devsym_file *)pFile;
  తిరిగి p->pReal->pMethods->x_Shm_తాళం(p->pReal, ofst, n, flags);
}
స్థిర పూర్ణాంకం devsymShmMap(
  sqlite3_file *pFile, 
  పూర్ణాంకం iRegion, 
  పూర్ణాంకం szRegion, 
  పూర్ణాంకం isWrite, 
  శూన్య అస్థిర **pp
){
  devsym_file *p = (devsym_file *)pFile;
  తిరిగి p->pReal->pMethods->xShmMap(p->pReal, iRegion, szRegion, isWrite, pp);
}
స్థిర శూన్య devsymShmBarrier(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  p->pReal->pMethods->xShmBarrier(p->pReal);
}
స్థిర పూర్ణాంకం devsymShmUnmap(sqlite3_file *pFile, పూర్ణాంకం delFlag){
  devsym_file *p = (devsym_file *)pFile;
  తిరిగి p->pReal->pMethods->xShmUnmap(p->pReal, delFlag);
}



/*
** Open an devsym file handle.
*/
స్థిర పూర్ణాంకం devsymOpen(
  sqlite3_vfs *pVfs,
  స్థిరరాశి అక్షరం *zName,
  sqlite3_file *pFile,
  పూర్ణాంకం flags,
  పూర్ణాంకం *p_బహిర్గతం_Flags
){
స్థిర sqlite3_io_methods devsym_io_methods = {
  2,                                /* iVersion */
  devsymClose,                      /* xClose */
  devsymRead,                       /* xRead */
  devsymWrite,                      /* xWrite */
  devsymTruncate,                   /* xTruncate */
  devsymSync,                       /* xSync */
  devsym_File_కొలత,                   /* xFileSize */
  devsym_తాళం,                       /* xLock */
  devsymUnlock,                     /* xUnlock */
  devsym_Check_Reserved_తాళం,          /* xCheckReservedLock */
  devsymFileControl,                /* xFileControl */
  devsym_Sector_కొలత,                 /* xSectorSize */
  devsymDeviceCharacteristics,      /* xDeviceCharacteristics */
  devsymShmMap,                     /* xShmMap */
  devsym_Shm_తాళం,                    /* xShmLock */
  devsymShmBarrier,                 /* xShmBarrier */
  devsymShmUnmap                    /* xShmUnmap */
};

  పూర్ణాంకం rc;
  devsym_file *p = (devsym_file *)pFile;
  p->pReal = (sqlite3_file *)&p[1];
  rc = sqlite3OsOpen(g.pVfs, zName, p->pReal, flags, p_బహిర్గతం_Flags);
  ఒకవేళ( p->pReal->pMethods ){
    pFile->pMethods = &devsym_io_methods;
  }
  తిరిగి rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
స్థిర పూర్ణాంకం devsymDelete(sqlite3_vfs *pVfs, స్థిరరాశి అక్షరం *z_మార్గం, పూర్ణాంకం dirSync){
  తిరిగి sqlite3OsDelete(g.pVfs, z_మార్గం, dirSync);
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
స్థిర పూర్ణాంకం devsymAccess(
  sqlite3_vfs *pVfs, 
  స్థిరరాశి అక్షరం *z_మార్గం, 
  పూర్ణాంకం flags, 
  పూర్ణాంకం *p_Res_బహిర్గతం
){
  తిరిగి sqlite3OsAccess(g.pVfs, z_మార్గం, flags, p_Res_బహిర్గతం);
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (DEVSYM_MAX_PATHNAME+1) bytes.
*/
స్థిర పూర్ణాంకం devsym_నిండిన_Pathname(
  sqlite3_vfs *pVfs, 
  స్థిరరాశి అక్షరం *z_మార్గం, 
  పూర్ణాంకం n_బహిర్గతం, 
  అక్షరం *z_బహిర్గతం
){
  తిరిగి sqlite3_Os_నిండిన_Pathname(g.pVfs, z_మార్గం, n_బహిర్గతం, z_బహిర్గతం);
}

#నిర్వచితం_కాకపోతే SQLITE_OMIT_ఎక్కించు_EXTENSION
/*
** Open the dynamic library located at zPath and return a handle.
*/
స్థిర శూన్య *devsymDlOpen(sqlite3_vfs *pVfs, స్థిరరాశి అక్షరం *z_మార్గం){
  తిరిగి sqlite3OsDlOpen(g.pVfs, z_మార్గం);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
స్థిర శూన్య devsymDlError(sqlite3_vfs *pVfs, పూర్ణాంకం nByte, అక్షరం *z_Err_సందేశం){
  sqlite3OsDlError(g.pVfs, nByte, z_Err_సందేశం);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
స్థిర శూన్య (*devsymDlSym(sqlite3_vfs *pVfs, శూన్య *p, స్థిరరాశి అక్షరం *zSym))(శూన్య){
  తిరిగి sqlite3OsDlSym(g.pVfs, p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
స్థిర శూన్య devsymDlClose(sqlite3_vfs *pVfs, శూన్య *pHandle){
  sqlite3OsDlClose(g.pVfs, pHandle);
}
#అయితే_ముగింపు /* SQLITE_OMIT_LOAD_EXTENSION */

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
స్థిర పూర్ణాంకం devsymRandomness(sqlite3_vfs *pVfs, పూర్ణాంకం nByte, అక్షరం *z_Buf_బహిర్గతం){
  తిరిగి sqlite3OsRandomness(g.pVfs, nByte, z_Buf_బహిర్గతం);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
స్థిర పూర్ణాంకం devsymSleep(sqlite3_vfs *pVfs, పూర్ణాంకం nMicro){
  తిరిగి sqlite3OsSleep(g.pVfs, nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
స్థిర పూర్ణాంకం devsym_Current_సమయం(sqlite3_vfs *pVfs, ద్విగుణ *p_సమయం_బహిర్గతం){
  తిరిగి g.pVfs->x_Current_సమయం(g.pVfs, p_సమయం_బహిర్గతం);
}

/*
** Return the sector-size in bytes for an writecrash-file.
*/
స్థిర పూర్ణాంకం writecrash_Sector_కొలత(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  తిరిగి sqlite3_Os_Sector_కొలత(p->pReal);
}

/*
** Return the device characteristic flags supported by an writecrash-file.
*/
స్థిర పూర్ణాంకం writecrashDeviceCharacteristics(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  తిరిగి sqlite3OsDeviceCharacteristics(p->pReal);
}

/*
** Write data to an writecrash-file.
*/
స్థిర పూర్ణాంకం writecrashWrite(
  sqlite3_file *pFile, 
  స్థిరరాశి శూన్య *zBuf, 
  పూర్ణాంకం iAmt, 
  sqlite_int64 iOfst
){
  devsym_file *p = (devsym_file *)pFile;
  ఒకవేళ( g.nWriteCrash>0 ){
    g.nWriteCrash--;
    ఒకవేళ( g.nWriteCrash==0 ) abort();
  }
  తిరిగి sqlite3OsWrite(p->pReal, zBuf, iAmt, iOfst);
}

/*
** Open an writecrash file handle.
*/
స్థిర పూర్ణాంకం writecrashOpen(
  sqlite3_vfs *pVfs,
  స్థిరరాశి అక్షరం *zName,
  sqlite3_file *pFile,
  పూర్ణాంకం flags,
  పూర్ణాంకం *p_బహిర్గతం_Flags
){
స్థిర sqlite3_io_methods writecrash_io_methods = {
  2,                                /* iVersion */
  devsymClose,                      /* xClose */
  devsymRead,                       /* xRead */
  writecrashWrite,                  /* xWrite */
  devsymTruncate,                   /* xTruncate */
  devsymSync,                       /* xSync */
  devsym_File_కొలత,                   /* xFileSize */
  devsym_తాళం,                       /* xLock */
  devsymUnlock,                     /* xUnlock */
  devsym_Check_Reserved_తాళం,          /* xCheckReservedLock */
  devsymFileControl,                /* xFileControl */
  writecrash_Sector_కొలత,             /* xSectorSize */
  writecrashDeviceCharacteristics,  /* xDeviceCharacteristics */
  devsymShmMap,                     /* xShmMap */
  devsym_Shm_తాళం,                    /* xShmLock */
  devsymShmBarrier,                 /* xShmBarrier */
  devsymShmUnmap                    /* xShmUnmap */
};

  పూర్ణాంకం rc;
  devsym_file *p = (devsym_file *)pFile;
  p->pReal = (sqlite3_file *)&p[1];
  rc = sqlite3OsOpen(g.pVfs, zName, p->pReal, flags, p_బహిర్గతం_Flags);
  ఒకవేళ( p->pReal->pMethods ){
    pFile->pMethods = &writecrash_io_methods;
  }
  తిరిగి rc;
}

స్థిర sqlite3_vfs devsym_vfs = {
  2,                     /* iVersion */
  పరిమాణం(devsym_file),      /* szOsFile */
  DEVSYM_MAX_PATHNAME,      /* mxPathname */
  0,                     /* pNext */
  DEVSYM_VFS_NAME,          /* zName */
  0,                     /* pAppData */
  devsymOpen,               /* xOpen */
  devsymDelete,             /* xDelete */
  devsymAccess,             /* xAccess */
  devsym_నిండిన_Pathname,       /* xFullPathname */
#నిర్వచితం_కాకపోతే SQLITE_OMIT_ఎక్కించు_EXTENSION
  devsymDlOpen,             /* xDlOpen */
  devsymDlError,            /* xDlError */
  devsymDlSym,              /* xDlSym */
  devsymDlClose,            /* xDlClose */
#లేకపోతే
  0,                        /* xDlOpen */
  0,                        /* xDlError */
  0,                        /* xDlSym */
  0,                        /* xDlClose */
#అయితే_ముగింపు /* SQLITE_OMIT_LOAD_EXTENSION */
  devsymRandomness,         /* xRandomness */
  devsymSleep,              /* xSleep */
  devsym_Current_సమయం,        /* xCurrentTime */
  0,                        /* xGetLastError */
  0                         /* xCurrentTimeInt64 */
};

స్థిర sqlite3_vfs writecrash_vfs = {
  2,                     /* iVersion */
  పరిమాణం(devsym_file),      /* szOsFile */
  DEVSYM_MAX_PATHNAME,      /* mxPathname */
  0,                     /* pNext */
  WRITECRASH_NAME,          /* zName */
  0,                     /* pAppData */
  writecrashOpen,           /* xOpen */
  devsymDelete,             /* xDelete */
  devsymAccess,             /* xAccess */
  devsym_నిండిన_Pathname,       /* xFullPathname */
#నిర్వచితం_కాకపోతే SQLITE_OMIT_ఎక్కించు_EXTENSION
  devsymDlOpen,             /* xDlOpen */
  devsymDlError,            /* xDlError */
  devsymDlSym,              /* xDlSym */
  devsymDlClose,            /* xDlClose */
#లేకపోతే
  0,                        /* xDlOpen */
  0,                        /* xDlError */
  0,                        /* xDlSym */
  0,                        /* xDlClose */
#అయితే_ముగింపు /* SQLITE_OMIT_LOAD_EXTENSION */
  devsymRandomness,         /* xRandomness */
  devsymSleep,              /* xSleep */
  devsym_Current_సమయం,        /* xCurrentTime */
  0,                        /* xGetLastError */
  0                         /* xCurrentTimeInt64 */
};


/*
** This procedure registers the devsym vfs with SQLite. If the argument is
** true, the devsym vfs becomes the new default vfs. It is the only publicly
** available function in this file.
*/
శూన్య devsym_రిజిస్టరు(పూర్ణాంకం iDeviceChar, పూర్ణాంకం i_Sector_కొలత){

  ఒకవేళ( g.pVfs==0 ){
    g.pVfs = sqlite3_vfs_కనుగొను(0);
    devsym_vfs.szOsFile += g.pVfs->szOsFile;
    writecrash_vfs.szOsFile += g.pVfs->szOsFile;
    sqlite3_vfs_రిజిస్టరు(&devsym_vfs, 0);
    sqlite3_vfs_రిజిస్టరు(&writecrash_vfs, 0);
  }
  ఒకవేళ( iDeviceChar>=0 ){
    g.iDeviceChar = iDeviceChar;
  }లేకపోతే{
    g.iDeviceChar = 0;
  }
  ఒకవేళ( i_Sector_కొలత>=0 ){
    g.i_Sector_కొలత = i_Sector_కొలత;
  }లేకపోతే{
    g.i_Sector_కొలత = 512;
  }
}

శూన్య devsym_unregister(){
  sqlite3_vfs_unregister(&devsym_vfs);
  sqlite3_vfs_unregister(&writecrash_vfs);
  g.pVfs = 0;
  g.iDeviceChar = 0;
  g.i_Sector_కొలత = 0;
}

శూన్య devsym_crash_on_write(పూర్ణాంకం nWrite){
  ఒకవేళ( g.pVfs==0 ){
    g.pVfs = sqlite3_vfs_కనుగొను(0);
    devsym_vfs.szOsFile += g.pVfs->szOsFile;
    writecrash_vfs.szOsFile += g.pVfs->szOsFile;
    sqlite3_vfs_రిజిస్టరు(&devsym_vfs, 0);
    sqlite3_vfs_రిజిస్టరు(&writecrash_vfs, 0);
  }
  g.nWriteCrash = nWrite;
}

#అయితే_ముగింపు
