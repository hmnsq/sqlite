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

#அந்தர்பாவ "sqlite3.h"

#ifdef _WIN32
#அந்தர்பாவ <windows.h>
#endif

#அந்தர்பாவ <string.h>
#அந்தர்பாவ <assert.h>


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

வகைவரை கட்டமைப்பு VfslogVfs VfslogVfs;
வகைவரை கட்டமைப்பு VfslogFile VfslogFile;

கட்டமைப்பு VfslogVfs {
  sqlite3_vfs base;               /* VFS methods */
  sqlite3_vfs *pVfs;              /* Parent VFS */
  முழு iNextFileId;                /* Next file id */
  sqlite3_file *pLog;             /* Log file handle */
  sqlite3_int64 iOffset;          /* Log file offset of start of write buffer */
  முழு nBuf;                       /* Number of valid bytes in aBuf[] */
  எழுத்து aBuf[VFSLOG_BUFFERSIZE];   /* Write buffer */
};

கட்டமைப்பு VfslogFile {
  sqlite3_file base;              /* IO methods */
  sqlite3_file *pReal;            /* Underlying file handle */
  sqlite3_vfs *pVfslog;           /* Associated VsflogVfs object */
  முழு iFileId;                    /* File id number */
};

#define REALVFS(p) (((VfslogVfs *)(p))->pVfs)



/*
** Method declarations for vfslog_file.
*/
நிலையான முழு vfslogClose(sqlite3_file*);
நிலையான முழு vfslogRead(sqlite3_file*, வெறுமை*, முழு iAmt, sqlite3_int64 iOfst);
நிலையான முழு vfslogWrite(sqlite3_file*,மாறிலி வெறுமை*,முழு iAmt, sqlite3_int64 iOfst);
நிலையான முழு vfslogTruncate(sqlite3_file*, sqlite3_int64 size);
நிலையான முழு vfslogSync(sqlite3_file*, முழு flags);
நிலையான முழு vfslogFileSize(sqlite3_file*, sqlite3_int64 *pSize);
நிலையான முழு vfslogLock(sqlite3_file*, முழு);
நிலையான முழு vfslogUnlock(sqlite3_file*, முழு);
நிலையான முழு vfslogCheckReservedLock(sqlite3_file*, முழு *pResOut);
நிலையான முழு vfslogFileControl(sqlite3_file*, முழு op, வெறுமை *pArg);
நிலையான முழு vfslogSectorSize(sqlite3_file*);
நிலையான முழு vfslogDeviceCharacteristics(sqlite3_file*);

நிலையான முழு vfslogShmLock(sqlite3_file *pFile, முழு ofst, முழு n, முழு flags);
நிலையான முழு vfslogShmMap(sqlite3_file *pFile,முழு,முழு,முழு,நிலையற்ற வெறுமை **);
நிலையான வெறுமை vfslogShmBarrier(sqlite3_file*);
நிலையான முழு vfslogShmUnmap(sqlite3_file *pFile, முழு deleteFlag);

/*
** Method declarations for vfslog_vfs.
*/
நிலையான முழு vfslogOpen(sqlite3_vfs*, மாறிலி எழுத்து *, sqlite3_file*, முழு , முழு *);
நிலையான முழு vfslogDelete(sqlite3_vfs*, மாறிலி எழுத்து *zName, முழு syncDir);
நிலையான முழு vfslogAccess(sqlite3_vfs*, மாறிலி எழுத்து *zName, முழு flags, முழு *);
நிலையான முழு vfslogFullPathname(sqlite3_vfs*, மாறிலி எழுத்து *zName, முழு, எழுத்து *zOut);
நிலையான வெறுமை *vfslogDlOpen(sqlite3_vfs*, மாறிலி எழுத்து *zFilename);
நிலையான வெறுமை vfslogDlError(sqlite3_vfs*, முழு nByte, எழுத்து *z_Err_செய்தி);
நிலையான வெறுமை (*vfslogDlSym(sqlite3_vfs *pVfs, வெறுமை *p, மாறிலி எழுத்து*zSym))(வெறுமை);
நிலையான வெறுமை vfslogDlClose(sqlite3_vfs*, வெறுமை*);
நிலையான முழு vfslogRandomness(sqlite3_vfs*, முழு nByte, எழுத்து *zOut);
நிலையான முழு vfslogSleep(sqlite3_vfs*, முழு microseconds);
நிலையான முழு vfslogCurrentTime(sqlite3_vfs*, இரட்டை*);

நிலையான முழு vfslogGetLastError(sqlite3_vfs*, முழு, எழுத்து *);
நிலையான முழு vfslogCurrentTimeInt64(sqlite3_vfs*, sqlite3_int64*);

நிலையான sqlite3_vfs vfslog_vfs = {
  1,                              /* iVersion */
  அளவு(VfslogFile),             /* szOsFile */
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

நிலையான sqlite3_io_methods vfslog_io_methods = {
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
#அந்தர்பாவ <time.h>
நிலையான sqlite3_uint64 vfslog_time(){
  FILETIME ft;
  sqlite3_uint64 u64time = 0;
 
  GetSystemTimeAsFileTime(&ft);

  u64time |= ft.dw_உயர்வு_Date_Time;
  u64time <<= 32;
  u64time |= ft.dw_கீழ்_Date_Time;

  /* ft is 100-nanosecond intervals, we want microseconds */
  திரும்பு u64time /(sqlite3_uint64)10;
}
#அல்லதெனில் !defined(NO_GETTOD)
#அந்தர்பாவ <sys/time.h>
நிலையான sqlite3_uint64 vfslog_time(){
  கட்டமைப்பு timeval sTime;
  gettimeofday(&sTime, 0);
  திரும்பு sTime.tv_usec + (sqlite3_uint64)sTime.tv_sec * 1000000;
}
#இல்லையெனில்
நிலையான sqlite3_uint64 vfslog_time(){
  திரும்பு 0;
}
#endif

நிலையான வெறுமை vfslog_call(sqlite3_vfs *, முழு, முழு, sqlite3_int64, முழு, முழு, முழு);
நிலையான வெறுமை vfslog_string(sqlite3_vfs *, மாறிலி எழுத்து *);

/*
** Close an vfslog-file.
*/
நிலையான முழு vfslogClose(sqlite3_file *pFile){
  sqlite3_uint64 t;
  முழு rc = SQLITE_OK;
  VfslogFile *p = (VfslogFile *)pFile;

  t = vfslog_time();
  எனில்( p->pReal->pMethods ){
    rc = p->pReal->pMethods->xClose(p->pReal);
  }
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_CLOSE, p->iFileId, t, rc, 0, 0);
  திரும்பு rc;
}

/*
** Read data from an vfslog-file.
*/
நிலையான முழு vfslogRead(
  sqlite3_file *pFile, 
  வெறுமை *zBuf, 
  முழு iAmt, 
  sqlite_int64 iOfst
){
  முழு rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xRead(p->pReal, zBuf, iAmt, iOfst);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_READ, p->iFileId, t, rc, iAmt, (முழு)iOfst);
  திரும்பு rc;
}

/*
** Write data to an vfslog-file.
*/
நிலையான முழு vfslogWrite(
  sqlite3_file *pFile,
  மாறிலி வெறுமை *z,
  முழு iAmt,
  sqlite_int64 iOfst
){
  முழு rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xWrite(p->pReal, z, iAmt, iOfst);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_WRITE, p->iFileId, t, rc, iAmt, (முழு)iOfst);
  திரும்பு rc;
}

/*
** Truncate an vfslog-file.
*/
நிலையான முழு vfslogTruncate(sqlite3_file *pFile, sqlite_int64 size){
  முழு rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xTruncate(p->pReal, size);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_TRUNCATE, p->iFileId, t, rc, 0, (முழு)size);
  திரும்பு rc;
}

/*
** Sync an vfslog-file.
*/
நிலையான முழு vfslogSync(sqlite3_file *pFile, முழு flags){
  முழு rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xSync(p->pReal, flags);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SYNC, p->iFileId, t, rc, flags, 0);
  திரும்பு rc;
}

/*
** Return the current file-size of an vfslog-file.
*/
நிலையான முழு vfslogFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  முழு rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xFileSize(p->pReal, pSize);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_FILESIZE, p->iFileId, t, rc, 0, (முழு)*pSize);
  திரும்பு rc;
}

/*
** Lock an vfslog-file.
*/
நிலையான முழு vfslogLock(sqlite3_file *pFile, முழு eLock){
  முழு rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xLock(p->pReal, eLock);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_LOCK, p->iFileId, t, rc, eLock, 0);
  திரும்பு rc;
}

/*
** Unlock an vfslog-file.
*/
நிலையான முழு vfslogUnlock(sqlite3_file *pFile, முழு eLock){
  முழு rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xUnlock(p->pReal, eLock);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_UNLOCK, p->iFileId, t, rc, eLock, 0);
  திரும்பு rc;
}

/*
** Check if another file-handle holds a RESERVED lock on an vfslog-file.
*/
நிலையான முழு vfslogCheckReservedLock(sqlite3_file *pFile, முழு *pResOut){
  முழு rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xCheckReservedLock(p->pReal, pResOut);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_CHECKRESERVEDLOCK, p->iFileId, t, rc, *pResOut, 0);
  திரும்பு rc;
}

/*
** File control method. For custom operations on an vfslog-file.
*/
நிலையான முழு vfslogFileControl(sqlite3_file *pFile, முழு op, வெறுமை *pArg){
  VfslogFile *p = (VfslogFile *)pFile;
  முழு rc = p->pReal->pMethods->xFileControl(p->pReal, op, pArg);
  எனில்( op==SQLITE_FCNTL_VFSNAME && rc==SQLITE_OK ){
    *(எழுத்து**)pArg = sqlite3_mprintf("vfslog/%z", *(எழுத்து**)pArg);
  }
  திரும்பு rc;
}

/*
** Return the sector-size in bytes for an vfslog-file.
*/
நிலையான முழு vfslogSectorSize(sqlite3_file *pFile){
  முழு rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xSectorSize(p->pReal);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SECTORSIZE, p->iFileId, t, rc, 0, 0);
  திரும்பு rc;
}

/*
** Return the device characteristic flags supported by an vfslog-file.
*/
நிலையான முழு vfslogDeviceCharacteristics(sqlite3_file *pFile){
  முழு rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xDeviceCharacteristics(p->pReal);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_DEVCHAR, p->iFileId, t, rc, 0, 0);
  திரும்பு rc;
}

நிலையான முழு vfslogShmLock(sqlite3_file *pFile, முழு ofst, முழு n, முழு flags){
  முழு rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xShmLock(p->pReal, ofst, n, flags);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SHMLOCK, p->iFileId, t, rc, 0, 0);
  திரும்பு rc;
}
நிலையான முழு vfslogShmMap(
  sqlite3_file *pFile, 
  முழு iRegion, 
  முழு szRegion, 
  முழு isWrite, 
  நிலையற்ற வெறுமை **pp
){
  முழு rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xShmMap(p->pReal, iRegion, szRegion, isWrite, pp);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SHMMAP, p->iFileId, t, rc, 0, 0);
  திரும்பு rc;
}
நிலையான வெறுமை vfslogShmBarrier(sqlite3_file *pFile){
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  p->pReal->pMethods->xShmBarrier(p->pReal);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SHMBARRIER, p->iFileId, t, SQLITE_OK, 0, 0);
}
நிலையான முழு vfslogShmUnmap(sqlite3_file *pFile, முழு deleteFlag){
  முழு rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xShmUnmap(p->pReal, deleteFlag);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SHMUNMAP, p->iFileId, t, rc, 0, 0);
  திரும்பு rc;
}


/*
** Open an vfslog file handle.
*/
நிலையான முழு vfslogOpen(
  sqlite3_vfs *pVfs,
  மாறிலி எழுத்து *zName,
  sqlite3_file *pFile,
  முழு flags,
  முழு *pOutFlags
){
  முழு rc;
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
  திரும்பு rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
நிலையான முழு vfslogDelete(sqlite3_vfs *pVfs, மாறிலி எழுத்து *zPath, முழு dirSync){
  முழு rc;
  sqlite3_uint64 t;
  t = vfslog_time();
  rc = REALVFS(pVfs)->xDelete(REALVFS(pVfs), zPath, dirSync);
  t = vfslog_time() - t;
  vfslog_call(pVfs, OS_DELETE, 0, t, rc, dirSync, 0);
  vfslog_string(pVfs, zPath);
  திரும்பு rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
நிலையான முழு vfslogAccess(
  sqlite3_vfs *pVfs, 
  மாறிலி எழுத்து *zPath, 
  முழு flags, 
  முழு *pResOut
){
  முழு rc;
  sqlite3_uint64 t;
  t = vfslog_time();
  rc = REALVFS(pVfs)->xAccess(REALVFS(pVfs), zPath, flags, pResOut);
  t = vfslog_time() - t;
  vfslog_call(pVfs, OS_ACCESS, 0, t, rc, flags, *pResOut);
  vfslog_string(pVfs, zPath);
  திரும்பு rc;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
நிலையான முழு vfslogFullPathname(
  sqlite3_vfs *pVfs, 
  மாறிலி எழுத்து *zPath, 
  முழு nOut, 
  எழுத்து *zOut
){
  திரும்பு REALVFS(pVfs)->xFullPathname(REALVFS(pVfs), zPath, nOut, zOut);
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
நிலையான வெறுமை *vfslogDlOpen(sqlite3_vfs *pVfs, மாறிலி எழுத்து *zPath){
  திரும்பு REALVFS(pVfs)->xDlOpen(REALVFS(pVfs), zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
நிலையான வெறுமை vfslogDlError(sqlite3_vfs *pVfs, முழு nByte, எழுத்து *z_Err_செய்தி){
  REALVFS(pVfs)->xDlError(REALVFS(pVfs), nByte, z_Err_செய்தி);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
நிலையான வெறுமை (*vfslogDlSym(sqlite3_vfs *pVfs, வெறுமை *p, மாறிலி எழுத்து *zSym))(வெறுமை){
  திரும்பு REALVFS(pVfs)->xDlSym(REALVFS(pVfs), p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
நிலையான வெறுமை vfslogDlClose(sqlite3_vfs *pVfs, வெறுமை *pHandle){
  REALVFS(pVfs)->xDlClose(REALVFS(pVfs), pHandle);
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
நிலையான முழு vfslogRandomness(sqlite3_vfs *pVfs, முழு nByte, எழுத்து *zBufOut){
  திரும்பு REALVFS(pVfs)->xRandomness(REALVFS(pVfs), nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
நிலையான முழு vfslogSleep(sqlite3_vfs *pVfs, முழு nMicro){
  திரும்பு REALVFS(pVfs)->xSleep(REALVFS(pVfs), nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
நிலையான முழு vfslogCurrentTime(sqlite3_vfs *pVfs, இரட்டை *pTimeOut){
  திரும்பு REALVFS(pVfs)->xCurrentTime(REALVFS(pVfs), pTimeOut);
}

நிலையான முழு vfslogGetLastError(sqlite3_vfs *pVfs, முழு a, எழுத்து *b){
  திரும்பு REALVFS(pVfs)->xGetLastError(REALVFS(pVfs), a, b);
}
நிலையான முழு vfslogCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *p){
  திரும்பு REALVFS(pVfs)->xCurrentTimeInt64(REALVFS(pVfs), p);
}

நிலையான வெறுமை vfslog_flush(VfslogVfs *p){
#ifdef SQLITE_TEST
  வெளி முழு sqlite3_io_error_pending;
  வெளி முழு sqlite3_io_error_persist;
  வெளி முழு sqlite3_diskfull_pending;

  முழு pending = sqlite3_io_error_pending;
  முழு persist = sqlite3_io_error_persist;
  முழு diskfull = sqlite3_diskfull_pending;

  sqlite3_io_error_pending = 0;
  sqlite3_io_error_persist = 0;
  sqlite3_diskfull_pending = 0;
#endif

  எனில்( p->nBuf ){
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

நிலையான வெறுமை put32bits(unsigned எழுத்து *p, unsigned முழு v){
  p[0] = v>>24;
  p[1] = (unsigned எழுத்து)(v>>16);
  p[2] = (unsigned எழுத்து)(v>>8);
  p[3] = (unsigned எழுத்து)v;
}

நிலையான வெறுமை vfslog_call(
  sqlite3_vfs *pVfs,
  முழு eEvent,
  முழு iFileid,
  sqlite3_int64 nClick,
  முழு return_code,
  முழு size,
  முழு offset
){
  VfslogVfs *p = (VfslogVfs *)pVfs;
  unsigned எழுத்து *zRec;
  எனில்( (24+p->nBuf)>அளவு(p->aBuf) ){
    vfslog_flush(p);
  }
  zRec = (unsigned எழுத்து *)&p->aBuf[p->nBuf];
  put32bits(&zRec[0], eEvent);
  put32bits(&zRec[4], iFileid);
  put32bits(&zRec[8], (unsigned முழு)(nClick&0xffff));
  put32bits(&zRec[12], return_code);
  put32bits(&zRec[16], size);
  put32bits(&zRec[20], offset);
  p->nBuf += 24;
}

நிலையான வெறுமை vfslog_string(sqlite3_vfs *pVfs, மாறிலி எழுத்து *zStr){
  VfslogVfs *p = (VfslogVfs *)pVfs;
  unsigned எழுத்து *zRec;
  முழு nStr = zStr ? (முழு)strlen(zStr) : 0;
  எனில்( (4+nStr+p->nBuf)>அளவு(p->aBuf) ){
    vfslog_flush(p);
  }
  zRec = (unsigned எழுத்து *)&p->aBuf[p->nBuf];
  put32bits(&zRec[0], nStr);
  எனில்( zStr ){
    memcpy(&zRec[4], zStr, nStr);
  }
  p->nBuf += (4 + nStr);
}

நிலையான வெறுமை vfslog_finalize(VfslogVfs *p){
  எனில்( p->pLog->pMethods ){
    vfslog_flush(p);
    p->pLog->pMethods->xClose(p->pLog);
  }
  sqlite3_free(p);
}

முழு sqlite3_vfslog_finalize(மாறிலி எழுத்து *zVfs){
  sqlite3_vfs *pVfs;
  pVfs = sqlite3_vfs_find(zVfs);
  எனில்( !pVfs || pVfs->xOpen!=vfslogOpen ){
    திரும்பு SQLITE_ERROR;
  } 
  sqlite3_vfs_unregister(pVfs);
  vfslog_finalize((VfslogVfs *)pVfs);
  திரும்பு SQLITE_OK;
}

முழு sqlite3_vfslog_new(
  மாறிலி எழுத்து *zVfs,               /* New VFS name */
  மாறிலி எழுத்து *zParentVfs,         /* Parent VFS name (or NULL) */
  மாறிலி எழுத்து *zLog                /* Log file name */
){
  VfslogVfs *p;
  sqlite3_vfs *pParent;
  முழு nByte;
  முழு flags;
  முழு rc;
  எழுத்து *zFile;
  முழு nVfs;

  pParent = sqlite3_vfs_find(zParentVfs);
  எனில்( !pParent ){
    திரும்பு SQLITE_ERROR;
  }

  nVfs = (முழு)strlen(zVfs);
  nByte = அளவு(VfslogVfs) + pParent->szOsFile + nVfs+1+pParent->mxPathname+1;
  p = (VfslogVfs *)sqlite3_malloc(nByte);
  memset(p, 0, nByte);

  p->pVfs = pParent;
  p->pLog = (sqlite3_file *)&p[1];
  memcpy(&p->base, &vfslog_vfs, அளவு(sqlite3_vfs));
  p->base.zName = &((எழுத்து *)p->pLog)[pParent->szOsFile];
  p->base.szOsFile += pParent->szOsFile;
  memcpy((எழுத்து *)p->base.zName, zVfs, nVfs);

  zFile = (எழுத்து *)&p->base.zName[nVfs+1];
  pParent->xFullPathname(pParent, zLog, pParent->mxPathname, zFile);

  flags = SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE|SQLITE_OPEN_SUPER_JOURNAL;
  pParent->xDelete(pParent, zFile, 0);
  rc = pParent->xOpen(pParent, zFile, p->pLog, flags, &flags);
  எனில்( rc==SQLITE_OK ){
    memcpy(p->aBuf, "sqlite_ostrace1.....", 20);
    p->iOffset = 0;
    p->nBuf = 20;
    rc = sqlite3_vfs_register((sqlite3_vfs *)p, 1);
  }
  எனில்( rc ){
    vfslog_finalize(p);
  }
  திரும்பு rc;
}

முழு sqlite3_vfslog_annotate(மாறிலி எழுத்து *zVfs, மாறிலி எழுத்து *z_செய்தி){
  sqlite3_vfs *pVfs;
  pVfs = sqlite3_vfs_find(zVfs);
  எனில்( !pVfs || pVfs->xOpen!=vfslogOpen ){
    திரும்பு SQLITE_ERROR;
  } 
  vfslog_call(pVfs, OS_ANNOTATE, 0, 0, 0, 0, 0);
  vfslog_string(pVfs, z_செய்தி);
  திரும்பு SQLITE_OK;
}

நிலையான மாறிலி எழுத்து *vfslog_eventname(முழு eEvent){
  மாறிலி எழுத்து *zEvent = 0;

  தேர்வு( eEvent ){
    நிலை OS_CLOSE:             zEvent = "xClose"; நிறுத்து;
    நிலை OS_READ:              zEvent = "xRead"; நிறுத்து;
    நிலை OS_WRITE:             zEvent = "xWrite"; நிறுத்து;
    நிலை OS_TRUNCATE:          zEvent = "xTruncate"; நிறுத்து;
    நிலை OS_SYNC:              zEvent = "xSync"; நிறுத்து;
    நிலை OS_FILESIZE:          zEvent = "xFilesize"; நிறுத்து;
    நிலை OS_LOCK:              zEvent = "xLock"; நிறுத்து;
    நிலை OS_UNLOCK:            zEvent = "xUnlock"; நிறுத்து;
    நிலை OS_CHECKRESERVEDLOCK: zEvent = "xCheckResLock"; நிறுத்து;
    நிலை OS_FILECONTROL:       zEvent = "xFileControl"; நிறுத்து;
    நிலை OS_SECTORSIZE:        zEvent = "xSectorSize"; நிறுத்து;
    நிலை OS_DEVCHAR:           zEvent = "xDeviceChar"; நிறுத்து;
    நிலை OS_OPEN:              zEvent = "xOpen"; நிறுத்து;
    நிலை OS_DELETE:            zEvent = "xDelete"; நிறுத்து;
    நிலை OS_ACCESS:            zEvent = "xAccess"; நிறுத்து;
    நிலை OS_FULLPATHNAME:      zEvent = "xFullPathname"; நிறுத்து;
    நிலை OS_RANDOMNESS:        zEvent = "xRandomness"; நிறுத்து;
    நிலை OS_SLEEP:             zEvent = "xSleep"; நிறுத்து;
    நிலை OS_CURRENTTIME:       zEvent = "xCurrentTime"; நிறுத்து;

    நிலை OS_SHMUNMAP:          zEvent = "xShmUnmap"; நிறுத்து;
    நிலை OS_SHMLOCK:           zEvent = "xShmLock"; நிறுத்து;
    நிலை OS_SHMBARRIER:        zEvent = "xShmBarrier"; நிறுத்து;
    நிலை OS_SHMMAP:            zEvent = "xShmMap"; நிறுத்து;

    நிலை OS_ANNOTATE:          zEvent = "annotation"; நிறுத்து;
  }

  திரும்பு zEvent;
}

வகைவரை கட்டமைப்பு VfslogVtab VfslogVtab;
வகைவரை கட்டமைப்பு VfslogCsr VfslogCsr;

/*
** Virtual table type for the vfslog reader module.
*/
கட்டமைப்பு VfslogVtab {
  sqlite3_vtab base;              /* Base class */
  sqlite3_file *pFd;              /* File descriptor open on vfslog file */
  sqlite3_int64 nByte;            /* Size of file in bytes */
  எழுத்து *zFile;                    /* File name for pFd */
};

/*
** Virtual table cursor type for the vfslog reader module.
*/
கட்டமைப்பு VfslogCsr {
  sqlite3_vtab_cursor base;       /* Base class */
  sqlite3_int64 iRowid;           /* Current rowid. */
  sqlite3_int64 iOffset;          /* Offset of next record in file */
  எழுத்து *zTransient;               /* Transient 'file' string */
  முழு nFile;                      /* Size of array azFile[] */
  எழுத்து **azFile;                  /* File strings */
  unsigned எழுத்து aBuf[1024];       /* Current vfs log entry (read from file) */
};

நிலையான unsigned முழு get32bits(unsigned எழுத்து *p){
  திரும்பு (p[0]<<24) + (p[1]<<16) + (p[2]<<8) + p[3];
}

/*
** The argument must point to a buffer containing a nul-terminated string.
** If the string begins with an SQL quote character it is overwritten by
** the dequoted version. Otherwise the buffer is left unmodified.
*/
நிலையான வெறுமை dequote(எழுத்து *z){
  எழுத்து quote;                     /* Quote character (if any ) */
  quote = z[0];
  எனில்( quote=='[' || quote=='\'' || quote=='"' || quote=='`' ){
    முழு iIn = 1;                  /* Index of next byte to read from input */
    முழு iOut = 0;                 /* Index of next byte to write to output */
    எனில்( quote=='[' ) quote = ']';  
    போது( z[iIn] ){
      எனில்( z[iIn]==quote ){
        எனில்( z[iIn+1]!=quote ) நிறுத்து;
        z[iOut++] = quote;
        iIn += 2;
      }இல்லையெனில்{
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
நிலையான முழு vlogConnect(
  sqlite3 *db,
  வெறுமை *pAux,
  முழு argc, மாறிலி எழுத்து *மாறிலி*argv,
  sqlite3_vtab **ppVtab,
  எழுத்து **pzErr
){
  sqlite3_vfs *pVfs;              /* VFS used to read log file */
  முழு flags;                      /* flags passed to pVfs->xOpen() */
  VfslogVtab *p;
  முழு rc;
  முழு nByte;
  எழுத்து *zFile;

  *ppVtab = 0;
  pVfs = sqlite3_vfs_find(0);
  nByte = அளவு(VfslogVtab) + pVfs->szOsFile + pVfs->mxPathname;
  p = sqlite3_malloc(nByte);
  எனில்( p==0 ) திரும்பு SQLITE_NOMEM;
  memset(p, 0, nByte);

  p->pFd = (sqlite3_file *)&p[1];
  p->zFile = &((எழுத்து *)p->pFd)[pVfs->szOsFile];

  zFile = sqlite3_mprintf("%s", argv[3]);
  எனில்( !zFile ){
    sqlite3_free(p);
    திரும்பு SQLITE_NOMEM;
  }
  dequote(zFile);
  pVfs->xFullPathname(pVfs, zFile, pVfs->mxPathname, p->zFile);
  sqlite3_free(zFile);

  flags = SQLITE_OPEN_READWRITE|SQLITE_OPEN_SUPER_JOURNAL;
  rc = pVfs->xOpen(pVfs, p->zFile, p->pFd, flags, &flags);

  எனில்( rc==SQLITE_OK ){
    p->pFd->pMethods->xFileSize(p->pFd, &p->nByte);
    sqlite3_declare_vtab(db, 
        "CREATE TABLE xxx(event, file, click, rc, size, offset)"
    );
    *ppVtab = &p->base;
  }இல்லையெனில்{
    sqlite3_free(p);
  }

  திரும்பு rc;
}

/*
** There is no "best-index". This virtual table always does a linear
** scan of the binary VFS log file.
*/
நிலையான முழு vlogBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  pIdxInfo->estimatedCost = 10.0;
  திரும்பு SQLITE_OK;
}

/*
** Disconnect from or destroy a vfslog virtual table.
*/
நிலையான முழு vlogDisconnect(sqlite3_vtab *pVtab){
  VfslogVtab *p = (VfslogVtab *)pVtab;
  எனில்( p->pFd->pMethods ){
    p->pFd->pMethods->xClose(p->pFd);
    p->pFd->pMethods = 0;
  }
  sqlite3_free(p);
  திரும்பு SQLITE_OK;
}

/*
** Open a new vfslog cursor.
*/
நிலையான முழு vlogOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  VfslogCsr *pCsr;                /* Newly allocated cursor object */

  pCsr = sqlite3_malloc(அளவு(VfslogCsr));
  எனில்( !pCsr ) திரும்பு SQLITE_NOMEM;
  memset(pCsr, 0, அளவு(VfslogCsr));
  *ppCursor = &pCsr->base;
  திரும்பு SQLITE_OK;
}

/*
** Close a vfslog cursor.
*/
நிலையான முழு vlogClose(sqlite3_vtab_cursor *pCursor){
  VfslogCsr *p = (VfslogCsr *)pCursor;
  முழு i;
  க்கு(i=0; i<p->nFile; i++){
    sqlite3_free(p->azFile[i]);
  }
  sqlite3_free(p->azFile);
  sqlite3_free(p->zTransient);
  sqlite3_free(p);
  திரும்பு SQLITE_OK;
}

/*
** Move a vfslog cursor to the next entry in the file.
*/
நிலையான முழு vlogNext(sqlite3_vtab_cursor *pCursor){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  VfslogVtab *p = (VfslogVtab *)pCursor->pVtab;
  முழு rc = SQLITE_OK;
  முழு nRead;

  sqlite3_free(pCsr->zTransient);
  pCsr->zTransient = 0;

  nRead = 24;
  எனில்( pCsr->iOffset+nRead<=p->nByte ){
    முழு eEvent;
    rc = p->pFd->pMethods->xRead(p->pFd, pCsr->aBuf, nRead, pCsr->iOffset);

    eEvent = get32bits(pCsr->aBuf);
    எனில்( (rc==SQLITE_OK)
     && (eEvent==OS_OPEN || eEvent==OS_DELETE || eEvent==OS_ACCESS) 
    ){
      எழுத்து buf[4];
      rc = p->pFd->pMethods->xRead(p->pFd, buf, 4, pCsr->iOffset+nRead);
      nRead += 4;
      எனில்( rc==SQLITE_OK ){
        முழு nStr = get32bits((unsigned எழுத்து *)buf);
        எழுத்து *zStr = sqlite3_malloc(nStr+1);
        rc = p->pFd->pMethods->xRead(p->pFd, zStr, nStr, pCsr->iOffset+nRead);
        zStr[nStr] = '\0';
        nRead += nStr;

        எனில்( eEvent==OS_OPEN ){
          முழு iFileid = get32bits(&pCsr->aBuf[4]);
          எனில்( iFileid>=pCsr->nFile ){
            முழு nNew = அளவு(pCsr->azFile[0])*(iFileid+1);
            pCsr->azFile = (எழுத்து **)sqlite3_realloc(pCsr->azFile, nNew);
            nNew -= அளவு(pCsr->azFile[0])*pCsr->nFile;
            memset(&pCsr->azFile[pCsr->nFile], 0, nNew);
            pCsr->nFile = iFileid+1;
          }
          sqlite3_free(pCsr->azFile[iFileid]);
          pCsr->azFile[iFileid] = zStr;
        }இல்லையெனில்{
          pCsr->zTransient = zStr;
        }
      }
    }
  }

  pCsr->iRowid += 1;
  pCsr->iOffset += nRead;
  திரும்பு rc;
}

நிலையான முழு vlogEof(sqlite3_vtab_cursor *pCursor){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  VfslogVtab *p = (VfslogVtab *)pCursor->pVtab;
  திரும்பு (pCsr->iOffset>=p->nByte);
}

நிலையான முழு vlogFilter(
  sqlite3_vtab_cursor *pCursor, 
  முழு idx_இலக்கம், மாறிலி எழுத்து *idxStr,
  முழு argc, sqlite3_மதிப்பு **argv
){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  pCsr->iRowid = 0;
  pCsr->iOffset = 20;
  திரும்பு vlogNext(pCursor);
}

நிலையான முழு vlogColumn(
  sqlite3_vtab_cursor *pCursor, 
  sqlite3_context *ctx, 
  முழு i
){
  unsigned முழு val;
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;

  ப்ரதிஜ்ஞா( i<7 );
  val = get32bits(&pCsr->aBuf[4*i]);

  தேர்வு( i ){
    நிலை 0: {
      sqlite3_result_text(ctx, vfslog_eventname(val), -1, SQLITE_STATIC);
      நிறுத்து;
    }
    நிலை 1: {
      எழுத்து *zStr = pCsr->zTransient;
      எனில்( val!=0 && val<(unsigned)pCsr->nFile ){
        zStr = pCsr->azFile[val];
      }
      sqlite3_result_text(ctx, zStr, -1, SQLITE_TRANSIENT);
      நிறுத்து;
    }
    இயல்பு:
      sqlite3_result_int(ctx, val);
      நிறுத்து;
  }

  திரும்பு SQLITE_OK;
}

நிலையான முழு vlogRowid(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  *pRowid = pCsr->iRowid;
  திரும்பு SQLITE_OK;
}

முழு sqlite3_vfslog_register(sqlite3 *db){
  நிலையான sqlite3_module vfslog_module = {
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
  திரும்பு SQLITE_OK;
}
#endif /* SQLITE_OMIT_VIRTUALTABLE */

/**************************************************************************
***************************************************************************
** Tcl interface starts here.
*/

#எனில் defined(SQLITE_TEST) || defined(TCLSH)

#அந்தர்பாவ "tclsqlite.h"

நிலையான முழு SQLITE_TCLAPI test_vfslog(
  வெறுமை *clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  கட்டமைப்பு SqliteDb { sqlite3 *db; };
  sqlite3 *db;
  Tcl_CmdInfo cmdInfo;
  முழு rc = SQLITE_ERROR;

  நிலையான மாறிலி எழுத்து *strs[] = { "annotate", "finalize", "new",  "register", 0 };
  எண்ணிக்கை VL_enum { VL_ANNOTATE, VL_FINALIZE, VL_NEW, VL_REGISTER };
  முழு iSub;

  எனில்( objc<2 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "SUB-COMMAND ...");
    திரும்பு TCL_ERROR;
  }
  எனில்( Tcl_GetIndexFromObj(interp, objv[1], strs, "sub-command", 0, &iSub) ){
    திரும்பு TCL_ERROR;
  }

  தேர்வு( (எண்ணிக்கை VL_enum)iSub ){
    நிலை VL_ANNOTATE: {
      எழுத்து *zVfs;
      எழுத்து *z_செய்தி;
      எனில்( objc!=4 ){
        Tcl_Wrong_இலக்கம்_Args(interp, 3, objv, "VFS");
        திரும்பு TCL_ERROR;
      }
      zVfs = Tcl_GetString(objv[2]);
      z_செய்தி = Tcl_GetString(objv[3]);
      rc = sqlite3_vfslog_annotate(zVfs, z_செய்தி);
      எனில்( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp, "failed", (எழுத்து*)0);
        திரும்பு TCL_ERROR;
      }
      நிறுத்து;
    }
    நிலை VL_FINALIZE: {
      எழுத்து *zVfs;
      எனில்( objc!=3 ){
        Tcl_Wrong_இலக்கம்_Args(interp, 2, objv, "VFS");
        திரும்பு TCL_ERROR;
      }
      zVfs = Tcl_GetString(objv[2]);
      rc = sqlite3_vfslog_finalize(zVfs);
      எனில்( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp, "failed", (எழுத்து*)0);
        திரும்பு TCL_ERROR;
      }
      நிறுத்து;
    };

    நிலை VL_NEW: {
      எழுத்து *zVfs;
      எழுத்து *zParent;
      எழுத்து *zLog;
      எனில்( objc!=5 ){
        Tcl_Wrong_இலக்கம்_Args(interp, 2, objv, "VFS PARENT LOGFILE");
        திரும்பு TCL_ERROR;
      }
      zVfs = Tcl_GetString(objv[2]);
      zParent = Tcl_GetString(objv[3]);
      zLog = Tcl_GetString(objv[4]);
      எனில்( *zParent=='\0' ) zParent = 0;
      rc = sqlite3_vfslog_new(zVfs, zParent, zLog);
      எனில்( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp, "failed", (எழுத்து*)0);
        திரும்பு TCL_ERROR;
      }
      நிறுத்து;
    };

    நிலை VL_REGISTER: {
      எழுத்து *zDb;
      எனில்( objc!=3 ){
        Tcl_Wrong_இலக்கம்_Args(interp, 2, objv, "DB");
        திரும்பு TCL_ERROR;
      }
#ifdef SQLITE_OMIT_VIRTUALTABLE
      Tcl_AppendResult(interp, "vfslog not available because of "
                               "SQLITE_OMIT_VIRTUALTABLE", (வெறுமை*)0);
      திரும்பு TCL_ERROR;
#இல்லையெனில்
      zDb = Tcl_GetString(objv[2]);
      எனில்( Tcl_GetCommandInfo(interp, zDb, &cmdInfo) ){
        db = ((கட்டமைப்பு SqliteDb*)cmdInfo.objClientData)->db;
        rc = sqlite3_vfslog_register(db);
      }
      எனில்( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp, "bad sqlite3 handle: ", zDb, (வெறுமை*)0);
        திரும்பு TCL_ERROR;
      }
      நிறுத்து;
#endif
    }
  }

  திரும்பு TCL_OK;
}

முழு SqlitetestOsinst_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "vfslog", test_vfslog, 0, 0);
  திரும்பு TCL_OK;
}

#endif /* SQLITE_TEST */
