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
#ਜੇ SQLITE_TEST          /* This file is used for testing only */

#ਅਨ੍ਤਰ੍ਭਾਵ "sqlite3.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"

/*
** Maximum pathname length supported by the devsym backend.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEVSYM_MAX_PATHNAME 512

/*
** Name used to identify this VFS.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEVSYM_VFS_NAME "devsym"
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WRITECRASH_NAME "writecrash"

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ devsym_file devsym_file;
ਬਣਤਰ devsym_file {
  sqlite3_file ਅਧਾਰ;
  sqlite3_file *pReal;
};

/*
** Method declarations for devsym_file.
*/
ਸਥਿਰ ਪੂਰਨ devsymClose(sqlite3_file*);
ਸਥਿਰ ਪੂਰਨ devsymRead(sqlite3_file*, ਖਾਲੀ*, ਪੂਰਨ iAmt, sqlite3_int64 iOfst);
ਸਥਿਰ ਪੂਰਨ devsymWrite(sqlite3_file*,ਅਚਲ ਖਾਲੀ*,ਪੂਰਨ iAmt, sqlite3_int64 iOfst);
ਸਥਿਰ ਪੂਰਨ devsymTruncate(sqlite3_file*, sqlite3_int64 ਮਾਪ);
ਸਥਿਰ ਪੂਰਨ devsymSync(sqlite3_file*, ਪੂਰਨ flags);
ਸਥਿਰ ਪੂਰਨ devsym_File_ਮਾਪ(sqlite3_file*, sqlite3_int64 *p_ਮਾਪ);
ਸਥਿਰ ਪੂਰਨ devsym_ਤਾਲਾ(sqlite3_file*, ਪੂਰਨ);
ਸਥਿਰ ਪੂਰਨ devsymUnlock(sqlite3_file*, ਪੂਰਨ);
ਸਥਿਰ ਪੂਰਨ devsym_Check_Reserved_ਤਾਲਾ(sqlite3_file*, ਪੂਰਨ *);
ਸਥਿਰ ਪੂਰਨ devsymFileControl(sqlite3_file*, ਪੂਰਨ op, ਖਾਲੀ *pArg);
ਸਥਿਰ ਪੂਰਨ devsym_Sector_ਮਾਪ(sqlite3_file*);
ਸਥਿਰ ਪੂਰਨ devsymDeviceCharacteristics(sqlite3_file*);
ਸਥਿਰ ਪੂਰਨ devsym_Shm_ਤਾਲਾ(sqlite3_file*,ਪੂਰਨ,ਪੂਰਨ,ਪੂਰਨ);
ਸਥਿਰ ਪੂਰਨ devsymShmMap(sqlite3_file*,ਪੂਰਨ,ਪੂਰਨ,ਪੂਰਨ, ਖਾਲੀ ਅਸਥਿਰ **);
ਸਥਿਰ ਖਾਲੀ devsymShmBarrier(sqlite3_file*);
ਸਥਿਰ ਪੂਰਨ devsymShmUnmap(sqlite3_file*,ਪੂਰਨ);

/*
** Method declarations for devsym_vfs.
*/
ਸਥਿਰ ਪੂਰਨ devsymOpen(sqlite3_vfs*, ਅਚਲ ਅੱਖਰ *, sqlite3_file*, ਪੂਰਨ , ਪੂਰਨ *);
ਸਥਿਰ ਪੂਰਨ devsymDelete(sqlite3_vfs*, ਅਚਲ ਅੱਖਰ *zName, ਪੂਰਨ syncDir);
ਸਥਿਰ ਪੂਰਨ devsymAccess(sqlite3_vfs*, ਅਚਲ ਅੱਖਰ *zName, ਪੂਰਨ flags, ਪੂਰਨ *);
ਸਥਿਰ ਪੂਰਨ devsym_ਭਰਿਆ_Pathname(sqlite3_vfs*, ਅਚਲ ਅੱਖਰ *zName, ਪੂਰਨ, ਅੱਖਰ *z_ਬਾਹਰਲਾ);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_ਲੱਦੋ_EXTENSION
ਸਥਿਰ ਖਾਲੀ *devsymDlOpen(sqlite3_vfs*, ਅਚਲ ਅੱਖਰ *zFilename);
ਸਥਿਰ ਖਾਲੀ devsymDlError(sqlite3_vfs*, ਪੂਰਨ nByte, ਅੱਖਰ *z_Err_ਸੁਨੇਹਾ);
ਸਥਿਰ ਖਾਲੀ (*devsymDlSym(sqlite3_vfs*,ਖਾਲੀ*, ਅਚਲ ਅੱਖਰ *z_ਚਿੰਨ੍ਹ))(ਖਾਲੀ);
ਸਥਿਰ ਖਾਲੀ devsymDlClose(sqlite3_vfs*, ਖਾਲੀ*);
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_LOAD_EXTENSION */
ਸਥਿਰ ਪੂਰਨ devsymRandomness(sqlite3_vfs*, ਪੂਰਨ nByte, ਅੱਖਰ *z_ਬਾਹਰਲਾ);
ਸਥਿਰ ਪੂਰਨ devsymSleep(sqlite3_vfs*, ਪੂਰਨ microseconds);
ਸਥਿਰ ਪੂਰਨ devsym_Current_ਸਮਾਂ(sqlite3_vfs*, ਦੁੱਗਣਾ*);

ਬਣਤਰ DevsymGlobal {
  sqlite3_vfs *pVfs;
  ਪੂਰਨ iDeviceChar;
  ਪੂਰਨ i_Sector_ਮਾਪ;
  ਪੂਰਨ nWriteCrash;
};
ਬਣਤਰ DevsymGlobal g = {0, 0, 512, 0};

/*
** Close an devsym-file.
*/
ਸਥਿਰ ਪੂਰਨ devsymClose(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  sqlite3OsClose(p->pReal);
  ਵਾਪਸ SQLITE_OK;
}

/*
** Read data from an devsym-file.
*/
ਸਥਿਰ ਪੂਰਨ devsymRead(
  sqlite3_file *pFile, 
  ਖਾਲੀ *zBuf, 
  ਪੂਰਨ iAmt, 
  sqlite_int64 iOfst
){
  devsym_file *p = (devsym_file *)pFile;
  ਵਾਪਸ sqlite3OsRead(p->pReal, zBuf, iAmt, iOfst);
}

/*
** Write data to an devsym-file.
*/
ਸਥਿਰ ਪੂਰਨ devsymWrite(
  sqlite3_file *pFile, 
  ਅਚਲ ਖਾਲੀ *zBuf, 
  ਪੂਰਨ iAmt, 
  sqlite_int64 iOfst
){
  devsym_file *p = (devsym_file *)pFile;
  ਵਾਪਸ sqlite3OsWrite(p->pReal, zBuf, iAmt, iOfst);
}

/*
** Truncate an devsym-file.
*/
ਸਥਿਰ ਪੂਰਨ devsymTruncate(sqlite3_file *pFile, sqlite_int64 ਮਾਪ){
  devsym_file *p = (devsym_file *)pFile;
  ਵਾਪਸ sqlite3OsTruncate(p->pReal, ਮਾਪ);
}

/*
** Sync an devsym-file.
*/
ਸਥਿਰ ਪੂਰਨ devsymSync(sqlite3_file *pFile, ਪੂਰਨ flags){
  devsym_file *p = (devsym_file *)pFile;
  ਵਾਪਸ sqlite3OsSync(p->pReal, flags);
}

/*
** Return the current file-size of an devsym-file.
*/
ਸਥਿਰ ਪੂਰਨ devsym_File_ਮਾਪ(sqlite3_file *pFile, sqlite_int64 *p_ਮਾਪ){
  devsym_file *p = (devsym_file *)pFile;
  ਵਾਪਸ sqlite3_Os_File_ਮਾਪ(p->pReal, p_ਮਾਪ);
}

/*
** Lock an devsym-file.
*/
ਸਥਿਰ ਪੂਰਨ devsym_ਤਾਲਾ(sqlite3_file *pFile, ਪੂਰਨ e_ਤਾਲਾ){
  devsym_file *p = (devsym_file *)pFile;
  ਵਾਪਸ sqlite3_Os_ਤਾਲਾ(p->pReal, e_ਤਾਲਾ);
}

/*
** Unlock an devsym-file.
*/
ਸਥਿਰ ਪੂਰਨ devsymUnlock(sqlite3_file *pFile, ਪੂਰਨ e_ਤਾਲਾ){
  devsym_file *p = (devsym_file *)pFile;
  ਵਾਪਸ sqlite3OsUnlock(p->pReal, e_ਤਾਲਾ);
}

/*
** Check if another file-handle holds a RESERVED lock on an devsym-file.
*/
ਸਥਿਰ ਪੂਰਨ devsym_Check_Reserved_ਤਾਲਾ(sqlite3_file *pFile, ਪੂਰਨ *p_Res_ਬਾਹਰਲਾ){
  devsym_file *p = (devsym_file *)pFile;
  ਵਾਪਸ sqlite3_Os_Check_Reserved_ਤਾਲਾ(p->pReal, p_Res_ਬਾਹਰਲਾ);
}

/*
** File control method. For custom operations on an devsym-file.
*/
ਸਥਿਰ ਪੂਰਨ devsymFileControl(sqlite3_file *pFile, ਪੂਰਨ op, ਖਾਲੀ *pArg){
  devsym_file *p = (devsym_file *)pFile;
  ਵਾਪਸ sqlite3OsFileControl(p->pReal, op, pArg);
}

/*
** Return the sector-size in bytes for an devsym-file.
*/
ਸਥਿਰ ਪੂਰਨ devsym_Sector_ਮਾਪ(sqlite3_file *pFile){
  ਵਾਪਸ g.i_Sector_ਮਾਪ;
}

/*
** Return the device characteristic flags supported by an devsym-file.
*/
ਸਥਿਰ ਪੂਰਨ devsymDeviceCharacteristics(sqlite3_file *pFile){
  ਵਾਪਸ g.iDeviceChar;
}

/*
** Shared-memory methods are all pass-throughs.
*/
ਸਥਿਰ ਪੂਰਨ devsym_Shm_ਤਾਲਾ(sqlite3_file *pFile, ਪੂਰਨ ofst, ਪੂਰਨ n, ਪੂਰਨ flags){
  devsym_file *p = (devsym_file *)pFile;
  ਵਾਪਸ p->pReal->pMethods->x_Shm_ਤਾਲਾ(p->pReal, ofst, n, flags);
}
ਸਥਿਰ ਪੂਰਨ devsymShmMap(
  sqlite3_file *pFile, 
  ਪੂਰਨ iRegion, 
  ਪੂਰਨ szRegion, 
  ਪੂਰਨ isWrite, 
  ਖਾਲੀ ਅਸਥਿਰ **pp
){
  devsym_file *p = (devsym_file *)pFile;
  ਵਾਪਸ p->pReal->pMethods->xShmMap(p->pReal, iRegion, szRegion, isWrite, pp);
}
ਸਥਿਰ ਖਾਲੀ devsymShmBarrier(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  p->pReal->pMethods->xShmBarrier(p->pReal);
}
ਸਥਿਰ ਪੂਰਨ devsymShmUnmap(sqlite3_file *pFile, ਪੂਰਨ delFlag){
  devsym_file *p = (devsym_file *)pFile;
  ਵਾਪਸ p->pReal->pMethods->xShmUnmap(p->pReal, delFlag);
}



/*
** Open an devsym file handle.
*/
ਸਥਿਰ ਪੂਰਨ devsymOpen(
  sqlite3_vfs *pVfs,
  ਅਚਲ ਅੱਖਰ *zName,
  sqlite3_file *pFile,
  ਪੂਰਨ flags,
  ਪੂਰਨ *p_ਬਾਹਰਲਾ_Flags
){
ਸਥਿਰ sqlite3_io_methods devsym_io_methods = {
  2,                                /* iVersion */
  devsymClose,                      /* xClose */
  devsymRead,                       /* xRead */
  devsymWrite,                      /* xWrite */
  devsymTruncate,                   /* xTruncate */
  devsymSync,                       /* xSync */
  devsym_File_ਮਾਪ,                   /* xFileSize */
  devsym_ਤਾਲਾ,                       /* xLock */
  devsymUnlock,                     /* xUnlock */
  devsym_Check_Reserved_ਤਾਲਾ,          /* xCheckReservedLock */
  devsymFileControl,                /* xFileControl */
  devsym_Sector_ਮਾਪ,                 /* xSectorSize */
  devsymDeviceCharacteristics,      /* xDeviceCharacteristics */
  devsymShmMap,                     /* xShmMap */
  devsym_Shm_ਤਾਲਾ,                    /* xShmLock */
  devsymShmBarrier,                 /* xShmBarrier */
  devsymShmUnmap                    /* xShmUnmap */
};

  ਪੂਰਨ rc;
  devsym_file *p = (devsym_file *)pFile;
  p->pReal = (sqlite3_file *)&p[1];
  rc = sqlite3OsOpen(g.pVfs, zName, p->pReal, flags, p_ਬਾਹਰਲਾ_Flags);
  ਜੇ( p->pReal->pMethods ){
    pFile->pMethods = &devsym_io_methods;
  }
  ਵਾਪਸ rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
ਸਥਿਰ ਪੂਰਨ devsymDelete(sqlite3_vfs *pVfs, ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ, ਪੂਰਨ dirSync){
  ਵਾਪਸ sqlite3OsDelete(g.pVfs, z_ਰਸਤਾ, dirSync);
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
ਸਥਿਰ ਪੂਰਨ devsymAccess(
  sqlite3_vfs *pVfs, 
  ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ, 
  ਪੂਰਨ flags, 
  ਪੂਰਨ *p_Res_ਬਾਹਰਲਾ
){
  ਵਾਪਸ sqlite3OsAccess(g.pVfs, z_ਰਸਤਾ, flags, p_Res_ਬਾਹਰਲਾ);
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (DEVSYM_MAX_PATHNAME+1) bytes.
*/
ਸਥਿਰ ਪੂਰਨ devsym_ਭਰਿਆ_Pathname(
  sqlite3_vfs *pVfs, 
  ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ, 
  ਪੂਰਨ n_ਬਾਹਰਲਾ, 
  ਅੱਖਰ *z_ਬਾਹਰਲਾ
){
  ਵਾਪਸ sqlite3_Os_ਭਰਿਆ_Pathname(g.pVfs, z_ਰਸਤਾ, n_ਬਾਹਰਲਾ, z_ਬਾਹਰਲਾ);
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_ਲੱਦੋ_EXTENSION
/*
** Open the dynamic library located at zPath and return a handle.
*/
ਸਥਿਰ ਖਾਲੀ *devsymDlOpen(sqlite3_vfs *pVfs, ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ){
  ਵਾਪਸ sqlite3OsDlOpen(g.pVfs, z_ਰਸਤਾ);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
ਸਥਿਰ ਖਾਲੀ devsymDlError(sqlite3_vfs *pVfs, ਪੂਰਨ nByte, ਅੱਖਰ *z_Err_ਸੁਨੇਹਾ){
  sqlite3OsDlError(g.pVfs, nByte, z_Err_ਸੁਨੇਹਾ);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
ਸਥਿਰ ਖਾਲੀ (*devsymDlSym(sqlite3_vfs *pVfs, ਖਾਲੀ *p, ਅਚਲ ਅੱਖਰ *zSym))(ਖਾਲੀ){
  ਵਾਪਸ sqlite3OsDlSym(g.pVfs, p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
ਸਥਿਰ ਖਾਲੀ devsymDlClose(sqlite3_vfs *pVfs, ਖਾਲੀ *pHandle){
  sqlite3OsDlClose(g.pVfs, pHandle);
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_LOAD_EXTENSION */

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
ਸਥਿਰ ਪੂਰਨ devsymRandomness(sqlite3_vfs *pVfs, ਪੂਰਨ nByte, ਅੱਖਰ *z_Buf_ਬਾਹਰਲਾ){
  ਵਾਪਸ sqlite3OsRandomness(g.pVfs, nByte, z_Buf_ਬਾਹਰਲਾ);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
ਸਥਿਰ ਪੂਰਨ devsymSleep(sqlite3_vfs *pVfs, ਪੂਰਨ nMicro){
  ਵਾਪਸ sqlite3OsSleep(g.pVfs, nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
ਸਥਿਰ ਪੂਰਨ devsym_Current_ਸਮਾਂ(sqlite3_vfs *pVfs, ਦੁੱਗਣਾ *p_ਸਮਾਂ_ਬਾਹਰਲਾ){
  ਵਾਪਸ g.pVfs->x_Current_ਸਮਾਂ(g.pVfs, p_ਸਮਾਂ_ਬਾਹਰਲਾ);
}

/*
** Return the sector-size in bytes for an writecrash-file.
*/
ਸਥਿਰ ਪੂਰਨ writecrash_Sector_ਮਾਪ(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  ਵਾਪਸ sqlite3_Os_Sector_ਮਾਪ(p->pReal);
}

/*
** Return the device characteristic flags supported by an writecrash-file.
*/
ਸਥਿਰ ਪੂਰਨ writecrashDeviceCharacteristics(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  ਵਾਪਸ sqlite3OsDeviceCharacteristics(p->pReal);
}

/*
** Write data to an writecrash-file.
*/
ਸਥਿਰ ਪੂਰਨ writecrashWrite(
  sqlite3_file *pFile, 
  ਅਚਲ ਖਾਲੀ *zBuf, 
  ਪੂਰਨ iAmt, 
  sqlite_int64 iOfst
){
  devsym_file *p = (devsym_file *)pFile;
  ਜੇ( g.nWriteCrash>0 ){
    g.nWriteCrash--;
    ਜੇ( g.nWriteCrash==0 ) abort();
  }
  ਵਾਪਸ sqlite3OsWrite(p->pReal, zBuf, iAmt, iOfst);
}

/*
** Open an writecrash file handle.
*/
ਸਥਿਰ ਪੂਰਨ writecrashOpen(
  sqlite3_vfs *pVfs,
  ਅਚਲ ਅੱਖਰ *zName,
  sqlite3_file *pFile,
  ਪੂਰਨ flags,
  ਪੂਰਨ *p_ਬਾਹਰਲਾ_Flags
){
ਸਥਿਰ sqlite3_io_methods writecrash_io_methods = {
  2,                                /* iVersion */
  devsymClose,                      /* xClose */
  devsymRead,                       /* xRead */
  writecrashWrite,                  /* xWrite */
  devsymTruncate,                   /* xTruncate */
  devsymSync,                       /* xSync */
  devsym_File_ਮਾਪ,                   /* xFileSize */
  devsym_ਤਾਲਾ,                       /* xLock */
  devsymUnlock,                     /* xUnlock */
  devsym_Check_Reserved_ਤਾਲਾ,          /* xCheckReservedLock */
  devsymFileControl,                /* xFileControl */
  writecrash_Sector_ਮਾਪ,             /* xSectorSize */
  writecrashDeviceCharacteristics,  /* xDeviceCharacteristics */
  devsymShmMap,                     /* xShmMap */
  devsym_Shm_ਤਾਲਾ,                    /* xShmLock */
  devsymShmBarrier,                 /* xShmBarrier */
  devsymShmUnmap                    /* xShmUnmap */
};

  ਪੂਰਨ rc;
  devsym_file *p = (devsym_file *)pFile;
  p->pReal = (sqlite3_file *)&p[1];
  rc = sqlite3OsOpen(g.pVfs, zName, p->pReal, flags, p_ਬਾਹਰਲਾ_Flags);
  ਜੇ( p->pReal->pMethods ){
    pFile->pMethods = &writecrash_io_methods;
  }
  ਵਾਪਸ rc;
}

ਸਥਿਰ sqlite3_vfs devsym_vfs = {
  2,                     /* iVersion */
  ਆਕਾਰ(devsym_file),      /* szOsFile */
  DEVSYM_MAX_PATHNAME,      /* mxPathname */
  0,                     /* pNext */
  DEVSYM_VFS_NAME,          /* zName */
  0,                     /* pAppData */
  devsymOpen,               /* xOpen */
  devsymDelete,             /* xDelete */
  devsymAccess,             /* xAccess */
  devsym_ਭਰਿਆ_Pathname,       /* xFullPathname */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_ਲੱਦੋ_EXTENSION
  devsymDlOpen,             /* xDlOpen */
  devsymDlError,            /* xDlError */
  devsymDlSym,              /* xDlSym */
  devsymDlClose,            /* xDlClose */
#ਨਹੀਂਤਾਂ
  0,                        /* xDlOpen */
  0,                        /* xDlError */
  0,                        /* xDlSym */
  0,                        /* xDlClose */
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_LOAD_EXTENSION */
  devsymRandomness,         /* xRandomness */
  devsymSleep,              /* xSleep */
  devsym_Current_ਸਮਾਂ,        /* xCurrentTime */
  0,                        /* xGetLastError */
  0                         /* xCurrentTimeInt64 */
};

ਸਥਿਰ sqlite3_vfs writecrash_vfs = {
  2,                     /* iVersion */
  ਆਕਾਰ(devsym_file),      /* szOsFile */
  DEVSYM_MAX_PATHNAME,      /* mxPathname */
  0,                     /* pNext */
  WRITECRASH_NAME,          /* zName */
  0,                     /* pAppData */
  writecrashOpen,           /* xOpen */
  devsymDelete,             /* xDelete */
  devsymAccess,             /* xAccess */
  devsym_ਭਰਿਆ_Pathname,       /* xFullPathname */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_ਲੱਦੋ_EXTENSION
  devsymDlOpen,             /* xDlOpen */
  devsymDlError,            /* xDlError */
  devsymDlSym,              /* xDlSym */
  devsymDlClose,            /* xDlClose */
#ਨਹੀਂਤਾਂ
  0,                        /* xDlOpen */
  0,                        /* xDlError */
  0,                        /* xDlSym */
  0,                        /* xDlClose */
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_LOAD_EXTENSION */
  devsymRandomness,         /* xRandomness */
  devsymSleep,              /* xSleep */
  devsym_Current_ਸਮਾਂ,        /* xCurrentTime */
  0,                        /* xGetLastError */
  0                         /* xCurrentTimeInt64 */
};


/*
** This procedure registers the devsym vfs with SQLite. If the argument is
** true, the devsym vfs becomes the new default vfs. It is the only publicly
** available function in this file.
*/
ਖਾਲੀ devsym_ਰਜਿਸਟਰ(ਪੂਰਨ iDeviceChar, ਪੂਰਨ i_Sector_ਮਾਪ){

  ਜੇ( g.pVfs==0 ){
    g.pVfs = sqlite3_vfs_ਲੱਭੋ(0);
    devsym_vfs.szOsFile += g.pVfs->szOsFile;
    writecrash_vfs.szOsFile += g.pVfs->szOsFile;
    sqlite3_vfs_ਰਜਿਸਟਰ(&devsym_vfs, 0);
    sqlite3_vfs_ਰਜਿਸਟਰ(&writecrash_vfs, 0);
  }
  ਜੇ( iDeviceChar>=0 ){
    g.iDeviceChar = iDeviceChar;
  }ਨਹੀਂਤਾਂ{
    g.iDeviceChar = 0;
  }
  ਜੇ( i_Sector_ਮਾਪ>=0 ){
    g.i_Sector_ਮਾਪ = i_Sector_ਮਾਪ;
  }ਨਹੀਂਤਾਂ{
    g.i_Sector_ਮਾਪ = 512;
  }
}

ਖਾਲੀ devsym_unregister(){
  sqlite3_vfs_unregister(&devsym_vfs);
  sqlite3_vfs_unregister(&writecrash_vfs);
  g.pVfs = 0;
  g.iDeviceChar = 0;
  g.i_Sector_ਮਾਪ = 0;
}

ਖਾਲੀ devsym_crash_on_write(ਪੂਰਨ nWrite){
  ਜੇ( g.pVfs==0 ){
    g.pVfs = sqlite3_vfs_ਲੱਭੋ(0);
    devsym_vfs.szOsFile += g.pVfs->szOsFile;
    writecrash_vfs.szOsFile += g.pVfs->szOsFile;
    sqlite3_vfs_ਰਜਿਸਟਰ(&devsym_vfs, 0);
    sqlite3_vfs_ਰਜਿਸਟਰ(&writecrash_vfs, 0);
  }
  g.nWriteCrash = nWrite;
}

#ਜੇ_ਅੰਤ
