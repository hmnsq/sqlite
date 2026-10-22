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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "btreeInt.h"

/*
** Structure allocated for each backup operation.
*/
ਬਣਤਰ sqlite3_backup {
  sqlite3* pDestDb;        /* Destination database handle */
  ਅੱਖਰ *zDestDb;
  Btree *pDest;            /* Destination b-tree file */
  u32 iDestSchema;         /* Original schema cookie in destination */
  ਪੂਰਨ bDestLocked;         /* True once a write-transaction is open on pDest */

  Pgno i_ਅਗਲਾ;              /* Page number of the next source page to copy */
  sqlite3* pSrcDb;         /* Source database handle */
  Btree *pSrc;             /* Source b-tree file */

  ਪੂਰਨ rc;                  /* Backup process error code */

  /* These two variables are set by every call to backup_step(). They are
  ** read by calls to backup_remaining() and backup_pagecount().
  */
  Pgno n_ਬਚਿਆ;         /* Number of pages left to copy */
  Pgno nPagecount;         /* Total number of pages to copy */

  ਪੂਰਨ isAttached;          /* True once backup has been registered with pager */
  sqlite3_backup *p_ਅਗਲਾ;   /* Next backup associated with source pager */
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
ਸਥਿਰ Btree *ਲੱਭੋ_Btree(sqlite3 *pErrorDb, sqlite3 *pDb, ਅਚਲ ਅੱਖਰ *zDb){
  ਪੂਰਨ i;

  ਪ੍ਰਤਿਜ੍ਞਾ( pDb!=0 );
  i = sqlite3_ਲੱਭੋ_Db_Name(pDb, zDb);
  ਜੇ( i==1 ){
    Parse sParse;
    ਪੂਰਨ rc = 0;
    sqlite3ParseObjectInit(&sParse,pDb);
    ਜੇ( sqlite3_Open_ਆਰਜ਼ੀ_Database(&sParse) ){
      sqlite3_Error_With_ਸੁਨੇਹਾ(pErrorDb, sParse.rc, "%s", sParse.z_Err_ਸੁਨੇਹਾ);
      rc = SQLITE_ERROR;
    }
    sqlite3DbFree(pErrorDb, sParse.z_Err_ਸੁਨੇਹਾ);
    sqlite3ParseObjectReset(&sParse);
    ਜੇ( rc ){
      ਵਾਪਸ 0;
    }
  }

  ਜੇ( i<0 ){
    sqlite3_Error_With_ਸੁਨੇਹਾ(pErrorDb, SQLITE_ERROR, "unknown database %s", zDb);
    ਵਾਪਸ 0;
  }

  ਵਾਪਸ pDb->aDb[i].pBt;
}

/*
** Attempt to set the page size of the destination to match the page size
** of the source.
*/
ਸਥਿਰ ਪੂਰਨ ਸੈੱਟ_Dest_Pgsz(Btree *pDest, Btree *pSrc){
  ਵਾਪਸ sqlite3_Btree_ਸੈੱਟ_ਪੰਨਾ_ਮਾਪ(pDest, sqlite3_Btree_Get_ਪੰਨਾ_ਮਾਪ(pSrc), 0, 0);
}

/*
** Check that there is no open read-transaction on the b-tree passed as the
** second argument. If there is not, return SQLITE_OK. Otherwise, if there
** is an open read-transaction, return SQLITE_ERROR and leave an error 
** message in database handle db.
*/
ਸਥਿਰ ਪੂਰਨ checkReadTransaction(sqlite3 *db, Btree *p){
  ਜੇ( sqlite3BtreeTxnState(p)!=SQLITE_TXN_NONE ){
    sqlite3_Error_With_ਸੁਨੇਹਾ(db, SQLITE_ERROR, "destination database is in use");
    ਵਾਪਸ SQLITE_ERROR;
  }
  ਵਾਪਸ SQLITE_OK;
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
  ਅਚਲ ਅੱਖਰ *zDestDb,                  /* Name of database within pDestDb */
  sqlite3* pSrcDb,                      /* Database connection to read from */
  ਅਚਲ ਅੱਖਰ *zSrcDb                    /* Name of database within pSrcDb */
){
  sqlite3_backup *p;                    /* Value to return */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !sqlite3SafetyCheckOk(pSrcDb)||!sqlite3SafetyCheckOk(pDestDb) ){
    (ਖਾਲੀ)SQLITE_MISUSE_BKPT;
    ਵਾਪਸ 0;
  }
#ਜੇ_ਅੰਤ

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

  ਜੇ( pSrcDb==pDestDb ){
    sqlite3_Error_With_ਸੁਨੇਹਾ(
        pDestDb, SQLITE_ERROR, "source and destination must be distinct"
    );
    p = 0;
  }ਨਹੀਂਤਾਂ {
    ਪੂਰਨ nDest = sqlite3Strlen30(zDestDb);

    /* Allocate space for a new sqlite3_backup object...
    ** EVIDENCE-OF: R-64852-21591 The sqlite3_backup object is created by a
    ** call to sqlite3_backup_init() and is destroyed by a call to
    ** sqlite3_backup_finish(). */
    p = (sqlite3_backup*)sqlite3MallocZero(ਆਕਾਰ(sqlite3_backup)+nDest+1);
    ਜੇ( !p ){
      sqlite3Error(pDestDb, SQLITE_NOMEM_BKPT);
    }ਨਹੀਂਤਾਂ{
      p->zDestDb = (ਅੱਖਰ*)&p[1];
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(p->zDestDb, zDestDb, nDest);
    }
  }

  /* If the allocation succeeded, populate the new object. */
  ਜੇ( p ){
    /* Do not store the pointer to the destination b-tree at this point.
    ** This is because there is nothing preventing it from being detached
    ** or otherwise freed before the first call to sqlite3_backup_step()
    ** on this object. The source b-tree does not have this problem, as
    ** incrementing Btree.nBackup (see below) effectively locks the object. */
    Btree *pDest = ਲੱਭੋ_Btree(pDestDb, pDestDb, zDestDb);
    p->pSrc = ਲੱਭੋ_Btree(pDestDb, pSrcDb, zSrcDb);
    p->pDestDb = pDestDb;
    p->pSrcDb = pSrcDb;
    p->i_ਅਗਲਾ = 1;
    p->isAttached = 0;

    ਜੇ( 0==p->pSrc || 0==pDest 
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
  ਜੇ( p ){
    p->pSrc->nBackup++;
  }

  sqlite3_mutex_leave(pDestDb->mutex);
  sqlite3_mutex_leave(pSrcDb->mutex);
  ਵਾਪਸ p;
}

/*
** Argument rc is an SQLite error code. Return true if this error is 
** considered fatal if encountered during a backup operation. All errors
** are considered fatal except for SQLITE_BUSY and SQLITE_LOCKED.
*/
ਸਥਿਰ ਪੂਰਨ isFatalError(ਪੂਰਨ rc){
  ਵਾਪਸ (rc!=SQLITE_OK && rc!=SQLITE_BUSY && ALWAYS(rc!=SQLITE_LOCKED));
}

/*
** Parameter zSrcData points to a buffer containing the data for 
** page iSrcPg from the source database. Copy this data into the 
** destination database.
*/
ਸਥਿਰ ਪੂਰਨ backup_One_ਪੰਨਾ(
  sqlite3_backup *p,              /* Backup handle */
  Pgno iSrcPg,                    /* Source database page to backup */
  ਅਚਲ u8 *z_Src_ਅੰਕੜੇ,             /* Source database page data */
  ਪੂਰਨ bUpdate                     /* True for an update, false otherwise */
){
  Pager * ਅਚਲ pDestPager = sqlite3BtreePager(p->pDest);
  ਅਚਲ ਪੂਰਨ nSrcPgsz = sqlite3_Btree_Get_ਪੰਨਾ_ਮਾਪ(p->pSrc);
  ਪੂਰਨ nDestPgsz = sqlite3_Btree_Get_ਪੰਨਾ_ਮਾਪ(p->pDest);
  ਅਚਲ ਪੂਰਨ nCopy = MIN(nSrcPgsz, nDestPgsz);
  ਅਚਲ i64 i_ਸਿਰਾ = (i64)iSrcPg*(i64)nSrcPgsz;
  ਪੂਰਨ rc = SQLITE_OK;
  i64 iOff;

  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3BtreeGetReserveNoMutex(p->pSrc)>=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p->bDestLocked );
  ਪ੍ਰਤਿਜ੍ਞਾ( !isFatalError(p->rc) );
  ਪ੍ਰਤਿਜ੍ਞਾ( iSrcPg!=PENDING_BYTE_ਪੰਨਾ(p->pSrc->pBt) );
  ਪ੍ਰਤਿਜ੍ਞਾ( z_Src_ਅੰਕੜੇ );
  ਪ੍ਰਤਿਜ੍ਞਾ( nSrcPgsz==nDestPgsz || sqlite3PagerIsMemdb(pDestPager)==0 );

  /* This loop runs once for each destination page spanned by the source 
  ** page. For each iteration, variable iOff is set to the byte offset
  ** of the destination page.
  */
  ਲਈ(iOff=i_ਸਿਰਾ-(i64)nSrcPgsz; rc==SQLITE_OK && iOff<i_ਸਿਰਾ; iOff+=nDestPgsz){
    Db_ਪੰਨਾ *pDestPg = 0;
    Pgno iDest = (Pgno)(iOff/nDestPgsz)+1;
    ਜੇ( iDest==PENDING_BYTE_ਪੰਨਾ(p->pDest->pBt) ) ਜਾਰੀ;
    ਜੇ( SQLITE_OK==(rc = sqlite3PagerGet(pDestPager, iDest, &pDestPg, 0))
     && SQLITE_OK==(rc = sqlite3PagerWrite(pDestPg))
    ){
      ਅਚਲ u8 *z_ਵਿੱਚ = &z_Src_ਅੰਕੜੇ[iOff%nSrcPgsz];
      u8 *z_Dest_ਅੰਕੜੇ = sqlite3_Pager_Get_ਅੰਕੜੇ(pDestPg);
      u8 *z_ਬਾਹਰਲਾ = &z_Dest_ਅੰਕੜੇ[iOff%nDestPgsz];

      /* Copy the data from the source page into the destination page.
      ** Then clear the Btree layer MemPage.isInit flag. Both this module
      ** and the pager code use this trick (clearing the first byte
      ** of the page 'extra' space to invalidate the Btree layers
      ** cached parse of the page). MemPage.isInit is marked 
      ** "MUST BE FIRST" for this purpose.
      */
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(z_ਬਾਹਰਲਾ, z_ਵਿੱਚ, nCopy);
      ((u8 *)sqlite3PagerGetExtra(pDestPg))[0] = 0;
      ਜੇ( iOff==0 && bUpdate==0 ){
        sqlite3Put4byte(&z_ਬਾਹਰਲਾ[28], sqlite3_Btree_ਆਖਰੀ_ਪੰਨਾ(p->pSrc));
      }
    }
    sqlite3PagerUnref(pDestPg);
  }

  ਵਾਪਸ rc;
}

/*
** If pFile is currently larger than iSize bytes, then truncate it to
** exactly iSize bytes. If pFile is not larger than iSize bytes, then
** this function is a no-op.
**
** Return SQLITE_OK if everything is successful, or an SQLite error 
** code if an error occurs.
*/
ਸਥਿਰ ਪੂਰਨ backupTruncateFile(sqlite3_file *pFile, i64 i_ਮਾਪ){
  i64 iCurrent;
  ਪੂਰਨ rc = sqlite3_Os_File_ਮਾਪ(pFile, &iCurrent);
  ਜੇ( rc==SQLITE_OK && iCurrent>i_ਮਾਪ ){
    rc = sqlite3OsTruncate(pFile, i_ਮਾਪ);
  }
  ਵਾਪਸ rc;
}

/*
** Register this backup object with the associated source pager for
** callbacks when pages are changed or the cache invalidated.
*/
ਸਥਿਰ ਖਾਲੀ attachBackupObject(sqlite3_backup *p){
  sqlite3_backup **pp;
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3BtreeHoldsMutex(p->pSrc) );
  pp = sqlite3PagerBackupPtr(sqlite3BtreePager(p->pSrc));
  p->p_ਅਗਲਾ = *pp;
  *pp = p;
  p->isAttached = 1;
}

/*
** Copy nPage pages from the source b-tree to the destination.
*/
ਪੂਰਨ sqlite3_backup_ਕਦਮ(sqlite3_backup *p, ਪੂਰਨ n_ਪੰਨਾ){
  ਪੂਰਨ rc;
  ਪੂਰਨ dest_ਬਹੁਲਕ = 0;   /* Destination journal mode */
  ਪੂਰਨ pgszSrc = 0;    /* Source page size */
  ਪੂਰਨ pgszDest = 0;   /* Destination page size */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( p==0 ) ਵਾਪਸ SQLITE_MISUSE_BKPT;
#ਜੇ_ਅੰਤ
  sqlite3_mutex_enter(p->pSrcDb->mutex);
  sqlite3BtreeEnter(p->pSrc);
  ਜੇ( p->pDestDb ){
    sqlite3_mutex_enter(p->pDestDb->mutex);
  }

  rc = p->rc;
  ਜੇ( !isFatalError(rc) ){
    Pager * ਅਚਲ pSrcPager = sqlite3BtreePager(p->pSrc);     /* Source pager */
    Btree * pDest = 0;                                        /* Dest btree */
    Pager * pDestPager = 0;                                   /* Dest pager */
    ਪੂਰਨ ii;                            /* Iterator variable */
    ਪੂਰਨ n_Src_ਪੰਨਾ = -1;                 /* Size of source db in pages */
    ਪੂਰਨ bCloseTrans = 0;               /* True if src db requires unlocking */

    /* If the source pager is currently in a write-transaction, return
    ** SQLITE_BUSY immediately.
    */
    ਜੇ( p->pDestDb && p->pSrc->pBt->ਵਿੱਚ_Transaction==TRANS_WRITE ){
      rc = SQLITE_BUSY;
    }ਨਹੀਂਤਾਂ{
      rc = SQLITE_OK;
    }


    /* If there is no open read-transaction on the source database, open
    ** one now. If a transaction is opened here, then it will be closed
    ** before this function exits.
    */
    ਜੇ( rc==SQLITE_OK && SQLITE_TXN_NONE==sqlite3BtreeTxnState(p->pSrc) ){
      rc = sqlite3BtreeBeginTrans(p->pSrc, 0, 0);
      bCloseTrans = 1;
    }

    /* Locate the destination btree and pager. */
    ਜੇ( (pDest = p->pDest)==0 ){
      pDest = ਲੱਭੋ_Btree(p->pDestDb, p->pDestDb, p->zDestDb);
    }
    ਜੇ( pDest==0 ){
      rc = SQLITE_ERROR;
    }ਨਹੀਂਤਾਂ{
      pDestPager = sqlite3BtreePager(pDest);
    }

    /* If the destination database has not yet been locked (i.e. if this
    ** is the first call to backup_step() for the current backup operation),
    ** try to set its page size to the same as the source database. This
    ** is especially important on ZipVFS systems, as in that case it is
    ** not possible to create a database file that uses one page size by
    ** writing to it with another.  */
    ਜੇ( p->bDestLocked==0 && rc==SQLITE_OK 
     && ਸੈੱਟ_Dest_Pgsz(pDest, p->pSrc)==SQLITE_NOMEM
    ){
      rc = SQLITE_NOMEM;
    }

    /* Lock the destination database, if it is not locked already. */
    ਜੇ( SQLITE_OK==rc && p->bDestLocked==0
     && SQLITE_OK==(rc = sqlite3BtreeBeginTrans(pDest, 2,
                                                (ਪੂਰਨ*)&p->iDestSchema)) 
    ){
      p->bDestLocked = 1;
      p->pDest = pDest;
    }

    /* Do not allow backup if the destination database is in WAL mode
    ** and the page sizes are different between source and destination */
    ਜੇ( rc==SQLITE_OK ){
      pgszSrc = sqlite3_Btree_Get_ਪੰਨਾ_ਮਾਪ(p->pSrc);
      pgszDest = sqlite3_Btree_Get_ਪੰਨਾ_ਮਾਪ(p->pDest);
      dest_ਬਹੁਲਕ = sqlite3_Pager_Get_Journal_ਬਹੁਲਕ(sqlite3BtreePager(p->pDest));
      ਜੇ( (dest_ਬਹੁਲਕ==PAGER_JOURNALMODE_WAL || sqlite3PagerIsMemdb(pDestPager))
        && pgszSrc!=pgszDest 
      ){
        rc = SQLITE_READONLY;
      }
    }
  
    /* Now that there is a read-lock on the source database, query the
    ** source pager for the number of pages in the database.
    */
    n_Src_ਪੰਨਾ = (ਪੂਰਨ)sqlite3_Btree_ਆਖਰੀ_ਪੰਨਾ(p->pSrc);
    ਪ੍ਰਤਿਜ੍ਞਾ( n_Src_ਪੰਨਾ>=0 );
    ਲਈ(ii=0; (n_ਪੰਨਾ<0 || ii<n_ਪੰਨਾ) && p->i_ਅਗਲਾ<=(Pgno)n_Src_ਪੰਨਾ && !rc; ii++){
      ਅਚਲ Pgno iSrcPg = p->i_ਅਗਲਾ;                 /* Source page number */
      ਜੇ( iSrcPg!=PENDING_BYTE_ਪੰਨਾ(p->pSrc->pBt) ){
        Db_ਪੰਨਾ *pSrcPg;                             /* Source page object */
        rc = sqlite3PagerGet(pSrcPager, iSrcPg, &pSrcPg,PAGER_GET_READONLY);
        ਜੇ( rc==SQLITE_OK ){
          rc = backup_One_ਪੰਨਾ(p, iSrcPg, sqlite3_Pager_Get_ਅੰਕੜੇ(pSrcPg), 0);
          sqlite3PagerUnref(pSrcPg);
        }
      }
      p->i_ਅਗਲਾ++;
    }
    ਜੇ( rc==SQLITE_OK ){
      p->nPagecount = n_Src_ਪੰਨਾ;
      p->n_ਬਚਿਆ = n_Src_ਪੰਨਾ+1-p->i_ਅਗਲਾ;
      ਜੇ( p->i_ਅਗਲਾ>(Pgno)n_Src_ਪੰਨਾ ){
        rc = SQLITE_ਪੂਰਾ;
      }ਨਹੀਂਤਾਂ ਜੇ( !p->isAttached ){
        attachBackupObject(p);
      }
    }
  
    /* Update the schema version field in the destination database. This
    ** is to make sure that the schema-version really does change in
    ** the case where the source and destination databases have the
    ** same schema version.
    */
    ਜੇ( rc==SQLITE_ਪੂਰਾ ){
      ਜੇ( n_Src_ਪੰਨਾ==0 ){
        rc = sqlite3BtreeNewDb(p->pDest);
        n_Src_ਪੰਨਾ = 1;
      }
      ਜੇ( rc==SQLITE_OK || rc==SQLITE_ਪੂਰਾ ){
        rc = sqlite3BtreeUpdateMeta(p->pDest,1,p->iDestSchema+1);
      }
      ਜੇ( rc==SQLITE_OK ){
        ਜੇ( p->pDestDb ){
          sqlite3_Reset_All_Schemas_ਦਾ_Connection(p->pDestDb);
        }
        ਜੇ( dest_ਬਹੁਲਕ==PAGER_JOURNALMODE_WAL ){
          rc = sqlite3_Btree_ਸੈੱਟ_ਸੰਸਕਰਣ(p->pDest, 2);
        }
      }
      ਜੇ( rc==SQLITE_OK ){
        ਪੂਰਨ nDestTruncate;
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
        ਪ੍ਰਤਿਜ੍ਞਾ( pgszSrc==sqlite3_Btree_Get_ਪੰਨਾ_ਮਾਪ(p->pSrc) );
        ਪ੍ਰਤਿਜ੍ਞਾ( pgszDest==sqlite3_Btree_Get_ਪੰਨਾ_ਮਾਪ(p->pDest) );
        ਜੇ( pgszSrc<pgszDest ){
          ਪੂਰਨ ਅਨੁਪਾਤ = pgszDest/pgszSrc;
          nDestTruncate = (n_Src_ਪੰਨਾ+ਅਨੁਪਾਤ-1)/ਅਨੁਪਾਤ;
          ਜੇ( nDestTruncate==(ਪੂਰਨ)PENDING_BYTE_ਪੰਨਾ(p->pDest->pBt) ){
            nDestTruncate--;
          }
        }ਨਹੀਂਤਾਂ{
          nDestTruncate = n_Src_ਪੰਨਾ * (pgszSrc/pgszDest);
        }
        ਪ੍ਰਤਿਜ੍ਞਾ( nDestTruncate>0 );

        ਜੇ( pgszSrc<pgszDest ){
          /* If the source page-size is smaller than the destination page-size,
          ** two extra things may need to happen:
          **
          **   * The destination may need to be truncated, and
          **
          **   * Data stored on the pages immediately following the 
          **     pending-byte page in the source database may need to be
          **     copied into the destination database.
          */
          ਅਚਲ i64 i_ਮਾਪ = (i64)pgszSrc * (i64)n_Src_ਪੰਨਾ;
          sqlite3_file * ਅਚਲ pFile = sqlite3PagerFile(pDestPager);
          Pgno iPg;
          ਪੂਰਨ n_Dst_ਪੰਨਾ;
          i64 iOff;
          i64 i_ਸਿਰਾ;

          ਪ੍ਰਤਿਜ੍ਞਾ( pFile );
          ਪ੍ਰਤਿਜ੍ਞਾ( nDestTruncate==0 
              || (i64)nDestTruncate*(i64)pgszDest >= i_ਮਾਪ || (
                nDestTruncate==(ਪੂਰਨ)(PENDING_BYTE_ਪੰਨਾ(p->pDest->pBt)-1)
             && i_ਮਾਪ>=PENDING_BYTE && i_ਮਾਪ<=PENDING_BYTE+pgszDest
          ));

          /* This block ensures that all data required to recreate the original
          ** database has been stored in the journal for pDestPager and the
          ** journal synced to disk. So at this point we may safely modify
          ** the database file in any way, knowing that if a power failure
          ** occurs, the original database will be reconstructed from the 
          ** journal file.  */
          sqlite3PagerPagecount(pDestPager, &n_Dst_ਪੰਨਾ);
          ਲਈ(iPg=nDestTruncate; rc==SQLITE_OK && iPg<=(Pgno)n_Dst_ਪੰਨਾ; iPg++){
            ਜੇ( iPg!=PENDING_BYTE_ਪੰਨਾ(p->pDest->pBt) ){
              Db_ਪੰਨਾ *pPg;
              rc = sqlite3PagerGet(pDestPager, iPg, &pPg, 0);
              ਜੇ( rc==SQLITE_OK ){
                rc = sqlite3PagerWrite(pPg);
                sqlite3PagerUnref(pPg);
              }
            }
          }
          ਜੇ( rc==SQLITE_OK ){
            rc = sqlite3PagerCommitPhaseOne(pDestPager, 0, 1);
          }

          /* Write the extra pages and truncate the database file as required */
          i_ਸਿਰਾ = MIN(PENDING_BYTE + pgszDest, i_ਮਾਪ);
          ਲਈ(
            iOff=PENDING_BYTE+pgszSrc; 
            rc==SQLITE_OK && iOff<i_ਸਿਰਾ; 
            iOff+=pgszSrc
          ){
            PgHdr *pSrcPg = 0;
            ਅਚਲ Pgno iSrcPg = (Pgno)((iOff/pgszSrc)+1);
            rc = sqlite3PagerGet(pSrcPager, iSrcPg, &pSrcPg, 0);
            ਜੇ( rc==SQLITE_OK ){
              u8 *z_ਅੰਕੜੇ = sqlite3_Pager_Get_ਅੰਕੜੇ(pSrcPg);
              rc = sqlite3OsWrite(pFile, z_ਅੰਕੜੇ, pgszSrc, iOff);
            }
            sqlite3PagerUnref(pSrcPg);
          }
          ਜੇ( rc==SQLITE_OK ){
            rc = backupTruncateFile(pFile, i_ਮਾਪ);
          }

          /* Sync the database file to disk. */
          ਜੇ( rc==SQLITE_OK ){
            rc = sqlite3PagerSync(pDestPager, 0);
          }
        }ਨਹੀਂਤਾਂ{
          sqlite3PagerTruncateImage(pDestPager, nDestTruncate);
          rc = sqlite3PagerCommitPhaseOne(pDestPager, 0, 0);
        }
    
        /* Finish committing the transaction to the destination database. */
        ਜੇ( SQLITE_OK==rc
         && SQLITE_OK==(rc = sqlite3BtreeCommitPhaseTwo(p->pDest, 0))
        ){
          rc = SQLITE_ਪੂਰਾ;
        }
      }
    }
  
    /* If bCloseTrans is true, then this function opened a read transaction
    ** on the source database. Close the read transaction here. There is
    ** no need to check the return values of the btree methods here, as
    ** "committing" a read-only transaction cannot fail.
    */
    ਜੇ( bCloseTrans ){
      TESTONLY( ਪੂਰਨ rc2 );
      TESTONLY( rc2  = ) sqlite3BtreeCommitPhaseOne(p->pSrc, 0);
      TESTONLY( rc2 |= ) sqlite3BtreeCommitPhaseTwo(p->pSrc, 0);
      ਪ੍ਰਤਿਜ੍ਞਾ( rc2==SQLITE_OK );
    }
  
    ਜੇ( rc==SQLITE_IOERR_NOMEM ){
      rc = SQLITE_NOMEM_BKPT;
    }
    p->rc = rc;
  }
  ਜੇ( p->pDestDb ){
    sqlite3_mutex_leave(p->pDestDb->mutex);
  }
  sqlite3BtreeLeave(p->pSrc);
  sqlite3_mutex_leave(p->pSrcDb->mutex);
  ਵਾਪਸ rc;
}

/*
** Release all resources associated with an sqlite3_backup* handle.
*/
ਪੂਰਨ sqlite3_backup_ਅੰਤ(sqlite3_backup *p){
  sqlite3_backup **pp;                 /* Ptr to head of pagers backup list */
  sqlite3 *pSrcDb;                     /* Source database connection */
  ਪੂਰਨ rc;                              /* Value to return */

  /* Enter the mutexes */
  ਜੇ( p==0 ) ਵਾਪਸ SQLITE_OK;
  pSrcDb = p->pSrcDb;
  sqlite3_mutex_enter(pSrcDb->mutex);
  sqlite3BtreeEnter(p->pSrc);
  ਜੇ( p->pDestDb ){
    sqlite3_mutex_enter(p->pDestDb->mutex);
  }

  /* Detach this backup from the source pager. */
  ਜੇ( p->pDestDb ){
    p->pSrc->nBackup--;
  }
  ਜੇ( p->isAttached ){
    pp = sqlite3PagerBackupPtr(sqlite3BtreePager(p->pSrc));
    ਪ੍ਰਤਿਜ੍ਞਾ( pp!=0 );
    ਜਦੋਂ( *pp!=p ){
      pp = &(*pp)->p_ਅਗਲਾ;
      ਪ੍ਰਤਿਜ੍ਞਾ( pp!=0 );
    }
    *pp = p->p_ਅਗਲਾ;
  }

  /* If a transaction is still open on the Btree, roll it back. */
  ਜੇ( p->pDest ){
    sqlite3BtreeRollback(p->pDest, SQLITE_OK, 0);
  }

  /* Set the error code of the destination database handle. */
  rc = (p->rc==SQLITE_ਪੂਰਾ) ? SQLITE_OK : p->rc;
  ਜੇ( p->pDestDb ){
    sqlite3Error(p->pDestDb, rc);

    /* Exit the mutexes and free the backup context structure. */
    sqlite3LeaveMutexAndCloseZombie(p->pDestDb);
  }
  sqlite3BtreeLeave(p->pSrc);
  ਜੇ( p->pDestDb ){
    /* EVIDENCE-OF: R-64852-21591 The sqlite3_backup object is created by a
    ** call to sqlite3_backup_init() and is destroyed by a call to
    ** sqlite3_backup_finish(). */
    sqlite3_free(p);
  }
  sqlite3LeaveMutexAndCloseZombie(pSrcDb);
  ਵਾਪਸ rc;
}

/*
** Return the number of pages still to be backed up as of the most recent
** call to sqlite3_backup_step().
*/
ਪੂਰਨ sqlite3_backup_ਬਚਿਆ(sqlite3_backup *p){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( p==0 ){
    (ਖਾਲੀ)SQLITE_MISUSE_BKPT;
    ਵਾਪਸ 0;
  }
#ਜੇ_ਅੰਤ
  ਵਾਪਸ p->n_ਬਚਿਆ;
}

/*
** Return the total number of pages in the source database as of the most 
** recent call to sqlite3_backup_step().
*/
ਪੂਰਨ sqlite3_backup_pagecount(sqlite3_backup *p){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( p==0 ){
    (ਖਾਲੀ)SQLITE_MISUSE_BKPT;
    ਵਾਪਸ 0;
  }
#ਜੇ_ਅੰਤ
  ਵਾਪਸ p->nPagecount;
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
ਸਥਿਰ SQLITE_NOINLINE ਖਾਲੀ backupUpdate(
  sqlite3_backup *p,
  Pgno i_ਪੰਨਾ,
  ਅਚਲ u8 *a_ਅੰਕੜੇ
){
  ਪ੍ਰਤਿਜ੍ਞਾ( p!=0 );
  ਕਰੋ{
    ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(p->pSrc->pBt->mutex) );
    ਜੇ( !isFatalError(p->rc) && i_ਪੰਨਾ<p->i_ਅਗਲਾ ){
      /* The backup process p has already copied page iPage. But now it
      ** has been modified by a transaction on the source pager. Copy
      ** the new data into the backup.
      */
      ਪੂਰਨ rc;
      ਪ੍ਰਤਿਜ੍ਞਾ( p->pDestDb );
      sqlite3_mutex_enter(p->pDestDb->mutex);
      rc = backup_One_ਪੰਨਾ(p, i_ਪੰਨਾ, a_ਅੰਕੜੇ, 1);
      sqlite3_mutex_leave(p->pDestDb->mutex);
      ਪ੍ਰਤਿਜ੍ਞਾ( rc!=SQLITE_BUSY && rc!=SQLITE_LOCKED );
      ਜੇ( rc!=SQLITE_OK ){
        p->rc = rc;
      }
    }
  }ਜਦੋਂ( (p = p->p_ਅਗਲਾ)!=0 );
}
ਖਾਲੀ sqlite3BackupUpdate(sqlite3_backup *pBackup, Pgno i_ਪੰਨਾ, ਅਚਲ u8 *a_ਅੰਕੜੇ){
  ਜੇ( pBackup ) backupUpdate(pBackup, i_ਪੰਨਾ, a_ਅੰਕੜੇ);
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
ਖਾਲੀ sqlite3BackupRestart(sqlite3_backup *pBackup){
  sqlite3_backup *p;                   /* Iterator variable */
  ਲਈ(p=pBackup; p; p=p->p_ਅਗਲਾ){
    ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(p->pSrc->pBt->mutex) );
    p->i_ਅਗਲਾ = 1;
  }
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VACUUM
/*
** Copy the complete content of pBtFrom into pBtTo.  A transaction
** must be active for both files.
**
** The size of file pTo may be reduced by this operation. If anything 
** goes wrong, the transaction on pTo is rolled back. If successful, the 
** transaction is committed before returning.
*/
ਪੂਰਨ sqlite3BtreeCopyFile(Btree *p_ਮੰਜ਼ਿਲ, Btree *pFrom){
  ਪੂਰਨ rc;
  sqlite3_file *pFd;              /* File descriptor for database pTo */
  sqlite3_backup b;
  sqlite3BtreeEnter(p_ਮੰਜ਼ਿਲ);
  sqlite3BtreeEnter(pFrom);

  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3BtreeTxnState(p_ਮੰਜ਼ਿਲ)==SQLITE_TXN_WRITE );
  pFd = sqlite3PagerFile(sqlite3BtreePager(p_ਮੰਜ਼ਿਲ));
  ਜੇ( pFd->pMethods ){
    i64 nByte = sqlite3_Btree_Get_ਪੰਨਾ_ਮਾਪ(pFrom)*(i64)sqlite3_Btree_ਆਖਰੀ_ਪੰਨਾ(pFrom);
    rc = sqlite3OsFileControl(pFd, SQLITE_FCNTL_OVERWRITE, &nByte);
    ਜੇ( rc==SQLITE_NOTFOUND ) rc = SQLITE_OK;
    ਜੇ( rc ) ਜਾਓ copy_ਮੁਕੰਮਲ;
  }

  /* Set up an sqlite3_backup object. sqlite3_backup.pDestDb must be set
  ** to 0. This is used by the implementations of sqlite3_backup_step()
  ** and sqlite3_backup_finish() to detect that they are being called
  ** from this function, not directly by the user.
  */
  memset(&b, 0, ਆਕਾਰ(b));
  b.pSrcDb = pFrom->db;
  b.pSrc = pFrom;
  b.pDest = p_ਮੰਜ਼ਿਲ;
  b.i_ਅਗਲਾ = 1;

  /* 0x7FFFFFFF is the hard limit for the number of pages in a database
  ** file. By passing this as the number of pages to copy to
  ** sqlite3_backup_step(), we can guarantee that the copy finishes 
  ** within a single call (unless an error occurs). The assert() statement
  ** checks this assumption - (p->rc) should be set to either SQLITE_DONE 
  ** or an error code.  */
  sqlite3_backup_ਕਦਮ(&b, 0x7FFFFFFF);
  ਪ੍ਰਤਿਜ੍ਞਾ( b.rc!=SQLITE_OK );

  rc = sqlite3_backup_ਅੰਤ(&b);
  ਜੇ( rc==SQLITE_OK ){
    p_ਮੰਜ਼ਿਲ->pBt->btsFlags &= ~BTS_PAGESIZE_FIXED;
  }ਨਹੀਂਤਾਂ{
    sqlite3PagerClearCache(sqlite3BtreePager(b.pDest));
  }

  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3BtreeTxnState(p_ਮੰਜ਼ਿਲ)!=SQLITE_TXN_WRITE );
copy_ਮੁਕੰਮਲ:
  sqlite3BtreeLeave(pFrom);
  sqlite3BtreeLeave(p_ਮੰਜ਼ਿਲ);
  ਵਾਪਸ rc;
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_VACUUM */
