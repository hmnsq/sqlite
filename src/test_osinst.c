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

#شامل "sqlite3.h"

#ifdef _WIN32
#شامل <windows.h>
#endif

#شامل <string.h>
#شامل <assert.h>


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

تعریفنوع ساختار VfslogVfs VfslogVfs;
تعریفنوع ساختار VfslogFile VfslogFile;

ساختار VfslogVfs {
  sqlite3_vfs base;               /* VFS methods */
  sqlite3_vfs *pVfs;              /* Parent VFS */
  صحیح iNextFileId;                /* Next file id */
  sqlite3_file *pLog;             /* Log file handle */
  sqlite3_int64 iOffset;          /* Log file offset of start of write buffer */
  صحیح nBuf;                       /* Number of valid bytes in aBuf[] */
  نویسه aBuf[VFSLOG_BUFFERSIZE];   /* Write buffer */
};

ساختار VfslogFile {
  sqlite3_file base;              /* IO methods */
  sqlite3_file *pReal;            /* Underlying file handle */
  sqlite3_vfs *pVfslog;           /* Associated VsflogVfs object */
  صحیح iFileId;                    /* File id number */
};

#define REALVFS(p) (((VfslogVfs *)(p))->pVfs)



/*
** Method declarations for vfslog_file.
*/
ایستا صحیح vfslogClose(sqlite3_file*);
ایستا صحیح vfslogRead(sqlite3_file*, تهی*, صحیح iAmt, sqlite3_int64 iOfst);
ایستا صحیح vfslogWrite(sqlite3_file*,ثابت تهی*,صحیح iAmt, sqlite3_int64 iOfst);
ایستا صحیح vfslogTruncate(sqlite3_file*, sqlite3_int64 size);
ایستا صحیح vfslogSync(sqlite3_file*, صحیح flags);
ایستا صحیح vfslogFileSize(sqlite3_file*, sqlite3_int64 *pSize);
ایستا صحیح vfslogLock(sqlite3_file*, صحیح);
ایستا صحیح vfslogUnlock(sqlite3_file*, صحیح);
ایستا صحیح vfslogCheckReservedLock(sqlite3_file*, صحیح *pResOut);
ایستا صحیح vfslogFileControl(sqlite3_file*, صحیح op, تهی *pArg);
ایستا صحیح vfslogSectorSize(sqlite3_file*);
ایستا صحیح vfslogDeviceCharacteristics(sqlite3_file*);

ایستا صحیح vfslogShmLock(sqlite3_file *pFile, صحیح ofst, صحیح n, صحیح flags);
ایستا صحیح vfslogShmMap(sqlite3_file *pFile,صحیح,صحیح,صحیح,فرار تهی **);
ایستا تهی vfslogShmBarrier(sqlite3_file*);
ایستا صحیح vfslogShmUnmap(sqlite3_file *pFile, صحیح deleteFlag);

/*
** Method declarations for vfslog_vfs.
*/
ایستا صحیح vfslogOpen(sqlite3_vfs*, ثابت نویسه *, sqlite3_file*, صحیح , صحیح *);
ایستا صحیح vfslogDelete(sqlite3_vfs*, ثابت نویسه *zName, صحیح syncDir);
ایستا صحیح vfslogAccess(sqlite3_vfs*, ثابت نویسه *zName, صحیح flags, صحیح *);
ایستا صحیح vfslogFullPathname(sqlite3_vfs*, ثابت نویسه *zName, صحیح, نویسه *zOut);
ایستا تهی *vfslogDlOpen(sqlite3_vfs*, ثابت نویسه *zFilename);
ایستا تهی vfslogDlError(sqlite3_vfs*, صحیح nByte, نویسه *z_Err_پیام);
ایستا تهی (*vfslogDlSym(sqlite3_vfs *pVfs, تهی *p, ثابت نویسه*zSym))(تهی);
ایستا تهی vfslogDlClose(sqlite3_vfs*, تهی*);
ایستا صحیح vfslogRandomness(sqlite3_vfs*, صحیح nByte, نویسه *zOut);
ایستا صحیح vfslogSleep(sqlite3_vfs*, صحیح microseconds);
ایستا صحیح vfslogCurrentTime(sqlite3_vfs*, دوگانه*);

ایستا صحیح vfslogGetLastError(sqlite3_vfs*, صحیح, نویسه *);
ایستا صحیح vfslogCurrentTimeInt64(sqlite3_vfs*, sqlite3_int64*);

ایستا sqlite3_vfs vfslog_vfs = {
  1,                              /* iVersion */
  اندازه(VfslogFile),             /* szOsFile */
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

ایستا sqlite3_io_methods vfslog_io_methods = {
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
#شامل <time.h>
ایستا sqlite3_uint64 vfslog_time(){
  FILETIME ft;
  sqlite3_uint64 u64time = 0;
 
  GetSystemTimeAsFileTime(&ft);

  u64time |= ft.dw_بالا_Date_Time;
  u64time <<= 32;
  u64time |= ft.dw_کم_Date_Time;

  /* ft is 100-nanosecond intervals, we want microseconds */
  بازگشت u64time /(sqlite3_uint64)10;
}
#وگرنهاگر !defined(NO_GETTOD)
#شامل <sys/time.h>
ایستا sqlite3_uint64 vfslog_time(){
  ساختار timeval sTime;
  gettimeofday(&sTime, 0);
  بازگشت sTime.tv_usec + (sqlite3_uint64)sTime.tv_sec * 1000000;
}
#وگرنه
ایستا sqlite3_uint64 vfslog_time(){
  بازگشت 0;
}
#endif

ایستا تهی vfslog_call(sqlite3_vfs *, صحیح, صحیح, sqlite3_int64, صحیح, صحیح, صحیح);
ایستا تهی vfslog_string(sqlite3_vfs *, ثابت نویسه *);

/*
** Close an vfslog-file.
*/
ایستا صحیح vfslogClose(sqlite3_file *pFile){
  sqlite3_uint64 t;
  صحیح rc = SQLITE_OK;
  VfslogFile *p = (VfslogFile *)pFile;

  t = vfslog_time();
  اگر( p->pReal->pMethods ){
    rc = p->pReal->pMethods->xClose(p->pReal);
  }
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_CLOSE, p->iFileId, t, rc, 0, 0);
  بازگشت rc;
}

/*
** Read data from an vfslog-file.
*/
ایستا صحیح vfslogRead(
  sqlite3_file *pFile, 
  تهی *zBuf, 
  صحیح iAmt, 
  sqlite_int64 iOfst
){
  صحیح rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xRead(p->pReal, zBuf, iAmt, iOfst);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_READ, p->iFileId, t, rc, iAmt, (صحیح)iOfst);
  بازگشت rc;
}

/*
** Write data to an vfslog-file.
*/
ایستا صحیح vfslogWrite(
  sqlite3_file *pFile,
  ثابت تهی *z,
  صحیح iAmt,
  sqlite_int64 iOfst
){
  صحیح rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xWrite(p->pReal, z, iAmt, iOfst);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_WRITE, p->iFileId, t, rc, iAmt, (صحیح)iOfst);
  بازگشت rc;
}

/*
** Truncate an vfslog-file.
*/
ایستا صحیح vfslogTruncate(sqlite3_file *pFile, sqlite_int64 size){
  صحیح rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xTruncate(p->pReal, size);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_TRUNCATE, p->iFileId, t, rc, 0, (صحیح)size);
  بازگشت rc;
}

/*
** Sync an vfslog-file.
*/
ایستا صحیح vfslogSync(sqlite3_file *pFile, صحیح flags){
  صحیح rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xSync(p->pReal, flags);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SYNC, p->iFileId, t, rc, flags, 0);
  بازگشت rc;
}

/*
** Return the current file-size of an vfslog-file.
*/
ایستا صحیح vfslogFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  صحیح rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xFileSize(p->pReal, pSize);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_FILESIZE, p->iFileId, t, rc, 0, (صحیح)*pSize);
  بازگشت rc;
}

/*
** Lock an vfslog-file.
*/
ایستا صحیح vfslogLock(sqlite3_file *pFile, صحیح eLock){
  صحیح rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xLock(p->pReal, eLock);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_LOCK, p->iFileId, t, rc, eLock, 0);
  بازگشت rc;
}

/*
** Unlock an vfslog-file.
*/
ایستا صحیح vfslogUnlock(sqlite3_file *pFile, صحیح eLock){
  صحیح rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xUnlock(p->pReal, eLock);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_UNLOCK, p->iFileId, t, rc, eLock, 0);
  بازگشت rc;
}

/*
** Check if another file-handle holds a RESERVED lock on an vfslog-file.
*/
ایستا صحیح vfslogCheckReservedLock(sqlite3_file *pFile, صحیح *pResOut){
  صحیح rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xCheckReservedLock(p->pReal, pResOut);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_CHECKRESERVEDLOCK, p->iFileId, t, rc, *pResOut, 0);
  بازگشت rc;
}

/*
** File control method. For custom operations on an vfslog-file.
*/
ایستا صحیح vfslogFileControl(sqlite3_file *pFile, صحیح op, تهی *pArg){
  VfslogFile *p = (VfslogFile *)pFile;
  صحیح rc = p->pReal->pMethods->xFileControl(p->pReal, op, pArg);
  اگر( op==SQLITE_FCNTL_VFSNAME && rc==SQLITE_OK ){
    *(نویسه**)pArg = sqlite3_mprintf("vfslog/%z", *(نویسه**)pArg);
  }
  بازگشت rc;
}

/*
** Return the sector-size in bytes for an vfslog-file.
*/
ایستا صحیح vfslogSectorSize(sqlite3_file *pFile){
  صحیح rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xSectorSize(p->pReal);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SECTORSIZE, p->iFileId, t, rc, 0, 0);
  بازگشت rc;
}

/*
** Return the device characteristic flags supported by an vfslog-file.
*/
ایستا صحیح vfslogDeviceCharacteristics(sqlite3_file *pFile){
  صحیح rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xDeviceCharacteristics(p->pReal);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_DEVCHAR, p->iFileId, t, rc, 0, 0);
  بازگشت rc;
}

ایستا صحیح vfslogShmLock(sqlite3_file *pFile, صحیح ofst, صحیح n, صحیح flags){
  صحیح rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xShmLock(p->pReal, ofst, n, flags);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SHMLOCK, p->iFileId, t, rc, 0, 0);
  بازگشت rc;
}
ایستا صحیح vfslogShmMap(
  sqlite3_file *pFile, 
  صحیح iRegion, 
  صحیح szRegion, 
  صحیح isWrite, 
  فرار تهی **pp
){
  صحیح rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xShmMap(p->pReal, iRegion, szRegion, isWrite, pp);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SHMMAP, p->iFileId, t, rc, 0, 0);
  بازگشت rc;
}
ایستا تهی vfslogShmBarrier(sqlite3_file *pFile){
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  p->pReal->pMethods->xShmBarrier(p->pReal);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SHMBARRIER, p->iFileId, t, SQLITE_OK, 0, 0);
}
ایستا صحیح vfslogShmUnmap(sqlite3_file *pFile, صحیح deleteFlag){
  صحیح rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xShmUnmap(p->pReal, deleteFlag);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SHMUNMAP, p->iFileId, t, rc, 0, 0);
  بازگشت rc;
}


/*
** Open an vfslog file handle.
*/
ایستا صحیح vfslogOpen(
  sqlite3_vfs *pVfs,
  ثابت نویسه *zName,
  sqlite3_file *pFile,
  صحیح flags,
  صحیح *pOutFlags
){
  صحیح rc;
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
  بازگشت rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
ایستا صحیح vfslogDelete(sqlite3_vfs *pVfs, ثابت نویسه *zPath, صحیح dirSync){
  صحیح rc;
  sqlite3_uint64 t;
  t = vfslog_time();
  rc = REALVFS(pVfs)->xDelete(REALVFS(pVfs), zPath, dirSync);
  t = vfslog_time() - t;
  vfslog_call(pVfs, OS_DELETE, 0, t, rc, dirSync, 0);
  vfslog_string(pVfs, zPath);
  بازگشت rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
ایستا صحیح vfslogAccess(
  sqlite3_vfs *pVfs, 
  ثابت نویسه *zPath, 
  صحیح flags, 
  صحیح *pResOut
){
  صحیح rc;
  sqlite3_uint64 t;
  t = vfslog_time();
  rc = REALVFS(pVfs)->xAccess(REALVFS(pVfs), zPath, flags, pResOut);
  t = vfslog_time() - t;
  vfslog_call(pVfs, OS_ACCESS, 0, t, rc, flags, *pResOut);
  vfslog_string(pVfs, zPath);
  بازگشت rc;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
ایستا صحیح vfslogFullPathname(
  sqlite3_vfs *pVfs, 
  ثابت نویسه *zPath, 
  صحیح nOut, 
  نویسه *zOut
){
  بازگشت REALVFS(pVfs)->xFullPathname(REALVFS(pVfs), zPath, nOut, zOut);
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
ایستا تهی *vfslogDlOpen(sqlite3_vfs *pVfs, ثابت نویسه *zPath){
  بازگشت REALVFS(pVfs)->xDlOpen(REALVFS(pVfs), zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
ایستا تهی vfslogDlError(sqlite3_vfs *pVfs, صحیح nByte, نویسه *z_Err_پیام){
  REALVFS(pVfs)->xDlError(REALVFS(pVfs), nByte, z_Err_پیام);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
ایستا تهی (*vfslogDlSym(sqlite3_vfs *pVfs, تهی *p, ثابت نویسه *zSym))(تهی){
  بازگشت REALVFS(pVfs)->xDlSym(REALVFS(pVfs), p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
ایستا تهی vfslogDlClose(sqlite3_vfs *pVfs, تهی *pHandle){
  REALVFS(pVfs)->xDlClose(REALVFS(pVfs), pHandle);
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
ایستا صحیح vfslogRandomness(sqlite3_vfs *pVfs, صحیح nByte, نویسه *zBufOut){
  بازگشت REALVFS(pVfs)->xRandomness(REALVFS(pVfs), nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
ایستا صحیح vfslogSleep(sqlite3_vfs *pVfs, صحیح nMicro){
  بازگشت REALVFS(pVfs)->xSleep(REALVFS(pVfs), nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
ایستا صحیح vfslogCurrentTime(sqlite3_vfs *pVfs, دوگانه *pTimeOut){
  بازگشت REALVFS(pVfs)->xCurrentTime(REALVFS(pVfs), pTimeOut);
}

ایستا صحیح vfslogGetLastError(sqlite3_vfs *pVfs, صحیح a, نویسه *b){
  بازگشت REALVFS(pVfs)->xGetLastError(REALVFS(pVfs), a, b);
}
ایستا صحیح vfslogCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *p){
  بازگشت REALVFS(pVfs)->xCurrentTimeInt64(REALVFS(pVfs), p);
}

ایستا تهی vfslog_flush(VfslogVfs *p){
#ifdef SQLITE_TEST
  بیرونی صحیح sqlite3_io_error_pending;
  بیرونی صحیح sqlite3_io_error_persist;
  بیرونی صحیح sqlite3_diskfull_pending;

  صحیح pending = sqlite3_io_error_pending;
  صحیح persist = sqlite3_io_error_persist;
  صحیح diskfull = sqlite3_diskfull_pending;

  sqlite3_io_error_pending = 0;
  sqlite3_io_error_persist = 0;
  sqlite3_diskfull_pending = 0;
#endif

  اگر( p->nBuf ){
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

ایستا تهی put32bits(unsigned نویسه *p, unsigned صحیح v){
  p[0] = v>>24;
  p[1] = (unsigned نویسه)(v>>16);
  p[2] = (unsigned نویسه)(v>>8);
  p[3] = (unsigned نویسه)v;
}

ایستا تهی vfslog_call(
  sqlite3_vfs *pVfs,
  صحیح eEvent,
  صحیح iFileid,
  sqlite3_int64 nClick,
  صحیح return_code,
  صحیح size,
  صحیح offset
){
  VfslogVfs *p = (VfslogVfs *)pVfs;
  unsigned نویسه *zRec;
  اگر( (24+p->nBuf)>اندازه(p->aBuf) ){
    vfslog_flush(p);
  }
  zRec = (unsigned نویسه *)&p->aBuf[p->nBuf];
  put32bits(&zRec[0], eEvent);
  put32bits(&zRec[4], iFileid);
  put32bits(&zRec[8], (unsigned صحیح)(nClick&0xffff));
  put32bits(&zRec[12], return_code);
  put32bits(&zRec[16], size);
  put32bits(&zRec[20], offset);
  p->nBuf += 24;
}

ایستا تهی vfslog_string(sqlite3_vfs *pVfs, ثابت نویسه *zStr){
  VfslogVfs *p = (VfslogVfs *)pVfs;
  unsigned نویسه *zRec;
  صحیح nStr = zStr ? (صحیح)strlen(zStr) : 0;
  اگر( (4+nStr+p->nBuf)>اندازه(p->aBuf) ){
    vfslog_flush(p);
  }
  zRec = (unsigned نویسه *)&p->aBuf[p->nBuf];
  put32bits(&zRec[0], nStr);
  اگر( zStr ){
    memcpy(&zRec[4], zStr, nStr);
  }
  p->nBuf += (4 + nStr);
}

ایستا تهی vfslog_finalize(VfslogVfs *p){
  اگر( p->pLog->pMethods ){
    vfslog_flush(p);
    p->pLog->pMethods->xClose(p->pLog);
  }
  sqlite3_free(p);
}

صحیح sqlite3_vfslog_finalize(ثابت نویسه *zVfs){
  sqlite3_vfs *pVfs;
  pVfs = sqlite3_vfs_find(zVfs);
  اگر( !pVfs || pVfs->xOpen!=vfslogOpen ){
    بازگشت SQLITE_ERROR;
  } 
  sqlite3_vfs_unregister(pVfs);
  vfslog_finalize((VfslogVfs *)pVfs);
  بازگشت SQLITE_OK;
}

صحیح sqlite3_vfslog_new(
  ثابت نویسه *zVfs,               /* New VFS name */
  ثابت نویسه *zParentVfs,         /* Parent VFS name (or NULL) */
  ثابت نویسه *zLog                /* Log file name */
){
  VfslogVfs *p;
  sqlite3_vfs *pParent;
  صحیح nByte;
  صحیح flags;
  صحیح rc;
  نویسه *zFile;
  صحیح nVfs;

  pParent = sqlite3_vfs_find(zParentVfs);
  اگر( !pParent ){
    بازگشت SQLITE_ERROR;
  }

  nVfs = (صحیح)strlen(zVfs);
  nByte = اندازه(VfslogVfs) + pParent->szOsFile + nVfs+1+pParent->mxPathname+1;
  p = (VfslogVfs *)sqlite3_malloc(nByte);
  memset(p, 0, nByte);

  p->pVfs = pParent;
  p->pLog = (sqlite3_file *)&p[1];
  memcpy(&p->base, &vfslog_vfs, اندازه(sqlite3_vfs));
  p->base.zName = &((نویسه *)p->pLog)[pParent->szOsFile];
  p->base.szOsFile += pParent->szOsFile;
  memcpy((نویسه *)p->base.zName, zVfs, nVfs);

  zFile = (نویسه *)&p->base.zName[nVfs+1];
  pParent->xFullPathname(pParent, zLog, pParent->mxPathname, zFile);

  flags = SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE|SQLITE_OPEN_SUPER_JOURNAL;
  pParent->xDelete(pParent, zFile, 0);
  rc = pParent->xOpen(pParent, zFile, p->pLog, flags, &flags);
  اگر( rc==SQLITE_OK ){
    memcpy(p->aBuf, "sqlite_ostrace1.....", 20);
    p->iOffset = 0;
    p->nBuf = 20;
    rc = sqlite3_vfs_register((sqlite3_vfs *)p, 1);
  }
  اگر( rc ){
    vfslog_finalize(p);
  }
  بازگشت rc;
}

صحیح sqlite3_vfslog_annotate(ثابت نویسه *zVfs, ثابت نویسه *z_پیام){
  sqlite3_vfs *pVfs;
  pVfs = sqlite3_vfs_find(zVfs);
  اگر( !pVfs || pVfs->xOpen!=vfslogOpen ){
    بازگشت SQLITE_ERROR;
  } 
  vfslog_call(pVfs, OS_ANNOTATE, 0, 0, 0, 0, 0);
  vfslog_string(pVfs, z_پیام);
  بازگشت SQLITE_OK;
}

ایستا ثابت نویسه *vfslog_eventname(صحیح eEvent){
  ثابت نویسه *zEvent = 0;

  انتخاب( eEvent ){
    حالت OS_CLOSE:             zEvent = "xClose"; بشکن;
    حالت OS_READ:              zEvent = "xRead"; بشکن;
    حالت OS_WRITE:             zEvent = "xWrite"; بشکن;
    حالت OS_TRUNCATE:          zEvent = "xTruncate"; بشکن;
    حالت OS_SYNC:              zEvent = "xSync"; بشکن;
    حالت OS_FILESIZE:          zEvent = "xFilesize"; بشکن;
    حالت OS_LOCK:              zEvent = "xLock"; بشکن;
    حالت OS_UNLOCK:            zEvent = "xUnlock"; بشکن;
    حالت OS_CHECKRESERVEDLOCK: zEvent = "xCheckResLock"; بشکن;
    حالت OS_FILECONTROL:       zEvent = "xFileControl"; بشکن;
    حالت OS_SECTORSIZE:        zEvent = "xSectorSize"; بشکن;
    حالت OS_DEVCHAR:           zEvent = "xDeviceChar"; بشکن;
    حالت OS_OPEN:              zEvent = "xOpen"; بشکن;
    حالت OS_DELETE:            zEvent = "xDelete"; بشکن;
    حالت OS_ACCESS:            zEvent = "xAccess"; بشکن;
    حالت OS_FULLPATHNAME:      zEvent = "xFullPathname"; بشکن;
    حالت OS_RANDOMNESS:        zEvent = "xRandomness"; بشکن;
    حالت OS_SLEEP:             zEvent = "xSleep"; بشکن;
    حالت OS_CURRENTTIME:       zEvent = "xCurrentTime"; بشکن;

    حالت OS_SHMUNMAP:          zEvent = "xShmUnmap"; بشکن;
    حالت OS_SHMLOCK:           zEvent = "xShmLock"; بشکن;
    حالت OS_SHMBARRIER:        zEvent = "xShmBarrier"; بشکن;
    حالت OS_SHMMAP:            zEvent = "xShmMap"; بشکن;

    حالت OS_ANNOTATE:          zEvent = "annotation"; بشکن;
  }

  بازگشت zEvent;
}

تعریفنوع ساختار VfslogVtab VfslogVtab;
تعریفنوع ساختار VfslogCsr VfslogCsr;

/*
** Virtual table type for the vfslog reader module.
*/
ساختار VfslogVtab {
  sqlite3_vtab base;              /* Base class */
  sqlite3_file *pFd;              /* File descriptor open on vfslog file */
  sqlite3_int64 nByte;            /* Size of file in bytes */
  نویسه *zFile;                    /* File name for pFd */
};

/*
** Virtual table cursor type for the vfslog reader module.
*/
ساختار VfslogCsr {
  sqlite3_vtab_cursor base;       /* Base class */
  sqlite3_int64 iRowid;           /* Current rowid. */
  sqlite3_int64 iOffset;          /* Offset of next record in file */
  نویسه *zTransient;               /* Transient 'file' string */
  صحیح nFile;                      /* Size of array azFile[] */
  نویسه **azFile;                  /* File strings */
  unsigned نویسه aBuf[1024];       /* Current vfs log entry (read from file) */
};

ایستا unsigned صحیح get32bits(unsigned نویسه *p){
  بازگشت (p[0]<<24) + (p[1]<<16) + (p[2]<<8) + p[3];
}

/*
** The argument must point to a buffer containing a nul-terminated string.
** If the string begins with an SQL quote character it is overwritten by
** the dequoted version. Otherwise the buffer is left unmodified.
*/
ایستا تهی dequote(نویسه *z){
  نویسه quote;                     /* Quote character (if any ) */
  quote = z[0];
  اگر( quote=='[' || quote=='\'' || quote=='"' || quote=='`' ){
    صحیح iIn = 1;                  /* Index of next byte to read from input */
    صحیح iOut = 0;                 /* Index of next byte to write to output */
    اگر( quote=='[' ) quote = ']';  
    وقتی( z[iIn] ){
      اگر( z[iIn]==quote ){
        اگر( z[iIn+1]!=quote ) بشکن;
        z[iOut++] = quote;
        iIn += 2;
      }وگرنه{
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
ایستا صحیح vlogConnect(
  sqlite3 *db,
  تهی *pAux,
  صحیح argc, ثابت نویسه *ثابت*argv,
  sqlite3_vtab **ppVtab,
  نویسه **pzErr
){
  sqlite3_vfs *pVfs;              /* VFS used to read log file */
  صحیح flags;                      /* flags passed to pVfs->xOpen() */
  VfslogVtab *p;
  صحیح rc;
  صحیح nByte;
  نویسه *zFile;

  *ppVtab = 0;
  pVfs = sqlite3_vfs_find(0);
  nByte = اندازه(VfslogVtab) + pVfs->szOsFile + pVfs->mxPathname;
  p = sqlite3_malloc(nByte);
  اگر( p==0 ) بازگشت SQLITE_NOMEM;
  memset(p, 0, nByte);

  p->pFd = (sqlite3_file *)&p[1];
  p->zFile = &((نویسه *)p->pFd)[pVfs->szOsFile];

  zFile = sqlite3_mprintf("%s", argv[3]);
  اگر( !zFile ){
    sqlite3_free(p);
    بازگشت SQLITE_NOMEM;
  }
  dequote(zFile);
  pVfs->xFullPathname(pVfs, zFile, pVfs->mxPathname, p->zFile);
  sqlite3_free(zFile);

  flags = SQLITE_OPEN_READWRITE|SQLITE_OPEN_SUPER_JOURNAL;
  rc = pVfs->xOpen(pVfs, p->zFile, p->pFd, flags, &flags);

  اگر( rc==SQLITE_OK ){
    p->pFd->pMethods->xFileSize(p->pFd, &p->nByte);
    sqlite3_declare_vtab(db, 
        "CREATE TABLE xxx(event, file, click, rc, size, offset)"
    );
    *ppVtab = &p->base;
  }وگرنه{
    sqlite3_free(p);
  }

  بازگشت rc;
}

/*
** There is no "best-index". This virtual table always does a linear
** scan of the binary VFS log file.
*/
ایستا صحیح vlogBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  pIdxInfo->estimatedCost = 10.0;
  بازگشت SQLITE_OK;
}

/*
** Disconnect from or destroy a vfslog virtual table.
*/
ایستا صحیح vlogDisconnect(sqlite3_vtab *pVtab){
  VfslogVtab *p = (VfslogVtab *)pVtab;
  اگر( p->pFd->pMethods ){
    p->pFd->pMethods->xClose(p->pFd);
    p->pFd->pMethods = 0;
  }
  sqlite3_free(p);
  بازگشت SQLITE_OK;
}

/*
** Open a new vfslog cursor.
*/
ایستا صحیح vlogOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  VfslogCsr *pCsr;                /* Newly allocated cursor object */

  pCsr = sqlite3_malloc(اندازه(VfslogCsr));
  اگر( !pCsr ) بازگشت SQLITE_NOMEM;
  memset(pCsr, 0, اندازه(VfslogCsr));
  *ppCursor = &pCsr->base;
  بازگشت SQLITE_OK;
}

/*
** Close a vfslog cursor.
*/
ایستا صحیح vlogClose(sqlite3_vtab_cursor *pCursor){
  VfslogCsr *p = (VfslogCsr *)pCursor;
  صحیح i;
  برای(i=0; i<p->nFile; i++){
    sqlite3_free(p->azFile[i]);
  }
  sqlite3_free(p->azFile);
  sqlite3_free(p->zTransient);
  sqlite3_free(p);
  بازگشت SQLITE_OK;
}

/*
** Move a vfslog cursor to the next entry in the file.
*/
ایستا صحیح vlogNext(sqlite3_vtab_cursor *pCursor){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  VfslogVtab *p = (VfslogVtab *)pCursor->pVtab;
  صحیح rc = SQLITE_OK;
  صحیح nRead;

  sqlite3_free(pCsr->zTransient);
  pCsr->zTransient = 0;

  nRead = 24;
  اگر( pCsr->iOffset+nRead<=p->nByte ){
    صحیح eEvent;
    rc = p->pFd->pMethods->xRead(p->pFd, pCsr->aBuf, nRead, pCsr->iOffset);

    eEvent = get32bits(pCsr->aBuf);
    اگر( (rc==SQLITE_OK)
     && (eEvent==OS_OPEN || eEvent==OS_DELETE || eEvent==OS_ACCESS) 
    ){
      نویسه buf[4];
      rc = p->pFd->pMethods->xRead(p->pFd, buf, 4, pCsr->iOffset+nRead);
      nRead += 4;
      اگر( rc==SQLITE_OK ){
        صحیح nStr = get32bits((unsigned نویسه *)buf);
        نویسه *zStr = sqlite3_malloc(nStr+1);
        rc = p->pFd->pMethods->xRead(p->pFd, zStr, nStr, pCsr->iOffset+nRead);
        zStr[nStr] = '\0';
        nRead += nStr;

        اگر( eEvent==OS_OPEN ){
          صحیح iFileid = get32bits(&pCsr->aBuf[4]);
          اگر( iFileid>=pCsr->nFile ){
            صحیح nNew = اندازه(pCsr->azFile[0])*(iFileid+1);
            pCsr->azFile = (نویسه **)sqlite3_realloc(pCsr->azFile, nNew);
            nNew -= اندازه(pCsr->azFile[0])*pCsr->nFile;
            memset(&pCsr->azFile[pCsr->nFile], 0, nNew);
            pCsr->nFile = iFileid+1;
          }
          sqlite3_free(pCsr->azFile[iFileid]);
          pCsr->azFile[iFileid] = zStr;
        }وگرنه{
          pCsr->zTransient = zStr;
        }
      }
    }
  }

  pCsr->iRowid += 1;
  pCsr->iOffset += nRead;
  بازگشت rc;
}

ایستا صحیح vlogEof(sqlite3_vtab_cursor *pCursor){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  VfslogVtab *p = (VfslogVtab *)pCursor->pVtab;
  بازگشت (pCsr->iOffset>=p->nByte);
}

ایستا صحیح vlogFilter(
  sqlite3_vtab_cursor *pCursor, 
  صحیح idx_رقم, ثابت نویسه *idxStr,
  صحیح argc, sqlite3_مقدار **argv
){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  pCsr->iRowid = 0;
  pCsr->iOffset = 20;
  بازگشت vlogNext(pCursor);
}

ایستا صحیح vlogColumn(
  sqlite3_vtab_cursor *pCursor, 
  sqlite3_context *ctx, 
  صحیح i
){
  unsigned صحیح val;
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;

  أكد( i<7 );
  val = get32bits(&pCsr->aBuf[4*i]);

  انتخاب( i ){
    حالت 0: {
      sqlite3_result_text(ctx, vfslog_eventname(val), -1, SQLITE_STATIC);
      بشکن;
    }
    حالت 1: {
      نویسه *zStr = pCsr->zTransient;
      اگر( val!=0 && val<(unsigned)pCsr->nFile ){
        zStr = pCsr->azFile[val];
      }
      sqlite3_result_text(ctx, zStr, -1, SQLITE_TRANSIENT);
      بشکن;
    }
    پیشفرض:
      sqlite3_result_int(ctx, val);
      بشکن;
  }

  بازگشت SQLITE_OK;
}

ایستا صحیح vlogRowid(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  *pRowid = pCsr->iRowid;
  بازگشت SQLITE_OK;
}

صحیح sqlite3_vfslog_register(sqlite3 *db){
  ایستا sqlite3_module vfslog_module = {
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
  بازگشت SQLITE_OK;
}
#endif /* SQLITE_OMIT_VIRTUALTABLE */

/**************************************************************************
***************************************************************************
** Tcl interface starts here.
*/

#اگر defined(SQLITE_TEST) || defined(TCLSH)

#شامل "tclsqlite.h"

ایستا صحیح SQLITE_TCLAPI test_vfslog(
  تهی *clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  ساختار SqliteDb { sqlite3 *db; };
  sqlite3 *db;
  Tcl_CmdInfo cmdInfo;
  صحیح rc = SQLITE_ERROR;

  ایستا ثابت نویسه *strs[] = { "annotate", "finalize", "new",  "register", 0 };
  شمارش VL_enum { VL_ANNOTATE, VL_FINALIZE, VL_NEW, VL_REGISTER };
  صحیح iSub;

  اگر( objc<2 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "SUB-COMMAND ...");
    بازگشت TCL_ERROR;
  }
  اگر( Tcl_GetIndexFromObj(interp, objv[1], strs, "sub-command", 0, &iSub) ){
    بازگشت TCL_ERROR;
  }

  انتخاب( (شمارش VL_enum)iSub ){
    حالت VL_ANNOTATE: {
      نویسه *zVfs;
      نویسه *z_پیام;
      اگر( objc!=4 ){
        Tcl_Wrong_رقم_Args(interp, 3, objv, "VFS");
        بازگشت TCL_ERROR;
      }
      zVfs = Tcl_GetString(objv[2]);
      z_پیام = Tcl_GetString(objv[3]);
      rc = sqlite3_vfslog_annotate(zVfs, z_پیام);
      اگر( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp, "failed", (نویسه*)0);
        بازگشت TCL_ERROR;
      }
      بشکن;
    }
    حالت VL_FINALIZE: {
      نویسه *zVfs;
      اگر( objc!=3 ){
        Tcl_Wrong_رقم_Args(interp, 2, objv, "VFS");
        بازگشت TCL_ERROR;
      }
      zVfs = Tcl_GetString(objv[2]);
      rc = sqlite3_vfslog_finalize(zVfs);
      اگر( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp, "failed", (نویسه*)0);
        بازگشت TCL_ERROR;
      }
      بشکن;
    };

    حالت VL_NEW: {
      نویسه *zVfs;
      نویسه *zParent;
      نویسه *zLog;
      اگر( objc!=5 ){
        Tcl_Wrong_رقم_Args(interp, 2, objv, "VFS PARENT LOGFILE");
        بازگشت TCL_ERROR;
      }
      zVfs = Tcl_GetString(objv[2]);
      zParent = Tcl_GetString(objv[3]);
      zLog = Tcl_GetString(objv[4]);
      اگر( *zParent=='\0' ) zParent = 0;
      rc = sqlite3_vfslog_new(zVfs, zParent, zLog);
      اگر( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp, "failed", (نویسه*)0);
        بازگشت TCL_ERROR;
      }
      بشکن;
    };

    حالت VL_REGISTER: {
      نویسه *zDb;
      اگر( objc!=3 ){
        Tcl_Wrong_رقم_Args(interp, 2, objv, "DB");
        بازگشت TCL_ERROR;
      }
#ifdef SQLITE_OMIT_VIRTUALTABLE
      Tcl_AppendResult(interp, "vfslog not available because of "
                               "SQLITE_OMIT_VIRTUALTABLE", (تهی*)0);
      بازگشت TCL_ERROR;
#وگرنه
      zDb = Tcl_GetString(objv[2]);
      اگر( Tcl_GetCommandInfo(interp, zDb, &cmdInfo) ){
        db = ((ساختار SqliteDb*)cmdInfo.objClientData)->db;
        rc = sqlite3_vfslog_register(db);
      }
      اگر( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp, "bad sqlite3 handle: ", zDb, (تهی*)0);
        بازگشت TCL_ERROR;
      }
      بشکن;
#endif
    }
  }

  بازگشت TCL_OK;
}

صحیح SqlitetestOsinst_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "vfslog", test_vfslog, 0, 0);
  بازگشت TCL_OK;
}

#endif /* SQLITE_TEST */
