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
#اگر SQLITE_TEST          /* This file is used for testing only */

#شامل "sqlite3.h"
#شامل "sqliteInt.h"

/*
** Maximum pathname length supported by the jt backend.
*/
#تعریف_کرو JT_MAX_PATHNAME 512

/*
** Name used to identify this VFS.
*/
#تعریف_کرو JT_VFS_NAME "jt"

قسمتعریف ساخت jt_file jt_file;
ساخت jt_file {
  sqlite3_file بنیاد;
  ثابت حرف *zName;       /* Name of open file */
  صحیح flags;               /* Flags the file was opened with */

  /* The following are only used by database file file handles */
  صحیح e_تالا;               /* Current lock held on the file */
  u32 n_صفحہ;               /* Size of file in pages when transaction started */
  u32 nPagesize;           /* Page size when transaction started */
  Bitvec *pWritable;       /* Bitvec of pages that may be written to the file */
  u32 *aCksum;             /* Checksum for first nPage pages */
  صحیح nSync;               /* Number of times journal file has been synced */

  /* Only used by journal file-handles */
  sqlite3_int64 iMaxOff;   /* Maximum offset written to this transaction */

  jt_file *p_اگلا;          /* All files are stored in a linked list */
  sqlite3_file *pReal;     /* The file handle for the underlying vfs */
};

/*
** Method declarations for jt_file.
*/
ساکن صحیح jtClose(sqlite3_file*);
ساکن صحیح jtRead(sqlite3_file*, خالی*, صحیح iAmt, sqlite3_int64 iOfst);
ساکن صحیح jtWrite(sqlite3_file*,ثابت خالی*,صحیح iAmt, sqlite3_int64 iOfst);
ساکن صحیح jtTruncate(sqlite3_file*, sqlite3_int64 سائز);
ساکن صحیح jtSync(sqlite3_file*, صحیح flags);
ساکن صحیح jt_File_سائز(sqlite3_file*, sqlite3_int64 *p_سائز);
ساکن صحیح jt_تالا(sqlite3_file*, صحیح);
ساکن صحیح jtUnlock(sqlite3_file*, صحیح);
ساکن صحیح jt_Check_Reserved_تالا(sqlite3_file*, صحیح *);
ساکن صحیح jtFileControl(sqlite3_file*, صحیح op, خالی *pArg);
ساکن صحیح jt_Sector_سائز(sqlite3_file*);
ساکن صحیح jtDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for jt_vfs.
*/
ساکن صحیح jtOpen(sqlite3_vfs*, ثابت حرف *, sqlite3_file*, صحیح , صحیح *);
ساکن صحیح jtDelete(sqlite3_vfs*, ثابت حرف *zName, صحیح syncDir);
ساکن صحیح jtAccess(sqlite3_vfs*, ثابت حرف *zName, صحیح flags, صحیح *);
ساکن صحیح jt_بھرا_Pathname(sqlite3_vfs*, ثابت حرف *zName, صحیح, حرف *z_برآمد);
ساکن خالی *jtDlOpen(sqlite3_vfs*, ثابت حرف *zFilename);
ساکن خالی jtDlError(sqlite3_vfs*, صحیح nByte, حرف *z_Err_پیغام);
ساکن خالی (*jtDlSym(sqlite3_vfs*,خالی*, ثابت حرف *z_علامت))(خالی);
ساکن خالی jtDlClose(sqlite3_vfs*, خالی*);
ساکن صحیح jtRandomness(sqlite3_vfs*, صحیح nByte, حرف *z_برآمد);
ساکن صحیح jtSleep(sqlite3_vfs*, صحیح microseconds);
ساکن صحیح jt_Current_وقت(sqlite3_vfs*, دوہرا*);
ساکن صحیح jt_Current_وقت_Int64(sqlite3_vfs*, sqlite3_int64*);
ساکن صحیح jt_Get_آخری_Error(sqlite3_vfs*, صحیح, حرف*);

ساکن sqlite3_vfs jt_vfs = {
  2,                             /* iVersion */
  حجم(jt_file),               /* szOsFile */
  JT_MAX_PATHNAME,               /* mxPathname */
  0,                             /* pNext */
  JT_VFS_NAME,                   /* zName */
  0,                             /* pAppData */
  jtOpen,                        /* xOpen */
  jtDelete,                      /* xDelete */
  jtAccess,                      /* xAccess */
  jt_بھرا_Pathname,                /* xFullPathname */
  jtDlOpen,                      /* xDlOpen */
  jtDlError,                     /* xDlError */
  jtDlSym,                       /* xDlSym */
  jtDlClose,                     /* xDlClose */
  jtRandomness,                  /* xRandomness */
  jtSleep,                       /* xSleep */
  jt_Current_وقت,                 /* xCurrentTime */
  jt_Get_آخری_Error,                /* xGetLastError */
  jt_Current_وقت_Int64             /* xCurrentTimeInt64 */
};

ساکن sqlite3_io_methods jt_io_methods = {
  1,                             /* iVersion */
  jtClose,                       /* xClose */
  jtRead,                        /* xRead */
  jtWrite,                       /* xWrite */
  jtTruncate,                    /* xTruncate */
  jtSync,                        /* xSync */
  jt_File_سائز,                    /* xFileSize */
  jt_تالا,                        /* xLock */
  jtUnlock,                      /* xUnlock */
  jt_Check_Reserved_تالا,           /* xCheckReservedLock */
  jtFileControl,                 /* xFileControl */
  jt_Sector_سائز,                  /* xSectorSize */
  jtDeviceCharacteristics        /* xDeviceCharacteristics */
};

ساخت JtGlobal {
  sqlite3_vfs *pVfs;             /* Parent VFS */
  jt_file *pList;                /* List of all open files */
};
ساکن ساخت JtGlobal g = {0, 0};

/*
** Functions to obtain and relinquish a mutex to protect g.pList. The
** STATIC_PRNG mutex is reused, purely for the sake of convenience.
*/
ساکن خالی enterJtMutex(خالی){
  sqlite3_mutex_enter(sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_PRNG));
}
ساکن خالی leaveJtMutex(خالی){
  sqlite3_mutex_leave(sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_PRNG));
}

خارجی صحیح sqlite3_io_error_pending;
خارجی صحیح sqlite3_io_error_کامیابی;
ساکن خالی stop_ioerr_simulation(صحیح *piSave, صحیح *piSave2){
  *piSave = sqlite3_io_error_pending;
  *piSave2 = sqlite3_io_error_کامیابی;
  sqlite3_io_error_pending = -1;
  sqlite3_io_error_کامیابی = 0;
}
ساکن خالی آغاز_ioerr_simulation(صحیح iSave, صحیح iSave2){
  sqlite3_io_error_pending = iSave;
  sqlite3_io_error_کامیابی = iSave2;
}

/*
** The jt_file pointed to by the argument may or may not be a file-handle
** open on a main database file. If it is, and a transaction is currently
** opened on the file, then discard all transaction related data.
*/
ساکن خالی closeTransaction(jt_file *p){
  sqlite3BitvecDestroy(p->pWritable);
  sqlite3_free(p->aCksum);
  p->pWritable = 0;
  p->aCksum = 0;
  p->nSync = 0;
}

/*
** Close an jt-file.
*/
ساکن صحیح jtClose(sqlite3_file *pFile){
  jt_file **pp;
  jt_file *p = (jt_file *)pFile;

  closeTransaction(p);
  enterJtMutex();
  اگر( p->zName ){
    برائے(pp=&g.pList; *pp!=p; pp=&(*pp)->p_اگلا);
    *pp = p->p_اگلا;
  }
  leaveJtMutex();
  sqlite3OsClose(p->pReal);
  واپس SQLITE_OK;
}

/*
** Read data from an jt-file.
*/
ساکن صحیح jtRead(
  sqlite3_file *pFile, 
  خالی *zBuf, 
  صحیح iAmt, 
  sqlite_int64 iOfst
){
  jt_file *p = (jt_file *)pFile;
  واپس sqlite3OsRead(p->pReal, zBuf, iAmt, iOfst);
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
ساکن jt_file *locateDatabaseHandle(ثابت حرف *zJournal, صحیح no_تالا){
  jt_file *pMain = 0;
  enterJtMutex();
  برائے(pMain=g.pList; pMain; pMain=pMain->p_اگلا){
    صحیح nName = (صحیح)(لڑی_لمبائی(zJournal) - لڑی_لمبائی("-journal"));
    اگر( (pMain->flags&SQLITE_OPEN_MAIN_DB)
     && ((صحیح)لڑی_لمبائی(pMain->zName)==nName)
     && 0==memcmp(pMain->zName, zJournal, nName)
     && ((pMain->e_تالا>=SQLITE_تالا_RESERVED) || no_تالا)
    ){
      توڑو;
    }
  }
  leaveJtMutex();
  واپس pMain;
}

/*
** Parameter z points to a buffer of 4 bytes in size containing a 
** unsigned 32-bit integer stored in big-endian format. Decode the 
** integer and return its value.
*/
ساکن u32 decodeUint32(ثابت غیرعلامتی حرف *z){
  واپس (z[0]<<24) + (z[1]<<16) + (z[2]<<8) + z[3];
}

/*
** Calculate a checksum from the buffer of length n bytes pointed to
** by parameter z.
*/
ساکن u32 genCksum(ثابت غیرعلامتی حرف *z, صحیح n){
  صحیح i;
  u32 cksum = 0;
  برائے(i=0; i<n; i++){
    cksum = cksum + z[i] + (cksum<<3);
  }
  واپس cksum;
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
ساکن صحیح decodeJournalHdr(
  ثابت غیرعلامتی حرف *zBuf,         /* Input: 28 byte journal header */
  u32 *pnRec,                        /* Out: Number of journalled records */
  u32 *pn_صفحہ,                       /* Out: Original database page count */
  u32 *pnSector,                     /* Out: Sector size in bytes */
  u32 *pnPagesize                    /* Out: Page size in bytes */
){
  غیرعلامتی حرف aMagic[] = { 0xd9, 0xd5, 0x05, 0xf9, 0x20, 0xa1, 0x63, 0xd7 };
  اگر( memcmp(aMagic, zBuf, 8) ) واپس SQLITE_ERROR;
  اگر( pnRec ) *pnRec = decodeUint32(&zBuf[8]);
  اگر( pn_صفحہ ) *pn_صفحہ = decodeUint32(&zBuf[16]);
  اگر( pnSector ) *pnSector = decodeUint32(&zBuf[20]);
  اگر( pnPagesize ) *pnPagesize = decodeUint32(&zBuf[24]);
  واپس SQLITE_OK;
}

/*
** This function is called when a new transaction is opened, just after
** the first journal-header is written to the journal file.
*/
ساکن صحیح openTransaction(jt_file *pMain, jt_file *pJournal){
  غیرعلامتی حرف *a_ڈیٹا;
  sqlite3_file *p = pMain->pReal;
  صحیح rc = SQLITE_OK;

  closeTransaction(pMain);
  a_ڈیٹا = sqlite3_malloc(pMain->nPagesize);
  pMain->pWritable = sqlite3BitvecCreate(pMain->n_صفحہ);
  pMain->aCksum = sqlite3_malloc(حجم(u32) * (pMain->n_صفحہ + 1));
  pJournal->iMaxOff = 0;

  اگر( !pMain->pWritable || !pMain->aCksum || !a_ڈیٹا ){
    rc = SQLITE_IOERR_NOMEM;
  }ورنہ اگر( pMain->n_صفحہ>0 ){
    u32 iTrunk;
    صحیح iSave;
    صحیح iSave2;

    stop_ioerr_simulation(&iSave, &iSave2);

    /* Read the database free-list. Add the page-number for each free-list
    ** leaf to the jt_file.pWritable bitvec.
    */
    rc = sqlite3OsRead(p, a_ڈیٹا, pMain->nPagesize, 0);
    اگر( rc==SQLITE_OK ){
      u32 nDbsize = decodeUint32(&a_ڈیٹا[28]);
      اگر( nDbsize>0 && memcmp(&a_ڈیٹا[24], &a_ڈیٹا[92], 4)==0 ){
        u32 iPg;
        برائے(iPg=nDbsize+1; iPg<=pMain->n_صفحہ; iPg++){
          sqlite3_Bitvec_سیٹ(pMain->pWritable, iPg);
        }
      }
    }
    iTrunk = decodeUint32(&a_ڈیٹا[32]);
    جب( rc==SQLITE_OK && iTrunk>0 ){
      u32 n_برگ;
      u32 i_برگ;
      sqlite3_int64 iOff = (i64)(iTrunk-1)*pMain->nPagesize;
      rc = sqlite3OsRead(p, a_ڈیٹا, pMain->nPagesize, iOff);
      n_برگ = decodeUint32(&a_ڈیٹا[4]);
      برائے(i_برگ=0; rc==SQLITE_OK && i_برگ<n_برگ; i_برگ++){
        u32 pgno = decodeUint32(&a_ڈیٹا[8+4*i_برگ]);
        sqlite3_Bitvec_سیٹ(pMain->pWritable, pgno);
      }
      iTrunk = decodeUint32(a_ڈیٹا);
    }

    /* Calculate and store a checksum for each page in the database file. */
    اگر( rc==SQLITE_OK ){
      صحیح ii;
      برائے(ii=0; rc==SQLITE_OK && ii<(صحیح)pMain->n_صفحہ; ii++){
        i64 iOff = (i64)(pMain->nPagesize) * (i64)ii;
        اگر( iOff==PENDING_BYTE ) جاری;
        rc = sqlite3OsRead(pMain->pReal, a_ڈیٹا, pMain->nPagesize, iOff);
        pMain->aCksum[ii] = genCksum(a_ڈیٹا, pMain->nPagesize);
        اگر( ii+1==(صحیح)pMain->n_صفحہ && rc==SQLITE_IOERR_SHORT_READ ){
          rc = SQLITE_OK;
        }
      }
    }

    آغاز_ioerr_simulation(iSave, iSave2);
  }

  sqlite3_free(a_ڈیٹا);
  واپس rc;
}

/*
** The first argument to this function is a handle open on a journal file.
** This function reads the journal file and adds the page number for each
** page in the journal to the Bitvec object passed as the second argument.
*/
ساکن صحیح readJournalFile(jt_file *p, jt_file *pMain){
  صحیح rc = SQLITE_OK;
  غیرعلامتی حرف zBuf[28];
  sqlite3_file *pReal = p->pReal;
  sqlite3_int64 iOff = 0;
  sqlite3_int64 i_سائز = p->iMaxOff;
  غیرعلامتی حرف *a_صفحہ;
  صحیح iSave;
  صحیح iSave2;

  a_صفحہ = sqlite3_malloc(pMain->nPagesize);
  اگر( !a_صفحہ ){
    واپس SQLITE_IOERR_NOMEM;
  }

  stop_ioerr_simulation(&iSave, &iSave2);

  جب( rc==SQLITE_OK && iOff<i_سائز ){
    u32 nRec, n_صفحہ, nSector, nPagesize;
    u32 ii;

    /* Read and decode the next journal-header from the journal file. */
    rc = sqlite3OsRead(pReal, zBuf, 28, iOff);
    اگر( rc!=SQLITE_OK 
     || decodeJournalHdr(zBuf, &nRec, &n_صفحہ, &nSector, &nPagesize) 
    ){
      جاؤ اختتام_rjf;
    }
    iOff += nSector;

    اگر( nRec==0 ){
      /* A trick. There might be another journal-header immediately 
      ** following this one. In this case, 0 records means 0 records, 
      ** not "read until the end of the file". See also ticket #2565.
      */
      اگر( i_سائز>=(iOff+nSector) ){
        rc = sqlite3OsRead(pReal, zBuf, 28, iOff);
        اگر( rc!=SQLITE_OK || 0==decodeJournalHdr(zBuf, 0, 0, 0, 0) ){
          جاری;
        }
      }
      nRec = (u32)((i_سائز-iOff) / (pMain->nPagesize+8));
    }

    /* Read all the records that follow the journal-header just read. */
    برائے(ii=0; rc==SQLITE_OK && ii<nRec && iOff<i_سائز; ii++){
      u32 pgno;
      rc = sqlite3OsRead(pReal, zBuf, 4, iOff);
      اگر( rc==SQLITE_OK ){
        pgno = decodeUint32(zBuf);
        اگر( pgno>0 && pgno<=pMain->n_صفحہ ){
          اگر( 0==sqlite3BitvecTest(pMain->pWritable, pgno) ){
            rc = sqlite3OsRead(pReal, a_صفحہ, pMain->nPagesize, iOff+4);
            اگر( rc==SQLITE_OK ){
              u32 cksum = genCksum(a_صفحہ, pMain->nPagesize);
              أكد( cksum==pMain->aCksum[pgno-1] );
            }
          }
          sqlite3_Bitvec_سیٹ(pMain->pWritable, pgno);
        }
        iOff += (8 + pMain->nPagesize);
      }
    }

    iOff = ((iOff + (nSector-1)) / nSector) * nSector;
  }

اختتام_rjf:
  آغاز_ioerr_simulation(iSave, iSave2);
  sqlite3_free(a_صفحہ);
  اگر( rc==SQLITE_IOERR_SHORT_READ ){
    rc = SQLITE_OK;
  }
  واپس rc;
}

/*
** Write data to an jt-file.
*/
ساکن صحیح jtWrite(
  sqlite3_file *pFile, 
  ثابت خالی *zBuf, 
  صحیح iAmt, 
  sqlite_int64 iOfst
){
  صحیح rc;
  jt_file *p = (jt_file *)pFile;
  اگر( p->flags&SQLITE_OPEN_MAIN_JOURNAL ){
    اگر( iOfst==0 ){
      jt_file *pMain = locateDatabaseHandle(p->zName, 0);
      أكد( pMain );
  
      اگر( iAmt==28 ){
        /* Zeroing the first journal-file header. This is the end of a
        ** transaction. */
        closeTransaction(pMain);
      }ورنہ اگر( iAmt!=12 ){
        /* Writing the first journal header to a journal file. This happens
        ** when a transaction is first started.  */
        u8 *z = (u8 *)zBuf;
        pMain->n_صفحہ = decodeUint32(&z[16]);
        pMain->nPagesize = decodeUint32(&z[24]);
        اگر( SQLITE_OK!=(rc=openTransaction(pMain, p)) ){
          واپس rc;
        }
      }
    }
    اگر( p->iMaxOff<(iOfst + iAmt) ){
      p->iMaxOff = iOfst + iAmt;
    }
  }

  اگر( p->flags&SQLITE_OPEN_MAIN_DB && p->pWritable ){
    اگر( iAmt<(صحیح)p->nPagesize 
     && p->nPagesize%iAmt==0 
     && iOfst>=(PENDING_BYTE+512) 
     && iOfst+iAmt<=PENDING_BYTE+p->nPagesize
    ){
      /* No-op. This special case is hit when the backup code is copying a
      ** to a database with a larger page-size than the source database and
      ** it needs to fill in the non-locking-region part of the original
      ** pending-byte page.
      */
    }ورنہ{
      u32 pgno = (u32)(iOfst/p->nPagesize + 1);
      أكد( (iAmt==1||iAmt==(صحیح)p->nPagesize) &&
              ((iOfst+iAmt)%p->nPagesize)==0 );
      /* The following assert() statements may fail if this layer is used
      ** with a connection in "PRAGMA synchronous=off" mode. If they
      ** fail with sync=normal or sync=full, this may indicate problem.  */
      أكد( p->n_صفحہ==0 || pgno<=p->n_صفحہ || p->nSync>0 );
      أكد( pgno>p->n_صفحہ || sqlite3BitvecTest(p->pWritable, pgno) );
    }
  }

  rc = sqlite3OsWrite(p->pReal, zBuf, iAmt, iOfst);
  اگر( (p->flags&SQLITE_OPEN_MAIN_JOURNAL) && iAmt==12 ){
    jt_file *pMain = locateDatabaseHandle(p->zName, 0);
    صحیح rc2 = readJournalFile(p, pMain);
    اگر( rc==SQLITE_OK ) rc = rc2;
  }
  واپس rc;
}

/*
** Truncate an jt-file.
*/
ساکن صحیح jtTruncate(sqlite3_file *pFile, sqlite_int64 سائز){
  jt_file *p = (jt_file *)pFile;
  اگر( p->flags&SQLITE_OPEN_MAIN_JOURNAL && سائز==0 ){
    /* Truncating a journal file. This is the end of a transaction. */
    jt_file *pMain = locateDatabaseHandle(p->zName, 0);
    closeTransaction(pMain);
  }
  اگر( p->flags&SQLITE_OPEN_MAIN_DB && p->pWritable ){
    u32 pgno;
    u32 locking_صفحہ = (u32)(PENDING_BYTE/p->nPagesize+1);
    برائے(pgno=(u32)(سائز/p->nPagesize+1); pgno<=p->n_صفحہ; pgno++){
      أكد( pgno==locking_صفحہ || sqlite3BitvecTest(p->pWritable, pgno) );
    }
  }
  واپس sqlite3OsTruncate(p->pReal, سائز);
}

/*
** Sync an jt-file.
*/
ساکن صحیح jtSync(sqlite3_file *pFile, صحیح flags){
  jt_file *p = (jt_file *)pFile;

  اگر( p->flags&SQLITE_OPEN_MAIN_JOURNAL ){
    صحیح rc;
    jt_file *pMain;                   /* The associated database file */

    /* The journal file is being synced. At this point, we inspect the 
    ** contents of the file up to this point and set each bit in the 
    ** jt_file.pWritable bitvec of the main database file associated with
    ** this journal file.
    */
    pMain = locateDatabaseHandle(p->zName, 0);

    /* Set the bitvec values */
    اگر( pMain && pMain->pWritable ){
      pMain->nSync++;
      rc = readJournalFile(p, pMain);
      اگر( rc!=SQLITE_OK ){
        واپس rc;
      }
    }
  }

  واپس sqlite3OsSync(p->pReal, flags);
}

/*
** Return the current file-size of an jt-file.
*/
ساکن صحیح jt_File_سائز(sqlite3_file *pFile, sqlite_int64 *p_سائز){
  jt_file *p = (jt_file *)pFile;
  واپس sqlite3_Os_File_سائز(p->pReal, p_سائز);
}

/*
** Lock an jt-file.
*/
ساکن صحیح jt_تالا(sqlite3_file *pFile, صحیح e_تالا){
  صحیح rc;
  jt_file *p = (jt_file *)pFile;
  rc = sqlite3_Os_تالا(p->pReal, e_تالا);
  اگر( rc==SQLITE_OK && e_تالا>p->e_تالا ){
    p->e_تالا = e_تالا;
  }
  واپس rc;
}

/*
** Unlock an jt-file.
*/
ساکن صحیح jtUnlock(sqlite3_file *pFile, صحیح e_تالا){
  صحیح rc;
  jt_file *p = (jt_file *)pFile;
  rc = sqlite3OsUnlock(p->pReal, e_تالا);
  اگر( rc==SQLITE_OK && e_تالا<p->e_تالا ){
    p->e_تالا = e_تالا;
  }
  واپس rc;
}

/*
** Check if another file-handle holds a RESERVED lock on an jt-file.
*/
ساکن صحیح jt_Check_Reserved_تالا(sqlite3_file *pFile, صحیح *p_Res_برآمد){
  jt_file *p = (jt_file *)pFile;
  واپس sqlite3_Os_Check_Reserved_تالا(p->pReal, p_Res_برآمد);
}

/*
** File control method. For custom operations on an jt-file.
*/
ساکن صحیح jtFileControl(sqlite3_file *pFile, صحیح op, خالی *pArg){
  jt_file *p = (jt_file *)pFile;
  واپس p->pReal->pMethods->xFileControl(p->pReal, op, pArg);
}

/*
** Return the sector-size in bytes for an jt-file.
*/
ساکن صحیح jt_Sector_سائز(sqlite3_file *pFile){
  jt_file *p = (jt_file *)pFile;
  واپس sqlite3_Os_Sector_سائز(p->pReal);
}

/*
** Return the device characteristic flags supported by an jt-file.
*/
ساکن صحیح jtDeviceCharacteristics(sqlite3_file *pFile){
  jt_file *p = (jt_file *)pFile;
  واپس sqlite3OsDeviceCharacteristics(p->pReal);
}

/*
** Open an jt file handle.
*/
ساکن صحیح jtOpen(
  sqlite3_vfs *pVfs,
  ثابت حرف *zName,
  sqlite3_file *pFile,
  صحیح flags,
  صحیح *p_برآمد_Flags
){
  صحیح rc;
  jt_file *p = (jt_file *)pFile;
  pFile->pMethods = 0;
  p->pReal = (sqlite3_file *)&p[1];
  p->pReal->pMethods = 0;
  rc = sqlite3OsOpen(g.pVfs, zName, p->pReal, flags, p_برآمد_Flags);
  أكد( rc==SQLITE_OK || p->pReal->pMethods==0 );
  اگر( rc==SQLITE_OK ){
    pFile->pMethods = &jt_io_methods;
    p->e_تالا = 0;
    p->zName = zName;
    p->flags = flags;
    p->p_اگلا = 0;
    p->pWritable = 0;
    p->aCksum = 0;
    enterJtMutex();
    اگر( zName ){
      p->p_اگلا = g.pList;
      g.pList = p;
    }
    leaveJtMutex();
  }
  واپس rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
ساکن صحیح jtDelete(sqlite3_vfs *pVfs, ثابت حرف *z_راستہ, صحیح dirSync){
  صحیح n_راستہ = (صحیح)لڑی_لمبائی(z_راستہ);
  اگر( n_راستہ>8 && 0==strcmp("-journal", &z_راستہ[n_راستہ-8]) ){
    /* Deleting a journal file. The end of a transaction. */
    jt_file *pMain = locateDatabaseHandle(z_راستہ, 0);
    اگر( pMain ){
      closeTransaction(pMain);
    }
  }

  واپس sqlite3OsDelete(g.pVfs, z_راستہ, dirSync);
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
ساکن صحیح jtAccess(
  sqlite3_vfs *pVfs, 
  ثابت حرف *z_راستہ, 
  صحیح flags, 
  صحیح *p_Res_برآمد
){
  واپس sqlite3OsAccess(g.pVfs, z_راستہ, flags, p_Res_برآمد);
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (JT_MAX_PATHNAME+1) bytes.
*/
ساکن صحیح jt_بھرا_Pathname(
  sqlite3_vfs *pVfs, 
  ثابت حرف *z_راستہ, 
  صحیح n_برآمد, 
  حرف *z_برآمد
){
  واپس sqlite3_Os_بھرا_Pathname(g.pVfs, z_راستہ, n_برآمد, z_برآمد);
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
ساکن خالی *jtDlOpen(sqlite3_vfs *pVfs, ثابت حرف *z_راستہ){
  واپس g.pVfs->xDlOpen(g.pVfs, z_راستہ);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
ساکن خالی jtDlError(sqlite3_vfs *pVfs, صحیح nByte, حرف *z_Err_پیغام){
  g.pVfs->xDlError(g.pVfs, nByte, z_Err_پیغام);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
ساکن خالی (*jtDlSym(sqlite3_vfs *pVfs, خالی *p, ثابت حرف *zSym))(خالی){
  واپس g.pVfs->xDlSym(g.pVfs, p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
ساکن خالی jtDlClose(sqlite3_vfs *pVfs, خالی *pHandle){
  g.pVfs->xDlClose(g.pVfs, pHandle);
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
ساکن صحیح jtRandomness(sqlite3_vfs *pVfs, صحیح nByte, حرف *z_Buf_برآمد){
  واپس sqlite3OsRandomness(g.pVfs, nByte, z_Buf_برآمد);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
ساکن صحیح jtSleep(sqlite3_vfs *pVfs, صحیح nMicro){
  واپس sqlite3OsSleep(g.pVfs, nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
ساکن صحیح jt_Current_وقت(sqlite3_vfs *pVfs, دوہرا *p_وقت_برآمد){
  واپس g.pVfs->x_Current_وقت(g.pVfs, p_وقت_برآمد);
}
/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
ساکن صحیح jt_Current_وقت_Int64(sqlite3_vfs *pVfs, sqlite3_int64 *p_وقت_برآمد){
  واپس g.pVfs->x_Current_وقت_Int64(g.pVfs, p_وقت_برآمد);
}

ساکن صحیح jt_Get_آخری_Error(sqlite3_vfs *pVfs, صحیح n, حرف *z){
  واپس g.pVfs->x_Get_آخری_Error(g.pVfs, n, z);
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
صحیح jt_رجسٹر(حرف *zWrap, صحیح isDefault){
  g.pVfs = sqlite3_vfs_ڈھونڈو(zWrap);
  اگر( g.pVfs==0 ){
    واپس SQLITE_ERROR;
  }
  jt_vfs.szOsFile = حجم(jt_file) + g.pVfs->szOsFile;
  اگر( g.pVfs->i_نسخہ==1 ){
    jt_vfs.i_نسخہ = 1;
  }ورنہ اگر( g.pVfs->x_Current_وقت_Int64==0 ){
    jt_vfs.x_Current_وقت_Int64 = 0;
  }
  sqlite3_vfs_رجسٹر(&jt_vfs, isDefault);
  واپس SQLITE_OK;
}

/*
** Uninstall the jt VFS, if it is installed.
*/
خالی jt_unregister(خالی){
  sqlite3_vfs_unregister(&jt_vfs);
}

#اگر_ختم
