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

প্রকারলেখ কাঠামো devsym_file devsym_file;
কাঠামো devsym_file {
  sqlite3_file base;
  sqlite3_file *pReal;
};

/*
** Method declarations for devsym_file.
*/
স্থির পূর্ণসংখ্যা devsymClose(sqlite3_file*);
স্থির পূর্ণসংখ্যা devsymRead(sqlite3_file*, শূন্য*, পূর্ণসংখ্যা iAmt, sqlite3_int64 iOfst);
স্থির পূর্ণসংখ্যা devsymWrite(sqlite3_file*,ধ্রুবক শূন্য*,পূর্ণসংখ্যা iAmt, sqlite3_int64 iOfst);
স্থির পূর্ণসংখ্যা devsymTruncate(sqlite3_file*, sqlite3_int64 আকার_2);
স্থির পূর্ণসংখ্যা devsymSync(sqlite3_file*, পূর্ণসংখ্যা flags);
স্থির পূর্ণসংখ্যা devsym_File_আকার(sqlite3_file*, sqlite3_int64 *p_আকার);
স্থির পূর্ণসংখ্যা devsymLock(sqlite3_file*, পূর্ণসংখ্যা);
স্থির পূর্ণসংখ্যা devsymUnlock(sqlite3_file*, পূর্ণসংখ্যা);
স্থির পূর্ণসংখ্যা devsymCheckReservedLock(sqlite3_file*, পূর্ণসংখ্যা *);
স্থির পূর্ণসংখ্যা devsymFileControl(sqlite3_file*, পূর্ণসংখ্যা op, শূন্য *pArg);
স্থির পূর্ণসংখ্যা devsym_Sector_আকার(sqlite3_file*);
স্থির পূর্ণসংখ্যা devsymDeviceCharacteristics(sqlite3_file*);
স্থির পূর্ণসংখ্যা devsymShmLock(sqlite3_file*,পূর্ণসংখ্যা,পূর্ণসংখ্যা,পূর্ণসংখ্যা);
স্থির পূর্ণসংখ্যা devsymShmMap(sqlite3_file*,পূর্ণসংখ্যা,পূর্ণসংখ্যা,পূর্ণসংখ্যা, শূন্য অস্থির **);
স্থির শূন্য devsymShmBarrier(sqlite3_file*);
স্থির পূর্ণসংখ্যা devsymShmUnmap(sqlite3_file*,পূর্ণসংখ্যা);

/*
** Method declarations for devsym_vfs.
*/
স্থির পূর্ণসংখ্যা devsymOpen(sqlite3_vfs*, ধ্রুবক অক্ষর *, sqlite3_file*, পূর্ণসংখ্যা , পূর্ণসংখ্যা *);
স্থির পূর্ণসংখ্যা devsymDelete(sqlite3_vfs*, ধ্রুবক অক্ষর *zName, পূর্ণসংখ্যা syncDir);
স্থির পূর্ণসংখ্যা devsymAccess(sqlite3_vfs*, ধ্রুবক অক্ষর *zName, পূর্ণসংখ্যা flags, পূর্ণসংখ্যা *);
স্থির পূর্ণসংখ্যা devsymFullPathname(sqlite3_vfs*, ধ্রুবক অক্ষর *zName, পূর্ণসংখ্যা, অক্ষর *zOut);
#ifndef SQLITE_OMIT_LOAD_EXTENSION
স্থির শূন্য *devsymDlOpen(sqlite3_vfs*, ধ্রুবক অক্ষর *zFilename);
স্থির শূন্য devsymDlError(sqlite3_vfs*, পূর্ণসংখ্যা nByte, অক্ষর *z_Err_বার্তা);
স্থির শূন্য (*devsymDlSym(sqlite3_vfs*,শূন্য*, ধ্রুবক অক্ষর *z_প্রতীক))(শূন্য);
স্থির শূন্য devsymDlClose(sqlite3_vfs*, শূন্য*);
#endif /* SQLITE_OMIT_LOAD_EXTENSION */
স্থির পূর্ণসংখ্যা devsymRandomness(sqlite3_vfs*, পূর্ণসংখ্যা nByte, অক্ষর *zOut);
স্থির পূর্ণসংখ্যা devsymSleep(sqlite3_vfs*, পূর্ণসংখ্যা microseconds);
স্থির পূর্ণসংখ্যা devsymCurrentTime(sqlite3_vfs*, দ্বিগুণ*);

কাঠামো DevsymGlobal {
  sqlite3_vfs *pVfs;
  পূর্ণসংখ্যা iDeviceChar;
  পূর্ণসংখ্যা i_Sector_আকার;
  পূর্ণসংখ্যা nWriteCrash;
};
কাঠামো DevsymGlobal g = {0, 0, 512, 0};

/*
** Close an devsym-file.
*/
স্থির পূর্ণসংখ্যা devsymClose(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  sqlite3OsClose(p->pReal);
  ফেরত SQLITE_OK;
}

/*
** Read data from an devsym-file.
*/
স্থির পূর্ণসংখ্যা devsymRead(
  sqlite3_file *pFile, 
  শূন্য *zBuf, 
  পূর্ণসংখ্যা iAmt, 
  sqlite_int64 iOfst
){
  devsym_file *p = (devsym_file *)pFile;
  ফেরত sqlite3OsRead(p->pReal, zBuf, iAmt, iOfst);
}

/*
** Write data to an devsym-file.
*/
স্থির পূর্ণসংখ্যা devsymWrite(
  sqlite3_file *pFile, 
  ধ্রুবক শূন্য *zBuf, 
  পূর্ণসংখ্যা iAmt, 
  sqlite_int64 iOfst
){
  devsym_file *p = (devsym_file *)pFile;
  ফেরত sqlite3OsWrite(p->pReal, zBuf, iAmt, iOfst);
}

/*
** Truncate an devsym-file.
*/
স্থির পূর্ণসংখ্যা devsymTruncate(sqlite3_file *pFile, sqlite_int64 আকার_2){
  devsym_file *p = (devsym_file *)pFile;
  ফেরত sqlite3OsTruncate(p->pReal, আকার_2);
}

/*
** Sync an devsym-file.
*/
স্থির পূর্ণসংখ্যা devsymSync(sqlite3_file *pFile, পূর্ণসংখ্যা flags){
  devsym_file *p = (devsym_file *)pFile;
  ফেরত sqlite3OsSync(p->pReal, flags);
}

/*
** Return the current file-size of an devsym-file.
*/
স্থির পূর্ণসংখ্যা devsym_File_আকার(sqlite3_file *pFile, sqlite_int64 *p_আকার){
  devsym_file *p = (devsym_file *)pFile;
  ফেরত sqlite3_Os_File_আকার(p->pReal, p_আকার);
}

/*
** Lock an devsym-file.
*/
স্থির পূর্ণসংখ্যা devsymLock(sqlite3_file *pFile, পূর্ণসংখ্যা eLock){
  devsym_file *p = (devsym_file *)pFile;
  ফেরত sqlite3OsLock(p->pReal, eLock);
}

/*
** Unlock an devsym-file.
*/
স্থির পূর্ণসংখ্যা devsymUnlock(sqlite3_file *pFile, পূর্ণসংখ্যা eLock){
  devsym_file *p = (devsym_file *)pFile;
  ফেরত sqlite3OsUnlock(p->pReal, eLock);
}

/*
** Check if another file-handle holds a RESERVED lock on an devsym-file.
*/
স্থির পূর্ণসংখ্যা devsymCheckReservedLock(sqlite3_file *pFile, পূর্ণসংখ্যা *pResOut){
  devsym_file *p = (devsym_file *)pFile;
  ফেরত sqlite3OsCheckReservedLock(p->pReal, pResOut);
}

/*
** File control method. For custom operations on an devsym-file.
*/
স্থির পূর্ণসংখ্যা devsymFileControl(sqlite3_file *pFile, পূর্ণসংখ্যা op, শূন্য *pArg){
  devsym_file *p = (devsym_file *)pFile;
  ফেরত sqlite3OsFileControl(p->pReal, op, pArg);
}

/*
** Return the sector-size in bytes for an devsym-file.
*/
স্থির পূর্ণসংখ্যা devsym_Sector_আকার(sqlite3_file *pFile){
  ফেরত g.i_Sector_আকার;
}

/*
** Return the device characteristic flags supported by an devsym-file.
*/
স্থির পূর্ণসংখ্যা devsymDeviceCharacteristics(sqlite3_file *pFile){
  ফেরত g.iDeviceChar;
}

/*
** Shared-memory methods are all pass-throughs.
*/
স্থির পূর্ণসংখ্যা devsymShmLock(sqlite3_file *pFile, পূর্ণসংখ্যা ofst, পূর্ণসংখ্যা n, পূর্ণসংখ্যা flags){
  devsym_file *p = (devsym_file *)pFile;
  ফেরত p->pReal->pMethods->xShmLock(p->pReal, ofst, n, flags);
}
স্থির পূর্ণসংখ্যা devsymShmMap(
  sqlite3_file *pFile, 
  পূর্ণসংখ্যা iRegion, 
  পূর্ণসংখ্যা szRegion, 
  পূর্ণসংখ্যা isWrite, 
  শূন্য অস্থির **pp
){
  devsym_file *p = (devsym_file *)pFile;
  ফেরত p->pReal->pMethods->xShmMap(p->pReal, iRegion, szRegion, isWrite, pp);
}
স্থির শূন্য devsymShmBarrier(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  p->pReal->pMethods->xShmBarrier(p->pReal);
}
স্থির পূর্ণসংখ্যা devsymShmUnmap(sqlite3_file *pFile, পূর্ণসংখ্যা delFlag){
  devsym_file *p = (devsym_file *)pFile;
  ফেরত p->pReal->pMethods->xShmUnmap(p->pReal, delFlag);
}



/*
** Open an devsym file handle.
*/
স্থির পূর্ণসংখ্যা devsymOpen(
  sqlite3_vfs *pVfs,
  ধ্রুবক অক্ষর *zName,
  sqlite3_file *pFile,
  পূর্ণসংখ্যা flags,
  পূর্ণসংখ্যা *pOutFlags
){
স্থির sqlite3_io_methods devsym_io_methods = {
  2,                                /* iVersion */
  devsymClose,                      /* xClose */
  devsymRead,                       /* xRead */
  devsymWrite,                      /* xWrite */
  devsymTruncate,                   /* xTruncate */
  devsymSync,                       /* xSync */
  devsym_File_আকার,                   /* xFileSize */
  devsymLock,                       /* xLock */
  devsymUnlock,                     /* xUnlock */
  devsymCheckReservedLock,          /* xCheckReservedLock */
  devsymFileControl,                /* xFileControl */
  devsym_Sector_আকার,                 /* xSectorSize */
  devsymDeviceCharacteristics,      /* xDeviceCharacteristics */
  devsymShmMap,                     /* xShmMap */
  devsymShmLock,                    /* xShmLock */
  devsymShmBarrier,                 /* xShmBarrier */
  devsymShmUnmap                    /* xShmUnmap */
};

  পূর্ণসংখ্যা rc;
  devsym_file *p = (devsym_file *)pFile;
  p->pReal = (sqlite3_file *)&p[1];
  rc = sqlite3OsOpen(g.pVfs, zName, p->pReal, flags, pOutFlags);
  যদি( p->pReal->pMethods ){
    pFile->pMethods = &devsym_io_methods;
  }
  ফেরত rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
স্থির পূর্ণসংখ্যা devsymDelete(sqlite3_vfs *pVfs, ধ্রুবক অক্ষর *zPath, পূর্ণসংখ্যা dirSync){
  ফেরত sqlite3OsDelete(g.pVfs, zPath, dirSync);
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
স্থির পূর্ণসংখ্যা devsymAccess(
  sqlite3_vfs *pVfs, 
  ধ্রুবক অক্ষর *zPath, 
  পূর্ণসংখ্যা flags, 
  পূর্ণসংখ্যা *pResOut
){
  ফেরত sqlite3OsAccess(g.pVfs, zPath, flags, pResOut);
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (DEVSYM_MAX_PATHNAME+1) bytes.
*/
স্থির পূর্ণসংখ্যা devsymFullPathname(
  sqlite3_vfs *pVfs, 
  ধ্রুবক অক্ষর *zPath, 
  পূর্ণসংখ্যা nOut, 
  অক্ষর *zOut
){
  ফেরত sqlite3OsFullPathname(g.pVfs, zPath, nOut, zOut);
}

#ifndef SQLITE_OMIT_LOAD_EXTENSION
/*
** Open the dynamic library located at zPath and return a handle.
*/
স্থির শূন্য *devsymDlOpen(sqlite3_vfs *pVfs, ধ্রুবক অক্ষর *zPath){
  ফেরত sqlite3OsDlOpen(g.pVfs, zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
স্থির শূন্য devsymDlError(sqlite3_vfs *pVfs, পূর্ণসংখ্যা nByte, অক্ষর *z_Err_বার্তা){
  sqlite3OsDlError(g.pVfs, nByte, z_Err_বার্তা);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
স্থির শূন্য (*devsymDlSym(sqlite3_vfs *pVfs, শূন্য *p, ধ্রুবক অক্ষর *zSym))(শূন্য){
  ফেরত sqlite3OsDlSym(g.pVfs, p, zSym);
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
স্থির পূর্ণসংখ্যা devsymRandomness(sqlite3_vfs *pVfs, পূর্ণসংখ্যা nByte, অক্ষর *zBufOut){
  ফেরত sqlite3OsRandomness(g.pVfs, nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
স্থির পূর্ণসংখ্যা devsymSleep(sqlite3_vfs *pVfs, পূর্ণসংখ্যা nMicro){
  ফেরত sqlite3OsSleep(g.pVfs, nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
স্থির পূর্ণসংখ্যা devsymCurrentTime(sqlite3_vfs *pVfs, দ্বিগুণ *pTimeOut){
  ফেরত g.pVfs->xCurrentTime(g.pVfs, pTimeOut);
}

/*
** Return the sector-size in bytes for an writecrash-file.
*/
স্থির পূর্ণসংখ্যা writecrash_Sector_আকার(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  ফেরত sqlite3_Os_Sector_আকার(p->pReal);
}

/*
** Return the device characteristic flags supported by an writecrash-file.
*/
স্থির পূর্ণসংখ্যা writecrashDeviceCharacteristics(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  ফেরত sqlite3OsDeviceCharacteristics(p->pReal);
}

/*
** Write data to an writecrash-file.
*/
স্থির পূর্ণসংখ্যা writecrashWrite(
  sqlite3_file *pFile, 
  ধ্রুবক শূন্য *zBuf, 
  পূর্ণসংখ্যা iAmt, 
  sqlite_int64 iOfst
){
  devsym_file *p = (devsym_file *)pFile;
  যদি( g.nWriteCrash>0 ){
    g.nWriteCrash--;
    যদি( g.nWriteCrash==0 ) abort();
  }
  ফেরত sqlite3OsWrite(p->pReal, zBuf, iAmt, iOfst);
}

/*
** Open an writecrash file handle.
*/
স্থির পূর্ণসংখ্যা writecrashOpen(
  sqlite3_vfs *pVfs,
  ধ্রুবক অক্ষর *zName,
  sqlite3_file *pFile,
  পূর্ণসংখ্যা flags,
  পূর্ণসংখ্যা *pOutFlags
){
স্থির sqlite3_io_methods writecrash_io_methods = {
  2,                                /* iVersion */
  devsymClose,                      /* xClose */
  devsymRead,                       /* xRead */
  writecrashWrite,                  /* xWrite */
  devsymTruncate,                   /* xTruncate */
  devsymSync,                       /* xSync */
  devsym_File_আকার,                   /* xFileSize */
  devsymLock,                       /* xLock */
  devsymUnlock,                     /* xUnlock */
  devsymCheckReservedLock,          /* xCheckReservedLock */
  devsymFileControl,                /* xFileControl */
  writecrash_Sector_আকার,             /* xSectorSize */
  writecrashDeviceCharacteristics,  /* xDeviceCharacteristics */
  devsymShmMap,                     /* xShmMap */
  devsymShmLock,                    /* xShmLock */
  devsymShmBarrier,                 /* xShmBarrier */
  devsymShmUnmap                    /* xShmUnmap */
};

  পূর্ণসংখ্যা rc;
  devsym_file *p = (devsym_file *)pFile;
  p->pReal = (sqlite3_file *)&p[1];
  rc = sqlite3OsOpen(g.pVfs, zName, p->pReal, flags, pOutFlags);
  যদি( p->pReal->pMethods ){
    pFile->pMethods = &writecrash_io_methods;
  }
  ফেরত rc;
}

স্থির sqlite3_vfs devsym_vfs = {
  2,                     /* iVersion */
  আকার(devsym_file),      /* szOsFile */
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
  আকার(devsym_file),      /* szOsFile */
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
শূন্য devsym_register(পূর্ণসংখ্যা iDeviceChar, পূর্ণসংখ্যা i_Sector_আকার){

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
  যদি( i_Sector_আকার>=0 ){
    g.i_Sector_আকার = i_Sector_আকার;
  }অন্যথা{
    g.i_Sector_আকার = 512;
  }
}

শূন্য devsym_unregister(){
  sqlite3_vfs_unregister(&devsym_vfs);
  sqlite3_vfs_unregister(&writecrash_vfs);
  g.pVfs = 0;
  g.iDeviceChar = 0;
  g.i_Sector_আকার = 0;
}

শূন্য devsym_crash_on_write(পূর্ণসংখ্যা nWrite){
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
