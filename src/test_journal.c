/*
** 2008 Jan 22
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
** This file contains code for a VFS layer that acts as a wrapper around
** an existing VFS. The code in this file attempts to verify that SQLite
** correctly populates and syncs a journal file before writing to a
** corresponding database file.
**
** INTERFACE
**
**   The public interface to this wrapper VFS is two functions:
**
**     jt_register()
**     jt_unregister()
**
**   See header comments associated with those two functions below for 
**   details.
**
** LIMITATIONS
**
**   This wrapper will not work if "PRAGMA synchronous = off" is used.
**
** OPERATION
**
**  Starting a Transaction:
**
**   When a write-transaction is started, the contents of the database is
**   inspected and the following data stored as part of the database file 
**   handle (type struct jt_file):
**
**     a) The page-size of the database file.
**     b) The number of pages that are in the database file.
**     c) The set of page numbers corresponding to free-list leaf pages.
**     d) A check-sum for every page in the database file.
**
**   The start of a write-transaction is deemed to have occurred when a 
**   28-byte journal header is written to byte offset 0 of the journal 
**   file.
**
**  Syncing the Journal File:
**
**   Whenever the xSync method is invoked to sync a journal-file, the
**   contents of the journal file are read. For each page written to
**   the journal file, a check-sum is calculated and compared to the  
**   check-sum calculated for the corresponding database page when the
**   write-transaction was initialized. The success of the comparison
**   is assert()ed. So if SQLite has written something other than the
**   original content to the database file, an assert() will fail.
**
**   Additionally, the set of page numbers for which records exist in
**   the journal file is added to (unioned with) the set of page numbers
**   corresponding to free-list leaf pages collected when the 
**   write-transaction was initialized. This set comprises the page-numbers 
**   corresponding to those pages that SQLite may now safely modify.
**
**  Writing to the Database File:
**
**   When a block of data is written to a database file, the following
**   invariants are asserted:
**
**     a) That the block of data is an aligned block of page-size bytes.
**
**     b) That if the page being written did not exist when the 
**        transaction was started (i.e. the database file is growing), then
**        the journal-file must have been synced at least once since
**        the start of the transaction.
**
**     c) That if the page being written did exist when the transaction 
**        was started, then the page must have either been a free-list
**        leaf page at the start of the transaction, or else must have
**        been stored in the journal file prior to the most recent sync.
**
**  Closing a Transaction:
**
**   When a transaction is closed, all data collected at the start of
**   the transaction, or following an xSync of a journal-file, is 
**   discarded. The end of a transaction is recognized when any one 
**   of the following occur:
**
**     a) A block of zeroes (or anything else that is not a valid 
**        journal-header) is written to the start of the journal file.
**
**     b) A journal file is truncated to zero bytes in size using xTruncate.
**
**     c) The journal file is deleted using xDelete.
*/
#যদি SQLITE_TEST          /* This file is used for testing only */

#অন্তর্ভাব "sqlite3.h"
#অন্তর্ভাব "sqliteInt.h"

/*
** Maximum pathname length supported by the jt backend.
*/
#define JT_MAX_PATHNAME 512

/*
** Name used to identify this VFS.
*/
#define JT_VFS_NAME "jt"

প্রকারলেখ কাঠামো jt_file jt_file;
কাঠামো jt_file {
  sqlite3_file base;
  ধ্রুবক অক্ষর *zName;       /* Name of open file */
  পূর্ণসংখ্যা flags;               /* Flags the file was opened with */

  /* The following are only used by database file file handles */
  পূর্ণসংখ্যা eLock;               /* Current lock held on the file */
  u32 nPage;               /* Size of file in pages when transaction started */
  u32 nPagesize;           /* Page size when transaction started */
  Bitvec *pWritable;       /* Bitvec of pages that may be written to the file */
  u32 *aCksum;             /* Checksum for first nPage pages */
  পূর্ণসংখ্যা nSync;               /* Number of times journal file has been synced */

  /* Only used by journal file-handles */
  sqlite3_int64 iMaxOff;   /* Maximum offset written to this transaction */

  jt_file *p_পরবর্তী;          /* All files are stored in a linked list */
  sqlite3_file *pReal;     /* The file handle for the underlying vfs */
};

/*
** Method declarations for jt_file.
*/
স্থির পূর্ণসংখ্যা jtClose(sqlite3_file*);
স্থির পূর্ণসংখ্যা jtRead(sqlite3_file*, শূন্য*, পূর্ণসংখ্যা iAmt, sqlite3_int64 iOfst);
স্থির পূর্ণসংখ্যা jtWrite(sqlite3_file*,ধ্রুবক শূন্য*,পূর্ণসংখ্যা iAmt, sqlite3_int64 iOfst);
স্থির পূর্ণসংখ্যা jtTruncate(sqlite3_file*, sqlite3_int64 আকার_2);
স্থির পূর্ণসংখ্যা jtSync(sqlite3_file*, পূর্ণসংখ্যা flags);
স্থির পূর্ণসংখ্যা jt_File_আকার(sqlite3_file*, sqlite3_int64 *p_আকার);
স্থির পূর্ণসংখ্যা jtLock(sqlite3_file*, পূর্ণসংখ্যা);
স্থির পূর্ণসংখ্যা jtUnlock(sqlite3_file*, পূর্ণসংখ্যা);
স্থির পূর্ণসংখ্যা jtCheckReservedLock(sqlite3_file*, পূর্ণসংখ্যা *);
স্থির পূর্ণসংখ্যা jtFileControl(sqlite3_file*, পূর্ণসংখ্যা op, শূন্য *pArg);
স্থির পূর্ণসংখ্যা jt_Sector_আকার(sqlite3_file*);
স্থির পূর্ণসংখ্যা jtDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for jt_vfs.
*/
স্থির পূর্ণসংখ্যা jtOpen(sqlite3_vfs*, ধ্রুবক অক্ষর *, sqlite3_file*, পূর্ণসংখ্যা , পূর্ণসংখ্যা *);
স্থির পূর্ণসংখ্যা jtDelete(sqlite3_vfs*, ধ্রুবক অক্ষর *zName, পূর্ণসংখ্যা syncDir);
স্থির পূর্ণসংখ্যা jtAccess(sqlite3_vfs*, ধ্রুবক অক্ষর *zName, পূর্ণসংখ্যা flags, পূর্ণসংখ্যা *);
স্থির পূর্ণসংখ্যা jtFullPathname(sqlite3_vfs*, ধ্রুবক অক্ষর *zName, পূর্ণসংখ্যা, অক্ষর *zOut);
স্থির শূন্য *jtDlOpen(sqlite3_vfs*, ধ্রুবক অক্ষর *zFilename);
স্থির শূন্য jtDlError(sqlite3_vfs*, পূর্ণসংখ্যা nByte, অক্ষর *z_Err_বার্তা);
স্থির শূন্য (*jtDlSym(sqlite3_vfs*,শূন্য*, ধ্রুবক অক্ষর *z_প্রতীক))(শূন্য);
স্থির শূন্য jtDlClose(sqlite3_vfs*, শূন্য*);
স্থির পূর্ণসংখ্যা jtRandomness(sqlite3_vfs*, পূর্ণসংখ্যা nByte, অক্ষর *zOut);
স্থির পূর্ণসংখ্যা jtSleep(sqlite3_vfs*, পূর্ণসংখ্যা microseconds);
স্থির পূর্ণসংখ্যা jtCurrentTime(sqlite3_vfs*, দ্বিগুণ*);
স্থির পূর্ণসংখ্যা jtCurrentTimeInt64(sqlite3_vfs*, sqlite3_int64*);
স্থির পূর্ণসংখ্যা jtGetLastError(sqlite3_vfs*, পূর্ণসংখ্যা, অক্ষর*);

স্থির sqlite3_vfs jt_vfs = {
  2,                             /* iVersion */
  আকার(jt_file),               /* szOsFile */
  JT_MAX_PATHNAME,               /* mxPathname */
  0,                             /* pNext */
  JT_VFS_NAME,                   /* zName */
  0,                             /* pAppData */
  jtOpen,                        /* xOpen */
  jtDelete,                      /* xDelete */
  jtAccess,                      /* xAccess */
  jtFullPathname,                /* xFullPathname */
  jtDlOpen,                      /* xDlOpen */
  jtDlError,                     /* xDlError */
  jtDlSym,                       /* xDlSym */
  jtDlClose,                     /* xDlClose */
  jtRandomness,                  /* xRandomness */
  jtSleep,                       /* xSleep */
  jtCurrentTime,                 /* xCurrentTime */
  jtGetLastError,                /* xGetLastError */
  jtCurrentTimeInt64             /* xCurrentTimeInt64 */
};

স্থির sqlite3_io_methods jt_io_methods = {
  1,                             /* iVersion */
  jtClose,                       /* xClose */
  jtRead,                        /* xRead */
  jtWrite,                       /* xWrite */
  jtTruncate,                    /* xTruncate */
  jtSync,                        /* xSync */
  jt_File_আকার,                    /* xFileSize */
  jtLock,                        /* xLock */
  jtUnlock,                      /* xUnlock */
  jtCheckReservedLock,           /* xCheckReservedLock */
  jtFileControl,                 /* xFileControl */
  jt_Sector_আকার,                  /* xSectorSize */
  jtDeviceCharacteristics        /* xDeviceCharacteristics */
};

কাঠামো JtGlobal {
  sqlite3_vfs *pVfs;             /* Parent VFS */
  jt_file *pList;                /* List of all open files */
};
স্থির কাঠামো JtGlobal g = {0, 0};

/*
** Functions to obtain and relinquish a mutex to protect g.pList. The
** STATIC_PRNG mutex is reused, purely for the sake of convenience.
*/
স্থির শূন্য enterJtMutex(শূন্য){
  sqlite3_mutex_enter(sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_PRNG));
}
স্থির শূন্য leaveJtMutex(শূন্য){
  sqlite3_mutex_leave(sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_PRNG));
}

বাহ্যিক পূর্ণসংখ্যা sqlite3_io_error_pending;
বাহ্যিক পূর্ণসংখ্যা sqlite3_io_error_hit;
স্থির শূন্য stop_ioerr_simulation(পূর্ণসংখ্যা *piSave, পূর্ণসংখ্যা *piSave2){
  *piSave = sqlite3_io_error_pending;
  *piSave2 = sqlite3_io_error_hit;
  sqlite3_io_error_pending = -1;
  sqlite3_io_error_hit = 0;
}
স্থির শূন্য start_ioerr_simulation(পূর্ণসংখ্যা iSave, পূর্ণসংখ্যা iSave2){
  sqlite3_io_error_pending = iSave;
  sqlite3_io_error_hit = iSave2;
}

/*
** The jt_file pointed to by the argument may or may not be a file-handle
** open on a main database file. If it is, and a transaction is currently
** opened on the file, then discard all transaction related data.
*/
স্থির শূন্য closeTransaction(jt_file *p){
  sqlite3BitvecDestroy(p->pWritable);
  sqlite3_free(p->aCksum);
  p->pWritable = 0;
  p->aCksum = 0;
  p->nSync = 0;
}

/*
** Close an jt-file.
*/
স্থির পূর্ণসংখ্যা jtClose(sqlite3_file *pFile){
  jt_file **pp;
  jt_file *p = (jt_file *)pFile;

  closeTransaction(p);
  enterJtMutex();
  যদি( p->zName ){
    জন্য(pp=&g.pList; *pp!=p; pp=&(*pp)->p_পরবর্তী);
    *pp = p->p_পরবর্তী;
  }
  leaveJtMutex();
  sqlite3OsClose(p->pReal);
  ফেরত SQLITE_OK;
}

/*
** Read data from an jt-file.
*/
স্থির পূর্ণসংখ্যা jtRead(
  sqlite3_file *pFile, 
  শূন্য *zBuf, 
  পূর্ণসংখ্যা iAmt, 
  sqlite_int64 iOfst
){
  jt_file *p = (jt_file *)pFile;
  ফেরত sqlite3OsRead(p->pReal, zBuf, iAmt, iOfst);
}

/*
** Parameter zJournal is the name of a journal file that is currently 
** open. This function locates and returns the handle opened on the
** corresponding database file by the pager that currently has the
** journal file opened. This file-handle is identified by the 
** following properties:
**
**   a) SQLITE_OPEN_MAIN_DB was specified when the file was opened.
**
**   b) The file-name specified when the file was opened matches
**      all but the final 8 characters of the journal file name.
**
**   c) There is currently a reserved lock on the file. This 
**      condition is waived if the noLock argument is non-zero.
**/
স্থির jt_file *locateDatabaseHandle(ধ্রুবক অক্ষর *zJournal, পূর্ণসংখ্যা noLock){
  jt_file *pMain = 0;
  enterJtMutex();
  জন্য(pMain=g.pList; pMain; pMain=pMain->p_পরবর্তী){
    পূর্ণসংখ্যা nName = (পূর্ণসংখ্যা)(strlen(zJournal) - strlen("-journal"));
    যদি( (pMain->flags&SQLITE_OPEN_MAIN_DB)
     && ((পূর্ণসংখ্যা)strlen(pMain->zName)==nName)
     && 0==memcmp(pMain->zName, zJournal, nName)
     && ((pMain->eLock>=SQLITE_LOCK_RESERVED) || noLock)
    ){
      বিরতি;
    }
  }
  leaveJtMutex();
  ফেরত pMain;
}

/*
** Parameter z points to a buffer of 4 bytes in size containing a 
** unsigned 32-bit integer stored in big-endian format. Decode the 
** integer and return its value.
*/
স্থির u32 decodeUint32(ধ্রুবক unsigned অক্ষর *z){
  ফেরত (z[0]<<24) + (z[1]<<16) + (z[2]<<8) + z[3];
}

/*
** Calculate a checksum from the buffer of length n bytes pointed to
** by parameter z.
*/
স্থির u32 genCksum(ধ্রুবক unsigned অক্ষর *z, পূর্ণসংখ্যা n){
  পূর্ণসংখ্যা i;
  u32 cksum = 0;
  জন্য(i=0; i<n; i++){
    cksum = cksum + z[i] + (cksum<<3);
  }
  ফেরত cksum;
}

/*
** The first argument, zBuf, points to a buffer containing a 28 byte
** serialized journal header. This function deserializes four of the
** integer fields contained in the journal header and writes their
** values to the output variables.
**
** SQLITE_OK is returned if the journal-header is successfully 
** decoded. Otherwise, SQLITE_ERROR.
*/
স্থির পূর্ণসংখ্যা decodeJournalHdr(
  ধ্রুবক unsigned অক্ষর *zBuf,         /* Input: 28 byte journal header */
  u32 *pnRec,                        /* Out: Number of journalled records */
  u32 *pnPage,                       /* Out: Original database page count */
  u32 *pnSector,                     /* Out: Sector size in bytes */
  u32 *pnPagesize                    /* Out: Page size in bytes */
){
  unsigned অক্ষর aMagic[] = { 0xd9, 0xd5, 0x05, 0xf9, 0x20, 0xa1, 0x63, 0xd7 };
  যদি( memcmp(aMagic, zBuf, 8) ) ফেরত SQLITE_ERROR;
  যদি( pnRec ) *pnRec = decodeUint32(&zBuf[8]);
  যদি( pnPage ) *pnPage = decodeUint32(&zBuf[16]);
  যদি( pnSector ) *pnSector = decodeUint32(&zBuf[20]);
  যদি( pnPagesize ) *pnPagesize = decodeUint32(&zBuf[24]);
  ফেরত SQLITE_OK;
}

/*
** This function is called when a new transaction is opened, just after
** the first journal-header is written to the journal file.
*/
স্থির পূর্ণসংখ্যা openTransaction(jt_file *pMain, jt_file *pJournal){
  unsigned অক্ষর *a_তথ্য;
  sqlite3_file *p = pMain->pReal;
  পূর্ণসংখ্যা rc = SQLITE_OK;

  closeTransaction(pMain);
  a_তথ্য = sqlite3_malloc(pMain->nPagesize);
  pMain->pWritable = sqlite3BitvecCreate(pMain->nPage);
  pMain->aCksum = sqlite3_malloc(আকার(u32) * (pMain->nPage + 1));
  pJournal->iMaxOff = 0;

  যদি( !pMain->pWritable || !pMain->aCksum || !a_তথ্য ){
    rc = SQLITE_IOERR_NOMEM;
  }অন্যথা যদি( pMain->nPage>0 ){
    u32 iTrunk;
    পূর্ণসংখ্যা iSave;
    পূর্ণসংখ্যা iSave2;

    stop_ioerr_simulation(&iSave, &iSave2);

    /* Read the database free-list. Add the page-number for each free-list
    ** leaf to the jt_file.pWritable bitvec.
    */
    rc = sqlite3OsRead(p, a_তথ্য, pMain->nPagesize, 0);
    যদি( rc==SQLITE_OK ){
      u32 nDbsize = decodeUint32(&a_তথ্য[28]);
      যদি( nDbsize>0 && memcmp(&a_তথ্য[24], &a_তথ্য[92], 4)==0 ){
        u32 iPg;
        জন্য(iPg=nDbsize+1; iPg<=pMain->nPage; iPg++){
          sqlite3BitvecSet(pMain->pWritable, iPg);
        }
      }
    }
    iTrunk = decodeUint32(&a_তথ্য[32]);
    যখন( rc==SQLITE_OK && iTrunk>0 ){
      u32 nLeaf;
      u32 iLeaf;
      sqlite3_int64 iOff = (i64)(iTrunk-1)*pMain->nPagesize;
      rc = sqlite3OsRead(p, a_তথ্য, pMain->nPagesize, iOff);
      nLeaf = decodeUint32(&a_তথ্য[4]);
      জন্য(iLeaf=0; rc==SQLITE_OK && iLeaf<nLeaf; iLeaf++){
        u32 pgno = decodeUint32(&a_তথ্য[8+4*iLeaf]);
        sqlite3BitvecSet(pMain->pWritable, pgno);
      }
      iTrunk = decodeUint32(a_তথ্য);
    }

    /* Calculate and store a checksum for each page in the database file. */
    যদি( rc==SQLITE_OK ){
      পূর্ণসংখ্যা ii;
      জন্য(ii=0; rc==SQLITE_OK && ii<(পূর্ণসংখ্যা)pMain->nPage; ii++){
        i64 iOff = (i64)(pMain->nPagesize) * (i64)ii;
        যদি( iOff==PENDING_BYTE ) চালু;
        rc = sqlite3OsRead(pMain->pReal, a_তথ্য, pMain->nPagesize, iOff);
        pMain->aCksum[ii] = genCksum(a_তথ্য, pMain->nPagesize);
        যদি( ii+1==(পূর্ণসংখ্যা)pMain->nPage && rc==SQLITE_IOERR_SHORT_READ ){
          rc = SQLITE_OK;
        }
      }
    }

    start_ioerr_simulation(iSave, iSave2);
  }

  sqlite3_free(a_তথ্য);
  ফেরত rc;
}

/*
** The first argument to this function is a handle open on a journal file.
** This function reads the journal file and adds the page number for each
** page in the journal to the Bitvec object passed as the second argument.
*/
স্থির পূর্ণসংখ্যা readJournalFile(jt_file *p, jt_file *pMain){
  পূর্ণসংখ্যা rc = SQLITE_OK;
  unsigned অক্ষর zBuf[28];
  sqlite3_file *pReal = p->pReal;
  sqlite3_int64 iOff = 0;
  sqlite3_int64 i_আকার = p->iMaxOff;
  unsigned অক্ষর *aPage;
  পূর্ণসংখ্যা iSave;
  পূর্ণসংখ্যা iSave2;

  aPage = sqlite3_malloc(pMain->nPagesize);
  যদি( !aPage ){
    ফেরত SQLITE_IOERR_NOMEM;
  }

  stop_ioerr_simulation(&iSave, &iSave2);

  যখন( rc==SQLITE_OK && iOff<i_আকার ){
    u32 nRec, nPage, nSector, nPagesize;
    u32 ii;

    /* Read and decode the next journal-header from the journal file. */
    rc = sqlite3OsRead(pReal, zBuf, 28, iOff);
    যদি( rc!=SQLITE_OK 
     || decodeJournalHdr(zBuf, &nRec, &nPage, &nSector, &nPagesize) 
    ){
      যাও finish_rjf;
    }
    iOff += nSector;

    যদি( nRec==0 ){
      /* A trick. There might be another journal-header immediately 
      ** following this one. In this case, 0 records means 0 records, 
      ** not "read until the end of the file". See also ticket #2565.
      */
      যদি( i_আকার>=(iOff+nSector) ){
        rc = sqlite3OsRead(pReal, zBuf, 28, iOff);
        যদি( rc!=SQLITE_OK || 0==decodeJournalHdr(zBuf, 0, 0, 0, 0) ){
          চালু;
        }
      }
      nRec = (u32)((i_আকার-iOff) / (pMain->nPagesize+8));
    }

    /* Read all the records that follow the journal-header just read. */
    জন্য(ii=0; rc==SQLITE_OK && ii<nRec && iOff<i_আকার; ii++){
      u32 pgno;
      rc = sqlite3OsRead(pReal, zBuf, 4, iOff);
      যদি( rc==SQLITE_OK ){
        pgno = decodeUint32(zBuf);
        যদি( pgno>0 && pgno<=pMain->nPage ){
          যদি( 0==sqlite3BitvecTest(pMain->pWritable, pgno) ){
            rc = sqlite3OsRead(pReal, aPage, pMain->nPagesize, iOff+4);
            যদি( rc==SQLITE_OK ){
              u32 cksum = genCksum(aPage, pMain->nPagesize);
              প্রতিজ্ঞা( cksum==pMain->aCksum[pgno-1] );
            }
          }
          sqlite3BitvecSet(pMain->pWritable, pgno);
        }
        iOff += (8 + pMain->nPagesize);
      }
    }

    iOff = ((iOff + (nSector-1)) / nSector) * nSector;
  }

finish_rjf:
  start_ioerr_simulation(iSave, iSave2);
  sqlite3_free(aPage);
  যদি( rc==SQLITE_IOERR_SHORT_READ ){
    rc = SQLITE_OK;
  }
  ফেরত rc;
}

/*
** Write data to an jt-file.
*/
স্থির পূর্ণসংখ্যা jtWrite(
  sqlite3_file *pFile, 
  ধ্রুবক শূন্য *zBuf, 
  পূর্ণসংখ্যা iAmt, 
  sqlite_int64 iOfst
){
  পূর্ণসংখ্যা rc;
  jt_file *p = (jt_file *)pFile;
  যদি( p->flags&SQLITE_OPEN_MAIN_JOURNAL ){
    যদি( iOfst==0 ){
      jt_file *pMain = locateDatabaseHandle(p->zName, 0);
      প্রতিজ্ঞা( pMain );
  
      যদি( iAmt==28 ){
        /* Zeroing the first journal-file header. This is the end of a
        ** transaction. */
        closeTransaction(pMain);
      }অন্যথা যদি( iAmt!=12 ){
        /* Writing the first journal header to a journal file. This happens
        ** when a transaction is first started.  */
        u8 *z = (u8 *)zBuf;
        pMain->nPage = decodeUint32(&z[16]);
        pMain->nPagesize = decodeUint32(&z[24]);
        যদি( SQLITE_OK!=(rc=openTransaction(pMain, p)) ){
          ফেরত rc;
        }
      }
    }
    যদি( p->iMaxOff<(iOfst + iAmt) ){
      p->iMaxOff = iOfst + iAmt;
    }
  }

  যদি( p->flags&SQLITE_OPEN_MAIN_DB && p->pWritable ){
    যদি( iAmt<(পূর্ণসংখ্যা)p->nPagesize 
     && p->nPagesize%iAmt==0 
     && iOfst>=(PENDING_BYTE+512) 
     && iOfst+iAmt<=PENDING_BYTE+p->nPagesize
    ){
      /* No-op. This special case is hit when the backup code is copying a
      ** to a database with a larger page-size than the source database and
      ** it needs to fill in the non-locking-region part of the original
      ** pending-byte page.
      */
    }অন্যথা{
      u32 pgno = (u32)(iOfst/p->nPagesize + 1);
      প্রতিজ্ঞা( (iAmt==1||iAmt==(পূর্ণসংখ্যা)p->nPagesize) &&
              ((iOfst+iAmt)%p->nPagesize)==0 );
      /* The following assert() statements may fail if this layer is used
      ** with a connection in "PRAGMA synchronous=off" mode. If they
      ** fail with sync=normal or sync=full, this may indicate problem.  */
      প্রতিজ্ঞা( p->nPage==0 || pgno<=p->nPage || p->nSync>0 );
      প্রতিজ্ঞা( pgno>p->nPage || sqlite3BitvecTest(p->pWritable, pgno) );
    }
  }

  rc = sqlite3OsWrite(p->pReal, zBuf, iAmt, iOfst);
  যদি( (p->flags&SQLITE_OPEN_MAIN_JOURNAL) && iAmt==12 ){
    jt_file *pMain = locateDatabaseHandle(p->zName, 0);
    পূর্ণসংখ্যা rc2 = readJournalFile(p, pMain);
    যদি( rc==SQLITE_OK ) rc = rc2;
  }
  ফেরত rc;
}

/*
** Truncate an jt-file.
*/
স্থির পূর্ণসংখ্যা jtTruncate(sqlite3_file *pFile, sqlite_int64 আকার_2){
  jt_file *p = (jt_file *)pFile;
  যদি( p->flags&SQLITE_OPEN_MAIN_JOURNAL && আকার_2==0 ){
    /* Truncating a journal file. This is the end of a transaction. */
    jt_file *pMain = locateDatabaseHandle(p->zName, 0);
    closeTransaction(pMain);
  }
  যদি( p->flags&SQLITE_OPEN_MAIN_DB && p->pWritable ){
    u32 pgno;
    u32 locking_page = (u32)(PENDING_BYTE/p->nPagesize+1);
    জন্য(pgno=(u32)(আকার_2/p->nPagesize+1); pgno<=p->nPage; pgno++){
      প্রতিজ্ঞা( pgno==locking_page || sqlite3BitvecTest(p->pWritable, pgno) );
    }
  }
  ফেরত sqlite3OsTruncate(p->pReal, আকার_2);
}

/*
** Sync an jt-file.
*/
স্থির পূর্ণসংখ্যা jtSync(sqlite3_file *pFile, পূর্ণসংখ্যা flags){
  jt_file *p = (jt_file *)pFile;

  যদি( p->flags&SQLITE_OPEN_MAIN_JOURNAL ){
    পূর্ণসংখ্যা rc;
    jt_file *pMain;                   /* The associated database file */

    /* The journal file is being synced. At this point, we inspect the 
    ** contents of the file up to this point and set each bit in the 
    ** jt_file.pWritable bitvec of the main database file associated with
    ** this journal file.
    */
    pMain = locateDatabaseHandle(p->zName, 0);

    /* Set the bitvec values */
    যদি( pMain && pMain->pWritable ){
      pMain->nSync++;
      rc = readJournalFile(p, pMain);
      যদি( rc!=SQLITE_OK ){
        ফেরত rc;
      }
    }
  }

  ফেরত sqlite3OsSync(p->pReal, flags);
}

/*
** Return the current file-size of an jt-file.
*/
স্থির পূর্ণসংখ্যা jt_File_আকার(sqlite3_file *pFile, sqlite_int64 *p_আকার){
  jt_file *p = (jt_file *)pFile;
  ফেরত sqlite3_Os_File_আকার(p->pReal, p_আকার);
}

/*
** Lock an jt-file.
*/
স্থির পূর্ণসংখ্যা jtLock(sqlite3_file *pFile, পূর্ণসংখ্যা eLock){
  পূর্ণসংখ্যা rc;
  jt_file *p = (jt_file *)pFile;
  rc = sqlite3OsLock(p->pReal, eLock);
  যদি( rc==SQLITE_OK && eLock>p->eLock ){
    p->eLock = eLock;
  }
  ফেরত rc;
}

/*
** Unlock an jt-file.
*/
স্থির পূর্ণসংখ্যা jtUnlock(sqlite3_file *pFile, পূর্ণসংখ্যা eLock){
  পূর্ণসংখ্যা rc;
  jt_file *p = (jt_file *)pFile;
  rc = sqlite3OsUnlock(p->pReal, eLock);
  যদি( rc==SQLITE_OK && eLock<p->eLock ){
    p->eLock = eLock;
  }
  ফেরত rc;
}

/*
** Check if another file-handle holds a RESERVED lock on an jt-file.
*/
স্থির পূর্ণসংখ্যা jtCheckReservedLock(sqlite3_file *pFile, পূর্ণসংখ্যা *pResOut){
  jt_file *p = (jt_file *)pFile;
  ফেরত sqlite3OsCheckReservedLock(p->pReal, pResOut);
}

/*
** File control method. For custom operations on an jt-file.
*/
স্থির পূর্ণসংখ্যা jtFileControl(sqlite3_file *pFile, পূর্ণসংখ্যা op, শূন্য *pArg){
  jt_file *p = (jt_file *)pFile;
  ফেরত p->pReal->pMethods->xFileControl(p->pReal, op, pArg);
}

/*
** Return the sector-size in bytes for an jt-file.
*/
স্থির পূর্ণসংখ্যা jt_Sector_আকার(sqlite3_file *pFile){
  jt_file *p = (jt_file *)pFile;
  ফেরত sqlite3_Os_Sector_আকার(p->pReal);
}

/*
** Return the device characteristic flags supported by an jt-file.
*/
স্থির পূর্ণসংখ্যা jtDeviceCharacteristics(sqlite3_file *pFile){
  jt_file *p = (jt_file *)pFile;
  ফেরত sqlite3OsDeviceCharacteristics(p->pReal);
}

/*
** Open an jt file handle.
*/
স্থির পূর্ণসংখ্যা jtOpen(
  sqlite3_vfs *pVfs,
  ধ্রুবক অক্ষর *zName,
  sqlite3_file *pFile,
  পূর্ণসংখ্যা flags,
  পূর্ণসংখ্যা *pOutFlags
){
  পূর্ণসংখ্যা rc;
  jt_file *p = (jt_file *)pFile;
  pFile->pMethods = 0;
  p->pReal = (sqlite3_file *)&p[1];
  p->pReal->pMethods = 0;
  rc = sqlite3OsOpen(g.pVfs, zName, p->pReal, flags, pOutFlags);
  প্রতিজ্ঞা( rc==SQLITE_OK || p->pReal->pMethods==0 );
  যদি( rc==SQLITE_OK ){
    pFile->pMethods = &jt_io_methods;
    p->eLock = 0;
    p->zName = zName;
    p->flags = flags;
    p->p_পরবর্তী = 0;
    p->pWritable = 0;
    p->aCksum = 0;
    enterJtMutex();
    যদি( zName ){
      p->p_পরবর্তী = g.pList;
      g.pList = p;
    }
    leaveJtMutex();
  }
  ফেরত rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
স্থির পূর্ণসংখ্যা jtDelete(sqlite3_vfs *pVfs, ধ্রুবক অক্ষর *zPath, পূর্ণসংখ্যা dirSync){
  পূর্ণসংখ্যা nPath = (পূর্ণসংখ্যা)strlen(zPath);
  যদি( nPath>8 && 0==strcmp("-journal", &zPath[nPath-8]) ){
    /* Deleting a journal file. The end of a transaction. */
    jt_file *pMain = locateDatabaseHandle(zPath, 0);
    যদি( pMain ){
      closeTransaction(pMain);
    }
  }

  ফেরত sqlite3OsDelete(g.pVfs, zPath, dirSync);
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
স্থির পূর্ণসংখ্যা jtAccess(
  sqlite3_vfs *pVfs, 
  ধ্রুবক অক্ষর *zPath, 
  পূর্ণসংখ্যা flags, 
  পূর্ণসংখ্যা *pResOut
){
  ফেরত sqlite3OsAccess(g.pVfs, zPath, flags, pResOut);
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (JT_MAX_PATHNAME+1) bytes.
*/
স্থির পূর্ণসংখ্যা jtFullPathname(
  sqlite3_vfs *pVfs, 
  ধ্রুবক অক্ষর *zPath, 
  পূর্ণসংখ্যা nOut, 
  অক্ষর *zOut
){
  ফেরত sqlite3OsFullPathname(g.pVfs, zPath, nOut, zOut);
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
স্থির শূন্য *jtDlOpen(sqlite3_vfs *pVfs, ধ্রুবক অক্ষর *zPath){
  ফেরত g.pVfs->xDlOpen(g.pVfs, zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
স্থির শূন্য jtDlError(sqlite3_vfs *pVfs, পূর্ণসংখ্যা nByte, অক্ষর *z_Err_বার্তা){
  g.pVfs->xDlError(g.pVfs, nByte, z_Err_বার্তা);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
স্থির শূন্য (*jtDlSym(sqlite3_vfs *pVfs, শূন্য *p, ধ্রুবক অক্ষর *zSym))(শূন্য){
  ফেরত g.pVfs->xDlSym(g.pVfs, p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
স্থির শূন্য jtDlClose(sqlite3_vfs *pVfs, শূন্য *pHandle){
  g.pVfs->xDlClose(g.pVfs, pHandle);
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
স্থির পূর্ণসংখ্যা jtRandomness(sqlite3_vfs *pVfs, পূর্ণসংখ্যা nByte, অক্ষর *zBufOut){
  ফেরত sqlite3OsRandomness(g.pVfs, nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
স্থির পূর্ণসংখ্যা jtSleep(sqlite3_vfs *pVfs, পূর্ণসংখ্যা nMicro){
  ফেরত sqlite3OsSleep(g.pVfs, nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
স্থির পূর্ণসংখ্যা jtCurrentTime(sqlite3_vfs *pVfs, দ্বিগুণ *pTimeOut){
  ফেরত g.pVfs->xCurrentTime(g.pVfs, pTimeOut);
}
/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
স্থির পূর্ণসংখ্যা jtCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *pTimeOut){
  ফেরত g.pVfs->xCurrentTimeInt64(g.pVfs, pTimeOut);
}

স্থির পূর্ণসংখ্যা jtGetLastError(sqlite3_vfs *pVfs, পূর্ণসংখ্যা n, অক্ষর *z){
  ফেরত g.pVfs->xGetLastError(g.pVfs, n, z);
}

/**************************************************************************
** Start of public API.
*/

/*
** Configure the jt VFS as a wrapper around the VFS named by parameter 
** zWrap. If the isDefault parameter is true, then the jt VFS is installed
** as the new default VFS for SQLite connections. If isDefault is not
** true, then the jt VFS is installed as non-default. In this case it
** is available via its name, "jt".
*/
পূর্ণসংখ্যা jt_register(অক্ষর *zWrap, পূর্ণসংখ্যা isDefault){
  g.pVfs = sqlite3_vfs_find(zWrap);
  যদি( g.pVfs==0 ){
    ফেরত SQLITE_ERROR;
  }
  jt_vfs.szOsFile = আকার(jt_file) + g.pVfs->szOsFile;
  যদি( g.pVfs->iVersion==1 ){
    jt_vfs.iVersion = 1;
  }অন্যথা যদি( g.pVfs->xCurrentTimeInt64==0 ){
    jt_vfs.xCurrentTimeInt64 = 0;
  }
  sqlite3_vfs_register(&jt_vfs, isDefault);
  ফেরত SQLITE_OK;
}

/*
** Uninstall the jt VFS, if it is installed.
*/
শূন্য jt_unregister(শূন্য){
  sqlite3_vfs_unregister(&jt_vfs);
}

#endif
