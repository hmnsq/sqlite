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

#ಅನ್ತರ್ಭಾವ "sqlite3.h"

#ifdef _WIN32
#ಅನ್ತರ್ಭಾವ <windows.h>
#endif

#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <assert.h>


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

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ VfslogVfs VfslogVfs;
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ VfslogFile VfslogFile;

ಸಂರಚನಾ VfslogVfs {
  sqlite3_vfs base;               /* VFS methods */
  sqlite3_vfs *pVfs;              /* Parent VFS */
  ಪೂರ್ಣಾಙ್ಕ iNextFileId;                /* Next file id */
  sqlite3_file *pLog;             /* Log file handle */
  sqlite3_int64 iOffset;          /* Log file offset of start of write buffer */
  ಪೂರ್ಣಾಙ್ಕ nBuf;                       /* Number of valid bytes in aBuf[] */
  ವರ್ಣ aBuf[VFSLOG_BUFFERSIZE];   /* Write buffer */
};

ಸಂರಚನಾ VfslogFile {
  sqlite3_file base;              /* IO methods */
  sqlite3_file *pReal;            /* Underlying file handle */
  sqlite3_vfs *pVfslog;           /* Associated VsflogVfs object */
  ಪೂರ್ಣಾಙ್ಕ iFileId;                    /* File id number */
};

#define REALVFS(p) (((VfslogVfs *)(p))->pVfs)



/*
** Method declarations for vfslog_file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vfslogClose(sqlite3_file*);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vfslogRead(sqlite3_file*, ಶೂನ್ಯ*, ಪೂರ್ಣಾಙ್ಕ iAmt, sqlite3_int64 iOfst);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vfslogWrite(sqlite3_file*,ಧ್ರುವ ಶೂನ್ಯ*,ಪೂರ್ಣಾಙ್ಕ iAmt, sqlite3_int64 iOfst);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vfslogTruncate(sqlite3_file*, sqlite3_int64 size);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vfslogSync(sqlite3_file*, ಪೂರ್ಣಾಙ್ಕ flags);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vfslogFileSize(sqlite3_file*, sqlite3_int64 *pSize);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vfslogLock(sqlite3_file*, ಪೂರ್ಣಾಙ್ಕ);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vfslogUnlock(sqlite3_file*, ಪೂರ್ಣಾಙ್ಕ);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vfslogCheckReservedLock(sqlite3_file*, ಪೂರ್ಣಾಙ್ಕ *pResOut);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vfslogFileControl(sqlite3_file*, ಪೂರ್ಣಾಙ್ಕ op, ಶೂನ್ಯ *pArg);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vfslogSectorSize(sqlite3_file*);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vfslogDeviceCharacteristics(sqlite3_file*);

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vfslogShmLock(sqlite3_file *pFile, ಪೂರ್ಣಾಙ್ಕ ofst, ಪೂರ್ಣಾಙ್ಕ n, ಪೂರ್ಣಾಙ್ಕ flags);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vfslogShmMap(sqlite3_file *pFile,ಪೂರ್ಣಾಙ್ಕ,ಪೂರ್ಣಾಙ್ಕ,ಪೂರ್ಣಾಙ್ಕ,ಚಞ್ಚಲ ಶೂನ್ಯ **);
ಸ್ಥಿರ ಶೂನ್ಯ vfslogShmBarrier(sqlite3_file*);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vfslogShmUnmap(sqlite3_file *pFile, ಪೂರ್ಣಾಙ್ಕ deleteFlag);

/*
** Method declarations for vfslog_vfs.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vfslogOpen(sqlite3_vfs*, ಧ್ರುವ ವರ್ಣ *, sqlite3_file*, ಪೂರ್ಣಾಙ್ಕ , ಪೂರ್ಣಾಙ್ಕ *);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vfslogDelete(sqlite3_vfs*, ಧ್ರುವ ವರ್ಣ *zName, ಪೂರ್ಣಾಙ್ಕ syncDir);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vfslogAccess(sqlite3_vfs*, ಧ್ರುವ ವರ್ಣ *zName, ಪೂರ್ಣಾಙ್ಕ flags, ಪೂರ್ಣಾಙ್ಕ *);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vfslogFullPathname(sqlite3_vfs*, ಧ್ರುವ ವರ್ಣ *zName, ಪೂರ್ಣಾಙ್ಕ, ವರ್ಣ *zOut);
ಸ್ಥಿರ ಶೂನ್ಯ *vfslogDlOpen(sqlite3_vfs*, ಧ್ರುವ ವರ್ಣ *zFilename);
ಸ್ಥಿರ ಶೂನ್ಯ vfslogDlError(sqlite3_vfs*, ಪೂರ್ಣಾಙ್ಕ nByte, ವರ್ಣ *z_Err_ಸಂದೇಶ);
ಸ್ಥಿರ ಶೂನ್ಯ (*vfslogDlSym(sqlite3_vfs *pVfs, ಶೂನ್ಯ *p, ಧ್ರುವ ವರ್ಣ*zSym))(ಶೂನ್ಯ);
ಸ್ಥಿರ ಶೂನ್ಯ vfslogDlClose(sqlite3_vfs*, ಶೂನ್ಯ*);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vfslogRandomness(sqlite3_vfs*, ಪೂರ್ಣಾಙ್ಕ nByte, ವರ್ಣ *zOut);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vfslogSleep(sqlite3_vfs*, ಪೂರ್ಣಾಙ್ಕ microseconds);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vfslogCurrentTime(sqlite3_vfs*, ದ್ವಿಗುಣ*);

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vfslogGetLastError(sqlite3_vfs*, ಪೂರ್ಣಾಙ್ಕ, ವರ್ಣ *);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vfslogCurrentTimeInt64(sqlite3_vfs*, sqlite3_int64*);

ಸ್ಥಿರ sqlite3_vfs vfslog_vfs = {
  1,                              /* iVersion */
  ಪರಿಮಾಣ(VfslogFile),             /* szOsFile */
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

ಸ್ಥಿರ sqlite3_io_methods vfslog_io_methods = {
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
#ಅನ್ತರ್ಭಾವ <time.h>
ಸ್ಥಿರ sqlite3_uint64 vfslog_time(){
  FILETIME ft;
  sqlite3_uint64 u64time = 0;
 
  GetSystemTimeAsFileTime(&ft);

  u64time |= ft.dw_ಮೇಲಿನ_Date_Time;
  u64time <<= 32;
  u64time |= ft.dw_ಕೆಳಗಿನ_Date_Time;

  /* ft is 100-nanosecond intervals, we want microseconds */
  ಪ್ರತ್ಯಾಗಮ u64time /(sqlite3_uint64)10;
}
#ನೋಚೇದ್ಯದಿ !defined(NO_GETTOD)
#ಅನ್ತರ್ಭಾವ <sys/time.h>
ಸ್ಥಿರ sqlite3_uint64 vfslog_time(){
  ಸಂರಚನಾ timeval sTime;
  gettimeofday(&sTime, 0);
  ಪ್ರತ್ಯಾಗಮ sTime.tv_usec + (sqlite3_uint64)sTime.tv_sec * 1000000;
}
#ಅನ್ಯಥಾ
ಸ್ಥಿರ sqlite3_uint64 vfslog_time(){
  ಪ್ರತ್ಯಾಗಮ 0;
}
#endif

ಸ್ಥಿರ ಶೂನ್ಯ vfslog_call(sqlite3_vfs *, ಪೂರ್ಣಾಙ್ಕ, ಪೂರ್ಣಾಙ್ಕ, sqlite3_int64, ಪೂರ್ಣಾಙ್ಕ, ಪೂರ್ಣಾಙ್ಕ, ಪೂರ್ಣಾಙ್ಕ);
ಸ್ಥಿರ ಶೂನ್ಯ vfslog_string(sqlite3_vfs *, ಧ್ರುವ ವರ್ಣ *);

/*
** Close an vfslog-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vfslogClose(sqlite3_file *pFile){
  sqlite3_uint64 t;
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;
  VfslogFile *p = (VfslogFile *)pFile;

  t = vfslog_time();
  ಯದಿ( p->pReal->pMethods ){
    rc = p->pReal->pMethods->xClose(p->pReal);
  }
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_CLOSE, p->iFileId, t, rc, 0, 0);
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Read data from an vfslog-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vfslogRead(
  sqlite3_file *pFile, 
  ಶೂನ್ಯ *zBuf, 
  ಪೂರ್ಣಾಙ್ಕ iAmt, 
  sqlite_int64 iOfst
){
  ಪೂರ್ಣಾಙ್ಕ rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xRead(p->pReal, zBuf, iAmt, iOfst);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_READ, p->iFileId, t, rc, iAmt, (ಪೂರ್ಣಾಙ್ಕ)iOfst);
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Write data to an vfslog-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vfslogWrite(
  sqlite3_file *pFile,
  ಧ್ರುವ ಶೂನ್ಯ *z,
  ಪೂರ್ಣಾಙ್ಕ iAmt,
  sqlite_int64 iOfst
){
  ಪೂರ್ಣಾಙ್ಕ rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xWrite(p->pReal, z, iAmt, iOfst);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_WRITE, p->iFileId, t, rc, iAmt, (ಪೂರ್ಣಾಙ್ಕ)iOfst);
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Truncate an vfslog-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vfslogTruncate(sqlite3_file *pFile, sqlite_int64 size){
  ಪೂರ್ಣಾಙ್ಕ rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xTruncate(p->pReal, size);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_TRUNCATE, p->iFileId, t, rc, 0, (ಪೂರ್ಣಾಙ್ಕ)size);
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Sync an vfslog-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vfslogSync(sqlite3_file *pFile, ಪೂರ್ಣಾಙ್ಕ flags){
  ಪೂರ್ಣಾಙ್ಕ rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xSync(p->pReal, flags);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SYNC, p->iFileId, t, rc, flags, 0);
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Return the current file-size of an vfslog-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vfslogFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  ಪೂರ್ಣಾಙ್ಕ rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xFileSize(p->pReal, pSize);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_FILESIZE, p->iFileId, t, rc, 0, (ಪೂರ್ಣಾಙ್ಕ)*pSize);
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Lock an vfslog-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vfslogLock(sqlite3_file *pFile, ಪೂರ್ಣಾಙ್ಕ eLock){
  ಪೂರ್ಣಾಙ್ಕ rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xLock(p->pReal, eLock);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_LOCK, p->iFileId, t, rc, eLock, 0);
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Unlock an vfslog-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vfslogUnlock(sqlite3_file *pFile, ಪೂರ್ಣಾಙ್ಕ eLock){
  ಪೂರ್ಣಾಙ್ಕ rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xUnlock(p->pReal, eLock);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_UNLOCK, p->iFileId, t, rc, eLock, 0);
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Check if another file-handle holds a RESERVED lock on an vfslog-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vfslogCheckReservedLock(sqlite3_file *pFile, ಪೂರ್ಣಾಙ್ಕ *pResOut){
  ಪೂರ್ಣಾಙ್ಕ rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xCheckReservedLock(p->pReal, pResOut);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_CHECKRESERVEDLOCK, p->iFileId, t, rc, *pResOut, 0);
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** File control method. For custom operations on an vfslog-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vfslogFileControl(sqlite3_file *pFile, ಪೂರ್ಣಾಙ್ಕ op, ಶೂನ್ಯ *pArg){
  VfslogFile *p = (VfslogFile *)pFile;
  ಪೂರ್ಣಾಙ್ಕ rc = p->pReal->pMethods->xFileControl(p->pReal, op, pArg);
  ಯದಿ( op==SQLITE_FCNTL_VFSNAME && rc==SQLITE_OK ){
    *(ವರ್ಣ**)pArg = sqlite3_mprintf("vfslog/%z", *(ವರ್ಣ**)pArg);
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Return the sector-size in bytes for an vfslog-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vfslogSectorSize(sqlite3_file *pFile){
  ಪೂರ್ಣಾಙ್ಕ rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xSectorSize(p->pReal);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SECTORSIZE, p->iFileId, t, rc, 0, 0);
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Return the device characteristic flags supported by an vfslog-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vfslogDeviceCharacteristics(sqlite3_file *pFile){
  ಪೂರ್ಣಾಙ್ಕ rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xDeviceCharacteristics(p->pReal);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_DEVCHAR, p->iFileId, t, rc, 0, 0);
  ಪ್ರತ್ಯಾಗಮ rc;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vfslogShmLock(sqlite3_file *pFile, ಪೂರ್ಣಾಙ್ಕ ofst, ಪೂರ್ಣಾಙ್ಕ n, ಪೂರ್ಣಾಙ್ಕ flags){
  ಪೂರ್ಣಾಙ್ಕ rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xShmLock(p->pReal, ofst, n, flags);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SHMLOCK, p->iFileId, t, rc, 0, 0);
  ಪ್ರತ್ಯಾಗಮ rc;
}
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vfslogShmMap(
  sqlite3_file *pFile, 
  ಪೂರ್ಣಾಙ್ಕ iRegion, 
  ಪೂರ್ಣಾಙ್ಕ szRegion, 
  ಪೂರ್ಣಾಙ್ಕ isWrite, 
  ಚಞ್ಚಲ ಶೂನ್ಯ **pp
){
  ಪೂರ್ಣಾಙ್ಕ rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xShmMap(p->pReal, iRegion, szRegion, isWrite, pp);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SHMMAP, p->iFileId, t, rc, 0, 0);
  ಪ್ರತ್ಯಾಗಮ rc;
}
ಸ್ಥಿರ ಶೂನ್ಯ vfslogShmBarrier(sqlite3_file *pFile){
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  p->pReal->pMethods->xShmBarrier(p->pReal);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SHMBARRIER, p->iFileId, t, SQLITE_OK, 0, 0);
}
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vfslogShmUnmap(sqlite3_file *pFile, ಪೂರ್ಣಾಙ್ಕ deleteFlag){
  ಪೂರ್ಣಾಙ್ಕ rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xShmUnmap(p->pReal, deleteFlag);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SHMUNMAP, p->iFileId, t, rc, 0, 0);
  ಪ್ರತ್ಯಾಗಮ rc;
}


/*
** Open an vfslog file handle.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vfslogOpen(
  sqlite3_vfs *pVfs,
  ಧ್ರುವ ವರ್ಣ *zName,
  sqlite3_file *pFile,
  ಪೂರ್ಣಾಙ್ಕ flags,
  ಪೂರ್ಣಾಙ್ಕ *pOutFlags
){
  ಪೂರ್ಣಾಙ್ಕ rc;
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
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vfslogDelete(sqlite3_vfs *pVfs, ಧ್ರುವ ವರ್ಣ *zPath, ಪೂರ್ಣಾಙ್ಕ dirSync){
  ಪೂರ್ಣಾಙ್ಕ rc;
  sqlite3_uint64 t;
  t = vfslog_time();
  rc = REALVFS(pVfs)->xDelete(REALVFS(pVfs), zPath, dirSync);
  t = vfslog_time() - t;
  vfslog_call(pVfs, OS_DELETE, 0, t, rc, dirSync, 0);
  vfslog_string(pVfs, zPath);
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vfslogAccess(
  sqlite3_vfs *pVfs, 
  ಧ್ರುವ ವರ್ಣ *zPath, 
  ಪೂರ್ಣಾಙ್ಕ flags, 
  ಪೂರ್ಣಾಙ್ಕ *pResOut
){
  ಪೂರ್ಣಾಙ್ಕ rc;
  sqlite3_uint64 t;
  t = vfslog_time();
  rc = REALVFS(pVfs)->xAccess(REALVFS(pVfs), zPath, flags, pResOut);
  t = vfslog_time() - t;
  vfslog_call(pVfs, OS_ACCESS, 0, t, rc, flags, *pResOut);
  vfslog_string(pVfs, zPath);
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vfslogFullPathname(
  sqlite3_vfs *pVfs, 
  ಧ್ರುವ ವರ್ಣ *zPath, 
  ಪೂರ್ಣಾಙ್ಕ nOut, 
  ವರ್ಣ *zOut
){
  ಪ್ರತ್ಯಾಗಮ REALVFS(pVfs)->xFullPathname(REALVFS(pVfs), zPath, nOut, zOut);
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
ಸ್ಥಿರ ಶೂನ್ಯ *vfslogDlOpen(sqlite3_vfs *pVfs, ಧ್ರುವ ವರ್ಣ *zPath){
  ಪ್ರತ್ಯಾಗಮ REALVFS(pVfs)->xDlOpen(REALVFS(pVfs), zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
ಸ್ಥಿರ ಶೂನ್ಯ vfslogDlError(sqlite3_vfs *pVfs, ಪೂರ್ಣಾಙ್ಕ nByte, ವರ್ಣ *z_Err_ಸಂದೇಶ){
  REALVFS(pVfs)->xDlError(REALVFS(pVfs), nByte, z_Err_ಸಂದೇಶ);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
ಸ್ಥಿರ ಶೂನ್ಯ (*vfslogDlSym(sqlite3_vfs *pVfs, ಶೂನ್ಯ *p, ಧ್ರುವ ವರ್ಣ *zSym))(ಶೂನ್ಯ){
  ಪ್ರತ್ಯಾಗಮ REALVFS(pVfs)->xDlSym(REALVFS(pVfs), p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
ಸ್ಥಿರ ಶೂನ್ಯ vfslogDlClose(sqlite3_vfs *pVfs, ಶೂನ್ಯ *pHandle){
  REALVFS(pVfs)->xDlClose(REALVFS(pVfs), pHandle);
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vfslogRandomness(sqlite3_vfs *pVfs, ಪೂರ್ಣಾಙ್ಕ nByte, ವರ್ಣ *zBufOut){
  ಪ್ರತ್ಯಾಗಮ REALVFS(pVfs)->xRandomness(REALVFS(pVfs), nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vfslogSleep(sqlite3_vfs *pVfs, ಪೂರ್ಣಾಙ್ಕ nMicro){
  ಪ್ರತ್ಯಾಗಮ REALVFS(pVfs)->xSleep(REALVFS(pVfs), nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vfslogCurrentTime(sqlite3_vfs *pVfs, ದ್ವಿಗುಣ *pTimeOut){
  ಪ್ರತ್ಯಾಗಮ REALVFS(pVfs)->xCurrentTime(REALVFS(pVfs), pTimeOut);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vfslogGetLastError(sqlite3_vfs *pVfs, ಪೂರ್ಣಾಙ್ಕ a, ವರ್ಣ *b){
  ಪ್ರತ್ಯಾಗಮ REALVFS(pVfs)->xGetLastError(REALVFS(pVfs), a, b);
}
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vfslogCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *p){
  ಪ್ರತ್ಯಾಗಮ REALVFS(pVfs)->xCurrentTimeInt64(REALVFS(pVfs), p);
}

ಸ್ಥಿರ ಶೂನ್ಯ vfslog_flush(VfslogVfs *p){
#ifdef SQLITE_TEST
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ sqlite3_io_error_pending;
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ sqlite3_io_error_persist;
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ sqlite3_diskfull_pending;

  ಪೂರ್ಣಾಙ್ಕ pending = sqlite3_io_error_pending;
  ಪೂರ್ಣಾಙ್ಕ persist = sqlite3_io_error_persist;
  ಪೂರ್ಣಾಙ್ಕ diskfull = sqlite3_diskfull_pending;

  sqlite3_io_error_pending = 0;
  sqlite3_io_error_persist = 0;
  sqlite3_diskfull_pending = 0;
#endif

  ಯದಿ( p->nBuf ){
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

ಸ್ಥಿರ ಶೂನ್ಯ put32bits(unsigned ವರ್ಣ *p, unsigned ಪೂರ್ಣಾಙ್ಕ v){
  p[0] = v>>24;
  p[1] = (unsigned ವರ್ಣ)(v>>16);
  p[2] = (unsigned ವರ್ಣ)(v>>8);
  p[3] = (unsigned ವರ್ಣ)v;
}

ಸ್ಥಿರ ಶೂನ್ಯ vfslog_call(
  sqlite3_vfs *pVfs,
  ಪೂರ್ಣಾಙ್ಕ eEvent,
  ಪೂರ್ಣಾಙ್ಕ iFileid,
  sqlite3_int64 nClick,
  ಪೂರ್ಣಾಙ್ಕ return_code,
  ಪೂರ್ಣಾಙ್ಕ size,
  ಪೂರ್ಣಾಙ್ಕ offset
){
  VfslogVfs *p = (VfslogVfs *)pVfs;
  unsigned ವರ್ಣ *zRec;
  ಯದಿ( (24+p->nBuf)>ಪರಿಮಾಣ(p->aBuf) ){
    vfslog_flush(p);
  }
  zRec = (unsigned ವರ್ಣ *)&p->aBuf[p->nBuf];
  put32bits(&zRec[0], eEvent);
  put32bits(&zRec[4], iFileid);
  put32bits(&zRec[8], (unsigned ಪೂರ್ಣಾಙ್ಕ)(nClick&0xffff));
  put32bits(&zRec[12], return_code);
  put32bits(&zRec[16], size);
  put32bits(&zRec[20], offset);
  p->nBuf += 24;
}

ಸ್ಥಿರ ಶೂನ್ಯ vfslog_string(sqlite3_vfs *pVfs, ಧ್ರುವ ವರ್ಣ *zStr){
  VfslogVfs *p = (VfslogVfs *)pVfs;
  unsigned ವರ್ಣ *zRec;
  ಪೂರ್ಣಾಙ್ಕ nStr = zStr ? (ಪೂರ್ಣಾಙ್ಕ)strlen(zStr) : 0;
  ಯದಿ( (4+nStr+p->nBuf)>ಪರಿಮಾಣ(p->aBuf) ){
    vfslog_flush(p);
  }
  zRec = (unsigned ವರ್ಣ *)&p->aBuf[p->nBuf];
  put32bits(&zRec[0], nStr);
  ಯದಿ( zStr ){
    memcpy(&zRec[4], zStr, nStr);
  }
  p->nBuf += (4 + nStr);
}

ಸ್ಥಿರ ಶೂನ್ಯ vfslog_finalize(VfslogVfs *p){
  ಯದಿ( p->pLog->pMethods ){
    vfslog_flush(p);
    p->pLog->pMethods->xClose(p->pLog);
  }
  sqlite3_free(p);
}

ಪೂರ್ಣಾಙ್ಕ sqlite3_vfslog_finalize(ಧ್ರುವ ವರ್ಣ *zVfs){
  sqlite3_vfs *pVfs;
  pVfs = sqlite3_vfs_find(zVfs);
  ಯದಿ( !pVfs || pVfs->xOpen!=vfslogOpen ){
    ಪ್ರತ್ಯಾಗಮ SQLITE_ERROR;
  } 
  sqlite3_vfs_unregister(pVfs);
  vfslog_finalize((VfslogVfs *)pVfs);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

ಪೂರ್ಣಾಙ್ಕ sqlite3_vfslog_new(
  ಧ್ರುವ ವರ್ಣ *zVfs,               /* New VFS name */
  ಧ್ರುವ ವರ್ಣ *zParentVfs,         /* Parent VFS name (or NULL) */
  ಧ್ರುವ ವರ್ಣ *zLog                /* Log file name */
){
  VfslogVfs *p;
  sqlite3_vfs *pParent;
  ಪೂರ್ಣಾಙ್ಕ nByte;
  ಪೂರ್ಣಾಙ್ಕ flags;
  ಪೂರ್ಣಾಙ್ಕ rc;
  ವರ್ಣ *zFile;
  ಪೂರ್ಣಾಙ್ಕ nVfs;

  pParent = sqlite3_vfs_find(zParentVfs);
  ಯದಿ( !pParent ){
    ಪ್ರತ್ಯಾಗಮ SQLITE_ERROR;
  }

  nVfs = (ಪೂರ್ಣಾಙ್ಕ)strlen(zVfs);
  nByte = ಪರಿಮಾಣ(VfslogVfs) + pParent->szOsFile + nVfs+1+pParent->mxPathname+1;
  p = (VfslogVfs *)sqlite3_malloc(nByte);
  memset(p, 0, nByte);

  p->pVfs = pParent;
  p->pLog = (sqlite3_file *)&p[1];
  memcpy(&p->base, &vfslog_vfs, ಪರಿಮಾಣ(sqlite3_vfs));
  p->base.zName = &((ವರ್ಣ *)p->pLog)[pParent->szOsFile];
  p->base.szOsFile += pParent->szOsFile;
  memcpy((ವರ್ಣ *)p->base.zName, zVfs, nVfs);

  zFile = (ವರ್ಣ *)&p->base.zName[nVfs+1];
  pParent->xFullPathname(pParent, zLog, pParent->mxPathname, zFile);

  flags = SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE|SQLITE_OPEN_SUPER_JOURNAL;
  pParent->xDelete(pParent, zFile, 0);
  rc = pParent->xOpen(pParent, zFile, p->pLog, flags, &flags);
  ಯದಿ( rc==SQLITE_OK ){
    memcpy(p->aBuf, "sqlite_ostrace1.....", 20);
    p->iOffset = 0;
    p->nBuf = 20;
    rc = sqlite3_vfs_register((sqlite3_vfs *)p, 1);
  }
  ಯದಿ( rc ){
    vfslog_finalize(p);
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}

ಪೂರ್ಣಾಙ್ಕ sqlite3_vfslog_annotate(ಧ್ರುವ ವರ್ಣ *zVfs, ಧ್ರುವ ವರ್ಣ *z_ಸಂದೇಶ){
  sqlite3_vfs *pVfs;
  pVfs = sqlite3_vfs_find(zVfs);
  ಯದಿ( !pVfs || pVfs->xOpen!=vfslogOpen ){
    ಪ್ರತ್ಯಾಗಮ SQLITE_ERROR;
  } 
  vfslog_call(pVfs, OS_ANNOTATE, 0, 0, 0, 0, 0);
  vfslog_string(pVfs, z_ಸಂದೇಶ);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *vfslog_eventname(ಪೂರ್ಣಾಙ್ಕ eEvent){
  ಧ್ರುವ ವರ್ಣ *zEvent = 0;

  ವಿಕಲ್ಪ( eEvent ){
    ಪಕ್ಷ OS_CLOSE:             zEvent = "xClose"; ಭಙ್ಗ;
    ಪಕ್ಷ OS_READ:              zEvent = "xRead"; ಭಙ್ಗ;
    ಪಕ್ಷ OS_WRITE:             zEvent = "xWrite"; ಭಙ್ಗ;
    ಪಕ್ಷ OS_TRUNCATE:          zEvent = "xTruncate"; ಭಙ್ಗ;
    ಪಕ್ಷ OS_SYNC:              zEvent = "xSync"; ಭಙ್ಗ;
    ಪಕ್ಷ OS_FILESIZE:          zEvent = "xFilesize"; ಭಙ್ಗ;
    ಪಕ್ಷ OS_LOCK:              zEvent = "xLock"; ಭಙ್ಗ;
    ಪಕ್ಷ OS_UNLOCK:            zEvent = "xUnlock"; ಭಙ್ಗ;
    ಪಕ್ಷ OS_CHECKRESERVEDLOCK: zEvent = "xCheckResLock"; ಭಙ್ಗ;
    ಪಕ್ಷ OS_FILECONTROL:       zEvent = "xFileControl"; ಭಙ್ಗ;
    ಪಕ್ಷ OS_SECTORSIZE:        zEvent = "xSectorSize"; ಭಙ್ಗ;
    ಪಕ್ಷ OS_DEVCHAR:           zEvent = "xDeviceChar"; ಭಙ್ಗ;
    ಪಕ್ಷ OS_OPEN:              zEvent = "xOpen"; ಭಙ್ಗ;
    ಪಕ್ಷ OS_DELETE:            zEvent = "xDelete"; ಭಙ್ಗ;
    ಪಕ್ಷ OS_ACCESS:            zEvent = "xAccess"; ಭಙ್ಗ;
    ಪಕ್ಷ OS_FULLPATHNAME:      zEvent = "xFullPathname"; ಭಙ್ಗ;
    ಪಕ್ಷ OS_RANDOMNESS:        zEvent = "xRandomness"; ಭಙ್ಗ;
    ಪಕ್ಷ OS_SLEEP:             zEvent = "xSleep"; ಭಙ್ಗ;
    ಪಕ್ಷ OS_CURRENTTIME:       zEvent = "xCurrentTime"; ಭಙ್ಗ;

    ಪಕ್ಷ OS_SHMUNMAP:          zEvent = "xShmUnmap"; ಭಙ್ಗ;
    ಪಕ್ಷ OS_SHMLOCK:           zEvent = "xShmLock"; ಭಙ್ಗ;
    ಪಕ್ಷ OS_SHMBARRIER:        zEvent = "xShmBarrier"; ಭಙ್ಗ;
    ಪಕ್ಷ OS_SHMMAP:            zEvent = "xShmMap"; ಭಙ್ಗ;

    ಪಕ್ಷ OS_ANNOTATE:          zEvent = "annotation"; ಭಙ್ಗ;
  }

  ಪ್ರತ್ಯಾಗಮ zEvent;
}

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ VfslogVtab VfslogVtab;
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ VfslogCsr VfslogCsr;

/*
** Virtual table type for the vfslog reader module.
*/
ಸಂರಚನಾ VfslogVtab {
  sqlite3_vtab base;              /* Base class */
  sqlite3_file *pFd;              /* File descriptor open on vfslog file */
  sqlite3_int64 nByte;            /* Size of file in bytes */
  ವರ್ಣ *zFile;                    /* File name for pFd */
};

/*
** Virtual table cursor type for the vfslog reader module.
*/
ಸಂರಚನಾ VfslogCsr {
  sqlite3_vtab_cursor base;       /* Base class */
  sqlite3_int64 iRowid;           /* Current rowid. */
  sqlite3_int64 iOffset;          /* Offset of next record in file */
  ವರ್ಣ *zTransient;               /* Transient 'file' string */
  ಪೂರ್ಣಾಙ್ಕ nFile;                      /* Size of array azFile[] */
  ವರ್ಣ **azFile;                  /* File strings */
  unsigned ವರ್ಣ aBuf[1024];       /* Current vfs log entry (read from file) */
};

ಸ್ಥಿರ unsigned ಪೂರ್ಣಾಙ್ಕ get32bits(unsigned ವರ್ಣ *p){
  ಪ್ರತ್ಯಾಗಮ (p[0]<<24) + (p[1]<<16) + (p[2]<<8) + p[3];
}

/*
** The argument must point to a buffer containing a nul-terminated string.
** If the string begins with an SQL quote character it is overwritten by
** the dequoted version. Otherwise the buffer is left unmodified.
*/
ಸ್ಥಿರ ಶೂನ್ಯ dequote(ವರ್ಣ *z){
  ವರ್ಣ quote;                     /* Quote character (if any ) */
  quote = z[0];
  ಯದಿ( quote=='[' || quote=='\'' || quote=='"' || quote=='`' ){
    ಪೂರ್ಣಾಙ್ಕ iIn = 1;                  /* Index of next byte to read from input */
    ಪೂರ್ಣಾಙ್ಕ iOut = 0;                 /* Index of next byte to write to output */
    ಯದಿ( quote=='[' ) quote = ']';  
    ಯಾವತ್( z[iIn] ){
      ಯದಿ( z[iIn]==quote ){
        ಯದಿ( z[iIn+1]!=quote ) ಭಙ್ಗ;
        z[iOut++] = quote;
        iIn += 2;
      }ಅನ್ಯಥಾ{
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vlogConnect(
  sqlite3 *db,
  ಶೂನ್ಯ *pAux,
  ಪೂರ್ಣಾಙ್ಕ argc, ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ*argv,
  sqlite3_vtab **ppVtab,
  ವರ್ಣ **pzErr
){
  sqlite3_vfs *pVfs;              /* VFS used to read log file */
  ಪೂರ್ಣಾಙ್ಕ flags;                      /* flags passed to pVfs->xOpen() */
  VfslogVtab *p;
  ಪೂರ್ಣಾಙ್ಕ rc;
  ಪೂರ್ಣಾಙ್ಕ nByte;
  ವರ್ಣ *zFile;

  *ppVtab = 0;
  pVfs = sqlite3_vfs_find(0);
  nByte = ಪರಿಮಾಣ(VfslogVtab) + pVfs->szOsFile + pVfs->mxPathname;
  p = sqlite3_malloc(nByte);
  ಯದಿ( p==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
  memset(p, 0, nByte);

  p->pFd = (sqlite3_file *)&p[1];
  p->zFile = &((ವರ್ಣ *)p->pFd)[pVfs->szOsFile];

  zFile = sqlite3_mprintf("%s", argv[3]);
  ಯದಿ( !zFile ){
    sqlite3_free(p);
    ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
  }
  dequote(zFile);
  pVfs->xFullPathname(pVfs, zFile, pVfs->mxPathname, p->zFile);
  sqlite3_free(zFile);

  flags = SQLITE_OPEN_READWRITE|SQLITE_OPEN_SUPER_JOURNAL;
  rc = pVfs->xOpen(pVfs, p->zFile, p->pFd, flags, &flags);

  ಯದಿ( rc==SQLITE_OK ){
    p->pFd->pMethods->xFileSize(p->pFd, &p->nByte);
    sqlite3_declare_vtab(db, 
        "CREATE TABLE xxx(event, file, click, rc, size, offset)"
    );
    *ppVtab = &p->base;
  }ಅನ್ಯಥಾ{
    sqlite3_free(p);
  }

  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** There is no "best-index". This virtual table always does a linear
** scan of the binary VFS log file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vlogBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  pIdxInfo->estimatedCost = 10.0;
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Disconnect from or destroy a vfslog virtual table.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vlogDisconnect(sqlite3_vtab *pVtab){
  VfslogVtab *p = (VfslogVtab *)pVtab;
  ಯದಿ( p->pFd->pMethods ){
    p->pFd->pMethods->xClose(p->pFd);
    p->pFd->pMethods = 0;
  }
  sqlite3_free(p);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Open a new vfslog cursor.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vlogOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  VfslogCsr *pCsr;                /* Newly allocated cursor object */

  pCsr = sqlite3_malloc(ಪರಿಮಾಣ(VfslogCsr));
  ಯದಿ( !pCsr ) ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
  memset(pCsr, 0, ಪರಿಮಾಣ(VfslogCsr));
  *ppCursor = &pCsr->base;
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Close a vfslog cursor.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vlogClose(sqlite3_vtab_cursor *pCursor){
  VfslogCsr *p = (VfslogCsr *)pCursor;
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪ್ರತಿ(i=0; i<p->nFile; i++){
    sqlite3_free(p->azFile[i]);
  }
  sqlite3_free(p->azFile);
  sqlite3_free(p->zTransient);
  sqlite3_free(p);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Move a vfslog cursor to the next entry in the file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vlogNext(sqlite3_vtab_cursor *pCursor){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  VfslogVtab *p = (VfslogVtab *)pCursor->pVtab;
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;
  ಪೂರ್ಣಾಙ್ಕ nRead;

  sqlite3_free(pCsr->zTransient);
  pCsr->zTransient = 0;

  nRead = 24;
  ಯದಿ( pCsr->iOffset+nRead<=p->nByte ){
    ಪೂರ್ಣಾಙ್ಕ eEvent;
    rc = p->pFd->pMethods->xRead(p->pFd, pCsr->aBuf, nRead, pCsr->iOffset);

    eEvent = get32bits(pCsr->aBuf);
    ಯದಿ( (rc==SQLITE_OK)
     && (eEvent==OS_OPEN || eEvent==OS_DELETE || eEvent==OS_ACCESS) 
    ){
      ವರ್ಣ buf[4];
      rc = p->pFd->pMethods->xRead(p->pFd, buf, 4, pCsr->iOffset+nRead);
      nRead += 4;
      ಯದಿ( rc==SQLITE_OK ){
        ಪೂರ್ಣಾಙ್ಕ nStr = get32bits((unsigned ವರ್ಣ *)buf);
        ವರ್ಣ *zStr = sqlite3_malloc(nStr+1);
        rc = p->pFd->pMethods->xRead(p->pFd, zStr, nStr, pCsr->iOffset+nRead);
        zStr[nStr] = '\0';
        nRead += nStr;

        ಯದಿ( eEvent==OS_OPEN ){
          ಪೂರ್ಣಾಙ್ಕ iFileid = get32bits(&pCsr->aBuf[4]);
          ಯದಿ( iFileid>=pCsr->nFile ){
            ಪೂರ್ಣಾಙ್ಕ nNew = ಪರಿಮಾಣ(pCsr->azFile[0])*(iFileid+1);
            pCsr->azFile = (ವರ್ಣ **)sqlite3_realloc(pCsr->azFile, nNew);
            nNew -= ಪರಿಮಾಣ(pCsr->azFile[0])*pCsr->nFile;
            memset(&pCsr->azFile[pCsr->nFile], 0, nNew);
            pCsr->nFile = iFileid+1;
          }
          sqlite3_free(pCsr->azFile[iFileid]);
          pCsr->azFile[iFileid] = zStr;
        }ಅನ್ಯಥಾ{
          pCsr->zTransient = zStr;
        }
      }
    }
  }

  pCsr->iRowid += 1;
  pCsr->iOffset += nRead;
  ಪ್ರತ್ಯಾಗಮ rc;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vlogEof(sqlite3_vtab_cursor *pCursor){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  VfslogVtab *p = (VfslogVtab *)pCursor->pVtab;
  ಪ್ರತ್ಯಾಗಮ (pCsr->iOffset>=p->nByte);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vlogFilter(
  sqlite3_vtab_cursor *pCursor, 
  ಪೂರ್ಣಾಙ್ಕ idx_ಅಂಕೆ, ಧ್ರುವ ವರ್ಣ *idxStr,
  ಪೂರ್ಣಾಙ್ಕ argc, sqlite3_ಮೌಲ್ಯ **argv
){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  pCsr->iRowid = 0;
  pCsr->iOffset = 20;
  ಪ್ರತ್ಯಾಗಮ vlogNext(pCursor);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vlogColumn(
  sqlite3_vtab_cursor *pCursor, 
  sqlite3_context *ctx, 
  ಪೂರ್ಣಾಙ್ಕ i
){
  unsigned ಪೂರ್ಣಾಙ್ಕ val;
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;

  ಪ್ರತಿಜ್ಞಾ( i<7 );
  val = get32bits(&pCsr->aBuf[4*i]);

  ವಿಕಲ್ಪ( i ){
    ಪಕ್ಷ 0: {
      sqlite3_result_text(ctx, vfslog_eventname(val), -1, SQLITE_STATIC);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ 1: {
      ವರ್ಣ *zStr = pCsr->zTransient;
      ಯದಿ( val!=0 && val<(unsigned)pCsr->nFile ){
        zStr = pCsr->azFile[val];
      }
      sqlite3_result_text(ctx, zStr, -1, SQLITE_TRANSIENT);
      ಭಙ್ಗ;
    }
    ಸ್ವಭಾವ:
      sqlite3_result_int(ctx, val);
      ಭಙ್ಗ;
  }

  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vlogRowid(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  *pRowid = pCsr->iRowid;
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

ಪೂರ್ಣಾಙ್ಕ sqlite3_vfslog_register(sqlite3 *db){
  ಸ್ಥಿರ sqlite3_module vfslog_module = {
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
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}
#endif /* SQLITE_OMIT_VIRTUALTABLE */

/**************************************************************************
***************************************************************************
** Tcl interface starts here.
*/

#ಯದಿ defined(SQLITE_TEST) || defined(TCLSH)

#ಅನ್ತರ್ಭಾವ "tclsqlite.h"

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_vfslog(
  ಶೂನ್ಯ *clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಸಂರಚನಾ SqliteDb { sqlite3 *db; };
  sqlite3 *db;
  Tcl_CmdInfo cmdInfo;
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_ERROR;

  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *strs[] = { "annotate", "finalize", "new",  "register", 0 };
  ಗಣನಾ VL_enum { VL_ANNOTATE, VL_FINALIZE, VL_NEW, VL_REGISTER };
  ಪೂರ್ಣಾಙ್ಕ iSub;

  ಯದಿ( objc<2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "SUB-COMMAND ...");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( Tcl_GetIndexFromObj(interp, objv[1], strs, "sub-command", 0, &iSub) ){
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  ವಿಕಲ್ಪ( (ಗಣನಾ VL_enum)iSub ){
    ಪಕ್ಷ VL_ANNOTATE: {
      ವರ್ಣ *zVfs;
      ವರ್ಣ *z_ಸಂದೇಶ;
      ಯದಿ( objc!=4 ){
        Tcl_Wrong_ಅಂಕೆ_Args(interp, 3, objv, "VFS");
        ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
      }
      zVfs = Tcl_GetString(objv[2]);
      z_ಸಂದೇಶ = Tcl_GetString(objv[3]);
      rc = sqlite3_vfslog_annotate(zVfs, z_ಸಂದೇಶ);
      ಯದಿ( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp, "failed", (ವರ್ಣ*)0);
        ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
      }
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ VL_FINALIZE: {
      ವರ್ಣ *zVfs;
      ಯದಿ( objc!=3 ){
        Tcl_Wrong_ಅಂಕೆ_Args(interp, 2, objv, "VFS");
        ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
      }
      zVfs = Tcl_GetString(objv[2]);
      rc = sqlite3_vfslog_finalize(zVfs);
      ಯದಿ( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp, "failed", (ವರ್ಣ*)0);
        ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
      }
      ಭಙ್ಗ;
    };

    ಪಕ್ಷ VL_NEW: {
      ವರ್ಣ *zVfs;
      ವರ್ಣ *zParent;
      ವರ್ಣ *zLog;
      ಯದಿ( objc!=5 ){
        Tcl_Wrong_ಅಂಕೆ_Args(interp, 2, objv, "VFS PARENT LOGFILE");
        ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
      }
      zVfs = Tcl_GetString(objv[2]);
      zParent = Tcl_GetString(objv[3]);
      zLog = Tcl_GetString(objv[4]);
      ಯದಿ( *zParent=='\0' ) zParent = 0;
      rc = sqlite3_vfslog_new(zVfs, zParent, zLog);
      ಯದಿ( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp, "failed", (ವರ್ಣ*)0);
        ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
      }
      ಭಙ್ಗ;
    };

    ಪಕ್ಷ VL_REGISTER: {
      ವರ್ಣ *zDb;
      ಯದಿ( objc!=3 ){
        Tcl_Wrong_ಅಂಕೆ_Args(interp, 2, objv, "DB");
        ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
      }
#ifdef SQLITE_OMIT_VIRTUALTABLE
      Tcl_AppendResult(interp, "vfslog not available because of "
                               "SQLITE_OMIT_VIRTUALTABLE", (ಶೂನ್ಯ*)0);
      ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
#ಅನ್ಯಥಾ
      zDb = Tcl_GetString(objv[2]);
      ಯದಿ( Tcl_GetCommandInfo(interp, zDb, &cmdInfo) ){
        db = ((ಸಂರಚನಾ SqliteDb*)cmdInfo.objClientData)->db;
        rc = sqlite3_vfslog_register(db);
      }
      ಯದಿ( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp, "bad sqlite3 handle: ", zDb, (ಶೂನ್ಯ*)0);
        ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
      }
      ಭಙ್ಗ;
#endif
    }
  }

  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

ಪೂರ್ಣಾಙ್ಕ SqlitetestOsinst_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "vfslog", test_vfslog, 0, 0);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

#endif /* SQLITE_TEST */
