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

#ಅನ್ತರ್ಭಾವ "sqlite3.h"
#ಅನ್ತರ್ಭಾವ <assert.h>
#ಅನ್ತರ್ಭಾವ <string.h>

/*
** Maximum pathname length supported by the fs backend.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು BLOCKSIZE 512
#ವ್ಯಾಖ್ಯಾನಿಸು BLOBSIZE 10485760

/*
** Name used to identify this VFS.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು FS_VFS_NAME "fs"

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ fs_real_file fs_real_file;
ರಚನೆ fs_real_file {
  sqlite3_file *pFile;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName;
  ಪೂರ್ಣಾಂಕ nDatabase;              /* Current size of database region */
  ಪೂರ್ಣಾಂಕ nJournal;               /* Current size of journal region */
  ಪೂರ್ಣಾಂಕ nBlob;                  /* Total size of allocated blob */
  ಪೂರ್ಣಾಂಕ nRef;                   /* Number of pointers to this structure */
  fs_real_file *p_ಮುಂದಿನ;
  fs_real_file **ppThis;
};

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ fs_file fs_file;
ರಚನೆ fs_file {
  sqlite3_file ತಳ;
  ಪೂರ್ಣಾಂಕ eType;
  fs_real_file *pReal;
};

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ tmp_file tmp_file;
ರಚನೆ tmp_file {
  sqlite3_file ತಳ;
  ಪೂರ್ಣಾಂಕ n_ಗಾತ್ರ;
  ಪೂರ್ಣಾಂಕ nAlloc;
  ಅಕ್ಷರ *zAlloc;
};

/* Values for fs_file.eType. */
#ವ್ಯಾಖ್ಯಾನಿಸು DATABASE_FILE   1
#ವ್ಯಾಖ್ಯಾನಿಸು JOURNAL_FILE    2

/*
** Method declarations for fs_file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fsClose(sqlite3_file*);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fsRead(sqlite3_file*, ಶೂನ್ಯ*, ಪೂರ್ಣಾಂಕ iAmt, sqlite3_int64 iOfst);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fsWrite(sqlite3_file*, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*, ಪೂರ್ಣಾಂಕ iAmt, sqlite3_int64 iOfst);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fsTruncate(sqlite3_file*, sqlite3_int64 ಗಾತ್ರ_2);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fsSync(sqlite3_file*, ಪೂರ್ಣಾಂಕ flags);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fs_File_ಗಾತ್ರ(sqlite3_file*, sqlite3_int64 *p_ಗಾತ್ರ);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fs_ಬೀಗ(sqlite3_file*, ಪೂರ್ಣಾಂಕ);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fsUnlock(sqlite3_file*, ಪೂರ್ಣಾಂಕ);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fs_Check_Reserved_ಬೀಗ(sqlite3_file*, ಪೂರ್ಣಾಂಕ *p_Res_ಹೊರಹೋಗು);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fsFileControl(sqlite3_file*, ಪೂರ್ಣಾಂಕ op, ಶೂನ್ಯ *pArg);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fs_Sector_ಗಾತ್ರ(sqlite3_file*);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fsDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for tmp_file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tmpClose(sqlite3_file*);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tmpRead(sqlite3_file*, ಶೂನ್ಯ*, ಪೂರ್ಣಾಂಕ iAmt, sqlite3_int64 iOfst);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tmpWrite(sqlite3_file*, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*, ಪೂರ್ಣಾಂಕ iAmt, sqlite3_int64 iOfst);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tmpTruncate(sqlite3_file*, sqlite3_int64 ಗಾತ್ರ_2);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tmpSync(sqlite3_file*, ಪೂರ್ಣಾಂಕ flags);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tmp_File_ಗಾತ್ರ(sqlite3_file*, sqlite3_int64 *p_ಗಾತ್ರ);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tmp_ಬೀಗ(sqlite3_file*, ಪೂರ್ಣಾಂಕ);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tmpUnlock(sqlite3_file*, ಪೂರ್ಣಾಂಕ);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tmp_Check_Reserved_ಬೀಗ(sqlite3_file*, ಪೂರ್ಣಾಂಕ *p_Res_ಹೊರಹೋಗು);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tmpFileControl(sqlite3_file*, ಪೂರ್ಣಾಂಕ op, ಶೂನ್ಯ *pArg);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tmp_Sector_ಗಾತ್ರ(sqlite3_file*);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tmpDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for fs_vfs.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fsOpen(sqlite3_vfs*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *, sqlite3_file*, ಪೂರ್ಣಾಂಕ , ಪೂರ್ಣಾಂಕ *);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fsDelete(sqlite3_vfs*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName, ಪೂರ್ಣಾಂಕ syncDir);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fsAccess(sqlite3_vfs*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName, ಪೂರ್ಣಾಂಕ flags, ಪೂರ್ಣಾಂಕ *);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fs_ತುಂಬಿದ_Pathname(sqlite3_vfs*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName, ಪೂರ್ಣಾಂಕ n_ಹೊರಹೋಗು,ಅಕ್ಷರ *z_ಹೊರಹೋಗು);
ಸ್ಥಿರ ಶೂನ್ಯ *fsDlOpen(sqlite3_vfs*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFilename);
ಸ್ಥಿರ ಶೂನ್ಯ fsDlError(sqlite3_vfs*, ಪೂರ್ಣಾಂಕ nByte, ಅಕ್ಷರ *z_Err_ಸಂದೇಶ);
ಸ್ಥಿರ ಶೂನ್ಯ (*fsDlSym(sqlite3_vfs*,ಶೂನ್ಯ*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಚಿಹ್ನೆ))(ಶೂನ್ಯ);
ಸ್ಥಿರ ಶೂನ್ಯ fsDlClose(sqlite3_vfs*, ಶೂನ್ಯ*);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fsRandomness(sqlite3_vfs*, ಪೂರ್ಣಾಂಕ nByte, ಅಕ್ಷರ *z_ಹೊರಹೋಗು);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fsSleep(sqlite3_vfs*, ಪೂರ್ಣಾಂಕ microseconds);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fs_Current_ಸಮಯ(sqlite3_vfs*, ದ್ವಿಗುಣ*);


ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ fs_vfs_t fs_vfs_t;
ರಚನೆ fs_vfs_t {
  sqlite3_vfs ತಳ;
  fs_real_file *pFileList;
  sqlite3_vfs *p_ಪೋಷಕ;
};

ಸ್ಥಿರ fs_vfs_t fs_vfs = {
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
    fs_ತುಂಬಿದ_Pathname,                             /* xFullPathname */
    fsDlOpen,                                   /* xDlOpen */
    fsDlError,                                  /* xDlError */
    fsDlSym,                                    /* xDlSym */
    fsDlClose,                                  /* xDlClose */
    fsRandomness,                               /* xRandomness */
    fsSleep,                                    /* xSleep */
    fs_Current_ಸಮಯ,                              /* xCurrentTime */
    0                                           /* xCurrentTimeInt64 */
  }, 
  0,                                            /* pFileList */
  0                                             /* pParent */
};

ಸ್ಥಿರ sqlite3_io_methods fs_io_methods = {
  1,                            /* iVersion */
  fsClose,                      /* xClose */
  fsRead,                       /* xRead */
  fsWrite,                      /* xWrite */
  fsTruncate,                   /* xTruncate */
  fsSync,                       /* xSync */
  fs_File_ಗಾತ್ರ,                   /* xFileSize */
  fs_ಬೀಗ,                       /* xLock */
  fsUnlock,                     /* xUnlock */
  fs_Check_Reserved_ಬೀಗ,          /* xCheckReservedLock */
  fsFileControl,                /* xFileControl */
  fs_Sector_ಗಾತ್ರ,                 /* xSectorSize */
  fsDeviceCharacteristics,      /* xDeviceCharacteristics */
  0,                            /* xShmMap */
  0,                            /* xShmLock */
  0,                            /* xShmBarrier */
  0                             /* xShmUnmap */
};


ಸ್ಥಿರ sqlite3_io_methods tmp_io_methods = {
  1,                            /* iVersion */
  tmpClose,                     /* xClose */
  tmpRead,                      /* xRead */
  tmpWrite,                     /* xWrite */
  tmpTruncate,                  /* xTruncate */
  tmpSync,                      /* xSync */
  tmp_File_ಗಾತ್ರ,                  /* xFileSize */
  tmp_ಬೀಗ,                      /* xLock */
  tmpUnlock,                    /* xUnlock */
  tmp_Check_Reserved_ಬೀಗ,         /* xCheckReservedLock */
  tmpFileControl,               /* xFileControl */
  tmp_Sector_ಗಾತ್ರ,                /* xSectorSize */
  tmpDeviceCharacteristics,     /* xDeviceCharacteristics */
  0,                            /* xShmMap */
  0,                            /* xShmLock */
  0,                            /* xShmBarrier */
  0                             /* xShmUnmap */
};

/* Useful macros used in several places */
#ವ್ಯಾಖ್ಯಾನಿಸು MIN(x,y) ((x)<(y)?(x):(y))
#ವ್ಯಾಖ್ಯಾನಿಸು MAX(x,y) ((x)>(y)?(x):(y))


/*
** Close a tmp-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tmpClose(sqlite3_file *pFile){
  tmp_file *pTmp = (tmp_file *)pFile;
  sqlite3_free(pTmp->zAlloc);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Read data from a tmp-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tmpRead(
  sqlite3_file *pFile, 
  ಶೂನ್ಯ *zBuf, 
  ಪೂರ್ಣಾಂಕ iAmt, 
  sqlite_int64 iOfst
){
  tmp_file *pTmp = (tmp_file *)pFile;
  ಆದರೆ( (iAmt+iOfst)>pTmp->n_ಗಾತ್ರ ){
    ಹಿಂತಿರುಗಿ SQLITE_IOERR_SHORT_READ;
  }
  ಸ್ಮರಣೆ_ನಕಲು(zBuf, &pTmp->zAlloc[iOfst], iAmt);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Write data to a tmp-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tmpWrite(
  sqlite3_file *pFile, 
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *zBuf, 
  ಪೂರ್ಣಾಂಕ iAmt, 
  sqlite_int64 iOfst
){
  tmp_file *pTmp = (tmp_file *)pFile;
  ಆದರೆ( (iAmt+iOfst)>pTmp->nAlloc ){
    ಪೂರ್ಣಾಂಕ nNew = (ಪೂರ್ಣಾಂಕ)(2*(iAmt+iOfst+pTmp->nAlloc));
    ಅಕ್ಷರ *zNew = sqlite3_realloc(pTmp->zAlloc, nNew);
    ಆದರೆ( !zNew ){
      ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
    }
    pTmp->zAlloc = zNew;
    pTmp->nAlloc = nNew;
  }
  ಸ್ಮರಣೆ_ನಕಲು(&pTmp->zAlloc[iOfst], zBuf, iAmt);
  pTmp->n_ಗಾತ್ರ = (ಪೂರ್ಣಾಂಕ)MAX(pTmp->n_ಗಾತ್ರ, iOfst+iAmt);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Truncate a tmp-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tmpTruncate(sqlite3_file *pFile, sqlite_int64 ಗಾತ್ರ_2){
  tmp_file *pTmp = (tmp_file *)pFile;
  pTmp->n_ಗಾತ್ರ = (ಪೂರ್ಣಾಂಕ)MIN(pTmp->n_ಗಾತ್ರ, ಗಾತ್ರ_2);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Sync a tmp-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tmpSync(sqlite3_file *pFile, ಪೂರ್ಣಾಂಕ flags){
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Return the current file-size of a tmp-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tmp_File_ಗಾತ್ರ(sqlite3_file *pFile, sqlite_int64 *p_ಗಾತ್ರ){
  tmp_file *pTmp = (tmp_file *)pFile;
  *p_ಗಾತ್ರ = pTmp->n_ಗಾತ್ರ;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Lock a tmp-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tmp_ಬೀಗ(sqlite3_file *pFile, ಪೂರ್ಣಾಂಕ e_ಬೀಗ){
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Unlock a tmp-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tmpUnlock(sqlite3_file *pFile, ಪೂರ್ಣಾಂಕ e_ಬೀಗ){
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on a tmp-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tmp_Check_Reserved_ಬೀಗ(sqlite3_file *pFile, ಪೂರ್ಣಾಂಕ *p_Res_ಹೊರಹೋಗು){
  *p_Res_ಹೊರಹೋಗು = 0;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** File control method. For custom operations on a tmp-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tmpFileControl(sqlite3_file *pFile, ಪೂರ್ಣಾಂಕ op, ಶೂನ್ಯ *pArg){
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Return the sector-size in bytes for a tmp-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tmp_Sector_ಗಾತ್ರ(sqlite3_file *pFile){
  ಹಿಂತಿರುಗಿ 0;
}

/*
** Return the device characteristic flags supported by a tmp-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tmpDeviceCharacteristics(sqlite3_file *pFile){
  ಹಿಂತಿರುಗಿ 0;
}

/*
** Close an fs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fsClose(sqlite3_file *pFile){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;

  /* Decrement the real_file ref-count. */
  pReal->nRef--;
  ಪ್ರತಿಜ್ಞಾ(pReal->nRef>=0);

  /* When the ref-count reaches 0, destroy the structure */
  ಆದರೆ( pReal->nRef==0 ){
    *pReal->ppThis = pReal->p_ಮುಂದಿನ;
    ಆದರೆ( pReal->p_ಮುಂದಿನ ){
      pReal->p_ಮುಂದಿನ->ppThis = pReal->ppThis;
    }
    rc = pReal->pFile->pMethods->xClose(pReal->pFile);
    sqlite3_free(pReal);
  }

  ಹಿಂತಿರುಗಿ rc;
}

/*
** Read data from an fs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fsRead(
  sqlite3_file *pFile, 
  ಶೂನ್ಯ *zBuf, 
  ಪೂರ್ಣಾಂಕ iAmt, 
  sqlite_int64 iOfst
){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  sqlite3_file *pF = pReal->pFile;

  ಆದರೆ( (p->eType==DATABASE_FILE && (iAmt+iOfst)>pReal->nDatabase)
   || (p->eType==JOURNAL_FILE && (iAmt+iOfst)>pReal->nJournal)
  ){
    rc = SQLITE_IOERR_SHORT_READ;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p->eType==DATABASE_FILE ){
    rc = pF->pMethods->xRead(pF, zBuf, iAmt, iOfst+BLOCKSIZE);
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* Journal file. */
    ಪೂರ್ಣಾಂಕ iRem = iAmt;
    ಪೂರ್ಣಾಂಕ iBuf = 0;
    ಪೂರ್ಣಾಂಕ ii = (ಪೂರ್ಣಾಂಕ)iOfst;
    ಆಗ( iRem>0 && rc==SQLITE_OK ){
      ಪೂರ್ಣಾಂಕ iRealOff = pReal->nBlob - BLOCKSIZE*((ii/BLOCKSIZE)+1) + ii%BLOCKSIZE;
      ಪೂರ್ಣಾಂಕ iRealAmt = MIN(iRem, BLOCKSIZE - (iRealOff%BLOCKSIZE));

      rc = pF->pMethods->xRead(pF, &((ಅಕ್ಷರ *)zBuf)[iBuf], iRealAmt, iRealOff);
      ii += iRealAmt;
      iBuf += iRealAmt;
      iRem -= iRealAmt;
    }
  }

  ಹಿಂತಿರುಗಿ rc;
}

/*
** Write data to an fs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fsWrite(
  sqlite3_file *pFile, 
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *zBuf, 
  ಪೂರ್ಣಾಂಕ iAmt, 
  sqlite_int64 iOfst
){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  sqlite3_file *pF = pReal->pFile;

  ಆದರೆ( p->eType==DATABASE_FILE ){
    ಆದರೆ( (iAmt+iOfst+BLOCKSIZE)>(pReal->nBlob-pReal->nJournal) ){
      rc = SQLITE_ತುಂಬಿದ;
    }ಇಲ್ಲದಿದ್ದರೆ{
      rc = pF->pMethods->xWrite(pF, zBuf, iAmt, iOfst+BLOCKSIZE);
      ಆದರೆ( rc==SQLITE_OK ){
        pReal->nDatabase = (ಪೂರ್ಣಾಂಕ)MAX(pReal->nDatabase, iAmt+iOfst);
      }
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* Journal file. */
    ಪೂರ್ಣಾಂಕ iRem = iAmt;
    ಪೂರ್ಣಾಂಕ iBuf = 0;
    ಪೂರ್ಣಾಂಕ ii = (ಪೂರ್ಣಾಂಕ)iOfst;
    ಆಗ( iRem>0 && rc==SQLITE_OK ){
      ಪೂರ್ಣಾಂಕ iRealOff = pReal->nBlob - BLOCKSIZE*((ii/BLOCKSIZE)+1) + ii%BLOCKSIZE;
      ಪೂರ್ಣಾಂಕ iRealAmt = MIN(iRem, BLOCKSIZE - (iRealOff%BLOCKSIZE));

      ಆದರೆ( iRealOff<(pReal->nDatabase+BLOCKSIZE) ){
        rc = SQLITE_ತುಂಬಿದ;
      }ಇಲ್ಲದಿದ್ದರೆ{
        rc = pF->pMethods->xWrite(pF, &((ಅಕ್ಷರ *)zBuf)[iBuf], iRealAmt,iRealOff);
        ii += iRealAmt;
        iBuf += iRealAmt;
        iRem -= iRealAmt;
      }
    }
    ಆದರೆ( rc==SQLITE_OK ){
      pReal->nJournal = (ಪೂರ್ಣಾಂಕ)MAX(pReal->nJournal, iAmt+iOfst);
    }
  }

  ಹಿಂತಿರುಗಿ rc;
}

/*
** Truncate an fs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fsTruncate(sqlite3_file *pFile, sqlite_int64 ಗಾತ್ರ_2){
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  ಆದರೆ( p->eType==DATABASE_FILE ){
    pReal->nDatabase = (ಪೂರ್ಣಾಂಕ)MIN(pReal->nDatabase, ಗಾತ್ರ_2);
  }ಇಲ್ಲದಿದ್ದರೆ{
    pReal->nJournal = (ಪೂರ್ಣಾಂಕ)MIN(pReal->nJournal, ಗಾತ್ರ_2);
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Sync an fs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fsSync(sqlite3_file *pFile, ಪೂರ್ಣಾಂಕ flags){
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  sqlite3_file *pRealFile = pReal->pFile;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;

  ಆದರೆ( p->eType==DATABASE_FILE ){
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ z_ಗಾತ್ರ[4];
    z_ಗಾತ್ರ[0] = (pReal->nDatabase&0xFF000000)>>24;
    z_ಗಾತ್ರ[1] = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)((pReal->nDatabase&0x00FF0000)>>16);
    z_ಗಾತ್ರ[2] = (pReal->nDatabase&0x0000FF00)>>8;
    z_ಗಾತ್ರ[3] = (pReal->nDatabase&0x000000FF);
    rc = pRealFile->pMethods->xWrite(pRealFile, z_ಗಾತ್ರ, 4, 0);
  }
  ಆದರೆ( rc==SQLITE_OK ){
    rc = pRealFile->pMethods->xSync(pRealFile, flags&(~SQLITE_SYNC_DATAONLY));
  }

  ಹಿಂತಿರುಗಿ rc;
}

/*
** Return the current file-size of an fs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fs_File_ಗಾತ್ರ(sqlite3_file *pFile, sqlite_int64 *p_ಗಾತ್ರ){
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  ಆದರೆ( p->eType==DATABASE_FILE ){
    *p_ಗಾತ್ರ = pReal->nDatabase;
  }ಇಲ್ಲದಿದ್ದರೆ{
    *p_ಗಾತ್ರ = pReal->nJournal;
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Lock an fs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fs_ಬೀಗ(sqlite3_file *pFile, ಪೂರ್ಣಾಂಕ e_ಬೀಗ){
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Unlock an fs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fsUnlock(sqlite3_file *pFile, ಪೂರ್ಣಾಂಕ e_ಬೀಗ){
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on an fs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fs_Check_Reserved_ಬೀಗ(sqlite3_file *pFile, ಪೂರ್ಣಾಂಕ *p_Res_ಹೊರಹೋಗು){
  *p_Res_ಹೊರಹೋಗು = 0;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** File control method. For custom operations on an fs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fsFileControl(sqlite3_file *pFile, ಪೂರ್ಣಾಂಕ op, ಶೂನ್ಯ *pArg){
  ಆದರೆ( op==SQLITE_FCNTL_PRAGMA ) ಹಿಂತಿರುಗಿ SQLITE_NOTFOUND;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Return the sector-size in bytes for an fs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fs_Sector_ಗಾತ್ರ(sqlite3_file *pFile){
  ಹಿಂತಿರುಗಿ BLOCKSIZE;
}

/*
** Return the device characteristic flags supported by an fs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fsDeviceCharacteristics(sqlite3_file *pFile){
  ಹಿಂತಿರುಗಿ 0;
}

/*
** Open an fs file handle.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fsOpen(
  sqlite3_vfs *pVfs,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName,
  sqlite3_file *pFile,
  ಪೂರ್ಣಾಂಕ flags,
  ಪೂರ್ಣಾಂಕ *p_ಹೊರಹೋಗು_Flags
){
  fs_vfs_t *pFsVfs = (fs_vfs_t *)pVfs;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = 0;
  ಪೂರ್ಣಾಂಕ eType;
  ಪೂರ್ಣಾಂಕ nName;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;

  ಆದರೆ( 0==(flags&(SQLITE_OPEN_MAIN_DB|SQLITE_OPEN_MAIN_JOURNAL)) ){
    tmp_file *p2 = (tmp_file *)pFile;
    memset(p2, 0, ಗಾತ್ರ(*p2));
    p2->ತಳ.pMethods = &tmp_io_methods;
    ಹಿಂತಿರುಗಿ SQLITE_OK;
  }

  eType = ((flags&(SQLITE_OPEN_MAIN_DB))?DATABASE_FILE:JOURNAL_FILE);
  p->ತಳ.pMethods = &fs_io_methods;
  p->eType = eType;

  ಪ್ರತಿಜ್ಞಾ(ಅಕ್ಷರಸರಣಿ_ಉದ್ದ("-journal")==8);
  nName = (ಪೂರ್ಣಾಂಕ)ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(zName)-((eType==JOURNAL_FILE)?8:0);
  pReal=pFsVfs->pFileList; 
  ಗಾಗಿ(; pReal && ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(pReal->zName, zName, nName); pReal=pReal->p_ಮುಂದಿನ);

  ಆದರೆ( !pReal ){
    ಪೂರ್ಣಾಂಕ real_flags = (flags&~(SQLITE_OPEN_MAIN_DB))|SQLITE_OPEN_ತಾತ್ಕಾಲಿಕ_DB;
    sqlite3_int64 ಗಾತ್ರ_2;
    sqlite3_file *pRealFile;
    sqlite3_vfs *p_ಪೋಷಕ = pFsVfs->p_ಪೋಷಕ;
    ಪ್ರತಿಜ್ಞಾ(eType==DATABASE_FILE);

    pReal = (fs_real_file *)sqlite3_malloc(ಗಾತ್ರ(*pReal)+p_ಪೋಷಕ->szOsFile);
    ಆದರೆ( !pReal ){
      rc = SQLITE_NOMEM;
      ಹೋಗು open_ಹೊರಹೋಗು;
    }
    memset(pReal, 0, ಗಾತ್ರ(*pReal)+p_ಪೋಷಕ->szOsFile);
    pReal->zName = zName;
    pReal->pFile = (sqlite3_file *)(&pReal[1]);

    rc = p_ಪೋಷಕ->xOpen(p_ಪೋಷಕ, zName, pReal->pFile, real_flags, p_ಹೊರಹೋಗು_Flags);
    ಆದರೆ( rc!=SQLITE_OK ){
      ಹೋಗು open_ಹೊರಹೋಗು;
    }
    pRealFile = pReal->pFile;

    rc = pRealFile->pMethods->x_File_ಗಾತ್ರ(pRealFile, &ಗಾತ್ರ_2);
    ಆದರೆ( rc!=SQLITE_OK ){
      ಹೋಗು open_ಹೊರಹೋಗು;
    }
    ಆದರೆ( ಗಾತ್ರ_2==0 ){
      rc = pRealFile->pMethods->xWrite(pRealFile, "\0", 1, BLOBSIZE-1);
      pReal->nBlob = BLOBSIZE;
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ zS[4];
      pReal->nBlob = (ಪೂರ್ಣಾಂಕ)ಗಾತ್ರ_2;
      rc = pRealFile->pMethods->xRead(pRealFile, zS, 4, 0);
      pReal->nDatabase = (zS[0]<<24)+(zS[1]<<16)+(zS[2]<<8)+zS[3];
      ಆದರೆ( rc==SQLITE_OK ){
        rc = pRealFile->pMethods->xRead(pRealFile, zS, 4, pReal->nBlob-4);
        ಆದರೆ( zS[0] || zS[1] || zS[2] || zS[3] ){
          pReal->nJournal = pReal->nBlob;
        }
      }
    }

    ಆದರೆ( rc==SQLITE_OK ){
      pReal->p_ಮುಂದಿನ = pFsVfs->pFileList;
      ಆದರೆ( pReal->p_ಮುಂದಿನ ){
        pReal->p_ಮುಂದಿನ->ppThis = &pReal->p_ಮುಂದಿನ;
      }
      pReal->ppThis = &pFsVfs->pFileList;
      pFsVfs->pFileList = pReal;
    }
  }

open_ಹೊರಹೋಗು:
  ಆದರೆ( pReal ){
    ಆದರೆ( rc==SQLITE_OK ){
      p->pReal = pReal;
      pReal->nRef++;
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಆದರೆ( pReal->pFile->pMethods ){
        pReal->pFile->pMethods->xClose(pReal->pFile);
      }
      sqlite3_free(pReal);
    }
  }
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fsDelete(sqlite3_vfs *pVfs, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮಾರ್ಗ, ಪೂರ್ಣಾಂಕ dirSync){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  fs_vfs_t *pFsVfs = (fs_vfs_t *)pVfs;
  fs_real_file *pReal;
  sqlite3_file *pF;
  ಪೂರ್ಣಾಂಕ nName = (ಪೂರ್ಣಾಂಕ)ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(z_ಮಾರ್ಗ) - 8;

  ಪ್ರತಿಜ್ಞಾ(ಅಕ್ಷರಸರಣಿ_ಉದ್ದ("-journal")==8);
  ಪ್ರತಿಜ್ಞಾ(strcmp("-journal", &z_ಮಾರ್ಗ[nName])==0);

  pReal = pFsVfs->pFileList; 
  ಗಾಗಿ(; pReal && ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(pReal->zName, z_ಮಾರ್ಗ, nName); pReal=pReal->p_ಮುಂದಿನ);
  ಆದರೆ( pReal ){
    pF = pReal->pFile;
    rc = pF->pMethods->xWrite(pF, "\0\0\0\0", 4, pReal->nBlob-BLOCKSIZE);
    ಆದರೆ( rc==SQLITE_OK ){
      pReal->nJournal = 0;
    }
  }
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fsAccess(
  sqlite3_vfs *pVfs, 
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮಾರ್ಗ, 
  ಪೂರ್ಣಾಂಕ flags, 
  ಪೂರ್ಣಾಂಕ *p_Res_ಹೊರಹೋಗು
){
  fs_vfs_t *pFsVfs = (fs_vfs_t *)pVfs;
  fs_real_file *pReal;
  ಪೂರ್ಣಾಂಕ isJournal = 0;
  ಪೂರ್ಣಾಂಕ nName = (ಪೂರ್ಣಾಂಕ)ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(z_ಮಾರ್ಗ);

  ಆದರೆ( flags!=SQLITE_ACCESS_EXISTS ){
    sqlite3_vfs *p_ಪೋಷಕ = ((fs_vfs_t *)pVfs)->p_ಪೋಷಕ;
    ಹಿಂತಿರುಗಿ p_ಪೋಷಕ->xAccess(p_ಪೋಷಕ, z_ಮಾರ್ಗ, flags, p_Res_ಹೊರಹೋಗು);
  }

  ಪ್ರತಿಜ್ಞಾ(ಅಕ್ಷರಸರಣಿ_ಉದ್ದ("-journal")==8);
  ಆದರೆ( nName>8 && strcmp("-journal", &z_ಮಾರ್ಗ[nName-8])==0 ){
    nName -= 8;
    isJournal = 1;
  }

  pReal = pFsVfs->pFileList; 
  ಗಾಗಿ(; pReal && ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(pReal->zName, z_ಮಾರ್ಗ, nName); pReal=pReal->p_ಮುಂದಿನ);

  *p_Res_ಹೊರಹೋಗು = (pReal && (!isJournal || pReal->nJournal>0));
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (FS_MAX_PATHNAME+1) bytes.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fs_ತುಂಬಿದ_Pathname(
  sqlite3_vfs *pVfs,            /* Pointer to vfs object */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮಾರ್ಗ,            /* Possibly relative input path */
  ಪೂರ್ಣಾಂಕ n_ಹೊರಹೋಗು,                     /* Size of output buffer in bytes */
  ಅಕ್ಷರ *z_ಹೊರಹೋಗು                    /* Output buffer */
){
  sqlite3_vfs *p_ಪೋಷಕ = ((fs_vfs_t *)pVfs)->p_ಪೋಷಕ;
  ಹಿಂತಿರುಗಿ p_ಪೋಷಕ->x_ತುಂಬಿದ_Pathname(p_ಪೋಷಕ, z_ಮಾರ್ಗ, n_ಹೊರಹೋಗು, z_ಹೊರಹೋಗು);
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
ಸ್ಥಿರ ಶೂನ್ಯ *fsDlOpen(sqlite3_vfs *pVfs, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮಾರ್ಗ){
  sqlite3_vfs *p_ಪೋಷಕ = ((fs_vfs_t *)pVfs)->p_ಪೋಷಕ;
  ಹಿಂತಿರುಗಿ p_ಪೋಷಕ->xDlOpen(p_ಪೋಷಕ, z_ಮಾರ್ಗ);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
ಸ್ಥಿರ ಶೂನ್ಯ fsDlError(sqlite3_vfs *pVfs, ಪೂರ್ಣಾಂಕ nByte, ಅಕ್ಷರ *z_Err_ಸಂದೇಶ){
  sqlite3_vfs *p_ಪೋಷಕ = ((fs_vfs_t *)pVfs)->p_ಪೋಷಕ;
  p_ಪೋಷಕ->xDlError(p_ಪೋಷಕ, nByte, z_Err_ಸಂದೇಶ);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
ಸ್ಥಿರ ಶೂನ್ಯ (*fsDlSym(sqlite3_vfs *pVfs, ಶೂನ್ಯ *pH, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSym))(ಶೂನ್ಯ){
  sqlite3_vfs *p_ಪೋಷಕ = ((fs_vfs_t *)pVfs)->p_ಪೋಷಕ;
  ಹಿಂತಿರುಗಿ p_ಪೋಷಕ->xDlSym(p_ಪೋಷಕ, pH, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
ಸ್ಥಿರ ಶೂನ್ಯ fsDlClose(sqlite3_vfs *pVfs, ಶೂನ್ಯ *pHandle){
  sqlite3_vfs *p_ಪೋಷಕ = ((fs_vfs_t *)pVfs)->p_ಪೋಷಕ;
  p_ಪೋಷಕ->xDlClose(p_ಪೋಷಕ, pHandle);
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fsRandomness(sqlite3_vfs *pVfs, ಪೂರ್ಣಾಂಕ nByte, ಅಕ್ಷರ *z_Buf_ಹೊರಹೋಗು){
  sqlite3_vfs *p_ಪೋಷಕ = ((fs_vfs_t *)pVfs)->p_ಪೋಷಕ;
  ಹಿಂತಿರುಗಿ p_ಪೋಷಕ->xRandomness(p_ಪೋಷಕ, nByte, z_Buf_ಹೊರಹೋಗು);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fsSleep(sqlite3_vfs *pVfs, ಪೂರ್ಣಾಂಕ nMicro){
  sqlite3_vfs *p_ಪೋಷಕ = ((fs_vfs_t *)pVfs)->p_ಪೋಷಕ;
  ಹಿಂತಿರುಗಿ p_ಪೋಷಕ->xSleep(p_ಪೋಷಕ, nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fs_Current_ಸಮಯ(sqlite3_vfs *pVfs, ದ್ವಿಗುಣ *p_ಸಮಯ_ಹೊರಹೋಗು){
  sqlite3_vfs *p_ಪೋಷಕ = ((fs_vfs_t *)pVfs)->p_ಪೋಷಕ;
  ಹಿಂತಿರುಗಿ p_ಪೋಷಕ->x_Current_ಸಮಯ(p_ಪೋಷಕ, p_ಸಮಯ_ಹೊರಹೋಗು);
}

/*
** This procedure registers the fs vfs with SQLite. If the argument is
** true, the fs vfs becomes the new default vfs. It is the only publicly
** available function in this file.
*/
ಪೂರ್ಣಾಂಕ fs_ರಿಜಿಸ್ಟರ್(ಶೂನ್ಯ){
  ಆದರೆ( fs_vfs.p_ಪೋಷಕ ) ಹಿಂತಿರುಗಿ SQLITE_OK;
  fs_vfs.p_ಪೋಷಕ = sqlite3_vfs_ಕಂಡುಹಿಡಿ(0);
  fs_vfs.ತಳ.mxPathname = fs_vfs.p_ಪೋಷಕ->mxPathname;
  fs_vfs.ತಳ.szOsFile = MAX(ಗಾತ್ರ(tmp_file), ಗಾತ್ರ(fs_file));
  ಹಿಂತಿರುಗಿ sqlite3_vfs_ರಿಜಿಸ್ಟರ್(&fs_vfs.ತಳ, 0);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_TEST
  ಪೂರ್ಣಾಂಕ SqlitetestOnefile_Init() {ಹಿಂತಿರುಗಿ fs_ರಿಜಿಸ್ಟರ್();}
#ಆದರೆ_ಅಂತ್ಯ
