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
#اگر SQLITE_TEST          /* This file is used for testing only */
#شامل "sqliteInt.h"
#شامل "tclsqlite.h"

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_DISKIO  /* This file is a no-op if disk I/O is disabled */

/* #define TRACE_CRASHTEST */

قسمتعریف ساخت CrashFile CrashFile;
قسمتعریف ساخت CrashGlobal CrashGlobal;
قسمتعریف ساخت Write_بفر Write_بفر;

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
ساخت Write_بفر {
  i64 i_انحراف;                 /* Byte offset of the start of this write() */
  صحیح nBuf;                    /* Number of bytes written */
  u8 *zBuf;                    /* Pointer to copy of written data */
  CrashFile *pFile;            /* File this write() applies to */

  Write_بفر *p_اگلا;          /* Next in CrashGlobal.pWriteList */
};

ساخت CrashFile {
  ثابت sqlite3_io_methods *p_طریقہ;   /* Must be first */
  sqlite3_file *pRealFile;             /* Underlying "real" file handle */
  حرف *zName;
  صحیح flags;                           /* Flags the file was opened with */

  /* Cache of the entire file. This is used to speed up OsRead() and
  ** OsFileSize() calls. Although both could be done by traversing the
  ** write-list, in practice this is impractically slow.
  */
  u8 *z_ڈیٹا;                           /* Buffer containing file contents */
  صحیح n_ڈیٹا;                           /* Size of buffer allocated at zData */
  i64 i_سائز;                           /* Size of file in bytes */
};

ساخت CrashGlobal {
  Write_بفر *pWriteList;     /* Head of write-list */
  Write_بفر *p_Write_List_انتہا;  /* End of write-list */

  صحیح i_Sector_سائز;             /* Value of simulated sector size */
  صحیح iDeviceCharacteristics;  /* Value of simulated device characteristics */

  صحیح iCrash;                  /* Crash on the iCrash'th call to xSync() */
  حرف zCrashFile[500];        /* Crash during an xSync() on this file */
};

ساکن CrashGlobal g = {0, 0, SQLITE_DEFAULT_SECTOR_سائز, 0, 0};

/*
** Set this global variable to 1 to enable crash testing.
*/
ساکن صحیح sqlite3CrashTestEnable = 0;

ساکن خالی *crash_malloc(صحیح nByte){
  واپس (خالی *)Tcl_AttemptAlloc((size_t)nByte);
}
ساکن خالی crash_free(خالی *p){
  Tcl_Free(p);
}
ساکن خالی *crash_realloc(خالی *p, صحیح n){
  واپس (خالی *)Tcl_AttemptRealloc(p, (size_t)n);
}

/*
** Wrapper around the sqlite3OsWrite() function that avoids writing to the
** 512 byte block beginning at offset PENDING_BYTE.
*/
ساکن صحیح writeDbFile(CrashFile *p, u8 *z, i64 iAmt, i64 iOff){
  صحیح rc = SQLITE_OK;
  صحیح iSkip = 0;
  اگر( (iAmt-iSkip)>0 ){
    rc = sqlite3OsWrite(p->pRealFile, &z[iSkip], (صحیح)(iAmt-iSkip), iOff+iSkip);
  }
  واپس rc;
}

/*
** Flush the write-list as if xSync() had been called on file handle
** pFile. If isCrash is true, simulate a crash.
*/
ساکن صحیح writeListSync(CrashFile *pFile, صحیح isCrash){
  صحیح rc = SQLITE_OK;
  صحیح iDc = g.iDeviceCharacteristics;

  Write_بفر *pWrite;
  Write_بفر **ppPtr;

  /* If this is not a crash simulation, set pFinal to point to the
  ** last element of the write-list that is associated with file handle
  ** pFile.
  **
  ** If this is a crash simulation, set pFinal to an arbitrarily selected
  ** element of the write-list.
  */
  Write_بفر *pFinal = 0;
  اگر( !isCrash ){
    برائے(pWrite=g.pWriteList; pWrite; pWrite=pWrite->p_اگلا){
      اگر( pWrite->pFile==pFile ){
        pFinal = pWrite;
      }
    }
  }ورنہ اگر( iDc&(SQLITE_IOCAP_SEQUENTIAL|SQLITE_IOCAP_محفوظ_APPEND) ){
    صحیح nWrite = 0;
    صحیح iFinal;
    برائے(pWrite=g.pWriteList; pWrite; pWrite=pWrite->p_اگلا) nWrite++;
    sqlite3_randomness(حجم(صحیح), &iFinal);
    iFinal = ((iFinal<0)?-1*iFinal:iFinal)%nWrite;
    برائے(pWrite=g.pWriteList; iFinal>0; pWrite=pWrite->p_اگلا) iFinal--;
    pFinal = pWrite;
  }

#اگر_تعریف_شدہ TRACE_CRASHTEST
  اگر( pFile ){
    لکھو("Sync %s (is %s crash)\n", pFile->zName, (isCrash?"a":"not a"));
  }
#اگر_ختم

  ppPtr = &g.pWriteList;
  برائے(pWrite=*ppPtr; rc==SQLITE_OK && pWrite; pWrite=*ppPtr){
    sqlite3_file *pRealFile = pWrite->pFile->pRealFile;

    /* (eAction==1)      -> write block out normally,
    ** (eAction==2)      -> do nothing,
    ** (eAction==3)      -> trash sectors.
    */
    صحیح eAction = 0;
    اگر( !isCrash ){
      eAction = 2;
      اگر( (pWrite->pFile==pFile || iDc&SQLITE_IOCAP_SEQUENTIAL) ){
        eAction = 1;
      }
    }ورنہ{
      حرف random;
      sqlite3_randomness(1, &random);

      /* Do not select option 3 (sector trashing) if the IOCAP_ATOMIC flag
      ** is set or this is an OsTruncate(), not an Oswrite().
      */
      اگر( (iDc&SQLITE_IOCAP_ATOMIC) || (pWrite->zBuf==0) ){
        random &= 0x01;
      }

      /* If IOCAP_SEQUENTIAL is set and this is not the final entry
      ** in the truncated write-list, always select option 1 (write
      ** out correctly).
      */
      اگر( (iDc&SQLITE_IOCAP_SEQUENTIAL && pWrite!=pFinal) ){
        random = 0;
      }

      /* If IOCAP_SAFE_APPEND is set and this OsWrite() operation is
      ** an append (first byte of the written region is 1 byte past the
      ** current EOF), always select option 1 (write out correctly).
      */
      اگر( iDc&SQLITE_IOCAP_محفوظ_APPEND && pWrite->zBuf ){
        i64 i_سائز;
        sqlite3_Os_File_سائز(pRealFile, &i_سائز);
        اگر( i_سائز==pWrite->i_انحراف ){
          random = 0;
        }
      }

      اگر( (random&0x06)==0x06 ){
        eAction = 3;
      }ورنہ{
        eAction = ((random&0x01)?2:1);
      }
    }

    انتخاب( eAction ){
      حالت 1: {               /* Write out correctly */
        اگر( pWrite->zBuf ){
          rc = writeDbFile(
              pWrite->pFile, pWrite->zBuf, pWrite->nBuf, pWrite->i_انحراف
          );
        }ورنہ{
          rc = sqlite3OsTruncate(pRealFile, pWrite->i_انحراف);
        }
        *ppPtr = pWrite->p_اگلا;
#اگر_تعریف_شدہ TRACE_CRASHTEST
        اگر( isCrash ){
          لکھو("Writing %d bytes @ %d (%s)\n",
            pWrite->nBuf, (صحیح)pWrite->i_انحراف, pWrite->pFile->zName
          );
        }
#اگر_ختم
        crash_free(pWrite);
        توڑو;
      }
      حالت 2: {               /* Do nothing */
        ppPtr = &pWrite->p_اگلا;
#اگر_تعریف_شدہ TRACE_CRASHTEST
        اگر( isCrash ){
          لکھو("Omiting %d bytes @ %d (%s)\n",
            pWrite->nBuf, (صحیح)pWrite->i_انحراف, pWrite->pFile->zName
          );
        }
#اگر_ختم
        توڑو;
      }
      حالت 3: {               /* Trash sectors */
        u8 *zGarbage;
        صحیح i_پہلا = (صحیح)(pWrite->i_انحراف/g.i_Sector_سائز);
        صحیح i_آخری = (صحیح)((pWrite->i_انحراف+pWrite->nBuf-1)/g.i_Sector_سائز);

        أكد(pWrite->zBuf);

#اگر_تعریف_شدہ TRACE_CRASHTEST
        لکھو("Trashing %d sectors (%d bytes) @ %lld (sector %d) (%s)\n",
            1+i_آخری-i_پہلا, (1+i_آخری-i_پہلا)*g.i_Sector_سائز,
            pWrite->i_انحراف, i_پہلا, pWrite->pFile->zName
        );
#اگر_ختم

        zGarbage = crash_malloc(g.i_Sector_سائز);
        اگر( zGarbage ){
          sqlite3_int64 i;
          برائے(i=i_پہلا; rc==SQLITE_OK && i<=i_آخری; i++){
            sqlite3_randomness(g.i_Sector_سائز, zGarbage);
            rc = writeDbFile(
              pWrite->pFile, zGarbage, g.i_Sector_سائز, i*g.i_Sector_سائز
            );
          }
          crash_free(zGarbage);
        }ورنہ{
          rc = SQLITE_NOMEM;
        }

        ppPtr = &pWrite->p_اگلا;
        توڑو;
      }

      طےشدہ:
        أكد(!"Cannot happen");
    }

    اگر( pWrite==pFinal ) توڑو;
  }

  اگر( rc==SQLITE_OK && isCrash ){
    باہر_نکلو(-1);
  }

  برائے(pWrite=g.pWriteList; pWrite && pWrite->p_اگلا; pWrite=pWrite->p_اگلا);
  g.p_Write_List_انتہا = pWrite;

  واپس rc;
}

/*
** Add an entry to the end of the write-list.
*/
ساکن صحیح writeListAppend(
  sqlite3_file *pFile,
  sqlite3_int64 i_انحراف,
  ثابت u8 *zBuf,
  صحیح nBuf
){
  Write_بفر *pNew;

  أكد((zBuf && nBuf) || (!nBuf && !zBuf));

  pNew = (Write_بفر *)crash_malloc(حجم(Write_بفر) + nBuf);
  اگر( pNew==0 ){
    fprintf(stderr, "out of memory in the crash simulator\n");
  }
  memset(pNew, 0, حجم(Write_بفر)+nBuf);
  pNew->i_انحراف = i_انحراف;
  pNew->nBuf = nBuf;
  pNew->pFile = (CrashFile *)pFile;
  اگر( zBuf ){
    pNew->zBuf = (u8 *)&pNew[1];
    حافظہ_نقل(pNew->zBuf, zBuf, nBuf);
  }

  اگر( g.pWriteList ){
    أكد(g.p_Write_List_انتہا);
    g.p_Write_List_انتہا->p_اگلا = pNew;
  }ورنہ{
    g.pWriteList = pNew;
  }
  g.p_Write_List_انتہا = pNew;
 
  واپس SQLITE_OK;
}

/*
** Close a crash-file.
*/
ساکن صحیح cfClose(sqlite3_file *pFile){
  CrashFile *pCrash = (CrashFile *)pFile;
  writeListSync(pCrash, 0);
  sqlite3OsClose(pCrash->pRealFile);
  واپس SQLITE_OK;
}

/*
** Read data from a crash-file.
*/
ساکن صحیح cfRead(
  sqlite3_file *pFile,
  خالی *zBuf,
  صحیح iAmt,
  sqlite_int64 iOfst
){
  CrashFile *pCrash = (CrashFile *)pFile;
  صحیح nCopy = (صحیح)MIN((i64)iAmt, (pCrash->i_سائز - iOfst));

  اگر( nCopy>0 ){
    حافظہ_نقل(zBuf, &pCrash->z_ڈیٹا[iOfst], nCopy);
  }

  /* Check the file-size to see if this is a short-read */
  اگر( nCopy<iAmt ){
    واپس SQLITE_IOERR_SHORT_READ;
  }

  واپس SQLITE_OK;
}

/*
** Write data to a crash-file.
*/
ساکن صحیح cfWrite(
  sqlite3_file *pFile,
  ثابت خالی *zBuf,
  صحیح iAmt,
  sqlite_int64 iOfst
){
  CrashFile *pCrash = (CrashFile *)pFile;
  اگر( iAmt+iOfst>pCrash->i_سائز ){
    pCrash->i_سائز = (صحیح)(iAmt+iOfst);
  }
  جب( pCrash->i_سائز>pCrash->n_ڈیٹا ){
    u8 *zNew;
    صحیح nNew = (pCrash->n_ڈیٹا*2) + 4096;
    zNew = crash_realloc(pCrash->z_ڈیٹا, nNew);
    اگر( !zNew ){
      واپس SQLITE_NOMEM;
    }
    memset(&zNew[pCrash->n_ڈیٹا], 0, nNew-pCrash->n_ڈیٹا);
    pCrash->n_ڈیٹا = nNew;
    pCrash->z_ڈیٹا = zNew;
  }
  حافظہ_نقل(&pCrash->z_ڈیٹا[iOfst], zBuf, iAmt);
  واپس writeListAppend(pFile, iOfst, zBuf, iAmt);
}

/*
** Truncate a crash-file.
*/
ساکن صحیح cfTruncate(sqlite3_file *pFile, sqlite_int64 سائز){
  CrashFile *pCrash = (CrashFile *)pFile;
  أكد(سائز>=0);
  اگر( pCrash->i_سائز>سائز ){
    pCrash->i_سائز = (صحیح)سائز;
  }
  واپس writeListAppend(pFile, سائز, 0, 0);
}

/*
** Sync a crash-file.
*/
ساکن صحیح cfSync(sqlite3_file *pFile, صحیح flags){
  CrashFile *pCrash = (CrashFile *)pFile;
  صحیح isCrash = 0;

  ثابت حرف *zName = pCrash->zName;
  ثابت حرف *zCrashFile = g.zCrashFile;
  صحیح nName = (صحیح)لڑی_لمبائی(zName);
  صحیح nCrashFile = (صحیح)لڑی_لمبائی(zCrashFile);

  اگر( nCrashFile>0 && zCrashFile[nCrashFile-1]=='*' ){
    nCrashFile--;
    اگر( nName>nCrashFile ) nName = nCrashFile;
  }

#اگر_تعریف_شدہ TRACE_CRASHTEST
  لکھو("cfSync(): nName = %d, nCrashFile = %d, zName = %s, zCrashFile = %s\n",
         nName, nCrashFile, zName, zCrashFile);
#اگر_ختم

  اگر( nName==nCrashFile && 0==memcmp(zName, zCrashFile, nName) ){
#اگر_تعریف_شدہ TRACE_CRASHTEST
    لکھو("cfSync(): name matched, g.iCrash = %d\n", g.iCrash);
#اگر_ختم
    اگر( (--g.iCrash)==0 ) isCrash = 1;
  }

  واپس writeListSync(pCrash, isCrash);
}

/*
** Return the current file-size of the crash-file.
*/
ساکن صحیح cf_File_سائز(sqlite3_file *pFile, sqlite_int64 *p_سائز){
  CrashFile *pCrash = (CrashFile *)pFile;
  *p_سائز = (i64)pCrash->i_سائز;
  واپس SQLITE_OK;
}

/*
** Calls related to file-locks are passed on to the real file handle.
*/
ساکن صحیح cf_تالا(sqlite3_file *pFile, صحیح e_تالا){
  واپس sqlite3_Os_تالا(((CrashFile *)pFile)->pRealFile, e_تالا);
}
ساکن صحیح cfUnlock(sqlite3_file *pFile, صحیح e_تالا){
  واپس sqlite3OsUnlock(((CrashFile *)pFile)->pRealFile, e_تالا);
}
ساکن صحیح cf_Check_Reserved_تالا(sqlite3_file *pFile, صحیح *p_Res_برآمد){
  واپس sqlite3_Os_Check_Reserved_تالا(((CrashFile *)pFile)->pRealFile, p_Res_برآمد);
}
ساکن صحیح cfFileControl(sqlite3_file *pFile, صحیح op, خالی *pArg){
  اگر( op==SQLITE_FCNTL_سائز_HINT ){
    CrashFile *pCrash = (CrashFile *)pFile;
    i64 nByte = *(i64 *)pArg;
    اگر( nByte>pCrash->i_سائز ){
      اگر( SQLITE_OK==writeListAppend(pFile, nByte, 0, 0) ){
        pCrash->i_سائز = (صحیح)nByte;
      }
    }
    واپس SQLITE_OK;
  }
  واپس sqlite3OsFileControl(((CrashFile *)pFile)->pRealFile, op, pArg);
}

/*
** The xSectorSize() and xDeviceCharacteristics() functions return
** the global values configured by the [sqlite_crashparams] tcl
*  interface.
*/
ساکن صحیح cf_Sector_سائز(sqlite3_file *pFile){
  واپس g.i_Sector_سائز;
}
ساکن صحیح cfDeviceCharacteristics(sqlite3_file *pFile){
  واپس g.iDeviceCharacteristics;
}

/*
** Pass-throughs for WAL support.
*/
ساکن صحیح cf_Shm_تالا(sqlite3_file *pFile, صحیح ofst, صحیح n, صحیح flags){
  sqlite3_file *pReal = ((CrashFile*)pFile)->pRealFile;
  واپس pReal->pMethods->x_Shm_تالا(pReal, ofst, n, flags);
}
ساکن خالی cfShmBarrier(sqlite3_file *pFile){
  sqlite3_file *pReal = ((CrashFile*)pFile)->pRealFile;
  pReal->pMethods->xShmBarrier(pReal);
}
ساکن صحیح cfShmUnmap(sqlite3_file *pFile, صحیح delFlag){
  sqlite3_file *pReal = ((CrashFile*)pFile)->pRealFile;
  واپس pReal->pMethods->xShmUnmap(pReal, delFlag);
}
ساکن صحیح cfShmMap(
  sqlite3_file *pFile,            /* Handle open on database file */
  صحیح iRegion,                    /* Region to retrieve */
  صحیح sz,                         /* Size of regions */
  صحیح w,                          /* True to extend file if necessary */
  خالی متلون **pp              /* OUT: Mapped memory */
){
  sqlite3_file *pReal = ((CrashFile*)pFile)->pRealFile;
  واپس pReal->pMethods->xShmMap(pReal, iRegion, sz, w, pp);
}

ساکن ثابت sqlite3_io_methods CrashFileVtab = {
  2,                            /* iVersion */
  cfClose,                      /* xClose */
  cfRead,                       /* xRead */
  cfWrite,                      /* xWrite */
  cfTruncate,                   /* xTruncate */
  cfSync,                       /* xSync */
  cf_File_سائز,                   /* xFileSize */
  cf_تالا,                       /* xLock */
  cfUnlock,                     /* xUnlock */
  cf_Check_Reserved_تالا,          /* xCheckReservedLock */
  cfFileControl,                /* xFileControl */
  cf_Sector_سائز,                 /* xSectorSize */
  cfDeviceCharacteristics,      /* xDeviceCharacteristics */
  cfShmMap,                     /* xShmMap */
  cf_Shm_تالا,                    /* xShmLock */
  cfShmBarrier,                 /* xShmBarrier */
  cfShmUnmap                    /* xShmUnmap */
};

/*
** Application data for the crash VFS
*/
ساخت crash_App_ڈیٹا {
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
ساکن صحیح cfOpen(
  sqlite3_vfs *pCfVfs,
  ثابت حرف *zName,
  sqlite3_file *pFile,
  صحیح flags,
  صحیح *p_برآمد_Flags
){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_ڈیٹا;
  صحیح rc;
  CrashFile *pWrapper = (CrashFile *)pFile;
  sqlite3_file *pReal = (sqlite3_file*)&pWrapper[1];

  memset(pWrapper, 0, حجم(CrashFile));
  rc = sqlite3OsOpen(pVfs, zName, pReal, flags, p_برآمد_Flags);

  اگر( rc==SQLITE_OK ){
    i64 i_سائز;
    pWrapper->p_طریقہ = &CrashFileVtab;
    pWrapper->zName = (حرف *)zName;
    pWrapper->pRealFile = pReal;
    rc = sqlite3_Os_File_سائز(pReal, &i_سائز);
    pWrapper->i_سائز = (صحیح)i_سائز;
    pWrapper->flags = flags;
  }
  اگر( rc==SQLITE_OK ){
    pWrapper->n_ڈیٹا = (صحیح)(4096 + pWrapper->i_سائز);
    pWrapper->z_ڈیٹا = crash_malloc(pWrapper->n_ڈیٹا);
    اگر( pWrapper->z_ڈیٹا ){
      /* os_unix.c contains an assert() that fails if the caller attempts
      ** to read data from the 512-byte locking region of a file opened
      ** with the SQLITE_OPEN_MAIN_DB flag. This region of a database file
      ** never contains valid data anyhow. So avoid doing such a read here.
      **
      ** UPDATE: It also contains an assert() verifying that each call
      ** to the xRead() method reads less than 128KB of data.
      */
      i64 iOff;

      memset(pWrapper->z_ڈیٹا, 0, pWrapper->n_ڈیٹا);
      برائے(iOff=0; iOff<pWrapper->i_سائز; iOff += 512){
        صحیح nRead = (صحیح)(pWrapper->i_سائز - iOff);
        اگر( nRead>512 ) nRead = 512;
        rc = sqlite3OsRead(pReal, &pWrapper->z_ڈیٹا[iOff], nRead, iOff);
      }
    }ورنہ{
      rc = SQLITE_NOMEM;
    }
  }
  اگر( rc!=SQLITE_OK && pWrapper->p_طریقہ ){
    sqlite3OsClose(pFile);
  }
  واپس rc;
}

ساکن صحیح cfDelete(sqlite3_vfs *pCfVfs, ثابت حرف *z_راستہ, صحیح dirSync){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_ڈیٹا;
  واپس pVfs->xDelete(pVfs, z_راستہ, dirSync);
}
ساکن صحیح cfAccess(
  sqlite3_vfs *pCfVfs,
  ثابت حرف *z_راستہ,
  صحیح flags,
  صحیح *p_Res_برآمد
){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_ڈیٹا;
  واپس pVfs->xAccess(pVfs, z_راستہ, flags, p_Res_برآمد);
}
ساکن صحیح cf_بھرا_Pathname(
  sqlite3_vfs *pCfVfs,
  ثابت حرف *z_راستہ,
  صحیح n_راستہ_برآمد,
  حرف *z_راستہ_برآمد
){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_ڈیٹا;
  واپس pVfs->x_بھرا_Pathname(pVfs, z_راستہ, n_راستہ_برآمد, z_راستہ_برآمد);
}
ساکن خالی *cfDlOpen(sqlite3_vfs *pCfVfs, ثابت حرف *z_راستہ){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_ڈیٹا;
  واپس pVfs->xDlOpen(pVfs, z_راستہ);
}
ساکن خالی cfDlError(sqlite3_vfs *pCfVfs, صحیح nByte, حرف *z_Err_پیغام){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_ڈیٹا;
  pVfs->xDlError(pVfs, nByte, z_Err_پیغام);
}
ساکن خالی (*cfDlSym(sqlite3_vfs *pCfVfs, خالی *pH, ثابت حرف *zSym))(خالی){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_ڈیٹا;
  واپس pVfs->xDlSym(pVfs, pH, zSym);
}
ساکن خالی cfDlClose(sqlite3_vfs *pCfVfs, خالی *pHandle){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_ڈیٹا;
  pVfs->xDlClose(pVfs, pHandle);
}
ساکن صحیح cfRandomness(sqlite3_vfs *pCfVfs, صحیح nByte, حرف *z_Buf_برآمد){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_ڈیٹا;
  واپس pVfs->xRandomness(pVfs, nByte, z_Buf_برآمد);
}
ساکن صحیح cfSleep(sqlite3_vfs *pCfVfs, صحیح nMicro){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_ڈیٹا;
  واپس pVfs->xSleep(pVfs, nMicro);
}
ساکن صحیح cf_Current_وقت(sqlite3_vfs *pCfVfs, دوہرا *p_وقت_برآمد){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_ڈیٹا;
  واپس pVfs->x_Current_وقت(pVfs, p_وقت_برآمد);
}
ساکن صحیح cf_Get_آخری_Error(sqlite3_vfs *pCfVfs, صحیح n, حرف *z){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_ڈیٹا;
  واپس pVfs->x_Get_آخری_Error(pVfs, n, z);
}

ساکن صحیح process_Dev_Sym_دلائل(
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[],
  صحیح *piDeviceChar,
  صحیح *pi_Sector_سائز
){
  ساخت DeviceFlag {
    حرف *zName;
    صحیح i_قدر;
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
    { "safe_append",         SQLITE_IOCAP_محفوظ_APPEND           },
    { "powersafe_overwrite", SQLITE_IOCAP_POWERSAFE_OVERWRITE   },
    { "batch-atomic",        SQLITE_IOCAP_BATCH_ATOMIC          },
    { 0, 0 }
  };

  صحیح i;
  صحیح iDc = 0;
  صحیح i_Sector_سائز = 0;
  صحیح سیٹ_Sectorsize = 0;
  صحیح سیٹ_Device_Char = 0;

  برائے(i=0; i<objc; i+=2){
    Tcl_سائز nOpt;
    حرف *zOpt = Tcl_GetStringFromObj(objv[i], &nOpt);

    اگر( (nOpt>11 || nOpt<2 || لڑی_موازنہ("-sectorsize", zOpt, nOpt))
     && (nOpt>16 || nOpt<2 || لڑی_موازنہ("-characteristics", zOpt, nOpt))
    ){
      Tcl_Append_نتیجہ(interp,
        "Bad option: \"", zOpt,
        "\" - must be \"-characteristics\" or \"-sectorsize\"", تہی
      );
      واپس TCL_ERROR;
    }
    اگر( i==objc-1 ){
      Tcl_Append_نتیجہ(interp, "Option requires an argument: \"", zOpt, "\"", تہی);
      واپس TCL_ERROR;
    }

    اگر( zOpt[1]=='s' ){
      اگر( Tcl_GetIntFromObj(interp, objv[i+1], &i_Sector_سائز) ){
        واپس TCL_ERROR;
      }
      سیٹ_Sectorsize = 1;
    }ورنہ{
      صحیح j;
      Tcl_Obj **apObj;
      Tcl_سائز nObj;
      اگر( Tcl_ListObjGetElements(interp, objv[i+1], &nObj, &apObj) ){
        واپس TCL_ERROR;
      }
      برائے(j=0; j<(صحیح)nObj; j++){
        صحیح rc;
        صحیح iChoice;
        Tcl_Obj *pFlag = Tcl_DuplicateObj(apObj[j]);
        Tcl_Incr_Ref_گنتی(pFlag);
        Tcl_Utf_مقصد_زیریں(Tcl_GetString(pFlag));

        rc = Tcl_Get_اشاریہ_From_Obj_Struct(
            interp, pFlag, aFlag, حجم(aFlag[0]), "no such flag", 0, &iChoice
        );
        Tcl_Decr_Ref_گنتی(pFlag);
        اگر( rc ){
          واپس TCL_ERROR;
        }

        iDc |= aFlag[iChoice].i_قدر;
      }
      سیٹ_Device_Char = 1;
    }
  }

  اگر( سیٹ_Device_Char ){
    *piDeviceChar = iDc;
  }
  اگر( سیٹ_Sectorsize ){
    *pi_Sector_سائز = i_Sector_سائز;
  }

  واپس TCL_OK;
}

/*
** tclcmd:   sqlite3_crash_now
**
** Simulate a crash immediately. This function does not return
** (writeListSync() calls exit(-1)).
*/
ساکن صحیح SQLITE_TCLAPI crashNowCmd(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  اگر( objc!=1 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "");
    واپس TCL_ERROR;
  }
  writeListSync(0, 1);
  أكد( 0 );
  واپس TCL_OK;
}

/*
** tclcmd:   sqlite_crash_enable ENABLE ?DEFAULT?
**
** Parameter ENABLE must be a boolean value. If true, then the "crash"
** vfs is added to the system. If false, it is removed.
*/
ساکن صحیح SQLITE_TCLAPI crashEnableCmd(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح isEnable;
  صحیح isDefault = 0;
  ساکن sqlite3_vfs crashVfs = {
    2,                  /* iVersion */
    0,                  /* szOsFile */
    0,                  /* mxPathname */
    0,                  /* pNext */
    "crash",            /* zName */
    0,                  /* pAppData */
 
    cfOpen,               /* xOpen */
    cfDelete,             /* xDelete */
    cfAccess,             /* xAccess */
    cf_بھرا_Pathname,       /* xFullPathname */
    cfDlOpen,             /* xDlOpen */
    cfDlError,            /* xDlError */
    cfDlSym,              /* xDlSym */
    cfDlClose,            /* xDlClose */
    cfRandomness,         /* xRandomness */
    cfSleep,              /* xSleep */
    cf_Current_وقت,        /* xCurrentTime */
    cf_Get_آخری_Error,       /* xGetLastError */
    0,                    /* xCurrentTimeInt64 */
  };

  اگر( objc!=2 && objc!=3 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "ENABLE ?DEFAULT?");
    واپس TCL_ERROR;
  }

  اگر( Tcl_GetBooleanFromObj(interp, objv[1], &isEnable) ){
    واپس TCL_ERROR;
  }
  اگر( objc==3 && Tcl_GetBooleanFromObj(interp, objv[2], &isDefault) ){
    واپس TCL_ERROR;
  }

  اگر( (isEnable && crashVfs.p_App_ڈیٹا) || (!isEnable && !crashVfs.p_App_ڈیٹا) ){
    واپس TCL_OK;
  }

  اگر( crashVfs.p_App_ڈیٹا==0 ){
    sqlite3_vfs *pOriginalVfs = sqlite3_vfs_ڈھونڈو(0);
    crashVfs.mxPathname = pOriginalVfs->mxPathname;
    crashVfs.p_App_ڈیٹا = (خالی *)pOriginalVfs;
    crashVfs.szOsFile = حجم(CrashFile) + pOriginalVfs->szOsFile;
    sqlite3_vfs_رجسٹر(&crashVfs, isDefault);
  }ورنہ{
    crashVfs.p_App_ڈیٹا = 0;
    sqlite3_vfs_unregister(&crashVfs);
  }

  واپس TCL_OK;
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
ساکن صحیح SQLITE_TCLAPI crashParamsObjCmd(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح iDelay;
  ثابت حرف *zCrashFile;
  Tcl_سائز nCrashFile;
  صحیح iDc, i_Sector_سائز;

  iDc = -1;
  i_Sector_سائز = -1;

  اگر( objc<3 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "?OPTIONS? DELAY CRASHFILE");
    جاؤ error;
  }

  zCrashFile = Tcl_GetStringFromObj(objv[objc-1], &nCrashFile);
  اگر( nCrashFile>=حجم(g.zCrashFile) ){
    Tcl_Append_نتیجہ(interp, "Filename is too long: \"", zCrashFile, "\"", تہی);
    جاؤ error;
  }
  اگر( Tcl_GetIntFromObj(interp, objv[objc-2], &iDelay) ){
    جاؤ error;
  }

  اگر( process_Dev_Sym_دلائل(interp, objc-3, &objv[1], &iDc, &i_Sector_سائز) ){
    واپس TCL_ERROR;
  }

  اگر( iDc>=0 ){
    g.iDeviceCharacteristics = iDc;
  }
  اگر( i_Sector_سائز>=0 ){
    g.i_Sector_سائز = i_Sector_سائز;
  }

  g.iCrash = iDelay;
  حافظہ_نقل(g.zCrashFile, zCrashFile, nCrashFile+1);
  sqlite3CrashTestEnable = 1;
  واپس TCL_OK;

error:
  واپس TCL_ERROR;
}

ساکن صحیح SQLITE_TCLAPI devSymObjCmd(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  خالی devsym_رجسٹر(صحیح iDeviceChar, صحیح i_Sector_سائز);

  صحیح iDc = -1;
  صحیح i_Sector_سائز = -1;

  اگر( process_Dev_Sym_دلائل(interp, objc-1, &objv[1], &iDc, &i_Sector_سائز) ){
    واپس TCL_ERROR;
  }
  devsym_رجسٹر(iDc, i_Sector_سائز);

  واپس TCL_OK;
}

/*
** tclcmd: sqlite3_crash_on_write N
*/
ساکن صحیح SQLITE_TCLAPI writeCrashObjCmd(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  خالی devsym_crash_on_write(صحیح);
  صحیح nWrite = 0;

  اگر( objc!=2 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "NWRITE");
    واپس TCL_ERROR;
  }
  اگر( Tcl_GetIntFromObj(interp, objv[1], &nWrite) ){
    واپس TCL_ERROR;
  }

  devsym_crash_on_write(nWrite);
  واپس TCL_OK;
}

/*
** tclcmd: unregister_devsim
*/
ساکن صحیح SQLITE_TCLAPI dsUnregisterObjCmd(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  خالی devsym_unregister(خالی);

  اگر( objc!=1 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "");
    واپس TCL_ERROR;
  }

  devsym_unregister();
  واپس TCL_OK;
}

/*
** tclcmd: register_jt_vfs ?-default? PARENT-VFS
*/
ساکن صحیح SQLITE_TCLAPI jtObjCmd(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح jt_رجسٹر(حرف *, صحیح);
  حرف *z_والد = 0;

  اگر( objc!=2 && objc!=3 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "?-default? PARENT-VFS");
    واپس TCL_ERROR;
  }
  z_والد = Tcl_GetString(objv[1]);
  اگر( objc==3 ){
    اگر( strcmp(z_والد, "-default") ){
      Tcl_Append_نتیجہ(interp,
          "bad option \"", z_والد, "\": must be -default", تہی
      );
      واپس TCL_ERROR;
    }
    z_والد = Tcl_GetString(objv[2]);
  }

  اگر( !(*z_والد) ){
    z_والد = 0;
  }
  اگر( jt_رجسٹر(z_والد, objc==3) ){
    Tcl_Append_نتیجہ(interp, "Error in jt_register", تہی);
    واپس TCL_ERROR;
  }

  واپس TCL_OK;
}

/*
** tclcmd: unregister_jt_vfs
*/
ساکن صحیح SQLITE_TCLAPI jtUnregisterObjCmd(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  خالی jt_unregister(خالی);

  اگر( objc!=1 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "");
    واپس TCL_ERROR;
  }

  jt_unregister();
  واپس TCL_OK;
}

#اگر_ختم /* SQLITE_OMIT_DISKIO */

/*
** This procedure registers the TCL procedures defined in this file.
*/
صحیح Sqlitetest6_Init(Tcl_Interp *interp){
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_DISKIO
  Tcl_CreateObjCommand(interp, "sqlite3_crash_enable", crashEnableCmd, 0, 0);
  Tcl_CreateObjCommand(interp, "sqlite3_crashparams", crashParamsObjCmd, 0, 0);
  Tcl_CreateObjCommand(interp, "sqlite3_crash_now", crashNowCmd, 0, 0);
  Tcl_CreateObjCommand(interp, "sqlite3_simulate_device", devSymObjCmd, 0, 0);
  Tcl_CreateObjCommand(interp, "sqlite3_crash_on_write", writeCrashObjCmd,0,0);
  Tcl_CreateObjCommand(interp, "unregister_devsim", dsUnregisterObjCmd, 0, 0);
  Tcl_CreateObjCommand(interp, "register_jt_vfs", jtObjCmd, 0, 0);
  Tcl_CreateObjCommand(interp, "unregister_jt_vfs", jtUnregisterObjCmd, 0, 0);
#اگر_ختم
  واپس TCL_OK;
}

#اگر_ختم /* SQLITE_TEST */
