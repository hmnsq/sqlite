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
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"
#ಅನ್ತರ್ಭಾವ "btreeInt.h"

/*
** Structure allocated for each backup operation.
*/
ರಚನೆ sqlite3_backup {
  sqlite3* pDestDb;        /* Destination database handle */
  ಅಕ್ಷರ *zDestDb;
  Btree *pDest;            /* Destination b-tree file */
  u32 iDestSchema;         /* Original schema cookie in destination */
  ಪೂರ್ಣಾಂಕ bDestLocked;         /* True once a write-transaction is open on pDest */

  Pgno i_ಮುಂದಿನ;              /* Page number of the next source page to copy */
  sqlite3* pSrcDb;         /* Source database handle */
  Btree *pSrc;             /* Source b-tree file */

  ಪೂರ್ಣಾಂಕ rc;                  /* Backup process error code */

  /* These two variables are set by every call to backup_step(). They are
  ** read by calls to backup_remaining() and backup_pagecount().
  */
  Pgno n_ಉಳಿದ;         /* Number of pages left to copy */
  Pgno nPagecount;         /* Total number of pages to copy */

  ಪೂರ್ಣಾಂಕ isAttached;          /* True once backup has been registered with pager */
  sqlite3_backup *p_ಮುಂದಿನ;   /* Next backup associated with source pager */
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
ಸ್ಥಿರ Btree *ಕಂಡುಹಿಡಿ_Btree(sqlite3 *pErrorDb, sqlite3 *pDb, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDb){
  ಪೂರ್ಣಾಂಕ i;

  ಪ್ರತಿಜ್ಞಾ( pDb!=0 );
  i = sqlite3_ಕಂಡುಹಿಡಿ_Db_Name(pDb, zDb);
  ಆದರೆ( i==1 ){
    Parse sParse;
    ಪೂರ್ಣಾಂಕ rc = 0;
    sqlite3ParseObjectInit(&sParse,pDb);
    ಆದರೆ( sqlite3_Open_ತಾತ್ಕಾಲಿಕ_Database(&sParse) ){
      sqlite3_Error_With_ಸಂದೇಶ(pErrorDb, sParse.rc, "%s", sParse.z_Err_ಸಂದೇಶ);
      rc = SQLITE_ERROR;
    }
    sqlite3DbFree(pErrorDb, sParse.z_Err_ಸಂದೇಶ);
    sqlite3ParseObjectReset(&sParse);
    ಆದರೆ( rc ){
      ಹಿಂತಿರುಗಿ 0;
    }
  }

  ಆದರೆ( i<0 ){
    sqlite3_Error_With_ಸಂದೇಶ(pErrorDb, SQLITE_ERROR, "unknown database %s", zDb);
    ಹಿಂತಿರುಗಿ 0;
  }

  ಹಿಂತಿರುಗಿ pDb->aDb[i].pBt;
}

/*
** Attempt to set the page size of the destination to match the page size
** of the source.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಗಣ_Dest_Pgsz(Btree *pDest, Btree *pSrc){
  ಹಿಂತಿರುಗಿ sqlite3_Btree_ಗಣ_ಪುಟ_ಗಾತ್ರ(pDest, sqlite3_Btree_Get_ಪುಟ_ಗಾತ್ರ(pSrc), 0, 0);
}

/*
** Check that there is no open read-transaction on the b-tree passed as the
** second argument. If there is not, return SQLITE_OK. Otherwise, if there
** is an open read-transaction, return SQLITE_ERROR and leave an error 
** message in database handle db.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ checkReadTransaction(sqlite3 *db, Btree *p){
  ಆದರೆ( sqlite3BtreeTxnState(p)!=SQLITE_TXN_NONE ){
    sqlite3_Error_With_ಸಂದೇಶ(db, SQLITE_ERROR, "destination database is in use");
    ಹಿಂತಿರುಗಿ SQLITE_ERROR;
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
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
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDestDb,                  /* Name of database within pDestDb */
  sqlite3* pSrcDb,                      /* Database connection to read from */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSrcDb                    /* Name of database within pSrcDb */
){
  sqlite3_backup *p;                    /* Value to return */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !sqlite3SafetyCheckOk(pSrcDb)||!sqlite3SafetyCheckOk(pDestDb) ){
    (ಶೂನ್ಯ)SQLITE_MISUSE_BKPT;
    ಹಿಂತಿರುಗಿ 0;
  }
#ಆದರೆ_ಅಂತ್ಯ

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

  ಆದರೆ( pSrcDb==pDestDb ){
    sqlite3_Error_With_ಸಂದೇಶ(
        pDestDb, SQLITE_ERROR, "source and destination must be distinct"
    );
    p = 0;
  }ಇಲ್ಲದಿದ್ದರೆ {
    ಪೂರ್ಣಾಂಕ nDest = sqlite3Strlen30(zDestDb);

    /* Allocate space for a new sqlite3_backup object...
    ** EVIDENCE-OF: R-64852-21591 The sqlite3_backup object is created by a
    ** call to sqlite3_backup_init() and is destroyed by a call to
    ** sqlite3_backup_finish(). */
    p = (sqlite3_backup*)sqlite3MallocZero(ಗಾತ್ರ(sqlite3_backup)+nDest+1);
    ಆದರೆ( !p ){
      sqlite3Error(pDestDb, SQLITE_NOMEM_BKPT);
    }ಇಲ್ಲದಿದ್ದರೆ{
      p->zDestDb = (ಅಕ್ಷರ*)&p[1];
      ಸ್ಮರಣೆ_ನಕಲು(p->zDestDb, zDestDb, nDest);
    }
  }

  /* If the allocation succeeded, populate the new object. */
  ಆದರೆ( p ){
    /* Do not store the pointer to the destination b-tree at this point.
    ** This is because there is nothing preventing it from being detached
    ** or otherwise freed before the first call to sqlite3_backup_step()
    ** on this object. The source b-tree does not have this problem, as
    ** incrementing Btree.nBackup (see below) effectively locks the object. */
    Btree *pDest = ಕಂಡುಹಿಡಿ_Btree(pDestDb, pDestDb, zDestDb);
    p->pSrc = ಕಂಡುಹಿಡಿ_Btree(pDestDb, pSrcDb, zSrcDb);
    p->pDestDb = pDestDb;
    p->pSrcDb = pSrcDb;
    p->i_ಮುಂದಿನ = 1;
    p->isAttached = 0;

    ಆದರೆ( 0==p->pSrc || 0==pDest 
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
  ಆದರೆ( p ){
    p->pSrc->nBackup++;
  }

  sqlite3_mutex_leave(pDestDb->mutex);
  sqlite3_mutex_leave(pSrcDb->mutex);
  ಹಿಂತಿರುಗಿ p;
}

/*
** Argument rc is an SQLite error code. Return true if this error is 
** considered fatal if encountered during a backup operation. All errors
** are considered fatal except for SQLITE_BUSY and SQLITE_LOCKED.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ isFatalError(ಪೂರ್ಣಾಂಕ rc){
  ಹಿಂತಿರುಗಿ (rc!=SQLITE_OK && rc!=SQLITE_BUSY && ALWAYS(rc!=SQLITE_LOCKED));
}

/*
** Parameter zSrcData points to a buffer containing the data for 
** page iSrcPg from the source database. Copy this data into the 
** destination database.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ backup_One_ಪುಟ(
  sqlite3_backup *p,              /* Backup handle */
  Pgno iSrcPg,                    /* Source database page to backup */
  ಸ್ಥಿರಾಂಕ u8 *z_Src_ದತ್ತಾಂಶ,             /* Source database page data */
  ಪೂರ್ಣಾಂಕ bUpdate                     /* True for an update, false otherwise */
){
  Pager * ಸ್ಥಿರಾಂಕ pDestPager = sqlite3BtreePager(p->pDest);
  ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ nSrcPgsz = sqlite3_Btree_Get_ಪುಟ_ಗಾತ್ರ(p->pSrc);
  ಪೂರ್ಣಾಂಕ nDestPgsz = sqlite3_Btree_Get_ಪುಟ_ಗಾತ್ರ(p->pDest);
  ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ nCopy = MIN(nSrcPgsz, nDestPgsz);
  ಸ್ಥಿರಾಂಕ i64 i_ಕೊನೆ = (i64)iSrcPg*(i64)nSrcPgsz;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  i64 iOff;

  ಪ್ರತಿಜ್ಞಾ( sqlite3BtreeGetReserveNoMutex(p->pSrc)>=0 );
  ಪ್ರತಿಜ್ಞಾ( p->bDestLocked );
  ಪ್ರತಿಜ್ಞಾ( !isFatalError(p->rc) );
  ಪ್ರತಿಜ್ಞಾ( iSrcPg!=PENDING_BYTE_ಪುಟ(p->pSrc->pBt) );
  ಪ್ರತಿಜ್ಞಾ( z_Src_ದತ್ತಾಂಶ );
  ಪ್ರತಿಜ್ಞಾ( nSrcPgsz==nDestPgsz || sqlite3PagerIsMemdb(pDestPager)==0 );

  /* This loop runs once for each destination page spanned by the source 
  ** page. For each iteration, variable iOff is set to the byte offset
  ** of the destination page.
  */
  ಗಾಗಿ(iOff=i_ಕೊನೆ-(i64)nSrcPgsz; rc==SQLITE_OK && iOff<i_ಕೊನೆ; iOff+=nDestPgsz){
    Db_ಪುಟ *pDestPg = 0;
    Pgno iDest = (Pgno)(iOff/nDestPgsz)+1;
    ಆದರೆ( iDest==PENDING_BYTE_ಪುಟ(p->pDest->pBt) ) ಮುಂದುವರಿ;
    ಆದರೆ( SQLITE_OK==(rc = sqlite3PagerGet(pDestPager, iDest, &pDestPg, 0))
     && SQLITE_OK==(rc = sqlite3PagerWrite(pDestPg))
    ){
      ಸ್ಥಿರಾಂಕ u8 *z_ಒಳ = &z_Src_ದತ್ತಾಂಶ[iOff%nSrcPgsz];
      u8 *z_Dest_ದತ್ತಾಂಶ = sqlite3_Pager_Get_ದತ್ತಾಂಶ(pDestPg);
      u8 *z_ಹೊರಹೋಗು = &z_Dest_ದತ್ತಾಂಶ[iOff%nDestPgsz];

      /* Copy the data from the source page into the destination page.
      ** Then clear the Btree layer MemPage.isInit flag. Both this module
      ** and the pager code use this trick (clearing the first byte
      ** of the page 'extra' space to invalidate the Btree layers
      ** cached parse of the page). MemPage.isInit is marked 
      ** "MUST BE FIRST" for this purpose.
      */
      ಸ್ಮರಣೆ_ನಕಲು(z_ಹೊರಹೋಗು, z_ಒಳ, nCopy);
      ((u8 *)sqlite3PagerGetExtra(pDestPg))[0] = 0;
      ಆದರೆ( iOff==0 && bUpdate==0 ){
        sqlite3Put4byte(&z_ಹೊರಹೋಗು[28], sqlite3_Btree_ಕೊನೆಯ_ಪುಟ(p->pSrc));
      }
    }
    sqlite3PagerUnref(pDestPg);
  }

  ಹಿಂತಿರುಗಿ rc;
}

/*
** If pFile is currently larger than iSize bytes, then truncate it to
** exactly iSize bytes. If pFile is not larger than iSize bytes, then
** this function is a no-op.
**
** Return SQLITE_OK if everything is successful, or an SQLite error 
** code if an error occurs.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ backupTruncateFile(sqlite3_file *pFile, i64 i_ಗಾತ್ರ){
  i64 iCurrent;
  ಪೂರ್ಣಾಂಕ rc = sqlite3_Os_File_ಗಾತ್ರ(pFile, &iCurrent);
  ಆದರೆ( rc==SQLITE_OK && iCurrent>i_ಗಾತ್ರ ){
    rc = sqlite3OsTruncate(pFile, i_ಗಾತ್ರ);
  }
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Register this backup object with the associated source pager for
** callbacks when pages are changed or the cache invalidated.
*/
ಸ್ಥಿರ ಶೂನ್ಯ attachBackupObject(sqlite3_backup *p){
  sqlite3_backup **pp;
  ಪ್ರತಿಜ್ಞಾ( sqlite3BtreeHoldsMutex(p->pSrc) );
  pp = sqlite3PagerBackupPtr(sqlite3BtreePager(p->pSrc));
  p->p_ಮುಂದಿನ = *pp;
  *pp = p;
  p->isAttached = 1;
}

/*
** Copy nPage pages from the source b-tree to the destination.
*/
ಪೂರ್ಣಾಂಕ sqlite3_backup_ಹೆಜ್ಜೆ(sqlite3_backup *p, ಪೂರ್ಣಾಂಕ n_ಪುಟ){
  ಪೂರ್ಣಾಂಕ rc;
  ಪೂರ್ಣಾಂಕ dest_ಬಹುಲಕ = 0;   /* Destination journal mode */
  ಪೂರ್ಣಾಂಕ pgszSrc = 0;    /* Source page size */
  ಪೂರ್ಣಾಂಕ pgszDest = 0;   /* Destination page size */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( p==0 ) ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_mutex_enter(p->pSrcDb->mutex);
  sqlite3BtreeEnter(p->pSrc);
  ಆದರೆ( p->pDestDb ){
    sqlite3_mutex_enter(p->pDestDb->mutex);
  }

  rc = p->rc;
  ಆದರೆ( !isFatalError(rc) ){
    Pager * ಸ್ಥಿರಾಂಕ pSrcPager = sqlite3BtreePager(p->pSrc);     /* Source pager */
    Btree * pDest = 0;                                        /* Dest btree */
    Pager * pDestPager = 0;                                   /* Dest pager */
    ಪೂರ್ಣಾಂಕ ii;                            /* Iterator variable */
    ಪೂರ್ಣಾಂಕ n_Src_ಪುಟ = -1;                 /* Size of source db in pages */
    ಪೂರ್ಣಾಂಕ bCloseTrans = 0;               /* True if src db requires unlocking */

    /* If the source pager is currently in a write-transaction, return
    ** SQLITE_BUSY immediately.
    */
    ಆದರೆ( p->pDestDb && p->pSrc->pBt->ಒಳ_Transaction==TRANS_WRITE ){
      rc = SQLITE_BUSY;
    }ಇಲ್ಲದಿದ್ದರೆ{
      rc = SQLITE_OK;
    }


    /* If there is no open read-transaction on the source database, open
    ** one now. If a transaction is opened here, then it will be closed
    ** before this function exits.
    */
    ಆದರೆ( rc==SQLITE_OK && SQLITE_TXN_NONE==sqlite3BtreeTxnState(p->pSrc) ){
      rc = sqlite3BtreeBeginTrans(p->pSrc, 0, 0);
      bCloseTrans = 1;
    }

    /* Locate the destination btree and pager. */
    ಆದರೆ( (pDest = p->pDest)==0 ){
      pDest = ಕಂಡುಹಿಡಿ_Btree(p->pDestDb, p->pDestDb, p->zDestDb);
    }
    ಆದರೆ( pDest==0 ){
      rc = SQLITE_ERROR;
    }ಇಲ್ಲದಿದ್ದರೆ{
      pDestPager = sqlite3BtreePager(pDest);
    }

    /* If the destination database has not yet been locked (i.e. if this
    ** is the first call to backup_step() for the current backup operation),
    ** try to set its page size to the same as the source database. This
    ** is especially important on ZipVFS systems, as in that case it is
    ** not possible to create a database file that uses one page size by
    ** writing to it with another.  */
    ಆದರೆ( p->bDestLocked==0 && rc==SQLITE_OK 
     && ಗಣ_Dest_Pgsz(pDest, p->pSrc)==SQLITE_NOMEM
    ){
      rc = SQLITE_NOMEM;
    }

    /* Lock the destination database, if it is not locked already. */
    ಆದರೆ( SQLITE_OK==rc && p->bDestLocked==0
     && SQLITE_OK==(rc = sqlite3BtreeBeginTrans(pDest, 2,
                                                (ಪೂರ್ಣಾಂಕ*)&p->iDestSchema)) 
    ){
      p->bDestLocked = 1;
      p->pDest = pDest;
    }

    /* Do not allow backup if the destination database is in WAL mode
    ** and the page sizes are different between source and destination */
    ಆದರೆ( rc==SQLITE_OK ){
      pgszSrc = sqlite3_Btree_Get_ಪುಟ_ಗಾತ್ರ(p->pSrc);
      pgszDest = sqlite3_Btree_Get_ಪುಟ_ಗಾತ್ರ(p->pDest);
      dest_ಬಹುಲಕ = sqlite3_Pager_Get_Journal_ಬಹುಲಕ(sqlite3BtreePager(p->pDest));
      ಆದರೆ( (dest_ಬಹುಲಕ==PAGER_JOURNALMODE_WAL || sqlite3PagerIsMemdb(pDestPager))
        && pgszSrc!=pgszDest 
      ){
        rc = SQLITE_READONLY;
      }
    }
  
    /* Now that there is a read-lock on the source database, query the
    ** source pager for the number of pages in the database.
    */
    n_Src_ಪುಟ = (ಪೂರ್ಣಾಂಕ)sqlite3_Btree_ಕೊನೆಯ_ಪುಟ(p->pSrc);
    ಪ್ರತಿಜ್ಞಾ( n_Src_ಪುಟ>=0 );
    ಗಾಗಿ(ii=0; (n_ಪುಟ<0 || ii<n_ಪುಟ) && p->i_ಮುಂದಿನ<=(Pgno)n_Src_ಪುಟ && !rc; ii++){
      ಸ್ಥಿರಾಂಕ Pgno iSrcPg = p->i_ಮುಂದಿನ;                 /* Source page number */
      ಆದರೆ( iSrcPg!=PENDING_BYTE_ಪುಟ(p->pSrc->pBt) ){
        Db_ಪುಟ *pSrcPg;                             /* Source page object */
        rc = sqlite3PagerGet(pSrcPager, iSrcPg, &pSrcPg,PAGER_GET_READONLY);
        ಆದರೆ( rc==SQLITE_OK ){
          rc = backup_One_ಪುಟ(p, iSrcPg, sqlite3_Pager_Get_ದತ್ತಾಂಶ(pSrcPg), 0);
          sqlite3PagerUnref(pSrcPg);
        }
      }
      p->i_ಮುಂದಿನ++;
    }
    ಆದರೆ( rc==SQLITE_OK ){
      p->nPagecount = n_Src_ಪುಟ;
      p->n_ಉಳಿದ = n_Src_ಪುಟ+1-p->i_ಮುಂದಿನ;
      ಆದರೆ( p->i_ಮುಂದಿನ>(Pgno)n_Src_ಪುಟ ){
        rc = SQLITE_ಮುಗಿದ;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( !p->isAttached ){
        attachBackupObject(p);
      }
    }
  
    /* Update the schema version field in the destination database. This
    ** is to make sure that the schema-version really does change in
    ** the case where the source and destination databases have the
    ** same schema version.
    */
    ಆದರೆ( rc==SQLITE_ಮುಗಿದ ){
      ಆದರೆ( n_Src_ಪುಟ==0 ){
        rc = sqlite3BtreeNewDb(p->pDest);
        n_Src_ಪುಟ = 1;
      }
      ಆದರೆ( rc==SQLITE_OK || rc==SQLITE_ಮುಗಿದ ){
        rc = sqlite3BtreeUpdateMeta(p->pDest,1,p->iDestSchema+1);
      }
      ಆದರೆ( rc==SQLITE_OK ){
        ಆದರೆ( p->pDestDb ){
          sqlite3_Reset_All_Schemas_ದ_Connection(p->pDestDb);
        }
        ಆದರೆ( dest_ಬಹುಲಕ==PAGER_JOURNALMODE_WAL ){
          rc = sqlite3_Btree_ಗಣ_ಆವೃತ್ತಿ(p->pDest, 2);
        }
      }
      ಆದರೆ( rc==SQLITE_OK ){
        ಪೂರ್ಣಾಂಕ nDestTruncate;
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
        ಪ್ರತಿಜ್ಞಾ( pgszSrc==sqlite3_Btree_Get_ಪುಟ_ಗಾತ್ರ(p->pSrc) );
        ಪ್ರತಿಜ್ಞಾ( pgszDest==sqlite3_Btree_Get_ಪುಟ_ಗಾತ್ರ(p->pDest) );
        ಆದರೆ( pgszSrc<pgszDest ){
          ಪೂರ್ಣಾಂಕ ಅನುಪಾತ = pgszDest/pgszSrc;
          nDestTruncate = (n_Src_ಪುಟ+ಅನುಪಾತ-1)/ಅನುಪಾತ;
          ಆದರೆ( nDestTruncate==(ಪೂರ್ಣಾಂಕ)PENDING_BYTE_ಪುಟ(p->pDest->pBt) ){
            nDestTruncate--;
          }
        }ಇಲ್ಲದಿದ್ದರೆ{
          nDestTruncate = n_Src_ಪುಟ * (pgszSrc/pgszDest);
        }
        ಪ್ರತಿಜ್ಞಾ( nDestTruncate>0 );

        ಆದರೆ( pgszSrc<pgszDest ){
          /* If the source page-size is smaller than the destination page-size,
          ** two extra things may need to happen:
          **
          **   * The destination may need to be truncated, and
          **
          **   * Data stored on the pages immediately following the 
          **     pending-byte page in the source database may need to be
          **     copied into the destination database.
          */
          ಸ್ಥಿರಾಂಕ i64 i_ಗಾತ್ರ = (i64)pgszSrc * (i64)n_Src_ಪುಟ;
          sqlite3_file * ಸ್ಥಿರಾಂಕ pFile = sqlite3PagerFile(pDestPager);
          Pgno iPg;
          ಪೂರ್ಣಾಂಕ n_Dst_ಪುಟ;
          i64 iOff;
          i64 i_ಕೊನೆ;

          ಪ್ರತಿಜ್ಞಾ( pFile );
          ಪ್ರತಿಜ್ಞಾ( nDestTruncate==0 
              || (i64)nDestTruncate*(i64)pgszDest >= i_ಗಾತ್ರ || (
                nDestTruncate==(ಪೂರ್ಣಾಂಕ)(PENDING_BYTE_ಪುಟ(p->pDest->pBt)-1)
             && i_ಗಾತ್ರ>=PENDING_BYTE && i_ಗಾತ್ರ<=PENDING_BYTE+pgszDest
          ));

          /* This block ensures that all data required to recreate the original
          ** database has been stored in the journal for pDestPager and the
          ** journal synced to disk. So at this point we may safely modify
          ** the database file in any way, knowing that if a power failure
          ** occurs, the original database will be reconstructed from the 
          ** journal file.  */
          sqlite3PagerPagecount(pDestPager, &n_Dst_ಪುಟ);
          ಗಾಗಿ(iPg=nDestTruncate; rc==SQLITE_OK && iPg<=(Pgno)n_Dst_ಪುಟ; iPg++){
            ಆದರೆ( iPg!=PENDING_BYTE_ಪುಟ(p->pDest->pBt) ){
              Db_ಪುಟ *pPg;
              rc = sqlite3PagerGet(pDestPager, iPg, &pPg, 0);
              ಆದರೆ( rc==SQLITE_OK ){
                rc = sqlite3PagerWrite(pPg);
                sqlite3PagerUnref(pPg);
              }
            }
          }
          ಆದರೆ( rc==SQLITE_OK ){
            rc = sqlite3PagerCommitPhaseOne(pDestPager, 0, 1);
          }

          /* Write the extra pages and truncate the database file as required */
          i_ಕೊನೆ = MIN(PENDING_BYTE + pgszDest, i_ಗಾತ್ರ);
          ಗಾಗಿ(
            iOff=PENDING_BYTE+pgszSrc; 
            rc==SQLITE_OK && iOff<i_ಕೊನೆ; 
            iOff+=pgszSrc
          ){
            PgHdr *pSrcPg = 0;
            ಸ್ಥಿರಾಂಕ Pgno iSrcPg = (Pgno)((iOff/pgszSrc)+1);
            rc = sqlite3PagerGet(pSrcPager, iSrcPg, &pSrcPg, 0);
            ಆದರೆ( rc==SQLITE_OK ){
              u8 *z_ದತ್ತಾಂಶ = sqlite3_Pager_Get_ದತ್ತಾಂಶ(pSrcPg);
              rc = sqlite3OsWrite(pFile, z_ದತ್ತಾಂಶ, pgszSrc, iOff);
            }
            sqlite3PagerUnref(pSrcPg);
          }
          ಆದರೆ( rc==SQLITE_OK ){
            rc = backupTruncateFile(pFile, i_ಗಾತ್ರ);
          }

          /* Sync the database file to disk. */
          ಆದರೆ( rc==SQLITE_OK ){
            rc = sqlite3PagerSync(pDestPager, 0);
          }
        }ಇಲ್ಲದಿದ್ದರೆ{
          sqlite3PagerTruncateImage(pDestPager, nDestTruncate);
          rc = sqlite3PagerCommitPhaseOne(pDestPager, 0, 0);
        }
    
        /* Finish committing the transaction to the destination database. */
        ಆದರೆ( SQLITE_OK==rc
         && SQLITE_OK==(rc = sqlite3BtreeCommitPhaseTwo(p->pDest, 0))
        ){
          rc = SQLITE_ಮುಗಿದ;
        }
      }
    }
  
    /* If bCloseTrans is true, then this function opened a read transaction
    ** on the source database. Close the read transaction here. There is
    ** no need to check the return values of the btree methods here, as
    ** "committing" a read-only transaction cannot fail.
    */
    ಆದರೆ( bCloseTrans ){
      TESTONLY( ಪೂರ್ಣಾಂಕ rc2 );
      TESTONLY( rc2  = ) sqlite3BtreeCommitPhaseOne(p->pSrc, 0);
      TESTONLY( rc2 |= ) sqlite3BtreeCommitPhaseTwo(p->pSrc, 0);
      ಪ್ರತಿಜ್ಞಾ( rc2==SQLITE_OK );
    }
  
    ಆದರೆ( rc==SQLITE_IOERR_NOMEM ){
      rc = SQLITE_NOMEM_BKPT;
    }
    p->rc = rc;
  }
  ಆದರೆ( p->pDestDb ){
    sqlite3_mutex_leave(p->pDestDb->mutex);
  }
  sqlite3BtreeLeave(p->pSrc);
  sqlite3_mutex_leave(p->pSrcDb->mutex);
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Release all resources associated with an sqlite3_backup* handle.
*/
ಪೂರ್ಣಾಂಕ sqlite3_backup_ಮುಕ್ತಾಯ(sqlite3_backup *p){
  sqlite3_backup **pp;                 /* Ptr to head of pagers backup list */
  sqlite3 *pSrcDb;                     /* Source database connection */
  ಪೂರ್ಣಾಂಕ rc;                              /* Value to return */

  /* Enter the mutexes */
  ಆದರೆ( p==0 ) ಹಿಂತಿರುಗಿ SQLITE_OK;
  pSrcDb = p->pSrcDb;
  sqlite3_mutex_enter(pSrcDb->mutex);
  sqlite3BtreeEnter(p->pSrc);
  ಆದರೆ( p->pDestDb ){
    sqlite3_mutex_enter(p->pDestDb->mutex);
  }

  /* Detach this backup from the source pager. */
  ಆದರೆ( p->pDestDb ){
    p->pSrc->nBackup--;
  }
  ಆದರೆ( p->isAttached ){
    pp = sqlite3PagerBackupPtr(sqlite3BtreePager(p->pSrc));
    ಪ್ರತಿಜ್ಞಾ( pp!=0 );
    ಆಗ( *pp!=p ){
      pp = &(*pp)->p_ಮುಂದಿನ;
      ಪ್ರತಿಜ್ಞಾ( pp!=0 );
    }
    *pp = p->p_ಮುಂದಿನ;
  }

  /* If a transaction is still open on the Btree, roll it back. */
  ಆದರೆ( p->pDest ){
    sqlite3BtreeRollback(p->pDest, SQLITE_OK, 0);
  }

  /* Set the error code of the destination database handle. */
  rc = (p->rc==SQLITE_ಮುಗಿದ) ? SQLITE_OK : p->rc;
  ಆದರೆ( p->pDestDb ){
    sqlite3Error(p->pDestDb, rc);

    /* Exit the mutexes and free the backup context structure. */
    sqlite3LeaveMutexAndCloseZombie(p->pDestDb);
  }
  sqlite3BtreeLeave(p->pSrc);
  ಆದರೆ( p->pDestDb ){
    /* EVIDENCE-OF: R-64852-21591 The sqlite3_backup object is created by a
    ** call to sqlite3_backup_init() and is destroyed by a call to
    ** sqlite3_backup_finish(). */
    sqlite3_free(p);
  }
  sqlite3LeaveMutexAndCloseZombie(pSrcDb);
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Return the number of pages still to be backed up as of the most recent
** call to sqlite3_backup_step().
*/
ಪೂರ್ಣಾಂಕ sqlite3_backup_ಉಳಿದ(sqlite3_backup *p){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( p==0 ){
    (ಶೂನ್ಯ)SQLITE_MISUSE_BKPT;
    ಹಿಂತಿರುಗಿ 0;
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ p->n_ಉಳಿದ;
}

/*
** Return the total number of pages in the source database as of the most 
** recent call to sqlite3_backup_step().
*/
ಪೂರ್ಣಾಂಕ sqlite3_backup_pagecount(sqlite3_backup *p){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( p==0 ){
    (ಶೂನ್ಯ)SQLITE_MISUSE_BKPT;
    ಹಿಂತಿರುಗಿ 0;
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ p->nPagecount;
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
ಸ್ಥಿರ SQLITE_NOINLINE ಶೂನ್ಯ backupUpdate(
  sqlite3_backup *p,
  Pgno i_ಪುಟ,
  ಸ್ಥಿರಾಂಕ u8 *a_ದತ್ತಾಂಶ
){
  ಪ್ರತಿಜ್ಞಾ( p!=0 );
  ಮಾಡು{
    ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(p->pSrc->pBt->mutex) );
    ಆದರೆ( !isFatalError(p->rc) && i_ಪುಟ<p->i_ಮುಂದಿನ ){
      /* The backup process p has already copied page iPage. But now it
      ** has been modified by a transaction on the source pager. Copy
      ** the new data into the backup.
      */
      ಪೂರ್ಣಾಂಕ rc;
      ಪ್ರತಿಜ್ಞಾ( p->pDestDb );
      sqlite3_mutex_enter(p->pDestDb->mutex);
      rc = backup_One_ಪುಟ(p, i_ಪುಟ, a_ದತ್ತಾಂಶ, 1);
      sqlite3_mutex_leave(p->pDestDb->mutex);
      ಪ್ರತಿಜ್ಞಾ( rc!=SQLITE_BUSY && rc!=SQLITE_LOCKED );
      ಆದರೆ( rc!=SQLITE_OK ){
        p->rc = rc;
      }
    }
  }ಆಗ( (p = p->p_ಮುಂದಿನ)!=0 );
}
ಶೂನ್ಯ sqlite3BackupUpdate(sqlite3_backup *pBackup, Pgno i_ಪುಟ, ಸ್ಥಿರಾಂಕ u8 *a_ದತ್ತಾಂಶ){
  ಆದರೆ( pBackup ) backupUpdate(pBackup, i_ಪುಟ, a_ದತ್ತಾಂಶ);
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
ಶೂನ್ಯ sqlite3BackupRestart(sqlite3_backup *pBackup){
  sqlite3_backup *p;                   /* Iterator variable */
  ಗಾಗಿ(p=pBackup; p; p=p->p_ಮುಂದಿನ){
    ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(p->pSrc->pBt->mutex) );
    p->i_ಮುಂದಿನ = 1;
  }
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VACUUM
/*
** Copy the complete content of pBtFrom into pBtTo.  A transaction
** must be active for both files.
**
** The size of file pTo may be reduced by this operation. If anything 
** goes wrong, the transaction on pTo is rolled back. If successful, the 
** transaction is committed before returning.
*/
ಪೂರ್ಣಾಂಕ sqlite3BtreeCopyFile(Btree *p_ಗಮ್ಯ, Btree *pFrom){
  ಪೂರ್ಣಾಂಕ rc;
  sqlite3_file *pFd;              /* File descriptor for database pTo */
  sqlite3_backup b;
  sqlite3BtreeEnter(p_ಗಮ್ಯ);
  sqlite3BtreeEnter(pFrom);

  ಪ್ರತಿಜ್ಞಾ( sqlite3BtreeTxnState(p_ಗಮ್ಯ)==SQLITE_TXN_WRITE );
  pFd = sqlite3PagerFile(sqlite3BtreePager(p_ಗಮ್ಯ));
  ಆದರೆ( pFd->pMethods ){
    i64 nByte = sqlite3_Btree_Get_ಪುಟ_ಗಾತ್ರ(pFrom)*(i64)sqlite3_Btree_ಕೊನೆಯ_ಪುಟ(pFrom);
    rc = sqlite3OsFileControl(pFd, SQLITE_FCNTL_OVERWRITE, &nByte);
    ಆದರೆ( rc==SQLITE_NOTFOUND ) rc = SQLITE_OK;
    ಆದರೆ( rc ) ಹೋಗು copy_ಪೂರ್ಣಗೊಂಡ;
  }

  /* Set up an sqlite3_backup object. sqlite3_backup.pDestDb must be set
  ** to 0. This is used by the implementations of sqlite3_backup_step()
  ** and sqlite3_backup_finish() to detect that they are being called
  ** from this function, not directly by the user.
  */
  memset(&b, 0, ಗಾತ್ರ(b));
  b.pSrcDb = pFrom->db;
  b.pSrc = pFrom;
  b.pDest = p_ಗಮ್ಯ;
  b.i_ಮುಂದಿನ = 1;

  /* 0x7FFFFFFF is the hard limit for the number of pages in a database
  ** file. By passing this as the number of pages to copy to
  ** sqlite3_backup_step(), we can guarantee that the copy finishes 
  ** within a single call (unless an error occurs). The assert() statement
  ** checks this assumption - (p->rc) should be set to either SQLITE_DONE 
  ** or an error code.  */
  sqlite3_backup_ಹೆಜ್ಜೆ(&b, 0x7FFFFFFF);
  ಪ್ರತಿಜ್ಞಾ( b.rc!=SQLITE_OK );

  rc = sqlite3_backup_ಮುಕ್ತಾಯ(&b);
  ಆದರೆ( rc==SQLITE_OK ){
    p_ಗಮ್ಯ->pBt->btsFlags &= ~BTS_PAGESIZE_FIXED;
  }ಇಲ್ಲದಿದ್ದರೆ{
    sqlite3PagerClearCache(sqlite3BtreePager(b.pDest));
  }

  ಪ್ರತಿಜ್ಞಾ( sqlite3BtreeTxnState(p_ಗಮ್ಯ)!=SQLITE_TXN_WRITE );
copy_ಪೂರ್ಣಗೊಂಡ:
  sqlite3BtreeLeave(pFrom);
  sqlite3BtreeLeave(p_ಗಮ್ಯ);
  ಹಿಂತಿರುಗಿ rc;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_VACUUM */
