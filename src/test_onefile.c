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
  puur_naa_nka nDatabase;              /* Current size of database region */
  puur_naa_nka nJournal;               /* Current size of journal region */
  puur_naa_nka nBlob;                  /* Total size of allocated blob */
  puur_naa_nka nRef;                   /* Number of pointers to this structure */
  fs_real_file *pNext;
  fs_real_file **ppThis;
};

typedef sa_nracanaa fs_file fs_file;
sa_nracanaa fs_file {
  sqlite3_file base;
  puur_naa_nka eType;
  fs_real_file *pReal;
};

typedef sa_nracanaa tmp_file tmp_file;
sa_nracanaa tmp_file {
  sqlite3_file base;
  puur_naa_nka nSize;
  puur_naa_nka nAlloc;
  var_na *zAlloc;
};

/* Values for fs_file.eType. */
#define DATABASE_FILE   1
#define JOURNAL_FILE    2

/*
** Method declarations for fs_file.
*/
static puur_naa_nka fsClose(sqlite3_file*);
static puur_naa_nka fsRead(sqlite3_file*, void*, puur_naa_nka iAmt, sqlite3_int64 iOfst);
static puur_naa_nka fsWrite(sqlite3_file*, const void*, puur_naa_nka iAmt, sqlite3_int64 iOfst);
static puur_naa_nka fsTruncate(sqlite3_file*, sqlite3_int64 size);
static puur_naa_nka fsSync(sqlite3_file*, puur_naa_nka flags);
static puur_naa_nka fsFileSize(sqlite3_file*, sqlite3_int64 *pSize);
static puur_naa_nka fsLock(sqlite3_file*, puur_naa_nka);
static puur_naa_nka fsUnlock(sqlite3_file*, puur_naa_nka);
static puur_naa_nka fsCheckReservedLock(sqlite3_file*, puur_naa_nka *pResOut);
static puur_naa_nka fsFileControl(sqlite3_file*, puur_naa_nka op, void *pArg);
static puur_naa_nka fsSectorSize(sqlite3_file*);
static puur_naa_nka fsDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for tmp_file.
*/
static puur_naa_nka tmpClose(sqlite3_file*);
static puur_naa_nka tmpRead(sqlite3_file*, void*, puur_naa_nka iAmt, sqlite3_int64 iOfst);
static puur_naa_nka tmpWrite(sqlite3_file*, const void*, puur_naa_nka iAmt, sqlite3_int64 iOfst);
static puur_naa_nka tmpTruncate(sqlite3_file*, sqlite3_int64 size);
static puur_naa_nka tmpSync(sqlite3_file*, puur_naa_nka flags);
static puur_naa_nka tmpFileSize(sqlite3_file*, sqlite3_int64 *pSize);
static puur_naa_nka tmpLock(sqlite3_file*, puur_naa_nka);
static puur_naa_nka tmpUnlock(sqlite3_file*, puur_naa_nka);
static puur_naa_nka tmpCheckReservedLock(sqlite3_file*, puur_naa_nka *pResOut);
static puur_naa_nka tmpFileControl(sqlite3_file*, puur_naa_nka op, void *pArg);
static puur_naa_nka tmpSectorSize(sqlite3_file*);
static puur_naa_nka tmpDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for fs_vfs.
*/
static puur_naa_nka fsOpen(sqlite3_vfs*, const var_na *, sqlite3_file*, puur_naa_nka , puur_naa_nka *);
static puur_naa_nka fsDelete(sqlite3_vfs*, const var_na *zName, puur_naa_nka syncDir);
static puur_naa_nka fsAccess(sqlite3_vfs*, const var_na *zName, puur_naa_nka flags, puur_naa_nka *);
static puur_naa_nka fsFullPathname(sqlite3_vfs*, const var_na *zName, puur_naa_nka nOut,var_na *zOut);
static void *fsDlOpen(sqlite3_vfs*, const var_na *zFilename);
static void fsDlError(sqlite3_vfs*, puur_naa_nka nByte, var_na *zErrMsg);
static void (*fsDlSym(sqlite3_vfs*,void*, const var_na *zSymbol))(void);
static void fsDlClose(sqlite3_vfs*, void*);
static puur_naa_nka fsRandomness(sqlite3_vfs*, puur_naa_nka nByte, var_na *zOut);
static puur_naa_nka fsSleep(sqlite3_vfs*, puur_naa_nka microseconds);
static puur_naa_nka fsCurrentTime(sqlite3_vfs*, dviguu_na*);


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
static puur_naa_nka tmpClose(sqlite3_file *pFile){
  tmp_file *pTmp = (tmp_file *)pFile;
  sqlite3_free(pTmp->zAlloc);
  ပွၵ်ႈ SQLITE_OK;
}

/*
** Read data from a tmp-file.
*/
static puur_naa_nka tmpRead(
  sqlite3_file *pFile, 
  void *zBuf, 
  puur_naa_nka iAmt, 
  sqlite_int64 iOfst
){
  tmp_file *pTmp = (tmp_file *)pFile;
  ပေႃး( (iAmt+iOfst)>pTmp->nSize ){
    ပွၵ်ႈ SQLITE_IOERR_SHORT_READ;
  }
  memcpy(zBuf, &pTmp->zAlloc[iOfst], iAmt);
  ပွၵ်ႈ SQLITE_OK;
}

/*
** Write data to a tmp-file.
*/
static puur_naa_nka tmpWrite(
  sqlite3_file *pFile, 
  const void *zBuf, 
  puur_naa_nka iAmt, 
  sqlite_int64 iOfst
){
  tmp_file *pTmp = (tmp_file *)pFile;
  ပေႃး( (iAmt+iOfst)>pTmp->nAlloc ){
    puur_naa_nka nNew = (puur_naa_nka)(2*(iAmt+iOfst+pTmp->nAlloc));
    var_na *zNew = sqlite3_realloc(pTmp->zAlloc, nNew);
    ပေႃး( !zNew ){
      ပွၵ်ႈ SQLITE_NOMEM;
    }
    pTmp->zAlloc = zNew;
    pTmp->nAlloc = nNew;
  }
  memcpy(&pTmp->zAlloc[iOfst], zBuf, iAmt);
  pTmp->nSize = (puur_naa_nka)MAX(pTmp->nSize, iOfst+iAmt);
  ပွၵ်ႈ SQLITE_OK;
}

/*
** Truncate a tmp-file.
*/
static puur_naa_nka tmpTruncate(sqlite3_file *pFile, sqlite_int64 size){
  tmp_file *pTmp = (tmp_file *)pFile;
  pTmp->nSize = (puur_naa_nka)MIN(pTmp->nSize, size);
  ပွၵ်ႈ SQLITE_OK;
}

/*
** Sync a tmp-file.
*/
static puur_naa_nka tmpSync(sqlite3_file *pFile, puur_naa_nka flags){
  ပွၵ်ႈ SQLITE_OK;
}

/*
** Return the current file-size of a tmp-file.
*/
static puur_naa_nka tmpFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  tmp_file *pTmp = (tmp_file *)pFile;
  *pSize = pTmp->nSize;
  ပွၵ်ႈ SQLITE_OK;
}

/*
** Lock a tmp-file.
*/
static puur_naa_nka tmpLock(sqlite3_file *pFile, puur_naa_nka eLock){
  ပွၵ်ႈ SQLITE_OK;
}

/*
** Unlock a tmp-file.
*/
static puur_naa_nka tmpUnlock(sqlite3_file *pFile, puur_naa_nka eLock){
  ပွၵ်ႈ SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on a tmp-file.
*/
static puur_naa_nka tmpCheckReservedLock(sqlite3_file *pFile, puur_naa_nka *pResOut){
  *pResOut = 0;
  ပွၵ်ႈ SQLITE_OK;
}

/*
** File control method. For custom operations on a tmp-file.
*/
static puur_naa_nka tmpFileControl(sqlite3_file *pFile, puur_naa_nka op, void *pArg){
  ပွၵ်ႈ SQLITE_OK;
}

/*
** Return the sector-size in bytes for a tmp-file.
*/
static puur_naa_nka tmpSectorSize(sqlite3_file *pFile){
  ပွၵ်ႈ 0;
}

/*
** Return the device characteristic flags supported by a tmp-file.
*/
static puur_naa_nka tmpDeviceCharacteristics(sqlite3_file *pFile){
  ပွၵ်ႈ 0;
}

/*
** Close an fs-file.
*/
static puur_naa_nka fsClose(sqlite3_file *pFile){
  puur_naa_nka rc = SQLITE_OK;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;

  /* Decrement the real_file ref-count. */
  pReal->nRef--;
  assert(pReal->nRef>=0);

  /* When the ref-count reaches 0, destroy the structure */
  ပေႃး( pReal->nRef==0 ){
    *pReal->ppThis = pReal->pNext;
    ပေႃး( pReal->pNext ){
      pReal->pNext->ppThis = pReal->ppThis;
    }
    rc = pReal->pFile->pMethods->xClose(pReal->pFile);
    sqlite3_free(pReal);
  }

  ပွၵ်ႈ rc;
}

/*
** Read data from an fs-file.
*/
static puur_naa_nka fsRead(
  sqlite3_file *pFile, 
  void *zBuf, 
  puur_naa_nka iAmt, 
  sqlite_int64 iOfst
){
  puur_naa_nka rc = SQLITE_OK;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  sqlite3_file *pF = pReal->pFile;

  ပေႃး( (p->eType==DATABASE_FILE && (iAmt+iOfst)>pReal->nDatabase)
   || (p->eType==JOURNAL_FILE && (iAmt+iOfst)>pReal->nJournal)
  ){
    rc = SQLITE_IOERR_SHORT_READ;
  }ပေႃးဢမ်ႇ ပေႃး( p->eType==DATABASE_FILE ){
    rc = pF->pMethods->xRead(pF, zBuf, iAmt, iOfst+BLOCKSIZE);
  }ပေႃးဢမ်ႇ{
    /* Journal file. */
    puur_naa_nka iRem = iAmt;
    puur_naa_nka iBuf = 0;
    puur_naa_nka ii = (puur_naa_nka)iOfst;
    ၶၢဝ်း( iRem>0 && rc==SQLITE_OK ){
      puur_naa_nka iRealOff = pReal->nBlob - BLOCKSIZE*((ii/BLOCKSIZE)+1) + ii%BLOCKSIZE;
      puur_naa_nka iRealAmt = MIN(iRem, BLOCKSIZE - (iRealOff%BLOCKSIZE));

      rc = pF->pMethods->xRead(pF, &((var_na *)zBuf)[iBuf], iRealAmt, iRealOff);
      ii += iRealAmt;
      iBuf += iRealAmt;
      iRem -= iRealAmt;
    }
  }

  ပွၵ်ႈ rc;
}

/*
** Write data to an fs-file.
*/
static puur_naa_nka fsWrite(
  sqlite3_file *pFile, 
  const void *zBuf, 
  puur_naa_nka iAmt, 
  sqlite_int64 iOfst
){
  puur_naa_nka rc = SQLITE_OK;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  sqlite3_file *pF = pReal->pFile;

  ပေႃး( p->eType==DATABASE_FILE ){
    ပေႃး( (iAmt+iOfst+BLOCKSIZE)>(pReal->nBlob-pReal->nJournal) ){
      rc = SQLITE_FULL;
    }ပေႃးဢမ်ႇ{
      rc = pF->pMethods->xWrite(pF, zBuf, iAmt, iOfst+BLOCKSIZE);
      ပေႃး( rc==SQLITE_OK ){
        pReal->nDatabase = (puur_naa_nka)MAX(pReal->nDatabase, iAmt+iOfst);
      }
    }
  }ပေႃးဢမ်ႇ{
    /* Journal file. */
    puur_naa_nka iRem = iAmt;
    puur_naa_nka iBuf = 0;
    puur_naa_nka ii = (puur_naa_nka)iOfst;
    ၶၢဝ်း( iRem>0 && rc==SQLITE_OK ){
      puur_naa_nka iRealOff = pReal->nBlob - BLOCKSIZE*((ii/BLOCKSIZE)+1) + ii%BLOCKSIZE;
      puur_naa_nka iRealAmt = MIN(iRem, BLOCKSIZE - (iRealOff%BLOCKSIZE));

      ပေႃး( iRealOff<(pReal->nDatabase+BLOCKSIZE) ){
        rc = SQLITE_FULL;
      }ပေႃးဢမ်ႇ{
        rc = pF->pMethods->xWrite(pF, &((var_na *)zBuf)[iBuf], iRealAmt,iRealOff);
        ii += iRealAmt;
        iBuf += iRealAmt;
        iRem -= iRealAmt;
      }
    }
    ပေႃး( rc==SQLITE_OK ){
      pReal->nJournal = (puur_naa_nka)MAX(pReal->nJournal, iAmt+iOfst);
    }
  }

  ပွၵ်ႈ rc;
}

/*
** Truncate an fs-file.
*/
static puur_naa_nka fsTruncate(sqlite3_file *pFile, sqlite_int64 size){
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  ပေႃး( p->eType==DATABASE_FILE ){
    pReal->nDatabase = (puur_naa_nka)MIN(pReal->nDatabase, size);
  }ပေႃးဢမ်ႇ{
    pReal->nJournal = (puur_naa_nka)MIN(pReal->nJournal, size);
  }
  ပွၵ်ႈ SQLITE_OK;
}

/*
** Sync an fs-file.
*/
static puur_naa_nka fsSync(sqlite3_file *pFile, puur_naa_nka flags){
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  sqlite3_file *pRealFile = pReal->pFile;
  puur_naa_nka rc = SQLITE_OK;

  ပေႃး( p->eType==DATABASE_FILE ){
    unsigned var_na zSize[4];
    zSize[0] = (pReal->nDatabase&0xFF000000)>>24;
    zSize[1] = (unsigned var_na)((pReal->nDatabase&0x00FF0000)>>16);
    zSize[2] = (pReal->nDatabase&0x0000FF00)>>8;
    zSize[3] = (pReal->nDatabase&0x000000FF);
    rc = pRealFile->pMethods->xWrite(pRealFile, zSize, 4, 0);
  }
  ပေႃး( rc==SQLITE_OK ){
    rc = pRealFile->pMethods->xSync(pRealFile, flags&(~SQLITE_SYNC_DATAONLY));
  }

  ပွၵ်ႈ rc;
}

/*
** Return the current file-size of an fs-file.
*/
static puur_naa_nka fsFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  ပေႃး( p->eType==DATABASE_FILE ){
    *pSize = pReal->nDatabase;
  }ပေႃးဢမ်ႇ{
    *pSize = pReal->nJournal;
  }
  ပွၵ်ႈ SQLITE_OK;
}

/*
** Lock an fs-file.
*/
static puur_naa_nka fsLock(sqlite3_file *pFile, puur_naa_nka eLock){
  ပွၵ်ႈ SQLITE_OK;
}

/*
** Unlock an fs-file.
*/
static puur_naa_nka fsUnlock(sqlite3_file *pFile, puur_naa_nka eLock){
  ပွၵ်ႈ SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on an fs-file.
*/
static puur_naa_nka fsCheckReservedLock(sqlite3_file *pFile, puur_naa_nka *pResOut){
  *pResOut = 0;
  ပွၵ်ႈ SQLITE_OK;
}

/*
** File control method. For custom operations on an fs-file.
*/
static puur_naa_nka fsFileControl(sqlite3_file *pFile, puur_naa_nka op, void *pArg){
  ပေႃး( op==SQLITE_FCNTL_PRAGMA ) ပွၵ်ႈ SQLITE_NOTFOUND;
  ပွၵ်ႈ SQLITE_OK;
}

/*
** Return the sector-size in bytes for an fs-file.
*/
static puur_naa_nka fsSectorSize(sqlite3_file *pFile){
  ပွၵ်ႈ BLOCKSIZE;
}

/*
** Return the device characteristic flags supported by an fs-file.
*/
static puur_naa_nka fsDeviceCharacteristics(sqlite3_file *pFile){
  ပွၵ်ႈ 0;
}

/*
** Open an fs file handle.
*/
static puur_naa_nka fsOpen(
  sqlite3_vfs *pVfs,
  const var_na *zName,
  sqlite3_file *pFile,
  puur_naa_nka flags,
  puur_naa_nka *pOutFlags
){
  fs_vfs_t *pFsVfs = (fs_vfs_t *)pVfs;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = 0;
  puur_naa_nka eType;
  puur_naa_nka nName;
  puur_naa_nka rc = SQLITE_OK;

  ပေႃး( 0==(flags&(SQLITE_OPEN_MAIN_DB|SQLITE_OPEN_MAIN_JOURNAL)) ){
    tmp_file *p2 = (tmp_file *)pFile;
    memset(p2, 0, parimaa_na(*p2));
    p2->base.pMethods = &tmp_io_methods;
    ပွၵ်ႈ SQLITE_OK;
  }

  eType = ((flags&(SQLITE_OPEN_MAIN_DB))?DATABASE_FILE:JOURNAL_FILE);
  p->base.pMethods = &fs_io_methods;
  p->eType = eType;

  assert(strlen("-journal")==8);
  nName = (puur_naa_nka)strlen(zName)-((eType==JOURNAL_FILE)?8:0);
  pReal=pFsVfs->pFileList; 
  for(; pReal && strncmp(pReal->zName, zName, nName); pReal=pReal->pNext);

  ပေႃး( !pReal ){
    puur_naa_nka real_flags = (flags&~(SQLITE_OPEN_MAIN_DB))|SQLITE_OPEN_TEMP_DB;
    sqlite3_int64 size;
    sqlite3_file *pRealFile;
    sqlite3_vfs *pParent = pFsVfs->pParent;
    assert(eType==DATABASE_FILE);

    pReal = (fs_real_file *)sqlite3_malloc(parimaa_na(*pReal)+pParent->szOsFile);
    ပေႃး( !pReal ){
      rc = SQLITE_NOMEM;
      goto open_out;
    }
    memset(pReal, 0, parimaa_na(*pReal)+pParent->szOsFile);
    pReal->zName = zName;
    pReal->pFile = (sqlite3_file *)(&pReal[1]);

    rc = pParent->xOpen(pParent, zName, pReal->pFile, real_flags, pOutFlags);
    ပေႃး( rc!=SQLITE_OK ){
      goto open_out;
    }
    pRealFile = pReal->pFile;

    rc = pRealFile->pMethods->xFileSize(pRealFile, &size);
    ပေႃး( rc!=SQLITE_OK ){
      goto open_out;
    }
    ပေႃး( size==0 ){
      rc = pRealFile->pMethods->xWrite(pRealFile, "\0", 1, BLOBSIZE-1);
      pReal->nBlob = BLOBSIZE;
    }ပေႃးဢမ်ႇ{
      unsigned var_na zS[4];
      pReal->nBlob = (puur_naa_nka)size;
      rc = pRealFile->pMethods->xRead(pRealFile, zS, 4, 0);
      pReal->nDatabase = (zS[0]<<24)+(zS[1]<<16)+(zS[2]<<8)+zS[3];
      ပေႃး( rc==SQLITE_OK ){
        rc = pRealFile->pMethods->xRead(pRealFile, zS, 4, pReal->nBlob-4);
        ပေႃး( zS[0] || zS[1] || zS[2] || zS[3] ){
          pReal->nJournal = pReal->nBlob;
        }
      }
    }

    ပေႃး( rc==SQLITE_OK ){
      pReal->pNext = pFsVfs->pFileList;
      ပေႃး( pReal->pNext ){
        pReal->pNext->ppThis = &pReal->pNext;
      }
      pReal->ppThis = &pFsVfs->pFileList;
      pFsVfs->pFileList = pReal;
    }
  }

open_out:
  ပေႃး( pReal ){
    ပေႃး( rc==SQLITE_OK ){
      p->pReal = pReal;
      pReal->nRef++;
    }ပေႃးဢမ်ႇ{
      ပေႃး( pReal->pFile->pMethods ){
        pReal->pFile->pMethods->xClose(pReal->pFile);
      }
      sqlite3_free(pReal);
    }
  }
  ပွၵ်ႈ rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
static puur_naa_nka fsDelete(sqlite3_vfs *pVfs, const var_na *zPath, puur_naa_nka dirSync){
  puur_naa_nka rc = SQLITE_OK;
  fs_vfs_t *pFsVfs = (fs_vfs_t *)pVfs;
  fs_real_file *pReal;
  sqlite3_file *pF;
  puur_naa_nka nName = (puur_naa_nka)strlen(zPath) - 8;

  assert(strlen("-journal")==8);
  assert(strcmp("-journal", &zPath[nName])==0);

  pReal = pFsVfs->pFileList; 
  for(; pReal && strncmp(pReal->zName, zPath, nName); pReal=pReal->pNext);
  ပေႃး( pReal ){
    pF = pReal->pFile;
    rc = pF->pMethods->xWrite(pF, "\0\0\0\0", 4, pReal->nBlob-BLOCKSIZE);
    ပေႃး( rc==SQLITE_OK ){
      pReal->nJournal = 0;
    }
  }
  ပွၵ်ႈ rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
static puur_naa_nka fsAccess(
  sqlite3_vfs *pVfs, 
  const var_na *zPath, 
  puur_naa_nka flags, 
  puur_naa_nka *pResOut
){
  fs_vfs_t *pFsVfs = (fs_vfs_t *)pVfs;
  fs_real_file *pReal;
  puur_naa_nka isJournal = 0;
  puur_naa_nka nName = (puur_naa_nka)strlen(zPath);

  ပေႃး( flags!=SQLITE_ACCESS_EXISTS ){
    sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
    ပွၵ်ႈ pParent->xAccess(pParent, zPath, flags, pResOut);
  }

  assert(strlen("-journal")==8);
  ပေႃး( nName>8 && strcmp("-journal", &zPath[nName-8])==0 ){
    nName -= 8;
    isJournal = 1;
  }

  pReal = pFsVfs->pFileList; 
  for(; pReal && strncmp(pReal->zName, zPath, nName); pReal=pReal->pNext);

  *pResOut = (pReal && (!isJournal || pReal->nJournal>0));
  ပွၵ်ႈ SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (FS_MAX_PATHNAME+1) bytes.
*/
static puur_naa_nka fsFullPathname(
  sqlite3_vfs *pVfs,            /* Pointer to vfs object */
  const var_na *zPath,            /* Possibly relative input path */
  puur_naa_nka nOut,                     /* Size of output buffer in bytes */
  var_na *zOut                    /* Output buffer */
){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  ပွၵ်ႈ pParent->xFullPathname(pParent, zPath, nOut, zOut);
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
static void *fsDlOpen(sqlite3_vfs *pVfs, const var_na *zPath){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  ပွၵ်ႈ pParent->xDlOpen(pParent, zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
static void fsDlError(sqlite3_vfs *pVfs, puur_naa_nka nByte, var_na *zErrMsg){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  pParent->xDlError(pParent, nByte, zErrMsg);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
static void (*fsDlSym(sqlite3_vfs *pVfs, void *pH, const var_na *zSym))(void){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  ပွၵ်ႈ pParent->xDlSym(pParent, pH, zSym);
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
static puur_naa_nka fsRandomness(sqlite3_vfs *pVfs, puur_naa_nka nByte, var_na *zBufOut){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  ပွၵ်ႈ pParent->xRandomness(pParent, nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
static puur_naa_nka fsSleep(sqlite3_vfs *pVfs, puur_naa_nka nMicro){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  ပွၵ်ႈ pParent->xSleep(pParent, nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
static puur_naa_nka fsCurrentTime(sqlite3_vfs *pVfs, dviguu_na *pTimeOut){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  ပွၵ်ႈ pParent->xCurrentTime(pParent, pTimeOut);
}

/*
** This procedure registers the fs vfs with SQLite. If the argument is
** true, the fs vfs becomes the new default vfs. It is the only publicly
** available function in this file.
*/
puur_naa_nka fs_register(void){
  ပေႃး( fs_vfs.pParent ) ပွၵ်ႈ SQLITE_OK;
  fs_vfs.pParent = sqlite3_vfs_find(0);
  fs_vfs.base.mxPathname = fs_vfs.pParent->mxPathname;
  fs_vfs.base.szOsFile = MAX(parimaa_na(tmp_file), parimaa_na(fs_file));
  ပွၵ်ႈ sqlite3_vfs_register(&fs_vfs.base, 0);
}

#ifdef SQLITE_TEST
  puur_naa_nka SqlitetestOnefile_Init() {ပွၵ်ႈ fs_register();}
#endif
