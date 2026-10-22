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
#ಆದರೆ SQLITE_TEST          /* This file is used for testing only */

#ಅನ್ತರ್ಭಾವ "sqlite3.h"
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"

/*
** Maximum pathname length supported by the jt backend.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು JT_MAX_PATHNAME 512

/*
** Name used to identify this VFS.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು JT_VFS_NAME "jt"

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ jt_file jt_file;
ರಚನೆ jt_file {
  sqlite3_file ತಳ;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName;       /* Name of open file */
  ಪೂರ್ಣಾಂಕ flags;               /* Flags the file was opened with */

  /* The following are only used by database file file handles */
  ಪೂರ್ಣಾಂಕ e_ಬೀಗ;               /* Current lock held on the file */
  u32 n_ಪುಟ;               /* Size of file in pages when transaction started */
  u32 nPagesize;           /* Page size when transaction started */
  Bitvec *pWritable;       /* Bitvec of pages that may be written to the file */
  u32 *aCksum;             /* Checksum for first nPage pages */
  ಪೂರ್ಣಾಂಕ nSync;               /* Number of times journal file has been synced */

  /* Only used by journal file-handles */
  sqlite3_int64 iMaxOff;   /* Maximum offset written to this transaction */

  jt_file *p_ಮುಂದಿನ;          /* All files are stored in a linked list */
  sqlite3_file *pReal;     /* The file handle for the underlying vfs */
};

/*
** Method declarations for jt_file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jtClose(sqlite3_file*);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jtRead(sqlite3_file*, ಶೂನ್ಯ*, ಪೂರ್ಣಾಂಕ iAmt, sqlite3_int64 iOfst);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jtWrite(sqlite3_file*,ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*,ಪೂರ್ಣಾಂಕ iAmt, sqlite3_int64 iOfst);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jtTruncate(sqlite3_file*, sqlite3_int64 ಗಾತ್ರ_2);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jtSync(sqlite3_file*, ಪೂರ್ಣಾಂಕ flags);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jt_File_ಗಾತ್ರ(sqlite3_file*, sqlite3_int64 *p_ಗಾತ್ರ);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jt_ಬೀಗ(sqlite3_file*, ಪೂರ್ಣಾಂಕ);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jtUnlock(sqlite3_file*, ಪೂರ್ಣಾಂಕ);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jt_Check_Reserved_ಬೀಗ(sqlite3_file*, ಪೂರ್ಣಾಂಕ *);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jtFileControl(sqlite3_file*, ಪೂರ್ಣಾಂಕ op, ಶೂನ್ಯ *pArg);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jt_Sector_ಗಾತ್ರ(sqlite3_file*);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jtDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for jt_vfs.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jtOpen(sqlite3_vfs*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *, sqlite3_file*, ಪೂರ್ಣಾಂಕ , ಪೂರ್ಣಾಂಕ *);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jtDelete(sqlite3_vfs*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName, ಪೂರ್ಣಾಂಕ syncDir);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jtAccess(sqlite3_vfs*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName, ಪೂರ್ಣಾಂಕ flags, ಪೂರ್ಣಾಂಕ *);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jt_ತುಂಬಿದ_Pathname(sqlite3_vfs*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName, ಪೂರ್ಣಾಂಕ, ಅಕ್ಷರ *z_ಹೊರಹೋಗು);
ಸ್ಥಿರ ಶೂನ್ಯ *jtDlOpen(sqlite3_vfs*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFilename);
ಸ್ಥಿರ ಶೂನ್ಯ jtDlError(sqlite3_vfs*, ಪೂರ್ಣಾಂಕ nByte, ಅಕ್ಷರ *z_Err_ಸಂದೇಶ);
ಸ್ಥಿರ ಶೂನ್ಯ (*jtDlSym(sqlite3_vfs*,ಶೂನ್ಯ*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಚಿಹ್ನೆ))(ಶೂನ್ಯ);
ಸ್ಥಿರ ಶೂನ್ಯ jtDlClose(sqlite3_vfs*, ಶೂನ್ಯ*);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jtRandomness(sqlite3_vfs*, ಪೂರ್ಣಾಂಕ nByte, ಅಕ್ಷರ *z_ಹೊರಹೋಗು);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jtSleep(sqlite3_vfs*, ಪೂರ್ಣಾಂಕ microseconds);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jt_Current_ಸಮಯ(sqlite3_vfs*, ದ್ವಿಗುಣ*);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jt_Current_ಸಮಯ_Int64(sqlite3_vfs*, sqlite3_int64*);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jt_Get_ಕೊನೆಯ_Error(sqlite3_vfs*, ಪೂರ್ಣಾಂಕ, ಅಕ್ಷರ*);

ಸ್ಥಿರ sqlite3_vfs jt_vfs = {
  2,                             /* iVersion */
  ಗಾತ್ರ(jt_file),               /* szOsFile */
  JT_MAX_PATHNAME,               /* mxPathname */
  0,                             /* pNext */
  JT_VFS_NAME,                   /* zName */
  0,                             /* pAppData */
  jtOpen,                        /* xOpen */
  jtDelete,                      /* xDelete */
  jtAccess,                      /* xAccess */
  jt_ತುಂಬಿದ_Pathname,                /* xFullPathname */
  jtDlOpen,                      /* xDlOpen */
  jtDlError,                     /* xDlError */
  jtDlSym,                       /* xDlSym */
  jtDlClose,                     /* xDlClose */
  jtRandomness,                  /* xRandomness */
  jtSleep,                       /* xSleep */
  jt_Current_ಸಮಯ,                 /* xCurrentTime */
  jt_Get_ಕೊನೆಯ_Error,                /* xGetLastError */
  jt_Current_ಸಮಯ_Int64             /* xCurrentTimeInt64 */
};

ಸ್ಥಿರ sqlite3_io_methods jt_io_methods = {
  1,                             /* iVersion */
  jtClose,                       /* xClose */
  jtRead,                        /* xRead */
  jtWrite,                       /* xWrite */
  jtTruncate,                    /* xTruncate */
  jtSync,                        /* xSync */
  jt_File_ಗಾತ್ರ,                    /* xFileSize */
  jt_ಬೀಗ,                        /* xLock */
  jtUnlock,                      /* xUnlock */
  jt_Check_Reserved_ಬೀಗ,           /* xCheckReservedLock */
  jtFileControl,                 /* xFileControl */
  jt_Sector_ಗಾತ್ರ,                  /* xSectorSize */
  jtDeviceCharacteristics        /* xDeviceCharacteristics */
};

ರಚನೆ JtGlobal {
  sqlite3_vfs *pVfs;             /* Parent VFS */
  jt_file *pList;                /* List of all open files */
};
ಸ್ಥಿರ ರಚನೆ JtGlobal g = {0, 0};

/*
** Functions to obtain and relinquish a mutex to protect g.pList. The
** STATIC_PRNG mutex is reused, purely for the sake of convenience.
*/
ಸ್ಥಿರ ಶೂನ್ಯ enterJtMutex(ಶೂನ್ಯ){
  sqlite3_mutex_enter(sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_PRNG));
}
ಸ್ಥಿರ ಶೂನ್ಯ leaveJtMutex(ಶೂನ್ಯ){
  sqlite3_mutex_leave(sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_PRNG));
}

ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ sqlite3_io_error_pending;
ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ sqlite3_io_error_ದೊರಕಿತು;
ಸ್ಥಿರ ಶೂನ್ಯ stop_ioerr_simulation(ಪೂರ್ಣಾಂಕ *piSave, ಪೂರ್ಣಾಂಕ *piSave2){
  *piSave = sqlite3_io_error_pending;
  *piSave2 = sqlite3_io_error_ದೊರಕಿತು;
  sqlite3_io_error_pending = -1;
  sqlite3_io_error_ದೊರಕಿತು = 0;
}
ಸ್ಥಿರ ಶೂನ್ಯ ಆರಂಭ_ioerr_simulation(ಪೂರ್ಣಾಂಕ iSave, ಪೂರ್ಣಾಂಕ iSave2){
  sqlite3_io_error_pending = iSave;
  sqlite3_io_error_ದೊರಕಿತು = iSave2;
}

/*
** The jt_file pointed to by the argument may or may not be a file-handle
** open on a main database file. If it is, and a transaction is currently
** opened on the file, then discard all transaction related data.
*/
ಸ್ಥಿರ ಶೂನ್ಯ closeTransaction(jt_file *p){
  sqlite3BitvecDestroy(p->pWritable);
  sqlite3_free(p->aCksum);
  p->pWritable = 0;
  p->aCksum = 0;
  p->nSync = 0;
}

/*
** Close an jt-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jtClose(sqlite3_file *pFile){
  jt_file **pp;
  jt_file *p = (jt_file *)pFile;

  closeTransaction(p);
  enterJtMutex();
  ಆದರೆ( p->zName ){
    ಗಾಗಿ(pp=&g.pList; *pp!=p; pp=&(*pp)->p_ಮುಂದಿನ);
    *pp = p->p_ಮುಂದಿನ;
  }
  leaveJtMutex();
  sqlite3OsClose(p->pReal);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Read data from an jt-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jtRead(
  sqlite3_file *pFile, 
  ಶೂನ್ಯ *zBuf, 
  ಪೂರ್ಣಾಂಕ iAmt, 
  sqlite_int64 iOfst
){
  jt_file *p = (jt_file *)pFile;
  ಹಿಂತಿರುಗಿ sqlite3OsRead(p->pReal, zBuf, iAmt, iOfst);
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
ಸ್ಥಿರ jt_file *locateDatabaseHandle(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zJournal, ಪೂರ್ಣಾಂಕ no_ಬೀಗ){
  jt_file *pMain = 0;
  enterJtMutex();
  ಗಾಗಿ(pMain=g.pList; pMain; pMain=pMain->p_ಮುಂದಿನ){
    ಪೂರ್ಣಾಂಕ nName = (ಪೂರ್ಣಾಂಕ)(ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(zJournal) - ಅಕ್ಷರಸರಣಿ_ಉದ್ದ("-journal"));
    ಆದರೆ( (pMain->flags&SQLITE_OPEN_MAIN_DB)
     && ((ಪೂರ್ಣಾಂಕ)ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(pMain->zName)==nName)
     && 0==memcmp(pMain->zName, zJournal, nName)
     && ((pMain->e_ಬೀಗ>=SQLITE_ಬೀಗ_RESERVED) || no_ಬೀಗ)
    ){
      ನಿಲ್ಲಿಸು;
    }
  }
  leaveJtMutex();
  ಹಿಂತಿರುಗಿ pMain;
}

/*
** Parameter z points to a buffer of 4 bytes in size containing a 
** unsigned 32-bit integer stored in big-endian format. Decode the 
** integer and return its value.
*/
ಸ್ಥಿರ u32 decodeUint32(ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *z){
  ಹಿಂತಿರುಗಿ (z[0]<<24) + (z[1]<<16) + (z[2]<<8) + z[3];
}

/*
** Calculate a checksum from the buffer of length n bytes pointed to
** by parameter z.
*/
ಸ್ಥಿರ u32 genCksum(ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *z, ಪೂರ್ಣಾಂಕ n){
  ಪೂರ್ಣಾಂಕ i;
  u32 cksum = 0;
  ಗಾಗಿ(i=0; i<n; i++){
    cksum = cksum + z[i] + (cksum<<3);
  }
  ಹಿಂತಿರುಗಿ cksum;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ decodeJournalHdr(
  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *zBuf,         /* Input: 28 byte journal header */
  u32 *pnRec,                        /* Out: Number of journalled records */
  u32 *pn_ಪುಟ,                       /* Out: Original database page count */
  u32 *pnSector,                     /* Out: Sector size in bytes */
  u32 *pnPagesize                    /* Out: Page size in bytes */
){
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ aMagic[] = { 0xd9, 0xd5, 0x05, 0xf9, 0x20, 0xa1, 0x63, 0xd7 };
  ಆದರೆ( memcmp(aMagic, zBuf, 8) ) ಹಿಂತಿರುಗಿ SQLITE_ERROR;
  ಆದರೆ( pnRec ) *pnRec = decodeUint32(&zBuf[8]);
  ಆದರೆ( pn_ಪುಟ ) *pn_ಪುಟ = decodeUint32(&zBuf[16]);
  ಆದರೆ( pnSector ) *pnSector = decodeUint32(&zBuf[20]);
  ಆದರೆ( pnPagesize ) *pnPagesize = decodeUint32(&zBuf[24]);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** This function is called when a new transaction is opened, just after
** the first journal-header is written to the journal file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ openTransaction(jt_file *pMain, jt_file *pJournal){
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *a_ದತ್ತಾಂಶ;
  sqlite3_file *p = pMain->pReal;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;

  closeTransaction(pMain);
  a_ದತ್ತಾಂಶ = sqlite3_malloc(pMain->nPagesize);
  pMain->pWritable = sqlite3BitvecCreate(pMain->n_ಪುಟ);
  pMain->aCksum = sqlite3_malloc(ಗಾತ್ರ(u32) * (pMain->n_ಪುಟ + 1));
  pJournal->iMaxOff = 0;

  ಆದರೆ( !pMain->pWritable || !pMain->aCksum || !a_ದತ್ತಾಂಶ ){
    rc = SQLITE_IOERR_NOMEM;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pMain->n_ಪುಟ>0 ){
    u32 iTrunk;
    ಪೂರ್ಣಾಂಕ iSave;
    ಪೂರ್ಣಾಂಕ iSave2;

    stop_ioerr_simulation(&iSave, &iSave2);

    /* Read the database free-list. Add the page-number for each free-list
    ** leaf to the jt_file.pWritable bitvec.
    */
    rc = sqlite3OsRead(p, a_ದತ್ತಾಂಶ, pMain->nPagesize, 0);
    ಆದರೆ( rc==SQLITE_OK ){
      u32 nDbsize = decodeUint32(&a_ದತ್ತಾಂಶ[28]);
      ಆದರೆ( nDbsize>0 && memcmp(&a_ದತ್ತಾಂಶ[24], &a_ದತ್ತಾಂಶ[92], 4)==0 ){
        u32 iPg;
        ಗಾಗಿ(iPg=nDbsize+1; iPg<=pMain->n_ಪುಟ; iPg++){
          sqlite3_Bitvec_ಗಣ(pMain->pWritable, iPg);
        }
      }
    }
    iTrunk = decodeUint32(&a_ದತ್ತಾಂಶ[32]);
    ಆಗ( rc==SQLITE_OK && iTrunk>0 ){
      u32 n_ಎಲೆ;
      u32 i_ಎಲೆ;
      sqlite3_int64 iOff = (i64)(iTrunk-1)*pMain->nPagesize;
      rc = sqlite3OsRead(p, a_ದತ್ತಾಂಶ, pMain->nPagesize, iOff);
      n_ಎಲೆ = decodeUint32(&a_ದತ್ತಾಂಶ[4]);
      ಗಾಗಿ(i_ಎಲೆ=0; rc==SQLITE_OK && i_ಎಲೆ<n_ಎಲೆ; i_ಎಲೆ++){
        u32 pgno = decodeUint32(&a_ದತ್ತಾಂಶ[8+4*i_ಎಲೆ]);
        sqlite3_Bitvec_ಗಣ(pMain->pWritable, pgno);
      }
      iTrunk = decodeUint32(a_ದತ್ತಾಂಶ);
    }

    /* Calculate and store a checksum for each page in the database file. */
    ಆದರೆ( rc==SQLITE_OK ){
      ಪೂರ್ಣಾಂಕ ii;
      ಗಾಗಿ(ii=0; rc==SQLITE_OK && ii<(ಪೂರ್ಣಾಂಕ)pMain->n_ಪುಟ; ii++){
        i64 iOff = (i64)(pMain->nPagesize) * (i64)ii;
        ಆದರೆ( iOff==PENDING_BYTE ) ಮುಂದುವರಿ;
        rc = sqlite3OsRead(pMain->pReal, a_ದತ್ತಾಂಶ, pMain->nPagesize, iOff);
        pMain->aCksum[ii] = genCksum(a_ದತ್ತಾಂಶ, pMain->nPagesize);
        ಆದರೆ( ii+1==(ಪೂರ್ಣಾಂಕ)pMain->n_ಪುಟ && rc==SQLITE_IOERR_SHORT_READ ){
          rc = SQLITE_OK;
        }
      }
    }

    ಆರಂಭ_ioerr_simulation(iSave, iSave2);
  }

  sqlite3_free(a_ದತ್ತಾಂಶ);
  ಹಿಂತಿರುಗಿ rc;
}

/*
** The first argument to this function is a handle open on a journal file.
** This function reads the journal file and adds the page number for each
** page in the journal to the Bitvec object passed as the second argument.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ readJournalFile(jt_file *p, jt_file *pMain){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ zBuf[28];
  sqlite3_file *pReal = p->pReal;
  sqlite3_int64 iOff = 0;
  sqlite3_int64 i_ಗಾತ್ರ = p->iMaxOff;
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *a_ಪುಟ;
  ಪೂರ್ಣಾಂಕ iSave;
  ಪೂರ್ಣಾಂಕ iSave2;

  a_ಪುಟ = sqlite3_malloc(pMain->nPagesize);
  ಆದರೆ( !a_ಪುಟ ){
    ಹಿಂತಿರುಗಿ SQLITE_IOERR_NOMEM;
  }

  stop_ioerr_simulation(&iSave, &iSave2);

  ಆಗ( rc==SQLITE_OK && iOff<i_ಗಾತ್ರ ){
    u32 nRec, n_ಪುಟ, nSector, nPagesize;
    u32 ii;

    /* Read and decode the next journal-header from the journal file. */
    rc = sqlite3OsRead(pReal, zBuf, 28, iOff);
    ಆದರೆ( rc!=SQLITE_OK 
     || decodeJournalHdr(zBuf, &nRec, &n_ಪುಟ, &nSector, &nPagesize) 
    ){
      ಹೋಗು ಮುಕ್ತಾಯ_rjf;
    }
    iOff += nSector;

    ಆದರೆ( nRec==0 ){
      /* A trick. There might be another journal-header immediately 
      ** following this one. In this case, 0 records means 0 records, 
      ** not "read until the end of the file". See also ticket #2565.
      */
      ಆದರೆ( i_ಗಾತ್ರ>=(iOff+nSector) ){
        rc = sqlite3OsRead(pReal, zBuf, 28, iOff);
        ಆದರೆ( rc!=SQLITE_OK || 0==decodeJournalHdr(zBuf, 0, 0, 0, 0) ){
          ಮುಂದುವರಿ;
        }
      }
      nRec = (u32)((i_ಗಾತ್ರ-iOff) / (pMain->nPagesize+8));
    }

    /* Read all the records that follow the journal-header just read. */
    ಗಾಗಿ(ii=0; rc==SQLITE_OK && ii<nRec && iOff<i_ಗಾತ್ರ; ii++){
      u32 pgno;
      rc = sqlite3OsRead(pReal, zBuf, 4, iOff);
      ಆದರೆ( rc==SQLITE_OK ){
        pgno = decodeUint32(zBuf);
        ಆದರೆ( pgno>0 && pgno<=pMain->n_ಪುಟ ){
          ಆದರೆ( 0==sqlite3BitvecTest(pMain->pWritable, pgno) ){
            rc = sqlite3OsRead(pReal, a_ಪುಟ, pMain->nPagesize, iOff+4);
            ಆದರೆ( rc==SQLITE_OK ){
              u32 cksum = genCksum(a_ಪುಟ, pMain->nPagesize);
              ಪ್ರತಿಜ್ಞಾ( cksum==pMain->aCksum[pgno-1] );
            }
          }
          sqlite3_Bitvec_ಗಣ(pMain->pWritable, pgno);
        }
        iOff += (8 + pMain->nPagesize);
      }
    }

    iOff = ((iOff + (nSector-1)) / nSector) * nSector;
  }

ಮುಕ್ತಾಯ_rjf:
  ಆರಂಭ_ioerr_simulation(iSave, iSave2);
  sqlite3_free(a_ಪುಟ);
  ಆದರೆ( rc==SQLITE_IOERR_SHORT_READ ){
    rc = SQLITE_OK;
  }
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Write data to an jt-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jtWrite(
  sqlite3_file *pFile, 
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *zBuf, 
  ಪೂರ್ಣಾಂಕ iAmt, 
  sqlite_int64 iOfst
){
  ಪೂರ್ಣಾಂಕ rc;
  jt_file *p = (jt_file *)pFile;
  ಆದರೆ( p->flags&SQLITE_OPEN_MAIN_JOURNAL ){
    ಆದರೆ( iOfst==0 ){
      jt_file *pMain = locateDatabaseHandle(p->zName, 0);
      ಪ್ರತಿಜ್ಞಾ( pMain );
  
      ಆದರೆ( iAmt==28 ){
        /* Zeroing the first journal-file header. This is the end of a
        ** transaction. */
        closeTransaction(pMain);
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( iAmt!=12 ){
        /* Writing the first journal header to a journal file. This happens
        ** when a transaction is first started.  */
        u8 *z = (u8 *)zBuf;
        pMain->n_ಪುಟ = decodeUint32(&z[16]);
        pMain->nPagesize = decodeUint32(&z[24]);
        ಆದರೆ( SQLITE_OK!=(rc=openTransaction(pMain, p)) ){
          ಹಿಂತಿರುಗಿ rc;
        }
      }
    }
    ಆದರೆ( p->iMaxOff<(iOfst + iAmt) ){
      p->iMaxOff = iOfst + iAmt;
    }
  }

  ಆದರೆ( p->flags&SQLITE_OPEN_MAIN_DB && p->pWritable ){
    ಆದರೆ( iAmt<(ಪೂರ್ಣಾಂಕ)p->nPagesize 
     && p->nPagesize%iAmt==0 
     && iOfst>=(PENDING_BYTE+512) 
     && iOfst+iAmt<=PENDING_BYTE+p->nPagesize
    ){
      /* No-op. This special case is hit when the backup code is copying a
      ** to a database with a larger page-size than the source database and
      ** it needs to fill in the non-locking-region part of the original
      ** pending-byte page.
      */
    }ಇಲ್ಲದಿದ್ದರೆ{
      u32 pgno = (u32)(iOfst/p->nPagesize + 1);
      ಪ್ರತಿಜ್ಞಾ( (iAmt==1||iAmt==(ಪೂರ್ಣಾಂಕ)p->nPagesize) &&
              ((iOfst+iAmt)%p->nPagesize)==0 );
      /* The following assert() statements may fail if this layer is used
      ** with a connection in "PRAGMA synchronous=off" mode. If they
      ** fail with sync=normal or sync=full, this may indicate problem.  */
      ಪ್ರತಿಜ್ಞಾ( p->n_ಪುಟ==0 || pgno<=p->n_ಪುಟ || p->nSync>0 );
      ಪ್ರತಿಜ್ಞಾ( pgno>p->n_ಪುಟ || sqlite3BitvecTest(p->pWritable, pgno) );
    }
  }

  rc = sqlite3OsWrite(p->pReal, zBuf, iAmt, iOfst);
  ಆದರೆ( (p->flags&SQLITE_OPEN_MAIN_JOURNAL) && iAmt==12 ){
    jt_file *pMain = locateDatabaseHandle(p->zName, 0);
    ಪೂರ್ಣಾಂಕ rc2 = readJournalFile(p, pMain);
    ಆದರೆ( rc==SQLITE_OK ) rc = rc2;
  }
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Truncate an jt-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jtTruncate(sqlite3_file *pFile, sqlite_int64 ಗಾತ್ರ_2){
  jt_file *p = (jt_file *)pFile;
  ಆದರೆ( p->flags&SQLITE_OPEN_MAIN_JOURNAL && ಗಾತ್ರ_2==0 ){
    /* Truncating a journal file. This is the end of a transaction. */
    jt_file *pMain = locateDatabaseHandle(p->zName, 0);
    closeTransaction(pMain);
  }
  ಆದರೆ( p->flags&SQLITE_OPEN_MAIN_DB && p->pWritable ){
    u32 pgno;
    u32 locking_ಪುಟ = (u32)(PENDING_BYTE/p->nPagesize+1);
    ಗಾಗಿ(pgno=(u32)(ಗಾತ್ರ_2/p->nPagesize+1); pgno<=p->n_ಪುಟ; pgno++){
      ಪ್ರತಿಜ್ಞಾ( pgno==locking_ಪುಟ || sqlite3BitvecTest(p->pWritable, pgno) );
    }
  }
  ಹಿಂತಿರುಗಿ sqlite3OsTruncate(p->pReal, ಗಾತ್ರ_2);
}

/*
** Sync an jt-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jtSync(sqlite3_file *pFile, ಪೂರ್ಣಾಂಕ flags){
  jt_file *p = (jt_file *)pFile;

  ಆದರೆ( p->flags&SQLITE_OPEN_MAIN_JOURNAL ){
    ಪೂರ್ಣಾಂಕ rc;
    jt_file *pMain;                   /* The associated database file */

    /* The journal file is being synced. At this point, we inspect the 
    ** contents of the file up to this point and set each bit in the 
    ** jt_file.pWritable bitvec of the main database file associated with
    ** this journal file.
    */
    pMain = locateDatabaseHandle(p->zName, 0);

    /* Set the bitvec values */
    ಆದರೆ( pMain && pMain->pWritable ){
      pMain->nSync++;
      rc = readJournalFile(p, pMain);
      ಆದರೆ( rc!=SQLITE_OK ){
        ಹಿಂತಿರುಗಿ rc;
      }
    }
  }

  ಹಿಂತಿರುಗಿ sqlite3OsSync(p->pReal, flags);
}

/*
** Return the current file-size of an jt-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jt_File_ಗಾತ್ರ(sqlite3_file *pFile, sqlite_int64 *p_ಗಾತ್ರ){
  jt_file *p = (jt_file *)pFile;
  ಹಿಂತಿರುಗಿ sqlite3_Os_File_ಗಾತ್ರ(p->pReal, p_ಗಾತ್ರ);
}

/*
** Lock an jt-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jt_ಬೀಗ(sqlite3_file *pFile, ಪೂರ್ಣಾಂಕ e_ಬೀಗ){
  ಪೂರ್ಣಾಂಕ rc;
  jt_file *p = (jt_file *)pFile;
  rc = sqlite3_Os_ಬೀಗ(p->pReal, e_ಬೀಗ);
  ಆದರೆ( rc==SQLITE_OK && e_ಬೀಗ>p->e_ಬೀಗ ){
    p->e_ಬೀಗ = e_ಬೀಗ;
  }
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Unlock an jt-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jtUnlock(sqlite3_file *pFile, ಪೂರ್ಣಾಂಕ e_ಬೀಗ){
  ಪೂರ್ಣಾಂಕ rc;
  jt_file *p = (jt_file *)pFile;
  rc = sqlite3OsUnlock(p->pReal, e_ಬೀಗ);
  ಆದರೆ( rc==SQLITE_OK && e_ಬೀಗ<p->e_ಬೀಗ ){
    p->e_ಬೀಗ = e_ಬೀಗ;
  }
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Check if another file-handle holds a RESERVED lock on an jt-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jt_Check_Reserved_ಬೀಗ(sqlite3_file *pFile, ಪೂರ್ಣಾಂಕ *p_Res_ಹೊರಹೋಗು){
  jt_file *p = (jt_file *)pFile;
  ಹಿಂತಿರುಗಿ sqlite3_Os_Check_Reserved_ಬೀಗ(p->pReal, p_Res_ಹೊರಹೋಗು);
}

/*
** File control method. For custom operations on an jt-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jtFileControl(sqlite3_file *pFile, ಪೂರ್ಣಾಂಕ op, ಶೂನ್ಯ *pArg){
  jt_file *p = (jt_file *)pFile;
  ಹಿಂತಿರುಗಿ p->pReal->pMethods->xFileControl(p->pReal, op, pArg);
}

/*
** Return the sector-size in bytes for an jt-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jt_Sector_ಗಾತ್ರ(sqlite3_file *pFile){
  jt_file *p = (jt_file *)pFile;
  ಹಿಂತಿರುಗಿ sqlite3_Os_Sector_ಗಾತ್ರ(p->pReal);
}

/*
** Return the device characteristic flags supported by an jt-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jtDeviceCharacteristics(sqlite3_file *pFile){
  jt_file *p = (jt_file *)pFile;
  ಹಿಂತಿರುಗಿ sqlite3OsDeviceCharacteristics(p->pReal);
}

/*
** Open an jt file handle.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jtOpen(
  sqlite3_vfs *pVfs,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName,
  sqlite3_file *pFile,
  ಪೂರ್ಣಾಂಕ flags,
  ಪೂರ್ಣಾಂಕ *p_ಹೊರಹೋಗು_Flags
){
  ಪೂರ್ಣಾಂಕ rc;
  jt_file *p = (jt_file *)pFile;
  pFile->pMethods = 0;
  p->pReal = (sqlite3_file *)&p[1];
  p->pReal->pMethods = 0;
  rc = sqlite3OsOpen(g.pVfs, zName, p->pReal, flags, p_ಹೊರಹೋಗು_Flags);
  ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_OK || p->pReal->pMethods==0 );
  ಆದರೆ( rc==SQLITE_OK ){
    pFile->pMethods = &jt_io_methods;
    p->e_ಬೀಗ = 0;
    p->zName = zName;
    p->flags = flags;
    p->p_ಮುಂದಿನ = 0;
    p->pWritable = 0;
    p->aCksum = 0;
    enterJtMutex();
    ಆದರೆ( zName ){
      p->p_ಮುಂದಿನ = g.pList;
      g.pList = p;
    }
    leaveJtMutex();
  }
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jtDelete(sqlite3_vfs *pVfs, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮಾರ್ಗ, ಪೂರ್ಣಾಂಕ dirSync){
  ಪೂರ್ಣಾಂಕ n_ಮಾರ್ಗ = (ಪೂರ್ಣಾಂಕ)ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(z_ಮಾರ್ಗ);
  ಆದರೆ( n_ಮಾರ್ಗ>8 && 0==strcmp("-journal", &z_ಮಾರ್ಗ[n_ಮಾರ್ಗ-8]) ){
    /* Deleting a journal file. The end of a transaction. */
    jt_file *pMain = locateDatabaseHandle(z_ಮಾರ್ಗ, 0);
    ಆದರೆ( pMain ){
      closeTransaction(pMain);
    }
  }

  ಹಿಂತಿರುಗಿ sqlite3OsDelete(g.pVfs, z_ಮಾರ್ಗ, dirSync);
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jtAccess(
  sqlite3_vfs *pVfs, 
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮಾರ್ಗ, 
  ಪೂರ್ಣಾಂಕ flags, 
  ಪೂರ್ಣಾಂಕ *p_Res_ಹೊರಹೋಗು
){
  ಹಿಂತಿರುಗಿ sqlite3OsAccess(g.pVfs, z_ಮಾರ್ಗ, flags, p_Res_ಹೊರಹೋಗು);
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (JT_MAX_PATHNAME+1) bytes.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jt_ತುಂಬಿದ_Pathname(
  sqlite3_vfs *pVfs, 
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮಾರ್ಗ, 
  ಪೂರ್ಣಾಂಕ n_ಹೊರಹೋಗು, 
  ಅಕ್ಷರ *z_ಹೊರಹೋಗು
){
  ಹಿಂತಿರುಗಿ sqlite3_Os_ತುಂಬಿದ_Pathname(g.pVfs, z_ಮಾರ್ಗ, n_ಹೊರಹೋಗು, z_ಹೊರಹೋಗು);
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
ಸ್ಥಿರ ಶೂನ್ಯ *jtDlOpen(sqlite3_vfs *pVfs, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮಾರ್ಗ){
  ಹಿಂತಿರುಗಿ g.pVfs->xDlOpen(g.pVfs, z_ಮಾರ್ಗ);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
ಸ್ಥಿರ ಶೂನ್ಯ jtDlError(sqlite3_vfs *pVfs, ಪೂರ್ಣಾಂಕ nByte, ಅಕ್ಷರ *z_Err_ಸಂದೇಶ){
  g.pVfs->xDlError(g.pVfs, nByte, z_Err_ಸಂದೇಶ);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
ಸ್ಥಿರ ಶೂನ್ಯ (*jtDlSym(sqlite3_vfs *pVfs, ಶೂನ್ಯ *p, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSym))(ಶೂನ್ಯ){
  ಹಿಂತಿರುಗಿ g.pVfs->xDlSym(g.pVfs, p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
ಸ್ಥಿರ ಶೂನ್ಯ jtDlClose(sqlite3_vfs *pVfs, ಶೂನ್ಯ *pHandle){
  g.pVfs->xDlClose(g.pVfs, pHandle);
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jtRandomness(sqlite3_vfs *pVfs, ಪೂರ್ಣಾಂಕ nByte, ಅಕ್ಷರ *z_Buf_ಹೊರಹೋಗು){
  ಹಿಂತಿರುಗಿ sqlite3OsRandomness(g.pVfs, nByte, z_Buf_ಹೊರಹೋಗು);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jtSleep(sqlite3_vfs *pVfs, ಪೂರ್ಣಾಂಕ nMicro){
  ಹಿಂತಿರುಗಿ sqlite3OsSleep(g.pVfs, nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jt_Current_ಸಮಯ(sqlite3_vfs *pVfs, ದ್ವಿಗುಣ *p_ಸಮಯ_ಹೊರಹೋಗು){
  ಹಿಂತಿರುಗಿ g.pVfs->x_Current_ಸಮಯ(g.pVfs, p_ಸಮಯ_ಹೊರಹೋಗು);
}
/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jt_Current_ಸಮಯ_Int64(sqlite3_vfs *pVfs, sqlite3_int64 *p_ಸಮಯ_ಹೊರಹೋಗು){
  ಹಿಂತಿರುಗಿ g.pVfs->x_Current_ಸಮಯ_Int64(g.pVfs, p_ಸಮಯ_ಹೊರಹೋಗು);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jt_Get_ಕೊನೆಯ_Error(sqlite3_vfs *pVfs, ಪೂರ್ಣಾಂಕ n, ಅಕ್ಷರ *z){
  ಹಿಂತಿರುಗಿ g.pVfs->x_Get_ಕೊನೆಯ_Error(g.pVfs, n, z);
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
ಪೂರ್ಣಾಂಕ jt_ರಿಜಿಸ್ಟರ್(ಅಕ್ಷರ *zWrap, ಪೂರ್ಣಾಂಕ isDefault){
  g.pVfs = sqlite3_vfs_ಕಂಡುಹಿಡಿ(zWrap);
  ಆದರೆ( g.pVfs==0 ){
    ಹಿಂತಿರುಗಿ SQLITE_ERROR;
  }
  jt_vfs.szOsFile = ಗಾತ್ರ(jt_file) + g.pVfs->szOsFile;
  ಆದರೆ( g.pVfs->i_ಆವೃತ್ತಿ==1 ){
    jt_vfs.i_ಆವೃತ್ತಿ = 1;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( g.pVfs->x_Current_ಸಮಯ_Int64==0 ){
    jt_vfs.x_Current_ಸಮಯ_Int64 = 0;
  }
  sqlite3_vfs_ರಿಜಿಸ್ಟರ್(&jt_vfs, isDefault);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Uninstall the jt VFS, if it is installed.
*/
ಶೂನ್ಯ jt_unregister(ಶೂನ್ಯ){
  sqlite3_vfs_unregister(&jt_vfs);
}

#ಆದರೆ_ಅಂತ್ಯ
