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
#تعریف_کړه DEVSYM_MAX_PATHNAME 512

/*
** Name used to identify this VFS.
*/
#تعریف_کړه DEVSYM_VFS_NAME "devsym"
#تعریف_کړه WRITECRASH_NAME "writecrash"

تعريفنوع بنية devsym_file devsym_file;
بنية devsym_file {
  sqlite3_file بنسټ;
  sqlite3_file *pReal;
};

/*
** Method declarations for devsym_file.
*/
ساكن صحيح devsymClose(sqlite3_file*);
ساكن صحيح devsymRead(sqlite3_file*, فراغ*, صحيح iAmt, sqlite3_int64 iOfst);
ساكن صحيح devsymWrite(sqlite3_file*,ثابت فراغ*,صحيح iAmt, sqlite3_int64 iOfst);
ساكن صحيح devsymTruncate(sqlite3_file*, sqlite3_int64 اندازه);
ساكن صحيح devsymSync(sqlite3_file*, صحيح flags);
ساكن صحيح devsym_File_اندازه(sqlite3_file*, sqlite3_int64 *p_اندازه);
ساكن صحيح devsym_قلف(sqlite3_file*, صحيح);
ساكن صحيح devsymUnlock(sqlite3_file*, صحيح);
ساكن صحيح devsym_Check_Reserved_قلف(sqlite3_file*, صحيح *);
ساكن صحيح devsymFileControl(sqlite3_file*, صحيح op, فراغ *pArg);
ساكن صحيح devsym_Sector_اندازه(sqlite3_file*);
ساكن صحيح devsymDeviceCharacteristics(sqlite3_file*);
ساكن صحيح devsym_Shm_قلف(sqlite3_file*,صحيح,صحيح,صحيح);
ساكن صحيح devsymShmMap(sqlite3_file*,صحيح,صحيح,صحيح, فراغ متطاير **);
ساكن فراغ devsymShmBarrier(sqlite3_file*);
ساكن صحيح devsymShmUnmap(sqlite3_file*,صحيح);

/*
** Method declarations for devsym_vfs.
*/
ساكن صحيح devsymOpen(sqlite3_vfs*, ثابت حرف *, sqlite3_file*, صحيح , صحيح *);
ساكن صحيح devsymDelete(sqlite3_vfs*, ثابت حرف *zName, صحيح syncDir);
ساكن صحيح devsymAccess(sqlite3_vfs*, ثابت حرف *zName, صحيح flags, صحيح *);
ساكن صحيح devsym_ډک_Pathname(sqlite3_vfs*, ثابت حرف *zName, صحيح, حرف *z_بهر);
#که_نه_تعریف_شوی SQLITE_OMIT_بارول_EXTENSION
ساكن فراغ *devsymDlOpen(sqlite3_vfs*, ثابت حرف *zFilename);
ساكن فراغ devsymDlError(sqlite3_vfs*, صحيح nByte, حرف *z_Err_پیغام);
ساكن فراغ (*devsymDlSym(sqlite3_vfs*,فراغ*, ثابت حرف *z_نښه))(فراغ);
ساكن فراغ devsymDlClose(sqlite3_vfs*, فراغ*);
#د_که_پای /* SQLITE_OMIT_LOAD_EXTENSION */
ساكن صحيح devsymRandomness(sqlite3_vfs*, صحيح nByte, حرف *z_بهر);
ساكن صحيح devsymSleep(sqlite3_vfs*, صحيح microseconds);
ساكن صحيح devsym_Current_وخت(sqlite3_vfs*, مزدوج*);

بنية DevsymGlobal {
  sqlite3_vfs *pVfs;
  صحيح iDeviceChar;
  صحيح i_Sector_اندازه;
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
ساكن صحيح devsymTruncate(sqlite3_file *pFile, sqlite_int64 اندازه){
  devsym_file *p = (devsym_file *)pFile;
  أرجع sqlite3OsTruncate(p->pReal, اندازه);
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
ساكن صحيح devsym_File_اندازه(sqlite3_file *pFile, sqlite_int64 *p_اندازه){
  devsym_file *p = (devsym_file *)pFile;
  أرجع sqlite3_Os_File_اندازه(p->pReal, p_اندازه);
}

/*
** Lock an devsym-file.
*/
ساكن صحيح devsym_قلف(sqlite3_file *pFile, صحيح e_قلف){
  devsym_file *p = (devsym_file *)pFile;
  أرجع sqlite3_Os_قلف(p->pReal, e_قلف);
}

/*
** Unlock an devsym-file.
*/
ساكن صحيح devsymUnlock(sqlite3_file *pFile, صحيح e_قلف){
  devsym_file *p = (devsym_file *)pFile;
  أرجع sqlite3OsUnlock(p->pReal, e_قلف);
}

/*
** Check if another file-handle holds a RESERVED lock on an devsym-file.
*/
ساكن صحيح devsym_Check_Reserved_قلف(sqlite3_file *pFile, صحيح *p_Res_بهر){
  devsym_file *p = (devsym_file *)pFile;
  أرجع sqlite3_Os_Check_Reserved_قلف(p->pReal, p_Res_بهر);
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
ساكن صحيح devsym_Sector_اندازه(sqlite3_file *pFile){
  أرجع g.i_Sector_اندازه;
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
ساكن صحيح devsym_Shm_قلف(sqlite3_file *pFile, صحيح ofst, صحيح n, صحيح flags){
  devsym_file *p = (devsym_file *)pFile;
  أرجع p->pReal->pMethods->x_Shm_قلف(p->pReal, ofst, n, flags);
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
  صحيح *p_بهر_Flags
){
ساكن sqlite3_io_methods devsym_io_methods = {
  2,                                /* iVersion */
  devsymClose,                      /* xClose */
  devsymRead,                       /* xRead */
  devsymWrite,                      /* xWrite */
  devsymTruncate,                   /* xTruncate */
  devsymSync,                       /* xSync */
  devsym_File_اندازه,                   /* xFileSize */
  devsym_قلف,                       /* xLock */
  devsymUnlock,                     /* xUnlock */
  devsym_Check_Reserved_قلف,          /* xCheckReservedLock */
  devsymFileControl,                /* xFileControl */
  devsym_Sector_اندازه,                 /* xSectorSize */
  devsymDeviceCharacteristics,      /* xDeviceCharacteristics */
  devsymShmMap,                     /* xShmMap */
  devsym_Shm_قلف,                    /* xShmLock */
  devsymShmBarrier,                 /* xShmBarrier */
  devsymShmUnmap                    /* xShmUnmap */
};

  صحيح rc;
  devsym_file *p = (devsym_file *)pFile;
  p->pReal = (sqlite3_file *)&p[1];
  rc = sqlite3OsOpen(g.pVfs, zName, p->pReal, flags, p_بهر_Flags);
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
ساكن صحيح devsymDelete(sqlite3_vfs *pVfs, ثابت حرف *z_لار, صحيح dirSync){
  أرجع sqlite3OsDelete(g.pVfs, z_لار, dirSync);
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
ساكن صحيح devsymAccess(
  sqlite3_vfs *pVfs, 
  ثابت حرف *z_لار, 
  صحيح flags, 
  صحيح *p_Res_بهر
){
  أرجع sqlite3OsAccess(g.pVfs, z_لار, flags, p_Res_بهر);
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (DEVSYM_MAX_PATHNAME+1) bytes.
*/
ساكن صحيح devsym_ډک_Pathname(
  sqlite3_vfs *pVfs, 
  ثابت حرف *z_لار, 
  صحيح n_بهر, 
  حرف *z_بهر
){
  أرجع sqlite3_Os_ډک_Pathname(g.pVfs, z_لار, n_بهر, z_بهر);
}

#که_نه_تعریف_شوی SQLITE_OMIT_بارول_EXTENSION
/*
** Open the dynamic library located at zPath and return a handle.
*/
ساكن فراغ *devsymDlOpen(sqlite3_vfs *pVfs, ثابت حرف *z_لار){
  أرجع sqlite3OsDlOpen(g.pVfs, z_لار);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
ساكن فراغ devsymDlError(sqlite3_vfs *pVfs, صحيح nByte, حرف *z_Err_پیغام){
  sqlite3OsDlError(g.pVfs, nByte, z_Err_پیغام);
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
#د_که_پای /* SQLITE_OMIT_LOAD_EXTENSION */

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
ساكن صحيح devsymRandomness(sqlite3_vfs *pVfs, صحيح nByte, حرف *z_Buf_بهر){
  أرجع sqlite3OsRandomness(g.pVfs, nByte, z_Buf_بهر);
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
ساكن صحيح devsym_Current_وخت(sqlite3_vfs *pVfs, مزدوج *p_وخت_بهر){
  أرجع g.pVfs->x_Current_وخت(g.pVfs, p_وخت_بهر);
}

/*
** Return the sector-size in bytes for an writecrash-file.
*/
ساكن صحيح writecrash_Sector_اندازه(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  أرجع sqlite3_Os_Sector_اندازه(p->pReal);
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
  صحيح *p_بهر_Flags
){
ساكن sqlite3_io_methods writecrash_io_methods = {
  2,                                /* iVersion */
  devsymClose,                      /* xClose */
  devsymRead,                       /* xRead */
  writecrashWrite,                  /* xWrite */
  devsymTruncate,                   /* xTruncate */
  devsymSync,                       /* xSync */
  devsym_File_اندازه,                   /* xFileSize */
  devsym_قلف,                       /* xLock */
  devsymUnlock,                     /* xUnlock */
  devsym_Check_Reserved_قلف,          /* xCheckReservedLock */
  devsymFileControl,                /* xFileControl */
  writecrash_Sector_اندازه,             /* xSectorSize */
  writecrashDeviceCharacteristics,  /* xDeviceCharacteristics */
  devsymShmMap,                     /* xShmMap */
  devsym_Shm_قلف,                    /* xShmLock */
  devsymShmBarrier,                 /* xShmBarrier */
  devsymShmUnmap                    /* xShmUnmap */
};

  صحيح rc;
  devsym_file *p = (devsym_file *)pFile;
  p->pReal = (sqlite3_file *)&p[1];
  rc = sqlite3OsOpen(g.pVfs, zName, p->pReal, flags, p_بهر_Flags);
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
  devsym_ډک_Pathname,       /* xFullPathname */
#که_نه_تعریف_شوی SQLITE_OMIT_بارول_EXTENSION
  devsymDlOpen,             /* xDlOpen */
  devsymDlError,            /* xDlError */
  devsymDlSym,              /* xDlSym */
  devsymDlClose,            /* xDlClose */
#وإلا
  0,                        /* xDlOpen */
  0,                        /* xDlError */
  0,                        /* xDlSym */
  0,                        /* xDlClose */
#د_که_پای /* SQLITE_OMIT_LOAD_EXTENSION */
  devsymRandomness,         /* xRandomness */
  devsymSleep,              /* xSleep */
  devsym_Current_وخت,        /* xCurrentTime */
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
  devsym_ډک_Pathname,       /* xFullPathname */
#که_نه_تعریف_شوی SQLITE_OMIT_بارول_EXTENSION
  devsymDlOpen,             /* xDlOpen */
  devsymDlError,            /* xDlError */
  devsymDlSym,              /* xDlSym */
  devsymDlClose,            /* xDlClose */
#وإلا
  0,                        /* xDlOpen */
  0,                        /* xDlError */
  0,                        /* xDlSym */
  0,                        /* xDlClose */
#د_که_پای /* SQLITE_OMIT_LOAD_EXTENSION */
  devsymRandomness,         /* xRandomness */
  devsymSleep,              /* xSleep */
  devsym_Current_وخت,        /* xCurrentTime */
  0,                        /* xGetLastError */
  0                         /* xCurrentTimeInt64 */
};


/*
** This procedure registers the devsym vfs with SQLite. If the argument is
** true, the devsym vfs becomes the new default vfs. It is the only publicly
** available function in this file.
*/
فراغ devsym_ثبتوونکی(صحيح iDeviceChar, صحيح i_Sector_اندازه){

  إذا( g.pVfs==0 ){
    g.pVfs = sqlite3_vfs_پیدا_کړه(0);
    devsym_vfs.szOsFile += g.pVfs->szOsFile;
    writecrash_vfs.szOsFile += g.pVfs->szOsFile;
    sqlite3_vfs_ثبتوونکی(&devsym_vfs, 0);
    sqlite3_vfs_ثبتوونکی(&writecrash_vfs, 0);
  }
  إذا( iDeviceChar>=0 ){
    g.iDeviceChar = iDeviceChar;
  }وإلا{
    g.iDeviceChar = 0;
  }
  إذا( i_Sector_اندازه>=0 ){
    g.i_Sector_اندازه = i_Sector_اندازه;
  }وإلا{
    g.i_Sector_اندازه = 512;
  }
}

فراغ devsym_unregister(){
  sqlite3_vfs_unregister(&devsym_vfs);
  sqlite3_vfs_unregister(&writecrash_vfs);
  g.pVfs = 0;
  g.iDeviceChar = 0;
  g.i_Sector_اندازه = 0;
}

فراغ devsym_crash_on_write(صحيح nWrite){
  إذا( g.pVfs==0 ){
    g.pVfs = sqlite3_vfs_پیدا_کړه(0);
    devsym_vfs.szOsFile += g.pVfs->szOsFile;
    writecrash_vfs.szOsFile += g.pVfs->szOsFile;
    sqlite3_vfs_ثبتوونکی(&devsym_vfs, 0);
    sqlite3_vfs_ثبتوونکی(&writecrash_vfs, 0);
  }
  g.nWriteCrash = nWrite;
}

#د_که_پای
