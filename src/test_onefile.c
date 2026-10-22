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

#شامل "sqlite3.h"
#شامل <assert.h>
#شامل <string.h>

/*
** Maximum pathname length supported by the fs backend.
*/
#تعریف_کرو BLOCKSIZE 512
#تعریف_کرو BLOBSIZE 10485760

/*
** Name used to identify this VFS.
*/
#تعریف_کرو FS_VFS_NAME "fs"

قسمتعریف ساخت fs_real_file fs_real_file;
ساخت fs_real_file {
  sqlite3_file *pFile;
  ثابت حرف *zName;
  صحیح nDatabase;              /* Current size of database region */
  صحیح nJournal;               /* Current size of journal region */
  صحیح nBlob;                  /* Total size of allocated blob */
  صحیح nRef;                   /* Number of pointers to this structure */
  fs_real_file *p_اگلا;
  fs_real_file **ppThis;
};

قسمتعریف ساخت fs_file fs_file;
ساخت fs_file {
  sqlite3_file بنیاد;
  صحیح eType;
  fs_real_file *pReal;
};

قسمتعریف ساخت tmp_file tmp_file;
ساخت tmp_file {
  sqlite3_file بنیاد;
  صحیح n_سائز;
  صحیح nAlloc;
  حرف *zAlloc;
};

/* Values for fs_file.eType. */
#تعریف_کرو DATABASE_FILE   1
#تعریف_کرو JOURNAL_FILE    2

/*
** Method declarations for fs_file.
*/
ساکن صحیح fsClose(sqlite3_file*);
ساکن صحیح fsRead(sqlite3_file*, خالی*, صحیح iAmt, sqlite3_int64 iOfst);
ساکن صحیح fsWrite(sqlite3_file*, ثابت خالی*, صحیح iAmt, sqlite3_int64 iOfst);
ساکن صحیح fsTruncate(sqlite3_file*, sqlite3_int64 سائز);
ساکن صحیح fsSync(sqlite3_file*, صحیح flags);
ساکن صحیح fs_File_سائز(sqlite3_file*, sqlite3_int64 *p_سائز);
ساکن صحیح fs_تالا(sqlite3_file*, صحیح);
ساکن صحیح fsUnlock(sqlite3_file*, صحیح);
ساکن صحیح fs_Check_Reserved_تالا(sqlite3_file*, صحیح *p_Res_برآمد);
ساکن صحیح fsFileControl(sqlite3_file*, صحیح op, خالی *pArg);
ساکن صحیح fs_Sector_سائز(sqlite3_file*);
ساکن صحیح fsDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for tmp_file.
*/
ساکن صحیح tmpClose(sqlite3_file*);
ساکن صحیح tmpRead(sqlite3_file*, خالی*, صحیح iAmt, sqlite3_int64 iOfst);
ساکن صحیح tmpWrite(sqlite3_file*, ثابت خالی*, صحیح iAmt, sqlite3_int64 iOfst);
ساکن صحیح tmpTruncate(sqlite3_file*, sqlite3_int64 سائز);
ساکن صحیح tmpSync(sqlite3_file*, صحیح flags);
ساکن صحیح tmp_File_سائز(sqlite3_file*, sqlite3_int64 *p_سائز);
ساکن صحیح tmp_تالا(sqlite3_file*, صحیح);
ساکن صحیح tmpUnlock(sqlite3_file*, صحیح);
ساکن صحیح tmp_Check_Reserved_تالا(sqlite3_file*, صحیح *p_Res_برآمد);
ساکن صحیح tmpFileControl(sqlite3_file*, صحیح op, خالی *pArg);
ساکن صحیح tmp_Sector_سائز(sqlite3_file*);
ساکن صحیح tmpDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for fs_vfs.
*/
ساکن صحیح fsOpen(sqlite3_vfs*, ثابت حرف *, sqlite3_file*, صحیح , صحیح *);
ساکن صحیح fsDelete(sqlite3_vfs*, ثابت حرف *zName, صحیح syncDir);
ساکن صحیح fsAccess(sqlite3_vfs*, ثابت حرف *zName, صحیح flags, صحیح *);
ساکن صحیح fs_بھرا_Pathname(sqlite3_vfs*, ثابت حرف *zName, صحیح n_برآمد,حرف *z_برآمد);
ساکن خالی *fsDlOpen(sqlite3_vfs*, ثابت حرف *zFilename);
ساکن خالی fsDlError(sqlite3_vfs*, صحیح nByte, حرف *z_Err_پیغام);
ساکن خالی (*fsDlSym(sqlite3_vfs*,خالی*, ثابت حرف *z_علامت))(خالی);
ساکن خالی fsDlClose(sqlite3_vfs*, خالی*);
ساکن صحیح fsRandomness(sqlite3_vfs*, صحیح nByte, حرف *z_برآمد);
ساکن صحیح fsSleep(sqlite3_vfs*, صحیح microseconds);
ساکن صحیح fs_Current_وقت(sqlite3_vfs*, دوہرا*);


قسمتعریف ساخت fs_vfs_t fs_vfs_t;
ساخت fs_vfs_t {
  sqlite3_vfs بنیاد;
  fs_real_file *pFileList;
  sqlite3_vfs *p_والد;
};

ساکن fs_vfs_t fs_vfs = {
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
    fs_بھرا_Pathname,                             /* xFullPathname */
    fsDlOpen,                                   /* xDlOpen */
    fsDlError,                                  /* xDlError */
    fsDlSym,                                    /* xDlSym */
    fsDlClose,                                  /* xDlClose */
    fsRandomness,                               /* xRandomness */
    fsSleep,                                    /* xSleep */
    fs_Current_وقت,                              /* xCurrentTime */
    0                                           /* xCurrentTimeInt64 */
  }, 
  0,                                            /* pFileList */
  0                                             /* pParent */
};

ساکن sqlite3_io_methods fs_io_methods = {
  1,                            /* iVersion */
  fsClose,                      /* xClose */
  fsRead,                       /* xRead */
  fsWrite,                      /* xWrite */
  fsTruncate,                   /* xTruncate */
  fsSync,                       /* xSync */
  fs_File_سائز,                   /* xFileSize */
  fs_تالا,                       /* xLock */
  fsUnlock,                     /* xUnlock */
  fs_Check_Reserved_تالا,          /* xCheckReservedLock */
  fsFileControl,                /* xFileControl */
  fs_Sector_سائز,                 /* xSectorSize */
  fsDeviceCharacteristics,      /* xDeviceCharacteristics */
  0,                            /* xShmMap */
  0,                            /* xShmLock */
  0,                            /* xShmBarrier */
  0                             /* xShmUnmap */
};


ساکن sqlite3_io_methods tmp_io_methods = {
  1,                            /* iVersion */
  tmpClose,                     /* xClose */
  tmpRead,                      /* xRead */
  tmpWrite,                     /* xWrite */
  tmpTruncate,                  /* xTruncate */
  tmpSync,                      /* xSync */
  tmp_File_سائز,                  /* xFileSize */
  tmp_تالا,                      /* xLock */
  tmpUnlock,                    /* xUnlock */
  tmp_Check_Reserved_تالا,         /* xCheckReservedLock */
  tmpFileControl,               /* xFileControl */
  tmp_Sector_سائز,                /* xSectorSize */
  tmpDeviceCharacteristics,     /* xDeviceCharacteristics */
  0,                            /* xShmMap */
  0,                            /* xShmLock */
  0,                            /* xShmBarrier */
  0                             /* xShmUnmap */
};

/* Useful macros used in several places */
#تعریف_کرو MIN(x,y) ((x)<(y)?(x):(y))
#تعریف_کرو MAX(x,y) ((x)>(y)?(x):(y))


/*
** Close a tmp-file.
*/
ساکن صحیح tmpClose(sqlite3_file *pFile){
  tmp_file *pTmp = (tmp_file *)pFile;
  sqlite3_free(pTmp->zAlloc);
  واپس SQLITE_OK;
}

/*
** Read data from a tmp-file.
*/
ساکن صحیح tmpRead(
  sqlite3_file *pFile, 
  خالی *zBuf, 
  صحیح iAmt, 
  sqlite_int64 iOfst
){
  tmp_file *pTmp = (tmp_file *)pFile;
  اگر( (iAmt+iOfst)>pTmp->n_سائز ){
    واپس SQLITE_IOERR_SHORT_READ;
  }
  حافظہ_نقل(zBuf, &pTmp->zAlloc[iOfst], iAmt);
  واپس SQLITE_OK;
}

/*
** Write data to a tmp-file.
*/
ساکن صحیح tmpWrite(
  sqlite3_file *pFile, 
  ثابت خالی *zBuf, 
  صحیح iAmt, 
  sqlite_int64 iOfst
){
  tmp_file *pTmp = (tmp_file *)pFile;
  اگر( (iAmt+iOfst)>pTmp->nAlloc ){
    صحیح nNew = (صحیح)(2*(iAmt+iOfst+pTmp->nAlloc));
    حرف *zNew = sqlite3_realloc(pTmp->zAlloc, nNew);
    اگر( !zNew ){
      واپس SQLITE_NOMEM;
    }
    pTmp->zAlloc = zNew;
    pTmp->nAlloc = nNew;
  }
  حافظہ_نقل(&pTmp->zAlloc[iOfst], zBuf, iAmt);
  pTmp->n_سائز = (صحیح)MAX(pTmp->n_سائز, iOfst+iAmt);
  واپس SQLITE_OK;
}

/*
** Truncate a tmp-file.
*/
ساکن صحیح tmpTruncate(sqlite3_file *pFile, sqlite_int64 سائز){
  tmp_file *pTmp = (tmp_file *)pFile;
  pTmp->n_سائز = (صحیح)MIN(pTmp->n_سائز, سائز);
  واپس SQLITE_OK;
}

/*
** Sync a tmp-file.
*/
ساکن صحیح tmpSync(sqlite3_file *pFile, صحیح flags){
  واپس SQLITE_OK;
}

/*
** Return the current file-size of a tmp-file.
*/
ساکن صحیح tmp_File_سائز(sqlite3_file *pFile, sqlite_int64 *p_سائز){
  tmp_file *pTmp = (tmp_file *)pFile;
  *p_سائز = pTmp->n_سائز;
  واپس SQLITE_OK;
}

/*
** Lock a tmp-file.
*/
ساکن صحیح tmp_تالا(sqlite3_file *pFile, صحیح e_تالا){
  واپس SQLITE_OK;
}

/*
** Unlock a tmp-file.
*/
ساکن صحیح tmpUnlock(sqlite3_file *pFile, صحیح e_تالا){
  واپس SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on a tmp-file.
*/
ساکن صحیح tmp_Check_Reserved_تالا(sqlite3_file *pFile, صحیح *p_Res_برآمد){
  *p_Res_برآمد = 0;
  واپس SQLITE_OK;
}

/*
** File control method. For custom operations on a tmp-file.
*/
ساکن صحیح tmpFileControl(sqlite3_file *pFile, صحیح op, خالی *pArg){
  واپس SQLITE_OK;
}

/*
** Return the sector-size in bytes for a tmp-file.
*/
ساکن صحیح tmp_Sector_سائز(sqlite3_file *pFile){
  واپس 0;
}

/*
** Return the device characteristic flags supported by a tmp-file.
*/
ساکن صحیح tmpDeviceCharacteristics(sqlite3_file *pFile){
  واپس 0;
}

/*
** Close an fs-file.
*/
ساکن صحیح fsClose(sqlite3_file *pFile){
  صحیح rc = SQLITE_OK;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;

  /* Decrement the real_file ref-count. */
  pReal->nRef--;
  أكد(pReal->nRef>=0);

  /* When the ref-count reaches 0, destroy the structure */
  اگر( pReal->nRef==0 ){
    *pReal->ppThis = pReal->p_اگلا;
    اگر( pReal->p_اگلا ){
      pReal->p_اگلا->ppThis = pReal->ppThis;
    }
    rc = pReal->pFile->pMethods->xClose(pReal->pFile);
    sqlite3_free(pReal);
  }

  واپس rc;
}

/*
** Read data from an fs-file.
*/
ساکن صحیح fsRead(
  sqlite3_file *pFile, 
  خالی *zBuf, 
  صحیح iAmt, 
  sqlite_int64 iOfst
){
  صحیح rc = SQLITE_OK;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  sqlite3_file *pF = pReal->pFile;

  اگر( (p->eType==DATABASE_FILE && (iAmt+iOfst)>pReal->nDatabase)
   || (p->eType==JOURNAL_FILE && (iAmt+iOfst)>pReal->nJournal)
  ){
    rc = SQLITE_IOERR_SHORT_READ;
  }ورنہ اگر( p->eType==DATABASE_FILE ){
    rc = pF->pMethods->xRead(pF, zBuf, iAmt, iOfst+BLOCKSIZE);
  }ورنہ{
    /* Journal file. */
    صحیح iRem = iAmt;
    صحیح iBuf = 0;
    صحیح ii = (صحیح)iOfst;
    جب( iRem>0 && rc==SQLITE_OK ){
      صحیح iRealOff = pReal->nBlob - BLOCKSIZE*((ii/BLOCKSIZE)+1) + ii%BLOCKSIZE;
      صحیح iRealAmt = MIN(iRem, BLOCKSIZE - (iRealOff%BLOCKSIZE));

      rc = pF->pMethods->xRead(pF, &((حرف *)zBuf)[iBuf], iRealAmt, iRealOff);
      ii += iRealAmt;
      iBuf += iRealAmt;
      iRem -= iRealAmt;
    }
  }

  واپس rc;
}

/*
** Write data to an fs-file.
*/
ساکن صحیح fsWrite(
  sqlite3_file *pFile, 
  ثابت خالی *zBuf, 
  صحیح iAmt, 
  sqlite_int64 iOfst
){
  صحیح rc = SQLITE_OK;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  sqlite3_file *pF = pReal->pFile;

  اگر( p->eType==DATABASE_FILE ){
    اگر( (iAmt+iOfst+BLOCKSIZE)>(pReal->nBlob-pReal->nJournal) ){
      rc = SQLITE_بھرا;
    }ورنہ{
      rc = pF->pMethods->xWrite(pF, zBuf, iAmt, iOfst+BLOCKSIZE);
      اگر( rc==SQLITE_OK ){
        pReal->nDatabase = (صحیح)MAX(pReal->nDatabase, iAmt+iOfst);
      }
    }
  }ورنہ{
    /* Journal file. */
    صحیح iRem = iAmt;
    صحیح iBuf = 0;
    صحیح ii = (صحیح)iOfst;
    جب( iRem>0 && rc==SQLITE_OK ){
      صحیح iRealOff = pReal->nBlob - BLOCKSIZE*((ii/BLOCKSIZE)+1) + ii%BLOCKSIZE;
      صحیح iRealAmt = MIN(iRem, BLOCKSIZE - (iRealOff%BLOCKSIZE));

      اگر( iRealOff<(pReal->nDatabase+BLOCKSIZE) ){
        rc = SQLITE_بھرا;
      }ورنہ{
        rc = pF->pMethods->xWrite(pF, &((حرف *)zBuf)[iBuf], iRealAmt,iRealOff);
        ii += iRealAmt;
        iBuf += iRealAmt;
        iRem -= iRealAmt;
      }
    }
    اگر( rc==SQLITE_OK ){
      pReal->nJournal = (صحیح)MAX(pReal->nJournal, iAmt+iOfst);
    }
  }

  واپس rc;
}

/*
** Truncate an fs-file.
*/
ساکن صحیح fsTruncate(sqlite3_file *pFile, sqlite_int64 سائز){
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  اگر( p->eType==DATABASE_FILE ){
    pReal->nDatabase = (صحیح)MIN(pReal->nDatabase, سائز);
  }ورنہ{
    pReal->nJournal = (صحیح)MIN(pReal->nJournal, سائز);
  }
  واپس SQLITE_OK;
}

/*
** Sync an fs-file.
*/
ساکن صحیح fsSync(sqlite3_file *pFile, صحیح flags){
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  sqlite3_file *pRealFile = pReal->pFile;
  صحیح rc = SQLITE_OK;

  اگر( p->eType==DATABASE_FILE ){
    غیرعلامتی حرف z_سائز[4];
    z_سائز[0] = (pReal->nDatabase&0xFF000000)>>24;
    z_سائز[1] = (غیرعلامتی حرف)((pReal->nDatabase&0x00FF0000)>>16);
    z_سائز[2] = (pReal->nDatabase&0x0000FF00)>>8;
    z_سائز[3] = (pReal->nDatabase&0x000000FF);
    rc = pRealFile->pMethods->xWrite(pRealFile, z_سائز, 4, 0);
  }
  اگر( rc==SQLITE_OK ){
    rc = pRealFile->pMethods->xSync(pRealFile, flags&(~SQLITE_SYNC_DATAONLY));
  }

  واپس rc;
}

/*
** Return the current file-size of an fs-file.
*/
ساکن صحیح fs_File_سائز(sqlite3_file *pFile, sqlite_int64 *p_سائز){
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  اگر( p->eType==DATABASE_FILE ){
    *p_سائز = pReal->nDatabase;
  }ورنہ{
    *p_سائز = pReal->nJournal;
  }
  واپس SQLITE_OK;
}

/*
** Lock an fs-file.
*/
ساکن صحیح fs_تالا(sqlite3_file *pFile, صحیح e_تالا){
  واپس SQLITE_OK;
}

/*
** Unlock an fs-file.
*/
ساکن صحیح fsUnlock(sqlite3_file *pFile, صحیح e_تالا){
  واپس SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on an fs-file.
*/
ساکن صحیح fs_Check_Reserved_تالا(sqlite3_file *pFile, صحیح *p_Res_برآمد){
  *p_Res_برآمد = 0;
  واپس SQLITE_OK;
}

/*
** File control method. For custom operations on an fs-file.
*/
ساکن صحیح fsFileControl(sqlite3_file *pFile, صحیح op, خالی *pArg){
  اگر( op==SQLITE_FCNTL_PRAGMA ) واپس SQLITE_NOTFOUND;
  واپس SQLITE_OK;
}

/*
** Return the sector-size in bytes for an fs-file.
*/
ساکن صحیح fs_Sector_سائز(sqlite3_file *pFile){
  واپس BLOCKSIZE;
}

/*
** Return the device characteristic flags supported by an fs-file.
*/
ساکن صحیح fsDeviceCharacteristics(sqlite3_file *pFile){
  واپس 0;
}

/*
** Open an fs file handle.
*/
ساکن صحیح fsOpen(
  sqlite3_vfs *pVfs,
  ثابت حرف *zName,
  sqlite3_file *pFile,
  صحیح flags,
  صحیح *p_برآمد_Flags
){
  fs_vfs_t *pFsVfs = (fs_vfs_t *)pVfs;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = 0;
  صحیح eType;
  صحیح nName;
  صحیح rc = SQLITE_OK;

  اگر( 0==(flags&(SQLITE_OPEN_MAIN_DB|SQLITE_OPEN_MAIN_JOURNAL)) ){
    tmp_file *p2 = (tmp_file *)pFile;
    memset(p2, 0, حجم(*p2));
    p2->بنیاد.pMethods = &tmp_io_methods;
    واپس SQLITE_OK;
  }

  eType = ((flags&(SQLITE_OPEN_MAIN_DB))?DATABASE_FILE:JOURNAL_FILE);
  p->بنیاد.pMethods = &fs_io_methods;
  p->eType = eType;

  أكد(لڑی_لمبائی("-journal")==8);
  nName = (صحیح)لڑی_لمبائی(zName)-((eType==JOURNAL_FILE)?8:0);
  pReal=pFsVfs->pFileList; 
  برائے(; pReal && لڑی_موازنہ(pReal->zName, zName, nName); pReal=pReal->p_اگلا);

  اگر( !pReal ){
    صحیح real_flags = (flags&~(SQLITE_OPEN_MAIN_DB))|SQLITE_OPEN_عارضی_DB;
    sqlite3_int64 سائز;
    sqlite3_file *pRealFile;
    sqlite3_vfs *p_والد = pFsVfs->p_والد;
    أكد(eType==DATABASE_FILE);

    pReal = (fs_real_file *)sqlite3_malloc(حجم(*pReal)+p_والد->szOsFile);
    اگر( !pReal ){
      rc = SQLITE_NOMEM;
      جاؤ open_برآمد;
    }
    memset(pReal, 0, حجم(*pReal)+p_والد->szOsFile);
    pReal->zName = zName;
    pReal->pFile = (sqlite3_file *)(&pReal[1]);

    rc = p_والد->xOpen(p_والد, zName, pReal->pFile, real_flags, p_برآمد_Flags);
    اگر( rc!=SQLITE_OK ){
      جاؤ open_برآمد;
    }
    pRealFile = pReal->pFile;

    rc = pRealFile->pMethods->x_File_سائز(pRealFile, &سائز);
    اگر( rc!=SQLITE_OK ){
      جاؤ open_برآمد;
    }
    اگر( سائز==0 ){
      rc = pRealFile->pMethods->xWrite(pRealFile, "\0", 1, BLOBSIZE-1);
      pReal->nBlob = BLOBSIZE;
    }ورنہ{
      غیرعلامتی حرف zS[4];
      pReal->nBlob = (صحیح)سائز;
      rc = pRealFile->pMethods->xRead(pRealFile, zS, 4, 0);
      pReal->nDatabase = (zS[0]<<24)+(zS[1]<<16)+(zS[2]<<8)+zS[3];
      اگر( rc==SQLITE_OK ){
        rc = pRealFile->pMethods->xRead(pRealFile, zS, 4, pReal->nBlob-4);
        اگر( zS[0] || zS[1] || zS[2] || zS[3] ){
          pReal->nJournal = pReal->nBlob;
        }
      }
    }

    اگر( rc==SQLITE_OK ){
      pReal->p_اگلا = pFsVfs->pFileList;
      اگر( pReal->p_اگلا ){
        pReal->p_اگلا->ppThis = &pReal->p_اگلا;
      }
      pReal->ppThis = &pFsVfs->pFileList;
      pFsVfs->pFileList = pReal;
    }
  }

open_برآمد:
  اگر( pReal ){
    اگر( rc==SQLITE_OK ){
      p->pReal = pReal;
      pReal->nRef++;
    }ورنہ{
      اگر( pReal->pFile->pMethods ){
        pReal->pFile->pMethods->xClose(pReal->pFile);
      }
      sqlite3_free(pReal);
    }
  }
  واپس rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
ساکن صحیح fsDelete(sqlite3_vfs *pVfs, ثابت حرف *z_راستہ, صحیح dirSync){
  صحیح rc = SQLITE_OK;
  fs_vfs_t *pFsVfs = (fs_vfs_t *)pVfs;
  fs_real_file *pReal;
  sqlite3_file *pF;
  صحیح nName = (صحیح)لڑی_لمبائی(z_راستہ) - 8;

  أكد(لڑی_لمبائی("-journal")==8);
  أكد(strcmp("-journal", &z_راستہ[nName])==0);

  pReal = pFsVfs->pFileList; 
  برائے(; pReal && لڑی_موازنہ(pReal->zName, z_راستہ, nName); pReal=pReal->p_اگلا);
  اگر( pReal ){
    pF = pReal->pFile;
    rc = pF->pMethods->xWrite(pF, "\0\0\0\0", 4, pReal->nBlob-BLOCKSIZE);
    اگر( rc==SQLITE_OK ){
      pReal->nJournal = 0;
    }
  }
  واپس rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
ساکن صحیح fsAccess(
  sqlite3_vfs *pVfs, 
  ثابت حرف *z_راستہ, 
  صحیح flags, 
  صحیح *p_Res_برآمد
){
  fs_vfs_t *pFsVfs = (fs_vfs_t *)pVfs;
  fs_real_file *pReal;
  صحیح isJournal = 0;
  صحیح nName = (صحیح)لڑی_لمبائی(z_راستہ);

  اگر( flags!=SQLITE_ACCESS_EXISTS ){
    sqlite3_vfs *p_والد = ((fs_vfs_t *)pVfs)->p_والد;
    واپس p_والد->xAccess(p_والد, z_راستہ, flags, p_Res_برآمد);
  }

  أكد(لڑی_لمبائی("-journal")==8);
  اگر( nName>8 && strcmp("-journal", &z_راستہ[nName-8])==0 ){
    nName -= 8;
    isJournal = 1;
  }

  pReal = pFsVfs->pFileList; 
  برائے(; pReal && لڑی_موازنہ(pReal->zName, z_راستہ, nName); pReal=pReal->p_اگلا);

  *p_Res_برآمد = (pReal && (!isJournal || pReal->nJournal>0));
  واپس SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (FS_MAX_PATHNAME+1) bytes.
*/
ساکن صحیح fs_بھرا_Pathname(
  sqlite3_vfs *pVfs,            /* Pointer to vfs object */
  ثابت حرف *z_راستہ,            /* Possibly relative input path */
  صحیح n_برآمد,                     /* Size of output buffer in bytes */
  حرف *z_برآمد                    /* Output buffer */
){
  sqlite3_vfs *p_والد = ((fs_vfs_t *)pVfs)->p_والد;
  واپس p_والد->x_بھرا_Pathname(p_والد, z_راستہ, n_برآمد, z_برآمد);
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
ساکن خالی *fsDlOpen(sqlite3_vfs *pVfs, ثابت حرف *z_راستہ){
  sqlite3_vfs *p_والد = ((fs_vfs_t *)pVfs)->p_والد;
  واپس p_والد->xDlOpen(p_والد, z_راستہ);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
ساکن خالی fsDlError(sqlite3_vfs *pVfs, صحیح nByte, حرف *z_Err_پیغام){
  sqlite3_vfs *p_والد = ((fs_vfs_t *)pVfs)->p_والد;
  p_والد->xDlError(p_والد, nByte, z_Err_پیغام);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
ساکن خالی (*fsDlSym(sqlite3_vfs *pVfs, خالی *pH, ثابت حرف *zSym))(خالی){
  sqlite3_vfs *p_والد = ((fs_vfs_t *)pVfs)->p_والد;
  واپس p_والد->xDlSym(p_والد, pH, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
ساکن خالی fsDlClose(sqlite3_vfs *pVfs, خالی *pHandle){
  sqlite3_vfs *p_والد = ((fs_vfs_t *)pVfs)->p_والد;
  p_والد->xDlClose(p_والد, pHandle);
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
ساکن صحیح fsRandomness(sqlite3_vfs *pVfs, صحیح nByte, حرف *z_Buf_برآمد){
  sqlite3_vfs *p_والد = ((fs_vfs_t *)pVfs)->p_والد;
  واپس p_والد->xRandomness(p_والد, nByte, z_Buf_برآمد);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
ساکن صحیح fsSleep(sqlite3_vfs *pVfs, صحیح nMicro){
  sqlite3_vfs *p_والد = ((fs_vfs_t *)pVfs)->p_والد;
  واپس p_والد->xSleep(p_والد, nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
ساکن صحیح fs_Current_وقت(sqlite3_vfs *pVfs, دوہرا *p_وقت_برآمد){
  sqlite3_vfs *p_والد = ((fs_vfs_t *)pVfs)->p_والد;
  واپس p_والد->x_Current_وقت(p_والد, p_وقت_برآمد);
}

/*
** This procedure registers the fs vfs with SQLite. If the argument is
** true, the fs vfs becomes the new default vfs. It is the only publicly
** available function in this file.
*/
صحیح fs_رجسٹر(خالی){
  اگر( fs_vfs.p_والد ) واپس SQLITE_OK;
  fs_vfs.p_والد = sqlite3_vfs_ڈھونڈو(0);
  fs_vfs.بنیاد.mxPathname = fs_vfs.p_والد->mxPathname;
  fs_vfs.بنیاد.szOsFile = MAX(حجم(tmp_file), حجم(fs_file));
  واپس sqlite3_vfs_رجسٹر(&fs_vfs.بنیاد, 0);
}

#اگر_تعریف_شدہ SQLITE_TEST
  صحیح SqlitetestOnefile_Init() {واپس fs_رجسٹر();}
#اگر_ختم
