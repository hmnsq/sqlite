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
#ਜੇ SQLITE_TEST          /* This file is used for testing only */
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "tclsqlite.h"

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_DISKIO  /* This file is a no-op if disk I/O is disabled */

/* #define TRACE_CRASHTEST */

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ CrashFile CrashFile;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ CrashGlobal CrashGlobal;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Write_ਬਫਰ Write_ਬਫਰ;

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
ਬਣਤਰ Write_ਬਫਰ {
  i64 i_ਵਿੱਥ;                 /* Byte offset of the start of this write() */
  ਪੂਰਨ nBuf;                    /* Number of bytes written */
  u8 *zBuf;                    /* Pointer to copy of written data */
  CrashFile *pFile;            /* File this write() applies to */

  Write_ਬਫਰ *p_ਅਗਲਾ;          /* Next in CrashGlobal.pWriteList */
};

ਬਣਤਰ CrashFile {
  ਅਚਲ sqlite3_io_methods *p_ਵਿਧੀ;   /* Must be first */
  sqlite3_file *pRealFile;             /* Underlying "real" file handle */
  ਅੱਖਰ *zName;
  ਪੂਰਨ flags;                           /* Flags the file was opened with */

  /* Cache of the entire file. This is used to speed up OsRead() and
  ** OsFileSize() calls. Although both could be done by traversing the
  ** write-list, in practice this is impractically slow.
  */
  u8 *z_ਅੰਕੜੇ;                           /* Buffer containing file contents */
  ਪੂਰਨ n_ਅੰਕੜੇ;                           /* Size of buffer allocated at zData */
  i64 i_ਮਾਪ;                           /* Size of file in bytes */
};

ਬਣਤਰ CrashGlobal {
  Write_ਬਫਰ *pWriteList;     /* Head of write-list */
  Write_ਬਫਰ *p_Write_List_ਸਿਰਾ;  /* End of write-list */

  ਪੂਰਨ i_Sector_ਮਾਪ;             /* Value of simulated sector size */
  ਪੂਰਨ iDeviceCharacteristics;  /* Value of simulated device characteristics */

  ਪੂਰਨ iCrash;                  /* Crash on the iCrash'th call to xSync() */
  ਅੱਖਰ zCrashFile[500];        /* Crash during an xSync() on this file */
};

ਸਥਿਰ CrashGlobal g = {0, 0, SQLITE_DEFAULT_SECTOR_ਮਾਪ, 0, 0};

/*
** Set this global variable to 1 to enable crash testing.
*/
ਸਥਿਰ ਪੂਰਨ sqlite3CrashTestEnable = 0;

ਸਥਿਰ ਖਾਲੀ *crash_malloc(ਪੂਰਨ nByte){
  ਵਾਪਸ (ਖਾਲੀ *)Tcl_AttemptAlloc((size_t)nByte);
}
ਸਥਿਰ ਖਾਲੀ crash_free(ਖਾਲੀ *p){
  Tcl_Free(p);
}
ਸਥਿਰ ਖਾਲੀ *crash_realloc(ਖਾਲੀ *p, ਪੂਰਨ n){
  ਵਾਪਸ (ਖਾਲੀ *)Tcl_AttemptRealloc(p, (size_t)n);
}

/*
** Wrapper around the sqlite3OsWrite() function that avoids writing to the
** 512 byte block beginning at offset PENDING_BYTE.
*/
ਸਥਿਰ ਪੂਰਨ writeDbFile(CrashFile *p, u8 *z, i64 iAmt, i64 iOff){
  ਪੂਰਨ rc = SQLITE_OK;
  ਪੂਰਨ iSkip = 0;
  ਜੇ( (iAmt-iSkip)>0 ){
    rc = sqlite3OsWrite(p->pRealFile, &z[iSkip], (ਪੂਰਨ)(iAmt-iSkip), iOff+iSkip);
  }
  ਵਾਪਸ rc;
}

/*
** Flush the write-list as if xSync() had been called on file handle
** pFile. If isCrash is true, simulate a crash.
*/
ਸਥਿਰ ਪੂਰਨ writeListSync(CrashFile *pFile, ਪੂਰਨ isCrash){
  ਪੂਰਨ rc = SQLITE_OK;
  ਪੂਰਨ iDc = g.iDeviceCharacteristics;

  Write_ਬਫਰ *pWrite;
  Write_ਬਫਰ **ppPtr;

  /* If this is not a crash simulation, set pFinal to point to the
  ** last element of the write-list that is associated with file handle
  ** pFile.
  **
  ** If this is a crash simulation, set pFinal to an arbitrarily selected
  ** element of the write-list.
  */
  Write_ਬਫਰ *pFinal = 0;
  ਜੇ( !isCrash ){
    ਲਈ(pWrite=g.pWriteList; pWrite; pWrite=pWrite->p_ਅਗਲਾ){
      ਜੇ( pWrite->pFile==pFile ){
        pFinal = pWrite;
      }
    }
  }ਨਹੀਂਤਾਂ ਜੇ( iDc&(SQLITE_IOCAP_SEQUENTIAL|SQLITE_IOCAP_ਸੁਰੱਖਿਅਤ_APPEND) ){
    ਪੂਰਨ nWrite = 0;
    ਪੂਰਨ iFinal;
    ਲਈ(pWrite=g.pWriteList; pWrite; pWrite=pWrite->p_ਅਗਲਾ) nWrite++;
    sqlite3_randomness(ਆਕਾਰ(ਪੂਰਨ), &iFinal);
    iFinal = ((iFinal<0)?-1*iFinal:iFinal)%nWrite;
    ਲਈ(pWrite=g.pWriteList; iFinal>0; pWrite=pWrite->p_ਅਗਲਾ) iFinal--;
    pFinal = pWrite;
  }

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ TRACE_CRASHTEST
  ਜੇ( pFile ){
    ਲਿਖੋ("Sync %s (is %s crash)\n", pFile->zName, (isCrash?"a":"not a"));
  }
#ਜੇ_ਅੰਤ

  ppPtr = &g.pWriteList;
  ਲਈ(pWrite=*ppPtr; rc==SQLITE_OK && pWrite; pWrite=*ppPtr){
    sqlite3_file *pRealFile = pWrite->pFile->pRealFile;

    /* (eAction==1)      -> write block out normally,
    ** (eAction==2)      -> do nothing,
    ** (eAction==3)      -> trash sectors.
    */
    ਪੂਰਨ eAction = 0;
    ਜੇ( !isCrash ){
      eAction = 2;
      ਜੇ( (pWrite->pFile==pFile || iDc&SQLITE_IOCAP_SEQUENTIAL) ){
        eAction = 1;
      }
    }ਨਹੀਂਤਾਂ{
      ਅੱਖਰ random;
      sqlite3_randomness(1, &random);

      /* Do not select option 3 (sector trashing) if the IOCAP_ATOMIC flag
      ** is set or this is an OsTruncate(), not an Oswrite().
      */
      ਜੇ( (iDc&SQLITE_IOCAP_ATOMIC) || (pWrite->zBuf==0) ){
        random &= 0x01;
      }

      /* If IOCAP_SEQUENTIAL is set and this is not the final entry
      ** in the truncated write-list, always select option 1 (write
      ** out correctly).
      */
      ਜੇ( (iDc&SQLITE_IOCAP_SEQUENTIAL && pWrite!=pFinal) ){
        random = 0;
      }

      /* If IOCAP_SAFE_APPEND is set and this OsWrite() operation is
      ** an append (first byte of the written region is 1 byte past the
      ** current EOF), always select option 1 (write out correctly).
      */
      ਜੇ( iDc&SQLITE_IOCAP_ਸੁਰੱਖਿਅਤ_APPEND && pWrite->zBuf ){
        i64 i_ਮਾਪ;
        sqlite3_Os_File_ਮਾਪ(pRealFile, &i_ਮਾਪ);
        ਜੇ( i_ਮਾਪ==pWrite->i_ਵਿੱਥ ){
          random = 0;
        }
      }

      ਜੇ( (random&0x06)==0x06 ){
        eAction = 3;
      }ਨਹੀਂਤਾਂ{
        eAction = ((random&0x01)?2:1);
      }
    }

    ਚੋਣ( eAction ){
      ਹਾਲਤ 1: {               /* Write out correctly */
        ਜੇ( pWrite->zBuf ){
          rc = writeDbFile(
              pWrite->pFile, pWrite->zBuf, pWrite->nBuf, pWrite->i_ਵਿੱਥ
          );
        }ਨਹੀਂਤਾਂ{
          rc = sqlite3OsTruncate(pRealFile, pWrite->i_ਵਿੱਥ);
        }
        *ppPtr = pWrite->p_ਅਗਲਾ;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ TRACE_CRASHTEST
        ਜੇ( isCrash ){
          ਲਿਖੋ("Writing %d bytes @ %d (%s)\n",
            pWrite->nBuf, (ਪੂਰਨ)pWrite->i_ਵਿੱਥ, pWrite->pFile->zName
          );
        }
#ਜੇ_ਅੰਤ
        crash_free(pWrite);
        ਰੋਕੋ;
      }
      ਹਾਲਤ 2: {               /* Do nothing */
        ppPtr = &pWrite->p_ਅਗਲਾ;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ TRACE_CRASHTEST
        ਜੇ( isCrash ){
          ਲਿਖੋ("Omiting %d bytes @ %d (%s)\n",
            pWrite->nBuf, (ਪੂਰਨ)pWrite->i_ਵਿੱਥ, pWrite->pFile->zName
          );
        }
#ਜੇ_ਅੰਤ
        ਰੋਕੋ;
      }
      ਹਾਲਤ 3: {               /* Trash sectors */
        u8 *zGarbage;
        ਪੂਰਨ i_ਪਹਿਲਾ = (ਪੂਰਨ)(pWrite->i_ਵਿੱਥ/g.i_Sector_ਮਾਪ);
        ਪੂਰਨ i_ਆਖਰੀ = (ਪੂਰਨ)((pWrite->i_ਵਿੱਥ+pWrite->nBuf-1)/g.i_Sector_ਮਾਪ);

        ਪ੍ਰਤਿਜ੍ਞਾ(pWrite->zBuf);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ TRACE_CRASHTEST
        ਲਿਖੋ("Trashing %d sectors (%d bytes) @ %lld (sector %d) (%s)\n",
            1+i_ਆਖਰੀ-i_ਪਹਿਲਾ, (1+i_ਆਖਰੀ-i_ਪਹਿਲਾ)*g.i_Sector_ਮਾਪ,
            pWrite->i_ਵਿੱਥ, i_ਪਹਿਲਾ, pWrite->pFile->zName
        );
#ਜੇ_ਅੰਤ

        zGarbage = crash_malloc(g.i_Sector_ਮਾਪ);
        ਜੇ( zGarbage ){
          sqlite3_int64 i;
          ਲਈ(i=i_ਪਹਿਲਾ; rc==SQLITE_OK && i<=i_ਆਖਰੀ; i++){
            sqlite3_randomness(g.i_Sector_ਮਾਪ, zGarbage);
            rc = writeDbFile(
              pWrite->pFile, zGarbage, g.i_Sector_ਮਾਪ, i*g.i_Sector_ਮਾਪ
            );
          }
          crash_free(zGarbage);
        }ਨਹੀਂਤਾਂ{
          rc = SQLITE_NOMEM;
        }

        ppPtr = &pWrite->p_ਅਗਲਾ;
        ਰੋਕੋ;
      }

      ਮੂਲ:
        ਪ੍ਰਤਿਜ੍ਞਾ(!"Cannot happen");
    }

    ਜੇ( pWrite==pFinal ) ਰੋਕੋ;
  }

  ਜੇ( rc==SQLITE_OK && isCrash ){
    ਬਾਹਰ_ਨਿਕਲੋ(-1);
  }

  ਲਈ(pWrite=g.pWriteList; pWrite && pWrite->p_ਅਗਲਾ; pWrite=pWrite->p_ਅਗਲਾ);
  g.p_Write_List_ਸਿਰਾ = pWrite;

  ਵਾਪਸ rc;
}

/*
** Add an entry to the end of the write-list.
*/
ਸਥਿਰ ਪੂਰਨ writeListAppend(
  sqlite3_file *pFile,
  sqlite3_int64 i_ਵਿੱਥ,
  ਅਚਲ u8 *zBuf,
  ਪੂਰਨ nBuf
){
  Write_ਬਫਰ *pNew;

  ਪ੍ਰਤਿਜ੍ਞਾ((zBuf && nBuf) || (!nBuf && !zBuf));

  pNew = (Write_ਬਫਰ *)crash_malloc(ਆਕਾਰ(Write_ਬਫਰ) + nBuf);
  ਜੇ( pNew==0 ){
    fprintf(stderr, "out of memory in the crash simulator\n");
  }
  memset(pNew, 0, ਆਕਾਰ(Write_ਬਫਰ)+nBuf);
  pNew->i_ਵਿੱਥ = i_ਵਿੱਥ;
  pNew->nBuf = nBuf;
  pNew->pFile = (CrashFile *)pFile;
  ਜੇ( zBuf ){
    pNew->zBuf = (u8 *)&pNew[1];
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(pNew->zBuf, zBuf, nBuf);
  }

  ਜੇ( g.pWriteList ){
    ਪ੍ਰਤਿਜ੍ਞਾ(g.p_Write_List_ਸਿਰਾ);
    g.p_Write_List_ਸਿਰਾ->p_ਅਗਲਾ = pNew;
  }ਨਹੀਂਤਾਂ{
    g.pWriteList = pNew;
  }
  g.p_Write_List_ਸਿਰਾ = pNew;
 
  ਵਾਪਸ SQLITE_OK;
}

/*
** Close a crash-file.
*/
ਸਥਿਰ ਪੂਰਨ cfClose(sqlite3_file *pFile){
  CrashFile *pCrash = (CrashFile *)pFile;
  writeListSync(pCrash, 0);
  sqlite3OsClose(pCrash->pRealFile);
  ਵਾਪਸ SQLITE_OK;
}

/*
** Read data from a crash-file.
*/
ਸਥਿਰ ਪੂਰਨ cfRead(
  sqlite3_file *pFile,
  ਖਾਲੀ *zBuf,
  ਪੂਰਨ iAmt,
  sqlite_int64 iOfst
){
  CrashFile *pCrash = (CrashFile *)pFile;
  ਪੂਰਨ nCopy = (ਪੂਰਨ)MIN((i64)iAmt, (pCrash->i_ਮਾਪ - iOfst));

  ਜੇ( nCopy>0 ){
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(zBuf, &pCrash->z_ਅੰਕੜੇ[iOfst], nCopy);
  }

  /* Check the file-size to see if this is a short-read */
  ਜੇ( nCopy<iAmt ){
    ਵਾਪਸ SQLITE_IOERR_SHORT_READ;
  }

  ਵਾਪਸ SQLITE_OK;
}

/*
** Write data to a crash-file.
*/
ਸਥਿਰ ਪੂਰਨ cfWrite(
  sqlite3_file *pFile,
  ਅਚਲ ਖਾਲੀ *zBuf,
  ਪੂਰਨ iAmt,
  sqlite_int64 iOfst
){
  CrashFile *pCrash = (CrashFile *)pFile;
  ਜੇ( iAmt+iOfst>pCrash->i_ਮਾਪ ){
    pCrash->i_ਮਾਪ = (ਪੂਰਨ)(iAmt+iOfst);
  }
  ਜਦੋਂ( pCrash->i_ਮਾਪ>pCrash->n_ਅੰਕੜੇ ){
    u8 *zNew;
    ਪੂਰਨ nNew = (pCrash->n_ਅੰਕੜੇ*2) + 4096;
    zNew = crash_realloc(pCrash->z_ਅੰਕੜੇ, nNew);
    ਜੇ( !zNew ){
      ਵਾਪਸ SQLITE_NOMEM;
    }
    memset(&zNew[pCrash->n_ਅੰਕੜੇ], 0, nNew-pCrash->n_ਅੰਕੜੇ);
    pCrash->n_ਅੰਕੜੇ = nNew;
    pCrash->z_ਅੰਕੜੇ = zNew;
  }
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(&pCrash->z_ਅੰਕੜੇ[iOfst], zBuf, iAmt);
  ਵਾਪਸ writeListAppend(pFile, iOfst, zBuf, iAmt);
}

/*
** Truncate a crash-file.
*/
ਸਥਿਰ ਪੂਰਨ cfTruncate(sqlite3_file *pFile, sqlite_int64 ਮਾਪ){
  CrashFile *pCrash = (CrashFile *)pFile;
  ਪ੍ਰਤਿਜ੍ਞਾ(ਮਾਪ>=0);
  ਜੇ( pCrash->i_ਮਾਪ>ਮਾਪ ){
    pCrash->i_ਮਾਪ = (ਪੂਰਨ)ਮਾਪ;
  }
  ਵਾਪਸ writeListAppend(pFile, ਮਾਪ, 0, 0);
}

/*
** Sync a crash-file.
*/
ਸਥਿਰ ਪੂਰਨ cfSync(sqlite3_file *pFile, ਪੂਰਨ flags){
  CrashFile *pCrash = (CrashFile *)pFile;
  ਪੂਰਨ isCrash = 0;

  ਅਚਲ ਅੱਖਰ *zName = pCrash->zName;
  ਅਚਲ ਅੱਖਰ *zCrashFile = g.zCrashFile;
  ਪੂਰਨ nName = (ਪੂਰਨ)ਲੜੀ_ਲੰਬਾਈ(zName);
  ਪੂਰਨ nCrashFile = (ਪੂਰਨ)ਲੜੀ_ਲੰਬਾਈ(zCrashFile);

  ਜੇ( nCrashFile>0 && zCrashFile[nCrashFile-1]=='*' ){
    nCrashFile--;
    ਜੇ( nName>nCrashFile ) nName = nCrashFile;
  }

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ TRACE_CRASHTEST
  ਲਿਖੋ("cfSync(): nName = %d, nCrashFile = %d, zName = %s, zCrashFile = %s\n",
         nName, nCrashFile, zName, zCrashFile);
#ਜੇ_ਅੰਤ

  ਜੇ( nName==nCrashFile && 0==memcmp(zName, zCrashFile, nName) ){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ TRACE_CRASHTEST
    ਲਿਖੋ("cfSync(): name matched, g.iCrash = %d\n", g.iCrash);
#ਜੇ_ਅੰਤ
    ਜੇ( (--g.iCrash)==0 ) isCrash = 1;
  }

  ਵਾਪਸ writeListSync(pCrash, isCrash);
}

/*
** Return the current file-size of the crash-file.
*/
ਸਥਿਰ ਪੂਰਨ cf_File_ਮਾਪ(sqlite3_file *pFile, sqlite_int64 *p_ਮਾਪ){
  CrashFile *pCrash = (CrashFile *)pFile;
  *p_ਮਾਪ = (i64)pCrash->i_ਮਾਪ;
  ਵਾਪਸ SQLITE_OK;
}

/*
** Calls related to file-locks are passed on to the real file handle.
*/
ਸਥਿਰ ਪੂਰਨ cf_ਤਾਲਾ(sqlite3_file *pFile, ਪੂਰਨ e_ਤਾਲਾ){
  ਵਾਪਸ sqlite3_Os_ਤਾਲਾ(((CrashFile *)pFile)->pRealFile, e_ਤਾਲਾ);
}
ਸਥਿਰ ਪੂਰਨ cfUnlock(sqlite3_file *pFile, ਪੂਰਨ e_ਤਾਲਾ){
  ਵਾਪਸ sqlite3OsUnlock(((CrashFile *)pFile)->pRealFile, e_ਤਾਲਾ);
}
ਸਥਿਰ ਪੂਰਨ cf_Check_Reserved_ਤਾਲਾ(sqlite3_file *pFile, ਪੂਰਨ *p_Res_ਬਾਹਰਲਾ){
  ਵਾਪਸ sqlite3_Os_Check_Reserved_ਤਾਲਾ(((CrashFile *)pFile)->pRealFile, p_Res_ਬਾਹਰਲਾ);
}
ਸਥਿਰ ਪੂਰਨ cfFileControl(sqlite3_file *pFile, ਪੂਰਨ op, ਖਾਲੀ *pArg){
  ਜੇ( op==SQLITE_FCNTL_ਮਾਪ_HINT ){
    CrashFile *pCrash = (CrashFile *)pFile;
    i64 nByte = *(i64 *)pArg;
    ਜੇ( nByte>pCrash->i_ਮਾਪ ){
      ਜੇ( SQLITE_OK==writeListAppend(pFile, nByte, 0, 0) ){
        pCrash->i_ਮਾਪ = (ਪੂਰਨ)nByte;
      }
    }
    ਵਾਪਸ SQLITE_OK;
  }
  ਵਾਪਸ sqlite3OsFileControl(((CrashFile *)pFile)->pRealFile, op, pArg);
}

/*
** The xSectorSize() and xDeviceCharacteristics() functions return
** the global values configured by the [sqlite_crashparams] tcl
*  interface.
*/
ਸਥਿਰ ਪੂਰਨ cf_Sector_ਮਾਪ(sqlite3_file *pFile){
  ਵਾਪਸ g.i_Sector_ਮਾਪ;
}
ਸਥਿਰ ਪੂਰਨ cfDeviceCharacteristics(sqlite3_file *pFile){
  ਵਾਪਸ g.iDeviceCharacteristics;
}

/*
** Pass-throughs for WAL support.
*/
ਸਥਿਰ ਪੂਰਨ cf_Shm_ਤਾਲਾ(sqlite3_file *pFile, ਪੂਰਨ ofst, ਪੂਰਨ n, ਪੂਰਨ flags){
  sqlite3_file *pReal = ((CrashFile*)pFile)->pRealFile;
  ਵਾਪਸ pReal->pMethods->x_Shm_ਤਾਲਾ(pReal, ofst, n, flags);
}
ਸਥਿਰ ਖਾਲੀ cfShmBarrier(sqlite3_file *pFile){
  sqlite3_file *pReal = ((CrashFile*)pFile)->pRealFile;
  pReal->pMethods->xShmBarrier(pReal);
}
ਸਥਿਰ ਪੂਰਨ cfShmUnmap(sqlite3_file *pFile, ਪੂਰਨ delFlag){
  sqlite3_file *pReal = ((CrashFile*)pFile)->pRealFile;
  ਵਾਪਸ pReal->pMethods->xShmUnmap(pReal, delFlag);
}
ਸਥਿਰ ਪੂਰਨ cfShmMap(
  sqlite3_file *pFile,            /* Handle open on database file */
  ਪੂਰਨ iRegion,                    /* Region to retrieve */
  ਪੂਰਨ sz,                         /* Size of regions */
  ਪੂਰਨ w,                          /* True to extend file if necessary */
  ਖਾਲੀ ਅਸਥਿਰ **pp              /* OUT: Mapped memory */
){
  sqlite3_file *pReal = ((CrashFile*)pFile)->pRealFile;
  ਵਾਪਸ pReal->pMethods->xShmMap(pReal, iRegion, sz, w, pp);
}

ਸਥਿਰ ਅਚਲ sqlite3_io_methods CrashFileVtab = {
  2,                            /* iVersion */
  cfClose,                      /* xClose */
  cfRead,                       /* xRead */
  cfWrite,                      /* xWrite */
  cfTruncate,                   /* xTruncate */
  cfSync,                       /* xSync */
  cf_File_ਮਾਪ,                   /* xFileSize */
  cf_ਤਾਲਾ,                       /* xLock */
  cfUnlock,                     /* xUnlock */
  cf_Check_Reserved_ਤਾਲਾ,          /* xCheckReservedLock */
  cfFileControl,                /* xFileControl */
  cf_Sector_ਮਾਪ,                 /* xSectorSize */
  cfDeviceCharacteristics,      /* xDeviceCharacteristics */
  cfShmMap,                     /* xShmMap */
  cf_Shm_ਤਾਲਾ,                    /* xShmLock */
  cfShmBarrier,                 /* xShmBarrier */
  cfShmUnmap                    /* xShmUnmap */
};

/*
** Application data for the crash VFS
*/
ਬਣਤਰ crash_App_ਅੰਕੜੇ {
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
ਸਥਿਰ ਪੂਰਨ cfOpen(
  sqlite3_vfs *pCfVfs,
  ਅਚਲ ਅੱਖਰ *zName,
  sqlite3_file *pFile,
  ਪੂਰਨ flags,
  ਪੂਰਨ *p_ਬਾਹਰਲਾ_Flags
){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_ਅੰਕੜੇ;
  ਪੂਰਨ rc;
  CrashFile *pWrapper = (CrashFile *)pFile;
  sqlite3_file *pReal = (sqlite3_file*)&pWrapper[1];

  memset(pWrapper, 0, ਆਕਾਰ(CrashFile));
  rc = sqlite3OsOpen(pVfs, zName, pReal, flags, p_ਬਾਹਰਲਾ_Flags);

  ਜੇ( rc==SQLITE_OK ){
    i64 i_ਮਾਪ;
    pWrapper->p_ਵਿਧੀ = &CrashFileVtab;
    pWrapper->zName = (ਅੱਖਰ *)zName;
    pWrapper->pRealFile = pReal;
    rc = sqlite3_Os_File_ਮਾਪ(pReal, &i_ਮਾਪ);
    pWrapper->i_ਮਾਪ = (ਪੂਰਨ)i_ਮਾਪ;
    pWrapper->flags = flags;
  }
  ਜੇ( rc==SQLITE_OK ){
    pWrapper->n_ਅੰਕੜੇ = (ਪੂਰਨ)(4096 + pWrapper->i_ਮਾਪ);
    pWrapper->z_ਅੰਕੜੇ = crash_malloc(pWrapper->n_ਅੰਕੜੇ);
    ਜੇ( pWrapper->z_ਅੰਕੜੇ ){
      /* os_unix.c contains an assert() that fails if the caller attempts
      ** to read data from the 512-byte locking region of a file opened
      ** with the SQLITE_OPEN_MAIN_DB flag. This region of a database file
      ** never contains valid data anyhow. So avoid doing such a read here.
      **
      ** UPDATE: It also contains an assert() verifying that each call
      ** to the xRead() method reads less than 128KB of data.
      */
      i64 iOff;

      memset(pWrapper->z_ਅੰਕੜੇ, 0, pWrapper->n_ਅੰਕੜੇ);
      ਲਈ(iOff=0; iOff<pWrapper->i_ਮਾਪ; iOff += 512){
        ਪੂਰਨ nRead = (ਪੂਰਨ)(pWrapper->i_ਮਾਪ - iOff);
        ਜੇ( nRead>512 ) nRead = 512;
        rc = sqlite3OsRead(pReal, &pWrapper->z_ਅੰਕੜੇ[iOff], nRead, iOff);
      }
    }ਨਹੀਂਤਾਂ{
      rc = SQLITE_NOMEM;
    }
  }
  ਜੇ( rc!=SQLITE_OK && pWrapper->p_ਵਿਧੀ ){
    sqlite3OsClose(pFile);
  }
  ਵਾਪਸ rc;
}

ਸਥਿਰ ਪੂਰਨ cfDelete(sqlite3_vfs *pCfVfs, ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ, ਪੂਰਨ dirSync){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_ਅੰਕੜੇ;
  ਵਾਪਸ pVfs->xDelete(pVfs, z_ਰਸਤਾ, dirSync);
}
ਸਥਿਰ ਪੂਰਨ cfAccess(
  sqlite3_vfs *pCfVfs,
  ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ,
  ਪੂਰਨ flags,
  ਪੂਰਨ *p_Res_ਬਾਹਰਲਾ
){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_ਅੰਕੜੇ;
  ਵਾਪਸ pVfs->xAccess(pVfs, z_ਰਸਤਾ, flags, p_Res_ਬਾਹਰਲਾ);
}
ਸਥਿਰ ਪੂਰਨ cf_ਭਰਿਆ_Pathname(
  sqlite3_vfs *pCfVfs,
  ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ,
  ਪੂਰਨ n_ਰਸਤਾ_ਬਾਹਰਲਾ,
  ਅੱਖਰ *z_ਰਸਤਾ_ਬਾਹਰਲਾ
){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_ਅੰਕੜੇ;
  ਵਾਪਸ pVfs->x_ਭਰਿਆ_Pathname(pVfs, z_ਰਸਤਾ, n_ਰਸਤਾ_ਬਾਹਰਲਾ, z_ਰਸਤਾ_ਬਾਹਰਲਾ);
}
ਸਥਿਰ ਖਾਲੀ *cfDlOpen(sqlite3_vfs *pCfVfs, ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_ਅੰਕੜੇ;
  ਵਾਪਸ pVfs->xDlOpen(pVfs, z_ਰਸਤਾ);
}
ਸਥਿਰ ਖਾਲੀ cfDlError(sqlite3_vfs *pCfVfs, ਪੂਰਨ nByte, ਅੱਖਰ *z_Err_ਸੁਨੇਹਾ){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_ਅੰਕੜੇ;
  pVfs->xDlError(pVfs, nByte, z_Err_ਸੁਨੇਹਾ);
}
ਸਥਿਰ ਖਾਲੀ (*cfDlSym(sqlite3_vfs *pCfVfs, ਖਾਲੀ *pH, ਅਚਲ ਅੱਖਰ *zSym))(ਖਾਲੀ){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_ਅੰਕੜੇ;
  ਵਾਪਸ pVfs->xDlSym(pVfs, pH, zSym);
}
ਸਥਿਰ ਖਾਲੀ cfDlClose(sqlite3_vfs *pCfVfs, ਖਾਲੀ *pHandle){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_ਅੰਕੜੇ;
  pVfs->xDlClose(pVfs, pHandle);
}
ਸਥਿਰ ਪੂਰਨ cfRandomness(sqlite3_vfs *pCfVfs, ਪੂਰਨ nByte, ਅੱਖਰ *z_Buf_ਬਾਹਰਲਾ){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_ਅੰਕੜੇ;
  ਵਾਪਸ pVfs->xRandomness(pVfs, nByte, z_Buf_ਬਾਹਰਲਾ);
}
ਸਥਿਰ ਪੂਰਨ cfSleep(sqlite3_vfs *pCfVfs, ਪੂਰਨ nMicro){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_ਅੰਕੜੇ;
  ਵਾਪਸ pVfs->xSleep(pVfs, nMicro);
}
ਸਥਿਰ ਪੂਰਨ cf_Current_ਸਮਾਂ(sqlite3_vfs *pCfVfs, ਦੁੱਗਣਾ *p_ਸਮਾਂ_ਬਾਹਰਲਾ){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_ਅੰਕੜੇ;
  ਵਾਪਸ pVfs->x_Current_ਸਮਾਂ(pVfs, p_ਸਮਾਂ_ਬਾਹਰਲਾ);
}
ਸਥਿਰ ਪੂਰਨ cf_Get_ਆਖਰੀ_Error(sqlite3_vfs *pCfVfs, ਪੂਰਨ n, ਅੱਖਰ *z){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_ਅੰਕੜੇ;
  ਵਾਪਸ pVfs->x_Get_ਆਖਰੀ_Error(pVfs, n, z);
}

ਸਥਿਰ ਪੂਰਨ process_Dev_Sym_ਦਲੀਲਾਂ(
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[],
  ਪੂਰਨ *piDeviceChar,
  ਪੂਰਨ *pi_Sector_ਮਾਪ
){
  ਬਣਤਰ DeviceFlag {
    ਅੱਖਰ *zName;
    ਪੂਰਨ i_ਮੁੱਲ;
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
    { "safe_append",         SQLITE_IOCAP_ਸੁਰੱਖਿਅਤ_APPEND           },
    { "powersafe_overwrite", SQLITE_IOCAP_POWERSAFE_OVERWRITE   },
    { "batch-atomic",        SQLITE_IOCAP_BATCH_ATOMIC          },
    { 0, 0 }
  };

  ਪੂਰਨ i;
  ਪੂਰਨ iDc = 0;
  ਪੂਰਨ i_Sector_ਮਾਪ = 0;
  ਪੂਰਨ ਸੈੱਟ_Sectorsize = 0;
  ਪੂਰਨ ਸੈੱਟ_Device_Char = 0;

  ਲਈ(i=0; i<objc; i+=2){
    Tcl_ਮਾਪ nOpt;
    ਅੱਖਰ *zOpt = Tcl_GetStringFromObj(objv[i], &nOpt);

    ਜੇ( (nOpt>11 || nOpt<2 || ਲੜੀ_ਤੁਲਨਾ("-sectorsize", zOpt, nOpt))
     && (nOpt>16 || nOpt<2 || ਲੜੀ_ਤੁਲਨਾ("-characteristics", zOpt, nOpt))
    ){
      Tcl_Append_ਨਤੀਜਾ(interp,
        "Bad option: \"", zOpt,
        "\" - must be \"-characteristics\" or \"-sectorsize\"", ਨਿਰਾ
      );
      ਵਾਪਸ TCL_ERROR;
    }
    ਜੇ( i==objc-1 ){
      Tcl_Append_ਨਤੀਜਾ(interp, "Option requires an argument: \"", zOpt, "\"", ਨਿਰਾ);
      ਵਾਪਸ TCL_ERROR;
    }

    ਜੇ( zOpt[1]=='s' ){
      ਜੇ( Tcl_GetIntFromObj(interp, objv[i+1], &i_Sector_ਮਾਪ) ){
        ਵਾਪਸ TCL_ERROR;
      }
      ਸੈੱਟ_Sectorsize = 1;
    }ਨਹੀਂਤਾਂ{
      ਪੂਰਨ j;
      Tcl_Obj **apObj;
      Tcl_ਮਾਪ nObj;
      ਜੇ( Tcl_ListObjGetElements(interp, objv[i+1], &nObj, &apObj) ){
        ਵਾਪਸ TCL_ERROR;
      }
      ਲਈ(j=0; j<(ਪੂਰਨ)nObj; j++){
        ਪੂਰਨ rc;
        ਪੂਰਨ iChoice;
        Tcl_Obj *pFlag = Tcl_DuplicateObj(apObj[j]);
        Tcl_Incr_Ref_ਗਿਣਤੀ(pFlag);
        Tcl_Utf_ਮੰਜ਼ਿਲ_ਹੇਠਲਾ(Tcl_GetString(pFlag));

        rc = Tcl_Get_ਸੂਚਕਾਂਕ_From_Obj_Struct(
            interp, pFlag, aFlag, ਆਕਾਰ(aFlag[0]), "no such flag", 0, &iChoice
        );
        Tcl_Decr_Ref_ਗਿਣਤੀ(pFlag);
        ਜੇ( rc ){
          ਵਾਪਸ TCL_ERROR;
        }

        iDc |= aFlag[iChoice].i_ਮੁੱਲ;
      }
      ਸੈੱਟ_Device_Char = 1;
    }
  }

  ਜੇ( ਸੈੱਟ_Device_Char ){
    *piDeviceChar = iDc;
  }
  ਜੇ( ਸੈੱਟ_Sectorsize ){
    *pi_Sector_ਮਾਪ = i_Sector_ਮਾਪ;
  }

  ਵਾਪਸ TCL_OK;
}

/*
** tclcmd:   sqlite3_crash_now
**
** Simulate a crash immediately. This function does not return
** (writeListSync() calls exit(-1)).
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI crashNowCmd(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਜੇ( objc!=1 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "");
    ਵਾਪਸ TCL_ERROR;
  }
  writeListSync(0, 1);
  ਪ੍ਰਤਿਜ੍ਞਾ( 0 );
  ਵਾਪਸ TCL_OK;
}

/*
** tclcmd:   sqlite_crash_enable ENABLE ?DEFAULT?
**
** Parameter ENABLE must be a boolean value. If true, then the "crash"
** vfs is added to the system. If false, it is removed.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI crashEnableCmd(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਪੂਰਨ isEnable;
  ਪੂਰਨ isDefault = 0;
  ਸਥਿਰ sqlite3_vfs crashVfs = {
    2,                  /* iVersion */
    0,                  /* szOsFile */
    0,                  /* mxPathname */
    0,                  /* pNext */
    "crash",            /* zName */
    0,                  /* pAppData */
 
    cfOpen,               /* xOpen */
    cfDelete,             /* xDelete */
    cfAccess,             /* xAccess */
    cf_ਭਰਿਆ_Pathname,       /* xFullPathname */
    cfDlOpen,             /* xDlOpen */
    cfDlError,            /* xDlError */
    cfDlSym,              /* xDlSym */
    cfDlClose,            /* xDlClose */
    cfRandomness,         /* xRandomness */
    cfSleep,              /* xSleep */
    cf_Current_ਸਮਾਂ,        /* xCurrentTime */
    cf_Get_ਆਖਰੀ_Error,       /* xGetLastError */
    0,                    /* xCurrentTimeInt64 */
  };

  ਜੇ( objc!=2 && objc!=3 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "ENABLE ?DEFAULT?");
    ਵਾਪਸ TCL_ERROR;
  }

  ਜੇ( Tcl_GetBooleanFromObj(interp, objv[1], &isEnable) ){
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( objc==3 && Tcl_GetBooleanFromObj(interp, objv[2], &isDefault) ){
    ਵਾਪਸ TCL_ERROR;
  }

  ਜੇ( (isEnable && crashVfs.p_App_ਅੰਕੜੇ) || (!isEnable && !crashVfs.p_App_ਅੰਕੜੇ) ){
    ਵਾਪਸ TCL_OK;
  }

  ਜੇ( crashVfs.p_App_ਅੰਕੜੇ==0 ){
    sqlite3_vfs *pOriginalVfs = sqlite3_vfs_ਲੱਭੋ(0);
    crashVfs.mxPathname = pOriginalVfs->mxPathname;
    crashVfs.p_App_ਅੰਕੜੇ = (ਖਾਲੀ *)pOriginalVfs;
    crashVfs.szOsFile = ਆਕਾਰ(CrashFile) + pOriginalVfs->szOsFile;
    sqlite3_vfs_ਰਜਿਸਟਰ(&crashVfs, isDefault);
  }ਨਹੀਂਤਾਂ{
    crashVfs.p_App_ਅੰਕੜੇ = 0;
    sqlite3_vfs_unregister(&crashVfs);
  }

  ਵਾਪਸ TCL_OK;
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
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI crashParamsObjCmd(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਪੂਰਨ iDelay;
  ਅਚਲ ਅੱਖਰ *zCrashFile;
  Tcl_ਮਾਪ nCrashFile;
  ਪੂਰਨ iDc, i_Sector_ਮਾਪ;

  iDc = -1;
  i_Sector_ਮਾਪ = -1;

  ਜੇ( objc<3 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "?OPTIONS? DELAY CRASHFILE");
    ਜਾਓ error;
  }

  zCrashFile = Tcl_GetStringFromObj(objv[objc-1], &nCrashFile);
  ਜੇ( nCrashFile>=ਆਕਾਰ(g.zCrashFile) ){
    Tcl_Append_ਨਤੀਜਾ(interp, "Filename is too long: \"", zCrashFile, "\"", ਨਿਰਾ);
    ਜਾਓ error;
  }
  ਜੇ( Tcl_GetIntFromObj(interp, objv[objc-2], &iDelay) ){
    ਜਾਓ error;
  }

  ਜੇ( process_Dev_Sym_ਦਲੀਲਾਂ(interp, objc-3, &objv[1], &iDc, &i_Sector_ਮਾਪ) ){
    ਵਾਪਸ TCL_ERROR;
  }

  ਜੇ( iDc>=0 ){
    g.iDeviceCharacteristics = iDc;
  }
  ਜੇ( i_Sector_ਮਾਪ>=0 ){
    g.i_Sector_ਮਾਪ = i_Sector_ਮਾਪ;
  }

  g.iCrash = iDelay;
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(g.zCrashFile, zCrashFile, nCrashFile+1);
  sqlite3CrashTestEnable = 1;
  ਵਾਪਸ TCL_OK;

error:
  ਵਾਪਸ TCL_ERROR;
}

ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI devSymObjCmd(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਖਾਲੀ devsym_ਰਜਿਸਟਰ(ਪੂਰਨ iDeviceChar, ਪੂਰਨ i_Sector_ਮਾਪ);

  ਪੂਰਨ iDc = -1;
  ਪੂਰਨ i_Sector_ਮਾਪ = -1;

  ਜੇ( process_Dev_Sym_ਦਲੀਲਾਂ(interp, objc-1, &objv[1], &iDc, &i_Sector_ਮਾਪ) ){
    ਵਾਪਸ TCL_ERROR;
  }
  devsym_ਰਜਿਸਟਰ(iDc, i_Sector_ਮਾਪ);

  ਵਾਪਸ TCL_OK;
}

/*
** tclcmd: sqlite3_crash_on_write N
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI writeCrashObjCmd(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਖਾਲੀ devsym_crash_on_write(ਪੂਰਨ);
  ਪੂਰਨ nWrite = 0;

  ਜੇ( objc!=2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "NWRITE");
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( Tcl_GetIntFromObj(interp, objv[1], &nWrite) ){
    ਵਾਪਸ TCL_ERROR;
  }

  devsym_crash_on_write(nWrite);
  ਵਾਪਸ TCL_OK;
}

/*
** tclcmd: unregister_devsim
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI dsUnregisterObjCmd(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਖਾਲੀ devsym_unregister(ਖਾਲੀ);

  ਜੇ( objc!=1 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "");
    ਵਾਪਸ TCL_ERROR;
  }

  devsym_unregister();
  ਵਾਪਸ TCL_OK;
}

/*
** tclcmd: register_jt_vfs ?-default? PARENT-VFS
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI jtObjCmd(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਪੂਰਨ jt_ਰਜਿਸਟਰ(ਅੱਖਰ *, ਪੂਰਨ);
  ਅੱਖਰ *z_ਮਾਪਾ = 0;

  ਜੇ( objc!=2 && objc!=3 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "?-default? PARENT-VFS");
    ਵਾਪਸ TCL_ERROR;
  }
  z_ਮਾਪਾ = Tcl_GetString(objv[1]);
  ਜੇ( objc==3 ){
    ਜੇ( strcmp(z_ਮਾਪਾ, "-default") ){
      Tcl_Append_ਨਤੀਜਾ(interp,
          "bad option \"", z_ਮਾਪਾ, "\": must be -default", ਨਿਰਾ
      );
      ਵਾਪਸ TCL_ERROR;
    }
    z_ਮਾਪਾ = Tcl_GetString(objv[2]);
  }

  ਜੇ( !(*z_ਮਾਪਾ) ){
    z_ਮਾਪਾ = 0;
  }
  ਜੇ( jt_ਰਜਿਸਟਰ(z_ਮਾਪਾ, objc==3) ){
    Tcl_Append_ਨਤੀਜਾ(interp, "Error in jt_register", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }

  ਵਾਪਸ TCL_OK;
}

/*
** tclcmd: unregister_jt_vfs
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI jtUnregisterObjCmd(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਖਾਲੀ jt_unregister(ਖਾਲੀ);

  ਜੇ( objc!=1 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "");
    ਵਾਪਸ TCL_ERROR;
  }

  jt_unregister();
  ਵਾਪਸ TCL_OK;
}

#ਜੇ_ਅੰਤ /* SQLITE_OMIT_DISKIO */

/*
** This procedure registers the TCL procedures defined in this file.
*/
ਪੂਰਨ Sqlitetest6_Init(Tcl_Interp *interp){
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_DISKIO
  Tcl_CreateObjCommand(interp, "sqlite3_crash_enable", crashEnableCmd, 0, 0);
  Tcl_CreateObjCommand(interp, "sqlite3_crashparams", crashParamsObjCmd, 0, 0);
  Tcl_CreateObjCommand(interp, "sqlite3_crash_now", crashNowCmd, 0, 0);
  Tcl_CreateObjCommand(interp, "sqlite3_simulate_device", devSymObjCmd, 0, 0);
  Tcl_CreateObjCommand(interp, "sqlite3_crash_on_write", writeCrashObjCmd,0,0);
  Tcl_CreateObjCommand(interp, "unregister_devsim", dsUnregisterObjCmd, 0, 0);
  Tcl_CreateObjCommand(interp, "register_jt_vfs", jtObjCmd, 0, 0);
  Tcl_CreateObjCommand(interp, "unregister_jt_vfs", jtUnregisterObjCmd, 0, 0);
#ਜੇ_ਅੰਤ
  ਵਾਪਸ TCL_OK;
}

#ਜੇ_ਅੰਤ /* SQLITE_TEST */
