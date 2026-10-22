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
#ఒకవేళ SQLITE_TEST          /* This file is used for testing only */
#అన్తర్భావ "sqliteInt.h"
#అన్తర్భావ "tclsqlite.h"

#నిర్వచితం_కాకపోతే SQLITE_OMIT_DISKIO  /* This file is a no-op if disk I/O is disabled */

/* #define TRACE_CRASHTEST */

రకనిర్వచనం నిర్మాణం CrashFile CrashFile;
రకనిర్వచనం నిర్మాణం CrashGlobal CrashGlobal;
రకనిర్వచనం నిర్మాణం Write_బఫరు Write_బఫరు;

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
నిర్మాణం Write_బఫరు {
  i64 i_స్థానభ్రంశం;                 /* Byte offset of the start of this write() */
  పూర్ణాంకం nBuf;                    /* Number of bytes written */
  u8 *zBuf;                    /* Pointer to copy of written data */
  CrashFile *pFile;            /* File this write() applies to */

  Write_బఫరు *p_తదుపరి;          /* Next in CrashGlobal.pWriteList */
};

నిర్మాణం CrashFile {
  స్థిరరాశి sqlite3_io_methods *p_పద్ధతి;   /* Must be first */
  sqlite3_file *pRealFile;             /* Underlying "real" file handle */
  అక్షరం *zName;
  పూర్ణాంకం flags;                           /* Flags the file was opened with */

  /* Cache of the entire file. This is used to speed up OsRead() and
  ** OsFileSize() calls. Although both could be done by traversing the
  ** write-list, in practice this is impractically slow.
  */
  u8 *z_దత్తాంశం;                           /* Buffer containing file contents */
  పూర్ణాంకం n_దత్తాంశం;                           /* Size of buffer allocated at zData */
  i64 i_కొలత;                           /* Size of file in bytes */
};

నిర్మాణం CrashGlobal {
  Write_బఫరు *pWriteList;     /* Head of write-list */
  Write_బఫరు *p_Write_List_చివర;  /* End of write-list */

  పూర్ణాంకం i_Sector_కొలత;             /* Value of simulated sector size */
  పూర్ణాంకం iDeviceCharacteristics;  /* Value of simulated device characteristics */

  పూర్ణాంకం iCrash;                  /* Crash on the iCrash'th call to xSync() */
  అక్షరం zCrashFile[500];        /* Crash during an xSync() on this file */
};

స్థిర CrashGlobal g = {0, 0, SQLITE_DEFAULT_SECTOR_కొలత, 0, 0};

/*
** Set this global variable to 1 to enable crash testing.
*/
స్థిర పూర్ణాంకం sqlite3CrashTestEnable = 0;

స్థిర శూన్య *crash_malloc(పూర్ణాంకం nByte){
  తిరిగి (శూన్య *)Tcl_AttemptAlloc((size_t)nByte);
}
స్థిర శూన్య crash_free(శూన్య *p){
  Tcl_Free(p);
}
స్థిర శూన్య *crash_realloc(శూన్య *p, పూర్ణాంకం n){
  తిరిగి (శూన్య *)Tcl_AttemptRealloc(p, (size_t)n);
}

/*
** Wrapper around the sqlite3OsWrite() function that avoids writing to the
** 512 byte block beginning at offset PENDING_BYTE.
*/
స్థిర పూర్ణాంకం writeDbFile(CrashFile *p, u8 *z, i64 iAmt, i64 iOff){
  పూర్ణాంకం rc = SQLITE_OK;
  పూర్ణాంకం iSkip = 0;
  ఒకవేళ( (iAmt-iSkip)>0 ){
    rc = sqlite3OsWrite(p->pRealFile, &z[iSkip], (పూర్ణాంకం)(iAmt-iSkip), iOff+iSkip);
  }
  తిరిగి rc;
}

/*
** Flush the write-list as if xSync() had been called on file handle
** pFile. If isCrash is true, simulate a crash.
*/
స్థిర పూర్ణాంకం writeListSync(CrashFile *pFile, పూర్ణాంకం isCrash){
  పూర్ణాంకం rc = SQLITE_OK;
  పూర్ణాంకం iDc = g.iDeviceCharacteristics;

  Write_బఫరు *pWrite;
  Write_బఫరు **ppPtr;

  /* If this is not a crash simulation, set pFinal to point to the
  ** last element of the write-list that is associated with file handle
  ** pFile.
  **
  ** If this is a crash simulation, set pFinal to an arbitrarily selected
  ** element of the write-list.
  */
  Write_బఫరు *pFinal = 0;
  ఒకవేళ( !isCrash ){
    కొరకు(pWrite=g.pWriteList; pWrite; pWrite=pWrite->p_తదుపరి){
      ఒకవేళ( pWrite->pFile==pFile ){
        pFinal = pWrite;
      }
    }
  }లేకపోతే ఒకవేళ( iDc&(SQLITE_IOCAP_SEQUENTIAL|SQLITE_IOCAP_సురక్షిత_APPEND) ){
    పూర్ణాంకం nWrite = 0;
    పూర్ణాంకం iFinal;
    కొరకు(pWrite=g.pWriteList; pWrite; pWrite=pWrite->p_తదుపరి) nWrite++;
    sqlite3_randomness(పరిమాణం(పూర్ణాంకం), &iFinal);
    iFinal = ((iFinal<0)?-1*iFinal:iFinal)%nWrite;
    కొరకు(pWrite=g.pWriteList; iFinal>0; pWrite=pWrite->p_తదుపరి) iFinal--;
    pFinal = pWrite;
  }

#నిర్వచితమైతే TRACE_CRASHTEST
  ఒకవేళ( pFile ){
    రాయి("Sync %s (is %s crash)\n", pFile->zName, (isCrash?"a":"not a"));
  }
#అయితే_ముగింపు

  ppPtr = &g.pWriteList;
  కొరకు(pWrite=*ppPtr; rc==SQLITE_OK && pWrite; pWrite=*ppPtr){
    sqlite3_file *pRealFile = pWrite->pFile->pRealFile;

    /* (eAction==1)      -> write block out normally,
    ** (eAction==2)      -> do nothing,
    ** (eAction==3)      -> trash sectors.
    */
    పూర్ణాంకం eAction = 0;
    ఒకవేళ( !isCrash ){
      eAction = 2;
      ఒకవేళ( (pWrite->pFile==pFile || iDc&SQLITE_IOCAP_SEQUENTIAL) ){
        eAction = 1;
      }
    }లేకపోతే{
      అక్షరం random;
      sqlite3_randomness(1, &random);

      /* Do not select option 3 (sector trashing) if the IOCAP_ATOMIC flag
      ** is set or this is an OsTruncate(), not an Oswrite().
      */
      ఒకవేళ( (iDc&SQLITE_IOCAP_ATOMIC) || (pWrite->zBuf==0) ){
        random &= 0x01;
      }

      /* If IOCAP_SEQUENTIAL is set and this is not the final entry
      ** in the truncated write-list, always select option 1 (write
      ** out correctly).
      */
      ఒకవేళ( (iDc&SQLITE_IOCAP_SEQUENTIAL && pWrite!=pFinal) ){
        random = 0;
      }

      /* If IOCAP_SAFE_APPEND is set and this OsWrite() operation is
      ** an append (first byte of the written region is 1 byte past the
      ** current EOF), always select option 1 (write out correctly).
      */
      ఒకవేళ( iDc&SQLITE_IOCAP_సురక్షిత_APPEND && pWrite->zBuf ){
        i64 i_కొలత;
        sqlite3_Os_File_కొలత(pRealFile, &i_కొలత);
        ఒకవేళ( i_కొలత==pWrite->i_స్థానభ్రంశం ){
          random = 0;
        }
      }

      ఒకవేళ( (random&0x06)==0x06 ){
        eAction = 3;
      }లేకపోతే{
        eAction = ((random&0x01)?2:1);
      }
    }

    ఎంపిక( eAction ){
      స్థితి 1: {               /* Write out correctly */
        ఒకవేళ( pWrite->zBuf ){
          rc = writeDbFile(
              pWrite->pFile, pWrite->zBuf, pWrite->nBuf, pWrite->i_స్థానభ్రంశం
          );
        }లేకపోతే{
          rc = sqlite3OsTruncate(pRealFile, pWrite->i_స్థానభ్రంశం);
        }
        *ppPtr = pWrite->p_తదుపరి;
#నిర్వచితమైతే TRACE_CRASHTEST
        ఒకవేళ( isCrash ){
          రాయి("Writing %d bytes @ %d (%s)\n",
            pWrite->nBuf, (పూర్ణాంకం)pWrite->i_స్థానభ్రంశం, pWrite->pFile->zName
          );
        }
#అయితే_ముగింపు
        crash_free(pWrite);
        ఆపు;
      }
      స్థితి 2: {               /* Do nothing */
        ppPtr = &pWrite->p_తదుపరి;
#నిర్వచితమైతే TRACE_CRASHTEST
        ఒకవేళ( isCrash ){
          రాయి("Omiting %d bytes @ %d (%s)\n",
            pWrite->nBuf, (పూర్ణాంకం)pWrite->i_స్థానభ్రంశం, pWrite->pFile->zName
          );
        }
#అయితే_ముగింపు
        ఆపు;
      }
      స్థితి 3: {               /* Trash sectors */
        u8 *zGarbage;
        పూర్ణాంకం i_మొదటి = (పూర్ణాంకం)(pWrite->i_స్థానభ్రంశం/g.i_Sector_కొలత);
        పూర్ణాంకం i_చివరి = (పూర్ణాంకం)((pWrite->i_స్థానభ్రంశం+pWrite->nBuf-1)/g.i_Sector_కొలత);

        ప్రతిజ్ఞా(pWrite->zBuf);

#నిర్వచితమైతే TRACE_CRASHTEST
        రాయి("Trashing %d sectors (%d bytes) @ %lld (sector %d) (%s)\n",
            1+i_చివరి-i_మొదటి, (1+i_చివరి-i_మొదటి)*g.i_Sector_కొలత,
            pWrite->i_స్థానభ్రంశం, i_మొదటి, pWrite->pFile->zName
        );
#అయితే_ముగింపు

        zGarbage = crash_malloc(g.i_Sector_కొలత);
        ఒకవేళ( zGarbage ){
          sqlite3_int64 i;
          కొరకు(i=i_మొదటి; rc==SQLITE_OK && i<=i_చివరి; i++){
            sqlite3_randomness(g.i_Sector_కొలత, zGarbage);
            rc = writeDbFile(
              pWrite->pFile, zGarbage, g.i_Sector_కొలత, i*g.i_Sector_కొలత
            );
          }
          crash_free(zGarbage);
        }లేకపోతే{
          rc = SQLITE_NOMEM;
        }

        ppPtr = &pWrite->p_తదుపరి;
        ఆపు;
      }

      అప్రమేయ:
        ప్రతిజ్ఞా(!"Cannot happen");
    }

    ఒకవేళ( pWrite==pFinal ) ఆపు;
  }

  ఒకవేళ( rc==SQLITE_OK && isCrash ){
    నిష్క్రమించు(-1);
  }

  కొరకు(pWrite=g.pWriteList; pWrite && pWrite->p_తదుపరి; pWrite=pWrite->p_తదుపరి);
  g.p_Write_List_చివర = pWrite;

  తిరిగి rc;
}

/*
** Add an entry to the end of the write-list.
*/
స్థిర పూర్ణాంకం writeListAppend(
  sqlite3_file *pFile,
  sqlite3_int64 i_స్థానభ్రంశం,
  స్థిరరాశి u8 *zBuf,
  పూర్ణాంకం nBuf
){
  Write_బఫరు *pNew;

  ప్రతిజ్ఞా((zBuf && nBuf) || (!nBuf && !zBuf));

  pNew = (Write_బఫరు *)crash_malloc(పరిమాణం(Write_బఫరు) + nBuf);
  ఒకవేళ( pNew==0 ){
    fprintf(stderr, "out of memory in the crash simulator\n");
  }
  memset(pNew, 0, పరిమాణం(Write_బఫరు)+nBuf);
  pNew->i_స్థానభ్రంశం = i_స్థానభ్రంశం;
  pNew->nBuf = nBuf;
  pNew->pFile = (CrashFile *)pFile;
  ఒకవేళ( zBuf ){
    pNew->zBuf = (u8 *)&pNew[1];
    జ్ఞాపకం_నకలు(pNew->zBuf, zBuf, nBuf);
  }

  ఒకవేళ( g.pWriteList ){
    ప్రతిజ్ఞా(g.p_Write_List_చివర);
    g.p_Write_List_చివర->p_తదుపరి = pNew;
  }లేకపోతే{
    g.pWriteList = pNew;
  }
  g.p_Write_List_చివర = pNew;
 
  తిరిగి SQLITE_OK;
}

/*
** Close a crash-file.
*/
స్థిర పూర్ణాంకం cfClose(sqlite3_file *pFile){
  CrashFile *pCrash = (CrashFile *)pFile;
  writeListSync(pCrash, 0);
  sqlite3OsClose(pCrash->pRealFile);
  తిరిగి SQLITE_OK;
}

/*
** Read data from a crash-file.
*/
స్థిర పూర్ణాంకం cfRead(
  sqlite3_file *pFile,
  శూన్య *zBuf,
  పూర్ణాంకం iAmt,
  sqlite_int64 iOfst
){
  CrashFile *pCrash = (CrashFile *)pFile;
  పూర్ణాంకం nCopy = (పూర్ణాంకం)MIN((i64)iAmt, (pCrash->i_కొలత - iOfst));

  ఒకవేళ( nCopy>0 ){
    జ్ఞాపకం_నకలు(zBuf, &pCrash->z_దత్తాంశం[iOfst], nCopy);
  }

  /* Check the file-size to see if this is a short-read */
  ఒకవేళ( nCopy<iAmt ){
    తిరిగి SQLITE_IOERR_SHORT_READ;
  }

  తిరిగి SQLITE_OK;
}

/*
** Write data to a crash-file.
*/
స్థిర పూర్ణాంకం cfWrite(
  sqlite3_file *pFile,
  స్థిరరాశి శూన్య *zBuf,
  పూర్ణాంకం iAmt,
  sqlite_int64 iOfst
){
  CrashFile *pCrash = (CrashFile *)pFile;
  ఒకవేళ( iAmt+iOfst>pCrash->i_కొలత ){
    pCrash->i_కొలత = (పూర్ణాంకం)(iAmt+iOfst);
  }
  అయితే( pCrash->i_కొలత>pCrash->n_దత్తాంశం ){
    u8 *zNew;
    పూర్ణాంకం nNew = (pCrash->n_దత్తాంశం*2) + 4096;
    zNew = crash_realloc(pCrash->z_దత్తాంశం, nNew);
    ఒకవేళ( !zNew ){
      తిరిగి SQLITE_NOMEM;
    }
    memset(&zNew[pCrash->n_దత్తాంశం], 0, nNew-pCrash->n_దత్తాంశం);
    pCrash->n_దత్తాంశం = nNew;
    pCrash->z_దత్తాంశం = zNew;
  }
  జ్ఞాపకం_నకలు(&pCrash->z_దత్తాంశం[iOfst], zBuf, iAmt);
  తిరిగి writeListAppend(pFile, iOfst, zBuf, iAmt);
}

/*
** Truncate a crash-file.
*/
స్థిర పూర్ణాంకం cfTruncate(sqlite3_file *pFile, sqlite_int64 కొలత){
  CrashFile *pCrash = (CrashFile *)pFile;
  ప్రతిజ్ఞా(కొలత>=0);
  ఒకవేళ( pCrash->i_కొలత>కొలత ){
    pCrash->i_కొలత = (పూర్ణాంకం)కొలత;
  }
  తిరిగి writeListAppend(pFile, కొలత, 0, 0);
}

/*
** Sync a crash-file.
*/
స్థిర పూర్ణాంకం cfSync(sqlite3_file *pFile, పూర్ణాంకం flags){
  CrashFile *pCrash = (CrashFile *)pFile;
  పూర్ణాంకం isCrash = 0;

  స్థిరరాశి అక్షరం *zName = pCrash->zName;
  స్థిరరాశి అక్షరం *zCrashFile = g.zCrashFile;
  పూర్ణాంకం nName = (పూర్ణాంకం)అక్షరశ్రేణి_పొడవు(zName);
  పూర్ణాంకం nCrashFile = (పూర్ణాంకం)అక్షరశ్రేణి_పొడవు(zCrashFile);

  ఒకవేళ( nCrashFile>0 && zCrashFile[nCrashFile-1]=='*' ){
    nCrashFile--;
    ఒకవేళ( nName>nCrashFile ) nName = nCrashFile;
  }

#నిర్వచితమైతే TRACE_CRASHTEST
  రాయి("cfSync(): nName = %d, nCrashFile = %d, zName = %s, zCrashFile = %s\n",
         nName, nCrashFile, zName, zCrashFile);
#అయితే_ముగింపు

  ఒకవేళ( nName==nCrashFile && 0==memcmp(zName, zCrashFile, nName) ){
#నిర్వచితమైతే TRACE_CRASHTEST
    రాయి("cfSync(): name matched, g.iCrash = %d\n", g.iCrash);
#అయితే_ముగింపు
    ఒకవేళ( (--g.iCrash)==0 ) isCrash = 1;
  }

  తిరిగి writeListSync(pCrash, isCrash);
}

/*
** Return the current file-size of the crash-file.
*/
స్థిర పూర్ణాంకం cf_File_కొలత(sqlite3_file *pFile, sqlite_int64 *p_కొలత){
  CrashFile *pCrash = (CrashFile *)pFile;
  *p_కొలత = (i64)pCrash->i_కొలత;
  తిరిగి SQLITE_OK;
}

/*
** Calls related to file-locks are passed on to the real file handle.
*/
స్థిర పూర్ణాంకం cf_తాళం(sqlite3_file *pFile, పూర్ణాంకం e_తాళం){
  తిరిగి sqlite3_Os_తాళం(((CrashFile *)pFile)->pRealFile, e_తాళం);
}
స్థిర పూర్ణాంకం cfUnlock(sqlite3_file *pFile, పూర్ణాంకం e_తాళం){
  తిరిగి sqlite3OsUnlock(((CrashFile *)pFile)->pRealFile, e_తాళం);
}
స్థిర పూర్ణాంకం cf_Check_Reserved_తాళం(sqlite3_file *pFile, పూర్ణాంకం *p_Res_బహిర్గతం){
  తిరిగి sqlite3_Os_Check_Reserved_తాళం(((CrashFile *)pFile)->pRealFile, p_Res_బహిర్గతం);
}
స్థిర పూర్ణాంకం cfFileControl(sqlite3_file *pFile, పూర్ణాంకం op, శూన్య *pArg){
  ఒకవేళ( op==SQLITE_FCNTL_కొలత_HINT ){
    CrashFile *pCrash = (CrashFile *)pFile;
    i64 nByte = *(i64 *)pArg;
    ఒకవేళ( nByte>pCrash->i_కొలత ){
      ఒకవేళ( SQLITE_OK==writeListAppend(pFile, nByte, 0, 0) ){
        pCrash->i_కొలత = (పూర్ణాంకం)nByte;
      }
    }
    తిరిగి SQLITE_OK;
  }
  తిరిగి sqlite3OsFileControl(((CrashFile *)pFile)->pRealFile, op, pArg);
}

/*
** The xSectorSize() and xDeviceCharacteristics() functions return
** the global values configured by the [sqlite_crashparams] tcl
*  interface.
*/
స్థిర పూర్ణాంకం cf_Sector_కొలత(sqlite3_file *pFile){
  తిరిగి g.i_Sector_కొలత;
}
స్థిర పూర్ణాంకం cfDeviceCharacteristics(sqlite3_file *pFile){
  తిరిగి g.iDeviceCharacteristics;
}

/*
** Pass-throughs for WAL support.
*/
స్థిర పూర్ణాంకం cf_Shm_తాళం(sqlite3_file *pFile, పూర్ణాంకం ofst, పూర్ణాంకం n, పూర్ణాంకం flags){
  sqlite3_file *pReal = ((CrashFile*)pFile)->pRealFile;
  తిరిగి pReal->pMethods->x_Shm_తాళం(pReal, ofst, n, flags);
}
స్థిర శూన్య cfShmBarrier(sqlite3_file *pFile){
  sqlite3_file *pReal = ((CrashFile*)pFile)->pRealFile;
  pReal->pMethods->xShmBarrier(pReal);
}
స్థిర పూర్ణాంకం cfShmUnmap(sqlite3_file *pFile, పూర్ణాంకం delFlag){
  sqlite3_file *pReal = ((CrashFile*)pFile)->pRealFile;
  తిరిగి pReal->pMethods->xShmUnmap(pReal, delFlag);
}
స్థిర పూర్ణాంకం cfShmMap(
  sqlite3_file *pFile,            /* Handle open on database file */
  పూర్ణాంకం iRegion,                    /* Region to retrieve */
  పూర్ణాంకం sz,                         /* Size of regions */
  పూర్ణాంకం w,                          /* True to extend file if necessary */
  శూన్య అస్థిర **pp              /* OUT: Mapped memory */
){
  sqlite3_file *pReal = ((CrashFile*)pFile)->pRealFile;
  తిరిగి pReal->pMethods->xShmMap(pReal, iRegion, sz, w, pp);
}

స్థిర స్థిరరాశి sqlite3_io_methods CrashFileVtab = {
  2,                            /* iVersion */
  cfClose,                      /* xClose */
  cfRead,                       /* xRead */
  cfWrite,                      /* xWrite */
  cfTruncate,                   /* xTruncate */
  cfSync,                       /* xSync */
  cf_File_కొలత,                   /* xFileSize */
  cf_తాళం,                       /* xLock */
  cfUnlock,                     /* xUnlock */
  cf_Check_Reserved_తాళం,          /* xCheckReservedLock */
  cfFileControl,                /* xFileControl */
  cf_Sector_కొలత,                 /* xSectorSize */
  cfDeviceCharacteristics,      /* xDeviceCharacteristics */
  cfShmMap,                     /* xShmMap */
  cf_Shm_తాళం,                    /* xShmLock */
  cfShmBarrier,                 /* xShmBarrier */
  cfShmUnmap                    /* xShmUnmap */
};

/*
** Application data for the crash VFS
*/
నిర్మాణం crash_App_దత్తాంశం {
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
స్థిర పూర్ణాంకం cfOpen(
  sqlite3_vfs *pCfVfs,
  స్థిరరాశి అక్షరం *zName,
  sqlite3_file *pFile,
  పూర్ణాంకం flags,
  పూర్ణాంకం *p_బహిర్గతం_Flags
){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_దత్తాంశం;
  పూర్ణాంకం rc;
  CrashFile *pWrapper = (CrashFile *)pFile;
  sqlite3_file *pReal = (sqlite3_file*)&pWrapper[1];

  memset(pWrapper, 0, పరిమాణం(CrashFile));
  rc = sqlite3OsOpen(pVfs, zName, pReal, flags, p_బహిర్గతం_Flags);

  ఒకవేళ( rc==SQLITE_OK ){
    i64 i_కొలత;
    pWrapper->p_పద్ధతి = &CrashFileVtab;
    pWrapper->zName = (అక్షరం *)zName;
    pWrapper->pRealFile = pReal;
    rc = sqlite3_Os_File_కొలత(pReal, &i_కొలత);
    pWrapper->i_కొలత = (పూర్ణాంకం)i_కొలత;
    pWrapper->flags = flags;
  }
  ఒకవేళ( rc==SQLITE_OK ){
    pWrapper->n_దత్తాంశం = (పూర్ణాంకం)(4096 + pWrapper->i_కొలత);
    pWrapper->z_దత్తాంశం = crash_malloc(pWrapper->n_దత్తాంశం);
    ఒకవేళ( pWrapper->z_దత్తాంశం ){
      /* os_unix.c contains an assert() that fails if the caller attempts
      ** to read data from the 512-byte locking region of a file opened
      ** with the SQLITE_OPEN_MAIN_DB flag. This region of a database file
      ** never contains valid data anyhow. So avoid doing such a read here.
      **
      ** UPDATE: It also contains an assert() verifying that each call
      ** to the xRead() method reads less than 128KB of data.
      */
      i64 iOff;

      memset(pWrapper->z_దత్తాంశం, 0, pWrapper->n_దత్తాంశం);
      కొరకు(iOff=0; iOff<pWrapper->i_కొలత; iOff += 512){
        పూర్ణాంకం nRead = (పూర్ణాంకం)(pWrapper->i_కొలత - iOff);
        ఒకవేళ( nRead>512 ) nRead = 512;
        rc = sqlite3OsRead(pReal, &pWrapper->z_దత్తాంశం[iOff], nRead, iOff);
      }
    }లేకపోతే{
      rc = SQLITE_NOMEM;
    }
  }
  ఒకవేళ( rc!=SQLITE_OK && pWrapper->p_పద్ధతి ){
    sqlite3OsClose(pFile);
  }
  తిరిగి rc;
}

స్థిర పూర్ణాంకం cfDelete(sqlite3_vfs *pCfVfs, స్థిరరాశి అక్షరం *z_మార్గం, పూర్ణాంకం dirSync){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_దత్తాంశం;
  తిరిగి pVfs->xDelete(pVfs, z_మార్గం, dirSync);
}
స్థిర పూర్ణాంకం cfAccess(
  sqlite3_vfs *pCfVfs,
  స్థిరరాశి అక్షరం *z_మార్గం,
  పూర్ణాంకం flags,
  పూర్ణాంకం *p_Res_బహిర్గతం
){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_దత్తాంశం;
  తిరిగి pVfs->xAccess(pVfs, z_మార్గం, flags, p_Res_బహిర్గతం);
}
స్థిర పూర్ణాంకం cf_నిండిన_Pathname(
  sqlite3_vfs *pCfVfs,
  స్థిరరాశి అక్షరం *z_మార్గం,
  పూర్ణాంకం n_మార్గం_బహిర్గతం,
  అక్షరం *z_మార్గం_బహిర్గతం
){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_దత్తాంశం;
  తిరిగి pVfs->x_నిండిన_Pathname(pVfs, z_మార్గం, n_మార్గం_బహిర్గతం, z_మార్గం_బహిర్గతం);
}
స్థిర శూన్య *cfDlOpen(sqlite3_vfs *pCfVfs, స్థిరరాశి అక్షరం *z_మార్గం){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_దత్తాంశం;
  తిరిగి pVfs->xDlOpen(pVfs, z_మార్గం);
}
స్థిర శూన్య cfDlError(sqlite3_vfs *pCfVfs, పూర్ణాంకం nByte, అక్షరం *z_Err_సందేశం){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_దత్తాంశం;
  pVfs->xDlError(pVfs, nByte, z_Err_సందేశం);
}
స్థిర శూన్య (*cfDlSym(sqlite3_vfs *pCfVfs, శూన్య *pH, స్థిరరాశి అక్షరం *zSym))(శూన్య){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_దత్తాంశం;
  తిరిగి pVfs->xDlSym(pVfs, pH, zSym);
}
స్థిర శూన్య cfDlClose(sqlite3_vfs *pCfVfs, శూన్య *pHandle){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_దత్తాంశం;
  pVfs->xDlClose(pVfs, pHandle);
}
స్థిర పూర్ణాంకం cfRandomness(sqlite3_vfs *pCfVfs, పూర్ణాంకం nByte, అక్షరం *z_Buf_బహిర్గతం){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_దత్తాంశం;
  తిరిగి pVfs->xRandomness(pVfs, nByte, z_Buf_బహిర్గతం);
}
స్థిర పూర్ణాంకం cfSleep(sqlite3_vfs *pCfVfs, పూర్ణాంకం nMicro){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_దత్తాంశం;
  తిరిగి pVfs->xSleep(pVfs, nMicro);
}
స్థిర పూర్ణాంకం cf_Current_సమయం(sqlite3_vfs *pCfVfs, ద్విగుణ *p_సమయం_బహిర్గతం){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_దత్తాంశం;
  తిరిగి pVfs->x_Current_సమయం(pVfs, p_సమయం_బహిర్గతం);
}
స్థిర పూర్ణాంకం cf_Get_చివరి_Error(sqlite3_vfs *pCfVfs, పూర్ణాంకం n, అక్షరం *z){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_దత్తాంశం;
  తిరిగి pVfs->x_Get_చివరి_Error(pVfs, n, z);
}

స్థిర పూర్ణాంకం process_Dev_Sym_వాదనలు(
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[],
  పూర్ణాంకం *piDeviceChar,
  పూర్ణాంకం *pi_Sector_కొలత
){
  నిర్మాణం DeviceFlag {
    అక్షరం *zName;
    పూర్ణాంకం i_విలువ;
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
    { "safe_append",         SQLITE_IOCAP_సురక్షిత_APPEND           },
    { "powersafe_overwrite", SQLITE_IOCAP_POWERSAFE_OVERWRITE   },
    { "batch-atomic",        SQLITE_IOCAP_BATCH_ATOMIC          },
    { 0, 0 }
  };

  పూర్ణాంకం i;
  పూర్ణాంకం iDc = 0;
  పూర్ణాంకం i_Sector_కొలత = 0;
  పూర్ణాంకం సమితి_Sectorsize = 0;
  పూర్ణాంకం సమితి_Device_Char = 0;

  కొరకు(i=0; i<objc; i+=2){
    Tcl_కొలత nOpt;
    అక్షరం *zOpt = Tcl_GetStringFromObj(objv[i], &nOpt);

    ఒకవేళ( (nOpt>11 || nOpt<2 || అక్షరశ్రేణి_పోలిక("-sectorsize", zOpt, nOpt))
     && (nOpt>16 || nOpt<2 || అక్షరశ్రేణి_పోలిక("-characteristics", zOpt, nOpt))
    ){
      Tcl_Append_ఫలితం(interp,
        "Bad option: \"", zOpt,
        "\" - must be \"-characteristics\" or \"-sectorsize\"", ఖాళీ
      );
      తిరిగి TCL_ERROR;
    }
    ఒకవేళ( i==objc-1 ){
      Tcl_Append_ఫలితం(interp, "Option requires an argument: \"", zOpt, "\"", ఖాళీ);
      తిరిగి TCL_ERROR;
    }

    ఒకవేళ( zOpt[1]=='s' ){
      ఒకవేళ( Tcl_GetIntFromObj(interp, objv[i+1], &i_Sector_కొలత) ){
        తిరిగి TCL_ERROR;
      }
      సమితి_Sectorsize = 1;
    }లేకపోతే{
      పూర్ణాంకం j;
      Tcl_Obj **apObj;
      Tcl_కొలత nObj;
      ఒకవేళ( Tcl_ListObjGetElements(interp, objv[i+1], &nObj, &apObj) ){
        తిరిగి TCL_ERROR;
      }
      కొరకు(j=0; j<(పూర్ణాంకం)nObj; j++){
        పూర్ణాంకం rc;
        పూర్ణాంకం iChoice;
        Tcl_Obj *pFlag = Tcl_DuplicateObj(apObj[j]);
        Tcl_Incr_Ref_లెక్క(pFlag);
        Tcl_Utf_గమ్యం_క్రింది(Tcl_GetString(pFlag));

        rc = Tcl_Get_సూచీ_From_Obj_Struct(
            interp, pFlag, aFlag, పరిమాణం(aFlag[0]), "no such flag", 0, &iChoice
        );
        Tcl_Decr_Ref_లెక్క(pFlag);
        ఒకవేళ( rc ){
          తిరిగి TCL_ERROR;
        }

        iDc |= aFlag[iChoice].i_విలువ;
      }
      సమితి_Device_Char = 1;
    }
  }

  ఒకవేళ( సమితి_Device_Char ){
    *piDeviceChar = iDc;
  }
  ఒకవేళ( సమితి_Sectorsize ){
    *pi_Sector_కొలత = i_Sector_కొలత;
  }

  తిరిగి TCL_OK;
}

/*
** tclcmd:   sqlite3_crash_now
**
** Simulate a crash immediately. This function does not return
** (writeListSync() calls exit(-1)).
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI crashNowCmd(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  ఒకవేళ( objc!=1 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "");
    తిరిగి TCL_ERROR;
  }
  writeListSync(0, 1);
  ప్రతిజ్ఞా( 0 );
  తిరిగి TCL_OK;
}

/*
** tclcmd:   sqlite_crash_enable ENABLE ?DEFAULT?
**
** Parameter ENABLE must be a boolean value. If true, then the "crash"
** vfs is added to the system. If false, it is removed.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI crashEnableCmd(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  పూర్ణాంకం isEnable;
  పూర్ణాంకం isDefault = 0;
  స్థిర sqlite3_vfs crashVfs = {
    2,                  /* iVersion */
    0,                  /* szOsFile */
    0,                  /* mxPathname */
    0,                  /* pNext */
    "crash",            /* zName */
    0,                  /* pAppData */
 
    cfOpen,               /* xOpen */
    cfDelete,             /* xDelete */
    cfAccess,             /* xAccess */
    cf_నిండిన_Pathname,       /* xFullPathname */
    cfDlOpen,             /* xDlOpen */
    cfDlError,            /* xDlError */
    cfDlSym,              /* xDlSym */
    cfDlClose,            /* xDlClose */
    cfRandomness,         /* xRandomness */
    cfSleep,              /* xSleep */
    cf_Current_సమయం,        /* xCurrentTime */
    cf_Get_చివరి_Error,       /* xGetLastError */
    0,                    /* xCurrentTimeInt64 */
  };

  ఒకవేళ( objc!=2 && objc!=3 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "ENABLE ?DEFAULT?");
    తిరిగి TCL_ERROR;
  }

  ఒకవేళ( Tcl_GetBooleanFromObj(interp, objv[1], &isEnable) ){
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( objc==3 && Tcl_GetBooleanFromObj(interp, objv[2], &isDefault) ){
    తిరిగి TCL_ERROR;
  }

  ఒకవేళ( (isEnable && crashVfs.p_App_దత్తాంశం) || (!isEnable && !crashVfs.p_App_దత్తాంశం) ){
    తిరిగి TCL_OK;
  }

  ఒకవేళ( crashVfs.p_App_దత్తాంశం==0 ){
    sqlite3_vfs *pOriginalVfs = sqlite3_vfs_కనుగొను(0);
    crashVfs.mxPathname = pOriginalVfs->mxPathname;
    crashVfs.p_App_దత్తాంశం = (శూన్య *)pOriginalVfs;
    crashVfs.szOsFile = పరిమాణం(CrashFile) + pOriginalVfs->szOsFile;
    sqlite3_vfs_రిజిస్టరు(&crashVfs, isDefault);
  }లేకపోతే{
    crashVfs.p_App_దత్తాంశం = 0;
    sqlite3_vfs_unregister(&crashVfs);
  }

  తిరిగి TCL_OK;
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
స్థిర పూర్ణాంకం SQLITE_TCLAPI crashParamsObjCmd(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  పూర్ణాంకం iDelay;
  స్థిరరాశి అక్షరం *zCrashFile;
  Tcl_కొలత nCrashFile;
  పూర్ణాంకం iDc, i_Sector_కొలత;

  iDc = -1;
  i_Sector_కొలత = -1;

  ఒకవేళ( objc<3 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "?OPTIONS? DELAY CRASHFILE");
    వెళ్ళు error;
  }

  zCrashFile = Tcl_GetStringFromObj(objv[objc-1], &nCrashFile);
  ఒకవేళ( nCrashFile>=పరిమాణం(g.zCrashFile) ){
    Tcl_Append_ఫలితం(interp, "Filename is too long: \"", zCrashFile, "\"", ఖాళీ);
    వెళ్ళు error;
  }
  ఒకవేళ( Tcl_GetIntFromObj(interp, objv[objc-2], &iDelay) ){
    వెళ్ళు error;
  }

  ఒకవేళ( process_Dev_Sym_వాదనలు(interp, objc-3, &objv[1], &iDc, &i_Sector_కొలత) ){
    తిరిగి TCL_ERROR;
  }

  ఒకవేళ( iDc>=0 ){
    g.iDeviceCharacteristics = iDc;
  }
  ఒకవేళ( i_Sector_కొలత>=0 ){
    g.i_Sector_కొలత = i_Sector_కొలత;
  }

  g.iCrash = iDelay;
  జ్ఞాపకం_నకలు(g.zCrashFile, zCrashFile, nCrashFile+1);
  sqlite3CrashTestEnable = 1;
  తిరిగి TCL_OK;

error:
  తిరిగి TCL_ERROR;
}

స్థిర పూర్ణాంకం SQLITE_TCLAPI devSymObjCmd(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  శూన్య devsym_రిజిస్టరు(పూర్ణాంకం iDeviceChar, పూర్ణాంకం i_Sector_కొలత);

  పూర్ణాంకం iDc = -1;
  పూర్ణాంకం i_Sector_కొలత = -1;

  ఒకవేళ( process_Dev_Sym_వాదనలు(interp, objc-1, &objv[1], &iDc, &i_Sector_కొలత) ){
    తిరిగి TCL_ERROR;
  }
  devsym_రిజిస్టరు(iDc, i_Sector_కొలత);

  తిరిగి TCL_OK;
}

/*
** tclcmd: sqlite3_crash_on_write N
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI writeCrashObjCmd(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  శూన్య devsym_crash_on_write(పూర్ణాంకం);
  పూర్ణాంకం nWrite = 0;

  ఒకవేళ( objc!=2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "NWRITE");
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( Tcl_GetIntFromObj(interp, objv[1], &nWrite) ){
    తిరిగి TCL_ERROR;
  }

  devsym_crash_on_write(nWrite);
  తిరిగి TCL_OK;
}

/*
** tclcmd: unregister_devsim
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI dsUnregisterObjCmd(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  శూన్య devsym_unregister(శూన్య);

  ఒకవేళ( objc!=1 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "");
    తిరిగి TCL_ERROR;
  }

  devsym_unregister();
  తిరిగి TCL_OK;
}

/*
** tclcmd: register_jt_vfs ?-default? PARENT-VFS
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI jtObjCmd(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  పూర్ణాంకం jt_రిజిస్టరు(అక్షరం *, పూర్ణాంకం);
  అక్షరం *z_మాతృ = 0;

  ఒకవేళ( objc!=2 && objc!=3 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "?-default? PARENT-VFS");
    తిరిగి TCL_ERROR;
  }
  z_మాతృ = Tcl_GetString(objv[1]);
  ఒకవేళ( objc==3 ){
    ఒకవేళ( strcmp(z_మాతృ, "-default") ){
      Tcl_Append_ఫలితం(interp,
          "bad option \"", z_మాతృ, "\": must be -default", ఖాళీ
      );
      తిరిగి TCL_ERROR;
    }
    z_మాతృ = Tcl_GetString(objv[2]);
  }

  ఒకవేళ( !(*z_మాతృ) ){
    z_మాతృ = 0;
  }
  ఒకవేళ( jt_రిజిస్టరు(z_మాతృ, objc==3) ){
    Tcl_Append_ఫలితం(interp, "Error in jt_register", ఖాళీ);
    తిరిగి TCL_ERROR;
  }

  తిరిగి TCL_OK;
}

/*
** tclcmd: unregister_jt_vfs
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI jtUnregisterObjCmd(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  శూన్య jt_unregister(శూన్య);

  ఒకవేళ( objc!=1 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "");
    తిరిగి TCL_ERROR;
  }

  jt_unregister();
  తిరిగి TCL_OK;
}

#అయితే_ముగింపు /* SQLITE_OMIT_DISKIO */

/*
** This procedure registers the TCL procedures defined in this file.
*/
పూర్ణాంకం Sqlitetest6_Init(Tcl_Interp *interp){
#నిర్వచితం_కాకపోతే SQLITE_OMIT_DISKIO
  Tcl_CreateObjCommand(interp, "sqlite3_crash_enable", crashEnableCmd, 0, 0);
  Tcl_CreateObjCommand(interp, "sqlite3_crashparams", crashParamsObjCmd, 0, 0);
  Tcl_CreateObjCommand(interp, "sqlite3_crash_now", crashNowCmd, 0, 0);
  Tcl_CreateObjCommand(interp, "sqlite3_simulate_device", devSymObjCmd, 0, 0);
  Tcl_CreateObjCommand(interp, "sqlite3_crash_on_write", writeCrashObjCmd,0,0);
  Tcl_CreateObjCommand(interp, "unregister_devsim", dsUnregisterObjCmd, 0, 0);
  Tcl_CreateObjCommand(interp, "register_jt_vfs", jtObjCmd, 0, 0);
  Tcl_CreateObjCommand(interp, "unregister_jt_vfs", jtUnregisterObjCmd, 0, 0);
#అయితే_ముగింపు
  తిరిగి TCL_OK;
}

#అయితే_ముగింపు /* SQLITE_TEST */
