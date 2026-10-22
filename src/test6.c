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

প্রকারলেখ কাঠামো CrashFile CrashFile;
প্রকারলেখ কাঠামো CrashGlobal CrashGlobal;
প্রকারলেখ কাঠামো Write_বাফার Write_বাফার;

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
কাঠামো Write_বাফার {
  i64 iOffset;                 /* Byte offset of the start of this write() */
  পূর্ণসংখ্যা nBuf;                    /* Number of bytes written */
  u8 *zBuf;                    /* Pointer to copy of written data */
  CrashFile *pFile;            /* File this write() applies to */

  Write_বাফার *p_পরবর্তী;          /* Next in CrashGlobal.pWriteList */
};

কাঠামো CrashFile {
  ধ্রুবক sqlite3_io_methods *pMethod;   /* Must be first */
  sqlite3_file *pRealFile;             /* Underlying "real" file handle */
  অক্ষর *zName;
  পূর্ণসংখ্যা flags;                           /* Flags the file was opened with */

  /* Cache of the entire file. This is used to speed up OsRead() and
  ** OsFileSize() calls. Although both could be done by traversing the
  ** write-list, in practice this is impractically slow.
  */
  u8 *z_তথ্য;                           /* Buffer containing file contents */
  পূর্ণসংখ্যা n_তথ্য;                           /* Size of buffer allocated at zData */
  i64 i_আকার;                           /* Size of file in bytes */
};

কাঠামো CrashGlobal {
  Write_বাফার *pWriteList;     /* Head of write-list */
  Write_বাফার *pWriteListEnd;  /* End of write-list */

  পূর্ণসংখ্যা i_Sector_আকার;             /* Value of simulated sector size */
  পূর্ণসংখ্যা iDeviceCharacteristics;  /* Value of simulated device characteristics */

  পূর্ণসংখ্যা iCrash;                  /* Crash on the iCrash'th call to xSync() */
  অক্ষর zCrashFile[500];        /* Crash during an xSync() on this file */
};

স্থির CrashGlobal g = {0, 0, SQLITE_DEFAULT_SECTOR_আকার, 0, 0};

/*
** Set this global variable to 1 to enable crash testing.
*/
স্থির পূর্ণসংখ্যা sqlite3CrashTestEnable = 0;

স্থির শূন্য *crash_malloc(পূর্ণসংখ্যা nByte){
  ফেরত (শূন্য *)Tcl_AttemptAlloc((size_t)nByte);
}
স্থির শূন্য crash_free(শূন্য *p){
  Tcl_Free(p);
}
স্থির শূন্য *crash_realloc(শূন্য *p, পূর্ণসংখ্যা n){
  ফেরত (শূন্য *)Tcl_AttemptRealloc(p, (size_t)n);
}

/*
** Wrapper around the sqlite3OsWrite() function that avoids writing to the
** 512 byte block beginning at offset PENDING_BYTE.
*/
স্থির পূর্ণসংখ্যা writeDbFile(CrashFile *p, u8 *z, i64 iAmt, i64 iOff){
  পূর্ণসংখ্যা rc = SQLITE_OK;
  পূর্ণসংখ্যা iSkip = 0;
  যদি( (iAmt-iSkip)>0 ){
    rc = sqlite3OsWrite(p->pRealFile, &z[iSkip], (পূর্ণসংখ্যা)(iAmt-iSkip), iOff+iSkip);
  }
  ফেরত rc;
}

/*
** Flush the write-list as if xSync() had been called on file handle
** pFile. If isCrash is true, simulate a crash.
*/
স্থির পূর্ণসংখ্যা writeListSync(CrashFile *pFile, পূর্ণসংখ্যা isCrash){
  পূর্ণসংখ্যা rc = SQLITE_OK;
  পূর্ণসংখ্যা iDc = g.iDeviceCharacteristics;

  Write_বাফার *pWrite;
  Write_বাফার **ppPtr;

  /* If this is not a crash simulation, set pFinal to point to the
  ** last element of the write-list that is associated with file handle
  ** pFile.
  **
  ** If this is a crash simulation, set pFinal to an arbitrarily selected
  ** element of the write-list.
  */
  Write_বাফার *pFinal = 0;
  যদি( !isCrash ){
    জন্য(pWrite=g.pWriteList; pWrite; pWrite=pWrite->p_পরবর্তী){
      যদি( pWrite->pFile==pFile ){
        pFinal = pWrite;
      }
    }
  }অন্যথা যদি( iDc&(SQLITE_IOCAP_SEQUENTIAL|SQLITE_IOCAP_SAFE_APPEND) ){
    পূর্ণসংখ্যা nWrite = 0;
    পূর্ণসংখ্যা iFinal;
    জন্য(pWrite=g.pWriteList; pWrite; pWrite=pWrite->p_পরবর্তী) nWrite++;
    sqlite3_randomness(আকার(পূর্ণসংখ্যা), &iFinal);
    iFinal = ((iFinal<0)?-1*iFinal:iFinal)%nWrite;
    জন্য(pWrite=g.pWriteList; iFinal>0; pWrite=pWrite->p_পরবর্তী) iFinal--;
    pFinal = pWrite;
  }

#ifdef TRACE_CRASHTEST
  যদি( pFile ){
    printf("Sync %s (is %s crash)\n", pFile->zName, (isCrash?"a":"not a"));
  }
#endif

  ppPtr = &g.pWriteList;
  জন্য(pWrite=*ppPtr; rc==SQLITE_OK && pWrite; pWrite=*ppPtr){
    sqlite3_file *pRealFile = pWrite->pFile->pRealFile;

    /* (eAction==1)      -> write block out normally,
    ** (eAction==2)      -> do nothing,
    ** (eAction==3)      -> trash sectors.
    */
    পূর্ণসংখ্যা eAction = 0;
    যদি( !isCrash ){
      eAction = 2;
      যদি( (pWrite->pFile==pFile || iDc&SQLITE_IOCAP_SEQUENTIAL) ){
        eAction = 1;
      }
    }অন্যথা{
      অক্ষর random;
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
        i64 i_আকার;
        sqlite3_Os_File_আকার(pRealFile, &i_আকার);
        যদি( i_আকার==pWrite->iOffset ){
          random = 0;
        }
      }

      যদি( (random&0x06)==0x06 ){
        eAction = 3;
      }অন্যথা{
        eAction = ((random&0x01)?2:1);
      }
    }

    নির্বাচন( eAction ){
      ক্ষেত্র 1: {               /* Write out correctly */
        যদি( pWrite->zBuf ){
          rc = writeDbFile(
              pWrite->pFile, pWrite->zBuf, pWrite->nBuf, pWrite->iOffset
          );
        }অন্যথা{
          rc = sqlite3OsTruncate(pRealFile, pWrite->iOffset);
        }
        *ppPtr = pWrite->p_পরবর্তী;
#ifdef TRACE_CRASHTEST
        যদি( isCrash ){
          printf("Writing %d bytes @ %d (%s)\n",
            pWrite->nBuf, (পূর্ণসংখ্যা)pWrite->iOffset, pWrite->pFile->zName
          );
        }
#endif
        crash_free(pWrite);
        বিরতি;
      }
      ক্ষেত্র 2: {               /* Do nothing */
        ppPtr = &pWrite->p_পরবর্তী;
#ifdef TRACE_CRASHTEST
        যদি( isCrash ){
          printf("Omiting %d bytes @ %d (%s)\n",
            pWrite->nBuf, (পূর্ণসংখ্যা)pWrite->iOffset, pWrite->pFile->zName
          );
        }
#endif
        বিরতি;
      }
      ক্ষেত্র 3: {               /* Trash sectors */
        u8 *zGarbage;
        পূর্ণসংখ্যা iFirst = (পূর্ণসংখ্যা)(pWrite->iOffset/g.i_Sector_আকার);
        পূর্ণসংখ্যা iLast = (পূর্ণসংখ্যা)((pWrite->iOffset+pWrite->nBuf-1)/g.i_Sector_আকার);

        প্রতিজ্ঞা(pWrite->zBuf);

#ifdef TRACE_CRASHTEST
        printf("Trashing %d sectors (%d bytes) @ %lld (sector %d) (%s)\n",
            1+iLast-iFirst, (1+iLast-iFirst)*g.i_Sector_আকার,
            pWrite->iOffset, iFirst, pWrite->pFile->zName
        );
#endif

        zGarbage = crash_malloc(g.i_Sector_আকার);
        যদি( zGarbage ){
          sqlite3_int64 i;
          জন্য(i=iFirst; rc==SQLITE_OK && i<=iLast; i++){
            sqlite3_randomness(g.i_Sector_আকার, zGarbage);
            rc = writeDbFile(
              pWrite->pFile, zGarbage, g.i_Sector_আকার, i*g.i_Sector_আকার
            );
          }
          crash_free(zGarbage);
        }অন্যথা{
          rc = SQLITE_NOMEM;
        }

        ppPtr = &pWrite->p_পরবর্তী;
        বিরতি;
      }

      পূর্বনির্ধারিত:
        প্রতিজ্ঞা(!"Cannot happen");
    }

    যদি( pWrite==pFinal ) বিরতি;
  }

  যদি( rc==SQLITE_OK && isCrash ){
    exit(-1);
  }

  জন্য(pWrite=g.pWriteList; pWrite && pWrite->p_পরবর্তী; pWrite=pWrite->p_পরবর্তী);
  g.pWriteListEnd = pWrite;

  ফেরত rc;
}

/*
** Add an entry to the end of the write-list.
*/
স্থির পূর্ণসংখ্যা writeListAppend(
  sqlite3_file *pFile,
  sqlite3_int64 iOffset,
  ধ্রুবক u8 *zBuf,
  পূর্ণসংখ্যা nBuf
){
  Write_বাফার *pNew;

  প্রতিজ্ঞা((zBuf && nBuf) || (!nBuf && !zBuf));

  pNew = (Write_বাফার *)crash_malloc(আকার(Write_বাফার) + nBuf);
  যদি( pNew==0 ){
    fprintf(stderr, "out of memory in the crash simulator\n");
  }
  memset(pNew, 0, আকার(Write_বাফার)+nBuf);
  pNew->iOffset = iOffset;
  pNew->nBuf = nBuf;
  pNew->pFile = (CrashFile *)pFile;
  যদি( zBuf ){
    pNew->zBuf = (u8 *)&pNew[1];
    memcpy(pNew->zBuf, zBuf, nBuf);
  }

  যদি( g.pWriteList ){
    প্রতিজ্ঞা(g.pWriteListEnd);
    g.pWriteListEnd->p_পরবর্তী = pNew;
  }অন্যথা{
    g.pWriteList = pNew;
  }
  g.pWriteListEnd = pNew;
 
  ফেরত SQLITE_OK;
}

/*
** Close a crash-file.
*/
স্থির পূর্ণসংখ্যা cfClose(sqlite3_file *pFile){
  CrashFile *pCrash = (CrashFile *)pFile;
  writeListSync(pCrash, 0);
  sqlite3OsClose(pCrash->pRealFile);
  ফেরত SQLITE_OK;
}

/*
** Read data from a crash-file.
*/
স্থির পূর্ণসংখ্যা cfRead(
  sqlite3_file *pFile,
  শূন্য *zBuf,
  পূর্ণসংখ্যা iAmt,
  sqlite_int64 iOfst
){
  CrashFile *pCrash = (CrashFile *)pFile;
  পূর্ণসংখ্যা nCopy = (পূর্ণসংখ্যা)MIN((i64)iAmt, (pCrash->i_আকার - iOfst));

  যদি( nCopy>0 ){
    memcpy(zBuf, &pCrash->z_তথ্য[iOfst], nCopy);
  }

  /* Check the file-size to see if this is a short-read */
  যদি( nCopy<iAmt ){
    ফেরত SQLITE_IOERR_SHORT_READ;
  }

  ফেরত SQLITE_OK;
}

/*
** Write data to a crash-file.
*/
স্থির পূর্ণসংখ্যা cfWrite(
  sqlite3_file *pFile,
  ধ্রুবক শূন্য *zBuf,
  পূর্ণসংখ্যা iAmt,
  sqlite_int64 iOfst
){
  CrashFile *pCrash = (CrashFile *)pFile;
  যদি( iAmt+iOfst>pCrash->i_আকার ){
    pCrash->i_আকার = (পূর্ণসংখ্যা)(iAmt+iOfst);
  }
  যখন( pCrash->i_আকার>pCrash->n_তথ্য ){
    u8 *zNew;
    পূর্ণসংখ্যা nNew = (pCrash->n_তথ্য*2) + 4096;
    zNew = crash_realloc(pCrash->z_তথ্য, nNew);
    যদি( !zNew ){
      ফেরত SQLITE_NOMEM;
    }
    memset(&zNew[pCrash->n_তথ্য], 0, nNew-pCrash->n_তথ্য);
    pCrash->n_তথ্য = nNew;
    pCrash->z_তথ্য = zNew;
  }
  memcpy(&pCrash->z_তথ্য[iOfst], zBuf, iAmt);
  ফেরত writeListAppend(pFile, iOfst, zBuf, iAmt);
}

/*
** Truncate a crash-file.
*/
স্থির পূর্ণসংখ্যা cfTruncate(sqlite3_file *pFile, sqlite_int64 আকার_2){
  CrashFile *pCrash = (CrashFile *)pFile;
  প্রতিজ্ঞা(আকার_2>=0);
  যদি( pCrash->i_আকার>আকার_2 ){
    pCrash->i_আকার = (পূর্ণসংখ্যা)আকার_2;
  }
  ফেরত writeListAppend(pFile, আকার_2, 0, 0);
}

/*
** Sync a crash-file.
*/
স্থির পূর্ণসংখ্যা cfSync(sqlite3_file *pFile, পূর্ণসংখ্যা flags){
  CrashFile *pCrash = (CrashFile *)pFile;
  পূর্ণসংখ্যা isCrash = 0;

  ধ্রুবক অক্ষর *zName = pCrash->zName;
  ধ্রুবক অক্ষর *zCrashFile = g.zCrashFile;
  পূর্ণসংখ্যা nName = (পূর্ণসংখ্যা)strlen(zName);
  পূর্ণসংখ্যা nCrashFile = (পূর্ণসংখ্যা)strlen(zCrashFile);

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

  ফেরত writeListSync(pCrash, isCrash);
}

/*
** Return the current file-size of the crash-file.
*/
স্থির পূর্ণসংখ্যা cf_File_আকার(sqlite3_file *pFile, sqlite_int64 *p_আকার){
  CrashFile *pCrash = (CrashFile *)pFile;
  *p_আকার = (i64)pCrash->i_আকার;
  ফেরত SQLITE_OK;
}

/*
** Calls related to file-locks are passed on to the real file handle.
*/
স্থির পূর্ণসংখ্যা cfLock(sqlite3_file *pFile, পূর্ণসংখ্যা eLock){
  ফেরত sqlite3OsLock(((CrashFile *)pFile)->pRealFile, eLock);
}
স্থির পূর্ণসংখ্যা cfUnlock(sqlite3_file *pFile, পূর্ণসংখ্যা eLock){
  ফেরত sqlite3OsUnlock(((CrashFile *)pFile)->pRealFile, eLock);
}
স্থির পূর্ণসংখ্যা cfCheckReservedLock(sqlite3_file *pFile, পূর্ণসংখ্যা *pResOut){
  ফেরত sqlite3OsCheckReservedLock(((CrashFile *)pFile)->pRealFile, pResOut);
}
স্থির পূর্ণসংখ্যা cfFileControl(sqlite3_file *pFile, পূর্ণসংখ্যা op, শূন্য *pArg){
  যদি( op==SQLITE_FCNTL_আকার_HINT ){
    CrashFile *pCrash = (CrashFile *)pFile;
    i64 nByte = *(i64 *)pArg;
    যদি( nByte>pCrash->i_আকার ){
      যদি( SQLITE_OK==writeListAppend(pFile, nByte, 0, 0) ){
        pCrash->i_আকার = (পূর্ণসংখ্যা)nByte;
      }
    }
    ফেরত SQLITE_OK;
  }
  ফেরত sqlite3OsFileControl(((CrashFile *)pFile)->pRealFile, op, pArg);
}

/*
** The xSectorSize() and xDeviceCharacteristics() functions return
** the global values configured by the [sqlite_crashparams] tcl
*  interface.
*/
স্থির পূর্ণসংখ্যা cf_Sector_আকার(sqlite3_file *pFile){
  ফেরত g.i_Sector_আকার;
}
স্থির পূর্ণসংখ্যা cfDeviceCharacteristics(sqlite3_file *pFile){
  ফেরত g.iDeviceCharacteristics;
}

/*
** Pass-throughs for WAL support.
*/
স্থির পূর্ণসংখ্যা cfShmLock(sqlite3_file *pFile, পূর্ণসংখ্যা ofst, পূর্ণসংখ্যা n, পূর্ণসংখ্যা flags){
  sqlite3_file *pReal = ((CrashFile*)pFile)->pRealFile;
  ফেরত pReal->pMethods->xShmLock(pReal, ofst, n, flags);
}
স্থির শূন্য cfShmBarrier(sqlite3_file *pFile){
  sqlite3_file *pReal = ((CrashFile*)pFile)->pRealFile;
  pReal->pMethods->xShmBarrier(pReal);
}
স্থির পূর্ণসংখ্যা cfShmUnmap(sqlite3_file *pFile, পূর্ণসংখ্যা delFlag){
  sqlite3_file *pReal = ((CrashFile*)pFile)->pRealFile;
  ফেরত pReal->pMethods->xShmUnmap(pReal, delFlag);
}
স্থির পূর্ণসংখ্যা cfShmMap(
  sqlite3_file *pFile,            /* Handle open on database file */
  পূর্ণসংখ্যা iRegion,                    /* Region to retrieve */
  পূর্ণসংখ্যা sz,                         /* Size of regions */
  পূর্ণসংখ্যা w,                          /* True to extend file if necessary */
  শূন্য অস্থির **pp              /* OUT: Mapped memory */
){
  sqlite3_file *pReal = ((CrashFile*)pFile)->pRealFile;
  ফেরত pReal->pMethods->xShmMap(pReal, iRegion, sz, w, pp);
}

স্থির ধ্রুবক sqlite3_io_methods CrashFileVtab = {
  2,                            /* iVersion */
  cfClose,                      /* xClose */
  cfRead,                       /* xRead */
  cfWrite,                      /* xWrite */
  cfTruncate,                   /* xTruncate */
  cfSync,                       /* xSync */
  cf_File_আকার,                   /* xFileSize */
  cfLock,                       /* xLock */
  cfUnlock,                     /* xUnlock */
  cfCheckReservedLock,          /* xCheckReservedLock */
  cfFileControl,                /* xFileControl */
  cf_Sector_আকার,                 /* xSectorSize */
  cfDeviceCharacteristics,      /* xDeviceCharacteristics */
  cfShmMap,                     /* xShmMap */
  cfShmLock,                    /* xShmLock */
  cfShmBarrier,                 /* xShmBarrier */
  cfShmUnmap                    /* xShmUnmap */
};

/*
** Application data for the crash VFS
*/
কাঠামো crash_App_তথ্য {
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
স্থির পূর্ণসংখ্যা cfOpen(
  sqlite3_vfs *pCfVfs,
  ধ্রুবক অক্ষর *zName,
  sqlite3_file *pFile,
  পূর্ণসংখ্যা flags,
  পূর্ণসংখ্যা *pOutFlags
){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_তথ্য;
  পূর্ণসংখ্যা rc;
  CrashFile *pWrapper = (CrashFile *)pFile;
  sqlite3_file *pReal = (sqlite3_file*)&pWrapper[1];

  memset(pWrapper, 0, আকার(CrashFile));
  rc = sqlite3OsOpen(pVfs, zName, pReal, flags, pOutFlags);

  যদি( rc==SQLITE_OK ){
    i64 i_আকার;
    pWrapper->pMethod = &CrashFileVtab;
    pWrapper->zName = (অক্ষর *)zName;
    pWrapper->pRealFile = pReal;
    rc = sqlite3_Os_File_আকার(pReal, &i_আকার);
    pWrapper->i_আকার = (পূর্ণসংখ্যা)i_আকার;
    pWrapper->flags = flags;
  }
  যদি( rc==SQLITE_OK ){
    pWrapper->n_তথ্য = (পূর্ণসংখ্যা)(4096 + pWrapper->i_আকার);
    pWrapper->z_তথ্য = crash_malloc(pWrapper->n_তথ্য);
    যদি( pWrapper->z_তথ্য ){
      /* os_unix.c contains an assert() that fails if the caller attempts
      ** to read data from the 512-byte locking region of a file opened
      ** with the SQLITE_OPEN_MAIN_DB flag. This region of a database file
      ** never contains valid data anyhow. So avoid doing such a read here.
      **
      ** UPDATE: It also contains an assert() verifying that each call
      ** to the xRead() method reads less than 128KB of data.
      */
      i64 iOff;

      memset(pWrapper->z_তথ্য, 0, pWrapper->n_তথ্য);
      জন্য(iOff=0; iOff<pWrapper->i_আকার; iOff += 512){
        পূর্ণসংখ্যা nRead = (পূর্ণসংখ্যা)(pWrapper->i_আকার - iOff);
        যদি( nRead>512 ) nRead = 512;
        rc = sqlite3OsRead(pReal, &pWrapper->z_তথ্য[iOff], nRead, iOff);
      }
    }অন্যথা{
      rc = SQLITE_NOMEM;
    }
  }
  যদি( rc!=SQLITE_OK && pWrapper->pMethod ){
    sqlite3OsClose(pFile);
  }
  ফেরত rc;
}

স্থির পূর্ণসংখ্যা cfDelete(sqlite3_vfs *pCfVfs, ধ্রুবক অক্ষর *zPath, পূর্ণসংখ্যা dirSync){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_তথ্য;
  ফেরত pVfs->xDelete(pVfs, zPath, dirSync);
}
স্থির পূর্ণসংখ্যা cfAccess(
  sqlite3_vfs *pCfVfs,
  ধ্রুবক অক্ষর *zPath,
  পূর্ণসংখ্যা flags,
  পূর্ণসংখ্যা *pResOut
){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_তথ্য;
  ফেরত pVfs->xAccess(pVfs, zPath, flags, pResOut);
}
স্থির পূর্ণসংখ্যা cfFullPathname(
  sqlite3_vfs *pCfVfs,
  ধ্রুবক অক্ষর *zPath,
  পূর্ণসংখ্যা nPathOut,
  অক্ষর *zPathOut
){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_তথ্য;
  ফেরত pVfs->xFullPathname(pVfs, zPath, nPathOut, zPathOut);
}
স্থির শূন্য *cfDlOpen(sqlite3_vfs *pCfVfs, ধ্রুবক অক্ষর *zPath){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_তথ্য;
  ফেরত pVfs->xDlOpen(pVfs, zPath);
}
স্থির শূন্য cfDlError(sqlite3_vfs *pCfVfs, পূর্ণসংখ্যা nByte, অক্ষর *z_Err_বার্তা){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_তথ্য;
  pVfs->xDlError(pVfs, nByte, z_Err_বার্তা);
}
স্থির শূন্য (*cfDlSym(sqlite3_vfs *pCfVfs, শূন্য *pH, ধ্রুবক অক্ষর *zSym))(শূন্য){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_তথ্য;
  ফেরত pVfs->xDlSym(pVfs, pH, zSym);
}
স্থির শূন্য cfDlClose(sqlite3_vfs *pCfVfs, শূন্য *pHandle){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_তথ্য;
  pVfs->xDlClose(pVfs, pHandle);
}
স্থির পূর্ণসংখ্যা cfRandomness(sqlite3_vfs *pCfVfs, পূর্ণসংখ্যা nByte, অক্ষর *zBufOut){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_তথ্য;
  ফেরত pVfs->xRandomness(pVfs, nByte, zBufOut);
}
স্থির পূর্ণসংখ্যা cfSleep(sqlite3_vfs *pCfVfs, পূর্ণসংখ্যা nMicro){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_তথ্য;
  ফেরত pVfs->xSleep(pVfs, nMicro);
}
স্থির পূর্ণসংখ্যা cfCurrentTime(sqlite3_vfs *pCfVfs, দ্বিগুণ *pTimeOut){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_তথ্য;
  ফেরত pVfs->xCurrentTime(pVfs, pTimeOut);
}
স্থির পূর্ণসংখ্যা cfGetLastError(sqlite3_vfs *pCfVfs, পূর্ণসংখ্যা n, অক্ষর *z){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_তথ্য;
  ফেরত pVfs->xGetLastError(pVfs, n, z);
}

স্থির পূর্ণসংখ্যা processDevSymArgs(
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[],
  পূর্ণসংখ্যা *piDeviceChar,
  পূর্ণসংখ্যা *pi_Sector_আকার
){
  কাঠামো DeviceFlag {
    অক্ষর *zName;
    পূর্ণসংখ্যা i_মান;
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

  পূর্ণসংখ্যা i;
  পূর্ণসংখ্যা iDc = 0;
  পূর্ণসংখ্যা i_Sector_আকার = 0;
  পূর্ণসংখ্যা setSectorsize = 0;
  পূর্ণসংখ্যা setDeviceChar = 0;

  জন্য(i=0; i<objc; i+=2){
    Tcl_আকার nOpt;
    অক্ষর *zOpt = Tcl_GetStringFromObj(objv[i], &nOpt);

    যদি( (nOpt>11 || nOpt<2 || strncmp("-sectorsize", zOpt, nOpt))
     && (nOpt>16 || nOpt<2 || strncmp("-characteristics", zOpt, nOpt))
    ){
      Tcl_Append_ফলাফল(interp,
        "Bad option: \"", zOpt,
        "\" - must be \"-characteristics\" or \"-sectorsize\"", ফাঁকা
      );
      ফেরত TCL_ERROR;
    }
    যদি( i==objc-1 ){
      Tcl_Append_ফলাফল(interp, "Option requires an argument: \"", zOpt, "\"", ফাঁকা);
      ফেরত TCL_ERROR;
    }

    যদি( zOpt[1]=='s' ){
      যদি( Tcl_GetIntFromObj(interp, objv[i+1], &i_Sector_আকার) ){
        ফেরত TCL_ERROR;
      }
      setSectorsize = 1;
    }অন্যথা{
      পূর্ণসংখ্যা j;
      Tcl_Obj **apObj;
      Tcl_আকার nObj;
      যদি( Tcl_ListObjGetElements(interp, objv[i+1], &nObj, &apObj) ){
        ফেরত TCL_ERROR;
      }
      জন্য(j=0; j<(পূর্ণসংখ্যা)nObj; j++){
        পূর্ণসংখ্যা rc;
        পূর্ণসংখ্যা iChoice;
        Tcl_Obj *pFlag = Tcl_DuplicateObj(apObj[j]);
        Tcl_Incr_Ref_গুনতি(pFlag);
        Tcl_UtfToLower(Tcl_GetString(pFlag));

        rc = Tcl_Get_সূচক_From_Obj_Struct(
            interp, pFlag, aFlag, আকার(aFlag[0]), "no such flag", 0, &iChoice
        );
        Tcl_Decr_Ref_গুনতি(pFlag);
        যদি( rc ){
          ফেরত TCL_ERROR;
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
    *pi_Sector_আকার = i_Sector_আকার;
  }

  ফেরত TCL_OK;
}

/*
** tclcmd:   sqlite3_crash_now
**
** Simulate a crash immediately. This function does not return
** (writeListSync() calls exit(-1)).
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI crashNowCmd(
  শূন্য * client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  যদি( objc!=1 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "");
    ফেরত TCL_ERROR;
  }
  writeListSync(0, 1);
  প্রতিজ্ঞা( 0 );
  ফেরত TCL_OK;
}

/*
** tclcmd:   sqlite_crash_enable ENABLE ?DEFAULT?
**
** Parameter ENABLE must be a boolean value. If true, then the "crash"
** vfs is added to the system. If false, it is removed.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI crashEnableCmd(
  শূন্য * client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  পূর্ণসংখ্যা isEnable;
  পূর্ণসংখ্যা isDefault = 0;
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
    ফেরত TCL_ERROR;
  }

  যদি( Tcl_GetBooleanFromObj(interp, objv[1], &isEnable) ){
    ফেরত TCL_ERROR;
  }
  যদি( objc==3 && Tcl_GetBooleanFromObj(interp, objv[2], &isDefault) ){
    ফেরত TCL_ERROR;
  }

  যদি( (isEnable && crashVfs.p_App_তথ্য) || (!isEnable && !crashVfs.p_App_তথ্য) ){
    ফেরত TCL_OK;
  }

  যদি( crashVfs.p_App_তথ্য==0 ){
    sqlite3_vfs *pOriginalVfs = sqlite3_vfs_find(0);
    crashVfs.mxPathname = pOriginalVfs->mxPathname;
    crashVfs.p_App_তথ্য = (শূন্য *)pOriginalVfs;
    crashVfs.szOsFile = আকার(CrashFile) + pOriginalVfs->szOsFile;
    sqlite3_vfs_register(&crashVfs, isDefault);
  }অন্যথা{
    crashVfs.p_App_তথ্য = 0;
    sqlite3_vfs_unregister(&crashVfs);
  }

  ফেরত TCL_OK;
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
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI crashParamsObjCmd(
  শূন্য * client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  পূর্ণসংখ্যা iDelay;
  ধ্রুবক অক্ষর *zCrashFile;
  Tcl_আকার nCrashFile;
  পূর্ণসংখ্যা iDc, i_Sector_আকার;

  iDc = -1;
  i_Sector_আকার = -1;

  যদি( objc<3 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "?OPTIONS? DELAY CRASHFILE");
    যাও error;
  }

  zCrashFile = Tcl_GetStringFromObj(objv[objc-1], &nCrashFile);
  যদি( nCrashFile>=আকার(g.zCrashFile) ){
    Tcl_Append_ফলাফল(interp, "Filename is too long: \"", zCrashFile, "\"", ফাঁকা);
    যাও error;
  }
  যদি( Tcl_GetIntFromObj(interp, objv[objc-2], &iDelay) ){
    যাও error;
  }

  যদি( processDevSymArgs(interp, objc-3, &objv[1], &iDc, &i_Sector_আকার) ){
    ফেরত TCL_ERROR;
  }

  যদি( iDc>=0 ){
    g.iDeviceCharacteristics = iDc;
  }
  যদি( i_Sector_আকার>=0 ){
    g.i_Sector_আকার = i_Sector_আকার;
  }

  g.iCrash = iDelay;
  memcpy(g.zCrashFile, zCrashFile, nCrashFile+1);
  sqlite3CrashTestEnable = 1;
  ফেরত TCL_OK;

error:
  ফেরত TCL_ERROR;
}

স্থির পূর্ণসংখ্যা SQLITE_TCLAPI devSymObjCmd(
  শূন্য * client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  শূন্য devsym_register(পূর্ণসংখ্যা iDeviceChar, পূর্ণসংখ্যা i_Sector_আকার);

  পূর্ণসংখ্যা iDc = -1;
  পূর্ণসংখ্যা i_Sector_আকার = -1;

  যদি( processDevSymArgs(interp, objc-1, &objv[1], &iDc, &i_Sector_আকার) ){
    ফেরত TCL_ERROR;
  }
  devsym_register(iDc, i_Sector_আকার);

  ফেরত TCL_OK;
}

/*
** tclcmd: sqlite3_crash_on_write N
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI writeCrashObjCmd(
  শূন্য * client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  শূন্য devsym_crash_on_write(পূর্ণসংখ্যা);
  পূর্ণসংখ্যা nWrite = 0;

  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "NWRITE");
    ফেরত TCL_ERROR;
  }
  যদি( Tcl_GetIntFromObj(interp, objv[1], &nWrite) ){
    ফেরত TCL_ERROR;
  }

  devsym_crash_on_write(nWrite);
  ফেরত TCL_OK;
}

/*
** tclcmd: unregister_devsim
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI dsUnregisterObjCmd(
  শূন্য * client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  শূন্য devsym_unregister(শূন্য);

  যদি( objc!=1 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "");
    ফেরত TCL_ERROR;
  }

  devsym_unregister();
  ফেরত TCL_OK;
}

/*
** tclcmd: register_jt_vfs ?-default? PARENT-VFS
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI jtObjCmd(
  শূন্য * client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  পূর্ণসংখ্যা jt_register(অক্ষর *, পূর্ণসংখ্যা);
  অক্ষর *zParent = 0;

  যদি( objc!=2 && objc!=3 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "?-default? PARENT-VFS");
    ফেরত TCL_ERROR;
  }
  zParent = Tcl_GetString(objv[1]);
  যদি( objc==3 ){
    যদি( strcmp(zParent, "-default") ){
      Tcl_Append_ফলাফল(interp,
          "bad option \"", zParent, "\": must be -default", ফাঁকা
      );
      ফেরত TCL_ERROR;
    }
    zParent = Tcl_GetString(objv[2]);
  }

  যদি( !(*zParent) ){
    zParent = 0;
  }
  যদি( jt_register(zParent, objc==3) ){
    Tcl_Append_ফলাফল(interp, "Error in jt_register", ফাঁকা);
    ফেরত TCL_ERROR;
  }

  ফেরত TCL_OK;
}

/*
** tclcmd: unregister_jt_vfs
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI jtUnregisterObjCmd(
  শূন্য * client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  শূন্য jt_unregister(শূন্য);

  যদি( objc!=1 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "");
    ফেরত TCL_ERROR;
  }

  jt_unregister();
  ফেরত TCL_OK;
}

#endif /* SQLITE_OMIT_DISKIO */

/*
** This procedure registers the TCL procedures defined in this file.
*/
পূর্ণসংখ্যা Sqlitetest6_Init(Tcl_Interp *interp){
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
  ফেরত TCL_OK;
}

#endif /* SQLITE_TEST */
