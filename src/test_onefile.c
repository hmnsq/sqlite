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

#అన్తర్భావ "sqlite3.h"
#అన్తర్భావ <assert.h>
#అన్తర్భావ <string.h>

/*
** Maximum pathname length supported by the fs backend.
*/
#నిర్వచించు BLOCKSIZE 512
#నిర్వచించు BLOBSIZE 10485760

/*
** Name used to identify this VFS.
*/
#నిర్వచించు FS_VFS_NAME "fs"

రకనిర్వచనం నిర్మాణం fs_real_file fs_real_file;
నిర్మాణం fs_real_file {
  sqlite3_file *pFile;
  స్థిరరాశి అక్షరం *zName;
  పూర్ణాంకం nDatabase;              /* Current size of database region */
  పూర్ణాంకం nJournal;               /* Current size of journal region */
  పూర్ణాంకం nBlob;                  /* Total size of allocated blob */
  పూర్ణాంకం nRef;                   /* Number of pointers to this structure */
  fs_real_file *p_తదుపరి;
  fs_real_file **ppThis;
};

రకనిర్వచనం నిర్మాణం fs_file fs_file;
నిర్మాణం fs_file {
  sqlite3_file పునాది;
  పూర్ణాంకం eType;
  fs_real_file *pReal;
};

రకనిర్వచనం నిర్మాణం tmp_file tmp_file;
నిర్మాణం tmp_file {
  sqlite3_file పునాది;
  పూర్ణాంకం n_కొలత;
  పూర్ణాంకం nAlloc;
  అక్షరం *zAlloc;
};

/* Values for fs_file.eType. */
#నిర్వచించు DATABASE_FILE   1
#నిర్వచించు JOURNAL_FILE    2

/*
** Method declarations for fs_file.
*/
స్థిర పూర్ణాంకం fsClose(sqlite3_file*);
స్థిర పూర్ణాంకం fsRead(sqlite3_file*, శూన్య*, పూర్ణాంకం iAmt, sqlite3_int64 iOfst);
స్థిర పూర్ణాంకం fsWrite(sqlite3_file*, స్థిరరాశి శూన్య*, పూర్ణాంకం iAmt, sqlite3_int64 iOfst);
స్థిర పూర్ణాంకం fsTruncate(sqlite3_file*, sqlite3_int64 కొలత);
స్థిర పూర్ణాంకం fsSync(sqlite3_file*, పూర్ణాంకం flags);
స్థిర పూర్ణాంకం fs_File_కొలత(sqlite3_file*, sqlite3_int64 *p_కొలత);
స్థిర పూర్ణాంకం fs_తాళం(sqlite3_file*, పూర్ణాంకం);
స్థిర పూర్ణాంకం fsUnlock(sqlite3_file*, పూర్ణాంకం);
స్థిర పూర్ణాంకం fs_Check_Reserved_తాళం(sqlite3_file*, పూర్ణాంకం *p_Res_బహిర్గతం);
స్థిర పూర్ణాంకం fsFileControl(sqlite3_file*, పూర్ణాంకం op, శూన్య *pArg);
స్థిర పూర్ణాంకం fs_Sector_కొలత(sqlite3_file*);
స్థిర పూర్ణాంకం fsDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for tmp_file.
*/
స్థిర పూర్ణాంకం tmpClose(sqlite3_file*);
స్థిర పూర్ణాంకం tmpRead(sqlite3_file*, శూన్య*, పూర్ణాంకం iAmt, sqlite3_int64 iOfst);
స్థిర పూర్ణాంకం tmpWrite(sqlite3_file*, స్థిరరాశి శూన్య*, పూర్ణాంకం iAmt, sqlite3_int64 iOfst);
స్థిర పూర్ణాంకం tmpTruncate(sqlite3_file*, sqlite3_int64 కొలత);
స్థిర పూర్ణాంకం tmpSync(sqlite3_file*, పూర్ణాంకం flags);
స్థిర పూర్ణాంకం tmp_File_కొలత(sqlite3_file*, sqlite3_int64 *p_కొలత);
స్థిర పూర్ణాంకం tmp_తాళం(sqlite3_file*, పూర్ణాంకం);
స్థిర పూర్ణాంకం tmpUnlock(sqlite3_file*, పూర్ణాంకం);
స్థిర పూర్ణాంకం tmp_Check_Reserved_తాళం(sqlite3_file*, పూర్ణాంకం *p_Res_బహిర్గతం);
స్థిర పూర్ణాంకం tmpFileControl(sqlite3_file*, పూర్ణాంకం op, శూన్య *pArg);
స్థిర పూర్ణాంకం tmp_Sector_కొలత(sqlite3_file*);
స్థిర పూర్ణాంకం tmpDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for fs_vfs.
*/
స్థిర పూర్ణాంకం fsOpen(sqlite3_vfs*, స్థిరరాశి అక్షరం *, sqlite3_file*, పూర్ణాంకం , పూర్ణాంకం *);
స్థిర పూర్ణాంకం fsDelete(sqlite3_vfs*, స్థిరరాశి అక్షరం *zName, పూర్ణాంకం syncDir);
స్థిర పూర్ణాంకం fsAccess(sqlite3_vfs*, స్థిరరాశి అక్షరం *zName, పూర్ణాంకం flags, పూర్ణాంకం *);
స్థిర పూర్ణాంకం fs_నిండిన_Pathname(sqlite3_vfs*, స్థిరరాశి అక్షరం *zName, పూర్ణాంకం n_బహిర్గతం,అక్షరం *z_బహిర్గతం);
స్థిర శూన్య *fsDlOpen(sqlite3_vfs*, స్థిరరాశి అక్షరం *zFilename);
స్థిర శూన్య fsDlError(sqlite3_vfs*, పూర్ణాంకం nByte, అక్షరం *z_Err_సందేశం);
స్థిర శూన్య (*fsDlSym(sqlite3_vfs*,శూన్య*, స్థిరరాశి అక్షరం *z_చిహ్నం))(శూన్య);
స్థిర శూన్య fsDlClose(sqlite3_vfs*, శూన్య*);
స్థిర పూర్ణాంకం fsRandomness(sqlite3_vfs*, పూర్ణాంకం nByte, అక్షరం *z_బహిర్గతం);
స్థిర పూర్ణాంకం fsSleep(sqlite3_vfs*, పూర్ణాంకం microseconds);
స్థిర పూర్ణాంకం fs_Current_సమయం(sqlite3_vfs*, ద్విగుణ*);


రకనిర్వచనం నిర్మాణం fs_vfs_t fs_vfs_t;
నిర్మాణం fs_vfs_t {
  sqlite3_vfs పునాది;
  fs_real_file *pFileList;
  sqlite3_vfs *p_మాతృ;
};

స్థిర fs_vfs_t fs_vfs = {
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
    fs_నిండిన_Pathname,                             /* xFullPathname */
    fsDlOpen,                                   /* xDlOpen */
    fsDlError,                                  /* xDlError */
    fsDlSym,                                    /* xDlSym */
    fsDlClose,                                  /* xDlClose */
    fsRandomness,                               /* xRandomness */
    fsSleep,                                    /* xSleep */
    fs_Current_సమయం,                              /* xCurrentTime */
    0                                           /* xCurrentTimeInt64 */
  }, 
  0,                                            /* pFileList */
  0                                             /* pParent */
};

స్థిర sqlite3_io_methods fs_io_methods = {
  1,                            /* iVersion */
  fsClose,                      /* xClose */
  fsRead,                       /* xRead */
  fsWrite,                      /* xWrite */
  fsTruncate,                   /* xTruncate */
  fsSync,                       /* xSync */
  fs_File_కొలత,                   /* xFileSize */
  fs_తాళం,                       /* xLock */
  fsUnlock,                     /* xUnlock */
  fs_Check_Reserved_తాళం,          /* xCheckReservedLock */
  fsFileControl,                /* xFileControl */
  fs_Sector_కొలత,                 /* xSectorSize */
  fsDeviceCharacteristics,      /* xDeviceCharacteristics */
  0,                            /* xShmMap */
  0,                            /* xShmLock */
  0,                            /* xShmBarrier */
  0                             /* xShmUnmap */
};


స్థిర sqlite3_io_methods tmp_io_methods = {
  1,                            /* iVersion */
  tmpClose,                     /* xClose */
  tmpRead,                      /* xRead */
  tmpWrite,                     /* xWrite */
  tmpTruncate,                  /* xTruncate */
  tmpSync,                      /* xSync */
  tmp_File_కొలత,                  /* xFileSize */
  tmp_తాళం,                      /* xLock */
  tmpUnlock,                    /* xUnlock */
  tmp_Check_Reserved_తాళం,         /* xCheckReservedLock */
  tmpFileControl,               /* xFileControl */
  tmp_Sector_కొలత,                /* xSectorSize */
  tmpDeviceCharacteristics,     /* xDeviceCharacteristics */
  0,                            /* xShmMap */
  0,                            /* xShmLock */
  0,                            /* xShmBarrier */
  0                             /* xShmUnmap */
};

/* Useful macros used in several places */
#నిర్వచించు MIN(x,y) ((x)<(y)?(x):(y))
#నిర్వచించు MAX(x,y) ((x)>(y)?(x):(y))


/*
** Close a tmp-file.
*/
స్థిర పూర్ణాంకం tmpClose(sqlite3_file *pFile){
  tmp_file *pTmp = (tmp_file *)pFile;
  sqlite3_free(pTmp->zAlloc);
  తిరిగి SQLITE_OK;
}

/*
** Read data from a tmp-file.
*/
స్థిర పూర్ణాంకం tmpRead(
  sqlite3_file *pFile, 
  శూన్య *zBuf, 
  పూర్ణాంకం iAmt, 
  sqlite_int64 iOfst
){
  tmp_file *pTmp = (tmp_file *)pFile;
  ఒకవేళ( (iAmt+iOfst)>pTmp->n_కొలత ){
    తిరిగి SQLITE_IOERR_SHORT_READ;
  }
  జ్ఞాపకం_నకలు(zBuf, &pTmp->zAlloc[iOfst], iAmt);
  తిరిగి SQLITE_OK;
}

/*
** Write data to a tmp-file.
*/
స్థిర పూర్ణాంకం tmpWrite(
  sqlite3_file *pFile, 
  స్థిరరాశి శూన్య *zBuf, 
  పూర్ణాంకం iAmt, 
  sqlite_int64 iOfst
){
  tmp_file *pTmp = (tmp_file *)pFile;
  ఒకవేళ( (iAmt+iOfst)>pTmp->nAlloc ){
    పూర్ణాంకం nNew = (పూర్ణాంకం)(2*(iAmt+iOfst+pTmp->nAlloc));
    అక్షరం *zNew = sqlite3_realloc(pTmp->zAlloc, nNew);
    ఒకవేళ( !zNew ){
      తిరిగి SQLITE_NOMEM;
    }
    pTmp->zAlloc = zNew;
    pTmp->nAlloc = nNew;
  }
  జ్ఞాపకం_నకలు(&pTmp->zAlloc[iOfst], zBuf, iAmt);
  pTmp->n_కొలత = (పూర్ణాంకం)MAX(pTmp->n_కొలత, iOfst+iAmt);
  తిరిగి SQLITE_OK;
}

/*
** Truncate a tmp-file.
*/
స్థిర పూర్ణాంకం tmpTruncate(sqlite3_file *pFile, sqlite_int64 కొలత){
  tmp_file *pTmp = (tmp_file *)pFile;
  pTmp->n_కొలత = (పూర్ణాంకం)MIN(pTmp->n_కొలత, కొలత);
  తిరిగి SQLITE_OK;
}

/*
** Sync a tmp-file.
*/
స్థిర పూర్ణాంకం tmpSync(sqlite3_file *pFile, పూర్ణాంకం flags){
  తిరిగి SQLITE_OK;
}

/*
** Return the current file-size of a tmp-file.
*/
స్థిర పూర్ణాంకం tmp_File_కొలత(sqlite3_file *pFile, sqlite_int64 *p_కొలత){
  tmp_file *pTmp = (tmp_file *)pFile;
  *p_కొలత = pTmp->n_కొలత;
  తిరిగి SQLITE_OK;
}

/*
** Lock a tmp-file.
*/
స్థిర పూర్ణాంకం tmp_తాళం(sqlite3_file *pFile, పూర్ణాంకం e_తాళం){
  తిరిగి SQLITE_OK;
}

/*
** Unlock a tmp-file.
*/
స్థిర పూర్ణాంకం tmpUnlock(sqlite3_file *pFile, పూర్ణాంకం e_తాళం){
  తిరిగి SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on a tmp-file.
*/
స్థిర పూర్ణాంకం tmp_Check_Reserved_తాళం(sqlite3_file *pFile, పూర్ణాంకం *p_Res_బహిర్గతం){
  *p_Res_బహిర్గతం = 0;
  తిరిగి SQLITE_OK;
}

/*
** File control method. For custom operations on a tmp-file.
*/
స్థిర పూర్ణాంకం tmpFileControl(sqlite3_file *pFile, పూర్ణాంకం op, శూన్య *pArg){
  తిరిగి SQLITE_OK;
}

/*
** Return the sector-size in bytes for a tmp-file.
*/
స్థిర పూర్ణాంకం tmp_Sector_కొలత(sqlite3_file *pFile){
  తిరిగి 0;
}

/*
** Return the device characteristic flags supported by a tmp-file.
*/
స్థిర పూర్ణాంకం tmpDeviceCharacteristics(sqlite3_file *pFile){
  తిరిగి 0;
}

/*
** Close an fs-file.
*/
స్థిర పూర్ణాంకం fsClose(sqlite3_file *pFile){
  పూర్ణాంకం rc = SQLITE_OK;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;

  /* Decrement the real_file ref-count. */
  pReal->nRef--;
  ప్రతిజ్ఞా(pReal->nRef>=0);

  /* When the ref-count reaches 0, destroy the structure */
  ఒకవేళ( pReal->nRef==0 ){
    *pReal->ppThis = pReal->p_తదుపరి;
    ఒకవేళ( pReal->p_తదుపరి ){
      pReal->p_తదుపరి->ppThis = pReal->ppThis;
    }
    rc = pReal->pFile->pMethods->xClose(pReal->pFile);
    sqlite3_free(pReal);
  }

  తిరిగి rc;
}

/*
** Read data from an fs-file.
*/
స్థిర పూర్ణాంకం fsRead(
  sqlite3_file *pFile, 
  శూన్య *zBuf, 
  పూర్ణాంకం iAmt, 
  sqlite_int64 iOfst
){
  పూర్ణాంకం rc = SQLITE_OK;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  sqlite3_file *pF = pReal->pFile;

  ఒకవేళ( (p->eType==DATABASE_FILE && (iAmt+iOfst)>pReal->nDatabase)
   || (p->eType==JOURNAL_FILE && (iAmt+iOfst)>pReal->nJournal)
  ){
    rc = SQLITE_IOERR_SHORT_READ;
  }లేకపోతే ఒకవేళ( p->eType==DATABASE_FILE ){
    rc = pF->pMethods->xRead(pF, zBuf, iAmt, iOfst+BLOCKSIZE);
  }లేకపోతే{
    /* Journal file. */
    పూర్ణాంకం iRem = iAmt;
    పూర్ణాంకం iBuf = 0;
    పూర్ణాంకం ii = (పూర్ణాంకం)iOfst;
    అయితే( iRem>0 && rc==SQLITE_OK ){
      పూర్ణాంకం iRealOff = pReal->nBlob - BLOCKSIZE*((ii/BLOCKSIZE)+1) + ii%BLOCKSIZE;
      పూర్ణాంకం iRealAmt = MIN(iRem, BLOCKSIZE - (iRealOff%BLOCKSIZE));

      rc = pF->pMethods->xRead(pF, &((అక్షరం *)zBuf)[iBuf], iRealAmt, iRealOff);
      ii += iRealAmt;
      iBuf += iRealAmt;
      iRem -= iRealAmt;
    }
  }

  తిరిగి rc;
}

/*
** Write data to an fs-file.
*/
స్థిర పూర్ణాంకం fsWrite(
  sqlite3_file *pFile, 
  స్థిరరాశి శూన్య *zBuf, 
  పూర్ణాంకం iAmt, 
  sqlite_int64 iOfst
){
  పూర్ణాంకం rc = SQLITE_OK;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  sqlite3_file *pF = pReal->pFile;

  ఒకవేళ( p->eType==DATABASE_FILE ){
    ఒకవేళ( (iAmt+iOfst+BLOCKSIZE)>(pReal->nBlob-pReal->nJournal) ){
      rc = SQLITE_నిండిన;
    }లేకపోతే{
      rc = pF->pMethods->xWrite(pF, zBuf, iAmt, iOfst+BLOCKSIZE);
      ఒకవేళ( rc==SQLITE_OK ){
        pReal->nDatabase = (పూర్ణాంకం)MAX(pReal->nDatabase, iAmt+iOfst);
      }
    }
  }లేకపోతే{
    /* Journal file. */
    పూర్ణాంకం iRem = iAmt;
    పూర్ణాంకం iBuf = 0;
    పూర్ణాంకం ii = (పూర్ణాంకం)iOfst;
    అయితే( iRem>0 && rc==SQLITE_OK ){
      పూర్ణాంకం iRealOff = pReal->nBlob - BLOCKSIZE*((ii/BLOCKSIZE)+1) + ii%BLOCKSIZE;
      పూర్ణాంకం iRealAmt = MIN(iRem, BLOCKSIZE - (iRealOff%BLOCKSIZE));

      ఒకవేళ( iRealOff<(pReal->nDatabase+BLOCKSIZE) ){
        rc = SQLITE_నిండిన;
      }లేకపోతే{
        rc = pF->pMethods->xWrite(pF, &((అక్షరం *)zBuf)[iBuf], iRealAmt,iRealOff);
        ii += iRealAmt;
        iBuf += iRealAmt;
        iRem -= iRealAmt;
      }
    }
    ఒకవేళ( rc==SQLITE_OK ){
      pReal->nJournal = (పూర్ణాంకం)MAX(pReal->nJournal, iAmt+iOfst);
    }
  }

  తిరిగి rc;
}

/*
** Truncate an fs-file.
*/
స్థిర పూర్ణాంకం fsTruncate(sqlite3_file *pFile, sqlite_int64 కొలత){
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  ఒకవేళ( p->eType==DATABASE_FILE ){
    pReal->nDatabase = (పూర్ణాంకం)MIN(pReal->nDatabase, కొలత);
  }లేకపోతే{
    pReal->nJournal = (పూర్ణాంకం)MIN(pReal->nJournal, కొలత);
  }
  తిరిగి SQLITE_OK;
}

/*
** Sync an fs-file.
*/
స్థిర పూర్ణాంకం fsSync(sqlite3_file *pFile, పూర్ణాంకం flags){
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  sqlite3_file *pRealFile = pReal->pFile;
  పూర్ణాంకం rc = SQLITE_OK;

  ఒకవేళ( p->eType==DATABASE_FILE ){
    గుర్తులేని అక్షరం z_కొలత[4];
    z_కొలత[0] = (pReal->nDatabase&0xFF000000)>>24;
    z_కొలత[1] = (గుర్తులేని అక్షరం)((pReal->nDatabase&0x00FF0000)>>16);
    z_కొలత[2] = (pReal->nDatabase&0x0000FF00)>>8;
    z_కొలత[3] = (pReal->nDatabase&0x000000FF);
    rc = pRealFile->pMethods->xWrite(pRealFile, z_కొలత, 4, 0);
  }
  ఒకవేళ( rc==SQLITE_OK ){
    rc = pRealFile->pMethods->xSync(pRealFile, flags&(~SQLITE_SYNC_DATAONLY));
  }

  తిరిగి rc;
}

/*
** Return the current file-size of an fs-file.
*/
స్థిర పూర్ణాంకం fs_File_కొలత(sqlite3_file *pFile, sqlite_int64 *p_కొలత){
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  ఒకవేళ( p->eType==DATABASE_FILE ){
    *p_కొలత = pReal->nDatabase;
  }లేకపోతే{
    *p_కొలత = pReal->nJournal;
  }
  తిరిగి SQLITE_OK;
}

/*
** Lock an fs-file.
*/
స్థిర పూర్ణాంకం fs_తాళం(sqlite3_file *pFile, పూర్ణాంకం e_తాళం){
  తిరిగి SQLITE_OK;
}

/*
** Unlock an fs-file.
*/
స్థిర పూర్ణాంకం fsUnlock(sqlite3_file *pFile, పూర్ణాంకం e_తాళం){
  తిరిగి SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on an fs-file.
*/
స్థిర పూర్ణాంకం fs_Check_Reserved_తాళం(sqlite3_file *pFile, పూర్ణాంకం *p_Res_బహిర్గతం){
  *p_Res_బహిర్గతం = 0;
  తిరిగి SQLITE_OK;
}

/*
** File control method. For custom operations on an fs-file.
*/
స్థిర పూర్ణాంకం fsFileControl(sqlite3_file *pFile, పూర్ణాంకం op, శూన్య *pArg){
  ఒకవేళ( op==SQLITE_FCNTL_PRAGMA ) తిరిగి SQLITE_NOTFOUND;
  తిరిగి SQLITE_OK;
}

/*
** Return the sector-size in bytes for an fs-file.
*/
స్థిర పూర్ణాంకం fs_Sector_కొలత(sqlite3_file *pFile){
  తిరిగి BLOCKSIZE;
}

/*
** Return the device characteristic flags supported by an fs-file.
*/
స్థిర పూర్ణాంకం fsDeviceCharacteristics(sqlite3_file *pFile){
  తిరిగి 0;
}

/*
** Open an fs file handle.
*/
స్థిర పూర్ణాంకం fsOpen(
  sqlite3_vfs *pVfs,
  స్థిరరాశి అక్షరం *zName,
  sqlite3_file *pFile,
  పూర్ణాంకం flags,
  పూర్ణాంకం *p_బహిర్గతం_Flags
){
  fs_vfs_t *pFsVfs = (fs_vfs_t *)pVfs;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = 0;
  పూర్ణాంకం eType;
  పూర్ణాంకం nName;
  పూర్ణాంకం rc = SQLITE_OK;

  ఒకవేళ( 0==(flags&(SQLITE_OPEN_MAIN_DB|SQLITE_OPEN_MAIN_JOURNAL)) ){
    tmp_file *p2 = (tmp_file *)pFile;
    memset(p2, 0, పరిమాణం(*p2));
    p2->పునాది.pMethods = &tmp_io_methods;
    తిరిగి SQLITE_OK;
  }

  eType = ((flags&(SQLITE_OPEN_MAIN_DB))?DATABASE_FILE:JOURNAL_FILE);
  p->పునాది.pMethods = &fs_io_methods;
  p->eType = eType;

  ప్రతిజ్ఞా(అక్షరశ్రేణి_పొడవు("-journal")==8);
  nName = (పూర్ణాంకం)అక్షరశ్రేణి_పొడవు(zName)-((eType==JOURNAL_FILE)?8:0);
  pReal=pFsVfs->pFileList; 
  కొరకు(; pReal && అక్షరశ్రేణి_పోలిక(pReal->zName, zName, nName); pReal=pReal->p_తదుపరి);

  ఒకవేళ( !pReal ){
    పూర్ణాంకం real_flags = (flags&~(SQLITE_OPEN_MAIN_DB))|SQLITE_OPEN_తాత్కాలిక_DB;
    sqlite3_int64 కొలత;
    sqlite3_file *pRealFile;
    sqlite3_vfs *p_మాతృ = pFsVfs->p_మాతృ;
    ప్రతిజ్ఞా(eType==DATABASE_FILE);

    pReal = (fs_real_file *)sqlite3_malloc(పరిమాణం(*pReal)+p_మాతృ->szOsFile);
    ఒకవేళ( !pReal ){
      rc = SQLITE_NOMEM;
      వెళ్ళు open_బహిర్గతం;
    }
    memset(pReal, 0, పరిమాణం(*pReal)+p_మాతృ->szOsFile);
    pReal->zName = zName;
    pReal->pFile = (sqlite3_file *)(&pReal[1]);

    rc = p_మాతృ->xOpen(p_మాతృ, zName, pReal->pFile, real_flags, p_బహిర్గతం_Flags);
    ఒకవేళ( rc!=SQLITE_OK ){
      వెళ్ళు open_బహిర్గతం;
    }
    pRealFile = pReal->pFile;

    rc = pRealFile->pMethods->x_File_కొలత(pRealFile, &కొలత);
    ఒకవేళ( rc!=SQLITE_OK ){
      వెళ్ళు open_బహిర్గతం;
    }
    ఒకవేళ( కొలత==0 ){
      rc = pRealFile->pMethods->xWrite(pRealFile, "\0", 1, BLOBSIZE-1);
      pReal->nBlob = BLOBSIZE;
    }లేకపోతే{
      గుర్తులేని అక్షరం zS[4];
      pReal->nBlob = (పూర్ణాంకం)కొలత;
      rc = pRealFile->pMethods->xRead(pRealFile, zS, 4, 0);
      pReal->nDatabase = (zS[0]<<24)+(zS[1]<<16)+(zS[2]<<8)+zS[3];
      ఒకవేళ( rc==SQLITE_OK ){
        rc = pRealFile->pMethods->xRead(pRealFile, zS, 4, pReal->nBlob-4);
        ఒకవేళ( zS[0] || zS[1] || zS[2] || zS[3] ){
          pReal->nJournal = pReal->nBlob;
        }
      }
    }

    ఒకవేళ( rc==SQLITE_OK ){
      pReal->p_తదుపరి = pFsVfs->pFileList;
      ఒకవేళ( pReal->p_తదుపరి ){
        pReal->p_తదుపరి->ppThis = &pReal->p_తదుపరి;
      }
      pReal->ppThis = &pFsVfs->pFileList;
      pFsVfs->pFileList = pReal;
    }
  }

open_బహిర్గతం:
  ఒకవేళ( pReal ){
    ఒకవేళ( rc==SQLITE_OK ){
      p->pReal = pReal;
      pReal->nRef++;
    }లేకపోతే{
      ఒకవేళ( pReal->pFile->pMethods ){
        pReal->pFile->pMethods->xClose(pReal->pFile);
      }
      sqlite3_free(pReal);
    }
  }
  తిరిగి rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
స్థిర పూర్ణాంకం fsDelete(sqlite3_vfs *pVfs, స్థిరరాశి అక్షరం *z_మార్గం, పూర్ణాంకం dirSync){
  పూర్ణాంకం rc = SQLITE_OK;
  fs_vfs_t *pFsVfs = (fs_vfs_t *)pVfs;
  fs_real_file *pReal;
  sqlite3_file *pF;
  పూర్ణాంకం nName = (పూర్ణాంకం)అక్షరశ్రేణి_పొడవు(z_మార్గం) - 8;

  ప్రతిజ్ఞా(అక్షరశ్రేణి_పొడవు("-journal")==8);
  ప్రతిజ్ఞా(strcmp("-journal", &z_మార్గం[nName])==0);

  pReal = pFsVfs->pFileList; 
  కొరకు(; pReal && అక్షరశ్రేణి_పోలిక(pReal->zName, z_మార్గం, nName); pReal=pReal->p_తదుపరి);
  ఒకవేళ( pReal ){
    pF = pReal->pFile;
    rc = pF->pMethods->xWrite(pF, "\0\0\0\0", 4, pReal->nBlob-BLOCKSIZE);
    ఒకవేళ( rc==SQLITE_OK ){
      pReal->nJournal = 0;
    }
  }
  తిరిగి rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
స్థిర పూర్ణాంకం fsAccess(
  sqlite3_vfs *pVfs, 
  స్థిరరాశి అక్షరం *z_మార్గం, 
  పూర్ణాంకం flags, 
  పూర్ణాంకం *p_Res_బహిర్గతం
){
  fs_vfs_t *pFsVfs = (fs_vfs_t *)pVfs;
  fs_real_file *pReal;
  పూర్ణాంకం isJournal = 0;
  పూర్ణాంకం nName = (పూర్ణాంకం)అక్షరశ్రేణి_పొడవు(z_మార్గం);

  ఒకవేళ( flags!=SQLITE_ACCESS_EXISTS ){
    sqlite3_vfs *p_మాతృ = ((fs_vfs_t *)pVfs)->p_మాతృ;
    తిరిగి p_మాతృ->xAccess(p_మాతృ, z_మార్గం, flags, p_Res_బహిర్గతం);
  }

  ప్రతిజ్ఞా(అక్షరశ్రేణి_పొడవు("-journal")==8);
  ఒకవేళ( nName>8 && strcmp("-journal", &z_మార్గం[nName-8])==0 ){
    nName -= 8;
    isJournal = 1;
  }

  pReal = pFsVfs->pFileList; 
  కొరకు(; pReal && అక్షరశ్రేణి_పోలిక(pReal->zName, z_మార్గం, nName); pReal=pReal->p_తదుపరి);

  *p_Res_బహిర్గతం = (pReal && (!isJournal || pReal->nJournal>0));
  తిరిగి SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (FS_MAX_PATHNAME+1) bytes.
*/
స్థిర పూర్ణాంకం fs_నిండిన_Pathname(
  sqlite3_vfs *pVfs,            /* Pointer to vfs object */
  స్థిరరాశి అక్షరం *z_మార్గం,            /* Possibly relative input path */
  పూర్ణాంకం n_బహిర్గతం,                     /* Size of output buffer in bytes */
  అక్షరం *z_బహిర్గతం                    /* Output buffer */
){
  sqlite3_vfs *p_మాతృ = ((fs_vfs_t *)pVfs)->p_మాతృ;
  తిరిగి p_మాతృ->x_నిండిన_Pathname(p_మాతృ, z_మార్గం, n_బహిర్గతం, z_బహిర్గతం);
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
స్థిర శూన్య *fsDlOpen(sqlite3_vfs *pVfs, స్థిరరాశి అక్షరం *z_మార్గం){
  sqlite3_vfs *p_మాతృ = ((fs_vfs_t *)pVfs)->p_మాతృ;
  తిరిగి p_మాతృ->xDlOpen(p_మాతృ, z_మార్గం);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
స్థిర శూన్య fsDlError(sqlite3_vfs *pVfs, పూర్ణాంకం nByte, అక్షరం *z_Err_సందేశం){
  sqlite3_vfs *p_మాతృ = ((fs_vfs_t *)pVfs)->p_మాతృ;
  p_మాతృ->xDlError(p_మాతృ, nByte, z_Err_సందేశం);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
స్థిర శూన్య (*fsDlSym(sqlite3_vfs *pVfs, శూన్య *pH, స్థిరరాశి అక్షరం *zSym))(శూన్య){
  sqlite3_vfs *p_మాతృ = ((fs_vfs_t *)pVfs)->p_మాతృ;
  తిరిగి p_మాతృ->xDlSym(p_మాతృ, pH, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
స్థిర శూన్య fsDlClose(sqlite3_vfs *pVfs, శూన్య *pHandle){
  sqlite3_vfs *p_మాతృ = ((fs_vfs_t *)pVfs)->p_మాతృ;
  p_మాతృ->xDlClose(p_మాతృ, pHandle);
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
స్థిర పూర్ణాంకం fsRandomness(sqlite3_vfs *pVfs, పూర్ణాంకం nByte, అక్షరం *z_Buf_బహిర్గతం){
  sqlite3_vfs *p_మాతృ = ((fs_vfs_t *)pVfs)->p_మాతృ;
  తిరిగి p_మాతృ->xRandomness(p_మాతృ, nByte, z_Buf_బహిర్గతం);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
స్థిర పూర్ణాంకం fsSleep(sqlite3_vfs *pVfs, పూర్ణాంకం nMicro){
  sqlite3_vfs *p_మాతృ = ((fs_vfs_t *)pVfs)->p_మాతృ;
  తిరిగి p_మాతృ->xSleep(p_మాతృ, nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
స్థిర పూర్ణాంకం fs_Current_సమయం(sqlite3_vfs *pVfs, ద్విగుణ *p_సమయం_బహిర్గతం){
  sqlite3_vfs *p_మాతృ = ((fs_vfs_t *)pVfs)->p_మాతృ;
  తిరిగి p_మాతృ->x_Current_సమయం(p_మాతృ, p_సమయం_బహిర్గతం);
}

/*
** This procedure registers the fs vfs with SQLite. If the argument is
** true, the fs vfs becomes the new default vfs. It is the only publicly
** available function in this file.
*/
పూర్ణాంకం fs_రిజిస్టరు(శూన్య){
  ఒకవేళ( fs_vfs.p_మాతృ ) తిరిగి SQLITE_OK;
  fs_vfs.p_మాతృ = sqlite3_vfs_కనుగొను(0);
  fs_vfs.పునాది.mxPathname = fs_vfs.p_మాతృ->mxPathname;
  fs_vfs.పునాది.szOsFile = MAX(పరిమాణం(tmp_file), పరిమాణం(fs_file));
  తిరిగి sqlite3_vfs_రిజిస్టరు(&fs_vfs.పునాది, 0);
}

#నిర్వచితమైతే SQLITE_TEST
  పూర్ణాంకం SqlitetestOnefile_Init() {తిరిగి fs_రిజిస్టరు();}
#అయితే_ముగింపు
