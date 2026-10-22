/*
** 2004 May 22
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
******************************************************************************
**
** This file contains code that modified the OS layer in order to simulate
** the effect on the database file of an OS crash or power failure.  This
** is used to test the ability of SQLite to recover from those situations.
*/
#اگر SQLITE_TEST          /* This file is used for testing only */
#شامل "sqliteInt.h"
#شامل "tclsqlite.h"

#ifndef SQLITE_OMIT_DISKIO  /* This file is a no-op if disk I/O is disabled */

/* #define TRACE_CRASHTEST */

تعریفنوع ساختار CrashFile CrashFile;
تعریفنوع ساختار CrashGlobal CrashGlobal;
تعریفنوع ساختار WriteBuffer WriteBuffer;

/*
** Method:
**
**   This layer is implemented as a wrapper around the "real"
**   sqlite3_file object for the host system. Each time data is
**   written to the file object, instead of being written to the
**   underlying file, the write operation is stored in an in-memory
**   structure (type WriteBuffer). This structure is placed at the
**   end of a global ordered list (the write-list).
**
**   When data is read from a file object, the requested region is
**   first retrieved from the real file. The write-list is then
**   traversed and data copied from any overlapping WriteBuffer
**   structures to the output buffer. i.e. a read() operation following
**   one or more write() operations works as expected, even if no
**   data has actually been written out to the real file.
**
**   When a fsync() operation is performed, an operating system crash
**   may be simulated, in which case exit(-1) is called (the call to
**   xSync() never returns). Whether or not a crash is simulated,
**   the data associated with a subset of the WriteBuffer structures
**   stored in the write-list is written to the real underlying files
**   and the entries removed from the write-list. If a crash is simulated,
**   a subset of the buffers may be corrupted before the data is written.
**
**   The exact subset of the write-list written and/or corrupted is
**   determined by the simulated device characteristics and sector-size.
**
** "Normal" mode:
**
**   Normal mode is used when the simulated device has none of the
**   SQLITE_IOCAP_XXX flags set.
**
**   In normal mode, if the fsync() is not a simulated crash, the
**   write-list is traversed from beginning to end. Each WriteBuffer
**   structure associated with the file handle used to call xSync()
**   is written to the real file and removed from the write-list.
**
**   If a crash is simulated, one of the following takes place for
**   each WriteBuffer in the write-list, regardless of which
**   file-handle it is associated with:
**
**     1. The buffer is correctly written to the file, just as if
**        a crash were not being simulated.
**
**     2. Nothing is done.
**
**     3. Garbage data is written to all sectors of the file that
**        overlap the region specified by the WriteBuffer. Or garbage
**        data is written to some contiguous section within the
**        overlapped sectors.
**
** Device Characteristic flag handling:
**
**   If the IOCAP_ATOMIC flag is set, then option (3) above is
**   never selected.
**
**   If the IOCAP_ATOMIC512 flag is set, and the WriteBuffer represents
**   an aligned write() of an integer number of 512 byte regions, then
**   option (3) above is never selected. Instead, each 512 byte region
**   is either correctly written or left completely untouched. Similar
**   logic governs the behavior if any of the other ATOMICXXX flags
**   is set.
**
**   If either the IOCAP_SAFEAPPEND or IOCAP_SEQUENTIAL flags are set
**   and a crash is being simulated, then an entry of the write-list is
**   selected at random. Everything in the list after the selected entry
**   is discarded before processing begins.
**
**   If IOCAP_SEQUENTIAL is set and a crash is being simulated, option
**   (1) is selected for all write-list entries except the last. If a
**   crash is not being simulated, then all entries in the write-list
**   that occur before at least one write() on the file-handle specified
**   as part of the xSync() are written to their associated real files.
**
**   If IOCAP_SAFEAPPEND is set and the first byte written by the write()
**   operation is one byte past the current end of the file, then option
**   (1) is always selected.
*/

/*
** Each write operation in the write-list is represented by an instance
** of the following structure.
**
** If zBuf is 0, then this structure represents a call to xTruncate(),
** not xWrite(). In that case, iOffset is the size that the file is
** truncated to.
*/
ساختار WriteBuffer {
  i64 iOffset;                 /* Byte offset of the start of this write() */
  صحیح nBuf;                    /* Number of bytes written */
  u8 *zBuf;                    /* Pointer to copy of written data */
  CrashFile *pFile;            /* File this write() applies to */

  WriteBuffer *pNext;          /* Next in CrashGlobal.pWriteList */
};

ساختار CrashFile {
  ثابت sqlite3_io_methods *pMethod;   /* Must be first */
  sqlite3_file *pRealFile;             /* Underlying "real" file handle */
  نویسه *zName;
  صحیح flags;                           /* Flags the file was opened with */

  /* Cache of the entire file. This is used to speed up OsRead() and
  ** OsFileSize() calls. Although both could be done by traversing the
  ** write-list, in practice this is impractically slow.
  */
  u8 *zData;                           /* Buffer containing file contents */
  صحیح nData;                           /* Size of buffer allocated at zData */
  i64 iSize;                           /* Size of file in bytes */
};

ساختار CrashGlobal {
  WriteBuffer *pWriteList;     /* Head of write-list */
  WriteBuffer *pWriteListEnd;  /* End of write-list */

  صحیح iSectorSize;             /* Value of simulated sector size */
  صحیح iDeviceCharacteristics;  /* Value of simulated device characteristics */

  صحیح iCrash;                  /* Crash on the iCrash'th call to xSync() */
  نویسه zCrashFile[500];        /* Crash during an xSync() on this file */
};

ایستا CrashGlobal g = {0, 0, SQLITE_DEFAULT_SECTOR_SIZE, 0, 0};

/*
** Set this global variable to 1 to enable crash testing.
*/
ایستا صحیح sqlite3CrashTestEnable = 0;

ایستا تهی *crash_malloc(صحیح nByte){
  بازگشت (تهی *)Tcl_AttemptAlloc((size_t)nByte);
}
ایستا تهی crash_free(تهی *p){
  Tcl_Free(p);
}
ایستا تهی *crash_realloc(تهی *p, صحیح n){
  بازگشت (تهی *)Tcl_AttemptRealloc(p, (size_t)n);
}

/*
** Wrapper around the sqlite3OsWrite() function that avoids writing to the
** 512 byte block beginning at offset PENDING_BYTE.
*/
ایستا صحیح writeDbFile(CrashFile *p, u8 *z, i64 iAmt, i64 iOff){
  صحیح rc = SQLITE_OK;
  صحیح iSkip = 0;
  اگر( (iAmt-iSkip)>0 ){
    rc = sqlite3OsWrite(p->pRealFile, &z[iSkip], (صحیح)(iAmt-iSkip), iOff+iSkip);
  }
  بازگشت rc;
}

/*
** Flush the write-list as if xSync() had been called on file handle
** pFile. If isCrash is true, simulate a crash.
*/
ایستا صحیح writeListSync(CrashFile *pFile, صحیح isCrash){
  صحیح rc = SQLITE_OK;
  صحیح iDc = g.iDeviceCharacteristics;

  WriteBuffer *pWrite;
  WriteBuffer **ppPtr;

  /* If this is not a crash simulation, set pFinal to point to the
  ** last element of the write-list that is associated with file handle
  ** pFile.
  **
  ** If this is a crash simulation, set pFinal to an arbitrarily selected
  ** element of the write-list.
  */
  WriteBuffer *pFinal = 0;
  اگر( !isCrash ){
    برای(pWrite=g.pWriteList; pWrite; pWrite=pWrite->pNext){
      اگر( pWrite->pFile==pFile ){
        pFinal = pWrite;
      }
    }
  }وگرنه اگر( iDc&(SQLITE_IOCAP_SEQUENTIAL|SQLITE_IOCAP_SAFE_APPEND) ){
    صحیح nWrite = 0;
    صحیح iFinal;
    برای(pWrite=g.pWriteList; pWrite; pWrite=pWrite->pNext) nWrite++;
    sqlite3_randomness(اندازه(صحیح), &iFinal);
    iFinal = ((iFinal<0)?-1*iFinal:iFinal)%nWrite;
    برای(pWrite=g.pWriteList; iFinal>0; pWrite=pWrite->pNext) iFinal--;
    pFinal = pWrite;
  }

#ifdef TRACE_CRASHTEST
  اگر( pFile ){
    printf("Sync %s (is %s crash)\n", pFile->zName, (isCrash?"a":"not a"));
  }
#endif

  ppPtr = &g.pWriteList;
  برای(pWrite=*ppPtr; rc==SQLITE_OK && pWrite; pWrite=*ppPtr){
    sqlite3_file *pRealFile = pWrite->pFile->pRealFile;

    /* (eAction==1)      -> write block out normally,
    ** (eAction==2)      -> do nothing,
    ** (eAction==3)      -> trash sectors.
    */
    صحیح eAction = 0;
    اگر( !isCrash ){
      eAction = 2;
      اگر( (pWrite->pFile==pFile || iDc&SQLITE_IOCAP_SEQUENTIAL) ){
        eAction = 1;
      }
    }وگرنه{
      نویسه random;
      sqlite3_randomness(1, &random);

      /* Do not select option 3 (sector trashing) if the IOCAP_ATOMIC flag
      ** is set or this is an OsTruncate(), not an Oswrite().
      */
      اگر( (iDc&SQLITE_IOCAP_ATOMIC) || (pWrite->zBuf==0) ){
        random &= 0x01;
      }

      /* If IOCAP_SEQUENTIAL is set and this is not the final entry
      ** in the truncated write-list, always select option 1 (write
      ** out correctly).
      */
      اگر( (iDc&SQLITE_IOCAP_SEQUENTIAL && pWrite!=pFinal) ){
        random = 0;
      }

      /* If IOCAP_SAFE_APPEND is set and this OsWrite() operation is
      ** an append (first byte of the written region is 1 byte past the
      ** current EOF), always select option 1 (write out correctly).
      */
      اگر( iDc&SQLITE_IOCAP_SAFE_APPEND && pWrite->zBuf ){
        i64 iSize;
        sqlite3OsFileSize(pRealFile, &iSize);
        اگر( iSize==pWrite->iOffset ){
          random = 0;
        }
      }

      اگر( (random&0x06)==0x06 ){
        eAction = 3;
      }وگرنه{
        eAction = ((random&0x01)?2:1);
      }
    }

    انتخاب( eAction ){
      حالت 1: {               /* Write out correctly */
        اگر( pWrite->zBuf ){
          rc = writeDbFile(
              pWrite->pFile, pWrite->zBuf, pWrite->nBuf, pWrite->iOffset
          );
        }وگرنه{
          rc = sqlite3OsTruncate(pRealFile, pWrite->iOffset);
        }
        *ppPtr = pWrite->pNext;
#ifdef TRACE_CRASHTEST
        اگر( isCrash ){
          printf("Writing %d bytes @ %d (%s)\n",
            pWrite->nBuf, (صحیح)pWrite->iOffset, pWrite->pFile->zName
          );
        }
#endif
        crash_free(pWrite);
        بشکن;
      }
      حالت 2: {               /* Do nothing */
        ppPtr = &pWrite->pNext;
#ifdef TRACE_CRASHTEST
        اگر( isCrash ){
          printf("Omiting %d bytes @ %d (%s)\n",
            pWrite->nBuf, (صحیح)pWrite->iOffset, pWrite->pFile->zName
          );
        }
#endif
        بشکن;
      }
      حالت 3: {               /* Trash sectors */
        u8 *zGarbage;
        صحیح iFirst = (صحیح)(pWrite->iOffset/g.iSectorSize);
        صحیح iLast = (صحیح)((pWrite->iOffset+pWrite->nBuf-1)/g.iSectorSize);

        أكد(pWrite->zBuf);

#ifdef TRACE_CRASHTEST
        printf("Trashing %d sectors (%d bytes) @ %lld (sector %d) (%s)\n",
            1+iLast-iFirst, (1+iLast-iFirst)*g.iSectorSize,
            pWrite->iOffset, iFirst, pWrite->pFile->zName
        );
#endif

        zGarbage = crash_malloc(g.iSectorSize);
        اگر( zGarbage ){
          sqlite3_int64 i;
          برای(i=iFirst; rc==SQLITE_OK && i<=iLast; i++){
            sqlite3_randomness(g.iSectorSize, zGarbage);
            rc = writeDbFile(
              pWrite->pFile, zGarbage, g.iSectorSize, i*g.iSectorSize
            );
          }
          crash_free(zGarbage);
        }وگرنه{
          rc = SQLITE_NOMEM;
        }

        ppPtr = &pWrite->pNext;
        بشکن;
      }

      پیشفرض:
        أكد(!"Cannot happen");
    }

    اگر( pWrite==pFinal ) بشکن;
  }

  اگر( rc==SQLITE_OK && isCrash ){
    exit(-1);
  }

  برای(pWrite=g.pWriteList; pWrite && pWrite->pNext; pWrite=pWrite->pNext);
  g.pWriteListEnd = pWrite;

  بازگشت rc;
}

/*
** Add an entry to the end of the write-list.
*/
ایستا صحیح writeListAppend(
  sqlite3_file *pFile,
  sqlite3_int64 iOffset,
  ثابت u8 *zBuf,
  صحیح nBuf
){
  WriteBuffer *pNew;

  أكد((zBuf && nBuf) || (!nBuf && !zBuf));

  pNew = (WriteBuffer *)crash_malloc(اندازه(WriteBuffer) + nBuf);
  اگر( pNew==0 ){
    fprintf(stderr, "out of memory in the crash simulator\n");
  }
  memset(pNew, 0, اندازه(WriteBuffer)+nBuf);
  pNew->iOffset = iOffset;
  pNew->nBuf = nBuf;
  pNew->pFile = (CrashFile *)pFile;
  اگر( zBuf ){
    pNew->zBuf = (u8 *)&pNew[1];
    memcpy(pNew->zBuf, zBuf, nBuf);
  }

  اگر( g.pWriteList ){
    أكد(g.pWriteListEnd);
    g.pWriteListEnd->pNext = pNew;
  }وگرنه{
    g.pWriteList = pNew;
  }
  g.pWriteListEnd = pNew;
 
  بازگشت SQLITE_OK;
}

/*
** Close a crash-file.
*/
ایستا صحیح cfClose(sqlite3_file *pFile){
  CrashFile *pCrash = (CrashFile *)pFile;
  writeListSync(pCrash, 0);
  sqlite3OsClose(pCrash->pRealFile);
  بازگشت SQLITE_OK;
}

/*
** Read data from a crash-file.
*/
ایستا صحیح cfRead(
  sqlite3_file *pFile,
  تهی *zBuf,
  صحیح iAmt,
  sqlite_int64 iOfst
){
  CrashFile *pCrash = (CrashFile *)pFile;
  صحیح nCopy = (صحیح)MIN((i64)iAmt, (pCrash->iSize - iOfst));

  اگر( nCopy>0 ){
    memcpy(zBuf, &pCrash->zData[iOfst], nCopy);
  }

  /* Check the file-size to see if this is a short-read */
  اگر( nCopy<iAmt ){
    بازگشت SQLITE_IOERR_SHORT_READ;
  }

  بازگشت SQLITE_OK;
}

/*
** Write data to a crash-file.
*/
ایستا صحیح cfWrite(
  sqlite3_file *pFile,
  ثابت تهی *zBuf,
  صحیح iAmt,
  sqlite_int64 iOfst
){
  CrashFile *pCrash = (CrashFile *)pFile;
  اگر( iAmt+iOfst>pCrash->iSize ){
    pCrash->iSize = (صحیح)(iAmt+iOfst);
  }
  وقتی( pCrash->iSize>pCrash->nData ){
    u8 *zNew;
    صحیح nNew = (pCrash->nData*2) + 4096;
    zNew = crash_realloc(pCrash->zData, nNew);
    اگر( !zNew ){
      بازگشت SQLITE_NOMEM;
    }
    memset(&zNew[pCrash->nData], 0, nNew-pCrash->nData);
    pCrash->nData = nNew;
    pCrash->zData = zNew;
  }
  memcpy(&pCrash->zData[iOfst], zBuf, iAmt);
  بازگشت writeListAppend(pFile, iOfst, zBuf, iAmt);
}

/*
** Truncate a crash-file.
*/
ایستا صحیح cfTruncate(sqlite3_file *pFile, sqlite_int64 size){
  CrashFile *pCrash = (CrashFile *)pFile;
  أكد(size>=0);
  اگر( pCrash->iSize>size ){
    pCrash->iSize = (صحیح)size;
  }
  بازگشت writeListAppend(pFile, size, 0, 0);
}

/*
** Sync a crash-file.
*/
ایستا صحیح cfSync(sqlite3_file *pFile, صحیح flags){
  CrashFile *pCrash = (CrashFile *)pFile;
  صحیح isCrash = 0;

  ثابت نویسه *zName = pCrash->zName;
  ثابت نویسه *zCrashFile = g.zCrashFile;
  صحیح nName = (صحیح)strlen(zName);
  صحیح nCrashFile = (صحیح)strlen(zCrashFile);

  اگر( nCrashFile>0 && zCrashFile[nCrashFile-1]=='*' ){
    nCrashFile--;
    اگر( nName>nCrashFile ) nName = nCrashFile;
  }

#ifdef TRACE_CRASHTEST
  printf("cfSync(): nName = %d, nCrashFile = %d, zName = %s, zCrashFile = %s\n",
         nName, nCrashFile, zName, zCrashFile);
#endif

  اگر( nName==nCrashFile && 0==memcmp(zName, zCrashFile, nName) ){
#ifdef TRACE_CRASHTEST
    printf("cfSync(): name matched, g.iCrash = %d\n", g.iCrash);
#endif
    اگر( (--g.iCrash)==0 ) isCrash = 1;
  }

  بازگشت writeListSync(pCrash, isCrash);
}

/*
** Return the current file-size of the crash-file.
*/
ایستا صحیح cfFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  CrashFile *pCrash = (CrashFile *)pFile;
  *pSize = (i64)pCrash->iSize;
  بازگشت SQLITE_OK;
}

/*
** Calls related to file-locks are passed on to the real file handle.
*/
ایستا صحیح cfLock(sqlite3_file *pFile, صحیح eLock){
  بازگشت sqlite3OsLock(((CrashFile *)pFile)->pRealFile, eLock);
}
ایستا صحیح cfUnlock(sqlite3_file *pFile, صحیح eLock){
  بازگشت sqlite3OsUnlock(((CrashFile *)pFile)->pRealFile, eLock);
}
ایستا صحیح cfCheckReservedLock(sqlite3_file *pFile, صحیح *pResOut){
  بازگشت sqlite3OsCheckReservedLock(((CrashFile *)pFile)->pRealFile, pResOut);
}
ایستا صحیح cfFileControl(sqlite3_file *pFile, صحیح op, تهی *pArg){
  اگر( op==SQLITE_FCNTL_SIZE_HINT ){
    CrashFile *pCrash = (CrashFile *)pFile;
    i64 nByte = *(i64 *)pArg;
    اگر( nByte>pCrash->iSize ){
      اگر( SQLITE_OK==writeListAppend(pFile, nByte, 0, 0) ){
        pCrash->iSize = (صحیح)nByte;
      }
    }
    بازگشت SQLITE_OK;
  }
  بازگشت sqlite3OsFileControl(((CrashFile *)pFile)->pRealFile, op, pArg);
}

/*
** The xSectorSize() and xDeviceCharacteristics() functions return
** the global values configured by the [sqlite_crashparams] tcl
*  interface.
*/
ایستا صحیح cfSectorSize(sqlite3_file *pFile){
  بازگشت g.iSectorSize;
}
ایستا صحیح cfDeviceCharacteristics(sqlite3_file *pFile){
  بازگشت g.iDeviceCharacteristics;
}

/*
** Pass-throughs for WAL support.
*/
ایستا صحیح cfShmLock(sqlite3_file *pFile, صحیح ofst, صحیح n, صحیح flags){
  sqlite3_file *pReal = ((CrashFile*)pFile)->pRealFile;
  بازگشت pReal->pMethods->xShmLock(pReal, ofst, n, flags);
}
ایستا تهی cfShmBarrier(sqlite3_file *pFile){
  sqlite3_file *pReal = ((CrashFile*)pFile)->pRealFile;
  pReal->pMethods->xShmBarrier(pReal);
}
ایستا صحیح cfShmUnmap(sqlite3_file *pFile, صحیح delFlag){
  sqlite3_file *pReal = ((CrashFile*)pFile)->pRealFile;
  بازگشت pReal->pMethods->xShmUnmap(pReal, delFlag);
}
ایستا صحیح cfShmMap(
  sqlite3_file *pFile,            /* Handle open on database file */
  صحیح iRegion,                    /* Region to retrieve */
  صحیح sz,                         /* Size of regions */
  صحیح w,                          /* True to extend file if necessary */
  تهی فرار **pp              /* OUT: Mapped memory */
){
  sqlite3_file *pReal = ((CrashFile*)pFile)->pRealFile;
  بازگشت pReal->pMethods->xShmMap(pReal, iRegion, sz, w, pp);
}

ایستا ثابت sqlite3_io_methods CrashFileVtab = {
  2,                            /* iVersion */
  cfClose,                      /* xClose */
  cfRead,                       /* xRead */
  cfWrite,                      /* xWrite */
  cfTruncate,                   /* xTruncate */
  cfSync,                       /* xSync */
  cfFileSize,                   /* xFileSize */
  cfLock,                       /* xLock */
  cfUnlock,                     /* xUnlock */
  cfCheckReservedLock,          /* xCheckReservedLock */
  cfFileControl,                /* xFileControl */
  cfSectorSize,                 /* xSectorSize */
  cfDeviceCharacteristics,      /* xDeviceCharacteristics */
  cfShmMap,                     /* xShmMap */
  cfShmLock,                    /* xShmLock */
  cfShmBarrier,                 /* xShmBarrier */
  cfShmUnmap                    /* xShmUnmap */
};

/*
** Application data for the crash VFS
*/
ساختار crashAppData {
  sqlite3_vfs *pOrig;                   /* Wrapped vfs structure */
};

/*
** Open a crash-file file handle.
**
** The caller will have allocated pVfs->szOsFile bytes of space
** at pFile. This file uses this space for the CrashFile structure
** and allocates space for the "real" file structure using
** sqlite3_malloc(). The assumption here is (pVfs->szOsFile) is
** equal or greater than sizeof(CrashFile).
*/
ایستا صحیح cfOpen(
  sqlite3_vfs *pCfVfs,
  ثابت نویسه *zName,
  sqlite3_file *pFile,
  صحیح flags,
  صحیح *pOutFlags
){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  صحیح rc;
  CrashFile *pWrapper = (CrashFile *)pFile;
  sqlite3_file *pReal = (sqlite3_file*)&pWrapper[1];

  memset(pWrapper, 0, اندازه(CrashFile));
  rc = sqlite3OsOpen(pVfs, zName, pReal, flags, pOutFlags);

  اگر( rc==SQLITE_OK ){
    i64 iSize;
    pWrapper->pMethod = &CrashFileVtab;
    pWrapper->zName = (نویسه *)zName;
    pWrapper->pRealFile = pReal;
    rc = sqlite3OsFileSize(pReal, &iSize);
    pWrapper->iSize = (صحیح)iSize;
    pWrapper->flags = flags;
  }
  اگر( rc==SQLITE_OK ){
    pWrapper->nData = (صحیح)(4096 + pWrapper->iSize);
    pWrapper->zData = crash_malloc(pWrapper->nData);
    اگر( pWrapper->zData ){
      /* os_unix.c contains an assert() that fails if the caller attempts
      ** to read data from the 512-byte locking region of a file opened
      ** with the SQLITE_OPEN_MAIN_DB flag. This region of a database file
      ** never contains valid data anyhow. So avoid doing such a read here.
      **
      ** UPDATE: It also contains an assert() verifying that each call
      ** to the xRead() method reads less than 128KB of data.
      */
      i64 iOff;

      memset(pWrapper->zData, 0, pWrapper->nData);
      برای(iOff=0; iOff<pWrapper->iSize; iOff += 512){
        صحیح nRead = (صحیح)(pWrapper->iSize - iOff);
        اگر( nRead>512 ) nRead = 512;
        rc = sqlite3OsRead(pReal, &pWrapper->zData[iOff], nRead, iOff);
      }
    }وگرنه{
      rc = SQLITE_NOMEM;
    }
  }
  اگر( rc!=SQLITE_OK && pWrapper->pMethod ){
    sqlite3OsClose(pFile);
  }
  بازگشت rc;
}

ایستا صحیح cfDelete(sqlite3_vfs *pCfVfs, ثابت نویسه *zPath, صحیح dirSync){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  بازگشت pVfs->xDelete(pVfs, zPath, dirSync);
}
ایستا صحیح cfAccess(
  sqlite3_vfs *pCfVfs,
  ثابت نویسه *zPath,
  صحیح flags,
  صحیح *pResOut
){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  بازگشت pVfs->xAccess(pVfs, zPath, flags, pResOut);
}
ایستا صحیح cfFullPathname(
  sqlite3_vfs *pCfVfs,
  ثابت نویسه *zPath,
  صحیح nPathOut,
  نویسه *zPathOut
){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  بازگشت pVfs->xFullPathname(pVfs, zPath, nPathOut, zPathOut);
}
ایستا تهی *cfDlOpen(sqlite3_vfs *pCfVfs, ثابت نویسه *zPath){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  بازگشت pVfs->xDlOpen(pVfs, zPath);
}
ایستا تهی cfDlError(sqlite3_vfs *pCfVfs, صحیح nByte, نویسه *z_Err_پیام){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  pVfs->xDlError(pVfs, nByte, z_Err_پیام);
}
ایستا تهی (*cfDlSym(sqlite3_vfs *pCfVfs, تهی *pH, ثابت نویسه *zSym))(تهی){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  بازگشت pVfs->xDlSym(pVfs, pH, zSym);
}
ایستا تهی cfDlClose(sqlite3_vfs *pCfVfs, تهی *pHandle){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  pVfs->xDlClose(pVfs, pHandle);
}
ایستا صحیح cfRandomness(sqlite3_vfs *pCfVfs, صحیح nByte, نویسه *zBufOut){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  بازگشت pVfs->xRandomness(pVfs, nByte, zBufOut);
}
ایستا صحیح cfSleep(sqlite3_vfs *pCfVfs, صحیح nMicro){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  بازگشت pVfs->xSleep(pVfs, nMicro);
}
ایستا صحیح cfCurrentTime(sqlite3_vfs *pCfVfs, دوگانه *pTimeOut){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  بازگشت pVfs->xCurrentTime(pVfs, pTimeOut);
}
ایستا صحیح cfGetLastError(sqlite3_vfs *pCfVfs, صحیح n, نویسه *z){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  بازگشت pVfs->xGetLastError(pVfs, n, z);
}

ایستا صحیح processDevSymArgs(
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[],
  صحیح *piDeviceChar,
  صحیح *piSectorSize
){
  ساختار DeviceFlag {
    نویسه *zName;
    صحیح i_مقدار;
  } aFlag[] = {
    { "atomic",              SQLITE_IOCAP_ATOMIC                },
    { "atomic512",           SQLITE_IOCAP_ATOMIC512             },
    { "atomic1k",            SQLITE_IOCAP_ATOMIC1K              },
    { "atomic2k",            SQLITE_IOCAP_ATOMIC2K              },
    { "atomic4k",            SQLITE_IOCAP_ATOMIC4K              },
    { "atomic8k",            SQLITE_IOCAP_ATOMIC8K              },
    { "atomic16k",           SQLITE_IOCAP_ATOMIC16K             },
    { "atomic32k",           SQLITE_IOCAP_ATOMIC32K             },
    { "atomic64k",           SQLITE_IOCAP_ATOMIC64K             },
    { "sequential",          SQLITE_IOCAP_SEQUENTIAL            },
    { "safe_append",         SQLITE_IOCAP_SAFE_APPEND           },
    { "powersafe_overwrite", SQLITE_IOCAP_POWERSAFE_OVERWRITE   },
    { "batch-atomic",        SQLITE_IOCAP_BATCH_ATOMIC          },
    { 0, 0 }
  };

  صحیح i;
  صحیح iDc = 0;
  صحیح iSectorSize = 0;
  صحیح setSectorsize = 0;
  صحیح setDeviceChar = 0;

  برای(i=0; i<objc; i+=2){
    Tcl_Size nOpt;
    نویسه *zOpt = Tcl_GetStringFromObj(objv[i], &nOpt);

    اگر( (nOpt>11 || nOpt<2 || strncmp("-sectorsize", zOpt, nOpt))
     && (nOpt>16 || nOpt<2 || strncmp("-characteristics", zOpt, nOpt))
    ){
      Tcl_AppendResult(interp,
        "Bad option: \"", zOpt,
        "\" - must be \"-characteristics\" or \"-sectorsize\"", پوچ
      );
      بازگشت TCL_ERROR;
    }
    اگر( i==objc-1 ){
      Tcl_AppendResult(interp, "Option requires an argument: \"", zOpt, "\"", پوچ);
      بازگشت TCL_ERROR;
    }

    اگر( zOpt[1]=='s' ){
      اگر( Tcl_GetIntFromObj(interp, objv[i+1], &iSectorSize) ){
        بازگشت TCL_ERROR;
      }
      setSectorsize = 1;
    }وگرنه{
      صحیح j;
      Tcl_Obj **apObj;
      Tcl_Size nObj;
      اگر( Tcl_ListObjGetElements(interp, objv[i+1], &nObj, &apObj) ){
        بازگشت TCL_ERROR;
      }
      برای(j=0; j<(صحیح)nObj; j++){
        صحیح rc;
        صحیح iChoice;
        Tcl_Obj *pFlag = Tcl_DuplicateObj(apObj[j]);
        Tcl_Incr_Ref_شمار(pFlag);
        Tcl_UtfToLower(Tcl_GetString(pFlag));

        rc = Tcl_GetIndexFromObjStruct(
            interp, pFlag, aFlag, اندازه(aFlag[0]), "no such flag", 0, &iChoice
        );
        Tcl_Decr_Ref_شمار(pFlag);
        اگر( rc ){
          بازگشت TCL_ERROR;
        }

        iDc |= aFlag[iChoice].i_مقدار;
      }
      setDeviceChar = 1;
    }
  }

  اگر( setDeviceChar ){
    *piDeviceChar = iDc;
  }
  اگر( setSectorsize ){
    *piSectorSize = iSectorSize;
  }

  بازگشت TCL_OK;
}

/*
** tclcmd:   sqlite3_crash_now
**
** Simulate a crash immediately. This function does not return
** (writeListSync() calls exit(-1)).
*/
ایستا صحیح SQLITE_TCLAPI crashNowCmd(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  اگر( objc!=1 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "");
    بازگشت TCL_ERROR;
  }
  writeListSync(0, 1);
  أكد( 0 );
  بازگشت TCL_OK;
}

/*
** tclcmd:   sqlite_crash_enable ENABLE ?DEFAULT?
**
** Parameter ENABLE must be a boolean value. If true, then the "crash"
** vfs is added to the system. If false, it is removed.
*/
ایستا صحیح SQLITE_TCLAPI crashEnableCmd(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح isEnable;
  صحیح isDefault = 0;
  ایستا sqlite3_vfs crashVfs = {
    2,                  /* iVersion */
    0,                  /* szOsFile */
    0,                  /* mxPathname */
    0,                  /* pNext */
    "crash",            /* zName */
    0,                  /* pAppData */
 
    cfOpen,               /* xOpen */
    cfDelete,             /* xDelete */
    cfAccess,             /* xAccess */
    cfFullPathname,       /* xFullPathname */
    cfDlOpen,             /* xDlOpen */
    cfDlError,            /* xDlError */
    cfDlSym,              /* xDlSym */
    cfDlClose,            /* xDlClose */
    cfRandomness,         /* xRandomness */
    cfSleep,              /* xSleep */
    cfCurrentTime,        /* xCurrentTime */
    cfGetLastError,       /* xGetLastError */
    0,                    /* xCurrentTimeInt64 */
  };

  اگر( objc!=2 && objc!=3 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "ENABLE ?DEFAULT?");
    بازگشت TCL_ERROR;
  }

  اگر( Tcl_GetBooleanFromObj(interp, objv[1], &isEnable) ){
    بازگشت TCL_ERROR;
  }
  اگر( objc==3 && Tcl_GetBooleanFromObj(interp, objv[2], &isDefault) ){
    بازگشت TCL_ERROR;
  }

  اگر( (isEnable && crashVfs.pAppData) || (!isEnable && !crashVfs.pAppData) ){
    بازگشت TCL_OK;
  }

  اگر( crashVfs.pAppData==0 ){
    sqlite3_vfs *pOriginalVfs = sqlite3_vfs_find(0);
    crashVfs.mxPathname = pOriginalVfs->mxPathname;
    crashVfs.pAppData = (تهی *)pOriginalVfs;
    crashVfs.szOsFile = اندازه(CrashFile) + pOriginalVfs->szOsFile;
    sqlite3_vfs_register(&crashVfs, isDefault);
  }وگرنه{
    crashVfs.pAppData = 0;
    sqlite3_vfs_unregister(&crashVfs);
  }

  بازگشت TCL_OK;
}

/*
** tclcmd:   sqlite_crashparams ?OPTIONS? DELAY CRASHFILE
**
** This procedure implements a TCL command that enables crash testing
** in testfixture.  Once enabled, crash testing cannot be disabled.
**
** Available options are "-characteristics" and "-sectorsize". Both require
** an argument. For -sectorsize, this is the simulated sector size in
** bytes. For -characteristics, the argument must be a list of io-capability
** flags to simulate. Valid flags are "atomic", "atomic512", "atomic1K",
** "atomic2K", "atomic4K", "atomic8K", "atomic16K", "atomic32K",
** "atomic64K", "sequential" and "safe_append".
**
** Example:
**
**   sqlite_crashparams -sect 1024 -char {atomic sequential} ./test.db 1
**
*/
ایستا صحیح SQLITE_TCLAPI crashParamsObjCmd(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح iDelay;
  ثابت نویسه *zCrashFile;
  Tcl_Size nCrashFile;
  صحیح iDc, iSectorSize;

  iDc = -1;
  iSectorSize = -1;

  اگر( objc<3 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "?OPTIONS? DELAY CRASHFILE");
    برو error;
  }

  zCrashFile = Tcl_GetStringFromObj(objv[objc-1], &nCrashFile);
  اگر( nCrashFile>=اندازه(g.zCrashFile) ){
    Tcl_AppendResult(interp, "Filename is too long: \"", zCrashFile, "\"", پوچ);
    برو error;
  }
  اگر( Tcl_GetIntFromObj(interp, objv[objc-2], &iDelay) ){
    برو error;
  }

  اگر( processDevSymArgs(interp, objc-3, &objv[1], &iDc, &iSectorSize) ){
    بازگشت TCL_ERROR;
  }

  اگر( iDc>=0 ){
    g.iDeviceCharacteristics = iDc;
  }
  اگر( iSectorSize>=0 ){
    g.iSectorSize = iSectorSize;
  }

  g.iCrash = iDelay;
  memcpy(g.zCrashFile, zCrashFile, nCrashFile+1);
  sqlite3CrashTestEnable = 1;
  بازگشت TCL_OK;

error:
  بازگشت TCL_ERROR;
}

ایستا صحیح SQLITE_TCLAPI devSymObjCmd(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  تهی devsym_register(صحیح iDeviceChar, صحیح iSectorSize);

  صحیح iDc = -1;
  صحیح iSectorSize = -1;

  اگر( processDevSymArgs(interp, objc-1, &objv[1], &iDc, &iSectorSize) ){
    بازگشت TCL_ERROR;
  }
  devsym_register(iDc, iSectorSize);

  بازگشت TCL_OK;
}

/*
** tclcmd: sqlite3_crash_on_write N
*/
ایستا صحیح SQLITE_TCLAPI writeCrashObjCmd(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  تهی devsym_crash_on_write(صحیح);
  صحیح nWrite = 0;

  اگر( objc!=2 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "NWRITE");
    بازگشت TCL_ERROR;
  }
  اگر( Tcl_GetIntFromObj(interp, objv[1], &nWrite) ){
    بازگشت TCL_ERROR;
  }

  devsym_crash_on_write(nWrite);
  بازگشت TCL_OK;
}

/*
** tclcmd: unregister_devsim
*/
ایستا صحیح SQLITE_TCLAPI dsUnregisterObjCmd(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  تهی devsym_unregister(تهی);

  اگر( objc!=1 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "");
    بازگشت TCL_ERROR;
  }

  devsym_unregister();
  بازگشت TCL_OK;
}

/*
** tclcmd: register_jt_vfs ?-default? PARENT-VFS
*/
ایستا صحیح SQLITE_TCLAPI jtObjCmd(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح jt_register(نویسه *, صحیح);
  نویسه *zParent = 0;

  اگر( objc!=2 && objc!=3 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "?-default? PARENT-VFS");
    بازگشت TCL_ERROR;
  }
  zParent = Tcl_GetString(objv[1]);
  اگر( objc==3 ){
    اگر( strcmp(zParent, "-default") ){
      Tcl_AppendResult(interp,
          "bad option \"", zParent, "\": must be -default", پوچ
      );
      بازگشت TCL_ERROR;
    }
    zParent = Tcl_GetString(objv[2]);
  }

  اگر( !(*zParent) ){
    zParent = 0;
  }
  اگر( jt_register(zParent, objc==3) ){
    Tcl_AppendResult(interp, "Error in jt_register", پوچ);
    بازگشت TCL_ERROR;
  }

  بازگشت TCL_OK;
}

/*
** tclcmd: unregister_jt_vfs
*/
ایستا صحیح SQLITE_TCLAPI jtUnregisterObjCmd(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  تهی jt_unregister(تهی);

  اگر( objc!=1 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "");
    بازگشت TCL_ERROR;
  }

  jt_unregister();
  بازگشت TCL_OK;
}

#endif /* SQLITE_OMIT_DISKIO */

/*
** This procedure registers the TCL procedures defined in this file.
*/
صحیح Sqlitetest6_Init(Tcl_Interp *interp){
#ifndef SQLITE_OMIT_DISKIO
  Tcl_CreateObjCommand(interp, "sqlite3_crash_enable", crashEnableCmd, 0, 0);
  Tcl_CreateObjCommand(interp, "sqlite3_crashparams", crashParamsObjCmd, 0, 0);
  Tcl_CreateObjCommand(interp, "sqlite3_crash_now", crashNowCmd, 0, 0);
  Tcl_CreateObjCommand(interp, "sqlite3_simulate_device", devSymObjCmd, 0, 0);
  Tcl_CreateObjCommand(interp, "sqlite3_crash_on_write", writeCrashObjCmd,0,0);
  Tcl_CreateObjCommand(interp, "unregister_devsim", dsUnregisterObjCmd, 0, 0);
  Tcl_CreateObjCommand(interp, "register_jt_vfs", jtObjCmd, 0, 0);
  Tcl_CreateObjCommand(interp, "unregister_jt_vfs", jtUnregisterObjCmd, 0, 0);
#endif
  بازگشت TCL_OK;
}

#endif /* SQLITE_TEST */
