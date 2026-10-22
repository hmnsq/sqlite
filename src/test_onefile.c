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

#include "sqlite3.h"
#include <assert.h>
#include <string.h>

/*
** Maximum pathname length supported by the fs backend.
*/
#define BLOCKSIZE 512
#define BLOBSIZE 10485760

/*
** Name used to identify this VFS.
*/
#define FS_VFS_NAME "fs"

typedef struct fs_real_file fs_real_file;
struct fs_real_file {
  sqlite3_file *pFile;
  const char *zName;
  цяло nDatabase;              /* Current size of database region */
  цяло nJournal;               /* Current size of journal region */
  цяло nBlob;                  /* Total size of allocated blob */
  цяло nRef;                   /* Number of pointers to this structure */
  fs_real_file *pNext;
  fs_real_file **ppThis;
};

typedef struct fs_file fs_file;
struct fs_file {
  sqlite3_file base;
  цяло eType;
  fs_real_file *pReal;
};

typedef struct tmp_file tmp_file;
struct tmp_file {
  sqlite3_file base;
  цяло nSize;
  цяло nAlloc;
  char *zAlloc;
};

/* Values for fs_file.eType. */
#define DATABASE_FILE   1
#define JOURNAL_FILE    2

/*
** Method declarations for fs_file.
*/
static цяло fsClose(sqlite3_file*);
static цяло fsRead(sqlite3_file*, void*, цяло iAmt, sqlite3_int64 iOfst);
static цяло fsWrite(sqlite3_file*, const void*, цяло iAmt, sqlite3_int64 iOfst);
static цяло fsTruncate(sqlite3_file*, sqlite3_int64 size);
static цяло fsSync(sqlite3_file*, цяло flags);
static цяло fsFileSize(sqlite3_file*, sqlite3_int64 *pSize);
static цяло fsLock(sqlite3_file*, цяло);
static цяло fsUnlock(sqlite3_file*, цяло);
static цяло fsCheckReservedLock(sqlite3_file*, цяло *pResOut);
static цяло fsFileControl(sqlite3_file*, цяло op, void *pArg);
static цяло fsSectorSize(sqlite3_file*);
static цяло fsDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for tmp_file.
*/
static цяло tmpClose(sqlite3_file*);
static цяло tmpRead(sqlite3_file*, void*, цяло iAmt, sqlite3_int64 iOfst);
static цяло tmpWrite(sqlite3_file*, const void*, цяло iAmt, sqlite3_int64 iOfst);
static цяло tmpTruncate(sqlite3_file*, sqlite3_int64 size);
static цяло tmpSync(sqlite3_file*, цяло flags);
static цяло tmpFileSize(sqlite3_file*, sqlite3_int64 *pSize);
static цяло tmpLock(sqlite3_file*, цяло);
static цяло tmpUnlock(sqlite3_file*, цяло);
static цяло tmpCheckReservedLock(sqlite3_file*, цяло *pResOut);
static цяло tmpFileControl(sqlite3_file*, цяло op, void *pArg);
static цяло tmpSectorSize(sqlite3_file*);
static цяло tmpDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for fs_vfs.
*/
static цяло fsOpen(sqlite3_vfs*, const char *, sqlite3_file*, цяло , цяло *);
static цяло fsDelete(sqlite3_vfs*, const char *zName, цяло syncDir);
static цяло fsAccess(sqlite3_vfs*, const char *zName, цяло flags, цяло *);
static цяло fsFullPathname(sqlite3_vfs*, const char *zName, цяло nOut,char *zOut);
static void *fsDlOpen(sqlite3_vfs*, const char *zFilename);
static void fsDlError(sqlite3_vfs*, цяло nByte, char *zErrMsg);
static void (*fsDlSym(sqlite3_vfs*,void*, const char *zSymbol))(void);
static void fsDlClose(sqlite3_vfs*, void*);
static цяло fsRandomness(sqlite3_vfs*, цяло nByte, char *zOut);
static цяло fsSleep(sqlite3_vfs*, цяло microseconds);
static цяло fsCurrentTime(sqlite3_vfs*, double*);


typedef struct fs_vfs_t fs_vfs_t;
struct fs_vfs_t {
  sqlite3_vfs base;
  fs_real_file *pFileList;
  sqlite3_vfs *pParent;
};

static fs_vfs_t fs_vfs = {
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

static sqlite3_io_methods fs_io_methods = {
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


static sqlite3_io_methods tmp_io_methods = {
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
static цяло tmpClose(sqlite3_file *pFile){
  tmp_file *pTmp = (tmp_file *)pFile;
  sqlite3_free(pTmp->zAlloc);
  върни SQLITE_OK;
}

/*
** Read data from a tmp-file.
*/
static цяло tmpRead(
  sqlite3_file *pFile, 
  void *zBuf, 
  цяло iAmt, 
  sqlite_int64 iOfst
){
  tmp_file *pTmp = (tmp_file *)pFile;
  ако( (iAmt+iOfst)>pTmp->nSize ){
    върни SQLITE_IOERR_SHORT_READ;
  }
  memcpy(zBuf, &pTmp->zAlloc[iOfst], iAmt);
  върни SQLITE_OK;
}

/*
** Write data to a tmp-file.
*/
static цяло tmpWrite(
  sqlite3_file *pFile, 
  const void *zBuf, 
  цяло iAmt, 
  sqlite_int64 iOfst
){
  tmp_file *pTmp = (tmp_file *)pFile;
  ако( (iAmt+iOfst)>pTmp->nAlloc ){
    цяло nNew = (цяло)(2*(iAmt+iOfst+pTmp->nAlloc));
    char *zNew = sqlite3_realloc(pTmp->zAlloc, nNew);
    ако( !zNew ){
      върни SQLITE_NOMEM;
    }
    pTmp->zAlloc = zNew;
    pTmp->nAlloc = nNew;
  }
  memcpy(&pTmp->zAlloc[iOfst], zBuf, iAmt);
  pTmp->nSize = (цяло)MAX(pTmp->nSize, iOfst+iAmt);
  върни SQLITE_OK;
}

/*
** Truncate a tmp-file.
*/
static цяло tmpTruncate(sqlite3_file *pFile, sqlite_int64 size){
  tmp_file *pTmp = (tmp_file *)pFile;
  pTmp->nSize = (цяло)MIN(pTmp->nSize, size);
  върни SQLITE_OK;
}

/*
** Sync a tmp-file.
*/
static цяло tmpSync(sqlite3_file *pFile, цяло flags){
  върни SQLITE_OK;
}

/*
** Return the current file-size of a tmp-file.
*/
static цяло tmpFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  tmp_file *pTmp = (tmp_file *)pFile;
  *pSize = pTmp->nSize;
  върни SQLITE_OK;
}

/*
** Lock a tmp-file.
*/
static цяло tmpLock(sqlite3_file *pFile, цяло eLock){
  върни SQLITE_OK;
}

/*
** Unlock a tmp-file.
*/
static цяло tmpUnlock(sqlite3_file *pFile, цяло eLock){
  върни SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on a tmp-file.
*/
static цяло tmpCheckReservedLock(sqlite3_file *pFile, цяло *pResOut){
  *pResOut = 0;
  върни SQLITE_OK;
}

/*
** File control method. For custom operations on a tmp-file.
*/
static цяло tmpFileControl(sqlite3_file *pFile, цяло op, void *pArg){
  върни SQLITE_OK;
}

/*
** Return the sector-size in bytes for a tmp-file.
*/
static цяло tmpSectorSize(sqlite3_file *pFile){
  върни 0;
}

/*
** Return the device characteristic flags supported by a tmp-file.
*/
static цяло tmpDeviceCharacteristics(sqlite3_file *pFile){
  върни 0;
}

/*
** Close an fs-file.
*/
static цяло fsClose(sqlite3_file *pFile){
  цяло rc = SQLITE_OK;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;

  /* Decrement the real_file ref-count. */
  pReal->nRef--;
  assert(pReal->nRef>=0);

  /* When the ref-count reaches 0, destroy the structure */
  ако( pReal->nRef==0 ){
    *pReal->ppThis = pReal->pNext;
    ако( pReal->pNext ){
      pReal->pNext->ppThis = pReal->ppThis;
    }
    rc = pReal->pFile->pMethods->xClose(pReal->pFile);
    sqlite3_free(pReal);
  }

  върни rc;
}

/*
** Read data from an fs-file.
*/
static цяло fsRead(
  sqlite3_file *pFile, 
  void *zBuf, 
  цяло iAmt, 
  sqlite_int64 iOfst
){
  цяло rc = SQLITE_OK;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  sqlite3_file *pF = pReal->pFile;

  ако( (p->eType==DATABASE_FILE && (iAmt+iOfst)>pReal->nDatabase)
   || (p->eType==JOURNAL_FILE && (iAmt+iOfst)>pReal->nJournal)
  ){
    rc = SQLITE_IOERR_SHORT_READ;
  }иначе ако( p->eType==DATABASE_FILE ){
    rc = pF->pMethods->xRead(pF, zBuf, iAmt, iOfst+BLOCKSIZE);
  }иначе{
    /* Journal file. */
    цяло iRem = iAmt;
    цяло iBuf = 0;
    цяло ii = (цяло)iOfst;
    докато( iRem>0 && rc==SQLITE_OK ){
      цяло iRealOff = pReal->nBlob - BLOCKSIZE*((ii/BLOCKSIZE)+1) + ii%BLOCKSIZE;
      цяло iRealAmt = MIN(iRem, BLOCKSIZE - (iRealOff%BLOCKSIZE));

      rc = pF->pMethods->xRead(pF, &((char *)zBuf)[iBuf], iRealAmt, iRealOff);
      ii += iRealAmt;
      iBuf += iRealAmt;
      iRem -= iRealAmt;
    }
  }

  върни rc;
}

/*
** Write data to an fs-file.
*/
static цяло fsWrite(
  sqlite3_file *pFile, 
  const void *zBuf, 
  цяло iAmt, 
  sqlite_int64 iOfst
){
  цяло rc = SQLITE_OK;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  sqlite3_file *pF = pReal->pFile;

  ако( p->eType==DATABASE_FILE ){
    ако( (iAmt+iOfst+BLOCKSIZE)>(pReal->nBlob-pReal->nJournal) ){
      rc = SQLITE_FULL;
    }иначе{
      rc = pF->pMethods->xWrite(pF, zBuf, iAmt, iOfst+BLOCKSIZE);
      ако( rc==SQLITE_OK ){
        pReal->nDatabase = (цяло)MAX(pReal->nDatabase, iAmt+iOfst);
      }
    }
  }иначе{
    /* Journal file. */
    цяло iRem = iAmt;
    цяло iBuf = 0;
    цяло ii = (цяло)iOfst;
    докато( iRem>0 && rc==SQLITE_OK ){
      цяло iRealOff = pReal->nBlob - BLOCKSIZE*((ii/BLOCKSIZE)+1) + ii%BLOCKSIZE;
      цяло iRealAmt = MIN(iRem, BLOCKSIZE - (iRealOff%BLOCKSIZE));

      ако( iRealOff<(pReal->nDatabase+BLOCKSIZE) ){
        rc = SQLITE_FULL;
      }иначе{
        rc = pF->pMethods->xWrite(pF, &((char *)zBuf)[iBuf], iRealAmt,iRealOff);
        ii += iRealAmt;
        iBuf += iRealAmt;
        iRem -= iRealAmt;
      }
    }
    ако( rc==SQLITE_OK ){
      pReal->nJournal = (цяло)MAX(pReal->nJournal, iAmt+iOfst);
    }
  }

  върни rc;
}

/*
** Truncate an fs-file.
*/
static цяло fsTruncate(sqlite3_file *pFile, sqlite_int64 size){
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  ако( p->eType==DATABASE_FILE ){
    pReal->nDatabase = (цяло)MIN(pReal->nDatabase, size);
  }иначе{
    pReal->nJournal = (цяло)MIN(pReal->nJournal, size);
  }
  върни SQLITE_OK;
}

/*
** Sync an fs-file.
*/
static цяло fsSync(sqlite3_file *pFile, цяло flags){
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  sqlite3_file *pRealFile = pReal->pFile;
  цяло rc = SQLITE_OK;

  ако( p->eType==DATABASE_FILE ){
    unsigned char zSize[4];
    zSize[0] = (pReal->nDatabase&0xFF000000)>>24;
    zSize[1] = (unsigned char)((pReal->nDatabase&0x00FF0000)>>16);
    zSize[2] = (pReal->nDatabase&0x0000FF00)>>8;
    zSize[3] = (pReal->nDatabase&0x000000FF);
    rc = pRealFile->pMethods->xWrite(pRealFile, zSize, 4, 0);
  }
  ако( rc==SQLITE_OK ){
    rc = pRealFile->pMethods->xSync(pRealFile, flags&(~SQLITE_SYNC_DATAONLY));
  }

  върни rc;
}

/*
** Return the current file-size of an fs-file.
*/
static цяло fsFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  ако( p->eType==DATABASE_FILE ){
    *pSize = pReal->nDatabase;
  }иначе{
    *pSize = pReal->nJournal;
  }
  върни SQLITE_OK;
}

/*
** Lock an fs-file.
*/
static цяло fsLock(sqlite3_file *pFile, цяло eLock){
  върни SQLITE_OK;
}

/*
** Unlock an fs-file.
*/
static цяло fsUnlock(sqlite3_file *pFile, цяло eLock){
  върни SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on an fs-file.
*/
static цяло fsCheckReservedLock(sqlite3_file *pFile, цяло *pResOut){
  *pResOut = 0;
  върни SQLITE_OK;
}

/*
** File control method. For custom operations on an fs-file.
*/
static цяло fsFileControl(sqlite3_file *pFile, цяло op, void *pArg){
  ако( op==SQLITE_FCNTL_PRAGMA ) върни SQLITE_NOTFOUND;
  върни SQLITE_OK;
}

/*
** Return the sector-size in bytes for an fs-file.
*/
static цяло fsSectorSize(sqlite3_file *pFile){
  върни BLOCKSIZE;
}

/*
** Return the device characteristic flags supported by an fs-file.
*/
static цяло fsDeviceCharacteristics(sqlite3_file *pFile){
  върни 0;
}

/*
** Open an fs file handle.
*/
static цяло fsOpen(
  sqlite3_vfs *pVfs,
  const char *zName,
  sqlite3_file *pFile,
  цяло flags,
  цяло *pOutFlags
){
  fs_vfs_t *pFsVfs = (fs_vfs_t *)pVfs;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = 0;
  цяло eType;
  цяло nName;
  цяло rc = SQLITE_OK;

  ако( 0==(flags&(SQLITE_OPEN_MAIN_DB|SQLITE_OPEN_MAIN_JOURNAL)) ){
    tmp_file *p2 = (tmp_file *)pFile;
    memset(p2, 0, sizeof(*p2));
    p2->base.pMethods = &tmp_io_methods;
    върни SQLITE_OK;
  }

  eType = ((flags&(SQLITE_OPEN_MAIN_DB))?DATABASE_FILE:JOURNAL_FILE);
  p->base.pMethods = &fs_io_methods;
  p->eType = eType;

  assert(strlen("-journal")==8);
  nName = (цяло)strlen(zName)-((eType==JOURNAL_FILE)?8:0);
  pReal=pFsVfs->pFileList; 
  за(; pReal && strncmp(pReal->zName, zName, nName); pReal=pReal->pNext);

  ако( !pReal ){
    цяло real_flags = (flags&~(SQLITE_OPEN_MAIN_DB))|SQLITE_OPEN_TEMP_DB;
    sqlite3_int64 size;
    sqlite3_file *pRealFile;
    sqlite3_vfs *pParent = pFsVfs->pParent;
    assert(eType==DATABASE_FILE);

    pReal = (fs_real_file *)sqlite3_malloc(sizeof(*pReal)+pParent->szOsFile);
    ако( !pReal ){
      rc = SQLITE_NOMEM;
      goto open_out;
    }
    memset(pReal, 0, sizeof(*pReal)+pParent->szOsFile);
    pReal->zName = zName;
    pReal->pFile = (sqlite3_file *)(&pReal[1]);

    rc = pParent->xOpen(pParent, zName, pReal->pFile, real_flags, pOutFlags);
    ако( rc!=SQLITE_OK ){
      goto open_out;
    }
    pRealFile = pReal->pFile;

    rc = pRealFile->pMethods->xFileSize(pRealFile, &size);
    ако( rc!=SQLITE_OK ){
      goto open_out;
    }
    ако( size==0 ){
      rc = pRealFile->pMethods->xWrite(pRealFile, "\0", 1, BLOBSIZE-1);
      pReal->nBlob = BLOBSIZE;
    }иначе{
      unsigned char zS[4];
      pReal->nBlob = (цяло)size;
      rc = pRealFile->pMethods->xRead(pRealFile, zS, 4, 0);
      pReal->nDatabase = (zS[0]<<24)+(zS[1]<<16)+(zS[2]<<8)+zS[3];
      ако( rc==SQLITE_OK ){
        rc = pRealFile->pMethods->xRead(pRealFile, zS, 4, pReal->nBlob-4);
        ако( zS[0] || zS[1] || zS[2] || zS[3] ){
          pReal->nJournal = pReal->nBlob;
        }
      }
    }

    ако( rc==SQLITE_OK ){
      pReal->pNext = pFsVfs->pFileList;
      ако( pReal->pNext ){
        pReal->pNext->ppThis = &pReal->pNext;
      }
      pReal->ppThis = &pFsVfs->pFileList;
      pFsVfs->pFileList = pReal;
    }
  }

open_out:
  ако( pReal ){
    ако( rc==SQLITE_OK ){
      p->pReal = pReal;
      pReal->nRef++;
    }иначе{
      ако( pReal->pFile->pMethods ){
        pReal->pFile->pMethods->xClose(pReal->pFile);
      }
      sqlite3_free(pReal);
    }
  }
  върни rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
static цяло fsDelete(sqlite3_vfs *pVfs, const char *zPath, цяло dirSync){
  цяло rc = SQLITE_OK;
  fs_vfs_t *pFsVfs = (fs_vfs_t *)pVfs;
  fs_real_file *pReal;
  sqlite3_file *pF;
  цяло nName = (цяло)strlen(zPath) - 8;

  assert(strlen("-journal")==8);
  assert(strcmp("-journal", &zPath[nName])==0);

  pReal = pFsVfs->pFileList; 
  за(; pReal && strncmp(pReal->zName, zPath, nName); pReal=pReal->pNext);
  ако( pReal ){
    pF = pReal->pFile;
    rc = pF->pMethods->xWrite(pF, "\0\0\0\0", 4, pReal->nBlob-BLOCKSIZE);
    ако( rc==SQLITE_OK ){
      pReal->nJournal = 0;
    }
  }
  върни rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
static цяло fsAccess(
  sqlite3_vfs *pVfs, 
  const char *zPath, 
  цяло flags, 
  цяло *pResOut
){
  fs_vfs_t *pFsVfs = (fs_vfs_t *)pVfs;
  fs_real_file *pReal;
  цяло isJournal = 0;
  цяло nName = (цяло)strlen(zPath);

  ако( flags!=SQLITE_ACCESS_EXISTS ){
    sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
    върни pParent->xAccess(pParent, zPath, flags, pResOut);
  }

  assert(strlen("-journal")==8);
  ако( nName>8 && strcmp("-journal", &zPath[nName-8])==0 ){
    nName -= 8;
    isJournal = 1;
  }

  pReal = pFsVfs->pFileList; 
  за(; pReal && strncmp(pReal->zName, zPath, nName); pReal=pReal->pNext);

  *pResOut = (pReal && (!isJournal || pReal->nJournal>0));
  върни SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (FS_MAX_PATHNAME+1) bytes.
*/
static цяло fsFullPathname(
  sqlite3_vfs *pVfs,            /* Pointer to vfs object */
  const char *zPath,            /* Possibly relative input path */
  цяло nOut,                     /* Size of output buffer in bytes */
  char *zOut                    /* Output buffer */
){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  върни pParent->xFullPathname(pParent, zPath, nOut, zOut);
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
static void *fsDlOpen(sqlite3_vfs *pVfs, const char *zPath){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  върни pParent->xDlOpen(pParent, zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
static void fsDlError(sqlite3_vfs *pVfs, цяло nByte, char *zErrMsg){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  pParent->xDlError(pParent, nByte, zErrMsg);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
static void (*fsDlSym(sqlite3_vfs *pVfs, void *pH, const char *zSym))(void){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  върни pParent->xDlSym(pParent, pH, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
static void fsDlClose(sqlite3_vfs *pVfs, void *pHandle){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  pParent->xDlClose(pParent, pHandle);
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
static цяло fsRandomness(sqlite3_vfs *pVfs, цяло nByte, char *zBufOut){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  върни pParent->xRandomness(pParent, nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
static цяло fsSleep(sqlite3_vfs *pVfs, цяло nMicro){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  върни pParent->xSleep(pParent, nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
static цяло fsCurrentTime(sqlite3_vfs *pVfs, double *pTimeOut){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  върни pParent->xCurrentTime(pParent, pTimeOut);
}

/*
** This procedure registers the fs vfs with SQLite. If the argument is
** true, the fs vfs becomes the new default vfs. It is the only publicly
** available function in this file.
*/
цяло fs_register(void){
  ако( fs_vfs.pParent ) върни SQLITE_OK;
  fs_vfs.pParent = sqlite3_vfs_find(0);
  fs_vfs.base.mxPathname = fs_vfs.pParent->mxPathname;
  fs_vfs.base.szOsFile = MAX(sizeof(tmp_file), sizeof(fs_file));
  върни sqlite3_vfs_register(&fs_vfs.base, 0);
}

#ifdef SQLITE_TEST
  цяло SqlitetestOnefile_Init() {върни fs_register();}
#endif
