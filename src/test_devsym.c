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
#എങ്കിൽ SQLITE_TEST          /* This file is used for testing only */

#അന്തര്ഭാവ "sqlite3.h"
#അന്തര്ഭാവ "sqliteInt.h"

/*
** Maximum pathname length supported by the devsym backend.
*/
#define DEVSYM_MAX_PATHNAME 512

/*
** Name used to identify this VFS.
*/
#define DEVSYM_VFS_NAME "devsym"
#define WRITECRASH_NAME "writecrash"

തരനിർവചനം ഘടന devsym_file devsym_file;
ഘടന devsym_file {
  sqlite3_file base;
  sqlite3_file *pReal;
};

/*
** Method declarations for devsym_file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ devsymClose(sqlite3_file*);
സ്ഥിതം പൂർണ്ണസംഖ്യ devsymRead(sqlite3_file*, ശൂന്യം*, പൂർണ്ണസംഖ്യ iAmt, sqlite3_int64 iOfst);
സ്ഥിതം പൂർണ്ണസംഖ്യ devsymWrite(sqlite3_file*,സ്ഥിരം ശൂന്യം*,പൂർണ്ണസംഖ്യ iAmt, sqlite3_int64 iOfst);
സ്ഥിതം പൂർണ്ണസംഖ്യ devsymTruncate(sqlite3_file*, sqlite3_int64 size);
സ്ഥിതം പൂർണ്ണസംഖ്യ devsymSync(sqlite3_file*, പൂർണ്ണസംഖ്യ flags);
സ്ഥിതം പൂർണ്ണസംഖ്യ devsymFileSize(sqlite3_file*, sqlite3_int64 *pSize);
സ്ഥിതം പൂർണ്ണസംഖ്യ devsymLock(sqlite3_file*, പൂർണ്ണസംഖ്യ);
സ്ഥിതം പൂർണ്ണസംഖ്യ devsymUnlock(sqlite3_file*, പൂർണ്ണസംഖ്യ);
സ്ഥിതം പൂർണ്ണസംഖ്യ devsymCheckReservedLock(sqlite3_file*, പൂർണ്ണസംഖ്യ *);
സ്ഥിതം പൂർണ്ണസംഖ്യ devsymFileControl(sqlite3_file*, പൂർണ്ണസംഖ്യ op, ശൂന്യം *pArg);
സ്ഥിതം പൂർണ്ണസംഖ്യ devsymSectorSize(sqlite3_file*);
സ്ഥിതം പൂർണ്ണസംഖ്യ devsymDeviceCharacteristics(sqlite3_file*);
സ്ഥിതം പൂർണ്ണസംഖ്യ devsymShmLock(sqlite3_file*,പൂർണ്ണസംഖ്യ,പൂർണ്ണസംഖ്യ,പൂർണ്ണസംഖ്യ);
സ്ഥിതം പൂർണ്ണസംഖ്യ devsymShmMap(sqlite3_file*,പൂർണ്ണസംഖ്യ,പൂർണ്ണസംഖ്യ,പൂർണ്ണസംഖ്യ, ശൂന്യം അസ്ഥിര **);
സ്ഥിതം ശൂന്യം devsymShmBarrier(sqlite3_file*);
സ്ഥിതം പൂർണ്ണസംഖ്യ devsymShmUnmap(sqlite3_file*,പൂർണ്ണസംഖ്യ);

/*
** Method declarations for devsym_vfs.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ devsymOpen(sqlite3_vfs*, സ്ഥിരം അക്ഷരം *, sqlite3_file*, പൂർണ്ണസംഖ്യ , പൂർണ്ണസംഖ്യ *);
സ്ഥിതം പൂർണ്ണസംഖ്യ devsymDelete(sqlite3_vfs*, സ്ഥിരം അക്ഷരം *zName, പൂർണ്ണസംഖ്യ syncDir);
സ്ഥിതം പൂർണ്ണസംഖ്യ devsymAccess(sqlite3_vfs*, സ്ഥിരം അക്ഷരം *zName, പൂർണ്ണസംഖ്യ flags, പൂർണ്ണസംഖ്യ *);
സ്ഥിതം പൂർണ്ണസംഖ്യ devsymFullPathname(sqlite3_vfs*, സ്ഥിരം അക്ഷരം *zName, പൂർണ്ണസംഖ്യ, അക്ഷരം *zOut);
#ifndef SQLITE_OMIT_LOAD_EXTENSION
സ്ഥിതം ശൂന്യം *devsymDlOpen(sqlite3_vfs*, സ്ഥിരം അക്ഷരം *zFilename);
സ്ഥിതം ശൂന്യം devsymDlError(sqlite3_vfs*, പൂർണ്ണസംഖ്യ nByte, അക്ഷരം *z_Err_സന്ദേശം);
സ്ഥിതം ശൂന്യം (*devsymDlSym(sqlite3_vfs*,ശൂന്യം*, സ്ഥിരം അക്ഷരം *z_ചിഹ്നം))(ശൂന്യം);
സ്ഥിതം ശൂന്യം devsymDlClose(sqlite3_vfs*, ശൂന്യം*);
#endif /* SQLITE_OMIT_LOAD_EXTENSION */
സ്ഥിതം പൂർണ്ണസംഖ്യ devsymRandomness(sqlite3_vfs*, പൂർണ്ണസംഖ്യ nByte, അക്ഷരം *zOut);
സ്ഥിതം പൂർണ്ണസംഖ്യ devsymSleep(sqlite3_vfs*, പൂർണ്ണസംഖ്യ microseconds);
സ്ഥിതം പൂർണ്ണസംഖ്യ devsymCurrentTime(sqlite3_vfs*, ഇരട്ട*);

ഘടന DevsymGlobal {
  sqlite3_vfs *pVfs;
  പൂർണ്ണസംഖ്യ iDeviceChar;
  പൂർണ്ണസംഖ്യ iSectorSize;
  പൂർണ്ണസംഖ്യ nWriteCrash;
};
ഘടന DevsymGlobal g = {0, 0, 512, 0};

/*
** Close an devsym-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ devsymClose(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  sqlite3OsClose(p->pReal);
  തിരികെ SQLITE_OK;
}

/*
** Read data from an devsym-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ devsymRead(
  sqlite3_file *pFile, 
  ശൂന്യം *zBuf, 
  പൂർണ്ണസംഖ്യ iAmt, 
  sqlite_int64 iOfst
){
  devsym_file *p = (devsym_file *)pFile;
  തിരികെ sqlite3OsRead(p->pReal, zBuf, iAmt, iOfst);
}

/*
** Write data to an devsym-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ devsymWrite(
  sqlite3_file *pFile, 
  സ്ഥിരം ശൂന്യം *zBuf, 
  പൂർണ്ണസംഖ്യ iAmt, 
  sqlite_int64 iOfst
){
  devsym_file *p = (devsym_file *)pFile;
  തിരികെ sqlite3OsWrite(p->pReal, zBuf, iAmt, iOfst);
}

/*
** Truncate an devsym-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ devsymTruncate(sqlite3_file *pFile, sqlite_int64 size){
  devsym_file *p = (devsym_file *)pFile;
  തിരികെ sqlite3OsTruncate(p->pReal, size);
}

/*
** Sync an devsym-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ devsymSync(sqlite3_file *pFile, പൂർണ്ണസംഖ്യ flags){
  devsym_file *p = (devsym_file *)pFile;
  തിരികെ sqlite3OsSync(p->pReal, flags);
}

/*
** Return the current file-size of an devsym-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ devsymFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  devsym_file *p = (devsym_file *)pFile;
  തിരികെ sqlite3OsFileSize(p->pReal, pSize);
}

/*
** Lock an devsym-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ devsymLock(sqlite3_file *pFile, പൂർണ്ണസംഖ്യ eLock){
  devsym_file *p = (devsym_file *)pFile;
  തിരികെ sqlite3OsLock(p->pReal, eLock);
}

/*
** Unlock an devsym-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ devsymUnlock(sqlite3_file *pFile, പൂർണ്ണസംഖ്യ eLock){
  devsym_file *p = (devsym_file *)pFile;
  തിരികെ sqlite3OsUnlock(p->pReal, eLock);
}

/*
** Check if another file-handle holds a RESERVED lock on an devsym-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ devsymCheckReservedLock(sqlite3_file *pFile, പൂർണ്ണസംഖ്യ *pResOut){
  devsym_file *p = (devsym_file *)pFile;
  തിരികെ sqlite3OsCheckReservedLock(p->pReal, pResOut);
}

/*
** File control method. For custom operations on an devsym-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ devsymFileControl(sqlite3_file *pFile, പൂർണ്ണസംഖ്യ op, ശൂന്യം *pArg){
  devsym_file *p = (devsym_file *)pFile;
  തിരികെ sqlite3OsFileControl(p->pReal, op, pArg);
}

/*
** Return the sector-size in bytes for an devsym-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ devsymSectorSize(sqlite3_file *pFile){
  തിരികെ g.iSectorSize;
}

/*
** Return the device characteristic flags supported by an devsym-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ devsymDeviceCharacteristics(sqlite3_file *pFile){
  തിരികെ g.iDeviceChar;
}

/*
** Shared-memory methods are all pass-throughs.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ devsymShmLock(sqlite3_file *pFile, പൂർണ്ണസംഖ്യ ofst, പൂർണ്ണസംഖ്യ n, പൂർണ്ണസംഖ്യ flags){
  devsym_file *p = (devsym_file *)pFile;
  തിരികെ p->pReal->pMethods->xShmLock(p->pReal, ofst, n, flags);
}
സ്ഥിതം പൂർണ്ണസംഖ്യ devsymShmMap(
  sqlite3_file *pFile, 
  പൂർണ്ണസംഖ്യ iRegion, 
  പൂർണ്ണസംഖ്യ szRegion, 
  പൂർണ്ണസംഖ്യ isWrite, 
  ശൂന്യം അസ്ഥിര **pp
){
  devsym_file *p = (devsym_file *)pFile;
  തിരികെ p->pReal->pMethods->xShmMap(p->pReal, iRegion, szRegion, isWrite, pp);
}
സ്ഥിതം ശൂന്യം devsymShmBarrier(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  p->pReal->pMethods->xShmBarrier(p->pReal);
}
സ്ഥിതം പൂർണ്ണസംഖ്യ devsymShmUnmap(sqlite3_file *pFile, പൂർണ്ണസംഖ്യ delFlag){
  devsym_file *p = (devsym_file *)pFile;
  തിരികെ p->pReal->pMethods->xShmUnmap(p->pReal, delFlag);
}



/*
** Open an devsym file handle.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ devsymOpen(
  sqlite3_vfs *pVfs,
  സ്ഥിരം അക്ഷരം *zName,
  sqlite3_file *pFile,
  പൂർണ്ണസംഖ്യ flags,
  പൂർണ്ണസംഖ്യ *pOutFlags
){
സ്ഥിതം sqlite3_io_methods devsym_io_methods = {
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

  പൂർണ്ണസംഖ്യ rc;
  devsym_file *p = (devsym_file *)pFile;
  p->pReal = (sqlite3_file *)&p[1];
  rc = sqlite3OsOpen(g.pVfs, zName, p->pReal, flags, pOutFlags);
  എങ്കിൽ( p->pReal->pMethods ){
    pFile->pMethods = &devsym_io_methods;
  }
  തിരികെ rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ devsymDelete(sqlite3_vfs *pVfs, സ്ഥിരം അക്ഷരം *zPath, പൂർണ്ണസംഖ്യ dirSync){
  തിരികെ sqlite3OsDelete(g.pVfs, zPath, dirSync);
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ devsymAccess(
  sqlite3_vfs *pVfs, 
  സ്ഥിരം അക്ഷരം *zPath, 
  പൂർണ്ണസംഖ്യ flags, 
  പൂർണ്ണസംഖ്യ *pResOut
){
  തിരികെ sqlite3OsAccess(g.pVfs, zPath, flags, pResOut);
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (DEVSYM_MAX_PATHNAME+1) bytes.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ devsymFullPathname(
  sqlite3_vfs *pVfs, 
  സ്ഥിരം അക്ഷരം *zPath, 
  പൂർണ്ണസംഖ്യ nOut, 
  അക്ഷരം *zOut
){
  തിരികെ sqlite3OsFullPathname(g.pVfs, zPath, nOut, zOut);
}

#ifndef SQLITE_OMIT_LOAD_EXTENSION
/*
** Open the dynamic library located at zPath and return a handle.
*/
സ്ഥിതം ശൂന്യം *devsymDlOpen(sqlite3_vfs *pVfs, സ്ഥിരം അക്ഷരം *zPath){
  തിരികെ sqlite3OsDlOpen(g.pVfs, zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
സ്ഥിതം ശൂന്യം devsymDlError(sqlite3_vfs *pVfs, പൂർണ്ണസംഖ്യ nByte, അക്ഷരം *z_Err_സന്ദേശം){
  sqlite3OsDlError(g.pVfs, nByte, z_Err_സന്ദേശം);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
സ്ഥിതം ശൂന്യം (*devsymDlSym(sqlite3_vfs *pVfs, ശൂന്യം *p, സ്ഥിരം അക്ഷരം *zSym))(ശൂന്യം){
  തിരികെ sqlite3OsDlSym(g.pVfs, p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
സ്ഥിതം ശൂന്യം devsymDlClose(sqlite3_vfs *pVfs, ശൂന്യം *pHandle){
  sqlite3OsDlClose(g.pVfs, pHandle);
}
#endif /* SQLITE_OMIT_LOAD_EXTENSION */

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ devsymRandomness(sqlite3_vfs *pVfs, പൂർണ്ണസംഖ്യ nByte, അക്ഷരം *zBufOut){
  തിരികെ sqlite3OsRandomness(g.pVfs, nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ devsymSleep(sqlite3_vfs *pVfs, പൂർണ്ണസംഖ്യ nMicro){
  തിരികെ sqlite3OsSleep(g.pVfs, nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ devsymCurrentTime(sqlite3_vfs *pVfs, ഇരട്ട *pTimeOut){
  തിരികെ g.pVfs->xCurrentTime(g.pVfs, pTimeOut);
}

/*
** Return the sector-size in bytes for an writecrash-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ writecrashSectorSize(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  തിരികെ sqlite3OsSectorSize(p->pReal);
}

/*
** Return the device characteristic flags supported by an writecrash-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ writecrashDeviceCharacteristics(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  തിരികെ sqlite3OsDeviceCharacteristics(p->pReal);
}

/*
** Write data to an writecrash-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ writecrashWrite(
  sqlite3_file *pFile, 
  സ്ഥിരം ശൂന്യം *zBuf, 
  പൂർണ്ണസംഖ്യ iAmt, 
  sqlite_int64 iOfst
){
  devsym_file *p = (devsym_file *)pFile;
  എങ്കിൽ( g.nWriteCrash>0 ){
    g.nWriteCrash--;
    എങ്കിൽ( g.nWriteCrash==0 ) abort();
  }
  തിരികെ sqlite3OsWrite(p->pReal, zBuf, iAmt, iOfst);
}

/*
** Open an writecrash file handle.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ writecrashOpen(
  sqlite3_vfs *pVfs,
  സ്ഥിരം അക്ഷരം *zName,
  sqlite3_file *pFile,
  പൂർണ്ണസംഖ്യ flags,
  പൂർണ്ണസംഖ്യ *pOutFlags
){
സ്ഥിതം sqlite3_io_methods writecrash_io_methods = {
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

  പൂർണ്ണസംഖ്യ rc;
  devsym_file *p = (devsym_file *)pFile;
  p->pReal = (sqlite3_file *)&p[1];
  rc = sqlite3OsOpen(g.pVfs, zName, p->pReal, flags, pOutFlags);
  എങ്കിൽ( p->pReal->pMethods ){
    pFile->pMethods = &writecrash_io_methods;
  }
  തിരികെ rc;
}

സ്ഥിതം sqlite3_vfs devsym_vfs = {
  2,                     /* iVersion */
  വലുപ്പം(devsym_file),      /* szOsFile */
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
#അല്ലെങ്കിൽ
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

സ്ഥിതം sqlite3_vfs writecrash_vfs = {
  2,                     /* iVersion */
  വലുപ്പം(devsym_file),      /* szOsFile */
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
#അല്ലെങ്കിൽ
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
ശൂന്യം devsym_register(പൂർണ്ണസംഖ്യ iDeviceChar, പൂർണ്ണസംഖ്യ iSectorSize){

  എങ്കിൽ( g.pVfs==0 ){
    g.pVfs = sqlite3_vfs_find(0);
    devsym_vfs.szOsFile += g.pVfs->szOsFile;
    writecrash_vfs.szOsFile += g.pVfs->szOsFile;
    sqlite3_vfs_register(&devsym_vfs, 0);
    sqlite3_vfs_register(&writecrash_vfs, 0);
  }
  എങ്കിൽ( iDeviceChar>=0 ){
    g.iDeviceChar = iDeviceChar;
  }അല്ലെങ്കിൽ{
    g.iDeviceChar = 0;
  }
  എങ്കിൽ( iSectorSize>=0 ){
    g.iSectorSize = iSectorSize;
  }അല്ലെങ്കിൽ{
    g.iSectorSize = 512;
  }
}

ശൂന്യം devsym_unregister(){
  sqlite3_vfs_unregister(&devsym_vfs);
  sqlite3_vfs_unregister(&writecrash_vfs);
  g.pVfs = 0;
  g.iDeviceChar = 0;
  g.iSectorSize = 0;
}

ശൂന്യം devsym_crash_on_write(പൂർണ്ണസംഖ്യ nWrite){
  എങ്കിൽ( g.pVfs==0 ){
    g.pVfs = sqlite3_vfs_find(0);
    devsym_vfs.szOsFile += g.pVfs->szOsFile;
    writecrash_vfs.szOsFile += g.pVfs->szOsFile;
    sqlite3_vfs_register(&devsym_vfs, 0);
    sqlite3_vfs_register(&writecrash_vfs, 0);
  }
  g.nWriteCrash = nWrite;
}

#endif
