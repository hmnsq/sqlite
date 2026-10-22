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

#ضمن "sqlite3.h"

#جيڪڏهن_وصف_ٿيل _WIN32
#ضمن <windows.h>
#جيڪڏهن_پورو

#ضمن <string.h>
#ضمن <assert.h>


/*
** Maximum pathname length supported by the vfslog backend.
*/
#وصف_ڪريو INST_MAX_PATHNAME 512

#وصف_ڪريو OS_ACCESS            1
#وصف_ڪريو OS_CHECKRESERVEDLOCK 2
#وصف_ڪريو OS_CLOSE             3
#وصف_ڪريو OS_CURRENTTIME       4
#وصف_ڪريو OS_DELETE            5
#وصف_ڪريو OS_DEVCHAR           6
#وصف_ڪريو OS_FILECONTROL       7
#وصف_ڪريو OS_FILESIZE          8
#وصف_ڪريو OS_FULLPATHNAME      9
#وصف_ڪريو OS_تالو              11
#وصف_ڪريو OS_OPEN              12
#وصف_ڪريو OS_RANDOMNESS        13
#وصف_ڪريو OS_READ              14 
#وصف_ڪريو OS_SECTORSIZE        15
#وصف_ڪريو OS_SLEEP             16
#وصف_ڪريو OS_SYNC              17
#وصف_ڪريو OS_TRUNCATE          18
#وصف_ڪريو OS_UNLOCK            19
#وصف_ڪريو OS_WRITE             20
#وصف_ڪريو OS_SHMUNMAP          22
#وصف_ڪريو OS_SHMMAP            23
#وصف_ڪريو OS_SHMLOCK           25
#وصف_ڪريو OS_SHMBARRIER        26
#وصف_ڪريو OS_ANNOTATE          28

#وصف_ڪريو OS_NUMEVENTS         29

#وصف_ڪريو VFSLOG_BUFFERSIZE 8192

تعريفنوع بنية VfslogVfs VfslogVfs;
تعريفنوع بنية VfslogFile VfslogFile;

بنية VfslogVfs {
  sqlite3_vfs بنياد;               /* VFS methods */
  sqlite3_vfs *pVfs;              /* Parent VFS */
  صحيح i_ايندڙ_File_Id;                /* Next file id */
  sqlite3_file *pLog;             /* Log file handle */
  sqlite3_int64 i_وٿي;          /* Log file offset of start of write buffer */
  صحيح nBuf;                       /* Number of valid bytes in aBuf[] */
  حرف aBuf[VFSLOG_BUFFERSIZE];   /* Write buffer */
};

بنية VfslogFile {
  sqlite3_file بنياد;              /* IO methods */
  sqlite3_file *pReal;            /* Underlying file handle */
  sqlite3_vfs *pVfslog;           /* Associated VsflogVfs object */
  صحيح iFileId;                    /* File id number */
};

#وصف_ڪريو REALVFS(p) (((VfslogVfs *)(p))->pVfs)



/*
** Method declarations for vfslog_file.
*/
ساكن صحيح vfslogClose(sqlite3_file*);
ساكن صحيح vfslogRead(sqlite3_file*, فراغ*, صحيح iAmt, sqlite3_int64 iOfst);
ساكن صحيح vfslogWrite(sqlite3_file*,ثابت فراغ*,صحيح iAmt, sqlite3_int64 iOfst);
ساكن صحيح vfslogTruncate(sqlite3_file*, sqlite3_int64 سائيز);
ساكن صحيح vfslogSync(sqlite3_file*, صحيح flags);
ساكن صحيح vfslog_File_سائيز(sqlite3_file*, sqlite3_int64 *p_سائيز);
ساكن صحيح vfslog_تالو(sqlite3_file*, صحيح);
ساكن صحيح vfslogUnlock(sqlite3_file*, صحيح);
ساكن صحيح vfslog_Check_Reserved_تالو(sqlite3_file*, صحيح *p_Res_ٻاهريون);
ساكن صحيح vfslogFileControl(sqlite3_file*, صحيح op, فراغ *pArg);
ساكن صحيح vfslog_Sector_سائيز(sqlite3_file*);
ساكن صحيح vfslogDeviceCharacteristics(sqlite3_file*);

ساكن صحيح vfslog_Shm_تالو(sqlite3_file *pFile, صحيح ofst, صحيح n, صحيح flags);
ساكن صحيح vfslogShmMap(sqlite3_file *pFile,صحيح,صحيح,صحيح,متطاير فراغ **);
ساكن فراغ vfslogShmBarrier(sqlite3_file*);
ساكن صحيح vfslogShmUnmap(sqlite3_file *pFile, صحيح deleteFlag);

/*
** Method declarations for vfslog_vfs.
*/
ساكن صحيح vfslogOpen(sqlite3_vfs*, ثابت حرف *, sqlite3_file*, صحيح , صحيح *);
ساكن صحيح vfslogDelete(sqlite3_vfs*, ثابت حرف *zName, صحيح syncDir);
ساكن صحيح vfslogAccess(sqlite3_vfs*, ثابت حرف *zName, صحيح flags, صحيح *);
ساكن صحيح vfslog_ڀريل_Pathname(sqlite3_vfs*, ثابت حرف *zName, صحيح, حرف *z_ٻاهريون);
ساكن فراغ *vfslogDlOpen(sqlite3_vfs*, ثابت حرف *zFilename);
ساكن فراغ vfslogDlError(sqlite3_vfs*, صحيح nByte, حرف *z_Err_نياپو);
ساكن فراغ (*vfslogDlSym(sqlite3_vfs *pVfs, فراغ *p, ثابت حرف*zSym))(فراغ);
ساكن فراغ vfslogDlClose(sqlite3_vfs*, فراغ*);
ساكن صحيح vfslogRandomness(sqlite3_vfs*, صحيح nByte, حرف *z_ٻاهريون);
ساكن صحيح vfslogSleep(sqlite3_vfs*, صحيح microseconds);
ساكن صحيح vfslog_Current_وقت(sqlite3_vfs*, مزدوج*);

ساكن صحيح vfslog_Get_آخري_Error(sqlite3_vfs*, صحيح, حرف *);
ساكن صحيح vfslog_Current_وقت_Int64(sqlite3_vfs*, sqlite3_int64*);

ساكن sqlite3_vfs vfslog_vfs = {
  1,                              /* iVersion */
  حجم(VfslogFile),             /* szOsFile */
  INST_MAX_PATHNAME,              /* mxPathname */
  0,                              /* pNext */
  0,                              /* zName */
  0,                              /* pAppData */
  vfslogOpen,                     /* xOpen */
  vfslogDelete,                   /* xDelete */
  vfslogAccess,                   /* xAccess */
  vfslog_ڀريل_Pathname,             /* xFullPathname */
  vfslogDlOpen,                   /* xDlOpen */
  vfslogDlError,                  /* xDlError */
  vfslogDlSym,                    /* xDlSym */
  vfslogDlClose,                  /* xDlClose */
  vfslogRandomness,               /* xRandomness */
  vfslogSleep,                    /* xSleep */
  vfslog_Current_وقت,              /* xCurrentTime */
  vfslog_Get_آخري_Error,             /* xGetLastError */
  vfslog_Current_وقت_Int64          /* xCurrentTime */
};

ساكن sqlite3_io_methods vfslog_io_methods = {
  2,                              /* iVersion */
  vfslogClose,                    /* xClose */
  vfslogRead,                     /* xRead */
  vfslogWrite,                    /* xWrite */
  vfslogTruncate,                 /* xTruncate */
  vfslogSync,                     /* xSync */
  vfslog_File_سائيز,                 /* xFileSize */
  vfslog_تالو,                     /* xLock */
  vfslogUnlock,                   /* xUnlock */
  vfslog_Check_Reserved_تالو,        /* xCheckReservedLock */
  vfslogFileControl,              /* xFileControl */
  vfslog_Sector_سائيز,               /* xSectorSize */
  vfslogDeviceCharacteristics,    /* xDeviceCharacteristics */
  vfslogShmMap,                   /* xShmMap */
  vfslog_Shm_تالو,                  /* xShmLock */
  vfslogShmBarrier,               /* xShmBarrier */
  vfslogShmUnmap                  /* xShmUnmap */
};

#جيڪڏهن_وصف_ٿيل _WIN32
#ضمن <time.h>
ساكن sqlite3_uint64 vfslog_وقت(){
  FILETIME ft;
  sqlite3_uint64 u64time = 0;
 
  Get_System_وقت_As_File_وقت(&ft);

  u64time |= ft.dw_مٿاهون_Date_وقت;
  u64time <<= 32;
  u64time |= ft.dw_هيٺيون_Date_وقت;

  /* ft is 100-nanosecond intervals, we want microseconds */
  أرجع u64time /(sqlite3_uint64)10;
}
#وإلاإذا !defined(NO_GETTOD)
#ضمن <sys/time.h>
ساكن sqlite3_uint64 vfslog_وقت(){
  بنية timeval s_وقت;
  gettimeofday(&s_وقت, 0);
  أرجع s_وقت.tv_usec + (sqlite3_uint64)s_وقت.tv_sec * 1000000;
}
#وإلا
ساكن sqlite3_uint64 vfslog_وقت(){
  أرجع 0;
}
#جيڪڏهن_پورو

ساكن فراغ vfslog_call(sqlite3_vfs *, صحيح, صحيح, sqlite3_int64, صحيح, صحيح, صحيح);
ساكن فراغ vfslog_string(sqlite3_vfs *, ثابت حرف *);

/*
** Close an vfslog-file.
*/
ساكن صحيح vfslogClose(sqlite3_file *pFile){
  sqlite3_uint64 t;
  صحيح rc = SQLITE_OK;
  VfslogFile *p = (VfslogFile *)pFile;

  t = vfslog_وقت();
  إذا( p->pReal->pMethods ){
    rc = p->pReal->pMethods->xClose(p->pReal);
  }
  t = vfslog_وقت() - t;
  vfslog_call(p->pVfslog, OS_CLOSE, p->iFileId, t, rc, 0, 0);
  أرجع rc;
}

/*
** Read data from an vfslog-file.
*/
ساكن صحيح vfslogRead(
  sqlite3_file *pFile, 
  فراغ *zBuf, 
  صحيح iAmt, 
  sqlite_int64 iOfst
){
  صحيح rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_وقت();
  rc = p->pReal->pMethods->xRead(p->pReal, zBuf, iAmt, iOfst);
  t = vfslog_وقت() - t;
  vfslog_call(p->pVfslog, OS_READ, p->iFileId, t, rc, iAmt, (صحيح)iOfst);
  أرجع rc;
}

/*
** Write data to an vfslog-file.
*/
ساكن صحيح vfslogWrite(
  sqlite3_file *pFile,
  ثابت فراغ *z,
  صحيح iAmt,
  sqlite_int64 iOfst
){
  صحيح rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_وقت();
  rc = p->pReal->pMethods->xWrite(p->pReal, z, iAmt, iOfst);
  t = vfslog_وقت() - t;
  vfslog_call(p->pVfslog, OS_WRITE, p->iFileId, t, rc, iAmt, (صحيح)iOfst);
  أرجع rc;
}

/*
** Truncate an vfslog-file.
*/
ساكن صحيح vfslogTruncate(sqlite3_file *pFile, sqlite_int64 سائيز){
  صحيح rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_وقت();
  rc = p->pReal->pMethods->xTruncate(p->pReal, سائيز);
  t = vfslog_وقت() - t;
  vfslog_call(p->pVfslog, OS_TRUNCATE, p->iFileId, t, rc, 0, (صحيح)سائيز);
  أرجع rc;
}

/*
** Sync an vfslog-file.
*/
ساكن صحيح vfslogSync(sqlite3_file *pFile, صحيح flags){
  صحيح rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_وقت();
  rc = p->pReal->pMethods->xSync(p->pReal, flags);
  t = vfslog_وقت() - t;
  vfslog_call(p->pVfslog, OS_SYNC, p->iFileId, t, rc, flags, 0);
  أرجع rc;
}

/*
** Return the current file-size of an vfslog-file.
*/
ساكن صحيح vfslog_File_سائيز(sqlite3_file *pFile, sqlite_int64 *p_سائيز){
  صحيح rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_وقت();
  rc = p->pReal->pMethods->x_File_سائيز(p->pReal, p_سائيز);
  t = vfslog_وقت() - t;
  vfslog_call(p->pVfslog, OS_FILESIZE, p->iFileId, t, rc, 0, (صحيح)*p_سائيز);
  أرجع rc;
}

/*
** Lock an vfslog-file.
*/
ساكن صحيح vfslog_تالو(sqlite3_file *pFile, صحيح e_تالو){
  صحيح rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_وقت();
  rc = p->pReal->pMethods->x_تالو(p->pReal, e_تالو);
  t = vfslog_وقت() - t;
  vfslog_call(p->pVfslog, OS_تالو, p->iFileId, t, rc, e_تالو, 0);
  أرجع rc;
}

/*
** Unlock an vfslog-file.
*/
ساكن صحيح vfslogUnlock(sqlite3_file *pFile, صحيح e_تالو){
  صحيح rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_وقت();
  rc = p->pReal->pMethods->xUnlock(p->pReal, e_تالو);
  t = vfslog_وقت() - t;
  vfslog_call(p->pVfslog, OS_UNLOCK, p->iFileId, t, rc, e_تالو, 0);
  أرجع rc;
}

/*
** Check if another file-handle holds a RESERVED lock on an vfslog-file.
*/
ساكن صحيح vfslog_Check_Reserved_تالو(sqlite3_file *pFile, صحيح *p_Res_ٻاهريون){
  صحيح rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_وقت();
  rc = p->pReal->pMethods->x_Check_Reserved_تالو(p->pReal, p_Res_ٻاهريون);
  t = vfslog_وقت() - t;
  vfslog_call(p->pVfslog, OS_CHECKRESERVEDLOCK, p->iFileId, t, rc, *p_Res_ٻاهريون, 0);
  أرجع rc;
}

/*
** File control method. For custom operations on an vfslog-file.
*/
ساكن صحيح vfslogFileControl(sqlite3_file *pFile, صحيح op, فراغ *pArg){
  VfslogFile *p = (VfslogFile *)pFile;
  صحيح rc = p->pReal->pMethods->xFileControl(p->pReal, op, pArg);
  إذا( op==SQLITE_FCNTL_VFSNAME && rc==SQLITE_OK ){
    *(حرف**)pArg = sqlite3_mprintf("vfslog/%z", *(حرف**)pArg);
  }
  أرجع rc;
}

/*
** Return the sector-size in bytes for an vfslog-file.
*/
ساكن صحيح vfslog_Sector_سائيز(sqlite3_file *pFile){
  صحيح rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_وقت();
  rc = p->pReal->pMethods->x_Sector_سائيز(p->pReal);
  t = vfslog_وقت() - t;
  vfslog_call(p->pVfslog, OS_SECTORSIZE, p->iFileId, t, rc, 0, 0);
  أرجع rc;
}

/*
** Return the device characteristic flags supported by an vfslog-file.
*/
ساكن صحيح vfslogDeviceCharacteristics(sqlite3_file *pFile){
  صحيح rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_وقت();
  rc = p->pReal->pMethods->xDeviceCharacteristics(p->pReal);
  t = vfslog_وقت() - t;
  vfslog_call(p->pVfslog, OS_DEVCHAR, p->iFileId, t, rc, 0, 0);
  أرجع rc;
}

ساكن صحيح vfslog_Shm_تالو(sqlite3_file *pFile, صحيح ofst, صحيح n, صحيح flags){
  صحيح rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_وقت();
  rc = p->pReal->pMethods->x_Shm_تالو(p->pReal, ofst, n, flags);
  t = vfslog_وقت() - t;
  vfslog_call(p->pVfslog, OS_SHMLOCK, p->iFileId, t, rc, 0, 0);
  أرجع rc;
}
ساكن صحيح vfslogShmMap(
  sqlite3_file *pFile, 
  صحيح iRegion, 
  صحيح szRegion, 
  صحيح isWrite, 
  متطاير فراغ **pp
){
  صحيح rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_وقت();
  rc = p->pReal->pMethods->xShmMap(p->pReal, iRegion, szRegion, isWrite, pp);
  t = vfslog_وقت() - t;
  vfslog_call(p->pVfslog, OS_SHMMAP, p->iFileId, t, rc, 0, 0);
  أرجع rc;
}
ساكن فراغ vfslogShmBarrier(sqlite3_file *pFile){
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_وقت();
  p->pReal->pMethods->xShmBarrier(p->pReal);
  t = vfslog_وقت() - t;
  vfslog_call(p->pVfslog, OS_SHMBARRIER, p->iFileId, t, SQLITE_OK, 0, 0);
}
ساكن صحيح vfslogShmUnmap(sqlite3_file *pFile, صحيح deleteFlag){
  صحيح rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_وقت();
  rc = p->pReal->pMethods->xShmUnmap(p->pReal, deleteFlag);
  t = vfslog_وقت() - t;
  vfslog_call(p->pVfslog, OS_SHMUNMAP, p->iFileId, t, rc, 0, 0);
  أرجع rc;
}


/*
** Open an vfslog file handle.
*/
ساكن صحيح vfslogOpen(
  sqlite3_vfs *pVfs,
  ثابت حرف *zName,
  sqlite3_file *pFile,
  صحيح flags,
  صحيح *p_ٻاهريون_Flags
){
  صحيح rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  VfslogVfs *pLog = (VfslogVfs *)pVfs;

  pFile->pMethods = &vfslog_io_methods;
  p->pReal = (sqlite3_file *)&p[1];
  p->pVfslog = pVfs;
  p->iFileId = ++pLog->i_ايندڙ_File_Id;

  t = vfslog_وقت();
  rc = REALVFS(pVfs)->xOpen(REALVFS(pVfs), zName, p->pReal, flags, p_ٻاهريون_Flags);
  t = vfslog_وقت() - t;

  vfslog_call(pVfs, OS_OPEN, p->iFileId, t, rc, 0, 0);
  vfslog_string(pVfs, zName);
  أرجع rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
ساكن صحيح vfslogDelete(sqlite3_vfs *pVfs, ثابت حرف *z_رستو, صحيح dirSync){
  صحيح rc;
  sqlite3_uint64 t;
  t = vfslog_وقت();
  rc = REALVFS(pVfs)->xDelete(REALVFS(pVfs), z_رستو, dirSync);
  t = vfslog_وقت() - t;
  vfslog_call(pVfs, OS_DELETE, 0, t, rc, dirSync, 0);
  vfslog_string(pVfs, z_رستو);
  أرجع rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
ساكن صحيح vfslogAccess(
  sqlite3_vfs *pVfs, 
  ثابت حرف *z_رستو, 
  صحيح flags, 
  صحيح *p_Res_ٻاهريون
){
  صحيح rc;
  sqlite3_uint64 t;
  t = vfslog_وقت();
  rc = REALVFS(pVfs)->xAccess(REALVFS(pVfs), z_رستو, flags, p_Res_ٻاهريون);
  t = vfslog_وقت() - t;
  vfslog_call(pVfs, OS_ACCESS, 0, t, rc, flags, *p_Res_ٻاهريون);
  vfslog_string(pVfs, z_رستو);
  أرجع rc;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
ساكن صحيح vfslog_ڀريل_Pathname(
  sqlite3_vfs *pVfs, 
  ثابت حرف *z_رستو, 
  صحيح n_ٻاهريون, 
  حرف *z_ٻاهريون
){
  أرجع REALVFS(pVfs)->x_ڀريل_Pathname(REALVFS(pVfs), z_رستو, n_ٻاهريون, z_ٻاهريون);
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
ساكن فراغ *vfslogDlOpen(sqlite3_vfs *pVfs, ثابت حرف *z_رستو){
  أرجع REALVFS(pVfs)->xDlOpen(REALVFS(pVfs), z_رستو);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
ساكن فراغ vfslogDlError(sqlite3_vfs *pVfs, صحيح nByte, حرف *z_Err_نياپو){
  REALVFS(pVfs)->xDlError(REALVFS(pVfs), nByte, z_Err_نياپو);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
ساكن فراغ (*vfslogDlSym(sqlite3_vfs *pVfs, فراغ *p, ثابت حرف *zSym))(فراغ){
  أرجع REALVFS(pVfs)->xDlSym(REALVFS(pVfs), p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
ساكن فراغ vfslogDlClose(sqlite3_vfs *pVfs, فراغ *pHandle){
  REALVFS(pVfs)->xDlClose(REALVFS(pVfs), pHandle);
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
ساكن صحيح vfslogRandomness(sqlite3_vfs *pVfs, صحيح nByte, حرف *z_Buf_ٻاهريون){
  أرجع REALVFS(pVfs)->xRandomness(REALVFS(pVfs), nByte, z_Buf_ٻاهريون);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
ساكن صحيح vfslogSleep(sqlite3_vfs *pVfs, صحيح nMicro){
  أرجع REALVFS(pVfs)->xSleep(REALVFS(pVfs), nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
ساكن صحيح vfslog_Current_وقت(sqlite3_vfs *pVfs, مزدوج *p_وقت_ٻاهريون){
  أرجع REALVFS(pVfs)->x_Current_وقت(REALVFS(pVfs), p_وقت_ٻاهريون);
}

ساكن صحيح vfslog_Get_آخري_Error(sqlite3_vfs *pVfs, صحيح a, حرف *b){
  أرجع REALVFS(pVfs)->x_Get_آخري_Error(REALVFS(pVfs), a, b);
}
ساكن صحيح vfslog_Current_وقت_Int64(sqlite3_vfs *pVfs, sqlite3_int64 *p){
  أرجع REALVFS(pVfs)->x_Current_وقت_Int64(REALVFS(pVfs), p);
}

ساكن فراغ vfslog_flush(VfslogVfs *p){
#جيڪڏهن_وصف_ٿيل SQLITE_TEST
  خارجي صحيح sqlite3_io_error_pending;
  خارجي صحيح sqlite3_io_error_persist;
  خارجي صحيح sqlite3_diskfull_pending;

  صحيح pending = sqlite3_io_error_pending;
  صحيح persist = sqlite3_io_error_persist;
  صحيح diskfull = sqlite3_diskfull_pending;

  sqlite3_io_error_pending = 0;
  sqlite3_io_error_persist = 0;
  sqlite3_diskfull_pending = 0;
#جيڪڏهن_پورو

  إذا( p->nBuf ){
    p->pLog->pMethods->xWrite(p->pLog, p->aBuf, p->nBuf, p->i_وٿي);
    p->i_وٿي += p->nBuf;
    p->nBuf = 0;
  }

#جيڪڏهن_وصف_ٿيل SQLITE_TEST
  sqlite3_io_error_pending = pending;
  sqlite3_io_error_persist = persist;
  sqlite3_diskfull_pending = diskfull;
#جيڪڏهن_پورو
}

ساكن فراغ put32bits(بنانشان حرف *p, بنانشان صحيح v){
  p[0] = v>>24;
  p[1] = (بنانشان حرف)(v>>16);
  p[2] = (بنانشان حرف)(v>>8);
  p[3] = (بنانشان حرف)v;
}

ساكن فراغ vfslog_call(
  sqlite3_vfs *pVfs,
  صحيح eEvent,
  صحيح iFileid,
  sqlite3_int64 nClick,
  صحيح return_ڪوڊ,
  صحيح سائيز,
  صحيح وٿي
){
  VfslogVfs *p = (VfslogVfs *)pVfs;
  بنانشان حرف *zRec;
  إذا( (24+p->nBuf)>حجم(p->aBuf) ){
    vfslog_flush(p);
  }
  zRec = (بنانشان حرف *)&p->aBuf[p->nBuf];
  put32bits(&zRec[0], eEvent);
  put32bits(&zRec[4], iFileid);
  put32bits(&zRec[8], (بنانشان صحيح)(nClick&0xffff));
  put32bits(&zRec[12], return_ڪوڊ);
  put32bits(&zRec[16], سائيز);
  put32bits(&zRec[20], وٿي);
  p->nBuf += 24;
}

ساكن فراغ vfslog_string(sqlite3_vfs *pVfs, ثابت حرف *zStr){
  VfslogVfs *p = (VfslogVfs *)pVfs;
  بنانشان حرف *zRec;
  صحيح nStr = zStr ? (صحيح)لڙي_ڊيگهه(zStr) : 0;
  إذا( (4+nStr+p->nBuf)>حجم(p->aBuf) ){
    vfslog_flush(p);
  }
  zRec = (بنانشان حرف *)&p->aBuf[p->nBuf];
  put32bits(&zRec[0], nStr);
  إذا( zStr ){
    يادگيري_نقل(&zRec[4], zStr, nStr);
  }
  p->nBuf += (4 + nStr);
}

ساكن فراغ vfslog_finalize(VfslogVfs *p){
  إذا( p->pLog->pMethods ){
    vfslog_flush(p);
    p->pLog->pMethods->xClose(p->pLog);
  }
  sqlite3_free(p);
}

صحيح sqlite3_vfslog_finalize(ثابت حرف *zVfs){
  sqlite3_vfs *pVfs;
  pVfs = sqlite3_vfs_ڳوليو(zVfs);
  إذا( !pVfs || pVfs->xOpen!=vfslogOpen ){
    أرجع SQLITE_ERROR;
  } 
  sqlite3_vfs_unregister(pVfs);
  vfslog_finalize((VfslogVfs *)pVfs);
  أرجع SQLITE_OK;
}

صحيح sqlite3_vfslog_new(
  ثابت حرف *zVfs,               /* New VFS name */
  ثابت حرف *z_والد_Vfs,         /* Parent VFS name (or NULL) */
  ثابت حرف *zLog                /* Log file name */
){
  VfslogVfs *p;
  sqlite3_vfs *p_والد;
  صحيح nByte;
  صحيح flags;
  صحيح rc;
  حرف *zFile;
  صحيح nVfs;

  p_والد = sqlite3_vfs_ڳوليو(z_والد_Vfs);
  إذا( !p_والد ){
    أرجع SQLITE_ERROR;
  }

  nVfs = (صحيح)لڙي_ڊيگهه(zVfs);
  nByte = حجم(VfslogVfs) + p_والد->szOsFile + nVfs+1+p_والد->mxPathname+1;
  p = (VfslogVfs *)sqlite3_malloc(nByte);
  memset(p, 0, nByte);

  p->pVfs = p_والد;
  p->pLog = (sqlite3_file *)&p[1];
  يادگيري_نقل(&p->بنياد, &vfslog_vfs, حجم(sqlite3_vfs));
  p->بنياد.zName = &((حرف *)p->pLog)[p_والد->szOsFile];
  p->بنياد.szOsFile += p_والد->szOsFile;
  يادگيري_نقل((حرف *)p->بنياد.zName, zVfs, nVfs);

  zFile = (حرف *)&p->بنياد.zName[nVfs+1];
  p_والد->x_ڀريل_Pathname(p_والد, zLog, p_والد->mxPathname, zFile);

  flags = SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE|SQLITE_OPEN_SUPER_JOURNAL;
  p_والد->xDelete(p_والد, zFile, 0);
  rc = p_والد->xOpen(p_والد, zFile, p->pLog, flags, &flags);
  إذا( rc==SQLITE_OK ){
    يادگيري_نقل(p->aBuf, "sqlite_ostrace1.....", 20);
    p->i_وٿي = 0;
    p->nBuf = 20;
    rc = sqlite3_vfs_رجسٽر((sqlite3_vfs *)p, 1);
  }
  إذا( rc ){
    vfslog_finalize(p);
  }
  أرجع rc;
}

صحيح sqlite3_vfslog_annotate(ثابت حرف *zVfs, ثابت حرف *z_نياپو){
  sqlite3_vfs *pVfs;
  pVfs = sqlite3_vfs_ڳوليو(zVfs);
  إذا( !pVfs || pVfs->xOpen!=vfslogOpen ){
    أرجع SQLITE_ERROR;
  } 
  vfslog_call(pVfs, OS_ANNOTATE, 0, 0, 0, 0, 0);
  vfslog_string(pVfs, z_نياپو);
  أرجع SQLITE_OK;
}

ساكن ثابت حرف *vfslog_eventname(صحيح eEvent){
  ثابت حرف *zEvent = 0;

  بدل( eEvent ){
    حالة OS_CLOSE:             zEvent = "xClose"; اكسر;
    حالة OS_READ:              zEvent = "xRead"; اكسر;
    حالة OS_WRITE:             zEvent = "xWrite"; اكسر;
    حالة OS_TRUNCATE:          zEvent = "xTruncate"; اكسر;
    حالة OS_SYNC:              zEvent = "xSync"; اكسر;
    حالة OS_FILESIZE:          zEvent = "xFilesize"; اكسر;
    حالة OS_تالو:              zEvent = "xLock"; اكسر;
    حالة OS_UNLOCK:            zEvent = "xUnlock"; اكسر;
    حالة OS_CHECKRESERVEDLOCK: zEvent = "xCheckResLock"; اكسر;
    حالة OS_FILECONTROL:       zEvent = "xFileControl"; اكسر;
    حالة OS_SECTORSIZE:        zEvent = "xSectorSize"; اكسر;
    حالة OS_DEVCHAR:           zEvent = "xDeviceChar"; اكسر;
    حالة OS_OPEN:              zEvent = "xOpen"; اكسر;
    حالة OS_DELETE:            zEvent = "xDelete"; اكسر;
    حالة OS_ACCESS:            zEvent = "xAccess"; اكسر;
    حالة OS_FULLPATHNAME:      zEvent = "xFullPathname"; اكسر;
    حالة OS_RANDOMNESS:        zEvent = "xRandomness"; اكسر;
    حالة OS_SLEEP:             zEvent = "xSleep"; اكسر;
    حالة OS_CURRENTTIME:       zEvent = "xCurrentTime"; اكسر;

    حالة OS_SHMUNMAP:          zEvent = "xShmUnmap"; اكسر;
    حالة OS_SHMLOCK:           zEvent = "xShmLock"; اكسر;
    حالة OS_SHMBARRIER:        zEvent = "xShmBarrier"; اكسر;
    حالة OS_SHMMAP:            zEvent = "xShmMap"; اكسر;

    حالة OS_ANNOTATE:          zEvent = "annotation"; اكسر;
  }

  أرجع zEvent;
}

تعريفنوع بنية VfslogVtab VfslogVtab;
تعريفنوع بنية VfslogCsr VfslogCsr;

/*
** Virtual table type for the vfslog reader module.
*/
بنية VfslogVtab {
  sqlite3_vtab بنياد;              /* Base class */
  sqlite3_file *pFd;              /* File descriptor open on vfslog file */
  sqlite3_int64 nByte;            /* Size of file in bytes */
  حرف *zFile;                    /* File name for pFd */
};

/*
** Virtual table cursor type for the vfslog reader module.
*/
بنية VfslogCsr {
  sqlite3_vtab_cursor بنياد;       /* Base class */
  sqlite3_int64 iRowid;           /* Current rowid. */
  sqlite3_int64 i_وٿي;          /* Offset of next record in file */
  حرف *zTransient;               /* Transient 'file' string */
  صحيح nFile;                      /* Size of array azFile[] */
  حرف **azFile;                  /* File strings */
  بنانشان حرف aBuf[1024];       /* Current vfs log entry (read from file) */
};

ساكن بنانشان صحيح get32bits(بنانشان حرف *p){
  أرجع (p[0]<<24) + (p[1]<<16) + (p[2]<<8) + p[3];
}

/*
** The argument must point to a buffer containing a nul-terminated string.
** If the string begins with an SQL quote character it is overwritten by
** the dequoted version. Otherwise the buffer is left unmodified.
*/
ساكن فراغ dequote(حرف *z){
  حرف quote;                     /* Quote character (if any ) */
  quote = z[0];
  إذا( quote=='[' || quote=='\'' || quote=='"' || quote=='`' ){
    صحيح i_اندريون = 1;                  /* Index of next byte to read from input */
    صحيح i_ٻاهريون = 0;                 /* Index of next byte to write to output */
    إذا( quote=='[' ) quote = ']';  
    طالما( z[i_اندريون] ){
      إذا( z[i_اندريون]==quote ){
        إذا( z[i_اندريون+1]!=quote ) اكسر;
        z[i_ٻاهريون++] = quote;
        i_اندريون += 2;
      }وإلا{
        z[i_ٻاهريون++] = z[i_اندريون++];
      }
    }
    z[i_ٻاهريون] = '\0';
  }
}

#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_VIRTUALTABLE
/*
** Connect to or create a vfslog virtual table.
*/
ساكن صحيح vlogConnect(
  sqlite3 *db,
  فراغ *pAux,
  صحيح argc, ثابت حرف *ثابت*argv,
  sqlite3_vtab **ppVtab,
  حرف **pzErr
){
  sqlite3_vfs *pVfs;              /* VFS used to read log file */
  صحيح flags;                      /* flags passed to pVfs->xOpen() */
  VfslogVtab *p;
  صحيح rc;
  صحيح nByte;
  حرف *zFile;

  *ppVtab = 0;
  pVfs = sqlite3_vfs_ڳوليو(0);
  nByte = حجم(VfslogVtab) + pVfs->szOsFile + pVfs->mxPathname;
  p = sqlite3_malloc(nByte);
  إذا( p==0 ) أرجع SQLITE_NOMEM;
  memset(p, 0, nByte);

  p->pFd = (sqlite3_file *)&p[1];
  p->zFile = &((حرف *)p->pFd)[pVfs->szOsFile];

  zFile = sqlite3_mprintf("%s", argv[3]);
  إذا( !zFile ){
    sqlite3_free(p);
    أرجع SQLITE_NOMEM;
  }
  dequote(zFile);
  pVfs->x_ڀريل_Pathname(pVfs, zFile, pVfs->mxPathname, p->zFile);
  sqlite3_free(zFile);

  flags = SQLITE_OPEN_READWRITE|SQLITE_OPEN_SUPER_JOURNAL;
  rc = pVfs->xOpen(pVfs, p->zFile, p->pFd, flags, &flags);

  إذا( rc==SQLITE_OK ){
    p->pFd->pMethods->x_File_سائيز(p->pFd, &p->nByte);
    sqlite3_declare_vtab(db, 
        "CREATE TABLE xxx(event, file, click, rc, size, offset)"
    );
    *ppVtab = &p->بنياد;
  }وإلا{
    sqlite3_free(p);
  }

  أرجع rc;
}

/*
** There is no "best-index". This virtual table always does a linear
** scan of the binary VFS log file.
*/
ساكن صحيح vlog_بهترين_اشاريو(sqlite3_vtab *tab, sqlite3_اشاريو_info *pIdxInfo){
  pIdxInfo->estimated_لاڳت = 10.0;
  أرجع SQLITE_OK;
}

/*
** Disconnect from or destroy a vfslog virtual table.
*/
ساكن صحيح vlogDisconnect(sqlite3_vtab *pVtab){
  VfslogVtab *p = (VfslogVtab *)pVtab;
  إذا( p->pFd->pMethods ){
    p->pFd->pMethods->xClose(p->pFd);
    p->pFd->pMethods = 0;
  }
  sqlite3_free(p);
  أرجع SQLITE_OK;
}

/*
** Open a new vfslog cursor.
*/
ساكن صحيح vlogOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  VfslogCsr *pCsr;                /* Newly allocated cursor object */

  pCsr = sqlite3_malloc(حجم(VfslogCsr));
  إذا( !pCsr ) أرجع SQLITE_NOMEM;
  memset(pCsr, 0, حجم(VfslogCsr));
  *ppCursor = &pCsr->بنياد;
  أرجع SQLITE_OK;
}

/*
** Close a vfslog cursor.
*/
ساكن صحيح vlogClose(sqlite3_vtab_cursor *pCursor){
  VfslogCsr *p = (VfslogCsr *)pCursor;
  صحيح i;
  لكل(i=0; i<p->nFile; i++){
    sqlite3_free(p->azFile[i]);
  }
  sqlite3_free(p->azFile);
  sqlite3_free(p->zTransient);
  sqlite3_free(p);
  أرجع SQLITE_OK;
}

/*
** Move a vfslog cursor to the next entry in the file.
*/
ساكن صحيح vlog_ايندڙ(sqlite3_vtab_cursor *pCursor){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  VfslogVtab *p = (VfslogVtab *)pCursor->pVtab;
  صحيح rc = SQLITE_OK;
  صحيح nRead;

  sqlite3_free(pCsr->zTransient);
  pCsr->zTransient = 0;

  nRead = 24;
  إذا( pCsr->i_وٿي+nRead<=p->nByte ){
    صحيح eEvent;
    rc = p->pFd->pMethods->xRead(p->pFd, pCsr->aBuf, nRead, pCsr->i_وٿي);

    eEvent = get32bits(pCsr->aBuf);
    إذا( (rc==SQLITE_OK)
     && (eEvent==OS_OPEN || eEvent==OS_DELETE || eEvent==OS_ACCESS) 
    ){
      حرف buf[4];
      rc = p->pFd->pMethods->xRead(p->pFd, buf, 4, pCsr->i_وٿي+nRead);
      nRead += 4;
      إذا( rc==SQLITE_OK ){
        صحيح nStr = get32bits((بنانشان حرف *)buf);
        حرف *zStr = sqlite3_malloc(nStr+1);
        rc = p->pFd->pMethods->xRead(p->pFd, zStr, nStr, pCsr->i_وٿي+nRead);
        zStr[nStr] = '\0';
        nRead += nStr;

        إذا( eEvent==OS_OPEN ){
          صحيح iFileid = get32bits(&pCsr->aBuf[4]);
          إذا( iFileid>=pCsr->nFile ){
            صحيح nNew = حجم(pCsr->azFile[0])*(iFileid+1);
            pCsr->azFile = (حرف **)sqlite3_realloc(pCsr->azFile, nNew);
            nNew -= حجم(pCsr->azFile[0])*pCsr->nFile;
            memset(&pCsr->azFile[pCsr->nFile], 0, nNew);
            pCsr->nFile = iFileid+1;
          }
          sqlite3_free(pCsr->azFile[iFileid]);
          pCsr->azFile[iFileid] = zStr;
        }وإلا{
          pCsr->zTransient = zStr;
        }
      }
    }
  }

  pCsr->iRowid += 1;
  pCsr->i_وٿي += nRead;
  أرجع rc;
}

ساكن صحيح vlogEof(sqlite3_vtab_cursor *pCursor){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  VfslogVtab *p = (VfslogVtab *)pCursor->pVtab;
  أرجع (pCsr->i_وٿي>=p->nByte);
}

ساكن صحيح vlogFilter(
  sqlite3_vtab_cursor *pCursor, 
  صحيح idx_انگ, ثابت حرف *idxStr,
  صحيح argc, sqlite3_قدر **argv
){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  pCsr->iRowid = 0;
  pCsr->i_وٿي = 20;
  أرجع vlog_ايندڙ(pCursor);
}

ساكن صحيح vlog_ڪالم(
  sqlite3_vtab_cursor *pCursor, 
  sqlite3_context *ctx, 
  صحيح i
){
  بنانشان صحيح val;
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;

  أكد( i<7 );
  val = get32bits(&pCsr->aBuf[4*i]);

  بدل( i ){
    حالة 0: {
      sqlite3_نتيجو_متن(ctx, vfslog_eventname(val), -1, SQLITE_STATIC);
      اكسر;
    }
    حالة 1: {
      حرف *zStr = pCsr->zTransient;
      إذا( val!=0 && val<(بنانشان)pCsr->nFile ){
        zStr = pCsr->azFile[val];
      }
      sqlite3_نتيجو_متن(ctx, zStr, -1, SQLITE_TRANSIENT);
      اكسر;
    }
    افتراضي:
      sqlite3_نتيجو_int(ctx, val);
      اكسر;
  }

  أرجع SQLITE_OK;
}

ساكن صحيح vlogRowid(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  *pRowid = pCsr->iRowid;
  أرجع SQLITE_OK;
}

صحيح sqlite3_vfslog_رجسٽر(sqlite3 *db){
  ساكن sqlite3_module vfslog_module = {
    0,                            /* iVersion */
    vlogConnect,                /* xCreate */
    vlogConnect,                /* xConnect */
    vlog_بهترين_اشاريو,              /* xBestIndex */
    vlogDisconnect,             /* xDisconnect */
    vlogDisconnect,             /* xDestroy */
    vlogOpen,                   /* xOpen - open a cursor */
    vlogClose,                  /* xClose - close a cursor */
    vlogFilter,                 /* xFilter - configure scan constraints */
    vlog_ايندڙ,                   /* xNext - advance a cursor */
    vlogEof,                    /* xEof - check for end of scan */
    vlog_ڪالم,                 /* xColumn - read data */
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
  أرجع SQLITE_OK;
}
#جيڪڏهن_پورو /* SQLITE_OMIT_VIRTUALTABLE */

/**************************************************************************
***************************************************************************
** Tcl interface starts here.
*/

#إذا defined(SQLITE_TEST) || defined(TCLSH)

#ضمن "tclsqlite.h"

ساكن صحيح SQLITE_TCLAPI test_vfslog(
  فراغ *client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  بنية SqliteDb { sqlite3 *db; };
  sqlite3 *db;
  Tcl_CmdInfo cmdInfo;
  صحيح rc = SQLITE_ERROR;

  ساكن ثابت حرف *strs[] = { "annotate", "finalize", "new",  "register", 0 };
  تعداد VL_enum { VL_ANNOTATE, VL_FINALIZE, VL_NEW, VL_رجسٽر };
  صحيح iSub;

  إذا( objc<2 ){
    Tcl_Wrong_انگ_دليلون(interp, 1, objv, "SUB-COMMAND ...");
    أرجع TCL_ERROR;
  }
  إذا( Tcl_Get_اشاريو_From_Obj(interp, objv[1], strs, "sub-command", 0, &iSub) ){
    أرجع TCL_ERROR;
  }

  بدل( (تعداد VL_enum)iSub ){
    حالة VL_ANNOTATE: {
      حرف *zVfs;
      حرف *z_نياپو;
      إذا( objc!=4 ){
        Tcl_Wrong_انگ_دليلون(interp, 3, objv, "VFS");
        أرجع TCL_ERROR;
      }
      zVfs = Tcl_GetString(objv[2]);
      z_نياپو = Tcl_GetString(objv[3]);
      rc = sqlite3_vfslog_annotate(zVfs, z_نياپو);
      إذا( rc!=SQLITE_OK ){
        Tcl_Append_نتيجو(interp, "failed", (حرف*)0);
        أرجع TCL_ERROR;
      }
      اكسر;
    }
    حالة VL_FINALIZE: {
      حرف *zVfs;
      إذا( objc!=3 ){
        Tcl_Wrong_انگ_دليلون(interp, 2, objv, "VFS");
        أرجع TCL_ERROR;
      }
      zVfs = Tcl_GetString(objv[2]);
      rc = sqlite3_vfslog_finalize(zVfs);
      إذا( rc!=SQLITE_OK ){
        Tcl_Append_نتيجو(interp, "failed", (حرف*)0);
        أرجع TCL_ERROR;
      }
      اكسر;
    };

    حالة VL_NEW: {
      حرف *zVfs;
      حرف *z_والد;
      حرف *zLog;
      إذا( objc!=5 ){
        Tcl_Wrong_انگ_دليلون(interp, 2, objv, "VFS PARENT LOGFILE");
        أرجع TCL_ERROR;
      }
      zVfs = Tcl_GetString(objv[2]);
      z_والد = Tcl_GetString(objv[3]);
      zLog = Tcl_GetString(objv[4]);
      إذا( *z_والد=='\0' ) z_والد = 0;
      rc = sqlite3_vfslog_new(zVfs, z_والد, zLog);
      إذا( rc!=SQLITE_OK ){
        Tcl_Append_نتيجو(interp, "failed", (حرف*)0);
        أرجع TCL_ERROR;
      }
      اكسر;
    };

    حالة VL_رجسٽر: {
      حرف *zDb;
      إذا( objc!=3 ){
        Tcl_Wrong_انگ_دليلون(interp, 2, objv, "DB");
        أرجع TCL_ERROR;
      }
#جيڪڏهن_وصف_ٿيل SQLITE_OMIT_VIRTUALTABLE
      Tcl_Append_نتيجو(interp, "vfslog not available because of "
                               "SQLITE_OMIT_VIRTUALTABLE", (فراغ*)0);
      أرجع TCL_ERROR;
#وإلا
      zDb = Tcl_GetString(objv[2]);
      إذا( Tcl_GetCommandInfo(interp, zDb, &cmdInfo) ){
        db = ((بنية SqliteDb*)cmdInfo.obj_Client_معلومات)->db;
        rc = sqlite3_vfslog_رجسٽر(db);
      }
      إذا( rc!=SQLITE_OK ){
        Tcl_Append_نتيجو(interp, "bad sqlite3 handle: ", zDb, (فراغ*)0);
        أرجع TCL_ERROR;
      }
      اكسر;
#جيڪڏهن_پورو
    }
  }

  أرجع TCL_OK;
}

صحيح SqlitetestOsinst_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "vfslog", test_vfslog, 0, 0);
  أرجع TCL_OK;
}

#جيڪڏهن_پورو /* SQLITE_TEST */
