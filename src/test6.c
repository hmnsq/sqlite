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
#எனில் SQLITE_TEST          /* This file is used for testing only */
#அந்தர்பாவ "sqliteInt.h"
#அந்தர்பாவ "tclsqlite.h"

#ifndef SQLITE_OMIT_DISKIO  /* This file is a no-op if disk I/O is disabled */

/* #define TRACE_CRASHTEST */

வகைவரை கட்டமைப்பு CrashFile CrashFile;
வகைவரை கட்டமைப்பு CrashGlobal CrashGlobal;
வகைவரை கட்டமைப்பு WriteBuffer WriteBuffer;

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
கட்டமைப்பு WriteBuffer {
  i64 iOffset;                 /* Byte offset of the start of this write() */
  முழு nBuf;                    /* Number of bytes written */
  u8 *zBuf;                    /* Pointer to copy of written data */
  CrashFile *pFile;            /* File this write() applies to */

  WriteBuffer *pNext;          /* Next in CrashGlobal.pWriteList */
};

கட்டமைப்பு CrashFile {
  மாறிலி sqlite3_io_methods *pMethod;   /* Must be first */
  sqlite3_file *pRealFile;             /* Underlying "real" file handle */
  எழுத்து *zName;
  முழு flags;                           /* Flags the file was opened with */

  /* Cache of the entire file. This is used to speed up OsRead() and
  ** OsFileSize() calls. Although both could be done by traversing the
  ** write-list, in practice this is impractically slow.
  */
  u8 *zData;                           /* Buffer containing file contents */
  முழு nData;                           /* Size of buffer allocated at zData */
  i64 iSize;                           /* Size of file in bytes */
};

கட்டமைப்பு CrashGlobal {
  WriteBuffer *pWriteList;     /* Head of write-list */
  WriteBuffer *pWriteListEnd;  /* End of write-list */

  முழு iSectorSize;             /* Value of simulated sector size */
  முழு iDeviceCharacteristics;  /* Value of simulated device characteristics */

  முழு iCrash;                  /* Crash on the iCrash'th call to xSync() */
  எழுத்து zCrashFile[500];        /* Crash during an xSync() on this file */
};

நிலையான CrashGlobal g = {0, 0, SQLITE_DEFAULT_SECTOR_SIZE, 0, 0};

/*
** Set this global variable to 1 to enable crash testing.
*/
நிலையான முழு sqlite3CrashTestEnable = 0;

நிலையான வெறுமை *crash_malloc(முழு nByte){
  திரும்பு (வெறுமை *)Tcl_AttemptAlloc((size_t)nByte);
}
நிலையான வெறுமை crash_free(வெறுமை *p){
  Tcl_Free(p);
}
நிலையான வெறுமை *crash_realloc(வெறுமை *p, முழு n){
  திரும்பு (வெறுமை *)Tcl_AttemptRealloc(p, (size_t)n);
}

/*
** Wrapper around the sqlite3OsWrite() function that avoids writing to the
** 512 byte block beginning at offset PENDING_BYTE.
*/
நிலையான முழு writeDbFile(CrashFile *p, u8 *z, i64 iAmt, i64 iOff){
  முழு rc = SQLITE_OK;
  முழு iSkip = 0;
  எனில்( (iAmt-iSkip)>0 ){
    rc = sqlite3OsWrite(p->pRealFile, &z[iSkip], (முழு)(iAmt-iSkip), iOff+iSkip);
  }
  திரும்பு rc;
}

/*
** Flush the write-list as if xSync() had been called on file handle
** pFile. If isCrash is true, simulate a crash.
*/
நிலையான முழு writeListSync(CrashFile *pFile, முழு isCrash){
  முழு rc = SQLITE_OK;
  முழு iDc = g.iDeviceCharacteristics;

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
  எனில்( !isCrash ){
    க்கு(pWrite=g.pWriteList; pWrite; pWrite=pWrite->pNext){
      எனில்( pWrite->pFile==pFile ){
        pFinal = pWrite;
      }
    }
  }இல்லையெனில் எனில்( iDc&(SQLITE_IOCAP_SEQUENTIAL|SQLITE_IOCAP_SAFE_APPEND) ){
    முழு nWrite = 0;
    முழு iFinal;
    க்கு(pWrite=g.pWriteList; pWrite; pWrite=pWrite->pNext) nWrite++;
    sqlite3_randomness(அளவு(முழு), &iFinal);
    iFinal = ((iFinal<0)?-1*iFinal:iFinal)%nWrite;
    க்கு(pWrite=g.pWriteList; iFinal>0; pWrite=pWrite->pNext) iFinal--;
    pFinal = pWrite;
  }

#ifdef TRACE_CRASHTEST
  எனில்( pFile ){
    printf("Sync %s (is %s crash)\n", pFile->zName, (isCrash?"a":"not a"));
  }
#endif

  ppPtr = &g.pWriteList;
  க்கு(pWrite=*ppPtr; rc==SQLITE_OK && pWrite; pWrite=*ppPtr){
    sqlite3_file *pRealFile = pWrite->pFile->pRealFile;

    /* (eAction==1)      -> write block out normally,
    ** (eAction==2)      -> do nothing,
    ** (eAction==3)      -> trash sectors.
    */
    முழு eAction = 0;
    எனில்( !isCrash ){
      eAction = 2;
      எனில்( (pWrite->pFile==pFile || iDc&SQLITE_IOCAP_SEQUENTIAL) ){
        eAction = 1;
      }
    }இல்லையெனில்{
      எழுத்து random;
      sqlite3_randomness(1, &random);

      /* Do not select option 3 (sector trashing) if the IOCAP_ATOMIC flag
      ** is set or this is an OsTruncate(), not an Oswrite().
      */
      எனில்( (iDc&SQLITE_IOCAP_ATOMIC) || (pWrite->zBuf==0) ){
        random &= 0x01;
      }

      /* If IOCAP_SEQUENTIAL is set and this is not the final entry
      ** in the truncated write-list, always select option 1 (write
      ** out correctly).
      */
      எனில்( (iDc&SQLITE_IOCAP_SEQUENTIAL && pWrite!=pFinal) ){
        random = 0;
      }

      /* If IOCAP_SAFE_APPEND is set and this OsWrite() operation is
      ** an append (first byte of the written region is 1 byte past the
      ** current EOF), always select option 1 (write out correctly).
      */
      எனில்( iDc&SQLITE_IOCAP_SAFE_APPEND && pWrite->zBuf ){
        i64 iSize;
        sqlite3OsFileSize(pRealFile, &iSize);
        எனில்( iSize==pWrite->iOffset ){
          random = 0;
        }
      }

      எனில்( (random&0x06)==0x06 ){
        eAction = 3;
      }இல்லையெனில்{
        eAction = ((random&0x01)?2:1);
      }
    }

    தேர்வு( eAction ){
      நிலை 1: {               /* Write out correctly */
        எனில்( pWrite->zBuf ){
          rc = writeDbFile(
              pWrite->pFile, pWrite->zBuf, pWrite->nBuf, pWrite->iOffset
          );
        }இல்லையெனில்{
          rc = sqlite3OsTruncate(pRealFile, pWrite->iOffset);
        }
        *ppPtr = pWrite->pNext;
#ifdef TRACE_CRASHTEST
        எனில்( isCrash ){
          printf("Writing %d bytes @ %d (%s)\n",
            pWrite->nBuf, (முழு)pWrite->iOffset, pWrite->pFile->zName
          );
        }
#endif
        crash_free(pWrite);
        நிறுத்து;
      }
      நிலை 2: {               /* Do nothing */
        ppPtr = &pWrite->pNext;
#ifdef TRACE_CRASHTEST
        எனில்( isCrash ){
          printf("Omiting %d bytes @ %d (%s)\n",
            pWrite->nBuf, (முழு)pWrite->iOffset, pWrite->pFile->zName
          );
        }
#endif
        நிறுத்து;
      }
      நிலை 3: {               /* Trash sectors */
        u8 *zGarbage;
        முழு iFirst = (முழு)(pWrite->iOffset/g.iSectorSize);
        முழு iLast = (முழு)((pWrite->iOffset+pWrite->nBuf-1)/g.iSectorSize);

        ப்ரதிஜ்ஞா(pWrite->zBuf);

#ifdef TRACE_CRASHTEST
        printf("Trashing %d sectors (%d bytes) @ %lld (sector %d) (%s)\n",
            1+iLast-iFirst, (1+iLast-iFirst)*g.iSectorSize,
            pWrite->iOffset, iFirst, pWrite->pFile->zName
        );
#endif

        zGarbage = crash_malloc(g.iSectorSize);
        எனில்( zGarbage ){
          sqlite3_int64 i;
          க்கு(i=iFirst; rc==SQLITE_OK && i<=iLast; i++){
            sqlite3_randomness(g.iSectorSize, zGarbage);
            rc = writeDbFile(
              pWrite->pFile, zGarbage, g.iSectorSize, i*g.iSectorSize
            );
          }
          crash_free(zGarbage);
        }இல்லையெனில்{
          rc = SQLITE_NOMEM;
        }

        ppPtr = &pWrite->pNext;
        நிறுத்து;
      }

      இயல்பு:
        ப்ரதிஜ்ஞா(!"Cannot happen");
    }

    எனில்( pWrite==pFinal ) நிறுத்து;
  }

  எனில்( rc==SQLITE_OK && isCrash ){
    exit(-1);
  }

  க்கு(pWrite=g.pWriteList; pWrite && pWrite->pNext; pWrite=pWrite->pNext);
  g.pWriteListEnd = pWrite;

  திரும்பு rc;
}

/*
** Add an entry to the end of the write-list.
*/
நிலையான முழு writeListAppend(
  sqlite3_file *pFile,
  sqlite3_int64 iOffset,
  மாறிலி u8 *zBuf,
  முழு nBuf
){
  WriteBuffer *pNew;

  ப்ரதிஜ்ஞா((zBuf && nBuf) || (!nBuf && !zBuf));

  pNew = (WriteBuffer *)crash_malloc(அளவு(WriteBuffer) + nBuf);
  எனில்( pNew==0 ){
    fprintf(stderr, "out of memory in the crash simulator\n");
  }
  memset(pNew, 0, அளவு(WriteBuffer)+nBuf);
  pNew->iOffset = iOffset;
  pNew->nBuf = nBuf;
  pNew->pFile = (CrashFile *)pFile;
  எனில்( zBuf ){
    pNew->zBuf = (u8 *)&pNew[1];
    memcpy(pNew->zBuf, zBuf, nBuf);
  }

  எனில்( g.pWriteList ){
    ப்ரதிஜ்ஞா(g.pWriteListEnd);
    g.pWriteListEnd->pNext = pNew;
  }இல்லையெனில்{
    g.pWriteList = pNew;
  }
  g.pWriteListEnd = pNew;
 
  திரும்பு SQLITE_OK;
}

/*
** Close a crash-file.
*/
நிலையான முழு cfClose(sqlite3_file *pFile){
  CrashFile *pCrash = (CrashFile *)pFile;
  writeListSync(pCrash, 0);
  sqlite3OsClose(pCrash->pRealFile);
  திரும்பு SQLITE_OK;
}

/*
** Read data from a crash-file.
*/
நிலையான முழு cfRead(
  sqlite3_file *pFile,
  வெறுமை *zBuf,
  முழு iAmt,
  sqlite_int64 iOfst
){
  CrashFile *pCrash = (CrashFile *)pFile;
  முழு nCopy = (முழு)MIN((i64)iAmt, (pCrash->iSize - iOfst));

  எனில்( nCopy>0 ){
    memcpy(zBuf, &pCrash->zData[iOfst], nCopy);
  }

  /* Check the file-size to see if this is a short-read */
  எனில்( nCopy<iAmt ){
    திரும்பு SQLITE_IOERR_SHORT_READ;
  }

  திரும்பு SQLITE_OK;
}

/*
** Write data to a crash-file.
*/
நிலையான முழு cfWrite(
  sqlite3_file *pFile,
  மாறிலி வெறுமை *zBuf,
  முழு iAmt,
  sqlite_int64 iOfst
){
  CrashFile *pCrash = (CrashFile *)pFile;
  எனில்( iAmt+iOfst>pCrash->iSize ){
    pCrash->iSize = (முழு)(iAmt+iOfst);
  }
  போது( pCrash->iSize>pCrash->nData ){
    u8 *zNew;
    முழு nNew = (pCrash->nData*2) + 4096;
    zNew = crash_realloc(pCrash->zData, nNew);
    எனில்( !zNew ){
      திரும்பு SQLITE_NOMEM;
    }
    memset(&zNew[pCrash->nData], 0, nNew-pCrash->nData);
    pCrash->nData = nNew;
    pCrash->zData = zNew;
  }
  memcpy(&pCrash->zData[iOfst], zBuf, iAmt);
  திரும்பு writeListAppend(pFile, iOfst, zBuf, iAmt);
}

/*
** Truncate a crash-file.
*/
நிலையான முழு cfTruncate(sqlite3_file *pFile, sqlite_int64 size){
  CrashFile *pCrash = (CrashFile *)pFile;
  ப்ரதிஜ்ஞா(size>=0);
  எனில்( pCrash->iSize>size ){
    pCrash->iSize = (முழு)size;
  }
  திரும்பு writeListAppend(pFile, size, 0, 0);
}

/*
** Sync a crash-file.
*/
நிலையான முழு cfSync(sqlite3_file *pFile, முழு flags){
  CrashFile *pCrash = (CrashFile *)pFile;
  முழு isCrash = 0;

  மாறிலி எழுத்து *zName = pCrash->zName;
  மாறிலி எழுத்து *zCrashFile = g.zCrashFile;
  முழு nName = (முழு)strlen(zName);
  முழு nCrashFile = (முழு)strlen(zCrashFile);

  எனில்( nCrashFile>0 && zCrashFile[nCrashFile-1]=='*' ){
    nCrashFile--;
    எனில்( nName>nCrashFile ) nName = nCrashFile;
  }

#ifdef TRACE_CRASHTEST
  printf("cfSync(): nName = %d, nCrashFile = %d, zName = %s, zCrashFile = %s\n",
         nName, nCrashFile, zName, zCrashFile);
#endif

  எனில்( nName==nCrashFile && 0==memcmp(zName, zCrashFile, nName) ){
#ifdef TRACE_CRASHTEST
    printf("cfSync(): name matched, g.iCrash = %d\n", g.iCrash);
#endif
    எனில்( (--g.iCrash)==0 ) isCrash = 1;
  }

  திரும்பு writeListSync(pCrash, isCrash);
}

/*
** Return the current file-size of the crash-file.
*/
நிலையான முழு cfFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  CrashFile *pCrash = (CrashFile *)pFile;
  *pSize = (i64)pCrash->iSize;
  திரும்பு SQLITE_OK;
}

/*
** Calls related to file-locks are passed on to the real file handle.
*/
நிலையான முழு cfLock(sqlite3_file *pFile, முழு eLock){
  திரும்பு sqlite3OsLock(((CrashFile *)pFile)->pRealFile, eLock);
}
நிலையான முழு cfUnlock(sqlite3_file *pFile, முழு eLock){
  திரும்பு sqlite3OsUnlock(((CrashFile *)pFile)->pRealFile, eLock);
}
நிலையான முழு cfCheckReservedLock(sqlite3_file *pFile, முழு *pResOut){
  திரும்பு sqlite3OsCheckReservedLock(((CrashFile *)pFile)->pRealFile, pResOut);
}
நிலையான முழு cfFileControl(sqlite3_file *pFile, முழு op, வெறுமை *pArg){
  எனில்( op==SQLITE_FCNTL_SIZE_HINT ){
    CrashFile *pCrash = (CrashFile *)pFile;
    i64 nByte = *(i64 *)pArg;
    எனில்( nByte>pCrash->iSize ){
      எனில்( SQLITE_OK==writeListAppend(pFile, nByte, 0, 0) ){
        pCrash->iSize = (முழு)nByte;
      }
    }
    திரும்பு SQLITE_OK;
  }
  திரும்பு sqlite3OsFileControl(((CrashFile *)pFile)->pRealFile, op, pArg);
}

/*
** The xSectorSize() and xDeviceCharacteristics() functions return
** the global values configured by the [sqlite_crashparams] tcl
*  interface.
*/
நிலையான முழு cfSectorSize(sqlite3_file *pFile){
  திரும்பு g.iSectorSize;
}
நிலையான முழு cfDeviceCharacteristics(sqlite3_file *pFile){
  திரும்பு g.iDeviceCharacteristics;
}

/*
** Pass-throughs for WAL support.
*/
நிலையான முழு cfShmLock(sqlite3_file *pFile, முழு ofst, முழு n, முழு flags){
  sqlite3_file *pReal = ((CrashFile*)pFile)->pRealFile;
  திரும்பு pReal->pMethods->xShmLock(pReal, ofst, n, flags);
}
நிலையான வெறுமை cfShmBarrier(sqlite3_file *pFile){
  sqlite3_file *pReal = ((CrashFile*)pFile)->pRealFile;
  pReal->pMethods->xShmBarrier(pReal);
}
நிலையான முழு cfShmUnmap(sqlite3_file *pFile, முழு delFlag){
  sqlite3_file *pReal = ((CrashFile*)pFile)->pRealFile;
  திரும்பு pReal->pMethods->xShmUnmap(pReal, delFlag);
}
நிலையான முழு cfShmMap(
  sqlite3_file *pFile,            /* Handle open on database file */
  முழு iRegion,                    /* Region to retrieve */
  முழு sz,                         /* Size of regions */
  முழு w,                          /* True to extend file if necessary */
  வெறுமை நிலையற்ற **pp              /* OUT: Mapped memory */
){
  sqlite3_file *pReal = ((CrashFile*)pFile)->pRealFile;
  திரும்பு pReal->pMethods->xShmMap(pReal, iRegion, sz, w, pp);
}

நிலையான மாறிலி sqlite3_io_methods CrashFileVtab = {
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
கட்டமைப்பு crashAppData {
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
நிலையான முழு cfOpen(
  sqlite3_vfs *pCfVfs,
  மாறிலி எழுத்து *zName,
  sqlite3_file *pFile,
  முழு flags,
  முழு *pOutFlags
){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  முழு rc;
  CrashFile *pWrapper = (CrashFile *)pFile;
  sqlite3_file *pReal = (sqlite3_file*)&pWrapper[1];

  memset(pWrapper, 0, அளவு(CrashFile));
  rc = sqlite3OsOpen(pVfs, zName, pReal, flags, pOutFlags);

  எனில்( rc==SQLITE_OK ){
    i64 iSize;
    pWrapper->pMethod = &CrashFileVtab;
    pWrapper->zName = (எழுத்து *)zName;
    pWrapper->pRealFile = pReal;
    rc = sqlite3OsFileSize(pReal, &iSize);
    pWrapper->iSize = (முழு)iSize;
    pWrapper->flags = flags;
  }
  எனில்( rc==SQLITE_OK ){
    pWrapper->nData = (முழு)(4096 + pWrapper->iSize);
    pWrapper->zData = crash_malloc(pWrapper->nData);
    எனில்( pWrapper->zData ){
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
      க்கு(iOff=0; iOff<pWrapper->iSize; iOff += 512){
        முழு nRead = (முழு)(pWrapper->iSize - iOff);
        எனில்( nRead>512 ) nRead = 512;
        rc = sqlite3OsRead(pReal, &pWrapper->zData[iOff], nRead, iOff);
      }
    }இல்லையெனில்{
      rc = SQLITE_NOMEM;
    }
  }
  எனில்( rc!=SQLITE_OK && pWrapper->pMethod ){
    sqlite3OsClose(pFile);
  }
  திரும்பு rc;
}

நிலையான முழு cfDelete(sqlite3_vfs *pCfVfs, மாறிலி எழுத்து *zPath, முழு dirSync){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  திரும்பு pVfs->xDelete(pVfs, zPath, dirSync);
}
நிலையான முழு cfAccess(
  sqlite3_vfs *pCfVfs,
  மாறிலி எழுத்து *zPath,
  முழு flags,
  முழு *pResOut
){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  திரும்பு pVfs->xAccess(pVfs, zPath, flags, pResOut);
}
நிலையான முழு cfFullPathname(
  sqlite3_vfs *pCfVfs,
  மாறிலி எழுத்து *zPath,
  முழு nPathOut,
  எழுத்து *zPathOut
){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  திரும்பு pVfs->xFullPathname(pVfs, zPath, nPathOut, zPathOut);
}
நிலையான வெறுமை *cfDlOpen(sqlite3_vfs *pCfVfs, மாறிலி எழுத்து *zPath){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  திரும்பு pVfs->xDlOpen(pVfs, zPath);
}
நிலையான வெறுமை cfDlError(sqlite3_vfs *pCfVfs, முழு nByte, எழுத்து *z_Err_செய்தி){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  pVfs->xDlError(pVfs, nByte, z_Err_செய்தி);
}
நிலையான வெறுமை (*cfDlSym(sqlite3_vfs *pCfVfs, வெறுமை *pH, மாறிலி எழுத்து *zSym))(வெறுமை){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  திரும்பு pVfs->xDlSym(pVfs, pH, zSym);
}
நிலையான வெறுமை cfDlClose(sqlite3_vfs *pCfVfs, வெறுமை *pHandle){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  pVfs->xDlClose(pVfs, pHandle);
}
நிலையான முழு cfRandomness(sqlite3_vfs *pCfVfs, முழு nByte, எழுத்து *zBufOut){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  திரும்பு pVfs->xRandomness(pVfs, nByte, zBufOut);
}
நிலையான முழு cfSleep(sqlite3_vfs *pCfVfs, முழு nMicro){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  திரும்பு pVfs->xSleep(pVfs, nMicro);
}
நிலையான முழு cfCurrentTime(sqlite3_vfs *pCfVfs, இரட்டை *pTimeOut){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  திரும்பு pVfs->xCurrentTime(pVfs, pTimeOut);
}
நிலையான முழு cfGetLastError(sqlite3_vfs *pCfVfs, முழு n, எழுத்து *z){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  திரும்பு pVfs->xGetLastError(pVfs, n, z);
}

நிலையான முழு processDevSymArgs(
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[],
  முழு *piDeviceChar,
  முழு *piSectorSize
){
  கட்டமைப்பு DeviceFlag {
    எழுத்து *zName;
    முழு i_மதிப்பு;
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

  முழு i;
  முழு iDc = 0;
  முழு iSectorSize = 0;
  முழு setSectorsize = 0;
  முழு setDeviceChar = 0;

  க்கு(i=0; i<objc; i+=2){
    Tcl_Size nOpt;
    எழுத்து *zOpt = Tcl_GetStringFromObj(objv[i], &nOpt);

    எனில்( (nOpt>11 || nOpt<2 || strncmp("-sectorsize", zOpt, nOpt))
     && (nOpt>16 || nOpt<2 || strncmp("-characteristics", zOpt, nOpt))
    ){
      Tcl_AppendResult(interp,
        "Bad option: \"", zOpt,
        "\" - must be \"-characteristics\" or \"-sectorsize\"", வெற்று
      );
      திரும்பு TCL_ERROR;
    }
    எனில்( i==objc-1 ){
      Tcl_AppendResult(interp, "Option requires an argument: \"", zOpt, "\"", வெற்று);
      திரும்பு TCL_ERROR;
    }

    எனில்( zOpt[1]=='s' ){
      எனில்( Tcl_GetIntFromObj(interp, objv[i+1], &iSectorSize) ){
        திரும்பு TCL_ERROR;
      }
      setSectorsize = 1;
    }இல்லையெனில்{
      முழு j;
      Tcl_Obj **apObj;
      Tcl_Size nObj;
      எனில்( Tcl_ListObjGetElements(interp, objv[i+1], &nObj, &apObj) ){
        திரும்பு TCL_ERROR;
      }
      க்கு(j=0; j<(முழு)nObj; j++){
        முழு rc;
        முழு iChoice;
        Tcl_Obj *pFlag = Tcl_DuplicateObj(apObj[j]);
        Tcl_Incr_Ref_மொத்தம்(pFlag);
        Tcl_UtfToLower(Tcl_GetString(pFlag));

        rc = Tcl_GetIndexFromObjStruct(
            interp, pFlag, aFlag, அளவு(aFlag[0]), "no such flag", 0, &iChoice
        );
        Tcl_Decr_Ref_மொத்தம்(pFlag);
        எனில்( rc ){
          திரும்பு TCL_ERROR;
        }

        iDc |= aFlag[iChoice].i_மதிப்பு;
      }
      setDeviceChar = 1;
    }
  }

  எனில்( setDeviceChar ){
    *piDeviceChar = iDc;
  }
  எனில்( setSectorsize ){
    *piSectorSize = iSectorSize;
  }

  திரும்பு TCL_OK;
}

/*
** tclcmd:   sqlite3_crash_now
**
** Simulate a crash immediately. This function does not return
** (writeListSync() calls exit(-1)).
*/
நிலையான முழு SQLITE_TCLAPI crashNowCmd(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  எனில்( objc!=1 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "");
    திரும்பு TCL_ERROR;
  }
  writeListSync(0, 1);
  ப்ரதிஜ்ஞா( 0 );
  திரும்பு TCL_OK;
}

/*
** tclcmd:   sqlite_crash_enable ENABLE ?DEFAULT?
**
** Parameter ENABLE must be a boolean value. If true, then the "crash"
** vfs is added to the system. If false, it is removed.
*/
நிலையான முழு SQLITE_TCLAPI crashEnableCmd(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  முழு isEnable;
  முழு isDefault = 0;
  நிலையான sqlite3_vfs crashVfs = {
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

  எனில்( objc!=2 && objc!=3 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "ENABLE ?DEFAULT?");
    திரும்பு TCL_ERROR;
  }

  எனில்( Tcl_GetBooleanFromObj(interp, objv[1], &isEnable) ){
    திரும்பு TCL_ERROR;
  }
  எனில்( objc==3 && Tcl_GetBooleanFromObj(interp, objv[2], &isDefault) ){
    திரும்பு TCL_ERROR;
  }

  எனில்( (isEnable && crashVfs.pAppData) || (!isEnable && !crashVfs.pAppData) ){
    திரும்பு TCL_OK;
  }

  எனில்( crashVfs.pAppData==0 ){
    sqlite3_vfs *pOriginalVfs = sqlite3_vfs_find(0);
    crashVfs.mxPathname = pOriginalVfs->mxPathname;
    crashVfs.pAppData = (வெறுமை *)pOriginalVfs;
    crashVfs.szOsFile = அளவு(CrashFile) + pOriginalVfs->szOsFile;
    sqlite3_vfs_register(&crashVfs, isDefault);
  }இல்லையெனில்{
    crashVfs.pAppData = 0;
    sqlite3_vfs_unregister(&crashVfs);
  }

  திரும்பு TCL_OK;
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
நிலையான முழு SQLITE_TCLAPI crashParamsObjCmd(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  முழு iDelay;
  மாறிலி எழுத்து *zCrashFile;
  Tcl_Size nCrashFile;
  முழு iDc, iSectorSize;

  iDc = -1;
  iSectorSize = -1;

  எனில்( objc<3 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "?OPTIONS? DELAY CRASHFILE");
    செல் error;
  }

  zCrashFile = Tcl_GetStringFromObj(objv[objc-1], &nCrashFile);
  எனில்( nCrashFile>=அளவு(g.zCrashFile) ){
    Tcl_AppendResult(interp, "Filename is too long: \"", zCrashFile, "\"", வெற்று);
    செல் error;
  }
  எனில்( Tcl_GetIntFromObj(interp, objv[objc-2], &iDelay) ){
    செல் error;
  }

  எனில்( processDevSymArgs(interp, objc-3, &objv[1], &iDc, &iSectorSize) ){
    திரும்பு TCL_ERROR;
  }

  எனில்( iDc>=0 ){
    g.iDeviceCharacteristics = iDc;
  }
  எனில்( iSectorSize>=0 ){
    g.iSectorSize = iSectorSize;
  }

  g.iCrash = iDelay;
  memcpy(g.zCrashFile, zCrashFile, nCrashFile+1);
  sqlite3CrashTestEnable = 1;
  திரும்பு TCL_OK;

error:
  திரும்பு TCL_ERROR;
}

நிலையான முழு SQLITE_TCLAPI devSymObjCmd(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  வெறுமை devsym_register(முழு iDeviceChar, முழு iSectorSize);

  முழு iDc = -1;
  முழு iSectorSize = -1;

  எனில்( processDevSymArgs(interp, objc-1, &objv[1], &iDc, &iSectorSize) ){
    திரும்பு TCL_ERROR;
  }
  devsym_register(iDc, iSectorSize);

  திரும்பு TCL_OK;
}

/*
** tclcmd: sqlite3_crash_on_write N
*/
நிலையான முழு SQLITE_TCLAPI writeCrashObjCmd(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  வெறுமை devsym_crash_on_write(முழு);
  முழு nWrite = 0;

  எனில்( objc!=2 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "NWRITE");
    திரும்பு TCL_ERROR;
  }
  எனில்( Tcl_GetIntFromObj(interp, objv[1], &nWrite) ){
    திரும்பு TCL_ERROR;
  }

  devsym_crash_on_write(nWrite);
  திரும்பு TCL_OK;
}

/*
** tclcmd: unregister_devsim
*/
நிலையான முழு SQLITE_TCLAPI dsUnregisterObjCmd(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  வெறுமை devsym_unregister(வெறுமை);

  எனில்( objc!=1 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "");
    திரும்பு TCL_ERROR;
  }

  devsym_unregister();
  திரும்பு TCL_OK;
}

/*
** tclcmd: register_jt_vfs ?-default? PARENT-VFS
*/
நிலையான முழு SQLITE_TCLAPI jtObjCmd(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  முழு jt_register(எழுத்து *, முழு);
  எழுத்து *zParent = 0;

  எனில்( objc!=2 && objc!=3 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "?-default? PARENT-VFS");
    திரும்பு TCL_ERROR;
  }
  zParent = Tcl_GetString(objv[1]);
  எனில்( objc==3 ){
    எனில்( strcmp(zParent, "-default") ){
      Tcl_AppendResult(interp,
          "bad option \"", zParent, "\": must be -default", வெற்று
      );
      திரும்பு TCL_ERROR;
    }
    zParent = Tcl_GetString(objv[2]);
  }

  எனில்( !(*zParent) ){
    zParent = 0;
  }
  எனில்( jt_register(zParent, objc==3) ){
    Tcl_AppendResult(interp, "Error in jt_register", வெற்று);
    திரும்பு TCL_ERROR;
  }

  திரும்பு TCL_OK;
}

/*
** tclcmd: unregister_jt_vfs
*/
நிலையான முழு SQLITE_TCLAPI jtUnregisterObjCmd(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  வெறுமை jt_unregister(வெறுமை);

  எனில்( objc!=1 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "");
    திரும்பு TCL_ERROR;
  }

  jt_unregister();
  திரும்பு TCL_OK;
}

#endif /* SQLITE_OMIT_DISKIO */

/*
** This procedure registers the TCL procedures defined in this file.
*/
முழு Sqlitetest6_Init(Tcl_Interp *interp){
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
  திரும்பு TCL_OK;
}

#endif /* SQLITE_TEST */
