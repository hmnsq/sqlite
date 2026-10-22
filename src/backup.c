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
#అన్తర్భావ "sqliteInt.h"
#అన్తర్భావ "btreeInt.h"

/*
** Structure allocated for each backup operation.
*/
నిర్మాణం sqlite3_backup {
  sqlite3* pDestDb;        /* Destination database handle */
  అక్షరం *zDestDb;
  Btree *pDest;            /* Destination b-tree file */
  u32 iDestSchema;         /* Original schema cookie in destination */
  పూర్ణాంకం bDestLocked;         /* True once a write-transaction is open on pDest */

  Pgno i_తదుపరి;              /* Page number of the next source page to copy */
  sqlite3* pSrcDb;         /* Source database handle */
  Btree *pSrc;             /* Source b-tree file */

  పూర్ణాంకం rc;                  /* Backup process error code */

  /* These two variables are set by every call to backup_step(). They are
  ** read by calls to backup_remaining() and backup_pagecount().
  */
  Pgno n_మిగిలిన;         /* Number of pages left to copy */
  Pgno nPagecount;         /* Total number of pages to copy */

  పూర్ణాంకం isAttached;          /* True once backup has been registered with pager */
  sqlite3_backup *p_తదుపరి;   /* Next backup associated with source pager */
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
స్థిర Btree *కనుగొను_Btree(sqlite3 *pErrorDb, sqlite3 *pDb, స్థిరరాశి అక్షరం *zDb){
  పూర్ణాంకం i;

  ప్రతిజ్ఞా( pDb!=0 );
  i = sqlite3_కనుగొను_Db_Name(pDb, zDb);
  ఒకవేళ( i==1 ){
    Parse sParse;
    పూర్ణాంకం rc = 0;
    sqlite3ParseObjectInit(&sParse,pDb);
    ఒకవేళ( sqlite3_Open_తాత్కాలిక_Database(&sParse) ){
      sqlite3_Error_With_సందేశం(pErrorDb, sParse.rc, "%s", sParse.z_Err_సందేశం);
      rc = SQLITE_ERROR;
    }
    sqlite3DbFree(pErrorDb, sParse.z_Err_సందేశం);
    sqlite3ParseObjectReset(&sParse);
    ఒకవేళ( rc ){
      తిరిగి 0;
    }
  }

  ఒకవేళ( i<0 ){
    sqlite3_Error_With_సందేశం(pErrorDb, SQLITE_ERROR, "unknown database %s", zDb);
    తిరిగి 0;
  }

  తిరిగి pDb->aDb[i].pBt;
}

/*
** Attempt to set the page size of the destination to match the page size
** of the source.
*/
స్థిర పూర్ణాంకం సమితి_Dest_Pgsz(Btree *pDest, Btree *pSrc){
  తిరిగి sqlite3_Btree_సమితి_పుట_కొలత(pDest, sqlite3_Btree_Get_పుట_కొలత(pSrc), 0, 0);
}

/*
** Check that there is no open read-transaction on the b-tree passed as the
** second argument. If there is not, return SQLITE_OK. Otherwise, if there
** is an open read-transaction, return SQLITE_ERROR and leave an error 
** message in database handle db.
*/
స్థిర పూర్ణాంకం checkReadTransaction(sqlite3 *db, Btree *p){
  ఒకవేళ( sqlite3BtreeTxnState(p)!=SQLITE_TXN_NONE ){
    sqlite3_Error_With_సందేశం(db, SQLITE_ERROR, "destination database is in use");
    తిరిగి SQLITE_ERROR;
  }
  తిరిగి SQLITE_OK;
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
  స్థిరరాశి అక్షరం *zDestDb,                  /* Name of database within pDestDb */
  sqlite3* pSrcDb,                      /* Database connection to read from */
  స్థిరరాశి అక్షరం *zSrcDb                    /* Name of database within pSrcDb */
){
  sqlite3_backup *p;                    /* Value to return */

#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !sqlite3SafetyCheckOk(pSrcDb)||!sqlite3SafetyCheckOk(pDestDb) ){
    (శూన్య)SQLITE_MISUSE_BKPT;
    తిరిగి 0;
  }
#అయితే_ముగింపు

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

  ఒకవేళ( pSrcDb==pDestDb ){
    sqlite3_Error_With_సందేశం(
        pDestDb, SQLITE_ERROR, "source and destination must be distinct"
    );
    p = 0;
  }లేకపోతే {
    పూర్ణాంకం nDest = sqlite3Strlen30(zDestDb);

    /* Allocate space for a new sqlite3_backup object...
    ** EVIDENCE-OF: R-64852-21591 The sqlite3_backup object is created by a
    ** call to sqlite3_backup_init() and is destroyed by a call to
    ** sqlite3_backup_finish(). */
    p = (sqlite3_backup*)sqlite3MallocZero(పరిమాణం(sqlite3_backup)+nDest+1);
    ఒకవేళ( !p ){
      sqlite3Error(pDestDb, SQLITE_NOMEM_BKPT);
    }లేకపోతే{
      p->zDestDb = (అక్షరం*)&p[1];
      జ్ఞాపకం_నకలు(p->zDestDb, zDestDb, nDest);
    }
  }

  /* If the allocation succeeded, populate the new object. */
  ఒకవేళ( p ){
    /* Do not store the pointer to the destination b-tree at this point.
    ** This is because there is nothing preventing it from being detached
    ** or otherwise freed before the first call to sqlite3_backup_step()
    ** on this object. The source b-tree does not have this problem, as
    ** incrementing Btree.nBackup (see below) effectively locks the object. */
    Btree *pDest = కనుగొను_Btree(pDestDb, pDestDb, zDestDb);
    p->pSrc = కనుగొను_Btree(pDestDb, pSrcDb, zSrcDb);
    p->pDestDb = pDestDb;
    p->pSrcDb = pSrcDb;
    p->i_తదుపరి = 1;
    p->isAttached = 0;

    ఒకవేళ( 0==p->pSrc || 0==pDest 
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
  ఒకవేళ( p ){
    p->pSrc->nBackup++;
  }

  sqlite3_mutex_leave(pDestDb->mutex);
  sqlite3_mutex_leave(pSrcDb->mutex);
  తిరిగి p;
}

/*
** Argument rc is an SQLite error code. Return true if this error is 
** considered fatal if encountered during a backup operation. All errors
** are considered fatal except for SQLITE_BUSY and SQLITE_LOCKED.
*/
స్థిర పూర్ణాంకం isFatalError(పూర్ణాంకం rc){
  తిరిగి (rc!=SQLITE_OK && rc!=SQLITE_BUSY && ALWAYS(rc!=SQLITE_LOCKED));
}

/*
** Parameter zSrcData points to a buffer containing the data for 
** page iSrcPg from the source database. Copy this data into the 
** destination database.
*/
స్థిర పూర్ణాంకం backup_One_పుట(
  sqlite3_backup *p,              /* Backup handle */
  Pgno iSrcPg,                    /* Source database page to backup */
  స్థిరరాశి u8 *z_Src_దత్తాంశం,             /* Source database page data */
  పూర్ణాంకం bUpdate                     /* True for an update, false otherwise */
){
  Pager * స్థిరరాశి pDestPager = sqlite3BtreePager(p->pDest);
  స్థిరరాశి పూర్ణాంకం nSrcPgsz = sqlite3_Btree_Get_పుట_కొలత(p->pSrc);
  పూర్ణాంకం nDestPgsz = sqlite3_Btree_Get_పుట_కొలత(p->pDest);
  స్థిరరాశి పూర్ణాంకం nCopy = MIN(nSrcPgsz, nDestPgsz);
  స్థిరరాశి i64 i_చివర = (i64)iSrcPg*(i64)nSrcPgsz;
  పూర్ణాంకం rc = SQLITE_OK;
  i64 iOff;

  ప్రతిజ్ఞా( sqlite3BtreeGetReserveNoMutex(p->pSrc)>=0 );
  ప్రతిజ్ఞా( p->bDestLocked );
  ప్రతిజ్ఞా( !isFatalError(p->rc) );
  ప్రతిజ్ఞా( iSrcPg!=PENDING_BYTE_పుట(p->pSrc->pBt) );
  ప్రతిజ్ఞా( z_Src_దత్తాంశం );
  ప్రతిజ్ఞా( nSrcPgsz==nDestPgsz || sqlite3PagerIsMemdb(pDestPager)==0 );

  /* This loop runs once for each destination page spanned by the source 
  ** page. For each iteration, variable iOff is set to the byte offset
  ** of the destination page.
  */
  కొరకు(iOff=i_చివర-(i64)nSrcPgsz; rc==SQLITE_OK && iOff<i_చివర; iOff+=nDestPgsz){
    Db_పుట *pDestPg = 0;
    Pgno iDest = (Pgno)(iOff/nDestPgsz)+1;
    ఒకవేళ( iDest==PENDING_BYTE_పుట(p->pDest->pBt) ) కొనసాగు;
    ఒకవేళ( SQLITE_OK==(rc = sqlite3PagerGet(pDestPager, iDest, &pDestPg, 0))
     && SQLITE_OK==(rc = sqlite3PagerWrite(pDestPg))
    ){
      స్థిరరాశి u8 *z_లోపల = &z_Src_దత్తాంశం[iOff%nSrcPgsz];
      u8 *z_Dest_దత్తాంశం = sqlite3_Pager_Get_దత్తాంశం(pDestPg);
      u8 *z_బహిర్గతం = &z_Dest_దత్తాంశం[iOff%nDestPgsz];

      /* Copy the data from the source page into the destination page.
      ** Then clear the Btree layer MemPage.isInit flag. Both this module
      ** and the pager code use this trick (clearing the first byte
      ** of the page 'extra' space to invalidate the Btree layers
      ** cached parse of the page). MemPage.isInit is marked 
      ** "MUST BE FIRST" for this purpose.
      */
      జ్ఞాపకం_నకలు(z_బహిర్గతం, z_లోపల, nCopy);
      ((u8 *)sqlite3PagerGetExtra(pDestPg))[0] = 0;
      ఒకవేళ( iOff==0 && bUpdate==0 ){
        sqlite3Put4byte(&z_బహిర్గతం[28], sqlite3_Btree_చివరి_పుట(p->pSrc));
      }
    }
    sqlite3PagerUnref(pDestPg);
  }

  తిరిగి rc;
}

/*
** If pFile is currently larger than iSize bytes, then truncate it to
** exactly iSize bytes. If pFile is not larger than iSize bytes, then
** this function is a no-op.
**
** Return SQLITE_OK if everything is successful, or an SQLite error 
** code if an error occurs.
*/
స్థిర పూర్ణాంకం backupTruncateFile(sqlite3_file *pFile, i64 i_కొలత){
  i64 iCurrent;
  పూర్ణాంకం rc = sqlite3_Os_File_కొలత(pFile, &iCurrent);
  ఒకవేళ( rc==SQLITE_OK && iCurrent>i_కొలత ){
    rc = sqlite3OsTruncate(pFile, i_కొలత);
  }
  తిరిగి rc;
}

/*
** Register this backup object with the associated source pager for
** callbacks when pages are changed or the cache invalidated.
*/
స్థిర శూన్య attachBackupObject(sqlite3_backup *p){
  sqlite3_backup **pp;
  ప్రతిజ్ఞా( sqlite3BtreeHoldsMutex(p->pSrc) );
  pp = sqlite3PagerBackupPtr(sqlite3BtreePager(p->pSrc));
  p->p_తదుపరి = *pp;
  *pp = p;
  p->isAttached = 1;
}

/*
** Copy nPage pages from the source b-tree to the destination.
*/
పూర్ణాంకం sqlite3_backup_అడుగు(sqlite3_backup *p, పూర్ణాంకం n_పుట){
  పూర్ణాంకం rc;
  పూర్ణాంకం dest_బహుళకం = 0;   /* Destination journal mode */
  పూర్ణాంకం pgszSrc = 0;    /* Source page size */
  పూర్ణాంకం pgszDest = 0;   /* Destination page size */

#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( p==0 ) తిరిగి SQLITE_MISUSE_BKPT;
#అయితే_ముగింపు
  sqlite3_mutex_enter(p->pSrcDb->mutex);
  sqlite3BtreeEnter(p->pSrc);
  ఒకవేళ( p->pDestDb ){
    sqlite3_mutex_enter(p->pDestDb->mutex);
  }

  rc = p->rc;
  ఒకవేళ( !isFatalError(rc) ){
    Pager * స్థిరరాశి pSrcPager = sqlite3BtreePager(p->pSrc);     /* Source pager */
    Btree * pDest = 0;                                        /* Dest btree */
    Pager * pDestPager = 0;                                   /* Dest pager */
    పూర్ణాంకం ii;                            /* Iterator variable */
    పూర్ణాంకం n_Src_పుట = -1;                 /* Size of source db in pages */
    పూర్ణాంకం bCloseTrans = 0;               /* True if src db requires unlocking */

    /* If the source pager is currently in a write-transaction, return
    ** SQLITE_BUSY immediately.
    */
    ఒకవేళ( p->pDestDb && p->pSrc->pBt->లోపల_Transaction==TRANS_WRITE ){
      rc = SQLITE_BUSY;
    }లేకపోతే{
      rc = SQLITE_OK;
    }


    /* If there is no open read-transaction on the source database, open
    ** one now. If a transaction is opened here, then it will be closed
    ** before this function exits.
    */
    ఒకవేళ( rc==SQLITE_OK && SQLITE_TXN_NONE==sqlite3BtreeTxnState(p->pSrc) ){
      rc = sqlite3BtreeBeginTrans(p->pSrc, 0, 0);
      bCloseTrans = 1;
    }

    /* Locate the destination btree and pager. */
    ఒకవేళ( (pDest = p->pDest)==0 ){
      pDest = కనుగొను_Btree(p->pDestDb, p->pDestDb, p->zDestDb);
    }
    ఒకవేళ( pDest==0 ){
      rc = SQLITE_ERROR;
    }లేకపోతే{
      pDestPager = sqlite3BtreePager(pDest);
    }

    /* If the destination database has not yet been locked (i.e. if this
    ** is the first call to backup_step() for the current backup operation),
    ** try to set its page size to the same as the source database. This
    ** is especially important on ZipVFS systems, as in that case it is
    ** not possible to create a database file that uses one page size by
    ** writing to it with another.  */
    ఒకవేళ( p->bDestLocked==0 && rc==SQLITE_OK 
     && సమితి_Dest_Pgsz(pDest, p->pSrc)==SQLITE_NOMEM
    ){
      rc = SQLITE_NOMEM;
    }

    /* Lock the destination database, if it is not locked already. */
    ఒకవేళ( SQLITE_OK==rc && p->bDestLocked==0
     && SQLITE_OK==(rc = sqlite3BtreeBeginTrans(pDest, 2,
                                                (పూర్ణాంకం*)&p->iDestSchema)) 
    ){
      p->bDestLocked = 1;
      p->pDest = pDest;
    }

    /* Do not allow backup if the destination database is in WAL mode
    ** and the page sizes are different between source and destination */
    ఒకవేళ( rc==SQLITE_OK ){
      pgszSrc = sqlite3_Btree_Get_పుట_కొలత(p->pSrc);
      pgszDest = sqlite3_Btree_Get_పుట_కొలత(p->pDest);
      dest_బహుళకం = sqlite3_Pager_Get_Journal_బహుళకం(sqlite3BtreePager(p->pDest));
      ఒకవేళ( (dest_బహుళకం==PAGER_JOURNALMODE_WAL || sqlite3PagerIsMemdb(pDestPager))
        && pgszSrc!=pgszDest 
      ){
        rc = SQLITE_READONLY;
      }
    }
  
    /* Now that there is a read-lock on the source database, query the
    ** source pager for the number of pages in the database.
    */
    n_Src_పుట = (పూర్ణాంకం)sqlite3_Btree_చివరి_పుట(p->pSrc);
    ప్రతిజ్ఞా( n_Src_పుట>=0 );
    కొరకు(ii=0; (n_పుట<0 || ii<n_పుట) && p->i_తదుపరి<=(Pgno)n_Src_పుట && !rc; ii++){
      స్థిరరాశి Pgno iSrcPg = p->i_తదుపరి;                 /* Source page number */
      ఒకవేళ( iSrcPg!=PENDING_BYTE_పుట(p->pSrc->pBt) ){
        Db_పుట *pSrcPg;                             /* Source page object */
        rc = sqlite3PagerGet(pSrcPager, iSrcPg, &pSrcPg,PAGER_GET_READONLY);
        ఒకవేళ( rc==SQLITE_OK ){
          rc = backup_One_పుట(p, iSrcPg, sqlite3_Pager_Get_దత్తాంశం(pSrcPg), 0);
          sqlite3PagerUnref(pSrcPg);
        }
      }
      p->i_తదుపరి++;
    }
    ఒకవేళ( rc==SQLITE_OK ){
      p->nPagecount = n_Src_పుట;
      p->n_మిగిలిన = n_Src_పుట+1-p->i_తదుపరి;
      ఒకవేళ( p->i_తదుపరి>(Pgno)n_Src_పుట ){
        rc = SQLITE_పూర్తి;
      }లేకపోతే ఒకవేళ( !p->isAttached ){
        attachBackupObject(p);
      }
    }
  
    /* Update the schema version field in the destination database. This
    ** is to make sure that the schema-version really does change in
    ** the case where the source and destination databases have the
    ** same schema version.
    */
    ఒకవేళ( rc==SQLITE_పూర్తి ){
      ఒకవేళ( n_Src_పుట==0 ){
        rc = sqlite3BtreeNewDb(p->pDest);
        n_Src_పుట = 1;
      }
      ఒకవేళ( rc==SQLITE_OK || rc==SQLITE_పూర్తి ){
        rc = sqlite3BtreeUpdateMeta(p->pDest,1,p->iDestSchema+1);
      }
      ఒకవేళ( rc==SQLITE_OK ){
        ఒకవేళ( p->pDestDb ){
          sqlite3_Reset_All_Schemas_యొక్క_Connection(p->pDestDb);
        }
        ఒకవేళ( dest_బహుళకం==PAGER_JOURNALMODE_WAL ){
          rc = sqlite3_Btree_సమితి_సంస్కరణ(p->pDest, 2);
        }
      }
      ఒకవేళ( rc==SQLITE_OK ){
        పూర్ణాంకం nDestTruncate;
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
        ప్రతిజ్ఞా( pgszSrc==sqlite3_Btree_Get_పుట_కొలత(p->pSrc) );
        ప్రతిజ్ఞా( pgszDest==sqlite3_Btree_Get_పుట_కొలత(p->pDest) );
        ఒకవేళ( pgszSrc<pgszDest ){
          పూర్ణాంకం నిష్పత్తి = pgszDest/pgszSrc;
          nDestTruncate = (n_Src_పుట+నిష్పత్తి-1)/నిష్పత్తి;
          ఒకవేళ( nDestTruncate==(పూర్ణాంకం)PENDING_BYTE_పుట(p->pDest->pBt) ){
            nDestTruncate--;
          }
        }లేకపోతే{
          nDestTruncate = n_Src_పుట * (pgszSrc/pgszDest);
        }
        ప్రతిజ్ఞా( nDestTruncate>0 );

        ఒకవేళ( pgszSrc<pgszDest ){
          /* If the source page-size is smaller than the destination page-size,
          ** two extra things may need to happen:
          **
          **   * The destination may need to be truncated, and
          **
          **   * Data stored on the pages immediately following the 
          **     pending-byte page in the source database may need to be
          **     copied into the destination database.
          */
          స్థిరరాశి i64 i_కొలత = (i64)pgszSrc * (i64)n_Src_పుట;
          sqlite3_file * స్థిరరాశి pFile = sqlite3PagerFile(pDestPager);
          Pgno iPg;
          పూర్ణాంకం n_Dst_పుట;
          i64 iOff;
          i64 i_చివర;

          ప్రతిజ్ఞా( pFile );
          ప్రతిజ్ఞా( nDestTruncate==0 
              || (i64)nDestTruncate*(i64)pgszDest >= i_కొలత || (
                nDestTruncate==(పూర్ణాంకం)(PENDING_BYTE_పుట(p->pDest->pBt)-1)
             && i_కొలత>=PENDING_BYTE && i_కొలత<=PENDING_BYTE+pgszDest
          ));

          /* This block ensures that all data required to recreate the original
          ** database has been stored in the journal for pDestPager and the
          ** journal synced to disk. So at this point we may safely modify
          ** the database file in any way, knowing that if a power failure
          ** occurs, the original database will be reconstructed from the 
          ** journal file.  */
          sqlite3PagerPagecount(pDestPager, &n_Dst_పుట);
          కొరకు(iPg=nDestTruncate; rc==SQLITE_OK && iPg<=(Pgno)n_Dst_పుట; iPg++){
            ఒకవేళ( iPg!=PENDING_BYTE_పుట(p->pDest->pBt) ){
              Db_పుట *pPg;
              rc = sqlite3PagerGet(pDestPager, iPg, &pPg, 0);
              ఒకవేళ( rc==SQLITE_OK ){
                rc = sqlite3PagerWrite(pPg);
                sqlite3PagerUnref(pPg);
              }
            }
          }
          ఒకవేళ( rc==SQLITE_OK ){
            rc = sqlite3PagerCommitPhaseOne(pDestPager, 0, 1);
          }

          /* Write the extra pages and truncate the database file as required */
          i_చివర = MIN(PENDING_BYTE + pgszDest, i_కొలత);
          కొరకు(
            iOff=PENDING_BYTE+pgszSrc; 
            rc==SQLITE_OK && iOff<i_చివర; 
            iOff+=pgszSrc
          ){
            PgHdr *pSrcPg = 0;
            స్థిరరాశి Pgno iSrcPg = (Pgno)((iOff/pgszSrc)+1);
            rc = sqlite3PagerGet(pSrcPager, iSrcPg, &pSrcPg, 0);
            ఒకవేళ( rc==SQLITE_OK ){
              u8 *z_దత్తాంశం = sqlite3_Pager_Get_దత్తాంశం(pSrcPg);
              rc = sqlite3OsWrite(pFile, z_దత్తాంశం, pgszSrc, iOff);
            }
            sqlite3PagerUnref(pSrcPg);
          }
          ఒకవేళ( rc==SQLITE_OK ){
            rc = backupTruncateFile(pFile, i_కొలత);
          }

          /* Sync the database file to disk. */
          ఒకవేళ( rc==SQLITE_OK ){
            rc = sqlite3PagerSync(pDestPager, 0);
          }
        }లేకపోతే{
          sqlite3PagerTruncateImage(pDestPager, nDestTruncate);
          rc = sqlite3PagerCommitPhaseOne(pDestPager, 0, 0);
        }
    
        /* Finish committing the transaction to the destination database. */
        ఒకవేళ( SQLITE_OK==rc
         && SQLITE_OK==(rc = sqlite3BtreeCommitPhaseTwo(p->pDest, 0))
        ){
          rc = SQLITE_పూర్తి;
        }
      }
    }
  
    /* If bCloseTrans is true, then this function opened a read transaction
    ** on the source database. Close the read transaction here. There is
    ** no need to check the return values of the btree methods here, as
    ** "committing" a read-only transaction cannot fail.
    */
    ఒకవేళ( bCloseTrans ){
      TESTONLY( పూర్ణాంకం rc2 );
      TESTONLY( rc2  = ) sqlite3BtreeCommitPhaseOne(p->pSrc, 0);
      TESTONLY( rc2 |= ) sqlite3BtreeCommitPhaseTwo(p->pSrc, 0);
      ప్రతిజ్ఞా( rc2==SQLITE_OK );
    }
  
    ఒకవేళ( rc==SQLITE_IOERR_NOMEM ){
      rc = SQLITE_NOMEM_BKPT;
    }
    p->rc = rc;
  }
  ఒకవేళ( p->pDestDb ){
    sqlite3_mutex_leave(p->pDestDb->mutex);
  }
  sqlite3BtreeLeave(p->pSrc);
  sqlite3_mutex_leave(p->pSrcDb->mutex);
  తిరిగి rc;
}

/*
** Release all resources associated with an sqlite3_backup* handle.
*/
పూర్ణాంకం sqlite3_backup_ముగింపు(sqlite3_backup *p){
  sqlite3_backup **pp;                 /* Ptr to head of pagers backup list */
  sqlite3 *pSrcDb;                     /* Source database connection */
  పూర్ణాంకం rc;                              /* Value to return */

  /* Enter the mutexes */
  ఒకవేళ( p==0 ) తిరిగి SQLITE_OK;
  pSrcDb = p->pSrcDb;
  sqlite3_mutex_enter(pSrcDb->mutex);
  sqlite3BtreeEnter(p->pSrc);
  ఒకవేళ( p->pDestDb ){
    sqlite3_mutex_enter(p->pDestDb->mutex);
  }

  /* Detach this backup from the source pager. */
  ఒకవేళ( p->pDestDb ){
    p->pSrc->nBackup--;
  }
  ఒకవేళ( p->isAttached ){
    pp = sqlite3PagerBackupPtr(sqlite3BtreePager(p->pSrc));
    ప్రతిజ్ఞా( pp!=0 );
    అయితే( *pp!=p ){
      pp = &(*pp)->p_తదుపరి;
      ప్రతిజ్ఞా( pp!=0 );
    }
    *pp = p->p_తదుపరి;
  }

  /* If a transaction is still open on the Btree, roll it back. */
  ఒకవేళ( p->pDest ){
    sqlite3BtreeRollback(p->pDest, SQLITE_OK, 0);
  }

  /* Set the error code of the destination database handle. */
  rc = (p->rc==SQLITE_పూర్తి) ? SQLITE_OK : p->rc;
  ఒకవేళ( p->pDestDb ){
    sqlite3Error(p->pDestDb, rc);

    /* Exit the mutexes and free the backup context structure. */
    sqlite3LeaveMutexAndCloseZombie(p->pDestDb);
  }
  sqlite3BtreeLeave(p->pSrc);
  ఒకవేళ( p->pDestDb ){
    /* EVIDENCE-OF: R-64852-21591 The sqlite3_backup object is created by a
    ** call to sqlite3_backup_init() and is destroyed by a call to
    ** sqlite3_backup_finish(). */
    sqlite3_free(p);
  }
  sqlite3LeaveMutexAndCloseZombie(pSrcDb);
  తిరిగి rc;
}

/*
** Return the number of pages still to be backed up as of the most recent
** call to sqlite3_backup_step().
*/
పూర్ణాంకం sqlite3_backup_మిగిలిన(sqlite3_backup *p){
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( p==0 ){
    (శూన్య)SQLITE_MISUSE_BKPT;
    తిరిగి 0;
  }
#అయితే_ముగింపు
  తిరిగి p->n_మిగిలిన;
}

/*
** Return the total number of pages in the source database as of the most 
** recent call to sqlite3_backup_step().
*/
పూర్ణాంకం sqlite3_backup_pagecount(sqlite3_backup *p){
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( p==0 ){
    (శూన్య)SQLITE_MISUSE_BKPT;
    తిరిగి 0;
  }
#అయితే_ముగింపు
  తిరిగి p->nPagecount;
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
స్థిర SQLITE_NOINLINE శూన్య backupUpdate(
  sqlite3_backup *p,
  Pgno i_పుట,
  స్థిరరాశి u8 *a_దత్తాంశం
){
  ప్రతిజ్ఞా( p!=0 );
  చేయి{
    ప్రతిజ్ఞా( sqlite3_mutex_held(p->pSrc->pBt->mutex) );
    ఒకవేళ( !isFatalError(p->rc) && i_పుట<p->i_తదుపరి ){
      /* The backup process p has already copied page iPage. But now it
      ** has been modified by a transaction on the source pager. Copy
      ** the new data into the backup.
      */
      పూర్ణాంకం rc;
      ప్రతిజ్ఞా( p->pDestDb );
      sqlite3_mutex_enter(p->pDestDb->mutex);
      rc = backup_One_పుట(p, i_పుట, a_దత్తాంశం, 1);
      sqlite3_mutex_leave(p->pDestDb->mutex);
      ప్రతిజ్ఞా( rc!=SQLITE_BUSY && rc!=SQLITE_LOCKED );
      ఒకవేళ( rc!=SQLITE_OK ){
        p->rc = rc;
      }
    }
  }అయితే( (p = p->p_తదుపరి)!=0 );
}
శూన్య sqlite3BackupUpdate(sqlite3_backup *pBackup, Pgno i_పుట, స్థిరరాశి u8 *a_దత్తాంశం){
  ఒకవేళ( pBackup ) backupUpdate(pBackup, i_పుట, a_దత్తాంశం);
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
శూన్య sqlite3BackupRestart(sqlite3_backup *pBackup){
  sqlite3_backup *p;                   /* Iterator variable */
  కొరకు(p=pBackup; p; p=p->p_తదుపరి){
    ప్రతిజ్ఞా( sqlite3_mutex_held(p->pSrc->pBt->mutex) );
    p->i_తదుపరి = 1;
  }
}

#నిర్వచితం_కాకపోతే SQLITE_OMIT_VACUUM
/*
** Copy the complete content of pBtFrom into pBtTo.  A transaction
** must be active for both files.
**
** The size of file pTo may be reduced by this operation. If anything 
** goes wrong, the transaction on pTo is rolled back. If successful, the 
** transaction is committed before returning.
*/
పూర్ణాంకం sqlite3BtreeCopyFile(Btree *p_గమ్యం, Btree *pFrom){
  పూర్ణాంకం rc;
  sqlite3_file *pFd;              /* File descriptor for database pTo */
  sqlite3_backup b;
  sqlite3BtreeEnter(p_గమ్యం);
  sqlite3BtreeEnter(pFrom);

  ప్రతిజ్ఞా( sqlite3BtreeTxnState(p_గమ్యం)==SQLITE_TXN_WRITE );
  pFd = sqlite3PagerFile(sqlite3BtreePager(p_గమ్యం));
  ఒకవేళ( pFd->pMethods ){
    i64 nByte = sqlite3_Btree_Get_పుట_కొలత(pFrom)*(i64)sqlite3_Btree_చివరి_పుట(pFrom);
    rc = sqlite3OsFileControl(pFd, SQLITE_FCNTL_OVERWRITE, &nByte);
    ఒకవేళ( rc==SQLITE_NOTFOUND ) rc = SQLITE_OK;
    ఒకవేళ( rc ) వెళ్ళు copy_ముగిసిన;
  }

  /* Set up an sqlite3_backup object. sqlite3_backup.pDestDb must be set
  ** to 0. This is used by the implementations of sqlite3_backup_step()
  ** and sqlite3_backup_finish() to detect that they are being called
  ** from this function, not directly by the user.
  */
  memset(&b, 0, పరిమాణం(b));
  b.pSrcDb = pFrom->db;
  b.pSrc = pFrom;
  b.pDest = p_గమ్యం;
  b.i_తదుపరి = 1;

  /* 0x7FFFFFFF is the hard limit for the number of pages in a database
  ** file. By passing this as the number of pages to copy to
  ** sqlite3_backup_step(), we can guarantee that the copy finishes 
  ** within a single call (unless an error occurs). The assert() statement
  ** checks this assumption - (p->rc) should be set to either SQLITE_DONE 
  ** or an error code.  */
  sqlite3_backup_అడుగు(&b, 0x7FFFFFFF);
  ప్రతిజ్ఞా( b.rc!=SQLITE_OK );

  rc = sqlite3_backup_ముగింపు(&b);
  ఒకవేళ( rc==SQLITE_OK ){
    p_గమ్యం->pBt->btsFlags &= ~BTS_PAGESIZE_FIXED;
  }లేకపోతే{
    sqlite3PagerClearCache(sqlite3BtreePager(b.pDest));
  }

  ప్రతిజ్ఞా( sqlite3BtreeTxnState(p_గమ్యం)!=SQLITE_TXN_WRITE );
copy_ముగిసిన:
  sqlite3BtreeLeave(pFrom);
  sqlite3BtreeLeave(p_గమ్యం);
  తిరిగి rc;
}
#అయితే_ముగింపు /* SQLITE_OMIT_VACUUM */
