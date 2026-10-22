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
#यदि SQLITE_TEST          /* This file is used for testing only */

#अन्तर्भाव "sqlite3.h"
#अन्तर्भाव "sqliteInt.h"

/*
** Maximum pathname length supported by the devsym backend.
*/
#परिभाषय DEVSYM_MAX_PATHNAME 512

/*
** Name used to identify this VFS.
*/
#परिभाषय DEVSYM_VFS_NAME "devsym"
#परिभाषय WRITECRASH_NAME "writecrash"

प्रकारनिर्देश संरचना devsym_file devsym_file;
संरचना devsym_file {
  sqlite3_file आधारः;
  sqlite3_file *pReal;
};

/*
** Method declarations for devsym_file.
*/
स्थिर पूर्णाङ्क devsymClose(sqlite3_file*);
स्थिर पूर्णाङ्क devsymRead(sqlite3_file*, शून्य*, पूर्णाङ्क iAmt, sqlite3_int64 iOfst);
स्थिर पूर्णाङ्क devsymWrite(sqlite3_file*,ध्रुव शून्य*,पूर्णाङ्क iAmt, sqlite3_int64 iOfst);
स्थिर पूर्णाङ्क devsymTruncate(sqlite3_file*, sqlite3_int64 आकार);
स्थिर पूर्णाङ्क devsymSync(sqlite3_file*, पूर्णाङ्क flags);
स्थिर पूर्णाङ्क devsym_File_आकार(sqlite3_file*, sqlite3_int64 *p_आकार);
स्थिर पूर्णाङ्क devsym_तालकम्(sqlite3_file*, पूर्णाङ्क);
स्थिर पूर्णाङ्क devsymUnlock(sqlite3_file*, पूर्णाङ्क);
स्थिर पूर्णाङ्क devsym_Check_Reserved_तालकम्(sqlite3_file*, पूर्णाङ्क *);
स्थिर पूर्णाङ्क devsymFileControl(sqlite3_file*, पूर्णाङ्क op, शून्य *pArg);
स्थिर पूर्णाङ्क devsym_Sector_आकार(sqlite3_file*);
स्थिर पूर्णाङ्क devsymDeviceCharacteristics(sqlite3_file*);
स्थिर पूर्णाङ्क devsym_Shm_तालकम्(sqlite3_file*,पूर्णाङ्क,पूर्णाङ्क,पूर्णाङ्क);
स्थिर पूर्णाङ्क devsymShmMap(sqlite3_file*,पूर्णाङ्क,पूर्णाङ्क,पूर्णाङ्क, शून्य चञ्चल **);
स्थिर शून्य devsymShmBarrier(sqlite3_file*);
स्थिर पूर्णाङ्क devsymShmUnmap(sqlite3_file*,पूर्णाङ्क);

/*
** Method declarations for devsym_vfs.
*/
स्थिर पूर्णाङ्क devsymOpen(sqlite3_vfs*, ध्रुव वर्ण *, sqlite3_file*, पूर्णाङ्क , पूर्णाङ्क *);
स्थिर पूर्णाङ्क devsymDelete(sqlite3_vfs*, ध्रुव वर्ण *zName, पूर्णाङ्क syncDir);
स्थिर पूर्णाङ्क devsymAccess(sqlite3_vfs*, ध्रुव वर्ण *zName, पूर्णाङ्क flags, पूर्णाङ्क *);
स्थिर पूर्णाङ्क devsym_पूर्णम्_Pathname(sqlite3_vfs*, ध्रुव वर्ण *zName, पूर्णाङ्क, वर्ण *z_बहिर्गमनम्);
#यदि_अपरिभाषितम् SQLITE_OMIT_आरोपय_EXTENSION
स्थिर शून्य *devsymDlOpen(sqlite3_vfs*, ध्रुव वर्ण *zFilename);
स्थिर शून्य devsymDlError(sqlite3_vfs*, पूर्णाङ्क nByte, वर्ण *z_Err_सन्देशः);
स्थिर शून्य (*devsymDlSym(sqlite3_vfs*,शून्य*, ध्रुव वर्ण *z_चिह्नम्))(शून्य);
स्थिर शून्य devsymDlClose(sqlite3_vfs*, शून्य*);
#यदि_अन्तः /* SQLITE_OMIT_LOAD_EXTENSION */
स्थिर पूर्णाङ्क devsymRandomness(sqlite3_vfs*, पूर्णाङ्क nByte, वर्ण *z_बहिर्गमनम्);
स्थिर पूर्णाङ्क devsymSleep(sqlite3_vfs*, पूर्णाङ्क microseconds);
स्थिर पूर्णाङ्क devsymCurrentTime(sqlite3_vfs*, द्विगुण*);

संरचना DevsymGlobal {
  sqlite3_vfs *pVfs;
  पूर्णाङ्क iDeviceChar;
  पूर्णाङ्क i_Sector_आकार;
  पूर्णाङ्क nWriteCrash;
};
संरचना DevsymGlobal g = {0, 0, 512, 0};

/*
** Close an devsym-file.
*/
स्थिर पूर्णाङ्क devsymClose(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  sqlite3OsClose(p->pReal);
  प्रत्यागम SQLITE_OK;
}

/*
** Read data from an devsym-file.
*/
स्थिर पूर्णाङ्क devsymRead(
  sqlite3_file *pFile, 
  शून्य *zBuf, 
  पूर्णाङ्क iAmt, 
  sqlite_int64 iOfst
){
  devsym_file *p = (devsym_file *)pFile;
  प्रत्यागम sqlite3OsRead(p->pReal, zBuf, iAmt, iOfst);
}

/*
** Write data to an devsym-file.
*/
स्थिर पूर्णाङ्क devsymWrite(
  sqlite3_file *pFile, 
  ध्रुव शून्य *zBuf, 
  पूर्णाङ्क iAmt, 
  sqlite_int64 iOfst
){
  devsym_file *p = (devsym_file *)pFile;
  प्रत्यागम sqlite3OsWrite(p->pReal, zBuf, iAmt, iOfst);
}

/*
** Truncate an devsym-file.
*/
स्थिर पूर्णाङ्क devsymTruncate(sqlite3_file *pFile, sqlite_int64 आकार){
  devsym_file *p = (devsym_file *)pFile;
  प्रत्यागम sqlite3OsTruncate(p->pReal, आकार);
}

/*
** Sync an devsym-file.
*/
स्थिर पूर्णाङ्क devsymSync(sqlite3_file *pFile, पूर्णाङ्क flags){
  devsym_file *p = (devsym_file *)pFile;
  प्रत्यागम sqlite3OsSync(p->pReal, flags);
}

/*
** Return the current file-size of an devsym-file.
*/
स्थिर पूर्णाङ्क devsym_File_आकार(sqlite3_file *pFile, sqlite_int64 *p_आकार){
  devsym_file *p = (devsym_file *)pFile;
  प्रत्यागम sqlite3_Os_File_आकार(p->pReal, p_आकार);
}

/*
** Lock an devsym-file.
*/
स्थिर पूर्णाङ्क devsym_तालकम्(sqlite3_file *pFile, पूर्णाङ्क e_तालकम्){
  devsym_file *p = (devsym_file *)pFile;
  प्रत्यागम sqlite3_Os_तालकम्(p->pReal, e_तालकम्);
}

/*
** Unlock an devsym-file.
*/
स्थिर पूर्णाङ्क devsymUnlock(sqlite3_file *pFile, पूर्णाङ्क e_तालकम्){
  devsym_file *p = (devsym_file *)pFile;
  प्रत्यागम sqlite3OsUnlock(p->pReal, e_तालकम्);
}

/*
** Check if another file-handle holds a RESERVED lock on an devsym-file.
*/
स्थिर पूर्णाङ्क devsym_Check_Reserved_तालकम्(sqlite3_file *pFile, पूर्णाङ्क *p_Res_बहिर्गमनम्){
  devsym_file *p = (devsym_file *)pFile;
  प्रत्यागम sqlite3_Os_Check_Reserved_तालकम्(p->pReal, p_Res_बहिर्गमनम्);
}

/*
** File control method. For custom operations on an devsym-file.
*/
स्थिर पूर्णाङ्क devsymFileControl(sqlite3_file *pFile, पूर्णाङ्क op, शून्य *pArg){
  devsym_file *p = (devsym_file *)pFile;
  प्रत्यागम sqlite3OsFileControl(p->pReal, op, pArg);
}

/*
** Return the sector-size in bytes for an devsym-file.
*/
स्थिर पूर्णाङ्क devsym_Sector_आकार(sqlite3_file *pFile){
  प्रत्यागम g.i_Sector_आकार;
}

/*
** Return the device characteristic flags supported by an devsym-file.
*/
स्थिर पूर्णाङ्क devsymDeviceCharacteristics(sqlite3_file *pFile){
  प्रत्यागम g.iDeviceChar;
}

/*
** Shared-memory methods are all pass-throughs.
*/
स्थिर पूर्णाङ्क devsym_Shm_तालकम्(sqlite3_file *pFile, पूर्णाङ्क ofst, पूर्णाङ्क n, पूर्णाङ्क flags){
  devsym_file *p = (devsym_file *)pFile;
  प्रत्यागम p->pReal->pMethods->x_Shm_तालकम्(p->pReal, ofst, n, flags);
}
स्थिर पूर्णाङ्क devsymShmMap(
  sqlite3_file *pFile, 
  पूर्णाङ्क iRegion, 
  पूर्णाङ्क szRegion, 
  पूर्णाङ्क isWrite, 
  शून्य चञ्चल **pp
){
  devsym_file *p = (devsym_file *)pFile;
  प्रत्यागम p->pReal->pMethods->xShmMap(p->pReal, iRegion, szRegion, isWrite, pp);
}
स्थिर शून्य devsymShmBarrier(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  p->pReal->pMethods->xShmBarrier(p->pReal);
}
स्थिर पूर्णाङ्क devsymShmUnmap(sqlite3_file *pFile, पूर्णाङ्क delFlag){
  devsym_file *p = (devsym_file *)pFile;
  प्रत्यागम p->pReal->pMethods->xShmUnmap(p->pReal, delFlag);
}



/*
** Open an devsym file handle.
*/
स्थिर पूर्णाङ्क devsymOpen(
  sqlite3_vfs *pVfs,
  ध्रुव वर्ण *zName,
  sqlite3_file *pFile,
  पूर्णाङ्क flags,
  पूर्णाङ्क *p_बहिर्गमनम्_Flags
){
स्थिर sqlite3_io_methods devsym_io_methods = {
  2,                                /* iVersion */
  devsymClose,                      /* xClose */
  devsymRead,                       /* xRead */
  devsymWrite,                      /* xWrite */
  devsymTruncate,                   /* xTruncate */
  devsymSync,                       /* xSync */
  devsym_File_आकार,                   /* xFileSize */
  devsym_तालकम्,                       /* xLock */
  devsymUnlock,                     /* xUnlock */
  devsym_Check_Reserved_तालकम्,          /* xCheckReservedLock */
  devsymFileControl,                /* xFileControl */
  devsym_Sector_आकार,                 /* xSectorSize */
  devsymDeviceCharacteristics,      /* xDeviceCharacteristics */
  devsymShmMap,                     /* xShmMap */
  devsym_Shm_तालकम्,                    /* xShmLock */
  devsymShmBarrier,                 /* xShmBarrier */
  devsymShmUnmap                    /* xShmUnmap */
};

  पूर्णाङ्क rc;
  devsym_file *p = (devsym_file *)pFile;
  p->pReal = (sqlite3_file *)&p[1];
  rc = sqlite3OsOpen(g.pVfs, zName, p->pReal, flags, p_बहिर्गमनम्_Flags);
  यदि( p->pReal->pMethods ){
    pFile->pMethods = &devsym_io_methods;
  }
  प्रत्यागम rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
स्थिर पूर्णाङ्क devsymDelete(sqlite3_vfs *pVfs, ध्रुव वर्ण *z_मार्गः, पूर्णाङ्क dirSync){
  प्रत्यागम sqlite3OsDelete(g.pVfs, z_मार्गः, dirSync);
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
स्थिर पूर्णाङ्क devsymAccess(
  sqlite3_vfs *pVfs, 
  ध्रुव वर्ण *z_मार्गः, 
  पूर्णाङ्क flags, 
  पूर्णाङ्क *p_Res_बहिर्गमनम्
){
  प्रत्यागम sqlite3OsAccess(g.pVfs, z_मार्गः, flags, p_Res_बहिर्गमनम्);
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (DEVSYM_MAX_PATHNAME+1) bytes.
*/
स्थिर पूर्णाङ्क devsym_पूर्णम्_Pathname(
  sqlite3_vfs *pVfs, 
  ध्रुव वर्ण *z_मार्गः, 
  पूर्णाङ्क n_बहिर्गमनम्, 
  वर्ण *z_बहिर्गमनम्
){
  प्रत्यागम sqlite3_Os_पूर्णम्_Pathname(g.pVfs, z_मार्गः, n_बहिर्गमनम्, z_बहिर्गमनम्);
}

#यदि_अपरिभाषितम् SQLITE_OMIT_आरोपय_EXTENSION
/*
** Open the dynamic library located at zPath and return a handle.
*/
स्थिर शून्य *devsymDlOpen(sqlite3_vfs *pVfs, ध्रुव वर्ण *z_मार्गः){
  प्रत्यागम sqlite3OsDlOpen(g.pVfs, z_मार्गः);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
स्थिर शून्य devsymDlError(sqlite3_vfs *pVfs, पूर्णाङ्क nByte, वर्ण *z_Err_सन्देशः){
  sqlite3OsDlError(g.pVfs, nByte, z_Err_सन्देशः);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
स्थिर शून्य (*devsymDlSym(sqlite3_vfs *pVfs, शून्य *p, ध्रुव वर्ण *zSym))(शून्य){
  प्रत्यागम sqlite3OsDlSym(g.pVfs, p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
स्थिर शून्य devsymDlClose(sqlite3_vfs *pVfs, शून्य *pHandle){
  sqlite3OsDlClose(g.pVfs, pHandle);
}
#यदि_अन्तः /* SQLITE_OMIT_LOAD_EXTENSION */

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
स्थिर पूर्णाङ्क devsymRandomness(sqlite3_vfs *pVfs, पूर्णाङ्क nByte, वर्ण *z_Buf_बहिर्गमनम्){
  प्रत्यागम sqlite3OsRandomness(g.pVfs, nByte, z_Buf_बहिर्गमनम्);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
स्थिर पूर्णाङ्क devsymSleep(sqlite3_vfs *pVfs, पूर्णाङ्क nMicro){
  प्रत्यागम sqlite3OsSleep(g.pVfs, nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
स्थिर पूर्णाङ्क devsymCurrentTime(sqlite3_vfs *pVfs, द्विगुण *p_Time_बहिर्गमनम्){
  प्रत्यागम g.pVfs->xCurrentTime(g.pVfs, p_Time_बहिर्गमनम्);
}

/*
** Return the sector-size in bytes for an writecrash-file.
*/
स्थिर पूर्णाङ्क writecrash_Sector_आकार(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  प्रत्यागम sqlite3_Os_Sector_आकार(p->pReal);
}

/*
** Return the device characteristic flags supported by an writecrash-file.
*/
स्थिर पूर्णाङ्क writecrashDeviceCharacteristics(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  प्रत्यागम sqlite3OsDeviceCharacteristics(p->pReal);
}

/*
** Write data to an writecrash-file.
*/
स्थिर पूर्णाङ्क writecrashWrite(
  sqlite3_file *pFile, 
  ध्रुव शून्य *zBuf, 
  पूर्णाङ्क iAmt, 
  sqlite_int64 iOfst
){
  devsym_file *p = (devsym_file *)pFile;
  यदि( g.nWriteCrash>0 ){
    g.nWriteCrash--;
    यदि( g.nWriteCrash==0 ) abort();
  }
  प्रत्यागम sqlite3OsWrite(p->pReal, zBuf, iAmt, iOfst);
}

/*
** Open an writecrash file handle.
*/
स्थिर पूर्णाङ्क writecrashOpen(
  sqlite3_vfs *pVfs,
  ध्रुव वर्ण *zName,
  sqlite3_file *pFile,
  पूर्णाङ्क flags,
  पूर्णाङ्क *p_बहिर्गमनम्_Flags
){
स्थिर sqlite3_io_methods writecrash_io_methods = {
  2,                                /* iVersion */
  devsymClose,                      /* xClose */
  devsymRead,                       /* xRead */
  writecrashWrite,                  /* xWrite */
  devsymTruncate,                   /* xTruncate */
  devsymSync,                       /* xSync */
  devsym_File_आकार,                   /* xFileSize */
  devsym_तालकम्,                       /* xLock */
  devsymUnlock,                     /* xUnlock */
  devsym_Check_Reserved_तालकम्,          /* xCheckReservedLock */
  devsymFileControl,                /* xFileControl */
  writecrash_Sector_आकार,             /* xSectorSize */
  writecrashDeviceCharacteristics,  /* xDeviceCharacteristics */
  devsymShmMap,                     /* xShmMap */
  devsym_Shm_तालकम्,                    /* xShmLock */
  devsymShmBarrier,                 /* xShmBarrier */
  devsymShmUnmap                    /* xShmUnmap */
};

  पूर्णाङ्क rc;
  devsym_file *p = (devsym_file *)pFile;
  p->pReal = (sqlite3_file *)&p[1];
  rc = sqlite3OsOpen(g.pVfs, zName, p->pReal, flags, p_बहिर्गमनम्_Flags);
  यदि( p->pReal->pMethods ){
    pFile->pMethods = &writecrash_io_methods;
  }
  प्रत्यागम rc;
}

स्थिर sqlite3_vfs devsym_vfs = {
  2,                     /* iVersion */
  परिमाण(devsym_file),      /* szOsFile */
  DEVSYM_MAX_PATHNAME,      /* mxPathname */
  0,                     /* pNext */
  DEVSYM_VFS_NAME,          /* zName */
  0,                     /* pAppData */
  devsymOpen,               /* xOpen */
  devsymDelete,             /* xDelete */
  devsymAccess,             /* xAccess */
  devsym_पूर्णम्_Pathname,       /* xFullPathname */
#यदि_अपरिभाषितम् SQLITE_OMIT_आरोपय_EXTENSION
  devsymDlOpen,             /* xDlOpen */
  devsymDlError,            /* xDlError */
  devsymDlSym,              /* xDlSym */
  devsymDlClose,            /* xDlClose */
#अन्यथा
  0,                        /* xDlOpen */
  0,                        /* xDlError */
  0,                        /* xDlSym */
  0,                        /* xDlClose */
#यदि_अन्तः /* SQLITE_OMIT_LOAD_EXTENSION */
  devsymRandomness,         /* xRandomness */
  devsymSleep,              /* xSleep */
  devsymCurrentTime,        /* xCurrentTime */
  0,                        /* xGetLastError */
  0                         /* xCurrentTimeInt64 */
};

स्थिर sqlite3_vfs writecrash_vfs = {
  2,                     /* iVersion */
  परिमाण(devsym_file),      /* szOsFile */
  DEVSYM_MAX_PATHNAME,      /* mxPathname */
  0,                     /* pNext */
  WRITECRASH_NAME,          /* zName */
  0,                     /* pAppData */
  writecrashOpen,           /* xOpen */
  devsymDelete,             /* xDelete */
  devsymAccess,             /* xAccess */
  devsym_पूर्णम्_Pathname,       /* xFullPathname */
#यदि_अपरिभाषितम् SQLITE_OMIT_आरोपय_EXTENSION
  devsymDlOpen,             /* xDlOpen */
  devsymDlError,            /* xDlError */
  devsymDlSym,              /* xDlSym */
  devsymDlClose,            /* xDlClose */
#अन्यथा
  0,                        /* xDlOpen */
  0,                        /* xDlError */
  0,                        /* xDlSym */
  0,                        /* xDlClose */
#यदि_अन्तः /* SQLITE_OMIT_LOAD_EXTENSION */
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
शून्य devsym_पञ्जिकाकोशः(पूर्णाङ्क iDeviceChar, पूर्णाङ्क i_Sector_आकार){

  यदि( g.pVfs==0 ){
    g.pVfs = sqlite3_vfs_find(0);
    devsym_vfs.szOsFile += g.pVfs->szOsFile;
    writecrash_vfs.szOsFile += g.pVfs->szOsFile;
    sqlite3_vfs_पञ्जिकाकोशः(&devsym_vfs, 0);
    sqlite3_vfs_पञ्जिकाकोशः(&writecrash_vfs, 0);
  }
  यदि( iDeviceChar>=0 ){
    g.iDeviceChar = iDeviceChar;
  }अन्यथा{
    g.iDeviceChar = 0;
  }
  यदि( i_Sector_आकार>=0 ){
    g.i_Sector_आकार = i_Sector_आकार;
  }अन्यथा{
    g.i_Sector_आकार = 512;
  }
}

शून्य devsym_unregister(){
  sqlite3_vfs_unregister(&devsym_vfs);
  sqlite3_vfs_unregister(&writecrash_vfs);
  g.pVfs = 0;
  g.iDeviceChar = 0;
  g.i_Sector_आकार = 0;
}

शून्य devsym_crash_on_write(पूर्णाङ्क nWrite){
  यदि( g.pVfs==0 ){
    g.pVfs = sqlite3_vfs_find(0);
    devsym_vfs.szOsFile += g.pVfs->szOsFile;
    writecrash_vfs.szOsFile += g.pVfs->szOsFile;
    sqlite3_vfs_पञ्जिकाकोशः(&devsym_vfs, 0);
    sqlite3_vfs_पञ्जिकाकोशः(&writecrash_vfs, 0);
  }
  g.nWriteCrash = nWrite;
}

#यदि_अन्तः
