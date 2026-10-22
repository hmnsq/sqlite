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
#如果 SQLITE_TEST          /* This file is used for testing only */

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

typedef struct devsym_file devsym_file;
struct devsym_file {
  sqlite3_file base;
  sqlite3_file *pReal;
};

/*
** Method declarations for devsym_file.
*/
static 整数 devsymClose(sqlite3_file*);
static 整数 devsymRead(sqlite3_file*, void*, 整数 iAmt, sqlite3_int64 iOfst);
static 整数 devsymWrite(sqlite3_file*,const void*,整数 iAmt, sqlite3_int64 iOfst);
static 整数 devsymTruncate(sqlite3_file*, sqlite3_int64 size);
static 整数 devsymSync(sqlite3_file*, 整数 flags);
static 整数 devsymFileSize(sqlite3_file*, sqlite3_int64 *pSize);
static 整数 devsymLock(sqlite3_file*, 整数);
static 整数 devsymUnlock(sqlite3_file*, 整数);
static 整数 devsymCheckReservedLock(sqlite3_file*, 整数 *);
static 整数 devsymFileControl(sqlite3_file*, 整数 op, void *pArg);
static 整数 devsymSectorSize(sqlite3_file*);
static 整数 devsymDeviceCharacteristics(sqlite3_file*);
static 整数 devsymShmLock(sqlite3_file*,整数,整数,整数);
static 整数 devsymShmMap(sqlite3_file*,整数,整数,整数, void volatile **);
static void devsymShmBarrier(sqlite3_file*);
static 整数 devsymShmUnmap(sqlite3_file*,整数);

/*
** Method declarations for devsym_vfs.
*/
static 整数 devsymOpen(sqlite3_vfs*, const char *, sqlite3_file*, 整数 , 整数 *);
static 整数 devsymDelete(sqlite3_vfs*, const char *zName, 整数 syncDir);
static 整数 devsymAccess(sqlite3_vfs*, const char *zName, 整数 flags, 整数 *);
static 整数 devsymFullPathname(sqlite3_vfs*, const char *zName, 整数, char *zOut);
#ifndef SQLITE_OMIT_LOAD_EXTENSION
static void *devsymDlOpen(sqlite3_vfs*, const char *zFilename);
static void devsymDlError(sqlite3_vfs*, 整数 nByte, char *zErrMsg);
static void (*devsymDlSym(sqlite3_vfs*,void*, const char *zSymbol))(void);
static void devsymDlClose(sqlite3_vfs*, void*);
#endif /* SQLITE_OMIT_LOAD_EXTENSION */
static 整数 devsymRandomness(sqlite3_vfs*, 整数 nByte, char *zOut);
static 整数 devsymSleep(sqlite3_vfs*, 整数 microseconds);
static 整数 devsymCurrentTime(sqlite3_vfs*, double*);

struct DevsymGlobal {
  sqlite3_vfs *pVfs;
  整数 iDeviceChar;
  整数 iSectorSize;
  整数 nWriteCrash;
};
struct DevsymGlobal g = {0, 0, 512, 0};

/*
** Close an devsym-file.
*/
static 整数 devsymClose(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  sqlite3OsClose(p->pReal);
  返回 SQLITE_OK;
}

/*
** Read data from an devsym-file.
*/
static 整数 devsymRead(
  sqlite3_file *pFile, 
  void *zBuf, 
  整数 iAmt, 
  sqlite_int64 iOfst
){
  devsym_file *p = (devsym_file *)pFile;
  返回 sqlite3OsRead(p->pReal, zBuf, iAmt, iOfst);
}

/*
** Write data to an devsym-file.
*/
static 整数 devsymWrite(
  sqlite3_file *pFile, 
  const void *zBuf, 
  整数 iAmt, 
  sqlite_int64 iOfst
){
  devsym_file *p = (devsym_file *)pFile;
  返回 sqlite3OsWrite(p->pReal, zBuf, iAmt, iOfst);
}

/*
** Truncate an devsym-file.
*/
static 整数 devsymTruncate(sqlite3_file *pFile, sqlite_int64 size){
  devsym_file *p = (devsym_file *)pFile;
  返回 sqlite3OsTruncate(p->pReal, size);
}

/*
** Sync an devsym-file.
*/
static 整数 devsymSync(sqlite3_file *pFile, 整数 flags){
  devsym_file *p = (devsym_file *)pFile;
  返回 sqlite3OsSync(p->pReal, flags);
}

/*
** Return the current file-size of an devsym-file.
*/
static 整数 devsymFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  devsym_file *p = (devsym_file *)pFile;
  返回 sqlite3OsFileSize(p->pReal, pSize);
}

/*
** Lock an devsym-file.
*/
static 整数 devsymLock(sqlite3_file *pFile, 整数 eLock){
  devsym_file *p = (devsym_file *)pFile;
  返回 sqlite3OsLock(p->pReal, eLock);
}

/*
** Unlock an devsym-file.
*/
static 整数 devsymUnlock(sqlite3_file *pFile, 整数 eLock){
  devsym_file *p = (devsym_file *)pFile;
  返回 sqlite3OsUnlock(p->pReal, eLock);
}

/*
** Check if another file-handle holds a RESERVED lock on an devsym-file.
*/
static 整数 devsymCheckReservedLock(sqlite3_file *pFile, 整数 *pResOut){
  devsym_file *p = (devsym_file *)pFile;
  返回 sqlite3OsCheckReservedLock(p->pReal, pResOut);
}

/*
** File control method. For custom operations on an devsym-file.
*/
static 整数 devsymFileControl(sqlite3_file *pFile, 整数 op, void *pArg){
  devsym_file *p = (devsym_file *)pFile;
  返回 sqlite3OsFileControl(p->pReal, op, pArg);
}

/*
** Return the sector-size in bytes for an devsym-file.
*/
static 整数 devsymSectorSize(sqlite3_file *pFile){
  返回 g.iSectorSize;
}

/*
** Return the device characteristic flags supported by an devsym-file.
*/
static 整数 devsymDeviceCharacteristics(sqlite3_file *pFile){
  返回 g.iDeviceChar;
}

/*
** Shared-memory methods are all pass-throughs.
*/
static 整数 devsymShmLock(sqlite3_file *pFile, 整数 ofst, 整数 n, 整数 flags){
  devsym_file *p = (devsym_file *)pFile;
  返回 p->pReal->pMethods->xShmLock(p->pReal, ofst, n, flags);
}
static 整数 devsymShmMap(
  sqlite3_file *pFile, 
  整数 iRegion, 
  整数 szRegion, 
  整数 isWrite, 
  void volatile **pp
){
  devsym_file *p = (devsym_file *)pFile;
  返回 p->pReal->pMethods->xShmMap(p->pReal, iRegion, szRegion, isWrite, pp);
}
static void devsymShmBarrier(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  p->pReal->pMethods->xShmBarrier(p->pReal);
}
static 整数 devsymShmUnmap(sqlite3_file *pFile, 整数 delFlag){
  devsym_file *p = (devsym_file *)pFile;
  返回 p->pReal->pMethods->xShmUnmap(p->pReal, delFlag);
}



/*
** Open an devsym file handle.
*/
static 整数 devsymOpen(
  sqlite3_vfs *pVfs,
  const char *zName,
  sqlite3_file *pFile,
  整数 flags,
  整数 *pOutFlags
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

  整数 rc;
  devsym_file *p = (devsym_file *)pFile;
  p->pReal = (sqlite3_file *)&p[1];
  rc = sqlite3OsOpen(g.pVfs, zName, p->pReal, flags, pOutFlags);
  如果( p->pReal->pMethods ){
    pFile->pMethods = &devsym_io_methods;
  }
  返回 rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
static 整数 devsymDelete(sqlite3_vfs *pVfs, const char *zPath, 整数 dirSync){
  返回 sqlite3OsDelete(g.pVfs, zPath, dirSync);
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
static 整数 devsymAccess(
  sqlite3_vfs *pVfs, 
  const char *zPath, 
  整数 flags, 
  整数 *pResOut
){
  返回 sqlite3OsAccess(g.pVfs, zPath, flags, pResOut);
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (DEVSYM_MAX_PATHNAME+1) bytes.
*/
static 整数 devsymFullPathname(
  sqlite3_vfs *pVfs, 
  const char *zPath, 
  整数 nOut, 
  char *zOut
){
  返回 sqlite3OsFullPathname(g.pVfs, zPath, nOut, zOut);
}

#ifndef SQLITE_OMIT_LOAD_EXTENSION
/*
** Open the dynamic library located at zPath and return a handle.
*/
static void *devsymDlOpen(sqlite3_vfs *pVfs, const char *zPath){
  返回 sqlite3OsDlOpen(g.pVfs, zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
static void devsymDlError(sqlite3_vfs *pVfs, 整数 nByte, char *zErrMsg){
  sqlite3OsDlError(g.pVfs, nByte, zErrMsg);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
static void (*devsymDlSym(sqlite3_vfs *pVfs, void *p, const char *zSym))(void){
  返回 sqlite3OsDlSym(g.pVfs, p, zSym);
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
static 整数 devsymRandomness(sqlite3_vfs *pVfs, 整数 nByte, char *zBufOut){
  返回 sqlite3OsRandomness(g.pVfs, nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
static 整数 devsymSleep(sqlite3_vfs *pVfs, 整数 nMicro){
  返回 sqlite3OsSleep(g.pVfs, nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
static 整数 devsymCurrentTime(sqlite3_vfs *pVfs, double *pTimeOut){
  返回 g.pVfs->xCurrentTime(g.pVfs, pTimeOut);
}

/*
** Return the sector-size in bytes for an writecrash-file.
*/
static 整数 writecrashSectorSize(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  返回 sqlite3OsSectorSize(p->pReal);
}

/*
** Return the device characteristic flags supported by an writecrash-file.
*/
static 整数 writecrashDeviceCharacteristics(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  返回 sqlite3OsDeviceCharacteristics(p->pReal);
}

/*
** Write data to an writecrash-file.
*/
static 整数 writecrashWrite(
  sqlite3_file *pFile, 
  const void *zBuf, 
  整数 iAmt, 
  sqlite_int64 iOfst
){
  devsym_file *p = (devsym_file *)pFile;
  如果( g.nWriteCrash>0 ){
    g.nWriteCrash--;
    如果( g.nWriteCrash==0 ) abort();
  }
  返回 sqlite3OsWrite(p->pReal, zBuf, iAmt, iOfst);
}

/*
** Open an writecrash file handle.
*/
static 整数 writecrashOpen(
  sqlite3_vfs *pVfs,
  const char *zName,
  sqlite3_file *pFile,
  整数 flags,
  整数 *pOutFlags
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

  整数 rc;
  devsym_file *p = (devsym_file *)pFile;
  p->pReal = (sqlite3_file *)&p[1];
  rc = sqlite3OsOpen(g.pVfs, zName, p->pReal, flags, pOutFlags);
  如果( p->pReal->pMethods ){
    pFile->pMethods = &writecrash_io_methods;
  }
  返回 rc;
}

static sqlite3_vfs devsym_vfs = {
  2,                     /* iVersion */
  sizeof(devsym_file),      /* szOsFile */
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
#否则
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
  sizeof(devsym_file),      /* szOsFile */
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
#否则
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
void devsym_register(整数 iDeviceChar, 整数 iSectorSize){

  如果( g.pVfs==0 ){
    g.pVfs = sqlite3_vfs_find(0);
    devsym_vfs.szOsFile += g.pVfs->szOsFile;
    writecrash_vfs.szOsFile += g.pVfs->szOsFile;
    sqlite3_vfs_register(&devsym_vfs, 0);
    sqlite3_vfs_register(&writecrash_vfs, 0);
  }
  如果( iDeviceChar>=0 ){
    g.iDeviceChar = iDeviceChar;
  }否则{
    g.iDeviceChar = 0;
  }
  如果( iSectorSize>=0 ){
    g.iSectorSize = iSectorSize;
  }否则{
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

void devsym_crash_on_write(整数 nWrite){
  如果( g.pVfs==0 ){
    g.pVfs = sqlite3_vfs_find(0);
    devsym_vfs.szOsFile += g.pVfs->szOsFile;
    writecrash_vfs.szOsFile += g.pVfs->szOsFile;
    sqlite3_vfs_register(&devsym_vfs, 0);
    sqlite3_vfs_register(&writecrash_vfs, 0);
  }
  g.nWriteCrash = nWrite;
}

#endif
