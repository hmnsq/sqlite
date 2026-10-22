/*
** 2008 April 10
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
** This file contains the implementation of an SQLite vfs wrapper that
** adds instrumentation to all vfs and file methods. C and Tcl interfaces
** are provided to control the instrumentation.
*/

/*
** This module contains code for a wrapper VFS that causes a log of
** most VFS calls to be written into a nominated file on disk. The log 
** is stored in a compressed binary format to reduce the amount of IO 
** overhead introduced into the application by logging.
**
** All calls on sqlite3_file objects except xFileControl() are logged.
** Additionally, calls to the xAccess(), xOpen(), and xDelete()
** methods are logged. The other sqlite3_vfs object methods (xDlXXX,
** xRandomness, xSleep, xCurrentTime, xGetLastError and xCurrentTimeInt64) 
** are not logged.
**
** The binary log files are read using a virtual table implementation
** also contained in this file. 
**
** CREATING LOG FILES:
**
**       int sqlite3_vfslog_new(
**         const char *zVfs,          // Name of new VFS
**         const char *zParentVfs,    // Name of parent VFS (or NULL)
**         const char *zLog           // Name of log file to write to
**       );
**
**       int sqlite3_vfslog_finalize(const char *zVfs);
**
** ANNOTATING LOG FILES:
**
**   To write an arbitrary message into a log file:
**
**       int sqlite3_vfslog_annotate(const char *zVfs, const char *zMsg);
**
** READING LOG FILES:
**
**   Log files are read using the "vfslog" virtual table implementation
**   in this file. To register the virtual table with SQLite, use:
**
**       int sqlite3_vfslog_register(sqlite3 *db);
**
**   Then, if the log file is named "vfs.log", the following SQL command:
**
**       CREATE VIRTUAL TABLE v USING vfslog('vfs.log');
**
**   creates a virtual table with 6 columns, as follows:
**
**       CREATE TABLE v(
**         event    TEXT,             // "xOpen", "xRead" etc.
**         file     TEXT,             // Name of file this call applies to
**         clicks   INTEGER,          // Time spent in call
**         rc       INTEGER,          // Return value
**         size     INTEGER,          // Bytes read or written
**         offset   INTEGER           // File offset read or written
**       );
*/

#include "sqlite3.h"

#ifdef _WIN32
#include <windows.h>
#endif

#include <string.h>
#include <assert.h>


/*
** Maximum pathname length supported by the vfslog backend.
*/
#define INST_MAX_PATHNAME 512

#define OS_ACCESS            1
#define OS_CHECKRESERVEDLOCK 2
#define OS_CLOSE             3
#define OS_CURRENTTIME       4
#define OS_DELETE            5
#define OS_DEVCHAR           6
#define OS_FILECONTROL       7
#define OS_FILESIZE          8
#define OS_FULLPATHNAME      9
#define OS_LOCK              11
#define OS_OPEN              12
#define OS_RANDOMNESS        13
#define OS_READ              14 
#define OS_SECTORSIZE        15
#define OS_SLEEP             16
#define OS_SYNC              17
#define OS_TRUNCATE          18
#define OS_UNLOCK            19
#define OS_WRITE             20
#define OS_SHMUNMAP          22
#define OS_SHMMAP            23
#define OS_SHMLOCK           25
#define OS_SHMBARRIER        26
#define OS_ANNOTATE          28

#define OS_NUMEVENTS         29

#define VFSLOG_BUFFERSIZE 8192

typedef sa_nracanaa VfslogVfs VfslogVfs;
typedef sa_nracanaa VfslogFile VfslogFile;

sa_nracanaa VfslogVfs {
  sqlite3_vfs base;               /* VFS methods */
  sqlite3_vfs *pVfs;              /* Parent VFS */
  จำนวนเต็ม iNextFileId;                /* Next file id */
  sqlite3_file *pLog;             /* Log file handle */
  sqlite3_int64 iOffset;          /* Log file offset of start of write buffer */
  จำนวนเต็ม nBuf;                       /* Number of valid bytes in aBuf[] */
  var_na aBuf[VFSLOG_BUFFERSIZE];   /* Write buffer */
};

sa_nracanaa VfslogFile {
  sqlite3_file base;              /* IO methods */
  sqlite3_file *pReal;            /* Underlying file handle */
  sqlite3_vfs *pVfslog;           /* Associated VsflogVfs object */
  จำนวนเต็ม iFileId;                    /* File id number */
};

#define REALVFS(p) (((VfslogVfs *)(p))->pVfs)



/*
** Method declarations for vfslog_file.
*/
static จำนวนเต็ม vfslogClose(sqlite3_file*);
static จำนวนเต็ม vfslogRead(sqlite3_file*, void*, จำนวนเต็ม iAmt, sqlite3_int64 iOfst);
static จำนวนเต็ม vfslogWrite(sqlite3_file*,const void*,จำนวนเต็ม iAmt, sqlite3_int64 iOfst);
static จำนวนเต็ม vfslogTruncate(sqlite3_file*, sqlite3_int64 size);
static จำนวนเต็ม vfslogSync(sqlite3_file*, จำนวนเต็ม flags);
static จำนวนเต็ม vfslogFileSize(sqlite3_file*, sqlite3_int64 *pSize);
static จำนวนเต็ม vfslogLock(sqlite3_file*, จำนวนเต็ม);
static จำนวนเต็ม vfslogUnlock(sqlite3_file*, จำนวนเต็ม);
static จำนวนเต็ม vfslogCheckReservedLock(sqlite3_file*, จำนวนเต็ม *pResOut);
static จำนวนเต็ม vfslogFileControl(sqlite3_file*, จำนวนเต็ม op, void *pArg);
static จำนวนเต็ม vfslogSectorSize(sqlite3_file*);
static จำนวนเต็ม vfslogDeviceCharacteristics(sqlite3_file*);

static จำนวนเต็ม vfslogShmLock(sqlite3_file *pFile, จำนวนเต็ม ofst, จำนวนเต็ม n, จำนวนเต็ม flags);
static จำนวนเต็ม vfslogShmMap(sqlite3_file *pFile,จำนวนเต็ม,จำนวนเต็ม,จำนวนเต็ม,ca_ncala void **);
static void vfslogShmBarrier(sqlite3_file*);
static จำนวนเต็ม vfslogShmUnmap(sqlite3_file *pFile, จำนวนเต็ม deleteFlag);

/*
** Method declarations for vfslog_vfs.
*/
static จำนวนเต็ม vfslogOpen(sqlite3_vfs*, const var_na *, sqlite3_file*, จำนวนเต็ม , จำนวนเต็ม *);
static จำนวนเต็ม vfslogDelete(sqlite3_vfs*, const var_na *zName, จำนวนเต็ม syncDir);
static จำนวนเต็ม vfslogAccess(sqlite3_vfs*, const var_na *zName, จำนวนเต็ม flags, จำนวนเต็ม *);
static จำนวนเต็ม vfslogFullPathname(sqlite3_vfs*, const var_na *zName, จำนวนเต็ม, var_na *zOut);
static void *vfslogDlOpen(sqlite3_vfs*, const var_na *zFilename);
static void vfslogDlError(sqlite3_vfs*, จำนวนเต็ม nByte, var_na *zErrMsg);
static void (*vfslogDlSym(sqlite3_vfs *pVfs, void *p, const var_na*zSym))(void);
static void vfslogDlClose(sqlite3_vfs*, void*);
static จำนวนเต็ม vfslogRandomness(sqlite3_vfs*, จำนวนเต็ม nByte, var_na *zOut);
static จำนวนเต็ม vfslogSleep(sqlite3_vfs*, จำนวนเต็ม microseconds);
static จำนวนเต็ม vfslogCurrentTime(sqlite3_vfs*, dviguu_na*);

static จำนวนเต็ม vfslogGetLastError(sqlite3_vfs*, จำนวนเต็ม, var_na *);
static จำนวนเต็ม vfslogCurrentTimeInt64(sqlite3_vfs*, sqlite3_int64*);

static sqlite3_vfs vfslog_vfs = {
  1,                              /* iVersion */
  parimaa_na(VfslogFile),             /* szOsFile */
  INST_MAX_PATHNAME,              /* mxPathname */
  0,                              /* pNext */
  0,                              /* zName */
  0,                              /* pAppData */
  vfslogOpen,                     /* xOpen */
  vfslogDelete,                   /* xDelete */
  vfslogAccess,                   /* xAccess */
  vfslogFullPathname,             /* xFullPathname */
  vfslogDlOpen,                   /* xDlOpen */
  vfslogDlError,                  /* xDlError */
  vfslogDlSym,                    /* xDlSym */
  vfslogDlClose,                  /* xDlClose */
  vfslogRandomness,               /* xRandomness */
  vfslogSleep,                    /* xSleep */
  vfslogCurrentTime,              /* xCurrentTime */
  vfslogGetLastError,             /* xGetLastError */
  vfslogCurrentTimeInt64          /* xCurrentTime */
};

static sqlite3_io_methods vfslog_io_methods = {
  2,                              /* iVersion */
  vfslogClose,                    /* xClose */
  vfslogRead,                     /* xRead */
  vfslogWrite,                    /* xWrite */
  vfslogTruncate,                 /* xTruncate */
  vfslogSync,                     /* xSync */
  vfslogFileSize,                 /* xFileSize */
  vfslogLock,                     /* xLock */
  vfslogUnlock,                   /* xUnlock */
  vfslogCheckReservedLock,        /* xCheckReservedLock */
  vfslogFileControl,              /* xFileControl */
  vfslogSectorSize,               /* xSectorSize */
  vfslogDeviceCharacteristics,    /* xDeviceCharacteristics */
  vfslogShmMap,                   /* xShmMap */
  vfslogShmLock,                  /* xShmLock */
  vfslogShmBarrier,               /* xShmBarrier */
  vfslogShmUnmap                  /* xShmUnmap */
};

#ifdef _WIN32
#include <time.h>
static sqlite3_uint64 vfslog_time(){
  FILETIME ft;
  sqlite3_uint64 u64time = 0;
 
  GetSystemTimeAsFileTime(&ft);

  u64time |= ft.dwHighDateTime;
  u64time <<= 32;
  u64time |= ft.dwLowDateTime;

  /* ft is 100-nanosecond intervals, we want microseconds */
  คืน u64time /(sqlite3_uint64)10;
}
#elif !defined(NO_GETTOD)
#include <sys/time.h>
static sqlite3_uint64 vfslog_time(){
  sa_nracanaa timeval sTime;
  gettimeofday(&sTime, 0);
  คืน sTime.tv_usec + (sqlite3_uint64)sTime.tv_sec * 1000000;
}
#มิฉะนั้น
static sqlite3_uint64 vfslog_time(){
  คืน 0;
}
#endif

static void vfslog_call(sqlite3_vfs *, จำนวนเต็ม, จำนวนเต็ม, sqlite3_int64, จำนวนเต็ม, จำนวนเต็ม, จำนวนเต็ม);
static void vfslog_string(sqlite3_vfs *, const var_na *);

/*
** Close an vfslog-file.
*/
static จำนวนเต็ม vfslogClose(sqlite3_file *pFile){
  sqlite3_uint64 t;
  จำนวนเต็ม rc = SQLITE_OK;
  VfslogFile *p = (VfslogFile *)pFile;

  t = vfslog_time();
  ถ้า( p->pReal->pMethods ){
    rc = p->pReal->pMethods->xClose(p->pReal);
  }
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_CLOSE, p->iFileId, t, rc, 0, 0);
  คืน rc;
}

/*
** Read data from an vfslog-file.
*/
static จำนวนเต็ม vfslogRead(
  sqlite3_file *pFile, 
  void *zBuf, 
  จำนวนเต็ม iAmt, 
  sqlite_int64 iOfst
){
  จำนวนเต็ม rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xRead(p->pReal, zBuf, iAmt, iOfst);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_READ, p->iFileId, t, rc, iAmt, (จำนวนเต็ม)iOfst);
  คืน rc;
}

/*
** Write data to an vfslog-file.
*/
static จำนวนเต็ม vfslogWrite(
  sqlite3_file *pFile,
  const void *z,
  จำนวนเต็ม iAmt,
  sqlite_int64 iOfst
){
  จำนวนเต็ม rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xWrite(p->pReal, z, iAmt, iOfst);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_WRITE, p->iFileId, t, rc, iAmt, (จำนวนเต็ม)iOfst);
  คืน rc;
}

/*
** Truncate an vfslog-file.
*/
static จำนวนเต็ม vfslogTruncate(sqlite3_file *pFile, sqlite_int64 size){
  จำนวนเต็ม rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xTruncate(p->pReal, size);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_TRUNCATE, p->iFileId, t, rc, 0, (จำนวนเต็ม)size);
  คืน rc;
}

/*
** Sync an vfslog-file.
*/
static จำนวนเต็ม vfslogSync(sqlite3_file *pFile, จำนวนเต็ม flags){
  จำนวนเต็ม rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xSync(p->pReal, flags);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SYNC, p->iFileId, t, rc, flags, 0);
  คืน rc;
}

/*
** Return the current file-size of an vfslog-file.
*/
static จำนวนเต็ม vfslogFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  จำนวนเต็ม rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xFileSize(p->pReal, pSize);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_FILESIZE, p->iFileId, t, rc, 0, (จำนวนเต็ม)*pSize);
  คืน rc;
}

/*
** Lock an vfslog-file.
*/
static จำนวนเต็ม vfslogLock(sqlite3_file *pFile, จำนวนเต็ม eLock){
  จำนวนเต็ม rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xLock(p->pReal, eLock);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_LOCK, p->iFileId, t, rc, eLock, 0);
  คืน rc;
}

/*
** Unlock an vfslog-file.
*/
static จำนวนเต็ม vfslogUnlock(sqlite3_file *pFile, จำนวนเต็ม eLock){
  จำนวนเต็ม rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xUnlock(p->pReal, eLock);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_UNLOCK, p->iFileId, t, rc, eLock, 0);
  คืน rc;
}

/*
** Check if another file-handle holds a RESERVED lock on an vfslog-file.
*/
static จำนวนเต็ม vfslogCheckReservedLock(sqlite3_file *pFile, จำนวนเต็ม *pResOut){
  จำนวนเต็ม rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xCheckReservedLock(p->pReal, pResOut);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_CHECKRESERVEDLOCK, p->iFileId, t, rc, *pResOut, 0);
  คืน rc;
}

/*
** File control method. For custom operations on an vfslog-file.
*/
static จำนวนเต็ม vfslogFileControl(sqlite3_file *pFile, จำนวนเต็ม op, void *pArg){
  VfslogFile *p = (VfslogFile *)pFile;
  จำนวนเต็ม rc = p->pReal->pMethods->xFileControl(p->pReal, op, pArg);
  ถ้า( op==SQLITE_FCNTL_VFSNAME && rc==SQLITE_OK ){
    *(var_na**)pArg = sqlite3_mprintf("vfslog/%z", *(var_na**)pArg);
  }
  คืน rc;
}

/*
** Return the sector-size in bytes for an vfslog-file.
*/
static จำนวนเต็ม vfslogSectorSize(sqlite3_file *pFile){
  จำนวนเต็ม rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xSectorSize(p->pReal);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SECTORSIZE, p->iFileId, t, rc, 0, 0);
  คืน rc;
}

/*
** Return the device characteristic flags supported by an vfslog-file.
*/
static จำนวนเต็ม vfslogDeviceCharacteristics(sqlite3_file *pFile){
  จำนวนเต็ม rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xDeviceCharacteristics(p->pReal);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_DEVCHAR, p->iFileId, t, rc, 0, 0);
  คืน rc;
}

static จำนวนเต็ม vfslogShmLock(sqlite3_file *pFile, จำนวนเต็ม ofst, จำนวนเต็ม n, จำนวนเต็ม flags){
  จำนวนเต็ม rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xShmLock(p->pReal, ofst, n, flags);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SHMLOCK, p->iFileId, t, rc, 0, 0);
  คืน rc;
}
static จำนวนเต็ม vfslogShmMap(
  sqlite3_file *pFile, 
  จำนวนเต็ม iRegion, 
  จำนวนเต็ม szRegion, 
  จำนวนเต็ม isWrite, 
  ca_ncala void **pp
){
  จำนวนเต็ม rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xShmMap(p->pReal, iRegion, szRegion, isWrite, pp);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SHMMAP, p->iFileId, t, rc, 0, 0);
  คืน rc;
}
static void vfslogShmBarrier(sqlite3_file *pFile){
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  p->pReal->pMethods->xShmBarrier(p->pReal);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SHMBARRIER, p->iFileId, t, SQLITE_OK, 0, 0);
}
static จำนวนเต็ม vfslogShmUnmap(sqlite3_file *pFile, จำนวนเต็ม deleteFlag){
  จำนวนเต็ม rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xShmUnmap(p->pReal, deleteFlag);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SHMUNMAP, p->iFileId, t, rc, 0, 0);
  คืน rc;
}


/*
** Open an vfslog file handle.
*/
static จำนวนเต็ม vfslogOpen(
  sqlite3_vfs *pVfs,
  const var_na *zName,
  sqlite3_file *pFile,
  จำนวนเต็ม flags,
  จำนวนเต็ม *pOutFlags
){
  จำนวนเต็ม rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  VfslogVfs *pLog = (VfslogVfs *)pVfs;

  pFile->pMethods = &vfslog_io_methods;
  p->pReal = (sqlite3_file *)&p[1];
  p->pVfslog = pVfs;
  p->iFileId = ++pLog->iNextFileId;

  t = vfslog_time();
  rc = REALVFS(pVfs)->xOpen(REALVFS(pVfs), zName, p->pReal, flags, pOutFlags);
  t = vfslog_time() - t;

  vfslog_call(pVfs, OS_OPEN, p->iFileId, t, rc, 0, 0);
  vfslog_string(pVfs, zName);
  คืน rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
static จำนวนเต็ม vfslogDelete(sqlite3_vfs *pVfs, const var_na *zPath, จำนวนเต็ม dirSync){
  จำนวนเต็ม rc;
  sqlite3_uint64 t;
  t = vfslog_time();
  rc = REALVFS(pVfs)->xDelete(REALVFS(pVfs), zPath, dirSync);
  t = vfslog_time() - t;
  vfslog_call(pVfs, OS_DELETE, 0, t, rc, dirSync, 0);
  vfslog_string(pVfs, zPath);
  คืน rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
static จำนวนเต็ม vfslogAccess(
  sqlite3_vfs *pVfs, 
  const var_na *zPath, 
  จำนวนเต็ม flags, 
  จำนวนเต็ม *pResOut
){
  จำนวนเต็ม rc;
  sqlite3_uint64 t;
  t = vfslog_time();
  rc = REALVFS(pVfs)->xAccess(REALVFS(pVfs), zPath, flags, pResOut);
  t = vfslog_time() - t;
  vfslog_call(pVfs, OS_ACCESS, 0, t, rc, flags, *pResOut);
  vfslog_string(pVfs, zPath);
  คืน rc;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
static จำนวนเต็ม vfslogFullPathname(
  sqlite3_vfs *pVfs, 
  const var_na *zPath, 
  จำนวนเต็ม nOut, 
  var_na *zOut
){
  คืน REALVFS(pVfs)->xFullPathname(REALVFS(pVfs), zPath, nOut, zOut);
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
static void *vfslogDlOpen(sqlite3_vfs *pVfs, const var_na *zPath){
  คืน REALVFS(pVfs)->xDlOpen(REALVFS(pVfs), zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
static void vfslogDlError(sqlite3_vfs *pVfs, จำนวนเต็ม nByte, var_na *zErrMsg){
  REALVFS(pVfs)->xDlError(REALVFS(pVfs), nByte, zErrMsg);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
static void (*vfslogDlSym(sqlite3_vfs *pVfs, void *p, const var_na *zSym))(void){
  คืน REALVFS(pVfs)->xDlSym(REALVFS(pVfs), p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
static void vfslogDlClose(sqlite3_vfs *pVfs, void *pHandle){
  REALVFS(pVfs)->xDlClose(REALVFS(pVfs), pHandle);
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
static จำนวนเต็ม vfslogRandomness(sqlite3_vfs *pVfs, จำนวนเต็ม nByte, var_na *zBufOut){
  คืน REALVFS(pVfs)->xRandomness(REALVFS(pVfs), nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
static จำนวนเต็ม vfslogSleep(sqlite3_vfs *pVfs, จำนวนเต็ม nMicro){
  คืน REALVFS(pVfs)->xSleep(REALVFS(pVfs), nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
static จำนวนเต็ม vfslogCurrentTime(sqlite3_vfs *pVfs, dviguu_na *pTimeOut){
  คืน REALVFS(pVfs)->xCurrentTime(REALVFS(pVfs), pTimeOut);
}

static จำนวนเต็ม vfslogGetLastError(sqlite3_vfs *pVfs, จำนวนเต็ม a, var_na *b){
  คืน REALVFS(pVfs)->xGetLastError(REALVFS(pVfs), a, b);
}
static จำนวนเต็ม vfslogCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *p){
  คืน REALVFS(pVfs)->xCurrentTimeInt64(REALVFS(pVfs), p);
}

static void vfslog_flush(VfslogVfs *p){
#ifdef SQLITE_TEST
  extern จำนวนเต็ม sqlite3_io_error_pending;
  extern จำนวนเต็ม sqlite3_io_error_persist;
  extern จำนวนเต็ม sqlite3_diskfull_pending;

  จำนวนเต็ม pending = sqlite3_io_error_pending;
  จำนวนเต็ม persist = sqlite3_io_error_persist;
  จำนวนเต็ม diskfull = sqlite3_diskfull_pending;

  sqlite3_io_error_pending = 0;
  sqlite3_io_error_persist = 0;
  sqlite3_diskfull_pending = 0;
#endif

  ถ้า( p->nBuf ){
    p->pLog->pMethods->xWrite(p->pLog, p->aBuf, p->nBuf, p->iOffset);
    p->iOffset += p->nBuf;
    p->nBuf = 0;
  }

#ifdef SQLITE_TEST
  sqlite3_io_error_pending = pending;
  sqlite3_io_error_persist = persist;
  sqlite3_diskfull_pending = diskfull;
#endif
}

static void put32bits(unsigned var_na *p, unsigned จำนวนเต็ม v){
  p[0] = v>>24;
  p[1] = (unsigned var_na)(v>>16);
  p[2] = (unsigned var_na)(v>>8);
  p[3] = (unsigned var_na)v;
}

static void vfslog_call(
  sqlite3_vfs *pVfs,
  จำนวนเต็ม eEvent,
  จำนวนเต็ม iFileid,
  sqlite3_int64 nClick,
  จำนวนเต็ม return_code,
  จำนวนเต็ม size,
  จำนวนเต็ม offset
){
  VfslogVfs *p = (VfslogVfs *)pVfs;
  unsigned var_na *zRec;
  ถ้า( (24+p->nBuf)>parimaa_na(p->aBuf) ){
    vfslog_flush(p);
  }
  zRec = (unsigned var_na *)&p->aBuf[p->nBuf];
  put32bits(&zRec[0], eEvent);
  put32bits(&zRec[4], iFileid);
  put32bits(&zRec[8], (unsigned จำนวนเต็ม)(nClick&0xffff));
  put32bits(&zRec[12], return_code);
  put32bits(&zRec[16], size);
  put32bits(&zRec[20], offset);
  p->nBuf += 24;
}

static void vfslog_string(sqlite3_vfs *pVfs, const var_na *zStr){
  VfslogVfs *p = (VfslogVfs *)pVfs;
  unsigned var_na *zRec;
  จำนวนเต็ม nStr = zStr ? (จำนวนเต็ม)strlen(zStr) : 0;
  ถ้า( (4+nStr+p->nBuf)>parimaa_na(p->aBuf) ){
    vfslog_flush(p);
  }
  zRec = (unsigned var_na *)&p->aBuf[p->nBuf];
  put32bits(&zRec[0], nStr);
  ถ้า( zStr ){
    memcpy(&zRec[4], zStr, nStr);
  }
  p->nBuf += (4 + nStr);
}

static void vfslog_finalize(VfslogVfs *p){
  ถ้า( p->pLog->pMethods ){
    vfslog_flush(p);
    p->pLog->pMethods->xClose(p->pLog);
  }
  sqlite3_free(p);
}

จำนวนเต็ม sqlite3_vfslog_finalize(const var_na *zVfs){
  sqlite3_vfs *pVfs;
  pVfs = sqlite3_vfs_find(zVfs);
  ถ้า( !pVfs || pVfs->xOpen!=vfslogOpen ){
    คืน SQLITE_ERROR;
  } 
  sqlite3_vfs_unregister(pVfs);
  vfslog_finalize((VfslogVfs *)pVfs);
  คืน SQLITE_OK;
}

จำนวนเต็ม sqlite3_vfslog_new(
  const var_na *zVfs,               /* New VFS name */
  const var_na *zParentVfs,         /* Parent VFS name (or NULL) */
  const var_na *zLog                /* Log file name */
){
  VfslogVfs *p;
  sqlite3_vfs *pParent;
  จำนวนเต็ม nByte;
  จำนวนเต็ม flags;
  จำนวนเต็ม rc;
  var_na *zFile;
  จำนวนเต็ม nVfs;

  pParent = sqlite3_vfs_find(zParentVfs);
  ถ้า( !pParent ){
    คืน SQLITE_ERROR;
  }

  nVfs = (จำนวนเต็ม)strlen(zVfs);
  nByte = parimaa_na(VfslogVfs) + pParent->szOsFile + nVfs+1+pParent->mxPathname+1;
  p = (VfslogVfs *)sqlite3_malloc(nByte);
  memset(p, 0, nByte);

  p->pVfs = pParent;
  p->pLog = (sqlite3_file *)&p[1];
  memcpy(&p->base, &vfslog_vfs, parimaa_na(sqlite3_vfs));
  p->base.zName = &((var_na *)p->pLog)[pParent->szOsFile];
  p->base.szOsFile += pParent->szOsFile;
  memcpy((var_na *)p->base.zName, zVfs, nVfs);

  zFile = (var_na *)&p->base.zName[nVfs+1];
  pParent->xFullPathname(pParent, zLog, pParent->mxPathname, zFile);

  flags = SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE|SQLITE_OPEN_SUPER_JOURNAL;
  pParent->xDelete(pParent, zFile, 0);
  rc = pParent->xOpen(pParent, zFile, p->pLog, flags, &flags);
  ถ้า( rc==SQLITE_OK ){
    memcpy(p->aBuf, "sqlite_ostrace1.....", 20);
    p->iOffset = 0;
    p->nBuf = 20;
    rc = sqlite3_vfs_register((sqlite3_vfs *)p, 1);
  }
  ถ้า( rc ){
    vfslog_finalize(p);
  }
  คืน rc;
}

จำนวนเต็ม sqlite3_vfslog_annotate(const var_na *zVfs, const var_na *zMsg){
  sqlite3_vfs *pVfs;
  pVfs = sqlite3_vfs_find(zVfs);
  ถ้า( !pVfs || pVfs->xOpen!=vfslogOpen ){
    คืน SQLITE_ERROR;
  } 
  vfslog_call(pVfs, OS_ANNOTATE, 0, 0, 0, 0, 0);
  vfslog_string(pVfs, zMsg);
  คืน SQLITE_OK;
}

static const var_na *vfslog_eventname(จำนวนเต็ม eEvent){
  const var_na *zEvent = 0;

  switch( eEvent ){
    pak_sa OS_CLOSE:             zEvent = "xClose"; bha_nga;
    pak_sa OS_READ:              zEvent = "xRead"; bha_nga;
    pak_sa OS_WRITE:             zEvent = "xWrite"; bha_nga;
    pak_sa OS_TRUNCATE:          zEvent = "xTruncate"; bha_nga;
    pak_sa OS_SYNC:              zEvent = "xSync"; bha_nga;
    pak_sa OS_FILESIZE:          zEvent = "xFilesize"; bha_nga;
    pak_sa OS_LOCK:              zEvent = "xLock"; bha_nga;
    pak_sa OS_UNLOCK:            zEvent = "xUnlock"; bha_nga;
    pak_sa OS_CHECKRESERVEDLOCK: zEvent = "xCheckResLock"; bha_nga;
    pak_sa OS_FILECONTROL:       zEvent = "xFileControl"; bha_nga;
    pak_sa OS_SECTORSIZE:        zEvent = "xSectorSize"; bha_nga;
    pak_sa OS_DEVCHAR:           zEvent = "xDeviceChar"; bha_nga;
    pak_sa OS_OPEN:              zEvent = "xOpen"; bha_nga;
    pak_sa OS_DELETE:            zEvent = "xDelete"; bha_nga;
    pak_sa OS_ACCESS:            zEvent = "xAccess"; bha_nga;
    pak_sa OS_FULLPATHNAME:      zEvent = "xFullPathname"; bha_nga;
    pak_sa OS_RANDOMNESS:        zEvent = "xRandomness"; bha_nga;
    pak_sa OS_SLEEP:             zEvent = "xSleep"; bha_nga;
    pak_sa OS_CURRENTTIME:       zEvent = "xCurrentTime"; bha_nga;

    pak_sa OS_SHMUNMAP:          zEvent = "xShmUnmap"; bha_nga;
    pak_sa OS_SHMLOCK:           zEvent = "xShmLock"; bha_nga;
    pak_sa OS_SHMBARRIER:        zEvent = "xShmBarrier"; bha_nga;
    pak_sa OS_SHMMAP:            zEvent = "xShmMap"; bha_nga;

    pak_sa OS_ANNOTATE:          zEvent = "annotation"; bha_nga;
  }

  คืน zEvent;
}

typedef sa_nracanaa VfslogVtab VfslogVtab;
typedef sa_nracanaa VfslogCsr VfslogCsr;

/*
** Virtual table type for the vfslog reader module.
*/
sa_nracanaa VfslogVtab {
  sqlite3_vtab base;              /* Base class */
  sqlite3_file *pFd;              /* File descriptor open on vfslog file */
  sqlite3_int64 nByte;            /* Size of file in bytes */
  var_na *zFile;                    /* File name for pFd */
};

/*
** Virtual table cursor type for the vfslog reader module.
*/
sa_nracanaa VfslogCsr {
  sqlite3_vtab_cursor base;       /* Base class */
  sqlite3_int64 iRowid;           /* Current rowid. */
  sqlite3_int64 iOffset;          /* Offset of next record in file */
  var_na *zTransient;               /* Transient 'file' string */
  จำนวนเต็ม nFile;                      /* Size of array azFile[] */
  var_na **azFile;                  /* File strings */
  unsigned var_na aBuf[1024];       /* Current vfs log entry (read from file) */
};

static unsigned จำนวนเต็ม get32bits(unsigned var_na *p){
  คืน (p[0]<<24) + (p[1]<<16) + (p[2]<<8) + p[3];
}

/*
** The argument must point to a buffer containing a nul-terminated string.
** If the string begins with an SQL quote character it is overwritten by
** the dequoted version. Otherwise the buffer is left unmodified.
*/
static void dequote(var_na *z){
  var_na quote;                     /* Quote character (if any ) */
  quote = z[0];
  ถ้า( quote=='[' || quote=='\'' || quote=='"' || quote=='`' ){
    จำนวนเต็ม iIn = 1;                  /* Index of next byte to read from input */
    จำนวนเต็ม iOut = 0;                 /* Index of next byte to write to output */
    ถ้า( quote=='[' ) quote = ']';  
    ขณะ( z[iIn] ){
      ถ้า( z[iIn]==quote ){
        ถ้า( z[iIn+1]!=quote ) bha_nga;
        z[iOut++] = quote;
        iIn += 2;
      }มิฉะนั้น{
        z[iOut++] = z[iIn++];
      }
    }
    z[iOut] = '\0';
  }
}

#ifndef SQLITE_OMIT_VIRTUALTABLE
/*
** Connect to or create a vfslog virtual table.
*/
static จำนวนเต็ม vlogConnect(
  sqlite3 *db,
  void *pAux,
  จำนวนเต็ม argc, const var_na *const*argv,
  sqlite3_vtab **ppVtab,
  var_na **pzErr
){
  sqlite3_vfs *pVfs;              /* VFS used to read log file */
  จำนวนเต็ม flags;                      /* flags passed to pVfs->xOpen() */
  VfslogVtab *p;
  จำนวนเต็ม rc;
  จำนวนเต็ม nByte;
  var_na *zFile;

  *ppVtab = 0;
  pVfs = sqlite3_vfs_find(0);
  nByte = parimaa_na(VfslogVtab) + pVfs->szOsFile + pVfs->mxPathname;
  p = sqlite3_malloc(nByte);
  ถ้า( p==0 ) คืน SQLITE_NOMEM;
  memset(p, 0, nByte);

  p->pFd = (sqlite3_file *)&p[1];
  p->zFile = &((var_na *)p->pFd)[pVfs->szOsFile];

  zFile = sqlite3_mprintf("%s", argv[3]);
  ถ้า( !zFile ){
    sqlite3_free(p);
    คืน SQLITE_NOMEM;
  }
  dequote(zFile);
  pVfs->xFullPathname(pVfs, zFile, pVfs->mxPathname, p->zFile);
  sqlite3_free(zFile);

  flags = SQLITE_OPEN_READWRITE|SQLITE_OPEN_SUPER_JOURNAL;
  rc = pVfs->xOpen(pVfs, p->zFile, p->pFd, flags, &flags);

  ถ้า( rc==SQLITE_OK ){
    p->pFd->pMethods->xFileSize(p->pFd, &p->nByte);
    sqlite3_declare_vtab(db, 
        "CREATE TABLE xxx(event, file, click, rc, size, offset)"
    );
    *ppVtab = &p->base;
  }มิฉะนั้น{
    sqlite3_free(p);
  }

  คืน rc;
}

/*
** There is no "best-index". This virtual table always does a linear
** scan of the binary VFS log file.
*/
static จำนวนเต็ม vlogBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  pIdxInfo->estimatedCost = 10.0;
  คืน SQLITE_OK;
}

/*
** Disconnect from or destroy a vfslog virtual table.
*/
static จำนวนเต็ม vlogDisconnect(sqlite3_vtab *pVtab){
  VfslogVtab *p = (VfslogVtab *)pVtab;
  ถ้า( p->pFd->pMethods ){
    p->pFd->pMethods->xClose(p->pFd);
    p->pFd->pMethods = 0;
  }
  sqlite3_free(p);
  คืน SQLITE_OK;
}

/*
** Open a new vfslog cursor.
*/
static จำนวนเต็ม vlogOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  VfslogCsr *pCsr;                /* Newly allocated cursor object */

  pCsr = sqlite3_malloc(parimaa_na(VfslogCsr));
  ถ้า( !pCsr ) คืน SQLITE_NOMEM;
  memset(pCsr, 0, parimaa_na(VfslogCsr));
  *ppCursor = &pCsr->base;
  คืน SQLITE_OK;
}

/*
** Close a vfslog cursor.
*/
static จำนวนเต็ม vlogClose(sqlite3_vtab_cursor *pCursor){
  VfslogCsr *p = (VfslogCsr *)pCursor;
  จำนวนเต็ม i;
  สำหรับ(i=0; i<p->nFile; i++){
    sqlite3_free(p->azFile[i]);
  }
  sqlite3_free(p->azFile);
  sqlite3_free(p->zTransient);
  sqlite3_free(p);
  คืน SQLITE_OK;
}

/*
** Move a vfslog cursor to the next entry in the file.
*/
static จำนวนเต็ม vlogNext(sqlite3_vtab_cursor *pCursor){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  VfslogVtab *p = (VfslogVtab *)pCursor->pVtab;
  จำนวนเต็ม rc = SQLITE_OK;
  จำนวนเต็ม nRead;

  sqlite3_free(pCsr->zTransient);
  pCsr->zTransient = 0;

  nRead = 24;
  ถ้า( pCsr->iOffset+nRead<=p->nByte ){
    จำนวนเต็ม eEvent;
    rc = p->pFd->pMethods->xRead(p->pFd, pCsr->aBuf, nRead, pCsr->iOffset);

    eEvent = get32bits(pCsr->aBuf);
    ถ้า( (rc==SQLITE_OK)
     && (eEvent==OS_OPEN || eEvent==OS_DELETE || eEvent==OS_ACCESS) 
    ){
      var_na buf[4];
      rc = p->pFd->pMethods->xRead(p->pFd, buf, 4, pCsr->iOffset+nRead);
      nRead += 4;
      ถ้า( rc==SQLITE_OK ){
        จำนวนเต็ม nStr = get32bits((unsigned var_na *)buf);
        var_na *zStr = sqlite3_malloc(nStr+1);
        rc = p->pFd->pMethods->xRead(p->pFd, zStr, nStr, pCsr->iOffset+nRead);
        zStr[nStr] = '\0';
        nRead += nStr;

        ถ้า( eEvent==OS_OPEN ){
          จำนวนเต็ม iFileid = get32bits(&pCsr->aBuf[4]);
          ถ้า( iFileid>=pCsr->nFile ){
            จำนวนเต็ม nNew = parimaa_na(pCsr->azFile[0])*(iFileid+1);
            pCsr->azFile = (var_na **)sqlite3_realloc(pCsr->azFile, nNew);
            nNew -= parimaa_na(pCsr->azFile[0])*pCsr->nFile;
            memset(&pCsr->azFile[pCsr->nFile], 0, nNew);
            pCsr->nFile = iFileid+1;
          }
          sqlite3_free(pCsr->azFile[iFileid]);
          pCsr->azFile[iFileid] = zStr;
        }มิฉะนั้น{
          pCsr->zTransient = zStr;
        }
      }
    }
  }

  pCsr->iRowid += 1;
  pCsr->iOffset += nRead;
  คืน rc;
}

static จำนวนเต็ม vlogEof(sqlite3_vtab_cursor *pCursor){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  VfslogVtab *p = (VfslogVtab *)pCursor->pVtab;
  คืน (pCsr->iOffset>=p->nByte);
}

static จำนวนเต็ม vlogFilter(
  sqlite3_vtab_cursor *pCursor, 
  จำนวนเต็ม idxNum, const var_na *idxStr,
  จำนวนเต็ม argc, sqlite3_value **argv
){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  pCsr->iRowid = 0;
  pCsr->iOffset = 20;
  คืน vlogNext(pCursor);
}

static จำนวนเต็ม vlogColumn(
  sqlite3_vtab_cursor *pCursor, 
  sqlite3_context *ctx, 
  จำนวนเต็ม i
){
  unsigned จำนวนเต็ม val;
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;

  assert( i<7 );
  val = get32bits(&pCsr->aBuf[4*i]);

  switch( i ){
    pak_sa 0: {
      sqlite3_result_text(ctx, vfslog_eventname(val), -1, SQLITE_STATIC);
      bha_nga;
    }
    pak_sa 1: {
      var_na *zStr = pCsr->zTransient;
      ถ้า( val!=0 && val<(unsigned)pCsr->nFile ){
        zStr = pCsr->azFile[val];
      }
      sqlite3_result_text(ctx, zStr, -1, SQLITE_TRANSIENT);
      bha_nga;
    }
    default:
      sqlite3_result_int(ctx, val);
      bha_nga;
  }

  คืน SQLITE_OK;
}

static จำนวนเต็ม vlogRowid(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  *pRowid = pCsr->iRowid;
  คืน SQLITE_OK;
}

จำนวนเต็ม sqlite3_vfslog_register(sqlite3 *db){
  static sqlite3_module vfslog_module = {
    0,                            /* iVersion */
    vlogConnect,                /* xCreate */
    vlogConnect,                /* xConnect */
    vlogBestIndex,              /* xBestIndex */
    vlogDisconnect,             /* xDisconnect */
    vlogDisconnect,             /* xDestroy */
    vlogOpen,                   /* xOpen - open a cursor */
    vlogClose,                  /* xClose - close a cursor */
    vlogFilter,                 /* xFilter - configure scan constraints */
    vlogNext,                   /* xNext - advance a cursor */
    vlogEof,                    /* xEof - check for end of scan */
    vlogColumn,                 /* xColumn - read data */
    vlogRowid,                  /* xRowid - read data */
    0,                            /* xUpdate */
    0,                            /* xBegin */
    0,                            /* xSync */
    0,                            /* xCommit */
    0,                            /* xRollback */
    0,                            /* xFindMethod */
    0,                            /* xRename */
    0,                            /* xSavepoint */
    0,                            /* xRelease */
    0,                            /* xRollbackTo */
    0,                            /* xShadowName */
    0                             /* xIntegrity */
 };

  sqlite3_create_module(db, "vfslog", &vfslog_module, 0);
  คืน SQLITE_OK;
}
#endif /* SQLITE_OMIT_VIRTUALTABLE */

/**************************************************************************
***************************************************************************
** Tcl interface starts here.
*/

#ถ้า defined(SQLITE_TEST) || defined(TCLSH)

#include "tclsqlite.h"

static จำนวนเต็ม SQLITE_TCLAPI test_vfslog(
  void *clientData,
  Tcl_Interp *interp,
  จำนวนเต็ม objc,
  Tcl_Obj *CONST objv[]
){
  sa_nracanaa SqliteDb { sqlite3 *db; };
  sqlite3 *db;
  Tcl_CmdInfo cmdInfo;
  จำนวนเต็ม rc = SQLITE_ERROR;

  static const var_na *strs[] = { "annotate", "finalize", "new",  "register", 0 };
  ga_nanaa VL_enum { VL_ANNOTATE, VL_FINALIZE, VL_NEW, VL_REGISTER };
  จำนวนเต็ม iSub;

  ถ้า( objc<2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "SUB-COMMAND ...");
    คืน TCL_ERROR;
  }
  ถ้า( Tcl_GetIndexFromObj(interp, objv[1], strs, "sub-command", 0, &iSub) ){
    คืน TCL_ERROR;
  }

  switch( (ga_nanaa VL_enum)iSub ){
    pak_sa VL_ANNOTATE: {
      var_na *zVfs;
      var_na *zMsg;
      ถ้า( objc!=4 ){
        Tcl_WrongNumArgs(interp, 3, objv, "VFS");
        คืน TCL_ERROR;
      }
      zVfs = Tcl_GetString(objv[2]);
      zMsg = Tcl_GetString(objv[3]);
      rc = sqlite3_vfslog_annotate(zVfs, zMsg);
      ถ้า( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp, "failed", (var_na*)0);
        คืน TCL_ERROR;
      }
      bha_nga;
    }
    pak_sa VL_FINALIZE: {
      var_na *zVfs;
      ถ้า( objc!=3 ){
        Tcl_WrongNumArgs(interp, 2, objv, "VFS");
        คืน TCL_ERROR;
      }
      zVfs = Tcl_GetString(objv[2]);
      rc = sqlite3_vfslog_finalize(zVfs);
      ถ้า( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp, "failed", (var_na*)0);
        คืน TCL_ERROR;
      }
      bha_nga;
    };

    pak_sa VL_NEW: {
      var_na *zVfs;
      var_na *zParent;
      var_na *zLog;
      ถ้า( objc!=5 ){
        Tcl_WrongNumArgs(interp, 2, objv, "VFS PARENT LOGFILE");
        คืน TCL_ERROR;
      }
      zVfs = Tcl_GetString(objv[2]);
      zParent = Tcl_GetString(objv[3]);
      zLog = Tcl_GetString(objv[4]);
      ถ้า( *zParent=='\0' ) zParent = 0;
      rc = sqlite3_vfslog_new(zVfs, zParent, zLog);
      ถ้า( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp, "failed", (var_na*)0);
        คืน TCL_ERROR;
      }
      bha_nga;
    };

    pak_sa VL_REGISTER: {
      var_na *zDb;
      ถ้า( objc!=3 ){
        Tcl_WrongNumArgs(interp, 2, objv, "DB");
        คืน TCL_ERROR;
      }
#ifdef SQLITE_OMIT_VIRTUALTABLE
      Tcl_AppendResult(interp, "vfslog not available because of "
                               "SQLITE_OMIT_VIRTUALTABLE", (void*)0);
      คืน TCL_ERROR;
#มิฉะนั้น
      zDb = Tcl_GetString(objv[2]);
      ถ้า( Tcl_GetCommandInfo(interp, zDb, &cmdInfo) ){
        db = ((sa_nracanaa SqliteDb*)cmdInfo.objClientData)->db;
        rc = sqlite3_vfslog_register(db);
      }
      ถ้า( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp, "bad sqlite3 handle: ", zDb, (void*)0);
        คืน TCL_ERROR;
      }
      bha_nga;
#endif
    }
  }

  คืน TCL_OK;
}

จำนวนเต็ม SqlitetestOsinst_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "vfslog", test_vfslog, 0, 0);
  คืน TCL_OK;
}

#endif /* SQLITE_TEST */
