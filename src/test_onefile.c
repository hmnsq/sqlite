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

#அந்தர்பாவ "sqlite3.h"
#அந்தர்பாவ <assert.h>
#அந்தர்பாவ <string.h>

/*
** Maximum pathname length supported by the fs backend.
*/
#define BLOCKSIZE 512
#define BLOBSIZE 10485760

/*
** Name used to identify this VFS.
*/
#define FS_VFS_NAME "fs"

வகைவரை கட்டமைப்பு fs_real_file fs_real_file;
கட்டமைப்பு fs_real_file {
  sqlite3_file *pFile;
  மாறிலி எழுத்து *zName;
  முழு nDatabase;              /* Current size of database region */
  முழு nJournal;               /* Current size of journal region */
  முழு nBlob;                  /* Total size of allocated blob */
  முழு nRef;                   /* Number of pointers to this structure */
  fs_real_file *pNext;
  fs_real_file **ppThis;
};

வகைவரை கட்டமைப்பு fs_file fs_file;
கட்டமைப்பு fs_file {
  sqlite3_file base;
  முழு eType;
  fs_real_file *pReal;
};

வகைவரை கட்டமைப்பு tmp_file tmp_file;
கட்டமைப்பு tmp_file {
  sqlite3_file base;
  முழு nSize;
  முழு nAlloc;
  எழுத்து *zAlloc;
};

/* Values for fs_file.eType. */
#define DATABASE_FILE   1
#define JOURNAL_FILE    2

/*
** Method declarations for fs_file.
*/
நிலையான முழு fsClose(sqlite3_file*);
நிலையான முழு fsRead(sqlite3_file*, வெறுமை*, முழு iAmt, sqlite3_int64 iOfst);
நிலையான முழு fsWrite(sqlite3_file*, மாறிலி வெறுமை*, முழு iAmt, sqlite3_int64 iOfst);
நிலையான முழு fsTruncate(sqlite3_file*, sqlite3_int64 size);
நிலையான முழு fsSync(sqlite3_file*, முழு flags);
நிலையான முழு fsFileSize(sqlite3_file*, sqlite3_int64 *pSize);
நிலையான முழு fsLock(sqlite3_file*, முழு);
நிலையான முழு fsUnlock(sqlite3_file*, முழு);
நிலையான முழு fsCheckReservedLock(sqlite3_file*, முழு *pResOut);
நிலையான முழு fsFileControl(sqlite3_file*, முழு op, வெறுமை *pArg);
நிலையான முழு fsSectorSize(sqlite3_file*);
நிலையான முழு fsDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for tmp_file.
*/
நிலையான முழு tmpClose(sqlite3_file*);
நிலையான முழு tmpRead(sqlite3_file*, வெறுமை*, முழு iAmt, sqlite3_int64 iOfst);
நிலையான முழு tmpWrite(sqlite3_file*, மாறிலி வெறுமை*, முழு iAmt, sqlite3_int64 iOfst);
நிலையான முழு tmpTruncate(sqlite3_file*, sqlite3_int64 size);
நிலையான முழு tmpSync(sqlite3_file*, முழு flags);
நிலையான முழு tmpFileSize(sqlite3_file*, sqlite3_int64 *pSize);
நிலையான முழு tmpLock(sqlite3_file*, முழு);
நிலையான முழு tmpUnlock(sqlite3_file*, முழு);
நிலையான முழு tmpCheckReservedLock(sqlite3_file*, முழு *pResOut);
நிலையான முழு tmpFileControl(sqlite3_file*, முழு op, வெறுமை *pArg);
நிலையான முழு tmpSectorSize(sqlite3_file*);
நிலையான முழு tmpDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for fs_vfs.
*/
நிலையான முழு fsOpen(sqlite3_vfs*, மாறிலி எழுத்து *, sqlite3_file*, முழு , முழு *);
நிலையான முழு fsDelete(sqlite3_vfs*, மாறிலி எழுத்து *zName, முழு syncDir);
நிலையான முழு fsAccess(sqlite3_vfs*, மாறிலி எழுத்து *zName, முழு flags, முழு *);
நிலையான முழு fsFullPathname(sqlite3_vfs*, மாறிலி எழுத்து *zName, முழு nOut,எழுத்து *zOut);
நிலையான வெறுமை *fsDlOpen(sqlite3_vfs*, மாறிலி எழுத்து *zFilename);
நிலையான வெறுமை fsDlError(sqlite3_vfs*, முழு nByte, எழுத்து *z_Err_செய்தி);
நிலையான வெறுமை (*fsDlSym(sqlite3_vfs*,வெறுமை*, மாறிலி எழுத்து *z_குறியீடு))(வெறுமை);
நிலையான வெறுமை fsDlClose(sqlite3_vfs*, வெறுமை*);
நிலையான முழு fsRandomness(sqlite3_vfs*, முழு nByte, எழுத்து *zOut);
நிலையான முழு fsSleep(sqlite3_vfs*, முழு microseconds);
நிலையான முழு fsCurrentTime(sqlite3_vfs*, இரட்டை*);


வகைவரை கட்டமைப்பு fs_vfs_t fs_vfs_t;
கட்டமைப்பு fs_vfs_t {
  sqlite3_vfs base;
  fs_real_file *pFileList;
  sqlite3_vfs *pParent;
};

நிலையான fs_vfs_t fs_vfs = {
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

நிலையான sqlite3_io_methods fs_io_methods = {
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


நிலையான sqlite3_io_methods tmp_io_methods = {
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
நிலையான முழு tmpClose(sqlite3_file *pFile){
  tmp_file *pTmp = (tmp_file *)pFile;
  sqlite3_free(pTmp->zAlloc);
  திரும்பு SQLITE_OK;
}

/*
** Read data from a tmp-file.
*/
நிலையான முழு tmpRead(
  sqlite3_file *pFile, 
  வெறுமை *zBuf, 
  முழு iAmt, 
  sqlite_int64 iOfst
){
  tmp_file *pTmp = (tmp_file *)pFile;
  எனில்( (iAmt+iOfst)>pTmp->nSize ){
    திரும்பு SQLITE_IOERR_SHORT_READ;
  }
  memcpy(zBuf, &pTmp->zAlloc[iOfst], iAmt);
  திரும்பு SQLITE_OK;
}

/*
** Write data to a tmp-file.
*/
நிலையான முழு tmpWrite(
  sqlite3_file *pFile, 
  மாறிலி வெறுமை *zBuf, 
  முழு iAmt, 
  sqlite_int64 iOfst
){
  tmp_file *pTmp = (tmp_file *)pFile;
  எனில்( (iAmt+iOfst)>pTmp->nAlloc ){
    முழு nNew = (முழு)(2*(iAmt+iOfst+pTmp->nAlloc));
    எழுத்து *zNew = sqlite3_realloc(pTmp->zAlloc, nNew);
    எனில்( !zNew ){
      திரும்பு SQLITE_NOMEM;
    }
    pTmp->zAlloc = zNew;
    pTmp->nAlloc = nNew;
  }
  memcpy(&pTmp->zAlloc[iOfst], zBuf, iAmt);
  pTmp->nSize = (முழு)MAX(pTmp->nSize, iOfst+iAmt);
  திரும்பு SQLITE_OK;
}

/*
** Truncate a tmp-file.
*/
நிலையான முழு tmpTruncate(sqlite3_file *pFile, sqlite_int64 size){
  tmp_file *pTmp = (tmp_file *)pFile;
  pTmp->nSize = (முழு)MIN(pTmp->nSize, size);
  திரும்பு SQLITE_OK;
}

/*
** Sync a tmp-file.
*/
நிலையான முழு tmpSync(sqlite3_file *pFile, முழு flags){
  திரும்பு SQLITE_OK;
}

/*
** Return the current file-size of a tmp-file.
*/
நிலையான முழு tmpFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  tmp_file *pTmp = (tmp_file *)pFile;
  *pSize = pTmp->nSize;
  திரும்பு SQLITE_OK;
}

/*
** Lock a tmp-file.
*/
நிலையான முழு tmpLock(sqlite3_file *pFile, முழு eLock){
  திரும்பு SQLITE_OK;
}

/*
** Unlock a tmp-file.
*/
நிலையான முழு tmpUnlock(sqlite3_file *pFile, முழு eLock){
  திரும்பு SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on a tmp-file.
*/
நிலையான முழு tmpCheckReservedLock(sqlite3_file *pFile, முழு *pResOut){
  *pResOut = 0;
  திரும்பு SQLITE_OK;
}

/*
** File control method. For custom operations on a tmp-file.
*/
நிலையான முழு tmpFileControl(sqlite3_file *pFile, முழு op, வெறுமை *pArg){
  திரும்பு SQLITE_OK;
}

/*
** Return the sector-size in bytes for a tmp-file.
*/
நிலையான முழு tmpSectorSize(sqlite3_file *pFile){
  திரும்பு 0;
}

/*
** Return the device characteristic flags supported by a tmp-file.
*/
நிலையான முழு tmpDeviceCharacteristics(sqlite3_file *pFile){
  திரும்பு 0;
}

/*
** Close an fs-file.
*/
நிலையான முழு fsClose(sqlite3_file *pFile){
  முழு rc = SQLITE_OK;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;

  /* Decrement the real_file ref-count. */
  pReal->nRef--;
  ப்ரதிஜ்ஞா(pReal->nRef>=0);

  /* When the ref-count reaches 0, destroy the structure */
  எனில்( pReal->nRef==0 ){
    *pReal->ppThis = pReal->pNext;
    எனில்( pReal->pNext ){
      pReal->pNext->ppThis = pReal->ppThis;
    }
    rc = pReal->pFile->pMethods->xClose(pReal->pFile);
    sqlite3_free(pReal);
  }

  திரும்பு rc;
}

/*
** Read data from an fs-file.
*/
நிலையான முழு fsRead(
  sqlite3_file *pFile, 
  வெறுமை *zBuf, 
  முழு iAmt, 
  sqlite_int64 iOfst
){
  முழு rc = SQLITE_OK;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  sqlite3_file *pF = pReal->pFile;

  எனில்( (p->eType==DATABASE_FILE && (iAmt+iOfst)>pReal->nDatabase)
   || (p->eType==JOURNAL_FILE && (iAmt+iOfst)>pReal->nJournal)
  ){
    rc = SQLITE_IOERR_SHORT_READ;
  }இல்லையெனில் எனில்( p->eType==DATABASE_FILE ){
    rc = pF->pMethods->xRead(pF, zBuf, iAmt, iOfst+BLOCKSIZE);
  }இல்லையெனில்{
    /* Journal file. */
    முழு iRem = iAmt;
    முழு iBuf = 0;
    முழு ii = (முழு)iOfst;
    போது( iRem>0 && rc==SQLITE_OK ){
      முழு iRealOff = pReal->nBlob - BLOCKSIZE*((ii/BLOCKSIZE)+1) + ii%BLOCKSIZE;
      முழு iRealAmt = MIN(iRem, BLOCKSIZE - (iRealOff%BLOCKSIZE));

      rc = pF->pMethods->xRead(pF, &((எழுத்து *)zBuf)[iBuf], iRealAmt, iRealOff);
      ii += iRealAmt;
      iBuf += iRealAmt;
      iRem -= iRealAmt;
    }
  }

  திரும்பு rc;
}

/*
** Write data to an fs-file.
*/
நிலையான முழு fsWrite(
  sqlite3_file *pFile, 
  மாறிலி வெறுமை *zBuf, 
  முழு iAmt, 
  sqlite_int64 iOfst
){
  முழு rc = SQLITE_OK;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  sqlite3_file *pF = pReal->pFile;

  எனில்( p->eType==DATABASE_FILE ){
    எனில்( (iAmt+iOfst+BLOCKSIZE)>(pReal->nBlob-pReal->nJournal) ){
      rc = SQLITE_FULL;
    }இல்லையெனில்{
      rc = pF->pMethods->xWrite(pF, zBuf, iAmt, iOfst+BLOCKSIZE);
      எனில்( rc==SQLITE_OK ){
        pReal->nDatabase = (முழு)MAX(pReal->nDatabase, iAmt+iOfst);
      }
    }
  }இல்லையெனில்{
    /* Journal file. */
    முழு iRem = iAmt;
    முழு iBuf = 0;
    முழு ii = (முழு)iOfst;
    போது( iRem>0 && rc==SQLITE_OK ){
      முழு iRealOff = pReal->nBlob - BLOCKSIZE*((ii/BLOCKSIZE)+1) + ii%BLOCKSIZE;
      முழு iRealAmt = MIN(iRem, BLOCKSIZE - (iRealOff%BLOCKSIZE));

      எனில்( iRealOff<(pReal->nDatabase+BLOCKSIZE) ){
        rc = SQLITE_FULL;
      }இல்லையெனில்{
        rc = pF->pMethods->xWrite(pF, &((எழுத்து *)zBuf)[iBuf], iRealAmt,iRealOff);
        ii += iRealAmt;
        iBuf += iRealAmt;
        iRem -= iRealAmt;
      }
    }
    எனில்( rc==SQLITE_OK ){
      pReal->nJournal = (முழு)MAX(pReal->nJournal, iAmt+iOfst);
    }
  }

  திரும்பு rc;
}

/*
** Truncate an fs-file.
*/
நிலையான முழு fsTruncate(sqlite3_file *pFile, sqlite_int64 size){
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  எனில்( p->eType==DATABASE_FILE ){
    pReal->nDatabase = (முழு)MIN(pReal->nDatabase, size);
  }இல்லையெனில்{
    pReal->nJournal = (முழு)MIN(pReal->nJournal, size);
  }
  திரும்பு SQLITE_OK;
}

/*
** Sync an fs-file.
*/
நிலையான முழு fsSync(sqlite3_file *pFile, முழு flags){
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  sqlite3_file *pRealFile = pReal->pFile;
  முழு rc = SQLITE_OK;

  எனில்( p->eType==DATABASE_FILE ){
    unsigned எழுத்து zSize[4];
    zSize[0] = (pReal->nDatabase&0xFF000000)>>24;
    zSize[1] = (unsigned எழுத்து)((pReal->nDatabase&0x00FF0000)>>16);
    zSize[2] = (pReal->nDatabase&0x0000FF00)>>8;
    zSize[3] = (pReal->nDatabase&0x000000FF);
    rc = pRealFile->pMethods->xWrite(pRealFile, zSize, 4, 0);
  }
  எனில்( rc==SQLITE_OK ){
    rc = pRealFile->pMethods->xSync(pRealFile, flags&(~SQLITE_SYNC_DATAONLY));
  }

  திரும்பு rc;
}

/*
** Return the current file-size of an fs-file.
*/
நிலையான முழு fsFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  எனில்( p->eType==DATABASE_FILE ){
    *pSize = pReal->nDatabase;
  }இல்லையெனில்{
    *pSize = pReal->nJournal;
  }
  திரும்பு SQLITE_OK;
}

/*
** Lock an fs-file.
*/
நிலையான முழு fsLock(sqlite3_file *pFile, முழு eLock){
  திரும்பு SQLITE_OK;
}

/*
** Unlock an fs-file.
*/
நிலையான முழு fsUnlock(sqlite3_file *pFile, முழு eLock){
  திரும்பு SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on an fs-file.
*/
நிலையான முழு fsCheckReservedLock(sqlite3_file *pFile, முழு *pResOut){
  *pResOut = 0;
  திரும்பு SQLITE_OK;
}

/*
** File control method. For custom operations on an fs-file.
*/
நிலையான முழு fsFileControl(sqlite3_file *pFile, முழு op, வெறுமை *pArg){
  எனில்( op==SQLITE_FCNTL_PRAGMA ) திரும்பு SQLITE_NOTFOUND;
  திரும்பு SQLITE_OK;
}

/*
** Return the sector-size in bytes for an fs-file.
*/
நிலையான முழு fsSectorSize(sqlite3_file *pFile){
  திரும்பு BLOCKSIZE;
}

/*
** Return the device characteristic flags supported by an fs-file.
*/
நிலையான முழு fsDeviceCharacteristics(sqlite3_file *pFile){
  திரும்பு 0;
}

/*
** Open an fs file handle.
*/
நிலையான முழு fsOpen(
  sqlite3_vfs *pVfs,
  மாறிலி எழுத்து *zName,
  sqlite3_file *pFile,
  முழு flags,
  முழு *pOutFlags
){
  fs_vfs_t *pFsVfs = (fs_vfs_t *)pVfs;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = 0;
  முழு eType;
  முழு nName;
  முழு rc = SQLITE_OK;

  எனில்( 0==(flags&(SQLITE_OPEN_MAIN_DB|SQLITE_OPEN_MAIN_JOURNAL)) ){
    tmp_file *p2 = (tmp_file *)pFile;
    memset(p2, 0, அளவு(*p2));
    p2->base.pMethods = &tmp_io_methods;
    திரும்பு SQLITE_OK;
  }

  eType = ((flags&(SQLITE_OPEN_MAIN_DB))?DATABASE_FILE:JOURNAL_FILE);
  p->base.pMethods = &fs_io_methods;
  p->eType = eType;

  ப்ரதிஜ்ஞா(strlen("-journal")==8);
  nName = (முழு)strlen(zName)-((eType==JOURNAL_FILE)?8:0);
  pReal=pFsVfs->pFileList; 
  க்கு(; pReal && strncmp(pReal->zName, zName, nName); pReal=pReal->pNext);

  எனில்( !pReal ){
    முழு real_flags = (flags&~(SQLITE_OPEN_MAIN_DB))|SQLITE_OPEN_TEMP_DB;
    sqlite3_int64 size;
    sqlite3_file *pRealFile;
    sqlite3_vfs *pParent = pFsVfs->pParent;
    ப்ரதிஜ்ஞா(eType==DATABASE_FILE);

    pReal = (fs_real_file *)sqlite3_malloc(அளவு(*pReal)+pParent->szOsFile);
    எனில்( !pReal ){
      rc = SQLITE_NOMEM;
      செல் open_out;
    }
    memset(pReal, 0, அளவு(*pReal)+pParent->szOsFile);
    pReal->zName = zName;
    pReal->pFile = (sqlite3_file *)(&pReal[1]);

    rc = pParent->xOpen(pParent, zName, pReal->pFile, real_flags, pOutFlags);
    எனில்( rc!=SQLITE_OK ){
      செல் open_out;
    }
    pRealFile = pReal->pFile;

    rc = pRealFile->pMethods->xFileSize(pRealFile, &size);
    எனில்( rc!=SQLITE_OK ){
      செல் open_out;
    }
    எனில்( size==0 ){
      rc = pRealFile->pMethods->xWrite(pRealFile, "\0", 1, BLOBSIZE-1);
      pReal->nBlob = BLOBSIZE;
    }இல்லையெனில்{
      unsigned எழுத்து zS[4];
      pReal->nBlob = (முழு)size;
      rc = pRealFile->pMethods->xRead(pRealFile, zS, 4, 0);
      pReal->nDatabase = (zS[0]<<24)+(zS[1]<<16)+(zS[2]<<8)+zS[3];
      எனில்( rc==SQLITE_OK ){
        rc = pRealFile->pMethods->xRead(pRealFile, zS, 4, pReal->nBlob-4);
        எனில்( zS[0] || zS[1] || zS[2] || zS[3] ){
          pReal->nJournal = pReal->nBlob;
        }
      }
    }

    எனில்( rc==SQLITE_OK ){
      pReal->pNext = pFsVfs->pFileList;
      எனில்( pReal->pNext ){
        pReal->pNext->ppThis = &pReal->pNext;
      }
      pReal->ppThis = &pFsVfs->pFileList;
      pFsVfs->pFileList = pReal;
    }
  }

open_out:
  எனில்( pReal ){
    எனில்( rc==SQLITE_OK ){
      p->pReal = pReal;
      pReal->nRef++;
    }இல்லையெனில்{
      எனில்( pReal->pFile->pMethods ){
        pReal->pFile->pMethods->xClose(pReal->pFile);
      }
      sqlite3_free(pReal);
    }
  }
  திரும்பு rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
நிலையான முழு fsDelete(sqlite3_vfs *pVfs, மாறிலி எழுத்து *zPath, முழு dirSync){
  முழு rc = SQLITE_OK;
  fs_vfs_t *pFsVfs = (fs_vfs_t *)pVfs;
  fs_real_file *pReal;
  sqlite3_file *pF;
  முழு nName = (முழு)strlen(zPath) - 8;

  ப்ரதிஜ்ஞா(strlen("-journal")==8);
  ப்ரதிஜ்ஞா(strcmp("-journal", &zPath[nName])==0);

  pReal = pFsVfs->pFileList; 
  க்கு(; pReal && strncmp(pReal->zName, zPath, nName); pReal=pReal->pNext);
  எனில்( pReal ){
    pF = pReal->pFile;
    rc = pF->pMethods->xWrite(pF, "\0\0\0\0", 4, pReal->nBlob-BLOCKSIZE);
    எனில்( rc==SQLITE_OK ){
      pReal->nJournal = 0;
    }
  }
  திரும்பு rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
நிலையான முழு fsAccess(
  sqlite3_vfs *pVfs, 
  மாறிலி எழுத்து *zPath, 
  முழு flags, 
  முழு *pResOut
){
  fs_vfs_t *pFsVfs = (fs_vfs_t *)pVfs;
  fs_real_file *pReal;
  முழு isJournal = 0;
  முழு nName = (முழு)strlen(zPath);

  எனில்( flags!=SQLITE_ACCESS_EXISTS ){
    sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
    திரும்பு pParent->xAccess(pParent, zPath, flags, pResOut);
  }

  ப்ரதிஜ்ஞா(strlen("-journal")==8);
  எனில்( nName>8 && strcmp("-journal", &zPath[nName-8])==0 ){
    nName -= 8;
    isJournal = 1;
  }

  pReal = pFsVfs->pFileList; 
  க்கு(; pReal && strncmp(pReal->zName, zPath, nName); pReal=pReal->pNext);

  *pResOut = (pReal && (!isJournal || pReal->nJournal>0));
  திரும்பு SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (FS_MAX_PATHNAME+1) bytes.
*/
நிலையான முழு fsFullPathname(
  sqlite3_vfs *pVfs,            /* Pointer to vfs object */
  மாறிலி எழுத்து *zPath,            /* Possibly relative input path */
  முழு nOut,                     /* Size of output buffer in bytes */
  எழுத்து *zOut                    /* Output buffer */
){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  திரும்பு pParent->xFullPathname(pParent, zPath, nOut, zOut);
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
நிலையான வெறுமை *fsDlOpen(sqlite3_vfs *pVfs, மாறிலி எழுத்து *zPath){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  திரும்பு pParent->xDlOpen(pParent, zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
நிலையான வெறுமை fsDlError(sqlite3_vfs *pVfs, முழு nByte, எழுத்து *z_Err_செய்தி){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  pParent->xDlError(pParent, nByte, z_Err_செய்தி);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
நிலையான வெறுமை (*fsDlSym(sqlite3_vfs *pVfs, வெறுமை *pH, மாறிலி எழுத்து *zSym))(வெறுமை){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  திரும்பு pParent->xDlSym(pParent, pH, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
நிலையான வெறுமை fsDlClose(sqlite3_vfs *pVfs, வெறுமை *pHandle){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  pParent->xDlClose(pParent, pHandle);
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
நிலையான முழு fsRandomness(sqlite3_vfs *pVfs, முழு nByte, எழுத்து *zBufOut){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  திரும்பு pParent->xRandomness(pParent, nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
நிலையான முழு fsSleep(sqlite3_vfs *pVfs, முழு nMicro){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  திரும்பு pParent->xSleep(pParent, nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
நிலையான முழு fsCurrentTime(sqlite3_vfs *pVfs, இரட்டை *pTimeOut){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  திரும்பு pParent->xCurrentTime(pParent, pTimeOut);
}

/*
** This procedure registers the fs vfs with SQLite. If the argument is
** true, the fs vfs becomes the new default vfs. It is the only publicly
** available function in this file.
*/
முழு fs_register(வெறுமை){
  எனில்( fs_vfs.pParent ) திரும்பு SQLITE_OK;
  fs_vfs.pParent = sqlite3_vfs_find(0);
  fs_vfs.base.mxPathname = fs_vfs.pParent->mxPathname;
  fs_vfs.base.szOsFile = MAX(அளவு(tmp_file), அளவு(fs_file));
  திரும்பு sqlite3_vfs_register(&fs_vfs.base, 0);
}

#ifdef SQLITE_TEST
  முழு SqlitetestOnefile_Init() {திரும்பு fs_register();}
#endif
