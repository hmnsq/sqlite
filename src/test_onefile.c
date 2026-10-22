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
  整数 nDatabase;              /* Current size of database region */
  整数 nJournal;               /* Current size of journal region */
  整数 nBlob;                  /* Total size of allocated blob */
  整数 nRef;                   /* Number of pointers to this structure */
  fs_real_file *pNext;
  fs_real_file **ppThis;
};

typedef struct fs_file fs_file;
struct fs_file {
  sqlite3_file base;
  整数 eType;
  fs_real_file *pReal;
};

typedef struct tmp_file tmp_file;
struct tmp_file {
  sqlite3_file base;
  整数 nSize;
  整数 nAlloc;
  char *zAlloc;
};

/* Values for fs_file.eType. */
#define DATABASE_FILE   1
#define JOURNAL_FILE    2

/*
** Method declarations for fs_file.
*/
static 整数 fsClose(sqlite3_file*);
static 整数 fsRead(sqlite3_file*, void*, 整数 iAmt, sqlite3_int64 iOfst);
static 整数 fsWrite(sqlite3_file*, const void*, 整数 iAmt, sqlite3_int64 iOfst);
static 整数 fsTruncate(sqlite3_file*, sqlite3_int64 size);
static 整数 fsSync(sqlite3_file*, 整数 flags);
static 整数 fsFileSize(sqlite3_file*, sqlite3_int64 *pSize);
static 整数 fsLock(sqlite3_file*, 整数);
static 整数 fsUnlock(sqlite3_file*, 整数);
static 整数 fsCheckReservedLock(sqlite3_file*, 整数 *pResOut);
static 整数 fsFileControl(sqlite3_file*, 整数 op, void *pArg);
static 整数 fsSectorSize(sqlite3_file*);
static 整数 fsDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for tmp_file.
*/
static 整数 tmpClose(sqlite3_file*);
static 整数 tmpRead(sqlite3_file*, void*, 整数 iAmt, sqlite3_int64 iOfst);
static 整数 tmpWrite(sqlite3_file*, const void*, 整数 iAmt, sqlite3_int64 iOfst);
static 整数 tmpTruncate(sqlite3_file*, sqlite3_int64 size);
static 整数 tmpSync(sqlite3_file*, 整数 flags);
static 整数 tmpFileSize(sqlite3_file*, sqlite3_int64 *pSize);
static 整数 tmpLock(sqlite3_file*, 整数);
static 整数 tmpUnlock(sqlite3_file*, 整数);
static 整数 tmpCheckReservedLock(sqlite3_file*, 整数 *pResOut);
static 整数 tmpFileControl(sqlite3_file*, 整数 op, void *pArg);
static 整数 tmpSectorSize(sqlite3_file*);
static 整数 tmpDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for fs_vfs.
*/
static 整数 fsOpen(sqlite3_vfs*, const char *, sqlite3_file*, 整数 , 整数 *);
static 整数 fsDelete(sqlite3_vfs*, const char *zName, 整数 syncDir);
static 整数 fsAccess(sqlite3_vfs*, const char *zName, 整数 flags, 整数 *);
static 整数 fsFullPathname(sqlite3_vfs*, const char *zName, 整数 nOut,char *zOut);
static void *fsDlOpen(sqlite3_vfs*, const char *zFilename);
static void fsDlError(sqlite3_vfs*, 整数 nByte, char *zErrMsg);
static void (*fsDlSym(sqlite3_vfs*,void*, const char *zSymbol))(void);
static void fsDlClose(sqlite3_vfs*, void*);
static 整数 fsRandomness(sqlite3_vfs*, 整数 nByte, char *zOut);
static 整数 fsSleep(sqlite3_vfs*, 整数 microseconds);
static 整数 fsCurrentTime(sqlite3_vfs*, double*);


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
static 整数 tmpClose(sqlite3_file *pFile){
  tmp_file *pTmp = (tmp_file *)pFile;
  sqlite3_free(pTmp->zAlloc);
  戻る SQLITE_OK;
}

/*
** Read data from a tmp-file.
*/
static 整数 tmpRead(
  sqlite3_file *pFile, 
  void *zBuf, 
  整数 iAmt, 
  sqlite_int64 iOfst
){
  tmp_file *pTmp = (tmp_file *)pFile;
  もし( (iAmt+iOfst)>pTmp->nSize ){
    戻る SQLITE_IOERR_SHORT_READ;
  }
  memcpy(zBuf, &pTmp->zAlloc[iOfst], iAmt);
  戻る SQLITE_OK;
}

/*
** Write data to a tmp-file.
*/
static 整数 tmpWrite(
  sqlite3_file *pFile, 
  const void *zBuf, 
  整数 iAmt, 
  sqlite_int64 iOfst
){
  tmp_file *pTmp = (tmp_file *)pFile;
  もし( (iAmt+iOfst)>pTmp->nAlloc ){
    整数 nNew = (整数)(2*(iAmt+iOfst+pTmp->nAlloc));
    char *zNew = sqlite3_realloc(pTmp->zAlloc, nNew);
    もし( !zNew ){
      戻る SQLITE_NOMEM;
    }
    pTmp->zAlloc = zNew;
    pTmp->nAlloc = nNew;
  }
  memcpy(&pTmp->zAlloc[iOfst], zBuf, iAmt);
  pTmp->nSize = (整数)MAX(pTmp->nSize, iOfst+iAmt);
  戻る SQLITE_OK;
}

/*
** Truncate a tmp-file.
*/
static 整数 tmpTruncate(sqlite3_file *pFile, sqlite_int64 size){
  tmp_file *pTmp = (tmp_file *)pFile;
  pTmp->nSize = (整数)MIN(pTmp->nSize, size);
  戻る SQLITE_OK;
}

/*
** Sync a tmp-file.
*/
static 整数 tmpSync(sqlite3_file *pFile, 整数 flags){
  戻る SQLITE_OK;
}

/*
** Return the current file-size of a tmp-file.
*/
static 整数 tmpFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  tmp_file *pTmp = (tmp_file *)pFile;
  *pSize = pTmp->nSize;
  戻る SQLITE_OK;
}

/*
** Lock a tmp-file.
*/
static 整数 tmpLock(sqlite3_file *pFile, 整数 eLock){
  戻る SQLITE_OK;
}

/*
** Unlock a tmp-file.
*/
static 整数 tmpUnlock(sqlite3_file *pFile, 整数 eLock){
  戻る SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on a tmp-file.
*/
static 整数 tmpCheckReservedLock(sqlite3_file *pFile, 整数 *pResOut){
  *pResOut = 0;
  戻る SQLITE_OK;
}

/*
** File control method. For custom operations on a tmp-file.
*/
static 整数 tmpFileControl(sqlite3_file *pFile, 整数 op, void *pArg){
  戻る SQLITE_OK;
}

/*
** Return the sector-size in bytes for a tmp-file.
*/
static 整数 tmpSectorSize(sqlite3_file *pFile){
  戻る 0;
}

/*
** Return the device characteristic flags supported by a tmp-file.
*/
static 整数 tmpDeviceCharacteristics(sqlite3_file *pFile){
  戻る 0;
}

/*
** Close an fs-file.
*/
static 整数 fsClose(sqlite3_file *pFile){
  整数 rc = SQLITE_OK;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;

  /* Decrement the real_file ref-count. */
  pReal->nRef--;
  assert(pReal->nRef>=0);

  /* When the ref-count reaches 0, destroy the structure */
  もし( pReal->nRef==0 ){
    *pReal->ppThis = pReal->pNext;
    もし( pReal->pNext ){
      pReal->pNext->ppThis = pReal->ppThis;
    }
    rc = pReal->pFile->pMethods->xClose(pReal->pFile);
    sqlite3_free(pReal);
  }

  戻る rc;
}

/*
** Read data from an fs-file.
*/
static 整数 fsRead(
  sqlite3_file *pFile, 
  void *zBuf, 
  整数 iAmt, 
  sqlite_int64 iOfst
){
  整数 rc = SQLITE_OK;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  sqlite3_file *pF = pReal->pFile;

  もし( (p->eType==DATABASE_FILE && (iAmt+iOfst)>pReal->nDatabase)
   || (p->eType==JOURNAL_FILE && (iAmt+iOfst)>pReal->nJournal)
  ){
    rc = SQLITE_IOERR_SHORT_READ;
  }否则 もし( p->eType==DATABASE_FILE ){
    rc = pF->pMethods->xRead(pF, zBuf, iAmt, iOfst+BLOCKSIZE);
  }否则{
    /* Journal file. */
    整数 iRem = iAmt;
    整数 iBuf = 0;
    整数 ii = (整数)iOfst;
    ながら( iRem>0 && rc==SQLITE_OK ){
      整数 iRealOff = pReal->nBlob - BLOCKSIZE*((ii/BLOCKSIZE)+1) + ii%BLOCKSIZE;
      整数 iRealAmt = MIN(iRem, BLOCKSIZE - (iRealOff%BLOCKSIZE));

      rc = pF->pMethods->xRead(pF, &((char *)zBuf)[iBuf], iRealAmt, iRealOff);
      ii += iRealAmt;
      iBuf += iRealAmt;
      iRem -= iRealAmt;
    }
  }

  戻る rc;
}

/*
** Write data to an fs-file.
*/
static 整数 fsWrite(
  sqlite3_file *pFile, 
  const void *zBuf, 
  整数 iAmt, 
  sqlite_int64 iOfst
){
  整数 rc = SQLITE_OK;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  sqlite3_file *pF = pReal->pFile;

  もし( p->eType==DATABASE_FILE ){
    もし( (iAmt+iOfst+BLOCKSIZE)>(pReal->nBlob-pReal->nJournal) ){
      rc = SQLITE_FULL;
    }否则{
      rc = pF->pMethods->xWrite(pF, zBuf, iAmt, iOfst+BLOCKSIZE);
      もし( rc==SQLITE_OK ){
        pReal->nDatabase = (整数)MAX(pReal->nDatabase, iAmt+iOfst);
      }
    }
  }否则{
    /* Journal file. */
    整数 iRem = iAmt;
    整数 iBuf = 0;
    整数 ii = (整数)iOfst;
    ながら( iRem>0 && rc==SQLITE_OK ){
      整数 iRealOff = pReal->nBlob - BLOCKSIZE*((ii/BLOCKSIZE)+1) + ii%BLOCKSIZE;
      整数 iRealAmt = MIN(iRem, BLOCKSIZE - (iRealOff%BLOCKSIZE));

      もし( iRealOff<(pReal->nDatabase+BLOCKSIZE) ){
        rc = SQLITE_FULL;
      }否则{
        rc = pF->pMethods->xWrite(pF, &((char *)zBuf)[iBuf], iRealAmt,iRealOff);
        ii += iRealAmt;
        iBuf += iRealAmt;
        iRem -= iRealAmt;
      }
    }
    もし( rc==SQLITE_OK ){
      pReal->nJournal = (整数)MAX(pReal->nJournal, iAmt+iOfst);
    }
  }

  戻る rc;
}

/*
** Truncate an fs-file.
*/
static 整数 fsTruncate(sqlite3_file *pFile, sqlite_int64 size){
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  もし( p->eType==DATABASE_FILE ){
    pReal->nDatabase = (整数)MIN(pReal->nDatabase, size);
  }否则{
    pReal->nJournal = (整数)MIN(pReal->nJournal, size);
  }
  戻る SQLITE_OK;
}

/*
** Sync an fs-file.
*/
static 整数 fsSync(sqlite3_file *pFile, 整数 flags){
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  sqlite3_file *pRealFile = pReal->pFile;
  整数 rc = SQLITE_OK;

  もし( p->eType==DATABASE_FILE ){
    unsigned char zSize[4];
    zSize[0] = (pReal->nDatabase&0xFF000000)>>24;
    zSize[1] = (unsigned char)((pReal->nDatabase&0x00FF0000)>>16);
    zSize[2] = (pReal->nDatabase&0x0000FF00)>>8;
    zSize[3] = (pReal->nDatabase&0x000000FF);
    rc = pRealFile->pMethods->xWrite(pRealFile, zSize, 4, 0);
  }
  もし( rc==SQLITE_OK ){
    rc = pRealFile->pMethods->xSync(pRealFile, flags&(~SQLITE_SYNC_DATAONLY));
  }

  戻る rc;
}

/*
** Return the current file-size of an fs-file.
*/
static 整数 fsFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  もし( p->eType==DATABASE_FILE ){
    *pSize = pReal->nDatabase;
  }否则{
    *pSize = pReal->nJournal;
  }
  戻る SQLITE_OK;
}

/*
** Lock an fs-file.
*/
static 整数 fsLock(sqlite3_file *pFile, 整数 eLock){
  戻る SQLITE_OK;
}

/*
** Unlock an fs-file.
*/
static 整数 fsUnlock(sqlite3_file *pFile, 整数 eLock){
  戻る SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on an fs-file.
*/
static 整数 fsCheckReservedLock(sqlite3_file *pFile, 整数 *pResOut){
  *pResOut = 0;
  戻る SQLITE_OK;
}

/*
** File control method. For custom operations on an fs-file.
*/
static 整数 fsFileControl(sqlite3_file *pFile, 整数 op, void *pArg){
  もし( op==SQLITE_FCNTL_PRAGMA ) 戻る SQLITE_NOTFOUND;
  戻る SQLITE_OK;
}

/*
** Return the sector-size in bytes for an fs-file.
*/
static 整数 fsSectorSize(sqlite3_file *pFile){
  戻る BLOCKSIZE;
}

/*
** Return the device characteristic flags supported by an fs-file.
*/
static 整数 fsDeviceCharacteristics(sqlite3_file *pFile){
  戻る 0;
}

/*
** Open an fs file handle.
*/
static 整数 fsOpen(
  sqlite3_vfs *pVfs,
  const char *zName,
  sqlite3_file *pFile,
  整数 flags,
  整数 *pOutFlags
){
  fs_vfs_t *pFsVfs = (fs_vfs_t *)pVfs;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = 0;
  整数 eType;
  整数 nName;
  整数 rc = SQLITE_OK;

  もし( 0==(flags&(SQLITE_OPEN_MAIN_DB|SQLITE_OPEN_MAIN_JOURNAL)) ){
    tmp_file *p2 = (tmp_file *)pFile;
    memset(p2, 0, sizeof(*p2));
    p2->base.pMethods = &tmp_io_methods;
    戻る SQLITE_OK;
  }

  eType = ((flags&(SQLITE_OPEN_MAIN_DB))?DATABASE_FILE:JOURNAL_FILE);
  p->base.pMethods = &fs_io_methods;
  p->eType = eType;

  assert(strlen("-journal")==8);
  nName = (整数)strlen(zName)-((eType==JOURNAL_FILE)?8:0);
  pReal=pFsVfs->pFileList; 
  ために(; pReal && strncmp(pReal->zName, zName, nName); pReal=pReal->pNext);

  もし( !pReal ){
    整数 real_flags = (flags&~(SQLITE_OPEN_MAIN_DB))|SQLITE_OPEN_TEMP_DB;
    sqlite3_int64 size;
    sqlite3_file *pRealFile;
    sqlite3_vfs *pParent = pFsVfs->pParent;
    assert(eType==DATABASE_FILE);

    pReal = (fs_real_file *)sqlite3_malloc(sizeof(*pReal)+pParent->szOsFile);
    もし( !pReal ){
      rc = SQLITE_NOMEM;
      goto open_out;
    }
    memset(pReal, 0, sizeof(*pReal)+pParent->szOsFile);
    pReal->zName = zName;
    pReal->pFile = (sqlite3_file *)(&pReal[1]);

    rc = pParent->xOpen(pParent, zName, pReal->pFile, real_flags, pOutFlags);
    もし( rc!=SQLITE_OK ){
      goto open_out;
    }
    pRealFile = pReal->pFile;

    rc = pRealFile->pMethods->xFileSize(pRealFile, &size);
    もし( rc!=SQLITE_OK ){
      goto open_out;
    }
    もし( size==0 ){
      rc = pRealFile->pMethods->xWrite(pRealFile, "\0", 1, BLOBSIZE-1);
      pReal->nBlob = BLOBSIZE;
    }否则{
      unsigned char zS[4];
      pReal->nBlob = (整数)size;
      rc = pRealFile->pMethods->xRead(pRealFile, zS, 4, 0);
      pReal->nDatabase = (zS[0]<<24)+(zS[1]<<16)+(zS[2]<<8)+zS[3];
      もし( rc==SQLITE_OK ){
        rc = pRealFile->pMethods->xRead(pRealFile, zS, 4, pReal->nBlob-4);
        もし( zS[0] || zS[1] || zS[2] || zS[3] ){
          pReal->nJournal = pReal->nBlob;
        }
      }
    }

    もし( rc==SQLITE_OK ){
      pReal->pNext = pFsVfs->pFileList;
      もし( pReal->pNext ){
        pReal->pNext->ppThis = &pReal->pNext;
      }
      pReal->ppThis = &pFsVfs->pFileList;
      pFsVfs->pFileList = pReal;
    }
  }

open_out:
  もし( pReal ){
    もし( rc==SQLITE_OK ){
      p->pReal = pReal;
      pReal->nRef++;
    }否则{
      もし( pReal->pFile->pMethods ){
        pReal->pFile->pMethods->xClose(pReal->pFile);
      }
      sqlite3_free(pReal);
    }
  }
  戻る rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
static 整数 fsDelete(sqlite3_vfs *pVfs, const char *zPath, 整数 dirSync){
  整数 rc = SQLITE_OK;
  fs_vfs_t *pFsVfs = (fs_vfs_t *)pVfs;
  fs_real_file *pReal;
  sqlite3_file *pF;
  整数 nName = (整数)strlen(zPath) - 8;

  assert(strlen("-journal")==8);
  assert(strcmp("-journal", &zPath[nName])==0);

  pReal = pFsVfs->pFileList; 
  ために(; pReal && strncmp(pReal->zName, zPath, nName); pReal=pReal->pNext);
  もし( pReal ){
    pF = pReal->pFile;
    rc = pF->pMethods->xWrite(pF, "\0\0\0\0", 4, pReal->nBlob-BLOCKSIZE);
    もし( rc==SQLITE_OK ){
      pReal->nJournal = 0;
    }
  }
  戻る rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
static 整数 fsAccess(
  sqlite3_vfs *pVfs, 
  const char *zPath, 
  整数 flags, 
  整数 *pResOut
){
  fs_vfs_t *pFsVfs = (fs_vfs_t *)pVfs;
  fs_real_file *pReal;
  整数 isJournal = 0;
  整数 nName = (整数)strlen(zPath);

  もし( flags!=SQLITE_ACCESS_EXISTS ){
    sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
    戻る pParent->xAccess(pParent, zPath, flags, pResOut);
  }

  assert(strlen("-journal")==8);
  もし( nName>8 && strcmp("-journal", &zPath[nName-8])==0 ){
    nName -= 8;
    isJournal = 1;
  }

  pReal = pFsVfs->pFileList; 
  ために(; pReal && strncmp(pReal->zName, zPath, nName); pReal=pReal->pNext);

  *pResOut = (pReal && (!isJournal || pReal->nJournal>0));
  戻る SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (FS_MAX_PATHNAME+1) bytes.
*/
static 整数 fsFullPathname(
  sqlite3_vfs *pVfs,            /* Pointer to vfs object */
  const char *zPath,            /* Possibly relative input path */
  整数 nOut,                     /* Size of output buffer in bytes */
  char *zOut                    /* Output buffer */
){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  戻る pParent->xFullPathname(pParent, zPath, nOut, zOut);
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
static void *fsDlOpen(sqlite3_vfs *pVfs, const char *zPath){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  戻る pParent->xDlOpen(pParent, zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
static void fsDlError(sqlite3_vfs *pVfs, 整数 nByte, char *zErrMsg){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  pParent->xDlError(pParent, nByte, zErrMsg);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
static void (*fsDlSym(sqlite3_vfs *pVfs, void *pH, const char *zSym))(void){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  戻る pParent->xDlSym(pParent, pH, zSym);
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
static 整数 fsRandomness(sqlite3_vfs *pVfs, 整数 nByte, char *zBufOut){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  戻る pParent->xRandomness(pParent, nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
static 整数 fsSleep(sqlite3_vfs *pVfs, 整数 nMicro){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  戻る pParent->xSleep(pParent, nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
static 整数 fsCurrentTime(sqlite3_vfs *pVfs, double *pTimeOut){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  戻る pParent->xCurrentTime(pParent, pTimeOut);
}

/*
** This procedure registers the fs vfs with SQLite. If the argument is
** true, the fs vfs becomes the new default vfs. It is the only publicly
** available function in this file.
*/
整数 fs_register(void){
  もし( fs_vfs.pParent ) 戻る SQLITE_OK;
  fs_vfs.pParent = sqlite3_vfs_find(0);
  fs_vfs.base.mxPathname = fs_vfs.pParent->mxPathname;
  fs_vfs.base.szOsFile = MAX(sizeof(tmp_file), sizeof(fs_file));
  戻る sqlite3_vfs_register(&fs_vfs.base, 0);
}

#ifdef SQLITE_TEST
  整数 SqlitetestOnefile_Init() {戻る fs_register();}
#endif
