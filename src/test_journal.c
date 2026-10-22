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
#ఒకవేళ SQLITE_TEST          /* This file is used for testing only */

#అన్తర్భావ "sqlite3.h"
#అన్తర్భావ "sqliteInt.h"

/*
** Maximum pathname length supported by the jt backend.
*/
#నిర్వచించు JT_MAX_PATHNAME 512

/*
** Name used to identify this VFS.
*/
#నిర్వచించు JT_VFS_NAME "jt"

రకనిర్వచనం నిర్మాణం jt_file jt_file;
నిర్మాణం jt_file {
  sqlite3_file పునాది;
  స్థిరరాశి అక్షరం *zName;       /* Name of open file */
  పూర్ణాంకం flags;               /* Flags the file was opened with */

  /* The following are only used by database file file handles */
  పూర్ణాంకం e_తాళం;               /* Current lock held on the file */
  u32 n_పుట;               /* Size of file in pages when transaction started */
  u32 nPagesize;           /* Page size when transaction started */
  Bitvec *pWritable;       /* Bitvec of pages that may be written to the file */
  u32 *aCksum;             /* Checksum for first nPage pages */
  పూర్ణాంకం nSync;               /* Number of times journal file has been synced */

  /* Only used by journal file-handles */
  sqlite3_int64 iMaxOff;   /* Maximum offset written to this transaction */

  jt_file *p_తదుపరి;          /* All files are stored in a linked list */
  sqlite3_file *pReal;     /* The file handle for the underlying vfs */
};

/*
** Method declarations for jt_file.
*/
స్థిర పూర్ణాంకం jtClose(sqlite3_file*);
స్థిర పూర్ణాంకం jtRead(sqlite3_file*, శూన్య*, పూర్ణాంకం iAmt, sqlite3_int64 iOfst);
స్థిర పూర్ణాంకం jtWrite(sqlite3_file*,స్థిరరాశి శూన్య*,పూర్ణాంకం iAmt, sqlite3_int64 iOfst);
స్థిర పూర్ణాంకం jtTruncate(sqlite3_file*, sqlite3_int64 కొలత);
స్థిర పూర్ణాంకం jtSync(sqlite3_file*, పూర్ణాంకం flags);
స్థిర పూర్ణాంకం jt_File_కొలత(sqlite3_file*, sqlite3_int64 *p_కొలత);
స్థిర పూర్ణాంకం jt_తాళం(sqlite3_file*, పూర్ణాంకం);
స్థిర పూర్ణాంకం jtUnlock(sqlite3_file*, పూర్ణాంకం);
స్థిర పూర్ణాంకం jt_Check_Reserved_తాళం(sqlite3_file*, పూర్ణాంకం *);
స్థిర పూర్ణాంకం jtFileControl(sqlite3_file*, పూర్ణాంకం op, శూన్య *pArg);
స్థిర పూర్ణాంకం jt_Sector_కొలత(sqlite3_file*);
స్థిర పూర్ణాంకం jtDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for jt_vfs.
*/
స్థిర పూర్ణాంకం jtOpen(sqlite3_vfs*, స్థిరరాశి అక్షరం *, sqlite3_file*, పూర్ణాంకం , పూర్ణాంకం *);
స్థిర పూర్ణాంకం jtDelete(sqlite3_vfs*, స్థిరరాశి అక్షరం *zName, పూర్ణాంకం syncDir);
స్థిర పూర్ణాంకం jtAccess(sqlite3_vfs*, స్థిరరాశి అక్షరం *zName, పూర్ణాంకం flags, పూర్ణాంకం *);
స్థిర పూర్ణాంకం jt_నిండిన_Pathname(sqlite3_vfs*, స్థిరరాశి అక్షరం *zName, పూర్ణాంకం, అక్షరం *z_బహిర్గతం);
స్థిర శూన్య *jtDlOpen(sqlite3_vfs*, స్థిరరాశి అక్షరం *zFilename);
స్థిర శూన్య jtDlError(sqlite3_vfs*, పూర్ణాంకం nByte, అక్షరం *z_Err_సందేశం);
స్థిర శూన్య (*jtDlSym(sqlite3_vfs*,శూన్య*, స్థిరరాశి అక్షరం *z_చిహ్నం))(శూన్య);
స్థిర శూన్య jtDlClose(sqlite3_vfs*, శూన్య*);
స్థిర పూర్ణాంకం jtRandomness(sqlite3_vfs*, పూర్ణాంకం nByte, అక్షరం *z_బహిర్గతం);
స్థిర పూర్ణాంకం jtSleep(sqlite3_vfs*, పూర్ణాంకం microseconds);
స్థిర పూర్ణాంకం jt_Current_సమయం(sqlite3_vfs*, ద్విగుణ*);
స్థిర పూర్ణాంకం jt_Current_సమయం_Int64(sqlite3_vfs*, sqlite3_int64*);
స్థిర పూర్ణాంకం jt_Get_చివరి_Error(sqlite3_vfs*, పూర్ణాంకం, అక్షరం*);

స్థిర sqlite3_vfs jt_vfs = {
  2,                             /* iVersion */
  పరిమాణం(jt_file),               /* szOsFile */
  JT_MAX_PATHNAME,               /* mxPathname */
  0,                             /* pNext */
  JT_VFS_NAME,                   /* zName */
  0,                             /* pAppData */
  jtOpen,                        /* xOpen */
  jtDelete,                      /* xDelete */
  jtAccess,                      /* xAccess */
  jt_నిండిన_Pathname,                /* xFullPathname */
  jtDlOpen,                      /* xDlOpen */
  jtDlError,                     /* xDlError */
  jtDlSym,                       /* xDlSym */
  jtDlClose,                     /* xDlClose */
  jtRandomness,                  /* xRandomness */
  jtSleep,                       /* xSleep */
  jt_Current_సమయం,                 /* xCurrentTime */
  jt_Get_చివరి_Error,                /* xGetLastError */
  jt_Current_సమయం_Int64             /* xCurrentTimeInt64 */
};

స్థిర sqlite3_io_methods jt_io_methods = {
  1,                             /* iVersion */
  jtClose,                       /* xClose */
  jtRead,                        /* xRead */
  jtWrite,                       /* xWrite */
  jtTruncate,                    /* xTruncate */
  jtSync,                        /* xSync */
  jt_File_కొలత,                    /* xFileSize */
  jt_తాళం,                        /* xLock */
  jtUnlock,                      /* xUnlock */
  jt_Check_Reserved_తాళం,           /* xCheckReservedLock */
  jtFileControl,                 /* xFileControl */
  jt_Sector_కొలత,                  /* xSectorSize */
  jtDeviceCharacteristics        /* xDeviceCharacteristics */
};

నిర్మాణం JtGlobal {
  sqlite3_vfs *pVfs;             /* Parent VFS */
  jt_file *pList;                /* List of all open files */
};
స్థిర నిర్మాణం JtGlobal g = {0, 0};

/*
** Functions to obtain and relinquish a mutex to protect g.pList. The
** STATIC_PRNG mutex is reused, purely for the sake of convenience.
*/
స్థిర శూన్య enterJtMutex(శూన్య){
  sqlite3_mutex_enter(sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_PRNG));
}
స్థిర శూన్య leaveJtMutex(శూన్య){
  sqlite3_mutex_leave(sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_PRNG));
}

బాహ్య పూర్ణాంకం sqlite3_io_error_pending;
బాహ్య పూర్ణాంకం sqlite3_io_error_తగిలింది;
స్థిర శూన్య stop_ioerr_simulation(పూర్ణాంకం *piSave, పూర్ణాంకం *piSave2){
  *piSave = sqlite3_io_error_pending;
  *piSave2 = sqlite3_io_error_తగిలింది;
  sqlite3_io_error_pending = -1;
  sqlite3_io_error_తగిలింది = 0;
}
స్థిర శూన్య ప్రారంభం_ioerr_simulation(పూర్ణాంకం iSave, పూర్ణాంకం iSave2){
  sqlite3_io_error_pending = iSave;
  sqlite3_io_error_తగిలింది = iSave2;
}

/*
** The jt_file pointed to by the argument may or may not be a file-handle
** open on a main database file. If it is, and a transaction is currently
** opened on the file, then discard all transaction related data.
*/
స్థిర శూన్య closeTransaction(jt_file *p){
  sqlite3BitvecDestroy(p->pWritable);
  sqlite3_free(p->aCksum);
  p->pWritable = 0;
  p->aCksum = 0;
  p->nSync = 0;
}

/*
** Close an jt-file.
*/
స్థిర పూర్ణాంకం jtClose(sqlite3_file *pFile){
  jt_file **pp;
  jt_file *p = (jt_file *)pFile;

  closeTransaction(p);
  enterJtMutex();
  ఒకవేళ( p->zName ){
    కొరకు(pp=&g.pList; *pp!=p; pp=&(*pp)->p_తదుపరి);
    *pp = p->p_తదుపరి;
  }
  leaveJtMutex();
  sqlite3OsClose(p->pReal);
  తిరిగి SQLITE_OK;
}

/*
** Read data from an jt-file.
*/
స్థిర పూర్ణాంకం jtRead(
  sqlite3_file *pFile, 
  శూన్య *zBuf, 
  పూర్ణాంకం iAmt, 
  sqlite_int64 iOfst
){
  jt_file *p = (jt_file *)pFile;
  తిరిగి sqlite3OsRead(p->pReal, zBuf, iAmt, iOfst);
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
స్థిర jt_file *locateDatabaseHandle(స్థిరరాశి అక్షరం *zJournal, పూర్ణాంకం no_తాళం){
  jt_file *pMain = 0;
  enterJtMutex();
  కొరకు(pMain=g.pList; pMain; pMain=pMain->p_తదుపరి){
    పూర్ణాంకం nName = (పూర్ణాంకం)(అక్షరశ్రేణి_పొడవు(zJournal) - అక్షరశ్రేణి_పొడవు("-journal"));
    ఒకవేళ( (pMain->flags&SQLITE_OPEN_MAIN_DB)
     && ((పూర్ణాంకం)అక్షరశ్రేణి_పొడవు(pMain->zName)==nName)
     && 0==memcmp(pMain->zName, zJournal, nName)
     && ((pMain->e_తాళం>=SQLITE_తాళం_RESERVED) || no_తాళం)
    ){
      ఆపు;
    }
  }
  leaveJtMutex();
  తిరిగి pMain;
}

/*
** Parameter z points to a buffer of 4 bytes in size containing a 
** unsigned 32-bit integer stored in big-endian format. Decode the 
** integer and return its value.
*/
స్థిర u32 decodeUint32(స్థిరరాశి గుర్తులేని అక్షరం *z){
  తిరిగి (z[0]<<24) + (z[1]<<16) + (z[2]<<8) + z[3];
}

/*
** Calculate a checksum from the buffer of length n bytes pointed to
** by parameter z.
*/
స్థిర u32 genCksum(స్థిరరాశి గుర్తులేని అక్షరం *z, పూర్ణాంకం n){
  పూర్ణాంకం i;
  u32 cksum = 0;
  కొరకు(i=0; i<n; i++){
    cksum = cksum + z[i] + (cksum<<3);
  }
  తిరిగి cksum;
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
స్థిర పూర్ణాంకం decodeJournalHdr(
  స్థిరరాశి గుర్తులేని అక్షరం *zBuf,         /* Input: 28 byte journal header */
  u32 *pnRec,                        /* Out: Number of journalled records */
  u32 *pn_పుట,                       /* Out: Original database page count */
  u32 *pnSector,                     /* Out: Sector size in bytes */
  u32 *pnPagesize                    /* Out: Page size in bytes */
){
  గుర్తులేని అక్షరం aMagic[] = { 0xd9, 0xd5, 0x05, 0xf9, 0x20, 0xa1, 0x63, 0xd7 };
  ఒకవేళ( memcmp(aMagic, zBuf, 8) ) తిరిగి SQLITE_ERROR;
  ఒకవేళ( pnRec ) *pnRec = decodeUint32(&zBuf[8]);
  ఒకవేళ( pn_పుట ) *pn_పుట = decodeUint32(&zBuf[16]);
  ఒకవేళ( pnSector ) *pnSector = decodeUint32(&zBuf[20]);
  ఒకవేళ( pnPagesize ) *pnPagesize = decodeUint32(&zBuf[24]);
  తిరిగి SQLITE_OK;
}

/*
** This function is called when a new transaction is opened, just after
** the first journal-header is written to the journal file.
*/
స్థిర పూర్ణాంకం openTransaction(jt_file *pMain, jt_file *pJournal){
  గుర్తులేని అక్షరం *a_దత్తాంశం;
  sqlite3_file *p = pMain->pReal;
  పూర్ణాంకం rc = SQLITE_OK;

  closeTransaction(pMain);
  a_దత్తాంశం = sqlite3_malloc(pMain->nPagesize);
  pMain->pWritable = sqlite3BitvecCreate(pMain->n_పుట);
  pMain->aCksum = sqlite3_malloc(పరిమాణం(u32) * (pMain->n_పుట + 1));
  pJournal->iMaxOff = 0;

  ఒకవేళ( !pMain->pWritable || !pMain->aCksum || !a_దత్తాంశం ){
    rc = SQLITE_IOERR_NOMEM;
  }లేకపోతే ఒకవేళ( pMain->n_పుట>0 ){
    u32 iTrunk;
    పూర్ణాంకం iSave;
    పూర్ణాంకం iSave2;

    stop_ioerr_simulation(&iSave, &iSave2);

    /* Read the database free-list. Add the page-number for each free-list
    ** leaf to the jt_file.pWritable bitvec.
    */
    rc = sqlite3OsRead(p, a_దత్తాంశం, pMain->nPagesize, 0);
    ఒకవేళ( rc==SQLITE_OK ){
      u32 nDbsize = decodeUint32(&a_దత్తాంశం[28]);
      ఒకవేళ( nDbsize>0 && memcmp(&a_దత్తాంశం[24], &a_దత్తాంశం[92], 4)==0 ){
        u32 iPg;
        కొరకు(iPg=nDbsize+1; iPg<=pMain->n_పుట; iPg++){
          sqlite3_Bitvec_సమితి(pMain->pWritable, iPg);
        }
      }
    }
    iTrunk = decodeUint32(&a_దత్తాంశం[32]);
    అయితే( rc==SQLITE_OK && iTrunk>0 ){
      u32 n_ఆకు;
      u32 i_ఆకు;
      sqlite3_int64 iOff = (i64)(iTrunk-1)*pMain->nPagesize;
      rc = sqlite3OsRead(p, a_దత్తాంశం, pMain->nPagesize, iOff);
      n_ఆకు = decodeUint32(&a_దత్తాంశం[4]);
      కొరకు(i_ఆకు=0; rc==SQLITE_OK && i_ఆకు<n_ఆకు; i_ఆకు++){
        u32 pgno = decodeUint32(&a_దత్తాంశం[8+4*i_ఆకు]);
        sqlite3_Bitvec_సమితి(pMain->pWritable, pgno);
      }
      iTrunk = decodeUint32(a_దత్తాంశం);
    }

    /* Calculate and store a checksum for each page in the database file. */
    ఒకవేళ( rc==SQLITE_OK ){
      పూర్ణాంకం ii;
      కొరకు(ii=0; rc==SQLITE_OK && ii<(పూర్ణాంకం)pMain->n_పుట; ii++){
        i64 iOff = (i64)(pMain->nPagesize) * (i64)ii;
        ఒకవేళ( iOff==PENDING_BYTE ) కొనసాగు;
        rc = sqlite3OsRead(pMain->pReal, a_దత్తాంశం, pMain->nPagesize, iOff);
        pMain->aCksum[ii] = genCksum(a_దత్తాంశం, pMain->nPagesize);
        ఒకవేళ( ii+1==(పూర్ణాంకం)pMain->n_పుట && rc==SQLITE_IOERR_SHORT_READ ){
          rc = SQLITE_OK;
        }
      }
    }

    ప్రారంభం_ioerr_simulation(iSave, iSave2);
  }

  sqlite3_free(a_దత్తాంశం);
  తిరిగి rc;
}

/*
** The first argument to this function is a handle open on a journal file.
** This function reads the journal file and adds the page number for each
** page in the journal to the Bitvec object passed as the second argument.
*/
స్థిర పూర్ణాంకం readJournalFile(jt_file *p, jt_file *pMain){
  పూర్ణాంకం rc = SQLITE_OK;
  గుర్తులేని అక్షరం zBuf[28];
  sqlite3_file *pReal = p->pReal;
  sqlite3_int64 iOff = 0;
  sqlite3_int64 i_కొలత = p->iMaxOff;
  గుర్తులేని అక్షరం *a_పుట;
  పూర్ణాంకం iSave;
  పూర్ణాంకం iSave2;

  a_పుట = sqlite3_malloc(pMain->nPagesize);
  ఒకవేళ( !a_పుట ){
    తిరిగి SQLITE_IOERR_NOMEM;
  }

  stop_ioerr_simulation(&iSave, &iSave2);

  అయితే( rc==SQLITE_OK && iOff<i_కొలత ){
    u32 nRec, n_పుట, nSector, nPagesize;
    u32 ii;

    /* Read and decode the next journal-header from the journal file. */
    rc = sqlite3OsRead(pReal, zBuf, 28, iOff);
    ఒకవేళ( rc!=SQLITE_OK 
     || decodeJournalHdr(zBuf, &nRec, &n_పుట, &nSector, &nPagesize) 
    ){
      వెళ్ళు ముగింపు_rjf;
    }
    iOff += nSector;

    ఒకవేళ( nRec==0 ){
      /* A trick. There might be another journal-header immediately 
      ** following this one. In this case, 0 records means 0 records, 
      ** not "read until the end of the file". See also ticket #2565.
      */
      ఒకవేళ( i_కొలత>=(iOff+nSector) ){
        rc = sqlite3OsRead(pReal, zBuf, 28, iOff);
        ఒకవేళ( rc!=SQLITE_OK || 0==decodeJournalHdr(zBuf, 0, 0, 0, 0) ){
          కొనసాగు;
        }
      }
      nRec = (u32)((i_కొలత-iOff) / (pMain->nPagesize+8));
    }

    /* Read all the records that follow the journal-header just read. */
    కొరకు(ii=0; rc==SQLITE_OK && ii<nRec && iOff<i_కొలత; ii++){
      u32 pgno;
      rc = sqlite3OsRead(pReal, zBuf, 4, iOff);
      ఒకవేళ( rc==SQLITE_OK ){
        pgno = decodeUint32(zBuf);
        ఒకవేళ( pgno>0 && pgno<=pMain->n_పుట ){
          ఒకవేళ( 0==sqlite3BitvecTest(pMain->pWritable, pgno) ){
            rc = sqlite3OsRead(pReal, a_పుట, pMain->nPagesize, iOff+4);
            ఒకవేళ( rc==SQLITE_OK ){
              u32 cksum = genCksum(a_పుట, pMain->nPagesize);
              ప్రతిజ్ఞా( cksum==pMain->aCksum[pgno-1] );
            }
          }
          sqlite3_Bitvec_సమితి(pMain->pWritable, pgno);
        }
        iOff += (8 + pMain->nPagesize);
      }
    }

    iOff = ((iOff + (nSector-1)) / nSector) * nSector;
  }

ముగింపు_rjf:
  ప్రారంభం_ioerr_simulation(iSave, iSave2);
  sqlite3_free(a_పుట);
  ఒకవేళ( rc==SQLITE_IOERR_SHORT_READ ){
    rc = SQLITE_OK;
  }
  తిరిగి rc;
}

/*
** Write data to an jt-file.
*/
స్థిర పూర్ణాంకం jtWrite(
  sqlite3_file *pFile, 
  స్థిరరాశి శూన్య *zBuf, 
  పూర్ణాంకం iAmt, 
  sqlite_int64 iOfst
){
  పూర్ణాంకం rc;
  jt_file *p = (jt_file *)pFile;
  ఒకవేళ( p->flags&SQLITE_OPEN_MAIN_JOURNAL ){
    ఒకవేళ( iOfst==0 ){
      jt_file *pMain = locateDatabaseHandle(p->zName, 0);
      ప్రతిజ్ఞా( pMain );
  
      ఒకవేళ( iAmt==28 ){
        /* Zeroing the first journal-file header. This is the end of a
        ** transaction. */
        closeTransaction(pMain);
      }లేకపోతే ఒకవేళ( iAmt!=12 ){
        /* Writing the first journal header to a journal file. This happens
        ** when a transaction is first started.  */
        u8 *z = (u8 *)zBuf;
        pMain->n_పుట = decodeUint32(&z[16]);
        pMain->nPagesize = decodeUint32(&z[24]);
        ఒకవేళ( SQLITE_OK!=(rc=openTransaction(pMain, p)) ){
          తిరిగి rc;
        }
      }
    }
    ఒకవేళ( p->iMaxOff<(iOfst + iAmt) ){
      p->iMaxOff = iOfst + iAmt;
    }
  }

  ఒకవేళ( p->flags&SQLITE_OPEN_MAIN_DB && p->pWritable ){
    ఒకవేళ( iAmt<(పూర్ణాంకం)p->nPagesize 
     && p->nPagesize%iAmt==0 
     && iOfst>=(PENDING_BYTE+512) 
     && iOfst+iAmt<=PENDING_BYTE+p->nPagesize
    ){
      /* No-op. This special case is hit when the backup code is copying a
      ** to a database with a larger page-size than the source database and
      ** it needs to fill in the non-locking-region part of the original
      ** pending-byte page.
      */
    }లేకపోతే{
      u32 pgno = (u32)(iOfst/p->nPagesize + 1);
      ప్రతిజ్ఞా( (iAmt==1||iAmt==(పూర్ణాంకం)p->nPagesize) &&
              ((iOfst+iAmt)%p->nPagesize)==0 );
      /* The following assert() statements may fail if this layer is used
      ** with a connection in "PRAGMA synchronous=off" mode. If they
      ** fail with sync=normal or sync=full, this may indicate problem.  */
      ప్రతిజ్ఞా( p->n_పుట==0 || pgno<=p->n_పుట || p->nSync>0 );
      ప్రతిజ్ఞా( pgno>p->n_పుట || sqlite3BitvecTest(p->pWritable, pgno) );
    }
  }

  rc = sqlite3OsWrite(p->pReal, zBuf, iAmt, iOfst);
  ఒకవేళ( (p->flags&SQLITE_OPEN_MAIN_JOURNAL) && iAmt==12 ){
    jt_file *pMain = locateDatabaseHandle(p->zName, 0);
    పూర్ణాంకం rc2 = readJournalFile(p, pMain);
    ఒకవేళ( rc==SQLITE_OK ) rc = rc2;
  }
  తిరిగి rc;
}

/*
** Truncate an jt-file.
*/
స్థిర పూర్ణాంకం jtTruncate(sqlite3_file *pFile, sqlite_int64 కొలత){
  jt_file *p = (jt_file *)pFile;
  ఒకవేళ( p->flags&SQLITE_OPEN_MAIN_JOURNAL && కొలత==0 ){
    /* Truncating a journal file. This is the end of a transaction. */
    jt_file *pMain = locateDatabaseHandle(p->zName, 0);
    closeTransaction(pMain);
  }
  ఒకవేళ( p->flags&SQLITE_OPEN_MAIN_DB && p->pWritable ){
    u32 pgno;
    u32 locking_పుట = (u32)(PENDING_BYTE/p->nPagesize+1);
    కొరకు(pgno=(u32)(కొలత/p->nPagesize+1); pgno<=p->n_పుట; pgno++){
      ప్రతిజ్ఞా( pgno==locking_పుట || sqlite3BitvecTest(p->pWritable, pgno) );
    }
  }
  తిరిగి sqlite3OsTruncate(p->pReal, కొలత);
}

/*
** Sync an jt-file.
*/
స్థిర పూర్ణాంకం jtSync(sqlite3_file *pFile, పూర్ణాంకం flags){
  jt_file *p = (jt_file *)pFile;

  ఒకవేళ( p->flags&SQLITE_OPEN_MAIN_JOURNAL ){
    పూర్ణాంకం rc;
    jt_file *pMain;                   /* The associated database file */

    /* The journal file is being synced. At this point, we inspect the 
    ** contents of the file up to this point and set each bit in the 
    ** jt_file.pWritable bitvec of the main database file associated with
    ** this journal file.
    */
    pMain = locateDatabaseHandle(p->zName, 0);

    /* Set the bitvec values */
    ఒకవేళ( pMain && pMain->pWritable ){
      pMain->nSync++;
      rc = readJournalFile(p, pMain);
      ఒకవేళ( rc!=SQLITE_OK ){
        తిరిగి rc;
      }
    }
  }

  తిరిగి sqlite3OsSync(p->pReal, flags);
}

/*
** Return the current file-size of an jt-file.
*/
స్థిర పూర్ణాంకం jt_File_కొలత(sqlite3_file *pFile, sqlite_int64 *p_కొలత){
  jt_file *p = (jt_file *)pFile;
  తిరిగి sqlite3_Os_File_కొలత(p->pReal, p_కొలత);
}

/*
** Lock an jt-file.
*/
స్థిర పూర్ణాంకం jt_తాళం(sqlite3_file *pFile, పూర్ణాంకం e_తాళం){
  పూర్ణాంకం rc;
  jt_file *p = (jt_file *)pFile;
  rc = sqlite3_Os_తాళం(p->pReal, e_తాళం);
  ఒకవేళ( rc==SQLITE_OK && e_తాళం>p->e_తాళం ){
    p->e_తాళం = e_తాళం;
  }
  తిరిగి rc;
}

/*
** Unlock an jt-file.
*/
స్థిర పూర్ణాంకం jtUnlock(sqlite3_file *pFile, పూర్ణాంకం e_తాళం){
  పూర్ణాంకం rc;
  jt_file *p = (jt_file *)pFile;
  rc = sqlite3OsUnlock(p->pReal, e_తాళం);
  ఒకవేళ( rc==SQLITE_OK && e_తాళం<p->e_తాళం ){
    p->e_తాళం = e_తాళం;
  }
  తిరిగి rc;
}

/*
** Check if another file-handle holds a RESERVED lock on an jt-file.
*/
స్థిర పూర్ణాంకం jt_Check_Reserved_తాళం(sqlite3_file *pFile, పూర్ణాంకం *p_Res_బహిర్గతం){
  jt_file *p = (jt_file *)pFile;
  తిరిగి sqlite3_Os_Check_Reserved_తాళం(p->pReal, p_Res_బహిర్గతం);
}

/*
** File control method. For custom operations on an jt-file.
*/
స్థిర పూర్ణాంకం jtFileControl(sqlite3_file *pFile, పూర్ణాంకం op, శూన్య *pArg){
  jt_file *p = (jt_file *)pFile;
  తిరిగి p->pReal->pMethods->xFileControl(p->pReal, op, pArg);
}

/*
** Return the sector-size in bytes for an jt-file.
*/
స్థిర పూర్ణాంకం jt_Sector_కొలత(sqlite3_file *pFile){
  jt_file *p = (jt_file *)pFile;
  తిరిగి sqlite3_Os_Sector_కొలత(p->pReal);
}

/*
** Return the device characteristic flags supported by an jt-file.
*/
స్థిర పూర్ణాంకం jtDeviceCharacteristics(sqlite3_file *pFile){
  jt_file *p = (jt_file *)pFile;
  తిరిగి sqlite3OsDeviceCharacteristics(p->pReal);
}

/*
** Open an jt file handle.
*/
స్థిర పూర్ణాంకం jtOpen(
  sqlite3_vfs *pVfs,
  స్థిరరాశి అక్షరం *zName,
  sqlite3_file *pFile,
  పూర్ణాంకం flags,
  పూర్ణాంకం *p_బహిర్గతం_Flags
){
  పూర్ణాంకం rc;
  jt_file *p = (jt_file *)pFile;
  pFile->pMethods = 0;
  p->pReal = (sqlite3_file *)&p[1];
  p->pReal->pMethods = 0;
  rc = sqlite3OsOpen(g.pVfs, zName, p->pReal, flags, p_బహిర్గతం_Flags);
  ప్రతిజ్ఞా( rc==SQLITE_OK || p->pReal->pMethods==0 );
  ఒకవేళ( rc==SQLITE_OK ){
    pFile->pMethods = &jt_io_methods;
    p->e_తాళం = 0;
    p->zName = zName;
    p->flags = flags;
    p->p_తదుపరి = 0;
    p->pWritable = 0;
    p->aCksum = 0;
    enterJtMutex();
    ఒకవేళ( zName ){
      p->p_తదుపరి = g.pList;
      g.pList = p;
    }
    leaveJtMutex();
  }
  తిరిగి rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
స్థిర పూర్ణాంకం jtDelete(sqlite3_vfs *pVfs, స్థిరరాశి అక్షరం *z_మార్గం, పూర్ణాంకం dirSync){
  పూర్ణాంకం n_మార్గం = (పూర్ణాంకం)అక్షరశ్రేణి_పొడవు(z_మార్గం);
  ఒకవేళ( n_మార్గం>8 && 0==strcmp("-journal", &z_మార్గం[n_మార్గం-8]) ){
    /* Deleting a journal file. The end of a transaction. */
    jt_file *pMain = locateDatabaseHandle(z_మార్గం, 0);
    ఒకవేళ( pMain ){
      closeTransaction(pMain);
    }
  }

  తిరిగి sqlite3OsDelete(g.pVfs, z_మార్గం, dirSync);
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
స్థిర పూర్ణాంకం jtAccess(
  sqlite3_vfs *pVfs, 
  స్థిరరాశి అక్షరం *z_మార్గం, 
  పూర్ణాంకం flags, 
  పూర్ణాంకం *p_Res_బహిర్గతం
){
  తిరిగి sqlite3OsAccess(g.pVfs, z_మార్గం, flags, p_Res_బహిర్గతం);
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (JT_MAX_PATHNAME+1) bytes.
*/
స్థిర పూర్ణాంకం jt_నిండిన_Pathname(
  sqlite3_vfs *pVfs, 
  స్థిరరాశి అక్షరం *z_మార్గం, 
  పూర్ణాంకం n_బహిర్గతం, 
  అక్షరం *z_బహిర్గతం
){
  తిరిగి sqlite3_Os_నిండిన_Pathname(g.pVfs, z_మార్గం, n_బహిర్గతం, z_బహిర్గతం);
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
స్థిర శూన్య *jtDlOpen(sqlite3_vfs *pVfs, స్థిరరాశి అక్షరం *z_మార్గం){
  తిరిగి g.pVfs->xDlOpen(g.pVfs, z_మార్గం);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
స్థిర శూన్య jtDlError(sqlite3_vfs *pVfs, పూర్ణాంకం nByte, అక్షరం *z_Err_సందేశం){
  g.pVfs->xDlError(g.pVfs, nByte, z_Err_సందేశం);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
స్థిర శూన్య (*jtDlSym(sqlite3_vfs *pVfs, శూన్య *p, స్థిరరాశి అక్షరం *zSym))(శూన్య){
  తిరిగి g.pVfs->xDlSym(g.pVfs, p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
స్థిర శూన్య jtDlClose(sqlite3_vfs *pVfs, శూన్య *pHandle){
  g.pVfs->xDlClose(g.pVfs, pHandle);
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
స్థిర పూర్ణాంకం jtRandomness(sqlite3_vfs *pVfs, పూర్ణాంకం nByte, అక్షరం *z_Buf_బహిర్గతం){
  తిరిగి sqlite3OsRandomness(g.pVfs, nByte, z_Buf_బహిర్గతం);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
స్థిర పూర్ణాంకం jtSleep(sqlite3_vfs *pVfs, పూర్ణాంకం nMicro){
  తిరిగి sqlite3OsSleep(g.pVfs, nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
స్థిర పూర్ణాంకం jt_Current_సమయం(sqlite3_vfs *pVfs, ద్విగుణ *p_సమయం_బహిర్గతం){
  తిరిగి g.pVfs->x_Current_సమయం(g.pVfs, p_సమయం_బహిర్గతం);
}
/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
స్థిర పూర్ణాంకం jt_Current_సమయం_Int64(sqlite3_vfs *pVfs, sqlite3_int64 *p_సమయం_బహిర్గతం){
  తిరిగి g.pVfs->x_Current_సమయం_Int64(g.pVfs, p_సమయం_బహిర్గతం);
}

స్థిర పూర్ణాంకం jt_Get_చివరి_Error(sqlite3_vfs *pVfs, పూర్ణాంకం n, అక్షరం *z){
  తిరిగి g.pVfs->x_Get_చివరి_Error(g.pVfs, n, z);
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
పూర్ణాంకం jt_రిజిస్టరు(అక్షరం *zWrap, పూర్ణాంకం isDefault){
  g.pVfs = sqlite3_vfs_కనుగొను(zWrap);
  ఒకవేళ( g.pVfs==0 ){
    తిరిగి SQLITE_ERROR;
  }
  jt_vfs.szOsFile = పరిమాణం(jt_file) + g.pVfs->szOsFile;
  ఒకవేళ( g.pVfs->i_సంస్కరణ==1 ){
    jt_vfs.i_సంస్కరణ = 1;
  }లేకపోతే ఒకవేళ( g.pVfs->x_Current_సమయం_Int64==0 ){
    jt_vfs.x_Current_సమయం_Int64 = 0;
  }
  sqlite3_vfs_రిజిస్టరు(&jt_vfs, isDefault);
  తిరిగి SQLITE_OK;
}

/*
** Uninstall the jt VFS, if it is installed.
*/
శూన్య jt_unregister(శూన్య){
  sqlite3_vfs_unregister(&jt_vfs);
}

#అయితే_ముగింపు
