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

#અન્તર્ભાવ "sqlite3.h"

#ifdef _WIN32
#અન્તર્ભાવ <windows.h>
#endif

#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <assert.h>


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

પ્રકારવ્યાખ્યા સંરચના VfslogVfs VfslogVfs;
પ્રકારવ્યાખ્યા સંરચના VfslogFile VfslogFile;

સંરચના VfslogVfs {
  sqlite3_vfs base;               /* VFS methods */
  sqlite3_vfs *pVfs;              /* Parent VFS */
  પૂર્ણાંક iNextFileId;                /* Next file id */
  sqlite3_file *pLog;             /* Log file handle */
  sqlite3_int64 iOffset;          /* Log file offset of start of write buffer */
  પૂર્ણાંક nBuf;                       /* Number of valid bytes in aBuf[] */
  અક્ષર aBuf[VFSLOG_BUFFERSIZE];   /* Write buffer */
};

સંરચના VfslogFile {
  sqlite3_file base;              /* IO methods */
  sqlite3_file *pReal;            /* Underlying file handle */
  sqlite3_vfs *pVfslog;           /* Associated VsflogVfs object */
  પૂર્ણાંક iFileId;                    /* File id number */
};

#define REALVFS(p) (((VfslogVfs *)(p))->pVfs)



/*
** Method declarations for vfslog_file.
*/
સ્થિર પૂર્ણાંક vfslogClose(sqlite3_file*);
સ્થિર પૂર્ણાંક vfslogRead(sqlite3_file*, શૂન્ય*, પૂર્ણાંક iAmt, sqlite3_int64 iOfst);
સ્થિર પૂર્ણાંક vfslogWrite(sqlite3_file*,અચલ શૂન્ય*,પૂર્ણાંક iAmt, sqlite3_int64 iOfst);
સ્થિર પૂર્ણાંક vfslogTruncate(sqlite3_file*, sqlite3_int64 size);
સ્થિર પૂર્ણાંક vfslogSync(sqlite3_file*, પૂર્ણાંક flags);
સ્થિર પૂર્ણાંક vfslogFileSize(sqlite3_file*, sqlite3_int64 *pSize);
સ્થિર પૂર્ણાંક vfslogLock(sqlite3_file*, પૂર્ણાંક);
સ્થિર પૂર્ણાંક vfslogUnlock(sqlite3_file*, પૂર્ણાંક);
સ્થિર પૂર્ણાંક vfslogCheckReservedLock(sqlite3_file*, પૂર્ણાંક *pResOut);
સ્થિર પૂર્ણાંક vfslogFileControl(sqlite3_file*, પૂર્ણાંક op, શૂન્ય *pArg);
સ્થિર પૂર્ણાંક vfslogSectorSize(sqlite3_file*);
સ્થિર પૂર્ણાંક vfslogDeviceCharacteristics(sqlite3_file*);

સ્થિર પૂર્ણાંક vfslogShmLock(sqlite3_file *pFile, પૂર્ણાંક ofst, પૂર્ણાંક n, પૂર્ણાંક flags);
સ્થિર પૂર્ણાંક vfslogShmMap(sqlite3_file *pFile,પૂર્ણાંક,પૂર્ણાંક,પૂર્ણાંક,અસ્થિર શૂન્ય **);
સ્થિર શૂન્ય vfslogShmBarrier(sqlite3_file*);
સ્થિર પૂર્ણાંક vfslogShmUnmap(sqlite3_file *pFile, પૂર્ણાંક deleteFlag);

/*
** Method declarations for vfslog_vfs.
*/
સ્થિર પૂર્ણાંક vfslogOpen(sqlite3_vfs*, અચલ અક્ષર *, sqlite3_file*, પૂર્ણાંક , પૂર્ણાંક *);
સ્થિર પૂર્ણાંક vfslogDelete(sqlite3_vfs*, અચલ અક્ષર *zName, પૂર્ણાંક syncDir);
સ્થિર પૂર્ણાંક vfslogAccess(sqlite3_vfs*, અચલ અક્ષર *zName, પૂર્ણાંક flags, પૂર્ણાંક *);
સ્થિર પૂર્ણાંક vfslogFullPathname(sqlite3_vfs*, અચલ અક્ષર *zName, પૂર્ણાંક, અક્ષર *zOut);
સ્થિર શૂન્ય *vfslogDlOpen(sqlite3_vfs*, અચલ અક્ષર *zFilename);
સ્થિર શૂન્ય vfslogDlError(sqlite3_vfs*, પૂર્ણાંક nByte, અક્ષર *z_Err_સંદેશ);
સ્થિર શૂન્ય (*vfslogDlSym(sqlite3_vfs *pVfs, શૂન્ય *p, અચલ અક્ષર*zSym))(શૂન્ય);
સ્થિર શૂન્ય vfslogDlClose(sqlite3_vfs*, શૂન્ય*);
સ્થિર પૂર્ણાંક vfslogRandomness(sqlite3_vfs*, પૂર્ણાંક nByte, અક્ષર *zOut);
સ્થિર પૂર્ણાંક vfslogSleep(sqlite3_vfs*, પૂર્ણાંક microseconds);
સ્થિર પૂર્ણાંક vfslogCurrentTime(sqlite3_vfs*, દ્વિગુણ*);

સ્થિર પૂર્ણાંક vfslogGetLastError(sqlite3_vfs*, પૂર્ણાંક, અક્ષર *);
સ્થિર પૂર્ણાંક vfslogCurrentTimeInt64(sqlite3_vfs*, sqlite3_int64*);

સ્થિર sqlite3_vfs vfslog_vfs = {
  1,                              /* iVersion */
  કદ(VfslogFile),             /* szOsFile */
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

સ્થિર sqlite3_io_methods vfslog_io_methods = {
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
#અન્તર્ભાવ <time.h>
સ્થિર sqlite3_uint64 vfslog_time(){
  FILETIME ft;
  sqlite3_uint64 u64time = 0;
 
  GetSystemTimeAsFileTime(&ft);

  u64time |= ft.dw_ઉપલું_Date_Time;
  u64time <<= 32;
  u64time |= ft.dw_નીચલું_Date_Time;

  /* ft is 100-nanosecond intervals, we want microseconds */
  પાછા u64time /(sqlite3_uint64)10;
}
#અથવાજો !defined(NO_GETTOD)
#અન્તર્ભાવ <sys/time.h>
સ્થિર sqlite3_uint64 vfslog_time(){
  સંરચના timeval sTime;
  gettimeofday(&sTime, 0);
  પાછા sTime.tv_usec + (sqlite3_uint64)sTime.tv_sec * 1000000;
}
#નહીંતર
સ્થિર sqlite3_uint64 vfslog_time(){
  પાછા 0;
}
#endif

સ્થિર શૂન્ય vfslog_call(sqlite3_vfs *, પૂર્ણાંક, પૂર્ણાંક, sqlite3_int64, પૂર્ણાંક, પૂર્ણાંક, પૂર્ણાંક);
સ્થિર શૂન્ય vfslog_string(sqlite3_vfs *, અચલ અક્ષર *);

/*
** Close an vfslog-file.
*/
સ્થિર પૂર્ણાંક vfslogClose(sqlite3_file *pFile){
  sqlite3_uint64 t;
  પૂર્ણાંક rc = SQLITE_OK;
  VfslogFile *p = (VfslogFile *)pFile;

  t = vfslog_time();
  જો( p->pReal->pMethods ){
    rc = p->pReal->pMethods->xClose(p->pReal);
  }
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_CLOSE, p->iFileId, t, rc, 0, 0);
  પાછા rc;
}

/*
** Read data from an vfslog-file.
*/
સ્થિર પૂર્ણાંક vfslogRead(
  sqlite3_file *pFile, 
  શૂન્ય *zBuf, 
  પૂર્ણાંક iAmt, 
  sqlite_int64 iOfst
){
  પૂર્ણાંક rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xRead(p->pReal, zBuf, iAmt, iOfst);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_READ, p->iFileId, t, rc, iAmt, (પૂર્ણાંક)iOfst);
  પાછા rc;
}

/*
** Write data to an vfslog-file.
*/
સ્થિર પૂર્ણાંક vfslogWrite(
  sqlite3_file *pFile,
  અચલ શૂન્ય *z,
  પૂર્ણાંક iAmt,
  sqlite_int64 iOfst
){
  પૂર્ણાંક rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xWrite(p->pReal, z, iAmt, iOfst);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_WRITE, p->iFileId, t, rc, iAmt, (પૂર્ણાંક)iOfst);
  પાછા rc;
}

/*
** Truncate an vfslog-file.
*/
સ્થિર પૂર્ણાંક vfslogTruncate(sqlite3_file *pFile, sqlite_int64 size){
  પૂર્ણાંક rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xTruncate(p->pReal, size);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_TRUNCATE, p->iFileId, t, rc, 0, (પૂર્ણાંક)size);
  પાછા rc;
}

/*
** Sync an vfslog-file.
*/
સ્થિર પૂર્ણાંક vfslogSync(sqlite3_file *pFile, પૂર્ણાંક flags){
  પૂર્ણાંક rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xSync(p->pReal, flags);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SYNC, p->iFileId, t, rc, flags, 0);
  પાછા rc;
}

/*
** Return the current file-size of an vfslog-file.
*/
સ્થિર પૂર્ણાંક vfslogFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  પૂર્ણાંક rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xFileSize(p->pReal, pSize);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_FILESIZE, p->iFileId, t, rc, 0, (પૂર્ણાંક)*pSize);
  પાછા rc;
}

/*
** Lock an vfslog-file.
*/
સ્થિર પૂર્ણાંક vfslogLock(sqlite3_file *pFile, પૂર્ણાંક eLock){
  પૂર્ણાંક rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xLock(p->pReal, eLock);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_LOCK, p->iFileId, t, rc, eLock, 0);
  પાછા rc;
}

/*
** Unlock an vfslog-file.
*/
સ્થિર પૂર્ણાંક vfslogUnlock(sqlite3_file *pFile, પૂર્ણાંક eLock){
  પૂર્ણાંક rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xUnlock(p->pReal, eLock);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_UNLOCK, p->iFileId, t, rc, eLock, 0);
  પાછા rc;
}

/*
** Check if another file-handle holds a RESERVED lock on an vfslog-file.
*/
સ્થિર પૂર્ણાંક vfslogCheckReservedLock(sqlite3_file *pFile, પૂર્ણાંક *pResOut){
  પૂર્ણાંક rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xCheckReservedLock(p->pReal, pResOut);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_CHECKRESERVEDLOCK, p->iFileId, t, rc, *pResOut, 0);
  પાછા rc;
}

/*
** File control method. For custom operations on an vfslog-file.
*/
સ્થિર પૂર્ણાંક vfslogFileControl(sqlite3_file *pFile, પૂર્ણાંક op, શૂન્ય *pArg){
  VfslogFile *p = (VfslogFile *)pFile;
  પૂર્ણાંક rc = p->pReal->pMethods->xFileControl(p->pReal, op, pArg);
  જો( op==SQLITE_FCNTL_VFSNAME && rc==SQLITE_OK ){
    *(અક્ષર**)pArg = sqlite3_mprintf("vfslog/%z", *(અક્ષર**)pArg);
  }
  પાછા rc;
}

/*
** Return the sector-size in bytes for an vfslog-file.
*/
સ્થિર પૂર્ણાંક vfslogSectorSize(sqlite3_file *pFile){
  પૂર્ણાંક rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xSectorSize(p->pReal);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SECTORSIZE, p->iFileId, t, rc, 0, 0);
  પાછા rc;
}

/*
** Return the device characteristic flags supported by an vfslog-file.
*/
સ્થિર પૂર્ણાંક vfslogDeviceCharacteristics(sqlite3_file *pFile){
  પૂર્ણાંક rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xDeviceCharacteristics(p->pReal);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_DEVCHAR, p->iFileId, t, rc, 0, 0);
  પાછા rc;
}

સ્થિર પૂર્ણાંક vfslogShmLock(sqlite3_file *pFile, પૂર્ણાંક ofst, પૂર્ણાંક n, પૂર્ણાંક flags){
  પૂર્ણાંક rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xShmLock(p->pReal, ofst, n, flags);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SHMLOCK, p->iFileId, t, rc, 0, 0);
  પાછા rc;
}
સ્થિર પૂર્ણાંક vfslogShmMap(
  sqlite3_file *pFile, 
  પૂર્ણાંક iRegion, 
  પૂર્ણાંક szRegion, 
  પૂર્ણાંક isWrite, 
  અસ્થિર શૂન્ય **pp
){
  પૂર્ણાંક rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xShmMap(p->pReal, iRegion, szRegion, isWrite, pp);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SHMMAP, p->iFileId, t, rc, 0, 0);
  પાછા rc;
}
સ્થિર શૂન્ય vfslogShmBarrier(sqlite3_file *pFile){
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  p->pReal->pMethods->xShmBarrier(p->pReal);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SHMBARRIER, p->iFileId, t, SQLITE_OK, 0, 0);
}
સ્થિર પૂર્ણાંક vfslogShmUnmap(sqlite3_file *pFile, પૂર્ણાંક deleteFlag){
  પૂર્ણાંક rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xShmUnmap(p->pReal, deleteFlag);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SHMUNMAP, p->iFileId, t, rc, 0, 0);
  પાછા rc;
}


/*
** Open an vfslog file handle.
*/
સ્થિર પૂર્ણાંક vfslogOpen(
  sqlite3_vfs *pVfs,
  અચલ અક્ષર *zName,
  sqlite3_file *pFile,
  પૂર્ણાંક flags,
  પૂર્ણાંક *pOutFlags
){
  પૂર્ણાંક rc;
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
  પાછા rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
સ્થિર પૂર્ણાંક vfslogDelete(sqlite3_vfs *pVfs, અચલ અક્ષર *zPath, પૂર્ણાંક dirSync){
  પૂર્ણાંક rc;
  sqlite3_uint64 t;
  t = vfslog_time();
  rc = REALVFS(pVfs)->xDelete(REALVFS(pVfs), zPath, dirSync);
  t = vfslog_time() - t;
  vfslog_call(pVfs, OS_DELETE, 0, t, rc, dirSync, 0);
  vfslog_string(pVfs, zPath);
  પાછા rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
સ્થિર પૂર્ણાંક vfslogAccess(
  sqlite3_vfs *pVfs, 
  અચલ અક્ષર *zPath, 
  પૂર્ણાંક flags, 
  પૂર્ણાંક *pResOut
){
  પૂર્ણાંક rc;
  sqlite3_uint64 t;
  t = vfslog_time();
  rc = REALVFS(pVfs)->xAccess(REALVFS(pVfs), zPath, flags, pResOut);
  t = vfslog_time() - t;
  vfslog_call(pVfs, OS_ACCESS, 0, t, rc, flags, *pResOut);
  vfslog_string(pVfs, zPath);
  પાછા rc;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
સ્થિર પૂર્ણાંક vfslogFullPathname(
  sqlite3_vfs *pVfs, 
  અચલ અક્ષર *zPath, 
  પૂર્ણાંક nOut, 
  અક્ષર *zOut
){
  પાછા REALVFS(pVfs)->xFullPathname(REALVFS(pVfs), zPath, nOut, zOut);
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
સ્થિર શૂન્ય *vfslogDlOpen(sqlite3_vfs *pVfs, અચલ અક્ષર *zPath){
  પાછા REALVFS(pVfs)->xDlOpen(REALVFS(pVfs), zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
સ્થિર શૂન્ય vfslogDlError(sqlite3_vfs *pVfs, પૂર્ણાંક nByte, અક્ષર *z_Err_સંદેશ){
  REALVFS(pVfs)->xDlError(REALVFS(pVfs), nByte, z_Err_સંદેશ);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
સ્થિર શૂન્ય (*vfslogDlSym(sqlite3_vfs *pVfs, શૂન્ય *p, અચલ અક્ષર *zSym))(શૂન્ય){
  પાછા REALVFS(pVfs)->xDlSym(REALVFS(pVfs), p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
સ્થિર શૂન્ય vfslogDlClose(sqlite3_vfs *pVfs, શૂન્ય *pHandle){
  REALVFS(pVfs)->xDlClose(REALVFS(pVfs), pHandle);
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
સ્થિર પૂર્ણાંક vfslogRandomness(sqlite3_vfs *pVfs, પૂર્ણાંક nByte, અક્ષર *zBufOut){
  પાછા REALVFS(pVfs)->xRandomness(REALVFS(pVfs), nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
સ્થિર પૂર્ણાંક vfslogSleep(sqlite3_vfs *pVfs, પૂર્ણાંક nMicro){
  પાછા REALVFS(pVfs)->xSleep(REALVFS(pVfs), nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
સ્થિર પૂર્ણાંક vfslogCurrentTime(sqlite3_vfs *pVfs, દ્વિગુણ *pTimeOut){
  પાછા REALVFS(pVfs)->xCurrentTime(REALVFS(pVfs), pTimeOut);
}

સ્થિર પૂર્ણાંક vfslogGetLastError(sqlite3_vfs *pVfs, પૂર્ણાંક a, અક્ષર *b){
  પાછા REALVFS(pVfs)->xGetLastError(REALVFS(pVfs), a, b);
}
સ્થિર પૂર્ણાંક vfslogCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *p){
  પાછા REALVFS(pVfs)->xCurrentTimeInt64(REALVFS(pVfs), p);
}

સ્થિર શૂન્ય vfslog_flush(VfslogVfs *p){
#ifdef SQLITE_TEST
  બાહ્ય પૂર્ણાંક sqlite3_io_error_pending;
  બાહ્ય પૂર્ણાંક sqlite3_io_error_persist;
  બાહ્ય પૂર્ણાંક sqlite3_diskfull_pending;

  પૂર્ણાંક pending = sqlite3_io_error_pending;
  પૂર્ણાંક persist = sqlite3_io_error_persist;
  પૂર્ણાંક diskfull = sqlite3_diskfull_pending;

  sqlite3_io_error_pending = 0;
  sqlite3_io_error_persist = 0;
  sqlite3_diskfull_pending = 0;
#endif

  જો( p->nBuf ){
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

સ્થિર શૂન્ય put32bits(unsigned અક્ષર *p, unsigned પૂર્ણાંક v){
  p[0] = v>>24;
  p[1] = (unsigned અક્ષર)(v>>16);
  p[2] = (unsigned અક્ષર)(v>>8);
  p[3] = (unsigned અક્ષર)v;
}

સ્થિર શૂન્ય vfslog_call(
  sqlite3_vfs *pVfs,
  પૂર્ણાંક eEvent,
  પૂર્ણાંક iFileid,
  sqlite3_int64 nClick,
  પૂર્ણાંક return_code,
  પૂર્ણાંક size,
  પૂર્ણાંક offset
){
  VfslogVfs *p = (VfslogVfs *)pVfs;
  unsigned અક્ષર *zRec;
  જો( (24+p->nBuf)>કદ(p->aBuf) ){
    vfslog_flush(p);
  }
  zRec = (unsigned અક્ષર *)&p->aBuf[p->nBuf];
  put32bits(&zRec[0], eEvent);
  put32bits(&zRec[4], iFileid);
  put32bits(&zRec[8], (unsigned પૂર્ણાંક)(nClick&0xffff));
  put32bits(&zRec[12], return_code);
  put32bits(&zRec[16], size);
  put32bits(&zRec[20], offset);
  p->nBuf += 24;
}

સ્થિર શૂન્ય vfslog_string(sqlite3_vfs *pVfs, અચલ અક્ષર *zStr){
  VfslogVfs *p = (VfslogVfs *)pVfs;
  unsigned અક્ષર *zRec;
  પૂર્ણાંક nStr = zStr ? (પૂર્ણાંક)strlen(zStr) : 0;
  જો( (4+nStr+p->nBuf)>કદ(p->aBuf) ){
    vfslog_flush(p);
  }
  zRec = (unsigned અક્ષર *)&p->aBuf[p->nBuf];
  put32bits(&zRec[0], nStr);
  જો( zStr ){
    memcpy(&zRec[4], zStr, nStr);
  }
  p->nBuf += (4 + nStr);
}

સ્થિર શૂન્ય vfslog_finalize(VfslogVfs *p){
  જો( p->pLog->pMethods ){
    vfslog_flush(p);
    p->pLog->pMethods->xClose(p->pLog);
  }
  sqlite3_free(p);
}

પૂર્ણાંક sqlite3_vfslog_finalize(અચલ અક્ષર *zVfs){
  sqlite3_vfs *pVfs;
  pVfs = sqlite3_vfs_find(zVfs);
  જો( !pVfs || pVfs->xOpen!=vfslogOpen ){
    પાછા SQLITE_ERROR;
  } 
  sqlite3_vfs_unregister(pVfs);
  vfslog_finalize((VfslogVfs *)pVfs);
  પાછા SQLITE_OK;
}

પૂર્ણાંક sqlite3_vfslog_new(
  અચલ અક્ષર *zVfs,               /* New VFS name */
  અચલ અક્ષર *zParentVfs,         /* Parent VFS name (or NULL) */
  અચલ અક્ષર *zLog                /* Log file name */
){
  VfslogVfs *p;
  sqlite3_vfs *pParent;
  પૂર્ણાંક nByte;
  પૂર્ણાંક flags;
  પૂર્ણાંક rc;
  અક્ષર *zFile;
  પૂર્ણાંક nVfs;

  pParent = sqlite3_vfs_find(zParentVfs);
  જો( !pParent ){
    પાછા SQLITE_ERROR;
  }

  nVfs = (પૂર્ણાંક)strlen(zVfs);
  nByte = કદ(VfslogVfs) + pParent->szOsFile + nVfs+1+pParent->mxPathname+1;
  p = (VfslogVfs *)sqlite3_malloc(nByte);
  memset(p, 0, nByte);

  p->pVfs = pParent;
  p->pLog = (sqlite3_file *)&p[1];
  memcpy(&p->base, &vfslog_vfs, કદ(sqlite3_vfs));
  p->base.zName = &((અક્ષર *)p->pLog)[pParent->szOsFile];
  p->base.szOsFile += pParent->szOsFile;
  memcpy((અક્ષર *)p->base.zName, zVfs, nVfs);

  zFile = (અક્ષર *)&p->base.zName[nVfs+1];
  pParent->xFullPathname(pParent, zLog, pParent->mxPathname, zFile);

  flags = SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE|SQLITE_OPEN_SUPER_JOURNAL;
  pParent->xDelete(pParent, zFile, 0);
  rc = pParent->xOpen(pParent, zFile, p->pLog, flags, &flags);
  જો( rc==SQLITE_OK ){
    memcpy(p->aBuf, "sqlite_ostrace1.....", 20);
    p->iOffset = 0;
    p->nBuf = 20;
    rc = sqlite3_vfs_register((sqlite3_vfs *)p, 1);
  }
  જો( rc ){
    vfslog_finalize(p);
  }
  પાછા rc;
}

પૂર્ણાંક sqlite3_vfslog_annotate(અચલ અક્ષર *zVfs, અચલ અક્ષર *z_સંદેશ){
  sqlite3_vfs *pVfs;
  pVfs = sqlite3_vfs_find(zVfs);
  જો( !pVfs || pVfs->xOpen!=vfslogOpen ){
    પાછા SQLITE_ERROR;
  } 
  vfslog_call(pVfs, OS_ANNOTATE, 0, 0, 0, 0, 0);
  vfslog_string(pVfs, z_સંદેશ);
  પાછા SQLITE_OK;
}

સ્થિર અચલ અક્ષર *vfslog_eventname(પૂર્ણાંક eEvent){
  અચલ અક્ષર *zEvent = 0;

  પસંદગી( eEvent ){
    સ્થિતિ OS_CLOSE:             zEvent = "xClose"; તોડો;
    સ્થિતિ OS_READ:              zEvent = "xRead"; તોડો;
    સ્થિતિ OS_WRITE:             zEvent = "xWrite"; તોડો;
    સ્થિતિ OS_TRUNCATE:          zEvent = "xTruncate"; તોડો;
    સ્થિતિ OS_SYNC:              zEvent = "xSync"; તોડો;
    સ્થિતિ OS_FILESIZE:          zEvent = "xFilesize"; તોડો;
    સ્થિતિ OS_LOCK:              zEvent = "xLock"; તોડો;
    સ્થિતિ OS_UNLOCK:            zEvent = "xUnlock"; તોડો;
    સ્થિતિ OS_CHECKRESERVEDLOCK: zEvent = "xCheckResLock"; તોડો;
    સ્થિતિ OS_FILECONTROL:       zEvent = "xFileControl"; તોડો;
    સ્થિતિ OS_SECTORSIZE:        zEvent = "xSectorSize"; તોડો;
    સ્થિતિ OS_DEVCHAR:           zEvent = "xDeviceChar"; તોડો;
    સ્થિતિ OS_OPEN:              zEvent = "xOpen"; તોડો;
    સ્થિતિ OS_DELETE:            zEvent = "xDelete"; તોડો;
    સ્થિતિ OS_ACCESS:            zEvent = "xAccess"; તોડો;
    સ્થિતિ OS_FULLPATHNAME:      zEvent = "xFullPathname"; તોડો;
    સ્થિતિ OS_RANDOMNESS:        zEvent = "xRandomness"; તોડો;
    સ્થિતિ OS_SLEEP:             zEvent = "xSleep"; તોડો;
    સ્થિતિ OS_CURRENTTIME:       zEvent = "xCurrentTime"; તોડો;

    સ્થિતિ OS_SHMUNMAP:          zEvent = "xShmUnmap"; તોડો;
    સ્થિતિ OS_SHMLOCK:           zEvent = "xShmLock"; તોડો;
    સ્થિતિ OS_SHMBARRIER:        zEvent = "xShmBarrier"; તોડો;
    સ્થિતિ OS_SHMMAP:            zEvent = "xShmMap"; તોડો;

    સ્થિતિ OS_ANNOTATE:          zEvent = "annotation"; તોડો;
  }

  પાછા zEvent;
}

પ્રકારવ્યાખ્યા સંરચના VfslogVtab VfslogVtab;
પ્રકારવ્યાખ્યા સંરચના VfslogCsr VfslogCsr;

/*
** Virtual table type for the vfslog reader module.
*/
સંરચના VfslogVtab {
  sqlite3_vtab base;              /* Base class */
  sqlite3_file *pFd;              /* File descriptor open on vfslog file */
  sqlite3_int64 nByte;            /* Size of file in bytes */
  અક્ષર *zFile;                    /* File name for pFd */
};

/*
** Virtual table cursor type for the vfslog reader module.
*/
સંરચના VfslogCsr {
  sqlite3_vtab_cursor base;       /* Base class */
  sqlite3_int64 iRowid;           /* Current rowid. */
  sqlite3_int64 iOffset;          /* Offset of next record in file */
  અક્ષર *zTransient;               /* Transient 'file' string */
  પૂર્ણાંક nFile;                      /* Size of array azFile[] */
  અક્ષર **azFile;                  /* File strings */
  unsigned અક્ષર aBuf[1024];       /* Current vfs log entry (read from file) */
};

સ્થિર unsigned પૂર્ણાંક get32bits(unsigned અક્ષર *p){
  પાછા (p[0]<<24) + (p[1]<<16) + (p[2]<<8) + p[3];
}

/*
** The argument must point to a buffer containing a nul-terminated string.
** If the string begins with an SQL quote character it is overwritten by
** the dequoted version. Otherwise the buffer is left unmodified.
*/
સ્થિર શૂન્ય dequote(અક્ષર *z){
  અક્ષર quote;                     /* Quote character (if any ) */
  quote = z[0];
  જો( quote=='[' || quote=='\'' || quote=='"' || quote=='`' ){
    પૂર્ણાંક iIn = 1;                  /* Index of next byte to read from input */
    પૂર્ણાંક iOut = 0;                 /* Index of next byte to write to output */
    જો( quote=='[' ) quote = ']';  
    જ્યારે( z[iIn] ){
      જો( z[iIn]==quote ){
        જો( z[iIn+1]!=quote ) તોડો;
        z[iOut++] = quote;
        iIn += 2;
      }નહીંતર{
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
સ્થિર પૂર્ણાંક vlogConnect(
  sqlite3 *db,
  શૂન્ય *pAux,
  પૂર્ણાંક argc, અચલ અક્ષર *અચલ*argv,
  sqlite3_vtab **ppVtab,
  અક્ષર **pzErr
){
  sqlite3_vfs *pVfs;              /* VFS used to read log file */
  પૂર્ણાંક flags;                      /* flags passed to pVfs->xOpen() */
  VfslogVtab *p;
  પૂર્ણાંક rc;
  પૂર્ણાંક nByte;
  અક્ષર *zFile;

  *ppVtab = 0;
  pVfs = sqlite3_vfs_find(0);
  nByte = કદ(VfslogVtab) + pVfs->szOsFile + pVfs->mxPathname;
  p = sqlite3_malloc(nByte);
  જો( p==0 ) પાછા SQLITE_NOMEM;
  memset(p, 0, nByte);

  p->pFd = (sqlite3_file *)&p[1];
  p->zFile = &((અક્ષર *)p->pFd)[pVfs->szOsFile];

  zFile = sqlite3_mprintf("%s", argv[3]);
  જો( !zFile ){
    sqlite3_free(p);
    પાછા SQLITE_NOMEM;
  }
  dequote(zFile);
  pVfs->xFullPathname(pVfs, zFile, pVfs->mxPathname, p->zFile);
  sqlite3_free(zFile);

  flags = SQLITE_OPEN_READWRITE|SQLITE_OPEN_SUPER_JOURNAL;
  rc = pVfs->xOpen(pVfs, p->zFile, p->pFd, flags, &flags);

  જો( rc==SQLITE_OK ){
    p->pFd->pMethods->xFileSize(p->pFd, &p->nByte);
    sqlite3_declare_vtab(db, 
        "CREATE TABLE xxx(event, file, click, rc, size, offset)"
    );
    *ppVtab = &p->base;
  }નહીંતર{
    sqlite3_free(p);
  }

  પાછા rc;
}

/*
** There is no "best-index". This virtual table always does a linear
** scan of the binary VFS log file.
*/
સ્થિર પૂર્ણાંક vlogBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  pIdxInfo->estimatedCost = 10.0;
  પાછા SQLITE_OK;
}

/*
** Disconnect from or destroy a vfslog virtual table.
*/
સ્થિર પૂર્ણાંક vlogDisconnect(sqlite3_vtab *pVtab){
  VfslogVtab *p = (VfslogVtab *)pVtab;
  જો( p->pFd->pMethods ){
    p->pFd->pMethods->xClose(p->pFd);
    p->pFd->pMethods = 0;
  }
  sqlite3_free(p);
  પાછા SQLITE_OK;
}

/*
** Open a new vfslog cursor.
*/
સ્થિર પૂર્ણાંક vlogOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  VfslogCsr *pCsr;                /* Newly allocated cursor object */

  pCsr = sqlite3_malloc(કદ(VfslogCsr));
  જો( !pCsr ) પાછા SQLITE_NOMEM;
  memset(pCsr, 0, કદ(VfslogCsr));
  *ppCursor = &pCsr->base;
  પાછા SQLITE_OK;
}

/*
** Close a vfslog cursor.
*/
સ્થિર પૂર્ણાંક vlogClose(sqlite3_vtab_cursor *pCursor){
  VfslogCsr *p = (VfslogCsr *)pCursor;
  પૂર્ણાંક i;
  માટે(i=0; i<p->nFile; i++){
    sqlite3_free(p->azFile[i]);
  }
  sqlite3_free(p->azFile);
  sqlite3_free(p->zTransient);
  sqlite3_free(p);
  પાછા SQLITE_OK;
}

/*
** Move a vfslog cursor to the next entry in the file.
*/
સ્થિર પૂર્ણાંક vlogNext(sqlite3_vtab_cursor *pCursor){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  VfslogVtab *p = (VfslogVtab *)pCursor->pVtab;
  પૂર્ણાંક rc = SQLITE_OK;
  પૂર્ણાંક nRead;

  sqlite3_free(pCsr->zTransient);
  pCsr->zTransient = 0;

  nRead = 24;
  જો( pCsr->iOffset+nRead<=p->nByte ){
    પૂર્ણાંક eEvent;
    rc = p->pFd->pMethods->xRead(p->pFd, pCsr->aBuf, nRead, pCsr->iOffset);

    eEvent = get32bits(pCsr->aBuf);
    જો( (rc==SQLITE_OK)
     && (eEvent==OS_OPEN || eEvent==OS_DELETE || eEvent==OS_ACCESS) 
    ){
      અક્ષર buf[4];
      rc = p->pFd->pMethods->xRead(p->pFd, buf, 4, pCsr->iOffset+nRead);
      nRead += 4;
      જો( rc==SQLITE_OK ){
        પૂર્ણાંક nStr = get32bits((unsigned અક્ષર *)buf);
        અક્ષર *zStr = sqlite3_malloc(nStr+1);
        rc = p->pFd->pMethods->xRead(p->pFd, zStr, nStr, pCsr->iOffset+nRead);
        zStr[nStr] = '\0';
        nRead += nStr;

        જો( eEvent==OS_OPEN ){
          પૂર્ણાંક iFileid = get32bits(&pCsr->aBuf[4]);
          જો( iFileid>=pCsr->nFile ){
            પૂર્ણાંક nNew = કદ(pCsr->azFile[0])*(iFileid+1);
            pCsr->azFile = (અક્ષર **)sqlite3_realloc(pCsr->azFile, nNew);
            nNew -= કદ(pCsr->azFile[0])*pCsr->nFile;
            memset(&pCsr->azFile[pCsr->nFile], 0, nNew);
            pCsr->nFile = iFileid+1;
          }
          sqlite3_free(pCsr->azFile[iFileid]);
          pCsr->azFile[iFileid] = zStr;
        }નહીંતર{
          pCsr->zTransient = zStr;
        }
      }
    }
  }

  pCsr->iRowid += 1;
  pCsr->iOffset += nRead;
  પાછા rc;
}

સ્થિર પૂર્ણાંક vlogEof(sqlite3_vtab_cursor *pCursor){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  VfslogVtab *p = (VfslogVtab *)pCursor->pVtab;
  પાછા (pCsr->iOffset>=p->nByte);
}

સ્થિર પૂર્ણાંક vlogFilter(
  sqlite3_vtab_cursor *pCursor, 
  પૂર્ણાંક idx_અંક, અચલ અક્ષર *idxStr,
  પૂર્ણાંક argc, sqlite3_કિંમત **argv
){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  pCsr->iRowid = 0;
  pCsr->iOffset = 20;
  પાછા vlogNext(pCursor);
}

સ્થિર પૂર્ણાંક vlogColumn(
  sqlite3_vtab_cursor *pCursor, 
  sqlite3_context *ctx, 
  પૂર્ણાંક i
){
  unsigned પૂર્ણાંક val;
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;

  પ્રતિજ્ઞા( i<7 );
  val = get32bits(&pCsr->aBuf[4*i]);

  પસંદગી( i ){
    સ્થિતિ 0: {
      sqlite3_result_text(ctx, vfslog_eventname(val), -1, SQLITE_STATIC);
      તોડો;
    }
    સ્થિતિ 1: {
      અક્ષર *zStr = pCsr->zTransient;
      જો( val!=0 && val<(unsigned)pCsr->nFile ){
        zStr = pCsr->azFile[val];
      }
      sqlite3_result_text(ctx, zStr, -1, SQLITE_TRANSIENT);
      તોડો;
    }
    મૂળભૂત:
      sqlite3_result_int(ctx, val);
      તોડો;
  }

  પાછા SQLITE_OK;
}

સ્થિર પૂર્ણાંક vlogRowid(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  *pRowid = pCsr->iRowid;
  પાછા SQLITE_OK;
}

પૂર્ણાંક sqlite3_vfslog_register(sqlite3 *db){
  સ્થિર sqlite3_module vfslog_module = {
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
  પાછા SQLITE_OK;
}
#endif /* SQLITE_OMIT_VIRTUALTABLE */

/**************************************************************************
***************************************************************************
** Tcl interface starts here.
*/

#જો defined(SQLITE_TEST) || defined(TCLSH)

#અન્તર્ભાવ "tclsqlite.h"

સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_vfslog(
  શૂન્ય *clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  સંરચના SqliteDb { sqlite3 *db; };
  sqlite3 *db;
  Tcl_CmdInfo cmdInfo;
  પૂર્ણાંક rc = SQLITE_ERROR;

  સ્થિર અચલ અક્ષર *strs[] = { "annotate", "finalize", "new",  "register", 0 };
  ગણના VL_enum { VL_ANNOTATE, VL_FINALIZE, VL_NEW, VL_REGISTER };
  પૂર્ણાંક iSub;

  જો( objc<2 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "SUB-COMMAND ...");
    પાછા TCL_ERROR;
  }
  જો( Tcl_GetIndexFromObj(interp, objv[1], strs, "sub-command", 0, &iSub) ){
    પાછા TCL_ERROR;
  }

  પસંદગી( (ગણના VL_enum)iSub ){
    સ્થિતિ VL_ANNOTATE: {
      અક્ષર *zVfs;
      અક્ષર *z_સંદેશ;
      જો( objc!=4 ){
        Tcl_Wrong_અંક_Args(interp, 3, objv, "VFS");
        પાછા TCL_ERROR;
      }
      zVfs = Tcl_GetString(objv[2]);
      z_સંદેશ = Tcl_GetString(objv[3]);
      rc = sqlite3_vfslog_annotate(zVfs, z_સંદેશ);
      જો( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp, "failed", (અક્ષર*)0);
        પાછા TCL_ERROR;
      }
      તોડો;
    }
    સ્થિતિ VL_FINALIZE: {
      અક્ષર *zVfs;
      જો( objc!=3 ){
        Tcl_Wrong_અંક_Args(interp, 2, objv, "VFS");
        પાછા TCL_ERROR;
      }
      zVfs = Tcl_GetString(objv[2]);
      rc = sqlite3_vfslog_finalize(zVfs);
      જો( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp, "failed", (અક્ષર*)0);
        પાછા TCL_ERROR;
      }
      તોડો;
    };

    સ્થિતિ VL_NEW: {
      અક્ષર *zVfs;
      અક્ષર *zParent;
      અક્ષર *zLog;
      જો( objc!=5 ){
        Tcl_Wrong_અંક_Args(interp, 2, objv, "VFS PARENT LOGFILE");
        પાછા TCL_ERROR;
      }
      zVfs = Tcl_GetString(objv[2]);
      zParent = Tcl_GetString(objv[3]);
      zLog = Tcl_GetString(objv[4]);
      જો( *zParent=='\0' ) zParent = 0;
      rc = sqlite3_vfslog_new(zVfs, zParent, zLog);
      જો( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp, "failed", (અક્ષર*)0);
        પાછા TCL_ERROR;
      }
      તોડો;
    };

    સ્થિતિ VL_REGISTER: {
      અક્ષર *zDb;
      જો( objc!=3 ){
        Tcl_Wrong_અંક_Args(interp, 2, objv, "DB");
        પાછા TCL_ERROR;
      }
#ifdef SQLITE_OMIT_VIRTUALTABLE
      Tcl_AppendResult(interp, "vfslog not available because of "
                               "SQLITE_OMIT_VIRTUALTABLE", (શૂન્ય*)0);
      પાછા TCL_ERROR;
#નહીંતર
      zDb = Tcl_GetString(objv[2]);
      જો( Tcl_GetCommandInfo(interp, zDb, &cmdInfo) ){
        db = ((સંરચના SqliteDb*)cmdInfo.objClientData)->db;
        rc = sqlite3_vfslog_register(db);
      }
      જો( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp, "bad sqlite3 handle: ", zDb, (શૂન્ય*)0);
        પાછા TCL_ERROR;
      }
      તોડો;
#endif
    }
  }

  પાછા TCL_OK;
}

પૂર્ણાંક SqlitetestOsinst_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "vfslog", test_vfslog, 0, 0);
  પાછા TCL_OK;
}

#endif /* SQLITE_TEST */
