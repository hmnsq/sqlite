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
#إذا SQLITE_TEST          /* This file is used for testing only */
#ضمن "sqliteInt.h"
#ضمن "tclsqlite.h"

#ifndef SQLITE_OMIT_DISKIO  /* This file is a no-op if disk I/O is disabled */

/* #define TRACE_CRASHTEST */

تعريفنوع بنية CrashFile CrashFile;
تعريفنوع بنية CrashGlobal CrashGlobal;
تعريفنوع بنية WriteBuffer WriteBuffer;

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
بنية WriteBuffer {
  i64 iOffset;                 /* Byte offset of the start of this write() */
  صحيح nBuf;                    /* Number of bytes written */
  u8 *zBuf;                    /* Pointer to copy of written data */
  CrashFile *pFile;            /* File this write() applies to */

  WriteBuffer *pNext;          /* Next in CrashGlobal.pWriteList */
};

بنية CrashFile {
  ثابت sqlite3_io_methods *pMethod;   /* Must be first */
  sqlite3_file *pRealFile;             /* Underlying "real" file handle */
  حرف *zName;
  صحيح flags;                           /* Flags the file was opened with */

  /* Cache of the entire file. This is used to speed up OsRead() and
  ** OsFileSize() calls. Although both could be done by traversing the
  ** write-list, in practice this is impractically slow.
  */
  u8 *zData;                           /* Buffer containing file contents */
  صحيح nData;                           /* Size of buffer allocated at zData */
  i64 iSize;                           /* Size of file in bytes */
};

بنية CrashGlobal {
  WriteBuffer *pWriteList;     /* Head of write-list */
  WriteBuffer *pWriteListEnd;  /* End of write-list */

  صحيح iSectorSize;             /* Value of simulated sector size */
  صحيح iDeviceCharacteristics;  /* Value of simulated device characteristics */

  صحيح iCrash;                  /* Crash on the iCrash'th call to xSync() */
  حرف zCrashFile[500];        /* Crash during an xSync() on this file */
};

ساكن CrashGlobal g = {0, 0, SQLITE_DEFAULT_SECTOR_SIZE, 0, 0};

/*
** Set this global variable to 1 to enable crash testing.
*/
ساكن صحيح sqlite3CrashTestEnable = 0;

ساكن فراغ *crash_malloc(صحيح nByte){
  أرجع (فراغ *)Tcl_AttemptAlloc((size_t)nByte);
}
ساكن فراغ crash_free(فراغ *p){
  Tcl_Free(p);
}
ساكن فراغ *crash_realloc(فراغ *p, صحيح n){
  أرجع (فراغ *)Tcl_AttemptRealloc(p, (size_t)n);
}

/*
** Wrapper around the sqlite3OsWrite() function that avoids writing to the
** 512 byte block beginning at offset PENDING_BYTE.
*/
ساكن صحيح writeDbFile(CrashFile *p, u8 *z, i64 iAmt, i64 iOff){
  صحيح rc = SQLITE_OK;
  صحيح iSkip = 0;
  إذا( (iAmt-iSkip)>0 ){
    rc = sqlite3OsWrite(p->pRealFile, &z[iSkip], (صحيح)(iAmt-iSkip), iOff+iSkip);
  }
  أرجع rc;
}

/*
** Flush the write-list as if xSync() had been called on file handle
** pFile. If isCrash is true, simulate a crash.
*/
ساكن صحيح writeListSync(CrashFile *pFile, صحيح isCrash){
  صحيح rc = SQLITE_OK;
  صحيح iDc = g.iDeviceCharacteristics;

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
  إذا( !isCrash ){
    لكل(pWrite=g.pWriteList; pWrite; pWrite=pWrite->pNext){
      إذا( pWrite->pFile==pFile ){
        pFinal = pWrite;
      }
    }
  }وإلا إذا( iDc&(SQLITE_IOCAP_SEQUENTIAL|SQLITE_IOCAP_SAFE_APPEND) ){
    صحيح nWrite = 0;
    صحيح iFinal;
    لكل(pWrite=g.pWriteList; pWrite; pWrite=pWrite->pNext) nWrite++;
    sqlite3_randomness(حجم(صحيح), &iFinal);
    iFinal = ((iFinal<0)?-1*iFinal:iFinal)%nWrite;
    لكل(pWrite=g.pWriteList; iFinal>0; pWrite=pWrite->pNext) iFinal--;
    pFinal = pWrite;
  }

#ifdef TRACE_CRASHTEST
  إذا( pFile ){
    printf("Sync %s (is %s crash)\n", pFile->zName, (isCrash?"a":"not a"));
  }
#endif

  ppPtr = &g.pWriteList;
  لكل(pWrite=*ppPtr; rc==SQLITE_OK && pWrite; pWrite=*ppPtr){
    sqlite3_file *pRealFile = pWrite->pFile->pRealFile;

    /* (eAction==1)      -> write block out normally,
    ** (eAction==2)      -> do nothing,
    ** (eAction==3)      -> trash sectors.
    */
    صحيح eAction = 0;
    إذا( !isCrash ){
      eAction = 2;
      إذا( (pWrite->pFile==pFile || iDc&SQLITE_IOCAP_SEQUENTIAL) ){
        eAction = 1;
      }
    }وإلا{
      حرف random;
      sqlite3_randomness(1, &random);

      /* Do not select option 3 (sector trashing) if the IOCAP_ATOMIC flag
      ** is set or this is an OsTruncate(), not an Oswrite().
      */
      إذا( (iDc&SQLITE_IOCAP_ATOMIC) || (pWrite->zBuf==0) ){
        random &= 0x01;
      }

      /* If IOCAP_SEQUENTIAL is set and this is not the final entry
      ** in the truncated write-list, always select option 1 (write
      ** out correctly).
      */
      إذا( (iDc&SQLITE_IOCAP_SEQUENTIAL && pWrite!=pFinal) ){
        random = 0;
      }

      /* If IOCAP_SAFE_APPEND is set and this OsWrite() operation is
      ** an append (first byte of the written region is 1 byte past the
      ** current EOF), always select option 1 (write out correctly).
      */
      إذا( iDc&SQLITE_IOCAP_SAFE_APPEND && pWrite->zBuf ){
        i64 iSize;
        sqlite3OsFileSize(pRealFile, &iSize);
        إذا( iSize==pWrite->iOffset ){
          random = 0;
        }
      }

      إذا( (random&0x06)==0x06 ){
        eAction = 3;
      }وإلا{
        eAction = ((random&0x01)?2:1);
      }
    }

    بدل( eAction ){
      حالة 1: {               /* Write out correctly */
        إذا( pWrite->zBuf ){
          rc = writeDbFile(
              pWrite->pFile, pWrite->zBuf, pWrite->nBuf, pWrite->iOffset
          );
        }وإلا{
          rc = sqlite3OsTruncate(pRealFile, pWrite->iOffset);
        }
        *ppPtr = pWrite->pNext;
#ifdef TRACE_CRASHTEST
        إذا( isCrash ){
          printf("Writing %d bytes @ %d (%s)\n",
            pWrite->nBuf, (صحيح)pWrite->iOffset, pWrite->pFile->zName
          );
        }
#endif
        crash_free(pWrite);
        اكسر;
      }
      حالة 2: {               /* Do nothing */
        ppPtr = &pWrite->pNext;
#ifdef TRACE_CRASHTEST
        إذا( isCrash ){
          printf("Omiting %d bytes @ %d (%s)\n",
            pWrite->nBuf, (صحيح)pWrite->iOffset, pWrite->pFile->zName
          );
        }
#endif
        اكسر;
      }
      حالة 3: {               /* Trash sectors */
        u8 *zGarbage;
        صحيح iFirst = (صحيح)(pWrite->iOffset/g.iSectorSize);
        صحيح iLast = (صحيح)((pWrite->iOffset+pWrite->nBuf-1)/g.iSectorSize);

        أكد(pWrite->zBuf);

#ifdef TRACE_CRASHTEST
        printf("Trashing %d sectors (%d bytes) @ %lld (sector %d) (%s)\n",
            1+iLast-iFirst, (1+iLast-iFirst)*g.iSectorSize,
            pWrite->iOffset, iFirst, pWrite->pFile->zName
        );
#endif

        zGarbage = crash_malloc(g.iSectorSize);
        إذا( zGarbage ){
          sqlite3_int64 i;
          لكل(i=iFirst; rc==SQLITE_OK && i<=iLast; i++){
            sqlite3_randomness(g.iSectorSize, zGarbage);
            rc = writeDbFile(
              pWrite->pFile, zGarbage, g.iSectorSize, i*g.iSectorSize
            );
          }
          crash_free(zGarbage);
        }وإلا{
          rc = SQLITE_NOMEM;
        }

        ppPtr = &pWrite->pNext;
        اكسر;
      }

      افتراضي:
        أكد(!"Cannot happen");
    }

    إذا( pWrite==pFinal ) اكسر;
  }

  إذا( rc==SQLITE_OK && isCrash ){
    exit(-1);
  }

  لكل(pWrite=g.pWriteList; pWrite && pWrite->pNext; pWrite=pWrite->pNext);
  g.pWriteListEnd = pWrite;

  أرجع rc;
}

/*
** Add an entry to the end of the write-list.
*/
ساكن صحيح writeListAppend(
  sqlite3_file *pFile,
  sqlite3_int64 iOffset,
  ثابت u8 *zBuf,
  صحيح nBuf
){
  WriteBuffer *pNew;

  أكد((zBuf && nBuf) || (!nBuf && !zBuf));

  pNew = (WriteBuffer *)crash_malloc(حجم(WriteBuffer) + nBuf);
  إذا( pNew==0 ){
    fprintf(stderr, "out of memory in the crash simulator\n");
  }
  memset(pNew, 0, حجم(WriteBuffer)+nBuf);
  pNew->iOffset = iOffset;
  pNew->nBuf = nBuf;
  pNew->pFile = (CrashFile *)pFile;
  إذا( zBuf ){
    pNew->zBuf = (u8 *)&pNew[1];
    memcpy(pNew->zBuf, zBuf, nBuf);
  }

  إذا( g.pWriteList ){
    أكد(g.pWriteListEnd);
    g.pWriteListEnd->pNext = pNew;
  }وإلا{
    g.pWriteList = pNew;
  }
  g.pWriteListEnd = pNew;
 
  أرجع SQLITE_OK;
}

/*
** Close a crash-file.
*/
ساكن صحيح cfClose(sqlite3_file *pFile){
  CrashFile *pCrash = (CrashFile *)pFile;
  writeListSync(pCrash, 0);
  sqlite3OsClose(pCrash->pRealFile);
  أرجع SQLITE_OK;
}

/*
** Read data from a crash-file.
*/
ساكن صحيح cfRead(
  sqlite3_file *pFile,
  فراغ *zBuf,
  صحيح iAmt,
  sqlite_int64 iOfst
){
  CrashFile *pCrash = (CrashFile *)pFile;
  صحيح nCopy = (صحيح)MIN((i64)iAmt, (pCrash->iSize - iOfst));

  إذا( nCopy>0 ){
    memcpy(zBuf, &pCrash->zData[iOfst], nCopy);
  }

  /* Check the file-size to see if this is a short-read */
  إذا( nCopy<iAmt ){
    أرجع SQLITE_IOERR_SHORT_READ;
  }

  أرجع SQLITE_OK;
}

/*
** Write data to a crash-file.
*/
ساكن صحيح cfWrite(
  sqlite3_file *pFile,
  ثابت فراغ *zBuf,
  صحيح iAmt,
  sqlite_int64 iOfst
){
  CrashFile *pCrash = (CrashFile *)pFile;
  إذا( iAmt+iOfst>pCrash->iSize ){
    pCrash->iSize = (صحيح)(iAmt+iOfst);
  }
  طالما( pCrash->iSize>pCrash->nData ){
    u8 *zNew;
    صحيح nNew = (pCrash->nData*2) + 4096;
    zNew = crash_realloc(pCrash->zData, nNew);
    إذا( !zNew ){
      أرجع SQLITE_NOMEM;
    }
    memset(&zNew[pCrash->nData], 0, nNew-pCrash->nData);
    pCrash->nData = nNew;
    pCrash->zData = zNew;
  }
  memcpy(&pCrash->zData[iOfst], zBuf, iAmt);
  أرجع writeListAppend(pFile, iOfst, zBuf, iAmt);
}

/*
** Truncate a crash-file.
*/
ساكن صحيح cfTruncate(sqlite3_file *pFile, sqlite_int64 size){
  CrashFile *pCrash = (CrashFile *)pFile;
  أكد(size>=0);
  إذا( pCrash->iSize>size ){
    pCrash->iSize = (صحيح)size;
  }
  أرجع writeListAppend(pFile, size, 0, 0);
}

/*
** Sync a crash-file.
*/
ساكن صحيح cfSync(sqlite3_file *pFile, صحيح flags){
  CrashFile *pCrash = (CrashFile *)pFile;
  صحيح isCrash = 0;

  ثابت حرف *zName = pCrash->zName;
  ثابت حرف *zCrashFile = g.zCrashFile;
  صحيح nName = (صحيح)strlen(zName);
  صحيح nCrashFile = (صحيح)strlen(zCrashFile);

  إذا( nCrashFile>0 && zCrashFile[nCrashFile-1]=='*' ){
    nCrashFile--;
    إذا( nName>nCrashFile ) nName = nCrashFile;
  }

#ifdef TRACE_CRASHTEST
  printf("cfSync(): nName = %d, nCrashFile = %d, zName = %s, zCrashFile = %s\n",
         nName, nCrashFile, zName, zCrashFile);
#endif

  إذا( nName==nCrashFile && 0==memcmp(zName, zCrashFile, nName) ){
#ifdef TRACE_CRASHTEST
    printf("cfSync(): name matched, g.iCrash = %d\n", g.iCrash);
#endif
    إذا( (--g.iCrash)==0 ) isCrash = 1;
  }

  أرجع writeListSync(pCrash, isCrash);
}

/*
** Return the current file-size of the crash-file.
*/
ساكن صحيح cfFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  CrashFile *pCrash = (CrashFile *)pFile;
  *pSize = (i64)pCrash->iSize;
  أرجع SQLITE_OK;
}

/*
** Calls related to file-locks are passed on to the real file handle.
*/
ساكن صحيح cfLock(sqlite3_file *pFile, صحيح eLock){
  أرجع sqlite3OsLock(((CrashFile *)pFile)->pRealFile, eLock);
}
ساكن صحيح cfUnlock(sqlite3_file *pFile, صحيح eLock){
  أرجع sqlite3OsUnlock(((CrashFile *)pFile)->pRealFile, eLock);
}
ساكن صحيح cfCheckReservedLock(sqlite3_file *pFile, صحيح *pResOut){
  أرجع sqlite3OsCheckReservedLock(((CrashFile *)pFile)->pRealFile, pResOut);
}
ساكن صحيح cfFileControl(sqlite3_file *pFile, صحيح op, فراغ *pArg){
  إذا( op==SQLITE_FCNTL_SIZE_HINT ){
    CrashFile *pCrash = (CrashFile *)pFile;
    i64 nByte = *(i64 *)pArg;
    إذا( nByte>pCrash->iSize ){
      إذا( SQLITE_OK==writeListAppend(pFile, nByte, 0, 0) ){
        pCrash->iSize = (صحيح)nByte;
      }
    }
    أرجع SQLITE_OK;
  }
  أرجع sqlite3OsFileControl(((CrashFile *)pFile)->pRealFile, op, pArg);
}

/*
** The xSectorSize() and xDeviceCharacteristics() functions return
** the global values configured by the [sqlite_crashparams] tcl
*  interface.
*/
ساكن صحيح cfSectorSize(sqlite3_file *pFile){
  أرجع g.iSectorSize;
}
ساكن صحيح cfDeviceCharacteristics(sqlite3_file *pFile){
  أرجع g.iDeviceCharacteristics;
}

/*
** Pass-throughs for WAL support.
*/
ساكن صحيح cfShmLock(sqlite3_file *pFile, صحيح ofst, صحيح n, صحيح flags){
  sqlite3_file *pReal = ((CrashFile*)pFile)->pRealFile;
  أرجع pReal->pMethods->xShmLock(pReal, ofst, n, flags);
}
ساكن فراغ cfShmBarrier(sqlite3_file *pFile){
  sqlite3_file *pReal = ((CrashFile*)pFile)->pRealFile;
  pReal->pMethods->xShmBarrier(pReal);
}
ساكن صحيح cfShmUnmap(sqlite3_file *pFile, صحيح delFlag){
  sqlite3_file *pReal = ((CrashFile*)pFile)->pRealFile;
  أرجع pReal->pMethods->xShmUnmap(pReal, delFlag);
}
ساكن صحيح cfShmMap(
  sqlite3_file *pFile,            /* Handle open on database file */
  صحيح iRegion,                    /* Region to retrieve */
  صحيح sz,                         /* Size of regions */
  صحيح w,                          /* True to extend file if necessary */
  فراغ متطاير **pp              /* OUT: Mapped memory */
){
  sqlite3_file *pReal = ((CrashFile*)pFile)->pRealFile;
  أرجع pReal->pMethods->xShmMap(pReal, iRegion, sz, w, pp);
}

ساكن ثابت sqlite3_io_methods CrashFileVtab = {
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
بنية crashAppData {
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
ساكن صحيح cfOpen(
  sqlite3_vfs *pCfVfs,
  ثابت حرف *zName,
  sqlite3_file *pFile,
  صحيح flags,
  صحيح *pOutFlags
){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  صحيح rc;
  CrashFile *pWrapper = (CrashFile *)pFile;
  sqlite3_file *pReal = (sqlite3_file*)&pWrapper[1];

  memset(pWrapper, 0, حجم(CrashFile));
  rc = sqlite3OsOpen(pVfs, zName, pReal, flags, pOutFlags);

  إذا( rc==SQLITE_OK ){
    i64 iSize;
    pWrapper->pMethod = &CrashFileVtab;
    pWrapper->zName = (حرف *)zName;
    pWrapper->pRealFile = pReal;
    rc = sqlite3OsFileSize(pReal, &iSize);
    pWrapper->iSize = (صحيح)iSize;
    pWrapper->flags = flags;
  }
  إذا( rc==SQLITE_OK ){
    pWrapper->nData = (صحيح)(4096 + pWrapper->iSize);
    pWrapper->zData = crash_malloc(pWrapper->nData);
    إذا( pWrapper->zData ){
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
      لكل(iOff=0; iOff<pWrapper->iSize; iOff += 512){
        صحيح nRead = (صحيح)(pWrapper->iSize - iOff);
        إذا( nRead>512 ) nRead = 512;
        rc = sqlite3OsRead(pReal, &pWrapper->zData[iOff], nRead, iOff);
      }
    }وإلا{
      rc = SQLITE_NOMEM;
    }
  }
  إذا( rc!=SQLITE_OK && pWrapper->pMethod ){
    sqlite3OsClose(pFile);
  }
  أرجع rc;
}

ساكن صحيح cfDelete(sqlite3_vfs *pCfVfs, ثابت حرف *zPath, صحيح dirSync){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  أرجع pVfs->xDelete(pVfs, zPath, dirSync);
}
ساكن صحيح cfAccess(
  sqlite3_vfs *pCfVfs,
  ثابت حرف *zPath,
  صحيح flags,
  صحيح *pResOut
){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  أرجع pVfs->xAccess(pVfs, zPath, flags, pResOut);
}
ساكن صحيح cfFullPathname(
  sqlite3_vfs *pCfVfs,
  ثابت حرف *zPath,
  صحيح nPathOut,
  حرف *zPathOut
){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  أرجع pVfs->xFullPathname(pVfs, zPath, nPathOut, zPathOut);
}
ساكن فراغ *cfDlOpen(sqlite3_vfs *pCfVfs, ثابت حرف *zPath){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  أرجع pVfs->xDlOpen(pVfs, zPath);
}
ساكن فراغ cfDlError(sqlite3_vfs *pCfVfs, صحيح nByte, حرف *z_Err_پیغام){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  pVfs->xDlError(pVfs, nByte, z_Err_پیغام);
}
ساكن فراغ (*cfDlSym(sqlite3_vfs *pCfVfs, فراغ *pH, ثابت حرف *zSym))(فراغ){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  أرجع pVfs->xDlSym(pVfs, pH, zSym);
}
ساكن فراغ cfDlClose(sqlite3_vfs *pCfVfs, فراغ *pHandle){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  pVfs->xDlClose(pVfs, pHandle);
}
ساكن صحيح cfRandomness(sqlite3_vfs *pCfVfs, صحيح nByte, حرف *zBufOut){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  أرجع pVfs->xRandomness(pVfs, nByte, zBufOut);
}
ساكن صحيح cfSleep(sqlite3_vfs *pCfVfs, صحيح nMicro){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  أرجع pVfs->xSleep(pVfs, nMicro);
}
ساكن صحيح cfCurrentTime(sqlite3_vfs *pCfVfs, مزدوج *pTimeOut){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  أرجع pVfs->xCurrentTime(pVfs, pTimeOut);
}
ساكن صحيح cfGetLastError(sqlite3_vfs *pCfVfs, صحيح n, حرف *z){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  أرجع pVfs->xGetLastError(pVfs, n, z);
}

ساكن صحيح processDevSymArgs(
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[],
  صحيح *piDeviceChar,
  صحيح *piSectorSize
){
  بنية DeviceFlag {
    حرف *zName;
    صحيح i_قدر;
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

  صحيح i;
  صحيح iDc = 0;
  صحيح iSectorSize = 0;
  صحيح setSectorsize = 0;
  صحيح setDeviceChar = 0;

  لكل(i=0; i<objc; i+=2){
    Tcl_Size nOpt;
    حرف *zOpt = Tcl_GetStringFromObj(objv[i], &nOpt);

    إذا( (nOpt>11 || nOpt<2 || strncmp("-sectorsize", zOpt, nOpt))
     && (nOpt>16 || nOpt<2 || strncmp("-characteristics", zOpt, nOpt))
    ){
      Tcl_AppendResult(interp,
        "Bad option: \"", zOpt,
        "\" - must be \"-characteristics\" or \"-sectorsize\"", عدم
      );
      أرجع TCL_ERROR;
    }
    إذا( i==objc-1 ){
      Tcl_AppendResult(interp, "Option requires an argument: \"", zOpt, "\"", عدم);
      أرجع TCL_ERROR;
    }

    إذا( zOpt[1]=='s' ){
      إذا( Tcl_GetIntFromObj(interp, objv[i+1], &iSectorSize) ){
        أرجع TCL_ERROR;
      }
      setSectorsize = 1;
    }وإلا{
      صحيح j;
      Tcl_Obj **apObj;
      Tcl_Size nObj;
      إذا( Tcl_ListObjGetElements(interp, objv[i+1], &nObj, &apObj) ){
        أرجع TCL_ERROR;
      }
      لكل(j=0; j<(صحيح)nObj; j++){
        صحيح rc;
        صحيح iChoice;
        Tcl_Obj *pFlag = Tcl_DuplicateObj(apObj[j]);
        Tcl_Incr_Ref_گنتی(pFlag);
        Tcl_UtfToLower(Tcl_GetString(pFlag));

        rc = Tcl_GetIndexFromObjStruct(
            interp, pFlag, aFlag, حجم(aFlag[0]), "no such flag", 0, &iChoice
        );
        Tcl_Decr_Ref_گنتی(pFlag);
        إذا( rc ){
          أرجع TCL_ERROR;
        }

        iDc |= aFlag[iChoice].i_قدر;
      }
      setDeviceChar = 1;
    }
  }

  إذا( setDeviceChar ){
    *piDeviceChar = iDc;
  }
  إذا( setSectorsize ){
    *piSectorSize = iSectorSize;
  }

  أرجع TCL_OK;
}

/*
** tclcmd:   sqlite3_crash_now
**
** Simulate a crash immediately. This function does not return
** (writeListSync() calls exit(-1)).
*/
ساكن صحيح SQLITE_TCLAPI crashNowCmd(
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  إذا( objc!=1 ){
    Tcl_Wrong_عدد_Args(interp, 1, objv, "");
    أرجع TCL_ERROR;
  }
  writeListSync(0, 1);
  أكد( 0 );
  أرجع TCL_OK;
}

/*
** tclcmd:   sqlite_crash_enable ENABLE ?DEFAULT?
**
** Parameter ENABLE must be a boolean value. If true, then the "crash"
** vfs is added to the system. If false, it is removed.
*/
ساكن صحيح SQLITE_TCLAPI crashEnableCmd(
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح isEnable;
  صحيح isDefault = 0;
  ساكن sqlite3_vfs crashVfs = {
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

  إذا( objc!=2 && objc!=3 ){
    Tcl_Wrong_عدد_Args(interp, 1, objv, "ENABLE ?DEFAULT?");
    أرجع TCL_ERROR;
  }

  إذا( Tcl_GetBooleanFromObj(interp, objv[1], &isEnable) ){
    أرجع TCL_ERROR;
  }
  إذا( objc==3 && Tcl_GetBooleanFromObj(interp, objv[2], &isDefault) ){
    أرجع TCL_ERROR;
  }

  إذا( (isEnable && crashVfs.pAppData) || (!isEnable && !crashVfs.pAppData) ){
    أرجع TCL_OK;
  }

  إذا( crashVfs.pAppData==0 ){
    sqlite3_vfs *pOriginalVfs = sqlite3_vfs_find(0);
    crashVfs.mxPathname = pOriginalVfs->mxPathname;
    crashVfs.pAppData = (فراغ *)pOriginalVfs;
    crashVfs.szOsFile = حجم(CrashFile) + pOriginalVfs->szOsFile;
    sqlite3_vfs_register(&crashVfs, isDefault);
  }وإلا{
    crashVfs.pAppData = 0;
    sqlite3_vfs_unregister(&crashVfs);
  }

  أرجع TCL_OK;
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
ساكن صحيح SQLITE_TCLAPI crashParamsObjCmd(
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح iDelay;
  ثابت حرف *zCrashFile;
  Tcl_Size nCrashFile;
  صحيح iDc, iSectorSize;

  iDc = -1;
  iSectorSize = -1;

  إذا( objc<3 ){
    Tcl_Wrong_عدد_Args(interp, 1, objv, "?OPTIONS? DELAY CRASHFILE");
    اذهب error;
  }

  zCrashFile = Tcl_GetStringFromObj(objv[objc-1], &nCrashFile);
  إذا( nCrashFile>=حجم(g.zCrashFile) ){
    Tcl_AppendResult(interp, "Filename is too long: \"", zCrashFile, "\"", عدم);
    اذهب error;
  }
  إذا( Tcl_GetIntFromObj(interp, objv[objc-2], &iDelay) ){
    اذهب error;
  }

  إذا( processDevSymArgs(interp, objc-3, &objv[1], &iDc, &iSectorSize) ){
    أرجع TCL_ERROR;
  }

  إذا( iDc>=0 ){
    g.iDeviceCharacteristics = iDc;
  }
  إذا( iSectorSize>=0 ){
    g.iSectorSize = iSectorSize;
  }

  g.iCrash = iDelay;
  memcpy(g.zCrashFile, zCrashFile, nCrashFile+1);
  sqlite3CrashTestEnable = 1;
  أرجع TCL_OK;

error:
  أرجع TCL_ERROR;
}

ساكن صحيح SQLITE_TCLAPI devSymObjCmd(
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  فراغ devsym_register(صحيح iDeviceChar, صحيح iSectorSize);

  صحيح iDc = -1;
  صحيح iSectorSize = -1;

  إذا( processDevSymArgs(interp, objc-1, &objv[1], &iDc, &iSectorSize) ){
    أرجع TCL_ERROR;
  }
  devsym_register(iDc, iSectorSize);

  أرجع TCL_OK;
}

/*
** tclcmd: sqlite3_crash_on_write N
*/
ساكن صحيح SQLITE_TCLAPI writeCrashObjCmd(
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  فراغ devsym_crash_on_write(صحيح);
  صحيح nWrite = 0;

  إذا( objc!=2 ){
    Tcl_Wrong_عدد_Args(interp, 1, objv, "NWRITE");
    أرجع TCL_ERROR;
  }
  إذا( Tcl_GetIntFromObj(interp, objv[1], &nWrite) ){
    أرجع TCL_ERROR;
  }

  devsym_crash_on_write(nWrite);
  أرجع TCL_OK;
}

/*
** tclcmd: unregister_devsim
*/
ساكن صحيح SQLITE_TCLAPI dsUnregisterObjCmd(
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  فراغ devsym_unregister(فراغ);

  إذا( objc!=1 ){
    Tcl_Wrong_عدد_Args(interp, 1, objv, "");
    أرجع TCL_ERROR;
  }

  devsym_unregister();
  أرجع TCL_OK;
}

/*
** tclcmd: register_jt_vfs ?-default? PARENT-VFS
*/
ساكن صحيح SQLITE_TCLAPI jtObjCmd(
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح jt_register(حرف *, صحيح);
  حرف *zParent = 0;

  إذا( objc!=2 && objc!=3 ){
    Tcl_Wrong_عدد_Args(interp, 1, objv, "?-default? PARENT-VFS");
    أرجع TCL_ERROR;
  }
  zParent = Tcl_GetString(objv[1]);
  إذا( objc==3 ){
    إذا( strcmp(zParent, "-default") ){
      Tcl_AppendResult(interp,
          "bad option \"", zParent, "\": must be -default", عدم
      );
      أرجع TCL_ERROR;
    }
    zParent = Tcl_GetString(objv[2]);
  }

  إذا( !(*zParent) ){
    zParent = 0;
  }
  إذا( jt_register(zParent, objc==3) ){
    Tcl_AppendResult(interp, "Error in jt_register", عدم);
    أرجع TCL_ERROR;
  }

  أرجع TCL_OK;
}

/*
** tclcmd: unregister_jt_vfs
*/
ساكن صحيح SQLITE_TCLAPI jtUnregisterObjCmd(
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  فراغ jt_unregister(فراغ);

  إذا( objc!=1 ){
    Tcl_Wrong_عدد_Args(interp, 1, objv, "");
    أرجع TCL_ERROR;
  }

  jt_unregister();
  أرجع TCL_OK;
}

#endif /* SQLITE_OMIT_DISKIO */

/*
** This procedure registers the TCL procedures defined in this file.
*/
صحيح Sqlitetest6_Init(Tcl_Interp *interp){
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
  أرجع TCL_OK;
}

#endif /* SQLITE_TEST */
