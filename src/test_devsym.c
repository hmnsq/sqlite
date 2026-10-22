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
#जर SQLITE_TEST          /* This file is used for testing only */

#अन्तर्भाव "sqlite3.h"
#अन्तर्भाव "sqliteInt.h"

/*
** Maximum pathname length supported by the devsym backend.
*/
#define DEVSYM_MAX_PATHNAME 512

/*
** Name used to identify this VFS.
*/
#define DEVSYM_VFS_NAME "devsym"
#define WRITECRASH_NAME "writecrash"

प्रकारव्याख्या संरचना devsym_file devsym_file;
संरचना devsym_file {
  sqlite3_file base;
  sqlite3_file *pReal;
};

/*
** Method declarations for devsym_file.
*/
स्थिर पूर्णांक devsymClose(sqlite3_file*);
स्थिर पूर्णांक devsymRead(sqlite3_file*, शून्य*, पूर्णांक iAmt, sqlite3_int64 iOfst);
स्थिर पूर्णांक devsymWrite(sqlite3_file*,स्थिरांक शून्य*,पूर्णांक iAmt, sqlite3_int64 iOfst);
स्थिर पूर्णांक devsymTruncate(sqlite3_file*, sqlite3_int64 size);
स्थिर पूर्णांक devsymSync(sqlite3_file*, पूर्णांक flags);
स्थिर पूर्णांक devsymFileSize(sqlite3_file*, sqlite3_int64 *pSize);
स्थिर पूर्णांक devsymLock(sqlite3_file*, पूर्णांक);
स्थिर पूर्णांक devsymUnlock(sqlite3_file*, पूर्णांक);
स्थिर पूर्णांक devsymCheckReservedLock(sqlite3_file*, पूर्णांक *);
स्थिर पूर्णांक devsymFileControl(sqlite3_file*, पूर्णांक op, शून्य *pArg);
स्थिर पूर्णांक devsymSectorSize(sqlite3_file*);
स्थिर पूर्णांक devsymDeviceCharacteristics(sqlite3_file*);
स्थिर पूर्णांक devsymShmLock(sqlite3_file*,पूर्णांक,पूर्णांक,पूर्णांक);
स्थिर पूर्णांक devsymShmMap(sqlite3_file*,पूर्णांक,पूर्णांक,पूर्णांक, शून्य अस्थिर **);
स्थिर शून्य devsymShmBarrier(sqlite3_file*);
स्थिर पूर्णांक devsymShmUnmap(sqlite3_file*,पूर्णांक);

/*
** Method declarations for devsym_vfs.
*/
स्थिर पूर्णांक devsymOpen(sqlite3_vfs*, स्थिरांक अक्षर *, sqlite3_file*, पूर्णांक , पूर्णांक *);
स्थिर पूर्णांक devsymDelete(sqlite3_vfs*, स्थिरांक अक्षर *zName, पूर्णांक syncDir);
स्थिर पूर्णांक devsymAccess(sqlite3_vfs*, स्थिरांक अक्षर *zName, पूर्णांक flags, पूर्णांक *);
स्थिर पूर्णांक devsymFullPathname(sqlite3_vfs*, स्थिरांक अक्षर *zName, पूर्णांक, अक्षर *zOut);
#ifndef SQLITE_OMIT_LOAD_EXTENSION
स्थिर शून्य *devsymDlOpen(sqlite3_vfs*, स्थिरांक अक्षर *zFilename);
स्थिर शून्य devsymDlError(sqlite3_vfs*, पूर्णांक nByte, अक्षर *z_Err_संदेश);
स्थिर शून्य (*devsymDlSym(sqlite3_vfs*,शून्य*, स्थिरांक अक्षर *z_चिन्ह))(शून्य);
स्थिर शून्य devsymDlClose(sqlite3_vfs*, शून्य*);
#endif /* SQLITE_OMIT_LOAD_EXTENSION */
स्थिर पूर्णांक devsymRandomness(sqlite3_vfs*, पूर्णांक nByte, अक्षर *zOut);
स्थिर पूर्णांक devsymSleep(sqlite3_vfs*, पूर्णांक microseconds);
स्थिर पूर्णांक devsymCurrentTime(sqlite3_vfs*, द्विगुण*);

संरचना DevsymGlobal {
  sqlite3_vfs *pVfs;
  पूर्णांक iDeviceChar;
  पूर्णांक iSectorSize;
  पूर्णांक nWriteCrash;
};
संरचना DevsymGlobal g = {0, 0, 512, 0};

/*
** Close an devsym-file.
*/
स्थिर पूर्णांक devsymClose(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  sqlite3OsClose(p->pReal);
  परत SQLITE_OK;
}

/*
** Read data from an devsym-file.
*/
स्थिर पूर्णांक devsymRead(
  sqlite3_file *pFile, 
  शून्य *zBuf, 
  पूर्णांक iAmt, 
  sqlite_int64 iOfst
){
  devsym_file *p = (devsym_file *)pFile;
  परत sqlite3OsRead(p->pReal, zBuf, iAmt, iOfst);
}

/*
** Write data to an devsym-file.
*/
स्थिर पूर्णांक devsymWrite(
  sqlite3_file *pFile, 
  स्थिरांक शून्य *zBuf, 
  पूर्णांक iAmt, 
  sqlite_int64 iOfst
){
  devsym_file *p = (devsym_file *)pFile;
  परत sqlite3OsWrite(p->pReal, zBuf, iAmt, iOfst);
}

/*
** Truncate an devsym-file.
*/
स्थिर पूर्णांक devsymTruncate(sqlite3_file *pFile, sqlite_int64 size){
  devsym_file *p = (devsym_file *)pFile;
  परत sqlite3OsTruncate(p->pReal, size);
}

/*
** Sync an devsym-file.
*/
स्थिर पूर्णांक devsymSync(sqlite3_file *pFile, पूर्णांक flags){
  devsym_file *p = (devsym_file *)pFile;
  परत sqlite3OsSync(p->pReal, flags);
}

/*
** Return the current file-size of an devsym-file.
*/
स्थिर पूर्णांक devsymFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  devsym_file *p = (devsym_file *)pFile;
  परत sqlite3OsFileSize(p->pReal, pSize);
}

/*
** Lock an devsym-file.
*/
स्थिर पूर्णांक devsymLock(sqlite3_file *pFile, पूर्णांक eLock){
  devsym_file *p = (devsym_file *)pFile;
  परत sqlite3OsLock(p->pReal, eLock);
}

/*
** Unlock an devsym-file.
*/
स्थिर पूर्णांक devsymUnlock(sqlite3_file *pFile, पूर्णांक eLock){
  devsym_file *p = (devsym_file *)pFile;
  परत sqlite3OsUnlock(p->pReal, eLock);
}

/*
** Check if another file-handle holds a RESERVED lock on an devsym-file.
*/
स्थिर पूर्णांक devsymCheckReservedLock(sqlite3_file *pFile, पूर्णांक *pResOut){
  devsym_file *p = (devsym_file *)pFile;
  परत sqlite3OsCheckReservedLock(p->pReal, pResOut);
}

/*
** File control method. For custom operations on an devsym-file.
*/
स्थिर पूर्णांक devsymFileControl(sqlite3_file *pFile, पूर्णांक op, शून्य *pArg){
  devsym_file *p = (devsym_file *)pFile;
  परत sqlite3OsFileControl(p->pReal, op, pArg);
}

/*
** Return the sector-size in bytes for an devsym-file.
*/
स्थिर पूर्णांक devsymSectorSize(sqlite3_file *pFile){
  परत g.iSectorSize;
}

/*
** Return the device characteristic flags supported by an devsym-file.
*/
स्थिर पूर्णांक devsymDeviceCharacteristics(sqlite3_file *pFile){
  परत g.iDeviceChar;
}

/*
** Shared-memory methods are all pass-throughs.
*/
स्थिर पूर्णांक devsymShmLock(sqlite3_file *pFile, पूर्णांक ofst, पूर्णांक n, पूर्णांक flags){
  devsym_file *p = (devsym_file *)pFile;
  परत p->pReal->pMethods->xShmLock(p->pReal, ofst, n, flags);
}
स्थिर पूर्णांक devsymShmMap(
  sqlite3_file *pFile, 
  पूर्णांक iRegion, 
  पूर्णांक szRegion, 
  पूर्णांक isWrite, 
  शून्य अस्थिर **pp
){
  devsym_file *p = (devsym_file *)pFile;
  परत p->pReal->pMethods->xShmMap(p->pReal, iRegion, szRegion, isWrite, pp);
}
स्थिर शून्य devsymShmBarrier(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  p->pReal->pMethods->xShmBarrier(p->pReal);
}
स्थिर पूर्णांक devsymShmUnmap(sqlite3_file *pFile, पूर्णांक delFlag){
  devsym_file *p = (devsym_file *)pFile;
  परत p->pReal->pMethods->xShmUnmap(p->pReal, delFlag);
}



/*
** Open an devsym file handle.
*/
स्थिर पूर्णांक devsymOpen(
  sqlite3_vfs *pVfs,
  स्थिरांक अक्षर *zName,
  sqlite3_file *pFile,
  पूर्णांक flags,
  पूर्णांक *pOutFlags
){
स्थिर sqlite3_io_methods devsym_io_methods = {
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

  पूर्णांक rc;
  devsym_file *p = (devsym_file *)pFile;
  p->pReal = (sqlite3_file *)&p[1];
  rc = sqlite3OsOpen(g.pVfs, zName, p->pReal, flags, pOutFlags);
  जर( p->pReal->pMethods ){
    pFile->pMethods = &devsym_io_methods;
  }
  परत rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
स्थिर पूर्णांक devsymDelete(sqlite3_vfs *pVfs, स्थिरांक अक्षर *zPath, पूर्णांक dirSync){
  परत sqlite3OsDelete(g.pVfs, zPath, dirSync);
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
स्थिर पूर्णांक devsymAccess(
  sqlite3_vfs *pVfs, 
  स्थिरांक अक्षर *zPath, 
  पूर्णांक flags, 
  पूर्णांक *pResOut
){
  परत sqlite3OsAccess(g.pVfs, zPath, flags, pResOut);
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (DEVSYM_MAX_PATHNAME+1) bytes.
*/
स्थिर पूर्णांक devsymFullPathname(
  sqlite3_vfs *pVfs, 
  स्थिरांक अक्षर *zPath, 
  पूर्णांक nOut, 
  अक्षर *zOut
){
  परत sqlite3OsFullPathname(g.pVfs, zPath, nOut, zOut);
}

#ifndef SQLITE_OMIT_LOAD_EXTENSION
/*
** Open the dynamic library located at zPath and return a handle.
*/
स्थिर शून्य *devsymDlOpen(sqlite3_vfs *pVfs, स्थिरांक अक्षर *zPath){
  परत sqlite3OsDlOpen(g.pVfs, zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
स्थिर शून्य devsymDlError(sqlite3_vfs *pVfs, पूर्णांक nByte, अक्षर *z_Err_संदेश){
  sqlite3OsDlError(g.pVfs, nByte, z_Err_संदेश);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
स्थिर शून्य (*devsymDlSym(sqlite3_vfs *pVfs, शून्य *p, स्थिरांक अक्षर *zSym))(शून्य){
  परत sqlite3OsDlSym(g.pVfs, p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
स्थिर शून्य devsymDlClose(sqlite3_vfs *pVfs, शून्य *pHandle){
  sqlite3OsDlClose(g.pVfs, pHandle);
}
#endif /* SQLITE_OMIT_LOAD_EXTENSION */

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
स्थिर पूर्णांक devsymRandomness(sqlite3_vfs *pVfs, पूर्णांक nByte, अक्षर *zBufOut){
  परत sqlite3OsRandomness(g.pVfs, nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
स्थिर पूर्णांक devsymSleep(sqlite3_vfs *pVfs, पूर्णांक nMicro){
  परत sqlite3OsSleep(g.pVfs, nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
स्थिर पूर्णांक devsymCurrentTime(sqlite3_vfs *pVfs, द्विगुण *pTimeOut){
  परत g.pVfs->xCurrentTime(g.pVfs, pTimeOut);
}

/*
** Return the sector-size in bytes for an writecrash-file.
*/
स्थिर पूर्णांक writecrashSectorSize(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  परत sqlite3OsSectorSize(p->pReal);
}

/*
** Return the device characteristic flags supported by an writecrash-file.
*/
स्थिर पूर्णांक writecrashDeviceCharacteristics(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  परत sqlite3OsDeviceCharacteristics(p->pReal);
}

/*
** Write data to an writecrash-file.
*/
स्थिर पूर्णांक writecrashWrite(
  sqlite3_file *pFile, 
  स्थिरांक शून्य *zBuf, 
  पूर्णांक iAmt, 
  sqlite_int64 iOfst
){
  devsym_file *p = (devsym_file *)pFile;
  जर( g.nWriteCrash>0 ){
    g.nWriteCrash--;
    जर( g.nWriteCrash==0 ) पात();
  }
  परत sqlite3OsWrite(p->pReal, zBuf, iAmt, iOfst);
}

/*
** Open an writecrash file handle.
*/
स्थिर पूर्णांक writecrashOpen(
  sqlite3_vfs *pVfs,
  स्थिरांक अक्षर *zName,
  sqlite3_file *pFile,
  पूर्णांक flags,
  पूर्णांक *pOutFlags
){
स्थिर sqlite3_io_methods writecrash_io_methods = {
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

  पूर्णांक rc;
  devsym_file *p = (devsym_file *)pFile;
  p->pReal = (sqlite3_file *)&p[1];
  rc = sqlite3OsOpen(g.pVfs, zName, p->pReal, flags, pOutFlags);
  जर( p->pReal->pMethods ){
    pFile->pMethods = &writecrash_io_methods;
  }
  परत rc;
}

स्थिर sqlite3_vfs devsym_vfs = {
  2,                     /* iVersion */
  आकार(devsym_file),      /* szOsFile */
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
#अन्यथा
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

स्थिर sqlite3_vfs writecrash_vfs = {
  2,                     /* iVersion */
  आकार(devsym_file),      /* szOsFile */
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
#अन्यथा
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
शून्य devsym_register(पूर्णांक iDeviceChar, पूर्णांक iSectorSize){

  जर( g.pVfs==0 ){
    g.pVfs = sqlite3_vfs_find(0);
    devsym_vfs.szOsFile += g.pVfs->szOsFile;
    writecrash_vfs.szOsFile += g.pVfs->szOsFile;
    sqlite3_vfs_register(&devsym_vfs, 0);
    sqlite3_vfs_register(&writecrash_vfs, 0);
  }
  जर( iDeviceChar>=0 ){
    g.iDeviceChar = iDeviceChar;
  }अन्यथा{
    g.iDeviceChar = 0;
  }
  जर( iSectorSize>=0 ){
    g.iSectorSize = iSectorSize;
  }अन्यथा{
    g.iSectorSize = 512;
  }
}

शून्य devsym_unregister(){
  sqlite3_vfs_unregister(&devsym_vfs);
  sqlite3_vfs_unregister(&writecrash_vfs);
  g.pVfs = 0;
  g.iDeviceChar = 0;
  g.iSectorSize = 0;
}

शून्य devsym_crash_on_write(पूर्णांक nWrite){
  जर( g.pVfs==0 ){
    g.pVfs = sqlite3_vfs_find(0);
    devsym_vfs.szOsFile += g.pVfs->szOsFile;
    writecrash_vfs.szOsFile += g.pVfs->szOsFile;
    sqlite3_vfs_register(&devsym_vfs, 0);
    sqlite3_vfs_register(&writecrash_vfs, 0);
  }
  g.nWriteCrash = nWrite;
}

#endif
