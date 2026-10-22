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

#ضمن "sqlite3.h"
#ضمن <assert.h>
#ضمن <string.h>

/*
** Maximum pathname length supported by the fs backend.
*/
#define BLOCKSIZE 512
#define BLOBSIZE 10485760

/*
** Name used to identify this VFS.
*/
#define FS_VFS_NAME "fs"

تعريفنوع بنية fs_real_file fs_real_file;
بنية fs_real_file {
  sqlite3_file *pFile;
  ثابت حرف *zName;
  صحيح nDatabase;              /* Current size of database region */
  صحيح nJournal;               /* Current size of journal region */
  صحيح nBlob;                  /* Total size of allocated blob */
  صحيح nRef;                   /* Number of pointers to this structure */
  fs_real_file *pNext;
  fs_real_file **ppThis;
};

تعريفنوع بنية fs_file fs_file;
بنية fs_file {
  sqlite3_file base;
  صحيح eType;
  fs_real_file *pReal;
};

تعريفنوع بنية tmp_file tmp_file;
بنية tmp_file {
  sqlite3_file base;
  صحيح nSize;
  صحيح nAlloc;
  حرف *zAlloc;
};

/* Values for fs_file.eType. */
#define DATABASE_FILE   1
#define JOURNAL_FILE    2

/*
** Method declarations for fs_file.
*/
ساكن صحيح fsClose(sqlite3_file*);
ساكن صحيح fsRead(sqlite3_file*, فراغ*, صحيح iAmt, sqlite3_int64 iOfst);
ساكن صحيح fsWrite(sqlite3_file*, ثابت فراغ*, صحيح iAmt, sqlite3_int64 iOfst);
ساكن صحيح fsTruncate(sqlite3_file*, sqlite3_int64 size);
ساكن صحيح fsSync(sqlite3_file*, صحيح flags);
ساكن صحيح fsFileSize(sqlite3_file*, sqlite3_int64 *pSize);
ساكن صحيح fsLock(sqlite3_file*, صحيح);
ساكن صحيح fsUnlock(sqlite3_file*, صحيح);
ساكن صحيح fsCheckReservedLock(sqlite3_file*, صحيح *pResOut);
ساكن صحيح fsFileControl(sqlite3_file*, صحيح op, فراغ *pArg);
ساكن صحيح fsSectorSize(sqlite3_file*);
ساكن صحيح fsDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for tmp_file.
*/
ساكن صحيح tmpClose(sqlite3_file*);
ساكن صحيح tmpRead(sqlite3_file*, فراغ*, صحيح iAmt, sqlite3_int64 iOfst);
ساكن صحيح tmpWrite(sqlite3_file*, ثابت فراغ*, صحيح iAmt, sqlite3_int64 iOfst);
ساكن صحيح tmpTruncate(sqlite3_file*, sqlite3_int64 size);
ساكن صحيح tmpSync(sqlite3_file*, صحيح flags);
ساكن صحيح tmpFileSize(sqlite3_file*, sqlite3_int64 *pSize);
ساكن صحيح tmpLock(sqlite3_file*, صحيح);
ساكن صحيح tmpUnlock(sqlite3_file*, صحيح);
ساكن صحيح tmpCheckReservedLock(sqlite3_file*, صحيح *pResOut);
ساكن صحيح tmpFileControl(sqlite3_file*, صحيح op, فراغ *pArg);
ساكن صحيح tmpSectorSize(sqlite3_file*);
ساكن صحيح tmpDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for fs_vfs.
*/
ساكن صحيح fsOpen(sqlite3_vfs*, ثابت حرف *, sqlite3_file*, صحيح , صحيح *);
ساكن صحيح fsDelete(sqlite3_vfs*, ثابت حرف *zName, صحيح syncDir);
ساكن صحيح fsAccess(sqlite3_vfs*, ثابت حرف *zName, صحيح flags, صحيح *);
ساكن صحيح fsFullPathname(sqlite3_vfs*, ثابت حرف *zName, صحيح nOut,حرف *zOut);
ساكن فراغ *fsDlOpen(sqlite3_vfs*, ثابت حرف *zFilename);
ساكن فراغ fsDlError(sqlite3_vfs*, صحيح nByte, حرف *z_Err_رسالة);
ساكن فراغ (*fsDlSym(sqlite3_vfs*,فراغ*, ثابت حرف *z_رمز))(فراغ);
ساكن فراغ fsDlClose(sqlite3_vfs*, فراغ*);
ساكن صحيح fsRandomness(sqlite3_vfs*, صحيح nByte, حرف *zOut);
ساكن صحيح fsSleep(sqlite3_vfs*, صحيح microseconds);
ساكن صحيح fsCurrentTime(sqlite3_vfs*, مزدوج*);


تعريفنوع بنية fs_vfs_t fs_vfs_t;
بنية fs_vfs_t {
  sqlite3_vfs base;
  fs_real_file *pFileList;
  sqlite3_vfs *pParent;
};

ساكن fs_vfs_t fs_vfs = {
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

ساكن sqlite3_io_methods fs_io_methods = {
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


ساكن sqlite3_io_methods tmp_io_methods = {
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
ساكن صحيح tmpClose(sqlite3_file *pFile){
  tmp_file *pTmp = (tmp_file *)pFile;
  sqlite3_free(pTmp->zAlloc);
  أرجع SQLITE_OK;
}

/*
** Read data from a tmp-file.
*/
ساكن صحيح tmpRead(
  sqlite3_file *pFile, 
  فراغ *zBuf, 
  صحيح iAmt, 
  sqlite_int64 iOfst
){
  tmp_file *pTmp = (tmp_file *)pFile;
  إذا( (iAmt+iOfst)>pTmp->nSize ){
    أرجع SQLITE_IOERR_SHORT_READ;
  }
  memcpy(zBuf, &pTmp->zAlloc[iOfst], iAmt);
  أرجع SQLITE_OK;
}

/*
** Write data to a tmp-file.
*/
ساكن صحيح tmpWrite(
  sqlite3_file *pFile, 
  ثابت فراغ *zBuf, 
  صحيح iAmt, 
  sqlite_int64 iOfst
){
  tmp_file *pTmp = (tmp_file *)pFile;
  إذا( (iAmt+iOfst)>pTmp->nAlloc ){
    صحيح nNew = (صحيح)(2*(iAmt+iOfst+pTmp->nAlloc));
    حرف *zNew = sqlite3_realloc(pTmp->zAlloc, nNew);
    إذا( !zNew ){
      أرجع SQLITE_NOMEM;
    }
    pTmp->zAlloc = zNew;
    pTmp->nAlloc = nNew;
  }
  memcpy(&pTmp->zAlloc[iOfst], zBuf, iAmt);
  pTmp->nSize = (صحيح)MAX(pTmp->nSize, iOfst+iAmt);
  أرجع SQLITE_OK;
}

/*
** Truncate a tmp-file.
*/
ساكن صحيح tmpTruncate(sqlite3_file *pFile, sqlite_int64 size){
  tmp_file *pTmp = (tmp_file *)pFile;
  pTmp->nSize = (صحيح)MIN(pTmp->nSize, size);
  أرجع SQLITE_OK;
}

/*
** Sync a tmp-file.
*/
ساكن صحيح tmpSync(sqlite3_file *pFile, صحيح flags){
  أرجع SQLITE_OK;
}

/*
** Return the current file-size of a tmp-file.
*/
ساكن صحيح tmpFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  tmp_file *pTmp = (tmp_file *)pFile;
  *pSize = pTmp->nSize;
  أرجع SQLITE_OK;
}

/*
** Lock a tmp-file.
*/
ساكن صحيح tmpLock(sqlite3_file *pFile, صحيح eLock){
  أرجع SQLITE_OK;
}

/*
** Unlock a tmp-file.
*/
ساكن صحيح tmpUnlock(sqlite3_file *pFile, صحيح eLock){
  أرجع SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on a tmp-file.
*/
ساكن صحيح tmpCheckReservedLock(sqlite3_file *pFile, صحيح *pResOut){
  *pResOut = 0;
  أرجع SQLITE_OK;
}

/*
** File control method. For custom operations on a tmp-file.
*/
ساكن صحيح tmpFileControl(sqlite3_file *pFile, صحيح op, فراغ *pArg){
  أرجع SQLITE_OK;
}

/*
** Return the sector-size in bytes for a tmp-file.
*/
ساكن صحيح tmpSectorSize(sqlite3_file *pFile){
  أرجع 0;
}

/*
** Return the device characteristic flags supported by a tmp-file.
*/
ساكن صحيح tmpDeviceCharacteristics(sqlite3_file *pFile){
  أرجع 0;
}

/*
** Close an fs-file.
*/
ساكن صحيح fsClose(sqlite3_file *pFile){
  صحيح rc = SQLITE_OK;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;

  /* Decrement the real_file ref-count. */
  pReal->nRef--;
  أكد(pReal->nRef>=0);

  /* When the ref-count reaches 0, destroy the structure */
  إذا( pReal->nRef==0 ){
    *pReal->ppThis = pReal->pNext;
    إذا( pReal->pNext ){
      pReal->pNext->ppThis = pReal->ppThis;
    }
    rc = pReal->pFile->pMethods->xClose(pReal->pFile);
    sqlite3_free(pReal);
  }

  أرجع rc;
}

/*
** Read data from an fs-file.
*/
ساكن صحيح fsRead(
  sqlite3_file *pFile, 
  فراغ *zBuf, 
  صحيح iAmt, 
  sqlite_int64 iOfst
){
  صحيح rc = SQLITE_OK;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  sqlite3_file *pF = pReal->pFile;

  إذا( (p->eType==DATABASE_FILE && (iAmt+iOfst)>pReal->nDatabase)
   || (p->eType==JOURNAL_FILE && (iAmt+iOfst)>pReal->nJournal)
  ){
    rc = SQLITE_IOERR_SHORT_READ;
  }وإلا إذا( p->eType==DATABASE_FILE ){
    rc = pF->pMethods->xRead(pF, zBuf, iAmt, iOfst+BLOCKSIZE);
  }وإلا{
    /* Journal file. */
    صحيح iRem = iAmt;
    صحيح iBuf = 0;
    صحيح ii = (صحيح)iOfst;
    طالما( iRem>0 && rc==SQLITE_OK ){
      صحيح iRealOff = pReal->nBlob - BLOCKSIZE*((ii/BLOCKSIZE)+1) + ii%BLOCKSIZE;
      صحيح iRealAmt = MIN(iRem, BLOCKSIZE - (iRealOff%BLOCKSIZE));

      rc = pF->pMethods->xRead(pF, &((حرف *)zBuf)[iBuf], iRealAmt, iRealOff);
      ii += iRealAmt;
      iBuf += iRealAmt;
      iRem -= iRealAmt;
    }
  }

  أرجع rc;
}

/*
** Write data to an fs-file.
*/
ساكن صحيح fsWrite(
  sqlite3_file *pFile, 
  ثابت فراغ *zBuf, 
  صحيح iAmt, 
  sqlite_int64 iOfst
){
  صحيح rc = SQLITE_OK;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  sqlite3_file *pF = pReal->pFile;

  إذا( p->eType==DATABASE_FILE ){
    إذا( (iAmt+iOfst+BLOCKSIZE)>(pReal->nBlob-pReal->nJournal) ){
      rc = SQLITE_FULL;
    }وإلا{
      rc = pF->pMethods->xWrite(pF, zBuf, iAmt, iOfst+BLOCKSIZE);
      إذا( rc==SQLITE_OK ){
        pReal->nDatabase = (صحيح)MAX(pReal->nDatabase, iAmt+iOfst);
      }
    }
  }وإلا{
    /* Journal file. */
    صحيح iRem = iAmt;
    صحيح iBuf = 0;
    صحيح ii = (صحيح)iOfst;
    طالما( iRem>0 && rc==SQLITE_OK ){
      صحيح iRealOff = pReal->nBlob - BLOCKSIZE*((ii/BLOCKSIZE)+1) + ii%BLOCKSIZE;
      صحيح iRealAmt = MIN(iRem, BLOCKSIZE - (iRealOff%BLOCKSIZE));

      إذا( iRealOff<(pReal->nDatabase+BLOCKSIZE) ){
        rc = SQLITE_FULL;
      }وإلا{
        rc = pF->pMethods->xWrite(pF, &((حرف *)zBuf)[iBuf], iRealAmt,iRealOff);
        ii += iRealAmt;
        iBuf += iRealAmt;
        iRem -= iRealAmt;
      }
    }
    إذا( rc==SQLITE_OK ){
      pReal->nJournal = (صحيح)MAX(pReal->nJournal, iAmt+iOfst);
    }
  }

  أرجع rc;
}

/*
** Truncate an fs-file.
*/
ساكن صحيح fsTruncate(sqlite3_file *pFile, sqlite_int64 size){
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  إذا( p->eType==DATABASE_FILE ){
    pReal->nDatabase = (صحيح)MIN(pReal->nDatabase, size);
  }وإلا{
    pReal->nJournal = (صحيح)MIN(pReal->nJournal, size);
  }
  أرجع SQLITE_OK;
}

/*
** Sync an fs-file.
*/
ساكن صحيح fsSync(sqlite3_file *pFile, صحيح flags){
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  sqlite3_file *pRealFile = pReal->pFile;
  صحيح rc = SQLITE_OK;

  إذا( p->eType==DATABASE_FILE ){
    unsigned حرف zSize[4];
    zSize[0] = (pReal->nDatabase&0xFF000000)>>24;
    zSize[1] = (unsigned حرف)((pReal->nDatabase&0x00FF0000)>>16);
    zSize[2] = (pReal->nDatabase&0x0000FF00)>>8;
    zSize[3] = (pReal->nDatabase&0x000000FF);
    rc = pRealFile->pMethods->xWrite(pRealFile, zSize, 4, 0);
  }
  إذا( rc==SQLITE_OK ){
    rc = pRealFile->pMethods->xSync(pRealFile, flags&(~SQLITE_SYNC_DATAONLY));
  }

  أرجع rc;
}

/*
** Return the current file-size of an fs-file.
*/
ساكن صحيح fsFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  إذا( p->eType==DATABASE_FILE ){
    *pSize = pReal->nDatabase;
  }وإلا{
    *pSize = pReal->nJournal;
  }
  أرجع SQLITE_OK;
}

/*
** Lock an fs-file.
*/
ساكن صحيح fsLock(sqlite3_file *pFile, صحيح eLock){
  أرجع SQLITE_OK;
}

/*
** Unlock an fs-file.
*/
ساكن صحيح fsUnlock(sqlite3_file *pFile, صحيح eLock){
  أرجع SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on an fs-file.
*/
ساكن صحيح fsCheckReservedLock(sqlite3_file *pFile, صحيح *pResOut){
  *pResOut = 0;
  أرجع SQLITE_OK;
}

/*
** File control method. For custom operations on an fs-file.
*/
ساكن صحيح fsFileControl(sqlite3_file *pFile, صحيح op, فراغ *pArg){
  إذا( op==SQLITE_FCNTL_PRAGMA ) أرجع SQLITE_NOTFOUND;
  أرجع SQLITE_OK;
}

/*
** Return the sector-size in bytes for an fs-file.
*/
ساكن صحيح fsSectorSize(sqlite3_file *pFile){
  أرجع BLOCKSIZE;
}

/*
** Return the device characteristic flags supported by an fs-file.
*/
ساكن صحيح fsDeviceCharacteristics(sqlite3_file *pFile){
  أرجع 0;
}

/*
** Open an fs file handle.
*/
ساكن صحيح fsOpen(
  sqlite3_vfs *pVfs,
  ثابت حرف *zName,
  sqlite3_file *pFile,
  صحيح flags,
  صحيح *pOutFlags
){
  fs_vfs_t *pFsVfs = (fs_vfs_t *)pVfs;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = 0;
  صحيح eType;
  صحيح nName;
  صحيح rc = SQLITE_OK;

  إذا( 0==(flags&(SQLITE_OPEN_MAIN_DB|SQLITE_OPEN_MAIN_JOURNAL)) ){
    tmp_file *p2 = (tmp_file *)pFile;
    memset(p2, 0, حجم(*p2));
    p2->base.pMethods = &tmp_io_methods;
    أرجع SQLITE_OK;
  }

  eType = ((flags&(SQLITE_OPEN_MAIN_DB))?DATABASE_FILE:JOURNAL_FILE);
  p->base.pMethods = &fs_io_methods;
  p->eType = eType;

  أكد(strlen("-journal")==8);
  nName = (صحيح)strlen(zName)-((eType==JOURNAL_FILE)?8:0);
  pReal=pFsVfs->pFileList; 
  لكل(; pReal && strncmp(pReal->zName, zName, nName); pReal=pReal->pNext);

  إذا( !pReal ){
    صحيح real_flags = (flags&~(SQLITE_OPEN_MAIN_DB))|SQLITE_OPEN_TEMP_DB;
    sqlite3_int64 size;
    sqlite3_file *pRealFile;
    sqlite3_vfs *pParent = pFsVfs->pParent;
    أكد(eType==DATABASE_FILE);

    pReal = (fs_real_file *)sqlite3_malloc(حجم(*pReal)+pParent->szOsFile);
    إذا( !pReal ){
      rc = SQLITE_NOMEM;
      اذهب open_out;
    }
    memset(pReal, 0, حجم(*pReal)+pParent->szOsFile);
    pReal->zName = zName;
    pReal->pFile = (sqlite3_file *)(&pReal[1]);

    rc = pParent->xOpen(pParent, zName, pReal->pFile, real_flags, pOutFlags);
    إذا( rc!=SQLITE_OK ){
      اذهب open_out;
    }
    pRealFile = pReal->pFile;

    rc = pRealFile->pMethods->xFileSize(pRealFile, &size);
    إذا( rc!=SQLITE_OK ){
      اذهب open_out;
    }
    إذا( size==0 ){
      rc = pRealFile->pMethods->xWrite(pRealFile, "\0", 1, BLOBSIZE-1);
      pReal->nBlob = BLOBSIZE;
    }وإلا{
      unsigned حرف zS[4];
      pReal->nBlob = (صحيح)size;
      rc = pRealFile->pMethods->xRead(pRealFile, zS, 4, 0);
      pReal->nDatabase = (zS[0]<<24)+(zS[1]<<16)+(zS[2]<<8)+zS[3];
      إذا( rc==SQLITE_OK ){
        rc = pRealFile->pMethods->xRead(pRealFile, zS, 4, pReal->nBlob-4);
        إذا( zS[0] || zS[1] || zS[2] || zS[3] ){
          pReal->nJournal = pReal->nBlob;
        }
      }
    }

    إذا( rc==SQLITE_OK ){
      pReal->pNext = pFsVfs->pFileList;
      إذا( pReal->pNext ){
        pReal->pNext->ppThis = &pReal->pNext;
      }
      pReal->ppThis = &pFsVfs->pFileList;
      pFsVfs->pFileList = pReal;
    }
  }

open_out:
  إذا( pReal ){
    إذا( rc==SQLITE_OK ){
      p->pReal = pReal;
      pReal->nRef++;
    }وإلا{
      إذا( pReal->pFile->pMethods ){
        pReal->pFile->pMethods->xClose(pReal->pFile);
      }
      sqlite3_free(pReal);
    }
  }
  أرجع rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
ساكن صحيح fsDelete(sqlite3_vfs *pVfs, ثابت حرف *zPath, صحيح dirSync){
  صحيح rc = SQLITE_OK;
  fs_vfs_t *pFsVfs = (fs_vfs_t *)pVfs;
  fs_real_file *pReal;
  sqlite3_file *pF;
  صحيح nName = (صحيح)strlen(zPath) - 8;

  أكد(strlen("-journal")==8);
  أكد(strcmp("-journal", &zPath[nName])==0);

  pReal = pFsVfs->pFileList; 
  لكل(; pReal && strncmp(pReal->zName, zPath, nName); pReal=pReal->pNext);
  إذا( pReal ){
    pF = pReal->pFile;
    rc = pF->pMethods->xWrite(pF, "\0\0\0\0", 4, pReal->nBlob-BLOCKSIZE);
    إذا( rc==SQLITE_OK ){
      pReal->nJournal = 0;
    }
  }
  أرجع rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
ساكن صحيح fsAccess(
  sqlite3_vfs *pVfs, 
  ثابت حرف *zPath, 
  صحيح flags, 
  صحيح *pResOut
){
  fs_vfs_t *pFsVfs = (fs_vfs_t *)pVfs;
  fs_real_file *pReal;
  صحيح isJournal = 0;
  صحيح nName = (صحيح)strlen(zPath);

  إذا( flags!=SQLITE_ACCESS_EXISTS ){
    sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
    أرجع pParent->xAccess(pParent, zPath, flags, pResOut);
  }

  أكد(strlen("-journal")==8);
  إذا( nName>8 && strcmp("-journal", &zPath[nName-8])==0 ){
    nName -= 8;
    isJournal = 1;
  }

  pReal = pFsVfs->pFileList; 
  لكل(; pReal && strncmp(pReal->zName, zPath, nName); pReal=pReal->pNext);

  *pResOut = (pReal && (!isJournal || pReal->nJournal>0));
  أرجع SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (FS_MAX_PATHNAME+1) bytes.
*/
ساكن صحيح fsFullPathname(
  sqlite3_vfs *pVfs,            /* Pointer to vfs object */
  ثابت حرف *zPath,            /* Possibly relative input path */
  صحيح nOut,                     /* Size of output buffer in bytes */
  حرف *zOut                    /* Output buffer */
){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  أرجع pParent->xFullPathname(pParent, zPath, nOut, zOut);
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
ساكن فراغ *fsDlOpen(sqlite3_vfs *pVfs, ثابت حرف *zPath){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  أرجع pParent->xDlOpen(pParent, zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
ساكن فراغ fsDlError(sqlite3_vfs *pVfs, صحيح nByte, حرف *z_Err_رسالة){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  pParent->xDlError(pParent, nByte, z_Err_رسالة);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
ساكن فراغ (*fsDlSym(sqlite3_vfs *pVfs, فراغ *pH, ثابت حرف *zSym))(فراغ){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  أرجع pParent->xDlSym(pParent, pH, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
ساكن فراغ fsDlClose(sqlite3_vfs *pVfs, فراغ *pHandle){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  pParent->xDlClose(pParent, pHandle);
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
ساكن صحيح fsRandomness(sqlite3_vfs *pVfs, صحيح nByte, حرف *zBufOut){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  أرجع pParent->xRandomness(pParent, nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
ساكن صحيح fsSleep(sqlite3_vfs *pVfs, صحيح nMicro){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  أرجع pParent->xSleep(pParent, nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
ساكن صحيح fsCurrentTime(sqlite3_vfs *pVfs, مزدوج *pTimeOut){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  أرجع pParent->xCurrentTime(pParent, pTimeOut);
}

/*
** This procedure registers the fs vfs with SQLite. If the argument is
** true, the fs vfs becomes the new default vfs. It is the only publicly
** available function in this file.
*/
صحيح fs_register(فراغ){
  إذا( fs_vfs.pParent ) أرجع SQLITE_OK;
  fs_vfs.pParent = sqlite3_vfs_find(0);
  fs_vfs.base.mxPathname = fs_vfs.pParent->mxPathname;
  fs_vfs.base.szOsFile = MAX(حجم(tmp_file), حجم(fs_file));
  أرجع sqlite3_vfs_register(&fs_vfs.base, 0);
}

#ifdef SQLITE_TEST
  صحيح SqlitetestOnefile_Init() {أرجع fs_register();}
#endif
