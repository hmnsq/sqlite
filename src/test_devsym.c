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
#اگر SQLITE_TEST          /* This file is used for testing only */

#شامل "sqlite3.h"
#شامل "sqliteInt.h"

/*
** Maximum pathname length supported by the devsym backend.
*/
#تعریف_کرو DEVSYM_MAX_PATHNAME 512

/*
** Name used to identify this VFS.
*/
#تعریف_کرو DEVSYM_VFS_NAME "devsym"
#تعریف_کرو WRITECRASH_NAME "writecrash"

قسمتعریف ساخت devsym_file devsym_file;
ساخت devsym_file {
  sqlite3_file بنیاد;
  sqlite3_file *pReal;
};

/*
** Method declarations for devsym_file.
*/
ساکن صحیح devsymClose(sqlite3_file*);
ساکن صحیح devsymRead(sqlite3_file*, خالی*, صحیح iAmt, sqlite3_int64 iOfst);
ساکن صحیح devsymWrite(sqlite3_file*,ثابت خالی*,صحیح iAmt, sqlite3_int64 iOfst);
ساکن صحیح devsymTruncate(sqlite3_file*, sqlite3_int64 سائز);
ساکن صحیح devsymSync(sqlite3_file*, صحیح flags);
ساکن صحیح devsym_File_سائز(sqlite3_file*, sqlite3_int64 *p_سائز);
ساکن صحیح devsym_تالا(sqlite3_file*, صحیح);
ساکن صحیح devsymUnlock(sqlite3_file*, صحیح);
ساکن صحیح devsym_Check_Reserved_تالا(sqlite3_file*, صحیح *);
ساکن صحیح devsymFileControl(sqlite3_file*, صحیح op, خالی *pArg);
ساکن صحیح devsym_Sector_سائز(sqlite3_file*);
ساکن صحیح devsymDeviceCharacteristics(sqlite3_file*);
ساکن صحیح devsym_Shm_تالا(sqlite3_file*,صحیح,صحیح,صحیح);
ساکن صحیح devsymShmMap(sqlite3_file*,صحیح,صحیح,صحیح, خالی متلون **);
ساکن خالی devsymShmBarrier(sqlite3_file*);
ساکن صحیح devsymShmUnmap(sqlite3_file*,صحیح);

/*
** Method declarations for devsym_vfs.
*/
ساکن صحیح devsymOpen(sqlite3_vfs*, ثابت حرف *, sqlite3_file*, صحیح , صحیح *);
ساکن صحیح devsymDelete(sqlite3_vfs*, ثابت حرف *zName, صحیح syncDir);
ساکن صحیح devsymAccess(sqlite3_vfs*, ثابت حرف *zName, صحیح flags, صحیح *);
ساکن صحیح devsym_بھرا_Pathname(sqlite3_vfs*, ثابت حرف *zName, صحیح, حرف *z_برآمد);
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_بھرو_EXTENSION
ساکن خالی *devsymDlOpen(sqlite3_vfs*, ثابت حرف *zFilename);
ساکن خالی devsymDlError(sqlite3_vfs*, صحیح nByte, حرف *z_Err_پیغام);
ساکن خالی (*devsymDlSym(sqlite3_vfs*,خالی*, ثابت حرف *z_علامت))(خالی);
ساکن خالی devsymDlClose(sqlite3_vfs*, خالی*);
#اگر_ختم /* SQLITE_OMIT_LOAD_EXTENSION */
ساکن صحیح devsymRandomness(sqlite3_vfs*, صحیح nByte, حرف *z_برآمد);
ساکن صحیح devsymSleep(sqlite3_vfs*, صحیح microseconds);
ساکن صحیح devsym_Current_وقت(sqlite3_vfs*, دوہرا*);

ساخت DevsymGlobal {
  sqlite3_vfs *pVfs;
  صحیح iDeviceChar;
  صحیح i_Sector_سائز;
  صحیح nWriteCrash;
};
ساخت DevsymGlobal g = {0, 0, 512, 0};

/*
** Close an devsym-file.
*/
ساکن صحیح devsymClose(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  sqlite3OsClose(p->pReal);
  واپس SQLITE_OK;
}

/*
** Read data from an devsym-file.
*/
ساکن صحیح devsymRead(
  sqlite3_file *pFile, 
  خالی *zBuf, 
  صحیح iAmt, 
  sqlite_int64 iOfst
){
  devsym_file *p = (devsym_file *)pFile;
  واپس sqlite3OsRead(p->pReal, zBuf, iAmt, iOfst);
}

/*
** Write data to an devsym-file.
*/
ساکن صحیح devsymWrite(
  sqlite3_file *pFile, 
  ثابت خالی *zBuf, 
  صحیح iAmt, 
  sqlite_int64 iOfst
){
  devsym_file *p = (devsym_file *)pFile;
  واپس sqlite3OsWrite(p->pReal, zBuf, iAmt, iOfst);
}

/*
** Truncate an devsym-file.
*/
ساکن صحیح devsymTruncate(sqlite3_file *pFile, sqlite_int64 سائز){
  devsym_file *p = (devsym_file *)pFile;
  واپس sqlite3OsTruncate(p->pReal, سائز);
}

/*
** Sync an devsym-file.
*/
ساکن صحیح devsymSync(sqlite3_file *pFile, صحیح flags){
  devsym_file *p = (devsym_file *)pFile;
  واپس sqlite3OsSync(p->pReal, flags);
}

/*
** Return the current file-size of an devsym-file.
*/
ساکن صحیح devsym_File_سائز(sqlite3_file *pFile, sqlite_int64 *p_سائز){
  devsym_file *p = (devsym_file *)pFile;
  واپس sqlite3_Os_File_سائز(p->pReal, p_سائز);
}

/*
** Lock an devsym-file.
*/
ساکن صحیح devsym_تالا(sqlite3_file *pFile, صحیح e_تالا){
  devsym_file *p = (devsym_file *)pFile;
  واپس sqlite3_Os_تالا(p->pReal, e_تالا);
}

/*
** Unlock an devsym-file.
*/
ساکن صحیح devsymUnlock(sqlite3_file *pFile, صحیح e_تالا){
  devsym_file *p = (devsym_file *)pFile;
  واپس sqlite3OsUnlock(p->pReal, e_تالا);
}

/*
** Check if another file-handle holds a RESERVED lock on an devsym-file.
*/
ساکن صحیح devsym_Check_Reserved_تالا(sqlite3_file *pFile, صحیح *p_Res_برآمد){
  devsym_file *p = (devsym_file *)pFile;
  واپس sqlite3_Os_Check_Reserved_تالا(p->pReal, p_Res_برآمد);
}

/*
** File control method. For custom operations on an devsym-file.
*/
ساکن صحیح devsymFileControl(sqlite3_file *pFile, صحیح op, خالی *pArg){
  devsym_file *p = (devsym_file *)pFile;
  واپس sqlite3OsFileControl(p->pReal, op, pArg);
}

/*
** Return the sector-size in bytes for an devsym-file.
*/
ساکن صحیح devsym_Sector_سائز(sqlite3_file *pFile){
  واپس g.i_Sector_سائز;
}

/*
** Return the device characteristic flags supported by an devsym-file.
*/
ساکن صحیح devsymDeviceCharacteristics(sqlite3_file *pFile){
  واپس g.iDeviceChar;
}

/*
** Shared-memory methods are all pass-throughs.
*/
ساکن صحیح devsym_Shm_تالا(sqlite3_file *pFile, صحیح ofst, صحیح n, صحیح flags){
  devsym_file *p = (devsym_file *)pFile;
  واپس p->pReal->pMethods->x_Shm_تالا(p->pReal, ofst, n, flags);
}
ساکن صحیح devsymShmMap(
  sqlite3_file *pFile, 
  صحیح iRegion, 
  صحیح szRegion, 
  صحیح isWrite, 
  خالی متلون **pp
){
  devsym_file *p = (devsym_file *)pFile;
  واپس p->pReal->pMethods->xShmMap(p->pReal, iRegion, szRegion, isWrite, pp);
}
ساکن خالی devsymShmBarrier(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  p->pReal->pMethods->xShmBarrier(p->pReal);
}
ساکن صحیح devsymShmUnmap(sqlite3_file *pFile, صحیح delFlag){
  devsym_file *p = (devsym_file *)pFile;
  واپس p->pReal->pMethods->xShmUnmap(p->pReal, delFlag);
}



/*
** Open an devsym file handle.
*/
ساکن صحیح devsymOpen(
  sqlite3_vfs *pVfs,
  ثابت حرف *zName,
  sqlite3_file *pFile,
  صحیح flags,
  صحیح *p_برآمد_Flags
){
ساکن sqlite3_io_methods devsym_io_methods = {
  2,                                /* iVersion */
  devsymClose,                      /* xClose */
  devsymRead,                       /* xRead */
  devsymWrite,                      /* xWrite */
  devsymTruncate,                   /* xTruncate */
  devsymSync,                       /* xSync */
  devsym_File_سائز,                   /* xFileSize */
  devsym_تالا,                       /* xLock */
  devsymUnlock,                     /* xUnlock */
  devsym_Check_Reserved_تالا,          /* xCheckReservedLock */
  devsymFileControl,                /* xFileControl */
  devsym_Sector_سائز,                 /* xSectorSize */
  devsymDeviceCharacteristics,      /* xDeviceCharacteristics */
  devsymShmMap,                     /* xShmMap */
  devsym_Shm_تالا,                    /* xShmLock */
  devsymShmBarrier,                 /* xShmBarrier */
  devsymShmUnmap                    /* xShmUnmap */
};

  صحیح rc;
  devsym_file *p = (devsym_file *)pFile;
  p->pReal = (sqlite3_file *)&p[1];
  rc = sqlite3OsOpen(g.pVfs, zName, p->pReal, flags, p_برآمد_Flags);
  اگر( p->pReal->pMethods ){
    pFile->pMethods = &devsym_io_methods;
  }
  واپس rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
ساکن صحیح devsymDelete(sqlite3_vfs *pVfs, ثابت حرف *z_راستہ, صحیح dirSync){
  واپس sqlite3OsDelete(g.pVfs, z_راستہ, dirSync);
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
ساکن صحیح devsymAccess(
  sqlite3_vfs *pVfs, 
  ثابت حرف *z_راستہ, 
  صحیح flags, 
  صحیح *p_Res_برآمد
){
  واپس sqlite3OsAccess(g.pVfs, z_راستہ, flags, p_Res_برآمد);
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (DEVSYM_MAX_PATHNAME+1) bytes.
*/
ساکن صحیح devsym_بھرا_Pathname(
  sqlite3_vfs *pVfs, 
  ثابت حرف *z_راستہ, 
  صحیح n_برآمد, 
  حرف *z_برآمد
){
  واپس sqlite3_Os_بھرا_Pathname(g.pVfs, z_راستہ, n_برآمد, z_برآمد);
}

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_بھرو_EXTENSION
/*
** Open the dynamic library located at zPath and return a handle.
*/
ساکن خالی *devsymDlOpen(sqlite3_vfs *pVfs, ثابت حرف *z_راستہ){
  واپس sqlite3OsDlOpen(g.pVfs, z_راستہ);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
ساکن خالی devsymDlError(sqlite3_vfs *pVfs, صحیح nByte, حرف *z_Err_پیغام){
  sqlite3OsDlError(g.pVfs, nByte, z_Err_پیغام);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
ساکن خالی (*devsymDlSym(sqlite3_vfs *pVfs, خالی *p, ثابت حرف *zSym))(خالی){
  واپس sqlite3OsDlSym(g.pVfs, p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
ساکن خالی devsymDlClose(sqlite3_vfs *pVfs, خالی *pHandle){
  sqlite3OsDlClose(g.pVfs, pHandle);
}
#اگر_ختم /* SQLITE_OMIT_LOAD_EXTENSION */

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
ساکن صحیح devsymRandomness(sqlite3_vfs *pVfs, صحیح nByte, حرف *z_Buf_برآمد){
  واپس sqlite3OsRandomness(g.pVfs, nByte, z_Buf_برآمد);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
ساکن صحیح devsymSleep(sqlite3_vfs *pVfs, صحیح nMicro){
  واپس sqlite3OsSleep(g.pVfs, nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
ساکن صحیح devsym_Current_وقت(sqlite3_vfs *pVfs, دوہرا *p_وقت_برآمد){
  واپس g.pVfs->x_Current_وقت(g.pVfs, p_وقت_برآمد);
}

/*
** Return the sector-size in bytes for an writecrash-file.
*/
ساکن صحیح writecrash_Sector_سائز(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  واپس sqlite3_Os_Sector_سائز(p->pReal);
}

/*
** Return the device characteristic flags supported by an writecrash-file.
*/
ساکن صحیح writecrashDeviceCharacteristics(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  واپس sqlite3OsDeviceCharacteristics(p->pReal);
}

/*
** Write data to an writecrash-file.
*/
ساکن صحیح writecrashWrite(
  sqlite3_file *pFile, 
  ثابت خالی *zBuf, 
  صحیح iAmt, 
  sqlite_int64 iOfst
){
  devsym_file *p = (devsym_file *)pFile;
  اگر( g.nWriteCrash>0 ){
    g.nWriteCrash--;
    اگر( g.nWriteCrash==0 ) abort();
  }
  واپس sqlite3OsWrite(p->pReal, zBuf, iAmt, iOfst);
}

/*
** Open an writecrash file handle.
*/
ساکن صحیح writecrashOpen(
  sqlite3_vfs *pVfs,
  ثابت حرف *zName,
  sqlite3_file *pFile,
  صحیح flags,
  صحیح *p_برآمد_Flags
){
ساکن sqlite3_io_methods writecrash_io_methods = {
  2,                                /* iVersion */
  devsymClose,                      /* xClose */
  devsymRead,                       /* xRead */
  writecrashWrite,                  /* xWrite */
  devsymTruncate,                   /* xTruncate */
  devsymSync,                       /* xSync */
  devsym_File_سائز,                   /* xFileSize */
  devsym_تالا,                       /* xLock */
  devsymUnlock,                     /* xUnlock */
  devsym_Check_Reserved_تالا,          /* xCheckReservedLock */
  devsymFileControl,                /* xFileControl */
  writecrash_Sector_سائز,             /* xSectorSize */
  writecrashDeviceCharacteristics,  /* xDeviceCharacteristics */
  devsymShmMap,                     /* xShmMap */
  devsym_Shm_تالا,                    /* xShmLock */
  devsymShmBarrier,                 /* xShmBarrier */
  devsymShmUnmap                    /* xShmUnmap */
};

  صحیح rc;
  devsym_file *p = (devsym_file *)pFile;
  p->pReal = (sqlite3_file *)&p[1];
  rc = sqlite3OsOpen(g.pVfs, zName, p->pReal, flags, p_برآمد_Flags);
  اگر( p->pReal->pMethods ){
    pFile->pMethods = &writecrash_io_methods;
  }
  واپس rc;
}

ساکن sqlite3_vfs devsym_vfs = {
  2,                     /* iVersion */
  حجم(devsym_file),      /* szOsFile */
  DEVSYM_MAX_PATHNAME,      /* mxPathname */
  0,                     /* pNext */
  DEVSYM_VFS_NAME,          /* zName */
  0,                     /* pAppData */
  devsymOpen,               /* xOpen */
  devsymDelete,             /* xDelete */
  devsymAccess,             /* xAccess */
  devsym_بھرا_Pathname,       /* xFullPathname */
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_بھرو_EXTENSION
  devsymDlOpen,             /* xDlOpen */
  devsymDlError,            /* xDlError */
  devsymDlSym,              /* xDlSym */
  devsymDlClose,            /* xDlClose */
#ورنہ
  0,                        /* xDlOpen */
  0,                        /* xDlError */
  0,                        /* xDlSym */
  0,                        /* xDlClose */
#اگر_ختم /* SQLITE_OMIT_LOAD_EXTENSION */
  devsymRandomness,         /* xRandomness */
  devsymSleep,              /* xSleep */
  devsym_Current_وقت,        /* xCurrentTime */
  0,                        /* xGetLastError */
  0                         /* xCurrentTimeInt64 */
};

ساکن sqlite3_vfs writecrash_vfs = {
  2,                     /* iVersion */
  حجم(devsym_file),      /* szOsFile */
  DEVSYM_MAX_PATHNAME,      /* mxPathname */
  0,                     /* pNext */
  WRITECRASH_NAME,          /* zName */
  0,                     /* pAppData */
  writecrashOpen,           /* xOpen */
  devsymDelete,             /* xDelete */
  devsymAccess,             /* xAccess */
  devsym_بھرا_Pathname,       /* xFullPathname */
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_بھرو_EXTENSION
  devsymDlOpen,             /* xDlOpen */
  devsymDlError,            /* xDlError */
  devsymDlSym,              /* xDlSym */
  devsymDlClose,            /* xDlClose */
#ورنہ
  0,                        /* xDlOpen */
  0,                        /* xDlError */
  0,                        /* xDlSym */
  0,                        /* xDlClose */
#اگر_ختم /* SQLITE_OMIT_LOAD_EXTENSION */
  devsymRandomness,         /* xRandomness */
  devsymSleep,              /* xSleep */
  devsym_Current_وقت,        /* xCurrentTime */
  0,                        /* xGetLastError */
  0                         /* xCurrentTimeInt64 */
};


/*
** This procedure registers the devsym vfs with SQLite. If the argument is
** true, the devsym vfs becomes the new default vfs. It is the only publicly
** available function in this file.
*/
خالی devsym_رجسٹر(صحیح iDeviceChar, صحیح i_Sector_سائز){

  اگر( g.pVfs==0 ){
    g.pVfs = sqlite3_vfs_ڈھونڈو(0);
    devsym_vfs.szOsFile += g.pVfs->szOsFile;
    writecrash_vfs.szOsFile += g.pVfs->szOsFile;
    sqlite3_vfs_رجسٹر(&devsym_vfs, 0);
    sqlite3_vfs_رجسٹر(&writecrash_vfs, 0);
  }
  اگر( iDeviceChar>=0 ){
    g.iDeviceChar = iDeviceChar;
  }ورنہ{
    g.iDeviceChar = 0;
  }
  اگر( i_Sector_سائز>=0 ){
    g.i_Sector_سائز = i_Sector_سائز;
  }ورنہ{
    g.i_Sector_سائز = 512;
  }
}

خالی devsym_unregister(){
  sqlite3_vfs_unregister(&devsym_vfs);
  sqlite3_vfs_unregister(&writecrash_vfs);
  g.pVfs = 0;
  g.iDeviceChar = 0;
  g.i_Sector_سائز = 0;
}

خالی devsym_crash_on_write(صحیح nWrite){
  اگر( g.pVfs==0 ){
    g.pVfs = sqlite3_vfs_ڈھونڈو(0);
    devsym_vfs.szOsFile += g.pVfs->szOsFile;
    writecrash_vfs.szOsFile += g.pVfs->szOsFile;
    sqlite3_vfs_رجسٹر(&devsym_vfs, 0);
    sqlite3_vfs_رجسٹر(&writecrash_vfs, 0);
  }
  g.nWriteCrash = nWrite;
}

#اگر_ختم
