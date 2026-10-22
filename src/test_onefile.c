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

#ଅନ୍ତର୍ଭାଵ "sqlite3.h"
#ଅନ୍ତର୍ଭାଵ <assert.h>
#ଅନ୍ତର୍ଭାଵ <string.h>

/*
** Maximum pathname length supported by the fs backend.
*/
#define BLOCKSIZE 512
#define BLOBSIZE 10485760

/*
** Name used to identify this VFS.
*/
#define FS_VFS_NAME "fs"

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା fs_real_file fs_real_file;
ସଂରଚନା fs_real_file {
  sqlite3_file *pFile;
  ଅଚଳ ଅକ୍ଷର *zName;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nDatabase;              /* Current size of database region */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nJournal;               /* Current size of journal region */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nBlob;                  /* Total size of allocated blob */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nRef;                   /* Number of pointers to this structure */
  fs_real_file *pNext;
  fs_real_file **ppThis;
};

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା fs_file fs_file;
ସଂରଚନା fs_file {
  sqlite3_file base;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା eType;
  fs_real_file *pReal;
};

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା tmp_file tmp_file;
ସଂରଚନା tmp_file {
  sqlite3_file base;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nSize;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nAlloc;
  ଅକ୍ଷର *zAlloc;
};

/* Values for fs_file.eType. */
#define DATABASE_FILE   1
#define JOURNAL_FILE    2

/*
** Method declarations for fs_file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fsClose(sqlite3_file*);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fsRead(sqlite3_file*, ଶୂନ୍ୟ*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iAmt, sqlite3_int64 iOfst);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fsWrite(sqlite3_file*, ଅଚଳ ଶୂନ୍ୟ*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iAmt, sqlite3_int64 iOfst);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fsTruncate(sqlite3_file*, sqlite3_int64 size);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fsSync(sqlite3_file*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fsFileSize(sqlite3_file*, sqlite3_int64 *pSize);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fsLock(sqlite3_file*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fsUnlock(sqlite3_file*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fsCheckReservedLock(sqlite3_file*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pResOut);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fsFileControl(sqlite3_file*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା op, ଶୂନ୍ୟ *pArg);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fsSectorSize(sqlite3_file*);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fsDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for tmp_file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tmpClose(sqlite3_file*);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tmpRead(sqlite3_file*, ଶୂନ୍ୟ*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iAmt, sqlite3_int64 iOfst);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tmpWrite(sqlite3_file*, ଅଚଳ ଶୂନ୍ୟ*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iAmt, sqlite3_int64 iOfst);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tmpTruncate(sqlite3_file*, sqlite3_int64 size);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tmpSync(sqlite3_file*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tmpFileSize(sqlite3_file*, sqlite3_int64 *pSize);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tmpLock(sqlite3_file*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tmpUnlock(sqlite3_file*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tmpCheckReservedLock(sqlite3_file*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pResOut);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tmpFileControl(sqlite3_file*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା op, ଶୂନ୍ୟ *pArg);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tmpSectorSize(sqlite3_file*);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tmpDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for fs_vfs.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fsOpen(sqlite3_vfs*, ଅଚଳ ଅକ୍ଷର *, sqlite3_file*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା , ପୂର୍ଣ୍ଣସଂଖ୍ୟା *);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fsDelete(sqlite3_vfs*, ଅଚଳ ଅକ୍ଷର *zName, ପୂର୍ଣ୍ଣସଂଖ୍ୟା syncDir);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fsAccess(sqlite3_vfs*, ଅଚଳ ଅକ୍ଷର *zName, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fsFullPathname(sqlite3_vfs*, ଅଚଳ ଅକ୍ଷର *zName, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nOut,ଅକ୍ଷର *zOut);
ସ୍ଥିର ଶୂନ୍ୟ *fsDlOpen(sqlite3_vfs*, ଅଚଳ ଅକ୍ଷର *zFilename);
ସ୍ଥିର ଶୂନ୍ୟ fsDlError(sqlite3_vfs*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nByte, ଅକ୍ଷର *z_Err_ସନ୍ଦେଶ);
ସ୍ଥିର ଶୂନ୍ୟ (*fsDlSym(sqlite3_vfs*,ଶୂନ୍ୟ*, ଅଚଳ ଅକ୍ଷର *z_ପ୍ରତୀକ))(ଶୂନ୍ୟ);
ସ୍ଥିର ଶୂନ୍ୟ fsDlClose(sqlite3_vfs*, ଶୂନ୍ୟ*);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fsRandomness(sqlite3_vfs*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nByte, ଅକ୍ଷର *zOut);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fsSleep(sqlite3_vfs*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା microseconds);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fsCurrentTime(sqlite3_vfs*, ଦ୍ୱିଗୁଣ*);


ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା fs_vfs_t fs_vfs_t;
ସଂରଚନା fs_vfs_t {
  sqlite3_vfs base;
  fs_real_file *pFileList;
  sqlite3_vfs *pParent;
};

ସ୍ଥିର fs_vfs_t fs_vfs = {
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

ସ୍ଥିର sqlite3_io_methods fs_io_methods = {
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


ସ୍ଥିର sqlite3_io_methods tmp_io_methods = {
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tmpClose(sqlite3_file *pFile){
  tmp_file *pTmp = (tmp_file *)pFile;
  sqlite3_free(pTmp->zAlloc);
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Read data from a tmp-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tmpRead(
  sqlite3_file *pFile, 
  ଶୂନ୍ୟ *zBuf, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iAmt, 
  sqlite_int64 iOfst
){
  tmp_file *pTmp = (tmp_file *)pFile;
  ଯଦି( (iAmt+iOfst)>pTmp->nSize ){
    ଫେରନ୍ତୁ SQLITE_IOERR_SHORT_READ;
  }
  memcpy(zBuf, &pTmp->zAlloc[iOfst], iAmt);
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Write data to a tmp-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tmpWrite(
  sqlite3_file *pFile, 
  ଅଚଳ ଶୂନ୍ୟ *zBuf, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iAmt, 
  sqlite_int64 iOfst
){
  tmp_file *pTmp = (tmp_file *)pFile;
  ଯଦି( (iAmt+iOfst)>pTmp->nAlloc ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା nNew = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(2*(iAmt+iOfst+pTmp->nAlloc));
    ଅକ୍ଷର *zNew = sqlite3_realloc(pTmp->zAlloc, nNew);
    ଯଦି( !zNew ){
      ଫେରନ୍ତୁ SQLITE_NOMEM;
    }
    pTmp->zAlloc = zNew;
    pTmp->nAlloc = nNew;
  }
  memcpy(&pTmp->zAlloc[iOfst], zBuf, iAmt);
  pTmp->nSize = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)MAX(pTmp->nSize, iOfst+iAmt);
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Truncate a tmp-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tmpTruncate(sqlite3_file *pFile, sqlite_int64 size){
  tmp_file *pTmp = (tmp_file *)pFile;
  pTmp->nSize = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)MIN(pTmp->nSize, size);
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Sync a tmp-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tmpSync(sqlite3_file *pFile, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags){
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Return the current file-size of a tmp-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tmpFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  tmp_file *pTmp = (tmp_file *)pFile;
  *pSize = pTmp->nSize;
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Lock a tmp-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tmpLock(sqlite3_file *pFile, ପୂର୍ଣ୍ଣସଂଖ୍ୟା eLock){
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Unlock a tmp-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tmpUnlock(sqlite3_file *pFile, ପୂର୍ଣ୍ଣସଂଖ୍ୟା eLock){
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on a tmp-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tmpCheckReservedLock(sqlite3_file *pFile, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pResOut){
  *pResOut = 0;
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** File control method. For custom operations on a tmp-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tmpFileControl(sqlite3_file *pFile, ପୂର୍ଣ୍ଣସଂଖ୍ୟା op, ଶୂନ୍ୟ *pArg){
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Return the sector-size in bytes for a tmp-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tmpSectorSize(sqlite3_file *pFile){
  ଫେରନ୍ତୁ 0;
}

/*
** Return the device characteristic flags supported by a tmp-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tmpDeviceCharacteristics(sqlite3_file *pFile){
  ଫେରନ୍ତୁ 0;
}

/*
** Close an fs-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fsClose(sqlite3_file *pFile){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;

  /* Decrement the real_file ref-count. */
  pReal->nRef--;
  ପ୍ରତିଜ୍ଞା(pReal->nRef>=0);

  /* When the ref-count reaches 0, destroy the structure */
  ଯଦି( pReal->nRef==0 ){
    *pReal->ppThis = pReal->pNext;
    ଯଦି( pReal->pNext ){
      pReal->pNext->ppThis = pReal->ppThis;
    }
    rc = pReal->pFile->pMethods->xClose(pReal->pFile);
    sqlite3_free(pReal);
  }

  ଫେରନ୍ତୁ rc;
}

/*
** Read data from an fs-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fsRead(
  sqlite3_file *pFile, 
  ଶୂନ୍ୟ *zBuf, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iAmt, 
  sqlite_int64 iOfst
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  sqlite3_file *pF = pReal->pFile;

  ଯଦି( (p->eType==DATABASE_FILE && (iAmt+iOfst)>pReal->nDatabase)
   || (p->eType==JOURNAL_FILE && (iAmt+iOfst)>pReal->nJournal)
  ){
    rc = SQLITE_IOERR_SHORT_READ;
  }ଅନ୍ୟଥା ଯଦି( p->eType==DATABASE_FILE ){
    rc = pF->pMethods->xRead(pF, zBuf, iAmt, iOfst+BLOCKSIZE);
  }ଅନ୍ୟଥା{
    /* Journal file. */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା iRem = iAmt;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା iBuf = 0;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ii = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)iOfst;
    ଯେତେବେଳେ( iRem>0 && rc==SQLITE_OK ){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା iRealOff = pReal->nBlob - BLOCKSIZE*((ii/BLOCKSIZE)+1) + ii%BLOCKSIZE;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା iRealAmt = MIN(iRem, BLOCKSIZE - (iRealOff%BLOCKSIZE));

      rc = pF->pMethods->xRead(pF, &((ଅକ୍ଷର *)zBuf)[iBuf], iRealAmt, iRealOff);
      ii += iRealAmt;
      iBuf += iRealAmt;
      iRem -= iRealAmt;
    }
  }

  ଫେରନ୍ତୁ rc;
}

/*
** Write data to an fs-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fsWrite(
  sqlite3_file *pFile, 
  ଅଚଳ ଶୂନ୍ୟ *zBuf, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iAmt, 
  sqlite_int64 iOfst
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  sqlite3_file *pF = pReal->pFile;

  ଯଦି( p->eType==DATABASE_FILE ){
    ଯଦି( (iAmt+iOfst+BLOCKSIZE)>(pReal->nBlob-pReal->nJournal) ){
      rc = SQLITE_FULL;
    }ଅନ୍ୟଥା{
      rc = pF->pMethods->xWrite(pF, zBuf, iAmt, iOfst+BLOCKSIZE);
      ଯଦି( rc==SQLITE_OK ){
        pReal->nDatabase = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)MAX(pReal->nDatabase, iAmt+iOfst);
      }
    }
  }ଅନ୍ୟଥା{
    /* Journal file. */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା iRem = iAmt;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା iBuf = 0;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ii = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)iOfst;
    ଯେତେବେଳେ( iRem>0 && rc==SQLITE_OK ){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା iRealOff = pReal->nBlob - BLOCKSIZE*((ii/BLOCKSIZE)+1) + ii%BLOCKSIZE;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା iRealAmt = MIN(iRem, BLOCKSIZE - (iRealOff%BLOCKSIZE));

      ଯଦି( iRealOff<(pReal->nDatabase+BLOCKSIZE) ){
        rc = SQLITE_FULL;
      }ଅନ୍ୟଥା{
        rc = pF->pMethods->xWrite(pF, &((ଅକ୍ଷର *)zBuf)[iBuf], iRealAmt,iRealOff);
        ii += iRealAmt;
        iBuf += iRealAmt;
        iRem -= iRealAmt;
      }
    }
    ଯଦି( rc==SQLITE_OK ){
      pReal->nJournal = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)MAX(pReal->nJournal, iAmt+iOfst);
    }
  }

  ଫେରନ୍ତୁ rc;
}

/*
** Truncate an fs-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fsTruncate(sqlite3_file *pFile, sqlite_int64 size){
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  ଯଦି( p->eType==DATABASE_FILE ){
    pReal->nDatabase = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)MIN(pReal->nDatabase, size);
  }ଅନ୍ୟଥା{
    pReal->nJournal = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)MIN(pReal->nJournal, size);
  }
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Sync an fs-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fsSync(sqlite3_file *pFile, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags){
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  sqlite3_file *pRealFile = pReal->pFile;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;

  ଯଦି( p->eType==DATABASE_FILE ){
    unsigned ଅକ୍ଷର zSize[4];
    zSize[0] = (pReal->nDatabase&0xFF000000)>>24;
    zSize[1] = (unsigned ଅକ୍ଷର)((pReal->nDatabase&0x00FF0000)>>16);
    zSize[2] = (pReal->nDatabase&0x0000FF00)>>8;
    zSize[3] = (pReal->nDatabase&0x000000FF);
    rc = pRealFile->pMethods->xWrite(pRealFile, zSize, 4, 0);
  }
  ଯଦି( rc==SQLITE_OK ){
    rc = pRealFile->pMethods->xSync(pRealFile, flags&(~SQLITE_SYNC_DATAONLY));
  }

  ଫେରନ୍ତୁ rc;
}

/*
** Return the current file-size of an fs-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fsFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  ଯଦି( p->eType==DATABASE_FILE ){
    *pSize = pReal->nDatabase;
  }ଅନ୍ୟଥା{
    *pSize = pReal->nJournal;
  }
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Lock an fs-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fsLock(sqlite3_file *pFile, ପୂର୍ଣ୍ଣସଂଖ୍ୟା eLock){
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Unlock an fs-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fsUnlock(sqlite3_file *pFile, ପୂର୍ଣ୍ଣସଂଖ୍ୟା eLock){
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on an fs-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fsCheckReservedLock(sqlite3_file *pFile, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pResOut){
  *pResOut = 0;
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** File control method. For custom operations on an fs-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fsFileControl(sqlite3_file *pFile, ପୂର୍ଣ୍ଣସଂଖ୍ୟା op, ଶୂନ୍ୟ *pArg){
  ଯଦି( op==SQLITE_FCNTL_PRAGMA ) ଫେରନ୍ତୁ SQLITE_NOTFOUND;
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Return the sector-size in bytes for an fs-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fsSectorSize(sqlite3_file *pFile){
  ଫେରନ୍ତୁ BLOCKSIZE;
}

/*
** Return the device characteristic flags supported by an fs-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fsDeviceCharacteristics(sqlite3_file *pFile){
  ଫେରନ୍ତୁ 0;
}

/*
** Open an fs file handle.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fsOpen(
  sqlite3_vfs *pVfs,
  ଅଚଳ ଅକ୍ଷର *zName,
  sqlite3_file *pFile,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pOutFlags
){
  fs_vfs_t *pFsVfs = (fs_vfs_t *)pVfs;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା eType;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nName;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;

  ଯଦି( 0==(flags&(SQLITE_OPEN_MAIN_DB|SQLITE_OPEN_MAIN_JOURNAL)) ){
    tmp_file *p2 = (tmp_file *)pFile;
    memset(p2, 0, ଆକାର(*p2));
    p2->base.pMethods = &tmp_io_methods;
    ଫେରନ୍ତୁ SQLITE_OK;
  }

  eType = ((flags&(SQLITE_OPEN_MAIN_DB))?DATABASE_FILE:JOURNAL_FILE);
  p->base.pMethods = &fs_io_methods;
  p->eType = eType;

  ପ୍ରତିଜ୍ଞା(strlen("-journal")==8);
  nName = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)strlen(zName)-((eType==JOURNAL_FILE)?8:0);
  pReal=pFsVfs->pFileList; 
  ପାଇଁ(; pReal && strncmp(pReal->zName, zName, nName); pReal=pReal->pNext);

  ଯଦି( !pReal ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା real_flags = (flags&~(SQLITE_OPEN_MAIN_DB))|SQLITE_OPEN_TEMP_DB;
    sqlite3_int64 size;
    sqlite3_file *pRealFile;
    sqlite3_vfs *pParent = pFsVfs->pParent;
    ପ୍ରତିଜ୍ଞା(eType==DATABASE_FILE);

    pReal = (fs_real_file *)sqlite3_malloc(ଆକାର(*pReal)+pParent->szOsFile);
    ଯଦି( !pReal ){
      rc = SQLITE_NOMEM;
      ଯାଅ open_out;
    }
    memset(pReal, 0, ଆକାର(*pReal)+pParent->szOsFile);
    pReal->zName = zName;
    pReal->pFile = (sqlite3_file *)(&pReal[1]);

    rc = pParent->xOpen(pParent, zName, pReal->pFile, real_flags, pOutFlags);
    ଯଦି( rc!=SQLITE_OK ){
      ଯାଅ open_out;
    }
    pRealFile = pReal->pFile;

    rc = pRealFile->pMethods->xFileSize(pRealFile, &size);
    ଯଦି( rc!=SQLITE_OK ){
      ଯାଅ open_out;
    }
    ଯଦି( size==0 ){
      rc = pRealFile->pMethods->xWrite(pRealFile, "\0", 1, BLOBSIZE-1);
      pReal->nBlob = BLOBSIZE;
    }ଅନ୍ୟଥା{
      unsigned ଅକ୍ଷର zS[4];
      pReal->nBlob = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)size;
      rc = pRealFile->pMethods->xRead(pRealFile, zS, 4, 0);
      pReal->nDatabase = (zS[0]<<24)+(zS[1]<<16)+(zS[2]<<8)+zS[3];
      ଯଦି( rc==SQLITE_OK ){
        rc = pRealFile->pMethods->xRead(pRealFile, zS, 4, pReal->nBlob-4);
        ଯଦି( zS[0] || zS[1] || zS[2] || zS[3] ){
          pReal->nJournal = pReal->nBlob;
        }
      }
    }

    ଯଦି( rc==SQLITE_OK ){
      pReal->pNext = pFsVfs->pFileList;
      ଯଦି( pReal->pNext ){
        pReal->pNext->ppThis = &pReal->pNext;
      }
      pReal->ppThis = &pFsVfs->pFileList;
      pFsVfs->pFileList = pReal;
    }
  }

open_out:
  ଯଦି( pReal ){
    ଯଦି( rc==SQLITE_OK ){
      p->pReal = pReal;
      pReal->nRef++;
    }ଅନ୍ୟଥା{
      ଯଦି( pReal->pFile->pMethods ){
        pReal->pFile->pMethods->xClose(pReal->pFile);
      }
      sqlite3_free(pReal);
    }
  }
  ଫେରନ୍ତୁ rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fsDelete(sqlite3_vfs *pVfs, ଅଚଳ ଅକ୍ଷର *zPath, ପୂର୍ଣ୍ଣସଂଖ୍ୟା dirSync){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
  fs_vfs_t *pFsVfs = (fs_vfs_t *)pVfs;
  fs_real_file *pReal;
  sqlite3_file *pF;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nName = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)strlen(zPath) - 8;

  ପ୍ରତିଜ୍ଞା(strlen("-journal")==8);
  ପ୍ରତିଜ୍ଞା(strcmp("-journal", &zPath[nName])==0);

  pReal = pFsVfs->pFileList; 
  ପାଇଁ(; pReal && strncmp(pReal->zName, zPath, nName); pReal=pReal->pNext);
  ଯଦି( pReal ){
    pF = pReal->pFile;
    rc = pF->pMethods->xWrite(pF, "\0\0\0\0", 4, pReal->nBlob-BLOCKSIZE);
    ଯଦି( rc==SQLITE_OK ){
      pReal->nJournal = 0;
    }
  }
  ଫେରନ୍ତୁ rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fsAccess(
  sqlite3_vfs *pVfs, 
  ଅଚଳ ଅକ୍ଷର *zPath, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pResOut
){
  fs_vfs_t *pFsVfs = (fs_vfs_t *)pVfs;
  fs_real_file *pReal;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା isJournal = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nName = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)strlen(zPath);

  ଯଦି( flags!=SQLITE_ACCESS_EXISTS ){
    sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
    ଫେରନ୍ତୁ pParent->xAccess(pParent, zPath, flags, pResOut);
  }

  ପ୍ରତିଜ୍ଞା(strlen("-journal")==8);
  ଯଦି( nName>8 && strcmp("-journal", &zPath[nName-8])==0 ){
    nName -= 8;
    isJournal = 1;
  }

  pReal = pFsVfs->pFileList; 
  ପାଇଁ(; pReal && strncmp(pReal->zName, zPath, nName); pReal=pReal->pNext);

  *pResOut = (pReal && (!isJournal || pReal->nJournal>0));
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (FS_MAX_PATHNAME+1) bytes.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fsFullPathname(
  sqlite3_vfs *pVfs,            /* Pointer to vfs object */
  ଅଚଳ ଅକ୍ଷର *zPath,            /* Possibly relative input path */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nOut,                     /* Size of output buffer in bytes */
  ଅକ୍ଷର *zOut                    /* Output buffer */
){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  ଫେରନ୍ତୁ pParent->xFullPathname(pParent, zPath, nOut, zOut);
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
ସ୍ଥିର ଶୂନ୍ୟ *fsDlOpen(sqlite3_vfs *pVfs, ଅଚଳ ଅକ୍ଷର *zPath){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  ଫେରନ୍ତୁ pParent->xDlOpen(pParent, zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
ସ୍ଥିର ଶୂନ୍ୟ fsDlError(sqlite3_vfs *pVfs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nByte, ଅକ୍ଷର *z_Err_ସନ୍ଦେଶ){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  pParent->xDlError(pParent, nByte, z_Err_ସନ୍ଦେଶ);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
ସ୍ଥିର ଶୂନ୍ୟ (*fsDlSym(sqlite3_vfs *pVfs, ଶୂନ୍ୟ *pH, ଅଚଳ ଅକ୍ଷର *zSym))(ଶୂନ୍ୟ){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  ଫେରନ୍ତୁ pParent->xDlSym(pParent, pH, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
ସ୍ଥିର ଶୂନ୍ୟ fsDlClose(sqlite3_vfs *pVfs, ଶୂନ୍ୟ *pHandle){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  pParent->xDlClose(pParent, pHandle);
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fsRandomness(sqlite3_vfs *pVfs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nByte, ଅକ୍ଷର *zBufOut){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  ଫେରନ୍ତୁ pParent->xRandomness(pParent, nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fsSleep(sqlite3_vfs *pVfs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nMicro){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  ଫେରନ୍ତୁ pParent->xSleep(pParent, nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fsCurrentTime(sqlite3_vfs *pVfs, ଦ୍ୱିଗୁଣ *pTimeOut){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  ଫେରନ୍ତୁ pParent->xCurrentTime(pParent, pTimeOut);
}

/*
** This procedure registers the fs vfs with SQLite. If the argument is
** true, the fs vfs becomes the new default vfs. It is the only publicly
** available function in this file.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା fs_register(ଶୂନ୍ୟ){
  ଯଦି( fs_vfs.pParent ) ଫେରନ୍ତୁ SQLITE_OK;
  fs_vfs.pParent = sqlite3_vfs_find(0);
  fs_vfs.base.mxPathname = fs_vfs.pParent->mxPathname;
  fs_vfs.base.szOsFile = MAX(ଆକାର(tmp_file), ଆକାର(fs_file));
  ଫେରନ୍ତୁ sqlite3_vfs_register(&fs_vfs.base, 0);
}

#ifdef SQLITE_TEST
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା SqlitetestOnefile_Init() {ଫେରନ୍ତୁ fs_register();}
#endif
