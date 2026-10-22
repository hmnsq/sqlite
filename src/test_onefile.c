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

#अन्तर्भाव "sqlite3.h"
#अन्तर्भाव <assert.h>
#अन्तर्भाव <string.h>

/*
** Maximum pathname length supported by the fs backend.
*/
#define BLOCKSIZE 512
#define BLOBSIZE 10485760

/*
** Name used to identify this VFS.
*/
#define FS_VFS_NAME "fs"

प्रकारव्याख्या संरचना fs_real_file fs_real_file;
संरचना fs_real_file {
  sqlite3_file *pFile;
  स्थिरांक अक्षर *zName;
  पूर्णांक nDatabase;              /* Current size of database region */
  पूर्णांक nJournal;               /* Current size of journal region */
  पूर्णांक nBlob;                  /* Total size of allocated blob */
  पूर्णांक nRef;                   /* Number of pointers to this structure */
  fs_real_file *pNext;
  fs_real_file **ppThis;
};

प्रकारव्याख्या संरचना fs_file fs_file;
संरचना fs_file {
  sqlite3_file base;
  पूर्णांक eType;
  fs_real_file *pReal;
};

प्रकारव्याख्या संरचना tmp_file tmp_file;
संरचना tmp_file {
  sqlite3_file base;
  पूर्णांक nSize;
  पूर्णांक nAlloc;
  अक्षर *zAlloc;
};

/* Values for fs_file.eType. */
#define DATABASE_FILE   1
#define JOURNAL_FILE    2

/*
** Method declarations for fs_file.
*/
स्थिर पूर्णांक fsClose(sqlite3_file*);
स्थिर पूर्णांक fsRead(sqlite3_file*, शून्य*, पूर्णांक iAmt, sqlite3_int64 iOfst);
स्थिर पूर्णांक fsWrite(sqlite3_file*, स्थिरांक शून्य*, पूर्णांक iAmt, sqlite3_int64 iOfst);
स्थिर पूर्णांक fsTruncate(sqlite3_file*, sqlite3_int64 size);
स्थिर पूर्णांक fsSync(sqlite3_file*, पूर्णांक flags);
स्थिर पूर्णांक fsFileSize(sqlite3_file*, sqlite3_int64 *pSize);
स्थिर पूर्णांक fsLock(sqlite3_file*, पूर्णांक);
स्थिर पूर्णांक fsUnlock(sqlite3_file*, पूर्णांक);
स्थिर पूर्णांक fsCheckReservedLock(sqlite3_file*, पूर्णांक *pResOut);
स्थिर पूर्णांक fsFileControl(sqlite3_file*, पूर्णांक op, शून्य *pArg);
स्थिर पूर्णांक fsSectorSize(sqlite3_file*);
स्थिर पूर्णांक fsDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for tmp_file.
*/
स्थिर पूर्णांक tmpClose(sqlite3_file*);
स्थिर पूर्णांक tmpRead(sqlite3_file*, शून्य*, पूर्णांक iAmt, sqlite3_int64 iOfst);
स्थिर पूर्णांक tmpWrite(sqlite3_file*, स्थिरांक शून्य*, पूर्णांक iAmt, sqlite3_int64 iOfst);
स्थिर पूर्णांक tmpTruncate(sqlite3_file*, sqlite3_int64 size);
स्थिर पूर्णांक tmpSync(sqlite3_file*, पूर्णांक flags);
स्थिर पूर्णांक tmpFileSize(sqlite3_file*, sqlite3_int64 *pSize);
स्थिर पूर्णांक tmpLock(sqlite3_file*, पूर्णांक);
स्थिर पूर्णांक tmpUnlock(sqlite3_file*, पूर्णांक);
स्थिर पूर्णांक tmpCheckReservedLock(sqlite3_file*, पूर्णांक *pResOut);
स्थिर पूर्णांक tmpFileControl(sqlite3_file*, पूर्णांक op, शून्य *pArg);
स्थिर पूर्णांक tmpSectorSize(sqlite3_file*);
स्थिर पूर्णांक tmpDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for fs_vfs.
*/
स्थिर पूर्णांक fsOpen(sqlite3_vfs*, स्थिरांक अक्षर *, sqlite3_file*, पूर्णांक , पूर्णांक *);
स्थिर पूर्णांक fsDelete(sqlite3_vfs*, स्थिरांक अक्षर *zName, पूर्णांक syncDir);
स्थिर पूर्णांक fsAccess(sqlite3_vfs*, स्थिरांक अक्षर *zName, पूर्णांक flags, पूर्णांक *);
स्थिर पूर्णांक fsFullPathname(sqlite3_vfs*, स्थिरांक अक्षर *zName, पूर्णांक nOut,अक्षर *zOut);
स्थिर शून्य *fsDlOpen(sqlite3_vfs*, स्थिरांक अक्षर *zFilename);
स्थिर शून्य fsDlError(sqlite3_vfs*, पूर्णांक nByte, अक्षर *z_Err_संदेश);
स्थिर शून्य (*fsDlSym(sqlite3_vfs*,शून्य*, स्थिरांक अक्षर *z_चिन्ह))(शून्य);
स्थिर शून्य fsDlClose(sqlite3_vfs*, शून्य*);
स्थिर पूर्णांक fsRandomness(sqlite3_vfs*, पूर्णांक nByte, अक्षर *zOut);
स्थिर पूर्णांक fsSleep(sqlite3_vfs*, पूर्णांक microseconds);
स्थिर पूर्णांक fsCurrentTime(sqlite3_vfs*, द्विगुण*);


प्रकारव्याख्या संरचना fs_vfs_t fs_vfs_t;
संरचना fs_vfs_t {
  sqlite3_vfs base;
  fs_real_file *pFileList;
  sqlite3_vfs *pParent;
};

स्थिर fs_vfs_t fs_vfs = {
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

स्थिर sqlite3_io_methods fs_io_methods = {
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


स्थिर sqlite3_io_methods tmp_io_methods = {
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
स्थिर पूर्णांक tmpClose(sqlite3_file *pFile){
  tmp_file *pTmp = (tmp_file *)pFile;
  sqlite3_free(pTmp->zAlloc);
  परत SQLITE_OK;
}

/*
** Read data from a tmp-file.
*/
स्थिर पूर्णांक tmpRead(
  sqlite3_file *pFile, 
  शून्य *zBuf, 
  पूर्णांक iAmt, 
  sqlite_int64 iOfst
){
  tmp_file *pTmp = (tmp_file *)pFile;
  जर( (iAmt+iOfst)>pTmp->nSize ){
    परत SQLITE_IOERR_SHORT_READ;
  }
  स_नकल(zBuf, &pTmp->zAlloc[iOfst], iAmt);
  परत SQLITE_OK;
}

/*
** Write data to a tmp-file.
*/
स्थिर पूर्णांक tmpWrite(
  sqlite3_file *pFile, 
  स्थिरांक शून्य *zBuf, 
  पूर्णांक iAmt, 
  sqlite_int64 iOfst
){
  tmp_file *pTmp = (tmp_file *)pFile;
  जर( (iAmt+iOfst)>pTmp->nAlloc ){
    पूर्णांक nNew = (पूर्णांक)(2*(iAmt+iOfst+pTmp->nAlloc));
    अक्षर *zNew = sqlite3_realloc(pTmp->zAlloc, nNew);
    जर( !zNew ){
      परत SQLITE_NOMEM;
    }
    pTmp->zAlloc = zNew;
    pTmp->nAlloc = nNew;
  }
  स_नकल(&pTmp->zAlloc[iOfst], zBuf, iAmt);
  pTmp->nSize = (पूर्णांक)MAX(pTmp->nSize, iOfst+iAmt);
  परत SQLITE_OK;
}

/*
** Truncate a tmp-file.
*/
स्थिर पूर्णांक tmpTruncate(sqlite3_file *pFile, sqlite_int64 size){
  tmp_file *pTmp = (tmp_file *)pFile;
  pTmp->nSize = (पूर्णांक)MIN(pTmp->nSize, size);
  परत SQLITE_OK;
}

/*
** Sync a tmp-file.
*/
स्थिर पूर्णांक tmpSync(sqlite3_file *pFile, पूर्णांक flags){
  परत SQLITE_OK;
}

/*
** Return the current file-size of a tmp-file.
*/
स्थिर पूर्णांक tmpFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  tmp_file *pTmp = (tmp_file *)pFile;
  *pSize = pTmp->nSize;
  परत SQLITE_OK;
}

/*
** Lock a tmp-file.
*/
स्थिर पूर्णांक tmpLock(sqlite3_file *pFile, पूर्णांक eLock){
  परत SQLITE_OK;
}

/*
** Unlock a tmp-file.
*/
स्थिर पूर्णांक tmpUnlock(sqlite3_file *pFile, पूर्णांक eLock){
  परत SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on a tmp-file.
*/
स्थिर पूर्णांक tmpCheckReservedLock(sqlite3_file *pFile, पूर्णांक *pResOut){
  *pResOut = 0;
  परत SQLITE_OK;
}

/*
** File control method. For custom operations on a tmp-file.
*/
स्थिर पूर्णांक tmpFileControl(sqlite3_file *pFile, पूर्णांक op, शून्य *pArg){
  परत SQLITE_OK;
}

/*
** Return the sector-size in bytes for a tmp-file.
*/
स्थिर पूर्णांक tmpSectorSize(sqlite3_file *pFile){
  परत 0;
}

/*
** Return the device characteristic flags supported by a tmp-file.
*/
स्थिर पूर्णांक tmpDeviceCharacteristics(sqlite3_file *pFile){
  परत 0;
}

/*
** Close an fs-file.
*/
स्थिर पूर्णांक fsClose(sqlite3_file *pFile){
  पूर्णांक rc = SQLITE_OK;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;

  /* Decrement the real_file ref-count. */
  pReal->nRef--;
  प्रतिज्ञा(pReal->nRef>=0);

  /* When the ref-count reaches 0, destroy the structure */
  जर( pReal->nRef==0 ){
    *pReal->ppThis = pReal->pNext;
    जर( pReal->pNext ){
      pReal->pNext->ppThis = pReal->ppThis;
    }
    rc = pReal->pFile->pMethods->xClose(pReal->pFile);
    sqlite3_free(pReal);
  }

  परत rc;
}

/*
** Read data from an fs-file.
*/
स्थिर पूर्णांक fsRead(
  sqlite3_file *pFile, 
  शून्य *zBuf, 
  पूर्णांक iAmt, 
  sqlite_int64 iOfst
){
  पूर्णांक rc = SQLITE_OK;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  sqlite3_file *pF = pReal->pFile;

  जर( (p->eType==DATABASE_FILE && (iAmt+iOfst)>pReal->nDatabase)
   || (p->eType==JOURNAL_FILE && (iAmt+iOfst)>pReal->nJournal)
  ){
    rc = SQLITE_IOERR_SHORT_READ;
  }अन्यथा जर( p->eType==DATABASE_FILE ){
    rc = pF->pMethods->xRead(pF, zBuf, iAmt, iOfst+BLOCKSIZE);
  }अन्यथा{
    /* Journal file. */
    पूर्णांक iRem = iAmt;
    पूर्णांक iBuf = 0;
    पूर्णांक ii = (पूर्णांक)iOfst;
    पर्यंत( iRem>0 && rc==SQLITE_OK ){
      पूर्णांक iRealOff = pReal->nBlob - BLOCKSIZE*((ii/BLOCKSIZE)+1) + ii%BLOCKSIZE;
      पूर्णांक iRealAmt = MIN(iRem, BLOCKSIZE - (iRealOff%BLOCKSIZE));

      rc = pF->pMethods->xRead(pF, &((अक्षर *)zBuf)[iBuf], iRealAmt, iRealOff);
      ii += iRealAmt;
      iBuf += iRealAmt;
      iRem -= iRealAmt;
    }
  }

  परत rc;
}

/*
** Write data to an fs-file.
*/
स्थिर पूर्णांक fsWrite(
  sqlite3_file *pFile, 
  स्थिरांक शून्य *zBuf, 
  पूर्णांक iAmt, 
  sqlite_int64 iOfst
){
  पूर्णांक rc = SQLITE_OK;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  sqlite3_file *pF = pReal->pFile;

  जर( p->eType==DATABASE_FILE ){
    जर( (iAmt+iOfst+BLOCKSIZE)>(pReal->nBlob-pReal->nJournal) ){
      rc = SQLITE_FULL;
    }अन्यथा{
      rc = pF->pMethods->xWrite(pF, zBuf, iAmt, iOfst+BLOCKSIZE);
      जर( rc==SQLITE_OK ){
        pReal->nDatabase = (पूर्णांक)MAX(pReal->nDatabase, iAmt+iOfst);
      }
    }
  }अन्यथा{
    /* Journal file. */
    पूर्णांक iRem = iAmt;
    पूर्णांक iBuf = 0;
    पूर्णांक ii = (पूर्णांक)iOfst;
    पर्यंत( iRem>0 && rc==SQLITE_OK ){
      पूर्णांक iRealOff = pReal->nBlob - BLOCKSIZE*((ii/BLOCKSIZE)+1) + ii%BLOCKSIZE;
      पूर्णांक iRealAmt = MIN(iRem, BLOCKSIZE - (iRealOff%BLOCKSIZE));

      जर( iRealOff<(pReal->nDatabase+BLOCKSIZE) ){
        rc = SQLITE_FULL;
      }अन्यथा{
        rc = pF->pMethods->xWrite(pF, &((अक्षर *)zBuf)[iBuf], iRealAmt,iRealOff);
        ii += iRealAmt;
        iBuf += iRealAmt;
        iRem -= iRealAmt;
      }
    }
    जर( rc==SQLITE_OK ){
      pReal->nJournal = (पूर्णांक)MAX(pReal->nJournal, iAmt+iOfst);
    }
  }

  परत rc;
}

/*
** Truncate an fs-file.
*/
स्थिर पूर्णांक fsTruncate(sqlite3_file *pFile, sqlite_int64 size){
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  जर( p->eType==DATABASE_FILE ){
    pReal->nDatabase = (पूर्णांक)MIN(pReal->nDatabase, size);
  }अन्यथा{
    pReal->nJournal = (पूर्णांक)MIN(pReal->nJournal, size);
  }
  परत SQLITE_OK;
}

/*
** Sync an fs-file.
*/
स्थिर पूर्णांक fsSync(sqlite3_file *pFile, पूर्णांक flags){
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  sqlite3_file *pRealFile = pReal->pFile;
  पूर्णांक rc = SQLITE_OK;

  जर( p->eType==DATABASE_FILE ){
    अचिन्हित अक्षर zSize[4];
    zSize[0] = (pReal->nDatabase&0xFF000000)>>24;
    zSize[1] = (अचिन्हित अक्षर)((pReal->nDatabase&0x00FF0000)>>16);
    zSize[2] = (pReal->nDatabase&0x0000FF00)>>8;
    zSize[3] = (pReal->nDatabase&0x000000FF);
    rc = pRealFile->pMethods->xWrite(pRealFile, zSize, 4, 0);
  }
  जर( rc==SQLITE_OK ){
    rc = pRealFile->pMethods->xSync(pRealFile, flags&(~SQLITE_SYNC_DATAONLY));
  }

  परत rc;
}

/*
** Return the current file-size of an fs-file.
*/
स्थिर पूर्णांक fsFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  जर( p->eType==DATABASE_FILE ){
    *pSize = pReal->nDatabase;
  }अन्यथा{
    *pSize = pReal->nJournal;
  }
  परत SQLITE_OK;
}

/*
** Lock an fs-file.
*/
स्थिर पूर्णांक fsLock(sqlite3_file *pFile, पूर्णांक eLock){
  परत SQLITE_OK;
}

/*
** Unlock an fs-file.
*/
स्थिर पूर्णांक fsUnlock(sqlite3_file *pFile, पूर्णांक eLock){
  परत SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on an fs-file.
*/
स्थिर पूर्णांक fsCheckReservedLock(sqlite3_file *pFile, पूर्णांक *pResOut){
  *pResOut = 0;
  परत SQLITE_OK;
}

/*
** File control method. For custom operations on an fs-file.
*/
स्थिर पूर्णांक fsFileControl(sqlite3_file *pFile, पूर्णांक op, शून्य *pArg){
  जर( op==SQLITE_FCNTL_PRAGMA ) परत SQLITE_NOTFOUND;
  परत SQLITE_OK;
}

/*
** Return the sector-size in bytes for an fs-file.
*/
स्थिर पूर्णांक fsSectorSize(sqlite3_file *pFile){
  परत BLOCKSIZE;
}

/*
** Return the device characteristic flags supported by an fs-file.
*/
स्थिर पूर्णांक fsDeviceCharacteristics(sqlite3_file *pFile){
  परत 0;
}

/*
** Open an fs file handle.
*/
स्थिर पूर्णांक fsOpen(
  sqlite3_vfs *pVfs,
  स्थिरांक अक्षर *zName,
  sqlite3_file *pFile,
  पूर्णांक flags,
  पूर्णांक *pOutFlags
){
  fs_vfs_t *pFsVfs = (fs_vfs_t *)pVfs;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = 0;
  पूर्णांक eType;
  पूर्णांक nName;
  पूर्णांक rc = SQLITE_OK;

  जर( 0==(flags&(SQLITE_OPEN_MAIN_DB|SQLITE_OPEN_MAIN_JOURNAL)) ){
    tmp_file *p2 = (tmp_file *)pFile;
    स_रखो(p2, 0, आकार(*p2));
    p2->base.pMethods = &tmp_io_methods;
    परत SQLITE_OK;
  }

  eType = ((flags&(SQLITE_OPEN_MAIN_DB))?DATABASE_FILE:JOURNAL_FILE);
  p->base.pMethods = &fs_io_methods;
  p->eType = eType;

  प्रतिज्ञा(म_माप("-journal")==8);
  nName = (पूर्णांक)म_माप(zName)-((eType==JOURNAL_FILE)?8:0);
  pReal=pFsVfs->pFileList; 
  साठी(; pReal && म_भेदन(pReal->zName, zName, nName); pReal=pReal->pNext);

  जर( !pReal ){
    पूर्णांक real_flags = (flags&~(SQLITE_OPEN_MAIN_DB))|SQLITE_OPEN_TEMP_DB;
    sqlite3_int64 size;
    sqlite3_file *pRealFile;
    sqlite3_vfs *pParent = pFsVfs->pParent;
    प्रतिज्ञा(eType==DATABASE_FILE);

    pReal = (fs_real_file *)sqlite3_malloc(आकार(*pReal)+pParent->szOsFile);
    जर( !pReal ){
      rc = SQLITE_NOMEM;
      जा open_out;
    }
    स_रखो(pReal, 0, आकार(*pReal)+pParent->szOsFile);
    pReal->zName = zName;
    pReal->pFile = (sqlite3_file *)(&pReal[1]);

    rc = pParent->xOpen(pParent, zName, pReal->pFile, real_flags, pOutFlags);
    जर( rc!=SQLITE_OK ){
      जा open_out;
    }
    pRealFile = pReal->pFile;

    rc = pRealFile->pMethods->xFileSize(pRealFile, &size);
    जर( rc!=SQLITE_OK ){
      जा open_out;
    }
    जर( size==0 ){
      rc = pRealFile->pMethods->xWrite(pRealFile, "\0", 1, BLOBSIZE-1);
      pReal->nBlob = BLOBSIZE;
    }अन्यथा{
      अचिन्हित अक्षर zS[4];
      pReal->nBlob = (पूर्णांक)size;
      rc = pRealFile->pMethods->xRead(pRealFile, zS, 4, 0);
      pReal->nDatabase = (zS[0]<<24)+(zS[1]<<16)+(zS[2]<<8)+zS[3];
      जर( rc==SQLITE_OK ){
        rc = pRealFile->pMethods->xRead(pRealFile, zS, 4, pReal->nBlob-4);
        जर( zS[0] || zS[1] || zS[2] || zS[3] ){
          pReal->nJournal = pReal->nBlob;
        }
      }
    }

    जर( rc==SQLITE_OK ){
      pReal->pNext = pFsVfs->pFileList;
      जर( pReal->pNext ){
        pReal->pNext->ppThis = &pReal->pNext;
      }
      pReal->ppThis = &pFsVfs->pFileList;
      pFsVfs->pFileList = pReal;
    }
  }

open_out:
  जर( pReal ){
    जर( rc==SQLITE_OK ){
      p->pReal = pReal;
      pReal->nRef++;
    }अन्यथा{
      जर( pReal->pFile->pMethods ){
        pReal->pFile->pMethods->xClose(pReal->pFile);
      }
      sqlite3_free(pReal);
    }
  }
  परत rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
स्थिर पूर्णांक fsDelete(sqlite3_vfs *pVfs, स्थिरांक अक्षर *zPath, पूर्णांक dirSync){
  पूर्णांक rc = SQLITE_OK;
  fs_vfs_t *pFsVfs = (fs_vfs_t *)pVfs;
  fs_real_file *pReal;
  sqlite3_file *pF;
  पूर्णांक nName = (पूर्णांक)म_माप(zPath) - 8;

  प्रतिज्ञा(म_माप("-journal")==8);
  प्रतिज्ञा(म_भेद("-journal", &zPath[nName])==0);

  pReal = pFsVfs->pFileList; 
  साठी(; pReal && म_भेदन(pReal->zName, zPath, nName); pReal=pReal->pNext);
  जर( pReal ){
    pF = pReal->pFile;
    rc = pF->pMethods->xWrite(pF, "\0\0\0\0", 4, pReal->nBlob-BLOCKSIZE);
    जर( rc==SQLITE_OK ){
      pReal->nJournal = 0;
    }
  }
  परत rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
स्थिर पूर्णांक fsAccess(
  sqlite3_vfs *pVfs, 
  स्थिरांक अक्षर *zPath, 
  पूर्णांक flags, 
  पूर्णांक *pResOut
){
  fs_vfs_t *pFsVfs = (fs_vfs_t *)pVfs;
  fs_real_file *pReal;
  पूर्णांक isJournal = 0;
  पूर्णांक nName = (पूर्णांक)म_माप(zPath);

  जर( flags!=SQLITE_ACCESS_EXISTS ){
    sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
    परत pParent->xAccess(pParent, zPath, flags, pResOut);
  }

  प्रतिज्ञा(म_माप("-journal")==8);
  जर( nName>8 && म_भेद("-journal", &zPath[nName-8])==0 ){
    nName -= 8;
    isJournal = 1;
  }

  pReal = pFsVfs->pFileList; 
  साठी(; pReal && म_भेदन(pReal->zName, zPath, nName); pReal=pReal->pNext);

  *pResOut = (pReal && (!isJournal || pReal->nJournal>0));
  परत SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (FS_MAX_PATHNAME+1) bytes.
*/
स्थिर पूर्णांक fsFullPathname(
  sqlite3_vfs *pVfs,            /* Pointer to vfs object */
  स्थिरांक अक्षर *zPath,            /* Possibly relative input path */
  पूर्णांक nOut,                     /* Size of output buffer in bytes */
  अक्षर *zOut                    /* Output buffer */
){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  परत pParent->xFullPathname(pParent, zPath, nOut, zOut);
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
स्थिर शून्य *fsDlOpen(sqlite3_vfs *pVfs, स्थिरांक अक्षर *zPath){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  परत pParent->xDlOpen(pParent, zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
स्थिर शून्य fsDlError(sqlite3_vfs *pVfs, पूर्णांक nByte, अक्षर *z_Err_संदेश){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  pParent->xDlError(pParent, nByte, z_Err_संदेश);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
स्थिर शून्य (*fsDlSym(sqlite3_vfs *pVfs, शून्य *pH, स्थिरांक अक्षर *zSym))(शून्य){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  परत pParent->xDlSym(pParent, pH, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
स्थिर शून्य fsDlClose(sqlite3_vfs *pVfs, शून्य *pHandle){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  pParent->xDlClose(pParent, pHandle);
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
स्थिर पूर्णांक fsRandomness(sqlite3_vfs *pVfs, पूर्णांक nByte, अक्षर *zBufOut){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  परत pParent->xRandomness(pParent, nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
स्थिर पूर्णांक fsSleep(sqlite3_vfs *pVfs, पूर्णांक nMicro){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  परत pParent->xSleep(pParent, nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
स्थिर पूर्णांक fsCurrentTime(sqlite3_vfs *pVfs, द्विगुण *pTimeOut){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  परत pParent->xCurrentTime(pParent, pTimeOut);
}

/*
** This procedure registers the fs vfs with SQLite. If the argument is
** true, the fs vfs becomes the new default vfs. It is the only publicly
** available function in this file.
*/
पूर्णांक fs_register(शून्य){
  जर( fs_vfs.pParent ) परत SQLITE_OK;
  fs_vfs.pParent = sqlite3_vfs_find(0);
  fs_vfs.base.mxPathname = fs_vfs.pParent->mxPathname;
  fs_vfs.base.szOsFile = MAX(आकार(tmp_file), आकार(fs_file));
  परत sqlite3_vfs_register(&fs_vfs.base, 0);
}

#ifdef SQLITE_TEST
  पूर्णांक SqlitetestOnefile_Init() {परत fs_register();}
#endif
