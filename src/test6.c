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
#ଯଦି SQLITE_TEST          /* This file is used for testing only */
#ଅନ୍ତର୍ଭାଵ "sqliteInt.h"
#ଅନ୍ତର୍ଭାଵ "tclsqlite.h"

#ifndef SQLITE_OMIT_DISKIO  /* This file is a no-op if disk I/O is disabled */

/* #define TRACE_CRASHTEST */

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା CrashFile CrashFile;
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା CrashGlobal CrashGlobal;
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା WriteBuffer WriteBuffer;

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
ସଂରଚନା WriteBuffer {
  i64 iOffset;                 /* Byte offset of the start of this write() */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nBuf;                    /* Number of bytes written */
  u8 *zBuf;                    /* Pointer to copy of written data */
  CrashFile *pFile;            /* File this write() applies to */

  WriteBuffer *pNext;          /* Next in CrashGlobal.pWriteList */
};

ସଂରଚନା CrashFile {
  ଅଚଳ sqlite3_io_methods *pMethod;   /* Must be first */
  sqlite3_file *pRealFile;             /* Underlying "real" file handle */
  ଅକ୍ଷର *zName;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags;                           /* Flags the file was opened with */

  /* Cache of the entire file. This is used to speed up OsRead() and
  ** OsFileSize() calls. Although both could be done by traversing the
  ** write-list, in practice this is impractically slow.
  */
  u8 *zData;                           /* Buffer containing file contents */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nData;                           /* Size of buffer allocated at zData */
  i64 iSize;                           /* Size of file in bytes */
};

ସଂରଚନା CrashGlobal {
  WriteBuffer *pWriteList;     /* Head of write-list */
  WriteBuffer *pWriteListEnd;  /* End of write-list */

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iSectorSize;             /* Value of simulated sector size */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDeviceCharacteristics;  /* Value of simulated device characteristics */

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCrash;                  /* Crash on the iCrash'th call to xSync() */
  ଅକ୍ଷର zCrashFile[500];        /* Crash during an xSync() on this file */
};

ସ୍ଥିର CrashGlobal g = {0, 0, SQLITE_DEFAULT_SECTOR_SIZE, 0, 0};

/*
** Set this global variable to 1 to enable crash testing.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3CrashTestEnable = 0;

ସ୍ଥିର ଶୂନ୍ୟ *crash_malloc(ପୂର୍ଣ୍ଣସଂଖ୍ୟା nByte){
  ଫେରନ୍ତୁ (ଶୂନ୍ୟ *)Tcl_AttemptAlloc((size_t)nByte);
}
ସ୍ଥିର ଶୂନ୍ୟ crash_free(ଶୂନ୍ୟ *p){
  Tcl_Free(p);
}
ସ୍ଥିର ଶୂନ୍ୟ *crash_realloc(ଶୂନ୍ୟ *p, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n){
  ଫେରନ୍ତୁ (ଶୂନ୍ୟ *)Tcl_AttemptRealloc(p, (size_t)n);
}

/*
** Wrapper around the sqlite3OsWrite() function that avoids writing to the
** 512 byte block beginning at offset PENDING_BYTE.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା writeDbFile(CrashFile *p, u8 *z, i64 iAmt, i64 iOff){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iSkip = 0;
  ଯଦି( (iAmt-iSkip)>0 ){
    rc = sqlite3OsWrite(p->pRealFile, &z[iSkip], (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(iAmt-iSkip), iOff+iSkip);
  }
  ଫେରନ୍ତୁ rc;
}

/*
** Flush the write-list as if xSync() had been called on file handle
** pFile. If isCrash is true, simulate a crash.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା writeListSync(CrashFile *pFile, ପୂର୍ଣ୍ଣସଂଖ୍ୟା isCrash){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDc = g.iDeviceCharacteristics;

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
  ଯଦି( !isCrash ){
    ପାଇଁ(pWrite=g.pWriteList; pWrite; pWrite=pWrite->pNext){
      ଯଦି( pWrite->pFile==pFile ){
        pFinal = pWrite;
      }
    }
  }ଅନ୍ୟଥା ଯଦି( iDc&(SQLITE_IOCAP_SEQUENTIAL|SQLITE_IOCAP_SAFE_APPEND) ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା nWrite = 0;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା iFinal;
    ପାଇଁ(pWrite=g.pWriteList; pWrite; pWrite=pWrite->pNext) nWrite++;
    sqlite3_randomness(ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା), &iFinal);
    iFinal = ((iFinal<0)?-1*iFinal:iFinal)%nWrite;
    ପାଇଁ(pWrite=g.pWriteList; iFinal>0; pWrite=pWrite->pNext) iFinal--;
    pFinal = pWrite;
  }

#ifdef TRACE_CRASHTEST
  ଯଦି( pFile ){
    printf("Sync %s (is %s crash)\n", pFile->zName, (isCrash?"a":"not a"));
  }
#endif

  ppPtr = &g.pWriteList;
  ପାଇଁ(pWrite=*ppPtr; rc==SQLITE_OK && pWrite; pWrite=*ppPtr){
    sqlite3_file *pRealFile = pWrite->pFile->pRealFile;

    /* (eAction==1)      -> write block out normally,
    ** (eAction==2)      -> do nothing,
    ** (eAction==3)      -> trash sectors.
    */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା eAction = 0;
    ଯଦି( !isCrash ){
      eAction = 2;
      ଯଦି( (pWrite->pFile==pFile || iDc&SQLITE_IOCAP_SEQUENTIAL) ){
        eAction = 1;
      }
    }ଅନ୍ୟଥା{
      ଅକ୍ଷର random;
      sqlite3_randomness(1, &random);

      /* Do not select option 3 (sector trashing) if the IOCAP_ATOMIC flag
      ** is set or this is an OsTruncate(), not an Oswrite().
      */
      ଯଦି( (iDc&SQLITE_IOCAP_ATOMIC) || (pWrite->zBuf==0) ){
        random &= 0x01;
      }

      /* If IOCAP_SEQUENTIAL is set and this is not the final entry
      ** in the truncated write-list, always select option 1 (write
      ** out correctly).
      */
      ଯଦି( (iDc&SQLITE_IOCAP_SEQUENTIAL && pWrite!=pFinal) ){
        random = 0;
      }

      /* If IOCAP_SAFE_APPEND is set and this OsWrite() operation is
      ** an append (first byte of the written region is 1 byte past the
      ** current EOF), always select option 1 (write out correctly).
      */
      ଯଦି( iDc&SQLITE_IOCAP_SAFE_APPEND && pWrite->zBuf ){
        i64 iSize;
        sqlite3OsFileSize(pRealFile, &iSize);
        ଯଦି( iSize==pWrite->iOffset ){
          random = 0;
        }
      }

      ଯଦି( (random&0x06)==0x06 ){
        eAction = 3;
      }ଅନ୍ୟଥା{
        eAction = ((random&0x01)?2:1);
      }
    }

    ବାଛ( eAction ){
      ସ୍ଥିତି 1: {               /* Write out correctly */
        ଯଦି( pWrite->zBuf ){
          rc = writeDbFile(
              pWrite->pFile, pWrite->zBuf, pWrite->nBuf, pWrite->iOffset
          );
        }ଅନ୍ୟଥା{
          rc = sqlite3OsTruncate(pRealFile, pWrite->iOffset);
        }
        *ppPtr = pWrite->pNext;
#ifdef TRACE_CRASHTEST
        ଯଦି( isCrash ){
          printf("Writing %d bytes @ %d (%s)\n",
            pWrite->nBuf, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)pWrite->iOffset, pWrite->pFile->zName
          );
        }
#endif
        crash_free(pWrite);
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି 2: {               /* Do nothing */
        ppPtr = &pWrite->pNext;
#ifdef TRACE_CRASHTEST
        ଯଦି( isCrash ){
          printf("Omiting %d bytes @ %d (%s)\n",
            pWrite->nBuf, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)pWrite->iOffset, pWrite->pFile->zName
          );
        }
#endif
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି 3: {               /* Trash sectors */
        u8 *zGarbage;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା iFirst = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(pWrite->iOffset/g.iSectorSize);
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା iLast = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)((pWrite->iOffset+pWrite->nBuf-1)/g.iSectorSize);

        ପ୍ରତିଜ୍ଞା(pWrite->zBuf);

#ifdef TRACE_CRASHTEST
        printf("Trashing %d sectors (%d bytes) @ %lld (sector %d) (%s)\n",
            1+iLast-iFirst, (1+iLast-iFirst)*g.iSectorSize,
            pWrite->iOffset, iFirst, pWrite->pFile->zName
        );
#endif

        zGarbage = crash_malloc(g.iSectorSize);
        ଯଦି( zGarbage ){
          sqlite3_int64 i;
          ପାଇଁ(i=iFirst; rc==SQLITE_OK && i<=iLast; i++){
            sqlite3_randomness(g.iSectorSize, zGarbage);
            rc = writeDbFile(
              pWrite->pFile, zGarbage, g.iSectorSize, i*g.iSectorSize
            );
          }
          crash_free(zGarbage);
        }ଅନ୍ୟଥା{
          rc = SQLITE_NOMEM;
        }

        ppPtr = &pWrite->pNext;
        ଭାଙ୍ଗ;
      }

      ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
        ପ୍ରତିଜ୍ଞା(!"Cannot happen");
    }

    ଯଦି( pWrite==pFinal ) ଭାଙ୍ଗ;
  }

  ଯଦି( rc==SQLITE_OK && isCrash ){
    exit(-1);
  }

  ପାଇଁ(pWrite=g.pWriteList; pWrite && pWrite->pNext; pWrite=pWrite->pNext);
  g.pWriteListEnd = pWrite;

  ଫେରନ୍ତୁ rc;
}

/*
** Add an entry to the end of the write-list.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା writeListAppend(
  sqlite3_file *pFile,
  sqlite3_int64 iOffset,
  ଅଚଳ u8 *zBuf,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nBuf
){
  WriteBuffer *pNew;

  ପ୍ରତିଜ୍ଞା((zBuf && nBuf) || (!nBuf && !zBuf));

  pNew = (WriteBuffer *)crash_malloc(ଆକାର(WriteBuffer) + nBuf);
  ଯଦି( pNew==0 ){
    fprintf(stderr, "out of memory in the crash simulator\n");
  }
  memset(pNew, 0, ଆକାର(WriteBuffer)+nBuf);
  pNew->iOffset = iOffset;
  pNew->nBuf = nBuf;
  pNew->pFile = (CrashFile *)pFile;
  ଯଦି( zBuf ){
    pNew->zBuf = (u8 *)&pNew[1];
    memcpy(pNew->zBuf, zBuf, nBuf);
  }

  ଯଦି( g.pWriteList ){
    ପ୍ରତିଜ୍ଞା(g.pWriteListEnd);
    g.pWriteListEnd->pNext = pNew;
  }ଅନ୍ୟଥା{
    g.pWriteList = pNew;
  }
  g.pWriteListEnd = pNew;
 
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Close a crash-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା cfClose(sqlite3_file *pFile){
  CrashFile *pCrash = (CrashFile *)pFile;
  writeListSync(pCrash, 0);
  sqlite3OsClose(pCrash->pRealFile);
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Read data from a crash-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା cfRead(
  sqlite3_file *pFile,
  ଶୂନ୍ୟ *zBuf,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iAmt,
  sqlite_int64 iOfst
){
  CrashFile *pCrash = (CrashFile *)pFile;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nCopy = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)MIN((i64)iAmt, (pCrash->iSize - iOfst));

  ଯଦି( nCopy>0 ){
    memcpy(zBuf, &pCrash->zData[iOfst], nCopy);
  }

  /* Check the file-size to see if this is a short-read */
  ଯଦି( nCopy<iAmt ){
    ଫେରନ୍ତୁ SQLITE_IOERR_SHORT_READ;
  }

  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Write data to a crash-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା cfWrite(
  sqlite3_file *pFile,
  ଅଚଳ ଶୂନ୍ୟ *zBuf,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iAmt,
  sqlite_int64 iOfst
){
  CrashFile *pCrash = (CrashFile *)pFile;
  ଯଦି( iAmt+iOfst>pCrash->iSize ){
    pCrash->iSize = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(iAmt+iOfst);
  }
  ଯେତେବେଳେ( pCrash->iSize>pCrash->nData ){
    u8 *zNew;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା nNew = (pCrash->nData*2) + 4096;
    zNew = crash_realloc(pCrash->zData, nNew);
    ଯଦି( !zNew ){
      ଫେରନ୍ତୁ SQLITE_NOMEM;
    }
    memset(&zNew[pCrash->nData], 0, nNew-pCrash->nData);
    pCrash->nData = nNew;
    pCrash->zData = zNew;
  }
  memcpy(&pCrash->zData[iOfst], zBuf, iAmt);
  ଫେରନ୍ତୁ writeListAppend(pFile, iOfst, zBuf, iAmt);
}

/*
** Truncate a crash-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା cfTruncate(sqlite3_file *pFile, sqlite_int64 size){
  CrashFile *pCrash = (CrashFile *)pFile;
  ପ୍ରତିଜ୍ଞା(size>=0);
  ଯଦି( pCrash->iSize>size ){
    pCrash->iSize = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)size;
  }
  ଫେରନ୍ତୁ writeListAppend(pFile, size, 0, 0);
}

/*
** Sync a crash-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା cfSync(sqlite3_file *pFile, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags){
  CrashFile *pCrash = (CrashFile *)pFile;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା isCrash = 0;

  ଅଚଳ ଅକ୍ଷର *zName = pCrash->zName;
  ଅଚଳ ଅକ୍ଷର *zCrashFile = g.zCrashFile;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nName = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)strlen(zName);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nCrashFile = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)strlen(zCrashFile);

  ଯଦି( nCrashFile>0 && zCrashFile[nCrashFile-1]=='*' ){
    nCrashFile--;
    ଯଦି( nName>nCrashFile ) nName = nCrashFile;
  }

#ifdef TRACE_CRASHTEST
  printf("cfSync(): nName = %d, nCrashFile = %d, zName = %s, zCrashFile = %s\n",
         nName, nCrashFile, zName, zCrashFile);
#endif

  ଯଦି( nName==nCrashFile && 0==memcmp(zName, zCrashFile, nName) ){
#ifdef TRACE_CRASHTEST
    printf("cfSync(): name matched, g.iCrash = %d\n", g.iCrash);
#endif
    ଯଦି( (--g.iCrash)==0 ) isCrash = 1;
  }

  ଫେରନ୍ତୁ writeListSync(pCrash, isCrash);
}

/*
** Return the current file-size of the crash-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା cfFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  CrashFile *pCrash = (CrashFile *)pFile;
  *pSize = (i64)pCrash->iSize;
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Calls related to file-locks are passed on to the real file handle.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା cfLock(sqlite3_file *pFile, ପୂର୍ଣ୍ଣସଂଖ୍ୟା eLock){
  ଫେରନ୍ତୁ sqlite3OsLock(((CrashFile *)pFile)->pRealFile, eLock);
}
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା cfUnlock(sqlite3_file *pFile, ପୂର୍ଣ୍ଣସଂଖ୍ୟା eLock){
  ଫେରନ୍ତୁ sqlite3OsUnlock(((CrashFile *)pFile)->pRealFile, eLock);
}
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା cfCheckReservedLock(sqlite3_file *pFile, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pResOut){
  ଫେରନ୍ତୁ sqlite3OsCheckReservedLock(((CrashFile *)pFile)->pRealFile, pResOut);
}
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା cfFileControl(sqlite3_file *pFile, ପୂର୍ଣ୍ଣସଂଖ୍ୟା op, ଶୂନ୍ୟ *pArg){
  ଯଦି( op==SQLITE_FCNTL_SIZE_HINT ){
    CrashFile *pCrash = (CrashFile *)pFile;
    i64 nByte = *(i64 *)pArg;
    ଯଦି( nByte>pCrash->iSize ){
      ଯଦି( SQLITE_OK==writeListAppend(pFile, nByte, 0, 0) ){
        pCrash->iSize = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)nByte;
      }
    }
    ଫେରନ୍ତୁ SQLITE_OK;
  }
  ଫେରନ୍ତୁ sqlite3OsFileControl(((CrashFile *)pFile)->pRealFile, op, pArg);
}

/*
** The xSectorSize() and xDeviceCharacteristics() functions return
** the global values configured by the [sqlite_crashparams] tcl
*  interface.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା cfSectorSize(sqlite3_file *pFile){
  ଫେରନ୍ତୁ g.iSectorSize;
}
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା cfDeviceCharacteristics(sqlite3_file *pFile){
  ଫେରନ୍ତୁ g.iDeviceCharacteristics;
}

/*
** Pass-throughs for WAL support.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା cfShmLock(sqlite3_file *pFile, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ofst, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags){
  sqlite3_file *pReal = ((CrashFile*)pFile)->pRealFile;
  ଫେରନ୍ତୁ pReal->pMethods->xShmLock(pReal, ofst, n, flags);
}
ସ୍ଥିର ଶୂନ୍ୟ cfShmBarrier(sqlite3_file *pFile){
  sqlite3_file *pReal = ((CrashFile*)pFile)->pRealFile;
  pReal->pMethods->xShmBarrier(pReal);
}
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା cfShmUnmap(sqlite3_file *pFile, ପୂର୍ଣ୍ଣସଂଖ୍ୟା delFlag){
  sqlite3_file *pReal = ((CrashFile*)pFile)->pRealFile;
  ଫେରନ୍ତୁ pReal->pMethods->xShmUnmap(pReal, delFlag);
}
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା cfShmMap(
  sqlite3_file *pFile,            /* Handle open on database file */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iRegion,                    /* Region to retrieve */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା sz,                         /* Size of regions */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା w,                          /* True to extend file if necessary */
  ଶୂନ୍ୟ ଅସ୍ଥିର **pp              /* OUT: Mapped memory */
){
  sqlite3_file *pReal = ((CrashFile*)pFile)->pRealFile;
  ଫେରନ୍ତୁ pReal->pMethods->xShmMap(pReal, iRegion, sz, w, pp);
}

ସ୍ଥିର ଅଚଳ sqlite3_io_methods CrashFileVtab = {
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
ସଂରଚନା crashAppData {
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା cfOpen(
  sqlite3_vfs *pCfVfs,
  ଅଚଳ ଅକ୍ଷର *zName,
  sqlite3_file *pFile,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pOutFlags
){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  CrashFile *pWrapper = (CrashFile *)pFile;
  sqlite3_file *pReal = (sqlite3_file*)&pWrapper[1];

  memset(pWrapper, 0, ଆକାର(CrashFile));
  rc = sqlite3OsOpen(pVfs, zName, pReal, flags, pOutFlags);

  ଯଦି( rc==SQLITE_OK ){
    i64 iSize;
    pWrapper->pMethod = &CrashFileVtab;
    pWrapper->zName = (ଅକ୍ଷର *)zName;
    pWrapper->pRealFile = pReal;
    rc = sqlite3OsFileSize(pReal, &iSize);
    pWrapper->iSize = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)iSize;
    pWrapper->flags = flags;
  }
  ଯଦି( rc==SQLITE_OK ){
    pWrapper->nData = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(4096 + pWrapper->iSize);
    pWrapper->zData = crash_malloc(pWrapper->nData);
    ଯଦି( pWrapper->zData ){
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
      ପାଇଁ(iOff=0; iOff<pWrapper->iSize; iOff += 512){
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା nRead = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(pWrapper->iSize - iOff);
        ଯଦି( nRead>512 ) nRead = 512;
        rc = sqlite3OsRead(pReal, &pWrapper->zData[iOff], nRead, iOff);
      }
    }ଅନ୍ୟଥା{
      rc = SQLITE_NOMEM;
    }
  }
  ଯଦି( rc!=SQLITE_OK && pWrapper->pMethod ){
    sqlite3OsClose(pFile);
  }
  ଫେରନ୍ତୁ rc;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା cfDelete(sqlite3_vfs *pCfVfs, ଅଚଳ ଅକ୍ଷର *zPath, ପୂର୍ଣ୍ଣସଂଖ୍ୟା dirSync){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  ଫେରନ୍ତୁ pVfs->xDelete(pVfs, zPath, dirSync);
}
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା cfAccess(
  sqlite3_vfs *pCfVfs,
  ଅଚଳ ଅକ୍ଷର *zPath,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pResOut
){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  ଫେରନ୍ତୁ pVfs->xAccess(pVfs, zPath, flags, pResOut);
}
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା cfFullPathname(
  sqlite3_vfs *pCfVfs,
  ଅଚଳ ଅକ୍ଷର *zPath,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nPathOut,
  ଅକ୍ଷର *zPathOut
){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  ଫେରନ୍ତୁ pVfs->xFullPathname(pVfs, zPath, nPathOut, zPathOut);
}
ସ୍ଥିର ଶୂନ୍ୟ *cfDlOpen(sqlite3_vfs *pCfVfs, ଅଚଳ ଅକ୍ଷର *zPath){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  ଫେରନ୍ତୁ pVfs->xDlOpen(pVfs, zPath);
}
ସ୍ଥିର ଶୂନ୍ୟ cfDlError(sqlite3_vfs *pCfVfs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nByte, ଅକ୍ଷର *z_Err_ସନ୍ଦେଶ){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  pVfs->xDlError(pVfs, nByte, z_Err_ସନ୍ଦେଶ);
}
ସ୍ଥିର ଶୂନ୍ୟ (*cfDlSym(sqlite3_vfs *pCfVfs, ଶୂନ୍ୟ *pH, ଅଚଳ ଅକ୍ଷର *zSym))(ଶୂନ୍ୟ){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  ଫେରନ୍ତୁ pVfs->xDlSym(pVfs, pH, zSym);
}
ସ୍ଥିର ଶୂନ୍ୟ cfDlClose(sqlite3_vfs *pCfVfs, ଶୂନ୍ୟ *pHandle){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  pVfs->xDlClose(pVfs, pHandle);
}
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା cfRandomness(sqlite3_vfs *pCfVfs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nByte, ଅକ୍ଷର *zBufOut){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  ଫେରନ୍ତୁ pVfs->xRandomness(pVfs, nByte, zBufOut);
}
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା cfSleep(sqlite3_vfs *pCfVfs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nMicro){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  ଫେରନ୍ତୁ pVfs->xSleep(pVfs, nMicro);
}
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା cfCurrentTime(sqlite3_vfs *pCfVfs, ଦ୍ୱିଗୁଣ *pTimeOut){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  ଫେରନ୍ତୁ pVfs->xCurrentTime(pVfs, pTimeOut);
}
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା cfGetLastError(sqlite3_vfs *pCfVfs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n, ଅକ୍ଷର *z){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  ଫେରନ୍ତୁ pVfs->xGetLastError(pVfs, n, z);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା processDevSymArgs(
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[],
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *piDeviceChar,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *piSectorSize
){
  ସଂରଚନା DeviceFlag {
    ଅକ୍ଷର *zName;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i_ମୂଲ୍ୟ;
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

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDc = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iSectorSize = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା setSectorsize = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା setDeviceChar = 0;

  ପାଇଁ(i=0; i<objc; i+=2){
    Tcl_Size nOpt;
    ଅକ୍ଷର *zOpt = Tcl_GetStringFromObj(objv[i], &nOpt);

    ଯଦି( (nOpt>11 || nOpt<2 || strncmp("-sectorsize", zOpt, nOpt))
     && (nOpt>16 || nOpt<2 || strncmp("-characteristics", zOpt, nOpt))
    ){
      Tcl_AppendResult(interp,
        "Bad option: \"", zOpt,
        "\" - must be \"-characteristics\" or \"-sectorsize\"", ଖାଲି
      );
      ଫେରନ୍ତୁ TCL_ERROR;
    }
    ଯଦି( i==objc-1 ){
      Tcl_AppendResult(interp, "Option requires an argument: \"", zOpt, "\"", ଖାଲି);
      ଫେରନ୍ତୁ TCL_ERROR;
    }

    ଯଦି( zOpt[1]=='s' ){
      ଯଦି( Tcl_GetIntFromObj(interp, objv[i+1], &iSectorSize) ){
        ଫେରନ୍ତୁ TCL_ERROR;
      }
      setSectorsize = 1;
    }ଅନ୍ୟଥା{
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା j;
      Tcl_Obj **apObj;
      Tcl_Size nObj;
      ଯଦି( Tcl_ListObjGetElements(interp, objv[i+1], &nObj, &apObj) ){
        ଫେରନ୍ତୁ TCL_ERROR;
      }
      ପାଇଁ(j=0; j<(ପୂର୍ଣ୍ଣସଂଖ୍ୟା)nObj; j++){
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା iChoice;
        Tcl_Obj *pFlag = Tcl_DuplicateObj(apObj[j]);
        Tcl_Incr_Ref_ଗଣନା(pFlag);
        Tcl_UtfToLower(Tcl_GetString(pFlag));

        rc = Tcl_GetIndexFromObjStruct(
            interp, pFlag, aFlag, ଆକାର(aFlag[0]), "no such flag", 0, &iChoice
        );
        Tcl_Decr_Ref_ଗଣନା(pFlag);
        ଯଦି( rc ){
          ଫେରନ୍ତୁ TCL_ERROR;
        }

        iDc |= aFlag[iChoice].i_ମୂଲ୍ୟ;
      }
      setDeviceChar = 1;
    }
  }

  ଯଦି( setDeviceChar ){
    *piDeviceChar = iDc;
  }
  ଯଦି( setSectorsize ){
    *piSectorSize = iSectorSize;
  }

  ଫେରନ୍ତୁ TCL_OK;
}

/*
** tclcmd:   sqlite3_crash_now
**
** Simulate a crash immediately. This function does not return
** (writeListSync() calls exit(-1)).
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI crashNowCmd(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ଯଦି( objc!=1 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  writeListSync(0, 1);
  ପ୍ରତିଜ୍ଞା( 0 );
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** tclcmd:   sqlite_crash_enable ENABLE ?DEFAULT?
**
** Parameter ENABLE must be a boolean value. If true, then the "crash"
** vfs is added to the system. If false, it is removed.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI crashEnableCmd(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା isEnable;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା isDefault = 0;
  ସ୍ଥିର sqlite3_vfs crashVfs = {
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

  ଯଦି( objc!=2 && objc!=3 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "ENABLE ?DEFAULT?");
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  ଯଦି( Tcl_GetBooleanFromObj(interp, objv[1], &isEnable) ){
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( objc==3 && Tcl_GetBooleanFromObj(interp, objv[2], &isDefault) ){
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  ଯଦି( (isEnable && crashVfs.pAppData) || (!isEnable && !crashVfs.pAppData) ){
    ଫେରନ୍ତୁ TCL_OK;
  }

  ଯଦି( crashVfs.pAppData==0 ){
    sqlite3_vfs *pOriginalVfs = sqlite3_vfs_find(0);
    crashVfs.mxPathname = pOriginalVfs->mxPathname;
    crashVfs.pAppData = (ଶୂନ୍ୟ *)pOriginalVfs;
    crashVfs.szOsFile = ଆକାର(CrashFile) + pOriginalVfs->szOsFile;
    sqlite3_vfs_register(&crashVfs, isDefault);
  }ଅନ୍ୟଥା{
    crashVfs.pAppData = 0;
    sqlite3_vfs_unregister(&crashVfs);
  }

  ଫେରନ୍ତୁ TCL_OK;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI crashParamsObjCmd(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDelay;
  ଅଚଳ ଅକ୍ଷର *zCrashFile;
  Tcl_Size nCrashFile;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDc, iSectorSize;

  iDc = -1;
  iSectorSize = -1;

  ଯଦି( objc<3 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "?OPTIONS? DELAY CRASHFILE");
    ଯାଅ error;
  }

  zCrashFile = Tcl_GetStringFromObj(objv[objc-1], &nCrashFile);
  ଯଦି( nCrashFile>=ଆକାର(g.zCrashFile) ){
    Tcl_AppendResult(interp, "Filename is too long: \"", zCrashFile, "\"", ଖାଲି);
    ଯାଅ error;
  }
  ଯଦି( Tcl_GetIntFromObj(interp, objv[objc-2], &iDelay) ){
    ଯାଅ error;
  }

  ଯଦି( processDevSymArgs(interp, objc-3, &objv[1], &iDc, &iSectorSize) ){
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  ଯଦି( iDc>=0 ){
    g.iDeviceCharacteristics = iDc;
  }
  ଯଦି( iSectorSize>=0 ){
    g.iSectorSize = iSectorSize;
  }

  g.iCrash = iDelay;
  memcpy(g.zCrashFile, zCrashFile, nCrashFile+1);
  sqlite3CrashTestEnable = 1;
  ଫେରନ୍ତୁ TCL_OK;

error:
  ଫେରନ୍ତୁ TCL_ERROR;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI devSymObjCmd(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ଶୂନ୍ୟ devsym_register(ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDeviceChar, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iSectorSize);

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDc = -1;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iSectorSize = -1;

  ଯଦି( processDevSymArgs(interp, objc-1, &objv[1], &iDc, &iSectorSize) ){
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  devsym_register(iDc, iSectorSize);

  ଫେରନ୍ତୁ TCL_OK;
}

/*
** tclcmd: sqlite3_crash_on_write N
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI writeCrashObjCmd(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ଶୂନ୍ୟ devsym_crash_on_write(ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nWrite = 0;

  ଯଦି( objc!=2 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "NWRITE");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( Tcl_GetIntFromObj(interp, objv[1], &nWrite) ){
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  devsym_crash_on_write(nWrite);
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** tclcmd: unregister_devsim
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI dsUnregisterObjCmd(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ଶୂନ୍ୟ devsym_unregister(ଶୂନ୍ୟ);

  ଯଦି( objc!=1 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "");
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  devsym_unregister();
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** tclcmd: register_jt_vfs ?-default? PARENT-VFS
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI jtObjCmd(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା jt_register(ଅକ୍ଷର *, ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
  ଅକ୍ଷର *zParent = 0;

  ଯଦି( objc!=2 && objc!=3 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "?-default? PARENT-VFS");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  zParent = Tcl_GetString(objv[1]);
  ଯଦି( objc==3 ){
    ଯଦି( strcmp(zParent, "-default") ){
      Tcl_AppendResult(interp,
          "bad option \"", zParent, "\": must be -default", ଖାଲି
      );
      ଫେରନ୍ତୁ TCL_ERROR;
    }
    zParent = Tcl_GetString(objv[2]);
  }

  ଯଦି( !(*zParent) ){
    zParent = 0;
  }
  ଯଦି( jt_register(zParent, objc==3) ){
    Tcl_AppendResult(interp, "Error in jt_register", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  ଫେରନ୍ତୁ TCL_OK;
}

/*
** tclcmd: unregister_jt_vfs
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI jtUnregisterObjCmd(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ଶୂନ୍ୟ jt_unregister(ଶୂନ୍ୟ);

  ଯଦି( objc!=1 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "");
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  jt_unregister();
  ଫେରନ୍ତୁ TCL_OK;
}

#endif /* SQLITE_OMIT_DISKIO */

/*
** This procedure registers the TCL procedures defined in this file.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା Sqlitetest6_Init(Tcl_Interp *interp){
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
  ଫେରନ୍ତୁ TCL_OK;
}

#endif /* SQLITE_TEST */
