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

প্রকারলেখ কাঠামো VfslogVfs VfslogVfs;
প্রকারলেখ কাঠামো VfslogFile VfslogFile;

কাঠামো VfslogVfs {
  sqlite3_vfs base;               /* VFS methods */
  sqlite3_vfs *pVfs;              /* Parent VFS */
  পূর্ণসংখ্যা i_পরবর্তী_File_Id;                /* Next file id */
  sqlite3_file *pLog;             /* Log file handle */
  sqlite3_int64 iOffset;          /* Log file offset of start of write buffer */
  পূর্ণসংখ্যা nBuf;                       /* Number of valid bytes in aBuf[] */
  অক্ষর aBuf[VFSLOG_BUFFERSIZE];   /* Write buffer */
};

কাঠামো VfslogFile {
  sqlite3_file base;              /* IO methods */
  sqlite3_file *pReal;            /* Underlying file handle */
  sqlite3_vfs *pVfslog;           /* Associated VsflogVfs object */
  পূর্ণসংখ্যা iFileId;                    /* File id number */
};

#define REALVFS(p) (((VfslogVfs *)(p))->pVfs)



/*
** Method declarations for vfslog_file.
*/
স্থির পূর্ণসংখ্যা vfslogClose(sqlite3_file*);
স্থির পূর্ণসংখ্যা vfslogRead(sqlite3_file*, শূন্য*, পূর্ণসংখ্যা iAmt, sqlite3_int64 iOfst);
স্থির পূর্ণসংখ্যা vfslogWrite(sqlite3_file*,ধ্রুবক শূন্য*,পূর্ণসংখ্যা iAmt, sqlite3_int64 iOfst);
স্থির পূর্ণসংখ্যা vfslogTruncate(sqlite3_file*, sqlite3_int64 আকার_2);
স্থির পূর্ণসংখ্যা vfslogSync(sqlite3_file*, পূর্ণসংখ্যা flags);
স্থির পূর্ণসংখ্যা vfslog_File_আকার(sqlite3_file*, sqlite3_int64 *p_আকার);
স্থির পূর্ণসংখ্যা vfslogLock(sqlite3_file*, পূর্ণসংখ্যা);
স্থির পূর্ণসংখ্যা vfslogUnlock(sqlite3_file*, পূর্ণসংখ্যা);
স্থির পূর্ণসংখ্যা vfslogCheckReservedLock(sqlite3_file*, পূর্ণসংখ্যা *pResOut);
স্থির পূর্ণসংখ্যা vfslogFileControl(sqlite3_file*, পূর্ণসংখ্যা op, শূন্য *pArg);
স্থির পূর্ণসংখ্যা vfslog_Sector_আকার(sqlite3_file*);
স্থির পূর্ণসংখ্যা vfslogDeviceCharacteristics(sqlite3_file*);

স্থির পূর্ণসংখ্যা vfslogShmLock(sqlite3_file *pFile, পূর্ণসংখ্যা ofst, পূর্ণসংখ্যা n, পূর্ণসংখ্যা flags);
স্থির পূর্ণসংখ্যা vfslogShmMap(sqlite3_file *pFile,পূর্ণসংখ্যা,পূর্ণসংখ্যা,পূর্ণসংখ্যা,অস্থির শূন্য **);
স্থির শূন্য vfslogShmBarrier(sqlite3_file*);
স্থির পূর্ণসংখ্যা vfslogShmUnmap(sqlite3_file *pFile, পূর্ণসংখ্যা deleteFlag);

/*
** Method declarations for vfslog_vfs.
*/
স্থির পূর্ণসংখ্যা vfslogOpen(sqlite3_vfs*, ধ্রুবক অক্ষর *, sqlite3_file*, পূর্ণসংখ্যা , পূর্ণসংখ্যা *);
স্থির পূর্ণসংখ্যা vfslogDelete(sqlite3_vfs*, ধ্রুবক অক্ষর *zName, পূর্ণসংখ্যা syncDir);
স্থির পূর্ণসংখ্যা vfslogAccess(sqlite3_vfs*, ধ্রুবক অক্ষর *zName, পূর্ণসংখ্যা flags, পূর্ণসংখ্যা *);
স্থির পূর্ণসংখ্যা vfslogFullPathname(sqlite3_vfs*, ধ্রুবক অক্ষর *zName, পূর্ণসংখ্যা, অক্ষর *zOut);
স্থির শূন্য *vfslogDlOpen(sqlite3_vfs*, ধ্রুবক অক্ষর *zFilename);
স্থির শূন্য vfslogDlError(sqlite3_vfs*, পূর্ণসংখ্যা nByte, অক্ষর *z_Err_বার্তা);
স্থির শূন্য (*vfslogDlSym(sqlite3_vfs *pVfs, শূন্য *p, ধ্রুবক অক্ষর*zSym))(শূন্য);
স্থির শূন্য vfslogDlClose(sqlite3_vfs*, শূন্য*);
স্থির পূর্ণসংখ্যা vfslogRandomness(sqlite3_vfs*, পূর্ণসংখ্যা nByte, অক্ষর *zOut);
স্থির পূর্ণসংখ্যা vfslogSleep(sqlite3_vfs*, পূর্ণসংখ্যা microseconds);
স্থির পূর্ণসংখ্যা vfslogCurrentTime(sqlite3_vfs*, দ্বিগুণ*);

স্থির পূর্ণসংখ্যা vfslogGetLastError(sqlite3_vfs*, পূর্ণসংখ্যা, অক্ষর *);
স্থির পূর্ণসংখ্যা vfslogCurrentTimeInt64(sqlite3_vfs*, sqlite3_int64*);

স্থির sqlite3_vfs vfslog_vfs = {
  1,                              /* iVersion */
  আকার(VfslogFile),             /* szOsFile */
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
  vfslog_File_আকার,                 /* xFileSize */
  vfslogLock,                     /* xLock */
  vfslogUnlock,                   /* xUnlock */
  vfslogCheckReservedLock,        /* xCheckReservedLock */
  vfslogFileControl,              /* xFileControl */
  vfslog_Sector_আকার,               /* xSectorSize */
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
  ফেরত u64time /(sqlite3_uint64)10;
}
#নতুবা_যদি !defined(NO_GETTOD)
#অন্তর্ভাব <sys/time.h>
স্থির sqlite3_uint64 vfslog_time(){
  কাঠামো timeval sTime;
  gettimeofday(&sTime, 0);
  ফেরত sTime.tv_usec + (sqlite3_uint64)sTime.tv_sec * 1000000;
}
#অন্যথা
স্থির sqlite3_uint64 vfslog_time(){
  ফেরত 0;
}
#endif

স্থির শূন্য vfslog_call(sqlite3_vfs *, পূর্ণসংখ্যা, পূর্ণসংখ্যা, sqlite3_int64, পূর্ণসংখ্যা, পূর্ণসংখ্যা, পূর্ণসংখ্যা);
স্থির শূন্য vfslog_string(sqlite3_vfs *, ধ্রুবক অক্ষর *);

/*
** Close an vfslog-file.
*/
স্থির পূর্ণসংখ্যা vfslogClose(sqlite3_file *pFile){
  sqlite3_uint64 t;
  পূর্ণসংখ্যা rc = SQLITE_OK;
  VfslogFile *p = (VfslogFile *)pFile;

  t = vfslog_time();
  যদি( p->pReal->pMethods ){
    rc = p->pReal->pMethods->xClose(p->pReal);
  }
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_CLOSE, p->iFileId, t, rc, 0, 0);
  ফেরত rc;
}

/*
** Read data from an vfslog-file.
*/
স্থির পূর্ণসংখ্যা vfslogRead(
  sqlite3_file *pFile, 
  শূন্য *zBuf, 
  পূর্ণসংখ্যা iAmt, 
  sqlite_int64 iOfst
){
  পূর্ণসংখ্যা rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xRead(p->pReal, zBuf, iAmt, iOfst);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_READ, p->iFileId, t, rc, iAmt, (পূর্ণসংখ্যা)iOfst);
  ফেরত rc;
}

/*
** Write data to an vfslog-file.
*/
স্থির পূর্ণসংখ্যা vfslogWrite(
  sqlite3_file *pFile,
  ধ্রুবক শূন্য *z,
  পূর্ণসংখ্যা iAmt,
  sqlite_int64 iOfst
){
  পূর্ণসংখ্যা rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xWrite(p->pReal, z, iAmt, iOfst);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_WRITE, p->iFileId, t, rc, iAmt, (পূর্ণসংখ্যা)iOfst);
  ফেরত rc;
}

/*
** Truncate an vfslog-file.
*/
স্থির পূর্ণসংখ্যা vfslogTruncate(sqlite3_file *pFile, sqlite_int64 আকার_2){
  পূর্ণসংখ্যা rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xTruncate(p->pReal, আকার_2);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_TRUNCATE, p->iFileId, t, rc, 0, (পূর্ণসংখ্যা)আকার_2);
  ফেরত rc;
}

/*
** Sync an vfslog-file.
*/
স্থির পূর্ণসংখ্যা vfslogSync(sqlite3_file *pFile, পূর্ণসংখ্যা flags){
  পূর্ণসংখ্যা rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xSync(p->pReal, flags);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SYNC, p->iFileId, t, rc, flags, 0);
  ফেরত rc;
}

/*
** Return the current file-size of an vfslog-file.
*/
স্থির পূর্ণসংখ্যা vfslog_File_আকার(sqlite3_file *pFile, sqlite_int64 *p_আকার){
  পূর্ণসংখ্যা rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->x_File_আকার(p->pReal, p_আকার);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_FILESIZE, p->iFileId, t, rc, 0, (পূর্ণসংখ্যা)*p_আকার);
  ফেরত rc;
}

/*
** Lock an vfslog-file.
*/
স্থির পূর্ণসংখ্যা vfslogLock(sqlite3_file *pFile, পূর্ণসংখ্যা eLock){
  পূর্ণসংখ্যা rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xLock(p->pReal, eLock);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_LOCK, p->iFileId, t, rc, eLock, 0);
  ফেরত rc;
}

/*
** Unlock an vfslog-file.
*/
স্থির পূর্ণসংখ্যা vfslogUnlock(sqlite3_file *pFile, পূর্ণসংখ্যা eLock){
  পূর্ণসংখ্যা rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xUnlock(p->pReal, eLock);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_UNLOCK, p->iFileId, t, rc, eLock, 0);
  ফেরত rc;
}

/*
** Check if another file-handle holds a RESERVED lock on an vfslog-file.
*/
স্থির পূর্ণসংখ্যা vfslogCheckReservedLock(sqlite3_file *pFile, পূর্ণসংখ্যা *pResOut){
  পূর্ণসংখ্যা rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xCheckReservedLock(p->pReal, pResOut);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_CHECKRESERVEDLOCK, p->iFileId, t, rc, *pResOut, 0);
  ফেরত rc;
}

/*
** File control method. For custom operations on an vfslog-file.
*/
স্থির পূর্ণসংখ্যা vfslogFileControl(sqlite3_file *pFile, পূর্ণসংখ্যা op, শূন্য *pArg){
  VfslogFile *p = (VfslogFile *)pFile;
  পূর্ণসংখ্যা rc = p->pReal->pMethods->xFileControl(p->pReal, op, pArg);
  যদি( op==SQLITE_FCNTL_VFSNAME && rc==SQLITE_OK ){
    *(অক্ষর**)pArg = sqlite3_mprintf("vfslog/%z", *(অক্ষর**)pArg);
  }
  ফেরত rc;
}

/*
** Return the sector-size in bytes for an vfslog-file.
*/
স্থির পূর্ণসংখ্যা vfslog_Sector_আকার(sqlite3_file *pFile){
  পূর্ণসংখ্যা rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->x_Sector_আকার(p->pReal);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SECTORSIZE, p->iFileId, t, rc, 0, 0);
  ফেরত rc;
}

/*
** Return the device characteristic flags supported by an vfslog-file.
*/
স্থির পূর্ণসংখ্যা vfslogDeviceCharacteristics(sqlite3_file *pFile){
  পূর্ণসংখ্যা rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xDeviceCharacteristics(p->pReal);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_DEVCHAR, p->iFileId, t, rc, 0, 0);
  ফেরত rc;
}

স্থির পূর্ণসংখ্যা vfslogShmLock(sqlite3_file *pFile, পূর্ণসংখ্যা ofst, পূর্ণসংখ্যা n, পূর্ণসংখ্যা flags){
  পূর্ণসংখ্যা rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xShmLock(p->pReal, ofst, n, flags);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SHMLOCK, p->iFileId, t, rc, 0, 0);
  ফেরত rc;
}
স্থির পূর্ণসংখ্যা vfslogShmMap(
  sqlite3_file *pFile, 
  পূর্ণসংখ্যা iRegion, 
  পূর্ণসংখ্যা szRegion, 
  পূর্ণসংখ্যা isWrite, 
  অস্থির শূন্য **pp
){
  পূর্ণসংখ্যা rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xShmMap(p->pReal, iRegion, szRegion, isWrite, pp);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SHMMAP, p->iFileId, t, rc, 0, 0);
  ফেরত rc;
}
স্থির শূন্য vfslogShmBarrier(sqlite3_file *pFile){
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  p->pReal->pMethods->xShmBarrier(p->pReal);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SHMBARRIER, p->iFileId, t, SQLITE_OK, 0, 0);
}
স্থির পূর্ণসংখ্যা vfslogShmUnmap(sqlite3_file *pFile, পূর্ণসংখ্যা deleteFlag){
  পূর্ণসংখ্যা rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xShmUnmap(p->pReal, deleteFlag);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SHMUNMAP, p->iFileId, t, rc, 0, 0);
  ফেরত rc;
}


/*
** Open an vfslog file handle.
*/
স্থির পূর্ণসংখ্যা vfslogOpen(
  sqlite3_vfs *pVfs,
  ধ্রুবক অক্ষর *zName,
  sqlite3_file *pFile,
  পূর্ণসংখ্যা flags,
  পূর্ণসংখ্যা *pOutFlags
){
  পূর্ণসংখ্যা rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  VfslogVfs *pLog = (VfslogVfs *)pVfs;

  pFile->pMethods = &vfslog_io_methods;
  p->pReal = (sqlite3_file *)&p[1];
  p->pVfslog = pVfs;
  p->iFileId = ++pLog->i_পরবর্তী_File_Id;

  t = vfslog_time();
  rc = REALVFS(pVfs)->xOpen(REALVFS(pVfs), zName, p->pReal, flags, pOutFlags);
  t = vfslog_time() - t;

  vfslog_call(pVfs, OS_OPEN, p->iFileId, t, rc, 0, 0);
  vfslog_string(pVfs, zName);
  ফেরত rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
স্থির পূর্ণসংখ্যা vfslogDelete(sqlite3_vfs *pVfs, ধ্রুবক অক্ষর *zPath, পূর্ণসংখ্যা dirSync){
  পূর্ণসংখ্যা rc;
  sqlite3_uint64 t;
  t = vfslog_time();
  rc = REALVFS(pVfs)->xDelete(REALVFS(pVfs), zPath, dirSync);
  t = vfslog_time() - t;
  vfslog_call(pVfs, OS_DELETE, 0, t, rc, dirSync, 0);
  vfslog_string(pVfs, zPath);
  ফেরত rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
স্থির পূর্ণসংখ্যা vfslogAccess(
  sqlite3_vfs *pVfs, 
  ধ্রুবক অক্ষর *zPath, 
  পূর্ণসংখ্যা flags, 
  পূর্ণসংখ্যা *pResOut
){
  পূর্ণসংখ্যা rc;
  sqlite3_uint64 t;
  t = vfslog_time();
  rc = REALVFS(pVfs)->xAccess(REALVFS(pVfs), zPath, flags, pResOut);
  t = vfslog_time() - t;
  vfslog_call(pVfs, OS_ACCESS, 0, t, rc, flags, *pResOut);
  vfslog_string(pVfs, zPath);
  ফেরত rc;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
স্থির পূর্ণসংখ্যা vfslogFullPathname(
  sqlite3_vfs *pVfs, 
  ধ্রুবক অক্ষর *zPath, 
  পূর্ণসংখ্যা nOut, 
  অক্ষর *zOut
){
  ফেরত REALVFS(pVfs)->xFullPathname(REALVFS(pVfs), zPath, nOut, zOut);
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
স্থির শূন্য *vfslogDlOpen(sqlite3_vfs *pVfs, ধ্রুবক অক্ষর *zPath){
  ফেরত REALVFS(pVfs)->xDlOpen(REALVFS(pVfs), zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
স্থির শূন্য vfslogDlError(sqlite3_vfs *pVfs, পূর্ণসংখ্যা nByte, অক্ষর *z_Err_বার্তা){
  REALVFS(pVfs)->xDlError(REALVFS(pVfs), nByte, z_Err_বার্তা);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
স্থির শূন্য (*vfslogDlSym(sqlite3_vfs *pVfs, শূন্য *p, ধ্রুবক অক্ষর *zSym))(শূন্য){
  ফেরত REALVFS(pVfs)->xDlSym(REALVFS(pVfs), p, zSym);
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
স্থির পূর্ণসংখ্যা vfslogRandomness(sqlite3_vfs *pVfs, পূর্ণসংখ্যা nByte, অক্ষর *zBufOut){
  ফেরত REALVFS(pVfs)->xRandomness(REALVFS(pVfs), nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
স্থির পূর্ণসংখ্যা vfslogSleep(sqlite3_vfs *pVfs, পূর্ণসংখ্যা nMicro){
  ফেরত REALVFS(pVfs)->xSleep(REALVFS(pVfs), nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
স্থির পূর্ণসংখ্যা vfslogCurrentTime(sqlite3_vfs *pVfs, দ্বিগুণ *pTimeOut){
  ফেরত REALVFS(pVfs)->xCurrentTime(REALVFS(pVfs), pTimeOut);
}

স্থির পূর্ণসংখ্যা vfslogGetLastError(sqlite3_vfs *pVfs, পূর্ণসংখ্যা a, অক্ষর *b){
  ফেরত REALVFS(pVfs)->xGetLastError(REALVFS(pVfs), a, b);
}
স্থির পূর্ণসংখ্যা vfslogCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *p){
  ফেরত REALVFS(pVfs)->xCurrentTimeInt64(REALVFS(pVfs), p);
}

স্থির শূন্য vfslog_flush(VfslogVfs *p){
#ifdef SQLITE_TEST
  বাহ্যিক পূর্ণসংখ্যা sqlite3_io_error_pending;
  বাহ্যিক পূর্ণসংখ্যা sqlite3_io_error_persist;
  বাহ্যিক পূর্ণসংখ্যা sqlite3_diskfull_pending;

  পূর্ণসংখ্যা pending = sqlite3_io_error_pending;
  পূর্ণসংখ্যা persist = sqlite3_io_error_persist;
  পূর্ণসংখ্যা diskfull = sqlite3_diskfull_pending;

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

স্থির শূন্য put32bits(unsigned অক্ষর *p, unsigned পূর্ণসংখ্যা v){
  p[0] = v>>24;
  p[1] = (unsigned অক্ষর)(v>>16);
  p[2] = (unsigned অক্ষর)(v>>8);
  p[3] = (unsigned অক্ষর)v;
}

স্থির শূন্য vfslog_call(
  sqlite3_vfs *pVfs,
  পূর্ণসংখ্যা eEvent,
  পূর্ণসংখ্যা iFileid,
  sqlite3_int64 nClick,
  পূর্ণসংখ্যা return_code,
  পূর্ণসংখ্যা আকার_2,
  পূর্ণসংখ্যা offset
){
  VfslogVfs *p = (VfslogVfs *)pVfs;
  unsigned অক্ষর *zRec;
  যদি( (24+p->nBuf)>আকার(p->aBuf) ){
    vfslog_flush(p);
  }
  zRec = (unsigned অক্ষর *)&p->aBuf[p->nBuf];
  put32bits(&zRec[0], eEvent);
  put32bits(&zRec[4], iFileid);
  put32bits(&zRec[8], (unsigned পূর্ণসংখ্যা)(nClick&0xffff));
  put32bits(&zRec[12], return_code);
  put32bits(&zRec[16], আকার_2);
  put32bits(&zRec[20], offset);
  p->nBuf += 24;
}

স্থির শূন্য vfslog_string(sqlite3_vfs *pVfs, ধ্রুবক অক্ষর *zStr){
  VfslogVfs *p = (VfslogVfs *)pVfs;
  unsigned অক্ষর *zRec;
  পূর্ণসংখ্যা nStr = zStr ? (পূর্ণসংখ্যা)strlen(zStr) : 0;
  যদি( (4+nStr+p->nBuf)>আকার(p->aBuf) ){
    vfslog_flush(p);
  }
  zRec = (unsigned অক্ষর *)&p->aBuf[p->nBuf];
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

পূর্ণসংখ্যা sqlite3_vfslog_finalize(ধ্রুবক অক্ষর *zVfs){
  sqlite3_vfs *pVfs;
  pVfs = sqlite3_vfs_find(zVfs);
  যদি( !pVfs || pVfs->xOpen!=vfslogOpen ){
    ফেরত SQLITE_ERROR;
  } 
  sqlite3_vfs_unregister(pVfs);
  vfslog_finalize((VfslogVfs *)pVfs);
  ফেরত SQLITE_OK;
}

পূর্ণসংখ্যা sqlite3_vfslog_new(
  ধ্রুবক অক্ষর *zVfs,               /* New VFS name */
  ধ্রুবক অক্ষর *zParentVfs,         /* Parent VFS name (or NULL) */
  ধ্রুবক অক্ষর *zLog                /* Log file name */
){
  VfslogVfs *p;
  sqlite3_vfs *pParent;
  পূর্ণসংখ্যা nByte;
  পূর্ণসংখ্যা flags;
  পূর্ণসংখ্যা rc;
  অক্ষর *zFile;
  পূর্ণসংখ্যা nVfs;

  pParent = sqlite3_vfs_find(zParentVfs);
  যদি( !pParent ){
    ফেরত SQLITE_ERROR;
  }

  nVfs = (পূর্ণসংখ্যা)strlen(zVfs);
  nByte = আকার(VfslogVfs) + pParent->szOsFile + nVfs+1+pParent->mxPathname+1;
  p = (VfslogVfs *)sqlite3_malloc(nByte);
  memset(p, 0, nByte);

  p->pVfs = pParent;
  p->pLog = (sqlite3_file *)&p[1];
  memcpy(&p->base, &vfslog_vfs, আকার(sqlite3_vfs));
  p->base.zName = &((অক্ষর *)p->pLog)[pParent->szOsFile];
  p->base.szOsFile += pParent->szOsFile;
  memcpy((অক্ষর *)p->base.zName, zVfs, nVfs);

  zFile = (অক্ষর *)&p->base.zName[nVfs+1];
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
  ফেরত rc;
}

পূর্ণসংখ্যা sqlite3_vfslog_annotate(ধ্রুবক অক্ষর *zVfs, ধ্রুবক অক্ষর *z_বার্তা){
  sqlite3_vfs *pVfs;
  pVfs = sqlite3_vfs_find(zVfs);
  যদি( !pVfs || pVfs->xOpen!=vfslogOpen ){
    ফেরত SQLITE_ERROR;
  } 
  vfslog_call(pVfs, OS_ANNOTATE, 0, 0, 0, 0, 0);
  vfslog_string(pVfs, z_বার্তা);
  ফেরত SQLITE_OK;
}

স্থির ধ্রুবক অক্ষর *vfslog_eventname(পূর্ণসংখ্যা eEvent){
  ধ্রুবক অক্ষর *zEvent = 0;

  নির্বাচন( eEvent ){
    ক্ষেত্র OS_CLOSE:             zEvent = "xClose"; বিরতি;
    ক্ষেত্র OS_READ:              zEvent = "xRead"; বিরতি;
    ক্ষেত্র OS_WRITE:             zEvent = "xWrite"; বিরতি;
    ক্ষেত্র OS_TRUNCATE:          zEvent = "xTruncate"; বিরতি;
    ক্ষেত্র OS_SYNC:              zEvent = "xSync"; বিরতি;
    ক্ষেত্র OS_FILESIZE:          zEvent = "xFilesize"; বিরতি;
    ক্ষেত্র OS_LOCK:              zEvent = "xLock"; বিরতি;
    ক্ষেত্র OS_UNLOCK:            zEvent = "xUnlock"; বিরতি;
    ক্ষেত্র OS_CHECKRESERVEDLOCK: zEvent = "xCheckResLock"; বিরতি;
    ক্ষেত্র OS_FILECONTROL:       zEvent = "xFileControl"; বিরতি;
    ক্ষেত্র OS_SECTORSIZE:        zEvent = "xSectorSize"; বিরতি;
    ক্ষেত্র OS_DEVCHAR:           zEvent = "xDeviceChar"; বিরতি;
    ক্ষেত্র OS_OPEN:              zEvent = "xOpen"; বিরতি;
    ক্ষেত্র OS_DELETE:            zEvent = "xDelete"; বিরতি;
    ক্ষেত্র OS_ACCESS:            zEvent = "xAccess"; বিরতি;
    ক্ষেত্র OS_FULLPATHNAME:      zEvent = "xFullPathname"; বিরতি;
    ক্ষেত্র OS_RANDOMNESS:        zEvent = "xRandomness"; বিরতি;
    ক্ষেত্র OS_SLEEP:             zEvent = "xSleep"; বিরতি;
    ক্ষেত্র OS_CURRENTTIME:       zEvent = "xCurrentTime"; বিরতি;

    ক্ষেত্র OS_SHMUNMAP:          zEvent = "xShmUnmap"; বিরতি;
    ক্ষেত্র OS_SHMLOCK:           zEvent = "xShmLock"; বিরতি;
    ক্ষেত্র OS_SHMBARRIER:        zEvent = "xShmBarrier"; বিরতি;
    ক্ষেত্র OS_SHMMAP:            zEvent = "xShmMap"; বিরতি;

    ক্ষেত্র OS_ANNOTATE:          zEvent = "annotation"; বিরতি;
  }

  ফেরত zEvent;
}

প্রকারলেখ কাঠামো VfslogVtab VfslogVtab;
প্রকারলেখ কাঠামো VfslogCsr VfslogCsr;

/*
** Virtual table type for the vfslog reader module.
*/
কাঠামো VfslogVtab {
  sqlite3_vtab base;              /* Base class */
  sqlite3_file *pFd;              /* File descriptor open on vfslog file */
  sqlite3_int64 nByte;            /* Size of file in bytes */
  অক্ষর *zFile;                    /* File name for pFd */
};

/*
** Virtual table cursor type for the vfslog reader module.
*/
কাঠামো VfslogCsr {
  sqlite3_vtab_cursor base;       /* Base class */
  sqlite3_int64 iRowid;           /* Current rowid. */
  sqlite3_int64 iOffset;          /* Offset of next record in file */
  অক্ষর *zTransient;               /* Transient 'file' string */
  পূর্ণসংখ্যা nFile;                      /* Size of array azFile[] */
  অক্ষর **azFile;                  /* File strings */
  unsigned অক্ষর aBuf[1024];       /* Current vfs log entry (read from file) */
};

স্থির unsigned পূর্ণসংখ্যা get32bits(unsigned অক্ষর *p){
  ফেরত (p[0]<<24) + (p[1]<<16) + (p[2]<<8) + p[3];
}

/*
** The argument must point to a buffer containing a nul-terminated string.
** If the string begins with an SQL quote character it is overwritten by
** the dequoted version. Otherwise the buffer is left unmodified.
*/
স্থির শূন্য dequote(অক্ষর *z){
  অক্ষর quote;                     /* Quote character (if any ) */
  quote = z[0];
  যদি( quote=='[' || quote=='\'' || quote=='"' || quote=='`' ){
    পূর্ণসংখ্যা iIn = 1;                  /* Index of next byte to read from input */
    পূর্ণসংখ্যা iOut = 0;                 /* Index of next byte to write to output */
    যদি( quote=='[' ) quote = ']';  
    যখন( z[iIn] ){
      যদি( z[iIn]==quote ){
        যদি( z[iIn+1]!=quote ) বিরতি;
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
স্থির পূর্ণসংখ্যা vlogConnect(
  sqlite3 *db,
  শূন্য *pAux,
  পূর্ণসংখ্যা argc, ধ্রুবক অক্ষর *ধ্রুবক*argv,
  sqlite3_vtab **ppVtab,
  অক্ষর **pzErr
){
  sqlite3_vfs *pVfs;              /* VFS used to read log file */
  পূর্ণসংখ্যা flags;                      /* flags passed to pVfs->xOpen() */
  VfslogVtab *p;
  পূর্ণসংখ্যা rc;
  পূর্ণসংখ্যা nByte;
  অক্ষর *zFile;

  *ppVtab = 0;
  pVfs = sqlite3_vfs_find(0);
  nByte = আকার(VfslogVtab) + pVfs->szOsFile + pVfs->mxPathname;
  p = sqlite3_malloc(nByte);
  যদি( p==0 ) ফেরত SQLITE_NOMEM;
  memset(p, 0, nByte);

  p->pFd = (sqlite3_file *)&p[1];
  p->zFile = &((অক্ষর *)p->pFd)[pVfs->szOsFile];

  zFile = sqlite3_mprintf("%s", argv[3]);
  যদি( !zFile ){
    sqlite3_free(p);
    ফেরত SQLITE_NOMEM;
  }
  dequote(zFile);
  pVfs->xFullPathname(pVfs, zFile, pVfs->mxPathname, p->zFile);
  sqlite3_free(zFile);

  flags = SQLITE_OPEN_READWRITE|SQLITE_OPEN_SUPER_JOURNAL;
  rc = pVfs->xOpen(pVfs, p->zFile, p->pFd, flags, &flags);

  যদি( rc==SQLITE_OK ){
    p->pFd->pMethods->x_File_আকার(p->pFd, &p->nByte);
    sqlite3_declare_vtab(db, 
        "CREATE TABLE xxx(event, file, click, rc, size, offset)"
    );
    *ppVtab = &p->base;
  }অন্যথা{
    sqlite3_free(p);
  }

  ফেরত rc;
}

/*
** There is no "best-index". This virtual table always does a linear
** scan of the binary VFS log file.
*/
স্থির পূর্ণসংখ্যা vlog_Best_সূচক(sqlite3_vtab *tab, sqlite3_সূচক_info *pIdxInfo){
  pIdxInfo->estimatedCost = 10.0;
  ফেরত SQLITE_OK;
}

/*
** Disconnect from or destroy a vfslog virtual table.
*/
স্থির পূর্ণসংখ্যা vlogDisconnect(sqlite3_vtab *pVtab){
  VfslogVtab *p = (VfslogVtab *)pVtab;
  যদি( p->pFd->pMethods ){
    p->pFd->pMethods->xClose(p->pFd);
    p->pFd->pMethods = 0;
  }
  sqlite3_free(p);
  ফেরত SQLITE_OK;
}

/*
** Open a new vfslog cursor.
*/
স্থির পূর্ণসংখ্যা vlogOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  VfslogCsr *pCsr;                /* Newly allocated cursor object */

  pCsr = sqlite3_malloc(আকার(VfslogCsr));
  যদি( !pCsr ) ফেরত SQLITE_NOMEM;
  memset(pCsr, 0, আকার(VfslogCsr));
  *ppCursor = &pCsr->base;
  ফেরত SQLITE_OK;
}

/*
** Close a vfslog cursor.
*/
স্থির পূর্ণসংখ্যা vlogClose(sqlite3_vtab_cursor *pCursor){
  VfslogCsr *p = (VfslogCsr *)pCursor;
  পূর্ণসংখ্যা i;
  জন্য(i=0; i<p->nFile; i++){
    sqlite3_free(p->azFile[i]);
  }
  sqlite3_free(p->azFile);
  sqlite3_free(p->zTransient);
  sqlite3_free(p);
  ফেরত SQLITE_OK;
}

/*
** Move a vfslog cursor to the next entry in the file.
*/
স্থির পূর্ণসংখ্যা vlog_পরবর্তী(sqlite3_vtab_cursor *pCursor){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  VfslogVtab *p = (VfslogVtab *)pCursor->pVtab;
  পূর্ণসংখ্যা rc = SQLITE_OK;
  পূর্ণসংখ্যা nRead;

  sqlite3_free(pCsr->zTransient);
  pCsr->zTransient = 0;

  nRead = 24;
  যদি( pCsr->iOffset+nRead<=p->nByte ){
    পূর্ণসংখ্যা eEvent;
    rc = p->pFd->pMethods->xRead(p->pFd, pCsr->aBuf, nRead, pCsr->iOffset);

    eEvent = get32bits(pCsr->aBuf);
    যদি( (rc==SQLITE_OK)
     && (eEvent==OS_OPEN || eEvent==OS_DELETE || eEvent==OS_ACCESS) 
    ){
      অক্ষর buf[4];
      rc = p->pFd->pMethods->xRead(p->pFd, buf, 4, pCsr->iOffset+nRead);
      nRead += 4;
      যদি( rc==SQLITE_OK ){
        পূর্ণসংখ্যা nStr = get32bits((unsigned অক্ষর *)buf);
        অক্ষর *zStr = sqlite3_malloc(nStr+1);
        rc = p->pFd->pMethods->xRead(p->pFd, zStr, nStr, pCsr->iOffset+nRead);
        zStr[nStr] = '\0';
        nRead += nStr;

        যদি( eEvent==OS_OPEN ){
          পূর্ণসংখ্যা iFileid = get32bits(&pCsr->aBuf[4]);
          যদি( iFileid>=pCsr->nFile ){
            পূর্ণসংখ্যা nNew = আকার(pCsr->azFile[0])*(iFileid+1);
            pCsr->azFile = (অক্ষর **)sqlite3_realloc(pCsr->azFile, nNew);
            nNew -= আকার(pCsr->azFile[0])*pCsr->nFile;
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
  ফেরত rc;
}

স্থির পূর্ণসংখ্যা vlogEof(sqlite3_vtab_cursor *pCursor){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  VfslogVtab *p = (VfslogVtab *)pCursor->pVtab;
  ফেরত (pCsr->iOffset>=p->nByte);
}

স্থির পূর্ণসংখ্যা vlogFilter(
  sqlite3_vtab_cursor *pCursor, 
  পূর্ণসংখ্যা idx_অঙ্ক, ধ্রুবক অক্ষর *idxStr,
  পূর্ণসংখ্যা argc, sqlite3_মান **argv
){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  pCsr->iRowid = 0;
  pCsr->iOffset = 20;
  ফেরত vlog_পরবর্তী(pCursor);
}

স্থির পূর্ণসংখ্যা vlogColumn(
  sqlite3_vtab_cursor *pCursor, 
  sqlite3_context *ctx, 
  পূর্ণসংখ্যা i
){
  unsigned পূর্ণসংখ্যা val;
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;

  প্রতিজ্ঞা( i<7 );
  val = get32bits(&pCsr->aBuf[4*i]);

  নির্বাচন( i ){
    ক্ষেত্র 0: {
      sqlite3_ফলাফল_text(ctx, vfslog_eventname(val), -1, SQLITE_STATIC);
      বিরতি;
    }
    ক্ষেত্র 1: {
      অক্ষর *zStr = pCsr->zTransient;
      যদি( val!=0 && val<(unsigned)pCsr->nFile ){
        zStr = pCsr->azFile[val];
      }
      sqlite3_ফলাফল_text(ctx, zStr, -1, SQLITE_TRANSIENT);
      বিরতি;
    }
    পূর্বনির্ধারিত:
      sqlite3_ফলাফল_int(ctx, val);
      বিরতি;
  }

  ফেরত SQLITE_OK;
}

স্থির পূর্ণসংখ্যা vlogRowid(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  *pRowid = pCsr->iRowid;
  ফেরত SQLITE_OK;
}

পূর্ণসংখ্যা sqlite3_vfslog_register(sqlite3 *db){
  স্থির sqlite3_module vfslog_module = {
    0,                            /* iVersion */
    vlogConnect,                /* xCreate */
    vlogConnect,                /* xConnect */
    vlog_Best_সূচক,              /* xBestIndex */
    vlogDisconnect,             /* xDisconnect */
    vlogDisconnect,             /* xDestroy */
    vlogOpen,                   /* xOpen - open a cursor */
    vlogClose,                  /* xClose - close a cursor */
    vlogFilter,                 /* xFilter - configure scan constraints */
    vlog_পরবর্তী,                   /* xNext - advance a cursor */
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
  ফেরত SQLITE_OK;
}
#endif /* SQLITE_OMIT_VIRTUALTABLE */

/**************************************************************************
***************************************************************************
** Tcl interface starts here.
*/

#যদি defined(SQLITE_TEST) || defined(TCLSH)

#অন্তর্ভাব "tclsqlite.h"

স্থির পূর্ণসংখ্যা SQLITE_TCLAPI test_vfslog(
  শূন্য *client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  কাঠামো SqliteDb { sqlite3 *db; };
  sqlite3 *db;
  Tcl_CmdInfo cmdInfo;
  পূর্ণসংখ্যা rc = SQLITE_ERROR;

  স্থির ধ্রুবক অক্ষর *strs[] = { "annotate", "finalize", "new",  "register", 0 };
  গণনা VL_enum { VL_ANNOTATE, VL_FINALIZE, VL_NEW, VL_REGISTER };
  পূর্ণসংখ্যা iSub;

  যদি( objc<2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "SUB-COMMAND ...");
    ফেরত TCL_ERROR;
  }
  যদি( Tcl_Get_সূচক_From_Obj(interp, objv[1], strs, "sub-command", 0, &iSub) ){
    ফেরত TCL_ERROR;
  }

  নির্বাচন( (গণনা VL_enum)iSub ){
    ক্ষেত্র VL_ANNOTATE: {
      অক্ষর *zVfs;
      অক্ষর *z_বার্তা;
      যদি( objc!=4 ){
        Tcl_Wrong_অঙ্ক_Args(interp, 3, objv, "VFS");
        ফেরত TCL_ERROR;
      }
      zVfs = Tcl_GetString(objv[2]);
      z_বার্তা = Tcl_GetString(objv[3]);
      rc = sqlite3_vfslog_annotate(zVfs, z_বার্তা);
      যদি( rc!=SQLITE_OK ){
        Tcl_Append_ফলাফল(interp, "failed", (অক্ষর*)0);
        ফেরত TCL_ERROR;
      }
      বিরতি;
    }
    ক্ষেত্র VL_FINALIZE: {
      অক্ষর *zVfs;
      যদি( objc!=3 ){
        Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "VFS");
        ফেরত TCL_ERROR;
      }
      zVfs = Tcl_GetString(objv[2]);
      rc = sqlite3_vfslog_finalize(zVfs);
      যদি( rc!=SQLITE_OK ){
        Tcl_Append_ফলাফল(interp, "failed", (অক্ষর*)0);
        ফেরত TCL_ERROR;
      }
      বিরতি;
    };

    ক্ষেত্র VL_NEW: {
      অক্ষর *zVfs;
      অক্ষর *zParent;
      অক্ষর *zLog;
      যদি( objc!=5 ){
        Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "VFS PARENT LOGFILE");
        ফেরত TCL_ERROR;
      }
      zVfs = Tcl_GetString(objv[2]);
      zParent = Tcl_GetString(objv[3]);
      zLog = Tcl_GetString(objv[4]);
      যদি( *zParent=='\0' ) zParent = 0;
      rc = sqlite3_vfslog_new(zVfs, zParent, zLog);
      যদি( rc!=SQLITE_OK ){
        Tcl_Append_ফলাফল(interp, "failed", (অক্ষর*)0);
        ফেরত TCL_ERROR;
      }
      বিরতি;
    };

    ক্ষেত্র VL_REGISTER: {
      অক্ষর *zDb;
      যদি( objc!=3 ){
        Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "DB");
        ফেরত TCL_ERROR;
      }
#ifdef SQLITE_OMIT_VIRTUALTABLE
      Tcl_Append_ফলাফল(interp, "vfslog not available because of "
                               "SQLITE_OMIT_VIRTUALTABLE", (শূন্য*)0);
      ফেরত TCL_ERROR;
#অন্যথা
      zDb = Tcl_GetString(objv[2]);
      যদি( Tcl_GetCommandInfo(interp, zDb, &cmdInfo) ){
        db = ((কাঠামো SqliteDb*)cmdInfo.obj_Client_তথ্য)->db;
        rc = sqlite3_vfslog_register(db);
      }
      যদি( rc!=SQLITE_OK ){
        Tcl_Append_ফলাফল(interp, "bad sqlite3 handle: ", zDb, (শূন্য*)0);
        ফেরত TCL_ERROR;
      }
      বিরতি;
#endif
    }
  }

  ফেরত TCL_OK;
}

পূর্ণসংখ্যা SqlitetestOsinst_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "vfslog", test_vfslog, 0, 0);
  ফেরত TCL_OK;
}

#endif /* SQLITE_TEST */
