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

#ਅਨ੍ਤਰ੍ਭਾਵ "sqlite3.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>

/*
** Maximum pathname length supported by the fs backend.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BLOCKSIZE 512
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BLOBSIZE 10485760

/*
** Name used to identify this VFS.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FS_VFS_NAME "fs"

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ fs_real_file fs_real_file;
ਬਣਤਰ fs_real_file {
  sqlite3_file *pFile;
  ਅਚਲ ਅੱਖਰ *zName;
  ਪੂਰਨ nDatabase;              /* Current size of database region */
  ਪੂਰਨ nJournal;               /* Current size of journal region */
  ਪੂਰਨ nBlob;                  /* Total size of allocated blob */
  ਪੂਰਨ nRef;                   /* Number of pointers to this structure */
  fs_real_file *p_ਅਗਲਾ;
  fs_real_file **ppThis;
};

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ fs_file fs_file;
ਬਣਤਰ fs_file {
  sqlite3_file ਅਧਾਰ;
  ਪੂਰਨ eType;
  fs_real_file *pReal;
};

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ tmp_file tmp_file;
ਬਣਤਰ tmp_file {
  sqlite3_file ਅਧਾਰ;
  ਪੂਰਨ n_ਮਾਪ;
  ਪੂਰਨ nAlloc;
  ਅੱਖਰ *zAlloc;
};

/* Values for fs_file.eType. */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DATABASE_FILE   1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JOURNAL_FILE    2

/*
** Method declarations for fs_file.
*/
ਸਥਿਰ ਪੂਰਨ fsClose(sqlite3_file*);
ਸਥਿਰ ਪੂਰਨ fsRead(sqlite3_file*, ਖਾਲੀ*, ਪੂਰਨ iAmt, sqlite3_int64 iOfst);
ਸਥਿਰ ਪੂਰਨ fsWrite(sqlite3_file*, ਅਚਲ ਖਾਲੀ*, ਪੂਰਨ iAmt, sqlite3_int64 iOfst);
ਸਥਿਰ ਪੂਰਨ fsTruncate(sqlite3_file*, sqlite3_int64 ਮਾਪ);
ਸਥਿਰ ਪੂਰਨ fsSync(sqlite3_file*, ਪੂਰਨ flags);
ਸਥਿਰ ਪੂਰਨ fs_File_ਮਾਪ(sqlite3_file*, sqlite3_int64 *p_ਮਾਪ);
ਸਥਿਰ ਪੂਰਨ fs_ਤਾਲਾ(sqlite3_file*, ਪੂਰਨ);
ਸਥਿਰ ਪੂਰਨ fsUnlock(sqlite3_file*, ਪੂਰਨ);
ਸਥਿਰ ਪੂਰਨ fs_Check_Reserved_ਤਾਲਾ(sqlite3_file*, ਪੂਰਨ *p_Res_ਬਾਹਰਲਾ);
ਸਥਿਰ ਪੂਰਨ fsFileControl(sqlite3_file*, ਪੂਰਨ op, ਖਾਲੀ *pArg);
ਸਥਿਰ ਪੂਰਨ fs_Sector_ਮਾਪ(sqlite3_file*);
ਸਥਿਰ ਪੂਰਨ fsDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for tmp_file.
*/
ਸਥਿਰ ਪੂਰਨ tmpClose(sqlite3_file*);
ਸਥਿਰ ਪੂਰਨ tmpRead(sqlite3_file*, ਖਾਲੀ*, ਪੂਰਨ iAmt, sqlite3_int64 iOfst);
ਸਥਿਰ ਪੂਰਨ tmpWrite(sqlite3_file*, ਅਚਲ ਖਾਲੀ*, ਪੂਰਨ iAmt, sqlite3_int64 iOfst);
ਸਥਿਰ ਪੂਰਨ tmpTruncate(sqlite3_file*, sqlite3_int64 ਮਾਪ);
ਸਥਿਰ ਪੂਰਨ tmpSync(sqlite3_file*, ਪੂਰਨ flags);
ਸਥਿਰ ਪੂਰਨ tmp_File_ਮਾਪ(sqlite3_file*, sqlite3_int64 *p_ਮਾਪ);
ਸਥਿਰ ਪੂਰਨ tmp_ਤਾਲਾ(sqlite3_file*, ਪੂਰਨ);
ਸਥਿਰ ਪੂਰਨ tmpUnlock(sqlite3_file*, ਪੂਰਨ);
ਸਥਿਰ ਪੂਰਨ tmp_Check_Reserved_ਤਾਲਾ(sqlite3_file*, ਪੂਰਨ *p_Res_ਬਾਹਰਲਾ);
ਸਥਿਰ ਪੂਰਨ tmpFileControl(sqlite3_file*, ਪੂਰਨ op, ਖਾਲੀ *pArg);
ਸਥਿਰ ਪੂਰਨ tmp_Sector_ਮਾਪ(sqlite3_file*);
ਸਥਿਰ ਪੂਰਨ tmpDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for fs_vfs.
*/
ਸਥਿਰ ਪੂਰਨ fsOpen(sqlite3_vfs*, ਅਚਲ ਅੱਖਰ *, sqlite3_file*, ਪੂਰਨ , ਪੂਰਨ *);
ਸਥਿਰ ਪੂਰਨ fsDelete(sqlite3_vfs*, ਅਚਲ ਅੱਖਰ *zName, ਪੂਰਨ syncDir);
ਸਥਿਰ ਪੂਰਨ fsAccess(sqlite3_vfs*, ਅਚਲ ਅੱਖਰ *zName, ਪੂਰਨ flags, ਪੂਰਨ *);
ਸਥਿਰ ਪੂਰਨ fs_ਭਰਿਆ_Pathname(sqlite3_vfs*, ਅਚਲ ਅੱਖਰ *zName, ਪੂਰਨ n_ਬਾਹਰਲਾ,ਅੱਖਰ *z_ਬਾਹਰਲਾ);
ਸਥਿਰ ਖਾਲੀ *fsDlOpen(sqlite3_vfs*, ਅਚਲ ਅੱਖਰ *zFilename);
ਸਥਿਰ ਖਾਲੀ fsDlError(sqlite3_vfs*, ਪੂਰਨ nByte, ਅੱਖਰ *z_Err_ਸੁਨੇਹਾ);
ਸਥਿਰ ਖਾਲੀ (*fsDlSym(sqlite3_vfs*,ਖਾਲੀ*, ਅਚਲ ਅੱਖਰ *z_ਚਿੰਨ੍ਹ))(ਖਾਲੀ);
ਸਥਿਰ ਖਾਲੀ fsDlClose(sqlite3_vfs*, ਖਾਲੀ*);
ਸਥਿਰ ਪੂਰਨ fsRandomness(sqlite3_vfs*, ਪੂਰਨ nByte, ਅੱਖਰ *z_ਬਾਹਰਲਾ);
ਸਥਿਰ ਪੂਰਨ fsSleep(sqlite3_vfs*, ਪੂਰਨ microseconds);
ਸਥਿਰ ਪੂਰਨ fs_Current_ਸਮਾਂ(sqlite3_vfs*, ਦੁੱਗਣਾ*);


ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ fs_vfs_t fs_vfs_t;
ਬਣਤਰ fs_vfs_t {
  sqlite3_vfs ਅਧਾਰ;
  fs_real_file *pFileList;
  sqlite3_vfs *p_ਮਾਪਾ;
};

ਸਥਿਰ fs_vfs_t fs_vfs = {
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
    fs_ਭਰਿਆ_Pathname,                             /* xFullPathname */
    fsDlOpen,                                   /* xDlOpen */
    fsDlError,                                  /* xDlError */
    fsDlSym,                                    /* xDlSym */
    fsDlClose,                                  /* xDlClose */
    fsRandomness,                               /* xRandomness */
    fsSleep,                                    /* xSleep */
    fs_Current_ਸਮਾਂ,                              /* xCurrentTime */
    0                                           /* xCurrentTimeInt64 */
  }, 
  0,                                            /* pFileList */
  0                                             /* pParent */
};

ਸਥਿਰ sqlite3_io_methods fs_io_methods = {
  1,                            /* iVersion */
  fsClose,                      /* xClose */
  fsRead,                       /* xRead */
  fsWrite,                      /* xWrite */
  fsTruncate,                   /* xTruncate */
  fsSync,                       /* xSync */
  fs_File_ਮਾਪ,                   /* xFileSize */
  fs_ਤਾਲਾ,                       /* xLock */
  fsUnlock,                     /* xUnlock */
  fs_Check_Reserved_ਤਾਲਾ,          /* xCheckReservedLock */
  fsFileControl,                /* xFileControl */
  fs_Sector_ਮਾਪ,                 /* xSectorSize */
  fsDeviceCharacteristics,      /* xDeviceCharacteristics */
  0,                            /* xShmMap */
  0,                            /* xShmLock */
  0,                            /* xShmBarrier */
  0                             /* xShmUnmap */
};


ਸਥਿਰ sqlite3_io_methods tmp_io_methods = {
  1,                            /* iVersion */
  tmpClose,                     /* xClose */
  tmpRead,                      /* xRead */
  tmpWrite,                     /* xWrite */
  tmpTruncate,                  /* xTruncate */
  tmpSync,                      /* xSync */
  tmp_File_ਮਾਪ,                  /* xFileSize */
  tmp_ਤਾਲਾ,                      /* xLock */
  tmpUnlock,                    /* xUnlock */
  tmp_Check_Reserved_ਤਾਲਾ,         /* xCheckReservedLock */
  tmpFileControl,               /* xFileControl */
  tmp_Sector_ਮਾਪ,                /* xSectorSize */
  tmpDeviceCharacteristics,     /* xDeviceCharacteristics */
  0,                            /* xShmMap */
  0,                            /* xShmLock */
  0,                            /* xShmBarrier */
  0                             /* xShmUnmap */
};

/* Useful macros used in several places */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MIN(x,y) ((x)<(y)?(x):(y))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAX(x,y) ((x)>(y)?(x):(y))


/*
** Close a tmp-file.
*/
ਸਥਿਰ ਪੂਰਨ tmpClose(sqlite3_file *pFile){
  tmp_file *pTmp = (tmp_file *)pFile;
  sqlite3_free(pTmp->zAlloc);
  ਵਾਪਸ SQLITE_OK;
}

/*
** Read data from a tmp-file.
*/
ਸਥਿਰ ਪੂਰਨ tmpRead(
  sqlite3_file *pFile, 
  ਖਾਲੀ *zBuf, 
  ਪੂਰਨ iAmt, 
  sqlite_int64 iOfst
){
  tmp_file *pTmp = (tmp_file *)pFile;
  ਜੇ( (iAmt+iOfst)>pTmp->n_ਮਾਪ ){
    ਵਾਪਸ SQLITE_IOERR_SHORT_READ;
  }
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(zBuf, &pTmp->zAlloc[iOfst], iAmt);
  ਵਾਪਸ SQLITE_OK;
}

/*
** Write data to a tmp-file.
*/
ਸਥਿਰ ਪੂਰਨ tmpWrite(
  sqlite3_file *pFile, 
  ਅਚਲ ਖਾਲੀ *zBuf, 
  ਪੂਰਨ iAmt, 
  sqlite_int64 iOfst
){
  tmp_file *pTmp = (tmp_file *)pFile;
  ਜੇ( (iAmt+iOfst)>pTmp->nAlloc ){
    ਪੂਰਨ nNew = (ਪੂਰਨ)(2*(iAmt+iOfst+pTmp->nAlloc));
    ਅੱਖਰ *zNew = sqlite3_realloc(pTmp->zAlloc, nNew);
    ਜੇ( !zNew ){
      ਵਾਪਸ SQLITE_NOMEM;
    }
    pTmp->zAlloc = zNew;
    pTmp->nAlloc = nNew;
  }
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(&pTmp->zAlloc[iOfst], zBuf, iAmt);
  pTmp->n_ਮਾਪ = (ਪੂਰਨ)MAX(pTmp->n_ਮਾਪ, iOfst+iAmt);
  ਵਾਪਸ SQLITE_OK;
}

/*
** Truncate a tmp-file.
*/
ਸਥਿਰ ਪੂਰਨ tmpTruncate(sqlite3_file *pFile, sqlite_int64 ਮਾਪ){
  tmp_file *pTmp = (tmp_file *)pFile;
  pTmp->n_ਮਾਪ = (ਪੂਰਨ)MIN(pTmp->n_ਮਾਪ, ਮਾਪ);
  ਵਾਪਸ SQLITE_OK;
}

/*
** Sync a tmp-file.
*/
ਸਥਿਰ ਪੂਰਨ tmpSync(sqlite3_file *pFile, ਪੂਰਨ flags){
  ਵਾਪਸ SQLITE_OK;
}

/*
** Return the current file-size of a tmp-file.
*/
ਸਥਿਰ ਪੂਰਨ tmp_File_ਮਾਪ(sqlite3_file *pFile, sqlite_int64 *p_ਮਾਪ){
  tmp_file *pTmp = (tmp_file *)pFile;
  *p_ਮਾਪ = pTmp->n_ਮਾਪ;
  ਵਾਪਸ SQLITE_OK;
}

/*
** Lock a tmp-file.
*/
ਸਥਿਰ ਪੂਰਨ tmp_ਤਾਲਾ(sqlite3_file *pFile, ਪੂਰਨ e_ਤਾਲਾ){
  ਵਾਪਸ SQLITE_OK;
}

/*
** Unlock a tmp-file.
*/
ਸਥਿਰ ਪੂਰਨ tmpUnlock(sqlite3_file *pFile, ਪੂਰਨ e_ਤਾਲਾ){
  ਵਾਪਸ SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on a tmp-file.
*/
ਸਥਿਰ ਪੂਰਨ tmp_Check_Reserved_ਤਾਲਾ(sqlite3_file *pFile, ਪੂਰਨ *p_Res_ਬਾਹਰਲਾ){
  *p_Res_ਬਾਹਰਲਾ = 0;
  ਵਾਪਸ SQLITE_OK;
}

/*
** File control method. For custom operations on a tmp-file.
*/
ਸਥਿਰ ਪੂਰਨ tmpFileControl(sqlite3_file *pFile, ਪੂਰਨ op, ਖਾਲੀ *pArg){
  ਵਾਪਸ SQLITE_OK;
}

/*
** Return the sector-size in bytes for a tmp-file.
*/
ਸਥਿਰ ਪੂਰਨ tmp_Sector_ਮਾਪ(sqlite3_file *pFile){
  ਵਾਪਸ 0;
}

/*
** Return the device characteristic flags supported by a tmp-file.
*/
ਸਥਿਰ ਪੂਰਨ tmpDeviceCharacteristics(sqlite3_file *pFile){
  ਵਾਪਸ 0;
}

/*
** Close an fs-file.
*/
ਸਥਿਰ ਪੂਰਨ fsClose(sqlite3_file *pFile){
  ਪੂਰਨ rc = SQLITE_OK;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;

  /* Decrement the real_file ref-count. */
  pReal->nRef--;
  ਪ੍ਰਤਿਜ੍ਞਾ(pReal->nRef>=0);

  /* When the ref-count reaches 0, destroy the structure */
  ਜੇ( pReal->nRef==0 ){
    *pReal->ppThis = pReal->p_ਅਗਲਾ;
    ਜੇ( pReal->p_ਅਗਲਾ ){
      pReal->p_ਅਗਲਾ->ppThis = pReal->ppThis;
    }
    rc = pReal->pFile->pMethods->xClose(pReal->pFile);
    sqlite3_free(pReal);
  }

  ਵਾਪਸ rc;
}

/*
** Read data from an fs-file.
*/
ਸਥਿਰ ਪੂਰਨ fsRead(
  sqlite3_file *pFile, 
  ਖਾਲੀ *zBuf, 
  ਪੂਰਨ iAmt, 
  sqlite_int64 iOfst
){
  ਪੂਰਨ rc = SQLITE_OK;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  sqlite3_file *pF = pReal->pFile;

  ਜੇ( (p->eType==DATABASE_FILE && (iAmt+iOfst)>pReal->nDatabase)
   || (p->eType==JOURNAL_FILE && (iAmt+iOfst)>pReal->nJournal)
  ){
    rc = SQLITE_IOERR_SHORT_READ;
  }ਨਹੀਂਤਾਂ ਜੇ( p->eType==DATABASE_FILE ){
    rc = pF->pMethods->xRead(pF, zBuf, iAmt, iOfst+BLOCKSIZE);
  }ਨਹੀਂਤਾਂ{
    /* Journal file. */
    ਪੂਰਨ iRem = iAmt;
    ਪੂਰਨ iBuf = 0;
    ਪੂਰਨ ii = (ਪੂਰਨ)iOfst;
    ਜਦੋਂ( iRem>0 && rc==SQLITE_OK ){
      ਪੂਰਨ iRealOff = pReal->nBlob - BLOCKSIZE*((ii/BLOCKSIZE)+1) + ii%BLOCKSIZE;
      ਪੂਰਨ iRealAmt = MIN(iRem, BLOCKSIZE - (iRealOff%BLOCKSIZE));

      rc = pF->pMethods->xRead(pF, &((ਅੱਖਰ *)zBuf)[iBuf], iRealAmt, iRealOff);
      ii += iRealAmt;
      iBuf += iRealAmt;
      iRem -= iRealAmt;
    }
  }

  ਵਾਪਸ rc;
}

/*
** Write data to an fs-file.
*/
ਸਥਿਰ ਪੂਰਨ fsWrite(
  sqlite3_file *pFile, 
  ਅਚਲ ਖਾਲੀ *zBuf, 
  ਪੂਰਨ iAmt, 
  sqlite_int64 iOfst
){
  ਪੂਰਨ rc = SQLITE_OK;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  sqlite3_file *pF = pReal->pFile;

  ਜੇ( p->eType==DATABASE_FILE ){
    ਜੇ( (iAmt+iOfst+BLOCKSIZE)>(pReal->nBlob-pReal->nJournal) ){
      rc = SQLITE_ਭਰਿਆ;
    }ਨਹੀਂਤਾਂ{
      rc = pF->pMethods->xWrite(pF, zBuf, iAmt, iOfst+BLOCKSIZE);
      ਜੇ( rc==SQLITE_OK ){
        pReal->nDatabase = (ਪੂਰਨ)MAX(pReal->nDatabase, iAmt+iOfst);
      }
    }
  }ਨਹੀਂਤਾਂ{
    /* Journal file. */
    ਪੂਰਨ iRem = iAmt;
    ਪੂਰਨ iBuf = 0;
    ਪੂਰਨ ii = (ਪੂਰਨ)iOfst;
    ਜਦੋਂ( iRem>0 && rc==SQLITE_OK ){
      ਪੂਰਨ iRealOff = pReal->nBlob - BLOCKSIZE*((ii/BLOCKSIZE)+1) + ii%BLOCKSIZE;
      ਪੂਰਨ iRealAmt = MIN(iRem, BLOCKSIZE - (iRealOff%BLOCKSIZE));

      ਜੇ( iRealOff<(pReal->nDatabase+BLOCKSIZE) ){
        rc = SQLITE_ਭਰਿਆ;
      }ਨਹੀਂਤਾਂ{
        rc = pF->pMethods->xWrite(pF, &((ਅੱਖਰ *)zBuf)[iBuf], iRealAmt,iRealOff);
        ii += iRealAmt;
        iBuf += iRealAmt;
        iRem -= iRealAmt;
      }
    }
    ਜੇ( rc==SQLITE_OK ){
      pReal->nJournal = (ਪੂਰਨ)MAX(pReal->nJournal, iAmt+iOfst);
    }
  }

  ਵਾਪਸ rc;
}

/*
** Truncate an fs-file.
*/
ਸਥਿਰ ਪੂਰਨ fsTruncate(sqlite3_file *pFile, sqlite_int64 ਮਾਪ){
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  ਜੇ( p->eType==DATABASE_FILE ){
    pReal->nDatabase = (ਪੂਰਨ)MIN(pReal->nDatabase, ਮਾਪ);
  }ਨਹੀਂਤਾਂ{
    pReal->nJournal = (ਪੂਰਨ)MIN(pReal->nJournal, ਮਾਪ);
  }
  ਵਾਪਸ SQLITE_OK;
}

/*
** Sync an fs-file.
*/
ਸਥਿਰ ਪੂਰਨ fsSync(sqlite3_file *pFile, ਪੂਰਨ flags){
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  sqlite3_file *pRealFile = pReal->pFile;
  ਪੂਰਨ rc = SQLITE_OK;

  ਜੇ( p->eType==DATABASE_FILE ){
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ z_ਮਾਪ[4];
    z_ਮਾਪ[0] = (pReal->nDatabase&0xFF000000)>>24;
    z_ਮਾਪ[1] = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)((pReal->nDatabase&0x00FF0000)>>16);
    z_ਮਾਪ[2] = (pReal->nDatabase&0x0000FF00)>>8;
    z_ਮਾਪ[3] = (pReal->nDatabase&0x000000FF);
    rc = pRealFile->pMethods->xWrite(pRealFile, z_ਮਾਪ, 4, 0);
  }
  ਜੇ( rc==SQLITE_OK ){
    rc = pRealFile->pMethods->xSync(pRealFile, flags&(~SQLITE_SYNC_DATAONLY));
  }

  ਵਾਪਸ rc;
}

/*
** Return the current file-size of an fs-file.
*/
ਸਥਿਰ ਪੂਰਨ fs_File_ਮਾਪ(sqlite3_file *pFile, sqlite_int64 *p_ਮਾਪ){
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  ਜੇ( p->eType==DATABASE_FILE ){
    *p_ਮਾਪ = pReal->nDatabase;
  }ਨਹੀਂਤਾਂ{
    *p_ਮਾਪ = pReal->nJournal;
  }
  ਵਾਪਸ SQLITE_OK;
}

/*
** Lock an fs-file.
*/
ਸਥਿਰ ਪੂਰਨ fs_ਤਾਲਾ(sqlite3_file *pFile, ਪੂਰਨ e_ਤਾਲਾ){
  ਵਾਪਸ SQLITE_OK;
}

/*
** Unlock an fs-file.
*/
ਸਥਿਰ ਪੂਰਨ fsUnlock(sqlite3_file *pFile, ਪੂਰਨ e_ਤਾਲਾ){
  ਵਾਪਸ SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on an fs-file.
*/
ਸਥਿਰ ਪੂਰਨ fs_Check_Reserved_ਤਾਲਾ(sqlite3_file *pFile, ਪੂਰਨ *p_Res_ਬਾਹਰਲਾ){
  *p_Res_ਬਾਹਰਲਾ = 0;
  ਵਾਪਸ SQLITE_OK;
}

/*
** File control method. For custom operations on an fs-file.
*/
ਸਥਿਰ ਪੂਰਨ fsFileControl(sqlite3_file *pFile, ਪੂਰਨ op, ਖਾਲੀ *pArg){
  ਜੇ( op==SQLITE_FCNTL_PRAGMA ) ਵਾਪਸ SQLITE_NOTFOUND;
  ਵਾਪਸ SQLITE_OK;
}

/*
** Return the sector-size in bytes for an fs-file.
*/
ਸਥਿਰ ਪੂਰਨ fs_Sector_ਮਾਪ(sqlite3_file *pFile){
  ਵਾਪਸ BLOCKSIZE;
}

/*
** Return the device characteristic flags supported by an fs-file.
*/
ਸਥਿਰ ਪੂਰਨ fsDeviceCharacteristics(sqlite3_file *pFile){
  ਵਾਪਸ 0;
}

/*
** Open an fs file handle.
*/
ਸਥਿਰ ਪੂਰਨ fsOpen(
  sqlite3_vfs *pVfs,
  ਅਚਲ ਅੱਖਰ *zName,
  sqlite3_file *pFile,
  ਪੂਰਨ flags,
  ਪੂਰਨ *p_ਬਾਹਰਲਾ_Flags
){
  fs_vfs_t *pFsVfs = (fs_vfs_t *)pVfs;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = 0;
  ਪੂਰਨ eType;
  ਪੂਰਨ nName;
  ਪੂਰਨ rc = SQLITE_OK;

  ਜੇ( 0==(flags&(SQLITE_OPEN_MAIN_DB|SQLITE_OPEN_MAIN_JOURNAL)) ){
    tmp_file *p2 = (tmp_file *)pFile;
    memset(p2, 0, ਆਕਾਰ(*p2));
    p2->ਅਧਾਰ.pMethods = &tmp_io_methods;
    ਵਾਪਸ SQLITE_OK;
  }

  eType = ((flags&(SQLITE_OPEN_MAIN_DB))?DATABASE_FILE:JOURNAL_FILE);
  p->ਅਧਾਰ.pMethods = &fs_io_methods;
  p->eType = eType;

  ਪ੍ਰਤਿਜ੍ਞਾ(ਲੜੀ_ਲੰਬਾਈ("-journal")==8);
  nName = (ਪੂਰਨ)ਲੜੀ_ਲੰਬਾਈ(zName)-((eType==JOURNAL_FILE)?8:0);
  pReal=pFsVfs->pFileList; 
  ਲਈ(; pReal && ਲੜੀ_ਤੁਲਨਾ(pReal->zName, zName, nName); pReal=pReal->p_ਅਗਲਾ);

  ਜੇ( !pReal ){
    ਪੂਰਨ real_flags = (flags&~(SQLITE_OPEN_MAIN_DB))|SQLITE_OPEN_ਆਰਜ਼ੀ_DB;
    sqlite3_int64 ਮਾਪ;
    sqlite3_file *pRealFile;
    sqlite3_vfs *p_ਮਾਪਾ = pFsVfs->p_ਮਾਪਾ;
    ਪ੍ਰਤਿਜ੍ਞਾ(eType==DATABASE_FILE);

    pReal = (fs_real_file *)sqlite3_malloc(ਆਕਾਰ(*pReal)+p_ਮਾਪਾ->szOsFile);
    ਜੇ( !pReal ){
      rc = SQLITE_NOMEM;
      ਜਾਓ open_ਬਾਹਰਲਾ;
    }
    memset(pReal, 0, ਆਕਾਰ(*pReal)+p_ਮਾਪਾ->szOsFile);
    pReal->zName = zName;
    pReal->pFile = (sqlite3_file *)(&pReal[1]);

    rc = p_ਮਾਪਾ->xOpen(p_ਮਾਪਾ, zName, pReal->pFile, real_flags, p_ਬਾਹਰਲਾ_Flags);
    ਜੇ( rc!=SQLITE_OK ){
      ਜਾਓ open_ਬਾਹਰਲਾ;
    }
    pRealFile = pReal->pFile;

    rc = pRealFile->pMethods->x_File_ਮਾਪ(pRealFile, &ਮਾਪ);
    ਜੇ( rc!=SQLITE_OK ){
      ਜਾਓ open_ਬਾਹਰਲਾ;
    }
    ਜੇ( ਮਾਪ==0 ){
      rc = pRealFile->pMethods->xWrite(pRealFile, "\0", 1, BLOBSIZE-1);
      pReal->nBlob = BLOBSIZE;
    }ਨਹੀਂਤਾਂ{
      ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ zS[4];
      pReal->nBlob = (ਪੂਰਨ)ਮਾਪ;
      rc = pRealFile->pMethods->xRead(pRealFile, zS, 4, 0);
      pReal->nDatabase = (zS[0]<<24)+(zS[1]<<16)+(zS[2]<<8)+zS[3];
      ਜੇ( rc==SQLITE_OK ){
        rc = pRealFile->pMethods->xRead(pRealFile, zS, 4, pReal->nBlob-4);
        ਜੇ( zS[0] || zS[1] || zS[2] || zS[3] ){
          pReal->nJournal = pReal->nBlob;
        }
      }
    }

    ਜੇ( rc==SQLITE_OK ){
      pReal->p_ਅਗਲਾ = pFsVfs->pFileList;
      ਜੇ( pReal->p_ਅਗਲਾ ){
        pReal->p_ਅਗਲਾ->ppThis = &pReal->p_ਅਗਲਾ;
      }
      pReal->ppThis = &pFsVfs->pFileList;
      pFsVfs->pFileList = pReal;
    }
  }

open_ਬਾਹਰਲਾ:
  ਜੇ( pReal ){
    ਜੇ( rc==SQLITE_OK ){
      p->pReal = pReal;
      pReal->nRef++;
    }ਨਹੀਂਤਾਂ{
      ਜੇ( pReal->pFile->pMethods ){
        pReal->pFile->pMethods->xClose(pReal->pFile);
      }
      sqlite3_free(pReal);
    }
  }
  ਵਾਪਸ rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
ਸਥਿਰ ਪੂਰਨ fsDelete(sqlite3_vfs *pVfs, ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ, ਪੂਰਨ dirSync){
  ਪੂਰਨ rc = SQLITE_OK;
  fs_vfs_t *pFsVfs = (fs_vfs_t *)pVfs;
  fs_real_file *pReal;
  sqlite3_file *pF;
  ਪੂਰਨ nName = (ਪੂਰਨ)ਲੜੀ_ਲੰਬਾਈ(z_ਰਸਤਾ) - 8;

  ਪ੍ਰਤਿਜ੍ਞਾ(ਲੜੀ_ਲੰਬਾਈ("-journal")==8);
  ਪ੍ਰਤਿਜ੍ਞਾ(strcmp("-journal", &z_ਰਸਤਾ[nName])==0);

  pReal = pFsVfs->pFileList; 
  ਲਈ(; pReal && ਲੜੀ_ਤੁਲਨਾ(pReal->zName, z_ਰਸਤਾ, nName); pReal=pReal->p_ਅਗਲਾ);
  ਜੇ( pReal ){
    pF = pReal->pFile;
    rc = pF->pMethods->xWrite(pF, "\0\0\0\0", 4, pReal->nBlob-BLOCKSIZE);
    ਜੇ( rc==SQLITE_OK ){
      pReal->nJournal = 0;
    }
  }
  ਵਾਪਸ rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
ਸਥਿਰ ਪੂਰਨ fsAccess(
  sqlite3_vfs *pVfs, 
  ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ, 
  ਪੂਰਨ flags, 
  ਪੂਰਨ *p_Res_ਬਾਹਰਲਾ
){
  fs_vfs_t *pFsVfs = (fs_vfs_t *)pVfs;
  fs_real_file *pReal;
  ਪੂਰਨ isJournal = 0;
  ਪੂਰਨ nName = (ਪੂਰਨ)ਲੜੀ_ਲੰਬਾਈ(z_ਰਸਤਾ);

  ਜੇ( flags!=SQLITE_ACCESS_EXISTS ){
    sqlite3_vfs *p_ਮਾਪਾ = ((fs_vfs_t *)pVfs)->p_ਮਾਪਾ;
    ਵਾਪਸ p_ਮਾਪਾ->xAccess(p_ਮਾਪਾ, z_ਰਸਤਾ, flags, p_Res_ਬਾਹਰਲਾ);
  }

  ਪ੍ਰਤਿਜ੍ਞਾ(ਲੜੀ_ਲੰਬਾਈ("-journal")==8);
  ਜੇ( nName>8 && strcmp("-journal", &z_ਰਸਤਾ[nName-8])==0 ){
    nName -= 8;
    isJournal = 1;
  }

  pReal = pFsVfs->pFileList; 
  ਲਈ(; pReal && ਲੜੀ_ਤੁਲਨਾ(pReal->zName, z_ਰਸਤਾ, nName); pReal=pReal->p_ਅਗਲਾ);

  *p_Res_ਬਾਹਰਲਾ = (pReal && (!isJournal || pReal->nJournal>0));
  ਵਾਪਸ SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (FS_MAX_PATHNAME+1) bytes.
*/
ਸਥਿਰ ਪੂਰਨ fs_ਭਰਿਆ_Pathname(
  sqlite3_vfs *pVfs,            /* Pointer to vfs object */
  ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ,            /* Possibly relative input path */
  ਪੂਰਨ n_ਬਾਹਰਲਾ,                     /* Size of output buffer in bytes */
  ਅੱਖਰ *z_ਬਾਹਰਲਾ                    /* Output buffer */
){
  sqlite3_vfs *p_ਮਾਪਾ = ((fs_vfs_t *)pVfs)->p_ਮਾਪਾ;
  ਵਾਪਸ p_ਮਾਪਾ->x_ਭਰਿਆ_Pathname(p_ਮਾਪਾ, z_ਰਸਤਾ, n_ਬਾਹਰਲਾ, z_ਬਾਹਰਲਾ);
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
ਸਥਿਰ ਖਾਲੀ *fsDlOpen(sqlite3_vfs *pVfs, ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ){
  sqlite3_vfs *p_ਮਾਪਾ = ((fs_vfs_t *)pVfs)->p_ਮਾਪਾ;
  ਵਾਪਸ p_ਮਾਪਾ->xDlOpen(p_ਮਾਪਾ, z_ਰਸਤਾ);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
ਸਥਿਰ ਖਾਲੀ fsDlError(sqlite3_vfs *pVfs, ਪੂਰਨ nByte, ਅੱਖਰ *z_Err_ਸੁਨੇਹਾ){
  sqlite3_vfs *p_ਮਾਪਾ = ((fs_vfs_t *)pVfs)->p_ਮਾਪਾ;
  p_ਮਾਪਾ->xDlError(p_ਮਾਪਾ, nByte, z_Err_ਸੁਨੇਹਾ);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
ਸਥਿਰ ਖਾਲੀ (*fsDlSym(sqlite3_vfs *pVfs, ਖਾਲੀ *pH, ਅਚਲ ਅੱਖਰ *zSym))(ਖਾਲੀ){
  sqlite3_vfs *p_ਮਾਪਾ = ((fs_vfs_t *)pVfs)->p_ਮਾਪਾ;
  ਵਾਪਸ p_ਮਾਪਾ->xDlSym(p_ਮਾਪਾ, pH, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
ਸਥਿਰ ਖਾਲੀ fsDlClose(sqlite3_vfs *pVfs, ਖਾਲੀ *pHandle){
  sqlite3_vfs *p_ਮਾਪਾ = ((fs_vfs_t *)pVfs)->p_ਮਾਪਾ;
  p_ਮਾਪਾ->xDlClose(p_ਮਾਪਾ, pHandle);
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
ਸਥਿਰ ਪੂਰਨ fsRandomness(sqlite3_vfs *pVfs, ਪੂਰਨ nByte, ਅੱਖਰ *z_Buf_ਬਾਹਰਲਾ){
  sqlite3_vfs *p_ਮਾਪਾ = ((fs_vfs_t *)pVfs)->p_ਮਾਪਾ;
  ਵਾਪਸ p_ਮਾਪਾ->xRandomness(p_ਮਾਪਾ, nByte, z_Buf_ਬਾਹਰਲਾ);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
ਸਥਿਰ ਪੂਰਨ fsSleep(sqlite3_vfs *pVfs, ਪੂਰਨ nMicro){
  sqlite3_vfs *p_ਮਾਪਾ = ((fs_vfs_t *)pVfs)->p_ਮਾਪਾ;
  ਵਾਪਸ p_ਮਾਪਾ->xSleep(p_ਮਾਪਾ, nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
ਸਥਿਰ ਪੂਰਨ fs_Current_ਸਮਾਂ(sqlite3_vfs *pVfs, ਦੁੱਗਣਾ *p_ਸਮਾਂ_ਬਾਹਰਲਾ){
  sqlite3_vfs *p_ਮਾਪਾ = ((fs_vfs_t *)pVfs)->p_ਮਾਪਾ;
  ਵਾਪਸ p_ਮਾਪਾ->x_Current_ਸਮਾਂ(p_ਮਾਪਾ, p_ਸਮਾਂ_ਬਾਹਰਲਾ);
}

/*
** This procedure registers the fs vfs with SQLite. If the argument is
** true, the fs vfs becomes the new default vfs. It is the only publicly
** available function in this file.
*/
ਪੂਰਨ fs_ਰਜਿਸਟਰ(ਖਾਲੀ){
  ਜੇ( fs_vfs.p_ਮਾਪਾ ) ਵਾਪਸ SQLITE_OK;
  fs_vfs.p_ਮਾਪਾ = sqlite3_vfs_ਲੱਭੋ(0);
  fs_vfs.ਅਧਾਰ.mxPathname = fs_vfs.p_ਮਾਪਾ->mxPathname;
  fs_vfs.ਅਧਾਰ.szOsFile = MAX(ਆਕਾਰ(tmp_file), ਆਕਾਰ(fs_file));
  ਵਾਪਸ sqlite3_vfs_ਰਜਿਸਟਰ(&fs_vfs.ਅਧਾਰ, 0);
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_TEST
  ਪੂਰਨ SqlitetestOnefile_Init() {ਵਾਪਸ fs_ਰਜਿਸਟਰ();}
#ਜੇ_ਅੰਤ
