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
#सचे SQLITE_TEST          /* This file is used for testing only */
#अन्तर्भाव "sqliteInt.h"
#अन्तर्भाव "tclsqlite.h"

#ifndef SQLITE_OMIT_DISKIO  /* This file is a no-op if disk I/O is disabled */

/* #define TRACE_CRASHTEST */

प्रकारनिर्देश संरचना CrashFile CrashFile;
प्रकारनिर्देश संरचना CrashGlobal CrashGlobal;
प्रकारनिर्देश संरचना WriteBuffer WriteBuffer;

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
संरचना WriteBuffer {
  i64 iOffset;                 /* Byte offset of the start of this write() */
  पूर्णाङ्क nBuf;                    /* Number of bytes written */
  u8 *zBuf;                    /* Pointer to copy of written data */
  CrashFile *pFile;            /* File this write() applies to */

  WriteBuffer *pNext;          /* Next in CrashGlobal.pWriteList */
};

संरचना CrashFile {
  ध्रुव sqlite3_io_methods *pMethod;   /* Must be first */
  sqlite3_file *pRealFile;             /* Underlying "real" file handle */
  वर्ण *zName;
  पूर्णाङ्क flags;                           /* Flags the file was opened with */

  /* Cache of the entire file. This is used to speed up OsRead() and
  ** OsFileSize() calls. Although both could be done by traversing the
  ** write-list, in practice this is impractically slow.
  */
  u8 *zData;                           /* Buffer containing file contents */
  पूर्णाङ्क nData;                           /* Size of buffer allocated at zData */
  i64 iSize;                           /* Size of file in bytes */
};

संरचना CrashGlobal {
  WriteBuffer *pWriteList;     /* Head of write-list */
  WriteBuffer *pWriteListEnd;  /* End of write-list */

  पूर्णाङ्क iSectorSize;             /* Value of simulated sector size */
  पूर्णाङ्क iDeviceCharacteristics;  /* Value of simulated device characteristics */

  पूर्णाङ्क iCrash;                  /* Crash on the iCrash'th call to xSync() */
  वर्ण zCrashFile[500];        /* Crash during an xSync() on this file */
};

स्थिर CrashGlobal g = {0, 0, SQLITE_DEFAULT_SECTOR_SIZE, 0, 0};

/*
** Set this global variable to 1 to enable crash testing.
*/
स्थिर पूर्णाङ्क sqlite3CrashTestEnable = 0;

स्थिर शून्य *crash_malloc(पूर्णाङ्क nByte){
  पटिनिवत्त (शून्य *)Tcl_AttemptAlloc((size_t)nByte);
}
स्थिर शून्य crash_free(शून्य *p){
  Tcl_Free(p);
}
स्थिर शून्य *crash_realloc(शून्य *p, पूर्णाङ्क n){
  पटिनिवत्त (शून्य *)Tcl_AttemptRealloc(p, (size_t)n);
}

/*
** Wrapper around the sqlite3OsWrite() function that avoids writing to the
** 512 byte block beginning at offset PENDING_BYTE.
*/
स्थिर पूर्णाङ्क writeDbFile(CrashFile *p, u8 *z, i64 iAmt, i64 iOff){
  पूर्णाङ्क rc = SQLITE_OK;
  पूर्णाङ्क iSkip = 0;
  सचे( (iAmt-iSkip)>0 ){
    rc = sqlite3OsWrite(p->pRealFile, &z[iSkip], (पूर्णाङ्क)(iAmt-iSkip), iOff+iSkip);
  }
  पटिनिवत्त rc;
}

/*
** Flush the write-list as if xSync() had been called on file handle
** pFile. If isCrash is true, simulate a crash.
*/
स्थिर पूर्णाङ्क writeListSync(CrashFile *pFile, पूर्णाङ्क isCrash){
  पूर्णाङ्क rc = SQLITE_OK;
  पूर्णाङ्क iDc = g.iDeviceCharacteristics;

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
  सचे( !isCrash ){
    अत्थाय(pWrite=g.pWriteList; pWrite; pWrite=pWrite->pNext){
      सचे( pWrite->pFile==pFile ){
        pFinal = pWrite;
      }
    }
  }अञ्ञथा सचे( iDc&(SQLITE_IOCAP_SEQUENTIAL|SQLITE_IOCAP_SAFE_APPEND) ){
    पूर्णाङ्क nWrite = 0;
    पूर्णाङ्क iFinal;
    अत्थाय(pWrite=g.pWriteList; pWrite; pWrite=pWrite->pNext) nWrite++;
    sqlite3_randomness(परिमाण(पूर्णाङ्क), &iFinal);
    iFinal = ((iFinal<0)?-1*iFinal:iFinal)%nWrite;
    अत्थाय(pWrite=g.pWriteList; iFinal>0; pWrite=pWrite->pNext) iFinal--;
    pFinal = pWrite;
  }

#ifdef TRACE_CRASHTEST
  सचे( pFile ){
    printf("Sync %s (is %s crash)\n", pFile->zName, (isCrash?"a":"not a"));
  }
#endif

  ppPtr = &g.pWriteList;
  अत्थाय(pWrite=*ppPtr; rc==SQLITE_OK && pWrite; pWrite=*ppPtr){
    sqlite3_file *pRealFile = pWrite->pFile->pRealFile;

    /* (eAction==1)      -> write block out normally,
    ** (eAction==2)      -> do nothing,
    ** (eAction==3)      -> trash sectors.
    */
    पूर्णाङ्क eAction = 0;
    सचे( !isCrash ){
      eAction = 2;
      सचे( (pWrite->pFile==pFile || iDc&SQLITE_IOCAP_SEQUENTIAL) ){
        eAction = 1;
      }
    }अञ्ञथा{
      वर्ण random;
      sqlite3_randomness(1, &random);

      /* Do not select option 3 (sector trashing) if the IOCAP_ATOMIC flag
      ** is set or this is an OsTruncate(), not an Oswrite().
      */
      सचे( (iDc&SQLITE_IOCAP_ATOMIC) || (pWrite->zBuf==0) ){
        random &= 0x01;
      }

      /* If IOCAP_SEQUENTIAL is set and this is not the final entry
      ** in the truncated write-list, always select option 1 (write
      ** out correctly).
      */
      सचे( (iDc&SQLITE_IOCAP_SEQUENTIAL && pWrite!=pFinal) ){
        random = 0;
      }

      /* If IOCAP_SAFE_APPEND is set and this OsWrite() operation is
      ** an append (first byte of the written region is 1 byte past the
      ** current EOF), always select option 1 (write out correctly).
      */
      सचे( iDc&SQLITE_IOCAP_SAFE_APPEND && pWrite->zBuf ){
        i64 iSize;
        sqlite3OsFileSize(pRealFile, &iSize);
        सचे( iSize==pWrite->iOffset ){
          random = 0;
        }
      }

      सचे( (random&0x06)==0x06 ){
        eAction = 3;
      }अञ्ञथा{
        eAction = ((random&0x01)?2:1);
      }
    }

    विकल्प( eAction ){
      पक्ष 1: {               /* Write out correctly */
        सचे( pWrite->zBuf ){
          rc = writeDbFile(
              pWrite->pFile, pWrite->zBuf, pWrite->nBuf, pWrite->iOffset
          );
        }अञ्ञथा{
          rc = sqlite3OsTruncate(pRealFile, pWrite->iOffset);
        }
        *ppPtr = pWrite->pNext;
#ifdef TRACE_CRASHTEST
        सचे( isCrash ){
          printf("Writing %d bytes @ %d (%s)\n",
            pWrite->nBuf, (पूर्णाङ्क)pWrite->iOffset, pWrite->pFile->zName
          );
        }
#endif
        crash_free(pWrite);
        भङ्ग;
      }
      पक्ष 2: {               /* Do nothing */
        ppPtr = &pWrite->pNext;
#ifdef TRACE_CRASHTEST
        सचे( isCrash ){
          printf("Omiting %d bytes @ %d (%s)\n",
            pWrite->nBuf, (पूर्णाङ्क)pWrite->iOffset, pWrite->pFile->zName
          );
        }
#endif
        भङ्ग;
      }
      पक्ष 3: {               /* Trash sectors */
        u8 *zGarbage;
        पूर्णाङ्क iFirst = (पूर्णाङ्क)(pWrite->iOffset/g.iSectorSize);
        पूर्णाङ्क iLast = (पूर्णाङ्क)((pWrite->iOffset+pWrite->nBuf-1)/g.iSectorSize);

        प्रतिज्ञा(pWrite->zBuf);

#ifdef TRACE_CRASHTEST
        printf("Trashing %d sectors (%d bytes) @ %lld (sector %d) (%s)\n",
            1+iLast-iFirst, (1+iLast-iFirst)*g.iSectorSize,
            pWrite->iOffset, iFirst, pWrite->pFile->zName
        );
#endif

        zGarbage = crash_malloc(g.iSectorSize);
        सचे( zGarbage ){
          sqlite3_int64 i;
          अत्थाय(i=iFirst; rc==SQLITE_OK && i<=iLast; i++){
            sqlite3_randomness(g.iSectorSize, zGarbage);
            rc = writeDbFile(
              pWrite->pFile, zGarbage, g.iSectorSize, i*g.iSectorSize
            );
          }
          crash_free(zGarbage);
        }अञ्ञथा{
          rc = SQLITE_NOMEM;
        }

        ppPtr = &pWrite->pNext;
        भङ्ग;
      }

      स्वभाव:
        प्रतिज्ञा(!"Cannot happen");
    }

    सचे( pWrite==pFinal ) भङ्ग;
  }

  सचे( rc==SQLITE_OK && isCrash ){
    exit(-1);
  }

  अत्थाय(pWrite=g.pWriteList; pWrite && pWrite->pNext; pWrite=pWrite->pNext);
  g.pWriteListEnd = pWrite;

  पटिनिवत्त rc;
}

/*
** Add an entry to the end of the write-list.
*/
स्थिर पूर्णाङ्क writeListAppend(
  sqlite3_file *pFile,
  sqlite3_int64 iOffset,
  ध्रुव u8 *zBuf,
  पूर्णाङ्क nBuf
){
  WriteBuffer *pNew;

  प्रतिज्ञा((zBuf && nBuf) || (!nBuf && !zBuf));

  pNew = (WriteBuffer *)crash_malloc(परिमाण(WriteBuffer) + nBuf);
  सचे( pNew==0 ){
    fprintf(stderr, "out of memory in the crash simulator\n");
  }
  memset(pNew, 0, परिमाण(WriteBuffer)+nBuf);
  pNew->iOffset = iOffset;
  pNew->nBuf = nBuf;
  pNew->pFile = (CrashFile *)pFile;
  सचे( zBuf ){
    pNew->zBuf = (u8 *)&pNew[1];
    memcpy(pNew->zBuf, zBuf, nBuf);
  }

  सचे( g.pWriteList ){
    प्रतिज्ञा(g.pWriteListEnd);
    g.pWriteListEnd->pNext = pNew;
  }अञ्ञथा{
    g.pWriteList = pNew;
  }
  g.pWriteListEnd = pNew;
 
  पटिनिवत्त SQLITE_OK;
}

/*
** Close a crash-file.
*/
स्थिर पूर्णाङ्क cfClose(sqlite3_file *pFile){
  CrashFile *pCrash = (CrashFile *)pFile;
  writeListSync(pCrash, 0);
  sqlite3OsClose(pCrash->pRealFile);
  पटिनिवत्त SQLITE_OK;
}

/*
** Read data from a crash-file.
*/
स्थिर पूर्णाङ्क cfRead(
  sqlite3_file *pFile,
  शून्य *zBuf,
  पूर्णाङ्क iAmt,
  sqlite_int64 iOfst
){
  CrashFile *pCrash = (CrashFile *)pFile;
  पूर्णाङ्क nCopy = (पूर्णाङ्क)MIN((i64)iAmt, (pCrash->iSize - iOfst));

  सचे( nCopy>0 ){
    memcpy(zBuf, &pCrash->zData[iOfst], nCopy);
  }

  /* Check the file-size to see if this is a short-read */
  सचे( nCopy<iAmt ){
    पटिनिवत्त SQLITE_IOERR_SHORT_READ;
  }

  पटिनिवत्त SQLITE_OK;
}

/*
** Write data to a crash-file.
*/
स्थिर पूर्णाङ्क cfWrite(
  sqlite3_file *pFile,
  ध्रुव शून्य *zBuf,
  पूर्णाङ्क iAmt,
  sqlite_int64 iOfst
){
  CrashFile *pCrash = (CrashFile *)pFile;
  सचे( iAmt+iOfst>pCrash->iSize ){
    pCrash->iSize = (पूर्णाङ्क)(iAmt+iOfst);
  }
  याव( pCrash->iSize>pCrash->nData ){
    u8 *zNew;
    पूर्णाङ्क nNew = (pCrash->nData*2) + 4096;
    zNew = crash_realloc(pCrash->zData, nNew);
    सचे( !zNew ){
      पटिनिवत्त SQLITE_NOMEM;
    }
    memset(&zNew[pCrash->nData], 0, nNew-pCrash->nData);
    pCrash->nData = nNew;
    pCrash->zData = zNew;
  }
  memcpy(&pCrash->zData[iOfst], zBuf, iAmt);
  पटिनिवत्त writeListAppend(pFile, iOfst, zBuf, iAmt);
}

/*
** Truncate a crash-file.
*/
स्थिर पूर्णाङ्क cfTruncate(sqlite3_file *pFile, sqlite_int64 size){
  CrashFile *pCrash = (CrashFile *)pFile;
  प्रतिज्ञा(size>=0);
  सचे( pCrash->iSize>size ){
    pCrash->iSize = (पूर्णाङ्क)size;
  }
  पटिनिवत्त writeListAppend(pFile, size, 0, 0);
}

/*
** Sync a crash-file.
*/
स्थिर पूर्णाङ्क cfSync(sqlite3_file *pFile, पूर्णाङ्क flags){
  CrashFile *pCrash = (CrashFile *)pFile;
  पूर्णाङ्क isCrash = 0;

  ध्रुव वर्ण *zName = pCrash->zName;
  ध्रुव वर्ण *zCrashFile = g.zCrashFile;
  पूर्णाङ्क nName = (पूर्णाङ्क)strlen(zName);
  पूर्णाङ्क nCrashFile = (पूर्णाङ्क)strlen(zCrashFile);

  सचे( nCrashFile>0 && zCrashFile[nCrashFile-1]=='*' ){
    nCrashFile--;
    सचे( nName>nCrashFile ) nName = nCrashFile;
  }

#ifdef TRACE_CRASHTEST
  printf("cfSync(): nName = %d, nCrashFile = %d, zName = %s, zCrashFile = %s\n",
         nName, nCrashFile, zName, zCrashFile);
#endif

  सचे( nName==nCrashFile && 0==memcmp(zName, zCrashFile, nName) ){
#ifdef TRACE_CRASHTEST
    printf("cfSync(): name matched, g.iCrash = %d\n", g.iCrash);
#endif
    सचे( (--g.iCrash)==0 ) isCrash = 1;
  }

  पटिनिवत्त writeListSync(pCrash, isCrash);
}

/*
** Return the current file-size of the crash-file.
*/
स्थिर पूर्णाङ्क cfFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  CrashFile *pCrash = (CrashFile *)pFile;
  *pSize = (i64)pCrash->iSize;
  पटिनिवत्त SQLITE_OK;
}

/*
** Calls related to file-locks are passed on to the real file handle.
*/
स्थिर पूर्णाङ्क cfLock(sqlite3_file *pFile, पूर्णाङ्क eLock){
  पटिनिवत्त sqlite3OsLock(((CrashFile *)pFile)->pRealFile, eLock);
}
स्थिर पूर्णाङ्क cfUnlock(sqlite3_file *pFile, पूर्णाङ्क eLock){
  पटिनिवत्त sqlite3OsUnlock(((CrashFile *)pFile)->pRealFile, eLock);
}
स्थिर पूर्णाङ्क cfCheckReservedLock(sqlite3_file *pFile, पूर्णाङ्क *pResOut){
  पटिनिवत्त sqlite3OsCheckReservedLock(((CrashFile *)pFile)->pRealFile, pResOut);
}
स्थिर पूर्णाङ्क cfFileControl(sqlite3_file *pFile, पूर्णाङ्क op, शून्य *pArg){
  सचे( op==SQLITE_FCNTL_SIZE_HINT ){
    CrashFile *pCrash = (CrashFile *)pFile;
    i64 nByte = *(i64 *)pArg;
    सचे( nByte>pCrash->iSize ){
      सचे( SQLITE_OK==writeListAppend(pFile, nByte, 0, 0) ){
        pCrash->iSize = (पूर्णाङ्क)nByte;
      }
    }
    पटिनिवत्त SQLITE_OK;
  }
  पटिनिवत्त sqlite3OsFileControl(((CrashFile *)pFile)->pRealFile, op, pArg);
}

/*
** The xSectorSize() and xDeviceCharacteristics() functions return
** the global values configured by the [sqlite_crashparams] tcl
*  interface.
*/
स्थिर पूर्णाङ्क cfSectorSize(sqlite3_file *pFile){
  पटिनिवत्त g.iSectorSize;
}
स्थिर पूर्णाङ्क cfDeviceCharacteristics(sqlite3_file *pFile){
  पटिनिवत्त g.iDeviceCharacteristics;
}

/*
** Pass-throughs for WAL support.
*/
स्थिर पूर्णाङ्क cfShmLock(sqlite3_file *pFile, पूर्णाङ्क ofst, पूर्णाङ्क n, पूर्णाङ्क flags){
  sqlite3_file *pReal = ((CrashFile*)pFile)->pRealFile;
  पटिनिवत्त pReal->pMethods->xShmLock(pReal, ofst, n, flags);
}
स्थिर शून्य cfShmBarrier(sqlite3_file *pFile){
  sqlite3_file *pReal = ((CrashFile*)pFile)->pRealFile;
  pReal->pMethods->xShmBarrier(pReal);
}
स्थिर पूर्णाङ्क cfShmUnmap(sqlite3_file *pFile, पूर्णाङ्क delFlag){
  sqlite3_file *pReal = ((CrashFile*)pFile)->pRealFile;
  पटिनिवत्त pReal->pMethods->xShmUnmap(pReal, delFlag);
}
स्थिर पूर्णाङ्क cfShmMap(
  sqlite3_file *pFile,            /* Handle open on database file */
  पूर्णाङ्क iRegion,                    /* Region to retrieve */
  पूर्णाङ्क sz,                         /* Size of regions */
  पूर्णाङ्क w,                          /* True to extend file if necessary */
  शून्य चञ्चल **pp              /* OUT: Mapped memory */
){
  sqlite3_file *pReal = ((CrashFile*)pFile)->pRealFile;
  पटिनिवत्त pReal->pMethods->xShmMap(pReal, iRegion, sz, w, pp);
}

स्थिर ध्रुव sqlite3_io_methods CrashFileVtab = {
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
संरचना crashAppData {
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
स्थिर पूर्णाङ्क cfOpen(
  sqlite3_vfs *pCfVfs,
  ध्रुव वर्ण *zName,
  sqlite3_file *pFile,
  पूर्णाङ्क flags,
  पूर्णाङ्क *pOutFlags
){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  पूर्णाङ्क rc;
  CrashFile *pWrapper = (CrashFile *)pFile;
  sqlite3_file *pReal = (sqlite3_file*)&pWrapper[1];

  memset(pWrapper, 0, परिमाण(CrashFile));
  rc = sqlite3OsOpen(pVfs, zName, pReal, flags, pOutFlags);

  सचे( rc==SQLITE_OK ){
    i64 iSize;
    pWrapper->pMethod = &CrashFileVtab;
    pWrapper->zName = (वर्ण *)zName;
    pWrapper->pRealFile = pReal;
    rc = sqlite3OsFileSize(pReal, &iSize);
    pWrapper->iSize = (पूर्णाङ्क)iSize;
    pWrapper->flags = flags;
  }
  सचे( rc==SQLITE_OK ){
    pWrapper->nData = (पूर्णाङ्क)(4096 + pWrapper->iSize);
    pWrapper->zData = crash_malloc(pWrapper->nData);
    सचे( pWrapper->zData ){
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
      अत्थाय(iOff=0; iOff<pWrapper->iSize; iOff += 512){
        पूर्णाङ्क nRead = (पूर्णाङ्क)(pWrapper->iSize - iOff);
        सचे( nRead>512 ) nRead = 512;
        rc = sqlite3OsRead(pReal, &pWrapper->zData[iOff], nRead, iOff);
      }
    }अञ्ञथा{
      rc = SQLITE_NOMEM;
    }
  }
  सचे( rc!=SQLITE_OK && pWrapper->pMethod ){
    sqlite3OsClose(pFile);
  }
  पटिनिवत्त rc;
}

स्थिर पूर्णाङ्क cfDelete(sqlite3_vfs *pCfVfs, ध्रुव वर्ण *zPath, पूर्णाङ्क dirSync){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  पटिनिवत्त pVfs->xDelete(pVfs, zPath, dirSync);
}
स्थिर पूर्णाङ्क cfAccess(
  sqlite3_vfs *pCfVfs,
  ध्रुव वर्ण *zPath,
  पूर्णाङ्क flags,
  पूर्णाङ्क *pResOut
){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  पटिनिवत्त pVfs->xAccess(pVfs, zPath, flags, pResOut);
}
स्थिर पूर्णाङ्क cfFullPathname(
  sqlite3_vfs *pCfVfs,
  ध्रुव वर्ण *zPath,
  पूर्णाङ्क nPathOut,
  वर्ण *zPathOut
){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  पटिनिवत्त pVfs->xFullPathname(pVfs, zPath, nPathOut, zPathOut);
}
स्थिर शून्य *cfDlOpen(sqlite3_vfs *pCfVfs, ध्रुव वर्ण *zPath){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  पटिनिवत्त pVfs->xDlOpen(pVfs, zPath);
}
स्थिर शून्य cfDlError(sqlite3_vfs *pCfVfs, पूर्णाङ्क nByte, वर्ण *z_Err_सन्देशः){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  pVfs->xDlError(pVfs, nByte, z_Err_सन्देशः);
}
स्थिर शून्य (*cfDlSym(sqlite3_vfs *pCfVfs, शून्य *pH, ध्रुव वर्ण *zSym))(शून्य){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  पटिनिवत्त pVfs->xDlSym(pVfs, pH, zSym);
}
स्थिर शून्य cfDlClose(sqlite3_vfs *pCfVfs, शून्य *pHandle){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  pVfs->xDlClose(pVfs, pHandle);
}
स्थिर पूर्णाङ्क cfRandomness(sqlite3_vfs *pCfVfs, पूर्णाङ्क nByte, वर्ण *zBufOut){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  पटिनिवत्त pVfs->xRandomness(pVfs, nByte, zBufOut);
}
स्थिर पूर्णाङ्क cfSleep(sqlite3_vfs *pCfVfs, पूर्णाङ्क nMicro){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  पटिनिवत्त pVfs->xSleep(pVfs, nMicro);
}
स्थिर पूर्णाङ्क cfCurrentTime(sqlite3_vfs *pCfVfs, द्विगुण *pTimeOut){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  पटिनिवत्त pVfs->xCurrentTime(pVfs, pTimeOut);
}
स्थिर पूर्णाङ्क cfGetLastError(sqlite3_vfs *pCfVfs, पूर्णाङ्क n, वर्ण *z){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  पटिनिवत्त pVfs->xGetLastError(pVfs, n, z);
}

स्थिर पूर्णाङ्क processDevSymArgs(
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[],
  पूर्णाङ्क *piDeviceChar,
  पूर्णाङ्क *piSectorSize
){
  संरचना DeviceFlag {
    वर्ण *zName;
    पूर्णाङ्क i_मानम्;
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

  पूर्णाङ्क i;
  पूर्णाङ्क iDc = 0;
  पूर्णाङ्क iSectorSize = 0;
  पूर्णाङ्क setSectorsize = 0;
  पूर्णाङ्क setDeviceChar = 0;

  अत्थाय(i=0; i<objc; i+=2){
    Tcl_Size nOpt;
    वर्ण *zOpt = Tcl_GetStringFromObj(objv[i], &nOpt);

    सचे( (nOpt>11 || nOpt<2 || strncmp("-sectorsize", zOpt, nOpt))
     && (nOpt>16 || nOpt<2 || strncmp("-characteristics", zOpt, nOpt))
    ){
      Tcl_AppendResult(interp,
        "Bad option: \"", zOpt,
        "\" - must be \"-characteristics\" or \"-sectorsize\"", रिक्त
      );
      पटिनिवत्त TCL_ERROR;
    }
    सचे( i==objc-1 ){
      Tcl_AppendResult(interp, "Option requires an argument: \"", zOpt, "\"", रिक्त);
      पटिनिवत्त TCL_ERROR;
    }

    सचे( zOpt[1]=='s' ){
      सचे( Tcl_GetIntFromObj(interp, objv[i+1], &iSectorSize) ){
        पटिनिवत्त TCL_ERROR;
      }
      setSectorsize = 1;
    }अञ्ञथा{
      पूर्णाङ्क j;
      Tcl_Obj **apObj;
      Tcl_Size nObj;
      सचे( Tcl_ListObjGetElements(interp, objv[i+1], &nObj, &apObj) ){
        पटिनिवत्त TCL_ERROR;
      }
      अत्थाय(j=0; j<(पूर्णाङ्क)nObj; j++){
        पूर्णाङ्क rc;
        पूर्णाङ्क iChoice;
        Tcl_Obj *pFlag = Tcl_DuplicateObj(apObj[j]);
        Tcl_Incr_Ref_गणनम्(pFlag);
        Tcl_UtfToLower(Tcl_GetString(pFlag));

        rc = Tcl_GetIndexFromObjStruct(
            interp, pFlag, aFlag, परिमाण(aFlag[0]), "no such flag", 0, &iChoice
        );
        Tcl_Decr_Ref_गणनम्(pFlag);
        सचे( rc ){
          पटिनिवत्त TCL_ERROR;
        }

        iDc |= aFlag[iChoice].i_मानम्;
      }
      setDeviceChar = 1;
    }
  }

  सचे( setDeviceChar ){
    *piDeviceChar = iDc;
  }
  सचे( setSectorsize ){
    *piSectorSize = iSectorSize;
  }

  पटिनिवत्त TCL_OK;
}

/*
** tclcmd:   sqlite3_crash_now
**
** Simulate a crash immediately. This function does not return
** (writeListSync() calls exit(-1)).
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI crashNowCmd(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  सचे( objc!=1 ){
    Tcl_Wrong_अङ्कः_Args(interp, 1, objv, "");
    पटिनिवत्त TCL_ERROR;
  }
  writeListSync(0, 1);
  प्रतिज्ञा( 0 );
  पटिनिवत्त TCL_OK;
}

/*
** tclcmd:   sqlite_crash_enable ENABLE ?DEFAULT?
**
** Parameter ENABLE must be a boolean value. If true, then the "crash"
** vfs is added to the system. If false, it is removed.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI crashEnableCmd(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क isEnable;
  पूर्णाङ्क isDefault = 0;
  स्थिर sqlite3_vfs crashVfs = {
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

  सचे( objc!=2 && objc!=3 ){
    Tcl_Wrong_अङ्कः_Args(interp, 1, objv, "ENABLE ?DEFAULT?");
    पटिनिवत्त TCL_ERROR;
  }

  सचे( Tcl_GetBooleanFromObj(interp, objv[1], &isEnable) ){
    पटिनिवत्त TCL_ERROR;
  }
  सचे( objc==3 && Tcl_GetBooleanFromObj(interp, objv[2], &isDefault) ){
    पटिनिवत्त TCL_ERROR;
  }

  सचे( (isEnable && crashVfs.pAppData) || (!isEnable && !crashVfs.pAppData) ){
    पटिनिवत्त TCL_OK;
  }

  सचे( crashVfs.pAppData==0 ){
    sqlite3_vfs *pOriginalVfs = sqlite3_vfs_find(0);
    crashVfs.mxPathname = pOriginalVfs->mxPathname;
    crashVfs.pAppData = (शून्य *)pOriginalVfs;
    crashVfs.szOsFile = परिमाण(CrashFile) + pOriginalVfs->szOsFile;
    sqlite3_vfs_register(&crashVfs, isDefault);
  }अञ्ञथा{
    crashVfs.pAppData = 0;
    sqlite3_vfs_unregister(&crashVfs);
  }

  पटिनिवत्त TCL_OK;
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
स्थिर पूर्णाङ्क SQLITE_TCLAPI crashParamsObjCmd(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क iDelay;
  ध्रुव वर्ण *zCrashFile;
  Tcl_Size nCrashFile;
  पूर्णाङ्क iDc, iSectorSize;

  iDc = -1;
  iSectorSize = -1;

  सचे( objc<3 ){
    Tcl_Wrong_अङ्कः_Args(interp, 1, objv, "?OPTIONS? DELAY CRASHFILE");
    गच्छ error;
  }

  zCrashFile = Tcl_GetStringFromObj(objv[objc-1], &nCrashFile);
  सचे( nCrashFile>=परिमाण(g.zCrashFile) ){
    Tcl_AppendResult(interp, "Filename is too long: \"", zCrashFile, "\"", रिक्त);
    गच्छ error;
  }
  सचे( Tcl_GetIntFromObj(interp, objv[objc-2], &iDelay) ){
    गच्छ error;
  }

  सचे( processDevSymArgs(interp, objc-3, &objv[1], &iDc, &iSectorSize) ){
    पटिनिवत्त TCL_ERROR;
  }

  सचे( iDc>=0 ){
    g.iDeviceCharacteristics = iDc;
  }
  सचे( iSectorSize>=0 ){
    g.iSectorSize = iSectorSize;
  }

  g.iCrash = iDelay;
  memcpy(g.zCrashFile, zCrashFile, nCrashFile+1);
  sqlite3CrashTestEnable = 1;
  पटिनिवत्त TCL_OK;

error:
  पटिनिवत्त TCL_ERROR;
}

स्थिर पूर्णाङ्क SQLITE_TCLAPI devSymObjCmd(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  शून्य devsym_register(पूर्णाङ्क iDeviceChar, पूर्णाङ्क iSectorSize);

  पूर्णाङ्क iDc = -1;
  पूर्णाङ्क iSectorSize = -1;

  सचे( processDevSymArgs(interp, objc-1, &objv[1], &iDc, &iSectorSize) ){
    पटिनिवत्त TCL_ERROR;
  }
  devsym_register(iDc, iSectorSize);

  पटिनिवत्त TCL_OK;
}

/*
** tclcmd: sqlite3_crash_on_write N
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI writeCrashObjCmd(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  शून्य devsym_crash_on_write(पूर्णाङ्क);
  पूर्णाङ्क nWrite = 0;

  सचे( objc!=2 ){
    Tcl_Wrong_अङ्कः_Args(interp, 1, objv, "NWRITE");
    पटिनिवत्त TCL_ERROR;
  }
  सचे( Tcl_GetIntFromObj(interp, objv[1], &nWrite) ){
    पटिनिवत्त TCL_ERROR;
  }

  devsym_crash_on_write(nWrite);
  पटिनिवत्त TCL_OK;
}

/*
** tclcmd: unregister_devsim
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI dsUnregisterObjCmd(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  शून्य devsym_unregister(शून्य);

  सचे( objc!=1 ){
    Tcl_Wrong_अङ्कः_Args(interp, 1, objv, "");
    पटिनिवत्त TCL_ERROR;
  }

  devsym_unregister();
  पटिनिवत्त TCL_OK;
}

/*
** tclcmd: register_jt_vfs ?-default? PARENT-VFS
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI jtObjCmd(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क jt_register(वर्ण *, पूर्णाङ्क);
  वर्ण *zParent = 0;

  सचे( objc!=2 && objc!=3 ){
    Tcl_Wrong_अङ्कः_Args(interp, 1, objv, "?-default? PARENT-VFS");
    पटिनिवत्त TCL_ERROR;
  }
  zParent = Tcl_GetString(objv[1]);
  सचे( objc==3 ){
    सचे( strcmp(zParent, "-default") ){
      Tcl_AppendResult(interp,
          "bad option \"", zParent, "\": must be -default", रिक्त
      );
      पटिनिवत्त TCL_ERROR;
    }
    zParent = Tcl_GetString(objv[2]);
  }

  सचे( !(*zParent) ){
    zParent = 0;
  }
  सचे( jt_register(zParent, objc==3) ){
    Tcl_AppendResult(interp, "Error in jt_register", रिक्त);
    पटिनिवत्त TCL_ERROR;
  }

  पटिनिवत्त TCL_OK;
}

/*
** tclcmd: unregister_jt_vfs
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI jtUnregisterObjCmd(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  शून्य jt_unregister(शून्य);

  सचे( objc!=1 ){
    Tcl_Wrong_अङ्कः_Args(interp, 1, objv, "");
    पटिनिवत्त TCL_ERROR;
  }

  jt_unregister();
  पटिनिवत्त TCL_OK;
}

#endif /* SQLITE_OMIT_DISKIO */

/*
** This procedure registers the TCL procedures defined in this file.
*/
पूर्णाङ्क Sqlitetest6_Init(Tcl_Interp *interp){
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
  पटिनिवत्त TCL_OK;
}

#endif /* SQLITE_TEST */
