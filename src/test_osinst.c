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

#ಅನ್ತರ್ಭಾವ "sqlite3.h"

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
#ಅನ್ತರ್ಭಾವ <windows.h>
#ಆದರೆ_ಅಂತ್ಯ

#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <assert.h>


/*
** Maximum pathname length supported by the vfslog backend.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು INST_MAX_PATHNAME 512

#ವ್ಯಾಖ್ಯಾನಿಸು OS_ACCESS            1
#ವ್ಯಾಖ್ಯಾನಿಸು OS_CHECKRESERVEDLOCK 2
#ವ್ಯಾಖ್ಯಾನಿಸು OS_CLOSE             3
#ವ್ಯಾಖ್ಯಾನಿಸು OS_CURRENTTIME       4
#ವ್ಯಾಖ್ಯಾನಿಸು OS_DELETE            5
#ವ್ಯಾಖ್ಯಾನಿಸು OS_DEVCHAR           6
#ವ್ಯಾಖ್ಯಾನಿಸು OS_FILECONTROL       7
#ವ್ಯಾಖ್ಯಾನಿಸು OS_FILESIZE          8
#ವ್ಯಾಖ್ಯಾನಿಸು OS_FULLPATHNAME      9
#ವ್ಯಾಖ್ಯಾನಿಸು OS_ಬೀಗ              11
#ವ್ಯಾಖ್ಯಾನಿಸು OS_OPEN              12
#ವ್ಯಾಖ್ಯಾನಿಸು OS_RANDOMNESS        13
#ವ್ಯಾಖ್ಯಾನಿಸು OS_READ              14 
#ವ್ಯಾಖ್ಯಾನಿಸು OS_SECTORSIZE        15
#ವ್ಯಾಖ್ಯಾನಿಸು OS_SLEEP             16
#ವ್ಯಾಖ್ಯಾನಿಸು OS_SYNC              17
#ವ್ಯಾಖ್ಯಾನಿಸು OS_TRUNCATE          18
#ವ್ಯಾಖ್ಯಾನಿಸು OS_UNLOCK            19
#ವ್ಯಾಖ್ಯಾನಿಸು OS_WRITE             20
#ವ್ಯಾಖ್ಯಾನಿಸು OS_SHMUNMAP          22
#ವ್ಯಾಖ್ಯಾನಿಸು OS_SHMMAP            23
#ವ್ಯಾಖ್ಯಾನಿಸು OS_SHMLOCK           25
#ವ್ಯಾಖ್ಯಾನಿಸು OS_SHMBARRIER        26
#ವ್ಯಾಖ್ಯಾನಿಸು OS_ANNOTATE          28

#ವ್ಯಾಖ್ಯಾನಿಸು OS_NUMEVENTS         29

#ವ್ಯಾಖ್ಯಾನಿಸು VFSLOG_BUFFERSIZE 8192

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ VfslogVfs VfslogVfs;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ VfslogFile VfslogFile;

ರಚನೆ VfslogVfs {
  sqlite3_vfs ತಳ;               /* VFS methods */
  sqlite3_vfs *pVfs;              /* Parent VFS */
  ಪೂರ್ಣಾಂಕ i_ಮುಂದಿನ_File_Id;                /* Next file id */
  sqlite3_file *pLog;             /* Log file handle */
  sqlite3_int64 i_ಸ್ಥಾನಾಂತರ;          /* Log file offset of start of write buffer */
  ಪೂರ್ಣಾಂಕ nBuf;                       /* Number of valid bytes in aBuf[] */
  ಅಕ್ಷರ aBuf[VFSLOG_BUFFERSIZE];   /* Write buffer */
};

ರಚನೆ VfslogFile {
  sqlite3_file ತಳ;              /* IO methods */
  sqlite3_file *pReal;            /* Underlying file handle */
  sqlite3_vfs *pVfslog;           /* Associated VsflogVfs object */
  ಪೂರ್ಣಾಂಕ iFileId;                    /* File id number */
};

#ವ್ಯಾಖ್ಯಾನಿಸು REALVFS(p) (((VfslogVfs *)(p))->pVfs)



/*
** Method declarations for vfslog_file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vfslogClose(sqlite3_file*);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vfslogRead(sqlite3_file*, ಶೂನ್ಯ*, ಪೂರ್ಣಾಂಕ iAmt, sqlite3_int64 iOfst);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vfslogWrite(sqlite3_file*,ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*,ಪೂರ್ಣಾಂಕ iAmt, sqlite3_int64 iOfst);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vfslogTruncate(sqlite3_file*, sqlite3_int64 ಗಾತ್ರ_2);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vfslogSync(sqlite3_file*, ಪೂರ್ಣಾಂಕ flags);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vfslog_File_ಗಾತ್ರ(sqlite3_file*, sqlite3_int64 *p_ಗಾತ್ರ);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vfslog_ಬೀಗ(sqlite3_file*, ಪೂರ್ಣಾಂಕ);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vfslogUnlock(sqlite3_file*, ಪೂರ್ಣಾಂಕ);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vfslog_Check_Reserved_ಬೀಗ(sqlite3_file*, ಪೂರ್ಣಾಂಕ *p_Res_ಹೊರಹೋಗು);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vfslogFileControl(sqlite3_file*, ಪೂರ್ಣಾಂಕ op, ಶೂನ್ಯ *pArg);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vfslog_Sector_ಗಾತ್ರ(sqlite3_file*);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vfslogDeviceCharacteristics(sqlite3_file*);

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vfslog_Shm_ಬೀಗ(sqlite3_file *pFile, ಪೂರ್ಣಾಂಕ ofst, ಪೂರ್ಣಾಂಕ n, ಪೂರ್ಣಾಂಕ flags);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vfslogShmMap(sqlite3_file *pFile,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ,ಅಸ್ಥಿರ ಶೂನ್ಯ **);
ಸ್ಥಿರ ಶೂನ್ಯ vfslogShmBarrier(sqlite3_file*);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vfslogShmUnmap(sqlite3_file *pFile, ಪೂರ್ಣಾಂಕ deleteFlag);

/*
** Method declarations for vfslog_vfs.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vfslogOpen(sqlite3_vfs*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *, sqlite3_file*, ಪೂರ್ಣಾಂಕ , ಪೂರ್ಣಾಂಕ *);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vfslogDelete(sqlite3_vfs*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName, ಪೂರ್ಣಾಂಕ syncDir);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vfslogAccess(sqlite3_vfs*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName, ಪೂರ್ಣಾಂಕ flags, ಪೂರ್ಣಾಂಕ *);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vfslog_ತುಂಬಿದ_Pathname(sqlite3_vfs*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName, ಪೂರ್ಣಾಂಕ, ಅಕ್ಷರ *z_ಹೊರಹೋಗು);
ಸ್ಥಿರ ಶೂನ್ಯ *vfslogDlOpen(sqlite3_vfs*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFilename);
ಸ್ಥಿರ ಶೂನ್ಯ vfslogDlError(sqlite3_vfs*, ಪೂರ್ಣಾಂಕ nByte, ಅಕ್ಷರ *z_Err_ಸಂದೇಶ);
ಸ್ಥಿರ ಶೂನ್ಯ (*vfslogDlSym(sqlite3_vfs *pVfs, ಶೂನ್ಯ *p, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*zSym))(ಶೂನ್ಯ);
ಸ್ಥಿರ ಶೂನ್ಯ vfslogDlClose(sqlite3_vfs*, ಶೂನ್ಯ*);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vfslogRandomness(sqlite3_vfs*, ಪೂರ್ಣಾಂಕ nByte, ಅಕ್ಷರ *z_ಹೊರಹೋಗು);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vfslogSleep(sqlite3_vfs*, ಪೂರ್ಣಾಂಕ microseconds);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vfslog_Current_ಸಮಯ(sqlite3_vfs*, ದ್ವಿಗುಣ*);

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vfslog_Get_ಕೊನೆಯ_Error(sqlite3_vfs*, ಪೂರ್ಣಾಂಕ, ಅಕ್ಷರ *);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vfslog_Current_ಸಮಯ_Int64(sqlite3_vfs*, sqlite3_int64*);

ಸ್ಥಿರ sqlite3_vfs vfslog_vfs = {
  1,                              /* iVersion */
  ಗಾತ್ರ(VfslogFile),             /* szOsFile */
  INST_MAX_PATHNAME,              /* mxPathname */
  0,                              /* pNext */
  0,                              /* zName */
  0,                              /* pAppData */
  vfslogOpen,                     /* xOpen */
  vfslogDelete,                   /* xDelete */
  vfslogAccess,                   /* xAccess */
  vfslog_ತುಂಬಿದ_Pathname,             /* xFullPathname */
  vfslogDlOpen,                   /* xDlOpen */
  vfslogDlError,                  /* xDlError */
  vfslogDlSym,                    /* xDlSym */
  vfslogDlClose,                  /* xDlClose */
  vfslogRandomness,               /* xRandomness */
  vfslogSleep,                    /* xSleep */
  vfslog_Current_ಸಮಯ,              /* xCurrentTime */
  vfslog_Get_ಕೊನೆಯ_Error,             /* xGetLastError */
  vfslog_Current_ಸಮಯ_Int64          /* xCurrentTime */
};

ಸ್ಥಿರ sqlite3_io_methods vfslog_io_methods = {
  2,                              /* iVersion */
  vfslogClose,                    /* xClose */
  vfslogRead,                     /* xRead */
  vfslogWrite,                    /* xWrite */
  vfslogTruncate,                 /* xTruncate */
  vfslogSync,                     /* xSync */
  vfslog_File_ಗಾತ್ರ,                 /* xFileSize */
  vfslog_ಬೀಗ,                     /* xLock */
  vfslogUnlock,                   /* xUnlock */
  vfslog_Check_Reserved_ಬೀಗ,        /* xCheckReservedLock */
  vfslogFileControl,              /* xFileControl */
  vfslog_Sector_ಗಾತ್ರ,               /* xSectorSize */
  vfslogDeviceCharacteristics,    /* xDeviceCharacteristics */
  vfslogShmMap,                   /* xShmMap */
  vfslog_Shm_ಬೀಗ,                  /* xShmLock */
  vfslogShmBarrier,               /* xShmBarrier */
  vfslogShmUnmap                  /* xShmUnmap */
};

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
#ಅನ್ತರ್ಭಾವ <time.h>
ಸ್ಥಿರ sqlite3_uint64 vfslog_ಸಮಯ(){
  FILETIME ft;
  sqlite3_uint64 u64time = 0;
 
  Get_System_ಸಮಯ_As_File_ಸಮಯ(&ft);

  u64time |= ft.dw_ಮೇಲಿನ_Date_ಸಮಯ;
  u64time <<= 32;
  u64time |= ft.dw_ಕೆಳಗಿನ_Date_ಸಮಯ;

  /* ft is 100-nanosecond intervals, we want microseconds */
  ಹಿಂತಿರುಗಿ u64time /(sqlite3_uint64)10;
}
#ಅಥವಾಆದರೆ !defined(NO_GETTOD)
#ಅನ್ತರ್ಭಾವ <sys/time.h>
ಸ್ಥಿರ sqlite3_uint64 vfslog_ಸಮಯ(){
  ರಚನೆ timeval s_ಸಮಯ;
  gettimeofday(&s_ಸಮಯ, 0);
  ಹಿಂತಿರುಗಿ s_ಸಮಯ.tv_usec + (sqlite3_uint64)s_ಸಮಯ.tv_sec * 1000000;
}
#ಇಲ್ಲದಿದ್ದರೆ
ಸ್ಥಿರ sqlite3_uint64 vfslog_ಸಮಯ(){
  ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಶೂನ್ಯ vfslog_call(sqlite3_vfs *, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ, sqlite3_int64, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ);
ಸ್ಥಿರ ಶೂನ್ಯ vfslog_string(sqlite3_vfs *, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *);

/*
** Close an vfslog-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vfslogClose(sqlite3_file *pFile){
  sqlite3_uint64 t;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  VfslogFile *p = (VfslogFile *)pFile;

  t = vfslog_ಸಮಯ();
  ಆದರೆ( p->pReal->pMethods ){
    rc = p->pReal->pMethods->xClose(p->pReal);
  }
  t = vfslog_ಸಮಯ() - t;
  vfslog_call(p->pVfslog, OS_CLOSE, p->iFileId, t, rc, 0, 0);
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Read data from an vfslog-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vfslogRead(
  sqlite3_file *pFile, 
  ಶೂನ್ಯ *zBuf, 
  ಪೂರ್ಣಾಂಕ iAmt, 
  sqlite_int64 iOfst
){
  ಪೂರ್ಣಾಂಕ rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_ಸಮಯ();
  rc = p->pReal->pMethods->xRead(p->pReal, zBuf, iAmt, iOfst);
  t = vfslog_ಸಮಯ() - t;
  vfslog_call(p->pVfslog, OS_READ, p->iFileId, t, rc, iAmt, (ಪೂರ್ಣಾಂಕ)iOfst);
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Write data to an vfslog-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vfslogWrite(
  sqlite3_file *pFile,
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *z,
  ಪೂರ್ಣಾಂಕ iAmt,
  sqlite_int64 iOfst
){
  ಪೂರ್ಣಾಂಕ rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_ಸಮಯ();
  rc = p->pReal->pMethods->xWrite(p->pReal, z, iAmt, iOfst);
  t = vfslog_ಸಮಯ() - t;
  vfslog_call(p->pVfslog, OS_WRITE, p->iFileId, t, rc, iAmt, (ಪೂರ್ಣಾಂಕ)iOfst);
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Truncate an vfslog-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vfslogTruncate(sqlite3_file *pFile, sqlite_int64 ಗಾತ್ರ_2){
  ಪೂರ್ಣಾಂಕ rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_ಸಮಯ();
  rc = p->pReal->pMethods->xTruncate(p->pReal, ಗಾತ್ರ_2);
  t = vfslog_ಸಮಯ() - t;
  vfslog_call(p->pVfslog, OS_TRUNCATE, p->iFileId, t, rc, 0, (ಪೂರ್ಣಾಂಕ)ಗಾತ್ರ_2);
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Sync an vfslog-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vfslogSync(sqlite3_file *pFile, ಪೂರ್ಣಾಂಕ flags){
  ಪೂರ್ಣಾಂಕ rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_ಸಮಯ();
  rc = p->pReal->pMethods->xSync(p->pReal, flags);
  t = vfslog_ಸಮಯ() - t;
  vfslog_call(p->pVfslog, OS_SYNC, p->iFileId, t, rc, flags, 0);
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Return the current file-size of an vfslog-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vfslog_File_ಗಾತ್ರ(sqlite3_file *pFile, sqlite_int64 *p_ಗಾತ್ರ){
  ಪೂರ್ಣಾಂಕ rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_ಸಮಯ();
  rc = p->pReal->pMethods->x_File_ಗಾತ್ರ(p->pReal, p_ಗಾತ್ರ);
  t = vfslog_ಸಮಯ() - t;
  vfslog_call(p->pVfslog, OS_FILESIZE, p->iFileId, t, rc, 0, (ಪೂರ್ಣಾಂಕ)*p_ಗಾತ್ರ);
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Lock an vfslog-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vfslog_ಬೀಗ(sqlite3_file *pFile, ಪೂರ್ಣಾಂಕ e_ಬೀಗ){
  ಪೂರ್ಣಾಂಕ rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_ಸಮಯ();
  rc = p->pReal->pMethods->x_ಬೀಗ(p->pReal, e_ಬೀಗ);
  t = vfslog_ಸಮಯ() - t;
  vfslog_call(p->pVfslog, OS_ಬೀಗ, p->iFileId, t, rc, e_ಬೀಗ, 0);
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Unlock an vfslog-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vfslogUnlock(sqlite3_file *pFile, ಪೂರ್ಣಾಂಕ e_ಬೀಗ){
  ಪೂರ್ಣಾಂಕ rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_ಸಮಯ();
  rc = p->pReal->pMethods->xUnlock(p->pReal, e_ಬೀಗ);
  t = vfslog_ಸಮಯ() - t;
  vfslog_call(p->pVfslog, OS_UNLOCK, p->iFileId, t, rc, e_ಬೀಗ, 0);
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Check if another file-handle holds a RESERVED lock on an vfslog-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vfslog_Check_Reserved_ಬೀಗ(sqlite3_file *pFile, ಪೂರ್ಣಾಂಕ *p_Res_ಹೊರಹೋಗು){
  ಪೂರ್ಣಾಂಕ rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_ಸಮಯ();
  rc = p->pReal->pMethods->x_Check_Reserved_ಬೀಗ(p->pReal, p_Res_ಹೊರಹೋಗು);
  t = vfslog_ಸಮಯ() - t;
  vfslog_call(p->pVfslog, OS_CHECKRESERVEDLOCK, p->iFileId, t, rc, *p_Res_ಹೊರಹೋಗು, 0);
  ಹಿಂತಿರುಗಿ rc;
}

/*
** File control method. For custom operations on an vfslog-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vfslogFileControl(sqlite3_file *pFile, ಪೂರ್ಣಾಂಕ op, ಶೂನ್ಯ *pArg){
  VfslogFile *p = (VfslogFile *)pFile;
  ಪೂರ್ಣಾಂಕ rc = p->pReal->pMethods->xFileControl(p->pReal, op, pArg);
  ಆದರೆ( op==SQLITE_FCNTL_VFSNAME && rc==SQLITE_OK ){
    *(ಅಕ್ಷರ**)pArg = sqlite3_mprintf("vfslog/%z", *(ಅಕ್ಷರ**)pArg);
  }
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Return the sector-size in bytes for an vfslog-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vfslog_Sector_ಗಾತ್ರ(sqlite3_file *pFile){
  ಪೂರ್ಣಾಂಕ rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_ಸಮಯ();
  rc = p->pReal->pMethods->x_Sector_ಗಾತ್ರ(p->pReal);
  t = vfslog_ಸಮಯ() - t;
  vfslog_call(p->pVfslog, OS_SECTORSIZE, p->iFileId, t, rc, 0, 0);
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Return the device characteristic flags supported by an vfslog-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vfslogDeviceCharacteristics(sqlite3_file *pFile){
  ಪೂರ್ಣಾಂಕ rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_ಸಮಯ();
  rc = p->pReal->pMethods->xDeviceCharacteristics(p->pReal);
  t = vfslog_ಸಮಯ() - t;
  vfslog_call(p->pVfslog, OS_DEVCHAR, p->iFileId, t, rc, 0, 0);
  ಹಿಂತಿರುಗಿ rc;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vfslog_Shm_ಬೀಗ(sqlite3_file *pFile, ಪೂರ್ಣಾಂಕ ofst, ಪೂರ್ಣಾಂಕ n, ಪೂರ್ಣಾಂಕ flags){
  ಪೂರ್ಣಾಂಕ rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_ಸಮಯ();
  rc = p->pReal->pMethods->x_Shm_ಬೀಗ(p->pReal, ofst, n, flags);
  t = vfslog_ಸಮಯ() - t;
  vfslog_call(p->pVfslog, OS_SHMLOCK, p->iFileId, t, rc, 0, 0);
  ಹಿಂತಿರುಗಿ rc;
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vfslogShmMap(
  sqlite3_file *pFile, 
  ಪೂರ್ಣಾಂಕ iRegion, 
  ಪೂರ್ಣಾಂಕ szRegion, 
  ಪೂರ್ಣಾಂಕ isWrite, 
  ಅಸ್ಥಿರ ಶೂನ್ಯ **pp
){
  ಪೂರ್ಣಾಂಕ rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_ಸಮಯ();
  rc = p->pReal->pMethods->xShmMap(p->pReal, iRegion, szRegion, isWrite, pp);
  t = vfslog_ಸಮಯ() - t;
  vfslog_call(p->pVfslog, OS_SHMMAP, p->iFileId, t, rc, 0, 0);
  ಹಿಂತಿರುಗಿ rc;
}
ಸ್ಥಿರ ಶೂನ್ಯ vfslogShmBarrier(sqlite3_file *pFile){
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_ಸಮಯ();
  p->pReal->pMethods->xShmBarrier(p->pReal);
  t = vfslog_ಸಮಯ() - t;
  vfslog_call(p->pVfslog, OS_SHMBARRIER, p->iFileId, t, SQLITE_OK, 0, 0);
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vfslogShmUnmap(sqlite3_file *pFile, ಪೂರ್ಣಾಂಕ deleteFlag){
  ಪೂರ್ಣಾಂಕ rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  t = vfslog_ಸಮಯ();
  rc = p->pReal->pMethods->xShmUnmap(p->pReal, deleteFlag);
  t = vfslog_ಸಮಯ() - t;
  vfslog_call(p->pVfslog, OS_SHMUNMAP, p->iFileId, t, rc, 0, 0);
  ಹಿಂತಿರುಗಿ rc;
}


/*
** Open an vfslog file handle.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vfslogOpen(
  sqlite3_vfs *pVfs,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName,
  sqlite3_file *pFile,
  ಪೂರ್ಣಾಂಕ flags,
  ಪೂರ್ಣಾಂಕ *p_ಹೊರಹೋಗು_Flags
){
  ಪೂರ್ಣಾಂಕ rc;
  sqlite3_uint64 t;
  VfslogFile *p = (VfslogFile *)pFile;
  VfslogVfs *pLog = (VfslogVfs *)pVfs;

  pFile->pMethods = &vfslog_io_methods;
  p->pReal = (sqlite3_file *)&p[1];
  p->pVfslog = pVfs;
  p->iFileId = ++pLog->i_ಮುಂದಿನ_File_Id;

  t = vfslog_ಸಮಯ();
  rc = REALVFS(pVfs)->xOpen(REALVFS(pVfs), zName, p->pReal, flags, p_ಹೊರಹೋಗು_Flags);
  t = vfslog_ಸಮಯ() - t;

  vfslog_call(pVfs, OS_OPEN, p->iFileId, t, rc, 0, 0);
  vfslog_string(pVfs, zName);
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vfslogDelete(sqlite3_vfs *pVfs, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮಾರ್ಗ, ಪೂರ್ಣಾಂಕ dirSync){
  ಪೂರ್ಣಾಂಕ rc;
  sqlite3_uint64 t;
  t = vfslog_ಸಮಯ();
  rc = REALVFS(pVfs)->xDelete(REALVFS(pVfs), z_ಮಾರ್ಗ, dirSync);
  t = vfslog_ಸಮಯ() - t;
  vfslog_call(pVfs, OS_DELETE, 0, t, rc, dirSync, 0);
  vfslog_string(pVfs, z_ಮಾರ್ಗ);
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vfslogAccess(
  sqlite3_vfs *pVfs, 
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮಾರ್ಗ, 
  ಪೂರ್ಣಾಂಕ flags, 
  ಪೂರ್ಣಾಂಕ *p_Res_ಹೊರಹೋಗು
){
  ಪೂರ್ಣಾಂಕ rc;
  sqlite3_uint64 t;
  t = vfslog_ಸಮಯ();
  rc = REALVFS(pVfs)->xAccess(REALVFS(pVfs), z_ಮಾರ್ಗ, flags, p_Res_ಹೊರಹೋಗು);
  t = vfslog_ಸಮಯ() - t;
  vfslog_call(pVfs, OS_ACCESS, 0, t, rc, flags, *p_Res_ಹೊರಹೋಗು);
  vfslog_string(pVfs, z_ಮಾರ್ಗ);
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vfslog_ತುಂಬಿದ_Pathname(
  sqlite3_vfs *pVfs, 
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮಾರ್ಗ, 
  ಪೂರ್ಣಾಂಕ n_ಹೊರಹೋಗು, 
  ಅಕ್ಷರ *z_ಹೊರಹೋಗು
){
  ಹಿಂತಿರುಗಿ REALVFS(pVfs)->x_ತುಂಬಿದ_Pathname(REALVFS(pVfs), z_ಮಾರ್ಗ, n_ಹೊರಹೋಗು, z_ಹೊರಹೋಗು);
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
ಸ್ಥಿರ ಶೂನ್ಯ *vfslogDlOpen(sqlite3_vfs *pVfs, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮಾರ್ಗ){
  ಹಿಂತಿರುಗಿ REALVFS(pVfs)->xDlOpen(REALVFS(pVfs), z_ಮಾರ್ಗ);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
ಸ್ಥಿರ ಶೂನ್ಯ vfslogDlError(sqlite3_vfs *pVfs, ಪೂರ್ಣಾಂಕ nByte, ಅಕ್ಷರ *z_Err_ಸಂದೇಶ){
  REALVFS(pVfs)->xDlError(REALVFS(pVfs), nByte, z_Err_ಸಂದೇಶ);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
ಸ್ಥಿರ ಶೂನ್ಯ (*vfslogDlSym(sqlite3_vfs *pVfs, ಶೂನ್ಯ *p, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSym))(ಶೂನ್ಯ){
  ಹಿಂತಿರುಗಿ REALVFS(pVfs)->xDlSym(REALVFS(pVfs), p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
ಸ್ಥಿರ ಶೂನ್ಯ vfslogDlClose(sqlite3_vfs *pVfs, ಶೂನ್ಯ *pHandle){
  REALVFS(pVfs)->xDlClose(REALVFS(pVfs), pHandle);
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vfslogRandomness(sqlite3_vfs *pVfs, ಪೂರ್ಣಾಂಕ nByte, ಅಕ್ಷರ *z_Buf_ಹೊರಹೋಗು){
  ಹಿಂತಿರುಗಿ REALVFS(pVfs)->xRandomness(REALVFS(pVfs), nByte, z_Buf_ಹೊರಹೋಗು);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vfslogSleep(sqlite3_vfs *pVfs, ಪೂರ್ಣಾಂಕ nMicro){
  ಹಿಂತಿರುಗಿ REALVFS(pVfs)->xSleep(REALVFS(pVfs), nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vfslog_Current_ಸಮಯ(sqlite3_vfs *pVfs, ದ್ವಿಗುಣ *p_ಸಮಯ_ಹೊರಹೋಗು){
  ಹಿಂತಿರುಗಿ REALVFS(pVfs)->x_Current_ಸಮಯ(REALVFS(pVfs), p_ಸಮಯ_ಹೊರಹೋಗು);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vfslog_Get_ಕೊನೆಯ_Error(sqlite3_vfs *pVfs, ಪೂರ್ಣಾಂಕ a, ಅಕ್ಷರ *b){
  ಹಿಂತಿರುಗಿ REALVFS(pVfs)->x_Get_ಕೊನೆಯ_Error(REALVFS(pVfs), a, b);
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vfslog_Current_ಸಮಯ_Int64(sqlite3_vfs *pVfs, sqlite3_int64 *p){
  ಹಿಂತಿರುಗಿ REALVFS(pVfs)->x_Current_ಸಮಯ_Int64(REALVFS(pVfs), p);
}

ಸ್ಥಿರ ಶೂನ್ಯ vfslog_flush(VfslogVfs *p){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_TEST
  ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ sqlite3_io_error_pending;
  ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ sqlite3_io_error_persist;
  ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ sqlite3_diskfull_pending;

  ಪೂರ್ಣಾಂಕ pending = sqlite3_io_error_pending;
  ಪೂರ್ಣಾಂಕ persist = sqlite3_io_error_persist;
  ಪೂರ್ಣಾಂಕ diskfull = sqlite3_diskfull_pending;

  sqlite3_io_error_pending = 0;
  sqlite3_io_error_persist = 0;
  sqlite3_diskfull_pending = 0;
#ಆದರೆ_ಅಂತ್ಯ

  ಆದರೆ( p->nBuf ){
    p->pLog->pMethods->xWrite(p->pLog, p->aBuf, p->nBuf, p->i_ಸ್ಥಾನಾಂತರ);
    p->i_ಸ್ಥಾನಾಂತರ += p->nBuf;
    p->nBuf = 0;
  }

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_TEST
  sqlite3_io_error_pending = pending;
  sqlite3_io_error_persist = persist;
  sqlite3_diskfull_pending = diskfull;
#ಆದರೆ_ಅಂತ್ಯ
}

ಸ್ಥಿರ ಶೂನ್ಯ put32bits(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *p, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ v){
  p[0] = v>>24;
  p[1] = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)(v>>16);
  p[2] = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)(v>>8);
  p[3] = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)v;
}

ಸ್ಥಿರ ಶೂನ್ಯ vfslog_call(
  sqlite3_vfs *pVfs,
  ಪೂರ್ಣಾಂಕ eEvent,
  ಪೂರ್ಣಾಂಕ iFileid,
  sqlite3_int64 nClick,
  ಪೂರ್ಣಾಂಕ return_ಸಂಕೇತ,
  ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2,
  ಪೂರ್ಣಾಂಕ ಸ್ಥಾನಾಂತರ
){
  VfslogVfs *p = (VfslogVfs *)pVfs;
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *zRec;
  ಆದರೆ( (24+p->nBuf)>ಗಾತ್ರ(p->aBuf) ){
    vfslog_flush(p);
  }
  zRec = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)&p->aBuf[p->nBuf];
  put32bits(&zRec[0], eEvent);
  put32bits(&zRec[4], iFileid);
  put32bits(&zRec[8], (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)(nClick&0xffff));
  put32bits(&zRec[12], return_ಸಂಕೇತ);
  put32bits(&zRec[16], ಗಾತ್ರ_2);
  put32bits(&zRec[20], ಸ್ಥಾನಾಂತರ);
  p->nBuf += 24;
}

ಸ್ಥಿರ ಶೂನ್ಯ vfslog_string(sqlite3_vfs *pVfs, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zStr){
  VfslogVfs *p = (VfslogVfs *)pVfs;
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *zRec;
  ಪೂರ್ಣಾಂಕ nStr = zStr ? (ಪೂರ್ಣಾಂಕ)ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(zStr) : 0;
  ಆದರೆ( (4+nStr+p->nBuf)>ಗಾತ್ರ(p->aBuf) ){
    vfslog_flush(p);
  }
  zRec = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)&p->aBuf[p->nBuf];
  put32bits(&zRec[0], nStr);
  ಆದರೆ( zStr ){
    ಸ್ಮರಣೆ_ನಕಲು(&zRec[4], zStr, nStr);
  }
  p->nBuf += (4 + nStr);
}

ಸ್ಥಿರ ಶೂನ್ಯ vfslog_finalize(VfslogVfs *p){
  ಆದರೆ( p->pLog->pMethods ){
    vfslog_flush(p);
    p->pLog->pMethods->xClose(p->pLog);
  }
  sqlite3_free(p);
}

ಪೂರ್ಣಾಂಕ sqlite3_vfslog_finalize(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zVfs){
  sqlite3_vfs *pVfs;
  pVfs = sqlite3_vfs_ಕಂಡುಹಿಡಿ(zVfs);
  ಆದರೆ( !pVfs || pVfs->xOpen!=vfslogOpen ){
    ಹಿಂತಿರುಗಿ SQLITE_ERROR;
  } 
  sqlite3_vfs_unregister(pVfs);
  vfslog_finalize((VfslogVfs *)pVfs);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

ಪೂರ್ಣಾಂಕ sqlite3_vfslog_new(
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zVfs,               /* New VFS name */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಪೋಷಕ_Vfs,         /* Parent VFS name (or NULL) */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zLog                /* Log file name */
){
  VfslogVfs *p;
  sqlite3_vfs *p_ಪೋಷಕ;
  ಪೂರ್ಣಾಂಕ nByte;
  ಪೂರ್ಣಾಂಕ flags;
  ಪೂರ್ಣಾಂಕ rc;
  ಅಕ್ಷರ *zFile;
  ಪೂರ್ಣಾಂಕ nVfs;

  p_ಪೋಷಕ = sqlite3_vfs_ಕಂಡುಹಿಡಿ(z_ಪೋಷಕ_Vfs);
  ಆದರೆ( !p_ಪೋಷಕ ){
    ಹಿಂತಿರುಗಿ SQLITE_ERROR;
  }

  nVfs = (ಪೂರ್ಣಾಂಕ)ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(zVfs);
  nByte = ಗಾತ್ರ(VfslogVfs) + p_ಪೋಷಕ->szOsFile + nVfs+1+p_ಪೋಷಕ->mxPathname+1;
  p = (VfslogVfs *)sqlite3_malloc(nByte);
  memset(p, 0, nByte);

  p->pVfs = p_ಪೋಷಕ;
  p->pLog = (sqlite3_file *)&p[1];
  ಸ್ಮರಣೆ_ನಕಲು(&p->ತಳ, &vfslog_vfs, ಗಾತ್ರ(sqlite3_vfs));
  p->ತಳ.zName = &((ಅಕ್ಷರ *)p->pLog)[p_ಪೋಷಕ->szOsFile];
  p->ತಳ.szOsFile += p_ಪೋಷಕ->szOsFile;
  ಸ್ಮರಣೆ_ನಕಲು((ಅಕ್ಷರ *)p->ತಳ.zName, zVfs, nVfs);

  zFile = (ಅಕ್ಷರ *)&p->ತಳ.zName[nVfs+1];
  p_ಪೋಷಕ->x_ತುಂಬಿದ_Pathname(p_ಪೋಷಕ, zLog, p_ಪೋಷಕ->mxPathname, zFile);

  flags = SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE|SQLITE_OPEN_SUPER_JOURNAL;
  p_ಪೋಷಕ->xDelete(p_ಪೋಷಕ, zFile, 0);
  rc = p_ಪೋಷಕ->xOpen(p_ಪೋಷಕ, zFile, p->pLog, flags, &flags);
  ಆದರೆ( rc==SQLITE_OK ){
    ಸ್ಮರಣೆ_ನಕಲು(p->aBuf, "sqlite_ostrace1.....", 20);
    p->i_ಸ್ಥಾನಾಂತರ = 0;
    p->nBuf = 20;
    rc = sqlite3_vfs_ರಿಜಿಸ್ಟರ್((sqlite3_vfs *)p, 1);
  }
  ಆದರೆ( rc ){
    vfslog_finalize(p);
  }
  ಹಿಂತಿರುಗಿ rc;
}

ಪೂರ್ಣಾಂಕ sqlite3_vfslog_annotate(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zVfs, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಸಂದೇಶ){
  sqlite3_vfs *pVfs;
  pVfs = sqlite3_vfs_ಕಂಡುಹಿಡಿ(zVfs);
  ಆದರೆ( !pVfs || pVfs->xOpen!=vfslogOpen ){
    ಹಿಂತಿರುಗಿ SQLITE_ERROR;
  } 
  vfslog_call(pVfs, OS_ANNOTATE, 0, 0, 0, 0, 0);
  vfslog_string(pVfs, z_ಸಂದೇಶ);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *vfslog_eventname(ಪೂರ್ಣಾಂಕ eEvent){
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zEvent = 0;

  ಆಯ್ಕೆ( eEvent ){
    ಸ್ಥಿತಿ OS_CLOSE:             zEvent = "xClose"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ OS_READ:              zEvent = "xRead"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ OS_WRITE:             zEvent = "xWrite"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ OS_TRUNCATE:          zEvent = "xTruncate"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ OS_SYNC:              zEvent = "xSync"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ OS_FILESIZE:          zEvent = "xFilesize"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ OS_ಬೀಗ:              zEvent = "xLock"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ OS_UNLOCK:            zEvent = "xUnlock"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ OS_CHECKRESERVEDLOCK: zEvent = "xCheckResLock"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ OS_FILECONTROL:       zEvent = "xFileControl"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ OS_SECTORSIZE:        zEvent = "xSectorSize"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ OS_DEVCHAR:           zEvent = "xDeviceChar"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ OS_OPEN:              zEvent = "xOpen"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ OS_DELETE:            zEvent = "xDelete"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ OS_ACCESS:            zEvent = "xAccess"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ OS_FULLPATHNAME:      zEvent = "xFullPathname"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ OS_RANDOMNESS:        zEvent = "xRandomness"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ OS_SLEEP:             zEvent = "xSleep"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ OS_CURRENTTIME:       zEvent = "xCurrentTime"; ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ OS_SHMUNMAP:          zEvent = "xShmUnmap"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ OS_SHMLOCK:           zEvent = "xShmLock"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ OS_SHMBARRIER:        zEvent = "xShmBarrier"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ OS_SHMMAP:            zEvent = "xShmMap"; ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ OS_ANNOTATE:          zEvent = "annotation"; ನಿಲ್ಲಿಸು;
  }

  ಹಿಂತಿರುಗಿ zEvent;
}

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ VfslogVtab VfslogVtab;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ VfslogCsr VfslogCsr;

/*
** Virtual table type for the vfslog reader module.
*/
ರಚನೆ VfslogVtab {
  sqlite3_vtab ತಳ;              /* Base class */
  sqlite3_file *pFd;              /* File descriptor open on vfslog file */
  sqlite3_int64 nByte;            /* Size of file in bytes */
  ಅಕ್ಷರ *zFile;                    /* File name for pFd */
};

/*
** Virtual table cursor type for the vfslog reader module.
*/
ರಚನೆ VfslogCsr {
  sqlite3_vtab_cursor ತಳ;       /* Base class */
  sqlite3_int64 iRowid;           /* Current rowid. */
  sqlite3_int64 i_ಸ್ಥಾನಾಂತರ;          /* Offset of next record in file */
  ಅಕ್ಷರ *zTransient;               /* Transient 'file' string */
  ಪೂರ್ಣಾಂಕ nFile;                      /* Size of array azFile[] */
  ಅಕ್ಷರ **azFile;                  /* File strings */
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ aBuf[1024];       /* Current vfs log entry (read from file) */
};

ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ get32bits(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *p){
  ಹಿಂತಿರುಗಿ (p[0]<<24) + (p[1]<<16) + (p[2]<<8) + p[3];
}

/*
** The argument must point to a buffer containing a nul-terminated string.
** If the string begins with an SQL quote character it is overwritten by
** the dequoted version. Otherwise the buffer is left unmodified.
*/
ಸ್ಥಿರ ಶೂನ್ಯ dequote(ಅಕ್ಷರ *z){
  ಅಕ್ಷರ quote;                     /* Quote character (if any ) */
  quote = z[0];
  ಆದರೆ( quote=='[' || quote=='\'' || quote=='"' || quote=='`' ){
    ಪೂರ್ಣಾಂಕ i_ಒಳ = 1;                  /* Index of next byte to read from input */
    ಪೂರ್ಣಾಂಕ i_ಹೊರಹೋಗು = 0;                 /* Index of next byte to write to output */
    ಆದರೆ( quote=='[' ) quote = ']';  
    ಆಗ( z[i_ಒಳ] ){
      ಆದರೆ( z[i_ಒಳ]==quote ){
        ಆದರೆ( z[i_ಒಳ+1]!=quote ) ನಿಲ್ಲಿಸು;
        z[i_ಹೊರಹೋಗು++] = quote;
        i_ಒಳ += 2;
      }ಇಲ್ಲದಿದ್ದರೆ{
        z[i_ಹೊರಹೋಗು++] = z[i_ಒಳ++];
      }
    }
    z[i_ಹೊರಹೋಗು] = '\0';
  }
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
/*
** Connect to or create a vfslog virtual table.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vlogConnect(
  sqlite3 *db,
  ಶೂನ್ಯ *pAux,
  ಪೂರ್ಣಾಂಕ argc, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ*argv,
  sqlite3_vtab **ppVtab,
  ಅಕ್ಷರ **pzErr
){
  sqlite3_vfs *pVfs;              /* VFS used to read log file */
  ಪೂರ್ಣಾಂಕ flags;                      /* flags passed to pVfs->xOpen() */
  VfslogVtab *p;
  ಪೂರ್ಣಾಂಕ rc;
  ಪೂರ್ಣಾಂಕ nByte;
  ಅಕ್ಷರ *zFile;

  *ppVtab = 0;
  pVfs = sqlite3_vfs_ಕಂಡುಹಿಡಿ(0);
  nByte = ಗಾತ್ರ(VfslogVtab) + pVfs->szOsFile + pVfs->mxPathname;
  p = sqlite3_malloc(nByte);
  ಆದರೆ( p==0 ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
  memset(p, 0, nByte);

  p->pFd = (sqlite3_file *)&p[1];
  p->zFile = &((ಅಕ್ಷರ *)p->pFd)[pVfs->szOsFile];

  zFile = sqlite3_mprintf("%s", argv[3]);
  ಆದರೆ( !zFile ){
    sqlite3_free(p);
    ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
  }
  dequote(zFile);
  pVfs->x_ತುಂಬಿದ_Pathname(pVfs, zFile, pVfs->mxPathname, p->zFile);
  sqlite3_free(zFile);

  flags = SQLITE_OPEN_READWRITE|SQLITE_OPEN_SUPER_JOURNAL;
  rc = pVfs->xOpen(pVfs, p->zFile, p->pFd, flags, &flags);

  ಆದರೆ( rc==SQLITE_OK ){
    p->pFd->pMethods->x_File_ಗಾತ್ರ(p->pFd, &p->nByte);
    sqlite3_declare_vtab(db, 
        "CREATE TABLE xxx(event, file, click, rc, size, offset)"
    );
    *ppVtab = &p->ತಳ;
  }ಇಲ್ಲದಿದ್ದರೆ{
    sqlite3_free(p);
  }

  ಹಿಂತಿರುಗಿ rc;
}

/*
** There is no "best-index". This virtual table always does a linear
** scan of the binary VFS log file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vlog_ಉತ್ತಮ_ಸೂಚ್ಯಂಕ(sqlite3_vtab *tab, sqlite3_ಸೂಚ್ಯಂಕ_info *pIdxInfo){
  pIdxInfo->estimated_ವೆಚ್ಚ = 10.0;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Disconnect from or destroy a vfslog virtual table.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vlogDisconnect(sqlite3_vtab *pVtab){
  VfslogVtab *p = (VfslogVtab *)pVtab;
  ಆದರೆ( p->pFd->pMethods ){
    p->pFd->pMethods->xClose(p->pFd);
    p->pFd->pMethods = 0;
  }
  sqlite3_free(p);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Open a new vfslog cursor.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vlogOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  VfslogCsr *pCsr;                /* Newly allocated cursor object */

  pCsr = sqlite3_malloc(ಗಾತ್ರ(VfslogCsr));
  ಆದರೆ( !pCsr ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
  memset(pCsr, 0, ಗಾತ್ರ(VfslogCsr));
  *ppCursor = &pCsr->ತಳ;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Close a vfslog cursor.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vlogClose(sqlite3_vtab_cursor *pCursor){
  VfslogCsr *p = (VfslogCsr *)pCursor;
  ಪೂರ್ಣಾಂಕ i;
  ಗಾಗಿ(i=0; i<p->nFile; i++){
    sqlite3_free(p->azFile[i]);
  }
  sqlite3_free(p->azFile);
  sqlite3_free(p->zTransient);
  sqlite3_free(p);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Move a vfslog cursor to the next entry in the file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vlog_ಮುಂದಿನ(sqlite3_vtab_cursor *pCursor){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  VfslogVtab *p = (VfslogVtab *)pCursor->pVtab;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  ಪೂರ್ಣಾಂಕ nRead;

  sqlite3_free(pCsr->zTransient);
  pCsr->zTransient = 0;

  nRead = 24;
  ಆದರೆ( pCsr->i_ಸ್ಥಾನಾಂತರ+nRead<=p->nByte ){
    ಪೂರ್ಣಾಂಕ eEvent;
    rc = p->pFd->pMethods->xRead(p->pFd, pCsr->aBuf, nRead, pCsr->i_ಸ್ಥಾನಾಂತರ);

    eEvent = get32bits(pCsr->aBuf);
    ಆದರೆ( (rc==SQLITE_OK)
     && (eEvent==OS_OPEN || eEvent==OS_DELETE || eEvent==OS_ACCESS) 
    ){
      ಅಕ್ಷರ buf[4];
      rc = p->pFd->pMethods->xRead(p->pFd, buf, 4, pCsr->i_ಸ್ಥಾನಾಂತರ+nRead);
      nRead += 4;
      ಆದರೆ( rc==SQLITE_OK ){
        ಪೂರ್ಣಾಂಕ nStr = get32bits((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)buf);
        ಅಕ್ಷರ *zStr = sqlite3_malloc(nStr+1);
        rc = p->pFd->pMethods->xRead(p->pFd, zStr, nStr, pCsr->i_ಸ್ಥಾನಾಂತರ+nRead);
        zStr[nStr] = '\0';
        nRead += nStr;

        ಆದರೆ( eEvent==OS_OPEN ){
          ಪೂರ್ಣಾಂಕ iFileid = get32bits(&pCsr->aBuf[4]);
          ಆದರೆ( iFileid>=pCsr->nFile ){
            ಪೂರ್ಣಾಂಕ nNew = ಗಾತ್ರ(pCsr->azFile[0])*(iFileid+1);
            pCsr->azFile = (ಅಕ್ಷರ **)sqlite3_realloc(pCsr->azFile, nNew);
            nNew -= ಗಾತ್ರ(pCsr->azFile[0])*pCsr->nFile;
            memset(&pCsr->azFile[pCsr->nFile], 0, nNew);
            pCsr->nFile = iFileid+1;
          }
          sqlite3_free(pCsr->azFile[iFileid]);
          pCsr->azFile[iFileid] = zStr;
        }ಇಲ್ಲದಿದ್ದರೆ{
          pCsr->zTransient = zStr;
        }
      }
    }
  }

  pCsr->iRowid += 1;
  pCsr->i_ಸ್ಥಾನಾಂತರ += nRead;
  ಹಿಂತಿರುಗಿ rc;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vlogEof(sqlite3_vtab_cursor *pCursor){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  VfslogVtab *p = (VfslogVtab *)pCursor->pVtab;
  ಹಿಂತಿರುಗಿ (pCsr->i_ಸ್ಥಾನಾಂತರ>=p->nByte);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vlogFilter(
  sqlite3_vtab_cursor *pCursor, 
  ಪೂರ್ಣಾಂಕ idx_ಅಂಕೆ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *idxStr,
  ಪೂರ್ಣಾಂಕ argc, sqlite3_ಮೌಲ್ಯ **argv
){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  pCsr->iRowid = 0;
  pCsr->i_ಸ್ಥಾನಾಂತರ = 20;
  ಹಿಂತಿರುಗಿ vlog_ಮುಂದಿನ(pCursor);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vlog_ಸ್ತಂಭ(
  sqlite3_vtab_cursor *pCursor, 
  sqlite3_context *ctx, 
  ಪೂರ್ಣಾಂಕ i
){
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ val;
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;

  ಪ್ರತಿಜ್ಞಾ( i<7 );
  val = get32bits(&pCsr->aBuf[4*i]);

  ಆಯ್ಕೆ( i ){
    ಸ್ಥಿತಿ 0: {
      sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(ctx, vfslog_eventname(val), -1, SQLITE_STATIC);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ 1: {
      ಅಕ್ಷರ *zStr = pCsr->zTransient;
      ಆದರೆ( val!=0 && val<(ಚಿಹ್ನೆರಹಿತ)pCsr->nFile ){
        zStr = pCsr->azFile[val];
      }
      sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(ctx, zStr, -1, SQLITE_TRANSIENT);
      ನಿಲ್ಲಿಸು;
    }
    ಪೂರ್ವನಿಯೋಜಿತ:
      sqlite3_ಫಲಿತಾಂಶ_int(ctx, val);
      ನಿಲ್ಲಿಸು;
  }

  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vlogRowid(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid){
  VfslogCsr *pCsr = (VfslogCsr *)pCursor;
  *pRowid = pCsr->iRowid;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

ಪೂರ್ಣಾಂಕ sqlite3_vfslog_ರಿಜಿಸ್ಟರ್(sqlite3 *db){
  ಸ್ಥಿರ sqlite3_module vfslog_module = {
    0,                            /* iVersion */
    vlogConnect,                /* xCreate */
    vlogConnect,                /* xConnect */
    vlog_ಉತ್ತಮ_ಸೂಚ್ಯಂಕ,              /* xBestIndex */
    vlogDisconnect,             /* xDisconnect */
    vlogDisconnect,             /* xDestroy */
    vlogOpen,                   /* xOpen - open a cursor */
    vlogClose,                  /* xClose - close a cursor */
    vlogFilter,                 /* xFilter - configure scan constraints */
    vlog_ಮುಂದಿನ,                   /* xNext - advance a cursor */
    vlogEof,                    /* xEof - check for end of scan */
    vlog_ಸ್ತಂಭ,                 /* xColumn - read data */
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
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_VIRTUALTABLE */

/**************************************************************************
***************************************************************************
** Tcl interface starts here.
*/

#ಆದರೆ defined(SQLITE_TEST) || defined(TCLSH)

#ಅನ್ತರ್ಭಾವ "tclsqlite.h"

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_vfslog(
  ಶೂನ್ಯ *client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ರಚನೆ SqliteDb { sqlite3 *db; };
  sqlite3 *db;
  Tcl_CmdInfo cmdInfo;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_ERROR;

  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *strs[] = { "annotate", "finalize", "new",  "register", 0 };
  ಎಣಿಕೆ VL_enum { VL_ANNOTATE, VL_FINALIZE, VL_NEW, VL_ರಿಜಿಸ್ಟರ್ };
  ಪೂರ್ಣಾಂಕ iSub;

  ಆದರೆ( objc<2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "SUB-COMMAND ...");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( Tcl_Get_ಸೂಚ್ಯಂಕ_From_Obj(interp, objv[1], strs, "sub-command", 0, &iSub) ){
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  ಆಯ್ಕೆ( (ಎಣಿಕೆ VL_enum)iSub ){
    ಸ್ಥಿತಿ VL_ANNOTATE: {
      ಅಕ್ಷರ *zVfs;
      ಅಕ್ಷರ *z_ಸಂದೇಶ;
      ಆದರೆ( objc!=4 ){
        Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 3, objv, "VFS");
        ಹಿಂತಿರುಗಿ TCL_ERROR;
      }
      zVfs = Tcl_GetString(objv[2]);
      z_ಸಂದೇಶ = Tcl_GetString(objv[3]);
      rc = sqlite3_vfslog_annotate(zVfs, z_ಸಂದೇಶ);
      ಆದರೆ( rc!=SQLITE_OK ){
        Tcl_Append_ಫಲಿತಾಂಶ(interp, "failed", (ಅಕ್ಷರ*)0);
        ಹಿಂತಿರುಗಿ TCL_ERROR;
      }
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ VL_FINALIZE: {
      ಅಕ್ಷರ *zVfs;
      ಆದರೆ( objc!=3 ){
        Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 2, objv, "VFS");
        ಹಿಂತಿರುಗಿ TCL_ERROR;
      }
      zVfs = Tcl_GetString(objv[2]);
      rc = sqlite3_vfslog_finalize(zVfs);
      ಆದರೆ( rc!=SQLITE_OK ){
        Tcl_Append_ಫಲಿತಾಂಶ(interp, "failed", (ಅಕ್ಷರ*)0);
        ಹಿಂತಿರುಗಿ TCL_ERROR;
      }
      ನಿಲ್ಲಿಸು;
    };

    ಸ್ಥಿತಿ VL_NEW: {
      ಅಕ್ಷರ *zVfs;
      ಅಕ್ಷರ *z_ಪೋಷಕ;
      ಅಕ್ಷರ *zLog;
      ಆದರೆ( objc!=5 ){
        Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 2, objv, "VFS PARENT LOGFILE");
        ಹಿಂತಿರುಗಿ TCL_ERROR;
      }
      zVfs = Tcl_GetString(objv[2]);
      z_ಪೋಷಕ = Tcl_GetString(objv[3]);
      zLog = Tcl_GetString(objv[4]);
      ಆದರೆ( *z_ಪೋಷಕ=='\0' ) z_ಪೋಷಕ = 0;
      rc = sqlite3_vfslog_new(zVfs, z_ಪೋಷಕ, zLog);
      ಆದರೆ( rc!=SQLITE_OK ){
        Tcl_Append_ಫಲಿತಾಂಶ(interp, "failed", (ಅಕ್ಷರ*)0);
        ಹಿಂತಿರುಗಿ TCL_ERROR;
      }
      ನಿಲ್ಲಿಸು;
    };

    ಸ್ಥಿತಿ VL_ರಿಜಿಸ್ಟರ್: {
      ಅಕ್ಷರ *zDb;
      ಆದರೆ( objc!=3 ){
        Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 2, objv, "DB");
        ಹಿಂತಿರುಗಿ TCL_ERROR;
      }
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_OMIT_VIRTUALTABLE
      Tcl_Append_ಫಲಿತಾಂಶ(interp, "vfslog not available because of "
                               "SQLITE_OMIT_VIRTUALTABLE", (ಶೂನ್ಯ*)0);
      ಹಿಂತಿರುಗಿ TCL_ERROR;
#ಇಲ್ಲದಿದ್ದರೆ
      zDb = Tcl_GetString(objv[2]);
      ಆದರೆ( Tcl_GetCommandInfo(interp, zDb, &cmdInfo) ){
        db = ((ರಚನೆ SqliteDb*)cmdInfo.obj_Client_ದತ್ತಾಂಶ)->db;
        rc = sqlite3_vfslog_ರಿಜಿಸ್ಟರ್(db);
      }
      ಆದರೆ( rc!=SQLITE_OK ){
        Tcl_Append_ಫಲಿತಾಂಶ(interp, "bad sqlite3 handle: ", zDb, (ಶೂನ್ಯ*)0);
        ಹಿಂತಿರುಗಿ TCL_ERROR;
      }
      ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ
    }
  }

  ಹಿಂತಿರುಗಿ TCL_OK;
}

ಪೂರ್ಣಾಂಕ SqlitetestOsinst_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "vfslog", test_vfslog, 0, 0);
  ಹಿಂತಿರುಗಿ TCL_OK;
}

#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_TEST */
