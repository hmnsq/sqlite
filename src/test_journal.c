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
#ਜੇ SQLITE_TEST          /* This file is used for testing only */

#ਅਨ੍ਤਰ੍ਭਾਵ "sqlite3.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"

/*
** Maximum pathname length supported by the jt backend.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JT_MAX_PATHNAME 512

/*
** Name used to identify this VFS.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JT_VFS_NAME "jt"

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ jt_file jt_file;
ਬਣਤਰ jt_file {
  sqlite3_file ਅਧਾਰ;
  ਅਚਲ ਅੱਖਰ *zName;       /* Name of open file */
  ਪੂਰਨ flags;               /* Flags the file was opened with */

  /* The following are only used by database file file handles */
  ਪੂਰਨ e_ਤਾਲਾ;               /* Current lock held on the file */
  u32 n_ਪੰਨਾ;               /* Size of file in pages when transaction started */
  u32 nPagesize;           /* Page size when transaction started */
  Bitvec *pWritable;       /* Bitvec of pages that may be written to the file */
  u32 *aCksum;             /* Checksum for first nPage pages */
  ਪੂਰਨ nSync;               /* Number of times journal file has been synced */

  /* Only used by journal file-handles */
  sqlite3_int64 iMaxOff;   /* Maximum offset written to this transaction */

  jt_file *p_ਅਗਲਾ;          /* All files are stored in a linked list */
  sqlite3_file *pReal;     /* The file handle for the underlying vfs */
};

/*
** Method declarations for jt_file.
*/
ਸਥਿਰ ਪੂਰਨ jtClose(sqlite3_file*);
ਸਥਿਰ ਪੂਰਨ jtRead(sqlite3_file*, ਖਾਲੀ*, ਪੂਰਨ iAmt, sqlite3_int64 iOfst);
ਸਥਿਰ ਪੂਰਨ jtWrite(sqlite3_file*,ਅਚਲ ਖਾਲੀ*,ਪੂਰਨ iAmt, sqlite3_int64 iOfst);
ਸਥਿਰ ਪੂਰਨ jtTruncate(sqlite3_file*, sqlite3_int64 ਮਾਪ);
ਸਥਿਰ ਪੂਰਨ jtSync(sqlite3_file*, ਪੂਰਨ flags);
ਸਥਿਰ ਪੂਰਨ jt_File_ਮਾਪ(sqlite3_file*, sqlite3_int64 *p_ਮਾਪ);
ਸਥਿਰ ਪੂਰਨ jt_ਤਾਲਾ(sqlite3_file*, ਪੂਰਨ);
ਸਥਿਰ ਪੂਰਨ jtUnlock(sqlite3_file*, ਪੂਰਨ);
ਸਥਿਰ ਪੂਰਨ jt_Check_Reserved_ਤਾਲਾ(sqlite3_file*, ਪੂਰਨ *);
ਸਥਿਰ ਪੂਰਨ jtFileControl(sqlite3_file*, ਪੂਰਨ op, ਖਾਲੀ *pArg);
ਸਥਿਰ ਪੂਰਨ jt_Sector_ਮਾਪ(sqlite3_file*);
ਸਥਿਰ ਪੂਰਨ jtDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for jt_vfs.
*/
ਸਥਿਰ ਪੂਰਨ jtOpen(sqlite3_vfs*, ਅਚਲ ਅੱਖਰ *, sqlite3_file*, ਪੂਰਨ , ਪੂਰਨ *);
ਸਥਿਰ ਪੂਰਨ jtDelete(sqlite3_vfs*, ਅਚਲ ਅੱਖਰ *zName, ਪੂਰਨ syncDir);
ਸਥਿਰ ਪੂਰਨ jtAccess(sqlite3_vfs*, ਅਚਲ ਅੱਖਰ *zName, ਪੂਰਨ flags, ਪੂਰਨ *);
ਸਥਿਰ ਪੂਰਨ jt_ਭਰਿਆ_Pathname(sqlite3_vfs*, ਅਚਲ ਅੱਖਰ *zName, ਪੂਰਨ, ਅੱਖਰ *z_ਬਾਹਰਲਾ);
ਸਥਿਰ ਖਾਲੀ *jtDlOpen(sqlite3_vfs*, ਅਚਲ ਅੱਖਰ *zFilename);
ਸਥਿਰ ਖਾਲੀ jtDlError(sqlite3_vfs*, ਪੂਰਨ nByte, ਅੱਖਰ *z_Err_ਸੁਨੇਹਾ);
ਸਥਿਰ ਖਾਲੀ (*jtDlSym(sqlite3_vfs*,ਖਾਲੀ*, ਅਚਲ ਅੱਖਰ *z_ਚਿੰਨ੍ਹ))(ਖਾਲੀ);
ਸਥਿਰ ਖਾਲੀ jtDlClose(sqlite3_vfs*, ਖਾਲੀ*);
ਸਥਿਰ ਪੂਰਨ jtRandomness(sqlite3_vfs*, ਪੂਰਨ nByte, ਅੱਖਰ *z_ਬਾਹਰਲਾ);
ਸਥਿਰ ਪੂਰਨ jtSleep(sqlite3_vfs*, ਪੂਰਨ microseconds);
ਸਥਿਰ ਪੂਰਨ jt_Current_ਸਮਾਂ(sqlite3_vfs*, ਦੁੱਗਣਾ*);
ਸਥਿਰ ਪੂਰਨ jt_Current_ਸਮਾਂ_Int64(sqlite3_vfs*, sqlite3_int64*);
ਸਥਿਰ ਪੂਰਨ jt_Get_ਆਖਰੀ_Error(sqlite3_vfs*, ਪੂਰਨ, ਅੱਖਰ*);

ਸਥਿਰ sqlite3_vfs jt_vfs = {
  2,                             /* iVersion */
  ਆਕਾਰ(jt_file),               /* szOsFile */
  JT_MAX_PATHNAME,               /* mxPathname */
  0,                             /* pNext */
  JT_VFS_NAME,                   /* zName */
  0,                             /* pAppData */
  jtOpen,                        /* xOpen */
  jtDelete,                      /* xDelete */
  jtAccess,                      /* xAccess */
  jt_ਭਰਿਆ_Pathname,                /* xFullPathname */
  jtDlOpen,                      /* xDlOpen */
  jtDlError,                     /* xDlError */
  jtDlSym,                       /* xDlSym */
  jtDlClose,                     /* xDlClose */
  jtRandomness,                  /* xRandomness */
  jtSleep,                       /* xSleep */
  jt_Current_ਸਮਾਂ,                 /* xCurrentTime */
  jt_Get_ਆਖਰੀ_Error,                /* xGetLastError */
  jt_Current_ਸਮਾਂ_Int64             /* xCurrentTimeInt64 */
};

ਸਥਿਰ sqlite3_io_methods jt_io_methods = {
  1,                             /* iVersion */
  jtClose,                       /* xClose */
  jtRead,                        /* xRead */
  jtWrite,                       /* xWrite */
  jtTruncate,                    /* xTruncate */
  jtSync,                        /* xSync */
  jt_File_ਮਾਪ,                    /* xFileSize */
  jt_ਤਾਲਾ,                        /* xLock */
  jtUnlock,                      /* xUnlock */
  jt_Check_Reserved_ਤਾਲਾ,           /* xCheckReservedLock */
  jtFileControl,                 /* xFileControl */
  jt_Sector_ਮਾਪ,                  /* xSectorSize */
  jtDeviceCharacteristics        /* xDeviceCharacteristics */
};

ਬਣਤਰ JtGlobal {
  sqlite3_vfs *pVfs;             /* Parent VFS */
  jt_file *pList;                /* List of all open files */
};
ਸਥਿਰ ਬਣਤਰ JtGlobal g = {0, 0};

/*
** Functions to obtain and relinquish a mutex to protect g.pList. The
** STATIC_PRNG mutex is reused, purely for the sake of convenience.
*/
ਸਥਿਰ ਖਾਲੀ enterJtMutex(ਖਾਲੀ){
  sqlite3_mutex_enter(sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_PRNG));
}
ਸਥਿਰ ਖਾਲੀ leaveJtMutex(ਖਾਲੀ){
  sqlite3_mutex_leave(sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_PRNG));
}

ਬਾਹਰੀ ਪੂਰਨ sqlite3_io_error_pending;
ਬਾਹਰੀ ਪੂਰਨ sqlite3_io_error_ਪ੍ਰਾਪਤੀ;
ਸਥਿਰ ਖਾਲੀ stop_ioerr_simulation(ਪੂਰਨ *piSave, ਪੂਰਨ *piSave2){
  *piSave = sqlite3_io_error_pending;
  *piSave2 = sqlite3_io_error_ਪ੍ਰਾਪਤੀ;
  sqlite3_io_error_pending = -1;
  sqlite3_io_error_ਪ੍ਰਾਪਤੀ = 0;
}
ਸਥਿਰ ਖਾਲੀ ਸ਼ੁਰੂ_ioerr_simulation(ਪੂਰਨ iSave, ਪੂਰਨ iSave2){
  sqlite3_io_error_pending = iSave;
  sqlite3_io_error_ਪ੍ਰਾਪਤੀ = iSave2;
}

/*
** The jt_file pointed to by the argument may or may not be a file-handle
** open on a main database file. If it is, and a transaction is currently
** opened on the file, then discard all transaction related data.
*/
ਸਥਿਰ ਖਾਲੀ closeTransaction(jt_file *p){
  sqlite3BitvecDestroy(p->pWritable);
  sqlite3_free(p->aCksum);
  p->pWritable = 0;
  p->aCksum = 0;
  p->nSync = 0;
}

/*
** Close an jt-file.
*/
ਸਥਿਰ ਪੂਰਨ jtClose(sqlite3_file *pFile){
  jt_file **pp;
  jt_file *p = (jt_file *)pFile;

  closeTransaction(p);
  enterJtMutex();
  ਜੇ( p->zName ){
    ਲਈ(pp=&g.pList; *pp!=p; pp=&(*pp)->p_ਅਗਲਾ);
    *pp = p->p_ਅਗਲਾ;
  }
  leaveJtMutex();
  sqlite3OsClose(p->pReal);
  ਵਾਪਸ SQLITE_OK;
}

/*
** Read data from an jt-file.
*/
ਸਥਿਰ ਪੂਰਨ jtRead(
  sqlite3_file *pFile, 
  ਖਾਲੀ *zBuf, 
  ਪੂਰਨ iAmt, 
  sqlite_int64 iOfst
){
  jt_file *p = (jt_file *)pFile;
  ਵਾਪਸ sqlite3OsRead(p->pReal, zBuf, iAmt, iOfst);
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
ਸਥਿਰ jt_file *locateDatabaseHandle(ਅਚਲ ਅੱਖਰ *zJournal, ਪੂਰਨ no_ਤਾਲਾ){
  jt_file *pMain = 0;
  enterJtMutex();
  ਲਈ(pMain=g.pList; pMain; pMain=pMain->p_ਅਗਲਾ){
    ਪੂਰਨ nName = (ਪੂਰਨ)(ਲੜੀ_ਲੰਬਾਈ(zJournal) - ਲੜੀ_ਲੰਬਾਈ("-journal"));
    ਜੇ( (pMain->flags&SQLITE_OPEN_MAIN_DB)
     && ((ਪੂਰਨ)ਲੜੀ_ਲੰਬਾਈ(pMain->zName)==nName)
     && 0==memcmp(pMain->zName, zJournal, nName)
     && ((pMain->e_ਤਾਲਾ>=SQLITE_ਤਾਲਾ_RESERVED) || no_ਤਾਲਾ)
    ){
      ਰੋਕੋ;
    }
  }
  leaveJtMutex();
  ਵਾਪਸ pMain;
}

/*
** Parameter z points to a buffer of 4 bytes in size containing a 
** unsigned 32-bit integer stored in big-endian format. Decode the 
** integer and return its value.
*/
ਸਥਿਰ u32 decodeUint32(ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *z){
  ਵਾਪਸ (z[0]<<24) + (z[1]<<16) + (z[2]<<8) + z[3];
}

/*
** Calculate a checksum from the buffer of length n bytes pointed to
** by parameter z.
*/
ਸਥਿਰ u32 genCksum(ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *z, ਪੂਰਨ n){
  ਪੂਰਨ i;
  u32 cksum = 0;
  ਲਈ(i=0; i<n; i++){
    cksum = cksum + z[i] + (cksum<<3);
  }
  ਵਾਪਸ cksum;
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
ਸਥਿਰ ਪੂਰਨ decodeJournalHdr(
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *zBuf,         /* Input: 28 byte journal header */
  u32 *pnRec,                        /* Out: Number of journalled records */
  u32 *pn_ਪੰਨਾ,                       /* Out: Original database page count */
  u32 *pnSector,                     /* Out: Sector size in bytes */
  u32 *pnPagesize                    /* Out: Page size in bytes */
){
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ aMagic[] = { 0xd9, 0xd5, 0x05, 0xf9, 0x20, 0xa1, 0x63, 0xd7 };
  ਜੇ( memcmp(aMagic, zBuf, 8) ) ਵਾਪਸ SQLITE_ERROR;
  ਜੇ( pnRec ) *pnRec = decodeUint32(&zBuf[8]);
  ਜੇ( pn_ਪੰਨਾ ) *pn_ਪੰਨਾ = decodeUint32(&zBuf[16]);
  ਜੇ( pnSector ) *pnSector = decodeUint32(&zBuf[20]);
  ਜੇ( pnPagesize ) *pnPagesize = decodeUint32(&zBuf[24]);
  ਵਾਪਸ SQLITE_OK;
}

/*
** This function is called when a new transaction is opened, just after
** the first journal-header is written to the journal file.
*/
ਸਥਿਰ ਪੂਰਨ openTransaction(jt_file *pMain, jt_file *pJournal){
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *a_ਅੰਕੜੇ;
  sqlite3_file *p = pMain->pReal;
  ਪੂਰਨ rc = SQLITE_OK;

  closeTransaction(pMain);
  a_ਅੰਕੜੇ = sqlite3_malloc(pMain->nPagesize);
  pMain->pWritable = sqlite3BitvecCreate(pMain->n_ਪੰਨਾ);
  pMain->aCksum = sqlite3_malloc(ਆਕਾਰ(u32) * (pMain->n_ਪੰਨਾ + 1));
  pJournal->iMaxOff = 0;

  ਜੇ( !pMain->pWritable || !pMain->aCksum || !a_ਅੰਕੜੇ ){
    rc = SQLITE_IOERR_NOMEM;
  }ਨਹੀਂਤਾਂ ਜੇ( pMain->n_ਪੰਨਾ>0 ){
    u32 iTrunk;
    ਪੂਰਨ iSave;
    ਪੂਰਨ iSave2;

    stop_ioerr_simulation(&iSave, &iSave2);

    /* Read the database free-list. Add the page-number for each free-list
    ** leaf to the jt_file.pWritable bitvec.
    */
    rc = sqlite3OsRead(p, a_ਅੰਕੜੇ, pMain->nPagesize, 0);
    ਜੇ( rc==SQLITE_OK ){
      u32 nDbsize = decodeUint32(&a_ਅੰਕੜੇ[28]);
      ਜੇ( nDbsize>0 && memcmp(&a_ਅੰਕੜੇ[24], &a_ਅੰਕੜੇ[92], 4)==0 ){
        u32 iPg;
        ਲਈ(iPg=nDbsize+1; iPg<=pMain->n_ਪੰਨਾ; iPg++){
          sqlite3_Bitvec_ਸੈੱਟ(pMain->pWritable, iPg);
        }
      }
    }
    iTrunk = decodeUint32(&a_ਅੰਕੜੇ[32]);
    ਜਦੋਂ( rc==SQLITE_OK && iTrunk>0 ){
      u32 n_ਪੱਤੀ;
      u32 i_ਪੱਤੀ;
      sqlite3_int64 iOff = (i64)(iTrunk-1)*pMain->nPagesize;
      rc = sqlite3OsRead(p, a_ਅੰਕੜੇ, pMain->nPagesize, iOff);
      n_ਪੱਤੀ = decodeUint32(&a_ਅੰਕੜੇ[4]);
      ਲਈ(i_ਪੱਤੀ=0; rc==SQLITE_OK && i_ਪੱਤੀ<n_ਪੱਤੀ; i_ਪੱਤੀ++){
        u32 pgno = decodeUint32(&a_ਅੰਕੜੇ[8+4*i_ਪੱਤੀ]);
        sqlite3_Bitvec_ਸੈੱਟ(pMain->pWritable, pgno);
      }
      iTrunk = decodeUint32(a_ਅੰਕੜੇ);
    }

    /* Calculate and store a checksum for each page in the database file. */
    ਜੇ( rc==SQLITE_OK ){
      ਪੂਰਨ ii;
      ਲਈ(ii=0; rc==SQLITE_OK && ii<(ਪੂਰਨ)pMain->n_ਪੰਨਾ; ii++){
        i64 iOff = (i64)(pMain->nPagesize) * (i64)ii;
        ਜੇ( iOff==PENDING_BYTE ) ਜਾਰੀ;
        rc = sqlite3OsRead(pMain->pReal, a_ਅੰਕੜੇ, pMain->nPagesize, iOff);
        pMain->aCksum[ii] = genCksum(a_ਅੰਕੜੇ, pMain->nPagesize);
        ਜੇ( ii+1==(ਪੂਰਨ)pMain->n_ਪੰਨਾ && rc==SQLITE_IOERR_SHORT_READ ){
          rc = SQLITE_OK;
        }
      }
    }

    ਸ਼ੁਰੂ_ioerr_simulation(iSave, iSave2);
  }

  sqlite3_free(a_ਅੰਕੜੇ);
  ਵਾਪਸ rc;
}

/*
** The first argument to this function is a handle open on a journal file.
** This function reads the journal file and adds the page number for each
** page in the journal to the Bitvec object passed as the second argument.
*/
ਸਥਿਰ ਪੂਰਨ readJournalFile(jt_file *p, jt_file *pMain){
  ਪੂਰਨ rc = SQLITE_OK;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ zBuf[28];
  sqlite3_file *pReal = p->pReal;
  sqlite3_int64 iOff = 0;
  sqlite3_int64 i_ਮਾਪ = p->iMaxOff;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *a_ਪੰਨਾ;
  ਪੂਰਨ iSave;
  ਪੂਰਨ iSave2;

  a_ਪੰਨਾ = sqlite3_malloc(pMain->nPagesize);
  ਜੇ( !a_ਪੰਨਾ ){
    ਵਾਪਸ SQLITE_IOERR_NOMEM;
  }

  stop_ioerr_simulation(&iSave, &iSave2);

  ਜਦੋਂ( rc==SQLITE_OK && iOff<i_ਮਾਪ ){
    u32 nRec, n_ਪੰਨਾ, nSector, nPagesize;
    u32 ii;

    /* Read and decode the next journal-header from the journal file. */
    rc = sqlite3OsRead(pReal, zBuf, 28, iOff);
    ਜੇ( rc!=SQLITE_OK 
     || decodeJournalHdr(zBuf, &nRec, &n_ਪੰਨਾ, &nSector, &nPagesize) 
    ){
      ਜਾਓ ਅੰਤ_rjf;
    }
    iOff += nSector;

    ਜੇ( nRec==0 ){
      /* A trick. There might be another journal-header immediately 
      ** following this one. In this case, 0 records means 0 records, 
      ** not "read until the end of the file". See also ticket #2565.
      */
      ਜੇ( i_ਮਾਪ>=(iOff+nSector) ){
        rc = sqlite3OsRead(pReal, zBuf, 28, iOff);
        ਜੇ( rc!=SQLITE_OK || 0==decodeJournalHdr(zBuf, 0, 0, 0, 0) ){
          ਜਾਰੀ;
        }
      }
      nRec = (u32)((i_ਮਾਪ-iOff) / (pMain->nPagesize+8));
    }

    /* Read all the records that follow the journal-header just read. */
    ਲਈ(ii=0; rc==SQLITE_OK && ii<nRec && iOff<i_ਮਾਪ; ii++){
      u32 pgno;
      rc = sqlite3OsRead(pReal, zBuf, 4, iOff);
      ਜੇ( rc==SQLITE_OK ){
        pgno = decodeUint32(zBuf);
        ਜੇ( pgno>0 && pgno<=pMain->n_ਪੰਨਾ ){
          ਜੇ( 0==sqlite3BitvecTest(pMain->pWritable, pgno) ){
            rc = sqlite3OsRead(pReal, a_ਪੰਨਾ, pMain->nPagesize, iOff+4);
            ਜੇ( rc==SQLITE_OK ){
              u32 cksum = genCksum(a_ਪੰਨਾ, pMain->nPagesize);
              ਪ੍ਰਤਿਜ੍ਞਾ( cksum==pMain->aCksum[pgno-1] );
            }
          }
          sqlite3_Bitvec_ਸੈੱਟ(pMain->pWritable, pgno);
        }
        iOff += (8 + pMain->nPagesize);
      }
    }

    iOff = ((iOff + (nSector-1)) / nSector) * nSector;
  }

ਅੰਤ_rjf:
  ਸ਼ੁਰੂ_ioerr_simulation(iSave, iSave2);
  sqlite3_free(a_ਪੰਨਾ);
  ਜੇ( rc==SQLITE_IOERR_SHORT_READ ){
    rc = SQLITE_OK;
  }
  ਵਾਪਸ rc;
}

/*
** Write data to an jt-file.
*/
ਸਥਿਰ ਪੂਰਨ jtWrite(
  sqlite3_file *pFile, 
  ਅਚਲ ਖਾਲੀ *zBuf, 
  ਪੂਰਨ iAmt, 
  sqlite_int64 iOfst
){
  ਪੂਰਨ rc;
  jt_file *p = (jt_file *)pFile;
  ਜੇ( p->flags&SQLITE_OPEN_MAIN_JOURNAL ){
    ਜੇ( iOfst==0 ){
      jt_file *pMain = locateDatabaseHandle(p->zName, 0);
      ਪ੍ਰਤਿਜ੍ਞਾ( pMain );
  
      ਜੇ( iAmt==28 ){
        /* Zeroing the first journal-file header. This is the end of a
        ** transaction. */
        closeTransaction(pMain);
      }ਨਹੀਂਤਾਂ ਜੇ( iAmt!=12 ){
        /* Writing the first journal header to a journal file. This happens
        ** when a transaction is first started.  */
        u8 *z = (u8 *)zBuf;
        pMain->n_ਪੰਨਾ = decodeUint32(&z[16]);
        pMain->nPagesize = decodeUint32(&z[24]);
        ਜੇ( SQLITE_OK!=(rc=openTransaction(pMain, p)) ){
          ਵਾਪਸ rc;
        }
      }
    }
    ਜੇ( p->iMaxOff<(iOfst + iAmt) ){
      p->iMaxOff = iOfst + iAmt;
    }
  }

  ਜੇ( p->flags&SQLITE_OPEN_MAIN_DB && p->pWritable ){
    ਜੇ( iAmt<(ਪੂਰਨ)p->nPagesize 
     && p->nPagesize%iAmt==0 
     && iOfst>=(PENDING_BYTE+512) 
     && iOfst+iAmt<=PENDING_BYTE+p->nPagesize
    ){
      /* No-op. This special case is hit when the backup code is copying a
      ** to a database with a larger page-size than the source database and
      ** it needs to fill in the non-locking-region part of the original
      ** pending-byte page.
      */
    }ਨਹੀਂਤਾਂ{
      u32 pgno = (u32)(iOfst/p->nPagesize + 1);
      ਪ੍ਰਤਿਜ੍ਞਾ( (iAmt==1||iAmt==(ਪੂਰਨ)p->nPagesize) &&
              ((iOfst+iAmt)%p->nPagesize)==0 );
      /* The following assert() statements may fail if this layer is used
      ** with a connection in "PRAGMA synchronous=off" mode. If they
      ** fail with sync=normal or sync=full, this may indicate problem.  */
      ਪ੍ਰਤਿਜ੍ਞਾ( p->n_ਪੰਨਾ==0 || pgno<=p->n_ਪੰਨਾ || p->nSync>0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( pgno>p->n_ਪੰਨਾ || sqlite3BitvecTest(p->pWritable, pgno) );
    }
  }

  rc = sqlite3OsWrite(p->pReal, zBuf, iAmt, iOfst);
  ਜੇ( (p->flags&SQLITE_OPEN_MAIN_JOURNAL) && iAmt==12 ){
    jt_file *pMain = locateDatabaseHandle(p->zName, 0);
    ਪੂਰਨ rc2 = readJournalFile(p, pMain);
    ਜੇ( rc==SQLITE_OK ) rc = rc2;
  }
  ਵਾਪਸ rc;
}

/*
** Truncate an jt-file.
*/
ਸਥਿਰ ਪੂਰਨ jtTruncate(sqlite3_file *pFile, sqlite_int64 ਮਾਪ){
  jt_file *p = (jt_file *)pFile;
  ਜੇ( p->flags&SQLITE_OPEN_MAIN_JOURNAL && ਮਾਪ==0 ){
    /* Truncating a journal file. This is the end of a transaction. */
    jt_file *pMain = locateDatabaseHandle(p->zName, 0);
    closeTransaction(pMain);
  }
  ਜੇ( p->flags&SQLITE_OPEN_MAIN_DB && p->pWritable ){
    u32 pgno;
    u32 locking_ਪੰਨਾ = (u32)(PENDING_BYTE/p->nPagesize+1);
    ਲਈ(pgno=(u32)(ਮਾਪ/p->nPagesize+1); pgno<=p->n_ਪੰਨਾ; pgno++){
      ਪ੍ਰਤਿਜ੍ਞਾ( pgno==locking_ਪੰਨਾ || sqlite3BitvecTest(p->pWritable, pgno) );
    }
  }
  ਵਾਪਸ sqlite3OsTruncate(p->pReal, ਮਾਪ);
}

/*
** Sync an jt-file.
*/
ਸਥਿਰ ਪੂਰਨ jtSync(sqlite3_file *pFile, ਪੂਰਨ flags){
  jt_file *p = (jt_file *)pFile;

  ਜੇ( p->flags&SQLITE_OPEN_MAIN_JOURNAL ){
    ਪੂਰਨ rc;
    jt_file *pMain;                   /* The associated database file */

    /* The journal file is being synced. At this point, we inspect the 
    ** contents of the file up to this point and set each bit in the 
    ** jt_file.pWritable bitvec of the main database file associated with
    ** this journal file.
    */
    pMain = locateDatabaseHandle(p->zName, 0);

    /* Set the bitvec values */
    ਜੇ( pMain && pMain->pWritable ){
      pMain->nSync++;
      rc = readJournalFile(p, pMain);
      ਜੇ( rc!=SQLITE_OK ){
        ਵਾਪਸ rc;
      }
    }
  }

  ਵਾਪਸ sqlite3OsSync(p->pReal, flags);
}

/*
** Return the current file-size of an jt-file.
*/
ਸਥਿਰ ਪੂਰਨ jt_File_ਮਾਪ(sqlite3_file *pFile, sqlite_int64 *p_ਮਾਪ){
  jt_file *p = (jt_file *)pFile;
  ਵਾਪਸ sqlite3_Os_File_ਮਾਪ(p->pReal, p_ਮਾਪ);
}

/*
** Lock an jt-file.
*/
ਸਥਿਰ ਪੂਰਨ jt_ਤਾਲਾ(sqlite3_file *pFile, ਪੂਰਨ e_ਤਾਲਾ){
  ਪੂਰਨ rc;
  jt_file *p = (jt_file *)pFile;
  rc = sqlite3_Os_ਤਾਲਾ(p->pReal, e_ਤਾਲਾ);
  ਜੇ( rc==SQLITE_OK && e_ਤਾਲਾ>p->e_ਤਾਲਾ ){
    p->e_ਤਾਲਾ = e_ਤਾਲਾ;
  }
  ਵਾਪਸ rc;
}

/*
** Unlock an jt-file.
*/
ਸਥਿਰ ਪੂਰਨ jtUnlock(sqlite3_file *pFile, ਪੂਰਨ e_ਤਾਲਾ){
  ਪੂਰਨ rc;
  jt_file *p = (jt_file *)pFile;
  rc = sqlite3OsUnlock(p->pReal, e_ਤਾਲਾ);
  ਜੇ( rc==SQLITE_OK && e_ਤਾਲਾ<p->e_ਤਾਲਾ ){
    p->e_ਤਾਲਾ = e_ਤਾਲਾ;
  }
  ਵਾਪਸ rc;
}

/*
** Check if another file-handle holds a RESERVED lock on an jt-file.
*/
ਸਥਿਰ ਪੂਰਨ jt_Check_Reserved_ਤਾਲਾ(sqlite3_file *pFile, ਪੂਰਨ *p_Res_ਬਾਹਰਲਾ){
  jt_file *p = (jt_file *)pFile;
  ਵਾਪਸ sqlite3_Os_Check_Reserved_ਤਾਲਾ(p->pReal, p_Res_ਬਾਹਰਲਾ);
}

/*
** File control method. For custom operations on an jt-file.
*/
ਸਥਿਰ ਪੂਰਨ jtFileControl(sqlite3_file *pFile, ਪੂਰਨ op, ਖਾਲੀ *pArg){
  jt_file *p = (jt_file *)pFile;
  ਵਾਪਸ p->pReal->pMethods->xFileControl(p->pReal, op, pArg);
}

/*
** Return the sector-size in bytes for an jt-file.
*/
ਸਥਿਰ ਪੂਰਨ jt_Sector_ਮਾਪ(sqlite3_file *pFile){
  jt_file *p = (jt_file *)pFile;
  ਵਾਪਸ sqlite3_Os_Sector_ਮਾਪ(p->pReal);
}

/*
** Return the device characteristic flags supported by an jt-file.
*/
ਸਥਿਰ ਪੂਰਨ jtDeviceCharacteristics(sqlite3_file *pFile){
  jt_file *p = (jt_file *)pFile;
  ਵਾਪਸ sqlite3OsDeviceCharacteristics(p->pReal);
}

/*
** Open an jt file handle.
*/
ਸਥਿਰ ਪੂਰਨ jtOpen(
  sqlite3_vfs *pVfs,
  ਅਚਲ ਅੱਖਰ *zName,
  sqlite3_file *pFile,
  ਪੂਰਨ flags,
  ਪੂਰਨ *p_ਬਾਹਰਲਾ_Flags
){
  ਪੂਰਨ rc;
  jt_file *p = (jt_file *)pFile;
  pFile->pMethods = 0;
  p->pReal = (sqlite3_file *)&p[1];
  p->pReal->pMethods = 0;
  rc = sqlite3OsOpen(g.pVfs, zName, p->pReal, flags, p_ਬਾਹਰਲਾ_Flags);
  ਪ੍ਰਤਿਜ੍ਞਾ( rc==SQLITE_OK || p->pReal->pMethods==0 );
  ਜੇ( rc==SQLITE_OK ){
    pFile->pMethods = &jt_io_methods;
    p->e_ਤਾਲਾ = 0;
    p->zName = zName;
    p->flags = flags;
    p->p_ਅਗਲਾ = 0;
    p->pWritable = 0;
    p->aCksum = 0;
    enterJtMutex();
    ਜੇ( zName ){
      p->p_ਅਗਲਾ = g.pList;
      g.pList = p;
    }
    leaveJtMutex();
  }
  ਵਾਪਸ rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
ਸਥਿਰ ਪੂਰਨ jtDelete(sqlite3_vfs *pVfs, ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ, ਪੂਰਨ dirSync){
  ਪੂਰਨ n_ਰਸਤਾ = (ਪੂਰਨ)ਲੜੀ_ਲੰਬਾਈ(z_ਰਸਤਾ);
  ਜੇ( n_ਰਸਤਾ>8 && 0==strcmp("-journal", &z_ਰਸਤਾ[n_ਰਸਤਾ-8]) ){
    /* Deleting a journal file. The end of a transaction. */
    jt_file *pMain = locateDatabaseHandle(z_ਰਸਤਾ, 0);
    ਜੇ( pMain ){
      closeTransaction(pMain);
    }
  }

  ਵਾਪਸ sqlite3OsDelete(g.pVfs, z_ਰਸਤਾ, dirSync);
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
ਸਥਿਰ ਪੂਰਨ jtAccess(
  sqlite3_vfs *pVfs, 
  ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ, 
  ਪੂਰਨ flags, 
  ਪੂਰਨ *p_Res_ਬਾਹਰਲਾ
){
  ਵਾਪਸ sqlite3OsAccess(g.pVfs, z_ਰਸਤਾ, flags, p_Res_ਬਾਹਰਲਾ);
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (JT_MAX_PATHNAME+1) bytes.
*/
ਸਥਿਰ ਪੂਰਨ jt_ਭਰਿਆ_Pathname(
  sqlite3_vfs *pVfs, 
  ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ, 
  ਪੂਰਨ n_ਬਾਹਰਲਾ, 
  ਅੱਖਰ *z_ਬਾਹਰਲਾ
){
  ਵਾਪਸ sqlite3_Os_ਭਰਿਆ_Pathname(g.pVfs, z_ਰਸਤਾ, n_ਬਾਹਰਲਾ, z_ਬਾਹਰਲਾ);
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
ਸਥਿਰ ਖਾਲੀ *jtDlOpen(sqlite3_vfs *pVfs, ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ){
  ਵਾਪਸ g.pVfs->xDlOpen(g.pVfs, z_ਰਸਤਾ);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
ਸਥਿਰ ਖਾਲੀ jtDlError(sqlite3_vfs *pVfs, ਪੂਰਨ nByte, ਅੱਖਰ *z_Err_ਸੁਨੇਹਾ){
  g.pVfs->xDlError(g.pVfs, nByte, z_Err_ਸੁਨੇਹਾ);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
ਸਥਿਰ ਖਾਲੀ (*jtDlSym(sqlite3_vfs *pVfs, ਖਾਲੀ *p, ਅਚਲ ਅੱਖਰ *zSym))(ਖਾਲੀ){
  ਵਾਪਸ g.pVfs->xDlSym(g.pVfs, p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
ਸਥਿਰ ਖਾਲੀ jtDlClose(sqlite3_vfs *pVfs, ਖਾਲੀ *pHandle){
  g.pVfs->xDlClose(g.pVfs, pHandle);
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
ਸਥਿਰ ਪੂਰਨ jtRandomness(sqlite3_vfs *pVfs, ਪੂਰਨ nByte, ਅੱਖਰ *z_Buf_ਬਾਹਰਲਾ){
  ਵਾਪਸ sqlite3OsRandomness(g.pVfs, nByte, z_Buf_ਬਾਹਰਲਾ);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
ਸਥਿਰ ਪੂਰਨ jtSleep(sqlite3_vfs *pVfs, ਪੂਰਨ nMicro){
  ਵਾਪਸ sqlite3OsSleep(g.pVfs, nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
ਸਥਿਰ ਪੂਰਨ jt_Current_ਸਮਾਂ(sqlite3_vfs *pVfs, ਦੁੱਗਣਾ *p_ਸਮਾਂ_ਬਾਹਰਲਾ){
  ਵਾਪਸ g.pVfs->x_Current_ਸਮਾਂ(g.pVfs, p_ਸਮਾਂ_ਬਾਹਰਲਾ);
}
/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
ਸਥਿਰ ਪੂਰਨ jt_Current_ਸਮਾਂ_Int64(sqlite3_vfs *pVfs, sqlite3_int64 *p_ਸਮਾਂ_ਬਾਹਰਲਾ){
  ਵਾਪਸ g.pVfs->x_Current_ਸਮਾਂ_Int64(g.pVfs, p_ਸਮਾਂ_ਬਾਹਰਲਾ);
}

ਸਥਿਰ ਪੂਰਨ jt_Get_ਆਖਰੀ_Error(sqlite3_vfs *pVfs, ਪੂਰਨ n, ਅੱਖਰ *z){
  ਵਾਪਸ g.pVfs->x_Get_ਆਖਰੀ_Error(g.pVfs, n, z);
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
ਪੂਰਨ jt_ਰਜਿਸਟਰ(ਅੱਖਰ *zWrap, ਪੂਰਨ isDefault){
  g.pVfs = sqlite3_vfs_ਲੱਭੋ(zWrap);
  ਜੇ( g.pVfs==0 ){
    ਵਾਪਸ SQLITE_ERROR;
  }
  jt_vfs.szOsFile = ਆਕਾਰ(jt_file) + g.pVfs->szOsFile;
  ਜੇ( g.pVfs->i_ਸੰਸਕਰਣ==1 ){
    jt_vfs.i_ਸੰਸਕਰਣ = 1;
  }ਨਹੀਂਤਾਂ ਜੇ( g.pVfs->x_Current_ਸਮਾਂ_Int64==0 ){
    jt_vfs.x_Current_ਸਮਾਂ_Int64 = 0;
  }
  sqlite3_vfs_ਰਜਿਸਟਰ(&jt_vfs, isDefault);
  ਵਾਪਸ SQLITE_OK;
}

/*
** Uninstall the jt VFS, if it is installed.
*/
ਖਾਲੀ jt_unregister(ਖਾਲੀ){
  sqlite3_vfs_unregister(&jt_vfs);
}

#ਜੇ_ਅੰਤ
