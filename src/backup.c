/*
** 2009 January 28
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file contains the implementation of the sqlite3_backup_XXX() 
** API functions and the related features.
*/
#include "sqliteInt.h"
#include "btreeInt.h"

/*
** Structure allocated for each backup operation.
*/
sa_nracanaa sqlite3_backup {
  sqlite3* pDestDb;        /* Destination database handle */
  var_na *zDestDb;
  Btree *pDest;            /* Destination b-tree file */
  u32 iDestSchema;         /* Original schema cookie in destination */
  ចំនួនគត់ bDestLocked;         /* True once a write-transaction is open on pDest */

  Pgno iNext;              /* Page number of the next source page to copy */
  sqlite3* pSrcDb;         /* Source database handle */
  Btree *pSrc;             /* Source b-tree file */

  ចំនួនគត់ rc;                  /* Backup process error code */

  /* These two variables are set by every call to backup_step(). They are
  ** read by calls to backup_remaining() and backup_pagecount().
  */
  Pgno nRemaining;         /* Number of pages left to copy */
  Pgno nPagecount;         /* Total number of pages to copy */

  ចំនួនគត់ isAttached;          /* True once backup has been registered with pager */
  sqlite3_backup *pNext;   /* Next backup associated with source pager */
};

/*
** THREAD SAFETY NOTES:
**
**   Once it has been created using backup_init(), a single sqlite3_backup
**   structure may be accessed via two groups of thread-safe entry points:
**
**     * Via the sqlite3_backup_XXX() API function backup_step() and 
**       backup_finish(). Both these functions obtain the source database
**       handle mutex and the mutex associated with the source BtShared 
**       structure, in that order.
**
**     * Via the BackupUpdate() and BackupRestart() functions, which are
**       invoked by the pager layer to report various state changes in
**       the page cache associated with the source database. The mutex
**       associated with the source database BtShared structure will always 
**       be held when either of these functions are invoked.
**
**   The other sqlite3_backup_XXX() API functions, backup_remaining() and
**   backup_pagecount() are not thread-safe functions. If they are called
**   while some other thread is calling backup_step() or backup_finish(),
**   the values returned may be invalid. There is no way for a call to
**   BackupUpdate() or BackupRestart() to interfere with backup_remaining()
**   or backup_pagecount().
**
**   Depending on the SQLite configuration, the database handles and/or
**   the Btree objects may have their own mutexes that require locking.
**   Non-sharable Btrees (in-memory databases for example), do not have
**   associated mutexes.
*/

/*
** Return a pointer corresponding to database zDb (i.e. "main", "temp")
** in connection handle pDb. If such a database cannot be found, return
** a NULL pointer and write an error message to pErrorDb.
**
** If the "temp" database is requested, it may need to be opened by this 
** function. If an error occurs while doing so, return 0 and write an 
** error message to pErrorDb.
*/
static Btree *findBtree(sqlite3 *pErrorDb, sqlite3 *pDb, const var_na *zDb){
  ចំនួនគត់ i;

  assert( pDb!=0 );
  i = sqlite3FindDbName(pDb, zDb);
  ប្រសិនបើ( i==1 ){
    Parse sParse;
    ចំនួនគត់ rc = 0;
    sqlite3ParseObjectInit(&sParse,pDb);
    ប្រសិនបើ( sqlite3OpenTempDatabase(&sParse) ){
      sqlite3ErrorWithMsg(pErrorDb, sParse.rc, "%s", sParse.zErrMsg);
      rc = SQLITE_ERROR;
    }
    sqlite3DbFree(pErrorDb, sParse.zErrMsg);
    sqlite3ParseObjectReset(&sParse);
    ប្រសិនបើ( rc ){
      ត្រឡប់ 0;
    }
  }

  ប្រសិនបើ( i<0 ){
    sqlite3ErrorWithMsg(pErrorDb, SQLITE_ERROR, "unknown database %s", zDb);
    ត្រឡប់ 0;
  }

  ត្រឡប់ pDb->aDb[i].pBt;
}

/*
** Attempt to set the page size of the destination to match the page size
** of the source.
*/
static ចំនួនគត់ setDestPgsz(Btree *pDest, Btree *pSrc){
  ត្រឡប់ sqlite3BtreeSetPageSize(pDest, sqlite3BtreeGetPageSize(pSrc), 0, 0);
}

/*
** Check that there is no open read-transaction on the b-tree passed as the
** second argument. If there is not, return SQLITE_OK. Otherwise, if there
** is an open read-transaction, return SQLITE_ERROR and leave an error 
** message in database handle db.
*/
static ចំនួនគត់ checkReadTransaction(sqlite3 *db, Btree *p){
  ប្រសិនបើ( sqlite3BtreeTxnState(p)!=SQLITE_TXN_NONE ){
    sqlite3ErrorWithMsg(db, SQLITE_ERROR, "destination database is in use");
    ត្រឡប់ SQLITE_ERROR;
  }
  ត្រឡប់ SQLITE_OK;
}

/*
** Create an sqlite3_backup process to copy the contents of zSrcDb from
** connection handle pSrcDb to zDestDb in pDestDb. If successful, return
** a pointer to the new sqlite3_backup object.
**
** If an error occurs, NULL is returned and an error code and error message
** stored in database handle pDestDb.
*/
sqlite3_backup *sqlite3_backup_init(
  sqlite3* pDestDb,                     /* Database to write to */
  const var_na *zDestDb,                  /* Name of database within pDestDb */
  sqlite3* pSrcDb,                      /* Database connection to read from */
  const var_na *zSrcDb                    /* Name of database within pSrcDb */
){
  sqlite3_backup *p;                    /* Value to return */

#ifdef SQLITE_ENABLE_API_ARMOR
  ប្រសិនបើ( !sqlite3SafetyCheckOk(pSrcDb)||!sqlite3SafetyCheckOk(pDestDb) ){
    (void)SQLITE_MISUSE_BKPT;
    ត្រឡប់ 0;
  }
#endif

  /* Lock the source database handle. The destination database
  ** handle is not locked in this routine, but it is locked in
  ** sqlite3_backup_step(). The user is required to ensure that no
  ** other thread accesses the destination handle for the duration
  ** of the backup operation.  Any attempt to use the destination
  ** database connection while a backup is in progress may cause
  ** a malfunction or a deadlock.
  */
  sqlite3_mutex_enter(pSrcDb->mutex);
  sqlite3_mutex_enter(pDestDb->mutex);

  ប្រសិនបើ( pSrcDb==pDestDb ){
    sqlite3ErrorWithMsg(
        pDestDb, SQLITE_ERROR, "source and destination must be distinct"
    );
    p = 0;
  }បើមិនដូច្នេះ {
    ចំនួនគត់ nDest = sqlite3Strlen30(zDestDb);

    /* Allocate space for a new sqlite3_backup object...
    ** EVIDENCE-OF: R-64852-21591 The sqlite3_backup object is created by a
    ** call to sqlite3_backup_init() and is destroyed by a call to
    ** sqlite3_backup_finish(). */
    p = (sqlite3_backup*)sqlite3MallocZero(parimaa_na(sqlite3_backup)+nDest+1);
    ប្រសិនបើ( !p ){
      sqlite3Error(pDestDb, SQLITE_NOMEM_BKPT);
    }បើមិនដូច្នេះ{
      p->zDestDb = (var_na*)&p[1];
      memcpy(p->zDestDb, zDestDb, nDest);
    }
  }

  /* If the allocation succeeded, populate the new object. */
  ប្រសិនបើ( p ){
    /* Do not store the pointer to the destination b-tree at this point.
    ** This is because there is nothing preventing it from being detached
    ** or otherwise freed before the first call to sqlite3_backup_step()
    ** on this object. The source b-tree does not have this problem, as
    ** incrementing Btree.nBackup (see below) effectively locks the object. */
    Btree *pDest = findBtree(pDestDb, pDestDb, zDestDb);
    p->pSrc = findBtree(pDestDb, pSrcDb, zSrcDb);
    p->pDestDb = pDestDb;
    p->pSrcDb = pSrcDb;
    p->iNext = 1;
    p->isAttached = 0;

    ប្រសិនបើ( 0==p->pSrc || 0==pDest 
     || checkReadTransaction(pDestDb, pDest)!=SQLITE_OK 
     ){
      /* One (or both) of the named databases did not exist or an OOM
      ** error was hit. Or there is a transaction open on the destination
      ** database. The error has already been written into the pDestDb 
      ** handle. All that is left to do here is free the sqlite3_backup 
      ** structure.  */
      sqlite3_free(p);
      p = 0;
    }
  }
  ប្រសិនបើ( p ){
    p->pSrc->nBackup++;
  }

  sqlite3_mutex_leave(pDestDb->mutex);
  sqlite3_mutex_leave(pSrcDb->mutex);
  ត្រឡប់ p;
}

/*
** Argument rc is an SQLite error code. Return true if this error is 
** considered fatal if encountered during a backup operation. All errors
** are considered fatal except for SQLITE_BUSY and SQLITE_LOCKED.
*/
static ចំនួនគត់ isFatalError(ចំនួនគត់ rc){
  ត្រឡប់ (rc!=SQLITE_OK && rc!=SQLITE_BUSY && ALWAYS(rc!=SQLITE_LOCKED));
}

/*
** Parameter zSrcData points to a buffer containing the data for 
** page iSrcPg from the source database. Copy this data into the 
** destination database.
*/
static ចំនួនគត់ backupOnePage(
  sqlite3_backup *p,              /* Backup handle */
  Pgno iSrcPg,                    /* Source database page to backup */
  const u8 *zSrcData,             /* Source database page data */
  ចំនួនគត់ bUpdate                     /* True for an update, false otherwise */
){
  Pager * const pDestPager = sqlite3BtreePager(p->pDest);
  const ចំនួនគត់ nSrcPgsz = sqlite3BtreeGetPageSize(p->pSrc);
  ចំនួនគត់ nDestPgsz = sqlite3BtreeGetPageSize(p->pDest);
  const ចំនួនគត់ nCopy = MIN(nSrcPgsz, nDestPgsz);
  const i64 iEnd = (i64)iSrcPg*(i64)nSrcPgsz;
  ចំនួនគត់ rc = SQLITE_OK;
  i64 iOff;

  assert( sqlite3BtreeGetReserveNoMutex(p->pSrc)>=0 );
  assert( p->bDestLocked );
  assert( !isFatalError(p->rc) );
  assert( iSrcPg!=PENDING_BYTE_PAGE(p->pSrc->pBt) );
  assert( zSrcData );
  assert( nSrcPgsz==nDestPgsz || sqlite3PagerIsMemdb(pDestPager)==0 );

  /* This loop runs once for each destination page spanned by the source 
  ** page. For each iteration, variable iOff is set to the byte offset
  ** of the destination page.
  */
  សម្រាប់(iOff=iEnd-(i64)nSrcPgsz; rc==SQLITE_OK && iOff<iEnd; iOff+=nDestPgsz){
    DbPage *pDestPg = 0;
    Pgno iDest = (Pgno)(iOff/nDestPgsz)+1;
    ប្រសិនបើ( iDest==PENDING_BYTE_PAGE(p->pDest->pBt) ) continue;
    ប្រសិនបើ( SQLITE_OK==(rc = sqlite3PagerGet(pDestPager, iDest, &pDestPg, 0))
     && SQLITE_OK==(rc = sqlite3PagerWrite(pDestPg))
    ){
      const u8 *zIn = &zSrcData[iOff%nSrcPgsz];
      u8 *zDestData = sqlite3PagerGetData(pDestPg);
      u8 *zOut = &zDestData[iOff%nDestPgsz];

      /* Copy the data from the source page into the destination page.
      ** Then clear the Btree layer MemPage.isInit flag. Both this module
      ** and the pager code use this trick (clearing the first byte
      ** of the page 'extra' space to invalidate the Btree layers
      ** cached parse of the page). MemPage.isInit is marked 
      ** "MUST BE FIRST" for this purpose.
      */
      memcpy(zOut, zIn, nCopy);
      ((u8 *)sqlite3PagerGetExtra(pDestPg))[0] = 0;
      ប្រសិនបើ( iOff==0 && bUpdate==0 ){
        sqlite3Put4byte(&zOut[28], sqlite3BtreeLastPage(p->pSrc));
      }
    }
    sqlite3PagerUnref(pDestPg);
  }

  ត្រឡប់ rc;
}

/*
** If pFile is currently larger than iSize bytes, then truncate it to
** exactly iSize bytes. If pFile is not larger than iSize bytes, then
** this function is a no-op.
**
** Return SQLITE_OK if everything is successful, or an SQLite error 
** code if an error occurs.
*/
static ចំនួនគត់ backupTruncateFile(sqlite3_file *pFile, i64 iSize){
  i64 iCurrent;
  ចំនួនគត់ rc = sqlite3OsFileSize(pFile, &iCurrent);
  ប្រសិនបើ( rc==SQLITE_OK && iCurrent>iSize ){
    rc = sqlite3OsTruncate(pFile, iSize);
  }
  ត្រឡប់ rc;
}

/*
** Register this backup object with the associated source pager for
** callbacks when pages are changed or the cache invalidated.
*/
static void attachBackupObject(sqlite3_backup *p){
  sqlite3_backup **pp;
  assert( sqlite3BtreeHoldsMutex(p->pSrc) );
  pp = sqlite3PagerBackupPtr(sqlite3BtreePager(p->pSrc));
  p->pNext = *pp;
  *pp = p;
  p->isAttached = 1;
}

/*
** Copy nPage pages from the source b-tree to the destination.
*/
ចំនួនគត់ sqlite3_backup_step(sqlite3_backup *p, ចំនួនគត់ nPage){
  ចំនួនគត់ rc;
  ចំនួនគត់ destMode = 0;   /* Destination journal mode */
  ចំនួនគត់ pgszSrc = 0;    /* Source page size */
  ចំនួនគត់ pgszDest = 0;   /* Destination page size */

#ifdef SQLITE_ENABLE_API_ARMOR
  ប្រសិនបើ( p==0 ) ត្រឡប់ SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(p->pSrcDb->mutex);
  sqlite3BtreeEnter(p->pSrc);
  ប្រសិនបើ( p->pDestDb ){
    sqlite3_mutex_enter(p->pDestDb->mutex);
  }

  rc = p->rc;
  ប្រសិនបើ( !isFatalError(rc) ){
    Pager * const pSrcPager = sqlite3BtreePager(p->pSrc);     /* Source pager */
    Btree * pDest = 0;                                        /* Dest btree */
    Pager * pDestPager = 0;                                   /* Dest pager */
    ចំនួនគត់ ii;                            /* Iterator variable */
    ចំនួនគត់ nSrcPage = -1;                 /* Size of source db in pages */
    ចំនួនគត់ bCloseTrans = 0;               /* True if src db requires unlocking */

    /* If the source pager is currently in a write-transaction, return
    ** SQLITE_BUSY immediately.
    */
    ប្រសិនបើ( p->pDestDb && p->pSrc->pBt->inTransaction==TRANS_WRITE ){
      rc = SQLITE_BUSY;
    }បើមិនដូច្នេះ{
      rc = SQLITE_OK;
    }


    /* If there is no open read-transaction on the source database, open
    ** one now. If a transaction is opened here, then it will be closed
    ** before this function exits.
    */
    ប្រសិនបើ( rc==SQLITE_OK && SQLITE_TXN_NONE==sqlite3BtreeTxnState(p->pSrc) ){
      rc = sqlite3BtreeBeginTrans(p->pSrc, 0, 0);
      bCloseTrans = 1;
    }

    /* Locate the destination btree and pager. */
    ប្រសិនបើ( (pDest = p->pDest)==0 ){
      pDest = findBtree(p->pDestDb, p->pDestDb, p->zDestDb);
    }
    ប្រសិនបើ( pDest==0 ){
      rc = SQLITE_ERROR;
    }បើមិនដូច្នេះ{
      pDestPager = sqlite3BtreePager(pDest);
    }

    /* If the destination database has not yet been locked (i.e. if this
    ** is the first call to backup_step() for the current backup operation),
    ** try to set its page size to the same as the source database. This
    ** is especially important on ZipVFS systems, as in that case it is
    ** not possible to create a database file that uses one page size by
    ** writing to it with another.  */
    ប្រសិនបើ( p->bDestLocked==0 && rc==SQLITE_OK 
     && setDestPgsz(pDest, p->pSrc)==SQLITE_NOMEM
    ){
      rc = SQLITE_NOMEM;
    }

    /* Lock the destination database, if it is not locked already. */
    ប្រសិនបើ( SQLITE_OK==rc && p->bDestLocked==0
     && SQLITE_OK==(rc = sqlite3BtreeBeginTrans(pDest, 2,
                                                (ចំនួនគត់*)&p->iDestSchema)) 
    ){
      p->bDestLocked = 1;
      p->pDest = pDest;
    }

    /* Do not allow backup if the destination database is in WAL mode
    ** and the page sizes are different between source and destination */
    ប្រសិនបើ( rc==SQLITE_OK ){
      pgszSrc = sqlite3BtreeGetPageSize(p->pSrc);
      pgszDest = sqlite3BtreeGetPageSize(p->pDest);
      destMode = sqlite3PagerGetJournalMode(sqlite3BtreePager(p->pDest));
      ប្រសិនបើ( (destMode==PAGER_JOURNALMODE_WAL || sqlite3PagerIsMemdb(pDestPager))
        && pgszSrc!=pgszDest 
      ){
        rc = SQLITE_READONLY;
      }
    }
  
    /* Now that there is a read-lock on the source database, query the
    ** source pager for the number of pages in the database.
    */
    nSrcPage = (ចំនួនគត់)sqlite3BtreeLastPage(p->pSrc);
    assert( nSrcPage>=0 );
    សម្រាប់(ii=0; (nPage<0 || ii<nPage) && p->iNext<=(Pgno)nSrcPage && !rc; ii++){
      const Pgno iSrcPg = p->iNext;                 /* Source page number */
      ប្រសិនបើ( iSrcPg!=PENDING_BYTE_PAGE(p->pSrc->pBt) ){
        DbPage *pSrcPg;                             /* Source page object */
        rc = sqlite3PagerGet(pSrcPager, iSrcPg, &pSrcPg,PAGER_GET_READONLY);
        ប្រសិនបើ( rc==SQLITE_OK ){
          rc = backupOnePage(p, iSrcPg, sqlite3PagerGetData(pSrcPg), 0);
          sqlite3PagerUnref(pSrcPg);
        }
      }
      p->iNext++;
    }
    ប្រសិនបើ( rc==SQLITE_OK ){
      p->nPagecount = nSrcPage;
      p->nRemaining = nSrcPage+1-p->iNext;
      ប្រសិនបើ( p->iNext>(Pgno)nSrcPage ){
        rc = SQLITE_DONE;
      }បើមិនដូច្នេះ ប្រសិនបើ( !p->isAttached ){
        attachBackupObject(p);
      }
    }
  
    /* Update the schema version field in the destination database. This
    ** is to make sure that the schema-version really does change in
    ** the case where the source and destination databases have the
    ** same schema version.
    */
    ប្រសិនបើ( rc==SQLITE_DONE ){
      ប្រសិនបើ( nSrcPage==0 ){
        rc = sqlite3BtreeNewDb(p->pDest);
        nSrcPage = 1;
      }
      ប្រសិនបើ( rc==SQLITE_OK || rc==SQLITE_DONE ){
        rc = sqlite3BtreeUpdateMeta(p->pDest,1,p->iDestSchema+1);
      }
      ប្រសិនបើ( rc==SQLITE_OK ){
        ប្រសិនបើ( p->pDestDb ){
          sqlite3ResetAllSchemasOfConnection(p->pDestDb);
        }
        ប្រសិនបើ( destMode==PAGER_JOURNALMODE_WAL ){
          rc = sqlite3BtreeSetVersion(p->pDest, 2);
        }
      }
      ប្រសិនបើ( rc==SQLITE_OK ){
        ចំនួនគត់ nDestTruncate;
        /* Set nDestTruncate to the final number of pages in the destination
        ** database. The complication here is that the destination page
        ** size may be different to the source page size. 
        **
        ** If the source page size is smaller than the destination page size, 
        ** round up. In this case the call to sqlite3OsTruncate() below will
        ** fix the size of the file. However it is important to call
        ** sqlite3PagerTruncateImage() here so that any pages in the 
        ** destination file that lie beyond the nDestTruncate page mark are
        ** journalled by PagerCommitPhaseOne() before they are destroyed
        ** by the file truncation.
        */
        assert( pgszSrc==sqlite3BtreeGetPageSize(p->pSrc) );
        assert( pgszDest==sqlite3BtreeGetPageSize(p->pDest) );
        ប្រសិនបើ( pgszSrc<pgszDest ){
          ចំនួនគត់ ratio = pgszDest/pgszSrc;
          nDestTruncate = (nSrcPage+ratio-1)/ratio;
          ប្រសិនបើ( nDestTruncate==(ចំនួនគត់)PENDING_BYTE_PAGE(p->pDest->pBt) ){
            nDestTruncate--;
          }
        }បើមិនដូច្នេះ{
          nDestTruncate = nSrcPage * (pgszSrc/pgszDest);
        }
        assert( nDestTruncate>0 );

        ប្រសិនបើ( pgszSrc<pgszDest ){
          /* If the source page-size is smaller than the destination page-size,
          ** two extra things may need to happen:
          **
          **   * The destination may need to be truncated, and
          **
          **   * Data stored on the pages immediately following the 
          **     pending-byte page in the source database may need to be
          **     copied into the destination database.
          */
          const i64 iSize = (i64)pgszSrc * (i64)nSrcPage;
          sqlite3_file * const pFile = sqlite3PagerFile(pDestPager);
          Pgno iPg;
          ចំនួនគត់ nDstPage;
          i64 iOff;
          i64 iEnd;

          assert( pFile );
          assert( nDestTruncate==0 
              || (i64)nDestTruncate*(i64)pgszDest >= iSize || (
                nDestTruncate==(ចំនួនគត់)(PENDING_BYTE_PAGE(p->pDest->pBt)-1)
             && iSize>=PENDING_BYTE && iSize<=PENDING_BYTE+pgszDest
          ));

          /* This block ensures that all data required to recreate the original
          ** database has been stored in the journal for pDestPager and the
          ** journal synced to disk. So at this point we may safely modify
          ** the database file in any way, knowing that if a power failure
          ** occurs, the original database will be reconstructed from the 
          ** journal file.  */
          sqlite3PagerPagecount(pDestPager, &nDstPage);
          សម្រាប់(iPg=nDestTruncate; rc==SQLITE_OK && iPg<=(Pgno)nDstPage; iPg++){
            ប្រសិនបើ( iPg!=PENDING_BYTE_PAGE(p->pDest->pBt) ){
              DbPage *pPg;
              rc = sqlite3PagerGet(pDestPager, iPg, &pPg, 0);
              ប្រសិនបើ( rc==SQLITE_OK ){
                rc = sqlite3PagerWrite(pPg);
                sqlite3PagerUnref(pPg);
              }
            }
          }
          ប្រសិនបើ( rc==SQLITE_OK ){
            rc = sqlite3PagerCommitPhaseOne(pDestPager, 0, 1);
          }

          /* Write the extra pages and truncate the database file as required */
          iEnd = MIN(PENDING_BYTE + pgszDest, iSize);
          សម្រាប់(
            iOff=PENDING_BYTE+pgszSrc; 
            rc==SQLITE_OK && iOff<iEnd; 
            iOff+=pgszSrc
          ){
            PgHdr *pSrcPg = 0;
            const Pgno iSrcPg = (Pgno)((iOff/pgszSrc)+1);
            rc = sqlite3PagerGet(pSrcPager, iSrcPg, &pSrcPg, 0);
            ប្រសិនបើ( rc==SQLITE_OK ){
              u8 *zData = sqlite3PagerGetData(pSrcPg);
              rc = sqlite3OsWrite(pFile, zData, pgszSrc, iOff);
            }
            sqlite3PagerUnref(pSrcPg);
          }
          ប្រសិនបើ( rc==SQLITE_OK ){
            rc = backupTruncateFile(pFile, iSize);
          }

          /* Sync the database file to disk. */
          ប្រសិនបើ( rc==SQLITE_OK ){
            rc = sqlite3PagerSync(pDestPager, 0);
          }
        }បើមិនដូច្នេះ{
          sqlite3PagerTruncateImage(pDestPager, nDestTruncate);
          rc = sqlite3PagerCommitPhaseOne(pDestPager, 0, 0);
        }
    
        /* Finish committing the transaction to the destination database. */
        ប្រសិនបើ( SQLITE_OK==rc
         && SQLITE_OK==(rc = sqlite3BtreeCommitPhaseTwo(p->pDest, 0))
        ){
          rc = SQLITE_DONE;
        }
      }
    }
  
    /* If bCloseTrans is true, then this function opened a read transaction
    ** on the source database. Close the read transaction here. There is
    ** no need to check the return values of the btree methods here, as
    ** "committing" a read-only transaction cannot fail.
    */
    ប្រសិនបើ( bCloseTrans ){
      TESTONLY( ចំនួនគត់ rc2 );
      TESTONLY( rc2  = ) sqlite3BtreeCommitPhaseOne(p->pSrc, 0);
      TESTONLY( rc2 |= ) sqlite3BtreeCommitPhaseTwo(p->pSrc, 0);
      assert( rc2==SQLITE_OK );
    }
  
    ប្រសិនបើ( rc==SQLITE_IOERR_NOMEM ){
      rc = SQLITE_NOMEM_BKPT;
    }
    p->rc = rc;
  }
  ប្រសិនបើ( p->pDestDb ){
    sqlite3_mutex_leave(p->pDestDb->mutex);
  }
  sqlite3BtreeLeave(p->pSrc);
  sqlite3_mutex_leave(p->pSrcDb->mutex);
  ត្រឡប់ rc;
}

/*
** Release all resources associated with an sqlite3_backup* handle.
*/
ចំនួនគត់ sqlite3_backup_finish(sqlite3_backup *p){
  sqlite3_backup **pp;                 /* Ptr to head of pagers backup list */
  sqlite3 *pSrcDb;                     /* Source database connection */
  ចំនួនគត់ rc;                              /* Value to return */

  /* Enter the mutexes */
  ប្រសិនបើ( p==0 ) ត្រឡប់ SQLITE_OK;
  pSrcDb = p->pSrcDb;
  sqlite3_mutex_enter(pSrcDb->mutex);
  sqlite3BtreeEnter(p->pSrc);
  ប្រសិនបើ( p->pDestDb ){
    sqlite3_mutex_enter(p->pDestDb->mutex);
  }

  /* Detach this backup from the source pager. */
  ប្រសិនបើ( p->pDestDb ){
    p->pSrc->nBackup--;
  }
  ប្រសិនបើ( p->isAttached ){
    pp = sqlite3PagerBackupPtr(sqlite3BtreePager(p->pSrc));
    assert( pp!=0 );
    ខណៈពេល( *pp!=p ){
      pp = &(*pp)->pNext;
      assert( pp!=0 );
    }
    *pp = p->pNext;
  }

  /* If a transaction is still open on the Btree, roll it back. */
  ប្រសិនបើ( p->pDest ){
    sqlite3BtreeRollback(p->pDest, SQLITE_OK, 0);
  }

  /* Set the error code of the destination database handle. */
  rc = (p->rc==SQLITE_DONE) ? SQLITE_OK : p->rc;
  ប្រសិនបើ( p->pDestDb ){
    sqlite3Error(p->pDestDb, rc);

    /* Exit the mutexes and free the backup context structure. */
    sqlite3LeaveMutexAndCloseZombie(p->pDestDb);
  }
  sqlite3BtreeLeave(p->pSrc);
  ប្រសិនបើ( p->pDestDb ){
    /* EVIDENCE-OF: R-64852-21591 The sqlite3_backup object is created by a
    ** call to sqlite3_backup_init() and is destroyed by a call to
    ** sqlite3_backup_finish(). */
    sqlite3_free(p);
  }
  sqlite3LeaveMutexAndCloseZombie(pSrcDb);
  ត្រឡប់ rc;
}

/*
** Return the number of pages still to be backed up as of the most recent
** call to sqlite3_backup_step().
*/
ចំនួនគត់ sqlite3_backup_remaining(sqlite3_backup *p){
#ifdef SQLITE_ENABLE_API_ARMOR
  ប្រសិនបើ( p==0 ){
    (void)SQLITE_MISUSE_BKPT;
    ត្រឡប់ 0;
  }
#endif
  ត្រឡប់ p->nRemaining;
}

/*
** Return the total number of pages in the source database as of the most 
** recent call to sqlite3_backup_step().
*/
ចំនួនគត់ sqlite3_backup_pagecount(sqlite3_backup *p){
#ifdef SQLITE_ENABLE_API_ARMOR
  ប្រសិនបើ( p==0 ){
    (void)SQLITE_MISUSE_BKPT;
    ត្រឡប់ 0;
  }
#endif
  ត្រឡប់ p->nPagecount;
}

/*
** This function is called after the contents of page iPage of the
** source database have been modified. If page iPage has already been 
** copied into the destination database, then the data written to the
** destination is now invalidated. The destination copy of iPage needs
** to be updated with the new data before the backup operation is
** complete.
**
** It is assumed that the mutex associated with the BtShared object
** corresponding to the source database is held when this function is
** called.
*/
static SQLITE_NOINLINE void backupUpdate(
  sqlite3_backup *p,
  Pgno iPage,
  const u8 *aData
){
  assert( p!=0 );
  do{
    assert( sqlite3_mutex_held(p->pSrc->pBt->mutex) );
    ប្រសិនបើ( !isFatalError(p->rc) && iPage<p->iNext ){
      /* The backup process p has already copied page iPage. But now it
      ** has been modified by a transaction on the source pager. Copy
      ** the new data into the backup.
      */
      ចំនួនគត់ rc;
      assert( p->pDestDb );
      sqlite3_mutex_enter(p->pDestDb->mutex);
      rc = backupOnePage(p, iPage, aData, 1);
      sqlite3_mutex_leave(p->pDestDb->mutex);
      assert( rc!=SQLITE_BUSY && rc!=SQLITE_LOCKED );
      ប្រសិនបើ( rc!=SQLITE_OK ){
        p->rc = rc;
      }
    }
  }ខណៈពេល( (p = p->pNext)!=0 );
}
void sqlite3BackupUpdate(sqlite3_backup *pBackup, Pgno iPage, const u8 *aData){
  ប្រសិនបើ( pBackup ) backupUpdate(pBackup, iPage, aData);
}

/*
** Restart the backup process. This is called when the pager layer
** detects that the database has been modified by an external database
** connection. In this case there is no way of knowing which of the
** pages that have been copied into the destination database are still 
** valid and which are not, so the entire process needs to be restarted.
**
** It is assumed that the mutex associated with the BtShared object
** corresponding to the source database is held when this function is
** called.
*/
void sqlite3BackupRestart(sqlite3_backup *pBackup){
  sqlite3_backup *p;                   /* Iterator variable */
  សម្រាប់(p=pBackup; p; p=p->pNext){
    assert( sqlite3_mutex_held(p->pSrc->pBt->mutex) );
    p->iNext = 1;
  }
}

#ifndef SQLITE_OMIT_VACUUM
/*
** Copy the complete content of pBtFrom into pBtTo.  A transaction
** must be active for both files.
**
** The size of file pTo may be reduced by this operation. If anything 
** goes wrong, the transaction on pTo is rolled back. If successful, the 
** transaction is committed before returning.
*/
ចំនួនគត់ sqlite3BtreeCopyFile(Btree *pTo, Btree *pFrom){
  ចំនួនគត់ rc;
  sqlite3_file *pFd;              /* File descriptor for database pTo */
  sqlite3_backup b;
  sqlite3BtreeEnter(pTo);
  sqlite3BtreeEnter(pFrom);

  assert( sqlite3BtreeTxnState(pTo)==SQLITE_TXN_WRITE );
  pFd = sqlite3PagerFile(sqlite3BtreePager(pTo));
  ប្រសិនបើ( pFd->pMethods ){
    i64 nByte = sqlite3BtreeGetPageSize(pFrom)*(i64)sqlite3BtreeLastPage(pFrom);
    rc = sqlite3OsFileControl(pFd, SQLITE_FCNTL_OVERWRITE, &nByte);
    ប្រសិនបើ( rc==SQLITE_NOTFOUND ) rc = SQLITE_OK;
    ប្រសិនបើ( rc ) goto copy_finished;
  }

  /* Set up an sqlite3_backup object. sqlite3_backup.pDestDb must be set
  ** to 0. This is used by the implementations of sqlite3_backup_step()
  ** and sqlite3_backup_finish() to detect that they are being called
  ** from this function, not directly by the user.
  */
  memset(&b, 0, parimaa_na(b));
  b.pSrcDb = pFrom->db;
  b.pSrc = pFrom;
  b.pDest = pTo;
  b.iNext = 1;

  /* 0x7FFFFFFF is the hard limit for the number of pages in a database
  ** file. By passing this as the number of pages to copy to
  ** sqlite3_backup_step(), we can guarantee that the copy finishes 
  ** within a single call (unless an error occurs). The assert() statement
  ** checks this assumption - (p->rc) should be set to either SQLITE_DONE 
  ** or an error code.  */
  sqlite3_backup_step(&b, 0x7FFFFFFF);
  assert( b.rc!=SQLITE_OK );

  rc = sqlite3_backup_finish(&b);
  ប្រសិនបើ( rc==SQLITE_OK ){
    pTo->pBt->btsFlags &= ~BTS_PAGESIZE_FIXED;
  }បើមិនដូច្នេះ{
    sqlite3PagerClearCache(sqlite3BtreePager(b.pDest));
  }

  assert( sqlite3BtreeTxnState(pTo)!=SQLITE_TXN_WRITE );
copy_finished:
  sqlite3BtreeLeave(pFrom);
  sqlite3BtreeLeave(pTo);
  ត្រឡប់ rc;
}
#endif /* SQLITE_OMIT_VACUUM */
