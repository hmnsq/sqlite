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

#समावेश "sqlite3.h"
#समावेश <assert.h>
#समावेश <string.h>

/*
** Maximum pathname length supported by the fs backend.
*/
#घोषणा BLOCKSIZE 512
#घोषणा BLOBSIZE 10485760

/*
** Name used to identify this VFS.
*/
#घोषणा FS_VFS_NAME "fs"

प्रकार काष्ठा fs_real_file fs_real_file;
काष्ठा fs_real_file {
  sqlite3_file *pFile;
  स्थिर अक्षर *zName;
  पूर्णांक nDatabase;              /* Current size of database region */
  पूर्णांक nJournal;               /* Current size of journal region */
  पूर्णांक nBlob;                  /* Total size of allocated blob */
  पूर्णांक nRef;                   /* Number of pointers to this structure */
  fs_real_file *p_अग्र;
  fs_real_file **ppThis;
};

प्रकार काष्ठा fs_file fs_file;
काष्ठा fs_file {
  sqlite3_file base;
  पूर्णांक eType;
  fs_real_file *pReal;
};

प्रकार काष्ठा tmp_file tmp_file;
काष्ठा tmp_file {
  sqlite3_file base;
  पूर्णांक n_आकार;
  पूर्णांक nAlloc;
  अक्षर *zAlloc;
};

/* Values for fs_file.eType. */
#घोषणा DATABASE_FILE   1
#घोषणा JOURNAL_FILE    2

/*
** Method declarations for fs_file.
*/
अटल पूर्णांक fsClose(sqlite3_file*);
अटल पूर्णांक fsRead(sqlite3_file*, व्योम*, पूर्णांक iAmt, sqlite3_int64 iOfst);
अटल पूर्णांक fsWrite(sqlite3_file*, स्थिर व्योम*, पूर्णांक iAmt, sqlite3_int64 iOfst);
अटल पूर्णांक fsTruncate(sqlite3_file*, sqlite3_int64 आकार);
अटल पूर्णांक fsSync(sqlite3_file*, पूर्णांक flags);
अटल पूर्णांक fs_File_आकार(sqlite3_file*, sqlite3_int64 *p_आकार);
अटल पूर्णांक fsLock(sqlite3_file*, पूर्णांक);
अटल पूर्णांक fsUnlock(sqlite3_file*, पूर्णांक);
अटल पूर्णांक fsCheckReservedLock(sqlite3_file*, पूर्णांक *pResOut);
अटल पूर्णांक fsFileControl(sqlite3_file*, पूर्णांक op, व्योम *pArg);
अटल पूर्णांक fs_Sector_आकार(sqlite3_file*);
अटल पूर्णांक fsDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for tmp_file.
*/
अटल पूर्णांक tmpClose(sqlite3_file*);
अटल पूर्णांक tmpRead(sqlite3_file*, व्योम*, पूर्णांक iAmt, sqlite3_int64 iOfst);
अटल पूर्णांक tmpWrite(sqlite3_file*, स्थिर व्योम*, पूर्णांक iAmt, sqlite3_int64 iOfst);
अटल पूर्णांक tmpTruncate(sqlite3_file*, sqlite3_int64 आकार);
अटल पूर्णांक tmpSync(sqlite3_file*, पूर्णांक flags);
अटल पूर्णांक tmp_File_आकार(sqlite3_file*, sqlite3_int64 *p_आकार);
अटल पूर्णांक tmpLock(sqlite3_file*, पूर्णांक);
अटल पूर्णांक tmpUnlock(sqlite3_file*, पूर्णांक);
अटल पूर्णांक tmpCheckReservedLock(sqlite3_file*, पूर्णांक *pResOut);
अटल पूर्णांक tmpFileControl(sqlite3_file*, पूर्णांक op, व्योम *pArg);
अटल पूर्णांक tmp_Sector_आकार(sqlite3_file*);
अटल पूर्णांक tmpDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for fs_vfs.
*/
अटल पूर्णांक fsOpen(sqlite3_vfs*, स्थिर अक्षर *, sqlite3_file*, पूर्णांक , पूर्णांक *);
अटल पूर्णांक fsDelete(sqlite3_vfs*, स्थिर अक्षर *zName, पूर्णांक syncDir);
अटल पूर्णांक fsAccess(sqlite3_vfs*, स्थिर अक्षर *zName, पूर्णांक flags, पूर्णांक *);
अटल पूर्णांक fsFullPathname(sqlite3_vfs*, स्थिर अक्षर *zName, पूर्णांक nOut,अक्षर *zOut);
अटल व्योम *fsDlOpen(sqlite3_vfs*, स्थिर अक्षर *zFilename);
अटल व्योम fsDlError(sqlite3_vfs*, पूर्णांक nByte, अक्षर *z_Err_संदेश);
अटल व्योम (*fsDlSym(sqlite3_vfs*,व्योम*, स्थिर अक्षर *z_प्रतीक))(व्योम);
अटल व्योम fsDlClose(sqlite3_vfs*, व्योम*);
अटल पूर्णांक fsRandomness(sqlite3_vfs*, पूर्णांक nByte, अक्षर *zOut);
अटल पूर्णांक fsSleep(sqlite3_vfs*, पूर्णांक microseconds);
अटल पूर्णांक fsCurrentTime(sqlite3_vfs*, द्विगुन*);


प्रकार काष्ठा fs_vfs_t fs_vfs_t;
काष्ठा fs_vfs_t {
  sqlite3_vfs base;
  fs_real_file *pFileList;
  sqlite3_vfs *pParent;
};

अटल fs_vfs_t fs_vfs = {
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

अटल sqlite3_io_methods fs_io_methods = {
  1,                            /* iVersion */
  fsClose,                      /* xClose */
  fsRead,                       /* xRead */
  fsWrite,                      /* xWrite */
  fsTruncate,                   /* xTruncate */
  fsSync,                       /* xSync */
  fs_File_आकार,                   /* xFileSize */
  fsLock,                       /* xLock */
  fsUnlock,                     /* xUnlock */
  fsCheckReservedLock,          /* xCheckReservedLock */
  fsFileControl,                /* xFileControl */
  fs_Sector_आकार,                 /* xSectorSize */
  fsDeviceCharacteristics,      /* xDeviceCharacteristics */
  0,                            /* xShmMap */
  0,                            /* xShmLock */
  0,                            /* xShmBarrier */
  0                             /* xShmUnmap */
};


अटल sqlite3_io_methods tmp_io_methods = {
  1,                            /* iVersion */
  tmpClose,                     /* xClose */
  tmpRead,                      /* xRead */
  tmpWrite,                     /* xWrite */
  tmpTruncate,                  /* xTruncate */
  tmpSync,                      /* xSync */
  tmp_File_आकार,                  /* xFileSize */
  tmpLock,                      /* xLock */
  tmpUnlock,                    /* xUnlock */
  tmpCheckReservedLock,         /* xCheckReservedLock */
  tmpFileControl,               /* xFileControl */
  tmp_Sector_आकार,                /* xSectorSize */
  tmpDeviceCharacteristics,     /* xDeviceCharacteristics */
  0,                            /* xShmMap */
  0,                            /* xShmLock */
  0,                            /* xShmBarrier */
  0                             /* xShmUnmap */
};

/* Useful macros used in several places */
#घोषणा MIN(x,y) ((x)<(y)?(x):(y))
#घोषणा MAX(x,y) ((x)>(y)?(x):(y))


/*
** Close a tmp-file.
*/
अटल पूर्णांक tmpClose(sqlite3_file *pFile){
  tmp_file *pTmp = (tmp_file *)pFile;
  sqlite3_free(pTmp->zAlloc);
  वापस SQLITE_OK;
}

/*
** Read data from a tmp-file.
*/
अटल पूर्णांक tmpRead(
  sqlite3_file *pFile, 
  व्योम *zBuf, 
  पूर्णांक iAmt, 
  sqlite_int64 iOfst
){
  tmp_file *pTmp = (tmp_file *)pFile;
  अगर( (iAmt+iOfst)>pTmp->n_आकार ){
    वापस SQLITE_IOERR_SHORT_READ;
  }
  स_नकल(zBuf, &pTmp->zAlloc[iOfst], iAmt);
  वापस SQLITE_OK;
}

/*
** Write data to a tmp-file.
*/
अटल पूर्णांक tmpWrite(
  sqlite3_file *pFile, 
  स्थिर व्योम *zBuf, 
  पूर्णांक iAmt, 
  sqlite_int64 iOfst
){
  tmp_file *pTmp = (tmp_file *)pFile;
  अगर( (iAmt+iOfst)>pTmp->nAlloc ){
    पूर्णांक nNew = (पूर्णांक)(2*(iAmt+iOfst+pTmp->nAlloc));
    अक्षर *zNew = sqlite3_realloc(pTmp->zAlloc, nNew);
    अगर( !zNew ){
      वापस SQLITE_NOMEM;
    }
    pTmp->zAlloc = zNew;
    pTmp->nAlloc = nNew;
  }
  स_नकल(&pTmp->zAlloc[iOfst], zBuf, iAmt);
  pTmp->n_आकार = (पूर्णांक)MAX(pTmp->n_आकार, iOfst+iAmt);
  वापस SQLITE_OK;
}

/*
** Truncate a tmp-file.
*/
अटल पूर्णांक tmpTruncate(sqlite3_file *pFile, sqlite_int64 आकार){
  tmp_file *pTmp = (tmp_file *)pFile;
  pTmp->n_आकार = (पूर्णांक)MIN(pTmp->n_आकार, आकार);
  वापस SQLITE_OK;
}

/*
** Sync a tmp-file.
*/
अटल पूर्णांक tmpSync(sqlite3_file *pFile, पूर्णांक flags){
  वापस SQLITE_OK;
}

/*
** Return the current file-size of a tmp-file.
*/
अटल पूर्णांक tmp_File_आकार(sqlite3_file *pFile, sqlite_int64 *p_आकार){
  tmp_file *pTmp = (tmp_file *)pFile;
  *p_आकार = pTmp->n_आकार;
  वापस SQLITE_OK;
}

/*
** Lock a tmp-file.
*/
अटल पूर्णांक tmpLock(sqlite3_file *pFile, पूर्णांक eLock){
  वापस SQLITE_OK;
}

/*
** Unlock a tmp-file.
*/
अटल पूर्णांक tmpUnlock(sqlite3_file *pFile, पूर्णांक eLock){
  वापस SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on a tmp-file.
*/
अटल पूर्णांक tmpCheckReservedLock(sqlite3_file *pFile, पूर्णांक *pResOut){
  *pResOut = 0;
  वापस SQLITE_OK;
}

/*
** File control method. For custom operations on a tmp-file.
*/
अटल पूर्णांक tmpFileControl(sqlite3_file *pFile, पूर्णांक op, व्योम *pArg){
  वापस SQLITE_OK;
}

/*
** Return the sector-size in bytes for a tmp-file.
*/
अटल पूर्णांक tmp_Sector_आकार(sqlite3_file *pFile){
  वापस 0;
}

/*
** Return the device characteristic flags supported by a tmp-file.
*/
अटल पूर्णांक tmpDeviceCharacteristics(sqlite3_file *pFile){
  वापस 0;
}

/*
** Close an fs-file.
*/
अटल पूर्णांक fsClose(sqlite3_file *pFile){
  पूर्णांक rc = SQLITE_OK;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;

  /* Decrement the real_file ref-count. */
  pReal->nRef--;
  निश्चित(pReal->nRef>=0);

  /* When the ref-count reaches 0, destroy the structure */
  अगर( pReal->nRef==0 ){
    *pReal->ppThis = pReal->p_अग्र;
    अगर( pReal->p_अग्र ){
      pReal->p_अग्र->ppThis = pReal->ppThis;
    }
    rc = pReal->pFile->pMethods->xClose(pReal->pFile);
    sqlite3_free(pReal);
  }

  वापस rc;
}

/*
** Read data from an fs-file.
*/
अटल पूर्णांक fsRead(
  sqlite3_file *pFile, 
  व्योम *zBuf, 
  पूर्णांक iAmt, 
  sqlite_int64 iOfst
){
  पूर्णांक rc = SQLITE_OK;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  sqlite3_file *pF = pReal->pFile;

  अगर( (p->eType==DATABASE_FILE && (iAmt+iOfst)>pReal->nDatabase)
   || (p->eType==JOURNAL_FILE && (iAmt+iOfst)>pReal->nJournal)
  ){
    rc = SQLITE_IOERR_SHORT_READ;
  }अन्यथा अगर( p->eType==DATABASE_FILE ){
    rc = pF->pMethods->xRead(pF, zBuf, iAmt, iOfst+BLOCKSIZE);
  }अन्यथा{
    /* Journal file. */
    पूर्णांक iRem = iAmt;
    पूर्णांक iBuf = 0;
    पूर्णांक ii = (पूर्णांक)iOfst;
    जबतक( iRem>0 && rc==SQLITE_OK ){
      पूर्णांक iRealOff = pReal->nBlob - BLOCKSIZE*((ii/BLOCKSIZE)+1) + ii%BLOCKSIZE;
      पूर्णांक iRealAmt = MIN(iRem, BLOCKSIZE - (iRealOff%BLOCKSIZE));

      rc = pF->pMethods->xRead(pF, &((अक्षर *)zBuf)[iBuf], iRealAmt, iRealOff);
      ii += iRealAmt;
      iBuf += iRealAmt;
      iRem -= iRealAmt;
    }
  }

  वापस rc;
}

/*
** Write data to an fs-file.
*/
अटल पूर्णांक fsWrite(
  sqlite3_file *pFile, 
  स्थिर व्योम *zBuf, 
  पूर्णांक iAmt, 
  sqlite_int64 iOfst
){
  पूर्णांक rc = SQLITE_OK;
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  sqlite3_file *pF = pReal->pFile;

  अगर( p->eType==DATABASE_FILE ){
    अगर( (iAmt+iOfst+BLOCKSIZE)>(pReal->nBlob-pReal->nJournal) ){
      rc = SQLITE_FULL;
    }अन्यथा{
      rc = pF->pMethods->xWrite(pF, zBuf, iAmt, iOfst+BLOCKSIZE);
      अगर( rc==SQLITE_OK ){
        pReal->nDatabase = (पूर्णांक)MAX(pReal->nDatabase, iAmt+iOfst);
      }
    }
  }अन्यथा{
    /* Journal file. */
    पूर्णांक iRem = iAmt;
    पूर्णांक iBuf = 0;
    पूर्णांक ii = (पूर्णांक)iOfst;
    जबतक( iRem>0 && rc==SQLITE_OK ){
      पूर्णांक iRealOff = pReal->nBlob - BLOCKSIZE*((ii/BLOCKSIZE)+1) + ii%BLOCKSIZE;
      पूर्णांक iRealAmt = MIN(iRem, BLOCKSIZE - (iRealOff%BLOCKSIZE));

      अगर( iRealOff<(pReal->nDatabase+BLOCKSIZE) ){
        rc = SQLITE_FULL;
      }अन्यथा{
        rc = pF->pMethods->xWrite(pF, &((अक्षर *)zBuf)[iBuf], iRealAmt,iRealOff);
        ii += iRealAmt;
        iBuf += iRealAmt;
        iRem -= iRealAmt;
      }
    }
    अगर( rc==SQLITE_OK ){
      pReal->nJournal = (पूर्णांक)MAX(pReal->nJournal, iAmt+iOfst);
    }
  }

  वापस rc;
}

/*
** Truncate an fs-file.
*/
अटल पूर्णांक fsTruncate(sqlite3_file *pFile, sqlite_int64 आकार){
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  अगर( p->eType==DATABASE_FILE ){
    pReal->nDatabase = (पूर्णांक)MIN(pReal->nDatabase, आकार);
  }अन्यथा{
    pReal->nJournal = (पूर्णांक)MIN(pReal->nJournal, आकार);
  }
  वापस SQLITE_OK;
}

/*
** Sync an fs-file.
*/
अटल पूर्णांक fsSync(sqlite3_file *pFile, पूर्णांक flags){
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  sqlite3_file *pRealFile = pReal->pFile;
  पूर्णांक rc = SQLITE_OK;

  अगर( p->eType==DATABASE_FILE ){
    अचिन्हित अक्षर z_आकार[4];
    z_आकार[0] = (pReal->nDatabase&0xFF000000)>>24;
    z_आकार[1] = (अचिन्हित अक्षर)((pReal->nDatabase&0x00FF0000)>>16);
    z_आकार[2] = (pReal->nDatabase&0x0000FF00)>>8;
    z_आकार[3] = (pReal->nDatabase&0x000000FF);
    rc = pRealFile->pMethods->xWrite(pRealFile, z_आकार, 4, 0);
  }
  अगर( rc==SQLITE_OK ){
    rc = pRealFile->pMethods->xSync(pRealFile, flags&(~SQLITE_SYNC_DATAONLY));
  }

  वापस rc;
}

/*
** Return the current file-size of an fs-file.
*/
अटल पूर्णांक fs_File_आकार(sqlite3_file *pFile, sqlite_int64 *p_आकार){
  fs_file *p = (fs_file *)pFile;
  fs_real_file *pReal = p->pReal;
  अगर( p->eType==DATABASE_FILE ){
    *p_आकार = pReal->nDatabase;
  }अन्यथा{
    *p_आकार = pReal->nJournal;
  }
  वापस SQLITE_OK;
}

/*
** Lock an fs-file.
*/
अटल पूर्णांक fsLock(sqlite3_file *pFile, पूर्णांक eLock){
  वापस SQLITE_OK;
}

/*
** Unlock an fs-file.
*/
अटल पूर्णांक fsUnlock(sqlite3_file *pFile, पूर्णांक eLock){
  वापस SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on an fs-file.
*/
अटल पूर्णांक fsCheckReservedLock(sqlite3_file *pFile, पूर्णांक *pResOut){
  *pResOut = 0;
  वापस SQLITE_OK;
}

/*
** File control method. For custom operations on an fs-file.
*/
अटल पूर्णांक fsFileControl(sqlite3_file *pFile, पूर्णांक op, व्योम *pArg){
  अगर( op==SQLITE_FCNTL_PRAGMA ) वापस SQLITE_NOTFOUND;
  वापस SQLITE_OK;
}

/*
** Return the sector-size in bytes for an fs-file.
*/
अटल पूर्णांक fs_Sector_आकार(sqlite3_file *pFile){
  वापस BLOCKSIZE;
}

/*
** Return the device characteristic flags supported by an fs-file.
*/
अटल पूर्णांक fsDeviceCharacteristics(sqlite3_file *pFile){
  वापस 0;
}

/*
** Open an fs file handle.
*/
अटल पूर्णांक fsOpen(
  sqlite3_vfs *pVfs,
  स्थिर अक्षर *zName,
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

  अगर( 0==(flags&(SQLITE_OPEN_MAIN_DB|SQLITE_OPEN_MAIN_JOURNAL)) ){
    tmp_file *p2 = (tmp_file *)pFile;
    स_रखो(p2, 0, माप(*p2));
    p2->base.pMethods = &tmp_io_methods;
    वापस SQLITE_OK;
  }

  eType = ((flags&(SQLITE_OPEN_MAIN_DB))?DATABASE_FILE:JOURNAL_FILE);
  p->base.pMethods = &fs_io_methods;
  p->eType = eType;

  निश्चित(म_माप("-journal")==8);
  nName = (पूर्णांक)म_माप(zName)-((eType==JOURNAL_FILE)?8:0);
  pReal=pFsVfs->pFileList; 
  क्रम(; pReal && म_भेदन(pReal->zName, zName, nName); pReal=pReal->p_अग्र);

  अगर( !pReal ){
    पूर्णांक real_flags = (flags&~(SQLITE_OPEN_MAIN_DB))|SQLITE_OPEN_अस्थायी_DB;
    sqlite3_int64 आकार;
    sqlite3_file *pRealFile;
    sqlite3_vfs *pParent = pFsVfs->pParent;
    निश्चित(eType==DATABASE_FILE);

    pReal = (fs_real_file *)sqlite3_malloc(माप(*pReal)+pParent->szOsFile);
    अगर( !pReal ){
      rc = SQLITE_NOMEM;
      जाओ open_out;
    }
    स_रखो(pReal, 0, माप(*pReal)+pParent->szOsFile);
    pReal->zName = zName;
    pReal->pFile = (sqlite3_file *)(&pReal[1]);

    rc = pParent->xOpen(pParent, zName, pReal->pFile, real_flags, pOutFlags);
    अगर( rc!=SQLITE_OK ){
      जाओ open_out;
    }
    pRealFile = pReal->pFile;

    rc = pRealFile->pMethods->x_File_आकार(pRealFile, &आकार);
    अगर( rc!=SQLITE_OK ){
      जाओ open_out;
    }
    अगर( आकार==0 ){
      rc = pRealFile->pMethods->xWrite(pRealFile, "\0", 1, BLOBSIZE-1);
      pReal->nBlob = BLOBSIZE;
    }अन्यथा{
      अचिन्हित अक्षर zS[4];
      pReal->nBlob = (पूर्णांक)आकार;
      rc = pRealFile->pMethods->xRead(pRealFile, zS, 4, 0);
      pReal->nDatabase = (zS[0]<<24)+(zS[1]<<16)+(zS[2]<<8)+zS[3];
      अगर( rc==SQLITE_OK ){
        rc = pRealFile->pMethods->xRead(pRealFile, zS, 4, pReal->nBlob-4);
        अगर( zS[0] || zS[1] || zS[2] || zS[3] ){
          pReal->nJournal = pReal->nBlob;
        }
      }
    }

    अगर( rc==SQLITE_OK ){
      pReal->p_अग्र = pFsVfs->pFileList;
      अगर( pReal->p_अग्र ){
        pReal->p_अग्र->ppThis = &pReal->p_अग्र;
      }
      pReal->ppThis = &pFsVfs->pFileList;
      pFsVfs->pFileList = pReal;
    }
  }

open_out:
  अगर( pReal ){
    अगर( rc==SQLITE_OK ){
      p->pReal = pReal;
      pReal->nRef++;
    }अन्यथा{
      अगर( pReal->pFile->pMethods ){
        pReal->pFile->pMethods->xClose(pReal->pFile);
      }
      sqlite3_free(pReal);
    }
  }
  वापस rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
अटल पूर्णांक fsDelete(sqlite3_vfs *pVfs, स्थिर अक्षर *zPath, पूर्णांक dirSync){
  पूर्णांक rc = SQLITE_OK;
  fs_vfs_t *pFsVfs = (fs_vfs_t *)pVfs;
  fs_real_file *pReal;
  sqlite3_file *pF;
  पूर्णांक nName = (पूर्णांक)म_माप(zPath) - 8;

  निश्चित(म_माप("-journal")==8);
  निश्चित(म_भेद("-journal", &zPath[nName])==0);

  pReal = pFsVfs->pFileList; 
  क्रम(; pReal && म_भेदन(pReal->zName, zPath, nName); pReal=pReal->p_अग्र);
  अगर( pReal ){
    pF = pReal->pFile;
    rc = pF->pMethods->xWrite(pF, "\0\0\0\0", 4, pReal->nBlob-BLOCKSIZE);
    अगर( rc==SQLITE_OK ){
      pReal->nJournal = 0;
    }
  }
  वापस rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
अटल पूर्णांक fsAccess(
  sqlite3_vfs *pVfs, 
  स्थिर अक्षर *zPath, 
  पूर्णांक flags, 
  पूर्णांक *pResOut
){
  fs_vfs_t *pFsVfs = (fs_vfs_t *)pVfs;
  fs_real_file *pReal;
  पूर्णांक isJournal = 0;
  पूर्णांक nName = (पूर्णांक)म_माप(zPath);

  अगर( flags!=SQLITE_ACCESS_EXISTS ){
    sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
    वापस pParent->xAccess(pParent, zPath, flags, pResOut);
  }

  निश्चित(म_माप("-journal")==8);
  अगर( nName>8 && म_भेद("-journal", &zPath[nName-8])==0 ){
    nName -= 8;
    isJournal = 1;
  }

  pReal = pFsVfs->pFileList; 
  क्रम(; pReal && म_भेदन(pReal->zName, zPath, nName); pReal=pReal->p_अग्र);

  *pResOut = (pReal && (!isJournal || pReal->nJournal>0));
  वापस SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (FS_MAX_PATHNAME+1) bytes.
*/
अटल पूर्णांक fsFullPathname(
  sqlite3_vfs *pVfs,            /* Pointer to vfs object */
  स्थिर अक्षर *zPath,            /* Possibly relative input path */
  पूर्णांक nOut,                     /* Size of output buffer in bytes */
  अक्षर *zOut                    /* Output buffer */
){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  वापस pParent->xFullPathname(pParent, zPath, nOut, zOut);
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
अटल व्योम *fsDlOpen(sqlite3_vfs *pVfs, स्थिर अक्षर *zPath){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  वापस pParent->xDlOpen(pParent, zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
अटल व्योम fsDlError(sqlite3_vfs *pVfs, पूर्णांक nByte, अक्षर *z_Err_संदेश){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  pParent->xDlError(pParent, nByte, z_Err_संदेश);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
अटल व्योम (*fsDlSym(sqlite3_vfs *pVfs, व्योम *pH, स्थिर अक्षर *zSym))(व्योम){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  वापस pParent->xDlSym(pParent, pH, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
अटल व्योम fsDlClose(sqlite3_vfs *pVfs, व्योम *pHandle){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  pParent->xDlClose(pParent, pHandle);
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
अटल पूर्णांक fsRandomness(sqlite3_vfs *pVfs, पूर्णांक nByte, अक्षर *zBufOut){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  वापस pParent->xRandomness(pParent, nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
अटल पूर्णांक fsSleep(sqlite3_vfs *pVfs, पूर्णांक nMicro){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  वापस pParent->xSleep(pParent, nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
अटल पूर्णांक fsCurrentTime(sqlite3_vfs *pVfs, द्विगुन *pTimeOut){
  sqlite3_vfs *pParent = ((fs_vfs_t *)pVfs)->pParent;
  वापस pParent->xCurrentTime(pParent, pTimeOut);
}

/*
** This procedure registers the fs vfs with SQLite. If the argument is
** true, the fs vfs becomes the new default vfs. It is the only publicly
** available function in this file.
*/
पूर्णांक fs_register(व्योम){
  अगर( fs_vfs.pParent ) वापस SQLITE_OK;
  fs_vfs.pParent = sqlite3_vfs_find(0);
  fs_vfs.base.mxPathname = fs_vfs.pParent->mxPathname;
  fs_vfs.base.szOsFile = MAX(माप(tmp_file), माप(fs_file));
  वापस sqlite3_vfs_register(&fs_vfs.base, 0);
}

#अगर_घोषित SQLITE_TEST
  पूर्णांक SqlitetestOnefile_Init() {वापस fs_register();}
#पूर्ण_अगर
