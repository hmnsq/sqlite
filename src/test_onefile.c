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

#অন্তর্ভাৱ "sqlite3.h"
#অন্তর্ভাৱ <assert.h>
#অন্তর্ভাৱ <string.h>

/*
** Maximum pathname length supported by the fs backend.
*/
#define BLOCKSIZE 512
#define BLOBSIZE 10485760

/*
** Name used to identify this VFS.
*/
#define FS_VFS_NAME "fs"

প্রকারনির্দেশ সংরচনা fs_real_file fs_real_file;
সংরচনা fs_real_file {
  sqlite3_file *pFile;
  ধ্রুৱ ৱর্ণ *zName;
  পূর্ণাঙ্ক nDatabase;              /* Current size of database region */
  পূর্ণাঙ্ক nJournal;               /* Current size of journal region */
  পূর্ণাঙ্ক nBlob;                  /* Total size of allocated blob */
  পূর্ণাঙ্ক nRef;                   /* Number of pointers to this structure */
  fs_real_file *pNext;
  fs_real_file **ppThis;
};

প্রকারনির্দেশ সংরচনা fs_file fs_file;
সংরচনা fs_file {
  sqlite3_file base;
  পূর্ণাঙ্ক eType;
  fs_real_file *pReal;
};

প্রকারনির্দেশ সংরচনা tmp_file tmp_file;
সংরচনা tmp_file {
  sqlite3_file base;
  পূর্ণাঙ্ক nSize;
  পূর্ণাঙ্ক nAlloc;
  ৱর্ণ *zAlloc;
};

/* Values for fs_file.eType. */
#define DATABASE_FILE   1
#define JOURNAL_FILE    2

/*
** Method declarations for fs_file.
*/
স্থির পূর্ণাঙ্ক fsClose(sqlite3_file*);
স্থির পূর্ণাঙ্ক fsRead(sqlite3_file*, শূন্য*, পূর্ণাঙ্ক iAmt, sqlite3_int64 iOfst);
স্থির পূর্ণাঙ্ক fsWrite(sqlite3_file*, ধ্রুৱ শূন্য*, পূর্ণাঙ্ক iAmt, sqlite3_int64 iOfst);
স্থির পূর্ণাঙ্ক fsTruncate(sqlite3_file*, sqlite3_int64 size);
স্থির পূর্ণাঙ্ক fsSync(sqlite3_file*, পূর্ণাঙ্ক flags);
স্থির পূর্ণাঙ্ক fsFileSize(sqlite3_file*, sqlite3_int64 *pSize);
স্থির পূর্ণাঙ্ক fsLock(sqlite3_file*, পূর্ণাঙ্ক);
স্থির পূর্ণাঙ্ক fsUnlock(sqlite3_file*, পূর্ণাঙ্ক);
স্থির পূর্ণাঙ্ক fsCheckReservedLock(sqlite3_file*, পূর্ণাঙ্ক *pResOut);
স্থির পূর্ণাঙ্ক fsFileControl(sqlite3_file*, পূর্ণাঙ্ক op, শূন্য *pArg);
স্থির পূর্ণাঙ্ক fsSectorSize(sqlite3_file*);
স্থির পূর্ণাঙ্ক fsDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for tmp_file.
*/
স্থির পূর্ণাঙ্ক tmpClose(sqlite3_file*);
স্থির পূর্ণাঙ্ক tmpRead(sqlite3_file*, শূন্য*, পূর্ণাঙ্ক iAmt, sqlite3_int64 iOfst);
স্থির পূর্ণাঙ্ক tmpWrite(sqlite3_file*, ধ্রুৱ শূন্য*, পূর্ণাঙ্ক iAmt, sqlite3_int64 iOfst);
স্থির পূর্ণাঙ্ক tmpTruncate(sqlite3_file*, sqlite3_int64 size);
স্থির পূর্ণাঙ্ক tmpSync(sqlite3_file*, পূর্ণাঙ্ক flags);
স্থির পূর্ণাঙ্ক tmpFileSize(sqlite3_file*, sqlite3_int64 *pSize);
স্থির পূর্ণাঙ্ক tmpLock(sqlite3_file*, পূর্ণাঙ্ক);
স্থির পূর্ণাঙ্ক tmpUnlock(sqlite3_file*, পূর্ণাঙ্ক);
স্থির পূর্ণাঙ্ক tmpCheckReservedLock(sqlite3_file*, পূর্ণাঙ্ক *pResOut);
স্থির পূর্ণাঙ্ক tmpFileControl(sqlite3_file*, পূর্ণাঙ্ক op, শূন্য *pArg);
স্থির পূর্ণাঙ্ক tmpSectorSize(sqlite3_file*);
স্থির পূর্ণাঙ্ক tmpDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for fs_vfs.
*/
স্থির পূর্ণাঙ্ক fsOpen(sqlite3_vfs*, ধ্রুৱ ৱর্ণ *, sqlite3_file*, পূর্ণাঙ্ক , পূর্ণাঙ্ক *);
স্থির পূর্ণাঙ্ক fsDelete(sqlite3_vfs*, ধ্রুৱ ৱর্ণ *zName, পূর্ণাঙ্ক syncDir);
স্থির পূর্ণাঙ্ক fsAccess(sqlite3_vfs*, ধ্রুৱ ৱর্ণ *zName, পূর্ণাঙ্ক flags, পূর্ণাঙ্ক *);
স্থির পূর্ণাঙ্ক fsFullPathname(sqlite3_vfs*, ধ্রুৱ ৱর্ণ *zName, পূর্ণাঙ্ক nOut,ৱর্ণ *zOut);
স্থির শূন্য *fsDlOpen(sqlite3_vfs*, ধ্রুৱ ৱর্ণ *zFilename);
স্থির শূন্য fsDlError(sqlite3_vfs*, পূর্ণাঙ্ক nByte, ৱর্ণ *z_Err_বাৰ্তা);
স্থির শূন্য (*fsDlSym(sqlite3_vfs*,শূন্য*, ধ্রুৱ ৱর্ণ *z_প্ৰতীক))(শূন্য);
স্থির শূন্য fsDlClose(sqlite3_vfs*, শূন্য*);
স্থির পূর্ণাঙ্ক fsRandomness(sqlite3_vfs*, পূর্ণাঙ্ক nByte, ৱর্ণ *zOut);
স্থির পূর্ণাঙ্ক fsSleep(sqlite3_vfs*, পূর্ণাঙ্ক microseconds);
স্থির পূর্ণাঙ্ক fsCurrentTime(sqlite3_vfs*, দ্ৱিগুণ*);


প্রকারনির্দেশ সংরচনা fs_vfs_t fs_vfs_t;
সংরচনা fs_vfs_t {
  sqlite3_vfs base;
  fs_real_file *pFileList;
  sqlite3_vfs *pParent;
};

স্থির fs_vfs_t fs_vfs = {
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

স্থির sqlite3_io_methods fs_io_methods = {
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


স্থির sqlite3_io_methods tmp_io_methods = {
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
স্থির পূর্ণাঙ্ক tmpClose(sqlite3_file *pFile){
  tmp_file *pTmp = (tmp_file *)pFile;
  sqlite3_free(pTmp->zAlloc);
  ঘূৰাই SQLITE_OK;
}

/*
** Read data from a tmp-file.
*/
স্থির পূর্ণাঙ্ক tmpRead(
  sqlite3_file *pFile, 
  শূন্য *zBuf, 
  পূর্ণাঙ্ক iAmt, 
  sqlite_int64 iOfst
){
  tmp_file *pTmp = (tmp_file *)pFile;
  যদি( (iAmt+iOfst)>pTmp->nSize ){
    ঘূৰাই SQLITE_IOERR_SHORT_READ;
  }
  memcpy(zBuf, &pTmp->zAlloc[iOfst], iAmt);
  ঘূৰাই SQLITE_OK;
}

/*
** Write data to a tmp-file.
*/
স্থির পূর্ণাঙ্ক tmpWrite(
  sqlite3_file *pFile, 
  ধ্রুৱ শূন্য *zBuf, 
  পূর্ণাঙ্ক iAmt, 
  sqlite_int64 iOfst
){
  tmp_file *pTmp = (tmp_file *)pFile;
  যদি( (iAmt+iOfst)>pTmp->nAlloc ){
    পূর্ণাঙ্ক nNew = (পূর্ণাঙ্ক)(2*(iAmt+iOfst+pTmp->nAlloc));
    ৱর্ণ *zNew = sqlite3_realloc(pTmp->zAlloc, nNew);
    যদি( !zNew ){
      ঘূৰাই SQLITE_NOMEM;
    }
    pTmp->zAlloc = zNew;
    pTmp->nAlloc = nNew;
  }
  memcpy(&pTmp->zAlloc[iOfst], zBuf, iAmt);
  pTmp->nSize = (পূর্ণাঙ্ক)MAX(pTmp->nSize, iOfst+iAmt);
  ঘূৰাই SQLITE_OK;
}

/*
** Truncate a tmp-file.
*/
স্থির পূর্ণাঙ্ক tmpTruncate(sqlite3_file *pFile, sqlite_int64 size){
  tmp_file *pTmp = (tmp_file *)pFile;
  pTmp->nSize = (পূর্ণাঙ্ক)MIN(pTmp->nSize, size);
  ঘূৰাই SQLITE_OK;
}

/*
** Sync a tmp-file.
*/
স্থির পূর্ণাঙ্ক tmpSync(sqlite3_file *pFile, পূর্ণাঙ্ক flags){
  ঘূৰাই SQLITE_OK;
}

/*
** Return the current file-size of a tmp-file.
*/
স্থির পূর্ণাঙ্ক tmpFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  tmp_file *pTmp = (tmp_file *)pFile;
  *pSize = pTmp->nSize;
  ঘূৰাই SQLITE_OK;
}

/*
** Lock a tmp-file.
*/
স্থির পূর্ণাঙ্ক tmpLock(sqlite3_file *pFile, পূর্ণাঙ্ক eLock){
  ঘূৰাই SQLITE_OK;
}

/*
** Unlock a tmp-file.
*/
স্থির পূর্ণাঙ্ক tmpUnlock(sqlite3_file *pFile, পূর্ণাঙ্ক eLock){
  ঘূৰাই SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on a tmp-file.
*/
স্থির পূর্ণাঙ্ক tmpCheckReservedLock(sqlite3_file *pFile, পূর্ণাঙ্ক *pResOut){
  *pResOut = 0;
  ঘূৰাই SQLITE_OK;
}

/*
** File control method. For custom operations on a tmp-file.
*/
স্থির পূর্ণাঙ্ক tmpFileControl(sqlite3_file *pFile, পূর্ণাঙ্ক op, শূন্য *pArg){
  ঘূৰাই SQLITE_OK;
}

/*
** Return the sector-size in bytes for a tmp-file.
*/
স্থির পূর্ণাঙ্ক tmpSectorSize(sqlite3_file *pFile){
  ঘূৰাই 0;
}

/*
** Return the device characteristic flags supported by a tmp-file.
*/
স্থির পূর্ণাঙ্ক tmpDeviceCharacteristics(sqlite3_file *pFile){
  ঘূৰাই 0;
}

/*
** Close an fs-file.
*/
স্থির পূর্ণাঙ্ক fsClose(sqlite3_file *pFile){
  পূর্ণাঙ্ক rc = SQLITE_OK;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;

  /* Decrement the real_file ref-count. */
  pReal->nRef--;
  প্রতিজ্ঞা(pReal->nRef>=0);

  /* When the ref-count reaches 0, destroy the structure */
  যদি( pReal->nRef==0 ){
    *pReal->ppThis = pReal->pNext;
    যদি( pReal->pNext ){
      pReal->pNext->ppThis = pReal->ppThis;
    }
    rc = pReal->pFile->pMethods->xClose(pReal->pFile);
    sqlite3_free(pReal);
  }

  ঘূৰাই rc;
}

/*
** Read data from an fs-file.
*/
স্থির পূর্ণাঙ্ক fsRead(
  sqlite3_file *pFile, 
  শূন্য *zBuf, 
  পূর্ণাঙ্ক iAmt, 
  sqlite_int64 iOfst
){
  পূর্ণাঙ্ক rc = SQLITE_OK;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  sqlite3_file *pF = pReal->pFile;

  যদি( (p->eType==DATABASE_FILE && (iAmt+iOfst)>pReal->nDatabase)
   || (p->eType==JOURNAL_FILE && (iAmt+iOfst)>pReal->nJournal)
  ){
    rc = SQLITE_IOERR_SHORT_READ;
  }অন্যথা যদি( p->eType==DATABASE_FILE ){
    rc = pF->pMethods->xRead(pF, zBuf, iAmt, iOfst+BLOCKSIZE);
  }অন্যথা{
    /* Journal file. */
    পূর্ণাঙ্ক iRem = iAmt;
    পূর্ণাঙ্ক iBuf = 0;
    পূর্ণাঙ্ক ii = (পূর্ণাঙ্ক)iOfst;
    যেতিয়ালৈ( iRem>0 && rc==SQLITE_OK ){
      পূর্ণাঙ্ক iRealOff = pReal->nBlob - BLOCKSIZE*((ii/BLOCKSIZE)+1) + ii%BLOCKSIZE;
      পূর্ণাঙ্ক iRealAmt = MIN(iRem, BLOCKSIZE - (iRealOff%BLOCKSIZE));

      rc = pF->pMethods->xRead(pF, &((ৱর্ণ *)zBuf)[iBuf], iRealAmt, iRealOff);
      ii += iRealAmt;
      iBuf += iRealAmt;
      iRem -= iRealAmt;
    }
  }

  ঘূৰাই rc;
}

/*
** Write data to an fs-file.
*/
স্থির পূর্ণাঙ্ক fsWrite(
  sqlite3_file *pFile, 
  ধ্রুৱ শূন্য *zBuf, 
  পূর্ণাঙ্ক iAmt, 
  sqlite_int64 iOfst
){
  পূর্ণাঙ্ক rc = SQLITE_OK;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  sqlite3_file *pF = pReal->pFile;

  যদি( p->eType==DATABASE_FILE ){
    যদি( (iAmt+iOfst+BLOCKSIZE)>(pReal->nBlob-pReal->nJournal) ){
      rc = SQLITE_FULL;
    }অন্যথা{
      rc = pF->pMethods->xWrite(pF, zBuf, iAmt, iOfst+BLOCKSIZE);
      যদি( rc==SQLITE_OK ){
        pReal->nDatabase = (পূর্ণাঙ্ক)MAX(pReal->nDatabase, iAmt+iOfst);
      }
    }
  }অন্যথা{
    /* Journal file. */
    পূর্ণাঙ্ক iRem = iAmt;
    পূর্ণাঙ্ক iBuf = 0;
    পূর্ণাঙ্ক ii = (পূর্ণাঙ্ক)iOfst;
    যেতিয়ালৈ( iRem>0 && rc==SQLITE_OK ){
      পূর্ণাঙ্ক iRealOff = pReal->nBlob - BLOCKSIZE*((ii/BLOCKSIZE)+1) + ii%BLOCKSIZE;
      পূর্ণাঙ্ক iRealAmt = MIN(iRem, BLOCKSIZE - (iRealOff%BLOCKSIZE));

      যদি( iRealOff<(pReal->nDatabase+BLOCKSIZE) ){
        rc = SQLITE_FULL;
      }অন্যথা{
        rc = pF->pMethods->xWrite(pF, &((ৱর্ণ *)zBuf)[iBuf], iRealAmt,iRealOff);
        ii += iRealAmt;
        iBuf += iRealAmt;
        iRem -= iRealAmt;
      }
    }
    যদি( rc==SQLITE_OK ){
      pReal->nJournal = (পূর্ণাঙ্ক)MAX(pReal->nJournal, iAmt+iOfst);
    }
  }

  ঘূৰাই rc;
}

/*
** Truncate an fs-file.
*/
স্থির পূর্ণাঙ্ক fsTruncate(sqlite3_file *pFile, sqlite_int64 size){
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  যদি( p->eType==DATABASE_FILE ){
    pReal->nDatabase = (পূর্ণাঙ্ক)MIN(pReal->nDatabase, size);
  }অন্যথা{
    pReal->nJournal = (পূর্ণাঙ্ক)MIN(pReal->nJournal, size);
  }
  ঘূৰাই SQLITE_OK;
}

/*
** Sync an fs-file.
*/
স্থির পূর্ণাঙ্ক fsSync(sqlite3_file *pFile, পূর্ণাঙ্ক flags){
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  sqlite3_file *pRealFile = pReal->pFile;
  পূর্ণাঙ্ক rc = SQLITE_OK;

  যদি( p->eType==DATABASE_FILE ){
    unsigned ৱর্ণ zSize[4];
    zSize[0] = (pReal->nDatabase&0xFF000000)>>24;
    zSize[1] = (unsigned ৱর্ণ)((pReal->nDatabase&0x00FF0000)>>16);
    zSize[2] = (pReal->nDatabase&0x0000FF00)>>8;
    zSize[3] = (pReal->nDatabase&0x000000FF);
    rc = pRealFile->pMethods->xWrite(pRealFile, zSize, 4, 0);
  }
  যদি( rc==SQLITE_OK ){
    rc = pRealFile->pMethods->xSync(pRealFile, flags&(~SQLITE_SYNC_DATAONLY));
  }

  ঘূৰাই rc;
}

/*
** Return the current file-size of an fs-file.
*/
স্থির পূর্ণাঙ্ক fsFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  যদি( p->eType==DATABASE_FILE ){
    *pSize = pReal->nDatabase;
  }অন্যথা{
    *pSize = pReal->nJournal;
  }
  ঘূৰাই SQLITE_OK;
}

/*
** Lock an fs-file.
*/
স্থির পূর্ণাঙ্ক fsLock(sqlite3_file *pFile, পূর্ণাঙ্ক eLock){
  ঘূৰাই SQLITE_OK;
}

/*
** Unlock an fs-file.
*/
স্থির পূর্ণাঙ্ক fsUnlock(sqlite3_file *pFile, পূর্ণাঙ্ক eLock){
  ঘূৰাই SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on an fs-file.
*/
স্থির পূর্ণাঙ্ক fsCheckReservedLock(sqlite3_file *pFile, পূর্ণাঙ্ক *pResOut){
  *pResOut = 0;
  ঘূৰাই SQLITE_OK;
}

/*
** File control method. For custom operations on an fs-file.
*/
স্থির পূর্ণাঙ্ক fsFileControl(sqlite3_file *pFile, পূর্ণাঙ্ক op, শূন্য *pArg){
  যদি( op==SQLITE_FCNTL_PRAGMA ) ঘূৰাই SQLITE_NOTFOUND;
  ঘূৰাই SQLITE_OK;
}

/*
** Return the sector-size in bytes for an fs-file.
*/
স্থির পূর্ণাঙ্ক fsSectorSize(sqlite3_file *pFile){
  ঘূৰাই BLOCKSIZE;
}

/*
** Return the device characteristic flags supported by an fs-file.
*/
স্থির পূর্ণাঙ্ক fsDeviceCharacteristics(sqlite3_file *pFile){
  ঘূৰাই 0;
}

/*
** Open an fs file handle.
*/
স্থির পূর্ণাঙ্ক fsOpen(
  sqlite3_vfs *pVfs,
  ধ্রুৱ ৱর্ণ *zName,
  sqlite3_file *pFile,
  পূর্ণাঙ্ক flags,
  পূর্ণাঙ্ক *pOutFlags
){
  fs_vfs_t *pFsVfs = (fs_vfs_t *)pVfs;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = 0;
  পূর্ণাঙ্ক eType;
  পূর্ণাঙ্ক nName;
  পূর্ণাঙ্ক rc = SQLITE_OK;

  যদি( 0==(flags&(SQLITE_OPEN_MAIN_DB|SQLITE_OPEN_MAIN_JOURNAL)) ){
    tmp_file *p2 = (tmp_file *)pFile;
    memset(p2, 0, পরিমাণ(*p2));
    p2->base.pMethods = &tmp_io_methods;
    ঘূৰাই SQLITE_OK;
  }

  eType = ((flags&(SQLITE_OPEN_MAIN_DB))?DATABASE_FILE:JOURNAL_FILE);
  p->base.pMethods = &fs_io_methods;
  p->eType = eType;

  প্রতিজ্ঞা(strlen("-journal")==8);
  nName = (পূর্ণাঙ্ক)strlen(zName)-((eType==JOURNAL_FILE)?8:0);
  pReal=pFsVfs->pFileList; 
  বাবে(; pReal && strncmp(pReal->zName, zName, nName); pReal=pReal->pNext);

  যদি( !pReal ){
    পূর্ণাঙ্ক real_flags = (flags&~(SQLITE_OPEN_MAIN_DB))|SQLITE_OPEN_TEMP_DB;
    sqlite3_int64 size;
    sqlite3_file *pRealFile;
    sqlite3_vfs *pParent = pFsVfs->pParent;
    প্রতিজ্ঞা(eType==DATABASE_FILE);

    pReal = (fs_real_file *)sqlite3_malloc(পরিমাণ(*pReal)+pParent->szOsFile);
    যদি( !pReal ){
      rc = SQLITE_NOMEM;
      গচ্ছ open_out;
    }
    memset(pReal, 0, পরিমাণ(*pReal)+pParent->szOsFile);
    pReal->zName = zName;
    pReal->pFile = (sqlite3_file *)(&pReal[1]);

    rc = pParent->xOpen(pParent, zName, pReal->pFile, real_flags, pOutFlags);
    যদি( rc!=SQLITE_OK ){
      গচ্ছ open_out;
    }
    pRealFile = pReal->pFile;

    rc = pRealFile->pMethods->xFileSize(pRealFile, &size);
    যদি( rc!=SQLITE_OK ){
      গচ্ছ open_out;
    }
    যদি( size==0 ){
      rc = pRealFile->pMethods->xWrite(pRealFile, "\0", 1, BLOBSIZE-1);
      pReal->nBlob = BLOBSIZE;
    }অন্যথা{
      unsigned ৱর্ণ zS[4];
      pReal->nBlob = (পূর্ণাঙ্ক)size;
      rc = pRealFile->pMethods->xRead(pRealFile, zS, 4, 0);
      pReal->nDatabase = (zS[0]<<24)+(zS[1]<<16)+(zS[2]<<8)+zS[3];
      যদি( rc==SQLITE_OK ){
        rc = pRealFile->pMethods->xRead(pRealFile, zS, 4, pReal->nBlob-4);
        যদি( zS[0] || zS[1] || zS[2] || zS[3] ){
          pReal->nJournal = pReal->nBlob;
        }
      }
    }

    যদি( rc==SQLITE_OK ){
      pReal->pNext = pFsVfs->pFileList;
      যদি( pReal->pNext ){
        pReal->pNext->ppThis = &pReal->pNext;
      }
      pReal->ppThis = &pFsVfs->pFileList;
      pFsVfs->pFileList = pReal;
    }
  }

open_out:
  যদি( pReal ){
    যদি( rc==SQLITE_OK ){
      p->pReal = pReal;
      pReal->nRef++;
    }অন্যথা{
      যদি( pReal->pFile->pMethods ){
        pReal->pFile->pMethods->xClose(pReal->pFile);
      }
      sqlite3_free(pReal);
    }
  }
  ঘূৰাই rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
স্থির পূর্ণাঙ্ক fsDelete(sqlite3_vfs *pVfs, ধ্রুৱ ৱর্ণ *zPath, পূর্ণাঙ্ক dirSync){
  পূর্ণাঙ্ক rc = SQLITE_OK;
  fs_vfs_t *pFsVfs = (fs_vfs_t *)pVfs;
  fs_real_file *pReal;
  sqlite3_file *pF;
  পূর্ণাঙ্ক nName = (পূর্ণাঙ্ক)strlen(zPath) - 8;

  প্রতিজ্ঞা(strlen("-journal")==8);
  প্রতিজ্ঞা(strcmp("-journal", &zPath[nName])==0);

  pReal = pFsVfs->pFileList; 
  বাবে(; pReal && strncmp(pReal->zName, zPath, nName); pReal=pReal->pNext);
  যদি( pReal ){
    pF = pReal->pFile;
    rc = pF->pMethods->xWrite(pF, "\0\0\0\0", 4, pReal->nBlob-BLOCKSIZE);
    যদি( rc==SQLITE_OK ){
      pReal->nJournal = 0;
    }
  }
  ঘূৰাই rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
স্থির পূর্ণাঙ্ক fsAccess(
  sqlite3_vfs *pVfs, 
  ধ্রুৱ ৱর্ণ *zPath, 
  পূর্ণাঙ্ক flags, 
  পূর্ণাঙ্ক *pResOut
){
  fs_vfs_t *pFsVfs = (fs_vfs_t *)pVfs;
  fs_real_file *pReal;
  পূর্ণাঙ্ক isJournal = 0;
  পূর্ণাঙ্ক nName = (পূর্ণাঙ্ক)strlen(zPath);

  যদি( flags!=SQLITE_ACCESS_EXISTS ){
    sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
    ঘূৰাই pParent->xAccess(pParent, zPath, flags, pResOut);
  }

  প্রতিজ্ঞা(strlen("-journal")==8);
  যদি( nName>8 && strcmp("-journal", &zPath[nName-8])==0 ){
    nName -= 8;
    isJournal = 1;
  }

  pReal = pFsVfs->pFileList; 
  বাবে(; pReal && strncmp(pReal->zName, zPath, nName); pReal=pReal->pNext);

  *pResOut = (pReal && (!isJournal || pReal->nJournal>0));
  ঘূৰাই SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (FS_MAX_PATHNAME+1) bytes.
*/
স্থির পূর্ণাঙ্ক fsFullPathname(
  sqlite3_vfs *pVfs,            /* Pointer to vfs object */
  ধ্রুৱ ৱর্ণ *zPath,            /* Possibly relative input path */
  পূর্ণাঙ্ক nOut,                     /* Size of output buffer in bytes */
  ৱর্ণ *zOut                    /* Output buffer */
){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  ঘূৰাই pParent->xFullPathname(pParent, zPath, nOut, zOut);
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
স্থির শূন্য *fsDlOpen(sqlite3_vfs *pVfs, ধ্রুৱ ৱর্ণ *zPath){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  ঘূৰাই pParent->xDlOpen(pParent, zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
স্থির শূন্য fsDlError(sqlite3_vfs *pVfs, পূর্ণাঙ্ক nByte, ৱর্ণ *z_Err_বাৰ্তা){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  pParent->xDlError(pParent, nByte, z_Err_বাৰ্তা);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
স্থির শূন্য (*fsDlSym(sqlite3_vfs *pVfs, শূন্য *pH, ধ্রুৱ ৱর্ণ *zSym))(শূন্য){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  ঘূৰাই pParent->xDlSym(pParent, pH, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
স্থির শূন্য fsDlClose(sqlite3_vfs *pVfs, শূন্য *pHandle){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  pParent->xDlClose(pParent, pHandle);
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
স্থির পূর্ণাঙ্ক fsRandomness(sqlite3_vfs *pVfs, পূর্ণাঙ্ক nByte, ৱর্ণ *zBufOut){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  ঘূৰাই pParent->xRandomness(pParent, nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
স্থির পূর্ণাঙ্ক fsSleep(sqlite3_vfs *pVfs, পূর্ণাঙ্ক nMicro){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  ঘূৰাই pParent->xSleep(pParent, nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
স্থির পূর্ণাঙ্ক fsCurrentTime(sqlite3_vfs *pVfs, দ্ৱিগুণ *pTimeOut){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  ঘূৰাই pParent->xCurrentTime(pParent, pTimeOut);
}

/*
** This procedure registers the fs vfs with SQLite. If the argument is
** true, the fs vfs becomes the new default vfs. It is the only publicly
** available function in this file.
*/
পূর্ণাঙ্ক fs_register(শূন্য){
  যদি( fs_vfs.pParent ) ঘূৰাই SQLITE_OK;
  fs_vfs.pParent = sqlite3_vfs_find(0);
  fs_vfs.base.mxPathname = fs_vfs.pParent->mxPathname;
  fs_vfs.base.szOsFile = MAX(পরিমাণ(tmp_file), পরিমাণ(fs_file));
  ঘূৰাই sqlite3_vfs_register(&fs_vfs.base, 0);
}

#ifdef SQLITE_TEST
  পূর্ণাঙ্ক SqlitetestOnefile_Init() {ঘূৰাই fs_register();}
#endif
