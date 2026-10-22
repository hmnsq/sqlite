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

#ଅନ୍ତର୍ଭାଵ "sqlite3.h"

#ifdef _WIN32
#ଅନ୍ତର୍ଭାଵ <windows.h>
#endif

#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <assert.h>


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

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା VfslogVfs VfslogVfs;
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା VfslogFile VfslogFile;

ସଂରଚନା VfslogVfs {
  sqlite3_vfs base;               /* VFS methods */
  sqlite3_vfs *pVfs;              /* Parent VFS */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iNextFileId;                /* Next file id */
  sqlite3_file *pLog;             /* Log file handle */
  sqlite3_int64 iOffset;          /* Log file offset of start of write buffer */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nBuf;                       /* Number of valid bytes in aBuf[] */
  ଅକ୍ଷର aBuf[VFSLOG_BUFFERSIZE];   /* Write buffer */
};

ସଂରଚନା VfslogFile {
  sqlite3_file base;              /* IO methods */
  sqlite3_file *pReal;            /* Underlying file handle */
  sqlite3_vfs *pVfslog;           /* Associated VsflogVfs object */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iFileId;                    /* File id number */
};

#define REALVFS(p) (((VfslogVfs *)(p))->pVfs)



/*
** Method declarations for vfslog_file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vfslogClose(sqlite3_file*);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vfslogRead(sqlite3_file*, ଶୂନ୍ୟ*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iAmt, sqlite3_int64 iOfst);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vfslogWrite(sqlite3_file*,ଅଚଳ ଶୂନ୍ୟ*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା iAmt, sqlite3_int64 iOfst);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vfslogTruncate(sqlite3_file*, sqlite3_int64 size);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vfslogSync(sqlite3_file*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vfslogFileSize(sqlite3_file*, sqlite3_int64 *pSize);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vfslogLock(sqlite3_file*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vfslogUnlock(sqlite3_file*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vfslogCheckReservedLock(sqlite3_file*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pResOut);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vfslogFileControl(sqlite3_file*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା op, ଶୂନ୍ୟ *pArg);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vfslogSectorSize(sqlite3_file*);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vfslogDeviceCharacteristics(sqlite3_file*);

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vfslogShmLock(sqlite3_file *pFile, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ofst, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vfslogShmMap(sqlite3_file *pFile,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ଅସ୍ଥିର ଶୂନ୍ୟ **);
ସ୍ଥିର ଶୂନ୍ୟ vfslogShmBarrier(sqlite3_file*);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vfslogShmUnmap(sqlite3_file *pFile, ପୂର୍ଣ୍ଣସଂଖ୍ୟା deleteFlag);

/*
** Method declarations for vfslog_vfs.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vfslogOpen(sqlite3_vfs*, ଅଚଳ ଅକ୍ଷର *, sqlite3_file*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା , ପୂର୍ଣ୍ଣସଂଖ୍ୟା *);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vfslogDelete(sqlite3_vfs*, ଅଚଳ ଅକ୍ଷର *zName, ପୂର୍ଣ୍ଣସଂଖ୍ୟା syncDir);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vfslogAccess(sqlite3_vfs*, ଅଚଳ ଅକ୍ଷର *zName, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vfslogFullPathname(sqlite3_vfs*, ଅଚଳ ଅକ୍ଷର *zName, ପୂର୍ଣ୍ଣସଂଖ୍ୟା, ଅକ୍ଷର *zOut);
ସ୍ଥିର ଶୂନ୍ୟ *vfslogDlOpen(sqlite3_vfs*, ଅଚଳ ଅକ୍ଷର *zFilename);
ସ୍ଥିର ଶୂନ୍ୟ vfslogDlError(sqlite3_vfs*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nByte, ଅକ୍ଷର *z_Err_ସନ୍ଦେଶ);
ସ୍ଥିର ଶୂନ୍ୟ (*vfslogDlSym(sqlite3_vfs *pVfs, ଶୂନ୍ୟ *p, ଅଚଳ ଅକ୍ଷର*zSym))(ଶୂନ୍ୟ);
ସ୍ଥିର ଶୂନ୍ୟ vfslogDlClose(sqlite3_vfs*, ଶୂନ୍ୟ*);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vfslogRandomness(sqlite3_vfs*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nByte, ଅକ୍ଷର *zOut);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vfslogSleep(sqlite3_vfs*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା microseconds);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vfslogCurrentTime(sqlite3_vfs*, ଦ୍ୱିଗୁଣ*);

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vfslogGetLastError(sqlite3_vfs*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା, ଅକ୍ଷର *);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vfslogCurrentTimeInt64(sqlite3_vfs*, sqlite3_int64*);

ସ୍ଥିର sqlite3_vfs vfslog_vfs = {
  1,                              /* iVersion */
  ଆକାର(VfslogFile),             /* szOsFile */
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

ସ୍ଥିର sqlite3_io_methods vfslog_io_methods = {
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
#ଅନ୍ତର୍ଭାଵ <time.h>
ସ୍ଥିର sqlite3_uint64 vfslog_time(){
  FILETIME ft;
  sqlite3_uint64 u64time = 0;
 
  GetSystemTimeAsFileTime(&ft);

  u64time |= ft.dw_ଉପର_Date_Time;
  u64time <<= 32;
  u64time |= ft.dw_ତଳ_Date_Time;

  /* ft is 100-nanosecond intervals, we want microseconds */
  ଫେରନ୍ତୁ u64time /(sqlite3_uint64)10;
}
#ନଚେତଯଦି !defined(NO_GETTOD)
#ଅନ୍ତର୍ଭାଵ <sys/time.h>
ସ୍ଥିର sqlite3_uint64 vfslog_time(){
  ସଂରଚନା timeval sTime;
  gettimeofday(&sTime, 0);
  ଫେରନ୍ତୁ sTime.tv_usec + (sqlite3_uint64)sTime.tv_sec * 1000000;
}
#ଅନ୍ୟଥା
ସ୍ଥିର sqlite3_uint64 vfslog_time(){
  ଫେରନ୍ତୁ 0;
}
#endif

ସ୍ଥିର ଶୂନ୍ୟ vfslog_call(sqlite3_vfs *, ପୂର୍ଣ୍ଣସଂଖ୍ୟା, ପୂର୍ଣ୍ଣସଂଖ୍ୟା, sqlite3_int64, ପୂର୍ଣ୍ଣସଂଖ୍ୟା, ପୂର୍ଣ୍ଣସଂଖ୍ୟା, ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
ସ୍ଥିର ଶୂନ୍ୟ vfslog_string(sqlite3_vfs *, ଅଚଳ ଅକ୍ଷର *);

/*
** Close an vfslog-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vfslogClose(sqlite3_file *pFile){
  sqlite3_uint64 t;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
  VfslogFile *p = (VfslogFile *)pFile;

  t = vfslog_time();
  ଯଦି( p->pReal->pMethods ){
    rc = p->pReal->pMethods->xClose(p->pReal);
  }
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_CLOSE, p->iFileId, t, rc, 0, 0);
  ଫେରନ୍ତୁ rc;
}

/*
** Read data from an vfslog-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vfslogRead(
  sqlite3_file *pFile, 
  ଶୂନ୍ୟ *zBuf, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iAmt, 
  sqlite_int64 iOfst
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xRead(p->pReal, zBuf, iAmt, iOfst);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_READ, p->iFileId, t, rc, iAmt, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)iOfst);
  ଫେରନ୍ତୁ rc;
}

/*
** Write data to an vfslog-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vfslogWrite(
  sqlite3_file *pFile,
  ଅଚଳ ଶୂନ୍ୟ *z,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iAmt,
  sqlite_int64 iOfst
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xWrite(p->pReal, z, iAmt, iOfst);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_WRITE, p->iFileId, t, rc, iAmt, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)iOfst);
  ଫେରନ୍ତୁ rc;
}

/*
** Truncate an vfslog-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vfslogTruncate(sqlite3_file *pFile, sqlite_int64 size){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xTruncate(p->pReal, size);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_TRUNCATE, p->iFileId, t, rc, 0, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)size);
  ଫେରନ୍ତୁ rc;
}

/*
** Sync an vfslog-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vfslogSync(sqlite3_file *pFile, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xSync(p->pReal, flags);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SYNC, p->iFileId, t, rc, flags, 0);
  ଫେରନ୍ତୁ rc;
}

/*
** Return the current file-size of an vfslog-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vfslogFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xFileSize(p->pReal, pSize);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_FILESIZE, p->iFileId, t, rc, 0, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)*pSize);
  ଫେରନ୍ତୁ rc;
}

/*
** Lock an vfslog-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vfslogLock(sqlite3_file *pFile, ପୂର୍ଣ୍ଣସଂଖ୍ୟା eLock){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xLock(p->pReal, eLock);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_LOCK, p->iFileId, t, rc, eLock, 0);
  ଫେରନ୍ତୁ rc;
}

/*
** Unlock an vfslog-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vfslogUnlock(sqlite3_file *pFile, ପୂର୍ଣ୍ଣସଂଖ୍ୟା eLock){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xUnlock(p->pReal, eLock);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_UNLOCK, p->iFileId, t, rc, eLock, 0);
  ଫେରନ୍ତୁ rc;
}

/*
** Check if another file-handle holds a RESERVED lock on an vfslog-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vfslogCheckReservedLock(sqlite3_file *pFile, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pResOut){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xCheckReservedLock(p->pReal, pResOut);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_CHECKRESERVEDLOCK, p->iFileId, t, rc, *pResOut, 0);
  ଫେରନ୍ତୁ rc;
}

/*
** File control method. For custom operations on an vfslog-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vfslogFileControl(sqlite3_file *pFile, ପୂର୍ଣ୍ଣସଂଖ୍ୟା op, ଶୂନ୍ୟ *pArg){
  VfslogFile *p = (VfslogFile *)pFile;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = p->pReal->pMethods->xFileControl(p->pReal, op, pArg);
  ଯଦି( op==SQLITE_FCNTL_VFSNAME && rc==SQLITE_OK ){
    *(ଅକ୍ଷର**)pArg = sqlite3_mprintf("vfslog/%z", *(ଅକ୍ଷର**)pArg);
  }
  ଫେରନ୍ତୁ rc;
}

/*
** Return the sector-size in bytes for an vfslog-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vfslogSectorSize(sqlite3_file *pFile){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xSectorSize(p->pReal);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SECTORSIZE, p->iFileId, t, rc, 0, 0);
  ଫେରନ୍ତୁ rc;
}

/*
** Return the device characteristic flags supported by an vfslog-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vfslogDeviceCharacteristics(sqlite3_file *pFile){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xDeviceCharacteristics(p->pReal);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_DEVCHAR, p->iFileId, t, rc, 0, 0);
  ଫେରନ୍ତୁ rc;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vfslogShmLock(sqlite3_file *pFile, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ofst, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xShmLock(p->pReal, ofst, n, flags);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SHMLOCK, p->iFileId, t, rc, 0, 0);
  ଫେରନ୍ତୁ rc;
}
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vfslogShmMap(
  sqlite3_file *pFile, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iRegion, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା szRegion, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା isWrite, 
  ଅସ୍ଥିର ଶୂନ୍ୟ **pp
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xShmMap(p->pReal, iRegion, szRegion, isWrite, pp);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SHMMAP, p->iFileId, t, rc, 0, 0);
  ଫେରନ୍ତୁ rc;
}
ସ୍ଥିର ଶୂନ୍ୟ vfslogShmBarrier(sqlite3_file *pFile){
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  p->pReal->pMethods->xShmBarrier(p->pReal);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SHMBARRIER, p->iFileId, t, SQLITE_OK, 0, 0);
}
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vfslogShmUnmap(sqlite3_file *pFile, ପୂର୍ଣ୍ଣସଂଖ୍ୟା deleteFlag){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xShmUnmap(p->pReal, deleteFlag);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SHMUNMAP, p->iFileId, t, rc, 0, 0);
  ଫେରନ୍ତୁ rc;
}


/*
** Open an vfslog file handle.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vfslogOpen(
  sqlite3_vfs *pVfs,
  ଅଚଳ ଅକ୍ଷର *zName,
  sqlite3_file *pFile,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pOutFlags
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
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
  ଫେରନ୍ତୁ rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vfslogDelete(sqlite3_vfs *pVfs, ଅଚଳ ଅକ୍ଷର *zPath, ପୂର୍ଣ୍ଣସଂଖ୍ୟା dirSync){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  sqlite3_uint64 t;
  t = vfslog_time();
  rc = REALVFS(pVfs)->xDelete(REALVFS(pVfs), zPath, dirSync);
  t = vfslog_time() - t;
  vfslog_call(pVfs, OS_DELETE, 0, t, rc, dirSync, 0);
  vfslog_string(pVfs, zPath);
  ଫେରନ୍ତୁ rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vfslogAccess(
  sqlite3_vfs *pVfs, 
  ଅଚଳ ଅକ୍ଷର *zPath, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pResOut
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  sqlite3_uint64 t;
  t = vfslog_time();
  rc = REALVFS(pVfs)->xAccess(REALVFS(pVfs), zPath, flags, pResOut);
  t = vfslog_time() - t;
  vfslog_call(pVfs, OS_ACCESS, 0, t, rc, flags, *pResOut);
  vfslog_string(pVfs, zPath);
  ଫେରନ୍ତୁ rc;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vfslogFullPathname(
  sqlite3_vfs *pVfs, 
  ଅଚଳ ଅକ୍ଷର *zPath, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nOut, 
  ଅକ୍ଷର *zOut
){
  ଫେରନ୍ତୁ REALVFS(pVfs)->xFullPathname(REALVFS(pVfs), zPath, nOut, zOut);
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
ସ୍ଥିର ଶୂନ୍ୟ *vfslogDlOpen(sqlite3_vfs *pVfs, ଅଚଳ ଅକ୍ଷର *zPath){
  ଫେରନ୍ତୁ REALVFS(pVfs)->xDlOpen(REALVFS(pVfs), zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
ସ୍ଥିର ଶୂନ୍ୟ vfslogDlError(sqlite3_vfs *pVfs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nByte, ଅକ୍ଷର *z_Err_ସନ୍ଦେଶ){
  REALVFS(pVfs)->xDlError(REALVFS(pVfs), nByte, z_Err_ସନ୍ଦେଶ);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
ସ୍ଥିର ଶୂନ୍ୟ (*vfslogDlSym(sqlite3_vfs *pVfs, ଶୂନ୍ୟ *p, ଅଚଳ ଅକ୍ଷର *zSym))(ଶୂନ୍ୟ){
  ଫେରନ୍ତୁ REALVFS(pVfs)->xDlSym(REALVFS(pVfs), p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
ସ୍ଥିର ଶୂନ୍ୟ vfslogDlClose(sqlite3_vfs *pVfs, ଶୂନ୍ୟ *pHandle){
  REALVFS(pVfs)->xDlClose(REALVFS(pVfs), pHandle);
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vfslogRandomness(sqlite3_vfs *pVfs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nByte, ଅକ୍ଷର *zBufOut){
  ଫେରନ୍ତୁ REALVFS(pVfs)->xRandomness(REALVFS(pVfs), nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vfslogSleep(sqlite3_vfs *pVfs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nMicro){
  ଫେରନ୍ତୁ REALVFS(pVfs)->xSleep(REALVFS(pVfs), nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vfslogCurrentTime(sqlite3_vfs *pVfs, ଦ୍ୱିଗୁଣ *pTimeOut){
  ଫେରନ୍ତୁ REALVFS(pVfs)->xCurrentTime(REALVFS(pVfs), pTimeOut);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vfslogGetLastError(sqlite3_vfs *pVfs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା a, ଅକ୍ଷର *b){
  ଫେରନ୍ତୁ REALVFS(pVfs)->xGetLastError(REALVFS(pVfs), a, b);
}
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vfslogCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *p){
  ଫେରନ୍ତୁ REALVFS(pVfs)->xCurrentTimeInt64(REALVFS(pVfs), p);
}

ସ୍ଥିର ଶୂନ୍ୟ vfslog_flush(VfslogVfs *p){
#ifdef SQLITE_TEST
  ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_io_error_pending;
  ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_io_error_persist;
  ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_diskfull_pending;

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା pending = sqlite3_io_error_pending;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା persist = sqlite3_io_error_persist;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା diskfull = sqlite3_diskfull_pending;

  sqlite3_io_error_pending = 0;
  sqlite3_io_error_persist = 0;
  sqlite3_diskfull_pending = 0;
#endif

  ଯଦି( p->nBuf ){
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

ସ୍ଥିର ଶୂନ୍ୟ put32bits(unsigned ଅକ୍ଷର *p, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା v){
  p[0] = v>>24;
  p[1] = (unsigned ଅକ୍ଷର)(v>>16);
  p[2] = (unsigned ଅକ୍ଷର)(v>>8);
  p[3] = (unsigned ଅକ୍ଷର)v;
}

ସ୍ଥିର ଶୂନ୍ୟ vfslog_call(
  sqlite3_vfs *pVfs,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା eEvent,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iFileid,
  sqlite3_int64 nClick,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା return_code,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା size,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା offset
){
  VfslogVfs *p = (VfslogVfs *)pVfs;
  unsigned ଅକ୍ଷର *zRec;
  ଯଦି( (24+p->nBuf)>ଆକାର(p->aBuf) ){
    vfslog_flush(p);
  }
  zRec = (unsigned ଅକ୍ଷର *)&p->aBuf[p->nBuf];
  put32bits(&zRec[0], eEvent);
  put32bits(&zRec[4], iFileid);
  put32bits(&zRec[8], (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(nClick&0xffff));
  put32bits(&zRec[12], return_code);
  put32bits(&zRec[16], size);
  put32bits(&zRec[20], offset);
  p->nBuf += 24;
}

ସ୍ଥିର ଶୂନ୍ୟ vfslog_string(sqlite3_vfs *pVfs, ଅଚଳ ଅକ୍ଷର *zStr){
  VfslogVfs *p = (VfslogVfs *)pVfs;
  unsigned ଅକ୍ଷର *zRec;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nStr = zStr ? (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)strlen(zStr) : 0;
  ଯଦି( (4+nStr+p->nBuf)>ଆକାର(p->aBuf) ){
    vfslog_flush(p);
  }
  zRec = (unsigned ଅକ୍ଷର *)&p->aBuf[p->nBuf];
  put32bits(&zRec[0], nStr);
  ଯଦି( zStr ){
    memcpy(&zRec[4], zStr, nStr);
  }
  p->nBuf += (4 + nStr);
}

ସ୍ଥିର ଶୂନ୍ୟ vfslog_finalize(VfslogVfs *p){
  ଯଦି( p->pLog->pMethods ){
    vfslog_flush(p);
    p->pLog->pMethods->xClose(p->pLog);
  }
  sqlite3_free(p);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_vfslog_finalize(ଅଚଳ ଅକ୍ଷର *zVfs){
  sqlite3_vfs *pVfs;
  pVfs = sqlite3_vfs_find(zVfs);
  ଯଦି( !pVfs || pVfs->xOpen!=vfslogOpen ){
    ଫେରନ୍ତୁ SQLITE_ERROR;
  } 
  sqlite3_vfs_unregister(pVfs);
  vfslog_finalize((VfslogVfs *)pVfs);
  ଫେରନ୍ତୁ SQLITE_OK;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_vfslog_new(
  ଅଚଳ ଅକ୍ଷର *zVfs,               /* New VFS name */
  ଅଚଳ ଅକ୍ଷର *zParentVfs,         /* Parent VFS name (or NULL) */
  ଅଚଳ ଅକ୍ଷର *zLog                /* Log file name */
){
  VfslogVfs *p;
  sqlite3_vfs *pParent;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nByte;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ଅକ୍ଷର *zFile;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nVfs;

  pParent = sqlite3_vfs_find(zParentVfs);
  ଯଦି( !pParent ){
    ଫେରନ୍ତୁ SQLITE_ERROR;
  }

  nVfs = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)strlen(zVfs);
  nByte = ଆକାର(VfslogVfs) + pParent->szOsFile + nVfs+1+pParent->mxPathname+1;
  p = (VfslogVfs *)sqlite3_malloc(nByte);
  memset(p, 0, nByte);

  p->pVfs = pParent;
  p->pLog = (sqlite3_file *)&p[1];
  memcpy(&p->base, &vfslog_vfs, ଆକାର(sqlite3_vfs));
  p->base.zName = &((ଅକ୍ଷର *)p->pLog)[pParent->szOsFile];
  p->base.szOsFile += pParent->szOsFile;
  memcpy((ଅକ୍ଷର *)p->base.zName, zVfs, nVfs);

  zFile = (ଅକ୍ଷର *)&p->base.zName[nVfs+1];
  pParent->xFullPathname(pParent, zLog, pParent->mxPathname, zFile);

  flags = SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE|SQLITE_OPEN_SUPER_JOURNAL;
  pParent->xDelete(pParent, zFile, 0);
  rc = pParent->xOpen(pParent, zFile, p->pLog, flags, &flags);
  ଯଦି( rc==SQLITE_OK ){
    memcpy(p->aBuf, "sqlite_ostrace1.....", 20);
    p->iOffset = 0;
    p->nBuf = 20;
    rc = sqlite3_vfs_register((sqlite3_vfs *)p, 1);
  }
  ଯଦି( rc ){
    vfslog_finalize(p);
  }
  ଫେରନ୍ତୁ rc;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_vfslog_annotate(ଅଚଳ ଅକ୍ଷର *zVfs, ଅଚଳ ଅକ୍ଷର *z_ସନ୍ଦେଶ){
  sqlite3_vfs *pVfs;
  pVfs = sqlite3_vfs_find(zVfs);
  ଯଦି( !pVfs || pVfs->xOpen!=vfslogOpen ){
    ଫେରନ୍ତୁ SQLITE_ERROR;
  } 
  vfslog_call(pVfs, OS_ANNOTATE, 0, 0, 0, 0, 0);
  vfslog_string(pVfs, z_ସନ୍ଦେଶ);
  ଫେରନ୍ତୁ SQLITE_OK;
}

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *vfslog_eventname(ପୂର୍ଣ୍ଣସଂଖ୍ୟା eEvent){
  ଅଚଳ ଅକ୍ଷର *zEvent = 0;

  ବାଛ( eEvent ){
    ସ୍ଥିତି OS_CLOSE:             zEvent = "xClose"; ଭାଙ୍ଗ;
    ସ୍ଥିତି OS_READ:              zEvent = "xRead"; ଭାଙ୍ଗ;
    ସ୍ଥିତି OS_WRITE:             zEvent = "xWrite"; ଭାଙ୍ଗ;
    ସ୍ଥିତି OS_TRUNCATE:          zEvent = "xTruncate"; ଭାଙ୍ଗ;
    ସ୍ଥିତି OS_SYNC:              zEvent = "xSync"; ଭାଙ୍ଗ;
    ସ୍ଥିତି OS_FILESIZE:          zEvent = "xFilesize"; ଭାଙ୍ଗ;
    ସ୍ଥିତି OS_LOCK:              zEvent = "xLock"; ଭାଙ୍ଗ;
    ସ୍ଥିତି OS_UNLOCK:            zEvent = "xUnlock"; ଭାଙ୍ଗ;
    ସ୍ଥିତି OS_CHECKRESERVEDLOCK: zEvent = "xCheckResLock"; ଭାଙ୍ଗ;
    ସ୍ଥିତି OS_FILECONTROL:       zEvent = "xFileControl"; ଭାଙ୍ଗ;
    ସ୍ଥିତି OS_SECTORSIZE:        zEvent = "xSectorSize"; ଭାଙ୍ଗ;
    ସ୍ଥିତି OS_DEVCHAR:           zEvent = "xDeviceChar"; ଭାଙ୍ଗ;
    ସ୍ଥିତି OS_OPEN:              zEvent = "xOpen"; ଭାଙ୍ଗ;
    ସ୍ଥିତି OS_DELETE:            zEvent = "xDelete"; ଭାଙ୍ଗ;
    ସ୍ଥିତି OS_ACCESS:            zEvent = "xAccess"; ଭାଙ୍ଗ;
    ସ୍ଥିତି OS_FULLPATHNAME:      zEvent = "xFullPathname"; ଭାଙ୍ଗ;
    ସ୍ଥିତି OS_RANDOMNESS:        zEvent = "xRandomness"; ଭାଙ୍ଗ;
    ସ୍ଥିତି OS_SLEEP:             zEvent = "xSleep"; ଭାଙ୍ଗ;
    ସ୍ଥିତି OS_CURRENTTIME:       zEvent = "xCurrentTime"; ଭାଙ୍ଗ;

    ସ୍ଥିତି OS_SHMUNMAP:          zEvent = "xShmUnmap"; ଭାଙ୍ଗ;
    ସ୍ଥିତି OS_SHMLOCK:           zEvent = "xShmLock"; ଭାଙ୍ଗ;
    ସ୍ଥିତି OS_SHMBARRIER:        zEvent = "xShmBarrier"; ଭାଙ୍ଗ;
    ସ୍ଥିତି OS_SHMMAP:            zEvent = "xShmMap"; ଭାଙ୍ଗ;

    ସ୍ଥିତି OS_ANNOTATE:          zEvent = "annotation"; ଭାଙ୍ଗ;
  }

  ଫେରନ୍ତୁ zEvent;
}

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା VfslogVtab VfslogVtab;
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା VfslogCsr VfslogCsr;

/*
** Virtual table type for the vfslog reader module.
*/
ସଂରଚନା VfslogVtab {
  sqlite3_vtab base;              /* Base class */
  sqlite3_file *pFd;              /* File descriptor open on vfslog file */
  sqlite3_int64 nByte;            /* Size of file in bytes */
  ଅକ୍ଷର *zFile;                    /* File name for pFd */
};

/*
** Virtual table cursor type for the vfslog reader module.
*/
ସଂରଚନା VfslogCsr {
  sqlite3_vtab_cursor base;       /* Base class */
  sqlite3_int64 iRowid;           /* Current rowid. */
  sqlite3_int64 iOffset;          /* Offset of next record in file */
  ଅକ୍ଷର *zTransient;               /* Transient 'file' string */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nFile;                      /* Size of array azFile[] */
  ଅକ୍ଷର **azFile;                  /* File strings */
  unsigned ଅକ୍ଷର aBuf[1024];       /* Current vfs log entry (read from file) */
};

ସ୍ଥିର unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା get32bits(unsigned ଅକ୍ଷର *p){
  ଫେରନ୍ତୁ (p[0]<<24) + (p[1]<<16) + (p[2]<<8) + p[3];
}

/*
** The argument must point to a buffer containing a nul-terminated string.
** If the string begins with an SQL quote character it is overwritten by
** the dequoted version. Otherwise the buffer is left unmodified.
*/
ସ୍ଥିର ଶୂନ୍ୟ dequote(ଅକ୍ଷର *z){
  ଅକ୍ଷର quote;                     /* Quote character (if any ) */
  quote = z[0];
  ଯଦି( quote=='[' || quote=='\'' || quote=='"' || quote=='`' ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା iIn = 1;                  /* Index of next byte to read from input */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା iOut = 0;                 /* Index of next byte to write to output */
    ଯଦି( quote=='[' ) quote = ']';  
    ଯେତେବେଳେ( z[iIn] ){
      ଯଦି( z[iIn]==quote ){
        ଯଦି( z[iIn+1]!=quote ) ଭାଙ୍ଗ;
        z[iOut++] = quote;
        iIn += 2;
      }ଅନ୍ୟଥା{
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vlogConnect(
  sqlite3 *db,
  ଶୂନ୍ୟ *pAux,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅଚଳ ଅକ୍ଷର *ଅଚଳ*argv,
  sqlite3_vtab **ppVtab,
  ଅକ୍ଷର **pzErr
){
  sqlite3_vfs *pVfs;              /* VFS used to read log file */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags;                      /* flags passed to pVfs->xOpen() */
  VfslogVtab *p;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nByte;
  ଅକ୍ଷର *zFile;

  *ppVtab = 0;
  pVfs = sqlite3_vfs_find(0);
  nByte = ଆକାର(VfslogVtab) + pVfs->szOsFile + pVfs->mxPathname;
  p = sqlite3_malloc(nByte);
  ଯଦି( p==0 ) ଫେରନ୍ତୁ SQLITE_NOMEM;
  memset(p, 0, nByte);

  p->pFd = (sqlite3_file *)&p[1];
  p->zFile = &((ଅକ୍ଷର *)p->pFd)[pVfs->szOsFile];

  zFile = sqlite3_mprintf("%s", argv[3]);
  ଯଦି( !zFile ){
    sqlite3_free(p);
    ଫେରନ୍ତୁ SQLITE_NOMEM;
  }
  dequote(zFile);
  pVfs->xFullPathname(pVfs, zFile, pVfs->mxPathname, p->zFile);
  sqlite3_free(zFile);

  flags = SQLITE_OPEN_READWRITE|SQLITE_OPEN_SUPER_JOURNAL;
  rc = pVfs->xOpen(pVfs, p->zFile, p->pFd, flags, &flags);

  ଯଦି( rc==SQLITE_OK ){
    p->pFd->pMethods->xFileSize(p->pFd, &p->nByte);
    sqlite3_declare_vtab(db, 
        "CREATE TABLE xxx(event, file, click, rc, size, offset)"
    );
    *ppVtab = &p->base;
  }ଅନ୍ୟଥା{
    sqlite3_free(p);
  }

  ଫେରନ୍ତୁ rc;
}

/*
** There is no "best-index". This virtual table always does a linear
** scan of the binary VFS log file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vlogBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  pIdxInfo->estimatedCost = 10.0;
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Disconnect from or destroy a vfslog virtual table.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vlogDisconnect(sqlite3_vtab *pVtab){
  VfslogVtab *p = (VfslogVtab *)pVtab;
  ଯଦି( p->pFd->pMethods ){
    p->pFd->pMethods->xClose(p->pFd);
    p->pFd->pMethods = 0;
  }
  sqlite3_free(p);
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Open a new vfslog cursor.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vlogOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  VfslogCsr *pCsr;                /* Newly allocated cursor object */

  pCsr = sqlite3_malloc(ଆକାର(VfslogCsr));
  ଯଦି( !pCsr ) ଫେରନ୍ତୁ SQLITE_NOMEM;
  memset(pCsr, 0, ଆକାର(VfslogCsr));
  *ppCursor = &pCsr->base;
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Close a vfslog cursor.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vlogClose(sqlite3_vtab_cursor *pCursor){
  VfslogCsr *p = (VfslogCsr *)pCursor;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପାଇଁ(i=0; i<p->nFile; i++){
    sqlite3_free(p->azFile[i]);
  }
  sqlite3_free(p->azFile);
  sqlite3_free(p->zTransient);
  sqlite3_free(p);
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Move a vfslog cursor to the next entry in the file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vlogNext(sqlite3_vtab_cursor *pCursor){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  VfslogVtab *p = (VfslogVtab *)pCursor->pVtab;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nRead;

  sqlite3_free(pCsr->zTransient);
  pCsr->zTransient = 0;

  nRead = 24;
  ଯଦି( pCsr->iOffset+nRead<=p->nByte ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା eEvent;
    rc = p->pFd->pMethods->xRead(p->pFd, pCsr->aBuf, nRead, pCsr->iOffset);

    eEvent = get32bits(pCsr->aBuf);
    ଯଦି( (rc==SQLITE_OK)
     && (eEvent==OS_OPEN || eEvent==OS_DELETE || eEvent==OS_ACCESS) 
    ){
      ଅକ୍ଷର buf[4];
      rc = p->pFd->pMethods->xRead(p->pFd, buf, 4, pCsr->iOffset+nRead);
      nRead += 4;
      ଯଦି( rc==SQLITE_OK ){
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା nStr = get32bits((unsigned ଅକ୍ଷର *)buf);
        ଅକ୍ଷର *zStr = sqlite3_malloc(nStr+1);
        rc = p->pFd->pMethods->xRead(p->pFd, zStr, nStr, pCsr->iOffset+nRead);
        zStr[nStr] = '\0';
        nRead += nStr;

        ଯଦି( eEvent==OS_OPEN ){
          ପୂର୍ଣ୍ଣସଂଖ୍ୟା iFileid = get32bits(&pCsr->aBuf[4]);
          ଯଦି( iFileid>=pCsr->nFile ){
            ପୂର୍ଣ୍ଣସଂଖ୍ୟା nNew = ଆକାର(pCsr->azFile[0])*(iFileid+1);
            pCsr->azFile = (ଅକ୍ଷର **)sqlite3_realloc(pCsr->azFile, nNew);
            nNew -= ଆକାର(pCsr->azFile[0])*pCsr->nFile;
            memset(&pCsr->azFile[pCsr->nFile], 0, nNew);
            pCsr->nFile = iFileid+1;
          }
          sqlite3_free(pCsr->azFile[iFileid]);
          pCsr->azFile[iFileid] = zStr;
        }ଅନ୍ୟଥା{
          pCsr->zTransient = zStr;
        }
      }
    }
  }

  pCsr->iRowid += 1;
  pCsr->iOffset += nRead;
  ଫେରନ୍ତୁ rc;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vlogEof(sqlite3_vtab_cursor *pCursor){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  VfslogVtab *p = (VfslogVtab *)pCursor->pVtab;
  ଫେରନ୍ତୁ (pCsr->iOffset>=p->nByte);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vlogFilter(
  sqlite3_vtab_cursor *pCursor, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx_ଅଙ୍କ, ଅଚଳ ଅକ୍ଷର *idxStr,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, sqlite3_ମୂଲ୍ୟ **argv
){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  pCsr->iRowid = 0;
  pCsr->iOffset = 20;
  ଫେରନ୍ତୁ vlogNext(pCursor);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vlogColumn(
  sqlite3_vtab_cursor *pCursor, 
  sqlite3_context *ctx, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i
){
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା val;
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;

  ପ୍ରତିଜ୍ଞା( i<7 );
  val = get32bits(&pCsr->aBuf[4*i]);

  ବାଛ( i ){
    ସ୍ଥିତି 0: {
      sqlite3_result_text(ctx, vfslog_eventname(val), -1, SQLITE_STATIC);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି 1: {
      ଅକ୍ଷର *zStr = pCsr->zTransient;
      ଯଦି( val!=0 && val<(unsigned)pCsr->nFile ){
        zStr = pCsr->azFile[val];
      }
      sqlite3_result_text(ctx, zStr, -1, SQLITE_TRANSIENT);
      ଭାଙ୍ଗ;
    }
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      sqlite3_result_int(ctx, val);
      ଭାଙ୍ଗ;
  }

  ଫେରନ୍ତୁ SQLITE_OK;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vlogRowid(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  *pRowid = pCsr->iRowid;
  ଫେରନ୍ତୁ SQLITE_OK;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_vfslog_register(sqlite3 *db){
  ସ୍ଥିର sqlite3_module vfslog_module = {
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
  ଫେରନ୍ତୁ SQLITE_OK;
}
#endif /* SQLITE_OMIT_VIRTUALTABLE */

/**************************************************************************
***************************************************************************
** Tcl interface starts here.
*/

#ଯଦି defined(SQLITE_TEST) || defined(TCLSH)

#ଅନ୍ତର୍ଭାଵ "tclsqlite.h"

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_vfslog(
  ଶୂନ୍ୟ *clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ସଂରଚନା SqliteDb { sqlite3 *db; };
  sqlite3 *db;
  Tcl_CmdInfo cmdInfo;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_ERROR;

  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *strs[] = { "annotate", "finalize", "new",  "register", 0 };
  ଗଣନା VL_enum { VL_ANNOTATE, VL_FINALIZE, VL_NEW, VL_REGISTER };
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iSub;

  ଯଦି( objc<2 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "SUB-COMMAND ...");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( Tcl_GetIndexFromObj(interp, objv[1], strs, "sub-command", 0, &iSub) ){
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  ବାଛ( (ଗଣନା VL_enum)iSub ){
    ସ୍ଥିତି VL_ANNOTATE: {
      ଅକ୍ଷର *zVfs;
      ଅକ୍ଷର *z_ସନ୍ଦେଶ;
      ଯଦି( objc!=4 ){
        Tcl_Wrong_ଅଙ୍କ_Args(interp, 3, objv, "VFS");
        ଫେରନ୍ତୁ TCL_ERROR;
      }
      zVfs = Tcl_GetString(objv[2]);
      z_ସନ୍ଦେଶ = Tcl_GetString(objv[3]);
      rc = sqlite3_vfslog_annotate(zVfs, z_ସନ୍ଦେଶ);
      ଯଦି( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp, "failed", (ଅକ୍ଷର*)0);
        ଫେରନ୍ତୁ TCL_ERROR;
      }
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି VL_FINALIZE: {
      ଅକ୍ଷର *zVfs;
      ଯଦି( objc!=3 ){
        Tcl_Wrong_ଅଙ୍କ_Args(interp, 2, objv, "VFS");
        ଫେରନ୍ତୁ TCL_ERROR;
      }
      zVfs = Tcl_GetString(objv[2]);
      rc = sqlite3_vfslog_finalize(zVfs);
      ଯଦି( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp, "failed", (ଅକ୍ଷର*)0);
        ଫେରନ୍ତୁ TCL_ERROR;
      }
      ଭାଙ୍ଗ;
    };

    ସ୍ଥିତି VL_NEW: {
      ଅକ୍ଷର *zVfs;
      ଅକ୍ଷର *zParent;
      ଅକ୍ଷର *zLog;
      ଯଦି( objc!=5 ){
        Tcl_Wrong_ଅଙ୍କ_Args(interp, 2, objv, "VFS PARENT LOGFILE");
        ଫେରନ୍ତୁ TCL_ERROR;
      }
      zVfs = Tcl_GetString(objv[2]);
      zParent = Tcl_GetString(objv[3]);
      zLog = Tcl_GetString(objv[4]);
      ଯଦି( *zParent=='\0' ) zParent = 0;
      rc = sqlite3_vfslog_new(zVfs, zParent, zLog);
      ଯଦି( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp, "failed", (ଅକ୍ଷର*)0);
        ଫେରନ୍ତୁ TCL_ERROR;
      }
      ଭାଙ୍ଗ;
    };

    ସ୍ଥିତି VL_REGISTER: {
      ଅକ୍ଷର *zDb;
      ଯଦି( objc!=3 ){
        Tcl_Wrong_ଅଙ୍କ_Args(interp, 2, objv, "DB");
        ଫେରନ୍ତୁ TCL_ERROR;
      }
#ifdef SQLITE_OMIT_VIRTUALTABLE
      Tcl_AppendResult(interp, "vfslog not available because of "
                               "SQLITE_OMIT_VIRTUALTABLE", (ଶୂନ୍ୟ*)0);
      ଫେରନ୍ତୁ TCL_ERROR;
#ଅନ୍ୟଥା
      zDb = Tcl_GetString(objv[2]);
      ଯଦି( Tcl_GetCommandInfo(interp, zDb, &cmdInfo) ){
        db = ((ସଂରଚନା SqliteDb*)cmdInfo.objClientData)->db;
        rc = sqlite3_vfslog_register(db);
      }
      ଯଦି( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp, "bad sqlite3 handle: ", zDb, (ଶୂନ୍ୟ*)0);
        ଫେରନ୍ତୁ TCL_ERROR;
      }
      ଭାଙ୍ଗ;
#endif
    }
  }

  ଫେରନ୍ତୁ TCL_OK;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା SqlitetestOsinst_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "vfslog", test_vfslog, 0, 0);
  ଫେରନ୍ତୁ TCL_OK;
}

#endif /* SQLITE_TEST */
