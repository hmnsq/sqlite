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

#అన్తర్భావ "sqlite3.h"

#నిర్వచితమైతే _WIN32
#అన్తర్భావ <windows.h>
#అయితే_ముగింపు

#అన్తర్భావ <string.h>
#అన్తర్భావ <assert.h>


/*
** Maximum pathname length supported by the vfslog backend.
*/
#నిర్వచించు INST_MAX_PATHNAME 512

#నిర్వచించు OS_ACCESS            1
#నిర్వచించు OS_CHECKRESERVEDLOCK 2
#నిర్వచించు OS_CLOSE             3
#నిర్వచించు OS_CURRENTTIME       4
#నిర్వచించు OS_DELETE            5
#నిర్వచించు OS_DEVCHAR           6
#నిర్వచించు OS_FILECONTROL       7
#నిర్వచించు OS_FILESIZE          8
#నిర్వచించు OS_FULLPATHNAME      9
#నిర్వచించు OS_తాళం              11
#నిర్వచించు OS_OPEN              12
#నిర్వచించు OS_RANDOMNESS        13
#నిర్వచించు OS_READ              14 
#నిర్వచించు OS_SECTORSIZE        15
#నిర్వచించు OS_SLEEP             16
#నిర్వచించు OS_SYNC              17
#నిర్వచించు OS_TRUNCATE          18
#నిర్వచించు OS_UNLOCK            19
#నిర్వచించు OS_WRITE             20
#నిర్వచించు OS_SHMUNMAP          22
#నిర్వచించు OS_SHMMAP            23
#నిర్వచించు OS_SHMLOCK           25
#నిర్వచించు OS_SHMBARRIER        26
#నిర్వచించు OS_ANNOTATE          28

#నిర్వచించు OS_NUMEVENTS         29

#నిర్వచించు VFSLOG_BUFFERSIZE 8192

రకనిర్వచనం నిర్మాణం VfslogVfs VfslogVfs;
రకనిర్వచనం నిర్మాణం VfslogFile VfslogFile;

నిర్మాణం VfslogVfs {
  sqlite3_vfs పునాది;               /* VFS methods */
  sqlite3_vfs *pVfs;              /* Parent VFS */
  పూర్ణాంకం i_తదుపరి_File_Id;                /* Next file id */
  sqlite3_file *pLog;             /* Log file handle */
  sqlite3_int64 i_స్థానభ్రంశం;          /* Log file offset of start of write buffer */
  పూర్ణాంకం nBuf;                       /* Number of valid bytes in aBuf[] */
  అక్షరం aBuf[VFSLOG_BUFFERSIZE];   /* Write buffer */
};

నిర్మాణం VfslogFile {
  sqlite3_file పునాది;              /* IO methods */
  sqlite3_file *pReal;            /* Underlying file handle */
  sqlite3_vfs *pVfslog;           /* Associated VsflogVfs object */
  పూర్ణాంకం iFileId;                    /* File id number */
};

#నిర్వచించు REALVFS(p) (((VfslogVfs *)(p))->pVfs)



/*
** Method declarations for vfslog_file.
*/
స్థిర పూర్ణాంకం vfslogClose(sqlite3_file*);
స్థిర పూర్ణాంకం vfslogRead(sqlite3_file*, శూన్య*, పూర్ణాంకం iAmt, sqlite3_int64 iOfst);
స్థిర పూర్ణాంకం vfslogWrite(sqlite3_file*,స్థిరరాశి శూన్య*,పూర్ణాంకం iAmt, sqlite3_int64 iOfst);
స్థిర పూర్ణాంకం vfslogTruncate(sqlite3_file*, sqlite3_int64 కొలత);
స్థిర పూర్ణాంకం vfslogSync(sqlite3_file*, పూర్ణాంకం flags);
స్థిర పూర్ణాంకం vfslog_File_కొలత(sqlite3_file*, sqlite3_int64 *p_కొలత);
స్థిర పూర్ణాంకం vfslog_తాళం(sqlite3_file*, పూర్ణాంకం);
స్థిర పూర్ణాంకం vfslogUnlock(sqlite3_file*, పూర్ణాంకం);
స్థిర పూర్ణాంకం vfslog_Check_Reserved_తాళం(sqlite3_file*, పూర్ణాంకం *p_Res_బహిర్గతం);
స్థిర పూర్ణాంకం vfslogFileControl(sqlite3_file*, పూర్ణాంకం op, శూన్య *pArg);
స్థిర పూర్ణాంకం vfslog_Sector_కొలత(sqlite3_file*);
స్థిర పూర్ణాంకం vfslogDeviceCharacteristics(sqlite3_file*);

స్థిర పూర్ణాంకం vfslog_Shm_తాళం(sqlite3_file *pFile, పూర్ణాంకం ofst, పూర్ణాంకం n, పూర్ణాంకం flags);
స్థిర పూర్ణాంకం vfslogShmMap(sqlite3_file *pFile,పూర్ణాంకం,పూర్ణాంకం,పూర్ణాంకం,అస్థిర శూన్య **);
స్థిర శూన్య vfslogShmBarrier(sqlite3_file*);
స్థిర పూర్ణాంకం vfslogShmUnmap(sqlite3_file *pFile, పూర్ణాంకం deleteFlag);

/*
** Method declarations for vfslog_vfs.
*/
స్థిర పూర్ణాంకం vfslogOpen(sqlite3_vfs*, స్థిరరాశి అక్షరం *, sqlite3_file*, పూర్ణాంకం , పూర్ణాంకం *);
స్థిర పూర్ణాంకం vfslogDelete(sqlite3_vfs*, స్థిరరాశి అక్షరం *zName, పూర్ణాంకం syncDir);
స్థిర పూర్ణాంకం vfslogAccess(sqlite3_vfs*, స్థిరరాశి అక్షరం *zName, పూర్ణాంకం flags, పూర్ణాంకం *);
స్థిర పూర్ణాంకం vfslog_నిండిన_Pathname(sqlite3_vfs*, స్థిరరాశి అక్షరం *zName, పూర్ణాంకం, అక్షరం *z_బహిర్గతం);
స్థిర శూన్య *vfslogDlOpen(sqlite3_vfs*, స్థిరరాశి అక్షరం *zFilename);
స్థిర శూన్య vfslogDlError(sqlite3_vfs*, పూర్ణాంకం nByte, అక్షరం *z_Err_సందేశం);
స్థిర శూన్య (*vfslogDlSym(sqlite3_vfs *pVfs, శూన్య *p, స్థిరరాశి అక్షరం*zSym))(శూన్య);
స్థిర శూన్య vfslogDlClose(sqlite3_vfs*, శూన్య*);
స్థిర పూర్ణాంకం vfslogRandomness(sqlite3_vfs*, పూర్ణాంకం nByte, అక్షరం *z_బహిర్గతం);
స్థిర పూర్ణాంకం vfslogSleep(sqlite3_vfs*, పూర్ణాంకం microseconds);
స్థిర పూర్ణాంకం vfslog_Current_సమయం(sqlite3_vfs*, ద్విగుణ*);

స్థిర పూర్ణాంకం vfslog_Get_చివరి_Error(sqlite3_vfs*, పూర్ణాంకం, అక్షరం *);
స్థిర పూర్ణాంకం vfslog_Current_సమయం_Int64(sqlite3_vfs*, sqlite3_int64*);

స్థిర sqlite3_vfs vfslog_vfs = {
  1,                              /* iVersion */
  పరిమాణం(VfslogFile),             /* szOsFile */
  INST_MAX_PATHNAME,              /* mxPathname */
  0,                              /* pNext */
  0,                              /* zName */
  0,                              /* pAppData */
  vfslogOpen,                     /* xOpen */
  vfslogDelete,                   /* xDelete */
  vfslogAccess,                   /* xAccess */
  vfslog_నిండిన_Pathname,             /* xFullPathname */
  vfslogDlOpen,                   /* xDlOpen */
  vfslogDlError,                  /* xDlError */
  vfslogDlSym,                    /* xDlSym */
  vfslogDlClose,                  /* xDlClose */
  vfslogRandomness,               /* xRandomness */
  vfslogSleep,                    /* xSleep */
  vfslog_Current_సమయం,              /* xCurrentTime */
  vfslog_Get_చివరి_Error,             /* xGetLastError */
  vfslog_Current_సమయం_Int64          /* xCurrentTime */
};

స్థిర sqlite3_io_methods vfslog_io_methods = {
  2,                              /* iVersion */
  vfslogClose,                    /* xClose */
  vfslogRead,                     /* xRead */
  vfslogWrite,                    /* xWrite */
  vfslogTruncate,                 /* xTruncate */
  vfslogSync,                     /* xSync */
  vfslog_File_కొలత,                 /* xFileSize */
  vfslog_తాళం,                     /* xLock */
  vfslogUnlock,                   /* xUnlock */
  vfslog_Check_Reserved_తాళం,        /* xCheckReservedLock */
  vfslogFileControl,              /* xFileControl */
  vfslog_Sector_కొలత,               /* xSectorSize */
  vfslogDeviceCharacteristics,    /* xDeviceCharacteristics */
  vfslogShmMap,                   /* xShmMap */
  vfslog_Shm_తాళం,                  /* xShmLock */
  vfslogShmBarrier,               /* xShmBarrier */
  vfslogShmUnmap                  /* xShmUnmap */
};

#నిర్వచితమైతే _WIN32
#అన్తర్భావ <time.h>
స్థిర sqlite3_uint64 vfslog_సమయం(){
  FILETIME ft;
  sqlite3_uint64 u64time = 0;
 
  Get_System_సమయం_As_File_సమయం(&ft);

  u64time |= ft.dw_ఎగువ_Date_సమయం;
  u64time <<= 32;
  u64time |= ft.dw_దిగువ_Date_సమయం;

  /* ft is 100-nanosecond intervals, we want microseconds */
  తిరిగి u64time /(sqlite3_uint64)10;
}
#లేదావేళ !defined(NO_GETTOD)
#అన్తర్భావ <sys/time.h>
స్థిర sqlite3_uint64 vfslog_సమయం(){
  నిర్మాణం timeval s_సమయం;
  gettimeofday(&s_సమయం, 0);
  తిరిగి s_సమయం.tv_usec + (sqlite3_uint64)s_సమయం.tv_sec * 1000000;
}
#లేకపోతే
స్థిర sqlite3_uint64 vfslog_సమయం(){
  తిరిగి 0;
}
#అయితే_ముగింపు

స్థిర శూన్య vfslog_call(sqlite3_vfs *, పూర్ణాంకం, పూర్ణాంకం, sqlite3_int64, పూర్ణాంకం, పూర్ణాంకం, పూర్ణాంకం);
స్థిర శూన్య vfslog_string(sqlite3_vfs *, స్థిరరాశి అక్షరం *);

/*
** Close an vfslog-file.
*/
స్థిర పూర్ణాంకం vfslogClose(sqlite3_file *pFile){
  sqlite3_uint64 t;
  పూర్ణాంకం rc = SQLITE_OK;
  VfslogFile *p = (VfslogFile *)pFile;

  t = vfslog_సమయం();
  ఒకవేళ( p->pReal->pMethods ){
    rc = p->pReal->pMethods->xClose(p->pReal);
  }
  t = vfslog_సమయం() - t;
  vfslog_call(p->pVfslog, OS_CLOSE, p->iFileId, t, rc, 0, 0);
  తిరిగి rc;
}

/*
** Read data from an vfslog-file.
*/
స్థిర పూర్ణాంకం vfslogRead(
  sqlite3_file *pFile, 
  శూన్య *zBuf, 
  పూర్ణాంకం iAmt, 
  sqlite_int64 iOfst
){
  పూర్ణాంకం rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_సమయం();
  rc = p->pReal->pMethods->xRead(p->pReal, zBuf, iAmt, iOfst);
  t = vfslog_సమయం() - t;
  vfslog_call(p->pVfslog, OS_READ, p->iFileId, t, rc, iAmt, (పూర్ణాంకం)iOfst);
  తిరిగి rc;
}

/*
** Write data to an vfslog-file.
*/
స్థిర పూర్ణాంకం vfslogWrite(
  sqlite3_file *pFile,
  స్థిరరాశి శూన్య *z,
  పూర్ణాంకం iAmt,
  sqlite_int64 iOfst
){
  పూర్ణాంకం rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_సమయం();
  rc = p->pReal->pMethods->xWrite(p->pReal, z, iAmt, iOfst);
  t = vfslog_సమయం() - t;
  vfslog_call(p->pVfslog, OS_WRITE, p->iFileId, t, rc, iAmt, (పూర్ణాంకం)iOfst);
  తిరిగి rc;
}

/*
** Truncate an vfslog-file.
*/
స్థిర పూర్ణాంకం vfslogTruncate(sqlite3_file *pFile, sqlite_int64 కొలత){
  పూర్ణాంకం rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_సమయం();
  rc = p->pReal->pMethods->xTruncate(p->pReal, కొలత);
  t = vfslog_సమయం() - t;
  vfslog_call(p->pVfslog, OS_TRUNCATE, p->iFileId, t, rc, 0, (పూర్ణాంకం)కొలత);
  తిరిగి rc;
}

/*
** Sync an vfslog-file.
*/
స్థిర పూర్ణాంకం vfslogSync(sqlite3_file *pFile, పూర్ణాంకం flags){
  పూర్ణాంకం rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_సమయం();
  rc = p->pReal->pMethods->xSync(p->pReal, flags);
  t = vfslog_సమయం() - t;
  vfslog_call(p->pVfslog, OS_SYNC, p->iFileId, t, rc, flags, 0);
  తిరిగి rc;
}

/*
** Return the current file-size of an vfslog-file.
*/
స్థిర పూర్ణాంకం vfslog_File_కొలత(sqlite3_file *pFile, sqlite_int64 *p_కొలత){
  పూర్ణాంకం rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_సమయం();
  rc = p->pReal->pMethods->x_File_కొలత(p->pReal, p_కొలత);
  t = vfslog_సమయం() - t;
  vfslog_call(p->pVfslog, OS_FILESIZE, p->iFileId, t, rc, 0, (పూర్ణాంకం)*p_కొలత);
  తిరిగి rc;
}

/*
** Lock an vfslog-file.
*/
స్థిర పూర్ణాంకం vfslog_తాళం(sqlite3_file *pFile, పూర్ణాంకం e_తాళం){
  పూర్ణాంకం rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_సమయం();
  rc = p->pReal->pMethods->x_తాళం(p->pReal, e_తాళం);
  t = vfslog_సమయం() - t;
  vfslog_call(p->pVfslog, OS_తాళం, p->iFileId, t, rc, e_తాళం, 0);
  తిరిగి rc;
}

/*
** Unlock an vfslog-file.
*/
స్థిర పూర్ణాంకం vfslogUnlock(sqlite3_file *pFile, పూర్ణాంకం e_తాళం){
  పూర్ణాంకం rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_సమయం();
  rc = p->pReal->pMethods->xUnlock(p->pReal, e_తాళం);
  t = vfslog_సమయం() - t;
  vfslog_call(p->pVfslog, OS_UNLOCK, p->iFileId, t, rc, e_తాళం, 0);
  తిరిగి rc;
}

/*
** Check if another file-handle holds a RESERVED lock on an vfslog-file.
*/
స్థిర పూర్ణాంకం vfslog_Check_Reserved_తాళం(sqlite3_file *pFile, పూర్ణాంకం *p_Res_బహిర్గతం){
  పూర్ణాంకం rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_సమయం();
  rc = p->pReal->pMethods->x_Check_Reserved_తాళం(p->pReal, p_Res_బహిర్గతం);
  t = vfslog_సమయం() - t;
  vfslog_call(p->pVfslog, OS_CHECKRESERVEDLOCK, p->iFileId, t, rc, *p_Res_బహిర్గతం, 0);
  తిరిగి rc;
}

/*
** File control method. For custom operations on an vfslog-file.
*/
స్థిర పూర్ణాంకం vfslogFileControl(sqlite3_file *pFile, పూర్ణాంకం op, శూన్య *pArg){
  VfslogFile *p = (VfslogFile *)pFile;
  పూర్ణాంకం rc = p->pReal->pMethods->xFileControl(p->pReal, op, pArg);
  ఒకవేళ( op==SQLITE_FCNTL_VFSNAME && rc==SQLITE_OK ){
    *(అక్షరం**)pArg = sqlite3_mprintf("vfslog/%z", *(అక్షరం**)pArg);
  }
  తిరిగి rc;
}

/*
** Return the sector-size in bytes for an vfslog-file.
*/
స్థిర పూర్ణాంకం vfslog_Sector_కొలత(sqlite3_file *pFile){
  పూర్ణాంకం rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_సమయం();
  rc = p->pReal->pMethods->x_Sector_కొలత(p->pReal);
  t = vfslog_సమయం() - t;
  vfslog_call(p->pVfslog, OS_SECTORSIZE, p->iFileId, t, rc, 0, 0);
  తిరిగి rc;
}

/*
** Return the device characteristic flags supported by an vfslog-file.
*/
స్థిర పూర్ణాంకం vfslogDeviceCharacteristics(sqlite3_file *pFile){
  పూర్ణాంకం rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_సమయం();
  rc = p->pReal->pMethods->xDeviceCharacteristics(p->pReal);
  t = vfslog_సమయం() - t;
  vfslog_call(p->pVfslog, OS_DEVCHAR, p->iFileId, t, rc, 0, 0);
  తిరిగి rc;
}

స్థిర పూర్ణాంకం vfslog_Shm_తాళం(sqlite3_file *pFile, పూర్ణాంకం ofst, పూర్ణాంకం n, పూర్ణాంకం flags){
  పూర్ణాంకం rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_సమయం();
  rc = p->pReal->pMethods->x_Shm_తాళం(p->pReal, ofst, n, flags);
  t = vfslog_సమయం() - t;
  vfslog_call(p->pVfslog, OS_SHMLOCK, p->iFileId, t, rc, 0, 0);
  తిరిగి rc;
}
స్థిర పూర్ణాంకం vfslogShmMap(
  sqlite3_file *pFile, 
  పూర్ణాంకం iRegion, 
  పూర్ణాంకం szRegion, 
  పూర్ణాంకం isWrite, 
  అస్థిర శూన్య **pp
){
  పూర్ణాంకం rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_సమయం();
  rc = p->pReal->pMethods->xShmMap(p->pReal, iRegion, szRegion, isWrite, pp);
  t = vfslog_సమయం() - t;
  vfslog_call(p->pVfslog, OS_SHMMAP, p->iFileId, t, rc, 0, 0);
  తిరిగి rc;
}
స్థిర శూన్య vfslogShmBarrier(sqlite3_file *pFile){
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_సమయం();
  p->pReal->pMethods->xShmBarrier(p->pReal);
  t = vfslog_సమయం() - t;
  vfslog_call(p->pVfslog, OS_SHMBARRIER, p->iFileId, t, SQLITE_OK, 0, 0);
}
స్థిర పూర్ణాంకం vfslogShmUnmap(sqlite3_file *pFile, పూర్ణాంకం deleteFlag){
  పూర్ణాంకం rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_సమయం();
  rc = p->pReal->pMethods->xShmUnmap(p->pReal, deleteFlag);
  t = vfslog_సమయం() - t;
  vfslog_call(p->pVfslog, OS_SHMUNMAP, p->iFileId, t, rc, 0, 0);
  తిరిగి rc;
}


/*
** Open an vfslog file handle.
*/
స్థిర పూర్ణాంకం vfslogOpen(
  sqlite3_vfs *pVfs,
  స్థిరరాశి అక్షరం *zName,
  sqlite3_file *pFile,
  పూర్ణాంకం flags,
  పూర్ణాంకం *p_బహిర్గతం_Flags
){
  పూర్ణాంకం rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  VfslogVfs *pLog = (VfslogVfs *)pVfs;

  pFile->pMethods = &vfslog_io_methods;
  p->pReal = (sqlite3_file *)&p[1];
  p->pVfslog = pVfs;
  p->iFileId = ++pLog->i_తదుపరి_File_Id;

  t = vfslog_సమయం();
  rc = REALVFS(pVfs)->xOpen(REALVFS(pVfs), zName, p->pReal, flags, p_బహిర్గతం_Flags);
  t = vfslog_సమయం() - t;

  vfslog_call(pVfs, OS_OPEN, p->iFileId, t, rc, 0, 0);
  vfslog_string(pVfs, zName);
  తిరిగి rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
స్థిర పూర్ణాంకం vfslogDelete(sqlite3_vfs *pVfs, స్థిరరాశి అక్షరం *z_మార్గం, పూర్ణాంకం dirSync){
  పూర్ణాంకం rc;
  sqlite3_uint64 t;
  t = vfslog_సమయం();
  rc = REALVFS(pVfs)->xDelete(REALVFS(pVfs), z_మార్గం, dirSync);
  t = vfslog_సమయం() - t;
  vfslog_call(pVfs, OS_DELETE, 0, t, rc, dirSync, 0);
  vfslog_string(pVfs, z_మార్గం);
  తిరిగి rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
స్థిర పూర్ణాంకం vfslogAccess(
  sqlite3_vfs *pVfs, 
  స్థిరరాశి అక్షరం *z_మార్గం, 
  పూర్ణాంకం flags, 
  పూర్ణాంకం *p_Res_బహిర్గతం
){
  పూర్ణాంకం rc;
  sqlite3_uint64 t;
  t = vfslog_సమయం();
  rc = REALVFS(pVfs)->xAccess(REALVFS(pVfs), z_మార్గం, flags, p_Res_బహిర్గతం);
  t = vfslog_సమయం() - t;
  vfslog_call(pVfs, OS_ACCESS, 0, t, rc, flags, *p_Res_బహిర్గతం);
  vfslog_string(pVfs, z_మార్గం);
  తిరిగి rc;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
స్థిర పూర్ణాంకం vfslog_నిండిన_Pathname(
  sqlite3_vfs *pVfs, 
  స్థిరరాశి అక్షరం *z_మార్గం, 
  పూర్ణాంకం n_బహిర్గతం, 
  అక్షరం *z_బహిర్గతం
){
  తిరిగి REALVFS(pVfs)->x_నిండిన_Pathname(REALVFS(pVfs), z_మార్గం, n_బహిర్గతం, z_బహిర్గతం);
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
స్థిర శూన్య *vfslogDlOpen(sqlite3_vfs *pVfs, స్థిరరాశి అక్షరం *z_మార్గం){
  తిరిగి REALVFS(pVfs)->xDlOpen(REALVFS(pVfs), z_మార్గం);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
స్థిర శూన్య vfslogDlError(sqlite3_vfs *pVfs, పూర్ణాంకం nByte, అక్షరం *z_Err_సందేశం){
  REALVFS(pVfs)->xDlError(REALVFS(pVfs), nByte, z_Err_సందేశం);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
స్థిర శూన్య (*vfslogDlSym(sqlite3_vfs *pVfs, శూన్య *p, స్థిరరాశి అక్షరం *zSym))(శూన్య){
  తిరిగి REALVFS(pVfs)->xDlSym(REALVFS(pVfs), p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
స్థిర శూన్య vfslogDlClose(sqlite3_vfs *pVfs, శూన్య *pHandle){
  REALVFS(pVfs)->xDlClose(REALVFS(pVfs), pHandle);
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
స్థిర పూర్ణాంకం vfslogRandomness(sqlite3_vfs *pVfs, పూర్ణాంకం nByte, అక్షరం *z_Buf_బహిర్గతం){
  తిరిగి REALVFS(pVfs)->xRandomness(REALVFS(pVfs), nByte, z_Buf_బహిర్గతం);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
స్థిర పూర్ణాంకం vfslogSleep(sqlite3_vfs *pVfs, పూర్ణాంకం nMicro){
  తిరిగి REALVFS(pVfs)->xSleep(REALVFS(pVfs), nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
స్థిర పూర్ణాంకం vfslog_Current_సమయం(sqlite3_vfs *pVfs, ద్విగుణ *p_సమయం_బహిర్గతం){
  తిరిగి REALVFS(pVfs)->x_Current_సమయం(REALVFS(pVfs), p_సమయం_బహిర్గతం);
}

స్థిర పూర్ణాంకం vfslog_Get_చివరి_Error(sqlite3_vfs *pVfs, పూర్ణాంకం a, అక్షరం *b){
  తిరిగి REALVFS(pVfs)->x_Get_చివరి_Error(REALVFS(pVfs), a, b);
}
స్థిర పూర్ణాంకం vfslog_Current_సమయం_Int64(sqlite3_vfs *pVfs, sqlite3_int64 *p){
  తిరిగి REALVFS(pVfs)->x_Current_సమయం_Int64(REALVFS(pVfs), p);
}

స్థిర శూన్య vfslog_flush(VfslogVfs *p){
#నిర్వచితమైతే SQLITE_TEST
  బాహ్య పూర్ణాంకం sqlite3_io_error_pending;
  బాహ్య పూర్ణాంకం sqlite3_io_error_persist;
  బాహ్య పూర్ణాంకం sqlite3_diskfull_pending;

  పూర్ణాంకం pending = sqlite3_io_error_pending;
  పూర్ణాంకం persist = sqlite3_io_error_persist;
  పూర్ణాంకం diskfull = sqlite3_diskfull_pending;

  sqlite3_io_error_pending = 0;
  sqlite3_io_error_persist = 0;
  sqlite3_diskfull_pending = 0;
#అయితే_ముగింపు

  ఒకవేళ( p->nBuf ){
    p->pLog->pMethods->xWrite(p->pLog, p->aBuf, p->nBuf, p->i_స్థానభ్రంశం);
    p->i_స్థానభ్రంశం += p->nBuf;
    p->nBuf = 0;
  }

#నిర్వచితమైతే SQLITE_TEST
  sqlite3_io_error_pending = pending;
  sqlite3_io_error_persist = persist;
  sqlite3_diskfull_pending = diskfull;
#అయితే_ముగింపు
}

స్థిర శూన్య put32bits(గుర్తులేని అక్షరం *p, గుర్తులేని పూర్ణాంకం v){
  p[0] = v>>24;
  p[1] = (గుర్తులేని అక్షరం)(v>>16);
  p[2] = (గుర్తులేని అక్షరం)(v>>8);
  p[3] = (గుర్తులేని అక్షరం)v;
}

స్థిర శూన్య vfslog_call(
  sqlite3_vfs *pVfs,
  పూర్ణాంకం eEvent,
  పూర్ణాంకం iFileid,
  sqlite3_int64 nClick,
  పూర్ణాంకం return_సంకేతం,
  పూర్ణాంకం కొలత,
  పూర్ణాంకం స్థానభ్రంశం
){
  VfslogVfs *p = (VfslogVfs *)pVfs;
  గుర్తులేని అక్షరం *zRec;
  ఒకవేళ( (24+p->nBuf)>పరిమాణం(p->aBuf) ){
    vfslog_flush(p);
  }
  zRec = (గుర్తులేని అక్షరం *)&p->aBuf[p->nBuf];
  put32bits(&zRec[0], eEvent);
  put32bits(&zRec[4], iFileid);
  put32bits(&zRec[8], (గుర్తులేని పూర్ణాంకం)(nClick&0xffff));
  put32bits(&zRec[12], return_సంకేతం);
  put32bits(&zRec[16], కొలత);
  put32bits(&zRec[20], స్థానభ్రంశం);
  p->nBuf += 24;
}

స్థిర శూన్య vfslog_string(sqlite3_vfs *pVfs, స్థిరరాశి అక్షరం *zStr){
  VfslogVfs *p = (VfslogVfs *)pVfs;
  గుర్తులేని అక్షరం *zRec;
  పూర్ణాంకం nStr = zStr ? (పూర్ణాంకం)అక్షరశ్రేణి_పొడవు(zStr) : 0;
  ఒకవేళ( (4+nStr+p->nBuf)>పరిమాణం(p->aBuf) ){
    vfslog_flush(p);
  }
  zRec = (గుర్తులేని అక్షరం *)&p->aBuf[p->nBuf];
  put32bits(&zRec[0], nStr);
  ఒకవేళ( zStr ){
    జ్ఞాపకం_నకలు(&zRec[4], zStr, nStr);
  }
  p->nBuf += (4 + nStr);
}

స్థిర శూన్య vfslog_finalize(VfslogVfs *p){
  ఒకవేళ( p->pLog->pMethods ){
    vfslog_flush(p);
    p->pLog->pMethods->xClose(p->pLog);
  }
  sqlite3_free(p);
}

పూర్ణాంకం sqlite3_vfslog_finalize(స్థిరరాశి అక్షరం *zVfs){
  sqlite3_vfs *pVfs;
  pVfs = sqlite3_vfs_కనుగొను(zVfs);
  ఒకవేళ( !pVfs || pVfs->xOpen!=vfslogOpen ){
    తిరిగి SQLITE_ERROR;
  } 
  sqlite3_vfs_unregister(pVfs);
  vfslog_finalize((VfslogVfs *)pVfs);
  తిరిగి SQLITE_OK;
}

పూర్ణాంకం sqlite3_vfslog_new(
  స్థిరరాశి అక్షరం *zVfs,               /* New VFS name */
  స్థిరరాశి అక్షరం *z_మాతృ_Vfs,         /* Parent VFS name (or NULL) */
  స్థిరరాశి అక్షరం *zLog                /* Log file name */
){
  VfslogVfs *p;
  sqlite3_vfs *p_మాతృ;
  పూర్ణాంకం nByte;
  పూర్ణాంకం flags;
  పూర్ణాంకం rc;
  అక్షరం *zFile;
  పూర్ణాంకం nVfs;

  p_మాతృ = sqlite3_vfs_కనుగొను(z_మాతృ_Vfs);
  ఒకవేళ( !p_మాతృ ){
    తిరిగి SQLITE_ERROR;
  }

  nVfs = (పూర్ణాంకం)అక్షరశ్రేణి_పొడవు(zVfs);
  nByte = పరిమాణం(VfslogVfs) + p_మాతృ->szOsFile + nVfs+1+p_మాతృ->mxPathname+1;
  p = (VfslogVfs *)sqlite3_malloc(nByte);
  memset(p, 0, nByte);

  p->pVfs = p_మాతృ;
  p->pLog = (sqlite3_file *)&p[1];
  జ్ఞాపకం_నకలు(&p->పునాది, &vfslog_vfs, పరిమాణం(sqlite3_vfs));
  p->పునాది.zName = &((అక్షరం *)p->pLog)[p_మాతృ->szOsFile];
  p->పునాది.szOsFile += p_మాతృ->szOsFile;
  జ్ఞాపకం_నకలు((అక్షరం *)p->పునాది.zName, zVfs, nVfs);

  zFile = (అక్షరం *)&p->పునాది.zName[nVfs+1];
  p_మాతృ->x_నిండిన_Pathname(p_మాతృ, zLog, p_మాతృ->mxPathname, zFile);

  flags = SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE|SQLITE_OPEN_SUPER_JOURNAL;
  p_మాతృ->xDelete(p_మాతృ, zFile, 0);
  rc = p_మాతృ->xOpen(p_మాతృ, zFile, p->pLog, flags, &flags);
  ఒకవేళ( rc==SQLITE_OK ){
    జ్ఞాపకం_నకలు(p->aBuf, "sqlite_ostrace1.....", 20);
    p->i_స్థానభ్రంశం = 0;
    p->nBuf = 20;
    rc = sqlite3_vfs_రిజిస్టరు((sqlite3_vfs *)p, 1);
  }
  ఒకవేళ( rc ){
    vfslog_finalize(p);
  }
  తిరిగి rc;
}

పూర్ణాంకం sqlite3_vfslog_annotate(స్థిరరాశి అక్షరం *zVfs, స్థిరరాశి అక్షరం *z_సందేశం){
  sqlite3_vfs *pVfs;
  pVfs = sqlite3_vfs_కనుగొను(zVfs);
  ఒకవేళ( !pVfs || pVfs->xOpen!=vfslogOpen ){
    తిరిగి SQLITE_ERROR;
  } 
  vfslog_call(pVfs, OS_ANNOTATE, 0, 0, 0, 0, 0);
  vfslog_string(pVfs, z_సందేశం);
  తిరిగి SQLITE_OK;
}

స్థిర స్థిరరాశి అక్షరం *vfslog_eventname(పూర్ణాంకం eEvent){
  స్థిరరాశి అక్షరం *zEvent = 0;

  ఎంపిక( eEvent ){
    స్థితి OS_CLOSE:             zEvent = "xClose"; ఆపు;
    స్థితి OS_READ:              zEvent = "xRead"; ఆపు;
    స్థితి OS_WRITE:             zEvent = "xWrite"; ఆపు;
    స్థితి OS_TRUNCATE:          zEvent = "xTruncate"; ఆపు;
    స్థితి OS_SYNC:              zEvent = "xSync"; ఆపు;
    స్థితి OS_FILESIZE:          zEvent = "xFilesize"; ఆపు;
    స్థితి OS_తాళం:              zEvent = "xLock"; ఆపు;
    స్థితి OS_UNLOCK:            zEvent = "xUnlock"; ఆపు;
    స్థితి OS_CHECKRESERVEDLOCK: zEvent = "xCheckResLock"; ఆపు;
    స్థితి OS_FILECONTROL:       zEvent = "xFileControl"; ఆపు;
    స్థితి OS_SECTORSIZE:        zEvent = "xSectorSize"; ఆపు;
    స్థితి OS_DEVCHAR:           zEvent = "xDeviceChar"; ఆపు;
    స్థితి OS_OPEN:              zEvent = "xOpen"; ఆపు;
    స్థితి OS_DELETE:            zEvent = "xDelete"; ఆపు;
    స్థితి OS_ACCESS:            zEvent = "xAccess"; ఆపు;
    స్థితి OS_FULLPATHNAME:      zEvent = "xFullPathname"; ఆపు;
    స్థితి OS_RANDOMNESS:        zEvent = "xRandomness"; ఆపు;
    స్థితి OS_SLEEP:             zEvent = "xSleep"; ఆపు;
    స్థితి OS_CURRENTTIME:       zEvent = "xCurrentTime"; ఆపు;

    స్థితి OS_SHMUNMAP:          zEvent = "xShmUnmap"; ఆపు;
    స్థితి OS_SHMLOCK:           zEvent = "xShmLock"; ఆపు;
    స్థితి OS_SHMBARRIER:        zEvent = "xShmBarrier"; ఆపు;
    స్థితి OS_SHMMAP:            zEvent = "xShmMap"; ఆపు;

    స్థితి OS_ANNOTATE:          zEvent = "annotation"; ఆపు;
  }

  తిరిగి zEvent;
}

రకనిర్వచనం నిర్మాణం VfslogVtab VfslogVtab;
రకనిర్వచనం నిర్మాణం VfslogCsr VfslogCsr;

/*
** Virtual table type for the vfslog reader module.
*/
నిర్మాణం VfslogVtab {
  sqlite3_vtab పునాది;              /* Base class */
  sqlite3_file *pFd;              /* File descriptor open on vfslog file */
  sqlite3_int64 nByte;            /* Size of file in bytes */
  అక్షరం *zFile;                    /* File name for pFd */
};

/*
** Virtual table cursor type for the vfslog reader module.
*/
నిర్మాణం VfslogCsr {
  sqlite3_vtab_cursor పునాది;       /* Base class */
  sqlite3_int64 iRowid;           /* Current rowid. */
  sqlite3_int64 i_స్థానభ్రంశం;          /* Offset of next record in file */
  అక్షరం *zTransient;               /* Transient 'file' string */
  పూర్ణాంకం nFile;                      /* Size of array azFile[] */
  అక్షరం **azFile;                  /* File strings */
  గుర్తులేని అక్షరం aBuf[1024];       /* Current vfs log entry (read from file) */
};

స్థిర గుర్తులేని పూర్ణాంకం get32bits(గుర్తులేని అక్షరం *p){
  తిరిగి (p[0]<<24) + (p[1]<<16) + (p[2]<<8) + p[3];
}

/*
** The argument must point to a buffer containing a nul-terminated string.
** If the string begins with an SQL quote character it is overwritten by
** the dequoted version. Otherwise the buffer is left unmodified.
*/
స్థిర శూన్య dequote(అక్షరం *z){
  అక్షరం quote;                     /* Quote character (if any ) */
  quote = z[0];
  ఒకవేళ( quote=='[' || quote=='\'' || quote=='"' || quote=='`' ){
    పూర్ణాంకం i_లోపల = 1;                  /* Index of next byte to read from input */
    పూర్ణాంకం i_బహిర్గతం = 0;                 /* Index of next byte to write to output */
    ఒకవేళ( quote=='[' ) quote = ']';  
    అయితే( z[i_లోపల] ){
      ఒకవేళ( z[i_లోపల]==quote ){
        ఒకవేళ( z[i_లోపల+1]!=quote ) ఆపు;
        z[i_బహిర్గతం++] = quote;
        i_లోపల += 2;
      }లేకపోతే{
        z[i_బహిర్గతం++] = z[i_లోపల++];
      }
    }
    z[i_బహిర్గతం] = '\0';
  }
}

#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
/*
** Connect to or create a vfslog virtual table.
*/
స్థిర పూర్ణాంకం vlogConnect(
  sqlite3 *db,
  శూన్య *pAux,
  పూర్ణాంకం argc, స్థిరరాశి అక్షరం *స్థిరరాశి*argv,
  sqlite3_vtab **ppVtab,
  అక్షరం **pzErr
){
  sqlite3_vfs *pVfs;              /* VFS used to read log file */
  పూర్ణాంకం flags;                      /* flags passed to pVfs->xOpen() */
  VfslogVtab *p;
  పూర్ణాంకం rc;
  పూర్ణాంకం nByte;
  అక్షరం *zFile;

  *ppVtab = 0;
  pVfs = sqlite3_vfs_కనుగొను(0);
  nByte = పరిమాణం(VfslogVtab) + pVfs->szOsFile + pVfs->mxPathname;
  p = sqlite3_malloc(nByte);
  ఒకవేళ( p==0 ) తిరిగి SQLITE_NOMEM;
  memset(p, 0, nByte);

  p->pFd = (sqlite3_file *)&p[1];
  p->zFile = &((అక్షరం *)p->pFd)[pVfs->szOsFile];

  zFile = sqlite3_mprintf("%s", argv[3]);
  ఒకవేళ( !zFile ){
    sqlite3_free(p);
    తిరిగి SQLITE_NOMEM;
  }
  dequote(zFile);
  pVfs->x_నిండిన_Pathname(pVfs, zFile, pVfs->mxPathname, p->zFile);
  sqlite3_free(zFile);

  flags = SQLITE_OPEN_READWRITE|SQLITE_OPEN_SUPER_JOURNAL;
  rc = pVfs->xOpen(pVfs, p->zFile, p->pFd, flags, &flags);

  ఒకవేళ( rc==SQLITE_OK ){
    p->pFd->pMethods->x_File_కొలత(p->pFd, &p->nByte);
    sqlite3_declare_vtab(db, 
        "CREATE TABLE xxx(event, file, click, rc, size, offset)"
    );
    *ppVtab = &p->పునాది;
  }లేకపోతే{
    sqlite3_free(p);
  }

  తిరిగి rc;
}

/*
** There is no "best-index". This virtual table always does a linear
** scan of the binary VFS log file.
*/
స్థిర పూర్ణాంకం vlog_ఉత్తమ_సూచీ(sqlite3_vtab *tab, sqlite3_సూచీ_info *pIdxInfo){
  pIdxInfo->estimated_ఖర్చు = 10.0;
  తిరిగి SQLITE_OK;
}

/*
** Disconnect from or destroy a vfslog virtual table.
*/
స్థిర పూర్ణాంకం vlogDisconnect(sqlite3_vtab *pVtab){
  VfslogVtab *p = (VfslogVtab *)pVtab;
  ఒకవేళ( p->pFd->pMethods ){
    p->pFd->pMethods->xClose(p->pFd);
    p->pFd->pMethods = 0;
  }
  sqlite3_free(p);
  తిరిగి SQLITE_OK;
}

/*
** Open a new vfslog cursor.
*/
స్థిర పూర్ణాంకం vlogOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  VfslogCsr *pCsr;                /* Newly allocated cursor object */

  pCsr = sqlite3_malloc(పరిమాణం(VfslogCsr));
  ఒకవేళ( !pCsr ) తిరిగి SQLITE_NOMEM;
  memset(pCsr, 0, పరిమాణం(VfslogCsr));
  *ppCursor = &pCsr->పునాది;
  తిరిగి SQLITE_OK;
}

/*
** Close a vfslog cursor.
*/
స్థిర పూర్ణాంకం vlogClose(sqlite3_vtab_cursor *pCursor){
  VfslogCsr *p = (VfslogCsr *)pCursor;
  పూర్ణాంకం i;
  కొరకు(i=0; i<p->nFile; i++){
    sqlite3_free(p->azFile[i]);
  }
  sqlite3_free(p->azFile);
  sqlite3_free(p->zTransient);
  sqlite3_free(p);
  తిరిగి SQLITE_OK;
}

/*
** Move a vfslog cursor to the next entry in the file.
*/
స్థిర పూర్ణాంకం vlog_తదుపరి(sqlite3_vtab_cursor *pCursor){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  VfslogVtab *p = (VfslogVtab *)pCursor->pVtab;
  పూర్ణాంకం rc = SQLITE_OK;
  పూర్ణాంకం nRead;

  sqlite3_free(pCsr->zTransient);
  pCsr->zTransient = 0;

  nRead = 24;
  ఒకవేళ( pCsr->i_స్థానభ్రంశం+nRead<=p->nByte ){
    పూర్ణాంకం eEvent;
    rc = p->pFd->pMethods->xRead(p->pFd, pCsr->aBuf, nRead, pCsr->i_స్థానభ్రంశం);

    eEvent = get32bits(pCsr->aBuf);
    ఒకవేళ( (rc==SQLITE_OK)
     && (eEvent==OS_OPEN || eEvent==OS_DELETE || eEvent==OS_ACCESS) 
    ){
      అక్షరం buf[4];
      rc = p->pFd->pMethods->xRead(p->pFd, buf, 4, pCsr->i_స్థానభ్రంశం+nRead);
      nRead += 4;
      ఒకవేళ( rc==SQLITE_OK ){
        పూర్ణాంకం nStr = get32bits((గుర్తులేని అక్షరం *)buf);
        అక్షరం *zStr = sqlite3_malloc(nStr+1);
        rc = p->pFd->pMethods->xRead(p->pFd, zStr, nStr, pCsr->i_స్థానభ్రంశం+nRead);
        zStr[nStr] = '\0';
        nRead += nStr;

        ఒకవేళ( eEvent==OS_OPEN ){
          పూర్ణాంకం iFileid = get32bits(&pCsr->aBuf[4]);
          ఒకవేళ( iFileid>=pCsr->nFile ){
            పూర్ణాంకం nNew = పరిమాణం(pCsr->azFile[0])*(iFileid+1);
            pCsr->azFile = (అక్షరం **)sqlite3_realloc(pCsr->azFile, nNew);
            nNew -= పరిమాణం(pCsr->azFile[0])*pCsr->nFile;
            memset(&pCsr->azFile[pCsr->nFile], 0, nNew);
            pCsr->nFile = iFileid+1;
          }
          sqlite3_free(pCsr->azFile[iFileid]);
          pCsr->azFile[iFileid] = zStr;
        }లేకపోతే{
          pCsr->zTransient = zStr;
        }
      }
    }
  }

  pCsr->iRowid += 1;
  pCsr->i_స్థానభ్రంశం += nRead;
  తిరిగి rc;
}

స్థిర పూర్ణాంకం vlogEof(sqlite3_vtab_cursor *pCursor){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  VfslogVtab *p = (VfslogVtab *)pCursor->pVtab;
  తిరిగి (pCsr->i_స్థానభ్రంశం>=p->nByte);
}

స్థిర పూర్ణాంకం vlogFilter(
  sqlite3_vtab_cursor *pCursor, 
  పూర్ణాంకం idx_అంకె, స్థిరరాశి అక్షరం *idxStr,
  పూర్ణాంకం argc, sqlite3_విలువ **argv
){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  pCsr->iRowid = 0;
  pCsr->i_స్థానభ్రంశం = 20;
  తిరిగి vlog_తదుపరి(pCursor);
}

స్థిర పూర్ణాంకం vlog_స్తంభం(
  sqlite3_vtab_cursor *pCursor, 
  sqlite3_context *ctx, 
  పూర్ణాంకం i
){
  గుర్తులేని పూర్ణాంకం val;
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;

  ప్రతిజ్ఞా( i<7 );
  val = get32bits(&pCsr->aBuf[4*i]);

  ఎంపిక( i ){
    స్థితి 0: {
      sqlite3_ఫలితం_పాఠ్యం(ctx, vfslog_eventname(val), -1, SQLITE_STATIC);
      ఆపు;
    }
    స్థితి 1: {
      అక్షరం *zStr = pCsr->zTransient;
      ఒకవేళ( val!=0 && val<(గుర్తులేని)pCsr->nFile ){
        zStr = pCsr->azFile[val];
      }
      sqlite3_ఫలితం_పాఠ్యం(ctx, zStr, -1, SQLITE_TRANSIENT);
      ఆపు;
    }
    అప్రమేయ:
      sqlite3_ఫలితం_int(ctx, val);
      ఆపు;
  }

  తిరిగి SQLITE_OK;
}

స్థిర పూర్ణాంకం vlogRowid(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  *pRowid = pCsr->iRowid;
  తిరిగి SQLITE_OK;
}

పూర్ణాంకం sqlite3_vfslog_రిజిస్టరు(sqlite3 *db){
  స్థిర sqlite3_module vfslog_module = {
    0,                            /* iVersion */
    vlogConnect,                /* xCreate */
    vlogConnect,                /* xConnect */
    vlog_ఉత్తమ_సూచీ,              /* xBestIndex */
    vlogDisconnect,             /* xDisconnect */
    vlogDisconnect,             /* xDestroy */
    vlogOpen,                   /* xOpen - open a cursor */
    vlogClose,                  /* xClose - close a cursor */
    vlogFilter,                 /* xFilter - configure scan constraints */
    vlog_తదుపరి,                   /* xNext - advance a cursor */
    vlogEof,                    /* xEof - check for end of scan */
    vlog_స్తంభం,                 /* xColumn - read data */
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
  తిరిగి SQLITE_OK;
}
#అయితే_ముగింపు /* SQLITE_OMIT_VIRTUALTABLE */

/**************************************************************************
***************************************************************************
** Tcl interface starts here.
*/

#ఒకవేళ defined(SQLITE_TEST) || defined(TCLSH)

#అన్తర్భావ "tclsqlite.h"

స్థిర పూర్ణాంకం SQLITE_TCLAPI test_vfslog(
  శూన్య *client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  నిర్మాణం SqliteDb { sqlite3 *db; };
  sqlite3 *db;
  Tcl_CmdInfo cmdInfo;
  పూర్ణాంకం rc = SQLITE_ERROR;

  స్థిర స్థిరరాశి అక్షరం *strs[] = { "annotate", "finalize", "new",  "register", 0 };
  గణన VL_enum { VL_ANNOTATE, VL_FINALIZE, VL_NEW, VL_రిజిస్టరు };
  పూర్ణాంకం iSub;

  ఒకవేళ( objc<2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "SUB-COMMAND ...");
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( Tcl_Get_సూచీ_From_Obj(interp, objv[1], strs, "sub-command", 0, &iSub) ){
    తిరిగి TCL_ERROR;
  }

  ఎంపిక( (గణన VL_enum)iSub ){
    స్థితి VL_ANNOTATE: {
      అక్షరం *zVfs;
      అక్షరం *z_సందేశం;
      ఒకవేళ( objc!=4 ){
        Tcl_Wrong_అంకె_వాదనలు(interp, 3, objv, "VFS");
        తిరిగి TCL_ERROR;
      }
      zVfs = Tcl_GetString(objv[2]);
      z_సందేశం = Tcl_GetString(objv[3]);
      rc = sqlite3_vfslog_annotate(zVfs, z_సందేశం);
      ఒకవేళ( rc!=SQLITE_OK ){
        Tcl_Append_ఫలితం(interp, "failed", (అక్షరం*)0);
        తిరిగి TCL_ERROR;
      }
      ఆపు;
    }
    స్థితి VL_FINALIZE: {
      అక్షరం *zVfs;
      ఒకవేళ( objc!=3 ){
        Tcl_Wrong_అంకె_వాదనలు(interp, 2, objv, "VFS");
        తిరిగి TCL_ERROR;
      }
      zVfs = Tcl_GetString(objv[2]);
      rc = sqlite3_vfslog_finalize(zVfs);
      ఒకవేళ( rc!=SQLITE_OK ){
        Tcl_Append_ఫలితం(interp, "failed", (అక్షరం*)0);
        తిరిగి TCL_ERROR;
      }
      ఆపు;
    };

    స్థితి VL_NEW: {
      అక్షరం *zVfs;
      అక్షరం *z_మాతృ;
      అక్షరం *zLog;
      ఒకవేళ( objc!=5 ){
        Tcl_Wrong_అంకె_వాదనలు(interp, 2, objv, "VFS PARENT LOGFILE");
        తిరిగి TCL_ERROR;
      }
      zVfs = Tcl_GetString(objv[2]);
      z_మాతృ = Tcl_GetString(objv[3]);
      zLog = Tcl_GetString(objv[4]);
      ఒకవేళ( *z_మాతృ=='\0' ) z_మాతృ = 0;
      rc = sqlite3_vfslog_new(zVfs, z_మాతృ, zLog);
      ఒకవేళ( rc!=SQLITE_OK ){
        Tcl_Append_ఫలితం(interp, "failed", (అక్షరం*)0);
        తిరిగి TCL_ERROR;
      }
      ఆపు;
    };

    స్థితి VL_రిజిస్టరు: {
      అక్షరం *zDb;
      ఒకవేళ( objc!=3 ){
        Tcl_Wrong_అంకె_వాదనలు(interp, 2, objv, "DB");
        తిరిగి TCL_ERROR;
      }
#నిర్వచితమైతే SQLITE_OMIT_VIRTUALTABLE
      Tcl_Append_ఫలితం(interp, "vfslog not available because of "
                               "SQLITE_OMIT_VIRTUALTABLE", (శూన్య*)0);
      తిరిగి TCL_ERROR;
#లేకపోతే
      zDb = Tcl_GetString(objv[2]);
      ఒకవేళ( Tcl_GetCommandInfo(interp, zDb, &cmdInfo) ){
        db = ((నిర్మాణం SqliteDb*)cmdInfo.obj_Client_దత్తాంశం)->db;
        rc = sqlite3_vfslog_రిజిస్టరు(db);
      }
      ఒకవేళ( rc!=SQLITE_OK ){
        Tcl_Append_ఫలితం(interp, "bad sqlite3 handle: ", zDb, (శూన్య*)0);
        తిరిగి TCL_ERROR;
      }
      ఆపు;
#అయితే_ముగింపు
    }
  }

  తిరిగి TCL_OK;
}

పూర్ణాంకం SqlitetestOsinst_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "vfslog", test_vfslog, 0, 0);
  తిరిగి TCL_OK;
}

#అయితే_ముగింపు /* SQLITE_TEST */
