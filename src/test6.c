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
#जर SQLITE_TEST          /* This file is used for testing only */
#अन्तर्भाव "sqliteInt.h"
#अन्तर्भाव "tclsqlite.h"

#ifndef SQLITE_OMIT_DISKIO  /* This file is a no-op if disk I/O is disabled */

/* #define TRACE_CRASHTEST */

प्रकारव्याख्या संरचना CrashFile CrashFile;
प्रकारव्याख्या संरचना CrashGlobal CrashGlobal;
प्रकारव्याख्या संरचना WriteBuffer WriteBuffer;

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
  पूर्णांक nBuf;                    /* Number of bytes written */
  u8 *zBuf;                    /* Pointer to copy of written data */
  CrashFile *pFile;            /* File this write() applies to */

  WriteBuffer *pNext;          /* Next in CrashGlobal.pWriteList */
};

संरचना CrashFile {
  स्थिरांक sqlite3_io_methods *pMethod;   /* Must be first */
  sqlite3_file *pRealFile;             /* Underlying "real" file handle */
  अक्षर *zName;
  पूर्णांक flags;                           /* Flags the file was opened with */

  /* Cache of the entire file. This is used to speed up OsRead() and
  ** OsFileSize() calls. Although both could be done by traversing the
  ** write-list, in practice this is impractically slow.
  */
  u8 *zData;                           /* Buffer containing file contents */
  पूर्णांक nData;                           /* Size of buffer allocated at zData */
  i64 iSize;                           /* Size of file in bytes */
};

संरचना CrashGlobal {
  WriteBuffer *pWriteList;     /* Head of write-list */
  WriteBuffer *pWriteListEnd;  /* End of write-list */

  पूर्णांक iSectorSize;             /* Value of simulated sector size */
  पूर्णांक iDeviceCharacteristics;  /* Value of simulated device characteristics */

  पूर्णांक iCrash;                  /* Crash on the iCrash'th call to xSync() */
  अक्षर zCrashFile[500];        /* Crash during an xSync() on this file */
};

स्थिर CrashGlobal g = {0, 0, SQLITE_DEFAULT_SECTOR_SIZE, 0, 0};

/*
** Set this global variable to 1 to enable crash testing.
*/
स्थिर पूर्णांक sqlite3CrashTestEnable = 0;

स्थिर शून्य *crash_malloc(पूर्णांक nByte){
  परत (शून्य *)Tcl_AttemptAlloc((माप_प्रकार)nByte);
}
स्थिर शून्य crash_free(शून्य *p){
  Tcl_Free(p);
}
स्थिर शून्य *crash_realloc(शून्य *p, पूर्णांक n){
  परत (शून्य *)Tcl_AttemptRealloc(p, (माप_प्रकार)n);
}

/*
** Wrapper around the sqlite3OsWrite() function that avoids writing to the
** 512 byte block beginning at offset PENDING_BYTE.
*/
स्थिर पूर्णांक writeDbFile(CrashFile *p, u8 *z, i64 iAmt, i64 iOff){
  पूर्णांक rc = SQLITE_OK;
  पूर्णांक iSkip = 0;
  जर( (iAmt-iSkip)>0 ){
    rc = sqlite3OsWrite(p->pRealFile, &z[iSkip], (पूर्णांक)(iAmt-iSkip), iOff+iSkip);
  }
  परत rc;
}

/*
** Flush the write-list as if xSync() had been called on file handle
** pFile. If isCrash is true, simulate a crash.
*/
स्थिर पूर्णांक writeListSync(CrashFile *pFile, पूर्णांक isCrash){
  पूर्णांक rc = SQLITE_OK;
  पूर्णांक iDc = g.iDeviceCharacteristics;

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
  जर( !isCrash ){
    साठी(pWrite=g.pWriteList; pWrite; pWrite=pWrite->pNext){
      जर( pWrite->pFile==pFile ){
        pFinal = pWrite;
      }
    }
  }अन्यथा जर( iDc&(SQLITE_IOCAP_SEQUENTIAL|SQLITE_IOCAP_SAFE_APPEND) ){
    पूर्णांक nWrite = 0;
    पूर्णांक iFinal;
    साठी(pWrite=g.pWriteList; pWrite; pWrite=pWrite->pNext) nWrite++;
    sqlite3_randomness(आकार(पूर्णांक), &iFinal);
    iFinal = ((iFinal<0)?-1*iFinal:iFinal)%nWrite;
    साठी(pWrite=g.pWriteList; iFinal>0; pWrite=pWrite->pNext) iFinal--;
    pFinal = pWrite;
  }

#ifdef TRACE_CRASHTEST
  जर( pFile ){
    म_लिखो("Sync %s (is %s crash)\n", pFile->zName, (isCrash?"a":"not a"));
  }
#endif

  ppPtr = &g.pWriteList;
  साठी(pWrite=*ppPtr; rc==SQLITE_OK && pWrite; pWrite=*ppPtr){
    sqlite3_file *pRealFile = pWrite->pFile->pRealFile;

    /* (eAction==1)      -> write block out normally,
    ** (eAction==2)      -> do nothing,
    ** (eAction==3)      -> trash sectors.
    */
    पूर्णांक eAction = 0;
    जर( !isCrash ){
      eAction = 2;
      जर( (pWrite->pFile==pFile || iDc&SQLITE_IOCAP_SEQUENTIAL) ){
        eAction = 1;
      }
    }अन्यथा{
      अक्षर random;
      sqlite3_randomness(1, &random);

      /* Do not select option 3 (sector trashing) if the IOCAP_ATOMIC flag
      ** is set or this is an OsTruncate(), not an Oswrite().
      */
      जर( (iDc&SQLITE_IOCAP_ATOMIC) || (pWrite->zBuf==0) ){
        random &= 0x01;
      }

      /* If IOCAP_SEQUENTIAL is set and this is not the final entry
      ** in the truncated write-list, always select option 1 (write
      ** out correctly).
      */
      जर( (iDc&SQLITE_IOCAP_SEQUENTIAL && pWrite!=pFinal) ){
        random = 0;
      }

      /* If IOCAP_SAFE_APPEND is set and this OsWrite() operation is
      ** an append (first byte of the written region is 1 byte past the
      ** current EOF), always select option 1 (write out correctly).
      */
      जर( iDc&SQLITE_IOCAP_SAFE_APPEND && pWrite->zBuf ){
        i64 iSize;
        sqlite3OsFileSize(pRealFile, &iSize);
        जर( iSize==pWrite->iOffset ){
          random = 0;
        }
      }

      जर( (random&0x06)==0x06 ){
        eAction = 3;
      }अन्यथा{
        eAction = ((random&0x01)?2:1);
      }
    }

    निवड( eAction ){
      स्थिती 1: {               /* Write out correctly */
        जर( pWrite->zBuf ){
          rc = writeDbFile(
              pWrite->pFile, pWrite->zBuf, pWrite->nBuf, pWrite->iOffset
          );
        }अन्यथा{
          rc = sqlite3OsTruncate(pRealFile, pWrite->iOffset);
        }
        *ppPtr = pWrite->pNext;
#ifdef TRACE_CRASHTEST
        जर( isCrash ){
          म_लिखो("Writing %d bytes @ %d (%s)\n",
            pWrite->nBuf, (पूर्णांक)pWrite->iOffset, pWrite->pFile->zName
          );
        }
#endif
        crash_free(pWrite);
        थांबा;
      }
      स्थिती 2: {               /* Do nothing */
        ppPtr = &pWrite->pNext;
#ifdef TRACE_CRASHTEST
        जर( isCrash ){
          म_लिखो("Omiting %d bytes @ %d (%s)\n",
            pWrite->nBuf, (पूर्णांक)pWrite->iOffset, pWrite->pFile->zName
          );
        }
#endif
        थांबा;
      }
      स्थिती 3: {               /* Trash sectors */
        u8 *zGarbage;
        पूर्णांक iFirst = (पूर्णांक)(pWrite->iOffset/g.iSectorSize);
        पूर्णांक iLast = (पूर्णांक)((pWrite->iOffset+pWrite->nBuf-1)/g.iSectorSize);

        प्रतिज्ञा(pWrite->zBuf);

#ifdef TRACE_CRASHTEST
        म_लिखो("Trashing %d sectors (%d bytes) @ %lld (sector %d) (%s)\n",
            1+iLast-iFirst, (1+iLast-iFirst)*g.iSectorSize,
            pWrite->iOffset, iFirst, pWrite->pFile->zName
        );
#endif

        zGarbage = crash_malloc(g.iSectorSize);
        जर( zGarbage ){
          sqlite3_int64 i;
          साठी(i=iFirst; rc==SQLITE_OK && i<=iLast; i++){
            sqlite3_randomness(g.iSectorSize, zGarbage);
            rc = writeDbFile(
              pWrite->pFile, zGarbage, g.iSectorSize, i*g.iSectorSize
            );
          }
          crash_free(zGarbage);
        }अन्यथा{
          rc = SQLITE_NOMEM;
        }

        ppPtr = &pWrite->pNext;
        थांबा;
      }

      पूर्वनिर्धारित:
        प्रतिज्ञा(!"Cannot happen");
    }

    जर( pWrite==pFinal ) थांबा;
  }

  जर( rc==SQLITE_OK && isCrash ){
    निकास(-1);
  }

  साठी(pWrite=g.pWriteList; pWrite && pWrite->pNext; pWrite=pWrite->pNext);
  g.pWriteListEnd = pWrite;

  परत rc;
}

/*
** Add an entry to the end of the write-list.
*/
स्थिर पूर्णांक writeListAppend(
  sqlite3_file *pFile,
  sqlite3_int64 iOffset,
  स्थिरांक u8 *zBuf,
  पूर्णांक nBuf
){
  WriteBuffer *pNew;

  प्रतिज्ञा((zBuf && nBuf) || (!nBuf && !zBuf));

  pNew = (WriteBuffer *)crash_malloc(आकार(WriteBuffer) + nBuf);
  जर( pNew==0 ){
    ख_लिखो(मानक_त्रुटि, "out of memory in the crash simulator\n");
  }
  स_रखो(pNew, 0, आकार(WriteBuffer)+nBuf);
  pNew->iOffset = iOffset;
  pNew->nBuf = nBuf;
  pNew->pFile = (CrashFile *)pFile;
  जर( zBuf ){
    pNew->zBuf = (u8 *)&pNew[1];
    स_नकल(pNew->zBuf, zBuf, nBuf);
  }

  जर( g.pWriteList ){
    प्रतिज्ञा(g.pWriteListEnd);
    g.pWriteListEnd->pNext = pNew;
  }अन्यथा{
    g.pWriteList = pNew;
  }
  g.pWriteListEnd = pNew;
 
  परत SQLITE_OK;
}

/*
** Close a crash-file.
*/
स्थिर पूर्णांक cfClose(sqlite3_file *pFile){
  CrashFile *pCrash = (CrashFile *)pFile;
  writeListSync(pCrash, 0);
  sqlite3OsClose(pCrash->pRealFile);
  परत SQLITE_OK;
}

/*
** Read data from a crash-file.
*/
स्थिर पूर्णांक cfRead(
  sqlite3_file *pFile,
  शून्य *zBuf,
  पूर्णांक iAmt,
  sqlite_int64 iOfst
){
  CrashFile *pCrash = (CrashFile *)pFile;
  पूर्णांक nCopy = (पूर्णांक)MIN((i64)iAmt, (pCrash->iSize - iOfst));

  जर( nCopy>0 ){
    स_नकल(zBuf, &pCrash->zData[iOfst], nCopy);
  }

  /* Check the file-size to see if this is a short-read */
  जर( nCopy<iAmt ){
    परत SQLITE_IOERR_SHORT_READ;
  }

  परत SQLITE_OK;
}

/*
** Write data to a crash-file.
*/
स्थिर पूर्णांक cfWrite(
  sqlite3_file *pFile,
  स्थिरांक शून्य *zBuf,
  पूर्णांक iAmt,
  sqlite_int64 iOfst
){
  CrashFile *pCrash = (CrashFile *)pFile;
  जर( iAmt+iOfst>pCrash->iSize ){
    pCrash->iSize = (पूर्णांक)(iAmt+iOfst);
  }
  पर्यंत( pCrash->iSize>pCrash->nData ){
    u8 *zNew;
    पूर्णांक nNew = (pCrash->nData*2) + 4096;
    zNew = crash_realloc(pCrash->zData, nNew);
    जर( !zNew ){
      परत SQLITE_NOMEM;
    }
    स_रखो(&zNew[pCrash->nData], 0, nNew-pCrash->nData);
    pCrash->nData = nNew;
    pCrash->zData = zNew;
  }
  स_नकल(&pCrash->zData[iOfst], zBuf, iAmt);
  परत writeListAppend(pFile, iOfst, zBuf, iAmt);
}

/*
** Truncate a crash-file.
*/
स्थिर पूर्णांक cfTruncate(sqlite3_file *pFile, sqlite_int64 size){
  CrashFile *pCrash = (CrashFile *)pFile;
  प्रतिज्ञा(size>=0);
  जर( pCrash->iSize>size ){
    pCrash->iSize = (पूर्णांक)size;
  }
  परत writeListAppend(pFile, size, 0, 0);
}

/*
** Sync a crash-file.
*/
स्थिर पूर्णांक cfSync(sqlite3_file *pFile, पूर्णांक flags){
  CrashFile *pCrash = (CrashFile *)pFile;
  पूर्णांक isCrash = 0;

  स्थिरांक अक्षर *zName = pCrash->zName;
  स्थिरांक अक्षर *zCrashFile = g.zCrashFile;
  पूर्णांक nName = (पूर्णांक)म_माप(zName);
  पूर्णांक nCrashFile = (पूर्णांक)म_माप(zCrashFile);

  जर( nCrashFile>0 && zCrashFile[nCrashFile-1]=='*' ){
    nCrashFile--;
    जर( nName>nCrashFile ) nName = nCrashFile;
  }

#ifdef TRACE_CRASHTEST
  म_लिखो("cfSync(): nName = %d, nCrashFile = %d, zName = %s, zCrashFile = %s\n",
         nName, nCrashFile, zName, zCrashFile);
#endif

  जर( nName==nCrashFile && 0==स_भेद(zName, zCrashFile, nName) ){
#ifdef TRACE_CRASHTEST
    म_लिखो("cfSync(): name matched, g.iCrash = %d\n", g.iCrash);
#endif
    जर( (--g.iCrash)==0 ) isCrash = 1;
  }

  परत writeListSync(pCrash, isCrash);
}

/*
** Return the current file-size of the crash-file.
*/
स्थिर पूर्णांक cfFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  CrashFile *pCrash = (CrashFile *)pFile;
  *pSize = (i64)pCrash->iSize;
  परत SQLITE_OK;
}

/*
** Calls related to file-locks are passed on to the real file handle.
*/
स्थिर पूर्णांक cfLock(sqlite3_file *pFile, पूर्णांक eLock){
  परत sqlite3OsLock(((CrashFile *)pFile)->pRealFile, eLock);
}
स्थिर पूर्णांक cfUnlock(sqlite3_file *pFile, पूर्णांक eLock){
  परत sqlite3OsUnlock(((CrashFile *)pFile)->pRealFile, eLock);
}
स्थिर पूर्णांक cfCheckReservedLock(sqlite3_file *pFile, पूर्णांक *pResOut){
  परत sqlite3OsCheckReservedLock(((CrashFile *)pFile)->pRealFile, pResOut);
}
स्थिर पूर्णांक cfFileControl(sqlite3_file *pFile, पूर्णांक op, शून्य *pArg){
  जर( op==SQLITE_FCNTL_SIZE_HINT ){
    CrashFile *pCrash = (CrashFile *)pFile;
    i64 nByte = *(i64 *)pArg;
    जर( nByte>pCrash->iSize ){
      जर( SQLITE_OK==writeListAppend(pFile, nByte, 0, 0) ){
        pCrash->iSize = (पूर्णांक)nByte;
      }
    }
    परत SQLITE_OK;
  }
  परत sqlite3OsFileControl(((CrashFile *)pFile)->pRealFile, op, pArg);
}

/*
** The xSectorSize() and xDeviceCharacteristics() functions return
** the global values configured by the [sqlite_crashparams] tcl
*  interface.
*/
स्थिर पूर्णांक cfSectorSize(sqlite3_file *pFile){
  परत g.iSectorSize;
}
स्थिर पूर्णांक cfDeviceCharacteristics(sqlite3_file *pFile){
  परत g.iDeviceCharacteristics;
}

/*
** Pass-throughs for WAL support.
*/
स्थिर पूर्णांक cfShmLock(sqlite3_file *pFile, पूर्णांक ofst, पूर्णांक n, पूर्णांक flags){
  sqlite3_file *pReal = ((CrashFile*)pFile)->pRealFile;
  परत pReal->pMethods->xShmLock(pReal, ofst, n, flags);
}
स्थिर शून्य cfShmBarrier(sqlite3_file *pFile){
  sqlite3_file *pReal = ((CrashFile*)pFile)->pRealFile;
  pReal->pMethods->xShmBarrier(pReal);
}
स्थिर पूर्णांक cfShmUnmap(sqlite3_file *pFile, पूर्णांक delFlag){
  sqlite3_file *pReal = ((CrashFile*)pFile)->pRealFile;
  परत pReal->pMethods->xShmUnmap(pReal, delFlag);
}
स्थिर पूर्णांक cfShmMap(
  sqlite3_file *pFile,            /* Handle open on database file */
  पूर्णांक iRegion,                    /* Region to retrieve */
  पूर्णांक sz,                         /* Size of regions */
  पूर्णांक w,                          /* True to extend file if necessary */
  शून्य अस्थिर **pp              /* OUT: Mapped memory */
){
  sqlite3_file *pReal = ((CrashFile*)pFile)->pRealFile;
  परत pReal->pMethods->xShmMap(pReal, iRegion, sz, w, pp);
}

स्थिर स्थिरांक sqlite3_io_methods CrashFileVtab = {
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
स्थिर पूर्णांक cfOpen(
  sqlite3_vfs *pCfVfs,
  स्थिरांक अक्षर *zName,
  sqlite3_file *pFile,
  पूर्णांक flags,
  पूर्णांक *pOutFlags
){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  पूर्णांक rc;
  CrashFile *pWrapper = (CrashFile *)pFile;
  sqlite3_file *pReal = (sqlite3_file*)&pWrapper[1];

  स_रखो(pWrapper, 0, आकार(CrashFile));
  rc = sqlite3OsOpen(pVfs, zName, pReal, flags, pOutFlags);

  जर( rc==SQLITE_OK ){
    i64 iSize;
    pWrapper->pMethod = &CrashFileVtab;
    pWrapper->zName = (अक्षर *)zName;
    pWrapper->pRealFile = pReal;
    rc = sqlite3OsFileSize(pReal, &iSize);
    pWrapper->iSize = (पूर्णांक)iSize;
    pWrapper->flags = flags;
  }
  जर( rc==SQLITE_OK ){
    pWrapper->nData = (पूर्णांक)(4096 + pWrapper->iSize);
    pWrapper->zData = crash_malloc(pWrapper->nData);
    जर( pWrapper->zData ){
      /* os_unix.c contains an assert() that fails if the caller attempts
      ** to read data from the 512-byte locking region of a file opened
      ** with the SQLITE_OPEN_MAIN_DB flag. This region of a database file
      ** never contains valid data anyhow. So avoid doing such a read here.
      **
      ** UPDATE: It also contains an assert() verifying that each call
      ** to the xRead() method reads less than 128KB of data.
      */
      i64 iOff;

      स_रखो(pWrapper->zData, 0, pWrapper->nData);
      साठी(iOff=0; iOff<pWrapper->iSize; iOff += 512){
        पूर्णांक nRead = (पूर्णांक)(pWrapper->iSize - iOff);
        जर( nRead>512 ) nRead = 512;
        rc = sqlite3OsRead(pReal, &pWrapper->zData[iOff], nRead, iOff);
      }
    }अन्यथा{
      rc = SQLITE_NOMEM;
    }
  }
  जर( rc!=SQLITE_OK && pWrapper->pMethod ){
    sqlite3OsClose(pFile);
  }
  परत rc;
}

स्थिर पूर्णांक cfDelete(sqlite3_vfs *pCfVfs, स्थिरांक अक्षर *zPath, पूर्णांक dirSync){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  परत pVfs->xDelete(pVfs, zPath, dirSync);
}
स्थिर पूर्णांक cfAccess(
  sqlite3_vfs *pCfVfs,
  स्थिरांक अक्षर *zPath,
  पूर्णांक flags,
  पूर्णांक *pResOut
){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  परत pVfs->xAccess(pVfs, zPath, flags, pResOut);
}
स्थिर पूर्णांक cfFullPathname(
  sqlite3_vfs *pCfVfs,
  स्थिरांक अक्षर *zPath,
  पूर्णांक nPathOut,
  अक्षर *zPathOut
){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  परत pVfs->xFullPathname(pVfs, zPath, nPathOut, zPathOut);
}
स्थिर शून्य *cfDlOpen(sqlite3_vfs *pCfVfs, स्थिरांक अक्षर *zPath){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  परत pVfs->xDlOpen(pVfs, zPath);
}
स्थिर शून्य cfDlError(sqlite3_vfs *pCfVfs, पूर्णांक nByte, अक्षर *z_Err_संदेश){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  pVfs->xDlError(pVfs, nByte, z_Err_संदेश);
}
स्थिर शून्य (*cfDlSym(sqlite3_vfs *pCfVfs, शून्य *pH, स्थिरांक अक्षर *zSym))(शून्य){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  परत pVfs->xDlSym(pVfs, pH, zSym);
}
स्थिर शून्य cfDlClose(sqlite3_vfs *pCfVfs, शून्य *pHandle){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  pVfs->xDlClose(pVfs, pHandle);
}
स्थिर पूर्णांक cfRandomness(sqlite3_vfs *pCfVfs, पूर्णांक nByte, अक्षर *zBufOut){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  परत pVfs->xRandomness(pVfs, nByte, zBufOut);
}
स्थिर पूर्णांक cfSleep(sqlite3_vfs *pCfVfs, पूर्णांक nMicro){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  परत pVfs->xSleep(pVfs, nMicro);
}
स्थिर पूर्णांक cfCurrentTime(sqlite3_vfs *pCfVfs, द्विगुण *pTimeOut){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  परत pVfs->xCurrentTime(pVfs, pTimeOut);
}
स्थिर पूर्णांक cfGetLastError(sqlite3_vfs *pCfVfs, पूर्णांक n, अक्षर *z){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->pAppData;
  परत pVfs->xGetLastError(pVfs, n, z);
}

स्थिर पूर्णांक processDevSymArgs(
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[],
  पूर्णांक *piDeviceChar,
  पूर्णांक *piSectorSize
){
  संरचना DeviceFlag {
    अक्षर *zName;
    पूर्णांक i_मूल्य;
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

  पूर्णांक i;
  पूर्णांक iDc = 0;
  पूर्णांक iSectorSize = 0;
  पूर्णांक setSectorsize = 0;
  पूर्णांक setDeviceChar = 0;

  साठी(i=0; i<objc; i+=2){
    Tcl_Size nOpt;
    अक्षर *zOpt = Tcl_GetStringFromObj(objv[i], &nOpt);

    जर( (nOpt>11 || nOpt<2 || म_भेदन("-sectorsize", zOpt, nOpt))
     && (nOpt>16 || nOpt<2 || म_भेदन("-characteristics", zOpt, nOpt))
    ){
      Tcl_AppendResult(interp,
        "Bad option: \"", zOpt,
        "\" - must be \"-characteristics\" or \"-sectorsize\"", रिक्त
      );
      परत TCL_ERROR;
    }
    जर( i==objc-1 ){
      Tcl_AppendResult(interp, "Option requires an argument: \"", zOpt, "\"", रिक्त);
      परत TCL_ERROR;
    }

    जर( zOpt[1]=='s' ){
      जर( Tcl_GetIntFromObj(interp, objv[i+1], &iSectorSize) ){
        परत TCL_ERROR;
      }
      setSectorsize = 1;
    }अन्यथा{
      पूर्णांक j;
      Tcl_Obj **apObj;
      Tcl_Size nObj;
      जर( Tcl_ListObjGetElements(interp, objv[i+1], &nObj, &apObj) ){
        परत TCL_ERROR;
      }
      साठी(j=0; j<(पूर्णांक)nObj; j++){
        पूर्णांक rc;
        पूर्णांक iChoice;
        Tcl_Obj *pFlag = Tcl_DuplicateObj(apObj[j]);
        Tcl_Incr_Ref_मोजणी(pFlag);
        Tcl_UtfToLower(Tcl_GetString(pFlag));

        rc = Tcl_GetIndexFromObjStruct(
            interp, pFlag, aFlag, आकार(aFlag[0]), "no such flag", 0, &iChoice
        );
        Tcl_Decr_Ref_मोजणी(pFlag);
        जर( rc ){
          परत TCL_ERROR;
        }

        iDc |= aFlag[iChoice].i_मूल्य;
      }
      setDeviceChar = 1;
    }
  }

  जर( setDeviceChar ){
    *piDeviceChar = iDc;
  }
  जर( setSectorsize ){
    *piSectorSize = iSectorSize;
  }

  परत TCL_OK;
}

/*
** tclcmd:   sqlite3_crash_now
**
** Simulate a crash immediately. This function does not return
** (writeListSync() calls exit(-1)).
*/
स्थिर पूर्णांक SQLITE_TCLAPI crashNowCmd(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  जर( objc!=1 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "");
    परत TCL_ERROR;
  }
  writeListSync(0, 1);
  प्रतिज्ञा( 0 );
  परत TCL_OK;
}

/*
** tclcmd:   sqlite_crash_enable ENABLE ?DEFAULT?
**
** Parameter ENABLE must be a boolean value. If true, then the "crash"
** vfs is added to the system. If false, it is removed.
*/
स्थिर पूर्णांक SQLITE_TCLAPI crashEnableCmd(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक isEnable;
  पूर्णांक isDefault = 0;
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

  जर( objc!=2 && objc!=3 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "ENABLE ?DEFAULT?");
    परत TCL_ERROR;
  }

  जर( Tcl_GetBooleanFromObj(interp, objv[1], &isEnable) ){
    परत TCL_ERROR;
  }
  जर( objc==3 && Tcl_GetBooleanFromObj(interp, objv[2], &isDefault) ){
    परत TCL_ERROR;
  }

  जर( (isEnable && crashVfs.pAppData) || (!isEnable && !crashVfs.pAppData) ){
    परत TCL_OK;
  }

  जर( crashVfs.pAppData==0 ){
    sqlite3_vfs *pOriginalVfs = sqlite3_vfs_find(0);
    crashVfs.mxPathname = pOriginalVfs->mxPathname;
    crashVfs.pAppData = (शून्य *)pOriginalVfs;
    crashVfs.szOsFile = आकार(CrashFile) + pOriginalVfs->szOsFile;
    sqlite3_vfs_register(&crashVfs, isDefault);
  }अन्यथा{
    crashVfs.pAppData = 0;
    sqlite3_vfs_unregister(&crashVfs);
  }

  परत TCL_OK;
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
स्थिर पूर्णांक SQLITE_TCLAPI crashParamsObjCmd(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक iDelay;
  स्थिरांक अक्षर *zCrashFile;
  Tcl_Size nCrashFile;
  पूर्णांक iDc, iSectorSize;

  iDc = -1;
  iSectorSize = -1;

  जर( objc<3 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "?OPTIONS? DELAY CRASHFILE");
    जा error;
  }

  zCrashFile = Tcl_GetStringFromObj(objv[objc-1], &nCrashFile);
  जर( nCrashFile>=आकार(g.zCrashFile) ){
    Tcl_AppendResult(interp, "Filename is too long: \"", zCrashFile, "\"", रिक्त);
    जा error;
  }
  जर( Tcl_GetIntFromObj(interp, objv[objc-2], &iDelay) ){
    जा error;
  }

  जर( processDevSymArgs(interp, objc-3, &objv[1], &iDc, &iSectorSize) ){
    परत TCL_ERROR;
  }

  जर( iDc>=0 ){
    g.iDeviceCharacteristics = iDc;
  }
  जर( iSectorSize>=0 ){
    g.iSectorSize = iSectorSize;
  }

  g.iCrash = iDelay;
  स_नकल(g.zCrashFile, zCrashFile, nCrashFile+1);
  sqlite3CrashTestEnable = 1;
  परत TCL_OK;

error:
  परत TCL_ERROR;
}

स्थिर पूर्णांक SQLITE_TCLAPI devSymObjCmd(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  शून्य devsym_register(पूर्णांक iDeviceChar, पूर्णांक iSectorSize);

  पूर्णांक iDc = -1;
  पूर्णांक iSectorSize = -1;

  जर( processDevSymArgs(interp, objc-1, &objv[1], &iDc, &iSectorSize) ){
    परत TCL_ERROR;
  }
  devsym_register(iDc, iSectorSize);

  परत TCL_OK;
}

/*
** tclcmd: sqlite3_crash_on_write N
*/
स्थिर पूर्णांक SQLITE_TCLAPI writeCrashObjCmd(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  शून्य devsym_crash_on_write(पूर्णांक);
  पूर्णांक nWrite = 0;

  जर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "NWRITE");
    परत TCL_ERROR;
  }
  जर( Tcl_GetIntFromObj(interp, objv[1], &nWrite) ){
    परत TCL_ERROR;
  }

  devsym_crash_on_write(nWrite);
  परत TCL_OK;
}

/*
** tclcmd: unregister_devsim
*/
स्थिर पूर्णांक SQLITE_TCLAPI dsUnregisterObjCmd(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  शून्य devsym_unregister(शून्य);

  जर( objc!=1 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "");
    परत TCL_ERROR;
  }

  devsym_unregister();
  परत TCL_OK;
}

/*
** tclcmd: register_jt_vfs ?-default? PARENT-VFS
*/
स्थिर पूर्णांक SQLITE_TCLAPI jtObjCmd(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक jt_register(अक्षर *, पूर्णांक);
  अक्षर *zParent = 0;

  जर( objc!=2 && objc!=3 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "?-default? PARENT-VFS");
    परत TCL_ERROR;
  }
  zParent = Tcl_GetString(objv[1]);
  जर( objc==3 ){
    जर( म_भेद(zParent, "-default") ){
      Tcl_AppendResult(interp,
          "bad option \"", zParent, "\": must be -default", रिक्त
      );
      परत TCL_ERROR;
    }
    zParent = Tcl_GetString(objv[2]);
  }

  जर( !(*zParent) ){
    zParent = 0;
  }
  जर( jt_register(zParent, objc==3) ){
    Tcl_AppendResult(interp, "Error in jt_register", रिक्त);
    परत TCL_ERROR;
  }

  परत TCL_OK;
}

/*
** tclcmd: unregister_jt_vfs
*/
स्थिर पूर्णांक SQLITE_TCLAPI jtUnregisterObjCmd(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  शून्य jt_unregister(शून्य);

  जर( objc!=1 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "");
    परत TCL_ERROR;
  }

  jt_unregister();
  परत TCL_OK;
}

#endif /* SQLITE_OMIT_DISKIO */

/*
** This procedure registers the TCL procedures defined in this file.
*/
पूर्णांक Sqlitetest6_Init(Tcl_Interp *interp){
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
  परत TCL_OK;
}

#endif /* SQLITE_TEST */
