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
#જો SQLITE_TEST          /* This file is used for testing only */

#અન્તર્ભાવ "sqlite3.h"
#અન્તર્ભાવ "sqliteInt.h"

/*
** Maximum pathname length supported by the devsym backend.
*/
#define DEVSYM_MAX_PATHNAME 512

/*
** Name used to identify this VFS.
*/
#define DEVSYM_VFS_NAME "devsym"
#define WRITECRASH_NAME "writecrash"

પ્રકારવ્યાખ્યા સંરચના devsym_file devsym_file;
સંરચના devsym_file {
  sqlite3_file base;
  sqlite3_file *pReal;
};

/*
** Method declarations for devsym_file.
*/
સ્થિર પૂર્ણાંક devsymClose(sqlite3_file*);
સ્થિર પૂર્ણાંક devsymRead(sqlite3_file*, શૂન્ય*, પૂર્ણાંક iAmt, sqlite3_int64 iOfst);
સ્થિર પૂર્ણાંક devsymWrite(sqlite3_file*,અચલ શૂન્ય*,પૂર્ણાંક iAmt, sqlite3_int64 iOfst);
સ્થિર પૂર્ણાંક devsymTruncate(sqlite3_file*, sqlite3_int64 size);
સ્થિર પૂર્ણાંક devsymSync(sqlite3_file*, પૂર્ણાંક flags);
સ્થિર પૂર્ણાંક devsymFileSize(sqlite3_file*, sqlite3_int64 *pSize);
સ્થિર પૂર્ણાંક devsymLock(sqlite3_file*, પૂર્ણાંક);
સ્થિર પૂર્ણાંક devsymUnlock(sqlite3_file*, પૂર્ણાંક);
સ્થિર પૂર્ણાંક devsymCheckReservedLock(sqlite3_file*, પૂર્ણાંક *);
સ્થિર પૂર્ણાંક devsymFileControl(sqlite3_file*, પૂર્ણાંક op, શૂન્ય *pArg);
સ્થિર પૂર્ણાંક devsymSectorSize(sqlite3_file*);
સ્થિર પૂર્ણાંક devsymDeviceCharacteristics(sqlite3_file*);
સ્થિર પૂર્ણાંક devsymShmLock(sqlite3_file*,પૂર્ણાંક,પૂર્ણાંક,પૂર્ણાંક);
સ્થિર પૂર્ણાંક devsymShmMap(sqlite3_file*,પૂર્ણાંક,પૂર્ણાંક,પૂર્ણાંક, શૂન્ય અસ્થિર **);
સ્થિર શૂન્ય devsymShmBarrier(sqlite3_file*);
સ્થિર પૂર્ણાંક devsymShmUnmap(sqlite3_file*,પૂર્ણાંક);

/*
** Method declarations for devsym_vfs.
*/
સ્થિર પૂર્ણાંક devsymOpen(sqlite3_vfs*, અચલ અક્ષર *, sqlite3_file*, પૂર્ણાંક , પૂર્ણાંક *);
સ્થિર પૂર્ણાંક devsymDelete(sqlite3_vfs*, અચલ અક્ષર *zName, પૂર્ણાંક syncDir);
સ્થિર પૂર્ણાંક devsymAccess(sqlite3_vfs*, અચલ અક્ષર *zName, પૂર્ણાંક flags, પૂર્ણાંક *);
સ્થિર પૂર્ણાંક devsymFullPathname(sqlite3_vfs*, અચલ અક્ષર *zName, પૂર્ણાંક, અક્ષર *zOut);
#ifndef SQLITE_OMIT_LOAD_EXTENSION
સ્થિર શૂન્ય *devsymDlOpen(sqlite3_vfs*, અચલ અક્ષર *zFilename);
સ્થિર શૂન્ય devsymDlError(sqlite3_vfs*, પૂર્ણાંક nByte, અક્ષર *z_Err_સંદેશ);
સ્થિર શૂન્ય (*devsymDlSym(sqlite3_vfs*,શૂન્ય*, અચલ અક્ષર *z_પ્રતીક))(શૂન્ય);
સ્થિર શૂન્ય devsymDlClose(sqlite3_vfs*, શૂન્ય*);
#endif /* SQLITE_OMIT_LOAD_EXTENSION */
સ્થિર પૂર્ણાંક devsymRandomness(sqlite3_vfs*, પૂર્ણાંક nByte, અક્ષર *zOut);
સ્થિર પૂર્ણાંક devsymSleep(sqlite3_vfs*, પૂર્ણાંક microseconds);
સ્થિર પૂર્ણાંક devsymCurrentTime(sqlite3_vfs*, દ્વિગુણ*);

સંરચના DevsymGlobal {
  sqlite3_vfs *pVfs;
  પૂર્ણાંક iDeviceChar;
  પૂર્ણાંક iSectorSize;
  પૂર્ણાંક nWriteCrash;
};
સંરચના DevsymGlobal g = {0, 0, 512, 0};

/*
** Close an devsym-file.
*/
સ્થિર પૂર્ણાંક devsymClose(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  sqlite3OsClose(p->pReal);
  પાછા SQLITE_OK;
}

/*
** Read data from an devsym-file.
*/
સ્થિર પૂર્ણાંક devsymRead(
  sqlite3_file *pFile, 
  શૂન્ય *zBuf, 
  પૂર્ણાંક iAmt, 
  sqlite_int64 iOfst
){
  devsym_file *p = (devsym_file *)pFile;
  પાછા sqlite3OsRead(p->pReal, zBuf, iAmt, iOfst);
}

/*
** Write data to an devsym-file.
*/
સ્થિર પૂર્ણાંક devsymWrite(
  sqlite3_file *pFile, 
  અચલ શૂન્ય *zBuf, 
  પૂર્ણાંક iAmt, 
  sqlite_int64 iOfst
){
  devsym_file *p = (devsym_file *)pFile;
  પાછા sqlite3OsWrite(p->pReal, zBuf, iAmt, iOfst);
}

/*
** Truncate an devsym-file.
*/
સ્થિર પૂર્ણાંક devsymTruncate(sqlite3_file *pFile, sqlite_int64 size){
  devsym_file *p = (devsym_file *)pFile;
  પાછા sqlite3OsTruncate(p->pReal, size);
}

/*
** Sync an devsym-file.
*/
સ્થિર પૂર્ણાંક devsymSync(sqlite3_file *pFile, પૂર્ણાંક flags){
  devsym_file *p = (devsym_file *)pFile;
  પાછા sqlite3OsSync(p->pReal, flags);
}

/*
** Return the current file-size of an devsym-file.
*/
સ્થિર પૂર્ણાંક devsymFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  devsym_file *p = (devsym_file *)pFile;
  પાછા sqlite3OsFileSize(p->pReal, pSize);
}

/*
** Lock an devsym-file.
*/
સ્થિર પૂર્ણાંક devsymLock(sqlite3_file *pFile, પૂર્ણાંક eLock){
  devsym_file *p = (devsym_file *)pFile;
  પાછા sqlite3OsLock(p->pReal, eLock);
}

/*
** Unlock an devsym-file.
*/
સ્થિર પૂર્ણાંક devsymUnlock(sqlite3_file *pFile, પૂર્ણાંક eLock){
  devsym_file *p = (devsym_file *)pFile;
  પાછા sqlite3OsUnlock(p->pReal, eLock);
}

/*
** Check if another file-handle holds a RESERVED lock on an devsym-file.
*/
સ્થિર પૂર્ણાંક devsymCheckReservedLock(sqlite3_file *pFile, પૂર્ણાંક *pResOut){
  devsym_file *p = (devsym_file *)pFile;
  પાછા sqlite3OsCheckReservedLock(p->pReal, pResOut);
}

/*
** File control method. For custom operations on an devsym-file.
*/
સ્થિર પૂર્ણાંક devsymFileControl(sqlite3_file *pFile, પૂર્ણાંક op, શૂન્ય *pArg){
  devsym_file *p = (devsym_file *)pFile;
  પાછા sqlite3OsFileControl(p->pReal, op, pArg);
}

/*
** Return the sector-size in bytes for an devsym-file.
*/
સ્થિર પૂર્ણાંક devsymSectorSize(sqlite3_file *pFile){
  પાછા g.iSectorSize;
}

/*
** Return the device characteristic flags supported by an devsym-file.
*/
સ્થિર પૂર્ણાંક devsymDeviceCharacteristics(sqlite3_file *pFile){
  પાછા g.iDeviceChar;
}

/*
** Shared-memory methods are all pass-throughs.
*/
સ્થિર પૂર્ણાંક devsymShmLock(sqlite3_file *pFile, પૂર્ણાંક ofst, પૂર્ણાંક n, પૂર્ણાંક flags){
  devsym_file *p = (devsym_file *)pFile;
  પાછા p->pReal->pMethods->xShmLock(p->pReal, ofst, n, flags);
}
સ્થિર પૂર્ણાંક devsymShmMap(
  sqlite3_file *pFile, 
  પૂર્ણાંક iRegion, 
  પૂર્ણાંક szRegion, 
  પૂર્ણાંક isWrite, 
  શૂન્ય અસ્થિર **pp
){
  devsym_file *p = (devsym_file *)pFile;
  પાછા p->pReal->pMethods->xShmMap(p->pReal, iRegion, szRegion, isWrite, pp);
}
સ્થિર શૂન્ય devsymShmBarrier(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  p->pReal->pMethods->xShmBarrier(p->pReal);
}
સ્થિર પૂર્ણાંક devsymShmUnmap(sqlite3_file *pFile, પૂર્ણાંક delFlag){
  devsym_file *p = (devsym_file *)pFile;
  પાછા p->pReal->pMethods->xShmUnmap(p->pReal, delFlag);
}



/*
** Open an devsym file handle.
*/
સ્થિર પૂર્ણાંક devsymOpen(
  sqlite3_vfs *pVfs,
  અચલ અક્ષર *zName,
  sqlite3_file *pFile,
  પૂર્ણાંક flags,
  પૂર્ણાંક *pOutFlags
){
સ્થિર sqlite3_io_methods devsym_io_methods = {
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

  પૂર્ણાંક rc;
  devsym_file *p = (devsym_file *)pFile;
  p->pReal = (sqlite3_file *)&p[1];
  rc = sqlite3OsOpen(g.pVfs, zName, p->pReal, flags, pOutFlags);
  જો( p->pReal->pMethods ){
    pFile->pMethods = &devsym_io_methods;
  }
  પાછા rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
સ્થિર પૂર્ણાંક devsymDelete(sqlite3_vfs *pVfs, અચલ અક્ષર *zPath, પૂર્ણાંક dirSync){
  પાછા sqlite3OsDelete(g.pVfs, zPath, dirSync);
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
સ્થિર પૂર્ણાંક devsymAccess(
  sqlite3_vfs *pVfs, 
  અચલ અક્ષર *zPath, 
  પૂર્ણાંક flags, 
  પૂર્ણાંક *pResOut
){
  પાછા sqlite3OsAccess(g.pVfs, zPath, flags, pResOut);
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (DEVSYM_MAX_PATHNAME+1) bytes.
*/
સ્થિર પૂર્ણાંક devsymFullPathname(
  sqlite3_vfs *pVfs, 
  અચલ અક્ષર *zPath, 
  પૂર્ણાંક nOut, 
  અક્ષર *zOut
){
  પાછા sqlite3OsFullPathname(g.pVfs, zPath, nOut, zOut);
}

#ifndef SQLITE_OMIT_LOAD_EXTENSION
/*
** Open the dynamic library located at zPath and return a handle.
*/
સ્થિર શૂન્ય *devsymDlOpen(sqlite3_vfs *pVfs, અચલ અક્ષર *zPath){
  પાછા sqlite3OsDlOpen(g.pVfs, zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
સ્થિર શૂન્ય devsymDlError(sqlite3_vfs *pVfs, પૂર્ણાંક nByte, અક્ષર *z_Err_સંદેશ){
  sqlite3OsDlError(g.pVfs, nByte, z_Err_સંદેશ);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
સ્થિર શૂન્ય (*devsymDlSym(sqlite3_vfs *pVfs, શૂન્ય *p, અચલ અક્ષર *zSym))(શૂન્ય){
  પાછા sqlite3OsDlSym(g.pVfs, p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
સ્થિર શૂન્ય devsymDlClose(sqlite3_vfs *pVfs, શૂન્ય *pHandle){
  sqlite3OsDlClose(g.pVfs, pHandle);
}
#endif /* SQLITE_OMIT_LOAD_EXTENSION */

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
સ્થિર પૂર્ણાંક devsymRandomness(sqlite3_vfs *pVfs, પૂર્ણાંક nByte, અક્ષર *zBufOut){
  પાછા sqlite3OsRandomness(g.pVfs, nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
સ્થિર પૂર્ણાંક devsymSleep(sqlite3_vfs *pVfs, પૂર્ણાંક nMicro){
  પાછા sqlite3OsSleep(g.pVfs, nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
સ્થિર પૂર્ણાંક devsymCurrentTime(sqlite3_vfs *pVfs, દ્વિગુણ *pTimeOut){
  પાછા g.pVfs->xCurrentTime(g.pVfs, pTimeOut);
}

/*
** Return the sector-size in bytes for an writecrash-file.
*/
સ્થિર પૂર્ણાંક writecrashSectorSize(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  પાછા sqlite3OsSectorSize(p->pReal);
}

/*
** Return the device characteristic flags supported by an writecrash-file.
*/
સ્થિર પૂર્ણાંક writecrashDeviceCharacteristics(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  પાછા sqlite3OsDeviceCharacteristics(p->pReal);
}

/*
** Write data to an writecrash-file.
*/
સ્થિર પૂર્ણાંક writecrashWrite(
  sqlite3_file *pFile, 
  અચલ શૂન્ય *zBuf, 
  પૂર્ણાંક iAmt, 
  sqlite_int64 iOfst
){
  devsym_file *p = (devsym_file *)pFile;
  જો( g.nWriteCrash>0 ){
    g.nWriteCrash--;
    જો( g.nWriteCrash==0 ) abort();
  }
  પાછા sqlite3OsWrite(p->pReal, zBuf, iAmt, iOfst);
}

/*
** Open an writecrash file handle.
*/
સ્થિર પૂર્ણાંક writecrashOpen(
  sqlite3_vfs *pVfs,
  અચલ અક્ષર *zName,
  sqlite3_file *pFile,
  પૂર્ણાંક flags,
  પૂર્ણાંક *pOutFlags
){
સ્થિર sqlite3_io_methods writecrash_io_methods = {
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

  પૂર્ણાંક rc;
  devsym_file *p = (devsym_file *)pFile;
  p->pReal = (sqlite3_file *)&p[1];
  rc = sqlite3OsOpen(g.pVfs, zName, p->pReal, flags, pOutFlags);
  જો( p->pReal->pMethods ){
    pFile->pMethods = &writecrash_io_methods;
  }
  પાછા rc;
}

સ્થિર sqlite3_vfs devsym_vfs = {
  2,                     /* iVersion */
  કદ(devsym_file),      /* szOsFile */
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
#નહીંતર
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

સ્થિર sqlite3_vfs writecrash_vfs = {
  2,                     /* iVersion */
  કદ(devsym_file),      /* szOsFile */
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
#નહીંતર
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
શૂન્ય devsym_register(પૂર્ણાંક iDeviceChar, પૂર્ણાંક iSectorSize){

  જો( g.pVfs==0 ){
    g.pVfs = sqlite3_vfs_find(0);
    devsym_vfs.szOsFile += g.pVfs->szOsFile;
    writecrash_vfs.szOsFile += g.pVfs->szOsFile;
    sqlite3_vfs_register(&devsym_vfs, 0);
    sqlite3_vfs_register(&writecrash_vfs, 0);
  }
  જો( iDeviceChar>=0 ){
    g.iDeviceChar = iDeviceChar;
  }નહીંતર{
    g.iDeviceChar = 0;
  }
  જો( iSectorSize>=0 ){
    g.iSectorSize = iSectorSize;
  }નહીંતર{
    g.iSectorSize = 512;
  }
}

શૂન્ય devsym_unregister(){
  sqlite3_vfs_unregister(&devsym_vfs);
  sqlite3_vfs_unregister(&writecrash_vfs);
  g.pVfs = 0;
  g.iDeviceChar = 0;
  g.iSectorSize = 0;
}

શૂન્ય devsym_crash_on_write(પૂર્ણાંક nWrite){
  જો( g.pVfs==0 ){
    g.pVfs = sqlite3_vfs_find(0);
    devsym_vfs.szOsFile += g.pVfs->szOsFile;
    writecrash_vfs.szOsFile += g.pVfs->szOsFile;
    sqlite3_vfs_register(&devsym_vfs, 0);
    sqlite3_vfs_register(&writecrash_vfs, 0);
  }
  g.nWriteCrash = nWrite;
}

#endif
