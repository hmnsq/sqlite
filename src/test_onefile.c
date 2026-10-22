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

#شامل "sqlite3.h"
#شامل <assert.h>
#شامل <string.h>

/*
** Maximum pathname length supported by the fs backend.
*/
#define BLOCKSIZE 512
#define BLOBSIZE 10485760

/*
** Name used to identify this VFS.
*/
#define FS_VFS_NAME "fs"

تعریفنوع ساختار fs_real_file fs_real_file;
ساختار fs_real_file {
  sqlite3_file *pFile;
  ثابت نویسه *zName;
  صحیح nDatabase;              /* Current size of database region */
  صحیح nJournal;               /* Current size of journal region */
  صحیح nBlob;                  /* Total size of allocated blob */
  صحیح nRef;                   /* Number of pointers to this structure */
  fs_real_file *pNext;
  fs_real_file **ppThis;
};

تعریفنوع ساختار fs_file fs_file;
ساختار fs_file {
  sqlite3_file base;
  صحیح eType;
  fs_real_file *pReal;
};

تعریفنوع ساختار tmp_file tmp_file;
ساختار tmp_file {
  sqlite3_file base;
  صحیح nSize;
  صحیح nAlloc;
  نویسه *zAlloc;
};

/* Values for fs_file.eType. */
#define DATABASE_FILE   1
#define JOURNAL_FILE    2

/*
** Method declarations for fs_file.
*/
ایستا صحیح fsClose(sqlite3_file*);
ایستا صحیح fsRead(sqlite3_file*, تهی*, صحیح iAmt, sqlite3_int64 iOfst);
ایستا صحیح fsWrite(sqlite3_file*, ثابت تهی*, صحیح iAmt, sqlite3_int64 iOfst);
ایستا صحیح fsTruncate(sqlite3_file*, sqlite3_int64 size);
ایستا صحیح fsSync(sqlite3_file*, صحیح flags);
ایستا صحیح fsFileSize(sqlite3_file*, sqlite3_int64 *pSize);
ایستا صحیح fsLock(sqlite3_file*, صحیح);
ایستا صحیح fsUnlock(sqlite3_file*, صحیح);
ایستا صحیح fsCheckReservedLock(sqlite3_file*, صحیح *pResOut);
ایستا صحیح fsFileControl(sqlite3_file*, صحیح op, تهی *pArg);
ایستا صحیح fsSectorSize(sqlite3_file*);
ایستا صحیح fsDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for tmp_file.
*/
ایستا صحیح tmpClose(sqlite3_file*);
ایستا صحیح tmpRead(sqlite3_file*, تهی*, صحیح iAmt, sqlite3_int64 iOfst);
ایستا صحیح tmpWrite(sqlite3_file*, ثابت تهی*, صحیح iAmt, sqlite3_int64 iOfst);
ایستا صحیح tmpTruncate(sqlite3_file*, sqlite3_int64 size);
ایستا صحیح tmpSync(sqlite3_file*, صحیح flags);
ایستا صحیح tmpFileSize(sqlite3_file*, sqlite3_int64 *pSize);
ایستا صحیح tmpLock(sqlite3_file*, صحیح);
ایستا صحیح tmpUnlock(sqlite3_file*, صحیح);
ایستا صحیح tmpCheckReservedLock(sqlite3_file*, صحیح *pResOut);
ایستا صحیح tmpFileControl(sqlite3_file*, صحیح op, تهی *pArg);
ایستا صحیح tmpSectorSize(sqlite3_file*);
ایستا صحیح tmpDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for fs_vfs.
*/
ایستا صحیح fsOpen(sqlite3_vfs*, ثابت نویسه *, sqlite3_file*, صحیح , صحیح *);
ایستا صحیح fsDelete(sqlite3_vfs*, ثابت نویسه *zName, صحیح syncDir);
ایستا صحیح fsAccess(sqlite3_vfs*, ثابت نویسه *zName, صحیح flags, صحیح *);
ایستا صحیح fsFullPathname(sqlite3_vfs*, ثابت نویسه *zName, صحیح nOut,نویسه *zOut);
ایستا تهی *fsDlOpen(sqlite3_vfs*, ثابت نویسه *zFilename);
ایستا تهی fsDlError(sqlite3_vfs*, صحیح nByte, نویسه *z_Err_پیام);
ایستا تهی (*fsDlSym(sqlite3_vfs*,تهی*, ثابت نویسه *z_نماد))(تهی);
ایستا تهی fsDlClose(sqlite3_vfs*, تهی*);
ایستا صحیح fsRandomness(sqlite3_vfs*, صحیح nByte, نویسه *zOut);
ایستا صحیح fsSleep(sqlite3_vfs*, صحیح microseconds);
ایستا صحیح fsCurrentTime(sqlite3_vfs*, دوگانه*);


تعریفنوع ساختار fs_vfs_t fs_vfs_t;
ساختار fs_vfs_t {
  sqlite3_vfs base;
  fs_real_file *pFileList;
  sqlite3_vfs *pParent;
};

ایستا fs_vfs_t fs_vfs = {
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

ایستا sqlite3_io_methods fs_io_methods = {
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


ایستا sqlite3_io_methods tmp_io_methods = {
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
ایستا صحیح tmpClose(sqlite3_file *pFile){
  tmp_file *pTmp = (tmp_file *)pFile;
  sqlite3_free(pTmp->zAlloc);
  بازگشت SQLITE_OK;
}

/*
** Read data from a tmp-file.
*/
ایستا صحیح tmpRead(
  sqlite3_file *pFile, 
  تهی *zBuf, 
  صحیح iAmt, 
  sqlite_int64 iOfst
){
  tmp_file *pTmp = (tmp_file *)pFile;
  اگر( (iAmt+iOfst)>pTmp->nSize ){
    بازگشت SQLITE_IOERR_SHORT_READ;
  }
  memcpy(zBuf, &pTmp->zAlloc[iOfst], iAmt);
  بازگشت SQLITE_OK;
}

/*
** Write data to a tmp-file.
*/
ایستا صحیح tmpWrite(
  sqlite3_file *pFile, 
  ثابت تهی *zBuf, 
  صحیح iAmt, 
  sqlite_int64 iOfst
){
  tmp_file *pTmp = (tmp_file *)pFile;
  اگر( (iAmt+iOfst)>pTmp->nAlloc ){
    صحیح nNew = (صحیح)(2*(iAmt+iOfst+pTmp->nAlloc));
    نویسه *zNew = sqlite3_realloc(pTmp->zAlloc, nNew);
    اگر( !zNew ){
      بازگشت SQLITE_NOMEM;
    }
    pTmp->zAlloc = zNew;
    pTmp->nAlloc = nNew;
  }
  memcpy(&pTmp->zAlloc[iOfst], zBuf, iAmt);
  pTmp->nSize = (صحیح)MAX(pTmp->nSize, iOfst+iAmt);
  بازگشت SQLITE_OK;
}

/*
** Truncate a tmp-file.
*/
ایستا صحیح tmpTruncate(sqlite3_file *pFile, sqlite_int64 size){
  tmp_file *pTmp = (tmp_file *)pFile;
  pTmp->nSize = (صحیح)MIN(pTmp->nSize, size);
  بازگشت SQLITE_OK;
}

/*
** Sync a tmp-file.
*/
ایستا صحیح tmpSync(sqlite3_file *pFile, صحیح flags){
  بازگشت SQLITE_OK;
}

/*
** Return the current file-size of a tmp-file.
*/
ایستا صحیح tmpFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  tmp_file *pTmp = (tmp_file *)pFile;
  *pSize = pTmp->nSize;
  بازگشت SQLITE_OK;
}

/*
** Lock a tmp-file.
*/
ایستا صحیح tmpLock(sqlite3_file *pFile, صحیح eLock){
  بازگشت SQLITE_OK;
}

/*
** Unlock a tmp-file.
*/
ایستا صحیح tmpUnlock(sqlite3_file *pFile, صحیح eLock){
  بازگشت SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on a tmp-file.
*/
ایستا صحیح tmpCheckReservedLock(sqlite3_file *pFile, صحیح *pResOut){
  *pResOut = 0;
  بازگشت SQLITE_OK;
}

/*
** File control method. For custom operations on a tmp-file.
*/
ایستا صحیح tmpFileControl(sqlite3_file *pFile, صحیح op, تهی *pArg){
  بازگشت SQLITE_OK;
}

/*
** Return the sector-size in bytes for a tmp-file.
*/
ایستا صحیح tmpSectorSize(sqlite3_file *pFile){
  بازگشت 0;
}

/*
** Return the device characteristic flags supported by a tmp-file.
*/
ایستا صحیح tmpDeviceCharacteristics(sqlite3_file *pFile){
  بازگشت 0;
}

/*
** Close an fs-file.
*/
ایستا صحیح fsClose(sqlite3_file *pFile){
  صحیح rc = SQLITE_OK;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;

  /* Decrement the real_file ref-count. */
  pReal->nRef--;
  أكد(pReal->nRef>=0);

  /* When the ref-count reaches 0, destroy the structure */
  اگر( pReal->nRef==0 ){
    *pReal->ppThis = pReal->pNext;
    اگر( pReal->pNext ){
      pReal->pNext->ppThis = pReal->ppThis;
    }
    rc = pReal->pFile->pMethods->xClose(pReal->pFile);
    sqlite3_free(pReal);
  }

  بازگشت rc;
}

/*
** Read data from an fs-file.
*/
ایستا صحیح fsRead(
  sqlite3_file *pFile, 
  تهی *zBuf, 
  صحیح iAmt, 
  sqlite_int64 iOfst
){
  صحیح rc = SQLITE_OK;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  sqlite3_file *pF = pReal->pFile;

  اگر( (p->eType==DATABASE_FILE && (iAmt+iOfst)>pReal->nDatabase)
   || (p->eType==JOURNAL_FILE && (iAmt+iOfst)>pReal->nJournal)
  ){
    rc = SQLITE_IOERR_SHORT_READ;
  }وگرنه اگر( p->eType==DATABASE_FILE ){
    rc = pF->pMethods->xRead(pF, zBuf, iAmt, iOfst+BLOCKSIZE);
  }وگرنه{
    /* Journal file. */
    صحیح iRem = iAmt;
    صحیح iBuf = 0;
    صحیح ii = (صحیح)iOfst;
    وقتی( iRem>0 && rc==SQLITE_OK ){
      صحیح iRealOff = pReal->nBlob - BLOCKSIZE*((ii/BLOCKSIZE)+1) + ii%BLOCKSIZE;
      صحیح iRealAmt = MIN(iRem, BLOCKSIZE - (iRealOff%BLOCKSIZE));

      rc = pF->pMethods->xRead(pF, &((نویسه *)zBuf)[iBuf], iRealAmt, iRealOff);
      ii += iRealAmt;
      iBuf += iRealAmt;
      iRem -= iRealAmt;
    }
  }

  بازگشت rc;
}

/*
** Write data to an fs-file.
*/
ایستا صحیح fsWrite(
  sqlite3_file *pFile, 
  ثابت تهی *zBuf, 
  صحیح iAmt, 
  sqlite_int64 iOfst
){
  صحیح rc = SQLITE_OK;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  sqlite3_file *pF = pReal->pFile;

  اگر( p->eType==DATABASE_FILE ){
    اگر( (iAmt+iOfst+BLOCKSIZE)>(pReal->nBlob-pReal->nJournal) ){
      rc = SQLITE_FULL;
    }وگرنه{
      rc = pF->pMethods->xWrite(pF, zBuf, iAmt, iOfst+BLOCKSIZE);
      اگر( rc==SQLITE_OK ){
        pReal->nDatabase = (صحیح)MAX(pReal->nDatabase, iAmt+iOfst);
      }
    }
  }وگرنه{
    /* Journal file. */
    صحیح iRem = iAmt;
    صحیح iBuf = 0;
    صحیح ii = (صحیح)iOfst;
    وقتی( iRem>0 && rc==SQLITE_OK ){
      صحیح iRealOff = pReal->nBlob - BLOCKSIZE*((ii/BLOCKSIZE)+1) + ii%BLOCKSIZE;
      صحیح iRealAmt = MIN(iRem, BLOCKSIZE - (iRealOff%BLOCKSIZE));

      اگر( iRealOff<(pReal->nDatabase+BLOCKSIZE) ){
        rc = SQLITE_FULL;
      }وگرنه{
        rc = pF->pMethods->xWrite(pF, &((نویسه *)zBuf)[iBuf], iRealAmt,iRealOff);
        ii += iRealAmt;
        iBuf += iRealAmt;
        iRem -= iRealAmt;
      }
    }
    اگر( rc==SQLITE_OK ){
      pReal->nJournal = (صحیح)MAX(pReal->nJournal, iAmt+iOfst);
    }
  }

  بازگشت rc;
}

/*
** Truncate an fs-file.
*/
ایستا صحیح fsTruncate(sqlite3_file *pFile, sqlite_int64 size){
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  اگر( p->eType==DATABASE_FILE ){
    pReal->nDatabase = (صحیح)MIN(pReal->nDatabase, size);
  }وگرنه{
    pReal->nJournal = (صحیح)MIN(pReal->nJournal, size);
  }
  بازگشت SQLITE_OK;
}

/*
** Sync an fs-file.
*/
ایستا صحیح fsSync(sqlite3_file *pFile, صحیح flags){
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  sqlite3_file *pRealFile = pReal->pFile;
  صحیح rc = SQLITE_OK;

  اگر( p->eType==DATABASE_FILE ){
    unsigned نویسه zSize[4];
    zSize[0] = (pReal->nDatabase&0xFF000000)>>24;
    zSize[1] = (unsigned نویسه)((pReal->nDatabase&0x00FF0000)>>16);
    zSize[2] = (pReal->nDatabase&0x0000FF00)>>8;
    zSize[3] = (pReal->nDatabase&0x000000FF);
    rc = pRealFile->pMethods->xWrite(pRealFile, zSize, 4, 0);
  }
  اگر( rc==SQLITE_OK ){
    rc = pRealFile->pMethods->xSync(pRealFile, flags&(~SQLITE_SYNC_DATAONLY));
  }

  بازگشت rc;
}

/*
** Return the current file-size of an fs-file.
*/
ایستا صحیح fsFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  اگر( p->eType==DATABASE_FILE ){
    *pSize = pReal->nDatabase;
  }وگرنه{
    *pSize = pReal->nJournal;
  }
  بازگشت SQLITE_OK;
}

/*
** Lock an fs-file.
*/
ایستا صحیح fsLock(sqlite3_file *pFile, صحیح eLock){
  بازگشت SQLITE_OK;
}

/*
** Unlock an fs-file.
*/
ایستا صحیح fsUnlock(sqlite3_file *pFile, صحیح eLock){
  بازگشت SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on an fs-file.
*/
ایستا صحیح fsCheckReservedLock(sqlite3_file *pFile, صحیح *pResOut){
  *pResOut = 0;
  بازگشت SQLITE_OK;
}

/*
** File control method. For custom operations on an fs-file.
*/
ایستا صحیح fsFileControl(sqlite3_file *pFile, صحیح op, تهی *pArg){
  اگر( op==SQLITE_FCNTL_PRAGMA ) بازگشت SQLITE_NOTFOUND;
  بازگشت SQLITE_OK;
}

/*
** Return the sector-size in bytes for an fs-file.
*/
ایستا صحیح fsSectorSize(sqlite3_file *pFile){
  بازگشت BLOCKSIZE;
}

/*
** Return the device characteristic flags supported by an fs-file.
*/
ایستا صحیح fsDeviceCharacteristics(sqlite3_file *pFile){
  بازگشت 0;
}

/*
** Open an fs file handle.
*/
ایستا صحیح fsOpen(
  sqlite3_vfs *pVfs,
  ثابت نویسه *zName,
  sqlite3_file *pFile,
  صحیح flags,
  صحیح *pOutFlags
){
  fs_vfs_t *pFsVfs = (fs_vfs_t *)pVfs;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = 0;
  صحیح eType;
  صحیح nName;
  صحیح rc = SQLITE_OK;

  اگر( 0==(flags&(SQLITE_OPEN_MAIN_DB|SQLITE_OPEN_MAIN_JOURNAL)) ){
    tmp_file *p2 = (tmp_file *)pFile;
    memset(p2, 0, اندازه(*p2));
    p2->base.pMethods = &tmp_io_methods;
    بازگشت SQLITE_OK;
  }

  eType = ((flags&(SQLITE_OPEN_MAIN_DB))?DATABASE_FILE:JOURNAL_FILE);
  p->base.pMethods = &fs_io_methods;
  p->eType = eType;

  أكد(strlen("-journal")==8);
  nName = (صحیح)strlen(zName)-((eType==JOURNAL_FILE)?8:0);
  pReal=pFsVfs->pFileList; 
  برای(; pReal && strncmp(pReal->zName, zName, nName); pReal=pReal->pNext);

  اگر( !pReal ){
    صحیح real_flags = (flags&~(SQLITE_OPEN_MAIN_DB))|SQLITE_OPEN_TEMP_DB;
    sqlite3_int64 size;
    sqlite3_file *pRealFile;
    sqlite3_vfs *pParent = pFsVfs->pParent;
    أكد(eType==DATABASE_FILE);

    pReal = (fs_real_file *)sqlite3_malloc(اندازه(*pReal)+pParent->szOsFile);
    اگر( !pReal ){
      rc = SQLITE_NOMEM;
      برو open_out;
    }
    memset(pReal, 0, اندازه(*pReal)+pParent->szOsFile);
    pReal->zName = zName;
    pReal->pFile = (sqlite3_file *)(&pReal[1]);

    rc = pParent->xOpen(pParent, zName, pReal->pFile, real_flags, pOutFlags);
    اگر( rc!=SQLITE_OK ){
      برو open_out;
    }
    pRealFile = pReal->pFile;

    rc = pRealFile->pMethods->xFileSize(pRealFile, &size);
    اگر( rc!=SQLITE_OK ){
      برو open_out;
    }
    اگر( size==0 ){
      rc = pRealFile->pMethods->xWrite(pRealFile, "\0", 1, BLOBSIZE-1);
      pReal->nBlob = BLOBSIZE;
    }وگرنه{
      unsigned نویسه zS[4];
      pReal->nBlob = (صحیح)size;
      rc = pRealFile->pMethods->xRead(pRealFile, zS, 4, 0);
      pReal->nDatabase = (zS[0]<<24)+(zS[1]<<16)+(zS[2]<<8)+zS[3];
      اگر( rc==SQLITE_OK ){
        rc = pRealFile->pMethods->xRead(pRealFile, zS, 4, pReal->nBlob-4);
        اگر( zS[0] || zS[1] || zS[2] || zS[3] ){
          pReal->nJournal = pReal->nBlob;
        }
      }
    }

    اگر( rc==SQLITE_OK ){
      pReal->pNext = pFsVfs->pFileList;
      اگر( pReal->pNext ){
        pReal->pNext->ppThis = &pReal->pNext;
      }
      pReal->ppThis = &pFsVfs->pFileList;
      pFsVfs->pFileList = pReal;
    }
  }

open_out:
  اگر( pReal ){
    اگر( rc==SQLITE_OK ){
      p->pReal = pReal;
      pReal->nRef++;
    }وگرنه{
      اگر( pReal->pFile->pMethods ){
        pReal->pFile->pMethods->xClose(pReal->pFile);
      }
      sqlite3_free(pReal);
    }
  }
  بازگشت rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
ایستا صحیح fsDelete(sqlite3_vfs *pVfs, ثابت نویسه *zPath, صحیح dirSync){
  صحیح rc = SQLITE_OK;
  fs_vfs_t *pFsVfs = (fs_vfs_t *)pVfs;
  fs_real_file *pReal;
  sqlite3_file *pF;
  صحیح nName = (صحیح)strlen(zPath) - 8;

  أكد(strlen("-journal")==8);
  أكد(strcmp("-journal", &zPath[nName])==0);

  pReal = pFsVfs->pFileList; 
  برای(; pReal && strncmp(pReal->zName, zPath, nName); pReal=pReal->pNext);
  اگر( pReal ){
    pF = pReal->pFile;
    rc = pF->pMethods->xWrite(pF, "\0\0\0\0", 4, pReal->nBlob-BLOCKSIZE);
    اگر( rc==SQLITE_OK ){
      pReal->nJournal = 0;
    }
  }
  بازگشت rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
ایستا صحیح fsAccess(
  sqlite3_vfs *pVfs, 
  ثابت نویسه *zPath, 
  صحیح flags, 
  صحیح *pResOut
){
  fs_vfs_t *pFsVfs = (fs_vfs_t *)pVfs;
  fs_real_file *pReal;
  صحیح isJournal = 0;
  صحیح nName = (صحیح)strlen(zPath);

  اگر( flags!=SQLITE_ACCESS_EXISTS ){
    sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
    بازگشت pParent->xAccess(pParent, zPath, flags, pResOut);
  }

  أكد(strlen("-journal")==8);
  اگر( nName>8 && strcmp("-journal", &zPath[nName-8])==0 ){
    nName -= 8;
    isJournal = 1;
  }

  pReal = pFsVfs->pFileList; 
  برای(; pReal && strncmp(pReal->zName, zPath, nName); pReal=pReal->pNext);

  *pResOut = (pReal && (!isJournal || pReal->nJournal>0));
  بازگشت SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (FS_MAX_PATHNAME+1) bytes.
*/
ایستا صحیح fsFullPathname(
  sqlite3_vfs *pVfs,            /* Pointer to vfs object */
  ثابت نویسه *zPath,            /* Possibly relative input path */
  صحیح nOut,                     /* Size of output buffer in bytes */
  نویسه *zOut                    /* Output buffer */
){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  بازگشت pParent->xFullPathname(pParent, zPath, nOut, zOut);
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
ایستا تهی *fsDlOpen(sqlite3_vfs *pVfs, ثابت نویسه *zPath){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  بازگشت pParent->xDlOpen(pParent, zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
ایستا تهی fsDlError(sqlite3_vfs *pVfs, صحیح nByte, نویسه *z_Err_پیام){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  pParent->xDlError(pParent, nByte, z_Err_پیام);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
ایستا تهی (*fsDlSym(sqlite3_vfs *pVfs, تهی *pH, ثابت نویسه *zSym))(تهی){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  بازگشت pParent->xDlSym(pParent, pH, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
ایستا تهی fsDlClose(sqlite3_vfs *pVfs, تهی *pHandle){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  pParent->xDlClose(pParent, pHandle);
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
ایستا صحیح fsRandomness(sqlite3_vfs *pVfs, صحیح nByte, نویسه *zBufOut){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  بازگشت pParent->xRandomness(pParent, nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
ایستا صحیح fsSleep(sqlite3_vfs *pVfs, صحیح nMicro){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  بازگشت pParent->xSleep(pParent, nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
ایستا صحیح fsCurrentTime(sqlite3_vfs *pVfs, دوگانه *pTimeOut){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  بازگشت pParent->xCurrentTime(pParent, pTimeOut);
}

/*
** This procedure registers the fs vfs with SQLite. If the argument is
** true, the fs vfs becomes the new default vfs. It is the only publicly
** available function in this file.
*/
صحیح fs_register(تهی){
  اگر( fs_vfs.pParent ) بازگشت SQLITE_OK;
  fs_vfs.pParent = sqlite3_vfs_find(0);
  fs_vfs.base.mxPathname = fs_vfs.pParent->mxPathname;
  fs_vfs.base.szOsFile = MAX(اندازه(tmp_file), اندازه(fs_file));
  بازگشت sqlite3_vfs_register(&fs_vfs.base, 0);
}

#ifdef SQLITE_TEST
  صحیح SqlitetestOnefile_Init() {بازگشت fs_register();}
#endif
