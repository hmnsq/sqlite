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

#અન્તર્ભાવ "sqlite3.h"
#અન્તર્ભાવ <assert.h>
#અન્તર્ભાવ <string.h>

/*
** Maximum pathname length supported by the fs backend.
*/
#define BLOCKSIZE 512
#define BLOBSIZE 10485760

/*
** Name used to identify this VFS.
*/
#define FS_VFS_NAME "fs"

પ્રકારવ્યાખ્યા સંરચના fs_real_file fs_real_file;
સંરચના fs_real_file {
  sqlite3_file *pFile;
  અચલ અક્ષર *zName;
  પૂર્ણાંક nDatabase;              /* Current size of database region */
  પૂર્ણાંક nJournal;               /* Current size of journal region */
  પૂર્ણાંક nBlob;                  /* Total size of allocated blob */
  પૂર્ણાંક nRef;                   /* Number of pointers to this structure */
  fs_real_file *pNext;
  fs_real_file **ppThis;
};

પ્રકારવ્યાખ્યા સંરચના fs_file fs_file;
સંરચના fs_file {
  sqlite3_file base;
  પૂર્ણાંક eType;
  fs_real_file *pReal;
};

પ્રકારવ્યાખ્યા સંરચના tmp_file tmp_file;
સંરચના tmp_file {
  sqlite3_file base;
  પૂર્ણાંક nSize;
  પૂર્ણાંક nAlloc;
  અક્ષર *zAlloc;
};

/* Values for fs_file.eType. */
#define DATABASE_FILE   1
#define JOURNAL_FILE    2

/*
** Method declarations for fs_file.
*/
સ્થિર પૂર્ણાંક fsClose(sqlite3_file*);
સ્થિર પૂર્ણાંક fsRead(sqlite3_file*, શૂન્ય*, પૂર્ણાંક iAmt, sqlite3_int64 iOfst);
સ્થિર પૂર્ણાંક fsWrite(sqlite3_file*, અચલ શૂન્ય*, પૂર્ણાંક iAmt, sqlite3_int64 iOfst);
સ્થિર પૂર્ણાંક fsTruncate(sqlite3_file*, sqlite3_int64 size);
સ્થિર પૂર્ણાંક fsSync(sqlite3_file*, પૂર્ણાંક flags);
સ્થિર પૂર્ણાંક fsFileSize(sqlite3_file*, sqlite3_int64 *pSize);
સ્થિર પૂર્ણાંક fsLock(sqlite3_file*, પૂર્ણાંક);
સ્થિર પૂર્ણાંક fsUnlock(sqlite3_file*, પૂર્ણાંક);
સ્થિર પૂર્ણાંક fsCheckReservedLock(sqlite3_file*, પૂર્ણાંક *pResOut);
સ્થિર પૂર્ણાંક fsFileControl(sqlite3_file*, પૂર્ણાંક op, શૂન્ય *pArg);
સ્થિર પૂર્ણાંક fsSectorSize(sqlite3_file*);
સ્થિર પૂર્ણાંક fsDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for tmp_file.
*/
સ્થિર પૂર્ણાંક tmpClose(sqlite3_file*);
સ્થિર પૂર્ણાંક tmpRead(sqlite3_file*, શૂન્ય*, પૂર્ણાંક iAmt, sqlite3_int64 iOfst);
સ્થિર પૂર્ણાંક tmpWrite(sqlite3_file*, અચલ શૂન્ય*, પૂર્ણાંક iAmt, sqlite3_int64 iOfst);
સ્થિર પૂર્ણાંક tmpTruncate(sqlite3_file*, sqlite3_int64 size);
સ્થિર પૂર્ણાંક tmpSync(sqlite3_file*, પૂર્ણાંક flags);
સ્થિર પૂર્ણાંક tmpFileSize(sqlite3_file*, sqlite3_int64 *pSize);
સ્થિર પૂર્ણાંક tmpLock(sqlite3_file*, પૂર્ણાંક);
સ્થિર પૂર્ણાંક tmpUnlock(sqlite3_file*, પૂર્ણાંક);
સ્થિર પૂર્ણાંક tmpCheckReservedLock(sqlite3_file*, પૂર્ણાંક *pResOut);
સ્થિર પૂર્ણાંક tmpFileControl(sqlite3_file*, પૂર્ણાંક op, શૂન્ય *pArg);
સ્થિર પૂર્ણાંક tmpSectorSize(sqlite3_file*);
સ્થિર પૂર્ણાંક tmpDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for fs_vfs.
*/
સ્થિર પૂર્ણાંક fsOpen(sqlite3_vfs*, અચલ અક્ષર *, sqlite3_file*, પૂર્ણાંક , પૂર્ણાંક *);
સ્થિર પૂર્ણાંક fsDelete(sqlite3_vfs*, અચલ અક્ષર *zName, પૂર્ણાંક syncDir);
સ્થિર પૂર્ણાંક fsAccess(sqlite3_vfs*, અચલ અક્ષર *zName, પૂર્ણાંક flags, પૂર્ણાંક *);
સ્થિર પૂર્ણાંક fsFullPathname(sqlite3_vfs*, અચલ અક્ષર *zName, પૂર્ણાંક nOut,અક્ષર *zOut);
સ્થિર શૂન્ય *fsDlOpen(sqlite3_vfs*, અચલ અક્ષર *zFilename);
સ્થિર શૂન્ય fsDlError(sqlite3_vfs*, પૂર્ણાંક nByte, અક્ષર *z_Err_સંદેશ);
સ્થિર શૂન્ય (*fsDlSym(sqlite3_vfs*,શૂન્ય*, અચલ અક્ષર *z_પ્રતીક))(શૂન્ય);
સ્થિર શૂન્ય fsDlClose(sqlite3_vfs*, શૂન્ય*);
સ્થિર પૂર્ણાંક fsRandomness(sqlite3_vfs*, પૂર્ણાંક nByte, અક્ષર *zOut);
સ્થિર પૂર્ણાંક fsSleep(sqlite3_vfs*, પૂર્ણાંક microseconds);
સ્થિર પૂર્ણાંક fsCurrentTime(sqlite3_vfs*, દ્વિગુણ*);


પ્રકારવ્યાખ્યા સંરચના fs_vfs_t fs_vfs_t;
સંરચના fs_vfs_t {
  sqlite3_vfs base;
  fs_real_file *pFileList;
  sqlite3_vfs *pParent;
};

સ્થિર fs_vfs_t fs_vfs = {
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

સ્થિર sqlite3_io_methods fs_io_methods = {
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


સ્થિર sqlite3_io_methods tmp_io_methods = {
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
સ્થિર પૂર્ણાંક tmpClose(sqlite3_file *pFile){
  tmp_file *pTmp = (tmp_file *)pFile;
  sqlite3_free(pTmp->zAlloc);
  પાછા SQLITE_OK;
}

/*
** Read data from a tmp-file.
*/
સ્થિર પૂર્ણાંક tmpRead(
  sqlite3_file *pFile, 
  શૂન્ય *zBuf, 
  પૂર્ણાંક iAmt, 
  sqlite_int64 iOfst
){
  tmp_file *pTmp = (tmp_file *)pFile;
  જો( (iAmt+iOfst)>pTmp->nSize ){
    પાછા SQLITE_IOERR_SHORT_READ;
  }
  memcpy(zBuf, &pTmp->zAlloc[iOfst], iAmt);
  પાછા SQLITE_OK;
}

/*
** Write data to a tmp-file.
*/
સ્થિર પૂર્ણાંક tmpWrite(
  sqlite3_file *pFile, 
  અચલ શૂન્ય *zBuf, 
  પૂર્ણાંક iAmt, 
  sqlite_int64 iOfst
){
  tmp_file *pTmp = (tmp_file *)pFile;
  જો( (iAmt+iOfst)>pTmp->nAlloc ){
    પૂર્ણાંક nNew = (પૂર્ણાંક)(2*(iAmt+iOfst+pTmp->nAlloc));
    અક્ષર *zNew = sqlite3_realloc(pTmp->zAlloc, nNew);
    જો( !zNew ){
      પાછા SQLITE_NOMEM;
    }
    pTmp->zAlloc = zNew;
    pTmp->nAlloc = nNew;
  }
  memcpy(&pTmp->zAlloc[iOfst], zBuf, iAmt);
  pTmp->nSize = (પૂર્ણાંક)MAX(pTmp->nSize, iOfst+iAmt);
  પાછા SQLITE_OK;
}

/*
** Truncate a tmp-file.
*/
સ્થિર પૂર્ણાંક tmpTruncate(sqlite3_file *pFile, sqlite_int64 size){
  tmp_file *pTmp = (tmp_file *)pFile;
  pTmp->nSize = (પૂર્ણાંક)MIN(pTmp->nSize, size);
  પાછા SQLITE_OK;
}

/*
** Sync a tmp-file.
*/
સ્થિર પૂર્ણાંક tmpSync(sqlite3_file *pFile, પૂર્ણાંક flags){
  પાછા SQLITE_OK;
}

/*
** Return the current file-size of a tmp-file.
*/
સ્થિર પૂર્ણાંક tmpFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  tmp_file *pTmp = (tmp_file *)pFile;
  *pSize = pTmp->nSize;
  પાછા SQLITE_OK;
}

/*
** Lock a tmp-file.
*/
સ્થિર પૂર્ણાંક tmpLock(sqlite3_file *pFile, પૂર્ણાંક eLock){
  પાછા SQLITE_OK;
}

/*
** Unlock a tmp-file.
*/
સ્થિર પૂર્ણાંક tmpUnlock(sqlite3_file *pFile, પૂર્ણાંક eLock){
  પાછા SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on a tmp-file.
*/
સ્થિર પૂર્ણાંક tmpCheckReservedLock(sqlite3_file *pFile, પૂર્ણાંક *pResOut){
  *pResOut = 0;
  પાછા SQLITE_OK;
}

/*
** File control method. For custom operations on a tmp-file.
*/
સ્થિર પૂર્ણાંક tmpFileControl(sqlite3_file *pFile, પૂર્ણાંક op, શૂન્ય *pArg){
  પાછા SQLITE_OK;
}

/*
** Return the sector-size in bytes for a tmp-file.
*/
સ્થિર પૂર્ણાંક tmpSectorSize(sqlite3_file *pFile){
  પાછા 0;
}

/*
** Return the device characteristic flags supported by a tmp-file.
*/
સ્થિર પૂર્ણાંક tmpDeviceCharacteristics(sqlite3_file *pFile){
  પાછા 0;
}

/*
** Close an fs-file.
*/
સ્થિર પૂર્ણાંક fsClose(sqlite3_file *pFile){
  પૂર્ણાંક rc = SQLITE_OK;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;

  /* Decrement the real_file ref-count. */
  pReal->nRef--;
  પ્રતિજ્ઞા(pReal->nRef>=0);

  /* When the ref-count reaches 0, destroy the structure */
  જો( pReal->nRef==0 ){
    *pReal->ppThis = pReal->pNext;
    જો( pReal->pNext ){
      pReal->pNext->ppThis = pReal->ppThis;
    }
    rc = pReal->pFile->pMethods->xClose(pReal->pFile);
    sqlite3_free(pReal);
  }

  પાછા rc;
}

/*
** Read data from an fs-file.
*/
સ્થિર પૂર્ણાંક fsRead(
  sqlite3_file *pFile, 
  શૂન્ય *zBuf, 
  પૂર્ણાંક iAmt, 
  sqlite_int64 iOfst
){
  પૂર્ણાંક rc = SQLITE_OK;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  sqlite3_file *pF = pReal->pFile;

  જો( (p->eType==DATABASE_FILE && (iAmt+iOfst)>pReal->nDatabase)
   || (p->eType==JOURNAL_FILE && (iAmt+iOfst)>pReal->nJournal)
  ){
    rc = SQLITE_IOERR_SHORT_READ;
  }નહીંતર જો( p->eType==DATABASE_FILE ){
    rc = pF->pMethods->xRead(pF, zBuf, iAmt, iOfst+BLOCKSIZE);
  }નહીંતર{
    /* Journal file. */
    પૂર્ણાંક iRem = iAmt;
    પૂર્ણાંક iBuf = 0;
    પૂર્ણાંક ii = (પૂર્ણાંક)iOfst;
    જ્યારે( iRem>0 && rc==SQLITE_OK ){
      પૂર્ણાંક iRealOff = pReal->nBlob - BLOCKSIZE*((ii/BLOCKSIZE)+1) + ii%BLOCKSIZE;
      પૂર્ણાંક iRealAmt = MIN(iRem, BLOCKSIZE - (iRealOff%BLOCKSIZE));

      rc = pF->pMethods->xRead(pF, &((અક્ષર *)zBuf)[iBuf], iRealAmt, iRealOff);
      ii += iRealAmt;
      iBuf += iRealAmt;
      iRem -= iRealAmt;
    }
  }

  પાછા rc;
}

/*
** Write data to an fs-file.
*/
સ્થિર પૂર્ણાંક fsWrite(
  sqlite3_file *pFile, 
  અચલ શૂન્ય *zBuf, 
  પૂર્ણાંક iAmt, 
  sqlite_int64 iOfst
){
  પૂર્ણાંક rc = SQLITE_OK;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  sqlite3_file *pF = pReal->pFile;

  જો( p->eType==DATABASE_FILE ){
    જો( (iAmt+iOfst+BLOCKSIZE)>(pReal->nBlob-pReal->nJournal) ){
      rc = SQLITE_FULL;
    }નહીંતર{
      rc = pF->pMethods->xWrite(pF, zBuf, iAmt, iOfst+BLOCKSIZE);
      જો( rc==SQLITE_OK ){
        pReal->nDatabase = (પૂર્ણાંક)MAX(pReal->nDatabase, iAmt+iOfst);
      }
    }
  }નહીંતર{
    /* Journal file. */
    પૂર્ણાંક iRem = iAmt;
    પૂર્ણાંક iBuf = 0;
    પૂર્ણાંક ii = (પૂર્ણાંક)iOfst;
    જ્યારે( iRem>0 && rc==SQLITE_OK ){
      પૂર્ણાંક iRealOff = pReal->nBlob - BLOCKSIZE*((ii/BLOCKSIZE)+1) + ii%BLOCKSIZE;
      પૂર્ણાંક iRealAmt = MIN(iRem, BLOCKSIZE - (iRealOff%BLOCKSIZE));

      જો( iRealOff<(pReal->nDatabase+BLOCKSIZE) ){
        rc = SQLITE_FULL;
      }નહીંતર{
        rc = pF->pMethods->xWrite(pF, &((અક્ષર *)zBuf)[iBuf], iRealAmt,iRealOff);
        ii += iRealAmt;
        iBuf += iRealAmt;
        iRem -= iRealAmt;
      }
    }
    જો( rc==SQLITE_OK ){
      pReal->nJournal = (પૂર્ણાંક)MAX(pReal->nJournal, iAmt+iOfst);
    }
  }

  પાછા rc;
}

/*
** Truncate an fs-file.
*/
સ્થિર પૂર્ણાંક fsTruncate(sqlite3_file *pFile, sqlite_int64 size){
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  જો( p->eType==DATABASE_FILE ){
    pReal->nDatabase = (પૂર્ણાંક)MIN(pReal->nDatabase, size);
  }નહીંતર{
    pReal->nJournal = (પૂર્ણાંક)MIN(pReal->nJournal, size);
  }
  પાછા SQLITE_OK;
}

/*
** Sync an fs-file.
*/
સ્થિર પૂર્ણાંક fsSync(sqlite3_file *pFile, પૂર્ણાંક flags){
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  sqlite3_file *pRealFile = pReal->pFile;
  પૂર્ણાંક rc = SQLITE_OK;

  જો( p->eType==DATABASE_FILE ){
    unsigned અક્ષર zSize[4];
    zSize[0] = (pReal->nDatabase&0xFF000000)>>24;
    zSize[1] = (unsigned અક્ષર)((pReal->nDatabase&0x00FF0000)>>16);
    zSize[2] = (pReal->nDatabase&0x0000FF00)>>8;
    zSize[3] = (pReal->nDatabase&0x000000FF);
    rc = pRealFile->pMethods->xWrite(pRealFile, zSize, 4, 0);
  }
  જો( rc==SQLITE_OK ){
    rc = pRealFile->pMethods->xSync(pRealFile, flags&(~SQLITE_SYNC_DATAONLY));
  }

  પાછા rc;
}

/*
** Return the current file-size of an fs-file.
*/
સ્થિર પૂર્ણાંક fsFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  જો( p->eType==DATABASE_FILE ){
    *pSize = pReal->nDatabase;
  }નહીંતર{
    *pSize = pReal->nJournal;
  }
  પાછા SQLITE_OK;
}

/*
** Lock an fs-file.
*/
સ્થિર પૂર્ણાંક fsLock(sqlite3_file *pFile, પૂર્ણાંક eLock){
  પાછા SQLITE_OK;
}

/*
** Unlock an fs-file.
*/
સ્થિર પૂર્ણાંક fsUnlock(sqlite3_file *pFile, પૂર્ણાંક eLock){
  પાછા SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on an fs-file.
*/
સ્થિર પૂર્ણાંક fsCheckReservedLock(sqlite3_file *pFile, પૂર્ણાંક *pResOut){
  *pResOut = 0;
  પાછા SQLITE_OK;
}

/*
** File control method. For custom operations on an fs-file.
*/
સ્થિર પૂર્ણાંક fsFileControl(sqlite3_file *pFile, પૂર્ણાંક op, શૂન્ય *pArg){
  જો( op==SQLITE_FCNTL_PRAGMA ) પાછા SQLITE_NOTFOUND;
  પાછા SQLITE_OK;
}

/*
** Return the sector-size in bytes for an fs-file.
*/
સ્થિર પૂર્ણાંક fsSectorSize(sqlite3_file *pFile){
  પાછા BLOCKSIZE;
}

/*
** Return the device characteristic flags supported by an fs-file.
*/
સ્થિર પૂર્ણાંક fsDeviceCharacteristics(sqlite3_file *pFile){
  પાછા 0;
}

/*
** Open an fs file handle.
*/
સ્થિર પૂર્ણાંક fsOpen(
  sqlite3_vfs *pVfs,
  અચલ અક્ષર *zName,
  sqlite3_file *pFile,
  પૂર્ણાંક flags,
  પૂર્ણાંક *pOutFlags
){
  fs_vfs_t *pFsVfs = (fs_vfs_t *)pVfs;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = 0;
  પૂર્ણાંક eType;
  પૂર્ણાંક nName;
  પૂર્ણાંક rc = SQLITE_OK;

  જો( 0==(flags&(SQLITE_OPEN_MAIN_DB|SQLITE_OPEN_MAIN_JOURNAL)) ){
    tmp_file *p2 = (tmp_file *)pFile;
    memset(p2, 0, કદ(*p2));
    p2->base.pMethods = &tmp_io_methods;
    પાછા SQLITE_OK;
  }

  eType = ((flags&(SQLITE_OPEN_MAIN_DB))?DATABASE_FILE:JOURNAL_FILE);
  p->base.pMethods = &fs_io_methods;
  p->eType = eType;

  પ્રતિજ્ઞા(strlen("-journal")==8);
  nName = (પૂર્ણાંક)strlen(zName)-((eType==JOURNAL_FILE)?8:0);
  pReal=pFsVfs->pFileList; 
  માટે(; pReal && strncmp(pReal->zName, zName, nName); pReal=pReal->pNext);

  જો( !pReal ){
    પૂર્ણાંક real_flags = (flags&~(SQLITE_OPEN_MAIN_DB))|SQLITE_OPEN_TEMP_DB;
    sqlite3_int64 size;
    sqlite3_file *pRealFile;
    sqlite3_vfs *pParent = pFsVfs->pParent;
    પ્રતિજ્ઞા(eType==DATABASE_FILE);

    pReal = (fs_real_file *)sqlite3_malloc(કદ(*pReal)+pParent->szOsFile);
    જો( !pReal ){
      rc = SQLITE_NOMEM;
      જાઓ open_out;
    }
    memset(pReal, 0, કદ(*pReal)+pParent->szOsFile);
    pReal->zName = zName;
    pReal->pFile = (sqlite3_file *)(&pReal[1]);

    rc = pParent->xOpen(pParent, zName, pReal->pFile, real_flags, pOutFlags);
    જો( rc!=SQLITE_OK ){
      જાઓ open_out;
    }
    pRealFile = pReal->pFile;

    rc = pRealFile->pMethods->xFileSize(pRealFile, &size);
    જો( rc!=SQLITE_OK ){
      જાઓ open_out;
    }
    જો( size==0 ){
      rc = pRealFile->pMethods->xWrite(pRealFile, "\0", 1, BLOBSIZE-1);
      pReal->nBlob = BLOBSIZE;
    }નહીંતર{
      unsigned અક્ષર zS[4];
      pReal->nBlob = (પૂર્ણાંક)size;
      rc = pRealFile->pMethods->xRead(pRealFile, zS, 4, 0);
      pReal->nDatabase = (zS[0]<<24)+(zS[1]<<16)+(zS[2]<<8)+zS[3];
      જો( rc==SQLITE_OK ){
        rc = pRealFile->pMethods->xRead(pRealFile, zS, 4, pReal->nBlob-4);
        જો( zS[0] || zS[1] || zS[2] || zS[3] ){
          pReal->nJournal = pReal->nBlob;
        }
      }
    }

    જો( rc==SQLITE_OK ){
      pReal->pNext = pFsVfs->pFileList;
      જો( pReal->pNext ){
        pReal->pNext->ppThis = &pReal->pNext;
      }
      pReal->ppThis = &pFsVfs->pFileList;
      pFsVfs->pFileList = pReal;
    }
  }

open_out:
  જો( pReal ){
    જો( rc==SQLITE_OK ){
      p->pReal = pReal;
      pReal->nRef++;
    }નહીંતર{
      જો( pReal->pFile->pMethods ){
        pReal->pFile->pMethods->xClose(pReal->pFile);
      }
      sqlite3_free(pReal);
    }
  }
  પાછા rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
સ્થિર પૂર્ણાંક fsDelete(sqlite3_vfs *pVfs, અચલ અક્ષર *zPath, પૂર્ણાંક dirSync){
  પૂર્ણાંક rc = SQLITE_OK;
  fs_vfs_t *pFsVfs = (fs_vfs_t *)pVfs;
  fs_real_file *pReal;
  sqlite3_file *pF;
  પૂર્ણાંક nName = (પૂર્ણાંક)strlen(zPath) - 8;

  પ્રતિજ્ઞા(strlen("-journal")==8);
  પ્રતિજ્ઞા(strcmp("-journal", &zPath[nName])==0);

  pReal = pFsVfs->pFileList; 
  માટે(; pReal && strncmp(pReal->zName, zPath, nName); pReal=pReal->pNext);
  જો( pReal ){
    pF = pReal->pFile;
    rc = pF->pMethods->xWrite(pF, "\0\0\0\0", 4, pReal->nBlob-BLOCKSIZE);
    જો( rc==SQLITE_OK ){
      pReal->nJournal = 0;
    }
  }
  પાછા rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
સ્થિર પૂર્ણાંક fsAccess(
  sqlite3_vfs *pVfs, 
  અચલ અક્ષર *zPath, 
  પૂર્ણાંક flags, 
  પૂર્ણાંક *pResOut
){
  fs_vfs_t *pFsVfs = (fs_vfs_t *)pVfs;
  fs_real_file *pReal;
  પૂર્ણાંક isJournal = 0;
  પૂર્ણાંક nName = (પૂર્ણાંક)strlen(zPath);

  જો( flags!=SQLITE_ACCESS_EXISTS ){
    sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
    પાછા pParent->xAccess(pParent, zPath, flags, pResOut);
  }

  પ્રતિજ્ઞા(strlen("-journal")==8);
  જો( nName>8 && strcmp("-journal", &zPath[nName-8])==0 ){
    nName -= 8;
    isJournal = 1;
  }

  pReal = pFsVfs->pFileList; 
  માટે(; pReal && strncmp(pReal->zName, zPath, nName); pReal=pReal->pNext);

  *pResOut = (pReal && (!isJournal || pReal->nJournal>0));
  પાછા SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (FS_MAX_PATHNAME+1) bytes.
*/
સ્થિર પૂર્ણાંક fsFullPathname(
  sqlite3_vfs *pVfs,            /* Pointer to vfs object */
  અચલ અક્ષર *zPath,            /* Possibly relative input path */
  પૂર્ણાંક nOut,                     /* Size of output buffer in bytes */
  અક્ષર *zOut                    /* Output buffer */
){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  પાછા pParent->xFullPathname(pParent, zPath, nOut, zOut);
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
સ્થિર શૂન્ય *fsDlOpen(sqlite3_vfs *pVfs, અચલ અક્ષર *zPath){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  પાછા pParent->xDlOpen(pParent, zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
સ્થિર શૂન્ય fsDlError(sqlite3_vfs *pVfs, પૂર્ણાંક nByte, અક્ષર *z_Err_સંદેશ){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  pParent->xDlError(pParent, nByte, z_Err_સંદેશ);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
સ્થિર શૂન્ય (*fsDlSym(sqlite3_vfs *pVfs, શૂન્ય *pH, અચલ અક્ષર *zSym))(શૂન્ય){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  પાછા pParent->xDlSym(pParent, pH, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
સ્થિર શૂન્ય fsDlClose(sqlite3_vfs *pVfs, શૂન્ય *pHandle){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  pParent->xDlClose(pParent, pHandle);
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
સ્થિર પૂર્ણાંક fsRandomness(sqlite3_vfs *pVfs, પૂર્ણાંક nByte, અક્ષર *zBufOut){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  પાછા pParent->xRandomness(pParent, nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
સ્થિર પૂર્ણાંક fsSleep(sqlite3_vfs *pVfs, પૂર્ણાંક nMicro){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  પાછા pParent->xSleep(pParent, nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
સ્થિર પૂર્ણાંક fsCurrentTime(sqlite3_vfs *pVfs, દ્વિગુણ *pTimeOut){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  પાછા pParent->xCurrentTime(pParent, pTimeOut);
}

/*
** This procedure registers the fs vfs with SQLite. If the argument is
** true, the fs vfs becomes the new default vfs. It is the only publicly
** available function in this file.
*/
પૂર્ણાંક fs_register(શૂન્ય){
  જો( fs_vfs.pParent ) પાછા SQLITE_OK;
  fs_vfs.pParent = sqlite3_vfs_find(0);
  fs_vfs.base.mxPathname = fs_vfs.pParent->mxPathname;
  fs_vfs.base.szOsFile = MAX(કદ(tmp_file), કદ(fs_file));
  પાછા sqlite3_vfs_register(&fs_vfs.base, 0);
}

#ifdef SQLITE_TEST
  પૂર્ણાંક SqlitetestOnefile_Init() {પાછા fs_register();}
#endif
