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

#അന്തര്ഭാവ "sqlite3.h"
#അന്തര്ഭാവ <assert.h>
#അന്തര്ഭാവ <string.h>

/*
** Maximum pathname length supported by the fs backend.
*/
#define BLOCKSIZE 512
#define BLOBSIZE 10485760

/*
** Name used to identify this VFS.
*/
#define FS_VFS_NAME "fs"

തരനിർവചനം ഘടന fs_real_file fs_real_file;
ഘടന fs_real_file {
  sqlite3_file *pFile;
  സ്ഥിരം അക്ഷരം *zName;
  പൂർണ്ണസംഖ്യ nDatabase;              /* Current size of database region */
  പൂർണ്ണസംഖ്യ nJournal;               /* Current size of journal region */
  പൂർണ്ണസംഖ്യ nBlob;                  /* Total size of allocated blob */
  പൂർണ്ണസംഖ്യ nRef;                   /* Number of pointers to this structure */
  fs_real_file *pNext;
  fs_real_file **ppThis;
};

തരനിർവചനം ഘടന fs_file fs_file;
ഘടന fs_file {
  sqlite3_file base;
  പൂർണ്ണസംഖ്യ eType;
  fs_real_file *pReal;
};

തരനിർവചനം ഘടന tmp_file tmp_file;
ഘടന tmp_file {
  sqlite3_file base;
  പൂർണ്ണസംഖ്യ nSize;
  പൂർണ്ണസംഖ്യ nAlloc;
  അക്ഷരം *zAlloc;
};

/* Values for fs_file.eType. */
#define DATABASE_FILE   1
#define JOURNAL_FILE    2

/*
** Method declarations for fs_file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ fsClose(sqlite3_file*);
സ്ഥിതം പൂർണ്ണസംഖ്യ fsRead(sqlite3_file*, ശൂന്യം*, പൂർണ്ണസംഖ്യ iAmt, sqlite3_int64 iOfst);
സ്ഥിതം പൂർണ്ണസംഖ്യ fsWrite(sqlite3_file*, സ്ഥിരം ശൂന്യം*, പൂർണ്ണസംഖ്യ iAmt, sqlite3_int64 iOfst);
സ്ഥിതം പൂർണ്ണസംഖ്യ fsTruncate(sqlite3_file*, sqlite3_int64 size);
സ്ഥിതം പൂർണ്ണസംഖ്യ fsSync(sqlite3_file*, പൂർണ്ണസംഖ്യ flags);
സ്ഥിതം പൂർണ്ണസംഖ്യ fsFileSize(sqlite3_file*, sqlite3_int64 *pSize);
സ്ഥിതം പൂർണ്ണസംഖ്യ fsLock(sqlite3_file*, പൂർണ്ണസംഖ്യ);
സ്ഥിതം പൂർണ്ണസംഖ്യ fsUnlock(sqlite3_file*, പൂർണ്ണസംഖ്യ);
സ്ഥിതം പൂർണ്ണസംഖ്യ fsCheckReservedLock(sqlite3_file*, പൂർണ്ണസംഖ്യ *pResOut);
സ്ഥിതം പൂർണ്ണസംഖ്യ fsFileControl(sqlite3_file*, പൂർണ്ണസംഖ്യ op, ശൂന്യം *pArg);
സ്ഥിതം പൂർണ്ണസംഖ്യ fsSectorSize(sqlite3_file*);
സ്ഥിതം പൂർണ്ണസംഖ്യ fsDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for tmp_file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ tmpClose(sqlite3_file*);
സ്ഥിതം പൂർണ്ണസംഖ്യ tmpRead(sqlite3_file*, ശൂന്യം*, പൂർണ്ണസംഖ്യ iAmt, sqlite3_int64 iOfst);
സ്ഥിതം പൂർണ്ണസംഖ്യ tmpWrite(sqlite3_file*, സ്ഥിരം ശൂന്യം*, പൂർണ്ണസംഖ്യ iAmt, sqlite3_int64 iOfst);
സ്ഥിതം പൂർണ്ണസംഖ്യ tmpTruncate(sqlite3_file*, sqlite3_int64 size);
സ്ഥിതം പൂർണ്ണസംഖ്യ tmpSync(sqlite3_file*, പൂർണ്ണസംഖ്യ flags);
സ്ഥിതം പൂർണ്ണസംഖ്യ tmpFileSize(sqlite3_file*, sqlite3_int64 *pSize);
സ്ഥിതം പൂർണ്ണസംഖ്യ tmpLock(sqlite3_file*, പൂർണ്ണസംഖ്യ);
സ്ഥിതം പൂർണ്ണസംഖ്യ tmpUnlock(sqlite3_file*, പൂർണ്ണസംഖ്യ);
സ്ഥിതം പൂർണ്ണസംഖ്യ tmpCheckReservedLock(sqlite3_file*, പൂർണ്ണസംഖ്യ *pResOut);
സ്ഥിതം പൂർണ്ണസംഖ്യ tmpFileControl(sqlite3_file*, പൂർണ്ണസംഖ്യ op, ശൂന്യം *pArg);
സ്ഥിതം പൂർണ്ണസംഖ്യ tmpSectorSize(sqlite3_file*);
സ്ഥിതം പൂർണ്ണസംഖ്യ tmpDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for fs_vfs.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ fsOpen(sqlite3_vfs*, സ്ഥിരം അക്ഷരം *, sqlite3_file*, പൂർണ്ണസംഖ്യ , പൂർണ്ണസംഖ്യ *);
സ്ഥിതം പൂർണ്ണസംഖ്യ fsDelete(sqlite3_vfs*, സ്ഥിരം അക്ഷരം *zName, പൂർണ്ണസംഖ്യ syncDir);
സ്ഥിതം പൂർണ്ണസംഖ്യ fsAccess(sqlite3_vfs*, സ്ഥിരം അക്ഷരം *zName, പൂർണ്ണസംഖ്യ flags, പൂർണ്ണസംഖ്യ *);
സ്ഥിതം പൂർണ്ണസംഖ്യ fsFullPathname(sqlite3_vfs*, സ്ഥിരം അക്ഷരം *zName, പൂർണ്ണസംഖ്യ nOut,അക്ഷരം *zOut);
സ്ഥിതം ശൂന്യം *fsDlOpen(sqlite3_vfs*, സ്ഥിരം അക്ഷരം *zFilename);
സ്ഥിതം ശൂന്യം fsDlError(sqlite3_vfs*, പൂർണ്ണസംഖ്യ nByte, അക്ഷരം *z_Err_സന്ദേശം);
സ്ഥിതം ശൂന്യം (*fsDlSym(sqlite3_vfs*,ശൂന്യം*, സ്ഥിരം അക്ഷരം *z_ചിഹ്നം))(ശൂന്യം);
സ്ഥിതം ശൂന്യം fsDlClose(sqlite3_vfs*, ശൂന്യം*);
സ്ഥിതം പൂർണ്ണസംഖ്യ fsRandomness(sqlite3_vfs*, പൂർണ്ണസംഖ്യ nByte, അക്ഷരം *zOut);
സ്ഥിതം പൂർണ്ണസംഖ്യ fsSleep(sqlite3_vfs*, പൂർണ്ണസംഖ്യ microseconds);
സ്ഥിതം പൂർണ്ണസംഖ്യ fsCurrentTime(sqlite3_vfs*, ഇരട്ട*);


തരനിർവചനം ഘടന fs_vfs_t fs_vfs_t;
ഘടന fs_vfs_t {
  sqlite3_vfs base;
  fs_real_file *pFileList;
  sqlite3_vfs *pParent;
};

സ്ഥിതം fs_vfs_t fs_vfs = {
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

സ്ഥിതം sqlite3_io_methods fs_io_methods = {
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


സ്ഥിതം sqlite3_io_methods tmp_io_methods = {
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
സ്ഥിതം പൂർണ്ണസംഖ്യ tmpClose(sqlite3_file *pFile){
  tmp_file *pTmp = (tmp_file *)pFile;
  sqlite3_free(pTmp->zAlloc);
  തിരികെ SQLITE_OK;
}

/*
** Read data from a tmp-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ tmpRead(
  sqlite3_file *pFile, 
  ശൂന്യം *zBuf, 
  പൂർണ്ണസംഖ്യ iAmt, 
  sqlite_int64 iOfst
){
  tmp_file *pTmp = (tmp_file *)pFile;
  എങ്കിൽ( (iAmt+iOfst)>pTmp->nSize ){
    തിരികെ SQLITE_IOERR_SHORT_READ;
  }
  memcpy(zBuf, &pTmp->zAlloc[iOfst], iAmt);
  തിരികെ SQLITE_OK;
}

/*
** Write data to a tmp-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ tmpWrite(
  sqlite3_file *pFile, 
  സ്ഥിരം ശൂന്യം *zBuf, 
  പൂർണ്ണസംഖ്യ iAmt, 
  sqlite_int64 iOfst
){
  tmp_file *pTmp = (tmp_file *)pFile;
  എങ്കിൽ( (iAmt+iOfst)>pTmp->nAlloc ){
    പൂർണ്ണസംഖ്യ nNew = (പൂർണ്ണസംഖ്യ)(2*(iAmt+iOfst+pTmp->nAlloc));
    അക്ഷരം *zNew = sqlite3_realloc(pTmp->zAlloc, nNew);
    എങ്കിൽ( !zNew ){
      തിരികെ SQLITE_NOMEM;
    }
    pTmp->zAlloc = zNew;
    pTmp->nAlloc = nNew;
  }
  memcpy(&pTmp->zAlloc[iOfst], zBuf, iAmt);
  pTmp->nSize = (പൂർണ്ണസംഖ്യ)MAX(pTmp->nSize, iOfst+iAmt);
  തിരികെ SQLITE_OK;
}

/*
** Truncate a tmp-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ tmpTruncate(sqlite3_file *pFile, sqlite_int64 size){
  tmp_file *pTmp = (tmp_file *)pFile;
  pTmp->nSize = (പൂർണ്ണസംഖ്യ)MIN(pTmp->nSize, size);
  തിരികെ SQLITE_OK;
}

/*
** Sync a tmp-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ tmpSync(sqlite3_file *pFile, പൂർണ്ണസംഖ്യ flags){
  തിരികെ SQLITE_OK;
}

/*
** Return the current file-size of a tmp-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ tmpFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  tmp_file *pTmp = (tmp_file *)pFile;
  *pSize = pTmp->nSize;
  തിരികെ SQLITE_OK;
}

/*
** Lock a tmp-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ tmpLock(sqlite3_file *pFile, പൂർണ്ണസംഖ്യ eLock){
  തിരികെ SQLITE_OK;
}

/*
** Unlock a tmp-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ tmpUnlock(sqlite3_file *pFile, പൂർണ്ണസംഖ്യ eLock){
  തിരികെ SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on a tmp-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ tmpCheckReservedLock(sqlite3_file *pFile, പൂർണ്ണസംഖ്യ *pResOut){
  *pResOut = 0;
  തിരികെ SQLITE_OK;
}

/*
** File control method. For custom operations on a tmp-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ tmpFileControl(sqlite3_file *pFile, പൂർണ്ണസംഖ്യ op, ശൂന്യം *pArg){
  തിരികെ SQLITE_OK;
}

/*
** Return the sector-size in bytes for a tmp-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ tmpSectorSize(sqlite3_file *pFile){
  തിരികെ 0;
}

/*
** Return the device characteristic flags supported by a tmp-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ tmpDeviceCharacteristics(sqlite3_file *pFile){
  തിരികെ 0;
}

/*
** Close an fs-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ fsClose(sqlite3_file *pFile){
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;

  /* Decrement the real_file ref-count. */
  pReal->nRef--;
  പ്രതിജ്ഞാ(pReal->nRef>=0);

  /* When the ref-count reaches 0, destroy the structure */
  എങ്കിൽ( pReal->nRef==0 ){
    *pReal->ppThis = pReal->pNext;
    എങ്കിൽ( pReal->pNext ){
      pReal->pNext->ppThis = pReal->ppThis;
    }
    rc = pReal->pFile->pMethods->xClose(pReal->pFile);
    sqlite3_free(pReal);
  }

  തിരികെ rc;
}

/*
** Read data from an fs-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ fsRead(
  sqlite3_file *pFile, 
  ശൂന്യം *zBuf, 
  പൂർണ്ണസംഖ്യ iAmt, 
  sqlite_int64 iOfst
){
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  sqlite3_file *pF = pReal->pFile;

  എങ്കിൽ( (p->eType==DATABASE_FILE && (iAmt+iOfst)>pReal->nDatabase)
   || (p->eType==JOURNAL_FILE && (iAmt+iOfst)>pReal->nJournal)
  ){
    rc = SQLITE_IOERR_SHORT_READ;
  }അല്ലെങ്കിൽ എങ്കിൽ( p->eType==DATABASE_FILE ){
    rc = pF->pMethods->xRead(pF, zBuf, iAmt, iOfst+BLOCKSIZE);
  }അല്ലെങ്കിൽ{
    /* Journal file. */
    പൂർണ്ണസംഖ്യ iRem = iAmt;
    പൂർണ്ണസംഖ്യ iBuf = 0;
    പൂർണ്ണസംഖ്യ ii = (പൂർണ്ണസംഖ്യ)iOfst;
    ആകുമ്പോൾ( iRem>0 && rc==SQLITE_OK ){
      പൂർണ്ണസംഖ്യ iRealOff = pReal->nBlob - BLOCKSIZE*((ii/BLOCKSIZE)+1) + ii%BLOCKSIZE;
      പൂർണ്ണസംഖ്യ iRealAmt = MIN(iRem, BLOCKSIZE - (iRealOff%BLOCKSIZE));

      rc = pF->pMethods->xRead(pF, &((അക്ഷരം *)zBuf)[iBuf], iRealAmt, iRealOff);
      ii += iRealAmt;
      iBuf += iRealAmt;
      iRem -= iRealAmt;
    }
  }

  തിരികെ rc;
}

/*
** Write data to an fs-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ fsWrite(
  sqlite3_file *pFile, 
  സ്ഥിരം ശൂന്യം *zBuf, 
  പൂർണ്ണസംഖ്യ iAmt, 
  sqlite_int64 iOfst
){
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  sqlite3_file *pF = pReal->pFile;

  എങ്കിൽ( p->eType==DATABASE_FILE ){
    എങ്കിൽ( (iAmt+iOfst+BLOCKSIZE)>(pReal->nBlob-pReal->nJournal) ){
      rc = SQLITE_FULL;
    }അല്ലെങ്കിൽ{
      rc = pF->pMethods->xWrite(pF, zBuf, iAmt, iOfst+BLOCKSIZE);
      എങ്കിൽ( rc==SQLITE_OK ){
        pReal->nDatabase = (പൂർണ്ണസംഖ്യ)MAX(pReal->nDatabase, iAmt+iOfst);
      }
    }
  }അല്ലെങ്കിൽ{
    /* Journal file. */
    പൂർണ്ണസംഖ്യ iRem = iAmt;
    പൂർണ്ണസംഖ്യ iBuf = 0;
    പൂർണ്ണസംഖ്യ ii = (പൂർണ്ണസംഖ്യ)iOfst;
    ആകുമ്പോൾ( iRem>0 && rc==SQLITE_OK ){
      പൂർണ്ണസംഖ്യ iRealOff = pReal->nBlob - BLOCKSIZE*((ii/BLOCKSIZE)+1) + ii%BLOCKSIZE;
      പൂർണ്ണസംഖ്യ iRealAmt = MIN(iRem, BLOCKSIZE - (iRealOff%BLOCKSIZE));

      എങ്കിൽ( iRealOff<(pReal->nDatabase+BLOCKSIZE) ){
        rc = SQLITE_FULL;
      }അല്ലെങ്കിൽ{
        rc = pF->pMethods->xWrite(pF, &((അക്ഷരം *)zBuf)[iBuf], iRealAmt,iRealOff);
        ii += iRealAmt;
        iBuf += iRealAmt;
        iRem -= iRealAmt;
      }
    }
    എങ്കിൽ( rc==SQLITE_OK ){
      pReal->nJournal = (പൂർണ്ണസംഖ്യ)MAX(pReal->nJournal, iAmt+iOfst);
    }
  }

  തിരികെ rc;
}

/*
** Truncate an fs-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ fsTruncate(sqlite3_file *pFile, sqlite_int64 size){
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  എങ്കിൽ( p->eType==DATABASE_FILE ){
    pReal->nDatabase = (പൂർണ്ണസംഖ്യ)MIN(pReal->nDatabase, size);
  }അല്ലെങ്കിൽ{
    pReal->nJournal = (പൂർണ്ണസംഖ്യ)MIN(pReal->nJournal, size);
  }
  തിരികെ SQLITE_OK;
}

/*
** Sync an fs-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ fsSync(sqlite3_file *pFile, പൂർണ്ണസംഖ്യ flags){
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  sqlite3_file *pRealFile = pReal->pFile;
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;

  എങ്കിൽ( p->eType==DATABASE_FILE ){
    unsigned അക്ഷരം zSize[4];
    zSize[0] = (pReal->nDatabase&0xFF000000)>>24;
    zSize[1] = (unsigned അക്ഷരം)((pReal->nDatabase&0x00FF0000)>>16);
    zSize[2] = (pReal->nDatabase&0x0000FF00)>>8;
    zSize[3] = (pReal->nDatabase&0x000000FF);
    rc = pRealFile->pMethods->xWrite(pRealFile, zSize, 4, 0);
  }
  എങ്കിൽ( rc==SQLITE_OK ){
    rc = pRealFile->pMethods->xSync(pRealFile, flags&(~SQLITE_SYNC_DATAONLY));
  }

  തിരികെ rc;
}

/*
** Return the current file-size of an fs-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ fsFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  എങ്കിൽ( p->eType==DATABASE_FILE ){
    *pSize = pReal->nDatabase;
  }അല്ലെങ്കിൽ{
    *pSize = pReal->nJournal;
  }
  തിരികെ SQLITE_OK;
}

/*
** Lock an fs-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ fsLock(sqlite3_file *pFile, പൂർണ്ണസംഖ്യ eLock){
  തിരികെ SQLITE_OK;
}

/*
** Unlock an fs-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ fsUnlock(sqlite3_file *pFile, പൂർണ്ണസംഖ്യ eLock){
  തിരികെ SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on an fs-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ fsCheckReservedLock(sqlite3_file *pFile, പൂർണ്ണസംഖ്യ *pResOut){
  *pResOut = 0;
  തിരികെ SQLITE_OK;
}

/*
** File control method. For custom operations on an fs-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ fsFileControl(sqlite3_file *pFile, പൂർണ്ണസംഖ്യ op, ശൂന്യം *pArg){
  എങ്കിൽ( op==SQLITE_FCNTL_PRAGMA ) തിരികെ SQLITE_NOTFOUND;
  തിരികെ SQLITE_OK;
}

/*
** Return the sector-size in bytes for an fs-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ fsSectorSize(sqlite3_file *pFile){
  തിരികെ BLOCKSIZE;
}

/*
** Return the device characteristic flags supported by an fs-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ fsDeviceCharacteristics(sqlite3_file *pFile){
  തിരികെ 0;
}

/*
** Open an fs file handle.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ fsOpen(
  sqlite3_vfs *pVfs,
  സ്ഥിരം അക്ഷരം *zName,
  sqlite3_file *pFile,
  പൂർണ്ണസംഖ്യ flags,
  പൂർണ്ണസംഖ്യ *pOutFlags
){
  fs_vfs_t *pFsVfs = (fs_vfs_t *)pVfs;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = 0;
  പൂർണ്ണസംഖ്യ eType;
  പൂർണ്ണസംഖ്യ nName;
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;

  എങ്കിൽ( 0==(flags&(SQLITE_OPEN_MAIN_DB|SQLITE_OPEN_MAIN_JOURNAL)) ){
    tmp_file *p2 = (tmp_file *)pFile;
    memset(p2, 0, വലുപ്പം(*p2));
    p2->base.pMethods = &tmp_io_methods;
    തിരികെ SQLITE_OK;
  }

  eType = ((flags&(SQLITE_OPEN_MAIN_DB))?DATABASE_FILE:JOURNAL_FILE);
  p->base.pMethods = &fs_io_methods;
  p->eType = eType;

  പ്രതിജ്ഞാ(strlen("-journal")==8);
  nName = (പൂർണ്ണസംഖ്യ)strlen(zName)-((eType==JOURNAL_FILE)?8:0);
  pReal=pFsVfs->pFileList; 
  വേണ്ടി(; pReal && strncmp(pReal->zName, zName, nName); pReal=pReal->pNext);

  എങ്കിൽ( !pReal ){
    പൂർണ്ണസംഖ്യ real_flags = (flags&~(SQLITE_OPEN_MAIN_DB))|SQLITE_OPEN_TEMP_DB;
    sqlite3_int64 size;
    sqlite3_file *pRealFile;
    sqlite3_vfs *pParent = pFsVfs->pParent;
    പ്രതിജ്ഞാ(eType==DATABASE_FILE);

    pReal = (fs_real_file *)sqlite3_malloc(വലുപ്പം(*pReal)+pParent->szOsFile);
    എങ്കിൽ( !pReal ){
      rc = SQLITE_NOMEM;
      പോകുക open_out;
    }
    memset(pReal, 0, വലുപ്പം(*pReal)+pParent->szOsFile);
    pReal->zName = zName;
    pReal->pFile = (sqlite3_file *)(&pReal[1]);

    rc = pParent->xOpen(pParent, zName, pReal->pFile, real_flags, pOutFlags);
    എങ്കിൽ( rc!=SQLITE_OK ){
      പോകുക open_out;
    }
    pRealFile = pReal->pFile;

    rc = pRealFile->pMethods->xFileSize(pRealFile, &size);
    എങ്കിൽ( rc!=SQLITE_OK ){
      പോകുക open_out;
    }
    എങ്കിൽ( size==0 ){
      rc = pRealFile->pMethods->xWrite(pRealFile, "\0", 1, BLOBSIZE-1);
      pReal->nBlob = BLOBSIZE;
    }അല്ലെങ്കിൽ{
      unsigned അക്ഷരം zS[4];
      pReal->nBlob = (പൂർണ്ണസംഖ്യ)size;
      rc = pRealFile->pMethods->xRead(pRealFile, zS, 4, 0);
      pReal->nDatabase = (zS[0]<<24)+(zS[1]<<16)+(zS[2]<<8)+zS[3];
      എങ്കിൽ( rc==SQLITE_OK ){
        rc = pRealFile->pMethods->xRead(pRealFile, zS, 4, pReal->nBlob-4);
        എങ്കിൽ( zS[0] || zS[1] || zS[2] || zS[3] ){
          pReal->nJournal = pReal->nBlob;
        }
      }
    }

    എങ്കിൽ( rc==SQLITE_OK ){
      pReal->pNext = pFsVfs->pFileList;
      എങ്കിൽ( pReal->pNext ){
        pReal->pNext->ppThis = &pReal->pNext;
      }
      pReal->ppThis = &pFsVfs->pFileList;
      pFsVfs->pFileList = pReal;
    }
  }

open_out:
  എങ്കിൽ( pReal ){
    എങ്കിൽ( rc==SQLITE_OK ){
      p->pReal = pReal;
      pReal->nRef++;
    }അല്ലെങ്കിൽ{
      എങ്കിൽ( pReal->pFile->pMethods ){
        pReal->pFile->pMethods->xClose(pReal->pFile);
      }
      sqlite3_free(pReal);
    }
  }
  തിരികെ rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ fsDelete(sqlite3_vfs *pVfs, സ്ഥിരം അക്ഷരം *zPath, പൂർണ്ണസംഖ്യ dirSync){
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;
  fs_vfs_t *pFsVfs = (fs_vfs_t *)pVfs;
  fs_real_file *pReal;
  sqlite3_file *pF;
  പൂർണ്ണസംഖ്യ nName = (പൂർണ്ണസംഖ്യ)strlen(zPath) - 8;

  പ്രതിജ്ഞാ(strlen("-journal")==8);
  പ്രതിജ്ഞാ(strcmp("-journal", &zPath[nName])==0);

  pReal = pFsVfs->pFileList; 
  വേണ്ടി(; pReal && strncmp(pReal->zName, zPath, nName); pReal=pReal->pNext);
  എങ്കിൽ( pReal ){
    pF = pReal->pFile;
    rc = pF->pMethods->xWrite(pF, "\0\0\0\0", 4, pReal->nBlob-BLOCKSIZE);
    എങ്കിൽ( rc==SQLITE_OK ){
      pReal->nJournal = 0;
    }
  }
  തിരികെ rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ fsAccess(
  sqlite3_vfs *pVfs, 
  സ്ഥിരം അക്ഷരം *zPath, 
  പൂർണ്ണസംഖ്യ flags, 
  പൂർണ്ണസംഖ്യ *pResOut
){
  fs_vfs_t *pFsVfs = (fs_vfs_t *)pVfs;
  fs_real_file *pReal;
  പൂർണ്ണസംഖ്യ isJournal = 0;
  പൂർണ്ണസംഖ്യ nName = (പൂർണ്ണസംഖ്യ)strlen(zPath);

  എങ്കിൽ( flags!=SQLITE_ACCESS_EXISTS ){
    sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
    തിരികെ pParent->xAccess(pParent, zPath, flags, pResOut);
  }

  പ്രതിജ്ഞാ(strlen("-journal")==8);
  എങ്കിൽ( nName>8 && strcmp("-journal", &zPath[nName-8])==0 ){
    nName -= 8;
    isJournal = 1;
  }

  pReal = pFsVfs->pFileList; 
  വേണ്ടി(; pReal && strncmp(pReal->zName, zPath, nName); pReal=pReal->pNext);

  *pResOut = (pReal && (!isJournal || pReal->nJournal>0));
  തിരികെ SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (FS_MAX_PATHNAME+1) bytes.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ fsFullPathname(
  sqlite3_vfs *pVfs,            /* Pointer to vfs object */
  സ്ഥിരം അക്ഷരം *zPath,            /* Possibly relative input path */
  പൂർണ്ണസംഖ്യ nOut,                     /* Size of output buffer in bytes */
  അക്ഷരം *zOut                    /* Output buffer */
){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  തിരികെ pParent->xFullPathname(pParent, zPath, nOut, zOut);
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
സ്ഥിതം ശൂന്യം *fsDlOpen(sqlite3_vfs *pVfs, സ്ഥിരം അക്ഷരം *zPath){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  തിരികെ pParent->xDlOpen(pParent, zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
സ്ഥിതം ശൂന്യം fsDlError(sqlite3_vfs *pVfs, പൂർണ്ണസംഖ്യ nByte, അക്ഷരം *z_Err_സന്ദേശം){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  pParent->xDlError(pParent, nByte, z_Err_സന്ദേശം);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
സ്ഥിതം ശൂന്യം (*fsDlSym(sqlite3_vfs *pVfs, ശൂന്യം *pH, സ്ഥിരം അക്ഷരം *zSym))(ശൂന്യം){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  തിരികെ pParent->xDlSym(pParent, pH, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
സ്ഥിതം ശൂന്യം fsDlClose(sqlite3_vfs *pVfs, ശൂന്യം *pHandle){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  pParent->xDlClose(pParent, pHandle);
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ fsRandomness(sqlite3_vfs *pVfs, പൂർണ്ണസംഖ്യ nByte, അക്ഷരം *zBufOut){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  തിരികെ pParent->xRandomness(pParent, nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ fsSleep(sqlite3_vfs *pVfs, പൂർണ്ണസംഖ്യ nMicro){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  തിരികെ pParent->xSleep(pParent, nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ fsCurrentTime(sqlite3_vfs *pVfs, ഇരട്ട *pTimeOut){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  തിരികെ pParent->xCurrentTime(pParent, pTimeOut);
}

/*
** This procedure registers the fs vfs with SQLite. If the argument is
** true, the fs vfs becomes the new default vfs. It is the only publicly
** available function in this file.
*/
പൂർണ്ണസംഖ്യ fs_register(ശൂന്യം){
  എങ്കിൽ( fs_vfs.pParent ) തിരികെ SQLITE_OK;
  fs_vfs.pParent = sqlite3_vfs_find(0);
  fs_vfs.base.mxPathname = fs_vfs.pParent->mxPathname;
  fs_vfs.base.szOsFile = MAX(വലുപ്പം(tmp_file), വലുപ്പം(fs_file));
  തിരികെ sqlite3_vfs_register(&fs_vfs.base, 0);
}

#ifdef SQLITE_TEST
  പൂർണ്ണസംഖ്യ SqlitetestOnefile_Init() {തിരികെ fs_register();}
#endif
