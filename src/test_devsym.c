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
#ಆದರೆ SQLITE_TEST          /* This file is used for testing only */

#ಅನ್ತರ್ಭಾವ "sqlite3.h"
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"

/*
** Maximum pathname length supported by the devsym backend.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು DEVSYM_MAX_PATHNAME 512

/*
** Name used to identify this VFS.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು DEVSYM_VFS_NAME "devsym"
#ವ್ಯಾಖ್ಯಾನಿಸು WRITECRASH_NAME "writecrash"

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ devsym_file devsym_file;
ರಚನೆ devsym_file {
  sqlite3_file ತಳ;
  sqlite3_file *pReal;
};

/*
** Method declarations for devsym_file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ devsymClose(sqlite3_file*);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ devsymRead(sqlite3_file*, ಶೂನ್ಯ*, ಪೂರ್ಣಾಂಕ iAmt, sqlite3_int64 iOfst);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ devsymWrite(sqlite3_file*,ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*,ಪೂರ್ಣಾಂಕ iAmt, sqlite3_int64 iOfst);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ devsymTruncate(sqlite3_file*, sqlite3_int64 ಗಾತ್ರ_2);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ devsymSync(sqlite3_file*, ಪೂರ್ಣಾಂಕ flags);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ devsym_File_ಗಾತ್ರ(sqlite3_file*, sqlite3_int64 *p_ಗಾತ್ರ);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ devsym_ಬೀಗ(sqlite3_file*, ಪೂರ್ಣಾಂಕ);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ devsymUnlock(sqlite3_file*, ಪೂರ್ಣಾಂಕ);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ devsym_Check_Reserved_ಬೀಗ(sqlite3_file*, ಪೂರ್ಣಾಂಕ *);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ devsymFileControl(sqlite3_file*, ಪೂರ್ಣಾಂಕ op, ಶೂನ್ಯ *pArg);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ devsym_Sector_ಗಾತ್ರ(sqlite3_file*);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ devsymDeviceCharacteristics(sqlite3_file*);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ devsym_Shm_ಬೀಗ(sqlite3_file*,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ devsymShmMap(sqlite3_file*,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ, ಶೂನ್ಯ ಅಸ್ಥಿರ **);
ಸ್ಥಿರ ಶೂನ್ಯ devsymShmBarrier(sqlite3_file*);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ devsymShmUnmap(sqlite3_file*,ಪೂರ್ಣಾಂಕ);

/*
** Method declarations for devsym_vfs.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ devsymOpen(sqlite3_vfs*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *, sqlite3_file*, ಪೂರ್ಣಾಂಕ , ಪೂರ್ಣಾಂಕ *);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ devsymDelete(sqlite3_vfs*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName, ಪೂರ್ಣಾಂಕ syncDir);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ devsymAccess(sqlite3_vfs*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName, ಪೂರ್ಣಾಂಕ flags, ಪೂರ್ಣಾಂಕ *);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ devsym_ತುಂಬಿದ_Pathname(sqlite3_vfs*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName, ಪೂರ್ಣಾಂಕ, ಅಕ್ಷರ *z_ಹೊರಹೋಗು);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_ತುಂಬು_EXTENSION
ಸ್ಥಿರ ಶೂನ್ಯ *devsymDlOpen(sqlite3_vfs*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFilename);
ಸ್ಥಿರ ಶೂನ್ಯ devsymDlError(sqlite3_vfs*, ಪೂರ್ಣಾಂಕ nByte, ಅಕ್ಷರ *z_Err_ಸಂದೇಶ);
ಸ್ಥಿರ ಶೂನ್ಯ (*devsymDlSym(sqlite3_vfs*,ಶೂನ್ಯ*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಚಿಹ್ನೆ))(ಶೂನ್ಯ);
ಸ್ಥಿರ ಶೂನ್ಯ devsymDlClose(sqlite3_vfs*, ಶೂನ್ಯ*);
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_LOAD_EXTENSION */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ devsymRandomness(sqlite3_vfs*, ಪೂರ್ಣಾಂಕ nByte, ಅಕ್ಷರ *z_ಹೊರಹೋಗು);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ devsymSleep(sqlite3_vfs*, ಪೂರ್ಣಾಂಕ microseconds);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ devsym_Current_ಸಮಯ(sqlite3_vfs*, ದ್ವಿಗುಣ*);

ರಚನೆ DevsymGlobal {
  sqlite3_vfs *pVfs;
  ಪೂರ್ಣಾಂಕ iDeviceChar;
  ಪೂರ್ಣಾಂಕ i_Sector_ಗಾತ್ರ;
  ಪೂರ್ಣಾಂಕ nWriteCrash;
};
ರಚನೆ DevsymGlobal g = {0, 0, 512, 0};

/*
** Close an devsym-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ devsymClose(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  sqlite3OsClose(p->pReal);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Read data from an devsym-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ devsymRead(
  sqlite3_file *pFile, 
  ಶೂನ್ಯ *zBuf, 
  ಪೂರ್ಣಾಂಕ iAmt, 
  sqlite_int64 iOfst
){
  devsym_file *p = (devsym_file *)pFile;
  ಹಿಂತಿರುಗಿ sqlite3OsRead(p->pReal, zBuf, iAmt, iOfst);
}

/*
** Write data to an devsym-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ devsymWrite(
  sqlite3_file *pFile, 
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *zBuf, 
  ಪೂರ್ಣಾಂಕ iAmt, 
  sqlite_int64 iOfst
){
  devsym_file *p = (devsym_file *)pFile;
  ಹಿಂತಿರುಗಿ sqlite3OsWrite(p->pReal, zBuf, iAmt, iOfst);
}

/*
** Truncate an devsym-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ devsymTruncate(sqlite3_file *pFile, sqlite_int64 ಗಾತ್ರ_2){
  devsym_file *p = (devsym_file *)pFile;
  ಹಿಂತಿರುಗಿ sqlite3OsTruncate(p->pReal, ಗಾತ್ರ_2);
}

/*
** Sync an devsym-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ devsymSync(sqlite3_file *pFile, ಪೂರ್ಣಾಂಕ flags){
  devsym_file *p = (devsym_file *)pFile;
  ಹಿಂತಿರುಗಿ sqlite3OsSync(p->pReal, flags);
}

/*
** Return the current file-size of an devsym-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ devsym_File_ಗಾತ್ರ(sqlite3_file *pFile, sqlite_int64 *p_ಗಾತ್ರ){
  devsym_file *p = (devsym_file *)pFile;
  ಹಿಂತಿರುಗಿ sqlite3_Os_File_ಗಾತ್ರ(p->pReal, p_ಗಾತ್ರ);
}

/*
** Lock an devsym-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ devsym_ಬೀಗ(sqlite3_file *pFile, ಪೂರ್ಣಾಂಕ e_ಬೀಗ){
  devsym_file *p = (devsym_file *)pFile;
  ಹಿಂತಿರುಗಿ sqlite3_Os_ಬೀಗ(p->pReal, e_ಬೀಗ);
}

/*
** Unlock an devsym-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ devsymUnlock(sqlite3_file *pFile, ಪೂರ್ಣಾಂಕ e_ಬೀಗ){
  devsym_file *p = (devsym_file *)pFile;
  ಹಿಂತಿರುಗಿ sqlite3OsUnlock(p->pReal, e_ಬೀಗ);
}

/*
** Check if another file-handle holds a RESERVED lock on an devsym-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ devsym_Check_Reserved_ಬೀಗ(sqlite3_file *pFile, ಪೂರ್ಣಾಂಕ *p_Res_ಹೊರಹೋಗು){
  devsym_file *p = (devsym_file *)pFile;
  ಹಿಂತಿರುಗಿ sqlite3_Os_Check_Reserved_ಬೀಗ(p->pReal, p_Res_ಹೊರಹೋಗು);
}

/*
** File control method. For custom operations on an devsym-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ devsymFileControl(sqlite3_file *pFile, ಪೂರ್ಣಾಂಕ op, ಶೂನ್ಯ *pArg){
  devsym_file *p = (devsym_file *)pFile;
  ಹಿಂತಿರುಗಿ sqlite3OsFileControl(p->pReal, op, pArg);
}

/*
** Return the sector-size in bytes for an devsym-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ devsym_Sector_ಗಾತ್ರ(sqlite3_file *pFile){
  ಹಿಂತಿರುಗಿ g.i_Sector_ಗಾತ್ರ;
}

/*
** Return the device characteristic flags supported by an devsym-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ devsymDeviceCharacteristics(sqlite3_file *pFile){
  ಹಿಂತಿರುಗಿ g.iDeviceChar;
}

/*
** Shared-memory methods are all pass-throughs.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ devsym_Shm_ಬೀಗ(sqlite3_file *pFile, ಪೂರ್ಣಾಂಕ ofst, ಪೂರ್ಣಾಂಕ n, ಪೂರ್ಣಾಂಕ flags){
  devsym_file *p = (devsym_file *)pFile;
  ಹಿಂತಿರುಗಿ p->pReal->pMethods->x_Shm_ಬೀಗ(p->pReal, ofst, n, flags);
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ devsymShmMap(
  sqlite3_file *pFile, 
  ಪೂರ್ಣಾಂಕ iRegion, 
  ಪೂರ್ಣಾಂಕ szRegion, 
  ಪೂರ್ಣಾಂಕ isWrite, 
  ಶೂನ್ಯ ಅಸ್ಥಿರ **pp
){
  devsym_file *p = (devsym_file *)pFile;
  ಹಿಂತಿರುಗಿ p->pReal->pMethods->xShmMap(p->pReal, iRegion, szRegion, isWrite, pp);
}
ಸ್ಥಿರ ಶೂನ್ಯ devsymShmBarrier(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  p->pReal->pMethods->xShmBarrier(p->pReal);
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ devsymShmUnmap(sqlite3_file *pFile, ಪೂರ್ಣಾಂಕ delFlag){
  devsym_file *p = (devsym_file *)pFile;
  ಹಿಂತಿರುಗಿ p->pReal->pMethods->xShmUnmap(p->pReal, delFlag);
}



/*
** Open an devsym file handle.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ devsymOpen(
  sqlite3_vfs *pVfs,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName,
  sqlite3_file *pFile,
  ಪೂರ್ಣಾಂಕ flags,
  ಪೂರ್ಣಾಂಕ *p_ಹೊರಹೋಗು_Flags
){
ಸ್ಥಿರ sqlite3_io_methods devsym_io_methods = {
  2,                                /* iVersion */
  devsymClose,                      /* xClose */
  devsymRead,                       /* xRead */
  devsymWrite,                      /* xWrite */
  devsymTruncate,                   /* xTruncate */
  devsymSync,                       /* xSync */
  devsym_File_ಗಾತ್ರ,                   /* xFileSize */
  devsym_ಬೀಗ,                       /* xLock */
  devsymUnlock,                     /* xUnlock */
  devsym_Check_Reserved_ಬೀಗ,          /* xCheckReservedLock */
  devsymFileControl,                /* xFileControl */
  devsym_Sector_ಗಾತ್ರ,                 /* xSectorSize */
  devsymDeviceCharacteristics,      /* xDeviceCharacteristics */
  devsymShmMap,                     /* xShmMap */
  devsym_Shm_ಬೀಗ,                    /* xShmLock */
  devsymShmBarrier,                 /* xShmBarrier */
  devsymShmUnmap                    /* xShmUnmap */
};

  ಪೂರ್ಣಾಂಕ rc;
  devsym_file *p = (devsym_file *)pFile;
  p->pReal = (sqlite3_file *)&p[1];
  rc = sqlite3OsOpen(g.pVfs, zName, p->pReal, flags, p_ಹೊರಹೋಗು_Flags);
  ಆದರೆ( p->pReal->pMethods ){
    pFile->pMethods = &devsym_io_methods;
  }
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ devsymDelete(sqlite3_vfs *pVfs, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮಾರ್ಗ, ಪೂರ್ಣಾಂಕ dirSync){
  ಹಿಂತಿರುಗಿ sqlite3OsDelete(g.pVfs, z_ಮಾರ್ಗ, dirSync);
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ devsymAccess(
  sqlite3_vfs *pVfs, 
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮಾರ್ಗ, 
  ಪೂರ್ಣಾಂಕ flags, 
  ಪೂರ್ಣಾಂಕ *p_Res_ಹೊರಹೋಗು
){
  ಹಿಂತಿರುಗಿ sqlite3OsAccess(g.pVfs, z_ಮಾರ್ಗ, flags, p_Res_ಹೊರಹೋಗು);
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (DEVSYM_MAX_PATHNAME+1) bytes.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ devsym_ತುಂಬಿದ_Pathname(
  sqlite3_vfs *pVfs, 
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮಾರ್ಗ, 
  ಪೂರ್ಣಾಂಕ n_ಹೊರಹೋಗು, 
  ಅಕ್ಷರ *z_ಹೊರಹೋಗು
){
  ಹಿಂತಿರುಗಿ sqlite3_Os_ತುಂಬಿದ_Pathname(g.pVfs, z_ಮಾರ್ಗ, n_ಹೊರಹೋಗು, z_ಹೊರಹೋಗು);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_ತುಂಬು_EXTENSION
/*
** Open the dynamic library located at zPath and return a handle.
*/
ಸ್ಥಿರ ಶೂನ್ಯ *devsymDlOpen(sqlite3_vfs *pVfs, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮಾರ್ಗ){
  ಹಿಂತಿರುಗಿ sqlite3OsDlOpen(g.pVfs, z_ಮಾರ್ಗ);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
ಸ್ಥಿರ ಶೂನ್ಯ devsymDlError(sqlite3_vfs *pVfs, ಪೂರ್ಣಾಂಕ nByte, ಅಕ್ಷರ *z_Err_ಸಂದೇಶ){
  sqlite3OsDlError(g.pVfs, nByte, z_Err_ಸಂದೇಶ);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
ಸ್ಥಿರ ಶೂನ್ಯ (*devsymDlSym(sqlite3_vfs *pVfs, ಶೂನ್ಯ *p, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSym))(ಶೂನ್ಯ){
  ಹಿಂತಿರುಗಿ sqlite3OsDlSym(g.pVfs, p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
ಸ್ಥಿರ ಶೂನ್ಯ devsymDlClose(sqlite3_vfs *pVfs, ಶೂನ್ಯ *pHandle){
  sqlite3OsDlClose(g.pVfs, pHandle);
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_LOAD_EXTENSION */

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ devsymRandomness(sqlite3_vfs *pVfs, ಪೂರ್ಣಾಂಕ nByte, ಅಕ್ಷರ *z_Buf_ಹೊರಹೋಗು){
  ಹಿಂತಿರುಗಿ sqlite3OsRandomness(g.pVfs, nByte, z_Buf_ಹೊರಹೋಗು);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ devsymSleep(sqlite3_vfs *pVfs, ಪೂರ್ಣಾಂಕ nMicro){
  ಹಿಂತಿರುಗಿ sqlite3OsSleep(g.pVfs, nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ devsym_Current_ಸಮಯ(sqlite3_vfs *pVfs, ದ್ವಿಗುಣ *p_ಸಮಯ_ಹೊರಹೋಗು){
  ಹಿಂತಿರುಗಿ g.pVfs->x_Current_ಸಮಯ(g.pVfs, p_ಸಮಯ_ಹೊರಹೋಗು);
}

/*
** Return the sector-size in bytes for an writecrash-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ writecrash_Sector_ಗಾತ್ರ(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  ಹಿಂತಿರುಗಿ sqlite3_Os_Sector_ಗಾತ್ರ(p->pReal);
}

/*
** Return the device characteristic flags supported by an writecrash-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ writecrashDeviceCharacteristics(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  ಹಿಂತಿರುಗಿ sqlite3OsDeviceCharacteristics(p->pReal);
}

/*
** Write data to an writecrash-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ writecrashWrite(
  sqlite3_file *pFile, 
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *zBuf, 
  ಪೂರ್ಣಾಂಕ iAmt, 
  sqlite_int64 iOfst
){
  devsym_file *p = (devsym_file *)pFile;
  ಆದರೆ( g.nWriteCrash>0 ){
    g.nWriteCrash--;
    ಆದರೆ( g.nWriteCrash==0 ) abort();
  }
  ಹಿಂತಿರುಗಿ sqlite3OsWrite(p->pReal, zBuf, iAmt, iOfst);
}

/*
** Open an writecrash file handle.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ writecrashOpen(
  sqlite3_vfs *pVfs,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName,
  sqlite3_file *pFile,
  ಪೂರ್ಣಾಂಕ flags,
  ಪೂರ್ಣಾಂಕ *p_ಹೊರಹೋಗು_Flags
){
ಸ್ಥಿರ sqlite3_io_methods writecrash_io_methods = {
  2,                                /* iVersion */
  devsymClose,                      /* xClose */
  devsymRead,                       /* xRead */
  writecrashWrite,                  /* xWrite */
  devsymTruncate,                   /* xTruncate */
  devsymSync,                       /* xSync */
  devsym_File_ಗಾತ್ರ,                   /* xFileSize */
  devsym_ಬೀಗ,                       /* xLock */
  devsymUnlock,                     /* xUnlock */
  devsym_Check_Reserved_ಬೀಗ,          /* xCheckReservedLock */
  devsymFileControl,                /* xFileControl */
  writecrash_Sector_ಗಾತ್ರ,             /* xSectorSize */
  writecrashDeviceCharacteristics,  /* xDeviceCharacteristics */
  devsymShmMap,                     /* xShmMap */
  devsym_Shm_ಬೀಗ,                    /* xShmLock */
  devsymShmBarrier,                 /* xShmBarrier */
  devsymShmUnmap                    /* xShmUnmap */
};

  ಪೂರ್ಣಾಂಕ rc;
  devsym_file *p = (devsym_file *)pFile;
  p->pReal = (sqlite3_file *)&p[1];
  rc = sqlite3OsOpen(g.pVfs, zName, p->pReal, flags, p_ಹೊರಹೋಗು_Flags);
  ಆದರೆ( p->pReal->pMethods ){
    pFile->pMethods = &writecrash_io_methods;
  }
  ಹಿಂತಿರುಗಿ rc;
}

ಸ್ಥಿರ sqlite3_vfs devsym_vfs = {
  2,                     /* iVersion */
  ಗಾತ್ರ(devsym_file),      /* szOsFile */
  DEVSYM_MAX_PATHNAME,      /* mxPathname */
  0,                     /* pNext */
  DEVSYM_VFS_NAME,          /* zName */
  0,                     /* pAppData */
  devsymOpen,               /* xOpen */
  devsymDelete,             /* xDelete */
  devsymAccess,             /* xAccess */
  devsym_ತುಂಬಿದ_Pathname,       /* xFullPathname */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_ತುಂಬು_EXTENSION
  devsymDlOpen,             /* xDlOpen */
  devsymDlError,            /* xDlError */
  devsymDlSym,              /* xDlSym */
  devsymDlClose,            /* xDlClose */
#ಇಲ್ಲದಿದ್ದರೆ
  0,                        /* xDlOpen */
  0,                        /* xDlError */
  0,                        /* xDlSym */
  0,                        /* xDlClose */
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_LOAD_EXTENSION */
  devsymRandomness,         /* xRandomness */
  devsymSleep,              /* xSleep */
  devsym_Current_ಸಮಯ,        /* xCurrentTime */
  0,                        /* xGetLastError */
  0                         /* xCurrentTimeInt64 */
};

ಸ್ಥಿರ sqlite3_vfs writecrash_vfs = {
  2,                     /* iVersion */
  ಗಾತ್ರ(devsym_file),      /* szOsFile */
  DEVSYM_MAX_PATHNAME,      /* mxPathname */
  0,                     /* pNext */
  WRITECRASH_NAME,          /* zName */
  0,                     /* pAppData */
  writecrashOpen,           /* xOpen */
  devsymDelete,             /* xDelete */
  devsymAccess,             /* xAccess */
  devsym_ತುಂಬಿದ_Pathname,       /* xFullPathname */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_ತುಂಬು_EXTENSION
  devsymDlOpen,             /* xDlOpen */
  devsymDlError,            /* xDlError */
  devsymDlSym,              /* xDlSym */
  devsymDlClose,            /* xDlClose */
#ಇಲ್ಲದಿದ್ದರೆ
  0,                        /* xDlOpen */
  0,                        /* xDlError */
  0,                        /* xDlSym */
  0,                        /* xDlClose */
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_LOAD_EXTENSION */
  devsymRandomness,         /* xRandomness */
  devsymSleep,              /* xSleep */
  devsym_Current_ಸಮಯ,        /* xCurrentTime */
  0,                        /* xGetLastError */
  0                         /* xCurrentTimeInt64 */
};


/*
** This procedure registers the devsym vfs with SQLite. If the argument is
** true, the devsym vfs becomes the new default vfs. It is the only publicly
** available function in this file.
*/
ಶೂನ್ಯ devsym_ರಿಜಿಸ್ಟರ್(ಪೂರ್ಣಾಂಕ iDeviceChar, ಪೂರ್ಣಾಂಕ i_Sector_ಗಾತ್ರ){

  ಆದರೆ( g.pVfs==0 ){
    g.pVfs = sqlite3_vfs_ಕಂಡುಹಿಡಿ(0);
    devsym_vfs.szOsFile += g.pVfs->szOsFile;
    writecrash_vfs.szOsFile += g.pVfs->szOsFile;
    sqlite3_vfs_ರಿಜಿಸ್ಟರ್(&devsym_vfs, 0);
    sqlite3_vfs_ರಿಜಿಸ್ಟರ್(&writecrash_vfs, 0);
  }
  ಆದರೆ( iDeviceChar>=0 ){
    g.iDeviceChar = iDeviceChar;
  }ಇಲ್ಲದಿದ್ದರೆ{
    g.iDeviceChar = 0;
  }
  ಆದರೆ( i_Sector_ಗಾತ್ರ>=0 ){
    g.i_Sector_ಗಾತ್ರ = i_Sector_ಗಾತ್ರ;
  }ಇಲ್ಲದಿದ್ದರೆ{
    g.i_Sector_ಗಾತ್ರ = 512;
  }
}

ಶೂನ್ಯ devsym_unregister(){
  sqlite3_vfs_unregister(&devsym_vfs);
  sqlite3_vfs_unregister(&writecrash_vfs);
  g.pVfs = 0;
  g.iDeviceChar = 0;
  g.i_Sector_ಗಾತ್ರ = 0;
}

ಶೂನ್ಯ devsym_crash_on_write(ಪೂರ್ಣಾಂಕ nWrite){
  ಆದರೆ( g.pVfs==0 ){
    g.pVfs = sqlite3_vfs_ಕಂಡುಹಿಡಿ(0);
    devsym_vfs.szOsFile += g.pVfs->szOsFile;
    writecrash_vfs.szOsFile += g.pVfs->szOsFile;
    sqlite3_vfs_ರಿಜಿಸ್ಟರ್(&devsym_vfs, 0);
    sqlite3_vfs_ರಿಜಿಸ್ಟರ್(&writecrash_vfs, 0);
  }
  g.nWriteCrash = nWrite;
}

#ಆದರೆ_ಅಂತ್ಯ
