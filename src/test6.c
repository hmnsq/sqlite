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
#যদি SQLITE_TEST          /* This file is used for testing only */
#অন্তর্ভাব "sqliteInt.h"
#অন্তর্ভাব "tclsqlite.h"

#ifndef SQLITE_OMIT_DISKIO  /* This file is a no-op if disk I/O is disabled */

/* #define TRACE_CRASHTEST */

প্রকারনির্দেশ সংরচনা CrashFile CrashFile;
প্রকারনির্দেশ সংরচনা CrashGlobal CrashGlobal;
প্রকারনির্দেশ সংরচনা WriteBuffer WriteBuffer;

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
সংরচনা WriteBuffer {
  i64 iOffset;                 /* Byte offset of the start of this write() */
  পূর্ণাঙ্ক nBuf;                    /* Number of bytes written */
  u8 *zBuf;                    /* Pointer to copy of written data */
  CrashFile *pFile;            /* File this write() applies to */

  WriteBuffer *pNext;          /* Next in CrashGlobal.pWriteList */
};

সংরচনা CrashFile {
  ধ্রুব sqlite3_io_methods *pMethod;   /* Must be first */
  sqlite3_file *pRealFile;             /* Underlying "real" file handle */
  বর্ণ *zName;
  পূর্ণাঙ্ক flags;                           /* Flags the file was opened with */

  /* Cache of the entire file. This is used to speed up OsRead() and
  ** OsFileSize() calls. Although both could be done by traversing the
  ** write-list, in practice this is impractically slow.
  */
  u8 *zData;                           /* Buffer containing file contents */
  পূর্ণাঙ্ক nData;                           /* Size of buffer allocated at zData */
  i64 iSize;                           /* Size of file in bytes */
};

সংরচনা CrashGlobal {
  WriteBuffer *pWriteList;     /* Head of write-list */
  WriteBuffer *pWriteListEnd;  /* End of write-list */

  পূর্ণাঙ্ক iSectorSize;             /* Value of simulated sector size */
  পূর্ণাঙ্ক iDeviceCharacteristics;  /* Value of simulated device characteristics */

  পূর্ণাঙ্ক iCrash;                  /* Crash on the iCrash'th call to xSync() */
  বর্ণ zCrashFile[500];        /* Crash during an xSync() on this file */
};

স্থির CrashGlobal g = {0, 0, SQLITE_DEFAULT_SECTOR_SIZE, 0, 0};

/*
** Set this global variable to 1 to enable crash testing.
*/
স্থির পূর্ণাঙ্ক sqlite3CrashTestEnable = 0;

স্থির শূন্য *crash_malloc(পূর্ণাঙ্ক nByte){
  প্রত্যাগম (শূন্য *)Tcl_AttemptAlloc((size_t)nByte);
}
স্থির শূন্য crash_free(শূন্য *p){
  Tcl_Free(p);
}
স্থির শূন্য *crash_realloc(শূন্য *p, পূর্ণাঙ্ক n){
  প্রত্যাগম (শূন্য *)Tcl_AttemptRealloc(p, (size_t)n);
}

/*
** Wrapper around the sqlite3OsWrite() function that avoids writing to the
** 512 byte block beginning at offset PENDING_BYTE.
*/
স্থির পূর্ণাঙ্ক writeDbFile(CrashFile *p, u8 *z, i64 iAmt, i64 iOff){
  পূর্ণাঙ্ক rc = SQLITE_OK;
  পূর্ণাঙ্ক iSkip = 0;
  যদি( (iAmt-iSkip)>0 ){
    rc = sqlite3OsWrite(p->pRealFile, &z[iSkip], (পূর্ণাঙ্ক)(iAmt-iSkip), iOff+iSkip);
  }
  প্রত্যাগম rc;
}

/*
** Flush the write-list as if xSync() had been called on file handle
** pFile. If isCrash is true, simulate a crash.
*/
স্থির পূর্ণাঙ্ক writeListSync(CrashFile *pFile, পূর্ণাঙ্ক isCrash){
  পূর্ণাঙ্ক rc = SQLITE_OK;
  পূর্ণাঙ্ক iDc = g.iDeviceCharacteristics;

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
  যদি( !isCrash ){
    প্রতি(pWrite=g.pWriteList; pWrite; pWrite=pWrite->pNext){
      যদি( pWrite->pFile==pFile ){
        pFinal = pWrite;
      }
    }
  }অন্যথা যদি( iDc&(SQLITE_IOCAP_SEQUENTIAL|SQLITE_IOCAP_SAFE_APPEND) ){
    পূর্ণাঙ্ক nWrite = 0;
    পূর্ণাঙ্ক iFinal;
    প্রতি(pWrite=g.pWriteList; pWrite; pWrite=pWrite->pNext) nWrite++;
    sqlite3_randomness(পরিমাণ(পূর্ণাঙ্ক), &iFinal);
    iFinal = ((iFinal<0)?-1*iFinal:iFinal)%nWrite;
    প্রতি(pWrite=g.pWriteList; iFinal>0; pWrite=pWrite->pNext) iFinal--;
    pFinal = pWrite;
  }

#ifdef TRACE_CRASHTEST
  যদি( pFile ){
    printf("Sync %s (is %s crash)\n", pFile->zName, (isCrash?"a":"not a"));
  }
#endif

  ppPtr = &g.pWriteList;
  প্রতি(pWrite=*ppPtr; rc==SQLITE_OK && pWrite; pWrite=*ppPtr){
    sqlite3_file *pRealFile = pWrite->pFile->pRealFile;

    /* (eAction==1)      -> write block out normally,
    ** (eAction==2)      -> do nothing,
    ** (eAction==3)      -> trash sectors.
    */
    পূর্ণাঙ্ক eAction = 0;
    যদি( !isCrash ){
      eAction = 2;
      যদি( (pWrite->pFile==pFile || iDc&SQLITE_IOCAP_SEQUENTIAL) ){
        eAction = 1;
      }
    }অন্যথা{
      বর্ণ random;
      sqlite3_randomness(1, &random);

      /* Do not select option 3 (sector trashing) if the IOCAP_ATOMIC flag
      ** is set or this is an OsTruncate(), not an Oswrite().
      */
      যদি( (iDc&SQLITE_IOCAP_ATOMIC) || (pWrite->zBuf==0) ){
        random &= 0x01;
      }

      /* If IOCAP_SEQUENTIAL is set and this is not the final entry
      ** in the truncated write-list, always select option 1 (write
      ** out correctly).
      */
      যদি( (iDc&SQLITE_IOCAP_SEQUENTIAL && pWrite!=pFinal) ){
        random = 0;
      }

      /* If IOCAP_SAFE_APPEND is set and this OsWrite() operation is
      ** an append (first byte of the written region is 1 byte past the
      ** current EOF), always select option 1 (write out correctly).
      */
      যদি( iDc&SQLITE_IOCAP_SAFE_APPEND && pWrite->zBuf ){
        i64 iSize;
        sqlite3OsFileSize(pRealFile, &iSize);
        যদি( iSize==pWrite->iOffset ){
          random = 0;
        }
      }

      যদি( (random&0x06)==0x06 ){
        eAction = 3;
      }অন্যথা{
        eAction = ((random&0x01)?2:1);
      }
    }

    বিকল্প( eAction ){
      পক্ষ 1: {               /* Write out correctly */
        যদি( pWrite->zBuf ){
          rc = writeDbFile(
              pWrite->pFile, pWrite->zBuf, pWrite->nBuf, pWrite->iOffset
          );
        }অন্যথা{
          rc = sqlite3OsTruncate(pRealFile, pWrite->iOffset);
        }
        *ppPtr = pWrite->pNext;
#ifdef TRACE_CRASHTEST
        যদি( isCrash ){
          printf("Writing %d bytes @ %d (%s)\n",
            pWrite->nBuf, (পূর্ণাঙ্ক)pWrite->iOffset, pWrite->pFile->zName
          );
        }
#endif
        crash_free(pWrite);
        ভঙ্গ;
      }
      পক্ষ 2: {               /* Do nothing */
        ppPtr = &pWrite->pNext;
#ifdef TRACE_CRASHTEST
        যদি( isCrash ){
          printf("Omiting %d bytes @ %d (%s)\n",
            pWrite->nBuf, (পূর্ণাঙ্ক)pWrite->iOffset, pWrite->pFile->zName
          );
        }
#endif
        ভঙ্গ;
      }
      পক্ষ 3: {               /* Trash sectors */
        u8 *zGarbage;
        পূর্ণাঙ্ক iFirst = (পূর্ণাঙ্ক)(pWrite->iOffset/g.iSectorSize);
        পূর্ণাঙ্ক iLast = (পূর্ণাঙ্ক)((pWrite->iOffset+pWrite->nBuf-1)/g.iSectorSize);

        প্রতিজ্ঞা(pWrite->zBuf);

#ifdef TRACE_CRASHTEST
        printf("Trashing %d sectors (%d bytes) @ %lld (sector %d) (%s)\n",
            1+iLast-iFirst, (1+iLast-iFirst)*g.iSectorSize,
            pWrite->iOffset, iFirst, pWrite->pFile->zName
        );
#endif

        zGarbage = crash_malloc(g.iSectorSize);
        যদি( zGarbage ){
          sqlite3_int64 i;
          প্রতি(i=iFirst; rc==SQLITE_OK && i<=iLast; i++){
            sqlite3_randomness(g.iSectorSize, zGarbage);
            rc = writeDbFile(
              pWrite->pFile, zGarbage, g.iSectorSize, i*g.iSectorSize
            );
          }
          crash_free(zGarbage);
        }অন্যথা{
          rc = SQLITE_NOMEM;
        }

        ppPtr = &pWrite->pNext;
        ভঙ্গ;
      }

      স্বভাব:
        প্রতিজ্ঞা(!"Cannot happen");
    }

    যদি( pWrite==pFinal ) ভঙ্গ;
  }

  যদি( rc==SQLITE_OK && isCrash ){
    exit(-1);
  }

  প্রতি(pWrite=g.pWriteList; pWrite && pWrite->pNext; pWrite=pWrite->pNext);
  g.pWriteListEnd = pWrite;

  প্রত্যাগম rc;
}

/*
** Add an entry to the end of the write-list.
*/
স্থির পূর্ণাঙ্ক writeListAppend(
  sqlite3_file *pFile,
  sqlite3_int64 iOffset,
  ধ্রুব u8 *zBuf,
  পূর্ণাঙ্ক nBuf
){
  WriteBuffer *pNew;

  প্রতিজ্ঞা((zBuf && nBuf) || (!nBuf && !zBuf));

  pNew = (WriteBuffer *)crash_malloc(পরিমাণ(WriteBuffer) + nBuf);
  যদি( pNew==0 ){
    fprintf(stderr, "out of memory in the crash simulator\n");
  }
  memset(pNew, 0, পরিমাণ(WriteBuffer)+nBuf);
  pNew->iOffset = iOffset;
  pNew->nBuf = nBuf;
  pNew->pFile = (CrashFile *)pFile;
  যদি( zBuf ){
    pNew->zBuf = (u8 *)&pNew[1];
    memcpy(pNew->zBuf, zBuf, nBuf);
  }

  যদি( g.pWriteList ){
    প্রতিজ্ঞা(g.pWriteListEnd);
    g.pWriteListEnd->pNext = pNew;
  }অন্যথা{
    g.pWriteList = pNew;
  }
  g.pWriteListEnd = pNew;
 
  প্রত্যাগম SQLITE_OK;
}

/*
** Close a crash-file.
*/
স্থির পূর্ণাঙ্ক cfClose(sqlite3_file *pFile){
  CrashFile *pCrash = (CrashFile *)pFile;
  writeListSync(pCrash, 0);
  sqlite3OsClose(pCrash->pRealFile);
  প্রত্যাগম SQLITE_OK;
}

/*
** Read data from a crash-file.
*/
স্থির পূর্ণাঙ্ক cfRead(
  sqlite3_file *pFile,
  শূন্য *zBuf,
  পূর্ণাঙ্ক iAmt,
  sqlite_int64 iOfst
){
  CrashFile *pCrash = (CrashFile *)pFile;
  পূর্ণাঙ্ক nCopy = (পূর্ণাঙ্ক)MIN((i64)iAmt, (pCrash->iSize - iOfst));

  যদি( nCopy>0 ){
    memcpy(zBuf, &pCrash->zData[iOfst], nCopy);
  }

  /* Check the file-size to see if this is a short-read */
  যদি( nCopy<iAmt ){
    প্রত্যাগম SQLITE_IOERR_SHORT_READ;
  }

  প্রত্যাগম SQLITE_OK;
}

/*
** Write data to a crash-file.
*/
স্থির পূর্ণাঙ্ক cfWrite(
  sqlite3_file *pFile,
  ধ্রুব শূন্য *zBuf,
  পূর্ণাঙ্ক iAmt,
  sqlite_int64 iOfst
){
  CrashFile *pCrash = (CrashFile *)pFile;
  যদি( iAmt+iOfst>pCrash->iSize ){
    pCrash->iSize = (পূর্ণাঙ্ক)(iAmt+iOfst);
  }
  যাবত্( pCrash->iSize>pCrash->nData ){
    u8 *zNew;
    পূর্ণাঙ্ক nNew = (pCrash->nData*2) + 4096;
    zNew = crash_realloc(pCrash->zData, nNew);
    যদি( !zNew ){
      প্রত্যাগম SQLITE_NOMEM;
    }
    memset(&zNew[pCrash->nData], 0, nNew-pCrash->nData);
    pCrash->nData = nNew;
    pCrash->zData = zNew;
  }
  memcpy(&pCrash->zData[iOfst], zBuf, iAmt);
  প্রত্যাগম writeListAppend(pFile, iOfst, zBuf, iAmt);
}

/*
** Truncate a crash-file.
*/
স্থির পূর্ণাঙ্ক cfTruncate(sqlite3_file *pFile, sqlite_int64 size){
  CrashFile *pCrash = (CrashFile *)pFile;
  প্রতিজ্ঞা(size>=0);
  যদি( pCrash->iSize>size ){
    pCrash->iSize = (পূর্ণাঙ্ক)size;
  }
  প্রত্যাগম writeListAppend(pFile, size, 0, 0);
}

/*
** Sync a crash-file.
*/
স্থির পূর্ণাঙ্ক cfSync(sqlite3_file *pFile, পূর্ণাঙ্ক flags){
  CrashFile *pCrash = (CrashFile *)pFile;
  পূর্ণাঙ্ক isCrash = 0;

  ধ্রুব বর্ণ *zName = pCrash->zName;
  ধ্রুব বর্ণ *zCrashFile = g.zCrashFile;
  পূর্ণাঙ্ক nName = (পূর্ণাঙ্ক)strlen(zName);
  পূর্ণাঙ্ক nCrashFile = (পূর্ণাঙ্ক)strlen(zCrashFile);

  যদি( nCrashFile>0 && zCrashFile[nCrashFile-1]=='*' ){
    nCrashFile--;
    যদি( nName>nCrashFile ) nName = nCrashFile;
  }

#ifdef TRACE_CRASHTEST
  printf("cfSync(): nName = %d, nCrashFile = %d, zName = %s, zCrashFile = %s\n",
         nName, nCrashFile, zName, zCrashFile);
#endif

  যদি( nName==nCrashFile && 0==memcmp(zName, zCrashFile, nName) ){
#ifdef TRACE_CRASHTEST
    printf("cfSync(): name matched, g.iCrash = %d\n", g.iCrash);
#endif
    যদি( (--g.iCrash)==0 ) isCrash = 1;
  }

  প্রত্যাগম writeListSync(pCrash, isCrash);
}

/*
** Return the current file-size of the crash-file.
*/
স্থির পূর্ণাঙ্ক cfFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  CrashFile *pCrash = (CrashFile *)pFile;
  *pSize = (i64)pCrash->iSize;
  প্রত্যাগম SQLITE_OK;
}

/*
** Calls related to file-locks are passed on to the real file handle.
*/
স্থির পূর্ণাঙ্ক cfLock(sqlite3_file *pFile, পূর্ণাঙ্ক eLock){
  প্রত্যাগম sqlite3OsLock(((CrashFile *)pFile)->pRealFile, eLock);
}
স্থির পূর্ণাঙ্ক cfUnlock(sqlite3_file *pFile, পূর্ণাঙ্ক eLock){
  প্রত্যাগম sqlite3OsUnlock(((CrashFile *)pFile)->pRealFile, eLock);
}
স্থির পূর্ণাঙ্ক cfCheckReservedLock(sqlite3_file *pFile, পূর্ণাঙ্ক *pResOut){
  প্রত্যাগম sqlite3OsCheckReservedLock(((CrashFile *)pFile)->pRealFile, pResOut);
}
স্থির পূর্ণাঙ্ক cfFileControl(sqlite3_file *pFile, পূর্ণাঙ্ক op, শূন্য *pArg){
  যদি( op==SQLITE_FCNTL_SIZE_HINT ){
    CrashFile *pCrash = (CrashFile *)pFile;
    i64 nByte = *(i64 *)pArg;
    যদি( nByte>pCrash->iSize ){
      যদি( SQLITE_OK==writeListAppend(pFile, nByte, 0, 0) ){
        pCrash->iSize = (পূর্ণাঙ্ক)nByte;
      }
    }
    প্রত্যাগম SQLITE_OK;
  }
  প্রত্যাগম sqlite3OsFileControl(((CrashFile *)pFile)->pRealFile, op, pArg);
}

/*
** The xSectorSize() and xDeviceCharacteristics() functions return
** the global values configured by the [sqlite_crashparams] tcl
*  interface.
*/
স্থির পূর্ণাঙ্ক cfSectorSize(sqlite3_file *pFile){
  প্রত্যাগম g.iSectorSize;
}
স্থির পূর্ণাঙ্ক cfDeviceCharacteristics(sqlite3_file *pFile){
  প্রত্যাগম g.iDeviceCharacteristics;
}

/*
** Pass-throughs for WAL support.
*/
স্থির পূর্ণাঙ্ক cfShmLock(sqlite3_file *pFile, পূর্ণাঙ্ক ofst, পূর্ণাঙ্ক n, পূর্ণাঙ্ক flags){
  sqlite3_file *pReal = ((CrashFile*)pFile)->pRealFile;
  প্রত্যাগম pReal->pMethods->xShmLock(pReal, ofst, n, flags);
}
স্থির শূন্য cfShmBarrier(sqlite3_file *pFile){
  sqlite3_file *pReal = ((CrashFile*)pFile)->pRealFile;
  pReal->pMethods->xShmBarrier(pReal);
}
স্থির পূর্ণাঙ্ক cfShmUnmap(sqlite3_file *pFile, পূর্ণাঙ্ক delFlag){
  sqlite3_file *pReal = ((CrashFile*)pFile)->pRealFile;
  প্রত্যাগম pReal->pMethods->xShmUnmap(pReal, delFlag);
}
স্থির পূর্ণাঙ্ক cfShmMap(
  sqlite3_file *pFile,            /* Handle open on database file */
  পূর্ণাঙ্ক iRegion,                    /* Region to retrieve */
  পূর্ণাঙ্ক sz,                         /* Size of regions */
  পূর্ণাঙ্ক w,                          /* True to extend file if necessary */
  শূন্য চঞ্চল **pp              /* OUT: Mapped memory */
){
  sqlite3_file *pReal = ((CrashFile*)pFile)->pRealFile;
  প্রত্যাগম pReal->pMethods->xShmMap(pReal, iRegion, sz, w, pp);
}

স্থির ধ্রুব sqlite3_io_methods CrashFileVtab = {
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
সংরচনা crashAppData {
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
স্থির পূর্ণাঙ্ক cfOpen(
  sqlite3_vfs *pCfVfs,
  ধ্রুব বর্ণ *zName,
  sqlite3_file *pFile,
  পূর্ণাঙ্ক flags,
  পূর্ণাঙ্ক *pOutFlags
){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  পূর্ণাঙ্ক rc;
  CrashFile *pWrapper = (CrashFile *)pFile;
  sqlite3_file *pReal = (sqlite3_file*)&pWrapper[1];

  memset(pWrapper, 0, পরিমাণ(CrashFile));
  rc = sqlite3OsOpen(pVfs, zName, pReal, flags, pOutFlags);

  যদি( rc==SQLITE_OK ){
    i64 iSize;
    pWrapper->pMethod = &CrashFileVtab;
    pWrapper->zName = (বর্ণ *)zName;
    pWrapper->pRealFile = pReal;
    rc = sqlite3OsFileSize(pReal, &iSize);
    pWrapper->iSize = (পূর্ণাঙ্ক)iSize;
    pWrapper->flags = flags;
  }
  যদি( rc==SQLITE_OK ){
    pWrapper->nData = (পূর্ণাঙ্ক)(4096 + pWrapper->iSize);
    pWrapper->zData = crash_malloc(pWrapper->nData);
    যদি( pWrapper->zData ){
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
      প্রতি(iOff=0; iOff<pWrapper->iSize; iOff += 512){
        পূর্ণাঙ্ক nRead = (পূর্ণাঙ্ক)(pWrapper->iSize - iOff);
        যদি( nRead>512 ) nRead = 512;
        rc = sqlite3OsRead(pReal, &pWrapper->zData[iOff], nRead, iOff);
      }
    }অন্যথা{
      rc = SQLITE_NOMEM;
    }
  }
  যদি( rc!=SQLITE_OK && pWrapper->pMethod ){
    sqlite3OsClose(pFile);
  }
  প্রত্যাগম rc;
}

স্থির পূর্ণাঙ্ক cfDelete(sqlite3_vfs *pCfVfs, ধ্রুব বর্ণ *zPath, পূর্ণাঙ্ক dirSync){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  প্রত্যাগম pVfs->xDelete(pVfs, zPath, dirSync);
}
স্থির পূর্ণাঙ্ক cfAccess(
  sqlite3_vfs *pCfVfs,
  ধ্রুব বর্ণ *zPath,
  পূর্ণাঙ্ক flags,
  পূর্ণাঙ্ক *pResOut
){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  প্রত্যাগম pVfs->xAccess(pVfs, zPath, flags, pResOut);
}
স্থির পূর্ণাঙ্ক cfFullPathname(
  sqlite3_vfs *pCfVfs,
  ধ্রুব বর্ণ *zPath,
  পূর্ণাঙ্ক nPathOut,
  বর্ণ *zPathOut
){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  প্রত্যাগম pVfs->xFullPathname(pVfs, zPath, nPathOut, zPathOut);
}
স্থির শূন্য *cfDlOpen(sqlite3_vfs *pCfVfs, ধ্রুব বর্ণ *zPath){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  প্রত্যাগম pVfs->xDlOpen(pVfs, zPath);
}
স্থির শূন্য cfDlError(sqlite3_vfs *pCfVfs, পূর্ণাঙ্ক nByte, বর্ণ *z_Err_বার্তা){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  pVfs->xDlError(pVfs, nByte, z_Err_বার্তা);
}
স্থির শূন্য (*cfDlSym(sqlite3_vfs *pCfVfs, শূন্য *pH, ধ্রুব বর্ণ *zSym))(শূন্য){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  প্রত্যাগম pVfs->xDlSym(pVfs, pH, zSym);
}
স্থির শূন্য cfDlClose(sqlite3_vfs *pCfVfs, শূন্য *pHandle){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  pVfs->xDlClose(pVfs, pHandle);
}
স্থির পূর্ণাঙ্ক cfRandomness(sqlite3_vfs *pCfVfs, পূর্ণাঙ্ক nByte, বর্ণ *zBufOut){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  প্রত্যাগম pVfs->xRandomness(pVfs, nByte, zBufOut);
}
স্থির পূর্ণাঙ্ক cfSleep(sqlite3_vfs *pCfVfs, পূর্ণাঙ্ক nMicro){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  প্রত্যাগম pVfs->xSleep(pVfs, nMicro);
}
স্থির পূর্ণাঙ্ক cfCurrentTime(sqlite3_vfs *pCfVfs, দ্বিগুণ *pTimeOut){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  প্রত্যাগম pVfs->xCurrentTime(pVfs, pTimeOut);
}
স্থির পূর্ণাঙ্ক cfGetLastError(sqlite3_vfs *pCfVfs, পূর্ণাঙ্ক n, বর্ণ *z){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  প্রত্যাগম pVfs->xGetLastError(pVfs, n, z);
}

স্থির পূর্ণাঙ্ক processDevSymArgs(
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[],
  পূর্ণাঙ্ক *piDeviceChar,
  পূর্ণাঙ্ক *piSectorSize
){
  সংরচনা DeviceFlag {
    বর্ণ *zName;
    পূর্ণাঙ্ক i_মান;
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

  পূর্ণাঙ্ক i;
  পূর্ণাঙ্ক iDc = 0;
  পূর্ণাঙ্ক iSectorSize = 0;
  পূর্ণাঙ্ক setSectorsize = 0;
  পূর্ণাঙ্ক setDeviceChar = 0;

  প্রতি(i=0; i<objc; i+=2){
    Tcl_Size nOpt;
    বর্ণ *zOpt = Tcl_GetStringFromObj(objv[i], &nOpt);

    যদি( (nOpt>11 || nOpt<2 || strncmp("-sectorsize", zOpt, nOpt))
     && (nOpt>16 || nOpt<2 || strncmp("-characteristics", zOpt, nOpt))
    ){
      Tcl_AppendResult(interp,
        "Bad option: \"", zOpt,
        "\" - must be \"-characteristics\" or \"-sectorsize\"", রিক্ত
      );
      প্রত্যাগম TCL_ERROR;
    }
    যদি( i==objc-1 ){
      Tcl_AppendResult(interp, "Option requires an argument: \"", zOpt, "\"", রিক্ত);
      প্রত্যাগম TCL_ERROR;
    }

    যদি( zOpt[1]=='s' ){
      যদি( Tcl_GetIntFromObj(interp, objv[i+1], &iSectorSize) ){
        প্রত্যাগম TCL_ERROR;
      }
      setSectorsize = 1;
    }অন্যথা{
      পূর্ণাঙ্ক j;
      Tcl_Obj **apObj;
      Tcl_Size nObj;
      যদি( Tcl_ListObjGetElements(interp, objv[i+1], &nObj, &apObj) ){
        প্রত্যাগম TCL_ERROR;
      }
      প্রতি(j=0; j<(পূর্ণাঙ্ক)nObj; j++){
        পূর্ণাঙ্ক rc;
        পূর্ণাঙ্ক iChoice;
        Tcl_Obj *pFlag = Tcl_DuplicateObj(apObj[j]);
        Tcl_Incr_Ref_গুনতি(pFlag);
        Tcl_UtfToLower(Tcl_GetString(pFlag));

        rc = Tcl_GetIndexFromObjStruct(
            interp, pFlag, aFlag, পরিমাণ(aFlag[0]), "no such flag", 0, &iChoice
        );
        Tcl_Decr_Ref_গুনতি(pFlag);
        যদি( rc ){
          প্রত্যাগম TCL_ERROR;
        }

        iDc |= aFlag[iChoice].i_মান;
      }
      setDeviceChar = 1;
    }
  }

  যদি( setDeviceChar ){
    *piDeviceChar = iDc;
  }
  যদি( setSectorsize ){
    *piSectorSize = iSectorSize;
  }

  প্রত্যাগম TCL_OK;
}

/*
** tclcmd:   sqlite3_crash_now
**
** Simulate a crash immediately. This function does not return
** (writeListSync() calls exit(-1)).
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI crashNowCmd(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  যদি( objc!=1 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "");
    প্রত্যাগম TCL_ERROR;
  }
  writeListSync(0, 1);
  প্রতিজ্ঞা( 0 );
  প্রত্যাগম TCL_OK;
}

/*
** tclcmd:   sqlite_crash_enable ENABLE ?DEFAULT?
**
** Parameter ENABLE must be a boolean value. If true, then the "crash"
** vfs is added to the system. If false, it is removed.
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI crashEnableCmd(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  পূর্ণাঙ্ক isEnable;
  পূর্ণাঙ্ক isDefault = 0;
  স্থির sqlite3_vfs crashVfs = {
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

  যদি( objc!=2 && objc!=3 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "ENABLE ?DEFAULT?");
    প্রত্যাগম TCL_ERROR;
  }

  যদি( Tcl_GetBooleanFromObj(interp, objv[1], &isEnable) ){
    প্রত্যাগম TCL_ERROR;
  }
  যদি( objc==3 && Tcl_GetBooleanFromObj(interp, objv[2], &isDefault) ){
    প্রত্যাগম TCL_ERROR;
  }

  যদি( (isEnable && crashVfs.pAppData) || (!isEnable && !crashVfs.pAppData) ){
    প্রত্যাগম TCL_OK;
  }

  যদি( crashVfs.pAppData==0 ){
    sqlite3_vfs *pOriginalVfs = sqlite3_vfs_find(0);
    crashVfs.mxPathname = pOriginalVfs->mxPathname;
    crashVfs.pAppData = (শূন্য *)pOriginalVfs;
    crashVfs.szOsFile = পরিমাণ(CrashFile) + pOriginalVfs->szOsFile;
    sqlite3_vfs_register(&crashVfs, isDefault);
  }অন্যথা{
    crashVfs.pAppData = 0;
    sqlite3_vfs_unregister(&crashVfs);
  }

  প্রত্যাগম TCL_OK;
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
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI crashParamsObjCmd(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  পূর্ণাঙ্ক iDelay;
  ধ্রুব বর্ণ *zCrashFile;
  Tcl_Size nCrashFile;
  পূর্ণাঙ্ক iDc, iSectorSize;

  iDc = -1;
  iSectorSize = -1;

  যদি( objc<3 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "?OPTIONS? DELAY CRASHFILE");
    গচ্ছ error;
  }

  zCrashFile = Tcl_GetStringFromObj(objv[objc-1], &nCrashFile);
  যদি( nCrashFile>=পরিমাণ(g.zCrashFile) ){
    Tcl_AppendResult(interp, "Filename is too long: \"", zCrashFile, "\"", রিক্ত);
    গচ্ছ error;
  }
  যদি( Tcl_GetIntFromObj(interp, objv[objc-2], &iDelay) ){
    গচ্ছ error;
  }

  যদি( processDevSymArgs(interp, objc-3, &objv[1], &iDc, &iSectorSize) ){
    প্রত্যাগম TCL_ERROR;
  }

  যদি( iDc>=0 ){
    g.iDeviceCharacteristics = iDc;
  }
  যদি( iSectorSize>=0 ){
    g.iSectorSize = iSectorSize;
  }

  g.iCrash = iDelay;
  memcpy(g.zCrashFile, zCrashFile, nCrashFile+1);
  sqlite3CrashTestEnable = 1;
  প্রত্যাগম TCL_OK;

error:
  প্রত্যাগম TCL_ERROR;
}

স্থির পূর্ণাঙ্ক SQLITE_TCLAPI devSymObjCmd(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  শূন্য devsym_register(পূর্ণাঙ্ক iDeviceChar, পূর্ণাঙ্ক iSectorSize);

  পূর্ণাঙ্ক iDc = -1;
  পূর্ণাঙ্ক iSectorSize = -1;

  যদি( processDevSymArgs(interp, objc-1, &objv[1], &iDc, &iSectorSize) ){
    প্রত্যাগম TCL_ERROR;
  }
  devsym_register(iDc, iSectorSize);

  প্রত্যাগম TCL_OK;
}

/*
** tclcmd: sqlite3_crash_on_write N
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI writeCrashObjCmd(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  শূন্য devsym_crash_on_write(পূর্ণাঙ্ক);
  পূর্ণাঙ্ক nWrite = 0;

  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "NWRITE");
    প্রত্যাগম TCL_ERROR;
  }
  যদি( Tcl_GetIntFromObj(interp, objv[1], &nWrite) ){
    প্রত্যাগম TCL_ERROR;
  }

  devsym_crash_on_write(nWrite);
  প্রত্যাগম TCL_OK;
}

/*
** tclcmd: unregister_devsim
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI dsUnregisterObjCmd(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  শূন্য devsym_unregister(শূন্য);

  যদি( objc!=1 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "");
    প্রত্যাগম TCL_ERROR;
  }

  devsym_unregister();
  প্রত্যাগম TCL_OK;
}

/*
** tclcmd: register_jt_vfs ?-default? PARENT-VFS
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI jtObjCmd(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  পূর্ণাঙ্ক jt_register(বর্ণ *, পূর্ণাঙ্ক);
  বর্ণ *zParent = 0;

  যদি( objc!=2 && objc!=3 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "?-default? PARENT-VFS");
    প্রত্যাগম TCL_ERROR;
  }
  zParent = Tcl_GetString(objv[1]);
  যদি( objc==3 ){
    যদি( strcmp(zParent, "-default") ){
      Tcl_AppendResult(interp,
          "bad option \"", zParent, "\": must be -default", রিক্ত
      );
      প্রত্যাগম TCL_ERROR;
    }
    zParent = Tcl_GetString(objv[2]);
  }

  যদি( !(*zParent) ){
    zParent = 0;
  }
  যদি( jt_register(zParent, objc==3) ){
    Tcl_AppendResult(interp, "Error in jt_register", রিক্ত);
    প্রত্যাগম TCL_ERROR;
  }

  প্রত্যাগম TCL_OK;
}

/*
** tclcmd: unregister_jt_vfs
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI jtUnregisterObjCmd(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  শূন্য jt_unregister(শূন্য);

  যদি( objc!=1 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "");
    প্রত্যাগম TCL_ERROR;
  }

  jt_unregister();
  প্রত্যাগম TCL_OK;
}

#endif /* SQLITE_OMIT_DISKIO */

/*
** This procedure registers the TCL procedures defined in this file.
*/
পূর্ণাঙ্ক Sqlitetest6_Init(Tcl_Interp *interp){
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
  প্রত্যাগম TCL_OK;
}

#endif /* SQLITE_TEST */
