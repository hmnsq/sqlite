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
#အကယ်၍ SQLITE_TEST          /* This file is used for testing only */

#include "sqlite3.h"
#include "sqliteInt.h"

/*
** Maximum pathname length supported by the devsym backend.
*/
#define DEVSYM_MAX_PATHNAME 512

/*
** Name used to identify this VFS.
*/
#define DEVSYM_VFS_NAME "devsym"
#define WRITECRASH_NAME "writecrash"

typedef sa_nracanaa devsym_file devsym_file;
sa_nracanaa devsym_file {
  sqlite3_file base;
  sqlite3_file *pReal;
};

/*
** Method declarations for devsym_file.
*/
static ကိန်းပြည့် devsymClose(sqlite3_file*);
static ကိန်းပြည့် devsymRead(sqlite3_file*, void*, ကိန်းပြည့် iAmt, sqlite3_int64 iOfst);
static ကိန်းပြည့် devsymWrite(sqlite3_file*,const void*,ကိန်းပြည့် iAmt, sqlite3_int64 iOfst);
static ကိန်းပြည့် devsymTruncate(sqlite3_file*, sqlite3_int64 size);
static ကိန်းပြည့် devsymSync(sqlite3_file*, ကိန်းပြည့် flags);
static ကိန်းပြည့် devsymFileSize(sqlite3_file*, sqlite3_int64 *pSize);
static ကိန်းပြည့် devsymLock(sqlite3_file*, ကိန်းပြည့်);
static ကိန်းပြည့် devsymUnlock(sqlite3_file*, ကိန်းပြည့်);
static ကိန်းပြည့် devsymCheckReservedLock(sqlite3_file*, ကိန်းပြည့် *);
static ကိန်းပြည့် devsymFileControl(sqlite3_file*, ကိန်းပြည့် op, void *pArg);
static ကိန်းပြည့် devsymSectorSize(sqlite3_file*);
static ကိန်းပြည့် devsymDeviceCharacteristics(sqlite3_file*);
static ကိန်းပြည့် devsymShmLock(sqlite3_file*,ကိန်းပြည့်,ကိန်းပြည့်,ကိန်းပြည့်);
static ကိန်းပြည့် devsymShmMap(sqlite3_file*,ကိန်းပြည့်,ကိန်းပြည့်,ကိန်းပြည့်, void ca_ncala **);
static void devsymShmBarrier(sqlite3_file*);
static ကိန်းပြည့် devsymShmUnmap(sqlite3_file*,ကိန်းပြည့်);

/*
** Method declarations for devsym_vfs.
*/
static ကိန်းပြည့် devsymOpen(sqlite3_vfs*, const var_na *, sqlite3_file*, ကိန်းပြည့် , ကိန်းပြည့် *);
static ကိန်းပြည့် devsymDelete(sqlite3_vfs*, const var_na *zName, ကိန်းပြည့် syncDir);
static ကိန်းပြည့် devsymAccess(sqlite3_vfs*, const var_na *zName, ကိန်းပြည့် flags, ကိန်းပြည့် *);
static ကိန်းပြည့် devsymFullPathname(sqlite3_vfs*, const var_na *zName, ကိန်းပြည့်, var_na *zOut);
#ifndef SQLITE_OMIT_LOAD_EXTENSION
static void *devsymDlOpen(sqlite3_vfs*, const var_na *zFilename);
static void devsymDlError(sqlite3_vfs*, ကိန်းပြည့် nByte, var_na *zErrMsg);
static void (*devsymDlSym(sqlite3_vfs*,void*, const var_na *zSymbol))(void);
static void devsymDlClose(sqlite3_vfs*, void*);
#endif /* SQLITE_OMIT_LOAD_EXTENSION */
static ကိန်းပြည့် devsymRandomness(sqlite3_vfs*, ကိန်းပြည့် nByte, var_na *zOut);
static ကိန်းပြည့် devsymSleep(sqlite3_vfs*, ကိန်းပြည့် microseconds);
static ကိန်းပြည့် devsymCurrentTime(sqlite3_vfs*, dviguu_na*);

sa_nracanaa DevsymGlobal {
  sqlite3_vfs *pVfs;
  ကိန်းပြည့် iDeviceChar;
  ကိန်းပြည့် iSectorSize;
  ကိန်းပြည့် nWriteCrash;
};
sa_nracanaa DevsymGlobal g = {0, 0, 512, 0};

/*
** Close an devsym-file.
*/
static ကိန်းပြည့် devsymClose(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  sqlite3OsClose(p->pReal);
  ပြန်ပေး SQLITE_OK;
}

/*
** Read data from an devsym-file.
*/
static ကိန်းပြည့် devsymRead(
  sqlite3_file *pFile, 
  void *zBuf, 
  ကိန်းပြည့် iAmt, 
  sqlite_int64 iOfst
){
  devsym_file *p = (devsym_file *)pFile;
  ပြန်ပေး sqlite3OsRead(p->pReal, zBuf, iAmt, iOfst);
}

/*
** Write data to an devsym-file.
*/
static ကိန်းပြည့် devsymWrite(
  sqlite3_file *pFile, 
  const void *zBuf, 
  ကိန်းပြည့် iAmt, 
  sqlite_int64 iOfst
){
  devsym_file *p = (devsym_file *)pFile;
  ပြန်ပေး sqlite3OsWrite(p->pReal, zBuf, iAmt, iOfst);
}

/*
** Truncate an devsym-file.
*/
static ကိန်းပြည့် devsymTruncate(sqlite3_file *pFile, sqlite_int64 size){
  devsym_file *p = (devsym_file *)pFile;
  ပြန်ပေး sqlite3OsTruncate(p->pReal, size);
}

/*
** Sync an devsym-file.
*/
static ကိန်းပြည့် devsymSync(sqlite3_file *pFile, ကိန်းပြည့် flags){
  devsym_file *p = (devsym_file *)pFile;
  ပြန်ပေး sqlite3OsSync(p->pReal, flags);
}

/*
** Return the current file-size of an devsym-file.
*/
static ကိန်းပြည့် devsymFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  devsym_file *p = (devsym_file *)pFile;
  ပြန်ပေး sqlite3OsFileSize(p->pReal, pSize);
}

/*
** Lock an devsym-file.
*/
static ကိန်းပြည့် devsymLock(sqlite3_file *pFile, ကိန်းပြည့် eLock){
  devsym_file *p = (devsym_file *)pFile;
  ပြန်ပေး sqlite3OsLock(p->pReal, eLock);
}

/*
** Unlock an devsym-file.
*/
static ကိန်းပြည့် devsymUnlock(sqlite3_file *pFile, ကိန်းပြည့် eLock){
  devsym_file *p = (devsym_file *)pFile;
  ပြန်ပေး sqlite3OsUnlock(p->pReal, eLock);
}

/*
** Check if another file-handle holds a RESERVED lock on an devsym-file.
*/
static ကိန်းပြည့် devsymCheckReservedLock(sqlite3_file *pFile, ကိန်းပြည့် *pResOut){
  devsym_file *p = (devsym_file *)pFile;
  ပြန်ပေး sqlite3OsCheckReservedLock(p->pReal, pResOut);
}

/*
** File control method. For custom operations on an devsym-file.
*/
static ကိန်းပြည့် devsymFileControl(sqlite3_file *pFile, ကိန်းပြည့် op, void *pArg){
  devsym_file *p = (devsym_file *)pFile;
  ပြန်ပေး sqlite3OsFileControl(p->pReal, op, pArg);
}

/*
** Return the sector-size in bytes for an devsym-file.
*/
static ကိန်းပြည့် devsymSectorSize(sqlite3_file *pFile){
  ပြန်ပေး g.iSectorSize;
}

/*
** Return the device characteristic flags supported by an devsym-file.
*/
static ကိန်းပြည့် devsymDeviceCharacteristics(sqlite3_file *pFile){
  ပြန်ပေး g.iDeviceChar;
}

/*
** Shared-memory methods are all pass-throughs.
*/
static ကိန်းပြည့် devsymShmLock(sqlite3_file *pFile, ကိန်းပြည့် ofst, ကိန်းပြည့် n, ကိန်းပြည့် flags){
  devsym_file *p = (devsym_file *)pFile;
  ပြန်ပေး p->pReal->pMethods->xShmLock(p->pReal, ofst, n, flags);
}
static ကိန်းပြည့် devsymShmMap(
  sqlite3_file *pFile, 
  ကိန်းပြည့် iRegion, 
  ကိန်းပြည့် szRegion, 
  ကိန်းပြည့် isWrite, 
  void ca_ncala **pp
){
  devsym_file *p = (devsym_file *)pFile;
  ပြန်ပေး p->pReal->pMethods->xShmMap(p->pReal, iRegion, szRegion, isWrite, pp);
}
static void devsymShmBarrier(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  p->pReal->pMethods->xShmBarrier(p->pReal);
}
static ကိန်းပြည့် devsymShmUnmap(sqlite3_file *pFile, ကိန်းပြည့် delFlag){
  devsym_file *p = (devsym_file *)pFile;
  ပြန်ပေး p->pReal->pMethods->xShmUnmap(p->pReal, delFlag);
}



/*
** Open an devsym file handle.
*/
static ကိန်းပြည့် devsymOpen(
  sqlite3_vfs *pVfs,
  const var_na *zName,
  sqlite3_file *pFile,
  ကိန်းပြည့် flags,
  ကိန်းပြည့် *pOutFlags
){
static sqlite3_io_methods devsym_io_methods = {
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

  ကိန်းပြည့် rc;
  devsym_file *p = (devsym_file *)pFile;
  p->pReal = (sqlite3_file *)&p[1];
  rc = sqlite3OsOpen(g.pVfs, zName, p->pReal, flags, pOutFlags);
  အကယ်၍( p->pReal->pMethods ){
    pFile->pMethods = &devsym_io_methods;
  }
  ပြန်ပေး rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
static ကိန်းပြည့် devsymDelete(sqlite3_vfs *pVfs, const var_na *zPath, ကိန်းပြည့် dirSync){
  ပြန်ပေး sqlite3OsDelete(g.pVfs, zPath, dirSync);
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
static ကိန်းပြည့် devsymAccess(
  sqlite3_vfs *pVfs, 
  const var_na *zPath, 
  ကိန်းပြည့် flags, 
  ကိန်းပြည့် *pResOut
){
  ပြန်ပေး sqlite3OsAccess(g.pVfs, zPath, flags, pResOut);
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (DEVSYM_MAX_PATHNAME+1) bytes.
*/
static ကိန်းပြည့် devsymFullPathname(
  sqlite3_vfs *pVfs, 
  const var_na *zPath, 
  ကိန်းပြည့် nOut, 
  var_na *zOut
){
  ပြန်ပေး sqlite3OsFullPathname(g.pVfs, zPath, nOut, zOut);
}

#ifndef SQLITE_OMIT_LOAD_EXTENSION
/*
** Open the dynamic library located at zPath and return a handle.
*/
static void *devsymDlOpen(sqlite3_vfs *pVfs, const var_na *zPath){
  ပြန်ပေး sqlite3OsDlOpen(g.pVfs, zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
static void devsymDlError(sqlite3_vfs *pVfs, ကိန်းပြည့် nByte, var_na *zErrMsg){
  sqlite3OsDlError(g.pVfs, nByte, zErrMsg);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
static void (*devsymDlSym(sqlite3_vfs *pVfs, void *p, const var_na *zSym))(void){
  ပြန်ပေး sqlite3OsDlSym(g.pVfs, p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
static void devsymDlClose(sqlite3_vfs *pVfs, void *pHandle){
  sqlite3OsDlClose(g.pVfs, pHandle);
}
#endif /* SQLITE_OMIT_LOAD_EXTENSION */

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
static ကိန်းပြည့် devsymRandomness(sqlite3_vfs *pVfs, ကိန်းပြည့် nByte, var_na *zBufOut){
  ပြန်ပေး sqlite3OsRandomness(g.pVfs, nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
static ကိန်းပြည့် devsymSleep(sqlite3_vfs *pVfs, ကိန်းပြည့် nMicro){
  ပြန်ပေး sqlite3OsSleep(g.pVfs, nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
static ကိန်းပြည့် devsymCurrentTime(sqlite3_vfs *pVfs, dviguu_na *pTimeOut){
  ပြန်ပေး g.pVfs->xCurrentTime(g.pVfs, pTimeOut);
}

/*
** Return the sector-size in bytes for an writecrash-file.
*/
static ကိန်းပြည့် writecrashSectorSize(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  ပြန်ပေး sqlite3OsSectorSize(p->pReal);
}

/*
** Return the device characteristic flags supported by an writecrash-file.
*/
static ကိန်းပြည့် writecrashDeviceCharacteristics(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  ပြန်ပေး sqlite3OsDeviceCharacteristics(p->pReal);
}

/*
** Write data to an writecrash-file.
*/
static ကိန်းပြည့် writecrashWrite(
  sqlite3_file *pFile, 
  const void *zBuf, 
  ကိန်းပြည့် iAmt, 
  sqlite_int64 iOfst
){
  devsym_file *p = (devsym_file *)pFile;
  အကယ်၍( g.nWriteCrash>0 ){
    g.nWriteCrash--;
    အကယ်၍( g.nWriteCrash==0 ) abort();
  }
  ပြန်ပေး sqlite3OsWrite(p->pReal, zBuf, iAmt, iOfst);
}

/*
** Open an writecrash file handle.
*/
static ကိန်းပြည့် writecrashOpen(
  sqlite3_vfs *pVfs,
  const var_na *zName,
  sqlite3_file *pFile,
  ကိန်းပြည့် flags,
  ကိန်းပြည့် *pOutFlags
){
static sqlite3_io_methods writecrash_io_methods = {
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

  ကိန်းပြည့် rc;
  devsym_file *p = (devsym_file *)pFile;
  p->pReal = (sqlite3_file *)&p[1];
  rc = sqlite3OsOpen(g.pVfs, zName, p->pReal, flags, pOutFlags);
  အကယ်၍( p->pReal->pMethods ){
    pFile->pMethods = &writecrash_io_methods;
  }
  ပြန်ပေး rc;
}

static sqlite3_vfs devsym_vfs = {
  2,                     /* iVersion */
  parimaa_na(devsym_file),      /* szOsFile */
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
#မဟုတ်ပါက
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

static sqlite3_vfs writecrash_vfs = {
  2,                     /* iVersion */
  parimaa_na(devsym_file),      /* szOsFile */
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
#မဟုတ်ပါက
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
void devsym_register(ကိန်းပြည့် iDeviceChar, ကိန်းပြည့် iSectorSize){

  အကယ်၍( g.pVfs==0 ){
    g.pVfs = sqlite3_vfs_find(0);
    devsym_vfs.szOsFile += g.pVfs->szOsFile;
    writecrash_vfs.szOsFile += g.pVfs->szOsFile;
    sqlite3_vfs_register(&devsym_vfs, 0);
    sqlite3_vfs_register(&writecrash_vfs, 0);
  }
  အကယ်၍( iDeviceChar>=0 ){
    g.iDeviceChar = iDeviceChar;
  }မဟုတ်ပါက{
    g.iDeviceChar = 0;
  }
  အကယ်၍( iSectorSize>=0 ){
    g.iSectorSize = iSectorSize;
  }မဟုတ်ပါက{
    g.iSectorSize = 512;
  }
}

void devsym_unregister(){
  sqlite3_vfs_unregister(&devsym_vfs);
  sqlite3_vfs_unregister(&writecrash_vfs);
  g.pVfs = 0;
  g.iDeviceChar = 0;
  g.iSectorSize = 0;
}

void devsym_crash_on_write(ကိန်းပြည့် nWrite){
  အကယ်၍( g.pVfs==0 ){
    g.pVfs = sqlite3_vfs_find(0);
    devsym_vfs.szOsFile += g.pVfs->szOsFile;
    writecrash_vfs.szOsFile += g.pVfs->szOsFile;
    sqlite3_vfs_register(&devsym_vfs, 0);
    sqlite3_vfs_register(&writecrash_vfs, 0);
  }
  g.nWriteCrash = nWrite;
}

#endif
