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
#જો SQLITE_TEST          /* This file is used for testing only */

#અન્તર્ભાવ "sqlite3.h"
#અન્તર્ભાવ "sqliteInt.h"

/*
** Maximum pathname length supported by the jt backend.
*/
#define JT_MAX_PATHNAME 512

/*
** Name used to identify this VFS.
*/
#define JT_VFS_NAME "jt"

પ્રકારવ્યાખ્યા સંરચના jt_file jt_file;
સંરચના jt_file {
  sqlite3_file base;
  અચલ અક્ષર *zName;       /* Name of open file */
  પૂર્ણાંક flags;               /* Flags the file was opened with */

  /* The following are only used by database file file handles */
  પૂર્ણાંક eLock;               /* Current lock held on the file */
  u32 nPage;               /* Size of file in pages when transaction started */
  u32 nPagesize;           /* Page size when transaction started */
  Bitvec *pWritable;       /* Bitvec of pages that may be written to the file */
  u32 *aCksum;             /* Checksum for first nPage pages */
  પૂર્ણાંક nSync;               /* Number of times journal file has been synced */

  /* Only used by journal file-handles */
  sqlite3_int64 iMaxOff;   /* Maximum offset written to this transaction */

  jt_file *pNext;          /* All files are stored in a linked list */
  sqlite3_file *pReal;     /* The file handle for the underlying vfs */
};

/*
** Method declarations for jt_file.
*/
સ્થિર પૂર્ણાંક jtClose(sqlite3_file*);
સ્થિર પૂર્ણાંક jtRead(sqlite3_file*, શૂન્ય*, પૂર્ણાંક iAmt, sqlite3_int64 iOfst);
સ્થિર પૂર્ણાંક jtWrite(sqlite3_file*,અચલ શૂન્ય*,પૂર્ણાંક iAmt, sqlite3_int64 iOfst);
સ્થિર પૂર્ણાંક jtTruncate(sqlite3_file*, sqlite3_int64 size);
સ્થિર પૂર્ણાંક jtSync(sqlite3_file*, પૂર્ણાંક flags);
સ્થિર પૂર્ણાંક jtFileSize(sqlite3_file*, sqlite3_int64 *pSize);
સ્થિર પૂર્ણાંક jtLock(sqlite3_file*, પૂર્ણાંક);
સ્થિર પૂર્ણાંક jtUnlock(sqlite3_file*, પૂર્ણાંક);
સ્થિર પૂર્ણાંક jtCheckReservedLock(sqlite3_file*, પૂર્ણાંક *);
સ્થિર પૂર્ણાંક jtFileControl(sqlite3_file*, પૂર્ણાંક op, શૂન્ય *pArg);
સ્થિર પૂર્ણાંક jtSectorSize(sqlite3_file*);
સ્થિર પૂર્ણાંક jtDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for jt_vfs.
*/
સ્થિર પૂર્ણાંક jtOpen(sqlite3_vfs*, અચલ અક્ષર *, sqlite3_file*, પૂર્ણાંક , પૂર્ણાંક *);
સ્થિર પૂર્ણાંક jtDelete(sqlite3_vfs*, અચલ અક્ષર *zName, પૂર્ણાંક syncDir);
સ્થિર પૂર્ણાંક jtAccess(sqlite3_vfs*, અચલ અક્ષર *zName, પૂર્ણાંક flags, પૂર્ણાંક *);
સ્થિર પૂર્ણાંક jtFullPathname(sqlite3_vfs*, અચલ અક્ષર *zName, પૂર્ણાંક, અક્ષર *zOut);
સ્થિર શૂન્ય *jtDlOpen(sqlite3_vfs*, અચલ અક્ષર *zFilename);
સ્થિર શૂન્ય jtDlError(sqlite3_vfs*, પૂર્ણાંક nByte, અક્ષર *z_Err_સંદેશ);
સ્થિર શૂન્ય (*jtDlSym(sqlite3_vfs*,શૂન્ય*, અચલ અક્ષર *z_પ્રતીક))(શૂન્ય);
સ્થિર શૂન્ય jtDlClose(sqlite3_vfs*, શૂન્ય*);
સ્થિર પૂર્ણાંક jtRandomness(sqlite3_vfs*, પૂર્ણાંક nByte, અક્ષર *zOut);
સ્થિર પૂર્ણાંક jtSleep(sqlite3_vfs*, પૂર્ણાંક microseconds);
સ્થિર પૂર્ણાંક jtCurrentTime(sqlite3_vfs*, દ્વિગુણ*);
સ્થિર પૂર્ણાંક jtCurrentTimeInt64(sqlite3_vfs*, sqlite3_int64*);
સ્થિર પૂર્ણાંક jtGetLastError(sqlite3_vfs*, પૂર્ણાંક, અક્ષર*);

સ્થિર sqlite3_vfs jt_vfs = {
  2,                             /* iVersion */
  કદ(jt_file),               /* szOsFile */
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

સ્થિર sqlite3_io_methods jt_io_methods = {
  1,                             /* iVersion */
  jtClose,                       /* xClose */
  jtRead,                        /* xRead */
  jtWrite,                       /* xWrite */
  jtTruncate,                    /* xTruncate */
  jtSync,                        /* xSync */
  jtFileSize,                    /* xFileSize */
  jtLock,                        /* xLock */
  jtUnlock,                      /* xUnlock */
  jtCheckReservedLock,           /* xCheckReservedLock */
  jtFileControl,                 /* xFileControl */
  jtSectorSize,                  /* xSectorSize */
  jtDeviceCharacteristics        /* xDeviceCharacteristics */
};

સંરચના JtGlobal {
  sqlite3_vfs *pVfs;             /* Parent VFS */
  jt_file *pList;                /* List of all open files */
};
સ્થિર સંરચના JtGlobal g = {0, 0};

/*
** Functions to obtain and relinquish a mutex to protect g.pList. The
** STATIC_PRNG mutex is reused, purely for the sake of convenience.
*/
સ્થિર શૂન્ય enterJtMutex(શૂન્ય){
  sqlite3_mutex_enter(sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_PRNG));
}
સ્થિર શૂન્ય leaveJtMutex(શૂન્ય){
  sqlite3_mutex_leave(sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_PRNG));
}

બાહ્ય પૂર્ણાંક sqlite3_io_error_pending;
બાહ્ય પૂર્ણાંક sqlite3_io_error_hit;
સ્થિર શૂન્ય stop_ioerr_simulation(પૂર્ણાંક *piSave, પૂર્ણાંક *piSave2){
  *piSave = sqlite3_io_error_pending;
  *piSave2 = sqlite3_io_error_hit;
  sqlite3_io_error_pending = -1;
  sqlite3_io_error_hit = 0;
}
સ્થિર શૂન્ય start_ioerr_simulation(પૂર્ણાંક iSave, પૂર્ણાંક iSave2){
  sqlite3_io_error_pending = iSave;
  sqlite3_io_error_hit = iSave2;
}

/*
** The jt_file pointed to by the argument may or may not be a file-handle
** open on a main database file. If it is, and a transaction is currently
** opened on the file, then discard all transaction related data.
*/
સ્થિર શૂન્ય closeTransaction(jt_file *p){
  sqlite3BitvecDestroy(p->pWritable);
  sqlite3_free(p->aCksum);
  p->pWritable = 0;
  p->aCksum = 0;
  p->nSync = 0;
}

/*
** Close an jt-file.
*/
સ્થિર પૂર્ણાંક jtClose(sqlite3_file *pFile){
  jt_file **pp;
  jt_file *p = (jt_file *)pFile;

  closeTransaction(p);
  enterJtMutex();
  જો( p->zName ){
    માટે(pp=&g.pList; *pp!=p; pp=&(*pp)->pNext);
    *pp = p->pNext;
  }
  leaveJtMutex();
  sqlite3OsClose(p->pReal);
  પાછા SQLITE_OK;
}

/*
** Read data from an jt-file.
*/
સ્થિર પૂર્ણાંક jtRead(
  sqlite3_file *pFile, 
  શૂન્ય *zBuf, 
  પૂર્ણાંક iAmt, 
  sqlite_int64 iOfst
){
  jt_file *p = (jt_file *)pFile;
  પાછા sqlite3OsRead(p->pReal, zBuf, iAmt, iOfst);
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
સ્થિર jt_file *locateDatabaseHandle(અચલ અક્ષર *zJournal, પૂર્ણાંક noLock){
  jt_file *pMain = 0;
  enterJtMutex();
  માટે(pMain=g.pList; pMain; pMain=pMain->pNext){
    પૂર્ણાંક nName = (પૂર્ણાંક)(strlen(zJournal) - strlen("-journal"));
    જો( (pMain->flags&SQLITE_OPEN_MAIN_DB)
     && ((પૂર્ણાંક)strlen(pMain->zName)==nName)
     && 0==memcmp(pMain->zName, zJournal, nName)
     && ((pMain->eLock>=SQLITE_LOCK_RESERVED) || noLock)
    ){
      તોડો;
    }
  }
  leaveJtMutex();
  પાછા pMain;
}

/*
** Parameter z points to a buffer of 4 bytes in size containing a 
** unsigned 32-bit integer stored in big-endian format. Decode the 
** integer and return its value.
*/
સ્થિર u32 decodeUint32(અચલ unsigned અક્ષર *z){
  પાછા (z[0]<<24) + (z[1]<<16) + (z[2]<<8) + z[3];
}

/*
** Calculate a checksum from the buffer of length n bytes pointed to
** by parameter z.
*/
સ્થિર u32 genCksum(અચલ unsigned અક્ષર *z, પૂર્ણાંક n){
  પૂર્ણાંક i;
  u32 cksum = 0;
  માટે(i=0; i<n; i++){
    cksum = cksum + z[i] + (cksum<<3);
  }
  પાછા cksum;
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
સ્થિર પૂર્ણાંક decodeJournalHdr(
  અચલ unsigned અક્ષર *zBuf,         /* Input: 28 byte journal header */
  u32 *pnRec,                        /* Out: Number of journalled records */
  u32 *pnPage,                       /* Out: Original database page count */
  u32 *pnSector,                     /* Out: Sector size in bytes */
  u32 *pnPagesize                    /* Out: Page size in bytes */
){
  unsigned અક્ષર aMagic[] = { 0xd9, 0xd5, 0x05, 0xf9, 0x20, 0xa1, 0x63, 0xd7 };
  જો( memcmp(aMagic, zBuf, 8) ) પાછા SQLITE_ERROR;
  જો( pnRec ) *pnRec = decodeUint32(&zBuf[8]);
  જો( pnPage ) *pnPage = decodeUint32(&zBuf[16]);
  જો( pnSector ) *pnSector = decodeUint32(&zBuf[20]);
  જો( pnPagesize ) *pnPagesize = decodeUint32(&zBuf[24]);
  પાછા SQLITE_OK;
}

/*
** This function is called when a new transaction is opened, just after
** the first journal-header is written to the journal file.
*/
સ્થિર પૂર્ણાંક openTransaction(jt_file *pMain, jt_file *pJournal){
  unsigned અક્ષર *aData;
  sqlite3_file *p = pMain->pReal;
  પૂર્ણાંક rc = SQLITE_OK;

  closeTransaction(pMain);
  aData = sqlite3_malloc(pMain->nPagesize);
  pMain->pWritable = sqlite3BitvecCreate(pMain->nPage);
  pMain->aCksum = sqlite3_malloc(કદ(u32) * (pMain->nPage + 1));
  pJournal->iMaxOff = 0;

  જો( !pMain->pWritable || !pMain->aCksum || !aData ){
    rc = SQLITE_IOERR_NOMEM;
  }નહીંતર જો( pMain->nPage>0 ){
    u32 iTrunk;
    પૂર્ણાંક iSave;
    પૂર્ણાંક iSave2;

    stop_ioerr_simulation(&iSave, &iSave2);

    /* Read the database free-list. Add the page-number for each free-list
    ** leaf to the jt_file.pWritable bitvec.
    */
    rc = sqlite3OsRead(p, aData, pMain->nPagesize, 0);
    જો( rc==SQLITE_OK ){
      u32 nDbsize = decodeUint32(&aData[28]);
      જો( nDbsize>0 && memcmp(&aData[24], &aData[92], 4)==0 ){
        u32 iPg;
        માટે(iPg=nDbsize+1; iPg<=pMain->nPage; iPg++){
          sqlite3BitvecSet(pMain->pWritable, iPg);
        }
      }
    }
    iTrunk = decodeUint32(&aData[32]);
    જ્યારે( rc==SQLITE_OK && iTrunk>0 ){
      u32 nLeaf;
      u32 iLeaf;
      sqlite3_int64 iOff = (i64)(iTrunk-1)*pMain->nPagesize;
      rc = sqlite3OsRead(p, aData, pMain->nPagesize, iOff);
      nLeaf = decodeUint32(&aData[4]);
      માટે(iLeaf=0; rc==SQLITE_OK && iLeaf<nLeaf; iLeaf++){
        u32 pgno = decodeUint32(&aData[8+4*iLeaf]);
        sqlite3BitvecSet(pMain->pWritable, pgno);
      }
      iTrunk = decodeUint32(aData);
    }

    /* Calculate and store a checksum for each page in the database file. */
    જો( rc==SQLITE_OK ){
      પૂર્ણાંક ii;
      માટે(ii=0; rc==SQLITE_OK && ii<(પૂર્ણાંક)pMain->nPage; ii++){
        i64 iOff = (i64)(pMain->nPagesize) * (i64)ii;
        જો( iOff==PENDING_BYTE ) ચાલુ;
        rc = sqlite3OsRead(pMain->pReal, aData, pMain->nPagesize, iOff);
        pMain->aCksum[ii] = genCksum(aData, pMain->nPagesize);
        જો( ii+1==(પૂર્ણાંક)pMain->nPage && rc==SQLITE_IOERR_SHORT_READ ){
          rc = SQLITE_OK;
        }
      }
    }

    start_ioerr_simulation(iSave, iSave2);
  }

  sqlite3_free(aData);
  પાછા rc;
}

/*
** The first argument to this function is a handle open on a journal file.
** This function reads the journal file and adds the page number for each
** page in the journal to the Bitvec object passed as the second argument.
*/
સ્થિર પૂર્ણાંક readJournalFile(jt_file *p, jt_file *pMain){
  પૂર્ણાંક rc = SQLITE_OK;
  unsigned અક્ષર zBuf[28];
  sqlite3_file *pReal = p->pReal;
  sqlite3_int64 iOff = 0;
  sqlite3_int64 iSize = p->iMaxOff;
  unsigned અક્ષર *aPage;
  પૂર્ણાંક iSave;
  પૂર્ણાંક iSave2;

  aPage = sqlite3_malloc(pMain->nPagesize);
  જો( !aPage ){
    પાછા SQLITE_IOERR_NOMEM;
  }

  stop_ioerr_simulation(&iSave, &iSave2);

  જ્યારે( rc==SQLITE_OK && iOff<iSize ){
    u32 nRec, nPage, nSector, nPagesize;
    u32 ii;

    /* Read and decode the next journal-header from the journal file. */
    rc = sqlite3OsRead(pReal, zBuf, 28, iOff);
    જો( rc!=SQLITE_OK 
     || decodeJournalHdr(zBuf, &nRec, &nPage, &nSector, &nPagesize) 
    ){
      જાઓ finish_rjf;
    }
    iOff += nSector;

    જો( nRec==0 ){
      /* A trick. There might be another journal-header immediately 
      ** following this one. In this case, 0 records means 0 records, 
      ** not "read until the end of the file". See also ticket #2565.
      */
      જો( iSize>=(iOff+nSector) ){
        rc = sqlite3OsRead(pReal, zBuf, 28, iOff);
        જો( rc!=SQLITE_OK || 0==decodeJournalHdr(zBuf, 0, 0, 0, 0) ){
          ચાલુ;
        }
      }
      nRec = (u32)((iSize-iOff) / (pMain->nPagesize+8));
    }

    /* Read all the records that follow the journal-header just read. */
    માટે(ii=0; rc==SQLITE_OK && ii<nRec && iOff<iSize; ii++){
      u32 pgno;
      rc = sqlite3OsRead(pReal, zBuf, 4, iOff);
      જો( rc==SQLITE_OK ){
        pgno = decodeUint32(zBuf);
        જો( pgno>0 && pgno<=pMain->nPage ){
          જો( 0==sqlite3BitvecTest(pMain->pWritable, pgno) ){
            rc = sqlite3OsRead(pReal, aPage, pMain->nPagesize, iOff+4);
            જો( rc==SQLITE_OK ){
              u32 cksum = genCksum(aPage, pMain->nPagesize);
              પ્રતિજ્ઞા( cksum==pMain->aCksum[pgno-1] );
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
  જો( rc==SQLITE_IOERR_SHORT_READ ){
    rc = SQLITE_OK;
  }
  પાછા rc;
}

/*
** Write data to an jt-file.
*/
સ્થિર પૂર્ણાંક jtWrite(
  sqlite3_file *pFile, 
  અચલ શૂન્ય *zBuf, 
  પૂર્ણાંક iAmt, 
  sqlite_int64 iOfst
){
  પૂર્ણાંક rc;
  jt_file *p = (jt_file *)pFile;
  જો( p->flags&SQLITE_OPEN_MAIN_JOURNAL ){
    જો( iOfst==0 ){
      jt_file *pMain = locateDatabaseHandle(p->zName, 0);
      પ્રતિજ્ઞા( pMain );
  
      જો( iAmt==28 ){
        /* Zeroing the first journal-file header. This is the end of a
        ** transaction. */
        closeTransaction(pMain);
      }નહીંતર જો( iAmt!=12 ){
        /* Writing the first journal header to a journal file. This happens
        ** when a transaction is first started.  */
        u8 *z = (u8 *)zBuf;
        pMain->nPage = decodeUint32(&z[16]);
        pMain->nPagesize = decodeUint32(&z[24]);
        જો( SQLITE_OK!=(rc=openTransaction(pMain, p)) ){
          પાછા rc;
        }
      }
    }
    જો( p->iMaxOff<(iOfst + iAmt) ){
      p->iMaxOff = iOfst + iAmt;
    }
  }

  જો( p->flags&SQLITE_OPEN_MAIN_DB && p->pWritable ){
    જો( iAmt<(પૂર્ણાંક)p->nPagesize 
     && p->nPagesize%iAmt==0 
     && iOfst>=(PENDING_BYTE+512) 
     && iOfst+iAmt<=PENDING_BYTE+p->nPagesize
    ){
      /* No-op. This special case is hit when the backup code is copying a
      ** to a database with a larger page-size than the source database and
      ** it needs to fill in the non-locking-region part of the original
      ** pending-byte page.
      */
    }નહીંતર{
      u32 pgno = (u32)(iOfst/p->nPagesize + 1);
      પ્રતિજ્ઞા( (iAmt==1||iAmt==(પૂર્ણાંક)p->nPagesize) &&
              ((iOfst+iAmt)%p->nPagesize)==0 );
      /* The following assert() statements may fail if this layer is used
      ** with a connection in "PRAGMA synchronous=off" mode. If they
      ** fail with sync=normal or sync=full, this may indicate problem.  */
      પ્રતિજ્ઞા( p->nPage==0 || pgno<=p->nPage || p->nSync>0 );
      પ્રતિજ્ઞા( pgno>p->nPage || sqlite3BitvecTest(p->pWritable, pgno) );
    }
  }

  rc = sqlite3OsWrite(p->pReal, zBuf, iAmt, iOfst);
  જો( (p->flags&SQLITE_OPEN_MAIN_JOURNAL) && iAmt==12 ){
    jt_file *pMain = locateDatabaseHandle(p->zName, 0);
    પૂર્ણાંક rc2 = readJournalFile(p, pMain);
    જો( rc==SQLITE_OK ) rc = rc2;
  }
  પાછા rc;
}

/*
** Truncate an jt-file.
*/
સ્થિર પૂર્ણાંક jtTruncate(sqlite3_file *pFile, sqlite_int64 size){
  jt_file *p = (jt_file *)pFile;
  જો( p->flags&SQLITE_OPEN_MAIN_JOURNAL && size==0 ){
    /* Truncating a journal file. This is the end of a transaction. */
    jt_file *pMain = locateDatabaseHandle(p->zName, 0);
    closeTransaction(pMain);
  }
  જો( p->flags&SQLITE_OPEN_MAIN_DB && p->pWritable ){
    u32 pgno;
    u32 locking_page = (u32)(PENDING_BYTE/p->nPagesize+1);
    માટે(pgno=(u32)(size/p->nPagesize+1); pgno<=p->nPage; pgno++){
      પ્રતિજ્ઞા( pgno==locking_page || sqlite3BitvecTest(p->pWritable, pgno) );
    }
  }
  પાછા sqlite3OsTruncate(p->pReal, size);
}

/*
** Sync an jt-file.
*/
સ્થિર પૂર્ણાંક jtSync(sqlite3_file *pFile, પૂર્ણાંક flags){
  jt_file *p = (jt_file *)pFile;

  જો( p->flags&SQLITE_OPEN_MAIN_JOURNAL ){
    પૂર્ણાંક rc;
    jt_file *pMain;                   /* The associated database file */

    /* The journal file is being synced. At this point, we inspect the 
    ** contents of the file up to this point and set each bit in the 
    ** jt_file.pWritable bitvec of the main database file associated with
    ** this journal file.
    */
    pMain = locateDatabaseHandle(p->zName, 0);

    /* Set the bitvec values */
    જો( pMain && pMain->pWritable ){
      pMain->nSync++;
      rc = readJournalFile(p, pMain);
      જો( rc!=SQLITE_OK ){
        પાછા rc;
      }
    }
  }

  પાછા sqlite3OsSync(p->pReal, flags);
}

/*
** Return the current file-size of an jt-file.
*/
સ્થિર પૂર્ણાંક jtFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  jt_file *p = (jt_file *)pFile;
  પાછા sqlite3OsFileSize(p->pReal, pSize);
}

/*
** Lock an jt-file.
*/
સ્થિર પૂર્ણાંક jtLock(sqlite3_file *pFile, પૂર્ણાંક eLock){
  પૂર્ણાંક rc;
  jt_file *p = (jt_file *)pFile;
  rc = sqlite3OsLock(p->pReal, eLock);
  જો( rc==SQLITE_OK && eLock>p->eLock ){
    p->eLock = eLock;
  }
  પાછા rc;
}

/*
** Unlock an jt-file.
*/
સ્થિર પૂર્ણાંક jtUnlock(sqlite3_file *pFile, પૂર્ણાંક eLock){
  પૂર્ણાંક rc;
  jt_file *p = (jt_file *)pFile;
  rc = sqlite3OsUnlock(p->pReal, eLock);
  જો( rc==SQLITE_OK && eLock<p->eLock ){
    p->eLock = eLock;
  }
  પાછા rc;
}

/*
** Check if another file-handle holds a RESERVED lock on an jt-file.
*/
સ્થિર પૂર્ણાંક jtCheckReservedLock(sqlite3_file *pFile, પૂર્ણાંક *pResOut){
  jt_file *p = (jt_file *)pFile;
  પાછા sqlite3OsCheckReservedLock(p->pReal, pResOut);
}

/*
** File control method. For custom operations on an jt-file.
*/
સ્થિર પૂર્ણાંક jtFileControl(sqlite3_file *pFile, પૂર્ણાંક op, શૂન્ય *pArg){
  jt_file *p = (jt_file *)pFile;
  પાછા p->pReal->pMethods->xFileControl(p->pReal, op, pArg);
}

/*
** Return the sector-size in bytes for an jt-file.
*/
સ્થિર પૂર્ણાંક jtSectorSize(sqlite3_file *pFile){
  jt_file *p = (jt_file *)pFile;
  પાછા sqlite3OsSectorSize(p->pReal);
}

/*
** Return the device characteristic flags supported by an jt-file.
*/
સ્થિર પૂર્ણાંક jtDeviceCharacteristics(sqlite3_file *pFile){
  jt_file *p = (jt_file *)pFile;
  પાછા sqlite3OsDeviceCharacteristics(p->pReal);
}

/*
** Open an jt file handle.
*/
સ્થિર પૂર્ણાંક jtOpen(
  sqlite3_vfs *pVfs,
  અચલ અક્ષર *zName,
  sqlite3_file *pFile,
  પૂર્ણાંક flags,
  પૂર્ણાંક *pOutFlags
){
  પૂર્ણાંક rc;
  jt_file *p = (jt_file *)pFile;
  pFile->pMethods = 0;
  p->pReal = (sqlite3_file *)&p[1];
  p->pReal->pMethods = 0;
  rc = sqlite3OsOpen(g.pVfs, zName, p->pReal, flags, pOutFlags);
  પ્રતિજ્ઞા( rc==SQLITE_OK || p->pReal->pMethods==0 );
  જો( rc==SQLITE_OK ){
    pFile->pMethods = &jt_io_methods;
    p->eLock = 0;
    p->zName = zName;
    p->flags = flags;
    p->pNext = 0;
    p->pWritable = 0;
    p->aCksum = 0;
    enterJtMutex();
    જો( zName ){
      p->pNext = g.pList;
      g.pList = p;
    }
    leaveJtMutex();
  }
  પાછા rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
સ્થિર પૂર્ણાંક jtDelete(sqlite3_vfs *pVfs, અચલ અક્ષર *zPath, પૂર્ણાંક dirSync){
  પૂર્ણાંક nPath = (પૂર્ણાંક)strlen(zPath);
  જો( nPath>8 && 0==strcmp("-journal", &zPath[nPath-8]) ){
    /* Deleting a journal file. The end of a transaction. */
    jt_file *pMain = locateDatabaseHandle(zPath, 0);
    જો( pMain ){
      closeTransaction(pMain);
    }
  }

  પાછા sqlite3OsDelete(g.pVfs, zPath, dirSync);
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
સ્થિર પૂર્ણાંક jtAccess(
  sqlite3_vfs *pVfs, 
  અચલ અક્ષર *zPath, 
  પૂર્ણાંક flags, 
  પૂર્ણાંક *pResOut
){
  પાછા sqlite3OsAccess(g.pVfs, zPath, flags, pResOut);
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (JT_MAX_PATHNAME+1) bytes.
*/
સ્થિર પૂર્ણાંક jtFullPathname(
  sqlite3_vfs *pVfs, 
  અચલ અક્ષર *zPath, 
  પૂર્ણાંક nOut, 
  અક્ષર *zOut
){
  પાછા sqlite3OsFullPathname(g.pVfs, zPath, nOut, zOut);
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
સ્થિર શૂન્ય *jtDlOpen(sqlite3_vfs *pVfs, અચલ અક્ષર *zPath){
  પાછા g.pVfs->xDlOpen(g.pVfs, zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
સ્થિર શૂન્ય jtDlError(sqlite3_vfs *pVfs, પૂર્ણાંક nByte, અક્ષર *z_Err_સંદેશ){
  g.pVfs->xDlError(g.pVfs, nByte, z_Err_સંદેશ);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
સ્થિર શૂન્ય (*jtDlSym(sqlite3_vfs *pVfs, શૂન્ય *p, અચલ અક્ષર *zSym))(શૂન્ય){
  પાછા g.pVfs->xDlSym(g.pVfs, p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
સ્થિર શૂન્ય jtDlClose(sqlite3_vfs *pVfs, શૂન્ય *pHandle){
  g.pVfs->xDlClose(g.pVfs, pHandle);
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
સ્થિર પૂર્ણાંક jtRandomness(sqlite3_vfs *pVfs, પૂર્ણાંક nByte, અક્ષર *zBufOut){
  પાછા sqlite3OsRandomness(g.pVfs, nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
સ્થિર પૂર્ણાંક jtSleep(sqlite3_vfs *pVfs, પૂર્ણાંક nMicro){
  પાછા sqlite3OsSleep(g.pVfs, nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
સ્થિર પૂર્ણાંક jtCurrentTime(sqlite3_vfs *pVfs, દ્વિગુણ *pTimeOut){
  પાછા g.pVfs->xCurrentTime(g.pVfs, pTimeOut);
}
/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
સ્થિર પૂર્ણાંક jtCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *pTimeOut){
  પાછા g.pVfs->xCurrentTimeInt64(g.pVfs, pTimeOut);
}

સ્થિર પૂર્ણાંક jtGetLastError(sqlite3_vfs *pVfs, પૂર્ણાંક n, અક્ષર *z){
  પાછા g.pVfs->xGetLastError(g.pVfs, n, z);
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
પૂર્ણાંક jt_register(અક્ષર *zWrap, પૂર્ણાંક isDefault){
  g.pVfs = sqlite3_vfs_find(zWrap);
  જો( g.pVfs==0 ){
    પાછા SQLITE_ERROR;
  }
  jt_vfs.szOsFile = કદ(jt_file) + g.pVfs->szOsFile;
  જો( g.pVfs->iVersion==1 ){
    jt_vfs.iVersion = 1;
  }નહીંતર જો( g.pVfs->xCurrentTimeInt64==0 ){
    jt_vfs.xCurrentTimeInt64 = 0;
  }
  sqlite3_vfs_register(&jt_vfs, isDefault);
  પાછા SQLITE_OK;
}

/*
** Uninstall the jt VFS, if it is installed.
*/
શૂન્ય jt_unregister(શૂન્ય){
  sqlite3_vfs_unregister(&jt_vfs);
}

#endif
