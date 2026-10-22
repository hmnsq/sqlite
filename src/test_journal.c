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
#ถ้า SQLITE_TEST          /* This file is used for testing only */

#include "sqlite3.h"
#include "sqliteInt.h"

/*
** Maximum pathname length supported by the jt backend.
*/
#define JT_MAX_PATHNAME 512

/*
** Name used to identify this VFS.
*/
#define JT_VFS_NAME "jt"

typedef sa_nracanaa jt_file jt_file;
sa_nracanaa jt_file {
  sqlite3_file base;
  const var_na *zName;       /* Name of open file */
  จำนวนเต็ม flags;               /* Flags the file was opened with */

  /* The following are only used by database file file handles */
  จำนวนเต็ม eLock;               /* Current lock held on the file */
  u32 nPage;               /* Size of file in pages when transaction started */
  u32 nPagesize;           /* Page size when transaction started */
  Bitvec *pWritable;       /* Bitvec of pages that may be written to the file */
  u32 *aCksum;             /* Checksum for first nPage pages */
  จำนวนเต็ม nSync;               /* Number of times journal file has been synced */

  /* Only used by journal file-handles */
  sqlite3_int64 iMaxOff;   /* Maximum offset written to this transaction */

  jt_file *pNext;          /* All files are stored in a linked list */
  sqlite3_file *pReal;     /* The file handle for the underlying vfs */
};

/*
** Method declarations for jt_file.
*/
static จำนวนเต็ม jtClose(sqlite3_file*);
static จำนวนเต็ม jtRead(sqlite3_file*, void*, จำนวนเต็ม iAmt, sqlite3_int64 iOfst);
static จำนวนเต็ม jtWrite(sqlite3_file*,const void*,จำนวนเต็ม iAmt, sqlite3_int64 iOfst);
static จำนวนเต็ม jtTruncate(sqlite3_file*, sqlite3_int64 size);
static จำนวนเต็ม jtSync(sqlite3_file*, จำนวนเต็ม flags);
static จำนวนเต็ม jtFileSize(sqlite3_file*, sqlite3_int64 *pSize);
static จำนวนเต็ม jtLock(sqlite3_file*, จำนวนเต็ม);
static จำนวนเต็ม jtUnlock(sqlite3_file*, จำนวนเต็ม);
static จำนวนเต็ม jtCheckReservedLock(sqlite3_file*, จำนวนเต็ม *);
static จำนวนเต็ม jtFileControl(sqlite3_file*, จำนวนเต็ม op, void *pArg);
static จำนวนเต็ม jtSectorSize(sqlite3_file*);
static จำนวนเต็ม jtDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for jt_vfs.
*/
static จำนวนเต็ม jtOpen(sqlite3_vfs*, const var_na *, sqlite3_file*, จำนวนเต็ม , จำนวนเต็ม *);
static จำนวนเต็ม jtDelete(sqlite3_vfs*, const var_na *zName, จำนวนเต็ม syncDir);
static จำนวนเต็ม jtAccess(sqlite3_vfs*, const var_na *zName, จำนวนเต็ม flags, จำนวนเต็ม *);
static จำนวนเต็ม jtFullPathname(sqlite3_vfs*, const var_na *zName, จำนวนเต็ม, var_na *zOut);
static void *jtDlOpen(sqlite3_vfs*, const var_na *zFilename);
static void jtDlError(sqlite3_vfs*, จำนวนเต็ม nByte, var_na *zErrMsg);
static void (*jtDlSym(sqlite3_vfs*,void*, const var_na *zSymbol))(void);
static void jtDlClose(sqlite3_vfs*, void*);
static จำนวนเต็ม jtRandomness(sqlite3_vfs*, จำนวนเต็ม nByte, var_na *zOut);
static จำนวนเต็ม jtSleep(sqlite3_vfs*, จำนวนเต็ม microseconds);
static จำนวนเต็ม jtCurrentTime(sqlite3_vfs*, dviguu_na*);
static จำนวนเต็ม jtCurrentTimeInt64(sqlite3_vfs*, sqlite3_int64*);
static จำนวนเต็ม jtGetLastError(sqlite3_vfs*, จำนวนเต็ม, var_na*);

static sqlite3_vfs jt_vfs = {
  2,                             /* iVersion */
  parimaa_na(jt_file),               /* szOsFile */
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

static sqlite3_io_methods jt_io_methods = {
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

sa_nracanaa JtGlobal {
  sqlite3_vfs *pVfs;             /* Parent VFS */
  jt_file *pList;                /* List of all open files */
};
static sa_nracanaa JtGlobal g = {0, 0};

/*
** Functions to obtain and relinquish a mutex to protect g.pList. The
** STATIC_PRNG mutex is reused, purely for the sake of convenience.
*/
static void enterJtMutex(void){
  sqlite3_mutex_enter(sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_PRNG));
}
static void leaveJtMutex(void){
  sqlite3_mutex_leave(sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_PRNG));
}

extern จำนวนเต็ม sqlite3_io_error_pending;
extern จำนวนเต็ม sqlite3_io_error_hit;
static void stop_ioerr_simulation(จำนวนเต็ม *piSave, จำนวนเต็ม *piSave2){
  *piSave = sqlite3_io_error_pending;
  *piSave2 = sqlite3_io_error_hit;
  sqlite3_io_error_pending = -1;
  sqlite3_io_error_hit = 0;
}
static void start_ioerr_simulation(จำนวนเต็ม iSave, จำนวนเต็ม iSave2){
  sqlite3_io_error_pending = iSave;
  sqlite3_io_error_hit = iSave2;
}

/*
** The jt_file pointed to by the argument may or may not be a file-handle
** open on a main database file. If it is, and a transaction is currently
** opened on the file, then discard all transaction related data.
*/
static void closeTransaction(jt_file *p){
  sqlite3BitvecDestroy(p->pWritable);
  sqlite3_free(p->aCksum);
  p->pWritable = 0;
  p->aCksum = 0;
  p->nSync = 0;
}

/*
** Close an jt-file.
*/
static จำนวนเต็ม jtClose(sqlite3_file *pFile){
  jt_file **pp;
  jt_file *p = (jt_file *)pFile;

  closeTransaction(p);
  enterJtMutex();
  ถ้า( p->zName ){
    สำหรับ(pp=&g.pList; *pp!=p; pp=&(*pp)->pNext);
    *pp = p->pNext;
  }
  leaveJtMutex();
  sqlite3OsClose(p->pReal);
  คืน SQLITE_OK;
}

/*
** Read data from an jt-file.
*/
static จำนวนเต็ม jtRead(
  sqlite3_file *pFile, 
  void *zBuf, 
  จำนวนเต็ม iAmt, 
  sqlite_int64 iOfst
){
  jt_file *p = (jt_file *)pFile;
  คืน sqlite3OsRead(p->pReal, zBuf, iAmt, iOfst);
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
static jt_file *locateDatabaseHandle(const var_na *zJournal, จำนวนเต็ม noLock){
  jt_file *pMain = 0;
  enterJtMutex();
  สำหรับ(pMain=g.pList; pMain; pMain=pMain->pNext){
    จำนวนเต็ม nName = (จำนวนเต็ม)(strlen(zJournal) - strlen("-journal"));
    ถ้า( (pMain->flags&SQLITE_OPEN_MAIN_DB)
     && ((จำนวนเต็ม)strlen(pMain->zName)==nName)
     && 0==memcmp(pMain->zName, zJournal, nName)
     && ((pMain->eLock>=SQLITE_LOCK_RESERVED) || noLock)
    ){
      bha_nga;
    }
  }
  leaveJtMutex();
  คืน pMain;
}

/*
** Parameter z points to a buffer of 4 bytes in size containing a 
** unsigned 32-bit integer stored in big-endian format. Decode the 
** integer and return its value.
*/
static u32 decodeUint32(const unsigned var_na *z){
  คืน (z[0]<<24) + (z[1]<<16) + (z[2]<<8) + z[3];
}

/*
** Calculate a checksum from the buffer of length n bytes pointed to
** by parameter z.
*/
static u32 genCksum(const unsigned var_na *z, จำนวนเต็ม n){
  จำนวนเต็ม i;
  u32 cksum = 0;
  สำหรับ(i=0; i<n; i++){
    cksum = cksum + z[i] + (cksum<<3);
  }
  คืน cksum;
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
static จำนวนเต็ม decodeJournalHdr(
  const unsigned var_na *zBuf,         /* Input: 28 byte journal header */
  u32 *pnRec,                        /* Out: Number of journalled records */
  u32 *pnPage,                       /* Out: Original database page count */
  u32 *pnSector,                     /* Out: Sector size in bytes */
  u32 *pnPagesize                    /* Out: Page size in bytes */
){
  unsigned var_na aMagic[] = { 0xd9, 0xd5, 0x05, 0xf9, 0x20, 0xa1, 0x63, 0xd7 };
  ถ้า( memcmp(aMagic, zBuf, 8) ) คืน SQLITE_ERROR;
  ถ้า( pnRec ) *pnRec = decodeUint32(&zBuf[8]);
  ถ้า( pnPage ) *pnPage = decodeUint32(&zBuf[16]);
  ถ้า( pnSector ) *pnSector = decodeUint32(&zBuf[20]);
  ถ้า( pnPagesize ) *pnPagesize = decodeUint32(&zBuf[24]);
  คืน SQLITE_OK;
}

/*
** This function is called when a new transaction is opened, just after
** the first journal-header is written to the journal file.
*/
static จำนวนเต็ม openTransaction(jt_file *pMain, jt_file *pJournal){
  unsigned var_na *aData;
  sqlite3_file *p = pMain->pReal;
  จำนวนเต็ม rc = SQLITE_OK;

  closeTransaction(pMain);
  aData = sqlite3_malloc(pMain->nPagesize);
  pMain->pWritable = sqlite3BitvecCreate(pMain->nPage);
  pMain->aCksum = sqlite3_malloc(parimaa_na(u32) * (pMain->nPage + 1));
  pJournal->iMaxOff = 0;

  ถ้า( !pMain->pWritable || !pMain->aCksum || !aData ){
    rc = SQLITE_IOERR_NOMEM;
  }มิฉะนั้น ถ้า( pMain->nPage>0 ){
    u32 iTrunk;
    จำนวนเต็ม iSave;
    จำนวนเต็ม iSave2;

    stop_ioerr_simulation(&iSave, &iSave2);

    /* Read the database free-list. Add the page-number for each free-list
    ** leaf to the jt_file.pWritable bitvec.
    */
    rc = sqlite3OsRead(p, aData, pMain->nPagesize, 0);
    ถ้า( rc==SQLITE_OK ){
      u32 nDbsize = decodeUint32(&aData[28]);
      ถ้า( nDbsize>0 && memcmp(&aData[24], &aData[92], 4)==0 ){
        u32 iPg;
        สำหรับ(iPg=nDbsize+1; iPg<=pMain->nPage; iPg++){
          sqlite3BitvecSet(pMain->pWritable, iPg);
        }
      }
    }
    iTrunk = decodeUint32(&aData[32]);
    ขณะ( rc==SQLITE_OK && iTrunk>0 ){
      u32 nLeaf;
      u32 iLeaf;
      sqlite3_int64 iOff = (i64)(iTrunk-1)*pMain->nPagesize;
      rc = sqlite3OsRead(p, aData, pMain->nPagesize, iOff);
      nLeaf = decodeUint32(&aData[4]);
      สำหรับ(iLeaf=0; rc==SQLITE_OK && iLeaf<nLeaf; iLeaf++){
        u32 pgno = decodeUint32(&aData[8+4*iLeaf]);
        sqlite3BitvecSet(pMain->pWritable, pgno);
      }
      iTrunk = decodeUint32(aData);
    }

    /* Calculate and store a checksum for each page in the database file. */
    ถ้า( rc==SQLITE_OK ){
      จำนวนเต็ม ii;
      สำหรับ(ii=0; rc==SQLITE_OK && ii<(จำนวนเต็ม)pMain->nPage; ii++){
        i64 iOff = (i64)(pMain->nPagesize) * (i64)ii;
        ถ้า( iOff==PENDING_BYTE ) continue;
        rc = sqlite3OsRead(pMain->pReal, aData, pMain->nPagesize, iOff);
        pMain->aCksum[ii] = genCksum(aData, pMain->nPagesize);
        ถ้า( ii+1==(จำนวนเต็ม)pMain->nPage && rc==SQLITE_IOERR_SHORT_READ ){
          rc = SQLITE_OK;
        }
      }
    }

    start_ioerr_simulation(iSave, iSave2);
  }

  sqlite3_free(aData);
  คืน rc;
}

/*
** The first argument to this function is a handle open on a journal file.
** This function reads the journal file and adds the page number for each
** page in the journal to the Bitvec object passed as the second argument.
*/
static จำนวนเต็ม readJournalFile(jt_file *p, jt_file *pMain){
  จำนวนเต็ม rc = SQLITE_OK;
  unsigned var_na zBuf[28];
  sqlite3_file *pReal = p->pReal;
  sqlite3_int64 iOff = 0;
  sqlite3_int64 iSize = p->iMaxOff;
  unsigned var_na *aPage;
  จำนวนเต็ม iSave;
  จำนวนเต็ม iSave2;

  aPage = sqlite3_malloc(pMain->nPagesize);
  ถ้า( !aPage ){
    คืน SQLITE_IOERR_NOMEM;
  }

  stop_ioerr_simulation(&iSave, &iSave2);

  ขณะ( rc==SQLITE_OK && iOff<iSize ){
    u32 nRec, nPage, nSector, nPagesize;
    u32 ii;

    /* Read and decode the next journal-header from the journal file. */
    rc = sqlite3OsRead(pReal, zBuf, 28, iOff);
    ถ้า( rc!=SQLITE_OK 
     || decodeJournalHdr(zBuf, &nRec, &nPage, &nSector, &nPagesize) 
    ){
      goto finish_rjf;
    }
    iOff += nSector;

    ถ้า( nRec==0 ){
      /* A trick. There might be another journal-header immediately 
      ** following this one. In this case, 0 records means 0 records, 
      ** not "read until the end of the file". See also ticket #2565.
      */
      ถ้า( iSize>=(iOff+nSector) ){
        rc = sqlite3OsRead(pReal, zBuf, 28, iOff);
        ถ้า( rc!=SQLITE_OK || 0==decodeJournalHdr(zBuf, 0, 0, 0, 0) ){
          continue;
        }
      }
      nRec = (u32)((iSize-iOff) / (pMain->nPagesize+8));
    }

    /* Read all the records that follow the journal-header just read. */
    สำหรับ(ii=0; rc==SQLITE_OK && ii<nRec && iOff<iSize; ii++){
      u32 pgno;
      rc = sqlite3OsRead(pReal, zBuf, 4, iOff);
      ถ้า( rc==SQLITE_OK ){
        pgno = decodeUint32(zBuf);
        ถ้า( pgno>0 && pgno<=pMain->nPage ){
          ถ้า( 0==sqlite3BitvecTest(pMain->pWritable, pgno) ){
            rc = sqlite3OsRead(pReal, aPage, pMain->nPagesize, iOff+4);
            ถ้า( rc==SQLITE_OK ){
              u32 cksum = genCksum(aPage, pMain->nPagesize);
              assert( cksum==pMain->aCksum[pgno-1] );
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
  ถ้า( rc==SQLITE_IOERR_SHORT_READ ){
    rc = SQLITE_OK;
  }
  คืน rc;
}

/*
** Write data to an jt-file.
*/
static จำนวนเต็ม jtWrite(
  sqlite3_file *pFile, 
  const void *zBuf, 
  จำนวนเต็ม iAmt, 
  sqlite_int64 iOfst
){
  จำนวนเต็ม rc;
  jt_file *p = (jt_file *)pFile;
  ถ้า( p->flags&SQLITE_OPEN_MAIN_JOURNAL ){
    ถ้า( iOfst==0 ){
      jt_file *pMain = locateDatabaseHandle(p->zName, 0);
      assert( pMain );
  
      ถ้า( iAmt==28 ){
        /* Zeroing the first journal-file header. This is the end of a
        ** transaction. */
        closeTransaction(pMain);
      }มิฉะนั้น ถ้า( iAmt!=12 ){
        /* Writing the first journal header to a journal file. This happens
        ** when a transaction is first started.  */
        u8 *z = (u8 *)zBuf;
        pMain->nPage = decodeUint32(&z[16]);
        pMain->nPagesize = decodeUint32(&z[24]);
        ถ้า( SQLITE_OK!=(rc=openTransaction(pMain, p)) ){
          คืน rc;
        }
      }
    }
    ถ้า( p->iMaxOff<(iOfst + iAmt) ){
      p->iMaxOff = iOfst + iAmt;
    }
  }

  ถ้า( p->flags&SQLITE_OPEN_MAIN_DB && p->pWritable ){
    ถ้า( iAmt<(จำนวนเต็ม)p->nPagesize 
     && p->nPagesize%iAmt==0 
     && iOfst>=(PENDING_BYTE+512) 
     && iOfst+iAmt<=PENDING_BYTE+p->nPagesize
    ){
      /* No-op. This special case is hit when the backup code is copying a
      ** to a database with a larger page-size than the source database and
      ** it needs to fill in the non-locking-region part of the original
      ** pending-byte page.
      */
    }มิฉะนั้น{
      u32 pgno = (u32)(iOfst/p->nPagesize + 1);
      assert( (iAmt==1||iAmt==(จำนวนเต็ม)p->nPagesize) &&
              ((iOfst+iAmt)%p->nPagesize)==0 );
      /* The following assert() statements may fail if this layer is used
      ** with a connection in "PRAGMA synchronous=off" mode. If they
      ** fail with sync=normal or sync=full, this may indicate problem.  */
      assert( p->nPage==0 || pgno<=p->nPage || p->nSync>0 );
      assert( pgno>p->nPage || sqlite3BitvecTest(p->pWritable, pgno) );
    }
  }

  rc = sqlite3OsWrite(p->pReal, zBuf, iAmt, iOfst);
  ถ้า( (p->flags&SQLITE_OPEN_MAIN_JOURNAL) && iAmt==12 ){
    jt_file *pMain = locateDatabaseHandle(p->zName, 0);
    จำนวนเต็ม rc2 = readJournalFile(p, pMain);
    ถ้า( rc==SQLITE_OK ) rc = rc2;
  }
  คืน rc;
}

/*
** Truncate an jt-file.
*/
static จำนวนเต็ม jtTruncate(sqlite3_file *pFile, sqlite_int64 size){
  jt_file *p = (jt_file *)pFile;
  ถ้า( p->flags&SQLITE_OPEN_MAIN_JOURNAL && size==0 ){
    /* Truncating a journal file. This is the end of a transaction. */
    jt_file *pMain = locateDatabaseHandle(p->zName, 0);
    closeTransaction(pMain);
  }
  ถ้า( p->flags&SQLITE_OPEN_MAIN_DB && p->pWritable ){
    u32 pgno;
    u32 locking_page = (u32)(PENDING_BYTE/p->nPagesize+1);
    สำหรับ(pgno=(u32)(size/p->nPagesize+1); pgno<=p->nPage; pgno++){
      assert( pgno==locking_page || sqlite3BitvecTest(p->pWritable, pgno) );
    }
  }
  คืน sqlite3OsTruncate(p->pReal, size);
}

/*
** Sync an jt-file.
*/
static จำนวนเต็ม jtSync(sqlite3_file *pFile, จำนวนเต็ม flags){
  jt_file *p = (jt_file *)pFile;

  ถ้า( p->flags&SQLITE_OPEN_MAIN_JOURNAL ){
    จำนวนเต็ม rc;
    jt_file *pMain;                   /* The associated database file */

    /* The journal file is being synced. At this point, we inspect the 
    ** contents of the file up to this point and set each bit in the 
    ** jt_file.pWritable bitvec of the main database file associated with
    ** this journal file.
    */
    pMain = locateDatabaseHandle(p->zName, 0);

    /* Set the bitvec values */
    ถ้า( pMain && pMain->pWritable ){
      pMain->nSync++;
      rc = readJournalFile(p, pMain);
      ถ้า( rc!=SQLITE_OK ){
        คืน rc;
      }
    }
  }

  คืน sqlite3OsSync(p->pReal, flags);
}

/*
** Return the current file-size of an jt-file.
*/
static จำนวนเต็ม jtFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  jt_file *p = (jt_file *)pFile;
  คืน sqlite3OsFileSize(p->pReal, pSize);
}

/*
** Lock an jt-file.
*/
static จำนวนเต็ม jtLock(sqlite3_file *pFile, จำนวนเต็ม eLock){
  จำนวนเต็ม rc;
  jt_file *p = (jt_file *)pFile;
  rc = sqlite3OsLock(p->pReal, eLock);
  ถ้า( rc==SQLITE_OK && eLock>p->eLock ){
    p->eLock = eLock;
  }
  คืน rc;
}

/*
** Unlock an jt-file.
*/
static จำนวนเต็ม jtUnlock(sqlite3_file *pFile, จำนวนเต็ม eLock){
  จำนวนเต็ม rc;
  jt_file *p = (jt_file *)pFile;
  rc = sqlite3OsUnlock(p->pReal, eLock);
  ถ้า( rc==SQLITE_OK && eLock<p->eLock ){
    p->eLock = eLock;
  }
  คืน rc;
}

/*
** Check if another file-handle holds a RESERVED lock on an jt-file.
*/
static จำนวนเต็ม jtCheckReservedLock(sqlite3_file *pFile, จำนวนเต็ม *pResOut){
  jt_file *p = (jt_file *)pFile;
  คืน sqlite3OsCheckReservedLock(p->pReal, pResOut);
}

/*
** File control method. For custom operations on an jt-file.
*/
static จำนวนเต็ม jtFileControl(sqlite3_file *pFile, จำนวนเต็ม op, void *pArg){
  jt_file *p = (jt_file *)pFile;
  คืน p->pReal->pMethods->xFileControl(p->pReal, op, pArg);
}

/*
** Return the sector-size in bytes for an jt-file.
*/
static จำนวนเต็ม jtSectorSize(sqlite3_file *pFile){
  jt_file *p = (jt_file *)pFile;
  คืน sqlite3OsSectorSize(p->pReal);
}

/*
** Return the device characteristic flags supported by an jt-file.
*/
static จำนวนเต็ม jtDeviceCharacteristics(sqlite3_file *pFile){
  jt_file *p = (jt_file *)pFile;
  คืน sqlite3OsDeviceCharacteristics(p->pReal);
}

/*
** Open an jt file handle.
*/
static จำนวนเต็ม jtOpen(
  sqlite3_vfs *pVfs,
  const var_na *zName,
  sqlite3_file *pFile,
  จำนวนเต็ม flags,
  จำนวนเต็ม *pOutFlags
){
  จำนวนเต็ม rc;
  jt_file *p = (jt_file *)pFile;
  pFile->pMethods = 0;
  p->pReal = (sqlite3_file *)&p[1];
  p->pReal->pMethods = 0;
  rc = sqlite3OsOpen(g.pVfs, zName, p->pReal, flags, pOutFlags);
  assert( rc==SQLITE_OK || p->pReal->pMethods==0 );
  ถ้า( rc==SQLITE_OK ){
    pFile->pMethods = &jt_io_methods;
    p->eLock = 0;
    p->zName = zName;
    p->flags = flags;
    p->pNext = 0;
    p->pWritable = 0;
    p->aCksum = 0;
    enterJtMutex();
    ถ้า( zName ){
      p->pNext = g.pList;
      g.pList = p;
    }
    leaveJtMutex();
  }
  คืน rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
static จำนวนเต็ม jtDelete(sqlite3_vfs *pVfs, const var_na *zPath, จำนวนเต็ม dirSync){
  จำนวนเต็ม nPath = (จำนวนเต็ม)strlen(zPath);
  ถ้า( nPath>8 && 0==strcmp("-journal", &zPath[nPath-8]) ){
    /* Deleting a journal file. The end of a transaction. */
    jt_file *pMain = locateDatabaseHandle(zPath, 0);
    ถ้า( pMain ){
      closeTransaction(pMain);
    }
  }

  คืน sqlite3OsDelete(g.pVfs, zPath, dirSync);
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
static จำนวนเต็ม jtAccess(
  sqlite3_vfs *pVfs, 
  const var_na *zPath, 
  จำนวนเต็ม flags, 
  จำนวนเต็ม *pResOut
){
  คืน sqlite3OsAccess(g.pVfs, zPath, flags, pResOut);
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (JT_MAX_PATHNAME+1) bytes.
*/
static จำนวนเต็ม jtFullPathname(
  sqlite3_vfs *pVfs, 
  const var_na *zPath, 
  จำนวนเต็ม nOut, 
  var_na *zOut
){
  คืน sqlite3OsFullPathname(g.pVfs, zPath, nOut, zOut);
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
static void *jtDlOpen(sqlite3_vfs *pVfs, const var_na *zPath){
  คืน g.pVfs->xDlOpen(g.pVfs, zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
static void jtDlError(sqlite3_vfs *pVfs, จำนวนเต็ม nByte, var_na *zErrMsg){
  g.pVfs->xDlError(g.pVfs, nByte, zErrMsg);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
static void (*jtDlSym(sqlite3_vfs *pVfs, void *p, const var_na *zSym))(void){
  คืน g.pVfs->xDlSym(g.pVfs, p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
static void jtDlClose(sqlite3_vfs *pVfs, void *pHandle){
  g.pVfs->xDlClose(g.pVfs, pHandle);
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
static จำนวนเต็ม jtRandomness(sqlite3_vfs *pVfs, จำนวนเต็ม nByte, var_na *zBufOut){
  คืน sqlite3OsRandomness(g.pVfs, nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
static จำนวนเต็ม jtSleep(sqlite3_vfs *pVfs, จำนวนเต็ม nMicro){
  คืน sqlite3OsSleep(g.pVfs, nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
static จำนวนเต็ม jtCurrentTime(sqlite3_vfs *pVfs, dviguu_na *pTimeOut){
  คืน g.pVfs->xCurrentTime(g.pVfs, pTimeOut);
}
/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
static จำนวนเต็ม jtCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *pTimeOut){
  คืน g.pVfs->xCurrentTimeInt64(g.pVfs, pTimeOut);
}

static จำนวนเต็ม jtGetLastError(sqlite3_vfs *pVfs, จำนวนเต็ม n, var_na *z){
  คืน g.pVfs->xGetLastError(g.pVfs, n, z);
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
จำนวนเต็ม jt_register(var_na *zWrap, จำนวนเต็ม isDefault){
  g.pVfs = sqlite3_vfs_find(zWrap);
  ถ้า( g.pVfs==0 ){
    คืน SQLITE_ERROR;
  }
  jt_vfs.szOsFile = parimaa_na(jt_file) + g.pVfs->szOsFile;
  ถ้า( g.pVfs->iVersion==1 ){
    jt_vfs.iVersion = 1;
  }มิฉะนั้น ถ้า( g.pVfs->xCurrentTimeInt64==0 ){
    jt_vfs.xCurrentTimeInt64 = 0;
  }
  sqlite3_vfs_register(&jt_vfs, isDefault);
  คืน SQLITE_OK;
}

/*
** Uninstall the jt VFS, if it is installed.
*/
void jt_unregister(void){
  sqlite3_vfs_unregister(&jt_vfs);
}

#endif
