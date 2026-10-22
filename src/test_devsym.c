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
#если SQLITE_TEST          /* This file is used for testing only */

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
static целое devsymClose(sqlite3_file*);
static целое devsymRead(sqlite3_file*, void*, целое iAmt, sqlite3_int64 iOfst);
static целое devsymWrite(sqlite3_file*,const void*,целое iAmt, sqlite3_int64 iOfst);
static целое devsymTruncate(sqlite3_file*, sqlite3_int64 size);
static целое devsymSync(sqlite3_file*, целое flags);
static целое devsymFileSize(sqlite3_file*, sqlite3_int64 *pSize);
static целое devsymLock(sqlite3_file*, целое);
static целое devsymUnlock(sqlite3_file*, целое);
static целое devsymCheckReservedLock(sqlite3_file*, целое *);
static целое devsymFileControl(sqlite3_file*, целое op, void *pArg);
static целое devsymSectorSize(sqlite3_file*);
static целое devsymDeviceCharacteristics(sqlite3_file*);
static целое devsymShmLock(sqlite3_file*,целое,целое,целое);
static целое devsymShmMap(sqlite3_file*,целое,целое,целое, void volatile **);
static void devsymShmBarrier(sqlite3_file*);
static целое devsymShmUnmap(sqlite3_file*,целое);

/*
** Method declarations for devsym_vfs.
*/
static целое devsymOpen(sqlite3_vfs*, const char *, sqlite3_file*, целое , целое *);
static целое devsymDelete(sqlite3_vfs*, const char *zName, целое syncDir);
static целое devsymAccess(sqlite3_vfs*, const char *zName, целое flags, целое *);
static целое devsymFullPathname(sqlite3_vfs*, const char *zName, целое, char *zOut);
#ifndef SQLITE_OMIT_LOAD_EXTENSION
static void *devsymDlOpen(sqlite3_vfs*, const char *zFilename);
static void devsymDlError(sqlite3_vfs*, целое nByte, char *zErrMsg);
static void (*devsymDlSym(sqlite3_vfs*,void*, const char *zSymbol))(void);
static void devsymDlClose(sqlite3_vfs*, void*);
#endif /* SQLITE_OMIT_LOAD_EXTENSION */
static целое devsymRandomness(sqlite3_vfs*, целое nByte, char *zOut);
static целое devsymSleep(sqlite3_vfs*, целое microseconds);
static целое devsymCurrentTime(sqlite3_vfs*, double*);

struct DevsymGlobal {
  sqlite3_vfs *pVfs;
  целое iDeviceChar;
  целое iSectorSize;
  целое nWriteCrash;
};
struct DevsymGlobal g = {0, 0, 512, 0};

/*
** Close an devsym-file.
*/
static целое devsymClose(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  sqlite3OsClose(p->pReal);
  вернуть SQLITE_OK;
}

/*
** Read data from an devsym-file.
*/
static целое devsymRead(
  sqlite3_file *pFile, 
  void *zBuf, 
  целое iAmt, 
  sqlite_int64 iOfst
){
  devsym_file *p = (devsym_file *)pFile;
  вернуть sqlite3OsRead(p->pReal, zBuf, iAmt, iOfst);
}

/*
** Write data to an devsym-file.
*/
static целое devsymWrite(
  sqlite3_file *pFile, 
  const void *zBuf, 
  целое iAmt, 
  sqlite_int64 iOfst
){
  devsym_file *p = (devsym_file *)pFile;
  вернуть sqlite3OsWrite(p->pReal, zBuf, iAmt, iOfst);
}

/*
** Truncate an devsym-file.
*/
static целое devsymTruncate(sqlite3_file *pFile, sqlite_int64 size){
  devsym_file *p = (devsym_file *)pFile;
  вернуть sqlite3OsTruncate(p->pReal, size);
}

/*
** Sync an devsym-file.
*/
static целое devsymSync(sqlite3_file *pFile, целое flags){
  devsym_file *p = (devsym_file *)pFile;
  вернуть sqlite3OsSync(p->pReal, flags);
}

/*
** Return the current file-size of an devsym-file.
*/
static целое devsymFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  devsym_file *p = (devsym_file *)pFile;
  вернуть sqlite3OsFileSize(p->pReal, pSize);
}

/*
** Lock an devsym-file.
*/
static целое devsymLock(sqlite3_file *pFile, целое eLock){
  devsym_file *p = (devsym_file *)pFile;
  вернуть sqlite3OsLock(p->pReal, eLock);
}

/*
** Unlock an devsym-file.
*/
static целое devsymUnlock(sqlite3_file *pFile, целое eLock){
  devsym_file *p = (devsym_file *)pFile;
  вернуть sqlite3OsUnlock(p->pReal, eLock);
}

/*
** Check if another file-handle holds a RESERVED lock on an devsym-file.
*/
static целое devsymCheckReservedLock(sqlite3_file *pFile, целое *pResOut){
  devsym_file *p = (devsym_file *)pFile;
  вернуть sqlite3OsCheckReservedLock(p->pReal, pResOut);
}

/*
** File control method. For custom operations on an devsym-file.
*/
static целое devsymFileControl(sqlite3_file *pFile, целое op, void *pArg){
  devsym_file *p = (devsym_file *)pFile;
  вернуть sqlite3OsFileControl(p->pReal, op, pArg);
}

/*
** Return the sector-size in bytes for an devsym-file.
*/
static целое devsymSectorSize(sqlite3_file *pFile){
  вернуть g.iSectorSize;
}

/*
** Return the device characteristic flags supported by an devsym-file.
*/
static целое devsymDeviceCharacteristics(sqlite3_file *pFile){
  вернуть g.iDeviceChar;
}

/*
** Shared-memory methods are all pass-throughs.
*/
static целое devsymShmLock(sqlite3_file *pFile, целое ofst, целое n, целое flags){
  devsym_file *p = (devsym_file *)pFile;
  вернуть p->pReal->pMethods->xShmLock(p->pReal, ofst, n, flags);
}
static целое devsymShmMap(
  sqlite3_file *pFile, 
  целое iRegion, 
  целое szRegion, 
  целое isWrite, 
  void volatile **pp
){
  devsym_file *p = (devsym_file *)pFile;
  вернуть p->pReal->pMethods->xShmMap(p->pReal, iRegion, szRegion, isWrite, pp);
}
static void devsymShmBarrier(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  p->pReal->pMethods->xShmBarrier(p->pReal);
}
static целое devsymShmUnmap(sqlite3_file *pFile, целое delFlag){
  devsym_file *p = (devsym_file *)pFile;
  вернуть p->pReal->pMethods->xShmUnmap(p->pReal, delFlag);
}



/*
** Open an devsym file handle.
*/
static целое devsymOpen(
  sqlite3_vfs *pVfs,
  const char *zName,
  sqlite3_file *pFile,
  целое flags,
  целое *pOutFlags
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

  целое rc;
  devsym_file *p = (devsym_file *)pFile;
  p->pReal = (sqlite3_file *)&p[1];
  rc = sqlite3OsOpen(g.pVfs, zName, p->pReal, flags, pOutFlags);
  если( p->pReal->pMethods ){
    pFile->pMethods = &devsym_io_methods;
  }
  вернуть rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
static целое devsymDelete(sqlite3_vfs *pVfs, const char *zPath, целое dirSync){
  вернуть sqlite3OsDelete(g.pVfs, zPath, dirSync);
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
static целое devsymAccess(
  sqlite3_vfs *pVfs, 
  const char *zPath, 
  целое flags, 
  целое *pResOut
){
  вернуть sqlite3OsAccess(g.pVfs, zPath, flags, pResOut);
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (DEVSYM_MAX_PATHNAME+1) bytes.
*/
static целое devsymFullPathname(
  sqlite3_vfs *pVfs, 
  const char *zPath, 
  целое nOut, 
  char *zOut
){
  вернуть sqlite3OsFullPathname(g.pVfs, zPath, nOut, zOut);
}

#ifndef SQLITE_OMIT_LOAD_EXTENSION
/*
** Open the dynamic library located at zPath and return a handle.
*/
static void *devsymDlOpen(sqlite3_vfs *pVfs, const char *zPath){
  вернуть sqlite3OsDlOpen(g.pVfs, zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
static void devsymDlError(sqlite3_vfs *pVfs, целое nByte, char *zErrMsg){
  sqlite3OsDlError(g.pVfs, nByte, zErrMsg);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
static void (*devsymDlSym(sqlite3_vfs *pVfs, void *p, const char *zSym))(void){
  вернуть sqlite3OsDlSym(g.pVfs, p, zSym);
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
static целое devsymRandomness(sqlite3_vfs *pVfs, целое nByte, char *zBufOut){
  вернуть sqlite3OsRandomness(g.pVfs, nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
static целое devsymSleep(sqlite3_vfs *pVfs, целое nMicro){
  вернуть sqlite3OsSleep(g.pVfs, nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
static целое devsymCurrentTime(sqlite3_vfs *pVfs, double *pTimeOut){
  вернуть g.pVfs->xCurrentTime(g.pVfs, pTimeOut);
}

/*
** Return the sector-size in bytes for an writecrash-file.
*/
static целое writecrashSectorSize(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  вернуть sqlite3OsSectorSize(p->pReal);
}

/*
** Return the device characteristic flags supported by an writecrash-file.
*/
static целое writecrashDeviceCharacteristics(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  вернуть sqlite3OsDeviceCharacteristics(p->pReal);
}

/*
** Write data to an writecrash-file.
*/
static целое writecrashWrite(
  sqlite3_file *pFile, 
  const void *zBuf, 
  целое iAmt, 
  sqlite_int64 iOfst
){
  devsym_file *p = (devsym_file *)pFile;
  если( g.nWriteCrash>0 ){
    g.nWriteCrash--;
    если( g.nWriteCrash==0 ) abort();
  }
  вернуть sqlite3OsWrite(p->pReal, zBuf, iAmt, iOfst);
}

/*
** Open an writecrash file handle.
*/
static целое writecrashOpen(
  sqlite3_vfs *pVfs,
  const char *zName,
  sqlite3_file *pFile,
  целое flags,
  целое *pOutFlags
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

  целое rc;
  devsym_file *p = (devsym_file *)pFile;
  p->pReal = (sqlite3_file *)&p[1];
  rc = sqlite3OsOpen(g.pVfs, zName, p->pReal, flags, pOutFlags);
  если( p->pReal->pMethods ){
    pFile->pMethods = &writecrash_io_methods;
  }
  вернуть rc;
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
#иначе
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
#иначе
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
void devsym_register(целое iDeviceChar, целое iSectorSize){

  если( g.pVfs==0 ){
    g.pVfs = sqlite3_vfs_find(0);
    devsym_vfs.szOsFile += g.pVfs->szOsFile;
    writecrash_vfs.szOsFile += g.pVfs->szOsFile;
    sqlite3_vfs_register(&devsym_vfs, 0);
    sqlite3_vfs_register(&writecrash_vfs, 0);
  }
  если( iDeviceChar>=0 ){
    g.iDeviceChar = iDeviceChar;
  }иначе{
    g.iDeviceChar = 0;
  }
  если( iSectorSize>=0 ){
    g.iSectorSize = iSectorSize;
  }иначе{
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

void devsym_crash_on_write(целое nWrite){
  если( g.pVfs==0 ){
    g.pVfs = sqlite3_vfs_find(0);
    devsym_vfs.szOsFile += g.pVfs->szOsFile;
    writecrash_vfs.szOsFile += g.pVfs->szOsFile;
    sqlite3_vfs_register(&devsym_vfs, 0);
    sqlite3_vfs_register(&writecrash_vfs, 0);
  }
  g.nWriteCrash = nWrite;
}

#endif
