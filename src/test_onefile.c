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
#تعریف_کړه BLOCKSIZE 512
#تعریف_کړه BLOBSIZE 10485760

/*
** Name used to identify this VFS.
*/
#تعریف_کړه FS_VFS_NAME "fs"

تعريفنوع بنية fs_real_file fs_real_file;
بنية fs_real_file {
  sqlite3_file *pFile;
  ثابت حرف *zName;
  صحيح nDatabase;              /* Current size of database region */
  صحيح nJournal;               /* Current size of journal region */
  صحيح nBlob;                  /* Total size of allocated blob */
  صحيح nRef;                   /* Number of pointers to this structure */
  fs_real_file *p_بل;
  fs_real_file **ppThis;
};

تعريفنوع بنية fs_file fs_file;
بنية fs_file {
  sqlite3_file بنسټ;
  صحيح eType;
  fs_real_file *pReal;
};

تعريفنوع بنية tmp_file tmp_file;
بنية tmp_file {
  sqlite3_file بنسټ;
  صحيح n_اندازه;
  صحيح nAlloc;
  حرف *zAlloc;
};

/* Values for fs_file.eType. */
#تعریف_کړه DATABASE_FILE   1
#تعریف_کړه JOURNAL_FILE    2

/*
** Method declarations for fs_file.
*/
ساكن صحيح fsClose(sqlite3_file*);
ساكن صحيح fsRead(sqlite3_file*, فراغ*, صحيح iAmt, sqlite3_int64 iOfst);
ساكن صحيح fsWrite(sqlite3_file*, ثابت فراغ*, صحيح iAmt, sqlite3_int64 iOfst);
ساكن صحيح fsTruncate(sqlite3_file*, sqlite3_int64 اندازه);
ساكن صحيح fsSync(sqlite3_file*, صحيح flags);
ساكن صحيح fs_File_اندازه(sqlite3_file*, sqlite3_int64 *p_اندازه);
ساكن صحيح fs_قلف(sqlite3_file*, صحيح);
ساكن صحيح fsUnlock(sqlite3_file*, صحيح);
ساكن صحيح fs_Check_Reserved_قلف(sqlite3_file*, صحيح *p_Res_بهر);
ساكن صحيح fsFileControl(sqlite3_file*, صحيح op, فراغ *pArg);
ساكن صحيح fs_Sector_اندازه(sqlite3_file*);
ساكن صحيح fsDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for tmp_file.
*/
ساكن صحيح tmpClose(sqlite3_file*);
ساكن صحيح tmpRead(sqlite3_file*, فراغ*, صحيح iAmt, sqlite3_int64 iOfst);
ساكن صحيح tmpWrite(sqlite3_file*, ثابت فراغ*, صحيح iAmt, sqlite3_int64 iOfst);
ساكن صحيح tmpTruncate(sqlite3_file*, sqlite3_int64 اندازه);
ساكن صحيح tmpSync(sqlite3_file*, صحيح flags);
ساكن صحيح tmp_File_اندازه(sqlite3_file*, sqlite3_int64 *p_اندازه);
ساكن صحيح tmp_قلف(sqlite3_file*, صحيح);
ساكن صحيح tmpUnlock(sqlite3_file*, صحيح);
ساكن صحيح tmp_Check_Reserved_قلف(sqlite3_file*, صحيح *p_Res_بهر);
ساكن صحيح tmpFileControl(sqlite3_file*, صحيح op, فراغ *pArg);
ساكن صحيح tmp_Sector_اندازه(sqlite3_file*);
ساكن صحيح tmpDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for fs_vfs.
*/
ساكن صحيح fsOpen(sqlite3_vfs*, ثابت حرف *, sqlite3_file*, صحيح , صحيح *);
ساكن صحيح fsDelete(sqlite3_vfs*, ثابت حرف *zName, صحيح syncDir);
ساكن صحيح fsAccess(sqlite3_vfs*, ثابت حرف *zName, صحيح flags, صحيح *);
ساكن صحيح fs_ډک_Pathname(sqlite3_vfs*, ثابت حرف *zName, صحيح n_بهر,حرف *z_بهر);
ساكن فراغ *fsDlOpen(sqlite3_vfs*, ثابت حرف *zFilename);
ساكن فراغ fsDlError(sqlite3_vfs*, صحيح nByte, حرف *z_Err_پیغام);
ساكن فراغ (*fsDlSym(sqlite3_vfs*,فراغ*, ثابت حرف *z_نښه))(فراغ);
ساكن فراغ fsDlClose(sqlite3_vfs*, فراغ*);
ساكن صحيح fsRandomness(sqlite3_vfs*, صحيح nByte, حرف *z_بهر);
ساكن صحيح fsSleep(sqlite3_vfs*, صحيح microseconds);
ساكن صحيح fs_Current_وخت(sqlite3_vfs*, مزدوج*);


تعريفنوع بنية fs_vfs_t fs_vfs_t;
بنية fs_vfs_t {
  sqlite3_vfs بنسټ;
  fs_real_file *pFileList;
  sqlite3_vfs *p_پلار;
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
    fs_ډک_Pathname,                             /* xFullPathname */
    fsDlOpen,                                   /* xDlOpen */
    fsDlError,                                  /* xDlError */
    fsDlSym,                                    /* xDlSym */
    fsDlClose,                                  /* xDlClose */
    fsRandomness,                               /* xRandomness */
    fsSleep,                                    /* xSleep */
    fs_Current_وخت,                              /* xCurrentTime */
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
  fs_File_اندازه,                   /* xFileSize */
  fs_قلف,                       /* xLock */
  fsUnlock,                     /* xUnlock */
  fs_Check_Reserved_قلف,          /* xCheckReservedLock */
  fsFileControl,                /* xFileControl */
  fs_Sector_اندازه,                 /* xSectorSize */
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
  tmp_File_اندازه,                  /* xFileSize */
  tmp_قلف,                      /* xLock */
  tmpUnlock,                    /* xUnlock */
  tmp_Check_Reserved_قلف,         /* xCheckReservedLock */
  tmpFileControl,               /* xFileControl */
  tmp_Sector_اندازه,                /* xSectorSize */
  tmpDeviceCharacteristics,     /* xDeviceCharacteristics */
  0,                            /* xShmMap */
  0,                            /* xShmLock */
  0,                            /* xShmBarrier */
  0                             /* xShmUnmap */
};

/* Useful macros used in several places */
#تعریف_کړه MIN(x,y) ((x)<(y)?(x):(y))
#تعریف_کړه MAX(x,y) ((x)>(y)?(x):(y))


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
  إذا( (iAmt+iOfst)>pTmp->n_اندازه ){
    أرجع SQLITE_IOERR_SHORT_READ;
  }
  حافظه_نقل(zBuf, &pTmp->zAlloc[iOfst], iAmt);
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
  حافظه_نقل(&pTmp->zAlloc[iOfst], zBuf, iAmt);
  pTmp->n_اندازه = (صحيح)MAX(pTmp->n_اندازه, iOfst+iAmt);
  أرجع SQLITE_OK;
}

/*
** Truncate a tmp-file.
*/
ساكن صحيح tmpTruncate(sqlite3_file *pFile, sqlite_int64 اندازه){
  tmp_file *pTmp = (tmp_file *)pFile;
  pTmp->n_اندازه = (صحيح)MIN(pTmp->n_اندازه, اندازه);
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
ساكن صحيح tmp_File_اندازه(sqlite3_file *pFile, sqlite_int64 *p_اندازه){
  tmp_file *pTmp = (tmp_file *)pFile;
  *p_اندازه = pTmp->n_اندازه;
  أرجع SQLITE_OK;
}

/*
** Lock a tmp-file.
*/
ساكن صحيح tmp_قلف(sqlite3_file *pFile, صحيح e_قلف){
  أرجع SQLITE_OK;
}

/*
** Unlock a tmp-file.
*/
ساكن صحيح tmpUnlock(sqlite3_file *pFile, صحيح e_قلف){
  أرجع SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on a tmp-file.
*/
ساكن صحيح tmp_Check_Reserved_قلف(sqlite3_file *pFile, صحيح *p_Res_بهر){
  *p_Res_بهر = 0;
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
ساكن صحيح tmp_Sector_اندازه(sqlite3_file *pFile){
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
    *pReal->ppThis = pReal->p_بل;
    إذا( pReal->p_بل ){
      pReal->p_بل->ppThis = pReal->ppThis;
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
      rc = SQLITE_ډک;
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
        rc = SQLITE_ډک;
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
ساكن صحيح fsTruncate(sqlite3_file *pFile, sqlite_int64 اندازه){
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  إذا( p->eType==DATABASE_FILE ){
    pReal->nDatabase = (صحيح)MIN(pReal->nDatabase, اندازه);
  }وإلا{
    pReal->nJournal = (صحيح)MIN(pReal->nJournal, اندازه);
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
    بینښه حرف z_اندازه[4];
    z_اندازه[0] = (pReal->nDatabase&0xFF000000)>>24;
    z_اندازه[1] = (بینښه حرف)((pReal->nDatabase&0x00FF0000)>>16);
    z_اندازه[2] = (pReal->nDatabase&0x0000FF00)>>8;
    z_اندازه[3] = (pReal->nDatabase&0x000000FF);
    rc = pRealFile->pMethods->xWrite(pRealFile, z_اندازه, 4, 0);
  }
  إذا( rc==SQLITE_OK ){
    rc = pRealFile->pMethods->xSync(pRealFile, flags&(~SQLITE_SYNC_DATAONLY));
  }

  أرجع rc;
}

/*
** Return the current file-size of an fs-file.
*/
ساكن صحيح fs_File_اندازه(sqlite3_file *pFile, sqlite_int64 *p_اندازه){
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  إذا( p->eType==DATABASE_FILE ){
    *p_اندازه = pReal->nDatabase;
  }وإلا{
    *p_اندازه = pReal->nJournal;
  }
  أرجع SQLITE_OK;
}

/*
** Lock an fs-file.
*/
ساكن صحيح fs_قلف(sqlite3_file *pFile, صحيح e_قلف){
  أرجع SQLITE_OK;
}

/*
** Unlock an fs-file.
*/
ساكن صحيح fsUnlock(sqlite3_file *pFile, صحيح e_قلف){
  أرجع SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on an fs-file.
*/
ساكن صحيح fs_Check_Reserved_قلف(sqlite3_file *pFile, صحيح *p_Res_بهر){
  *p_Res_بهر = 0;
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
ساكن صحيح fs_Sector_اندازه(sqlite3_file *pFile){
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
  صحيح *p_بهر_Flags
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
    p2->بنسټ.pMethods = &tmp_io_methods;
    أرجع SQLITE_OK;
  }

  eType = ((flags&(SQLITE_OPEN_MAIN_DB))?DATABASE_FILE:JOURNAL_FILE);
  p->بنسټ.pMethods = &fs_io_methods;
  p->eType = eType;

  أكد(مزي_اوږدوالی("-journal")==8);
  nName = (صحيح)مزي_اوږدوالی(zName)-((eType==JOURNAL_FILE)?8:0);
  pReal=pFsVfs->pFileList; 
  لكل(; pReal && مزي_پرتله(pReal->zName, zName, nName); pReal=pReal->p_بل);

  إذا( !pReal ){
    صحيح real_flags = (flags&~(SQLITE_OPEN_MAIN_DB))|SQLITE_OPEN_لنډمهاله_DB;
    sqlite3_int64 اندازه;
    sqlite3_file *pRealFile;
    sqlite3_vfs *p_پلار = pFsVfs->p_پلار;
    أكد(eType==DATABASE_FILE);

    pReal = (fs_real_file *)sqlite3_malloc(حجم(*pReal)+p_پلار->szOsFile);
    إذا( !pReal ){
      rc = SQLITE_NOMEM;
      اذهب open_بهر;
    }
    memset(pReal, 0, حجم(*pReal)+p_پلار->szOsFile);
    pReal->zName = zName;
    pReal->pFile = (sqlite3_file *)(&pReal[1]);

    rc = p_پلار->xOpen(p_پلار, zName, pReal->pFile, real_flags, p_بهر_Flags);
    إذا( rc!=SQLITE_OK ){
      اذهب open_بهر;
    }
    pRealFile = pReal->pFile;

    rc = pRealFile->pMethods->x_File_اندازه(pRealFile, &اندازه);
    إذا( rc!=SQLITE_OK ){
      اذهب open_بهر;
    }
    إذا( اندازه==0 ){
      rc = pRealFile->pMethods->xWrite(pRealFile, "\0", 1, BLOBSIZE-1);
      pReal->nBlob = BLOBSIZE;
    }وإلا{
      بینښه حرف zS[4];
      pReal->nBlob = (صحيح)اندازه;
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
      pReal->p_بل = pFsVfs->pFileList;
      إذا( pReal->p_بل ){
        pReal->p_بل->ppThis = &pReal->p_بل;
      }
      pReal->ppThis = &pFsVfs->pFileList;
      pFsVfs->pFileList = pReal;
    }
  }

open_بهر:
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
ساكن صحيح fsDelete(sqlite3_vfs *pVfs, ثابت حرف *z_لار, صحيح dirSync){
  صحيح rc = SQLITE_OK;
  fs_vfs_t *pFsVfs = (fs_vfs_t *)pVfs;
  fs_real_file *pReal;
  sqlite3_file *pF;
  صحيح nName = (صحيح)مزي_اوږدوالی(z_لار) - 8;

  أكد(مزي_اوږدوالی("-journal")==8);
  أكد(strcmp("-journal", &z_لار[nName])==0);

  pReal = pFsVfs->pFileList; 
  لكل(; pReal && مزي_پرتله(pReal->zName, z_لار, nName); pReal=pReal->p_بل);
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
  ثابت حرف *z_لار, 
  صحيح flags, 
  صحيح *p_Res_بهر
){
  fs_vfs_t *pFsVfs = (fs_vfs_t *)pVfs;
  fs_real_file *pReal;
  صحيح isJournal = 0;
  صحيح nName = (صحيح)مزي_اوږدوالی(z_لار);

  إذا( flags!=SQLITE_ACCESS_EXISTS ){
    sqlite3_vfs *p_پلار = ((fs_vfs_t *)pVfs)->p_پلار;
    أرجع p_پلار->xAccess(p_پلار, z_لار, flags, p_Res_بهر);
  }

  أكد(مزي_اوږدوالی("-journal")==8);
  إذا( nName>8 && strcmp("-journal", &z_لار[nName-8])==0 ){
    nName -= 8;
    isJournal = 1;
  }

  pReal = pFsVfs->pFileList; 
  لكل(; pReal && مزي_پرتله(pReal->zName, z_لار, nName); pReal=pReal->p_بل);

  *p_Res_بهر = (pReal && (!isJournal || pReal->nJournal>0));
  أرجع SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (FS_MAX_PATHNAME+1) bytes.
*/
ساكن صحيح fs_ډک_Pathname(
  sqlite3_vfs *pVfs,            /* Pointer to vfs object */
  ثابت حرف *z_لار,            /* Possibly relative input path */
  صحيح n_بهر,                     /* Size of output buffer in bytes */
  حرف *z_بهر                    /* Output buffer */
){
  sqlite3_vfs *p_پلار = ((fs_vfs_t *)pVfs)->p_پلار;
  أرجع p_پلار->x_ډک_Pathname(p_پلار, z_لار, n_بهر, z_بهر);
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
ساكن فراغ *fsDlOpen(sqlite3_vfs *pVfs, ثابت حرف *z_لار){
  sqlite3_vfs *p_پلار = ((fs_vfs_t *)pVfs)->p_پلار;
  أرجع p_پلار->xDlOpen(p_پلار, z_لار);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
ساكن فراغ fsDlError(sqlite3_vfs *pVfs, صحيح nByte, حرف *z_Err_پیغام){
  sqlite3_vfs *p_پلار = ((fs_vfs_t *)pVfs)->p_پلار;
  p_پلار->xDlError(p_پلار, nByte, z_Err_پیغام);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
ساكن فراغ (*fsDlSym(sqlite3_vfs *pVfs, فراغ *pH, ثابت حرف *zSym))(فراغ){
  sqlite3_vfs *p_پلار = ((fs_vfs_t *)pVfs)->p_پلار;
  أرجع p_پلار->xDlSym(p_پلار, pH, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
ساكن فراغ fsDlClose(sqlite3_vfs *pVfs, فراغ *pHandle){
  sqlite3_vfs *p_پلار = ((fs_vfs_t *)pVfs)->p_پلار;
  p_پلار->xDlClose(p_پلار, pHandle);
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
ساكن صحيح fsRandomness(sqlite3_vfs *pVfs, صحيح nByte, حرف *z_Buf_بهر){
  sqlite3_vfs *p_پلار = ((fs_vfs_t *)pVfs)->p_پلار;
  أرجع p_پلار->xRandomness(p_پلار, nByte, z_Buf_بهر);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
ساكن صحيح fsSleep(sqlite3_vfs *pVfs, صحيح nMicro){
  sqlite3_vfs *p_پلار = ((fs_vfs_t *)pVfs)->p_پلار;
  أرجع p_پلار->xSleep(p_پلار, nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
ساكن صحيح fs_Current_وخت(sqlite3_vfs *pVfs, مزدوج *p_وخت_بهر){
  sqlite3_vfs *p_پلار = ((fs_vfs_t *)pVfs)->p_پلار;
  أرجع p_پلار->x_Current_وخت(p_پلار, p_وخت_بهر);
}

/*
** This procedure registers the fs vfs with SQLite. If the argument is
** true, the fs vfs becomes the new default vfs. It is the only publicly
** available function in this file.
*/
صحيح fs_ثبتوونکی(فراغ){
  إذا( fs_vfs.p_پلار ) أرجع SQLITE_OK;
  fs_vfs.p_پلار = sqlite3_vfs_پیدا_کړه(0);
  fs_vfs.بنسټ.mxPathname = fs_vfs.p_پلار->mxPathname;
  fs_vfs.بنسټ.szOsFile = MAX(حجم(tmp_file), حجم(fs_file));
  أرجع sqlite3_vfs_ثبتوونکی(&fs_vfs.بنسټ, 0);
}

#که_تعریف_شوی SQLITE_TEST
  صحيح SqlitetestOnefile_Init() {أرجع fs_ثبتوونکی();}
#د_که_پای
