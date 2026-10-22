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
#যদি SQLITE_TEST          /* This file is used for testing only */

#অন্তর্ভাব "sqlite3.h"
#অন্তর্ভাব "sqliteInt.h"

/*
** Maximum pathname length supported by the devsym backend.
*/
#define DEVSYM_MAX_PATHNAME 512

/*
** Name used to identify this VFS.
*/
#define DEVSYM_VFS_NAME "devsym"
#define WRITECRASH_NAME "writecrash"

প্রকারনির্দেশ সংরচনা devsym_file devsym_file;
সংরচনা devsym_file {
  sqlite3_file base;
  sqlite3_file *pReal;
};

/*
** Method declarations for devsym_file.
*/
স্থির পূর্ণাঙ্ক devsymClose(sqlite3_file*);
স্থির পূর্ণাঙ্ক devsymRead(sqlite3_file*, শূন্য*, পূর্ণাঙ্ক iAmt, sqlite3_int64 iOfst);
স্থির পূর্ণাঙ্ক devsymWrite(sqlite3_file*,ধ্রুব শূন্য*,পূর্ণাঙ্ক iAmt, sqlite3_int64 iOfst);
স্থির পূর্ণাঙ্ক devsymTruncate(sqlite3_file*, sqlite3_int64 size);
স্থির পূর্ণাঙ্ক devsymSync(sqlite3_file*, পূর্ণাঙ্ক flags);
স্থির পূর্ণাঙ্ক devsymFileSize(sqlite3_file*, sqlite3_int64 *pSize);
স্থির পূর্ণাঙ্ক devsymLock(sqlite3_file*, পূর্ণাঙ্ক);
স্থির পূর্ণাঙ্ক devsymUnlock(sqlite3_file*, পূর্ণাঙ্ক);
স্থির পূর্ণাঙ্ক devsymCheckReservedLock(sqlite3_file*, পূর্ণাঙ্ক *);
স্থির পূর্ণাঙ্ক devsymFileControl(sqlite3_file*, পূর্ণাঙ্ক op, শূন্য *pArg);
স্থির পূর্ণাঙ্ক devsymSectorSize(sqlite3_file*);
স্থির পূর্ণাঙ্ক devsymDeviceCharacteristics(sqlite3_file*);
স্থির পূর্ণাঙ্ক devsymShmLock(sqlite3_file*,পূর্ণাঙ্ক,পূর্ণাঙ্ক,পূর্ণাঙ্ক);
স্থির পূর্ণাঙ্ক devsymShmMap(sqlite3_file*,পূর্ণাঙ্ক,পূর্ণাঙ্ক,পূর্ণাঙ্ক, শূন্য চঞ্চল **);
স্থির শূন্য devsymShmBarrier(sqlite3_file*);
স্থির পূর্ণাঙ্ক devsymShmUnmap(sqlite3_file*,পূর্ণাঙ্ক);

/*
** Method declarations for devsym_vfs.
*/
স্থির পূর্ণাঙ্ক devsymOpen(sqlite3_vfs*, ধ্রুব বর্ণ *, sqlite3_file*, পূর্ণাঙ্ক , পূর্ণাঙ্ক *);
স্থির পূর্ণাঙ্ক devsymDelete(sqlite3_vfs*, ধ্রুব বর্ণ *zName, পূর্ণাঙ্ক syncDir);
স্থির পূর্ণাঙ্ক devsymAccess(sqlite3_vfs*, ধ্রুব বর্ণ *zName, পূর্ণাঙ্ক flags, পূর্ণাঙ্ক *);
স্থির পূর্ণাঙ্ক devsymFullPathname(sqlite3_vfs*, ধ্রুব বর্ণ *zName, পূর্ণাঙ্ক, বর্ণ *zOut);
#ifndef SQLITE_OMIT_LOAD_EXTENSION
স্থির শূন্য *devsymDlOpen(sqlite3_vfs*, ধ্রুব বর্ণ *zFilename);
স্থির শূন্য devsymDlError(sqlite3_vfs*, পূর্ণাঙ্ক nByte, বর্ণ *z_Err_বার্তা);
স্থির শূন্য (*devsymDlSym(sqlite3_vfs*,শূন্য*, ধ্রুব বর্ণ *z_প্রতীক))(শূন্য);
স্থির শূন্য devsymDlClose(sqlite3_vfs*, শূন্য*);
#endif /* SQLITE_OMIT_LOAD_EXTENSION */
স্থির পূর্ণাঙ্ক devsymRandomness(sqlite3_vfs*, পূর্ণাঙ্ক nByte, বর্ণ *zOut);
স্থির পূর্ণাঙ্ক devsymSleep(sqlite3_vfs*, পূর্ণাঙ্ক microseconds);
স্থির পূর্ণাঙ্ক devsymCurrentTime(sqlite3_vfs*, দ্বিগুণ*);

সংরচনা DevsymGlobal {
  sqlite3_vfs *pVfs;
  পূর্ণাঙ্ক iDeviceChar;
  পূর্ণাঙ্ক iSectorSize;
  পূর্ণাঙ্ক nWriteCrash;
};
সংরচনা DevsymGlobal g = {0, 0, 512, 0};

/*
** Close an devsym-file.
*/
স্থির পূর্ণাঙ্ক devsymClose(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  sqlite3OsClose(p->pReal);
  প্রত্যাগম SQLITE_OK;
}

/*
** Read data from an devsym-file.
*/
স্থির পূর্ণাঙ্ক devsymRead(
  sqlite3_file *pFile, 
  শূন্য *zBuf, 
  পূর্ণাঙ্ক iAmt, 
  sqlite_int64 iOfst
){
  devsym_file *p = (devsym_file *)pFile;
  প্রত্যাগম sqlite3OsRead(p->pReal, zBuf, iAmt, iOfst);
}

/*
** Write data to an devsym-file.
*/
স্থির পূর্ণাঙ্ক devsymWrite(
  sqlite3_file *pFile, 
  ধ্রুব শূন্য *zBuf, 
  পূর্ণাঙ্ক iAmt, 
  sqlite_int64 iOfst
){
  devsym_file *p = (devsym_file *)pFile;
  প্রত্যাগম sqlite3OsWrite(p->pReal, zBuf, iAmt, iOfst);
}

/*
** Truncate an devsym-file.
*/
স্থির পূর্ণাঙ্ক devsymTruncate(sqlite3_file *pFile, sqlite_int64 size){
  devsym_file *p = (devsym_file *)pFile;
  প্রত্যাগম sqlite3OsTruncate(p->pReal, size);
}

/*
** Sync an devsym-file.
*/
স্থির পূর্ণাঙ্ক devsymSync(sqlite3_file *pFile, পূর্ণাঙ্ক flags){
  devsym_file *p = (devsym_file *)pFile;
  প্রত্যাগম sqlite3OsSync(p->pReal, flags);
}

/*
** Return the current file-size of an devsym-file.
*/
স্থির পূর্ণাঙ্ক devsymFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  devsym_file *p = (devsym_file *)pFile;
  প্রত্যাগম sqlite3OsFileSize(p->pReal, pSize);
}

/*
** Lock an devsym-file.
*/
স্থির পূর্ণাঙ্ক devsymLock(sqlite3_file *pFile, পূর্ণাঙ্ক eLock){
  devsym_file *p = (devsym_file *)pFile;
  প্রত্যাগম sqlite3OsLock(p->pReal, eLock);
}

/*
** Unlock an devsym-file.
*/
স্থির পূর্ণাঙ্ক devsymUnlock(sqlite3_file *pFile, পূর্ণাঙ্ক eLock){
  devsym_file *p = (devsym_file *)pFile;
  প্রত্যাগম sqlite3OsUnlock(p->pReal, eLock);
}

/*
** Check if another file-handle holds a RESERVED lock on an devsym-file.
*/
স্থির পূর্ণাঙ্ক devsymCheckReservedLock(sqlite3_file *pFile, পূর্ণাঙ্ক *pResOut){
  devsym_file *p = (devsym_file *)pFile;
  প্রত্যাগম sqlite3OsCheckReservedLock(p->pReal, pResOut);
}

/*
** File control method. For custom operations on an devsym-file.
*/
স্থির পূর্ণাঙ্ক devsymFileControl(sqlite3_file *pFile, পূর্ণাঙ্ক op, শূন্য *pArg){
  devsym_file *p = (devsym_file *)pFile;
  প্রত্যাগম sqlite3OsFileControl(p->pReal, op, pArg);
}

/*
** Return the sector-size in bytes for an devsym-file.
*/
স্থির পূর্ণাঙ্ক devsymSectorSize(sqlite3_file *pFile){
  প্রত্যাগম g.iSectorSize;
}

/*
** Return the device characteristic flags supported by an devsym-file.
*/
স্থির পূর্ণাঙ্ক devsymDeviceCharacteristics(sqlite3_file *pFile){
  প্রত্যাগম g.iDeviceChar;
}

/*
** Shared-memory methods are all pass-throughs.
*/
স্থির পূর্ণাঙ্ক devsymShmLock(sqlite3_file *pFile, পূর্ণাঙ্ক ofst, পূর্ণাঙ্ক n, পূর্ণাঙ্ক flags){
  devsym_file *p = (devsym_file *)pFile;
  প্রত্যাগম p->pReal->pMethods->xShmLock(p->pReal, ofst, n, flags);
}
স্থির পূর্ণাঙ্ক devsymShmMap(
  sqlite3_file *pFile, 
  পূর্ণাঙ্ক iRegion, 
  পূর্ণাঙ্ক szRegion, 
  পূর্ণাঙ্ক isWrite, 
  শূন্য চঞ্চল **pp
){
  devsym_file *p = (devsym_file *)pFile;
  প্রত্যাগম p->pReal->pMethods->xShmMap(p->pReal, iRegion, szRegion, isWrite, pp);
}
স্থির শূন্য devsymShmBarrier(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  p->pReal->pMethods->xShmBarrier(p->pReal);
}
স্থির পূর্ণাঙ্ক devsymShmUnmap(sqlite3_file *pFile, পূর্ণাঙ্ক delFlag){
  devsym_file *p = (devsym_file *)pFile;
  প্রত্যাগম p->pReal->pMethods->xShmUnmap(p->pReal, delFlag);
}



/*
** Open an devsym file handle.
*/
স্থির পূর্ণাঙ্ক devsymOpen(
  sqlite3_vfs *pVfs,
  ধ্রুব বর্ণ *zName,
  sqlite3_file *pFile,
  পূর্ণাঙ্ক flags,
  পূর্ণাঙ্ক *pOutFlags
){
স্থির sqlite3_io_methods devsym_io_methods = {
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

  পূর্ণাঙ্ক rc;
  devsym_file *p = (devsym_file *)pFile;
  p->pReal = (sqlite3_file *)&p[1];
  rc = sqlite3OsOpen(g.pVfs, zName, p->pReal, flags, pOutFlags);
  যদি( p->pReal->pMethods ){
    pFile->pMethods = &devsym_io_methods;
  }
  প্রত্যাগম rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
স্থির পূর্ণাঙ্ক devsymDelete(sqlite3_vfs *pVfs, ধ্রুব বর্ণ *zPath, পূর্ণাঙ্ক dirSync){
  প্রত্যাগম sqlite3OsDelete(g.pVfs, zPath, dirSync);
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
স্থির পূর্ণাঙ্ক devsymAccess(
  sqlite3_vfs *pVfs, 
  ধ্রুব বর্ণ *zPath, 
  পূর্ণাঙ্ক flags, 
  পূর্ণাঙ্ক *pResOut
){
  প্রত্যাগম sqlite3OsAccess(g.pVfs, zPath, flags, pResOut);
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (DEVSYM_MAX_PATHNAME+1) bytes.
*/
স্থির পূর্ণাঙ্ক devsymFullPathname(
  sqlite3_vfs *pVfs, 
  ধ্রুব বর্ণ *zPath, 
  পূর্ণাঙ্ক nOut, 
  বর্ণ *zOut
){
  প্রত্যাগম sqlite3OsFullPathname(g.pVfs, zPath, nOut, zOut);
}

#ifndef SQLITE_OMIT_LOAD_EXTENSION
/*
** Open the dynamic library located at zPath and return a handle.
*/
স্থির শূন্য *devsymDlOpen(sqlite3_vfs *pVfs, ধ্রুব বর্ণ *zPath){
  প্রত্যাগম sqlite3OsDlOpen(g.pVfs, zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
স্থির শূন্য devsymDlError(sqlite3_vfs *pVfs, পূর্ণাঙ্ক nByte, বর্ণ *z_Err_বার্তা){
  sqlite3OsDlError(g.pVfs, nByte, z_Err_বার্তা);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
স্থির শূন্য (*devsymDlSym(sqlite3_vfs *pVfs, শূন্য *p, ধ্রুব বর্ণ *zSym))(শূন্য){
  প্রত্যাগম sqlite3OsDlSym(g.pVfs, p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
স্থির শূন্য devsymDlClose(sqlite3_vfs *pVfs, শূন্য *pHandle){
  sqlite3OsDlClose(g.pVfs, pHandle);
}
#endif /* SQLITE_OMIT_LOAD_EXTENSION */

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
স্থির পূর্ণাঙ্ক devsymRandomness(sqlite3_vfs *pVfs, পূর্ণাঙ্ক nByte, বর্ণ *zBufOut){
  প্রত্যাগম sqlite3OsRandomness(g.pVfs, nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
স্থির পূর্ণাঙ্ক devsymSleep(sqlite3_vfs *pVfs, পূর্ণাঙ্ক nMicro){
  প্রত্যাগম sqlite3OsSleep(g.pVfs, nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
স্থির পূর্ণাঙ্ক devsymCurrentTime(sqlite3_vfs *pVfs, দ্বিগুণ *pTimeOut){
  প্রত্যাগম g.pVfs->xCurrentTime(g.pVfs, pTimeOut);
}

/*
** Return the sector-size in bytes for an writecrash-file.
*/
স্থির পূর্ণাঙ্ক writecrashSectorSize(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  প্রত্যাগম sqlite3OsSectorSize(p->pReal);
}

/*
** Return the device characteristic flags supported by an writecrash-file.
*/
স্থির পূর্ণাঙ্ক writecrashDeviceCharacteristics(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  প্রত্যাগম sqlite3OsDeviceCharacteristics(p->pReal);
}

/*
** Write data to an writecrash-file.
*/
স্থির পূর্ণাঙ্ক writecrashWrite(
  sqlite3_file *pFile, 
  ধ্রুব শূন্য *zBuf, 
  পূর্ণাঙ্ক iAmt, 
  sqlite_int64 iOfst
){
  devsym_file *p = (devsym_file *)pFile;
  যদি( g.nWriteCrash>0 ){
    g.nWriteCrash--;
    যদি( g.nWriteCrash==0 ) abort();
  }
  প্রত্যাগম sqlite3OsWrite(p->pReal, zBuf, iAmt, iOfst);
}

/*
** Open an writecrash file handle.
*/
স্থির পূর্ণাঙ্ক writecrashOpen(
  sqlite3_vfs *pVfs,
  ধ্রুব বর্ণ *zName,
  sqlite3_file *pFile,
  পূর্ণাঙ্ক flags,
  পূর্ণাঙ্ক *pOutFlags
){
স্থির sqlite3_io_methods writecrash_io_methods = {
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

  পূর্ণাঙ্ক rc;
  devsym_file *p = (devsym_file *)pFile;
  p->pReal = (sqlite3_file *)&p[1];
  rc = sqlite3OsOpen(g.pVfs, zName, p->pReal, flags, pOutFlags);
  যদি( p->pReal->pMethods ){
    pFile->pMethods = &writecrash_io_methods;
  }
  প্রত্যাগম rc;
}

স্থির sqlite3_vfs devsym_vfs = {
  2,                     /* iVersion */
  পরিমাণ(devsym_file),      /* szOsFile */
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
#অন্যথা
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

স্থির sqlite3_vfs writecrash_vfs = {
  2,                     /* iVersion */
  পরিমাণ(devsym_file),      /* szOsFile */
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
#অন্যথা
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
শূন্য devsym_register(পূর্ণাঙ্ক iDeviceChar, পূর্ণাঙ্ক iSectorSize){

  যদি( g.pVfs==0 ){
    g.pVfs = sqlite3_vfs_find(0);
    devsym_vfs.szOsFile += g.pVfs->szOsFile;
    writecrash_vfs.szOsFile += g.pVfs->szOsFile;
    sqlite3_vfs_register(&devsym_vfs, 0);
    sqlite3_vfs_register(&writecrash_vfs, 0);
  }
  যদি( iDeviceChar>=0 ){
    g.iDeviceChar = iDeviceChar;
  }অন্যথা{
    g.iDeviceChar = 0;
  }
  যদি( iSectorSize>=0 ){
    g.iSectorSize = iSectorSize;
  }অন্যথা{
    g.iSectorSize = 512;
  }
}

শূন্য devsym_unregister(){
  sqlite3_vfs_unregister(&devsym_vfs);
  sqlite3_vfs_unregister(&writecrash_vfs);
  g.pVfs = 0;
  g.iDeviceChar = 0;
  g.iSectorSize = 0;
}

শূন্য devsym_crash_on_write(পূর্ণাঙ্ক nWrite){
  যদি( g.pVfs==0 ){
    g.pVfs = sqlite3_vfs_find(0);
    devsym_vfs.szOsFile += g.pVfs->szOsFile;
    writecrash_vfs.szOsFile += g.pVfs->szOsFile;
    sqlite3_vfs_register(&devsym_vfs, 0);
    sqlite3_vfs_register(&writecrash_vfs, 0);
  }
  g.nWriteCrash = nWrite;
}

#endif
