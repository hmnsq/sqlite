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
#ಯದಿ SQLITE_TEST          /* This file is used for testing only */
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"
#ಅನ್ತರ್ಭಾವ "tclsqlite.h"

#ifndef SQLITE_OMIT_DISKIO  /* This file is a no-op if disk I/O is disabled */

/* #define TRACE_CRASHTEST */

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ CrashFile CrashFile;
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ CrashGlobal CrashGlobal;
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ WriteBuffer WriteBuffer;

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
ಸಂರಚನಾ WriteBuffer {
  i64 iOffset;                 /* Byte offset of the start of this write() */
  ಪೂರ್ಣಾಙ್ಕ nBuf;                    /* Number of bytes written */
  u8 *zBuf;                    /* Pointer to copy of written data */
  CrashFile *pFile;            /* File this write() applies to */

  WriteBuffer *pNext;          /* Next in CrashGlobal.pWriteList */
};

ಸಂರಚನಾ CrashFile {
  ಧ್ರುವ sqlite3_io_methods *pMethod;   /* Must be first */
  sqlite3_file *pRealFile;             /* Underlying "real" file handle */
  ವರ್ಣ *zName;
  ಪೂರ್ಣಾಙ್ಕ flags;                           /* Flags the file was opened with */

  /* Cache of the entire file. This is used to speed up OsRead() and
  ** OsFileSize() calls. Although both could be done by traversing the
  ** write-list, in practice this is impractically slow.
  */
  u8 *zData;                           /* Buffer containing file contents */
  ಪೂರ್ಣಾಙ್ಕ nData;                           /* Size of buffer allocated at zData */
  i64 iSize;                           /* Size of file in bytes */
};

ಸಂರಚನಾ CrashGlobal {
  WriteBuffer *pWriteList;     /* Head of write-list */
  WriteBuffer *pWriteListEnd;  /* End of write-list */

  ಪೂರ್ಣಾಙ್ಕ iSectorSize;             /* Value of simulated sector size */
  ಪೂರ್ಣಾಙ್ಕ iDeviceCharacteristics;  /* Value of simulated device characteristics */

  ಪೂರ್ಣಾಙ್ಕ iCrash;                  /* Crash on the iCrash'th call to xSync() */
  ವರ್ಣ zCrashFile[500];        /* Crash during an xSync() on this file */
};

ಸ್ಥಿರ CrashGlobal g = {0, 0, SQLITE_DEFAULT_SECTOR_SIZE, 0, 0};

/*
** Set this global variable to 1 to enable crash testing.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ sqlite3CrashTestEnable = 0;

ಸ್ಥಿರ ಶೂನ್ಯ *crash_malloc(ಪೂರ್ಣಾಙ್ಕ nByte){
  ಪ್ರತ್ಯಾಗಮ (ಶೂನ್ಯ *)Tcl_AttemptAlloc((size_t)nByte);
}
ಸ್ಥಿರ ಶೂನ್ಯ crash_free(ಶೂನ್ಯ *p){
  Tcl_Free(p);
}
ಸ್ಥಿರ ಶೂನ್ಯ *crash_realloc(ಶೂನ್ಯ *p, ಪೂರ್ಣಾಙ್ಕ n){
  ಪ್ರತ್ಯಾಗಮ (ಶೂನ್ಯ *)Tcl_AttemptRealloc(p, (size_t)n);
}

/*
** Wrapper around the sqlite3OsWrite() function that avoids writing to the
** 512 byte block beginning at offset PENDING_BYTE.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ writeDbFile(CrashFile *p, u8 *z, i64 iAmt, i64 iOff){
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;
  ಪೂರ್ಣಾಙ್ಕ iSkip = 0;
  ಯದಿ( (iAmt-iSkip)>0 ){
    rc = sqlite3OsWrite(p->pRealFile, &z[iSkip], (ಪೂರ್ಣಾಙ್ಕ)(iAmt-iSkip), iOff+iSkip);
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Flush the write-list as if xSync() had been called on file handle
** pFile. If isCrash is true, simulate a crash.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ writeListSync(CrashFile *pFile, ಪೂರ್ಣಾಙ್ಕ isCrash){
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;
  ಪೂರ್ಣಾಙ್ಕ iDc = g.iDeviceCharacteristics;

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
  ಯದಿ( !isCrash ){
    ಪ್ರತಿ(pWrite=g.pWriteList; pWrite; pWrite=pWrite->pNext){
      ಯದಿ( pWrite->pFile==pFile ){
        pFinal = pWrite;
      }
    }
  }ಅನ್ಯಥಾ ಯದಿ( iDc&(SQLITE_IOCAP_SEQUENTIAL|SQLITE_IOCAP_SAFE_APPEND) ){
    ಪೂರ್ಣಾಙ್ಕ nWrite = 0;
    ಪೂರ್ಣಾಙ್ಕ iFinal;
    ಪ್ರತಿ(pWrite=g.pWriteList; pWrite; pWrite=pWrite->pNext) nWrite++;
    sqlite3_randomness(ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ), &iFinal);
    iFinal = ((iFinal<0)?-1*iFinal:iFinal)%nWrite;
    ಪ್ರತಿ(pWrite=g.pWriteList; iFinal>0; pWrite=pWrite->pNext) iFinal--;
    pFinal = pWrite;
  }

#ifdef TRACE_CRASHTEST
  ಯದಿ( pFile ){
    printf("Sync %s (is %s crash)\n", pFile->zName, (isCrash?"a":"not a"));
  }
#endif

  ppPtr = &g.pWriteList;
  ಪ್ರತಿ(pWrite=*ppPtr; rc==SQLITE_OK && pWrite; pWrite=*ppPtr){
    sqlite3_file *pRealFile = pWrite->pFile->pRealFile;

    /* (eAction==1)      -> write block out normally,
    ** (eAction==2)      -> do nothing,
    ** (eAction==3)      -> trash sectors.
    */
    ಪೂರ್ಣಾಙ್ಕ eAction = 0;
    ಯದಿ( !isCrash ){
      eAction = 2;
      ಯದಿ( (pWrite->pFile==pFile || iDc&SQLITE_IOCAP_SEQUENTIAL) ){
        eAction = 1;
      }
    }ಅನ್ಯಥಾ{
      ವರ್ಣ random;
      sqlite3_randomness(1, &random);

      /* Do not select option 3 (sector trashing) if the IOCAP_ATOMIC flag
      ** is set or this is an OsTruncate(), not an Oswrite().
      */
      ಯದಿ( (iDc&SQLITE_IOCAP_ATOMIC) || (pWrite->zBuf==0) ){
        random &= 0x01;
      }

      /* If IOCAP_SEQUENTIAL is set and this is not the final entry
      ** in the truncated write-list, always select option 1 (write
      ** out correctly).
      */
      ಯದಿ( (iDc&SQLITE_IOCAP_SEQUENTIAL && pWrite!=pFinal) ){
        random = 0;
      }

      /* If IOCAP_SAFE_APPEND is set and this OsWrite() operation is
      ** an append (first byte of the written region is 1 byte past the
      ** current EOF), always select option 1 (write out correctly).
      */
      ಯದಿ( iDc&SQLITE_IOCAP_SAFE_APPEND && pWrite->zBuf ){
        i64 iSize;
        sqlite3OsFileSize(pRealFile, &iSize);
        ಯದಿ( iSize==pWrite->iOffset ){
          random = 0;
        }
      }

      ಯದಿ( (random&0x06)==0x06 ){
        eAction = 3;
      }ಅನ್ಯಥಾ{
        eAction = ((random&0x01)?2:1);
      }
    }

    ವಿಕಲ್ಪ( eAction ){
      ಪಕ್ಷ 1: {               /* Write out correctly */
        ಯದಿ( pWrite->zBuf ){
          rc = writeDbFile(
              pWrite->pFile, pWrite->zBuf, pWrite->nBuf, pWrite->iOffset
          );
        }ಅನ್ಯಥಾ{
          rc = sqlite3OsTruncate(pRealFile, pWrite->iOffset);
        }
        *ppPtr = pWrite->pNext;
#ifdef TRACE_CRASHTEST
        ಯದಿ( isCrash ){
          printf("Writing %d bytes @ %d (%s)\n",
            pWrite->nBuf, (ಪೂರ್ಣಾಙ್ಕ)pWrite->iOffset, pWrite->pFile->zName
          );
        }
#endif
        crash_free(pWrite);
        ಭಙ್ಗ;
      }
      ಪಕ್ಷ 2: {               /* Do nothing */
        ppPtr = &pWrite->pNext;
#ifdef TRACE_CRASHTEST
        ಯದಿ( isCrash ){
          printf("Omiting %d bytes @ %d (%s)\n",
            pWrite->nBuf, (ಪೂರ್ಣಾಙ್ಕ)pWrite->iOffset, pWrite->pFile->zName
          );
        }
#endif
        ಭಙ್ಗ;
      }
      ಪಕ್ಷ 3: {               /* Trash sectors */
        u8 *zGarbage;
        ಪೂರ್ಣಾಙ್ಕ iFirst = (ಪೂರ್ಣಾಙ್ಕ)(pWrite->iOffset/g.iSectorSize);
        ಪೂರ್ಣಾಙ್ಕ iLast = (ಪೂರ್ಣಾಙ್ಕ)((pWrite->iOffset+pWrite->nBuf-1)/g.iSectorSize);

        ಪ್ರತಿಜ್ಞಾ(pWrite->zBuf);

#ifdef TRACE_CRASHTEST
        printf("Trashing %d sectors (%d bytes) @ %lld (sector %d) (%s)\n",
            1+iLast-iFirst, (1+iLast-iFirst)*g.iSectorSize,
            pWrite->iOffset, iFirst, pWrite->pFile->zName
        );
#endif

        zGarbage = crash_malloc(g.iSectorSize);
        ಯದಿ( zGarbage ){
          sqlite3_int64 i;
          ಪ್ರತಿ(i=iFirst; rc==SQLITE_OK && i<=iLast; i++){
            sqlite3_randomness(g.iSectorSize, zGarbage);
            rc = writeDbFile(
              pWrite->pFile, zGarbage, g.iSectorSize, i*g.iSectorSize
            );
          }
          crash_free(zGarbage);
        }ಅನ್ಯಥಾ{
          rc = SQLITE_NOMEM;
        }

        ppPtr = &pWrite->pNext;
        ಭಙ್ಗ;
      }

      ಸ್ವಭಾವ:
        ಪ್ರತಿಜ್ಞಾ(!"Cannot happen");
    }

    ಯದಿ( pWrite==pFinal ) ಭಙ್ಗ;
  }

  ಯದಿ( rc==SQLITE_OK && isCrash ){
    exit(-1);
  }

  ಪ್ರತಿ(pWrite=g.pWriteList; pWrite && pWrite->pNext; pWrite=pWrite->pNext);
  g.pWriteListEnd = pWrite;

  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Add an entry to the end of the write-list.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ writeListAppend(
  sqlite3_file *pFile,
  sqlite3_int64 iOffset,
  ಧ್ರುವ u8 *zBuf,
  ಪೂರ್ಣಾಙ್ಕ nBuf
){
  WriteBuffer *pNew;

  ಪ್ರತಿಜ್ಞಾ((zBuf && nBuf) || (!nBuf && !zBuf));

  pNew = (WriteBuffer *)crash_malloc(ಪರಿಮಾಣ(WriteBuffer) + nBuf);
  ಯದಿ( pNew==0 ){
    fprintf(stderr, "out of memory in the crash simulator\n");
  }
  memset(pNew, 0, ಪರಿಮಾಣ(WriteBuffer)+nBuf);
  pNew->iOffset = iOffset;
  pNew->nBuf = nBuf;
  pNew->pFile = (CrashFile *)pFile;
  ಯದಿ( zBuf ){
    pNew->zBuf = (u8 *)&pNew[1];
    memcpy(pNew->zBuf, zBuf, nBuf);
  }

  ಯದಿ( g.pWriteList ){
    ಪ್ರತಿಜ್ಞಾ(g.pWriteListEnd);
    g.pWriteListEnd->pNext = pNew;
  }ಅನ್ಯಥಾ{
    g.pWriteList = pNew;
  }
  g.pWriteListEnd = pNew;
 
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Close a crash-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ cfClose(sqlite3_file *pFile){
  CrashFile *pCrash = (CrashFile *)pFile;
  writeListSync(pCrash, 0);
  sqlite3OsClose(pCrash->pRealFile);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Read data from a crash-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ cfRead(
  sqlite3_file *pFile,
  ಶೂನ್ಯ *zBuf,
  ಪೂರ್ಣಾಙ್ಕ iAmt,
  sqlite_int64 iOfst
){
  CrashFile *pCrash = (CrashFile *)pFile;
  ಪೂರ್ಣಾಙ್ಕ nCopy = (ಪೂರ್ಣಾಙ್ಕ)MIN((i64)iAmt, (pCrash->iSize - iOfst));

  ಯದಿ( nCopy>0 ){
    memcpy(zBuf, &pCrash->zData[iOfst], nCopy);
  }

  /* Check the file-size to see if this is a short-read */
  ಯದಿ( nCopy<iAmt ){
    ಪ್ರತ್ಯಾಗಮ SQLITE_IOERR_SHORT_READ;
  }

  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Write data to a crash-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ cfWrite(
  sqlite3_file *pFile,
  ಧ್ರುವ ಶೂನ್ಯ *zBuf,
  ಪೂರ್ಣಾಙ್ಕ iAmt,
  sqlite_int64 iOfst
){
  CrashFile *pCrash = (CrashFile *)pFile;
  ಯದಿ( iAmt+iOfst>pCrash->iSize ){
    pCrash->iSize = (ಪೂರ್ಣಾಙ್ಕ)(iAmt+iOfst);
  }
  ಯಾವತ್( pCrash->iSize>pCrash->nData ){
    u8 *zNew;
    ಪೂರ್ಣಾಙ್ಕ nNew = (pCrash->nData*2) + 4096;
    zNew = crash_realloc(pCrash->zData, nNew);
    ಯದಿ( !zNew ){
      ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
    }
    memset(&zNew[pCrash->nData], 0, nNew-pCrash->nData);
    pCrash->nData = nNew;
    pCrash->zData = zNew;
  }
  memcpy(&pCrash->zData[iOfst], zBuf, iAmt);
  ಪ್ರತ್ಯಾಗಮ writeListAppend(pFile, iOfst, zBuf, iAmt);
}

/*
** Truncate a crash-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ cfTruncate(sqlite3_file *pFile, sqlite_int64 size){
  CrashFile *pCrash = (CrashFile *)pFile;
  ಪ್ರತಿಜ್ಞಾ(size>=0);
  ಯದಿ( pCrash->iSize>size ){
    pCrash->iSize = (ಪೂರ್ಣಾಙ್ಕ)size;
  }
  ಪ್ರತ್ಯಾಗಮ writeListAppend(pFile, size, 0, 0);
}

/*
** Sync a crash-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ cfSync(sqlite3_file *pFile, ಪೂರ್ಣಾಙ್ಕ flags){
  CrashFile *pCrash = (CrashFile *)pFile;
  ಪೂರ್ಣಾಙ್ಕ isCrash = 0;

  ಧ್ರುವ ವರ್ಣ *zName = pCrash->zName;
  ಧ್ರುವ ವರ್ಣ *zCrashFile = g.zCrashFile;
  ಪೂರ್ಣಾಙ್ಕ nName = (ಪೂರ್ಣಾಙ್ಕ)strlen(zName);
  ಪೂರ್ಣಾಙ್ಕ nCrashFile = (ಪೂರ್ಣಾಙ್ಕ)strlen(zCrashFile);

  ಯದಿ( nCrashFile>0 && zCrashFile[nCrashFile-1]=='*' ){
    nCrashFile--;
    ಯದಿ( nName>nCrashFile ) nName = nCrashFile;
  }

#ifdef TRACE_CRASHTEST
  printf("cfSync(): nName = %d, nCrashFile = %d, zName = %s, zCrashFile = %s\n",
         nName, nCrashFile, zName, zCrashFile);
#endif

  ಯದಿ( nName==nCrashFile && 0==memcmp(zName, zCrashFile, nName) ){
#ifdef TRACE_CRASHTEST
    printf("cfSync(): name matched, g.iCrash = %d\n", g.iCrash);
#endif
    ಯದಿ( (--g.iCrash)==0 ) isCrash = 1;
  }

  ಪ್ರತ್ಯಾಗಮ writeListSync(pCrash, isCrash);
}

/*
** Return the current file-size of the crash-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ cfFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  CrashFile *pCrash = (CrashFile *)pFile;
  *pSize = (i64)pCrash->iSize;
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Calls related to file-locks are passed on to the real file handle.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ cfLock(sqlite3_file *pFile, ಪೂರ್ಣಾಙ್ಕ eLock){
  ಪ್ರತ್ಯಾಗಮ sqlite3OsLock(((CrashFile *)pFile)->pRealFile, eLock);
}
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ cfUnlock(sqlite3_file *pFile, ಪೂರ್ಣಾಙ್ಕ eLock){
  ಪ್ರತ್ಯಾಗಮ sqlite3OsUnlock(((CrashFile *)pFile)->pRealFile, eLock);
}
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ cfCheckReservedLock(sqlite3_file *pFile, ಪೂರ್ಣಾಙ್ಕ *pResOut){
  ಪ್ರತ್ಯಾಗಮ sqlite3OsCheckReservedLock(((CrashFile *)pFile)->pRealFile, pResOut);
}
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ cfFileControl(sqlite3_file *pFile, ಪೂರ್ಣಾಙ್ಕ op, ಶೂನ್ಯ *pArg){
  ಯದಿ( op==SQLITE_FCNTL_SIZE_HINT ){
    CrashFile *pCrash = (CrashFile *)pFile;
    i64 nByte = *(i64 *)pArg;
    ಯದಿ( nByte>pCrash->iSize ){
      ಯದಿ( SQLITE_OK==writeListAppend(pFile, nByte, 0, 0) ){
        pCrash->iSize = (ಪೂರ್ಣಾಙ್ಕ)nByte;
      }
    }
    ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
  }
  ಪ್ರತ್ಯಾಗಮ sqlite3OsFileControl(((CrashFile *)pFile)->pRealFile, op, pArg);
}

/*
** The xSectorSize() and xDeviceCharacteristics() functions return
** the global values configured by the [sqlite_crashparams] tcl
*  interface.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ cfSectorSize(sqlite3_file *pFile){
  ಪ್ರತ್ಯಾಗಮ g.iSectorSize;
}
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ cfDeviceCharacteristics(sqlite3_file *pFile){
  ಪ್ರತ್ಯಾಗಮ g.iDeviceCharacteristics;
}

/*
** Pass-throughs for WAL support.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ cfShmLock(sqlite3_file *pFile, ಪೂರ್ಣಾಙ್ಕ ofst, ಪೂರ್ಣಾಙ್ಕ n, ಪೂರ್ಣಾಙ್ಕ flags){
  sqlite3_file *pReal = ((CrashFile*)pFile)->pRealFile;
  ಪ್ರತ್ಯಾಗಮ pReal->pMethods->xShmLock(pReal, ofst, n, flags);
}
ಸ್ಥಿರ ಶೂನ್ಯ cfShmBarrier(sqlite3_file *pFile){
  sqlite3_file *pReal = ((CrashFile*)pFile)->pRealFile;
  pReal->pMethods->xShmBarrier(pReal);
}
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ cfShmUnmap(sqlite3_file *pFile, ಪೂರ್ಣಾಙ್ಕ delFlag){
  sqlite3_file *pReal = ((CrashFile*)pFile)->pRealFile;
  ಪ್ರತ್ಯಾಗಮ pReal->pMethods->xShmUnmap(pReal, delFlag);
}
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ cfShmMap(
  sqlite3_file *pFile,            /* Handle open on database file */
  ಪೂರ್ಣಾಙ್ಕ iRegion,                    /* Region to retrieve */
  ಪೂರ್ಣಾಙ್ಕ sz,                         /* Size of regions */
  ಪೂರ್ಣಾಙ್ಕ w,                          /* True to extend file if necessary */
  ಶೂನ್ಯ ಚಞ್ಚಲ **pp              /* OUT: Mapped memory */
){
  sqlite3_file *pReal = ((CrashFile*)pFile)->pRealFile;
  ಪ್ರತ್ಯಾಗಮ pReal->pMethods->xShmMap(pReal, iRegion, sz, w, pp);
}

ಸ್ಥಿರ ಧ್ರುವ sqlite3_io_methods CrashFileVtab = {
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
ಸಂರಚನಾ crashAppData {
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ cfOpen(
  sqlite3_vfs *pCfVfs,
  ಧ್ರುವ ವರ್ಣ *zName,
  sqlite3_file *pFile,
  ಪೂರ್ಣಾಙ್ಕ flags,
  ಪೂರ್ಣಾಙ್ಕ *pOutFlags
){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  ಪೂರ್ಣಾಙ್ಕ rc;
  CrashFile *pWrapper = (CrashFile *)pFile;
  sqlite3_file *pReal = (sqlite3_file*)&pWrapper[1];

  memset(pWrapper, 0, ಪರಿಮಾಣ(CrashFile));
  rc = sqlite3OsOpen(pVfs, zName, pReal, flags, pOutFlags);

  ಯದಿ( rc==SQLITE_OK ){
    i64 iSize;
    pWrapper->pMethod = &CrashFileVtab;
    pWrapper->zName = (ವರ್ಣ *)zName;
    pWrapper->pRealFile = pReal;
    rc = sqlite3OsFileSize(pReal, &iSize);
    pWrapper->iSize = (ಪೂರ್ಣಾಙ್ಕ)iSize;
    pWrapper->flags = flags;
  }
  ಯದಿ( rc==SQLITE_OK ){
    pWrapper->nData = (ಪೂರ್ಣಾಙ್ಕ)(4096 + pWrapper->iSize);
    pWrapper->zData = crash_malloc(pWrapper->nData);
    ಯದಿ( pWrapper->zData ){
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
      ಪ್ರತಿ(iOff=0; iOff<pWrapper->iSize; iOff += 512){
        ಪೂರ್ಣಾಙ್ಕ nRead = (ಪೂರ್ಣಾಙ್ಕ)(pWrapper->iSize - iOff);
        ಯದಿ( nRead>512 ) nRead = 512;
        rc = sqlite3OsRead(pReal, &pWrapper->zData[iOff], nRead, iOff);
      }
    }ಅನ್ಯಥಾ{
      rc = SQLITE_NOMEM;
    }
  }
  ಯದಿ( rc!=SQLITE_OK && pWrapper->pMethod ){
    sqlite3OsClose(pFile);
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ cfDelete(sqlite3_vfs *pCfVfs, ಧ್ರುವ ವರ್ಣ *zPath, ಪೂರ್ಣಾಙ್ಕ dirSync){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  ಪ್ರತ್ಯಾಗಮ pVfs->xDelete(pVfs, zPath, dirSync);
}
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ cfAccess(
  sqlite3_vfs *pCfVfs,
  ಧ್ರುವ ವರ್ಣ *zPath,
  ಪೂರ್ಣಾಙ್ಕ flags,
  ಪೂರ್ಣಾಙ್ಕ *pResOut
){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  ಪ್ರತ್ಯಾಗಮ pVfs->xAccess(pVfs, zPath, flags, pResOut);
}
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ cfFullPathname(
  sqlite3_vfs *pCfVfs,
  ಧ್ರುವ ವರ್ಣ *zPath,
  ಪೂರ್ಣಾಙ್ಕ nPathOut,
  ವರ್ಣ *zPathOut
){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  ಪ್ರತ್ಯಾಗಮ pVfs->xFullPathname(pVfs, zPath, nPathOut, zPathOut);
}
ಸ್ಥಿರ ಶೂನ್ಯ *cfDlOpen(sqlite3_vfs *pCfVfs, ಧ್ರುವ ವರ್ಣ *zPath){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  ಪ್ರತ್ಯಾಗಮ pVfs->xDlOpen(pVfs, zPath);
}
ಸ್ಥಿರ ಶೂನ್ಯ cfDlError(sqlite3_vfs *pCfVfs, ಪೂರ್ಣಾಙ್ಕ nByte, ವರ್ಣ *z_Err_ಸಂದೇಶ){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  pVfs->xDlError(pVfs, nByte, z_Err_ಸಂದೇಶ);
}
ಸ್ಥಿರ ಶೂನ್ಯ (*cfDlSym(sqlite3_vfs *pCfVfs, ಶೂನ್ಯ *pH, ಧ್ರುವ ವರ್ಣ *zSym))(ಶೂನ್ಯ){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  ಪ್ರತ್ಯಾಗಮ pVfs->xDlSym(pVfs, pH, zSym);
}
ಸ್ಥಿರ ಶೂನ್ಯ cfDlClose(sqlite3_vfs *pCfVfs, ಶೂನ್ಯ *pHandle){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  pVfs->xDlClose(pVfs, pHandle);
}
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ cfRandomness(sqlite3_vfs *pCfVfs, ಪೂರ್ಣಾಙ್ಕ nByte, ವರ್ಣ *zBufOut){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  ಪ್ರತ್ಯಾಗಮ pVfs->xRandomness(pVfs, nByte, zBufOut);
}
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ cfSleep(sqlite3_vfs *pCfVfs, ಪೂರ್ಣಾಙ್ಕ nMicro){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  ಪ್ರತ್ಯಾಗಮ pVfs->xSleep(pVfs, nMicro);
}
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ cfCurrentTime(sqlite3_vfs *pCfVfs, ದ್ವಿಗುಣ *pTimeOut){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  ಪ್ರತ್ಯಾಗಮ pVfs->xCurrentTime(pVfs, pTimeOut);
}
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ cfGetLastError(sqlite3_vfs *pCfVfs, ಪೂರ್ಣಾಙ್ಕ n, ವರ್ಣ *z){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  ಪ್ರತ್ಯಾಗಮ pVfs->xGetLastError(pVfs, n, z);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ processDevSymArgs(
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[],
  ಪೂರ್ಣಾಙ್ಕ *piDeviceChar,
  ಪೂರ್ಣಾಙ್ಕ *piSectorSize
){
  ಸಂರಚನಾ DeviceFlag {
    ವರ್ಣ *zName;
    ಪೂರ್ಣಾಙ್ಕ i_ಮೌಲ್ಯ;
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

  ಪೂರ್ಣಾಙ್ಕ i;
  ಪೂರ್ಣಾಙ್ಕ iDc = 0;
  ಪೂರ್ಣಾಙ್ಕ iSectorSize = 0;
  ಪೂರ್ಣಾಙ್ಕ setSectorsize = 0;
  ಪೂರ್ಣಾಙ್ಕ setDeviceChar = 0;

  ಪ್ರತಿ(i=0; i<objc; i+=2){
    Tcl_Size nOpt;
    ವರ್ಣ *zOpt = Tcl_GetStringFromObj(objv[i], &nOpt);

    ಯದಿ( (nOpt>11 || nOpt<2 || strncmp("-sectorsize", zOpt, nOpt))
     && (nOpt>16 || nOpt<2 || strncmp("-characteristics", zOpt, nOpt))
    ){
      Tcl_AppendResult(interp,
        "Bad option: \"", zOpt,
        "\" - must be \"-characteristics\" or \"-sectorsize\"", ರಿಕ್ತ
      );
      ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
    }
    ಯದಿ( i==objc-1 ){
      Tcl_AppendResult(interp, "Option requires an argument: \"", zOpt, "\"", ರಿಕ್ತ);
      ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
    }

    ಯದಿ( zOpt[1]=='s' ){
      ಯದಿ( Tcl_GetIntFromObj(interp, objv[i+1], &iSectorSize) ){
        ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
      }
      setSectorsize = 1;
    }ಅನ್ಯಥಾ{
      ಪೂರ್ಣಾಙ್ಕ j;
      Tcl_Obj **apObj;
      Tcl_Size nObj;
      ಯದಿ( Tcl_ListObjGetElements(interp, objv[i+1], &nObj, &apObj) ){
        ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
      }
      ಪ್ರತಿ(j=0; j<(ಪೂರ್ಣಾಙ್ಕ)nObj; j++){
        ಪೂರ್ಣಾಙ್ಕ rc;
        ಪೂರ್ಣಾಙ್ಕ iChoice;
        Tcl_Obj *pFlag = Tcl_DuplicateObj(apObj[j]);
        Tcl_Incr_Ref_ಎಣಿಕೆ(pFlag);
        Tcl_UtfToLower(Tcl_GetString(pFlag));

        rc = Tcl_GetIndexFromObjStruct(
            interp, pFlag, aFlag, ಪರಿಮಾಣ(aFlag[0]), "no such flag", 0, &iChoice
        );
        Tcl_Decr_Ref_ಎಣಿಕೆ(pFlag);
        ಯದಿ( rc ){
          ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
        }

        iDc |= aFlag[iChoice].i_ಮೌಲ್ಯ;
      }
      setDeviceChar = 1;
    }
  }

  ಯದಿ( setDeviceChar ){
    *piDeviceChar = iDc;
  }
  ಯದಿ( setSectorsize ){
    *piSectorSize = iSectorSize;
  }

  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** tclcmd:   sqlite3_crash_now
**
** Simulate a crash immediately. This function does not return
** (writeListSync() calls exit(-1)).
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI crashNowCmd(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಯದಿ( objc!=1 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  writeListSync(0, 1);
  ಪ್ರತಿಜ್ಞಾ( 0 );
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** tclcmd:   sqlite_crash_enable ENABLE ?DEFAULT?
**
** Parameter ENABLE must be a boolean value. If true, then the "crash"
** vfs is added to the system. If false, it is removed.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI crashEnableCmd(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಙ್ಕ isEnable;
  ಪೂರ್ಣಾಙ್ಕ isDefault = 0;
  ಸ್ಥಿರ sqlite3_vfs crashVfs = {
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

  ಯದಿ( objc!=2 && objc!=3 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "ENABLE ?DEFAULT?");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  ಯದಿ( Tcl_GetBooleanFromObj(interp, objv[1], &isEnable) ){
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( objc==3 && Tcl_GetBooleanFromObj(interp, objv[2], &isDefault) ){
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  ಯದಿ( (isEnable && crashVfs.pAppData) || (!isEnable && !crashVfs.pAppData) ){
    ಪ್ರತ್ಯಾಗಮ TCL_OK;
  }

  ಯದಿ( crashVfs.pAppData==0 ){
    sqlite3_vfs *pOriginalVfs = sqlite3_vfs_find(0);
    crashVfs.mxPathname = pOriginalVfs->mxPathname;
    crashVfs.pAppData = (ಶೂನ್ಯ *)pOriginalVfs;
    crashVfs.szOsFile = ಪರಿಮಾಣ(CrashFile) + pOriginalVfs->szOsFile;
    sqlite3_vfs_register(&crashVfs, isDefault);
  }ಅನ್ಯಥಾ{
    crashVfs.pAppData = 0;
    sqlite3_vfs_unregister(&crashVfs);
  }

  ಪ್ರತ್ಯಾಗಮ TCL_OK;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI crashParamsObjCmd(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಙ್ಕ iDelay;
  ಧ್ರುವ ವರ್ಣ *zCrashFile;
  Tcl_Size nCrashFile;
  ಪೂರ್ಣಾಙ್ಕ iDc, iSectorSize;

  iDc = -1;
  iSectorSize = -1;

  ಯದಿ( objc<3 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "?OPTIONS? DELAY CRASHFILE");
    ಗಚ್ಛ error;
  }

  zCrashFile = Tcl_GetStringFromObj(objv[objc-1], &nCrashFile);
  ಯದಿ( nCrashFile>=ಪರಿಮಾಣ(g.zCrashFile) ){
    Tcl_AppendResult(interp, "Filename is too long: \"", zCrashFile, "\"", ರಿಕ್ತ);
    ಗಚ್ಛ error;
  }
  ಯದಿ( Tcl_GetIntFromObj(interp, objv[objc-2], &iDelay) ){
    ಗಚ್ಛ error;
  }

  ಯದಿ( processDevSymArgs(interp, objc-3, &objv[1], &iDc, &iSectorSize) ){
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  ಯದಿ( iDc>=0 ){
    g.iDeviceCharacteristics = iDc;
  }
  ಯದಿ( iSectorSize>=0 ){
    g.iSectorSize = iSectorSize;
  }

  g.iCrash = iDelay;
  memcpy(g.zCrashFile, zCrashFile, nCrashFile+1);
  sqlite3CrashTestEnable = 1;
  ಪ್ರತ್ಯಾಗಮ TCL_OK;

error:
  ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI devSymObjCmd(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಶೂನ್ಯ devsym_register(ಪೂರ್ಣಾಙ್ಕ iDeviceChar, ಪೂರ್ಣಾಙ್ಕ iSectorSize);

  ಪೂರ್ಣಾಙ್ಕ iDc = -1;
  ಪೂರ್ಣಾಙ್ಕ iSectorSize = -1;

  ಯದಿ( processDevSymArgs(interp, objc-1, &objv[1], &iDc, &iSectorSize) ){
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  devsym_register(iDc, iSectorSize);

  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** tclcmd: sqlite3_crash_on_write N
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI writeCrashObjCmd(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಶೂನ್ಯ devsym_crash_on_write(ಪೂರ್ಣಾಙ್ಕ);
  ಪೂರ್ಣಾಙ್ಕ nWrite = 0;

  ಯದಿ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "NWRITE");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( Tcl_GetIntFromObj(interp, objv[1], &nWrite) ){
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  devsym_crash_on_write(nWrite);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** tclcmd: unregister_devsim
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI dsUnregisterObjCmd(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಶೂನ್ಯ devsym_unregister(ಶೂನ್ಯ);

  ಯದಿ( objc!=1 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  devsym_unregister();
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** tclcmd: register_jt_vfs ?-default? PARENT-VFS
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI jtObjCmd(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಙ್ಕ jt_register(ವರ್ಣ *, ಪೂರ್ಣಾಙ್ಕ);
  ವರ್ಣ *zParent = 0;

  ಯದಿ( objc!=2 && objc!=3 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "?-default? PARENT-VFS");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  zParent = Tcl_GetString(objv[1]);
  ಯದಿ( objc==3 ){
    ಯದಿ( strcmp(zParent, "-default") ){
      Tcl_AppendResult(interp,
          "bad option \"", zParent, "\": must be -default", ರಿಕ್ತ
      );
      ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
    }
    zParent = Tcl_GetString(objv[2]);
  }

  ಯದಿ( !(*zParent) ){
    zParent = 0;
  }
  ಯದಿ( jt_register(zParent, objc==3) ){
    Tcl_AppendResult(interp, "Error in jt_register", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** tclcmd: unregister_jt_vfs
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI jtUnregisterObjCmd(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಶೂನ್ಯ jt_unregister(ಶೂನ್ಯ);

  ಯದಿ( objc!=1 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  jt_unregister();
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

#endif /* SQLITE_OMIT_DISKIO */

/*
** This procedure registers the TCL procedures defined in this file.
*/
ಪೂರ್ಣಾಙ್ಕ Sqlitetest6_Init(Tcl_Interp *interp){
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
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

#endif /* SQLITE_TEST */
