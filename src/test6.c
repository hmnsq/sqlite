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
#જો SQLITE_TEST          /* This file is used for testing only */
#અન્તર્ભાવ "sqliteInt.h"
#અન્તર્ભાવ "tclsqlite.h"

#ifndef SQLITE_OMIT_DISKIO  /* This file is a no-op if disk I/O is disabled */

/* #define TRACE_CRASHTEST */

પ્રકારવ્યાખ્યા સંરચના CrashFile CrashFile;
પ્રકારવ્યાખ્યા સંરચના CrashGlobal CrashGlobal;
પ્રકારવ્યાખ્યા સંરચના WriteBuffer WriteBuffer;

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
સંરચના WriteBuffer {
  i64 iOffset;                 /* Byte offset of the start of this write() */
  પૂર્ણાંક nBuf;                    /* Number of bytes written */
  u8 *zBuf;                    /* Pointer to copy of written data */
  CrashFile *pFile;            /* File this write() applies to */

  WriteBuffer *pNext;          /* Next in CrashGlobal.pWriteList */
};

સંરચના CrashFile {
  અચલ sqlite3_io_methods *pMethod;   /* Must be first */
  sqlite3_file *pRealFile;             /* Underlying "real" file handle */
  અક્ષર *zName;
  પૂર્ણાંક flags;                           /* Flags the file was opened with */

  /* Cache of the entire file. This is used to speed up OsRead() and
  ** OsFileSize() calls. Although both could be done by traversing the
  ** write-list, in practice this is impractically slow.
  */
  u8 *zData;                           /* Buffer containing file contents */
  પૂર્ણાંક nData;                           /* Size of buffer allocated at zData */
  i64 iSize;                           /* Size of file in bytes */
};

સંરચના CrashGlobal {
  WriteBuffer *pWriteList;     /* Head of write-list */
  WriteBuffer *pWriteListEnd;  /* End of write-list */

  પૂર્ણાંક iSectorSize;             /* Value of simulated sector size */
  પૂર્ણાંક iDeviceCharacteristics;  /* Value of simulated device characteristics */

  પૂર્ણાંક iCrash;                  /* Crash on the iCrash'th call to xSync() */
  અક્ષર zCrashFile[500];        /* Crash during an xSync() on this file */
};

સ્થિર CrashGlobal g = {0, 0, SQLITE_DEFAULT_SECTOR_SIZE, 0, 0};

/*
** Set this global variable to 1 to enable crash testing.
*/
સ્થિર પૂર્ણાંક sqlite3CrashTestEnable = 0;

સ્થિર શૂન્ય *crash_malloc(પૂર્ણાંક nByte){
  પાછા (શૂન્ય *)Tcl_AttemptAlloc((size_t)nByte);
}
સ્થિર શૂન્ય crash_free(શૂન્ય *p){
  Tcl_Free(p);
}
સ્થિર શૂન્ય *crash_realloc(શૂન્ય *p, પૂર્ણાંક n){
  પાછા (શૂન્ય *)Tcl_AttemptRealloc(p, (size_t)n);
}

/*
** Wrapper around the sqlite3OsWrite() function that avoids writing to the
** 512 byte block beginning at offset PENDING_BYTE.
*/
સ્થિર પૂર્ણાંક writeDbFile(CrashFile *p, u8 *z, i64 iAmt, i64 iOff){
  પૂર્ણાંક rc = SQLITE_OK;
  પૂર્ણાંક iSkip = 0;
  જો( (iAmt-iSkip)>0 ){
    rc = sqlite3OsWrite(p->pRealFile, &z[iSkip], (પૂર્ણાંક)(iAmt-iSkip), iOff+iSkip);
  }
  પાછા rc;
}

/*
** Flush the write-list as if xSync() had been called on file handle
** pFile. If isCrash is true, simulate a crash.
*/
સ્થિર પૂર્ણાંક writeListSync(CrashFile *pFile, પૂર્ણાંક isCrash){
  પૂર્ણાંક rc = SQLITE_OK;
  પૂર્ણાંક iDc = g.iDeviceCharacteristics;

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
  જો( !isCrash ){
    માટે(pWrite=g.pWriteList; pWrite; pWrite=pWrite->pNext){
      જો( pWrite->pFile==pFile ){
        pFinal = pWrite;
      }
    }
  }નહીંતર જો( iDc&(SQLITE_IOCAP_SEQUENTIAL|SQLITE_IOCAP_SAFE_APPEND) ){
    પૂર્ણાંક nWrite = 0;
    પૂર્ણાંક iFinal;
    માટે(pWrite=g.pWriteList; pWrite; pWrite=pWrite->pNext) nWrite++;
    sqlite3_randomness(કદ(પૂર્ણાંક), &iFinal);
    iFinal = ((iFinal<0)?-1*iFinal:iFinal)%nWrite;
    માટે(pWrite=g.pWriteList; iFinal>0; pWrite=pWrite->pNext) iFinal--;
    pFinal = pWrite;
  }

#ifdef TRACE_CRASHTEST
  જો( pFile ){
    printf("Sync %s (is %s crash)\n", pFile->zName, (isCrash?"a":"not a"));
  }
#endif

  ppPtr = &g.pWriteList;
  માટે(pWrite=*ppPtr; rc==SQLITE_OK && pWrite; pWrite=*ppPtr){
    sqlite3_file *pRealFile = pWrite->pFile->pRealFile;

    /* (eAction==1)      -> write block out normally,
    ** (eAction==2)      -> do nothing,
    ** (eAction==3)      -> trash sectors.
    */
    પૂર્ણાંક eAction = 0;
    જો( !isCrash ){
      eAction = 2;
      જો( (pWrite->pFile==pFile || iDc&SQLITE_IOCAP_SEQUENTIAL) ){
        eAction = 1;
      }
    }નહીંતર{
      અક્ષર random;
      sqlite3_randomness(1, &random);

      /* Do not select option 3 (sector trashing) if the IOCAP_ATOMIC flag
      ** is set or this is an OsTruncate(), not an Oswrite().
      */
      જો( (iDc&SQLITE_IOCAP_ATOMIC) || (pWrite->zBuf==0) ){
        random &= 0x01;
      }

      /* If IOCAP_SEQUENTIAL is set and this is not the final entry
      ** in the truncated write-list, always select option 1 (write
      ** out correctly).
      */
      જો( (iDc&SQLITE_IOCAP_SEQUENTIAL && pWrite!=pFinal) ){
        random = 0;
      }

      /* If IOCAP_SAFE_APPEND is set and this OsWrite() operation is
      ** an append (first byte of the written region is 1 byte past the
      ** current EOF), always select option 1 (write out correctly).
      */
      જો( iDc&SQLITE_IOCAP_SAFE_APPEND && pWrite->zBuf ){
        i64 iSize;
        sqlite3OsFileSize(pRealFile, &iSize);
        જો( iSize==pWrite->iOffset ){
          random = 0;
        }
      }

      જો( (random&0x06)==0x06 ){
        eAction = 3;
      }નહીંતર{
        eAction = ((random&0x01)?2:1);
      }
    }

    પસંદગી( eAction ){
      સ્થિતિ 1: {               /* Write out correctly */
        જો( pWrite->zBuf ){
          rc = writeDbFile(
              pWrite->pFile, pWrite->zBuf, pWrite->nBuf, pWrite->iOffset
          );
        }નહીંતર{
          rc = sqlite3OsTruncate(pRealFile, pWrite->iOffset);
        }
        *ppPtr = pWrite->pNext;
#ifdef TRACE_CRASHTEST
        જો( isCrash ){
          printf("Writing %d bytes @ %d (%s)\n",
            pWrite->nBuf, (પૂર્ણાંક)pWrite->iOffset, pWrite->pFile->zName
          );
        }
#endif
        crash_free(pWrite);
        તોડો;
      }
      સ્થિતિ 2: {               /* Do nothing */
        ppPtr = &pWrite->pNext;
#ifdef TRACE_CRASHTEST
        જો( isCrash ){
          printf("Omiting %d bytes @ %d (%s)\n",
            pWrite->nBuf, (પૂર્ણાંક)pWrite->iOffset, pWrite->pFile->zName
          );
        }
#endif
        તોડો;
      }
      સ્થિતિ 3: {               /* Trash sectors */
        u8 *zGarbage;
        પૂર્ણાંક iFirst = (પૂર્ણાંક)(pWrite->iOffset/g.iSectorSize);
        પૂર્ણાંક iLast = (પૂર્ણાંક)((pWrite->iOffset+pWrite->nBuf-1)/g.iSectorSize);

        પ્રતિજ્ઞા(pWrite->zBuf);

#ifdef TRACE_CRASHTEST
        printf("Trashing %d sectors (%d bytes) @ %lld (sector %d) (%s)\n",
            1+iLast-iFirst, (1+iLast-iFirst)*g.iSectorSize,
            pWrite->iOffset, iFirst, pWrite->pFile->zName
        );
#endif

        zGarbage = crash_malloc(g.iSectorSize);
        જો( zGarbage ){
          sqlite3_int64 i;
          માટે(i=iFirst; rc==SQLITE_OK && i<=iLast; i++){
            sqlite3_randomness(g.iSectorSize, zGarbage);
            rc = writeDbFile(
              pWrite->pFile, zGarbage, g.iSectorSize, i*g.iSectorSize
            );
          }
          crash_free(zGarbage);
        }નહીંતર{
          rc = SQLITE_NOMEM;
        }

        ppPtr = &pWrite->pNext;
        તોડો;
      }

      મૂળભૂત:
        પ્રતિજ્ઞા(!"Cannot happen");
    }

    જો( pWrite==pFinal ) તોડો;
  }

  જો( rc==SQLITE_OK && isCrash ){
    exit(-1);
  }

  માટે(pWrite=g.pWriteList; pWrite && pWrite->pNext; pWrite=pWrite->pNext);
  g.pWriteListEnd = pWrite;

  પાછા rc;
}

/*
** Add an entry to the end of the write-list.
*/
સ્થિર પૂર્ણાંક writeListAppend(
  sqlite3_file *pFile,
  sqlite3_int64 iOffset,
  અચલ u8 *zBuf,
  પૂર્ણાંક nBuf
){
  WriteBuffer *pNew;

  પ્રતિજ્ઞા((zBuf && nBuf) || (!nBuf && !zBuf));

  pNew = (WriteBuffer *)crash_malloc(કદ(WriteBuffer) + nBuf);
  જો( pNew==0 ){
    fprintf(stderr, "out of memory in the crash simulator\n");
  }
  memset(pNew, 0, કદ(WriteBuffer)+nBuf);
  pNew->iOffset = iOffset;
  pNew->nBuf = nBuf;
  pNew->pFile = (CrashFile *)pFile;
  જો( zBuf ){
    pNew->zBuf = (u8 *)&pNew[1];
    memcpy(pNew->zBuf, zBuf, nBuf);
  }

  જો( g.pWriteList ){
    પ્રતિજ્ઞા(g.pWriteListEnd);
    g.pWriteListEnd->pNext = pNew;
  }નહીંતર{
    g.pWriteList = pNew;
  }
  g.pWriteListEnd = pNew;
 
  પાછા SQLITE_OK;
}

/*
** Close a crash-file.
*/
સ્થિર પૂર્ણાંક cfClose(sqlite3_file *pFile){
  CrashFile *pCrash = (CrashFile *)pFile;
  writeListSync(pCrash, 0);
  sqlite3OsClose(pCrash->pRealFile);
  પાછા SQLITE_OK;
}

/*
** Read data from a crash-file.
*/
સ્થિર પૂર્ણાંક cfRead(
  sqlite3_file *pFile,
  શૂન્ય *zBuf,
  પૂર્ણાંક iAmt,
  sqlite_int64 iOfst
){
  CrashFile *pCrash = (CrashFile *)pFile;
  પૂર્ણાંક nCopy = (પૂર્ણાંક)MIN((i64)iAmt, (pCrash->iSize - iOfst));

  જો( nCopy>0 ){
    memcpy(zBuf, &pCrash->zData[iOfst], nCopy);
  }

  /* Check the file-size to see if this is a short-read */
  જો( nCopy<iAmt ){
    પાછા SQLITE_IOERR_SHORT_READ;
  }

  પાછા SQLITE_OK;
}

/*
** Write data to a crash-file.
*/
સ્થિર પૂર્ણાંક cfWrite(
  sqlite3_file *pFile,
  અચલ શૂન્ય *zBuf,
  પૂર્ણાંક iAmt,
  sqlite_int64 iOfst
){
  CrashFile *pCrash = (CrashFile *)pFile;
  જો( iAmt+iOfst>pCrash->iSize ){
    pCrash->iSize = (પૂર્ણાંક)(iAmt+iOfst);
  }
  જ્યારે( pCrash->iSize>pCrash->nData ){
    u8 *zNew;
    પૂર્ણાંક nNew = (pCrash->nData*2) + 4096;
    zNew = crash_realloc(pCrash->zData, nNew);
    જો( !zNew ){
      પાછા SQLITE_NOMEM;
    }
    memset(&zNew[pCrash->nData], 0, nNew-pCrash->nData);
    pCrash->nData = nNew;
    pCrash->zData = zNew;
  }
  memcpy(&pCrash->zData[iOfst], zBuf, iAmt);
  પાછા writeListAppend(pFile, iOfst, zBuf, iAmt);
}

/*
** Truncate a crash-file.
*/
સ્થિર પૂર્ણાંક cfTruncate(sqlite3_file *pFile, sqlite_int64 size){
  CrashFile *pCrash = (CrashFile *)pFile;
  પ્રતિજ્ઞા(size>=0);
  જો( pCrash->iSize>size ){
    pCrash->iSize = (પૂર્ણાંક)size;
  }
  પાછા writeListAppend(pFile, size, 0, 0);
}

/*
** Sync a crash-file.
*/
સ્થિર પૂર્ણાંક cfSync(sqlite3_file *pFile, પૂર્ણાંક flags){
  CrashFile *pCrash = (CrashFile *)pFile;
  પૂર્ણાંક isCrash = 0;

  અચલ અક્ષર *zName = pCrash->zName;
  અચલ અક્ષર *zCrashFile = g.zCrashFile;
  પૂર્ણાંક nName = (પૂર્ણાંક)strlen(zName);
  પૂર્ણાંક nCrashFile = (પૂર્ણાંક)strlen(zCrashFile);

  જો( nCrashFile>0 && zCrashFile[nCrashFile-1]=='*' ){
    nCrashFile--;
    જો( nName>nCrashFile ) nName = nCrashFile;
  }

#ifdef TRACE_CRASHTEST
  printf("cfSync(): nName = %d, nCrashFile = %d, zName = %s, zCrashFile = %s\n",
         nName, nCrashFile, zName, zCrashFile);
#endif

  જો( nName==nCrashFile && 0==memcmp(zName, zCrashFile, nName) ){
#ifdef TRACE_CRASHTEST
    printf("cfSync(): name matched, g.iCrash = %d\n", g.iCrash);
#endif
    જો( (--g.iCrash)==0 ) isCrash = 1;
  }

  પાછા writeListSync(pCrash, isCrash);
}

/*
** Return the current file-size of the crash-file.
*/
સ્થિર પૂર્ણાંક cfFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  CrashFile *pCrash = (CrashFile *)pFile;
  *pSize = (i64)pCrash->iSize;
  પાછા SQLITE_OK;
}

/*
** Calls related to file-locks are passed on to the real file handle.
*/
સ્થિર પૂર્ણાંક cfLock(sqlite3_file *pFile, પૂર્ણાંક eLock){
  પાછા sqlite3OsLock(((CrashFile *)pFile)->pRealFile, eLock);
}
સ્થિર પૂર્ણાંક cfUnlock(sqlite3_file *pFile, પૂર્ણાંક eLock){
  પાછા sqlite3OsUnlock(((CrashFile *)pFile)->pRealFile, eLock);
}
સ્થિર પૂર્ણાંક cfCheckReservedLock(sqlite3_file *pFile, પૂર્ણાંક *pResOut){
  પાછા sqlite3OsCheckReservedLock(((CrashFile *)pFile)->pRealFile, pResOut);
}
સ્થિર પૂર્ણાંક cfFileControl(sqlite3_file *pFile, પૂર્ણાંક op, શૂન્ય *pArg){
  જો( op==SQLITE_FCNTL_SIZE_HINT ){
    CrashFile *pCrash = (CrashFile *)pFile;
    i64 nByte = *(i64 *)pArg;
    જો( nByte>pCrash->iSize ){
      જો( SQLITE_OK==writeListAppend(pFile, nByte, 0, 0) ){
        pCrash->iSize = (પૂર્ણાંક)nByte;
      }
    }
    પાછા SQLITE_OK;
  }
  પાછા sqlite3OsFileControl(((CrashFile *)pFile)->pRealFile, op, pArg);
}

/*
** The xSectorSize() and xDeviceCharacteristics() functions return
** the global values configured by the [sqlite_crashparams] tcl
*  interface.
*/
સ્થિર પૂર્ણાંક cfSectorSize(sqlite3_file *pFile){
  પાછા g.iSectorSize;
}
સ્થિર પૂર્ણાંક cfDeviceCharacteristics(sqlite3_file *pFile){
  પાછા g.iDeviceCharacteristics;
}

/*
** Pass-throughs for WAL support.
*/
સ્થિર પૂર્ણાંક cfShmLock(sqlite3_file *pFile, પૂર્ણાંક ofst, પૂર્ણાંક n, પૂર્ણાંક flags){
  sqlite3_file *pReal = ((CrashFile*)pFile)->pRealFile;
  પાછા pReal->pMethods->xShmLock(pReal, ofst, n, flags);
}
સ્થિર શૂન્ય cfShmBarrier(sqlite3_file *pFile){
  sqlite3_file *pReal = ((CrashFile*)pFile)->pRealFile;
  pReal->pMethods->xShmBarrier(pReal);
}
સ્થિર પૂર્ણાંક cfShmUnmap(sqlite3_file *pFile, પૂર્ણાંક delFlag){
  sqlite3_file *pReal = ((CrashFile*)pFile)->pRealFile;
  પાછા pReal->pMethods->xShmUnmap(pReal, delFlag);
}
સ્થિર પૂર્ણાંક cfShmMap(
  sqlite3_file *pFile,            /* Handle open on database file */
  પૂર્ણાંક iRegion,                    /* Region to retrieve */
  પૂર્ણાંક sz,                         /* Size of regions */
  પૂર્ણાંક w,                          /* True to extend file if necessary */
  શૂન્ય અસ્થિર **pp              /* OUT: Mapped memory */
){
  sqlite3_file *pReal = ((CrashFile*)pFile)->pRealFile;
  પાછા pReal->pMethods->xShmMap(pReal, iRegion, sz, w, pp);
}

સ્થિર અચલ sqlite3_io_methods CrashFileVtab = {
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
સંરચના crashAppData {
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
સ્થિર પૂર્ણાંક cfOpen(
  sqlite3_vfs *pCfVfs,
  અચલ અક્ષર *zName,
  sqlite3_file *pFile,
  પૂર્ણાંક flags,
  પૂર્ણાંક *pOutFlags
){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  પૂર્ણાંક rc;
  CrashFile *pWrapper = (CrashFile *)pFile;
  sqlite3_file *pReal = (sqlite3_file*)&pWrapper[1];

  memset(pWrapper, 0, કદ(CrashFile));
  rc = sqlite3OsOpen(pVfs, zName, pReal, flags, pOutFlags);

  જો( rc==SQLITE_OK ){
    i64 iSize;
    pWrapper->pMethod = &CrashFileVtab;
    pWrapper->zName = (અક્ષર *)zName;
    pWrapper->pRealFile = pReal;
    rc = sqlite3OsFileSize(pReal, &iSize);
    pWrapper->iSize = (પૂર્ણાંક)iSize;
    pWrapper->flags = flags;
  }
  જો( rc==SQLITE_OK ){
    pWrapper->nData = (પૂર્ણાંક)(4096 + pWrapper->iSize);
    pWrapper->zData = crash_malloc(pWrapper->nData);
    જો( pWrapper->zData ){
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
      માટે(iOff=0; iOff<pWrapper->iSize; iOff += 512){
        પૂર્ણાંક nRead = (પૂર્ણાંક)(pWrapper->iSize - iOff);
        જો( nRead>512 ) nRead = 512;
        rc = sqlite3OsRead(pReal, &pWrapper->zData[iOff], nRead, iOff);
      }
    }નહીંતર{
      rc = SQLITE_NOMEM;
    }
  }
  જો( rc!=SQLITE_OK && pWrapper->pMethod ){
    sqlite3OsClose(pFile);
  }
  પાછા rc;
}

સ્થિર પૂર્ણાંક cfDelete(sqlite3_vfs *pCfVfs, અચલ અક્ષર *zPath, પૂર્ણાંક dirSync){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  પાછા pVfs->xDelete(pVfs, zPath, dirSync);
}
સ્થિર પૂર્ણાંક cfAccess(
  sqlite3_vfs *pCfVfs,
  અચલ અક્ષર *zPath,
  પૂર્ણાંક flags,
  પૂર્ણાંક *pResOut
){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  પાછા pVfs->xAccess(pVfs, zPath, flags, pResOut);
}
સ્થિર પૂર્ણાંક cfFullPathname(
  sqlite3_vfs *pCfVfs,
  અચલ અક્ષર *zPath,
  પૂર્ણાંક nPathOut,
  અક્ષર *zPathOut
){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  પાછા pVfs->xFullPathname(pVfs, zPath, nPathOut, zPathOut);
}
સ્થિર શૂન્ય *cfDlOpen(sqlite3_vfs *pCfVfs, અચલ અક્ષર *zPath){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  પાછા pVfs->xDlOpen(pVfs, zPath);
}
સ્થિર શૂન્ય cfDlError(sqlite3_vfs *pCfVfs, પૂર્ણાંક nByte, અક્ષર *z_Err_સંદેશ){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  pVfs->xDlError(pVfs, nByte, z_Err_સંદેશ);
}
સ્થિર શૂન્ય (*cfDlSym(sqlite3_vfs *pCfVfs, શૂન્ય *pH, અચલ અક્ષર *zSym))(શૂન્ય){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  પાછા pVfs->xDlSym(pVfs, pH, zSym);
}
સ્થિર શૂન્ય cfDlClose(sqlite3_vfs *pCfVfs, શૂન્ય *pHandle){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  pVfs->xDlClose(pVfs, pHandle);
}
સ્થિર પૂર્ણાંક cfRandomness(sqlite3_vfs *pCfVfs, પૂર્ણાંક nByte, અક્ષર *zBufOut){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  પાછા pVfs->xRandomness(pVfs, nByte, zBufOut);
}
સ્થિર પૂર્ણાંક cfSleep(sqlite3_vfs *pCfVfs, પૂર્ણાંક nMicro){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  પાછા pVfs->xSleep(pVfs, nMicro);
}
સ્થિર પૂર્ણાંક cfCurrentTime(sqlite3_vfs *pCfVfs, દ્વિગુણ *pTimeOut){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  પાછા pVfs->xCurrentTime(pVfs, pTimeOut);
}
સ્થિર પૂર્ણાંક cfGetLastError(sqlite3_vfs *pCfVfs, પૂર્ણાંક n, અક્ષર *z){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  પાછા pVfs->xGetLastError(pVfs, n, z);
}

સ્થિર પૂર્ણાંક processDevSymArgs(
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[],
  પૂર્ણાંક *piDeviceChar,
  પૂર્ણાંક *piSectorSize
){
  સંરચના DeviceFlag {
    અક્ષર *zName;
    પૂર્ણાંક i_કિંમત;
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

  પૂર્ણાંક i;
  પૂર્ણાંક iDc = 0;
  પૂર્ણાંક iSectorSize = 0;
  પૂર્ણાંક setSectorsize = 0;
  પૂર્ણાંક setDeviceChar = 0;

  માટે(i=0; i<objc; i+=2){
    Tcl_Size nOpt;
    અક્ષર *zOpt = Tcl_GetStringFromObj(objv[i], &nOpt);

    જો( (nOpt>11 || nOpt<2 || strncmp("-sectorsize", zOpt, nOpt))
     && (nOpt>16 || nOpt<2 || strncmp("-characteristics", zOpt, nOpt))
    ){
      Tcl_AppendResult(interp,
        "Bad option: \"", zOpt,
        "\" - must be \"-characteristics\" or \"-sectorsize\"", ખાલી
      );
      પાછા TCL_ERROR;
    }
    જો( i==objc-1 ){
      Tcl_AppendResult(interp, "Option requires an argument: \"", zOpt, "\"", ખાલી);
      પાછા TCL_ERROR;
    }

    જો( zOpt[1]=='s' ){
      જો( Tcl_GetIntFromObj(interp, objv[i+1], &iSectorSize) ){
        પાછા TCL_ERROR;
      }
      setSectorsize = 1;
    }નહીંતર{
      પૂર્ણાંક j;
      Tcl_Obj **apObj;
      Tcl_Size nObj;
      જો( Tcl_ListObjGetElements(interp, objv[i+1], &nObj, &apObj) ){
        પાછા TCL_ERROR;
      }
      માટે(j=0; j<(પૂર્ણાંક)nObj; j++){
        પૂર્ણાંક rc;
        પૂર્ણાંક iChoice;
        Tcl_Obj *pFlag = Tcl_DuplicateObj(apObj[j]);
        Tcl_Incr_Ref_ગણતરી(pFlag);
        Tcl_UtfToLower(Tcl_GetString(pFlag));

        rc = Tcl_GetIndexFromObjStruct(
            interp, pFlag, aFlag, કદ(aFlag[0]), "no such flag", 0, &iChoice
        );
        Tcl_Decr_Ref_ગણતરી(pFlag);
        જો( rc ){
          પાછા TCL_ERROR;
        }

        iDc |= aFlag[iChoice].i_કિંમત;
      }
      setDeviceChar = 1;
    }
  }

  જો( setDeviceChar ){
    *piDeviceChar = iDc;
  }
  જો( setSectorsize ){
    *piSectorSize = iSectorSize;
  }

  પાછા TCL_OK;
}

/*
** tclcmd:   sqlite3_crash_now
**
** Simulate a crash immediately. This function does not return
** (writeListSync() calls exit(-1)).
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI crashNowCmd(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  જો( objc!=1 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "");
    પાછા TCL_ERROR;
  }
  writeListSync(0, 1);
  પ્રતિજ્ઞા( 0 );
  પાછા TCL_OK;
}

/*
** tclcmd:   sqlite_crash_enable ENABLE ?DEFAULT?
**
** Parameter ENABLE must be a boolean value. If true, then the "crash"
** vfs is added to the system. If false, it is removed.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI crashEnableCmd(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  પૂર્ણાંક isEnable;
  પૂર્ણાંક isDefault = 0;
  સ્થિર sqlite3_vfs crashVfs = {
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

  જો( objc!=2 && objc!=3 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "ENABLE ?DEFAULT?");
    પાછા TCL_ERROR;
  }

  જો( Tcl_GetBooleanFromObj(interp, objv[1], &isEnable) ){
    પાછા TCL_ERROR;
  }
  જો( objc==3 && Tcl_GetBooleanFromObj(interp, objv[2], &isDefault) ){
    પાછા TCL_ERROR;
  }

  જો( (isEnable && crashVfs.pAppData) || (!isEnable && !crashVfs.pAppData) ){
    પાછા TCL_OK;
  }

  જો( crashVfs.pAppData==0 ){
    sqlite3_vfs *pOriginalVfs = sqlite3_vfs_find(0);
    crashVfs.mxPathname = pOriginalVfs->mxPathname;
    crashVfs.pAppData = (શૂન્ય *)pOriginalVfs;
    crashVfs.szOsFile = કદ(CrashFile) + pOriginalVfs->szOsFile;
    sqlite3_vfs_register(&crashVfs, isDefault);
  }નહીંતર{
    crashVfs.pAppData = 0;
    sqlite3_vfs_unregister(&crashVfs);
  }

  પાછા TCL_OK;
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
સ્થિર પૂર્ણાંક SQLITE_TCLAPI crashParamsObjCmd(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  પૂર્ણાંક iDelay;
  અચલ અક્ષર *zCrashFile;
  Tcl_Size nCrashFile;
  પૂર્ણાંક iDc, iSectorSize;

  iDc = -1;
  iSectorSize = -1;

  જો( objc<3 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "?OPTIONS? DELAY CRASHFILE");
    જાઓ error;
  }

  zCrashFile = Tcl_GetStringFromObj(objv[objc-1], &nCrashFile);
  જો( nCrashFile>=કદ(g.zCrashFile) ){
    Tcl_AppendResult(interp, "Filename is too long: \"", zCrashFile, "\"", ખાલી);
    જાઓ error;
  }
  જો( Tcl_GetIntFromObj(interp, objv[objc-2], &iDelay) ){
    જાઓ error;
  }

  જો( processDevSymArgs(interp, objc-3, &objv[1], &iDc, &iSectorSize) ){
    પાછા TCL_ERROR;
  }

  જો( iDc>=0 ){
    g.iDeviceCharacteristics = iDc;
  }
  જો( iSectorSize>=0 ){
    g.iSectorSize = iSectorSize;
  }

  g.iCrash = iDelay;
  memcpy(g.zCrashFile, zCrashFile, nCrashFile+1);
  sqlite3CrashTestEnable = 1;
  પાછા TCL_OK;

error:
  પાછા TCL_ERROR;
}

સ્થિર પૂર્ણાંક SQLITE_TCLAPI devSymObjCmd(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  શૂન્ય devsym_register(પૂર્ણાંક iDeviceChar, પૂર્ણાંક iSectorSize);

  પૂર્ણાંક iDc = -1;
  પૂર્ણાંક iSectorSize = -1;

  જો( processDevSymArgs(interp, objc-1, &objv[1], &iDc, &iSectorSize) ){
    પાછા TCL_ERROR;
  }
  devsym_register(iDc, iSectorSize);

  પાછા TCL_OK;
}

/*
** tclcmd: sqlite3_crash_on_write N
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI writeCrashObjCmd(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  શૂન્ય devsym_crash_on_write(પૂર્ણાંક);
  પૂર્ણાંક nWrite = 0;

  જો( objc!=2 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "NWRITE");
    પાછા TCL_ERROR;
  }
  જો( Tcl_GetIntFromObj(interp, objv[1], &nWrite) ){
    પાછા TCL_ERROR;
  }

  devsym_crash_on_write(nWrite);
  પાછા TCL_OK;
}

/*
** tclcmd: unregister_devsim
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI dsUnregisterObjCmd(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  શૂન્ય devsym_unregister(શૂન્ય);

  જો( objc!=1 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "");
    પાછા TCL_ERROR;
  }

  devsym_unregister();
  પાછા TCL_OK;
}

/*
** tclcmd: register_jt_vfs ?-default? PARENT-VFS
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI jtObjCmd(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  પૂર્ણાંક jt_register(અક્ષર *, પૂર્ણાંક);
  અક્ષર *zParent = 0;

  જો( objc!=2 && objc!=3 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "?-default? PARENT-VFS");
    પાછા TCL_ERROR;
  }
  zParent = Tcl_GetString(objv[1]);
  જો( objc==3 ){
    જો( strcmp(zParent, "-default") ){
      Tcl_AppendResult(interp,
          "bad option \"", zParent, "\": must be -default", ખાલી
      );
      પાછા TCL_ERROR;
    }
    zParent = Tcl_GetString(objv[2]);
  }

  જો( !(*zParent) ){
    zParent = 0;
  }
  જો( jt_register(zParent, objc==3) ){
    Tcl_AppendResult(interp, "Error in jt_register", ખાલી);
    પાછા TCL_ERROR;
  }

  પાછા TCL_OK;
}

/*
** tclcmd: unregister_jt_vfs
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI jtUnregisterObjCmd(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  શૂન્ય jt_unregister(શૂન્ય);

  જો( objc!=1 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "");
    પાછા TCL_ERROR;
  }

  jt_unregister();
  પાછા TCL_OK;
}

#endif /* SQLITE_OMIT_DISKIO */

/*
** This procedure registers the TCL procedures defined in this file.
*/
પૂર્ણાંક Sqlitetest6_Init(Tcl_Interp *interp){
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
  પાછા TCL_OK;
}

#endif /* SQLITE_TEST */
