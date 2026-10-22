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
#ಯದಿ SQLITE_TEST          /* This file is used for testing only */

#ಅನ್ತರ್ಭಾವ "sqlite3.h"
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"

/*
** Maximum pathname length supported by the devsym backend.
*/
#define DEVSYM_MAX_PATHNAME 512

/*
** Name used to identify this VFS.
*/
#define DEVSYM_VFS_NAME "devsym"
#define WRITECRASH_NAME "writecrash"

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ devsym_file devsym_file;
ಸಂರಚನಾ devsym_file {
  sqlite3_file base;
  sqlite3_file *pReal;
};

/*
** Method declarations for devsym_file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ devsymClose(sqlite3_file*);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ devsymRead(sqlite3_file*, ಶೂನ್ಯ*, ಪೂರ್ಣಾಙ್ಕ iAmt, sqlite3_int64 iOfst);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ devsymWrite(sqlite3_file*,ಧ್ರುವ ಶೂನ್ಯ*,ಪೂರ್ಣಾಙ್ಕ iAmt, sqlite3_int64 iOfst);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ devsymTruncate(sqlite3_file*, sqlite3_int64 size);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ devsymSync(sqlite3_file*, ಪೂರ್ಣಾಙ್ಕ flags);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ devsymFileSize(sqlite3_file*, sqlite3_int64 *pSize);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ devsymLock(sqlite3_file*, ಪೂರ್ಣಾಙ್ಕ);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ devsymUnlock(sqlite3_file*, ಪೂರ್ಣಾಙ್ಕ);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ devsymCheckReservedLock(sqlite3_file*, ಪೂರ್ಣಾಙ್ಕ *);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ devsymFileControl(sqlite3_file*, ಪೂರ್ಣಾಙ್ಕ op, ಶೂನ್ಯ *pArg);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ devsymSectorSize(sqlite3_file*);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ devsymDeviceCharacteristics(sqlite3_file*);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ devsymShmLock(sqlite3_file*,ಪೂರ್ಣಾಙ್ಕ,ಪೂರ್ಣಾಙ್ಕ,ಪೂರ್ಣಾಙ್ಕ);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ devsymShmMap(sqlite3_file*,ಪೂರ್ಣಾಙ್ಕ,ಪೂರ್ಣಾಙ್ಕ,ಪೂರ್ಣಾಙ್ಕ, ಶೂನ್ಯ ಚಞ್ಚಲ **);
ಸ್ಥಿರ ಶೂನ್ಯ devsymShmBarrier(sqlite3_file*);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ devsymShmUnmap(sqlite3_file*,ಪೂರ್ಣಾಙ್ಕ);

/*
** Method declarations for devsym_vfs.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ devsymOpen(sqlite3_vfs*, ಧ್ರುವ ವರ್ಣ *, sqlite3_file*, ಪೂರ್ಣಾಙ್ಕ , ಪೂರ್ಣಾಙ್ಕ *);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ devsymDelete(sqlite3_vfs*, ಧ್ರುವ ವರ್ಣ *zName, ಪೂರ್ಣಾಙ್ಕ syncDir);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ devsymAccess(sqlite3_vfs*, ಧ್ರುವ ವರ್ಣ *zName, ಪೂರ್ಣಾಙ್ಕ flags, ಪೂರ್ಣಾಙ್ಕ *);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ devsymFullPathname(sqlite3_vfs*, ಧ್ರುವ ವರ್ಣ *zName, ಪೂರ್ಣಾಙ್ಕ, ವರ್ಣ *zOut);
#ifndef SQLITE_OMIT_LOAD_EXTENSION
ಸ್ಥಿರ ಶೂನ್ಯ *devsymDlOpen(sqlite3_vfs*, ಧ್ರುವ ವರ್ಣ *zFilename);
ಸ್ಥಿರ ಶೂನ್ಯ devsymDlError(sqlite3_vfs*, ಪೂರ್ಣಾಙ್ಕ nByte, ವರ್ಣ *z_Err_ಸಂದೇಶ);
ಸ್ಥಿರ ಶೂನ್ಯ (*devsymDlSym(sqlite3_vfs*,ಶೂನ್ಯ*, ಧ್ರುವ ವರ್ಣ *z_ಚಿಹ್ನೆ))(ಶೂನ್ಯ);
ಸ್ಥಿರ ಶೂನ್ಯ devsymDlClose(sqlite3_vfs*, ಶೂನ್ಯ*);
#endif /* SQLITE_OMIT_LOAD_EXTENSION */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ devsymRandomness(sqlite3_vfs*, ಪೂರ್ಣಾಙ್ಕ nByte, ವರ್ಣ *zOut);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ devsymSleep(sqlite3_vfs*, ಪೂರ್ಣಾಙ್ಕ microseconds);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ devsymCurrentTime(sqlite3_vfs*, ದ್ವಿಗುಣ*);

ಸಂರಚನಾ DevsymGlobal {
  sqlite3_vfs *pVfs;
  ಪೂರ್ಣಾಙ್ಕ iDeviceChar;
  ಪೂರ್ಣಾಙ್ಕ iSectorSize;
  ಪೂರ್ಣಾಙ್ಕ nWriteCrash;
};
ಸಂರಚನಾ DevsymGlobal g = {0, 0, 512, 0};

/*
** Close an devsym-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ devsymClose(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  sqlite3OsClose(p->pReal);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Read data from an devsym-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ devsymRead(
  sqlite3_file *pFile, 
  ಶೂನ್ಯ *zBuf, 
  ಪೂರ್ಣಾಙ್ಕ iAmt, 
  sqlite_int64 iOfst
){
  devsym_file *p = (devsym_file *)pFile;
  ಪ್ರತ್ಯಾಗಮ sqlite3OsRead(p->pReal, zBuf, iAmt, iOfst);
}

/*
** Write data to an devsym-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ devsymWrite(
  sqlite3_file *pFile, 
  ಧ್ರುವ ಶೂನ್ಯ *zBuf, 
  ಪೂರ್ಣಾಙ್ಕ iAmt, 
  sqlite_int64 iOfst
){
  devsym_file *p = (devsym_file *)pFile;
  ಪ್ರತ್ಯಾಗಮ sqlite3OsWrite(p->pReal, zBuf, iAmt, iOfst);
}

/*
** Truncate an devsym-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ devsymTruncate(sqlite3_file *pFile, sqlite_int64 size){
  devsym_file *p = (devsym_file *)pFile;
  ಪ್ರತ್ಯಾಗಮ sqlite3OsTruncate(p->pReal, size);
}

/*
** Sync an devsym-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ devsymSync(sqlite3_file *pFile, ಪೂರ್ಣಾಙ್ಕ flags){
  devsym_file *p = (devsym_file *)pFile;
  ಪ್ರತ್ಯಾಗಮ sqlite3OsSync(p->pReal, flags);
}

/*
** Return the current file-size of an devsym-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ devsymFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  devsym_file *p = (devsym_file *)pFile;
  ಪ್ರತ್ಯಾಗಮ sqlite3OsFileSize(p->pReal, pSize);
}

/*
** Lock an devsym-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ devsymLock(sqlite3_file *pFile, ಪೂರ್ಣಾಙ್ಕ eLock){
  devsym_file *p = (devsym_file *)pFile;
  ಪ್ರತ್ಯಾಗಮ sqlite3OsLock(p->pReal, eLock);
}

/*
** Unlock an devsym-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ devsymUnlock(sqlite3_file *pFile, ಪೂರ್ಣಾಙ್ಕ eLock){
  devsym_file *p = (devsym_file *)pFile;
  ಪ್ರತ್ಯಾಗಮ sqlite3OsUnlock(p->pReal, eLock);
}

/*
** Check if another file-handle holds a RESERVED lock on an devsym-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ devsymCheckReservedLock(sqlite3_file *pFile, ಪೂರ್ಣಾಙ್ಕ *pResOut){
  devsym_file *p = (devsym_file *)pFile;
  ಪ್ರತ್ಯಾಗಮ sqlite3OsCheckReservedLock(p->pReal, pResOut);
}

/*
** File control method. For custom operations on an devsym-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ devsymFileControl(sqlite3_file *pFile, ಪೂರ್ಣಾಙ್ಕ op, ಶೂನ್ಯ *pArg){
  devsym_file *p = (devsym_file *)pFile;
  ಪ್ರತ್ಯಾಗಮ sqlite3OsFileControl(p->pReal, op, pArg);
}

/*
** Return the sector-size in bytes for an devsym-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ devsymSectorSize(sqlite3_file *pFile){
  ಪ್ರತ್ಯಾಗಮ g.iSectorSize;
}

/*
** Return the device characteristic flags supported by an devsym-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ devsymDeviceCharacteristics(sqlite3_file *pFile){
  ಪ್ರತ್ಯಾಗಮ g.iDeviceChar;
}

/*
** Shared-memory methods are all pass-throughs.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ devsymShmLock(sqlite3_file *pFile, ಪೂರ್ಣಾಙ್ಕ ofst, ಪೂರ್ಣಾಙ್ಕ n, ಪೂರ್ಣಾಙ್ಕ flags){
  devsym_file *p = (devsym_file *)pFile;
  ಪ್ರತ್ಯಾಗಮ p->pReal->pMethods->xShmLock(p->pReal, ofst, n, flags);
}
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ devsymShmMap(
  sqlite3_file *pFile, 
  ಪೂರ್ಣಾಙ್ಕ iRegion, 
  ಪೂರ್ಣಾಙ್ಕ szRegion, 
  ಪೂರ್ಣಾಙ್ಕ isWrite, 
  ಶೂನ್ಯ ಚಞ್ಚಲ **pp
){
  devsym_file *p = (devsym_file *)pFile;
  ಪ್ರತ್ಯಾಗಮ p->pReal->pMethods->xShmMap(p->pReal, iRegion, szRegion, isWrite, pp);
}
ಸ್ಥಿರ ಶೂನ್ಯ devsymShmBarrier(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  p->pReal->pMethods->xShmBarrier(p->pReal);
}
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ devsymShmUnmap(sqlite3_file *pFile, ಪೂರ್ಣಾಙ್ಕ delFlag){
  devsym_file *p = (devsym_file *)pFile;
  ಪ್ರತ್ಯಾಗಮ p->pReal->pMethods->xShmUnmap(p->pReal, delFlag);
}



/*
** Open an devsym file handle.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ devsymOpen(
  sqlite3_vfs *pVfs,
  ಧ್ರುವ ವರ್ಣ *zName,
  sqlite3_file *pFile,
  ಪೂರ್ಣಾಙ್ಕ flags,
  ಪೂರ್ಣಾಙ್ಕ *pOutFlags
){
ಸ್ಥಿರ sqlite3_io_methods devsym_io_methods = {
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

  ಪೂರ್ಣಾಙ್ಕ rc;
  devsym_file *p = (devsym_file *)pFile;
  p->pReal = (sqlite3_file *)&p[1];
  rc = sqlite3OsOpen(g.pVfs, zName, p->pReal, flags, pOutFlags);
  ಯದಿ( p->pReal->pMethods ){
    pFile->pMethods = &devsym_io_methods;
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ devsymDelete(sqlite3_vfs *pVfs, ಧ್ರುವ ವರ್ಣ *zPath, ಪೂರ್ಣಾಙ್ಕ dirSync){
  ಪ್ರತ್ಯಾಗಮ sqlite3OsDelete(g.pVfs, zPath, dirSync);
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ devsymAccess(
  sqlite3_vfs *pVfs, 
  ಧ್ರುವ ವರ್ಣ *zPath, 
  ಪೂರ್ಣಾಙ್ಕ flags, 
  ಪೂರ್ಣಾಙ್ಕ *pResOut
){
  ಪ್ರತ್ಯಾಗಮ sqlite3OsAccess(g.pVfs, zPath, flags, pResOut);
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (DEVSYM_MAX_PATHNAME+1) bytes.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ devsymFullPathname(
  sqlite3_vfs *pVfs, 
  ಧ್ರುವ ವರ್ಣ *zPath, 
  ಪೂರ್ಣಾಙ್ಕ nOut, 
  ವರ್ಣ *zOut
){
  ಪ್ರತ್ಯಾಗಮ sqlite3OsFullPathname(g.pVfs, zPath, nOut, zOut);
}

#ifndef SQLITE_OMIT_LOAD_EXTENSION
/*
** Open the dynamic library located at zPath and return a handle.
*/
ಸ್ಥಿರ ಶೂನ್ಯ *devsymDlOpen(sqlite3_vfs *pVfs, ಧ್ರುವ ವರ್ಣ *zPath){
  ಪ್ರತ್ಯಾಗಮ sqlite3OsDlOpen(g.pVfs, zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
ಸ್ಥಿರ ಶೂನ್ಯ devsymDlError(sqlite3_vfs *pVfs, ಪೂರ್ಣಾಙ್ಕ nByte, ವರ್ಣ *z_Err_ಸಂದೇಶ){
  sqlite3OsDlError(g.pVfs, nByte, z_Err_ಸಂದೇಶ);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
ಸ್ಥಿರ ಶೂನ್ಯ (*devsymDlSym(sqlite3_vfs *pVfs, ಶೂನ್ಯ *p, ಧ್ರುವ ವರ್ಣ *zSym))(ಶೂನ್ಯ){
  ಪ್ರತ್ಯಾಗಮ sqlite3OsDlSym(g.pVfs, p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
ಸ್ಥಿರ ಶೂನ್ಯ devsymDlClose(sqlite3_vfs *pVfs, ಶೂನ್ಯ *pHandle){
  sqlite3OsDlClose(g.pVfs, pHandle);
}
#endif /* SQLITE_OMIT_LOAD_EXTENSION */

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ devsymRandomness(sqlite3_vfs *pVfs, ಪೂರ್ಣಾಙ್ಕ nByte, ವರ್ಣ *zBufOut){
  ಪ್ರತ್ಯಾಗಮ sqlite3OsRandomness(g.pVfs, nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ devsymSleep(sqlite3_vfs *pVfs, ಪೂರ್ಣಾಙ್ಕ nMicro){
  ಪ್ರತ್ಯಾಗಮ sqlite3OsSleep(g.pVfs, nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ devsymCurrentTime(sqlite3_vfs *pVfs, ದ್ವಿಗುಣ *pTimeOut){
  ಪ್ರತ್ಯಾಗಮ g.pVfs->xCurrentTime(g.pVfs, pTimeOut);
}

/*
** Return the sector-size in bytes for an writecrash-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ writecrashSectorSize(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  ಪ್ರತ್ಯಾಗಮ sqlite3OsSectorSize(p->pReal);
}

/*
** Return the device characteristic flags supported by an writecrash-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ writecrashDeviceCharacteristics(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  ಪ್ರತ್ಯಾಗಮ sqlite3OsDeviceCharacteristics(p->pReal);
}

/*
** Write data to an writecrash-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ writecrashWrite(
  sqlite3_file *pFile, 
  ಧ್ರುವ ಶೂನ್ಯ *zBuf, 
  ಪೂರ್ಣಾಙ್ಕ iAmt, 
  sqlite_int64 iOfst
){
  devsym_file *p = (devsym_file *)pFile;
  ಯದಿ( g.nWriteCrash>0 ){
    g.nWriteCrash--;
    ಯದಿ( g.nWriteCrash==0 ) abort();
  }
  ಪ್ರತ್ಯಾಗಮ sqlite3OsWrite(p->pReal, zBuf, iAmt, iOfst);
}

/*
** Open an writecrash file handle.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ writecrashOpen(
  sqlite3_vfs *pVfs,
  ಧ್ರುವ ವರ್ಣ *zName,
  sqlite3_file *pFile,
  ಪೂರ್ಣಾಙ್ಕ flags,
  ಪೂರ್ಣಾಙ್ಕ *pOutFlags
){
ಸ್ಥಿರ sqlite3_io_methods writecrash_io_methods = {
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

  ಪೂರ್ಣಾಙ್ಕ rc;
  devsym_file *p = (devsym_file *)pFile;
  p->pReal = (sqlite3_file *)&p[1];
  rc = sqlite3OsOpen(g.pVfs, zName, p->pReal, flags, pOutFlags);
  ಯದಿ( p->pReal->pMethods ){
    pFile->pMethods = &writecrash_io_methods;
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}

ಸ್ಥಿರ sqlite3_vfs devsym_vfs = {
  2,                     /* iVersion */
  ಪರಿಮಾಣ(devsym_file),      /* szOsFile */
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
#ಅನ್ಯಥಾ
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

ಸ್ಥಿರ sqlite3_vfs writecrash_vfs = {
  2,                     /* iVersion */
  ಪರಿಮಾಣ(devsym_file),      /* szOsFile */
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
#ಅನ್ಯಥಾ
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
ಶೂನ್ಯ devsym_register(ಪೂರ್ಣಾಙ್ಕ iDeviceChar, ಪೂರ್ಣಾಙ್ಕ iSectorSize){

  ಯದಿ( g.pVfs==0 ){
    g.pVfs = sqlite3_vfs_find(0);
    devsym_vfs.szOsFile += g.pVfs->szOsFile;
    writecrash_vfs.szOsFile += g.pVfs->szOsFile;
    sqlite3_vfs_register(&devsym_vfs, 0);
    sqlite3_vfs_register(&writecrash_vfs, 0);
  }
  ಯದಿ( iDeviceChar>=0 ){
    g.iDeviceChar = iDeviceChar;
  }ಅನ್ಯಥಾ{
    g.iDeviceChar = 0;
  }
  ಯದಿ( iSectorSize>=0 ){
    g.iSectorSize = iSectorSize;
  }ಅನ್ಯಥಾ{
    g.iSectorSize = 512;
  }
}

ಶೂನ್ಯ devsym_unregister(){
  sqlite3_vfs_unregister(&devsym_vfs);
  sqlite3_vfs_unregister(&writecrash_vfs);
  g.pVfs = 0;
  g.iDeviceChar = 0;
  g.iSectorSize = 0;
}

ಶೂನ್ಯ devsym_crash_on_write(ಪೂರ್ಣಾಙ್ಕ nWrite){
  ಯದಿ( g.pVfs==0 ){
    g.pVfs = sqlite3_vfs_find(0);
    devsym_vfs.szOsFile += g.pVfs->szOsFile;
    writecrash_vfs.szOsFile += g.pVfs->szOsFile;
    sqlite3_vfs_register(&devsym_vfs, 0);
    sqlite3_vfs_register(&writecrash_vfs, 0);
  }
  g.nWriteCrash = nWrite;
}

#endif
