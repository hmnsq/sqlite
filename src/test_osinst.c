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

#അന്തര്ഭാവ "sqlite3.h"

#ifdef _WIN32
#അന്തര്ഭാവ <windows.h>
#endif

#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <assert.h>


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

തരനിർവചനം ഘടന VfslogVfs VfslogVfs;
തരനിർവചനം ഘടന VfslogFile VfslogFile;

ഘടന VfslogVfs {
  sqlite3_vfs base;               /* VFS methods */
  sqlite3_vfs *pVfs;              /* Parent VFS */
  പൂർണ്ണസംഖ്യ iNextFileId;                /* Next file id */
  sqlite3_file *pLog;             /* Log file handle */
  sqlite3_int64 iOffset;          /* Log file offset of start of write buffer */
  പൂർണ്ണസംഖ്യ nBuf;                       /* Number of valid bytes in aBuf[] */
  അക്ഷരം aBuf[VFSLOG_BUFFERSIZE];   /* Write buffer */
};

ഘടന VfslogFile {
  sqlite3_file base;              /* IO methods */
  sqlite3_file *pReal;            /* Underlying file handle */
  sqlite3_vfs *pVfslog;           /* Associated VsflogVfs object */
  പൂർണ്ണസംഖ്യ iFileId;                    /* File id number */
};

#define REALVFS(p) (((VfslogVfs *)(p))->pVfs)



/*
** Method declarations for vfslog_file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ vfslogClose(sqlite3_file*);
സ്ഥിതം പൂർണ്ണസംഖ്യ vfslogRead(sqlite3_file*, ശൂന്യം*, പൂർണ്ണസംഖ്യ iAmt, sqlite3_int64 iOfst);
സ്ഥിതം പൂർണ്ണസംഖ്യ vfslogWrite(sqlite3_file*,സ്ഥിരം ശൂന്യം*,പൂർണ്ണസംഖ്യ iAmt, sqlite3_int64 iOfst);
സ്ഥിതം പൂർണ്ണസംഖ്യ vfslogTruncate(sqlite3_file*, sqlite3_int64 size);
സ്ഥിതം പൂർണ്ണസംഖ്യ vfslogSync(sqlite3_file*, പൂർണ്ണസംഖ്യ flags);
സ്ഥിതം പൂർണ്ണസംഖ്യ vfslogFileSize(sqlite3_file*, sqlite3_int64 *pSize);
സ്ഥിതം പൂർണ്ണസംഖ്യ vfslogLock(sqlite3_file*, പൂർണ്ണസംഖ്യ);
സ്ഥിതം പൂർണ്ണസംഖ്യ vfslogUnlock(sqlite3_file*, പൂർണ്ണസംഖ്യ);
സ്ഥിതം പൂർണ്ണസംഖ്യ vfslogCheckReservedLock(sqlite3_file*, പൂർണ്ണസംഖ്യ *pResOut);
സ്ഥിതം പൂർണ്ണസംഖ്യ vfslogFileControl(sqlite3_file*, പൂർണ്ണസംഖ്യ op, ശൂന്യം *pArg);
സ്ഥിതം പൂർണ്ണസംഖ്യ vfslogSectorSize(sqlite3_file*);
സ്ഥിതം പൂർണ്ണസംഖ്യ vfslogDeviceCharacteristics(sqlite3_file*);

സ്ഥിതം പൂർണ്ണസംഖ്യ vfslogShmLock(sqlite3_file *pFile, പൂർണ്ണസംഖ്യ ofst, പൂർണ്ണസംഖ്യ n, പൂർണ്ണസംഖ്യ flags);
സ്ഥിതം പൂർണ്ണസംഖ്യ vfslogShmMap(sqlite3_file *pFile,പൂർണ്ണസംഖ്യ,പൂർണ്ണസംഖ്യ,പൂർണ്ണസംഖ്യ,അസ്ഥിര ശൂന്യം **);
സ്ഥിതം ശൂന്യം vfslogShmBarrier(sqlite3_file*);
സ്ഥിതം പൂർണ്ണസംഖ്യ vfslogShmUnmap(sqlite3_file *pFile, പൂർണ്ണസംഖ്യ deleteFlag);

/*
** Method declarations for vfslog_vfs.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ vfslogOpen(sqlite3_vfs*, സ്ഥിരം അക്ഷരം *, sqlite3_file*, പൂർണ്ണസംഖ്യ , പൂർണ്ണസംഖ്യ *);
സ്ഥിതം പൂർണ്ണസംഖ്യ vfslogDelete(sqlite3_vfs*, സ്ഥിരം അക്ഷരം *zName, പൂർണ്ണസംഖ്യ syncDir);
സ്ഥിതം പൂർണ്ണസംഖ്യ vfslogAccess(sqlite3_vfs*, സ്ഥിരം അക്ഷരം *zName, പൂർണ്ണസംഖ്യ flags, പൂർണ്ണസംഖ്യ *);
സ്ഥിതം പൂർണ്ണസംഖ്യ vfslogFullPathname(sqlite3_vfs*, സ്ഥിരം അക്ഷരം *zName, പൂർണ്ണസംഖ്യ, അക്ഷരം *zOut);
സ്ഥിതം ശൂന്യം *vfslogDlOpen(sqlite3_vfs*, സ്ഥിരം അക്ഷരം *zFilename);
സ്ഥിതം ശൂന്യം vfslogDlError(sqlite3_vfs*, പൂർണ്ണസംഖ്യ nByte, അക്ഷരം *z_Err_സന്ദേശം);
സ്ഥിതം ശൂന്യം (*vfslogDlSym(sqlite3_vfs *pVfs, ശൂന്യം *p, സ്ഥിരം അക്ഷരം*zSym))(ശൂന്യം);
സ്ഥിതം ശൂന്യം vfslogDlClose(sqlite3_vfs*, ശൂന്യം*);
സ്ഥിതം പൂർണ്ണസംഖ്യ vfslogRandomness(sqlite3_vfs*, പൂർണ്ണസംഖ്യ nByte, അക്ഷരം *zOut);
സ്ഥിതം പൂർണ്ണസംഖ്യ vfslogSleep(sqlite3_vfs*, പൂർണ്ണസംഖ്യ microseconds);
സ്ഥിതം പൂർണ്ണസംഖ്യ vfslogCurrentTime(sqlite3_vfs*, ഇരട്ട*);

സ്ഥിതം പൂർണ്ണസംഖ്യ vfslogGetLastError(sqlite3_vfs*, പൂർണ്ണസംഖ്യ, അക്ഷരം *);
സ്ഥിതം പൂർണ്ണസംഖ്യ vfslogCurrentTimeInt64(sqlite3_vfs*, sqlite3_int64*);

സ്ഥിതം sqlite3_vfs vfslog_vfs = {
  1,                              /* iVersion */
  വലുപ്പം(VfslogFile),             /* szOsFile */
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

സ്ഥിതം sqlite3_io_methods vfslog_io_methods = {
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
#അന്തര്ഭാവ <time.h>
സ്ഥിതം sqlite3_uint64 vfslog_time(){
  FILETIME ft;
  sqlite3_uint64 u64time = 0;
 
  GetSystemTimeAsFileTime(&ft);

  u64time |= ft.dw_മുകളിലെ_Date_Time;
  u64time <<= 32;
  u64time |= ft.dw_താഴെയുള്ള_Date_Time;

  /* ft is 100-nanosecond intervals, we want microseconds */
  തിരികെ u64time /(sqlite3_uint64)10;
}
#അല്ലെങ്കിലെങ്കിൽ !defined(NO_GETTOD)
#അന്തര്ഭാവ <sys/time.h>
സ്ഥിതം sqlite3_uint64 vfslog_time(){
  ഘടന timeval sTime;
  gettimeofday(&sTime, 0);
  തിരികെ sTime.tv_usec + (sqlite3_uint64)sTime.tv_sec * 1000000;
}
#അല്ലെങ്കിൽ
സ്ഥിതം sqlite3_uint64 vfslog_time(){
  തിരികെ 0;
}
#endif

സ്ഥിതം ശൂന്യം vfslog_call(sqlite3_vfs *, പൂർണ്ണസംഖ്യ, പൂർണ്ണസംഖ്യ, sqlite3_int64, പൂർണ്ണസംഖ്യ, പൂർണ്ണസംഖ്യ, പൂർണ്ണസംഖ്യ);
സ്ഥിതം ശൂന്യം vfslog_string(sqlite3_vfs *, സ്ഥിരം അക്ഷരം *);

/*
** Close an vfslog-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ vfslogClose(sqlite3_file *pFile){
  sqlite3_uint64 t;
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;
  VfslogFile *p = (VfslogFile *)pFile;

  t = vfslog_time();
  എങ്കിൽ( p->pReal->pMethods ){
    rc = p->pReal->pMethods->xClose(p->pReal);
  }
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_CLOSE, p->iFileId, t, rc, 0, 0);
  തിരികെ rc;
}

/*
** Read data from an vfslog-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ vfslogRead(
  sqlite3_file *pFile, 
  ശൂന്യം *zBuf, 
  പൂർണ്ണസംഖ്യ iAmt, 
  sqlite_int64 iOfst
){
  പൂർണ്ണസംഖ്യ rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xRead(p->pReal, zBuf, iAmt, iOfst);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_READ, p->iFileId, t, rc, iAmt, (പൂർണ്ണസംഖ്യ)iOfst);
  തിരികെ rc;
}

/*
** Write data to an vfslog-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ vfslogWrite(
  sqlite3_file *pFile,
  സ്ഥിരം ശൂന്യം *z,
  പൂർണ്ണസംഖ്യ iAmt,
  sqlite_int64 iOfst
){
  പൂർണ്ണസംഖ്യ rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xWrite(p->pReal, z, iAmt, iOfst);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_WRITE, p->iFileId, t, rc, iAmt, (പൂർണ്ണസംഖ്യ)iOfst);
  തിരികെ rc;
}

/*
** Truncate an vfslog-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ vfslogTruncate(sqlite3_file *pFile, sqlite_int64 size){
  പൂർണ്ണസംഖ്യ rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xTruncate(p->pReal, size);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_TRUNCATE, p->iFileId, t, rc, 0, (പൂർണ്ണസംഖ്യ)size);
  തിരികെ rc;
}

/*
** Sync an vfslog-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ vfslogSync(sqlite3_file *pFile, പൂർണ്ണസംഖ്യ flags){
  പൂർണ്ണസംഖ്യ rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xSync(p->pReal, flags);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SYNC, p->iFileId, t, rc, flags, 0);
  തിരികെ rc;
}

/*
** Return the current file-size of an vfslog-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ vfslogFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  പൂർണ്ണസംഖ്യ rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xFileSize(p->pReal, pSize);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_FILESIZE, p->iFileId, t, rc, 0, (പൂർണ്ണസംഖ്യ)*pSize);
  തിരികെ rc;
}

/*
** Lock an vfslog-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ vfslogLock(sqlite3_file *pFile, പൂർണ്ണസംഖ്യ eLock){
  പൂർണ്ണസംഖ്യ rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xLock(p->pReal, eLock);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_LOCK, p->iFileId, t, rc, eLock, 0);
  തിരികെ rc;
}

/*
** Unlock an vfslog-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ vfslogUnlock(sqlite3_file *pFile, പൂർണ്ണസംഖ്യ eLock){
  പൂർണ്ണസംഖ്യ rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xUnlock(p->pReal, eLock);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_UNLOCK, p->iFileId, t, rc, eLock, 0);
  തിരികെ rc;
}

/*
** Check if another file-handle holds a RESERVED lock on an vfslog-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ vfslogCheckReservedLock(sqlite3_file *pFile, പൂർണ്ണസംഖ്യ *pResOut){
  പൂർണ്ണസംഖ്യ rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xCheckReservedLock(p->pReal, pResOut);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_CHECKRESERVEDLOCK, p->iFileId, t, rc, *pResOut, 0);
  തിരികെ rc;
}

/*
** File control method. For custom operations on an vfslog-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ vfslogFileControl(sqlite3_file *pFile, പൂർണ്ണസംഖ്യ op, ശൂന്യം *pArg){
  VfslogFile *p = (VfslogFile *)pFile;
  പൂർണ്ണസംഖ്യ rc = p->pReal->pMethods->xFileControl(p->pReal, op, pArg);
  എങ്കിൽ( op==SQLITE_FCNTL_VFSNAME && rc==SQLITE_OK ){
    *(അക്ഷരം**)pArg = sqlite3_mprintf("vfslog/%z", *(അക്ഷരം**)pArg);
  }
  തിരികെ rc;
}

/*
** Return the sector-size in bytes for an vfslog-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ vfslogSectorSize(sqlite3_file *pFile){
  പൂർണ്ണസംഖ്യ rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xSectorSize(p->pReal);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SECTORSIZE, p->iFileId, t, rc, 0, 0);
  തിരികെ rc;
}

/*
** Return the device characteristic flags supported by an vfslog-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ vfslogDeviceCharacteristics(sqlite3_file *pFile){
  പൂർണ്ണസംഖ്യ rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xDeviceCharacteristics(p->pReal);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_DEVCHAR, p->iFileId, t, rc, 0, 0);
  തിരികെ rc;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ vfslogShmLock(sqlite3_file *pFile, പൂർണ്ണസംഖ്യ ofst, പൂർണ്ണസംഖ്യ n, പൂർണ്ണസംഖ്യ flags){
  പൂർണ്ണസംഖ്യ rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xShmLock(p->pReal, ofst, n, flags);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SHMLOCK, p->iFileId, t, rc, 0, 0);
  തിരികെ rc;
}
സ്ഥിതം പൂർണ്ണസംഖ്യ vfslogShmMap(
  sqlite3_file *pFile, 
  പൂർണ്ണസംഖ്യ iRegion, 
  പൂർണ്ണസംഖ്യ szRegion, 
  പൂർണ്ണസംഖ്യ isWrite, 
  അസ്ഥിര ശൂന്യം **pp
){
  പൂർണ്ണസംഖ്യ rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xShmMap(p->pReal, iRegion, szRegion, isWrite, pp);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SHMMAP, p->iFileId, t, rc, 0, 0);
  തിരികെ rc;
}
സ്ഥിതം ശൂന്യം vfslogShmBarrier(sqlite3_file *pFile){
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  p->pReal->pMethods->xShmBarrier(p->pReal);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SHMBARRIER, p->iFileId, t, SQLITE_OK, 0, 0);
}
സ്ഥിതം പൂർണ്ണസംഖ്യ vfslogShmUnmap(sqlite3_file *pFile, പൂർണ്ണസംഖ്യ deleteFlag){
  പൂർണ്ണസംഖ്യ rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xShmUnmap(p->pReal, deleteFlag);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SHMUNMAP, p->iFileId, t, rc, 0, 0);
  തിരികെ rc;
}


/*
** Open an vfslog file handle.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ vfslogOpen(
  sqlite3_vfs *pVfs,
  സ്ഥിരം അക്ഷരം *zName,
  sqlite3_file *pFile,
  പൂർണ്ണസംഖ്യ flags,
  പൂർണ്ണസംഖ്യ *pOutFlags
){
  പൂർണ്ണസംഖ്യ rc;
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
  തിരികെ rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ vfslogDelete(sqlite3_vfs *pVfs, സ്ഥിരം അക്ഷരം *zPath, പൂർണ്ണസംഖ്യ dirSync){
  പൂർണ്ണസംഖ്യ rc;
  sqlite3_uint64 t;
  t = vfslog_time();
  rc = REALVFS(pVfs)->xDelete(REALVFS(pVfs), zPath, dirSync);
  t = vfslog_time() - t;
  vfslog_call(pVfs, OS_DELETE, 0, t, rc, dirSync, 0);
  vfslog_string(pVfs, zPath);
  തിരികെ rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ vfslogAccess(
  sqlite3_vfs *pVfs, 
  സ്ഥിരം അക്ഷരം *zPath, 
  പൂർണ്ണസംഖ്യ flags, 
  പൂർണ്ണസംഖ്യ *pResOut
){
  പൂർണ്ണസംഖ്യ rc;
  sqlite3_uint64 t;
  t = vfslog_time();
  rc = REALVFS(pVfs)->xAccess(REALVFS(pVfs), zPath, flags, pResOut);
  t = vfslog_time() - t;
  vfslog_call(pVfs, OS_ACCESS, 0, t, rc, flags, *pResOut);
  vfslog_string(pVfs, zPath);
  തിരികെ rc;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ vfslogFullPathname(
  sqlite3_vfs *pVfs, 
  സ്ഥിരം അക്ഷരം *zPath, 
  പൂർണ്ണസംഖ്യ nOut, 
  അക്ഷരം *zOut
){
  തിരികെ REALVFS(pVfs)->xFullPathname(REALVFS(pVfs), zPath, nOut, zOut);
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
സ്ഥിതം ശൂന്യം *vfslogDlOpen(sqlite3_vfs *pVfs, സ്ഥിരം അക്ഷരം *zPath){
  തിരികെ REALVFS(pVfs)->xDlOpen(REALVFS(pVfs), zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
സ്ഥിതം ശൂന്യം vfslogDlError(sqlite3_vfs *pVfs, പൂർണ്ണസംഖ്യ nByte, അക്ഷരം *z_Err_സന്ദേശം){
  REALVFS(pVfs)->xDlError(REALVFS(pVfs), nByte, z_Err_സന്ദേശം);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
സ്ഥിതം ശൂന്യം (*vfslogDlSym(sqlite3_vfs *pVfs, ശൂന്യം *p, സ്ഥിരം അക്ഷരം *zSym))(ശൂന്യം){
  തിരികെ REALVFS(pVfs)->xDlSym(REALVFS(pVfs), p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
സ്ഥിതം ശൂന്യം vfslogDlClose(sqlite3_vfs *pVfs, ശൂന്യം *pHandle){
  REALVFS(pVfs)->xDlClose(REALVFS(pVfs), pHandle);
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ vfslogRandomness(sqlite3_vfs *pVfs, പൂർണ്ണസംഖ്യ nByte, അക്ഷരം *zBufOut){
  തിരികെ REALVFS(pVfs)->xRandomness(REALVFS(pVfs), nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ vfslogSleep(sqlite3_vfs *pVfs, പൂർണ്ണസംഖ്യ nMicro){
  തിരികെ REALVFS(pVfs)->xSleep(REALVFS(pVfs), nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ vfslogCurrentTime(sqlite3_vfs *pVfs, ഇരട്ട *pTimeOut){
  തിരികെ REALVFS(pVfs)->xCurrentTime(REALVFS(pVfs), pTimeOut);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ vfslogGetLastError(sqlite3_vfs *pVfs, പൂർണ്ണസംഖ്യ a, അക്ഷരം *b){
  തിരികെ REALVFS(pVfs)->xGetLastError(REALVFS(pVfs), a, b);
}
സ്ഥിതം പൂർണ്ണസംഖ്യ vfslogCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *p){
  തിരികെ REALVFS(pVfs)->xCurrentTimeInt64(REALVFS(pVfs), p);
}

സ്ഥിതം ശൂന്യം vfslog_flush(VfslogVfs *p){
#ifdef SQLITE_TEST
  ബാഹ്യ പൂർണ്ണസംഖ്യ sqlite3_io_error_pending;
  ബാഹ്യ പൂർണ്ണസംഖ്യ sqlite3_io_error_persist;
  ബാഹ്യ പൂർണ്ണസംഖ്യ sqlite3_diskfull_pending;

  പൂർണ്ണസംഖ്യ pending = sqlite3_io_error_pending;
  പൂർണ്ണസംഖ്യ persist = sqlite3_io_error_persist;
  പൂർണ്ണസംഖ്യ diskfull = sqlite3_diskfull_pending;

  sqlite3_io_error_pending = 0;
  sqlite3_io_error_persist = 0;
  sqlite3_diskfull_pending = 0;
#endif

  എങ്കിൽ( p->nBuf ){
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

സ്ഥിതം ശൂന്യം put32bits(unsigned അക്ഷരം *p, unsigned പൂർണ്ണസംഖ്യ v){
  p[0] = v>>24;
  p[1] = (unsigned അക്ഷരം)(v>>16);
  p[2] = (unsigned അക്ഷരം)(v>>8);
  p[3] = (unsigned അക്ഷരം)v;
}

സ്ഥിതം ശൂന്യം vfslog_call(
  sqlite3_vfs *pVfs,
  പൂർണ്ണസംഖ്യ eEvent,
  പൂർണ്ണസംഖ്യ iFileid,
  sqlite3_int64 nClick,
  പൂർണ്ണസംഖ്യ return_code,
  പൂർണ്ണസംഖ്യ size,
  പൂർണ്ണസംഖ്യ offset
){
  VfslogVfs *p = (VfslogVfs *)pVfs;
  unsigned അക്ഷരം *zRec;
  എങ്കിൽ( (24+p->nBuf)>വലുപ്പം(p->aBuf) ){
    vfslog_flush(p);
  }
  zRec = (unsigned അക്ഷരം *)&p->aBuf[p->nBuf];
  put32bits(&zRec[0], eEvent);
  put32bits(&zRec[4], iFileid);
  put32bits(&zRec[8], (unsigned പൂർണ്ണസംഖ്യ)(nClick&0xffff));
  put32bits(&zRec[12], return_code);
  put32bits(&zRec[16], size);
  put32bits(&zRec[20], offset);
  p->nBuf += 24;
}

സ്ഥിതം ശൂന്യം vfslog_string(sqlite3_vfs *pVfs, സ്ഥിരം അക്ഷരം *zStr){
  VfslogVfs *p = (VfslogVfs *)pVfs;
  unsigned അക്ഷരം *zRec;
  പൂർണ്ണസംഖ്യ nStr = zStr ? (പൂർണ്ണസംഖ്യ)strlen(zStr) : 0;
  എങ്കിൽ( (4+nStr+p->nBuf)>വലുപ്പം(p->aBuf) ){
    vfslog_flush(p);
  }
  zRec = (unsigned അക്ഷരം *)&p->aBuf[p->nBuf];
  put32bits(&zRec[0], nStr);
  എങ്കിൽ( zStr ){
    memcpy(&zRec[4], zStr, nStr);
  }
  p->nBuf += (4 + nStr);
}

സ്ഥിതം ശൂന്യം vfslog_finalize(VfslogVfs *p){
  എങ്കിൽ( p->pLog->pMethods ){
    vfslog_flush(p);
    p->pLog->pMethods->xClose(p->pLog);
  }
  sqlite3_free(p);
}

പൂർണ്ണസംഖ്യ sqlite3_vfslog_finalize(സ്ഥിരം അക്ഷരം *zVfs){
  sqlite3_vfs *pVfs;
  pVfs = sqlite3_vfs_find(zVfs);
  എങ്കിൽ( !pVfs || pVfs->xOpen!=vfslogOpen ){
    തിരികെ SQLITE_ERROR;
  } 
  sqlite3_vfs_unregister(pVfs);
  vfslog_finalize((VfslogVfs *)pVfs);
  തിരികെ SQLITE_OK;
}

പൂർണ്ണസംഖ്യ sqlite3_vfslog_new(
  സ്ഥിരം അക്ഷരം *zVfs,               /* New VFS name */
  സ്ഥിരം അക്ഷരം *zParentVfs,         /* Parent VFS name (or NULL) */
  സ്ഥിരം അക്ഷരം *zLog                /* Log file name */
){
  VfslogVfs *p;
  sqlite3_vfs *pParent;
  പൂർണ്ണസംഖ്യ nByte;
  പൂർണ്ണസംഖ്യ flags;
  പൂർണ്ണസംഖ്യ rc;
  അക്ഷരം *zFile;
  പൂർണ്ണസംഖ്യ nVfs;

  pParent = sqlite3_vfs_find(zParentVfs);
  എങ്കിൽ( !pParent ){
    തിരികെ SQLITE_ERROR;
  }

  nVfs = (പൂർണ്ണസംഖ്യ)strlen(zVfs);
  nByte = വലുപ്പം(VfslogVfs) + pParent->szOsFile + nVfs+1+pParent->mxPathname+1;
  p = (VfslogVfs *)sqlite3_malloc(nByte);
  memset(p, 0, nByte);

  p->pVfs = pParent;
  p->pLog = (sqlite3_file *)&p[1];
  memcpy(&p->base, &vfslog_vfs, വലുപ്പം(sqlite3_vfs));
  p->base.zName = &((അക്ഷരം *)p->pLog)[pParent->szOsFile];
  p->base.szOsFile += pParent->szOsFile;
  memcpy((അക്ഷരം *)p->base.zName, zVfs, nVfs);

  zFile = (അക്ഷരം *)&p->base.zName[nVfs+1];
  pParent->xFullPathname(pParent, zLog, pParent->mxPathname, zFile);

  flags = SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE|SQLITE_OPEN_SUPER_JOURNAL;
  pParent->xDelete(pParent, zFile, 0);
  rc = pParent->xOpen(pParent, zFile, p->pLog, flags, &flags);
  എങ്കിൽ( rc==SQLITE_OK ){
    memcpy(p->aBuf, "sqlite_ostrace1.....", 20);
    p->iOffset = 0;
    p->nBuf = 20;
    rc = sqlite3_vfs_register((sqlite3_vfs *)p, 1);
  }
  എങ്കിൽ( rc ){
    vfslog_finalize(p);
  }
  തിരികെ rc;
}

പൂർണ്ണസംഖ്യ sqlite3_vfslog_annotate(സ്ഥിരം അക്ഷരം *zVfs, സ്ഥിരം അക്ഷരം *z_സന്ദേശം){
  sqlite3_vfs *pVfs;
  pVfs = sqlite3_vfs_find(zVfs);
  എങ്കിൽ( !pVfs || pVfs->xOpen!=vfslogOpen ){
    തിരികെ SQLITE_ERROR;
  } 
  vfslog_call(pVfs, OS_ANNOTATE, 0, 0, 0, 0, 0);
  vfslog_string(pVfs, z_സന്ദേശം);
  തിരികെ SQLITE_OK;
}

സ്ഥിതം സ്ഥിരം അക്ഷരം *vfslog_eventname(പൂർണ്ണസംഖ്യ eEvent){
  സ്ഥിരം അക്ഷരം *zEvent = 0;

  തിരഞ്ഞെടുപ്പ്( eEvent ){
    അവസ്ഥ OS_CLOSE:             zEvent = "xClose"; നിർത്തുക;
    അവസ്ഥ OS_READ:              zEvent = "xRead"; നിർത്തുക;
    അവസ്ഥ OS_WRITE:             zEvent = "xWrite"; നിർത്തുക;
    അവസ്ഥ OS_TRUNCATE:          zEvent = "xTruncate"; നിർത്തുക;
    അവസ്ഥ OS_SYNC:              zEvent = "xSync"; നിർത്തുക;
    അവസ്ഥ OS_FILESIZE:          zEvent = "xFilesize"; നിർത്തുക;
    അവസ്ഥ OS_LOCK:              zEvent = "xLock"; നിർത്തുക;
    അവസ്ഥ OS_UNLOCK:            zEvent = "xUnlock"; നിർത്തുക;
    അവസ്ഥ OS_CHECKRESERVEDLOCK: zEvent = "xCheckResLock"; നിർത്തുക;
    അവസ്ഥ OS_FILECONTROL:       zEvent = "xFileControl"; നിർത്തുക;
    അവസ്ഥ OS_SECTORSIZE:        zEvent = "xSectorSize"; നിർത്തുക;
    അവസ്ഥ OS_DEVCHAR:           zEvent = "xDeviceChar"; നിർത്തുക;
    അവസ്ഥ OS_OPEN:              zEvent = "xOpen"; നിർത്തുക;
    അവസ്ഥ OS_DELETE:            zEvent = "xDelete"; നിർത്തുക;
    അവസ്ഥ OS_ACCESS:            zEvent = "xAccess"; നിർത്തുക;
    അവസ്ഥ OS_FULLPATHNAME:      zEvent = "xFullPathname"; നിർത്തുക;
    അവസ്ഥ OS_RANDOMNESS:        zEvent = "xRandomness"; നിർത്തുക;
    അവസ്ഥ OS_SLEEP:             zEvent = "xSleep"; നിർത്തുക;
    അവസ്ഥ OS_CURRENTTIME:       zEvent = "xCurrentTime"; നിർത്തുക;

    അവസ്ഥ OS_SHMUNMAP:          zEvent = "xShmUnmap"; നിർത്തുക;
    അവസ്ഥ OS_SHMLOCK:           zEvent = "xShmLock"; നിർത്തുക;
    അവസ്ഥ OS_SHMBARRIER:        zEvent = "xShmBarrier"; നിർത്തുക;
    അവസ്ഥ OS_SHMMAP:            zEvent = "xShmMap"; നിർത്തുക;

    അവസ്ഥ OS_ANNOTATE:          zEvent = "annotation"; നിർത്തുക;
  }

  തിരികെ zEvent;
}

തരനിർവചനം ഘടന VfslogVtab VfslogVtab;
തരനിർവചനം ഘടന VfslogCsr VfslogCsr;

/*
** Virtual table type for the vfslog reader module.
*/
ഘടന VfslogVtab {
  sqlite3_vtab base;              /* Base class */
  sqlite3_file *pFd;              /* File descriptor open on vfslog file */
  sqlite3_int64 nByte;            /* Size of file in bytes */
  അക്ഷരം *zFile;                    /* File name for pFd */
};

/*
** Virtual table cursor type for the vfslog reader module.
*/
ഘടന VfslogCsr {
  sqlite3_vtab_cursor base;       /* Base class */
  sqlite3_int64 iRowid;           /* Current rowid. */
  sqlite3_int64 iOffset;          /* Offset of next record in file */
  അക്ഷരം *zTransient;               /* Transient 'file' string */
  പൂർണ്ണസംഖ്യ nFile;                      /* Size of array azFile[] */
  അക്ഷരം **azFile;                  /* File strings */
  unsigned അക്ഷരം aBuf[1024];       /* Current vfs log entry (read from file) */
};

സ്ഥിതം unsigned പൂർണ്ണസംഖ്യ get32bits(unsigned അക്ഷരം *p){
  തിരികെ (p[0]<<24) + (p[1]<<16) + (p[2]<<8) + p[3];
}

/*
** The argument must point to a buffer containing a nul-terminated string.
** If the string begins with an SQL quote character it is overwritten by
** the dequoted version. Otherwise the buffer is left unmodified.
*/
സ്ഥിതം ശൂന്യം dequote(അക്ഷരം *z){
  അക്ഷരം quote;                     /* Quote character (if any ) */
  quote = z[0];
  എങ്കിൽ( quote=='[' || quote=='\'' || quote=='"' || quote=='`' ){
    പൂർണ്ണസംഖ്യ iIn = 1;                  /* Index of next byte to read from input */
    പൂർണ്ണസംഖ്യ iOut = 0;                 /* Index of next byte to write to output */
    എങ്കിൽ( quote=='[' ) quote = ']';  
    ആകുമ്പോൾ( z[iIn] ){
      എങ്കിൽ( z[iIn]==quote ){
        എങ്കിൽ( z[iIn+1]!=quote ) നിർത്തുക;
        z[iOut++] = quote;
        iIn += 2;
      }അല്ലെങ്കിൽ{
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
സ്ഥിതം പൂർണ്ണസംഖ്യ vlogConnect(
  sqlite3 *db,
  ശൂന്യം *pAux,
  പൂർണ്ണസംഖ്യ argc, സ്ഥിരം അക്ഷരം *സ്ഥിരം*argv,
  sqlite3_vtab **ppVtab,
  അക്ഷരം **pzErr
){
  sqlite3_vfs *pVfs;              /* VFS used to read log file */
  പൂർണ്ണസംഖ്യ flags;                      /* flags passed to pVfs->xOpen() */
  VfslogVtab *p;
  പൂർണ്ണസംഖ്യ rc;
  പൂർണ്ണസംഖ്യ nByte;
  അക്ഷരം *zFile;

  *ppVtab = 0;
  pVfs = sqlite3_vfs_find(0);
  nByte = വലുപ്പം(VfslogVtab) + pVfs->szOsFile + pVfs->mxPathname;
  p = sqlite3_malloc(nByte);
  എങ്കിൽ( p==0 ) തിരികെ SQLITE_NOMEM;
  memset(p, 0, nByte);

  p->pFd = (sqlite3_file *)&p[1];
  p->zFile = &((അക്ഷരം *)p->pFd)[pVfs->szOsFile];

  zFile = sqlite3_mprintf("%s", argv[3]);
  എങ്കിൽ( !zFile ){
    sqlite3_free(p);
    തിരികെ SQLITE_NOMEM;
  }
  dequote(zFile);
  pVfs->xFullPathname(pVfs, zFile, pVfs->mxPathname, p->zFile);
  sqlite3_free(zFile);

  flags = SQLITE_OPEN_READWRITE|SQLITE_OPEN_SUPER_JOURNAL;
  rc = pVfs->xOpen(pVfs, p->zFile, p->pFd, flags, &flags);

  എങ്കിൽ( rc==SQLITE_OK ){
    p->pFd->pMethods->xFileSize(p->pFd, &p->nByte);
    sqlite3_declare_vtab(db, 
        "CREATE TABLE xxx(event, file, click, rc, size, offset)"
    );
    *ppVtab = &p->base;
  }അല്ലെങ്കിൽ{
    sqlite3_free(p);
  }

  തിരികെ rc;
}

/*
** There is no "best-index". This virtual table always does a linear
** scan of the binary VFS log file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ vlogBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  pIdxInfo->estimatedCost = 10.0;
  തിരികെ SQLITE_OK;
}

/*
** Disconnect from or destroy a vfslog virtual table.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ vlogDisconnect(sqlite3_vtab *pVtab){
  VfslogVtab *p = (VfslogVtab *)pVtab;
  എങ്കിൽ( p->pFd->pMethods ){
    p->pFd->pMethods->xClose(p->pFd);
    p->pFd->pMethods = 0;
  }
  sqlite3_free(p);
  തിരികെ SQLITE_OK;
}

/*
** Open a new vfslog cursor.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ vlogOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  VfslogCsr *pCsr;                /* Newly allocated cursor object */

  pCsr = sqlite3_malloc(വലുപ്പം(VfslogCsr));
  എങ്കിൽ( !pCsr ) തിരികെ SQLITE_NOMEM;
  memset(pCsr, 0, വലുപ്പം(VfslogCsr));
  *ppCursor = &pCsr->base;
  തിരികെ SQLITE_OK;
}

/*
** Close a vfslog cursor.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ vlogClose(sqlite3_vtab_cursor *pCursor){
  VfslogCsr *p = (VfslogCsr *)pCursor;
  പൂർണ്ണസംഖ്യ i;
  വേണ്ടി(i=0; i<p->nFile; i++){
    sqlite3_free(p->azFile[i]);
  }
  sqlite3_free(p->azFile);
  sqlite3_free(p->zTransient);
  sqlite3_free(p);
  തിരികെ SQLITE_OK;
}

/*
** Move a vfslog cursor to the next entry in the file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ vlogNext(sqlite3_vtab_cursor *pCursor){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  VfslogVtab *p = (VfslogVtab *)pCursor->pVtab;
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;
  പൂർണ്ണസംഖ്യ nRead;

  sqlite3_free(pCsr->zTransient);
  pCsr->zTransient = 0;

  nRead = 24;
  എങ്കിൽ( pCsr->iOffset+nRead<=p->nByte ){
    പൂർണ്ണസംഖ്യ eEvent;
    rc = p->pFd->pMethods->xRead(p->pFd, pCsr->aBuf, nRead, pCsr->iOffset);

    eEvent = get32bits(pCsr->aBuf);
    എങ്കിൽ( (rc==SQLITE_OK)
     && (eEvent==OS_OPEN || eEvent==OS_DELETE || eEvent==OS_ACCESS) 
    ){
      അക്ഷരം buf[4];
      rc = p->pFd->pMethods->xRead(p->pFd, buf, 4, pCsr->iOffset+nRead);
      nRead += 4;
      എങ്കിൽ( rc==SQLITE_OK ){
        പൂർണ്ണസംഖ്യ nStr = get32bits((unsigned അക്ഷരം *)buf);
        അക്ഷരം *zStr = sqlite3_malloc(nStr+1);
        rc = p->pFd->pMethods->xRead(p->pFd, zStr, nStr, pCsr->iOffset+nRead);
        zStr[nStr] = '\0';
        nRead += nStr;

        എങ്കിൽ( eEvent==OS_OPEN ){
          പൂർണ്ണസംഖ്യ iFileid = get32bits(&pCsr->aBuf[4]);
          എങ്കിൽ( iFileid>=pCsr->nFile ){
            പൂർണ്ണസംഖ്യ nNew = വലുപ്പം(pCsr->azFile[0])*(iFileid+1);
            pCsr->azFile = (അക്ഷരം **)sqlite3_realloc(pCsr->azFile, nNew);
            nNew -= വലുപ്പം(pCsr->azFile[0])*pCsr->nFile;
            memset(&pCsr->azFile[pCsr->nFile], 0, nNew);
            pCsr->nFile = iFileid+1;
          }
          sqlite3_free(pCsr->azFile[iFileid]);
          pCsr->azFile[iFileid] = zStr;
        }അല്ലെങ്കിൽ{
          pCsr->zTransient = zStr;
        }
      }
    }
  }

  pCsr->iRowid += 1;
  pCsr->iOffset += nRead;
  തിരികെ rc;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ vlogEof(sqlite3_vtab_cursor *pCursor){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  VfslogVtab *p = (VfslogVtab *)pCursor->pVtab;
  തിരികെ (pCsr->iOffset>=p->nByte);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ vlogFilter(
  sqlite3_vtab_cursor *pCursor, 
  പൂർണ്ണസംഖ്യ idx_അക്കം, സ്ഥിരം അക്ഷരം *idxStr,
  പൂർണ്ണസംഖ്യ argc, sqlite3_മൂല്യം **argv
){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  pCsr->iRowid = 0;
  pCsr->iOffset = 20;
  തിരികെ vlogNext(pCursor);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ vlogColumn(
  sqlite3_vtab_cursor *pCursor, 
  sqlite3_context *ctx, 
  പൂർണ്ണസംഖ്യ i
){
  unsigned പൂർണ്ണസംഖ്യ val;
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;

  പ്രതിജ്ഞാ( i<7 );
  val = get32bits(&pCsr->aBuf[4*i]);

  തിരഞ്ഞെടുപ്പ്( i ){
    അവസ്ഥ 0: {
      sqlite3_result_text(ctx, vfslog_eventname(val), -1, SQLITE_STATIC);
      നിർത്തുക;
    }
    അവസ്ഥ 1: {
      അക്ഷരം *zStr = pCsr->zTransient;
      എങ്കിൽ( val!=0 && val<(unsigned)pCsr->nFile ){
        zStr = pCsr->azFile[val];
      }
      sqlite3_result_text(ctx, zStr, -1, SQLITE_TRANSIENT);
      നിർത്തുക;
    }
    സ്ഥിരസ്ഥിതി:
      sqlite3_result_int(ctx, val);
      നിർത്തുക;
  }

  തിരികെ SQLITE_OK;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ vlogRowid(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  *pRowid = pCsr->iRowid;
  തിരികെ SQLITE_OK;
}

പൂർണ്ണസംഖ്യ sqlite3_vfslog_register(sqlite3 *db){
  സ്ഥിതം sqlite3_module vfslog_module = {
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
  തിരികെ SQLITE_OK;
}
#endif /* SQLITE_OMIT_VIRTUALTABLE */

/**************************************************************************
***************************************************************************
** Tcl interface starts here.
*/

#എങ്കിൽ defined(SQLITE_TEST) || defined(TCLSH)

#അന്തര്ഭാവ "tclsqlite.h"

സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_vfslog(
  ശൂന്യം *clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  ഘടന SqliteDb { sqlite3 *db; };
  sqlite3 *db;
  Tcl_CmdInfo cmdInfo;
  പൂർണ്ണസംഖ്യ rc = SQLITE_ERROR;

  സ്ഥിതം സ്ഥിരം അക്ഷരം *strs[] = { "annotate", "finalize", "new",  "register", 0 };
  എണ്ണൽ VL_enum { VL_ANNOTATE, VL_FINALIZE, VL_NEW, VL_REGISTER };
  പൂർണ്ണസംഖ്യ iSub;

  എങ്കിൽ( objc<2 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "SUB-COMMAND ...");
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( Tcl_GetIndexFromObj(interp, objv[1], strs, "sub-command", 0, &iSub) ){
    തിരികെ TCL_ERROR;
  }

  തിരഞ്ഞെടുപ്പ്( (എണ്ണൽ VL_enum)iSub ){
    അവസ്ഥ VL_ANNOTATE: {
      അക്ഷരം *zVfs;
      അക്ഷരം *z_സന്ദേശം;
      എങ്കിൽ( objc!=4 ){
        Tcl_Wrong_അക്കം_Args(interp, 3, objv, "VFS");
        തിരികെ TCL_ERROR;
      }
      zVfs = Tcl_GetString(objv[2]);
      z_സന്ദേശം = Tcl_GetString(objv[3]);
      rc = sqlite3_vfslog_annotate(zVfs, z_സന്ദേശം);
      എങ്കിൽ( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp, "failed", (അക്ഷരം*)0);
        തിരികെ TCL_ERROR;
      }
      നിർത്തുക;
    }
    അവസ്ഥ VL_FINALIZE: {
      അക്ഷരം *zVfs;
      എങ്കിൽ( objc!=3 ){
        Tcl_Wrong_അക്കം_Args(interp, 2, objv, "VFS");
        തിരികെ TCL_ERROR;
      }
      zVfs = Tcl_GetString(objv[2]);
      rc = sqlite3_vfslog_finalize(zVfs);
      എങ്കിൽ( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp, "failed", (അക്ഷരം*)0);
        തിരികെ TCL_ERROR;
      }
      നിർത്തുക;
    };

    അവസ്ഥ VL_NEW: {
      അക്ഷരം *zVfs;
      അക്ഷരം *zParent;
      അക്ഷരം *zLog;
      എങ്കിൽ( objc!=5 ){
        Tcl_Wrong_അക്കം_Args(interp, 2, objv, "VFS PARENT LOGFILE");
        തിരികെ TCL_ERROR;
      }
      zVfs = Tcl_GetString(objv[2]);
      zParent = Tcl_GetString(objv[3]);
      zLog = Tcl_GetString(objv[4]);
      എങ്കിൽ( *zParent=='\0' ) zParent = 0;
      rc = sqlite3_vfslog_new(zVfs, zParent, zLog);
      എങ്കിൽ( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp, "failed", (അക്ഷരം*)0);
        തിരികെ TCL_ERROR;
      }
      നിർത്തുക;
    };

    അവസ്ഥ VL_REGISTER: {
      അക്ഷരം *zDb;
      എങ്കിൽ( objc!=3 ){
        Tcl_Wrong_അക്കം_Args(interp, 2, objv, "DB");
        തിരികെ TCL_ERROR;
      }
#ifdef SQLITE_OMIT_VIRTUALTABLE
      Tcl_AppendResult(interp, "vfslog not available because of "
                               "SQLITE_OMIT_VIRTUALTABLE", (ശൂന്യം*)0);
      തിരികെ TCL_ERROR;
#അല്ലെങ്കിൽ
      zDb = Tcl_GetString(objv[2]);
      എങ്കിൽ( Tcl_GetCommandInfo(interp, zDb, &cmdInfo) ){
        db = ((ഘടന SqliteDb*)cmdInfo.objClientData)->db;
        rc = sqlite3_vfslog_register(db);
      }
      എങ്കിൽ( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp, "bad sqlite3 handle: ", zDb, (ശൂന്യം*)0);
        തിരികെ TCL_ERROR;
      }
      നിർത്തുക;
#endif
    }
  }

  തിരികെ TCL_OK;
}

പൂർണ്ണസംഖ്യ SqlitetestOsinst_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "vfslog", test_vfslog, 0, 0);
  തിരികെ TCL_OK;
}

#endif /* SQLITE_TEST */
