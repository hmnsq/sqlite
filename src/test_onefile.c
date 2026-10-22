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
  ціле nDatabase;              /* Current size of database region */
  ціле nJournal;               /* Current size of journal region */
  ціле nBlob;                  /* Total size of allocated blob */
  ціле nRef;                   /* Number of pointers to this structure */
  fs_real_file *pNext;
  fs_real_file **ppThis;
};

typedef struct fs_file fs_file;
struct fs_file {
  sqlite3_file base;
  ціле eType;
  fs_real_file *pReal;
};

typedef struct tmp_file tmp_file;
struct tmp_file {
  sqlite3_file base;
  ціле nSize;
  ціле nAlloc;
  char *zAlloc;
};

/* Values for fs_file.eType. */
#define DATABASE_FILE   1
#define JOURNAL_FILE    2

/*
** Method declarations for fs_file.
*/
static ціле fsClose(sqlite3_file*);
static ціле fsRead(sqlite3_file*, void*, ціле iAmt, sqlite3_int64 iOfst);
static ціле fsWrite(sqlite3_file*, const void*, ціле iAmt, sqlite3_int64 iOfst);
static ціле fsTruncate(sqlite3_file*, sqlite3_int64 size);
static ціле fsSync(sqlite3_file*, ціле flags);
static ціле fsFileSize(sqlite3_file*, sqlite3_int64 *pSize);
static ціле fsLock(sqlite3_file*, ціле);
static ціле fsUnlock(sqlite3_file*, ціле);
static ціле fsCheckReservedLock(sqlite3_file*, ціле *pResOut);
static ціле fsFileControl(sqlite3_file*, ціле op, void *pArg);
static ціле fsSectorSize(sqlite3_file*);
static ціле fsDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for tmp_file.
*/
static ціле tmpClose(sqlite3_file*);
static ціле tmpRead(sqlite3_file*, void*, ціле iAmt, sqlite3_int64 iOfst);
static ціле tmpWrite(sqlite3_file*, const void*, ціле iAmt, sqlite3_int64 iOfst);
static ціле tmpTruncate(sqlite3_file*, sqlite3_int64 size);
static ціле tmpSync(sqlite3_file*, ціле flags);
static ціле tmpFileSize(sqlite3_file*, sqlite3_int64 *pSize);
static ціле tmpLock(sqlite3_file*, ціле);
static ціле tmpUnlock(sqlite3_file*, ціле);
static ціле tmpCheckReservedLock(sqlite3_file*, ціле *pResOut);
static ціле tmpFileControl(sqlite3_file*, ціле op, void *pArg);
static ціле tmpSectorSize(sqlite3_file*);
static ціле tmpDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for fs_vfs.
*/
static ціле fsOpen(sqlite3_vfs*, const char *, sqlite3_file*, ціле , ціле *);
static ціле fsDelete(sqlite3_vfs*, const char *zName, ціле syncDir);
static ціле fsAccess(sqlite3_vfs*, const char *zName, ціле flags, ціле *);
static ціле fsFullPathname(sqlite3_vfs*, const char *zName, ціле nOut,char *zOut);
static void *fsDlOpen(sqlite3_vfs*, const char *zFilename);
static void fsDlError(sqlite3_vfs*, ціле nByte, char *zErrMsg);
static void (*fsDlSym(sqlite3_vfs*,void*, const char *zSymbol))(void);
static void fsDlClose(sqlite3_vfs*, void*);
static ціле fsRandomness(sqlite3_vfs*, ціле nByte, char *zOut);
static ціле fsSleep(sqlite3_vfs*, ціле microseconds);
static ціле fsCurrentTime(sqlite3_vfs*, double*);


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
static ціле tmpClose(sqlite3_file *pFile){
  tmp_file *pTmp = (tmp_file *)pFile;
  sqlite3_free(pTmp->zAlloc);
  повернути SQLITE_OK;
}

/*
** Read data from a tmp-file.
*/
static ціле tmpRead(
  sqlite3_file *pFile, 
  void *zBuf, 
  ціле iAmt, 
  sqlite_int64 iOfst
){
  tmp_file *pTmp = (tmp_file *)pFile;
  якщо( (iAmt+iOfst)>pTmp->nSize ){
    повернути SQLITE_IOERR_SHORT_READ;
  }
  memcpy(zBuf, &pTmp->zAlloc[iOfst], iAmt);
  повернути SQLITE_OK;
}

/*
** Write data to a tmp-file.
*/
static ціле tmpWrite(
  sqlite3_file *pFile, 
  const void *zBuf, 
  ціле iAmt, 
  sqlite_int64 iOfst
){
  tmp_file *pTmp = (tmp_file *)pFile;
  якщо( (iAmt+iOfst)>pTmp->nAlloc ){
    ціле nNew = (ціле)(2*(iAmt+iOfst+pTmp->nAlloc));
    char *zNew = sqlite3_realloc(pTmp->zAlloc, nNew);
    якщо( !zNew ){
      повернути SQLITE_NOMEM;
    }
    pTmp->zAlloc = zNew;
    pTmp->nAlloc = nNew;
  }
  memcpy(&pTmp->zAlloc[iOfst], zBuf, iAmt);
  pTmp->nSize = (ціле)MAX(pTmp->nSize, iOfst+iAmt);
  повернути SQLITE_OK;
}

/*
** Truncate a tmp-file.
*/
static ціле tmpTruncate(sqlite3_file *pFile, sqlite_int64 size){
  tmp_file *pTmp = (tmp_file *)pFile;
  pTmp->nSize = (ціле)MIN(pTmp->nSize, size);
  повернути SQLITE_OK;
}

/*
** Sync a tmp-file.
*/
static ціле tmpSync(sqlite3_file *pFile, ціле flags){
  повернути SQLITE_OK;
}

/*
** Return the current file-size of a tmp-file.
*/
static ціле tmpFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  tmp_file *pTmp = (tmp_file *)pFile;
  *pSize = pTmp->nSize;
  повернути SQLITE_OK;
}

/*
** Lock a tmp-file.
*/
static ціле tmpLock(sqlite3_file *pFile, ціле eLock){
  повернути SQLITE_OK;
}

/*
** Unlock a tmp-file.
*/
static ціле tmpUnlock(sqlite3_file *pFile, ціле eLock){
  повернути SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on a tmp-file.
*/
static ціле tmpCheckReservedLock(sqlite3_file *pFile, ціле *pResOut){
  *pResOut = 0;
  повернути SQLITE_OK;
}

/*
** File control method. For custom operations on a tmp-file.
*/
static ціле tmpFileControl(sqlite3_file *pFile, ціле op, void *pArg){
  повернути SQLITE_OK;
}

/*
** Return the sector-size in bytes for a tmp-file.
*/
static ціле tmpSectorSize(sqlite3_file *pFile){
  повернути 0;
}

/*
** Return the device characteristic flags supported by a tmp-file.
*/
static ціле tmpDeviceCharacteristics(sqlite3_file *pFile){
  повернути 0;
}

/*
** Close an fs-file.
*/
static ціле fsClose(sqlite3_file *pFile){
  ціле rc = SQLITE_OK;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;

  /* Decrement the real_file ref-count. */
  pReal->nRef--;
  assert(pReal->nRef>=0);

  /* When the ref-count reaches 0, destroy the structure */
  якщо( pReal->nRef==0 ){
    *pReal->ppThis = pReal->pNext;
    якщо( pReal->pNext ){
      pReal->pNext->ppThis = pReal->ppThis;
    }
    rc = pReal->pFile->pMethods->xClose(pReal->pFile);
    sqlite3_free(pReal);
  }

  повернути rc;
}

/*
** Read data from an fs-file.
*/
static ціле fsRead(
  sqlite3_file *pFile, 
  void *zBuf, 
  ціле iAmt, 
  sqlite_int64 iOfst
){
  ціле rc = SQLITE_OK;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  sqlite3_file *pF = pReal->pFile;

  якщо( (p->eType==DATABASE_FILE && (iAmt+iOfst)>pReal->nDatabase)
   || (p->eType==JOURNAL_FILE && (iAmt+iOfst)>pReal->nJournal)
  ){
    rc = SQLITE_IOERR_SHORT_READ;
  }інакше якщо( p->eType==DATABASE_FILE ){
    rc = pF->pMethods->xRead(pF, zBuf, iAmt, iOfst+BLOCKSIZE);
  }інакше{
    /* Journal file. */
    ціле iRem = iAmt;
    ціле iBuf = 0;
    ціле ii = (ціле)iOfst;
    поки( iRem>0 && rc==SQLITE_OK ){
      ціле iRealOff = pReal->nBlob - BLOCKSIZE*((ii/BLOCKSIZE)+1) + ii%BLOCKSIZE;
      ціле iRealAmt = MIN(iRem, BLOCKSIZE - (iRealOff%BLOCKSIZE));

      rc = pF->pMethods->xRead(pF, &((char *)zBuf)[iBuf], iRealAmt, iRealOff);
      ii += iRealAmt;
      iBuf += iRealAmt;
      iRem -= iRealAmt;
    }
  }

  повернути rc;
}

/*
** Write data to an fs-file.
*/
static ціле fsWrite(
  sqlite3_file *pFile, 
  const void *zBuf, 
  ціле iAmt, 
  sqlite_int64 iOfst
){
  ціле rc = SQLITE_OK;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  sqlite3_file *pF = pReal->pFile;

  якщо( p->eType==DATABASE_FILE ){
    якщо( (iAmt+iOfst+BLOCKSIZE)>(pReal->nBlob-pReal->nJournal) ){
      rc = SQLITE_FULL;
    }інакше{
      rc = pF->pMethods->xWrite(pF, zBuf, iAmt, iOfst+BLOCKSIZE);
      якщо( rc==SQLITE_OK ){
        pReal->nDatabase = (ціле)MAX(pReal->nDatabase, iAmt+iOfst);
      }
    }
  }інакше{
    /* Journal file. */
    ціле iRem = iAmt;
    ціле iBuf = 0;
    ціле ii = (ціле)iOfst;
    поки( iRem>0 && rc==SQLITE_OK ){
      ціле iRealOff = pReal->nBlob - BLOCKSIZE*((ii/BLOCKSIZE)+1) + ii%BLOCKSIZE;
      ціле iRealAmt = MIN(iRem, BLOCKSIZE - (iRealOff%BLOCKSIZE));

      якщо( iRealOff<(pReal->nDatabase+BLOCKSIZE) ){
        rc = SQLITE_FULL;
      }інакше{
        rc = pF->pMethods->xWrite(pF, &((char *)zBuf)[iBuf], iRealAmt,iRealOff);
        ii += iRealAmt;
        iBuf += iRealAmt;
        iRem -= iRealAmt;
      }
    }
    якщо( rc==SQLITE_OK ){
      pReal->nJournal = (ціле)MAX(pReal->nJournal, iAmt+iOfst);
    }
  }

  повернути rc;
}

/*
** Truncate an fs-file.
*/
static ціле fsTruncate(sqlite3_file *pFile, sqlite_int64 size){
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  якщо( p->eType==DATABASE_FILE ){
    pReal->nDatabase = (ціле)MIN(pReal->nDatabase, size);
  }інакше{
    pReal->nJournal = (ціле)MIN(pReal->nJournal, size);
  }
  повернути SQLITE_OK;
}

/*
** Sync an fs-file.
*/
static ціле fsSync(sqlite3_file *pFile, ціле flags){
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  sqlite3_file *pRealFile = pReal->pFile;
  ціле rc = SQLITE_OK;

  якщо( p->eType==DATABASE_FILE ){
    unsigned char zSize[4];
    zSize[0] = (pReal->nDatabase&0xFF000000)>>24;
    zSize[1] = (unsigned char)((pReal->nDatabase&0x00FF0000)>>16);
    zSize[2] = (pReal->nDatabase&0x0000FF00)>>8;
    zSize[3] = (pReal->nDatabase&0x000000FF);
    rc = pRealFile->pMethods->xWrite(pRealFile, zSize, 4, 0);
  }
  якщо( rc==SQLITE_OK ){
    rc = pRealFile->pMethods->xSync(pRealFile, flags&(~SQLITE_SYNC_DATAONLY));
  }

  повернути rc;
}

/*
** Return the current file-size of an fs-file.
*/
static ціле fsFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  якщо( p->eType==DATABASE_FILE ){
    *pSize = pReal->nDatabase;
  }інакше{
    *pSize = pReal->nJournal;
  }
  повернути SQLITE_OK;
}

/*
** Lock an fs-file.
*/
static ціле fsLock(sqlite3_file *pFile, ціле eLock){
  повернути SQLITE_OK;
}

/*
** Unlock an fs-file.
*/
static ціле fsUnlock(sqlite3_file *pFile, ціле eLock){
  повернути SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on an fs-file.
*/
static ціле fsCheckReservedLock(sqlite3_file *pFile, ціле *pResOut){
  *pResOut = 0;
  повернути SQLITE_OK;
}

/*
** File control method. For custom operations on an fs-file.
*/
static ціле fsFileControl(sqlite3_file *pFile, ціле op, void *pArg){
  якщо( op==SQLITE_FCNTL_PRAGMA ) повернути SQLITE_NOTFOUND;
  повернути SQLITE_OK;
}

/*
** Return the sector-size in bytes for an fs-file.
*/
static ціле fsSectorSize(sqlite3_file *pFile){
  повернути BLOCKSIZE;
}

/*
** Return the device characteristic flags supported by an fs-file.
*/
static ціле fsDeviceCharacteristics(sqlite3_file *pFile){
  повернути 0;
}

/*
** Open an fs file handle.
*/
static ціле fsOpen(
  sqlite3_vfs *pVfs,
  const char *zName,
  sqlite3_file *pFile,
  ціле flags,
  ціле *pOutFlags
){
  fs_vfs_t *pFsVfs = (fs_vfs_t *)pVfs;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = 0;
  ціле eType;
  ціле nName;
  ціле rc = SQLITE_OK;

  якщо( 0==(flags&(SQLITE_OPEN_MAIN_DB|SQLITE_OPEN_MAIN_JOURNAL)) ){
    tmp_file *p2 = (tmp_file *)pFile;
    memset(p2, 0, sizeof(*p2));
    p2->base.pMethods = &tmp_io_methods;
    повернути SQLITE_OK;
  }

  eType = ((flags&(SQLITE_OPEN_MAIN_DB))?DATABASE_FILE:JOURNAL_FILE);
  p->base.pMethods = &fs_io_methods;
  p->eType = eType;

  assert(strlen("-journal")==8);
  nName = (ціле)strlen(zName)-((eType==JOURNAL_FILE)?8:0);
  pReal=pFsVfs->pFileList; 
  для(; pReal && strncmp(pReal->zName, zName, nName); pReal=pReal->pNext);

  якщо( !pReal ){
    ціле real_flags = (flags&~(SQLITE_OPEN_MAIN_DB))|SQLITE_OPEN_TEMP_DB;
    sqlite3_int64 size;
    sqlite3_file *pRealFile;
    sqlite3_vfs *pParent = pFsVfs->pParent;
    assert(eType==DATABASE_FILE);

    pReal = (fs_real_file *)sqlite3_malloc(sizeof(*pReal)+pParent->szOsFile);
    якщо( !pReal ){
      rc = SQLITE_NOMEM;
      goto open_out;
    }
    memset(pReal, 0, sizeof(*pReal)+pParent->szOsFile);
    pReal->zName = zName;
    pReal->pFile = (sqlite3_file *)(&pReal[1]);

    rc = pParent->xOpen(pParent, zName, pReal->pFile, real_flags, pOutFlags);
    якщо( rc!=SQLITE_OK ){
      goto open_out;
    }
    pRealFile = pReal->pFile;

    rc = pRealFile->pMethods->xFileSize(pRealFile, &size);
    якщо( rc!=SQLITE_OK ){
      goto open_out;
    }
    якщо( size==0 ){
      rc = pRealFile->pMethods->xWrite(pRealFile, "\0", 1, BLOBSIZE-1);
      pReal->nBlob = BLOBSIZE;
    }інакше{
      unsigned char zS[4];
      pReal->nBlob = (ціле)size;
      rc = pRealFile->pMethods->xRead(pRealFile, zS, 4, 0);
      pReal->nDatabase = (zS[0]<<24)+(zS[1]<<16)+(zS[2]<<8)+zS[3];
      якщо( rc==SQLITE_OK ){
        rc = pRealFile->pMethods->xRead(pRealFile, zS, 4, pReal->nBlob-4);
        якщо( zS[0] || zS[1] || zS[2] || zS[3] ){
          pReal->nJournal = pReal->nBlob;
        }
      }
    }

    якщо( rc==SQLITE_OK ){
      pReal->pNext = pFsVfs->pFileList;
      якщо( pReal->pNext ){
        pReal->pNext->ppThis = &pReal->pNext;
      }
      pReal->ppThis = &pFsVfs->pFileList;
      pFsVfs->pFileList = pReal;
    }
  }

open_out:
  якщо( pReal ){
    якщо( rc==SQLITE_OK ){
      p->pReal = pReal;
      pReal->nRef++;
    }інакше{
      якщо( pReal->pFile->pMethods ){
        pReal->pFile->pMethods->xClose(pReal->pFile);
      }
      sqlite3_free(pReal);
    }
  }
  повернути rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
static ціле fsDelete(sqlite3_vfs *pVfs, const char *zPath, ціле dirSync){
  ціле rc = SQLITE_OK;
  fs_vfs_t *pFsVfs = (fs_vfs_t *)pVfs;
  fs_real_file *pReal;
  sqlite3_file *pF;
  ціле nName = (ціле)strlen(zPath) - 8;

  assert(strlen("-journal")==8);
  assert(strcmp("-journal", &zPath[nName])==0);

  pReal = pFsVfs->pFileList; 
  для(; pReal && strncmp(pReal->zName, zPath, nName); pReal=pReal->pNext);
  якщо( pReal ){
    pF = pReal->pFile;
    rc = pF->pMethods->xWrite(pF, "\0\0\0\0", 4, pReal->nBlob-BLOCKSIZE);
    якщо( rc==SQLITE_OK ){
      pReal->nJournal = 0;
    }
  }
  повернути rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
static ціле fsAccess(
  sqlite3_vfs *pVfs, 
  const char *zPath, 
  ціле flags, 
  ціле *pResOut
){
  fs_vfs_t *pFsVfs = (fs_vfs_t *)pVfs;
  fs_real_file *pReal;
  ціле isJournal = 0;
  ціле nName = (ціле)strlen(zPath);

  якщо( flags!=SQLITE_ACCESS_EXISTS ){
    sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
    повернути pParent->xAccess(pParent, zPath, flags, pResOut);
  }

  assert(strlen("-journal")==8);
  якщо( nName>8 && strcmp("-journal", &zPath[nName-8])==0 ){
    nName -= 8;
    isJournal = 1;
  }

  pReal = pFsVfs->pFileList; 
  для(; pReal && strncmp(pReal->zName, zPath, nName); pReal=pReal->pNext);

  *pResOut = (pReal && (!isJournal || pReal->nJournal>0));
  повернути SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (FS_MAX_PATHNAME+1) bytes.
*/
static ціле fsFullPathname(
  sqlite3_vfs *pVfs,            /* Pointer to vfs object */
  const char *zPath,            /* Possibly relative input path */
  ціле nOut,                     /* Size of output buffer in bytes */
  char *zOut                    /* Output buffer */
){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  повернути pParent->xFullPathname(pParent, zPath, nOut, zOut);
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
static void *fsDlOpen(sqlite3_vfs *pVfs, const char *zPath){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  повернути pParent->xDlOpen(pParent, zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
static void fsDlError(sqlite3_vfs *pVfs, ціле nByte, char *zErrMsg){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  pParent->xDlError(pParent, nByte, zErrMsg);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
static void (*fsDlSym(sqlite3_vfs *pVfs, void *pH, const char *zSym))(void){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  повернути pParent->xDlSym(pParent, pH, zSym);
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
static ціле fsRandomness(sqlite3_vfs *pVfs, ціле nByte, char *zBufOut){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  повернути pParent->xRandomness(pParent, nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
static ціле fsSleep(sqlite3_vfs *pVfs, ціле nMicro){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  повернути pParent->xSleep(pParent, nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
static ціле fsCurrentTime(sqlite3_vfs *pVfs, double *pTimeOut){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  повернути pParent->xCurrentTime(pParent, pTimeOut);
}

/*
** This procedure registers the fs vfs with SQLite. If the argument is
** true, the fs vfs becomes the new default vfs. It is the only publicly
** available function in this file.
*/
ціле fs_register(void){
  якщо( fs_vfs.pParent ) повернути SQLITE_OK;
  fs_vfs.pParent = sqlite3_vfs_find(0);
  fs_vfs.base.mxPathname = fs_vfs.pParent->mxPathname;
  fs_vfs.base.szOsFile = MAX(sizeof(tmp_file), sizeof(fs_file));
  повернути sqlite3_vfs_register(&fs_vfs.base, 0);
}

#ifdef SQLITE_TEST
  ціле SqlitetestOnefile_Init() {повернути fs_register();}
#endif
