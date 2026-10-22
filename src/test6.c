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
#ಆದರೆ SQLITE_TEST          /* This file is used for testing only */
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"
#ಅನ್ತರ್ಭಾವ "tclsqlite.h"

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_DISKIO  /* This file is a no-op if disk I/O is disabled */

/* #define TRACE_CRASHTEST */

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ CrashFile CrashFile;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ CrashGlobal CrashGlobal;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Write_ಬಫರ್ Write_ಬಫರ್;

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
ರಚನೆ Write_ಬಫರ್ {
  i64 i_ಸ್ಥಾನಾಂತರ;                 /* Byte offset of the start of this write() */
  ಪೂರ್ಣಾಂಕ nBuf;                    /* Number of bytes written */
  u8 *zBuf;                    /* Pointer to copy of written data */
  CrashFile *pFile;            /* File this write() applies to */

  Write_ಬಫರ್ *p_ಮುಂದಿನ;          /* Next in CrashGlobal.pWriteList */
};

ರಚನೆ CrashFile {
  ಸ್ಥಿರಾಂಕ sqlite3_io_methods *p_ವಿಧಾನ;   /* Must be first */
  sqlite3_file *pRealFile;             /* Underlying "real" file handle */
  ಅಕ್ಷರ *zName;
  ಪೂರ್ಣಾಂಕ flags;                           /* Flags the file was opened with */

  /* Cache of the entire file. This is used to speed up OsRead() and
  ** OsFileSize() calls. Although both could be done by traversing the
  ** write-list, in practice this is impractically slow.
  */
  u8 *z_ದತ್ತಾಂಶ;                           /* Buffer containing file contents */
  ಪೂರ್ಣಾಂಕ n_ದತ್ತಾಂಶ;                           /* Size of buffer allocated at zData */
  i64 i_ಗಾತ್ರ;                           /* Size of file in bytes */
};

ರಚನೆ CrashGlobal {
  Write_ಬಫರ್ *pWriteList;     /* Head of write-list */
  Write_ಬಫರ್ *p_Write_List_ಕೊನೆ;  /* End of write-list */

  ಪೂರ್ಣಾಂಕ i_Sector_ಗಾತ್ರ;             /* Value of simulated sector size */
  ಪೂರ್ಣಾಂಕ iDeviceCharacteristics;  /* Value of simulated device characteristics */

  ಪೂರ್ಣಾಂಕ iCrash;                  /* Crash on the iCrash'th call to xSync() */
  ಅಕ್ಷರ zCrashFile[500];        /* Crash during an xSync() on this file */
};

ಸ್ಥಿರ CrashGlobal g = {0, 0, SQLITE_DEFAULT_SECTOR_ಗಾತ್ರ, 0, 0};

/*
** Set this global variable to 1 to enable crash testing.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ sqlite3CrashTestEnable = 0;

ಸ್ಥಿರ ಶೂನ್ಯ *crash_malloc(ಪೂರ್ಣಾಂಕ nByte){
  ಹಿಂತಿರುಗಿ (ಶೂನ್ಯ *)Tcl_AttemptAlloc((size_t)nByte);
}
ಸ್ಥಿರ ಶೂನ್ಯ crash_free(ಶೂನ್ಯ *p){
  Tcl_Free(p);
}
ಸ್ಥಿರ ಶೂನ್ಯ *crash_realloc(ಶೂನ್ಯ *p, ಪೂರ್ಣಾಂಕ n){
  ಹಿಂತಿರುಗಿ (ಶೂನ್ಯ *)Tcl_AttemptRealloc(p, (size_t)n);
}

/*
** Wrapper around the sqlite3OsWrite() function that avoids writing to the
** 512 byte block beginning at offset PENDING_BYTE.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ writeDbFile(CrashFile *p, u8 *z, i64 iAmt, i64 iOff){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  ಪೂರ್ಣಾಂಕ iSkip = 0;
  ಆದರೆ( (iAmt-iSkip)>0 ){
    rc = sqlite3OsWrite(p->pRealFile, &z[iSkip], (ಪೂರ್ಣಾಂಕ)(iAmt-iSkip), iOff+iSkip);
  }
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Flush the write-list as if xSync() had been called on file handle
** pFile. If isCrash is true, simulate a crash.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ writeListSync(CrashFile *pFile, ಪೂರ್ಣಾಂಕ isCrash){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  ಪೂರ್ಣಾಂಕ iDc = g.iDeviceCharacteristics;

  Write_ಬಫರ್ *pWrite;
  Write_ಬಫರ್ **ppPtr;

  /* If this is not a crash simulation, set pFinal to point to the
  ** last element of the write-list that is associated with file handle
  ** pFile.
  **
  ** If this is a crash simulation, set pFinal to an arbitrarily selected
  ** element of the write-list.
  */
  Write_ಬಫರ್ *pFinal = 0;
  ಆದರೆ( !isCrash ){
    ಗಾಗಿ(pWrite=g.pWriteList; pWrite; pWrite=pWrite->p_ಮುಂದಿನ){
      ಆದರೆ( pWrite->pFile==pFile ){
        pFinal = pWrite;
      }
    }
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( iDc&(SQLITE_IOCAP_SEQUENTIAL|SQLITE_IOCAP_ಸುರಕ್ಷಿತ_APPEND) ){
    ಪೂರ್ಣಾಂಕ nWrite = 0;
    ಪೂರ್ಣಾಂಕ iFinal;
    ಗಾಗಿ(pWrite=g.pWriteList; pWrite; pWrite=pWrite->p_ಮುಂದಿನ) nWrite++;
    sqlite3_randomness(ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ), &iFinal);
    iFinal = ((iFinal<0)?-1*iFinal:iFinal)%nWrite;
    ಗಾಗಿ(pWrite=g.pWriteList; iFinal>0; pWrite=pWrite->p_ಮುಂದಿನ) iFinal--;
    pFinal = pWrite;
  }

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ TRACE_CRASHTEST
  ಆದರೆ( pFile ){
    ಬರೆ("Sync %s (is %s crash)\n", pFile->zName, (isCrash?"a":"not a"));
  }
#ಆದರೆ_ಅಂತ್ಯ

  ppPtr = &g.pWriteList;
  ಗಾಗಿ(pWrite=*ppPtr; rc==SQLITE_OK && pWrite; pWrite=*ppPtr){
    sqlite3_file *pRealFile = pWrite->pFile->pRealFile;

    /* (eAction==1)      -> write block out normally,
    ** (eAction==2)      -> do nothing,
    ** (eAction==3)      -> trash sectors.
    */
    ಪೂರ್ಣಾಂಕ eAction = 0;
    ಆದರೆ( !isCrash ){
      eAction = 2;
      ಆದರೆ( (pWrite->pFile==pFile || iDc&SQLITE_IOCAP_SEQUENTIAL) ){
        eAction = 1;
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಅಕ್ಷರ random;
      sqlite3_randomness(1, &random);

      /* Do not select option 3 (sector trashing) if the IOCAP_ATOMIC flag
      ** is set or this is an OsTruncate(), not an Oswrite().
      */
      ಆದರೆ( (iDc&SQLITE_IOCAP_ATOMIC) || (pWrite->zBuf==0) ){
        random &= 0x01;
      }

      /* If IOCAP_SEQUENTIAL is set and this is not the final entry
      ** in the truncated write-list, always select option 1 (write
      ** out correctly).
      */
      ಆದರೆ( (iDc&SQLITE_IOCAP_SEQUENTIAL && pWrite!=pFinal) ){
        random = 0;
      }

      /* If IOCAP_SAFE_APPEND is set and this OsWrite() operation is
      ** an append (first byte of the written region is 1 byte past the
      ** current EOF), always select option 1 (write out correctly).
      */
      ಆದರೆ( iDc&SQLITE_IOCAP_ಸುರಕ್ಷಿತ_APPEND && pWrite->zBuf ){
        i64 i_ಗಾತ್ರ;
        sqlite3_Os_File_ಗಾತ್ರ(pRealFile, &i_ಗಾತ್ರ);
        ಆದರೆ( i_ಗಾತ್ರ==pWrite->i_ಸ್ಥಾನಾಂತರ ){
          random = 0;
        }
      }

      ಆದರೆ( (random&0x06)==0x06 ){
        eAction = 3;
      }ಇಲ್ಲದಿದ್ದರೆ{
        eAction = ((random&0x01)?2:1);
      }
    }

    ಆಯ್ಕೆ( eAction ){
      ಸ್ಥಿತಿ 1: {               /* Write out correctly */
        ಆದರೆ( pWrite->zBuf ){
          rc = writeDbFile(
              pWrite->pFile, pWrite->zBuf, pWrite->nBuf, pWrite->i_ಸ್ಥಾನಾಂತರ
          );
        }ಇಲ್ಲದಿದ್ದರೆ{
          rc = sqlite3OsTruncate(pRealFile, pWrite->i_ಸ್ಥಾನಾಂತರ);
        }
        *ppPtr = pWrite->p_ಮುಂದಿನ;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ TRACE_CRASHTEST
        ಆದರೆ( isCrash ){
          ಬರೆ("Writing %d bytes @ %d (%s)\n",
            pWrite->nBuf, (ಪೂರ್ಣಾಂಕ)pWrite->i_ಸ್ಥಾನಾಂತರ, pWrite->pFile->zName
          );
        }
#ಆದರೆ_ಅಂತ್ಯ
        crash_free(pWrite);
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ 2: {               /* Do nothing */
        ppPtr = &pWrite->p_ಮುಂದಿನ;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ TRACE_CRASHTEST
        ಆದರೆ( isCrash ){
          ಬರೆ("Omiting %d bytes @ %d (%s)\n",
            pWrite->nBuf, (ಪೂರ್ಣಾಂಕ)pWrite->i_ಸ್ಥಾನಾಂತರ, pWrite->pFile->zName
          );
        }
#ಆದರೆ_ಅಂತ್ಯ
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ 3: {               /* Trash sectors */
        u8 *zGarbage;
        ಪೂರ್ಣಾಂಕ i_ಮೊದಲ = (ಪೂರ್ಣಾಂಕ)(pWrite->i_ಸ್ಥಾನಾಂತರ/g.i_Sector_ಗಾತ್ರ);
        ಪೂರ್ಣಾಂಕ i_ಕೊನೆಯ = (ಪೂರ್ಣಾಂಕ)((pWrite->i_ಸ್ಥಾನಾಂತರ+pWrite->nBuf-1)/g.i_Sector_ಗಾತ್ರ);

        ಪ್ರತಿಜ್ಞಾ(pWrite->zBuf);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ TRACE_CRASHTEST
        ಬರೆ("Trashing %d sectors (%d bytes) @ %lld (sector %d) (%s)\n",
            1+i_ಕೊನೆಯ-i_ಮೊದಲ, (1+i_ಕೊನೆಯ-i_ಮೊದಲ)*g.i_Sector_ಗಾತ್ರ,
            pWrite->i_ಸ್ಥಾನಾಂತರ, i_ಮೊದಲ, pWrite->pFile->zName
        );
#ಆದರೆ_ಅಂತ್ಯ

        zGarbage = crash_malloc(g.i_Sector_ಗಾತ್ರ);
        ಆದರೆ( zGarbage ){
          sqlite3_int64 i;
          ಗಾಗಿ(i=i_ಮೊದಲ; rc==SQLITE_OK && i<=i_ಕೊನೆಯ; i++){
            sqlite3_randomness(g.i_Sector_ಗಾತ್ರ, zGarbage);
            rc = writeDbFile(
              pWrite->pFile, zGarbage, g.i_Sector_ಗಾತ್ರ, i*g.i_Sector_ಗಾತ್ರ
            );
          }
          crash_free(zGarbage);
        }ಇಲ್ಲದಿದ್ದರೆ{
          rc = SQLITE_NOMEM;
        }

        ppPtr = &pWrite->p_ಮುಂದಿನ;
        ನಿಲ್ಲಿಸು;
      }

      ಪೂರ್ವನಿಯೋಜಿತ:
        ಪ್ರತಿಜ್ಞಾ(!"Cannot happen");
    }

    ಆದರೆ( pWrite==pFinal ) ನಿಲ್ಲಿಸು;
  }

  ಆದರೆ( rc==SQLITE_OK && isCrash ){
    ನಿರ್ಗಮಿಸು(-1);
  }

  ಗಾಗಿ(pWrite=g.pWriteList; pWrite && pWrite->p_ಮುಂದಿನ; pWrite=pWrite->p_ಮುಂದಿನ);
  g.p_Write_List_ಕೊನೆ = pWrite;

  ಹಿಂತಿರುಗಿ rc;
}

/*
** Add an entry to the end of the write-list.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ writeListAppend(
  sqlite3_file *pFile,
  sqlite3_int64 i_ಸ್ಥಾನಾಂತರ,
  ಸ್ಥಿರಾಂಕ u8 *zBuf,
  ಪೂರ್ಣಾಂಕ nBuf
){
  Write_ಬಫರ್ *pNew;

  ಪ್ರತಿಜ್ಞಾ((zBuf && nBuf) || (!nBuf && !zBuf));

  pNew = (Write_ಬಫರ್ *)crash_malloc(ಗಾತ್ರ(Write_ಬಫರ್) + nBuf);
  ಆದರೆ( pNew==0 ){
    fprintf(stderr, "out of memory in the crash simulator\n");
  }
  memset(pNew, 0, ಗಾತ್ರ(Write_ಬಫರ್)+nBuf);
  pNew->i_ಸ್ಥಾನಾಂತರ = i_ಸ್ಥಾನಾಂತರ;
  pNew->nBuf = nBuf;
  pNew->pFile = (CrashFile *)pFile;
  ಆದರೆ( zBuf ){
    pNew->zBuf = (u8 *)&pNew[1];
    ಸ್ಮರಣೆ_ನಕಲು(pNew->zBuf, zBuf, nBuf);
  }

  ಆದರೆ( g.pWriteList ){
    ಪ್ರತಿಜ್ಞಾ(g.p_Write_List_ಕೊನೆ);
    g.p_Write_List_ಕೊನೆ->p_ಮುಂದಿನ = pNew;
  }ಇಲ್ಲದಿದ್ದರೆ{
    g.pWriteList = pNew;
  }
  g.p_Write_List_ಕೊನೆ = pNew;
 
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Close a crash-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ cfClose(sqlite3_file *pFile){
  CrashFile *pCrash = (CrashFile *)pFile;
  writeListSync(pCrash, 0);
  sqlite3OsClose(pCrash->pRealFile);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Read data from a crash-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ cfRead(
  sqlite3_file *pFile,
  ಶೂನ್ಯ *zBuf,
  ಪೂರ್ಣಾಂಕ iAmt,
  sqlite_int64 iOfst
){
  CrashFile *pCrash = (CrashFile *)pFile;
  ಪೂರ್ಣಾಂಕ nCopy = (ಪೂರ್ಣಾಂಕ)MIN((i64)iAmt, (pCrash->i_ಗಾತ್ರ - iOfst));

  ಆದರೆ( nCopy>0 ){
    ಸ್ಮರಣೆ_ನಕಲು(zBuf, &pCrash->z_ದತ್ತಾಂಶ[iOfst], nCopy);
  }

  /* Check the file-size to see if this is a short-read */
  ಆದರೆ( nCopy<iAmt ){
    ಹಿಂತಿರುಗಿ SQLITE_IOERR_SHORT_READ;
  }

  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Write data to a crash-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ cfWrite(
  sqlite3_file *pFile,
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *zBuf,
  ಪೂರ್ಣಾಂಕ iAmt,
  sqlite_int64 iOfst
){
  CrashFile *pCrash = (CrashFile *)pFile;
  ಆದರೆ( iAmt+iOfst>pCrash->i_ಗಾತ್ರ ){
    pCrash->i_ಗಾತ್ರ = (ಪೂರ್ಣಾಂಕ)(iAmt+iOfst);
  }
  ಆಗ( pCrash->i_ಗಾತ್ರ>pCrash->n_ದತ್ತಾಂಶ ){
    u8 *zNew;
    ಪೂರ್ಣಾಂಕ nNew = (pCrash->n_ದತ್ತಾಂಶ*2) + 4096;
    zNew = crash_realloc(pCrash->z_ದತ್ತಾಂಶ, nNew);
    ಆದರೆ( !zNew ){
      ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
    }
    memset(&zNew[pCrash->n_ದತ್ತಾಂಶ], 0, nNew-pCrash->n_ದತ್ತಾಂಶ);
    pCrash->n_ದತ್ತಾಂಶ = nNew;
    pCrash->z_ದತ್ತಾಂಶ = zNew;
  }
  ಸ್ಮರಣೆ_ನಕಲು(&pCrash->z_ದತ್ತಾಂಶ[iOfst], zBuf, iAmt);
  ಹಿಂತಿರುಗಿ writeListAppend(pFile, iOfst, zBuf, iAmt);
}

/*
** Truncate a crash-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ cfTruncate(sqlite3_file *pFile, sqlite_int64 ಗಾತ್ರ_2){
  CrashFile *pCrash = (CrashFile *)pFile;
  ಪ್ರತಿಜ್ಞಾ(ಗಾತ್ರ_2>=0);
  ಆದರೆ( pCrash->i_ಗಾತ್ರ>ಗಾತ್ರ_2 ){
    pCrash->i_ಗಾತ್ರ = (ಪೂರ್ಣಾಂಕ)ಗಾತ್ರ_2;
  }
  ಹಿಂತಿರುಗಿ writeListAppend(pFile, ಗಾತ್ರ_2, 0, 0);
}

/*
** Sync a crash-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ cfSync(sqlite3_file *pFile, ಪೂರ್ಣಾಂಕ flags){
  CrashFile *pCrash = (CrashFile *)pFile;
  ಪೂರ್ಣಾಂಕ isCrash = 0;

  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName = pCrash->zName;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zCrashFile = g.zCrashFile;
  ಪೂರ್ಣಾಂಕ nName = (ಪೂರ್ಣಾಂಕ)ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(zName);
  ಪೂರ್ಣಾಂಕ nCrashFile = (ಪೂರ್ಣಾಂಕ)ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(zCrashFile);

  ಆದರೆ( nCrashFile>0 && zCrashFile[nCrashFile-1]=='*' ){
    nCrashFile--;
    ಆದರೆ( nName>nCrashFile ) nName = nCrashFile;
  }

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ TRACE_CRASHTEST
  ಬರೆ("cfSync(): nName = %d, nCrashFile = %d, zName = %s, zCrashFile = %s\n",
         nName, nCrashFile, zName, zCrashFile);
#ಆದರೆ_ಅಂತ್ಯ

  ಆದರೆ( nName==nCrashFile && 0==memcmp(zName, zCrashFile, nName) ){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ TRACE_CRASHTEST
    ಬರೆ("cfSync(): name matched, g.iCrash = %d\n", g.iCrash);
#ಆದರೆ_ಅಂತ್ಯ
    ಆದರೆ( (--g.iCrash)==0 ) isCrash = 1;
  }

  ಹಿಂತಿರುಗಿ writeListSync(pCrash, isCrash);
}

/*
** Return the current file-size of the crash-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ cf_File_ಗಾತ್ರ(sqlite3_file *pFile, sqlite_int64 *p_ಗಾತ್ರ){
  CrashFile *pCrash = (CrashFile *)pFile;
  *p_ಗಾತ್ರ = (i64)pCrash->i_ಗಾತ್ರ;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Calls related to file-locks are passed on to the real file handle.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ cf_ಬೀಗ(sqlite3_file *pFile, ಪೂರ್ಣಾಂಕ e_ಬೀಗ){
  ಹಿಂತಿರುಗಿ sqlite3_Os_ಬೀಗ(((CrashFile *)pFile)->pRealFile, e_ಬೀಗ);
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ cfUnlock(sqlite3_file *pFile, ಪೂರ್ಣಾಂಕ e_ಬೀಗ){
  ಹಿಂತಿರುಗಿ sqlite3OsUnlock(((CrashFile *)pFile)->pRealFile, e_ಬೀಗ);
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ cf_Check_Reserved_ಬೀಗ(sqlite3_file *pFile, ಪೂರ್ಣಾಂಕ *p_Res_ಹೊರಹೋಗು){
  ಹಿಂತಿರುಗಿ sqlite3_Os_Check_Reserved_ಬೀಗ(((CrashFile *)pFile)->pRealFile, p_Res_ಹೊರಹೋಗು);
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ cfFileControl(sqlite3_file *pFile, ಪೂರ್ಣಾಂಕ op, ಶೂನ್ಯ *pArg){
  ಆದರೆ( op==SQLITE_FCNTL_ಗಾತ್ರ_HINT ){
    CrashFile *pCrash = (CrashFile *)pFile;
    i64 nByte = *(i64 *)pArg;
    ಆದರೆ( nByte>pCrash->i_ಗಾತ್ರ ){
      ಆದರೆ( SQLITE_OK==writeListAppend(pFile, nByte, 0, 0) ){
        pCrash->i_ಗಾತ್ರ = (ಪೂರ್ಣಾಂಕ)nByte;
      }
    }
    ಹಿಂತಿರುಗಿ SQLITE_OK;
  }
  ಹಿಂತಿರುಗಿ sqlite3OsFileControl(((CrashFile *)pFile)->pRealFile, op, pArg);
}

/*
** The xSectorSize() and xDeviceCharacteristics() functions return
** the global values configured by the [sqlite_crashparams] tcl
*  interface.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ cf_Sector_ಗಾತ್ರ(sqlite3_file *pFile){
  ಹಿಂತಿರುಗಿ g.i_Sector_ಗಾತ್ರ;
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ cfDeviceCharacteristics(sqlite3_file *pFile){
  ಹಿಂತಿರುಗಿ g.iDeviceCharacteristics;
}

/*
** Pass-throughs for WAL support.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ cf_Shm_ಬೀಗ(sqlite3_file *pFile, ಪೂರ್ಣಾಂಕ ofst, ಪೂರ್ಣಾಂಕ n, ಪೂರ್ಣಾಂಕ flags){
  sqlite3_file *pReal = ((CrashFile*)pFile)->pRealFile;
  ಹಿಂತಿರುಗಿ pReal->pMethods->x_Shm_ಬೀಗ(pReal, ofst, n, flags);
}
ಸ್ಥಿರ ಶೂನ್ಯ cfShmBarrier(sqlite3_file *pFile){
  sqlite3_file *pReal = ((CrashFile*)pFile)->pRealFile;
  pReal->pMethods->xShmBarrier(pReal);
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ cfShmUnmap(sqlite3_file *pFile, ಪೂರ್ಣಾಂಕ delFlag){
  sqlite3_file *pReal = ((CrashFile*)pFile)->pRealFile;
  ಹಿಂತಿರುಗಿ pReal->pMethods->xShmUnmap(pReal, delFlag);
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ cfShmMap(
  sqlite3_file *pFile,            /* Handle open on database file */
  ಪೂರ್ಣಾಂಕ iRegion,                    /* Region to retrieve */
  ಪೂರ್ಣಾಂಕ sz,                         /* Size of regions */
  ಪೂರ್ಣಾಂಕ w,                          /* True to extend file if necessary */
  ಶೂನ್ಯ ಅಸ್ಥಿರ **pp              /* OUT: Mapped memory */
){
  sqlite3_file *pReal = ((CrashFile*)pFile)->pRealFile;
  ಹಿಂತಿರುಗಿ pReal->pMethods->xShmMap(pReal, iRegion, sz, w, pp);
}

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ sqlite3_io_methods CrashFileVtab = {
  2,                            /* iVersion */
  cfClose,                      /* xClose */
  cfRead,                       /* xRead */
  cfWrite,                      /* xWrite */
  cfTruncate,                   /* xTruncate */
  cfSync,                       /* xSync */
  cf_File_ಗಾತ್ರ,                   /* xFileSize */
  cf_ಬೀಗ,                       /* xLock */
  cfUnlock,                     /* xUnlock */
  cf_Check_Reserved_ಬೀಗ,          /* xCheckReservedLock */
  cfFileControl,                /* xFileControl */
  cf_Sector_ಗಾತ್ರ,                 /* xSectorSize */
  cfDeviceCharacteristics,      /* xDeviceCharacteristics */
  cfShmMap,                     /* xShmMap */
  cf_Shm_ಬೀಗ,                    /* xShmLock */
  cfShmBarrier,                 /* xShmBarrier */
  cfShmUnmap                    /* xShmUnmap */
};

/*
** Application data for the crash VFS
*/
ರಚನೆ crash_App_ದತ್ತಾಂಶ {
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ cfOpen(
  sqlite3_vfs *pCfVfs,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName,
  sqlite3_file *pFile,
  ಪೂರ್ಣಾಂಕ flags,
  ಪೂರ್ಣಾಂಕ *p_ಹೊರಹೋಗು_Flags
){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_ದತ್ತಾಂಶ;
  ಪೂರ್ಣಾಂಕ rc;
  CrashFile *pWrapper = (CrashFile *)pFile;
  sqlite3_file *pReal = (sqlite3_file*)&pWrapper[1];

  memset(pWrapper, 0, ಗಾತ್ರ(CrashFile));
  rc = sqlite3OsOpen(pVfs, zName, pReal, flags, p_ಹೊರಹೋಗು_Flags);

  ಆದರೆ( rc==SQLITE_OK ){
    i64 i_ಗಾತ್ರ;
    pWrapper->p_ವಿಧಾನ = &CrashFileVtab;
    pWrapper->zName = (ಅಕ್ಷರ *)zName;
    pWrapper->pRealFile = pReal;
    rc = sqlite3_Os_File_ಗಾತ್ರ(pReal, &i_ಗಾತ್ರ);
    pWrapper->i_ಗಾತ್ರ = (ಪೂರ್ಣಾಂಕ)i_ಗಾತ್ರ;
    pWrapper->flags = flags;
  }
  ಆದರೆ( rc==SQLITE_OK ){
    pWrapper->n_ದತ್ತಾಂಶ = (ಪೂರ್ಣಾಂಕ)(4096 + pWrapper->i_ಗಾತ್ರ);
    pWrapper->z_ದತ್ತಾಂಶ = crash_malloc(pWrapper->n_ದತ್ತಾಂಶ);
    ಆದರೆ( pWrapper->z_ದತ್ತಾಂಶ ){
      /* os_unix.c contains an assert() that fails if the caller attempts
      ** to read data from the 512-byte locking region of a file opened
      ** with the SQLITE_OPEN_MAIN_DB flag. This region of a database file
      ** never contains valid data anyhow. So avoid doing such a read here.
      **
      ** UPDATE: It also contains an assert() verifying that each call
      ** to the xRead() method reads less than 128KB of data.
      */
      i64 iOff;

      memset(pWrapper->z_ದತ್ತಾಂಶ, 0, pWrapper->n_ದತ್ತಾಂಶ);
      ಗಾಗಿ(iOff=0; iOff<pWrapper->i_ಗಾತ್ರ; iOff += 512){
        ಪೂರ್ಣಾಂಕ nRead = (ಪೂರ್ಣಾಂಕ)(pWrapper->i_ಗಾತ್ರ - iOff);
        ಆದರೆ( nRead>512 ) nRead = 512;
        rc = sqlite3OsRead(pReal, &pWrapper->z_ದತ್ತಾಂಶ[iOff], nRead, iOff);
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      rc = SQLITE_NOMEM;
    }
  }
  ಆದರೆ( rc!=SQLITE_OK && pWrapper->p_ವಿಧಾನ ){
    sqlite3OsClose(pFile);
  }
  ಹಿಂತಿರುಗಿ rc;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ cfDelete(sqlite3_vfs *pCfVfs, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮಾರ್ಗ, ಪೂರ್ಣಾಂಕ dirSync){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_ದತ್ತಾಂಶ;
  ಹಿಂತಿರುಗಿ pVfs->xDelete(pVfs, z_ಮಾರ್ಗ, dirSync);
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ cfAccess(
  sqlite3_vfs *pCfVfs,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮಾರ್ಗ,
  ಪೂರ್ಣಾಂಕ flags,
  ಪೂರ್ಣಾಂಕ *p_Res_ಹೊರಹೋಗು
){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_ದತ್ತಾಂಶ;
  ಹಿಂತಿರುಗಿ pVfs->xAccess(pVfs, z_ಮಾರ್ಗ, flags, p_Res_ಹೊರಹೋಗು);
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ cf_ತುಂಬಿದ_Pathname(
  sqlite3_vfs *pCfVfs,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮಾರ್ಗ,
  ಪೂರ್ಣಾಂಕ n_ಮಾರ್ಗ_ಹೊರಹೋಗು,
  ಅಕ್ಷರ *z_ಮಾರ್ಗ_ಹೊರಹೋಗು
){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_ದತ್ತಾಂಶ;
  ಹಿಂತಿರುಗಿ pVfs->x_ತುಂಬಿದ_Pathname(pVfs, z_ಮಾರ್ಗ, n_ಮಾರ್ಗ_ಹೊರಹೋಗು, z_ಮಾರ್ಗ_ಹೊರಹೋಗು);
}
ಸ್ಥಿರ ಶೂನ್ಯ *cfDlOpen(sqlite3_vfs *pCfVfs, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮಾರ್ಗ){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_ದತ್ತಾಂಶ;
  ಹಿಂತಿರುಗಿ pVfs->xDlOpen(pVfs, z_ಮಾರ್ಗ);
}
ಸ್ಥಿರ ಶೂನ್ಯ cfDlError(sqlite3_vfs *pCfVfs, ಪೂರ್ಣಾಂಕ nByte, ಅಕ್ಷರ *z_Err_ಸಂದೇಶ){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_ದತ್ತಾಂಶ;
  pVfs->xDlError(pVfs, nByte, z_Err_ಸಂದೇಶ);
}
ಸ್ಥಿರ ಶೂನ್ಯ (*cfDlSym(sqlite3_vfs *pCfVfs, ಶೂನ್ಯ *pH, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSym))(ಶೂನ್ಯ){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_ದತ್ತಾಂಶ;
  ಹಿಂತಿರುಗಿ pVfs->xDlSym(pVfs, pH, zSym);
}
ಸ್ಥಿರ ಶೂನ್ಯ cfDlClose(sqlite3_vfs *pCfVfs, ಶೂನ್ಯ *pHandle){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_ದತ್ತಾಂಶ;
  pVfs->xDlClose(pVfs, pHandle);
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ cfRandomness(sqlite3_vfs *pCfVfs, ಪೂರ್ಣಾಂಕ nByte, ಅಕ್ಷರ *z_Buf_ಹೊರಹೋಗು){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_ದತ್ತಾಂಶ;
  ಹಿಂತಿರುಗಿ pVfs->xRandomness(pVfs, nByte, z_Buf_ಹೊರಹೋಗು);
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ cfSleep(sqlite3_vfs *pCfVfs, ಪೂರ್ಣಾಂಕ nMicro){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_ದತ್ತಾಂಶ;
  ಹಿಂತಿರುಗಿ pVfs->xSleep(pVfs, nMicro);
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ cf_Current_ಸಮಯ(sqlite3_vfs *pCfVfs, ದ್ವಿಗುಣ *p_ಸಮಯ_ಹೊರಹೋಗು){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_ದತ್ತಾಂಶ;
  ಹಿಂತಿರುಗಿ pVfs->x_Current_ಸಮಯ(pVfs, p_ಸಮಯ_ಹೊರಹೋಗು);
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ cf_Get_ಕೊನೆಯ_Error(sqlite3_vfs *pCfVfs, ಪೂರ್ಣಾಂಕ n, ಅಕ್ಷರ *z){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_ದತ್ತಾಂಶ;
  ಹಿಂತಿರುಗಿ pVfs->x_Get_ಕೊನೆಯ_Error(pVfs, n, z);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ process_Dev_Sym_ವಾದಗಳು(
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[],
  ಪೂರ್ಣಾಂಕ *piDeviceChar,
  ಪೂರ್ಣಾಂಕ *pi_Sector_ಗಾತ್ರ
){
  ರಚನೆ DeviceFlag {
    ಅಕ್ಷರ *zName;
    ಪೂರ್ಣಾಂಕ i_ಮೌಲ್ಯ;
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
    { "safe_append",         SQLITE_IOCAP_ಸುರಕ್ಷಿತ_APPEND           },
    { "powersafe_overwrite", SQLITE_IOCAP_POWERSAFE_OVERWRITE   },
    { "batch-atomic",        SQLITE_IOCAP_BATCH_ATOMIC          },
    { 0, 0 }
  };

  ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ iDc = 0;
  ಪೂರ್ಣಾಂಕ i_Sector_ಗಾತ್ರ = 0;
  ಪೂರ್ಣಾಂಕ ಗಣ_Sectorsize = 0;
  ಪೂರ್ಣಾಂಕ ಗಣ_Device_Char = 0;

  ಗಾಗಿ(i=0; i<objc; i+=2){
    Tcl_ಗಾತ್ರ nOpt;
    ಅಕ್ಷರ *zOpt = Tcl_GetStringFromObj(objv[i], &nOpt);

    ಆದರೆ( (nOpt>11 || nOpt<2 || ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ("-sectorsize", zOpt, nOpt))
     && (nOpt>16 || nOpt<2 || ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ("-characteristics", zOpt, nOpt))
    ){
      Tcl_Append_ಫಲಿತಾಂಶ(interp,
        "Bad option: \"", zOpt,
        "\" - must be \"-characteristics\" or \"-sectorsize\"", ಖಾಲಿ
      );
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }
    ಆದರೆ( i==objc-1 ){
      Tcl_Append_ಫಲಿತಾಂಶ(interp, "Option requires an argument: \"", zOpt, "\"", ಖಾಲಿ);
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }

    ಆದರೆ( zOpt[1]=='s' ){
      ಆದರೆ( Tcl_GetIntFromObj(interp, objv[i+1], &i_Sector_ಗಾತ್ರ) ){
        ಹಿಂತಿರುಗಿ TCL_ERROR;
      }
      ಗಣ_Sectorsize = 1;
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಪೂರ್ಣಾಂಕ j;
      Tcl_Obj **apObj;
      Tcl_ಗಾತ್ರ nObj;
      ಆದರೆ( Tcl_ListObjGetElements(interp, objv[i+1], &nObj, &apObj) ){
        ಹಿಂತಿರುಗಿ TCL_ERROR;
      }
      ಗಾಗಿ(j=0; j<(ಪೂರ್ಣಾಂಕ)nObj; j++){
        ಪೂರ್ಣಾಂಕ rc;
        ಪೂರ್ಣಾಂಕ iChoice;
        Tcl_Obj *pFlag = Tcl_DuplicateObj(apObj[j]);
        Tcl_Incr_Ref_ಎಣಿಕೆ(pFlag);
        Tcl_Utf_ಗಮ್ಯ_ಅಧೋ(Tcl_GetString(pFlag));

        rc = Tcl_Get_ಸೂಚ್ಯಂಕ_From_Obj_Struct(
            interp, pFlag, aFlag, ಗಾತ್ರ(aFlag[0]), "no such flag", 0, &iChoice
        );
        Tcl_Decr_Ref_ಎಣಿಕೆ(pFlag);
        ಆದರೆ( rc ){
          ಹಿಂತಿರುಗಿ TCL_ERROR;
        }

        iDc |= aFlag[iChoice].i_ಮೌಲ್ಯ;
      }
      ಗಣ_Device_Char = 1;
    }
  }

  ಆದರೆ( ಗಣ_Device_Char ){
    *piDeviceChar = iDc;
  }
  ಆದರೆ( ಗಣ_Sectorsize ){
    *pi_Sector_ಗಾತ್ರ = i_Sector_ಗಾತ್ರ;
  }

  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** tclcmd:   sqlite3_crash_now
**
** Simulate a crash immediately. This function does not return
** (writeListSync() calls exit(-1)).
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI crashNowCmd(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಆದರೆ( objc!=1 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  writeListSync(0, 1);
  ಪ್ರತಿಜ್ಞಾ( 0 );
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** tclcmd:   sqlite_crash_enable ENABLE ?DEFAULT?
**
** Parameter ENABLE must be a boolean value. If true, then the "crash"
** vfs is added to the system. If false, it is removed.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI crashEnableCmd(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಂಕ isEnable;
  ಪೂರ್ಣಾಂಕ isDefault = 0;
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
    cf_ತುಂಬಿದ_Pathname,       /* xFullPathname */
    cfDlOpen,             /* xDlOpen */
    cfDlError,            /* xDlError */
    cfDlSym,              /* xDlSym */
    cfDlClose,            /* xDlClose */
    cfRandomness,         /* xRandomness */
    cfSleep,              /* xSleep */
    cf_Current_ಸಮಯ,        /* xCurrentTime */
    cf_Get_ಕೊನೆಯ_Error,       /* xGetLastError */
    0,                    /* xCurrentTimeInt64 */
  };

  ಆದರೆ( objc!=2 && objc!=3 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "ENABLE ?DEFAULT?");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  ಆದರೆ( Tcl_GetBooleanFromObj(interp, objv[1], &isEnable) ){
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( objc==3 && Tcl_GetBooleanFromObj(interp, objv[2], &isDefault) ){
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  ಆದರೆ( (isEnable && crashVfs.p_App_ದತ್ತಾಂಶ) || (!isEnable && !crashVfs.p_App_ದತ್ತಾಂಶ) ){
    ಹಿಂತಿರುಗಿ TCL_OK;
  }

  ಆದರೆ( crashVfs.p_App_ದತ್ತಾಂಶ==0 ){
    sqlite3_vfs *pOriginalVfs = sqlite3_vfs_ಕಂಡುಹಿಡಿ(0);
    crashVfs.mxPathname = pOriginalVfs->mxPathname;
    crashVfs.p_App_ದತ್ತಾಂಶ = (ಶೂನ್ಯ *)pOriginalVfs;
    crashVfs.szOsFile = ಗಾತ್ರ(CrashFile) + pOriginalVfs->szOsFile;
    sqlite3_vfs_ರಿಜಿಸ್ಟರ್(&crashVfs, isDefault);
  }ಇಲ್ಲದಿದ್ದರೆ{
    crashVfs.p_App_ದತ್ತಾಂಶ = 0;
    sqlite3_vfs_unregister(&crashVfs);
  }

  ಹಿಂತಿರುಗಿ TCL_OK;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI crashParamsObjCmd(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಂಕ iDelay;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zCrashFile;
  Tcl_ಗಾತ್ರ nCrashFile;
  ಪೂರ್ಣಾಂಕ iDc, i_Sector_ಗಾತ್ರ;

  iDc = -1;
  i_Sector_ಗಾತ್ರ = -1;

  ಆದರೆ( objc<3 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "?OPTIONS? DELAY CRASHFILE");
    ಹೋಗು error;
  }

  zCrashFile = Tcl_GetStringFromObj(objv[objc-1], &nCrashFile);
  ಆದರೆ( nCrashFile>=ಗಾತ್ರ(g.zCrashFile) ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "Filename is too long: \"", zCrashFile, "\"", ಖಾಲಿ);
    ಹೋಗು error;
  }
  ಆದರೆ( Tcl_GetIntFromObj(interp, objv[objc-2], &iDelay) ){
    ಹೋಗು error;
  }

  ಆದರೆ( process_Dev_Sym_ವಾದಗಳು(interp, objc-3, &objv[1], &iDc, &i_Sector_ಗಾತ್ರ) ){
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  ಆದರೆ( iDc>=0 ){
    g.iDeviceCharacteristics = iDc;
  }
  ಆದರೆ( i_Sector_ಗಾತ್ರ>=0 ){
    g.i_Sector_ಗಾತ್ರ = i_Sector_ಗಾತ್ರ;
  }

  g.iCrash = iDelay;
  ಸ್ಮರಣೆ_ನಕಲು(g.zCrashFile, zCrashFile, nCrashFile+1);
  sqlite3CrashTestEnable = 1;
  ಹಿಂತಿರುಗಿ TCL_OK;

error:
  ಹಿಂತಿರುಗಿ TCL_ERROR;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI devSymObjCmd(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಶೂನ್ಯ devsym_ರಿಜಿಸ್ಟರ್(ಪೂರ್ಣಾಂಕ iDeviceChar, ಪೂರ್ಣಾಂಕ i_Sector_ಗಾತ್ರ);

  ಪೂರ್ಣಾಂಕ iDc = -1;
  ಪೂರ್ಣಾಂಕ i_Sector_ಗಾತ್ರ = -1;

  ಆದರೆ( process_Dev_Sym_ವಾದಗಳು(interp, objc-1, &objv[1], &iDc, &i_Sector_ಗಾತ್ರ) ){
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  devsym_ರಿಜಿಸ್ಟರ್(iDc, i_Sector_ಗಾತ್ರ);

  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** tclcmd: sqlite3_crash_on_write N
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI writeCrashObjCmd(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಶೂನ್ಯ devsym_crash_on_write(ಪೂರ್ಣಾಂಕ);
  ಪೂರ್ಣಾಂಕ nWrite = 0;

  ಆದರೆ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "NWRITE");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( Tcl_GetIntFromObj(interp, objv[1], &nWrite) ){
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  devsym_crash_on_write(nWrite);
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** tclcmd: unregister_devsim
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI dsUnregisterObjCmd(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಶೂನ್ಯ devsym_unregister(ಶೂನ್ಯ);

  ಆದರೆ( objc!=1 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  devsym_unregister();
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** tclcmd: register_jt_vfs ?-default? PARENT-VFS
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI jtObjCmd(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಂಕ jt_ರಿಜಿಸ್ಟರ್(ಅಕ್ಷರ *, ಪೂರ್ಣಾಂಕ);
  ಅಕ್ಷರ *z_ಪೋಷಕ = 0;

  ಆದರೆ( objc!=2 && objc!=3 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "?-default? PARENT-VFS");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  z_ಪೋಷಕ = Tcl_GetString(objv[1]);
  ಆದರೆ( objc==3 ){
    ಆದರೆ( strcmp(z_ಪೋಷಕ, "-default") ){
      Tcl_Append_ಫಲಿತಾಂಶ(interp,
          "bad option \"", z_ಪೋಷಕ, "\": must be -default", ಖಾಲಿ
      );
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }
    z_ಪೋಷಕ = Tcl_GetString(objv[2]);
  }

  ಆದರೆ( !(*z_ಪೋಷಕ) ){
    z_ಪೋಷಕ = 0;
  }
  ಆದರೆ( jt_ರಿಜಿಸ್ಟರ್(z_ಪೋಷಕ, objc==3) ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "Error in jt_register", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** tclcmd: unregister_jt_vfs
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI jtUnregisterObjCmd(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಶೂನ್ಯ jt_unregister(ಶೂನ್ಯ);

  ಆದರೆ( objc!=1 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  jt_unregister();
  ಹಿಂತಿರುಗಿ TCL_OK;
}

#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_DISKIO */

/*
** This procedure registers the TCL procedures defined in this file.
*/
ಪೂರ್ಣಾಂಕ Sqlitetest6_Init(Tcl_Interp *interp){
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_DISKIO
  Tcl_CreateObjCommand(interp, "sqlite3_crash_enable", crashEnableCmd, 0, 0);
  Tcl_CreateObjCommand(interp, "sqlite3_crashparams", crashParamsObjCmd, 0, 0);
  Tcl_CreateObjCommand(interp, "sqlite3_crash_now", crashNowCmd, 0, 0);
  Tcl_CreateObjCommand(interp, "sqlite3_simulate_device", devSymObjCmd, 0, 0);
  Tcl_CreateObjCommand(interp, "sqlite3_crash_on_write", writeCrashObjCmd,0,0);
  Tcl_CreateObjCommand(interp, "unregister_devsim", dsUnregisterObjCmd, 0, 0);
  Tcl_CreateObjCommand(interp, "register_jt_vfs", jtObjCmd, 0, 0);
  Tcl_CreateObjCommand(interp, "unregister_jt_vfs", jtUnregisterObjCmd, 0, 0);
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ TCL_OK;
}

#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_TEST */
