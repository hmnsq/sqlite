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
#अगर SQLITE_TEST          /* This file is used for testing only */

#समावेश "sqlite3.h"
#समावेश "sqliteInt.h"

/*
** Maximum pathname length supported by the jt backend.
*/
#घोषणा JT_MAX_PATHNAME 512

/*
** Name used to identify this VFS.
*/
#घोषणा JT_VFS_NAME "jt"

प्रकार काष्ठा jt_file jt_file;
काष्ठा jt_file {
  sqlite3_file base;
  स्थिर अक्षर *zName;       /* Name of open file */
  पूर्णांक flags;               /* Flags the file was opened with */

  /* The following are only used by database file file handles */
  पूर्णांक eLock;               /* Current lock held on the file */
  u32 nPage;               /* Size of file in pages when transaction started */
  u32 nPagesize;           /* Page size when transaction started */
  Bitvec *pWritable;       /* Bitvec of pages that may be written to the file */
  u32 *aCksum;             /* Checksum for first nPage pages */
  पूर्णांक nSync;               /* Number of times journal file has been synced */

  /* Only used by journal file-handles */
  sqlite3_int64 iMaxOff;   /* Maximum offset written to this transaction */

  jt_file *p_अग्र;          /* All files are stored in a linked list */
  sqlite3_file *pReal;     /* The file handle for the underlying vfs */
};

/*
** Method declarations for jt_file.
*/
अटल पूर्णांक jtClose(sqlite3_file*);
अटल पूर्णांक jtRead(sqlite3_file*, व्योम*, पूर्णांक iAmt, sqlite3_int64 iOfst);
अटल पूर्णांक jtWrite(sqlite3_file*,स्थिर व्योम*,पूर्णांक iAmt, sqlite3_int64 iOfst);
अटल पूर्णांक jtTruncate(sqlite3_file*, sqlite3_int64 आकार);
अटल पूर्णांक jtSync(sqlite3_file*, पूर्णांक flags);
अटल पूर्णांक jt_File_आकार(sqlite3_file*, sqlite3_int64 *p_आकार);
अटल पूर्णांक jtLock(sqlite3_file*, पूर्णांक);
अटल पूर्णांक jtUnlock(sqlite3_file*, पूर्णांक);
अटल पूर्णांक jtCheckReservedLock(sqlite3_file*, पूर्णांक *);
अटल पूर्णांक jtFileControl(sqlite3_file*, पूर्णांक op, व्योम *pArg);
अटल पूर्णांक jt_Sector_आकार(sqlite3_file*);
अटल पूर्णांक jtDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for jt_vfs.
*/
अटल पूर्णांक jtOpen(sqlite3_vfs*, स्थिर अक्षर *, sqlite3_file*, पूर्णांक , पूर्णांक *);
अटल पूर्णांक jtDelete(sqlite3_vfs*, स्थिर अक्षर *zName, पूर्णांक syncDir);
अटल पूर्णांक jtAccess(sqlite3_vfs*, स्थिर अक्षर *zName, पूर्णांक flags, पूर्णांक *);
अटल पूर्णांक jtFullPathname(sqlite3_vfs*, स्थिर अक्षर *zName, पूर्णांक, अक्षर *zOut);
अटल व्योम *jtDlOpen(sqlite3_vfs*, स्थिर अक्षर *zFilename);
अटल व्योम jtDlError(sqlite3_vfs*, पूर्णांक nByte, अक्षर *z_Err_संदेश);
अटल व्योम (*jtDlSym(sqlite3_vfs*,व्योम*, स्थिर अक्षर *z_प्रतीक))(व्योम);
अटल व्योम jtDlClose(sqlite3_vfs*, व्योम*);
अटल पूर्णांक jtRandomness(sqlite3_vfs*, पूर्णांक nByte, अक्षर *zOut);
अटल पूर्णांक jtSleep(sqlite3_vfs*, पूर्णांक microseconds);
अटल पूर्णांक jtCurrentTime(sqlite3_vfs*, द्विगुन*);
अटल पूर्णांक jtCurrentTimeInt64(sqlite3_vfs*, sqlite3_int64*);
अटल पूर्णांक jtGetLastError(sqlite3_vfs*, पूर्णांक, अक्षर*);

अटल sqlite3_vfs jt_vfs = {
  2,                             /* iVersion */
  माप(jt_file),               /* szOsFile */
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

अटल sqlite3_io_methods jt_io_methods = {
  1,                             /* iVersion */
  jtClose,                       /* xClose */
  jtRead,                        /* xRead */
  jtWrite,                       /* xWrite */
  jtTruncate,                    /* xTruncate */
  jtSync,                        /* xSync */
  jt_File_आकार,                    /* xFileSize */
  jtLock,                        /* xLock */
  jtUnlock,                      /* xUnlock */
  jtCheckReservedLock,           /* xCheckReservedLock */
  jtFileControl,                 /* xFileControl */
  jt_Sector_आकार,                  /* xSectorSize */
  jtDeviceCharacteristics        /* xDeviceCharacteristics */
};

काष्ठा JtGlobal {
  sqlite3_vfs *pVfs;             /* Parent VFS */
  jt_file *pList;                /* List of all open files */
};
अटल काष्ठा JtGlobal g = {0, 0};

/*
** Functions to obtain and relinquish a mutex to protect g.pList. The
** STATIC_PRNG mutex is reused, purely for the sake of convenience.
*/
अटल व्योम enterJtMutex(व्योम){
  sqlite3_mutex_enter(sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_PRNG));
}
अटल व्योम leaveJtMutex(व्योम){
  sqlite3_mutex_leave(sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_PRNG));
}

बाह्य पूर्णांक sqlite3_io_error_pending;
बाह्य पूर्णांक sqlite3_io_error_hit;
अटल व्योम stop_ioerr_simulation(पूर्णांक *piSave, पूर्णांक *piSave2){
  *piSave = sqlite3_io_error_pending;
  *piSave2 = sqlite3_io_error_hit;
  sqlite3_io_error_pending = -1;
  sqlite3_io_error_hit = 0;
}
अटल व्योम start_ioerr_simulation(पूर्णांक iSave, पूर्णांक iSave2){
  sqlite3_io_error_pending = iSave;
  sqlite3_io_error_hit = iSave2;
}

/*
** The jt_file pointed to by the argument may or may not be a file-handle
** open on a main database file. If it is, and a transaction is currently
** opened on the file, then discard all transaction related data.
*/
अटल व्योम closeTransaction(jt_file *p){
  sqlite3BitvecDestroy(p->pWritable);
  sqlite3_free(p->aCksum);
  p->pWritable = 0;
  p->aCksum = 0;
  p->nSync = 0;
}

/*
** Close an jt-file.
*/
अटल पूर्णांक jtClose(sqlite3_file *pFile){
  jt_file **pp;
  jt_file *p = (jt_file *)pFile;

  closeTransaction(p);
  enterJtMutex();
  अगर( p->zName ){
    क्रम(pp=&g.pList; *pp!=p; pp=&(*pp)->p_अग्र);
    *pp = p->p_अग्र;
  }
  leaveJtMutex();
  sqlite3OsClose(p->pReal);
  वापस SQLITE_OK;
}

/*
** Read data from an jt-file.
*/
अटल पूर्णांक jtRead(
  sqlite3_file *pFile, 
  व्योम *zBuf, 
  पूर्णांक iAmt, 
  sqlite_int64 iOfst
){
  jt_file *p = (jt_file *)pFile;
  वापस sqlite3OsRead(p->pReal, zBuf, iAmt, iOfst);
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
अटल jt_file *locateDatabaseHandle(स्थिर अक्षर *zJournal, पूर्णांक noLock){
  jt_file *pMain = 0;
  enterJtMutex();
  क्रम(pMain=g.pList; pMain; pMain=pMain->p_अग्र){
    पूर्णांक nName = (पूर्णांक)(म_माप(zJournal) - म_माप("-journal"));
    अगर( (pMain->flags&SQLITE_OPEN_MAIN_DB)
     && ((पूर्णांक)म_माप(pMain->zName)==nName)
     && 0==स_भेद(pMain->zName, zJournal, nName)
     && ((pMain->eLock>=SQLITE_LOCK_RESERVED) || noLock)
    ){
      अवरोध;
    }
  }
  leaveJtMutex();
  वापस pMain;
}

/*
** Parameter z points to a buffer of 4 bytes in size containing a 
** unsigned 32-bit integer stored in big-endian format. Decode the 
** integer and return its value.
*/
अटल u32 decodeUint32(स्थिर अचिन्हित अक्षर *z){
  वापस (z[0]<<24) + (z[1]<<16) + (z[2]<<8) + z[3];
}

/*
** Calculate a checksum from the buffer of length n bytes pointed to
** by parameter z.
*/
अटल u32 genCksum(स्थिर अचिन्हित अक्षर *z, पूर्णांक n){
  पूर्णांक i;
  u32 cksum = 0;
  क्रम(i=0; i<n; i++){
    cksum = cksum + z[i] + (cksum<<3);
  }
  वापस cksum;
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
अटल पूर्णांक decodeJournalHdr(
  स्थिर अचिन्हित अक्षर *zBuf,         /* Input: 28 byte journal header */
  u32 *pnRec,                        /* Out: Number of journalled records */
  u32 *pnPage,                       /* Out: Original database page count */
  u32 *pnSector,                     /* Out: Sector size in bytes */
  u32 *pnPagesize                    /* Out: Page size in bytes */
){
  अचिन्हित अक्षर aMagic[] = { 0xd9, 0xd5, 0x05, 0xf9, 0x20, 0xa1, 0x63, 0xd7 };
  अगर( स_भेद(aMagic, zBuf, 8) ) वापस SQLITE_ERROR;
  अगर( pnRec ) *pnRec = decodeUint32(&zBuf[8]);
  अगर( pnPage ) *pnPage = decodeUint32(&zBuf[16]);
  अगर( pnSector ) *pnSector = decodeUint32(&zBuf[20]);
  अगर( pnPagesize ) *pnPagesize = decodeUint32(&zBuf[24]);
  वापस SQLITE_OK;
}

/*
** This function is called when a new transaction is opened, just after
** the first journal-header is written to the journal file.
*/
अटल पूर्णांक openTransaction(jt_file *pMain, jt_file *pJournal){
  अचिन्हित अक्षर *a_तथ्य;
  sqlite3_file *p = pMain->pReal;
  पूर्णांक rc = SQLITE_OK;

  closeTransaction(pMain);
  a_तथ्य = sqlite3_malloc(pMain->nPagesize);
  pMain->pWritable = sqlite3BitvecCreate(pMain->nPage);
  pMain->aCksum = sqlite3_malloc(माप(u32) * (pMain->nPage + 1));
  pJournal->iMaxOff = 0;

  अगर( !pMain->pWritable || !pMain->aCksum || !a_तथ्य ){
    rc = SQLITE_IOERR_NOMEM;
  }अन्यथा अगर( pMain->nPage>0 ){
    u32 iTrunk;
    पूर्णांक iSave;
    पूर्णांक iSave2;

    stop_ioerr_simulation(&iSave, &iSave2);

    /* Read the database free-list. Add the page-number for each free-list
    ** leaf to the jt_file.pWritable bitvec.
    */
    rc = sqlite3OsRead(p, a_तथ्य, pMain->nPagesize, 0);
    अगर( rc==SQLITE_OK ){
      u32 nDbsize = decodeUint32(&a_तथ्य[28]);
      अगर( nDbsize>0 && स_भेद(&a_तथ्य[24], &a_तथ्य[92], 4)==0 ){
        u32 iPg;
        क्रम(iPg=nDbsize+1; iPg<=pMain->nPage; iPg++){
          sqlite3BitvecSet(pMain->pWritable, iPg);
        }
      }
    }
    iTrunk = decodeUint32(&a_तथ्य[32]);
    जबतक( rc==SQLITE_OK && iTrunk>0 ){
      u32 nLeaf;
      u32 iLeaf;
      sqlite3_int64 iOff = (i64)(iTrunk-1)*pMain->nPagesize;
      rc = sqlite3OsRead(p, a_तथ्य, pMain->nPagesize, iOff);
      nLeaf = decodeUint32(&a_तथ्य[4]);
      क्रम(iLeaf=0; rc==SQLITE_OK && iLeaf<nLeaf; iLeaf++){
        u32 pgno = decodeUint32(&a_तथ्य[8+4*iLeaf]);
        sqlite3BitvecSet(pMain->pWritable, pgno);
      }
      iTrunk = decodeUint32(a_तथ्य);
    }

    /* Calculate and store a checksum for each page in the database file. */
    अगर( rc==SQLITE_OK ){
      पूर्णांक ii;
      क्रम(ii=0; rc==SQLITE_OK && ii<(पूर्णांक)pMain->nPage; ii++){
        i64 iOff = (i64)(pMain->nPagesize) * (i64)ii;
        अगर( iOff==PENDING_BYTE ) जारी;
        rc = sqlite3OsRead(pMain->pReal, a_तथ्य, pMain->nPagesize, iOff);
        pMain->aCksum[ii] = genCksum(a_तथ्य, pMain->nPagesize);
        अगर( ii+1==(पूर्णांक)pMain->nPage && rc==SQLITE_IOERR_SHORT_READ ){
          rc = SQLITE_OK;
        }
      }
    }

    start_ioerr_simulation(iSave, iSave2);
  }

  sqlite3_free(a_तथ्य);
  वापस rc;
}

/*
** The first argument to this function is a handle open on a journal file.
** This function reads the journal file and adds the page number for each
** page in the journal to the Bitvec object passed as the second argument.
*/
अटल पूर्णांक readJournalFile(jt_file *p, jt_file *pMain){
  पूर्णांक rc = SQLITE_OK;
  अचिन्हित अक्षर zBuf[28];
  sqlite3_file *pReal = p->pReal;
  sqlite3_int64 iOff = 0;
  sqlite3_int64 i_आकार = p->iMaxOff;
  अचिन्हित अक्षर *aPage;
  पूर्णांक iSave;
  पूर्णांक iSave2;

  aPage = sqlite3_malloc(pMain->nPagesize);
  अगर( !aPage ){
    वापस SQLITE_IOERR_NOMEM;
  }

  stop_ioerr_simulation(&iSave, &iSave2);

  जबतक( rc==SQLITE_OK && iOff<i_आकार ){
    u32 nRec, nPage, nSector, nPagesize;
    u32 ii;

    /* Read and decode the next journal-header from the journal file. */
    rc = sqlite3OsRead(pReal, zBuf, 28, iOff);
    अगर( rc!=SQLITE_OK 
     || decodeJournalHdr(zBuf, &nRec, &nPage, &nSector, &nPagesize) 
    ){
      जाओ finish_rjf;
    }
    iOff += nSector;

    अगर( nRec==0 ){
      /* A trick. There might be another journal-header immediately 
      ** following this one. In this case, 0 records means 0 records, 
      ** not "read until the end of the file". See also ticket #2565.
      */
      अगर( i_आकार>=(iOff+nSector) ){
        rc = sqlite3OsRead(pReal, zBuf, 28, iOff);
        अगर( rc!=SQLITE_OK || 0==decodeJournalHdr(zBuf, 0, 0, 0, 0) ){
          जारी;
        }
      }
      nRec = (u32)((i_आकार-iOff) / (pMain->nPagesize+8));
    }

    /* Read all the records that follow the journal-header just read. */
    क्रम(ii=0; rc==SQLITE_OK && ii<nRec && iOff<i_आकार; ii++){
      u32 pgno;
      rc = sqlite3OsRead(pReal, zBuf, 4, iOff);
      अगर( rc==SQLITE_OK ){
        pgno = decodeUint32(zBuf);
        अगर( pgno>0 && pgno<=pMain->nPage ){
          अगर( 0==sqlite3BitvecTest(pMain->pWritable, pgno) ){
            rc = sqlite3OsRead(pReal, aPage, pMain->nPagesize, iOff+4);
            अगर( rc==SQLITE_OK ){
              u32 cksum = genCksum(aPage, pMain->nPagesize);
              निश्चित( cksum==pMain->aCksum[pgno-1] );
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
  अगर( rc==SQLITE_IOERR_SHORT_READ ){
    rc = SQLITE_OK;
  }
  वापस rc;
}

/*
** Write data to an jt-file.
*/
अटल पूर्णांक jtWrite(
  sqlite3_file *pFile, 
  स्थिर व्योम *zBuf, 
  पूर्णांक iAmt, 
  sqlite_int64 iOfst
){
  पूर्णांक rc;
  jt_file *p = (jt_file *)pFile;
  अगर( p->flags&SQLITE_OPEN_MAIN_JOURNAL ){
    अगर( iOfst==0 ){
      jt_file *pMain = locateDatabaseHandle(p->zName, 0);
      निश्चित( pMain );
  
      अगर( iAmt==28 ){
        /* Zeroing the first journal-file header. This is the end of a
        ** transaction. */
        closeTransaction(pMain);
      }अन्यथा अगर( iAmt!=12 ){
        /* Writing the first journal header to a journal file. This happens
        ** when a transaction is first started.  */
        u8 *z = (u8 *)zBuf;
        pMain->nPage = decodeUint32(&z[16]);
        pMain->nPagesize = decodeUint32(&z[24]);
        अगर( SQLITE_OK!=(rc=openTransaction(pMain, p)) ){
          वापस rc;
        }
      }
    }
    अगर( p->iMaxOff<(iOfst + iAmt) ){
      p->iMaxOff = iOfst + iAmt;
    }
  }

  अगर( p->flags&SQLITE_OPEN_MAIN_DB && p->pWritable ){
    अगर( iAmt<(पूर्णांक)p->nPagesize 
     && p->nPagesize%iAmt==0 
     && iOfst>=(PENDING_BYTE+512) 
     && iOfst+iAmt<=PENDING_BYTE+p->nPagesize
    ){
      /* No-op. This special case is hit when the backup code is copying a
      ** to a database with a larger page-size than the source database and
      ** it needs to fill in the non-locking-region part of the original
      ** pending-byte page.
      */
    }अन्यथा{
      u32 pgno = (u32)(iOfst/p->nPagesize + 1);
      निश्चित( (iAmt==1||iAmt==(पूर्णांक)p->nPagesize) &&
              ((iOfst+iAmt)%p->nPagesize)==0 );
      /* The following assert() statements may fail if this layer is used
      ** with a connection in "PRAGMA synchronous=off" mode. If they
      ** fail with sync=normal or sync=full, this may indicate problem.  */
      निश्चित( p->nPage==0 || pgno<=p->nPage || p->nSync>0 );
      निश्चित( pgno>p->nPage || sqlite3BitvecTest(p->pWritable, pgno) );
    }
  }

  rc = sqlite3OsWrite(p->pReal, zBuf, iAmt, iOfst);
  अगर( (p->flags&SQLITE_OPEN_MAIN_JOURNAL) && iAmt==12 ){
    jt_file *pMain = locateDatabaseHandle(p->zName, 0);
    पूर्णांक rc2 = readJournalFile(p, pMain);
    अगर( rc==SQLITE_OK ) rc = rc2;
  }
  वापस rc;
}

/*
** Truncate an jt-file.
*/
अटल पूर्णांक jtTruncate(sqlite3_file *pFile, sqlite_int64 आकार){
  jt_file *p = (jt_file *)pFile;
  अगर( p->flags&SQLITE_OPEN_MAIN_JOURNAL && आकार==0 ){
    /* Truncating a journal file. This is the end of a transaction. */
    jt_file *pMain = locateDatabaseHandle(p->zName, 0);
    closeTransaction(pMain);
  }
  अगर( p->flags&SQLITE_OPEN_MAIN_DB && p->pWritable ){
    u32 pgno;
    u32 locking_page = (u32)(PENDING_BYTE/p->nPagesize+1);
    क्रम(pgno=(u32)(आकार/p->nPagesize+1); pgno<=p->nPage; pgno++){
      निश्चित( pgno==locking_page || sqlite3BitvecTest(p->pWritable, pgno) );
    }
  }
  वापस sqlite3OsTruncate(p->pReal, आकार);
}

/*
** Sync an jt-file.
*/
अटल पूर्णांक jtSync(sqlite3_file *pFile, पूर्णांक flags){
  jt_file *p = (jt_file *)pFile;

  अगर( p->flags&SQLITE_OPEN_MAIN_JOURNAL ){
    पूर्णांक rc;
    jt_file *pMain;                   /* The associated database file */

    /* The journal file is being synced. At this point, we inspect the 
    ** contents of the file up to this point and set each bit in the 
    ** jt_file.pWritable bitvec of the main database file associated with
    ** this journal file.
    */
    pMain = locateDatabaseHandle(p->zName, 0);

    /* Set the bitvec values */
    अगर( pMain && pMain->pWritable ){
      pMain->nSync++;
      rc = readJournalFile(p, pMain);
      अगर( rc!=SQLITE_OK ){
        वापस rc;
      }
    }
  }

  वापस sqlite3OsSync(p->pReal, flags);
}

/*
** Return the current file-size of an jt-file.
*/
अटल पूर्णांक jt_File_आकार(sqlite3_file *pFile, sqlite_int64 *p_आकार){
  jt_file *p = (jt_file *)pFile;
  वापस sqlite3_Os_File_आकार(p->pReal, p_आकार);
}

/*
** Lock an jt-file.
*/
अटल पूर्णांक jtLock(sqlite3_file *pFile, पूर्णांक eLock){
  पूर्णांक rc;
  jt_file *p = (jt_file *)pFile;
  rc = sqlite3OsLock(p->pReal, eLock);
  अगर( rc==SQLITE_OK && eLock>p->eLock ){
    p->eLock = eLock;
  }
  वापस rc;
}

/*
** Unlock an jt-file.
*/
अटल पूर्णांक jtUnlock(sqlite3_file *pFile, पूर्णांक eLock){
  पूर्णांक rc;
  jt_file *p = (jt_file *)pFile;
  rc = sqlite3OsUnlock(p->pReal, eLock);
  अगर( rc==SQLITE_OK && eLock<p->eLock ){
    p->eLock = eLock;
  }
  वापस rc;
}

/*
** Check if another file-handle holds a RESERVED lock on an jt-file.
*/
अटल पूर्णांक jtCheckReservedLock(sqlite3_file *pFile, पूर्णांक *pResOut){
  jt_file *p = (jt_file *)pFile;
  वापस sqlite3OsCheckReservedLock(p->pReal, pResOut);
}

/*
** File control method. For custom operations on an jt-file.
*/
अटल पूर्णांक jtFileControl(sqlite3_file *pFile, पूर्णांक op, व्योम *pArg){
  jt_file *p = (jt_file *)pFile;
  वापस p->pReal->pMethods->xFileControl(p->pReal, op, pArg);
}

/*
** Return the sector-size in bytes for an jt-file.
*/
अटल पूर्णांक jt_Sector_आकार(sqlite3_file *pFile){
  jt_file *p = (jt_file *)pFile;
  वापस sqlite3_Os_Sector_आकार(p->pReal);
}

/*
** Return the device characteristic flags supported by an jt-file.
*/
अटल पूर्णांक jtDeviceCharacteristics(sqlite3_file *pFile){
  jt_file *p = (jt_file *)pFile;
  वापस sqlite3OsDeviceCharacteristics(p->pReal);
}

/*
** Open an jt file handle.
*/
अटल पूर्णांक jtOpen(
  sqlite3_vfs *pVfs,
  स्थिर अक्षर *zName,
  sqlite3_file *pFile,
  पूर्णांक flags,
  पूर्णांक *pOutFlags
){
  पूर्णांक rc;
  jt_file *p = (jt_file *)pFile;
  pFile->pMethods = 0;
  p->pReal = (sqlite3_file *)&p[1];
  p->pReal->pMethods = 0;
  rc = sqlite3OsOpen(g.pVfs, zName, p->pReal, flags, pOutFlags);
  निश्चित( rc==SQLITE_OK || p->pReal->pMethods==0 );
  अगर( rc==SQLITE_OK ){
    pFile->pMethods = &jt_io_methods;
    p->eLock = 0;
    p->zName = zName;
    p->flags = flags;
    p->p_अग्र = 0;
    p->pWritable = 0;
    p->aCksum = 0;
    enterJtMutex();
    अगर( zName ){
      p->p_अग्र = g.pList;
      g.pList = p;
    }
    leaveJtMutex();
  }
  वापस rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
अटल पूर्णांक jtDelete(sqlite3_vfs *pVfs, स्थिर अक्षर *zPath, पूर्णांक dirSync){
  पूर्णांक nPath = (पूर्णांक)म_माप(zPath);
  अगर( nPath>8 && 0==म_भेद("-journal", &zPath[nPath-8]) ){
    /* Deleting a journal file. The end of a transaction. */
    jt_file *pMain = locateDatabaseHandle(zPath, 0);
    अगर( pMain ){
      closeTransaction(pMain);
    }
  }

  वापस sqlite3OsDelete(g.pVfs, zPath, dirSync);
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
अटल पूर्णांक jtAccess(
  sqlite3_vfs *pVfs, 
  स्थिर अक्षर *zPath, 
  पूर्णांक flags, 
  पूर्णांक *pResOut
){
  वापस sqlite3OsAccess(g.pVfs, zPath, flags, pResOut);
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (JT_MAX_PATHNAME+1) bytes.
*/
अटल पूर्णांक jtFullPathname(
  sqlite3_vfs *pVfs, 
  स्थिर अक्षर *zPath, 
  पूर्णांक nOut, 
  अक्षर *zOut
){
  वापस sqlite3OsFullPathname(g.pVfs, zPath, nOut, zOut);
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
अटल व्योम *jtDlOpen(sqlite3_vfs *pVfs, स्थिर अक्षर *zPath){
  वापस g.pVfs->xDlOpen(g.pVfs, zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
अटल व्योम jtDlError(sqlite3_vfs *pVfs, पूर्णांक nByte, अक्षर *z_Err_संदेश){
  g.pVfs->xDlError(g.pVfs, nByte, z_Err_संदेश);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
अटल व्योम (*jtDlSym(sqlite3_vfs *pVfs, व्योम *p, स्थिर अक्षर *zSym))(व्योम){
  वापस g.pVfs->xDlSym(g.pVfs, p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
अटल व्योम jtDlClose(sqlite3_vfs *pVfs, व्योम *pHandle){
  g.pVfs->xDlClose(g.pVfs, pHandle);
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
अटल पूर्णांक jtRandomness(sqlite3_vfs *pVfs, पूर्णांक nByte, अक्षर *zBufOut){
  वापस sqlite3OsRandomness(g.pVfs, nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
अटल पूर्णांक jtSleep(sqlite3_vfs *pVfs, पूर्णांक nMicro){
  वापस sqlite3OsSleep(g.pVfs, nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
अटल पूर्णांक jtCurrentTime(sqlite3_vfs *pVfs, द्विगुन *pTimeOut){
  वापस g.pVfs->xCurrentTime(g.pVfs, pTimeOut);
}
/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
अटल पूर्णांक jtCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *pTimeOut){
  वापस g.pVfs->xCurrentTimeInt64(g.pVfs, pTimeOut);
}

अटल पूर्णांक jtGetLastError(sqlite3_vfs *pVfs, पूर्णांक n, अक्षर *z){
  वापस g.pVfs->xGetLastError(g.pVfs, n, z);
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
पूर्णांक jt_register(अक्षर *zWrap, पूर्णांक isDefault){
  g.pVfs = sqlite3_vfs_find(zWrap);
  अगर( g.pVfs==0 ){
    वापस SQLITE_ERROR;
  }
  jt_vfs.szOsFile = माप(jt_file) + g.pVfs->szOsFile;
  अगर( g.pVfs->iVersion==1 ){
    jt_vfs.iVersion = 1;
  }अन्यथा अगर( g.pVfs->xCurrentTimeInt64==0 ){
    jt_vfs.xCurrentTimeInt64 = 0;
  }
  sqlite3_vfs_register(&jt_vfs, isDefault);
  वापस SQLITE_OK;
}

/*
** Uninstall the jt VFS, if it is installed.
*/
व्योम jt_unregister(व्योम){
  sqlite3_vfs_unregister(&jt_vfs);
}

#पूर्ण_अगर
