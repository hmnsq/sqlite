/*
** 2004 April 6
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file implements an external (disk-based) database using BTrees.
** See the header comment on "btreeInt.h" for additional information.
** Including a description of file format and an overview of operation.
*/
#ضمن "btreeInt.h"

/*
** Suppress false-positive compiler warnings from GCC.  Warnings are
** re-enabled at the bottom of this source file.
*/
#إذا defined(__GNUC__) && __GNUC__>=11
# pragma GCC diagnostic ئىتتىرىش
# pragma GCC diagnostic ignored "-Wstringop-overread"
# pragma GCC diagnostic ignored "-Wstringop-overflow"
#endif

/*
** The header string that appears at the beginning of every
** SQLite database.
*/
ساكن ثابت حرف zMagicHeader[] = SQLITE_FILE_HEADER;

/*
** Set this global variable to 1 to enable tracing using the TRACE
** macro.
*/
#إذا 0
صحيح sqlite3BtreeTrace=1;  /* True to enable tracing */
# define TRACE(X)  إذا(sqlite3BtreeTrace){printf X;fflush(stdout);}
#وإلا
# define TRACE(X)
#endif

/*
** Extract a 2-byte big-endian integer from an array of unsigned bytes.
** But if the value is zero, make it 65536.
**
** This routine is used to extract the "offset to cell content area" value
** from the header of a btree page.  If the page size is 65536 and the page
** is empty, the offset should be 65536, but the 2-byte value stores zero.
** This routine makes the necessary adjustment to 65536.
*/
#define get2byteNotZero(X)  (((((صحيح)get2byte(X))-1)&0xffff)+1)

/*
** Values passed as the 5th argument to allocateBtreePage()
*/
#define BTALLOC_ANY   0           /* Allocate any page */
#define BTALLOC_EXACT 1           /* Allocate exact page if possible */
#define BTALLOC_LE    2           /* Allocate any page <= the parameter */

/*
** Macro IfNotOmitAV(x) returns (x) if SQLITE_OMIT_AUTOVACUUM is not
** defined, or 0 if it is. For example:
**
**   bIncrVacuum = IfNotOmitAV(pBtShared->incrVacuum);
*/
#ifndef SQLITE_OMIT_AUTOVACUUM
#define IfNotOmitAV(ئىپادە_2) (ئىپادە_2)
#وإلا
#define IfNotOmitAV(ئىپادە_2) 0
#endif

#ifndef SQLITE_OMIT_SHARED_CACHE
/*
** A list of BtShared objects that are eligible for participation
** in shared cache.  This variable has file scope during normal builds,
** but the test harness needs to access it so we make it global for
** test builds.
**
** Access to this variable is protected by SQLITE_MUTEX_STATIC_MAIN.
*/
#ifdef SQLITE_TEST
BtShared *SQLITE_WSD sqlite3SharedCacheList = 0;
#وإلا
ساكن BtShared *SQLITE_WSD sqlite3SharedCacheList = 0;
#endif
#endif /* SQLITE_OMIT_SHARED_CACHE */

#ifndef SQLITE_OMIT_SHARED_CACHE
/*
** Enable or disable the shared pager and schema features.
**
** This routine has no effect on existing database connections.
** The shared cache setting effects only future calls to
** sqlite3_open(), sqlite3_open16(), or sqlite3_open_v2().
*/
صحيح sqlite3_enable_shared_cache(صحيح enable){
  sqlite3GlobalConfig.sharedCacheEnabled = enable;
  أرجع SQLITE_OK;
}
#endif



#ifdef SQLITE_OMIT_SHARED_CACHE
  /*
  ** The functions querySharedCacheTableLock(), setSharedCacheTableLock(),
  ** and clearAllSharedCacheTableLocks()
  ** manipulate entries in the BtShared.pLock linked list used to store
  ** shared-cache table level locks. If the library is compiled with the
  ** shared-cache feature disabled, then there is only ever one user
  ** of each BtShared structure and so this locking is not necessary.
  ** So define the lock related functions as no-ops.
  */
  #define querySharedCacheTableLock(a,b,c) SQLITE_OK
  #define setSharedCacheTableLock(a,b,c) SQLITE_OK
  #define clearAllSharedCacheTableLocks(a)
  #define downgradeAllSharedCacheTableLocks(a)
  #define hasSharedCacheTableLock(a,b,c,d) 1
  #define hasReadConflicts(a, b) 0
#endif

#ifdef SQLITE_DEBUG
/*
** Return and reset the seek counter for a Btree object.
*/
sqlite3_uint64 sqlite3_Btree_Seek_سانى(Btree *pBt){
  u64 n =  pBt->nSeek;
  pBt->nSeek = 0;
  أرجع n;
}
#endif

/*
** Implementation of the SQLITE_CORRUPT_PAGE() macro. Takes a single
** (MemPage*) as an argument. The (MemPage*) must not be NULL.
**
** If SQLITE_DEBUG is not defined, then this macro is equivalent to
** SQLITE_CORRUPT_BKPT. Or, if SQLITE_DEBUG is set, then the log message
** normally produced as a side-effect of SQLITE_CORRUPT_BKPT is augmented
** with the page number and filename associated with the (MemPage*).
*/
#ifdef SQLITE_DEBUG
صحيح corruptPageError(صحيح lineno, MemPage *p){
  حرف *z_ئۇچۇر;
  sqlite3BeginBenignMalloc();
  z_ئۇچۇر = sqlite3_mprintf("database corruption page %u of %s",
             p->pgno, sqlite3PagerFilename(p->pBt->pPager, 0)
  );
  sqlite3EndBenignMalloc();
  إذا( z_ئۇچۇر ){
    sqlite3ReportError(SQLITE_CORRUPT, lineno, z_ئۇچۇر);
  }
  sqlite3_free(z_ئۇچۇر);
  أرجع SQLITE_CORRUPT_BKPT;
}
# define SQLITE_CORRUPT_PAGE(pMemPage) corruptPageError(__LINE__, pMemPage)
#وإلا
# define SQLITE_CORRUPT_PAGE(pMemPage) SQLITE_CORRUPT_PGNO(pMemPage->pgno)
#endif

/* Default value for SHARED_LOCK_TRACE macro if shared-cache is disabled
** or if the lock tracking is disabled.  This is always the value for
** release builds.
*/
#define SHARED_LOCK_TRACE(X,ئۇچۇر,TAB,TYPE)  /*no-op*/

#ifndef SQLITE_OMIT_SHARED_CACHE

#إذا 0
/*  ^----  Change to 1 and recompile to enable shared-lock tracing
**         for debugging purposes.
**
** Print all shared-cache locks on a BtShared.  Debugging use only.
*/
ساكن فراغ sharedLockTrace(
  BtShared *pBt,
  ثابت حرف *z_ئۇچۇر,
  صحيح i_يىلتىز,
  صحيح eLockType
){
  BtLock *pLock;
  إذا( i_يىلتىز>0 ){
    printf("%s-%p %u%s:", z_ئۇچۇر, pBt, i_يىلتىز, eLockType==READ_LOCK?"R":"W");
  }وإلا{
    printf("%s-%p:", z_ئۇچۇر, pBt);
  }
  لكل(pLock=pBt->pLock; pLock; pLock=pLock->pNext){
    printf(" %p/%u%s", pLock->pBtree, pLock->iTable,
           pLock->eLock==READ_LOCK ? "R" : "W");
    طالما( pLock->pNext && pLock->pBtree==pLock->pNext->pBtree ){
      pLock = pLock->pNext;
      printf(",%u%s", pLock->iTable, pLock->eLock==READ_LOCK ? "R" : "W");
    }
  }
  printf("\n");
  fflush(stdout);
}
#undef SHARED_LOCK_TRACE
#define SHARED_LOCK_TRACE(X,ئۇچۇر,TAB,TYPE)  sharedLockTrace(X,ئۇچۇر,TAB,TYPE)
#endif /* Shared-lock tracing */

#ifdef SQLITE_DEBUG
/*
**** This function is only used as part of an assert() statement. ***
**
** Check to see if pBtree holds the required locks to read or write to the
** table with root page iRoot.   Return 1 if it does and 0 if not.
**
** For example, when writing to a table with root-page iRoot via
** Btree connection pBtree:
**
**    assert( hasSharedCacheTableLock(pBtree, iRoot, 0, WRITE_LOCK) );
**
** When writing to an index that resides in a sharable database, the
** caller should have first obtained a lock specifying the root page of
** the corresponding table. This makes things a bit more complicated,
** as this module treats each table as a separate structure. To determine
** the table corresponding to the index being written, this
** function has to search through the database schema.
**
** Instead of a lock on the table/index rooted at page iRoot, the caller may
** hold a write-lock on the schema table (root page 1). This is also
** acceptable.
*/
ساكن صحيح hasSharedCacheTableLock(
  Btree *pBtree,         /* Handle that must hold lock */
  Pgno i_يىلتىز,            /* Root page of b-tree */
  صحيح isIndex,           /* True if iRoot is the root of an index b-tree */
  صحيح eLockType          /* Required lock type (READ_LOCK or WRITE_LOCK) */
){
  Schema *pSchema = (Schema *)pBtree->pBt->pSchema;
  Pgno iTab = 0;
  BtLock *pLock;

  /* If this database is not shareable, or if the client is reading
  ** and has the read-uncommitted flag set, then no lock is required.
  ** Return true immediately.
  */
  إذا( (pBtree->sharable==0)
   || (eLockType==READ_LOCK && (pBtree->db->flags & SQLITE_ReadUncommit))
  ){
    أرجع 1;
  }

  /* If the client is reading  or writing an index and the schema is
  ** not loaded, then it is too difficult to actually check to see if
  ** the correct locks are held.  So do not bother - just return true.
  ** This case does not come up very often anyhow.
  */
  إذا( isIndex && (!pSchema || (pSchema->schemaFlags&DB_SchemaLoaded)==0) ){
    أرجع 1;
  }

  /* Figure out the root-page that the lock should be held on. For table
  ** b-trees, this is just the root page of the b-tree being read or
  ** written. For index b-trees, it is the root page of the associated
  ** table.  */
  إذا( isIndex ){
    HashElem *p;
    صحيح bSeen = 0;
    لكل(p=sqliteHashFirst(&pSchema->idxHash); p; p=sqliteHashNext(p)){
      Index *pIdx = (Index *)sqliteHashData(p);
      إذا( pIdx->tnum==i_يىلتىز ){
        إذا( bSeen ){
          /* Two or more indexes share the same root page.  There must
          ** be imposter tables.  So just return true.  The assert is not
          ** useful in that case. */
          أرجع 1;
        }
        iTab = pIdx->pTable->tnum;
        bSeen = 1;
      }
    }
  }وإلا{
    iTab = i_يىلتىز;
  }

  SHARED_LOCK_TRACE(pBtree->pBt,"hasLock",i_يىلتىز,eLockType);

  /* Search for the required lock. Either a write-lock on root-page iTab, a
  ** write-lock on the schema table, or (if the client is reading) a
  ** read-lock on iTab will suffice. Return 1 if any of these are found.  */
  لكل(pLock=pBtree->pBt->pLock; pLock; pLock=pLock->pNext){
    إذا( pLock->pBtree==pBtree
     && (pLock->iTable==iTab || (pLock->eLock==WRITE_LOCK && pLock->iTable==1))
     && pLock->eLock>=eLockType
    ){
      أرجع 1;
    }
  }

  /* Failed to find the required lock. */
  أرجع 0;
}
#endif /* SQLITE_DEBUG */

#ifdef SQLITE_DEBUG
/*
**** This function may be used as part of assert() statements only. ****
**
** Return true if it would be illegal for pBtree to write into the
** table or index rooted at iRoot because other shared connections are
** simultaneously reading that same table or index.
**
** It is illegal for pBtree to write if some other Btree object that
** shares the same BtShared object is currently reading or writing
** the iRoot table.  Except, if the other Btree object has the
** read-uncommitted flag set, then it is OK for the other object to
** have a read cursor.
**
** For example, before writing to any part of the table or index
** rooted at page iRoot, one should call:
**
**    assert( !hasReadConflicts(pBtree, iRoot) );
*/
ساكن صحيح hasReadConflicts(Btree *pBtree, Pgno i_يىلتىز){
  BtCursor *p;
  لكل(p=pBtree->pBt->pCursor; p; p=p->pNext){
    إذا( p->pgno_يىلتىز==i_يىلتىز
     && p->pBtree!=pBtree
     && 0==(p->pBtree->db->flags & SQLITE_ReadUncommit)
    ){
      أرجع 1;
    }
  }
  أرجع 0;
}
#endif    /* #ifdef SQLITE_DEBUG */

/*
** Query to see if Btree handle p may obtain a lock of type eLock
** (READ_LOCK or WRITE_LOCK) on the table with root-page iTab. Return
** SQLITE_OK if the lock may be obtained (by calling
** setSharedCacheTableLock()), or SQLITE_LOCKED if not.
*/
ساكن صحيح querySharedCacheTableLock(Btree *p, Pgno iTab, u8 eLock){
  BtShared *pBt = p->pBt;
  BtLock *pIter;

  أكد( sqlite3BtreeHoldsMutex(p) );
  أكد( eLock==READ_LOCK || eLock==WRITE_LOCK );
  أكد( p->db!=0 );
  أكد( !(p->db->flags&SQLITE_ReadUncommit)||eLock==WRITE_LOCK||iTab==1 );
 
  /* If requesting a write-lock, then the Btree must have an open write
  ** transaction on this file. And, obviously, for this to be so there
  ** must be an open write transaction on the file itself.
  */
  أكد( eLock==READ_LOCK || (p==pBt->pWriter && p->inTrans==TRANS_WRITE) );
  أكد( eLock==READ_LOCK || pBt->inTransaction==TRANS_WRITE );
 
  /* This routine is a no-op if the shared-cache is not enabled */
  إذا( !p->sharable ){
    أرجع SQLITE_OK;
  }

  /* If some other connection is holding an exclusive lock, the
  ** requested lock may not be obtained.
  */
  إذا( pBt->pWriter!=p && (pBt->btsFlags & BTS_EXCLUSIVE)!=0 ){
    sqlite3ConnectionBlocked(p->db, pBt->pWriter->db);
    أرجع SQLITE_LOCKED_SHAREDCACHE;
  }

  لكل(pIter=pBt->pLock; pIter; pIter=pIter->pNext){
    /* The condition (pIter->eLock!=eLock) in the following if(...)
    ** statement is a simplification of:
    **
    **   (eLock==WRITE_LOCK || pIter->eLock==WRITE_LOCK)
    **
    ** since we know that if eLock==WRITE_LOCK, then no other connection
    ** may hold a WRITE_LOCK on any table in this file (since there can
    ** only be a single writer).
    */
    أكد( pIter->eLock==READ_LOCK || pIter->eLock==WRITE_LOCK );
    أكد( eLock==READ_LOCK || pIter->pBtree==p || pIter->eLock==READ_LOCK);
    إذا( pIter->pBtree!=p && pIter->iTable==iTab && pIter->eLock!=eLock ){
      sqlite3ConnectionBlocked(p->db, pIter->pBtree->db);
      إذا( eLock==WRITE_LOCK ){
        أكد( p==pBt->pWriter );
        pBt->btsFlags |= BTS_PENDING;
      }
      أرجع SQLITE_LOCKED_SHAREDCACHE;
    }
  }
  أرجع SQLITE_OK;
}
#endif /* !SQLITE_OMIT_SHARED_CACHE */

#ifndef SQLITE_OMIT_SHARED_CACHE
/*
** Add a lock on the table with root-page iTable to the shared-btree used
** by Btree handle p. Parameter eLock must be either READ_LOCK or
** WRITE_LOCK.
**
** This function assumes the following:
**
**   (a) The specified Btree object p is connected to a sharable
**       database (one with the BtShared.sharable flag set), and
**
**   (b) No other Btree objects hold a lock that conflicts
**       with the requested lock (i.e. querySharedCacheTableLock() has
**       already been called and returned SQLITE_OK).
**
** SQLITE_OK is returned if the lock is added successfully. SQLITE_NOMEM
** is returned if a malloc attempt fails.
*/
ساكن صحيح setSharedCacheTableLock(Btree *p, Pgno iTable, u8 eLock){
  BtShared *pBt = p->pBt;
  BtLock *pLock = 0;
  BtLock *pIter;

  SHARED_LOCK_TRACE(pBt,"setLock", iTable, eLock);

  أكد( sqlite3BtreeHoldsMutex(p) );
  أكد( eLock==READ_LOCK || eLock==WRITE_LOCK );
  أكد( p->db!=0 );

  /* A connection with the read-uncommitted flag set will never try to
  ** obtain a read-lock using this function. The only read-lock obtained
  ** by a connection in read-uncommitted mode is on the sqlite_schema
  ** table, and that lock is obtained in BtreeBeginTrans().  */
  أكد( 0==(p->db->flags&SQLITE_ReadUncommit) || eLock==WRITE_LOCK );

  /* This function should only be called on a sharable b-tree after it
  ** has been determined that no other b-tree holds a conflicting lock.  */
  أكد( p->sharable );
  أكد( SQLITE_OK==querySharedCacheTableLock(p, iTable, eLock) );

  /* First search the list for an existing lock on this table. */
  لكل(pIter=pBt->pLock; pIter; pIter=pIter->pNext){
    إذا( pIter->iTable==iTable && pIter->pBtree==p ){
      pLock = pIter;
      اكسر;
    }
  }

  /* If the above search did not find a BtLock struct associating Btree p
  ** with table iTable, allocate one and link it into the list.
  */
  إذا( !pLock ){
    pLock = (BtLock *)sqlite3MallocZero(حجم(BtLock));
    إذا( !pLock ){
      أرجع SQLITE_NOMEM_BKPT;
    }
    pLock->iTable = iTable;
    pLock->pBtree = p;
    pLock->pNext = pBt->pLock;
    pBt->pLock = pLock;
  }

  /* Set the BtLock.eLock variable to the maximum of the current lock
  ** and the requested lock. This means if a write-lock was already held
  ** and a read-lock requested, we don't incorrectly downgrade the lock.
  */
  أكد( WRITE_LOCK>READ_LOCK );
  إذا( eLock>pLock->eLock ){
    pLock->eLock = eLock;
  }

  أرجع SQLITE_OK;
}
#endif /* !SQLITE_OMIT_SHARED_CACHE */

#ifndef SQLITE_OMIT_SHARED_CACHE
/*
** Release all the table locks (locks obtained via calls to
** the setSharedCacheTableLock() procedure) held by Btree object p.
**
** This function assumes that Btree p has an open read or write
** transaction. If it does not, then the BTS_PENDING flag
** may be incorrectly cleared.
*/
ساكن فراغ clearAllSharedCacheTableLocks(Btree *p){
  BtShared *pBt = p->pBt;
  BtLock **ppIter = &pBt->pLock;

  أكد( sqlite3BtreeHoldsMutex(p) );
  أكد( p->sharable || 0==*ppIter );
  أكد( p->inTrans>0 );

  SHARED_LOCK_TRACE(pBt, "clearAllLocks", 0, 0);

  طالما( *ppIter ){
    BtLock *pLock = *ppIter;
    أكد( (pBt->btsFlags & BTS_EXCLUSIVE)==0 || pBt->pWriter==pLock->pBtree );
    أكد( pLock->pBtree->inTrans>=pLock->eLock );
    إذا( pLock->pBtree==p ){
      *ppIter = pLock->pNext;
      أكد( pLock->iTable!=1 || pLock==&p->lock );
      إذا( pLock->iTable!=1 ){
        sqlite3_free(pLock);
      }
    }وإلا{
      ppIter = &pLock->pNext;
    }
  }

  أكد( (pBt->btsFlags & BTS_PENDING)==0 || pBt->pWriter );
  إذا( pBt->pWriter==p ){
    pBt->pWriter = 0;
    pBt->btsFlags &= ~(BTS_EXCLUSIVE|BTS_PENDING);
  }وإلا إذا( pBt->nTransaction==2 ){
    /* This function is called when Btree p is concluding its
    ** transaction. If there currently exists a writer, and p is not
    ** that writer, then the number of locks held by connections other
    ** than the writer must be about to drop to zero. In this case
    ** set the BTS_PENDING flag to 0.
    **
    ** If there is not currently a writer, then BTS_PENDING must
    ** be zero already. So this next line is harmless in that case.
    */
    pBt->btsFlags &= ~BTS_PENDING;
  }
}

/*
** This function changes all write-locks held by Btree p into read-locks.
*/
ساكن فراغ downgradeAllSharedCacheTableLocks(Btree *p){
  BtShared *pBt = p->pBt;

  SHARED_LOCK_TRACE(pBt, "downgradeLocks", 0, 0);

  إذا( pBt->pWriter==p ){
    BtLock *pLock;
    pBt->pWriter = 0;
    pBt->btsFlags &= ~(BTS_EXCLUSIVE|BTS_PENDING);
    لكل(pLock=pBt->pLock; pLock; pLock=pLock->pNext){
      أكد( pLock->eLock==READ_LOCK || pLock->pBtree==p );
      pLock->eLock = READ_LOCK;
    }
  }
}

#endif /* SQLITE_OMIT_SHARED_CACHE */

ساكن فراغ releasePage(MemPage *pPage);         /* Forward reference */
ساكن فراغ releasePageOne(MemPage *pPage);      /* Forward reference */
ساكن فراغ releasePageNotNull(MemPage *pPage);  /* Forward reference */

/*
***** This routine is used inside of assert() only ****
**
** Verify that the cursor holds the mutex on its BtShared
*/
#ifdef SQLITE_DEBUG
ساكن صحيح cursorHoldsMutex(BtCursor *p){
  أرجع sqlite3_mutex_held(p->pBt->mutex);
}

/* Verify that the cursor and the BtShared agree about what is the current
** database connetion. This is important in shared-cache mode. If the database
** connection pointers get out-of-sync, it is possible for routines like
** btreeInitPage() to reference an stale connection pointer that references a
** a connection that has already closed.  This routine is used inside assert()
** statements only and for the purpose of double-checking that the btree code
** does keep the database connection pointers up-to-date.
*/
ساكن صحيح cursorOwnsBtShared(BtCursor *p){
  أكد( cursorHoldsMutex(p) );
  أرجع (p->pBtree->db==p->pBt->db);
}
#endif

/*
** Invalidate the overflow cache of the cursor passed as the first argument.
** on the shared btree structure pBt.
*/
#define invalidateOverflowCache(pCur) (pCur->curFlags &= ~BTCF_ValidOvfl)

/*
** Invalidate the overflow page-list cache for all cursors opened
** on the shared btree structure pBt.
*/
ساكن فراغ invalidateAllOverflowCache(BtShared *pBt){
  BtCursor *p;
  أكد( sqlite3_mutex_held(pBt->mutex) );
  لكل(p=pBt->pCursor; p; p=p->pNext){
    invalidateOverflowCache(p);
  }
}

#ifndef SQLITE_OMIT_INCRBLOB
/*
** This function is called before modifying the contents of a table
** to invalidate any incrblob cursors that are open on the
** row or one of the rows being modified.
**
** If argument isClearTable is true, then the entire contents of the
** table is about to be deleted. In this case invalidate all incrblob
** cursors open on any row within the table with root-page pgnoRoot.
**
** Otherwise, if argument isClearTable is false, then the row with
** rowid iRow is being replaced or deleted. In this case invalidate
** only those incrblob cursors open on that specific row.
*/
ساكن فراغ invalidateIncrblobCursors(
  Btree *pBtree,          /* The database file to check */
  Pgno pgno_يىلتىز,          /* The table that might be changing */
  i64 iRow,               /* The rowid that might be changing */
  صحيح isClearTable        /* True if all rows are being deleted */
){
  BtCursor *p;
  أكد( pBtree->hasIncrblobCur );
  أكد( sqlite3BtreeHoldsMutex(pBtree) );
  pBtree->hasIncrblobCur = 0;
  لكل(p=pBtree->pBt->pCursor; p; p=p->pNext){
    إذا( (p->curFlags & BTCF_Incrblob)!=0 ){
      pBtree->hasIncrblobCur = 1;
      إذا( p->pgno_يىلتىز==pgno_يىلتىز && (isClearTable || p->info.n_ئاچقۇچ==iRow) ){
        p->eState = CURSOR_INVALID;
      }
    }
  }
}

#وإلا
  /* Stub function when INCRBLOB is omitted */
  #define invalidateIncrblobCursors(w,x,y,z)
#endif /* SQLITE_OMIT_INCRBLOB */

/*
** Set bit pgno of the BtShared.pHasContent bitvec. This is called
** when a page that previously contained data becomes a free-list leaf
** page.
**
** The BtShared.pHasContent bitvec exists to work around an obscure
** bug caused by the interaction of two useful IO optimizations surrounding
** free-list leaf pages:
**
**   1) When all data is deleted from a page and the page becomes
**      a free-list leaf page, the page is not written to the database
**      (as free-list leaf pages contain no meaningful data). Sometimes
**      such a page is not even journalled (as it will not be modified,
**      why bother journalling it?).
**
**   2) When a free-list leaf page is reused, its content is not read
**      from the database or written to the journal file (why should it
**      be, if it is not at all meaningful?).
**
** By themselves, these optimizations work fine and provide a handy
** performance boost to bulk delete or insert operations. However, if
** a page is moved to the free-list and then reused within the same
** transaction, a problem comes up. If the page is not journalled when
** it is moved to the free-list and it is also not journalled when it
** is extracted from the free-list and reused, then the original data
** may be lost. In the event of a rollback, it may not be possible
** to restore the database to its original configuration.
**
** The solution is the BtShared.pHasContent bitvec. Whenever a page is
** moved to become a free-list leaf page, the corresponding bit is
** set in the bitvec. Whenever a leaf page is extracted from the free-list,
** optimization 2 above is omitted if the corresponding bit is already
** set in BtShared.pHasContent. The contents of the bitvec are cleared
** at the end of every transaction.
*/
ساكن صحيح btreeSetHasContent(BtShared *pBt, Pgno pgno){
  صحيح rc = SQLITE_OK;
  إذا( !pBt->pHasContent ){
    أكد( pgno<=pBt->nPage );
    pBt->pHasContent = sqlite3BitvecCreate(pBt->nPage);
    إذا( !pBt->pHasContent ){
      rc = SQLITE_NOMEM_BKPT;
    }
  }
  إذا( rc==SQLITE_OK && pgno<=sqlite3BitvecSize(pBt->pHasContent) ){
    rc = sqlite3BitvecSet(pBt->pHasContent, pgno);
  }
  أرجع rc;
}

/*
** Query the BtShared.pHasContent vector.
**
** This function is called when a free-list leaf page is removed from the
** free-list for reuse. It returns false if it is safe to retrieve the
** page from the pager layer with the 'no-content' flag set. True otherwise.
*/
ساكن صحيح btreeGetHasContent(BtShared *pBt, Pgno pgno){
  Bitvec *p = pBt->pHasContent;
  أرجع p && (pgno>sqlite3BitvecSize(p) || sqlite3BitvecTestNotNull(p, pgno));
}

/*
** Clear (destroy) the BtShared.pHasContent bitvec. This should be
** invoked at the conclusion of each write-transaction.
*/
ساكن فراغ btreeClearHasContent(BtShared *pBt){
  sqlite3BitvecDestroy(pBt->pHasContent);
  pBt->pHasContent = 0;
}

/*
** Release all of the apPage[] pages for a cursor.
*/
ساكن فراغ btreeReleaseAllCursorPages(BtCursor *pCur){
  صحيح i;
  إذا( pCur->iPage>=0 ){
    لكل(i=0; i<pCur->iPage; i++){
      releasePageNotNull(pCur->apPage[i]);
    }
    releasePageNotNull(pCur->pPage);
    pCur->iPage = -1;
  }
}

/*
** The cursor passed as the only argument must point to a valid entry
** when this function is called (i.e. have eState==CURSOR_VALID). This
** function saves the current cursor key in variables pCur->nKey and
** pCur->pKey. SQLITE_OK is returned if successful or an SQLite error
** code otherwise.
**
** If the cursor is open on an intkey table, then the integer key
** (the rowid) is stored in pCur->nKey and pCur->pKey is left set to
** NULL. If the cursor is open on a non-intkey table, then pCur->pKey is
** set to point to a malloced buffer pCur->nKey bytes in size containing
** the key.
*/
ساكن صحيح save_Cursor_ئاچقۇچ(BtCursor *pCur){
  صحيح rc = SQLITE_OK;
  أكد( CURSOR_VALID==pCur->eState );
  أكد( 0==pCur->p_ئاچقۇچ );
  أكد( cursorHoldsMutex(pCur) );

  إذا( pCur->cur_Int_ئاچقۇچ ){
    /* Only the rowid is required for a table btree */
    pCur->n_ئاچقۇچ = sqlite3_Btree_Integer_ئاچقۇچ(pCur);
  }وإلا{
    /* For an index btree, save the complete key content. It is possible
    ** that the current key is corrupt. In that case, it is possible that
    ** the sqlite3VdbeRecordUnpack() function may overread the buffer by
    ** up to the size of 1 varint plus 1 8-byte value when the cursor
    ** position is restored. Hence the 17 bytes of padding allocated
    ** below. */
    فراغ *p_ئاچقۇچ;
    pCur->n_ئاچقۇچ = sqlite3BtreePayloadSize(pCur);
    p_ئاچقۇچ = sqlite3Malloc( ((i64)pCur->n_ئاچقۇچ) + 9 + 8 );
    إذا( p_ئاچقۇچ ){
      rc = sqlite3BtreePayload(pCur, 0, (صحيح)pCur->n_ئاچقۇچ, p_ئاچقۇچ);
      إذا( rc==SQLITE_OK ){
        memset(((u8*)p_ئاچقۇچ)+pCur->n_ئاچقۇچ, 0, 9+8);
        pCur->p_ئاچقۇچ = p_ئاچقۇچ;
      }وإلا{
        sqlite3_free(p_ئاچقۇچ);
      }
    }وإلا{
      rc = SQLITE_NOMEM_BKPT;
    }
  }
  أكد( !pCur->cur_Int_ئاچقۇچ || !pCur->p_ئاچقۇچ );
  أرجع rc;
}

/*
** Save the current cursor position in the variables BtCursor.nKey
** and BtCursor.pKey. The cursor's state is set to CURSOR_REQUIRESEEK.
**
** The caller must ensure that the cursor is valid (has eState==CURSOR_VALID)
** prior to calling this routine. 
*/
ساكن صحيح saveCursorPosition(BtCursor *pCur){
  صحيح rc;

  أكد( CURSOR_VALID==pCur->eState || CURSOR_SKIPNEXT==pCur->eState );
  أكد( 0==pCur->p_ئاچقۇچ );
  أكد( cursorHoldsMutex(pCur) );

  إذا( pCur->curFlags & BTCF_Pinned ){
    أرجع SQLITE_CONSTRAINT_PINNED;
  }
  إذا( pCur->eState==CURSOR_SKIPNEXT ){
    pCur->eState = CURSOR_VALID;
  }وإلا{
    pCur->skipNext = 0;
  }

  rc = save_Cursor_ئاچقۇچ(pCur);
  إذا( rc==SQLITE_OK ){
    btreeReleaseAllCursorPages(pCur);
    pCur->eState = CURSOR_REQUIRESEEK;
  }

  pCur->curFlags &= ~(BTCF_ValidNKey|BTCF_ValidOvfl|BTCF_AtLast);
  أرجع rc;
}

/* Forward reference */
ساكن صحيح SQLITE_NOINLINE saveCursorsOnList(BtCursor*,Pgno,BtCursor*);

/*
** Save the positions of all cursors (except pExcept) that are open on
** the table with root-page iRoot.  "Saving the cursor position" means that
** the location in the btree is remembered in such a way that it can be
** moved back to the same spot after the btree has been modified.  This
** routine is called just before cursor pExcept is used to modify the
** table, for example in BtreeDelete() or BtreeInsert().
**
** If there are two or more cursors on the same btree, then all such
** cursors should have their BTCF_Multiple flag set.  The btreeCursor()
** routine enforces that rule.  This routine only needs to be called in
** the uncommon case when pExpect has the BTCF_Multiple flag set.
**
** If pExpect!=NULL and if no other cursors are found on the same root-page,
** then the BTCF_Multiple flag on pExpect is cleared, to avoid another
** pointless call to this routine.
**
** Implementation note:  This routine merely checks to see if any cursors
** need to be saved.  It calls out to saveCursorsOnList() in the (unusual)
** event that cursors are in need to being saved.
*/
ساكن صحيح saveAllCursors(BtShared *pBt, Pgno i_يىلتىز, BtCursor *pExcept){
  BtCursor *p;
  أكد( sqlite3_mutex_held(pBt->mutex) );
  أكد( pExcept==0 || pExcept->pBt==pBt );
  لكل(p=pBt->pCursor; p; p=p->pNext){
    إذا( p!=pExcept && (0==i_يىلتىز || p->pgno_يىلتىز==i_يىلتىز) ) اكسر;
  }
  إذا( p ) أرجع saveCursorsOnList(p, i_يىلتىز, pExcept);
  إذا( pExcept ) pExcept->curFlags &= ~BTCF_Multiple;
  أرجع SQLITE_OK;
}

/* This helper routine to saveAllCursors does the actual work of saving
** the cursors if and when a cursor is found that actually requires saving.
** The common case is that no cursors need to be saved, so this routine is
** broken out from its caller to avoid unnecessary stack pointer movement.
*/
ساكن صحيح SQLITE_NOINLINE saveCursorsOnList(
  BtCursor *p,         /* The first cursor that needs saving */
  Pgno i_يىلتىز,          /* Only save cursor with this iRoot. Save all if zero */
  BtCursor *pExcept    /* Do not save this cursor */
){
  افعل{
    إذا( p!=pExcept && (0==i_يىلتىز || p->pgno_يىلتىز==i_يىلتىز) ){
      إذا( p->eState==CURSOR_VALID || p->eState==CURSOR_SKIPNEXT ){
        صحيح rc = saveCursorPosition(p);
        إذا( SQLITE_OK!=rc ){
          أرجع rc;
        }
      }وإلا{
        testcase( p->iPage>=0 );
        btreeReleaseAllCursorPages(p);
      }
    }
    p = p->pNext;
  }طالما( p );
  أرجع SQLITE_OK;
}

/*
** Clear the current cursor position.
*/
فراغ sqlite3BtreeClearCursor(BtCursor *pCur){
  أكد( cursorHoldsMutex(pCur) );
  sqlite3_free(pCur->p_ئاچقۇچ);
  pCur->p_ئاچقۇچ = 0;
  pCur->eState = CURSOR_INVALID;
}

/*
** In this version of BtreeMoveto, pKey is a packed index record
** such as is generated by the OP_MakeRecord opcode.  Unpack the
** record and then call sqlite3BtreeIndexMoveto() to do the work.
*/
ساكن صحيح btreeMoveto(
  BtCursor *pCur,     /* Cursor open on the btree to be searched */
  ثابت فراغ *p_ئاچقۇچ,   /* Packed key if the btree is an index */
  i64 n_ئاچقۇچ,           /* Integer key for tables.  Size of pKey for indices */
  صحيح bias,           /* Bias search to the high end */
  صحيح *pRes           /* Write search results here */
){
  صحيح rc;                    /* Status code */
  UnpackedRecord *p_Idx_ئاچقۇچ;   /* Unpacked index key */

  إذا( p_ئاچقۇچ ){
    ئاچقۇچ_Info *p_ئاچقۇچ_Info = pCur->p_ئاچقۇچ_Info;
    أكد( n_ئاچقۇچ==(i64)(صحيح)n_ئاچقۇچ );
    p_Idx_ئاچقۇچ = sqlite3VdbeAllocUnpackedRecord(p_ئاچقۇچ_Info);
    إذا( p_Idx_ئاچقۇچ==0 ) أرجع SQLITE_NOMEM_BKPT;
    sqlite3VdbeRecordUnpack((صحيح)n_ئاچقۇچ, p_ئاچقۇچ, p_Idx_ئاچقۇچ);
    إذا( p_Idx_ئاچقۇچ->nField==0 || p_Idx_ئاچقۇچ->nField>p_ئاچقۇچ_Info->nAllField ){
      rc = SQLITE_CORRUPT_BKPT;
    }وإلا{
      rc = sqlite3BtreeIndexMoveto(pCur, p_Idx_ئاچقۇچ, pRes);
    }
    sqlite3DbFree(pCur->p_ئاچقۇچ_Info->db, p_Idx_ئاچقۇچ);
  }وإلا{
    p_Idx_ئاچقۇچ = 0;
    rc = sqlite3BtreeTableMoveto(pCur, n_ئاچقۇچ, bias, pRes);
  }
  أرجع rc;
}

/*
** Restore the cursor to the position it was in (or as close to as possible)
** when saveCursorPosition() was called. Note that this call deletes the
** saved position info stored by saveCursorPosition(), so there can be
** at most one effective restoreCursorPosition() call after each
** saveCursorPosition().
*/
ساكن صحيح btreeRestoreCursorPosition(BtCursor *pCur){
  صحيح rc;
  صحيح skipNext = 0;
  أكد( cursorOwnsBtShared(pCur) );
  أكد( pCur->eState>=CURSOR_REQUIRESEEK );
  إذا( pCur->eState==CURSOR_FAULT ){
    أرجع pCur->skipNext;
  }
  pCur->eState = CURSOR_INVALID;
  إذا( sqlite3FaultSim(410) ){
    rc = SQLITE_IOERR;
  }وإلا{
    rc = btreeMoveto(pCur, pCur->p_ئاچقۇچ, pCur->n_ئاچقۇچ, 0, &skipNext);
  }
  إذا( rc==SQLITE_OK ){
    sqlite3_free(pCur->p_ئاچقۇچ);
    pCur->p_ئاچقۇچ = 0;
    أكد( pCur->eState==CURSOR_VALID || pCur->eState==CURSOR_INVALID );
    إذا( skipNext ) pCur->skipNext = skipNext;
    إذا( pCur->skipNext && pCur->eState==CURSOR_VALID ){
      pCur->eState = CURSOR_SKIPNEXT;
    }
  }
  أرجع rc;
}

#define restoreCursorPosition(p) \
  (p->eState>=CURSOR_REQUIRESEEK ? \
         btreeRestoreCursorPosition(p) : \
         SQLITE_OK)

/*
** Determine whether or not a cursor has moved from the position where
** it was last placed, or has been invalidated for any other reason.
** Cursors can move when the row they are pointing at is deleted out
** from under them, for example.  Cursor might also move if a btree
** is rebalanced.
**
** Calling this routine with a NULL cursor pointer returns false.
**
** Use the separate sqlite3BtreeCursorRestore() routine to restore a cursor
** back to where it ought to be if this routine returns true.
*/
صحيح sqlite3BtreeCursorHasMoved(BtCursor *pCur){
  أكد( EIGHT_BYTE_ALIGNMENT(pCur)
       || pCur==sqlite3BtreeFakeValidCursor() );
  أكد( offsetof(BtCursor, eState)==0 );
  أكد( حجم(pCur->eState)==1 );
  أرجع CURSOR_VALID != *(u8*)pCur;
}

/*
** Return a pointer to a fake BtCursor object that will always answer
** false to the sqlite3BtreeCursorHasMoved() routine above.  The fake
** cursor returned must not be used with any other Btree interface.
*/
BtCursor *sqlite3BtreeFakeValidCursor(فراغ){
  ساكن u8 fakeCursor = CURSOR_VALID;
  أكد( offsetof(BtCursor, eState)==0 );
  أرجع (BtCursor*)&fakeCursor;
}

/*
** This routine restores a cursor back to its original position after it
** has been moved by some outside activity (such as a btree rebalance or
** a row having been deleted out from under the cursor). 
**
** On success, the *pDifferentRow parameter is false if the cursor is left
** pointing at exactly the same row.  *pDifferntRow is the row the cursor
** was pointing to has been deleted, forcing the cursor to point to some
** nearby row.
**
** This routine should only be called for a cursor that just returned
** TRUE from sqlite3BtreeCursorHasMoved().
*/
صحيح sqlite3BtreeCursorRestore(BtCursor *pCur, صحيح *pDifferentRow){
  صحيح rc;

  أكد( pCur!=0 );
  أكد( pCur->eState!=CURSOR_VALID );
  rc = restoreCursorPosition(pCur);
  إذا( rc ){
    *pDifferentRow = 1;
    أرجع rc;
  }
  إذا( pCur->eState!=CURSOR_VALID ){
    *pDifferentRow = 1;
  }وإلا{
    *pDifferentRow = 0;
  }
  أرجع SQLITE_OK;
}

#ifdef SQLITE_ENABLE_CURSOR_HINTS
/*
** Provide hints to the cursor.  The particular hint given (and the type
** and number of the varargs parameters) is determined by the eHintType
** parameter.  See the definitions of the BTREE_HINT_* macros for details.
*/
فراغ sqlite3BtreeCursorHint(BtCursor *pCur, صحيح eHintType, ...){
  /* Used only by system that substitute their own storage engine */
#ifdef SQLITE_DEBUG
  va_list ap;
  va_start(ap, eHintType);
  إذا( eHintType==BTREE_HINT_RANGE ){
    ئىپادە *p_ئىپادە;
    Walker w;
    memset(&w, 0, حجم(w));
    w.x_ئىپادە_Callback = sqlite3_Cursor_Range_Hint_ئىپادە_Check;
    p_ئىپادە = va_arg(ap, ئىپادە*);
    w.u.aMem = va_arg(ap, Mem*);
    أكد( p_ئىپادە!=0 );
    أكد( w.u.aMem!=0 );
    sqlite3_Walk_ئىپادە(&w, p_ئىپادە);
  }وإلا إذا( ALWAYS(eHintType==BTREE_HINT_TABLECURSOR) ){
    BtCursor *pCsr = va_arg(ap, BtCursor*);
    أكد( pCur->pCursorHintTableCursor==0 
         || pCur->pCursorHintTableCursor==pCsr
    );
    أكد( pCsr->p_ئاچقۇچ_Info==0 || CORRUPT_DB );
    pCur->pCursorHintTableCursor = pCsr;
  }
  va_end(ap);
#endif /* SQLITE_DEBUG */
}
#endif /* SQLITE_ENABLE_CURSOR_HINTS */

#إذا defined(SQLITE_ENABLE_CURSOR_HINTS) && defined(SQLITE_DEBUG)
/*
** Return the pointer configured via the BTREE_HINT_TABLECURSOR hint on
** cursor pCsr. This is used from OP_DeferredSeek to assert() that the
** index cursor has been correctly configured with the table cursor.
*/
BtCursor *sqlite3BtreeCursorHintTblCsr(BtCursor *pCsr){
  أرجع pCsr->pCursorHintTableCursor;
}
#endif

/*
** Provide flag hints to the cursor.
*/
فراغ sqlite3BtreeCursorHintFlags(BtCursor *pCur, unsigned x){
  أكد( x==BTREE_SEEK_EQ || x==BTREE_BULKLOAD || x==0 );
  pCur->hints = (u8)x;
}


#ifndef SQLITE_OMIT_AUTOVACUUM
/*
** Given a page number of a regular database page, return the page
** number for the pointer-map page that contains the entry for the
** input page number.
**
** Return 0 (not a valid page) for pgno==1 since there is
** no pointer map associated with page 1.  The integrity_check logic
** requires that ptrmapPageno(*,1)!=1.
*/
ساكن Pgno ptrmapPageno(BtShared *pBt, Pgno pgno){
  صحيح nPagesPerMapPage;
  Pgno iPtrMap, ret;
  أكد( sqlite3_mutex_held(pBt->mutex) );
  إذا( pgno<2 ) أرجع 0;
  nPagesPerMapPage = (pBt->usableSize/5)+1;
  iPtrMap = (pgno-2)/nPagesPerMapPage;
  ret = (iPtrMap*nPagesPerMapPage) + 2;
  إذا( ret==PENDING_BYTE_PAGE(pBt) ){
    ret++;
  }
  أرجع ret;
}

/*
** Write an entry into the pointer map.
**
** This routine updates the pointer map entry for page number 'key'
** so that it maps to type 'eType' and parent page number 'pgno'.
**
** If *pRC is initially non-zero (non-SQLITE_OK) then this routine is
** a no-op.  If an error occurs, the appropriate error code is written
** into *pRC.
*/
ساكن فراغ ptrmapPut(BtShared *pBt, Pgno ئاچقۇچ, u8 eType, Pgno parent, صحيح *pRC){
  DbPage *pDbPage;  /* The pointer map page */
  u8 *pPtrmap;      /* The pointer map data */
  Pgno iPtrmap;     /* The pointer map page number */
  صحيح offset;       /* Offset in pointer map page */
  صحيح rc;           /* Return code from subfunctions */

  إذا( *pRC ) أرجع;

  أكد( sqlite3_mutex_held(pBt->mutex) );
  /* The super-journal page number must never be used as a pointer map page */
  أكد( 0==PTRMAP_ISPAGE(pBt, PENDING_BYTE_PAGE(pBt)) );

  أكد( pBt->autoVacuum );
  إذا( ئاچقۇچ==0 ){
    *pRC = SQLITE_CORRUPT_BKPT;
    أرجع;
  }
  iPtrmap = PTRMAP_PAGENO(pBt, ئاچقۇچ);
  rc = sqlite3PagerGet(pBt->pPager, iPtrmap, &pDbPage, 0);
  إذا( rc!=SQLITE_OK ){
    *pRC = rc;
    أرجع;
  }
  إذا( ((حرف*)sqlite3PagerGetExtra(pDbPage))[0]!=0 ){
    /* The first byte of the extra data is the MemPage.isInit byte.
    ** If that byte is set, it means this page is also being used
    ** as a btree page. */
    *pRC = SQLITE_CORRUPT_BKPT;
    اذهب ptrmap_exit;
  }
  offset = PTRMAP_PTROFFSET(iPtrmap, ئاچقۇچ);
  إذا( offset<0 ){
    *pRC = SQLITE_CORRUPT_BKPT;
    اذهب ptrmap_exit;
  }
  أكد( offset <= (صحيح)pBt->usableSize-5 );
  pPtrmap = (u8 *)sqlite3PagerGetData(pDbPage);

  إذا( eType!=pPtrmap[offset] || get4byte(&pPtrmap[offset+1])!=parent ){
    TRACE(("PTRMAP_UPDATE: %u->(%u,%u)\n", ئاچقۇچ, eType, parent));
    *pRC= rc = sqlite3PagerWrite(pDbPage);
    إذا( rc==SQLITE_OK ){
      pPtrmap[offset] = eType;
      put4byte(&pPtrmap[offset+1], parent);
    }
  }

ptrmap_exit:
  sqlite3PagerUnref(pDbPage);
}

/*
** Read an entry from the pointer map.
**
** This routine retrieves the pointer map entry for page 'key', writing
** the type and parent page number to *pEType and *pPgno respectively.
** An error code is returned if something goes wrong, otherwise SQLITE_OK.
*/
ساكن صحيح ptrmapGet(BtShared *pBt, Pgno ئاچقۇچ, u8 *pEType, Pgno *pPgno){
  DbPage *pDbPage;   /* The pointer map page */
  صحيح iPtrmap;       /* Pointer map page index */
  u8 *pPtrmap;       /* Pointer map page data */
  صحيح offset;        /* Offset of entry in pointer map */
  صحيح rc;

  أكد( sqlite3_mutex_held(pBt->mutex) );

  iPtrmap = PTRMAP_PAGENO(pBt, ئاچقۇچ);
  rc = sqlite3PagerGet(pBt->pPager, iPtrmap, &pDbPage, 0);
  إذا( rc!=0 ){
    أرجع rc;
  }
  pPtrmap = (u8 *)sqlite3PagerGetData(pDbPage);

  offset = PTRMAP_PTROFFSET(iPtrmap, ئاچقۇچ);
  إذا( offset<0 ){
    sqlite3PagerUnref(pDbPage);
    أرجع SQLITE_CORRUPT_BKPT;
  }
  أكد( offset <= (صحيح)pBt->usableSize-5 );
  أكد( pEType!=0 );
  *pEType = pPtrmap[offset];
  إذا( pPgno ) *pPgno = get4byte(&pPtrmap[offset+1]);

  sqlite3PagerUnref(pDbPage);
  إذا( *pEType<1 || *pEType>5 ) أرجع SQLITE_CORRUPT_PGNO(iPtrmap);
  أرجع SQLITE_OK;
}

#وإلا /* if defined SQLITE_OMIT_AUTOVACUUM */
  #define ptrmapPut(w,x,y,z,rc)
  #define ptrmapGet(w,x,y,z) SQLITE_OK
  #define ptrmapPutOvflPtr(x, y, z, rc)
#endif

/*
** Given a btree page and a cell index (0 means the first cell on
** the page, 1 means the second cell, and so forth) return a pointer
** to the cell content.
**
** findCellPastPtr() does the same except it skips past the initial
** 4-byte child pointer found on interior pages, if there is one.
**
** This routine works only for pages that do not contain overflow cells.
*/
#define findCell(P,I) \
  ((P)->aData + ((P)->maskPage & get2byteAligned(&(P)->aCellIdx[2*(I)])))
#define findCellPastPtr(P,I) \
  ((P)->aDataOfst + ((P)->maskPage & get2byteAligned(&(P)->aCellIdx[2*(I)])))


/*
** This is common tail processing for btreeParseCellPtr() and
** btreeParseCellPtrIndex() for the case when the cell does not fit entirely
** on a single B-tree page.  Make necessary adjustments to the CellInfo
** structure.
*/
ساكن SQLITE_NOINLINE فراغ btreeParseCellAdjustSizeForOverflow(
  MemPage *pPage,         /* Page containing the cell */
  u8 *pCell,              /* Pointer to the cell text. */
  CellInfo *pInfo         /* Fill in this structure */
){
  /* If the payload will not fit completely on the local page, we have
  ** to decide how much to store locally and how much to spill onto
  ** overflow pages.  The strategy is to minimize the amount of unused
  ** space on overflow pages while keeping the amount of local storage
  ** in between minLocal and maxLocal.
  **
  ** Warning:  changing the way overflow payload is distributed in any
  ** way will result in an incompatible file format.
  */
  صحيح minLocal;  /* Minimum amount of payload held locally */
  صحيح maxLocal;  /* Maximum amount of payload held locally */
  صحيح surplus;   /* Overflow payload available for local storage */

  minLocal = pPage->minLocal;
  maxLocal = pPage->maxLocal;
  surplus = minLocal + (pInfo->nPayload - minLocal)%(pPage->pBt->usableSize-4);
  testcase( surplus==maxLocal );
  testcase( surplus==maxLocal+1 );
  إذا( surplus <= maxLocal ){
    pInfo->nLocal = (u16)surplus;
  }وإلا{
    pInfo->nLocal = (u16)minLocal;
  }
  pInfo->nSize = (u16)(&pInfo->pPayload[pInfo->nLocal] - pCell) + 4;
}

/*
** Given a record with nPayload bytes of payload stored within btree
** page pPage, return the number of bytes of payload stored locally.
*/
ساكن صحيح btreePayloadToLocal(MemPage *pPage, i64 nPayload){
  صحيح maxLocal;  /* Maximum amount of payload held locally */
  maxLocal = pPage->maxLocal;
  أكد( nPayload>=0 );
  إذا( nPayload<=maxLocal ){
    أرجع (صحيح)nPayload;
  }وإلا{
    صحيح minLocal;  /* Minimum amount of payload held locally */
    صحيح surplus;   /* Overflow payload available for local storage */
    minLocal = pPage->minLocal;
    surplus = (صحيح)(minLocal +(nPayload - minLocal)%(pPage->pBt->usableSize-4));
    أرجع (surplus <= maxLocal) ? surplus : minLocal;
  }
}

/*
** The following routines are implementations of the MemPage.xParseCell()
** method.
**
** Parse a cell content block and fill in the CellInfo structure.
**
** btreeParseCellPtr()        =>   table btree leaf nodes
** btreeParseCellNoPayload()  =>   table btree internal nodes
** btreeParseCellPtrIndex()   =>   index btree nodes
**
** There is also a wrapper function btreeParseCell() that works for
** all MemPage types and that references the cell by index rather than
** by pointer.
*/
ساكن فراغ btreeParseCellPtrNoPayload(
  MemPage *pPage,         /* Page containing the cell */
  u8 *pCell,              /* Pointer to the cell text. */
  CellInfo *pInfo         /* Fill in this structure */
){
  أكد( sqlite3_mutex_held(pPage->pBt->mutex) );
  أكد( pPage->leaf==0 );
  أكد( pPage->childPtrSize==4 );
#ifndef SQLITE_DEBUG
  UNUSED_PARAMETER(pPage);
#endif
  pInfo->nSize = 4 + sqlite3GetVarint(&pCell[4], (u64*)&pInfo->n_ئاچقۇچ);
  pInfo->nPayload = 0;
  pInfo->nLocal = 0;
  pInfo->pPayload = 0;
  أرجع;
}
ساكن فراغ btreeParseCellPtr(
  MemPage *pPage,         /* Page containing the cell */
  u8 *pCell,              /* Pointer to the cell text. */
  CellInfo *pInfo         /* Fill in this structure */
){
  u8 *pIter;              /* For scanning through pCell */
  u64 nPayload;           /* Number of bytes of cell payload */
  u64 i_ئاچقۇچ;               /* Extracted Key value */

  أكد( sqlite3_mutex_held(pPage->pBt->mutex) );
  أكد( pPage->leaf==0 || pPage->leaf==1 );
  أكد( pPage->int_ئاچقۇچ_Leaf );
  أكد( pPage->childPtrSize==0 );
  pIter = pCell;

  /* The next block of code is equivalent to:
  **
  **     pIter += getVarint32(pIter, nPayload);
  **
  ** The code is inlined to avoid a function call.
  */
  nPayload = *pIter;
  إذا( nPayload>=0x80 ){
    u8 *pEnd = &pIter[8];
    nPayload &= 0x7f;
    افعل{
      nPayload = (nPayload<<7) | (*++pIter & 0x7f);
    }طالما( (*pIter)>=0x80 && pIter<pEnd );
    nPayload &= 0xffffffff;
  }
  pIter++;

  /* The next block of code is equivalent to:
  **
  **     pIter += sqlite3GetVarint(pIter, (u64*)&pInfo->nKey);
  **
  ** The code is inlined and the loop is unrolled for performance.
  ** This routine is a high-runner.
  */
  i_ئاچقۇچ = *pIter;
  إذا( i_ئاچقۇچ>=0x80 ){
    u8 x;
    i_ئاچقۇچ = (i_ئاچقۇچ<<7) ^ (x = *++pIter);
    إذا( x>=0x80 ){
      i_ئاچقۇچ = (i_ئاچقۇچ<<7) ^ (x = *++pIter);
      إذا( x>=0x80 ){
        i_ئاچقۇچ = (i_ئاچقۇچ<<7) ^ 0x10204000 ^ (x = *++pIter);
        إذا( x>=0x80 ){
          i_ئاچقۇچ = (i_ئاچقۇچ<<7) ^ 0x4000 ^ (x = *++pIter);
          إذا( x>=0x80 ){
            i_ئاچقۇچ = (i_ئاچقۇچ<<7) ^ 0x4000 ^ (x = *++pIter);
            إذا( x>=0x80 ){
              i_ئاچقۇچ = (i_ئاچقۇچ<<7) ^ 0x4000 ^ (x = *++pIter);
              إذا( x>=0x80 ){
                i_ئاچقۇچ = (i_ئاچقۇچ<<7) ^ 0x4000 ^ (x = *++pIter);
                إذا( x>=0x80 ){
                  i_ئاچقۇچ = (i_ئاچقۇچ<<8) ^ 0x8000 ^ (*++pIter);
                }
              }
            }
          }
        }
      }وإلا{
        i_ئاچقۇچ ^= 0x204000;
      }
    }وإلا{
      i_ئاچقۇچ ^= 0x4000;
    }
  }
  pIter++;

  pInfo->n_ئاچقۇچ = *(i64*)&i_ئاچقۇچ;
  pInfo->nPayload = (u32)nPayload;
  pInfo->pPayload = pIter;
  testcase( nPayload==pPage->maxLocal );
  testcase( nPayload==(u32)pPage->maxLocal+1 );
  أكد( pPage->maxLocal <= BT_MAX_LOCAL );
  إذا( nPayload<=pPage->maxLocal ){
    /* This is the (easy) common case where the entire payload fits
    ** on the local page.  No overflow is required.
    */
    pInfo->nSize = (u16)nPayload + (u16)(pIter - pCell);
    إذا( pInfo->nSize<4 ) pInfo->nSize = 4;
    pInfo->nLocal = (u16)nPayload;
  }وإلا{
    btreeParseCellAdjustSizeForOverflow(pPage, pCell, pInfo);
  }
}
ساكن فراغ btreeParseCellPtrIndex(
  MemPage *pPage,         /* Page containing the cell */
  u8 *pCell,              /* Pointer to the cell text. */
  CellInfo *pInfo         /* Fill in this structure */
){
  u8 *pIter;              /* For scanning through pCell */
  u32 nPayload;           /* Number of bytes of cell payload */

  أكد( sqlite3_mutex_held(pPage->pBt->mutex) );
  أكد( pPage->leaf==0 || pPage->leaf==1 );
  أكد( pPage->int_ئاچقۇچ_Leaf==0 );
  pIter = pCell + pPage->childPtrSize;
  nPayload = *pIter;
  إذا( nPayload>=0x80 ){
    u8 *pEnd = &pIter[8];
    nPayload &= 0x7f;
    افعل{
      nPayload = (nPayload<<7) | (*++pIter & 0x7f);
    }طالما( *(pIter)>=0x80 && pIter<pEnd );
  }
  pIter++;
  pInfo->n_ئاچقۇچ = nPayload;
  pInfo->nPayload = nPayload;
  pInfo->pPayload = pIter;
  testcase( nPayload==pPage->maxLocal );
  testcase( nPayload==(u32)pPage->maxLocal+1 );
  أكد( nPayload>=0 );
  أكد( pPage->maxLocal <= BT_MAX_LOCAL );
  إذا( nPayload<=pPage->maxLocal ){
    /* This is the (easy) common case where the entire payload fits
    ** on the local page.  No overflow is required.
    */
    pInfo->nSize = (u16)nPayload + (u16)(pIter - pCell);
    إذا( pInfo->nSize<4 ) pInfo->nSize = 4;
    pInfo->nLocal = (u16)nPayload;
  }وإلا{
    btreeParseCellAdjustSizeForOverflow(pPage, pCell, pInfo);
  }
}
ساكن فراغ btreeParseCell(
  MemPage *pPage,         /* Page containing the cell */
  صحيح iCell,              /* The cell index.  First cell is 0 */
  CellInfo *pInfo         /* Fill in this structure */
){
  pPage->xParseCell(pPage, findCell(pPage, iCell), pInfo);
}

/*
** The following routines are implementations of the MemPage.xCellSize
** method.
**
** Compute the total number of bytes that a Cell needs in the cell
** data area of the btree-page.  The return number includes the cell
** data header and the local payload, but not any overflow page or
** the space used by the cell pointer.
**
** cellSizePtrNoPayload()    =>   table internal nodes
** cellSizePtrTableLeaf()    =>   table leaf nodes
** cellSizePtr()             =>   index internal nodes
** cellSizeIdxLeaf()         =>   index leaf nodes
*/
ساكن u16 cellSizePtr(MemPage *pPage, u8 *pCell){
  u8 *pIter = pCell + 4;                   /* For looping over bytes of pCell */
  u8 *pEnd;                                /* End mark for a varint */
  u32 nSize;                               /* Size value to return */

#ifdef SQLITE_DEBUG
  /* The value returned by this function should always be the same as
  ** the (CellInfo.nSize) value found by doing a full parse of the
  ** cell. If SQLITE_DEBUG is defined, an assert() at the bottom of
  ** this function verifies that this invariant is not violated. */
  CellInfo debuginfo;
  pPage->xParseCell(pPage, pCell, &debuginfo);
#endif

  أكد( pPage->childPtrSize==4 );
  nSize = *pIter;
  إذا( nSize>=0x80 ){
    pEnd = &pIter[8];
    nSize &= 0x7f;
    افعل{
      nSize = (nSize<<7) | (*++pIter & 0x7f);
    }طالما( *(pIter)>=0x80 && pIter<pEnd );
  }
  pIter++;
  testcase( nSize==pPage->maxLocal );
  testcase( nSize==(u32)pPage->maxLocal+1 );
  إذا( nSize<=pPage->maxLocal ){
    nSize += (u32)(pIter - pCell);
    أكد( nSize>4 );
  }وإلا{
    صحيح minLocal = pPage->minLocal;
    nSize = minLocal + (nSize - minLocal) % (pPage->pBt->usableSize - 4);
    testcase( nSize==pPage->maxLocal );
    testcase( nSize==(u32)pPage->maxLocal+1 );
    إذا( nSize>pPage->maxLocal ){
      nSize = minLocal;
    }
    nSize += 4 + (u16)(pIter - pCell);
  }
  أكد( nSize==debuginfo.nSize || CORRUPT_DB );
  أرجع (u16)nSize;
}
ساكن u16 cellSizePtrIdxLeaf(MemPage *pPage, u8 *pCell){
  u8 *pIter = pCell;                       /* For looping over bytes of pCell */
  u8 *pEnd;                                /* End mark for a varint */
  u32 nSize;                               /* Size value to return */

#ifdef SQLITE_DEBUG
  /* The value returned by this function should always be the same as
  ** the (CellInfo.nSize) value found by doing a full parse of the
  ** cell. If SQLITE_DEBUG is defined, an assert() at the bottom of
  ** this function verifies that this invariant is not violated. */
  CellInfo debuginfo;
  pPage->xParseCell(pPage, pCell, &debuginfo);
#endif

  أكد( pPage->childPtrSize==0 );
  nSize = *pIter;
  إذا( nSize>=0x80 ){
    pEnd = &pIter[8];
    nSize &= 0x7f;
    افعل{
      nSize = (nSize<<7) | (*++pIter & 0x7f);
    }طالما( *(pIter)>=0x80 && pIter<pEnd );
  }
  pIter++;
  testcase( nSize==pPage->maxLocal );
  testcase( nSize==(u32)pPage->maxLocal+1 );
  إذا( nSize<=pPage->maxLocal ){
    nSize += (u32)(pIter - pCell);
    إذا( nSize<4 ) nSize = 4;
  }وإلا{
    صحيح minLocal = pPage->minLocal;
    nSize = minLocal + (nSize - minLocal) % (pPage->pBt->usableSize - 4);
    testcase( nSize==pPage->maxLocal );
    testcase( nSize==(u32)pPage->maxLocal+1 );
    إذا( nSize>pPage->maxLocal ){
      nSize = minLocal;
    }
    nSize += 4 + (u16)(pIter - pCell);
  }
  أكد( nSize==debuginfo.nSize || CORRUPT_DB );
  أرجع (u16)nSize;
}
ساكن u16 cellSizePtrNoPayload(MemPage *pPage, u8 *pCell){
  u8 *pIter = pCell + 4; /* For looping over bytes of pCell */
  u8 *pEnd;              /* End mark for a varint */

#ifdef SQLITE_DEBUG
  /* The value returned by this function should always be the same as
  ** the (CellInfo.nSize) value found by doing a full parse of the
  ** cell. If SQLITE_DEBUG is defined, an assert() at the bottom of
  ** this function verifies that this invariant is not violated. */
  CellInfo debuginfo;
  pPage->xParseCell(pPage, pCell, &debuginfo);
#وإلا
  UNUSED_PARAMETER(pPage);
#endif

  أكد( pPage->childPtrSize==4 );
  pEnd = pIter + 9;
  طالما( (*pIter++)&0x80 && pIter<pEnd );
  أكد( debuginfo.nSize==(u16)(pIter - pCell) || CORRUPT_DB );
  أرجع (u16)(pIter - pCell);
}
ساكن u16 cellSizePtrTableLeaf(MemPage *pPage, u8 *pCell){
  u8 *pIter = pCell;   /* For looping over bytes of pCell */
  u8 *pEnd;            /* End mark for a varint */
  u32 nSize;           /* Size value to return */

#ifdef SQLITE_DEBUG
  /* The value returned by this function should always be the same as
  ** the (CellInfo.nSize) value found by doing a full parse of the
  ** cell. If SQLITE_DEBUG is defined, an assert() at the bottom of
  ** this function verifies that this invariant is not violated. */
  CellInfo debuginfo;
  pPage->xParseCell(pPage, pCell, &debuginfo);
#endif

  nSize = *pIter;
  إذا( nSize>=0x80 ){
    pEnd = &pIter[8];
    nSize &= 0x7f;
    افعل{
      nSize = (nSize<<7) | (*++pIter & 0x7f);
    }طالما( *(pIter)>=0x80 && pIter<pEnd );
  }
  pIter++;
  /* pIter now points at the 64-bit integer key value, a variable length
  ** integer. The following block moves pIter to point at the first byte
  ** past the end of the key value. */
  إذا( (*pIter++)&0x80
   && (*pIter++)&0x80
   && (*pIter++)&0x80
   && (*pIter++)&0x80
   && (*pIter++)&0x80
   && (*pIter++)&0x80
   && (*pIter++)&0x80
   && (*pIter++)&0x80 ){ pIter++; }
  testcase( nSize==pPage->maxLocal );
  testcase( nSize==(u32)pPage->maxLocal+1 );
  إذا( nSize<=pPage->maxLocal ){
    nSize += (u32)(pIter - pCell);
    إذا( nSize<4 ) nSize = 4;
  }وإلا{
    صحيح minLocal = pPage->minLocal;
    nSize = minLocal + (nSize - minLocal) % (pPage->pBt->usableSize - 4);
    testcase( nSize==pPage->maxLocal );
    testcase( nSize==(u32)pPage->maxLocal+1 );
    إذا( nSize>pPage->maxLocal ){
      nSize = minLocal;
    }
    nSize += 4 + (u16)(pIter - pCell);
  }
  أكد( nSize==debuginfo.nSize || CORRUPT_DB );
  أرجع (u16)nSize;
}


#ifdef SQLITE_DEBUG
/* This variation on cellSizePtr() is used inside of assert() statements
** only. */
ساكن u16 cellSize(MemPage *pPage, صحيح iCell){
  أرجع pPage->xCellSize(pPage, findCell(pPage, iCell));
}
#endif

#ifndef SQLITE_OMIT_AUTOVACUUM
/*
** The cell pCell is currently part of page pSrc but will ultimately be part
** of pPage.  (pSrc and pPage are often the same.)  If pCell contains a
** pointer to an overflow page, insert an entry into the pointer-map for
** the overflow page that will be valid after pCell has been moved to pPage.
*/
ساكن فراغ ptrmapPutOvflPtr(MemPage *pPage, MemPage *pSrc, u8 *pCell,صحيح *pRC){
  CellInfo info;
  إذا( *pRC ) أرجع;
  أكد( pCell!=0 );
  pPage->xParseCell(pPage, pCell, &info);
  إذا( info.nLocal<info.nPayload ){
    Pgno ovfl;
    إذا( SQLITE_OVERFLOW(pSrc->aDataEnd, pCell, pCell+info.nLocal) ){
      testcase( pSrc!=pPage );
      *pRC = SQLITE_CORRUPT_BKPT;
      أرجع;
    }
    ovfl = get4byte(&pCell[info.nSize-4]);
    ptrmapPut(pPage->pBt, ovfl, PTRMAP_OVERFLOW1, pPage->pgno, pRC);
  }
}
#endif


/*
** Defragment the page given. This routine reorganizes cells within the
** page so that there are no free-blocks on the free-block list.
**
** Parameter nMaxFrag is the maximum amount of fragmented space that may be
** present in the page after this routine returns.
**
** EVIDENCE-OF: R-44582-60138 SQLite may from time to time reorganize a
** b-tree page so that there are no freeblocks or fragment bytes, all
** unused bytes are contained in the unallocated space region, and all
** cells are packed tightly at the end of the page.
*/
ساكن صحيح defragmentPage(MemPage *pPage, صحيح nMaxFrag){
  صحيح i;                     /* Loop counter */
  صحيح pc;                    /* Address of the i-th cell */
  صحيح hdr;                   /* Offset to the page header */
  صحيح size;                  /* Size of a cell */
  صحيح usableSize;            /* Number of usable bytes on a page */
  صحيح cellOffset;            /* Offset to the cell pointer array */
  صحيح cbrk;                  /* Offset to the cell content area */
  صحيح nCell;                 /* Number of cells on the page */
  unsigned حرف *data;       /* The page data */
  unsigned حرف *temp;       /* Temp area for cell content */
  unsigned حرف *src;        /* Source of content */
  صحيح iCellFirst;            /* First allowable cell index */
  صحيح iCellLast;             /* Last possible cell index */
  صحيح iCellStart;            /* First cell offset in input */

  أكد( sqlite3PagerIswriteable(pPage->pDbPage) );
  أكد( pPage->pBt!=0 );
  أكد( pPage->pBt->usableSize <= SQLITE_MAX_PAGE_SIZE );
  أكد( pPage->nOverflow==0 );
  أكد( sqlite3_mutex_held(pPage->pBt->mutex) );
  data = pPage->aData;
  hdr = pPage->hdrOffset;
  cellOffset = pPage->cellOffset;
  nCell = pPage->nCell;
  أكد( nCell==get2byte(&data[hdr+3]) || CORRUPT_DB );
  iCellFirst = cellOffset + 2*nCell;
  usableSize = pPage->pBt->usableSize;

  /* This block handles pages with two or fewer free blocks and nMaxFrag
  ** or fewer fragmented bytes. In this case it is faster to move the
  ** two (or one) blocks of cells using memmove() and add the required
  ** offsets to each pointer in the cell-pointer array than it is to
  ** reconstruct the entire page.  */
  إذا( (صحيح)data[hdr+7]<=nMaxFrag ){
    صحيح iFree = get2byte(&data[hdr+1]);
    إذا( iFree>usableSize-4 ) أرجع SQLITE_CORRUPT_PAGE(pPage);
    إذا( iFree ){
      صحيح iFree2 = get2byte(&data[iFree]);
      إذا( iFree2>usableSize-4 ) أرجع SQLITE_CORRUPT_PAGE(pPage);
      إذا( 0==iFree2 || (data[iFree2]==0 && data[iFree2+1]==0) ){
        u8 *pEnd = &data[cellOffset + nCell*2];
        u8 *pAddr;
        صحيح sz2 = 0;
        صحيح sz = get2byte(&data[iFree+2]);
        صحيح چوققا = get2byte(&data[hdr+5]);
        إذا( چوققا>=iFree ){
          أرجع SQLITE_CORRUPT_PAGE(pPage);
        }
        إذا( iFree2 ){
          إذا( iFree+sz>iFree2 ) أرجع SQLITE_CORRUPT_PAGE(pPage);
          sz2 = get2byte(&data[iFree2+2]);
          إذا( iFree2+sz2 > usableSize ) أرجع SQLITE_CORRUPT_PAGE(pPage);
          memmove(&data[iFree+sz+sz2], &data[iFree+sz], iFree2-(iFree+sz));
          sz += sz2;
        }وإلا إذا( iFree+sz>usableSize ){
          أرجع SQLITE_CORRUPT_PAGE(pPage);
        }

        cbrk = چوققا+sz;
        أكد( cbrk+(iFree-چوققا) <= usableSize );
        memmove(&data[cbrk], &data[چوققا], iFree-چوققا);
        لكل(pAddr=&data[cellOffset]; pAddr<pEnd; pAddr+=2){
          pc = get2byte(pAddr);
          إذا( pc<iFree ){ put2byte(pAddr, pc+sz); }
          وإلا إذا( pc<iFree2 ){ put2byte(pAddr, pc+sz2); }
        }
        اذهب defragment_out;
      }
    }
  }

  cbrk = usableSize;
  iCellLast = usableSize - 4;
  iCellStart = get2byte(&data[hdr+5]);
  إذا( nCell>0 ){
    temp = sqlite3PagerTempSpace(pPage->pBt->pPager);
    memcpy(temp, data, usableSize);
    src = temp;
    لكل(i=0; i<nCell; i++){
      u8 *pAddr;     /* The i-th cell pointer */
      pAddr = &data[cellOffset + i*2];
      pc = get2byte(pAddr);
      testcase( pc==iCellFirst );
      testcase( pc==iCellLast );
      /* These conditions have already been verified in btreeInitPage()
      ** if PRAGMA cell_size_check=ON.
      */
      إذا( pc>iCellLast ){
        أرجع SQLITE_CORRUPT_PAGE(pPage);
      }
      أكد( pc>=0 && pc<=iCellLast );
      size = pPage->xCellSize(pPage, &src[pc]);
      cbrk -= size;
      إذا( cbrk<iCellStart || pc+size>usableSize ){
        أرجع SQLITE_CORRUPT_PAGE(pPage);
      }
      أكد( cbrk+size<=usableSize && cbrk>=iCellStart );
      testcase( cbrk+size==usableSize );
      testcase( pc+size==usableSize );
      put2byte(pAddr, cbrk);
      memcpy(&data[cbrk], &src[pc], size);
    }
  }
  data[hdr+7] = 0;

defragment_out:
  أكد( pPage->nFree>=0 );
  إذا( data[hdr+7]+cbrk-iCellFirst!=pPage->nFree ){
    أرجع SQLITE_CORRUPT_PAGE(pPage);
  }
  أكد( cbrk>=iCellFirst );
  put2byte(&data[hdr+5], cbrk);
  data[hdr+1] = 0;
  data[hdr+2] = 0;
  memset(&data[iCellFirst], 0, cbrk-iCellFirst);
  أكد( sqlite3PagerIswriteable(pPage->pDbPage) );
  أرجع SQLITE_OK;
}

/*
** Search the free-list on page pPg for space to store a cell nByte bytes in
** size. If one can be found, return a pointer to the space and remove it
** from the free-list.
**
** If no suitable space can be found on the free-list, return NULL.
**
** This function may detect corruption within pPg.  If corruption is
** detected then *pRc is set to SQLITE_CORRUPT and NULL is returned.
**
** Slots on the free list that are between 1 and 3 bytes larger than nByte
** will be ignored if adding the extra space to the fragmentation count
** causes the fragmentation count to exceed 60.
*/
ساكن u8 *pageFindSlot(MemPage *pPg, صحيح nByte, صحيح *pRc){
  ثابت صحيح hdr = pPg->hdrOffset;            /* Offset to page header */
  u8 * ثابت aData = pPg->aData;             /* Page data */
  صحيح iAddr = hdr + 1;                       /* Address of ptr to pc */
  u8 *pTmp = &aData[iAddr];                  /* Temporary ptr into aData[] */
  صحيح pc = get2byte(pTmp);                   /* Address of a free slot */
  صحيح x;                                     /* Excess size of the slot */
  صحيح maxPC = pPg->pBt->usableSize - nByte;  /* Max address for a usable slot */
  صحيح size;                                  /* Size of the free slot */

  أكد( pc>0 );
  طالما( pc<=maxPC ){
    /* EVIDENCE-OF: R-22710-53328 The third and fourth bytes of each
    ** freeblock form a big-endian integer which is the size of the freeblock
    ** in bytes, including the 4-byte header. */
    pTmp = &aData[pc+2];
    size = get2byte(pTmp);
    إذا( (x = size - nByte)>=0 ){
      testcase( x==4 );
      testcase( x==3 );
      إذا( x<4 ){
        /* EVIDENCE-OF: R-11498-58022 In a well-formed b-tree page, the total
        ** number of bytes in fragments may not exceed 60. */
        إذا( aData[hdr+7]>57 ) أرجع 0;

        /* Remove the slot from the free-list. Update the number of
        ** fragmented bytes within the page. */
        memcpy(&aData[iAddr], &aData[pc], 2);
        aData[hdr+7] += (u8)x;
        أرجع &aData[pc];
      }وإلا إذا( x+pc > maxPC ){
        /* This slot extends off the end of the usable part of the page */
        *pRc = SQLITE_CORRUPT_PAGE(pPg);
        أرجع 0;
      }وإلا{
        /* The slot remains on the free-list. Reduce its size to account
        ** for the portion used by the new allocation. */
        put2byte(&aData[pc+2], x);
      }
      أرجع &aData[pc + x];
    }
    iAddr = pc;
    pTmp = &aData[pc];
    pc = get2byte(pTmp);
    إذا( pc<=iAddr ){
      إذا( pc ){
        /* The next slot in the chain comes before the current slot */
        *pRc = SQLITE_CORRUPT_PAGE(pPg);
      }
      أرجع 0;
    }
  }
  إذا( pc>maxPC+nByte-4 ){
    /* The free slot chain extends off the end of the page */
    *pRc = SQLITE_CORRUPT_PAGE(pPg);
  }
  أرجع 0;
}

/*
** Allocate nByte bytes of space from within the B-Tree page passed
** as the first argument. Write into *pIdx the index into pPage->aData[]
** of the first byte of allocated space. Return either SQLITE_OK or
** an error code (usually SQLITE_CORRUPT).
**
** The caller guarantees that there is sufficient space to make the
** allocation.  This routine might need to defragment in order to bring
** all the space together, however.  This routine will avoid using
** the first two bytes past the cell pointer area since presumably this
** allocation is being made in order to insert a new cell, so we will
** also end up needing a new cell pointer.
*/
ساكن SQLITE_INLINE صحيح allocateSpace(MemPage *pPage, صحيح nByte, صحيح *pIdx){
  ثابت صحيح hdr = pPage->hdrOffset;    /* Local cache of pPage->hdrOffset */
  u8 * ثابت data = pPage->aData;      /* Local cache of pPage->aData */
  صحيح چوققا;                             /* First byte of cell content area */
  صحيح rc = SQLITE_OK;                  /* Integer return code */
  u8 *pTmp;                            /* Temp ptr into data[] */
  صحيح gap;        /* First byte of gap between cell pointers and cell content */
 
  أكد( sqlite3PagerIswriteable(pPage->pDbPage) );
  أكد( pPage->pBt );
  أكد( sqlite3_mutex_held(pPage->pBt->mutex) );
  أكد( nByte>=0 );  /* Minimum cell size is 4 */
  أكد( pPage->nFree>=nByte );
  أكد( pPage->nOverflow==0 );
  أكد( nByte < (صحيح)(pPage->pBt->usableSize-8) );

  أكد( pPage->cellOffset == hdr + 12 - 4*pPage->leaf );
  gap = pPage->cellOffset + 2*pPage->nCell;
  أكد( gap<=65536 );
  /* EVIDENCE-OF: R-29356-02391 If the database uses a 65536-byte page size
  ** and the reserved space is zero (the usual value for reserved space)
  ** then the cell content offset of an empty page wants to be 65536.
  ** However, that integer is too large to be stored in a 2-byte unsigned
  ** integer, so a value of 0 is used in its place. */
  pTmp = &data[hdr+5];
  چوققا = get2byte(pTmp);
  إذا( gap>چوققا ){
    إذا( چوققا==0 && pPage->pBt->usableSize==65536 ){
      چوققا = 65536;
    }وإلا{
      أرجع SQLITE_CORRUPT_PAGE(pPage);
    }
  }وإلا إذا( چوققا>(صحيح)pPage->pBt->usableSize ){
    أرجع SQLITE_CORRUPT_PAGE(pPage);
  }

  /* If there is enough space between gap and top for one more cell pointer,
  ** and if the freelist is not empty, then search the
  ** freelist looking for a slot big enough to satisfy the request.
  */
  testcase( gap+2==چوققا );
  testcase( gap+1==چوققا );
  testcase( gap==چوققا );
  إذا( (data[hdr+2] || data[hdr+1]) && gap+2<=چوققا ){
    u8 *pSpace = pageFindSlot(pPage, nByte, &rc);
    إذا( pSpace ){
      صحيح g2;
      أكد( pSpace+nByte<=data+pPage->pBt->usableSize );
      *pIdx = g2 = (صحيح)(pSpace-data);
      إذا( g2<=gap ){
        أرجع SQLITE_CORRUPT_PAGE(pPage);
      }وإلا{
        أرجع SQLITE_OK;
      }
    }وإلا إذا( rc ){
      أرجع rc;
    }
  }

  /* The request could not be fulfilled using a freelist slot.  Check
  ** to see if defragmentation is necessary.
  */
  testcase( gap+2+nByte==چوققا );
  إذا( gap+2+nByte>چوققا ){
    أكد( pPage->nCell>0 || CORRUPT_DB );
    أكد( pPage->nFree>=0 );
    rc = defragmentPage(pPage, MIN(4, pPage->nFree - (2+nByte)));
    إذا( rc ) أرجع rc;
    چوققا = get2byteNotZero(&data[hdr+5]);
    أكد( gap+2+nByte<=چوققا );
  }


  /* Allocate memory from the gap in between the cell pointer array
  ** and the cell content area.  The btreeComputeFreeSpace() call has already
  ** validated the freelist.  Given that the freelist is valid, there
  ** is no way that the allocation can extend off the end of the page.
  ** The assert() below verifies the previous sentence.
  */
  چوققا -= nByte;
  put2byte(&data[hdr+5], چوققا);
  أكد( چوققا+nByte <= (صحيح)pPage->pBt->usableSize );
  *pIdx = چوققا;
  أرجع SQLITE_OK;
}

/*
** Return a section of the pPage->aData to the freelist.
** The first byte of the new free block is pPage->aData[iStart]
** and the size of the block is iSize bytes.
**
** Adjacent freeblocks are coalesced.
**
** Even though the freeblock list was checked by btreeComputeFreeSpace(),
** that routine will not detect overlap between cells or freeblocks.  Nor
** does it detect cells or freeblocks that encroach into the reserved bytes
** at the end of the page.  So do additional corruption checks inside this
** routine and return SQLITE_CORRUPT if any problems are found.
*/
ساكن صحيح freeSpace(MemPage *pPage, صحيح iStart, صحيح iSize){
  صحيح iPtr;                             /* Address of ptr to next freeblock */
  صحيح iFreeBlk;                         /* Address of the next freeblock */
  u8 hdr;                               /* Page header size.  0 or 100 */
  صحيح nFrag = 0;                        /* Reduction in fragmentation */
  صحيح iOrigSize = iSize;                /* Original value of iSize */
  صحيح x;                                /* Offset to cell content area */
  صحيح iEnd = iStart + iSize;            /* First byte past the iStart buffer */
  unsigned حرف *data = pPage->aData;   /* Page content */
  u8 *pTmp;                             /* Temporary ptr into data[] */

  أكد( pPage->pBt!=0 );
  أكد( sqlite3PagerIswriteable(pPage->pDbPage) );
  أكد( CORRUPT_DB || iStart>=pPage->hdrOffset+6+pPage->childPtrSize );
  أكد( CORRUPT_DB || iEnd <= (صحيح)pPage->pBt->usableSize );
  أكد( sqlite3_mutex_held(pPage->pBt->mutex) );
  أكد( iSize>=4 );   /* Minimum cell size is 4 */
  أكد( CORRUPT_DB || iStart<=(صحيح)pPage->pBt->usableSize-4 );

  /* The list of freeblocks must be in ascending order.  Find the
  ** spot on the list where iStart should be inserted.
  */
  hdr = pPage->hdrOffset;
  iPtr = hdr + 1;
  إذا( data[iPtr+1]==0 && data[iPtr]==0 ){
    iFreeBlk = 0;  /* Shortcut for the case when the freelist is empty */
  }وإلا{
    طالما( (iFreeBlk = get2byte(&data[iPtr]))<iStart ){
      إذا( iFreeBlk<=iPtr ){
        إذا( iFreeBlk==0 ) اكسر; /* TH3: corrupt082.100 */
        أرجع SQLITE_CORRUPT_PAGE(pPage);
      }
      iPtr = iFreeBlk;
    }
    إذا( iFreeBlk>(صحيح)pPage->pBt->usableSize-4 ){ /* TH3: corrupt081.100 */
      أرجع SQLITE_CORRUPT_PAGE(pPage);
    }
    أكد( iFreeBlk>iPtr || iFreeBlk==0 || CORRUPT_DB );
 
    /* At this point:
    **    iFreeBlk:   First freeblock after iStart, or zero if none
    **    iPtr:       The address of a pointer to iFreeBlk
    **
    ** Check to see if iFreeBlk should be coalesced onto the end of iStart.
    */
    إذا( iFreeBlk && iEnd+3>=iFreeBlk ){
      nFrag = iFreeBlk - iEnd;
      إذا( iEnd>iFreeBlk ) أرجع SQLITE_CORRUPT_PAGE(pPage);
      iEnd = iFreeBlk + get2byte(&data[iFreeBlk+2]);
      إذا( iEnd > (صحيح)pPage->pBt->usableSize ){
        أرجع SQLITE_CORRUPT_PAGE(pPage);
      }
      iSize = iEnd - iStart;
      iFreeBlk = get2byte(&data[iFreeBlk]);
    }
 
    /* If iPtr is another freeblock (that is, if iPtr is not the freelist
    ** pointer in the page header) then check to see if iStart should be
    ** coalesced onto the end of iPtr.
    */
    إذا( iPtr>hdr+1 ){
      صحيح iPtrEnd = iPtr + get2byte(&data[iPtr+2]);
      إذا( iPtrEnd+3>=iStart ){
        إذا( iPtrEnd>iStart ) أرجع SQLITE_CORRUPT_PAGE(pPage);
        nFrag += iStart - iPtrEnd;
        iSize = iEnd - iPtr;
        iStart = iPtr;
      }
    }
    إذا( nFrag>data[hdr+7] ) أرجع SQLITE_CORRUPT_PAGE(pPage);
    data[hdr+7] -= (u8)nFrag;
  }
  pTmp = &data[hdr+5];
  x = get2byte(pTmp);
  إذا( pPage->pBt->btsFlags & BTS_FAST_SECURE ){
    /* Overwrite deleted information with zeros when the secure_delete
    ** option is enabled */
    memset(&data[iStart], 0, iSize);
  }
  إذا( iStart<=x ){
    /* The new freeblock is at the beginning of the cell content area,
    ** so just extend the cell content area rather than create another
    ** freelist entry */
    إذا( iStart<x ) أرجع SQLITE_CORRUPT_PAGE(pPage);
    إذا( iPtr!=hdr+1 ) أرجع SQLITE_CORRUPT_PAGE(pPage);
    put2byte(&data[hdr+1], iFreeBlk);
    put2byte(&data[hdr+5], iEnd);
  }وإلا{
    /* Insert the new freeblock into the freelist */
    put2byte(&data[iPtr], iStart);
    put2byte(&data[iStart], iFreeBlk);
    أكد( iSize>=0 && iSize<=0xffff );
    put2byte(&data[iStart+2], (u16)iSize);
  }
  pPage->nFree += iOrigSize;
  أرجع SQLITE_OK;
}

/*
** Decode the flags byte (the first byte of the header) for a page
** and initialize fields of the MemPage structure accordingly.
**
** Only the following combinations are supported.  Anything different
** indicates a corrupt database files:
**
**         PTF_ZERODATA                             (0x02,  2)
**         PTF_LEAFDATA | PTF_INTKEY                (0x05,  5)
**         PTF_ZERODATA | PTF_LEAF                  (0x0a, 10)
**         PTF_LEAFDATA | PTF_INTKEY | PTF_LEAF     (0x0d, 13)
*/
ساكن صحيح decodeFlags(MemPage *pPage, صحيح flagByte){
  BtShared *pBt;     /* A copy of pPage->pBt */

  أكد( pPage->hdrOffset==(pPage->pgno==1 ? 100 : 0) );
  أكد( sqlite3_mutex_held(pPage->pBt->mutex) );
  pBt = pPage->pBt;
  pPage->max1bytePayload = pBt->max1bytePayload;
  إذا( flagByte>=(PTF_ZERODATA | PTF_LEAF) ){
    pPage->childPtrSize = 0;
    pPage->leaf = 1;
    إذا( flagByte==(PTF_LEAFDATA | PTF_INTKEY | PTF_LEAF) ){
      pPage->int_ئاچقۇچ_Leaf = 1;
      pPage->xCellSize = cellSizePtrTableLeaf;
      pPage->xParseCell = btreeParseCellPtr;
      pPage->int_ئاچقۇچ = 1;
      pPage->maxLocal = pBt->maxLeaf;
      pPage->minLocal = pBt->minLeaf;
    }وإلا إذا( flagByte==(PTF_ZERODATA | PTF_LEAF) ){
      pPage->int_ئاچقۇچ = 0;
      pPage->int_ئاچقۇچ_Leaf = 0;
      pPage->xCellSize = cellSizePtrIdxLeaf;
      pPage->xParseCell = btreeParseCellPtrIndex;
      pPage->maxLocal = pBt->maxLocal;
      pPage->minLocal = pBt->minLocal;
    }وإلا{
      pPage->int_ئاچقۇچ = 0;
      pPage->int_ئاچقۇچ_Leaf = 0;
      pPage->xCellSize = cellSizePtrIdxLeaf;
      pPage->xParseCell = btreeParseCellPtrIndex;
      أرجع SQLITE_CORRUPT_PAGE(pPage);
    }
  }وإلا{
    pPage->childPtrSize = 4;
    pPage->leaf = 0;
    إذا( flagByte==(PTF_ZERODATA) ){
      pPage->int_ئاچقۇچ = 0;
      pPage->int_ئاچقۇچ_Leaf = 0;
      pPage->xCellSize = cellSizePtr;
      pPage->xParseCell = btreeParseCellPtrIndex;
      pPage->maxLocal = pBt->maxLocal;
      pPage->minLocal = pBt->minLocal;
    }وإلا إذا( flagByte==(PTF_LEAFDATA | PTF_INTKEY) ){
      pPage->int_ئاچقۇچ_Leaf = 0;
      pPage->xCellSize = cellSizePtrNoPayload;
      pPage->xParseCell = btreeParseCellPtrNoPayload;
      pPage->int_ئاچقۇچ = 1;
      pPage->maxLocal = pBt->maxLeaf;
      pPage->minLocal = pBt->minLeaf;
    }وإلا{
      pPage->int_ئاچقۇچ = 0;
      pPage->int_ئاچقۇچ_Leaf = 0;
      pPage->xCellSize = cellSizePtr;
      pPage->xParseCell = btreeParseCellPtrIndex;
      أرجع SQLITE_CORRUPT_PAGE(pPage);
    }
  }
  أرجع SQLITE_OK;
}

/*
** Compute the amount of freespace on the page.  In other words, fill
** in the pPage->nFree field.
*/
ساكن صحيح btreeComputeFreeSpace(MemPage *pPage){
  صحيح pc;            /* Address of a freeblock within pPage->aData[] */
  u8 hdr;            /* Offset to beginning of page header */
  u8 *data;          /* Equal to pPage->aData */
  صحيح usableSize;    /* Amount of usable space on each page */
  صحيح nFree;         /* Number of unused bytes on the page */
  صحيح چوققا;           /* First byte of the cell content area */
  صحيح iCellFirst;    /* First allowable cell or freeblock offset */
  صحيح iCellLast;     /* Last possible cell or freeblock offset */

  أكد( pPage->pBt!=0 );
  أكد( pPage->pBt->db!=0 );
  أكد( sqlite3_mutex_held(pPage->pBt->mutex) );
  أكد( pPage->pgno==sqlite3PagerPagenumber(pPage->pDbPage) );
  أكد( pPage == sqlite3PagerGetExtra(pPage->pDbPage) );
  أكد( pPage->aData == sqlite3PagerGetData(pPage->pDbPage) );
  أكد( pPage->isInit==1 );
  أكد( pPage->nFree<0 );

  usableSize = pPage->pBt->usableSize;
  hdr = pPage->hdrOffset;
  data = pPage->aData;
  /* EVIDENCE-OF: R-58015-48175 The two-byte integer at offset 5 designates
  ** the start of the cell content area. A zero value for this integer is
  ** interpreted as 65536. */
  چوققا = get2byteNotZero(&data[hdr+5]);
  iCellFirst = hdr + 8 + pPage->childPtrSize + 2*pPage->nCell;
  iCellLast = usableSize - 4;

  /* Compute the total free space on the page
  ** EVIDENCE-OF: R-23588-34450 The two-byte integer at offset 1 gives the
  ** start of the first freeblock on the page, or is zero if there are no
  ** freeblocks. */
  pc = get2byte(&data[hdr+1]);
  nFree = data[hdr+7] + چوققا;  /* Init nFree to non-freeblock free space */
  إذا( pc>0 ){
    u32 next, size;
    إذا( pc<چوققا ){
      /* EVIDENCE-OF: R-55530-52930 In a well-formed b-tree page, there will
      ** always be at least one cell before the first freeblock.
      */
      أرجع SQLITE_CORRUPT_PAGE(pPage);
    }
    طالما( 1 ){
      إذا( pc>iCellLast ){
        /* Freeblock off the end of the page */
        أرجع SQLITE_CORRUPT_PAGE(pPage);
      }
      next = get2byte(&data[pc]);
      size = get2byte(&data[pc+2]);
      إذا( size<4 && sqlite3FaultSim(422)==SQLITE_OK ){
        /* Minimum freeblock size is 4.  Enable fault-sim 422 to disable this
        ** check to reach interesting error states.  However, disabling this
        ** check can cause assertion faults due to min-heap overflow.  All
        ** fault-sims are for testing use only, but this one especially so. */
        أرجع SQLITE_CORRUPT_PAGE(pPage);
      }
      nFree = nFree + size;
      إذا( next<pc+size+4 ) اكسر;
      pc = next;
    }
    إذا( next>0 ){
      /* Freeblock not in ascending order */
      أرجع SQLITE_CORRUPT_PAGE(pPage);
    }
    إذا( pc+size>(unsigned صحيح)usableSize ){
      /* Last freeblock extends past page end */
      أرجع SQLITE_CORRUPT_PAGE(pPage);
    }
  }

  /* At this point, nFree contains the sum of the offset to the start
  ** of the cell-content area plus the number of free bytes within
  ** the cell-content area. If this is greater than the usable-size
  ** of the page, then the page must be corrupted. This check also
  ** serves to verify that the offset to the start of the cell-content
  ** area, according to the page header, lies within the page.
  */
  إذا( nFree>usableSize || nFree<iCellFirst ){
    أرجع SQLITE_CORRUPT_PAGE(pPage);
  }
  pPage->nFree = (u16)(nFree - iCellFirst);
  أرجع SQLITE_OK;
}

/*
** Do additional sanity check after btreeInitPage() if
** PRAGMA cell_size_check=ON
*/
ساكن SQLITE_NOINLINE صحيح btreeCellSizeCheck(MemPage *pPage){
  صحيح iCellFirst;    /* First allowable cell or freeblock offset */
  صحيح iCellLast;     /* Last possible cell or freeblock offset */
  صحيح i;             /* Index into the cell pointer array */
  صحيح sz;            /* Size of a cell */
  صحيح pc;            /* Address of a freeblock within pPage->aData[] */
  u8 *data;          /* Equal to pPage->aData */
  صحيح usableSize;    /* Maximum usable space on the page */
  صحيح cellOffset;    /* Start of cell content area */

  iCellFirst = pPage->cellOffset + 2*pPage->nCell;
  usableSize = pPage->pBt->usableSize;
  iCellLast = usableSize - 4;
  data = pPage->aData;
  cellOffset = pPage->cellOffset;
  إذا( !pPage->leaf ) iCellLast--;
  لكل(i=0; i<pPage->nCell; i++){
    pc = get2byteAligned(&data[cellOffset+i*2]);
    testcase( pc==iCellFirst );
    testcase( pc==iCellLast );
    إذا( pc<iCellFirst || pc>iCellLast ){
      أرجع SQLITE_CORRUPT_PAGE(pPage);
    }
    sz = pPage->xCellSize(pPage, &data[pc]);
    testcase( pc+sz==usableSize );
    إذا( pc+sz>usableSize ){
      أرجع SQLITE_CORRUPT_PAGE(pPage);
    }
  }
  أرجع SQLITE_OK;
}

/*
** Initialize the auxiliary information for a disk block.
**
** Return SQLITE_OK on success.  If we see that the page does
** not contain a well-formed database page, then return
** SQLITE_CORRUPT.  Note that a return of SQLITE_OK does not
** guarantee that the page is well-formed.  It only shows that
** we failed to detect any corruption.
*/
ساكن صحيح btreeInitPage(MemPage *pPage){
  u8 *data;          /* Equal to pPage->aData */
  BtShared *pBt;        /* The main btree structure */

  أكد( pPage->pBt!=0 );
  أكد( pPage->pBt->db!=0 );
  أكد( sqlite3_mutex_held(pPage->pBt->mutex) );
  أكد( pPage->pgno==sqlite3PagerPagenumber(pPage->pDbPage) );
  أكد( pPage == sqlite3PagerGetExtra(pPage->pDbPage) );
  أكد( pPage->aData == sqlite3PagerGetData(pPage->pDbPage) );
  أكد( pPage->isInit==0 );

  pBt = pPage->pBt;
  data = pPage->aData + pPage->hdrOffset;
  /* EVIDENCE-OF: R-28594-02890 The one-byte flag at offset 0 indicating
  ** the b-tree page type. */
  إذا( decodeFlags(pPage, data[0]) ){
    أرجع SQLITE_CORRUPT_PAGE(pPage);
  }
  أكد( pBt->pageSize>=512 && pBt->pageSize<=65536 );
  pPage->maskPage = (u16)(pBt->pageSize - 1);
  pPage->nOverflow = 0;
  pPage->cellOffset = (u16)(pPage->hdrOffset + 8 + pPage->childPtrSize);
  pPage->aCellIdx = data + pPage->childPtrSize + 8;
  pPage->aDataEnd = pPage->aData + pBt->pageSize;
  pPage->aDataOfst = pPage->aData + pPage->childPtrSize;
  /* EVIDENCE-OF: R-37002-32774 The two-byte integer at offset 3 gives the
  ** number of cells on the page. */
  pPage->nCell = get2byte(&data[3]);
  إذا( pPage->nCell>MX_CELL(pBt) ){
    /* To many cells for a single page.  The page must be corrupt */
    أرجع SQLITE_CORRUPT_PAGE(pPage);
  }
  testcase( pPage->nCell==MX_CELL(pBt) );
  /* EVIDENCE-OF: R-24089-57979 If a page contains no cells (which is only
  ** possible for a root page of a table that contains no rows) then the
  ** offset to the cell content area will equal the page size minus the
  ** bytes of reserved space. */
  أكد( pPage->nCell>0
       || get2byteNotZero(&data[5])==(صحيح)pBt->usableSize
       || CORRUPT_DB );
  pPage->nFree = -1;  /* Indicate that this value is yet uncomputed */
  pPage->isInit = 1;
  إذا( pBt->db->flags & SQLITE_CellSizeCk ){
    أرجع btreeCellSizeCheck(pPage);
  }
  أرجع SQLITE_OK;
}

/*
** Set up a raw page so that it looks like a database page holding
** no entries.
*/
ساكن فراغ zeroPage(MemPage *pPage, صحيح flags){
  unsigned حرف *data = pPage->aData;
  BtShared *pBt = pPage->pBt;
  صحيح hdr = pPage->hdrOffset;
  صحيح first;

  أكد( sqlite3PagerPagenumber(pPage->pDbPage)==pPage->pgno || CORRUPT_DB );
  أكد( sqlite3PagerGetExtra(pPage->pDbPage) == (فراغ*)pPage );
  أكد( sqlite3PagerGetData(pPage->pDbPage) == data );
  أكد( sqlite3PagerIswriteable(pPage->pDbPage) );
  أكد( sqlite3_mutex_held(pBt->mutex) );
  إذا( pBt->btsFlags & BTS_FAST_SECURE ){
    memset(&data[hdr], 0, pBt->usableSize - hdr);
  }
  data[hdr] = (حرف)flags;
  first = hdr + ((flags&PTF_LEAF)==0 ? 12 : 8);
  memset(&data[hdr+1], 0, 4);
  data[hdr+7] = 0;
  put2byte(&data[hdr+5], pBt->usableSize);
  pPage->nFree = (u16)(pBt->usableSize - first);
  decodeFlags(pPage, flags);
  pPage->cellOffset = (u16)first;
  pPage->aDataEnd = &data[pBt->pageSize];
  pPage->aCellIdx = &data[first];
  pPage->aDataOfst = &data[pPage->childPtrSize];
  pPage->nOverflow = 0;
  أكد( pBt->pageSize>=512 && pBt->pageSize<=65536 );
  pPage->maskPage = (u16)(pBt->pageSize - 1);
  pPage->nCell = 0;
  pPage->isInit = 1;
}


/*
** Convert a DbPage obtained from the pager into a MemPage used by
** the btree layer.
*/
ساكن MemPage *btreePageFromDbPage(DbPage *pDbPage, Pgno pgno, BtShared *pBt){
  MemPage *pPage = (MemPage*)sqlite3PagerGetExtra(pDbPage);
  إذا( pgno!=pPage->pgno ){
    pPage->aData = sqlite3PagerGetData(pDbPage);
    pPage->pDbPage = pDbPage;
    pPage->pBt = pBt;
    pPage->pgno = pgno;
    pPage->hdrOffset = pgno==1 ? 100 : 0;
  }
  أكد( pPage->aData==sqlite3PagerGetData(pDbPage) );
  أرجع pPage;
}

/*
** Get a page from the pager.  Initialize the MemPage.pBt and
** MemPage.aData elements if needed.  See also: btreeGetUnusedPage().
**
** If the PAGER_GET_NOCONTENT flag is set, it means that we do not care
** about the content of the page at this time.  So do not go to the disk
** to fetch the content.  Just fill in the content with zeros for now.
** If in the future we call sqlite3PagerWrite() on this page, that
** means we have started to be concerned about content and the disk
** read should occur at that point.
*/
ساكن صحيح btreeGetPage(
  BtShared *pBt,       /* The btree */
  Pgno pgno,           /* Number of the page to fetch */
  MemPage **ppPage,    /* Return the page in this parameter */
  صحيح flags            /* PAGER_GET_NOCONTENT or PAGER_GET_READONLY */
){
  صحيح rc;
  DbPage *pDbPage;

  أكد( flags==0 || flags==PAGER_GET_NOCONTENT || flags==PAGER_GET_READONLY );
  أكد( sqlite3_mutex_held(pBt->mutex) );
  rc = sqlite3PagerGet(pBt->pPager, pgno, (DbPage**)&pDbPage, flags);
  إذا( rc ) أرجع rc;
  *ppPage = btreePageFromDbPage(pDbPage, pgno, pBt);
  أرجع SQLITE_OK;
}

/*
** Retrieve a page from the pager cache. If the requested page is not
** already in the pager cache return NULL. Initialize the MemPage.pBt and
** MemPage.aData elements if needed.
*/
ساكن MemPage *btreePageLookup(BtShared *pBt, Pgno pgno){
  DbPage *pDbPage;
  أكد( sqlite3_mutex_held(pBt->mutex) );
  pDbPage = sqlite3PagerLookup(pBt->pPager, pgno);
  إذا( pDbPage ){
    أرجع btreePageFromDbPage(pDbPage, pgno, pBt);
  }
  أرجع 0;
}

/*
** Return the size of the database file in pages. If there is any kind of
** error, return ((unsigned int)-1).
*/
ساكن Pgno btreePagecount(BtShared *pBt){
  أرجع pBt->nPage;
}
Pgno sqlite3BtreeLastPage(Btree *p){
  أكد( sqlite3BtreeHoldsMutex(p) );
  أرجع btreePagecount(p->pBt);
}

/*
** Get a page from the pager and initialize it.
*/
ساكن صحيح getAndInitPage(
  BtShared *pBt,                  /* The database file */
  Pgno pgno,                      /* Number of the page to get */
  MemPage **ppPage,               /* Write the page pointer here */
  صحيح bReadOnly                   /* True for a read-only page */
){
  صحيح rc;
  DbPage *pDbPage;
  MemPage *pPage;
  أكد( sqlite3_mutex_held(pBt->mutex) );

  إذا( pgno>btreePagecount(pBt) ){
    *ppPage = 0;
    أرجع SQLITE_CORRUPT_BKPT;
  }
  rc = sqlite3PagerGet(pBt->pPager, pgno, (DbPage**)&pDbPage, bReadOnly);
  إذا( rc ){
    *ppPage = 0;
    أرجع rc;
  }
  pPage = (MemPage*)sqlite3PagerGetExtra(pDbPage);
  إذا( pPage->isInit==0 ){
    btreePageFromDbPage(pDbPage, pgno, pBt);
    rc = btreeInitPage(pPage);
    إذا( rc!=SQLITE_OK ){
      releasePage(pPage);
      *ppPage = 0;
      أرجع rc;
    }
  }
  أكد( pPage->pgno==pgno || CORRUPT_DB );
  أكد( pPage->aData==sqlite3PagerGetData(pDbPage) );
  *ppPage = pPage;
  أرجع SQLITE_OK;
}

/*
** Release a MemPage.  This should be called once for each prior
** call to btreeGetPage.
**
** Page1 is a special case and must be released using releasePageOne().
*/
ساكن فراغ releasePageNotNull(MemPage *pPage){
  أكد( pPage->aData );
  أكد( pPage->pBt );
  أكد( pPage->pDbPage!=0 );
  أكد( sqlite3PagerGetExtra(pPage->pDbPage) == (فراغ*)pPage );
  أكد( sqlite3PagerGetData(pPage->pDbPage)==pPage->aData );
  أكد( sqlite3_mutex_held(pPage->pBt->mutex) );
  sqlite3PagerUnrefNotNull(pPage->pDbPage);
}
ساكن فراغ releasePage(MemPage *pPage){
  إذا( pPage ) releasePageNotNull(pPage);
}
ساكن فراغ releasePageOne(MemPage *pPage){
  أكد( pPage!=0 );
  أكد( pPage->aData );
  أكد( pPage->pBt );
  أكد( pPage->pDbPage!=0 );
  أكد( sqlite3PagerGetExtra(pPage->pDbPage) == (فراغ*)pPage );
  أكد( sqlite3PagerGetData(pPage->pDbPage)==pPage->aData );
  أكد( sqlite3_mutex_held(pPage->pBt->mutex) );
  sqlite3PagerUnrefPageOne(pPage->pDbPage);
}

/*
** Get an unused page.
**
** This works just like btreeGetPage() with the addition:
**
**   *  If the page is already in use for some other purpose, immediately
**      release it and return an SQLITE_CURRUPT error.
**   *  Make sure the isInit flag is clear
*/
ساكن صحيح btreeGetUnusedPage(
  BtShared *pBt,       /* The btree */
  Pgno pgno,           /* Number of the page to fetch */
  MemPage **ppPage,    /* Return the page in this parameter */
  صحيح flags            /* PAGER_GET_NOCONTENT or PAGER_GET_READONLY */
){
  صحيح rc = btreeGetPage(pBt, pgno, ppPage, flags);
  إذا( rc==SQLITE_OK ){
    إذا( sqlite3PagerPageRefcount((*ppPage)->pDbPage)>1 ){
      releasePage(*ppPage);
      *ppPage = 0;
      أرجع SQLITE_CORRUPT_BKPT;
    }
    (*ppPage)->isInit = 0;
  }وإلا{
    *ppPage = 0;
  }
  أرجع rc;
}


/*
** During a rollback, when the pager reloads information into the cache
** so that the cache is restored to its original state at the start of
** the transaction, for each page restored this routine is called.
**
** This routine needs to reset the extra data section at the end of the
** page to agree with the restored data.
*/
ساكن فراغ pageReinit(DbPage *pData){
  MemPage *pPage;
  pPage = (MemPage *)sqlite3PagerGetExtra(pData);
  أكد( sqlite3PagerPageRefcount(pData)>0 );
  إذا( pPage->isInit ){
    أكد( sqlite3_mutex_held(pPage->pBt->mutex) );
    pPage->isInit = 0;
    إذا( sqlite3PagerPageRefcount(pData)>1 ){
      /* pPage might not be a btree page;  it might be an overflow page
      ** or ptrmap page or a free page.  In those cases, the following
      ** call to btreeInitPage() will likely return SQLITE_CORRUPT.
      ** But no harm is done by this.  And it is very important that
      ** btreeInitPage() be called on every btree page so we make
      ** the call for every page that comes in for re-initializing. */
      btreeInitPage(pPage);
    }
  }
}

/*
** Invoke the busy handler for a btree.
*/
ساكن صحيح btreeInvokeBusyHandler(فراغ *pArg){
  BtShared *pBt = (BtShared*)pArg;
  أكد( pBt->db );
  أكد( sqlite3_mutex_held(pBt->db->mutex) );
  أرجع sqlite3InvokeBusyHandler(&pBt->db->busyHandler);
}

/*
** Open a database file.
**
** zFilename is the name of the database file.  If zFilename is NULL
** then an ephemeral database is created.  The ephemeral database might
** be exclusively in memory, or it might use a disk-based memory cache.
** Either way, the ephemeral database will be automatically deleted
** when sqlite3BtreeClose() is called.
**
** If zFilename is ":memory:" then an in-memory database is created
** that is automatically destroyed when it is closed.
**
** The "flags" parameter is a bitmask that might contain bits like
** BTREE_OMIT_JOURNAL and/or BTREE_MEMORY.
**
** If the database is already opened in the same database connection
** and we are in shared cache mode, then the open will fail with an
** SQLITE_CONSTRAINT error.  We cannot allow two or more BtShared
** objects in the same database connection since doing so will lead
** to problems with locking.
*/
صحيح sqlite3BtreeOpen(
  sqlite3_vfs *pVfs,      /* VFS to use for this b-tree */
  ثابت حرف *zFilename,  /* Name of the file containing the BTree database */
  sqlite3 *db,            /* Associated database handle */
  Btree **ppBtree,        /* Pointer to new Btree object written here */
  صحيح flags,              /* Options */
  صحيح vfsFlags            /* Flags passed through to sqlite3_vfs.xOpen() */
){
  BtShared *pBt = 0;             /* Shared part of btree structure */
  Btree *p;                      /* Handle to return */
  sqlite3_mutex *mutexOpen = 0;  /* Prevents a race condition. Ticket #3537 */
  صحيح rc = SQLITE_OK;            /* Result code from this function */
  u8 nReserve;                   /* Byte of unused space on each page */
  unsigned حرف zDbHeader[100];  /* Database header content */

  /* True if opening an ephemeral, temporary database */
  ثابت صحيح isTempDb = zFilename==0 || zFilename[0]==0;

  /* Set the variable isMemdb to true for an in-memory database, or
  ** false for a file-based database.
  */
#ifdef SQLITE_OMIT_MEMORYDB
  ثابت صحيح isMemdb = 0;
#وإلا
  ثابت صحيح isMemdb = (zFilename && strcmp(zFilename, ":memory:")==0)
                       || (isTempDb && sqlite3TempInMemory(db))
                       || (vfsFlags & SQLITE_OPEN_MEMORY)!=0;
#endif

  أكد( db!=0 );
  أكد( pVfs!=0 );
  أكد( sqlite3_mutex_held(db->mutex) );
  أكد( (flags&0xff)==flags );   /* flags fit in 8 bits */

  /* Only a BTREE_SINGLE database can be BTREE_UNORDERED */
  أكد( (flags & BTREE_UNORDERED)==0 || (flags & BTREE_SINGLE)!=0 );

  /* A BTREE_SINGLE database is always a temporary and/or ephemeral */
  أكد( (flags & BTREE_SINGLE)==0 || isTempDb );

  إذا( isMemdb ){
    flags |= BTREE_MEMORY;
  }
  إذا( (vfsFlags & SQLITE_OPEN_MAIN_DB)!=0 && (isMemdb || isTempDb) ){
    vfsFlags = (vfsFlags & ~SQLITE_OPEN_MAIN_DB) | SQLITE_OPEN_TEMP_DB;
  }
  p = sqlite3MallocZero(حجم(Btree));
  إذا( !p ){
    أرجع SQLITE_NOMEM_BKPT;
  }
  p->inTrans = TRANS_NONE;
  p->db = db;
#ifndef SQLITE_OMIT_SHARED_CACHE
  p->lock.pBtree = p;
  p->lock.iTable = 1;
#endif

#إذا !defined(SQLITE_OMIT_SHARED_CACHE) && !defined(SQLITE_OMIT_DISKIO)
  /*
  ** If this Btree is a candidate for shared cache, try to find an
  ** existing BtShared object that we can share with
  */
  إذا( isTempDb==0 && (isMemdb==0 || (vfsFlags&SQLITE_OPEN_URI)!=0) ){
    إذا( vfsFlags & SQLITE_OPEN_SHAREDCACHE ){
      صحيح nFilename = sqlite3Strlen30(zFilename)+1;
      صحيح nFullPathname = pVfs->mxPathname+1;
      حرف *zFullPathname = sqlite3Malloc(MAX(nFullPathname,nFilename));
      MUTEX_LOGIC( sqlite3_mutex *mutexShared; )

      p->sharable = 1;
      إذا( !zFullPathname ){
        sqlite3_free(p);
        أرجع SQLITE_NOMEM_BKPT;
      }
      إذا( isMemdb ){
        memcpy(zFullPathname, zFilename, nFilename);
      }وإلا{
        rc = sqlite3OsFullPathname(pVfs, zFilename,
                                   nFullPathname, zFullPathname);
        إذا( rc ){
          إذا( rc==SQLITE_OK_SYMLINK ){
            rc = SQLITE_OK;
          }وإلا{
            sqlite3_free(zFullPathname);
            sqlite3_free(p);
            أرجع rc;
          }
        }
      }
#إذا SQLITE_THREADSAFE
      mutexOpen = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_OPEN);
      sqlite3_mutex_enter(mutexOpen);
      mutexShared = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
      sqlite3_mutex_enter(mutexShared);
#endif
      لكل(pBt=GLOBAL(BtShared*,sqlite3SharedCacheList); pBt; pBt=pBt->pNext){
        أكد( pBt->nRef>0 );
        إذا( 0==strcmp(zFullPathname, sqlite3PagerFilename(pBt->pPager, 0))
                 && sqlite3PagerVfs(pBt->pPager)==pVfs ){
          صحيح iDb;
          لكل(iDb=db->nDb-1; iDb>=0; iDb--){
            Btree *pExisting = db->aDb[iDb].pBt;
            إذا( pExisting && pExisting->pBt==pBt ){
              sqlite3_mutex_leave(mutexShared);
              sqlite3_mutex_leave(mutexOpen);
              sqlite3_free(zFullPathname);
              sqlite3_free(p);
              أرجع SQLITE_CONSTRAINT;
            }
          }
          p->pBt = pBt;
          pBt->nRef++;
          اكسر;
        }
      }
      sqlite3_mutex_leave(mutexShared);
      sqlite3_free(zFullPathname);
    }
#ifdef SQLITE_DEBUG
    وإلا{
      /* In debug mode, we mark all persistent databases as sharable
      ** even when they are not.  This exercises the locking code and
      ** gives more opportunity for asserts(sqlite3_mutex_held())
      ** statements to find locking problems.
      */
      p->sharable = 1;
    }
#endif
  }
#endif
  إذا( pBt==0 ){
    /*
    ** The following asserts make sure that structures used by the btree are
    ** the right size.  This is to guard against size changes that result
    ** when compiling on a different architecture.
    */
    أكد( حجم(i64)==8 );
    أكد( حجم(u64)==8 );
    أكد( حجم(u32)==4 );
    أكد( حجم(u16)==2 );
    أكد( حجم(Pgno)==4 );

    /* Suppress false-positive compiler warning from PVS-Studio */
    memset(&zDbHeader[16], 0, 8);
 
    pBt = sqlite3MallocZero( حجم(*pBt) );
    إذا( pBt==0 ){
      rc = SQLITE_NOMEM_BKPT;
      اذهب btree_open_out;
    }
    rc = sqlite3PagerOpen(pVfs, &pBt->pPager, zFilename,
                          حجم(MemPage), flags, vfsFlags, pageReinit);
    إذا( rc==SQLITE_OK ){
      sqlite3PagerSetMmapLimit(pBt->pPager, db->szMmap);
      rc = sqlite3PagerReadFileheader(pBt->pPager,حجم(zDbHeader),zDbHeader);
    }
    إذا( rc!=SQLITE_OK ){
      اذهب btree_open_out;
    }
    pBt->openFlags = (u8)flags;
    pBt->db = db;
    sqlite3PagerSetBusyHandler(pBt->pPager, btreeInvokeBusyHandler, pBt);
    p->pBt = pBt;
 
    pBt->pCursor = 0;
    pBt->pPage1 = 0;
    إذا( sqlite3PagerIsreadonly(pBt->pPager) ) pBt->btsFlags |= BTS_READ_ONLY;
#إذا defined(SQLITE_SECURE_DELETE)
    pBt->btsFlags |= BTS_SECURE_DELETE;
#وإلاإذا defined(SQLITE_FAST_SECURE_DELETE)
    pBt->btsFlags |= BTS_OVERWRITE;
#endif
    /* EVIDENCE-OF: R-51873-39618 The page size for a database file is
    ** determined by the 2-byte integer located at an offset of 16 bytes from
    ** the beginning of the database file. */
    pBt->pageSize = (zDbHeader[16]<<8) | (zDbHeader[17]<<16);
    إذا( pBt->pageSize<512 || pBt->pageSize>SQLITE_MAX_PAGE_SIZE
         || ((pBt->pageSize-1)&pBt->pageSize)!=0 ){
      pBt->pageSize = 0;
#ifndef SQLITE_OMIT_AUTOVACUUM
      /* If the magic name ":memory:" will create an in-memory database, then
      ** leave the autoVacuum mode at 0 (do not auto-vacuum), even if
      ** SQLITE_DEFAULT_AUTOVACUUM is true. On the other hand, if
      ** SQLITE_OMIT_MEMORYDB has been defined, then ":memory:" is just a
      ** regular file-name. In this case the auto-vacuum applies as per normal.
      */
      إذا( zFilename && !isMemdb ){
        pBt->autoVacuum = (SQLITE_DEFAULT_AUTOVACUUM ? 1 : 0);
        pBt->incrVacuum = (SQLITE_DEFAULT_AUTOVACUUM==2 ? 1 : 0);
      }
#endif
      nReserve = 0;
    }وإلا{
      /* EVIDENCE-OF: R-37497-42412 The size of the reserved region is
      ** determined by the one-byte unsigned integer found at an offset of 20
      ** into the database file header. */
      nReserve = zDbHeader[20];
      pBt->btsFlags |= BTS_PAGESIZE_FIXED;
#ifndef SQLITE_OMIT_AUTOVACUUM
      pBt->autoVacuum = (get4byte(&zDbHeader[36 + 4*4])?1:0);
      pBt->incrVacuum = (get4byte(&zDbHeader[36 + 7*4])?1:0);
#endif
    }
    rc = sqlite3PagerSetPagesize(pBt->pPager, &pBt->pageSize, nReserve);
    إذا( rc ) اذهب btree_open_out;
    pBt->usableSize = pBt->pageSize - nReserve;
    أكد( (pBt->pageSize & 7)==0 );  /* 8-byte alignment of pageSize */
  
#إذا !defined(SQLITE_OMIT_SHARED_CACHE) && !defined(SQLITE_OMIT_DISKIO)
    /* Add the new BtShared object to the linked list sharable BtShareds.
    */
    pBt->nRef = 1;
    إذا( p->sharable ){
      MUTEX_LOGIC( sqlite3_mutex *mutexShared; )
      MUTEX_LOGIC( mutexShared = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);)
      إذا( SQLITE_THREADSAFE && sqlite3GlobalConfig.bCoreMutex ){
        pBt->mutex = sqlite3MutexAlloc(SQLITE_MUTEX_FAST);
        إذا( pBt->mutex==0 ){
          rc = SQLITE_NOMEM_BKPT;
          اذهب btree_open_out;
        }
      }
      sqlite3_mutex_enter(mutexShared);
      pBt->pNext = GLOBAL(BtShared*,sqlite3SharedCacheList);
      GLOBAL(BtShared*,sqlite3SharedCacheList) = pBt;
      sqlite3_mutex_leave(mutexShared);
    }
#endif
  }

#إذا !defined(SQLITE_OMIT_SHARED_CACHE) && !defined(SQLITE_OMIT_DISKIO)
  /* If the new Btree uses a sharable pBtShared, then link the new
  ** Btree into the list of all sharable Btrees for the same connection.
  ** The list is kept in ascending order by pBt address.
  */
  إذا( p->sharable ){
    صحيح i;
    Btree *pSib;
    لكل(i=0; i<db->nDb; i++){
      إذا( (pSib = db->aDb[i].pBt)!=0 && pSib->sharable ){
        طالما( pSib->pPrev ){ pSib = pSib->pPrev; }
        إذا( (uptr)p->pBt<(uptr)pSib->pBt ){
          p->pNext = pSib;
          p->pPrev = 0;
          pSib->pPrev = p;
        }وإلا{
          طالما( pSib->pNext && (uptr)pSib->pNext->pBt<(uptr)p->pBt ){
            pSib = pSib->pNext;
          }
          p->pNext = pSib->pNext;
          p->pPrev = pSib;
          إذا( p->pNext ){
            p->pNext->pPrev = p;
          }
          pSib->pNext = p;
        }
        اكسر;
      }
    }
  }
#endif
  *ppBtree = p;

btree_open_out:
  إذا( rc!=SQLITE_OK ){
    إذا( pBt && pBt->pPager ){
      sqlite3PagerClose(pBt->pPager, 0);
    }
    sqlite3_free(pBt);
    sqlite3_free(p);
    *ppBtree = 0;
  }وإلا{
    sqlite3_file *pFile;

    /* If the B-Tree was successfully opened, set the pager-cache size to the
    ** default value. Except, when opening on an existing shared pager-cache,
    ** do not change the pager-cache size.
    */
    إذا( sqlite3BtreeSchema(p, 0, 0)==0 ){
      sqlite3BtreeSetCacheSize(p, SQLITE_DEFAULT_CACHE_SIZE);
    }

    pFile = sqlite3PagerFile(pBt->pPager);
    إذا( pFile->pMethods ){
      sqlite3OsFileControlHint(pFile, SQLITE_FCNTL_PDB, (فراغ*)&pBt->db);
    }
  }
  إذا( mutexOpen ){
    أكد( sqlite3_mutex_held(mutexOpen) );
    sqlite3_mutex_leave(mutexOpen);
  }
  أكد( rc!=SQLITE_OK || sqlite3_Btree_Connection_سانى(*ppBtree)>0 );
  أرجع rc;
}

/*
** Decrement the BtShared.nRef counter.  When it reaches zero,
** remove the BtShared structure from the sharing list.  Return
** true if the BtShared.nRef counter reaches zero and return
** false if it is still positive.
*/
ساكن صحيح removeFromSharingList(BtShared *pBt){
#ifndef SQLITE_OMIT_SHARED_CACHE
  MUTEX_LOGIC( sqlite3_mutex *pMainMtx; )
  BtShared *pList;
  صحيح removed = 0;

  أكد( sqlite3_mutex_notheld(pBt->mutex) );
  MUTEX_LOGIC( pMainMtx = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  sqlite3_mutex_enter(pMainMtx);
  pBt->nRef--;
  إذا( pBt->nRef<=0 ){
    إذا( GLOBAL(BtShared*,sqlite3SharedCacheList)==pBt ){
      GLOBAL(BtShared*,sqlite3SharedCacheList) = pBt->pNext;
    }وإلا{
      pList = GLOBAL(BtShared*,sqlite3SharedCacheList);
      طالما( ALWAYS(pList) && pList->pNext!=pBt ){
        pList=pList->pNext;
      }
      إذا( ALWAYS(pList) ){
        pList->pNext = pBt->pNext;
      }
    }
    إذا( SQLITE_THREADSAFE ){
      sqlite3_mutex_free(pBt->mutex);
    }
    removed = 1;
  }
  sqlite3_mutex_leave(pMainMtx);
  أرجع removed;
#وإلا
  UNUSED_PARAMETER( pBt );
  أرجع 1;
#endif
}

/*
** Make sure pBt->pTmpSpace points to an allocation of
** MX_CELL_SIZE(pBt) bytes with a 4-byte prefix for a left-child
** pointer.
*/
ساكن SQLITE_NOINLINE صحيح allocateTempSpace(BtShared *pBt){
  أكد( pBt!=0 );
  أكد( pBt->pTmpSpace==0 );
  /* This routine is called only by btreeCursor() when allocating the
  ** first write cursor for the BtShared object */
  أكد( pBt->pCursor!=0 && (pBt->pCursor->curFlags & BTCF_WriteFlag)!=0 );
  pBt->pTmpSpace = sqlite3PageMalloc( pBt->pageSize );
  إذا( pBt->pTmpSpace==0 ){
    BtCursor *pCur = pBt->pCursor;
    pBt->pCursor = pCur->pNext;  /* Unlink the cursor */
    memset(pCur, 0, حجم(*pCur));
    أرجع SQLITE_NOMEM_BKPT;
  }

  /* One of the uses of pBt->pTmpSpace is to format cells before
  ** inserting them into a leaf page (function fillInCell()). If
  ** a cell is less than 4 bytes in size, it is rounded up to 4 bytes
  ** by the various routines that manipulate binary cells. Which
  ** can mean that fillInCell() only initializes the first 2 or 3
  ** bytes of pTmpSpace, but that the first 4 bytes are copied from
  ** it into a database page. This is not actually a problem, but it
  ** does cause a valgrind error when the 1 or 2 bytes of uninitialized
  ** data is passed to system call write(). So to avoid this error,
  ** zero the first 4 bytes of temp space here.
  **
  ** Also:  Provide four bytes of initialized space before the
  ** beginning of pTmpSpace as an area available to prepend the
  ** left-child pointer to the beginning of a cell.
  */
  memset(pBt->pTmpSpace, 0, 8);
  pBt->pTmpSpace += 4;
  أرجع SQLITE_OK;
}

/*
** Free the pBt->pTmpSpace allocation
*/
ساكن فراغ freeTempSpace(BtShared *pBt){
  إذا( pBt->pTmpSpace ){
    pBt->pTmpSpace -= 4;
    sqlite3PageFree(pBt->pTmpSpace);
    pBt->pTmpSpace = 0;
  }
}

/*
** Close an open database and invalidate all cursors.
*/
صحيح sqlite3BtreeClose(Btree *p){
  BtShared *pBt = p->pBt;

  /* Close all cursors opened via this handle.  */
  أكد( sqlite3_mutex_held(p->db->mutex) );
  sqlite3BtreeEnter(p);

  /* Verify that no other cursors have this Btree open */
#ifdef SQLITE_DEBUG
  {
    BtCursor *pCur = pBt->pCursor;
    طالما( pCur ){
      BtCursor *pTmp = pCur;
      pCur = pCur->pNext;
      أكد( pTmp->pBtree!=p );

    }
  }
#endif

  /* Rollback any active transaction and free the handle structure.
  ** The call to sqlite3BtreeRollback() drops any table-locks held by
  ** this handle.
  */
  sqlite3BtreeRollback(p, SQLITE_OK, 0);
  sqlite3BtreeLeave(p);

  /* If there are still other outstanding references to the shared-btree
  ** structure, return now. The remainder of this procedure cleans
  ** up the shared-btree.
  */
  أكد( p->wantToLock==0 && p->locked==0 );
  إذا( !p->sharable || removeFromSharingList(pBt) ){
    /* The pBt is no longer on the sharing list, so we can access
    ** it without having to hold the mutex.
    **
    ** Clean out and delete the BtShared object.
    */
    أكد( !pBt->pCursor );
    sqlite3PagerClose(pBt->pPager, p->db);
    إذا( pBt->xFreeSchema && pBt->pSchema ){
      pBt->xFreeSchema(pBt->pSchema);
    }
    sqlite3DbFree(0, pBt->pSchema);
    freeTempSpace(pBt);
    sqlite3_free(pBt);
  }

#ifndef SQLITE_OMIT_SHARED_CACHE
  أكد( p->wantToLock==0 );
  أكد( p->locked==0 );
  إذا( p->pPrev ) p->pPrev->pNext = p->pNext;
  إذا( p->pNext ) p->pNext->pPrev = p->pPrev;
#endif

  sqlite3_free(p);
  أرجع SQLITE_OK;
}

/*
** Change the "soft" limit on the number of pages in the cache.
** Unused and unmodified pages will be recycled when the number of
** pages in the cache exceeds this soft limit.  But the size of the
** cache is allowed to grow larger than this limit if it contains
** dirty pages or pages still in active use.
*/
صحيح sqlite3BtreeSetCacheSize(Btree *p, صحيح mxPage){
  BtShared *pBt = p->pBt;
  أكد( sqlite3_mutex_held(p->db->mutex) );
  sqlite3BtreeEnter(p);
  sqlite3PagerSetCachesize(pBt->pPager, mxPage);
  sqlite3BtreeLeave(p);
  أرجع SQLITE_OK;
}

/*
** Change the "spill" limit on the number of pages in the cache.
** If the number of pages exceeds this limit during a write transaction,
** the pager might attempt to "spill" pages to the journal early in
** order to free up memory.
**
** The value returned is the current spill size.  If zero is passed
** as an argument, no changes are made to the spill size setting, so
** using mxPage of 0 is a way to query the current spill size.
*/
صحيح sqlite3BtreeSetSpillSize(Btree *p, صحيح mxPage){
  BtShared *pBt = p->pBt;
  صحيح res;
  أكد( sqlite3_mutex_held(p->db->mutex) );
  sqlite3BtreeEnter(p);
  res = sqlite3PagerSetSpillsize(pBt->pPager, mxPage);
  sqlite3BtreeLeave(p);
  أرجع res;
}

#إذا SQLITE_MAX_MMAP_SIZE>0
/*
** Change the limit on the amount of the database file that may be
** memory mapped.
*/
صحيح sqlite3BtreeSetMmapLimit(Btree *p, sqlite3_int64 szMmap){
  BtShared *pBt = p->pBt;
  أكد( sqlite3_mutex_held(p->db->mutex) );
  sqlite3BtreeEnter(p);
  sqlite3PagerSetMmapLimit(pBt->pPager, szMmap);
  sqlite3BtreeLeave(p);
  أرجع SQLITE_OK;
}
#endif /* SQLITE_MAX_MMAP_SIZE>0 */

/*
** Change the way data is synced to disk in order to increase or decrease
** how well the database resists damage due to OS crashes and power
** failures.  Level 1 is the same as asynchronous (no syncs() occur and
** there is a high probability of damage)  Level 2 is the default.  There
** is a very low but non-zero probability of damage.  Level 3 reduces the
** probability of damage to near zero but with a write performance reduction.
*/
#ifndef SQLITE_OMIT_PAGER_PRAGMAS
صحيح sqlite3BtreeSetPagerFlags(
  Btree *p,              /* The btree to set the safety level on */
  unsigned pgFlags       /* Various PAGER_* flags */
){
  BtShared *pBt = p->pBt;
  أكد( sqlite3_mutex_held(p->db->mutex) );
  sqlite3BtreeEnter(p);
  sqlite3PagerSetFlags(pBt->pPager, pgFlags);
  sqlite3BtreeLeave(p);
  أرجع SQLITE_OK;
}
#endif

/*
** Change the default pages size and the number of reserved bytes per page.
** Or, if the page size has already been fixed, return SQLITE_READONLY
** without changing anything.
**
** The page size must be a power of 2 between 512 and 65536.  If the page
** size supplied does not meet this constraint then the page size is not
** changed.
**
** Page sizes are constrained to be a power of two so that the region
** of the database file used for locking (beginning at PENDING_BYTE,
** the first byte past the 1GB boundary, 0x40000000) needs to occur
** at the beginning of a page.
**
** If parameter nReserve is less than zero, then the number of reserved
** bytes per page is left unchanged.
**
** If the iFix!=0 then the BTS_PAGESIZE_FIXED flag is set so that the page size
** and autovacuum mode can no longer be changed.
*/
صحيح sqlite3BtreeSetPageSize(Btree *p, صحيح pageSize, صحيح nReserve, صحيح iFix){
  صحيح rc = SQLITE_OK;
  صحيح x;
  BtShared *pBt = p->pBt;
  أكد( nReserve>=0 && nReserve<=255 );
  sqlite3BtreeEnter(p);
  pBt->nReserveWanted = (u8)nReserve;
  x = pBt->pageSize - pBt->usableSize;
  إذا( x==nReserve && (pageSize==0 || (u32)pageSize==pBt->pageSize) ){
    sqlite3BtreeLeave(p);
    أرجع SQLITE_OK;
  }
  إذا( nReserve<x ) nReserve = x;
  إذا( pBt->btsFlags & BTS_PAGESIZE_FIXED ){
    sqlite3BtreeLeave(p);
    أرجع SQLITE_READONLY;
  }
  أكد( nReserve>=0 && nReserve<=255 );
  إذا( pageSize>=512 && pageSize<=SQLITE_MAX_PAGE_SIZE &&
        ((pageSize-1)&pageSize)==0 ){
    أكد( (pageSize & 7)==0 );
    أكد( !pBt->pCursor );
    إذا( nReserve>32 && pageSize==512 ) pageSize = 1024;
    pBt->pageSize = (u32)pageSize;
    freeTempSpace(pBt);
  }
  rc = sqlite3PagerSetPagesize(pBt->pPager, &pBt->pageSize, nReserve);
  pBt->usableSize = pBt->pageSize - (u16)nReserve;
  إذا( iFix ) pBt->btsFlags |= BTS_PAGESIZE_FIXED;
  sqlite3BtreeLeave(p);
  أرجع rc;
}

/*
** Return the currently defined page size
*/
صحيح sqlite3BtreeGetPageSize(Btree *p){
  أرجع p->pBt->pageSize;
}

/*
** This function is similar to sqlite3BtreeGetReserve(), except that it
** may only be called if it is guaranteed that the b-tree mutex is already
** held.
**
** This is useful in one special case in the backup API code where it is
** known that the shared b-tree mutex is held, but the mutex on the
** database handle that owns *p is not. In this case if sqlite3BtreeEnter()
** were to be called, it might collide with some other operation on the
** database handle that owns *p, causing undefined behavior.
*/
صحيح sqlite3BtreeGetReserveNoMutex(Btree *p){
  صحيح n;
  أكد( sqlite3_mutex_held(p->pBt->mutex) );
  n = p->pBt->pageSize - p->pBt->usableSize;
  أرجع n;
}

/*
** Return the number of bytes of space at the end of every page that
** are intentionally left unused.  This is the "reserved" space that is
** sometimes used by extensions.
**
** The value returned is the larger of the current reserve size and
** the latest reserve size requested by SQLITE_FILECTRL_RESERVE_BYTES.
** The amount of reserve can only grow - never shrink.
*/
صحيح sqlite3BtreeGetRequestedReserve(Btree *p){
  صحيح n1, n2;
  sqlite3BtreeEnter(p);
  n1 = (صحيح)p->pBt->nReserveWanted;
  n2 = sqlite3BtreeGetReserveNoMutex(p);
  sqlite3BtreeLeave(p);
  أرجع n1>n2 ? n1 : n2;
}


/*
** Set the maximum page count for a database if mxPage is positive.
** No changes are made if mxPage is 0 or negative.
** Regardless of the value of mxPage, return the maximum page count.
*/
Pgno sqlite3_Btree_Max_Page_سانى(Btree *p, Pgno mxPage){
  Pgno n;
  sqlite3BtreeEnter(p);
  n = sqlite3_Pager_Max_Page_سانى(p->pBt->pPager, mxPage);
  sqlite3BtreeLeave(p);
  أرجع n;
}

/*
** Change the values for the BTS_SECURE_DELETE and BTS_OVERWRITE flags:
**
**    newFlag==0       Both BTS_SECURE_DELETE and BTS_OVERWRITE are cleared
**    newFlag==1       BTS_SECURE_DELETE set and BTS_OVERWRITE is cleared
**    newFlag==2       BTS_SECURE_DELETE cleared and BTS_OVERWRITE is set
**    newFlag==(-1)    No changes
**
** This routine acts as a query if newFlag is less than zero
**
** With BTS_OVERWRITE set, deleted content is overwritten by zeros, but
** freelist leaf pages are not written back to the database.  Thus in-page
** deleted content is cleared, but freelist deleted content is not.
**
** With BTS_SECURE_DELETE, operation is like BTS_OVERWRITE with the addition
** that freelist leaf pages are written back into the database, increasing
** the amount of disk I/O.
*/
صحيح sqlite3BtreeSecureDelete(Btree *p, صحيح newFlag){
  صحيح b;
  إذا( p==0 ) أرجع 0;
  sqlite3BtreeEnter(p);
  أكد( BTS_OVERWRITE==BTS_SECURE_DELETE*2 );
  أكد( BTS_FAST_SECURE==(BTS_OVERWRITE|BTS_SECURE_DELETE) );
  إذا( newFlag>=0 ){
    p->pBt->btsFlags &= ~BTS_FAST_SECURE;
    p->pBt->btsFlags |= (u16)(BTS_SECURE_DELETE*newFlag);
  }
  b = (p->pBt->btsFlags & BTS_FAST_SECURE)/BTS_SECURE_DELETE;
  sqlite3BtreeLeave(p);
  أرجع b;
}

/*
** Change the 'auto-vacuum' property of the database. If the 'autoVacuum'
** parameter is non-zero, then auto-vacuum mode is enabled. If zero, it
** is disabled. The default value for the auto-vacuum property is
** determined by the SQLITE_DEFAULT_AUTOVACUUM macro.
*/
صحيح sqlite3BtreeSetAutoVacuum(Btree *p, صحيح autoVacuum){
#ifdef SQLITE_OMIT_AUTOVACUUM
  أرجع SQLITE_READONLY;
#وإلا
  BtShared *pBt = p->pBt;
  صحيح rc = SQLITE_OK;
  u8 av = (u8)autoVacuum;

  sqlite3BtreeEnter(p);
  إذا( (pBt->btsFlags & BTS_PAGESIZE_FIXED)!=0 && (av ?1:0)!=pBt->autoVacuum ){
    rc = SQLITE_READONLY;
  }وإلا{
    pBt->autoVacuum = av ?1:0;
    pBt->incrVacuum = av==2 ?1:0;
  }
  sqlite3BtreeLeave(p);
  أرجع rc;
#endif
}

/*
** Return the value of the 'auto-vacuum' property. If auto-vacuum is
** enabled 1 is returned. Otherwise 0.
*/
صحيح sqlite3BtreeGetAutoVacuum(Btree *p){
#ifdef SQLITE_OMIT_AUTOVACUUM
  أرجع BTREE_AUTOVACUUM_NONE;
#وإلا
  صحيح rc;
  sqlite3BtreeEnter(p);
  rc = (
    (!p->pBt->autoVacuum)?BTREE_AUTOVACUUM_NONE:
    (!p->pBt->incrVacuum)?BTREE_AUTOVACUUM_FULL:
    BTREE_AUTOVACUUM_INCR
  );
  sqlite3BtreeLeave(p);
  أرجع rc;
#endif
}

/*
** If the user has not set the safety-level for this database connection
** using "PRAGMA synchronous", and if the safety-level is not already
** set to the value passed to this function as the second parameter,
** set it so.
*/
#إذا SQLITE_DEFAULT_SYNCHRONOUS!=SQLITE_DEFAULT_WAL_SYNCHRONOUS \
    && !defined(SQLITE_OMIT_WAL)
ساكن فراغ setDefaultSyncFlag(BtShared *pBt, u8 safety_level){
  sqlite3 *db;
  Db *pDb;
  إذا( (db=pBt->db)!=0 && (pDb=db->aDb)!=0 ){
    طالما( pDb->pBt==0 || pDb->pBt->pBt!=pBt ){ pDb++; }
    إذا( pDb->bSyncSet==0
     && pDb->safety_level!=safety_level
     && pDb!=&db->aDb[1]
    ){
      pDb->safety_level = safety_level;
      sqlite3PagerSetFlags(pBt->pPager,
          pDb->safety_level | (db->flags & PAGER_FLAGS_MASK));
    }
  }
}
#وإلا
# define setDefaultSyncFlag(pBt,safety_level)
#endif

/* Forward declaration */
ساكن صحيح newDatabase(BtShared*);


/*
** Get a reference to pPage1 of the database file.  This will
** also acquire a readlock on that file.
**
** SQLITE_OK is returned on success.  If the file is not a
** well-formed database file, then SQLITE_CORRUPT is returned.
** SQLITE_BUSY is returned if the database is locked.  SQLITE_NOMEM
** is returned if we run out of memory.
*/
ساكن صحيح lockBtree(BtShared *pBt){
  صحيح rc;              /* Result code from subfunctions */
  MemPage *pPage1;     /* Page 1 of the database file */
  u32 nPage;           /* Number of pages in the database */
  u32 nPageFile = 0;   /* Number of pages in the database file */

  أكد( sqlite3_mutex_held(pBt->mutex) );
  أكد( pBt->pPage1==0 );
  rc = sqlite3PagerSharedLock(pBt->pPager);
  إذا( rc!=SQLITE_OK ) أرجع rc;
  rc = btreeGetPage(pBt, 1, &pPage1, 0);
  إذا( rc!=SQLITE_OK ) أرجع rc;

  /* Do some checking to help insure the file we opened really is
  ** a valid database file.
  */
  nPage = get4byte(28+(u8*)pPage1->aData);
  sqlite3PagerPagecount(pBt->pPager, (صحيح*)&nPageFile);
  إذا( nPage==0 || memcmp(24+(u8*)pPage1->aData, 92+(u8*)pPage1->aData,4)!=0 ){
    nPage = nPageFile;
  }
  إذا( (pBt->db->flags & SQLITE_ResetDatabase)!=0 ){
    nPage = 0;
  }
  إذا( nPage>0 ){
    u32 pageSize;
    u32 usableSize;
    u8 *page1 = pPage1->aData;
    rc = SQLITE_NOTADB;
    /* EVIDENCE-OF: R-43737-39999 Every valid SQLite database file begins
    ** with the following 16 bytes (in hex): 53 51 4c 69 74 65 20 66 6f 72 6d
    ** 61 74 20 33 00. */
    إذا( memcmp(page1, zMagicHeader, 16)!=0 ){
      اذهب page1_init_failed;
    }

#ifdef SQLITE_OMIT_WAL
    إذا( page1[18]>1 ){
      pBt->btsFlags |= BTS_READ_ONLY;
    }
    إذا( page1[19]>1 ){
      اذهب page1_init_failed;
    }
#وإلا
    إذا( page1[18]>2 ){
      pBt->btsFlags |= BTS_READ_ONLY;
    }
    إذا( page1[19]>2 ){
      اذهب page1_init_failed;
    }

    /* If the read version is set to 2, this database should be accessed
    ** in WAL mode. If the log is not already open, open it now. Then
    ** return SQLITE_OK and return without populating BtShared.pPage1.
    ** The caller detects this and calls this function again. This is
    ** required as the version of page 1 currently in the page1 buffer
    ** may not be the latest version - there may be a newer one in the log
    ** file.
    */
    إذا( page1[19]==2 && (pBt->btsFlags & BTS_NO_WAL)==0 ){
      صحيح isOpen = 0;
      rc = sqlite3PagerOpenWal(pBt->pPager, &isOpen);
      إذا( rc!=SQLITE_OK ){
        اذهب page1_init_failed;
      }وإلا{
        setDefaultSyncFlag(pBt, SQLITE_DEFAULT_WAL_SYNCHRONOUS+1);
        إذا( isOpen==0 ){
          releasePageOne(pPage1);
          أرجع SQLITE_OK;
        }
      }
      rc = SQLITE_NOTADB;
    }وإلا{
      setDefaultSyncFlag(pBt, SQLITE_DEFAULT_SYNCHRONOUS+1);
    }
#endif

    /* EVIDENCE-OF: R-15465-20813 The maximum and minimum embedded payload
    ** fractions and the leaf payload fraction values must be 64, 32, and 32.
    **
    ** The original design allowed these amounts to vary, but as of
    ** version 3.6.0, we require them to be fixed.
    */
    إذا( memcmp(&page1[21], "\100\040\040",3)!=0 ){
      اذهب page1_init_failed;
    }
    /* EVIDENCE-OF: R-51873-39618 The page size for a database file is
    ** determined by the 2-byte integer located at an offset of 16 bytes from
    ** the beginning of the database file. */
    pageSize = (page1[16]<<8) | (page1[17]<<16);
    /* EVIDENCE-OF: R-25008-21688 The size of a page is a power of two
    ** between 512 and 65536 inclusive. */
    إذا( ((pageSize-1)&pageSize)!=0
     || pageSize>SQLITE_MAX_PAGE_SIZE
     || pageSize<=256
    ){
      اذهب page1_init_failed;
    }
    أكد( (pageSize & 7)==0 );
    /* EVIDENCE-OF: R-59310-51205 The "reserved space" size in the 1-byte
    ** integer at offset 20 is the number of bytes of space at the end of
    ** each page to reserve for extensions.
    **
    ** EVIDENCE-OF: R-37497-42412 The size of the reserved region is
    ** determined by the one-byte unsigned integer found at an offset of 20
    ** into the database file header. */
    usableSize = pageSize - page1[20];
    إذا( (u32)pageSize!=pBt->pageSize ){
      /* After reading the first page of the database assuming a page size
      ** of BtShared.pageSize, we have discovered that the page-size is
      ** actually pageSize. Unlock the database, leave pBt->pPage1 at
      ** zero and return SQLITE_OK. The caller will call this function
      ** again with the correct page-size.
      */
      releasePageOne(pPage1);
      pBt->usableSize = usableSize;
      pBt->pageSize = pageSize;
      pBt->btsFlags |= BTS_PAGESIZE_FIXED;
      freeTempSpace(pBt);
      rc = sqlite3PagerSetPagesize(pBt->pPager, &pBt->pageSize,
                                   pageSize-usableSize);
      أرجع rc;
    }
    إذا( nPage>nPageFile ){
      إذا( sqlite3WritableSchema(pBt->db)==0 ){
        rc = SQLITE_CORRUPT_BKPT;
        اذهب page1_init_failed;
      }وإلا{
        nPage = nPageFile;
      }
    }
    /* EVIDENCE-OF: R-28312-64704 However, the usable size is not allowed to
    ** be less than 480. In other words, if the page size is 512, then the
    ** reserved space size cannot exceed 32. */
    إذا( usableSize<480 ){
      اذهب page1_init_failed;
    }
    pBt->btsFlags |= BTS_PAGESIZE_FIXED;
    pBt->pageSize = pageSize;
    pBt->usableSize = usableSize;
#ifndef SQLITE_OMIT_AUTOVACUUM
    pBt->autoVacuum = (get4byte(&page1[36 + 4*4])?1:0);
    pBt->incrVacuum = (get4byte(&page1[36 + 7*4])?1:0);
#endif
  }

  /* maxLocal is the maximum amount of payload to store locally for
  ** a cell.  Make sure it is small enough so that at least minFanout
  ** cells can will fit on one page.  We assume a 10-byte page header.
  ** Besides the payload, the cell must store:
  **     2-byte pointer to the cell
  **     4-byte child pointer
  **     9-byte nKey value
  **     4-byte nData value
  **     4-byte overflow page pointer
  ** So a cell consists of a 2-byte pointer, a header which is as much as
  ** 17 bytes long, 0 to N bytes of payload, and an optional 4 byte overflow
  ** page pointer.
  */
  pBt->maxLocal = (u16)((pBt->usableSize-12)*64/255 - 23);
  pBt->minLocal = (u16)((pBt->usableSize-12)*32/255 - 23);
  pBt->maxLeaf = (u16)(pBt->usableSize - 35);
  pBt->minLeaf = (u16)((pBt->usableSize-12)*32/255 - 23);
  إذا( pBt->maxLocal>127 ){
    pBt->max1bytePayload = 127;
  }وإلا{
    pBt->max1bytePayload = (u8)pBt->maxLocal;
  }
  أكد( pBt->maxLeaf + 23 <= MX_CELL_SIZE(pBt) );
  pBt->pPage1 = pPage1;
  pBt->nPage = nPage;
  أرجع SQLITE_OK;

page1_init_failed:
  releasePageOne(pPage1);
  pBt->pPage1 = 0;
  أرجع rc;
}

#ifndef NDEBUG
/*
** Return the number of cursors open on pBt. This is for use
** in assert() expressions, so it is only compiled if NDEBUG is not
** defined.
**
** Only write cursors are counted if wrOnly is true.  If wrOnly is
** false then all cursors are counted.
**
** For the purposes of this routine, a cursor is any cursor that
** is capable of reading or writing to the database.  Cursors that
** have been tripped into the CURSOR_FAULT state are not counted.
*/
ساكن صحيح سانى_Valid_Cursors(BtShared *pBt, صحيح wrOnly){
  BtCursor *pCur;
  صحيح r = 0;
  لكل(pCur=pBt->pCursor; pCur; pCur=pCur->pNext){
    إذا( (wrOnly==0 || (pCur->curFlags & BTCF_WriteFlag)!=0)
     && pCur->eState!=CURSOR_FAULT ) r++;
  }
  أرجع r;
}
#endif

/*
** If there are no outstanding cursors and we are not in the middle
** of a transaction but there is a read lock on the database, then
** this routine unrefs the first page of the database file which
** has the effect of releasing the read lock.
**
** If there is a transaction in progress, this routine is a no-op.
*/
ساكن فراغ unlockBtreeIfUnused(BtShared *pBt){
  أكد( sqlite3_mutex_held(pBt->mutex) );
  أكد( سانى_Valid_Cursors(pBt,0)==0 || pBt->inTransaction>TRANS_NONE );
  إذا( pBt->inTransaction==TRANS_NONE && pBt->pPage1!=0 ){
    MemPage *pPage1 = pBt->pPage1;
    أكد( pPage1->aData );
    أكد( sqlite3PagerRefcount(pBt->pPager)==1 );
    pBt->pPage1 = 0;
    releasePageOne(pPage1);
  }
}

/*
** If pBt points to an empty file then convert that empty file
** into a new empty database by initializing the first page of
** the database.
*/
ساكن صحيح newDatabase(BtShared *pBt){
  MemPage *pP1;
  unsigned حرف *data;
  صحيح rc;

  أكد( sqlite3_mutex_held(pBt->mutex) );
  إذا( pBt->nPage>0 ){
    أرجع SQLITE_OK;
  }
  pP1 = pBt->pPage1;
  أكد( pP1!=0 );
  data = pP1->aData;
  rc = sqlite3PagerWrite(pP1->pDbPage);
  إذا( rc ) أرجع rc;
  memcpy(data, zMagicHeader, حجم(zMagicHeader));
  أكد( حجم(zMagicHeader)==16 );
  data[16] = (u8)((pBt->pageSize>>8)&0xff);
  data[17] = (u8)((pBt->pageSize>>16)&0xff);
  data[18] = 1;
  data[19] = 1;
  أكد( pBt->usableSize<=pBt->pageSize && pBt->usableSize+255>=pBt->pageSize);
  data[20] = (u8)(pBt->pageSize - pBt->usableSize);
  data[21] = 64;
  data[22] = 32;
  data[23] = 32;
  memset(&data[24], 0, 100-24);
  zeroPage(pP1, PTF_INTKEY|PTF_LEAF|PTF_LEAFDATA );
  pBt->btsFlags |= BTS_PAGESIZE_FIXED;
#ifndef SQLITE_OMIT_AUTOVACUUM
  أكد( pBt->autoVacuum==1 || pBt->autoVacuum==0 );
  أكد( pBt->incrVacuum==1 || pBt->incrVacuum==0 );
  put4byte(&data[36 + 4*4], pBt->autoVacuum);
  put4byte(&data[36 + 7*4], pBt->incrVacuum);
#endif
  pBt->nPage = 1;
  data[31] = 1;
  أرجع SQLITE_OK;
}

/*
** Initialize the first page of the database file (creating a database
** consisting of a single page and no schema objects). Return SQLITE_OK
** if successful, or an SQLite error code otherwise.
*/
صحيح sqlite3BtreeNewDb(Btree *p){
  صحيح rc;
  sqlite3BtreeEnter(p);
  p->pBt->nPage = 0;
  rc = newDatabase(p->pBt);
  sqlite3BtreeLeave(p);
  أرجع rc;
}

/*
** Attempt to start a new transaction. A write-transaction
** is started if the second argument is nonzero, otherwise a read-
** transaction.  If the second argument is 2 or more and exclusive
** transaction is started, meaning that no other process is allowed
** to access the database.  A preexisting transaction may not be
** upgraded to exclusive by calling this routine a second time - the
** exclusivity flag only works for a new transaction.
**
** A write-transaction must be started before attempting any
** changes to the database.  None of the following routines
** will work unless a transaction is started first:
**
**      sqlite3BtreeCreateTable()
**      sqlite3BtreeCreateIndex()
**      sqlite3BtreeClearTable()
**      sqlite3BtreeDropTable()
**      sqlite3BtreeInsert()
**      sqlite3BtreeDelete()
**      sqlite3BtreeUpdateMeta()
**
** If an initial attempt to acquire the lock fails because of lock contention
** and the database was previously unlocked, then invoke the busy handler
** if there is one.  But if there was previously a read-lock, do not
** invoke the busy handler - just return SQLITE_BUSY.  SQLITE_BUSY is
** returned when there is already a read-lock in order to avoid a deadlock.
**
** Suppose there are two processes A and B.  A has a read lock and B has
** a reserved lock.  B tries to promote to exclusive but is blocked because
** of A's read lock.  A tries to promote to reserved but is blocked by B.
** One or the other of the two processes must give way or there can be
** no progress.  By returning SQLITE_BUSY and not invoking the busy callback
** when A already has a read lock, we encourage A to give up and let B
** proceed.
*/
ساكن SQLITE_NOINLINE صحيح btreeBeginTrans(
  Btree *p,                 /* The btree in which to start the transaction */
  صحيح wrflag,               /* True to start a write transaction */
  صحيح *pSchemaVersion       /* Put schema version number here, if not NULL */
){
  BtShared *pBt = p->pBt;
  Pager *pPager = pBt->pPager;
  صحيح rc = SQLITE_OK;

  sqlite3BtreeEnter(p);
  btreeIntegrity(p);

  /* If the btree is already in a write-transaction, or it
  ** is already in a read-transaction and a read-transaction
  ** is requested, this is a no-op.
  */
  إذا( p->inTrans==TRANS_WRITE || (p->inTrans==TRANS_READ && !wrflag) ){
    اذهب trans_begun;
  }
  أكد( pBt->inTransaction==TRANS_WRITE || IfNotOmitAV(pBt->bDoTruncate)==0 );

  إذا( (p->db->flags & SQLITE_ResetDatabase)
   && sqlite3PagerIsreadonly(pPager)==0
  ){
    pBt->btsFlags &= ~BTS_READ_ONLY;
  }

  /* Write transactions are not possible on a read-only database */
  إذا( (pBt->btsFlags & BTS_READ_ONLY)!=0 && wrflag ){
    rc = SQLITE_READONLY;
    اذهب trans_begun;
  }

#ifndef SQLITE_OMIT_SHARED_CACHE
  {
    sqlite3 *pBlock = 0;
    /* If another database handle has already opened a write transaction
    ** on this shared-btree structure and a second write transaction is
    ** requested, return SQLITE_LOCKED.
    */
    إذا( (wrflag && pBt->inTransaction==TRANS_WRITE)
     || (pBt->btsFlags & BTS_PENDING)!=0
    ){
      pBlock = pBt->pWriter->db;
    }وإلا إذا( wrflag>1 ){
      BtLock *pIter;
      لكل(pIter=pBt->pLock; pIter; pIter=pIter->pNext){
        إذا( pIter->pBtree!=p ){
          pBlock = pIter->pBtree->db;
          اكسر;
        }
      }
    }
    إذا( pBlock ){
      sqlite3ConnectionBlocked(p->db, pBlock);
      rc = SQLITE_LOCKED_SHAREDCACHE;
      اذهب trans_begun;
    }
  }
#endif

  /* Any read-only or read-write transaction implies a read-lock on
  ** page 1. So if some other shared-cache client already has a write-lock
  ** on page 1, the transaction cannot be opened. */
  rc = querySharedCacheTableLock(p, SCHEMA_يىلتىز, READ_LOCK);
  إذا( SQLITE_OK!=rc ) اذهب trans_begun;

  pBt->btsFlags &= ~BTS_INITIALLY_EMPTY;
  إذا( pBt->nPage==0 ) pBt->btsFlags |= BTS_INITIALLY_EMPTY;
  افعل {
    sqlite3PagerWalDb(pPager, p->db);

#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
    /* If transitioning from no transaction directly to a write transaction,
    ** block for the WRITER lock first if possible. */
    إذا( pBt->pPage1==0 && wrflag ){
      أكد( pBt->inTransaction==TRANS_NONE );
      rc = sqlite3PagerWalWriteLock(pPager, 1);
      إذا( rc!=SQLITE_BUSY && rc!=SQLITE_OK ) اكسر;
    }
#endif

#ifdef SQLITE_EXPERIMENTAL_PRAGMA_20251114
    /* If both a read and write transaction will be opened by this call,
    ** then issue a file-control as if the following pragma command had
    ** been evaluated:
    **
    **     PRAGMA experimental_pragma_20251114 = 1|2
    **
    ** where the RHS is "1" if wrflag is 1 (RESERVED lock), or "2" if wrflag 
    ** is 2 (EXCLUSIVE lock). Ignore any result or error returned by the VFS.
    **
    ** WARNING: This code will likely remain part of SQLite only temporarily -
    ** it exists to allow users to experiment with certain types of blocking 
    ** locks in custom VFS implementations. It MAY BE REMOVED AT ANY TIME.  */
    إذا( pBt->pPage1==0 && wrflag ){
      sqlite3_file *fd = sqlite3PagerFile(pPager);
      حرف *aFcntl[3] = {0,0,0};
      aFcntl[1] = "experimental_pragma_20251114";
      أكد( wrflag==1 || wrflag==2 );
      aFcntl[2] = (wrflag==1 ? "1" : "2");
      sqlite3OsFileControlHint(fd, SQLITE_FCNTL_PRAGMA, (فراغ*)aFcntl);
      sqlite3_free(aFcntl[0]);
    }
#endif

    /* Call lockBtree() until either pBt->pPage1 is populated or
    ** lockBtree() returns something other than SQLITE_OK. lockBtree()
    ** may return SQLITE_OK but leave pBt->pPage1 set to 0 if after
    ** reading page 1 it discovers that the page-size of the database
    ** file is not pBt->pageSize. In this case lockBtree() will update
    ** pBt->pageSize to the page-size of the file on disk.
    */
    طالما( pBt->pPage1==0 && SQLITE_OK==(rc = lockBtree(pBt)) );

    إذا( rc==SQLITE_OK && wrflag ){
      إذا( (pBt->btsFlags & BTS_READ_ONLY)!=0 ){
        rc = SQLITE_READONLY;
      }وإلا{
        rc = sqlite3PagerBegin(pPager, wrflag>1, sqlite3TempInMemory(p->db));
        إذا( rc==SQLITE_OK ){
          rc = newDatabase(pBt);
        }وإلا إذا( rc==SQLITE_BUSY_SNAPSHOT && pBt->inTransaction==TRANS_NONE ){
          /* if there was no transaction opened when this function was
          ** called and SQLITE_BUSY_SNAPSHOT is returned, change the error
          ** code to SQLITE_BUSY. */
          rc = SQLITE_BUSY;
        }
      }
    }
 
    إذا( rc!=SQLITE_OK ){
      (فراغ)sqlite3PagerWalWriteLock(pPager, 0);
      unlockBtreeIfUnused(pBt);
    }
#إذا defined(SQLITE_ENABLE_SETLK_TIMEOUT)
    إذا( rc==SQLITE_BUSY_TIMEOUT ){
      /* If a blocking lock timed out, break out of the loop here so that
      ** the busy-handler is not invoked.  */
      اكسر;
    }
#endif
  }طالما( (rc&0xFF)==SQLITE_BUSY && pBt->inTransaction==TRANS_NONE &&
          btreeInvokeBusyHandler(pBt) );
  sqlite3PagerWalDb(pPager, 0);
#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
  إذا( rc==SQLITE_BUSY_TIMEOUT ) rc = SQLITE_BUSY;
#endif

  إذا( rc==SQLITE_OK ){
    إذا( p->inTrans==TRANS_NONE ){
      pBt->nTransaction++;
#ifndef SQLITE_OMIT_SHARED_CACHE
      إذا( p->sharable ){
        أكد( p->lock.pBtree==p && p->lock.iTable==1 );
        p->lock.eLock = READ_LOCK;
        p->lock.pNext = pBt->pLock;
        pBt->pLock = &p->lock;
      }
#endif
    }
    p->inTrans = (wrflag?TRANS_WRITE:TRANS_READ);
    إذا( p->inTrans>pBt->inTransaction ){
      pBt->inTransaction = p->inTrans;
    }
    إذا( wrflag ){
      MemPage *pPage1 = pBt->pPage1;
#ifndef SQLITE_OMIT_SHARED_CACHE
      أكد( !pBt->pWriter );
      pBt->pWriter = p;
      pBt->btsFlags &= ~BTS_EXCLUSIVE;
      إذا( wrflag>1 ) pBt->btsFlags |= BTS_EXCLUSIVE;
#endif

      /* If the db-size header field is incorrect (as it may be if an old
      ** client has been writing the database file), update it now. Doing
      ** this sooner rather than later means the database size can safely
      ** re-read the database size from page 1 if a savepoint or transaction
      ** rollback occurs within the transaction.
      */
      إذا( pBt->nPage!=get4byte(&pPage1->aData[28]) ){
        rc = sqlite3PagerWrite(pPage1->pDbPage);
        إذا( rc==SQLITE_OK ){
          put4byte(&pPage1->aData[28], pBt->nPage);
        }
      }
    }
  }

trans_begun:
  إذا( rc==SQLITE_OK ){
    إذا( pSchemaVersion ){
      *pSchemaVersion = get4byte(&pBt->pPage1->aData[40]);
    }
    إذا( wrflag ){
      /* This call makes sure that the pager has the correct number of
      ** open savepoints. If the second parameter is greater than 0 and
      ** the sub-journal is not already open, then it will be opened here.
      */
      rc = sqlite3PagerOpenSavepoint(pPager, p->db->nSavepoint);
    }
  }

  btreeIntegrity(p);
  sqlite3BtreeLeave(p);
  أرجع rc;
}
صحيح sqlite3BtreeBeginTrans(Btree *p, صحيح wrflag, صحيح *pSchemaVersion){
  BtShared *pBt;
  إذا( p->sharable
   || p->inTrans==TRANS_NONE
   || (p->inTrans==TRANS_READ && wrflag!=0)
  ){
    أرجع btreeBeginTrans(p,wrflag,pSchemaVersion);
  }
  pBt = p->pBt;
  إذا( pSchemaVersion ){
    *pSchemaVersion = get4byte(&pBt->pPage1->aData[40]);
  }
  إذا( wrflag ){
    /* This call makes sure that the pager has the correct number of
    ** open savepoints. If the second parameter is greater than 0 and
    ** the sub-journal is not already open, then it will be opened here.
    */
    أرجع sqlite3PagerOpenSavepoint(pBt->pPager, p->db->nSavepoint);
  }وإلا{
    أرجع SQLITE_OK;
  }
}

#ifndef SQLITE_OMIT_AUTOVACUUM

/*
** Set the pointer-map entries for all children of page pPage. Also, if
** pPage contains cells that point to overflow pages, set the pointer
** map entries for the overflow pages as well.
*/
ساكن صحيح setChildPtrmaps(MemPage *pPage){
  صحيح i;                             /* Counter variable */
  صحيح nCell;                         /* Number of cells in page pPage */
  صحيح rc;                            /* Return code */
  BtShared *pBt = pPage->pBt;
  Pgno pgno = pPage->pgno;

  أكد( sqlite3_mutex_held(pPage->pBt->mutex) );
  rc = pPage->isInit ? SQLITE_OK : btreeInitPage(pPage);
  إذا( rc!=SQLITE_OK ) أرجع rc;
  nCell = pPage->nCell;

  لكل(i=0; i<nCell; i++){
    u8 *pCell = findCell(pPage, i);

    ptrmapPutOvflPtr(pPage, pPage, pCell, &rc);

    إذا( !pPage->leaf ){
      Pgno childPgno = get4byte(pCell);
      ptrmapPut(pBt, childPgno, PTRMAP_BTREE, pgno, &rc);
    }
  }

  إذا( !pPage->leaf ){
    Pgno childPgno = get4byte(&pPage->aData[pPage->hdrOffset+8]);
    ptrmapPut(pBt, childPgno, PTRMAP_BTREE, pgno, &rc);
  }

  أرجع rc;
}

/*
** Somewhere on pPage is a pointer to page iFrom.  Modify this pointer so
** that it points to iTo. Parameter eType describes the type of pointer to
** be modified, as  follows:
**
** PTRMAP_BTREE:     pPage is a btree-page. The pointer points at a child
**                   page of pPage.
**
** PTRMAP_OVERFLOW1: pPage is a btree-page. The pointer points at an overflow
**                   page pointed to by one of the cells on pPage.
**
** PTRMAP_OVERFLOW2: pPage is an overflow-page. The pointer points at the next
**                   overflow page in the list.
*/
ساكن صحيح modifyPagePointer(MemPage *pPage, Pgno iFrom, Pgno iTo, u8 eType){
  أكد( sqlite3_mutex_held(pPage->pBt->mutex) );
  أكد( sqlite3PagerIswriteable(pPage->pDbPage) );
  إذا( eType==PTRMAP_OVERFLOW2 ){
    /* The pointer is always the first 4 bytes of the page in this case.  */
    إذا( get4byte(pPage->aData)!=iFrom ){
      أرجع SQLITE_CORRUPT_PAGE(pPage);
    }
    put4byte(pPage->aData, iTo);
  }وإلا{
    صحيح i;
    صحيح nCell;
    صحيح rc;

    rc = pPage->isInit ? SQLITE_OK : btreeInitPage(pPage);
    إذا( rc ) أرجع rc;
    nCell = pPage->nCell;

    لكل(i=0; i<nCell; i++){
      u8 *pCell = findCell(pPage, i);
      إذا( eType==PTRMAP_OVERFLOW1 ){
        CellInfo info;
        pPage->xParseCell(pPage, pCell, &info);
        إذا( info.nLocal<info.nPayload ){
          إذا( pCell+info.nSize > pPage->aData+pPage->pBt->usableSize ){
            أرجع SQLITE_CORRUPT_PAGE(pPage);
          }
          إذا( iFrom==get4byte(pCell+info.nSize-4) ){
            put4byte(pCell+info.nSize-4, iTo);
            اكسر;
          }
        }
      }وإلا{
        إذا( pCell+4 > pPage->aData+pPage->pBt->usableSize ){
          أرجع SQLITE_CORRUPT_PAGE(pPage);
        }
        إذا( get4byte(pCell)==iFrom ){
          put4byte(pCell, iTo);
          اكسر;
        }
      }
    }
 
    إذا( i==nCell ){
      إذا( eType!=PTRMAP_BTREE ||
          get4byte(&pPage->aData[pPage->hdrOffset+8])!=iFrom ){
        أرجع SQLITE_CORRUPT_PAGE(pPage);
      }
      put4byte(&pPage->aData[pPage->hdrOffset+8], iTo);
    }
  }
  أرجع SQLITE_OK;
}


/*
** Move the open database page pDbPage to location iFreePage in the
** database. The pDbPage reference remains valid.
**
** The isCommit flag indicates that there is no need to remember that
** the journal needs to be sync()ed before database page pDbPage->pgno
** can be written to. The caller has already promised not to write to that
** page.
*/
ساكن صحيح relocatePage(
  BtShared *pBt,           /* Btree */
  MemPage *pDbPage,        /* Open page to move */
  u8 eType,                /* Pointer map 'type' entry for pDbPage */
  Pgno iPtrPage,           /* Pointer map 'page-no' entry for pDbPage */
  Pgno iFreePage,          /* The location to move pDbPage to */
  صحيح isCommit             /* isCommit flag passed to sqlite3PagerMovepage */
){
  MemPage *pPtrPage;   /* The page that contains a pointer to pDbPage */
  Pgno iDbPage = pDbPage->pgno;
  Pager *pPager = pBt->pPager;
  صحيح rc;

  أكد( eType==PTRMAP_OVERFLOW2 || eType==PTRMAP_OVERFLOW1 ||
      eType==PTRMAP_BTREE || eType==PTRMAP_ROOTPAGE );
  أكد( sqlite3_mutex_held(pBt->mutex) );
  أكد( pDbPage->pBt==pBt );
  إذا( iDbPage<3 ) أرجع SQLITE_CORRUPT_BKPT;

  /* Move page iDbPage from its current location to page number iFreePage */
  TRACE(("AUTOVACUUM: Moving %u to free page %u (ptr page %u type %u)\n",
      iDbPage, iFreePage, iPtrPage, eType));
  rc = sqlite3PagerMovepage(pPager, pDbPage->pDbPage, iFreePage, isCommit);
  إذا( rc!=SQLITE_OK ){
    أرجع rc;
  }
  pDbPage->pgno = iFreePage;

  /* If pDbPage was a btree-page, then it may have child pages and/or cells
  ** that point to overflow pages. The pointer map entries for all these
  ** pages need to be changed.
  **
  ** If pDbPage is an overflow page, then the first 4 bytes may store a
  ** pointer to a subsequent overflow page. If this is the case, then
  ** the pointer map needs to be updated for the subsequent overflow page.
  */
  إذا( eType==PTRMAP_BTREE || eType==PTRMAP_ROOTPAGE ){
    rc = setChildPtrmaps(pDbPage);
    إذا( rc!=SQLITE_OK ){
      أرجع rc;
    }
  }وإلا{
    Pgno nextOvfl = get4byte(pDbPage->aData);
    إذا( nextOvfl!=0 ){
      ptrmapPut(pBt, nextOvfl, PTRMAP_OVERFLOW2, iFreePage, &rc);
      إذا( rc!=SQLITE_OK ){
        أرجع rc;
      }
    }
  }

  /* Fix the database pointer on page iPtrPage that pointed at iDbPage so
  ** that it points at iFreePage. Also fix the pointer map entry for
  ** iPtrPage.
  */
  إذا( eType!=PTRMAP_ROOTPAGE ){
    rc = btreeGetPage(pBt, iPtrPage, &pPtrPage, 0);
    إذا( rc!=SQLITE_OK ){
      أرجع rc;
    }
    rc = sqlite3PagerWrite(pPtrPage->pDbPage);
    إذا( rc!=SQLITE_OK ){
      releasePage(pPtrPage);
      أرجع rc;
    }
    rc = modifyPagePointer(pPtrPage, iDbPage, iFreePage, eType);
    releasePage(pPtrPage);
    إذا( rc==SQLITE_OK ){
      ptrmapPut(pBt, iFreePage, eType, iPtrPage, &rc);
    }
  }
  أرجع rc;
}

/* Forward declaration required by incrVacuumStep(). */
ساكن صحيح allocateBtreePage(BtShared *, MemPage **, Pgno *, Pgno, u8);

/*
** Perform a single step of an incremental-vacuum. If successful, return
** SQLITE_OK. If there is no work to do (and therefore no point in
** calling this function again), return SQLITE_DONE. Or, if an error
** occurs, return some other error code.
**
** More specifically, this function attempts to re-organize the database so
** that the last page of the file currently in use is no longer in use.
**
** Parameter nFin is the number of pages that this database would contain
** were this function called until it returns SQLITE_DONE.
**
** If the bCommit parameter is non-zero, this function assumes that the
** caller will keep calling incrVacuumStep() until it returns SQLITE_DONE
** or an error. bCommit is passed true for an auto-vacuum-on-commit
** operation, or false for an incremental vacuum.
*/
ساكن صحيح incrVacuumStep(BtShared *pBt, Pgno nFin, Pgno iLastPg, صحيح bCommit){
  Pgno nFreeList;           /* Number of pages still on the free-list */
  صحيح rc;

  أكد( sqlite3_mutex_held(pBt->mutex) );
  أكد( iLastPg>nFin );

  إذا( !PTRMAP_ISPAGE(pBt, iLastPg) && iLastPg!=PENDING_BYTE_PAGE(pBt) ){
    u8 eType;
    Pgno iPtrPage;

    nFreeList = get4byte(&pBt->pPage1->aData[36]);
    إذا( nFreeList==0 ){
      أرجع SQLITE_DONE;
    }

    rc = ptrmapGet(pBt, iLastPg, &eType, &iPtrPage);
    إذا( rc!=SQLITE_OK ){
      أرجع rc;
    }
    إذا( eType==PTRMAP_ROOTPAGE ){
      أرجع SQLITE_CORRUPT_BKPT;
    }

    إذا( eType==PTRMAP_FREEPAGE ){
      إذا( bCommit==0 ){
        /* Remove the page from the files free-list. This is not required
        ** if bCommit is non-zero. In that case, the free-list will be
        ** truncated to zero after this function returns, so it doesn't
        ** matter if it still contains some garbage entries.
        */
        Pgno iFreePg;
        MemPage *pFreePg;
        rc = allocateBtreePage(pBt, &pFreePg, &iFreePg, iLastPg, BTALLOC_EXACT);
        إذا( rc!=SQLITE_OK ){
          أرجع rc;
        }
        أكد( iFreePg==iLastPg );
        releasePage(pFreePg);
      }
    } وإلا {
      Pgno iFreePg;             /* Index of free page to move pLastPg to */
      MemPage *pLastPg;
      u8 eMode = BTALLOC_ANY;   /* Mode parameter for allocateBtreePage() */
      Pgno iNear = 0;           /* nearby parameter for allocateBtreePage() */

      rc = btreeGetPage(pBt, iLastPg, &pLastPg, 0);
      إذا( rc!=SQLITE_OK ){
        أرجع rc;
      }

      /* If bCommit is zero, this loop runs exactly once and page pLastPg
      ** is swapped with the first free page pulled off the free list.
      **
      ** On the other hand, if bCommit is greater than zero, then keep
      ** looping until a free-page located within the first nFin pages
      ** of the file is found.
      */
      إذا( bCommit==0 ){
        eMode = BTALLOC_LE;
        iNear = nFin;
      }
      افعل {
        MemPage *pFreePg;
        Pgno dbSize = btreePagecount(pBt);
        rc = allocateBtreePage(pBt, &pFreePg, &iFreePg, iNear, eMode);
        إذا( rc!=SQLITE_OK ){
          releasePage(pLastPg);
          أرجع rc;
        }
        releasePage(pFreePg);
        إذا( iFreePg>dbSize ){
          releasePage(pLastPg);
          أرجع SQLITE_CORRUPT_BKPT;
        }
      }طالما( bCommit && iFreePg>nFin );
      أكد( iFreePg<iLastPg );
     
      rc = relocatePage(pBt, pLastPg, eType, iPtrPage, iFreePg, bCommit);
      releasePage(pLastPg);
      إذا( rc!=SQLITE_OK ){
        أرجع rc;
      }
    }
  }

  إذا( bCommit==0 ){
    افعل {
      iLastPg--;
    }طالما( iLastPg==PENDING_BYTE_PAGE(pBt) || PTRMAP_ISPAGE(pBt, iLastPg) );
    pBt->bDoTruncate = 1;
    pBt->nPage = iLastPg;
  }
  أرجع SQLITE_OK;
}

/*
** The database opened by the first argument is an auto-vacuum database
** nOrig pages in size containing nFree free pages. Return the expected
** size of the database in pages following an auto-vacuum operation.
*/
ساكن Pgno finalDbSize(BtShared *pBt, Pgno nOrig, Pgno nFree){
  صحيح nEntry;                     /* Number of entries on one ptrmap page */
  Pgno nPtrmap;                   /* Number of PtrMap pages to be freed */
  Pgno nFin;                      /* Return value */

  nEntry = pBt->usableSize/5;
  nPtrmap = (nFree-nOrig+PTRMAP_PAGENO(pBt, nOrig)+nEntry)/nEntry;
  nFin = nOrig - nFree - nPtrmap;
  إذا( nOrig>PENDING_BYTE_PAGE(pBt) && nFin<PENDING_BYTE_PAGE(pBt) ){
    nFin--;
  }
  طالما( PTRMAP_ISPAGE(pBt, nFin) || nFin==PENDING_BYTE_PAGE(pBt) ){
    nFin--;
  }

  أرجع nFin;
}

/*
** A write-transaction must be opened before calling this function.
** It performs a single unit of work towards an incremental vacuum.
**
** If the incremental vacuum is finished after this function has run,
** SQLITE_DONE is returned. If it is not finished, but no error occurred,
** SQLITE_OK is returned. Otherwise an SQLite error code.
*/
صحيح sqlite3BtreeIncrVacuum(Btree *p){
  صحيح rc;
  BtShared *pBt = p->pBt;

  sqlite3BtreeEnter(p);
  أكد( pBt->inTransaction==TRANS_WRITE && p->inTrans==TRANS_WRITE );
  إذا( !pBt->autoVacuum ){
    rc = SQLITE_DONE;
  }وإلا{
    Pgno nOrig = btreePagecount(pBt);
    Pgno nFree = get4byte(&pBt->pPage1->aData[36]);
    Pgno nFin = finalDbSize(pBt, nOrig, nFree);

    إذا( nOrig<nFin || nFree>=nOrig ){
      rc = SQLITE_CORRUPT_BKPT;
    }وإلا إذا( nFree>0 ){
      rc = saveAllCursors(pBt, 0, 0);
      إذا( rc==SQLITE_OK ){
        invalidateAllOverflowCache(pBt);
        rc = incrVacuumStep(pBt, nFin, nOrig, 0);
      }
      إذا( rc==SQLITE_OK ){
        rc = sqlite3PagerWrite(pBt->pPage1->pDbPage);
        put4byte(&pBt->pPage1->aData[28], pBt->nPage);
      }
    }وإلا{
      rc = SQLITE_DONE;
    }
  }
  sqlite3BtreeLeave(p);
  أرجع rc;
}

/*
** This routine is called prior to sqlite3PagerCommit when a transaction
** is committed for an auto-vacuum database.
*/
ساكن صحيح autoVacuumCommit(Btree *p){
  صحيح rc = SQLITE_OK;
  Pager *pPager;
  BtShared *pBt;
  sqlite3 *db;
  VVA_ONLY( صحيح nRef );

  أكد( p!=0 );
  pBt = p->pBt; 
  pPager = pBt->pPager;
  VVA_ONLY( nRef = sqlite3PagerRefcount(pPager); )

  أكد( sqlite3_mutex_held(pBt->mutex) );
  invalidateAllOverflowCache(pBt);
  أكد(pBt->autoVacuum);
  إذا( !pBt->incrVacuum ){
    Pgno nFin;         /* Number of pages in database after autovacuuming */
    Pgno nFree;        /* Number of pages on the freelist initially */
    Pgno nVac;         /* Number of pages to vacuum */
    Pgno iFree;        /* The next page to be freed */
    Pgno nOrig;        /* Database size before freeing */

    nOrig = btreePagecount(pBt);
    إذا( PTRMAP_ISPAGE(pBt, nOrig) || nOrig==PENDING_BYTE_PAGE(pBt) ){
      /* It is not possible to create a database for which the final page
      ** is either a pointer-map page or the pending-byte page. If one
      ** is encountered, this indicates corruption.
      */
      أرجع SQLITE_CORRUPT_BKPT;
    }

    nFree = get4byte(&pBt->pPage1->aData[36]);
    db = p->db;
    إذا( db->xAutovacPages ){
      صحيح iDb;
      لكل(iDb=0; ALWAYS(iDb<db->nDb); iDb++){
        إذا( db->aDb[iDb].pBt==p ) اكسر;
      }
      nVac = db->xAutovacPages(
        db->pAutovacPagesArg,
        db->aDb[iDb].zDbSName,
        nOrig,
        nFree,
        pBt->pageSize
      );
      إذا( nVac>nFree ){
        nVac = nFree;
      }
      إذا( nVac==0 ){
        أرجع SQLITE_OK;
      }
    }وإلا{
      nVac = nFree;
    }
    nFin = finalDbSize(pBt, nOrig, nVac);
    إذا( nFin>nOrig ) أرجع SQLITE_CORRUPT_BKPT;
    إذا( nFin<nOrig ){
      rc = saveAllCursors(pBt, 0, 0);
    }
    لكل(iFree=nOrig; iFree>nFin && rc==SQLITE_OK; iFree--){
      rc = incrVacuumStep(pBt, nFin, iFree, nVac==nFree);
    }
    إذا( (rc==SQLITE_DONE || rc==SQLITE_OK) && nFree>0 ){
      rc = sqlite3PagerWrite(pBt->pPage1->pDbPage);
      إذا( nVac==nFree ){
        put4byte(&pBt->pPage1->aData[32], 0);
        put4byte(&pBt->pPage1->aData[36], 0);
      }
      put4byte(&pBt->pPage1->aData[28], nFin);
      pBt->bDoTruncate = 1;
      pBt->nPage = nFin;
    }
    إذا( rc!=SQLITE_OK ){
      sqlite3PagerRollback(pPager);
    }
  }

  أكد( nRef>=sqlite3PagerRefcount(pPager) );
  أرجع rc;
}

#وإلا /* ifndef SQLITE_OMIT_AUTOVACUUM */
# define setChildPtrmaps(x) SQLITE_OK
#endif

/*
** This routine does the first phase of a two-phase commit.  This routine
** causes a rollback journal to be created (if it does not already exist)
** and populated with enough information so that if a power loss occurs
** the database can be restored to its original state by playing back
** the journal.  Then the contents of the journal are flushed out to
** the disk.  After the journal is safely on oxide, the changes to the
** database are written into the database file and flushed to oxide.
** At the end of this call, the rollback journal still exists on the
** disk and we are still holding all locks, so the transaction has not
** committed.  See sqlite3BtreeCommitPhaseTwo() for the second phase of the
** commit process.
**
** This call is a no-op if no write-transaction is currently active on pBt.
**
** Otherwise, sync the database file for the btree pBt. zSuperJrnl points to
** the name of a super-journal file that should be written into the
** individual journal file, or is NULL, indicating no super-journal file
** (single database transaction).
**
** When this is called, the super-journal should already have been
** created, populated with this journal pointer and synced to disk.
**
** Once this is routine has returned, the only thing required to commit
** the write-transaction for this database file is to delete the journal.
*/
صحيح sqlite3BtreeCommitPhaseOne(Btree *p, ثابت حرف *zSuperJrnl){
  صحيح rc = SQLITE_OK;
  إذا( p->inTrans==TRANS_WRITE ){
    BtShared *pBt = p->pBt;
    sqlite3BtreeEnter(p);
#ifndef SQLITE_OMIT_AUTOVACUUM
    إذا( pBt->autoVacuum ){
      rc = autoVacuumCommit(p);
      إذا( rc!=SQLITE_OK ){
        sqlite3BtreeLeave(p);
        أرجع rc;
      }
    }
    إذا( pBt->bDoTruncate ){
      sqlite3PagerTruncateImage(pBt->pPager, pBt->nPage);
    }
#endif
    rc = sqlite3PagerCommitPhaseOne(pBt->pPager, zSuperJrnl, 0);
    sqlite3BtreeLeave(p);
  }
  أرجع rc;
}

/*
** This function is called from both BtreeCommitPhaseTwo() and BtreeRollback()
** at the conclusion of a transaction.
*/
ساكن فراغ btreeEndTransaction(Btree *p){
  BtShared *pBt = p->pBt;
  sqlite3 *db = p->db;
  أكد( sqlite3BtreeHoldsMutex(p) );

#ifndef SQLITE_OMIT_AUTOVACUUM
  pBt->bDoTruncate = 0;
#endif
  إذا( p->inTrans>TRANS_NONE && db->nVdbeRead>1 ){
    /* If there are other active statements that belong to this database
    ** handle, downgrade to a read-only transaction. The other statements
    ** may still be reading from the database.  */
    downgradeAllSharedCacheTableLocks(p);
    p->inTrans = TRANS_READ;
  }وإلا{
    /* If the handle had any kind of transaction open, decrement the
    ** transaction count of the shared btree. If the transaction count
    ** reaches 0, set the shared state to TRANS_NONE. The unlockBtreeIfUnused()
    ** call below will unlock the pager.  */
    إذا( p->inTrans!=TRANS_NONE ){
      clearAllSharedCacheTableLocks(p);
      pBt->nTransaction--;
      إذا( 0==pBt->nTransaction ){
        pBt->inTransaction = TRANS_NONE;
      }
    }

    /* Set the current transaction state to TRANS_NONE and unlock the
    ** pager if this call closed the only read or write transaction.  */
    p->inTrans = TRANS_NONE;
    unlockBtreeIfUnused(pBt);
  }

  btreeIntegrity(p);
}

/*
** Commit the transaction currently in progress.
**
** This routine implements the second phase of a 2-phase commit.  The
** sqlite3BtreeCommitPhaseOne() routine does the first phase and should
** be invoked prior to calling this routine.  The sqlite3BtreeCommitPhaseOne()
** routine did all the work of writing information out to disk and flushing the
** contents so that they are written onto the disk platter.  All this
** routine has to do is delete or truncate or zero the header in the
** the rollback journal (which causes the transaction to commit) and
** drop locks.
**
** Normally, if an error occurs while the pager layer is attempting to
** finalize the underlying journal file, this function returns an error and
** the upper layer will attempt a rollback. However, if the second argument
** is non-zero then this b-tree transaction is part of a multi-file
** transaction. In this case, the transaction has already been committed
** (by deleting a super-journal file) and the caller will ignore this
** functions return code. So, even if an error occurs in the pager layer,
** reset the b-tree objects internal state to indicate that the write
** transaction has been closed. This is quite safe, as the pager will have
** transitioned to the error state.
**
** This will release the write lock on the database file.  If there
** are no active cursors, it also releases the read lock.
*/
صحيح sqlite3BtreeCommitPhaseTwo(Btree *p, صحيح bCleanup){

  إذا( p->inTrans==TRANS_NONE ) أرجع SQLITE_OK;
  sqlite3BtreeEnter(p);
  btreeIntegrity(p);

  /* If the handle has a write-transaction open, commit the shared-btrees
  ** transaction and set the shared state to TRANS_READ.
  */
  إذا( p->inTrans==TRANS_WRITE ){
    صحيح rc;
    BtShared *pBt = p->pBt;
    أكد( pBt->inTransaction==TRANS_WRITE );
    أكد( pBt->nTransaction>0 );
    rc = sqlite3PagerCommitPhaseTwo(pBt->pPager);
    إذا( rc!=SQLITE_OK && bCleanup==0 ){
      sqlite3BtreeLeave(p);
      أرجع rc;
    }
    p->iBDataVersion--;  /* Compensate for pPager->iDataVersion++; */
    pBt->inTransaction = TRANS_READ;
    btreeClearHasContent(pBt);
  }

  btreeEndTransaction(p);
  sqlite3BtreeLeave(p);
  أرجع SQLITE_OK;
}

/*
** Do both phases of a commit.
*/
صحيح sqlite3BtreeCommit(Btree *p){
  صحيح rc;
  sqlite3BtreeEnter(p);
  rc = sqlite3BtreeCommitPhaseOne(p, 0);
  إذا( rc==SQLITE_OK ){
    rc = sqlite3BtreeCommitPhaseTwo(p, 0);
  }
  sqlite3BtreeLeave(p);
  أرجع rc;
}

/*
** This routine sets the state to CURSOR_FAULT and the error
** code to errCode for every cursor on any BtShared that pBtree
** references.  Or if the writeOnly flag is set to 1, then only
** trip write cursors and leave read cursors unchanged.
**
** Every cursor is a candidate to be tripped, including cursors
** that belong to other database connections that happen to be
** sharing the cache with pBtree.
**
** This routine gets called when a rollback occurs. If the writeOnly
** flag is true, then only write-cursors need be tripped - read-only
** cursors save their current positions so that they may continue
** following the rollback. Or, if writeOnly is false, all cursors are
** tripped. In general, writeOnly is false if the transaction being
** rolled back modified the database schema. In this case b-tree root
** pages may be moved or deleted from the database altogether, making
** it unsafe for read cursors to continue.
**
** If the writeOnly flag is true and an error is encountered while
** saving the current position of a read-only cursor, all cursors,
** including all read-cursors are tripped.
**
** SQLITE_OK is returned if successful, or if an error occurs while
** saving a cursor position, an SQLite error code.
*/
صحيح sqlite3BtreeTripAllCursors(Btree *pBtree, صحيح errCode, صحيح writeOnly){
  BtCursor *p;
  صحيح rc = SQLITE_OK;

  أكد( (writeOnly==0 || writeOnly==1) && BTCF_WriteFlag==1 );
  إذا( pBtree ){
    sqlite3BtreeEnter(pBtree);
    لكل(p=pBtree->pBt->pCursor; p; p=p->pNext){
      إذا( writeOnly && (p->curFlags & BTCF_WriteFlag)==0 ){
        إذا( p->eState==CURSOR_VALID || p->eState==CURSOR_SKIPNEXT ){
          rc = saveCursorPosition(p);
          إذا( rc!=SQLITE_OK ){
            (فراغ)sqlite3BtreeTripAllCursors(pBtree, rc, 0);
            اكسر;
          }
        }
      }وإلا{
        sqlite3BtreeClearCursor(p);
        p->eState = CURSOR_FAULT;
        p->skipNext = errCode;
      }
      btreeReleaseAllCursorPages(p);
    }
    sqlite3BtreeLeave(pBtree);
  }
  أرجع rc;
}

/*
** Set the pBt->nPage field correctly, according to the current
** state of the database.  Assume pBt->pPage1 is valid.
*/
ساكن فراغ btreeSetNPage(BtShared *pBt, MemPage *pPage1){
  صحيح nPage = get4byte(&pPage1->aData[28]);
  testcase( nPage==0 );
  إذا( nPage==0 ) sqlite3PagerPagecount(pBt->pPager, &nPage);
  testcase( pBt->nPage!=(u32)nPage );
  pBt->nPage = nPage;
}

/*
** Rollback the transaction in progress.
**
** If tripCode is not SQLITE_OK then cursors will be invalidated (tripped).
** Only write cursors are tripped if writeOnly is true but all cursors are
** tripped if writeOnly is false.  Any attempt to use
** a tripped cursor will result in an error.
**
** This will release the write lock on the database file.  If there
** are no active cursors, it also releases the read lock.
*/
صحيح sqlite3BtreeRollback(Btree *p, صحيح tripCode, صحيح writeOnly){
  صحيح rc;
  BtShared *pBt = p->pBt;
  MemPage *pPage1;

  أكد( writeOnly==1 || writeOnly==0 );
  أكد( tripCode==SQLITE_ABORT_ROLLBACK || tripCode==SQLITE_OK );
  sqlite3BtreeEnter(p);
  إذا( tripCode==SQLITE_OK ){
    rc = tripCode = saveAllCursors(pBt, 0, 0);
    إذا( rc ) writeOnly = 0;
  }وإلا{
    rc = SQLITE_OK;
  }
  إذا( tripCode ){
    صحيح rc2 = sqlite3BtreeTripAllCursors(p, tripCode, writeOnly);
    أكد( rc==SQLITE_OK || (writeOnly==0 && rc2==SQLITE_OK) );
    إذا( rc2!=SQLITE_OK ) rc = rc2;
  }
  btreeIntegrity(p);

  إذا( p->inTrans==TRANS_WRITE ){
    صحيح rc2;

    أكد( TRANS_WRITE==pBt->inTransaction );
    rc2 = sqlite3PagerRollback(pBt->pPager);
    إذا( rc2!=SQLITE_OK ){
      rc = rc2;
    }

    /* The rollback may have destroyed the pPage1->aData value.  So
    ** call btreeGetPage() on page 1 again to make
    ** sure pPage1->aData is set correctly. */
    إذا( btreeGetPage(pBt, 1, &pPage1, 0)==SQLITE_OK ){
      btreeSetNPage(pBt, pPage1);
      releasePageOne(pPage1);
    }
    أكد( سانى_Valid_Cursors(pBt, 1)==0 );
    pBt->inTransaction = TRANS_READ;
    btreeClearHasContent(pBt);
  }

  btreeEndTransaction(p);
  sqlite3BtreeLeave(p);
  أرجع rc;
}

/*
** Start a statement subtransaction. The subtransaction can be rolled
** back independently of the main transaction. You must start a transaction
** before starting a subtransaction. The subtransaction is ended automatically
** if the main transaction commits or rolls back.
**
** Statement subtransactions are used around individual SQL statements
** that are contained within a BEGIN...COMMIT block.  If a constraint
** error occurs within the statement, the effect of that one statement
** can be rolled back without having to rollback the entire transaction.
**
** A statement sub-transaction is implemented as an anonymous savepoint. The
** value passed as the second parameter is the total number of savepoints,
** including the new anonymous savepoint, open on the B-Tree. i.e. if there
** are no active savepoints and no other statement-transactions open,
** iStatement is 1. This anonymous savepoint can be released or rolled back
** using the sqlite3BtreeSavepoint() function.
*/
صحيح sqlite3BtreeBeginStmt(Btree *p, صحيح iStatement){
  صحيح rc;
  BtShared *pBt = p->pBt;
  sqlite3BtreeEnter(p);
  أكد( p->inTrans==TRANS_WRITE );
  أكد( (pBt->btsFlags & BTS_READ_ONLY)==0 );
  أكد( iStatement>0 );
  أكد( iStatement>p->db->nSavepoint );
  أكد( pBt->inTransaction==TRANS_WRITE );
  /* At the pager level, a statement transaction is a savepoint with
  ** an index greater than all savepoints created explicitly using
  ** SQL statements. It is illegal to open, release or rollback any
  ** such savepoints while the statement transaction savepoint is active.
  */
  rc = sqlite3PagerOpenSavepoint(pBt->pPager, iStatement);
  sqlite3BtreeLeave(p);
  أرجع rc;
}

/*
** The second argument to this function, op, is always SAVEPOINT_ROLLBACK
** or SAVEPOINT_RELEASE. This function either releases or rolls back the
** savepoint identified by parameter iSavepoint, depending on the value
** of op.
**
** Normally, iSavepoint is greater than or equal to zero. However, if op is
** SAVEPOINT_ROLLBACK, then iSavepoint may also be -1. In this case the
** contents of the entire transaction are rolled back. This is different
** from a normal transaction rollback, as no locks are released and the
** transaction remains open.
*/
صحيح sqlite3BtreeSavepoint(Btree *p, صحيح op, صحيح iSavepoint){
  صحيح rc = SQLITE_OK;
  إذا( p && p->inTrans==TRANS_WRITE ){
    BtShared *pBt = p->pBt;
    أكد( op==SAVEPOINT_RELEASE || op==SAVEPOINT_ROLLBACK );
    أكد( iSavepoint>=0 || (iSavepoint==-1 && op==SAVEPOINT_ROLLBACK) );
    sqlite3BtreeEnter(p);
    إذا( op==SAVEPOINT_ROLLBACK ){
      rc = saveAllCursors(pBt, 0, 0);
    }
    إذا( rc==SQLITE_OK ){
      rc = sqlite3PagerSavepoint(pBt->pPager, op, iSavepoint);
    }
    إذا( rc==SQLITE_OK ){
      إذا( iSavepoint<0 && (pBt->btsFlags & BTS_INITIALLY_EMPTY)!=0 ){
        pBt->nPage = 0;
      }
      rc = newDatabase(pBt);
      btreeSetNPage(pBt, pBt->pPage1);

      /* pBt->nPage might be zero if the database was corrupt when
      ** the transaction was started. Otherwise, it must be at least 1.  */
      أكد( CORRUPT_DB || pBt->nPage>0 );
    }
    sqlite3BtreeLeave(p);
  }
  أرجع rc;
}

/*
** Create a new cursor for the BTree whose root is on the page
** iTable. If a read-only cursor is requested, it is assumed that
** the caller already has at least a read-only transaction open
** on the database already. If a write-cursor is requested, then
** the caller is assumed to have an open write transaction.
**
** If the BTREE_WRCSR bit of wrFlag is clear, then the cursor can only
** be used for reading.  If the BTREE_WRCSR bit is set, then the cursor
** can be used for reading or for writing if other conditions for writing
** are also met.  These are the conditions that must be met in order
** for writing to be allowed:
**
** 1:  The cursor must have been opened with wrFlag containing BTREE_WRCSR
**
** 2:  Other database connections that share the same pager cache
**     but which are not in the READ_UNCOMMITTED state may not have
**     cursors open with wrFlag==0 on the same table.  Otherwise
**     the changes made by this write cursor would be visible to
**     the read cursors in the other database connection.
**
** 3:  The database must be writable (not on read-only media)
**
** 4:  There must be an active transaction.
**
** The BTREE_FORDELETE bit of wrFlag may optionally be set if BTREE_WRCSR
** is set.  If FORDELETE is set, that is a hint to the implementation that
** this cursor will only be used to seek to and delete entries of an index
** as part of a larger DELETE statement.  The FORDELETE hint is not used by
** this implementation.  But in a hypothetical alternative storage engine
** in which index entries are automatically deleted when corresponding table
** rows are deleted, the FORDELETE flag is a hint that all SEEK and DELETE
** operations on this cursor can be no-ops and all READ operations can
** return a null row (2-bytes: 0x01 0x00).
**
** No checking is done to make sure that page iTable really is the
** root page of a b-tree.  If it is not, then the cursor acquired
** will not work correctly.
**
** It is assumed that the sqlite3BtreeCursorZero() has been called
** on pCur to initialize the memory space prior to invoking this routine.
*/
ساكن صحيح btreeCursor(
  Btree *p,                              /* The btree */
  Pgno iTable,                           /* Root page of table to open */
  صحيح wrFlag,                            /* 1 to write. 0 read-only */
  بنية ئاچقۇچ_Info *p_ئاچقۇچ_Info,              /* First arg to comparison function */
  BtCursor *pCur                         /* Space for new cursor */
){
  BtShared *pBt = p->pBt;                /* Shared b-tree handle */
  BtCursor *pX;                          /* Looping over other all cursors */

  أكد( sqlite3BtreeHoldsMutex(p) );
  أكد( wrFlag==0
       || wrFlag==BTREE_WRCSR
       || wrFlag==(BTREE_WRCSR|BTREE_FORDELETE)
  );

  /* The following assert statements verify that if this is a sharable
  ** b-tree database, the connection is holding the required table locks,
  ** and that no other connection has any open cursor that conflicts with
  ** this lock.  The iTable<1 term disables the check for corrupt schemas. */
  أكد( hasSharedCacheTableLock(p, iTable, p_ئاچقۇچ_Info!=0, (wrFlag?2:1))
          || iTable<1 );
  أكد( wrFlag==0 || !hasReadConflicts(p, iTable) );

  /* Assert that the caller has opened the required transaction. */
  أكد( p->inTrans>TRANS_NONE );
  أكد( wrFlag==0 || p->inTrans==TRANS_WRITE );
  أكد( pBt->pPage1 && pBt->pPage1->aData );
  أكد( wrFlag==0 || (pBt->btsFlags & BTS_READ_ONLY)==0 );

  إذا( iTable<=1 ){
    إذا( iTable<1 ){
      أرجع SQLITE_CORRUPT_BKPT;
    }وإلا إذا( btreePagecount(pBt)==0 ){
      أكد( wrFlag==0 );
      iTable = 0;
    }
  }

  /* Now that no other errors can occur, finish filling in the BtCursor
  ** variables and link the cursor into the BtShared list.  */
  pCur->pgno_يىلتىز = iTable;
  pCur->iPage = -1;
  pCur->p_ئاچقۇچ_Info = p_ئاچقۇچ_Info;
  pCur->pBtree = p;
  pCur->pBt = pBt;
  pCur->curFlags = 0;
  /* If there are two or more cursors on the same btree, then all such
  ** cursors *must* have the BTCF_Multiple flag set. */
  لكل(pX=pBt->pCursor; pX; pX=pX->pNext){
    إذا( pX->pgno_يىلتىز==iTable ){
      pX->curFlags |= BTCF_Multiple;
      pCur->curFlags = BTCF_Multiple;
    }
  }
  pCur->eState = CURSOR_INVALID;
  pCur->pNext = pBt->pCursor;
  pBt->pCursor = pCur;
  إذا( wrFlag ){
    pCur->curFlags |= BTCF_WriteFlag;
    pCur->curPagerFlags = 0;
    إذا( pBt->pTmpSpace==0 ) أرجع allocateTempSpace(pBt);
  }وإلا{
    pCur->curPagerFlags = PAGER_GET_READONLY;
  }
  أرجع SQLITE_OK;
}
ساكن صحيح btreeCursorWithLock(
  Btree *p,                              /* The btree */
  Pgno iTable,                           /* Root page of table to open */
  صحيح wrFlag,                            /* 1 to write. 0 read-only */
  بنية ئاچقۇچ_Info *p_ئاچقۇچ_Info,              /* First arg to comparison function */
  BtCursor *pCur                         /* Space for new cursor */
){
  صحيح rc;
  sqlite3BtreeEnter(p);
  rc = btreeCursor(p, iTable, wrFlag, p_ئاچقۇچ_Info, pCur);
  sqlite3BtreeLeave(p);
  أرجع rc;
}
صحيح sqlite3BtreeCursor(
  Btree *p,                                   /* The btree */
  Pgno iTable,                                /* Root page of table to open */
  صحيح wrFlag,                                 /* 1 to write. 0 read-only */
  بنية ئاچقۇچ_Info *p_ئاچقۇچ_Info,                   /* First arg to xCompare() */
  BtCursor *pCur                              /* Write new cursor here */
){
  إذا( p->sharable ){
    أرجع btreeCursorWithLock(p, iTable, wrFlag, p_ئاچقۇچ_Info, pCur);
  }وإلا{
    أرجع btreeCursor(p, iTable, wrFlag, p_ئاچقۇچ_Info, pCur);
  }
}

/*
** Return the size of a BtCursor object in bytes.
**
** This interfaces is needed so that users of cursors can preallocate
** sufficient storage to hold a cursor.  The BtCursor object is opaque
** to users so they cannot do the sizeof() themselves - they must call
** this routine.
*/
صحيح sqlite3BtreeCursorSize(فراغ){
  أرجع ROUND8(حجم(BtCursor));
}

#ifdef SQLITE_DEBUG
/*
** Return true if and only if the Btree object will be automatically
** closed with the BtCursor closes.  This is used within assert() statements
** only.
*/
صحيح sqlite3BtreeClosesWithCursor(
  Btree *pBtree,       /* the btree object */
  BtCursor *pCur       /* Corresponding cursor */
){
  BtShared *pBt = pBtree->pBt;
  إذا( (pBt->openFlags & BTREE_SINGLE)==0 ) أرجع 0;
  إذا( pBt->pCursor!=pCur ) أرجع 0;
  إذا( pCur->pNext!=0 ) أرجع 0;
  إذا( pCur->pBtree!=pBtree ) أرجع 0;
  أرجع 1;
}
#endif

/*
** Initialize memory that will be converted into a BtCursor object.
**
** The simple approach here would be to memset() the entire object
** to zero.  But it turns out that the apPage[] and aiIdx[] arrays
** do not need to be zeroed and they are large, so we can save a lot
** of run-time by skipping the initialization of those elements.
*/
فراغ sqlite3BtreeCursorZero(BtCursor *p){
  memset(p, 0, offsetof(BtCursor, BTCURSOR_FIRST_UNINIT));
}

/*
** Close a cursor.  The read lock on the database file is released
** when the last cursor is closed.
*/
صحيح sqlite3BtreeCloseCursor(BtCursor *pCur){
  Btree *pBtree = pCur->pBtree;
  إذا( pBtree ){
    BtShared *pBt = pCur->pBt;
    sqlite3BtreeEnter(pBtree);
    أكد( pBt->pCursor!=0 );
    إذا( pBt->pCursor==pCur ){
      pBt->pCursor = pCur->pNext;
    }وإلا{
      BtCursor *pPrev = pBt->pCursor;
      افعل{
        إذا( pPrev->pNext==pCur ){
          pPrev->pNext = pCur->pNext;
          اكسر;
        }
        pPrev = pPrev->pNext;
      }طالما( ALWAYS(pPrev) );
    }
    btreeReleaseAllCursorPages(pCur);
    unlockBtreeIfUnused(pBt);
    sqlite3_free(pCur->aOverflow);
    sqlite3_free(pCur->p_ئاچقۇچ);
    إذا( (pBt->openFlags & BTREE_SINGLE) && pBt->pCursor==0 ){
      /* Since the BtShared is not sharable, there is no need to
      ** worry about the missing sqlite3BtreeLeave() call here.  */
      أكد( pBtree->sharable==0 );
      sqlite3BtreeClose(pBtree);
    }وإلا{
      sqlite3BtreeLeave(pBtree);
    }
    pCur->pBtree = 0;
  }
  أرجع SQLITE_OK;
}

/*
** Make sure the BtCursor* given in the argument has a valid
** BtCursor.info structure.  If it is not already valid, call
** btreeParseCell() to fill it in.
**
** BtCursor.info is a cache of the information in the current cell.
** Using this cache reduces the number of calls to btreeParseCell().
*/
#ifndef NDEBUG
  ساكن صحيح cellInfoEqual(CellInfo *a, CellInfo *b){
    إذا( a->n_ئاچقۇچ!=b->n_ئاچقۇچ ) أرجع 0;
    إذا( a->pPayload!=b->pPayload ) أرجع 0;
    إذا( a->nPayload!=b->nPayload ) أرجع 0;
    إذا( a->nLocal!=b->nLocal ) أرجع 0;
    إذا( a->nSize!=b->nSize ) أرجع 0;
    أرجع 1;
  }
  ساكن فراغ assertCellInfo(BtCursor *pCur){
    CellInfo info;
    memset(&info, 0, حجم(info));
    btreeParseCell(pCur->pPage, pCur->ix, &info);
    أكد( CORRUPT_DB || cellInfoEqual(&info, &pCur->info) );
  }
#وإلا
  #define assertCellInfo(x)
#endif
ساكن SQLITE_NOINLINE فراغ getCellInfo(BtCursor *pCur){
  إذا( pCur->info.nSize==0 ){
    pCur->curFlags |= BTCF_ValidNKey;
    btreeParseCell(pCur->pPage,pCur->ix,&pCur->info);
  }وإلا{
    assertCellInfo(pCur);
  }
}

#ifndef NDEBUG  /* The next routine used only within assert() statements */
/*
** Return true if the given BtCursor is valid.  A valid cursor is one
** that is currently pointing to a row in a (non-empty) table.
** This is a verification routine is used only within assert() statements.
*/
صحيح sqlite3BtreeCursorIsValid(BtCursor *pCur){
  أرجع pCur && pCur->eState==CURSOR_VALID;
}
#endif /* NDEBUG */
صحيح sqlite3BtreeCursorIsValidNN(BtCursor *pCur){
  أكد( pCur!=0 );
  أرجع pCur->eState==CURSOR_VALID;
}

/*
** Return the value of the integer key or "rowid" for a table btree.
** This routine is only valid for a cursor that is pointing into a
** ordinary table btree.  If the cursor points to an index btree or
** is invalid, the result of this routine is undefined.
*/
i64 sqlite3_Btree_Integer_ئاچقۇچ(BtCursor *pCur){
  أكد( cursorHoldsMutex(pCur) );
  أكد( pCur->eState==CURSOR_VALID );
  أكد( pCur->cur_Int_ئاچقۇچ );
  getCellInfo(pCur);
  أرجع pCur->info.n_ئاچقۇچ;
}

/*
** Pin or unpin a cursor.
*/
فراغ sqlite3BtreeCursorPin(BtCursor *pCur){
  أكد( (pCur->curFlags & BTCF_Pinned)==0 );
  pCur->curFlags |= BTCF_Pinned;
}
فراغ sqlite3BtreeCursorUnpin(BtCursor *pCur){
  أكد( (pCur->curFlags & BTCF_Pinned)!=0 );
  pCur->curFlags &= ~BTCF_Pinned;
}

/*
** Return the offset into the database file for the start of the
** payload to which the cursor is pointing.
*/
i64 sqlite3BtreeOffset(BtCursor *pCur){
  أكد( cursorHoldsMutex(pCur) );
  أكد( pCur->eState==CURSOR_VALID );
  getCellInfo(pCur);
  أرجع (i64)pCur->pBt->pageSize*((i64)pCur->pPage->pgno - 1) +
         (i64)(pCur->info.pPayload - pCur->pPage->aData);
}

/*
** Return the number of bytes of payload for the entry that pCur is
** currently pointing to.  For table btrees, this will be the amount
** of data.  For index btrees, this will be the size of the key.
**
** The caller must guarantee that the cursor is pointing to a non-NULL
** valid entry.  In other words, the calling procedure must guarantee
** that the cursor has Cursor.eState==CURSOR_VALID.
*/
u32 sqlite3BtreePayloadSize(BtCursor *pCur){
  أكد( cursorHoldsMutex(pCur) );
  أكد( pCur->eState==CURSOR_VALID );
  getCellInfo(pCur);
  أرجع pCur->info.nPayload;
}

/*
** Return an upper bound on the size of any record for the table
** that the cursor is pointing into.
**
** This is an optimization.  Everything will still work if this
** routine always returns 2147483647 (which is the largest record
** that SQLite can handle) or more.  But returning a smaller value might
** prevent large memory allocations when trying to interpret a
** corrupt database.
**
** The current implementation merely returns the size of the underlying
** database file.
*/
sqlite3_int64 sqlite3BtreeMaxRecordSize(BtCursor *pCur){
  أكد( cursorHoldsMutex(pCur) );
  أكد( pCur->eState==CURSOR_VALID );
  أرجع pCur->pBt->pageSize * (sqlite3_int64)pCur->pBt->nPage;
}

/*
** Given the page number of an overflow page in the database (parameter
** ovfl), this function finds the page number of the next page in the
** linked list of overflow pages. If possible, it uses the auto-vacuum
** pointer-map data instead of reading the content of page ovfl to do so.
**
** If an error occurs an SQLite error code is returned. Otherwise:
**
** The page number of the next overflow page in the linked list is
** written to *pPgnoNext. If page ovfl is the last page in its linked
** list, *pPgnoNext is set to zero.
**
** If ppPage is not NULL, and a reference to the MemPage object corresponding
** to page number pOvfl was obtained, then *ppPage is set to point to that
** reference. It is the responsibility of the caller to call releasePage()
** on *ppPage to free the reference. In no reference was obtained (because
** the pointer-map was used to obtain the value for *pPgnoNext), then
** *ppPage is set to zero.
*/
ساكن صحيح getOverflowPage(
  BtShared *pBt,               /* The database file */
  Pgno ovfl,                   /* Current overflow page number */
  MemPage **ppPage,            /* OUT: MemPage handle (may be NULL) */
  Pgno *pPgnoNext              /* OUT: Next overflow page number */
){
  Pgno next = 0;
  MemPage *pPage = 0;
  صحيح rc = SQLITE_OK;

  أكد( sqlite3_mutex_held(pBt->mutex) );
  أكد(pPgnoNext);

#ifndef SQLITE_OMIT_AUTOVACUUM
  /* Try to find the next page in the overflow list using the
  ** autovacuum pointer-map pages. Guess that the next page in
  ** the overflow list is page number (ovfl+1). If that guess turns
  ** out to be wrong, fall back to loading the data of page
  ** number ovfl to determine the next page number.
  */
  إذا( pBt->autoVacuum ){
    Pgno pgno;
    Pgno iGuess = ovfl+1;
    u8 eType;

    طالما( PTRMAP_ISPAGE(pBt, iGuess) || iGuess==PENDING_BYTE_PAGE(pBt) ){
      iGuess++;
    }

    إذا( iGuess<=btreePagecount(pBt) ){
      rc = ptrmapGet(pBt, iGuess, &eType, &pgno);
      إذا( rc==SQLITE_OK && eType==PTRMAP_OVERFLOW2 && pgno==ovfl ){
        next = iGuess;
        rc = SQLITE_DONE;
      }
    }
  }
#endif

  أكد( next==0 || rc==SQLITE_DONE );
  إذا( rc==SQLITE_OK ){
    rc = btreeGetPage(pBt, ovfl, &pPage, (ppPage==0) ? PAGER_GET_READONLY : 0);
    أكد( rc==SQLITE_OK || pPage==0 );
    إذا( rc==SQLITE_OK ){
      next = get4byte(pPage->aData);
    }
  }

  *pPgnoNext = next;
  إذا( ppPage ){
    *ppPage = pPage;
  }وإلا{
    releasePage(pPage);
  }
  أرجع (rc==SQLITE_DONE ? SQLITE_OK : rc);
}

/*
** Copy data from a buffer to a page, or from a page to a buffer.
**
** pPayload is a pointer to data stored on database page pDbPage.
** If argument eOp is false, then nByte bytes of data are copied
** from pPayload to the buffer pointed at by pBuf. If eOp is true,
** then sqlite3PagerWrite() is called on pDbPage and nByte bytes
** of data are copied from the buffer pBuf to pPayload.
**
** SQLITE_OK is returned on success, otherwise an error code.
*/
ساكن صحيح copyPayload(
  فراغ *pPayload,           /* Pointer to page data */
  فراغ *pBuf,               /* Pointer to buffer */
  صحيح nByte,                /* Number of bytes to copy */
  صحيح eOp,                  /* 0 -> copy from page, 1 -> copy to page */
  DbPage *pDbPage           /* Page containing pPayload */
){
  إذا( eOp ){
    /* Copy data from buffer to page (a write operation) */
    صحيح rc = sqlite3PagerWrite(pDbPage);
    إذا( rc!=SQLITE_OK ){
      أرجع rc;
    }
    memcpy(pPayload, pBuf, nByte);
  }وإلا{
    /* Copy data from page to buffer (a read operation) */
    memcpy(pBuf, pPayload, nByte);
  }
  أرجع SQLITE_OK;
}

/*
** This function is used to read or overwrite payload information
** for the entry that the pCur cursor is pointing to. The eOp
** argument is interpreted as follows:
**
**   0: The operation is a read. Populate the overflow cache.
**   1: The operation is a write. Populate the overflow cache.
**
** A total of "amt" bytes are read or written beginning at "offset".
** Data is read to or from the buffer pBuf.
**
** The content being read or written might appear on the main page
** or be scattered out on multiple overflow pages.
**
** If the current cursor entry uses one or more overflow pages
** this function may allocate space for and lazily populate
** the overflow page-list cache array (BtCursor.aOverflow).
** Subsequent calls use this cache to make seeking to the supplied offset
** more efficient.
**
** Once an overflow page-list cache has been allocated, it must be
** invalidated if some other cursor writes to the same table, or if
** the cursor is moved to a different row. Additionally, in auto-vacuum
** mode, the following events may invalidate an overflow page-list cache.
**
**   * An incremental vacuum,
**   * A commit in auto_vacuum="full" mode,
**   * Creating a table (may require moving an overflow page).
*/
ساكن صحيح accessPayload(
  BtCursor *pCur,      /* Cursor pointing to entry to read from */
  u32 offset,          /* Begin reading this far into payload */
  u32 amt,             /* Read this many bytes */
  unsigned حرف *pBuf, /* Write the bytes into this buffer */
  صحيح eOp              /* zero to read. non-zero to write. */
){
  unsigned حرف *aPayload;
  صحيح rc = SQLITE_OK;
  صحيح iIdx = 0;
  MemPage *pPage = pCur->pPage;               /* Btree page of current entry */
  BtShared *pBt = pCur->pBt;                  /* Btree this cursor belongs to */
#ifdef SQLITE_DIRECT_OVERFLOW_READ
  unsigned حرف * ثابت pBufStart = pBuf;     /* Start of original out buffer */
#endif

  أكد( pPage );
  أكد( eOp==0 || eOp==1 );
  أكد( pCur->eState==CURSOR_VALID );
  إذا( pCur->ix>=pPage->nCell ){
    أرجع SQLITE_CORRUPT_PAGE(pPage);
  }
  أكد( cursorHoldsMutex(pCur) );

  getCellInfo(pCur);
  aPayload = pCur->info.pPayload;
  أكد( (u64)offset+(u64)amt <= (u64)pCur->info.nPayload );

  أكد( aPayload > pPage->aData );
  إذا( (uptr)(aPayload - pPage->aData) > (pBt->usableSize - pCur->info.nLocal) ){
    /* Trying to read or write past the end of the data is an error.  The
    ** conditional above is really:
    **    &aPayload[pCur->info.nLocal] > &pPage->aData[pBt->usableSize]
    ** but is recast into its current form to avoid integer overflow problems
    */
    أرجع SQLITE_CORRUPT_PAGE(pPage);
  }

  /* Check if data must be read/written to/from the btree page itself. */
  إذا( offset<pCur->info.nLocal ){
    صحيح a = amt;
    إذا( a+offset>pCur->info.nLocal ){
      a = pCur->info.nLocal - offset;
    }
    rc = copyPayload(&aPayload[offset], pBuf, a, eOp, pPage->pDbPage);
    offset = 0;
    pBuf += a;
    amt -= a;
  }وإلا{
    offset -= pCur->info.nLocal;
  }


  إذا( rc==SQLITE_OK && amt>0 ){
    ثابت u32 ovflSize = pBt->usableSize - 4;  /* Bytes content per ovfl page */
    Pgno nextPage;

    nextPage = get4byte(&aPayload[pCur->info.nLocal]);

    /* If the BtCursor.aOverflow[] has not been allocated, allocate it now.
    **
    ** The aOverflow[] array is sized at one entry for each overflow page
    ** in the overflow chain. The page number of the first overflow page is
    ** stored in aOverflow[0], etc. A value of 0 in the aOverflow[] array
    ** means "not yet known" (the cache is lazily populated).
    */
    إذا( (pCur->curFlags & BTCF_ValidOvfl)==0 ){
      i64 nOvfl = pCur->info.nPayload;
      testcase( nOvfl - pCur->info.nLocal + ovflSize - 1 > 0xffffffffU );
      nOvfl = (nOvfl - pCur->info.nLocal + ovflSize-1)/ovflSize;
      إذا( pCur->aOverflow==0
       || nOvfl*(صحيح)حجم(Pgno) > sqlite3MallocSize(pCur->aOverflow)
      ){
        Pgno *aNew;
        إذا( sqlite3FaultSim(413) ){
          aNew = 0;
        }وإلا{
          aNew = (Pgno*)sqlite3Realloc(pCur->aOverflow, nOvfl*2*حجم(Pgno));
        }
        إذا( aNew==0 ){
          أرجع SQLITE_NOMEM_BKPT;
        }وإلا{
          pCur->aOverflow = aNew;
        }
      }
      memset(pCur->aOverflow, 0, nOvfl*حجم(Pgno));
      pCur->curFlags |= BTCF_ValidOvfl;
    }وإلا{
      /* Sanity check the validity of the overflow page cache */
      أكد( pCur->aOverflow[0]==nextPage
           || pCur->aOverflow[0]==0
           || CORRUPT_DB );
      أكد( pCur->aOverflow[0]!=0 || pCur->aOverflow[offset/ovflSize]==0 );

      /* If the overflow page-list cache has been allocated and the
      ** entry for the first required overflow page is valid, skip
      ** directly to it.
      */
      إذا( pCur->aOverflow[offset/ovflSize] ){
        iIdx = (offset/ovflSize);
        nextPage = pCur->aOverflow[iIdx];
        offset = (offset%ovflSize);
      }
    }

    أكد( rc==SQLITE_OK && amt>0 );
    طالما( nextPage ){
      /* If required, populate the overflow page-list cache. */
      إذا( nextPage > pBt->nPage ) أرجع SQLITE_CORRUPT_BKPT;
      أكد( pCur->aOverflow[iIdx]==0
              || pCur->aOverflow[iIdx]==nextPage
              || CORRUPT_DB );
      pCur->aOverflow[iIdx] = nextPage;

      إذا( offset>=ovflSize ){
        /* The only reason to read this page is to obtain the page
        ** number for the next page in the overflow chain. The page
        ** data is not required. So first try to lookup the overflow
        ** page-list cache, if any, then fall back to the getOverflowPage()
        ** function.
        */
        أكد( pCur->curFlags & BTCF_ValidOvfl );
        أكد( pCur->pBtree->db==pBt->db );
        إذا( pCur->aOverflow[iIdx+1] ){
          nextPage = pCur->aOverflow[iIdx+1];
        }وإلا{
          rc = getOverflowPage(pBt, nextPage, 0, &nextPage);
        }
        offset -= ovflSize;
      }وإلا{
        /* Need to read this page properly. It contains some of the
        ** range of data that is being read (eOp==0) or written (eOp!=0).
        */
        صحيح a = amt;
        إذا( a + offset > ovflSize ){
          a = ovflSize - offset;
        }

#ifdef SQLITE_DIRECT_OVERFLOW_READ
        /* If all the following are true:
        **
        **   1) this is a read operation, and
        **   2) data is required from the start of this overflow page, and
        **   3) there are no dirty pages in the page-cache
        **   4) the database is file-backed, and
        **   5) the page is not in the WAL file
        **   6) at least 4 bytes have already been read into the output buffer
        **
        ** then data can be read directly from the database file into the
        ** output buffer, bypassing the page-cache altogether. This speeds
        ** up loading large records that span many overflow pages.
        */
        إذا( eOp==0                                             /* (1) */
         && offset==0                                          /* (2) */
         && sqlite3PagerDirectReadOk(pBt->pPager, nextPage)    /* (3,4,5) */
         && &pBuf[-4]>=pBufStart                               /* (6) */
        ){
          sqlite3_file *fd = sqlite3PagerFile(pBt->pPager);
          u8 aSave[4];
          u8 *aWrite = &pBuf[-4];
          أكد( aWrite>=pBufStart );                         /* due to (6) */
          memcpy(aSave, aWrite, 4);
          rc = sqlite3OsRead(fd, aWrite, a+4, (i64)pBt->pageSize*(nextPage-1));
          nextPage = get4byte(aWrite);
          memcpy(aWrite, aSave, 4);
        }وإلا
#endif

        {
          DbPage *pDbPage;
          rc = sqlite3PagerGet(pBt->pPager, nextPage, &pDbPage,
              (eOp==0 ? PAGER_GET_READONLY : 0)
          );
          إذا( rc==SQLITE_OK ){
            إذا( eOp!=0
             && (sqlite3PagerPageRefcount(pDbPage)!=1
                 || NEVER(((MemPage*)sqlite3PagerGetExtra(pDbPage))->isInit))
             && sqlite3FaultSim(411)==SQLITE_OK
            ){
              sqlite3PagerUnref(pDbPage);
              أرجع SQLITE_CORRUPT_PAGE(pPage);
            }
            aPayload = sqlite3PagerGetData(pDbPage);
            nextPage = get4byte(aPayload);
            rc = copyPayload(&aPayload[offset+4], pBuf, a, eOp, pDbPage);
            sqlite3PagerUnref(pDbPage);
            offset = 0;
          }
        }
        amt -= a;
        إذا( amt==0 ) أرجع rc;
        pBuf += a;
      }
      إذا( rc ) اكسر;
      iIdx++;
    }
  }

  إذا( rc==SQLITE_OK && amt>0 ){
    /* Overflow chain ends prematurely */
    أرجع SQLITE_CORRUPT_PAGE(pPage);
  }
  أرجع rc;
}

/*
** Read part of the payload for the row at which that cursor pCur is currently
** pointing.  "amt" bytes will be transferred into pBuf[].  The transfer
** begins at "offset".
**
** pCur can be pointing to either a table or an index b-tree.
** If pointing to a table btree, then the content section is read.  If
** pCur is pointing to an index b-tree then the key section is read.
**
** For sqlite3BtreePayload(), the caller must ensure that pCur is pointing
** to a valid row in the table.  For sqlite3BtreePayloadChecked(), the
** cursor might be invalid or might need to be restored before being read.
**
** Return SQLITE_OK on success or an error code if anything goes
** wrong.  An error is returned if "offset+amt" is larger than
** the available payload.
*/
صحيح sqlite3BtreePayload(BtCursor *pCur, u32 offset, u32 amt, فراغ *pBuf){
  أكد( cursorHoldsMutex(pCur) );
  أكد( pCur->eState==CURSOR_VALID );
  أكد( pCur->iPage>=0 && pCur->pPage );
  أرجع accessPayload(pCur, offset, amt, (unsigned حرف*)pBuf, 0);
}

/*
** This variant of sqlite3BtreePayload() works even if the cursor has not
** in the CURSOR_VALID state.  It is only used by the sqlite3_blob_read()
** interface.
*/
#ifndef SQLITE_OMIT_INCRBLOB
ساكن SQLITE_NOINLINE صحيح accessPayloadChecked(
  BtCursor *pCur,
  u32 offset,
  u32 amt,
  فراغ *pBuf
){
  صحيح rc;
  إذا ( pCur->eState==CURSOR_INVALID ){
    أرجع SQLITE_ABORT;
  }
  أكد( cursorOwnsBtShared(pCur) );
  rc = btreeRestoreCursorPosition(pCur);
  أرجع rc ? rc : accessPayload(pCur, offset, amt, pBuf, 0);
}
صحيح sqlite3BtreePayloadChecked(BtCursor *pCur, u32 offset, u32 amt, فراغ *pBuf){
  إذا( pCur->eState==CURSOR_VALID ){
    أكد( cursorOwnsBtShared(pCur) );
    أرجع accessPayload(pCur, offset, amt, pBuf, 0);
  }وإلا{
    أرجع accessPayloadChecked(pCur, offset, amt, pBuf);
  }
}
#endif /* SQLITE_OMIT_INCRBLOB */

/*
** Return a pointer to payload information from the entry that the
** pCur cursor is pointing to.  The pointer is to the beginning of
** the key if index btrees (pPage->intKey==0) and is the data for
** table btrees (pPage->intKey==1). The number of bytes of available
** key/data is written into *pAmt.  If *pAmt==0, then the value
** returned will not be a valid pointer.
**
** This routine is an optimization.  It is common for the entire key
** and data to fit on the local page and for there to be no overflow
** pages.  When that is so, this routine can be used to access the
** key and data without making a copy.  If the key and/or data spills
** onto overflow pages, then accessPayload() must be used to reassemble
** the key/data and copy it into a preallocated buffer.
**
** The pointer returned by this routine looks directly into the cached
** page of the database.  The data might change or move the next time
** any btree routine is called.
*/
ساكن ثابت فراغ *fetchPayload(
  BtCursor *pCur,      /* Cursor pointing to entry to read from */
  u32 *pAmt            /* Write the number of available bytes here */
){
  صحيح amt;
  أكد( pCur!=0 && pCur->iPage>=0 && pCur->pPage);
  أكد( pCur->eState==CURSOR_VALID );
  أكد( sqlite3_mutex_held(pCur->pBtree->db->mutex) );
  أكد( cursorOwnsBtShared(pCur) );
  أكد( pCur->ix<pCur->pPage->nCell || CORRUPT_DB );
  أكد( pCur->info.nSize>0 );
  أكد( pCur->info.pPayload>pCur->pPage->aData || CORRUPT_DB );
  أكد( pCur->info.pPayload<pCur->pPage->aDataEnd ||CORRUPT_DB);
  amt = pCur->info.nLocal;
  إذا( amt>(صحيح)(pCur->pPage->aDataEnd - pCur->info.pPayload) ){
    /* There is too little space on the page for the expected amount
    ** of local content. Database must be corrupt. */
    أكد( CORRUPT_DB );
    amt = MAX(0, (صحيح)(pCur->pPage->aDataEnd - pCur->info.pPayload));
  }
  *pAmt = (u32)amt;
  أرجع (فراغ*)pCur->info.pPayload;
}


/*
** For the entry that cursor pCur is point to, return as
** many bytes of the key or data as are available on the local
** b-tree page.  Write the number of available bytes into *pAmt.
**
** The pointer returned is ephemeral.  The key/data may move
** or be destroyed on the next call to any Btree routine,
** including calls from other threads against the same cache.
** Hence, a mutex on the BtShared should be held prior to calling
** this routine.
**
** These routines is used to get quick access to key and data
** in the common case where no overflow pages are used.
*/
ثابت فراغ *sqlite3BtreePayloadFetch(BtCursor *pCur, u32 *pAmt){
  أرجع fetchPayload(pCur, pAmt);
}


/*
** Move the cursor down to a new child page.  The newPgno argument is the
** page number of the child page to move to.
**
** This function returns SQLITE_CORRUPT if the page-header flags field of
** the new child page does not match the flags field of the parent (i.e.
** if an intkey page appears to be the parent of a non-intkey page, or
** vice-versa).
*/
ساكن صحيح moveToChild(BtCursor *pCur, u32 newPgno){
  صحيح rc;
  أكد( cursorOwnsBtShared(pCur) );
  أكد( pCur->eState==CURSOR_VALID );
  أكد( pCur->iPage<BTCURSOR_MAX_DEPTH );
  أكد( pCur->iPage>=0 );
  إذا( pCur->iPage>=(BTCURSOR_MAX_DEPTH-1) ){
    أرجع SQLITE_CORRUPT_BKPT;
  }
  pCur->info.nSize = 0;
  pCur->curFlags &= ~(BTCF_ValidNKey|BTCF_ValidOvfl);
  pCur->aiIdx[pCur->iPage] = pCur->ix;
  pCur->apPage[pCur->iPage] = pCur->pPage;
  pCur->ix = 0;
  pCur->iPage++;
  rc = getAndInitPage(pCur->pBt, newPgno, &pCur->pPage, pCur->curPagerFlags);
  أكد( pCur->pPage!=0 || rc!=SQLITE_OK );
  إذا( rc==SQLITE_OK
   && (pCur->pPage->nCell<1 || pCur->pPage->int_ئاچقۇچ!=pCur->cur_Int_ئاچقۇچ)
  ){
    releasePage(pCur->pPage);
    rc = SQLITE_CORRUPT_PGNO(newPgno);
  }
  إذا( rc ){
    pCur->pPage = pCur->apPage[--pCur->iPage];
  }
  أرجع rc;
}

#ifdef SQLITE_DEBUG
/*
** Page pParent is an internal (non-leaf) tree page. This function
** asserts that page number iChild is the left-child if the iIdx'th
** cell in page pParent. Or, if iIdx is equal to the total number of
** cells in pParent, that page number iChild is the right-child of
** the page.
*/
ساكن فراغ assertParentIndex(MemPage *pParent, صحيح iIdx, Pgno iChild){
  إذا( CORRUPT_DB ) أرجع;  /* The conditions tested below might not be true
                            ** in a corrupt database */
  أكد( iIdx<=pParent->nCell );
  إذا( iIdx==pParent->nCell ){
    أكد( get4byte(&pParent->aData[pParent->hdrOffset+8])==iChild );
  }وإلا{
    أكد( get4byte(findCell(pParent, iIdx))==iChild );
  }
}
#وإلا
#  define assertParentIndex(x,y,z)
#endif

/*
** Move the cursor up to the parent page.
**
** pCur->idx is set to the cell index that contains the pointer
** to the page we are coming from.  If we are coming from the
** right-most child page then pCur->idx is set to one more than
** the largest cell index.
*/
ساكن فراغ moveToParent(BtCursor *pCur){
  MemPage *pLeaf;
  أكد( cursorOwnsBtShared(pCur) );
  أكد( pCur->eState==CURSOR_VALID );
  أكد( pCur->iPage>0 );
  أكد( pCur->pPage );
  assertParentIndex(
    pCur->apPage[pCur->iPage-1],
    pCur->aiIdx[pCur->iPage-1],
    pCur->pPage->pgno
  );
  testcase( pCur->aiIdx[pCur->iPage-1] > pCur->apPage[pCur->iPage-1]->nCell );
  pCur->info.nSize = 0;
  pCur->curFlags &= ~(BTCF_ValidNKey|BTCF_ValidOvfl);
  pCur->ix = pCur->aiIdx[pCur->iPage-1];
  pLeaf = pCur->pPage;
  pCur->pPage = pCur->apPage[--pCur->iPage];
  releasePageNotNull(pLeaf);
}

/*
** Move the cursor to point to the root page of its b-tree structure.
**
** If the table has a virtual root page, then the cursor is moved to point
** to the virtual root page instead of the actual root page. A table has a
** virtual root page when the actual root page contains no cells and a
** single child page. This can only happen with the table rooted at page 1.
**
** If the b-tree structure is empty, the cursor state is set to
** CURSOR_INVALID and this routine returns SQLITE_EMPTY. Otherwise,
** the cursor is set to point to the first cell located on the root
** (or virtual root) page and the cursor state is set to CURSOR_VALID.
**
** If this function returns successfully, it may be assumed that the
** page-header flags indicate that the [virtual] root-page is the expected
** kind of b-tree page (i.e. if when opening the cursor the caller did not
** specify a KeyInfo structure the flags byte is set to 0x05 or 0x0D,
** indicating a table b-tree, or if the caller did specify a KeyInfo
** structure the flags byte is set to 0x02 or 0x0A, indicating an index
** b-tree).
*/
ساكن صحيح move_To_يىلتىز(BtCursor *pCur){
  MemPage *p_يىلتىز;
  صحيح rc = SQLITE_OK;

  أكد( cursorOwnsBtShared(pCur) );
  أكد( CURSOR_INVALID < CURSOR_REQUIRESEEK );
  أكد( CURSOR_VALID   < CURSOR_REQUIRESEEK );
  أكد( CURSOR_FAULT   > CURSOR_REQUIRESEEK );
  أكد( pCur->eState < CURSOR_REQUIRESEEK || pCur->iPage<0 );
  أكد( pCur->pgno_يىلتىز>0 || pCur->iPage<0 );

  إذا( pCur->iPage>=0 ){
    إذا( pCur->iPage ){
      releasePageNotNull(pCur->pPage);
      طالما( --pCur->iPage ){
        releasePageNotNull(pCur->apPage[pCur->iPage]);
      }
      p_يىلتىز = pCur->pPage = pCur->apPage[0];
      اذهب skip_init;
    }
  }وإلا إذا( pCur->pgno_يىلتىز==0 ){
    pCur->eState = CURSOR_INVALID;
    أرجع SQLITE_EMPTY;
  }وإلا{
    أكد( pCur->iPage==(-1) );
    إذا( pCur->eState>=CURSOR_REQUIRESEEK ){
      إذا( pCur->eState==CURSOR_FAULT ){
        أكد( pCur->skipNext!=SQLITE_OK );
        أرجع pCur->skipNext;
      }
      sqlite3BtreeClearCursor(pCur);
    }
    rc = getAndInitPage(pCur->pBt, pCur->pgno_يىلتىز, &pCur->pPage,
                        pCur->curPagerFlags);
    إذا( rc!=SQLITE_OK ){
      pCur->eState = CURSOR_INVALID;
      أرجع rc;
    }
    pCur->iPage = 0;
    pCur->cur_Int_ئاچقۇچ = pCur->pPage->int_ئاچقۇچ;
  }
  p_يىلتىز = pCur->pPage;
  أكد( p_يىلتىز->pgno==pCur->pgno_يىلتىز || CORRUPT_DB );

  /* If pCur->pKeyInfo is not NULL, then the caller that opened this cursor
  ** expected to open it on an index b-tree. Otherwise, if pKeyInfo is
  ** NULL, the caller expects a table b-tree. If this is not the case,
  ** return an SQLITE_CORRUPT error.
  **
  ** Earlier versions of SQLite assumed that this test could not fail
  ** if the root page was already loaded when this function was called (i.e.
  ** if pCur->iPage>=0). But this is not so if the database is corrupted
  ** in such a way that page pRoot is linked into a second b-tree table
  ** (or the freelist).  */
  أكد( p_يىلتىز->int_ئاچقۇچ==1 || p_يىلتىز->int_ئاچقۇچ==0 );
  إذا( p_يىلتىز->isInit==0 || (pCur->p_ئاچقۇچ_Info==0)!=p_يىلتىز->int_ئاچقۇچ ){
    أرجع SQLITE_CORRUPT_PAGE(pCur->pPage);
  }

skip_init: 
  pCur->ix = 0;
  pCur->info.nSize = 0;
  pCur->curFlags &= ~(BTCF_AtLast|BTCF_ValidNKey|BTCF_ValidOvfl);

  إذا( p_يىلتىز->nCell>0 ){
    pCur->eState = CURSOR_VALID;
  }وإلا إذا( !p_يىلتىز->leaf ){
    Pgno subpage;
    إذا( p_يىلتىز->pgno!=1 ) أرجع SQLITE_CORRUPT_BKPT;
    subpage = get4byte(&p_يىلتىز->aData[p_يىلتىز->hdrOffset+8]);
    pCur->eState = CURSOR_VALID;
    rc = moveToChild(pCur, subpage);
  }وإلا{
    pCur->eState = CURSOR_INVALID;
    rc = SQLITE_EMPTY;
  }
  أرجع rc;
}

/*
** Move the cursor down to the left-most leaf entry beneath the
** entry to which it is currently pointing.
**
** The left-most leaf is the one with the smallest key - the first
** in ascending order.
*/
ساكن صحيح moveToLeftmost(BtCursor *pCur){
  Pgno pgno;
  صحيح rc = SQLITE_OK;
  MemPage *pPage;

  أكد( cursorOwnsBtShared(pCur) );
  أكد( pCur->eState==CURSOR_VALID );
  طالما( rc==SQLITE_OK && !(pPage = pCur->pPage)->leaf ){
    أكد( pCur->ix<pPage->nCell );
    pgno = get4byte(findCell(pPage, pCur->ix));
    rc = moveToChild(pCur, pgno);
  }
  أرجع rc;
}

/*
** Move the cursor down to the right-most leaf entry beneath the
** page to which it is currently pointing.  Notice the difference
** between moveToLeftmost() and moveToRightmost().  moveToLeftmost()
** finds the left-most entry beneath the *entry* whereas moveToRightmost()
** finds the right-most entry beneath the *page*.
**
** The right-most entry is the one with the largest key - the last
** key in ascending order.
*/
ساكن صحيح moveToRightmost(BtCursor *pCur){
  Pgno pgno;
  صحيح rc = SQLITE_OK;
  MemPage *pPage = 0;

  أكد( cursorOwnsBtShared(pCur) );
  أكد( pCur->eState==CURSOR_VALID );
  طالما( !(pPage = pCur->pPage)->leaf ){
    pgno = get4byte(&pPage->aData[pPage->hdrOffset+8]);
    pCur->ix = pPage->nCell;
    rc = moveToChild(pCur, pgno);
    إذا( rc ) أرجع rc;
  }
  pCur->ix = pPage->nCell-1;
  أكد( pCur->info.nSize==0 );
  أكد( (pCur->curFlags & BTCF_ValidNKey)==0 );
  أرجع SQLITE_OK;
}

/* Move the cursor to the first entry in the table.  Return SQLITE_OK
** on success.  Set *pRes to 0 if the cursor actually points to something
** or set *pRes to 1 if the table is empty.
*/
صحيح sqlite3BtreeFirst(BtCursor *pCur, صحيح *pRes){
  صحيح rc;

  أكد( cursorOwnsBtShared(pCur) );
  أكد( sqlite3_mutex_held(pCur->pBtree->db->mutex) );
  rc = move_To_يىلتىز(pCur);
  إذا( rc==SQLITE_OK ){
    أكد( pCur->pPage->nCell>0 );
    *pRes = 0;
    rc = moveToLeftmost(pCur);
  }وإلا إذا( rc==SQLITE_EMPTY ){
    أكد( pCur->pgno_يىلتىز==0 || (pCur->pPage!=0 && pCur->pPage->nCell==0) );
    *pRes = 1;
    rc = SQLITE_OK;
  }
  أرجع rc;
}

/* Set *pRes to 1 (true) if the BTree pointed to by cursor pCur contains zero
** rows of content.  Set *pRes to 0 (false) if the table contains content.
** Return SQLITE_OK on success or some error code (ex: SQLITE_NOMEM) if
** something goes wrong.
*/
صحيح sqlite3BtreeIsEmpty(BtCursor *pCur, صحيح *pRes){
  صحيح rc;

  أكد( cursorOwnsBtShared(pCur) );
  أكد( sqlite3_mutex_held(pCur->pBtree->db->mutex) );
  إذا( NEVER(pCur->eState==CURSOR_VALID) ){
    *pRes = 0;
    أرجع SQLITE_OK;
  }
  rc = move_To_يىلتىز(pCur);
  إذا( rc==SQLITE_EMPTY ){
    *pRes = 1;
    rc = SQLITE_OK;
  }وإلا{
    *pRes = 0;
  }
  أرجع rc;
}

#ifdef SQLITE_DEBUG
/* The cursors is CURSOR_VALID and has BTCF_AtLast set.  Verify that
** this flags are true for a consistent database.
**
** This routine is is called from within assert() statements only.
** It is an internal verification routine and does not appear in production
** builds.
*/
ساكن صحيح cursorIsAtLastEntry(BtCursor *pCur){
  صحيح ii;
  لكل(ii=0; ii<pCur->iPage; ii++){
    إذا( pCur->aiIdx[ii]!=pCur->apPage[ii]->nCell ) أرجع 0;
  }
  أرجع pCur->ix==pCur->pPage->nCell-1 && pCur->pPage->leaf!=0;
}
#endif

/* Move the cursor to the last entry in the table.  Return SQLITE_OK
** on success.  Set *pRes to 0 if the cursor actually points to something
** or set *pRes to 1 if the table is empty.
*/
ساكن SQLITE_NOINLINE صحيح btreeLast(BtCursor *pCur, صحيح *pRes){
  صحيح rc = move_To_يىلتىز(pCur);
  إذا( rc==SQLITE_OK ){
    أكد( pCur->eState==CURSOR_VALID );
    *pRes = 0;
    rc = moveToRightmost(pCur);
    إذا( rc==SQLITE_OK ){
      pCur->curFlags |= BTCF_AtLast;
    }وإلا{
      pCur->curFlags &= ~BTCF_AtLast;
    }
  }وإلا إذا( rc==SQLITE_EMPTY ){
    أكد( pCur->pgno_يىلتىز==0 || pCur->pPage->nCell==0 );
    *pRes = 1;
    rc = SQLITE_OK;
  }
  أرجع rc;
}
صحيح sqlite3BtreeLast(BtCursor *pCur, صحيح *pRes){
  أكد( cursorOwnsBtShared(pCur) );
  أكد( sqlite3_mutex_held(pCur->pBtree->db->mutex) );

  /* If the cursor already points to the last entry, this is a no-op. */
  إذا( CURSOR_VALID==pCur->eState && (pCur->curFlags & BTCF_AtLast)!=0 ){
    أكد( cursorIsAtLastEntry(pCur) || CORRUPT_DB );
    *pRes = 0;
    أرجع SQLITE_OK;
  }
  أرجع btreeLast(pCur, pRes);
}

/* Move the cursor so that it points to an entry in a table (a.k.a INTKEY)
** table near the key intKey.   Return a success code.
**
** If an exact match is not found, then the cursor is always
** left pointing at a leaf page which would hold the entry if it
** were present.  The cursor might point to an entry that comes
** before or after the key.
**
** An integer is written into *pRes which is the result of
** comparing the key with the entry to which the cursor is
** pointing.  The meaning of the integer written into
** *pRes is as follows:
**
**     *pRes<0      The cursor is left pointing at an entry that
**                  is smaller than intKey or if the table is empty
**                  and the cursor is therefore left point to nothing.
**
**     *pRes==0     The cursor is left pointing at an entry that
**                  exactly matches intKey.
**
**     *pRes>0      The cursor is left pointing at an entry that
**                  is larger than intKey.
*/
صحيح sqlite3BtreeTableMoveto(
  BtCursor *pCur,          /* The cursor to be moved */
  i64 int_ئاچقۇچ,              /* The table key */
  صحيح bias_ئوڭ,           /* If true, bias the search to the high end */
  صحيح *pRes                /* Write search results here */
){
  صحيح rc;

  أكد( cursorOwnsBtShared(pCur) );
  أكد( sqlite3_mutex_held(pCur->pBtree->db->mutex) );
  أكد( pRes );
  أكد( pCur->p_ئاچقۇچ_Info==0 );
  أكد( pCur->eState!=CURSOR_VALID || pCur->cur_Int_ئاچقۇچ!=0 );

  /* If the cursor is already positioned at the point we are trying
  ** to move to, then just return without doing any work */
  إذا( pCur->eState==CURSOR_VALID && (pCur->curFlags & BTCF_ValidNKey)!=0 ){
    إذا( pCur->info.n_ئاچقۇچ==int_ئاچقۇچ ){
      *pRes = 0;
      أرجع SQLITE_OK;
    }
    إذا( pCur->info.n_ئاچقۇچ<int_ئاچقۇچ ){
      إذا( (pCur->curFlags & BTCF_AtLast)!=0 ){
        أكد( cursorIsAtLastEntry(pCur) || CORRUPT_DB );
        *pRes = -1;
        أرجع SQLITE_OK;
      }
      /* If the requested key is one more than the previous key, then
      ** try to get there using sqlite3BtreeNext() rather than a full
      ** binary search.  This is an optimization only.  The correct answer
      ** is still obtained without this case, only a little more slowly. */
      إذا( pCur->info.n_ئاچقۇچ+1==int_ئاچقۇچ ){
        *pRes = 0;
        rc = sqlite3BtreeNext(pCur, 0);
        إذا( rc==SQLITE_OK ){
          getCellInfo(pCur);
          إذا( pCur->info.n_ئاچقۇچ==int_ئاچقۇچ ){
            أرجع SQLITE_OK;
          }
        }وإلا إذا( rc!=SQLITE_DONE ){
          أرجع rc;
        }
      }
    }
  }

#ifdef SQLITE_DEBUG
  pCur->pBtree->nSeek++;   /* Performance measurement during testing */
#endif

  rc = move_To_يىلتىز(pCur);
  إذا( rc ){
    إذا( rc==SQLITE_EMPTY ){
      أكد( pCur->pgno_يىلتىز==0 || pCur->pPage->nCell==0 );
      *pRes = -1;
      أرجع SQLITE_OK;
    }
    أرجع rc;
  }
  أكد( pCur->pPage );
  أكد( pCur->pPage->isInit );
  أكد( pCur->eState==CURSOR_VALID );
  أكد( pCur->pPage->nCell > 0 );
  أكد( pCur->iPage==0 || pCur->apPage[0]->int_ئاچقۇچ==pCur->cur_Int_ئاچقۇچ );
  أكد( pCur->cur_Int_ئاچقۇچ );

  لكل(;;){
    صحيح lwr, upr, idx, c;
    Pgno chldPg;
    MemPage *pPage = pCur->pPage;
    u8 *pCell;                          /* Pointer to current cell in pPage */

    /* pPage->nCell must be greater than zero. If this is the root-page
    ** the cursor would have been INVALID above and this for(;;) loop
    ** not run. If this is not the root-page, then the moveToChild() routine
    ** would have already detected db corruption. Similarly, pPage must
    ** be the right kind (index or table) of b-tree page. Otherwise
    ** a moveToChild() or moveToRoot() call would have detected corruption.  */
    أكد( pPage->nCell>0 );
    أكد( pPage->int_ئاچقۇچ );
    lwr = 0;
    upr = pPage->nCell-1;
    أكد( bias_ئوڭ==0 || bias_ئوڭ==1 );
    idx = upr>>(1-bias_ئوڭ); /* idx = biasRight ? upr : (lwr+upr)/2; */
    لكل(;;){
      i64 n_Cell_ئاچقۇچ;
      pCell = findCellPastPtr(pPage, idx);
      إذا( pPage->int_ئاچقۇچ_Leaf ){
        طالما( 0x80 <= *(pCell++) ){
          إذا( pCell>=pPage->aDataEnd ){
            أرجع SQLITE_CORRUPT_PAGE(pPage);
          }
        }
      }
      n_Cell_ئاچقۇچ = sqlite3_Varint_قىممەت(pCell);
      إذا( n_Cell_ئاچقۇچ<int_ئاچقۇچ ){
        lwr = idx+1;
        إذا( lwr>upr ){ c = -1; اكسر; }
      }وإلا إذا( n_Cell_ئاچقۇچ>int_ئاچقۇچ ){
        upr = idx-1;
        إذا( lwr>upr ){ c = +1; اكسر; }
      }وإلا{
        أكد( n_Cell_ئاچقۇچ==int_ئاچقۇچ );
        pCur->ix = (u16)idx;
        إذا( !pPage->leaf ){
          lwr = idx;
          اذهب moveto_table_next_layer;
        }وإلا{
          pCur->curFlags |= BTCF_ValidNKey;
          pCur->info.n_ئاچقۇچ = n_Cell_ئاچقۇچ;
          pCur->info.nSize = 0;
          *pRes = 0;
          أرجع SQLITE_OK;
        }
      }
      أكد( lwr+upr>=0 );
      idx = (lwr+upr)>>1;  /* idx = (lwr+upr)/2; */
    }
    أكد( lwr==upr+1 || !pPage->leaf );
    أكد( pPage->isInit );
    إذا( pPage->leaf ){
      أكد( pCur->ix<pCur->pPage->nCell );
      pCur->ix = (u16)idx;
      *pRes = c;
      rc = SQLITE_OK;
      اذهب moveto_table_finish;
    }
moveto_table_next_layer:
    إذا( lwr>=pPage->nCell ){
      chldPg = get4byte(&pPage->aData[pPage->hdrOffset+8]);
    }وإلا{
      chldPg = get4byte(findCell(pPage, lwr));
    }
    pCur->ix = (u16)lwr;
    rc = moveToChild(pCur, chldPg);
    إذا( rc ) اكسر;
  }
moveto_table_finish:
  pCur->info.nSize = 0;
  أكد( (pCur->curFlags & BTCF_ValidOvfl)==0 );
  أرجع rc;
}

/*
** Compare the "idx"-th cell on the page pPage against the key
** pointing to by pIdxKey using xRecordCompare.  Return negative or
** zero if the cell is less than or equal pIdxKey.  Return positive
** if unknown.
**
**    Return value negative:     Cell at pPage[idx] less than pIdxKey
**
**    Return value is zero:      Cell at pPage[idx] equals pIdxKey
**
**    Return value positive:     Nothing is known about the relationship
**                               of the cell at pPage[idx] and pIdxKey.
**
** This routine is part of an optimization.  It is always safe to return
** a positive value as that will cause the optimization to be skipped.
*/
ساكن صحيح indexCellCompare(
  MemPage *pPage,
  صحيح idx,
  UnpackedRecord *p_Idx_ئاچقۇچ,
  RecordCompare xRecordCompare
){
  صحيح c;
  صحيح nCell;  /* Size of the pCell cell in bytes */
  u8 *pCell = findCellPastPtr(pPage, idx);

  nCell = pCell[0];
  إذا( nCell<=pPage->max1bytePayload ){
    /* This branch runs if the record-size field of the cell is a
    ** single byte varint and the record fits entirely on the main
    ** b-tree page.  */
    إذا( pCell + nCell >= pPage->aDataEnd ) أرجع 99;
    c = xRecordCompare(nCell, (فراغ*)&pCell[1], p_Idx_ئاچقۇچ);
  }وإلا إذا( !(pCell[1] & 0x80)
    && (nCell = ((nCell&0x7f)<<7) + pCell[1])<=pPage->maxLocal
  ){
    /* The record-size field is a 2 byte varint and the record
    ** fits entirely on the main b-tree page.  */
    إذا( pCell + nCell >= pPage->aDataEnd ) أرجع 99;
    c = xRecordCompare(nCell, (فراغ*)&pCell[2], p_Idx_ئاچقۇچ);
  }وإلا{
    /* If the record extends into overflow pages, do not attempt
    ** the optimization. */
    c = 99;
  }
  أرجع c;
}

/*
** Return true (non-zero) if pCur is current pointing to the last
** page of a table.
*/
ساكن صحيح cursorOnLastPage(BtCursor *pCur){
  صحيح i;
  أكد( pCur->eState==CURSOR_VALID );
  لكل(i=0; i<pCur->iPage; i++){
    MemPage *pPage = pCur->apPage[i];
    إذا( pCur->aiIdx[i]<pPage->nCell ) أرجع 0;
  }
  أرجع 1;
}

/* Move the cursor so that it points to an entry in an index table
** near the key pIdxKey.   Return a success code.
**
** If an exact match is not found, then the cursor is always
** left pointing at a leaf page which would hold the entry if it
** were present.  The cursor might point to an entry that comes
** before or after the key.
**
** An integer is written into *pRes which is the result of
** comparing the key with the entry to which the cursor is
** pointing.  The meaning of the integer written into
** *pRes is as follows:
**
**     *pRes<0      The cursor is left pointing at an entry that
**                  is smaller than pIdxKey or if the table is empty
**                  and the cursor is therefore left point to nothing.
**
**     *pRes==0     The cursor is left pointing at an entry that
**                  exactly matches pIdxKey.
**
**     *pRes>0      The cursor is left pointing at an entry that
**                  is larger than pIdxKey.
**
** The pIdxKey->eqSeen field is set to 1 if there
** exists an entry in the table that exactly matches pIdxKey. 
*/
صحيح sqlite3BtreeIndexMoveto(
  BtCursor *pCur,          /* The cursor to be moved */
  UnpackedRecord *p_Idx_ئاچقۇچ, /* Unpacked index key */
  صحيح *pRes                /* Write search results here */
){
  صحيح rc;
  RecordCompare xRecordCompare;

  أكد( cursorOwnsBtShared(pCur) );
  أكد( sqlite3_mutex_held(pCur->pBtree->db->mutex) );
  أكد( pRes );
  أكد( pCur->p_ئاچقۇچ_Info!=0 );

#ifdef SQLITE_DEBUG
  pCur->pBtree->nSeek++;   /* Performance measurement during testing */
#endif

  xRecordCompare = sqlite3VdbeFindCompare(p_Idx_ئاچقۇچ);
  p_Idx_ئاچقۇچ->errCode = 0;
  أكد( p_Idx_ئاچقۇچ->default_rc==1
       || p_Idx_ئاچقۇچ->default_rc==0
       || p_Idx_ئاچقۇچ->default_rc==-1
  );


  /* Check to see if we can skip a lot of work.  Two cases:
  **
  **    (1) If the cursor is already pointing to the very last cell
  **        in the table and the pIdxKey search key is greater than or
  **        equal to that last cell, then no movement is required.
  **
  **    (2) If the cursor is on the last page of the table and the first
  **        cell on that last page is less than or equal to the pIdxKey
  **        search key, then we can start the search on the current page
  **        without needing to go back to root.
  */
  إذا( pCur->eState==CURSOR_VALID
   && pCur->pPage->leaf
   && cursorOnLastPage(pCur)
  ){
    صحيح c;
    إذا( pCur->ix==pCur->pPage->nCell-1
     && (c = indexCellCompare(pCur->pPage,pCur->ix,p_Idx_ئاچقۇچ,xRecordCompare))<=0
     && p_Idx_ئاچقۇچ->errCode==SQLITE_OK
    ){
      *pRes = c;
      أرجع SQLITE_OK;  /* Cursor already pointing at the correct spot */
    }
    إذا( pCur->iPage>0
     && indexCellCompare(pCur->pPage, 0, p_Idx_ئاچقۇچ, xRecordCompare)<=0
     && p_Idx_ئاچقۇچ->errCode==SQLITE_OK
    ){
      pCur->curFlags &= ~(BTCF_ValidOvfl|BTCF_AtLast);
      إذا( !pCur->pPage->isInit ){
        أرجع SQLITE_CORRUPT_BKPT;
      }
      اذهب bypass_moveto_يىلتىز;  /* Start search on the current page */
    }
    p_Idx_ئاچقۇچ->errCode = SQLITE_OK;
  }

  rc = move_To_يىلتىز(pCur);
  إذا( rc ){
    إذا( rc==SQLITE_EMPTY ){
      أكد( pCur->pgno_يىلتىز==0 || pCur->pPage->nCell==0 );
      *pRes = -1;
      أرجع SQLITE_OK;
    }
    أرجع rc;
  }

bypass_moveto_يىلتىز:
  أكد( pCur->pPage );
  أكد( pCur->pPage->isInit );
  أكد( pCur->eState==CURSOR_VALID );
  أكد( pCur->pPage->nCell > 0 );
  أكد( pCur->cur_Int_ئاچقۇچ==0 );
  أكد( p_Idx_ئاچقۇچ!=0 );
  لكل(;;){
    صحيح lwr, upr, idx, c;
    Pgno chldPg;
    MemPage *pPage = pCur->pPage;
    u8 *pCell;                          /* Pointer to current cell in pPage */

    /* pPage->nCell must be greater than zero. If this is the root-page
    ** the cursor would have been INVALID above and this for(;;) loop
    ** not run. If this is not the root-page, then the moveToChild() routine
    ** would have already detected db corruption. Similarly, pPage must
    ** be the right kind (index or table) of b-tree page. Otherwise
    ** a moveToChild() or moveToRoot() call would have detected corruption.  */
    أكد( pPage->nCell>0 );
    أكد( pPage->int_ئاچقۇچ==0 );
    lwr = 0;
    upr = pPage->nCell-1;
    idx = upr>>1; /* idx = (lwr+upr)/2; */
    لكل(;;){
      صحيح nCell;  /* Size of the pCell cell in bytes */
      pCell = findCellPastPtr(pPage, idx);

      /* The maximum supported page-size is 65536 bytes. This means that
      ** the maximum number of record bytes stored on an index B-Tree
      ** page is less than 16384 bytes and may be stored as a 2-byte
      ** varint. This information is used to attempt to avoid parsing
      ** the entire cell by checking for the cases where the record is
      ** stored entirely within the b-tree page by inspecting the first
      ** 2 bytes of the cell.
      */
      nCell = pCell[0];
      إذا( nCell<=pPage->max1bytePayload ){
        /* This branch runs if the record-size field of the cell is a
        ** single byte varint and the record fits entirely on the main
        ** b-tree page.  */
        إذا( pCell + nCell >= pPage->aDataEnd ){
          rc = SQLITE_CORRUPT_PAGE(pPage);
          اذهب moveto_index_finish;
        }
        c = xRecordCompare(nCell, (فراغ*)&pCell[1], p_Idx_ئاچقۇچ);
      }وإلا إذا( !(pCell[1] & 0x80)
        && (nCell = ((nCell&0x7f)<<7) + pCell[1])<=pPage->maxLocal
        && pCell + nCell < pPage->aDataEnd
      ){
        /* The record-size field is a 2 byte varint and the record
        ** fits entirely on the main b-tree page.  */
        c = xRecordCompare(nCell, (فراغ*)&pCell[2], p_Idx_ئاچقۇچ);
      }وإلا{
        /* The record flows over onto one or more overflow pages. In
        ** this case the whole cell needs to be parsed, a buffer allocated
        ** and accessPayload() used to retrieve the record into the
        ** buffer before VdbeRecordCompare() can be called.
        **
        ** If the record is corrupt, the xRecordCompare routine may read
        ** up to two varints past the end of the buffer. An extra 18
        ** bytes of padding is allocated at the end of the buffer in
        ** case this happens.  */
        فراغ *p_Cell_ئاچقۇچ;
        u8 * ثابت pCellBody = pCell - pPage->childPtrSize;
        ثابت صحيح nOverrun = 18;  /* Size of the overrun padding */
        pPage->xParseCell(pPage, pCellBody, &pCur->info);
        nCell = (صحيح)pCur->info.n_ئاچقۇچ;
        testcase( nCell<0 );   /* True if key size is 2^32 or more */
        testcase( nCell==0 );  /* Invalid key size:  0x80 0x80 0x00 */
        testcase( nCell==1 );  /* Invalid key size:  0x80 0x80 0x01 */
        testcase( nCell==2 );  /* Minimum legal index key size */
        إذا( nCell<2 || nCell/pCur->pBt->usableSize>pCur->pBt->nPage ){
          rc = SQLITE_CORRUPT_PAGE(pPage);
          اذهب moveto_index_finish;
        }
        p_Cell_ئاچقۇچ = sqlite3Malloc( (u64)nCell+(u64)nOverrun );
        إذا( p_Cell_ئاچقۇچ==0 ){
          rc = SQLITE_NOMEM_BKPT;
          اذهب moveto_index_finish;
        }
        pCur->ix = (u16)idx;
        rc = accessPayload(pCur, 0, nCell, (unsigned حرف*)p_Cell_ئاچقۇچ, 0);
        memset(((u8*)p_Cell_ئاچقۇچ)+nCell,0,nOverrun); /* Fix uninit warnings */
        pCur->curFlags &= ~BTCF_ValidOvfl;
        إذا( rc ){
          sqlite3_free(p_Cell_ئاچقۇچ);
          اذهب moveto_index_finish;
        }
        c = sqlite3VdbeRecordCompare(nCell, p_Cell_ئاچقۇچ, p_Idx_ئاچقۇچ);
        sqlite3_free(p_Cell_ئاچقۇچ);
      }
      أكد(
          (p_Idx_ئاچقۇچ->errCode!=SQLITE_CORRUPT || c==0)
       && (p_Idx_ئاچقۇچ->errCode!=SQLITE_NOMEM || pCur->pBtree->db->mallocFailed)
      );
      إذا( c<0 ){
        lwr = idx+1;
      }وإلا إذا( c>0 ){
        upr = idx-1;
      }وإلا{
        أكد( c==0 );
        *pRes = 0;
        rc = SQLITE_OK;
        pCur->ix = (u16)idx;
        إذا( p_Idx_ئاچقۇچ->errCode ) rc = SQLITE_CORRUPT_BKPT;
        اذهب moveto_index_finish;
      }
      إذا( lwr>upr ) اكسر;
      أكد( lwr+upr>=0 );
      idx = (lwr+upr)>>1;  /* idx = (lwr+upr)/2 */
    }
    أكد( lwr==upr+1 || (pPage->int_ئاچقۇچ && !pPage->leaf) );
    أكد( pPage->isInit );
    إذا( pPage->leaf ){
      أكد( pCur->ix<pCur->pPage->nCell || CORRUPT_DB );
      pCur->ix = (u16)idx;
      *pRes = c;
      rc = SQLITE_OK;
      اذهب moveto_index_finish;
    }
    إذا( lwr>=pPage->nCell ){
      chldPg = get4byte(&pPage->aData[pPage->hdrOffset+8]);
    }وإلا{
      chldPg = get4byte(findCell(pPage, lwr));
    }

    /* This block is similar to an in-lined version of:
    **
    **    pCur->ix = (u16)lwr;
    **    rc = moveToChild(pCur, chldPg);
    **    if( rc ) break;
    */
    pCur->info.nSize = 0;
    pCur->curFlags &= ~(BTCF_ValidNKey|BTCF_ValidOvfl);
    إذا( pCur->iPage>=(BTCURSOR_MAX_DEPTH-1) ){
      أرجع SQLITE_CORRUPT_BKPT;
    }
    pCur->aiIdx[pCur->iPage] = (u16)lwr;
    pCur->apPage[pCur->iPage] = pCur->pPage;
    pCur->ix = 0;
    pCur->iPage++;
    rc = getAndInitPage(pCur->pBt, chldPg, &pCur->pPage, pCur->curPagerFlags);
    إذا( rc==SQLITE_OK
     && (pCur->pPage->nCell<1 || pCur->pPage->int_ئاچقۇچ!=pCur->cur_Int_ئاچقۇچ)
    ){
      releasePage(pCur->pPage);
      rc = SQLITE_CORRUPT_PGNO(chldPg);
    }
    إذا( rc ){
      pCur->pPage = pCur->apPage[--pCur->iPage];
      اكسر;
    }
    /*
    ***** End of in-lined moveToChild() call */
 }
moveto_index_finish:
  pCur->info.nSize = 0;
  أكد( (pCur->curFlags & BTCF_ValidOvfl)==0 );
  أرجع rc;
}


/*
** Return TRUE if the cursor is not pointing at an entry of the table.
**
** TRUE will be returned after a call to sqlite3BtreeNext() moves
** past the last entry in the table or sqlite3BtreePrev() moves past
** the first entry.  TRUE is also returned if the table is empty.
*/
صحيح sqlite3BtreeEof(BtCursor *pCur){
  /* TODO: What if the cursor is in CURSOR_REQUIRESEEK but all table entries
  ** have been deleted? This API will need to change to return an error code
  ** as well as the boolean result value.
  */
  أرجع (CURSOR_VALID!=pCur->eState);
}

/*
** Return an estimate for the number of rows in the table that pCur is
** pointing to.  Return a negative number if no estimate is currently
** available.
*/
i64 sqlite3_Btree_Row_سانى_Est(BtCursor *pCur){
  i64 n;
  u8 i;

  أكد( cursorOwnsBtShared(pCur) );
  أكد( sqlite3_mutex_held(pCur->pBtree->db->mutex) );

  /* Currently this interface is only called by the OP_IfSizeBetween
  ** opcode and the OP_Count opcode with P3=1.  In either case,
  ** the cursor will always be valid unless the btree is empty. */
  إذا( pCur->eState!=CURSOR_VALID ) أرجع 0;
  إذا( NEVER(pCur->pPage->leaf==0) ) أرجع -1;

  n = pCur->pPage->nCell;
  لكل(i=0; i<pCur->iPage; i++){
    n *= pCur->apPage[i]->nCell+1;
  }
  أرجع n;
}

/*
** Advance the cursor to the next entry in the database.
** Return value:
**
**    SQLITE_OK        success
**    SQLITE_DONE      cursor is already pointing at the last element
**    otherwise        some kind of error occurred
**
** The main entry point is sqlite3BtreeNext().  That routine is optimized
** for the common case of merely incrementing the cell counter BtCursor.aiIdx
** to the next cell on the current page.  The (slower) btreeNext() helper
** routine is called when it is necessary to move to a different page or
** to restore the cursor.
**
** If bit 0x01 of the F argument in sqlite3BtreeNext(C,F) is 1, then the
** cursor corresponds to an SQL index and this routine could have been
** skipped if the SQL index had been a unique index.  The F argument
** is a hint to the implement.  SQLite btree implementation does not use
** this hint, but COMDB2 does.
*/
ساكن SQLITE_NOINLINE صحيح btreeNext(BtCursor *pCur){
  صحيح rc;
  صحيح idx;
  MemPage *pPage;

  أكد( cursorOwnsBtShared(pCur) );
  إذا( pCur->eState!=CURSOR_VALID ){
    أكد( (pCur->curFlags & BTCF_ValidOvfl)==0 );
    rc = restoreCursorPosition(pCur);
    إذا( rc!=SQLITE_OK ){
      أرجع rc;
    }
    إذا( CURSOR_INVALID==pCur->eState ){
      أرجع SQLITE_DONE;
    }
    إذا( pCur->eState==CURSOR_SKIPNEXT ){
      pCur->eState = CURSOR_VALID;
      إذا( pCur->skipNext>0 ) أرجع SQLITE_OK;
    }
  }

  pPage = pCur->pPage;
  idx = ++pCur->ix;
  إذا( sqlite3FaultSim(412) ) pPage->isInit = 0;
  إذا( !pPage->isInit ){
    أرجع SQLITE_CORRUPT_BKPT;
  }

  إذا( idx>=pPage->nCell ){
    إذا( !pPage->leaf ){
      rc = moveToChild(pCur, get4byte(&pPage->aData[pPage->hdrOffset+8]));
      إذا( rc ) أرجع rc;
      أرجع moveToLeftmost(pCur);
    }
    افعل{
      إذا( pCur->iPage==0 ){
        pCur->eState = CURSOR_INVALID;
        أرجع SQLITE_DONE;
      }
      moveToParent(pCur);
      pPage = pCur->pPage;
    }طالما( pCur->ix>=pPage->nCell );
    إذا( pPage->int_ئاچقۇچ ){
      أرجع sqlite3BtreeNext(pCur, 0);
    }وإلا{
      أرجع SQLITE_OK;
    }
  }
  إذا( pPage->leaf ){
    أرجع SQLITE_OK;
  }وإلا{
    أرجع moveToLeftmost(pCur);
  }
}
صحيح sqlite3BtreeNext(BtCursor *pCur, صحيح flags){
  MemPage *pPage;
  UNUSED_PARAMETER( flags );  /* Used in COMDB2 but not native SQLite */
  أكد( cursorOwnsBtShared(pCur) );
  أكد( flags==0 || flags==1 );
  pCur->info.nSize = 0;
  pCur->curFlags &= ~(BTCF_ValidNKey|BTCF_ValidOvfl);
  إذا( pCur->eState!=CURSOR_VALID ) أرجع btreeNext(pCur);
  pPage = pCur->pPage;
  إذا( (++pCur->ix)>=pPage->nCell ){
    pCur->ix--;
    أرجع btreeNext(pCur);
  }
  إذا( pPage->leaf ){
    أرجع SQLITE_OK;
  }وإلا{
    أرجع moveToLeftmost(pCur);
  }
}

/*
** Step the cursor to the back to the previous entry in the database.
** Return values:
**
**     SQLITE_OK     success
**     SQLITE_DONE   the cursor is already on the first element of the table
**     otherwise     some kind of error occurred
**
** The main entry point is sqlite3BtreePrevious().  That routine is optimized
** for the common case of merely decrementing the cell counter BtCursor.aiIdx
** to the previous cell on the current page.  The (slower) btreePrevious()
** helper routine is called when it is necessary to move to a different page
** or to restore the cursor.
**
** If bit 0x01 of the F argument to sqlite3BtreePrevious(C,F) is 1, then
** the cursor corresponds to an SQL index and this routine could have been
** skipped if the SQL index had been a unique index.  The F argument is a
** hint to the implement.  The native SQLite btree implementation does not
** use this hint, but COMDB2 does.
*/
ساكن SQLITE_NOINLINE صحيح btreePrevious(BtCursor *pCur){
  صحيح rc;
  MemPage *pPage;

  أكد( cursorOwnsBtShared(pCur) );
  أكد( (pCur->curFlags & (BTCF_AtLast|BTCF_ValidOvfl|BTCF_ValidNKey))==0 );
  أكد( pCur->info.nSize==0 );
  إذا( pCur->eState!=CURSOR_VALID ){
    rc = restoreCursorPosition(pCur);
    إذا( rc!=SQLITE_OK ){
      أرجع rc;
    }
    إذا( CURSOR_INVALID==pCur->eState ){
      أرجع SQLITE_DONE;
    }
    إذا( CURSOR_SKIPNEXT==pCur->eState ){
      pCur->eState = CURSOR_VALID;
      إذا( pCur->skipNext<0 ) أرجع SQLITE_OK;
    }
  }

  pPage = pCur->pPage;
  إذا( sqlite3FaultSim(412) ) pPage->isInit = 0;
  إذا( !pPage->isInit ){
    أرجع SQLITE_CORRUPT_BKPT;
  }
  إذا( !pPage->leaf ){
    صحيح idx = pCur->ix;
    rc = moveToChild(pCur, get4byte(findCell(pPage, idx)));
    إذا( rc ) أرجع rc;
    rc = moveToRightmost(pCur);
  }وإلا{
    طالما( pCur->ix==0 ){
      إذا( pCur->iPage==0 ){
        pCur->eState = CURSOR_INVALID;
        أرجع SQLITE_DONE;
      }
      moveToParent(pCur);
    }
    أكد( pCur->info.nSize==0 );
    أكد( (pCur->curFlags & (BTCF_ValidOvfl))==0 );

    pCur->ix--;
    pPage = pCur->pPage;
    إذا( pPage->int_ئاچقۇچ && !pPage->leaf ){
      rc = sqlite3BtreePrevious(pCur, 0);
    }وإلا{
      rc = SQLITE_OK;
    }
  }
  أرجع rc;
}
صحيح sqlite3BtreePrevious(BtCursor *pCur, صحيح flags){
  أكد( cursorOwnsBtShared(pCur) );
  أكد( flags==0 || flags==1 );
  UNUSED_PARAMETER( flags );  /* Used in COMDB2 but not native SQLite */
  pCur->curFlags &= ~(BTCF_AtLast|BTCF_ValidOvfl|BTCF_ValidNKey);
  pCur->info.nSize = 0;
  إذا( pCur->eState!=CURSOR_VALID
   || pCur->ix==0
   || pCur->pPage->leaf==0
  ){
    أرجع btreePrevious(pCur);
  }
  pCur->ix--;
  أرجع SQLITE_OK;
}

/*
** Allocate a new page from the database file.
**
** The new page is marked as dirty.  (In other words, sqlite3PagerWrite()
** has already been called on the new page.)  The new page has also
** been referenced and the calling routine is responsible for calling
** sqlite3PagerUnref() on the new page when it is done.
**
** SQLITE_OK is returned on success.  Any other return value indicates
** an error.  *ppPage is set to NULL in the event of an error.
**
** If the "nearby" parameter is not 0, then an effort is made to
** locate a page close to the page number "nearby".  This can be used in an
** attempt to keep related pages close to each other in the database file,
** which in turn can make database access faster.
**
** If the eMode parameter is BTALLOC_EXACT and the nearby page exists
** anywhere on the free-list, then it is guaranteed to be returned.  If
** eMode is BTALLOC_LT then the page returned will be less than or equal
** to nearby if any such page exists.  If eMode is BTALLOC_ANY then there
** are no restrictions on which page is returned.
*/
ساكن صحيح allocateBtreePage(
  BtShared *pBt,         /* The btree */
  MemPage **ppPage,      /* Store pointer to the allocated page here */
  Pgno *pPgno,           /* Store the page number here */
  Pgno nearby,           /* Search for a page near this one */
  u8 eMode               /* BTALLOC_EXACT, BTALLOC_LT, or BTALLOC_ANY */
){
  MemPage *pPage1;
  صحيح rc;
  u32 n;     /* Number of pages on the freelist */
  u32 k;     /* Number of leaves on the trunk of the freelist */
  MemPage *pTrunk = 0;
  MemPage *pPrevTrunk = 0;
  Pgno mxPage;     /* Total size of the database file */

  أكد( sqlite3_mutex_held(pBt->mutex) );
  أكد( eMode==BTALLOC_ANY || (nearby>0 && IfNotOmitAV(pBt->autoVacuum)) );
  pPage1 = pBt->pPage1;
  mxPage = btreePagecount(pBt);
  /* EVIDENCE-OF: R-21003-45125 The 4-byte big-endian integer at offset 36
  ** stores the total number of pages on the freelist. */
  n = get4byte(&pPage1->aData[36]);
  testcase( n==mxPage-1 );
  إذا( n>=mxPage ){
    أرجع SQLITE_CORRUPT_BKPT;
  }
  إذا( n>0 ){
    /* There are pages on the freelist.  Reuse one of those pages. */
    Pgno iTrunk;
    u8 ئىزدەش_List = 0; /* If the free-list must be searched for 'nearby' */
    u32 n_ئىزدەش = 0;   /* Count of the number of search attempts */
   
    /* If eMode==BTALLOC_EXACT and a query of the pointer-map
    ** shows that the page 'nearby' is somewhere on the free-list, then
    ** the entire-list will be searched for that page.
    */
#ifndef SQLITE_OMIT_AUTOVACUUM
    إذا( eMode==BTALLOC_EXACT ){
      إذا( nearby<=mxPage ){
        u8 eType;
        أكد( nearby>0 );
        أكد( pBt->autoVacuum );
        rc = ptrmapGet(pBt, nearby, &eType, 0);
        إذا( rc ) أرجع rc;
        إذا( eType==PTRMAP_FREEPAGE ){
          ئىزدەش_List = 1;
        }
      }
    }وإلا إذا( eMode==BTALLOC_LE ){
      ئىزدەش_List = 1;
    }
#endif

    /* Decrement the free-list count by 1. Set iTrunk to the index of the
    ** first free-list trunk page. iPrevTrunk is initially 1.
    */
    rc = sqlite3PagerWrite(pPage1->pDbPage);
    إذا( rc ) أرجع rc;
    put4byte(&pPage1->aData[36], n-1);

    /* The code within this loop is run only once if the 'searchList' variable
    ** is not true. Otherwise, it runs once for each trunk-page on the
    ** free-list until the page 'nearby' is located (eMode==BTALLOC_EXACT)
    ** or until a page less than 'nearby' is located (eMode==BTALLOC_LT)
    */
    افعل {
      pPrevTrunk = pTrunk;
      إذا( pPrevTrunk ){
        /* EVIDENCE-OF: R-01506-11053 The first integer on a freelist trunk page
        ** is the page number of the next freelist trunk page in the list or
        ** zero if this is the last freelist trunk page. */
        iTrunk = get4byte(&pPrevTrunk->aData[0]);
      }وإلا{
        /* EVIDENCE-OF: R-59841-13798 The 4-byte big-endian integer at offset 32
        ** stores the page number of the first page of the freelist, or zero if
        ** the freelist is empty. */
        iTrunk = get4byte(&pPage1->aData[32]);
      }
      testcase( iTrunk==mxPage );
      إذا( iTrunk>mxPage || n_ئىزدەش++ > n ){
        rc = SQLITE_CORRUPT_PGNO(pPrevTrunk ? pPrevTrunk->pgno : 1);
      }وإلا{
        rc = btreeGetUnusedPage(pBt, iTrunk, &pTrunk, 0);
      }
      إذا( rc ){
        pTrunk = 0;
        اذهب end_allocate_page;
      }
      أكد( pTrunk!=0 );
      أكد( pTrunk->aData!=0 );
      /* EVIDENCE-OF: R-13523-04394 The second integer on a freelist trunk page
      ** is the number of leaf page pointers to follow. */
      k = get4byte(&pTrunk->aData[4]);
      إذا( k==0 && !ئىزدەش_List ){
        /* The trunk has no leaves and the list is not being searched.
        ** So extract the trunk page itself and use it as the newly
        ** allocated page */
        أكد( pPrevTrunk==0 );
        rc = sqlite3PagerWrite(pTrunk->pDbPage);
        إذا( rc ){
          اذهب end_allocate_page;
        }
        *pPgno = iTrunk;
        memcpy(&pPage1->aData[32], &pTrunk->aData[0], 4);
        *ppPage = pTrunk;
        pTrunk = 0;
        TRACE(("ALLOCATE: %u trunk - %u free pages left\n", *pPgno, n-1));
      }وإلا إذا( k>(u32)(pBt->usableSize/4 - 2) ){
        /* Value of k is out of range.  Database corruption */
        rc = SQLITE_CORRUPT_PGNO(iTrunk);
        اذهب end_allocate_page;
#ifndef SQLITE_OMIT_AUTOVACUUM
      }وإلا إذا( ئىزدەش_List
            && (nearby==iTrunk || (iTrunk<nearby && eMode==BTALLOC_LE))
      ){
        /* The list is being searched and this trunk page is the page
        ** to allocate, regardless of whether it has leaves.
        */
        *pPgno = iTrunk;
        *ppPage = pTrunk;
        ئىزدەش_List = 0;
        rc = sqlite3PagerWrite(pTrunk->pDbPage);
        إذا( rc ){
          اذهب end_allocate_page;
        }
        إذا( k==0 ){
          إذا( !pPrevTrunk ){
            memcpy(&pPage1->aData[32], &pTrunk->aData[0], 4);
          }وإلا{
            rc = sqlite3PagerWrite(pPrevTrunk->pDbPage);
            إذا( rc!=SQLITE_OK ){
              اذهب end_allocate_page;
            }
            memcpy(&pPrevTrunk->aData[0], &pTrunk->aData[0], 4);
          }
        }وإلا{
          /* The trunk page is required by the caller but it contains
          ** pointers to free-list leaves. The first leaf becomes a trunk
          ** page in this case.
          */
          MemPage *pNewTrunk;
          Pgno iNewTrunk = get4byte(&pTrunk->aData[8]);
          إذا( iNewTrunk>mxPage ){
            rc = SQLITE_CORRUPT_PGNO(iTrunk);
            اذهب end_allocate_page;
          }
          testcase( iNewTrunk==mxPage );
          rc = btreeGetUnusedPage(pBt, iNewTrunk, &pNewTrunk, 0);
          إذا( rc!=SQLITE_OK ){
            اذهب end_allocate_page;
          }
          rc = sqlite3PagerWrite(pNewTrunk->pDbPage);
          إذا( rc!=SQLITE_OK ){
            releasePage(pNewTrunk);
            اذهب end_allocate_page;
          }
          memcpy(&pNewTrunk->aData[0], &pTrunk->aData[0], 4);
          put4byte(&pNewTrunk->aData[4], k-1);
          memcpy(&pNewTrunk->aData[8], &pTrunk->aData[12], (k-1)*4);
          releasePage(pNewTrunk);
          إذا( !pPrevTrunk ){
            أكد( sqlite3PagerIswriteable(pPage1->pDbPage) );
            put4byte(&pPage1->aData[32], iNewTrunk);
          }وإلا{
            rc = sqlite3PagerWrite(pPrevTrunk->pDbPage);
            إذا( rc ){
              اذهب end_allocate_page;
            }
            put4byte(&pPrevTrunk->aData[0], iNewTrunk);
          }
        }
        pTrunk = 0;
        TRACE(("ALLOCATE: %u trunk - %u free pages left\n", *pPgno, n-1));
#endif
      }وإلا إذا( k>0 ){
        /* Extract a leaf from the trunk */
        u32 closest;
        Pgno iPage;
        unsigned حرف *aData = pTrunk->aData;
        إذا( nearby>0 ){
          u32 i;
          closest = 0;
          إذا( eMode==BTALLOC_LE ){
            لكل(i=0; i<k; i++){
              iPage = get4byte(&aData[8+i*4]);
              إذا( iPage<=nearby ){
                closest = i;
                اكسر;
              }
            }
          }وإلا{
            صحيح dist;
            dist = sqlite3AbsInt32(get4byte(&aData[8]) - nearby);
            لكل(i=1; i<k; i++){
              صحيح d2 = sqlite3AbsInt32(get4byte(&aData[8+i*4]) - nearby);
              إذا( d2<dist ){
                closest = i;
                dist = d2;
              }
            }
          }
        }وإلا{
          closest = 0;
        }

        iPage = get4byte(&aData[8+closest*4]);
        testcase( iPage==mxPage );
        إذا( iPage>mxPage || iPage<2 ){
          rc = SQLITE_CORRUPT_PGNO(iTrunk);
          اذهب end_allocate_page;
        }
        testcase( iPage==mxPage );
        إذا( !ئىزدەش_List
         || (iPage==nearby || (iPage<nearby && eMode==BTALLOC_LE))
        ){
          صحيح noContent;
          *pPgno = iPage;
          TRACE(("ALLOCATE: %u was leaf %u of %u on trunk %u"
                 ": %u more free pages\n",
                 *pPgno, closest+1, k, pTrunk->pgno, n-1));
          rc = sqlite3PagerWrite(pTrunk->pDbPage);
          إذا( rc ) اذهب end_allocate_page;
          إذا( closest<k-1 ){
            memcpy(&aData[8+closest*4], &aData[4+k*4], 4);
          }
          put4byte(&aData[4], k-1);
          noContent = !btreeGetHasContent(pBt, *pPgno)? PAGER_GET_NOCONTENT : 0;
          rc = btreeGetUnusedPage(pBt, *pPgno, ppPage, noContent);
          إذا( rc==SQLITE_OK ){
            rc = sqlite3PagerWrite((*ppPage)->pDbPage);
            إذا( rc!=SQLITE_OK ){
              releasePage(*ppPage);
              *ppPage = 0;
            }
          }
          ئىزدەش_List = 0;
        }
      }
      releasePage(pPrevTrunk);
      pPrevTrunk = 0;
    }طالما( ئىزدەش_List );
  }وإلا{
    /* There are no pages on the freelist, so append a new page to the
    ** database image.
    **
    ** Normally, new pages allocated by this block can be requested from the
    ** pager layer with the 'no-content' flag set. This prevents the pager
    ** from trying to read the pages content from disk. However, if the
    ** current transaction has already run one or more incremental-vacuum
    ** steps, then the page we are about to allocate may contain content
    ** that is required in the event of a rollback. In this case, do
    ** not set the no-content flag. This causes the pager to load and journal
    ** the current page content before overwriting it.
    **
    ** Note that the pager will not actually attempt to load or journal
    ** content for any page that really does lie past the end of the database
    ** file on disk. So the effects of disabling the no-content optimization
    ** here are confined to those pages that lie between the end of the
    ** database image and the end of the database file.
    */
    صحيح bNoContent = (0==IfNotOmitAV(pBt->bDoTruncate))? PAGER_GET_NOCONTENT:0;

    rc = sqlite3PagerWrite(pBt->pPage1->pDbPage);
    إذا( rc ) أرجع rc;
    pBt->nPage++;
    إذا( pBt->nPage==PENDING_BYTE_PAGE(pBt) ) pBt->nPage++;

#ifndef SQLITE_OMIT_AUTOVACUUM
    إذا( pBt->autoVacuum && PTRMAP_ISPAGE(pBt, pBt->nPage) ){
      /* If *pPgno refers to a pointer-map page, allocate two new pages
      ** at the end of the file instead of one. The first allocated page
      ** becomes a new pointer-map page, the second is used by the caller.
      */
      MemPage *pPg = 0;
      TRACE(("ALLOCATE: %u from end of file (pointer-map page)\n", pBt->nPage));
      أكد( pBt->nPage!=PENDING_BYTE_PAGE(pBt) );
      rc = btreeGetUnusedPage(pBt, pBt->nPage, &pPg, bNoContent);
      إذا( rc==SQLITE_OK ){
        rc = sqlite3PagerWrite(pPg->pDbPage);
        releasePage(pPg);
      }
      إذا( rc ) أرجع rc;
      pBt->nPage++;
      إذا( pBt->nPage==PENDING_BYTE_PAGE(pBt) ){ pBt->nPage++; }
    }
#endif
    put4byte(28 + (u8*)pBt->pPage1->aData, pBt->nPage);
    *pPgno = pBt->nPage;

    أكد( *pPgno!=PENDING_BYTE_PAGE(pBt) );
    rc = btreeGetUnusedPage(pBt, *pPgno, ppPage, bNoContent);
    إذا( rc ) أرجع rc;
    rc = sqlite3PagerWrite((*ppPage)->pDbPage);
    إذا( rc!=SQLITE_OK ){
      releasePage(*ppPage);
      *ppPage = 0;
    }
    TRACE(("ALLOCATE: %u from end of file\n", *pPgno));
  }

  أكد( CORRUPT_DB || *pPgno!=PENDING_BYTE_PAGE(pBt) );

end_allocate_page:
  releasePage(pTrunk);
  releasePage(pPrevTrunk);
  أكد( rc!=SQLITE_OK || sqlite3PagerPageRefcount((*ppPage)->pDbPage)<=1 );
  أكد( rc!=SQLITE_OK || (*ppPage)->isInit==0 );
  أرجع rc;
}

/*
** This function is used to add page iPage to the database file free-list.
** It is assumed that the page is not already a part of the free-list.
**
** The value passed as the second argument to this function is optional.
** If the caller happens to have a pointer to the MemPage object
** corresponding to page iPage handy, it may pass it as the second value.
** Otherwise, it may pass NULL.
**
** If a pointer to a MemPage object is passed as the second argument,
** its reference count is not altered by this function.
*/
ساكن صحيح freePage2(BtShared *pBt, MemPage *pMemPage, Pgno iPage){
  MemPage *pTrunk = 0;                /* Free-list trunk page */
  Pgno iTrunk = 0;                    /* Page number of free-list trunk page */
  MemPage *pPage1 = pBt->pPage1;      /* Local reference to page 1 */
  MemPage *pPage;                     /* Page being freed. May be NULL. */
  صحيح rc;                             /* Return Code */
  u32 nFree;                          /* Initial number of pages on free-list */

  أكد( sqlite3_mutex_held(pBt->mutex) );
  أكد( CORRUPT_DB || iPage>1 );
  أكد( !pMemPage || pMemPage->pgno==iPage );

  إذا( iPage<2 || iPage>pBt->nPage ){
    أرجع SQLITE_CORRUPT_BKPT;
  }
  إذا( pMemPage ){
    pPage = pMemPage;
    sqlite3PagerRef(pPage->pDbPage);
  }وإلا{
    pPage = btreePageLookup(pBt, iPage);
  }

  /* Increment the free page count on pPage1 */
  rc = sqlite3PagerWrite(pPage1->pDbPage);
  إذا( rc ) اذهب freepage_out;
  nFree = get4byte(&pPage1->aData[36]);
  put4byte(&pPage1->aData[36], nFree+1);

  إذا( pBt->btsFlags & BTS_SECURE_DELETE ){
    /* If the secure_delete option is enabled, then
    ** always fully overwrite deleted information with zeros.
    */
    إذا( (!pPage && ((rc = btreeGetPage(pBt, iPage, &pPage, 0))!=0) )
     ||            ((rc = sqlite3PagerWrite(pPage->pDbPage))!=0)
    ){
      اذهب freepage_out;
    }
    memset(pPage->aData, 0, pPage->pBt->pageSize);
  }

  /* If the database supports auto-vacuum, write an entry in the pointer-map
  ** to indicate that the page is free.
  */
  إذا( ISAUTOVACUUM(pBt) ){
    ptrmapPut(pBt, iPage, PTRMAP_FREEPAGE, 0, &rc);
    إذا( rc ) اذهب freepage_out;
  }

  /* Now manipulate the actual database free-list structure. There are two
  ** possibilities. If the free-list is currently empty, or if the first
  ** trunk page in the free-list is full, then this page will become a
  ** new free-list trunk page. Otherwise, it will become a leaf of the
  ** first trunk page in the current free-list. This block tests if it
  ** is possible to add the page as a new free-list leaf.
  */
  إذا( nFree!=0 ){
    u32 nLeaf;                /* Initial number of leaf cells on trunk page */

    iTrunk = get4byte(&pPage1->aData[32]);
    إذا( iTrunk>btreePagecount(pBt) ){
      rc = SQLITE_CORRUPT_BKPT;
      اذهب freepage_out;
    }
    rc = btreeGetPage(pBt, iTrunk, &pTrunk, 0);
    إذا( rc!=SQLITE_OK ){
      اذهب freepage_out;
    }

    nLeaf = get4byte(&pTrunk->aData[4]);
    أكد( pBt->usableSize>32 );
    إذا( nLeaf > (u32)pBt->usableSize/4 - 2 ){
      rc = SQLITE_CORRUPT_BKPT;
      اذهب freepage_out;
    }
    إذا( nLeaf < (u32)pBt->usableSize/4 - 8 ){
      /* In this case there is room on the trunk page to insert the page
      ** being freed as a new leaf.
      **
      ** Note that the trunk page is not really full until it contains
      ** usableSize/4 - 2 entries, not usableSize/4 - 8 entries as we have
      ** coded.  But due to a coding error in versions of SQLite prior to
      ** 3.6.0, databases with freelist trunk pages holding more than
      ** usableSize/4 - 8 entries will be reported as corrupt.  In order
      ** to maintain backwards compatibility with older versions of SQLite,
      ** we will continue to restrict the number of entries to usableSize/4 - 8
      ** for now.  At some point in the future (once everyone has upgraded
      ** to 3.6.0 or later) we should consider fixing the conditional above
      ** to read "usableSize/4-2" instead of "usableSize/4-8".
      **
      ** EVIDENCE-OF: R-19920-11576 However, newer versions of SQLite still
      ** avoid using the last six entries in the freelist trunk page array in
      ** order that database files created by newer versions of SQLite can be
      ** read by older versions of SQLite.
      */
      rc = sqlite3PagerWrite(pTrunk->pDbPage);
      إذا( rc==SQLITE_OK ){
        put4byte(&pTrunk->aData[4], nLeaf+1);
        put4byte(&pTrunk->aData[8+nLeaf*4], iPage);
        إذا( pPage && (pBt->btsFlags & BTS_SECURE_DELETE)==0 ){
          sqlite3PagerDontWrite(pPage->pDbPage);
        }
        rc = btreeSetHasContent(pBt, iPage);
      }
      TRACE(("FREE-PAGE: %u leaf on trunk page %u\n",pPage->pgno,pTrunk->pgno));
      اذهب freepage_out;
    }
  }

  /* If control flows to this point, then it was not possible to add the
  ** the page being freed as a leaf page of the first trunk in the free-list.
  ** Possibly because the free-list is empty, or possibly because the
  ** first trunk in the free-list is full. Either way, the page being freed
  ** will become the new first trunk page in the free-list.
  */
  إذا( pPage==0 && SQLITE_OK!=(rc = btreeGetPage(pBt, iPage, &pPage, 0)) ){
    اذهب freepage_out;
  }
  rc = sqlite3PagerWrite(pPage->pDbPage);
  إذا( rc!=SQLITE_OK ){
    اذهب freepage_out;
  }
  put4byte(pPage->aData, iTrunk);
  put4byte(&pPage->aData[4], 0);
  put4byte(&pPage1->aData[32], iPage);
  TRACE(("FREE-PAGE: %u new trunk page replacing %u\n", pPage->pgno, iTrunk));

freepage_out:
  إذا( pPage ){
    pPage->isInit = 0;
  }
  releasePage(pPage);
  releasePage(pTrunk);
  أرجع rc;
}
ساكن فراغ freePage(MemPage *pPage, صحيح *pRC){
  إذا( (*pRC)==SQLITE_OK ){
    *pRC = freePage2(pPage->pBt, pPage, pPage->pgno);
  }
}

/*
** Free the overflow pages associated with the given Cell.
*/
ساكن SQLITE_NOINLINE صحيح clearCellOverflow(
  MemPage *pPage,          /* The page that contains the Cell */
  unsigned حرف *pCell,    /* First byte of the Cell */
  CellInfo *pInfo          /* Size information about the cell */
){
  BtShared *pBt;
  Pgno ovflPgno;
  صحيح rc;
  صحيح nOvfl;
  u32 ovflPageSize;

  أكد( sqlite3_mutex_held(pPage->pBt->mutex) );
  أكد( pInfo->nLocal!=pInfo->nPayload );
  testcase( pCell + pInfo->nSize == pPage->aDataEnd );
  testcase( pCell + (pInfo->nSize-1) == pPage->aDataEnd );
  إذا( pCell + pInfo->nSize > pPage->aDataEnd ){
    /* Cell extends past end of page */
    أرجع SQLITE_CORRUPT_PAGE(pPage);
  }
  ovflPgno = get4byte(pCell + pInfo->nSize - 4);
  pBt = pPage->pBt;
  أكد( pBt->usableSize > 4 );
  ovflPageSize = pBt->usableSize - 4;
  nOvfl = (pInfo->nPayload - pInfo->nLocal + ovflPageSize - 1)/ovflPageSize;
  أكد( nOvfl>0 ||
    (CORRUPT_DB && (pInfo->nPayload + ovflPageSize)<ovflPageSize)
  );
  طالما( nOvfl-- ){
    Pgno iNext = 0;
    MemPage *pOvfl = 0;
    إذا( ovflPgno<2 || ovflPgno>btreePagecount(pBt) ){
      /* 0 is not a legal page number and page 1 cannot be an
      ** overflow page. Therefore if ovflPgno<2 or past the end of the
      ** file the database must be corrupt. */
      أرجع SQLITE_CORRUPT_BKPT;
    }
    إذا( nOvfl ){
      rc = getOverflowPage(pBt, ovflPgno, &pOvfl, &iNext);
      إذا( rc ) أرجع rc;
    }

    إذا( ( pOvfl || ((pOvfl = btreePageLookup(pBt, ovflPgno))!=0) )
     && sqlite3PagerPageRefcount(pOvfl->pDbPage)!=1
    ){
      /* There is no reason any cursor should have an outstanding reference
      ** to an overflow page belonging to a cell that is being deleted/updated.
      ** So if there exists more than one reference to this page, then it
      ** must not really be an overflow page and the database must be corrupt.
      ** It is helpful to detect this before calling freePage2(), as
      ** freePage2() may zero the page contents if secure-delete mode is
      ** enabled. If this 'overflow' page happens to be a page that the
      ** caller is iterating through or using in some other way, this
      ** can be problematic.
      */
      rc = SQLITE_CORRUPT_BKPT;
    }وإلا{
      rc = freePage2(pBt, pOvfl, ovflPgno);
    }

    إذا( pOvfl ){
      sqlite3PagerUnref(pOvfl->pDbPage);
    }
    إذا( rc ) أرجع rc;
    ovflPgno = iNext;
  }
  أرجع SQLITE_OK;
}

/* Call xParseCell to compute the size of a cell.  If the cell contains
** overflow, then invoke cellClearOverflow to clear out that overflow.
** Store the result code (SQLITE_OK or some error code) in rc.
**
** Implemented as macro to force inlining for performance.
*/
#define BTREE_CLEAR_CELL(rc, pPage, pCell, sInfo)   \
  pPage->xParseCell(pPage, pCell, &sInfo);          \
  إذا( sInfo.nLocal!=sInfo.nPayload ){               \
    rc = clearCellOverflow(pPage, pCell, &sInfo);   \
  }وإلا{                                            \
    rc = SQLITE_OK;                                 \
  }


/*
** Create the byte sequence used to represent a cell on page pPage
** and write that byte sequence into pCell[].  Overflow pages are
** allocated and filled in as necessary.  The calling procedure
** is responsible for making sure sufficient space has been allocated
** for pCell[].
**
** Note that pCell does not necessary need to point to the pPage->aData
** area.  pCell might point to some temporary storage.  The cell will
** be constructed in this temporary area then copied into pPage->aData
** later.
*/
ساكن صحيح fillInCell(
  MemPage *pPage,                /* The page that contains the cell */
  unsigned حرف *pCell,          /* Complete text of the cell */
  ثابت BtreePayload *pX,        /* Payload with which to construct the cell */
  صحيح *pnSize                    /* Write cell size here */
){
  صحيح nPayload;
  ثابت u8 *pSrc;
  صحيح nSrc, n, rc, mn;
  صحيح space_سول;
  MemPage *pToRelease;
  unsigned حرف *pPrior;
  unsigned حرف *pPayload;
  BtShared *pBt;
  Pgno pgnoOvfl;
  صحيح nHeader;

  أكد( sqlite3_mutex_held(pPage->pBt->mutex) );

  /* pPage is not necessarily writeable since pCell might be auxiliary
  ** buffer space that is separate from the pPage buffer area */
  أكد( pCell<pPage->aData || pCell>=&pPage->aData[pPage->pBt->pageSize]
            || sqlite3PagerIswriteable(pPage->pDbPage) );

  /* Fill in the header. */
  nHeader = pPage->childPtrSize;
  إذا( pPage->int_ئاچقۇچ ){
    nPayload = pX->nData + pX->nZero;
    pSrc = pX->pData;
    nSrc = pX->nData;
    أكد( pPage->int_ئاچقۇچ_Leaf ); /* fillInCell() only called for leaves */
    nHeader += putVarint32(&pCell[nHeader], nPayload);
    nHeader += sqlite3PutVarint(&pCell[nHeader], *(u64*)&pX->n_ئاچقۇچ);
  }وإلا{
    أكد( pX->n_ئاچقۇچ<=0x7fffffff && pX->p_ئاچقۇچ!=0 );
    nSrc = nPayload = (صحيح)pX->n_ئاچقۇچ;
    pSrc = pX->p_ئاچقۇچ;
    nHeader += putVarint32(&pCell[nHeader], nPayload);
  }
 
  /* Fill in the payload */
  pPayload = &pCell[nHeader];
  إذا( nPayload<=pPage->maxLocal ){
    /* This is the common case where everything fits on the btree page
    ** and no overflow pages are required. */
    n = nHeader + nPayload;
    testcase( n==3 );
    testcase( n==4 );
    إذا( n<4 ){
      n = 4;
      pPayload[nPayload] = 0;
    }
    *pnSize = n;
    أكد( nSrc<=nPayload );
    testcase( nSrc<nPayload );
    memcpy(pPayload, pSrc, nSrc);
    memset(pPayload+nSrc, 0, nPayload-nSrc);
    أرجع SQLITE_OK;
  }

  /* If we reach this point, it means that some of the content will need
  ** to spill onto overflow pages.
  */
  mn = pPage->minLocal;
  n = mn + (nPayload - mn) % (pPage->pBt->usableSize - 4);
  testcase( n==pPage->maxLocal );
  testcase( n==pPage->maxLocal+1 );
  إذا( n > pPage->maxLocal ) n = mn;
  space_سول = n;
  *pnSize = n + nHeader + 4;
  pPrior = &pCell[nHeader+n];
  pToRelease = 0;
  pgnoOvfl = 0;
  pBt = pPage->pBt;

  /* At this point variables should be set as follows:
  **
  **   nPayload           Total payload size in bytes
  **   pPayload           Begin writing payload here
  **   spaceLeft          Space available at pPayload.  If nPayload>spaceLeft,
  **                      that means content must spill into overflow pages.
  **   *pnSize            Size of the local cell (not counting overflow pages)
  **   pPrior             Where to write the pgno of the first overflow page
  **
  ** Use a call to btreeParseCellPtr() to verify that the values above
  ** were computed correctly.
  */
#ifdef SQLITE_DEBUG
  {
    CellInfo info;
    pPage->xParseCell(pPage, pCell, &info);
    أكد( nHeader==(صحيح)(info.pPayload - pCell) );
    أكد( info.n_ئاچقۇچ==pX->n_ئاچقۇچ );
    أكد( *pnSize == info.nSize );
    أكد( space_سول == info.nLocal );
  }
#endif

  /* Write the payload into the local Cell and any extra into overflow pages */
  طالما( 1 ){
    n = nPayload;
    إذا( n>space_سول ) n = space_سول;

    /* If pToRelease is not zero than pPayload points into the data area
    ** of pToRelease.  Make sure pToRelease is still writeable. */
    أكد( pToRelease==0 || sqlite3PagerIswriteable(pToRelease->pDbPage) );

    /* If pPayload is part of the data area of pPage, then make sure pPage
    ** is still writeable */
    أكد( pPayload<pPage->aData || pPayload>=&pPage->aData[pBt->pageSize]
            || sqlite3PagerIswriteable(pPage->pDbPage) );

    إذا( nSrc>=n ){
      memcpy(pPayload, pSrc, n);
    }وإلا إذا( nSrc>0 ){
      n = nSrc;
      memcpy(pPayload, pSrc, n);
    }وإلا{
      memset(pPayload, 0, n);
    }
    nPayload -= n;
    إذا( nPayload<=0 ) اكسر;
    pPayload += n;
    pSrc += n;
    nSrc -= n;
    space_سول -= n;
    إذا( space_سول==0 ){
      MemPage *pOvfl = 0;
#ifndef SQLITE_OMIT_AUTOVACUUM
      Pgno pgnoPtrmap = pgnoOvfl; /* Overflow page pointer-map entry page */
      إذا( pBt->autoVacuum ){
        افعل{
          pgnoOvfl++;
        } طالما(
          PTRMAP_ISPAGE(pBt, pgnoOvfl) || pgnoOvfl==PENDING_BYTE_PAGE(pBt)
        );
      }
#endif
      rc = allocateBtreePage(pBt, &pOvfl, &pgnoOvfl, pgnoOvfl, 0);
#ifndef SQLITE_OMIT_AUTOVACUUM
      /* If the database supports auto-vacuum, and the second or subsequent
      ** overflow page is being allocated, add an entry to the pointer-map
      ** for that page now.
      **
      ** If this is the first overflow page, then write a partial entry
      ** to the pointer-map. If we write nothing to this pointer-map slot,
      ** then the optimistic overflow chain processing in clearCell()
      ** may misinterpret the uninitialized values and delete the
      ** wrong pages from the database.
      */
      إذا( pBt->autoVacuum && rc==SQLITE_OK ){
        u8 eType = (pgnoPtrmap?PTRMAP_OVERFLOW2:PTRMAP_OVERFLOW1);
        ptrmapPut(pBt, pgnoOvfl, eType, pgnoPtrmap, &rc);
        إذا( rc ){
          releasePage(pOvfl);
        }
      }
#endif
      إذا( rc ){
        releasePage(pToRelease);
        أرجع rc;
      }

      /* If pToRelease is not zero than pPrior points into the data area
      ** of pToRelease.  Make sure pToRelease is still writeable. */
      أكد( pToRelease==0 || sqlite3PagerIswriteable(pToRelease->pDbPage) );

      /* If pPrior is part of the data area of pPage, then make sure pPage
      ** is still writeable */
      أكد( pPrior<pPage->aData || pPrior>=&pPage->aData[pBt->pageSize]
            || sqlite3PagerIswriteable(pPage->pDbPage) );

      put4byte(pPrior, pgnoOvfl);
      releasePage(pToRelease);
      pToRelease = pOvfl;
      pPrior = pOvfl->aData;
      put4byte(pPrior, 0);
      pPayload = &pOvfl->aData[4];
      space_سول = pBt->usableSize - 4;
    }
  }
  releasePage(pToRelease);
  أرجع SQLITE_OK;
}

/*
** Remove the i-th cell from pPage.  This routine effects pPage only.
** The cell content is not freed or deallocated.  It is assumed that
** the cell content has been copied someplace else.  This routine just
** removes the reference to the cell from pPage.
**
** "sz" must be the number of bytes in the cell.
*/
ساكن فراغ dropCell(MemPage *pPage, صحيح idx, صحيح sz, صحيح *pRC){
  u32 pc;         /* Offset to cell content of cell being deleted */
  u8 *data;       /* pPage->aData */
  u8 *ptr;        /* Used to move bytes around within data[] */
  صحيح rc;         /* The return code */
  صحيح hdr;        /* Beginning of the header.  0 most pages.  100 page 1 */

  إذا( *pRC ) أرجع;
  أكد( idx>=0 );
  أكد( idx<pPage->nCell );
  أكد( CORRUPT_DB || sz==cellSize(pPage, idx) );
  أكد( sqlite3PagerIswriteable(pPage->pDbPage) );
  أكد( sqlite3_mutex_held(pPage->pBt->mutex) );
  أكد( pPage->nFree>=0 );
  data = pPage->aData;
  ptr = &pPage->aCellIdx[2*idx];
  أكد( pPage->pBt->usableSize > (u32)(ptr-data) );
  pc = get2byte(ptr);
  hdr = pPage->hdrOffset;
  testcase( pc==(u32)get2byte(&data[hdr+5]) );
  testcase( pc+sz==pPage->pBt->usableSize );
  إذا( pc+sz > pPage->pBt->usableSize ){
    *pRC = SQLITE_CORRUPT_BKPT;
    أرجع;
  }
  rc = freeSpace(pPage, pc, sz);
  إذا( rc ){
    *pRC = rc;
    أرجع;
  }
  pPage->nCell--;
  إذا( pPage->nCell==0 ){
    memset(&data[hdr+1], 0, 4);
    data[hdr+7] = 0;
    put2byte(&data[hdr+5], pPage->pBt->usableSize);
    pPage->nFree = pPage->pBt->usableSize - pPage->hdrOffset
                       - pPage->childPtrSize - 8;
  }وإلا{
    memmove(ptr, ptr+2, 2*(pPage->nCell - idx));
    put2byte(&data[hdr+3], pPage->nCell);
    pPage->nFree += 2;
  }
}

/*
** Insert a new cell on pPage at cell index "i".  pCell points to the
** content of the cell.
**
** If the cell content will fit on the page, then put it there.  If it
** will not fit, then make a copy of the cell content into pTemp if
** pTemp is not null.  Regardless of pTemp, allocate a new entry
** in pPage->apOvfl[] and make it point to the cell content (either
** in pTemp or the original pCell) and also record its index.
** Allocating a new entry in pPage->aCell[] implies that
** pPage->nOverflow is incremented.
**
** The insertCellFast() routine below works exactly the same as
** insertCell() except that it lacks the pTemp and iChild parameters
** which are assumed zero.  Other than that, the two routines are the
** same.
**
** Fixes or enhancements to this routine should be reflected in
** insertCellFast()!
*/
ساكن صحيح قىستۇرۇش_Cell(
  MemPage *pPage,   /* Page into which we are copying */
  صحيح i,            /* New cell becomes the i-th cell of the page */
  u8 *pCell,        /* Content of the new cell */
  صحيح sz,           /* Bytes of content in pCell */
  u8 *pTemp,        /* Temp storage space for pCell, if needed */
  Pgno iChild       /* If non-zero, replace first 4 bytes with this value */
){
  صحيح idx = 0;      /* Where to write new cell content in data[] */
  صحيح j;            /* Loop counter */
  u8 *data;         /* The content of the whole page */
  u8 *pIns;         /* The point in pPage->aCellIdx[] where no cell inserted */

  أكد( i>=0 && i<=pPage->nCell+pPage->nOverflow );
  أكد( MX_CELL(pPage->pBt)<=10921 );
  أكد( pPage->nCell<=MX_CELL(pPage->pBt) || CORRUPT_DB );
  أكد( pPage->nOverflow<=ArraySize(pPage->apOvfl) );
  أكد( ArraySize(pPage->apOvfl)==ArraySize(pPage->aiOvfl) );
  أكد( sqlite3_mutex_held(pPage->pBt->mutex) );
  أكد( sz==pPage->xCellSize(pPage, pCell) || CORRUPT_DB );
  أكد( pPage->nFree>=0 );
  أكد( iChild>0 );
  إذا( pPage->nOverflow || sz+2>pPage->nFree ){
    إذا( pTemp ){
      memcpy(pTemp, pCell, sz);
      pCell = pTemp;
    }
    put4byte(pCell, iChild);
    j = pPage->nOverflow++;
    /* Comparison against ArraySize-1 since we hold back one extra slot
    ** as a contingency.  In other words, never need more than 3 overflow
    ** slots but 4 are allocated, just to be safe. */
    أكد( j < ArraySize(pPage->apOvfl)-1 );
    pPage->apOvfl[j] = pCell;
    pPage->aiOvfl[j] = (u16)i;

    /* When multiple overflows occur, they are always sequential and in
    ** sorted order.  This invariants arise because multiple overflows can
    ** only occur when inserting divider cells into the parent page during
    ** balancing, and the dividers are adjacent and sorted.
    */
    أكد( j==0 || pPage->aiOvfl[j-1]<(u16)i ); /* Overflows in sorted order */
    أكد( j==0 || i==pPage->aiOvfl[j-1]+1 );   /* Overflows are sequential */
  }وإلا{
    صحيح rc = sqlite3PagerWrite(pPage->pDbPage);
    إذا( NEVER(rc!=SQLITE_OK) ){
      أرجع rc;
    }
    أكد( sqlite3PagerIswriteable(pPage->pDbPage) );
    data = pPage->aData;
    أكد( &data[pPage->cellOffset]==pPage->aCellIdx );
    rc = allocateSpace(pPage, sz, &idx);
    إذا( rc ){ أرجع rc; }
    /* The allocateSpace() routine guarantees the following properties
    ** if it returns successfully */
    أكد( idx >= 0 );
    أكد( idx >= pPage->cellOffset+2*pPage->nCell+2 || CORRUPT_DB );
    أكد( idx+sz <= (صحيح)pPage->pBt->usableSize );
    pPage->nFree -= (u16)(2 + sz);
    /* In a corrupt database where an entry in the cell index section of
    ** a btree page has a value of 3 or less, the pCell value might point
    ** as many as 4 bytes in front of the start of the aData buffer for
    ** the source page.  Make sure this does not cause problems by not
    ** reading the first 4 bytes */
    memcpy(&data[idx+4], pCell+4, sz-4);
    put4byte(&data[idx], iChild);
    pIns = pPage->aCellIdx + i*2;
    إذا( i<pPage->nCell ) memmove(pIns+2, pIns, 2*(pPage->nCell - i));
    put2byte(pIns, idx);
    pPage->nCell++;
    /* increment the cell count */
    إذا( (++data[pPage->hdrOffset+4])==0 ) data[pPage->hdrOffset+3]++;
    أكد( get2byte(&data[pPage->hdrOffset+3])==pPage->nCell || CORRUPT_DB );
#ifndef SQLITE_OMIT_AUTOVACUUM
    إذا( pPage->pBt->autoVacuum ){
      صحيح rc2 = SQLITE_OK;
      /* The cell may contain a pointer to an overflow page. If so, write
      ** the entry for the overflow page into the pointer map.
      */
      ptrmapPutOvflPtr(pPage, pPage, pCell, &rc2);
      إذا( rc2 ) أرجع rc2;
    }
#endif
  }
  أرجع SQLITE_OK;
}

/*
** This variant of insertCell() assumes that the pTemp and iChild
** parameters are both zero.  Use this variant in sqlite3BtreeInsert()
** for performance improvement, and also so that this variant is only
** called from that one place, and is thus inlined, and thus runs must
** faster.
**
** Fixes or enhancements to this routine should be reflected into
** the insertCell() routine.
*/
ساكن صحيح قىستۇرۇش_Cell_Fast(
  MemPage *pPage,   /* Page into which we are copying */
  صحيح i,            /* New cell becomes the i-th cell of the page */
  u8 *pCell,        /* Content of the new cell */
  صحيح sz            /* Bytes of content in pCell */
){
  صحيح idx = 0;      /* Where to write new cell content in data[] */
  صحيح j;            /* Loop counter */
  u8 *data;         /* The content of the whole page */
  u8 *pIns;         /* The point in pPage->aCellIdx[] where no cell inserted */

  أكد( i>=0 && i<=pPage->nCell+pPage->nOverflow );
  أكد( MX_CELL(pPage->pBt)<=10921 );
  أكد( pPage->nCell<=MX_CELL(pPage->pBt) || CORRUPT_DB );
  أكد( pPage->nOverflow<=ArraySize(pPage->apOvfl) );
  أكد( ArraySize(pPage->apOvfl)==ArraySize(pPage->aiOvfl) );
  أكد( sqlite3_mutex_held(pPage->pBt->mutex) );
  أكد( sz==pPage->xCellSize(pPage, pCell) || CORRUPT_DB );
  أكد( pPage->nFree>=0 );
  أكد( pPage->nOverflow==0 );
  إذا( sz+2>pPage->nFree ){
    j = pPage->nOverflow++;
    /* Comparison against ArraySize-1 since we hold back one extra slot
    ** as a contingency.  In other words, never need more than 3 overflow
    ** slots but 4 are allocated, just to be safe. */
    أكد( j < ArraySize(pPage->apOvfl)-1 );
    pPage->apOvfl[j] = pCell;
    pPage->aiOvfl[j] = (u16)i;

    /* When multiple overflows occur, they are always sequential and in
    ** sorted order.  This invariants arise because multiple overflows can
    ** only occur when inserting divider cells into the parent page during
    ** balancing, and the dividers are adjacent and sorted.
    */
    أكد( j==0 || pPage->aiOvfl[j-1]<(u16)i ); /* Overflows in sorted order */
    أكد( j==0 || i==pPage->aiOvfl[j-1]+1 );   /* Overflows are sequential */
  }وإلا{
    صحيح rc = sqlite3PagerWrite(pPage->pDbPage);
    إذا( rc!=SQLITE_OK ){
      أرجع rc;
    }
    أكد( sqlite3PagerIswriteable(pPage->pDbPage) );
    data = pPage->aData;
    أكد( &data[pPage->cellOffset]==pPage->aCellIdx );
    rc = allocateSpace(pPage, sz, &idx);
    إذا( rc ){ أرجع rc; }
    /* The allocateSpace() routine guarantees the following properties
    ** if it returns successfully */
    أكد( idx >= 0 );
    أكد( idx >= pPage->cellOffset+2*pPage->nCell+2 || CORRUPT_DB );
    أكد( idx+sz <= (صحيح)pPage->pBt->usableSize );
    pPage->nFree -= (u16)(2 + sz);
    memcpy(&data[idx], pCell, sz);
    pIns = pPage->aCellIdx + i*2;
    إذا( i<pPage->nCell ) memmove(pIns+2, pIns, 2*(pPage->nCell - i));
    put2byte(pIns, idx);
    pPage->nCell++;
    /* increment the cell count */
    إذا( (++data[pPage->hdrOffset+4])==0 ) data[pPage->hdrOffset+3]++;
    أكد( get2byte(&data[pPage->hdrOffset+3])==pPage->nCell || CORRUPT_DB );
#ifndef SQLITE_OMIT_AUTOVACUUM
    إذا( pPage->pBt->autoVacuum ){
      صحيح rc2 = SQLITE_OK;
      /* The cell may contain a pointer to an overflow page. If so, write
      ** the entry for the overflow page into the pointer map.
      */
      ptrmapPutOvflPtr(pPage, pPage, pCell, &rc2);
      إذا( rc2 ) أرجع rc2;
    }
#endif
  }
  أرجع SQLITE_OK;
}

/*
** The following parameters determine how many adjacent pages get involved
** in a balancing operation.  NN is the number of neighbors on either side
** of the page that participate in the balancing operation.  NB is the
** total number of pages that participate, including the target page and
** NN neighbors on either side.
**
** The minimum value of NN is 1 (of course).  Increasing NN above 1
** (to 2 or 3) gives a modest improvement in SELECT and DELETE performance
** in exchange for a larger degradation in INSERT and UPDATE performance.
** The value of NN appears to give the best results overall.
**
** (Later:) The description above makes it seem as if these values are
** tunable - as if you could change them and recompile and it would all work.
** But that is unlikely.  NB has been 3 since the inception of SQLite and
** we have never tested any other value.
*/
#define NN 1             /* Number of neighbors on either side of pPage */
#define NB 3             /* (NN*2+1): Total pages involved in the balance */

/*
** A CellArray object contains a cache of pointers and sizes for a
** consecutive sequence of cells that might be held on multiple pages.
**
** The cells in this array are the divider cell or cells from the pParent
** page plus up to three child pages.  There are a total of nCell cells.
**
** pRef is a pointer to one of the pages that contributes cells.  This is
** used to access information such as MemPage.intKey and MemPage.pBt->pageSize
** which should be common to all pages that contribute cells to this array.
**
** apCell[] and szCell[] hold, respectively, pointers to the start of each
** cell and the size of each cell.  Some of the apCell[] pointers might refer
** to overflow cells.  In other words, some apCel[] pointers might not point
** to content area of the pages.
**
** A szCell[] of zero means the size of that cell has not yet been computed.
**
** The cells come from as many as four different pages:
**
**             -----------
**             | Parent  |
**             -----------
**            /     |     \
**           /      |      \
**  ---------   ---------   ---------
**  |Child-1|   |Child-2|   |Child-3|
**  ---------   ---------   ---------
**
** The order of cells is in the array is for an index btree is:
**
**       1.  All cells from Child-1 in order
**       2.  The first divider cell from Parent
**       3.  All cells from Child-2 in order
**       4.  The second divider cell from Parent
**       5.  All cells from Child-3 in order
**
** For a table-btree (with rowids) the items 2 and 4 are empty because
** content exists only in leaves and there are no divider cells.
**
** For an index btree, the apEnd[] array holds pointer to the end of page
** for Child-1, the Parent, Child-2, the Parent (again), and Child-3,
** respectively. The ixNx[] array holds the number of cells contained in
** each of these 5 stages, and all stages to the left.  Hence:
**
**    ixNx[0] = Number of cells in Child-1.
**    ixNx[1] = Number of cells in Child-1 plus 1 for first divider.
**    ixNx[2] = Number of cells in Child-1 and Child-2 + 1 for 1st divider.
**    ixNx[3] = Number of cells in Child-1 and Child-2 + both divider cells
**    ixNx[4] = Total number of cells.
**
** For a table-btree, the concept is similar, except only apEnd[0]..apEnd[2]
** are used and they point to the leaf pages only, and the ixNx value are:
**
**    ixNx[0] = Number of cells in Child-1.
**    ixNx[1] = Number of cells in Child-1 and Child-2.
**    ixNx[2] = Total number of cells.
**
** Sometimes when deleting, a child page can have zero cells.  In those
** cases, ixNx[] entries with higher indexes, and the corresponding apEnd[]
** entries, shift down.  The end result is that each ixNx[] entry should
** be larger than the previous
*/
تعريفنوع بنية CellArray CellArray;
بنية CellArray {
  صحيح nCell;              /* Number of cells in apCell[] */
  MemPage *pRef;          /* Reference page */
  u8 **apCell;            /* All cells begin balanced */
  u16 *szCell;            /* Local size of all cells in apCell[] */
  u8 *apEnd[NB*2];        /* MemPage.aDataEnd values */
  صحيح ixNx[NB*2];         /* Index of at which we move to the next apEnd[] */
};

/*
** Make sure the cell sizes at idx, idx+1, ..., idx+N-1 have been
** computed.
*/
ساكن فراغ populateCellCache(CellArray *p, صحيح idx, صحيح N){
  MemPage *pRef = p->pRef;
  u16 *szCell = p->szCell;
  أكد( idx>=0 && idx+N<=p->nCell );
  طالما( N>0 ){
    أكد( p->apCell[idx]!=0 );
    إذا( szCell[idx]==0 ){
      szCell[idx] = pRef->xCellSize(pRef, p->apCell[idx]);
    }وإلا{
      أكد( CORRUPT_DB ||
              szCell[idx]==pRef->xCellSize(pRef, p->apCell[idx]) );
    }
    idx++;
    N--;
  }
}

/*
** Return the size of the Nth element of the cell array
*/
ساكن SQLITE_NOINLINE u16 computeCellSize(CellArray *p, صحيح N){
  أكد( N>=0 && N<p->nCell );
  أكد( p->szCell[N]==0 );
  p->szCell[N] = p->pRef->xCellSize(p->pRef, p->apCell[N]);
  أرجع p->szCell[N];
}
ساكن u16 cachedCellSize(CellArray *p, صحيح N){
  أكد( N>=0 && N<p->nCell );
  إذا( p->szCell[N] ) أرجع p->szCell[N];
  أرجع computeCellSize(p, N);
}

/*
** Array apCell[] contains pointers to nCell b-tree page cells. The
** szCell[] array contains the size in bytes of each cell. This function
** replaces the current contents of page pPg with the contents of the cell
** array.
**
** Some of the cells in apCell[] may currently be stored in pPg. This
** function works around problems caused by this by making a copy of any
** such cells before overwriting the page data.
**
** The MemPage.nFree field is invalidated by this function. It is the
** responsibility of the caller to set it correctly.
*/
ساكن صحيح rebuildPage(
  CellArray *pCArray,             /* Content to be added to page pPg */
  صحيح iFirst,                     /* First cell in pCArray to use */
  صحيح nCell,                      /* Final number of cells on page */
  MemPage *pPg                    /* The page to be reconstructed */
){
  ثابت صحيح hdr = pPg->hdrOffset;          /* Offset of header on pPg */
  u8 * ثابت aData = pPg->aData;           /* Pointer to data for pPg */
  ثابت صحيح usableSize = pPg->pBt->usableSize;
  u8 * ثابت pEnd = &aData[usableSize];
  صحيح i = iFirst;                 /* Which cell to copy from pCArray*/
  u32 j;                          /* Start of cell content area */
  صحيح iEnd = i+nCell;             /* Loop terminator */
  u8 *pCellptr = pPg->aCellIdx;
  u8 *pTmp = sqlite3PagerTempSpace(pPg->pBt->pPager);
  u8 *pData;
  صحيح k;                          /* Current slot in pCArray->apEnd[] */
  u8 *pSrcEnd;                    /* Current pCArray->apEnd[k] value */

  أكد( nCell>0 );
  أكد( i<iEnd );
  j = get2byte(&aData[hdr+5]);
  إذا( j>(u32)usableSize ){ j = 0; }
  memcpy(&pTmp[j], &aData[j], usableSize - j);

  أكد( pCArray->ixNx[NB*2-1]>i );
  لكل(k=0; pCArray->ixNx[k]<=i; k++){}
  pSrcEnd = pCArray->apEnd[k];

  pData = pEnd;
  طالما( 1/*exit by break*/ ){
    u8 *pCell = pCArray->apCell[i];
    u16 sz = pCArray->szCell[i];
    أكد( sz>0 );
    إذا( SQLITE_WITHIN(pCell,aData+j,pEnd) ){
      إذا( ((uptr)(pCell+sz))>(uptr)pEnd ) أرجع SQLITE_CORRUPT_BKPT;
      pCell = &pTmp[pCell - aData];
    }وإلا إذا( (uptr)(pCell+sz)>(uptr)pSrcEnd
           && (uptr)(pCell)<(uptr)pSrcEnd
    ){
      أرجع SQLITE_CORRUPT_BKPT;
    }

    pData -= sz;
    put2byte(pCellptr, (pData - aData));
    pCellptr += 2;
    إذا( pData < pCellptr ) أرجع SQLITE_CORRUPT_BKPT;
    memmove(pData, pCell, sz);
    أكد( sz==pPg->xCellSize(pPg, pCell) || CORRUPT_DB );
    i++;
    إذا( i>=iEnd ) اكسر;
    إذا( pCArray->ixNx[k]<=i ){
      k++;
      pSrcEnd = pCArray->apEnd[k];
    }
  }

  /* The pPg->nFree field is now set incorrectly. The caller will fix it. */
  أكد( nCell < 10922 );
  pPg->nCell = (u16)nCell;
  pPg->nOverflow = 0;

  put2byte(&aData[hdr+1], 0);
  put2byte(&aData[hdr+3], pPg->nCell);
  put2byte(&aData[hdr+5], pData - aData);
  aData[hdr+7] = 0x00;
  أرجع SQLITE_OK;
}

/*
** The pCArray objects contains pointers to b-tree cells and the cell sizes.
** This function attempts to add the cells stored in the array to page pPg.
** If it cannot (because the page needs to be defragmented before the cells
** will fit), non-zero is returned. Otherwise, if the cells are added
** successfully, zero is returned.
**
** Argument pCellptr points to the first entry in the cell-pointer array
** (part of page pPg) to populate. After cell apCell[0] is written to the
** page body, a 16-bit offset is written to pCellptr. And so on, for each
** cell in the array. It is the responsibility of the caller to ensure
** that it is safe to overwrite this part of the cell-pointer array.
**
** When this function is called, *ppData points to the start of the
** content area on page pPg. If the size of the content area is extended,
** *ppData is updated to point to the new start of the content area
** before returning.
**
** Finally, argument pBegin points to the byte immediately following the
** end of the space required by this page for the cell-pointer area (for
** all cells - not just those inserted by the current call). If the content
** area must be extended to before this point in order to accommodate all
** cells in apCell[], then the cells do not fit and non-zero is returned.
*/
ساكن صحيح page_قىستۇرۇش_Array(
  MemPage *pPg,                   /* Page to add cells to */
  u8 *pBegin,                     /* End of cell-pointer array */
  u8 **ppData,                    /* IN/OUT: Page content-area pointer */
  u8 *pCellptr,                   /* Pointer to cell-pointer area */
  صحيح iFirst,                     /* Index of first cell to add */
  صحيح nCell,                      /* Number of cells to add to pPg */
  CellArray *pCArray              /* Array of cells */
){
  صحيح i = iFirst;                 /* Loop counter - cell index to insert */
  u8 *aData = pPg->aData;         /* Complete page */
  u8 *pData = *ppData;            /* Content area.  A subset of aData[] */
  صحيح iEnd = iFirst + nCell;      /* End of loop. One past last cell to ins */
  صحيح k;                          /* Current slot in pCArray->apEnd[] */
  u8 *pEnd;                       /* Maximum extent of cell data */
  أكد( CORRUPT_DB || pPg->hdrOffset==0 );    /* Never called on page 1 */
  إذا( iEnd<=iFirst ) أرجع 0;
  أكد( pCArray->ixNx[NB*2-1]>i );
  لكل(k=0; pCArray->ixNx[k]<=i ; k++){}
  pEnd = pCArray->apEnd[k];
  طالما( 1 /*Exit by break*/ ){
    صحيح sz, rc;
    u8 *pSlot;
    أكد( pCArray->szCell[i]!=0 );
    sz = pCArray->szCell[i];
    إذا( (aData[1]==0 && aData[2]==0) || (pSlot = pageFindSlot(pPg,sz,&rc))==0 ){
      إذا( (pData - pBegin)<sz ) أرجع 1;
      pData -= sz;
      pSlot = pData;
    }
    /* pSlot and pCArray->apCell[i] will never overlap on a well-formed
    ** database.  But they might for a corrupt database.  Hence use memmove()
    ** since memcpy() sends SIGABORT with overlapping buffers on OpenBSD */
    أكد( (pSlot+sz)<=pCArray->apCell[i]
         || pSlot>=(pCArray->apCell[i]+sz)
         || CORRUPT_DB );
    إذا( (uptr)(pCArray->apCell[i]+sz)>(uptr)pEnd
     && (uptr)(pCArray->apCell[i])<(uptr)pEnd
    ){
      أكد( CORRUPT_DB );
      (فراغ)SQLITE_CORRUPT_BKPT;
      أرجع 1;
    }
    memmove(pSlot, pCArray->apCell[i], sz);
    put2byte(pCellptr, (pSlot - aData));
    pCellptr += 2;
    i++;
    إذا( i>=iEnd ) اكسر;
    إذا( pCArray->ixNx[k]<=i ){
      k++;
      pEnd = pCArray->apEnd[k];
    }
  }
  *ppData = pData;
  أرجع 0;
}

/*
** The pCArray object contains pointers to b-tree cells and their sizes.
**
** This function adds the space associated with each cell in the array
** that is currently stored within the body of pPg to the pPg free-list.
** The cell-pointers and other fields of the page are not updated.
**
** This function returns the total number of cells added to the free-list.
*/
ساكن صحيح pageFreeArray(
  MemPage *pPg,                   /* Page to edit */
  صحيح iFirst,                     /* First cell to delete */
  صحيح nCell,                      /* Cells to delete */
  CellArray *pCArray              /* Array of cells */
){
  u8 * ثابت aData = pPg->aData;
  u8 * ثابت pEnd = &aData[pPg->pBt->usableSize];
  u8 * ثابت pStart = &aData[pPg->hdrOffset + 8 + pPg->childPtrSize];
  صحيح nRet = 0;
  صحيح i, j;
  صحيح iEnd = iFirst + nCell;
  صحيح nFree = 0;
  صحيح aOfst[10];
  صحيح aAfter[10];

  لكل(i=iFirst; i<iEnd; i++){
    u8 *pCell = pCArray->apCell[i];
    إذا( SQLITE_WITHIN(pCell, pStart, pEnd) ){
      صحيح sz;
      صحيح iAfter;
      صحيح iOfst;
      /* No need to use cachedCellSize() here.  The sizes of all cells that
      ** are to be freed have already been computing while deciding which
      ** cells need freeing */
      sz = pCArray->szCell[i];  أكد( sz>0 );
      iOfst = (u16)(pCell - aData);
      iAfter = iOfst+sz;
      لكل(j=0; j<nFree; j++){
        إذا( aOfst[j]==iAfter ){
          aOfst[j] = iOfst;
          اكسر;
        }وإلا إذا( aAfter[j]==iOfst ){
          aAfter[j] = iAfter;
          اكسر;
        }
      }
      إذا( j>=nFree ){
        إذا( nFree>=(صحيح)(حجم(aOfst)/حجم(aOfst[0])) ){
          لكل(j=0; j<nFree; j++){
            freeSpace(pPg, aOfst[j], aAfter[j]-aOfst[j]);
          }
          nFree = 0;
        }
        aOfst[nFree] = iOfst;
        aAfter[nFree] = iAfter;
        إذا( &aData[iAfter]>pEnd ) أرجع 0;
        nFree++;
      }
      nRet++;
    }
  }
  لكل(j=0; j<nFree; j++){
    freeSpace(pPg, aOfst[j], aAfter[j]-aOfst[j]);
  }
  أرجع nRet;
}

/*
** pCArray contains pointers to and sizes of all cells in the page being
** balanced.  The current page, pPg, has pPg->nCell cells starting with
** pCArray->apCell[iOld].  After balancing, this page should hold nNew cells
** starting at apCell[iNew].
**
** This routine makes the necessary adjustments to pPg so that it contains
** the correct cells after being balanced.
**
** The pPg->nFree field is invalid when this function returns. It is the
** responsibility of the caller to set it correctly.
*/
ساكن صحيح editPage(
  MemPage *pPg,                   /* Edit this page */
  صحيح iOld,                       /* Index of first cell currently on page */
  صحيح iNew,                       /* Index of new first cell on page */
  صحيح nNew,                       /* Final number of cells on page */
  CellArray *pCArray              /* Array of cells and sizes */
){
  u8 * ثابت aData = pPg->aData;
  ثابت صحيح hdr = pPg->hdrOffset;
  u8 *pBegin = &pPg->aCellIdx[nNew * 2];
  صحيح nCell = pPg->nCell;       /* Cells stored on pPg */
  u8 *pData;
  u8 *pCellptr;
  صحيح i;
  صحيح iOldEnd = iOld + pPg->nCell + pPg->nOverflow;
  صحيح iNewEnd = iNew + nNew;

#ifdef SQLITE_DEBUG
  u8 *pTmp = sqlite3PagerTempSpace(pPg->pBt->pPager);
  memcpy(pTmp, aData, pPg->pBt->usableSize);
#endif

  /* Remove cells from the start and end of the page */
  أكد( nCell>=0 );
  إذا( iOld<iNew ){
    صحيح nShift = pageFreeArray(pPg, iOld, iNew-iOld, pCArray);
    إذا( NEVER(nShift>nCell) ) أرجع SQLITE_CORRUPT_BKPT;
    memmove(pPg->aCellIdx, &pPg->aCellIdx[nShift*2], nCell*2);
    nCell -= nShift;
  }
  إذا( iNewEnd < iOldEnd ){
    صحيح nTail = pageFreeArray(pPg, iNewEnd, iOldEnd - iNewEnd, pCArray);
    أكد( nCell>=nTail );
    nCell -= nTail;
  }

  pData = &aData[get2byte(&aData[hdr+5])];
  إذا( pData<pBegin ) اذهب editpage_fail;
  إذا( NEVER(pData>pPg->aDataEnd) ) اذهب editpage_fail;

  /* Add cells to the start of the page */
  إذا( iNew<iOld ){
    صحيح nAdd = MIN(nNew,iOld-iNew);
    أكد( (iOld-iNew)<nNew || nCell==0 || CORRUPT_DB );
    أكد( nAdd>=0 );
    pCellptr = pPg->aCellIdx;
    memmove(&pCellptr[nAdd*2], pCellptr, nCell*2);
    إذا( page_قىستۇرۇش_Array(
          pPg, pBegin, &pData, pCellptr,
          iNew, nAdd, pCArray
    ) ) اذهب editpage_fail;
    nCell += nAdd;
  }

  /* Add any overflow cells */
  لكل(i=0; i<pPg->nOverflow; i++){
    صحيح iCell = (iOld + pPg->aiOvfl[i]) - iNew;
    إذا( iCell>=0 && iCell<nNew ){
      pCellptr = &pPg->aCellIdx[iCell * 2];
      إذا( nCell>iCell ){
        memmove(&pCellptr[2], pCellptr, (nCell - iCell) * 2);
      }
      nCell++;
      cachedCellSize(pCArray, iCell+iNew);
      إذا( page_قىستۇرۇش_Array(
            pPg, pBegin, &pData, pCellptr,
            iCell+iNew, 1, pCArray
      ) ) اذهب editpage_fail;
    }
  }

  /* Append cells to the end of the page */
  أكد( nCell>=0 );
  pCellptr = &pPg->aCellIdx[nCell*2];
  إذا( page_قىستۇرۇش_Array(
        pPg, pBegin, &pData, pCellptr,
        iNew+nCell, nNew-nCell, pCArray
      )
  ){
    اذهب editpage_fail;
  }

  أكد( nNew < 10922 );
  pPg->nCell = (u16)nNew;
  pPg->nOverflow = 0;

  put2byte(&aData[hdr+3], pPg->nCell);
  put2byte(&aData[hdr+5], pData - aData);

#ifdef SQLITE_DEBUG
  لكل(i=0; i<nNew && !CORRUPT_DB; i++){
    u8 *pCell = pCArray->apCell[i+iNew];
    صحيح iOff = get2byteAligned(&pPg->aCellIdx[i*2]);
    إذا( SQLITE_WITHIN(pCell, aData, &aData[pPg->pBt->usableSize]) ){
      pCell = &pTmp[pCell - aData];
    }
    أكد( 0==memcmp(pCell, &aData[iOff],
            pCArray->pRef->xCellSize(pCArray->pRef, pCArray->apCell[i+iNew])) );
  }
#endif

  أرجع SQLITE_OK;
 editpage_fail:
  /* Unable to edit this page. Rebuild it from scratch instead. */
  إذا( nNew<1 ) أرجع SQLITE_CORRUPT_BKPT;
  populateCellCache(pCArray, iNew, nNew);
  أرجع rebuildPage(pCArray, iNew, nNew, pPg);
}


#ifndef SQLITE_OMIT_QUICKBALANCE
/*
** This version of balance() handles the common special case where
** a new entry is being inserted on the extreme right-end of the
** tree, in other words, when the new entry will become the largest
** entry in the tree.
**
** Instead of trying to balance the 3 right-most leaf pages, just add
** a new page to the right-hand side and put the one new entry in
** that page.  This leaves the right side of the tree somewhat
** unbalanced.  But odds are that we will be inserting new entries
** at the end soon afterwards so the nearly empty page will quickly
** fill up.  On average.
**
** pPage is the leaf page which is the right-most page in the tree.
** pParent is its parent.  pPage must have a single overflow entry
** which is also the right-most entry on the page.
**
** The pSpace buffer is used to store a temporary copy of the divider
** cell that will be inserted into pParent. Such a cell consists of a 4
** byte page number followed by a variable length integer. In other
** words, at most 13 bytes. Hence the pSpace buffer must be at
** least 13 bytes in size.
*/
ساكن صحيح balance_quick(MemPage *pParent, MemPage *pPage, u8 *pSpace){
  BtShared *ثابت pBt = pPage->pBt;    /* B-Tree Database */
  MemPage *pNew;                       /* Newly allocated page */
  صحيح rc;                              /* Return Code */
  Pgno pgnoNew;                        /* Page number of pNew */

  أكد( sqlite3_mutex_held(pPage->pBt->mutex) );
  أكد( sqlite3PagerIswriteable(pParent->pDbPage) );
  أكد( pPage->nOverflow==1 );
 
  إذا( pPage->nCell==0 ) أرجع SQLITE_CORRUPT_BKPT;  /* dbfuzz001.test */
  أكد( pPage->nFree>=0 );
  أكد( pParent->nFree>=0 );

  /* Allocate a new page. This page will become the right-sibling of
  ** pPage. Make the parent page writable, so that the new divider cell
  ** may be inserted. If both these operations are successful, proceed.
  */
  rc = allocateBtreePage(pBt, &pNew, &pgnoNew, 0, 0);

  إذا( rc==SQLITE_OK ){

    u8 *pOut = &pSpace[4];
    u8 *pCell = pPage->apOvfl[0];
    u16 szCell = pPage->xCellSize(pPage, pCell);
    u8 *pStop;
    CellArray b;

    أكد( sqlite3PagerIswriteable(pNew->pDbPage) );
    أكد( CORRUPT_DB || pPage->aData[0]==(PTF_INTKEY|PTF_LEAFDATA|PTF_LEAF) );
    zeroPage(pNew, PTF_INTKEY|PTF_LEAFDATA|PTF_LEAF);
    b.nCell = 1;
    b.pRef = pPage;
    b.apCell = &pCell;
    b.szCell = &szCell;
    b.apEnd[0] = pPage->aDataEnd;
    b.ixNx[0] = 2;
    b.ixNx[NB*2-1] = 0x7fffffff;
    rc = rebuildPage(&b, 0, 1, pNew);
    إذا( NEVER(rc) ){
      releasePage(pNew);
      أرجع rc;
    }
    pNew->nFree = pBt->usableSize - pNew->cellOffset - 2 - szCell;

    /* If this is an auto-vacuum database, update the pointer map
    ** with entries for the new page, and any pointer from the
    ** cell on the page to an overflow page. If either of these
    ** operations fails, the return code is set, but the contents
    ** of the parent page are still manipulated by the code below.
    ** That is Ok, at this point the parent page is guaranteed to
    ** be marked as dirty. Returning an error code will cause a
    ** rollback, undoing any changes made to the parent page.
    */
    إذا( ISAUTOVACUUM(pBt) ){
      ptrmapPut(pBt, pgnoNew, PTRMAP_BTREE, pParent->pgno, &rc);
      إذا( szCell>pNew->minLocal ){
        ptrmapPutOvflPtr(pNew, pNew, pCell, &rc);
      }
    }
 
    /* Create a divider cell to insert into pParent. The divider cell
    ** consists of a 4-byte page number (the page number of pPage) and
    ** a variable length key value (which must be the same value as the
    ** largest key on pPage).
    **
    ** To find the largest key value on pPage, first find the right-most
    ** cell on pPage. The first two fields of this cell are the
    ** record-length (a variable length integer at most 32-bits in size)
    ** and the key value (a variable length integer, may have any value).
    ** The first of the while(...) loops below skips over the record-length
    ** field. The second while(...) loop copies the key value from the
    ** cell on pPage into the pSpace buffer.
    */
    pCell = findCell(pPage, pPage->nCell-1);
    pStop = &pCell[9];
    طالما( (*(pCell++)&0x80) && pCell<pStop );
    pStop = &pCell[9];
    طالما( ((*(pOut++) = *(pCell++))&0x80) && pCell<pStop );

    /* Insert the new divider cell into pParent. */
    إذا( rc==SQLITE_OK ){
      rc = قىستۇرۇش_Cell(pParent, pParent->nCell, pSpace, (صحيح)(pOut-pSpace),
                      0, pPage->pgno);
    }

    /* Set the right-child pointer of pParent to point to the new page. */
    put4byte(&pParent->aData[pParent->hdrOffset+8], pgnoNew);
 
    /* Release the reference to the new page. */
    releasePage(pNew);
  }

  أرجع rc;
}
#endif /* SQLITE_OMIT_QUICKBALANCE */

#إذا 0
/*
** This function does not contribute anything to the operation of SQLite.
** it is sometimes activated temporarily while debugging code responsible
** for setting pointer-map entries.
*/
ساكن صحيح ptrmapCheckPages(MemPage **apPage, صحيح nPage){
  صحيح i, j;
  لكل(i=0; i<nPage; i++){
    Pgno n;
    u8 e;
    MemPage *pPage = apPage[i];
    BtShared *pBt = pPage->pBt;
    أكد( pPage->isInit );

    لكل(j=0; j<pPage->nCell; j++){
      CellInfo info;
      u8 *z;
    
      z = findCell(pPage, j);
      pPage->xParseCell(pPage, z, &info);
      إذا( info.nLocal<info.nPayload ){
        Pgno ovfl = get4byte(&z[info.nSize-4]);
        ptrmapGet(pBt, ovfl, &e, &n);
        أكد( n==pPage->pgno && e==PTRMAP_OVERFLOW1 );
      }
      إذا( !pPage->leaf ){
        Pgno child = get4byte(z);
        ptrmapGet(pBt, child, &e, &n);
        أكد( n==pPage->pgno && e==PTRMAP_BTREE );
      }
    }
    إذا( !pPage->leaf ){
      Pgno child = get4byte(&pPage->aData[pPage->hdrOffset+8]);
      ptrmapGet(pBt, child, &e, &n);
      أكد( n==pPage->pgno && e==PTRMAP_BTREE );
    }
  }
  أرجع 1;
}
#endif

/*
** This function is used to copy the contents of the b-tree node stored
** on page pFrom to page pTo. If page pFrom was not a leaf page, then
** the pointer-map entries for each child page are updated so that the
** parent page stored in the pointer map is page pTo. If pFrom contained
** any cells with overflow page pointers, then the corresponding pointer
** map entries are also updated so that the parent page is page pTo.
**
** If pFrom is currently carrying any overflow cells (entries in the
** MemPage.apOvfl[] array), they are not copied to pTo.
**
** Before returning, page pTo is reinitialized using btreeInitPage().
**
** The performance of this function is not critical. It is only used by
** the balance_shallower() and balance_deeper() procedures, neither of
** which are called often under normal circumstances.
*/
ساكن فراغ copy_تۈگۈن_Content(MemPage *pFrom, MemPage *pTo, صحيح *pRC){
  إذا( (*pRC)==SQLITE_OK ){
    BtShared * ثابت pBt = pFrom->pBt;
    u8 * ثابت aFrom = pFrom->aData;
    u8 * ثابت aTo = pTo->aData;
    صحيح ثابت iFromHdr = pFrom->hdrOffset;
    صحيح ثابت iToHdr = ((pTo->pgno==1) ? 100 : 0);
    صحيح rc;
    صحيح iData;
 
 
    أكد( pFrom->isInit );
    أكد( pFrom->nFree>=iToHdr );
    أكد( get2byte(&aFrom[iFromHdr+5]) <= (صحيح)pBt->usableSize );
 
    /* Copy the b-tree node content from page pFrom to page pTo. */
    iData = get2byte(&aFrom[iFromHdr+5]);
    memcpy(&aTo[iData], &aFrom[iData], pBt->usableSize-iData);
    memcpy(&aTo[iToHdr], &aFrom[iFromHdr], pFrom->cellOffset + 2*pFrom->nCell);
 
    /* Reinitialize page pTo so that the contents of the MemPage structure
    ** match the new data. The initialization of pTo can actually fail under
    ** fairly obscure circumstances, even though it is a copy of initialized
    ** page pFrom.
    */
    pTo->isInit = 0;
    rc = btreeInitPage(pTo);
    إذا( rc==SQLITE_OK ) rc = btreeComputeFreeSpace(pTo);
    إذا( rc!=SQLITE_OK ){
      *pRC = rc;
      أرجع;
    }
 
    /* If this is an auto-vacuum database, update the pointer-map entries
    ** for any b-tree or overflow pages that pTo now contains the pointers to.
    */
    إذا( ISAUTOVACUUM(pBt) ){
      *pRC = setChildPtrmaps(pTo);
    }
  }
}

/*
** This routine redistributes cells on the iParentIdx'th child of pParent
** (hereafter "the page") and up to 2 siblings so that all pages have about the
** same amount of free space. Usually a single sibling on either side of the
** page are used in the balancing, though both siblings might come from one
** side if the page is the first or last child of its parent. If the page
** has fewer than 2 siblings (something which can only happen if the page
** is a root page or a child of a root page) then all available siblings
** participate in the balancing.
**
** The number of siblings of the page might be increased or decreased by
** one or two in an effort to keep pages nearly full but not over full.
**
** Note that when this routine is called, some of the cells on the page
** might not actually be stored in MemPage.aData[]. This can happen
** if the page is overfull. This routine ensures that all cells allocated
** to the page and its siblings fit into MemPage.aData[] before returning.
**
** In the course of balancing the page and its siblings, cells may be
** inserted into or removed from the parent page (pParent). Doing so
** may cause the parent page to become overfull or underfull. If this
** happens, it is the responsibility of the caller to invoke the correct
** balancing routine to fix this problem (see the balance() routine).
**
** If this routine fails for any reason, it might leave the database
** in a corrupted state. So if this routine fails, the database should
** be rolled back.
**
** The third argument to this function, aOvflSpace, is a pointer to a
** buffer big enough to hold one page. If while inserting cells into the parent
** page (pParent) the parent page becomes overfull, this buffer is
** used to store the parent's overflow cells. Because this function inserts
** a maximum of four divider cells into the parent page, and the maximum
** size of a cell stored within an internal node is always less than 1/4
** of the page-size, the aOvflSpace[] buffer is guaranteed to be large
** enough for all overflow cells.
**
** If aOvflSpace is set to a null pointer, this function returns
** SQLITE_NOMEM.
*/
ساكن صحيح balance_nonroot(
  MemPage *pParent,               /* Parent page of siblings being balanced */
  صحيح iParentIdx,                 /* Index of "the page" in pParent */
  u8 *aOvflSpace,                 /* page-size bytes of space for parent ovfl */
  صحيح is_يىلتىز,                     /* True if pParent is a root-page */
  صحيح bBulk                       /* True if this call is part of a bulk load */
){
  BtShared *pBt;               /* The whole database */
  صحيح nMaxCells = 0;           /* Allocated size of apCell, szCell, aFrom. */
  صحيح nNew = 0;                /* Number of pages in apNew[] */
  صحيح nOld;                    /* Number of pages in apOld[] */
  صحيح i, j, k;                 /* Loop counters */
  صحيح nxDiv;                   /* Next divider slot in pParent->aCell[] */
  صحيح rc = SQLITE_OK;          /* The return code */
  u16 leafCorrection;          /* 4 if pPage is a leaf.  0 if not */
  صحيح leafData;                /* True if pPage is a leaf of a LEAFDATA tree */
  صحيح usableSpace;             /* Bytes in pPage beyond the header */
  صحيح pageFlags;               /* Value of pPage->aData[0] */
  صحيح iSpace1 = 0;             /* First unused byte of aSpace1[] */
  صحيح iOvflSpace = 0;          /* First unused byte of aOvflSpace[] */
  u64 szScratch;               /* Size of scratch memory requested */
  MemPage *apOld[NB];          /* pPage and up to two siblings */
  MemPage *apNew[NB+2];        /* pPage and up to NB siblings after balancing */
  u8 *p_ئوڭ;                  /* Location in parent of right-sibling pointer */
  u8 *apDiv[NB-1];             /* Divider cells in pParent */
  صحيح cntNew[NB+2];            /* Index in b.paCell[] of cell after i-th page */
  صحيح cntOld[NB+2];            /* Old index in b.apCell[] */
  صحيح szNew[NB+2];             /* Combined size of cells placed on i-th page */
  u8 *aSpace1;                 /* Space for copies of dividers cells */
  Pgno pgno;                   /* Temp var to store a page number in */
  u8 abDone[NB+2];             /* True after i'th new page is populated */
  Pgno aPgno[NB+2];            /* Page numbers of new pages before shuffling */
  CellArray b;                 /* Parsed information on cells being balanced */

  memset(abDone, 0, حجم(abDone));
  أكد( حجم(b) - حجم(b.ixNx) == offsetof(CellArray,ixNx) );
  memset(&b, 0, حجم(b)-حجم(b.ixNx[0]));
  b.ixNx[NB*2-1] = 0x7fffffff;
  pBt = pParent->pBt;
  أكد( sqlite3_mutex_held(pBt->mutex) );
  أكد( sqlite3PagerIswriteable(pParent->pDbPage) );

  /* At this point pParent may have at most one overflow cell. And if
  ** this overflow cell is present, it must be the cell with
  ** index iParentIdx. This scenario comes about when this function
  ** is called (indirectly) from sqlite3BtreeDelete().
  */
  أكد( pParent->nOverflow==0 || pParent->nOverflow==1 );
  أكد( pParent->nOverflow==0 || pParent->aiOvfl[0]==iParentIdx );

  إذا( !aOvflSpace ){
    أرجع SQLITE_NOMEM_BKPT;
  }
  أكد( pParent->nFree>=0 );

  /* Find the sibling pages to balance. Also locate the cells in pParent
  ** that divide the siblings. An attempt is made to find NN siblings on
  ** either side of pPage. More siblings are taken from one side, however,
  ** if there are fewer than NN siblings on the other side. If pParent
  ** has NB or fewer children then all children of pParent are taken. 
  **
  ** This loop also drops the divider cells from the parent page. This
  ** way, the remainder of the function does not have to deal with any
  ** overflow cells in the parent page, since if any existed they will
  ** have already been removed.
  */
  i = pParent->nOverflow + pParent->nCell;
  إذا( i<2 ){
    nxDiv = 0;
  }وإلا{
    أكد( bBulk==0 || bBulk==1 );
    إذا( iParentIdx==0 ){                
      nxDiv = 0;
    }وإلا إذا( iParentIdx==i ){
      nxDiv = i-2+bBulk;
    }وإلا{
      nxDiv = iParentIdx-1;
    }
    i = 2-bBulk;
  }
  nOld = i+1;
  إذا( (i+nxDiv-pParent->nOverflow)==pParent->nCell ){
    p_ئوڭ = &pParent->aData[pParent->hdrOffset+8];
  }وإلا{
    p_ئوڭ = findCell(pParent, i+nxDiv-pParent->nOverflow);
  }
  pgno = get4byte(p_ئوڭ);
  طالما( 1 ){
    إذا( rc==SQLITE_OK ){
      rc = getAndInitPage(pBt, pgno, &apOld[i], 0);
    }
    إذا( rc ){
      memset(apOld, 0, (i+1)*حجم(MemPage*));
      اذهب balance_cleanup;
    }
    إذا( apOld[i]->nFree<0 ){
      rc = btreeComputeFreeSpace(apOld[i]);
      إذا( rc ){
        memset(apOld, 0, (i)*حجم(MemPage*));
        اذهب balance_cleanup;
      }
    }
    nMaxCells += apOld[i]->nCell + ArraySize(pParent->apOvfl);
    إذا( (i--)==0 ) اكسر;

    إذا( pParent->nOverflow && i+nxDiv==pParent->aiOvfl[0] ){
      apDiv[i] = pParent->apOvfl[0];
      pgno = get4byte(apDiv[i]);
      szNew[i] = pParent->xCellSize(pParent, apDiv[i]);
      pParent->nOverflow = 0;
    }وإلا{
      apDiv[i] = findCell(pParent, i+nxDiv-pParent->nOverflow);
      pgno = get4byte(apDiv[i]);
      szNew[i] = pParent->xCellSize(pParent, apDiv[i]);

      /* Drop the cell from the parent page. apDiv[i] still points to
      ** the cell within the parent, even though it has been dropped.
      ** This is safe because dropping a cell only overwrites the first
      ** four bytes of it, and this function does not need the first
      ** four bytes of the divider cell. So the pointer is safe to use
      ** later on. 
      **
      ** But not if we are in secure-delete mode. In secure-delete mode,
      ** the dropCell() routine will overwrite the entire cell with zeroes.
      ** In this case, temporarily copy the cell into the aOvflSpace[]
      ** buffer. It will be copied out again as soon as the aSpace[] buffer
      ** is allocated.  */
      إذا( pBt->btsFlags & BTS_FAST_SECURE ){
        صحيح iOff;

        /* If the following if() condition is not true, the db is corrupted.
        ** The call to dropCell() below will detect this.  */
        iOff = SQLITE_PTR_TO_INT(apDiv[i]) - SQLITE_PTR_TO_INT(pParent->aData);
        إذا( (iOff+szNew[i])<=(صحيح)pBt->usableSize ){
          memcpy(&aOvflSpace[iOff], apDiv[i], szNew[i]);
          apDiv[i] = &aOvflSpace[apDiv[i]-pParent->aData];
        }
      }
      dropCell(pParent, i+nxDiv-pParent->nOverflow, szNew[i], &rc);
    }
  }

  /* Make nMaxCells a multiple of 4 in order to preserve 8-byte
  ** alignment */
  nMaxCells = (nMaxCells + 3)&~3;

  /*
  ** Allocate space for memory structures
  */
  szScratch =
       nMaxCells*حجم(u8*)                       /* b.apCell */
     + nMaxCells*حجم(u16)                       /* b.szCell */
     + pBt->pageSize;                              /* aSpace1 */

  أكد( szScratch<=7*(صحيح)pBt->pageSize );
  b.apCell = sqlite3_دۆۋە_Alloc_Raw(0, szScratch );
  إذا( b.apCell==0 ){
    rc = SQLITE_NOMEM_BKPT;
    اذهب balance_cleanup;
  }
  b.szCell = (u16*)&b.apCell[nMaxCells];
  aSpace1 = (u8*)&b.szCell[nMaxCells];
  أكد( EIGHT_BYTE_ALIGNMENT(aSpace1) );

  /*
  ** Load pointers to all cells on sibling pages and the divider cells
  ** into the local b.apCell[] array.  Make copies of the divider cells
  ** into space obtained from aSpace1[]. The divider cells have already
  ** been removed from pParent.
  **
  ** If the siblings are on leaf pages, then the child pointers of the
  ** divider cells are stripped from the cells before they are copied
  ** into aSpace1[].  In this way, all cells in b.apCell[] are without
  ** child pointers.  If siblings are not leaves, then all cell in
  ** b.apCell[] include child pointers.  Either way, all cells in b.apCell[]
  ** are alike.
  **
  ** leafCorrection:  4 if pPage is a leaf.  0 if pPage is not a leaf.
  **       leafData:  1 if pPage holds key+data and pParent holds only keys.
  */
  b.pRef = apOld[0];
  leafCorrection = b.pRef->leaf*4;
  leafData = b.pRef->int_ئاچقۇچ_Leaf;
  لكل(i=0; i<nOld; i++){
    MemPage *pOld = apOld[i];
    صحيح limit = pOld->nCell;
    u8 *aData = pOld->aData;
    u16 maskPage = pOld->maskPage;
    u8 *piCell = aData + pOld->cellOffset;
    u8 *piEnd;
    u8 **ppCell;
    VVA_ONLY( صحيح nCellAtStart = b.nCell; )

    /* Verify that all sibling pages are of the same "type" (table-leaf,
    ** table-interior, index-leaf, or index-interior).
    */
    إذا( pOld->aData[0]!=apOld[0]->aData[0] ){
      rc = SQLITE_CORRUPT_PAGE(pOld);
      اذهب balance_cleanup;
    }

    /* Load b.apCell[] with pointers to all cells in pOld.  If pOld
    ** contains overflow cells, include them in the b.apCell[] array
    ** in the correct spot.
    **
    ** Note that when there are multiple overflow cells, it is always the
    ** case that they are sequential and adjacent.  This invariant arises
    ** because multiple overflows can only occurs when inserting divider
    ** cells into a parent on a prior balance, and divider cells are always
    ** adjacent and are inserted in order.  There is an assert() tagged
    ** with "NOTE 1" in the overflow cell insertion loop to prove this
    ** invariant.
    **
    ** This must be done in advance.  Once the balance starts, the cell
    ** offset section of the btree page will be overwritten and we will no
    ** long be able to find the cells if a pointer to each cell is not saved
    ** first.
    */
    memset(&b.szCell[b.nCell], 0, حجم(b.szCell[0])*(limit+pOld->nOverflow));
    إذا( pOld->nOverflow>0 ){
      إذا( NEVER(limit<pOld->aiOvfl[0]) ){
        rc = SQLITE_CORRUPT_PAGE(pOld);
        اذهب balance_cleanup;
      }
      limit = pOld->aiOvfl[0];
      لكل(j=0; j<limit; j++){
        b.apCell[b.nCell+j] = aData + (maskPage & get2byteAligned(piCell));
        piCell += 2;
      }
      b.nCell += limit;
      لكل(k=0; k<pOld->nOverflow; k++){
        أكد( k==0 || pOld->aiOvfl[k-1]+1==pOld->aiOvfl[k] );/* NOTE 1 */
        b.apCell[b.nCell] = pOld->apOvfl[k];
        b.nCell++;
      }
    }
    piEnd = aData + pOld->cellOffset + 2*pOld->nCell;
    /* Collect cell pointers before updating the cell count. */
    ppCell = &b.apCell[b.nCell];
    طالما( piCell<piEnd ){
      أكد( ppCell<&b.apCell[nMaxCells] );
      *ppCell++ = aData + (maskPage & get2byteAligned(piCell));
      piCell += 2;
    }
    b.nCell = (صحيح)(ppCell - b.apCell);
    أكد( (b.nCell-nCellAtStart)==(pOld->nCell+pOld->nOverflow) );

    cntOld[i] = b.nCell;
    إذا( i<nOld-1 && !leafData){
      u16 sz = (u16)szNew[i];
      u8 *pTemp;
      أكد( b.nCell<nMaxCells );
      b.szCell[b.nCell] = sz;
      pTemp = &aSpace1[iSpace1];
      iSpace1 += sz;
      أكد( sz<=pBt->maxLocal+23 );
      أكد( iSpace1 <= (صحيح)pBt->pageSize );
      memcpy(pTemp, apDiv[i], sz);
      b.apCell[b.nCell] = pTemp+leafCorrection;
      أكد( leafCorrection==0 || leafCorrection==4 );
      b.szCell[b.nCell] = b.szCell[b.nCell] - leafCorrection;
      إذا( !pOld->leaf ){
        أكد( leafCorrection==0 );
        أكد( pOld->hdrOffset==0 || CORRUPT_DB );
        /* The right pointer of the child page pOld becomes the left
        ** pointer of the divider cell */
        memcpy(b.apCell[b.nCell], &pOld->aData[8], 4);
      }وإلا{
        أكد( leafCorrection==4 );
        طالما( b.szCell[b.nCell]<4 ){
          /* Do not allow any cells smaller than 4 bytes. If a smaller cell
          ** does exist, pad it with 0x00 bytes. */
          أكد( b.szCell[b.nCell]==3 || CORRUPT_DB );
          أكد( b.apCell[b.nCell]==&aSpace1[iSpace1-3] || CORRUPT_DB );
          aSpace1[iSpace1++] = 0x00;
          b.szCell[b.nCell]++;
        }
      }
      b.nCell++;
    }
  }

  /*
  ** Figure out the number of pages needed to hold all b.nCell cells.
  ** Store this number in "k".  Also compute szNew[] which is the total
  ** size of all cells on the i-th page and cntNew[] which is the index
  ** in b.apCell[] of the cell that divides page i from page i+1. 
  ** cntNew[k] should equal b.nCell.
  **
  ** Values computed by this block:
  **
  **           k: The total number of sibling pages
  **    szNew[i]: Spaced used on the i-th sibling page.
  **   cntNew[i]: Index in b.apCell[] and b.szCell[] for the first cell to
  **              the right of the i-th sibling page.
  ** usableSpace: Number of bytes of space available on each sibling.
  **
  */
  usableSpace = pBt->usableSize - 12 + leafCorrection;
  لكل(i=k=0; i<nOld; i++, k++){
    MemPage *p = apOld[i];
    b.apEnd[k] = p->aDataEnd;
    b.ixNx[k] = cntOld[i];
    إذا( k && b.ixNx[k]==b.ixNx[k-1] ){
      k--;  /* Omit b.ixNx[] entry for child pages with no cells */
    }
    إذا( !leafData ){
      k++;
      b.apEnd[k] = pParent->aDataEnd;
      b.ixNx[k] = cntOld[i]+1;
    }
    أكد( p->nFree>=0 );
    szNew[i] = usableSpace - p->nFree;
    لكل(j=0; j<p->nOverflow; j++){
      szNew[i] += 2 + p->xCellSize(p, p->apOvfl[j]);
    }
    cntNew[i] = cntOld[i];
  }
  k = nOld;
  لكل(i=0; i<k; i++){
    صحيح sz;
    طالما( szNew[i]>usableSpace ){
      إذا( i+1>=k ){
        k = i+2;
        إذا( k>NB+2 ){ rc = SQLITE_CORRUPT_BKPT; اذهب balance_cleanup; }
        szNew[k-1] = 0;
        cntNew[k-1] = b.nCell;
      }
      sz = 2 + cachedCellSize(&b, cntNew[i]-1);
      szNew[i] -= sz;
      إذا( !leafData ){
        إذا( cntNew[i]<b.nCell ){
          sz = 2 + cachedCellSize(&b, cntNew[i]);
        }وإلا{
          sz = 0;
        }
      }
      szNew[i+1] += sz;
      cntNew[i]--;
    }
    طالما( cntNew[i]<b.nCell ){
      sz = 2 + cachedCellSize(&b, cntNew[i]);
      إذا( szNew[i]+sz>usableSpace ) اكسر;
      szNew[i] += sz;
      cntNew[i]++;
      إذا( !leafData ){
        إذا( cntNew[i]<b.nCell ){
          sz = 2 + cachedCellSize(&b, cntNew[i]);
        }وإلا{
          sz = 0;
        }
      }
      szNew[i+1] -= sz;
    }
    إذا( cntNew[i]>=b.nCell ){
      k = i+1;
    }وإلا إذا( cntNew[i] <= (i>0 ? cntNew[i-1] : 0) ){
      rc = SQLITE_CORRUPT_BKPT;
      اذهب balance_cleanup;
    }
  }

  /*
  ** The packing computed by the previous block is biased toward the siblings
  ** on the left side (siblings with smaller keys). The left siblings are
  ** always nearly full, while the right-most sibling might be nearly empty.
  ** The next block of code attempts to adjust the packing of siblings to
  ** get a better balance.
  **
  ** This adjustment is more than an optimization.  The packing above might
  ** be so out of balance as to be illegal.  For example, the right-most
  ** sibling might be completely empty.  This adjustment is not optional.
  */
  لكل(i=k-1; i>0; i--){
    صحيح sz_ئوڭ = szNew[i];  /* Size of sibling on the right */
    صحيح sz_سول = szNew[i-1]; /* Size of sibling on the left */
    صحيح r;              /* Index of right-most cell in left sibling */
    صحيح d;              /* Index of first cell to the left of right sibling */

    r = cntNew[i-1] - 1;
    d = r + 1 - leafData;
    (فراغ)cachedCellSize(&b, d);
    افعل{
      صحيح szR, szD;
      أكد( d<nMaxCells );
      أكد( r<nMaxCells );
      szR = cachedCellSize(&b, r);
      szD = b.szCell[d];
      إذا( sz_ئوڭ!=0
       && (bBulk || sz_ئوڭ+szD+2 > sz_سول-(szR+(i==k-1?0:2)))){
        اكسر;
      }
      sz_ئوڭ += szD + 2;
      sz_سول -= szR + 2;
      cntNew[i-1] = r;
      r--;
      d--;
    }طالما( r>=0 );
    szNew[i] = sz_ئوڭ;
    szNew[i-1] = sz_سول;
    إذا( cntNew[i-1] <= (i>1 ? cntNew[i-2] : 0) ){
      rc = SQLITE_CORRUPT_BKPT;
      اذهب balance_cleanup;
    }
  }

  /* Sanity check:  For a non-corrupt database file one of the following
  ** must be true:
  **    (1) We found one or more cells (cntNew[0])>0), or
  **    (2) pPage is a virtual root page.  A virtual root page is when
  **        the real root page is page 1 and we are the only child of
  **        that page.
  */
  أكد( cntNew[0]>0 || (pParent->pgno==1 && pParent->nCell==0) || CORRUPT_DB);
  TRACE(("BALANCE: old: %u(nc=%u) %u(nc=%u) %u(nc=%u)\n",
    apOld[0]->pgno, apOld[0]->nCell,
    nOld>=2 ? apOld[1]->pgno : 0, nOld>=2 ? apOld[1]->nCell : 0,
    nOld>=3 ? apOld[2]->pgno : 0, nOld>=3 ? apOld[2]->nCell : 0
  ));

  /*
  ** Allocate k new pages.  Reuse old pages where possible.
  */
  pageFlags = apOld[0]->aData[0];
  لكل(i=0; i<k; i++){
    MemPage *pNew;
    إذا( i<nOld ){
      pNew = apNew[i] = apOld[i];
      apOld[i] = 0;
      rc = sqlite3PagerWrite(pNew->pDbPage);
      nNew++;
      إذا( sqlite3PagerPageRefcount(pNew->pDbPage)!=1+(i==(iParentIdx-nxDiv))
       && rc==SQLITE_OK
      ){
        rc = SQLITE_CORRUPT_BKPT;
      }
      إذا( rc ) اذهب balance_cleanup;
    }وإلا{
      أكد( i>0 );
      rc = allocateBtreePage(pBt, &pNew, &pgno, (bBulk ? 1 : pgno), 0);
      إذا( rc ) اذهب balance_cleanup;
      zeroPage(pNew, pageFlags);
      apNew[i] = pNew;
      nNew++;
      cntOld[i] = b.nCell;

      /* Set the pointer-map entry for the new sibling page. */
      إذا( ISAUTOVACUUM(pBt) ){
        ptrmapPut(pBt, pNew->pgno, PTRMAP_BTREE, pParent->pgno, &rc);
        إذا( rc!=SQLITE_OK ){
          اذهب balance_cleanup;
        }
      }
    }
  }

  /*
  ** Reassign page numbers so that the new pages are in ascending order.
  ** This helps to keep entries in the disk file in order so that a scan
  ** of the table is closer to a linear scan through the file. That in turn
  ** helps the operating system to deliver pages from the disk more rapidly.
  **
  ** An O(N*N) sort algorithm is used, but since N is never more than NB+2
  ** (5), that is not a performance concern.
  **
  ** When NB==3, this one optimization makes the database about 25% faster
  ** for large insertions and deletions.
  */
  لكل(i=0; i<nNew; i++){
    aPgno[i] = apNew[i]->pgno;
    أكد( apNew[i]->pDbPage->flags & PGHDR_WRITEABLE );
    أكد( apNew[i]->pDbPage->flags & PGHDR_DIRTY );
  }
  لكل(i=0; i<nNew-1; i++){
    صحيح iB = i;
    لكل(j=i+1; j<nNew; j++){
      إذا( apNew[j]->pgno < apNew[iB]->pgno ) iB = j;
    }

    /* If apNew[i] has a page number that is bigger than any of the
    ** subsequence apNew[i] entries, then swap apNew[i] with the subsequent
    ** entry that has the smallest page number (which we know to be
    ** entry apNew[iB]).
    */
    إذا( iB!=i ){
      Pgno pgnoA = apNew[i]->pgno;
      Pgno pgnoB = apNew[iB]->pgno;
      Pgno pgnoTemp = (PENDING_BYTE/pBt->pageSize)+1;
      u16 fgA = apNew[i]->pDbPage->flags;
      u16 fgB = apNew[iB]->pDbPage->flags;
      sqlite3PagerRekey(apNew[i]->pDbPage, pgnoTemp, fgB);
      sqlite3PagerRekey(apNew[iB]->pDbPage, pgnoA, fgA);
      sqlite3PagerRekey(apNew[i]->pDbPage, pgnoB, fgB);
      apNew[i]->pgno = pgnoB;
      apNew[iB]->pgno = pgnoA;
    }
  }

  TRACE(("BALANCE: new: %u(%u nc=%u) %u(%u nc=%u) %u(%u nc=%u) "
         "%u(%u nc=%u) %u(%u nc=%u)\n",
    apNew[0]->pgno, szNew[0], cntNew[0],
    nNew>=2 ? apNew[1]->pgno : 0, nNew>=2 ? szNew[1] : 0,
    nNew>=2 ? cntNew[1] - cntNew[0] - !leafData : 0,
    nNew>=3 ? apNew[2]->pgno : 0, nNew>=3 ? szNew[2] : 0,
    nNew>=3 ? cntNew[2] - cntNew[1] - !leafData : 0,
    nNew>=4 ? apNew[3]->pgno : 0, nNew>=4 ? szNew[3] : 0,
    nNew>=4 ? cntNew[3] - cntNew[2] - !leafData : 0,
    nNew>=5 ? apNew[4]->pgno : 0, nNew>=5 ? szNew[4] : 0,
    nNew>=5 ? cntNew[4] - cntNew[3] - !leafData : 0
  ));

  أكد( sqlite3PagerIswriteable(pParent->pDbPage) );
  أكد( nNew>=1 && nNew<=ArraySize(apNew) );
  أكد( apNew[nNew-1]!=0 );
  put4byte(p_ئوڭ, apNew[nNew-1]->pgno);

  /* If the sibling pages are not leaves, ensure that the right-child pointer
  ** of the right-most new sibling page is set to the value that was
  ** originally in the same field of the right-most old sibling page. */
  إذا( (pageFlags & PTF_LEAF)==0 && nOld!=nNew ){
    MemPage *pOld;
    إذا( nNew>nOld ){
      pOld = apNew[nOld-1];
    }وإلا{
      pOld = apOld[nOld-1];
    }
    memcpy(&apNew[nNew-1]->aData[8], &pOld->aData[8], 4);
  }

  /* Make any required updates to pointer map entries associated with
  ** cells stored on sibling pages following the balance operation. Pointer
  ** map entries associated with divider cells are set by the insertCell()
  ** routine. The associated pointer map entries are:
  **
  **   a) if the cell contains a reference to an overflow chain, the
  **      entry associated with the first page in the overflow chain, and
  **
  **   b) if the sibling pages are not leaves, the child page associated
  **      with the cell.
  **
  ** If the sibling pages are not leaves, then the pointer map entry
  ** associated with the right-child of each sibling may also need to be
  ** updated. This happens below, after the sibling pages have been
  ** populated, not here.
  */
  إذا( ISAUTOVACUUM(pBt) ){
    MemPage *pOld;
    MemPage *pNew = pOld = apNew[0];
    صحيح cntOldNext = pNew->nCell + pNew->nOverflow;
    صحيح iNew = 0;
    صحيح iOld = 0;

    لكل(i=0; i<b.nCell; i++){
      u8 *pCell = b.apCell[i];
      طالما( i==cntOldNext ){
        iOld++;
        أكد( iOld<nNew || iOld<nOld );
        أكد( iOld>=0 && iOld<NB );
        pOld = iOld<nNew ? apNew[iOld] : apOld[iOld];
        cntOldNext += pOld->nCell + pOld->nOverflow + !leafData;
      }
      إذا( i==cntNew[iNew] ){
        pNew = apNew[++iNew];
        إذا( !leafData ) استمر;
      }

      /* Cell pCell is destined for new sibling page pNew. Originally, it
      ** was either part of sibling page iOld (possibly an overflow cell),
      ** or else the divider cell to the left of sibling page iOld. So,
      ** if sibling page iOld had the same page number as pNew, and if
      ** pCell really was a part of sibling page iOld (not a divider or
      ** overflow cell), we can skip updating the pointer map entries.  */
      إذا( iOld>=nNew
       || pNew->pgno!=aPgno[iOld]
       || !SQLITE_WITHIN(pCell,pOld->aData,pOld->aDataEnd)
      ){
        إذا( !leafCorrection ){
          ptrmapPut(pBt, get4byte(pCell), PTRMAP_BTREE, pNew->pgno, &rc);
        }
        إذا( cachedCellSize(&b,i)>pNew->minLocal ){
          ptrmapPutOvflPtr(pNew, pOld, pCell, &rc);
        }
        إذا( rc ) اذهب balance_cleanup;
      }
    }
  }

  /* Insert new divider cells into pParent. */
  لكل(i=0; i<nNew-1; i++){
    u8 *pCell;
    u8 *pTemp;
    صحيح sz;
    u8 *pSrcEnd;
    MemPage *pNew = apNew[i];
    j = cntNew[i];

    أكد( j<nMaxCells );
    أكد( b.apCell[j]!=0 );
    pCell = b.apCell[j];
    sz = b.szCell[j] + leafCorrection;
    pTemp = &aOvflSpace[iOvflSpace];
    إذا( !pNew->leaf ){
      memcpy(&pNew->aData[8], pCell, 4);
    }وإلا إذا( leafData ){
      /* If the tree is a leaf-data tree, and the siblings are leaves,
      ** then there is no divider cell in b.apCell[]. Instead, the divider
      ** cell consists of the integer key for the right-most cell of
      ** the sibling-page assembled above only.
      */
      CellInfo info;
      j--;
      pNew->xParseCell(pNew, b.apCell[j], &info);
      pCell = pTemp;
      sz = 4 + sqlite3PutVarint(&pCell[4], info.n_ئاچقۇچ);
      pTemp = 0;
    }وإلا{
      pCell -= 4;
      /* Obscure case for non-leaf-data trees: If the cell at pCell was
      ** previously stored on a leaf node, and its reported size was 4
      ** bytes, then it may actually be smaller than this
      ** (see btreeParseCellPtr(), 4 bytes is the minimum size of
      ** any cell). But it is important to pass the correct size to
      ** insertCell(), so reparse the cell now.
      **
      ** This can only happen for b-trees used to evaluate "IN (SELECT ...)"
      ** and WITHOUT ROWID tables with exactly one column which is the
      ** primary key.
      */
      إذا( b.szCell[j]==4 ){
        أكد(leafCorrection==4);
        sz = pParent->xCellSize(pParent, pCell);
      }
    }
    iOvflSpace += sz;
    أكد( sz<=pBt->maxLocal+23 );
    أكد( iOvflSpace <= (صحيح)pBt->pageSize );
    أكد( b.ixNx[NB*2-1]>j );
    لكل(k=0; b.ixNx[k]<=j; k++){}
    pSrcEnd = b.apEnd[k];
    إذا( SQLITE_OVERFLOW(pSrcEnd, pCell, pCell+sz) ){
      rc = SQLITE_CORRUPT_BKPT;
      اذهب balance_cleanup;
    }
    rc = قىستۇرۇش_Cell(pParent, nxDiv+i, pCell, sz, pTemp, pNew->pgno);
    إذا( rc!=SQLITE_OK ) اذهب balance_cleanup;
    أكد( sqlite3PagerIswriteable(pParent->pDbPage) );
  }

  /* Now update the actual sibling pages. The order in which they are updated
  ** is important, as this code needs to avoid disrupting any page from which
  ** cells may still to be read. In practice, this means:
  **
  **  (1) If cells are moving left (from apNew[iPg] to apNew[iPg-1])
  **      then it is not safe to update page apNew[iPg] until after
  **      the left-hand sibling apNew[iPg-1] has been updated.
  **
  **  (2) If cells are moving right (from apNew[iPg] to apNew[iPg+1])
  **      then it is not safe to update page apNew[iPg] until after
  **      the right-hand sibling apNew[iPg+1] has been updated.
  **
  ** If neither of the above apply, the page is safe to update.
  **
  ** The iPg value in the following loop starts at nNew-1 goes down
  ** to 0, then back up to nNew-1 again, thus making two passes over
  ** the pages.  On the initial downward pass, only condition (1) above
  ** needs to be tested because (2) will always be true from the previous
  ** step.  On the upward pass, both conditions are always true, so the
  ** upwards pass simply processes pages that were missed on the downward
  ** pass.
  */
  لكل(i=1-nNew; i<nNew; i++){
    صحيح iPg = i<0 ? -i : i;
    أكد( iPg>=0 && iPg<nNew );
    أكد( iPg>=1 || i>=0 );
    أكد( iPg<ArraySize(cntOld) );
    إذا( abDone[iPg] ) استمر;         /* Skip pages already processed */
    إذا( i>=0                            /* On the upwards pass, or... */
     || cntOld[iPg-1]>=cntNew[iPg-1]    /* Condition (1) is true */
    ){
      صحيح iNew;
      صحيح iOld;
      صحيح nNewCell;

      /* Verify condition (1):  If cells are moving left, update iPg
      ** only after iPg-1 has already been updated. */
      أكد( iPg==0 || cntOld[iPg-1]>=cntNew[iPg-1] || abDone[iPg-1] );

      /* Verify condition (2):  If cells are moving right, update iPg
      ** only after iPg+1 has already been updated. */
      أكد( cntNew[iPg]>=cntOld[iPg] || abDone[iPg+1] );

      إذا( iPg==0 ){
        iNew = iOld = 0;
        nNewCell = cntNew[0];
      }وإلا{
        iOld = iPg<nOld ? (cntOld[iPg-1] + !leafData) : b.nCell;
        iNew = cntNew[iPg-1] + !leafData;
        nNewCell = cntNew[iPg] - iNew;
      }

      rc = editPage(apNew[iPg], iOld, iNew, nNewCell, &b);
      إذا( rc ) اذهب balance_cleanup;
      abDone[iPg]++;
      apNew[iPg]->nFree = usableSpace-szNew[iPg];
      أكد( apNew[iPg]->nOverflow==0 );
      أكد( apNew[iPg]->nCell==nNewCell );
    }
  }

  /* All pages have been processed exactly once */
  أكد( memcmp(abDone, "\01\01\01\01\01", nNew)==0 );

  أكد( nOld>0 );
  أكد( nNew>0 );

  إذا( is_يىلتىز && pParent->nCell==0 && pParent->hdrOffset<=apNew[0]->nFree ){
    /* The root page of the b-tree now contains no cells. The only sibling
    ** page is the right-child of the parent. Copy the contents of the
    ** child page into the parent, decreasing the overall height of the
    ** b-tree structure by one. This is described as the "balance-shallower"
    ** sub-algorithm in some documentation.
    **
    ** If this is an auto-vacuum database, the call to copyNodeContent()
    ** sets all pointer-map entries corresponding to database image pages
    ** for which the pointer is stored within the content being copied.
    **
    ** It is critical that the child page be defragmented before being
    ** copied into the parent, because if the parent is page 1 then it will
    ** by smaller than the child due to the database header, and so all the
    ** free space needs to be up front.
    */
    أكد( nNew==1 || CORRUPT_DB );
    rc = defragmentPage(apNew[0], -1);
    testcase( rc!=SQLITE_OK );
    أكد( apNew[0]->nFree ==
        (get2byteNotZero(&apNew[0]->aData[5]) - apNew[0]->cellOffset
          - apNew[0]->nCell*2)
      || rc!=SQLITE_OK
    );
    copy_تۈگۈن_Content(apNew[0], pParent, &rc);
    freePage(apNew[0], &rc);
  }وإلا إذا( ISAUTOVACUUM(pBt) && !leafCorrection ){
    /* Fix the pointer map entries associated with the right-child of each
    ** sibling page. All other pointer map entries have already been taken
    ** care of.  */
    لكل(i=0; i<nNew; i++){
      u32 ئاچقۇچ = get4byte(&apNew[i]->aData[8]);
      ptrmapPut(pBt, ئاچقۇچ, PTRMAP_BTREE, apNew[i]->pgno, &rc);
    }
  }

  أكد( pParent->isInit );
  TRACE(("BALANCE: finished: old=%u new=%u cells=%u\n",
          nOld, nNew, b.nCell));

  /* Free any old pages that were not reused as new pages.
  */
  لكل(i=nNew; i<nOld; i++){
    freePage(apOld[i], &rc);
  }

#إذا 0
  إذا( ISAUTOVACUUM(pBt) && rc==SQLITE_OK && apNew[0]->isInit ){
    /* The ptrmapCheckPages() contains assert() statements that verify that
    ** all pointer map pages are set correctly. This is helpful while
    ** debugging. This is usually disabled because a corrupt database may
    ** cause an assert() statement to fail.  */
    ptrmapCheckPages(apNew, nNew);
    ptrmapCheckPages(&pParent, 1);
  }
#endif

  /*
  ** Cleanup before returning.
  */
balance_cleanup:
  sqlite3_دۆۋە_Free(0, b.apCell);
  لكل(i=0; i<nOld; i++){
    releasePage(apOld[i]);
  }
  لكل(i=0; i<nNew; i++){
    releasePage(apNew[i]);
  }

  أرجع rc;
}


/*
** This function is called when the root page of a b-tree structure is
** overfull (has one or more overflow pages).
**
** A new child page is allocated and the contents of the current root
** page, including overflow cells, are copied into the child. The root
** page is then overwritten to make it an empty page with the right-child
** pointer pointing to the new page.
**
** Before returning, all pointer-map entries corresponding to pages
** that the new child-page now contains pointers to are updated. The
** entry corresponding to the new right-child pointer of the root
** page is also updated.
**
** If successful, *ppChild is set to contain a reference to the child
** page and SQLITE_OK is returned. In this case the caller is required
** to call releasePage() on *ppChild exactly once. If an error occurs,
** an error code is returned and *ppChild is set to 0.
*/
ساكن صحيح balance_deeper(MemPage *p_يىلتىز, MemPage **ppChild){
  صحيح rc;                        /* Return value from subprocedures */
  MemPage *pChild = 0;           /* Pointer to a new child page */
  Pgno pgnoChild = 0;            /* Page number of the new child page */
  BtShared *pBt = p_يىلتىز->pBt;    /* The BTree */

  أكد( p_يىلتىز->nOverflow>0 );
  أكد( sqlite3_mutex_held(pBt->mutex) );

  /* Make pRoot, the root page of the b-tree, writable. Allocate a new
  ** page that will become the new right-child of pPage. Copy the contents
  ** of the node stored on pRoot into the new child page.
  */
  rc = sqlite3PagerWrite(p_يىلتىز->pDbPage);
  إذا( rc==SQLITE_OK ){
    rc = allocateBtreePage(pBt,&pChild,&pgnoChild,p_يىلتىز->pgno,0);
    copy_تۈگۈن_Content(p_يىلتىز, pChild, &rc);
    إذا( ISAUTOVACUUM(pBt) ){
      ptrmapPut(pBt, pgnoChild, PTRMAP_BTREE, p_يىلتىز->pgno, &rc);
    }
  }
  إذا( rc ){
    *ppChild = 0;
    releasePage(pChild);
    أرجع rc;
  }
  أكد( sqlite3PagerIswriteable(pChild->pDbPage) );
  أكد( sqlite3PagerIswriteable(p_يىلتىز->pDbPage) );
  أكد( pChild->nCell==p_يىلتىز->nCell || CORRUPT_DB );

  TRACE(("BALANCE: copy root %u into %u\n", p_يىلتىز->pgno, pChild->pgno));

  /* Copy the overflow cells from pRoot to pChild */
  memcpy(pChild->aiOvfl, p_يىلتىز->aiOvfl,
         p_يىلتىز->nOverflow*حجم(p_يىلتىز->aiOvfl[0]));
  memcpy(pChild->apOvfl, p_يىلتىز->apOvfl,
         p_يىلتىز->nOverflow*حجم(p_يىلتىز->apOvfl[0]));
  pChild->nOverflow = p_يىلتىز->nOverflow;

  /* Zero the contents of pRoot. Then install pChild as the right-child. */
  zeroPage(p_يىلتىز, pChild->aData[0] & ~PTF_LEAF);
  put4byte(&p_يىلتىز->aData[p_يىلتىز->hdrOffset+8], pgnoChild);

  *ppChild = pChild;
  أرجع SQLITE_OK;
}

/*
** Return SQLITE_CORRUPT if any cursor other than pCur is currently valid
** on the same B-tree as pCur.
**
** This can occur if a database is corrupt with two or more SQL tables
** pointing to the same b-tree.  If an insert occurs on one SQL table
** and causes a BEFORE TRIGGER to do a secondary insert on the other SQL
** table linked to the same b-tree.  If the secondary insert causes a
** rebalance, that can change content out from under the cursor on the
** first SQL table, violating invariants on the first insert.
*/
ساكن صحيح anotherValidCursor(BtCursor *pCur){
  BtCursor *pOther;
  لكل(pOther=pCur->pBt->pCursor; pOther; pOther=pOther->pNext){
    إذا( pOther!=pCur
     && pOther->eState==CURSOR_VALID
     && pOther->pPage==pCur->pPage
    ){
      أرجع SQLITE_CORRUPT_PAGE(pCur->pPage);
    }
  }
  أرجع SQLITE_OK;
}

/*
** The page that pCur currently points to has just been modified in
** some way. This function figures out if this modification means the
** tree needs to be balanced, and if so calls the appropriate balancing
** routine. Balancing routines are:
**
**   balance_quick()
**   balance_deeper()
**   balance_nonroot()
*/
ساكن صحيح balance(BtCursor *pCur){
  صحيح rc = SQLITE_OK;
  u8 aBalanceQuickSpace[13];
  u8 *pFree = 0;

  VVA_ONLY( صحيح balance_quick_called = 0 );
  VVA_ONLY( صحيح balance_deeper_called = 0 );

  افعل {
    صحيح iPage;
    MemPage *pPage = pCur->pPage;

    إذا( NEVER(pPage->nFree<0) && btreeComputeFreeSpace(pPage) ) اكسر;
    إذا( pPage->nOverflow==0 && pPage->nFree*3<=(صحيح)pCur->pBt->usableSize*2 ){
      /* No rebalance required as long as:
      **   (1) There are no overflow cells
      **   (2) The amount of free space on the page is less than 2/3rds of
      **       the total usable space on the page. */
      اكسر;
    }وإلا إذا( (iPage = pCur->iPage)==0 ){
      إذا( pPage->nOverflow && (rc = anotherValidCursor(pCur))==SQLITE_OK ){
        /* The root page of the b-tree is overfull. In this case call the
        ** balance_deeper() function to create a new child for the root-page
        ** and copy the current contents of the root-page to it. The
        ** next iteration of the do-loop will balance the child page.
        */
        أكد( balance_deeper_called==0 );
        VVA_ONLY( balance_deeper_called++ );
        rc = balance_deeper(pPage, &pCur->apPage[1]);
        إذا( rc==SQLITE_OK ){
          pCur->iPage = 1;
          pCur->ix = 0;
          pCur->aiIdx[0] = 0;
          pCur->apPage[0] = pPage;
          pCur->pPage = pCur->apPage[1];
          أكد( pCur->pPage->nOverflow );
        }
      }وإلا{
        اكسر;
      }
    }وإلا إذا( sqlite3PagerPageRefcount(pPage->pDbPage)>1 ){
      /* The page being written is not a root page, and there is currently
      ** more than one reference to it. This only happens if the page is one
      ** of its own ancestor pages. Corruption. */
      rc = SQLITE_CORRUPT_PAGE(pPage);
    }وإلا{
      MemPage * ثابت pParent = pCur->apPage[iPage-1];
      صحيح ثابت iIdx = pCur->aiIdx[iPage-1];

      rc = sqlite3PagerWrite(pParent->pDbPage);
      إذا( rc==SQLITE_OK && pParent->nFree<0 ){
        rc = btreeComputeFreeSpace(pParent);
      }
      إذا( rc==SQLITE_OK ){
#ifndef SQLITE_OMIT_QUICKBALANCE
        إذا( pPage->int_ئاچقۇچ_Leaf
         && pPage->nOverflow==1
         && pPage->aiOvfl[0]==pPage->nCell
         && pParent->pgno!=1
         && pParent->nCell==iIdx
        ){
          /* Call balance_quick() to create a new sibling of pPage on which
          ** to store the overflow cell. balance_quick() inserts a new cell
          ** into pParent, which may cause pParent overflow. If this
          ** happens, the next iteration of the do-loop will balance pParent
          ** use either balance_nonroot() or balance_deeper(). Until this
          ** happens, the overflow cell is stored in the aBalanceQuickSpace[]
          ** buffer.
          **
          ** The purpose of the following assert() is to check that only a
          ** single call to balance_quick() is made for each call to this
          ** function. If this were not verified, a subtle bug involving reuse
          ** of the aBalanceQuickSpace[] might sneak in.
          */
          أكد( balance_quick_called==0 );
          VVA_ONLY( balance_quick_called++ );
          rc = balance_quick(pParent, pPage, aBalanceQuickSpace);
        }وإلا
#endif
        {
          /* In this case, call balance_nonroot() to redistribute cells
          ** between pPage and up to 2 of its sibling pages. This involves
          ** modifying the contents of pParent, which may cause pParent to
          ** become overfull or underfull. The next iteration of the do-loop
          ** will balance the parent page to correct this.
          **
          ** If the parent page becomes overfull, the overflow cell or cells
          ** are stored in the pSpace buffer allocated immediately below.
          ** A subsequent iteration of the do-loop will deal with this by
          ** calling balance_nonroot() (balance_deeper() may be called first,
          ** but it doesn't deal with overflow cells - just moves them to a
          ** different page). Once this subsequent call to balance_nonroot()
          ** has completed, it is safe to release the pSpace buffer used by
          ** the previous call, as the overflow cell data will have been
          ** copied either into the body of a database page or into the new
          ** pSpace buffer passed to the latter call to balance_nonroot().
          */
          u8 *pSpace = sqlite3PageMalloc(pCur->pBt->pageSize);
          rc = balance_nonroot(pParent, iIdx, pSpace, iPage==1,
                               pCur->hints&BTREE_BULKLOAD);
          إذا( pFree ){
            /* If pFree is not NULL, it points to the pSpace buffer used
            ** by a previous call to balance_nonroot(). Its contents are
            ** now stored either on real database pages or within the
            ** new pSpace buffer, so it may be safely freed here. */
            sqlite3PageFree(pFree);
          }

          /* The pSpace buffer will be freed after the next call to
          ** balance_nonroot(), or just before this function returns, whichever
          ** comes first. */
          pFree = pSpace;
        }
      }

      pPage->nOverflow = 0;

      /* The next iteration of the do-loop balances the parent page. */
      releasePage(pPage);
      pCur->iPage--;
      أكد( pCur->iPage>=0 );
      pCur->pPage = pCur->apPage[pCur->iPage];
    }
  }طالما( rc==SQLITE_OK );

  إذا( pFree ){
    sqlite3PageFree(pFree);
  }
  أرجع rc;
}

/* Overwrite content from pX into pDest.  Only do the write if the
** content is different from what is already there.
*/
ساكن صحيح btreeOverwriteContent(
  MemPage *pPage,           /* MemPage on which writing will occur */
  u8 *pDest,                /* Pointer to the place to start writing */
  ثابت BtreePayload *pX,   /* Source of data to write */
  صحيح iOffset,              /* Offset of first byte to write */
  صحيح iAmt                  /* Number of bytes to be written */
){
  صحيح nData = pX->nData - iOffset;
  إذا( nData<=0 ){
    /* Overwriting with zeros */
    صحيح i;
    لكل(i=0; i<iAmt && pDest[i]==0; i++){}
    إذا( i<iAmt ){
      صحيح rc = sqlite3PagerWrite(pPage->pDbPage);
      إذا( rc ) أرجع rc;
      memset(pDest + i, 0, iAmt - i);
    }
  }وإلا{
    إذا( nData<iAmt ){
      /* Mixed read data and zeros at the end.  Make a recursive call
      ** to write the zeros then fall through to write the real data */
      صحيح rc = btreeOverwriteContent(pPage, pDest+nData, pX, iOffset+nData,
                                 iAmt-nData);
      إذا( rc ) أرجع rc;
      iAmt = nData;
    }
    إذا( memcmp(pDest, ((u8*)pX->pData) + iOffset, iAmt)!=0 ){
      صحيح rc = sqlite3PagerWrite(pPage->pDbPage);
      إذا( rc ) أرجع rc;
      /* In a corrupt database, it is possible for the source and destination
      ** buffers to overlap.  This is harmless since the database is already
      ** corrupt but it does cause valgrind and ASAN warnings.  So use
      ** memmove(). */
      memmove(pDest, ((u8*)pX->pData) + iOffset, iAmt);
    }
  }
  أرجع SQLITE_OK;
}

/*
** Overwrite the cell that cursor pCur is pointing to with fresh content
** contained in pX.  In this variant, pCur is pointing to an overflow
** cell.
*/
ساكن SQLITE_NOINLINE صحيح btreeOverwriteOverflowCell(
  BtCursor *pCur,                     /* Cursor pointing to cell to overwrite */
  ثابت BtreePayload *pX              /* Content to write into the cell */
){
  صحيح iOffset;                        /* Next byte of pX->pData to write */
  صحيح nTotal = pX->nData + pX->nZero; /* Total bytes of to write */
  صحيح rc;                             /* Return code */
  MemPage *pPage = pCur->pPage;       /* Page being written */
  BtShared *pBt;                      /* Btree */
  Pgno ovflPgno;                      /* Next overflow page to write */
  u32 ovflPageSize;                   /* Size to write on overflow page */

  أكد( pCur->info.nLocal<nTotal );  /* pCur is an overflow cell */

  /* Overwrite the local portion first */
  rc = btreeOverwriteContent(pPage, pCur->info.pPayload, pX,
                             0, pCur->info.nLocal);
  إذا( rc ) أرجع rc;

  /* Now overwrite the overflow pages */
  iOffset = pCur->info.nLocal;
  أكد( nTotal>=0 );
  أكد( iOffset>=0 );
  ovflPgno = get4byte(pCur->info.pPayload + iOffset);
  pBt = pPage->pBt;
  ovflPageSize = pBt->usableSize - 4;
  افعل{
    rc = btreeGetPage(pBt, ovflPgno, &pPage, 0);
    إذا( rc ) أرجع rc;
    إذا( sqlite3PagerPageRefcount(pPage->pDbPage)!=1 || pPage->isInit ){
      rc = SQLITE_CORRUPT_PAGE(pPage);
    }وإلا{
      إذا( iOffset+ovflPageSize<(u32)nTotal ){
        ovflPgno = get4byte(pPage->aData);
      }وإلا{
        ovflPageSize = nTotal - iOffset;
      }
      rc = btreeOverwriteContent(pPage, pPage->aData+4, pX,
                                 iOffset, ovflPageSize);
    }
    sqlite3PagerUnref(pPage->pDbPage);
    إذا( rc ) أرجع rc;
    iOffset += ovflPageSize;
  }طالما( iOffset<nTotal );
  أرجع SQLITE_OK;   
}

/*
** Overwrite the cell that cursor pCur is pointing to with fresh content
** contained in pX.
*/
ساكن صحيح btreeOverwriteCell(BtCursor *pCur, ثابت BtreePayload *pX){
  صحيح nTotal = pX->nData + pX->nZero; /* Total bytes of to write */
  MemPage *pPage = pCur->pPage;       /* Page being written */

  إذا( pCur->info.pPayload + pCur->info.nLocal > pPage->aDataEnd
   || pCur->info.pPayload < pPage->aData + pPage->cellOffset
  ){
    أرجع SQLITE_CORRUPT_PAGE(pPage);
  }
  إذا( pCur->info.nLocal==nTotal ){
    /* The entire cell is local */
    أرجع btreeOverwriteContent(pPage, pCur->info.pPayload, pX,
                                 0, pCur->info.nLocal);
  }وإلا{
    /* The cell contains overflow content */
    أرجع btreeOverwriteOverflowCell(pCur, pX);
  }
}


/*
** Insert a new record into the BTree.  The content of the new record
** is described by the pX object.  The pCur cursor is used only to
** define what table the record should be inserted into, and is left
** pointing at a random location.
**
** For a table btree (used for rowid tables), only the pX.nKey value of
** the key is used. The pX.pKey value must be NULL.  The pX.nKey is the
** rowid or INTEGER PRIMARY KEY of the row.  The pX.nData,pData,nZero fields
** hold the content of the row.
**
** For an index btree (used for indexes and WITHOUT ROWID tables), the
** key is an arbitrary byte sequence stored in pX.pKey,nKey.  The
** pX.pData,nData,nZero fields must be zero.
**
** If the seekResult parameter is non-zero, then a successful call to
** sqlite3BtreeIndexMoveto() to seek cursor pCur to (pKey,nKey) has already
** been performed.  In other words, if seekResult!=0 then the cursor
** is currently pointing to a cell that will be adjacent to the cell
** to be inserted.  If seekResult<0 then pCur points to a cell that is
** smaller then (pKey,nKey).  If seekResult>0 then pCur points to a cell
** that is larger than (pKey,nKey).
**
** If seekResult==0, that means pCur is pointing at some unknown location.
** In that case, this routine must seek the cursor to the correct insertion
** point for (pKey,nKey) before doing the insertion.  For index btrees,
** if pX->nMem is non-zero, then pX->aMem contains pointers to the unpacked
** key values and pX->aMem can be used instead of pX->pKey to avoid having
** to decode the key.
*/
صحيح sqlite3_Btree_قىستۇرۇش(
  BtCursor *pCur,                /* Insert data into the table of this cursor */
  ثابت BtreePayload *pX,        /* Content of the row to be inserted */
  صحيح flags,                     /* True if this is likely an append */
  صحيح seekResult                 /* Result of prior IndexMoveto() call */
){
  صحيح rc;
  صحيح loc = seekResult;          /* -1: before desired location  +1: after */
  صحيح szNew = 0;
  صحيح idx;
  MemPage *pPage;
  Btree *p = pCur->pBtree;
  unsigned حرف *oldCell;
  unsigned حرف *newCell = 0;

  أكد( (flags & (BTREE_SAVEPOSITION|BTREE_APPEND|BTREE_PREFORMAT))==flags );
  أكد( (flags & BTREE_PREFORMAT)==0 || seekResult || pCur->p_ئاچقۇچ_Info==0 );

  /* Save the positions of any other cursors open on this table.
  **
  ** In some cases, the call to btreeMoveto() below is a no-op. For
  ** example, when inserting data into a table with auto-generated integer
  ** keys, the VDBE layer invokes sqlite3BtreeLast() to figure out the
  ** integer key to use. It then calls this function to actually insert the
  ** data into the intkey B-Tree. In this case btreeMoveto() recognizes
  ** that the cursor is already where it needs to be and returns without
  ** doing any work. To avoid thwarting these optimizations, it is important
  ** not to clear the cursor here.
  */
  إذا( pCur->curFlags & BTCF_Multiple ){
    rc = saveAllCursors(p->pBt, pCur->pgno_يىلتىز, pCur);
    إذا( rc ) أرجع rc;
    إذا( loc && pCur->iPage<0 ){
      /* This can only happen if the schema is corrupt such that there is more
      ** than one table or index with the same root page as used by the cursor.
      ** Which can only happen if the SQLITE_NoSchemaError flag was set when
      ** the schema was loaded. This cannot be asserted though, as a user might
      ** set the flag, load the schema, and then unset the flag.  */
      أرجع SQLITE_CORRUPT_PGNO(pCur->pgno_يىلتىز);
    }
  }

  /* Ensure that the cursor is not in the CURSOR_FAULT state and that it
  ** points to a valid cell.
  */
  إذا( pCur->eState>=CURSOR_REQUIRESEEK ){
    testcase( pCur->eState==CURSOR_REQUIRESEEK );
    testcase( pCur->eState==CURSOR_FAULT );
    rc = move_To_يىلتىز(pCur);
    إذا( rc && rc!=SQLITE_EMPTY ) أرجع rc;
  }

  أكد( cursorOwnsBtShared(pCur) );
  أكد( (pCur->curFlags & BTCF_WriteFlag)!=0
              && p->pBt->inTransaction==TRANS_WRITE
              && (p->pBt->btsFlags & BTS_READ_ONLY)==0 );
  أكد( hasSharedCacheTableLock(p, pCur->pgno_يىلتىز, pCur->p_ئاچقۇچ_Info!=0, 2) );

  /* Assert that the caller has been consistent. If this cursor was opened
  ** expecting an index b-tree, then the caller should be inserting blob
  ** keys with no associated data. If the cursor was opened expecting an
  ** intkey table, the caller should be inserting integer keys with a
  ** blob of associated data.  */
  أكد( (flags & BTREE_PREFORMAT) || (pX->p_ئاچقۇچ==0)==(pCur->p_ئاچقۇچ_Info==0) );

  إذا( pCur->p_ئاچقۇچ_Info==0 ){
    أكد( pX->p_ئاچقۇچ==0 );
    /* If this is an insert into a table b-tree, invalidate any incrblob
    ** cursors open on the row being replaced */
    إذا( p->hasIncrblobCur ){
      invalidateIncrblobCursors(p, pCur->pgno_يىلتىز, pX->n_ئاچقۇچ, 0);
    }

    /* If BTREE_SAVEPOSITION is set, the cursor must already be pointing
    ** to a row with the same key as the new entry being inserted.
    */
#ifdef SQLITE_DEBUG
    إذا( flags & BTREE_SAVEPOSITION ){
      أكد( pCur->curFlags & BTCF_ValidNKey );
      أكد( pX->n_ئاچقۇچ==pCur->info.n_ئاچقۇچ );
      أكد( loc==0 );
    }
#endif

    /* On the other hand, BTREE_SAVEPOSITION==0 does not imply
    ** that the cursor is not pointing to a row to be overwritten.
    ** So do a complete check.
    */
    إذا( (pCur->curFlags&BTCF_ValidNKey)!=0 && pX->n_ئاچقۇچ==pCur->info.n_ئاچقۇچ ){
      /* The cursor is pointing to the entry that is to be
      ** overwritten */
      أكد( pX->nData>=0 && pX->nZero>=0 );
      إذا( pCur->info.nSize!=0
       && pCur->info.nPayload==(u32)pX->nData+pX->nZero
      ){
        /* New entry is the same size as the old.  Do an overwrite */
        أرجع btreeOverwriteCell(pCur, pX);
      }
      أكد( loc==0 );
    }وإلا إذا( loc==0 ){
      /* The cursor is *not* pointing to the cell to be overwritten, nor
      ** to an adjacent cell.  Move the cursor so that it is pointing either
      ** to the cell to be overwritten or an adjacent cell.
      */
      rc = sqlite3BtreeTableMoveto(pCur, pX->n_ئاچقۇچ,
               (flags & BTREE_APPEND)!=0, &loc);
      إذا( rc ) أرجع rc;
    }
  }وإلا{
    /* This is an index or a WITHOUT ROWID table */

    /* If BTREE_SAVEPOSITION is set, the cursor must already be pointing
    ** to a row with the same key as the new entry being inserted.
    */
    أكد( (flags & BTREE_SAVEPOSITION)==0 || loc==0 );

    /* If the cursor is not already pointing either to the cell to be
    ** overwritten, or if a new cell is being inserted, if the cursor is
    ** not pointing to an immediately adjacent cell, then move the cursor
    ** so that it does.
    */
    إذا( loc==0 && (flags & BTREE_SAVEPOSITION)==0 ){
      إذا( pX->nMem ){
        UnpackedRecord r;
        r.p_ئاچقۇچ_Info = pCur->p_ئاچقۇچ_Info;
        r.aMem = pX->aMem;
        r.nField = pX->nMem;
        r.default_rc = 0;
        r.eqSeen = 0;
        rc = sqlite3BtreeIndexMoveto(pCur, &r, &loc);
      }وإلا{
        rc = btreeMoveto(pCur, pX->p_ئاچقۇچ, pX->n_ئاچقۇچ,
                    (flags & BTREE_APPEND)!=0, &loc);
      }
      إذا( rc ) أرجع rc;
    }

    /* If the cursor is currently pointing to an entry to be overwritten
    ** and the new content is the same as as the old, then use the
    ** overwrite optimization.
    */
    إذا( loc==0 ){
      getCellInfo(pCur);
      إذا( pCur->info.n_ئاچقۇچ==pX->n_ئاچقۇچ ){
        BtreePayload x2;
        x2.pData = pX->p_ئاچقۇچ;
        x2.nData = (صحيح)pX->n_ئاچقۇچ;  أكد( pX->n_ئاچقۇچ<=0x7fffffff );
        x2.nZero = 0;
        أرجع btreeOverwriteCell(pCur, &x2);
      }
    }
  }
  أكد( pCur->eState==CURSOR_VALID
       || (pCur->eState==CURSOR_INVALID && loc) || CORRUPT_DB );

  pPage = pCur->pPage;
  أكد( pPage->int_ئاچقۇچ || pX->n_ئاچقۇچ>=0 || (flags & BTREE_PREFORMAT) );
  أكد( pPage->leaf || !pPage->int_ئاچقۇچ );
  إذا( pPage->nFree<0 ){
    إذا( NEVER(pCur->eState>CURSOR_INVALID) ){
     /* ^^^^^--- due to the moveToRoot() call above */
      rc = SQLITE_CORRUPT_PAGE(pPage);
    }وإلا{
      rc = btreeComputeFreeSpace(pPage);
    }
    إذا( rc ) أرجع rc;
  }

  TRACE(("INSERT: table=%u nkey=%lld ndata=%u page=%u %s\n",
          pCur->pgno_يىلتىز, pX->n_ئاچقۇچ, pX->nData, pPage->pgno,
          loc==0 ? "overwrite" : "new entry"));
  أكد( pPage->isInit || CORRUPT_DB );
  newCell = p->pBt->pTmpSpace;
  أكد( newCell!=0 );
  أكد( BTREE_PREFORMAT==OPFLAG_PREFORMAT );
  إذا( flags & BTREE_PREFORMAT ){
    rc = SQLITE_OK;
    szNew = p->pBt->nPreformatSize;
    إذا( szNew<4 ){
      szNew = 4;
      newCell[3] = 0;
    }
    إذا( ISAUTOVACUUM(p->pBt) && szNew>pPage->maxLocal ){
      CellInfo info;
      pPage->xParseCell(pPage, newCell, &info);
      إذا( info.nPayload!=info.nLocal ){
        Pgno ovfl = get4byte(&newCell[szNew-4]);
        ptrmapPut(p->pBt, ovfl, PTRMAP_OVERFLOW1, pPage->pgno, &rc);
        إذا( NEVER(rc) ) اذهب end_قىستۇرۇش;
      }
    }
  }وإلا{
    rc = fillInCell(pPage, newCell, pX, &szNew);
    إذا( rc ) اذهب end_قىستۇرۇش;
  }
  أكد( szNew==pPage->xCellSize(pPage, newCell) );
  أكد( szNew <= MX_CELL_SIZE(p->pBt) );
  idx = pCur->ix;
  pCur->info.nSize = 0;
  إذا( loc==0 ){
    CellInfo info;
    أكد( idx>=0 );
    إذا( idx>=pPage->nCell ){
      أرجع SQLITE_CORRUPT_PAGE(pPage);
    }
    rc = sqlite3PagerWrite(pPage->pDbPage);
    إذا( rc ){
      اذهب end_قىستۇرۇش;
    }
    oldCell = findCell(pPage, idx);
    إذا( !pPage->leaf ){
      memcpy(newCell, oldCell, 4);
    }
    BTREE_CLEAR_CELL(rc, pPage, oldCell, info);
    testcase( pCur->curFlags & BTCF_ValidOvfl );
    invalidateOverflowCache(pCur);
    إذا( info.nSize==szNew && info.nLocal==info.nPayload
     && (!ISAUTOVACUUM(p->pBt) || szNew<pPage->minLocal)
    ){
      /* Overwrite the old cell with the new if they are the same size.
      ** We could also try to do this if the old cell is smaller, then add
      ** the leftover space to the free list.  But experiments show that
      ** doing that is no faster then skipping this optimization and just
      ** calling dropCell() and insertCell().
      **
      ** This optimization cannot be used on an autovacuum database if the
      ** new entry uses overflow pages, as the insertCell() call below is
      ** necessary to add the PTRMAP_OVERFLOW1 pointer-map entry.  */
      أكد( rc==SQLITE_OK ); /* clearCell never fails when nLocal==nPayload */
      إذا( oldCell < pPage->aData+pPage->hdrOffset+10 ){
        أرجع SQLITE_CORRUPT_PAGE(pPage);
      }
      إذا( oldCell+szNew > pPage->aDataEnd ){
        أرجع SQLITE_CORRUPT_PAGE(pPage);
      }
      memcpy(oldCell, newCell, szNew);
      أرجع SQLITE_OK;
    }
    dropCell(pPage, idx, info.nSize, &rc);
    إذا( rc ) اذهب end_قىستۇرۇش;
  }وإلا إذا( loc<0 && pPage->nCell>0 ){
    أكد( pPage->leaf );
    idx = ++pCur->ix;
    pCur->curFlags &= ~(BTCF_ValidNKey|BTCF_ValidOvfl);
  }وإلا{
    أكد( pPage->leaf );
  }
  rc = قىستۇرۇش_Cell_Fast(pPage, idx, newCell, szNew);
  أكد( pPage->nOverflow==0 || rc==SQLITE_OK );
  أكد( rc!=SQLITE_OK || pPage->nCell>0 || pPage->nOverflow>0 );

  /* If no error has occurred and pPage has an overflow cell, call balance()
  ** to redistribute the cells within the tree. Since balance() may move
  ** the cursor, zero the BtCursor.info.nSize and BTCF_ValidNKey
  ** variables.
  **
  ** Previous versions of SQLite called moveToRoot() to move the cursor
  ** back to the root page as balance() used to invalidate the contents
  ** of BtCursor.apPage[] and BtCursor.aiIdx[]. Instead of doing that,
  ** set the cursor state to "invalid". This makes common insert operations
  ** slightly faster.
  **
  ** There is a subtle but important optimization here too. When inserting
  ** multiple records into an intkey b-tree using a single cursor (as can
  ** happen while processing an "INSERT INTO ... SELECT" statement), it
  ** is advantageous to leave the cursor pointing to the last entry in
  ** the b-tree if possible. If the cursor is left pointing to the last
  ** entry in the table, and the next row inserted has an integer key
  ** larger than the largest existing key, it is possible to insert the
  ** row without seeking the cursor. This can be a big performance boost.
  */
  إذا( pPage->nOverflow ){
    أكد( rc==SQLITE_OK );
    pCur->curFlags &= ~(BTCF_ValidNKey|BTCF_ValidOvfl);
    rc = balance(pCur);

    /* Must make sure nOverflow is reset to zero even if the balance()
    ** fails. Internal data structure corruption will result otherwise.
    ** Also, set the cursor state to invalid. This stops saveCursorPosition()
    ** from trying to save the current position of the cursor.  */
    pCur->pPage->nOverflow = 0;
    pCur->eState = CURSOR_INVALID;
    إذا( (flags & BTREE_SAVEPOSITION) && rc==SQLITE_OK ){
      btreeReleaseAllCursorPages(pCur);
      إذا( pCur->p_ئاچقۇچ_Info ){
        أكد( pCur->p_ئاچقۇچ==0 );
        pCur->p_ئاچقۇچ = sqlite3Malloc( pX->n_ئاچقۇچ );
        إذا( pCur->p_ئاچقۇچ==0 ){
          rc = SQLITE_NOMEM;
        }وإلا{
          memcpy(pCur->p_ئاچقۇچ, pX->p_ئاچقۇچ, pX->n_ئاچقۇچ);
        }
      }
      pCur->eState = CURSOR_REQUIRESEEK;
      pCur->n_ئاچقۇچ = pX->n_ئاچقۇچ;
    }
  }
  أكد( pCur->iPage<0 || pCur->pPage->nOverflow==0 );

end_قىستۇرۇش:
  أرجع rc;
}

/*
** This function is used as part of copying the current row from cursor
** pSrc into cursor pDest. If the cursors are open on intkey tables, then
** parameter iKey is used as the rowid value when the record is copied
** into pDest. Otherwise, the record is copied verbatim.
**
** This function does not actually write the new value to cursor pDest.
** Instead, it creates and populates any required overflow pages and
** writes the data for the new cell into the BtShared.pTmpSpace buffer
** for the destination database. The size of the cell, in bytes, is left
** in BtShared.nPreformatSize. The caller completes the insertion by
** calling sqlite3BtreeInsert() with the BTREE_PREFORMAT flag specified.
**
** SQLITE_OK is returned if successful, or an SQLite error code otherwise.
*/
صحيح sqlite3BtreeTransferRow(BtCursor *pDest, BtCursor *pSrc, i64 i_ئاچقۇچ){
  BtShared *pBt = pDest->pBt;
  u8 *aOut = pBt->pTmpSpace;    /* Pointer to next output buffer */
  ثابت u8 *aIn;                /* Pointer to next input buffer */
  u32 nIn;                      /* Size of input buffer aIn[] */
  u32 nRem;                     /* Bytes of data still to copy */

  getCellInfo(pSrc);
  إذا( pSrc->info.nPayload<0x80 ){
    *(aOut++) = (u8)pSrc->info.nPayload;
  }وإلا{
    aOut += sqlite3PutVarint(aOut, pSrc->info.nPayload);
  }
  إذا( pDest->p_ئاچقۇچ_Info==0 ) aOut += sqlite3PutVarint(aOut, i_ئاچقۇچ);
  nIn = pSrc->info.nLocal;
  aIn = pSrc->info.pPayload;
  إذا( aIn+nIn>pSrc->pPage->aDataEnd ){
    أرجع SQLITE_CORRUPT_PAGE(pSrc->pPage);
  }
  nRem = pSrc->info.nPayload;
  إذا( nIn==nRem && nIn<pDest->pPage->maxLocal ){
    memcpy(aOut, aIn, nIn);
    pBt->nPreformatSize = nIn + (صحيح)(aOut - pBt->pTmpSpace);
    أرجع SQLITE_OK;
  }وإلا{
    صحيح rc = SQLITE_OK;
    Pager *pSrcPager = pSrc->pBt->pPager;
    u8 *pPgnoOut = 0;
    Pgno ovflIn = 0;
    DbPage *pPageIn = 0;
    MemPage *pPageOut = 0;
    u32 nOut;                     /* Size of output buffer aOut[] */

    nOut = btreePayloadToLocal(pDest->pPage, pSrc->info.nPayload);
    pBt->nPreformatSize = (صحيح)nOut + (صحيح)(aOut - pBt->pTmpSpace);
    إذا( nOut<pSrc->info.nPayload ){
      pPgnoOut = &aOut[nOut];
      pBt->nPreformatSize += 4;
    }
 
    إذا( nRem>nIn ){
      إذا( aIn+nIn+4>pSrc->pPage->aDataEnd ){
        أرجع SQLITE_CORRUPT_PAGE(pSrc->pPage);
      }
      ovflIn = get4byte(&pSrc->info.pPayload[nIn]);
    }
 
    افعل {
      nRem -= nOut;
      افعل{
        أكد( nOut>0 );
        إذا( nIn>0 ){
          صحيح nCopy = MIN(nOut, nIn);
          memcpy(aOut, aIn, nCopy);
          nOut -= nCopy;
          nIn -= nCopy;
          aOut += nCopy;
          aIn += nCopy;
        }
        إذا( nOut>0 ){
          sqlite3PagerUnref(pPageIn);
          pPageIn = 0;
          rc = sqlite3PagerGet(pSrcPager, ovflIn, &pPageIn, PAGER_GET_READONLY);
          إذا( rc==SQLITE_OK ){
            aIn = (ثابت u8*)sqlite3PagerGetData(pPageIn);
            ovflIn = get4byte(aIn);
            aIn += 4;
            nIn = pSrc->pBt->usableSize - 4;
          }
        }
      }طالما( rc==SQLITE_OK && nOut>0 );
 
      إذا( rc==SQLITE_OK && nRem>0 && ALWAYS(pPgnoOut) ){
        Pgno pgnoNew = 0;  /* Prevent harmless static-analyzer warning */
        MemPage *pNew = 0;
        rc = allocateBtreePage(pBt, &pNew, &pgnoNew, 0, 0);
        put4byte(pPgnoOut, pgnoNew);
        إذا( ISAUTOVACUUM(pBt) && pPageOut ){
          ptrmapPut(pBt, pgnoNew, PTRMAP_OVERFLOW2, pPageOut->pgno, &rc);
        }
        releasePage(pPageOut);
        pPageOut = pNew;
        إذا( pPageOut ){
          pPgnoOut = pPageOut->aData;
          put4byte(pPgnoOut, 0);
          aOut = &pPgnoOut[4];
          nOut = MIN(pBt->usableSize - 4, nRem);
        }
      }
    }طالما( nRem>0 && rc==SQLITE_OK );
 
    releasePage(pPageOut);
    sqlite3PagerUnref(pPageIn);
    أرجع rc;
  }
}

/*
** Delete the entry that the cursor is pointing to.
**
** If the BTREE_SAVEPOSITION bit of the flags parameter is zero, then
** the cursor is left pointing at an arbitrary location after the delete.
** But if that bit is set, then the cursor is left in a state such that
** the next call to BtreeNext() or BtreePrev() moves it to the same row
** as it would have been on if the call to BtreeDelete() had been omitted.
**
** The BTREE_AUXDELETE bit of flags indicates that is one of several deletes
** associated with a single table entry and its indexes.  Only one of those
** deletes is considered the "primary" delete.  The primary delete occurs
** on a cursor that is not a BTREE_FORDELETE cursor.  All but one delete
** operation on non-FORDELETE cursors is tagged with the AUXDELETE flag.
** The BTREE_AUXDELETE bit is a hint that is not used by this implementation,
** but which might be used by alternative storage engines.
*/
صحيح sqlite3BtreeDelete(BtCursor *pCur, u8 flags){
  Btree *p = pCur->pBtree;
  BtShared *pBt = p->pBt;             
  صحيح rc;                    /* Return code */
  MemPage *pPage;            /* Page to delete cell from */
  unsigned حرف *pCell;      /* Pointer to cell to delete */
  صحيح iCellIdx;              /* Index of cell to delete */
  صحيح iCellDepth;            /* Depth of node containing pCell */
  CellInfo info;             /* Size of the cell being deleted */
  u8 bPreserve;              /* Keep cursor valid.  2 for CURSOR_SKIPNEXT */

  أكد( cursorOwnsBtShared(pCur) );
  أكد( pBt->inTransaction==TRANS_WRITE );
  أكد( (pBt->btsFlags & BTS_READ_ONLY)==0 );
  أكد( pCur->curFlags & BTCF_WriteFlag );
  أكد( hasSharedCacheTableLock(p, pCur->pgno_يىلتىز, pCur->p_ئاچقۇچ_Info!=0, 2) );
  أكد( !hasReadConflicts(p, pCur->pgno_يىلتىز) );
  أكد( (flags & ~(BTREE_SAVEPOSITION | BTREE_AUXDELETE))==0 );
  إذا( pCur->eState!=CURSOR_VALID ){
    إذا( pCur->eState>=CURSOR_REQUIRESEEK ){
      rc = btreeRestoreCursorPosition(pCur);
      أكد( rc!=SQLITE_OK || CORRUPT_DB || pCur->eState==CURSOR_VALID );
      إذا( rc || pCur->eState!=CURSOR_VALID ) أرجع rc;
    }وإلا{
      أرجع SQLITE_CORRUPT_PGNO(pCur->pgno_يىلتىز);
    }
  }
  أكد( pCur->eState==CURSOR_VALID );

  iCellDepth = pCur->iPage;
  iCellIdx = pCur->ix;
  pPage = pCur->pPage;
  إذا( pPage->nCell<=iCellIdx ){
    أرجع SQLITE_CORRUPT_PAGE(pPage);
  }
  pCell = findCell(pPage, iCellIdx);
  إذا( pPage->nFree<0 && btreeComputeFreeSpace(pPage) ){
    أرجع SQLITE_CORRUPT_PAGE(pPage);
  }
  إذا( pCell<&pPage->aCellIdx[pPage->nCell] ){
    أرجع SQLITE_CORRUPT_PAGE(pPage);
  }

  /* If the BTREE_SAVEPOSITION bit is on, then the cursor position must
  ** be preserved following this delete operation. If the current delete
  ** will cause a b-tree rebalance, then this is done by saving the cursor
  ** key and leaving the cursor in CURSOR_REQUIRESEEK state before
  ** returning.
  **
  ** If the current delete will not cause a rebalance, then the cursor
  ** will be left in CURSOR_SKIPNEXT state pointing to the entry immediately
  ** before or after the deleted entry.
  **
  ** The bPreserve value records which path is required:
  **
  **    bPreserve==0         Not necessary to save the cursor position
  **    bPreserve==1         Use CURSOR_REQUIRESEEK to save the cursor position
  **    bPreserve==2         Cursor won't move.  Set CURSOR_SKIPNEXT.
  */
  bPreserve = (flags & BTREE_SAVEPOSITION)!=0;
  إذا( bPreserve ){
    إذا( !pPage->leaf
     || (pPage->nFree+pPage->xCellSize(pPage,pCell)+2) >
                                                   (صحيح)(pBt->usableSize*2/3)
     || pPage->nCell==1  /* See dbfuzz001.test for a test case */
    ){
      /* A b-tree rebalance will be required after deleting this entry.
      ** Save the cursor key.  */
      rc = save_Cursor_ئاچقۇچ(pCur);
      إذا( rc ) أرجع rc;
    }وإلا{
      bPreserve = 2;
    }
  }

  /* If the page containing the entry to delete is not a leaf page, move
  ** the cursor to the largest entry in the tree that is smaller than
  ** the entry being deleted. This cell will replace the cell being deleted
  ** from the internal node. The 'previous' entry is used for this instead
  ** of the 'next' entry, as the previous entry is always a part of the
  ** sub-tree headed by the child page of the cell being deleted. This makes
  ** balancing the tree following the delete operation easier.  */
  إذا( !pPage->leaf ){
    rc = sqlite3BtreePrevious(pCur, 0);
    أكد( rc!=SQLITE_DONE );
    إذا( rc ) أرجع rc;
  }

  /* Save the positions of any other cursors open on this table before
  ** making any modifications.  */
  إذا( pCur->curFlags & BTCF_Multiple ){
    rc = saveAllCursors(pBt, pCur->pgno_يىلتىز, pCur);
    إذا( rc ) أرجع rc;
  }

  /* If this is a delete operation to remove a row from a table b-tree,
  ** invalidate any incrblob cursors open on the row being deleted.  */
  إذا( pCur->p_ئاچقۇچ_Info==0 && p->hasIncrblobCur ){
    invalidateIncrblobCursors(p, pCur->pgno_يىلتىز, pCur->info.n_ئاچقۇچ, 0);
  }

  /* Make the page containing the entry to be deleted writable. Then free any
  ** overflow pages associated with the entry and finally remove the cell
  ** itself from within the page.  */
  rc = sqlite3PagerWrite(pPage->pDbPage);
  إذا( rc ) أرجع rc;
  BTREE_CLEAR_CELL(rc, pPage, pCell, info);
  dropCell(pPage, iCellIdx, info.nSize, &rc);
  إذا( rc ) أرجع rc;

  /* If the cell deleted was not located on a leaf page, then the cursor
  ** is currently pointing to the largest entry in the sub-tree headed
  ** by the child-page of the cell that was just deleted from an internal
  ** node. The cell from the leaf node needs to be moved to the internal
  ** node to replace the deleted cell.  */
  إذا( !pPage->leaf ){
    MemPage *pLeaf = pCur->pPage;
    صحيح nCell;
    Pgno n;
    unsigned حرف *pTmp;

    إذا( pLeaf->nFree<0 ){
      rc = btreeComputeFreeSpace(pLeaf);
      إذا( rc ) أرجع rc;
    }
    إذا( iCellDepth<pCur->iPage-1 ){
      n = pCur->apPage[iCellDepth+1]->pgno;
    }وإلا{
      n = pCur->pPage->pgno;
    }
    pCell = findCell(pLeaf, pLeaf->nCell-1);
    إذا( pCell<&pLeaf->aData[4] ) أرجع SQLITE_CORRUPT_PAGE(pLeaf);
    nCell = pLeaf->xCellSize(pLeaf, pCell);
    أكد( MX_CELL_SIZE(pBt) >= nCell );
    pTmp = pBt->pTmpSpace;
    أكد( pTmp!=0 );
    rc = sqlite3PagerWrite(pLeaf->pDbPage);
    إذا( rc==SQLITE_OK ){
      rc = قىستۇرۇش_Cell(pPage, iCellIdx, pCell-4, nCell+4-(pCell[0]==2), pTmp, n);
    }
    dropCell(pLeaf, pLeaf->nCell-1, nCell, &rc);
    إذا( rc ) أرجع rc;
  }

  /* Balance the tree. If the entry deleted was located on a leaf page,
  ** then the cursor still points to that page. In this case the first
  ** call to balance() repairs the tree, and the if(...) condition is
  ** never true.
  **
  ** Otherwise, if the entry deleted was on an internal node page, then
  ** pCur is pointing to the leaf page from which a cell was removed to
  ** replace the cell deleted from the internal node. This is slightly
  ** tricky as the leaf node may be underfull, and the internal node may
  ** be either under or overfull. In this case run the balancing algorithm
  ** on the leaf node first. If the balance proceeds far enough up the
  ** tree that we can be sure that any problem in the internal node has
  ** been corrected, so be it. Otherwise, after balancing the leaf node,
  ** walk the cursor up the tree to the internal node and balance it as
  ** well.  */
  أكد( pCur->pPage->nOverflow==0 );
  أكد( pCur->pPage->nFree>=0 );
  إذا( pCur->pPage->nFree*3<=(صحيح)pCur->pBt->usableSize*2 ){
    /* Optimization: If the free space is less than 2/3rds of the page,
    ** then balance() will always be a no-op.  No need to invoke it. */
    rc = SQLITE_OK;
  }وإلا{
    rc = balance(pCur);
  }
  إذا( rc==SQLITE_OK && pCur->iPage>iCellDepth ){
    releasePageNotNull(pCur->pPage);
    pCur->iPage--;
    طالما( pCur->iPage>iCellDepth ){
      releasePage(pCur->apPage[pCur->iPage--]);
    }
    pCur->pPage = pCur->apPage[pCur->iPage];
    rc = balance(pCur);
  }

  إذا( rc==SQLITE_OK ){
    إذا( bPreserve>1 ){
      أكد( (pCur->iPage==iCellDepth || CORRUPT_DB) );
      أكد( pPage==pCur->pPage || CORRUPT_DB );
      أكد( (pPage->nCell>0 || CORRUPT_DB) && iCellIdx<=pPage->nCell );
      pCur->eState = CURSOR_SKIPNEXT;
      إذا( iCellIdx>=pPage->nCell ){
        pCur->skipNext = -1;
        pCur->ix = pPage->nCell-1;
      }وإلا{
        pCur->skipNext = 1;
      }
      /* The pCur->info structure is currently populated with the details
      ** of the cell that was just removed. But pCur->ix now points to a
      ** different cell in the b-tree. So zero info.nSize here so that if
      ** saveCursorPosition() is called on this cursor it does not trust
      ** the contents of pCur->info.  */
      pCur->info.nSize = 0;
    }وإلا{
      rc = move_To_يىلتىز(pCur);
      إذا( bPreserve ){
        btreeReleaseAllCursorPages(pCur);
        pCur->eState = CURSOR_REQUIRESEEK;
      }
      إذا( rc==SQLITE_EMPTY ) rc = SQLITE_OK;
    }
  }
  أرجع rc;
}

/*
** Create a new BTree table.  Write into *piTable the page
** number for the root page of the new table.
**
** The type of type is determined by the flags parameter.  Only the
** following values of flags are currently in use.  Other values for
** flags might not work:
**
**     BTREE_INTKEY|BTREE_LEAFDATA     Used for SQL tables with rowid keys
**     BTREE_ZERODATA                  Used for SQL indices
*/
ساكن صحيح btreeCreateTable(Btree *p, Pgno *piTable, صحيح createTabFlags){
  BtShared *pBt = p->pBt;
  MemPage *p_يىلتىز;
  Pgno pgno_يىلتىز;
  صحيح rc;
  صحيح ptfFlags;          /* Page-type flags for the root page of new table */

  أكد( sqlite3BtreeHoldsMutex(p) );
  أكد( pBt->inTransaction==TRANS_WRITE );
  أكد( (pBt->btsFlags & BTS_READ_ONLY)==0 );

#ifdef SQLITE_OMIT_AUTOVACUUM
  rc = allocateBtreePage(pBt, &p_يىلتىز, &pgno_يىلتىز, 1, 0);
  إذا( rc ){
    أرجع rc;
  }
#وإلا
  إذا( pBt->autoVacuum ){
    Pgno pgnoMove;      /* Move a page here to make room for the root-page */
    MemPage *pPageMove; /* The page to move to. */

    /* Creating a new table may probably require moving an existing database
    ** to make room for the new tables root page. In case this page turns
    ** out to be an overflow page, delete all overflow page-map caches
    ** held by open cursors.
    */
    invalidateAllOverflowCache(pBt);

    /* Read the value of meta[3] from the database to determine where the
    ** root page of the new table should go. meta[3] is the largest root-page
    ** created so far, so the new root-page is (meta[3]+1).
    */
    sqlite3BtreeGetMeta(p, BTREE_LARGEST_يىلتىز_PAGE, &pgno_يىلتىز);
    إذا( pgno_يىلتىز>btreePagecount(pBt) ){
      أرجع SQLITE_CORRUPT_PGNO(pgno_يىلتىز);
    }
    pgno_يىلتىز++;

    /* The new root-page may not be allocated on a pointer-map page, or the
    ** PENDING_BYTE page.
    */
    طالما( pgno_يىلتىز==PTRMAP_PAGENO(pBt, pgno_يىلتىز) ||
        pgno_يىلتىز==PENDING_BYTE_PAGE(pBt) ){
      pgno_يىلتىز++;
    }
    أكد( pgno_يىلتىز>=3 );

    /* Allocate a page. The page that currently resides at pgnoRoot will
    ** be moved to the allocated page (unless the allocated page happens
    ** to reside at pgnoRoot).
    */
    rc = allocateBtreePage(pBt, &pPageMove, &pgnoMove, pgno_يىلتىز, BTALLOC_EXACT);
    إذا( rc!=SQLITE_OK ){
      أرجع rc;
    }

    إذا( pgnoMove!=pgno_يىلتىز ){
      /* pgnoRoot is the page that will be used for the root-page of
      ** the new table (assuming an error did not occur). But we were
      ** allocated pgnoMove. If required (i.e. if it was not allocated
      ** by extending the file), the current page at position pgnoMove
      ** is already journaled.
      */
      u8 eType = 0;
      Pgno iPtrPage = 0;

      /* Save the positions of any open cursors. This is required in
      ** case they are holding a reference to an xFetch reference
      ** corresponding to page pgnoRoot.  */
      rc = saveAllCursors(pBt, 0, 0);
      releasePage(pPageMove);
      إذا( rc!=SQLITE_OK ){
        أرجع rc;
      }

      /* Move the page currently at pgnoRoot to pgnoMove. */
      rc = btreeGetPage(pBt, pgno_يىلتىز, &p_يىلتىز, 0);
      إذا( rc!=SQLITE_OK ){
        أرجع rc;
      }
      rc = ptrmapGet(pBt, pgno_يىلتىز, &eType, &iPtrPage);
      إذا( eType==PTRMAP_ROOTPAGE || eType==PTRMAP_FREEPAGE ){
        rc = SQLITE_CORRUPT_PGNO(pgno_يىلتىز);
      }
      إذا( rc!=SQLITE_OK ){
        releasePage(p_يىلتىز);
        أرجع rc;
      }
      أكد( eType!=PTRMAP_ROOTPAGE );
      أكد( eType!=PTRMAP_FREEPAGE );
      rc = relocatePage(pBt, p_يىلتىز, eType, iPtrPage, pgnoMove, 0);
      releasePage(p_يىلتىز);

      /* Obtain the page at pgnoRoot */
      إذا( rc!=SQLITE_OK ){
        أرجع rc;
      }
      rc = btreeGetPage(pBt, pgno_يىلتىز, &p_يىلتىز, 0);
      إذا( rc!=SQLITE_OK ){
        أرجع rc;
      }
      rc = sqlite3PagerWrite(p_يىلتىز->pDbPage);
      إذا( rc!=SQLITE_OK ){
        releasePage(p_يىلتىز);
        أرجع rc;
      }
    }وإلا{
      p_يىلتىز = pPageMove;
    }

    /* Update the pointer-map and meta-data with the new root-page number. */
    ptrmapPut(pBt, pgno_يىلتىز, PTRMAP_ROOTPAGE, 0, &rc);
    إذا( rc ){
      releasePage(p_يىلتىز);
      أرجع rc;
    }

    /* When the new root page was allocated, page 1 was made writable in
    ** order either to increase the database filesize, or to decrement the
    ** freelist count.  Hence, the sqlite3BtreeUpdateMeta() call cannot fail.
    */
    أكد( sqlite3PagerIswriteable(pBt->pPage1->pDbPage) );
    rc = sqlite3BtreeUpdateMeta(p, 4, pgno_يىلتىز);
    إذا( NEVER(rc) ){
      releasePage(p_يىلتىز);
      أرجع rc;
    }

  }وإلا{
    rc = allocateBtreePage(pBt, &p_يىلتىز, &pgno_يىلتىز, 1, 0);
    إذا( rc ) أرجع rc;
  }
#endif
  أكد( sqlite3PagerIswriteable(p_يىلتىز->pDbPage) );
  إذا( createTabFlags & BTREE_INTKEY ){
    ptfFlags = PTF_INTKEY | PTF_LEAFDATA | PTF_LEAF;
  }وإلا{
    ptfFlags = PTF_ZERODATA | PTF_LEAF;
  }
  zeroPage(p_يىلتىز, ptfFlags);
  sqlite3PagerUnref(p_يىلتىز->pDbPage);
  أكد( (pBt->openFlags & BTREE_SINGLE)==0 || pgno_يىلتىز==2 );
  *piTable = pgno_يىلتىز;
  أرجع SQLITE_OK;
}
صحيح sqlite3BtreeCreateTable(Btree *p, Pgno *piTable, صحيح flags){
  صحيح rc;
  sqlite3BtreeEnter(p);
  rc = btreeCreateTable(p, piTable, flags);
  sqlite3BtreeLeave(p);
  أرجع rc;
}

/*
** Erase the given database page and all its children.  Return
** the page to the freelist.
**
** The freePageFlag parameter serves a double role:
**
**    *  Bit 0 (freePageFlag&1) means that the page should be freed
**       after it is cleared.
**
**    *  Bits 1-31 (freePageFlag>>1) is the depth of recursion.  Use this
**       to prevent a corrupt database file from recursing too deeply and
**       overflowing the CPU stack.
*/
ساكن صحيح clearDatabasePage(
  BtShared *pBt,           /* The BTree that contains the table */
  Pgno pgno,               /* Page number to clear */
  صحيح freePageFlag,        /* bit 0: Deallocate page.  Bits 1-31: depth */
  i64 *pnChange            /* Add number of Cells freed to this counter */
){
  MemPage *pPage;
  صحيح rc;
  unsigned حرف *pCell;
  صحيح i;
  صحيح hdr;
  CellInfo info;

  أكد( sqlite3_mutex_held(pBt->mutex) );
  إذا( pgno>btreePagecount(pBt) ){
    أرجع SQLITE_CORRUPT_PGNO(pgno);
  }
  إذا( (freePageFlag>>1) > BTCURSOR_MAX_DEPTH ){
    أرجع SQLITE_CORRUPT_PGNO(pgno);
  }
  rc = getAndInitPage(pBt, pgno, &pPage, 0);
  إذا( rc ) أرجع rc;
  إذا( (pBt->openFlags & BTREE_SINGLE)==0
   && sqlite3PagerPageRefcount(pPage->pDbPage) != (1 + (pgno==1))
  ){
    rc = SQLITE_CORRUPT_PAGE(pPage);
    اذهب cleardatabasepage_out;
  }
  hdr = pPage->hdrOffset;
  لكل(i=0; i<pPage->nCell; i++){
    pCell = findCell(pPage, i);
    إذا( !pPage->leaf ){
      rc = clearDatabasePage(pBt, get4byte(pCell),
                             (freePageFlag+2)|1, pnChange);
      إذا( rc ) اذهب cleardatabasepage_out;
    }
    BTREE_CLEAR_CELL(rc, pPage, pCell, info);
    إذا( rc ) اذهب cleardatabasepage_out;
  }
  إذا( !pPage->leaf ){
    rc = clearDatabasePage(pBt, get4byte(&pPage->aData[hdr+8]), 
                           (freePageFlag+2)|1, pnChange);
    إذا( rc ) اذهب cleardatabasepage_out;
    إذا( pPage->int_ئاچقۇچ ) pnChange = 0;
  }
  إذا( pnChange ){
    testcase( !pPage->int_ئاچقۇچ );
    *pnChange += pPage->nCell;
  }
  إذا( (freePageFlag&1)!=0 ){
    freePage(pPage, &rc);
  }وإلا إذا( (rc = sqlite3PagerWrite(pPage->pDbPage))==0 ){
    zeroPage(pPage, pPage->aData[hdr] | PTF_LEAF);
  }

cleardatabasepage_out:
  releasePage(pPage);
  أرجع rc;
}

/*
** Delete all information from a single table in the database.  iTable is
** the page number of the root of the table.  After this routine returns,
** the root page is empty, but still exists.
**
** This routine will fail with SQLITE_LOCKED if there are any open
** read cursors on the table.  Open write cursors are moved to the
** root of the table.
**
** If pnChange is not NULL, then the integer value pointed to by pnChange
** is incremented by the number of entries in the table.
*/
صحيح sqlite3BtreeClearTable(Btree *p, صحيح iTable, i64 *pnChange){
  صحيح rc;
  BtShared *pBt = p->pBt;
  sqlite3BtreeEnter(p);
  أكد( p->inTrans==TRANS_WRITE );

  rc = saveAllCursors(pBt, (Pgno)iTable, 0);

  إذا( SQLITE_OK==rc ){
    /* Invalidate all incrblob cursors open on table iTable (assuming iTable
    ** is the root of a table b-tree - if it is not, the following call is
    ** a no-op).  */
    إذا( p->hasIncrblobCur ){
      invalidateIncrblobCursors(p, (Pgno)iTable, 0, 1);
    }
    rc = clearDatabasePage(pBt, (Pgno)iTable, 0, pnChange);
  }
  sqlite3BtreeLeave(p);
  أرجع rc;
}

/*
** Delete all information from the single table that pCur is open on.
**
** This routine only work for pCur on an ephemeral table.
*/
صحيح sqlite3BtreeClearTableOfCursor(BtCursor *pCur){
  أرجع sqlite3BtreeClearTable(pCur->pBtree, pCur->pgno_يىلتىز, 0);
}

/*
** Erase all information in a table and add the root of the table to
** the freelist.  Except, the root of the principle table (the one on
** page 1) is never added to the freelist.
**
** This routine will fail with SQLITE_LOCKED if there are any open
** cursors on the table.
**
** If AUTOVACUUM is enabled and the page at iTable is not the last
** root page in the database file, then the last root page
** in the database file is moved into the slot formerly occupied by
** iTable and that last slot formerly occupied by the last root page
** is added to the freelist instead of iTable.  In this say, all
** root pages are kept at the beginning of the database file, which
** is necessary for AUTOVACUUM to work right.  *piMoved is set to the
** page number that used to be the last root page in the file before
** the move.  If no page gets moved, *piMoved is set to 0.
** The last root page is recorded in meta[3] and the value of
** meta[3] is updated by this procedure.
*/
ساكن صحيح btreeDropTable(Btree *p, Pgno iTable, صحيح *piMoved){
  صحيح rc;
  MemPage *pPage = 0;
  BtShared *pBt = p->pBt;

  أكد( sqlite3BtreeHoldsMutex(p) );
  أكد( p->inTrans==TRANS_WRITE );
  أكد( iTable>=2 );
  إذا( iTable>btreePagecount(pBt) ){
    أرجع SQLITE_CORRUPT_PGNO(iTable);
  }

  rc = sqlite3BtreeClearTable(p, iTable, 0);
  إذا( rc ) أرجع rc;
  rc = btreeGetPage(pBt, (Pgno)iTable, &pPage, 0);
  إذا( NEVER(rc) ){
    releasePage(pPage);
    أرجع rc;
  }

  *piMoved = 0;

#ifdef SQLITE_OMIT_AUTOVACUUM
  freePage(pPage, &rc);
  releasePage(pPage);
#وإلا
  إذا( pBt->autoVacuum ){
    Pgno max_يىلتىز_Pgno;
    sqlite3BtreeGetMeta(p, BTREE_LARGEST_يىلتىز_PAGE, &max_يىلتىز_Pgno);

    إذا( iTable==max_يىلتىز_Pgno ){
      /* If the table being dropped is the table with the largest root-page
      ** number in the database, put the root page on the free list.
      */
      freePage(pPage, &rc);
      releasePage(pPage);
      إذا( rc!=SQLITE_OK ){
        أرجع rc;
      }
    }وإلا{
      /* The table being dropped does not have the largest root-page
      ** number in the database. So move the page that does into the
      ** gap left by the deleted root-page.
      */
      MemPage *pMove;
      releasePage(pPage);
      rc = btreeGetPage(pBt, max_يىلتىز_Pgno, &pMove, 0);
      إذا( rc!=SQLITE_OK ){
        أرجع rc;
      }
      rc = relocatePage(pBt, pMove, PTRMAP_ROOTPAGE, 0, iTable, 0);
      releasePage(pMove);
      إذا( rc!=SQLITE_OK ){
        أرجع rc;
      }
      pMove = 0;
      rc = btreeGetPage(pBt, max_يىلتىز_Pgno, &pMove, 0);
      إذا( rc==SQLITE_OK ){
        rc = sqlite3PagerWrite(pMove->pDbPage);
      }
      freePage(pMove, &rc);
      releasePage(pMove);
      إذا( rc!=SQLITE_OK ){
        أرجع rc;
      }
      *piMoved = max_يىلتىز_Pgno;
    }

    /* Set the new 'max-root-page' value in the database header. This
    ** is the old value less one, less one more if that happens to
    ** be a root-page number, less one again if that is the
    ** PENDING_BYTE_PAGE.
    */
    max_يىلتىز_Pgno--;
    طالما( max_يىلتىز_Pgno==PENDING_BYTE_PAGE(pBt)
           || PTRMAP_ISPAGE(pBt, max_يىلتىز_Pgno) ){
      max_يىلتىز_Pgno--;
    }
    أكد( max_يىلتىز_Pgno!=PENDING_BYTE_PAGE(pBt) );

    rc = sqlite3BtreeUpdateMeta(p, 4, max_يىلتىز_Pgno);
  }وإلا{
    freePage(pPage, &rc);
    releasePage(pPage);
  }
#endif
  أرجع rc; 
}
صحيح sqlite3BtreeDropTable(Btree *p, صحيح iTable, صحيح *piMoved){
  صحيح rc;
  sqlite3BtreeEnter(p);
  rc = btreeDropTable(p, iTable, piMoved);
  sqlite3BtreeLeave(p);
  أرجع rc;
}


/*
** This function may only be called if the b-tree connection already
** has a read or write transaction open on the database.
**
** Read the meta-information out of a database file.  Meta[0]
** is the number of free pages currently in the database.  Meta[1]
** through meta[15] are available for use by higher layers.  Meta[0]
** is read-only, the others are read/write.
**
** The schema layer numbers meta values differently.  At the schema
** layer (and the SetCookie and ReadCookie opcodes) the number of
** free pages is not visible.  So Cookie[0] is the same as Meta[1].
**
** This routine treats Meta[BTREE_DATA_VERSION] as a special case.  Instead
** of reading the value out of the header, it instead loads the "DataVersion"
** from the pager.  The BTREE_DATA_VERSION value is not actually stored in the
** database file.  It is a number computed by the pager.  But its access
** pattern is the same as header meta values, and so it is convenient to
** read it from this routine.
*/
فراغ sqlite3BtreeGetMeta(Btree *p, صحيح idx, u32 *pMeta){
  BtShared *pBt = p->pBt;

  sqlite3BtreeEnter(p);
  أكد( p->inTrans>TRANS_NONE );
  أكد( SQLITE_OK==querySharedCacheTableLock(p, SCHEMA_يىلتىز, READ_LOCK) );
  أكد( pBt->pPage1 );
  أكد( idx>=0 && idx<=15 );

  إذا( idx==BTREE_DATA_VERSION ){
    *pMeta = sqlite3PagerDataVersion(pBt->pPager) + p->iBDataVersion;
  }وإلا{
    *pMeta = get4byte(&pBt->pPage1->aData[36 + idx*4]);
  }

  /* If auto-vacuum is disabled in this build and this is an auto-vacuum
  ** database, mark the database as read-only.  */
#ifdef SQLITE_OMIT_AUTOVACUUM
  إذا( idx==BTREE_LARGEST_يىلتىز_PAGE && *pMeta>0 ){
    pBt->btsFlags |= BTS_READ_ONLY;
  }
#endif

  sqlite3BtreeLeave(p);
}

/*
** Write meta-information back into the database.  Meta[0] is
** read-only and may not be written.
*/
صحيح sqlite3BtreeUpdateMeta(Btree *p, صحيح idx, u32 iMeta){
  BtShared *pBt = p->pBt;
  unsigned حرف *pP1;
  صحيح rc;
  أكد( idx>=1 && idx<=15 );
  sqlite3BtreeEnter(p);
  أكد( p->inTrans==TRANS_WRITE );
  أكد( pBt->pPage1!=0 );
  pP1 = pBt->pPage1->aData;
  rc = sqlite3PagerWrite(pBt->pPage1->pDbPage);
  إذا( rc==SQLITE_OK ){
    put4byte(&pP1[36 + idx*4], iMeta);
#ifndef SQLITE_OMIT_AUTOVACUUM
    إذا( idx==BTREE_INCR_VACUUM ){
      أكد( pBt->autoVacuum || iMeta==0 );
      أكد( iMeta==0 || iMeta==1 );
      pBt->incrVacuum = (u8)iMeta;
    }
#endif
  }
  sqlite3BtreeLeave(p);
  أرجع rc;
}

/*
** The first argument, pCur, is a cursor opened on some b-tree. Count the
** number of entries in the b-tree and write the result to *pnEntry.
**
** SQLITE_OK is returned if the operation is successfully executed.
** Otherwise, if an error is encountered (i.e. an IO error or database
** corruption) an SQLite error code is returned.
*/
صحيح sqlite3_Btree_سانى(sqlite3 *db, BtCursor *pCur, i64 *pnEntry){
  i64 nEntry = 0;                      /* Value to return in *pnEntry */
  صحيح rc;                              /* Return code */

  rc = move_To_يىلتىز(pCur);
  إذا( rc==SQLITE_EMPTY ){
    *pnEntry = 0;
    أرجع SQLITE_OK;
  }

  /* Unless an error occurs, the following loop runs one iteration for each
  ** page in the B-Tree structure (not including overflow pages).
  */
  طالما( rc==SQLITE_OK && !AtomicLoad(&db->u1.isInterrupted) ){
    صحيح iIdx;                          /* Index of child node in parent */
    MemPage *pPage;                    /* Current page of the b-tree */

    /* If this is a leaf page or the tree is not an int-key tree, then
    ** this page contains countable entries. Increment the entry counter
    ** accordingly.
    */
    pPage = pCur->pPage;
    إذا( pPage->leaf || !pPage->int_ئاچقۇچ ){
      nEntry += pPage->nCell;
    }

    /* pPage is a leaf node. This loop navigates the cursor so that it
    ** points to the first interior cell that it points to the parent of
    ** the next page in the tree that has not yet been visited. The
    ** pCur->aiIdx[pCur->iPage] value is set to the index of the parent cell
    ** of the page, or to the number of cells in the page if the next page
    ** to visit is the right-child of its parent.
    **
    ** If all pages in the tree have been visited, return SQLITE_OK to the
    ** caller.
    */
    إذا( pPage->leaf ){
      افعل {
        إذا( pCur->iPage==0 ){
          /* All pages of the b-tree have been visited. Return successfully. */
          *pnEntry = nEntry;
          أرجع move_To_يىلتىز(pCur);
        }
        moveToParent(pCur);
      }طالما ( pCur->ix>=pCur->pPage->nCell );

      pCur->ix++;
      pPage = pCur->pPage;
    }

    /* Descend to the child node of the cell that the cursor currently
    ** points at. This is the right-child if (iIdx==pPage->nCell).
    */
    iIdx = pCur->ix;
    إذا( iIdx==pPage->nCell ){
      rc = moveToChild(pCur, get4byte(&pPage->aData[pPage->hdrOffset+8]));
    }وإلا{
      rc = moveToChild(pCur, get4byte(findCell(pPage, iIdx)));
    }
  }

  /* An error has occurred. Return an error code. */
  أرجع rc;
}

/*
** Return the pager associated with a BTree.  This routine is used for
** testing and debugging only.
*/
Pager *sqlite3BtreePager(Btree *p){
  أرجع p->pBt->pPager;
}

#ifndef SQLITE_OMIT_INTEGRITY_CHECK
/*
** Record an OOM error during integrity_check
*/
ساكن فراغ checkOom(IntegrityCk *pCheck){
  pCheck->rc = SQLITE_NOMEM;
  pCheck->mxErr = 0;  /* Causes integrity_check processing to stop */
  إذا( pCheck->nErr==0 ) pCheck->nErr++;
}

/*
** Invoke the progress handler, if appropriate.  Also check for an
** interrupt.
*/
ساكن فراغ checkProgress(IntegrityCk *pCheck){
  sqlite3 *db = pCheck->db;
  إذا( AtomicLoad(&db->u1.isInterrupted) ){
    pCheck->rc = SQLITE_INTERRUPT;
    pCheck->nErr++;
    pCheck->mxErr = 0;
  }
#ifndef SQLITE_OMIT_PROGRESS_CALLBACK
  إذا( db->xProgress ){
    أكد( db->nProgressOps>0 );
    pCheck->nStep++;
    إذا( (pCheck->nStep % db->nProgressOps)==0
     && db->xProgress(db->pProgressArg)
    ){
      pCheck->rc = SQLITE_INTERRUPT;
      pCheck->nErr++;
      pCheck->mxErr = 0;
    }
  }
#endif
}

/*
** Append a message to the error message string.
*/
ساكن فراغ check_Append_ئۇچۇر(
  IntegrityCk *pCheck,
  ثابت حرف *zFormat,
  ...
){
  va_list ap;
  checkProgress(pCheck);
  إذا( !pCheck->mxErr ) أرجع;
  pCheck->mxErr--;
  pCheck->nErr++;
  va_start(ap, zFormat);
  إذا( pCheck->err_ئۇچۇر.nChar ){
    sqlite3_str_append(&pCheck->err_ئۇچۇر, "\n", 1);
  }
  إذا( pCheck->zPfx ){
    sqlite3_str_appendf(&pCheck->err_ئۇچۇر, pCheck->zPfx,
                        pCheck->v0, pCheck->v1, pCheck->v2);
  }
  sqlite3_str_vappendf(&pCheck->err_ئۇچۇر, zFormat, ap);
  va_end(ap);
  إذا( pCheck->err_ئۇچۇر.accError==SQLITE_NOMEM ){
    checkOom(pCheck);
  }
}
#endif /* SQLITE_OMIT_INTEGRITY_CHECK */

#ifndef SQLITE_OMIT_INTEGRITY_CHECK

/*
** Return non-zero if the bit in the IntegrityCk.aPgRef[] array that
** corresponds to page iPg is already set.
*/
ساكن صحيح getPageReferenced(IntegrityCk *pCheck, Pgno iPg){
  أكد( pCheck->aPgRef!=0 );
  أكد( iPg<=pCheck->nCkPage && حجم(pCheck->aPgRef[0])==1 );
  أرجع (pCheck->aPgRef[iPg/8] & (1 << (iPg & 0x07)));
}

/*
** Set the bit in the IntegrityCk.aPgRef[] array that corresponds to page iPg.
*/
ساكن فراغ setPageReferenced(IntegrityCk *pCheck, Pgno iPg){
  أكد( pCheck->aPgRef!=0 );
  أكد( iPg<=pCheck->nCkPage && حجم(pCheck->aPgRef[0])==1 );
  pCheck->aPgRef[iPg/8] |= (1 << (iPg & 0x07));
}


/*
** Add 1 to the reference count for page iPage.  If this is the second
** reference to the page, add an error message to pCheck->zErrMsg.
** Return 1 if there are 2 or more references to the page and 0 if
** if this is the first reference to the page.
**
** Also check that the page number is in bounds.
*/
ساكن صحيح checkRef(IntegrityCk *pCheck, Pgno iPage){
  إذا( iPage>pCheck->nCkPage || iPage==0 ){
    check_Append_ئۇچۇر(pCheck, "invalid page number %u", iPage);
    أرجع 1;
  }
  إذا( getPageReferenced(pCheck, iPage) ){
    check_Append_ئۇچۇر(pCheck, "2nd reference to page %u", iPage);
    أرجع 1;
  }
  setPageReferenced(pCheck, iPage);
  أرجع 0;
}

#ifndef SQLITE_OMIT_AUTOVACUUM
/*
** Check that the entry in the pointer-map for page iChild maps to
** page iParent, pointer type ptrType. If not, append an error message
** to pCheck.
*/
ساكن فراغ checkPtrmap(
  IntegrityCk *pCheck,   /* Integrity check context */
  Pgno iChild,           /* Child page number */
  u8 eType,              /* Expected pointer map type */
  Pgno iParent           /* Expected pointer map parent page number */
){
  صحيح rc;
  u8 ePtrmapType;
  Pgno iPtrmapParent;

  rc = ptrmapGet(pCheck->pBt, iChild, &ePtrmapType, &iPtrmapParent);
  إذا( rc!=SQLITE_OK ){
    إذا( rc==SQLITE_NOMEM || rc==SQLITE_IOERR_NOMEM ) checkOom(pCheck);
    check_Append_ئۇچۇر(pCheck, "Failed to read ptrmap key=%u", iChild);
    أرجع;
  }

  إذا( ePtrmapType!=eType || iPtrmapParent!=iParent ){
    check_Append_ئۇچۇر(pCheck,
      "Bad ptr map entry key=%u expected=(%u,%u) got=(%u,%u)",
      iChild, eType, iParent, ePtrmapType, iPtrmapParent);
  }
}
#endif

/*
** Check the integrity of the freelist or of an overflow page list.
** Verify that the number of pages on the list is N.
*/
ساكن فراغ checkList(
  IntegrityCk *pCheck,  /* Integrity checking context */
  صحيح isFreeList,       /* True for a freelist.  False for overflow page list */
  Pgno iPage,           /* Page number for first page in the list */
  u32 N                 /* Expected number of pages in the list */
){
  صحيح i;
  u32 expected = N;
  صحيح nErrAtStart = pCheck->nErr;
  طالما( iPage!=0 && pCheck->mxErr ){
    DbPage *pOvflPage;
    unsigned حرف *pOvflData;
    إذا( checkRef(pCheck, iPage) ) اكسر;
    N--;
    إذا( sqlite3PagerGet(pCheck->pPager, (Pgno)iPage, &pOvflPage, 0) ){
      check_Append_ئۇچۇر(pCheck, "failed to get page %u", iPage);
      اكسر;
    }
    pOvflData = (unsigned حرف *)sqlite3PagerGetData(pOvflPage);
    إذا( isFreeList ){
      u32 n = (u32)get4byte(&pOvflData[4]);
#ifndef SQLITE_OMIT_AUTOVACUUM
      إذا( pCheck->pBt->autoVacuum ){
        checkPtrmap(pCheck, iPage, PTRMAP_FREEPAGE, 0);
      }
#endif
      إذا( n>pCheck->pBt->usableSize/4-2 ){
        check_Append_ئۇچۇر(pCheck,
           "freelist leaf count too big on page %u", iPage);
        N--;
      }وإلا{
        لكل(i=0; i<(صحيح)n; i++){
          Pgno iFreePage = get4byte(&pOvflData[8+i*4]);
#ifndef SQLITE_OMIT_AUTOVACUUM
          إذا( pCheck->pBt->autoVacuum ){
            checkPtrmap(pCheck, iFreePage, PTRMAP_FREEPAGE, 0);
          }
#endif
          checkRef(pCheck, iFreePage);
        }
        N -= n;
      }
    }
#ifndef SQLITE_OMIT_AUTOVACUUM
    وإلا{
      /* If this database supports auto-vacuum and iPage is not the last
      ** page in this overflow list, check that the pointer-map entry for
      ** the following page matches iPage.
      */
      إذا( pCheck->pBt->autoVacuum && N>0 ){
        i = get4byte(pOvflData);
        checkPtrmap(pCheck, i, PTRMAP_OVERFLOW2, iPage);
      }
    }
#endif
    iPage = get4byte(pOvflData);
    sqlite3PagerUnref(pOvflPage);
  }
  إذا( N && nErrAtStart==pCheck->nErr ){
    check_Append_ئۇچۇر(pCheck,
      "%s is %u but should be %u",
      isFreeList ? "size" : "overflow list length",
      expected-N, expected);
  }
}
#endif /* SQLITE_OMIT_INTEGRITY_CHECK */

/*
** An implementation of a min-heap.
**
** aHeap[0] is the number of elements on the heap.  aHeap[1] is the
** root element.  The daughter nodes of aHeap[N] are aHeap[N*2]
** and aHeap[N*2+1].
**
** The heap property is this:  Every node is less than or equal to both
** of its daughter nodes.  A consequence of the heap property is that the
** root node aHeap[1] is always the minimum value currently in the heap.
**
** The btreeHeapInsert() routine inserts an unsigned 32-bit number onto
** the heap, preserving the heap property.  The btreeHeapPull() routine
** removes the root element from the heap (the minimum value in the heap)
** and then moves other nodes around as necessary to preserve the heap
** property.
**
** This heap is used for cell overlap and coverage testing.  Each u32
** entry represents the span of a cell or freeblock on a btree page. 
** The upper 16 bits are the index of the first byte of a range and the
** lower 16 bits are the index of the last byte of that range.
*/
ساكن فراغ btree_Heap_قىستۇرۇش(u32 *aHeap, u32 x){
  u32 j, i;
  أكد( aHeap!=0 );
  i = ++aHeap[0];
  aHeap[i] = x;
  طالما( (j = i/2)>0 && aHeap[j]>aHeap[i] ){
    x = aHeap[j];
    aHeap[j] = aHeap[i];
    aHeap[i] = x;
    i = j;
  }
}
ساكن صحيح btreeHeapPull(u32 *aHeap, u32 *pOut){
  u32 j, i, x;
  إذا( (x = aHeap[0])==0 ) أرجع 0;
  *pOut = aHeap[1];
  aHeap[1] = aHeap[x];
  aHeap[x] = 0xffffffff;
  aHeap[0]--;
  i = 1;
  طالما( (j = i*2)<=aHeap[0] ){
    إذا( aHeap[j]>aHeap[j+1] ) j++;
    إذا( aHeap[i]<aHeap[j] ) اكسر;
    x = aHeap[i];
    aHeap[i] = aHeap[j];
    aHeap[j] = x;
    i = j;
  }
  أرجع 1; 
}

#ifndef SQLITE_OMIT_INTEGRITY_CHECK
/*
** Do various sanity checks on a single page of a tree.  Return
** the tree depth.  Root pages return 0.  Parents of root pages
** return 1, and so forth.
**
** These checks are done:
**
**      1.  Make sure that cells and freeblocks do not overlap
**          but combine to completely cover the page.
**      2.  Make sure integer cell keys are in order.
**      3.  Check the integrity of overflow pages.
**      4.  Recursively call checkTreePage on all children.
**      5.  Verify that the depth of all children is the same.
*/
ساكن صحيح check_دەرەخ_Page(
  IntegrityCk *pCheck,  /* Context for the sanity check */
  Pgno iPage,           /* Page number of the page to check */
  i64 *pi_Min_ئاچقۇچ,        /* Write minimum integer primary key here */
  i64 max_ئاچقۇچ            /* Error if integer primary key greater than this */
){
  MemPage *pPage = 0;      /* The page being analyzed */
  صحيح i;                   /* Loop counter */
  صحيح rc;                  /* Result code from subroutine call */
  صحيح depth = -1, d2;      /* Depth of a subtree */
  صحيح pgno;                /* Page number */
  صحيح nFrag;               /* Number of fragmented bytes on the page */
  صحيح hdr;                 /* Offset to the page header */
  صحيح cellStart;           /* Offset to the start of the cell pointer array */
  صحيح nCell;               /* Number of cells */
  صحيح doCoverageCheck = 1; /* True if cell coverage checking should be done */
  صحيح ئاچقۇچ_Can_Be_Equal = 1;   /* True if IPK can be equal to maxKey
                           ** False if IPK must be strictly less than maxKey */
  u8 *data;                /* Page content */
  u8 *pCell;               /* Cell content */
  u8 *pCellIdx;            /* Next element of the cell pointer array */
  BtShared *pBt;           /* The BtShared object that owns pPage */
  u32 pc;                  /* Address of a cell */
  u32 usableSize;          /* Usable size of the page */
  u32 contentOffset;       /* Offset to the start of the cell content area */
  u32 *heap = 0;           /* Min-heap used for checking cell coverage */
  u32 x, prev = 0;         /* Next and previous entry on the min-heap */
  ثابت حرف *saved_zPfx = pCheck->zPfx;
  صحيح saved_v1 = pCheck->v1;
  صحيح saved_v2 = pCheck->v2;
  u8 savedIsInit = 0;

  /* Check that the page exists
  */
  checkProgress(pCheck);
  إذا( pCheck->mxErr==0 ) أرجع 0;
  pBt = pCheck->pBt;
  usableSize = pBt->usableSize;
  إذا( iPage==0 ) أرجع 0;
  إذا( checkRef(pCheck, iPage) ) أرجع 0;
  pCheck->zPfx = "Tree %u page %u: ";
  pCheck->v1 = iPage;
  pCheck->nAbove++;
  إذا( (rc = btreeGetPage(pBt, iPage, &pPage, 0))!=0 ){
    check_Append_ئۇچۇر(pCheck,
       "unable to get the page. error code=%d", rc);
    إذا( rc==SQLITE_IOERR_NOMEM ) pCheck->rc = SQLITE_NOMEM;
    اذهب end_of_check;
  }
  إذا( pCheck->nAbove > BTCURSOR_MAX_DEPTH ){
    check_Append_ئۇچۇر(pCheck,"btree depth exceeds %d",BTCURSOR_MAX_DEPTH);
    اذهب end_of_check;
  }

  /* Clear MemPage.isInit to make sure the corruption detection code in
  ** btreeInitPage() is executed.  */
  savedIsInit = pPage->isInit;
  pPage->isInit = 0;
  إذا( (rc = btreeInitPage(pPage))!=0 ){
    أكد( rc==SQLITE_CORRUPT );  /* The only possible error from InitPage */
    check_Append_ئۇچۇر(pCheck,
                   "btreeInitPage() returns error code %d", rc);
    اذهب end_of_check;
  }
  إذا( (rc = btreeComputeFreeSpace(pPage))!=0 ){
    أكد( rc==SQLITE_CORRUPT );
    check_Append_ئۇچۇر(pCheck, "free space corruption", rc);
    اذهب end_of_check;
  }
  data = pPage->aData;
  hdr = pPage->hdrOffset;

  /* Set up for cell analysis */
  pCheck->zPfx = "Tree %u page %u cell %u: ";
  contentOffset = get2byteNotZero(&data[hdr+5]);
  أكد( contentOffset<=usableSize );  /* Enforced by btreeInitPage() */

  /* EVIDENCE-OF: R-37002-32774 The two-byte integer at offset 3 gives the
  ** number of cells on the page. */
  nCell = get2byte(&data[hdr+3]);
  أكد( pPage->nCell==nCell );
  إذا( pPage->leaf || pPage->int_ئاچقۇچ==0 ){
    pCheck->nRow += nCell;
  }

  /* EVIDENCE-OF: R-23882-45353 The cell pointer array of a b-tree page
  ** immediately follows the b-tree page header. */
  cellStart = hdr + 12 - 4*pPage->leaf;
  أكد( pPage->aCellIdx==&data[cellStart] );
  pCellIdx = &data[cellStart + 2*(nCell-1)];

  إذا( !pPage->leaf ){
    /* Analyze the right-child page of internal pages */
    pgno = get4byte(&data[hdr+8]);
#ifndef SQLITE_OMIT_AUTOVACUUM
    إذا( pBt->autoVacuum ){
      pCheck->zPfx = "Tree %u page %u right child: ";
      checkPtrmap(pCheck, pgno, PTRMAP_BTREE, iPage);
    }
#endif
    depth = check_دەرەخ_Page(pCheck, pgno, &max_ئاچقۇچ, max_ئاچقۇچ);
    ئاچقۇچ_Can_Be_Equal = 0;
  }وإلا{
    /* For leaf pages, the coverage check will occur in the same loop
    ** as the other cell checks, so initialize the heap.  */
    heap = pCheck->heap;
    heap[0] = 0;
  }

  /* EVIDENCE-OF: R-02776-14802 The cell pointer array consists of K 2-byte
  ** integer offsets to the cell contents. */
  لكل(i=nCell-1; i>=0 && pCheck->mxErr; i--){
    CellInfo info;

    /* Check cell size */
    pCheck->v2 = i;
    أكد( pCellIdx==&data[cellStart + i*2] );
    pc = get2byteAligned(pCellIdx);
    pCellIdx -= 2;
    إذا( pc<contentOffset || pc>usableSize-4 ){
      check_Append_ئۇچۇر(pCheck, "Offset %u out of range %u..%u",
                             pc, contentOffset, usableSize-4);
      doCoverageCheck = 0;
      استمر;
    }
    pCell = &data[pc];
    pPage->xParseCell(pPage, pCell, &info);
    إذا( pc+info.nSize>usableSize ){
      check_Append_ئۇچۇر(pCheck, "Extends off end of page");
      doCoverageCheck = 0;
      استمر;
    }
    إذا( info.nPayload && info.pPayload[0]<2 ){
      check_Append_ئۇچۇر(pCheck, "Bad cell header size");
      doCoverageCheck = 0;
      استمر;
    }

    /* Check for integer primary key out of range */
    إذا( pPage->int_ئاچقۇچ ){
      إذا( ئاچقۇچ_Can_Be_Equal ? (info.n_ئاچقۇچ > max_ئاچقۇچ) : (info.n_ئاچقۇچ >= max_ئاچقۇچ) ){
        check_Append_ئۇچۇر(pCheck, "Rowid %lld out of order", info.n_ئاچقۇچ);
      }
      max_ئاچقۇچ = info.n_ئاچقۇچ;
      ئاچقۇچ_Can_Be_Equal = 0;     /* Only the first key on the page may ==maxKey */
    }

    /* Check the content overflow list */
    إذا( info.nPayload>info.nLocal ){
      u32 nPage;       /* Number of pages on the overflow chain */
      Pgno pgnoOvfl;   /* First page of the overflow chain */
      أكد( pc + info.nSize - 4 <= usableSize );
      nPage = (info.nPayload - info.nLocal + usableSize - 5)/(usableSize - 4);
      pgnoOvfl = get4byte(&pCell[info.nSize - 4]);
#ifndef SQLITE_OMIT_AUTOVACUUM
      إذا( pBt->autoVacuum ){
        checkPtrmap(pCheck, pgnoOvfl, PTRMAP_OVERFLOW1, iPage);
      }
#endif
      checkList(pCheck, 0, pgnoOvfl, nPage);
    }

    إذا( !pPage->leaf ){
      /* Check sanity of left child page for internal pages */
      pgno = get4byte(pCell);
#ifndef SQLITE_OMIT_AUTOVACUUM
      إذا( pBt->autoVacuum ){
        checkPtrmap(pCheck, pgno, PTRMAP_BTREE, iPage);
      }
#endif
      d2 = check_دەرەخ_Page(pCheck, pgno, &max_ئاچقۇچ, max_ئاچقۇچ);
      ئاچقۇچ_Can_Be_Equal = 0;
      إذا( d2!=depth ){
        check_Append_ئۇچۇر(pCheck, "Child page depth differs");
        depth = d2;
      }
    }وإلا{
      /* Populate the coverage-checking heap for leaf pages */
      أكد( heap!=0 && heap[0] < pCheck->mxHeap );
      btree_Heap_قىستۇرۇش(heap, (pc<<16)|(pc+info.nSize-1));
    }
  }
  *pi_Min_ئاچقۇچ = max_ئاچقۇچ;

  /* Check for complete coverage of the page
  */
  pCheck->zPfx = 0;
  إذا( doCoverageCheck && pCheck->mxErr>0 ){
    /* For leaf pages, the min-heap has already been initialized and the
    ** cells have already been inserted.  But for internal pages, that has
    ** not yet been done, so do it now */
    إذا( !pPage->leaf ){
      heap = pCheck->heap;
      heap[0] = 0;
      لكل(i=nCell-1; i>=0; i--){
        u32 size;
        pc = get2byteAligned(&data[cellStart+i*2]);
        size = pPage->xCellSize(pPage, &data[pc]);
        أكد( heap!=0 && heap[0] < pCheck->mxHeap );
        btree_Heap_قىستۇرۇش(heap, (pc<<16)|(pc+size-1));
      }
    }
    أكد( heap!=0 );
    /* Add the freeblocks to the min-heap
    **
    ** EVIDENCE-OF: R-20690-50594 The second field of the b-tree page header
    ** is the offset of the first freeblock, or zero if there are no
    ** freeblocks on the page.
    */
    i = get2byte(&data[hdr+1]);
    طالما( i>0 ){
      صحيح size, j;
      أكد( (u32)i<=usableSize-4 ); /* Enforced by btreeComputeFreeSpace() */
      size = get2byte(&data[i+2]);
      أكد( (u32)(i+size)<=usableSize ); /* due to btreeComputeFreeSpace() */
      أكد( heap!=0 && heap[0] < pCheck->mxHeap );
      btree_Heap_قىستۇرۇش(heap, (((u32)i)<<16)|(i+size-1));
      /* EVIDENCE-OF: R-58208-19414 The first 2 bytes of a freeblock are a
      ** big-endian integer which is the offset in the b-tree page of the next
      ** freeblock in the chain, or zero if the freeblock is the last on the
      ** chain. */
      j = get2byte(&data[i]);
      /* EVIDENCE-OF: R-06866-39125 Freeblocks are always connected in order of
      ** increasing offset. */
      أكد( j==0 || j>i+size );     /* Enforced by btreeComputeFreeSpace() */
      أكد( (u32)j<=usableSize-4 ); /* Enforced by btreeComputeFreeSpace() */
      i = j;
    }
    /* Analyze the min-heap looking for overlap between cells and/or
    ** freeblocks, and counting the number of untracked bytes in nFrag.
    **
    ** Each min-heap entry is of the form:    (start_address<<16)|end_address.
    ** There is an implied first entry the covers the page header, the cell
    ** pointer index, and the gap between the cell pointer index and the start
    ** of cell content. 
    **
    ** The loop below pulls entries from the min-heap in order and compares
    ** the start_address against the previous end_address.  If there is an
    ** overlap, that means bytes are used multiple times.  If there is a gap,
    ** that gap is added to the fragmentation count.
    */
    nFrag = 0;
    prev = contentOffset - 1;   /* Implied first min-heap entry */
    طالما( btreeHeapPull(heap,&x) ){
      إذا( (prev&0xffff)>=(x>>16) ){
        check_Append_ئۇچۇر(pCheck,
          "Multiple uses for byte %u of page %u", x>>16, iPage);
        اكسر;
      }وإلا{
        nFrag += (x>>16) - (prev&0xffff) - 1;
        prev = x;
      }
    }
    nFrag += usableSize - (prev&0xffff) - 1;
    /* EVIDENCE-OF: R-43263-13491 The total number of bytes in all fragments
    ** is stored in the fifth field of the b-tree page header.
    ** EVIDENCE-OF: R-07161-27322 The one-byte integer at offset 7 gives the
    ** number of fragmented free bytes within the cell content area.
    */
    إذا( heap[0]==0 && nFrag!=data[hdr+7] ){
      check_Append_ئۇچۇر(pCheck,
          "Fragmentation of %u bytes reported as %u on page %u",
          nFrag, data[hdr+7], iPage);
    }
  }

end_of_check:
  إذا( !doCoverageCheck ) pPage->isInit = savedIsInit;
  releasePage(pPage);
  pCheck->zPfx = saved_zPfx;
  pCheck->v1 = saved_v1;
  pCheck->v2 = saved_v2;
  pCheck->nAbove--;
  أرجع depth+1;
}
#endif /* SQLITE_OMIT_INTEGRITY_CHECK */

#ifndef SQLITE_OMIT_INTEGRITY_CHECK
/*
** This routine does a complete check of the given BTree file.  aRoot[] is
** an array of pages numbers were each page number is the root page of
** a table.  nRoot is the number of entries in aRoot.
**
** A read-only or read-write transaction must be opened before calling
** this function.
**
** Write the number of error seen in *pnErr.  Except for some memory
** allocation errors,  an error message held in memory obtained from
** malloc is returned if *pnErr is non-zero.  If *pnErr==0 then NULL is
** returned.  If a memory allocation error occurs, NULL is returned.
**
** If the first entry in aRoot[] is 0, that indicates that the list of
** root pages is incomplete.  This is a "partial integrity-check".  This
** happens when performing an integrity check on a single table.  The
** zero is skipped, of course.  But in addition, the freelist checks
** and the checks to make sure every page is referenced are also skipped,
** since obviously it is not possible to know which pages are covered by
** the unverified btrees.  Except, if aRoot[1] is 1, then the freelist
** checks are still performed.
*/
صحيح sqlite3BtreeIntegrityCheck(
  sqlite3 *db,  /* Database connection that is running the check */
  Btree *p,     /* The btree to be checked */
  Pgno *a_يىلتىز,  /* An array of root pages numbers for individual trees */
  Mem *aCnt,    /* Memory cells to write counts for each tree to */
  صحيح n_يىلتىز,    /* Number of entries in aRoot[] */
  صحيح mxErr,    /* Stop reporting errors after this many */
  صحيح *pnErr,   /* OUT: Write number of errors seen to this variable */
  حرف **pzOut  /* OUT: Write the error message string here */
){
  Pgno i;
  IntegrityCk sCheck;
  BtShared *pBt = p->pBt;
  u64 savedDbFlags = pBt->db->flags;
  حرف zErr[100];
  صحيح bPartial = 0;            /* True if not checking all btrees */
  صحيح bCkFreelist = 1;         /* True to scan the freelist */
  VVA_ONLY( صحيح nRef );

  أكد( n_يىلتىز>0 );
  أكد( aCnt!=0 );

  /* aRoot[0]==0 means this is a partial check */
  إذا( a_يىلتىز[0]==0 ){
    أكد( n_يىلتىز>1 );
    bPartial = 1;
    إذا( a_يىلتىز[1]!=1 ) bCkFreelist = 0;
  }

  sqlite3BtreeEnter(p);
  أكد( p->inTrans>TRANS_NONE && pBt->inTransaction>TRANS_NONE );
  VVA_ONLY( nRef = sqlite3PagerRefcount(pBt->pPager) );
  أكد( nRef>=0 );
  memset(&sCheck, 0, حجم(sCheck));
  sCheck.db = db;
  sCheck.pBt = pBt;
  sCheck.pPager = pBt->pPager;
  sCheck.nCkPage = btreePagecount(sCheck.pBt);
  sCheck.mxErr = mxErr;
  sqlite3StrAccumInit(&sCheck.err_ئۇچۇر, 0, zErr, حجم(zErr), SQLITE_MAX_LENGTH);
  sCheck.err_ئۇچۇر.printfFlags = SQLITE_PRINTF_INTERNAL;
  إذا( sCheck.nCkPage==0 ){
    اذهب integrity_ck_cleanup;
  }

  sCheck.aPgRef = sqlite3MallocZero((sCheck.nCkPage / 8)+ 1);
  إذا( !sCheck.aPgRef ){
    checkOom(&sCheck);
    اذهب integrity_ck_cleanup;
  }
  sCheck.heap = (u32*)sqlite3Malloc( pBt->pageSize*2 );
#ifdef SQLITE_DEBUG
  sCheck.mxHeap = pBt->pageSize/2 - 1;
#endif
  إذا( sCheck.heap==0 ){
    checkOom(&sCheck);
    اذهب integrity_ck_cleanup;
  }

  i = PENDING_BYTE_PAGE(pBt);
  إذا( i<=sCheck.nCkPage ) setPageReferenced(&sCheck, i);

  /* Check the integrity of the freelist
  */
  إذا( bCkFreelist ){
    sCheck.zPfx = "Freelist: ";
    checkList(&sCheck, 1, get4byte(&pBt->pPage1->aData[32]),
              get4byte(&pBt->pPage1->aData[36]));
    sCheck.zPfx = 0;
  }

  /* Check all the tables.
  */
#ifndef SQLITE_OMIT_AUTOVACUUM
  إذا( !bPartial ){
    إذا( pBt->autoVacuum ){
      Pgno mx = 0;
      Pgno mxInHdr;
      لكل(i=0; (صحيح)i<n_يىلتىز; i++) إذا( mx<a_يىلتىز[i] ) mx = a_يىلتىز[i];
      mxInHdr = get4byte(&pBt->pPage1->aData[52]);
      إذا( mx!=mxInHdr ){
        check_Append_ئۇچۇر(&sCheck,
          "max rootpage (%u) disagrees with header (%u)",
          mx, mxInHdr
        );
      }
    }وإلا إذا( get4byte(&pBt->pPage1->aData[64])!=0 ){
      check_Append_ئۇچۇر(&sCheck,
        "incremental_vacuum enabled with a max rootpage of zero"
      );
    }
  }
#endif
  testcase( pBt->db->flags & SQLITE_CellSizeCk );
  pBt->db->flags &= ~(u64)SQLITE_CellSizeCk;
  لكل(i=0; (صحيح)i<n_يىلتىز && sCheck.mxErr; i++){
    sCheck.nRow = 0;
    إذا( a_يىلتىز[i] ){
      i64 notUsed;
#ifndef SQLITE_OMIT_AUTOVACUUM
      إذا( pBt->autoVacuum && a_يىلتىز[i]>1 && !bPartial ){
        checkPtrmap(&sCheck, a_يىلتىز[i], PTRMAP_ROOTPAGE, 0);
      }
#endif
      sCheck.v0 = a_يىلتىز[i];
      أكد( sCheck.nAbove==0 );
      check_دەرەخ_Page(&sCheck, a_يىلتىز[i], &notUsed, LARGEST_INT64);
      أكد( sCheck.nAbove==0 );
    }
    sqlite3MemSetArrayInt64(aCnt, i, sCheck.nRow);
  }
  pBt->db->flags = savedDbFlags;

  /* Make sure every page in the file is referenced
  */
  إذا( !bPartial ){
    لكل(i=1; i<=sCheck.nCkPage && sCheck.mxErr; i++){
#ifdef SQLITE_OMIT_AUTOVACUUM
      إذا( getPageReferenced(&sCheck, i)==0 ){
        check_Append_ئۇچۇر(&sCheck, "Page %u: never used", i);
      }
#وإلا
      /* If the database supports auto-vacuum, make sure no tables contain
      ** references to pointer-map pages.
      */
      إذا( getPageReferenced(&sCheck, i)==0 &&
         (PTRMAP_PAGENO(pBt, i)!=i || !pBt->autoVacuum) ){
        check_Append_ئۇچۇر(&sCheck, "Page %u: never used", i);
      }
      إذا( getPageReferenced(&sCheck, i)!=0 &&
         (PTRMAP_PAGENO(pBt, i)==i && pBt->autoVacuum) ){
        check_Append_ئۇچۇر(&sCheck, "Page %u: pointer map referenced", i);
      }
#endif
    }
  }

  /* Clean  up and report errors.
  */
integrity_ck_cleanup:
  sqlite3_free(sCheck.heap);
  sqlite3_free(sCheck.aPgRef);
  *pnErr = sCheck.nErr;
  إذا( sCheck.nErr==0 ){
    sqlite3_str_reset(&sCheck.err_ئۇچۇر);
    *pzOut = 0;
  }وإلا{
    *pzOut = sqlite3StrAccumFinish(&sCheck.err_ئۇچۇر);
  }
  /* Make sure this analysis did not leave any unref() pages. */
  أكد( nRef==sqlite3PagerRefcount(pBt->pPager) );
  sqlite3BtreeLeave(p);
  أرجع sCheck.rc;
}
#endif /* SQLITE_OMIT_INTEGRITY_CHECK */

/*
** Return the full pathname of the underlying database file.  Return
** an empty string if the database is in-memory or a TEMP database.
**
** The pager filename is invariant as long as the pager is
** open so it is safe to access without the BtShared mutex.
*/
ثابت حرف *sqlite3BtreeGetFilename(Btree *p){
  أكد( p->pBt->pPager!=0 );
  أرجع sqlite3PagerFilename(p->pBt->pPager, 1);
}

/*
** Return the pathname of the journal file for this database. The return
** value of this routine is the same regardless of whether the journal file
** has been created or not.
**
** The pager journal filename is invariant as long as the pager is
** open so it is safe to access without the BtShared mutex.
*/
ثابت حرف *sqlite3BtreeGetJournalname(Btree *p){
  أكد( p->pBt->pPager!=0 );
  أرجع sqlite3PagerJournalname(p->pBt->pPager);
}

/*
** Return one of SQLITE_TXN_NONE, SQLITE_TXN_READ, or SQLITE_TXN_WRITE
** to describe the current transaction state of Btree p.
*/
صحيح sqlite3BtreeTxnState(Btree *p){
  أكد( p==0 || sqlite3_mutex_held(p->db->mutex) );
  أرجع p ? p->inTrans : 0;
}

#ifndef SQLITE_OMIT_WAL
/*
** Run a checkpoint on the Btree passed as the first argument.
**
** Return SQLITE_LOCKED if this or any other connection has an open
** transaction on the shared-cache the argument Btree is connected to.
**
** Parameter eMode is one of SQLITE_CHECKPOINT_PASSIVE, FULL or RESTART.
*/
صحيح sqlite3BtreeCheckpoint(Btree *p, صحيح eMode, صحيح *pnLog, صحيح *pnCkpt){
  صحيح rc = SQLITE_OK;
  إذا( p ){
    BtShared *pBt = p->pBt;
    sqlite3BtreeEnter(p);
    إذا( pBt->inTransaction!=TRANS_NONE ){
      rc = SQLITE_LOCKED;
    }وإلا{
      rc = sqlite3PagerCheckpoint(pBt->pPager, p->db, eMode, pnLog, pnCkpt);
    }
    sqlite3BtreeLeave(p);
  }
  أرجع rc;
}
#endif

/*
** Return true if there is currently a backup running on Btree p.
*/
صحيح sqlite3BtreeIsInBackup(Btree *p){
  أكد( p );
  أكد( sqlite3_mutex_held(p->db->mutex) );
  أرجع p->nBackup!=0;
}

/*
** This function returns a pointer to a blob of memory associated with
** a single shared-btree. The memory is used by client code for its own
** purposes (for example, to store a high-level schema associated with
** the shared-btree). The btree layer manages reference counting issues.
**
** The first time this is called on a shared-btree, nBytes bytes of memory
** are allocated, zeroed, and returned to the caller. For each subsequent
** call the nBytes parameter is ignored and a pointer to the same blob
** of memory returned.
**
** If the nBytes parameter is 0 and the blob of memory has not yet been
** allocated, a null pointer is returned. If the blob has already been
** allocated, it is returned as normal.
**
** Just before the shared-btree is closed, the function passed as the
** xFree argument when the memory allocation was made is invoked on the
** blob of allocated memory. The xFree function should not call sqlite3_free()
** on the memory, the btree layer does that.
*/
فراغ *sqlite3BtreeSchema(Btree *p, صحيح nBytes, فراغ(*xFree)(فراغ *)){
  BtShared *pBt = p->pBt;
  أكد( nBytes==0 || nBytes==حجم(Schema) );
  sqlite3BtreeEnter(p);
  إذا( !pBt->pSchema && nBytes ){
    pBt->pSchema = sqlite3DbMallocZero(0, nBytes);
    pBt->xFreeSchema = xFree;
  }
  sqlite3BtreeLeave(p);
  أرجع pBt->pSchema;
}

/*
** Return SQLITE_LOCKED_SHAREDCACHE if another user of the same shared
** btree as the argument handle holds an exclusive lock on the
** sqlite_schema table. Otherwise SQLITE_OK.
*/
صحيح sqlite3BtreeSchemaLocked(Btree *p){
  صحيح rc;
  UNUSED_PARAMETER(p);  /* only used in DEBUG builds */
  أكد( sqlite3_mutex_held(p->db->mutex) );
  sqlite3BtreeEnter(p);
  rc = querySharedCacheTableLock(p, SCHEMA_يىلتىز, READ_LOCK);
  أكد( rc==SQLITE_OK || rc==SQLITE_LOCKED_SHAREDCACHE );
  sqlite3BtreeLeave(p);
  أرجع rc;
}


#ifndef SQLITE_OMIT_SHARED_CACHE
/*
** Obtain a lock on the table whose root page is iTab.  The
** lock is a write lock if isWritelock is true or a read lock
** if it is false.
*/
صحيح sqlite3BtreeLockTable(Btree *p, صحيح iTab, u8 isWriteLock){
  صحيح rc = SQLITE_OK;
  أكد( p->inTrans!=TRANS_NONE );
  إذا( p->sharable ){
    u8 lockType = READ_LOCK + isWriteLock;
    أكد( READ_LOCK+1==WRITE_LOCK );
    أكد( isWriteLock==0 || isWriteLock==1 );

    sqlite3BtreeEnter(p);
    rc = querySharedCacheTableLock(p, iTab, lockType);
    إذا( rc==SQLITE_OK ){
      rc = setSharedCacheTableLock(p, iTab, lockType);
    }
    sqlite3BtreeLeave(p);
  }
  أرجع rc;
}
#endif

#ifndef SQLITE_OMIT_INCRBLOB
/*
** Argument pCsr must be a cursor opened for writing on an
** INTKEY table currently pointing at a valid table entry.
** This function modifies the data stored as part of that entry.
**
** Only the data content may only be modified, it is not possible to
** change the length of the data stored. If this function is called with
** parameters that attempt to write past the end of the existing data,
** no modifications are made and SQLITE_CORRUPT is returned.
*/
صحيح sqlite3BtreePutData(BtCursor *pCsr, u32 offset, u32 amt, فراغ *z){
  صحيح rc;
  أكد( cursorOwnsBtShared(pCsr) );
  أكد( sqlite3_mutex_held(pCsr->pBtree->db->mutex) );
  أكد( pCsr->curFlags & BTCF_Incrblob );

  rc = restoreCursorPosition(pCsr);
  إذا( rc!=SQLITE_OK ){
    أرجع rc;
  }
  أكد( pCsr->eState!=CURSOR_REQUIRESEEK );
  إذا( pCsr->eState!=CURSOR_VALID ){
    أرجع SQLITE_ABORT;
  }

  /* Save the positions of all other cursors open on this table. This is
  ** required in case any of them are holding references to an xFetch
  ** version of the b-tree page modified by the accessPayload call below.
  **
  ** Note that pCsr must be open on a INTKEY table and saveCursorPosition()
  ** and hence saveAllCursors() cannot fail on a BTREE_INTKEY table, hence
  ** saveAllCursors can only return SQLITE_OK.
  */
  VVA_ONLY(rc =) saveAllCursors(pCsr->pBt, pCsr->pgno_يىلتىز, pCsr);
  أكد( rc==SQLITE_OK );

  /* Check some assumptions:
  **   (a) the cursor is open for writing,
  **   (b) there is a read/write transaction open,
  **   (c) the connection holds a write-lock on the table (if required),
  **   (d) there are no conflicting read-locks, and
  **   (e) the cursor points at a valid row of an intKey table.
  */
  إذا( (pCsr->curFlags & BTCF_WriteFlag)==0 ){
    أرجع SQLITE_READONLY;
  }
  أكد( (pCsr->pBt->btsFlags & BTS_READ_ONLY)==0
              && pCsr->pBt->inTransaction==TRANS_WRITE );
  أكد( hasSharedCacheTableLock(pCsr->pBtree, pCsr->pgno_يىلتىز, 0, 2) );
  أكد( !hasReadConflicts(pCsr->pBtree, pCsr->pgno_يىلتىز) );
  أكد( pCsr->pPage->int_ئاچقۇچ );

  أرجع accessPayload(pCsr, offset, amt, (unsigned حرف *)z, 1);
}

/*
** Mark this cursor as an incremental blob cursor.
*/
فراغ sqlite3BtreeIncrblobCursor(BtCursor *pCur){
  pCur->curFlags |= BTCF_Incrblob;
  pCur->pBtree->hasIncrblobCur = 1;
}
#endif

/*
** Set both the "read version" (single byte at byte offset 18) and
** "write version" (single byte at byte offset 19) fields in the database
** header to iVersion.
*/
صحيح sqlite3BtreeSetVersion(Btree *pBtree, صحيح iVersion){
  BtShared *pBt = pBtree->pBt;
  صحيح rc;                         /* Return code */

  أكد( iVersion==1 || iVersion==2 );

  /* If setting the version fields to 1, do not automatically open the
  ** WAL connection, even if the version fields are currently set to 2.
  */
  pBt->btsFlags &= ~BTS_NO_WAL;
  إذا( iVersion==1 ) pBt->btsFlags |= BTS_NO_WAL;

  rc = sqlite3BtreeBeginTrans(pBtree, 0, 0);
  إذا( rc==SQLITE_OK ){
    u8 *aData = pBt->pPage1->aData;
    إذا( aData[18]!=(u8)iVersion || aData[19]!=(u8)iVersion ){
      rc = sqlite3BtreeBeginTrans(pBtree, 2, 0);
      إذا( rc==SQLITE_OK ){
        rc = sqlite3PagerWrite(pBt->pPage1->pDbPage);
        إذا( rc==SQLITE_OK ){
          aData[18] = (u8)iVersion;
          aData[19] = (u8)iVersion;
        }
      }
    }
  }

  pBt->btsFlags &= ~BTS_NO_WAL;
  أرجع rc;
}

/*
** Return true if the cursor has a hint specified.  This routine is
** only used from within assert() statements
*/
صحيح sqlite3BtreeCursorHasHint(BtCursor *pCsr, unsigned صحيح mask){
  أرجع (pCsr->hints & mask)!=0;
}

/*
** Return true if the given Btree is read-only.
*/
صحيح sqlite3BtreeIsReadonly(Btree *p){
  أرجع (p->pBt->btsFlags & BTS_READ_ONLY)!=0;
}

/*
** Return the size of the header added to each page by this module.
*/
صحيح sqlite3HeaderSizeBtree(فراغ){ أرجع ROUND8(حجم(MemPage)); }

/*
** If no transaction is active and the database is not a temp-db, clear
** the in-memory pager cache.
*/
فراغ sqlite3BtreeClearCache(Btree *p){
  BtShared *pBt = p->pBt;
  إذا( pBt->inTransaction==TRANS_NONE ){
    sqlite3PagerClearCache(pBt->pPager);
  }
}

#إذا !defined(SQLITE_OMIT_SHARED_CACHE)
/*
** Return true if the Btree passed as the only argument is sharable.
*/
صحيح sqlite3BtreeSharable(Btree *p){
  أرجع p->sharable;
}

/*
** Return the number of connections to the BtShared object accessed by
** the Btree handle passed as the only argument. For private caches
** this is always 1. For shared caches it may be 1 or greater.
*/
صحيح sqlite3_Btree_Connection_سانى(Btree *p){
  testcase( p->sharable );
  أرجع p->pBt->nRef;
}
#endif

/* Re-enable GCC compiler warnings that were suppressed at the top
** of this source file to prevent annoying false-positives.
*/
#إذا defined(__GNUC__) && __GNUC__>=11
# pragma GCC diagnostic چىقىرىش
#endif
