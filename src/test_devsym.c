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
#अगर SQLITE_TEST          /* This file is used for testing only */

#समावेश "sqlite3.h"
#समावेश "sqliteInt.h"

/*
** Maximum pathname length supported by the devsym backend.
*/
#घोषणा DEVSYM_MAX_PATHNAME 512

/*
** Name used to identify this VFS.
*/
#घोषणा DEVSYM_VFS_NAME "devsym"
#घोषणा WRITECRASH_NAME "writecrash"

प्रकार काष्ठा devsym_file devsym_file;
काष्ठा devsym_file {
  sqlite3_file base;
  sqlite3_file *pReal;
};

/*
** Method declarations for devsym_file.
*/
अटल पूर्णांक devsymClose(sqlite3_file*);
अटल पूर्णांक devsymRead(sqlite3_file*, व्योम*, पूर्णांक iAmt, sqlite3_int64 iOfst);
अटल पूर्णांक devsymWrite(sqlite3_file*,स्थिर व्योम*,पूर्णांक iAmt, sqlite3_int64 iOfst);
अटल पूर्णांक devsymTruncate(sqlite3_file*, sqlite3_int64 आकार);
अटल पूर्णांक devsymSync(sqlite3_file*, पूर्णांक flags);
अटल पूर्णांक devsym_File_आकार(sqlite3_file*, sqlite3_int64 *p_आकार);
अटल पूर्णांक devsymLock(sqlite3_file*, पूर्णांक);
अटल पूर्णांक devsymUnlock(sqlite3_file*, पूर्णांक);
अटल पूर्णांक devsymCheckReservedLock(sqlite3_file*, पूर्णांक *);
अटल पूर्णांक devsymFileControl(sqlite3_file*, पूर्णांक op, व्योम *pArg);
अटल पूर्णांक devsym_Sector_आकार(sqlite3_file*);
अटल पूर्णांक devsymDeviceCharacteristics(sqlite3_file*);
अटल पूर्णांक devsymShmLock(sqlite3_file*,पूर्णांक,पूर्णांक,पूर्णांक);
अटल पूर्णांक devsymShmMap(sqlite3_file*,पूर्णांक,पूर्णांक,पूर्णांक, व्योम अस्थिर **);
अटल व्योम devsymShmBarrier(sqlite3_file*);
अटल पूर्णांक devsymShmUnmap(sqlite3_file*,पूर्णांक);

/*
** Method declarations for devsym_vfs.
*/
अटल पूर्णांक devsymOpen(sqlite3_vfs*, स्थिर अक्षर *, sqlite3_file*, पूर्णांक , पूर्णांक *);
अटल पूर्णांक devsymDelete(sqlite3_vfs*, स्थिर अक्षर *zName, पूर्णांक syncDir);
अटल पूर्णांक devsymAccess(sqlite3_vfs*, स्थिर अक्षर *zName, पूर्णांक flags, पूर्णांक *);
अटल पूर्णांक devsymFullPathname(sqlite3_vfs*, स्थिर अक्षर *zName, पूर्णांक, अक्षर *zOut);
#अगर_अघोषित SQLITE_OMIT_LOAD_EXTENSION
अटल व्योम *devsymDlOpen(sqlite3_vfs*, स्थिर अक्षर *zFilename);
अटल व्योम devsymDlError(sqlite3_vfs*, पूर्णांक nByte, अक्षर *z_Err_संदेश);
अटल व्योम (*devsymDlSym(sqlite3_vfs*,व्योम*, स्थिर अक्षर *z_प्रतीक))(व्योम);
अटल व्योम devsymDlClose(sqlite3_vfs*, व्योम*);
#पूर्ण_अगर /* SQLITE_OMIT_LOAD_EXTENSION */
अटल पूर्णांक devsymRandomness(sqlite3_vfs*, पूर्णांक nByte, अक्षर *zOut);
अटल पूर्णांक devsymSleep(sqlite3_vfs*, पूर्णांक microseconds);
अटल पूर्णांक devsymCurrentTime(sqlite3_vfs*, द्विगुन*);

काष्ठा DevsymGlobal {
  sqlite3_vfs *pVfs;
  पूर्णांक iDeviceChar;
  पूर्णांक i_Sector_आकार;
  पूर्णांक nWriteCrash;
};
काष्ठा DevsymGlobal g = {0, 0, 512, 0};

/*
** Close an devsym-file.
*/
अटल पूर्णांक devsymClose(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  sqlite3OsClose(p->pReal);
  वापस SQLITE_OK;
}

/*
** Read data from an devsym-file.
*/
अटल पूर्णांक devsymRead(
  sqlite3_file *pFile, 
  व्योम *zBuf, 
  पूर्णांक iAmt, 
  sqlite_int64 iOfst
){
  devsym_file *p = (devsym_file *)pFile;
  वापस sqlite3OsRead(p->pReal, zBuf, iAmt, iOfst);
}

/*
** Write data to an devsym-file.
*/
अटल पूर्णांक devsymWrite(
  sqlite3_file *pFile, 
  स्थिर व्योम *zBuf, 
  पूर्णांक iAmt, 
  sqlite_int64 iOfst
){
  devsym_file *p = (devsym_file *)pFile;
  वापस sqlite3OsWrite(p->pReal, zBuf, iAmt, iOfst);
}

/*
** Truncate an devsym-file.
*/
अटल पूर्णांक devsymTruncate(sqlite3_file *pFile, sqlite_int64 आकार){
  devsym_file *p = (devsym_file *)pFile;
  वापस sqlite3OsTruncate(p->pReal, आकार);
}

/*
** Sync an devsym-file.
*/
अटल पूर्णांक devsymSync(sqlite3_file *pFile, पूर्णांक flags){
  devsym_file *p = (devsym_file *)pFile;
  वापस sqlite3OsSync(p->pReal, flags);
}

/*
** Return the current file-size of an devsym-file.
*/
अटल पूर्णांक devsym_File_आकार(sqlite3_file *pFile, sqlite_int64 *p_आकार){
  devsym_file *p = (devsym_file *)pFile;
  वापस sqlite3_Os_File_आकार(p->pReal, p_आकार);
}

/*
** Lock an devsym-file.
*/
अटल पूर्णांक devsymLock(sqlite3_file *pFile, पूर्णांक eLock){
  devsym_file *p = (devsym_file *)pFile;
  वापस sqlite3OsLock(p->pReal, eLock);
}

/*
** Unlock an devsym-file.
*/
अटल पूर्णांक devsymUnlock(sqlite3_file *pFile, पूर्णांक eLock){
  devsym_file *p = (devsym_file *)pFile;
  वापस sqlite3OsUnlock(p->pReal, eLock);
}

/*
** Check if another file-handle holds a RESERVED lock on an devsym-file.
*/
अटल पूर्णांक devsymCheckReservedLock(sqlite3_file *pFile, पूर्णांक *pResOut){
  devsym_file *p = (devsym_file *)pFile;
  वापस sqlite3OsCheckReservedLock(p->pReal, pResOut);
}

/*
** File control method. For custom operations on an devsym-file.
*/
अटल पूर्णांक devsymFileControl(sqlite3_file *pFile, पूर्णांक op, व्योम *pArg){
  devsym_file *p = (devsym_file *)pFile;
  वापस sqlite3OsFileControl(p->pReal, op, pArg);
}

/*
** Return the sector-size in bytes for an devsym-file.
*/
अटल पूर्णांक devsym_Sector_आकार(sqlite3_file *pFile){
  वापस g.i_Sector_आकार;
}

/*
** Return the device characteristic flags supported by an devsym-file.
*/
अटल पूर्णांक devsymDeviceCharacteristics(sqlite3_file *pFile){
  वापस g.iDeviceChar;
}

/*
** Shared-memory methods are all pass-throughs.
*/
अटल पूर्णांक devsymShmLock(sqlite3_file *pFile, पूर्णांक ofst, पूर्णांक n, पूर्णांक flags){
  devsym_file *p = (devsym_file *)pFile;
  वापस p->pReal->pMethods->xShmLock(p->pReal, ofst, n, flags);
}
अटल पूर्णांक devsymShmMap(
  sqlite3_file *pFile, 
  पूर्णांक iRegion, 
  पूर्णांक szRegion, 
  पूर्णांक isWrite, 
  व्योम अस्थिर **pp
){
  devsym_file *p = (devsym_file *)pFile;
  वापस p->pReal->pMethods->xShmMap(p->pReal, iRegion, szRegion, isWrite, pp);
}
अटल व्योम devsymShmBarrier(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  p->pReal->pMethods->xShmBarrier(p->pReal);
}
अटल पूर्णांक devsymShmUnmap(sqlite3_file *pFile, पूर्णांक delFlag){
  devsym_file *p = (devsym_file *)pFile;
  वापस p->pReal->pMethods->xShmUnmap(p->pReal, delFlag);
}



/*
** Open an devsym file handle.
*/
अटल पूर्णांक devsymOpen(
  sqlite3_vfs *pVfs,
  स्थिर अक्षर *zName,
  sqlite3_file *pFile,
  पूर्णांक flags,
  पूर्णांक *pOutFlags
){
अटल sqlite3_io_methods devsym_io_methods = {
  2,                                /* iVersion */
  devsymClose,                      /* xClose */
  devsymRead,                       /* xRead */
  devsymWrite,                      /* xWrite */
  devsymTruncate,                   /* xTruncate */
  devsymSync,                       /* xSync */
  devsym_File_आकार,                   /* xFileSize */
  devsymLock,                       /* xLock */
  devsymUnlock,                     /* xUnlock */
  devsymCheckReservedLock,          /* xCheckReservedLock */
  devsymFileControl,                /* xFileControl */
  devsym_Sector_आकार,                 /* xSectorSize */
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
  अगर( p->pReal->pMethods ){
    pFile->pMethods = &devsym_io_methods;
  }
  वापस rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
अटल पूर्णांक devsymDelete(sqlite3_vfs *pVfs, स्थिर अक्षर *zPath, पूर्णांक dirSync){
  वापस sqlite3OsDelete(g.pVfs, zPath, dirSync);
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
अटल पूर्णांक devsymAccess(
  sqlite3_vfs *pVfs, 
  स्थिर अक्षर *zPath, 
  पूर्णांक flags, 
  पूर्णांक *pResOut
){
  वापस sqlite3OsAccess(g.pVfs, zPath, flags, pResOut);
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (DEVSYM_MAX_PATHNAME+1) bytes.
*/
अटल पूर्णांक devsymFullPathname(
  sqlite3_vfs *pVfs, 
  स्थिर अक्षर *zPath, 
  पूर्णांक nOut, 
  अक्षर *zOut
){
  वापस sqlite3OsFullPathname(g.pVfs, zPath, nOut, zOut);
}

#अगर_अघोषित SQLITE_OMIT_LOAD_EXTENSION
/*
** Open the dynamic library located at zPath and return a handle.
*/
अटल व्योम *devsymDlOpen(sqlite3_vfs *pVfs, स्थिर अक्षर *zPath){
  वापस sqlite3OsDlOpen(g.pVfs, zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
अटल व्योम devsymDlError(sqlite3_vfs *pVfs, पूर्णांक nByte, अक्षर *z_Err_संदेश){
  sqlite3OsDlError(g.pVfs, nByte, z_Err_संदेश);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
अटल व्योम (*devsymDlSym(sqlite3_vfs *pVfs, व्योम *p, स्थिर अक्षर *zSym))(व्योम){
  वापस sqlite3OsDlSym(g.pVfs, p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
अटल व्योम devsymDlClose(sqlite3_vfs *pVfs, व्योम *pHandle){
  sqlite3OsDlClose(g.pVfs, pHandle);
}
#पूर्ण_अगर /* SQLITE_OMIT_LOAD_EXTENSION */

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
अटल पूर्णांक devsymRandomness(sqlite3_vfs *pVfs, पूर्णांक nByte, अक्षर *zBufOut){
  वापस sqlite3OsRandomness(g.pVfs, nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
अटल पूर्णांक devsymSleep(sqlite3_vfs *pVfs, पूर्णांक nMicro){
  वापस sqlite3OsSleep(g.pVfs, nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
अटल पूर्णांक devsymCurrentTime(sqlite3_vfs *pVfs, द्विगुन *pTimeOut){
  वापस g.pVfs->xCurrentTime(g.pVfs, pTimeOut);
}

/*
** Return the sector-size in bytes for an writecrash-file.
*/
अटल पूर्णांक writecrash_Sector_आकार(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  वापस sqlite3_Os_Sector_आकार(p->pReal);
}

/*
** Return the device characteristic flags supported by an writecrash-file.
*/
अटल पूर्णांक writecrashDeviceCharacteristics(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  वापस sqlite3OsDeviceCharacteristics(p->pReal);
}

/*
** Write data to an writecrash-file.
*/
अटल पूर्णांक writecrashWrite(
  sqlite3_file *pFile, 
  स्थिर व्योम *zBuf, 
  पूर्णांक iAmt, 
  sqlite_int64 iOfst
){
  devsym_file *p = (devsym_file *)pFile;
  अगर( g.nWriteCrash>0 ){
    g.nWriteCrash--;
    अगर( g.nWriteCrash==0 ) पात();
  }
  वापस sqlite3OsWrite(p->pReal, zBuf, iAmt, iOfst);
}

/*
** Open an writecrash file handle.
*/
अटल पूर्णांक writecrashOpen(
  sqlite3_vfs *pVfs,
  स्थिर अक्षर *zName,
  sqlite3_file *pFile,
  पूर्णांक flags,
  पूर्णांक *pOutFlags
){
अटल sqlite3_io_methods writecrash_io_methods = {
  2,                                /* iVersion */
  devsymClose,                      /* xClose */
  devsymRead,                       /* xRead */
  writecrashWrite,                  /* xWrite */
  devsymTruncate,                   /* xTruncate */
  devsymSync,                       /* xSync */
  devsym_File_आकार,                   /* xFileSize */
  devsymLock,                       /* xLock */
  devsymUnlock,                     /* xUnlock */
  devsymCheckReservedLock,          /* xCheckReservedLock */
  devsymFileControl,                /* xFileControl */
  writecrash_Sector_आकार,             /* xSectorSize */
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
  अगर( p->pReal->pMethods ){
    pFile->pMethods = &writecrash_io_methods;
  }
  वापस rc;
}

अटल sqlite3_vfs devsym_vfs = {
  2,                     /* iVersion */
  माप(devsym_file),      /* szOsFile */
  DEVSYM_MAX_PATHNAME,      /* mxPathname */
  0,                     /* pNext */
  DEVSYM_VFS_NAME,          /* zName */
  0,                     /* pAppData */
  devsymOpen,               /* xOpen */
  devsymDelete,             /* xDelete */
  devsymAccess,             /* xAccess */
  devsymFullPathname,       /* xFullPathname */
#अगर_अघोषित SQLITE_OMIT_LOAD_EXTENSION
  devsymDlOpen,             /* xDlOpen */
  devsymDlError,            /* xDlError */
  devsymDlSym,              /* xDlSym */
  devsymDlClose,            /* xDlClose */
#अन्यथा
  0,                        /* xDlOpen */
  0,                        /* xDlError */
  0,                        /* xDlSym */
  0,                        /* xDlClose */
#पूर्ण_अगर /* SQLITE_OMIT_LOAD_EXTENSION */
  devsymRandomness,         /* xRandomness */
  devsymSleep,              /* xSleep */
  devsymCurrentTime,        /* xCurrentTime */
  0,                        /* xGetLastError */
  0                         /* xCurrentTimeInt64 */
};

अटल sqlite3_vfs writecrash_vfs = {
  2,                     /* iVersion */
  माप(devsym_file),      /* szOsFile */
  DEVSYM_MAX_PATHNAME,      /* mxPathname */
  0,                     /* pNext */
  WRITECRASH_NAME,          /* zName */
  0,                     /* pAppData */
  writecrashOpen,           /* xOpen */
  devsymDelete,             /* xDelete */
  devsymAccess,             /* xAccess */
  devsymFullPathname,       /* xFullPathname */
#अगर_अघोषित SQLITE_OMIT_LOAD_EXTENSION
  devsymDlOpen,             /* xDlOpen */
  devsymDlError,            /* xDlError */
  devsymDlSym,              /* xDlSym */
  devsymDlClose,            /* xDlClose */
#अन्यथा
  0,                        /* xDlOpen */
  0,                        /* xDlError */
  0,                        /* xDlSym */
  0,                        /* xDlClose */
#पूर्ण_अगर /* SQLITE_OMIT_LOAD_EXTENSION */
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
व्योम devsym_register(पूर्णांक iDeviceChar, पूर्णांक i_Sector_आकार){

  अगर( g.pVfs==0 ){
    g.pVfs = sqlite3_vfs_find(0);
    devsym_vfs.szOsFile += g.pVfs->szOsFile;
    writecrash_vfs.szOsFile += g.pVfs->szOsFile;
    sqlite3_vfs_register(&devsym_vfs, 0);
    sqlite3_vfs_register(&writecrash_vfs, 0);
  }
  अगर( iDeviceChar>=0 ){
    g.iDeviceChar = iDeviceChar;
  }अन्यथा{
    g.iDeviceChar = 0;
  }
  अगर( i_Sector_आकार>=0 ){
    g.i_Sector_आकार = i_Sector_आकार;
  }अन्यथा{
    g.i_Sector_आकार = 512;
  }
}

व्योम devsym_unregister(){
  sqlite3_vfs_unregister(&devsym_vfs);
  sqlite3_vfs_unregister(&writecrash_vfs);
  g.pVfs = 0;
  g.iDeviceChar = 0;
  g.i_Sector_आकार = 0;
}

व्योम devsym_crash_on_write(पूर्णांक nWrite){
  अगर( g.pVfs==0 ){
    g.pVfs = sqlite3_vfs_find(0);
    devsym_vfs.szOsFile += g.pVfs->szOsFile;
    writecrash_vfs.szOsFile += g.pVfs->szOsFile;
    sqlite3_vfs_register(&devsym_vfs, 0);
    sqlite3_vfs_register(&writecrash_vfs, 0);
  }
  g.nWriteCrash = nWrite;
}

#पूर्ण_अगर
