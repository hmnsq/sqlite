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

#यदि_परिभाषितम् _WIN32
#अन्तर्भाव <windows.h>
#यदि_अन्तः

#अन्तर्भाव <string.h>
#अन्तर्भाव <assert.h>


/*
** Maximum pathname length supported by the vfslog backend.
*/
#परिभाषय INST_MAX_PATHNAME 512

#परिभाषय OS_ACCESS            1
#परिभाषय OS_CHECKRESERVEDLOCK 2
#परिभाषय OS_CLOSE             3
#परिभाषय OS_CURRENTTIME       4
#परिभाषय OS_DELETE            5
#परिभाषय OS_DEVCHAR           6
#परिभाषय OS_FILECONTROL       7
#परिभाषय OS_FILESIZE          8
#परिभाषय OS_FULLPATHNAME      9
#परिभाषय OS_तालकम्              11
#परिभाषय OS_OPEN              12
#परिभाषय OS_RANDOMNESS        13
#परिभाषय OS_READ              14 
#परिभाषय OS_SECTORSIZE        15
#परिभाषय OS_SLEEP             16
#परिभाषय OS_SYNC              17
#परिभाषय OS_TRUNCATE          18
#परिभाषय OS_UNLOCK            19
#परिभाषय OS_WRITE             20
#परिभाषय OS_SHMUNMAP          22
#परिभाषय OS_SHMMAP            23
#परिभाषय OS_SHMLOCK           25
#परिभाषय OS_SHMBARRIER        26
#परिभाषय OS_ANNOTATE          28

#परिभाषय OS_NUMEVENTS         29

#परिभाषय VFSLOG_BUFFERSIZE 8192

प्रकारनिर्देश संरचना VfslogVfs VfslogVfs;
प्रकारनिर्देश संरचना VfslogFile VfslogFile;

संरचना VfslogVfs {
  sqlite3_vfs आधारः;               /* VFS methods */
  sqlite3_vfs *pVfs;              /* Parent VFS */
  पूर्णाङ्क i_अग्र_File_Id;                /* Next file id */
  sqlite3_file *pLog;             /* Log file handle */
  sqlite3_int64 i_विस्थापनम्;          /* Log file offset of start of write buffer */
  पूर्णाङ्क nBuf;                       /* Number of valid bytes in aBuf[] */
  वर्ण aBuf[VFSLOG_BUFFERSIZE];   /* Write buffer */
};

संरचना VfslogFile {
  sqlite3_file आधारः;              /* IO methods */
  sqlite3_file *pReal;            /* Underlying file handle */
  sqlite3_vfs *pVfslog;           /* Associated VsflogVfs object */
  पूर्णाङ्क iFileId;                    /* File id number */
};

#परिभाषय REALVFS(p) (((VfslogVfs *)(p))->pVfs)



/*
** Method declarations for vfslog_file.
*/
स्थिर पूर्णाङ्क vfslogClose(sqlite3_file*);
स्थिर पूर्णाङ्क vfslogRead(sqlite3_file*, शून्य*, पूर्णाङ्क iAmt, sqlite3_int64 iOfst);
स्थिर पूर्णाङ्क vfslogWrite(sqlite3_file*,ध्रुव शून्य*,पूर्णाङ्क iAmt, sqlite3_int64 iOfst);
स्थिर पूर्णाङ्क vfslogTruncate(sqlite3_file*, sqlite3_int64 आकार);
स्थिर पूर्णाङ्क vfslogSync(sqlite3_file*, पूर्णाङ्क flags);
स्थिर पूर्णाङ्क vfslog_File_आकार(sqlite3_file*, sqlite3_int64 *p_आकार);
स्थिर पूर्णाङ्क vfslog_तालकम्(sqlite3_file*, पूर्णाङ्क);
स्थिर पूर्णाङ्क vfslogUnlock(sqlite3_file*, पूर्णाङ्क);
स्थिर पूर्णाङ्क vfslog_Check_Reserved_तालकम्(sqlite3_file*, पूर्णाङ्क *p_Res_बहिर्गमनम्);
स्थिर पूर्णाङ्क vfslogFileControl(sqlite3_file*, पूर्णाङ्क op, शून्य *pArg);
स्थिर पूर्णाङ्क vfslog_Sector_आकार(sqlite3_file*);
स्थिर पूर्णाङ्क vfslogDeviceCharacteristics(sqlite3_file*);

स्थिर पूर्णाङ्क vfslog_Shm_तालकम्(sqlite3_file *pFile, पूर्णाङ्क ofst, पूर्णाङ्क n, पूर्णाङ्क flags);
स्थिर पूर्णाङ्क vfslogShmMap(sqlite3_file *pFile,पूर्णाङ्क,पूर्णाङ्क,पूर्णाङ्क,चञ्चल शून्य **);
स्थिर शून्य vfslogShmBarrier(sqlite3_file*);
स्थिर पूर्णाङ्क vfslogShmUnmap(sqlite3_file *pFile, पूर्णाङ्क deleteFlag);

/*
** Method declarations for vfslog_vfs.
*/
स्थिर पूर्णाङ्क vfslogOpen(sqlite3_vfs*, ध्रुव वर्ण *, sqlite3_file*, पूर्णाङ्क , पूर्णाङ्क *);
स्थिर पूर्णाङ्क vfslogDelete(sqlite3_vfs*, ध्रुव वर्ण *zName, पूर्णाङ्क syncDir);
स्थिर पूर्णाङ्क vfslogAccess(sqlite3_vfs*, ध्रुव वर्ण *zName, पूर्णाङ्क flags, पूर्णाङ्क *);
स्थिर पूर्णाङ्क vfslog_पूर्णम्_Pathname(sqlite3_vfs*, ध्रुव वर्ण *zName, पूर्णाङ्क, वर्ण *z_बहिर्गमनम्);
स्थिर शून्य *vfslogDlOpen(sqlite3_vfs*, ध्रुव वर्ण *zFilename);
स्थिर शून्य vfslogDlError(sqlite3_vfs*, पूर्णाङ्क nByte, वर्ण *z_Err_सन्देशः);
स्थिर शून्य (*vfslogDlSym(sqlite3_vfs *pVfs, शून्य *p, ध्रुव वर्ण*zSym))(शून्य);
स्थिर शून्य vfslogDlClose(sqlite3_vfs*, शून्य*);
स्थिर पूर्णाङ्क vfslogRandomness(sqlite3_vfs*, पूर्णाङ्क nByte, वर्ण *z_बहिर्गमनम्);
स्थिर पूर्णाङ्क vfslogSleep(sqlite3_vfs*, पूर्णाङ्क microseconds);
स्थिर पूर्णाङ्क vfslogCurrentTime(sqlite3_vfs*, द्विगुण*);

स्थिर पूर्णाङ्क vfslog_Get_चरमः_Error(sqlite3_vfs*, पूर्णाङ्क, वर्ण *);
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
  vfslog_पूर्णम्_Pathname,             /* xFullPathname */
  vfslogDlOpen,                   /* xDlOpen */
  vfslogDlError,                  /* xDlError */
  vfslogDlSym,                    /* xDlSym */
  vfslogDlClose,                  /* xDlClose */
  vfslogRandomness,               /* xRandomness */
  vfslogSleep,                    /* xSleep */
  vfslogCurrentTime,              /* xCurrentTime */
  vfslog_Get_चरमः_Error,             /* xGetLastError */
  vfslogCurrentTimeInt64          /* xCurrentTime */
};

स्थिर sqlite3_io_methods vfslog_io_methods = {
  2,                              /* iVersion */
  vfslogClose,                    /* xClose */
  vfslogRead,                     /* xRead */
  vfslogWrite,                    /* xWrite */
  vfslogTruncate,                 /* xTruncate */
  vfslogSync,                     /* xSync */
  vfslog_File_आकार,                 /* xFileSize */
  vfslog_तालकम्,                     /* xLock */
  vfslogUnlock,                   /* xUnlock */
  vfslog_Check_Reserved_तालकम्,        /* xCheckReservedLock */
  vfslogFileControl,              /* xFileControl */
  vfslog_Sector_आकार,               /* xSectorSize */
  vfslogDeviceCharacteristics,    /* xDeviceCharacteristics */
  vfslogShmMap,                   /* xShmMap */
  vfslog_Shm_तालकम्,                  /* xShmLock */
  vfslogShmBarrier,               /* xShmBarrier */
  vfslogShmUnmap                  /* xShmUnmap */
};

#यदि_परिभाषितम् _WIN32
#अन्तर्भाव <time.h>
स्थिर sqlite3_uint64 vfslog_time(){
  FILETIME ft;
  sqlite3_uint64 u64time = 0;
 
  GetSystemTimeAsFileTime(&ft);

  u64time |= ft.dw_उच्चः_Date_Time;
  u64time <<= 32;
  u64time |= ft.dw_नीचः_Date_Time;

  /* ft is 100-nanosecond intervals, we want microseconds */
  प्रत्यागम u64time /(sqlite3_uint64)10;
}
#नोचेद्यदि !defined(NO_GETTOD)
#अन्तर्भाव <sys/time.h>
स्थिर sqlite3_uint64 vfslog_time(){
  संरचना timeval sTime;
  gettimeofday(&sTime, 0);
  प्रत्यागम sTime.tv_usec + (sqlite3_uint64)sTime.tv_sec * 1000000;
}
#अन्यथा
स्थिर sqlite3_uint64 vfslog_time(){
  प्रत्यागम 0;
}
#यदि_अन्तः

स्थिर शून्य vfslog_call(sqlite3_vfs *, पूर्णाङ्क, पूर्णाङ्क, sqlite3_int64, पूर्णाङ्क, पूर्णाङ्क, पूर्णाङ्क);
स्थिर शून्य vfslog_string(sqlite3_vfs *, ध्रुव वर्ण *);

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
  प्रत्यागम rc;
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
  प्रत्यागम rc;
}

/*
** Write data to an vfslog-file.
*/
स्थिर पूर्णाङ्क vfslogWrite(
  sqlite3_file *pFile,
  ध्रुव शून्य *z,
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
  प्रत्यागम rc;
}

/*
** Truncate an vfslog-file.
*/
स्थिर पूर्णाङ्क vfslogTruncate(sqlite3_file *pFile, sqlite_int64 आकार){
  पूर्णाङ्क rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xTruncate(p->pReal, आकार);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_TRUNCATE, p->iFileId, t, rc, 0, (पूर्णाङ्क)आकार);
  प्रत्यागम rc;
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
  प्रत्यागम rc;
}

/*
** Return the current file-size of an vfslog-file.
*/
स्थिर पूर्णाङ्क vfslog_File_आकार(sqlite3_file *pFile, sqlite_int64 *p_आकार){
  पूर्णाङ्क rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->x_File_आकार(p->pReal, p_आकार);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_FILESIZE, p->iFileId, t, rc, 0, (पूर्णाङ्क)*p_आकार);
  प्रत्यागम rc;
}

/*
** Lock an vfslog-file.
*/
स्थिर पूर्णाङ्क vfslog_तालकम्(sqlite3_file *pFile, पूर्णाङ्क e_तालकम्){
  पूर्णाङ्क rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->x_तालकम्(p->pReal, e_तालकम्);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_तालकम्, p->iFileId, t, rc, e_तालकम्, 0);
  प्रत्यागम rc;
}

/*
** Unlock an vfslog-file.
*/
स्थिर पूर्णाङ्क vfslogUnlock(sqlite3_file *pFile, पूर्णाङ्क e_तालकम्){
  पूर्णाङ्क rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->xUnlock(p->pReal, e_तालकम्);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_UNLOCK, p->iFileId, t, rc, e_तालकम्, 0);
  प्रत्यागम rc;
}

/*
** Check if another file-handle holds a RESERVED lock on an vfslog-file.
*/
स्थिर पूर्णाङ्क vfslog_Check_Reserved_तालकम्(sqlite3_file *pFile, पूर्णाङ्क *p_Res_बहिर्गमनम्){
  पूर्णाङ्क rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->x_Check_Reserved_तालकम्(p->pReal, p_Res_बहिर्गमनम्);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_CHECKRESERVEDLOCK, p->iFileId, t, rc, *p_Res_बहिर्गमनम्, 0);
  प्रत्यागम rc;
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
  प्रत्यागम rc;
}

/*
** Return the sector-size in bytes for an vfslog-file.
*/
स्थिर पूर्णाङ्क vfslog_Sector_आकार(sqlite3_file *pFile){
  पूर्णाङ्क rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->x_Sector_आकार(p->pReal);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SECTORSIZE, p->iFileId, t, rc, 0, 0);
  प्रत्यागम rc;
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
  प्रत्यागम rc;
}

स्थिर पूर्णाङ्क vfslog_Shm_तालकम्(sqlite3_file *pFile, पूर्णाङ्क ofst, पूर्णाङ्क n, पूर्णाङ्क flags){
  पूर्णाङ्क rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_time();
  rc = p->pReal->pMethods->x_Shm_तालकम्(p->pReal, ofst, n, flags);
  t = vfslog_time() - t;
  vfslog_call(p->pVfslog, OS_SHMLOCK, p->iFileId, t, rc, 0, 0);
  प्रत्यागम rc;
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
  प्रत्यागम rc;
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
  प्रत्यागम rc;
}


/*
** Open an vfslog file handle.
*/
स्थिर पूर्णाङ्क vfslogOpen(
  sqlite3_vfs *pVfs,
  ध्रुव वर्ण *zName,
  sqlite3_file *pFile,
  पूर्णाङ्क flags,
  पूर्णाङ्क *p_बहिर्गमनम्_Flags
){
  पूर्णाङ्क rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  VfslogVfs *pLog = (VfslogVfs *)pVfs;

  pFile->pMethods = &vfslog_io_methods;
  p->pReal = (sqlite3_file *)&p[1];
  p->pVfslog = pVfs;
  p->iFileId = ++pLog->i_अग्र_File_Id;

  t = vfslog_time();
  rc = REALVFS(pVfs)->xOpen(REALVFS(pVfs), zName, p->pReal, flags, p_बहिर्गमनम्_Flags);
  t = vfslog_time() - t;

  vfslog_call(pVfs, OS_OPEN, p->iFileId, t, rc, 0, 0);
  vfslog_string(pVfs, zName);
  प्रत्यागम rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
स्थिर पूर्णाङ्क vfslogDelete(sqlite3_vfs *pVfs, ध्रुव वर्ण *z_मार्गः, पूर्णाङ्क dirSync){
  पूर्णाङ्क rc;
  sqlite3_uint64 t;
  t = vfslog_time();
  rc = REALVFS(pVfs)->xDelete(REALVFS(pVfs), z_मार्गः, dirSync);
  t = vfslog_time() - t;
  vfslog_call(pVfs, OS_DELETE, 0, t, rc, dirSync, 0);
  vfslog_string(pVfs, z_मार्गः);
  प्रत्यागम rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
स्थिर पूर्णाङ्क vfslogAccess(
  sqlite3_vfs *pVfs, 
  ध्रुव वर्ण *z_मार्गः, 
  पूर्णाङ्क flags, 
  पूर्णाङ्क *p_Res_बहिर्गमनम्
){
  पूर्णाङ्क rc;
  sqlite3_uint64 t;
  t = vfslog_time();
  rc = REALVFS(pVfs)->xAccess(REALVFS(pVfs), z_मार्गः, flags, p_Res_बहिर्गमनम्);
  t = vfslog_time() - t;
  vfslog_call(pVfs, OS_ACCESS, 0, t, rc, flags, *p_Res_बहिर्गमनम्);
  vfslog_string(pVfs, z_मार्गः);
  प्रत्यागम rc;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
स्थिर पूर्णाङ्क vfslog_पूर्णम्_Pathname(
  sqlite3_vfs *pVfs, 
  ध्रुव वर्ण *z_मार्गः, 
  पूर्णाङ्क n_बहिर्गमनम्, 
  वर्ण *z_बहिर्गमनम्
){
  प्रत्यागम REALVFS(pVfs)->x_पूर्णम्_Pathname(REALVFS(pVfs), z_मार्गः, n_बहिर्गमनम्, z_बहिर्गमनम्);
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
स्थिर शून्य *vfslogDlOpen(sqlite3_vfs *pVfs, ध्रुव वर्ण *z_मार्गः){
  प्रत्यागम REALVFS(pVfs)->xDlOpen(REALVFS(pVfs), z_मार्गः);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
स्थिर शून्य vfslogDlError(sqlite3_vfs *pVfs, पूर्णाङ्क nByte, वर्ण *z_Err_सन्देशः){
  REALVFS(pVfs)->xDlError(REALVFS(pVfs), nByte, z_Err_सन्देशः);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
स्थिर शून्य (*vfslogDlSym(sqlite3_vfs *pVfs, शून्य *p, ध्रुव वर्ण *zSym))(शून्य){
  प्रत्यागम REALVFS(pVfs)->xDlSym(REALVFS(pVfs), p, zSym);
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
स्थिर पूर्णाङ्क vfslogRandomness(sqlite3_vfs *pVfs, पूर्णाङ्क nByte, वर्ण *z_Buf_बहिर्गमनम्){
  प्रत्यागम REALVFS(pVfs)->xRandomness(REALVFS(pVfs), nByte, z_Buf_बहिर्गमनम्);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
स्थिर पूर्णाङ्क vfslogSleep(sqlite3_vfs *pVfs, पूर्णाङ्क nMicro){
  प्रत्यागम REALVFS(pVfs)->xSleep(REALVFS(pVfs), nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
स्थिर पूर्णाङ्क vfslogCurrentTime(sqlite3_vfs *pVfs, द्विगुण *p_Time_बहिर्गमनम्){
  प्रत्यागम REALVFS(pVfs)->xCurrentTime(REALVFS(pVfs), p_Time_बहिर्गमनम्);
}

स्थिर पूर्णाङ्क vfslog_Get_चरमः_Error(sqlite3_vfs *pVfs, पूर्णाङ्क a, वर्ण *b){
  प्रत्यागम REALVFS(pVfs)->x_Get_चरमः_Error(REALVFS(pVfs), a, b);
}
स्थिर पूर्णाङ्क vfslogCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *p){
  प्रत्यागम REALVFS(pVfs)->xCurrentTimeInt64(REALVFS(pVfs), p);
}

स्थिर शून्य vfslog_flush(VfslogVfs *p){
#यदि_परिभाषितम् SQLITE_TEST
  बाह्य पूर्णाङ्क sqlite3_io_error_pending;
  बाह्य पूर्णाङ्क sqlite3_io_error_persist;
  बाह्य पूर्णाङ्क sqlite3_diskfull_pending;

  पूर्णाङ्क pending = sqlite3_io_error_pending;
  पूर्णाङ्क persist = sqlite3_io_error_persist;
  पूर्णाङ्क diskfull = sqlite3_diskfull_pending;

  sqlite3_io_error_pending = 0;
  sqlite3_io_error_persist = 0;
  sqlite3_diskfull_pending = 0;
#यदि_अन्तः

  यदि( p->nBuf ){
    p->pLog->pMethods->xWrite(p->pLog, p->aBuf, p->nBuf, p->i_विस्थापनम्);
    p->i_विस्थापनम् += p->nBuf;
    p->nBuf = 0;
  }

#यदि_परिभाषितम् SQLITE_TEST
  sqlite3_io_error_pending = pending;
  sqlite3_io_error_persist = persist;
  sqlite3_diskfull_pending = diskfull;
#यदि_अन्तः
}

स्थिर शून्य put32bits(अचिह्नित वर्ण *p, अचिह्नित पूर्णाङ्क v){
  p[0] = v>>24;
  p[1] = (अचिह्नित वर्ण)(v>>16);
  p[2] = (अचिह्नित वर्ण)(v>>8);
  p[3] = (अचिह्नित वर्ण)v;
}

स्थिर शून्य vfslog_call(
  sqlite3_vfs *pVfs,
  पूर्णाङ्क eEvent,
  पूर्णाङ्क iFileid,
  sqlite3_int64 nClick,
  पूर्णाङ्क return_संहिता,
  पूर्णाङ्क आकार,
  पूर्णाङ्क विस्थापनम्
){
  VfslogVfs *p = (VfslogVfs *)pVfs;
  अचिह्नित वर्ण *zRec;
  यदि( (24+p->nBuf)>परिमाण(p->aBuf) ){
    vfslog_flush(p);
  }
  zRec = (अचिह्नित वर्ण *)&p->aBuf[p->nBuf];
  put32bits(&zRec[0], eEvent);
  put32bits(&zRec[4], iFileid);
  put32bits(&zRec[8], (अचिह्नित पूर्णाङ्क)(nClick&0xffff));
  put32bits(&zRec[12], return_संहिता);
  put32bits(&zRec[16], आकार);
  put32bits(&zRec[20], विस्थापनम्);
  p->nBuf += 24;
}

स्थिर शून्य vfslog_string(sqlite3_vfs *pVfs, ध्रुव वर्ण *zStr){
  VfslogVfs *p = (VfslogVfs *)pVfs;
  अचिह्नित वर्ण *zRec;
  पूर्णाङ्क nStr = zStr ? (पूर्णाङ्क)वाक्यदैर्घ्यम्(zStr) : 0;
  यदि( (4+nStr+p->nBuf)>परिमाण(p->aBuf) ){
    vfslog_flush(p);
  }
  zRec = (अचिह्नित वर्ण *)&p->aBuf[p->nBuf];
  put32bits(&zRec[0], nStr);
  यदि( zStr ){
    स्मृतिं_प्रतिलिख(&zRec[4], zStr, nStr);
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

पूर्णाङ्क sqlite3_vfslog_finalize(ध्रुव वर्ण *zVfs){
  sqlite3_vfs *pVfs;
  pVfs = sqlite3_vfs_find(zVfs);
  यदि( !pVfs || pVfs->xOpen!=vfslogOpen ){
    प्रत्यागम SQLITE_ERROR;
  } 
  sqlite3_vfs_unregister(pVfs);
  vfslog_finalize((VfslogVfs *)pVfs);
  प्रत्यागम SQLITE_OK;
}

पूर्णाङ्क sqlite3_vfslog_new(
  ध्रुव वर्ण *zVfs,               /* New VFS name */
  ध्रुव वर्ण *zParentVfs,         /* Parent VFS name (or NULL) */
  ध्रुव वर्ण *zLog                /* Log file name */
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
    प्रत्यागम SQLITE_ERROR;
  }

  nVfs = (पूर्णाङ्क)वाक्यदैर्घ्यम्(zVfs);
  nByte = परिमाण(VfslogVfs) + pParent->szOsFile + nVfs+1+pParent->mxPathname+1;
  p = (VfslogVfs *)sqlite3_malloc(nByte);
  स्मृतिं_पूरय(p, 0, nByte);

  p->pVfs = pParent;
  p->pLog = (sqlite3_file *)&p[1];
  स्मृतिं_प्रतिलिख(&p->आधारः, &vfslog_vfs, परिमाण(sqlite3_vfs));
  p->आधारः.zName = &((वर्ण *)p->pLog)[pParent->szOsFile];
  p->आधारः.szOsFile += pParent->szOsFile;
  स्मृतिं_प्रतिलिख((वर्ण *)p->आधारः.zName, zVfs, nVfs);

  zFile = (वर्ण *)&p->आधारः.zName[nVfs+1];
  pParent->x_पूर्णम्_Pathname(pParent, zLog, pParent->mxPathname, zFile);

  flags = SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE|SQLITE_OPEN_SUPER_JOURNAL;
  pParent->xDelete(pParent, zFile, 0);
  rc = pParent->xOpen(pParent, zFile, p->pLog, flags, &flags);
  यदि( rc==SQLITE_OK ){
    स्मृतिं_प्रतिलिख(p->aBuf, "sqlite_ostrace1.....", 20);
    p->i_विस्थापनम् = 0;
    p->nBuf = 20;
    rc = sqlite3_vfs_पञ्जिकाकोशः((sqlite3_vfs *)p, 1);
  }
  यदि( rc ){
    vfslog_finalize(p);
  }
  प्रत्यागम rc;
}

पूर्णाङ्क sqlite3_vfslog_annotate(ध्रुव वर्ण *zVfs, ध्रुव वर्ण *z_सन्देशः){
  sqlite3_vfs *pVfs;
  pVfs = sqlite3_vfs_find(zVfs);
  यदि( !pVfs || pVfs->xOpen!=vfslogOpen ){
    प्रत्यागम SQLITE_ERROR;
  } 
  vfslog_call(pVfs, OS_ANNOTATE, 0, 0, 0, 0, 0);
  vfslog_string(pVfs, z_सन्देशः);
  प्रत्यागम SQLITE_OK;
}

स्थिर ध्रुव वर्ण *vfslog_eventname(पूर्णाङ्क eEvent){
  ध्रुव वर्ण *zEvent = 0;

  विकल्प( eEvent ){
    पक्ष OS_CLOSE:             zEvent = "xClose"; भङ्ग;
    पक्ष OS_READ:              zEvent = "xRead"; भङ्ग;
    पक्ष OS_WRITE:             zEvent = "xWrite"; भङ्ग;
    पक्ष OS_TRUNCATE:          zEvent = "xTruncate"; भङ्ग;
    पक्ष OS_SYNC:              zEvent = "xSync"; भङ्ग;
    पक्ष OS_FILESIZE:          zEvent = "xFilesize"; भङ्ग;
    पक्ष OS_तालकम्:              zEvent = "xLock"; भङ्ग;
    पक्ष OS_UNLOCK:            zEvent = "xUnlock"; भङ्ग;
    पक्ष OS_CHECKRESERVEDLOCK: zEvent = "xCheckResLock"; भङ्ग;
    पक्ष OS_FILECONTROL:       zEvent = "xFileControl"; भङ्ग;
    पक्ष OS_SECTORSIZE:        zEvent = "xSectorSize"; भङ्ग;
    पक्ष OS_DEVCHAR:           zEvent = "xDeviceChar"; भङ्ग;
    पक्ष OS_OPEN:              zEvent = "xOpen"; भङ्ग;
    पक्ष OS_DELETE:            zEvent = "xDelete"; भङ्ग;
    पक्ष OS_ACCESS:            zEvent = "xAccess"; भङ्ग;
    पक्ष OS_FULLPATHNAME:      zEvent = "xFullPathname"; भङ्ग;
    पक्ष OS_RANDOMNESS:        zEvent = "xRandomness"; भङ्ग;
    पक्ष OS_SLEEP:             zEvent = "xSleep"; भङ्ग;
    पक्ष OS_CURRENTTIME:       zEvent = "xCurrentTime"; भङ्ग;

    पक्ष OS_SHMUNMAP:          zEvent = "xShmUnmap"; भङ्ग;
    पक्ष OS_SHMLOCK:           zEvent = "xShmLock"; भङ्ग;
    पक्ष OS_SHMBARRIER:        zEvent = "xShmBarrier"; भङ्ग;
    पक्ष OS_SHMMAP:            zEvent = "xShmMap"; भङ्ग;

    पक्ष OS_ANNOTATE:          zEvent = "annotation"; भङ्ग;
  }

  प्रत्यागम zEvent;
}

प्रकारनिर्देश संरचना VfslogVtab VfslogVtab;
प्रकारनिर्देश संरचना VfslogCsr VfslogCsr;

/*
** Virtual table type for the vfslog reader module.
*/
संरचना VfslogVtab {
  sqlite3_vtab आधारः;              /* Base class */
  sqlite3_file *pFd;              /* File descriptor open on vfslog file */
  sqlite3_int64 nByte;            /* Size of file in bytes */
  वर्ण *zFile;                    /* File name for pFd */
};

/*
** Virtual table cursor type for the vfslog reader module.
*/
संरचना VfslogCsr {
  sqlite3_vtab_cursor आधारः;       /* Base class */
  sqlite3_int64 iRowid;           /* Current rowid. */
  sqlite3_int64 i_विस्थापनम्;          /* Offset of next record in file */
  वर्ण *zTransient;               /* Transient 'file' string */
  पूर्णाङ्क nFile;                      /* Size of array azFile[] */
  वर्ण **azFile;                  /* File strings */
  अचिह्नित वर्ण aBuf[1024];       /* Current vfs log entry (read from file) */
};

स्थिर अचिह्नित पूर्णाङ्क get32bits(अचिह्नित वर्ण *p){
  प्रत्यागम (p[0]<<24) + (p[1]<<16) + (p[2]<<8) + p[3];
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
    पूर्णाङ्क i_अन्तर् = 1;                  /* Index of next byte to read from input */
    पूर्णाङ्क i_बहिर्गमनम् = 0;                 /* Index of next byte to write to output */
    यदि( quote=='[' ) quote = ']';  
    यावत्( z[i_अन्तर्] ){
      यदि( z[i_अन्तर्]==quote ){
        यदि( z[i_अन्तर्+1]!=quote ) भङ्ग;
        z[i_बहिर्गमनम्++] = quote;
        i_अन्तर् += 2;
      }अन्यथा{
        z[i_बहिर्गमनम्++] = z[i_अन्तर्++];
      }
    }
    z[i_बहिर्गमनम्] = '\0';
  }
}

#यदि_अपरिभाषितम् SQLITE_OMIT_VIRTUALTABLE
/*
** Connect to or create a vfslog virtual table.
*/
स्थिर पूर्णाङ्क vlogConnect(
  sqlite3 *db,
  शून्य *pAux,
  पूर्णाङ्क argc, ध्रुव वर्ण *ध्रुव*argv,
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
  यदि( p==0 ) प्रत्यागम SQLITE_NOMEM;
  स्मृतिं_पूरय(p, 0, nByte);

  p->pFd = (sqlite3_file *)&p[1];
  p->zFile = &((वर्ण *)p->pFd)[pVfs->szOsFile];

  zFile = sqlite3_mprintf("%s", argv[3]);
  यदि( !zFile ){
    sqlite3_free(p);
    प्रत्यागम SQLITE_NOMEM;
  }
  dequote(zFile);
  pVfs->x_पूर्णम्_Pathname(pVfs, zFile, pVfs->mxPathname, p->zFile);
  sqlite3_free(zFile);

  flags = SQLITE_OPEN_READWRITE|SQLITE_OPEN_SUPER_JOURNAL;
  rc = pVfs->xOpen(pVfs, p->zFile, p->pFd, flags, &flags);

  यदि( rc==SQLITE_OK ){
    p->pFd->pMethods->x_File_आकार(p->pFd, &p->nByte);
    sqlite3_declare_vtab(db, 
        "CREATE TABLE xxx(event, file, click, rc, size, offset)"
    );
    *ppVtab = &p->आधारः;
  }अन्यथा{
    sqlite3_free(p);
  }

  प्रत्यागम rc;
}

/*
** There is no "best-index". This virtual table always does a linear
** scan of the binary VFS log file.
*/
स्थिर पूर्णाङ्क vlog_Best_सूचकांक(sqlite3_vtab *tab, sqlite3_सूचकांक_info *pIdxInfo){
  pIdxInfo->estimatedCost = 10.0;
  प्रत्यागम SQLITE_OK;
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
  प्रत्यागम SQLITE_OK;
}

/*
** Open a new vfslog cursor.
*/
स्थिर पूर्णाङ्क vlogOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  VfslogCsr *pCsr;                /* Newly allocated cursor object */

  pCsr = sqlite3_malloc(परिमाण(VfslogCsr));
  यदि( !pCsr ) प्रत्यागम SQLITE_NOMEM;
  स्मृतिं_पूरय(pCsr, 0, परिमाण(VfslogCsr));
  *ppCursor = &pCsr->आधारः;
  प्रत्यागम SQLITE_OK;
}

/*
** Close a vfslog cursor.
*/
स्थिर पूर्णाङ्क vlogClose(sqlite3_vtab_cursor *pCursor){
  VfslogCsr *p = (VfslogCsr *)pCursor;
  पूर्णाङ्क i;
  प्रति(i=0; i<p->nFile; i++){
    sqlite3_free(p->azFile[i]);
  }
  sqlite3_free(p->azFile);
  sqlite3_free(p->zTransient);
  sqlite3_free(p);
  प्रत्यागम SQLITE_OK;
}

/*
** Move a vfslog cursor to the next entry in the file.
*/
स्थिर पूर्णाङ्क vlog_अग्र(sqlite3_vtab_cursor *pCursor){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  VfslogVtab *p = (VfslogVtab *)pCursor->pVtab;
  पूर्णाङ्क rc = SQLITE_OK;
  पूर्णाङ्क nRead;

  sqlite3_free(pCsr->zTransient);
  pCsr->zTransient = 0;

  nRead = 24;
  यदि( pCsr->i_विस्थापनम्+nRead<=p->nByte ){
    पूर्णाङ्क eEvent;
    rc = p->pFd->pMethods->xRead(p->pFd, pCsr->aBuf, nRead, pCsr->i_विस्थापनम्);

    eEvent = get32bits(pCsr->aBuf);
    यदि( (rc==SQLITE_OK)
     && (eEvent==OS_OPEN || eEvent==OS_DELETE || eEvent==OS_ACCESS) 
    ){
      वर्ण buf[4];
      rc = p->pFd->pMethods->xRead(p->pFd, buf, 4, pCsr->i_विस्थापनम्+nRead);
      nRead += 4;
      यदि( rc==SQLITE_OK ){
        पूर्णाङ्क nStr = get32bits((अचिह्नित वर्ण *)buf);
        वर्ण *zStr = sqlite3_malloc(nStr+1);
        rc = p->pFd->pMethods->xRead(p->pFd, zStr, nStr, pCsr->i_विस्थापनम्+nRead);
        zStr[nStr] = '\0';
        nRead += nStr;

        यदि( eEvent==OS_OPEN ){
          पूर्णाङ्क iFileid = get32bits(&pCsr->aBuf[4]);
          यदि( iFileid>=pCsr->nFile ){
            पूर्णाङ्क nNew = परिमाण(pCsr->azFile[0])*(iFileid+1);
            pCsr->azFile = (वर्ण **)sqlite3_realloc(pCsr->azFile, nNew);
            nNew -= परिमाण(pCsr->azFile[0])*pCsr->nFile;
            स्मृतिं_पूरय(&pCsr->azFile[pCsr->nFile], 0, nNew);
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
  pCsr->i_विस्थापनम् += nRead;
  प्रत्यागम rc;
}

स्थिर पूर्णाङ्क vlogEof(sqlite3_vtab_cursor *pCursor){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  VfslogVtab *p = (VfslogVtab *)pCursor->pVtab;
  प्रत्यागम (pCsr->i_विस्थापनम्>=p->nByte);
}

स्थिर पूर्णाङ्क vlogFilter(
  sqlite3_vtab_cursor *pCursor, 
  पूर्णाङ्क idx_अङ्कः, ध्रुव वर्ण *idxStr,
  पूर्णाङ्क argc, sqlite3_मानम् **argv
){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  pCsr->iRowid = 0;
  pCsr->i_विस्थापनम् = 20;
  प्रत्यागम vlog_अग्र(pCursor);
}

स्थिर पूर्णाङ्क vlogColumn(
  sqlite3_vtab_cursor *pCursor, 
  sqlite3_context *ctx, 
  पूर्णाङ्क i
){
  अचिह्नित पूर्णाङ्क val;
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;

  प्रतिज्ञा( i<7 );
  val = get32bits(&pCsr->aBuf[4*i]);

  विकल्प( i ){
    पक्ष 0: {
      sqlite3_परिणाम_text(ctx, vfslog_eventname(val), -1, SQLITE_STATIC);
      भङ्ग;
    }
    पक्ष 1: {
      वर्ण *zStr = pCsr->zTransient;
      यदि( val!=0 && val<(अचिह्नित)pCsr->nFile ){
        zStr = pCsr->azFile[val];
      }
      sqlite3_परिणाम_text(ctx, zStr, -1, SQLITE_TRANSIENT);
      भङ्ग;
    }
    स्वभाव:
      sqlite3_परिणाम_int(ctx, val);
      भङ्ग;
  }

  प्रत्यागम SQLITE_OK;
}

स्थिर पूर्णाङ्क vlogRowid(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  *pRowid = pCsr->iRowid;
  प्रत्यागम SQLITE_OK;
}

पूर्णाङ्क sqlite3_vfslog_पञ्जिकाकोशः(sqlite3 *db){
  स्थिर sqlite3_module vfslog_module = {
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
  प्रत्यागम SQLITE_OK;
}
#यदि_अन्तः /* SQLITE_OMIT_VIRTUALTABLE */

/**************************************************************************
***************************************************************************
** Tcl interface starts here.
*/

#यदि defined(SQLITE_TEST) || defined(TCLSH)

#अन्तर्भाव "tclsqlite.h"

स्थिर पूर्णाङ्क SQLITE_TCLAPI test_vfslog(
  शून्य *client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  संरचना SqliteDb { sqlite3 *db; };
  sqlite3 *db;
  Tcl_CmdInfo cmdInfo;
  पूर्णाङ्क rc = SQLITE_ERROR;

  स्थिर ध्रुव वर्ण *strs[] = { "annotate", "finalize", "new",  "register", 0 };
  गणना VL_enum { VL_ANNOTATE, VL_FINALIZE, VL_NEW, VL_पञ्जिकाकोशः };
  पूर्णाङ्क iSub;

  यदि( objc<2 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "SUB-COMMAND ...");
    प्रत्यागम TCL_ERROR;
  }
  यदि( Tcl_Get_सूचकांक_From_Obj(interp, objv[1], strs, "sub-command", 0, &iSub) ){
    प्रत्यागम TCL_ERROR;
  }

  विकल्प( (गणना VL_enum)iSub ){
    पक्ष VL_ANNOTATE: {
      वर्ण *zVfs;
      वर्ण *z_सन्देशः;
      यदि( objc!=4 ){
        Tcl_Wrong_अङ्कः_तर्काः(interp, 3, objv, "VFS");
        प्रत्यागम TCL_ERROR;
      }
      zVfs = Tcl_GetString(objv[2]);
      z_सन्देशः = Tcl_GetString(objv[3]);
      rc = sqlite3_vfslog_annotate(zVfs, z_सन्देशः);
      यदि( rc!=SQLITE_OK ){
        Tcl_Append_परिणाम(interp, "failed", (वर्ण*)0);
        प्रत्यागम TCL_ERROR;
      }
      भङ्ग;
    }
    पक्ष VL_FINALIZE: {
      वर्ण *zVfs;
      यदि( objc!=3 ){
        Tcl_Wrong_अङ्कः_तर्काः(interp, 2, objv, "VFS");
        प्रत्यागम TCL_ERROR;
      }
      zVfs = Tcl_GetString(objv[2]);
      rc = sqlite3_vfslog_finalize(zVfs);
      यदि( rc!=SQLITE_OK ){
        Tcl_Append_परिणाम(interp, "failed", (वर्ण*)0);
        प्रत्यागम TCL_ERROR;
      }
      भङ्ग;
    };

    पक्ष VL_NEW: {
      वर्ण *zVfs;
      वर्ण *zParent;
      वर्ण *zLog;
      यदि( objc!=5 ){
        Tcl_Wrong_अङ्कः_तर्काः(interp, 2, objv, "VFS PARENT LOGFILE");
        प्रत्यागम TCL_ERROR;
      }
      zVfs = Tcl_GetString(objv[2]);
      zParent = Tcl_GetString(objv[3]);
      zLog = Tcl_GetString(objv[4]);
      यदि( *zParent=='\0' ) zParent = 0;
      rc = sqlite3_vfslog_new(zVfs, zParent, zLog);
      यदि( rc!=SQLITE_OK ){
        Tcl_Append_परिणाम(interp, "failed", (वर्ण*)0);
        प्रत्यागम TCL_ERROR;
      }
      भङ्ग;
    };

    पक्ष VL_पञ्जिकाकोशः: {
      वर्ण *zDb;
      यदि( objc!=3 ){
        Tcl_Wrong_अङ्कः_तर्काः(interp, 2, objv, "DB");
        प्रत्यागम TCL_ERROR;
      }
#यदि_परिभाषितम् SQLITE_OMIT_VIRTUALTABLE
      Tcl_Append_परिणाम(interp, "vfslog not available because of "
                               "SQLITE_OMIT_VIRTUALTABLE", (शून्य*)0);
      प्रत्यागम TCL_ERROR;
#अन्यथा
      zDb = Tcl_GetString(objv[2]);
      यदि( Tcl_GetCommandInfo(interp, zDb, &cmdInfo) ){
        db = ((संरचना SqliteDb*)cmdInfo.obj_Client_दत्त)->db;
        rc = sqlite3_vfslog_पञ्जिकाकोशः(db);
      }
      यदि( rc!=SQLITE_OK ){
        Tcl_Append_परिणाम(interp, "bad sqlite3 handle: ", zDb, (शून्य*)0);
        प्रत्यागम TCL_ERROR;
      }
      भङ्ग;
#यदि_अन्तः
    }
  }

  प्रत्यागम TCL_OK;
}

पूर्णाङ्क SqlitetestOsinst_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "vfslog", test_vfslog, 0, 0);
  प्रत्यागम TCL_OK;
}

#यदि_अन्तः /* SQLITE_TEST */
