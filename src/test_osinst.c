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

#ਅਨ੍ਤਰ੍ਭਾਵ "sqlite3.h"

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
#ਅਨ੍ਤਰ੍ਭਾਵ <windows.h>
#ਜੇ_ਅੰਤ

#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>


/*
** Maximum pathname length supported by the vfslog backend.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INST_MAX_PATHNAME 512

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OS_ACCESS            1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OS_CHECKRESERVEDLOCK 2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OS_CLOSE             3
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OS_CURRENTTIME       4
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OS_DELETE            5
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OS_DEVCHAR           6
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OS_FILECONTROL       7
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OS_FILESIZE          8
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OS_FULLPATHNAME      9
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OS_ਤਾਲਾ              11
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OS_OPEN              12
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OS_RANDOMNESS        13
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OS_READ              14 
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OS_SECTORSIZE        15
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OS_SLEEP             16
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OS_SYNC              17
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OS_TRUNCATE          18
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OS_UNLOCK            19
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OS_WRITE             20
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OS_SHMUNMAP          22
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OS_SHMMAP            23
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OS_SHMLOCK           25
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OS_SHMBARRIER        26
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OS_ANNOTATE          28

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OS_NUMEVENTS         29

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VFSLOG_BUFFERSIZE 8192

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ VfslogVfs VfslogVfs;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ VfslogFile VfslogFile;

ਬਣਤਰ VfslogVfs {
  sqlite3_vfs ਅਧਾਰ;               /* VFS methods */
  sqlite3_vfs *pVfs;              /* Parent VFS */
  ਪੂਰਨ i_ਅਗਲਾ_File_Id;                /* Next file id */
  sqlite3_file *pLog;             /* Log file handle */
  sqlite3_int64 i_ਵਿੱਥ;          /* Log file offset of start of write buffer */
  ਪੂਰਨ nBuf;                       /* Number of valid bytes in aBuf[] */
  ਅੱਖਰ aBuf[VFSLOG_BUFFERSIZE];   /* Write buffer */
};

ਬਣਤਰ VfslogFile {
  sqlite3_file ਅਧਾਰ;              /* IO methods */
  sqlite3_file *pReal;            /* Underlying file handle */
  sqlite3_vfs *pVfslog;           /* Associated VsflogVfs object */
  ਪੂਰਨ iFileId;                    /* File id number */
};

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ REALVFS(p) (((VfslogVfs *)(p))->pVfs)



/*
** Method declarations for vfslog_file.
*/
ਸਥਿਰ ਪੂਰਨ vfslogClose(sqlite3_file*);
ਸਥਿਰ ਪੂਰਨ vfslogRead(sqlite3_file*, ਖਾਲੀ*, ਪੂਰਨ iAmt, sqlite3_int64 iOfst);
ਸਥਿਰ ਪੂਰਨ vfslogWrite(sqlite3_file*,ਅਚਲ ਖਾਲੀ*,ਪੂਰਨ iAmt, sqlite3_int64 iOfst);
ਸਥਿਰ ਪੂਰਨ vfslogTruncate(sqlite3_file*, sqlite3_int64 ਮਾਪ);
ਸਥਿਰ ਪੂਰਨ vfslogSync(sqlite3_file*, ਪੂਰਨ flags);
ਸਥਿਰ ਪੂਰਨ vfslog_File_ਮਾਪ(sqlite3_file*, sqlite3_int64 *p_ਮਾਪ);
ਸਥਿਰ ਪੂਰਨ vfslog_ਤਾਲਾ(sqlite3_file*, ਪੂਰਨ);
ਸਥਿਰ ਪੂਰਨ vfslogUnlock(sqlite3_file*, ਪੂਰਨ);
ਸਥਿਰ ਪੂਰਨ vfslog_Check_Reserved_ਤਾਲਾ(sqlite3_file*, ਪੂਰਨ *p_Res_ਬਾਹਰਲਾ);
ਸਥਿਰ ਪੂਰਨ vfslogFileControl(sqlite3_file*, ਪੂਰਨ op, ਖਾਲੀ *pArg);
ਸਥਿਰ ਪੂਰਨ vfslog_Sector_ਮਾਪ(sqlite3_file*);
ਸਥਿਰ ਪੂਰਨ vfslogDeviceCharacteristics(sqlite3_file*);

ਸਥਿਰ ਪੂਰਨ vfslog_Shm_ਤਾਲਾ(sqlite3_file *pFile, ਪੂਰਨ ofst, ਪੂਰਨ n, ਪੂਰਨ flags);
ਸਥਿਰ ਪੂਰਨ vfslogShmMap(sqlite3_file *pFile,ਪੂਰਨ,ਪੂਰਨ,ਪੂਰਨ,ਅਸਥਿਰ ਖਾਲੀ **);
ਸਥਿਰ ਖਾਲੀ vfslogShmBarrier(sqlite3_file*);
ਸਥਿਰ ਪੂਰਨ vfslogShmUnmap(sqlite3_file *pFile, ਪੂਰਨ deleteFlag);

/*
** Method declarations for vfslog_vfs.
*/
ਸਥਿਰ ਪੂਰਨ vfslogOpen(sqlite3_vfs*, ਅਚਲ ਅੱਖਰ *, sqlite3_file*, ਪੂਰਨ , ਪੂਰਨ *);
ਸਥਿਰ ਪੂਰਨ vfslogDelete(sqlite3_vfs*, ਅਚਲ ਅੱਖਰ *zName, ਪੂਰਨ syncDir);
ਸਥਿਰ ਪੂਰਨ vfslogAccess(sqlite3_vfs*, ਅਚਲ ਅੱਖਰ *zName, ਪੂਰਨ flags, ਪੂਰਨ *);
ਸਥਿਰ ਪੂਰਨ vfslog_ਭਰਿਆ_Pathname(sqlite3_vfs*, ਅਚਲ ਅੱਖਰ *zName, ਪੂਰਨ, ਅੱਖਰ *z_ਬਾਹਰਲਾ);
ਸਥਿਰ ਖਾਲੀ *vfslogDlOpen(sqlite3_vfs*, ਅਚਲ ਅੱਖਰ *zFilename);
ਸਥਿਰ ਖਾਲੀ vfslogDlError(sqlite3_vfs*, ਪੂਰਨ nByte, ਅੱਖਰ *z_Err_ਸੁਨੇਹਾ);
ਸਥਿਰ ਖਾਲੀ (*vfslogDlSym(sqlite3_vfs *pVfs, ਖਾਲੀ *p, ਅਚਲ ਅੱਖਰ*zSym))(ਖਾਲੀ);
ਸਥਿਰ ਖਾਲੀ vfslogDlClose(sqlite3_vfs*, ਖਾਲੀ*);
ਸਥਿਰ ਪੂਰਨ vfslogRandomness(sqlite3_vfs*, ਪੂਰਨ nByte, ਅੱਖਰ *z_ਬਾਹਰਲਾ);
ਸਥਿਰ ਪੂਰਨ vfslogSleep(sqlite3_vfs*, ਪੂਰਨ microseconds);
ਸਥਿਰ ਪੂਰਨ vfslog_Current_ਸਮਾਂ(sqlite3_vfs*, ਦੁੱਗਣਾ*);

ਸਥਿਰ ਪੂਰਨ vfslog_Get_ਆਖਰੀ_Error(sqlite3_vfs*, ਪੂਰਨ, ਅੱਖਰ *);
ਸਥਿਰ ਪੂਰਨ vfslog_Current_ਸਮਾਂ_Int64(sqlite3_vfs*, sqlite3_int64*);

ਸਥਿਰ sqlite3_vfs vfslog_vfs = {
  1,                              /* iVersion */
  ਆਕਾਰ(VfslogFile),             /* szOsFile */
  INST_MAX_PATHNAME,              /* mxPathname */
  0,                              /* pNext */
  0,                              /* zName */
  0,                              /* pAppData */
  vfslogOpen,                     /* xOpen */
  vfslogDelete,                   /* xDelete */
  vfslogAccess,                   /* xAccess */
  vfslog_ਭਰਿਆ_Pathname,             /* xFullPathname */
  vfslogDlOpen,                   /* xDlOpen */
  vfslogDlError,                  /* xDlError */
  vfslogDlSym,                    /* xDlSym */
  vfslogDlClose,                  /* xDlClose */
  vfslogRandomness,               /* xRandomness */
  vfslogSleep,                    /* xSleep */
  vfslog_Current_ਸਮਾਂ,              /* xCurrentTime */
  vfslog_Get_ਆਖਰੀ_Error,             /* xGetLastError */
  vfslog_Current_ਸਮਾਂ_Int64          /* xCurrentTime */
};

ਸਥਿਰ sqlite3_io_methods vfslog_io_methods = {
  2,                              /* iVersion */
  vfslogClose,                    /* xClose */
  vfslogRead,                     /* xRead */
  vfslogWrite,                    /* xWrite */
  vfslogTruncate,                 /* xTruncate */
  vfslogSync,                     /* xSync */
  vfslog_File_ਮਾਪ,                 /* xFileSize */
  vfslog_ਤਾਲਾ,                     /* xLock */
  vfslogUnlock,                   /* xUnlock */
  vfslog_Check_Reserved_ਤਾਲਾ,        /* xCheckReservedLock */
  vfslogFileControl,              /* xFileControl */
  vfslog_Sector_ਮਾਪ,               /* xSectorSize */
  vfslogDeviceCharacteristics,    /* xDeviceCharacteristics */
  vfslogShmMap,                   /* xShmMap */
  vfslog_Shm_ਤਾਲਾ,                  /* xShmLock */
  vfslogShmBarrier,               /* xShmBarrier */
  vfslogShmUnmap                  /* xShmUnmap */
};

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
#ਅਨ੍ਤਰ੍ਭਾਵ <time.h>
ਸਥਿਰ sqlite3_uint64 vfslog_ਸਮਾਂ(){
  FILETIME ft;
  sqlite3_uint64 u64time = 0;
 
  Get_System_ਸਮਾਂ_As_File_ਸਮਾਂ(&ft);

  u64time |= ft.dw_ਉੱਚਾ_Date_ਸਮਾਂ;
  u64time <<= 32;
  u64time |= ft.dw_ਨੀਵਾਂ_Date_ਸਮਾਂ;

  /* ft is 100-nanosecond intervals, we want microseconds */
  ਵਾਪਸ u64time /(sqlite3_uint64)10;
}
#ਜਾਂਜੇ !defined(NO_GETTOD)
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/time.h>
ਸਥਿਰ sqlite3_uint64 vfslog_ਸਮਾਂ(){
  ਬਣਤਰ timeval s_ਸਮਾਂ;
  gettimeofday(&s_ਸਮਾਂ, 0);
  ਵਾਪਸ s_ਸਮਾਂ.tv_usec + (sqlite3_uint64)s_ਸਮਾਂ.tv_sec * 1000000;
}
#ਨਹੀਂਤਾਂ
ਸਥਿਰ sqlite3_uint64 vfslog_ਸਮਾਂ(){
  ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਖਾਲੀ vfslog_call(sqlite3_vfs *, ਪੂਰਨ, ਪੂਰਨ, sqlite3_int64, ਪੂਰਨ, ਪੂਰਨ, ਪੂਰਨ);
ਸਥਿਰ ਖਾਲੀ vfslog_string(sqlite3_vfs *, ਅਚਲ ਅੱਖਰ *);

/*
** Close an vfslog-file.
*/
ਸਥਿਰ ਪੂਰਨ vfslogClose(sqlite3_file *pFile){
  sqlite3_uint64 t;
  ਪੂਰਨ rc = SQLITE_OK;
  VfslogFile *p = (VfslogFile *)pFile;

  t = vfslog_ਸਮਾਂ();
  ਜੇ( p->pReal->pMethods ){
    rc = p->pReal->pMethods->xClose(p->pReal);
  }
  t = vfslog_ਸਮਾਂ() - t;
  vfslog_call(p->pVfslog, OS_CLOSE, p->iFileId, t, rc, 0, 0);
  ਵਾਪਸ rc;
}

/*
** Read data from an vfslog-file.
*/
ਸਥਿਰ ਪੂਰਨ vfslogRead(
  sqlite3_file *pFile, 
  ਖਾਲੀ *zBuf, 
  ਪੂਰਨ iAmt, 
  sqlite_int64 iOfst
){
  ਪੂਰਨ rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_ਸਮਾਂ();
  rc = p->pReal->pMethods->xRead(p->pReal, zBuf, iAmt, iOfst);
  t = vfslog_ਸਮਾਂ() - t;
  vfslog_call(p->pVfslog, OS_READ, p->iFileId, t, rc, iAmt, (ਪੂਰਨ)iOfst);
  ਵਾਪਸ rc;
}

/*
** Write data to an vfslog-file.
*/
ਸਥਿਰ ਪੂਰਨ vfslogWrite(
  sqlite3_file *pFile,
  ਅਚਲ ਖਾਲੀ *z,
  ਪੂਰਨ iAmt,
  sqlite_int64 iOfst
){
  ਪੂਰਨ rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_ਸਮਾਂ();
  rc = p->pReal->pMethods->xWrite(p->pReal, z, iAmt, iOfst);
  t = vfslog_ਸਮਾਂ() - t;
  vfslog_call(p->pVfslog, OS_WRITE, p->iFileId, t, rc, iAmt, (ਪੂਰਨ)iOfst);
  ਵਾਪਸ rc;
}

/*
** Truncate an vfslog-file.
*/
ਸਥਿਰ ਪੂਰਨ vfslogTruncate(sqlite3_file *pFile, sqlite_int64 ਮਾਪ){
  ਪੂਰਨ rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_ਸਮਾਂ();
  rc = p->pReal->pMethods->xTruncate(p->pReal, ਮਾਪ);
  t = vfslog_ਸਮਾਂ() - t;
  vfslog_call(p->pVfslog, OS_TRUNCATE, p->iFileId, t, rc, 0, (ਪੂਰਨ)ਮਾਪ);
  ਵਾਪਸ rc;
}

/*
** Sync an vfslog-file.
*/
ਸਥਿਰ ਪੂਰਨ vfslogSync(sqlite3_file *pFile, ਪੂਰਨ flags){
  ਪੂਰਨ rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_ਸਮਾਂ();
  rc = p->pReal->pMethods->xSync(p->pReal, flags);
  t = vfslog_ਸਮਾਂ() - t;
  vfslog_call(p->pVfslog, OS_SYNC, p->iFileId, t, rc, flags, 0);
  ਵਾਪਸ rc;
}

/*
** Return the current file-size of an vfslog-file.
*/
ਸਥਿਰ ਪੂਰਨ vfslog_File_ਮਾਪ(sqlite3_file *pFile, sqlite_int64 *p_ਮਾਪ){
  ਪੂਰਨ rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_ਸਮਾਂ();
  rc = p->pReal->pMethods->x_File_ਮਾਪ(p->pReal, p_ਮਾਪ);
  t = vfslog_ਸਮਾਂ() - t;
  vfslog_call(p->pVfslog, OS_FILESIZE, p->iFileId, t, rc, 0, (ਪੂਰਨ)*p_ਮਾਪ);
  ਵਾਪਸ rc;
}

/*
** Lock an vfslog-file.
*/
ਸਥਿਰ ਪੂਰਨ vfslog_ਤਾਲਾ(sqlite3_file *pFile, ਪੂਰਨ e_ਤਾਲਾ){
  ਪੂਰਨ rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_ਸਮਾਂ();
  rc = p->pReal->pMethods->x_ਤਾਲਾ(p->pReal, e_ਤਾਲਾ);
  t = vfslog_ਸਮਾਂ() - t;
  vfslog_call(p->pVfslog, OS_ਤਾਲਾ, p->iFileId, t, rc, e_ਤਾਲਾ, 0);
  ਵਾਪਸ rc;
}

/*
** Unlock an vfslog-file.
*/
ਸਥਿਰ ਪੂਰਨ vfslogUnlock(sqlite3_file *pFile, ਪੂਰਨ e_ਤਾਲਾ){
  ਪੂਰਨ rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_ਸਮਾਂ();
  rc = p->pReal->pMethods->xUnlock(p->pReal, e_ਤਾਲਾ);
  t = vfslog_ਸਮਾਂ() - t;
  vfslog_call(p->pVfslog, OS_UNLOCK, p->iFileId, t, rc, e_ਤਾਲਾ, 0);
  ਵਾਪਸ rc;
}

/*
** Check if another file-handle holds a RESERVED lock on an vfslog-file.
*/
ਸਥਿਰ ਪੂਰਨ vfslog_Check_Reserved_ਤਾਲਾ(sqlite3_file *pFile, ਪੂਰਨ *p_Res_ਬਾਹਰਲਾ){
  ਪੂਰਨ rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_ਸਮਾਂ();
  rc = p->pReal->pMethods->x_Check_Reserved_ਤਾਲਾ(p->pReal, p_Res_ਬਾਹਰਲਾ);
  t = vfslog_ਸਮਾਂ() - t;
  vfslog_call(p->pVfslog, OS_CHECKRESERVEDLOCK, p->iFileId, t, rc, *p_Res_ਬਾਹਰਲਾ, 0);
  ਵਾਪਸ rc;
}

/*
** File control method. For custom operations on an vfslog-file.
*/
ਸਥਿਰ ਪੂਰਨ vfslogFileControl(sqlite3_file *pFile, ਪੂਰਨ op, ਖਾਲੀ *pArg){
  VfslogFile *p = (VfslogFile *)pFile;
  ਪੂਰਨ rc = p->pReal->pMethods->xFileControl(p->pReal, op, pArg);
  ਜੇ( op==SQLITE_FCNTL_VFSNAME && rc==SQLITE_OK ){
    *(ਅੱਖਰ**)pArg = sqlite3_mprintf("vfslog/%z", *(ਅੱਖਰ**)pArg);
  }
  ਵਾਪਸ rc;
}

/*
** Return the sector-size in bytes for an vfslog-file.
*/
ਸਥਿਰ ਪੂਰਨ vfslog_Sector_ਮਾਪ(sqlite3_file *pFile){
  ਪੂਰਨ rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_ਸਮਾਂ();
  rc = p->pReal->pMethods->x_Sector_ਮਾਪ(p->pReal);
  t = vfslog_ਸਮਾਂ() - t;
  vfslog_call(p->pVfslog, OS_SECTORSIZE, p->iFileId, t, rc, 0, 0);
  ਵਾਪਸ rc;
}

/*
** Return the device characteristic flags supported by an vfslog-file.
*/
ਸਥਿਰ ਪੂਰਨ vfslogDeviceCharacteristics(sqlite3_file *pFile){
  ਪੂਰਨ rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_ਸਮਾਂ();
  rc = p->pReal->pMethods->xDeviceCharacteristics(p->pReal);
  t = vfslog_ਸਮਾਂ() - t;
  vfslog_call(p->pVfslog, OS_DEVCHAR, p->iFileId, t, rc, 0, 0);
  ਵਾਪਸ rc;
}

ਸਥਿਰ ਪੂਰਨ vfslog_Shm_ਤਾਲਾ(sqlite3_file *pFile, ਪੂਰਨ ofst, ਪੂਰਨ n, ਪੂਰਨ flags){
  ਪੂਰਨ rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_ਸਮਾਂ();
  rc = p->pReal->pMethods->x_Shm_ਤਾਲਾ(p->pReal, ofst, n, flags);
  t = vfslog_ਸਮਾਂ() - t;
  vfslog_call(p->pVfslog, OS_SHMLOCK, p->iFileId, t, rc, 0, 0);
  ਵਾਪਸ rc;
}
ਸਥਿਰ ਪੂਰਨ vfslogShmMap(
  sqlite3_file *pFile, 
  ਪੂਰਨ iRegion, 
  ਪੂਰਨ szRegion, 
  ਪੂਰਨ isWrite, 
  ਅਸਥਿਰ ਖਾਲੀ **pp
){
  ਪੂਰਨ rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_ਸਮਾਂ();
  rc = p->pReal->pMethods->xShmMap(p->pReal, iRegion, szRegion, isWrite, pp);
  t = vfslog_ਸਮਾਂ() - t;
  vfslog_call(p->pVfslog, OS_SHMMAP, p->iFileId, t, rc, 0, 0);
  ਵਾਪਸ rc;
}
ਸਥਿਰ ਖਾਲੀ vfslogShmBarrier(sqlite3_file *pFile){
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_ਸਮਾਂ();
  p->pReal->pMethods->xShmBarrier(p->pReal);
  t = vfslog_ਸਮਾਂ() - t;
  vfslog_call(p->pVfslog, OS_SHMBARRIER, p->iFileId, t, SQLITE_OK, 0, 0);
}
ਸਥਿਰ ਪੂਰਨ vfslogShmUnmap(sqlite3_file *pFile, ਪੂਰਨ deleteFlag){
  ਪੂਰਨ rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_ਸਮਾਂ();
  rc = p->pReal->pMethods->xShmUnmap(p->pReal, deleteFlag);
  t = vfslog_ਸਮਾਂ() - t;
  vfslog_call(p->pVfslog, OS_SHMUNMAP, p->iFileId, t, rc, 0, 0);
  ਵਾਪਸ rc;
}


/*
** Open an vfslog file handle.
*/
ਸਥਿਰ ਪੂਰਨ vfslogOpen(
  sqlite3_vfs *pVfs,
  ਅਚਲ ਅੱਖਰ *zName,
  sqlite3_file *pFile,
  ਪੂਰਨ flags,
  ਪੂਰਨ *p_ਬਾਹਰਲਾ_Flags
){
  ਪੂਰਨ rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  VfslogVfs *pLog = (VfslogVfs *)pVfs;

  pFile->pMethods = &vfslog_io_methods;
  p->pReal = (sqlite3_file *)&p[1];
  p->pVfslog = pVfs;
  p->iFileId = ++pLog->i_ਅਗਲਾ_File_Id;

  t = vfslog_ਸਮਾਂ();
  rc = REALVFS(pVfs)->xOpen(REALVFS(pVfs), zName, p->pReal, flags, p_ਬਾਹਰਲਾ_Flags);
  t = vfslog_ਸਮਾਂ() - t;

  vfslog_call(pVfs, OS_OPEN, p->iFileId, t, rc, 0, 0);
  vfslog_string(pVfs, zName);
  ਵਾਪਸ rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
ਸਥਿਰ ਪੂਰਨ vfslogDelete(sqlite3_vfs *pVfs, ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ, ਪੂਰਨ dirSync){
  ਪੂਰਨ rc;
  sqlite3_uint64 t;
  t = vfslog_ਸਮਾਂ();
  rc = REALVFS(pVfs)->xDelete(REALVFS(pVfs), z_ਰਸਤਾ, dirSync);
  t = vfslog_ਸਮਾਂ() - t;
  vfslog_call(pVfs, OS_DELETE, 0, t, rc, dirSync, 0);
  vfslog_string(pVfs, z_ਰਸਤਾ);
  ਵਾਪਸ rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
ਸਥਿਰ ਪੂਰਨ vfslogAccess(
  sqlite3_vfs *pVfs, 
  ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ, 
  ਪੂਰਨ flags, 
  ਪੂਰਨ *p_Res_ਬਾਹਰਲਾ
){
  ਪੂਰਨ rc;
  sqlite3_uint64 t;
  t = vfslog_ਸਮਾਂ();
  rc = REALVFS(pVfs)->xAccess(REALVFS(pVfs), z_ਰਸਤਾ, flags, p_Res_ਬਾਹਰਲਾ);
  t = vfslog_ਸਮਾਂ() - t;
  vfslog_call(pVfs, OS_ACCESS, 0, t, rc, flags, *p_Res_ਬਾਹਰਲਾ);
  vfslog_string(pVfs, z_ਰਸਤਾ);
  ਵਾਪਸ rc;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
ਸਥਿਰ ਪੂਰਨ vfslog_ਭਰਿਆ_Pathname(
  sqlite3_vfs *pVfs, 
  ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ, 
  ਪੂਰਨ n_ਬਾਹਰਲਾ, 
  ਅੱਖਰ *z_ਬਾਹਰਲਾ
){
  ਵਾਪਸ REALVFS(pVfs)->x_ਭਰਿਆ_Pathname(REALVFS(pVfs), z_ਰਸਤਾ, n_ਬਾਹਰਲਾ, z_ਬਾਹਰਲਾ);
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
ਸਥਿਰ ਖਾਲੀ *vfslogDlOpen(sqlite3_vfs *pVfs, ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ){
  ਵਾਪਸ REALVFS(pVfs)->xDlOpen(REALVFS(pVfs), z_ਰਸਤਾ);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
ਸਥਿਰ ਖਾਲੀ vfslogDlError(sqlite3_vfs *pVfs, ਪੂਰਨ nByte, ਅੱਖਰ *z_Err_ਸੁਨੇਹਾ){
  REALVFS(pVfs)->xDlError(REALVFS(pVfs), nByte, z_Err_ਸੁਨੇਹਾ);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
ਸਥਿਰ ਖਾਲੀ (*vfslogDlSym(sqlite3_vfs *pVfs, ਖਾਲੀ *p, ਅਚਲ ਅੱਖਰ *zSym))(ਖਾਲੀ){
  ਵਾਪਸ REALVFS(pVfs)->xDlSym(REALVFS(pVfs), p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
ਸਥਿਰ ਖਾਲੀ vfslogDlClose(sqlite3_vfs *pVfs, ਖਾਲੀ *pHandle){
  REALVFS(pVfs)->xDlClose(REALVFS(pVfs), pHandle);
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
ਸਥਿਰ ਪੂਰਨ vfslogRandomness(sqlite3_vfs *pVfs, ਪੂਰਨ nByte, ਅੱਖਰ *z_Buf_ਬਾਹਰਲਾ){
  ਵਾਪਸ REALVFS(pVfs)->xRandomness(REALVFS(pVfs), nByte, z_Buf_ਬਾਹਰਲਾ);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
ਸਥਿਰ ਪੂਰਨ vfslogSleep(sqlite3_vfs *pVfs, ਪੂਰਨ nMicro){
  ਵਾਪਸ REALVFS(pVfs)->xSleep(REALVFS(pVfs), nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
ਸਥਿਰ ਪੂਰਨ vfslog_Current_ਸਮਾਂ(sqlite3_vfs *pVfs, ਦੁੱਗਣਾ *p_ਸਮਾਂ_ਬਾਹਰਲਾ){
  ਵਾਪਸ REALVFS(pVfs)->x_Current_ਸਮਾਂ(REALVFS(pVfs), p_ਸਮਾਂ_ਬਾਹਰਲਾ);
}

ਸਥਿਰ ਪੂਰਨ vfslog_Get_ਆਖਰੀ_Error(sqlite3_vfs *pVfs, ਪੂਰਨ a, ਅੱਖਰ *b){
  ਵਾਪਸ REALVFS(pVfs)->x_Get_ਆਖਰੀ_Error(REALVFS(pVfs), a, b);
}
ਸਥਿਰ ਪੂਰਨ vfslog_Current_ਸਮਾਂ_Int64(sqlite3_vfs *pVfs, sqlite3_int64 *p){
  ਵਾਪਸ REALVFS(pVfs)->x_Current_ਸਮਾਂ_Int64(REALVFS(pVfs), p);
}

ਸਥਿਰ ਖਾਲੀ vfslog_flush(VfslogVfs *p){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_TEST
  ਬਾਹਰੀ ਪੂਰਨ sqlite3_io_error_pending;
  ਬਾਹਰੀ ਪੂਰਨ sqlite3_io_error_persist;
  ਬਾਹਰੀ ਪੂਰਨ sqlite3_diskfull_pending;

  ਪੂਰਨ pending = sqlite3_io_error_pending;
  ਪੂਰਨ persist = sqlite3_io_error_persist;
  ਪੂਰਨ diskfull = sqlite3_diskfull_pending;

  sqlite3_io_error_pending = 0;
  sqlite3_io_error_persist = 0;
  sqlite3_diskfull_pending = 0;
#ਜੇ_ਅੰਤ

  ਜੇ( p->nBuf ){
    p->pLog->pMethods->xWrite(p->pLog, p->aBuf, p->nBuf, p->i_ਵਿੱਥ);
    p->i_ਵਿੱਥ += p->nBuf;
    p->nBuf = 0;
  }

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_TEST
  sqlite3_io_error_pending = pending;
  sqlite3_io_error_persist = persist;
  sqlite3_diskfull_pending = diskfull;
#ਜੇ_ਅੰਤ
}

ਸਥਿਰ ਖਾਲੀ put32bits(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *p, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ v){
  p[0] = v>>24;
  p[1] = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)(v>>16);
  p[2] = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)(v>>8);
  p[3] = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)v;
}

ਸਥਿਰ ਖਾਲੀ vfslog_call(
  sqlite3_vfs *pVfs,
  ਪੂਰਨ eEvent,
  ਪੂਰਨ iFileid,
  sqlite3_int64 nClick,
  ਪੂਰਨ return_ਕੋਡ,
  ਪੂਰਨ ਮਾਪ,
  ਪੂਰਨ ਵਿੱਥ
){
  VfslogVfs *p = (VfslogVfs *)pVfs;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *zRec;
  ਜੇ( (24+p->nBuf)>ਆਕਾਰ(p->aBuf) ){
    vfslog_flush(p);
  }
  zRec = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)&p->aBuf[p->nBuf];
  put32bits(&zRec[0], eEvent);
  put32bits(&zRec[4], iFileid);
  put32bits(&zRec[8], (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)(nClick&0xffff));
  put32bits(&zRec[12], return_ਕੋਡ);
  put32bits(&zRec[16], ਮਾਪ);
  put32bits(&zRec[20], ਵਿੱਥ);
  p->nBuf += 24;
}

ਸਥਿਰ ਖਾਲੀ vfslog_string(sqlite3_vfs *pVfs, ਅਚਲ ਅੱਖਰ *zStr){
  VfslogVfs *p = (VfslogVfs *)pVfs;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *zRec;
  ਪੂਰਨ nStr = zStr ? (ਪੂਰਨ)ਲੜੀ_ਲੰਬਾਈ(zStr) : 0;
  ਜੇ( (4+nStr+p->nBuf)>ਆਕਾਰ(p->aBuf) ){
    vfslog_flush(p);
  }
  zRec = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)&p->aBuf[p->nBuf];
  put32bits(&zRec[0], nStr);
  ਜੇ( zStr ){
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(&zRec[4], zStr, nStr);
  }
  p->nBuf += (4 + nStr);
}

ਸਥਿਰ ਖਾਲੀ vfslog_finalize(VfslogVfs *p){
  ਜੇ( p->pLog->pMethods ){
    vfslog_flush(p);
    p->pLog->pMethods->xClose(p->pLog);
  }
  sqlite3_free(p);
}

ਪੂਰਨ sqlite3_vfslog_finalize(ਅਚਲ ਅੱਖਰ *zVfs){
  sqlite3_vfs *pVfs;
  pVfs = sqlite3_vfs_ਲੱਭੋ(zVfs);
  ਜੇ( !pVfs || pVfs->xOpen!=vfslogOpen ){
    ਵਾਪਸ SQLITE_ERROR;
  } 
  sqlite3_vfs_unregister(pVfs);
  vfslog_finalize((VfslogVfs *)pVfs);
  ਵਾਪਸ SQLITE_OK;
}

ਪੂਰਨ sqlite3_vfslog_new(
  ਅਚਲ ਅੱਖਰ *zVfs,               /* New VFS name */
  ਅਚਲ ਅੱਖਰ *z_ਮਾਪਾ_Vfs,         /* Parent VFS name (or NULL) */
  ਅਚਲ ਅੱਖਰ *zLog                /* Log file name */
){
  VfslogVfs *p;
  sqlite3_vfs *p_ਮਾਪਾ;
  ਪੂਰਨ nByte;
  ਪੂਰਨ flags;
  ਪੂਰਨ rc;
  ਅੱਖਰ *zFile;
  ਪੂਰਨ nVfs;

  p_ਮਾਪਾ = sqlite3_vfs_ਲੱਭੋ(z_ਮਾਪਾ_Vfs);
  ਜੇ( !p_ਮਾਪਾ ){
    ਵਾਪਸ SQLITE_ERROR;
  }

  nVfs = (ਪੂਰਨ)ਲੜੀ_ਲੰਬਾਈ(zVfs);
  nByte = ਆਕਾਰ(VfslogVfs) + p_ਮਾਪਾ->szOsFile + nVfs+1+p_ਮਾਪਾ->mxPathname+1;
  p = (VfslogVfs *)sqlite3_malloc(nByte);
  memset(p, 0, nByte);

  p->pVfs = p_ਮਾਪਾ;
  p->pLog = (sqlite3_file *)&p[1];
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(&p->ਅਧਾਰ, &vfslog_vfs, ਆਕਾਰ(sqlite3_vfs));
  p->ਅਧਾਰ.zName = &((ਅੱਖਰ *)p->pLog)[p_ਮਾਪਾ->szOsFile];
  p->ਅਧਾਰ.szOsFile += p_ਮਾਪਾ->szOsFile;
  ਸਮ੍ਰਿਤੀ_ਨਕਲ((ਅੱਖਰ *)p->ਅਧਾਰ.zName, zVfs, nVfs);

  zFile = (ਅੱਖਰ *)&p->ਅਧਾਰ.zName[nVfs+1];
  p_ਮਾਪਾ->x_ਭਰਿਆ_Pathname(p_ਮਾਪਾ, zLog, p_ਮਾਪਾ->mxPathname, zFile);

  flags = SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE|SQLITE_OPEN_SUPER_JOURNAL;
  p_ਮਾਪਾ->xDelete(p_ਮਾਪਾ, zFile, 0);
  rc = p_ਮਾਪਾ->xOpen(p_ਮਾਪਾ, zFile, p->pLog, flags, &flags);
  ਜੇ( rc==SQLITE_OK ){
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(p->aBuf, "sqlite_ostrace1.....", 20);
    p->i_ਵਿੱਥ = 0;
    p->nBuf = 20;
    rc = sqlite3_vfs_ਰਜਿਸਟਰ((sqlite3_vfs *)p, 1);
  }
  ਜੇ( rc ){
    vfslog_finalize(p);
  }
  ਵਾਪਸ rc;
}

ਪੂਰਨ sqlite3_vfslog_annotate(ਅਚਲ ਅੱਖਰ *zVfs, ਅਚਲ ਅੱਖਰ *z_ਸੁਨੇਹਾ){
  sqlite3_vfs *pVfs;
  pVfs = sqlite3_vfs_ਲੱਭੋ(zVfs);
  ਜੇ( !pVfs || pVfs->xOpen!=vfslogOpen ){
    ਵਾਪਸ SQLITE_ERROR;
  } 
  vfslog_call(pVfs, OS_ANNOTATE, 0, 0, 0, 0, 0);
  vfslog_string(pVfs, z_ਸੁਨੇਹਾ);
  ਵਾਪਸ SQLITE_OK;
}

ਸਥਿਰ ਅਚਲ ਅੱਖਰ *vfslog_eventname(ਪੂਰਨ eEvent){
  ਅਚਲ ਅੱਖਰ *zEvent = 0;

  ਚੋਣ( eEvent ){
    ਹਾਲਤ OS_CLOSE:             zEvent = "xClose"; ਰੋਕੋ;
    ਹਾਲਤ OS_READ:              zEvent = "xRead"; ਰੋਕੋ;
    ਹਾਲਤ OS_WRITE:             zEvent = "xWrite"; ਰੋਕੋ;
    ਹਾਲਤ OS_TRUNCATE:          zEvent = "xTruncate"; ਰੋਕੋ;
    ਹਾਲਤ OS_SYNC:              zEvent = "xSync"; ਰੋਕੋ;
    ਹਾਲਤ OS_FILESIZE:          zEvent = "xFilesize"; ਰੋਕੋ;
    ਹਾਲਤ OS_ਤਾਲਾ:              zEvent = "xLock"; ਰੋਕੋ;
    ਹਾਲਤ OS_UNLOCK:            zEvent = "xUnlock"; ਰੋਕੋ;
    ਹਾਲਤ OS_CHECKRESERVEDLOCK: zEvent = "xCheckResLock"; ਰੋਕੋ;
    ਹਾਲਤ OS_FILECONTROL:       zEvent = "xFileControl"; ਰੋਕੋ;
    ਹਾਲਤ OS_SECTORSIZE:        zEvent = "xSectorSize"; ਰੋਕੋ;
    ਹਾਲਤ OS_DEVCHAR:           zEvent = "xDeviceChar"; ਰੋਕੋ;
    ਹਾਲਤ OS_OPEN:              zEvent = "xOpen"; ਰੋਕੋ;
    ਹਾਲਤ OS_DELETE:            zEvent = "xDelete"; ਰੋਕੋ;
    ਹਾਲਤ OS_ACCESS:            zEvent = "xAccess"; ਰੋਕੋ;
    ਹਾਲਤ OS_FULLPATHNAME:      zEvent = "xFullPathname"; ਰੋਕੋ;
    ਹਾਲਤ OS_RANDOMNESS:        zEvent = "xRandomness"; ਰੋਕੋ;
    ਹਾਲਤ OS_SLEEP:             zEvent = "xSleep"; ਰੋਕੋ;
    ਹਾਲਤ OS_CURRENTTIME:       zEvent = "xCurrentTime"; ਰੋਕੋ;

    ਹਾਲਤ OS_SHMUNMAP:          zEvent = "xShmUnmap"; ਰੋਕੋ;
    ਹਾਲਤ OS_SHMLOCK:           zEvent = "xShmLock"; ਰੋਕੋ;
    ਹਾਲਤ OS_SHMBARRIER:        zEvent = "xShmBarrier"; ਰੋਕੋ;
    ਹਾਲਤ OS_SHMMAP:            zEvent = "xShmMap"; ਰੋਕੋ;

    ਹਾਲਤ OS_ANNOTATE:          zEvent = "annotation"; ਰੋਕੋ;
  }

  ਵਾਪਸ zEvent;
}

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ VfslogVtab VfslogVtab;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ VfslogCsr VfslogCsr;

/*
** Virtual table type for the vfslog reader module.
*/
ਬਣਤਰ VfslogVtab {
  sqlite3_vtab ਅਧਾਰ;              /* Base class */
  sqlite3_file *pFd;              /* File descriptor open on vfslog file */
  sqlite3_int64 nByte;            /* Size of file in bytes */
  ਅੱਖਰ *zFile;                    /* File name for pFd */
};

/*
** Virtual table cursor type for the vfslog reader module.
*/
ਬਣਤਰ VfslogCsr {
  sqlite3_vtab_cursor ਅਧਾਰ;       /* Base class */
  sqlite3_int64 iRowid;           /* Current rowid. */
  sqlite3_int64 i_ਵਿੱਥ;          /* Offset of next record in file */
  ਅੱਖਰ *zTransient;               /* Transient 'file' string */
  ਪੂਰਨ nFile;                      /* Size of array azFile[] */
  ਅੱਖਰ **azFile;                  /* File strings */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ aBuf[1024];       /* Current vfs log entry (read from file) */
};

ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ get32bits(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *p){
  ਵਾਪਸ (p[0]<<24) + (p[1]<<16) + (p[2]<<8) + p[3];
}

/*
** The argument must point to a buffer containing a nul-terminated string.
** If the string begins with an SQL quote character it is overwritten by
** the dequoted version. Otherwise the buffer is left unmodified.
*/
ਸਥਿਰ ਖਾਲੀ dequote(ਅੱਖਰ *z){
  ਅੱਖਰ quote;                     /* Quote character (if any ) */
  quote = z[0];
  ਜੇ( quote=='[' || quote=='\'' || quote=='"' || quote=='`' ){
    ਪੂਰਨ i_ਵਿੱਚ = 1;                  /* Index of next byte to read from input */
    ਪੂਰਨ i_ਬਾਹਰਲਾ = 0;                 /* Index of next byte to write to output */
    ਜੇ( quote=='[' ) quote = ']';  
    ਜਦੋਂ( z[i_ਵਿੱਚ] ){
      ਜੇ( z[i_ਵਿੱਚ]==quote ){
        ਜੇ( z[i_ਵਿੱਚ+1]!=quote ) ਰੋਕੋ;
        z[i_ਬਾਹਰਲਾ++] = quote;
        i_ਵਿੱਚ += 2;
      }ਨਹੀਂਤਾਂ{
        z[i_ਬਾਹਰਲਾ++] = z[i_ਵਿੱਚ++];
      }
    }
    z[i_ਬਾਹਰਲਾ] = '\0';
  }
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
/*
** Connect to or create a vfslog virtual table.
*/
ਸਥਿਰ ਪੂਰਨ vlogConnect(
  sqlite3 *db,
  ਖਾਲੀ *pAux,
  ਪੂਰਨ argc, ਅਚਲ ਅੱਖਰ *ਅਚਲ*argv,
  sqlite3_vtab **ppVtab,
  ਅੱਖਰ **pzErr
){
  sqlite3_vfs *pVfs;              /* VFS used to read log file */
  ਪੂਰਨ flags;                      /* flags passed to pVfs->xOpen() */
  VfslogVtab *p;
  ਪੂਰਨ rc;
  ਪੂਰਨ nByte;
  ਅੱਖਰ *zFile;

  *ppVtab = 0;
  pVfs = sqlite3_vfs_ਲੱਭੋ(0);
  nByte = ਆਕਾਰ(VfslogVtab) + pVfs->szOsFile + pVfs->mxPathname;
  p = sqlite3_malloc(nByte);
  ਜੇ( p==0 ) ਵਾਪਸ SQLITE_NOMEM;
  memset(p, 0, nByte);

  p->pFd = (sqlite3_file *)&p[1];
  p->zFile = &((ਅੱਖਰ *)p->pFd)[pVfs->szOsFile];

  zFile = sqlite3_mprintf("%s", argv[3]);
  ਜੇ( !zFile ){
    sqlite3_free(p);
    ਵਾਪਸ SQLITE_NOMEM;
  }
  dequote(zFile);
  pVfs->x_ਭਰਿਆ_Pathname(pVfs, zFile, pVfs->mxPathname, p->zFile);
  sqlite3_free(zFile);

  flags = SQLITE_OPEN_READWRITE|SQLITE_OPEN_SUPER_JOURNAL;
  rc = pVfs->xOpen(pVfs, p->zFile, p->pFd, flags, &flags);

  ਜੇ( rc==SQLITE_OK ){
    p->pFd->pMethods->x_File_ਮਾਪ(p->pFd, &p->nByte);
    sqlite3_declare_vtab(db, 
        "CREATE TABLE xxx(event, file, click, rc, size, offset)"
    );
    *ppVtab = &p->ਅਧਾਰ;
  }ਨਹੀਂਤਾਂ{
    sqlite3_free(p);
  }

  ਵਾਪਸ rc;
}

/*
** There is no "best-index". This virtual table always does a linear
** scan of the binary VFS log file.
*/
ਸਥਿਰ ਪੂਰਨ vlog_ਉੱਤਮ_ਸੂਚਕਾਂਕ(sqlite3_vtab *tab, sqlite3_ਸੂਚਕਾਂਕ_info *pIdxInfo){
  pIdxInfo->estimated_ਲਾਗਤ = 10.0;
  ਵਾਪਸ SQLITE_OK;
}

/*
** Disconnect from or destroy a vfslog virtual table.
*/
ਸਥਿਰ ਪੂਰਨ vlogDisconnect(sqlite3_vtab *pVtab){
  VfslogVtab *p = (VfslogVtab *)pVtab;
  ਜੇ( p->pFd->pMethods ){
    p->pFd->pMethods->xClose(p->pFd);
    p->pFd->pMethods = 0;
  }
  sqlite3_free(p);
  ਵਾਪਸ SQLITE_OK;
}

/*
** Open a new vfslog cursor.
*/
ਸਥਿਰ ਪੂਰਨ vlogOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  VfslogCsr *pCsr;                /* Newly allocated cursor object */

  pCsr = sqlite3_malloc(ਆਕਾਰ(VfslogCsr));
  ਜੇ( !pCsr ) ਵਾਪਸ SQLITE_NOMEM;
  memset(pCsr, 0, ਆਕਾਰ(VfslogCsr));
  *ppCursor = &pCsr->ਅਧਾਰ;
  ਵਾਪਸ SQLITE_OK;
}

/*
** Close a vfslog cursor.
*/
ਸਥਿਰ ਪੂਰਨ vlogClose(sqlite3_vtab_cursor *pCursor){
  VfslogCsr *p = (VfslogCsr *)pCursor;
  ਪੂਰਨ i;
  ਲਈ(i=0; i<p->nFile; i++){
    sqlite3_free(p->azFile[i]);
  }
  sqlite3_free(p->azFile);
  sqlite3_free(p->zTransient);
  sqlite3_free(p);
  ਵਾਪਸ SQLITE_OK;
}

/*
** Move a vfslog cursor to the next entry in the file.
*/
ਸਥਿਰ ਪੂਰਨ vlog_ਅਗਲਾ(sqlite3_vtab_cursor *pCursor){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  VfslogVtab *p = (VfslogVtab *)pCursor->pVtab;
  ਪੂਰਨ rc = SQLITE_OK;
  ਪੂਰਨ nRead;

  sqlite3_free(pCsr->zTransient);
  pCsr->zTransient = 0;

  nRead = 24;
  ਜੇ( pCsr->i_ਵਿੱਥ+nRead<=p->nByte ){
    ਪੂਰਨ eEvent;
    rc = p->pFd->pMethods->xRead(p->pFd, pCsr->aBuf, nRead, pCsr->i_ਵਿੱਥ);

    eEvent = get32bits(pCsr->aBuf);
    ਜੇ( (rc==SQLITE_OK)
     && (eEvent==OS_OPEN || eEvent==OS_DELETE || eEvent==OS_ACCESS) 
    ){
      ਅੱਖਰ buf[4];
      rc = p->pFd->pMethods->xRead(p->pFd, buf, 4, pCsr->i_ਵਿੱਥ+nRead);
      nRead += 4;
      ਜੇ( rc==SQLITE_OK ){
        ਪੂਰਨ nStr = get32bits((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)buf);
        ਅੱਖਰ *zStr = sqlite3_malloc(nStr+1);
        rc = p->pFd->pMethods->xRead(p->pFd, zStr, nStr, pCsr->i_ਵਿੱਥ+nRead);
        zStr[nStr] = '\0';
        nRead += nStr;

        ਜੇ( eEvent==OS_OPEN ){
          ਪੂਰਨ iFileid = get32bits(&pCsr->aBuf[4]);
          ਜੇ( iFileid>=pCsr->nFile ){
            ਪੂਰਨ nNew = ਆਕਾਰ(pCsr->azFile[0])*(iFileid+1);
            pCsr->azFile = (ਅੱਖਰ **)sqlite3_realloc(pCsr->azFile, nNew);
            nNew -= ਆਕਾਰ(pCsr->azFile[0])*pCsr->nFile;
            memset(&pCsr->azFile[pCsr->nFile], 0, nNew);
            pCsr->nFile = iFileid+1;
          }
          sqlite3_free(pCsr->azFile[iFileid]);
          pCsr->azFile[iFileid] = zStr;
        }ਨਹੀਂਤਾਂ{
          pCsr->zTransient = zStr;
        }
      }
    }
  }

  pCsr->iRowid += 1;
  pCsr->i_ਵਿੱਥ += nRead;
  ਵਾਪਸ rc;
}

ਸਥਿਰ ਪੂਰਨ vlogEof(sqlite3_vtab_cursor *pCursor){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  VfslogVtab *p = (VfslogVtab *)pCursor->pVtab;
  ਵਾਪਸ (pCsr->i_ਵਿੱਥ>=p->nByte);
}

ਸਥਿਰ ਪੂਰਨ vlogFilter(
  sqlite3_vtab_cursor *pCursor, 
  ਪੂਰਨ idx_ਅੰਕ, ਅਚਲ ਅੱਖਰ *idxStr,
  ਪੂਰਨ argc, sqlite3_ਮੁੱਲ **argv
){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  pCsr->iRowid = 0;
  pCsr->i_ਵਿੱਥ = 20;
  ਵਾਪਸ vlog_ਅਗਲਾ(pCursor);
}

ਸਥਿਰ ਪੂਰਨ vlog_ਕਾਲਮ(
  sqlite3_vtab_cursor *pCursor, 
  sqlite3_context *ctx, 
  ਪੂਰਨ i
){
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ val;
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;

  ਪ੍ਰਤਿਜ੍ਞਾ( i<7 );
  val = get32bits(&pCsr->aBuf[4*i]);

  ਚੋਣ( i ){
    ਹਾਲਤ 0: {
      sqlite3_ਨਤੀਜਾ_ਪਾਠ(ctx, vfslog_eventname(val), -1, SQLITE_STATIC);
      ਰੋਕੋ;
    }
    ਹਾਲਤ 1: {
      ਅੱਖਰ *zStr = pCsr->zTransient;
      ਜੇ( val!=0 && val<(ਬਿਨਾਨਿਸ਼ਾਨੀ)pCsr->nFile ){
        zStr = pCsr->azFile[val];
      }
      sqlite3_ਨਤੀਜਾ_ਪਾਠ(ctx, zStr, -1, SQLITE_TRANSIENT);
      ਰੋਕੋ;
    }
    ਮੂਲ:
      sqlite3_ਨਤੀਜਾ_int(ctx, val);
      ਰੋਕੋ;
  }

  ਵਾਪਸ SQLITE_OK;
}

ਸਥਿਰ ਪੂਰਨ vlogRowid(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  *pRowid = pCsr->iRowid;
  ਵਾਪਸ SQLITE_OK;
}

ਪੂਰਨ sqlite3_vfslog_ਰਜਿਸਟਰ(sqlite3 *db){
  ਸਥਿਰ sqlite3_module vfslog_module = {
    0,                            /* iVersion */
    vlogConnect,                /* xCreate */
    vlogConnect,                /* xConnect */
    vlog_ਉੱਤਮ_ਸੂਚਕਾਂਕ,              /* xBestIndex */
    vlogDisconnect,             /* xDisconnect */
    vlogDisconnect,             /* xDestroy */
    vlogOpen,                   /* xOpen - open a cursor */
    vlogClose,                  /* xClose - close a cursor */
    vlogFilter,                 /* xFilter - configure scan constraints */
    vlog_ਅਗਲਾ,                   /* xNext - advance a cursor */
    vlogEof,                    /* xEof - check for end of scan */
    vlog_ਕਾਲਮ,                 /* xColumn - read data */
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
  ਵਾਪਸ SQLITE_OK;
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_VIRTUALTABLE */

/**************************************************************************
***************************************************************************
** Tcl interface starts here.
*/

#ਜੇ defined(SQLITE_TEST) || defined(TCLSH)

#ਅਨ੍ਤਰ੍ਭਾਵ "tclsqlite.h"

ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_vfslog(
  ਖਾਲੀ *client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਬਣਤਰ SqliteDb { sqlite3 *db; };
  sqlite3 *db;
  Tcl_CmdInfo cmdInfo;
  ਪੂਰਨ rc = SQLITE_ERROR;

  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *strs[] = { "annotate", "finalize", "new",  "register", 0 };
  ਗਿਣਤੀ VL_enum { VL_ANNOTATE, VL_FINALIZE, VL_NEW, VL_ਰਜਿਸਟਰ };
  ਪੂਰਨ iSub;

  ਜੇ( objc<2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "SUB-COMMAND ...");
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( Tcl_Get_ਸੂਚਕਾਂਕ_From_Obj(interp, objv[1], strs, "sub-command", 0, &iSub) ){
    ਵਾਪਸ TCL_ERROR;
  }

  ਚੋਣ( (ਗਿਣਤੀ VL_enum)iSub ){
    ਹਾਲਤ VL_ANNOTATE: {
      ਅੱਖਰ *zVfs;
      ਅੱਖਰ *z_ਸੁਨੇਹਾ;
      ਜੇ( objc!=4 ){
        Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 3, objv, "VFS");
        ਵਾਪਸ TCL_ERROR;
      }
      zVfs = Tcl_GetString(objv[2]);
      z_ਸੁਨੇਹਾ = Tcl_GetString(objv[3]);
      rc = sqlite3_vfslog_annotate(zVfs, z_ਸੁਨੇਹਾ);
      ਜੇ( rc!=SQLITE_OK ){
        Tcl_Append_ਨਤੀਜਾ(interp, "failed", (ਅੱਖਰ*)0);
        ਵਾਪਸ TCL_ERROR;
      }
      ਰੋਕੋ;
    }
    ਹਾਲਤ VL_FINALIZE: {
      ਅੱਖਰ *zVfs;
      ਜੇ( objc!=3 ){
        Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 2, objv, "VFS");
        ਵਾਪਸ TCL_ERROR;
      }
      zVfs = Tcl_GetString(objv[2]);
      rc = sqlite3_vfslog_finalize(zVfs);
      ਜੇ( rc!=SQLITE_OK ){
        Tcl_Append_ਨਤੀਜਾ(interp, "failed", (ਅੱਖਰ*)0);
        ਵਾਪਸ TCL_ERROR;
      }
      ਰੋਕੋ;
    };

    ਹਾਲਤ VL_NEW: {
      ਅੱਖਰ *zVfs;
      ਅੱਖਰ *z_ਮਾਪਾ;
      ਅੱਖਰ *zLog;
      ਜੇ( objc!=5 ){
        Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 2, objv, "VFS PARENT LOGFILE");
        ਵਾਪਸ TCL_ERROR;
      }
      zVfs = Tcl_GetString(objv[2]);
      z_ਮਾਪਾ = Tcl_GetString(objv[3]);
      zLog = Tcl_GetString(objv[4]);
      ਜੇ( *z_ਮਾਪਾ=='\0' ) z_ਮਾਪਾ = 0;
      rc = sqlite3_vfslog_new(zVfs, z_ਮਾਪਾ, zLog);
      ਜੇ( rc!=SQLITE_OK ){
        Tcl_Append_ਨਤੀਜਾ(interp, "failed", (ਅੱਖਰ*)0);
        ਵਾਪਸ TCL_ERROR;
      }
      ਰੋਕੋ;
    };

    ਹਾਲਤ VL_ਰਜਿਸਟਰ: {
      ਅੱਖਰ *zDb;
      ਜੇ( objc!=3 ){
        Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 2, objv, "DB");
        ਵਾਪਸ TCL_ERROR;
      }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
      Tcl_Append_ਨਤੀਜਾ(interp, "vfslog not available because of "
                               "SQLITE_OMIT_VIRTUALTABLE", (ਖਾਲੀ*)0);
      ਵਾਪਸ TCL_ERROR;
#ਨਹੀਂਤਾਂ
      zDb = Tcl_GetString(objv[2]);
      ਜੇ( Tcl_GetCommandInfo(interp, zDb, &cmdInfo) ){
        db = ((ਬਣਤਰ SqliteDb*)cmdInfo.obj_Client_ਅੰਕੜੇ)->db;
        rc = sqlite3_vfslog_ਰਜਿਸਟਰ(db);
      }
      ਜੇ( rc!=SQLITE_OK ){
        Tcl_Append_ਨਤੀਜਾ(interp, "bad sqlite3 handle: ", zDb, (ਖਾਲੀ*)0);
        ਵਾਪਸ TCL_ERROR;
      }
      ਰੋਕੋ;
#ਜੇ_ਅੰਤ
    }
  }

  ਵਾਪਸ TCL_OK;
}

ਪੂਰਨ SqlitetestOsinst_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "vfslog", test_vfslog, 0, 0);
  ਵਾਪਸ TCL_OK;
}

#ਜੇ_ਅੰਤ /* SQLITE_TEST */
