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

typedef sa_nracanaa fs_real_file fs_real_file;
sa_nracanaa fs_real_file {
  sqlite3_file *pFile;
  const var_na *zName;
  ཧྲིལ nDatabase;              /* Current size of database region */
  ཧྲིལ nJournal;               /* Current size of journal region */
  ཧྲིལ nBlob;                  /* Total size of allocated blob */
  ཧྲིལ nRef;                   /* Number of pointers to this structure */
  fs_real_file *pNext;
  fs_real_file **ppThis;
};

typedef sa_nracanaa fs_file fs_file;
sa_nracanaa fs_file {
  sqlite3_file base;
  ཧྲིལ eType;
  fs_real_file *pReal;
};

typedef sa_nracanaa tmp_file tmp_file;
sa_nracanaa tmp_file {
  sqlite3_file base;
  ཧྲིལ nSize;
  ཧྲིལ nAlloc;
  var_na *zAlloc;
};

/* Values for fs_file.eType. */
#define DATABASE_FILE   1
#define JOURNAL_FILE    2

/*
** Method declarations for fs_file.
*/
static ཧྲིལ fsClose(sqlite3_file*);
static ཧྲིལ fsRead(sqlite3_file*, void*, ཧྲིལ iAmt, sqlite3_int64 iOfst);
static ཧྲིལ fsWrite(sqlite3_file*, const void*, ཧྲིལ iAmt, sqlite3_int64 iOfst);
static ཧྲིལ fsTruncate(sqlite3_file*, sqlite3_int64 size);
static ཧྲིལ fsSync(sqlite3_file*, ཧྲིལ flags);
static ཧྲིལ fsFileSize(sqlite3_file*, sqlite3_int64 *pSize);
static ཧྲིལ fsLock(sqlite3_file*, ཧྲིལ);
static ཧྲིལ fsUnlock(sqlite3_file*, ཧྲིལ);
static ཧྲིལ fsCheckReservedLock(sqlite3_file*, ཧྲིལ *pResOut);
static ཧྲིལ fsFileControl(sqlite3_file*, ཧྲིལ op, void *pArg);
static ཧྲིལ fsSectorSize(sqlite3_file*);
static ཧྲིལ fsDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for tmp_file.
*/
static ཧྲིལ tmpClose(sqlite3_file*);
static ཧྲིལ tmpRead(sqlite3_file*, void*, ཧྲིལ iAmt, sqlite3_int64 iOfst);
static ཧྲིལ tmpWrite(sqlite3_file*, const void*, ཧྲིལ iAmt, sqlite3_int64 iOfst);
static ཧྲིལ tmpTruncate(sqlite3_file*, sqlite3_int64 size);
static ཧྲིལ tmpSync(sqlite3_file*, ཧྲིལ flags);
static ཧྲིལ tmpFileSize(sqlite3_file*, sqlite3_int64 *pSize);
static ཧྲིལ tmpLock(sqlite3_file*, ཧྲིལ);
static ཧྲིལ tmpUnlock(sqlite3_file*, ཧྲིལ);
static ཧྲིལ tmpCheckReservedLock(sqlite3_file*, ཧྲིལ *pResOut);
static ཧྲིལ tmpFileControl(sqlite3_file*, ཧྲིལ op, void *pArg);
static ཧྲིལ tmpSectorSize(sqlite3_file*);
static ཧྲིལ tmpDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for fs_vfs.
*/
static ཧྲིལ fsOpen(sqlite3_vfs*, const var_na *, sqlite3_file*, ཧྲིལ , ཧྲིལ *);
static ཧྲིལ fsDelete(sqlite3_vfs*, const var_na *zName, ཧྲིལ syncDir);
static ཧྲིལ fsAccess(sqlite3_vfs*, const var_na *zName, ཧྲིལ flags, ཧྲིལ *);
static ཧྲིལ fsFullPathname(sqlite3_vfs*, const var_na *zName, ཧྲིལ nOut,var_na *zOut);
static void *fsDlOpen(sqlite3_vfs*, const var_na *zFilename);
static void fsDlError(sqlite3_vfs*, ཧྲིལ nByte, var_na *zErrMsg);
static void (*fsDlSym(sqlite3_vfs*,void*, const var_na *zSymbol))(void);
static void fsDlClose(sqlite3_vfs*, void*);
static ཧྲིལ fsRandomness(sqlite3_vfs*, ཧྲིལ nByte, var_na *zOut);
static ཧྲིལ fsSleep(sqlite3_vfs*, ཧྲིལ microseconds);
static ཧྲིལ fsCurrentTime(sqlite3_vfs*, dviguu_na*);


typedef sa_nracanaa fs_vfs_t fs_vfs_t;
sa_nracanaa fs_vfs_t {
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
static ཧྲིལ tmpClose(sqlite3_file *pFile){
  tmp_file *pTmp = (tmp_file *)pFile;
  sqlite3_free(pTmp->zAlloc);
  ལོག SQLITE_OK;
}

/*
** Read data from a tmp-file.
*/
static ཧྲིལ tmpRead(
  sqlite3_file *pFile, 
  void *zBuf, 
  ཧྲིལ iAmt, 
  sqlite_int64 iOfst
){
  tmp_file *pTmp = (tmp_file *)pFile;
  གལ་སྲིད( (iAmt+iOfst)>pTmp->nSize ){
    ལོག SQLITE_IOERR_SHORT_READ;
  }
  memcpy(zBuf, &pTmp->zAlloc[iOfst], iAmt);
  ལོག SQLITE_OK;
}

/*
** Write data to a tmp-file.
*/
static ཧྲིལ tmpWrite(
  sqlite3_file *pFile, 
  const void *zBuf, 
  ཧྲིལ iAmt, 
  sqlite_int64 iOfst
){
  tmp_file *pTmp = (tmp_file *)pFile;
  གལ་སྲིད( (iAmt+iOfst)>pTmp->nAlloc ){
    ཧྲིལ nNew = (ཧྲིལ)(2*(iAmt+iOfst+pTmp->nAlloc));
    var_na *zNew = sqlite3_realloc(pTmp->zAlloc, nNew);
    གལ་སྲིད( !zNew ){
      ལོག SQLITE_NOMEM;
    }
    pTmp->zAlloc = zNew;
    pTmp->nAlloc = nNew;
  }
  memcpy(&pTmp->zAlloc[iOfst], zBuf, iAmt);
  pTmp->nSize = (ཧྲིལ)MAX(pTmp->nSize, iOfst+iAmt);
  ལོག SQLITE_OK;
}

/*
** Truncate a tmp-file.
*/
static ཧྲིལ tmpTruncate(sqlite3_file *pFile, sqlite_int64 size){
  tmp_file *pTmp = (tmp_file *)pFile;
  pTmp->nSize = (ཧྲིལ)MIN(pTmp->nSize, size);
  ལོག SQLITE_OK;
}

/*
** Sync a tmp-file.
*/
static ཧྲིལ tmpSync(sqlite3_file *pFile, ཧྲིལ flags){
  ལོག SQLITE_OK;
}

/*
** Return the current file-size of a tmp-file.
*/
static ཧྲིལ tmpFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  tmp_file *pTmp = (tmp_file *)pFile;
  *pSize = pTmp->nSize;
  ལོག SQLITE_OK;
}

/*
** Lock a tmp-file.
*/
static ཧྲིལ tmpLock(sqlite3_file *pFile, ཧྲིལ eLock){
  ལོག SQLITE_OK;
}

/*
** Unlock a tmp-file.
*/
static ཧྲིལ tmpUnlock(sqlite3_file *pFile, ཧྲིལ eLock){
  ལོག SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on a tmp-file.
*/
static ཧྲིལ tmpCheckReservedLock(sqlite3_file *pFile, ཧྲིལ *pResOut){
  *pResOut = 0;
  ལོག SQLITE_OK;
}

/*
** File control method. For custom operations on a tmp-file.
*/
static ཧྲིལ tmpFileControl(sqlite3_file *pFile, ཧྲིལ op, void *pArg){
  ལོག SQLITE_OK;
}

/*
** Return the sector-size in bytes for a tmp-file.
*/
static ཧྲིལ tmpSectorSize(sqlite3_file *pFile){
  ལོག 0;
}

/*
** Return the device characteristic flags supported by a tmp-file.
*/
static ཧྲིལ tmpDeviceCharacteristics(sqlite3_file *pFile){
  ལོག 0;
}

/*
** Close an fs-file.
*/
static ཧྲིལ fsClose(sqlite3_file *pFile){
  ཧྲིལ rc = SQLITE_OK;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;

  /* Decrement the real_file ref-count. */
  pReal->nRef--;
  assert(pReal->nRef>=0);

  /* When the ref-count reaches 0, destroy the structure */
  གལ་སྲིད( pReal->nRef==0 ){
    *pReal->ppThis = pReal->pNext;
    གལ་སྲིད( pReal->pNext ){
      pReal->pNext->ppThis = pReal->ppThis;
    }
    rc = pReal->pFile->pMethods->xClose(pReal->pFile);
    sqlite3_free(pReal);
  }

  ལོག rc;
}

/*
** Read data from an fs-file.
*/
static ཧྲིལ fsRead(
  sqlite3_file *pFile, 
  void *zBuf, 
  ཧྲིལ iAmt, 
  sqlite_int64 iOfst
){
  ཧྲིལ rc = SQLITE_OK;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  sqlite3_file *pF = pReal->pFile;

  གལ་སྲིད( (p->eType==DATABASE_FILE && (iAmt+iOfst)>pReal->nDatabase)
   || (p->eType==JOURNAL_FILE && (iAmt+iOfst)>pReal->nJournal)
  ){
    rc = SQLITE_IOERR_SHORT_READ;
  }དེ་མིན གལ་སྲིད( p->eType==DATABASE_FILE ){
    rc = pF->pMethods->xRead(pF, zBuf, iAmt, iOfst+BLOCKSIZE);
  }དེ་མིན{
    /* Journal file. */
    ཧྲིལ iRem = iAmt;
    ཧྲིལ iBuf = 0;
    ཧྲིལ ii = (ཧྲིལ)iOfst;
    སྐབས( iRem>0 && rc==SQLITE_OK ){
      ཧྲིལ iRealOff = pReal->nBlob - BLOCKSIZE*((ii/BLOCKSIZE)+1) + ii%BLOCKSIZE;
      ཧྲིལ iRealAmt = MIN(iRem, BLOCKSIZE - (iRealOff%BLOCKSIZE));

      rc = pF->pMethods->xRead(pF, &((var_na *)zBuf)[iBuf], iRealAmt, iRealOff);
      ii += iRealAmt;
      iBuf += iRealAmt;
      iRem -= iRealAmt;
    }
  }

  ལོག rc;
}

/*
** Write data to an fs-file.
*/
static ཧྲིལ fsWrite(
  sqlite3_file *pFile, 
  const void *zBuf, 
  ཧྲིལ iAmt, 
  sqlite_int64 iOfst
){
  ཧྲིལ rc = SQLITE_OK;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  sqlite3_file *pF = pReal->pFile;

  གལ་སྲིད( p->eType==DATABASE_FILE ){
    གལ་སྲིད( (iAmt+iOfst+BLOCKSIZE)>(pReal->nBlob-pReal->nJournal) ){
      rc = SQLITE_FULL;
    }དེ་མིན{
      rc = pF->pMethods->xWrite(pF, zBuf, iAmt, iOfst+BLOCKSIZE);
      གལ་སྲིད( rc==SQLITE_OK ){
        pReal->nDatabase = (ཧྲིལ)MAX(pReal->nDatabase, iAmt+iOfst);
      }
    }
  }དེ་མིན{
    /* Journal file. */
    ཧྲིལ iRem = iAmt;
    ཧྲིལ iBuf = 0;
    ཧྲིལ ii = (ཧྲིལ)iOfst;
    སྐབས( iRem>0 && rc==SQLITE_OK ){
      ཧྲིལ iRealOff = pReal->nBlob - BLOCKSIZE*((ii/BLOCKSIZE)+1) + ii%BLOCKSIZE;
      ཧྲིལ iRealAmt = MIN(iRem, BLOCKSIZE - (iRealOff%BLOCKSIZE));

      གལ་སྲིད( iRealOff<(pReal->nDatabase+BLOCKSIZE) ){
        rc = SQLITE_FULL;
      }དེ་མིན{
        rc = pF->pMethods->xWrite(pF, &((var_na *)zBuf)[iBuf], iRealAmt,iRealOff);
        ii += iRealAmt;
        iBuf += iRealAmt;
        iRem -= iRealAmt;
      }
    }
    གལ་སྲིད( rc==SQLITE_OK ){
      pReal->nJournal = (ཧྲིལ)MAX(pReal->nJournal, iAmt+iOfst);
    }
  }

  ལོག rc;
}

/*
** Truncate an fs-file.
*/
static ཧྲིལ fsTruncate(sqlite3_file *pFile, sqlite_int64 size){
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  གལ་སྲིད( p->eType==DATABASE_FILE ){
    pReal->nDatabase = (ཧྲིལ)MIN(pReal->nDatabase, size);
  }དེ་མིན{
    pReal->nJournal = (ཧྲིལ)MIN(pReal->nJournal, size);
  }
  ལོག SQLITE_OK;
}

/*
** Sync an fs-file.
*/
static ཧྲིལ fsSync(sqlite3_file *pFile, ཧྲིལ flags){
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  sqlite3_file *pRealFile = pReal->pFile;
  ཧྲིལ rc = SQLITE_OK;

  གལ་སྲིད( p->eType==DATABASE_FILE ){
    unsigned var_na zSize[4];
    zSize[0] = (pReal->nDatabase&0xFF000000)>>24;
    zSize[1] = (unsigned var_na)((pReal->nDatabase&0x00FF0000)>>16);
    zSize[2] = (pReal->nDatabase&0x0000FF00)>>8;
    zSize[3] = (pReal->nDatabase&0x000000FF);
    rc = pRealFile->pMethods->xWrite(pRealFile, zSize, 4, 0);
  }
  གལ་སྲིད( rc==SQLITE_OK ){
    rc = pRealFile->pMethods->xSync(pRealFile, flags&(~SQLITE_SYNC_DATAONLY));
  }

  ལོག rc;
}

/*
** Return the current file-size of an fs-file.
*/
static ཧྲིལ fsFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  གལ་སྲིད( p->eType==DATABASE_FILE ){
    *pSize = pReal->nDatabase;
  }དེ་མིན{
    *pSize = pReal->nJournal;
  }
  ལོག SQLITE_OK;
}

/*
** Lock an fs-file.
*/
static ཧྲིལ fsLock(sqlite3_file *pFile, ཧྲིལ eLock){
  ལོག SQLITE_OK;
}

/*
** Unlock an fs-file.
*/
static ཧྲིལ fsUnlock(sqlite3_file *pFile, ཧྲིལ eLock){
  ལོག SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on an fs-file.
*/
static ཧྲིལ fsCheckReservedLock(sqlite3_file *pFile, ཧྲིལ *pResOut){
  *pResOut = 0;
  ལོག SQLITE_OK;
}

/*
** File control method. For custom operations on an fs-file.
*/
static ཧྲིལ fsFileControl(sqlite3_file *pFile, ཧྲིལ op, void *pArg){
  གལ་སྲིད( op==SQLITE_FCNTL_PRAGMA ) ལོག SQLITE_NOTFOUND;
  ལོག SQLITE_OK;
}

/*
** Return the sector-size in bytes for an fs-file.
*/
static ཧྲིལ fsSectorSize(sqlite3_file *pFile){
  ལོག BLOCKSIZE;
}

/*
** Return the device characteristic flags supported by an fs-file.
*/
static ཧྲིལ fsDeviceCharacteristics(sqlite3_file *pFile){
  ལོག 0;
}

/*
** Open an fs file handle.
*/
static ཧྲིལ fsOpen(
  sqlite3_vfs *pVfs,
  const var_na *zName,
  sqlite3_file *pFile,
  ཧྲིལ flags,
  ཧྲིལ *pOutFlags
){
  fs_vfs_t *pFsVfs = (fs_vfs_t *)pVfs;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = 0;
  ཧྲིལ eType;
  ཧྲིལ nName;
  ཧྲིལ rc = SQLITE_OK;

  གལ་སྲིད( 0==(flags&(SQLITE_OPEN_MAIN_DB|SQLITE_OPEN_MAIN_JOURNAL)) ){
    tmp_file *p2 = (tmp_file *)pFile;
    memset(p2, 0, parimaa_na(*p2));
    p2->base.pMethods = &tmp_io_methods;
    ལོག SQLITE_OK;
  }

  eType = ((flags&(SQLITE_OPEN_MAIN_DB))?DATABASE_FILE:JOURNAL_FILE);
  p->base.pMethods = &fs_io_methods;
  p->eType = eType;

  assert(strlen("-journal")==8);
  nName = (ཧྲིལ)strlen(zName)-((eType==JOURNAL_FILE)?8:0);
  pReal=pFsVfs->pFileList; 
  དོན་ལུ(; pReal && strncmp(pReal->zName, zName, nName); pReal=pReal->pNext);

  གལ་སྲིད( !pReal ){
    ཧྲིལ real_flags = (flags&~(SQLITE_OPEN_MAIN_DB))|SQLITE_OPEN_TEMP_DB;
    sqlite3_int64 size;
    sqlite3_file *pRealFile;
    sqlite3_vfs *pParent = pFsVfs->pParent;
    assert(eType==DATABASE_FILE);

    pReal = (fs_real_file *)sqlite3_malloc(parimaa_na(*pReal)+pParent->szOsFile);
    གལ་སྲིད( !pReal ){
      rc = SQLITE_NOMEM;
      goto open_out;
    }
    memset(pReal, 0, parimaa_na(*pReal)+pParent->szOsFile);
    pReal->zName = zName;
    pReal->pFile = (sqlite3_file *)(&pReal[1]);

    rc = pParent->xOpen(pParent, zName, pReal->pFile, real_flags, pOutFlags);
    གལ་སྲིད( rc!=SQLITE_OK ){
      goto open_out;
    }
    pRealFile = pReal->pFile;

    rc = pRealFile->pMethods->xFileSize(pRealFile, &size);
    གལ་སྲིད( rc!=SQLITE_OK ){
      goto open_out;
    }
    གལ་སྲིད( size==0 ){
      rc = pRealFile->pMethods->xWrite(pRealFile, "\0", 1, BLOBSIZE-1);
      pReal->nBlob = BLOBSIZE;
    }དེ་མིན{
      unsigned var_na zS[4];
      pReal->nBlob = (ཧྲིལ)size;
      rc = pRealFile->pMethods->xRead(pRealFile, zS, 4, 0);
      pReal->nDatabase = (zS[0]<<24)+(zS[1]<<16)+(zS[2]<<8)+zS[3];
      གལ་སྲིད( rc==SQLITE_OK ){
        rc = pRealFile->pMethods->xRead(pRealFile, zS, 4, pReal->nBlob-4);
        གལ་སྲིད( zS[0] || zS[1] || zS[2] || zS[3] ){
          pReal->nJournal = pReal->nBlob;
        }
      }
    }

    གལ་སྲིད( rc==SQLITE_OK ){
      pReal->pNext = pFsVfs->pFileList;
      གལ་སྲིད( pReal->pNext ){
        pReal->pNext->ppThis = &pReal->pNext;
      }
      pReal->ppThis = &pFsVfs->pFileList;
      pFsVfs->pFileList = pReal;
    }
  }

open_out:
  གལ་སྲིད( pReal ){
    གལ་སྲིད( rc==SQLITE_OK ){
      p->pReal = pReal;
      pReal->nRef++;
    }དེ་མིན{
      གལ་སྲིད( pReal->pFile->pMethods ){
        pReal->pFile->pMethods->xClose(pReal->pFile);
      }
      sqlite3_free(pReal);
    }
  }
  ལོག rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
static ཧྲིལ fsDelete(sqlite3_vfs *pVfs, const var_na *zPath, ཧྲིལ dirSync){
  ཧྲིལ rc = SQLITE_OK;
  fs_vfs_t *pFsVfs = (fs_vfs_t *)pVfs;
  fs_real_file *pReal;
  sqlite3_file *pF;
  ཧྲིལ nName = (ཧྲིལ)strlen(zPath) - 8;

  assert(strlen("-journal")==8);
  assert(strcmp("-journal", &zPath[nName])==0);

  pReal = pFsVfs->pFileList; 
  དོན་ལུ(; pReal && strncmp(pReal->zName, zPath, nName); pReal=pReal->pNext);
  གལ་སྲིད( pReal ){
    pF = pReal->pFile;
    rc = pF->pMethods->xWrite(pF, "\0\0\0\0", 4, pReal->nBlob-BLOCKSIZE);
    གལ་སྲིད( rc==SQLITE_OK ){
      pReal->nJournal = 0;
    }
  }
  ལོག rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
static ཧྲིལ fsAccess(
  sqlite3_vfs *pVfs, 
  const var_na *zPath, 
  ཧྲིལ flags, 
  ཧྲིལ *pResOut
){
  fs_vfs_t *pFsVfs = (fs_vfs_t *)pVfs;
  fs_real_file *pReal;
  ཧྲིལ isJournal = 0;
  ཧྲིལ nName = (ཧྲིལ)strlen(zPath);

  གལ་སྲིད( flags!=SQLITE_ACCESS_EXISTS ){
    sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
    ལོག pParent->xAccess(pParent, zPath, flags, pResOut);
  }

  assert(strlen("-journal")==8);
  གལ་སྲིད( nName>8 && strcmp("-journal", &zPath[nName-8])==0 ){
    nName -= 8;
    isJournal = 1;
  }

  pReal = pFsVfs->pFileList; 
  དོན་ལུ(; pReal && strncmp(pReal->zName, zPath, nName); pReal=pReal->pNext);

  *pResOut = (pReal && (!isJournal || pReal->nJournal>0));
  ལོག SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (FS_MAX_PATHNAME+1) bytes.
*/
static ཧྲིལ fsFullPathname(
  sqlite3_vfs *pVfs,            /* Pointer to vfs object */
  const var_na *zPath,            /* Possibly relative input path */
  ཧྲིལ nOut,                     /* Size of output buffer in bytes */
  var_na *zOut                    /* Output buffer */
){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  ལོག pParent->xFullPathname(pParent, zPath, nOut, zOut);
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
static void *fsDlOpen(sqlite3_vfs *pVfs, const var_na *zPath){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  ལོག pParent->xDlOpen(pParent, zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
static void fsDlError(sqlite3_vfs *pVfs, ཧྲིལ nByte, var_na *zErrMsg){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  pParent->xDlError(pParent, nByte, zErrMsg);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
static void (*fsDlSym(sqlite3_vfs *pVfs, void *pH, const var_na *zSym))(void){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  ལོག pParent->xDlSym(pParent, pH, zSym);
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
static ཧྲིལ fsRandomness(sqlite3_vfs *pVfs, ཧྲིལ nByte, var_na *zBufOut){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  ལོག pParent->xRandomness(pParent, nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
static ཧྲིལ fsSleep(sqlite3_vfs *pVfs, ཧྲིལ nMicro){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  ལོག pParent->xSleep(pParent, nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
static ཧྲིལ fsCurrentTime(sqlite3_vfs *pVfs, dviguu_na *pTimeOut){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  ལོག pParent->xCurrentTime(pParent, pTimeOut);
}

/*
** This procedure registers the fs vfs with SQLite. If the argument is
** true, the fs vfs becomes the new default vfs. It is the only publicly
** available function in this file.
*/
ཧྲིལ fs_register(void){
  གལ་སྲིད( fs_vfs.pParent ) ལོག SQLITE_OK;
  fs_vfs.pParent = sqlite3_vfs_find(0);
  fs_vfs.base.mxPathname = fs_vfs.pParent->mxPathname;
  fs_vfs.base.szOsFile = MAX(parimaa_na(tmp_file), parimaa_na(fs_file));
  ལོག sqlite3_vfs_register(&fs_vfs.base, 0);
}

#ifdef SQLITE_TEST
  ཧྲིལ SqlitetestOnefile_Init() {ལོག fs_register();}
#endif
