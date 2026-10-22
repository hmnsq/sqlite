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

#समावेश "sqlite3.h"

#अगर_घोषित _WIN32
#समावेश <windows.h>
#पूर्ण_अगर

#समावेश <string.h>
#समावेश <assert.h>


/*
** Maximum pathname length supported by the vfslog backend.
*/
#घोषणा INST_MAX_PATHNAME 512

#घोषणा OS_ACCESS            1
#घोषणा OS_CHECKRESERVEDLOCK 2
#घोषणा OS_CLOSE             3
#घोषणा OS_CURRENTTIME       4
#घोषणा OS_DELETE            5
#घोषणा OS_DEVCHAR           6
#घोषणा OS_FILECONTROL       7
#घोषणा OS_FILESIZE          8
#घोषणा OS_FULLPATHNAME      9
#घोषणा OS_LOCK              11
#घोषणा OS_OPEN              12
#घोषणा OS_RANDOMNESS        13
#घोषणा OS_READ              14 
#घोषणा OS_SECTORSIZE        15
#घोषणा OS_SLEEP             16
#घोषणा OS_SYNC              17
#घोषणा OS_TRUNCATE          18
#घोषणा OS_UNLOCK            19
#घोषणा OS_WRITE             20
#घोषणा OS_SHMUNMAP          22
#घोषणा OS_SHMMAP            23
#घोषणा OS_SHMLOCK           25
#घोषणा OS_SHMBARRIER        26
#घोषणा OS_ANNOTATE          28

#घोषणा OS_NUMEVENTS         29

#घोषणा VFSLOG_BUFFERSIZE 8192

प्रकार काष्ठा VfslogVfs VfslogVfs;
प्रकार काष्ठा VfslogFile VfslogFile;

काष्ठा VfslogVfs {
  sqlite3_vfs base;               /* VFS methods */
  sqlite3_vfs *pVfs;              /* Parent VFS */
  पूर्णांक i_अग्र_File_Id;                /* Next file id */
  sqlite3_file *pLog;             /* Log file handle */
  sqlite3_int64 iOffset;          /* Log file offset of start of write buffer */
  पूर्णांक nBuf;                       /* Number of valid bytes in aBuf[] */
  अक्षर aBuf[VFSLOG_BUFFERSIZE];   /* Write buffer */
};

काष्ठा VfslogFile {
  sqlite3_file base;              /* IO methods */
  sqlite3_file *pReal;            /* Underlying file handle */
  sqlite3_vfs *pVfslog;           /* Associated VsflogVfs object */
  पूर्णांक iFileId;                    /* File id number */
};

#घोषणा REALVFS(p) (((VfslogVfs *)(p))->pVfs)



/*
** Method declarations for vfslog_file.
*/
अटल पूर्णांक vfslogClose(sqlite3_file*);
अटल पूर्णांक vfslogRead(sqlite3_file*, व्योम*, पूर्णांक iAmt, sqlite3_int64 iOfst);
अटल पूर्णांक vfslogWrite(sqlite3_file*,स्थिर व्योम*,पूर्णांक iAmt, sqlite3_int64 iOfst);
अटल पूर्णांक vfslogTruncate(sqlite3_file*, sqlite3_int64 आकार);
अटल पूर्णांक vfslogSync(sqlite3_file*, पूर्णांक flags);
अटल पूर्णांक vfslog_File_आकार(sqlite3_file*, sqlite3_int64 *p_आकार);
अटल पूर्णांक vfslogLock(sqlite3_file*, पूर्णांक);
अटल पूर्णांक vfslogUnlock(sqlite3_file*, पूर्णांक);
अटल पूर्णांक vfslogCheckReservedLock(sqlite3_file*, पूर्णांक *pResOut);
अटल पूर्णांक vfslogFileControl(sqlite3_file*, पूर्णांक op, व्योम *pArg);
अटल पूर्णांक vfslog_Sector_आकार(sqlite3_file*);
अटल पूर्णांक vfslogDeviceCharacteristics(sqlite3_file*);

अटल पूर्णांक vfslogShmLock(sqlite3_file *pFile, पूर्णांक ofst, पूर्णांक n, पूर्णांक flags);
अटल पूर्णांक vfslogShmMap(sqlite3_file *pFile,पूर्णांक,पूर्णांक,पूर्णांक,अस्थिर व्योम **);
अटल व्योम vfslogShmBarrier(sqlite3_file*);
अटल पूर्णांक vfslogShmUnmap(sqlite3_file *pFile, पूर्णांक deleteFlag);

/*
** Method declarations for vfslog_vfs.
*/
अटल पूर्णांक vfslogOpen(sqlite3_vfs*, स्थिर अक्षर *, sqlite3_file*, पूर्णांक , पूर्णांक *);
अटल पूर्णांक vfslogDelete(sqlite3_vfs*, स्थिर अक्षर *zName, पूर्णांक syncDir);
अटल पूर्णांक vfslogAccess(sqlite3_vfs*, स्थिर अक्षर *zName, पूर्णांक flags, पूर्णांक *);
अटल पूर्णांक vfslogFullPathname(sqlite3_vfs*, स्थिर अक्षर *zName, पूर्णांक, अक्षर *zOut);
अटल व्योम *vfslogDlOpen(sqlite3_vfs*, स्थिर अक्षर *zFilename);
अटल व्योम vfslogDlError(sqlite3_vfs*, पूर्णांक nByte, अक्षर *z_Err_संदेश);
अटल व्योम (*vfslogDlSym(sqlite3_vfs *pVfs, व्योम *p, स्थिर अक्षर*zSym))(व्योम);
अटल व्योम vfslogDlClose(sqlite3_vfs*, व्योम*);
अटल पूर्णांक vfslogRandomness(sqlite3_vfs*, पूर्णांक nByte, अक्षर *zOut);
अटल पूर्णांक vfslogSleep(sqlite3_vfs*, पूर्णांक microseconds);
अटल पूर्णांक vfslogCurrentTime(sqlite3_vfs*, द्विगुन*);

अटल पूर्णांक vfslogGetLastError(sqlite3_vfs*, पूर्णांक, अक्षर *);
अटल पूर्णांक vfslogCurrentTimeInt64(sqlite3_vfs*, sqlite3_int64*);

अटल sqlite3_vfs vfslog_vfs = {
  1,                              /* iVersion */
  माप(VfslogFile),             /* szOsFile */
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

अटल sqlite3_io_methods vfslog_io_methods = {
  2,                              /* iVersion */
  vfslogClose,                    /* xClose */
  vfslogRead,                     /* xRead */
  vfslogWrite,                    /* xWrite */
  vfslogTruncate,                 /* xTruncate */
  vfslogSync,                     /* xSync */
  vfslog_File_आकार,                 /* xFileSize */
  vfslogLock,                     /* xLock */
  vfslogUnlock,                   /* xUnlock */
  vfslogCheckReservedLock,        /* xCheckReservedLock */
  vfslogFileControl,              /* xFileControl */
  vfslog_Sector_आकार,               /* xSectorSize */
  vfslogDeviceCharacteristics,    /* xDeviceCharacteristics */
  vfslogShmMap,                   /* xShmMap */
  vfslogShmLock,                  /* xShmLock */
  vfslogShmBarrier,               /* xShmBarrier */
  vfslogShmUnmap                  /* xShmUnmap */
};

#अगर_घोषित _WIN32
#समावेश <time.h>
अटल sqlite3_uint64 vfslog_time(){
  FILETIME ft;
  sqlite3_uint64 u64time = 0;
 
  GetSystemTimeAsFileTime(&ft);

  u64time |= ft.dw_ऊपरी_Date_Time;
  u64time <<= 32;
  u64time |= ft.dw_निचला_Date_Time;

  /* ft is 100-nanosecond intervals, we want microseconds */
  वापस u64time /(sqlite3_uint64)10;
}
#या_अगर !defined(NO_GETTOD)
#समावेश <sys/time.h>
अटल sqlite3_uint64 vfslog_time(){
  काष्ठा timeval sTime;
  gettimeofday(&sTime, 0);
  वापस sTime.tv_usec + (sqlite3_uint64)sTime.tv_sec * 1000000;
}
#अन्यथा
अटल sqlite3_uint64 vfslog_time(){
  वापस 0;
}
#पूर्ण_अगर

अटल व्योम vfslog_call(sqlite3_vfs *, पूर्णांक, पूर्णांक, sqlite3_int64, पूर्णांक, पूर्णांक, पूर्णांक);
अटल व्योम vfslog_string(sqlite3_vfs *, स्थिर अक्षर *);

/*
** Close an vfslog-file.
*/
अटल पूर्णांक vfslogClose(sqlite3_file *pFile){
  sqlite3_uint64 t;
  पूर्णांक rc = SQLITE_OK;
  VfslogFile *p = (VfslogFile *)pFile;

  t = vfslog_time();
  अगर( p->pReal->pMethods ){
    rc = p->pReal->pMethods->xClose(p->pReal);
  }
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_CLOSE, p->iFileId, t, rc, 0, 0);
  वापस rc;
}

/*
** Read data from an vfslog-file.
*/
अटल पूर्णांक vfslogRead(
  sqlite3_file *pFile, 
  व्योम *zBuf, 
  पूर्णांक iAmt, 
  sqlite_int64 iOfst
){
  पूर्णांक rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xRead(p->pReal, zBuf, iAmt, iOfst);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_READ, p->iFileId, t, rc, iAmt, (पूर्णांक)iOfst);
  वापस rc;
}

/*
** Write data to an vfslog-file.
*/
अटल पूर्णांक vfslogWrite(
  sqlite3_file *pFile,
  स्थिर व्योम *z,
  पूर्णांक iAmt,
  sqlite_int64 iOfst
){
  पूर्णांक rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xWrite(p->pReal, z, iAmt, iOfst);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_WRITE, p->iFileId, t, rc, iAmt, (पूर्णांक)iOfst);
  वापस rc;
}

/*
** Truncate an vfslog-file.
*/
अटल पूर्णांक vfslogTruncate(sqlite3_file *pFile, sqlite_int64 आकार){
  पूर्णांक rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xTruncate(p->pReal, आकार);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_TRUNCATE, p->iFileId, t, rc, 0, (पूर्णांक)आकार);
  वापस rc;
}

/*
** Sync an vfslog-file.
*/
अटल पूर्णांक vfslogSync(sqlite3_file *pFile, पूर्णांक flags){
  पूर्णांक rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xSync(p->pReal, flags);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SYNC, p->iFileId, t, rc, flags, 0);
  वापस rc;
}

/*
** Return the current file-size of an vfslog-file.
*/
अटल पूर्णांक vfslog_File_आकार(sqlite3_file *pFile, sqlite_int64 *p_आकार){
  पूर्णांक rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->x_File_आकार(p->pReal, p_आकार);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_FILESIZE, p->iFileId, t, rc, 0, (पूर्णांक)*p_आकार);
  वापस rc;
}

/*
** Lock an vfslog-file.
*/
अटल पूर्णांक vfslogLock(sqlite3_file *pFile, पूर्णांक eLock){
  पूर्णांक rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xLock(p->pReal, eLock);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_LOCK, p->iFileId, t, rc, eLock, 0);
  वापस rc;
}

/*
** Unlock an vfslog-file.
*/
अटल पूर्णांक vfslogUnlock(sqlite3_file *pFile, पूर्णांक eLock){
  पूर्णांक rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xUnlock(p->pReal, eLock);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_UNLOCK, p->iFileId, t, rc, eLock, 0);
  वापस rc;
}

/*
** Check if another file-handle holds a RESERVED lock on an vfslog-file.
*/
अटल पूर्णांक vfslogCheckReservedLock(sqlite3_file *pFile, पूर्णांक *pResOut){
  पूर्णांक rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xCheckReservedLock(p->pReal, pResOut);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_CHECKRESERVEDLOCK, p->iFileId, t, rc, *pResOut, 0);
  वापस rc;
}

/*
** File control method. For custom operations on an vfslog-file.
*/
अटल पूर्णांक vfslogFileControl(sqlite3_file *pFile, पूर्णांक op, व्योम *pArg){
  VfslogFile *p = (VfslogFile *)pFile;
  पूर्णांक rc = p->pReal->pMethods->xFileControl(p->pReal, op, pArg);
  अगर( op==SQLITE_FCNTL_VFSNAME && rc==SQLITE_OK ){
    *(अक्षर**)pArg = sqlite3_mprintf("vfslog/%z", *(अक्षर**)pArg);
  }
  वापस rc;
}

/*
** Return the sector-size in bytes for an vfslog-file.
*/
अटल पूर्णांक vfslog_Sector_आकार(sqlite3_file *pFile){
  पूर्णांक rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->x_Sector_आकार(p->pReal);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SECTORSIZE, p->iFileId, t, rc, 0, 0);
  वापस rc;
}

/*
** Return the device characteristic flags supported by an vfslog-file.
*/
अटल पूर्णांक vfslogDeviceCharacteristics(sqlite3_file *pFile){
  पूर्णांक rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xDeviceCharacteristics(p->pReal);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_DEVCHAR, p->iFileId, t, rc, 0, 0);
  वापस rc;
}

अटल पूर्णांक vfslogShmLock(sqlite3_file *pFile, पूर्णांक ofst, पूर्णांक n, पूर्णांक flags){
  पूर्णांक rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xShmLock(p->pReal, ofst, n, flags);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SHMLOCK, p->iFileId, t, rc, 0, 0);
  वापस rc;
}
अटल पूर्णांक vfslogShmMap(
  sqlite3_file *pFile, 
  पूर्णांक iRegion, 
  पूर्णांक szRegion, 
  पूर्णांक isWrite, 
  अस्थिर व्योम **pp
){
  पूर्णांक rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xShmMap(p->pReal, iRegion, szRegion, isWrite, pp);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SHMMAP, p->iFileId, t, rc, 0, 0);
  वापस rc;
}
अटल व्योम vfslogShmBarrier(sqlite3_file *pFile){
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  p->pReal->pMethods->xShmBarrier(p->pReal);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SHMBARRIER, p->iFileId, t, SQLITE_OK, 0, 0);
}
अटल पूर्णांक vfslogShmUnmap(sqlite3_file *pFile, पूर्णांक deleteFlag){
  पूर्णांक rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xShmUnmap(p->pReal, deleteFlag);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SHMUNMAP, p->iFileId, t, rc, 0, 0);
  वापस rc;
}


/*
** Open an vfslog file handle.
*/
अटल पूर्णांक vfslogOpen(
  sqlite3_vfs *pVfs,
  स्थिर अक्षर *zName,
  sqlite3_file *pFile,
  पूर्णांक flags,
  पूर्णांक *pOutFlags
){
  पूर्णांक rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  VfslogVfs *pLog = (VfslogVfs *)pVfs;

  pFile->pMethods = &vfslog_io_methods;
  p->pReal = (sqlite3_file *)&p[1];
  p->pVfslog = pVfs;
  p->iFileId = ++pLog->i_अग्र_File_Id;

  t = vfslog_time();
  rc = REALVFS(pVfs)->xOpen(REALVFS(pVfs), zName, p->pReal, flags, pOutFlags);
  t = vfslog_time() - t;

  vfslog_call(pVfs, OS_OPEN, p->iFileId, t, rc, 0, 0);
  vfslog_string(pVfs, zName);
  वापस rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
अटल पूर्णांक vfslogDelete(sqlite3_vfs *pVfs, स्थिर अक्षर *zPath, पूर्णांक dirSync){
  पूर्णांक rc;
  sqlite3_uint64 t;
  t = vfslog_time();
  rc = REALVFS(pVfs)->xDelete(REALVFS(pVfs), zPath, dirSync);
  t = vfslog_time() - t;
  vfslog_call(pVfs, OS_DELETE, 0, t, rc, dirSync, 0);
  vfslog_string(pVfs, zPath);
  वापस rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
अटल पूर्णांक vfslogAccess(
  sqlite3_vfs *pVfs, 
  स्थिर अक्षर *zPath, 
  पूर्णांक flags, 
  पूर्णांक *pResOut
){
  पूर्णांक rc;
  sqlite3_uint64 t;
  t = vfslog_time();
  rc = REALVFS(pVfs)->xAccess(REALVFS(pVfs), zPath, flags, pResOut);
  t = vfslog_time() - t;
  vfslog_call(pVfs, OS_ACCESS, 0, t, rc, flags, *pResOut);
  vfslog_string(pVfs, zPath);
  वापस rc;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
अटल पूर्णांक vfslogFullPathname(
  sqlite3_vfs *pVfs, 
  स्थिर अक्षर *zPath, 
  पूर्णांक nOut, 
  अक्षर *zOut
){
  वापस REALVFS(pVfs)->xFullPathname(REALVFS(pVfs), zPath, nOut, zOut);
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
अटल व्योम *vfslogDlOpen(sqlite3_vfs *pVfs, स्थिर अक्षर *zPath){
  वापस REALVFS(pVfs)->xDlOpen(REALVFS(pVfs), zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
अटल व्योम vfslogDlError(sqlite3_vfs *pVfs, पूर्णांक nByte, अक्षर *z_Err_संदेश){
  REALVFS(pVfs)->xDlError(REALVFS(pVfs), nByte, z_Err_संदेश);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
अटल व्योम (*vfslogDlSym(sqlite3_vfs *pVfs, व्योम *p, स्थिर अक्षर *zSym))(व्योम){
  वापस REALVFS(pVfs)->xDlSym(REALVFS(pVfs), p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
अटल व्योम vfslogDlClose(sqlite3_vfs *pVfs, व्योम *pHandle){
  REALVFS(pVfs)->xDlClose(REALVFS(pVfs), pHandle);
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
अटल पूर्णांक vfslogRandomness(sqlite3_vfs *pVfs, पूर्णांक nByte, अक्षर *zBufOut){
  वापस REALVFS(pVfs)->xRandomness(REALVFS(pVfs), nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
अटल पूर्णांक vfslogSleep(sqlite3_vfs *pVfs, पूर्णांक nMicro){
  वापस REALVFS(pVfs)->xSleep(REALVFS(pVfs), nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
अटल पूर्णांक vfslogCurrentTime(sqlite3_vfs *pVfs, द्विगुन *pTimeOut){
  वापस REALVFS(pVfs)->xCurrentTime(REALVFS(pVfs), pTimeOut);
}

अटल पूर्णांक vfslogGetLastError(sqlite3_vfs *pVfs, पूर्णांक a, अक्षर *b){
  वापस REALVFS(pVfs)->xGetLastError(REALVFS(pVfs), a, b);
}
अटल पूर्णांक vfslogCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *p){
  वापस REALVFS(pVfs)->xCurrentTimeInt64(REALVFS(pVfs), p);
}

अटल व्योम vfslog_flush(VfslogVfs *p){
#अगर_घोषित SQLITE_TEST
  बाह्य पूर्णांक sqlite3_io_error_pending;
  बाह्य पूर्णांक sqlite3_io_error_persist;
  बाह्य पूर्णांक sqlite3_diskfull_pending;

  पूर्णांक pending = sqlite3_io_error_pending;
  पूर्णांक persist = sqlite3_io_error_persist;
  पूर्णांक diskfull = sqlite3_diskfull_pending;

  sqlite3_io_error_pending = 0;
  sqlite3_io_error_persist = 0;
  sqlite3_diskfull_pending = 0;
#पूर्ण_अगर

  अगर( p->nBuf ){
    p->pLog->pMethods->xWrite(p->pLog, p->aBuf, p->nBuf, p->iOffset);
    p->iOffset += p->nBuf;
    p->nBuf = 0;
  }

#अगर_घोषित SQLITE_TEST
  sqlite3_io_error_pending = pending;
  sqlite3_io_error_persist = persist;
  sqlite3_diskfull_pending = diskfull;
#पूर्ण_अगर
}

अटल व्योम put32bits(अचिन्हित अक्षर *p, अचिन्हित पूर्णांक v){
  p[0] = v>>24;
  p[1] = (अचिन्हित अक्षर)(v>>16);
  p[2] = (अचिन्हित अक्षर)(v>>8);
  p[3] = (अचिन्हित अक्षर)v;
}

अटल व्योम vfslog_call(
  sqlite3_vfs *pVfs,
  पूर्णांक eEvent,
  पूर्णांक iFileid,
  sqlite3_int64 nClick,
  पूर्णांक return_code,
  पूर्णांक आकार,
  पूर्णांक offset
){
  VfslogVfs *p = (VfslogVfs *)pVfs;
  अचिन्हित अक्षर *zRec;
  अगर( (24+p->nBuf)>माप(p->aBuf) ){
    vfslog_flush(p);
  }
  zRec = (अचिन्हित अक्षर *)&p->aBuf[p->nBuf];
  put32bits(&zRec[0], eEvent);
  put32bits(&zRec[4], iFileid);
  put32bits(&zRec[8], (अचिन्हित पूर्णांक)(nClick&0xffff));
  put32bits(&zRec[12], return_code);
  put32bits(&zRec[16], आकार);
  put32bits(&zRec[20], offset);
  p->nBuf += 24;
}

अटल व्योम vfslog_string(sqlite3_vfs *pVfs, स्थिर अक्षर *zStr){
  VfslogVfs *p = (VfslogVfs *)pVfs;
  अचिन्हित अक्षर *zRec;
  पूर्णांक nStr = zStr ? (पूर्णांक)म_माप(zStr) : 0;
  अगर( (4+nStr+p->nBuf)>माप(p->aBuf) ){
    vfslog_flush(p);
  }
  zRec = (अचिन्हित अक्षर *)&p->aBuf[p->nBuf];
  put32bits(&zRec[0], nStr);
  अगर( zStr ){
    स_नकल(&zRec[4], zStr, nStr);
  }
  p->nBuf += (4 + nStr);
}

अटल व्योम vfslog_finalize(VfslogVfs *p){
  अगर( p->pLog->pMethods ){
    vfslog_flush(p);
    p->pLog->pMethods->xClose(p->pLog);
  }
  sqlite3_free(p);
}

पूर्णांक sqlite3_vfslog_finalize(स्थिर अक्षर *zVfs){
  sqlite3_vfs *pVfs;
  pVfs = sqlite3_vfs_find(zVfs);
  अगर( !pVfs || pVfs->xOpen!=vfslogOpen ){
    वापस SQLITE_ERROR;
  } 
  sqlite3_vfs_unregister(pVfs);
  vfslog_finalize((VfslogVfs *)pVfs);
  वापस SQLITE_OK;
}

पूर्णांक sqlite3_vfslog_new(
  स्थिर अक्षर *zVfs,               /* New VFS name */
  स्थिर अक्षर *zParentVfs,         /* Parent VFS name (or NULL) */
  स्थिर अक्षर *zLog                /* Log file name */
){
  VfslogVfs *p;
  sqlite3_vfs *pParent;
  पूर्णांक nByte;
  पूर्णांक flags;
  पूर्णांक rc;
  अक्षर *zFile;
  पूर्णांक nVfs;

  pParent = sqlite3_vfs_find(zParentVfs);
  अगर( !pParent ){
    वापस SQLITE_ERROR;
  }

  nVfs = (पूर्णांक)म_माप(zVfs);
  nByte = माप(VfslogVfs) + pParent->szOsFile + nVfs+1+pParent->mxPathname+1;
  p = (VfslogVfs *)sqlite3_malloc(nByte);
  स_रखो(p, 0, nByte);

  p->pVfs = pParent;
  p->pLog = (sqlite3_file *)&p[1];
  स_नकल(&p->base, &vfslog_vfs, माप(sqlite3_vfs));
  p->base.zName = &((अक्षर *)p->pLog)[pParent->szOsFile];
  p->base.szOsFile += pParent->szOsFile;
  स_नकल((अक्षर *)p->base.zName, zVfs, nVfs);

  zFile = (अक्षर *)&p->base.zName[nVfs+1];
  pParent->xFullPathname(pParent, zLog, pParent->mxPathname, zFile);

  flags = SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE|SQLITE_OPEN_SUPER_JOURNAL;
  pParent->xDelete(pParent, zFile, 0);
  rc = pParent->xOpen(pParent, zFile, p->pLog, flags, &flags);
  अगर( rc==SQLITE_OK ){
    स_नकल(p->aBuf, "sqlite_ostrace1.....", 20);
    p->iOffset = 0;
    p->nBuf = 20;
    rc = sqlite3_vfs_register((sqlite3_vfs *)p, 1);
  }
  अगर( rc ){
    vfslog_finalize(p);
  }
  वापस rc;
}

पूर्णांक sqlite3_vfslog_annotate(स्थिर अक्षर *zVfs, स्थिर अक्षर *z_संदेश){
  sqlite3_vfs *pVfs;
  pVfs = sqlite3_vfs_find(zVfs);
  अगर( !pVfs || pVfs->xOpen!=vfslogOpen ){
    वापस SQLITE_ERROR;
  } 
  vfslog_call(pVfs, OS_ANNOTATE, 0, 0, 0, 0, 0);
  vfslog_string(pVfs, z_संदेश);
  वापस SQLITE_OK;
}

अटल स्थिर अक्षर *vfslog_eventname(पूर्णांक eEvent){
  स्थिर अक्षर *zEvent = 0;

  चयन( eEvent ){
    हाल OS_CLOSE:             zEvent = "xClose"; अवरोध;
    हाल OS_READ:              zEvent = "xRead"; अवरोध;
    हाल OS_WRITE:             zEvent = "xWrite"; अवरोध;
    हाल OS_TRUNCATE:          zEvent = "xTruncate"; अवरोध;
    हाल OS_SYNC:              zEvent = "xSync"; अवरोध;
    हाल OS_FILESIZE:          zEvent = "xFilesize"; अवरोध;
    हाल OS_LOCK:              zEvent = "xLock"; अवरोध;
    हाल OS_UNLOCK:            zEvent = "xUnlock"; अवरोध;
    हाल OS_CHECKRESERVEDLOCK: zEvent = "xCheckResLock"; अवरोध;
    हाल OS_FILECONTROL:       zEvent = "xFileControl"; अवरोध;
    हाल OS_SECTORSIZE:        zEvent = "xSectorSize"; अवरोध;
    हाल OS_DEVCHAR:           zEvent = "xDeviceChar"; अवरोध;
    हाल OS_OPEN:              zEvent = "xOpen"; अवरोध;
    हाल OS_DELETE:            zEvent = "xDelete"; अवरोध;
    हाल OS_ACCESS:            zEvent = "xAccess"; अवरोध;
    हाल OS_FULLPATHNAME:      zEvent = "xFullPathname"; अवरोध;
    हाल OS_RANDOMNESS:        zEvent = "xRandomness"; अवरोध;
    हाल OS_SLEEP:             zEvent = "xSleep"; अवरोध;
    हाल OS_CURRENTTIME:       zEvent = "xCurrentTime"; अवरोध;

    हाल OS_SHMUNMAP:          zEvent = "xShmUnmap"; अवरोध;
    हाल OS_SHMLOCK:           zEvent = "xShmLock"; अवरोध;
    हाल OS_SHMBARRIER:        zEvent = "xShmBarrier"; अवरोध;
    हाल OS_SHMMAP:            zEvent = "xShmMap"; अवरोध;

    हाल OS_ANNOTATE:          zEvent = "annotation"; अवरोध;
  }

  वापस zEvent;
}

प्रकार काष्ठा VfslogVtab VfslogVtab;
प्रकार काष्ठा VfslogCsr VfslogCsr;

/*
** Virtual table type for the vfslog reader module.
*/
काष्ठा VfslogVtab {
  sqlite3_vtab base;              /* Base class */
  sqlite3_file *pFd;              /* File descriptor open on vfslog file */
  sqlite3_int64 nByte;            /* Size of file in bytes */
  अक्षर *zFile;                    /* File name for pFd */
};

/*
** Virtual table cursor type for the vfslog reader module.
*/
काष्ठा VfslogCsr {
  sqlite3_vtab_cursor base;       /* Base class */
  sqlite3_int64 iRowid;           /* Current rowid. */
  sqlite3_int64 iOffset;          /* Offset of next record in file */
  अक्षर *zTransient;               /* Transient 'file' string */
  पूर्णांक nFile;                      /* Size of array azFile[] */
  अक्षर **azFile;                  /* File strings */
  अचिन्हित अक्षर aBuf[1024];       /* Current vfs log entry (read from file) */
};

अटल अचिन्हित पूर्णांक get32bits(अचिन्हित अक्षर *p){
  वापस (p[0]<<24) + (p[1]<<16) + (p[2]<<8) + p[3];
}

/*
** The argument must point to a buffer containing a nul-terminated string.
** If the string begins with an SQL quote character it is overwritten by
** the dequoted version. Otherwise the buffer is left unmodified.
*/
अटल व्योम dequote(अक्षर *z){
  अक्षर quote;                     /* Quote character (if any ) */
  quote = z[0];
  अगर( quote=='[' || quote=='\'' || quote=='"' || quote=='`' ){
    पूर्णांक iIn = 1;                  /* Index of next byte to read from input */
    पूर्णांक iOut = 0;                 /* Index of next byte to write to output */
    अगर( quote=='[' ) quote = ']';  
    जबतक( z[iIn] ){
      अगर( z[iIn]==quote ){
        अगर( z[iIn+1]!=quote ) अवरोध;
        z[iOut++] = quote;
        iIn += 2;
      }अन्यथा{
        z[iOut++] = z[iIn++];
      }
    }
    z[iOut] = '\0';
  }
}

#अगर_अघोषित SQLITE_OMIT_VIRTUALTABLE
/*
** Connect to or create a vfslog virtual table.
*/
अटल पूर्णांक vlogConnect(
  sqlite3 *db,
  व्योम *pAux,
  पूर्णांक argc, स्थिर अक्षर *स्थिर*argv,
  sqlite3_vtab **ppVtab,
  अक्षर **pzErr
){
  sqlite3_vfs *pVfs;              /* VFS used to read log file */
  पूर्णांक flags;                      /* flags passed to pVfs->xOpen() */
  VfslogVtab *p;
  पूर्णांक rc;
  पूर्णांक nByte;
  अक्षर *zFile;

  *ppVtab = 0;
  pVfs = sqlite3_vfs_find(0);
  nByte = माप(VfslogVtab) + pVfs->szOsFile + pVfs->mxPathname;
  p = sqlite3_malloc(nByte);
  अगर( p==0 ) वापस SQLITE_NOMEM;
  स_रखो(p, 0, nByte);

  p->pFd = (sqlite3_file *)&p[1];
  p->zFile = &((अक्षर *)p->pFd)[pVfs->szOsFile];

  zFile = sqlite3_mprintf("%s", argv[3]);
  अगर( !zFile ){
    sqlite3_free(p);
    वापस SQLITE_NOMEM;
  }
  dequote(zFile);
  pVfs->xFullPathname(pVfs, zFile, pVfs->mxPathname, p->zFile);
  sqlite3_free(zFile);

  flags = SQLITE_OPEN_READWRITE|SQLITE_OPEN_SUPER_JOURNAL;
  rc = pVfs->xOpen(pVfs, p->zFile, p->pFd, flags, &flags);

  अगर( rc==SQLITE_OK ){
    p->pFd->pMethods->x_File_आकार(p->pFd, &p->nByte);
    sqlite3_declare_vtab(db, 
        "CREATE TABLE xxx(event, file, click, rc, size, offset)"
    );
    *ppVtab = &p->base;
  }अन्यथा{
    sqlite3_free(p);
  }

  वापस rc;
}

/*
** There is no "best-index". This virtual table always does a linear
** scan of the binary VFS log file.
*/
अटल पूर्णांक vlog_Best_सूचकांक(sqlite3_vtab *tab, sqlite3_सूचकांक_info *pIdxInfo){
  pIdxInfo->estimatedCost = 10.0;
  वापस SQLITE_OK;
}

/*
** Disconnect from or destroy a vfslog virtual table.
*/
अटल पूर्णांक vlogDisconnect(sqlite3_vtab *pVtab){
  VfslogVtab *p = (VfslogVtab *)pVtab;
  अगर( p->pFd->pMethods ){
    p->pFd->pMethods->xClose(p->pFd);
    p->pFd->pMethods = 0;
  }
  sqlite3_free(p);
  वापस SQLITE_OK;
}

/*
** Open a new vfslog cursor.
*/
अटल पूर्णांक vlogOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  VfslogCsr *pCsr;                /* Newly allocated cursor object */

  pCsr = sqlite3_malloc(माप(VfslogCsr));
  अगर( !pCsr ) वापस SQLITE_NOMEM;
  स_रखो(pCsr, 0, माप(VfslogCsr));
  *ppCursor = &pCsr->base;
  वापस SQLITE_OK;
}

/*
** Close a vfslog cursor.
*/
अटल पूर्णांक vlogClose(sqlite3_vtab_cursor *pCursor){
  VfslogCsr *p = (VfslogCsr *)pCursor;
  पूर्णांक i;
  क्रम(i=0; i<p->nFile; i++){
    sqlite3_free(p->azFile[i]);
  }
  sqlite3_free(p->azFile);
  sqlite3_free(p->zTransient);
  sqlite3_free(p);
  वापस SQLITE_OK;
}

/*
** Move a vfslog cursor to the next entry in the file.
*/
अटल पूर्णांक vlog_अग्र(sqlite3_vtab_cursor *pCursor){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  VfslogVtab *p = (VfslogVtab *)pCursor->pVtab;
  पूर्णांक rc = SQLITE_OK;
  पूर्णांक nRead;

  sqlite3_free(pCsr->zTransient);
  pCsr->zTransient = 0;

  nRead = 24;
  अगर( pCsr->iOffset+nRead<=p->nByte ){
    पूर्णांक eEvent;
    rc = p->pFd->pMethods->xRead(p->pFd, pCsr->aBuf, nRead, pCsr->iOffset);

    eEvent = get32bits(pCsr->aBuf);
    अगर( (rc==SQLITE_OK)
     && (eEvent==OS_OPEN || eEvent==OS_DELETE || eEvent==OS_ACCESS) 
    ){
      अक्षर buf[4];
      rc = p->pFd->pMethods->xRead(p->pFd, buf, 4, pCsr->iOffset+nRead);
      nRead += 4;
      अगर( rc==SQLITE_OK ){
        पूर्णांक nStr = get32bits((अचिन्हित अक्षर *)buf);
        अक्षर *zStr = sqlite3_malloc(nStr+1);
        rc = p->pFd->pMethods->xRead(p->pFd, zStr, nStr, pCsr->iOffset+nRead);
        zStr[nStr] = '\0';
        nRead += nStr;

        अगर( eEvent==OS_OPEN ){
          पूर्णांक iFileid = get32bits(&pCsr->aBuf[4]);
          अगर( iFileid>=pCsr->nFile ){
            पूर्णांक nNew = माप(pCsr->azFile[0])*(iFileid+1);
            pCsr->azFile = (अक्षर **)sqlite3_realloc(pCsr->azFile, nNew);
            nNew -= माप(pCsr->azFile[0])*pCsr->nFile;
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
  वापस rc;
}

अटल पूर्णांक vlogEof(sqlite3_vtab_cursor *pCursor){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  VfslogVtab *p = (VfslogVtab *)pCursor->pVtab;
  वापस (pCsr->iOffset>=p->nByte);
}

अटल पूर्णांक vlogFilter(
  sqlite3_vtab_cursor *pCursor, 
  पूर्णांक idx_अंक, स्थिर अक्षर *idxStr,
  पूर्णांक argc, sqlite3_मूल्य **argv
){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  pCsr->iRowid = 0;
  pCsr->iOffset = 20;
  वापस vlog_अग्र(pCursor);
}

अटल पूर्णांक vlogColumn(
  sqlite3_vtab_cursor *pCursor, 
  sqlite3_context *ctx, 
  पूर्णांक i
){
  अचिन्हित पूर्णांक val;
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;

  निश्चित( i<7 );
  val = get32bits(&pCsr->aBuf[4*i]);

  चयन( i ){
    हाल 0: {
      sqlite3_परिणाम_text(ctx, vfslog_eventname(val), -1, SQLITE_STATIC);
      अवरोध;
    }
    हाल 1: {
      अक्षर *zStr = pCsr->zTransient;
      अगर( val!=0 && val<(अचिन्हित)pCsr->nFile ){
        zStr = pCsr->azFile[val];
      }
      sqlite3_परिणाम_text(ctx, zStr, -1, SQLITE_TRANSIENT);
      अवरोध;
    }
    शेष:
      sqlite3_परिणाम_int(ctx, val);
      अवरोध;
  }

  वापस SQLITE_OK;
}

अटल पूर्णांक vlogRowid(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  *pRowid = pCsr->iRowid;
  वापस SQLITE_OK;
}

पूर्णांक sqlite3_vfslog_register(sqlite3 *db){
  अटल sqlite3_module vfslog_module = {
    0,                            /* iVersion */
    vlogConnect,                /* xCreate */
    vlogConnect,                /* xConnect */
    vlog_Best_सूचकांक,              /* xBestIndex */
    vlogDisconnect,             /* xDisconnect */
    vlogDisconnect,             /* xDestroy */
    vlogOpen,                   /* xOpen - open a cursor */
    vlogClose,                  /* xClose - close a cursor */
    vlogFilter,                 /* xFilter - configure scan constraints */
    vlog_अग्र,                   /* xNext - advance a cursor */
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
  वापस SQLITE_OK;
}
#पूर्ण_अगर /* SQLITE_OMIT_VIRTUALTABLE */

/**************************************************************************
***************************************************************************
** Tcl interface starts here.
*/

#अगर defined(SQLITE_TEST) || defined(TCLSH)

#समावेश "tclsqlite.h"

अटल पूर्णांक SQLITE_TCLAPI test_vfslog(
  व्योम *client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  काष्ठा SqliteDb { sqlite3 *db; };
  sqlite3 *db;
  Tcl_CmdInfo cmdInfo;
  पूर्णांक rc = SQLITE_ERROR;

  अटल स्थिर अक्षर *strs[] = { "annotate", "finalize", "new",  "register", 0 };
  क्रमागत VL_enum { VL_ANNOTATE, VL_FINALIZE, VL_NEW, VL_REGISTER };
  पूर्णांक iSub;

  अगर( objc<2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "SUB-COMMAND ...");
    वापस TCL_ERROR;
  }
  अगर( Tcl_Get_सूचकांक_From_Obj(interp, objv[1], strs, "sub-command", 0, &iSub) ){
    वापस TCL_ERROR;
  }

  चयन( (क्रमागत VL_enum)iSub ){
    हाल VL_ANNOTATE: {
      अक्षर *zVfs;
      अक्षर *z_संदेश;
      अगर( objc!=4 ){
        Tcl_Wrong_अंक_Args(interp, 3, objv, "VFS");
        वापस TCL_ERROR;
      }
      zVfs = Tcl_GetString(objv[2]);
      z_संदेश = Tcl_GetString(objv[3]);
      rc = sqlite3_vfslog_annotate(zVfs, z_संदेश);
      अगर( rc!=SQLITE_OK ){
        Tcl_Append_परिणाम(interp, "failed", (अक्षर*)0);
        वापस TCL_ERROR;
      }
      अवरोध;
    }
    हाल VL_FINALIZE: {
      अक्षर *zVfs;
      अगर( objc!=3 ){
        Tcl_Wrong_अंक_Args(interp, 2, objv, "VFS");
        वापस TCL_ERROR;
      }
      zVfs = Tcl_GetString(objv[2]);
      rc = sqlite3_vfslog_finalize(zVfs);
      अगर( rc!=SQLITE_OK ){
        Tcl_Append_परिणाम(interp, "failed", (अक्षर*)0);
        वापस TCL_ERROR;
      }
      अवरोध;
    };

    हाल VL_NEW: {
      अक्षर *zVfs;
      अक्षर *zParent;
      अक्षर *zLog;
      अगर( objc!=5 ){
        Tcl_Wrong_अंक_Args(interp, 2, objv, "VFS PARENT LOGFILE");
        वापस TCL_ERROR;
      }
      zVfs = Tcl_GetString(objv[2]);
      zParent = Tcl_GetString(objv[3]);
      zLog = Tcl_GetString(objv[4]);
      अगर( *zParent=='\0' ) zParent = 0;
      rc = sqlite3_vfslog_new(zVfs, zParent, zLog);
      अगर( rc!=SQLITE_OK ){
        Tcl_Append_परिणाम(interp, "failed", (अक्षर*)0);
        वापस TCL_ERROR;
      }
      अवरोध;
    };

    हाल VL_REGISTER: {
      अक्षर *zDb;
      अगर( objc!=3 ){
        Tcl_Wrong_अंक_Args(interp, 2, objv, "DB");
        वापस TCL_ERROR;
      }
#अगर_घोषित SQLITE_OMIT_VIRTUALTABLE
      Tcl_Append_परिणाम(interp, "vfslog not available because of "
                               "SQLITE_OMIT_VIRTUALTABLE", (व्योम*)0);
      वापस TCL_ERROR;
#अन्यथा
      zDb = Tcl_GetString(objv[2]);
      अगर( Tcl_GetCommandInfo(interp, zDb, &cmdInfo) ){
        db = ((काष्ठा SqliteDb*)cmdInfo.obj_Client_तथ्य)->db;
        rc = sqlite3_vfslog_register(db);
      }
      अगर( rc!=SQLITE_OK ){
        Tcl_Append_परिणाम(interp, "bad sqlite3 handle: ", zDb, (व्योम*)0);
        वापस TCL_ERROR;
      }
      अवरोध;
#पूर्ण_अगर
    }
  }

  वापस TCL_OK;
}

पूर्णांक SqlitetestOsinst_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "vfslog", test_vfslog, 0, 0);
  वापस TCL_OK;
}

#पूर्ण_अगर /* SQLITE_TEST */
