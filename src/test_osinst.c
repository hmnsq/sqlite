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

#অন্তর্ভাব "sqlite3.h"

#ifdef _WIN32
#অন্তর্ভাব <windows.h>
#endif

#অন্তর্ভাব <string.h>
#অন্তর্ভাব <assert.h>


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

প্রকারনির্দেশ সংরচনা VfslogVfs VfslogVfs;
প্রকারনির্দেশ সংরচনা VfslogFile VfslogFile;

সংরচনা VfslogVfs {
  sqlite3_vfs base;               /* VFS methods */
  sqlite3_vfs *pVfs;              /* Parent VFS */
  পূর্ণাঙ্ক iNextFileId;                /* Next file id */
  sqlite3_file *pLog;             /* Log file handle */
  sqlite3_int64 iOffset;          /* Log file offset of start of write buffer */
  পূর্ণাঙ্ক nBuf;                       /* Number of valid bytes in aBuf[] */
  বর্ণ aBuf[VFSLOG_BUFFERSIZE];   /* Write buffer */
};

সংরচনা VfslogFile {
  sqlite3_file base;              /* IO methods */
  sqlite3_file *pReal;            /* Underlying file handle */
  sqlite3_vfs *pVfslog;           /* Associated VsflogVfs object */
  পূর্ণাঙ্ক iFileId;                    /* File id number */
};

#define REALVFS(p) (((VfslogVfs *)(p))->pVfs)



/*
** Method declarations for vfslog_file.
*/
স্থির পূর্ণাঙ্ক vfslogClose(sqlite3_file*);
স্থির পূর্ণাঙ্ক vfslogRead(sqlite3_file*, শূন্য*, পূর্ণাঙ্ক iAmt, sqlite3_int64 iOfst);
স্থির পূর্ণাঙ্ক vfslogWrite(sqlite3_file*,ধ্রুব শূন্য*,পূর্ণাঙ্ক iAmt, sqlite3_int64 iOfst);
স্থির পূর্ণাঙ্ক vfslogTruncate(sqlite3_file*, sqlite3_int64 size);
স্থির পূর্ণাঙ্ক vfslogSync(sqlite3_file*, পূর্ণাঙ্ক flags);
স্থির পূর্ণাঙ্ক vfslogFileSize(sqlite3_file*, sqlite3_int64 *pSize);
স্থির পূর্ণাঙ্ক vfslogLock(sqlite3_file*, পূর্ণাঙ্ক);
স্থির পূর্ণাঙ্ক vfslogUnlock(sqlite3_file*, পূর্ণাঙ্ক);
স্থির পূর্ণাঙ্ক vfslogCheckReservedLock(sqlite3_file*, পূর্ণাঙ্ক *pResOut);
স্থির পূর্ণাঙ্ক vfslogFileControl(sqlite3_file*, পূর্ণাঙ্ক op, শূন্য *pArg);
স্থির পূর্ণাঙ্ক vfslogSectorSize(sqlite3_file*);
স্থির পূর্ণাঙ্ক vfslogDeviceCharacteristics(sqlite3_file*);

স্থির পূর্ণাঙ্ক vfslogShmLock(sqlite3_file *pFile, পূর্ণাঙ্ক ofst, পূর্ণাঙ্ক n, পূর্ণাঙ্ক flags);
স্থির পূর্ণাঙ্ক vfslogShmMap(sqlite3_file *pFile,পূর্ণাঙ্ক,পূর্ণাঙ্ক,পূর্ণাঙ্ক,চঞ্চল শূন্য **);
স্থির শূন্য vfslogShmBarrier(sqlite3_file*);
স্থির পূর্ণাঙ্ক vfslogShmUnmap(sqlite3_file *pFile, পূর্ণাঙ্ক deleteFlag);

/*
** Method declarations for vfslog_vfs.
*/
স্থির পূর্ণাঙ্ক vfslogOpen(sqlite3_vfs*, ধ্রুব বর্ণ *, sqlite3_file*, পূর্ণাঙ্ক , পূর্ণাঙ্ক *);
স্থির পূর্ণাঙ্ক vfslogDelete(sqlite3_vfs*, ধ্রুব বর্ণ *zName, পূর্ণাঙ্ক syncDir);
স্থির পূর্ণাঙ্ক vfslogAccess(sqlite3_vfs*, ধ্রুব বর্ণ *zName, পূর্ণাঙ্ক flags, পূর্ণাঙ্ক *);
স্থির পূর্ণাঙ্ক vfslogFullPathname(sqlite3_vfs*, ধ্রুব বর্ণ *zName, পূর্ণাঙ্ক, বর্ণ *zOut);
স্থির শূন্য *vfslogDlOpen(sqlite3_vfs*, ধ্রুব বর্ণ *zFilename);
স্থির শূন্য vfslogDlError(sqlite3_vfs*, পূর্ণাঙ্ক nByte, বর্ণ *z_Err_বার্তা);
স্থির শূন্য (*vfslogDlSym(sqlite3_vfs *pVfs, শূন্য *p, ধ্রুব বর্ণ*zSym))(শূন্য);
স্থির শূন্য vfslogDlClose(sqlite3_vfs*, শূন্য*);
স্থির পূর্ণাঙ্ক vfslogRandomness(sqlite3_vfs*, পূর্ণাঙ্ক nByte, বর্ণ *zOut);
স্থির পূর্ণাঙ্ক vfslogSleep(sqlite3_vfs*, পূর্ণাঙ্ক microseconds);
স্থির পূর্ণাঙ্ক vfslogCurrentTime(sqlite3_vfs*, দ্বিগুণ*);

স্থির পূর্ণাঙ্ক vfslogGetLastError(sqlite3_vfs*, পূর্ণাঙ্ক, বর্ণ *);
স্থির পূর্ণাঙ্ক vfslogCurrentTimeInt64(sqlite3_vfs*, sqlite3_int64*);

স্থির sqlite3_vfs vfslog_vfs = {
  1,                              /* iVersion */
  পরিমাণ(VfslogFile),             /* szOsFile */
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

স্থির sqlite3_io_methods vfslog_io_methods = {
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
#অন্তর্ভাব <time.h>
স্থির sqlite3_uint64 vfslog_time(){
  FILETIME ft;
  sqlite3_uint64 u64time = 0;
 
  GetSystemTimeAsFileTime(&ft);

  u64time |= ft.dw_উপরের_Date_Time;
  u64time <<= 32;
  u64time |= ft.dw_নিচের_Date_Time;

  /* ft is 100-nanosecond intervals, we want microseconds */
  প্রত্যাগম u64time /(sqlite3_uint64)10;
}
#নোচেদ্যদি !defined(NO_GETTOD)
#অন্তর্ভাব <sys/time.h>
স্থির sqlite3_uint64 vfslog_time(){
  সংরচনা timeval sTime;
  gettimeofday(&sTime, 0);
  প্রত্যাগম sTime.tv_usec + (sqlite3_uint64)sTime.tv_sec * 1000000;
}
#অন্যথা
স্থির sqlite3_uint64 vfslog_time(){
  প্রত্যাগম 0;
}
#endif

স্থির শূন্য vfslog_call(sqlite3_vfs *, পূর্ণাঙ্ক, পূর্ণাঙ্ক, sqlite3_int64, পূর্ণাঙ্ক, পূর্ণাঙ্ক, পূর্ণাঙ্ক);
স্থির শূন্য vfslog_string(sqlite3_vfs *, ধ্রুব বর্ণ *);

/*
** Close an vfslog-file.
*/
স্থির পূর্ণাঙ্ক vfslogClose(sqlite3_file *pFile){
  sqlite3_uint64 t;
  পূর্ণাঙ্ক rc = SQLITE_OK;
  VfslogFile *p = (VfslogFile *)pFile;

  t = vfslog_time();
  যদি( p->pReal->pMethods ){
    rc = p->pReal->pMethods->xClose(p->pReal);
  }
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_CLOSE, p->iFileId, t, rc, 0, 0);
  প্রত্যাগম rc;
}

/*
** Read data from an vfslog-file.
*/
স্থির পূর্ণাঙ্ক vfslogRead(
  sqlite3_file *pFile, 
  শূন্য *zBuf, 
  পূর্ণাঙ্ক iAmt, 
  sqlite_int64 iOfst
){
  পূর্ণাঙ্ক rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xRead(p->pReal, zBuf, iAmt, iOfst);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_READ, p->iFileId, t, rc, iAmt, (পূর্ণাঙ্ক)iOfst);
  প্রত্যাগম rc;
}

/*
** Write data to an vfslog-file.
*/
স্থির পূর্ণাঙ্ক vfslogWrite(
  sqlite3_file *pFile,
  ধ্রুব শূন্য *z,
  পূর্ণাঙ্ক iAmt,
  sqlite_int64 iOfst
){
  পূর্ণাঙ্ক rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xWrite(p->pReal, z, iAmt, iOfst);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_WRITE, p->iFileId, t, rc, iAmt, (পূর্ণাঙ্ক)iOfst);
  প্রত্যাগম rc;
}

/*
** Truncate an vfslog-file.
*/
স্থির পূর্ণাঙ্ক vfslogTruncate(sqlite3_file *pFile, sqlite_int64 size){
  পূর্ণাঙ্ক rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xTruncate(p->pReal, size);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_TRUNCATE, p->iFileId, t, rc, 0, (পূর্ণাঙ্ক)size);
  প্রত্যাগম rc;
}

/*
** Sync an vfslog-file.
*/
স্থির পূর্ণাঙ্ক vfslogSync(sqlite3_file *pFile, পূর্ণাঙ্ক flags){
  পূর্ণাঙ্ক rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xSync(p->pReal, flags);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SYNC, p->iFileId, t, rc, flags, 0);
  প্রত্যাগম rc;
}

/*
** Return the current file-size of an vfslog-file.
*/
স্থির পূর্ণাঙ্ক vfslogFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  পূর্ণাঙ্ক rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xFileSize(p->pReal, pSize);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_FILESIZE, p->iFileId, t, rc, 0, (পূর্ণাঙ্ক)*pSize);
  প্রত্যাগম rc;
}

/*
** Lock an vfslog-file.
*/
স্থির পূর্ণাঙ্ক vfslogLock(sqlite3_file *pFile, পূর্ণাঙ্ক eLock){
  পূর্ণাঙ্ক rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xLock(p->pReal, eLock);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_LOCK, p->iFileId, t, rc, eLock, 0);
  প্রত্যাগম rc;
}

/*
** Unlock an vfslog-file.
*/
স্থির পূর্ণাঙ্ক vfslogUnlock(sqlite3_file *pFile, পূর্ণাঙ্ক eLock){
  পূর্ণাঙ্ক rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xUnlock(p->pReal, eLock);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_UNLOCK, p->iFileId, t, rc, eLock, 0);
  প্রত্যাগম rc;
}

/*
** Check if another file-handle holds a RESERVED lock on an vfslog-file.
*/
স্থির পূর্ণাঙ্ক vfslogCheckReservedLock(sqlite3_file *pFile, পূর্ণাঙ্ক *pResOut){
  পূর্ণাঙ্ক rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xCheckReservedLock(p->pReal, pResOut);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_CHECKRESERVEDLOCK, p->iFileId, t, rc, *pResOut, 0);
  প্রত্যাগম rc;
}

/*
** File control method. For custom operations on an vfslog-file.
*/
স্থির পূর্ণাঙ্ক vfslogFileControl(sqlite3_file *pFile, পূর্ণাঙ্ক op, শূন্য *pArg){
  VfslogFile *p = (VfslogFile *)pFile;
  পূর্ণাঙ্ক rc = p->pReal->pMethods->xFileControl(p->pReal, op, pArg);
  যদি( op==SQLITE_FCNTL_VFSNAME && rc==SQLITE_OK ){
    *(বর্ণ**)pArg = sqlite3_mprintf("vfslog/%z", *(বর্ণ**)pArg);
  }
  প্রত্যাগম rc;
}

/*
** Return the sector-size in bytes for an vfslog-file.
*/
স্থির পূর্ণাঙ্ক vfslogSectorSize(sqlite3_file *pFile){
  পূর্ণাঙ্ক rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xSectorSize(p->pReal);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SECTORSIZE, p->iFileId, t, rc, 0, 0);
  প্রত্যাগম rc;
}

/*
** Return the device characteristic flags supported by an vfslog-file.
*/
স্থির পূর্ণাঙ্ক vfslogDeviceCharacteristics(sqlite3_file *pFile){
  পূর্ণাঙ্ক rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xDeviceCharacteristics(p->pReal);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_DEVCHAR, p->iFileId, t, rc, 0, 0);
  প্রত্যাগম rc;
}

স্থির পূর্ণাঙ্ক vfslogShmLock(sqlite3_file *pFile, পূর্ণাঙ্ক ofst, পূর্ণাঙ্ক n, পূর্ণাঙ্ক flags){
  পূর্ণাঙ্ক rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xShmLock(p->pReal, ofst, n, flags);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SHMLOCK, p->iFileId, t, rc, 0, 0);
  প্রত্যাগম rc;
}
স্থির পূর্ণাঙ্ক vfslogShmMap(
  sqlite3_file *pFile, 
  পূর্ণাঙ্ক iRegion, 
  পূর্ণাঙ্ক szRegion, 
  পূর্ণাঙ্ক isWrite, 
  চঞ্চল শূন্য **pp
){
  পূর্ণাঙ্ক rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xShmMap(p->pReal, iRegion, szRegion, isWrite, pp);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SHMMAP, p->iFileId, t, rc, 0, 0);
  প্রত্যাগম rc;
}
স্থির শূন্য vfslogShmBarrier(sqlite3_file *pFile){
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  p->pReal->pMethods->xShmBarrier(p->pReal);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SHMBARRIER, p->iFileId, t, SQLITE_OK, 0, 0);
}
স্থির পূর্ণাঙ্ক vfslogShmUnmap(sqlite3_file *pFile, পূর্ণাঙ্ক deleteFlag){
  পূর্ণাঙ্ক rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xShmUnmap(p->pReal, deleteFlag);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SHMUNMAP, p->iFileId, t, rc, 0, 0);
  প্রত্যাগম rc;
}


/*
** Open an vfslog file handle.
*/
স্থির পূর্ণাঙ্ক vfslogOpen(
  sqlite3_vfs *pVfs,
  ধ্রুব বর্ণ *zName,
  sqlite3_file *pFile,
  পূর্ণাঙ্ক flags,
  পূর্ণাঙ্ক *pOutFlags
){
  পূর্ণাঙ্ক rc;
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
  প্রত্যাগম rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
স্থির পূর্ণাঙ্ক vfslogDelete(sqlite3_vfs *pVfs, ধ্রুব বর্ণ *zPath, পূর্ণাঙ্ক dirSync){
  পূর্ণাঙ্ক rc;
  sqlite3_uint64 t;
  t = vfslog_time();
  rc = REALVFS(pVfs)->xDelete(REALVFS(pVfs), zPath, dirSync);
  t = vfslog_time() - t;
  vfslog_call(pVfs, OS_DELETE, 0, t, rc, dirSync, 0);
  vfslog_string(pVfs, zPath);
  প্রত্যাগম rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
স্থির পূর্ণাঙ্ক vfslogAccess(
  sqlite3_vfs *pVfs, 
  ধ্রুব বর্ণ *zPath, 
  পূর্ণাঙ্ক flags, 
  পূর্ণাঙ্ক *pResOut
){
  পূর্ণাঙ্ক rc;
  sqlite3_uint64 t;
  t = vfslog_time();
  rc = REALVFS(pVfs)->xAccess(REALVFS(pVfs), zPath, flags, pResOut);
  t = vfslog_time() - t;
  vfslog_call(pVfs, OS_ACCESS, 0, t, rc, flags, *pResOut);
  vfslog_string(pVfs, zPath);
  প্রত্যাগম rc;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
স্থির পূর্ণাঙ্ক vfslogFullPathname(
  sqlite3_vfs *pVfs, 
  ধ্রুব বর্ণ *zPath, 
  পূর্ণাঙ্ক nOut, 
  বর্ণ *zOut
){
  প্রত্যাগম REALVFS(pVfs)->xFullPathname(REALVFS(pVfs), zPath, nOut, zOut);
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
স্থির শূন্য *vfslogDlOpen(sqlite3_vfs *pVfs, ধ্রুব বর্ণ *zPath){
  প্রত্যাগম REALVFS(pVfs)->xDlOpen(REALVFS(pVfs), zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
স্থির শূন্য vfslogDlError(sqlite3_vfs *pVfs, পূর্ণাঙ্ক nByte, বর্ণ *z_Err_বার্তা){
  REALVFS(pVfs)->xDlError(REALVFS(pVfs), nByte, z_Err_বার্তা);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
স্থির শূন্য (*vfslogDlSym(sqlite3_vfs *pVfs, শূন্য *p, ধ্রুব বর্ণ *zSym))(শূন্য){
  প্রত্যাগম REALVFS(pVfs)->xDlSym(REALVFS(pVfs), p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
স্থির শূন্য vfslogDlClose(sqlite3_vfs *pVfs, শূন্য *pHandle){
  REALVFS(pVfs)->xDlClose(REALVFS(pVfs), pHandle);
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
স্থির পূর্ণাঙ্ক vfslogRandomness(sqlite3_vfs *pVfs, পূর্ণাঙ্ক nByte, বর্ণ *zBufOut){
  প্রত্যাগম REALVFS(pVfs)->xRandomness(REALVFS(pVfs), nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
স্থির পূর্ণাঙ্ক vfslogSleep(sqlite3_vfs *pVfs, পূর্ণাঙ্ক nMicro){
  প্রত্যাগম REALVFS(pVfs)->xSleep(REALVFS(pVfs), nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
স্থির পূর্ণাঙ্ক vfslogCurrentTime(sqlite3_vfs *pVfs, দ্বিগুণ *pTimeOut){
  প্রত্যাগম REALVFS(pVfs)->xCurrentTime(REALVFS(pVfs), pTimeOut);
}

স্থির পূর্ণাঙ্ক vfslogGetLastError(sqlite3_vfs *pVfs, পূর্ণাঙ্ক a, বর্ণ *b){
  প্রত্যাগম REALVFS(pVfs)->xGetLastError(REALVFS(pVfs), a, b);
}
স্থির পূর্ণাঙ্ক vfslogCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *p){
  প্রত্যাগম REALVFS(pVfs)->xCurrentTimeInt64(REALVFS(pVfs), p);
}

স্থির শূন্য vfslog_flush(VfslogVfs *p){
#ifdef SQLITE_TEST
  বাহ্য পূর্ণাঙ্ক sqlite3_io_error_pending;
  বাহ্য পূর্ণাঙ্ক sqlite3_io_error_persist;
  বাহ্য পূর্ণাঙ্ক sqlite3_diskfull_pending;

  পূর্ণাঙ্ক pending = sqlite3_io_error_pending;
  পূর্ণাঙ্ক persist = sqlite3_io_error_persist;
  পূর্ণাঙ্ক diskfull = sqlite3_diskfull_pending;

  sqlite3_io_error_pending = 0;
  sqlite3_io_error_persist = 0;
  sqlite3_diskfull_pending = 0;
#endif

  যদি( p->nBuf ){
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

স্থির শূন্য put32bits(unsigned বর্ণ *p, unsigned পূর্ণাঙ্ক v){
  p[0] = v>>24;
  p[1] = (unsigned বর্ণ)(v>>16);
  p[2] = (unsigned বর্ণ)(v>>8);
  p[3] = (unsigned বর্ণ)v;
}

স্থির শূন্য vfslog_call(
  sqlite3_vfs *pVfs,
  পূর্ণাঙ্ক eEvent,
  পূর্ণাঙ্ক iFileid,
  sqlite3_int64 nClick,
  পূর্ণাঙ্ক return_code,
  পূর্ণাঙ্ক size,
  পূর্ণাঙ্ক offset
){
  VfslogVfs *p = (VfslogVfs *)pVfs;
  unsigned বর্ণ *zRec;
  যদি( (24+p->nBuf)>পরিমাণ(p->aBuf) ){
    vfslog_flush(p);
  }
  zRec = (unsigned বর্ণ *)&p->aBuf[p->nBuf];
  put32bits(&zRec[0], eEvent);
  put32bits(&zRec[4], iFileid);
  put32bits(&zRec[8], (unsigned পূর্ণাঙ্ক)(nClick&0xffff));
  put32bits(&zRec[12], return_code);
  put32bits(&zRec[16], size);
  put32bits(&zRec[20], offset);
  p->nBuf += 24;
}

স্থির শূন্য vfslog_string(sqlite3_vfs *pVfs, ধ্রুব বর্ণ *zStr){
  VfslogVfs *p = (VfslogVfs *)pVfs;
  unsigned বর্ণ *zRec;
  পূর্ণাঙ্ক nStr = zStr ? (পূর্ণাঙ্ক)strlen(zStr) : 0;
  যদি( (4+nStr+p->nBuf)>পরিমাণ(p->aBuf) ){
    vfslog_flush(p);
  }
  zRec = (unsigned বর্ণ *)&p->aBuf[p->nBuf];
  put32bits(&zRec[0], nStr);
  যদি( zStr ){
    memcpy(&zRec[4], zStr, nStr);
  }
  p->nBuf += (4 + nStr);
}

স্থির শূন্য vfslog_finalize(VfslogVfs *p){
  যদি( p->pLog->pMethods ){
    vfslog_flush(p);
    p->pLog->pMethods->xClose(p->pLog);
  }
  sqlite3_free(p);
}

পূর্ণাঙ্ক sqlite3_vfslog_finalize(ধ্রুব বর্ণ *zVfs){
  sqlite3_vfs *pVfs;
  pVfs = sqlite3_vfs_find(zVfs);
  যদি( !pVfs || pVfs->xOpen!=vfslogOpen ){
    প্রত্যাগম SQLITE_ERROR;
  } 
  sqlite3_vfs_unregister(pVfs);
  vfslog_finalize((VfslogVfs *)pVfs);
  প্রত্যাগম SQLITE_OK;
}

পূর্ণাঙ্ক sqlite3_vfslog_new(
  ধ্রুব বর্ণ *zVfs,               /* New VFS name */
  ধ্রুব বর্ণ *zParentVfs,         /* Parent VFS name (or NULL) */
  ধ্রুব বর্ণ *zLog                /* Log file name */
){
  VfslogVfs *p;
  sqlite3_vfs *pParent;
  পূর্ণাঙ্ক nByte;
  পূর্ণাঙ্ক flags;
  পূর্ণাঙ্ক rc;
  বর্ণ *zFile;
  পূর্ণাঙ্ক nVfs;

  pParent = sqlite3_vfs_find(zParentVfs);
  যদি( !pParent ){
    প্রত্যাগম SQLITE_ERROR;
  }

  nVfs = (পূর্ণাঙ্ক)strlen(zVfs);
  nByte = পরিমাণ(VfslogVfs) + pParent->szOsFile + nVfs+1+pParent->mxPathname+1;
  p = (VfslogVfs *)sqlite3_malloc(nByte);
  memset(p, 0, nByte);

  p->pVfs = pParent;
  p->pLog = (sqlite3_file *)&p[1];
  memcpy(&p->base, &vfslog_vfs, পরিমাণ(sqlite3_vfs));
  p->base.zName = &((বর্ণ *)p->pLog)[pParent->szOsFile];
  p->base.szOsFile += pParent->szOsFile;
  memcpy((বর্ণ *)p->base.zName, zVfs, nVfs);

  zFile = (বর্ণ *)&p->base.zName[nVfs+1];
  pParent->xFullPathname(pParent, zLog, pParent->mxPathname, zFile);

  flags = SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE|SQLITE_OPEN_SUPER_JOURNAL;
  pParent->xDelete(pParent, zFile, 0);
  rc = pParent->xOpen(pParent, zFile, p->pLog, flags, &flags);
  যদি( rc==SQLITE_OK ){
    memcpy(p->aBuf, "sqlite_ostrace1.....", 20);
    p->iOffset = 0;
    p->nBuf = 20;
    rc = sqlite3_vfs_register((sqlite3_vfs *)p, 1);
  }
  যদি( rc ){
    vfslog_finalize(p);
  }
  প্রত্যাগম rc;
}

পূর্ণাঙ্ক sqlite3_vfslog_annotate(ধ্রুব বর্ণ *zVfs, ধ্রুব বর্ণ *z_বার্তা){
  sqlite3_vfs *pVfs;
  pVfs = sqlite3_vfs_find(zVfs);
  যদি( !pVfs || pVfs->xOpen!=vfslogOpen ){
    প্রত্যাগম SQLITE_ERROR;
  } 
  vfslog_call(pVfs, OS_ANNOTATE, 0, 0, 0, 0, 0);
  vfslog_string(pVfs, z_বার্তা);
  প্রত্যাগম SQLITE_OK;
}

স্থির ধ্রুব বর্ণ *vfslog_eventname(পূর্ণাঙ্ক eEvent){
  ধ্রুব বর্ণ *zEvent = 0;

  বিকল্প( eEvent ){
    পক্ষ OS_CLOSE:             zEvent = "xClose"; ভঙ্গ;
    পক্ষ OS_READ:              zEvent = "xRead"; ভঙ্গ;
    পক্ষ OS_WRITE:             zEvent = "xWrite"; ভঙ্গ;
    পক্ষ OS_TRUNCATE:          zEvent = "xTruncate"; ভঙ্গ;
    পক্ষ OS_SYNC:              zEvent = "xSync"; ভঙ্গ;
    পক্ষ OS_FILESIZE:          zEvent = "xFilesize"; ভঙ্গ;
    পক্ষ OS_LOCK:              zEvent = "xLock"; ভঙ্গ;
    পক্ষ OS_UNLOCK:            zEvent = "xUnlock"; ভঙ্গ;
    পক্ষ OS_CHECKRESERVEDLOCK: zEvent = "xCheckResLock"; ভঙ্গ;
    পক্ষ OS_FILECONTROL:       zEvent = "xFileControl"; ভঙ্গ;
    পক্ষ OS_SECTORSIZE:        zEvent = "xSectorSize"; ভঙ্গ;
    পক্ষ OS_DEVCHAR:           zEvent = "xDeviceChar"; ভঙ্গ;
    পক্ষ OS_OPEN:              zEvent = "xOpen"; ভঙ্গ;
    পক্ষ OS_DELETE:            zEvent = "xDelete"; ভঙ্গ;
    পক্ষ OS_ACCESS:            zEvent = "xAccess"; ভঙ্গ;
    পক্ষ OS_FULLPATHNAME:      zEvent = "xFullPathname"; ভঙ্গ;
    পক্ষ OS_RANDOMNESS:        zEvent = "xRandomness"; ভঙ্গ;
    পক্ষ OS_SLEEP:             zEvent = "xSleep"; ভঙ্গ;
    পক্ষ OS_CURRENTTIME:       zEvent = "xCurrentTime"; ভঙ্গ;

    পক্ষ OS_SHMUNMAP:          zEvent = "xShmUnmap"; ভঙ্গ;
    পক্ষ OS_SHMLOCK:           zEvent = "xShmLock"; ভঙ্গ;
    পক্ষ OS_SHMBARRIER:        zEvent = "xShmBarrier"; ভঙ্গ;
    পক্ষ OS_SHMMAP:            zEvent = "xShmMap"; ভঙ্গ;

    পক্ষ OS_ANNOTATE:          zEvent = "annotation"; ভঙ্গ;
  }

  প্রত্যাগম zEvent;
}

প্রকারনির্দেশ সংরচনা VfslogVtab VfslogVtab;
প্রকারনির্দেশ সংরচনা VfslogCsr VfslogCsr;

/*
** Virtual table type for the vfslog reader module.
*/
সংরচনা VfslogVtab {
  sqlite3_vtab base;              /* Base class */
  sqlite3_file *pFd;              /* File descriptor open on vfslog file */
  sqlite3_int64 nByte;            /* Size of file in bytes */
  বর্ণ *zFile;                    /* File name for pFd */
};

/*
** Virtual table cursor type for the vfslog reader module.
*/
সংরচনা VfslogCsr {
  sqlite3_vtab_cursor base;       /* Base class */
  sqlite3_int64 iRowid;           /* Current rowid. */
  sqlite3_int64 iOffset;          /* Offset of next record in file */
  বর্ণ *zTransient;               /* Transient 'file' string */
  পূর্ণাঙ্ক nFile;                      /* Size of array azFile[] */
  বর্ণ **azFile;                  /* File strings */
  unsigned বর্ণ aBuf[1024];       /* Current vfs log entry (read from file) */
};

স্থির unsigned পূর্ণাঙ্ক get32bits(unsigned বর্ণ *p){
  প্রত্যাগম (p[0]<<24) + (p[1]<<16) + (p[2]<<8) + p[3];
}

/*
** The argument must point to a buffer containing a nul-terminated string.
** If the string begins with an SQL quote character it is overwritten by
** the dequoted version. Otherwise the buffer is left unmodified.
*/
স্থির শূন্য dequote(বর্ণ *z){
  বর্ণ quote;                     /* Quote character (if any ) */
  quote = z[0];
  যদি( quote=='[' || quote=='\'' || quote=='"' || quote=='`' ){
    পূর্ণাঙ্ক iIn = 1;                  /* Index of next byte to read from input */
    পূর্ণাঙ্ক iOut = 0;                 /* Index of next byte to write to output */
    যদি( quote=='[' ) quote = ']';  
    যাবত্( z[iIn] ){
      যদি( z[iIn]==quote ){
        যদি( z[iIn+1]!=quote ) ভঙ্গ;
        z[iOut++] = quote;
        iIn += 2;
      }অন্যথা{
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
স্থির পূর্ণাঙ্ক vlogConnect(
  sqlite3 *db,
  শূন্য *pAux,
  পূর্ণাঙ্ক argc, ধ্রুব বর্ণ *ধ্রুব*argv,
  sqlite3_vtab **ppVtab,
  বর্ণ **pzErr
){
  sqlite3_vfs *pVfs;              /* VFS used to read log file */
  পূর্ণাঙ্ক flags;                      /* flags passed to pVfs->xOpen() */
  VfslogVtab *p;
  পূর্ণাঙ্ক rc;
  পূর্ণাঙ্ক nByte;
  বর্ণ *zFile;

  *ppVtab = 0;
  pVfs = sqlite3_vfs_find(0);
  nByte = পরিমাণ(VfslogVtab) + pVfs->szOsFile + pVfs->mxPathname;
  p = sqlite3_malloc(nByte);
  যদি( p==0 ) প্রত্যাগম SQLITE_NOMEM;
  memset(p, 0, nByte);

  p->pFd = (sqlite3_file *)&p[1];
  p->zFile = &((বর্ণ *)p->pFd)[pVfs->szOsFile];

  zFile = sqlite3_mprintf("%s", argv[3]);
  যদি( !zFile ){
    sqlite3_free(p);
    প্রত্যাগম SQLITE_NOMEM;
  }
  dequote(zFile);
  pVfs->xFullPathname(pVfs, zFile, pVfs->mxPathname, p->zFile);
  sqlite3_free(zFile);

  flags = SQLITE_OPEN_READWRITE|SQLITE_OPEN_SUPER_JOURNAL;
  rc = pVfs->xOpen(pVfs, p->zFile, p->pFd, flags, &flags);

  যদি( rc==SQLITE_OK ){
    p->pFd->pMethods->xFileSize(p->pFd, &p->nByte);
    sqlite3_declare_vtab(db, 
        "CREATE TABLE xxx(event, file, click, rc, size, offset)"
    );
    *ppVtab = &p->base;
  }অন্যথা{
    sqlite3_free(p);
  }

  প্রত্যাগম rc;
}

/*
** There is no "best-index". This virtual table always does a linear
** scan of the binary VFS log file.
*/
স্থির পূর্ণাঙ্ক vlogBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  pIdxInfo->estimatedCost = 10.0;
  প্রত্যাগম SQLITE_OK;
}

/*
** Disconnect from or destroy a vfslog virtual table.
*/
স্থির পূর্ণাঙ্ক vlogDisconnect(sqlite3_vtab *pVtab){
  VfslogVtab *p = (VfslogVtab *)pVtab;
  যদি( p->pFd->pMethods ){
    p->pFd->pMethods->xClose(p->pFd);
    p->pFd->pMethods = 0;
  }
  sqlite3_free(p);
  প্রত্যাগম SQLITE_OK;
}

/*
** Open a new vfslog cursor.
*/
স্থির পূর্ণাঙ্ক vlogOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  VfslogCsr *pCsr;                /* Newly allocated cursor object */

  pCsr = sqlite3_malloc(পরিমাণ(VfslogCsr));
  যদি( !pCsr ) প্রত্যাগম SQLITE_NOMEM;
  memset(pCsr, 0, পরিমাণ(VfslogCsr));
  *ppCursor = &pCsr->base;
  প্রত্যাগম SQLITE_OK;
}

/*
** Close a vfslog cursor.
*/
স্থির পূর্ণাঙ্ক vlogClose(sqlite3_vtab_cursor *pCursor){
  VfslogCsr *p = (VfslogCsr *)pCursor;
  পূর্ণাঙ্ক i;
  প্রতি(i=0; i<p->nFile; i++){
    sqlite3_free(p->azFile[i]);
  }
  sqlite3_free(p->azFile);
  sqlite3_free(p->zTransient);
  sqlite3_free(p);
  প্রত্যাগম SQLITE_OK;
}

/*
** Move a vfslog cursor to the next entry in the file.
*/
স্থির পূর্ণাঙ্ক vlogNext(sqlite3_vtab_cursor *pCursor){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  VfslogVtab *p = (VfslogVtab *)pCursor->pVtab;
  পূর্ণাঙ্ক rc = SQLITE_OK;
  পূর্ণাঙ্ক nRead;

  sqlite3_free(pCsr->zTransient);
  pCsr->zTransient = 0;

  nRead = 24;
  যদি( pCsr->iOffset+nRead<=p->nByte ){
    পূর্ণাঙ্ক eEvent;
    rc = p->pFd->pMethods->xRead(p->pFd, pCsr->aBuf, nRead, pCsr->iOffset);

    eEvent = get32bits(pCsr->aBuf);
    যদি( (rc==SQLITE_OK)
     && (eEvent==OS_OPEN || eEvent==OS_DELETE || eEvent==OS_ACCESS) 
    ){
      বর্ণ buf[4];
      rc = p->pFd->pMethods->xRead(p->pFd, buf, 4, pCsr->iOffset+nRead);
      nRead += 4;
      যদি( rc==SQLITE_OK ){
        পূর্ণাঙ্ক nStr = get32bits((unsigned বর্ণ *)buf);
        বর্ণ *zStr = sqlite3_malloc(nStr+1);
        rc = p->pFd->pMethods->xRead(p->pFd, zStr, nStr, pCsr->iOffset+nRead);
        zStr[nStr] = '\0';
        nRead += nStr;

        যদি( eEvent==OS_OPEN ){
          পূর্ণাঙ্ক iFileid = get32bits(&pCsr->aBuf[4]);
          যদি( iFileid>=pCsr->nFile ){
            পূর্ণাঙ্ক nNew = পরিমাণ(pCsr->azFile[0])*(iFileid+1);
            pCsr->azFile = (বর্ণ **)sqlite3_realloc(pCsr->azFile, nNew);
            nNew -= পরিমাণ(pCsr->azFile[0])*pCsr->nFile;
            memset(&pCsr->azFile[pCsr->nFile], 0, nNew);
            pCsr->nFile = iFileid+1;
          }
          sqlite3_free(pCsr->azFile[iFileid]);
          pCsr->azFile[iFileid] = zStr;
        }অন্যথা{
          pCsr->zTransient = zStr;
        }
      }
    }
  }

  pCsr->iRowid += 1;
  pCsr->iOffset += nRead;
  প্রত্যাগম rc;
}

স্থির পূর্ণাঙ্ক vlogEof(sqlite3_vtab_cursor *pCursor){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  VfslogVtab *p = (VfslogVtab *)pCursor->pVtab;
  প্রত্যাগম (pCsr->iOffset>=p->nByte);
}

স্থির পূর্ণাঙ্ক vlogFilter(
  sqlite3_vtab_cursor *pCursor, 
  পূর্ণাঙ্ক idx_অঙ্ক, ধ্রুব বর্ণ *idxStr,
  পূর্ণাঙ্ক argc, sqlite3_মান **argv
){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  pCsr->iRowid = 0;
  pCsr->iOffset = 20;
  প্রত্যাগম vlogNext(pCursor);
}

স্থির পূর্ণাঙ্ক vlogColumn(
  sqlite3_vtab_cursor *pCursor, 
  sqlite3_context *ctx, 
  পূর্ণাঙ্ক i
){
  unsigned পূর্ণাঙ্ক val;
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;

  প্রতিজ্ঞা( i<7 );
  val = get32bits(&pCsr->aBuf[4*i]);

  বিকল্প( i ){
    পক্ষ 0: {
      sqlite3_result_text(ctx, vfslog_eventname(val), -1, SQLITE_STATIC);
      ভঙ্গ;
    }
    পক্ষ 1: {
      বর্ণ *zStr = pCsr->zTransient;
      যদি( val!=0 && val<(unsigned)pCsr->nFile ){
        zStr = pCsr->azFile[val];
      }
      sqlite3_result_text(ctx, zStr, -1, SQLITE_TRANSIENT);
      ভঙ্গ;
    }
    স্বভাব:
      sqlite3_result_int(ctx, val);
      ভঙ্গ;
  }

  প্রত্যাগম SQLITE_OK;
}

স্থির পূর্ণাঙ্ক vlogRowid(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  *pRowid = pCsr->iRowid;
  প্রত্যাগম SQLITE_OK;
}

পূর্ণাঙ্ক sqlite3_vfslog_register(sqlite3 *db){
  স্থির sqlite3_module vfslog_module = {
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
  প্রত্যাগম SQLITE_OK;
}
#endif /* SQLITE_OMIT_VIRTUALTABLE */

/**************************************************************************
***************************************************************************
** Tcl interface starts here.
*/

#যদি defined(SQLITE_TEST) || defined(TCLSH)

#অন্তর্ভাব "tclsqlite.h"

স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_vfslog(
  শূন্য *clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  সংরচনা SqliteDb { sqlite3 *db; };
  sqlite3 *db;
  Tcl_CmdInfo cmdInfo;
  পূর্ণাঙ্ক rc = SQLITE_ERROR;

  স্থির ধ্রুব বর্ণ *strs[] = { "annotate", "finalize", "new",  "register", 0 };
  গণনা VL_enum { VL_ANNOTATE, VL_FINALIZE, VL_NEW, VL_REGISTER };
  পূর্ণাঙ্ক iSub;

  যদি( objc<2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "SUB-COMMAND ...");
    প্রত্যাগম TCL_ERROR;
  }
  যদি( Tcl_GetIndexFromObj(interp, objv[1], strs, "sub-command", 0, &iSub) ){
    প্রত্যাগম TCL_ERROR;
  }

  বিকল্প( (গণনা VL_enum)iSub ){
    পক্ষ VL_ANNOTATE: {
      বর্ণ *zVfs;
      বর্ণ *z_বার্তা;
      যদি( objc!=4 ){
        Tcl_Wrong_অঙ্ক_Args(interp, 3, objv, "VFS");
        প্রত্যাগম TCL_ERROR;
      }
      zVfs = Tcl_GetString(objv[2]);
      z_বার্তা = Tcl_GetString(objv[3]);
      rc = sqlite3_vfslog_annotate(zVfs, z_বার্তা);
      যদি( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp, "failed", (বর্ণ*)0);
        প্রত্যাগম TCL_ERROR;
      }
      ভঙ্গ;
    }
    পক্ষ VL_FINALIZE: {
      বর্ণ *zVfs;
      যদি( objc!=3 ){
        Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "VFS");
        প্রত্যাগম TCL_ERROR;
      }
      zVfs = Tcl_GetString(objv[2]);
      rc = sqlite3_vfslog_finalize(zVfs);
      যদি( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp, "failed", (বর্ণ*)0);
        প্রত্যাগম TCL_ERROR;
      }
      ভঙ্গ;
    };

    পক্ষ VL_NEW: {
      বর্ণ *zVfs;
      বর্ণ *zParent;
      বর্ণ *zLog;
      যদি( objc!=5 ){
        Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "VFS PARENT LOGFILE");
        প্রত্যাগম TCL_ERROR;
      }
      zVfs = Tcl_GetString(objv[2]);
      zParent = Tcl_GetString(objv[3]);
      zLog = Tcl_GetString(objv[4]);
      যদি( *zParent=='\0' ) zParent = 0;
      rc = sqlite3_vfslog_new(zVfs, zParent, zLog);
      যদি( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp, "failed", (বর্ণ*)0);
        প্রত্যাগম TCL_ERROR;
      }
      ভঙ্গ;
    };

    পক্ষ VL_REGISTER: {
      বর্ণ *zDb;
      যদি( objc!=3 ){
        Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "DB");
        প্রত্যাগম TCL_ERROR;
      }
#ifdef SQLITE_OMIT_VIRTUALTABLE
      Tcl_AppendResult(interp, "vfslog not available because of "
                               "SQLITE_OMIT_VIRTUALTABLE", (শূন্য*)0);
      প্রত্যাগম TCL_ERROR;
#অন্যথা
      zDb = Tcl_GetString(objv[2]);
      যদি( Tcl_GetCommandInfo(interp, zDb, &cmdInfo) ){
        db = ((সংরচনা SqliteDb*)cmdInfo.objClientData)->db;
        rc = sqlite3_vfslog_register(db);
      }
      যদি( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp, "bad sqlite3 handle: ", zDb, (শূন্য*)0);
        প্রত্যাগম TCL_ERROR;
      }
      ভঙ্গ;
#endif
    }
  }

  প্রত্যাগম TCL_OK;
}

পূর্ণাঙ্ক SqlitetestOsinst_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "vfslog", test_vfslog, 0, 0);
  প্রত্যাগম TCL_OK;
}

#endif /* SQLITE_TEST */
