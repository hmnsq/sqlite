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

#شامل "sqlite3.h"

#اگر_تعریف_شدہ _WIN32
#شامل <windows.h>
#اگر_ختم

#شامل <string.h>
#شامل <assert.h>


/*
** Maximum pathname length supported by the vfslog backend.
*/
#تعریف_کرو INST_MAX_PATHNAME 512

#تعریف_کرو OS_ACCESS            1
#تعریف_کرو OS_CHECKRESERVEDLOCK 2
#تعریف_کرو OS_CLOSE             3
#تعریف_کرو OS_CURRENTTIME       4
#تعریف_کرو OS_DELETE            5
#تعریف_کرو OS_DEVCHAR           6
#تعریف_کرو OS_FILECONTROL       7
#تعریف_کرو OS_FILESIZE          8
#تعریف_کرو OS_FULLPATHNAME      9
#تعریف_کرو OS_تالا              11
#تعریف_کرو OS_OPEN              12
#تعریف_کرو OS_RANDOMNESS        13
#تعریف_کرو OS_READ              14 
#تعریف_کرو OS_SECTORSIZE        15
#تعریف_کرو OS_SLEEP             16
#تعریف_کرو OS_SYNC              17
#تعریف_کرو OS_TRUNCATE          18
#تعریف_کرو OS_UNLOCK            19
#تعریف_کرو OS_WRITE             20
#تعریف_کرو OS_SHMUNMAP          22
#تعریف_کرو OS_SHMMAP            23
#تعریف_کرو OS_SHMLOCK           25
#تعریف_کرو OS_SHMBARRIER        26
#تعریف_کرو OS_ANNOTATE          28

#تعریف_کرو OS_NUMEVENTS         29

#تعریف_کرو VFSLOG_BUFFERSIZE 8192

قسمتعریف ساخت VfslogVfs VfslogVfs;
قسمتعریف ساخت VfslogFile VfslogFile;

ساخت VfslogVfs {
  sqlite3_vfs بنیاد;               /* VFS methods */
  sqlite3_vfs *pVfs;              /* Parent VFS */
  صحیح i_اگلا_File_Id;                /* Next file id */
  sqlite3_file *pLog;             /* Log file handle */
  sqlite3_int64 i_انحراف;          /* Log file offset of start of write buffer */
  صحیح nBuf;                       /* Number of valid bytes in aBuf[] */
  حرف aBuf[VFSLOG_BUFFERSIZE];   /* Write buffer */
};

ساخت VfslogFile {
  sqlite3_file بنیاد;              /* IO methods */
  sqlite3_file *pReal;            /* Underlying file handle */
  sqlite3_vfs *pVfslog;           /* Associated VsflogVfs object */
  صحیح iFileId;                    /* File id number */
};

#تعریف_کرو REALVFS(p) (((VfslogVfs *)(p))->pVfs)



/*
** Method declarations for vfslog_file.
*/
ساکن صحیح vfslogClose(sqlite3_file*);
ساکن صحیح vfslogRead(sqlite3_file*, خالی*, صحیح iAmt, sqlite3_int64 iOfst);
ساکن صحیح vfslogWrite(sqlite3_file*,ثابت خالی*,صحیح iAmt, sqlite3_int64 iOfst);
ساکن صحیح vfslogTruncate(sqlite3_file*, sqlite3_int64 سائز);
ساکن صحیح vfslogSync(sqlite3_file*, صحیح flags);
ساکن صحیح vfslog_File_سائز(sqlite3_file*, sqlite3_int64 *p_سائز);
ساکن صحیح vfslog_تالا(sqlite3_file*, صحیح);
ساکن صحیح vfslogUnlock(sqlite3_file*, صحیح);
ساکن صحیح vfslog_Check_Reserved_تالا(sqlite3_file*, صحیح *p_Res_برآمد);
ساکن صحیح vfslogFileControl(sqlite3_file*, صحیح op, خالی *pArg);
ساکن صحیح vfslog_Sector_سائز(sqlite3_file*);
ساکن صحیح vfslogDeviceCharacteristics(sqlite3_file*);

ساکن صحیح vfslog_Shm_تالا(sqlite3_file *pFile, صحیح ofst, صحیح n, صحیح flags);
ساکن صحیح vfslogShmMap(sqlite3_file *pFile,صحیح,صحیح,صحیح,متلون خالی **);
ساکن خالی vfslogShmBarrier(sqlite3_file*);
ساکن صحیح vfslogShmUnmap(sqlite3_file *pFile, صحیح deleteFlag);

/*
** Method declarations for vfslog_vfs.
*/
ساکن صحیح vfslogOpen(sqlite3_vfs*, ثابت حرف *, sqlite3_file*, صحیح , صحیح *);
ساکن صحیح vfslogDelete(sqlite3_vfs*, ثابت حرف *zName, صحیح syncDir);
ساکن صحیح vfslogAccess(sqlite3_vfs*, ثابت حرف *zName, صحیح flags, صحیح *);
ساکن صحیح vfslog_بھرا_Pathname(sqlite3_vfs*, ثابت حرف *zName, صحیح, حرف *z_برآمد);
ساکن خالی *vfslogDlOpen(sqlite3_vfs*, ثابت حرف *zFilename);
ساکن خالی vfslogDlError(sqlite3_vfs*, صحیح nByte, حرف *z_Err_پیغام);
ساکن خالی (*vfslogDlSym(sqlite3_vfs *pVfs, خالی *p, ثابت حرف*zSym))(خالی);
ساکن خالی vfslogDlClose(sqlite3_vfs*, خالی*);
ساکن صحیح vfslogRandomness(sqlite3_vfs*, صحیح nByte, حرف *z_برآمد);
ساکن صحیح vfslogSleep(sqlite3_vfs*, صحیح microseconds);
ساکن صحیح vfslog_Current_وقت(sqlite3_vfs*, دوہرا*);

ساکن صحیح vfslog_Get_آخری_Error(sqlite3_vfs*, صحیح, حرف *);
ساکن صحیح vfslog_Current_وقت_Int64(sqlite3_vfs*, sqlite3_int64*);

ساکن sqlite3_vfs vfslog_vfs = {
  1,                              /* iVersion */
  حجم(VfslogFile),             /* szOsFile */
  INST_MAX_PATHNAME,              /* mxPathname */
  0,                              /* pNext */
  0,                              /* zName */
  0,                              /* pAppData */
  vfslogOpen,                     /* xOpen */
  vfslogDelete,                   /* xDelete */
  vfslogAccess,                   /* xAccess */
  vfslog_بھرا_Pathname,             /* xFullPathname */
  vfslogDlOpen,                   /* xDlOpen */
  vfslogDlError,                  /* xDlError */
  vfslogDlSym,                    /* xDlSym */
  vfslogDlClose,                  /* xDlClose */
  vfslogRandomness,               /* xRandomness */
  vfslogSleep,                    /* xSleep */
  vfslog_Current_وقت,              /* xCurrentTime */
  vfslog_Get_آخری_Error,             /* xGetLastError */
  vfslog_Current_وقت_Int64          /* xCurrentTime */
};

ساکن sqlite3_io_methods vfslog_io_methods = {
  2,                              /* iVersion */
  vfslogClose,                    /* xClose */
  vfslogRead,                     /* xRead */
  vfslogWrite,                    /* xWrite */
  vfslogTruncate,                 /* xTruncate */
  vfslogSync,                     /* xSync */
  vfslog_File_سائز,                 /* xFileSize */
  vfslog_تالا,                     /* xLock */
  vfslogUnlock,                   /* xUnlock */
  vfslog_Check_Reserved_تالا,        /* xCheckReservedLock */
  vfslogFileControl,              /* xFileControl */
  vfslog_Sector_سائز,               /* xSectorSize */
  vfslogDeviceCharacteristics,    /* xDeviceCharacteristics */
  vfslogShmMap,                   /* xShmMap */
  vfslog_Shm_تالا,                  /* xShmLock */
  vfslogShmBarrier,               /* xShmBarrier */
  vfslogShmUnmap                  /* xShmUnmap */
};

#اگر_تعریف_شدہ _WIN32
#شامل <time.h>
ساکن sqlite3_uint64 vfslog_وقت(){
  FILETIME ft;
  sqlite3_uint64 u64time = 0;
 
  Get_System_وقت_As_File_وقت(&ft);

  u64time |= ft.dw_بلند_Date_وقت;
  u64time <<= 32;
  u64time |= ft.dw_کم_Date_وقت;

  /* ft is 100-nanosecond intervals, we want microseconds */
  واپس u64time /(sqlite3_uint64)10;
}
#وگرنہ_اگر !defined(NO_GETTOD)
#شامل <sys/time.h>
ساکن sqlite3_uint64 vfslog_وقت(){
  ساخت timeval s_وقت;
  gettimeofday(&s_وقت, 0);
  واپس s_وقت.tv_usec + (sqlite3_uint64)s_وقت.tv_sec * 1000000;
}
#ورنہ
ساکن sqlite3_uint64 vfslog_وقت(){
  واپس 0;
}
#اگر_ختم

ساکن خالی vfslog_call(sqlite3_vfs *, صحیح, صحیح, sqlite3_int64, صحیح, صحیح, صحیح);
ساکن خالی vfslog_string(sqlite3_vfs *, ثابت حرف *);

/*
** Close an vfslog-file.
*/
ساکن صحیح vfslogClose(sqlite3_file *pFile){
  sqlite3_uint64 t;
  صحیح rc = SQLITE_OK;
  VfslogFile *p = (VfslogFile *)pFile;

  t = vfslog_وقت();
  اگر( p->pReal->pMethods ){
    rc = p->pReal->pMethods->xClose(p->pReal);
  }
  t = vfslog_وقت() - t;
  vfslog_call(p->pVfslog, OS_CLOSE, p->iFileId, t, rc, 0, 0);
  واپس rc;
}

/*
** Read data from an vfslog-file.
*/
ساکن صحیح vfslogRead(
  sqlite3_file *pFile, 
  خالی *zBuf, 
  صحیح iAmt, 
  sqlite_int64 iOfst
){
  صحیح rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_وقت();
  rc = p->pReal->pMethods->xRead(p->pReal, zBuf, iAmt, iOfst);
  t = vfslog_وقت() - t;
  vfslog_call(p->pVfslog, OS_READ, p->iFileId, t, rc, iAmt, (صحیح)iOfst);
  واپس rc;
}

/*
** Write data to an vfslog-file.
*/
ساکن صحیح vfslogWrite(
  sqlite3_file *pFile,
  ثابت خالی *z,
  صحیح iAmt,
  sqlite_int64 iOfst
){
  صحیح rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_وقت();
  rc = p->pReal->pMethods->xWrite(p->pReal, z, iAmt, iOfst);
  t = vfslog_وقت() - t;
  vfslog_call(p->pVfslog, OS_WRITE, p->iFileId, t, rc, iAmt, (صحیح)iOfst);
  واپس rc;
}

/*
** Truncate an vfslog-file.
*/
ساکن صحیح vfslogTruncate(sqlite3_file *pFile, sqlite_int64 سائز){
  صحیح rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_وقت();
  rc = p->pReal->pMethods->xTruncate(p->pReal, سائز);
  t = vfslog_وقت() - t;
  vfslog_call(p->pVfslog, OS_TRUNCATE, p->iFileId, t, rc, 0, (صحیح)سائز);
  واپس rc;
}

/*
** Sync an vfslog-file.
*/
ساکن صحیح vfslogSync(sqlite3_file *pFile, صحیح flags){
  صحیح rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_وقت();
  rc = p->pReal->pMethods->xSync(p->pReal, flags);
  t = vfslog_وقت() - t;
  vfslog_call(p->pVfslog, OS_SYNC, p->iFileId, t, rc, flags, 0);
  واپس rc;
}

/*
** Return the current file-size of an vfslog-file.
*/
ساکن صحیح vfslog_File_سائز(sqlite3_file *pFile, sqlite_int64 *p_سائز){
  صحیح rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_وقت();
  rc = p->pReal->pMethods->x_File_سائز(p->pReal, p_سائز);
  t = vfslog_وقت() - t;
  vfslog_call(p->pVfslog, OS_FILESIZE, p->iFileId, t, rc, 0, (صحیح)*p_سائز);
  واپس rc;
}

/*
** Lock an vfslog-file.
*/
ساکن صحیح vfslog_تالا(sqlite3_file *pFile, صحیح e_تالا){
  صحیح rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_وقت();
  rc = p->pReal->pMethods->x_تالا(p->pReal, e_تالا);
  t = vfslog_وقت() - t;
  vfslog_call(p->pVfslog, OS_تالا, p->iFileId, t, rc, e_تالا, 0);
  واپس rc;
}

/*
** Unlock an vfslog-file.
*/
ساکن صحیح vfslogUnlock(sqlite3_file *pFile, صحیح e_تالا){
  صحیح rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_وقت();
  rc = p->pReal->pMethods->xUnlock(p->pReal, e_تالا);
  t = vfslog_وقت() - t;
  vfslog_call(p->pVfslog, OS_UNLOCK, p->iFileId, t, rc, e_تالا, 0);
  واپس rc;
}

/*
** Check if another file-handle holds a RESERVED lock on an vfslog-file.
*/
ساکن صحیح vfslog_Check_Reserved_تالا(sqlite3_file *pFile, صحیح *p_Res_برآمد){
  صحیح rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_وقت();
  rc = p->pReal->pMethods->x_Check_Reserved_تالا(p->pReal, p_Res_برآمد);
  t = vfslog_وقت() - t;
  vfslog_call(p->pVfslog, OS_CHECKRESERVEDLOCK, p->iFileId, t, rc, *p_Res_برآمد, 0);
  واپس rc;
}

/*
** File control method. For custom operations on an vfslog-file.
*/
ساکن صحیح vfslogFileControl(sqlite3_file *pFile, صحیح op, خالی *pArg){
  VfslogFile *p = (VfslogFile *)pFile;
  صحیح rc = p->pReal->pMethods->xFileControl(p->pReal, op, pArg);
  اگر( op==SQLITE_FCNTL_VFSNAME && rc==SQLITE_OK ){
    *(حرف**)pArg = sqlite3_mprintf("vfslog/%z", *(حرف**)pArg);
  }
  واپس rc;
}

/*
** Return the sector-size in bytes for an vfslog-file.
*/
ساکن صحیح vfslog_Sector_سائز(sqlite3_file *pFile){
  صحیح rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_وقت();
  rc = p->pReal->pMethods->x_Sector_سائز(p->pReal);
  t = vfslog_وقت() - t;
  vfslog_call(p->pVfslog, OS_SECTORSIZE, p->iFileId, t, rc, 0, 0);
  واپس rc;
}

/*
** Return the device characteristic flags supported by an vfslog-file.
*/
ساکن صحیح vfslogDeviceCharacteristics(sqlite3_file *pFile){
  صحیح rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_وقت();
  rc = p->pReal->pMethods->xDeviceCharacteristics(p->pReal);
  t = vfslog_وقت() - t;
  vfslog_call(p->pVfslog, OS_DEVCHAR, p->iFileId, t, rc, 0, 0);
  واپس rc;
}

ساکن صحیح vfslog_Shm_تالا(sqlite3_file *pFile, صحیح ofst, صحیح n, صحیح flags){
  صحیح rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_وقت();
  rc = p->pReal->pMethods->x_Shm_تالا(p->pReal, ofst, n, flags);
  t = vfslog_وقت() - t;
  vfslog_call(p->pVfslog, OS_SHMLOCK, p->iFileId, t, rc, 0, 0);
  واپس rc;
}
ساکن صحیح vfslogShmMap(
  sqlite3_file *pFile, 
  صحیح iRegion, 
  صحیح szRegion, 
  صحیح isWrite, 
  متلون خالی **pp
){
  صحیح rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_وقت();
  rc = p->pReal->pMethods->xShmMap(p->pReal, iRegion, szRegion, isWrite, pp);
  t = vfslog_وقت() - t;
  vfslog_call(p->pVfslog, OS_SHMMAP, p->iFileId, t, rc, 0, 0);
  واپس rc;
}
ساکن خالی vfslogShmBarrier(sqlite3_file *pFile){
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_وقت();
  p->pReal->pMethods->xShmBarrier(p->pReal);
  t = vfslog_وقت() - t;
  vfslog_call(p->pVfslog, OS_SHMBARRIER, p->iFileId, t, SQLITE_OK, 0, 0);
}
ساکن صحیح vfslogShmUnmap(sqlite3_file *pFile, صحیح deleteFlag){
  صحیح rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_وقت();
  rc = p->pReal->pMethods->xShmUnmap(p->pReal, deleteFlag);
  t = vfslog_وقت() - t;
  vfslog_call(p->pVfslog, OS_SHMUNMAP, p->iFileId, t, rc, 0, 0);
  واپس rc;
}


/*
** Open an vfslog file handle.
*/
ساکن صحیح vfslogOpen(
  sqlite3_vfs *pVfs,
  ثابت حرف *zName,
  sqlite3_file *pFile,
  صحیح flags,
  صحیح *p_برآمد_Flags
){
  صحیح rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  VfslogVfs *pLog = (VfslogVfs *)pVfs;

  pFile->pMethods = &vfslog_io_methods;
  p->pReal = (sqlite3_file *)&p[1];
  p->pVfslog = pVfs;
  p->iFileId = ++pLog->i_اگلا_File_Id;

  t = vfslog_وقت();
  rc = REALVFS(pVfs)->xOpen(REALVFS(pVfs), zName, p->pReal, flags, p_برآمد_Flags);
  t = vfslog_وقت() - t;

  vfslog_call(pVfs, OS_OPEN, p->iFileId, t, rc, 0, 0);
  vfslog_string(pVfs, zName);
  واپس rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
ساکن صحیح vfslogDelete(sqlite3_vfs *pVfs, ثابت حرف *z_راستہ, صحیح dirSync){
  صحیح rc;
  sqlite3_uint64 t;
  t = vfslog_وقت();
  rc = REALVFS(pVfs)->xDelete(REALVFS(pVfs), z_راستہ, dirSync);
  t = vfslog_وقت() - t;
  vfslog_call(pVfs, OS_DELETE, 0, t, rc, dirSync, 0);
  vfslog_string(pVfs, z_راستہ);
  واپس rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
ساکن صحیح vfslogAccess(
  sqlite3_vfs *pVfs, 
  ثابت حرف *z_راستہ, 
  صحیح flags, 
  صحیح *p_Res_برآمد
){
  صحیح rc;
  sqlite3_uint64 t;
  t = vfslog_وقت();
  rc = REALVFS(pVfs)->xAccess(REALVFS(pVfs), z_راستہ, flags, p_Res_برآمد);
  t = vfslog_وقت() - t;
  vfslog_call(pVfs, OS_ACCESS, 0, t, rc, flags, *p_Res_برآمد);
  vfslog_string(pVfs, z_راستہ);
  واپس rc;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
ساکن صحیح vfslog_بھرا_Pathname(
  sqlite3_vfs *pVfs, 
  ثابت حرف *z_راستہ, 
  صحیح n_برآمد, 
  حرف *z_برآمد
){
  واپس REALVFS(pVfs)->x_بھرا_Pathname(REALVFS(pVfs), z_راستہ, n_برآمد, z_برآمد);
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
ساکن خالی *vfslogDlOpen(sqlite3_vfs *pVfs, ثابت حرف *z_راستہ){
  واپس REALVFS(pVfs)->xDlOpen(REALVFS(pVfs), z_راستہ);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
ساکن خالی vfslogDlError(sqlite3_vfs *pVfs, صحیح nByte, حرف *z_Err_پیغام){
  REALVFS(pVfs)->xDlError(REALVFS(pVfs), nByte, z_Err_پیغام);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
ساکن خالی (*vfslogDlSym(sqlite3_vfs *pVfs, خالی *p, ثابت حرف *zSym))(خالی){
  واپس REALVFS(pVfs)->xDlSym(REALVFS(pVfs), p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
ساکن خالی vfslogDlClose(sqlite3_vfs *pVfs, خالی *pHandle){
  REALVFS(pVfs)->xDlClose(REALVFS(pVfs), pHandle);
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
ساکن صحیح vfslogRandomness(sqlite3_vfs *pVfs, صحیح nByte, حرف *z_Buf_برآمد){
  واپس REALVFS(pVfs)->xRandomness(REALVFS(pVfs), nByte, z_Buf_برآمد);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
ساکن صحیح vfslogSleep(sqlite3_vfs *pVfs, صحیح nMicro){
  واپس REALVFS(pVfs)->xSleep(REALVFS(pVfs), nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
ساکن صحیح vfslog_Current_وقت(sqlite3_vfs *pVfs, دوہرا *p_وقت_برآمد){
  واپس REALVFS(pVfs)->x_Current_وقت(REALVFS(pVfs), p_وقت_برآمد);
}

ساکن صحیح vfslog_Get_آخری_Error(sqlite3_vfs *pVfs, صحیح a, حرف *b){
  واپس REALVFS(pVfs)->x_Get_آخری_Error(REALVFS(pVfs), a, b);
}
ساکن صحیح vfslog_Current_وقت_Int64(sqlite3_vfs *pVfs, sqlite3_int64 *p){
  واپس REALVFS(pVfs)->x_Current_وقت_Int64(REALVFS(pVfs), p);
}

ساکن خالی vfslog_flush(VfslogVfs *p){
#اگر_تعریف_شدہ SQLITE_TEST
  خارجی صحیح sqlite3_io_error_pending;
  خارجی صحیح sqlite3_io_error_persist;
  خارجی صحیح sqlite3_diskfull_pending;

  صحیح pending = sqlite3_io_error_pending;
  صحیح persist = sqlite3_io_error_persist;
  صحیح diskfull = sqlite3_diskfull_pending;

  sqlite3_io_error_pending = 0;
  sqlite3_io_error_persist = 0;
  sqlite3_diskfull_pending = 0;
#اگر_ختم

  اگر( p->nBuf ){
    p->pLog->pMethods->xWrite(p->pLog, p->aBuf, p->nBuf, p->i_انحراف);
    p->i_انحراف += p->nBuf;
    p->nBuf = 0;
  }

#اگر_تعریف_شدہ SQLITE_TEST
  sqlite3_io_error_pending = pending;
  sqlite3_io_error_persist = persist;
  sqlite3_diskfull_pending = diskfull;
#اگر_ختم
}

ساکن خالی put32bits(غیرعلامتی حرف *p, غیرعلامتی صحیح v){
  p[0] = v>>24;
  p[1] = (غیرعلامتی حرف)(v>>16);
  p[2] = (غیرعلامتی حرف)(v>>8);
  p[3] = (غیرعلامتی حرف)v;
}

ساکن خالی vfslog_call(
  sqlite3_vfs *pVfs,
  صحیح eEvent,
  صحیح iFileid,
  sqlite3_int64 nClick,
  صحیح return_رمز,
  صحیح سائز,
  صحیح انحراف
){
  VfslogVfs *p = (VfslogVfs *)pVfs;
  غیرعلامتی حرف *zRec;
  اگر( (24+p->nBuf)>حجم(p->aBuf) ){
    vfslog_flush(p);
  }
  zRec = (غیرعلامتی حرف *)&p->aBuf[p->nBuf];
  put32bits(&zRec[0], eEvent);
  put32bits(&zRec[4], iFileid);
  put32bits(&zRec[8], (غیرعلامتی صحیح)(nClick&0xffff));
  put32bits(&zRec[12], return_رمز);
  put32bits(&zRec[16], سائز);
  put32bits(&zRec[20], انحراف);
  p->nBuf += 24;
}

ساکن خالی vfslog_string(sqlite3_vfs *pVfs, ثابت حرف *zStr){
  VfslogVfs *p = (VfslogVfs *)pVfs;
  غیرعلامتی حرف *zRec;
  صحیح nStr = zStr ? (صحیح)لڑی_لمبائی(zStr) : 0;
  اگر( (4+nStr+p->nBuf)>حجم(p->aBuf) ){
    vfslog_flush(p);
  }
  zRec = (غیرعلامتی حرف *)&p->aBuf[p->nBuf];
  put32bits(&zRec[0], nStr);
  اگر( zStr ){
    حافظہ_نقل(&zRec[4], zStr, nStr);
  }
  p->nBuf += (4 + nStr);
}

ساکن خالی vfslog_finalize(VfslogVfs *p){
  اگر( p->pLog->pMethods ){
    vfslog_flush(p);
    p->pLog->pMethods->xClose(p->pLog);
  }
  sqlite3_free(p);
}

صحیح sqlite3_vfslog_finalize(ثابت حرف *zVfs){
  sqlite3_vfs *pVfs;
  pVfs = sqlite3_vfs_ڈھونڈو(zVfs);
  اگر( !pVfs || pVfs->xOpen!=vfslogOpen ){
    واپس SQLITE_ERROR;
  } 
  sqlite3_vfs_unregister(pVfs);
  vfslog_finalize((VfslogVfs *)pVfs);
  واپس SQLITE_OK;
}

صحیح sqlite3_vfslog_new(
  ثابت حرف *zVfs,               /* New VFS name */
  ثابت حرف *z_والد_Vfs,         /* Parent VFS name (or NULL) */
  ثابت حرف *zLog                /* Log file name */
){
  VfslogVfs *p;
  sqlite3_vfs *p_والد;
  صحیح nByte;
  صحیح flags;
  صحیح rc;
  حرف *zFile;
  صحیح nVfs;

  p_والد = sqlite3_vfs_ڈھونڈو(z_والد_Vfs);
  اگر( !p_والد ){
    واپس SQLITE_ERROR;
  }

  nVfs = (صحیح)لڑی_لمبائی(zVfs);
  nByte = حجم(VfslogVfs) + p_والد->szOsFile + nVfs+1+p_والد->mxPathname+1;
  p = (VfslogVfs *)sqlite3_malloc(nByte);
  memset(p, 0, nByte);

  p->pVfs = p_والد;
  p->pLog = (sqlite3_file *)&p[1];
  حافظہ_نقل(&p->بنیاد, &vfslog_vfs, حجم(sqlite3_vfs));
  p->بنیاد.zName = &((حرف *)p->pLog)[p_والد->szOsFile];
  p->بنیاد.szOsFile += p_والد->szOsFile;
  حافظہ_نقل((حرف *)p->بنیاد.zName, zVfs, nVfs);

  zFile = (حرف *)&p->بنیاد.zName[nVfs+1];
  p_والد->x_بھرا_Pathname(p_والد, zLog, p_والد->mxPathname, zFile);

  flags = SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE|SQLITE_OPEN_SUPER_JOURNAL;
  p_والد->xDelete(p_والد, zFile, 0);
  rc = p_والد->xOpen(p_والد, zFile, p->pLog, flags, &flags);
  اگر( rc==SQLITE_OK ){
    حافظہ_نقل(p->aBuf, "sqlite_ostrace1.....", 20);
    p->i_انحراف = 0;
    p->nBuf = 20;
    rc = sqlite3_vfs_رجسٹر((sqlite3_vfs *)p, 1);
  }
  اگر( rc ){
    vfslog_finalize(p);
  }
  واپس rc;
}

صحیح sqlite3_vfslog_annotate(ثابت حرف *zVfs, ثابت حرف *z_پیغام){
  sqlite3_vfs *pVfs;
  pVfs = sqlite3_vfs_ڈھونڈو(zVfs);
  اگر( !pVfs || pVfs->xOpen!=vfslogOpen ){
    واپس SQLITE_ERROR;
  } 
  vfslog_call(pVfs, OS_ANNOTATE, 0, 0, 0, 0, 0);
  vfslog_string(pVfs, z_پیغام);
  واپس SQLITE_OK;
}

ساکن ثابت حرف *vfslog_eventname(صحیح eEvent){
  ثابت حرف *zEvent = 0;

  انتخاب( eEvent ){
    حالت OS_CLOSE:             zEvent = "xClose"; توڑو;
    حالت OS_READ:              zEvent = "xRead"; توڑو;
    حالت OS_WRITE:             zEvent = "xWrite"; توڑو;
    حالت OS_TRUNCATE:          zEvent = "xTruncate"; توڑو;
    حالت OS_SYNC:              zEvent = "xSync"; توڑو;
    حالت OS_FILESIZE:          zEvent = "xFilesize"; توڑو;
    حالت OS_تالا:              zEvent = "xLock"; توڑو;
    حالت OS_UNLOCK:            zEvent = "xUnlock"; توڑو;
    حالت OS_CHECKRESERVEDLOCK: zEvent = "xCheckResLock"; توڑو;
    حالت OS_FILECONTROL:       zEvent = "xFileControl"; توڑو;
    حالت OS_SECTORSIZE:        zEvent = "xSectorSize"; توڑو;
    حالت OS_DEVCHAR:           zEvent = "xDeviceChar"; توڑو;
    حالت OS_OPEN:              zEvent = "xOpen"; توڑو;
    حالت OS_DELETE:            zEvent = "xDelete"; توڑو;
    حالت OS_ACCESS:            zEvent = "xAccess"; توڑو;
    حالت OS_FULLPATHNAME:      zEvent = "xFullPathname"; توڑو;
    حالت OS_RANDOMNESS:        zEvent = "xRandomness"; توڑو;
    حالت OS_SLEEP:             zEvent = "xSleep"; توڑو;
    حالت OS_CURRENTTIME:       zEvent = "xCurrentTime"; توڑو;

    حالت OS_SHMUNMAP:          zEvent = "xShmUnmap"; توڑو;
    حالت OS_SHMLOCK:           zEvent = "xShmLock"; توڑو;
    حالت OS_SHMBARRIER:        zEvent = "xShmBarrier"; توڑو;
    حالت OS_SHMMAP:            zEvent = "xShmMap"; توڑو;

    حالت OS_ANNOTATE:          zEvent = "annotation"; توڑو;
  }

  واپس zEvent;
}

قسمتعریف ساخت VfslogVtab VfslogVtab;
قسمتعریف ساخت VfslogCsr VfslogCsr;

/*
** Virtual table type for the vfslog reader module.
*/
ساخت VfslogVtab {
  sqlite3_vtab بنیاد;              /* Base class */
  sqlite3_file *pFd;              /* File descriptor open on vfslog file */
  sqlite3_int64 nByte;            /* Size of file in bytes */
  حرف *zFile;                    /* File name for pFd */
};

/*
** Virtual table cursor type for the vfslog reader module.
*/
ساخت VfslogCsr {
  sqlite3_vtab_cursor بنیاد;       /* Base class */
  sqlite3_int64 iRowid;           /* Current rowid. */
  sqlite3_int64 i_انحراف;          /* Offset of next record in file */
  حرف *zTransient;               /* Transient 'file' string */
  صحیح nFile;                      /* Size of array azFile[] */
  حرف **azFile;                  /* File strings */
  غیرعلامتی حرف aBuf[1024];       /* Current vfs log entry (read from file) */
};

ساکن غیرعلامتی صحیح get32bits(غیرعلامتی حرف *p){
  واپس (p[0]<<24) + (p[1]<<16) + (p[2]<<8) + p[3];
}

/*
** The argument must point to a buffer containing a nul-terminated string.
** If the string begins with an SQL quote character it is overwritten by
** the dequoted version. Otherwise the buffer is left unmodified.
*/
ساکن خالی dequote(حرف *z){
  حرف quote;                     /* Quote character (if any ) */
  quote = z[0];
  اگر( quote=='[' || quote=='\'' || quote=='"' || quote=='`' ){
    صحیح i_اندر = 1;                  /* Index of next byte to read from input */
    صحیح i_برآمد = 0;                 /* Index of next byte to write to output */
    اگر( quote=='[' ) quote = ']';  
    جب( z[i_اندر] ){
      اگر( z[i_اندر]==quote ){
        اگر( z[i_اندر+1]!=quote ) توڑو;
        z[i_برآمد++] = quote;
        i_اندر += 2;
      }ورنہ{
        z[i_برآمد++] = z[i_اندر++];
      }
    }
    z[i_برآمد] = '\0';
  }
}

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_VIRTUALTABLE
/*
** Connect to or create a vfslog virtual table.
*/
ساکن صحیح vlogConnect(
  sqlite3 *db,
  خالی *pAux,
  صحیح argc, ثابت حرف *ثابت*argv,
  sqlite3_vtab **ppVtab,
  حرف **pzErr
){
  sqlite3_vfs *pVfs;              /* VFS used to read log file */
  صحیح flags;                      /* flags passed to pVfs->xOpen() */
  VfslogVtab *p;
  صحیح rc;
  صحیح nByte;
  حرف *zFile;

  *ppVtab = 0;
  pVfs = sqlite3_vfs_ڈھونڈو(0);
  nByte = حجم(VfslogVtab) + pVfs->szOsFile + pVfs->mxPathname;
  p = sqlite3_malloc(nByte);
  اگر( p==0 ) واپس SQLITE_NOMEM;
  memset(p, 0, nByte);

  p->pFd = (sqlite3_file *)&p[1];
  p->zFile = &((حرف *)p->pFd)[pVfs->szOsFile];

  zFile = sqlite3_mprintf("%s", argv[3]);
  اگر( !zFile ){
    sqlite3_free(p);
    واپس SQLITE_NOMEM;
  }
  dequote(zFile);
  pVfs->x_بھرا_Pathname(pVfs, zFile, pVfs->mxPathname, p->zFile);
  sqlite3_free(zFile);

  flags = SQLITE_OPEN_READWRITE|SQLITE_OPEN_SUPER_JOURNAL;
  rc = pVfs->xOpen(pVfs, p->zFile, p->pFd, flags, &flags);

  اگر( rc==SQLITE_OK ){
    p->pFd->pMethods->x_File_سائز(p->pFd, &p->nByte);
    sqlite3_declare_vtab(db, 
        "CREATE TABLE xxx(event, file, click, rc, size, offset)"
    );
    *ppVtab = &p->بنیاد;
  }ورنہ{
    sqlite3_free(p);
  }

  واپس rc;
}

/*
** There is no "best-index". This virtual table always does a linear
** scan of the binary VFS log file.
*/
ساکن صحیح vlog_بہترین_اشاریہ(sqlite3_vtab *tab, sqlite3_اشاریہ_info *pIdxInfo){
  pIdxInfo->estimated_لاگت = 10.0;
  واپس SQLITE_OK;
}

/*
** Disconnect from or destroy a vfslog virtual table.
*/
ساکن صحیح vlogDisconnect(sqlite3_vtab *pVtab){
  VfslogVtab *p = (VfslogVtab *)pVtab;
  اگر( p->pFd->pMethods ){
    p->pFd->pMethods->xClose(p->pFd);
    p->pFd->pMethods = 0;
  }
  sqlite3_free(p);
  واپس SQLITE_OK;
}

/*
** Open a new vfslog cursor.
*/
ساکن صحیح vlogOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  VfslogCsr *pCsr;                /* Newly allocated cursor object */

  pCsr = sqlite3_malloc(حجم(VfslogCsr));
  اگر( !pCsr ) واپس SQLITE_NOMEM;
  memset(pCsr, 0, حجم(VfslogCsr));
  *ppCursor = &pCsr->بنیاد;
  واپس SQLITE_OK;
}

/*
** Close a vfslog cursor.
*/
ساکن صحیح vlogClose(sqlite3_vtab_cursor *pCursor){
  VfslogCsr *p = (VfslogCsr *)pCursor;
  صحیح i;
  برائے(i=0; i<p->nFile; i++){
    sqlite3_free(p->azFile[i]);
  }
  sqlite3_free(p->azFile);
  sqlite3_free(p->zTransient);
  sqlite3_free(p);
  واپس SQLITE_OK;
}

/*
** Move a vfslog cursor to the next entry in the file.
*/
ساکن صحیح vlog_اگلا(sqlite3_vtab_cursor *pCursor){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  VfslogVtab *p = (VfslogVtab *)pCursor->pVtab;
  صحیح rc = SQLITE_OK;
  صحیح nRead;

  sqlite3_free(pCsr->zTransient);
  pCsr->zTransient = 0;

  nRead = 24;
  اگر( pCsr->i_انحراف+nRead<=p->nByte ){
    صحیح eEvent;
    rc = p->pFd->pMethods->xRead(p->pFd, pCsr->aBuf, nRead, pCsr->i_انحراف);

    eEvent = get32bits(pCsr->aBuf);
    اگر( (rc==SQLITE_OK)
     && (eEvent==OS_OPEN || eEvent==OS_DELETE || eEvent==OS_ACCESS) 
    ){
      حرف buf[4];
      rc = p->pFd->pMethods->xRead(p->pFd, buf, 4, pCsr->i_انحراف+nRead);
      nRead += 4;
      اگر( rc==SQLITE_OK ){
        صحیح nStr = get32bits((غیرعلامتی حرف *)buf);
        حرف *zStr = sqlite3_malloc(nStr+1);
        rc = p->pFd->pMethods->xRead(p->pFd, zStr, nStr, pCsr->i_انحراف+nRead);
        zStr[nStr] = '\0';
        nRead += nStr;

        اگر( eEvent==OS_OPEN ){
          صحیح iFileid = get32bits(&pCsr->aBuf[4]);
          اگر( iFileid>=pCsr->nFile ){
            صحیح nNew = حجم(pCsr->azFile[0])*(iFileid+1);
            pCsr->azFile = (حرف **)sqlite3_realloc(pCsr->azFile, nNew);
            nNew -= حجم(pCsr->azFile[0])*pCsr->nFile;
            memset(&pCsr->azFile[pCsr->nFile], 0, nNew);
            pCsr->nFile = iFileid+1;
          }
          sqlite3_free(pCsr->azFile[iFileid]);
          pCsr->azFile[iFileid] = zStr;
        }ورنہ{
          pCsr->zTransient = zStr;
        }
      }
    }
  }

  pCsr->iRowid += 1;
  pCsr->i_انحراف += nRead;
  واپس rc;
}

ساکن صحیح vlogEof(sqlite3_vtab_cursor *pCursor){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  VfslogVtab *p = (VfslogVtab *)pCursor->pVtab;
  واپس (pCsr->i_انحراف>=p->nByte);
}

ساکن صحیح vlogFilter(
  sqlite3_vtab_cursor *pCursor, 
  صحیح idx_عدد, ثابت حرف *idxStr,
  صحیح argc, sqlite3_قدر **argv
){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  pCsr->iRowid = 0;
  pCsr->i_انحراف = 20;
  واپس vlog_اگلا(pCursor);
}

ساکن صحیح vlog_عمود(
  sqlite3_vtab_cursor *pCursor, 
  sqlite3_context *ctx, 
  صحیح i
){
  غیرعلامتی صحیح val;
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;

  أكد( i<7 );
  val = get32bits(&pCsr->aBuf[4*i]);

  انتخاب( i ){
    حالت 0: {
      sqlite3_نتیجہ_متن(ctx, vfslog_eventname(val), -1, SQLITE_STATIC);
      توڑو;
    }
    حالت 1: {
      حرف *zStr = pCsr->zTransient;
      اگر( val!=0 && val<(غیرعلامتی)pCsr->nFile ){
        zStr = pCsr->azFile[val];
      }
      sqlite3_نتیجہ_متن(ctx, zStr, -1, SQLITE_TRANSIENT);
      توڑو;
    }
    طےشدہ:
      sqlite3_نتیجہ_int(ctx, val);
      توڑو;
  }

  واپس SQLITE_OK;
}

ساکن صحیح vlogRowid(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  *pRowid = pCsr->iRowid;
  واپس SQLITE_OK;
}

صحیح sqlite3_vfslog_رجسٹر(sqlite3 *db){
  ساکن sqlite3_module vfslog_module = {
    0,                            /* iVersion */
    vlogConnect,                /* xCreate */
    vlogConnect,                /* xConnect */
    vlog_بہترین_اشاریہ,              /* xBestIndex */
    vlogDisconnect,             /* xDisconnect */
    vlogDisconnect,             /* xDestroy */
    vlogOpen,                   /* xOpen - open a cursor */
    vlogClose,                  /* xClose - close a cursor */
    vlogFilter,                 /* xFilter - configure scan constraints */
    vlog_اگلا,                   /* xNext - advance a cursor */
    vlogEof,                    /* xEof - check for end of scan */
    vlog_عمود,                 /* xColumn - read data */
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
  واپس SQLITE_OK;
}
#اگر_ختم /* SQLITE_OMIT_VIRTUALTABLE */

/**************************************************************************
***************************************************************************
** Tcl interface starts here.
*/

#اگر defined(SQLITE_TEST) || defined(TCLSH)

#شامل "tclsqlite.h"

ساکن صحیح SQLITE_TCLAPI test_vfslog(
  خالی *client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  ساخت SqliteDb { sqlite3 *db; };
  sqlite3 *db;
  Tcl_CmdInfo cmdInfo;
  صحیح rc = SQLITE_ERROR;

  ساکن ثابت حرف *strs[] = { "annotate", "finalize", "new",  "register", 0 };
  شمار VL_enum { VL_ANNOTATE, VL_FINALIZE, VL_NEW, VL_رجسٹر };
  صحیح iSub;

  اگر( objc<2 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "SUB-COMMAND ...");
    واپس TCL_ERROR;
  }
  اگر( Tcl_Get_اشاریہ_From_Obj(interp, objv[1], strs, "sub-command", 0, &iSub) ){
    واپس TCL_ERROR;
  }

  انتخاب( (شمار VL_enum)iSub ){
    حالت VL_ANNOTATE: {
      حرف *zVfs;
      حرف *z_پیغام;
      اگر( objc!=4 ){
        Tcl_Wrong_عدد_دلائل(interp, 3, objv, "VFS");
        واپس TCL_ERROR;
      }
      zVfs = Tcl_GetString(objv[2]);
      z_پیغام = Tcl_GetString(objv[3]);
      rc = sqlite3_vfslog_annotate(zVfs, z_پیغام);
      اگر( rc!=SQLITE_OK ){
        Tcl_Append_نتیجہ(interp, "failed", (حرف*)0);
        واپس TCL_ERROR;
      }
      توڑو;
    }
    حالت VL_FINALIZE: {
      حرف *zVfs;
      اگر( objc!=3 ){
        Tcl_Wrong_عدد_دلائل(interp, 2, objv, "VFS");
        واپس TCL_ERROR;
      }
      zVfs = Tcl_GetString(objv[2]);
      rc = sqlite3_vfslog_finalize(zVfs);
      اگر( rc!=SQLITE_OK ){
        Tcl_Append_نتیجہ(interp, "failed", (حرف*)0);
        واپس TCL_ERROR;
      }
      توڑو;
    };

    حالت VL_NEW: {
      حرف *zVfs;
      حرف *z_والد;
      حرف *zLog;
      اگر( objc!=5 ){
        Tcl_Wrong_عدد_دلائل(interp, 2, objv, "VFS PARENT LOGFILE");
        واپس TCL_ERROR;
      }
      zVfs = Tcl_GetString(objv[2]);
      z_والد = Tcl_GetString(objv[3]);
      zLog = Tcl_GetString(objv[4]);
      اگر( *z_والد=='\0' ) z_والد = 0;
      rc = sqlite3_vfslog_new(zVfs, z_والد, zLog);
      اگر( rc!=SQLITE_OK ){
        Tcl_Append_نتیجہ(interp, "failed", (حرف*)0);
        واپس TCL_ERROR;
      }
      توڑو;
    };

    حالت VL_رجسٹر: {
      حرف *zDb;
      اگر( objc!=3 ){
        Tcl_Wrong_عدد_دلائل(interp, 2, objv, "DB");
        واپس TCL_ERROR;
      }
#اگر_تعریف_شدہ SQLITE_OMIT_VIRTUALTABLE
      Tcl_Append_نتیجہ(interp, "vfslog not available because of "
                               "SQLITE_OMIT_VIRTUALTABLE", (خالی*)0);
      واپس TCL_ERROR;
#ورنہ
      zDb = Tcl_GetString(objv[2]);
      اگر( Tcl_GetCommandInfo(interp, zDb, &cmdInfo) ){
        db = ((ساخت SqliteDb*)cmdInfo.obj_Client_ڈیٹا)->db;
        rc = sqlite3_vfslog_رجسٹر(db);
      }
      اگر( rc!=SQLITE_OK ){
        Tcl_Append_نتیجہ(interp, "bad sqlite3 handle: ", zDb, (خالی*)0);
        واپس TCL_ERROR;
      }
      توڑو;
#اگر_ختم
    }
  }

  واپس TCL_OK;
}

صحیح SqlitetestOsinst_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "vfslog", test_vfslog, 0, 0);
  واپس TCL_OK;
}

#اگر_ختم /* SQLITE_TEST */
