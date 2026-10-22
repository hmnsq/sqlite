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

typedef struct VfslogVfs VfslogVfs;
typedef struct VfslogFile VfslogFile;

struct VfslogVfs {
  sqlite3_vfs base;               /* VFS methods */
  sqlite3_vfs *pVfs;              /* Parent VFS */
  цяло iNextFileId;                /* Next file id */
  sqlite3_file *pLog;             /* Log file handle */
  sqlite3_int64 iOffset;          /* Log file offset of start of write buffer */
  цяло nBuf;                       /* Number of valid bytes in aBuf[] */
  char aBuf[VFSLOG_BUFFERSIZE];   /* Write buffer */
};

struct VfslogFile {
  sqlite3_file base;              /* IO methods */
  sqlite3_file *pReal;            /* Underlying file handle */
  sqlite3_vfs *pVfslog;           /* Associated VsflogVfs object */
  цяло iFileId;                    /* File id number */
};

#define REALVFS(p) (((VfslogVfs *)(p))->pVfs)



/*
** Method declarations for vfslog_file.
*/
static цяло vfslogClose(sqlite3_file*);
static цяло vfslogRead(sqlite3_file*, void*, цяло iAmt, sqlite3_int64 iOfst);
static цяло vfslogWrite(sqlite3_file*,const void*,цяло iAmt, sqlite3_int64 iOfst);
static цяло vfslogTruncate(sqlite3_file*, sqlite3_int64 size);
static цяло vfslogSync(sqlite3_file*, цяло flags);
static цяло vfslogFileSize(sqlite3_file*, sqlite3_int64 *pSize);
static цяло vfslogLock(sqlite3_file*, цяло);
static цяло vfslogUnlock(sqlite3_file*, цяло);
static цяло vfslogCheckReservedLock(sqlite3_file*, цяло *pResOut);
static цяло vfslogFileControl(sqlite3_file*, цяло op, void *pArg);
static цяло vfslogSectorSize(sqlite3_file*);
static цяло vfslogDeviceCharacteristics(sqlite3_file*);

static цяло vfslogShmLock(sqlite3_file *pFile, цяло ofst, цяло n, цяло flags);
static цяло vfslogShmMap(sqlite3_file *pFile,цяло,цяло,цяло,volatile void **);
static void vfslogShmBarrier(sqlite3_file*);
static цяло vfslogShmUnmap(sqlite3_file *pFile, цяло deleteFlag);

/*
** Method declarations for vfslog_vfs.
*/
static цяло vfslogOpen(sqlite3_vfs*, const char *, sqlite3_file*, цяло , цяло *);
static цяло vfslogDelete(sqlite3_vfs*, const char *zName, цяло syncDir);
static цяло vfslogAccess(sqlite3_vfs*, const char *zName, цяло flags, цяло *);
static цяло vfslogFullPathname(sqlite3_vfs*, const char *zName, цяло, char *zOut);
static void *vfslogDlOpen(sqlite3_vfs*, const char *zFilename);
static void vfslogDlError(sqlite3_vfs*, цяло nByte, char *zErrMsg);
static void (*vfslogDlSym(sqlite3_vfs *pVfs, void *p, const char*zSym))(void);
static void vfslogDlClose(sqlite3_vfs*, void*);
static цяло vfslogRandomness(sqlite3_vfs*, цяло nByte, char *zOut);
static цяло vfslogSleep(sqlite3_vfs*, цяло microseconds);
static цяло vfslogCurrentTime(sqlite3_vfs*, double*);

static цяло vfslogGetLastError(sqlite3_vfs*, цяло, char *);
static цяло vfslogCurrentTimeInt64(sqlite3_vfs*, sqlite3_int64*);

static sqlite3_vfs vfslog_vfs = {
  1,                              /* iVersion */
  sizeof(VfslogFile),             /* szOsFile */
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
  върни u64time /(sqlite3_uint64)10;
}
#elif !defined(NO_GETTOD)
#include <sys/time.h>
static sqlite3_uint64 vfslog_time(){
  struct timeval sTime;
  gettimeofday(&sTime, 0);
  върни sTime.tv_usec + (sqlite3_uint64)sTime.tv_sec * 1000000;
}
#иначе
static sqlite3_uint64 vfslog_time(){
  върни 0;
}
#endif

static void vfslog_call(sqlite3_vfs *, цяло, цяло, sqlite3_int64, цяло, цяло, цяло);
static void vfslog_string(sqlite3_vfs *, const char *);

/*
** Close an vfslog-file.
*/
static цяло vfslogClose(sqlite3_file *pFile){
  sqlite3_uint64 t;
  цяло rc = SQLITE_OK;
  VfslogFile *p = (VfslogFile *)pFile;

  t = vfslog_time();
  ако( p->pReal->pMethods ){
    rc = p->pReal->pMethods->xClose(p->pReal);
  }
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_CLOSE, p->iFileId, t, rc, 0, 0);
  върни rc;
}

/*
** Read data from an vfslog-file.
*/
static цяло vfslogRead(
  sqlite3_file *pFile, 
  void *zBuf, 
  цяло iAmt, 
  sqlite_int64 iOfst
){
  цяло rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xRead(p->pReal, zBuf, iAmt, iOfst);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_READ, p->iFileId, t, rc, iAmt, (цяло)iOfst);
  върни rc;
}

/*
** Write data to an vfslog-file.
*/
static цяло vfslogWrite(
  sqlite3_file *pFile,
  const void *z,
  цяло iAmt,
  sqlite_int64 iOfst
){
  цяло rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xWrite(p->pReal, z, iAmt, iOfst);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_WRITE, p->iFileId, t, rc, iAmt, (цяло)iOfst);
  върни rc;
}

/*
** Truncate an vfslog-file.
*/
static цяло vfslogTruncate(sqlite3_file *pFile, sqlite_int64 size){
  цяло rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xTruncate(p->pReal, size);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_TRUNCATE, p->iFileId, t, rc, 0, (цяло)size);
  върни rc;
}

/*
** Sync an vfslog-file.
*/
static цяло vfslogSync(sqlite3_file *pFile, цяло flags){
  цяло rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xSync(p->pReal, flags);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SYNC, p->iFileId, t, rc, flags, 0);
  върни rc;
}

/*
** Return the current file-size of an vfslog-file.
*/
static цяло vfslogFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  цяло rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xFileSize(p->pReal, pSize);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_FILESIZE, p->iFileId, t, rc, 0, (цяло)*pSize);
  върни rc;
}

/*
** Lock an vfslog-file.
*/
static цяло vfslogLock(sqlite3_file *pFile, цяло eLock){
  цяло rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xLock(p->pReal, eLock);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_LOCK, p->iFileId, t, rc, eLock, 0);
  върни rc;
}

/*
** Unlock an vfslog-file.
*/
static цяло vfslogUnlock(sqlite3_file *pFile, цяло eLock){
  цяло rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xUnlock(p->pReal, eLock);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_UNLOCK, p->iFileId, t, rc, eLock, 0);
  върни rc;
}

/*
** Check if another file-handle holds a RESERVED lock on an vfslog-file.
*/
static цяло vfslogCheckReservedLock(sqlite3_file *pFile, цяло *pResOut){
  цяло rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xCheckReservedLock(p->pReal, pResOut);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_CHECKRESERVEDLOCK, p->iFileId, t, rc, *pResOut, 0);
  върни rc;
}

/*
** File control method. For custom operations on an vfslog-file.
*/
static цяло vfslogFileControl(sqlite3_file *pFile, цяло op, void *pArg){
  VfslogFile *p = (VfslogFile *)pFile;
  цяло rc = p->pReal->pMethods->xFileControl(p->pReal, op, pArg);
  ако( op==SQLITE_FCNTL_VFSNAME && rc==SQLITE_OK ){
    *(char**)pArg = sqlite3_mprintf("vfslog/%z", *(char**)pArg);
  }
  върни rc;
}

/*
** Return the sector-size in bytes for an vfslog-file.
*/
static цяло vfslogSectorSize(sqlite3_file *pFile){
  цяло rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xSectorSize(p->pReal);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SECTORSIZE, p->iFileId, t, rc, 0, 0);
  върни rc;
}

/*
** Return the device characteristic flags supported by an vfslog-file.
*/
static цяло vfslogDeviceCharacteristics(sqlite3_file *pFile){
  цяло rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xDeviceCharacteristics(p->pReal);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_DEVCHAR, p->iFileId, t, rc, 0, 0);
  върни rc;
}

static цяло vfslogShmLock(sqlite3_file *pFile, цяло ofst, цяло n, цяло flags){
  цяло rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xShmLock(p->pReal, ofst, n, flags);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SHMLOCK, p->iFileId, t, rc, 0, 0);
  върни rc;
}
static цяло vfslogShmMap(
  sqlite3_file *pFile, 
  цяло iRegion, 
  цяло szRegion, 
  цяло isWrite, 
  volatile void **pp
){
  цяло rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xShmMap(p->pReal, iRegion, szRegion, isWrite, pp);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SHMMAP, p->iFileId, t, rc, 0, 0);
  върни rc;
}
static void vfslogShmBarrier(sqlite3_file *pFile){
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  p->pReal->pMethods->xShmBarrier(p->pReal);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SHMBARRIER, p->iFileId, t, SQLITE_OK, 0, 0);
}
static цяло vfslogShmUnmap(sqlite3_file *pFile, цяло deleteFlag){
  цяло rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xShmUnmap(p->pReal, deleteFlag);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SHMUNMAP, p->iFileId, t, rc, 0, 0);
  върни rc;
}


/*
** Open an vfslog file handle.
*/
static цяло vfslogOpen(
  sqlite3_vfs *pVfs,
  const char *zName,
  sqlite3_file *pFile,
  цяло flags,
  цяло *pOutFlags
){
  цяло rc;
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
  върни rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
static цяло vfslogDelete(sqlite3_vfs *pVfs, const char *zPath, цяло dirSync){
  цяло rc;
  sqlite3_uint64 t;
  t = vfslog_time();
  rc = REALVFS(pVfs)->xDelete(REALVFS(pVfs), zPath, dirSync);
  t = vfslog_time() - t;
  vfslog_call(pVfs, OS_DELETE, 0, t, rc, dirSync, 0);
  vfslog_string(pVfs, zPath);
  върни rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
static цяло vfslogAccess(
  sqlite3_vfs *pVfs, 
  const char *zPath, 
  цяло flags, 
  цяло *pResOut
){
  цяло rc;
  sqlite3_uint64 t;
  t = vfslog_time();
  rc = REALVFS(pVfs)->xAccess(REALVFS(pVfs), zPath, flags, pResOut);
  t = vfslog_time() - t;
  vfslog_call(pVfs, OS_ACCESS, 0, t, rc, flags, *pResOut);
  vfslog_string(pVfs, zPath);
  върни rc;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
static цяло vfslogFullPathname(
  sqlite3_vfs *pVfs, 
  const char *zPath, 
  цяло nOut, 
  char *zOut
){
  върни REALVFS(pVfs)->xFullPathname(REALVFS(pVfs), zPath, nOut, zOut);
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
static void *vfslogDlOpen(sqlite3_vfs *pVfs, const char *zPath){
  върни REALVFS(pVfs)->xDlOpen(REALVFS(pVfs), zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
static void vfslogDlError(sqlite3_vfs *pVfs, цяло nByte, char *zErrMsg){
  REALVFS(pVfs)->xDlError(REALVFS(pVfs), nByte, zErrMsg);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
static void (*vfslogDlSym(sqlite3_vfs *pVfs, void *p, const char *zSym))(void){
  върни REALVFS(pVfs)->xDlSym(REALVFS(pVfs), p, zSym);
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
static цяло vfslogRandomness(sqlite3_vfs *pVfs, цяло nByte, char *zBufOut){
  върни REALVFS(pVfs)->xRandomness(REALVFS(pVfs), nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
static цяло vfslogSleep(sqlite3_vfs *pVfs, цяло nMicro){
  върни REALVFS(pVfs)->xSleep(REALVFS(pVfs), nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
static цяло vfslogCurrentTime(sqlite3_vfs *pVfs, double *pTimeOut){
  върни REALVFS(pVfs)->xCurrentTime(REALVFS(pVfs), pTimeOut);
}

static цяло vfslogGetLastError(sqlite3_vfs *pVfs, цяло a, char *b){
  върни REALVFS(pVfs)->xGetLastError(REALVFS(pVfs), a, b);
}
static цяло vfslogCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *p){
  върни REALVFS(pVfs)->xCurrentTimeInt64(REALVFS(pVfs), p);
}

static void vfslog_flush(VfslogVfs *p){
#ifdef SQLITE_TEST
  extern цяло sqlite3_io_error_pending;
  extern цяло sqlite3_io_error_persist;
  extern цяло sqlite3_diskfull_pending;

  цяло pending = sqlite3_io_error_pending;
  цяло persist = sqlite3_io_error_persist;
  цяло diskfull = sqlite3_diskfull_pending;

  sqlite3_io_error_pending = 0;
  sqlite3_io_error_persist = 0;
  sqlite3_diskfull_pending = 0;
#endif

  ако( p->nBuf ){
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

static void put32bits(unsigned char *p, unsigned цяло v){
  p[0] = v>>24;
  p[1] = (unsigned char)(v>>16);
  p[2] = (unsigned char)(v>>8);
  p[3] = (unsigned char)v;
}

static void vfslog_call(
  sqlite3_vfs *pVfs,
  цяло eEvent,
  цяло iFileid,
  sqlite3_int64 nClick,
  цяло return_code,
  цяло size,
  цяло offset
){
  VfslogVfs *p = (VfslogVfs *)pVfs;
  unsigned char *zRec;
  ако( (24+p->nBuf)>sizeof(p->aBuf) ){
    vfslog_flush(p);
  }
  zRec = (unsigned char *)&p->aBuf[p->nBuf];
  put32bits(&zRec[0], eEvent);
  put32bits(&zRec[4], iFileid);
  put32bits(&zRec[8], (unsigned цяло)(nClick&0xffff));
  put32bits(&zRec[12], return_code);
  put32bits(&zRec[16], size);
  put32bits(&zRec[20], offset);
  p->nBuf += 24;
}

static void vfslog_string(sqlite3_vfs *pVfs, const char *zStr){
  VfslogVfs *p = (VfslogVfs *)pVfs;
  unsigned char *zRec;
  цяло nStr = zStr ? (цяло)strlen(zStr) : 0;
  ако( (4+nStr+p->nBuf)>sizeof(p->aBuf) ){
    vfslog_flush(p);
  }
  zRec = (unsigned char *)&p->aBuf[p->nBuf];
  put32bits(&zRec[0], nStr);
  ако( zStr ){
    memcpy(&zRec[4], zStr, nStr);
  }
  p->nBuf += (4 + nStr);
}

static void vfslog_finalize(VfslogVfs *p){
  ако( p->pLog->pMethods ){
    vfslog_flush(p);
    p->pLog->pMethods->xClose(p->pLog);
  }
  sqlite3_free(p);
}

цяло sqlite3_vfslog_finalize(const char *zVfs){
  sqlite3_vfs *pVfs;
  pVfs = sqlite3_vfs_find(zVfs);
  ако( !pVfs || pVfs->xOpen!=vfslogOpen ){
    върни SQLITE_ERROR;
  } 
  sqlite3_vfs_unregister(pVfs);
  vfslog_finalize((VfslogVfs *)pVfs);
  върни SQLITE_OK;
}

цяло sqlite3_vfslog_new(
  const char *zVfs,               /* New VFS name */
  const char *zParentVfs,         /* Parent VFS name (or NULL) */
  const char *zLog                /* Log file name */
){
  VfslogVfs *p;
  sqlite3_vfs *pParent;
  цяло nByte;
  цяло flags;
  цяло rc;
  char *zFile;
  цяло nVfs;

  pParent = sqlite3_vfs_find(zParentVfs);
  ако( !pParent ){
    върни SQLITE_ERROR;
  }

  nVfs = (цяло)strlen(zVfs);
  nByte = sizeof(VfslogVfs) + pParent->szOsFile + nVfs+1+pParent->mxPathname+1;
  p = (VfslogVfs *)sqlite3_malloc(nByte);
  memset(p, 0, nByte);

  p->pVfs = pParent;
  p->pLog = (sqlite3_file *)&p[1];
  memcpy(&p->base, &vfslog_vfs, sizeof(sqlite3_vfs));
  p->base.zName = &((char *)p->pLog)[pParent->szOsFile];
  p->base.szOsFile += pParent->szOsFile;
  memcpy((char *)p->base.zName, zVfs, nVfs);

  zFile = (char *)&p->base.zName[nVfs+1];
  pParent->xFullPathname(pParent, zLog, pParent->mxPathname, zFile);

  flags = SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE|SQLITE_OPEN_SUPER_JOURNAL;
  pParent->xDelete(pParent, zFile, 0);
  rc = pParent->xOpen(pParent, zFile, p->pLog, flags, &flags);
  ако( rc==SQLITE_OK ){
    memcpy(p->aBuf, "sqlite_ostrace1.....", 20);
    p->iOffset = 0;
    p->nBuf = 20;
    rc = sqlite3_vfs_register((sqlite3_vfs *)p, 1);
  }
  ако( rc ){
    vfslog_finalize(p);
  }
  върни rc;
}

цяло sqlite3_vfslog_annotate(const char *zVfs, const char *zMsg){
  sqlite3_vfs *pVfs;
  pVfs = sqlite3_vfs_find(zVfs);
  ако( !pVfs || pVfs->xOpen!=vfslogOpen ){
    върни SQLITE_ERROR;
  } 
  vfslog_call(pVfs, OS_ANNOTATE, 0, 0, 0, 0, 0);
  vfslog_string(pVfs, zMsg);
  върни SQLITE_OK;
}

static const char *vfslog_eventname(цяло eEvent){
  const char *zEvent = 0;

  switch( eEvent ){
    case OS_CLOSE:             zEvent = "xClose"; break;
    case OS_READ:              zEvent = "xRead"; break;
    case OS_WRITE:             zEvent = "xWrite"; break;
    case OS_TRUNCATE:          zEvent = "xTruncate"; break;
    case OS_SYNC:              zEvent = "xSync"; break;
    case OS_FILESIZE:          zEvent = "xFilesize"; break;
    case OS_LOCK:              zEvent = "xLock"; break;
    case OS_UNLOCK:            zEvent = "xUnlock"; break;
    case OS_CHECKRESERVEDLOCK: zEvent = "xCheckResLock"; break;
    case OS_FILECONTROL:       zEvent = "xFileControl"; break;
    case OS_SECTORSIZE:        zEvent = "xSectorSize"; break;
    case OS_DEVCHAR:           zEvent = "xDeviceChar"; break;
    case OS_OPEN:              zEvent = "xOpen"; break;
    case OS_DELETE:            zEvent = "xDelete"; break;
    case OS_ACCESS:            zEvent = "xAccess"; break;
    case OS_FULLPATHNAME:      zEvent = "xFullPathname"; break;
    case OS_RANDOMNESS:        zEvent = "xRandomness"; break;
    case OS_SLEEP:             zEvent = "xSleep"; break;
    case OS_CURRENTTIME:       zEvent = "xCurrentTime"; break;

    case OS_SHMUNMAP:          zEvent = "xShmUnmap"; break;
    case OS_SHMLOCK:           zEvent = "xShmLock"; break;
    case OS_SHMBARRIER:        zEvent = "xShmBarrier"; break;
    case OS_SHMMAP:            zEvent = "xShmMap"; break;

    case OS_ANNOTATE:          zEvent = "annotation"; break;
  }

  върни zEvent;
}

typedef struct VfslogVtab VfslogVtab;
typedef struct VfslogCsr VfslogCsr;

/*
** Virtual table type for the vfslog reader module.
*/
struct VfslogVtab {
  sqlite3_vtab base;              /* Base class */
  sqlite3_file *pFd;              /* File descriptor open on vfslog file */
  sqlite3_int64 nByte;            /* Size of file in bytes */
  char *zFile;                    /* File name for pFd */
};

/*
** Virtual table cursor type for the vfslog reader module.
*/
struct VfslogCsr {
  sqlite3_vtab_cursor base;       /* Base class */
  sqlite3_int64 iRowid;           /* Current rowid. */
  sqlite3_int64 iOffset;          /* Offset of next record in file */
  char *zTransient;               /* Transient 'file' string */
  цяло nFile;                      /* Size of array azFile[] */
  char **azFile;                  /* File strings */
  unsigned char aBuf[1024];       /* Current vfs log entry (read from file) */
};

static unsigned цяло get32bits(unsigned char *p){
  върни (p[0]<<24) + (p[1]<<16) + (p[2]<<8) + p[3];
}

/*
** The argument must point to a buffer containing a nul-terminated string.
** If the string begins with an SQL quote character it is overwritten by
** the dequoted version. Otherwise the buffer is left unmodified.
*/
static void dequote(char *z){
  char quote;                     /* Quote character (if any ) */
  quote = z[0];
  ако( quote=='[' || quote=='\'' || quote=='"' || quote=='`' ){
    цяло iIn = 1;                  /* Index of next byte to read from input */
    цяло iOut = 0;                 /* Index of next byte to write to output */
    ако( quote=='[' ) quote = ']';  
    докато( z[iIn] ){
      ако( z[iIn]==quote ){
        ако( z[iIn+1]!=quote ) break;
        z[iOut++] = quote;
        iIn += 2;
      }иначе{
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
static цяло vlogConnect(
  sqlite3 *db,
  void *pAux,
  цяло argc, const char *const*argv,
  sqlite3_vtab **ppVtab,
  char **pzErr
){
  sqlite3_vfs *pVfs;              /* VFS used to read log file */
  цяло flags;                      /* flags passed to pVfs->xOpen() */
  VfslogVtab *p;
  цяло rc;
  цяло nByte;
  char *zFile;

  *ppVtab = 0;
  pVfs = sqlite3_vfs_find(0);
  nByte = sizeof(VfslogVtab) + pVfs->szOsFile + pVfs->mxPathname;
  p = sqlite3_malloc(nByte);
  ако( p==0 ) върни SQLITE_NOMEM;
  memset(p, 0, nByte);

  p->pFd = (sqlite3_file *)&p[1];
  p->zFile = &((char *)p->pFd)[pVfs->szOsFile];

  zFile = sqlite3_mprintf("%s", argv[3]);
  ако( !zFile ){
    sqlite3_free(p);
    върни SQLITE_NOMEM;
  }
  dequote(zFile);
  pVfs->xFullPathname(pVfs, zFile, pVfs->mxPathname, p->zFile);
  sqlite3_free(zFile);

  flags = SQLITE_OPEN_READWRITE|SQLITE_OPEN_SUPER_JOURNAL;
  rc = pVfs->xOpen(pVfs, p->zFile, p->pFd, flags, &flags);

  ако( rc==SQLITE_OK ){
    p->pFd->pMethods->xFileSize(p->pFd, &p->nByte);
    sqlite3_declare_vtab(db, 
        "CREATE TABLE xxx(event, file, click, rc, size, offset)"
    );
    *ppVtab = &p->base;
  }иначе{
    sqlite3_free(p);
  }

  върни rc;
}

/*
** There is no "best-index". This virtual table always does a linear
** scan of the binary VFS log file.
*/
static цяло vlogBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  pIdxInfo->estimatedCost = 10.0;
  върни SQLITE_OK;
}

/*
** Disconnect from or destroy a vfslog virtual table.
*/
static цяло vlogDisconnect(sqlite3_vtab *pVtab){
  VfslogVtab *p = (VfslogVtab *)pVtab;
  ако( p->pFd->pMethods ){
    p->pFd->pMethods->xClose(p->pFd);
    p->pFd->pMethods = 0;
  }
  sqlite3_free(p);
  върни SQLITE_OK;
}

/*
** Open a new vfslog cursor.
*/
static цяло vlogOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  VfslogCsr *pCsr;                /* Newly allocated cursor object */

  pCsr = sqlite3_malloc(sizeof(VfslogCsr));
  ако( !pCsr ) върни SQLITE_NOMEM;
  memset(pCsr, 0, sizeof(VfslogCsr));
  *ppCursor = &pCsr->base;
  върни SQLITE_OK;
}

/*
** Close a vfslog cursor.
*/
static цяло vlogClose(sqlite3_vtab_cursor *pCursor){
  VfslogCsr *p = (VfslogCsr *)pCursor;
  цяло i;
  за(i=0; i<p->nFile; i++){
    sqlite3_free(p->azFile[i]);
  }
  sqlite3_free(p->azFile);
  sqlite3_free(p->zTransient);
  sqlite3_free(p);
  върни SQLITE_OK;
}

/*
** Move a vfslog cursor to the next entry in the file.
*/
static цяло vlogNext(sqlite3_vtab_cursor *pCursor){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  VfslogVtab *p = (VfslogVtab *)pCursor->pVtab;
  цяло rc = SQLITE_OK;
  цяло nRead;

  sqlite3_free(pCsr->zTransient);
  pCsr->zTransient = 0;

  nRead = 24;
  ако( pCsr->iOffset+nRead<=p->nByte ){
    цяло eEvent;
    rc = p->pFd->pMethods->xRead(p->pFd, pCsr->aBuf, nRead, pCsr->iOffset);

    eEvent = get32bits(pCsr->aBuf);
    ако( (rc==SQLITE_OK)
     && (eEvent==OS_OPEN || eEvent==OS_DELETE || eEvent==OS_ACCESS) 
    ){
      char buf[4];
      rc = p->pFd->pMethods->xRead(p->pFd, buf, 4, pCsr->iOffset+nRead);
      nRead += 4;
      ако( rc==SQLITE_OK ){
        цяло nStr = get32bits((unsigned char *)buf);
        char *zStr = sqlite3_malloc(nStr+1);
        rc = p->pFd->pMethods->xRead(p->pFd, zStr, nStr, pCsr->iOffset+nRead);
        zStr[nStr] = '\0';
        nRead += nStr;

        ако( eEvent==OS_OPEN ){
          цяло iFileid = get32bits(&pCsr->aBuf[4]);
          ако( iFileid>=pCsr->nFile ){
            цяло nNew = sizeof(pCsr->azFile[0])*(iFileid+1);
            pCsr->azFile = (char **)sqlite3_realloc(pCsr->azFile, nNew);
            nNew -= sizeof(pCsr->azFile[0])*pCsr->nFile;
            memset(&pCsr->azFile[pCsr->nFile], 0, nNew);
            pCsr->nFile = iFileid+1;
          }
          sqlite3_free(pCsr->azFile[iFileid]);
          pCsr->azFile[iFileid] = zStr;
        }иначе{
          pCsr->zTransient = zStr;
        }
      }
    }
  }

  pCsr->iRowid += 1;
  pCsr->iOffset += nRead;
  върни rc;
}

static цяло vlogEof(sqlite3_vtab_cursor *pCursor){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  VfslogVtab *p = (VfslogVtab *)pCursor->pVtab;
  върни (pCsr->iOffset>=p->nByte);
}

static цяло vlogFilter(
  sqlite3_vtab_cursor *pCursor, 
  цяло idxNum, const char *idxStr,
  цяло argc, sqlite3_value **argv
){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  pCsr->iRowid = 0;
  pCsr->iOffset = 20;
  върни vlogNext(pCursor);
}

static цяло vlogColumn(
  sqlite3_vtab_cursor *pCursor, 
  sqlite3_context *ctx, 
  цяло i
){
  unsigned цяло val;
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;

  assert( i<7 );
  val = get32bits(&pCsr->aBuf[4*i]);

  switch( i ){
    case 0: {
      sqlite3_result_text(ctx, vfslog_eventname(val), -1, SQLITE_STATIC);
      break;
    }
    case 1: {
      char *zStr = pCsr->zTransient;
      ако( val!=0 && val<(unsigned)pCsr->nFile ){
        zStr = pCsr->azFile[val];
      }
      sqlite3_result_text(ctx, zStr, -1, SQLITE_TRANSIENT);
      break;
    }
    default:
      sqlite3_result_int(ctx, val);
      break;
  }

  върни SQLITE_OK;
}

static цяло vlogRowid(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  *pRowid = pCsr->iRowid;
  върни SQLITE_OK;
}

цяло sqlite3_vfslog_register(sqlite3 *db){
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
  върни SQLITE_OK;
}
#endif /* SQLITE_OMIT_VIRTUALTABLE */

/**************************************************************************
***************************************************************************
** Tcl interface starts here.
*/

#ако defined(SQLITE_TEST) || defined(TCLSH)

#include "tclsqlite.h"

static цяло SQLITE_TCLAPI test_vfslog(
  void *clientData,
  Tcl_Interp *interp,
  цяло objc,
  Tcl_Obj *CONST objv[]
){
  struct SqliteDb { sqlite3 *db; };
  sqlite3 *db;
  Tcl_CmdInfo cmdInfo;
  цяло rc = SQLITE_ERROR;

  static const char *strs[] = { "annotate", "finalize", "new",  "register", 0 };
  enum VL_enum { VL_ANNOTATE, VL_FINALIZE, VL_NEW, VL_REGISTER };
  цяло iSub;

  ако( objc<2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "SUB-COMMAND ...");
    върни TCL_ERROR;
  }
  ако( Tcl_GetIndexFromObj(interp, objv[1], strs, "sub-command", 0, &iSub) ){
    върни TCL_ERROR;
  }

  switch( (enum VL_enum)iSub ){
    case VL_ANNOTATE: {
      char *zVfs;
      char *zMsg;
      ако( objc!=4 ){
        Tcl_WrongNumArgs(interp, 3, objv, "VFS");
        върни TCL_ERROR;
      }
      zVfs = Tcl_GetString(objv[2]);
      zMsg = Tcl_GetString(objv[3]);
      rc = sqlite3_vfslog_annotate(zVfs, zMsg);
      ако( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp, "failed", (char*)0);
        върни TCL_ERROR;
      }
      break;
    }
    case VL_FINALIZE: {
      char *zVfs;
      ако( objc!=3 ){
        Tcl_WrongNumArgs(interp, 2, objv, "VFS");
        върни TCL_ERROR;
      }
      zVfs = Tcl_GetString(objv[2]);
      rc = sqlite3_vfslog_finalize(zVfs);
      ако( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp, "failed", (char*)0);
        върни TCL_ERROR;
      }
      break;
    };

    case VL_NEW: {
      char *zVfs;
      char *zParent;
      char *zLog;
      ако( objc!=5 ){
        Tcl_WrongNumArgs(interp, 2, objv, "VFS PARENT LOGFILE");
        върни TCL_ERROR;
      }
      zVfs = Tcl_GetString(objv[2]);
      zParent = Tcl_GetString(objv[3]);
      zLog = Tcl_GetString(objv[4]);
      ако( *zParent=='\0' ) zParent = 0;
      rc = sqlite3_vfslog_new(zVfs, zParent, zLog);
      ако( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp, "failed", (char*)0);
        върни TCL_ERROR;
      }
      break;
    };

    case VL_REGISTER: {
      char *zDb;
      ако( objc!=3 ){
        Tcl_WrongNumArgs(interp, 2, objv, "DB");
        върни TCL_ERROR;
      }
#ifdef SQLITE_OMIT_VIRTUALTABLE
      Tcl_AppendResult(interp, "vfslog not available because of "
                               "SQLITE_OMIT_VIRTUALTABLE", (void*)0);
      върни TCL_ERROR;
#иначе
      zDb = Tcl_GetString(objv[2]);
      ако( Tcl_GetCommandInfo(interp, zDb, &cmdInfo) ){
        db = ((struct SqliteDb*)cmdInfo.objClientData)->db;
        rc = sqlite3_vfslog_register(db);
      }
      ако( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp, "bad sqlite3 handle: ", zDb, (void*)0);
        върни TCL_ERROR;
      }
      break;
#endif
    }
  }

  върни TCL_OK;
}

цяло SqlitetestOsinst_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "vfslog", test_vfslog, 0, 0);
  върни TCL_OK;
}

#endif /* SQLITE_TEST */
