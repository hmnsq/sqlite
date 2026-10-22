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
#ଯଦି SQLITE_TEST          /* This file is used for testing only */

#ଅନ୍ତର୍ଭାଵ "sqlite3.h"
#ଅନ୍ତର୍ଭାଵ "sqliteInt.h"

/*
** Maximum pathname length supported by the devsym backend.
*/
#define DEVSYM_MAX_PATHNAME 512

/*
** Name used to identify this VFS.
*/
#define DEVSYM_VFS_NAME "devsym"
#define WRITECRASH_NAME "writecrash"

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା devsym_file devsym_file;
ସଂରଚନା devsym_file {
  sqlite3_file base;
  sqlite3_file *pReal;
};

/*
** Method declarations for devsym_file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା devsymClose(sqlite3_file*);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା devsymRead(sqlite3_file*, ଶୂନ୍ୟ*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iAmt, sqlite3_int64 iOfst);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା devsymWrite(sqlite3_file*,ଅଚଳ ଶୂନ୍ୟ*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା iAmt, sqlite3_int64 iOfst);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା devsymTruncate(sqlite3_file*, sqlite3_int64 size);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା devsymSync(sqlite3_file*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା devsymFileSize(sqlite3_file*, sqlite3_int64 *pSize);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା devsymLock(sqlite3_file*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା devsymUnlock(sqlite3_file*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା devsymCheckReservedLock(sqlite3_file*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା devsymFileControl(sqlite3_file*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା op, ଶୂନ୍ୟ *pArg);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା devsymSectorSize(sqlite3_file*);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା devsymDeviceCharacteristics(sqlite3_file*);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା devsymShmLock(sqlite3_file*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା devsymShmMap(sqlite3_file*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ପୂର୍ଣ୍ଣସଂଖ୍ୟା, ଶୂନ୍ୟ ଅସ୍ଥିର **);
ସ୍ଥିର ଶୂନ୍ୟ devsymShmBarrier(sqlite3_file*);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା devsymShmUnmap(sqlite3_file*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା);

/*
** Method declarations for devsym_vfs.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା devsymOpen(sqlite3_vfs*, ଅଚଳ ଅକ୍ଷର *, sqlite3_file*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା , ପୂର୍ଣ୍ଣସଂଖ୍ୟା *);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା devsymDelete(sqlite3_vfs*, ଅଚଳ ଅକ୍ଷର *zName, ପୂର୍ଣ୍ଣସଂଖ୍ୟା syncDir);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା devsymAccess(sqlite3_vfs*, ଅଚଳ ଅକ୍ଷର *zName, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା devsymFullPathname(sqlite3_vfs*, ଅଚଳ ଅକ୍ଷର *zName, ପୂର୍ଣ୍ଣସଂଖ୍ୟା, ଅକ୍ଷର *zOut);
#ifndef SQLITE_OMIT_LOAD_EXTENSION
ସ୍ଥିର ଶୂନ୍ୟ *devsymDlOpen(sqlite3_vfs*, ଅଚଳ ଅକ୍ଷର *zFilename);
ସ୍ଥିର ଶୂନ୍ୟ devsymDlError(sqlite3_vfs*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nByte, ଅକ୍ଷର *z_Err_ସନ୍ଦେଶ);
ସ୍ଥିର ଶୂନ୍ୟ (*devsymDlSym(sqlite3_vfs*,ଶୂନ୍ୟ*, ଅଚଳ ଅକ୍ଷର *z_ପ୍ରତୀକ))(ଶୂନ୍ୟ);
ସ୍ଥିର ଶୂନ୍ୟ devsymDlClose(sqlite3_vfs*, ଶୂନ୍ୟ*);
#endif /* SQLITE_OMIT_LOAD_EXTENSION */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା devsymRandomness(sqlite3_vfs*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nByte, ଅକ୍ଷର *zOut);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା devsymSleep(sqlite3_vfs*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା microseconds);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା devsymCurrentTime(sqlite3_vfs*, ଦ୍ୱିଗୁଣ*);

ସଂରଚନା DevsymGlobal {
  sqlite3_vfs *pVfs;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDeviceChar;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iSectorSize;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nWriteCrash;
};
ସଂରଚନା DevsymGlobal g = {0, 0, 512, 0};

/*
** Close an devsym-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା devsymClose(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  sqlite3OsClose(p->pReal);
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Read data from an devsym-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା devsymRead(
  sqlite3_file *pFile, 
  ଶୂନ୍ୟ *zBuf, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iAmt, 
  sqlite_int64 iOfst
){
  devsym_file *p = (devsym_file *)pFile;
  ଫେରନ୍ତୁ sqlite3OsRead(p->pReal, zBuf, iAmt, iOfst);
}

/*
** Write data to an devsym-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା devsymWrite(
  sqlite3_file *pFile, 
  ଅଚଳ ଶୂନ୍ୟ *zBuf, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iAmt, 
  sqlite_int64 iOfst
){
  devsym_file *p = (devsym_file *)pFile;
  ଫେରନ୍ତୁ sqlite3OsWrite(p->pReal, zBuf, iAmt, iOfst);
}

/*
** Truncate an devsym-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା devsymTruncate(sqlite3_file *pFile, sqlite_int64 size){
  devsym_file *p = (devsym_file *)pFile;
  ଫେରନ୍ତୁ sqlite3OsTruncate(p->pReal, size);
}

/*
** Sync an devsym-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା devsymSync(sqlite3_file *pFile, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags){
  devsym_file *p = (devsym_file *)pFile;
  ଫେରନ୍ତୁ sqlite3OsSync(p->pReal, flags);
}

/*
** Return the current file-size of an devsym-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା devsymFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  devsym_file *p = (devsym_file *)pFile;
  ଫେରନ୍ତୁ sqlite3OsFileSize(p->pReal, pSize);
}

/*
** Lock an devsym-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା devsymLock(sqlite3_file *pFile, ପୂର୍ଣ୍ଣସଂଖ୍ୟା eLock){
  devsym_file *p = (devsym_file *)pFile;
  ଫେରନ୍ତୁ sqlite3OsLock(p->pReal, eLock);
}

/*
** Unlock an devsym-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା devsymUnlock(sqlite3_file *pFile, ପୂର୍ଣ୍ଣସଂଖ୍ୟା eLock){
  devsym_file *p = (devsym_file *)pFile;
  ଫେରନ୍ତୁ sqlite3OsUnlock(p->pReal, eLock);
}

/*
** Check if another file-handle holds a RESERVED lock on an devsym-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା devsymCheckReservedLock(sqlite3_file *pFile, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pResOut){
  devsym_file *p = (devsym_file *)pFile;
  ଫେରନ୍ତୁ sqlite3OsCheckReservedLock(p->pReal, pResOut);
}

/*
** File control method. For custom operations on an devsym-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା devsymFileControl(sqlite3_file *pFile, ପୂର୍ଣ୍ଣସଂଖ୍ୟା op, ଶୂନ୍ୟ *pArg){
  devsym_file *p = (devsym_file *)pFile;
  ଫେରନ୍ତୁ sqlite3OsFileControl(p->pReal, op, pArg);
}

/*
** Return the sector-size in bytes for an devsym-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା devsymSectorSize(sqlite3_file *pFile){
  ଫେରନ୍ତୁ g.iSectorSize;
}

/*
** Return the device characteristic flags supported by an devsym-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା devsymDeviceCharacteristics(sqlite3_file *pFile){
  ଫେରନ୍ତୁ g.iDeviceChar;
}

/*
** Shared-memory methods are all pass-throughs.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା devsymShmLock(sqlite3_file *pFile, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ofst, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags){
  devsym_file *p = (devsym_file *)pFile;
  ଫେରନ୍ତୁ p->pReal->pMethods->xShmLock(p->pReal, ofst, n, flags);
}
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା devsymShmMap(
  sqlite3_file *pFile, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iRegion, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା szRegion, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା isWrite, 
  ଶୂନ୍ୟ ଅସ୍ଥିର **pp
){
  devsym_file *p = (devsym_file *)pFile;
  ଫେରନ୍ତୁ p->pReal->pMethods->xShmMap(p->pReal, iRegion, szRegion, isWrite, pp);
}
ସ୍ଥିର ଶୂନ୍ୟ devsymShmBarrier(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  p->pReal->pMethods->xShmBarrier(p->pReal);
}
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା devsymShmUnmap(sqlite3_file *pFile, ପୂର୍ଣ୍ଣସଂଖ୍ୟା delFlag){
  devsym_file *p = (devsym_file *)pFile;
  ଫେରନ୍ତୁ p->pReal->pMethods->xShmUnmap(p->pReal, delFlag);
}



/*
** Open an devsym file handle.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା devsymOpen(
  sqlite3_vfs *pVfs,
  ଅଚଳ ଅକ୍ଷର *zName,
  sqlite3_file *pFile,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pOutFlags
){
ସ୍ଥିର sqlite3_io_methods devsym_io_methods = {
  2,                                /* iVersion */
  devsymClose,                      /* xClose */
  devsymRead,                       /* xRead */
  devsymWrite,                      /* xWrite */
  devsymTruncate,                   /* xTruncate */
  devsymSync,                       /* xSync */
  devsymFileSize,                   /* xFileSize */
  devsymLock,                       /* xLock */
  devsymUnlock,                     /* xUnlock */
  devsymCheckReservedLock,          /* xCheckReservedLock */
  devsymFileControl,                /* xFileControl */
  devsymSectorSize,                 /* xSectorSize */
  devsymDeviceCharacteristics,      /* xDeviceCharacteristics */
  devsymShmMap,                     /* xShmMap */
  devsymShmLock,                    /* xShmLock */
  devsymShmBarrier,                 /* xShmBarrier */
  devsymShmUnmap                    /* xShmUnmap */
};

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  devsym_file *p = (devsym_file *)pFile;
  p->pReal = (sqlite3_file *)&p[1];
  rc = sqlite3OsOpen(g.pVfs, zName, p->pReal, flags, pOutFlags);
  ଯଦି( p->pReal->pMethods ){
    pFile->pMethods = &devsym_io_methods;
  }
  ଫେରନ୍ତୁ rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା devsymDelete(sqlite3_vfs *pVfs, ଅଚଳ ଅକ୍ଷର *zPath, ପୂର୍ଣ୍ଣସଂଖ୍ୟା dirSync){
  ଫେରନ୍ତୁ sqlite3OsDelete(g.pVfs, zPath, dirSync);
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା devsymAccess(
  sqlite3_vfs *pVfs, 
  ଅଚଳ ଅକ୍ଷର *zPath, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pResOut
){
  ଫେରନ୍ତୁ sqlite3OsAccess(g.pVfs, zPath, flags, pResOut);
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (DEVSYM_MAX_PATHNAME+1) bytes.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା devsymFullPathname(
  sqlite3_vfs *pVfs, 
  ଅଚଳ ଅକ୍ଷର *zPath, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nOut, 
  ଅକ୍ଷର *zOut
){
  ଫେରନ୍ତୁ sqlite3OsFullPathname(g.pVfs, zPath, nOut, zOut);
}

#ifndef SQLITE_OMIT_LOAD_EXTENSION
/*
** Open the dynamic library located at zPath and return a handle.
*/
ସ୍ଥିର ଶୂନ୍ୟ *devsymDlOpen(sqlite3_vfs *pVfs, ଅଚଳ ଅକ୍ଷର *zPath){
  ଫେରନ୍ତୁ sqlite3OsDlOpen(g.pVfs, zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
ସ୍ଥିର ଶୂନ୍ୟ devsymDlError(sqlite3_vfs *pVfs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nByte, ଅକ୍ଷର *z_Err_ସନ୍ଦେଶ){
  sqlite3OsDlError(g.pVfs, nByte, z_Err_ସନ୍ଦେଶ);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
ସ୍ଥିର ଶୂନ୍ୟ (*devsymDlSym(sqlite3_vfs *pVfs, ଶୂନ୍ୟ *p, ଅଚଳ ଅକ୍ଷର *zSym))(ଶୂନ୍ୟ){
  ଫେରନ୍ତୁ sqlite3OsDlSym(g.pVfs, p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
ସ୍ଥିର ଶୂନ୍ୟ devsymDlClose(sqlite3_vfs *pVfs, ଶୂନ୍ୟ *pHandle){
  sqlite3OsDlClose(g.pVfs, pHandle);
}
#endif /* SQLITE_OMIT_LOAD_EXTENSION */

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା devsymRandomness(sqlite3_vfs *pVfs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nByte, ଅକ୍ଷର *zBufOut){
  ଫେରନ୍ତୁ sqlite3OsRandomness(g.pVfs, nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା devsymSleep(sqlite3_vfs *pVfs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nMicro){
  ଫେରନ୍ତୁ sqlite3OsSleep(g.pVfs, nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା devsymCurrentTime(sqlite3_vfs *pVfs, ଦ୍ୱିଗୁଣ *pTimeOut){
  ଫେରନ୍ତୁ g.pVfs->xCurrentTime(g.pVfs, pTimeOut);
}

/*
** Return the sector-size in bytes for an writecrash-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା writecrashSectorSize(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  ଫେରନ୍ତୁ sqlite3OsSectorSize(p->pReal);
}

/*
** Return the device characteristic flags supported by an writecrash-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା writecrashDeviceCharacteristics(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  ଫେରନ୍ତୁ sqlite3OsDeviceCharacteristics(p->pReal);
}

/*
** Write data to an writecrash-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା writecrashWrite(
  sqlite3_file *pFile, 
  ଅଚଳ ଶୂନ୍ୟ *zBuf, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iAmt, 
  sqlite_int64 iOfst
){
  devsym_file *p = (devsym_file *)pFile;
  ଯଦି( g.nWriteCrash>0 ){
    g.nWriteCrash--;
    ଯଦି( g.nWriteCrash==0 ) abort();
  }
  ଫେରନ୍ତୁ sqlite3OsWrite(p->pReal, zBuf, iAmt, iOfst);
}

/*
** Open an writecrash file handle.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା writecrashOpen(
  sqlite3_vfs *pVfs,
  ଅଚଳ ଅକ୍ଷର *zName,
  sqlite3_file *pFile,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pOutFlags
){
ସ୍ଥିର sqlite3_io_methods writecrash_io_methods = {
  2,                                /* iVersion */
  devsymClose,                      /* xClose */
  devsymRead,                       /* xRead */
  writecrashWrite,                  /* xWrite */
  devsymTruncate,                   /* xTruncate */
  devsymSync,                       /* xSync */
  devsymFileSize,                   /* xFileSize */
  devsymLock,                       /* xLock */
  devsymUnlock,                     /* xUnlock */
  devsymCheckReservedLock,          /* xCheckReservedLock */
  devsymFileControl,                /* xFileControl */
  writecrashSectorSize,             /* xSectorSize */
  writecrashDeviceCharacteristics,  /* xDeviceCharacteristics */
  devsymShmMap,                     /* xShmMap */
  devsymShmLock,                    /* xShmLock */
  devsymShmBarrier,                 /* xShmBarrier */
  devsymShmUnmap                    /* xShmUnmap */
};

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  devsym_file *p = (devsym_file *)pFile;
  p->pReal = (sqlite3_file *)&p[1];
  rc = sqlite3OsOpen(g.pVfs, zName, p->pReal, flags, pOutFlags);
  ଯଦି( p->pReal->pMethods ){
    pFile->pMethods = &writecrash_io_methods;
  }
  ଫେରନ୍ତୁ rc;
}

ସ୍ଥିର sqlite3_vfs devsym_vfs = {
  2,                     /* iVersion */
  ଆକାର(devsym_file),      /* szOsFile */
  DEVSYM_MAX_PATHNAME,      /* mxPathname */
  0,                     /* pNext */
  DEVSYM_VFS_NAME,          /* zName */
  0,                     /* pAppData */
  devsymOpen,               /* xOpen */
  devsymDelete,             /* xDelete */
  devsymAccess,             /* xAccess */
  devsymFullPathname,       /* xFullPathname */
#ifndef SQLITE_OMIT_LOAD_EXTENSION
  devsymDlOpen,             /* xDlOpen */
  devsymDlError,            /* xDlError */
  devsymDlSym,              /* xDlSym */
  devsymDlClose,            /* xDlClose */
#ଅନ୍ୟଥା
  0,                        /* xDlOpen */
  0,                        /* xDlError */
  0,                        /* xDlSym */
  0,                        /* xDlClose */
#endif /* SQLITE_OMIT_LOAD_EXTENSION */
  devsymRandomness,         /* xRandomness */
  devsymSleep,              /* xSleep */
  devsymCurrentTime,        /* xCurrentTime */
  0,                        /* xGetLastError */
  0                         /* xCurrentTimeInt64 */
};

ସ୍ଥିର sqlite3_vfs writecrash_vfs = {
  2,                     /* iVersion */
  ଆକାର(devsym_file),      /* szOsFile */
  DEVSYM_MAX_PATHNAME,      /* mxPathname */
  0,                     /* pNext */
  WRITECRASH_NAME,          /* zName */
  0,                     /* pAppData */
  writecrashOpen,           /* xOpen */
  devsymDelete,             /* xDelete */
  devsymAccess,             /* xAccess */
  devsymFullPathname,       /* xFullPathname */
#ifndef SQLITE_OMIT_LOAD_EXTENSION
  devsymDlOpen,             /* xDlOpen */
  devsymDlError,            /* xDlError */
  devsymDlSym,              /* xDlSym */
  devsymDlClose,            /* xDlClose */
#ଅନ୍ୟଥା
  0,                        /* xDlOpen */
  0,                        /* xDlError */
  0,                        /* xDlSym */
  0,                        /* xDlClose */
#endif /* SQLITE_OMIT_LOAD_EXTENSION */
  devsymRandomness,         /* xRandomness */
  devsymSleep,              /* xSleep */
  devsymCurrentTime,        /* xCurrentTime */
  0,                        /* xGetLastError */
  0                         /* xCurrentTimeInt64 */
};


/*
** This procedure registers the devsym vfs with SQLite. If the argument is
** true, the devsym vfs becomes the new default vfs. It is the only publicly
** available function in this file.
*/
ଶୂନ୍ୟ devsym_register(ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDeviceChar, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iSectorSize){

  ଯଦି( g.pVfs==0 ){
    g.pVfs = sqlite3_vfs_find(0);
    devsym_vfs.szOsFile += g.pVfs->szOsFile;
    writecrash_vfs.szOsFile += g.pVfs->szOsFile;
    sqlite3_vfs_register(&devsym_vfs, 0);
    sqlite3_vfs_register(&writecrash_vfs, 0);
  }
  ଯଦି( iDeviceChar>=0 ){
    g.iDeviceChar = iDeviceChar;
  }ଅନ୍ୟଥା{
    g.iDeviceChar = 0;
  }
  ଯଦି( iSectorSize>=0 ){
    g.iSectorSize = iSectorSize;
  }ଅନ୍ୟଥା{
    g.iSectorSize = 512;
  }
}

ଶୂନ୍ୟ devsym_unregister(){
  sqlite3_vfs_unregister(&devsym_vfs);
  sqlite3_vfs_unregister(&writecrash_vfs);
  g.pVfs = 0;
  g.iDeviceChar = 0;
  g.iSectorSize = 0;
}

ଶୂନ୍ୟ devsym_crash_on_write(ପୂର୍ଣ୍ଣସଂଖ୍ୟା nWrite){
  ଯଦି( g.pVfs==0 ){
    g.pVfs = sqlite3_vfs_find(0);
    devsym_vfs.szOsFile += g.pVfs->szOsFile;
    writecrash_vfs.szOsFile += g.pVfs->szOsFile;
    sqlite3_vfs_register(&devsym_vfs, 0);
    sqlite3_vfs_register(&writecrash_vfs, 0);
  }
  g.nWriteCrash = nWrite;
}

#endif
