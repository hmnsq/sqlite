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
#إذا SQLITE_TEST          /* This file is used for testing only */
#ضمن "sqliteInt.h"
#ضمن "tclsqlite.h"

#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_DISKIO  /* This file is a no-op if disk I/O is disabled */

/* #define TRACE_CRASHTEST */

تعريفنوع بنية CrashFile CrashFile;
تعريفنوع بنية CrashGlobal CrashGlobal;
تعريفنوع بنية Write_بفر Write_بفر;

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
بنية Write_بفر {
  i64 i_وٿي;                 /* Byte offset of the start of this write() */
  صحيح nBuf;                    /* Number of bytes written */
  u8 *zBuf;                    /* Pointer to copy of written data */
  CrashFile *pFile;            /* File this write() applies to */

  Write_بفر *p_ايندڙ;          /* Next in CrashGlobal.pWriteList */
};

بنية CrashFile {
  ثابت sqlite3_io_methods *p_طريقو;   /* Must be first */
  sqlite3_file *pRealFile;             /* Underlying "real" file handle */
  حرف *zName;
  صحيح flags;                           /* Flags the file was opened with */

  /* Cache of the entire file. This is used to speed up OsRead() and
  ** OsFileSize() calls. Although both could be done by traversing the
  ** write-list, in practice this is impractically slow.
  */
  u8 *z_معلومات;                           /* Buffer containing file contents */
  صحيح n_معلومات;                           /* Size of buffer allocated at zData */
  i64 i_سائيز;                           /* Size of file in bytes */
};

بنية CrashGlobal {
  Write_بفر *pWriteList;     /* Head of write-list */
  Write_بفر *p_Write_List_پڇاڙي;  /* End of write-list */

  صحيح i_Sector_سائيز;             /* Value of simulated sector size */
  صحيح iDeviceCharacteristics;  /* Value of simulated device characteristics */

  صحيح iCrash;                  /* Crash on the iCrash'th call to xSync() */
  حرف zCrashFile[500];        /* Crash during an xSync() on this file */
};

ساكن CrashGlobal g = {0, 0, SQLITE_DEFAULT_SECTOR_سائيز, 0, 0};

/*
** Set this global variable to 1 to enable crash testing.
*/
ساكن صحيح sqlite3CrashTestEnable = 0;

ساكن فراغ *crash_malloc(صحيح nByte){
  أرجع (فراغ *)Tcl_AttemptAlloc((size_t)nByte);
}
ساكن فراغ crash_free(فراغ *p){
  Tcl_Free(p);
}
ساكن فراغ *crash_realloc(فراغ *p, صحيح n){
  أرجع (فراغ *)Tcl_AttemptRealloc(p, (size_t)n);
}

/*
** Wrapper around the sqlite3OsWrite() function that avoids writing to the
** 512 byte block beginning at offset PENDING_BYTE.
*/
ساكن صحيح writeDbFile(CrashFile *p, u8 *z, i64 iAmt, i64 iOff){
  صحيح rc = SQLITE_OK;
  صحيح iSkip = 0;
  إذا( (iAmt-iSkip)>0 ){
    rc = sqlite3OsWrite(p->pRealFile, &z[iSkip], (صحيح)(iAmt-iSkip), iOff+iSkip);
  }
  أرجع rc;
}

/*
** Flush the write-list as if xSync() had been called on file handle
** pFile. If isCrash is true, simulate a crash.
*/
ساكن صحيح writeListSync(CrashFile *pFile, صحيح isCrash){
  صحيح rc = SQLITE_OK;
  صحيح iDc = g.iDeviceCharacteristics;

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
  إذا( !isCrash ){
    لكل(pWrite=g.pWriteList; pWrite; pWrite=pWrite->p_ايندڙ){
      إذا( pWrite->pFile==pFile ){
        pFinal = pWrite;
      }
    }
  }وإلا إذا( iDc&(SQLITE_IOCAP_SEQUENTIAL|SQLITE_IOCAP_محفوظ_APPEND) ){
    صحيح nWrite = 0;
    صحيح iFinal;
    لكل(pWrite=g.pWriteList; pWrite; pWrite=pWrite->p_ايندڙ) nWrite++;
    sqlite3_randomness(حجم(صحيح), &iFinal);
    iFinal = ((iFinal<0)?-1*iFinal:iFinal)%nWrite;
    لكل(pWrite=g.pWriteList; iFinal>0; pWrite=pWrite->p_ايندڙ) iFinal--;
    pFinal = pWrite;
  }

#جيڪڏهن_وصف_ٿيل TRACE_CRASHTEST
  إذا( pFile ){
    لکو("Sync %s (is %s crash)\n", pFile->zName, (isCrash?"a":"not a"));
  }
#جيڪڏهن_پورو

  ppPtr = &g.pWriteList;
  لكل(pWrite=*ppPtr; rc==SQLITE_OK && pWrite; pWrite=*ppPtr){
    sqlite3_file *pRealFile = pWrite->pFile->pRealFile;

    /* (eAction==1)      -> write block out normally,
    ** (eAction==2)      -> do nothing,
    ** (eAction==3)      -> trash sectors.
    */
    صحيح eAction = 0;
    إذا( !isCrash ){
      eAction = 2;
      إذا( (pWrite->pFile==pFile || iDc&SQLITE_IOCAP_SEQUENTIAL) ){
        eAction = 1;
      }
    }وإلا{
      حرف random;
      sqlite3_randomness(1, &random);

      /* Do not select option 3 (sector trashing) if the IOCAP_ATOMIC flag
      ** is set or this is an OsTruncate(), not an Oswrite().
      */
      إذا( (iDc&SQLITE_IOCAP_ATOMIC) || (pWrite->zBuf==0) ){
        random &= 0x01;
      }

      /* If IOCAP_SEQUENTIAL is set and this is not the final entry
      ** in the truncated write-list, always select option 1 (write
      ** out correctly).
      */
      إذا( (iDc&SQLITE_IOCAP_SEQUENTIAL && pWrite!=pFinal) ){
        random = 0;
      }

      /* If IOCAP_SAFE_APPEND is set and this OsWrite() operation is
      ** an append (first byte of the written region is 1 byte past the
      ** current EOF), always select option 1 (write out correctly).
      */
      إذا( iDc&SQLITE_IOCAP_محفوظ_APPEND && pWrite->zBuf ){
        i64 i_سائيز;
        sqlite3_Os_File_سائيز(pRealFile, &i_سائيز);
        إذا( i_سائيز==pWrite->i_وٿي ){
          random = 0;
        }
      }

      إذا( (random&0x06)==0x06 ){
        eAction = 3;
      }وإلا{
        eAction = ((random&0x01)?2:1);
      }
    }

    بدل( eAction ){
      حالة 1: {               /* Write out correctly */
        إذا( pWrite->zBuf ){
          rc = writeDbFile(
              pWrite->pFile, pWrite->zBuf, pWrite->nBuf, pWrite->i_وٿي
          );
        }وإلا{
          rc = sqlite3OsTruncate(pRealFile, pWrite->i_وٿي);
        }
        *ppPtr = pWrite->p_ايندڙ;
#جيڪڏهن_وصف_ٿيل TRACE_CRASHTEST
        إذا( isCrash ){
          لکو("Writing %d bytes @ %d (%s)\n",
            pWrite->nBuf, (صحيح)pWrite->i_وٿي, pWrite->pFile->zName
          );
        }
#جيڪڏهن_پورو
        crash_free(pWrite);
        اكسر;
      }
      حالة 2: {               /* Do nothing */
        ppPtr = &pWrite->p_ايندڙ;
#جيڪڏهن_وصف_ٿيل TRACE_CRASHTEST
        إذا( isCrash ){
          لکو("Omiting %d bytes @ %d (%s)\n",
            pWrite->nBuf, (صحيح)pWrite->i_وٿي, pWrite->pFile->zName
          );
        }
#جيڪڏهن_پورو
        اكسر;
      }
      حالة 3: {               /* Trash sectors */
        u8 *zGarbage;
        صحيح i_پهريون = (صحيح)(pWrite->i_وٿي/g.i_Sector_سائيز);
        صحيح i_آخري = (صحيح)((pWrite->i_وٿي+pWrite->nBuf-1)/g.i_Sector_سائيز);

        أكد(pWrite->zBuf);

#جيڪڏهن_وصف_ٿيل TRACE_CRASHTEST
        لکو("Trashing %d sectors (%d bytes) @ %lld (sector %d) (%s)\n",
            1+i_آخري-i_پهريون, (1+i_آخري-i_پهريون)*g.i_Sector_سائيز,
            pWrite->i_وٿي, i_پهريون, pWrite->pFile->zName
        );
#جيڪڏهن_پورو

        zGarbage = crash_malloc(g.i_Sector_سائيز);
        إذا( zGarbage ){
          sqlite3_int64 i;
          لكل(i=i_پهريون; rc==SQLITE_OK && i<=i_آخري; i++){
            sqlite3_randomness(g.i_Sector_سائيز, zGarbage);
            rc = writeDbFile(
              pWrite->pFile, zGarbage, g.i_Sector_سائيز, i*g.i_Sector_سائيز
            );
          }
          crash_free(zGarbage);
        }وإلا{
          rc = SQLITE_NOMEM;
        }

        ppPtr = &pWrite->p_ايندڙ;
        اكسر;
      }

      افتراضي:
        أكد(!"Cannot happen");
    }

    إذا( pWrite==pFinal ) اكسر;
  }

  إذا( rc==SQLITE_OK && isCrash ){
    ٻاهر_نڪرو(-1);
  }

  لكل(pWrite=g.pWriteList; pWrite && pWrite->p_ايندڙ; pWrite=pWrite->p_ايندڙ);
  g.p_Write_List_پڇاڙي = pWrite;

  أرجع rc;
}

/*
** Add an entry to the end of the write-list.
*/
ساكن صحيح writeListAppend(
  sqlite3_file *pFile,
  sqlite3_int64 i_وٿي,
  ثابت u8 *zBuf,
  صحيح nBuf
){
  Write_بفر *pNew;

  أكد((zBuf && nBuf) || (!nBuf && !zBuf));

  pNew = (Write_بفر *)crash_malloc(حجم(Write_بفر) + nBuf);
  إذا( pNew==0 ){
    fprintf(stderr, "out of memory in the crash simulator\n");
  }
  memset(pNew, 0, حجم(Write_بفر)+nBuf);
  pNew->i_وٿي = i_وٿي;
  pNew->nBuf = nBuf;
  pNew->pFile = (CrashFile *)pFile;
  إذا( zBuf ){
    pNew->zBuf = (u8 *)&pNew[1];
    يادگيري_نقل(pNew->zBuf, zBuf, nBuf);
  }

  إذا( g.pWriteList ){
    أكد(g.p_Write_List_پڇاڙي);
    g.p_Write_List_پڇاڙي->p_ايندڙ = pNew;
  }وإلا{
    g.pWriteList = pNew;
  }
  g.p_Write_List_پڇاڙي = pNew;
 
  أرجع SQLITE_OK;
}

/*
** Close a crash-file.
*/
ساكن صحيح cfClose(sqlite3_file *pFile){
  CrashFile *pCrash = (CrashFile *)pFile;
  writeListSync(pCrash, 0);
  sqlite3OsClose(pCrash->pRealFile);
  أرجع SQLITE_OK;
}

/*
** Read data from a crash-file.
*/
ساكن صحيح cfRead(
  sqlite3_file *pFile,
  فراغ *zBuf,
  صحيح iAmt,
  sqlite_int64 iOfst
){
  CrashFile *pCrash = (CrashFile *)pFile;
  صحيح nCopy = (صحيح)MIN((i64)iAmt, (pCrash->i_سائيز - iOfst));

  إذا( nCopy>0 ){
    يادگيري_نقل(zBuf, &pCrash->z_معلومات[iOfst], nCopy);
  }

  /* Check the file-size to see if this is a short-read */
  إذا( nCopy<iAmt ){
    أرجع SQLITE_IOERR_SHORT_READ;
  }

  أرجع SQLITE_OK;
}

/*
** Write data to a crash-file.
*/
ساكن صحيح cfWrite(
  sqlite3_file *pFile,
  ثابت فراغ *zBuf,
  صحيح iAmt,
  sqlite_int64 iOfst
){
  CrashFile *pCrash = (CrashFile *)pFile;
  إذا( iAmt+iOfst>pCrash->i_سائيز ){
    pCrash->i_سائيز = (صحيح)(iAmt+iOfst);
  }
  طالما( pCrash->i_سائيز>pCrash->n_معلومات ){
    u8 *zNew;
    صحيح nNew = (pCrash->n_معلومات*2) + 4096;
    zNew = crash_realloc(pCrash->z_معلومات, nNew);
    إذا( !zNew ){
      أرجع SQLITE_NOMEM;
    }
    memset(&zNew[pCrash->n_معلومات], 0, nNew-pCrash->n_معلومات);
    pCrash->n_معلومات = nNew;
    pCrash->z_معلومات = zNew;
  }
  يادگيري_نقل(&pCrash->z_معلومات[iOfst], zBuf, iAmt);
  أرجع writeListAppend(pFile, iOfst, zBuf, iAmt);
}

/*
** Truncate a crash-file.
*/
ساكن صحيح cfTruncate(sqlite3_file *pFile, sqlite_int64 سائيز){
  CrashFile *pCrash = (CrashFile *)pFile;
  أكد(سائيز>=0);
  إذا( pCrash->i_سائيز>سائيز ){
    pCrash->i_سائيز = (صحيح)سائيز;
  }
  أرجع writeListAppend(pFile, سائيز, 0, 0);
}

/*
** Sync a crash-file.
*/
ساكن صحيح cfSync(sqlite3_file *pFile, صحيح flags){
  CrashFile *pCrash = (CrashFile *)pFile;
  صحيح isCrash = 0;

  ثابت حرف *zName = pCrash->zName;
  ثابت حرف *zCrashFile = g.zCrashFile;
  صحيح nName = (صحيح)لڙي_ڊيگهه(zName);
  صحيح nCrashFile = (صحيح)لڙي_ڊيگهه(zCrashFile);

  إذا( nCrashFile>0 && zCrashFile[nCrashFile-1]=='*' ){
    nCrashFile--;
    إذا( nName>nCrashFile ) nName = nCrashFile;
  }

#جيڪڏهن_وصف_ٿيل TRACE_CRASHTEST
  لکو("cfSync(): nName = %d, nCrashFile = %d, zName = %s, zCrashFile = %s\n",
         nName, nCrashFile, zName, zCrashFile);
#جيڪڏهن_پورو

  إذا( nName==nCrashFile && 0==memcmp(zName, zCrashFile, nName) ){
#جيڪڏهن_وصف_ٿيل TRACE_CRASHTEST
    لکو("cfSync(): name matched, g.iCrash = %d\n", g.iCrash);
#جيڪڏهن_پورو
    إذا( (--g.iCrash)==0 ) isCrash = 1;
  }

  أرجع writeListSync(pCrash, isCrash);
}

/*
** Return the current file-size of the crash-file.
*/
ساكن صحيح cf_File_سائيز(sqlite3_file *pFile, sqlite_int64 *p_سائيز){
  CrashFile *pCrash = (CrashFile *)pFile;
  *p_سائيز = (i64)pCrash->i_سائيز;
  أرجع SQLITE_OK;
}

/*
** Calls related to file-locks are passed on to the real file handle.
*/
ساكن صحيح cf_تالو(sqlite3_file *pFile, صحيح e_تالو){
  أرجع sqlite3_Os_تالو(((CrashFile *)pFile)->pRealFile, e_تالو);
}
ساكن صحيح cfUnlock(sqlite3_file *pFile, صحيح e_تالو){
  أرجع sqlite3OsUnlock(((CrashFile *)pFile)->pRealFile, e_تالو);
}
ساكن صحيح cf_Check_Reserved_تالو(sqlite3_file *pFile, صحيح *p_Res_ٻاهريون){
  أرجع sqlite3_Os_Check_Reserved_تالو(((CrashFile *)pFile)->pRealFile, p_Res_ٻاهريون);
}
ساكن صحيح cfFileControl(sqlite3_file *pFile, صحيح op, فراغ *pArg){
  إذا( op==SQLITE_FCNTL_سائيز_HINT ){
    CrashFile *pCrash = (CrashFile *)pFile;
    i64 nByte = *(i64 *)pArg;
    إذا( nByte>pCrash->i_سائيز ){
      إذا( SQLITE_OK==writeListAppend(pFile, nByte, 0, 0) ){
        pCrash->i_سائيز = (صحيح)nByte;
      }
    }
    أرجع SQLITE_OK;
  }
  أرجع sqlite3OsFileControl(((CrashFile *)pFile)->pRealFile, op, pArg);
}

/*
** The xSectorSize() and xDeviceCharacteristics() functions return
** the global values configured by the [sqlite_crashparams] tcl
*  interface.
*/
ساكن صحيح cf_Sector_سائيز(sqlite3_file *pFile){
  أرجع g.i_Sector_سائيز;
}
ساكن صحيح cfDeviceCharacteristics(sqlite3_file *pFile){
  أرجع g.iDeviceCharacteristics;
}

/*
** Pass-throughs for WAL support.
*/
ساكن صحيح cf_Shm_تالو(sqlite3_file *pFile, صحيح ofst, صحيح n, صحيح flags){
  sqlite3_file *pReal = ((CrashFile*)pFile)->pRealFile;
  أرجع pReal->pMethods->x_Shm_تالو(pReal, ofst, n, flags);
}
ساكن فراغ cfShmBarrier(sqlite3_file *pFile){
  sqlite3_file *pReal = ((CrashFile*)pFile)->pRealFile;
  pReal->pMethods->xShmBarrier(pReal);
}
ساكن صحيح cfShmUnmap(sqlite3_file *pFile, صحيح delFlag){
  sqlite3_file *pReal = ((CrashFile*)pFile)->pRealFile;
  أرجع pReal->pMethods->xShmUnmap(pReal, delFlag);
}
ساكن صحيح cfShmMap(
  sqlite3_file *pFile,            /* Handle open on database file */
  صحيح iRegion,                    /* Region to retrieve */
  صحيح sz,                         /* Size of regions */
  صحيح w,                          /* True to extend file if necessary */
  فراغ متطاير **pp              /* OUT: Mapped memory */
){
  sqlite3_file *pReal = ((CrashFile*)pFile)->pRealFile;
  أرجع pReal->pMethods->xShmMap(pReal, iRegion, sz, w, pp);
}

ساكن ثابت sqlite3_io_methods CrashFileVtab = {
  2,                            /* iVersion */
  cfClose,                      /* xClose */
  cfRead,                       /* xRead */
  cfWrite,                      /* xWrite */
  cfTruncate,                   /* xTruncate */
  cfSync,                       /* xSync */
  cf_File_سائيز,                   /* xFileSize */
  cf_تالو,                       /* xLock */
  cfUnlock,                     /* xUnlock */
  cf_Check_Reserved_تالو,          /* xCheckReservedLock */
  cfFileControl,                /* xFileControl */
  cf_Sector_سائيز,                 /* xSectorSize */
  cfDeviceCharacteristics,      /* xDeviceCharacteristics */
  cfShmMap,                     /* xShmMap */
  cf_Shm_تالو,                    /* xShmLock */
  cfShmBarrier,                 /* xShmBarrier */
  cfShmUnmap                    /* xShmUnmap */
};

/*
** Application data for the crash VFS
*/
بنية crash_App_معلومات {
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
ساكن صحيح cfOpen(
  sqlite3_vfs *pCfVfs,
  ثابت حرف *zName,
  sqlite3_file *pFile,
  صحيح flags,
  صحيح *p_ٻاهريون_Flags
){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_معلومات;
  صحيح rc;
  CrashFile *pWrapper = (CrashFile *)pFile;
  sqlite3_file *pReal = (sqlite3_file*)&pWrapper[1];

  memset(pWrapper, 0, حجم(CrashFile));
  rc = sqlite3OsOpen(pVfs, zName, pReal, flags, p_ٻاهريون_Flags);

  إذا( rc==SQLITE_OK ){
    i64 i_سائيز;
    pWrapper->p_طريقو = &CrashFileVtab;
    pWrapper->zName = (حرف *)zName;
    pWrapper->pRealFile = pReal;
    rc = sqlite3_Os_File_سائيز(pReal, &i_سائيز);
    pWrapper->i_سائيز = (صحيح)i_سائيز;
    pWrapper->flags = flags;
  }
  إذا( rc==SQLITE_OK ){
    pWrapper->n_معلومات = (صحيح)(4096 + pWrapper->i_سائيز);
    pWrapper->z_معلومات = crash_malloc(pWrapper->n_معلومات);
    إذا( pWrapper->z_معلومات ){
      /* os_unix.c contains an assert() that fails if the caller attempts
      ** to read data from the 512-byte locking region of a file opened
      ** with the SQLITE_OPEN_MAIN_DB flag. This region of a database file
      ** never contains valid data anyhow. So avoid doing such a read here.
      **
      ** UPDATE: It also contains an assert() verifying that each call
      ** to the xRead() method reads less than 128KB of data.
      */
      i64 iOff;

      memset(pWrapper->z_معلومات, 0, pWrapper->n_معلومات);
      لكل(iOff=0; iOff<pWrapper->i_سائيز; iOff += 512){
        صحيح nRead = (صحيح)(pWrapper->i_سائيز - iOff);
        إذا( nRead>512 ) nRead = 512;
        rc = sqlite3OsRead(pReal, &pWrapper->z_معلومات[iOff], nRead, iOff);
      }
    }وإلا{
      rc = SQLITE_NOMEM;
    }
  }
  إذا( rc!=SQLITE_OK && pWrapper->p_طريقو ){
    sqlite3OsClose(pFile);
  }
  أرجع rc;
}

ساكن صحيح cfDelete(sqlite3_vfs *pCfVfs, ثابت حرف *z_رستو, صحيح dirSync){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_معلومات;
  أرجع pVfs->xDelete(pVfs, z_رستو, dirSync);
}
ساكن صحيح cfAccess(
  sqlite3_vfs *pCfVfs,
  ثابت حرف *z_رستو,
  صحيح flags,
  صحيح *p_Res_ٻاهريون
){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_معلومات;
  أرجع pVfs->xAccess(pVfs, z_رستو, flags, p_Res_ٻاهريون);
}
ساكن صحيح cf_ڀريل_Pathname(
  sqlite3_vfs *pCfVfs,
  ثابت حرف *z_رستو,
  صحيح n_رستو_ٻاهريون,
  حرف *z_رستو_ٻاهريون
){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_معلومات;
  أرجع pVfs->x_ڀريل_Pathname(pVfs, z_رستو, n_رستو_ٻاهريون, z_رستو_ٻاهريون);
}
ساكن فراغ *cfDlOpen(sqlite3_vfs *pCfVfs, ثابت حرف *z_رستو){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_معلومات;
  أرجع pVfs->xDlOpen(pVfs, z_رستو);
}
ساكن فراغ cfDlError(sqlite3_vfs *pCfVfs, صحيح nByte, حرف *z_Err_نياپو){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_معلومات;
  pVfs->xDlError(pVfs, nByte, z_Err_نياپو);
}
ساكن فراغ (*cfDlSym(sqlite3_vfs *pCfVfs, فراغ *pH, ثابت حرف *zSym))(فراغ){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_معلومات;
  أرجع pVfs->xDlSym(pVfs, pH, zSym);
}
ساكن فراغ cfDlClose(sqlite3_vfs *pCfVfs, فراغ *pHandle){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_معلومات;
  pVfs->xDlClose(pVfs, pHandle);
}
ساكن صحيح cfRandomness(sqlite3_vfs *pCfVfs, صحيح nByte, حرف *z_Buf_ٻاهريون){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_معلومات;
  أرجع pVfs->xRandomness(pVfs, nByte, z_Buf_ٻاهريون);
}
ساكن صحيح cfSleep(sqlite3_vfs *pCfVfs, صحيح nMicro){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_معلومات;
  أرجع pVfs->xSleep(pVfs, nMicro);
}
ساكن صحيح cf_Current_وقت(sqlite3_vfs *pCfVfs, مزدوج *p_وقت_ٻاهريون){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_معلومات;
  أرجع pVfs->x_Current_وقت(pVfs, p_وقت_ٻاهريون);
}
ساكن صحيح cf_Get_آخري_Error(sqlite3_vfs *pCfVfs, صحيح n, حرف *z){
  sqlite3_vfs *pVfs = (sqlite3_vfs *)pCfVfs->p_App_معلومات;
  أرجع pVfs->x_Get_آخري_Error(pVfs, n, z);
}

ساكن صحيح process_Dev_Sym_دليلون(
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[],
  صحيح *piDeviceChar,
  صحيح *pi_Sector_سائيز
){
  بنية DeviceFlag {
    حرف *zName;
    صحيح i_قدر;
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

  صحيح i;
  صحيح iDc = 0;
  صحيح i_Sector_سائيز = 0;
  صحيح سيٽ_Sectorsize = 0;
  صحيح سيٽ_Device_Char = 0;

  لكل(i=0; i<objc; i+=2){
    Tcl_سائيز nOpt;
    حرف *zOpt = Tcl_GetStringFromObj(objv[i], &nOpt);

    إذا( (nOpt>11 || nOpt<2 || لڙي_ڀيٽ("-sectorsize", zOpt, nOpt))
     && (nOpt>16 || nOpt<2 || لڙي_ڀيٽ("-characteristics", zOpt, nOpt))
    ){
      Tcl_Append_نتيجو(interp,
        "Bad option: \"", zOpt,
        "\" - must be \"-characteristics\" or \"-sectorsize\"", عدم
      );
      أرجع TCL_ERROR;
    }
    إذا( i==objc-1 ){
      Tcl_Append_نتيجو(interp, "Option requires an argument: \"", zOpt, "\"", عدم);
      أرجع TCL_ERROR;
    }

    إذا( zOpt[1]=='s' ){
      إذا( Tcl_GetIntFromObj(interp, objv[i+1], &i_Sector_سائيز) ){
        أرجع TCL_ERROR;
      }
      سيٽ_Sectorsize = 1;
    }وإلا{
      صحيح j;
      Tcl_Obj **apObj;
      Tcl_سائيز nObj;
      إذا( Tcl_ListObjGetElements(interp, objv[i+1], &nObj, &apObj) ){
        أرجع TCL_ERROR;
      }
      لكل(j=0; j<(صحيح)nObj; j++){
        صحيح rc;
        صحيح iChoice;
        Tcl_Obj *pFlag = Tcl_DuplicateObj(apObj[j]);
        Tcl_Incr_Ref_ڳڻپ(pFlag);
        Tcl_Utf_منزل_هيٺاهون(Tcl_GetString(pFlag));

        rc = Tcl_Get_اشاريو_From_Obj_Struct(
            interp, pFlag, aFlag, حجم(aFlag[0]), "no such flag", 0, &iChoice
        );
        Tcl_Decr_Ref_ڳڻپ(pFlag);
        إذا( rc ){
          أرجع TCL_ERROR;
        }

        iDc |= aFlag[iChoice].i_قدر;
      }
      سيٽ_Device_Char = 1;
    }
  }

  إذا( سيٽ_Device_Char ){
    *piDeviceChar = iDc;
  }
  إذا( سيٽ_Sectorsize ){
    *pi_Sector_سائيز = i_Sector_سائيز;
  }

  أرجع TCL_OK;
}

/*
** tclcmd:   sqlite3_crash_now
**
** Simulate a crash immediately. This function does not return
** (writeListSync() calls exit(-1)).
*/
ساكن صحيح SQLITE_TCLAPI crashNowCmd(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  إذا( objc!=1 ){
    Tcl_Wrong_انگ_دليلون(interp, 1, objv, "");
    أرجع TCL_ERROR;
  }
  writeListSync(0, 1);
  أكد( 0 );
  أرجع TCL_OK;
}

/*
** tclcmd:   sqlite_crash_enable ENABLE ?DEFAULT?
**
** Parameter ENABLE must be a boolean value. If true, then the "crash"
** vfs is added to the system. If false, it is removed.
*/
ساكن صحيح SQLITE_TCLAPI crashEnableCmd(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح isEnable;
  صحيح isDefault = 0;
  ساكن sqlite3_vfs crashVfs = {
    2,                  /* iVersion */
    0,                  /* szOsFile */
    0,                  /* mxPathname */
    0,                  /* pNext */
    "crash",            /* zName */
    0,                  /* pAppData */
 
    cfOpen,               /* xOpen */
    cfDelete,             /* xDelete */
    cfAccess,             /* xAccess */
    cf_ڀريل_Pathname,       /* xFullPathname */
    cfDlOpen,             /* xDlOpen */
    cfDlError,            /* xDlError */
    cfDlSym,              /* xDlSym */
    cfDlClose,            /* xDlClose */
    cfRandomness,         /* xRandomness */
    cfSleep,              /* xSleep */
    cf_Current_وقت,        /* xCurrentTime */
    cf_Get_آخري_Error,       /* xGetLastError */
    0,                    /* xCurrentTimeInt64 */
  };

  إذا( objc!=2 && objc!=3 ){
    Tcl_Wrong_انگ_دليلون(interp, 1, objv, "ENABLE ?DEFAULT?");
    أرجع TCL_ERROR;
  }

  إذا( Tcl_GetBooleanFromObj(interp, objv[1], &isEnable) ){
    أرجع TCL_ERROR;
  }
  إذا( objc==3 && Tcl_GetBooleanFromObj(interp, objv[2], &isDefault) ){
    أرجع TCL_ERROR;
  }

  إذا( (isEnable && crashVfs.p_App_معلومات) || (!isEnable && !crashVfs.p_App_معلومات) ){
    أرجع TCL_OK;
  }

  إذا( crashVfs.p_App_معلومات==0 ){
    sqlite3_vfs *pOriginalVfs = sqlite3_vfs_ڳوليو(0);
    crashVfs.mxPathname = pOriginalVfs->mxPathname;
    crashVfs.p_App_معلومات = (فراغ *)pOriginalVfs;
    crashVfs.szOsFile = حجم(CrashFile) + pOriginalVfs->szOsFile;
    sqlite3_vfs_رجسٽر(&crashVfs, isDefault);
  }وإلا{
    crashVfs.p_App_معلومات = 0;
    sqlite3_vfs_unregister(&crashVfs);
  }

  أرجع TCL_OK;
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
ساكن صحيح SQLITE_TCLAPI crashParamsObjCmd(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح iDelay;
  ثابت حرف *zCrashFile;
  Tcl_سائيز nCrashFile;
  صحيح iDc, i_Sector_سائيز;

  iDc = -1;
  i_Sector_سائيز = -1;

  إذا( objc<3 ){
    Tcl_Wrong_انگ_دليلون(interp, 1, objv, "?OPTIONS? DELAY CRASHFILE");
    اذهب error;
  }

  zCrashFile = Tcl_GetStringFromObj(objv[objc-1], &nCrashFile);
  إذا( nCrashFile>=حجم(g.zCrashFile) ){
    Tcl_Append_نتيجو(interp, "Filename is too long: \"", zCrashFile, "\"", عدم);
    اذهب error;
  }
  إذا( Tcl_GetIntFromObj(interp, objv[objc-2], &iDelay) ){
    اذهب error;
  }

  إذا( process_Dev_Sym_دليلون(interp, objc-3, &objv[1], &iDc, &i_Sector_سائيز) ){
    أرجع TCL_ERROR;
  }

  إذا( iDc>=0 ){
    g.iDeviceCharacteristics = iDc;
  }
  إذا( i_Sector_سائيز>=0 ){
    g.i_Sector_سائيز = i_Sector_سائيز;
  }

  g.iCrash = iDelay;
  يادگيري_نقل(g.zCrashFile, zCrashFile, nCrashFile+1);
  sqlite3CrashTestEnable = 1;
  أرجع TCL_OK;

error:
  أرجع TCL_ERROR;
}

ساكن صحيح SQLITE_TCLAPI devSymObjCmd(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  فراغ devsym_رجسٽر(صحيح iDeviceChar, صحيح i_Sector_سائيز);

  صحيح iDc = -1;
  صحيح i_Sector_سائيز = -1;

  إذا( process_Dev_Sym_دليلون(interp, objc-1, &objv[1], &iDc, &i_Sector_سائيز) ){
    أرجع TCL_ERROR;
  }
  devsym_رجسٽر(iDc, i_Sector_سائيز);

  أرجع TCL_OK;
}

/*
** tclcmd: sqlite3_crash_on_write N
*/
ساكن صحيح SQLITE_TCLAPI writeCrashObjCmd(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  فراغ devsym_crash_on_write(صحيح);
  صحيح nWrite = 0;

  إذا( objc!=2 ){
    Tcl_Wrong_انگ_دليلون(interp, 1, objv, "NWRITE");
    أرجع TCL_ERROR;
  }
  إذا( Tcl_GetIntFromObj(interp, objv[1], &nWrite) ){
    أرجع TCL_ERROR;
  }

  devsym_crash_on_write(nWrite);
  أرجع TCL_OK;
}

/*
** tclcmd: unregister_devsim
*/
ساكن صحيح SQLITE_TCLAPI dsUnregisterObjCmd(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  فراغ devsym_unregister(فراغ);

  إذا( objc!=1 ){
    Tcl_Wrong_انگ_دليلون(interp, 1, objv, "");
    أرجع TCL_ERROR;
  }

  devsym_unregister();
  أرجع TCL_OK;
}

/*
** tclcmd: register_jt_vfs ?-default? PARENT-VFS
*/
ساكن صحيح SQLITE_TCLAPI jtObjCmd(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح jt_رجسٽر(حرف *, صحيح);
  حرف *z_والد = 0;

  إذا( objc!=2 && objc!=3 ){
    Tcl_Wrong_انگ_دليلون(interp, 1, objv, "?-default? PARENT-VFS");
    أرجع TCL_ERROR;
  }
  z_والد = Tcl_GetString(objv[1]);
  إذا( objc==3 ){
    إذا( strcmp(z_والد, "-default") ){
      Tcl_Append_نتيجو(interp,
          "bad option \"", z_والد, "\": must be -default", عدم
      );
      أرجع TCL_ERROR;
    }
    z_والد = Tcl_GetString(objv[2]);
  }

  إذا( !(*z_والد) ){
    z_والد = 0;
  }
  إذا( jt_رجسٽر(z_والد, objc==3) ){
    Tcl_Append_نتيجو(interp, "Error in jt_register", عدم);
    أرجع TCL_ERROR;
  }

  أرجع TCL_OK;
}

/*
** tclcmd: unregister_jt_vfs
*/
ساكن صحيح SQLITE_TCLAPI jtUnregisterObjCmd(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  فراغ jt_unregister(فراغ);

  إذا( objc!=1 ){
    Tcl_Wrong_انگ_دليلون(interp, 1, objv, "");
    أرجع TCL_ERROR;
  }

  jt_unregister();
  أرجع TCL_OK;
}

#جيڪڏهن_پورو /* SQLITE_OMIT_DISKIO */

/*
** This procedure registers the TCL procedures defined in this file.
*/
صحيح Sqlitetest6_Init(Tcl_Interp *interp){
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_DISKIO
  Tcl_CreateObjCommand(interp, "sqlite3_crash_enable", crashEnableCmd, 0, 0);
  Tcl_CreateObjCommand(interp, "sqlite3_crashparams", crashParamsObjCmd, 0, 0);
  Tcl_CreateObjCommand(interp, "sqlite3_crash_now", crashNowCmd, 0, 0);
  Tcl_CreateObjCommand(interp, "sqlite3_simulate_device", devSymObjCmd, 0, 0);
  Tcl_CreateObjCommand(interp, "sqlite3_crash_on_write", writeCrashObjCmd,0,0);
  Tcl_CreateObjCommand(interp, "unregister_devsim", dsUnregisterObjCmd, 0, 0);
  Tcl_CreateObjCommand(interp, "register_jt_vfs", jtObjCmd, 0, 0);
  Tcl_CreateObjCommand(interp, "unregister_jt_vfs", jtUnregisterObjCmd, 0, 0);
#جيڪڏهن_پورو
  أرجع TCL_OK;
}

#جيڪڏهن_پورو /* SQLITE_TEST */
