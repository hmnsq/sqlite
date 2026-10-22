/*
** 2007 September 14
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
**
** OVERVIEW:
**
**   This file contains some example code demonstrating how the SQLite 
**   vfs feature can be used to have SQLite operate directly on an 
**   embedded media, without using an intermediate file system.
**
**   Because this is only a demo designed to run on a workstation, the
**   underlying media is simulated using a regular file-system file. The
**   size of the file is fixed when it is first created (default size 10 MB).
**   From SQLite's point of view, this space is used to store a single
**   database file and the journal file. 
**
**   Any statement journal created is stored in volatile memory obtained 
**   from sqlite3_malloc(). Any attempt to create a temporary database file 
**   will fail (SQLITE_IOERR). To prevent SQLite from attempting this,
**   it should be configured to store all temporary database files in 
**   main memory (see pragma "temp_store" or the SQLITE_TEMP_STORE compile 
**   time option).
**
** ASSUMPTIONS:
**
**   After it has been created, the blob file is accessed using the
**   following three functions only:
**
**       mediaRead();            - Read a 512 byte block from the file.
**       mediaWrite();           - Write a 512 byte block to the file.
**       mediaSync();            - Tell the media hardware to sync.
**
**   It is assumed that these can be easily implemented by any "real"
**   media vfs driver adapting this code.
**
** FILE FORMAT:
**
**   The basic principle is that the "database file" is stored at the
**   beginning of the 10 MB blob and grows in a forward direction. The 
**   "journal file" is stored at the end of the 10MB blob and grows
**   in the reverse direction. If, during a transaction, insufficient
**   space is available to expand either the journal or database file,
**   an SQLITE_FULL error is returned. The database file is never allowed
**   to consume more than 90% of the blob space. If SQLite tries to
**   create a file larger than this, SQLITE_FULL is returned.
**
**   No allowance is made for "wear-leveling", as is required by.
**   embedded devices in the absence of equivalent hardware features.
**
**   The first 512 block byte of the file is reserved for storing the
**   size of the "database file". It is updated as part of the sync()
**   operation. On startup, it can only be trusted if no journal file
**   exists. If a journal-file does exist, then it stores the real size
**   of the database region. The second and subsequent blocks store the 
**   actual database content.
**
**   The size of the "journal file" is not stored persistently in the 
**   file. When the system is running, the size of the journal file is
**   stored in volatile memory. When recovering from a crash, this vfs
**   reports a very large size for the journal file. The normal journal
**   header and checksum mechanisms serve to prevent SQLite from 
**   processing any data that lies past the logical end of the journal.
**
**   When SQLite calls OsDelete() to delete the journal file, the final
**   512 bytes of the blob (the area containing the first journal header)
**   are zeroed.
**
** LOCKING:
**
**   File locking is a no-op. Only one connection may be open at any one
**   time using this demo vfs.
*/

#ಅನ್ತರ್ಭಾವ "sqlite3.h"
#ಅನ್ತರ್ಭಾವ <assert.h>
#ಅನ್ತರ್ಭಾವ <string.h>

/*
** Maximum pathname length supported by the fs backend.
*/
#define BLOCKSIZE 512
#define BLOBSIZE 10485760

/*
** Name used to identify this VFS.
*/
#define FS_VFS_NAME "fs"

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ fs_real_file fs_real_file;
ಸಂರಚನಾ fs_real_file {
  sqlite3_file *pFile;
  ಧ್ರುವ ವರ್ಣ *zName;
  ಪೂರ್ಣಾಙ್ಕ nDatabase;              /* Current size of database region */
  ಪೂರ್ಣಾಙ್ಕ nJournal;               /* Current size of journal region */
  ಪೂರ್ಣಾಙ್ಕ nBlob;                  /* Total size of allocated blob */
  ಪೂರ್ಣಾಙ್ಕ nRef;                   /* Number of pointers to this structure */
  fs_real_file *pNext;
  fs_real_file **ppThis;
};

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ fs_file fs_file;
ಸಂರಚನಾ fs_file {
  sqlite3_file base;
  ಪೂರ್ಣಾಙ್ಕ eType;
  fs_real_file *pReal;
};

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ tmp_file tmp_file;
ಸಂರಚನಾ tmp_file {
  sqlite3_file base;
  ಪೂರ್ಣಾಙ್ಕ nSize;
  ಪೂರ್ಣಾಙ್ಕ nAlloc;
  ವರ್ಣ *zAlloc;
};

/* Values for fs_file.eType. */
#define DATABASE_FILE   1
#define JOURNAL_FILE    2

/*
** Method declarations for fs_file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fsClose(sqlite3_file*);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fsRead(sqlite3_file*, ಶೂನ್ಯ*, ಪೂರ್ಣಾಙ್ಕ iAmt, sqlite3_int64 iOfst);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fsWrite(sqlite3_file*, ಧ್ರುವ ಶೂನ್ಯ*, ಪೂರ್ಣಾಙ್ಕ iAmt, sqlite3_int64 iOfst);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fsTruncate(sqlite3_file*, sqlite3_int64 size);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fsSync(sqlite3_file*, ಪೂರ್ಣಾಙ್ಕ flags);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fsFileSize(sqlite3_file*, sqlite3_int64 *pSize);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fsLock(sqlite3_file*, ಪೂರ್ಣಾಙ್ಕ);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fsUnlock(sqlite3_file*, ಪೂರ್ಣಾಙ್ಕ);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fsCheckReservedLock(sqlite3_file*, ಪೂರ್ಣಾಙ್ಕ *pResOut);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fsFileControl(sqlite3_file*, ಪೂರ್ಣಾಙ್ಕ op, ಶೂನ್ಯ *pArg);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fsSectorSize(sqlite3_file*);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fsDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for tmp_file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tmpClose(sqlite3_file*);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tmpRead(sqlite3_file*, ಶೂನ್ಯ*, ಪೂರ್ಣಾಙ್ಕ iAmt, sqlite3_int64 iOfst);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tmpWrite(sqlite3_file*, ಧ್ರುವ ಶೂನ್ಯ*, ಪೂರ್ಣಾಙ್ಕ iAmt, sqlite3_int64 iOfst);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tmpTruncate(sqlite3_file*, sqlite3_int64 size);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tmpSync(sqlite3_file*, ಪೂರ್ಣಾಙ್ಕ flags);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tmpFileSize(sqlite3_file*, sqlite3_int64 *pSize);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tmpLock(sqlite3_file*, ಪೂರ್ಣಾಙ್ಕ);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tmpUnlock(sqlite3_file*, ಪೂರ್ಣಾಙ್ಕ);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tmpCheckReservedLock(sqlite3_file*, ಪೂರ್ಣಾಙ್ಕ *pResOut);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tmpFileControl(sqlite3_file*, ಪೂರ್ಣಾಙ್ಕ op, ಶೂನ್ಯ *pArg);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tmpSectorSize(sqlite3_file*);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tmpDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for fs_vfs.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fsOpen(sqlite3_vfs*, ಧ್ರುವ ವರ್ಣ *, sqlite3_file*, ಪೂರ್ಣಾಙ್ಕ , ಪೂರ್ಣಾಙ್ಕ *);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fsDelete(sqlite3_vfs*, ಧ್ರುವ ವರ್ಣ *zName, ಪೂರ್ಣಾಙ್ಕ syncDir);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fsAccess(sqlite3_vfs*, ಧ್ರುವ ವರ್ಣ *zName, ಪೂರ್ಣಾಙ್ಕ flags, ಪೂರ್ಣಾಙ್ಕ *);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fsFullPathname(sqlite3_vfs*, ಧ್ರುವ ವರ್ಣ *zName, ಪೂರ್ಣಾಙ್ಕ nOut,ವರ್ಣ *zOut);
ಸ್ಥಿರ ಶೂನ್ಯ *fsDlOpen(sqlite3_vfs*, ಧ್ರುವ ವರ್ಣ *zFilename);
ಸ್ಥಿರ ಶೂನ್ಯ fsDlError(sqlite3_vfs*, ಪೂರ್ಣಾಙ್ಕ nByte, ವರ್ಣ *z_Err_ಸಂದೇಶ);
ಸ್ಥಿರ ಶೂನ್ಯ (*fsDlSym(sqlite3_vfs*,ಶೂನ್ಯ*, ಧ್ರುವ ವರ್ಣ *z_ಚಿಹ್ನೆ))(ಶೂನ್ಯ);
ಸ್ಥಿರ ಶೂನ್ಯ fsDlClose(sqlite3_vfs*, ಶೂನ್ಯ*);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fsRandomness(sqlite3_vfs*, ಪೂರ್ಣಾಙ್ಕ nByte, ವರ್ಣ *zOut);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fsSleep(sqlite3_vfs*, ಪೂರ್ಣಾಙ್ಕ microseconds);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fsCurrentTime(sqlite3_vfs*, ದ್ವಿಗುಣ*);


ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ fs_vfs_t fs_vfs_t;
ಸಂರಚನಾ fs_vfs_t {
  sqlite3_vfs base;
  fs_real_file *pFileList;
  sqlite3_vfs *pParent;
};

ಸ್ಥಿರ fs_vfs_t fs_vfs = {
  {
    1,                                          /* iVersion */
    0,                                          /* szOsFile */
    0,                                          /* mxPathname */
    0,                                          /* pNext */
    FS_VFS_NAME,                                /* zName */
    0,                                          /* pAppData */
    fsOpen,                                     /* xOpen */
    fsDelete,                                   /* xDelete */
    fsAccess,                                   /* xAccess */
    fsFullPathname,                             /* xFullPathname */
    fsDlOpen,                                   /* xDlOpen */
    fsDlError,                                  /* xDlError */
    fsDlSym,                                    /* xDlSym */
    fsDlClose,                                  /* xDlClose */
    fsRandomness,                               /* xRandomness */
    fsSleep,                                    /* xSleep */
    fsCurrentTime,                              /* xCurrentTime */
    0                                           /* xCurrentTimeInt64 */
  }, 
  0,                                            /* pFileList */
  0                                             /* pParent */
};

ಸ್ಥಿರ sqlite3_io_methods fs_io_methods = {
  1,                            /* iVersion */
  fsClose,                      /* xClose */
  fsRead,                       /* xRead */
  fsWrite,                      /* xWrite */
  fsTruncate,                   /* xTruncate */
  fsSync,                       /* xSync */
  fsFileSize,                   /* xFileSize */
  fsLock,                       /* xLock */
  fsUnlock,                     /* xUnlock */
  fsCheckReservedLock,          /* xCheckReservedLock */
  fsFileControl,                /* xFileControl */
  fsSectorSize,                 /* xSectorSize */
  fsDeviceCharacteristics,      /* xDeviceCharacteristics */
  0,                            /* xShmMap */
  0,                            /* xShmLock */
  0,                            /* xShmBarrier */
  0                             /* xShmUnmap */
};


ಸ್ಥಿರ sqlite3_io_methods tmp_io_methods = {
  1,                            /* iVersion */
  tmpClose,                     /* xClose */
  tmpRead,                      /* xRead */
  tmpWrite,                     /* xWrite */
  tmpTruncate,                  /* xTruncate */
  tmpSync,                      /* xSync */
  tmpFileSize,                  /* xFileSize */
  tmpLock,                      /* xLock */
  tmpUnlock,                    /* xUnlock */
  tmpCheckReservedLock,         /* xCheckReservedLock */
  tmpFileControl,               /* xFileControl */
  tmpSectorSize,                /* xSectorSize */
  tmpDeviceCharacteristics,     /* xDeviceCharacteristics */
  0,                            /* xShmMap */
  0,                            /* xShmLock */
  0,                            /* xShmBarrier */
  0                             /* xShmUnmap */
};

/* Useful macros used in several places */
#define MIN(x,y) ((x)<(y)?(x):(y))
#define MAX(x,y) ((x)>(y)?(x):(y))


/*
** Close a tmp-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tmpClose(sqlite3_file *pFile){
  tmp_file *pTmp = (tmp_file *)pFile;
  sqlite3_free(pTmp->zAlloc);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Read data from a tmp-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tmpRead(
  sqlite3_file *pFile, 
  ಶೂನ್ಯ *zBuf, 
  ಪೂರ್ಣಾಙ್ಕ iAmt, 
  sqlite_int64 iOfst
){
  tmp_file *pTmp = (tmp_file *)pFile;
  ಯದಿ( (iAmt+iOfst)>pTmp->nSize ){
    ಪ್ರತ್ಯಾಗಮ SQLITE_IOERR_SHORT_READ;
  }
  memcpy(zBuf, &pTmp->zAlloc[iOfst], iAmt);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Write data to a tmp-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tmpWrite(
  sqlite3_file *pFile, 
  ಧ್ರುವ ಶೂನ್ಯ *zBuf, 
  ಪೂರ್ಣಾಙ್ಕ iAmt, 
  sqlite_int64 iOfst
){
  tmp_file *pTmp = (tmp_file *)pFile;
  ಯದಿ( (iAmt+iOfst)>pTmp->nAlloc ){
    ಪೂರ್ಣಾಙ್ಕ nNew = (ಪೂರ್ಣಾಙ್ಕ)(2*(iAmt+iOfst+pTmp->nAlloc));
    ವರ್ಣ *zNew = sqlite3_realloc(pTmp->zAlloc, nNew);
    ಯದಿ( !zNew ){
      ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
    }
    pTmp->zAlloc = zNew;
    pTmp->nAlloc = nNew;
  }
  memcpy(&pTmp->zAlloc[iOfst], zBuf, iAmt);
  pTmp->nSize = (ಪೂರ್ಣಾಙ್ಕ)MAX(pTmp->nSize, iOfst+iAmt);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Truncate a tmp-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tmpTruncate(sqlite3_file *pFile, sqlite_int64 size){
  tmp_file *pTmp = (tmp_file *)pFile;
  pTmp->nSize = (ಪೂರ್ಣಾಙ್ಕ)MIN(pTmp->nSize, size);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Sync a tmp-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tmpSync(sqlite3_file *pFile, ಪೂರ್ಣಾಙ್ಕ flags){
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Return the current file-size of a tmp-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tmpFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  tmp_file *pTmp = (tmp_file *)pFile;
  *pSize = pTmp->nSize;
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Lock a tmp-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tmpLock(sqlite3_file *pFile, ಪೂರ್ಣಾಙ್ಕ eLock){
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Unlock a tmp-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tmpUnlock(sqlite3_file *pFile, ಪೂರ್ಣಾಙ್ಕ eLock){
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on a tmp-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tmpCheckReservedLock(sqlite3_file *pFile, ಪೂರ್ಣಾಙ್ಕ *pResOut){
  *pResOut = 0;
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** File control method. For custom operations on a tmp-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tmpFileControl(sqlite3_file *pFile, ಪೂರ್ಣಾಙ್ಕ op, ಶೂನ್ಯ *pArg){
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Return the sector-size in bytes for a tmp-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tmpSectorSize(sqlite3_file *pFile){
  ಪ್ರತ್ಯಾಗಮ 0;
}

/*
** Return the device characteristic flags supported by a tmp-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tmpDeviceCharacteristics(sqlite3_file *pFile){
  ಪ್ರತ್ಯಾಗಮ 0;
}

/*
** Close an fs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fsClose(sqlite3_file *pFile){
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;

  /* Decrement the real_file ref-count. */
  pReal->nRef--;
  ಪ್ರತಿಜ್ಞಾ(pReal->nRef>=0);

  /* When the ref-count reaches 0, destroy the structure */
  ಯದಿ( pReal->nRef==0 ){
    *pReal->ppThis = pReal->pNext;
    ಯದಿ( pReal->pNext ){
      pReal->pNext->ppThis = pReal->ppThis;
    }
    rc = pReal->pFile->pMethods->xClose(pReal->pFile);
    sqlite3_free(pReal);
  }

  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Read data from an fs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fsRead(
  sqlite3_file *pFile, 
  ಶೂನ್ಯ *zBuf, 
  ಪೂರ್ಣಾಙ್ಕ iAmt, 
  sqlite_int64 iOfst
){
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  sqlite3_file *pF = pReal->pFile;

  ಯದಿ( (p->eType==DATABASE_FILE && (iAmt+iOfst)>pReal->nDatabase)
   || (p->eType==JOURNAL_FILE && (iAmt+iOfst)>pReal->nJournal)
  ){
    rc = SQLITE_IOERR_SHORT_READ;
  }ಅನ್ಯಥಾ ಯದಿ( p->eType==DATABASE_FILE ){
    rc = pF->pMethods->xRead(pF, zBuf, iAmt, iOfst+BLOCKSIZE);
  }ಅನ್ಯಥಾ{
    /* Journal file. */
    ಪೂರ್ಣಾಙ್ಕ iRem = iAmt;
    ಪೂರ್ಣಾಙ್ಕ iBuf = 0;
    ಪೂರ್ಣಾಙ್ಕ ii = (ಪೂರ್ಣಾಙ್ಕ)iOfst;
    ಯಾವತ್( iRem>0 && rc==SQLITE_OK ){
      ಪೂರ್ಣಾಙ್ಕ iRealOff = pReal->nBlob - BLOCKSIZE*((ii/BLOCKSIZE)+1) + ii%BLOCKSIZE;
      ಪೂರ್ಣಾಙ್ಕ iRealAmt = MIN(iRem, BLOCKSIZE - (iRealOff%BLOCKSIZE));

      rc = pF->pMethods->xRead(pF, &((ವರ್ಣ *)zBuf)[iBuf], iRealAmt, iRealOff);
      ii += iRealAmt;
      iBuf += iRealAmt;
      iRem -= iRealAmt;
    }
  }

  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Write data to an fs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fsWrite(
  sqlite3_file *pFile, 
  ಧ್ರುವ ಶೂನ್ಯ *zBuf, 
  ಪೂರ್ಣಾಙ್ಕ iAmt, 
  sqlite_int64 iOfst
){
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  sqlite3_file *pF = pReal->pFile;

  ಯದಿ( p->eType==DATABASE_FILE ){
    ಯದಿ( (iAmt+iOfst+BLOCKSIZE)>(pReal->nBlob-pReal->nJournal) ){
      rc = SQLITE_FULL;
    }ಅನ್ಯಥಾ{
      rc = pF->pMethods->xWrite(pF, zBuf, iAmt, iOfst+BLOCKSIZE);
      ಯದಿ( rc==SQLITE_OK ){
        pReal->nDatabase = (ಪೂರ್ಣಾಙ್ಕ)MAX(pReal->nDatabase, iAmt+iOfst);
      }
    }
  }ಅನ್ಯಥಾ{
    /* Journal file. */
    ಪೂರ್ಣಾಙ್ಕ iRem = iAmt;
    ಪೂರ್ಣಾಙ್ಕ iBuf = 0;
    ಪೂರ್ಣಾಙ್ಕ ii = (ಪೂರ್ಣಾಙ್ಕ)iOfst;
    ಯಾವತ್( iRem>0 && rc==SQLITE_OK ){
      ಪೂರ್ಣಾಙ್ಕ iRealOff = pReal->nBlob - BLOCKSIZE*((ii/BLOCKSIZE)+1) + ii%BLOCKSIZE;
      ಪೂರ್ಣಾಙ್ಕ iRealAmt = MIN(iRem, BLOCKSIZE - (iRealOff%BLOCKSIZE));

      ಯದಿ( iRealOff<(pReal->nDatabase+BLOCKSIZE) ){
        rc = SQLITE_FULL;
      }ಅನ್ಯಥಾ{
        rc = pF->pMethods->xWrite(pF, &((ವರ್ಣ *)zBuf)[iBuf], iRealAmt,iRealOff);
        ii += iRealAmt;
        iBuf += iRealAmt;
        iRem -= iRealAmt;
      }
    }
    ಯದಿ( rc==SQLITE_OK ){
      pReal->nJournal = (ಪೂರ್ಣಾಙ್ಕ)MAX(pReal->nJournal, iAmt+iOfst);
    }
  }

  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Truncate an fs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fsTruncate(sqlite3_file *pFile, sqlite_int64 size){
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  ಯದಿ( p->eType==DATABASE_FILE ){
    pReal->nDatabase = (ಪೂರ್ಣಾಙ್ಕ)MIN(pReal->nDatabase, size);
  }ಅನ್ಯಥಾ{
    pReal->nJournal = (ಪೂರ್ಣಾಙ್ಕ)MIN(pReal->nJournal, size);
  }
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Sync an fs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fsSync(sqlite3_file *pFile, ಪೂರ್ಣಾಙ್ಕ flags){
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  sqlite3_file *pRealFile = pReal->pFile;
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;

  ಯದಿ( p->eType==DATABASE_FILE ){
    unsigned ವರ್ಣ zSize[4];
    zSize[0] = (pReal->nDatabase&0xFF000000)>>24;
    zSize[1] = (unsigned ವರ್ಣ)((pReal->nDatabase&0x00FF0000)>>16);
    zSize[2] = (pReal->nDatabase&0x0000FF00)>>8;
    zSize[3] = (pReal->nDatabase&0x000000FF);
    rc = pRealFile->pMethods->xWrite(pRealFile, zSize, 4, 0);
  }
  ಯದಿ( rc==SQLITE_OK ){
    rc = pRealFile->pMethods->xSync(pRealFile, flags&(~SQLITE_SYNC_DATAONLY));
  }

  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Return the current file-size of an fs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fsFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  ಯದಿ( p->eType==DATABASE_FILE ){
    *pSize = pReal->nDatabase;
  }ಅನ್ಯಥಾ{
    *pSize = pReal->nJournal;
  }
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Lock an fs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fsLock(sqlite3_file *pFile, ಪೂರ್ಣಾಙ್ಕ eLock){
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Unlock an fs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fsUnlock(sqlite3_file *pFile, ಪೂರ್ಣಾಙ್ಕ eLock){
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on an fs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fsCheckReservedLock(sqlite3_file *pFile, ಪೂರ್ಣಾಙ್ಕ *pResOut){
  *pResOut = 0;
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** File control method. For custom operations on an fs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fsFileControl(sqlite3_file *pFile, ಪೂರ್ಣಾಙ್ಕ op, ಶೂನ್ಯ *pArg){
  ಯದಿ( op==SQLITE_FCNTL_PRAGMA ) ಪ್ರತ್ಯಾಗಮ SQLITE_NOTFOUND;
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Return the sector-size in bytes for an fs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fsSectorSize(sqlite3_file *pFile){
  ಪ್ರತ್ಯಾಗಮ BLOCKSIZE;
}

/*
** Return the device characteristic flags supported by an fs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fsDeviceCharacteristics(sqlite3_file *pFile){
  ಪ್ರತ್ಯಾಗಮ 0;
}

/*
** Open an fs file handle.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fsOpen(
  sqlite3_vfs *pVfs,
  ಧ್ರುವ ವರ್ಣ *zName,
  sqlite3_file *pFile,
  ಪೂರ್ಣಾಙ್ಕ flags,
  ಪೂರ್ಣಾಙ್ಕ *pOutFlags
){
  fs_vfs_t *pFsVfs = (fs_vfs_t *)pVfs;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = 0;
  ಪೂರ್ಣಾಙ್ಕ eType;
  ಪೂರ್ಣಾಙ್ಕ nName;
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;

  ಯದಿ( 0==(flags&(SQLITE_OPEN_MAIN_DB|SQLITE_OPEN_MAIN_JOURNAL)) ){
    tmp_file *p2 = (tmp_file *)pFile;
    memset(p2, 0, ಪರಿಮಾಣ(*p2));
    p2->base.pMethods = &tmp_io_methods;
    ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
  }

  eType = ((flags&(SQLITE_OPEN_MAIN_DB))?DATABASE_FILE:JOURNAL_FILE);
  p->base.pMethods = &fs_io_methods;
  p->eType = eType;

  ಪ್ರತಿಜ್ಞಾ(strlen("-journal")==8);
  nName = (ಪೂರ್ಣಾಙ್ಕ)strlen(zName)-((eType==JOURNAL_FILE)?8:0);
  pReal=pFsVfs->pFileList; 
  ಪ್ರತಿ(; pReal && strncmp(pReal->zName, zName, nName); pReal=pReal->pNext);

  ಯದಿ( !pReal ){
    ಪೂರ್ಣಾಙ್ಕ real_flags = (flags&~(SQLITE_OPEN_MAIN_DB))|SQLITE_OPEN_TEMP_DB;
    sqlite3_int64 size;
    sqlite3_file *pRealFile;
    sqlite3_vfs *pParent = pFsVfs->pParent;
    ಪ್ರತಿಜ್ಞಾ(eType==DATABASE_FILE);

    pReal = (fs_real_file *)sqlite3_malloc(ಪರಿಮಾಣ(*pReal)+pParent->szOsFile);
    ಯದಿ( !pReal ){
      rc = SQLITE_NOMEM;
      ಗಚ್ಛ open_out;
    }
    memset(pReal, 0, ಪರಿಮಾಣ(*pReal)+pParent->szOsFile);
    pReal->zName = zName;
    pReal->pFile = (sqlite3_file *)(&pReal[1]);

    rc = pParent->xOpen(pParent, zName, pReal->pFile, real_flags, pOutFlags);
    ಯದಿ( rc!=SQLITE_OK ){
      ಗಚ್ಛ open_out;
    }
    pRealFile = pReal->pFile;

    rc = pRealFile->pMethods->xFileSize(pRealFile, &size);
    ಯದಿ( rc!=SQLITE_OK ){
      ಗಚ್ಛ open_out;
    }
    ಯದಿ( size==0 ){
      rc = pRealFile->pMethods->xWrite(pRealFile, "\0", 1, BLOBSIZE-1);
      pReal->nBlob = BLOBSIZE;
    }ಅನ್ಯಥಾ{
      unsigned ವರ್ಣ zS[4];
      pReal->nBlob = (ಪೂರ್ಣಾಙ್ಕ)size;
      rc = pRealFile->pMethods->xRead(pRealFile, zS, 4, 0);
      pReal->nDatabase = (zS[0]<<24)+(zS[1]<<16)+(zS[2]<<8)+zS[3];
      ಯದಿ( rc==SQLITE_OK ){
        rc = pRealFile->pMethods->xRead(pRealFile, zS, 4, pReal->nBlob-4);
        ಯದಿ( zS[0] || zS[1] || zS[2] || zS[3] ){
          pReal->nJournal = pReal->nBlob;
        }
      }
    }

    ಯದಿ( rc==SQLITE_OK ){
      pReal->pNext = pFsVfs->pFileList;
      ಯದಿ( pReal->pNext ){
        pReal->pNext->ppThis = &pReal->pNext;
      }
      pReal->ppThis = &pFsVfs->pFileList;
      pFsVfs->pFileList = pReal;
    }
  }

open_out:
  ಯದಿ( pReal ){
    ಯದಿ( rc==SQLITE_OK ){
      p->pReal = pReal;
      pReal->nRef++;
    }ಅನ್ಯಥಾ{
      ಯದಿ( pReal->pFile->pMethods ){
        pReal->pFile->pMethods->xClose(pReal->pFile);
      }
      sqlite3_free(pReal);
    }
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fsDelete(sqlite3_vfs *pVfs, ಧ್ರುವ ವರ್ಣ *zPath, ಪೂರ್ಣಾಙ್ಕ dirSync){
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;
  fs_vfs_t *pFsVfs = (fs_vfs_t *)pVfs;
  fs_real_file *pReal;
  sqlite3_file *pF;
  ಪೂರ್ಣಾಙ್ಕ nName = (ಪೂರ್ಣಾಙ್ಕ)strlen(zPath) - 8;

  ಪ್ರತಿಜ್ಞಾ(strlen("-journal")==8);
  ಪ್ರತಿಜ್ಞಾ(strcmp("-journal", &zPath[nName])==0);

  pReal = pFsVfs->pFileList; 
  ಪ್ರತಿ(; pReal && strncmp(pReal->zName, zPath, nName); pReal=pReal->pNext);
  ಯದಿ( pReal ){
    pF = pReal->pFile;
    rc = pF->pMethods->xWrite(pF, "\0\0\0\0", 4, pReal->nBlob-BLOCKSIZE);
    ಯದಿ( rc==SQLITE_OK ){
      pReal->nJournal = 0;
    }
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fsAccess(
  sqlite3_vfs *pVfs, 
  ಧ್ರುವ ವರ್ಣ *zPath, 
  ಪೂರ್ಣಾಙ್ಕ flags, 
  ಪೂರ್ಣಾಙ್ಕ *pResOut
){
  fs_vfs_t *pFsVfs = (fs_vfs_t *)pVfs;
  fs_real_file *pReal;
  ಪೂರ್ಣಾಙ್ಕ isJournal = 0;
  ಪೂರ್ಣಾಙ್ಕ nName = (ಪೂರ್ಣಾಙ್ಕ)strlen(zPath);

  ಯದಿ( flags!=SQLITE_ACCESS_EXISTS ){
    sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
    ಪ್ರತ್ಯಾಗಮ pParent->xAccess(pParent, zPath, flags, pResOut);
  }

  ಪ್ರತಿಜ್ಞಾ(strlen("-journal")==8);
  ಯದಿ( nName>8 && strcmp("-journal", &zPath[nName-8])==0 ){
    nName -= 8;
    isJournal = 1;
  }

  pReal = pFsVfs->pFileList; 
  ಪ್ರತಿ(; pReal && strncmp(pReal->zName, zPath, nName); pReal=pReal->pNext);

  *pResOut = (pReal && (!isJournal || pReal->nJournal>0));
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (FS_MAX_PATHNAME+1) bytes.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fsFullPathname(
  sqlite3_vfs *pVfs,            /* Pointer to vfs object */
  ಧ್ರುವ ವರ್ಣ *zPath,            /* Possibly relative input path */
  ಪೂರ್ಣಾಙ್ಕ nOut,                     /* Size of output buffer in bytes */
  ವರ್ಣ *zOut                    /* Output buffer */
){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  ಪ್ರತ್ಯಾಗಮ pParent->xFullPathname(pParent, zPath, nOut, zOut);
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
ಸ್ಥಿರ ಶೂನ್ಯ *fsDlOpen(sqlite3_vfs *pVfs, ಧ್ರುವ ವರ್ಣ *zPath){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  ಪ್ರತ್ಯಾಗಮ pParent->xDlOpen(pParent, zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
ಸ್ಥಿರ ಶೂನ್ಯ fsDlError(sqlite3_vfs *pVfs, ಪೂರ್ಣಾಙ್ಕ nByte, ವರ್ಣ *z_Err_ಸಂದೇಶ){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  pParent->xDlError(pParent, nByte, z_Err_ಸಂದೇಶ);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
ಸ್ಥಿರ ಶೂನ್ಯ (*fsDlSym(sqlite3_vfs *pVfs, ಶೂನ್ಯ *pH, ಧ್ರುವ ವರ್ಣ *zSym))(ಶೂನ್ಯ){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  ಪ್ರತ್ಯಾಗಮ pParent->xDlSym(pParent, pH, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
ಸ್ಥಿರ ಶೂನ್ಯ fsDlClose(sqlite3_vfs *pVfs, ಶೂನ್ಯ *pHandle){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  pParent->xDlClose(pParent, pHandle);
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fsRandomness(sqlite3_vfs *pVfs, ಪೂರ್ಣಾಙ್ಕ nByte, ವರ್ಣ *zBufOut){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  ಪ್ರತ್ಯಾಗಮ pParent->xRandomness(pParent, nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fsSleep(sqlite3_vfs *pVfs, ಪೂರ್ಣಾಙ್ಕ nMicro){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  ಪ್ರತ್ಯಾಗಮ pParent->xSleep(pParent, nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fsCurrentTime(sqlite3_vfs *pVfs, ದ್ವಿಗುಣ *pTimeOut){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  ಪ್ರತ್ಯಾಗಮ pParent->xCurrentTime(pParent, pTimeOut);
}

/*
** This procedure registers the fs vfs with SQLite. If the argument is
** true, the fs vfs becomes the new default vfs. It is the only publicly
** available function in this file.
*/
ಪೂರ್ಣಾಙ್ಕ fs_register(ಶೂನ್ಯ){
  ಯದಿ( fs_vfs.pParent ) ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
  fs_vfs.pParent = sqlite3_vfs_find(0);
  fs_vfs.base.mxPathname = fs_vfs.pParent->mxPathname;
  fs_vfs.base.szOsFile = MAX(ಪರಿಮಾಣ(tmp_file), ಪರಿಮಾಣ(fs_file));
  ಪ್ರತ್ಯಾಗಮ sqlite3_vfs_register(&fs_vfs.base, 0);
}

#ifdef SQLITE_TEST
  ಪೂರ್ಣಾಙ್ಕ SqlitetestOnefile_Init() {ಪ್ರತ್ಯಾಗಮ fs_register();}
#endif
