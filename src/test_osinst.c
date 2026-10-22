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

#ضمن "sqlite3.h"

#ifdef _WIN32
#ضمن <windows.h>
#endif

#ضمن <string.h>
#ضمن <assert.h>


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

تعريفنوع بنية VfslogVfs VfslogVfs;
تعريفنوع بنية VfslogFile VfslogFile;

بنية VfslogVfs {
  sqlite3_vfs base;               /* VFS methods */
  sqlite3_vfs *pVfs;              /* Parent VFS */
  صحيح iNextFileId;                /* Next file id */
  sqlite3_file *pLog;             /* Log file handle */
  sqlite3_int64 iOffset;          /* Log file offset of start of write buffer */
  صحيح nBuf;                       /* Number of valid bytes in aBuf[] */
  حرف aBuf[VFSLOG_BUFFERSIZE];   /* Write buffer */
};

بنية VfslogFile {
  sqlite3_file base;              /* IO methods */
  sqlite3_file *pReal;            /* Underlying file handle */
  sqlite3_vfs *pVfslog;           /* Associated VsflogVfs object */
  صحيح iFileId;                    /* File id number */
};

#define REALVFS(p) (((VfslogVfs *)(p))->pVfs)



/*
** Method declarations for vfslog_file.
*/
ساكن صحيح vfslogClose(sqlite3_file*);
ساكن صحيح vfslogRead(sqlite3_file*, فراغ*, صحيح iAmt, sqlite3_int64 iOfst);
ساكن صحيح vfslogWrite(sqlite3_file*,ثابت فراغ*,صحيح iAmt, sqlite3_int64 iOfst);
ساكن صحيح vfslogTruncate(sqlite3_file*, sqlite3_int64 size);
ساكن صحيح vfslogSync(sqlite3_file*, صحيح flags);
ساكن صحيح vfslogFileSize(sqlite3_file*, sqlite3_int64 *pSize);
ساكن صحيح vfslogLock(sqlite3_file*, صحيح);
ساكن صحيح vfslogUnlock(sqlite3_file*, صحيح);
ساكن صحيح vfslogCheckReservedLock(sqlite3_file*, صحيح *pResOut);
ساكن صحيح vfslogFileControl(sqlite3_file*, صحيح op, فراغ *pArg);
ساكن صحيح vfslogSectorSize(sqlite3_file*);
ساكن صحيح vfslogDeviceCharacteristics(sqlite3_file*);

ساكن صحيح vfslogShmLock(sqlite3_file *pFile, صحيح ofst, صحيح n, صحيح flags);
ساكن صحيح vfslogShmMap(sqlite3_file *pFile,صحيح,صحيح,صحيح,متطاير فراغ **);
ساكن فراغ vfslogShmBarrier(sqlite3_file*);
ساكن صحيح vfslogShmUnmap(sqlite3_file *pFile, صحيح deleteFlag);

/*
** Method declarations for vfslog_vfs.
*/
ساكن صحيح vfslogOpen(sqlite3_vfs*, ثابت حرف *, sqlite3_file*, صحيح , صحيح *);
ساكن صحيح vfslogDelete(sqlite3_vfs*, ثابت حرف *zName, صحيح syncDir);
ساكن صحيح vfslogAccess(sqlite3_vfs*, ثابت حرف *zName, صحيح flags, صحيح *);
ساكن صحيح vfslogFullPathname(sqlite3_vfs*, ثابت حرف *zName, صحيح, حرف *zOut);
ساكن فراغ *vfslogDlOpen(sqlite3_vfs*, ثابت حرف *zFilename);
ساكن فراغ vfslogDlError(sqlite3_vfs*, صحيح nByte, حرف *zErrMsg);
ساكن فراغ (*vfslogDlSym(sqlite3_vfs *pVfs, فراغ *p, ثابت حرف*zSym))(فراغ);
ساكن فراغ vfslogDlClose(sqlite3_vfs*, فراغ*);
ساكن صحيح vfslogRandomness(sqlite3_vfs*, صحيح nByte, حرف *zOut);
ساكن صحيح vfslogSleep(sqlite3_vfs*, صحيح microseconds);
ساكن صحيح vfslogCurrentTime(sqlite3_vfs*, مزدوج*);

ساكن صحيح vfslogGetLastError(sqlite3_vfs*, صحيح, حرف *);
ساكن صحيح vfslogCurrentTimeInt64(sqlite3_vfs*, sqlite3_int64*);

ساكن sqlite3_vfs vfslog_vfs = {
  1,                              /* iVersion */
  حجم(VfslogFile),             /* szOsFile */
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

ساكن sqlite3_io_methods vfslog_io_methods = {
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
#ضمن <time.h>
ساكن sqlite3_uint64 vfslog_time(){
  FILETIME ft;
  sqlite3_uint64 u64time = 0;
 
  GetSystemTimeAsFileTime(&ft);

  u64time |= ft.dwHighDateTime;
  u64time <<= 32;
  u64time |= ft.dwLowDateTime;

  /* ft is 100-nanosecond intervals, we want microseconds */
  החזר u64time /(sqlite3_uint64)10;
}
#وإلاإذا !defined(NO_GETTOD)
#ضمن <sys/time.h>
ساكن sqlite3_uint64 vfslog_time(){
  بنية timeval sTime;
  gettimeofday(&sTime, 0);
  החזר sTime.tv_usec + (sqlite3_uint64)sTime.tv_sec * 1000000;
}
#אחרת
ساكن sqlite3_uint64 vfslog_time(){
  החזר 0;
}
#endif

ساكن فراغ vfslog_call(sqlite3_vfs *, صحيح, صحيح, sqlite3_int64, صحيح, صحيح, صحيح);
ساكن فراغ vfslog_string(sqlite3_vfs *, ثابت حرف *);

/*
** Close an vfslog-file.
*/
ساكن صحيح vfslogClose(sqlite3_file *pFile){
  sqlite3_uint64 t;
  صحيح rc = SQLITE_OK;
  VfslogFile *p = (VfslogFile *)pFile;

  t = vfslog_time();
  אם( p->pReal->pMethods ){
    rc = p->pReal->pMethods->xClose(p->pReal);
  }
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_CLOSE, p->iFileId, t, rc, 0, 0);
  החזר rc;
}

/*
** Read data from an vfslog-file.
*/
ساكن صحيح vfslogRead(
  sqlite3_file *pFile, 
  فراغ *zBuf, 
  صحيح iAmt, 
  sqlite_int64 iOfst
){
  صحيح rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xRead(p->pReal, zBuf, iAmt, iOfst);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_READ, p->iFileId, t, rc, iAmt, (صحيح)iOfst);
  החזר rc;
}

/*
** Write data to an vfslog-file.
*/
ساكن صحيح vfslogWrite(
  sqlite3_file *pFile,
  ثابت فراغ *z,
  صحيح iAmt,
  sqlite_int64 iOfst
){
  صحيح rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xWrite(p->pReal, z, iAmt, iOfst);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_WRITE, p->iFileId, t, rc, iAmt, (صحيح)iOfst);
  החזר rc;
}

/*
** Truncate an vfslog-file.
*/
ساكن صحيح vfslogTruncate(sqlite3_file *pFile, sqlite_int64 size){
  صحيح rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xTruncate(p->pReal, size);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_TRUNCATE, p->iFileId, t, rc, 0, (صحيح)size);
  החזר rc;
}

/*
** Sync an vfslog-file.
*/
ساكن صحيح vfslogSync(sqlite3_file *pFile, صحيح flags){
  صحيح rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xSync(p->pReal, flags);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SYNC, p->iFileId, t, rc, flags, 0);
  החזר rc;
}

/*
** Return the current file-size of an vfslog-file.
*/
ساكن صحيح vfslogFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  صحيح rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xFileSize(p->pReal, pSize);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_FILESIZE, p->iFileId, t, rc, 0, (صحيح)*pSize);
  החזר rc;
}

/*
** Lock an vfslog-file.
*/
ساكن صحيح vfslogLock(sqlite3_file *pFile, صحيح eLock){
  صحيح rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xLock(p->pReal, eLock);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_LOCK, p->iFileId, t, rc, eLock, 0);
  החזר rc;
}

/*
** Unlock an vfslog-file.
*/
ساكن صحيح vfslogUnlock(sqlite3_file *pFile, صحيح eLock){
  صحيح rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xUnlock(p->pReal, eLock);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_UNLOCK, p->iFileId, t, rc, eLock, 0);
  החזר rc;
}

/*
** Check if another file-handle holds a RESERVED lock on an vfslog-file.
*/
ساكن صحيح vfslogCheckReservedLock(sqlite3_file *pFile, صحيح *pResOut){
  صحيح rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xCheckReservedLock(p->pReal, pResOut);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_CHECKRESERVEDLOCK, p->iFileId, t, rc, *pResOut, 0);
  החזר rc;
}

/*
** File control method. For custom operations on an vfslog-file.
*/
ساكن صحيح vfslogFileControl(sqlite3_file *pFile, صحيح op, فراغ *pArg){
  VfslogFile *p = (VfslogFile *)pFile;
  صحيح rc = p->pReal->pMethods->xFileControl(p->pReal, op, pArg);
  אם( op==SQLITE_FCNTL_VFSNAME && rc==SQLITE_OK ){
    *(حرف**)pArg = sqlite3_mprintf("vfslog/%z", *(حرف**)pArg);
  }
  החזר rc;
}

/*
** Return the sector-size in bytes for an vfslog-file.
*/
ساكن صحيح vfslogSectorSize(sqlite3_file *pFile){
  صحيح rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xSectorSize(p->pReal);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SECTORSIZE, p->iFileId, t, rc, 0, 0);
  החזר rc;
}

/*
** Return the device characteristic flags supported by an vfslog-file.
*/
ساكن صحيح vfslogDeviceCharacteristics(sqlite3_file *pFile){
  صحيح rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xDeviceCharacteristics(p->pReal);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_DEVCHAR, p->iFileId, t, rc, 0, 0);
  החזר rc;
}

ساكن صحيح vfslogShmLock(sqlite3_file *pFile, صحيح ofst, صحيح n, صحيح flags){
  صحيح rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xShmLock(p->pReal, ofst, n, flags);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SHMLOCK, p->iFileId, t, rc, 0, 0);
  החזר rc;
}
ساكن صحيح vfslogShmMap(
  sqlite3_file *pFile, 
  صحيح iRegion, 
  صحيح szRegion, 
  صحيح isWrite, 
  متطاير فراغ **pp
){
  صحيح rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xShmMap(p->pReal, iRegion, szRegion, isWrite, pp);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SHMMAP, p->iFileId, t, rc, 0, 0);
  החזר rc;
}
ساكن فراغ vfslogShmBarrier(sqlite3_file *pFile){
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  p->pReal->pMethods->xShmBarrier(p->pReal);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SHMBARRIER, p->iFileId, t, SQLITE_OK, 0, 0);
}
ساكن صحيح vfslogShmUnmap(sqlite3_file *pFile, صحيح deleteFlag){
  صحيح rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xShmUnmap(p->pReal, deleteFlag);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SHMUNMAP, p->iFileId, t, rc, 0, 0);
  החזר rc;
}


/*
** Open an vfslog file handle.
*/
ساكن صحيح vfslogOpen(
  sqlite3_vfs *pVfs,
  ثابت حرف *zName,
  sqlite3_file *pFile,
  صحيح flags,
  صحيح *pOutFlags
){
  صحيح rc;
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
  החזר rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
ساكن صحيح vfslogDelete(sqlite3_vfs *pVfs, ثابت حرف *zPath, صحيح dirSync){
  صحيح rc;
  sqlite3_uint64 t;
  t = vfslog_time();
  rc = REALVFS(pVfs)->xDelete(REALVFS(pVfs), zPath, dirSync);
  t = vfslog_time() - t;
  vfslog_call(pVfs, OS_DELETE, 0, t, rc, dirSync, 0);
  vfslog_string(pVfs, zPath);
  החזר rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
ساكن صحيح vfslogAccess(
  sqlite3_vfs *pVfs, 
  ثابت حرف *zPath, 
  صحيح flags, 
  صحيح *pResOut
){
  صحيح rc;
  sqlite3_uint64 t;
  t = vfslog_time();
  rc = REALVFS(pVfs)->xAccess(REALVFS(pVfs), zPath, flags, pResOut);
  t = vfslog_time() - t;
  vfslog_call(pVfs, OS_ACCESS, 0, t, rc, flags, *pResOut);
  vfslog_string(pVfs, zPath);
  החזר rc;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
ساكن صحيح vfslogFullPathname(
  sqlite3_vfs *pVfs, 
  ثابت حرف *zPath, 
  صحيح nOut, 
  حرف *zOut
){
  החזר REALVFS(pVfs)->xFullPathname(REALVFS(pVfs), zPath, nOut, zOut);
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
ساكن فراغ *vfslogDlOpen(sqlite3_vfs *pVfs, ثابت حرف *zPath){
  החזר REALVFS(pVfs)->xDlOpen(REALVFS(pVfs), zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
ساكن فراغ vfslogDlError(sqlite3_vfs *pVfs, صحيح nByte, حرف *zErrMsg){
  REALVFS(pVfs)->xDlError(REALVFS(pVfs), nByte, zErrMsg);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
ساكن فراغ (*vfslogDlSym(sqlite3_vfs *pVfs, فراغ *p, ثابت حرف *zSym))(فراغ){
  החזר REALVFS(pVfs)->xDlSym(REALVFS(pVfs), p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
ساكن فراغ vfslogDlClose(sqlite3_vfs *pVfs, فراغ *pHandle){
  REALVFS(pVfs)->xDlClose(REALVFS(pVfs), pHandle);
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
ساكن صحيح vfslogRandomness(sqlite3_vfs *pVfs, صحيح nByte, حرف *zBufOut){
  החזר REALVFS(pVfs)->xRandomness(REALVFS(pVfs), nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
ساكن صحيح vfslogSleep(sqlite3_vfs *pVfs, صحيح nMicro){
  החזר REALVFS(pVfs)->xSleep(REALVFS(pVfs), nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
ساكن صحيح vfslogCurrentTime(sqlite3_vfs *pVfs, مزدوج *pTimeOut){
  החזר REALVFS(pVfs)->xCurrentTime(REALVFS(pVfs), pTimeOut);
}

ساكن صحيح vfslogGetLastError(sqlite3_vfs *pVfs, صحيح a, حرف *b){
  החזר REALVFS(pVfs)->xGetLastError(REALVFS(pVfs), a, b);
}
ساكن صحيح vfslogCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *p){
  החזר REALVFS(pVfs)->xCurrentTimeInt64(REALVFS(pVfs), p);
}

ساكن فراغ vfslog_flush(VfslogVfs *p){
#ifdef SQLITE_TEST
  خارجي صحيح sqlite3_io_error_pending;
  خارجي صحيح sqlite3_io_error_persist;
  خارجي صحيح sqlite3_diskfull_pending;

  صحيح pending = sqlite3_io_error_pending;
  صحيح persist = sqlite3_io_error_persist;
  صحيح diskfull = sqlite3_diskfull_pending;

  sqlite3_io_error_pending = 0;
  sqlite3_io_error_persist = 0;
  sqlite3_diskfull_pending = 0;
#endif

  אם( p->nBuf ){
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

ساكن فراغ put32bits(unsigned حرف *p, unsigned صحيح v){
  p[0] = v>>24;
  p[1] = (unsigned حرف)(v>>16);
  p[2] = (unsigned حرف)(v>>8);
  p[3] = (unsigned حرف)v;
}

ساكن فراغ vfslog_call(
  sqlite3_vfs *pVfs,
  صحيح eEvent,
  صحيح iFileid,
  sqlite3_int64 nClick,
  صحيح return_code,
  صحيح size,
  صحيح offset
){
  VfslogVfs *p = (VfslogVfs *)pVfs;
  unsigned حرف *zRec;
  אם( (24+p->nBuf)>حجم(p->aBuf) ){
    vfslog_flush(p);
  }
  zRec = (unsigned حرف *)&p->aBuf[p->nBuf];
  put32bits(&zRec[0], eEvent);
  put32bits(&zRec[4], iFileid);
  put32bits(&zRec[8], (unsigned صحيح)(nClick&0xffff));
  put32bits(&zRec[12], return_code);
  put32bits(&zRec[16], size);
  put32bits(&zRec[20], offset);
  p->nBuf += 24;
}

ساكن فراغ vfslog_string(sqlite3_vfs *pVfs, ثابت حرف *zStr){
  VfslogVfs *p = (VfslogVfs *)pVfs;
  unsigned حرف *zRec;
  صحيح nStr = zStr ? (صحيح)strlen(zStr) : 0;
  אם( (4+nStr+p->nBuf)>حجم(p->aBuf) ){
    vfslog_flush(p);
  }
  zRec = (unsigned حرف *)&p->aBuf[p->nBuf];
  put32bits(&zRec[0], nStr);
  אם( zStr ){
    memcpy(&zRec[4], zStr, nStr);
  }
  p->nBuf += (4 + nStr);
}

ساكن فراغ vfslog_finalize(VfslogVfs *p){
  אם( p->pLog->pMethods ){
    vfslog_flush(p);
    p->pLog->pMethods->xClose(p->pLog);
  }
  sqlite3_free(p);
}

صحيح sqlite3_vfslog_finalize(ثابت حرف *zVfs){
  sqlite3_vfs *pVfs;
  pVfs = sqlite3_vfs_find(zVfs);
  אם( !pVfs || pVfs->xOpen!=vfslogOpen ){
    החזר SQLITE_ERROR;
  } 
  sqlite3_vfs_unregister(pVfs);
  vfslog_finalize((VfslogVfs *)pVfs);
  החזר SQLITE_OK;
}

صحيح sqlite3_vfslog_new(
  ثابت حرف *zVfs,               /* New VFS name */
  ثابت حرف *zParentVfs,         /* Parent VFS name (or NULL) */
  ثابت حرف *zLog                /* Log file name */
){
  VfslogVfs *p;
  sqlite3_vfs *pParent;
  صحيح nByte;
  صحيح flags;
  صحيح rc;
  حرف *zFile;
  صحيح nVfs;

  pParent = sqlite3_vfs_find(zParentVfs);
  אם( !pParent ){
    החזר SQLITE_ERROR;
  }

  nVfs = (صحيح)strlen(zVfs);
  nByte = حجم(VfslogVfs) + pParent->szOsFile + nVfs+1+pParent->mxPathname+1;
  p = (VfslogVfs *)sqlite3_malloc(nByte);
  memset(p, 0, nByte);

  p->pVfs = pParent;
  p->pLog = (sqlite3_file *)&p[1];
  memcpy(&p->base, &vfslog_vfs, حجم(sqlite3_vfs));
  p->base.zName = &((حرف *)p->pLog)[pParent->szOsFile];
  p->base.szOsFile += pParent->szOsFile;
  memcpy((حرف *)p->base.zName, zVfs, nVfs);

  zFile = (حرف *)&p->base.zName[nVfs+1];
  pParent->xFullPathname(pParent, zLog, pParent->mxPathname, zFile);

  flags = SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE|SQLITE_OPEN_SUPER_JOURNAL;
  pParent->xDelete(pParent, zFile, 0);
  rc = pParent->xOpen(pParent, zFile, p->pLog, flags, &flags);
  אם( rc==SQLITE_OK ){
    memcpy(p->aBuf, "sqlite_ostrace1.....", 20);
    p->iOffset = 0;
    p->nBuf = 20;
    rc = sqlite3_vfs_register((sqlite3_vfs *)p, 1);
  }
  אם( rc ){
    vfslog_finalize(p);
  }
  החזר rc;
}

صحيح sqlite3_vfslog_annotate(ثابت حرف *zVfs, ثابت حرف *zMsg){
  sqlite3_vfs *pVfs;
  pVfs = sqlite3_vfs_find(zVfs);
  אם( !pVfs || pVfs->xOpen!=vfslogOpen ){
    החזר SQLITE_ERROR;
  } 
  vfslog_call(pVfs, OS_ANNOTATE, 0, 0, 0, 0, 0);
  vfslog_string(pVfs, zMsg);
  החזר SQLITE_OK;
}

ساكن ثابت حرف *vfslog_eventname(صحيح eEvent){
  ثابت حرف *zEvent = 0;

  بدل( eEvent ){
    حالة OS_CLOSE:             zEvent = "xClose"; اكسر;
    حالة OS_READ:              zEvent = "xRead"; اكسر;
    حالة OS_WRITE:             zEvent = "xWrite"; اكسر;
    حالة OS_TRUNCATE:          zEvent = "xTruncate"; اكسر;
    حالة OS_SYNC:              zEvent = "xSync"; اكسر;
    حالة OS_FILESIZE:          zEvent = "xFilesize"; اكسر;
    حالة OS_LOCK:              zEvent = "xLock"; اكسر;
    حالة OS_UNLOCK:            zEvent = "xUnlock"; اكسر;
    حالة OS_CHECKRESERVEDLOCK: zEvent = "xCheckResLock"; اكسر;
    حالة OS_FILECONTROL:       zEvent = "xFileControl"; اكسر;
    حالة OS_SECTORSIZE:        zEvent = "xSectorSize"; اكسر;
    حالة OS_DEVCHAR:           zEvent = "xDeviceChar"; اكسر;
    حالة OS_OPEN:              zEvent = "xOpen"; اكسر;
    حالة OS_DELETE:            zEvent = "xDelete"; اكسر;
    حالة OS_ACCESS:            zEvent = "xAccess"; اكسر;
    حالة OS_FULLPATHNAME:      zEvent = "xFullPathname"; اكسر;
    حالة OS_RANDOMNESS:        zEvent = "xRandomness"; اكسر;
    حالة OS_SLEEP:             zEvent = "xSleep"; اكسر;
    حالة OS_CURRENTTIME:       zEvent = "xCurrentTime"; اكسر;

    حالة OS_SHMUNMAP:          zEvent = "xShmUnmap"; اكسر;
    حالة OS_SHMLOCK:           zEvent = "xShmLock"; اكسر;
    حالة OS_SHMBARRIER:        zEvent = "xShmBarrier"; اكسر;
    حالة OS_SHMMAP:            zEvent = "xShmMap"; اكسر;

    حالة OS_ANNOTATE:          zEvent = "annotation"; اكسر;
  }

  החזר zEvent;
}

تعريفنوع بنية VfslogVtab VfslogVtab;
تعريفنوع بنية VfslogCsr VfslogCsr;

/*
** Virtual table type for the vfslog reader module.
*/
بنية VfslogVtab {
  sqlite3_vtab base;              /* Base class */
  sqlite3_file *pFd;              /* File descriptor open on vfslog file */
  sqlite3_int64 nByte;            /* Size of file in bytes */
  حرف *zFile;                    /* File name for pFd */
};

/*
** Virtual table cursor type for the vfslog reader module.
*/
بنية VfslogCsr {
  sqlite3_vtab_cursor base;       /* Base class */
  sqlite3_int64 iRowid;           /* Current rowid. */
  sqlite3_int64 iOffset;          /* Offset of next record in file */
  حرف *zTransient;               /* Transient 'file' string */
  صحيح nFile;                      /* Size of array azFile[] */
  حرف **azFile;                  /* File strings */
  unsigned حرف aBuf[1024];       /* Current vfs log entry (read from file) */
};

ساكن unsigned صحيح get32bits(unsigned حرف *p){
  החזר (p[0]<<24) + (p[1]<<16) + (p[2]<<8) + p[3];
}

/*
** The argument must point to a buffer containing a nul-terminated string.
** If the string begins with an SQL quote character it is overwritten by
** the dequoted version. Otherwise the buffer is left unmodified.
*/
ساكن فراغ dequote(حرف *z){
  حرف quote;                     /* Quote character (if any ) */
  quote = z[0];
  אם( quote=='[' || quote=='\'' || quote=='"' || quote=='`' ){
    صحيح iIn = 1;                  /* Index of next byte to read from input */
    صحيح iOut = 0;                 /* Index of next byte to write to output */
    אם( quote=='[' ) quote = ']';  
    כלעוד( z[iIn] ){
      אם( z[iIn]==quote ){
        אם( z[iIn+1]!=quote ) اكسر;
        z[iOut++] = quote;
        iIn += 2;
      }אחרת{
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
ساكن صحيح vlogConnect(
  sqlite3 *db,
  فراغ *pAux,
  صحيح argc, ثابت حرف *ثابت*argv,
  sqlite3_vtab **ppVtab,
  حرف **pzErr
){
  sqlite3_vfs *pVfs;              /* VFS used to read log file */
  صحيح flags;                      /* flags passed to pVfs->xOpen() */
  VfslogVtab *p;
  صحيح rc;
  صحيح nByte;
  حرف *zFile;

  *ppVtab = 0;
  pVfs = sqlite3_vfs_find(0);
  nByte = حجم(VfslogVtab) + pVfs->szOsFile + pVfs->mxPathname;
  p = sqlite3_malloc(nByte);
  אם( p==0 ) החזר SQLITE_NOMEM;
  memset(p, 0, nByte);

  p->pFd = (sqlite3_file *)&p[1];
  p->zFile = &((حرف *)p->pFd)[pVfs->szOsFile];

  zFile = sqlite3_mprintf("%s", argv[3]);
  אם( !zFile ){
    sqlite3_free(p);
    החזר SQLITE_NOMEM;
  }
  dequote(zFile);
  pVfs->xFullPathname(pVfs, zFile, pVfs->mxPathname, p->zFile);
  sqlite3_free(zFile);

  flags = SQLITE_OPEN_READWRITE|SQLITE_OPEN_SUPER_JOURNAL;
  rc = pVfs->xOpen(pVfs, p->zFile, p->pFd, flags, &flags);

  אם( rc==SQLITE_OK ){
    p->pFd->pMethods->xFileSize(p->pFd, &p->nByte);
    sqlite3_declare_vtab(db, 
        "CREATE TABLE xxx(event, file, click, rc, size, offset)"
    );
    *ppVtab = &p->base;
  }אחרת{
    sqlite3_free(p);
  }

  החזר rc;
}

/*
** There is no "best-index". This virtual table always does a linear
** scan of the binary VFS log file.
*/
ساكن صحيح vlogBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  pIdxInfo->estimatedCost = 10.0;
  החזר SQLITE_OK;
}

/*
** Disconnect from or destroy a vfslog virtual table.
*/
ساكن صحيح vlogDisconnect(sqlite3_vtab *pVtab){
  VfslogVtab *p = (VfslogVtab *)pVtab;
  אם( p->pFd->pMethods ){
    p->pFd->pMethods->xClose(p->pFd);
    p->pFd->pMethods = 0;
  }
  sqlite3_free(p);
  החזר SQLITE_OK;
}

/*
** Open a new vfslog cursor.
*/
ساكن صحيح vlogOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  VfslogCsr *pCsr;                /* Newly allocated cursor object */

  pCsr = sqlite3_malloc(حجم(VfslogCsr));
  אם( !pCsr ) החזר SQLITE_NOMEM;
  memset(pCsr, 0, حجم(VfslogCsr));
  *ppCursor = &pCsr->base;
  החזר SQLITE_OK;
}

/*
** Close a vfslog cursor.
*/
ساكن صحيح vlogClose(sqlite3_vtab_cursor *pCursor){
  VfslogCsr *p = (VfslogCsr *)pCursor;
  صحيح i;
  עבור(i=0; i<p->nFile; i++){
    sqlite3_free(p->azFile[i]);
  }
  sqlite3_free(p->azFile);
  sqlite3_free(p->zTransient);
  sqlite3_free(p);
  החזר SQLITE_OK;
}

/*
** Move a vfslog cursor to the next entry in the file.
*/
ساكن صحيح vlogNext(sqlite3_vtab_cursor *pCursor){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  VfslogVtab *p = (VfslogVtab *)pCursor->pVtab;
  صحيح rc = SQLITE_OK;
  صحيح nRead;

  sqlite3_free(pCsr->zTransient);
  pCsr->zTransient = 0;

  nRead = 24;
  אם( pCsr->iOffset+nRead<=p->nByte ){
    صحيح eEvent;
    rc = p->pFd->pMethods->xRead(p->pFd, pCsr->aBuf, nRead, pCsr->iOffset);

    eEvent = get32bits(pCsr->aBuf);
    אם( (rc==SQLITE_OK)
     && (eEvent==OS_OPEN || eEvent==OS_DELETE || eEvent==OS_ACCESS) 
    ){
      حرف buf[4];
      rc = p->pFd->pMethods->xRead(p->pFd, buf, 4, pCsr->iOffset+nRead);
      nRead += 4;
      אם( rc==SQLITE_OK ){
        صحيح nStr = get32bits((unsigned حرف *)buf);
        حرف *zStr = sqlite3_malloc(nStr+1);
        rc = p->pFd->pMethods->xRead(p->pFd, zStr, nStr, pCsr->iOffset+nRead);
        zStr[nStr] = '\0';
        nRead += nStr;

        אם( eEvent==OS_OPEN ){
          صحيح iFileid = get32bits(&pCsr->aBuf[4]);
          אם( iFileid>=pCsr->nFile ){
            صحيح nNew = حجم(pCsr->azFile[0])*(iFileid+1);
            pCsr->azFile = (حرف **)sqlite3_realloc(pCsr->azFile, nNew);
            nNew -= حجم(pCsr->azFile[0])*pCsr->nFile;
            memset(&pCsr->azFile[pCsr->nFile], 0, nNew);
            pCsr->nFile = iFileid+1;
          }
          sqlite3_free(pCsr->azFile[iFileid]);
          pCsr->azFile[iFileid] = zStr;
        }אחרת{
          pCsr->zTransient = zStr;
        }
      }
    }
  }

  pCsr->iRowid += 1;
  pCsr->iOffset += nRead;
  החזר rc;
}

ساكن صحيح vlogEof(sqlite3_vtab_cursor *pCursor){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  VfslogVtab *p = (VfslogVtab *)pCursor->pVtab;
  החזר (pCsr->iOffset>=p->nByte);
}

ساكن صحيح vlogFilter(
  sqlite3_vtab_cursor *pCursor, 
  صحيح idxNum, ثابت حرف *idxStr,
  صحيح argc, sqlite3_value **argv
){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  pCsr->iRowid = 0;
  pCsr->iOffset = 20;
  החזר vlogNext(pCursor);
}

ساكن صحيح vlogColumn(
  sqlite3_vtab_cursor *pCursor, 
  sqlite3_context *ctx, 
  صحيح i
){
  unsigned صحيح val;
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;

  أكد( i<7 );
  val = get32bits(&pCsr->aBuf[4*i]);

  بدل( i ){
    حالة 0: {
      sqlite3_result_text(ctx, vfslog_eventname(val), -1, SQLITE_STATIC);
      اكسر;
    }
    حالة 1: {
      حرف *zStr = pCsr->zTransient;
      אם( val!=0 && val<(unsigned)pCsr->nFile ){
        zStr = pCsr->azFile[val];
      }
      sqlite3_result_text(ctx, zStr, -1, SQLITE_TRANSIENT);
      اكسر;
    }
    افتراضي:
      sqlite3_result_int(ctx, val);
      اكسر;
  }

  החזר SQLITE_OK;
}

ساكن صحيح vlogRowid(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  *pRowid = pCsr->iRowid;
  החזר SQLITE_OK;
}

صحيح sqlite3_vfslog_register(sqlite3 *db){
  ساكن sqlite3_module vfslog_module = {
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
  החזר SQLITE_OK;
}
#endif /* SQLITE_OMIT_VIRTUALTABLE */

/**************************************************************************
***************************************************************************
** Tcl interface starts here.
*/

#אם defined(SQLITE_TEST) || defined(TCLSH)

#ضمن "tclsqlite.h"

ساكن صحيح SQLITE_TCLAPI test_vfslog(
  فراغ *clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  بنية SqliteDb { sqlite3 *db; };
  sqlite3 *db;
  Tcl_CmdInfo cmdInfo;
  صحيح rc = SQLITE_ERROR;

  ساكن ثابت حرف *strs[] = { "annotate", "finalize", "new",  "register", 0 };
  تعداد VL_enum { VL_ANNOTATE, VL_FINALIZE, VL_NEW, VL_REGISTER };
  صحيح iSub;

  אם( objc<2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "SUB-COMMAND ...");
    החזר TCL_ERROR;
  }
  אם( Tcl_GetIndexFromObj(interp, objv[1], strs, "sub-command", 0, &iSub) ){
    החזר TCL_ERROR;
  }

  بدل( (تعداد VL_enum)iSub ){
    حالة VL_ANNOTATE: {
      حرف *zVfs;
      حرف *zMsg;
      אם( objc!=4 ){
        Tcl_WrongNumArgs(interp, 3, objv, "VFS");
        החזר TCL_ERROR;
      }
      zVfs = Tcl_GetString(objv[2]);
      zMsg = Tcl_GetString(objv[3]);
      rc = sqlite3_vfslog_annotate(zVfs, zMsg);
      אם( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp, "failed", (حرف*)0);
        החזר TCL_ERROR;
      }
      اكسر;
    }
    حالة VL_FINALIZE: {
      حرف *zVfs;
      אם( objc!=3 ){
        Tcl_WrongNumArgs(interp, 2, objv, "VFS");
        החזר TCL_ERROR;
      }
      zVfs = Tcl_GetString(objv[2]);
      rc = sqlite3_vfslog_finalize(zVfs);
      אם( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp, "failed", (حرف*)0);
        החזר TCL_ERROR;
      }
      اكسر;
    };

    حالة VL_NEW: {
      حرف *zVfs;
      حرف *zParent;
      حرف *zLog;
      אם( objc!=5 ){
        Tcl_WrongNumArgs(interp, 2, objv, "VFS PARENT LOGFILE");
        החזר TCL_ERROR;
      }
      zVfs = Tcl_GetString(objv[2]);
      zParent = Tcl_GetString(objv[3]);
      zLog = Tcl_GetString(objv[4]);
      אם( *zParent=='\0' ) zParent = 0;
      rc = sqlite3_vfslog_new(zVfs, zParent, zLog);
      אם( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp, "failed", (حرف*)0);
        החזר TCL_ERROR;
      }
      اكسر;
    };

    حالة VL_REGISTER: {
      حرف *zDb;
      אם( objc!=3 ){
        Tcl_WrongNumArgs(interp, 2, objv, "DB");
        החזר TCL_ERROR;
      }
#ifdef SQLITE_OMIT_VIRTUALTABLE
      Tcl_AppendResult(interp, "vfslog not available because of "
                               "SQLITE_OMIT_VIRTUALTABLE", (فراغ*)0);
      החזר TCL_ERROR;
#אחרת
      zDb = Tcl_GetString(objv[2]);
      אם( Tcl_GetCommandInfo(interp, zDb, &cmdInfo) ){
        db = ((بنية SqliteDb*)cmdInfo.objClientData)->db;
        rc = sqlite3_vfslog_register(db);
      }
      אם( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp, "bad sqlite3 handle: ", zDb, (فراغ*)0);
        החזר TCL_ERROR;
      }
      اكسر;
#endif
    }
  }

  החזר TCL_OK;
}

صحيح SqlitetestOsinst_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "vfslog", test_vfslog, 0, 0);
  החזר TCL_OK;
}

#endif /* SQLITE_TEST */
