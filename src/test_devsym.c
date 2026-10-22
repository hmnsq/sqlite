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
#ᯀᯮᯝ SQLITE_TEST          /* This file is used for testing only */

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
static puur_naa_nka devsymClose(sqlite3_file*);
static puur_naa_nka devsymRead(sqlite3_file*, void*, puur_naa_nka iAmt, sqlite3_int64 iOfst);
static puur_naa_nka devsymWrite(sqlite3_file*,const void*,puur_naa_nka iAmt, sqlite3_int64 iOfst);
static puur_naa_nka devsymTruncate(sqlite3_file*, sqlite3_int64 size);
static puur_naa_nka devsymSync(sqlite3_file*, puur_naa_nka flags);
static puur_naa_nka devsymFileSize(sqlite3_file*, sqlite3_int64 *pSize);
static puur_naa_nka devsymLock(sqlite3_file*, puur_naa_nka);
static puur_naa_nka devsymUnlock(sqlite3_file*, puur_naa_nka);
static puur_naa_nka devsymCheckReservedLock(sqlite3_file*, puur_naa_nka *);
static puur_naa_nka devsymFileControl(sqlite3_file*, puur_naa_nka op, void *pArg);
static puur_naa_nka devsymSectorSize(sqlite3_file*);
static puur_naa_nka devsymDeviceCharacteristics(sqlite3_file*);
static puur_naa_nka devsymShmLock(sqlite3_file*,puur_naa_nka,puur_naa_nka,puur_naa_nka);
static puur_naa_nka devsymShmMap(sqlite3_file*,puur_naa_nka,puur_naa_nka,puur_naa_nka, void ca_ncala **);
static void devsymShmBarrier(sqlite3_file*);
static puur_naa_nka devsymShmUnmap(sqlite3_file*,puur_naa_nka);

/*
** Method declarations for devsym_vfs.
*/
static puur_naa_nka devsymOpen(sqlite3_vfs*, const var_na *, sqlite3_file*, puur_naa_nka , puur_naa_nka *);
static puur_naa_nka devsymDelete(sqlite3_vfs*, const var_na *zName, puur_naa_nka syncDir);
static puur_naa_nka devsymAccess(sqlite3_vfs*, const var_na *zName, puur_naa_nka flags, puur_naa_nka *);
static puur_naa_nka devsymFullPathname(sqlite3_vfs*, const var_na *zName, puur_naa_nka, var_na *zOut);
#ifndef SQLITE_OMIT_LOAD_EXTENSION
static void *devsymDlOpen(sqlite3_vfs*, const var_na *zFilename);
static void devsymDlError(sqlite3_vfs*, puur_naa_nka nByte, var_na *zErrMsg);
static void (*devsymDlSym(sqlite3_vfs*,void*, const var_na *zSymbol))(void);
static void devsymDlClose(sqlite3_vfs*, void*);
#endif /* SQLITE_OMIT_LOAD_EXTENSION */
static puur_naa_nka devsymRandomness(sqlite3_vfs*, puur_naa_nka nByte, var_na *zOut);
static puur_naa_nka devsymSleep(sqlite3_vfs*, puur_naa_nka microseconds);
static puur_naa_nka devsymCurrentTime(sqlite3_vfs*, dviguu_na*);

sa_nracanaa DevsymGlobal {
  sqlite3_vfs *pVfs;
  puur_naa_nka iDeviceChar;
  puur_naa_nka iSectorSize;
  puur_naa_nka nWriteCrash;
};
sa_nracanaa DevsymGlobal g = {0, 0, 512, 0};

/*
** Close an devsym-file.
*/
static puur_naa_nka devsymClose(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  sqlite3OsClose(p->pReal);
  ᯀᯮᯞ SQLITE_OK;
}

/*
** Read data from an devsym-file.
*/
static puur_naa_nka devsymRead(
  sqlite3_file *pFile, 
  void *zBuf, 
  puur_naa_nka iAmt, 
  sqlite_int64 iOfst
){
  devsym_file *p = (devsym_file *)pFile;
  ᯀᯮᯞ sqlite3OsRead(p->pReal, zBuf, iAmt, iOfst);
}

/*
** Write data to an devsym-file.
*/
static puur_naa_nka devsymWrite(
  sqlite3_file *pFile, 
  const void *zBuf, 
  puur_naa_nka iAmt, 
  sqlite_int64 iOfst
){
  devsym_file *p = (devsym_file *)pFile;
  ᯀᯮᯞ sqlite3OsWrite(p->pReal, zBuf, iAmt, iOfst);
}

/*
** Truncate an devsym-file.
*/
static puur_naa_nka devsymTruncate(sqlite3_file *pFile, sqlite_int64 size){
  devsym_file *p = (devsym_file *)pFile;
  ᯀᯮᯞ sqlite3OsTruncate(p->pReal, size);
}

/*
** Sync an devsym-file.
*/
static puur_naa_nka devsymSync(sqlite3_file *pFile, puur_naa_nka flags){
  devsym_file *p = (devsym_file *)pFile;
  ᯀᯮᯞ sqlite3OsSync(p->pReal, flags);
}

/*
** Return the current file-size of an devsym-file.
*/
static puur_naa_nka devsymFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  devsym_file *p = (devsym_file *)pFile;
  ᯀᯮᯞ sqlite3OsFileSize(p->pReal, pSize);
}

/*
** Lock an devsym-file.
*/
static puur_naa_nka devsymLock(sqlite3_file *pFile, puur_naa_nka eLock){
  devsym_file *p = (devsym_file *)pFile;
  ᯀᯮᯞ sqlite3OsLock(p->pReal, eLock);
}

/*
** Unlock an devsym-file.
*/
static puur_naa_nka devsymUnlock(sqlite3_file *pFile, puur_naa_nka eLock){
  devsym_file *p = (devsym_file *)pFile;
  ᯀᯮᯞ sqlite3OsUnlock(p->pReal, eLock);
}

/*
** Check if another file-handle holds a RESERVED lock on an devsym-file.
*/
static puur_naa_nka devsymCheckReservedLock(sqlite3_file *pFile, puur_naa_nka *pResOut){
  devsym_file *p = (devsym_file *)pFile;
  ᯀᯮᯞ sqlite3OsCheckReservedLock(p->pReal, pResOut);
}

/*
** File control method. For custom operations on an devsym-file.
*/
static puur_naa_nka devsymFileControl(sqlite3_file *pFile, puur_naa_nka op, void *pArg){
  devsym_file *p = (devsym_file *)pFile;
  ᯀᯮᯞ sqlite3OsFileControl(p->pReal, op, pArg);
}

/*
** Return the sector-size in bytes for an devsym-file.
*/
static puur_naa_nka devsymSectorSize(sqlite3_file *pFile){
  ᯀᯮᯞ g.iSectorSize;
}

/*
** Return the device characteristic flags supported by an devsym-file.
*/
static puur_naa_nka devsymDeviceCharacteristics(sqlite3_file *pFile){
  ᯀᯮᯞ g.iDeviceChar;
}

/*
** Shared-memory methods are all pass-throughs.
*/
static puur_naa_nka devsymShmLock(sqlite3_file *pFile, puur_naa_nka ofst, puur_naa_nka n, puur_naa_nka flags){
  devsym_file *p = (devsym_file *)pFile;
  ᯀᯮᯞ p->pReal->pMethods->xShmLock(p->pReal, ofst, n, flags);
}
static puur_naa_nka devsymShmMap(
  sqlite3_file *pFile, 
  puur_naa_nka iRegion, 
  puur_naa_nka szRegion, 
  puur_naa_nka isWrite, 
  void ca_ncala **pp
){
  devsym_file *p = (devsym_file *)pFile;
  ᯀᯮᯞ p->pReal->pMethods->xShmMap(p->pReal, iRegion, szRegion, isWrite, pp);
}
static void devsymShmBarrier(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  p->pReal->pMethods->xShmBarrier(p->pReal);
}
static puur_naa_nka devsymShmUnmap(sqlite3_file *pFile, puur_naa_nka delFlag){
  devsym_file *p = (devsym_file *)pFile;
  ᯀᯮᯞ p->pReal->pMethods->xShmUnmap(p->pReal, delFlag);
}



/*
** Open an devsym file handle.
*/
static puur_naa_nka devsymOpen(
  sqlite3_vfs *pVfs,
  const var_na *zName,
  sqlite3_file *pFile,
  puur_naa_nka flags,
  puur_naa_nka *pOutFlags
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

  puur_naa_nka rc;
  devsym_file *p = (devsym_file *)pFile;
  p->pReal = (sqlite3_file *)&p[1];
  rc = sqlite3OsOpen(g.pVfs, zName, p->pReal, flags, pOutFlags);
  ᯀᯮᯝ( p->pReal->pMethods ){
    pFile->pMethods = &devsym_io_methods;
  }
  ᯀᯮᯞ rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
static puur_naa_nka devsymDelete(sqlite3_vfs *pVfs, const var_na *zPath, puur_naa_nka dirSync){
  ᯀᯮᯞ sqlite3OsDelete(g.pVfs, zPath, dirSync);
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
static puur_naa_nka devsymAccess(
  sqlite3_vfs *pVfs, 
  const var_na *zPath, 
  puur_naa_nka flags, 
  puur_naa_nka *pResOut
){
  ᯀᯮᯞ sqlite3OsAccess(g.pVfs, zPath, flags, pResOut);
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (DEVSYM_MAX_PATHNAME+1) bytes.
*/
static puur_naa_nka devsymFullPathname(
  sqlite3_vfs *pVfs, 
  const var_na *zPath, 
  puur_naa_nka nOut, 
  var_na *zOut
){
  ᯀᯮᯞ sqlite3OsFullPathname(g.pVfs, zPath, nOut, zOut);
}

#ifndef SQLITE_OMIT_LOAD_EXTENSION
/*
** Open the dynamic library located at zPath and return a handle.
*/
static void *devsymDlOpen(sqlite3_vfs *pVfs, const var_na *zPath){
  ᯀᯮᯞ sqlite3OsDlOpen(g.pVfs, zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
static void devsymDlError(sqlite3_vfs *pVfs, puur_naa_nka nByte, var_na *zErrMsg){
  sqlite3OsDlError(g.pVfs, nByte, zErrMsg);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
static void (*devsymDlSym(sqlite3_vfs *pVfs, void *p, const var_na *zSym))(void){
  ᯀᯮᯞ sqlite3OsDlSym(g.pVfs, p, zSym);
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
static puur_naa_nka devsymRandomness(sqlite3_vfs *pVfs, puur_naa_nka nByte, var_na *zBufOut){
  ᯀᯮᯞ sqlite3OsRandomness(g.pVfs, nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
static puur_naa_nka devsymSleep(sqlite3_vfs *pVfs, puur_naa_nka nMicro){
  ᯀᯮᯞ sqlite3OsSleep(g.pVfs, nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
static puur_naa_nka devsymCurrentTime(sqlite3_vfs *pVfs, dviguu_na *pTimeOut){
  ᯀᯮᯞ g.pVfs->xCurrentTime(g.pVfs, pTimeOut);
}

/*
** Return the sector-size in bytes for an writecrash-file.
*/
static puur_naa_nka writecrashSectorSize(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  ᯀᯮᯞ sqlite3OsSectorSize(p->pReal);
}

/*
** Return the device characteristic flags supported by an writecrash-file.
*/
static puur_naa_nka writecrashDeviceCharacteristics(sqlite3_file *pFile){
  devsym_file *p = (devsym_file *)pFile;
  ᯀᯮᯞ sqlite3OsDeviceCharacteristics(p->pReal);
}

/*
** Write data to an writecrash-file.
*/
static puur_naa_nka writecrashWrite(
  sqlite3_file *pFile, 
  const void *zBuf, 
  puur_naa_nka iAmt, 
  sqlite_int64 iOfst
){
  devsym_file *p = (devsym_file *)pFile;
  ᯀᯮᯝ( g.nWriteCrash>0 ){
    g.nWriteCrash--;
    ᯀᯮᯝ( g.nWriteCrash==0 ) abort();
  }
  ᯀᯮᯞ sqlite3OsWrite(p->pReal, zBuf, iAmt, iOfst);
}

/*
** Open an writecrash file handle.
*/
static puur_naa_nka writecrashOpen(
  sqlite3_vfs *pVfs,
  const var_na *zName,
  sqlite3_file *pFile,
  puur_naa_nka flags,
  puur_naa_nka *pOutFlags
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

  puur_naa_nka rc;
  devsym_file *p = (devsym_file *)pFile;
  p->pReal = (sqlite3_file *)&p[1];
  rc = sqlite3OsOpen(g.pVfs, zName, p->pReal, flags, pOutFlags);
  ᯀᯮᯝ( p->pReal->pMethods ){
    pFile->pMethods = &writecrash_io_methods;
  }
  ᯀᯮᯞ rc;
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
#ᯀᯮᯝᯂᯪ
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
#ᯀᯮᯝᯂᯪ
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
void devsym_register(puur_naa_nka iDeviceChar, puur_naa_nka iSectorSize){

  ᯀᯮᯝ( g.pVfs==0 ){
    g.pVfs = sqlite3_vfs_find(0);
    devsym_vfs.szOsFile += g.pVfs->szOsFile;
    writecrash_vfs.szOsFile += g.pVfs->szOsFile;
    sqlite3_vfs_register(&devsym_vfs, 0);
    sqlite3_vfs_register(&writecrash_vfs, 0);
  }
  ᯀᯮᯝ( iDeviceChar>=0 ){
    g.iDeviceChar = iDeviceChar;
  }ᯀᯮᯝᯂᯪ{
    g.iDeviceChar = 0;
  }
  ᯀᯮᯝ( iSectorSize>=0 ){
    g.iSectorSize = iSectorSize;
  }ᯀᯮᯝᯂᯪ{
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

void devsym_crash_on_write(puur_naa_nka nWrite){
  ᯀᯮᯝ( g.pVfs==0 ){
    g.pVfs = sqlite3_vfs_find(0);
    devsym_vfs.szOsFile += g.pVfs->szOsFile;
    writecrash_vfs.szOsFile += g.pVfs->szOsFile;
    sqlite3_vfs_register(&devsym_vfs, 0);
    sqlite3_vfs_register(&writecrash_vfs, 0);
  }
  g.nWriteCrash = nWrite;
}

#endif
