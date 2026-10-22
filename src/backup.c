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
#ضمن "sqliteInt.h"
#ضمن "btreeInt.h"

/*
** Structure allocated for each backup operation.
*/
بنية sqlite3_backup {
  sqlite3* pDestDb;        /* Destination database handle */
  حرف *zDestDb;
  Btree *pDest;            /* Destination b-tree file */
  u32 iDestSchema;         /* Original schema cookie in destination */
  صحيح bDestLocked;         /* True once a write-transaction is open on pDest */

  Pgno i_ايندڙ;              /* Page number of the next source page to copy */
  sqlite3* pSrcDb;         /* Source database handle */
  Btree *pSrc;             /* Source b-tree file */

  صحيح rc;                  /* Backup process error code */

  /* These two variables are set by every call to backup_step(). They are
  ** read by calls to backup_remaining() and backup_pagecount().
  */
  Pgno n_بچيل;         /* Number of pages left to copy */
  Pgno nPagecount;         /* Total number of pages to copy */

  صحيح isAttached;          /* True once backup has been registered with pager */
  sqlite3_backup *p_ايندڙ;   /* Next backup associated with source pager */
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
ساكن Btree *ڳوليو_Btree(sqlite3 *pErrorDb, sqlite3 *pDb, ثابت حرف *zDb){
  صحيح i;

  أكد( pDb!=0 );
  i = sqlite3_ڳوليو_Db_Name(pDb, zDb);
  إذا( i==1 ){
    Parse sParse;
    صحيح rc = 0;
    sqlite3ParseObjectInit(&sParse,pDb);
    إذا( sqlite3_Open_عارضي_Database(&sParse) ){
      sqlite3_Error_With_نياپو(pErrorDb, sParse.rc, "%s", sParse.z_Err_نياپو);
      rc = SQLITE_ERROR;
    }
    sqlite3DbFree(pErrorDb, sParse.z_Err_نياپو);
    sqlite3ParseObjectReset(&sParse);
    إذا( rc ){
      أرجع 0;
    }
  }

  إذا( i<0 ){
    sqlite3_Error_With_نياپو(pErrorDb, SQLITE_ERROR, "unknown database %s", zDb);
    أرجع 0;
  }

  أرجع pDb->aDb[i].pBt;
}

/*
** Attempt to set the page size of the destination to match the page size
** of the source.
*/
ساكن صحيح سيٽ_Dest_Pgsz(Btree *pDest, Btree *pSrc){
  أرجع sqlite3_Btree_سيٽ_صفحو_سائيز(pDest, sqlite3_Btree_Get_صفحو_سائيز(pSrc), 0, 0);
}

/*
** Check that there is no open read-transaction on the b-tree passed as the
** second argument. If there is not, return SQLITE_OK. Otherwise, if there
** is an open read-transaction, return SQLITE_ERROR and leave an error 
** message in database handle db.
*/
ساكن صحيح checkReadTransaction(sqlite3 *db, Btree *p){
  إذا( sqlite3BtreeTxnState(p)!=SQLITE_TXN_NONE ){
    sqlite3_Error_With_نياپو(db, SQLITE_ERROR, "destination database is in use");
    أرجع SQLITE_ERROR;
  }
  أرجع SQLITE_OK;
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
  ثابت حرف *zDestDb,                  /* Name of database within pDestDb */
  sqlite3* pSrcDb,                      /* Database connection to read from */
  ثابت حرف *zSrcDb                    /* Name of database within pSrcDb */
){
  sqlite3_backup *p;                    /* Value to return */

#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_API_ARMOR
  إذا( !sqlite3SafetyCheckOk(pSrcDb)||!sqlite3SafetyCheckOk(pDestDb) ){
    (فراغ)SQLITE_MISUSE_BKPT;
    أرجع 0;
  }
#جيڪڏهن_پورو

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

  إذا( pSrcDb==pDestDb ){
    sqlite3_Error_With_نياپو(
        pDestDb, SQLITE_ERROR, "source and destination must be distinct"
    );
    p = 0;
  }وإلا {
    صحيح nDest = sqlite3Strlen30(zDestDb);

    /* Allocate space for a new sqlite3_backup object...
    ** EVIDENCE-OF: R-64852-21591 The sqlite3_backup object is created by a
    ** call to sqlite3_backup_init() and is destroyed by a call to
    ** sqlite3_backup_finish(). */
    p = (sqlite3_backup*)sqlite3MallocZero(حجم(sqlite3_backup)+nDest+1);
    إذا( !p ){
      sqlite3Error(pDestDb, SQLITE_NOMEM_BKPT);
    }وإلا{
      p->zDestDb = (حرف*)&p[1];
      يادگيري_نقل(p->zDestDb, zDestDb, nDest);
    }
  }

  /* If the allocation succeeded, populate the new object. */
  إذا( p ){
    /* Do not store the pointer to the destination b-tree at this point.
    ** This is because there is nothing preventing it from being detached
    ** or otherwise freed before the first call to sqlite3_backup_step()
    ** on this object. The source b-tree does not have this problem, as
    ** incrementing Btree.nBackup (see below) effectively locks the object. */
    Btree *pDest = ڳوليو_Btree(pDestDb, pDestDb, zDestDb);
    p->pSrc = ڳوليو_Btree(pDestDb, pSrcDb, zSrcDb);
    p->pDestDb = pDestDb;
    p->pSrcDb = pSrcDb;
    p->i_ايندڙ = 1;
    p->isAttached = 0;

    إذا( 0==p->pSrc || 0==pDest 
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
  إذا( p ){
    p->pSrc->nBackup++;
  }

  sqlite3_mutex_leave(pDestDb->mutex);
  sqlite3_mutex_leave(pSrcDb->mutex);
  أرجع p;
}

/*
** Argument rc is an SQLite error code. Return true if this error is 
** considered fatal if encountered during a backup operation. All errors
** are considered fatal except for SQLITE_BUSY and SQLITE_LOCKED.
*/
ساكن صحيح isFatalError(صحيح rc){
  أرجع (rc!=SQLITE_OK && rc!=SQLITE_BUSY && ALWAYS(rc!=SQLITE_LOCKED));
}

/*
** Parameter zSrcData points to a buffer containing the data for 
** page iSrcPg from the source database. Copy this data into the 
** destination database.
*/
ساكن صحيح backup_One_صفحو(
  sqlite3_backup *p,              /* Backup handle */
  Pgno iSrcPg,                    /* Source database page to backup */
  ثابت u8 *z_Src_معلومات,             /* Source database page data */
  صحيح bUpdate                     /* True for an update, false otherwise */
){
  Pager * ثابت pDestPager = sqlite3BtreePager(p->pDest);
  ثابت صحيح nSrcPgsz = sqlite3_Btree_Get_صفحو_سائيز(p->pSrc);
  صحيح nDestPgsz = sqlite3_Btree_Get_صفحو_سائيز(p->pDest);
  ثابت صحيح nCopy = MIN(nSrcPgsz, nDestPgsz);
  ثابت i64 i_پڇاڙي = (i64)iSrcPg*(i64)nSrcPgsz;
  صحيح rc = SQLITE_OK;
  i64 iOff;

  أكد( sqlite3BtreeGetReserveNoMutex(p->pSrc)>=0 );
  أكد( p->bDestLocked );
  أكد( !isFatalError(p->rc) );
  أكد( iSrcPg!=PENDING_BYTE_صفحو(p->pSrc->pBt) );
  أكد( z_Src_معلومات );
  أكد( nSrcPgsz==nDestPgsz || sqlite3PagerIsMemdb(pDestPager)==0 );

  /* This loop runs once for each destination page spanned by the source 
  ** page. For each iteration, variable iOff is set to the byte offset
  ** of the destination page.
  */
  لكل(iOff=i_پڇاڙي-(i64)nSrcPgsz; rc==SQLITE_OK && iOff<i_پڇاڙي; iOff+=nDestPgsz){
    Db_صفحو *pDestPg = 0;
    Pgno iDest = (Pgno)(iOff/nDestPgsz)+1;
    إذا( iDest==PENDING_BYTE_صفحو(p->pDest->pBt) ) استمر;
    إذا( SQLITE_OK==(rc = sqlite3PagerGet(pDestPager, iDest, &pDestPg, 0))
     && SQLITE_OK==(rc = sqlite3PagerWrite(pDestPg))
    ){
      ثابت u8 *z_اندريون = &z_Src_معلومات[iOff%nSrcPgsz];
      u8 *z_Dest_معلومات = sqlite3_Pager_Get_معلومات(pDestPg);
      u8 *z_ٻاهريون = &z_Dest_معلومات[iOff%nDestPgsz];

      /* Copy the data from the source page into the destination page.
      ** Then clear the Btree layer MemPage.isInit flag. Both this module
      ** and the pager code use this trick (clearing the first byte
      ** of the page 'extra' space to invalidate the Btree layers
      ** cached parse of the page). MemPage.isInit is marked 
      ** "MUST BE FIRST" for this purpose.
      */
      يادگيري_نقل(z_ٻاهريون, z_اندريون, nCopy);
      ((u8 *)sqlite3PagerGetExtra(pDestPg))[0] = 0;
      إذا( iOff==0 && bUpdate==0 ){
        sqlite3Put4byte(&z_ٻاهريون[28], sqlite3_Btree_آخري_صفحو(p->pSrc));
      }
    }
    sqlite3PagerUnref(pDestPg);
  }

  أرجع rc;
}

/*
** If pFile is currently larger than iSize bytes, then truncate it to
** exactly iSize bytes. If pFile is not larger than iSize bytes, then
** this function is a no-op.
**
** Return SQLITE_OK if everything is successful, or an SQLite error 
** code if an error occurs.
*/
ساكن صحيح backupTruncateFile(sqlite3_file *pFile, i64 i_سائيز){
  i64 iCurrent;
  صحيح rc = sqlite3_Os_File_سائيز(pFile, &iCurrent);
  إذا( rc==SQLITE_OK && iCurrent>i_سائيز ){
    rc = sqlite3OsTruncate(pFile, i_سائيز);
  }
  أرجع rc;
}

/*
** Register this backup object with the associated source pager for
** callbacks when pages are changed or the cache invalidated.
*/
ساكن فراغ attachBackupObject(sqlite3_backup *p){
  sqlite3_backup **pp;
  أكد( sqlite3BtreeHoldsMutex(p->pSrc) );
  pp = sqlite3PagerBackupPtr(sqlite3BtreePager(p->pSrc));
  p->p_ايندڙ = *pp;
  *pp = p;
  p->isAttached = 1;
}

/*
** Copy nPage pages from the source b-tree to the destination.
*/
صحيح sqlite3_backup_قدم(sqlite3_backup *p, صحيح n_صفحو){
  صحيح rc;
  صحيح dest_گهڻائي = 0;   /* Destination journal mode */
  صحيح pgszSrc = 0;    /* Source page size */
  صحيح pgszDest = 0;   /* Destination page size */

#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_API_ARMOR
  إذا( p==0 ) أرجع SQLITE_MISUSE_BKPT;
#جيڪڏهن_پورو
  sqlite3_mutex_enter(p->pSrcDb->mutex);
  sqlite3BtreeEnter(p->pSrc);
  إذا( p->pDestDb ){
    sqlite3_mutex_enter(p->pDestDb->mutex);
  }

  rc = p->rc;
  إذا( !isFatalError(rc) ){
    Pager * ثابت pSrcPager = sqlite3BtreePager(p->pSrc);     /* Source pager */
    Btree * pDest = 0;                                        /* Dest btree */
    Pager * pDestPager = 0;                                   /* Dest pager */
    صحيح ii;                            /* Iterator variable */
    صحيح n_Src_صفحو = -1;                 /* Size of source db in pages */
    صحيح bCloseTrans = 0;               /* True if src db requires unlocking */

    /* If the source pager is currently in a write-transaction, return
    ** SQLITE_BUSY immediately.
    */
    إذا( p->pDestDb && p->pSrc->pBt->اندريون_Transaction==TRANS_WRITE ){
      rc = SQLITE_BUSY;
    }وإلا{
      rc = SQLITE_OK;
    }


    /* If there is no open read-transaction on the source database, open
    ** one now. If a transaction is opened here, then it will be closed
    ** before this function exits.
    */
    إذا( rc==SQLITE_OK && SQLITE_TXN_NONE==sqlite3BtreeTxnState(p->pSrc) ){
      rc = sqlite3BtreeBeginTrans(p->pSrc, 0, 0);
      bCloseTrans = 1;
    }

    /* Locate the destination btree and pager. */
    إذا( (pDest = p->pDest)==0 ){
      pDest = ڳوليو_Btree(p->pDestDb, p->pDestDb, p->zDestDb);
    }
    إذا( pDest==0 ){
      rc = SQLITE_ERROR;
    }وإلا{
      pDestPager = sqlite3BtreePager(pDest);
    }

    /* If the destination database has not yet been locked (i.e. if this
    ** is the first call to backup_step() for the current backup operation),
    ** try to set its page size to the same as the source database. This
    ** is especially important on ZipVFS systems, as in that case it is
    ** not possible to create a database file that uses one page size by
    ** writing to it with another.  */
    إذا( p->bDestLocked==0 && rc==SQLITE_OK 
     && سيٽ_Dest_Pgsz(pDest, p->pSrc)==SQLITE_NOMEM
    ){
      rc = SQLITE_NOMEM;
    }

    /* Lock the destination database, if it is not locked already. */
    إذا( SQLITE_OK==rc && p->bDestLocked==0
     && SQLITE_OK==(rc = sqlite3BtreeBeginTrans(pDest, 2,
                                                (صحيح*)&p->iDestSchema)) 
    ){
      p->bDestLocked = 1;
      p->pDest = pDest;
    }

    /* Do not allow backup if the destination database is in WAL mode
    ** and the page sizes are different between source and destination */
    إذا( rc==SQLITE_OK ){
      pgszSrc = sqlite3_Btree_Get_صفحو_سائيز(p->pSrc);
      pgszDest = sqlite3_Btree_Get_صفحو_سائيز(p->pDest);
      dest_گهڻائي = sqlite3_Pager_Get_Journal_گهڻائي(sqlite3BtreePager(p->pDest));
      إذا( (dest_گهڻائي==PAGER_JOURNALMODE_WAL || sqlite3PagerIsMemdb(pDestPager))
        && pgszSrc!=pgszDest 
      ){
        rc = SQLITE_READONLY;
      }
    }
  
    /* Now that there is a read-lock on the source database, query the
    ** source pager for the number of pages in the database.
    */
    n_Src_صفحو = (صحيح)sqlite3_Btree_آخري_صفحو(p->pSrc);
    أكد( n_Src_صفحو>=0 );
    لكل(ii=0; (n_صفحو<0 || ii<n_صفحو) && p->i_ايندڙ<=(Pgno)n_Src_صفحو && !rc; ii++){
      ثابت Pgno iSrcPg = p->i_ايندڙ;                 /* Source page number */
      إذا( iSrcPg!=PENDING_BYTE_صفحو(p->pSrc->pBt) ){
        Db_صفحو *pSrcPg;                             /* Source page object */
        rc = sqlite3PagerGet(pSrcPager, iSrcPg, &pSrcPg,PAGER_GET_READONLY);
        إذا( rc==SQLITE_OK ){
          rc = backup_One_صفحو(p, iSrcPg, sqlite3_Pager_Get_معلومات(pSrcPg), 0);
          sqlite3PagerUnref(pSrcPg);
        }
      }
      p->i_ايندڙ++;
    }
    إذا( rc==SQLITE_OK ){
      p->nPagecount = n_Src_صفحو;
      p->n_بچيل = n_Src_صفحو+1-p->i_ايندڙ;
      إذا( p->i_ايندڙ>(Pgno)n_Src_صفحو ){
        rc = SQLITE_پورو;
      }وإلا إذا( !p->isAttached ){
        attachBackupObject(p);
      }
    }
  
    /* Update the schema version field in the destination database. This
    ** is to make sure that the schema-version really does change in
    ** the case where the source and destination databases have the
    ** same schema version.
    */
    إذا( rc==SQLITE_پورو ){
      إذا( n_Src_صفحو==0 ){
        rc = sqlite3BtreeNewDb(p->pDest);
        n_Src_صفحو = 1;
      }
      إذا( rc==SQLITE_OK || rc==SQLITE_پورو ){
        rc = sqlite3BtreeUpdateMeta(p->pDest,1,p->iDestSchema+1);
      }
      إذا( rc==SQLITE_OK ){
        إذا( p->pDestDb ){
          sqlite3_Reset_All_Schemas_جو_Connection(p->pDestDb);
        }
        إذا( dest_گهڻائي==PAGER_JOURNALMODE_WAL ){
          rc = sqlite3_Btree_سيٽ_نسخو(p->pDest, 2);
        }
      }
      إذا( rc==SQLITE_OK ){
        صحيح nDestTruncate;
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
        أكد( pgszSrc==sqlite3_Btree_Get_صفحو_سائيز(p->pSrc) );
        أكد( pgszDest==sqlite3_Btree_Get_صفحو_سائيز(p->pDest) );
        إذا( pgszSrc<pgszDest ){
          صحيح تناسب = pgszDest/pgszSrc;
          nDestTruncate = (n_Src_صفحو+تناسب-1)/تناسب;
          إذا( nDestTruncate==(صحيح)PENDING_BYTE_صفحو(p->pDest->pBt) ){
            nDestTruncate--;
          }
        }وإلا{
          nDestTruncate = n_Src_صفحو * (pgszSrc/pgszDest);
        }
        أكد( nDestTruncate>0 );

        إذا( pgszSrc<pgszDest ){
          /* If the source page-size is smaller than the destination page-size,
          ** two extra things may need to happen:
          **
          **   * The destination may need to be truncated, and
          **
          **   * Data stored on the pages immediately following the 
          **     pending-byte page in the source database may need to be
          **     copied into the destination database.
          */
          ثابت i64 i_سائيز = (i64)pgszSrc * (i64)n_Src_صفحو;
          sqlite3_file * ثابت pFile = sqlite3PagerFile(pDestPager);
          Pgno iPg;
          صحيح n_Dst_صفحو;
          i64 iOff;
          i64 i_پڇاڙي;

          أكد( pFile );
          أكد( nDestTruncate==0 
              || (i64)nDestTruncate*(i64)pgszDest >= i_سائيز || (
                nDestTruncate==(صحيح)(PENDING_BYTE_صفحو(p->pDest->pBt)-1)
             && i_سائيز>=PENDING_BYTE && i_سائيز<=PENDING_BYTE+pgszDest
          ));

          /* This block ensures that all data required to recreate the original
          ** database has been stored in the journal for pDestPager and the
          ** journal synced to disk. So at this point we may safely modify
          ** the database file in any way, knowing that if a power failure
          ** occurs, the original database will be reconstructed from the 
          ** journal file.  */
          sqlite3PagerPagecount(pDestPager, &n_Dst_صفحو);
          لكل(iPg=nDestTruncate; rc==SQLITE_OK && iPg<=(Pgno)n_Dst_صفحو; iPg++){
            إذا( iPg!=PENDING_BYTE_صفحو(p->pDest->pBt) ){
              Db_صفحو *pPg;
              rc = sqlite3PagerGet(pDestPager, iPg, &pPg, 0);
              إذا( rc==SQLITE_OK ){
                rc = sqlite3PagerWrite(pPg);
                sqlite3PagerUnref(pPg);
              }
            }
          }
          إذا( rc==SQLITE_OK ){
            rc = sqlite3PagerCommitPhaseOne(pDestPager, 0, 1);
          }

          /* Write the extra pages and truncate the database file as required */
          i_پڇاڙي = MIN(PENDING_BYTE + pgszDest, i_سائيز);
          لكل(
            iOff=PENDING_BYTE+pgszSrc; 
            rc==SQLITE_OK && iOff<i_پڇاڙي; 
            iOff+=pgszSrc
          ){
            PgHdr *pSrcPg = 0;
            ثابت Pgno iSrcPg = (Pgno)((iOff/pgszSrc)+1);
            rc = sqlite3PagerGet(pSrcPager, iSrcPg, &pSrcPg, 0);
            إذا( rc==SQLITE_OK ){
              u8 *z_معلومات = sqlite3_Pager_Get_معلومات(pSrcPg);
              rc = sqlite3OsWrite(pFile, z_معلومات, pgszSrc, iOff);
            }
            sqlite3PagerUnref(pSrcPg);
          }
          إذا( rc==SQLITE_OK ){
            rc = backupTruncateFile(pFile, i_سائيز);
          }

          /* Sync the database file to disk. */
          إذا( rc==SQLITE_OK ){
            rc = sqlite3PagerSync(pDestPager, 0);
          }
        }وإلا{
          sqlite3PagerTruncateImage(pDestPager, nDestTruncate);
          rc = sqlite3PagerCommitPhaseOne(pDestPager, 0, 0);
        }
    
        /* Finish committing the transaction to the destination database. */
        إذا( SQLITE_OK==rc
         && SQLITE_OK==(rc = sqlite3BtreeCommitPhaseTwo(p->pDest, 0))
        ){
          rc = SQLITE_پورو;
        }
      }
    }
  
    /* If bCloseTrans is true, then this function opened a read transaction
    ** on the source database. Close the read transaction here. There is
    ** no need to check the return values of the btree methods here, as
    ** "committing" a read-only transaction cannot fail.
    */
    إذا( bCloseTrans ){
      TESTONLY( صحيح rc2 );
      TESTONLY( rc2  = ) sqlite3BtreeCommitPhaseOne(p->pSrc, 0);
      TESTONLY( rc2 |= ) sqlite3BtreeCommitPhaseTwo(p->pSrc, 0);
      أكد( rc2==SQLITE_OK );
    }
  
    إذا( rc==SQLITE_IOERR_NOMEM ){
      rc = SQLITE_NOMEM_BKPT;
    }
    p->rc = rc;
  }
  إذا( p->pDestDb ){
    sqlite3_mutex_leave(p->pDestDb->mutex);
  }
  sqlite3BtreeLeave(p->pSrc);
  sqlite3_mutex_leave(p->pSrcDb->mutex);
  أرجع rc;
}

/*
** Release all resources associated with an sqlite3_backup* handle.
*/
صحيح sqlite3_backup_پڄاڻي(sqlite3_backup *p){
  sqlite3_backup **pp;                 /* Ptr to head of pagers backup list */
  sqlite3 *pSrcDb;                     /* Source database connection */
  صحيح rc;                              /* Value to return */

  /* Enter the mutexes */
  إذا( p==0 ) أرجع SQLITE_OK;
  pSrcDb = p->pSrcDb;
  sqlite3_mutex_enter(pSrcDb->mutex);
  sqlite3BtreeEnter(p->pSrc);
  إذا( p->pDestDb ){
    sqlite3_mutex_enter(p->pDestDb->mutex);
  }

  /* Detach this backup from the source pager. */
  إذا( p->pDestDb ){
    p->pSrc->nBackup--;
  }
  إذا( p->isAttached ){
    pp = sqlite3PagerBackupPtr(sqlite3BtreePager(p->pSrc));
    أكد( pp!=0 );
    طالما( *pp!=p ){
      pp = &(*pp)->p_ايندڙ;
      أكد( pp!=0 );
    }
    *pp = p->p_ايندڙ;
  }

  /* If a transaction is still open on the Btree, roll it back. */
  إذا( p->pDest ){
    sqlite3BtreeRollback(p->pDest, SQLITE_OK, 0);
  }

  /* Set the error code of the destination database handle. */
  rc = (p->rc==SQLITE_پورو) ? SQLITE_OK : p->rc;
  إذا( p->pDestDb ){
    sqlite3Error(p->pDestDb, rc);

    /* Exit the mutexes and free the backup context structure. */
    sqlite3LeaveMutexAndCloseZombie(p->pDestDb);
  }
  sqlite3BtreeLeave(p->pSrc);
  إذا( p->pDestDb ){
    /* EVIDENCE-OF: R-64852-21591 The sqlite3_backup object is created by a
    ** call to sqlite3_backup_init() and is destroyed by a call to
    ** sqlite3_backup_finish(). */
    sqlite3_free(p);
  }
  sqlite3LeaveMutexAndCloseZombie(pSrcDb);
  أرجع rc;
}

/*
** Return the number of pages still to be backed up as of the most recent
** call to sqlite3_backup_step().
*/
صحيح sqlite3_backup_بچيل(sqlite3_backup *p){
#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_API_ARMOR
  إذا( p==0 ){
    (فراغ)SQLITE_MISUSE_BKPT;
    أرجع 0;
  }
#جيڪڏهن_پورو
  أرجع p->n_بچيل;
}

/*
** Return the total number of pages in the source database as of the most 
** recent call to sqlite3_backup_step().
*/
صحيح sqlite3_backup_pagecount(sqlite3_backup *p){
#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_API_ARMOR
  إذا( p==0 ){
    (فراغ)SQLITE_MISUSE_BKPT;
    أرجع 0;
  }
#جيڪڏهن_پورو
  أرجع p->nPagecount;
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
ساكن SQLITE_NOINLINE فراغ backupUpdate(
  sqlite3_backup *p,
  Pgno i_صفحو,
  ثابت u8 *a_معلومات
){
  أكد( p!=0 );
  افعل{
    أكد( sqlite3_mutex_held(p->pSrc->pBt->mutex) );
    إذا( !isFatalError(p->rc) && i_صفحو<p->i_ايندڙ ){
      /* The backup process p has already copied page iPage. But now it
      ** has been modified by a transaction on the source pager. Copy
      ** the new data into the backup.
      */
      صحيح rc;
      أكد( p->pDestDb );
      sqlite3_mutex_enter(p->pDestDb->mutex);
      rc = backup_One_صفحو(p, i_صفحو, a_معلومات, 1);
      sqlite3_mutex_leave(p->pDestDb->mutex);
      أكد( rc!=SQLITE_BUSY && rc!=SQLITE_LOCKED );
      إذا( rc!=SQLITE_OK ){
        p->rc = rc;
      }
    }
  }طالما( (p = p->p_ايندڙ)!=0 );
}
فراغ sqlite3BackupUpdate(sqlite3_backup *pBackup, Pgno i_صفحو, ثابت u8 *a_معلومات){
  إذا( pBackup ) backupUpdate(pBackup, i_صفحو, a_معلومات);
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
فراغ sqlite3BackupRestart(sqlite3_backup *pBackup){
  sqlite3_backup *p;                   /* Iterator variable */
  لكل(p=pBackup; p; p=p->p_ايندڙ){
    أكد( sqlite3_mutex_held(p->pSrc->pBt->mutex) );
    p->i_ايندڙ = 1;
  }
}

#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_VACUUM
/*
** Copy the complete content of pBtFrom into pBtTo.  A transaction
** must be active for both files.
**
** The size of file pTo may be reduced by this operation. If anything 
** goes wrong, the transaction on pTo is rolled back. If successful, the 
** transaction is committed before returning.
*/
صحيح sqlite3BtreeCopyFile(Btree *p_منزل, Btree *pFrom){
  صحيح rc;
  sqlite3_file *pFd;              /* File descriptor for database pTo */
  sqlite3_backup b;
  sqlite3BtreeEnter(p_منزل);
  sqlite3BtreeEnter(pFrom);

  أكد( sqlite3BtreeTxnState(p_منزل)==SQLITE_TXN_WRITE );
  pFd = sqlite3PagerFile(sqlite3BtreePager(p_منزل));
  إذا( pFd->pMethods ){
    i64 nByte = sqlite3_Btree_Get_صفحو_سائيز(pFrom)*(i64)sqlite3_Btree_آخري_صفحو(pFrom);
    rc = sqlite3OsFileControl(pFd, SQLITE_FCNTL_OVERWRITE, &nByte);
    إذا( rc==SQLITE_NOTFOUND ) rc = SQLITE_OK;
    إذا( rc ) اذهب copy_ختم;
  }

  /* Set up an sqlite3_backup object. sqlite3_backup.pDestDb must be set
  ** to 0. This is used by the implementations of sqlite3_backup_step()
  ** and sqlite3_backup_finish() to detect that they are being called
  ** from this function, not directly by the user.
  */
  memset(&b, 0, حجم(b));
  b.pSrcDb = pFrom->db;
  b.pSrc = pFrom;
  b.pDest = p_منزل;
  b.i_ايندڙ = 1;

  /* 0x7FFFFFFF is the hard limit for the number of pages in a database
  ** file. By passing this as the number of pages to copy to
  ** sqlite3_backup_step(), we can guarantee that the copy finishes 
  ** within a single call (unless an error occurs). The assert() statement
  ** checks this assumption - (p->rc) should be set to either SQLITE_DONE 
  ** or an error code.  */
  sqlite3_backup_قدم(&b, 0x7FFFFFFF);
  أكد( b.rc!=SQLITE_OK );

  rc = sqlite3_backup_پڄاڻي(&b);
  إذا( rc==SQLITE_OK ){
    p_منزل->pBt->btsFlags &= ~BTS_PAGESIZE_FIXED;
  }وإلا{
    sqlite3PagerClearCache(sqlite3BtreePager(b.pDest));
  }

  أكد( sqlite3BtreeTxnState(p_منزل)!=SQLITE_TXN_WRITE );
copy_ختم:
  sqlite3BtreeLeave(pFrom);
  sqlite3BtreeLeave(p_منزل);
  أرجع rc;
}
#جيڪڏهن_پورو /* SQLITE_OMIT_VACUUM */
