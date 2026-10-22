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
#إذا SQLITE_TEST          /* This file is used for testing only */

#ضمن "sqlite3.h"
#ضمن "sqliteInt.h"

/*
** Maximum pathname length supported by the devsym backend.
*/
#define DEVSYM_MAX_PATHNAME 512

/*
** Name used to identify this VFS.
*/
#define DEVSYM_VFS_NAME "devsym"
#define WRITECRASH_NAME "writecrash"

تعريفنوع بنية devsym_file devsym_file;
بنية devsym_file {
  sqlite3_file base;
  sqlite3_file *pReal;
};

/*
** Method declarations for devsym_file.
*/
ساكن صحيح devsymClose(sqlite3_file*);
ساكن صحيح devsymRead(sqlite3_file*, فراغ*, صحيح iAmt, sqlite3_int64 iOfst);
ساكن صحيح devsymWrite(sqlite3_file*,ثابت فراغ*,صحيح iAmt, sqlite3_int64 iOfst);
ساكن صحيح devsymTruncate(sqlite3_file*, sqlite3_int64 size);
ساكن صحيح devsymSync(sqlite3_file*, صحيح flags);
ساكن صحيح devsymFileSize(sqlite3_file*, sqlite3_int64 *pSize);
ساكن صحيح devsymLock(sqlite3_file*, صحيح);
ساكن صحيح devsymUnlock(sqlite3_file*, صحيح);
ساكن صحيح devsymCheckReservedLock(sqlite3_file*, صحيح *);
ساكن صحيح devsymFileControl(sqlite3_file*, صحيح op, فراغ *pArg);
ساكن صحيح devsymSectorSize(sqlite3_file*);
ساكن صحيح devsymDeviceCharacteristics(sqlite3_file*);
ساكن صحيح devsymShmLock(sqlite3_file*,صحيح,صحيح,صحيح);
ساكن صحيح devsymShmMap(sqlite3_file*,صحيح,صحيح,صحيح, فراغ متطاير **);
ساكن فراغ devsymShmBarrier(sqlite3_file*);
ساكن صحيح devsymShmUnmap(sqlite3_file*,صحيح);

/*
** Method declarations for devsym_vfs.
*/
ساكن صحيح devsymOpen(sqlite3_vfs*, ثابت حرف *, sqlite3_file*, صحيح , صحيح *);
ساكن صحيح devsymDelete(sqlite3_vfs*, ثابت حرف *zName, صحيح syncDir);
ساكن صحيح devsymAccess(sqlite3_vfs*, ثابت حرف *zName, صحيح flags, صحيح *);
ساكن صحيح devsymFullPathname(sqlite3_vfs*, ثابت حرف *zName, صحيح, حرف *zOut);
#ifndef SQLITE_OMIT_LOAD_EXTENSION
ساكن فراغ *devsymDlOpen(sqlite3_vfs*, ثابت حرف *zFilename);
ساكن فراغ devsymDlError(sqlite3_vfs*, صحيح nByte, حرف *z_Err_ڤسنن);
ساكن فراغ (*devsymDlSym(sqlite3_vfs*,فراغ*, ثابت حرف *z_سيمبول))(فراغ);
ساكن فراغ devsymDlClose(sqlite3_vfs*, فراغ*);
#endif /* SQLITE_OMIT_LOAD_EXTENSION */
ساكن صحيح devsymRandomness(sqlite3_vfs*, صحيح nByte, حرف *zOut);
ساكن صحيح devsymSleep(sqlite3_vfs*, صحيح microseconds);
ساكن صحيح devsymCurrentTime(sqlite3_vfs*, مزدوج*);

بنية DevsymGlobal {
  sqlite3_vfs *pVfs;
  صحيح iDeviceChar;
  صحيح iSectorSize;
  صحيح nWriteCrash;
};
بنية DevsymGlobal g = {0, 0, 512, 0};

/*
** Close an devsym-file.
*/
ساكن صحيح devsymClose(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  sqlite3OsClose(p->pReal);
  أرجع SQLITE_OK;
}

/*
** Read data from an devsym-file.
*/
ساكن صحيح devsymRead(
  sqlite3_file *pFile, 
  فراغ *zBuf, 
  صحيح iAmt, 
  sqlite_int64 iOfst
){
  devsym_file *p = (devsym_file *)pFile;
  أرجع sqlite3OsRead(p->pReal, zBuf, iAmt, iOfst);
}

/*
** Write data to an devsym-file.
*/
ساكن صحيح devsymWrite(
  sqlite3_file *pFile, 
  ثابت فراغ *zBuf, 
  صحيح iAmt, 
  sqlite_int64 iOfst
){
  devsym_file *p = (devsym_file *)pFile;
  أرجع sqlite3OsWrite(p->pReal, zBuf, iAmt, iOfst);
}

/*
** Truncate an devsym-file.
*/
ساكن صحيح devsymTruncate(sqlite3_file *pFile, sqlite_int64 size){
  devsym_file *p = (devsym_file *)pFile;
  أرجع sqlite3OsTruncate(p->pReal, size);
}

/*
** Sync an devsym-file.
*/
ساكن صحيح devsymSync(sqlite3_file *pFile, صحيح flags){
  devsym_file *p = (devsym_file *)pFile;
  أرجع sqlite3OsSync(p->pReal, flags);
}

/*
** Return the current file-size of an devsym-file.
*/
ساكن صحيح devsymFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  devsym_file *p = (devsym_file *)pFile;
  أرجع sqlite3OsFileSize(p->pReal, pSize);
}

/*
** Lock an devsym-file.
*/
ساكن صحيح devsymLock(sqlite3_file *pFile, صحيح eLock){
  devsym_file *p = (devsym_file *)pFile;
  أرجع sqlite3OsLock(p->pReal, eLock);
}

/*
** Unlock an devsym-file.
*/
ساكن صحيح devsymUnlock(sqlite3_file *pFile, صحيح eLock){
  devsym_file *p = (devsym_file *)pFile;
  أرجع sqlite3OsUnlock(p->pReal, eLock);
}

/*
** Check if another file-handle holds a RESERVED lock on an devsym-file.
*/
ساكن صحيح devsymCheckReservedLock(sqlite3_file *pFile, صحيح *pResOut){
  devsym_file *p = (devsym_file *)pFile;
  أرجع sqlite3OsCheckReservedLock(p->pReal, pResOut);
}

/*
** File control method. For custom operations on an devsym-file.
*/
ساكن صحيح devsymFileControl(sqlite3_file *pFile, صحيح op, فراغ *pArg){
  devsym_file *p = (devsym_file *)pFile;
  أرجع sqlite3OsFileControl(p->pReal, op, pArg);
}

/*
** Return the sector-size in bytes for an devsym-file.
*/
ساكن صحيح devsymSectorSize(sqlite3_file *pFile){
  أرجع g.iSectorSize;
}

/*
** Return the device characteristic flags supported by an devsym-file.
*/
ساكن صحيح devsymDeviceCharacteristics(sqlite3_file *pFile){
  أرجع g.iDeviceChar;
}

/*
** Shared-memory methods are all pass-throughs.
*/
ساكن صحيح devsymShmLock(sqlite3_file *pFile, صحيح ofst, صحيح n, صحيح flags){
  devsym_file *p = (devsym_file *)pFile;
  أرجع p->pReal->pMethods->xShmLock(p->pReal, ofst, n, flags);
}
ساكن صحيح devsymShmMap(
  sqlite3_file *pFile, 
  صحيح iRegion, 
  صحيح szRegion, 
  صحيح isWrite, 
  فراغ متطاير **pp
){
  devsym_file *p = (devsym_file *)pFile;
  أرجع p->pReal->pMethods->xShmMap(p->pReal, iRegion, szRegion, isWrite, pp);
}
ساكن فراغ devsymShmBarrier(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  p->pReal->pMethods->xShmBarrier(p->pReal);
}
ساكن صحيح devsymShmUnmap(sqlite3_file *pFile, صحيح delFlag){
  devsym_file *p = (devsym_file *)pFile;
  أرجع p->pReal->pMethods->xShmUnmap(p->pReal, delFlag);
}



/*
** Open an devsym file handle.
*/
ساكن صحيح devsymOpen(
  sqlite3_vfs *pVfs,
  ثابت حرف *zName,
  sqlite3_file *pFile,
  صحيح flags,
  صحيح *pOutFlags
){
ساكن sqlite3_io_methods devsym_io_methods = {
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

  صحيح rc;
  devsym_file *p = (devsym_file *)pFile;
  p->pReal = (sqlite3_file *)&p[1];
  rc = sqlite3OsOpen(g.pVfs, zName, p->pReal, flags, pOutFlags);
  إذا( p->pReal->pMethods ){
    pFile->pMethods = &devsym_io_methods;
  }
  أرجع rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
ساكن صحيح devsymDelete(sqlite3_vfs *pVfs, ثابت حرف *zPath, صحيح dirSync){
  أرجع sqlite3OsDelete(g.pVfs, zPath, dirSync);
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
ساكن صحيح devsymAccess(
  sqlite3_vfs *pVfs, 
  ثابت حرف *zPath, 
  صحيح flags, 
  صحيح *pResOut
){
  أرجع sqlite3OsAccess(g.pVfs, zPath, flags, pResOut);
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (DEVSYM_MAX_PATHNAME+1) bytes.
*/
ساكن صحيح devsymFullPathname(
  sqlite3_vfs *pVfs, 
  ثابت حرف *zPath, 
  صحيح nOut, 
  حرف *zOut
){
  أرجع sqlite3OsFullPathname(g.pVfs, zPath, nOut, zOut);
}

#ifndef SQLITE_OMIT_LOAD_EXTENSION
/*
** Open the dynamic library located at zPath and return a handle.
*/
ساكن فراغ *devsymDlOpen(sqlite3_vfs *pVfs, ثابت حرف *zPath){
  أرجع sqlite3OsDlOpen(g.pVfs, zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
ساكن فراغ devsymDlError(sqlite3_vfs *pVfs, صحيح nByte, حرف *z_Err_ڤسنن){
  sqlite3OsDlError(g.pVfs, nByte, z_Err_ڤسنن);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
ساكن فراغ (*devsymDlSym(sqlite3_vfs *pVfs, فراغ *p, ثابت حرف *zSym))(فراغ){
  أرجع sqlite3OsDlSym(g.pVfs, p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
ساكن فراغ devsymDlClose(sqlite3_vfs *pVfs, فراغ *pHandle){
  sqlite3OsDlClose(g.pVfs, pHandle);
}
#endif /* SQLITE_OMIT_LOAD_EXTENSION */

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
ساكن صحيح devsymRandomness(sqlite3_vfs *pVfs, صحيح nByte, حرف *zBufOut){
  أرجع sqlite3OsRandomness(g.pVfs, nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
ساكن صحيح devsymSleep(sqlite3_vfs *pVfs, صحيح nMicro){
  أرجع sqlite3OsSleep(g.pVfs, nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
ساكن صحيح devsymCurrentTime(sqlite3_vfs *pVfs, مزدوج *pTimeOut){
  أرجع g.pVfs->xCurrentTime(g.pVfs, pTimeOut);
}

/*
** Return the sector-size in bytes for an writecrash-file.
*/
ساكن صحيح writecrashSectorSize(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  أرجع sqlite3OsSectorSize(p->pReal);
}

/*
** Return the device characteristic flags supported by an writecrash-file.
*/
ساكن صحيح writecrashDeviceCharacteristics(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  أرجع sqlite3OsDeviceCharacteristics(p->pReal);
}

/*
** Write data to an writecrash-file.
*/
ساكن صحيح writecrashWrite(
  sqlite3_file *pFile, 
  ثابت فراغ *zBuf, 
  صحيح iAmt, 
  sqlite_int64 iOfst
){
  devsym_file *p = (devsym_file *)pFile;
  إذا( g.nWriteCrash>0 ){
    g.nWriteCrash--;
    إذا( g.nWriteCrash==0 ) abort();
  }
  أرجع sqlite3OsWrite(p->pReal, zBuf, iAmt, iOfst);
}

/*
** Open an writecrash file handle.
*/
ساكن صحيح writecrashOpen(
  sqlite3_vfs *pVfs,
  ثابت حرف *zName,
  sqlite3_file *pFile,
  صحيح flags,
  صحيح *pOutFlags
){
ساكن sqlite3_io_methods writecrash_io_methods = {
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

  صحيح rc;
  devsym_file *p = (devsym_file *)pFile;
  p->pReal = (sqlite3_file *)&p[1];
  rc = sqlite3OsOpen(g.pVfs, zName, p->pReal, flags, pOutFlags);
  إذا( p->pReal->pMethods ){
    pFile->pMethods = &writecrash_io_methods;
  }
  أرجع rc;
}

ساكن sqlite3_vfs devsym_vfs = {
  2,                     /* iVersion */
  حجم(devsym_file),      /* szOsFile */
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
#وإلا
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

ساكن sqlite3_vfs writecrash_vfs = {
  2,                     /* iVersion */
  حجم(devsym_file),      /* szOsFile */
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
#وإلا
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
فراغ devsym_register(صحيح iDeviceChar, صحيح iSectorSize){

  إذا( g.pVfs==0 ){
    g.pVfs = sqlite3_vfs_find(0);
    devsym_vfs.szOsFile += g.pVfs->szOsFile;
    writecrash_vfs.szOsFile += g.pVfs->szOsFile;
    sqlite3_vfs_register(&devsym_vfs, 0);
    sqlite3_vfs_register(&writecrash_vfs, 0);
  }
  إذا( iDeviceChar>=0 ){
    g.iDeviceChar = iDeviceChar;
  }وإلا{
    g.iDeviceChar = 0;
  }
  إذا( iSectorSize>=0 ){
    g.iSectorSize = iSectorSize;
  }وإلا{
    g.iSectorSize = 512;
  }
}

فراغ devsym_unregister(){
  sqlite3_vfs_unregister(&devsym_vfs);
  sqlite3_vfs_unregister(&writecrash_vfs);
  g.pVfs = 0;
  g.iDeviceChar = 0;
  g.iSectorSize = 0;
}

فراغ devsym_crash_on_write(صحيح nWrite){
  إذا( g.pVfs==0 ){
    g.pVfs = sqlite3_vfs_find(0);
    devsym_vfs.szOsFile += g.pVfs->szOsFile;
    writecrash_vfs.szOsFile += g.pVfs->szOsFile;
    sqlite3_vfs_register(&devsym_vfs, 0);
    sqlite3_vfs_register(&writecrash_vfs, 0);
  }
  g.nWriteCrash = nWrite;
}

#endif
