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

#अन्तर्भाव "sqlite3.h"

#ifdef _WIN32
#अन्तर्भाव <windows.h>
#endif

#अन्तर्भाव <string.h>
#अन्तर्भाव <assert.h>


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

प्रकारनिर्देश संरचना VfslogVfs VfslogVfs;
प्रकारनिर्देश संरचना VfslogFile VfslogFile;

संरचना VfslogVfs {
  sqlite3_vfs base;               /* VFS methods */
  sqlite3_vfs *pVfs;              /* Parent VFS */
  पूर्णाङ्क iNextFileId;                /* Next file id */
  sqlite3_file *pLog;             /* Log file handle */
  sqlite3_int64 iOffset;          /* Log file offset of start of write buffer */
  पूर्णाङ्क nBuf;                       /* Number of valid bytes in aBuf[] */
  वर्ण aBuf[VFSLOG_BUFFERSIZE];   /* Write buffer */
};

संरचना VfslogFile {
  sqlite3_file base;              /* IO methods */
  sqlite3_file *pReal;            /* Underlying file handle */
  sqlite3_vfs *pVfslog;           /* Associated VsflogVfs object */
  पूर्णाङ्क iFileId;                    /* File id number */
};

#define REALVFS(p) (((VfslogVfs *)(p))->pVfs)



/*
** Method declarations for vfslog_file.
*/
स्थिर पूर्णाङ्क vfslogClose(sqlite3_file*);
स्थिर पूर्णाङ्क vfslogRead(sqlite3_file*, शून्य*, पूर्णाङ्क iAmt, sqlite3_int64 iOfst);
स्थिर पूर्णाङ्क vfslogWrite(sqlite3_file*,अचर शून्य*,पूर्णाङ्क iAmt, sqlite3_int64 iOfst);
स्थिर पूर्णाङ्क vfslogTruncate(sqlite3_file*, sqlite3_int64 size);
स्थिर पूर्णाङ्क vfslogSync(sqlite3_file*, पूर्णाङ्क flags);
स्थिर पूर्णाङ्क vfslogFileSize(sqlite3_file*, sqlite3_int64 *pSize);
स्थिर पूर्णाङ्क vfslogLock(sqlite3_file*, पूर्णाङ्क);
स्थिर पूर्णाङ्क vfslogUnlock(sqlite3_file*, पूर्णाङ्क);
स्थिर पूर्णाङ्क vfslogCheckReservedLock(sqlite3_file*, पूर्णाङ्क *pResOut);
स्थिर पूर्णाङ्क vfslogFileControl(sqlite3_file*, पूर्णाङ्क op, शून्य *pArg);
स्थिर पूर्णाङ्क vfslogSectorSize(sqlite3_file*);
स्थिर पूर्णाङ्क vfslogDeviceCharacteristics(sqlite3_file*);

स्थिर पूर्णाङ्क vfslogShmLock(sqlite3_file *pFile, पूर्णाङ्क ofst, पूर्णाङ्क n, पूर्णाङ्क flags);
स्थिर पूर्णाङ्क vfslogShmMap(sqlite3_file *pFile,पूर्णाङ्क,पूर्णाङ्क,पूर्णाङ्क,चञ्चल शून्य **);
स्थिर शून्य vfslogShmBarrier(sqlite3_file*);
स्थिर पूर्णाङ्क vfslogShmUnmap(sqlite3_file *pFile, पूर्णाङ्क deleteFlag);

/*
** Method declarations for vfslog_vfs.
*/
स्थिर पूर्णाङ्क vfslogOpen(sqlite3_vfs*, अचर वर्ण *, sqlite3_file*, पूर्णाङ्क , पूर्णाङ्क *);
स्थिर पूर्णाङ्क vfslogDelete(sqlite3_vfs*, अचर वर्ण *zName, पूर्णाङ्क syncDir);
स्थिर पूर्णाङ्क vfslogAccess(sqlite3_vfs*, अचर वर्ण *zName, पूर्णाङ्क flags, पूर्णाङ्क *);
स्थिर पूर्णाङ्क vfslogFullPathname(sqlite3_vfs*, अचर वर्ण *zName, पूर्णाङ्क, वर्ण *zOut);
स्थिर शून्य *vfslogDlOpen(sqlite3_vfs*, अचर वर्ण *zFilename);
स्थिर शून्य vfslogDlError(sqlite3_vfs*, पूर्णाङ्क nByte, वर्ण *z_Err_सन्देश);
स्थिर शून्य (*vfslogDlSym(sqlite3_vfs *pVfs, शून्य *p, अचर वर्ण*zSym))(शून्य);
स्थिर शून्य vfslogDlClose(sqlite3_vfs*, शून्य*);
स्थिर पूर्णाङ्क vfslogRandomness(sqlite3_vfs*, पूर्णाङ्क nByte, वर्ण *zOut);
स्थिर पूर्णाङ्क vfslogSleep(sqlite3_vfs*, पूर्णाङ्क microseconds);
स्थिर पूर्णाङ्क vfslogCurrentTime(sqlite3_vfs*, द्विगुण*);

स्थिर पूर्णाङ्क vfslogGetLastError(sqlite3_vfs*, पूर्णाङ्क, वर्ण *);
स्थिर पूर्णाङ्क vfslogCurrentTimeInt64(sqlite3_vfs*, sqlite3_int64*);

स्थिर sqlite3_vfs vfslog_vfs = {
  1,                              /* iVersion */
  परिमाण(VfslogFile),             /* szOsFile */
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

स्थिर sqlite3_io_methods vfslog_io_methods = {
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
#अन्तर्भाव <time.h>
स्थिर sqlite3_uint64 vfslog_time(){
  FILETIME ft;
  sqlite3_uint64 u64time = 0;
 
  GetSystemTimeAsFileTime(&ft);

  u64time |= ft.dw_माथिल्लो_Date_Time;
  u64time <<= 32;
  u64time |= ft.dw_तल्लो_Date_Time;

  /* ft is 100-nanosecond intervals, we want microseconds */
  फर्काऊ u64time /(sqlite3_uint64)10;
}
#अथवायदि !defined(NO_GETTOD)
#अन्तर्भाव <sys/time.h>
स्थिर sqlite3_uint64 vfslog_time(){
  संरचना timeval sTime;
  gettimeofday(&sTime, 0);
  फर्काऊ sTime.tv_usec + (sqlite3_uint64)sTime.tv_sec * 1000000;
}
#अन्यथा
स्थिर sqlite3_uint64 vfslog_time(){
  फर्काऊ 0;
}
#endif

स्थिर शून्य vfslog_call(sqlite3_vfs *, पूर्णाङ्क, पूर्णाङ्क, sqlite3_int64, पूर्णाङ्क, पूर्णाङ्क, पूर्णाङ्क);
स्थिर शून्य vfslog_string(sqlite3_vfs *, अचर वर्ण *);

/*
** Close an vfslog-file.
*/
स्थिर पूर्णाङ्क vfslogClose(sqlite3_file *pFile){
  sqlite3_uint64 t;
  पूर्णाङ्क rc = SQLITE_OK;
  VfslogFile *p = (VfslogFile *)pFile;

  t = vfslog_time();
  यदि( p->pReal->pMethods ){
    rc = p->pReal->pMethods->xClose(p->pReal);
  }
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_CLOSE, p->iFileId, t, rc, 0, 0);
  फर्काऊ rc;
}

/*
** Read data from an vfslog-file.
*/
स्थिर पूर्णाङ्क vfslogRead(
  sqlite3_file *pFile, 
  शून्य *zBuf, 
  पूर्णाङ्क iAmt, 
  sqlite_int64 iOfst
){
  पूर्णाङ्क rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xRead(p->pReal, zBuf, iAmt, iOfst);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_READ, p->iFileId, t, rc, iAmt, (पूर्णाङ्क)iOfst);
  फर्काऊ rc;
}

/*
** Write data to an vfslog-file.
*/
स्थिर पूर्णाङ्क vfslogWrite(
  sqlite3_file *pFile,
  अचर शून्य *z,
  पूर्णाङ्क iAmt,
  sqlite_int64 iOfst
){
  पूर्णाङ्क rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xWrite(p->pReal, z, iAmt, iOfst);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_WRITE, p->iFileId, t, rc, iAmt, (पूर्णाङ्क)iOfst);
  फर्काऊ rc;
}

/*
** Truncate an vfslog-file.
*/
स्थिर पूर्णाङ्क vfslogTruncate(sqlite3_file *pFile, sqlite_int64 size){
  पूर्णाङ्क rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xTruncate(p->pReal, size);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_TRUNCATE, p->iFileId, t, rc, 0, (पूर्णाङ्क)size);
  फर्काऊ rc;
}

/*
** Sync an vfslog-file.
*/
स्थिर पूर्णाङ्क vfslogSync(sqlite3_file *pFile, पूर्णाङ्क flags){
  पूर्णाङ्क rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xSync(p->pReal, flags);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SYNC, p->iFileId, t, rc, flags, 0);
  फर्काऊ rc;
}

/*
** Return the current file-size of an vfslog-file.
*/
स्थिर पूर्णाङ्क vfslogFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  पूर्णाङ्क rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xFileSize(p->pReal, pSize);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_FILESIZE, p->iFileId, t, rc, 0, (पूर्णाङ्क)*pSize);
  फर्काऊ rc;
}

/*
** Lock an vfslog-file.
*/
स्थिर पूर्णाङ्क vfslogLock(sqlite3_file *pFile, पूर्णाङ्क eLock){
  पूर्णाङ्क rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xLock(p->pReal, eLock);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_LOCK, p->iFileId, t, rc, eLock, 0);
  फर्काऊ rc;
}

/*
** Unlock an vfslog-file.
*/
स्थिर पूर्णाङ्क vfslogUnlock(sqlite3_file *pFile, पूर्णाङ्क eLock){
  पूर्णाङ्क rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xUnlock(p->pReal, eLock);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_UNLOCK, p->iFileId, t, rc, eLock, 0);
  फर्काऊ rc;
}

/*
** Check if another file-handle holds a RESERVED lock on an vfslog-file.
*/
स्थिर पूर्णाङ्क vfslogCheckReservedLock(sqlite3_file *pFile, पूर्णाङ्क *pResOut){
  पूर्णाङ्क rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xCheckReservedLock(p->pReal, pResOut);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_CHECKRESERVEDLOCK, p->iFileId, t, rc, *pResOut, 0);
  फर्काऊ rc;
}

/*
** File control method. For custom operations on an vfslog-file.
*/
स्थिर पूर्णाङ्क vfslogFileControl(sqlite3_file *pFile, पूर्णाङ्क op, शून्य *pArg){
  VfslogFile *p = (VfslogFile *)pFile;
  पूर्णाङ्क rc = p->pReal->pMethods->xFileControl(p->pReal, op, pArg);
  यदि( op==SQLITE_FCNTL_VFSNAME && rc==SQLITE_OK ){
    *(वर्ण**)pArg = sqlite3_mprintf("vfslog/%z", *(वर्ण**)pArg);
  }
  फर्काऊ rc;
}

/*
** Return the sector-size in bytes for an vfslog-file.
*/
स्थिर पूर्णाङ्क vfslogSectorSize(sqlite3_file *pFile){
  पूर्णाङ्क rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xSectorSize(p->pReal);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SECTORSIZE, p->iFileId, t, rc, 0, 0);
  फर्काऊ rc;
}

/*
** Return the device characteristic flags supported by an vfslog-file.
*/
स्थिर पूर्णाङ्क vfslogDeviceCharacteristics(sqlite3_file *pFile){
  पूर्णाङ्क rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xDeviceCharacteristics(p->pReal);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_DEVCHAR, p->iFileId, t, rc, 0, 0);
  फर्काऊ rc;
}

स्थिर पूर्णाङ्क vfslogShmLock(sqlite3_file *pFile, पूर्णाङ्क ofst, पूर्णाङ्क n, पूर्णाङ्क flags){
  पूर्णाङ्क rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xShmLock(p->pReal, ofst, n, flags);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SHMLOCK, p->iFileId, t, rc, 0, 0);
  फर्काऊ rc;
}
स्थिर पूर्णाङ्क vfslogShmMap(
  sqlite3_file *pFile, 
  पूर्णाङ्क iRegion, 
  पूर्णाङ्क szRegion, 
  पूर्णाङ्क isWrite, 
  चञ्चल शून्य **pp
){
  पूर्णाङ्क rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xShmMap(p->pReal, iRegion, szRegion, isWrite, pp);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SHMMAP, p->iFileId, t, rc, 0, 0);
  फर्काऊ rc;
}
स्थिर शून्य vfslogShmBarrier(sqlite3_file *pFile){
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  p->pReal->pMethods->xShmBarrier(p->pReal);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SHMBARRIER, p->iFileId, t, SQLITE_OK, 0, 0);
}
स्थिर पूर्णाङ्क vfslogShmUnmap(sqlite3_file *pFile, पूर्णाङ्क deleteFlag){
  पूर्णाङ्क rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xShmUnmap(p->pReal, deleteFlag);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SHMUNMAP, p->iFileId, t, rc, 0, 0);
  फर्काऊ rc;
}


/*
** Open an vfslog file handle.
*/
स्थिर पूर्णाङ्क vfslogOpen(
  sqlite3_vfs *pVfs,
  अचर वर्ण *zName,
  sqlite3_file *pFile,
  पूर्णाङ्क flags,
  पूर्णाङ्क *pOutFlags
){
  पूर्णाङ्क rc;
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
  फर्काऊ rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
स्थिर पूर्णाङ्क vfslogDelete(sqlite3_vfs *pVfs, अचर वर्ण *zPath, पूर्णाङ्क dirSync){
  पूर्णाङ्क rc;
  sqlite3_uint64 t;
  t = vfslog_time();
  rc = REALVFS(pVfs)->xDelete(REALVFS(pVfs), zPath, dirSync);
  t = vfslog_time() - t;
  vfslog_call(pVfs, OS_DELETE, 0, t, rc, dirSync, 0);
  vfslog_string(pVfs, zPath);
  फर्काऊ rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
स्थिर पूर्णाङ्क vfslogAccess(
  sqlite3_vfs *pVfs, 
  अचर वर्ण *zPath, 
  पूर्णाङ्क flags, 
  पूर्णाङ्क *pResOut
){
  पूर्णाङ्क rc;
  sqlite3_uint64 t;
  t = vfslog_time();
  rc = REALVFS(pVfs)->xAccess(REALVFS(pVfs), zPath, flags, pResOut);
  t = vfslog_time() - t;
  vfslog_call(pVfs, OS_ACCESS, 0, t, rc, flags, *pResOut);
  vfslog_string(pVfs, zPath);
  फर्काऊ rc;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
स्थिर पूर्णाङ्क vfslogFullPathname(
  sqlite3_vfs *pVfs, 
  अचर वर्ण *zPath, 
  पूर्णाङ्क nOut, 
  वर्ण *zOut
){
  फर्काऊ REALVFS(pVfs)->xFullPathname(REALVFS(pVfs), zPath, nOut, zOut);
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
स्थिर शून्य *vfslogDlOpen(sqlite3_vfs *pVfs, अचर वर्ण *zPath){
  फर्काऊ REALVFS(pVfs)->xDlOpen(REALVFS(pVfs), zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
स्थिर शून्य vfslogDlError(sqlite3_vfs *pVfs, पूर्णाङ्क nByte, वर्ण *z_Err_सन्देश){
  REALVFS(pVfs)->xDlError(REALVFS(pVfs), nByte, z_Err_सन्देश);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
स्थिर शून्य (*vfslogDlSym(sqlite3_vfs *pVfs, शून्य *p, अचर वर्ण *zSym))(शून्य){
  फर्काऊ REALVFS(pVfs)->xDlSym(REALVFS(pVfs), p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
स्थिर शून्य vfslogDlClose(sqlite3_vfs *pVfs, शून्य *pHandle){
  REALVFS(pVfs)->xDlClose(REALVFS(pVfs), pHandle);
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
स्थिर पूर्णाङ्क vfslogRandomness(sqlite3_vfs *pVfs, पूर्णाङ्क nByte, वर्ण *zBufOut){
  फर्काऊ REALVFS(pVfs)->xRandomness(REALVFS(pVfs), nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
स्थिर पूर्णाङ्क vfslogSleep(sqlite3_vfs *pVfs, पूर्णाङ्क nMicro){
  फर्काऊ REALVFS(pVfs)->xSleep(REALVFS(pVfs), nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
स्थिर पूर्णाङ्क vfslogCurrentTime(sqlite3_vfs *pVfs, द्विगुण *pTimeOut){
  फर्काऊ REALVFS(pVfs)->xCurrentTime(REALVFS(pVfs), pTimeOut);
}

स्थिर पूर्णाङ्क vfslogGetLastError(sqlite3_vfs *pVfs, पूर्णाङ्क a, वर्ण *b){
  फर्काऊ REALVFS(pVfs)->xGetLastError(REALVFS(pVfs), a, b);
}
स्थिर पूर्णाङ्क vfslogCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *p){
  फर्काऊ REALVFS(pVfs)->xCurrentTimeInt64(REALVFS(pVfs), p);
}

स्थिर शून्य vfslog_flush(VfslogVfs *p){
#ifdef SQLITE_TEST
  बाह्य पूर्णाङ्क sqlite3_io_error_pending;
  बाह्य पूर्णाङ्क sqlite3_io_error_persist;
  बाह्य पूर्णाङ्क sqlite3_diskfull_pending;

  पूर्णाङ्क pending = sqlite3_io_error_pending;
  पूर्णाङ्क persist = sqlite3_io_error_persist;
  पूर्णाङ्क diskfull = sqlite3_diskfull_pending;

  sqlite3_io_error_pending = 0;
  sqlite3_io_error_persist = 0;
  sqlite3_diskfull_pending = 0;
#endif

  यदि( p->nBuf ){
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

स्थिर शून्य put32bits(अचिन्हित वर्ण *p, अचिन्हित पूर्णाङ्क v){
  p[0] = v>>24;
  p[1] = (अचिन्हित वर्ण)(v>>16);
  p[2] = (अचिन्हित वर्ण)(v>>8);
  p[3] = (अचिन्हित वर्ण)v;
}

स्थिर शून्य vfslog_call(
  sqlite3_vfs *pVfs,
  पूर्णाङ्क eEvent,
  पूर्णाङ्क iFileid,
  sqlite3_int64 nClick,
  पूर्णाङ्क return_code,
  पूर्णाङ्क size,
  पूर्णाङ्क offset
){
  VfslogVfs *p = (VfslogVfs *)pVfs;
  अचिन्हित वर्ण *zRec;
  यदि( (24+p->nBuf)>परिमाण(p->aBuf) ){
    vfslog_flush(p);
  }
  zRec = (अचिन्हित वर्ण *)&p->aBuf[p->nBuf];
  put32bits(&zRec[0], eEvent);
  put32bits(&zRec[4], iFileid);
  put32bits(&zRec[8], (अचिन्हित पूर्णाङ्क)(nClick&0xffff));
  put32bits(&zRec[12], return_code);
  put32bits(&zRec[16], size);
  put32bits(&zRec[20], offset);
  p->nBuf += 24;
}

स्थिर शून्य vfslog_string(sqlite3_vfs *pVfs, अचर वर्ण *zStr){
  VfslogVfs *p = (VfslogVfs *)pVfs;
  अचिन्हित वर्ण *zRec;
  पूर्णाङ्क nStr = zStr ? (पूर्णाङ्क)म_माप(zStr) : 0;
  यदि( (4+nStr+p->nBuf)>परिमाण(p->aBuf) ){
    vfslog_flush(p);
  }
  zRec = (अचिन्हित वर्ण *)&p->aBuf[p->nBuf];
  put32bits(&zRec[0], nStr);
  यदि( zStr ){
    स_नकल(&zRec[4], zStr, nStr);
  }
  p->nBuf += (4 + nStr);
}

स्थिर शून्य vfslog_finalize(VfslogVfs *p){
  यदि( p->pLog->pMethods ){
    vfslog_flush(p);
    p->pLog->pMethods->xClose(p->pLog);
  }
  sqlite3_free(p);
}

पूर्णाङ्क sqlite3_vfslog_finalize(अचर वर्ण *zVfs){
  sqlite3_vfs *pVfs;
  pVfs = sqlite3_vfs_find(zVfs);
  यदि( !pVfs || pVfs->xOpen!=vfslogOpen ){
    फर्काऊ SQLITE_ERROR;
  } 
  sqlite3_vfs_unregister(pVfs);
  vfslog_finalize((VfslogVfs *)pVfs);
  फर्काऊ SQLITE_OK;
}

पूर्णाङ्क sqlite3_vfslog_new(
  अचर वर्ण *zVfs,               /* New VFS name */
  अचर वर्ण *zParentVfs,         /* Parent VFS name (or NULL) */
  अचर वर्ण *zLog                /* Log file name */
){
  VfslogVfs *p;
  sqlite3_vfs *pParent;
  पूर्णाङ्क nByte;
  पूर्णाङ्क flags;
  पूर्णाङ्क rc;
  वर्ण *zFile;
  पूर्णाङ्क nVfs;

  pParent = sqlite3_vfs_find(zParentVfs);
  यदि( !pParent ){
    फर्काऊ SQLITE_ERROR;
  }

  nVfs = (पूर्णाङ्क)म_माप(zVfs);
  nByte = परिमाण(VfslogVfs) + pParent->szOsFile + nVfs+1+pParent->mxPathname+1;
  p = (VfslogVfs *)sqlite3_malloc(nByte);
  स_रखो(p, 0, nByte);

  p->pVfs = pParent;
  p->pLog = (sqlite3_file *)&p[1];
  स_नकल(&p->base, &vfslog_vfs, परिमाण(sqlite3_vfs));
  p->base.zName = &((वर्ण *)p->pLog)[pParent->szOsFile];
  p->base.szOsFile += pParent->szOsFile;
  स_नकल((वर्ण *)p->base.zName, zVfs, nVfs);

  zFile = (वर्ण *)&p->base.zName[nVfs+1];
  pParent->xFullPathname(pParent, zLog, pParent->mxPathname, zFile);

  flags = SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE|SQLITE_OPEN_SUPER_JOURNAL;
  pParent->xDelete(pParent, zFile, 0);
  rc = pParent->xOpen(pParent, zFile, p->pLog, flags, &flags);
  यदि( rc==SQLITE_OK ){
    स_नकल(p->aBuf, "sqlite_ostrace1.....", 20);
    p->iOffset = 0;
    p->nBuf = 20;
    rc = sqlite3_vfs_register((sqlite3_vfs *)p, 1);
  }
  यदि( rc ){
    vfslog_finalize(p);
  }
  फर्काऊ rc;
}

पूर्णाङ्क sqlite3_vfslog_annotate(अचर वर्ण *zVfs, अचर वर्ण *z_सन्देश){
  sqlite3_vfs *pVfs;
  pVfs = sqlite3_vfs_find(zVfs);
  यदि( !pVfs || pVfs->xOpen!=vfslogOpen ){
    फर्काऊ SQLITE_ERROR;
  } 
  vfslog_call(pVfs, OS_ANNOTATE, 0, 0, 0, 0, 0);
  vfslog_string(pVfs, z_सन्देश);
  फर्काऊ SQLITE_OK;
}

स्थिर अचर वर्ण *vfslog_eventname(पूर्णाङ्क eEvent){
  अचर वर्ण *zEvent = 0;

  विकल्प( eEvent ){
    पक्ष OS_CLOSE:             zEvent = "xClose"; रोक;
    पक्ष OS_READ:              zEvent = "xRead"; रोक;
    पक्ष OS_WRITE:             zEvent = "xWrite"; रोक;
    पक्ष OS_TRUNCATE:          zEvent = "xTruncate"; रोक;
    पक्ष OS_SYNC:              zEvent = "xSync"; रोक;
    पक्ष OS_FILESIZE:          zEvent = "xFilesize"; रोक;
    पक्ष OS_LOCK:              zEvent = "xLock"; रोक;
    पक्ष OS_UNLOCK:            zEvent = "xUnlock"; रोक;
    पक्ष OS_CHECKRESERVEDLOCK: zEvent = "xCheckResLock"; रोक;
    पक्ष OS_FILECONTROL:       zEvent = "xFileControl"; रोक;
    पक्ष OS_SECTORSIZE:        zEvent = "xSectorSize"; रोक;
    पक्ष OS_DEVCHAR:           zEvent = "xDeviceChar"; रोक;
    पक्ष OS_OPEN:              zEvent = "xOpen"; रोक;
    पक्ष OS_DELETE:            zEvent = "xDelete"; रोक;
    पक्ष OS_ACCESS:            zEvent = "xAccess"; रोक;
    पक्ष OS_FULLPATHNAME:      zEvent = "xFullPathname"; रोक;
    पक्ष OS_RANDOMNESS:        zEvent = "xRandomness"; रोक;
    पक्ष OS_SLEEP:             zEvent = "xSleep"; रोक;
    पक्ष OS_CURRENTTIME:       zEvent = "xCurrentTime"; रोक;

    पक्ष OS_SHMUNMAP:          zEvent = "xShmUnmap"; रोक;
    पक्ष OS_SHMLOCK:           zEvent = "xShmLock"; रोक;
    पक्ष OS_SHMBARRIER:        zEvent = "xShmBarrier"; रोक;
    पक्ष OS_SHMMAP:            zEvent = "xShmMap"; रोक;

    पक्ष OS_ANNOTATE:          zEvent = "annotation"; रोक;
  }

  फर्काऊ zEvent;
}

प्रकारनिर्देश संरचना VfslogVtab VfslogVtab;
प्रकारनिर्देश संरचना VfslogCsr VfslogCsr;

/*
** Virtual table type for the vfslog reader module.
*/
संरचना VfslogVtab {
  sqlite3_vtab base;              /* Base class */
  sqlite3_file *pFd;              /* File descriptor open on vfslog file */
  sqlite3_int64 nByte;            /* Size of file in bytes */
  वर्ण *zFile;                    /* File name for pFd */
};

/*
** Virtual table cursor type for the vfslog reader module.
*/
संरचना VfslogCsr {
  sqlite3_vtab_cursor base;       /* Base class */
  sqlite3_int64 iRowid;           /* Current rowid. */
  sqlite3_int64 iOffset;          /* Offset of next record in file */
  वर्ण *zTransient;               /* Transient 'file' string */
  पूर्णाङ्क nFile;                      /* Size of array azFile[] */
  वर्ण **azFile;                  /* File strings */
  अचिन्हित वर्ण aBuf[1024];       /* Current vfs log entry (read from file) */
};

स्थिर अचिन्हित पूर्णाङ्क get32bits(अचिन्हित वर्ण *p){
  फर्काऊ (p[0]<<24) + (p[1]<<16) + (p[2]<<8) + p[3];
}

/*
** The argument must point to a buffer containing a nul-terminated string.
** If the string begins with an SQL quote character it is overwritten by
** the dequoted version. Otherwise the buffer is left unmodified.
*/
स्थिर शून्य dequote(वर्ण *z){
  वर्ण quote;                     /* Quote character (if any ) */
  quote = z[0];
  यदि( quote=='[' || quote=='\'' || quote=='"' || quote=='`' ){
    पूर्णाङ्क iIn = 1;                  /* Index of next byte to read from input */
    पूर्णाङ्क iOut = 0;                 /* Index of next byte to write to output */
    यदि( quote=='[' ) quote = ']';  
    जबसम्म( z[iIn] ){
      यदि( z[iIn]==quote ){
        यदि( z[iIn+1]!=quote ) रोक;
        z[iOut++] = quote;
        iIn += 2;
      }अन्यथा{
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
स्थिर पूर्णाङ्क vlogConnect(
  sqlite3 *db,
  शून्य *pAux,
  पूर्णाङ्क argc, अचर वर्ण *अचर*argv,
  sqlite3_vtab **ppVtab,
  वर्ण **pzErr
){
  sqlite3_vfs *pVfs;              /* VFS used to read log file */
  पूर्णाङ्क flags;                      /* flags passed to pVfs->xOpen() */
  VfslogVtab *p;
  पूर्णाङ्क rc;
  पूर्णाङ्क nByte;
  वर्ण *zFile;

  *ppVtab = 0;
  pVfs = sqlite3_vfs_find(0);
  nByte = परिमाण(VfslogVtab) + pVfs->szOsFile + pVfs->mxPathname;
  p = sqlite3_malloc(nByte);
  यदि( p==0 ) फर्काऊ SQLITE_NOMEM;
  स_रखो(p, 0, nByte);

  p->pFd = (sqlite3_file *)&p[1];
  p->zFile = &((वर्ण *)p->pFd)[pVfs->szOsFile];

  zFile = sqlite3_mprintf("%s", argv[3]);
  यदि( !zFile ){
    sqlite3_free(p);
    फर्काऊ SQLITE_NOMEM;
  }
  dequote(zFile);
  pVfs->xFullPathname(pVfs, zFile, pVfs->mxPathname, p->zFile);
  sqlite3_free(zFile);

  flags = SQLITE_OPEN_READWRITE|SQLITE_OPEN_SUPER_JOURNAL;
  rc = pVfs->xOpen(pVfs, p->zFile, p->pFd, flags, &flags);

  यदि( rc==SQLITE_OK ){
    p->pFd->pMethods->xFileSize(p->pFd, &p->nByte);
    sqlite3_declare_vtab(db, 
        "CREATE TABLE xxx(event, file, click, rc, size, offset)"
    );
    *ppVtab = &p->base;
  }अन्यथा{
    sqlite3_free(p);
  }

  फर्काऊ rc;
}

/*
** There is no "best-index". This virtual table always does a linear
** scan of the binary VFS log file.
*/
स्थिर पूर्णाङ्क vlogBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  pIdxInfo->estimatedCost = 10.0;
  फर्काऊ SQLITE_OK;
}

/*
** Disconnect from or destroy a vfslog virtual table.
*/
स्थिर पूर्णाङ्क vlogDisconnect(sqlite3_vtab *pVtab){
  VfslogVtab *p = (VfslogVtab *)pVtab;
  यदि( p->pFd->pMethods ){
    p->pFd->pMethods->xClose(p->pFd);
    p->pFd->pMethods = 0;
  }
  sqlite3_free(p);
  फर्काऊ SQLITE_OK;
}

/*
** Open a new vfslog cursor.
*/
स्थिर पूर्णाङ्क vlogOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  VfslogCsr *pCsr;                /* Newly allocated cursor object */

  pCsr = sqlite3_malloc(परिमाण(VfslogCsr));
  यदि( !pCsr ) फर्काऊ SQLITE_NOMEM;
  स_रखो(pCsr, 0, परिमाण(VfslogCsr));
  *ppCursor = &pCsr->base;
  फर्काऊ SQLITE_OK;
}

/*
** Close a vfslog cursor.
*/
स्थिर पूर्णाङ्क vlogClose(sqlite3_vtab_cursor *pCursor){
  VfslogCsr *p = (VfslogCsr *)pCursor;
  पूर्णाङ्क i;
  लागि(i=0; i<p->nFile; i++){
    sqlite3_free(p->azFile[i]);
  }
  sqlite3_free(p->azFile);
  sqlite3_free(p->zTransient);
  sqlite3_free(p);
  फर्काऊ SQLITE_OK;
}

/*
** Move a vfslog cursor to the next entry in the file.
*/
स्थिर पूर्णाङ्क vlogNext(sqlite3_vtab_cursor *pCursor){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  VfslogVtab *p = (VfslogVtab *)pCursor->pVtab;
  पूर्णाङ्क rc = SQLITE_OK;
  पूर्णाङ्क nRead;

  sqlite3_free(pCsr->zTransient);
  pCsr->zTransient = 0;

  nRead = 24;
  यदि( pCsr->iOffset+nRead<=p->nByte ){
    पूर्णाङ्क eEvent;
    rc = p->pFd->pMethods->xRead(p->pFd, pCsr->aBuf, nRead, pCsr->iOffset);

    eEvent = get32bits(pCsr->aBuf);
    यदि( (rc==SQLITE_OK)
     && (eEvent==OS_OPEN || eEvent==OS_DELETE || eEvent==OS_ACCESS) 
    ){
      वर्ण buf[4];
      rc = p->pFd->pMethods->xRead(p->pFd, buf, 4, pCsr->iOffset+nRead);
      nRead += 4;
      यदि( rc==SQLITE_OK ){
        पूर्णाङ्क nStr = get32bits((अचिन्हित वर्ण *)buf);
        वर्ण *zStr = sqlite3_malloc(nStr+1);
        rc = p->pFd->pMethods->xRead(p->pFd, zStr, nStr, pCsr->iOffset+nRead);
        zStr[nStr] = '\0';
        nRead += nStr;

        यदि( eEvent==OS_OPEN ){
          पूर्णाङ्क iFileid = get32bits(&pCsr->aBuf[4]);
          यदि( iFileid>=pCsr->nFile ){
            पूर्णाङ्क nNew = परिमाण(pCsr->azFile[0])*(iFileid+1);
            pCsr->azFile = (वर्ण **)sqlite3_realloc(pCsr->azFile, nNew);
            nNew -= परिमाण(pCsr->azFile[0])*pCsr->nFile;
            स_रखो(&pCsr->azFile[pCsr->nFile], 0, nNew);
            pCsr->nFile = iFileid+1;
          }
          sqlite3_free(pCsr->azFile[iFileid]);
          pCsr->azFile[iFileid] = zStr;
        }अन्यथा{
          pCsr->zTransient = zStr;
        }
      }
    }
  }

  pCsr->iRowid += 1;
  pCsr->iOffset += nRead;
  फर्काऊ rc;
}

स्थिर पूर्णाङ्क vlogEof(sqlite3_vtab_cursor *pCursor){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  VfslogVtab *p = (VfslogVtab *)pCursor->pVtab;
  फर्काऊ (pCsr->iOffset>=p->nByte);
}

स्थिर पूर्णाङ्क vlogFilter(
  sqlite3_vtab_cursor *pCursor, 
  पूर्णाङ्क idx_अङ्क, अचर वर्ण *idxStr,
  पूर्णाङ्क argc, sqlite3_मान **argv
){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  pCsr->iRowid = 0;
  pCsr->iOffset = 20;
  फर्काऊ vlogNext(pCursor);
}

स्थिर पूर्णाङ्क vlogColumn(
  sqlite3_vtab_cursor *pCursor, 
  sqlite3_context *ctx, 
  पूर्णाङ्क i
){
  अचिन्हित पूर्णाङ्क val;
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;

  प्रतिज्ञा( i<7 );
  val = get32bits(&pCsr->aBuf[4*i]);

  विकल्प( i ){
    पक्ष 0: {
      sqlite3_result_text(ctx, vfslog_eventname(val), -1, SQLITE_STATIC);
      रोक;
    }
    पक्ष 1: {
      वर्ण *zStr = pCsr->zTransient;
      यदि( val!=0 && val<(अचिन्हित)pCsr->nFile ){
        zStr = pCsr->azFile[val];
      }
      sqlite3_result_text(ctx, zStr, -1, SQLITE_TRANSIENT);
      रोक;
    }
    स्वभाव:
      sqlite3_result_int(ctx, val);
      रोक;
  }

  फर्काऊ SQLITE_OK;
}

स्थिर पूर्णाङ्क vlogRowid(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  *pRowid = pCsr->iRowid;
  फर्काऊ SQLITE_OK;
}

पूर्णाङ्क sqlite3_vfslog_register(sqlite3 *db){
  स्थिर sqlite3_module vfslog_module = {
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
  फर्काऊ SQLITE_OK;
}
#endif /* SQLITE_OMIT_VIRTUALTABLE */

/**************************************************************************
***************************************************************************
** Tcl interface starts here.
*/

#यदि defined(SQLITE_TEST) || defined(TCLSH)

#अन्तर्भाव "tclsqlite.h"

स्थिर पूर्णाङ्क SQLITE_TCLAPI test_vfslog(
  शून्य *clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  संरचना SqliteDb { sqlite3 *db; };
  sqlite3 *db;
  Tcl_CmdInfo cmdInfo;
  पूर्णाङ्क rc = SQLITE_ERROR;

  स्थिर अचर वर्ण *strs[] = { "annotate", "finalize", "new",  "register", 0 };
  गणना VL_enum { VL_ANNOTATE, VL_FINALIZE, VL_NEW, VL_REGISTER };
  पूर्णाङ्क iSub;

  यदि( objc<2 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "SUB-COMMAND ...");
    फर्काऊ TCL_ERROR;
  }
  यदि( Tcl_GetIndexFromObj(interp, objv[1], strs, "sub-command", 0, &iSub) ){
    फर्काऊ TCL_ERROR;
  }

  विकल्प( (गणना VL_enum)iSub ){
    पक्ष VL_ANNOTATE: {
      वर्ण *zVfs;
      वर्ण *z_सन्देश;
      यदि( objc!=4 ){
        Tcl_Wrong_अङ्क_Args(interp, 3, objv, "VFS");
        फर्काऊ TCL_ERROR;
      }
      zVfs = Tcl_GetString(objv[2]);
      z_सन्देश = Tcl_GetString(objv[3]);
      rc = sqlite3_vfslog_annotate(zVfs, z_सन्देश);
      यदि( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp, "failed", (वर्ण*)0);
        फर्काऊ TCL_ERROR;
      }
      रोक;
    }
    पक्ष VL_FINALIZE: {
      वर्ण *zVfs;
      यदि( objc!=3 ){
        Tcl_Wrong_अङ्क_Args(interp, 2, objv, "VFS");
        फर्काऊ TCL_ERROR;
      }
      zVfs = Tcl_GetString(objv[2]);
      rc = sqlite3_vfslog_finalize(zVfs);
      यदि( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp, "failed", (वर्ण*)0);
        फर्काऊ TCL_ERROR;
      }
      रोक;
    };

    पक्ष VL_NEW: {
      वर्ण *zVfs;
      वर्ण *zParent;
      वर्ण *zLog;
      यदि( objc!=5 ){
        Tcl_Wrong_अङ्क_Args(interp, 2, objv, "VFS PARENT LOGFILE");
        फर्काऊ TCL_ERROR;
      }
      zVfs = Tcl_GetString(objv[2]);
      zParent = Tcl_GetString(objv[3]);
      zLog = Tcl_GetString(objv[4]);
      यदि( *zParent=='\0' ) zParent = 0;
      rc = sqlite3_vfslog_new(zVfs, zParent, zLog);
      यदि( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp, "failed", (वर्ण*)0);
        फर्काऊ TCL_ERROR;
      }
      रोक;
    };

    पक्ष VL_REGISTER: {
      वर्ण *zDb;
      यदि( objc!=3 ){
        Tcl_Wrong_अङ्क_Args(interp, 2, objv, "DB");
        फर्काऊ TCL_ERROR;
      }
#ifdef SQLITE_OMIT_VIRTUALTABLE
      Tcl_AppendResult(interp, "vfslog not available because of "
                               "SQLITE_OMIT_VIRTUALTABLE", (शून्य*)0);
      फर्काऊ TCL_ERROR;
#अन्यथा
      zDb = Tcl_GetString(objv[2]);
      यदि( Tcl_GetCommandInfo(interp, zDb, &cmdInfo) ){
        db = ((संरचना SqliteDb*)cmdInfo.objClientData)->db;
        rc = sqlite3_vfslog_register(db);
      }
      यदि( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp, "bad sqlite3 handle: ", zDb, (शून्य*)0);
        फर्काऊ TCL_ERROR;
      }
      रोक;
#endif
    }
  }

  फर्काऊ TCL_OK;
}

पूर्णाङ्क SqlitetestOsinst_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "vfslog", test_vfslog, 0, 0);
  फर्काऊ TCL_OK;
}

#endif /* SQLITE_TEST */
