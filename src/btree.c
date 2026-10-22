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
#அந்தர்பாவ "btreeInt.h"

/*
** Suppress false-positive compiler warnings from GCC.  Warnings are
** re-enabled at the bottom of this source file.
*/
#எனில் defined(__GNUC__) && __GNUC__>=11
# pragma GCC diagnostic தள்ளு
# pragma GCC diagnostic ignored "-Wstringop-overread"
# pragma GCC diagnostic ignored "-Wstringop-overflow"
#endif

/*
** The header string that appears at the beginning of every
** SQLite database.
*/
நிலையான மாறிலி எழுத்து zMagicHeader[] = SQLITE_FILE_HEADER;

/*
** Set this global variable to 1 to enable tracing using the TRACE
** macro.
*/
#எனில் 0
முழு sqlite3BtreeTrace=1;  /* True to enable tracing */
# define TRACE(X)  எனில்(sqlite3BtreeTrace){printf X;fflush(stdout);}
#இல்லையெனில்
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
#define get2byteNotZero(X)  (((((முழு)get2byte(X))-1)&0xffff)+1)

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
#define IfNotOmitAV(கோவை_2) (கோவை_2)
#இல்லையெனில்
#define IfNotOmitAV(கோவை_2) 0
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
#இல்லையெனில்
நிலையான BtShared *SQLITE_WSD sqlite3SharedCacheList = 0;
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
முழு sqlite3_enable_shared_cache(முழு enable){
  sqlite3GlobalConfig.sharedCacheEnabled = enable;
  திரும்பு SQLITE_OK;
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
sqlite3_uint64 sqlite3_Btree_Seek_மொத்தம்(Btree *pBt){
  u64 n =  pBt->nSeek;
  pBt->nSeek = 0;
  திரும்பு n;
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
முழு corruptPageError(முழு lineno, MemPage *p){
  எழுத்து *z_செய்தி;
  sqlite3BeginBenignMalloc();
  z_செய்தி = sqlite3_mprintf("database corruption page %u of %s",
             p->pgno, sqlite3PagerFilename(p->pBt->pPager, 0)
  );
  sqlite3EndBenignMalloc();
  எனில்( z_செய்தி ){
    sqlite3ReportError(SQLITE_CORRUPT, lineno, z_செய்தி);
  }
  sqlite3_free(z_செய்தி);
  திரும்பு SQLITE_CORRUPT_BKPT;
}
# define SQLITE_CORRUPT_PAGE(pMemPage) corruptPageError(__LINE__, pMemPage)
#இல்லையெனில்
# define SQLITE_CORRUPT_PAGE(pMemPage) SQLITE_CORRUPT_PGNO(pMemPage->pgno)
#endif

/* Default value for SHARED_LOCK_TRACE macro if shared-cache is disabled
** or if the lock tracking is disabled.  This is always the value for
** release builds.
*/
#define SHARED_LOCK_TRACE(X,செய்தி,TAB,TYPE)  /*no-op*/

#ifndef SQLITE_OMIT_SHARED_CACHE

#எனில் 0
/*  ^----  Change to 1 and recompile to enable shared-lock tracing
**         for debugging purposes.
**
** Print all shared-cache locks on a BtShared.  Debugging use only.
*/
நிலையான வெறுமை sharedLockTrace(
  BtShared *pBt,
  மாறிலி எழுத்து *z_செய்தி,
  முழு i_வேர்,
  முழு eLockType
){
  BtLock *pLock;
  எனில்( i_வேர்>0 ){
    printf("%s-%p %u%s:", z_செய்தி, pBt, i_வேர், eLockType==READ_LOCK?"R":"W");
  }இல்லையெனில்{
    printf("%s-%p:", z_செய்தி, pBt);
  }
  க்கு(pLock=pBt->pLock; pLock; pLock=pLock->pNext){
    printf(" %p/%u%s", pLock->pBtree, pLock->iTable,
           pLock->eLock==READ_LOCK ? "R" : "W");
    போது( pLock->pNext && pLock->pBtree==pLock->pNext->pBtree ){
      pLock = pLock->pNext;
      printf(",%u%s", pLock->iTable, pLock->eLock==READ_LOCK ? "R" : "W");
    }
  }
  printf("\n");
  fflush(stdout);
}
#undef SHARED_LOCK_TRACE
#define SHARED_LOCK_TRACE(X,செய்தி,TAB,TYPE)  sharedLockTrace(X,செய்தி,TAB,TYPE)
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
நிலையான முழு hasSharedCacheTableLock(
  Btree *pBtree,         /* Handle that must hold lock */
  Pgno i_வேர்,            /* Root page of b-tree */
  முழு isIndex,           /* True if iRoot is the root of an index b-tree */
  முழு eLockType          /* Required lock type (READ_LOCK or WRITE_LOCK) */
){
  Schema *pSchema = (Schema *)pBtree->pBt->pSchema;
  Pgno iTab = 0;
  BtLock *pLock;

  /* If this database is not shareable, or if the client is reading
  ** and has the read-uncommitted flag set, then no lock is required.
  ** Return true immediately.
  */
  எனில்( (pBtree->sharable==0)
   || (eLockType==READ_LOCK && (pBtree->db->flags & SQLITE_ReadUncommit))
  ){
    திரும்பு 1;
  }

  /* If the client is reading  or writing an index and the schema is
  ** not loaded, then it is too difficult to actually check to see if
  ** the correct locks are held.  So do not bother - just return true.
  ** This case does not come up very often anyhow.
  */
  எனில்( isIndex && (!pSchema || (pSchema->schemaFlags&DB_SchemaLoaded)==0) ){
    திரும்பு 1;
  }

  /* Figure out the root-page that the lock should be held on. For table
  ** b-trees, this is just the root page of the b-tree being read or
  ** written. For index b-trees, it is the root page of the associated
  ** table.  */
  எனில்( isIndex ){
    HashElem *p;
    முழு bSeen = 0;
    க்கு(p=sqliteHashFirst(&pSchema->idxHash); p; p=sqliteHashNext(p)){
      Index *pIdx = (Index *)sqliteHashData(p);
      எனில்( pIdx->tnum==i_வேர் ){
        எனில்( bSeen ){
          /* Two or more indexes share the same root page.  There must
          ** be imposter tables.  So just return true.  The assert is not
          ** useful in that case. */
          திரும்பு 1;
        }
        iTab = pIdx->pTable->tnum;
        bSeen = 1;
      }
    }
  }இல்லையெனில்{
    iTab = i_வேர்;
  }

  SHARED_LOCK_TRACE(pBtree->pBt,"hasLock",i_வேர்,eLockType);

  /* Search for the required lock. Either a write-lock on root-page iTab, a
  ** write-lock on the schema table, or (if the client is reading) a
  ** read-lock on iTab will suffice. Return 1 if any of these are found.  */
  க்கு(pLock=pBtree->pBt->pLock; pLock; pLock=pLock->pNext){
    எனில்( pLock->pBtree==pBtree
     && (pLock->iTable==iTab || (pLock->eLock==WRITE_LOCK && pLock->iTable==1))
     && pLock->eLock>=eLockType
    ){
      திரும்பு 1;
    }
  }

  /* Failed to find the required lock. */
  திரும்பு 0;
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
நிலையான முழு hasReadConflicts(Btree *pBtree, Pgno i_வேர்){
  BtCursor *p;
  க்கு(p=pBtree->pBt->pCursor; p; p=p->pNext){
    எனில்( p->pgno_வேர்==i_வேர்
     && p->pBtree!=pBtree
     && 0==(p->pBtree->db->flags & SQLITE_ReadUncommit)
    ){
      திரும்பு 1;
    }
  }
  திரும்பு 0;
}
#endif    /* #ifdef SQLITE_DEBUG */

/*
** Query to see if Btree handle p may obtain a lock of type eLock
** (READ_LOCK or WRITE_LOCK) on the table with root-page iTab. Return
** SQLITE_OK if the lock may be obtained (by calling
** setSharedCacheTableLock()), or SQLITE_LOCKED if not.
*/
நிலையான முழு querySharedCacheTableLock(Btree *p, Pgno iTab, u8 eLock){
  BtShared *pBt = p->pBt;
  BtLock *pIter;

  ப்ரதிஜ்ஞா( sqlite3BtreeHoldsMutex(p) );
  ப்ரதிஜ்ஞா( eLock==READ_LOCK || eLock==WRITE_LOCK );
  ப்ரதிஜ்ஞா( p->db!=0 );
  ப்ரதிஜ்ஞா( !(p->db->flags&SQLITE_ReadUncommit)||eLock==WRITE_LOCK||iTab==1 );
 
  /* If requesting a write-lock, then the Btree must have an open write
  ** transaction on this file. And, obviously, for this to be so there
  ** must be an open write transaction on the file itself.
  */
  ப்ரதிஜ்ஞா( eLock==READ_LOCK || (p==pBt->pWriter && p->inTrans==TRANS_WRITE) );
  ப்ரதிஜ்ஞா( eLock==READ_LOCK || pBt->inTransaction==TRANS_WRITE );
 
  /* This routine is a no-op if the shared-cache is not enabled */
  எனில்( !p->sharable ){
    திரும்பு SQLITE_OK;
  }

  /* If some other connection is holding an exclusive lock, the
  ** requested lock may not be obtained.
  */
  எனில்( pBt->pWriter!=p && (pBt->btsFlags & BTS_EXCLUSIVE)!=0 ){
    sqlite3ConnectionBlocked(p->db, pBt->pWriter->db);
    திரும்பு SQLITE_LOCKED_SHAREDCACHE;
  }

  க்கு(pIter=pBt->pLock; pIter; pIter=pIter->pNext){
    /* The condition (pIter->eLock!=eLock) in the following if(...)
    ** statement is a simplification of:
    **
    **   (eLock==WRITE_LOCK || pIter->eLock==WRITE_LOCK)
    **
    ** since we know that if eLock==WRITE_LOCK, then no other connection
    ** may hold a WRITE_LOCK on any table in this file (since there can
    ** only be a single writer).
    */
    ப்ரதிஜ்ஞா( pIter->eLock==READ_LOCK || pIter->eLock==WRITE_LOCK );
    ப்ரதிஜ்ஞா( eLock==READ_LOCK || pIter->pBtree==p || pIter->eLock==READ_LOCK);
    எனில்( pIter->pBtree!=p && pIter->iTable==iTab && pIter->eLock!=eLock ){
      sqlite3ConnectionBlocked(p->db, pIter->pBtree->db);
      எனில்( eLock==WRITE_LOCK ){
        ப்ரதிஜ்ஞா( p==pBt->pWriter );
        pBt->btsFlags |= BTS_PENDING;
      }
      திரும்பு SQLITE_LOCKED_SHAREDCACHE;
    }
  }
  திரும்பு SQLITE_OK;
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
நிலையான முழு setSharedCacheTableLock(Btree *p, Pgno iTable, u8 eLock){
  BtShared *pBt = p->pBt;
  BtLock *pLock = 0;
  BtLock *pIter;

  SHARED_LOCK_TRACE(pBt,"setLock", iTable, eLock);

  ப்ரதிஜ்ஞா( sqlite3BtreeHoldsMutex(p) );
  ப்ரதிஜ்ஞா( eLock==READ_LOCK || eLock==WRITE_LOCK );
  ப்ரதிஜ்ஞா( p->db!=0 );

  /* A connection with the read-uncommitted flag set will never try to
  ** obtain a read-lock using this function. The only read-lock obtained
  ** by a connection in read-uncommitted mode is on the sqlite_schema
  ** table, and that lock is obtained in BtreeBeginTrans().  */
  ப்ரதிஜ்ஞா( 0==(p->db->flags&SQLITE_ReadUncommit) || eLock==WRITE_LOCK );

  /* This function should only be called on a sharable b-tree after it
  ** has been determined that no other b-tree holds a conflicting lock.  */
  ப்ரதிஜ்ஞா( p->sharable );
  ப்ரதிஜ்ஞா( SQLITE_OK==querySharedCacheTableLock(p, iTable, eLock) );

  /* First search the list for an existing lock on this table. */
  க்கு(pIter=pBt->pLock; pIter; pIter=pIter->pNext){
    எனில்( pIter->iTable==iTable && pIter->pBtree==p ){
      pLock = pIter;
      நிறுத்து;
    }
  }

  /* If the above search did not find a BtLock struct associating Btree p
  ** with table iTable, allocate one and link it into the list.
  */
  எனில்( !pLock ){
    pLock = (BtLock *)sqlite3MallocZero(அளவு(BtLock));
    எனில்( !pLock ){
      திரும்பு SQLITE_NOMEM_BKPT;
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
  ப்ரதிஜ்ஞா( WRITE_LOCK>READ_LOCK );
  எனில்( eLock>pLock->eLock ){
    pLock->eLock = eLock;
  }

  திரும்பு SQLITE_OK;
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
நிலையான வெறுமை clearAllSharedCacheTableLocks(Btree *p){
  BtShared *pBt = p->pBt;
  BtLock **ppIter = &pBt->pLock;

  ப்ரதிஜ்ஞா( sqlite3BtreeHoldsMutex(p) );
  ப்ரதிஜ்ஞா( p->sharable || 0==*ppIter );
  ப்ரதிஜ்ஞா( p->inTrans>0 );

  SHARED_LOCK_TRACE(pBt, "clearAllLocks", 0, 0);

  போது( *ppIter ){
    BtLock *pLock = *ppIter;
    ப்ரதிஜ்ஞா( (pBt->btsFlags & BTS_EXCLUSIVE)==0 || pBt->pWriter==pLock->pBtree );
    ப்ரதிஜ்ஞா( pLock->pBtree->inTrans>=pLock->eLock );
    எனில்( pLock->pBtree==p ){
      *ppIter = pLock->pNext;
      ப்ரதிஜ்ஞா( pLock->iTable!=1 || pLock==&p->lock );
      எனில்( pLock->iTable!=1 ){
        sqlite3_free(pLock);
      }
    }இல்லையெனில்{
      ppIter = &pLock->pNext;
    }
  }

  ப்ரதிஜ்ஞா( (pBt->btsFlags & BTS_PENDING)==0 || pBt->pWriter );
  எனில்( pBt->pWriter==p ){
    pBt->pWriter = 0;
    pBt->btsFlags &= ~(BTS_EXCLUSIVE|BTS_PENDING);
  }இல்லையெனில் எனில்( pBt->nTransaction==2 ){
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
நிலையான வெறுமை downgradeAllSharedCacheTableLocks(Btree *p){
  BtShared *pBt = p->pBt;

  SHARED_LOCK_TRACE(pBt, "downgradeLocks", 0, 0);

  எனில்( pBt->pWriter==p ){
    BtLock *pLock;
    pBt->pWriter = 0;
    pBt->btsFlags &= ~(BTS_EXCLUSIVE|BTS_PENDING);
    க்கு(pLock=pBt->pLock; pLock; pLock=pLock->pNext){
      ப்ரதிஜ்ஞா( pLock->eLock==READ_LOCK || pLock->pBtree==p );
      pLock->eLock = READ_LOCK;
    }
  }
}

#endif /* SQLITE_OMIT_SHARED_CACHE */

நிலையான வெறுமை releasePage(MemPage *pPage);         /* Forward reference */
நிலையான வெறுமை releasePageOne(MemPage *pPage);      /* Forward reference */
நிலையான வெறுமை releasePageNotNull(MemPage *pPage);  /* Forward reference */

/*
***** This routine is used inside of assert() only ****
**
** Verify that the cursor holds the mutex on its BtShared
*/
#ifdef SQLITE_DEBUG
நிலையான முழு cursorHoldsMutex(BtCursor *p){
  திரும்பு sqlite3_mutex_held(p->pBt->mutex);
}

/* Verify that the cursor and the BtShared agree about what is the current
** database connetion. This is important in shared-cache mode. If the database
** connection pointers get out-of-sync, it is possible for routines like
** btreeInitPage() to reference an stale connection pointer that references a
** a connection that has already closed.  This routine is used inside assert()
** statements only and for the purpose of double-checking that the btree code
** does keep the database connection pointers up-to-date.
*/
நிலையான முழு cursorOwnsBtShared(BtCursor *p){
  ப்ரதிஜ்ஞா( cursorHoldsMutex(p) );
  திரும்பு (p->pBtree->db==p->pBt->db);
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
நிலையான வெறுமை invalidateAllOverflowCache(BtShared *pBt){
  BtCursor *p;
  ப்ரதிஜ்ஞா( sqlite3_mutex_held(pBt->mutex) );
  க்கு(p=pBt->pCursor; p; p=p->pNext){
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
நிலையான வெறுமை invalidateIncrblobCursors(
  Btree *pBtree,          /* The database file to check */
  Pgno pgno_வேர்,          /* The table that might be changing */
  i64 iRow,               /* The rowid that might be changing */
  முழு isClearTable        /* True if all rows are being deleted */
){
  BtCursor *p;
  ப்ரதிஜ்ஞா( pBtree->hasIncrblobCur );
  ப்ரதிஜ்ஞா( sqlite3BtreeHoldsMutex(pBtree) );
  pBtree->hasIncrblobCur = 0;
  க்கு(p=pBtree->pBt->pCursor; p; p=p->pNext){
    எனில்( (p->curFlags & BTCF_Incrblob)!=0 ){
      pBtree->hasIncrblobCur = 1;
      எனில்( p->pgno_வேர்==pgno_வேர் && (isClearTable || p->info.n_சாவி==iRow) ){
        p->eState = CURSOR_INVALID;
      }
    }
  }
}

#இல்லையெனில்
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
நிலையான முழு btreeSetHasContent(BtShared *pBt, Pgno pgno){
  முழு rc = SQLITE_OK;
  எனில்( !pBt->pHasContent ){
    ப்ரதிஜ்ஞா( pgno<=pBt->nPage );
    pBt->pHasContent = sqlite3BitvecCreate(pBt->nPage);
    எனில்( !pBt->pHasContent ){
      rc = SQLITE_NOMEM_BKPT;
    }
  }
  எனில்( rc==SQLITE_OK && pgno<=sqlite3BitvecSize(pBt->pHasContent) ){
    rc = sqlite3BitvecSet(pBt->pHasContent, pgno);
  }
  திரும்பு rc;
}

/*
** Query the BtShared.pHasContent vector.
**
** This function is called when a free-list leaf page is removed from the
** free-list for reuse. It returns false if it is safe to retrieve the
** page from the pager layer with the 'no-content' flag set. True otherwise.
*/
நிலையான முழு btreeGetHasContent(BtShared *pBt, Pgno pgno){
  Bitvec *p = pBt->pHasContent;
  திரும்பு p && (pgno>sqlite3BitvecSize(p) || sqlite3BitvecTestNotNull(p, pgno));
}

/*
** Clear (destroy) the BtShared.pHasContent bitvec. This should be
** invoked at the conclusion of each write-transaction.
*/
நிலையான வெறுமை btreeClearHasContent(BtShared *pBt){
  sqlite3BitvecDestroy(pBt->pHasContent);
  pBt->pHasContent = 0;
}

/*
** Release all of the apPage[] pages for a cursor.
*/
நிலையான வெறுமை btreeReleaseAllCursorPages(BtCursor *pCur){
  முழு i;
  எனில்( pCur->iPage>=0 ){
    க்கு(i=0; i<pCur->iPage; i++){
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
நிலையான முழு save_Cursor_சாவி(BtCursor *pCur){
  முழு rc = SQLITE_OK;
  ப்ரதிஜ்ஞா( CURSOR_VALID==pCur->eState );
  ப்ரதிஜ்ஞா( 0==pCur->p_சாவி );
  ப்ரதிஜ்ஞா( cursorHoldsMutex(pCur) );

  எனில்( pCur->cur_Int_சாவி ){
    /* Only the rowid is required for a table btree */
    pCur->n_சாவி = sqlite3_Btree_Integer_சாவி(pCur);
  }இல்லையெனில்{
    /* For an index btree, save the complete key content. It is possible
    ** that the current key is corrupt. In that case, it is possible that
    ** the sqlite3VdbeRecordUnpack() function may overread the buffer by
    ** up to the size of 1 varint plus 1 8-byte value when the cursor
    ** position is restored. Hence the 17 bytes of padding allocated
    ** below. */
    வெறுமை *p_சாவி;
    pCur->n_சாவி = sqlite3BtreePayloadSize(pCur);
    p_சாவி = sqlite3Malloc( ((i64)pCur->n_சாவி) + 9 + 8 );
    எனில்( p_சாவி ){
      rc = sqlite3BtreePayload(pCur, 0, (முழு)pCur->n_சாவி, p_சாவி);
      எனில்( rc==SQLITE_OK ){
        memset(((u8*)p_சாவி)+pCur->n_சாவி, 0, 9+8);
        pCur->p_சாவி = p_சாவி;
      }இல்லையெனில்{
        sqlite3_free(p_சாவி);
      }
    }இல்லையெனில்{
      rc = SQLITE_NOMEM_BKPT;
    }
  }
  ப்ரதிஜ்ஞா( !pCur->cur_Int_சாவி || !pCur->p_சாவி );
  திரும்பு rc;
}

/*
** Save the current cursor position in the variables BtCursor.nKey
** and BtCursor.pKey. The cursor's state is set to CURSOR_REQUIRESEEK.
**
** The caller must ensure that the cursor is valid (has eState==CURSOR_VALID)
** prior to calling this routine. 
*/
நிலையான முழு saveCursorPosition(BtCursor *pCur){
  முழு rc;

  ப்ரதிஜ்ஞா( CURSOR_VALID==pCur->eState || CURSOR_SKIPNEXT==pCur->eState );
  ப்ரதிஜ்ஞா( 0==pCur->p_சாவி );
  ப்ரதிஜ்ஞா( cursorHoldsMutex(pCur) );

  எனில்( pCur->curFlags & BTCF_Pinned ){
    திரும்பு SQLITE_CONSTRAINT_PINNED;
  }
  எனில்( pCur->eState==CURSOR_SKIPNEXT ){
    pCur->eState = CURSOR_VALID;
  }இல்லையெனில்{
    pCur->skipNext = 0;
  }

  rc = save_Cursor_சாவி(pCur);
  எனில்( rc==SQLITE_OK ){
    btreeReleaseAllCursorPages(pCur);
    pCur->eState = CURSOR_REQUIRESEEK;
  }

  pCur->curFlags &= ~(BTCF_ValidNKey|BTCF_ValidOvfl|BTCF_AtLast);
  திரும்பு rc;
}

/* Forward reference */
நிலையான முழு SQLITE_NOINLINE saveCursorsOnList(BtCursor*,Pgno,BtCursor*);

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
நிலையான முழு saveAllCursors(BtShared *pBt, Pgno i_வேர், BtCursor *pExcept){
  BtCursor *p;
  ப்ரதிஜ்ஞா( sqlite3_mutex_held(pBt->mutex) );
  ப்ரதிஜ்ஞா( pExcept==0 || pExcept->pBt==pBt );
  க்கு(p=pBt->pCursor; p; p=p->pNext){
    எனில்( p!=pExcept && (0==i_வேர் || p->pgno_வேர்==i_வேர்) ) நிறுத்து;
  }
  எனில்( p ) திரும்பு saveCursorsOnList(p, i_வேர், pExcept);
  எனில்( pExcept ) pExcept->curFlags &= ~BTCF_Multiple;
  திரும்பு SQLITE_OK;
}

/* This helper routine to saveAllCursors does the actual work of saving
** the cursors if and when a cursor is found that actually requires saving.
** The common case is that no cursors need to be saved, so this routine is
** broken out from its caller to avoid unnecessary stack pointer movement.
*/
நிலையான முழு SQLITE_NOINLINE saveCursorsOnList(
  BtCursor *p,         /* The first cursor that needs saving */
  Pgno i_வேர்,          /* Only save cursor with this iRoot. Save all if zero */
  BtCursor *pExcept    /* Do not save this cursor */
){
  செய்{
    எனில்( p!=pExcept && (0==i_வேர் || p->pgno_வேர்==i_வேர்) ){
      எனில்( p->eState==CURSOR_VALID || p->eState==CURSOR_SKIPNEXT ){
        முழு rc = saveCursorPosition(p);
        எனில்( SQLITE_OK!=rc ){
          திரும்பு rc;
        }
      }இல்லையெனில்{
        testcase( p->iPage>=0 );
        btreeReleaseAllCursorPages(p);
      }
    }
    p = p->pNext;
  }போது( p );
  திரும்பு SQLITE_OK;
}

/*
** Clear the current cursor position.
*/
வெறுமை sqlite3BtreeClearCursor(BtCursor *pCur){
  ப்ரதிஜ்ஞா( cursorHoldsMutex(pCur) );
  sqlite3_free(pCur->p_சாவி);
  pCur->p_சாவி = 0;
  pCur->eState = CURSOR_INVALID;
}

/*
** In this version of BtreeMoveto, pKey is a packed index record
** such as is generated by the OP_MakeRecord opcode.  Unpack the
** record and then call sqlite3BtreeIndexMoveto() to do the work.
*/
நிலையான முழு btreeMoveto(
  BtCursor *pCur,     /* Cursor open on the btree to be searched */
  மாறிலி வெறுமை *p_சாவி,   /* Packed key if the btree is an index */
  i64 n_சாவி,           /* Integer key for tables.  Size of pKey for indices */
  முழு bias,           /* Bias search to the high end */
  முழு *pRes           /* Write search results here */
){
  முழு rc;                    /* Status code */
  UnpackedRecord *p_Idx_சாவி;   /* Unpacked index key */

  எனில்( p_சாவி ){
    சாவி_Info *p_சாவி_Info = pCur->p_சாவி_Info;
    ப்ரதிஜ்ஞா( n_சாவி==(i64)(முழு)n_சாவி );
    p_Idx_சாவி = sqlite3VdbeAllocUnpackedRecord(p_சாவி_Info);
    எனில்( p_Idx_சாவி==0 ) திரும்பு SQLITE_NOMEM_BKPT;
    sqlite3VdbeRecordUnpack((முழு)n_சாவி, p_சாவி, p_Idx_சாவி);
    எனில்( p_Idx_சாவி->nField==0 || p_Idx_சாவி->nField>p_சாவி_Info->nAllField ){
      rc = SQLITE_CORRUPT_BKPT;
    }இல்லையெனில்{
      rc = sqlite3BtreeIndexMoveto(pCur, p_Idx_சாவி, pRes);
    }
    sqlite3DbFree(pCur->p_சாவி_Info->db, p_Idx_சாவி);
  }இல்லையெனில்{
    p_Idx_சாவி = 0;
    rc = sqlite3BtreeTableMoveto(pCur, n_சாவி, bias, pRes);
  }
  திரும்பு rc;
}

/*
** Restore the cursor to the position it was in (or as close to as possible)
** when saveCursorPosition() was called. Note that this call deletes the
** saved position info stored by saveCursorPosition(), so there can be
** at most one effective restoreCursorPosition() call after each
** saveCursorPosition().
*/
நிலையான முழு btreeRestoreCursorPosition(BtCursor *pCur){
  முழு rc;
  முழு skipNext = 0;
  ப்ரதிஜ்ஞா( cursorOwnsBtShared(pCur) );
  ப்ரதிஜ்ஞா( pCur->eState>=CURSOR_REQUIRESEEK );
  எனில்( pCur->eState==CURSOR_FAULT ){
    திரும்பு pCur->skipNext;
  }
  pCur->eState = CURSOR_INVALID;
  எனில்( sqlite3FaultSim(410) ){
    rc = SQLITE_IOERR;
  }இல்லையெனில்{
    rc = btreeMoveto(pCur, pCur->p_சாவி, pCur->n_சாவி, 0, &skipNext);
  }
  எனில்( rc==SQLITE_OK ){
    sqlite3_free(pCur->p_சாவி);
    pCur->p_சாவி = 0;
    ப்ரதிஜ்ஞா( pCur->eState==CURSOR_VALID || pCur->eState==CURSOR_INVALID );
    எனில்( skipNext ) pCur->skipNext = skipNext;
    எனில்( pCur->skipNext && pCur->eState==CURSOR_VALID ){
      pCur->eState = CURSOR_SKIPNEXT;
    }
  }
  திரும்பு rc;
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
முழு sqlite3BtreeCursorHasMoved(BtCursor *pCur){
  ப்ரதிஜ்ஞா( EIGHT_BYTE_ALIGNMENT(pCur)
       || pCur==sqlite3BtreeFakeValidCursor() );
  ப்ரதிஜ்ஞா( offsetof(BtCursor, eState)==0 );
  ப்ரதிஜ்ஞா( அளவு(pCur->eState)==1 );
  திரும்பு CURSOR_VALID != *(u8*)pCur;
}

/*
** Return a pointer to a fake BtCursor object that will always answer
** false to the sqlite3BtreeCursorHasMoved() routine above.  The fake
** cursor returned must not be used with any other Btree interface.
*/
BtCursor *sqlite3BtreeFakeValidCursor(வெறுமை){
  நிலையான u8 fakeCursor = CURSOR_VALID;
  ப்ரதிஜ்ஞா( offsetof(BtCursor, eState)==0 );
  திரும்பு (BtCursor*)&fakeCursor;
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
முழு sqlite3BtreeCursorRestore(BtCursor *pCur, முழு *pDifferentRow){
  முழு rc;

  ப்ரதிஜ்ஞா( pCur!=0 );
  ப்ரதிஜ்ஞா( pCur->eState!=CURSOR_VALID );
  rc = restoreCursorPosition(pCur);
  எனில்( rc ){
    *pDifferentRow = 1;
    திரும்பு rc;
  }
  எனில்( pCur->eState!=CURSOR_VALID ){
    *pDifferentRow = 1;
  }இல்லையெனில்{
    *pDifferentRow = 0;
  }
  திரும்பு SQLITE_OK;
}

#ifdef SQLITE_ENABLE_CURSOR_HINTS
/*
** Provide hints to the cursor.  The particular hint given (and the type
** and number of the varargs parameters) is determined by the eHintType
** parameter.  See the definitions of the BTREE_HINT_* macros for details.
*/
வெறுமை sqlite3BtreeCursorHint(BtCursor *pCur, முழு eHintType, ...){
  /* Used only by system that substitute their own storage engine */
#ifdef SQLITE_DEBUG
  va_list ap;
  va_start(ap, eHintType);
  எனில்( eHintType==BTREE_HINT_RANGE ){
    கோவை *p_கோவை;
    Walker w;
    memset(&w, 0, அளவு(w));
    w.x_கோவை_Callback = sqlite3_Cursor_Range_Hint_கோவை_Check;
    p_கோவை = va_arg(ap, கோவை*);
    w.u.aMem = va_arg(ap, Mem*);
    ப்ரதிஜ்ஞா( p_கோவை!=0 );
    ப்ரதிஜ்ஞா( w.u.aMem!=0 );
    sqlite3_Walk_கோவை(&w, p_கோவை);
  }இல்லையெனில் எனில்( ALWAYS(eHintType==BTREE_HINT_TABLECURSOR) ){
    BtCursor *pCsr = va_arg(ap, BtCursor*);
    ப்ரதிஜ்ஞா( pCur->pCursorHintTableCursor==0 
         || pCur->pCursorHintTableCursor==pCsr
    );
    ப்ரதிஜ்ஞா( pCsr->p_சாவி_Info==0 || CORRUPT_DB );
    pCur->pCursorHintTableCursor = pCsr;
  }
  va_end(ap);
#endif /* SQLITE_DEBUG */
}
#endif /* SQLITE_ENABLE_CURSOR_HINTS */

#எனில் defined(SQLITE_ENABLE_CURSOR_HINTS) && defined(SQLITE_DEBUG)
/*
** Return the pointer configured via the BTREE_HINT_TABLECURSOR hint on
** cursor pCsr. This is used from OP_DeferredSeek to assert() that the
** index cursor has been correctly configured with the table cursor.
*/
BtCursor *sqlite3BtreeCursorHintTblCsr(BtCursor *pCsr){
  திரும்பு pCsr->pCursorHintTableCursor;
}
#endif

/*
** Provide flag hints to the cursor.
*/
வெறுமை sqlite3BtreeCursorHintFlags(BtCursor *pCur, unsigned x){
  ப்ரதிஜ்ஞா( x==BTREE_SEEK_EQ || x==BTREE_BULKLOAD || x==0 );
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
நிலையான Pgno ptrmapPageno(BtShared *pBt, Pgno pgno){
  முழு nPagesPerMapPage;
  Pgno iPtrMap, ret;
  ப்ரதிஜ்ஞா( sqlite3_mutex_held(pBt->mutex) );
  எனில்( pgno<2 ) திரும்பு 0;
  nPagesPerMapPage = (pBt->usableSize/5)+1;
  iPtrMap = (pgno-2)/nPagesPerMapPage;
  ret = (iPtrMap*nPagesPerMapPage) + 2;
  எனில்( ret==PENDING_BYTE_PAGE(pBt) ){
    ret++;
  }
  திரும்பு ret;
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
நிலையான வெறுமை ptrmapPut(BtShared *pBt, Pgno சாவி, u8 eType, Pgno parent, முழு *pRC){
  DbPage *pDbPage;  /* The pointer map page */
  u8 *pPtrmap;      /* The pointer map data */
  Pgno iPtrmap;     /* The pointer map page number */
  முழு offset;       /* Offset in pointer map page */
  முழு rc;           /* Return code from subfunctions */

  எனில்( *pRC ) திரும்பு;

  ப்ரதிஜ்ஞா( sqlite3_mutex_held(pBt->mutex) );
  /* The super-journal page number must never be used as a pointer map page */
  ப்ரதிஜ்ஞா( 0==PTRMAP_ISPAGE(pBt, PENDING_BYTE_PAGE(pBt)) );

  ப்ரதிஜ்ஞா( pBt->autoVacuum );
  எனில்( சாவி==0 ){
    *pRC = SQLITE_CORRUPT_BKPT;
    திரும்பு;
  }
  iPtrmap = PTRMAP_PAGENO(pBt, சாவி);
  rc = sqlite3PagerGet(pBt->pPager, iPtrmap, &pDbPage, 0);
  எனில்( rc!=SQLITE_OK ){
    *pRC = rc;
    திரும்பு;
  }
  எனில்( ((எழுத்து*)sqlite3PagerGetExtra(pDbPage))[0]!=0 ){
    /* The first byte of the extra data is the MemPage.isInit byte.
    ** If that byte is set, it means this page is also being used
    ** as a btree page. */
    *pRC = SQLITE_CORRUPT_BKPT;
    செல் ptrmap_exit;
  }
  offset = PTRMAP_PTROFFSET(iPtrmap, சாவி);
  எனில்( offset<0 ){
    *pRC = SQLITE_CORRUPT_BKPT;
    செல் ptrmap_exit;
  }
  ப்ரதிஜ்ஞா( offset <= (முழு)pBt->usableSize-5 );
  pPtrmap = (u8 *)sqlite3PagerGetData(pDbPage);

  எனில்( eType!=pPtrmap[offset] || get4byte(&pPtrmap[offset+1])!=parent ){
    TRACE(("PTRMAP_UPDATE: %u->(%u,%u)\n", சாவி, eType, parent));
    *pRC= rc = sqlite3PagerWrite(pDbPage);
    எனில்( rc==SQLITE_OK ){
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
நிலையான முழு ptrmapGet(BtShared *pBt, Pgno சாவி, u8 *pEType, Pgno *pPgno){
  DbPage *pDbPage;   /* The pointer map page */
  முழு iPtrmap;       /* Pointer map page index */
  u8 *pPtrmap;       /* Pointer map page data */
  முழு offset;        /* Offset of entry in pointer map */
  முழு rc;

  ப்ரதிஜ்ஞா( sqlite3_mutex_held(pBt->mutex) );

  iPtrmap = PTRMAP_PAGENO(pBt, சாவி);
  rc = sqlite3PagerGet(pBt->pPager, iPtrmap, &pDbPage, 0);
  எனில்( rc!=0 ){
    திரும்பு rc;
  }
  pPtrmap = (u8 *)sqlite3PagerGetData(pDbPage);

  offset = PTRMAP_PTROFFSET(iPtrmap, சாவி);
  எனில்( offset<0 ){
    sqlite3PagerUnref(pDbPage);
    திரும்பு SQLITE_CORRUPT_BKPT;
  }
  ப்ரதிஜ்ஞா( offset <= (முழு)pBt->usableSize-5 );
  ப்ரதிஜ்ஞா( pEType!=0 );
  *pEType = pPtrmap[offset];
  எனில்( pPgno ) *pPgno = get4byte(&pPtrmap[offset+1]);

  sqlite3PagerUnref(pDbPage);
  எனில்( *pEType<1 || *pEType>5 ) திரும்பு SQLITE_CORRUPT_PGNO(iPtrmap);
  திரும்பு SQLITE_OK;
}

#இல்லையெனில் /* if defined SQLITE_OMIT_AUTOVACUUM */
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
நிலையான SQLITE_NOINLINE வெறுமை btreeParseCellAdjustSizeForOverflow(
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
  முழு minLocal;  /* Minimum amount of payload held locally */
  முழு maxLocal;  /* Maximum amount of payload held locally */
  முழு surplus;   /* Overflow payload available for local storage */

  minLocal = pPage->minLocal;
  maxLocal = pPage->maxLocal;
  surplus = minLocal + (pInfo->nPayload - minLocal)%(pPage->pBt->usableSize-4);
  testcase( surplus==maxLocal );
  testcase( surplus==maxLocal+1 );
  எனில்( surplus <= maxLocal ){
    pInfo->nLocal = (u16)surplus;
  }இல்லையெனில்{
    pInfo->nLocal = (u16)minLocal;
  }
  pInfo->nSize = (u16)(&pInfo->pPayload[pInfo->nLocal] - pCell) + 4;
}

/*
** Given a record with nPayload bytes of payload stored within btree
** page pPage, return the number of bytes of payload stored locally.
*/
நிலையான முழு btreePayloadToLocal(MemPage *pPage, i64 nPayload){
  முழு maxLocal;  /* Maximum amount of payload held locally */
  maxLocal = pPage->maxLocal;
  ப்ரதிஜ்ஞா( nPayload>=0 );
  எனில்( nPayload<=maxLocal ){
    திரும்பு (முழு)nPayload;
  }இல்லையெனில்{
    முழு minLocal;  /* Minimum amount of payload held locally */
    முழு surplus;   /* Overflow payload available for local storage */
    minLocal = pPage->minLocal;
    surplus = (முழு)(minLocal +(nPayload - minLocal)%(pPage->pBt->usableSize-4));
    திரும்பு (surplus <= maxLocal) ? surplus : minLocal;
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
நிலையான வெறுமை btreeParseCellPtrNoPayload(
  MemPage *pPage,         /* Page containing the cell */
  u8 *pCell,              /* Pointer to the cell text. */
  CellInfo *pInfo         /* Fill in this structure */
){
  ப்ரதிஜ்ஞா( sqlite3_mutex_held(pPage->pBt->mutex) );
  ப்ரதிஜ்ஞா( pPage->leaf==0 );
  ப்ரதிஜ்ஞா( pPage->childPtrSize==4 );
#ifndef SQLITE_DEBUG
  UNUSED_PARAMETER(pPage);
#endif
  pInfo->nSize = 4 + sqlite3GetVarint(&pCell[4], (u64*)&pInfo->n_சாவி);
  pInfo->nPayload = 0;
  pInfo->nLocal = 0;
  pInfo->pPayload = 0;
  திரும்பு;
}
நிலையான வெறுமை btreeParseCellPtr(
  MemPage *pPage,         /* Page containing the cell */
  u8 *pCell,              /* Pointer to the cell text. */
  CellInfo *pInfo         /* Fill in this structure */
){
  u8 *pIter;              /* For scanning through pCell */
  u64 nPayload;           /* Number of bytes of cell payload */
  u64 i_சாவி;               /* Extracted Key value */

  ப்ரதிஜ்ஞா( sqlite3_mutex_held(pPage->pBt->mutex) );
  ப்ரதிஜ்ஞா( pPage->leaf==0 || pPage->leaf==1 );
  ப்ரதிஜ்ஞா( pPage->int_சாவி_Leaf );
  ப்ரதிஜ்ஞா( pPage->childPtrSize==0 );
  pIter = pCell;

  /* The next block of code is equivalent to:
  **
  **     pIter += getVarint32(pIter, nPayload);
  **
  ** The code is inlined to avoid a function call.
  */
  nPayload = *pIter;
  எனில்( nPayload>=0x80 ){
    u8 *pEnd = &pIter[8];
    nPayload &= 0x7f;
    செய்{
      nPayload = (nPayload<<7) | (*++pIter & 0x7f);
    }போது( (*pIter)>=0x80 && pIter<pEnd );
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
  i_சாவி = *pIter;
  எனில்( i_சாவி>=0x80 ){
    u8 x;
    i_சாவி = (i_சாவி<<7) ^ (x = *++pIter);
    எனில்( x>=0x80 ){
      i_சாவி = (i_சாவி<<7) ^ (x = *++pIter);
      எனில்( x>=0x80 ){
        i_சாவி = (i_சாவி<<7) ^ 0x10204000 ^ (x = *++pIter);
        எனில்( x>=0x80 ){
          i_சாவி = (i_சாவி<<7) ^ 0x4000 ^ (x = *++pIter);
          எனில்( x>=0x80 ){
            i_சாவி = (i_சாவி<<7) ^ 0x4000 ^ (x = *++pIter);
            எனில்( x>=0x80 ){
              i_சாவி = (i_சாவி<<7) ^ 0x4000 ^ (x = *++pIter);
              எனில்( x>=0x80 ){
                i_சாவி = (i_சாவி<<7) ^ 0x4000 ^ (x = *++pIter);
                எனில்( x>=0x80 ){
                  i_சாவி = (i_சாவி<<8) ^ 0x8000 ^ (*++pIter);
                }
              }
            }
          }
        }
      }இல்லையெனில்{
        i_சாவி ^= 0x204000;
      }
    }இல்லையெனில்{
      i_சாவி ^= 0x4000;
    }
  }
  pIter++;

  pInfo->n_சாவி = *(i64*)&i_சாவி;
  pInfo->nPayload = (u32)nPayload;
  pInfo->pPayload = pIter;
  testcase( nPayload==pPage->maxLocal );
  testcase( nPayload==(u32)pPage->maxLocal+1 );
  ப்ரதிஜ்ஞா( pPage->maxLocal <= BT_MAX_LOCAL );
  எனில்( nPayload<=pPage->maxLocal ){
    /* This is the (easy) common case where the entire payload fits
    ** on the local page.  No overflow is required.
    */
    pInfo->nSize = (u16)nPayload + (u16)(pIter - pCell);
    எனில்( pInfo->nSize<4 ) pInfo->nSize = 4;
    pInfo->nLocal = (u16)nPayload;
  }இல்லையெனில்{
    btreeParseCellAdjustSizeForOverflow(pPage, pCell, pInfo);
  }
}
நிலையான வெறுமை btreeParseCellPtrIndex(
  MemPage *pPage,         /* Page containing the cell */
  u8 *pCell,              /* Pointer to the cell text. */
  CellInfo *pInfo         /* Fill in this structure */
){
  u8 *pIter;              /* For scanning through pCell */
  u32 nPayload;           /* Number of bytes of cell payload */

  ப்ரதிஜ்ஞா( sqlite3_mutex_held(pPage->pBt->mutex) );
  ப்ரதிஜ்ஞா( pPage->leaf==0 || pPage->leaf==1 );
  ப்ரதிஜ்ஞா( pPage->int_சாவி_Leaf==0 );
  pIter = pCell + pPage->childPtrSize;
  nPayload = *pIter;
  எனில்( nPayload>=0x80 ){
    u8 *pEnd = &pIter[8];
    nPayload &= 0x7f;
    செய்{
      nPayload = (nPayload<<7) | (*++pIter & 0x7f);
    }போது( *(pIter)>=0x80 && pIter<pEnd );
  }
  pIter++;
  pInfo->n_சாவி = nPayload;
  pInfo->nPayload = nPayload;
  pInfo->pPayload = pIter;
  testcase( nPayload==pPage->maxLocal );
  testcase( nPayload==(u32)pPage->maxLocal+1 );
  ப்ரதிஜ்ஞா( nPayload>=0 );
  ப்ரதிஜ்ஞா( pPage->maxLocal <= BT_MAX_LOCAL );
  எனில்( nPayload<=pPage->maxLocal ){
    /* This is the (easy) common case where the entire payload fits
    ** on the local page.  No overflow is required.
    */
    pInfo->nSize = (u16)nPayload + (u16)(pIter - pCell);
    எனில்( pInfo->nSize<4 ) pInfo->nSize = 4;
    pInfo->nLocal = (u16)nPayload;
  }இல்லையெனில்{
    btreeParseCellAdjustSizeForOverflow(pPage, pCell, pInfo);
  }
}
நிலையான வெறுமை btreeParseCell(
  MemPage *pPage,         /* Page containing the cell */
  முழு iCell,              /* The cell index.  First cell is 0 */
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
நிலையான u16 cellSizePtr(MemPage *pPage, u8 *pCell){
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

  ப்ரதிஜ்ஞா( pPage->childPtrSize==4 );
  nSize = *pIter;
  எனில்( nSize>=0x80 ){
    pEnd = &pIter[8];
    nSize &= 0x7f;
    செய்{
      nSize = (nSize<<7) | (*++pIter & 0x7f);
    }போது( *(pIter)>=0x80 && pIter<pEnd );
  }
  pIter++;
  testcase( nSize==pPage->maxLocal );
  testcase( nSize==(u32)pPage->maxLocal+1 );
  எனில்( nSize<=pPage->maxLocal ){
    nSize += (u32)(pIter - pCell);
    ப்ரதிஜ்ஞா( nSize>4 );
  }இல்லையெனில்{
    முழு minLocal = pPage->minLocal;
    nSize = minLocal + (nSize - minLocal) % (pPage->pBt->usableSize - 4);
    testcase( nSize==pPage->maxLocal );
    testcase( nSize==(u32)pPage->maxLocal+1 );
    எனில்( nSize>pPage->maxLocal ){
      nSize = minLocal;
    }
    nSize += 4 + (u16)(pIter - pCell);
  }
  ப்ரதிஜ்ஞா( nSize==debuginfo.nSize || CORRUPT_DB );
  திரும்பு (u16)nSize;
}
நிலையான u16 cellSizePtrIdxLeaf(MemPage *pPage, u8 *pCell){
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

  ப்ரதிஜ்ஞா( pPage->childPtrSize==0 );
  nSize = *pIter;
  எனில்( nSize>=0x80 ){
    pEnd = &pIter[8];
    nSize &= 0x7f;
    செய்{
      nSize = (nSize<<7) | (*++pIter & 0x7f);
    }போது( *(pIter)>=0x80 && pIter<pEnd );
  }
  pIter++;
  testcase( nSize==pPage->maxLocal );
  testcase( nSize==(u32)pPage->maxLocal+1 );
  எனில்( nSize<=pPage->maxLocal ){
    nSize += (u32)(pIter - pCell);
    எனில்( nSize<4 ) nSize = 4;
  }இல்லையெனில்{
    முழு minLocal = pPage->minLocal;
    nSize = minLocal + (nSize - minLocal) % (pPage->pBt->usableSize - 4);
    testcase( nSize==pPage->maxLocal );
    testcase( nSize==(u32)pPage->maxLocal+1 );
    எனில்( nSize>pPage->maxLocal ){
      nSize = minLocal;
    }
    nSize += 4 + (u16)(pIter - pCell);
  }
  ப்ரதிஜ்ஞா( nSize==debuginfo.nSize || CORRUPT_DB );
  திரும்பு (u16)nSize;
}
நிலையான u16 cellSizePtrNoPayload(MemPage *pPage, u8 *pCell){
  u8 *pIter = pCell + 4; /* For looping over bytes of pCell */
  u8 *pEnd;              /* End mark for a varint */

#ifdef SQLITE_DEBUG
  /* The value returned by this function should always be the same as
  ** the (CellInfo.nSize) value found by doing a full parse of the
  ** cell. If SQLITE_DEBUG is defined, an assert() at the bottom of
  ** this function verifies that this invariant is not violated. */
  CellInfo debuginfo;
  pPage->xParseCell(pPage, pCell, &debuginfo);
#இல்லையெனில்
  UNUSED_PARAMETER(pPage);
#endif

  ப்ரதிஜ்ஞா( pPage->childPtrSize==4 );
  pEnd = pIter + 9;
  போது( (*pIter++)&0x80 && pIter<pEnd );
  ப்ரதிஜ்ஞா( debuginfo.nSize==(u16)(pIter - pCell) || CORRUPT_DB );
  திரும்பு (u16)(pIter - pCell);
}
நிலையான u16 cellSizePtrTableLeaf(MemPage *pPage, u8 *pCell){
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
  எனில்( nSize>=0x80 ){
    pEnd = &pIter[8];
    nSize &= 0x7f;
    செய்{
      nSize = (nSize<<7) | (*++pIter & 0x7f);
    }போது( *(pIter)>=0x80 && pIter<pEnd );
  }
  pIter++;
  /* pIter now points at the 64-bit integer key value, a variable length
  ** integer. The following block moves pIter to point at the first byte
  ** past the end of the key value. */
  எனில்( (*pIter++)&0x80
   && (*pIter++)&0x80
   && (*pIter++)&0x80
   && (*pIter++)&0x80
   && (*pIter++)&0x80
   && (*pIter++)&0x80
   && (*pIter++)&0x80
   && (*pIter++)&0x80 ){ pIter++; }
  testcase( nSize==pPage->maxLocal );
  testcase( nSize==(u32)pPage->maxLocal+1 );
  எனில்( nSize<=pPage->maxLocal ){
    nSize += (u32)(pIter - pCell);
    எனில்( nSize<4 ) nSize = 4;
  }இல்லையெனில்{
    முழு minLocal = pPage->minLocal;
    nSize = minLocal + (nSize - minLocal) % (pPage->pBt->usableSize - 4);
    testcase( nSize==pPage->maxLocal );
    testcase( nSize==(u32)pPage->maxLocal+1 );
    எனில்( nSize>pPage->maxLocal ){
      nSize = minLocal;
    }
    nSize += 4 + (u16)(pIter - pCell);
  }
  ப்ரதிஜ்ஞா( nSize==debuginfo.nSize || CORRUPT_DB );
  திரும்பு (u16)nSize;
}


#ifdef SQLITE_DEBUG
/* This variation on cellSizePtr() is used inside of assert() statements
** only. */
நிலையான u16 cellSize(MemPage *pPage, முழு iCell){
  திரும்பு pPage->xCellSize(pPage, findCell(pPage, iCell));
}
#endif

#ifndef SQLITE_OMIT_AUTOVACUUM
/*
** The cell pCell is currently part of page pSrc but will ultimately be part
** of pPage.  (pSrc and pPage are often the same.)  If pCell contains a
** pointer to an overflow page, insert an entry into the pointer-map for
** the overflow page that will be valid after pCell has been moved to pPage.
*/
நிலையான வெறுமை ptrmapPutOvflPtr(MemPage *pPage, MemPage *pSrc, u8 *pCell,முழு *pRC){
  CellInfo info;
  எனில்( *pRC ) திரும்பு;
  ப்ரதிஜ்ஞா( pCell!=0 );
  pPage->xParseCell(pPage, pCell, &info);
  எனில்( info.nLocal<info.nPayload ){
    Pgno ovfl;
    எனில்( SQLITE_OVERFLOW(pSrc->aDataEnd, pCell, pCell+info.nLocal) ){
      testcase( pSrc!=pPage );
      *pRC = SQLITE_CORRUPT_BKPT;
      திரும்பு;
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
நிலையான முழு defragmentPage(MemPage *pPage, முழு nMaxFrag){
  முழு i;                     /* Loop counter */
  முழு pc;                    /* Address of the i-th cell */
  முழு hdr;                   /* Offset to the page header */
  முழு size;                  /* Size of a cell */
  முழு usableSize;            /* Number of usable bytes on a page */
  முழு cellOffset;            /* Offset to the cell pointer array */
  முழு cbrk;                  /* Offset to the cell content area */
  முழு nCell;                 /* Number of cells on the page */
  unsigned எழுத்து *data;       /* The page data */
  unsigned எழுத்து *temp;       /* Temp area for cell content */
  unsigned எழுத்து *src;        /* Source of content */
  முழு iCellFirst;            /* First allowable cell index */
  முழு iCellLast;             /* Last possible cell index */
  முழு iCellStart;            /* First cell offset in input */

  ப்ரதிஜ்ஞா( sqlite3PagerIswriteable(pPage->pDbPage) );
  ப்ரதிஜ்ஞா( pPage->pBt!=0 );
  ப்ரதிஜ்ஞா( pPage->pBt->usableSize <= SQLITE_MAX_PAGE_SIZE );
  ப்ரதிஜ்ஞா( pPage->nOverflow==0 );
  ப்ரதிஜ்ஞா( sqlite3_mutex_held(pPage->pBt->mutex) );
  data = pPage->aData;
  hdr = pPage->hdrOffset;
  cellOffset = pPage->cellOffset;
  nCell = pPage->nCell;
  ப்ரதிஜ்ஞா( nCell==get2byte(&data[hdr+3]) || CORRUPT_DB );
  iCellFirst = cellOffset + 2*nCell;
  usableSize = pPage->pBt->usableSize;

  /* This block handles pages with two or fewer free blocks and nMaxFrag
  ** or fewer fragmented bytes. In this case it is faster to move the
  ** two (or one) blocks of cells using memmove() and add the required
  ** offsets to each pointer in the cell-pointer array than it is to
  ** reconstruct the entire page.  */
  எனில்( (முழு)data[hdr+7]<=nMaxFrag ){
    முழு iFree = get2byte(&data[hdr+1]);
    எனில்( iFree>usableSize-4 ) திரும்பு SQLITE_CORRUPT_PAGE(pPage);
    எனில்( iFree ){
      முழு iFree2 = get2byte(&data[iFree]);
      எனில்( iFree2>usableSize-4 ) திரும்பு SQLITE_CORRUPT_PAGE(pPage);
      எனில்( 0==iFree2 || (data[iFree2]==0 && data[iFree2+1]==0) ){
        u8 *pEnd = &data[cellOffset + nCell*2];
        u8 *pAddr;
        முழு sz2 = 0;
        முழு sz = get2byte(&data[iFree+2]);
        முழு உச்சி = get2byte(&data[hdr+5]);
        எனில்( உச்சி>=iFree ){
          திரும்பு SQLITE_CORRUPT_PAGE(pPage);
        }
        எனில்( iFree2 ){
          எனில்( iFree+sz>iFree2 ) திரும்பு SQLITE_CORRUPT_PAGE(pPage);
          sz2 = get2byte(&data[iFree2+2]);
          எனில்( iFree2+sz2 > usableSize ) திரும்பு SQLITE_CORRUPT_PAGE(pPage);
          memmove(&data[iFree+sz+sz2], &data[iFree+sz], iFree2-(iFree+sz));
          sz += sz2;
        }இல்லையெனில் எனில்( iFree+sz>usableSize ){
          திரும்பு SQLITE_CORRUPT_PAGE(pPage);
        }

        cbrk = உச்சி+sz;
        ப்ரதிஜ்ஞா( cbrk+(iFree-உச்சி) <= usableSize );
        memmove(&data[cbrk], &data[உச்சி], iFree-உச்சி);
        க்கு(pAddr=&data[cellOffset]; pAddr<pEnd; pAddr+=2){
          pc = get2byte(pAddr);
          எனில்( pc<iFree ){ put2byte(pAddr, pc+sz); }
          இல்லையெனில் எனில்( pc<iFree2 ){ put2byte(pAddr, pc+sz2); }
        }
        செல் defragment_out;
      }
    }
  }

  cbrk = usableSize;
  iCellLast = usableSize - 4;
  iCellStart = get2byte(&data[hdr+5]);
  எனில்( nCell>0 ){
    temp = sqlite3PagerTempSpace(pPage->pBt->pPager);
    memcpy(temp, data, usableSize);
    src = temp;
    க்கு(i=0; i<nCell; i++){
      u8 *pAddr;     /* The i-th cell pointer */
      pAddr = &data[cellOffset + i*2];
      pc = get2byte(pAddr);
      testcase( pc==iCellFirst );
      testcase( pc==iCellLast );
      /* These conditions have already been verified in btreeInitPage()
      ** if PRAGMA cell_size_check=ON.
      */
      எனில்( pc>iCellLast ){
        திரும்பு SQLITE_CORRUPT_PAGE(pPage);
      }
      ப்ரதிஜ்ஞா( pc>=0 && pc<=iCellLast );
      size = pPage->xCellSize(pPage, &src[pc]);
      cbrk -= size;
      எனில்( cbrk<iCellStart || pc+size>usableSize ){
        திரும்பு SQLITE_CORRUPT_PAGE(pPage);
      }
      ப்ரதிஜ்ஞா( cbrk+size<=usableSize && cbrk>=iCellStart );
      testcase( cbrk+size==usableSize );
      testcase( pc+size==usableSize );
      put2byte(pAddr, cbrk);
      memcpy(&data[cbrk], &src[pc], size);
    }
  }
  data[hdr+7] = 0;

defragment_out:
  ப்ரதிஜ்ஞா( pPage->nFree>=0 );
  எனில்( data[hdr+7]+cbrk-iCellFirst!=pPage->nFree ){
    திரும்பு SQLITE_CORRUPT_PAGE(pPage);
  }
  ப்ரதிஜ்ஞா( cbrk>=iCellFirst );
  put2byte(&data[hdr+5], cbrk);
  data[hdr+1] = 0;
  data[hdr+2] = 0;
  memset(&data[iCellFirst], 0, cbrk-iCellFirst);
  ப்ரதிஜ்ஞா( sqlite3PagerIswriteable(pPage->pDbPage) );
  திரும்பு SQLITE_OK;
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
நிலையான u8 *pageFindSlot(MemPage *pPg, முழு nByte, முழு *pRc){
  மாறிலி முழு hdr = pPg->hdrOffset;            /* Offset to page header */
  u8 * மாறிலி aData = pPg->aData;             /* Page data */
  முழு iAddr = hdr + 1;                       /* Address of ptr to pc */
  u8 *pTmp = &aData[iAddr];                  /* Temporary ptr into aData[] */
  முழு pc = get2byte(pTmp);                   /* Address of a free slot */
  முழு x;                                     /* Excess size of the slot */
  முழு maxPC = pPg->pBt->usableSize - nByte;  /* Max address for a usable slot */
  முழு size;                                  /* Size of the free slot */

  ப்ரதிஜ்ஞா( pc>0 );
  போது( pc<=maxPC ){
    /* EVIDENCE-OF: R-22710-53328 The third and fourth bytes of each
    ** freeblock form a big-endian integer which is the size of the freeblock
    ** in bytes, including the 4-byte header. */
    pTmp = &aData[pc+2];
    size = get2byte(pTmp);
    எனில்( (x = size - nByte)>=0 ){
      testcase( x==4 );
      testcase( x==3 );
      எனில்( x<4 ){
        /* EVIDENCE-OF: R-11498-58022 In a well-formed b-tree page, the total
        ** number of bytes in fragments may not exceed 60. */
        எனில்( aData[hdr+7]>57 ) திரும்பு 0;

        /* Remove the slot from the free-list. Update the number of
        ** fragmented bytes within the page. */
        memcpy(&aData[iAddr], &aData[pc], 2);
        aData[hdr+7] += (u8)x;
        திரும்பு &aData[pc];
      }இல்லையெனில் எனில்( x+pc > maxPC ){
        /* This slot extends off the end of the usable part of the page */
        *pRc = SQLITE_CORRUPT_PAGE(pPg);
        திரும்பு 0;
      }இல்லையெனில்{
        /* The slot remains on the free-list. Reduce its size to account
        ** for the portion used by the new allocation. */
        put2byte(&aData[pc+2], x);
      }
      திரும்பு &aData[pc + x];
    }
    iAddr = pc;
    pTmp = &aData[pc];
    pc = get2byte(pTmp);
    எனில்( pc<=iAddr ){
      எனில்( pc ){
        /* The next slot in the chain comes before the current slot */
        *pRc = SQLITE_CORRUPT_PAGE(pPg);
      }
      திரும்பு 0;
    }
  }
  எனில்( pc>maxPC+nByte-4 ){
    /* The free slot chain extends off the end of the page */
    *pRc = SQLITE_CORRUPT_PAGE(pPg);
  }
  திரும்பு 0;
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
நிலையான SQLITE_INLINE முழு allocateSpace(MemPage *pPage, முழு nByte, முழு *pIdx){
  மாறிலி முழு hdr = pPage->hdrOffset;    /* Local cache of pPage->hdrOffset */
  u8 * மாறிலி data = pPage->aData;      /* Local cache of pPage->aData */
  முழு உச்சி;                             /* First byte of cell content area */
  முழு rc = SQLITE_OK;                  /* Integer return code */
  u8 *pTmp;                            /* Temp ptr into data[] */
  முழு gap;        /* First byte of gap between cell pointers and cell content */
 
  ப்ரதிஜ்ஞா( sqlite3PagerIswriteable(pPage->pDbPage) );
  ப்ரதிஜ்ஞா( pPage->pBt );
  ப்ரதிஜ்ஞா( sqlite3_mutex_held(pPage->pBt->mutex) );
  ப்ரதிஜ்ஞா( nByte>=0 );  /* Minimum cell size is 4 */
  ப்ரதிஜ்ஞா( pPage->nFree>=nByte );
  ப்ரதிஜ்ஞா( pPage->nOverflow==0 );
  ப்ரதிஜ்ஞா( nByte < (முழு)(pPage->pBt->usableSize-8) );

  ப்ரதிஜ்ஞா( pPage->cellOffset == hdr + 12 - 4*pPage->leaf );
  gap = pPage->cellOffset + 2*pPage->nCell;
  ப்ரதிஜ்ஞா( gap<=65536 );
  /* EVIDENCE-OF: R-29356-02391 If the database uses a 65536-byte page size
  ** and the reserved space is zero (the usual value for reserved space)
  ** then the cell content offset of an empty page wants to be 65536.
  ** However, that integer is too large to be stored in a 2-byte unsigned
  ** integer, so a value of 0 is used in its place. */
  pTmp = &data[hdr+5];
  உச்சி = get2byte(pTmp);
  எனில்( gap>உச்சி ){
    எனில்( உச்சி==0 && pPage->pBt->usableSize==65536 ){
      உச்சி = 65536;
    }இல்லையெனில்{
      திரும்பு SQLITE_CORRUPT_PAGE(pPage);
    }
  }இல்லையெனில் எனில்( உச்சி>(முழு)pPage->pBt->usableSize ){
    திரும்பு SQLITE_CORRUPT_PAGE(pPage);
  }

  /* If there is enough space between gap and top for one more cell pointer,
  ** and if the freelist is not empty, then search the
  ** freelist looking for a slot big enough to satisfy the request.
  */
  testcase( gap+2==உச்சி );
  testcase( gap+1==உச்சி );
  testcase( gap==உச்சி );
  எனில்( (data[hdr+2] || data[hdr+1]) && gap+2<=உச்சி ){
    u8 *pSpace = pageFindSlot(pPage, nByte, &rc);
    எனில்( pSpace ){
      முழு g2;
      ப்ரதிஜ்ஞா( pSpace+nByte<=data+pPage->pBt->usableSize );
      *pIdx = g2 = (முழு)(pSpace-data);
      எனில்( g2<=gap ){
        திரும்பு SQLITE_CORRUPT_PAGE(pPage);
      }இல்லையெனில்{
        திரும்பு SQLITE_OK;
      }
    }இல்லையெனில் எனில்( rc ){
      திரும்பு rc;
    }
  }

  /* The request could not be fulfilled using a freelist slot.  Check
  ** to see if defragmentation is necessary.
  */
  testcase( gap+2+nByte==உச்சி );
  எனில்( gap+2+nByte>உச்சி ){
    ப்ரதிஜ்ஞா( pPage->nCell>0 || CORRUPT_DB );
    ப்ரதிஜ்ஞா( pPage->nFree>=0 );
    rc = defragmentPage(pPage, MIN(4, pPage->nFree - (2+nByte)));
    எனில்( rc ) திரும்பு rc;
    உச்சி = get2byteNotZero(&data[hdr+5]);
    ப்ரதிஜ்ஞா( gap+2+nByte<=உச்சி );
  }


  /* Allocate memory from the gap in between the cell pointer array
  ** and the cell content area.  The btreeComputeFreeSpace() call has already
  ** validated the freelist.  Given that the freelist is valid, there
  ** is no way that the allocation can extend off the end of the page.
  ** The assert() below verifies the previous sentence.
  */
  உச்சி -= nByte;
  put2byte(&data[hdr+5], உச்சி);
  ப்ரதிஜ்ஞா( உச்சி+nByte <= (முழு)pPage->pBt->usableSize );
  *pIdx = உச்சி;
  திரும்பு SQLITE_OK;
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
நிலையான முழு freeSpace(MemPage *pPage, முழு iStart, முழு iSize){
  முழு iPtr;                             /* Address of ptr to next freeblock */
  முழு iFreeBlk;                         /* Address of the next freeblock */
  u8 hdr;                               /* Page header size.  0 or 100 */
  முழு nFrag = 0;                        /* Reduction in fragmentation */
  முழு iOrigSize = iSize;                /* Original value of iSize */
  முழு x;                                /* Offset to cell content area */
  முழு iEnd = iStart + iSize;            /* First byte past the iStart buffer */
  unsigned எழுத்து *data = pPage->aData;   /* Page content */
  u8 *pTmp;                             /* Temporary ptr into data[] */

  ப்ரதிஜ்ஞா( pPage->pBt!=0 );
  ப்ரதிஜ்ஞா( sqlite3PagerIswriteable(pPage->pDbPage) );
  ப்ரதிஜ்ஞா( CORRUPT_DB || iStart>=pPage->hdrOffset+6+pPage->childPtrSize );
  ப்ரதிஜ்ஞா( CORRUPT_DB || iEnd <= (முழு)pPage->pBt->usableSize );
  ப்ரதிஜ்ஞா( sqlite3_mutex_held(pPage->pBt->mutex) );
  ப்ரதிஜ்ஞா( iSize>=4 );   /* Minimum cell size is 4 */
  ப்ரதிஜ்ஞா( CORRUPT_DB || iStart<=(முழு)pPage->pBt->usableSize-4 );

  /* The list of freeblocks must be in ascending order.  Find the
  ** spot on the list where iStart should be inserted.
  */
  hdr = pPage->hdrOffset;
  iPtr = hdr + 1;
  எனில்( data[iPtr+1]==0 && data[iPtr]==0 ){
    iFreeBlk = 0;  /* Shortcut for the case when the freelist is empty */
  }இல்லையெனில்{
    போது( (iFreeBlk = get2byte(&data[iPtr]))<iStart ){
      எனில்( iFreeBlk<=iPtr ){
        எனில்( iFreeBlk==0 ) நிறுத்து; /* TH3: corrupt082.100 */
        திரும்பு SQLITE_CORRUPT_PAGE(pPage);
      }
      iPtr = iFreeBlk;
    }
    எனில்( iFreeBlk>(முழு)pPage->pBt->usableSize-4 ){ /* TH3: corrupt081.100 */
      திரும்பு SQLITE_CORRUPT_PAGE(pPage);
    }
    ப்ரதிஜ்ஞா( iFreeBlk>iPtr || iFreeBlk==0 || CORRUPT_DB );
 
    /* At this point:
    **    iFreeBlk:   First freeblock after iStart, or zero if none
    **    iPtr:       The address of a pointer to iFreeBlk
    **
    ** Check to see if iFreeBlk should be coalesced onto the end of iStart.
    */
    எனில்( iFreeBlk && iEnd+3>=iFreeBlk ){
      nFrag = iFreeBlk - iEnd;
      எனில்( iEnd>iFreeBlk ) திரும்பு SQLITE_CORRUPT_PAGE(pPage);
      iEnd = iFreeBlk + get2byte(&data[iFreeBlk+2]);
      எனில்( iEnd > (முழு)pPage->pBt->usableSize ){
        திரும்பு SQLITE_CORRUPT_PAGE(pPage);
      }
      iSize = iEnd - iStart;
      iFreeBlk = get2byte(&data[iFreeBlk]);
    }
 
    /* If iPtr is another freeblock (that is, if iPtr is not the freelist
    ** pointer in the page header) then check to see if iStart should be
    ** coalesced onto the end of iPtr.
    */
    எனில்( iPtr>hdr+1 ){
      முழு iPtrEnd = iPtr + get2byte(&data[iPtr+2]);
      எனில்( iPtrEnd+3>=iStart ){
        எனில்( iPtrEnd>iStart ) திரும்பு SQLITE_CORRUPT_PAGE(pPage);
        nFrag += iStart - iPtrEnd;
        iSize = iEnd - iPtr;
        iStart = iPtr;
      }
    }
    எனில்( nFrag>data[hdr+7] ) திரும்பு SQLITE_CORRUPT_PAGE(pPage);
    data[hdr+7] -= (u8)nFrag;
  }
  pTmp = &data[hdr+5];
  x = get2byte(pTmp);
  எனில்( pPage->pBt->btsFlags & BTS_FAST_SECURE ){
    /* Overwrite deleted information with zeros when the secure_delete
    ** option is enabled */
    memset(&data[iStart], 0, iSize);
  }
  எனில்( iStart<=x ){
    /* The new freeblock is at the beginning of the cell content area,
    ** so just extend the cell content area rather than create another
    ** freelist entry */
    எனில்( iStart<x ) திரும்பு SQLITE_CORRUPT_PAGE(pPage);
    எனில்( iPtr!=hdr+1 ) திரும்பு SQLITE_CORRUPT_PAGE(pPage);
    put2byte(&data[hdr+1], iFreeBlk);
    put2byte(&data[hdr+5], iEnd);
  }இல்லையெனில்{
    /* Insert the new freeblock into the freelist */
    put2byte(&data[iPtr], iStart);
    put2byte(&data[iStart], iFreeBlk);
    ப்ரதிஜ்ஞா( iSize>=0 && iSize<=0xffff );
    put2byte(&data[iStart+2], (u16)iSize);
  }
  pPage->nFree += iOrigSize;
  திரும்பு SQLITE_OK;
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
நிலையான முழு decodeFlags(MemPage *pPage, முழு flagByte){
  BtShared *pBt;     /* A copy of pPage->pBt */

  ப்ரதிஜ்ஞா( pPage->hdrOffset==(pPage->pgno==1 ? 100 : 0) );
  ப்ரதிஜ்ஞா( sqlite3_mutex_held(pPage->pBt->mutex) );
  pBt = pPage->pBt;
  pPage->max1bytePayload = pBt->max1bytePayload;
  எனில்( flagByte>=(PTF_ZERODATA | PTF_LEAF) ){
    pPage->childPtrSize = 0;
    pPage->leaf = 1;
    எனில்( flagByte==(PTF_LEAFDATA | PTF_INTKEY | PTF_LEAF) ){
      pPage->int_சாவி_Leaf = 1;
      pPage->xCellSize = cellSizePtrTableLeaf;
      pPage->xParseCell = btreeParseCellPtr;
      pPage->int_சாவி = 1;
      pPage->maxLocal = pBt->maxLeaf;
      pPage->minLocal = pBt->minLeaf;
    }இல்லையெனில் எனில்( flagByte==(PTF_ZERODATA | PTF_LEAF) ){
      pPage->int_சாவி = 0;
      pPage->int_சாவி_Leaf = 0;
      pPage->xCellSize = cellSizePtrIdxLeaf;
      pPage->xParseCell = btreeParseCellPtrIndex;
      pPage->maxLocal = pBt->maxLocal;
      pPage->minLocal = pBt->minLocal;
    }இல்லையெனில்{
      pPage->int_சாவி = 0;
      pPage->int_சாவி_Leaf = 0;
      pPage->xCellSize = cellSizePtrIdxLeaf;
      pPage->xParseCell = btreeParseCellPtrIndex;
      திரும்பு SQLITE_CORRUPT_PAGE(pPage);
    }
  }இல்லையெனில்{
    pPage->childPtrSize = 4;
    pPage->leaf = 0;
    எனில்( flagByte==(PTF_ZERODATA) ){
      pPage->int_சாவி = 0;
      pPage->int_சாவி_Leaf = 0;
      pPage->xCellSize = cellSizePtr;
      pPage->xParseCell = btreeParseCellPtrIndex;
      pPage->maxLocal = pBt->maxLocal;
      pPage->minLocal = pBt->minLocal;
    }இல்லையெனில் எனில்( flagByte==(PTF_LEAFDATA | PTF_INTKEY) ){
      pPage->int_சாவி_Leaf = 0;
      pPage->xCellSize = cellSizePtrNoPayload;
      pPage->xParseCell = btreeParseCellPtrNoPayload;
      pPage->int_சாவி = 1;
      pPage->maxLocal = pBt->maxLeaf;
      pPage->minLocal = pBt->minLeaf;
    }இல்லையெனில்{
      pPage->int_சாவி = 0;
      pPage->int_சாவி_Leaf = 0;
      pPage->xCellSize = cellSizePtr;
      pPage->xParseCell = btreeParseCellPtrIndex;
      திரும்பு SQLITE_CORRUPT_PAGE(pPage);
    }
  }
  திரும்பு SQLITE_OK;
}

/*
** Compute the amount of freespace on the page.  In other words, fill
** in the pPage->nFree field.
*/
நிலையான முழு btreeComputeFreeSpace(MemPage *pPage){
  முழு pc;            /* Address of a freeblock within pPage->aData[] */
  u8 hdr;            /* Offset to beginning of page header */
  u8 *data;          /* Equal to pPage->aData */
  முழு usableSize;    /* Amount of usable space on each page */
  முழு nFree;         /* Number of unused bytes on the page */
  முழு உச்சி;           /* First byte of the cell content area */
  முழு iCellFirst;    /* First allowable cell or freeblock offset */
  முழு iCellLast;     /* Last possible cell or freeblock offset */

  ப்ரதிஜ்ஞா( pPage->pBt!=0 );
  ப்ரதிஜ்ஞா( pPage->pBt->db!=0 );
  ப்ரதிஜ்ஞா( sqlite3_mutex_held(pPage->pBt->mutex) );
  ப்ரதிஜ்ஞா( pPage->pgno==sqlite3PagerPagenumber(pPage->pDbPage) );
  ப்ரதிஜ்ஞா( pPage == sqlite3PagerGetExtra(pPage->pDbPage) );
  ப்ரதிஜ்ஞா( pPage->aData == sqlite3PagerGetData(pPage->pDbPage) );
  ப்ரதிஜ்ஞா( pPage->isInit==1 );
  ப்ரதிஜ்ஞா( pPage->nFree<0 );

  usableSize = pPage->pBt->usableSize;
  hdr = pPage->hdrOffset;
  data = pPage->aData;
  /* EVIDENCE-OF: R-58015-48175 The two-byte integer at offset 5 designates
  ** the start of the cell content area. A zero value for this integer is
  ** interpreted as 65536. */
  உச்சி = get2byteNotZero(&data[hdr+5]);
  iCellFirst = hdr + 8 + pPage->childPtrSize + 2*pPage->nCell;
  iCellLast = usableSize - 4;

  /* Compute the total free space on the page
  ** EVIDENCE-OF: R-23588-34450 The two-byte integer at offset 1 gives the
  ** start of the first freeblock on the page, or is zero if there are no
  ** freeblocks. */
  pc = get2byte(&data[hdr+1]);
  nFree = data[hdr+7] + உச்சி;  /* Init nFree to non-freeblock free space */
  எனில்( pc>0 ){
    u32 next, size;
    எனில்( pc<உச்சி ){
      /* EVIDENCE-OF: R-55530-52930 In a well-formed b-tree page, there will
      ** always be at least one cell before the first freeblock.
      */
      திரும்பு SQLITE_CORRUPT_PAGE(pPage);
    }
    போது( 1 ){
      எனில்( pc>iCellLast ){
        /* Freeblock off the end of the page */
        திரும்பு SQLITE_CORRUPT_PAGE(pPage);
      }
      next = get2byte(&data[pc]);
      size = get2byte(&data[pc+2]);
      எனில்( size<4 && sqlite3FaultSim(422)==SQLITE_OK ){
        /* Minimum freeblock size is 4.  Enable fault-sim 422 to disable this
        ** check to reach interesting error states.  However, disabling this
        ** check can cause assertion faults due to min-heap overflow.  All
        ** fault-sims are for testing use only, but this one especially so. */
        திரும்பு SQLITE_CORRUPT_PAGE(pPage);
      }
      nFree = nFree + size;
      எனில்( next<pc+size+4 ) நிறுத்து;
      pc = next;
    }
    எனில்( next>0 ){
      /* Freeblock not in ascending order */
      திரும்பு SQLITE_CORRUPT_PAGE(pPage);
    }
    எனில்( pc+size>(unsigned முழு)usableSize ){
      /* Last freeblock extends past page end */
      திரும்பு SQLITE_CORRUPT_PAGE(pPage);
    }
  }

  /* At this point, nFree contains the sum of the offset to the start
  ** of the cell-content area plus the number of free bytes within
  ** the cell-content area. If this is greater than the usable-size
  ** of the page, then the page must be corrupted. This check also
  ** serves to verify that the offset to the start of the cell-content
  ** area, according to the page header, lies within the page.
  */
  எனில்( nFree>usableSize || nFree<iCellFirst ){
    திரும்பு SQLITE_CORRUPT_PAGE(pPage);
  }
  pPage->nFree = (u16)(nFree - iCellFirst);
  திரும்பு SQLITE_OK;
}

/*
** Do additional sanity check after btreeInitPage() if
** PRAGMA cell_size_check=ON
*/
நிலையான SQLITE_NOINLINE முழு btreeCellSizeCheck(MemPage *pPage){
  முழு iCellFirst;    /* First allowable cell or freeblock offset */
  முழு iCellLast;     /* Last possible cell or freeblock offset */
  முழு i;             /* Index into the cell pointer array */
  முழு sz;            /* Size of a cell */
  முழு pc;            /* Address of a freeblock within pPage->aData[] */
  u8 *data;          /* Equal to pPage->aData */
  முழு usableSize;    /* Maximum usable space on the page */
  முழு cellOffset;    /* Start of cell content area */

  iCellFirst = pPage->cellOffset + 2*pPage->nCell;
  usableSize = pPage->pBt->usableSize;
  iCellLast = usableSize - 4;
  data = pPage->aData;
  cellOffset = pPage->cellOffset;
  எனில்( !pPage->leaf ) iCellLast--;
  க்கு(i=0; i<pPage->nCell; i++){
    pc = get2byteAligned(&data[cellOffset+i*2]);
    testcase( pc==iCellFirst );
    testcase( pc==iCellLast );
    எனில்( pc<iCellFirst || pc>iCellLast ){
      திரும்பு SQLITE_CORRUPT_PAGE(pPage);
    }
    sz = pPage->xCellSize(pPage, &data[pc]);
    testcase( pc+sz==usableSize );
    எனில்( pc+sz>usableSize ){
      திரும்பு SQLITE_CORRUPT_PAGE(pPage);
    }
  }
  திரும்பு SQLITE_OK;
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
நிலையான முழு btreeInitPage(MemPage *pPage){
  u8 *data;          /* Equal to pPage->aData */
  BtShared *pBt;        /* The main btree structure */

  ப்ரதிஜ்ஞா( pPage->pBt!=0 );
  ப்ரதிஜ்ஞா( pPage->pBt->db!=0 );
  ப்ரதிஜ்ஞா( sqlite3_mutex_held(pPage->pBt->mutex) );
  ப்ரதிஜ்ஞா( pPage->pgno==sqlite3PagerPagenumber(pPage->pDbPage) );
  ப்ரதிஜ்ஞா( pPage == sqlite3PagerGetExtra(pPage->pDbPage) );
  ப்ரதிஜ்ஞா( pPage->aData == sqlite3PagerGetData(pPage->pDbPage) );
  ப்ரதிஜ்ஞா( pPage->isInit==0 );

  pBt = pPage->pBt;
  data = pPage->aData + pPage->hdrOffset;
  /* EVIDENCE-OF: R-28594-02890 The one-byte flag at offset 0 indicating
  ** the b-tree page type. */
  எனில்( decodeFlags(pPage, data[0]) ){
    திரும்பு SQLITE_CORRUPT_PAGE(pPage);
  }
  ப்ரதிஜ்ஞா( pBt->pageSize>=512 && pBt->pageSize<=65536 );
  pPage->maskPage = (u16)(pBt->pageSize - 1);
  pPage->nOverflow = 0;
  pPage->cellOffset = (u16)(pPage->hdrOffset + 8 + pPage->childPtrSize);
  pPage->aCellIdx = data + pPage->childPtrSize + 8;
  pPage->aDataEnd = pPage->aData + pBt->pageSize;
  pPage->aDataOfst = pPage->aData + pPage->childPtrSize;
  /* EVIDENCE-OF: R-37002-32774 The two-byte integer at offset 3 gives the
  ** number of cells on the page. */
  pPage->nCell = get2byte(&data[3]);
  எனில்( pPage->nCell>MX_CELL(pBt) ){
    /* To many cells for a single page.  The page must be corrupt */
    திரும்பு SQLITE_CORRUPT_PAGE(pPage);
  }
  testcase( pPage->nCell==MX_CELL(pBt) );
  /* EVIDENCE-OF: R-24089-57979 If a page contains no cells (which is only
  ** possible for a root page of a table that contains no rows) then the
  ** offset to the cell content area will equal the page size minus the
  ** bytes of reserved space. */
  ப்ரதிஜ்ஞா( pPage->nCell>0
       || get2byteNotZero(&data[5])==(முழு)pBt->usableSize
       || CORRUPT_DB );
  pPage->nFree = -1;  /* Indicate that this value is yet uncomputed */
  pPage->isInit = 1;
  எனில்( pBt->db->flags & SQLITE_CellSizeCk ){
    திரும்பு btreeCellSizeCheck(pPage);
  }
  திரும்பு SQLITE_OK;
}

/*
** Set up a raw page so that it looks like a database page holding
** no entries.
*/
நிலையான வெறுமை zeroPage(MemPage *pPage, முழு flags){
  unsigned எழுத்து *data = pPage->aData;
  BtShared *pBt = pPage->pBt;
  முழு hdr = pPage->hdrOffset;
  முழு first;

  ப்ரதிஜ்ஞா( sqlite3PagerPagenumber(pPage->pDbPage)==pPage->pgno || CORRUPT_DB );
  ப்ரதிஜ்ஞா( sqlite3PagerGetExtra(pPage->pDbPage) == (வெறுமை*)pPage );
  ப்ரதிஜ்ஞா( sqlite3PagerGetData(pPage->pDbPage) == data );
  ப்ரதிஜ்ஞா( sqlite3PagerIswriteable(pPage->pDbPage) );
  ப்ரதிஜ்ஞா( sqlite3_mutex_held(pBt->mutex) );
  எனில்( pBt->btsFlags & BTS_FAST_SECURE ){
    memset(&data[hdr], 0, pBt->usableSize - hdr);
  }
  data[hdr] = (எழுத்து)flags;
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
  ப்ரதிஜ்ஞா( pBt->pageSize>=512 && pBt->pageSize<=65536 );
  pPage->maskPage = (u16)(pBt->pageSize - 1);
  pPage->nCell = 0;
  pPage->isInit = 1;
}


/*
** Convert a DbPage obtained from the pager into a MemPage used by
** the btree layer.
*/
நிலையான MemPage *btreePageFromDbPage(DbPage *pDbPage, Pgno pgno, BtShared *pBt){
  MemPage *pPage = (MemPage*)sqlite3PagerGetExtra(pDbPage);
  எனில்( pgno!=pPage->pgno ){
    pPage->aData = sqlite3PagerGetData(pDbPage);
    pPage->pDbPage = pDbPage;
    pPage->pBt = pBt;
    pPage->pgno = pgno;
    pPage->hdrOffset = pgno==1 ? 100 : 0;
  }
  ப்ரதிஜ்ஞா( pPage->aData==sqlite3PagerGetData(pDbPage) );
  திரும்பு pPage;
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
நிலையான முழு btreeGetPage(
  BtShared *pBt,       /* The btree */
  Pgno pgno,           /* Number of the page to fetch */
  MemPage **ppPage,    /* Return the page in this parameter */
  முழு flags            /* PAGER_GET_NOCONTENT or PAGER_GET_READONLY */
){
  முழு rc;
  DbPage *pDbPage;

  ப்ரதிஜ்ஞா( flags==0 || flags==PAGER_GET_NOCONTENT || flags==PAGER_GET_READONLY );
  ப்ரதிஜ்ஞா( sqlite3_mutex_held(pBt->mutex) );
  rc = sqlite3PagerGet(pBt->pPager, pgno, (DbPage**)&pDbPage, flags);
  எனில்( rc ) திரும்பு rc;
  *ppPage = btreePageFromDbPage(pDbPage, pgno, pBt);
  திரும்பு SQLITE_OK;
}

/*
** Retrieve a page from the pager cache. If the requested page is not
** already in the pager cache return NULL. Initialize the MemPage.pBt and
** MemPage.aData elements if needed.
*/
நிலையான MemPage *btreePageLookup(BtShared *pBt, Pgno pgno){
  DbPage *pDbPage;
  ப்ரதிஜ்ஞா( sqlite3_mutex_held(pBt->mutex) );
  pDbPage = sqlite3PagerLookup(pBt->pPager, pgno);
  எனில்( pDbPage ){
    திரும்பு btreePageFromDbPage(pDbPage, pgno, pBt);
  }
  திரும்பு 0;
}

/*
** Return the size of the database file in pages. If there is any kind of
** error, return ((unsigned int)-1).
*/
நிலையான Pgno btreePagecount(BtShared *pBt){
  திரும்பு pBt->nPage;
}
Pgno sqlite3BtreeLastPage(Btree *p){
  ப்ரதிஜ்ஞா( sqlite3BtreeHoldsMutex(p) );
  திரும்பு btreePagecount(p->pBt);
}

/*
** Get a page from the pager and initialize it.
*/
நிலையான முழு getAndInitPage(
  BtShared *pBt,                  /* The database file */
  Pgno pgno,                      /* Number of the page to get */
  MemPage **ppPage,               /* Write the page pointer here */
  முழு bReadOnly                   /* True for a read-only page */
){
  முழு rc;
  DbPage *pDbPage;
  MemPage *pPage;
  ப்ரதிஜ்ஞா( sqlite3_mutex_held(pBt->mutex) );

  எனில்( pgno>btreePagecount(pBt) ){
    *ppPage = 0;
    திரும்பு SQLITE_CORRUPT_BKPT;
  }
  rc = sqlite3PagerGet(pBt->pPager, pgno, (DbPage**)&pDbPage, bReadOnly);
  எனில்( rc ){
    *ppPage = 0;
    திரும்பு rc;
  }
  pPage = (MemPage*)sqlite3PagerGetExtra(pDbPage);
  எனில்( pPage->isInit==0 ){
    btreePageFromDbPage(pDbPage, pgno, pBt);
    rc = btreeInitPage(pPage);
    எனில்( rc!=SQLITE_OK ){
      releasePage(pPage);
      *ppPage = 0;
      திரும்பு rc;
    }
  }
  ப்ரதிஜ்ஞா( pPage->pgno==pgno || CORRUPT_DB );
  ப்ரதிஜ்ஞா( pPage->aData==sqlite3PagerGetData(pDbPage) );
  *ppPage = pPage;
  திரும்பு SQLITE_OK;
}

/*
** Release a MemPage.  This should be called once for each prior
** call to btreeGetPage.
**
** Page1 is a special case and must be released using releasePageOne().
*/
நிலையான வெறுமை releasePageNotNull(MemPage *pPage){
  ப்ரதிஜ்ஞா( pPage->aData );
  ப்ரதிஜ்ஞா( pPage->pBt );
  ப்ரதிஜ்ஞா( pPage->pDbPage!=0 );
  ப்ரதிஜ்ஞா( sqlite3PagerGetExtra(pPage->pDbPage) == (வெறுமை*)pPage );
  ப்ரதிஜ்ஞா( sqlite3PagerGetData(pPage->pDbPage)==pPage->aData );
  ப்ரதிஜ்ஞா( sqlite3_mutex_held(pPage->pBt->mutex) );
  sqlite3PagerUnrefNotNull(pPage->pDbPage);
}
நிலையான வெறுமை releasePage(MemPage *pPage){
  எனில்( pPage ) releasePageNotNull(pPage);
}
நிலையான வெறுமை releasePageOne(MemPage *pPage){
  ப்ரதிஜ்ஞா( pPage!=0 );
  ப்ரதிஜ்ஞா( pPage->aData );
  ப்ரதிஜ்ஞா( pPage->pBt );
  ப்ரதிஜ்ஞா( pPage->pDbPage!=0 );
  ப்ரதிஜ்ஞா( sqlite3PagerGetExtra(pPage->pDbPage) == (வெறுமை*)pPage );
  ப்ரதிஜ்ஞா( sqlite3PagerGetData(pPage->pDbPage)==pPage->aData );
  ப்ரதிஜ்ஞா( sqlite3_mutex_held(pPage->pBt->mutex) );
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
நிலையான முழு btreeGetUnusedPage(
  BtShared *pBt,       /* The btree */
  Pgno pgno,           /* Number of the page to fetch */
  MemPage **ppPage,    /* Return the page in this parameter */
  முழு flags            /* PAGER_GET_NOCONTENT or PAGER_GET_READONLY */
){
  முழு rc = btreeGetPage(pBt, pgno, ppPage, flags);
  எனில்( rc==SQLITE_OK ){
    எனில்( sqlite3PagerPageRefcount((*ppPage)->pDbPage)>1 ){
      releasePage(*ppPage);
      *ppPage = 0;
      திரும்பு SQLITE_CORRUPT_BKPT;
    }
    (*ppPage)->isInit = 0;
  }இல்லையெனில்{
    *ppPage = 0;
  }
  திரும்பு rc;
}


/*
** During a rollback, when the pager reloads information into the cache
** so that the cache is restored to its original state at the start of
** the transaction, for each page restored this routine is called.
**
** This routine needs to reset the extra data section at the end of the
** page to agree with the restored data.
*/
நிலையான வெறுமை pageReinit(DbPage *pData){
  MemPage *pPage;
  pPage = (MemPage *)sqlite3PagerGetExtra(pData);
  ப்ரதிஜ்ஞா( sqlite3PagerPageRefcount(pData)>0 );
  எனில்( pPage->isInit ){
    ப்ரதிஜ்ஞா( sqlite3_mutex_held(pPage->pBt->mutex) );
    pPage->isInit = 0;
    எனில்( sqlite3PagerPageRefcount(pData)>1 ){
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
நிலையான முழு btreeInvokeBusyHandler(வெறுமை *pArg){
  BtShared *pBt = (BtShared*)pArg;
  ப்ரதிஜ்ஞா( pBt->db );
  ப்ரதிஜ்ஞா( sqlite3_mutex_held(pBt->db->mutex) );
  திரும்பு sqlite3InvokeBusyHandler(&pBt->db->busyHandler);
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
முழு sqlite3BtreeOpen(
  sqlite3_vfs *pVfs,      /* VFS to use for this b-tree */
  மாறிலி எழுத்து *zFilename,  /* Name of the file containing the BTree database */
  sqlite3 *db,            /* Associated database handle */
  Btree **ppBtree,        /* Pointer to new Btree object written here */
  முழு flags,              /* Options */
  முழு vfsFlags            /* Flags passed through to sqlite3_vfs.xOpen() */
){
  BtShared *pBt = 0;             /* Shared part of btree structure */
  Btree *p;                      /* Handle to return */
  sqlite3_mutex *mutexOpen = 0;  /* Prevents a race condition. Ticket #3537 */
  முழு rc = SQLITE_OK;            /* Result code from this function */
  u8 nReserve;                   /* Byte of unused space on each page */
  unsigned எழுத்து zDbHeader[100];  /* Database header content */

  /* True if opening an ephemeral, temporary database */
  மாறிலி முழு isTempDb = zFilename==0 || zFilename[0]==0;

  /* Set the variable isMemdb to true for an in-memory database, or
  ** false for a file-based database.
  */
#ifdef SQLITE_OMIT_MEMORYDB
  மாறிலி முழு isMemdb = 0;
#இல்லையெனில்
  மாறிலி முழு isMemdb = (zFilename && strcmp(zFilename, ":memory:")==0)
                       || (isTempDb && sqlite3TempInMemory(db))
                       || (vfsFlags & SQLITE_OPEN_MEMORY)!=0;
#endif

  ப்ரதிஜ்ஞா( db!=0 );
  ப்ரதிஜ்ஞா( pVfs!=0 );
  ப்ரதிஜ்ஞா( sqlite3_mutex_held(db->mutex) );
  ப்ரதிஜ்ஞா( (flags&0xff)==flags );   /* flags fit in 8 bits */

  /* Only a BTREE_SINGLE database can be BTREE_UNORDERED */
  ப்ரதிஜ்ஞா( (flags & BTREE_UNORDERED)==0 || (flags & BTREE_SINGLE)!=0 );

  /* A BTREE_SINGLE database is always a temporary and/or ephemeral */
  ப்ரதிஜ்ஞா( (flags & BTREE_SINGLE)==0 || isTempDb );

  எனில்( isMemdb ){
    flags |= BTREE_MEMORY;
  }
  எனில்( (vfsFlags & SQLITE_OPEN_MAIN_DB)!=0 && (isMemdb || isTempDb) ){
    vfsFlags = (vfsFlags & ~SQLITE_OPEN_MAIN_DB) | SQLITE_OPEN_TEMP_DB;
  }
  p = sqlite3MallocZero(அளவு(Btree));
  எனில்( !p ){
    திரும்பு SQLITE_NOMEM_BKPT;
  }
  p->inTrans = TRANS_NONE;
  p->db = db;
#ifndef SQLITE_OMIT_SHARED_CACHE
  p->lock.pBtree = p;
  p->lock.iTable = 1;
#endif

#எனில் !defined(SQLITE_OMIT_SHARED_CACHE) && !defined(SQLITE_OMIT_DISKIO)
  /*
  ** If this Btree is a candidate for shared cache, try to find an
  ** existing BtShared object that we can share with
  */
  எனில்( isTempDb==0 && (isMemdb==0 || (vfsFlags&SQLITE_OPEN_URI)!=0) ){
    எனில்( vfsFlags & SQLITE_OPEN_SHAREDCACHE ){
      முழு nFilename = sqlite3Strlen30(zFilename)+1;
      முழு nFullPathname = pVfs->mxPathname+1;
      எழுத்து *zFullPathname = sqlite3Malloc(MAX(nFullPathname,nFilename));
      MUTEX_LOGIC( sqlite3_mutex *mutexShared; )

      p->sharable = 1;
      எனில்( !zFullPathname ){
        sqlite3_free(p);
        திரும்பு SQLITE_NOMEM_BKPT;
      }
      எனில்( isMemdb ){
        memcpy(zFullPathname, zFilename, nFilename);
      }இல்லையெனில்{
        rc = sqlite3OsFullPathname(pVfs, zFilename,
                                   nFullPathname, zFullPathname);
        எனில்( rc ){
          எனில்( rc==SQLITE_OK_SYMLINK ){
            rc = SQLITE_OK;
          }இல்லையெனில்{
            sqlite3_free(zFullPathname);
            sqlite3_free(p);
            திரும்பு rc;
          }
        }
      }
#எனில் SQLITE_THREADSAFE
      mutexOpen = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_OPEN);
      sqlite3_mutex_enter(mutexOpen);
      mutexShared = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
      sqlite3_mutex_enter(mutexShared);
#endif
      க்கு(pBt=GLOBAL(BtShared*,sqlite3SharedCacheList); pBt; pBt=pBt->pNext){
        ப்ரதிஜ்ஞா( pBt->nRef>0 );
        எனில்( 0==strcmp(zFullPathname, sqlite3PagerFilename(pBt->pPager, 0))
                 && sqlite3PagerVfs(pBt->pPager)==pVfs ){
          முழு iDb;
          க்கு(iDb=db->nDb-1; iDb>=0; iDb--){
            Btree *pExisting = db->aDb[iDb].pBt;
            எனில்( pExisting && pExisting->pBt==pBt ){
              sqlite3_mutex_leave(mutexShared);
              sqlite3_mutex_leave(mutexOpen);
              sqlite3_free(zFullPathname);
              sqlite3_free(p);
              திரும்பு SQLITE_CONSTRAINT;
            }
          }
          p->pBt = pBt;
          pBt->nRef++;
          நிறுத்து;
        }
      }
      sqlite3_mutex_leave(mutexShared);
      sqlite3_free(zFullPathname);
    }
#ifdef SQLITE_DEBUG
    இல்லையெனில்{
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
  எனில்( pBt==0 ){
    /*
    ** The following asserts make sure that structures used by the btree are
    ** the right size.  This is to guard against size changes that result
    ** when compiling on a different architecture.
    */
    ப்ரதிஜ்ஞா( அளவு(i64)==8 );
    ப்ரதிஜ்ஞா( அளவு(u64)==8 );
    ப்ரதிஜ்ஞா( அளவு(u32)==4 );
    ப்ரதிஜ்ஞா( அளவு(u16)==2 );
    ப்ரதிஜ்ஞா( அளவு(Pgno)==4 );

    /* Suppress false-positive compiler warning from PVS-Studio */
    memset(&zDbHeader[16], 0, 8);
 
    pBt = sqlite3MallocZero( அளவு(*pBt) );
    எனில்( pBt==0 ){
      rc = SQLITE_NOMEM_BKPT;
      செல் btree_open_out;
    }
    rc = sqlite3PagerOpen(pVfs, &pBt->pPager, zFilename,
                          அளவு(MemPage), flags, vfsFlags, pageReinit);
    எனில்( rc==SQLITE_OK ){
      sqlite3PagerSetMmapLimit(pBt->pPager, db->szMmap);
      rc = sqlite3PagerReadFileheader(pBt->pPager,அளவு(zDbHeader),zDbHeader);
    }
    எனில்( rc!=SQLITE_OK ){
      செல் btree_open_out;
    }
    pBt->openFlags = (u8)flags;
    pBt->db = db;
    sqlite3PagerSetBusyHandler(pBt->pPager, btreeInvokeBusyHandler, pBt);
    p->pBt = pBt;
 
    pBt->pCursor = 0;
    pBt->pPage1 = 0;
    எனில்( sqlite3PagerIsreadonly(pBt->pPager) ) pBt->btsFlags |= BTS_READ_ONLY;
#எனில் defined(SQLITE_SECURE_DELETE)
    pBt->btsFlags |= BTS_SECURE_DELETE;
#அல்லதெனில் defined(SQLITE_FAST_SECURE_DELETE)
    pBt->btsFlags |= BTS_OVERWRITE;
#endif
    /* EVIDENCE-OF: R-51873-39618 The page size for a database file is
    ** determined by the 2-byte integer located at an offset of 16 bytes from
    ** the beginning of the database file. */
    pBt->pageSize = (zDbHeader[16]<<8) | (zDbHeader[17]<<16);
    எனில்( pBt->pageSize<512 || pBt->pageSize>SQLITE_MAX_PAGE_SIZE
         || ((pBt->pageSize-1)&pBt->pageSize)!=0 ){
      pBt->pageSize = 0;
#ifndef SQLITE_OMIT_AUTOVACUUM
      /* If the magic name ":memory:" will create an in-memory database, then
      ** leave the autoVacuum mode at 0 (do not auto-vacuum), even if
      ** SQLITE_DEFAULT_AUTOVACUUM is true. On the other hand, if
      ** SQLITE_OMIT_MEMORYDB has been defined, then ":memory:" is just a
      ** regular file-name. In this case the auto-vacuum applies as per normal.
      */
      எனில்( zFilename && !isMemdb ){
        pBt->autoVacuum = (SQLITE_DEFAULT_AUTOVACUUM ? 1 : 0);
        pBt->incrVacuum = (SQLITE_DEFAULT_AUTOVACUUM==2 ? 1 : 0);
      }
#endif
      nReserve = 0;
    }இல்லையெனில்{
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
    எனில்( rc ) செல் btree_open_out;
    pBt->usableSize = pBt->pageSize - nReserve;
    ப்ரதிஜ்ஞா( (pBt->pageSize & 7)==0 );  /* 8-byte alignment of pageSize */
  
#எனில் !defined(SQLITE_OMIT_SHARED_CACHE) && !defined(SQLITE_OMIT_DISKIO)
    /* Add the new BtShared object to the linked list sharable BtShareds.
    */
    pBt->nRef = 1;
    எனில்( p->sharable ){
      MUTEX_LOGIC( sqlite3_mutex *mutexShared; )
      MUTEX_LOGIC( mutexShared = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);)
      எனில்( SQLITE_THREADSAFE && sqlite3GlobalConfig.bCoreMutex ){
        pBt->mutex = sqlite3MutexAlloc(SQLITE_MUTEX_FAST);
        எனில்( pBt->mutex==0 ){
          rc = SQLITE_NOMEM_BKPT;
          செல் btree_open_out;
        }
      }
      sqlite3_mutex_enter(mutexShared);
      pBt->pNext = GLOBAL(BtShared*,sqlite3SharedCacheList);
      GLOBAL(BtShared*,sqlite3SharedCacheList) = pBt;
      sqlite3_mutex_leave(mutexShared);
    }
#endif
  }

#எனில் !defined(SQLITE_OMIT_SHARED_CACHE) && !defined(SQLITE_OMIT_DISKIO)
  /* If the new Btree uses a sharable pBtShared, then link the new
  ** Btree into the list of all sharable Btrees for the same connection.
  ** The list is kept in ascending order by pBt address.
  */
  எனில்( p->sharable ){
    முழு i;
    Btree *pSib;
    க்கு(i=0; i<db->nDb; i++){
      எனில்( (pSib = db->aDb[i].pBt)!=0 && pSib->sharable ){
        போது( pSib->pPrev ){ pSib = pSib->pPrev; }
        எனில்( (uptr)p->pBt<(uptr)pSib->pBt ){
          p->pNext = pSib;
          p->pPrev = 0;
          pSib->pPrev = p;
        }இல்லையெனில்{
          போது( pSib->pNext && (uptr)pSib->pNext->pBt<(uptr)p->pBt ){
            pSib = pSib->pNext;
          }
          p->pNext = pSib->pNext;
          p->pPrev = pSib;
          எனில்( p->pNext ){
            p->pNext->pPrev = p;
          }
          pSib->pNext = p;
        }
        நிறுத்து;
      }
    }
  }
#endif
  *ppBtree = p;

btree_open_out:
  எனில்( rc!=SQLITE_OK ){
    எனில்( pBt && pBt->pPager ){
      sqlite3PagerClose(pBt->pPager, 0);
    }
    sqlite3_free(pBt);
    sqlite3_free(p);
    *ppBtree = 0;
  }இல்லையெனில்{
    sqlite3_file *pFile;

    /* If the B-Tree was successfully opened, set the pager-cache size to the
    ** default value. Except, when opening on an existing shared pager-cache,
    ** do not change the pager-cache size.
    */
    எனில்( sqlite3BtreeSchema(p, 0, 0)==0 ){
      sqlite3BtreeSetCacheSize(p, SQLITE_DEFAULT_CACHE_SIZE);
    }

    pFile = sqlite3PagerFile(pBt->pPager);
    எனில்( pFile->pMethods ){
      sqlite3OsFileControlHint(pFile, SQLITE_FCNTL_PDB, (வெறுமை*)&pBt->db);
    }
  }
  எனில்( mutexOpen ){
    ப்ரதிஜ்ஞா( sqlite3_mutex_held(mutexOpen) );
    sqlite3_mutex_leave(mutexOpen);
  }
  ப்ரதிஜ்ஞா( rc!=SQLITE_OK || sqlite3_Btree_Connection_மொத்தம்(*ppBtree)>0 );
  திரும்பு rc;
}

/*
** Decrement the BtShared.nRef counter.  When it reaches zero,
** remove the BtShared structure from the sharing list.  Return
** true if the BtShared.nRef counter reaches zero and return
** false if it is still positive.
*/
நிலையான முழு removeFromSharingList(BtShared *pBt){
#ifndef SQLITE_OMIT_SHARED_CACHE
  MUTEX_LOGIC( sqlite3_mutex *pMainMtx; )
  BtShared *pList;
  முழு removed = 0;

  ப்ரதிஜ்ஞா( sqlite3_mutex_notheld(pBt->mutex) );
  MUTEX_LOGIC( pMainMtx = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  sqlite3_mutex_enter(pMainMtx);
  pBt->nRef--;
  எனில்( pBt->nRef<=0 ){
    எனில்( GLOBAL(BtShared*,sqlite3SharedCacheList)==pBt ){
      GLOBAL(BtShared*,sqlite3SharedCacheList) = pBt->pNext;
    }இல்லையெனில்{
      pList = GLOBAL(BtShared*,sqlite3SharedCacheList);
      போது( ALWAYS(pList) && pList->pNext!=pBt ){
        pList=pList->pNext;
      }
      எனில்( ALWAYS(pList) ){
        pList->pNext = pBt->pNext;
      }
    }
    எனில்( SQLITE_THREADSAFE ){
      sqlite3_mutex_free(pBt->mutex);
    }
    removed = 1;
  }
  sqlite3_mutex_leave(pMainMtx);
  திரும்பு removed;
#இல்லையெனில்
  UNUSED_PARAMETER( pBt );
  திரும்பு 1;
#endif
}

/*
** Make sure pBt->pTmpSpace points to an allocation of
** MX_CELL_SIZE(pBt) bytes with a 4-byte prefix for a left-child
** pointer.
*/
நிலையான SQLITE_NOINLINE முழு allocateTempSpace(BtShared *pBt){
  ப்ரதிஜ்ஞா( pBt!=0 );
  ப்ரதிஜ்ஞா( pBt->pTmpSpace==0 );
  /* This routine is called only by btreeCursor() when allocating the
  ** first write cursor for the BtShared object */
  ப்ரதிஜ்ஞா( pBt->pCursor!=0 && (pBt->pCursor->curFlags & BTCF_WriteFlag)!=0 );
  pBt->pTmpSpace = sqlite3PageMalloc( pBt->pageSize );
  எனில்( pBt->pTmpSpace==0 ){
    BtCursor *pCur = pBt->pCursor;
    pBt->pCursor = pCur->pNext;  /* Unlink the cursor */
    memset(pCur, 0, அளவு(*pCur));
    திரும்பு SQLITE_NOMEM_BKPT;
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
  திரும்பு SQLITE_OK;
}

/*
** Free the pBt->pTmpSpace allocation
*/
நிலையான வெறுமை freeTempSpace(BtShared *pBt){
  எனில்( pBt->pTmpSpace ){
    pBt->pTmpSpace -= 4;
    sqlite3PageFree(pBt->pTmpSpace);
    pBt->pTmpSpace = 0;
  }
}

/*
** Close an open database and invalidate all cursors.
*/
முழு sqlite3BtreeClose(Btree *p){
  BtShared *pBt = p->pBt;

  /* Close all cursors opened via this handle.  */
  ப்ரதிஜ்ஞா( sqlite3_mutex_held(p->db->mutex) );
  sqlite3BtreeEnter(p);

  /* Verify that no other cursors have this Btree open */
#ifdef SQLITE_DEBUG
  {
    BtCursor *pCur = pBt->pCursor;
    போது( pCur ){
      BtCursor *pTmp = pCur;
      pCur = pCur->pNext;
      ப்ரதிஜ்ஞா( pTmp->pBtree!=p );

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
  ப்ரதிஜ்ஞா( p->wantToLock==0 && p->locked==0 );
  எனில்( !p->sharable || removeFromSharingList(pBt) ){
    /* The pBt is no longer on the sharing list, so we can access
    ** it without having to hold the mutex.
    **
    ** Clean out and delete the BtShared object.
    */
    ப்ரதிஜ்ஞா( !pBt->pCursor );
    sqlite3PagerClose(pBt->pPager, p->db);
    எனில்( pBt->xFreeSchema && pBt->pSchema ){
      pBt->xFreeSchema(pBt->pSchema);
    }
    sqlite3DbFree(0, pBt->pSchema);
    freeTempSpace(pBt);
    sqlite3_free(pBt);
  }

#ifndef SQLITE_OMIT_SHARED_CACHE
  ப்ரதிஜ்ஞா( p->wantToLock==0 );
  ப்ரதிஜ்ஞா( p->locked==0 );
  எனில்( p->pPrev ) p->pPrev->pNext = p->pNext;
  எனில்( p->pNext ) p->pNext->pPrev = p->pPrev;
#endif

  sqlite3_free(p);
  திரும்பு SQLITE_OK;
}

/*
** Change the "soft" limit on the number of pages in the cache.
** Unused and unmodified pages will be recycled when the number of
** pages in the cache exceeds this soft limit.  But the size of the
** cache is allowed to grow larger than this limit if it contains
** dirty pages or pages still in active use.
*/
முழு sqlite3BtreeSetCacheSize(Btree *p, முழு mxPage){
  BtShared *pBt = p->pBt;
  ப்ரதிஜ்ஞா( sqlite3_mutex_held(p->db->mutex) );
  sqlite3BtreeEnter(p);
  sqlite3PagerSetCachesize(pBt->pPager, mxPage);
  sqlite3BtreeLeave(p);
  திரும்பு SQLITE_OK;
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
முழு sqlite3BtreeSetSpillSize(Btree *p, முழு mxPage){
  BtShared *pBt = p->pBt;
  முழு res;
  ப்ரதிஜ்ஞா( sqlite3_mutex_held(p->db->mutex) );
  sqlite3BtreeEnter(p);
  res = sqlite3PagerSetSpillsize(pBt->pPager, mxPage);
  sqlite3BtreeLeave(p);
  திரும்பு res;
}

#எனில் SQLITE_MAX_MMAP_SIZE>0
/*
** Change the limit on the amount of the database file that may be
** memory mapped.
*/
முழு sqlite3BtreeSetMmapLimit(Btree *p, sqlite3_int64 szMmap){
  BtShared *pBt = p->pBt;
  ப்ரதிஜ்ஞா( sqlite3_mutex_held(p->db->mutex) );
  sqlite3BtreeEnter(p);
  sqlite3PagerSetMmapLimit(pBt->pPager, szMmap);
  sqlite3BtreeLeave(p);
  திரும்பு SQLITE_OK;
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
முழு sqlite3BtreeSetPagerFlags(
  Btree *p,              /* The btree to set the safety level on */
  unsigned pgFlags       /* Various PAGER_* flags */
){
  BtShared *pBt = p->pBt;
  ப்ரதிஜ்ஞா( sqlite3_mutex_held(p->db->mutex) );
  sqlite3BtreeEnter(p);
  sqlite3PagerSetFlags(pBt->pPager, pgFlags);
  sqlite3BtreeLeave(p);
  திரும்பு SQLITE_OK;
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
முழு sqlite3BtreeSetPageSize(Btree *p, முழு pageSize, முழு nReserve, முழு iFix){
  முழு rc = SQLITE_OK;
  முழு x;
  BtShared *pBt = p->pBt;
  ப்ரதிஜ்ஞா( nReserve>=0 && nReserve<=255 );
  sqlite3BtreeEnter(p);
  pBt->nReserveWanted = (u8)nReserve;
  x = pBt->pageSize - pBt->usableSize;
  எனில்( x==nReserve && (pageSize==0 || (u32)pageSize==pBt->pageSize) ){
    sqlite3BtreeLeave(p);
    திரும்பு SQLITE_OK;
  }
  எனில்( nReserve<x ) nReserve = x;
  எனில்( pBt->btsFlags & BTS_PAGESIZE_FIXED ){
    sqlite3BtreeLeave(p);
    திரும்பு SQLITE_READONLY;
  }
  ப்ரதிஜ்ஞா( nReserve>=0 && nReserve<=255 );
  எனில்( pageSize>=512 && pageSize<=SQLITE_MAX_PAGE_SIZE &&
        ((pageSize-1)&pageSize)==0 ){
    ப்ரதிஜ்ஞா( (pageSize & 7)==0 );
    ப்ரதிஜ்ஞா( !pBt->pCursor );
    எனில்( nReserve>32 && pageSize==512 ) pageSize = 1024;
    pBt->pageSize = (u32)pageSize;
    freeTempSpace(pBt);
  }
  rc = sqlite3PagerSetPagesize(pBt->pPager, &pBt->pageSize, nReserve);
  pBt->usableSize = pBt->pageSize - (u16)nReserve;
  எனில்( iFix ) pBt->btsFlags |= BTS_PAGESIZE_FIXED;
  sqlite3BtreeLeave(p);
  திரும்பு rc;
}

/*
** Return the currently defined page size
*/
முழு sqlite3BtreeGetPageSize(Btree *p){
  திரும்பு p->pBt->pageSize;
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
முழு sqlite3BtreeGetReserveNoMutex(Btree *p){
  முழு n;
  ப்ரதிஜ்ஞா( sqlite3_mutex_held(p->pBt->mutex) );
  n = p->pBt->pageSize - p->pBt->usableSize;
  திரும்பு n;
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
முழு sqlite3BtreeGetRequestedReserve(Btree *p){
  முழு n1, n2;
  sqlite3BtreeEnter(p);
  n1 = (முழு)p->pBt->nReserveWanted;
  n2 = sqlite3BtreeGetReserveNoMutex(p);
  sqlite3BtreeLeave(p);
  திரும்பு n1>n2 ? n1 : n2;
}


/*
** Set the maximum page count for a database if mxPage is positive.
** No changes are made if mxPage is 0 or negative.
** Regardless of the value of mxPage, return the maximum page count.
*/
Pgno sqlite3_Btree_Max_Page_மொத்தம்(Btree *p, Pgno mxPage){
  Pgno n;
  sqlite3BtreeEnter(p);
  n = sqlite3_Pager_Max_Page_மொத்தம்(p->pBt->pPager, mxPage);
  sqlite3BtreeLeave(p);
  திரும்பு n;
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
முழு sqlite3BtreeSecureDelete(Btree *p, முழு newFlag){
  முழு b;
  எனில்( p==0 ) திரும்பு 0;
  sqlite3BtreeEnter(p);
  ப்ரதிஜ்ஞா( BTS_OVERWRITE==BTS_SECURE_DELETE*2 );
  ப்ரதிஜ்ஞா( BTS_FAST_SECURE==(BTS_OVERWRITE|BTS_SECURE_DELETE) );
  எனில்( newFlag>=0 ){
    p->pBt->btsFlags &= ~BTS_FAST_SECURE;
    p->pBt->btsFlags |= (u16)(BTS_SECURE_DELETE*newFlag);
  }
  b = (p->pBt->btsFlags & BTS_FAST_SECURE)/BTS_SECURE_DELETE;
  sqlite3BtreeLeave(p);
  திரும்பு b;
}

/*
** Change the 'auto-vacuum' property of the database. If the 'autoVacuum'
** parameter is non-zero, then auto-vacuum mode is enabled. If zero, it
** is disabled. The default value for the auto-vacuum property is
** determined by the SQLITE_DEFAULT_AUTOVACUUM macro.
*/
முழு sqlite3BtreeSetAutoVacuum(Btree *p, முழு autoVacuum){
#ifdef SQLITE_OMIT_AUTOVACUUM
  திரும்பு SQLITE_READONLY;
#இல்லையெனில்
  BtShared *pBt = p->pBt;
  முழு rc = SQLITE_OK;
  u8 av = (u8)autoVacuum;

  sqlite3BtreeEnter(p);
  எனில்( (pBt->btsFlags & BTS_PAGESIZE_FIXED)!=0 && (av ?1:0)!=pBt->autoVacuum ){
    rc = SQLITE_READONLY;
  }இல்லையெனில்{
    pBt->autoVacuum = av ?1:0;
    pBt->incrVacuum = av==2 ?1:0;
  }
  sqlite3BtreeLeave(p);
  திரும்பு rc;
#endif
}

/*
** Return the value of the 'auto-vacuum' property. If auto-vacuum is
** enabled 1 is returned. Otherwise 0.
*/
முழு sqlite3BtreeGetAutoVacuum(Btree *p){
#ifdef SQLITE_OMIT_AUTOVACUUM
  திரும்பு BTREE_AUTOVACUUM_NONE;
#இல்லையெனில்
  முழு rc;
  sqlite3BtreeEnter(p);
  rc = (
    (!p->pBt->autoVacuum)?BTREE_AUTOVACUUM_NONE:
    (!p->pBt->incrVacuum)?BTREE_AUTOVACUUM_FULL:
    BTREE_AUTOVACUUM_INCR
  );
  sqlite3BtreeLeave(p);
  திரும்பு rc;
#endif
}

/*
** If the user has not set the safety-level for this database connection
** using "PRAGMA synchronous", and if the safety-level is not already
** set to the value passed to this function as the second parameter,
** set it so.
*/
#எனில் SQLITE_DEFAULT_SYNCHRONOUS!=SQLITE_DEFAULT_WAL_SYNCHRONOUS \
    && !defined(SQLITE_OMIT_WAL)
நிலையான வெறுமை setDefaultSyncFlag(BtShared *pBt, u8 safety_level){
  sqlite3 *db;
  Db *pDb;
  எனில்( (db=pBt->db)!=0 && (pDb=db->aDb)!=0 ){
    போது( pDb->pBt==0 || pDb->pBt->pBt!=pBt ){ pDb++; }
    எனில்( pDb->bSyncSet==0
     && pDb->safety_level!=safety_level
     && pDb!=&db->aDb[1]
    ){
      pDb->safety_level = safety_level;
      sqlite3PagerSetFlags(pBt->pPager,
          pDb->safety_level | (db->flags & PAGER_FLAGS_MASK));
    }
  }
}
#இல்லையெனில்
# define setDefaultSyncFlag(pBt,safety_level)
#endif

/* Forward declaration */
நிலையான முழு newDatabase(BtShared*);


/*
** Get a reference to pPage1 of the database file.  This will
** also acquire a readlock on that file.
**
** SQLITE_OK is returned on success.  If the file is not a
** well-formed database file, then SQLITE_CORRUPT is returned.
** SQLITE_BUSY is returned if the database is locked.  SQLITE_NOMEM
** is returned if we run out of memory.
*/
நிலையான முழு lockBtree(BtShared *pBt){
  முழு rc;              /* Result code from subfunctions */
  MemPage *pPage1;     /* Page 1 of the database file */
  u32 nPage;           /* Number of pages in the database */
  u32 nPageFile = 0;   /* Number of pages in the database file */

  ப்ரதிஜ்ஞா( sqlite3_mutex_held(pBt->mutex) );
  ப்ரதிஜ்ஞா( pBt->pPage1==0 );
  rc = sqlite3PagerSharedLock(pBt->pPager);
  எனில்( rc!=SQLITE_OK ) திரும்பு rc;
  rc = btreeGetPage(pBt, 1, &pPage1, 0);
  எனில்( rc!=SQLITE_OK ) திரும்பு rc;

  /* Do some checking to help insure the file we opened really is
  ** a valid database file.
  */
  nPage = get4byte(28+(u8*)pPage1->aData);
  sqlite3PagerPagecount(pBt->pPager, (முழு*)&nPageFile);
  எனில்( nPage==0 || memcmp(24+(u8*)pPage1->aData, 92+(u8*)pPage1->aData,4)!=0 ){
    nPage = nPageFile;
  }
  எனில்( (pBt->db->flags & SQLITE_ResetDatabase)!=0 ){
    nPage = 0;
  }
  எனில்( nPage>0 ){
    u32 pageSize;
    u32 usableSize;
    u8 *page1 = pPage1->aData;
    rc = SQLITE_NOTADB;
    /* EVIDENCE-OF: R-43737-39999 Every valid SQLite database file begins
    ** with the following 16 bytes (in hex): 53 51 4c 69 74 65 20 66 6f 72 6d
    ** 61 74 20 33 00. */
    எனில்( memcmp(page1, zMagicHeader, 16)!=0 ){
      செல் page1_init_failed;
    }

#ifdef SQLITE_OMIT_WAL
    எனில்( page1[18]>1 ){
      pBt->btsFlags |= BTS_READ_ONLY;
    }
    எனில்( page1[19]>1 ){
      செல் page1_init_failed;
    }
#இல்லையெனில்
    எனில்( page1[18]>2 ){
      pBt->btsFlags |= BTS_READ_ONLY;
    }
    எனில்( page1[19]>2 ){
      செல் page1_init_failed;
    }

    /* If the read version is set to 2, this database should be accessed
    ** in WAL mode. If the log is not already open, open it now. Then
    ** return SQLITE_OK and return without populating BtShared.pPage1.
    ** The caller detects this and calls this function again. This is
    ** required as the version of page 1 currently in the page1 buffer
    ** may not be the latest version - there may be a newer one in the log
    ** file.
    */
    எனில்( page1[19]==2 && (pBt->btsFlags & BTS_NO_WAL)==0 ){
      முழு isOpen = 0;
      rc = sqlite3PagerOpenWal(pBt->pPager, &isOpen);
      எனில்( rc!=SQLITE_OK ){
        செல் page1_init_failed;
      }இல்லையெனில்{
        setDefaultSyncFlag(pBt, SQLITE_DEFAULT_WAL_SYNCHRONOUS+1);
        எனில்( isOpen==0 ){
          releasePageOne(pPage1);
          திரும்பு SQLITE_OK;
        }
      }
      rc = SQLITE_NOTADB;
    }இல்லையெனில்{
      setDefaultSyncFlag(pBt, SQLITE_DEFAULT_SYNCHRONOUS+1);
    }
#endif

    /* EVIDENCE-OF: R-15465-20813 The maximum and minimum embedded payload
    ** fractions and the leaf payload fraction values must be 64, 32, and 32.
    **
    ** The original design allowed these amounts to vary, but as of
    ** version 3.6.0, we require them to be fixed.
    */
    எனில்( memcmp(&page1[21], "\100\040\040",3)!=0 ){
      செல் page1_init_failed;
    }
    /* EVIDENCE-OF: R-51873-39618 The page size for a database file is
    ** determined by the 2-byte integer located at an offset of 16 bytes from
    ** the beginning of the database file. */
    pageSize = (page1[16]<<8) | (page1[17]<<16);
    /* EVIDENCE-OF: R-25008-21688 The size of a page is a power of two
    ** between 512 and 65536 inclusive. */
    எனில்( ((pageSize-1)&pageSize)!=0
     || pageSize>SQLITE_MAX_PAGE_SIZE
     || pageSize<=256
    ){
      செல் page1_init_failed;
    }
    ப்ரதிஜ்ஞா( (pageSize & 7)==0 );
    /* EVIDENCE-OF: R-59310-51205 The "reserved space" size in the 1-byte
    ** integer at offset 20 is the number of bytes of space at the end of
    ** each page to reserve for extensions.
    **
    ** EVIDENCE-OF: R-37497-42412 The size of the reserved region is
    ** determined by the one-byte unsigned integer found at an offset of 20
    ** into the database file header. */
    usableSize = pageSize - page1[20];
    எனில்( (u32)pageSize!=pBt->pageSize ){
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
      திரும்பு rc;
    }
    எனில்( nPage>nPageFile ){
      எனில்( sqlite3WritableSchema(pBt->db)==0 ){
        rc = SQLITE_CORRUPT_BKPT;
        செல் page1_init_failed;
      }இல்லையெனில்{
        nPage = nPageFile;
      }
    }
    /* EVIDENCE-OF: R-28312-64704 However, the usable size is not allowed to
    ** be less than 480. In other words, if the page size is 512, then the
    ** reserved space size cannot exceed 32. */
    எனில்( usableSize<480 ){
      செல் page1_init_failed;
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
  எனில்( pBt->maxLocal>127 ){
    pBt->max1bytePayload = 127;
  }இல்லையெனில்{
    pBt->max1bytePayload = (u8)pBt->maxLocal;
  }
  ப்ரதிஜ்ஞா( pBt->maxLeaf + 23 <= MX_CELL_SIZE(pBt) );
  pBt->pPage1 = pPage1;
  pBt->nPage = nPage;
  திரும்பு SQLITE_OK;

page1_init_failed:
  releasePageOne(pPage1);
  pBt->pPage1 = 0;
  திரும்பு rc;
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
நிலையான முழு மொத்தம்_Valid_Cursors(BtShared *pBt, முழு wrOnly){
  BtCursor *pCur;
  முழு r = 0;
  க்கு(pCur=pBt->pCursor; pCur; pCur=pCur->pNext){
    எனில்( (wrOnly==0 || (pCur->curFlags & BTCF_WriteFlag)!=0)
     && pCur->eState!=CURSOR_FAULT ) r++;
  }
  திரும்பு r;
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
நிலையான வெறுமை unlockBtreeIfUnused(BtShared *pBt){
  ப்ரதிஜ்ஞா( sqlite3_mutex_held(pBt->mutex) );
  ப்ரதிஜ்ஞா( மொத்தம்_Valid_Cursors(pBt,0)==0 || pBt->inTransaction>TRANS_NONE );
  எனில்( pBt->inTransaction==TRANS_NONE && pBt->pPage1!=0 ){
    MemPage *pPage1 = pBt->pPage1;
    ப்ரதிஜ்ஞா( pPage1->aData );
    ப்ரதிஜ்ஞா( sqlite3PagerRefcount(pBt->pPager)==1 );
    pBt->pPage1 = 0;
    releasePageOne(pPage1);
  }
}

/*
** If pBt points to an empty file then convert that empty file
** into a new empty database by initializing the first page of
** the database.
*/
நிலையான முழு newDatabase(BtShared *pBt){
  MemPage *pP1;
  unsigned எழுத்து *data;
  முழு rc;

  ப்ரதிஜ்ஞா( sqlite3_mutex_held(pBt->mutex) );
  எனில்( pBt->nPage>0 ){
    திரும்பு SQLITE_OK;
  }
  pP1 = pBt->pPage1;
  ப்ரதிஜ்ஞா( pP1!=0 );
  data = pP1->aData;
  rc = sqlite3PagerWrite(pP1->pDbPage);
  எனில்( rc ) திரும்பு rc;
  memcpy(data, zMagicHeader, அளவு(zMagicHeader));
  ப்ரதிஜ்ஞா( அளவு(zMagicHeader)==16 );
  data[16] = (u8)((pBt->pageSize>>8)&0xff);
  data[17] = (u8)((pBt->pageSize>>16)&0xff);
  data[18] = 1;
  data[19] = 1;
  ப்ரதிஜ்ஞா( pBt->usableSize<=pBt->pageSize && pBt->usableSize+255>=pBt->pageSize);
  data[20] = (u8)(pBt->pageSize - pBt->usableSize);
  data[21] = 64;
  data[22] = 32;
  data[23] = 32;
  memset(&data[24], 0, 100-24);
  zeroPage(pP1, PTF_INTKEY|PTF_LEAF|PTF_LEAFDATA );
  pBt->btsFlags |= BTS_PAGESIZE_FIXED;
#ifndef SQLITE_OMIT_AUTOVACUUM
  ப்ரதிஜ்ஞா( pBt->autoVacuum==1 || pBt->autoVacuum==0 );
  ப்ரதிஜ்ஞா( pBt->incrVacuum==1 || pBt->incrVacuum==0 );
  put4byte(&data[36 + 4*4], pBt->autoVacuum);
  put4byte(&data[36 + 7*4], pBt->incrVacuum);
#endif
  pBt->nPage = 1;
  data[31] = 1;
  திரும்பு SQLITE_OK;
}

/*
** Initialize the first page of the database file (creating a database
** consisting of a single page and no schema objects). Return SQLITE_OK
** if successful, or an SQLite error code otherwise.
*/
முழு sqlite3BtreeNewDb(Btree *p){
  முழு rc;
  sqlite3BtreeEnter(p);
  p->pBt->nPage = 0;
  rc = newDatabase(p->pBt);
  sqlite3BtreeLeave(p);
  திரும்பு rc;
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
நிலையான SQLITE_NOINLINE முழு btreeBeginTrans(
  Btree *p,                 /* The btree in which to start the transaction */
  முழு wrflag,               /* True to start a write transaction */
  முழு *pSchemaVersion       /* Put schema version number here, if not NULL */
){
  BtShared *pBt = p->pBt;
  Pager *pPager = pBt->pPager;
  முழு rc = SQLITE_OK;

  sqlite3BtreeEnter(p);
  btreeIntegrity(p);

  /* If the btree is already in a write-transaction, or it
  ** is already in a read-transaction and a read-transaction
  ** is requested, this is a no-op.
  */
  எனில்( p->inTrans==TRANS_WRITE || (p->inTrans==TRANS_READ && !wrflag) ){
    செல் trans_begun;
  }
  ப்ரதிஜ்ஞா( pBt->inTransaction==TRANS_WRITE || IfNotOmitAV(pBt->bDoTruncate)==0 );

  எனில்( (p->db->flags & SQLITE_ResetDatabase)
   && sqlite3PagerIsreadonly(pPager)==0
  ){
    pBt->btsFlags &= ~BTS_READ_ONLY;
  }

  /* Write transactions are not possible on a read-only database */
  எனில்( (pBt->btsFlags & BTS_READ_ONLY)!=0 && wrflag ){
    rc = SQLITE_READONLY;
    செல் trans_begun;
  }

#ifndef SQLITE_OMIT_SHARED_CACHE
  {
    sqlite3 *pBlock = 0;
    /* If another database handle has already opened a write transaction
    ** on this shared-btree structure and a second write transaction is
    ** requested, return SQLITE_LOCKED.
    */
    எனில்( (wrflag && pBt->inTransaction==TRANS_WRITE)
     || (pBt->btsFlags & BTS_PENDING)!=0
    ){
      pBlock = pBt->pWriter->db;
    }இல்லையெனில் எனில்( wrflag>1 ){
      BtLock *pIter;
      க்கு(pIter=pBt->pLock; pIter; pIter=pIter->pNext){
        எனில்( pIter->pBtree!=p ){
          pBlock = pIter->pBtree->db;
          நிறுத்து;
        }
      }
    }
    எனில்( pBlock ){
      sqlite3ConnectionBlocked(p->db, pBlock);
      rc = SQLITE_LOCKED_SHAREDCACHE;
      செல் trans_begun;
    }
  }
#endif

  /* Any read-only or read-write transaction implies a read-lock on
  ** page 1. So if some other shared-cache client already has a write-lock
  ** on page 1, the transaction cannot be opened. */
  rc = querySharedCacheTableLock(p, SCHEMA_வேர், READ_LOCK);
  எனில்( SQLITE_OK!=rc ) செல் trans_begun;

  pBt->btsFlags &= ~BTS_INITIALLY_EMPTY;
  எனில்( pBt->nPage==0 ) pBt->btsFlags |= BTS_INITIALLY_EMPTY;
  செய் {
    sqlite3PagerWalDb(pPager, p->db);

#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
    /* If transitioning from no transaction directly to a write transaction,
    ** block for the WRITER lock first if possible. */
    எனில்( pBt->pPage1==0 && wrflag ){
      ப்ரதிஜ்ஞா( pBt->inTransaction==TRANS_NONE );
      rc = sqlite3PagerWalWriteLock(pPager, 1);
      எனில்( rc!=SQLITE_BUSY && rc!=SQLITE_OK ) நிறுத்து;
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
    எனில்( pBt->pPage1==0 && wrflag ){
      sqlite3_file *fd = sqlite3PagerFile(pPager);
      எழுத்து *aFcntl[3] = {0,0,0};
      aFcntl[1] = "experimental_pragma_20251114";
      ப்ரதிஜ்ஞா( wrflag==1 || wrflag==2 );
      aFcntl[2] = (wrflag==1 ? "1" : "2");
      sqlite3OsFileControlHint(fd, SQLITE_FCNTL_PRAGMA, (வெறுமை*)aFcntl);
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
    போது( pBt->pPage1==0 && SQLITE_OK==(rc = lockBtree(pBt)) );

    எனில்( rc==SQLITE_OK && wrflag ){
      எனில்( (pBt->btsFlags & BTS_READ_ONLY)!=0 ){
        rc = SQLITE_READONLY;
      }இல்லையெனில்{
        rc = sqlite3PagerBegin(pPager, wrflag>1, sqlite3TempInMemory(p->db));
        எனில்( rc==SQLITE_OK ){
          rc = newDatabase(pBt);
        }இல்லையெனில் எனில்( rc==SQLITE_BUSY_SNAPSHOT && pBt->inTransaction==TRANS_NONE ){
          /* if there was no transaction opened when this function was
          ** called and SQLITE_BUSY_SNAPSHOT is returned, change the error
          ** code to SQLITE_BUSY. */
          rc = SQLITE_BUSY;
        }
      }
    }
 
    எனில்( rc!=SQLITE_OK ){
      (வெறுமை)sqlite3PagerWalWriteLock(pPager, 0);
      unlockBtreeIfUnused(pBt);
    }
#எனில் defined(SQLITE_ENABLE_SETLK_TIMEOUT)
    எனில்( rc==SQLITE_BUSY_TIMEOUT ){
      /* If a blocking lock timed out, break out of the loop here so that
      ** the busy-handler is not invoked.  */
      நிறுத்து;
    }
#endif
  }போது( (rc&0xFF)==SQLITE_BUSY && pBt->inTransaction==TRANS_NONE &&
          btreeInvokeBusyHandler(pBt) );
  sqlite3PagerWalDb(pPager, 0);
#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
  எனில்( rc==SQLITE_BUSY_TIMEOUT ) rc = SQLITE_BUSY;
#endif

  எனில்( rc==SQLITE_OK ){
    எனில்( p->inTrans==TRANS_NONE ){
      pBt->nTransaction++;
#ifndef SQLITE_OMIT_SHARED_CACHE
      எனில்( p->sharable ){
        ப்ரதிஜ்ஞா( p->lock.pBtree==p && p->lock.iTable==1 );
        p->lock.eLock = READ_LOCK;
        p->lock.pNext = pBt->pLock;
        pBt->pLock = &p->lock;
      }
#endif
    }
    p->inTrans = (wrflag?TRANS_WRITE:TRANS_READ);
    எனில்( p->inTrans>pBt->inTransaction ){
      pBt->inTransaction = p->inTrans;
    }
    எனில்( wrflag ){
      MemPage *pPage1 = pBt->pPage1;
#ifndef SQLITE_OMIT_SHARED_CACHE
      ப்ரதிஜ்ஞா( !pBt->pWriter );
      pBt->pWriter = p;
      pBt->btsFlags &= ~BTS_EXCLUSIVE;
      எனில்( wrflag>1 ) pBt->btsFlags |= BTS_EXCLUSIVE;
#endif

      /* If the db-size header field is incorrect (as it may be if an old
      ** client has been writing the database file), update it now. Doing
      ** this sooner rather than later means the database size can safely
      ** re-read the database size from page 1 if a savepoint or transaction
      ** rollback occurs within the transaction.
      */
      எனில்( pBt->nPage!=get4byte(&pPage1->aData[28]) ){
        rc = sqlite3PagerWrite(pPage1->pDbPage);
        எனில்( rc==SQLITE_OK ){
          put4byte(&pPage1->aData[28], pBt->nPage);
        }
      }
    }
  }

trans_begun:
  எனில்( rc==SQLITE_OK ){
    எனில்( pSchemaVersion ){
      *pSchemaVersion = get4byte(&pBt->pPage1->aData[40]);
    }
    எனில்( wrflag ){
      /* This call makes sure that the pager has the correct number of
      ** open savepoints. If the second parameter is greater than 0 and
      ** the sub-journal is not already open, then it will be opened here.
      */
      rc = sqlite3PagerOpenSavepoint(pPager, p->db->nSavepoint);
    }
  }

  btreeIntegrity(p);
  sqlite3BtreeLeave(p);
  திரும்பு rc;
}
முழு sqlite3BtreeBeginTrans(Btree *p, முழு wrflag, முழு *pSchemaVersion){
  BtShared *pBt;
  எனில்( p->sharable
   || p->inTrans==TRANS_NONE
   || (p->inTrans==TRANS_READ && wrflag!=0)
  ){
    திரும்பு btreeBeginTrans(p,wrflag,pSchemaVersion);
  }
  pBt = p->pBt;
  எனில்( pSchemaVersion ){
    *pSchemaVersion = get4byte(&pBt->pPage1->aData[40]);
  }
  எனில்( wrflag ){
    /* This call makes sure that the pager has the correct number of
    ** open savepoints. If the second parameter is greater than 0 and
    ** the sub-journal is not already open, then it will be opened here.
    */
    திரும்பு sqlite3PagerOpenSavepoint(pBt->pPager, p->db->nSavepoint);
  }இல்லையெனில்{
    திரும்பு SQLITE_OK;
  }
}

#ifndef SQLITE_OMIT_AUTOVACUUM

/*
** Set the pointer-map entries for all children of page pPage. Also, if
** pPage contains cells that point to overflow pages, set the pointer
** map entries for the overflow pages as well.
*/
நிலையான முழு setChildPtrmaps(MemPage *pPage){
  முழு i;                             /* Counter variable */
  முழு nCell;                         /* Number of cells in page pPage */
  முழு rc;                            /* Return code */
  BtShared *pBt = pPage->pBt;
  Pgno pgno = pPage->pgno;

  ப்ரதிஜ்ஞா( sqlite3_mutex_held(pPage->pBt->mutex) );
  rc = pPage->isInit ? SQLITE_OK : btreeInitPage(pPage);
  எனில்( rc!=SQLITE_OK ) திரும்பு rc;
  nCell = pPage->nCell;

  க்கு(i=0; i<nCell; i++){
    u8 *pCell = findCell(pPage, i);

    ptrmapPutOvflPtr(pPage, pPage, pCell, &rc);

    எனில்( !pPage->leaf ){
      Pgno childPgno = get4byte(pCell);
      ptrmapPut(pBt, childPgno, PTRMAP_BTREE, pgno, &rc);
    }
  }

  எனில்( !pPage->leaf ){
    Pgno childPgno = get4byte(&pPage->aData[pPage->hdrOffset+8]);
    ptrmapPut(pBt, childPgno, PTRMAP_BTREE, pgno, &rc);
  }

  திரும்பு rc;
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
நிலையான முழு modifyPagePointer(MemPage *pPage, Pgno iFrom, Pgno iTo, u8 eType){
  ப்ரதிஜ்ஞா( sqlite3_mutex_held(pPage->pBt->mutex) );
  ப்ரதிஜ்ஞா( sqlite3PagerIswriteable(pPage->pDbPage) );
  எனில்( eType==PTRMAP_OVERFLOW2 ){
    /* The pointer is always the first 4 bytes of the page in this case.  */
    எனில்( get4byte(pPage->aData)!=iFrom ){
      திரும்பு SQLITE_CORRUPT_PAGE(pPage);
    }
    put4byte(pPage->aData, iTo);
  }இல்லையெனில்{
    முழு i;
    முழு nCell;
    முழு rc;

    rc = pPage->isInit ? SQLITE_OK : btreeInitPage(pPage);
    எனில்( rc ) திரும்பு rc;
    nCell = pPage->nCell;

    க்கு(i=0; i<nCell; i++){
      u8 *pCell = findCell(pPage, i);
      எனில்( eType==PTRMAP_OVERFLOW1 ){
        CellInfo info;
        pPage->xParseCell(pPage, pCell, &info);
        எனில்( info.nLocal<info.nPayload ){
          எனில்( pCell+info.nSize > pPage->aData+pPage->pBt->usableSize ){
            திரும்பு SQLITE_CORRUPT_PAGE(pPage);
          }
          எனில்( iFrom==get4byte(pCell+info.nSize-4) ){
            put4byte(pCell+info.nSize-4, iTo);
            நிறுத்து;
          }
        }
      }இல்லையெனில்{
        எனில்( pCell+4 > pPage->aData+pPage->pBt->usableSize ){
          திரும்பு SQLITE_CORRUPT_PAGE(pPage);
        }
        எனில்( get4byte(pCell)==iFrom ){
          put4byte(pCell, iTo);
          நிறுத்து;
        }
      }
    }
 
    எனில்( i==nCell ){
      எனில்( eType!=PTRMAP_BTREE ||
          get4byte(&pPage->aData[pPage->hdrOffset+8])!=iFrom ){
        திரும்பு SQLITE_CORRUPT_PAGE(pPage);
      }
      put4byte(&pPage->aData[pPage->hdrOffset+8], iTo);
    }
  }
  திரும்பு SQLITE_OK;
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
நிலையான முழு relocatePage(
  BtShared *pBt,           /* Btree */
  MemPage *pDbPage,        /* Open page to move */
  u8 eType,                /* Pointer map 'type' entry for pDbPage */
  Pgno iPtrPage,           /* Pointer map 'page-no' entry for pDbPage */
  Pgno iFreePage,          /* The location to move pDbPage to */
  முழு isCommit             /* isCommit flag passed to sqlite3PagerMovepage */
){
  MemPage *pPtrPage;   /* The page that contains a pointer to pDbPage */
  Pgno iDbPage = pDbPage->pgno;
  Pager *pPager = pBt->pPager;
  முழு rc;

  ப்ரதிஜ்ஞா( eType==PTRMAP_OVERFLOW2 || eType==PTRMAP_OVERFLOW1 ||
      eType==PTRMAP_BTREE || eType==PTRMAP_ROOTPAGE );
  ப்ரதிஜ்ஞா( sqlite3_mutex_held(pBt->mutex) );
  ப்ரதிஜ்ஞா( pDbPage->pBt==pBt );
  எனில்( iDbPage<3 ) திரும்பு SQLITE_CORRUPT_BKPT;

  /* Move page iDbPage from its current location to page number iFreePage */
  TRACE(("AUTOVACUUM: Moving %u to free page %u (ptr page %u type %u)\n",
      iDbPage, iFreePage, iPtrPage, eType));
  rc = sqlite3PagerMovepage(pPager, pDbPage->pDbPage, iFreePage, isCommit);
  எனில்( rc!=SQLITE_OK ){
    திரும்பு rc;
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
  எனில்( eType==PTRMAP_BTREE || eType==PTRMAP_ROOTPAGE ){
    rc = setChildPtrmaps(pDbPage);
    எனில்( rc!=SQLITE_OK ){
      திரும்பு rc;
    }
  }இல்லையெனில்{
    Pgno nextOvfl = get4byte(pDbPage->aData);
    எனில்( nextOvfl!=0 ){
      ptrmapPut(pBt, nextOvfl, PTRMAP_OVERFLOW2, iFreePage, &rc);
      எனில்( rc!=SQLITE_OK ){
        திரும்பு rc;
      }
    }
  }

  /* Fix the database pointer on page iPtrPage that pointed at iDbPage so
  ** that it points at iFreePage. Also fix the pointer map entry for
  ** iPtrPage.
  */
  எனில்( eType!=PTRMAP_ROOTPAGE ){
    rc = btreeGetPage(pBt, iPtrPage, &pPtrPage, 0);
    எனில்( rc!=SQLITE_OK ){
      திரும்பு rc;
    }
    rc = sqlite3PagerWrite(pPtrPage->pDbPage);
    எனில்( rc!=SQLITE_OK ){
      releasePage(pPtrPage);
      திரும்பு rc;
    }
    rc = modifyPagePointer(pPtrPage, iDbPage, iFreePage, eType);
    releasePage(pPtrPage);
    எனில்( rc==SQLITE_OK ){
      ptrmapPut(pBt, iFreePage, eType, iPtrPage, &rc);
    }
  }
  திரும்பு rc;
}

/* Forward declaration required by incrVacuumStep(). */
நிலையான முழு allocateBtreePage(BtShared *, MemPage **, Pgno *, Pgno, u8);

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
நிலையான முழு incrVacuumStep(BtShared *pBt, Pgno nFin, Pgno iLastPg, முழு bCommit){
  Pgno nFreeList;           /* Number of pages still on the free-list */
  முழு rc;

  ப்ரதிஜ்ஞா( sqlite3_mutex_held(pBt->mutex) );
  ப்ரதிஜ்ஞா( iLastPg>nFin );

  எனில்( !PTRMAP_ISPAGE(pBt, iLastPg) && iLastPg!=PENDING_BYTE_PAGE(pBt) ){
    u8 eType;
    Pgno iPtrPage;

    nFreeList = get4byte(&pBt->pPage1->aData[36]);
    எனில்( nFreeList==0 ){
      திரும்பு SQLITE_DONE;
    }

    rc = ptrmapGet(pBt, iLastPg, &eType, &iPtrPage);
    எனில்( rc!=SQLITE_OK ){
      திரும்பு rc;
    }
    எனில்( eType==PTRMAP_ROOTPAGE ){
      திரும்பு SQLITE_CORRUPT_BKPT;
    }

    எனில்( eType==PTRMAP_FREEPAGE ){
      எனில்( bCommit==0 ){
        /* Remove the page from the files free-list. This is not required
        ** if bCommit is non-zero. In that case, the free-list will be
        ** truncated to zero after this function returns, so it doesn't
        ** matter if it still contains some garbage entries.
        */
        Pgno iFreePg;
        MemPage *pFreePg;
        rc = allocateBtreePage(pBt, &pFreePg, &iFreePg, iLastPg, BTALLOC_EXACT);
        எனில்( rc!=SQLITE_OK ){
          திரும்பு rc;
        }
        ப்ரதிஜ்ஞா( iFreePg==iLastPg );
        releasePage(pFreePg);
      }
    } இல்லையெனில் {
      Pgno iFreePg;             /* Index of free page to move pLastPg to */
      MemPage *pLastPg;
      u8 eMode = BTALLOC_ANY;   /* Mode parameter for allocateBtreePage() */
      Pgno iNear = 0;           /* nearby parameter for allocateBtreePage() */

      rc = btreeGetPage(pBt, iLastPg, &pLastPg, 0);
      எனில்( rc!=SQLITE_OK ){
        திரும்பு rc;
      }

      /* If bCommit is zero, this loop runs exactly once and page pLastPg
      ** is swapped with the first free page pulled off the free list.
      **
      ** On the other hand, if bCommit is greater than zero, then keep
      ** looping until a free-page located within the first nFin pages
      ** of the file is found.
      */
      எனில்( bCommit==0 ){
        eMode = BTALLOC_LE;
        iNear = nFin;
      }
      செய் {
        MemPage *pFreePg;
        Pgno dbSize = btreePagecount(pBt);
        rc = allocateBtreePage(pBt, &pFreePg, &iFreePg, iNear, eMode);
        எனில்( rc!=SQLITE_OK ){
          releasePage(pLastPg);
          திரும்பு rc;
        }
        releasePage(pFreePg);
        எனில்( iFreePg>dbSize ){
          releasePage(pLastPg);
          திரும்பு SQLITE_CORRUPT_BKPT;
        }
      }போது( bCommit && iFreePg>nFin );
      ப்ரதிஜ்ஞா( iFreePg<iLastPg );
     
      rc = relocatePage(pBt, pLastPg, eType, iPtrPage, iFreePg, bCommit);
      releasePage(pLastPg);
      எனில்( rc!=SQLITE_OK ){
        திரும்பு rc;
      }
    }
  }

  எனில்( bCommit==0 ){
    செய் {
      iLastPg--;
    }போது( iLastPg==PENDING_BYTE_PAGE(pBt) || PTRMAP_ISPAGE(pBt, iLastPg) );
    pBt->bDoTruncate = 1;
    pBt->nPage = iLastPg;
  }
  திரும்பு SQLITE_OK;
}

/*
** The database opened by the first argument is an auto-vacuum database
** nOrig pages in size containing nFree free pages. Return the expected
** size of the database in pages following an auto-vacuum operation.
*/
நிலையான Pgno finalDbSize(BtShared *pBt, Pgno nOrig, Pgno nFree){
  முழு nEntry;                     /* Number of entries on one ptrmap page */
  Pgno nPtrmap;                   /* Number of PtrMap pages to be freed */
  Pgno nFin;                      /* Return value */

  nEntry = pBt->usableSize/5;
  nPtrmap = (nFree-nOrig+PTRMAP_PAGENO(pBt, nOrig)+nEntry)/nEntry;
  nFin = nOrig - nFree - nPtrmap;
  எனில்( nOrig>PENDING_BYTE_PAGE(pBt) && nFin<PENDING_BYTE_PAGE(pBt) ){
    nFin--;
  }
  போது( PTRMAP_ISPAGE(pBt, nFin) || nFin==PENDING_BYTE_PAGE(pBt) ){
    nFin--;
  }

  திரும்பு nFin;
}

/*
** A write-transaction must be opened before calling this function.
** It performs a single unit of work towards an incremental vacuum.
**
** If the incremental vacuum is finished after this function has run,
** SQLITE_DONE is returned. If it is not finished, but no error occurred,
** SQLITE_OK is returned. Otherwise an SQLite error code.
*/
முழு sqlite3BtreeIncrVacuum(Btree *p){
  முழு rc;
  BtShared *pBt = p->pBt;

  sqlite3BtreeEnter(p);
  ப்ரதிஜ்ஞா( pBt->inTransaction==TRANS_WRITE && p->inTrans==TRANS_WRITE );
  எனில்( !pBt->autoVacuum ){
    rc = SQLITE_DONE;
  }இல்லையெனில்{
    Pgno nOrig = btreePagecount(pBt);
    Pgno nFree = get4byte(&pBt->pPage1->aData[36]);
    Pgno nFin = finalDbSize(pBt, nOrig, nFree);

    எனில்( nOrig<nFin || nFree>=nOrig ){
      rc = SQLITE_CORRUPT_BKPT;
    }இல்லையெனில் எனில்( nFree>0 ){
      rc = saveAllCursors(pBt, 0, 0);
      எனில்( rc==SQLITE_OK ){
        invalidateAllOverflowCache(pBt);
        rc = incrVacuumStep(pBt, nFin, nOrig, 0);
      }
      எனில்( rc==SQLITE_OK ){
        rc = sqlite3PagerWrite(pBt->pPage1->pDbPage);
        put4byte(&pBt->pPage1->aData[28], pBt->nPage);
      }
    }இல்லையெனில்{
      rc = SQLITE_DONE;
    }
  }
  sqlite3BtreeLeave(p);
  திரும்பு rc;
}

/*
** This routine is called prior to sqlite3PagerCommit when a transaction
** is committed for an auto-vacuum database.
*/
நிலையான முழு autoVacuumCommit(Btree *p){
  முழு rc = SQLITE_OK;
  Pager *pPager;
  BtShared *pBt;
  sqlite3 *db;
  VVA_ONLY( முழு nRef );

  ப்ரதிஜ்ஞா( p!=0 );
  pBt = p->pBt; 
  pPager = pBt->pPager;
  VVA_ONLY( nRef = sqlite3PagerRefcount(pPager); )

  ப்ரதிஜ்ஞா( sqlite3_mutex_held(pBt->mutex) );
  invalidateAllOverflowCache(pBt);
  ப்ரதிஜ்ஞா(pBt->autoVacuum);
  எனில்( !pBt->incrVacuum ){
    Pgno nFin;         /* Number of pages in database after autovacuuming */
    Pgno nFree;        /* Number of pages on the freelist initially */
    Pgno nVac;         /* Number of pages to vacuum */
    Pgno iFree;        /* The next page to be freed */
    Pgno nOrig;        /* Database size before freeing */

    nOrig = btreePagecount(pBt);
    எனில்( PTRMAP_ISPAGE(pBt, nOrig) || nOrig==PENDING_BYTE_PAGE(pBt) ){
      /* It is not possible to create a database for which the final page
      ** is either a pointer-map page or the pending-byte page. If one
      ** is encountered, this indicates corruption.
      */
      திரும்பு SQLITE_CORRUPT_BKPT;
    }

    nFree = get4byte(&pBt->pPage1->aData[36]);
    db = p->db;
    எனில்( db->xAutovacPages ){
      முழு iDb;
      க்கு(iDb=0; ALWAYS(iDb<db->nDb); iDb++){
        எனில்( db->aDb[iDb].pBt==p ) நிறுத்து;
      }
      nVac = db->xAutovacPages(
        db->pAutovacPagesArg,
        db->aDb[iDb].zDbSName,
        nOrig,
        nFree,
        pBt->pageSize
      );
      எனில்( nVac>nFree ){
        nVac = nFree;
      }
      எனில்( nVac==0 ){
        திரும்பு SQLITE_OK;
      }
    }இல்லையெனில்{
      nVac = nFree;
    }
    nFin = finalDbSize(pBt, nOrig, nVac);
    எனில்( nFin>nOrig ) திரும்பு SQLITE_CORRUPT_BKPT;
    எனில்( nFin<nOrig ){
      rc = saveAllCursors(pBt, 0, 0);
    }
    க்கு(iFree=nOrig; iFree>nFin && rc==SQLITE_OK; iFree--){
      rc = incrVacuumStep(pBt, nFin, iFree, nVac==nFree);
    }
    எனில்( (rc==SQLITE_DONE || rc==SQLITE_OK) && nFree>0 ){
      rc = sqlite3PagerWrite(pBt->pPage1->pDbPage);
      எனில்( nVac==nFree ){
        put4byte(&pBt->pPage1->aData[32], 0);
        put4byte(&pBt->pPage1->aData[36], 0);
      }
      put4byte(&pBt->pPage1->aData[28], nFin);
      pBt->bDoTruncate = 1;
      pBt->nPage = nFin;
    }
    எனில்( rc!=SQLITE_OK ){
      sqlite3PagerRollback(pPager);
    }
  }

  ப்ரதிஜ்ஞா( nRef>=sqlite3PagerRefcount(pPager) );
  திரும்பு rc;
}

#இல்லையெனில் /* ifndef SQLITE_OMIT_AUTOVACUUM */
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
முழு sqlite3BtreeCommitPhaseOne(Btree *p, மாறிலி எழுத்து *zSuperJrnl){
  முழு rc = SQLITE_OK;
  எனில்( p->inTrans==TRANS_WRITE ){
    BtShared *pBt = p->pBt;
    sqlite3BtreeEnter(p);
#ifndef SQLITE_OMIT_AUTOVACUUM
    எனில்( pBt->autoVacuum ){
      rc = autoVacuumCommit(p);
      எனில்( rc!=SQLITE_OK ){
        sqlite3BtreeLeave(p);
        திரும்பு rc;
      }
    }
    எனில்( pBt->bDoTruncate ){
      sqlite3PagerTruncateImage(pBt->pPager, pBt->nPage);
    }
#endif
    rc = sqlite3PagerCommitPhaseOne(pBt->pPager, zSuperJrnl, 0);
    sqlite3BtreeLeave(p);
  }
  திரும்பு rc;
}

/*
** This function is called from both BtreeCommitPhaseTwo() and BtreeRollback()
** at the conclusion of a transaction.
*/
நிலையான வெறுமை btreeEndTransaction(Btree *p){
  BtShared *pBt = p->pBt;
  sqlite3 *db = p->db;
  ப்ரதிஜ்ஞா( sqlite3BtreeHoldsMutex(p) );

#ifndef SQLITE_OMIT_AUTOVACUUM
  pBt->bDoTruncate = 0;
#endif
  எனில்( p->inTrans>TRANS_NONE && db->nVdbeRead>1 ){
    /* If there are other active statements that belong to this database
    ** handle, downgrade to a read-only transaction. The other statements
    ** may still be reading from the database.  */
    downgradeAllSharedCacheTableLocks(p);
    p->inTrans = TRANS_READ;
  }இல்லையெனில்{
    /* If the handle had any kind of transaction open, decrement the
    ** transaction count of the shared btree. If the transaction count
    ** reaches 0, set the shared state to TRANS_NONE. The unlockBtreeIfUnused()
    ** call below will unlock the pager.  */
    எனில்( p->inTrans!=TRANS_NONE ){
      clearAllSharedCacheTableLocks(p);
      pBt->nTransaction--;
      எனில்( 0==pBt->nTransaction ){
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
முழு sqlite3BtreeCommitPhaseTwo(Btree *p, முழு bCleanup){

  எனில்( p->inTrans==TRANS_NONE ) திரும்பு SQLITE_OK;
  sqlite3BtreeEnter(p);
  btreeIntegrity(p);

  /* If the handle has a write-transaction open, commit the shared-btrees
  ** transaction and set the shared state to TRANS_READ.
  */
  எனில்( p->inTrans==TRANS_WRITE ){
    முழு rc;
    BtShared *pBt = p->pBt;
    ப்ரதிஜ்ஞா( pBt->inTransaction==TRANS_WRITE );
    ப்ரதிஜ்ஞா( pBt->nTransaction>0 );
    rc = sqlite3PagerCommitPhaseTwo(pBt->pPager);
    எனில்( rc!=SQLITE_OK && bCleanup==0 ){
      sqlite3BtreeLeave(p);
      திரும்பு rc;
    }
    p->iBDataVersion--;  /* Compensate for pPager->iDataVersion++; */
    pBt->inTransaction = TRANS_READ;
    btreeClearHasContent(pBt);
  }

  btreeEndTransaction(p);
  sqlite3BtreeLeave(p);
  திரும்பு SQLITE_OK;
}

/*
** Do both phases of a commit.
*/
முழு sqlite3BtreeCommit(Btree *p){
  முழு rc;
  sqlite3BtreeEnter(p);
  rc = sqlite3BtreeCommitPhaseOne(p, 0);
  எனில்( rc==SQLITE_OK ){
    rc = sqlite3BtreeCommitPhaseTwo(p, 0);
  }
  sqlite3BtreeLeave(p);
  திரும்பு rc;
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
முழு sqlite3BtreeTripAllCursors(Btree *pBtree, முழு errCode, முழு writeOnly){
  BtCursor *p;
  முழு rc = SQLITE_OK;

  ப்ரதிஜ்ஞா( (writeOnly==0 || writeOnly==1) && BTCF_WriteFlag==1 );
  எனில்( pBtree ){
    sqlite3BtreeEnter(pBtree);
    க்கு(p=pBtree->pBt->pCursor; p; p=p->pNext){
      எனில்( writeOnly && (p->curFlags & BTCF_WriteFlag)==0 ){
        எனில்( p->eState==CURSOR_VALID || p->eState==CURSOR_SKIPNEXT ){
          rc = saveCursorPosition(p);
          எனில்( rc!=SQLITE_OK ){
            (வெறுமை)sqlite3BtreeTripAllCursors(pBtree, rc, 0);
            நிறுத்து;
          }
        }
      }இல்லையெனில்{
        sqlite3BtreeClearCursor(p);
        p->eState = CURSOR_FAULT;
        p->skipNext = errCode;
      }
      btreeReleaseAllCursorPages(p);
    }
    sqlite3BtreeLeave(pBtree);
  }
  திரும்பு rc;
}

/*
** Set the pBt->nPage field correctly, according to the current
** state of the database.  Assume pBt->pPage1 is valid.
*/
நிலையான வெறுமை btreeSetNPage(BtShared *pBt, MemPage *pPage1){
  முழு nPage = get4byte(&pPage1->aData[28]);
  testcase( nPage==0 );
  எனில்( nPage==0 ) sqlite3PagerPagecount(pBt->pPager, &nPage);
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
முழு sqlite3BtreeRollback(Btree *p, முழு tripCode, முழு writeOnly){
  முழு rc;
  BtShared *pBt = p->pBt;
  MemPage *pPage1;

  ப்ரதிஜ்ஞா( writeOnly==1 || writeOnly==0 );
  ப்ரதிஜ்ஞா( tripCode==SQLITE_ABORT_ROLLBACK || tripCode==SQLITE_OK );
  sqlite3BtreeEnter(p);
  எனில்( tripCode==SQLITE_OK ){
    rc = tripCode = saveAllCursors(pBt, 0, 0);
    எனில்( rc ) writeOnly = 0;
  }இல்லையெனில்{
    rc = SQLITE_OK;
  }
  எனில்( tripCode ){
    முழு rc2 = sqlite3BtreeTripAllCursors(p, tripCode, writeOnly);
    ப்ரதிஜ்ஞா( rc==SQLITE_OK || (writeOnly==0 && rc2==SQLITE_OK) );
    எனில்( rc2!=SQLITE_OK ) rc = rc2;
  }
  btreeIntegrity(p);

  எனில்( p->inTrans==TRANS_WRITE ){
    முழு rc2;

    ப்ரதிஜ்ஞா( TRANS_WRITE==pBt->inTransaction );
    rc2 = sqlite3PagerRollback(pBt->pPager);
    எனில்( rc2!=SQLITE_OK ){
      rc = rc2;
    }

    /* The rollback may have destroyed the pPage1->aData value.  So
    ** call btreeGetPage() on page 1 again to make
    ** sure pPage1->aData is set correctly. */
    எனில்( btreeGetPage(pBt, 1, &pPage1, 0)==SQLITE_OK ){
      btreeSetNPage(pBt, pPage1);
      releasePageOne(pPage1);
    }
    ப்ரதிஜ்ஞா( மொத்தம்_Valid_Cursors(pBt, 1)==0 );
    pBt->inTransaction = TRANS_READ;
    btreeClearHasContent(pBt);
  }

  btreeEndTransaction(p);
  sqlite3BtreeLeave(p);
  திரும்பு rc;
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
முழு sqlite3BtreeBeginStmt(Btree *p, முழு iStatement){
  முழு rc;
  BtShared *pBt = p->pBt;
  sqlite3BtreeEnter(p);
  ப்ரதிஜ்ஞா( p->inTrans==TRANS_WRITE );
  ப்ரதிஜ்ஞா( (pBt->btsFlags & BTS_READ_ONLY)==0 );
  ப்ரதிஜ்ஞா( iStatement>0 );
  ப்ரதிஜ்ஞா( iStatement>p->db->nSavepoint );
  ப்ரதிஜ்ஞா( pBt->inTransaction==TRANS_WRITE );
  /* At the pager level, a statement transaction is a savepoint with
  ** an index greater than all savepoints created explicitly using
  ** SQL statements. It is illegal to open, release or rollback any
  ** such savepoints while the statement transaction savepoint is active.
  */
  rc = sqlite3PagerOpenSavepoint(pBt->pPager, iStatement);
  sqlite3BtreeLeave(p);
  திரும்பு rc;
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
முழு sqlite3BtreeSavepoint(Btree *p, முழு op, முழு iSavepoint){
  முழு rc = SQLITE_OK;
  எனில்( p && p->inTrans==TRANS_WRITE ){
    BtShared *pBt = p->pBt;
    ப்ரதிஜ்ஞா( op==SAVEPOINT_RELEASE || op==SAVEPOINT_ROLLBACK );
    ப்ரதிஜ்ஞா( iSavepoint>=0 || (iSavepoint==-1 && op==SAVEPOINT_ROLLBACK) );
    sqlite3BtreeEnter(p);
    எனில்( op==SAVEPOINT_ROLLBACK ){
      rc = saveAllCursors(pBt, 0, 0);
    }
    எனில்( rc==SQLITE_OK ){
      rc = sqlite3PagerSavepoint(pBt->pPager, op, iSavepoint);
    }
    எனில்( rc==SQLITE_OK ){
      எனில்( iSavepoint<0 && (pBt->btsFlags & BTS_INITIALLY_EMPTY)!=0 ){
        pBt->nPage = 0;
      }
      rc = newDatabase(pBt);
      btreeSetNPage(pBt, pBt->pPage1);

      /* pBt->nPage might be zero if the database was corrupt when
      ** the transaction was started. Otherwise, it must be at least 1.  */
      ப்ரதிஜ்ஞா( CORRUPT_DB || pBt->nPage>0 );
    }
    sqlite3BtreeLeave(p);
  }
  திரும்பு rc;
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
நிலையான முழு btreeCursor(
  Btree *p,                              /* The btree */
  Pgno iTable,                           /* Root page of table to open */
  முழு wrFlag,                            /* 1 to write. 0 read-only */
  கட்டமைப்பு சாவி_Info *p_சாவி_Info,              /* First arg to comparison function */
  BtCursor *pCur                         /* Space for new cursor */
){
  BtShared *pBt = p->pBt;                /* Shared b-tree handle */
  BtCursor *pX;                          /* Looping over other all cursors */

  ப்ரதிஜ்ஞா( sqlite3BtreeHoldsMutex(p) );
  ப்ரதிஜ்ஞா( wrFlag==0
       || wrFlag==BTREE_WRCSR
       || wrFlag==(BTREE_WRCSR|BTREE_FORDELETE)
  );

  /* The following assert statements verify that if this is a sharable
  ** b-tree database, the connection is holding the required table locks,
  ** and that no other connection has any open cursor that conflicts with
  ** this lock.  The iTable<1 term disables the check for corrupt schemas. */
  ப்ரதிஜ்ஞா( hasSharedCacheTableLock(p, iTable, p_சாவி_Info!=0, (wrFlag?2:1))
          || iTable<1 );
  ப்ரதிஜ்ஞா( wrFlag==0 || !hasReadConflicts(p, iTable) );

  /* Assert that the caller has opened the required transaction. */
  ப்ரதிஜ்ஞா( p->inTrans>TRANS_NONE );
  ப்ரதிஜ்ஞா( wrFlag==0 || p->inTrans==TRANS_WRITE );
  ப்ரதிஜ்ஞா( pBt->pPage1 && pBt->pPage1->aData );
  ப்ரதிஜ்ஞா( wrFlag==0 || (pBt->btsFlags & BTS_READ_ONLY)==0 );

  எனில்( iTable<=1 ){
    எனில்( iTable<1 ){
      திரும்பு SQLITE_CORRUPT_BKPT;
    }இல்லையெனில் எனில்( btreePagecount(pBt)==0 ){
      ப்ரதிஜ்ஞா( wrFlag==0 );
      iTable = 0;
    }
  }

  /* Now that no other errors can occur, finish filling in the BtCursor
  ** variables and link the cursor into the BtShared list.  */
  pCur->pgno_வேர் = iTable;
  pCur->iPage = -1;
  pCur->p_சாவி_Info = p_சாவி_Info;
  pCur->pBtree = p;
  pCur->pBt = pBt;
  pCur->curFlags = 0;
  /* If there are two or more cursors on the same btree, then all such
  ** cursors *must* have the BTCF_Multiple flag set. */
  க்கு(pX=pBt->pCursor; pX; pX=pX->pNext){
    எனில்( pX->pgno_வேர்==iTable ){
      pX->curFlags |= BTCF_Multiple;
      pCur->curFlags = BTCF_Multiple;
    }
  }
  pCur->eState = CURSOR_INVALID;
  pCur->pNext = pBt->pCursor;
  pBt->pCursor = pCur;
  எனில்( wrFlag ){
    pCur->curFlags |= BTCF_WriteFlag;
    pCur->curPagerFlags = 0;
    எனில்( pBt->pTmpSpace==0 ) திரும்பு allocateTempSpace(pBt);
  }இல்லையெனில்{
    pCur->curPagerFlags = PAGER_GET_READONLY;
  }
  திரும்பு SQLITE_OK;
}
நிலையான முழு btreeCursorWithLock(
  Btree *p,                              /* The btree */
  Pgno iTable,                           /* Root page of table to open */
  முழு wrFlag,                            /* 1 to write. 0 read-only */
  கட்டமைப்பு சாவி_Info *p_சாவி_Info,              /* First arg to comparison function */
  BtCursor *pCur                         /* Space for new cursor */
){
  முழு rc;
  sqlite3BtreeEnter(p);
  rc = btreeCursor(p, iTable, wrFlag, p_சாவி_Info, pCur);
  sqlite3BtreeLeave(p);
  திரும்பு rc;
}
முழு sqlite3BtreeCursor(
  Btree *p,                                   /* The btree */
  Pgno iTable,                                /* Root page of table to open */
  முழு wrFlag,                                 /* 1 to write. 0 read-only */
  கட்டமைப்பு சாவி_Info *p_சாவி_Info,                   /* First arg to xCompare() */
  BtCursor *pCur                              /* Write new cursor here */
){
  எனில்( p->sharable ){
    திரும்பு btreeCursorWithLock(p, iTable, wrFlag, p_சாவி_Info, pCur);
  }இல்லையெனில்{
    திரும்பு btreeCursor(p, iTable, wrFlag, p_சாவி_Info, pCur);
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
முழு sqlite3BtreeCursorSize(வெறுமை){
  திரும்பு ROUND8(அளவு(BtCursor));
}

#ifdef SQLITE_DEBUG
/*
** Return true if and only if the Btree object will be automatically
** closed with the BtCursor closes.  This is used within assert() statements
** only.
*/
முழு sqlite3BtreeClosesWithCursor(
  Btree *pBtree,       /* the btree object */
  BtCursor *pCur       /* Corresponding cursor */
){
  BtShared *pBt = pBtree->pBt;
  எனில்( (pBt->openFlags & BTREE_SINGLE)==0 ) திரும்பு 0;
  எனில்( pBt->pCursor!=pCur ) திரும்பு 0;
  எனில்( pCur->pNext!=0 ) திரும்பு 0;
  எனில்( pCur->pBtree!=pBtree ) திரும்பு 0;
  திரும்பு 1;
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
வெறுமை sqlite3BtreeCursorZero(BtCursor *p){
  memset(p, 0, offsetof(BtCursor, BTCURSOR_FIRST_UNINIT));
}

/*
** Close a cursor.  The read lock on the database file is released
** when the last cursor is closed.
*/
முழு sqlite3BtreeCloseCursor(BtCursor *pCur){
  Btree *pBtree = pCur->pBtree;
  எனில்( pBtree ){
    BtShared *pBt = pCur->pBt;
    sqlite3BtreeEnter(pBtree);
    ப்ரதிஜ்ஞா( pBt->pCursor!=0 );
    எனில்( pBt->pCursor==pCur ){
      pBt->pCursor = pCur->pNext;
    }இல்லையெனில்{
      BtCursor *pPrev = pBt->pCursor;
      செய்{
        எனில்( pPrev->pNext==pCur ){
          pPrev->pNext = pCur->pNext;
          நிறுத்து;
        }
        pPrev = pPrev->pNext;
      }போது( ALWAYS(pPrev) );
    }
    btreeReleaseAllCursorPages(pCur);
    unlockBtreeIfUnused(pBt);
    sqlite3_free(pCur->aOverflow);
    sqlite3_free(pCur->p_சாவி);
    எனில்( (pBt->openFlags & BTREE_SINGLE) && pBt->pCursor==0 ){
      /* Since the BtShared is not sharable, there is no need to
      ** worry about the missing sqlite3BtreeLeave() call here.  */
      ப்ரதிஜ்ஞா( pBtree->sharable==0 );
      sqlite3BtreeClose(pBtree);
    }இல்லையெனில்{
      sqlite3BtreeLeave(pBtree);
    }
    pCur->pBtree = 0;
  }
  திரும்பு SQLITE_OK;
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
  நிலையான முழு cellInfoEqual(CellInfo *a, CellInfo *b){
    எனில்( a->n_சாவி!=b->n_சாவி ) திரும்பு 0;
    எனில்( a->pPayload!=b->pPayload ) திரும்பு 0;
    எனில்( a->nPayload!=b->nPayload ) திரும்பு 0;
    எனில்( a->nLocal!=b->nLocal ) திரும்பு 0;
    எனில்( a->nSize!=b->nSize ) திரும்பு 0;
    திரும்பு 1;
  }
  நிலையான வெறுமை assertCellInfo(BtCursor *pCur){
    CellInfo info;
    memset(&info, 0, அளவு(info));
    btreeParseCell(pCur->pPage, pCur->ix, &info);
    ப்ரதிஜ்ஞா( CORRUPT_DB || cellInfoEqual(&info, &pCur->info) );
  }
#இல்லையெனில்
  #define assertCellInfo(x)
#endif
நிலையான SQLITE_NOINLINE வெறுமை getCellInfo(BtCursor *pCur){
  எனில்( pCur->info.nSize==0 ){
    pCur->curFlags |= BTCF_ValidNKey;
    btreeParseCell(pCur->pPage,pCur->ix,&pCur->info);
  }இல்லையெனில்{
    assertCellInfo(pCur);
  }
}

#ifndef NDEBUG  /* The next routine used only within assert() statements */
/*
** Return true if the given BtCursor is valid.  A valid cursor is one
** that is currently pointing to a row in a (non-empty) table.
** This is a verification routine is used only within assert() statements.
*/
முழு sqlite3BtreeCursorIsValid(BtCursor *pCur){
  திரும்பு pCur && pCur->eState==CURSOR_VALID;
}
#endif /* NDEBUG */
முழு sqlite3BtreeCursorIsValidNN(BtCursor *pCur){
  ப்ரதிஜ்ஞா( pCur!=0 );
  திரும்பு pCur->eState==CURSOR_VALID;
}

/*
** Return the value of the integer key or "rowid" for a table btree.
** This routine is only valid for a cursor that is pointing into a
** ordinary table btree.  If the cursor points to an index btree or
** is invalid, the result of this routine is undefined.
*/
i64 sqlite3_Btree_Integer_சாவி(BtCursor *pCur){
  ப்ரதிஜ்ஞா( cursorHoldsMutex(pCur) );
  ப்ரதிஜ்ஞா( pCur->eState==CURSOR_VALID );
  ப்ரதிஜ்ஞா( pCur->cur_Int_சாவி );
  getCellInfo(pCur);
  திரும்பு pCur->info.n_சாவி;
}

/*
** Pin or unpin a cursor.
*/
வெறுமை sqlite3BtreeCursorPin(BtCursor *pCur){
  ப்ரதிஜ்ஞா( (pCur->curFlags & BTCF_Pinned)==0 );
  pCur->curFlags |= BTCF_Pinned;
}
வெறுமை sqlite3BtreeCursorUnpin(BtCursor *pCur){
  ப்ரதிஜ்ஞா( (pCur->curFlags & BTCF_Pinned)!=0 );
  pCur->curFlags &= ~BTCF_Pinned;
}

/*
** Return the offset into the database file for the start of the
** payload to which the cursor is pointing.
*/
i64 sqlite3BtreeOffset(BtCursor *pCur){
  ப்ரதிஜ்ஞா( cursorHoldsMutex(pCur) );
  ப்ரதிஜ்ஞா( pCur->eState==CURSOR_VALID );
  getCellInfo(pCur);
  திரும்பு (i64)pCur->pBt->pageSize*((i64)pCur->pPage->pgno - 1) +
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
  ப்ரதிஜ்ஞா( cursorHoldsMutex(pCur) );
  ப்ரதிஜ்ஞா( pCur->eState==CURSOR_VALID );
  getCellInfo(pCur);
  திரும்பு pCur->info.nPayload;
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
  ப்ரதிஜ்ஞா( cursorHoldsMutex(pCur) );
  ப்ரதிஜ்ஞா( pCur->eState==CURSOR_VALID );
  திரும்பு pCur->pBt->pageSize * (sqlite3_int64)pCur->pBt->nPage;
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
நிலையான முழு getOverflowPage(
  BtShared *pBt,               /* The database file */
  Pgno ovfl,                   /* Current overflow page number */
  MemPage **ppPage,            /* OUT: MemPage handle (may be NULL) */
  Pgno *pPgnoNext              /* OUT: Next overflow page number */
){
  Pgno next = 0;
  MemPage *pPage = 0;
  முழு rc = SQLITE_OK;

  ப்ரதிஜ்ஞா( sqlite3_mutex_held(pBt->mutex) );
  ப்ரதிஜ்ஞா(pPgnoNext);

#ifndef SQLITE_OMIT_AUTOVACUUM
  /* Try to find the next page in the overflow list using the
  ** autovacuum pointer-map pages. Guess that the next page in
  ** the overflow list is page number (ovfl+1). If that guess turns
  ** out to be wrong, fall back to loading the data of page
  ** number ovfl to determine the next page number.
  */
  எனில்( pBt->autoVacuum ){
    Pgno pgno;
    Pgno iGuess = ovfl+1;
    u8 eType;

    போது( PTRMAP_ISPAGE(pBt, iGuess) || iGuess==PENDING_BYTE_PAGE(pBt) ){
      iGuess++;
    }

    எனில்( iGuess<=btreePagecount(pBt) ){
      rc = ptrmapGet(pBt, iGuess, &eType, &pgno);
      எனில்( rc==SQLITE_OK && eType==PTRMAP_OVERFLOW2 && pgno==ovfl ){
        next = iGuess;
        rc = SQLITE_DONE;
      }
    }
  }
#endif

  ப்ரதிஜ்ஞா( next==0 || rc==SQLITE_DONE );
  எனில்( rc==SQLITE_OK ){
    rc = btreeGetPage(pBt, ovfl, &pPage, (ppPage==0) ? PAGER_GET_READONLY : 0);
    ப்ரதிஜ்ஞா( rc==SQLITE_OK || pPage==0 );
    எனில்( rc==SQLITE_OK ){
      next = get4byte(pPage->aData);
    }
  }

  *pPgnoNext = next;
  எனில்( ppPage ){
    *ppPage = pPage;
  }இல்லையெனில்{
    releasePage(pPage);
  }
  திரும்பு (rc==SQLITE_DONE ? SQLITE_OK : rc);
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
நிலையான முழு copyPayload(
  வெறுமை *pPayload,           /* Pointer to page data */
  வெறுமை *pBuf,               /* Pointer to buffer */
  முழு nByte,                /* Number of bytes to copy */
  முழு eOp,                  /* 0 -> copy from page, 1 -> copy to page */
  DbPage *pDbPage           /* Page containing pPayload */
){
  எனில்( eOp ){
    /* Copy data from buffer to page (a write operation) */
    முழு rc = sqlite3PagerWrite(pDbPage);
    எனில்( rc!=SQLITE_OK ){
      திரும்பு rc;
    }
    memcpy(pPayload, pBuf, nByte);
  }இல்லையெனில்{
    /* Copy data from page to buffer (a read operation) */
    memcpy(pBuf, pPayload, nByte);
  }
  திரும்பு SQLITE_OK;
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
நிலையான முழு accessPayload(
  BtCursor *pCur,      /* Cursor pointing to entry to read from */
  u32 offset,          /* Begin reading this far into payload */
  u32 amt,             /* Read this many bytes */
  unsigned எழுத்து *pBuf, /* Write the bytes into this buffer */
  முழு eOp              /* zero to read. non-zero to write. */
){
  unsigned எழுத்து *aPayload;
  முழு rc = SQLITE_OK;
  முழு iIdx = 0;
  MemPage *pPage = pCur->pPage;               /* Btree page of current entry */
  BtShared *pBt = pCur->pBt;                  /* Btree this cursor belongs to */
#ifdef SQLITE_DIRECT_OVERFLOW_READ
  unsigned எழுத்து * மாறிலி pBufStart = pBuf;     /* Start of original out buffer */
#endif

  ப்ரதிஜ்ஞா( pPage );
  ப்ரதிஜ்ஞா( eOp==0 || eOp==1 );
  ப்ரதிஜ்ஞா( pCur->eState==CURSOR_VALID );
  எனில்( pCur->ix>=pPage->nCell ){
    திரும்பு SQLITE_CORRUPT_PAGE(pPage);
  }
  ப்ரதிஜ்ஞா( cursorHoldsMutex(pCur) );

  getCellInfo(pCur);
  aPayload = pCur->info.pPayload;
  ப்ரதிஜ்ஞா( (u64)offset+(u64)amt <= (u64)pCur->info.nPayload );

  ப்ரதிஜ்ஞா( aPayload > pPage->aData );
  எனில்( (uptr)(aPayload - pPage->aData) > (pBt->usableSize - pCur->info.nLocal) ){
    /* Trying to read or write past the end of the data is an error.  The
    ** conditional above is really:
    **    &aPayload[pCur->info.nLocal] > &pPage->aData[pBt->usableSize]
    ** but is recast into its current form to avoid integer overflow problems
    */
    திரும்பு SQLITE_CORRUPT_PAGE(pPage);
  }

  /* Check if data must be read/written to/from the btree page itself. */
  எனில்( offset<pCur->info.nLocal ){
    முழு a = amt;
    எனில்( a+offset>pCur->info.nLocal ){
      a = pCur->info.nLocal - offset;
    }
    rc = copyPayload(&aPayload[offset], pBuf, a, eOp, pPage->pDbPage);
    offset = 0;
    pBuf += a;
    amt -= a;
  }இல்லையெனில்{
    offset -= pCur->info.nLocal;
  }


  எனில்( rc==SQLITE_OK && amt>0 ){
    மாறிலி u32 ovflSize = pBt->usableSize - 4;  /* Bytes content per ovfl page */
    Pgno nextPage;

    nextPage = get4byte(&aPayload[pCur->info.nLocal]);

    /* If the BtCursor.aOverflow[] has not been allocated, allocate it now.
    **
    ** The aOverflow[] array is sized at one entry for each overflow page
    ** in the overflow chain. The page number of the first overflow page is
    ** stored in aOverflow[0], etc. A value of 0 in the aOverflow[] array
    ** means "not yet known" (the cache is lazily populated).
    */
    எனில்( (pCur->curFlags & BTCF_ValidOvfl)==0 ){
      i64 nOvfl = pCur->info.nPayload;
      testcase( nOvfl - pCur->info.nLocal + ovflSize - 1 > 0xffffffffU );
      nOvfl = (nOvfl - pCur->info.nLocal + ovflSize-1)/ovflSize;
      எனில்( pCur->aOverflow==0
       || nOvfl*(முழு)அளவு(Pgno) > sqlite3MallocSize(pCur->aOverflow)
      ){
        Pgno *aNew;
        எனில்( sqlite3FaultSim(413) ){
          aNew = 0;
        }இல்லையெனில்{
          aNew = (Pgno*)sqlite3Realloc(pCur->aOverflow, nOvfl*2*அளவு(Pgno));
        }
        எனில்( aNew==0 ){
          திரும்பு SQLITE_NOMEM_BKPT;
        }இல்லையெனில்{
          pCur->aOverflow = aNew;
        }
      }
      memset(pCur->aOverflow, 0, nOvfl*அளவு(Pgno));
      pCur->curFlags |= BTCF_ValidOvfl;
    }இல்லையெனில்{
      /* Sanity check the validity of the overflow page cache */
      ப்ரதிஜ்ஞா( pCur->aOverflow[0]==nextPage
           || pCur->aOverflow[0]==0
           || CORRUPT_DB );
      ப்ரதிஜ்ஞா( pCur->aOverflow[0]!=0 || pCur->aOverflow[offset/ovflSize]==0 );

      /* If the overflow page-list cache has been allocated and the
      ** entry for the first required overflow page is valid, skip
      ** directly to it.
      */
      எனில்( pCur->aOverflow[offset/ovflSize] ){
        iIdx = (offset/ovflSize);
        nextPage = pCur->aOverflow[iIdx];
        offset = (offset%ovflSize);
      }
    }

    ப்ரதிஜ்ஞா( rc==SQLITE_OK && amt>0 );
    போது( nextPage ){
      /* If required, populate the overflow page-list cache. */
      எனில்( nextPage > pBt->nPage ) திரும்பு SQLITE_CORRUPT_BKPT;
      ப்ரதிஜ்ஞா( pCur->aOverflow[iIdx]==0
              || pCur->aOverflow[iIdx]==nextPage
              || CORRUPT_DB );
      pCur->aOverflow[iIdx] = nextPage;

      எனில்( offset>=ovflSize ){
        /* The only reason to read this page is to obtain the page
        ** number for the next page in the overflow chain. The page
        ** data is not required. So first try to lookup the overflow
        ** page-list cache, if any, then fall back to the getOverflowPage()
        ** function.
        */
        ப்ரதிஜ்ஞா( pCur->curFlags & BTCF_ValidOvfl );
        ப்ரதிஜ்ஞா( pCur->pBtree->db==pBt->db );
        எனில்( pCur->aOverflow[iIdx+1] ){
          nextPage = pCur->aOverflow[iIdx+1];
        }இல்லையெனில்{
          rc = getOverflowPage(pBt, nextPage, 0, &nextPage);
        }
        offset -= ovflSize;
      }இல்லையெனில்{
        /* Need to read this page properly. It contains some of the
        ** range of data that is being read (eOp==0) or written (eOp!=0).
        */
        முழு a = amt;
        எனில்( a + offset > ovflSize ){
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
        எனில்( eOp==0                                             /* (1) */
         && offset==0                                          /* (2) */
         && sqlite3PagerDirectReadOk(pBt->pPager, nextPage)    /* (3,4,5) */
         && &pBuf[-4]>=pBufStart                               /* (6) */
        ){
          sqlite3_file *fd = sqlite3PagerFile(pBt->pPager);
          u8 aSave[4];
          u8 *aWrite = &pBuf[-4];
          ப்ரதிஜ்ஞா( aWrite>=pBufStart );                         /* due to (6) */
          memcpy(aSave, aWrite, 4);
          rc = sqlite3OsRead(fd, aWrite, a+4, (i64)pBt->pageSize*(nextPage-1));
          nextPage = get4byte(aWrite);
          memcpy(aWrite, aSave, 4);
        }இல்லையெனில்
#endif

        {
          DbPage *pDbPage;
          rc = sqlite3PagerGet(pBt->pPager, nextPage, &pDbPage,
              (eOp==0 ? PAGER_GET_READONLY : 0)
          );
          எனில்( rc==SQLITE_OK ){
            எனில்( eOp!=0
             && (sqlite3PagerPageRefcount(pDbPage)!=1
                 || NEVER(((MemPage*)sqlite3PagerGetExtra(pDbPage))->isInit))
             && sqlite3FaultSim(411)==SQLITE_OK
            ){
              sqlite3PagerUnref(pDbPage);
              திரும்பு SQLITE_CORRUPT_PAGE(pPage);
            }
            aPayload = sqlite3PagerGetData(pDbPage);
            nextPage = get4byte(aPayload);
            rc = copyPayload(&aPayload[offset+4], pBuf, a, eOp, pDbPage);
            sqlite3PagerUnref(pDbPage);
            offset = 0;
          }
        }
        amt -= a;
        எனில்( amt==0 ) திரும்பு rc;
        pBuf += a;
      }
      எனில்( rc ) நிறுத்து;
      iIdx++;
    }
  }

  எனில்( rc==SQLITE_OK && amt>0 ){
    /* Overflow chain ends prematurely */
    திரும்பு SQLITE_CORRUPT_PAGE(pPage);
  }
  திரும்பு rc;
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
முழு sqlite3BtreePayload(BtCursor *pCur, u32 offset, u32 amt, வெறுமை *pBuf){
  ப்ரதிஜ்ஞா( cursorHoldsMutex(pCur) );
  ப்ரதிஜ்ஞா( pCur->eState==CURSOR_VALID );
  ப்ரதிஜ்ஞா( pCur->iPage>=0 && pCur->pPage );
  திரும்பு accessPayload(pCur, offset, amt, (unsigned எழுத்து*)pBuf, 0);
}

/*
** This variant of sqlite3BtreePayload() works even if the cursor has not
** in the CURSOR_VALID state.  It is only used by the sqlite3_blob_read()
** interface.
*/
#ifndef SQLITE_OMIT_INCRBLOB
நிலையான SQLITE_NOINLINE முழு accessPayloadChecked(
  BtCursor *pCur,
  u32 offset,
  u32 amt,
  வெறுமை *pBuf
){
  முழு rc;
  எனில் ( pCur->eState==CURSOR_INVALID ){
    திரும்பு SQLITE_ABORT;
  }
  ப்ரதிஜ்ஞா( cursorOwnsBtShared(pCur) );
  rc = btreeRestoreCursorPosition(pCur);
  திரும்பு rc ? rc : accessPayload(pCur, offset, amt, pBuf, 0);
}
முழு sqlite3BtreePayloadChecked(BtCursor *pCur, u32 offset, u32 amt, வெறுமை *pBuf){
  எனில்( pCur->eState==CURSOR_VALID ){
    ப்ரதிஜ்ஞா( cursorOwnsBtShared(pCur) );
    திரும்பு accessPayload(pCur, offset, amt, pBuf, 0);
  }இல்லையெனில்{
    திரும்பு accessPayloadChecked(pCur, offset, amt, pBuf);
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
நிலையான மாறிலி வெறுமை *fetchPayload(
  BtCursor *pCur,      /* Cursor pointing to entry to read from */
  u32 *pAmt            /* Write the number of available bytes here */
){
  முழு amt;
  ப்ரதிஜ்ஞா( pCur!=0 && pCur->iPage>=0 && pCur->pPage);
  ப்ரதிஜ்ஞா( pCur->eState==CURSOR_VALID );
  ப்ரதிஜ்ஞா( sqlite3_mutex_held(pCur->pBtree->db->mutex) );
  ப்ரதிஜ்ஞா( cursorOwnsBtShared(pCur) );
  ப்ரதிஜ்ஞா( pCur->ix<pCur->pPage->nCell || CORRUPT_DB );
  ப்ரதிஜ்ஞா( pCur->info.nSize>0 );
  ப்ரதிஜ்ஞா( pCur->info.pPayload>pCur->pPage->aData || CORRUPT_DB );
  ப்ரதிஜ்ஞா( pCur->info.pPayload<pCur->pPage->aDataEnd ||CORRUPT_DB);
  amt = pCur->info.nLocal;
  எனில்( amt>(முழு)(pCur->pPage->aDataEnd - pCur->info.pPayload) ){
    /* There is too little space on the page for the expected amount
    ** of local content. Database must be corrupt. */
    ப்ரதிஜ்ஞா( CORRUPT_DB );
    amt = MAX(0, (முழு)(pCur->pPage->aDataEnd - pCur->info.pPayload));
  }
  *pAmt = (u32)amt;
  திரும்பு (வெறுமை*)pCur->info.pPayload;
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
மாறிலி வெறுமை *sqlite3BtreePayloadFetch(BtCursor *pCur, u32 *pAmt){
  திரும்பு fetchPayload(pCur, pAmt);
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
நிலையான முழு moveToChild(BtCursor *pCur, u32 newPgno){
  முழு rc;
  ப்ரதிஜ்ஞா( cursorOwnsBtShared(pCur) );
  ப்ரதிஜ்ஞா( pCur->eState==CURSOR_VALID );
  ப்ரதிஜ்ஞா( pCur->iPage<BTCURSOR_MAX_DEPTH );
  ப்ரதிஜ்ஞா( pCur->iPage>=0 );
  எனில்( pCur->iPage>=(BTCURSOR_MAX_DEPTH-1) ){
    திரும்பு SQLITE_CORRUPT_BKPT;
  }
  pCur->info.nSize = 0;
  pCur->curFlags &= ~(BTCF_ValidNKey|BTCF_ValidOvfl);
  pCur->aiIdx[pCur->iPage] = pCur->ix;
  pCur->apPage[pCur->iPage] = pCur->pPage;
  pCur->ix = 0;
  pCur->iPage++;
  rc = getAndInitPage(pCur->pBt, newPgno, &pCur->pPage, pCur->curPagerFlags);
  ப்ரதிஜ்ஞா( pCur->pPage!=0 || rc!=SQLITE_OK );
  எனில்( rc==SQLITE_OK
   && (pCur->pPage->nCell<1 || pCur->pPage->int_சாவி!=pCur->cur_Int_சாவி)
  ){
    releasePage(pCur->pPage);
    rc = SQLITE_CORRUPT_PGNO(newPgno);
  }
  எனில்( rc ){
    pCur->pPage = pCur->apPage[--pCur->iPage];
  }
  திரும்பு rc;
}

#ifdef SQLITE_DEBUG
/*
** Page pParent is an internal (non-leaf) tree page. This function
** asserts that page number iChild is the left-child if the iIdx'th
** cell in page pParent. Or, if iIdx is equal to the total number of
** cells in pParent, that page number iChild is the right-child of
** the page.
*/
நிலையான வெறுமை assertParentIndex(MemPage *pParent, முழு iIdx, Pgno iChild){
  எனில்( CORRUPT_DB ) திரும்பு;  /* The conditions tested below might not be true
                            ** in a corrupt database */
  ப்ரதிஜ்ஞா( iIdx<=pParent->nCell );
  எனில்( iIdx==pParent->nCell ){
    ப்ரதிஜ்ஞா( get4byte(&pParent->aData[pParent->hdrOffset+8])==iChild );
  }இல்லையெனில்{
    ப்ரதிஜ்ஞா( get4byte(findCell(pParent, iIdx))==iChild );
  }
}
#இல்லையெனில்
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
நிலையான வெறுமை moveToParent(BtCursor *pCur){
  MemPage *pLeaf;
  ப்ரதிஜ்ஞா( cursorOwnsBtShared(pCur) );
  ப்ரதிஜ்ஞா( pCur->eState==CURSOR_VALID );
  ப்ரதிஜ்ஞா( pCur->iPage>0 );
  ப்ரதிஜ்ஞா( pCur->pPage );
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
நிலையான முழு move_To_வேர்(BtCursor *pCur){
  MemPage *p_வேர்;
  முழு rc = SQLITE_OK;

  ப்ரதிஜ்ஞா( cursorOwnsBtShared(pCur) );
  ப்ரதிஜ்ஞா( CURSOR_INVALID < CURSOR_REQUIRESEEK );
  ப்ரதிஜ்ஞா( CURSOR_VALID   < CURSOR_REQUIRESEEK );
  ப்ரதிஜ்ஞா( CURSOR_FAULT   > CURSOR_REQUIRESEEK );
  ப்ரதிஜ்ஞா( pCur->eState < CURSOR_REQUIRESEEK || pCur->iPage<0 );
  ப்ரதிஜ்ஞா( pCur->pgno_வேர்>0 || pCur->iPage<0 );

  எனில்( pCur->iPage>=0 ){
    எனில்( pCur->iPage ){
      releasePageNotNull(pCur->pPage);
      போது( --pCur->iPage ){
        releasePageNotNull(pCur->apPage[pCur->iPage]);
      }
      p_வேர் = pCur->pPage = pCur->apPage[0];
      செல் skip_init;
    }
  }இல்லையெனில் எனில்( pCur->pgno_வேர்==0 ){
    pCur->eState = CURSOR_INVALID;
    திரும்பு SQLITE_EMPTY;
  }இல்லையெனில்{
    ப்ரதிஜ்ஞா( pCur->iPage==(-1) );
    எனில்( pCur->eState>=CURSOR_REQUIRESEEK ){
      எனில்( pCur->eState==CURSOR_FAULT ){
        ப்ரதிஜ்ஞா( pCur->skipNext!=SQLITE_OK );
        திரும்பு pCur->skipNext;
      }
      sqlite3BtreeClearCursor(pCur);
    }
    rc = getAndInitPage(pCur->pBt, pCur->pgno_வேர், &pCur->pPage,
                        pCur->curPagerFlags);
    எனில்( rc!=SQLITE_OK ){
      pCur->eState = CURSOR_INVALID;
      திரும்பு rc;
    }
    pCur->iPage = 0;
    pCur->cur_Int_சாவி = pCur->pPage->int_சாவி;
  }
  p_வேர் = pCur->pPage;
  ப்ரதிஜ்ஞா( p_வேர்->pgno==pCur->pgno_வேர் || CORRUPT_DB );

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
  ப்ரதிஜ்ஞா( p_வேர்->int_சாவி==1 || p_வேர்->int_சாவி==0 );
  எனில்( p_வேர்->isInit==0 || (pCur->p_சாவி_Info==0)!=p_வேர்->int_சாவி ){
    திரும்பு SQLITE_CORRUPT_PAGE(pCur->pPage);
  }

skip_init: 
  pCur->ix = 0;
  pCur->info.nSize = 0;
  pCur->curFlags &= ~(BTCF_AtLast|BTCF_ValidNKey|BTCF_ValidOvfl);

  எனில்( p_வேர்->nCell>0 ){
    pCur->eState = CURSOR_VALID;
  }இல்லையெனில் எனில்( !p_வேர்->leaf ){
    Pgno subpage;
    எனில்( p_வேர்->pgno!=1 ) திரும்பு SQLITE_CORRUPT_BKPT;
    subpage = get4byte(&p_வேர்->aData[p_வேர்->hdrOffset+8]);
    pCur->eState = CURSOR_VALID;
    rc = moveToChild(pCur, subpage);
  }இல்லையெனில்{
    pCur->eState = CURSOR_INVALID;
    rc = SQLITE_EMPTY;
  }
  திரும்பு rc;
}

/*
** Move the cursor down to the left-most leaf entry beneath the
** entry to which it is currently pointing.
**
** The left-most leaf is the one with the smallest key - the first
** in ascending order.
*/
நிலையான முழு moveToLeftmost(BtCursor *pCur){
  Pgno pgno;
  முழு rc = SQLITE_OK;
  MemPage *pPage;

  ப்ரதிஜ்ஞா( cursorOwnsBtShared(pCur) );
  ப்ரதிஜ்ஞா( pCur->eState==CURSOR_VALID );
  போது( rc==SQLITE_OK && !(pPage = pCur->pPage)->leaf ){
    ப்ரதிஜ்ஞா( pCur->ix<pPage->nCell );
    pgno = get4byte(findCell(pPage, pCur->ix));
    rc = moveToChild(pCur, pgno);
  }
  திரும்பு rc;
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
நிலையான முழு moveToRightmost(BtCursor *pCur){
  Pgno pgno;
  முழு rc = SQLITE_OK;
  MemPage *pPage = 0;

  ப்ரதிஜ்ஞா( cursorOwnsBtShared(pCur) );
  ப்ரதிஜ்ஞா( pCur->eState==CURSOR_VALID );
  போது( !(pPage = pCur->pPage)->leaf ){
    pgno = get4byte(&pPage->aData[pPage->hdrOffset+8]);
    pCur->ix = pPage->nCell;
    rc = moveToChild(pCur, pgno);
    எனில்( rc ) திரும்பு rc;
  }
  pCur->ix = pPage->nCell-1;
  ப்ரதிஜ்ஞா( pCur->info.nSize==0 );
  ப்ரதிஜ்ஞா( (pCur->curFlags & BTCF_ValidNKey)==0 );
  திரும்பு SQLITE_OK;
}

/* Move the cursor to the first entry in the table.  Return SQLITE_OK
** on success.  Set *pRes to 0 if the cursor actually points to something
** or set *pRes to 1 if the table is empty.
*/
முழு sqlite3BtreeFirst(BtCursor *pCur, முழு *pRes){
  முழு rc;

  ப்ரதிஜ்ஞா( cursorOwnsBtShared(pCur) );
  ப்ரதிஜ்ஞா( sqlite3_mutex_held(pCur->pBtree->db->mutex) );
  rc = move_To_வேர்(pCur);
  எனில்( rc==SQLITE_OK ){
    ப்ரதிஜ்ஞா( pCur->pPage->nCell>0 );
    *pRes = 0;
    rc = moveToLeftmost(pCur);
  }இல்லையெனில் எனில்( rc==SQLITE_EMPTY ){
    ப்ரதிஜ்ஞா( pCur->pgno_வேர்==0 || (pCur->pPage!=0 && pCur->pPage->nCell==0) );
    *pRes = 1;
    rc = SQLITE_OK;
  }
  திரும்பு rc;
}

/* Set *pRes to 1 (true) if the BTree pointed to by cursor pCur contains zero
** rows of content.  Set *pRes to 0 (false) if the table contains content.
** Return SQLITE_OK on success or some error code (ex: SQLITE_NOMEM) if
** something goes wrong.
*/
முழு sqlite3BtreeIsEmpty(BtCursor *pCur, முழு *pRes){
  முழு rc;

  ப்ரதிஜ்ஞா( cursorOwnsBtShared(pCur) );
  ப்ரதிஜ்ஞா( sqlite3_mutex_held(pCur->pBtree->db->mutex) );
  எனில்( NEVER(pCur->eState==CURSOR_VALID) ){
    *pRes = 0;
    திரும்பு SQLITE_OK;
  }
  rc = move_To_வேர்(pCur);
  எனில்( rc==SQLITE_EMPTY ){
    *pRes = 1;
    rc = SQLITE_OK;
  }இல்லையெனில்{
    *pRes = 0;
  }
  திரும்பு rc;
}

#ifdef SQLITE_DEBUG
/* The cursors is CURSOR_VALID and has BTCF_AtLast set.  Verify that
** this flags are true for a consistent database.
**
** This routine is is called from within assert() statements only.
** It is an internal verification routine and does not appear in production
** builds.
*/
நிலையான முழு cursorIsAtLastEntry(BtCursor *pCur){
  முழு ii;
  க்கு(ii=0; ii<pCur->iPage; ii++){
    எனில்( pCur->aiIdx[ii]!=pCur->apPage[ii]->nCell ) திரும்பு 0;
  }
  திரும்பு pCur->ix==pCur->pPage->nCell-1 && pCur->pPage->leaf!=0;
}
#endif

/* Move the cursor to the last entry in the table.  Return SQLITE_OK
** on success.  Set *pRes to 0 if the cursor actually points to something
** or set *pRes to 1 if the table is empty.
*/
நிலையான SQLITE_NOINLINE முழு btreeLast(BtCursor *pCur, முழு *pRes){
  முழு rc = move_To_வேர்(pCur);
  எனில்( rc==SQLITE_OK ){
    ப்ரதிஜ்ஞா( pCur->eState==CURSOR_VALID );
    *pRes = 0;
    rc = moveToRightmost(pCur);
    எனில்( rc==SQLITE_OK ){
      pCur->curFlags |= BTCF_AtLast;
    }இல்லையெனில்{
      pCur->curFlags &= ~BTCF_AtLast;
    }
  }இல்லையெனில் எனில்( rc==SQLITE_EMPTY ){
    ப்ரதிஜ்ஞா( pCur->pgno_வேர்==0 || pCur->pPage->nCell==0 );
    *pRes = 1;
    rc = SQLITE_OK;
  }
  திரும்பு rc;
}
முழு sqlite3BtreeLast(BtCursor *pCur, முழு *pRes){
  ப்ரதிஜ்ஞா( cursorOwnsBtShared(pCur) );
  ப்ரதிஜ்ஞா( sqlite3_mutex_held(pCur->pBtree->db->mutex) );

  /* If the cursor already points to the last entry, this is a no-op. */
  எனில்( CURSOR_VALID==pCur->eState && (pCur->curFlags & BTCF_AtLast)!=0 ){
    ப்ரதிஜ்ஞா( cursorIsAtLastEntry(pCur) || CORRUPT_DB );
    *pRes = 0;
    திரும்பு SQLITE_OK;
  }
  திரும்பு btreeLast(pCur, pRes);
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
முழு sqlite3BtreeTableMoveto(
  BtCursor *pCur,          /* The cursor to be moved */
  i64 int_சாவி,              /* The table key */
  முழு bias_வலது,           /* If true, bias the search to the high end */
  முழு *pRes                /* Write search results here */
){
  முழு rc;

  ப்ரதிஜ்ஞா( cursorOwnsBtShared(pCur) );
  ப்ரதிஜ்ஞா( sqlite3_mutex_held(pCur->pBtree->db->mutex) );
  ப்ரதிஜ்ஞா( pRes );
  ப்ரதிஜ்ஞா( pCur->p_சாவி_Info==0 );
  ப்ரதிஜ்ஞா( pCur->eState!=CURSOR_VALID || pCur->cur_Int_சாவி!=0 );

  /* If the cursor is already positioned at the point we are trying
  ** to move to, then just return without doing any work */
  எனில்( pCur->eState==CURSOR_VALID && (pCur->curFlags & BTCF_ValidNKey)!=0 ){
    எனில்( pCur->info.n_சாவி==int_சாவி ){
      *pRes = 0;
      திரும்பு SQLITE_OK;
    }
    எனில்( pCur->info.n_சாவி<int_சாவி ){
      எனில்( (pCur->curFlags & BTCF_AtLast)!=0 ){
        ப்ரதிஜ்ஞா( cursorIsAtLastEntry(pCur) || CORRUPT_DB );
        *pRes = -1;
        திரும்பு SQLITE_OK;
      }
      /* If the requested key is one more than the previous key, then
      ** try to get there using sqlite3BtreeNext() rather than a full
      ** binary search.  This is an optimization only.  The correct answer
      ** is still obtained without this case, only a little more slowly. */
      எனில்( pCur->info.n_சாவி+1==int_சாவி ){
        *pRes = 0;
        rc = sqlite3BtreeNext(pCur, 0);
        எனில்( rc==SQLITE_OK ){
          getCellInfo(pCur);
          எனில்( pCur->info.n_சாவி==int_சாவி ){
            திரும்பு SQLITE_OK;
          }
        }இல்லையெனில் எனில்( rc!=SQLITE_DONE ){
          திரும்பு rc;
        }
      }
    }
  }

#ifdef SQLITE_DEBUG
  pCur->pBtree->nSeek++;   /* Performance measurement during testing */
#endif

  rc = move_To_வேர்(pCur);
  எனில்( rc ){
    எனில்( rc==SQLITE_EMPTY ){
      ப்ரதிஜ்ஞா( pCur->pgno_வேர்==0 || pCur->pPage->nCell==0 );
      *pRes = -1;
      திரும்பு SQLITE_OK;
    }
    திரும்பு rc;
  }
  ப்ரதிஜ்ஞா( pCur->pPage );
  ப்ரதிஜ்ஞா( pCur->pPage->isInit );
  ப்ரதிஜ்ஞா( pCur->eState==CURSOR_VALID );
  ப்ரதிஜ்ஞா( pCur->pPage->nCell > 0 );
  ப்ரதிஜ்ஞா( pCur->iPage==0 || pCur->apPage[0]->int_சாவி==pCur->cur_Int_சாவி );
  ப்ரதிஜ்ஞா( pCur->cur_Int_சாவி );

  க்கு(;;){
    முழு lwr, upr, idx, c;
    Pgno chldPg;
    MemPage *pPage = pCur->pPage;
    u8 *pCell;                          /* Pointer to current cell in pPage */

    /* pPage->nCell must be greater than zero. If this is the root-page
    ** the cursor would have been INVALID above and this for(;;) loop
    ** not run. If this is not the root-page, then the moveToChild() routine
    ** would have already detected db corruption. Similarly, pPage must
    ** be the right kind (index or table) of b-tree page. Otherwise
    ** a moveToChild() or moveToRoot() call would have detected corruption.  */
    ப்ரதிஜ்ஞா( pPage->nCell>0 );
    ப்ரதிஜ்ஞா( pPage->int_சாவி );
    lwr = 0;
    upr = pPage->nCell-1;
    ப்ரதிஜ்ஞா( bias_வலது==0 || bias_வலது==1 );
    idx = upr>>(1-bias_வலது); /* idx = biasRight ? upr : (lwr+upr)/2; */
    க்கு(;;){
      i64 n_Cell_சாவி;
      pCell = findCellPastPtr(pPage, idx);
      எனில்( pPage->int_சாவி_Leaf ){
        போது( 0x80 <= *(pCell++) ){
          எனில்( pCell>=pPage->aDataEnd ){
            திரும்பு SQLITE_CORRUPT_PAGE(pPage);
          }
        }
      }
      n_Cell_சாவி = sqlite3_Varint_மதிப்பு(pCell);
      எனில்( n_Cell_சாவி<int_சாவி ){
        lwr = idx+1;
        எனில்( lwr>upr ){ c = -1; நிறுத்து; }
      }இல்லையெனில் எனில்( n_Cell_சாவி>int_சாவி ){
        upr = idx-1;
        எனில்( lwr>upr ){ c = +1; நிறுத்து; }
      }இல்லையெனில்{
        ப்ரதிஜ்ஞா( n_Cell_சாவி==int_சாவி );
        pCur->ix = (u16)idx;
        எனில்( !pPage->leaf ){
          lwr = idx;
          செல் moveto_table_next_layer;
        }இல்லையெனில்{
          pCur->curFlags |= BTCF_ValidNKey;
          pCur->info.n_சாவி = n_Cell_சாவி;
          pCur->info.nSize = 0;
          *pRes = 0;
          திரும்பு SQLITE_OK;
        }
      }
      ப்ரதிஜ்ஞா( lwr+upr>=0 );
      idx = (lwr+upr)>>1;  /* idx = (lwr+upr)/2; */
    }
    ப்ரதிஜ்ஞா( lwr==upr+1 || !pPage->leaf );
    ப்ரதிஜ்ஞா( pPage->isInit );
    எனில்( pPage->leaf ){
      ப்ரதிஜ்ஞா( pCur->ix<pCur->pPage->nCell );
      pCur->ix = (u16)idx;
      *pRes = c;
      rc = SQLITE_OK;
      செல் moveto_table_finish;
    }
moveto_table_next_layer:
    எனில்( lwr>=pPage->nCell ){
      chldPg = get4byte(&pPage->aData[pPage->hdrOffset+8]);
    }இல்லையெனில்{
      chldPg = get4byte(findCell(pPage, lwr));
    }
    pCur->ix = (u16)lwr;
    rc = moveToChild(pCur, chldPg);
    எனில்( rc ) நிறுத்து;
  }
moveto_table_finish:
  pCur->info.nSize = 0;
  ப்ரதிஜ்ஞா( (pCur->curFlags & BTCF_ValidOvfl)==0 );
  திரும்பு rc;
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
நிலையான முழு indexCellCompare(
  MemPage *pPage,
  முழு idx,
  UnpackedRecord *p_Idx_சாவி,
  RecordCompare xRecordCompare
){
  முழு c;
  முழு nCell;  /* Size of the pCell cell in bytes */
  u8 *pCell = findCellPastPtr(pPage, idx);

  nCell = pCell[0];
  எனில்( nCell<=pPage->max1bytePayload ){
    /* This branch runs if the record-size field of the cell is a
    ** single byte varint and the record fits entirely on the main
    ** b-tree page.  */
    எனில்( pCell + nCell >= pPage->aDataEnd ) திரும்பு 99;
    c = xRecordCompare(nCell, (வெறுமை*)&pCell[1], p_Idx_சாவி);
  }இல்லையெனில் எனில்( !(pCell[1] & 0x80)
    && (nCell = ((nCell&0x7f)<<7) + pCell[1])<=pPage->maxLocal
  ){
    /* The record-size field is a 2 byte varint and the record
    ** fits entirely on the main b-tree page.  */
    எனில்( pCell + nCell >= pPage->aDataEnd ) திரும்பு 99;
    c = xRecordCompare(nCell, (வெறுமை*)&pCell[2], p_Idx_சாவி);
  }இல்லையெனில்{
    /* If the record extends into overflow pages, do not attempt
    ** the optimization. */
    c = 99;
  }
  திரும்பு c;
}

/*
** Return true (non-zero) if pCur is current pointing to the last
** page of a table.
*/
நிலையான முழு cursorOnLastPage(BtCursor *pCur){
  முழு i;
  ப்ரதிஜ்ஞா( pCur->eState==CURSOR_VALID );
  க்கு(i=0; i<pCur->iPage; i++){
    MemPage *pPage = pCur->apPage[i];
    எனில்( pCur->aiIdx[i]<pPage->nCell ) திரும்பு 0;
  }
  திரும்பு 1;
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
முழு sqlite3BtreeIndexMoveto(
  BtCursor *pCur,          /* The cursor to be moved */
  UnpackedRecord *p_Idx_சாவி, /* Unpacked index key */
  முழு *pRes                /* Write search results here */
){
  முழு rc;
  RecordCompare xRecordCompare;

  ப்ரதிஜ்ஞா( cursorOwnsBtShared(pCur) );
  ப்ரதிஜ்ஞா( sqlite3_mutex_held(pCur->pBtree->db->mutex) );
  ப்ரதிஜ்ஞா( pRes );
  ப்ரதிஜ்ஞா( pCur->p_சாவி_Info!=0 );

#ifdef SQLITE_DEBUG
  pCur->pBtree->nSeek++;   /* Performance measurement during testing */
#endif

  xRecordCompare = sqlite3VdbeFindCompare(p_Idx_சாவி);
  p_Idx_சாவி->errCode = 0;
  ப்ரதிஜ்ஞா( p_Idx_சாவி->default_rc==1
       || p_Idx_சாவி->default_rc==0
       || p_Idx_சாவி->default_rc==-1
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
  எனில்( pCur->eState==CURSOR_VALID
   && pCur->pPage->leaf
   && cursorOnLastPage(pCur)
  ){
    முழு c;
    எனில்( pCur->ix==pCur->pPage->nCell-1
     && (c = indexCellCompare(pCur->pPage,pCur->ix,p_Idx_சாவி,xRecordCompare))<=0
     && p_Idx_சாவி->errCode==SQLITE_OK
    ){
      *pRes = c;
      திரும்பு SQLITE_OK;  /* Cursor already pointing at the correct spot */
    }
    எனில்( pCur->iPage>0
     && indexCellCompare(pCur->pPage, 0, p_Idx_சாவி, xRecordCompare)<=0
     && p_Idx_சாவி->errCode==SQLITE_OK
    ){
      pCur->curFlags &= ~(BTCF_ValidOvfl|BTCF_AtLast);
      எனில்( !pCur->pPage->isInit ){
        திரும்பு SQLITE_CORRUPT_BKPT;
      }
      செல் bypass_moveto_வேர்;  /* Start search on the current page */
    }
    p_Idx_சாவி->errCode = SQLITE_OK;
  }

  rc = move_To_வேர்(pCur);
  எனில்( rc ){
    எனில்( rc==SQLITE_EMPTY ){
      ப்ரதிஜ்ஞா( pCur->pgno_வேர்==0 || pCur->pPage->nCell==0 );
      *pRes = -1;
      திரும்பு SQLITE_OK;
    }
    திரும்பு rc;
  }

bypass_moveto_வேர்:
  ப்ரதிஜ்ஞா( pCur->pPage );
  ப்ரதிஜ்ஞா( pCur->pPage->isInit );
  ப்ரதிஜ்ஞா( pCur->eState==CURSOR_VALID );
  ப்ரதிஜ்ஞா( pCur->pPage->nCell > 0 );
  ப்ரதிஜ்ஞா( pCur->cur_Int_சாவி==0 );
  ப்ரதிஜ்ஞா( p_Idx_சாவி!=0 );
  க்கு(;;){
    முழு lwr, upr, idx, c;
    Pgno chldPg;
    MemPage *pPage = pCur->pPage;
    u8 *pCell;                          /* Pointer to current cell in pPage */

    /* pPage->nCell must be greater than zero. If this is the root-page
    ** the cursor would have been INVALID above and this for(;;) loop
    ** not run. If this is not the root-page, then the moveToChild() routine
    ** would have already detected db corruption. Similarly, pPage must
    ** be the right kind (index or table) of b-tree page. Otherwise
    ** a moveToChild() or moveToRoot() call would have detected corruption.  */
    ப்ரதிஜ்ஞா( pPage->nCell>0 );
    ப்ரதிஜ்ஞா( pPage->int_சாவி==0 );
    lwr = 0;
    upr = pPage->nCell-1;
    idx = upr>>1; /* idx = (lwr+upr)/2; */
    க்கு(;;){
      முழு nCell;  /* Size of the pCell cell in bytes */
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
      எனில்( nCell<=pPage->max1bytePayload ){
        /* This branch runs if the record-size field of the cell is a
        ** single byte varint and the record fits entirely on the main
        ** b-tree page.  */
        எனில்( pCell + nCell >= pPage->aDataEnd ){
          rc = SQLITE_CORRUPT_PAGE(pPage);
          செல் moveto_index_finish;
        }
        c = xRecordCompare(nCell, (வெறுமை*)&pCell[1], p_Idx_சாவி);
      }இல்லையெனில் எனில்( !(pCell[1] & 0x80)
        && (nCell = ((nCell&0x7f)<<7) + pCell[1])<=pPage->maxLocal
        && pCell + nCell < pPage->aDataEnd
      ){
        /* The record-size field is a 2 byte varint and the record
        ** fits entirely on the main b-tree page.  */
        c = xRecordCompare(nCell, (வெறுமை*)&pCell[2], p_Idx_சாவி);
      }இல்லையெனில்{
        /* The record flows over onto one or more overflow pages. In
        ** this case the whole cell needs to be parsed, a buffer allocated
        ** and accessPayload() used to retrieve the record into the
        ** buffer before VdbeRecordCompare() can be called.
        **
        ** If the record is corrupt, the xRecordCompare routine may read
        ** up to two varints past the end of the buffer. An extra 18
        ** bytes of padding is allocated at the end of the buffer in
        ** case this happens.  */
        வெறுமை *p_Cell_சாவி;
        u8 * மாறிலி pCellBody = pCell - pPage->childPtrSize;
        மாறிலி முழு nOverrun = 18;  /* Size of the overrun padding */
        pPage->xParseCell(pPage, pCellBody, &pCur->info);
        nCell = (முழு)pCur->info.n_சாவி;
        testcase( nCell<0 );   /* True if key size is 2^32 or more */
        testcase( nCell==0 );  /* Invalid key size:  0x80 0x80 0x00 */
        testcase( nCell==1 );  /* Invalid key size:  0x80 0x80 0x01 */
        testcase( nCell==2 );  /* Minimum legal index key size */
        எனில்( nCell<2 || nCell/pCur->pBt->usableSize>pCur->pBt->nPage ){
          rc = SQLITE_CORRUPT_PAGE(pPage);
          செல் moveto_index_finish;
        }
        p_Cell_சாவி = sqlite3Malloc( (u64)nCell+(u64)nOverrun );
        எனில்( p_Cell_சாவி==0 ){
          rc = SQLITE_NOMEM_BKPT;
          செல் moveto_index_finish;
        }
        pCur->ix = (u16)idx;
        rc = accessPayload(pCur, 0, nCell, (unsigned எழுத்து*)p_Cell_சாவி, 0);
        memset(((u8*)p_Cell_சாவி)+nCell,0,nOverrun); /* Fix uninit warnings */
        pCur->curFlags &= ~BTCF_ValidOvfl;
        எனில்( rc ){
          sqlite3_free(p_Cell_சாவி);
          செல் moveto_index_finish;
        }
        c = sqlite3VdbeRecordCompare(nCell, p_Cell_சாவி, p_Idx_சாவி);
        sqlite3_free(p_Cell_சாவி);
      }
      ப்ரதிஜ்ஞா(
          (p_Idx_சாவி->errCode!=SQLITE_CORRUPT || c==0)
       && (p_Idx_சாவி->errCode!=SQLITE_NOMEM || pCur->pBtree->db->mallocFailed)
      );
      எனில்( c<0 ){
        lwr = idx+1;
      }இல்லையெனில் எனில்( c>0 ){
        upr = idx-1;
      }இல்லையெனில்{
        ப்ரதிஜ்ஞா( c==0 );
        *pRes = 0;
        rc = SQLITE_OK;
        pCur->ix = (u16)idx;
        எனில்( p_Idx_சாவி->errCode ) rc = SQLITE_CORRUPT_BKPT;
        செல் moveto_index_finish;
      }
      எனில்( lwr>upr ) நிறுத்து;
      ப்ரதிஜ்ஞா( lwr+upr>=0 );
      idx = (lwr+upr)>>1;  /* idx = (lwr+upr)/2 */
    }
    ப்ரதிஜ்ஞா( lwr==upr+1 || (pPage->int_சாவி && !pPage->leaf) );
    ப்ரதிஜ்ஞா( pPage->isInit );
    எனில்( pPage->leaf ){
      ப்ரதிஜ்ஞா( pCur->ix<pCur->pPage->nCell || CORRUPT_DB );
      pCur->ix = (u16)idx;
      *pRes = c;
      rc = SQLITE_OK;
      செல் moveto_index_finish;
    }
    எனில்( lwr>=pPage->nCell ){
      chldPg = get4byte(&pPage->aData[pPage->hdrOffset+8]);
    }இல்லையெனில்{
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
    எனில்( pCur->iPage>=(BTCURSOR_MAX_DEPTH-1) ){
      திரும்பு SQLITE_CORRUPT_BKPT;
    }
    pCur->aiIdx[pCur->iPage] = (u16)lwr;
    pCur->apPage[pCur->iPage] = pCur->pPage;
    pCur->ix = 0;
    pCur->iPage++;
    rc = getAndInitPage(pCur->pBt, chldPg, &pCur->pPage, pCur->curPagerFlags);
    எனில்( rc==SQLITE_OK
     && (pCur->pPage->nCell<1 || pCur->pPage->int_சாவி!=pCur->cur_Int_சாவி)
    ){
      releasePage(pCur->pPage);
      rc = SQLITE_CORRUPT_PGNO(chldPg);
    }
    எனில்( rc ){
      pCur->pPage = pCur->apPage[--pCur->iPage];
      நிறுத்து;
    }
    /*
    ***** End of in-lined moveToChild() call */
 }
moveto_index_finish:
  pCur->info.nSize = 0;
  ப்ரதிஜ்ஞா( (pCur->curFlags & BTCF_ValidOvfl)==0 );
  திரும்பு rc;
}


/*
** Return TRUE if the cursor is not pointing at an entry of the table.
**
** TRUE will be returned after a call to sqlite3BtreeNext() moves
** past the last entry in the table or sqlite3BtreePrev() moves past
** the first entry.  TRUE is also returned if the table is empty.
*/
முழு sqlite3BtreeEof(BtCursor *pCur){
  /* TODO: What if the cursor is in CURSOR_REQUIRESEEK but all table entries
  ** have been deleted? This API will need to change to return an error code
  ** as well as the boolean result value.
  */
  திரும்பு (CURSOR_VALID!=pCur->eState);
}

/*
** Return an estimate for the number of rows in the table that pCur is
** pointing to.  Return a negative number if no estimate is currently
** available.
*/
i64 sqlite3_Btree_Row_மொத்தம்_Est(BtCursor *pCur){
  i64 n;
  u8 i;

  ப்ரதிஜ்ஞா( cursorOwnsBtShared(pCur) );
  ப்ரதிஜ்ஞா( sqlite3_mutex_held(pCur->pBtree->db->mutex) );

  /* Currently this interface is only called by the OP_IfSizeBetween
  ** opcode and the OP_Count opcode with P3=1.  In either case,
  ** the cursor will always be valid unless the btree is empty. */
  எனில்( pCur->eState!=CURSOR_VALID ) திரும்பு 0;
  எனில்( NEVER(pCur->pPage->leaf==0) ) திரும்பு -1;

  n = pCur->pPage->nCell;
  க்கு(i=0; i<pCur->iPage; i++){
    n *= pCur->apPage[i]->nCell+1;
  }
  திரும்பு n;
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
நிலையான SQLITE_NOINLINE முழு btreeNext(BtCursor *pCur){
  முழு rc;
  முழு idx;
  MemPage *pPage;

  ப்ரதிஜ்ஞா( cursorOwnsBtShared(pCur) );
  எனில்( pCur->eState!=CURSOR_VALID ){
    ப்ரதிஜ்ஞா( (pCur->curFlags & BTCF_ValidOvfl)==0 );
    rc = restoreCursorPosition(pCur);
    எனில்( rc!=SQLITE_OK ){
      திரும்பு rc;
    }
    எனில்( CURSOR_INVALID==pCur->eState ){
      திரும்பு SQLITE_DONE;
    }
    எனில்( pCur->eState==CURSOR_SKIPNEXT ){
      pCur->eState = CURSOR_VALID;
      எனில்( pCur->skipNext>0 ) திரும்பு SQLITE_OK;
    }
  }

  pPage = pCur->pPage;
  idx = ++pCur->ix;
  எனில்( sqlite3FaultSim(412) ) pPage->isInit = 0;
  எனில்( !pPage->isInit ){
    திரும்பு SQLITE_CORRUPT_BKPT;
  }

  எனில்( idx>=pPage->nCell ){
    எனில்( !pPage->leaf ){
      rc = moveToChild(pCur, get4byte(&pPage->aData[pPage->hdrOffset+8]));
      எனில்( rc ) திரும்பு rc;
      திரும்பு moveToLeftmost(pCur);
    }
    செய்{
      எனில்( pCur->iPage==0 ){
        pCur->eState = CURSOR_INVALID;
        திரும்பு SQLITE_DONE;
      }
      moveToParent(pCur);
      pPage = pCur->pPage;
    }போது( pCur->ix>=pPage->nCell );
    எனில்( pPage->int_சாவி ){
      திரும்பு sqlite3BtreeNext(pCur, 0);
    }இல்லையெனில்{
      திரும்பு SQLITE_OK;
    }
  }
  எனில்( pPage->leaf ){
    திரும்பு SQLITE_OK;
  }இல்லையெனில்{
    திரும்பு moveToLeftmost(pCur);
  }
}
முழு sqlite3BtreeNext(BtCursor *pCur, முழு flags){
  MemPage *pPage;
  UNUSED_PARAMETER( flags );  /* Used in COMDB2 but not native SQLite */
  ப்ரதிஜ்ஞா( cursorOwnsBtShared(pCur) );
  ப்ரதிஜ்ஞா( flags==0 || flags==1 );
  pCur->info.nSize = 0;
  pCur->curFlags &= ~(BTCF_ValidNKey|BTCF_ValidOvfl);
  எனில்( pCur->eState!=CURSOR_VALID ) திரும்பு btreeNext(pCur);
  pPage = pCur->pPage;
  எனில்( (++pCur->ix)>=pPage->nCell ){
    pCur->ix--;
    திரும்பு btreeNext(pCur);
  }
  எனில்( pPage->leaf ){
    திரும்பு SQLITE_OK;
  }இல்லையெனில்{
    திரும்பு moveToLeftmost(pCur);
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
நிலையான SQLITE_NOINLINE முழு btreePrevious(BtCursor *pCur){
  முழு rc;
  MemPage *pPage;

  ப்ரதிஜ்ஞா( cursorOwnsBtShared(pCur) );
  ப்ரதிஜ்ஞா( (pCur->curFlags & (BTCF_AtLast|BTCF_ValidOvfl|BTCF_ValidNKey))==0 );
  ப்ரதிஜ்ஞா( pCur->info.nSize==0 );
  எனில்( pCur->eState!=CURSOR_VALID ){
    rc = restoreCursorPosition(pCur);
    எனில்( rc!=SQLITE_OK ){
      திரும்பு rc;
    }
    எனில்( CURSOR_INVALID==pCur->eState ){
      திரும்பு SQLITE_DONE;
    }
    எனில்( CURSOR_SKIPNEXT==pCur->eState ){
      pCur->eState = CURSOR_VALID;
      எனில்( pCur->skipNext<0 ) திரும்பு SQLITE_OK;
    }
  }

  pPage = pCur->pPage;
  எனில்( sqlite3FaultSim(412) ) pPage->isInit = 0;
  எனில்( !pPage->isInit ){
    திரும்பு SQLITE_CORRUPT_BKPT;
  }
  எனில்( !pPage->leaf ){
    முழு idx = pCur->ix;
    rc = moveToChild(pCur, get4byte(findCell(pPage, idx)));
    எனில்( rc ) திரும்பு rc;
    rc = moveToRightmost(pCur);
  }இல்லையெனில்{
    போது( pCur->ix==0 ){
      எனில்( pCur->iPage==0 ){
        pCur->eState = CURSOR_INVALID;
        திரும்பு SQLITE_DONE;
      }
      moveToParent(pCur);
    }
    ப்ரதிஜ்ஞா( pCur->info.nSize==0 );
    ப்ரதிஜ்ஞா( (pCur->curFlags & (BTCF_ValidOvfl))==0 );

    pCur->ix--;
    pPage = pCur->pPage;
    எனில்( pPage->int_சாவி && !pPage->leaf ){
      rc = sqlite3BtreePrevious(pCur, 0);
    }இல்லையெனில்{
      rc = SQLITE_OK;
    }
  }
  திரும்பு rc;
}
முழு sqlite3BtreePrevious(BtCursor *pCur, முழு flags){
  ப்ரதிஜ்ஞா( cursorOwnsBtShared(pCur) );
  ப்ரதிஜ்ஞா( flags==0 || flags==1 );
  UNUSED_PARAMETER( flags );  /* Used in COMDB2 but not native SQLite */
  pCur->curFlags &= ~(BTCF_AtLast|BTCF_ValidOvfl|BTCF_ValidNKey);
  pCur->info.nSize = 0;
  எனில்( pCur->eState!=CURSOR_VALID
   || pCur->ix==0
   || pCur->pPage->leaf==0
  ){
    திரும்பு btreePrevious(pCur);
  }
  pCur->ix--;
  திரும்பு SQLITE_OK;
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
நிலையான முழு allocateBtreePage(
  BtShared *pBt,         /* The btree */
  MemPage **ppPage,      /* Store pointer to the allocated page here */
  Pgno *pPgno,           /* Store the page number here */
  Pgno nearby,           /* Search for a page near this one */
  u8 eMode               /* BTALLOC_EXACT, BTALLOC_LT, or BTALLOC_ANY */
){
  MemPage *pPage1;
  முழு rc;
  u32 n;     /* Number of pages on the freelist */
  u32 k;     /* Number of leaves on the trunk of the freelist */
  MemPage *pTrunk = 0;
  MemPage *pPrevTrunk = 0;
  Pgno mxPage;     /* Total size of the database file */

  ப்ரதிஜ்ஞா( sqlite3_mutex_held(pBt->mutex) );
  ப்ரதிஜ்ஞா( eMode==BTALLOC_ANY || (nearby>0 && IfNotOmitAV(pBt->autoVacuum)) );
  pPage1 = pBt->pPage1;
  mxPage = btreePagecount(pBt);
  /* EVIDENCE-OF: R-21003-45125 The 4-byte big-endian integer at offset 36
  ** stores the total number of pages on the freelist. */
  n = get4byte(&pPage1->aData[36]);
  testcase( n==mxPage-1 );
  எனில்( n>=mxPage ){
    திரும்பு SQLITE_CORRUPT_BKPT;
  }
  எனில்( n>0 ){
    /* There are pages on the freelist.  Reuse one of those pages. */
    Pgno iTrunk;
    u8 தேடல்_List = 0; /* If the free-list must be searched for 'nearby' */
    u32 n_தேடல் = 0;   /* Count of the number of search attempts */
   
    /* If eMode==BTALLOC_EXACT and a query of the pointer-map
    ** shows that the page 'nearby' is somewhere on the free-list, then
    ** the entire-list will be searched for that page.
    */
#ifndef SQLITE_OMIT_AUTOVACUUM
    எனில்( eMode==BTALLOC_EXACT ){
      எனில்( nearby<=mxPage ){
        u8 eType;
        ப்ரதிஜ்ஞா( nearby>0 );
        ப்ரதிஜ்ஞா( pBt->autoVacuum );
        rc = ptrmapGet(pBt, nearby, &eType, 0);
        எனில்( rc ) திரும்பு rc;
        எனில்( eType==PTRMAP_FREEPAGE ){
          தேடல்_List = 1;
        }
      }
    }இல்லையெனில் எனில்( eMode==BTALLOC_LE ){
      தேடல்_List = 1;
    }
#endif

    /* Decrement the free-list count by 1. Set iTrunk to the index of the
    ** first free-list trunk page. iPrevTrunk is initially 1.
    */
    rc = sqlite3PagerWrite(pPage1->pDbPage);
    எனில்( rc ) திரும்பு rc;
    put4byte(&pPage1->aData[36], n-1);

    /* The code within this loop is run only once if the 'searchList' variable
    ** is not true. Otherwise, it runs once for each trunk-page on the
    ** free-list until the page 'nearby' is located (eMode==BTALLOC_EXACT)
    ** or until a page less than 'nearby' is located (eMode==BTALLOC_LT)
    */
    செய் {
      pPrevTrunk = pTrunk;
      எனில்( pPrevTrunk ){
        /* EVIDENCE-OF: R-01506-11053 The first integer on a freelist trunk page
        ** is the page number of the next freelist trunk page in the list or
        ** zero if this is the last freelist trunk page. */
        iTrunk = get4byte(&pPrevTrunk->aData[0]);
      }இல்லையெனில்{
        /* EVIDENCE-OF: R-59841-13798 The 4-byte big-endian integer at offset 32
        ** stores the page number of the first page of the freelist, or zero if
        ** the freelist is empty. */
        iTrunk = get4byte(&pPage1->aData[32]);
      }
      testcase( iTrunk==mxPage );
      எனில்( iTrunk>mxPage || n_தேடல்++ > n ){
        rc = SQLITE_CORRUPT_PGNO(pPrevTrunk ? pPrevTrunk->pgno : 1);
      }இல்லையெனில்{
        rc = btreeGetUnusedPage(pBt, iTrunk, &pTrunk, 0);
      }
      எனில்( rc ){
        pTrunk = 0;
        செல் end_allocate_page;
      }
      ப்ரதிஜ்ஞா( pTrunk!=0 );
      ப்ரதிஜ்ஞா( pTrunk->aData!=0 );
      /* EVIDENCE-OF: R-13523-04394 The second integer on a freelist trunk page
      ** is the number of leaf page pointers to follow. */
      k = get4byte(&pTrunk->aData[4]);
      எனில்( k==0 && !தேடல்_List ){
        /* The trunk has no leaves and the list is not being searched.
        ** So extract the trunk page itself and use it as the newly
        ** allocated page */
        ப்ரதிஜ்ஞா( pPrevTrunk==0 );
        rc = sqlite3PagerWrite(pTrunk->pDbPage);
        எனில்( rc ){
          செல் end_allocate_page;
        }
        *pPgno = iTrunk;
        memcpy(&pPage1->aData[32], &pTrunk->aData[0], 4);
        *ppPage = pTrunk;
        pTrunk = 0;
        TRACE(("ALLOCATE: %u trunk - %u free pages left\n", *pPgno, n-1));
      }இல்லையெனில் எனில்( k>(u32)(pBt->usableSize/4 - 2) ){
        /* Value of k is out of range.  Database corruption */
        rc = SQLITE_CORRUPT_PGNO(iTrunk);
        செல் end_allocate_page;
#ifndef SQLITE_OMIT_AUTOVACUUM
      }இல்லையெனில் எனில்( தேடல்_List
            && (nearby==iTrunk || (iTrunk<nearby && eMode==BTALLOC_LE))
      ){
        /* The list is being searched and this trunk page is the page
        ** to allocate, regardless of whether it has leaves.
        */
        *pPgno = iTrunk;
        *ppPage = pTrunk;
        தேடல்_List = 0;
        rc = sqlite3PagerWrite(pTrunk->pDbPage);
        எனில்( rc ){
          செல் end_allocate_page;
        }
        எனில்( k==0 ){
          எனில்( !pPrevTrunk ){
            memcpy(&pPage1->aData[32], &pTrunk->aData[0], 4);
          }இல்லையெனில்{
            rc = sqlite3PagerWrite(pPrevTrunk->pDbPage);
            எனில்( rc!=SQLITE_OK ){
              செல் end_allocate_page;
            }
            memcpy(&pPrevTrunk->aData[0], &pTrunk->aData[0], 4);
          }
        }இல்லையெனில்{
          /* The trunk page is required by the caller but it contains
          ** pointers to free-list leaves. The first leaf becomes a trunk
          ** page in this case.
          */
          MemPage *pNewTrunk;
          Pgno iNewTrunk = get4byte(&pTrunk->aData[8]);
          எனில்( iNewTrunk>mxPage ){
            rc = SQLITE_CORRUPT_PGNO(iTrunk);
            செல் end_allocate_page;
          }
          testcase( iNewTrunk==mxPage );
          rc = btreeGetUnusedPage(pBt, iNewTrunk, &pNewTrunk, 0);
          எனில்( rc!=SQLITE_OK ){
            செல் end_allocate_page;
          }
          rc = sqlite3PagerWrite(pNewTrunk->pDbPage);
          எனில்( rc!=SQLITE_OK ){
            releasePage(pNewTrunk);
            செல் end_allocate_page;
          }
          memcpy(&pNewTrunk->aData[0], &pTrunk->aData[0], 4);
          put4byte(&pNewTrunk->aData[4], k-1);
          memcpy(&pNewTrunk->aData[8], &pTrunk->aData[12], (k-1)*4);
          releasePage(pNewTrunk);
          எனில்( !pPrevTrunk ){
            ப்ரதிஜ்ஞா( sqlite3PagerIswriteable(pPage1->pDbPage) );
            put4byte(&pPage1->aData[32], iNewTrunk);
          }இல்லையெனில்{
            rc = sqlite3PagerWrite(pPrevTrunk->pDbPage);
            எனில்( rc ){
              செல் end_allocate_page;
            }
            put4byte(&pPrevTrunk->aData[0], iNewTrunk);
          }
        }
        pTrunk = 0;
        TRACE(("ALLOCATE: %u trunk - %u free pages left\n", *pPgno, n-1));
#endif
      }இல்லையெனில் எனில்( k>0 ){
        /* Extract a leaf from the trunk */
        u32 closest;
        Pgno iPage;
        unsigned எழுத்து *aData = pTrunk->aData;
        எனில்( nearby>0 ){
          u32 i;
          closest = 0;
          எனில்( eMode==BTALLOC_LE ){
            க்கு(i=0; i<k; i++){
              iPage = get4byte(&aData[8+i*4]);
              எனில்( iPage<=nearby ){
                closest = i;
                நிறுத்து;
              }
            }
          }இல்லையெனில்{
            முழு dist;
            dist = sqlite3AbsInt32(get4byte(&aData[8]) - nearby);
            க்கு(i=1; i<k; i++){
              முழு d2 = sqlite3AbsInt32(get4byte(&aData[8+i*4]) - nearby);
              எனில்( d2<dist ){
                closest = i;
                dist = d2;
              }
            }
          }
        }இல்லையெனில்{
          closest = 0;
        }

        iPage = get4byte(&aData[8+closest*4]);
        testcase( iPage==mxPage );
        எனில்( iPage>mxPage || iPage<2 ){
          rc = SQLITE_CORRUPT_PGNO(iTrunk);
          செல் end_allocate_page;
        }
        testcase( iPage==mxPage );
        எனில்( !தேடல்_List
         || (iPage==nearby || (iPage<nearby && eMode==BTALLOC_LE))
        ){
          முழு noContent;
          *pPgno = iPage;
          TRACE(("ALLOCATE: %u was leaf %u of %u on trunk %u"
                 ": %u more free pages\n",
                 *pPgno, closest+1, k, pTrunk->pgno, n-1));
          rc = sqlite3PagerWrite(pTrunk->pDbPage);
          எனில்( rc ) செல் end_allocate_page;
          எனில்( closest<k-1 ){
            memcpy(&aData[8+closest*4], &aData[4+k*4], 4);
          }
          put4byte(&aData[4], k-1);
          noContent = !btreeGetHasContent(pBt, *pPgno)? PAGER_GET_NOCONTENT : 0;
          rc = btreeGetUnusedPage(pBt, *pPgno, ppPage, noContent);
          எனில்( rc==SQLITE_OK ){
            rc = sqlite3PagerWrite((*ppPage)->pDbPage);
            எனில்( rc!=SQLITE_OK ){
              releasePage(*ppPage);
              *ppPage = 0;
            }
          }
          தேடல்_List = 0;
        }
      }
      releasePage(pPrevTrunk);
      pPrevTrunk = 0;
    }போது( தேடல்_List );
  }இல்லையெனில்{
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
    முழு bNoContent = (0==IfNotOmitAV(pBt->bDoTruncate))? PAGER_GET_NOCONTENT:0;

    rc = sqlite3PagerWrite(pBt->pPage1->pDbPage);
    எனில்( rc ) திரும்பு rc;
    pBt->nPage++;
    எனில்( pBt->nPage==PENDING_BYTE_PAGE(pBt) ) pBt->nPage++;

#ifndef SQLITE_OMIT_AUTOVACUUM
    எனில்( pBt->autoVacuum && PTRMAP_ISPAGE(pBt, pBt->nPage) ){
      /* If *pPgno refers to a pointer-map page, allocate two new pages
      ** at the end of the file instead of one. The first allocated page
      ** becomes a new pointer-map page, the second is used by the caller.
      */
      MemPage *pPg = 0;
      TRACE(("ALLOCATE: %u from end of file (pointer-map page)\n", pBt->nPage));
      ப்ரதிஜ்ஞா( pBt->nPage!=PENDING_BYTE_PAGE(pBt) );
      rc = btreeGetUnusedPage(pBt, pBt->nPage, &pPg, bNoContent);
      எனில்( rc==SQLITE_OK ){
        rc = sqlite3PagerWrite(pPg->pDbPage);
        releasePage(pPg);
      }
      எனில்( rc ) திரும்பு rc;
      pBt->nPage++;
      எனில்( pBt->nPage==PENDING_BYTE_PAGE(pBt) ){ pBt->nPage++; }
    }
#endif
    put4byte(28 + (u8*)pBt->pPage1->aData, pBt->nPage);
    *pPgno = pBt->nPage;

    ப்ரதிஜ்ஞா( *pPgno!=PENDING_BYTE_PAGE(pBt) );
    rc = btreeGetUnusedPage(pBt, *pPgno, ppPage, bNoContent);
    எனில்( rc ) திரும்பு rc;
    rc = sqlite3PagerWrite((*ppPage)->pDbPage);
    எனில்( rc!=SQLITE_OK ){
      releasePage(*ppPage);
      *ppPage = 0;
    }
    TRACE(("ALLOCATE: %u from end of file\n", *pPgno));
  }

  ப்ரதிஜ்ஞா( CORRUPT_DB || *pPgno!=PENDING_BYTE_PAGE(pBt) );

end_allocate_page:
  releasePage(pTrunk);
  releasePage(pPrevTrunk);
  ப்ரதிஜ்ஞா( rc!=SQLITE_OK || sqlite3PagerPageRefcount((*ppPage)->pDbPage)<=1 );
  ப்ரதிஜ்ஞா( rc!=SQLITE_OK || (*ppPage)->isInit==0 );
  திரும்பு rc;
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
நிலையான முழு freePage2(BtShared *pBt, MemPage *pMemPage, Pgno iPage){
  MemPage *pTrunk = 0;                /* Free-list trunk page */
  Pgno iTrunk = 0;                    /* Page number of free-list trunk page */
  MemPage *pPage1 = pBt->pPage1;      /* Local reference to page 1 */
  MemPage *pPage;                     /* Page being freed. May be NULL. */
  முழு rc;                             /* Return Code */
  u32 nFree;                          /* Initial number of pages on free-list */

  ப்ரதிஜ்ஞா( sqlite3_mutex_held(pBt->mutex) );
  ப்ரதிஜ்ஞா( CORRUPT_DB || iPage>1 );
  ப்ரதிஜ்ஞா( !pMemPage || pMemPage->pgno==iPage );

  எனில்( iPage<2 || iPage>pBt->nPage ){
    திரும்பு SQLITE_CORRUPT_BKPT;
  }
  எனில்( pMemPage ){
    pPage = pMemPage;
    sqlite3PagerRef(pPage->pDbPage);
  }இல்லையெனில்{
    pPage = btreePageLookup(pBt, iPage);
  }

  /* Increment the free page count on pPage1 */
  rc = sqlite3PagerWrite(pPage1->pDbPage);
  எனில்( rc ) செல் freepage_out;
  nFree = get4byte(&pPage1->aData[36]);
  put4byte(&pPage1->aData[36], nFree+1);

  எனில்( pBt->btsFlags & BTS_SECURE_DELETE ){
    /* If the secure_delete option is enabled, then
    ** always fully overwrite deleted information with zeros.
    */
    எனில்( (!pPage && ((rc = btreeGetPage(pBt, iPage, &pPage, 0))!=0) )
     ||            ((rc = sqlite3PagerWrite(pPage->pDbPage))!=0)
    ){
      செல் freepage_out;
    }
    memset(pPage->aData, 0, pPage->pBt->pageSize);
  }

  /* If the database supports auto-vacuum, write an entry in the pointer-map
  ** to indicate that the page is free.
  */
  எனில்( ISAUTOVACUUM(pBt) ){
    ptrmapPut(pBt, iPage, PTRMAP_FREEPAGE, 0, &rc);
    எனில்( rc ) செல் freepage_out;
  }

  /* Now manipulate the actual database free-list structure. There are two
  ** possibilities. If the free-list is currently empty, or if the first
  ** trunk page in the free-list is full, then this page will become a
  ** new free-list trunk page. Otherwise, it will become a leaf of the
  ** first trunk page in the current free-list. This block tests if it
  ** is possible to add the page as a new free-list leaf.
  */
  எனில்( nFree!=0 ){
    u32 nLeaf;                /* Initial number of leaf cells on trunk page */

    iTrunk = get4byte(&pPage1->aData[32]);
    எனில்( iTrunk>btreePagecount(pBt) ){
      rc = SQLITE_CORRUPT_BKPT;
      செல் freepage_out;
    }
    rc = btreeGetPage(pBt, iTrunk, &pTrunk, 0);
    எனில்( rc!=SQLITE_OK ){
      செல் freepage_out;
    }

    nLeaf = get4byte(&pTrunk->aData[4]);
    ப்ரதிஜ்ஞா( pBt->usableSize>32 );
    எனில்( nLeaf > (u32)pBt->usableSize/4 - 2 ){
      rc = SQLITE_CORRUPT_BKPT;
      செல் freepage_out;
    }
    எனில்( nLeaf < (u32)pBt->usableSize/4 - 8 ){
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
      எனில்( rc==SQLITE_OK ){
        put4byte(&pTrunk->aData[4], nLeaf+1);
        put4byte(&pTrunk->aData[8+nLeaf*4], iPage);
        எனில்( pPage && (pBt->btsFlags & BTS_SECURE_DELETE)==0 ){
          sqlite3PagerDontWrite(pPage->pDbPage);
        }
        rc = btreeSetHasContent(pBt, iPage);
      }
      TRACE(("FREE-PAGE: %u leaf on trunk page %u\n",pPage->pgno,pTrunk->pgno));
      செல் freepage_out;
    }
  }

  /* If control flows to this point, then it was not possible to add the
  ** the page being freed as a leaf page of the first trunk in the free-list.
  ** Possibly because the free-list is empty, or possibly because the
  ** first trunk in the free-list is full. Either way, the page being freed
  ** will become the new first trunk page in the free-list.
  */
  எனில்( pPage==0 && SQLITE_OK!=(rc = btreeGetPage(pBt, iPage, &pPage, 0)) ){
    செல் freepage_out;
  }
  rc = sqlite3PagerWrite(pPage->pDbPage);
  எனில்( rc!=SQLITE_OK ){
    செல் freepage_out;
  }
  put4byte(pPage->aData, iTrunk);
  put4byte(&pPage->aData[4], 0);
  put4byte(&pPage1->aData[32], iPage);
  TRACE(("FREE-PAGE: %u new trunk page replacing %u\n", pPage->pgno, iTrunk));

freepage_out:
  எனில்( pPage ){
    pPage->isInit = 0;
  }
  releasePage(pPage);
  releasePage(pTrunk);
  திரும்பு rc;
}
நிலையான வெறுமை freePage(MemPage *pPage, முழு *pRC){
  எனில்( (*pRC)==SQLITE_OK ){
    *pRC = freePage2(pPage->pBt, pPage, pPage->pgno);
  }
}

/*
** Free the overflow pages associated with the given Cell.
*/
நிலையான SQLITE_NOINLINE முழு clearCellOverflow(
  MemPage *pPage,          /* The page that contains the Cell */
  unsigned எழுத்து *pCell,    /* First byte of the Cell */
  CellInfo *pInfo          /* Size information about the cell */
){
  BtShared *pBt;
  Pgno ovflPgno;
  முழு rc;
  முழு nOvfl;
  u32 ovflPageSize;

  ப்ரதிஜ்ஞா( sqlite3_mutex_held(pPage->pBt->mutex) );
  ப்ரதிஜ்ஞா( pInfo->nLocal!=pInfo->nPayload );
  testcase( pCell + pInfo->nSize == pPage->aDataEnd );
  testcase( pCell + (pInfo->nSize-1) == pPage->aDataEnd );
  எனில்( pCell + pInfo->nSize > pPage->aDataEnd ){
    /* Cell extends past end of page */
    திரும்பு SQLITE_CORRUPT_PAGE(pPage);
  }
  ovflPgno = get4byte(pCell + pInfo->nSize - 4);
  pBt = pPage->pBt;
  ப்ரதிஜ்ஞா( pBt->usableSize > 4 );
  ovflPageSize = pBt->usableSize - 4;
  nOvfl = (pInfo->nPayload - pInfo->nLocal + ovflPageSize - 1)/ovflPageSize;
  ப்ரதிஜ்ஞா( nOvfl>0 ||
    (CORRUPT_DB && (pInfo->nPayload + ovflPageSize)<ovflPageSize)
  );
  போது( nOvfl-- ){
    Pgno iNext = 0;
    MemPage *pOvfl = 0;
    எனில்( ovflPgno<2 || ovflPgno>btreePagecount(pBt) ){
      /* 0 is not a legal page number and page 1 cannot be an
      ** overflow page. Therefore if ovflPgno<2 or past the end of the
      ** file the database must be corrupt. */
      திரும்பு SQLITE_CORRUPT_BKPT;
    }
    எனில்( nOvfl ){
      rc = getOverflowPage(pBt, ovflPgno, &pOvfl, &iNext);
      எனில்( rc ) திரும்பு rc;
    }

    எனில்( ( pOvfl || ((pOvfl = btreePageLookup(pBt, ovflPgno))!=0) )
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
    }இல்லையெனில்{
      rc = freePage2(pBt, pOvfl, ovflPgno);
    }

    எனில்( pOvfl ){
      sqlite3PagerUnref(pOvfl->pDbPage);
    }
    எனில்( rc ) திரும்பு rc;
    ovflPgno = iNext;
  }
  திரும்பு SQLITE_OK;
}

/* Call xParseCell to compute the size of a cell.  If the cell contains
** overflow, then invoke cellClearOverflow to clear out that overflow.
** Store the result code (SQLITE_OK or some error code) in rc.
**
** Implemented as macro to force inlining for performance.
*/
#define BTREE_CLEAR_CELL(rc, pPage, pCell, sInfo)   \
  pPage->xParseCell(pPage, pCell, &sInfo);          \
  எனில்( sInfo.nLocal!=sInfo.nPayload ){               \
    rc = clearCellOverflow(pPage, pCell, &sInfo);   \
  }இல்லையெனில்{                                            \
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
நிலையான முழு fillInCell(
  MemPage *pPage,                /* The page that contains the cell */
  unsigned எழுத்து *pCell,          /* Complete text of the cell */
  மாறிலி BtreePayload *pX,        /* Payload with which to construct the cell */
  முழு *pnSize                    /* Write cell size here */
){
  முழு nPayload;
  மாறிலி u8 *pSrc;
  முழு nSrc, n, rc, mn;
  முழு space_இடது;
  MemPage *pToRelease;
  unsigned எழுத்து *pPrior;
  unsigned எழுத்து *pPayload;
  BtShared *pBt;
  Pgno pgnoOvfl;
  முழு nHeader;

  ப்ரதிஜ்ஞா( sqlite3_mutex_held(pPage->pBt->mutex) );

  /* pPage is not necessarily writeable since pCell might be auxiliary
  ** buffer space that is separate from the pPage buffer area */
  ப்ரதிஜ்ஞா( pCell<pPage->aData || pCell>=&pPage->aData[pPage->pBt->pageSize]
            || sqlite3PagerIswriteable(pPage->pDbPage) );

  /* Fill in the header. */
  nHeader = pPage->childPtrSize;
  எனில்( pPage->int_சாவி ){
    nPayload = pX->nData + pX->nZero;
    pSrc = pX->pData;
    nSrc = pX->nData;
    ப்ரதிஜ்ஞா( pPage->int_சாவி_Leaf ); /* fillInCell() only called for leaves */
    nHeader += putVarint32(&pCell[nHeader], nPayload);
    nHeader += sqlite3PutVarint(&pCell[nHeader], *(u64*)&pX->n_சாவி);
  }இல்லையெனில்{
    ப்ரதிஜ்ஞா( pX->n_சாவி<=0x7fffffff && pX->p_சாவி!=0 );
    nSrc = nPayload = (முழு)pX->n_சாவி;
    pSrc = pX->p_சாவி;
    nHeader += putVarint32(&pCell[nHeader], nPayload);
  }
 
  /* Fill in the payload */
  pPayload = &pCell[nHeader];
  எனில்( nPayload<=pPage->maxLocal ){
    /* This is the common case where everything fits on the btree page
    ** and no overflow pages are required. */
    n = nHeader + nPayload;
    testcase( n==3 );
    testcase( n==4 );
    எனில்( n<4 ){
      n = 4;
      pPayload[nPayload] = 0;
    }
    *pnSize = n;
    ப்ரதிஜ்ஞா( nSrc<=nPayload );
    testcase( nSrc<nPayload );
    memcpy(pPayload, pSrc, nSrc);
    memset(pPayload+nSrc, 0, nPayload-nSrc);
    திரும்பு SQLITE_OK;
  }

  /* If we reach this point, it means that some of the content will need
  ** to spill onto overflow pages.
  */
  mn = pPage->minLocal;
  n = mn + (nPayload - mn) % (pPage->pBt->usableSize - 4);
  testcase( n==pPage->maxLocal );
  testcase( n==pPage->maxLocal+1 );
  எனில்( n > pPage->maxLocal ) n = mn;
  space_இடது = n;
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
    ப்ரதிஜ்ஞா( nHeader==(முழு)(info.pPayload - pCell) );
    ப்ரதிஜ்ஞா( info.n_சாவி==pX->n_சாவி );
    ப்ரதிஜ்ஞா( *pnSize == info.nSize );
    ப்ரதிஜ்ஞா( space_இடது == info.nLocal );
  }
#endif

  /* Write the payload into the local Cell and any extra into overflow pages */
  போது( 1 ){
    n = nPayload;
    எனில்( n>space_இடது ) n = space_இடது;

    /* If pToRelease is not zero than pPayload points into the data area
    ** of pToRelease.  Make sure pToRelease is still writeable. */
    ப்ரதிஜ்ஞா( pToRelease==0 || sqlite3PagerIswriteable(pToRelease->pDbPage) );

    /* If pPayload is part of the data area of pPage, then make sure pPage
    ** is still writeable */
    ப்ரதிஜ்ஞா( pPayload<pPage->aData || pPayload>=&pPage->aData[pBt->pageSize]
            || sqlite3PagerIswriteable(pPage->pDbPage) );

    எனில்( nSrc>=n ){
      memcpy(pPayload, pSrc, n);
    }இல்லையெனில் எனில்( nSrc>0 ){
      n = nSrc;
      memcpy(pPayload, pSrc, n);
    }இல்லையெனில்{
      memset(pPayload, 0, n);
    }
    nPayload -= n;
    எனில்( nPayload<=0 ) நிறுத்து;
    pPayload += n;
    pSrc += n;
    nSrc -= n;
    space_இடது -= n;
    எனில்( space_இடது==0 ){
      MemPage *pOvfl = 0;
#ifndef SQLITE_OMIT_AUTOVACUUM
      Pgno pgnoPtrmap = pgnoOvfl; /* Overflow page pointer-map entry page */
      எனில்( pBt->autoVacuum ){
        செய்{
          pgnoOvfl++;
        } போது(
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
      எனில்( pBt->autoVacuum && rc==SQLITE_OK ){
        u8 eType = (pgnoPtrmap?PTRMAP_OVERFLOW2:PTRMAP_OVERFLOW1);
        ptrmapPut(pBt, pgnoOvfl, eType, pgnoPtrmap, &rc);
        எனில்( rc ){
          releasePage(pOvfl);
        }
      }
#endif
      எனில்( rc ){
        releasePage(pToRelease);
        திரும்பு rc;
      }

      /* If pToRelease is not zero than pPrior points into the data area
      ** of pToRelease.  Make sure pToRelease is still writeable. */
      ப்ரதிஜ்ஞா( pToRelease==0 || sqlite3PagerIswriteable(pToRelease->pDbPage) );

      /* If pPrior is part of the data area of pPage, then make sure pPage
      ** is still writeable */
      ப்ரதிஜ்ஞா( pPrior<pPage->aData || pPrior>=&pPage->aData[pBt->pageSize]
            || sqlite3PagerIswriteable(pPage->pDbPage) );

      put4byte(pPrior, pgnoOvfl);
      releasePage(pToRelease);
      pToRelease = pOvfl;
      pPrior = pOvfl->aData;
      put4byte(pPrior, 0);
      pPayload = &pOvfl->aData[4];
      space_இடது = pBt->usableSize - 4;
    }
  }
  releasePage(pToRelease);
  திரும்பு SQLITE_OK;
}

/*
** Remove the i-th cell from pPage.  This routine effects pPage only.
** The cell content is not freed or deallocated.  It is assumed that
** the cell content has been copied someplace else.  This routine just
** removes the reference to the cell from pPage.
**
** "sz" must be the number of bytes in the cell.
*/
நிலையான வெறுமை dropCell(MemPage *pPage, முழு idx, முழு sz, முழு *pRC){
  u32 pc;         /* Offset to cell content of cell being deleted */
  u8 *data;       /* pPage->aData */
  u8 *ptr;        /* Used to move bytes around within data[] */
  முழு rc;         /* The return code */
  முழு hdr;        /* Beginning of the header.  0 most pages.  100 page 1 */

  எனில்( *pRC ) திரும்பு;
  ப்ரதிஜ்ஞா( idx>=0 );
  ப்ரதிஜ்ஞா( idx<pPage->nCell );
  ப்ரதிஜ்ஞா( CORRUPT_DB || sz==cellSize(pPage, idx) );
  ப்ரதிஜ்ஞா( sqlite3PagerIswriteable(pPage->pDbPage) );
  ப்ரதிஜ்ஞா( sqlite3_mutex_held(pPage->pBt->mutex) );
  ப்ரதிஜ்ஞா( pPage->nFree>=0 );
  data = pPage->aData;
  ptr = &pPage->aCellIdx[2*idx];
  ப்ரதிஜ்ஞா( pPage->pBt->usableSize > (u32)(ptr-data) );
  pc = get2byte(ptr);
  hdr = pPage->hdrOffset;
  testcase( pc==(u32)get2byte(&data[hdr+5]) );
  testcase( pc+sz==pPage->pBt->usableSize );
  எனில்( pc+sz > pPage->pBt->usableSize ){
    *pRC = SQLITE_CORRUPT_BKPT;
    திரும்பு;
  }
  rc = freeSpace(pPage, pc, sz);
  எனில்( rc ){
    *pRC = rc;
    திரும்பு;
  }
  pPage->nCell--;
  எனில்( pPage->nCell==0 ){
    memset(&data[hdr+1], 0, 4);
    data[hdr+7] = 0;
    put2byte(&data[hdr+5], pPage->pBt->usableSize);
    pPage->nFree = pPage->pBt->usableSize - pPage->hdrOffset
                       - pPage->childPtrSize - 8;
  }இல்லையெனில்{
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
நிலையான முழு செருகு_Cell(
  MemPage *pPage,   /* Page into which we are copying */
  முழு i,            /* New cell becomes the i-th cell of the page */
  u8 *pCell,        /* Content of the new cell */
  முழு sz,           /* Bytes of content in pCell */
  u8 *pTemp,        /* Temp storage space for pCell, if needed */
  Pgno iChild       /* If non-zero, replace first 4 bytes with this value */
){
  முழு idx = 0;      /* Where to write new cell content in data[] */
  முழு j;            /* Loop counter */
  u8 *data;         /* The content of the whole page */
  u8 *pIns;         /* The point in pPage->aCellIdx[] where no cell inserted */

  ப்ரதிஜ்ஞா( i>=0 && i<=pPage->nCell+pPage->nOverflow );
  ப்ரதிஜ்ஞா( MX_CELL(pPage->pBt)<=10921 );
  ப்ரதிஜ்ஞா( pPage->nCell<=MX_CELL(pPage->pBt) || CORRUPT_DB );
  ப்ரதிஜ்ஞா( pPage->nOverflow<=ArraySize(pPage->apOvfl) );
  ப்ரதிஜ்ஞா( ArraySize(pPage->apOvfl)==ArraySize(pPage->aiOvfl) );
  ப்ரதிஜ்ஞா( sqlite3_mutex_held(pPage->pBt->mutex) );
  ப்ரதிஜ்ஞா( sz==pPage->xCellSize(pPage, pCell) || CORRUPT_DB );
  ப்ரதிஜ்ஞா( pPage->nFree>=0 );
  ப்ரதிஜ்ஞா( iChild>0 );
  எனில்( pPage->nOverflow || sz+2>pPage->nFree ){
    எனில்( pTemp ){
      memcpy(pTemp, pCell, sz);
      pCell = pTemp;
    }
    put4byte(pCell, iChild);
    j = pPage->nOverflow++;
    /* Comparison against ArraySize-1 since we hold back one extra slot
    ** as a contingency.  In other words, never need more than 3 overflow
    ** slots but 4 are allocated, just to be safe. */
    ப்ரதிஜ்ஞா( j < ArraySize(pPage->apOvfl)-1 );
    pPage->apOvfl[j] = pCell;
    pPage->aiOvfl[j] = (u16)i;

    /* When multiple overflows occur, they are always sequential and in
    ** sorted order.  This invariants arise because multiple overflows can
    ** only occur when inserting divider cells into the parent page during
    ** balancing, and the dividers are adjacent and sorted.
    */
    ப்ரதிஜ்ஞா( j==0 || pPage->aiOvfl[j-1]<(u16)i ); /* Overflows in sorted order */
    ப்ரதிஜ்ஞா( j==0 || i==pPage->aiOvfl[j-1]+1 );   /* Overflows are sequential */
  }இல்லையெனில்{
    முழு rc = sqlite3PagerWrite(pPage->pDbPage);
    எனில்( NEVER(rc!=SQLITE_OK) ){
      திரும்பு rc;
    }
    ப்ரதிஜ்ஞா( sqlite3PagerIswriteable(pPage->pDbPage) );
    data = pPage->aData;
    ப்ரதிஜ்ஞா( &data[pPage->cellOffset]==pPage->aCellIdx );
    rc = allocateSpace(pPage, sz, &idx);
    எனில்( rc ){ திரும்பு rc; }
    /* The allocateSpace() routine guarantees the following properties
    ** if it returns successfully */
    ப்ரதிஜ்ஞா( idx >= 0 );
    ப்ரதிஜ்ஞா( idx >= pPage->cellOffset+2*pPage->nCell+2 || CORRUPT_DB );
    ப்ரதிஜ்ஞா( idx+sz <= (முழு)pPage->pBt->usableSize );
    pPage->nFree -= (u16)(2 + sz);
    /* In a corrupt database where an entry in the cell index section of
    ** a btree page has a value of 3 or less, the pCell value might point
    ** as many as 4 bytes in front of the start of the aData buffer for
    ** the source page.  Make sure this does not cause problems by not
    ** reading the first 4 bytes */
    memcpy(&data[idx+4], pCell+4, sz-4);
    put4byte(&data[idx], iChild);
    pIns = pPage->aCellIdx + i*2;
    எனில்( i<pPage->nCell ) memmove(pIns+2, pIns, 2*(pPage->nCell - i));
    put2byte(pIns, idx);
    pPage->nCell++;
    /* increment the cell count */
    எனில்( (++data[pPage->hdrOffset+4])==0 ) data[pPage->hdrOffset+3]++;
    ப்ரதிஜ்ஞா( get2byte(&data[pPage->hdrOffset+3])==pPage->nCell || CORRUPT_DB );
#ifndef SQLITE_OMIT_AUTOVACUUM
    எனில்( pPage->pBt->autoVacuum ){
      முழு rc2 = SQLITE_OK;
      /* The cell may contain a pointer to an overflow page. If so, write
      ** the entry for the overflow page into the pointer map.
      */
      ptrmapPutOvflPtr(pPage, pPage, pCell, &rc2);
      எனில்( rc2 ) திரும்பு rc2;
    }
#endif
  }
  திரும்பு SQLITE_OK;
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
நிலையான முழு செருகு_Cell_Fast(
  MemPage *pPage,   /* Page into which we are copying */
  முழு i,            /* New cell becomes the i-th cell of the page */
  u8 *pCell,        /* Content of the new cell */
  முழு sz            /* Bytes of content in pCell */
){
  முழு idx = 0;      /* Where to write new cell content in data[] */
  முழு j;            /* Loop counter */
  u8 *data;         /* The content of the whole page */
  u8 *pIns;         /* The point in pPage->aCellIdx[] where no cell inserted */

  ப்ரதிஜ்ஞா( i>=0 && i<=pPage->nCell+pPage->nOverflow );
  ப்ரதிஜ்ஞா( MX_CELL(pPage->pBt)<=10921 );
  ப்ரதிஜ்ஞா( pPage->nCell<=MX_CELL(pPage->pBt) || CORRUPT_DB );
  ப்ரதிஜ்ஞா( pPage->nOverflow<=ArraySize(pPage->apOvfl) );
  ப்ரதிஜ்ஞா( ArraySize(pPage->apOvfl)==ArraySize(pPage->aiOvfl) );
  ப்ரதிஜ்ஞா( sqlite3_mutex_held(pPage->pBt->mutex) );
  ப்ரதிஜ்ஞா( sz==pPage->xCellSize(pPage, pCell) || CORRUPT_DB );
  ப்ரதிஜ்ஞா( pPage->nFree>=0 );
  ப்ரதிஜ்ஞா( pPage->nOverflow==0 );
  எனில்( sz+2>pPage->nFree ){
    j = pPage->nOverflow++;
    /* Comparison against ArraySize-1 since we hold back one extra slot
    ** as a contingency.  In other words, never need more than 3 overflow
    ** slots but 4 are allocated, just to be safe. */
    ப்ரதிஜ்ஞா( j < ArraySize(pPage->apOvfl)-1 );
    pPage->apOvfl[j] = pCell;
    pPage->aiOvfl[j] = (u16)i;

    /* When multiple overflows occur, they are always sequential and in
    ** sorted order.  This invariants arise because multiple overflows can
    ** only occur when inserting divider cells into the parent page during
    ** balancing, and the dividers are adjacent and sorted.
    */
    ப்ரதிஜ்ஞா( j==0 || pPage->aiOvfl[j-1]<(u16)i ); /* Overflows in sorted order */
    ப்ரதிஜ்ஞா( j==0 || i==pPage->aiOvfl[j-1]+1 );   /* Overflows are sequential */
  }இல்லையெனில்{
    முழு rc = sqlite3PagerWrite(pPage->pDbPage);
    எனில்( rc!=SQLITE_OK ){
      திரும்பு rc;
    }
    ப்ரதிஜ்ஞா( sqlite3PagerIswriteable(pPage->pDbPage) );
    data = pPage->aData;
    ப்ரதிஜ்ஞா( &data[pPage->cellOffset]==pPage->aCellIdx );
    rc = allocateSpace(pPage, sz, &idx);
    எனில்( rc ){ திரும்பு rc; }
    /* The allocateSpace() routine guarantees the following properties
    ** if it returns successfully */
    ப்ரதிஜ்ஞா( idx >= 0 );
    ப்ரதிஜ்ஞா( idx >= pPage->cellOffset+2*pPage->nCell+2 || CORRUPT_DB );
    ப்ரதிஜ்ஞா( idx+sz <= (முழு)pPage->pBt->usableSize );
    pPage->nFree -= (u16)(2 + sz);
    memcpy(&data[idx], pCell, sz);
    pIns = pPage->aCellIdx + i*2;
    எனில்( i<pPage->nCell ) memmove(pIns+2, pIns, 2*(pPage->nCell - i));
    put2byte(pIns, idx);
    pPage->nCell++;
    /* increment the cell count */
    எனில்( (++data[pPage->hdrOffset+4])==0 ) data[pPage->hdrOffset+3]++;
    ப்ரதிஜ்ஞா( get2byte(&data[pPage->hdrOffset+3])==pPage->nCell || CORRUPT_DB );
#ifndef SQLITE_OMIT_AUTOVACUUM
    எனில்( pPage->pBt->autoVacuum ){
      முழு rc2 = SQLITE_OK;
      /* The cell may contain a pointer to an overflow page. If so, write
      ** the entry for the overflow page into the pointer map.
      */
      ptrmapPutOvflPtr(pPage, pPage, pCell, &rc2);
      எனில்( rc2 ) திரும்பு rc2;
    }
#endif
  }
  திரும்பு SQLITE_OK;
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
வகைவரை கட்டமைப்பு CellArray CellArray;
கட்டமைப்பு CellArray {
  முழு nCell;              /* Number of cells in apCell[] */
  MemPage *pRef;          /* Reference page */
  u8 **apCell;            /* All cells begin balanced */
  u16 *szCell;            /* Local size of all cells in apCell[] */
  u8 *apEnd[NB*2];        /* MemPage.aDataEnd values */
  முழு ixNx[NB*2];         /* Index of at which we move to the next apEnd[] */
};

/*
** Make sure the cell sizes at idx, idx+1, ..., idx+N-1 have been
** computed.
*/
நிலையான வெறுமை populateCellCache(CellArray *p, முழு idx, முழு N){
  MemPage *pRef = p->pRef;
  u16 *szCell = p->szCell;
  ப்ரதிஜ்ஞா( idx>=0 && idx+N<=p->nCell );
  போது( N>0 ){
    ப்ரதிஜ்ஞா( p->apCell[idx]!=0 );
    எனில்( szCell[idx]==0 ){
      szCell[idx] = pRef->xCellSize(pRef, p->apCell[idx]);
    }இல்லையெனில்{
      ப்ரதிஜ்ஞா( CORRUPT_DB ||
              szCell[idx]==pRef->xCellSize(pRef, p->apCell[idx]) );
    }
    idx++;
    N--;
  }
}

/*
** Return the size of the Nth element of the cell array
*/
நிலையான SQLITE_NOINLINE u16 computeCellSize(CellArray *p, முழு N){
  ப்ரதிஜ்ஞா( N>=0 && N<p->nCell );
  ப்ரதிஜ்ஞா( p->szCell[N]==0 );
  p->szCell[N] = p->pRef->xCellSize(p->pRef, p->apCell[N]);
  திரும்பு p->szCell[N];
}
நிலையான u16 cachedCellSize(CellArray *p, முழு N){
  ப்ரதிஜ்ஞா( N>=0 && N<p->nCell );
  எனில்( p->szCell[N] ) திரும்பு p->szCell[N];
  திரும்பு computeCellSize(p, N);
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
நிலையான முழு rebuildPage(
  CellArray *pCArray,             /* Content to be added to page pPg */
  முழு iFirst,                     /* First cell in pCArray to use */
  முழு nCell,                      /* Final number of cells on page */
  MemPage *pPg                    /* The page to be reconstructed */
){
  மாறிலி முழு hdr = pPg->hdrOffset;          /* Offset of header on pPg */
  u8 * மாறிலி aData = pPg->aData;           /* Pointer to data for pPg */
  மாறிலி முழு usableSize = pPg->pBt->usableSize;
  u8 * மாறிலி pEnd = &aData[usableSize];
  முழு i = iFirst;                 /* Which cell to copy from pCArray*/
  u32 j;                          /* Start of cell content area */
  முழு iEnd = i+nCell;             /* Loop terminator */
  u8 *pCellptr = pPg->aCellIdx;
  u8 *pTmp = sqlite3PagerTempSpace(pPg->pBt->pPager);
  u8 *pData;
  முழு k;                          /* Current slot in pCArray->apEnd[] */
  u8 *pSrcEnd;                    /* Current pCArray->apEnd[k] value */

  ப்ரதிஜ்ஞா( nCell>0 );
  ப்ரதிஜ்ஞா( i<iEnd );
  j = get2byte(&aData[hdr+5]);
  எனில்( j>(u32)usableSize ){ j = 0; }
  memcpy(&pTmp[j], &aData[j], usableSize - j);

  ப்ரதிஜ்ஞா( pCArray->ixNx[NB*2-1]>i );
  க்கு(k=0; pCArray->ixNx[k]<=i; k++){}
  pSrcEnd = pCArray->apEnd[k];

  pData = pEnd;
  போது( 1/*exit by break*/ ){
    u8 *pCell = pCArray->apCell[i];
    u16 sz = pCArray->szCell[i];
    ப்ரதிஜ்ஞா( sz>0 );
    எனில்( SQLITE_WITHIN(pCell,aData+j,pEnd) ){
      எனில்( ((uptr)(pCell+sz))>(uptr)pEnd ) திரும்பு SQLITE_CORRUPT_BKPT;
      pCell = &pTmp[pCell - aData];
    }இல்லையெனில் எனில்( (uptr)(pCell+sz)>(uptr)pSrcEnd
           && (uptr)(pCell)<(uptr)pSrcEnd
    ){
      திரும்பு SQLITE_CORRUPT_BKPT;
    }

    pData -= sz;
    put2byte(pCellptr, (pData - aData));
    pCellptr += 2;
    எனில்( pData < pCellptr ) திரும்பு SQLITE_CORRUPT_BKPT;
    memmove(pData, pCell, sz);
    ப்ரதிஜ்ஞா( sz==pPg->xCellSize(pPg, pCell) || CORRUPT_DB );
    i++;
    எனில்( i>=iEnd ) நிறுத்து;
    எனில்( pCArray->ixNx[k]<=i ){
      k++;
      pSrcEnd = pCArray->apEnd[k];
    }
  }

  /* The pPg->nFree field is now set incorrectly. The caller will fix it. */
  ப்ரதிஜ்ஞா( nCell < 10922 );
  pPg->nCell = (u16)nCell;
  pPg->nOverflow = 0;

  put2byte(&aData[hdr+1], 0);
  put2byte(&aData[hdr+3], pPg->nCell);
  put2byte(&aData[hdr+5], pData - aData);
  aData[hdr+7] = 0x00;
  திரும்பு SQLITE_OK;
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
நிலையான முழு page_செருகு_Array(
  MemPage *pPg,                   /* Page to add cells to */
  u8 *pBegin,                     /* End of cell-pointer array */
  u8 **ppData,                    /* IN/OUT: Page content-area pointer */
  u8 *pCellptr,                   /* Pointer to cell-pointer area */
  முழு iFirst,                     /* Index of first cell to add */
  முழு nCell,                      /* Number of cells to add to pPg */
  CellArray *pCArray              /* Array of cells */
){
  முழு i = iFirst;                 /* Loop counter - cell index to insert */
  u8 *aData = pPg->aData;         /* Complete page */
  u8 *pData = *ppData;            /* Content area.  A subset of aData[] */
  முழு iEnd = iFirst + nCell;      /* End of loop. One past last cell to ins */
  முழு k;                          /* Current slot in pCArray->apEnd[] */
  u8 *pEnd;                       /* Maximum extent of cell data */
  ப்ரதிஜ்ஞா( CORRUPT_DB || pPg->hdrOffset==0 );    /* Never called on page 1 */
  எனில்( iEnd<=iFirst ) திரும்பு 0;
  ப்ரதிஜ்ஞா( pCArray->ixNx[NB*2-1]>i );
  க்கு(k=0; pCArray->ixNx[k]<=i ; k++){}
  pEnd = pCArray->apEnd[k];
  போது( 1 /*Exit by break*/ ){
    முழு sz, rc;
    u8 *pSlot;
    ப்ரதிஜ்ஞா( pCArray->szCell[i]!=0 );
    sz = pCArray->szCell[i];
    எனில்( (aData[1]==0 && aData[2]==0) || (pSlot = pageFindSlot(pPg,sz,&rc))==0 ){
      எனில்( (pData - pBegin)<sz ) திரும்பு 1;
      pData -= sz;
      pSlot = pData;
    }
    /* pSlot and pCArray->apCell[i] will never overlap on a well-formed
    ** database.  But they might for a corrupt database.  Hence use memmove()
    ** since memcpy() sends SIGABORT with overlapping buffers on OpenBSD */
    ப்ரதிஜ்ஞா( (pSlot+sz)<=pCArray->apCell[i]
         || pSlot>=(pCArray->apCell[i]+sz)
         || CORRUPT_DB );
    எனில்( (uptr)(pCArray->apCell[i]+sz)>(uptr)pEnd
     && (uptr)(pCArray->apCell[i])<(uptr)pEnd
    ){
      ப்ரதிஜ்ஞா( CORRUPT_DB );
      (வெறுமை)SQLITE_CORRUPT_BKPT;
      திரும்பு 1;
    }
    memmove(pSlot, pCArray->apCell[i], sz);
    put2byte(pCellptr, (pSlot - aData));
    pCellptr += 2;
    i++;
    எனில்( i>=iEnd ) நிறுத்து;
    எனில்( pCArray->ixNx[k]<=i ){
      k++;
      pEnd = pCArray->apEnd[k];
    }
  }
  *ppData = pData;
  திரும்பு 0;
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
நிலையான முழு pageFreeArray(
  MemPage *pPg,                   /* Page to edit */
  முழு iFirst,                     /* First cell to delete */
  முழு nCell,                      /* Cells to delete */
  CellArray *pCArray              /* Array of cells */
){
  u8 * மாறிலி aData = pPg->aData;
  u8 * மாறிலி pEnd = &aData[pPg->pBt->usableSize];
  u8 * மாறிலி pStart = &aData[pPg->hdrOffset + 8 + pPg->childPtrSize];
  முழு nRet = 0;
  முழு i, j;
  முழு iEnd = iFirst + nCell;
  முழு nFree = 0;
  முழு aOfst[10];
  முழு aAfter[10];

  க்கு(i=iFirst; i<iEnd; i++){
    u8 *pCell = pCArray->apCell[i];
    எனில்( SQLITE_WITHIN(pCell, pStart, pEnd) ){
      முழு sz;
      முழு iAfter;
      முழு iOfst;
      /* No need to use cachedCellSize() here.  The sizes of all cells that
      ** are to be freed have already been computing while deciding which
      ** cells need freeing */
      sz = pCArray->szCell[i];  ப்ரதிஜ்ஞா( sz>0 );
      iOfst = (u16)(pCell - aData);
      iAfter = iOfst+sz;
      க்கு(j=0; j<nFree; j++){
        எனில்( aOfst[j]==iAfter ){
          aOfst[j] = iOfst;
          நிறுத்து;
        }இல்லையெனில் எனில்( aAfter[j]==iOfst ){
          aAfter[j] = iAfter;
          நிறுத்து;
        }
      }
      எனில்( j>=nFree ){
        எனில்( nFree>=(முழு)(அளவு(aOfst)/அளவு(aOfst[0])) ){
          க்கு(j=0; j<nFree; j++){
            freeSpace(pPg, aOfst[j], aAfter[j]-aOfst[j]);
          }
          nFree = 0;
        }
        aOfst[nFree] = iOfst;
        aAfter[nFree] = iAfter;
        எனில்( &aData[iAfter]>pEnd ) திரும்பு 0;
        nFree++;
      }
      nRet++;
    }
  }
  க்கு(j=0; j<nFree; j++){
    freeSpace(pPg, aOfst[j], aAfter[j]-aOfst[j]);
  }
  திரும்பு nRet;
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
நிலையான முழு editPage(
  MemPage *pPg,                   /* Edit this page */
  முழு iOld,                       /* Index of first cell currently on page */
  முழு iNew,                       /* Index of new first cell on page */
  முழு nNew,                       /* Final number of cells on page */
  CellArray *pCArray              /* Array of cells and sizes */
){
  u8 * மாறிலி aData = pPg->aData;
  மாறிலி முழு hdr = pPg->hdrOffset;
  u8 *pBegin = &pPg->aCellIdx[nNew * 2];
  முழு nCell = pPg->nCell;       /* Cells stored on pPg */
  u8 *pData;
  u8 *pCellptr;
  முழு i;
  முழு iOldEnd = iOld + pPg->nCell + pPg->nOverflow;
  முழு iNewEnd = iNew + nNew;

#ifdef SQLITE_DEBUG
  u8 *pTmp = sqlite3PagerTempSpace(pPg->pBt->pPager);
  memcpy(pTmp, aData, pPg->pBt->usableSize);
#endif

  /* Remove cells from the start and end of the page */
  ப்ரதிஜ்ஞா( nCell>=0 );
  எனில்( iOld<iNew ){
    முழு nShift = pageFreeArray(pPg, iOld, iNew-iOld, pCArray);
    எனில்( NEVER(nShift>nCell) ) திரும்பு SQLITE_CORRUPT_BKPT;
    memmove(pPg->aCellIdx, &pPg->aCellIdx[nShift*2], nCell*2);
    nCell -= nShift;
  }
  எனில்( iNewEnd < iOldEnd ){
    முழு nTail = pageFreeArray(pPg, iNewEnd, iOldEnd - iNewEnd, pCArray);
    ப்ரதிஜ்ஞா( nCell>=nTail );
    nCell -= nTail;
  }

  pData = &aData[get2byte(&aData[hdr+5])];
  எனில்( pData<pBegin ) செல் editpage_fail;
  எனில்( NEVER(pData>pPg->aDataEnd) ) செல் editpage_fail;

  /* Add cells to the start of the page */
  எனில்( iNew<iOld ){
    முழு nAdd = MIN(nNew,iOld-iNew);
    ப்ரதிஜ்ஞா( (iOld-iNew)<nNew || nCell==0 || CORRUPT_DB );
    ப்ரதிஜ்ஞா( nAdd>=0 );
    pCellptr = pPg->aCellIdx;
    memmove(&pCellptr[nAdd*2], pCellptr, nCell*2);
    எனில்( page_செருகு_Array(
          pPg, pBegin, &pData, pCellptr,
          iNew, nAdd, pCArray
    ) ) செல் editpage_fail;
    nCell += nAdd;
  }

  /* Add any overflow cells */
  க்கு(i=0; i<pPg->nOverflow; i++){
    முழு iCell = (iOld + pPg->aiOvfl[i]) - iNew;
    எனில்( iCell>=0 && iCell<nNew ){
      pCellptr = &pPg->aCellIdx[iCell * 2];
      எனில்( nCell>iCell ){
        memmove(&pCellptr[2], pCellptr, (nCell - iCell) * 2);
      }
      nCell++;
      cachedCellSize(pCArray, iCell+iNew);
      எனில்( page_செருகு_Array(
            pPg, pBegin, &pData, pCellptr,
            iCell+iNew, 1, pCArray
      ) ) செல் editpage_fail;
    }
  }

  /* Append cells to the end of the page */
  ப்ரதிஜ்ஞா( nCell>=0 );
  pCellptr = &pPg->aCellIdx[nCell*2];
  எனில்( page_செருகு_Array(
        pPg, pBegin, &pData, pCellptr,
        iNew+nCell, nNew-nCell, pCArray
      )
  ){
    செல் editpage_fail;
  }

  ப்ரதிஜ்ஞா( nNew < 10922 );
  pPg->nCell = (u16)nNew;
  pPg->nOverflow = 0;

  put2byte(&aData[hdr+3], pPg->nCell);
  put2byte(&aData[hdr+5], pData - aData);

#ifdef SQLITE_DEBUG
  க்கு(i=0; i<nNew && !CORRUPT_DB; i++){
    u8 *pCell = pCArray->apCell[i+iNew];
    முழு iOff = get2byteAligned(&pPg->aCellIdx[i*2]);
    எனில்( SQLITE_WITHIN(pCell, aData, &aData[pPg->pBt->usableSize]) ){
      pCell = &pTmp[pCell - aData];
    }
    ப்ரதிஜ்ஞா( 0==memcmp(pCell, &aData[iOff],
            pCArray->pRef->xCellSize(pCArray->pRef, pCArray->apCell[i+iNew])) );
  }
#endif

  திரும்பு SQLITE_OK;
 editpage_fail:
  /* Unable to edit this page. Rebuild it from scratch instead. */
  எனில்( nNew<1 ) திரும்பு SQLITE_CORRUPT_BKPT;
  populateCellCache(pCArray, iNew, nNew);
  திரும்பு rebuildPage(pCArray, iNew, nNew, pPg);
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
நிலையான முழு balance_quick(MemPage *pParent, MemPage *pPage, u8 *pSpace){
  BtShared *மாறிலி pBt = pPage->pBt;    /* B-Tree Database */
  MemPage *pNew;                       /* Newly allocated page */
  முழு rc;                              /* Return Code */
  Pgno pgnoNew;                        /* Page number of pNew */

  ப்ரதிஜ்ஞா( sqlite3_mutex_held(pPage->pBt->mutex) );
  ப்ரதிஜ்ஞா( sqlite3PagerIswriteable(pParent->pDbPage) );
  ப்ரதிஜ்ஞா( pPage->nOverflow==1 );
 
  எனில்( pPage->nCell==0 ) திரும்பு SQLITE_CORRUPT_BKPT;  /* dbfuzz001.test */
  ப்ரதிஜ்ஞா( pPage->nFree>=0 );
  ப்ரதிஜ்ஞா( pParent->nFree>=0 );

  /* Allocate a new page. This page will become the right-sibling of
  ** pPage. Make the parent page writable, so that the new divider cell
  ** may be inserted. If both these operations are successful, proceed.
  */
  rc = allocateBtreePage(pBt, &pNew, &pgnoNew, 0, 0);

  எனில்( rc==SQLITE_OK ){

    u8 *pOut = &pSpace[4];
    u8 *pCell = pPage->apOvfl[0];
    u16 szCell = pPage->xCellSize(pPage, pCell);
    u8 *pStop;
    CellArray b;

    ப்ரதிஜ்ஞா( sqlite3PagerIswriteable(pNew->pDbPage) );
    ப்ரதிஜ்ஞா( CORRUPT_DB || pPage->aData[0]==(PTF_INTKEY|PTF_LEAFDATA|PTF_LEAF) );
    zeroPage(pNew, PTF_INTKEY|PTF_LEAFDATA|PTF_LEAF);
    b.nCell = 1;
    b.pRef = pPage;
    b.apCell = &pCell;
    b.szCell = &szCell;
    b.apEnd[0] = pPage->aDataEnd;
    b.ixNx[0] = 2;
    b.ixNx[NB*2-1] = 0x7fffffff;
    rc = rebuildPage(&b, 0, 1, pNew);
    எனில்( NEVER(rc) ){
      releasePage(pNew);
      திரும்பு rc;
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
    எனில்( ISAUTOVACUUM(pBt) ){
      ptrmapPut(pBt, pgnoNew, PTRMAP_BTREE, pParent->pgno, &rc);
      எனில்( szCell>pNew->minLocal ){
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
    போது( (*(pCell++)&0x80) && pCell<pStop );
    pStop = &pCell[9];
    போது( ((*(pOut++) = *(pCell++))&0x80) && pCell<pStop );

    /* Insert the new divider cell into pParent. */
    எனில்( rc==SQLITE_OK ){
      rc = செருகு_Cell(pParent, pParent->nCell, pSpace, (முழு)(pOut-pSpace),
                      0, pPage->pgno);
    }

    /* Set the right-child pointer of pParent to point to the new page. */
    put4byte(&pParent->aData[pParent->hdrOffset+8], pgnoNew);
 
    /* Release the reference to the new page. */
    releasePage(pNew);
  }

  திரும்பு rc;
}
#endif /* SQLITE_OMIT_QUICKBALANCE */

#எனில் 0
/*
** This function does not contribute anything to the operation of SQLite.
** it is sometimes activated temporarily while debugging code responsible
** for setting pointer-map entries.
*/
நிலையான முழு ptrmapCheckPages(MemPage **apPage, முழு nPage){
  முழு i, j;
  க்கு(i=0; i<nPage; i++){
    Pgno n;
    u8 e;
    MemPage *pPage = apPage[i];
    BtShared *pBt = pPage->pBt;
    ப்ரதிஜ்ஞா( pPage->isInit );

    க்கு(j=0; j<pPage->nCell; j++){
      CellInfo info;
      u8 *z;
    
      z = findCell(pPage, j);
      pPage->xParseCell(pPage, z, &info);
      எனில்( info.nLocal<info.nPayload ){
        Pgno ovfl = get4byte(&z[info.nSize-4]);
        ptrmapGet(pBt, ovfl, &e, &n);
        ப்ரதிஜ்ஞா( n==pPage->pgno && e==PTRMAP_OVERFLOW1 );
      }
      எனில்( !pPage->leaf ){
        Pgno child = get4byte(z);
        ptrmapGet(pBt, child, &e, &n);
        ப்ரதிஜ்ஞா( n==pPage->pgno && e==PTRMAP_BTREE );
      }
    }
    எனில்( !pPage->leaf ){
      Pgno child = get4byte(&pPage->aData[pPage->hdrOffset+8]);
      ptrmapGet(pBt, child, &e, &n);
      ப்ரதிஜ்ஞா( n==pPage->pgno && e==PTRMAP_BTREE );
    }
  }
  திரும்பு 1;
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
நிலையான வெறுமை copy_முனை_Content(MemPage *pFrom, MemPage *pTo, முழு *pRC){
  எனில்( (*pRC)==SQLITE_OK ){
    BtShared * மாறிலி pBt = pFrom->pBt;
    u8 * மாறிலி aFrom = pFrom->aData;
    u8 * மாறிலி aTo = pTo->aData;
    முழு மாறிலி iFromHdr = pFrom->hdrOffset;
    முழு மாறிலி iToHdr = ((pTo->pgno==1) ? 100 : 0);
    முழு rc;
    முழு iData;
 
 
    ப்ரதிஜ்ஞா( pFrom->isInit );
    ப்ரதிஜ்ஞா( pFrom->nFree>=iToHdr );
    ப்ரதிஜ்ஞா( get2byte(&aFrom[iFromHdr+5]) <= (முழு)pBt->usableSize );
 
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
    எனில்( rc==SQLITE_OK ) rc = btreeComputeFreeSpace(pTo);
    எனில்( rc!=SQLITE_OK ){
      *pRC = rc;
      திரும்பு;
    }
 
    /* If this is an auto-vacuum database, update the pointer-map entries
    ** for any b-tree or overflow pages that pTo now contains the pointers to.
    */
    எனில்( ISAUTOVACUUM(pBt) ){
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
நிலையான முழு balance_nonroot(
  MemPage *pParent,               /* Parent page of siblings being balanced */
  முழு iParentIdx,                 /* Index of "the page" in pParent */
  u8 *aOvflSpace,                 /* page-size bytes of space for parent ovfl */
  முழு is_வேர்,                     /* True if pParent is a root-page */
  முழு bBulk                       /* True if this call is part of a bulk load */
){
  BtShared *pBt;               /* The whole database */
  முழு nMaxCells = 0;           /* Allocated size of apCell, szCell, aFrom. */
  முழு nNew = 0;                /* Number of pages in apNew[] */
  முழு nOld;                    /* Number of pages in apOld[] */
  முழு i, j, k;                 /* Loop counters */
  முழு nxDiv;                   /* Next divider slot in pParent->aCell[] */
  முழு rc = SQLITE_OK;          /* The return code */
  u16 leafCorrection;          /* 4 if pPage is a leaf.  0 if not */
  முழு leafData;                /* True if pPage is a leaf of a LEAFDATA tree */
  முழு usableSpace;             /* Bytes in pPage beyond the header */
  முழு pageFlags;               /* Value of pPage->aData[0] */
  முழு iSpace1 = 0;             /* First unused byte of aSpace1[] */
  முழு iOvflSpace = 0;          /* First unused byte of aOvflSpace[] */
  u64 szScratch;               /* Size of scratch memory requested */
  MemPage *apOld[NB];          /* pPage and up to two siblings */
  MemPage *apNew[NB+2];        /* pPage and up to NB siblings after balancing */
  u8 *p_வலது;                  /* Location in parent of right-sibling pointer */
  u8 *apDiv[NB-1];             /* Divider cells in pParent */
  முழு cntNew[NB+2];            /* Index in b.paCell[] of cell after i-th page */
  முழு cntOld[NB+2];            /* Old index in b.apCell[] */
  முழு szNew[NB+2];             /* Combined size of cells placed on i-th page */
  u8 *aSpace1;                 /* Space for copies of dividers cells */
  Pgno pgno;                   /* Temp var to store a page number in */
  u8 abDone[NB+2];             /* True after i'th new page is populated */
  Pgno aPgno[NB+2];            /* Page numbers of new pages before shuffling */
  CellArray b;                 /* Parsed information on cells being balanced */

  memset(abDone, 0, அளவு(abDone));
  ப்ரதிஜ்ஞா( அளவு(b) - அளவு(b.ixNx) == offsetof(CellArray,ixNx) );
  memset(&b, 0, அளவு(b)-அளவு(b.ixNx[0]));
  b.ixNx[NB*2-1] = 0x7fffffff;
  pBt = pParent->pBt;
  ப்ரதிஜ்ஞா( sqlite3_mutex_held(pBt->mutex) );
  ப்ரதிஜ்ஞா( sqlite3PagerIswriteable(pParent->pDbPage) );

  /* At this point pParent may have at most one overflow cell. And if
  ** this overflow cell is present, it must be the cell with
  ** index iParentIdx. This scenario comes about when this function
  ** is called (indirectly) from sqlite3BtreeDelete().
  */
  ப்ரதிஜ்ஞா( pParent->nOverflow==0 || pParent->nOverflow==1 );
  ப்ரதிஜ்ஞா( pParent->nOverflow==0 || pParent->aiOvfl[0]==iParentIdx );

  எனில்( !aOvflSpace ){
    திரும்பு SQLITE_NOMEM_BKPT;
  }
  ப்ரதிஜ்ஞா( pParent->nFree>=0 );

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
  எனில்( i<2 ){
    nxDiv = 0;
  }இல்லையெனில்{
    ப்ரதிஜ்ஞா( bBulk==0 || bBulk==1 );
    எனில்( iParentIdx==0 ){                
      nxDiv = 0;
    }இல்லையெனில் எனில்( iParentIdx==i ){
      nxDiv = i-2+bBulk;
    }இல்லையெனில்{
      nxDiv = iParentIdx-1;
    }
    i = 2-bBulk;
  }
  nOld = i+1;
  எனில்( (i+nxDiv-pParent->nOverflow)==pParent->nCell ){
    p_வலது = &pParent->aData[pParent->hdrOffset+8];
  }இல்லையெனில்{
    p_வலது = findCell(pParent, i+nxDiv-pParent->nOverflow);
  }
  pgno = get4byte(p_வலது);
  போது( 1 ){
    எனில்( rc==SQLITE_OK ){
      rc = getAndInitPage(pBt, pgno, &apOld[i], 0);
    }
    எனில்( rc ){
      memset(apOld, 0, (i+1)*அளவு(MemPage*));
      செல் balance_cleanup;
    }
    எனில்( apOld[i]->nFree<0 ){
      rc = btreeComputeFreeSpace(apOld[i]);
      எனில்( rc ){
        memset(apOld, 0, (i)*அளவு(MemPage*));
        செல் balance_cleanup;
      }
    }
    nMaxCells += apOld[i]->nCell + ArraySize(pParent->apOvfl);
    எனில்( (i--)==0 ) நிறுத்து;

    எனில்( pParent->nOverflow && i+nxDiv==pParent->aiOvfl[0] ){
      apDiv[i] = pParent->apOvfl[0];
      pgno = get4byte(apDiv[i]);
      szNew[i] = pParent->xCellSize(pParent, apDiv[i]);
      pParent->nOverflow = 0;
    }இல்லையெனில்{
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
      எனில்( pBt->btsFlags & BTS_FAST_SECURE ){
        முழு iOff;

        /* If the following if() condition is not true, the db is corrupted.
        ** The call to dropCell() below will detect this.  */
        iOff = SQLITE_PTR_TO_INT(apDiv[i]) - SQLITE_PTR_TO_INT(pParent->aData);
        எனில்( (iOff+szNew[i])<=(முழு)pBt->usableSize ){
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
       nMaxCells*அளவு(u8*)                       /* b.apCell */
     + nMaxCells*அளவு(u16)                       /* b.szCell */
     + pBt->pageSize;                              /* aSpace1 */

  ப்ரதிஜ்ஞா( szScratch<=7*(முழு)pBt->pageSize );
  b.apCell = sqlite3_அடுக்கு_Alloc_Raw(0, szScratch );
  எனில்( b.apCell==0 ){
    rc = SQLITE_NOMEM_BKPT;
    செல் balance_cleanup;
  }
  b.szCell = (u16*)&b.apCell[nMaxCells];
  aSpace1 = (u8*)&b.szCell[nMaxCells];
  ப்ரதிஜ்ஞா( EIGHT_BYTE_ALIGNMENT(aSpace1) );

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
  leafData = b.pRef->int_சாவி_Leaf;
  க்கு(i=0; i<nOld; i++){
    MemPage *pOld = apOld[i];
    முழு limit = pOld->nCell;
    u8 *aData = pOld->aData;
    u16 maskPage = pOld->maskPage;
    u8 *piCell = aData + pOld->cellOffset;
    u8 *piEnd;
    u8 **ppCell;
    VVA_ONLY( முழு nCellAtStart = b.nCell; )

    /* Verify that all sibling pages are of the same "type" (table-leaf,
    ** table-interior, index-leaf, or index-interior).
    */
    எனில்( pOld->aData[0]!=apOld[0]->aData[0] ){
      rc = SQLITE_CORRUPT_PAGE(pOld);
      செல் balance_cleanup;
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
    memset(&b.szCell[b.nCell], 0, அளவு(b.szCell[0])*(limit+pOld->nOverflow));
    எனில்( pOld->nOverflow>0 ){
      எனில்( NEVER(limit<pOld->aiOvfl[0]) ){
        rc = SQLITE_CORRUPT_PAGE(pOld);
        செல் balance_cleanup;
      }
      limit = pOld->aiOvfl[0];
      க்கு(j=0; j<limit; j++){
        b.apCell[b.nCell+j] = aData + (maskPage & get2byteAligned(piCell));
        piCell += 2;
      }
      b.nCell += limit;
      க்கு(k=0; k<pOld->nOverflow; k++){
        ப்ரதிஜ்ஞா( k==0 || pOld->aiOvfl[k-1]+1==pOld->aiOvfl[k] );/* NOTE 1 */
        b.apCell[b.nCell] = pOld->apOvfl[k];
        b.nCell++;
      }
    }
    piEnd = aData + pOld->cellOffset + 2*pOld->nCell;
    /* Collect cell pointers before updating the cell count. */
    ppCell = &b.apCell[b.nCell];
    போது( piCell<piEnd ){
      ப்ரதிஜ்ஞா( ppCell<&b.apCell[nMaxCells] );
      *ppCell++ = aData + (maskPage & get2byteAligned(piCell));
      piCell += 2;
    }
    b.nCell = (முழு)(ppCell - b.apCell);
    ப்ரதிஜ்ஞா( (b.nCell-nCellAtStart)==(pOld->nCell+pOld->nOverflow) );

    cntOld[i] = b.nCell;
    எனில்( i<nOld-1 && !leafData){
      u16 sz = (u16)szNew[i];
      u8 *pTemp;
      ப்ரதிஜ்ஞா( b.nCell<nMaxCells );
      b.szCell[b.nCell] = sz;
      pTemp = &aSpace1[iSpace1];
      iSpace1 += sz;
      ப்ரதிஜ்ஞா( sz<=pBt->maxLocal+23 );
      ப்ரதிஜ்ஞா( iSpace1 <= (முழு)pBt->pageSize );
      memcpy(pTemp, apDiv[i], sz);
      b.apCell[b.nCell] = pTemp+leafCorrection;
      ப்ரதிஜ்ஞா( leafCorrection==0 || leafCorrection==4 );
      b.szCell[b.nCell] = b.szCell[b.nCell] - leafCorrection;
      எனில்( !pOld->leaf ){
        ப்ரதிஜ்ஞா( leafCorrection==0 );
        ப்ரதிஜ்ஞா( pOld->hdrOffset==0 || CORRUPT_DB );
        /* The right pointer of the child page pOld becomes the left
        ** pointer of the divider cell */
        memcpy(b.apCell[b.nCell], &pOld->aData[8], 4);
      }இல்லையெனில்{
        ப்ரதிஜ்ஞா( leafCorrection==4 );
        போது( b.szCell[b.nCell]<4 ){
          /* Do not allow any cells smaller than 4 bytes. If a smaller cell
          ** does exist, pad it with 0x00 bytes. */
          ப்ரதிஜ்ஞா( b.szCell[b.nCell]==3 || CORRUPT_DB );
          ப்ரதிஜ்ஞா( b.apCell[b.nCell]==&aSpace1[iSpace1-3] || CORRUPT_DB );
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
  க்கு(i=k=0; i<nOld; i++, k++){
    MemPage *p = apOld[i];
    b.apEnd[k] = p->aDataEnd;
    b.ixNx[k] = cntOld[i];
    எனில்( k && b.ixNx[k]==b.ixNx[k-1] ){
      k--;  /* Omit b.ixNx[] entry for child pages with no cells */
    }
    எனில்( !leafData ){
      k++;
      b.apEnd[k] = pParent->aDataEnd;
      b.ixNx[k] = cntOld[i]+1;
    }
    ப்ரதிஜ்ஞா( p->nFree>=0 );
    szNew[i] = usableSpace - p->nFree;
    க்கு(j=0; j<p->nOverflow; j++){
      szNew[i] += 2 + p->xCellSize(p, p->apOvfl[j]);
    }
    cntNew[i] = cntOld[i];
  }
  k = nOld;
  க்கு(i=0; i<k; i++){
    முழு sz;
    போது( szNew[i]>usableSpace ){
      எனில்( i+1>=k ){
        k = i+2;
        எனில்( k>NB+2 ){ rc = SQLITE_CORRUPT_BKPT; செல் balance_cleanup; }
        szNew[k-1] = 0;
        cntNew[k-1] = b.nCell;
      }
      sz = 2 + cachedCellSize(&b, cntNew[i]-1);
      szNew[i] -= sz;
      எனில்( !leafData ){
        எனில்( cntNew[i]<b.nCell ){
          sz = 2 + cachedCellSize(&b, cntNew[i]);
        }இல்லையெனில்{
          sz = 0;
        }
      }
      szNew[i+1] += sz;
      cntNew[i]--;
    }
    போது( cntNew[i]<b.nCell ){
      sz = 2 + cachedCellSize(&b, cntNew[i]);
      எனில்( szNew[i]+sz>usableSpace ) நிறுத்து;
      szNew[i] += sz;
      cntNew[i]++;
      எனில்( !leafData ){
        எனில்( cntNew[i]<b.nCell ){
          sz = 2 + cachedCellSize(&b, cntNew[i]);
        }இல்லையெனில்{
          sz = 0;
        }
      }
      szNew[i+1] -= sz;
    }
    எனில்( cntNew[i]>=b.nCell ){
      k = i+1;
    }இல்லையெனில் எனில்( cntNew[i] <= (i>0 ? cntNew[i-1] : 0) ){
      rc = SQLITE_CORRUPT_BKPT;
      செல் balance_cleanup;
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
  க்கு(i=k-1; i>0; i--){
    முழு sz_வலது = szNew[i];  /* Size of sibling on the right */
    முழு sz_இடது = szNew[i-1]; /* Size of sibling on the left */
    முழு r;              /* Index of right-most cell in left sibling */
    முழு d;              /* Index of first cell to the left of right sibling */

    r = cntNew[i-1] - 1;
    d = r + 1 - leafData;
    (வெறுமை)cachedCellSize(&b, d);
    செய்{
      முழு szR, szD;
      ப்ரதிஜ்ஞா( d<nMaxCells );
      ப்ரதிஜ்ஞா( r<nMaxCells );
      szR = cachedCellSize(&b, r);
      szD = b.szCell[d];
      எனில்( sz_வலது!=0
       && (bBulk || sz_வலது+szD+2 > sz_இடது-(szR+(i==k-1?0:2)))){
        நிறுத்து;
      }
      sz_வலது += szD + 2;
      sz_இடது -= szR + 2;
      cntNew[i-1] = r;
      r--;
      d--;
    }போது( r>=0 );
    szNew[i] = sz_வலது;
    szNew[i-1] = sz_இடது;
    எனில்( cntNew[i-1] <= (i>1 ? cntNew[i-2] : 0) ){
      rc = SQLITE_CORRUPT_BKPT;
      செல் balance_cleanup;
    }
  }

  /* Sanity check:  For a non-corrupt database file one of the following
  ** must be true:
  **    (1) We found one or more cells (cntNew[0])>0), or
  **    (2) pPage is a virtual root page.  A virtual root page is when
  **        the real root page is page 1 and we are the only child of
  **        that page.
  */
  ப்ரதிஜ்ஞா( cntNew[0]>0 || (pParent->pgno==1 && pParent->nCell==0) || CORRUPT_DB);
  TRACE(("BALANCE: old: %u(nc=%u) %u(nc=%u) %u(nc=%u)\n",
    apOld[0]->pgno, apOld[0]->nCell,
    nOld>=2 ? apOld[1]->pgno : 0, nOld>=2 ? apOld[1]->nCell : 0,
    nOld>=3 ? apOld[2]->pgno : 0, nOld>=3 ? apOld[2]->nCell : 0
  ));

  /*
  ** Allocate k new pages.  Reuse old pages where possible.
  */
  pageFlags = apOld[0]->aData[0];
  க்கு(i=0; i<k; i++){
    MemPage *pNew;
    எனில்( i<nOld ){
      pNew = apNew[i] = apOld[i];
      apOld[i] = 0;
      rc = sqlite3PagerWrite(pNew->pDbPage);
      nNew++;
      எனில்( sqlite3PagerPageRefcount(pNew->pDbPage)!=1+(i==(iParentIdx-nxDiv))
       && rc==SQLITE_OK
      ){
        rc = SQLITE_CORRUPT_BKPT;
      }
      எனில்( rc ) செல் balance_cleanup;
    }இல்லையெனில்{
      ப்ரதிஜ்ஞா( i>0 );
      rc = allocateBtreePage(pBt, &pNew, &pgno, (bBulk ? 1 : pgno), 0);
      எனில்( rc ) செல் balance_cleanup;
      zeroPage(pNew, pageFlags);
      apNew[i] = pNew;
      nNew++;
      cntOld[i] = b.nCell;

      /* Set the pointer-map entry for the new sibling page. */
      எனில்( ISAUTOVACUUM(pBt) ){
        ptrmapPut(pBt, pNew->pgno, PTRMAP_BTREE, pParent->pgno, &rc);
        எனில்( rc!=SQLITE_OK ){
          செல் balance_cleanup;
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
  க்கு(i=0; i<nNew; i++){
    aPgno[i] = apNew[i]->pgno;
    ப்ரதிஜ்ஞா( apNew[i]->pDbPage->flags & PGHDR_WRITEABLE );
    ப்ரதிஜ்ஞா( apNew[i]->pDbPage->flags & PGHDR_DIRTY );
  }
  க்கு(i=0; i<nNew-1; i++){
    முழு iB = i;
    க்கு(j=i+1; j<nNew; j++){
      எனில்( apNew[j]->pgno < apNew[iB]->pgno ) iB = j;
    }

    /* If apNew[i] has a page number that is bigger than any of the
    ** subsequence apNew[i] entries, then swap apNew[i] with the subsequent
    ** entry that has the smallest page number (which we know to be
    ** entry apNew[iB]).
    */
    எனில்( iB!=i ){
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

  ப்ரதிஜ்ஞா( sqlite3PagerIswriteable(pParent->pDbPage) );
  ப்ரதிஜ்ஞா( nNew>=1 && nNew<=ArraySize(apNew) );
  ப்ரதிஜ்ஞா( apNew[nNew-1]!=0 );
  put4byte(p_வலது, apNew[nNew-1]->pgno);

  /* If the sibling pages are not leaves, ensure that the right-child pointer
  ** of the right-most new sibling page is set to the value that was
  ** originally in the same field of the right-most old sibling page. */
  எனில்( (pageFlags & PTF_LEAF)==0 && nOld!=nNew ){
    MemPage *pOld;
    எனில்( nNew>nOld ){
      pOld = apNew[nOld-1];
    }இல்லையெனில்{
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
  எனில்( ISAUTOVACUUM(pBt) ){
    MemPage *pOld;
    MemPage *pNew = pOld = apNew[0];
    முழு cntOldNext = pNew->nCell + pNew->nOverflow;
    முழு iNew = 0;
    முழு iOld = 0;

    க்கு(i=0; i<b.nCell; i++){
      u8 *pCell = b.apCell[i];
      போது( i==cntOldNext ){
        iOld++;
        ப்ரதிஜ்ஞா( iOld<nNew || iOld<nOld );
        ப்ரதிஜ்ஞா( iOld>=0 && iOld<NB );
        pOld = iOld<nNew ? apNew[iOld] : apOld[iOld];
        cntOldNext += pOld->nCell + pOld->nOverflow + !leafData;
      }
      எனில்( i==cntNew[iNew] ){
        pNew = apNew[++iNew];
        எனில்( !leafData ) தொடர்;
      }

      /* Cell pCell is destined for new sibling page pNew. Originally, it
      ** was either part of sibling page iOld (possibly an overflow cell),
      ** or else the divider cell to the left of sibling page iOld. So,
      ** if sibling page iOld had the same page number as pNew, and if
      ** pCell really was a part of sibling page iOld (not a divider or
      ** overflow cell), we can skip updating the pointer map entries.  */
      எனில்( iOld>=nNew
       || pNew->pgno!=aPgno[iOld]
       || !SQLITE_WITHIN(pCell,pOld->aData,pOld->aDataEnd)
      ){
        எனில்( !leafCorrection ){
          ptrmapPut(pBt, get4byte(pCell), PTRMAP_BTREE, pNew->pgno, &rc);
        }
        எனில்( cachedCellSize(&b,i)>pNew->minLocal ){
          ptrmapPutOvflPtr(pNew, pOld, pCell, &rc);
        }
        எனில்( rc ) செல் balance_cleanup;
      }
    }
  }

  /* Insert new divider cells into pParent. */
  க்கு(i=0; i<nNew-1; i++){
    u8 *pCell;
    u8 *pTemp;
    முழு sz;
    u8 *pSrcEnd;
    MemPage *pNew = apNew[i];
    j = cntNew[i];

    ப்ரதிஜ்ஞா( j<nMaxCells );
    ப்ரதிஜ்ஞா( b.apCell[j]!=0 );
    pCell = b.apCell[j];
    sz = b.szCell[j] + leafCorrection;
    pTemp = &aOvflSpace[iOvflSpace];
    எனில்( !pNew->leaf ){
      memcpy(&pNew->aData[8], pCell, 4);
    }இல்லையெனில் எனில்( leafData ){
      /* If the tree is a leaf-data tree, and the siblings are leaves,
      ** then there is no divider cell in b.apCell[]. Instead, the divider
      ** cell consists of the integer key for the right-most cell of
      ** the sibling-page assembled above only.
      */
      CellInfo info;
      j--;
      pNew->xParseCell(pNew, b.apCell[j], &info);
      pCell = pTemp;
      sz = 4 + sqlite3PutVarint(&pCell[4], info.n_சாவி);
      pTemp = 0;
    }இல்லையெனில்{
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
      எனில்( b.szCell[j]==4 ){
        ப்ரதிஜ்ஞா(leafCorrection==4);
        sz = pParent->xCellSize(pParent, pCell);
      }
    }
    iOvflSpace += sz;
    ப்ரதிஜ்ஞா( sz<=pBt->maxLocal+23 );
    ப்ரதிஜ்ஞா( iOvflSpace <= (முழு)pBt->pageSize );
    ப்ரதிஜ்ஞா( b.ixNx[NB*2-1]>j );
    க்கு(k=0; b.ixNx[k]<=j; k++){}
    pSrcEnd = b.apEnd[k];
    எனில்( SQLITE_OVERFLOW(pSrcEnd, pCell, pCell+sz) ){
      rc = SQLITE_CORRUPT_BKPT;
      செல் balance_cleanup;
    }
    rc = செருகு_Cell(pParent, nxDiv+i, pCell, sz, pTemp, pNew->pgno);
    எனில்( rc!=SQLITE_OK ) செல் balance_cleanup;
    ப்ரதிஜ்ஞா( sqlite3PagerIswriteable(pParent->pDbPage) );
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
  க்கு(i=1-nNew; i<nNew; i++){
    முழு iPg = i<0 ? -i : i;
    ப்ரதிஜ்ஞா( iPg>=0 && iPg<nNew );
    ப்ரதிஜ்ஞா( iPg>=1 || i>=0 );
    ப்ரதிஜ்ஞா( iPg<ArraySize(cntOld) );
    எனில்( abDone[iPg] ) தொடர்;         /* Skip pages already processed */
    எனில்( i>=0                            /* On the upwards pass, or... */
     || cntOld[iPg-1]>=cntNew[iPg-1]    /* Condition (1) is true */
    ){
      முழு iNew;
      முழு iOld;
      முழு nNewCell;

      /* Verify condition (1):  If cells are moving left, update iPg
      ** only after iPg-1 has already been updated. */
      ப்ரதிஜ்ஞா( iPg==0 || cntOld[iPg-1]>=cntNew[iPg-1] || abDone[iPg-1] );

      /* Verify condition (2):  If cells are moving right, update iPg
      ** only after iPg+1 has already been updated. */
      ப்ரதிஜ்ஞா( cntNew[iPg]>=cntOld[iPg] || abDone[iPg+1] );

      எனில்( iPg==0 ){
        iNew = iOld = 0;
        nNewCell = cntNew[0];
      }இல்லையெனில்{
        iOld = iPg<nOld ? (cntOld[iPg-1] + !leafData) : b.nCell;
        iNew = cntNew[iPg-1] + !leafData;
        nNewCell = cntNew[iPg] - iNew;
      }

      rc = editPage(apNew[iPg], iOld, iNew, nNewCell, &b);
      எனில்( rc ) செல் balance_cleanup;
      abDone[iPg]++;
      apNew[iPg]->nFree = usableSpace-szNew[iPg];
      ப்ரதிஜ்ஞா( apNew[iPg]->nOverflow==0 );
      ப்ரதிஜ்ஞா( apNew[iPg]->nCell==nNewCell );
    }
  }

  /* All pages have been processed exactly once */
  ப்ரதிஜ்ஞா( memcmp(abDone, "\01\01\01\01\01", nNew)==0 );

  ப்ரதிஜ்ஞா( nOld>0 );
  ப்ரதிஜ்ஞா( nNew>0 );

  எனில்( is_வேர் && pParent->nCell==0 && pParent->hdrOffset<=apNew[0]->nFree ){
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
    ப்ரதிஜ்ஞா( nNew==1 || CORRUPT_DB );
    rc = defragmentPage(apNew[0], -1);
    testcase( rc!=SQLITE_OK );
    ப்ரதிஜ்ஞா( apNew[0]->nFree ==
        (get2byteNotZero(&apNew[0]->aData[5]) - apNew[0]->cellOffset
          - apNew[0]->nCell*2)
      || rc!=SQLITE_OK
    );
    copy_முனை_Content(apNew[0], pParent, &rc);
    freePage(apNew[0], &rc);
  }இல்லையெனில் எனில்( ISAUTOVACUUM(pBt) && !leafCorrection ){
    /* Fix the pointer map entries associated with the right-child of each
    ** sibling page. All other pointer map entries have already been taken
    ** care of.  */
    க்கு(i=0; i<nNew; i++){
      u32 சாவி = get4byte(&apNew[i]->aData[8]);
      ptrmapPut(pBt, சாவி, PTRMAP_BTREE, apNew[i]->pgno, &rc);
    }
  }

  ப்ரதிஜ்ஞா( pParent->isInit );
  TRACE(("BALANCE: finished: old=%u new=%u cells=%u\n",
          nOld, nNew, b.nCell));

  /* Free any old pages that were not reused as new pages.
  */
  க்கு(i=nNew; i<nOld; i++){
    freePage(apOld[i], &rc);
  }

#எனில் 0
  எனில்( ISAUTOVACUUM(pBt) && rc==SQLITE_OK && apNew[0]->isInit ){
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
  sqlite3_அடுக்கு_Free(0, b.apCell);
  க்கு(i=0; i<nOld; i++){
    releasePage(apOld[i]);
  }
  க்கு(i=0; i<nNew; i++){
    releasePage(apNew[i]);
  }

  திரும்பு rc;
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
நிலையான முழு balance_deeper(MemPage *p_வேர், MemPage **ppChild){
  முழு rc;                        /* Return value from subprocedures */
  MemPage *pChild = 0;           /* Pointer to a new child page */
  Pgno pgnoChild = 0;            /* Page number of the new child page */
  BtShared *pBt = p_வேர்->pBt;    /* The BTree */

  ப்ரதிஜ்ஞா( p_வேர்->nOverflow>0 );
  ப்ரதிஜ்ஞா( sqlite3_mutex_held(pBt->mutex) );

  /* Make pRoot, the root page of the b-tree, writable. Allocate a new
  ** page that will become the new right-child of pPage. Copy the contents
  ** of the node stored on pRoot into the new child page.
  */
  rc = sqlite3PagerWrite(p_வேர்->pDbPage);
  எனில்( rc==SQLITE_OK ){
    rc = allocateBtreePage(pBt,&pChild,&pgnoChild,p_வேர்->pgno,0);
    copy_முனை_Content(p_வேர், pChild, &rc);
    எனில்( ISAUTOVACUUM(pBt) ){
      ptrmapPut(pBt, pgnoChild, PTRMAP_BTREE, p_வேர்->pgno, &rc);
    }
  }
  எனில்( rc ){
    *ppChild = 0;
    releasePage(pChild);
    திரும்பு rc;
  }
  ப்ரதிஜ்ஞா( sqlite3PagerIswriteable(pChild->pDbPage) );
  ப்ரதிஜ்ஞா( sqlite3PagerIswriteable(p_வேர்->pDbPage) );
  ப்ரதிஜ்ஞா( pChild->nCell==p_வேர்->nCell || CORRUPT_DB );

  TRACE(("BALANCE: copy root %u into %u\n", p_வேர்->pgno, pChild->pgno));

  /* Copy the overflow cells from pRoot to pChild */
  memcpy(pChild->aiOvfl, p_வேர்->aiOvfl,
         p_வேர்->nOverflow*அளவு(p_வேர்->aiOvfl[0]));
  memcpy(pChild->apOvfl, p_வேர்->apOvfl,
         p_வேர்->nOverflow*அளவு(p_வேர்->apOvfl[0]));
  pChild->nOverflow = p_வேர்->nOverflow;

  /* Zero the contents of pRoot. Then install pChild as the right-child. */
  zeroPage(p_வேர், pChild->aData[0] & ~PTF_LEAF);
  put4byte(&p_வேர்->aData[p_வேர்->hdrOffset+8], pgnoChild);

  *ppChild = pChild;
  திரும்பு SQLITE_OK;
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
நிலையான முழு anotherValidCursor(BtCursor *pCur){
  BtCursor *pOther;
  க்கு(pOther=pCur->pBt->pCursor; pOther; pOther=pOther->pNext){
    எனில்( pOther!=pCur
     && pOther->eState==CURSOR_VALID
     && pOther->pPage==pCur->pPage
    ){
      திரும்பு SQLITE_CORRUPT_PAGE(pCur->pPage);
    }
  }
  திரும்பு SQLITE_OK;
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
நிலையான முழு balance(BtCursor *pCur){
  முழு rc = SQLITE_OK;
  u8 aBalanceQuickSpace[13];
  u8 *pFree = 0;

  VVA_ONLY( முழு balance_quick_called = 0 );
  VVA_ONLY( முழு balance_deeper_called = 0 );

  செய் {
    முழு iPage;
    MemPage *pPage = pCur->pPage;

    எனில்( NEVER(pPage->nFree<0) && btreeComputeFreeSpace(pPage) ) நிறுத்து;
    எனில்( pPage->nOverflow==0 && pPage->nFree*3<=(முழு)pCur->pBt->usableSize*2 ){
      /* No rebalance required as long as:
      **   (1) There are no overflow cells
      **   (2) The amount of free space on the page is less than 2/3rds of
      **       the total usable space on the page. */
      நிறுத்து;
    }இல்லையெனில் எனில்( (iPage = pCur->iPage)==0 ){
      எனில்( pPage->nOverflow && (rc = anotherValidCursor(pCur))==SQLITE_OK ){
        /* The root page of the b-tree is overfull. In this case call the
        ** balance_deeper() function to create a new child for the root-page
        ** and copy the current contents of the root-page to it. The
        ** next iteration of the do-loop will balance the child page.
        */
        ப்ரதிஜ்ஞா( balance_deeper_called==0 );
        VVA_ONLY( balance_deeper_called++ );
        rc = balance_deeper(pPage, &pCur->apPage[1]);
        எனில்( rc==SQLITE_OK ){
          pCur->iPage = 1;
          pCur->ix = 0;
          pCur->aiIdx[0] = 0;
          pCur->apPage[0] = pPage;
          pCur->pPage = pCur->apPage[1];
          ப்ரதிஜ்ஞா( pCur->pPage->nOverflow );
        }
      }இல்லையெனில்{
        நிறுத்து;
      }
    }இல்லையெனில் எனில்( sqlite3PagerPageRefcount(pPage->pDbPage)>1 ){
      /* The page being written is not a root page, and there is currently
      ** more than one reference to it. This only happens if the page is one
      ** of its own ancestor pages. Corruption. */
      rc = SQLITE_CORRUPT_PAGE(pPage);
    }இல்லையெனில்{
      MemPage * மாறிலி pParent = pCur->apPage[iPage-1];
      முழு மாறிலி iIdx = pCur->aiIdx[iPage-1];

      rc = sqlite3PagerWrite(pParent->pDbPage);
      எனில்( rc==SQLITE_OK && pParent->nFree<0 ){
        rc = btreeComputeFreeSpace(pParent);
      }
      எனில்( rc==SQLITE_OK ){
#ifndef SQLITE_OMIT_QUICKBALANCE
        எனில்( pPage->int_சாவி_Leaf
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
          ப்ரதிஜ்ஞா( balance_quick_called==0 );
          VVA_ONLY( balance_quick_called++ );
          rc = balance_quick(pParent, pPage, aBalanceQuickSpace);
        }இல்லையெனில்
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
          எனில்( pFree ){
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
      ப்ரதிஜ்ஞா( pCur->iPage>=0 );
      pCur->pPage = pCur->apPage[pCur->iPage];
    }
  }போது( rc==SQLITE_OK );

  எனில்( pFree ){
    sqlite3PageFree(pFree);
  }
  திரும்பு rc;
}

/* Overwrite content from pX into pDest.  Only do the write if the
** content is different from what is already there.
*/
நிலையான முழு btreeOverwriteContent(
  MemPage *pPage,           /* MemPage on which writing will occur */
  u8 *pDest,                /* Pointer to the place to start writing */
  மாறிலி BtreePayload *pX,   /* Source of data to write */
  முழு iOffset,              /* Offset of first byte to write */
  முழு iAmt                  /* Number of bytes to be written */
){
  முழு nData = pX->nData - iOffset;
  எனில்( nData<=0 ){
    /* Overwriting with zeros */
    முழு i;
    க்கு(i=0; i<iAmt && pDest[i]==0; i++){}
    எனில்( i<iAmt ){
      முழு rc = sqlite3PagerWrite(pPage->pDbPage);
      எனில்( rc ) திரும்பு rc;
      memset(pDest + i, 0, iAmt - i);
    }
  }இல்லையெனில்{
    எனில்( nData<iAmt ){
      /* Mixed read data and zeros at the end.  Make a recursive call
      ** to write the zeros then fall through to write the real data */
      முழு rc = btreeOverwriteContent(pPage, pDest+nData, pX, iOffset+nData,
                                 iAmt-nData);
      எனில்( rc ) திரும்பு rc;
      iAmt = nData;
    }
    எனில்( memcmp(pDest, ((u8*)pX->pData) + iOffset, iAmt)!=0 ){
      முழு rc = sqlite3PagerWrite(pPage->pDbPage);
      எனில்( rc ) திரும்பு rc;
      /* In a corrupt database, it is possible for the source and destination
      ** buffers to overlap.  This is harmless since the database is already
      ** corrupt but it does cause valgrind and ASAN warnings.  So use
      ** memmove(). */
      memmove(pDest, ((u8*)pX->pData) + iOffset, iAmt);
    }
  }
  திரும்பு SQLITE_OK;
}

/*
** Overwrite the cell that cursor pCur is pointing to with fresh content
** contained in pX.  In this variant, pCur is pointing to an overflow
** cell.
*/
நிலையான SQLITE_NOINLINE முழு btreeOverwriteOverflowCell(
  BtCursor *pCur,                     /* Cursor pointing to cell to overwrite */
  மாறிலி BtreePayload *pX              /* Content to write into the cell */
){
  முழு iOffset;                        /* Next byte of pX->pData to write */
  முழு nTotal = pX->nData + pX->nZero; /* Total bytes of to write */
  முழு rc;                             /* Return code */
  MemPage *pPage = pCur->pPage;       /* Page being written */
  BtShared *pBt;                      /* Btree */
  Pgno ovflPgno;                      /* Next overflow page to write */
  u32 ovflPageSize;                   /* Size to write on overflow page */

  ப்ரதிஜ்ஞா( pCur->info.nLocal<nTotal );  /* pCur is an overflow cell */

  /* Overwrite the local portion first */
  rc = btreeOverwriteContent(pPage, pCur->info.pPayload, pX,
                             0, pCur->info.nLocal);
  எனில்( rc ) திரும்பு rc;

  /* Now overwrite the overflow pages */
  iOffset = pCur->info.nLocal;
  ப்ரதிஜ்ஞா( nTotal>=0 );
  ப்ரதிஜ்ஞா( iOffset>=0 );
  ovflPgno = get4byte(pCur->info.pPayload + iOffset);
  pBt = pPage->pBt;
  ovflPageSize = pBt->usableSize - 4;
  செய்{
    rc = btreeGetPage(pBt, ovflPgno, &pPage, 0);
    எனில்( rc ) திரும்பு rc;
    எனில்( sqlite3PagerPageRefcount(pPage->pDbPage)!=1 || pPage->isInit ){
      rc = SQLITE_CORRUPT_PAGE(pPage);
    }இல்லையெனில்{
      எனில்( iOffset+ovflPageSize<(u32)nTotal ){
        ovflPgno = get4byte(pPage->aData);
      }இல்லையெனில்{
        ovflPageSize = nTotal - iOffset;
      }
      rc = btreeOverwriteContent(pPage, pPage->aData+4, pX,
                                 iOffset, ovflPageSize);
    }
    sqlite3PagerUnref(pPage->pDbPage);
    எனில்( rc ) திரும்பு rc;
    iOffset += ovflPageSize;
  }போது( iOffset<nTotal );
  திரும்பு SQLITE_OK;   
}

/*
** Overwrite the cell that cursor pCur is pointing to with fresh content
** contained in pX.
*/
நிலையான முழு btreeOverwriteCell(BtCursor *pCur, மாறிலி BtreePayload *pX){
  முழு nTotal = pX->nData + pX->nZero; /* Total bytes of to write */
  MemPage *pPage = pCur->pPage;       /* Page being written */

  எனில்( pCur->info.pPayload + pCur->info.nLocal > pPage->aDataEnd
   || pCur->info.pPayload < pPage->aData + pPage->cellOffset
  ){
    திரும்பு SQLITE_CORRUPT_PAGE(pPage);
  }
  எனில்( pCur->info.nLocal==nTotal ){
    /* The entire cell is local */
    திரும்பு btreeOverwriteContent(pPage, pCur->info.pPayload, pX,
                                 0, pCur->info.nLocal);
  }இல்லையெனில்{
    /* The cell contains overflow content */
    திரும்பு btreeOverwriteOverflowCell(pCur, pX);
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
முழு sqlite3_Btree_செருகு(
  BtCursor *pCur,                /* Insert data into the table of this cursor */
  மாறிலி BtreePayload *pX,        /* Content of the row to be inserted */
  முழு flags,                     /* True if this is likely an append */
  முழு seekResult                 /* Result of prior IndexMoveto() call */
){
  முழு rc;
  முழு loc = seekResult;          /* -1: before desired location  +1: after */
  முழு szNew = 0;
  முழு idx;
  MemPage *pPage;
  Btree *p = pCur->pBtree;
  unsigned எழுத்து *oldCell;
  unsigned எழுத்து *newCell = 0;

  ப்ரதிஜ்ஞா( (flags & (BTREE_SAVEPOSITION|BTREE_APPEND|BTREE_PREFORMAT))==flags );
  ப்ரதிஜ்ஞா( (flags & BTREE_PREFORMAT)==0 || seekResult || pCur->p_சாவி_Info==0 );

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
  எனில்( pCur->curFlags & BTCF_Multiple ){
    rc = saveAllCursors(p->pBt, pCur->pgno_வேர், pCur);
    எனில்( rc ) திரும்பு rc;
    எனில்( loc && pCur->iPage<0 ){
      /* This can only happen if the schema is corrupt such that there is more
      ** than one table or index with the same root page as used by the cursor.
      ** Which can only happen if the SQLITE_NoSchemaError flag was set when
      ** the schema was loaded. This cannot be asserted though, as a user might
      ** set the flag, load the schema, and then unset the flag.  */
      திரும்பு SQLITE_CORRUPT_PGNO(pCur->pgno_வேர்);
    }
  }

  /* Ensure that the cursor is not in the CURSOR_FAULT state and that it
  ** points to a valid cell.
  */
  எனில்( pCur->eState>=CURSOR_REQUIRESEEK ){
    testcase( pCur->eState==CURSOR_REQUIRESEEK );
    testcase( pCur->eState==CURSOR_FAULT );
    rc = move_To_வேர்(pCur);
    எனில்( rc && rc!=SQLITE_EMPTY ) திரும்பு rc;
  }

  ப்ரதிஜ்ஞா( cursorOwnsBtShared(pCur) );
  ப்ரதிஜ்ஞா( (pCur->curFlags & BTCF_WriteFlag)!=0
              && p->pBt->inTransaction==TRANS_WRITE
              && (p->pBt->btsFlags & BTS_READ_ONLY)==0 );
  ப்ரதிஜ்ஞா( hasSharedCacheTableLock(p, pCur->pgno_வேர், pCur->p_சாவி_Info!=0, 2) );

  /* Assert that the caller has been consistent. If this cursor was opened
  ** expecting an index b-tree, then the caller should be inserting blob
  ** keys with no associated data. If the cursor was opened expecting an
  ** intkey table, the caller should be inserting integer keys with a
  ** blob of associated data.  */
  ப்ரதிஜ்ஞா( (flags & BTREE_PREFORMAT) || (pX->p_சாவி==0)==(pCur->p_சாவி_Info==0) );

  எனில்( pCur->p_சாவி_Info==0 ){
    ப்ரதிஜ்ஞா( pX->p_சாவி==0 );
    /* If this is an insert into a table b-tree, invalidate any incrblob
    ** cursors open on the row being replaced */
    எனில்( p->hasIncrblobCur ){
      invalidateIncrblobCursors(p, pCur->pgno_வேர், pX->n_சாவி, 0);
    }

    /* If BTREE_SAVEPOSITION is set, the cursor must already be pointing
    ** to a row with the same key as the new entry being inserted.
    */
#ifdef SQLITE_DEBUG
    எனில்( flags & BTREE_SAVEPOSITION ){
      ப்ரதிஜ்ஞா( pCur->curFlags & BTCF_ValidNKey );
      ப்ரதிஜ்ஞா( pX->n_சாவி==pCur->info.n_சாவி );
      ப்ரதிஜ்ஞா( loc==0 );
    }
#endif

    /* On the other hand, BTREE_SAVEPOSITION==0 does not imply
    ** that the cursor is not pointing to a row to be overwritten.
    ** So do a complete check.
    */
    எனில்( (pCur->curFlags&BTCF_ValidNKey)!=0 && pX->n_சாவி==pCur->info.n_சாவி ){
      /* The cursor is pointing to the entry that is to be
      ** overwritten */
      ப்ரதிஜ்ஞா( pX->nData>=0 && pX->nZero>=0 );
      எனில்( pCur->info.nSize!=0
       && pCur->info.nPayload==(u32)pX->nData+pX->nZero
      ){
        /* New entry is the same size as the old.  Do an overwrite */
        திரும்பு btreeOverwriteCell(pCur, pX);
      }
      ப்ரதிஜ்ஞா( loc==0 );
    }இல்லையெனில் எனில்( loc==0 ){
      /* The cursor is *not* pointing to the cell to be overwritten, nor
      ** to an adjacent cell.  Move the cursor so that it is pointing either
      ** to the cell to be overwritten or an adjacent cell.
      */
      rc = sqlite3BtreeTableMoveto(pCur, pX->n_சாவி,
               (flags & BTREE_APPEND)!=0, &loc);
      எனில்( rc ) திரும்பு rc;
    }
  }இல்லையெனில்{
    /* This is an index or a WITHOUT ROWID table */

    /* If BTREE_SAVEPOSITION is set, the cursor must already be pointing
    ** to a row with the same key as the new entry being inserted.
    */
    ப்ரதிஜ்ஞா( (flags & BTREE_SAVEPOSITION)==0 || loc==0 );

    /* If the cursor is not already pointing either to the cell to be
    ** overwritten, or if a new cell is being inserted, if the cursor is
    ** not pointing to an immediately adjacent cell, then move the cursor
    ** so that it does.
    */
    எனில்( loc==0 && (flags & BTREE_SAVEPOSITION)==0 ){
      எனில்( pX->nMem ){
        UnpackedRecord r;
        r.p_சாவி_Info = pCur->p_சாவி_Info;
        r.aMem = pX->aMem;
        r.nField = pX->nMem;
        r.default_rc = 0;
        r.eqSeen = 0;
        rc = sqlite3BtreeIndexMoveto(pCur, &r, &loc);
      }இல்லையெனில்{
        rc = btreeMoveto(pCur, pX->p_சாவி, pX->n_சாவி,
                    (flags & BTREE_APPEND)!=0, &loc);
      }
      எனில்( rc ) திரும்பு rc;
    }

    /* If the cursor is currently pointing to an entry to be overwritten
    ** and the new content is the same as as the old, then use the
    ** overwrite optimization.
    */
    எனில்( loc==0 ){
      getCellInfo(pCur);
      எனில்( pCur->info.n_சாவி==pX->n_சாவி ){
        BtreePayload x2;
        x2.pData = pX->p_சாவி;
        x2.nData = (முழு)pX->n_சாவி;  ப்ரதிஜ்ஞா( pX->n_சாவி<=0x7fffffff );
        x2.nZero = 0;
        திரும்பு btreeOverwriteCell(pCur, &x2);
      }
    }
  }
  ப்ரதிஜ்ஞா( pCur->eState==CURSOR_VALID
       || (pCur->eState==CURSOR_INVALID && loc) || CORRUPT_DB );

  pPage = pCur->pPage;
  ப்ரதிஜ்ஞா( pPage->int_சாவி || pX->n_சாவி>=0 || (flags & BTREE_PREFORMAT) );
  ப்ரதிஜ்ஞா( pPage->leaf || !pPage->int_சாவி );
  எனில்( pPage->nFree<0 ){
    எனில்( NEVER(pCur->eState>CURSOR_INVALID) ){
     /* ^^^^^--- due to the moveToRoot() call above */
      rc = SQLITE_CORRUPT_PAGE(pPage);
    }இல்லையெனில்{
      rc = btreeComputeFreeSpace(pPage);
    }
    எனில்( rc ) திரும்பு rc;
  }

  TRACE(("INSERT: table=%u nkey=%lld ndata=%u page=%u %s\n",
          pCur->pgno_வேர், pX->n_சாவி, pX->nData, pPage->pgno,
          loc==0 ? "overwrite" : "new entry"));
  ப்ரதிஜ்ஞா( pPage->isInit || CORRUPT_DB );
  newCell = p->pBt->pTmpSpace;
  ப்ரதிஜ்ஞா( newCell!=0 );
  ப்ரதிஜ்ஞா( BTREE_PREFORMAT==OPFLAG_PREFORMAT );
  எனில்( flags & BTREE_PREFORMAT ){
    rc = SQLITE_OK;
    szNew = p->pBt->nPreformatSize;
    எனில்( szNew<4 ){
      szNew = 4;
      newCell[3] = 0;
    }
    எனில்( ISAUTOVACUUM(p->pBt) && szNew>pPage->maxLocal ){
      CellInfo info;
      pPage->xParseCell(pPage, newCell, &info);
      எனில்( info.nPayload!=info.nLocal ){
        Pgno ovfl = get4byte(&newCell[szNew-4]);
        ptrmapPut(p->pBt, ovfl, PTRMAP_OVERFLOW1, pPage->pgno, &rc);
        எனில்( NEVER(rc) ) செல் end_செருகு;
      }
    }
  }இல்லையெனில்{
    rc = fillInCell(pPage, newCell, pX, &szNew);
    எனில்( rc ) செல் end_செருகு;
  }
  ப்ரதிஜ்ஞா( szNew==pPage->xCellSize(pPage, newCell) );
  ப்ரதிஜ்ஞா( szNew <= MX_CELL_SIZE(p->pBt) );
  idx = pCur->ix;
  pCur->info.nSize = 0;
  எனில்( loc==0 ){
    CellInfo info;
    ப்ரதிஜ்ஞா( idx>=0 );
    எனில்( idx>=pPage->nCell ){
      திரும்பு SQLITE_CORRUPT_PAGE(pPage);
    }
    rc = sqlite3PagerWrite(pPage->pDbPage);
    எனில்( rc ){
      செல் end_செருகு;
    }
    oldCell = findCell(pPage, idx);
    எனில்( !pPage->leaf ){
      memcpy(newCell, oldCell, 4);
    }
    BTREE_CLEAR_CELL(rc, pPage, oldCell, info);
    testcase( pCur->curFlags & BTCF_ValidOvfl );
    invalidateOverflowCache(pCur);
    எனில்( info.nSize==szNew && info.nLocal==info.nPayload
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
      ப்ரதிஜ்ஞா( rc==SQLITE_OK ); /* clearCell never fails when nLocal==nPayload */
      எனில்( oldCell < pPage->aData+pPage->hdrOffset+10 ){
        திரும்பு SQLITE_CORRUPT_PAGE(pPage);
      }
      எனில்( oldCell+szNew > pPage->aDataEnd ){
        திரும்பு SQLITE_CORRUPT_PAGE(pPage);
      }
      memcpy(oldCell, newCell, szNew);
      திரும்பு SQLITE_OK;
    }
    dropCell(pPage, idx, info.nSize, &rc);
    எனில்( rc ) செல் end_செருகு;
  }இல்லையெனில் எனில்( loc<0 && pPage->nCell>0 ){
    ப்ரதிஜ்ஞா( pPage->leaf );
    idx = ++pCur->ix;
    pCur->curFlags &= ~(BTCF_ValidNKey|BTCF_ValidOvfl);
  }இல்லையெனில்{
    ப்ரதிஜ்ஞா( pPage->leaf );
  }
  rc = செருகு_Cell_Fast(pPage, idx, newCell, szNew);
  ப்ரதிஜ்ஞா( pPage->nOverflow==0 || rc==SQLITE_OK );
  ப்ரதிஜ்ஞா( rc!=SQLITE_OK || pPage->nCell>0 || pPage->nOverflow>0 );

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
  எனில்( pPage->nOverflow ){
    ப்ரதிஜ்ஞா( rc==SQLITE_OK );
    pCur->curFlags &= ~(BTCF_ValidNKey|BTCF_ValidOvfl);
    rc = balance(pCur);

    /* Must make sure nOverflow is reset to zero even if the balance()
    ** fails. Internal data structure corruption will result otherwise.
    ** Also, set the cursor state to invalid. This stops saveCursorPosition()
    ** from trying to save the current position of the cursor.  */
    pCur->pPage->nOverflow = 0;
    pCur->eState = CURSOR_INVALID;
    எனில்( (flags & BTREE_SAVEPOSITION) && rc==SQLITE_OK ){
      btreeReleaseAllCursorPages(pCur);
      எனில்( pCur->p_சாவி_Info ){
        ப்ரதிஜ்ஞா( pCur->p_சாவி==0 );
        pCur->p_சாவி = sqlite3Malloc( pX->n_சாவி );
        எனில்( pCur->p_சாவி==0 ){
          rc = SQLITE_NOMEM;
        }இல்லையெனில்{
          memcpy(pCur->p_சாவி, pX->p_சாவி, pX->n_சாவி);
        }
      }
      pCur->eState = CURSOR_REQUIRESEEK;
      pCur->n_சாவி = pX->n_சாவி;
    }
  }
  ப்ரதிஜ்ஞா( pCur->iPage<0 || pCur->pPage->nOverflow==0 );

end_செருகு:
  திரும்பு rc;
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
முழு sqlite3BtreeTransferRow(BtCursor *pDest, BtCursor *pSrc, i64 i_சாவி){
  BtShared *pBt = pDest->pBt;
  u8 *aOut = pBt->pTmpSpace;    /* Pointer to next output buffer */
  மாறிலி u8 *aIn;                /* Pointer to next input buffer */
  u32 nIn;                      /* Size of input buffer aIn[] */
  u32 nRem;                     /* Bytes of data still to copy */

  getCellInfo(pSrc);
  எனில்( pSrc->info.nPayload<0x80 ){
    *(aOut++) = (u8)pSrc->info.nPayload;
  }இல்லையெனில்{
    aOut += sqlite3PutVarint(aOut, pSrc->info.nPayload);
  }
  எனில்( pDest->p_சாவி_Info==0 ) aOut += sqlite3PutVarint(aOut, i_சாவி);
  nIn = pSrc->info.nLocal;
  aIn = pSrc->info.pPayload;
  எனில்( aIn+nIn>pSrc->pPage->aDataEnd ){
    திரும்பு SQLITE_CORRUPT_PAGE(pSrc->pPage);
  }
  nRem = pSrc->info.nPayload;
  எனில்( nIn==nRem && nIn<pDest->pPage->maxLocal ){
    memcpy(aOut, aIn, nIn);
    pBt->nPreformatSize = nIn + (முழு)(aOut - pBt->pTmpSpace);
    திரும்பு SQLITE_OK;
  }இல்லையெனில்{
    முழு rc = SQLITE_OK;
    Pager *pSrcPager = pSrc->pBt->pPager;
    u8 *pPgnoOut = 0;
    Pgno ovflIn = 0;
    DbPage *pPageIn = 0;
    MemPage *pPageOut = 0;
    u32 nOut;                     /* Size of output buffer aOut[] */

    nOut = btreePayloadToLocal(pDest->pPage, pSrc->info.nPayload);
    pBt->nPreformatSize = (முழு)nOut + (முழு)(aOut - pBt->pTmpSpace);
    எனில்( nOut<pSrc->info.nPayload ){
      pPgnoOut = &aOut[nOut];
      pBt->nPreformatSize += 4;
    }
 
    எனில்( nRem>nIn ){
      எனில்( aIn+nIn+4>pSrc->pPage->aDataEnd ){
        திரும்பு SQLITE_CORRUPT_PAGE(pSrc->pPage);
      }
      ovflIn = get4byte(&pSrc->info.pPayload[nIn]);
    }
 
    செய் {
      nRem -= nOut;
      செய்{
        ப்ரதிஜ்ஞா( nOut>0 );
        எனில்( nIn>0 ){
          முழு nCopy = MIN(nOut, nIn);
          memcpy(aOut, aIn, nCopy);
          nOut -= nCopy;
          nIn -= nCopy;
          aOut += nCopy;
          aIn += nCopy;
        }
        எனில்( nOut>0 ){
          sqlite3PagerUnref(pPageIn);
          pPageIn = 0;
          rc = sqlite3PagerGet(pSrcPager, ovflIn, &pPageIn, PAGER_GET_READONLY);
          எனில்( rc==SQLITE_OK ){
            aIn = (மாறிலி u8*)sqlite3PagerGetData(pPageIn);
            ovflIn = get4byte(aIn);
            aIn += 4;
            nIn = pSrc->pBt->usableSize - 4;
          }
        }
      }போது( rc==SQLITE_OK && nOut>0 );
 
      எனில்( rc==SQLITE_OK && nRem>0 && ALWAYS(pPgnoOut) ){
        Pgno pgnoNew = 0;  /* Prevent harmless static-analyzer warning */
        MemPage *pNew = 0;
        rc = allocateBtreePage(pBt, &pNew, &pgnoNew, 0, 0);
        put4byte(pPgnoOut, pgnoNew);
        எனில்( ISAUTOVACUUM(pBt) && pPageOut ){
          ptrmapPut(pBt, pgnoNew, PTRMAP_OVERFLOW2, pPageOut->pgno, &rc);
        }
        releasePage(pPageOut);
        pPageOut = pNew;
        எனில்( pPageOut ){
          pPgnoOut = pPageOut->aData;
          put4byte(pPgnoOut, 0);
          aOut = &pPgnoOut[4];
          nOut = MIN(pBt->usableSize - 4, nRem);
        }
      }
    }போது( nRem>0 && rc==SQLITE_OK );
 
    releasePage(pPageOut);
    sqlite3PagerUnref(pPageIn);
    திரும்பு rc;
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
முழு sqlite3BtreeDelete(BtCursor *pCur, u8 flags){
  Btree *p = pCur->pBtree;
  BtShared *pBt = p->pBt;             
  முழு rc;                    /* Return code */
  MemPage *pPage;            /* Page to delete cell from */
  unsigned எழுத்து *pCell;      /* Pointer to cell to delete */
  முழு iCellIdx;              /* Index of cell to delete */
  முழு iCellDepth;            /* Depth of node containing pCell */
  CellInfo info;             /* Size of the cell being deleted */
  u8 bPreserve;              /* Keep cursor valid.  2 for CURSOR_SKIPNEXT */

  ப்ரதிஜ்ஞா( cursorOwnsBtShared(pCur) );
  ப்ரதிஜ்ஞா( pBt->inTransaction==TRANS_WRITE );
  ப்ரதிஜ்ஞா( (pBt->btsFlags & BTS_READ_ONLY)==0 );
  ப்ரதிஜ்ஞா( pCur->curFlags & BTCF_WriteFlag );
  ப்ரதிஜ்ஞா( hasSharedCacheTableLock(p, pCur->pgno_வேர், pCur->p_சாவி_Info!=0, 2) );
  ப்ரதிஜ்ஞா( !hasReadConflicts(p, pCur->pgno_வேர்) );
  ப்ரதிஜ்ஞா( (flags & ~(BTREE_SAVEPOSITION | BTREE_AUXDELETE))==0 );
  எனில்( pCur->eState!=CURSOR_VALID ){
    எனில்( pCur->eState>=CURSOR_REQUIRESEEK ){
      rc = btreeRestoreCursorPosition(pCur);
      ப்ரதிஜ்ஞா( rc!=SQLITE_OK || CORRUPT_DB || pCur->eState==CURSOR_VALID );
      எனில்( rc || pCur->eState!=CURSOR_VALID ) திரும்பு rc;
    }இல்லையெனில்{
      திரும்பு SQLITE_CORRUPT_PGNO(pCur->pgno_வேர்);
    }
  }
  ப்ரதிஜ்ஞா( pCur->eState==CURSOR_VALID );

  iCellDepth = pCur->iPage;
  iCellIdx = pCur->ix;
  pPage = pCur->pPage;
  எனில்( pPage->nCell<=iCellIdx ){
    திரும்பு SQLITE_CORRUPT_PAGE(pPage);
  }
  pCell = findCell(pPage, iCellIdx);
  எனில்( pPage->nFree<0 && btreeComputeFreeSpace(pPage) ){
    திரும்பு SQLITE_CORRUPT_PAGE(pPage);
  }
  எனில்( pCell<&pPage->aCellIdx[pPage->nCell] ){
    திரும்பு SQLITE_CORRUPT_PAGE(pPage);
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
  எனில்( bPreserve ){
    எனில்( !pPage->leaf
     || (pPage->nFree+pPage->xCellSize(pPage,pCell)+2) >
                                                   (முழு)(pBt->usableSize*2/3)
     || pPage->nCell==1  /* See dbfuzz001.test for a test case */
    ){
      /* A b-tree rebalance will be required after deleting this entry.
      ** Save the cursor key.  */
      rc = save_Cursor_சாவி(pCur);
      எனில்( rc ) திரும்பு rc;
    }இல்லையெனில்{
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
  எனில்( !pPage->leaf ){
    rc = sqlite3BtreePrevious(pCur, 0);
    ப்ரதிஜ்ஞா( rc!=SQLITE_DONE );
    எனில்( rc ) திரும்பு rc;
  }

  /* Save the positions of any other cursors open on this table before
  ** making any modifications.  */
  எனில்( pCur->curFlags & BTCF_Multiple ){
    rc = saveAllCursors(pBt, pCur->pgno_வேர், pCur);
    எனில்( rc ) திரும்பு rc;
  }

  /* If this is a delete operation to remove a row from a table b-tree,
  ** invalidate any incrblob cursors open on the row being deleted.  */
  எனில்( pCur->p_சாவி_Info==0 && p->hasIncrblobCur ){
    invalidateIncrblobCursors(p, pCur->pgno_வேர், pCur->info.n_சாவி, 0);
  }

  /* Make the page containing the entry to be deleted writable. Then free any
  ** overflow pages associated with the entry and finally remove the cell
  ** itself from within the page.  */
  rc = sqlite3PagerWrite(pPage->pDbPage);
  எனில்( rc ) திரும்பு rc;
  BTREE_CLEAR_CELL(rc, pPage, pCell, info);
  dropCell(pPage, iCellIdx, info.nSize, &rc);
  எனில்( rc ) திரும்பு rc;

  /* If the cell deleted was not located on a leaf page, then the cursor
  ** is currently pointing to the largest entry in the sub-tree headed
  ** by the child-page of the cell that was just deleted from an internal
  ** node. The cell from the leaf node needs to be moved to the internal
  ** node to replace the deleted cell.  */
  எனில்( !pPage->leaf ){
    MemPage *pLeaf = pCur->pPage;
    முழு nCell;
    Pgno n;
    unsigned எழுத்து *pTmp;

    எனில்( pLeaf->nFree<0 ){
      rc = btreeComputeFreeSpace(pLeaf);
      எனில்( rc ) திரும்பு rc;
    }
    எனில்( iCellDepth<pCur->iPage-1 ){
      n = pCur->apPage[iCellDepth+1]->pgno;
    }இல்லையெனில்{
      n = pCur->pPage->pgno;
    }
    pCell = findCell(pLeaf, pLeaf->nCell-1);
    எனில்( pCell<&pLeaf->aData[4] ) திரும்பு SQLITE_CORRUPT_PAGE(pLeaf);
    nCell = pLeaf->xCellSize(pLeaf, pCell);
    ப்ரதிஜ்ஞா( MX_CELL_SIZE(pBt) >= nCell );
    pTmp = pBt->pTmpSpace;
    ப்ரதிஜ்ஞா( pTmp!=0 );
    rc = sqlite3PagerWrite(pLeaf->pDbPage);
    எனில்( rc==SQLITE_OK ){
      rc = செருகு_Cell(pPage, iCellIdx, pCell-4, nCell+4-(pCell[0]==2), pTmp, n);
    }
    dropCell(pLeaf, pLeaf->nCell-1, nCell, &rc);
    எனில்( rc ) திரும்பு rc;
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
  ப்ரதிஜ்ஞா( pCur->pPage->nOverflow==0 );
  ப்ரதிஜ்ஞா( pCur->pPage->nFree>=0 );
  எனில்( pCur->pPage->nFree*3<=(முழு)pCur->pBt->usableSize*2 ){
    /* Optimization: If the free space is less than 2/3rds of the page,
    ** then balance() will always be a no-op.  No need to invoke it. */
    rc = SQLITE_OK;
  }இல்லையெனில்{
    rc = balance(pCur);
  }
  எனில்( rc==SQLITE_OK && pCur->iPage>iCellDepth ){
    releasePageNotNull(pCur->pPage);
    pCur->iPage--;
    போது( pCur->iPage>iCellDepth ){
      releasePage(pCur->apPage[pCur->iPage--]);
    }
    pCur->pPage = pCur->apPage[pCur->iPage];
    rc = balance(pCur);
  }

  எனில்( rc==SQLITE_OK ){
    எனில்( bPreserve>1 ){
      ப்ரதிஜ்ஞா( (pCur->iPage==iCellDepth || CORRUPT_DB) );
      ப்ரதிஜ்ஞா( pPage==pCur->pPage || CORRUPT_DB );
      ப்ரதிஜ்ஞா( (pPage->nCell>0 || CORRUPT_DB) && iCellIdx<=pPage->nCell );
      pCur->eState = CURSOR_SKIPNEXT;
      எனில்( iCellIdx>=pPage->nCell ){
        pCur->skipNext = -1;
        pCur->ix = pPage->nCell-1;
      }இல்லையெனில்{
        pCur->skipNext = 1;
      }
      /* The pCur->info structure is currently populated with the details
      ** of the cell that was just removed. But pCur->ix now points to a
      ** different cell in the b-tree. So zero info.nSize here so that if
      ** saveCursorPosition() is called on this cursor it does not trust
      ** the contents of pCur->info.  */
      pCur->info.nSize = 0;
    }இல்லையெனில்{
      rc = move_To_வேர்(pCur);
      எனில்( bPreserve ){
        btreeReleaseAllCursorPages(pCur);
        pCur->eState = CURSOR_REQUIRESEEK;
      }
      எனில்( rc==SQLITE_EMPTY ) rc = SQLITE_OK;
    }
  }
  திரும்பு rc;
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
நிலையான முழு btreeCreateTable(Btree *p, Pgno *piTable, முழு createTabFlags){
  BtShared *pBt = p->pBt;
  MemPage *p_வேர்;
  Pgno pgno_வேர்;
  முழு rc;
  முழு ptfFlags;          /* Page-type flags for the root page of new table */

  ப்ரதிஜ்ஞா( sqlite3BtreeHoldsMutex(p) );
  ப்ரதிஜ்ஞா( pBt->inTransaction==TRANS_WRITE );
  ப்ரதிஜ்ஞா( (pBt->btsFlags & BTS_READ_ONLY)==0 );

#ifdef SQLITE_OMIT_AUTOVACUUM
  rc = allocateBtreePage(pBt, &p_வேர், &pgno_வேர், 1, 0);
  எனில்( rc ){
    திரும்பு rc;
  }
#இல்லையெனில்
  எனில்( pBt->autoVacuum ){
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
    sqlite3BtreeGetMeta(p, BTREE_LARGEST_வேர்_PAGE, &pgno_வேர்);
    எனில்( pgno_வேர்>btreePagecount(pBt) ){
      திரும்பு SQLITE_CORRUPT_PGNO(pgno_வேர்);
    }
    pgno_வேர்++;

    /* The new root-page may not be allocated on a pointer-map page, or the
    ** PENDING_BYTE page.
    */
    போது( pgno_வேர்==PTRMAP_PAGENO(pBt, pgno_வேர்) ||
        pgno_வேர்==PENDING_BYTE_PAGE(pBt) ){
      pgno_வேர்++;
    }
    ப்ரதிஜ்ஞா( pgno_வேர்>=3 );

    /* Allocate a page. The page that currently resides at pgnoRoot will
    ** be moved to the allocated page (unless the allocated page happens
    ** to reside at pgnoRoot).
    */
    rc = allocateBtreePage(pBt, &pPageMove, &pgnoMove, pgno_வேர், BTALLOC_EXACT);
    எனில்( rc!=SQLITE_OK ){
      திரும்பு rc;
    }

    எனில்( pgnoMove!=pgno_வேர் ){
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
      எனில்( rc!=SQLITE_OK ){
        திரும்பு rc;
      }

      /* Move the page currently at pgnoRoot to pgnoMove. */
      rc = btreeGetPage(pBt, pgno_வேர், &p_வேர், 0);
      எனில்( rc!=SQLITE_OK ){
        திரும்பு rc;
      }
      rc = ptrmapGet(pBt, pgno_வேர், &eType, &iPtrPage);
      எனில்( eType==PTRMAP_ROOTPAGE || eType==PTRMAP_FREEPAGE ){
        rc = SQLITE_CORRUPT_PGNO(pgno_வேர்);
      }
      எனில்( rc!=SQLITE_OK ){
        releasePage(p_வேர்);
        திரும்பு rc;
      }
      ப்ரதிஜ்ஞா( eType!=PTRMAP_ROOTPAGE );
      ப்ரதிஜ்ஞா( eType!=PTRMAP_FREEPAGE );
      rc = relocatePage(pBt, p_வேர், eType, iPtrPage, pgnoMove, 0);
      releasePage(p_வேர்);

      /* Obtain the page at pgnoRoot */
      எனில்( rc!=SQLITE_OK ){
        திரும்பு rc;
      }
      rc = btreeGetPage(pBt, pgno_வேர், &p_வேர், 0);
      எனில்( rc!=SQLITE_OK ){
        திரும்பு rc;
      }
      rc = sqlite3PagerWrite(p_வேர்->pDbPage);
      எனில்( rc!=SQLITE_OK ){
        releasePage(p_வேர்);
        திரும்பு rc;
      }
    }இல்லையெனில்{
      p_வேர் = pPageMove;
    }

    /* Update the pointer-map and meta-data with the new root-page number. */
    ptrmapPut(pBt, pgno_வேர், PTRMAP_ROOTPAGE, 0, &rc);
    எனில்( rc ){
      releasePage(p_வேர்);
      திரும்பு rc;
    }

    /* When the new root page was allocated, page 1 was made writable in
    ** order either to increase the database filesize, or to decrement the
    ** freelist count.  Hence, the sqlite3BtreeUpdateMeta() call cannot fail.
    */
    ப்ரதிஜ்ஞா( sqlite3PagerIswriteable(pBt->pPage1->pDbPage) );
    rc = sqlite3BtreeUpdateMeta(p, 4, pgno_வேர்);
    எனில்( NEVER(rc) ){
      releasePage(p_வேர்);
      திரும்பு rc;
    }

  }இல்லையெனில்{
    rc = allocateBtreePage(pBt, &p_வேர், &pgno_வேர், 1, 0);
    எனில்( rc ) திரும்பு rc;
  }
#endif
  ப்ரதிஜ்ஞா( sqlite3PagerIswriteable(p_வேர்->pDbPage) );
  எனில்( createTabFlags & BTREE_INTKEY ){
    ptfFlags = PTF_INTKEY | PTF_LEAFDATA | PTF_LEAF;
  }இல்லையெனில்{
    ptfFlags = PTF_ZERODATA | PTF_LEAF;
  }
  zeroPage(p_வேர், ptfFlags);
  sqlite3PagerUnref(p_வேர்->pDbPage);
  ப்ரதிஜ்ஞா( (pBt->openFlags & BTREE_SINGLE)==0 || pgno_வேர்==2 );
  *piTable = pgno_வேர்;
  திரும்பு SQLITE_OK;
}
முழு sqlite3BtreeCreateTable(Btree *p, Pgno *piTable, முழு flags){
  முழு rc;
  sqlite3BtreeEnter(p);
  rc = btreeCreateTable(p, piTable, flags);
  sqlite3BtreeLeave(p);
  திரும்பு rc;
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
நிலையான முழு clearDatabasePage(
  BtShared *pBt,           /* The BTree that contains the table */
  Pgno pgno,               /* Page number to clear */
  முழு freePageFlag,        /* bit 0: Deallocate page.  Bits 1-31: depth */
  i64 *pnChange            /* Add number of Cells freed to this counter */
){
  MemPage *pPage;
  முழு rc;
  unsigned எழுத்து *pCell;
  முழு i;
  முழு hdr;
  CellInfo info;

  ப்ரதிஜ்ஞா( sqlite3_mutex_held(pBt->mutex) );
  எனில்( pgno>btreePagecount(pBt) ){
    திரும்பு SQLITE_CORRUPT_PGNO(pgno);
  }
  எனில்( (freePageFlag>>1) > BTCURSOR_MAX_DEPTH ){
    திரும்பு SQLITE_CORRUPT_PGNO(pgno);
  }
  rc = getAndInitPage(pBt, pgno, &pPage, 0);
  எனில்( rc ) திரும்பு rc;
  எனில்( (pBt->openFlags & BTREE_SINGLE)==0
   && sqlite3PagerPageRefcount(pPage->pDbPage) != (1 + (pgno==1))
  ){
    rc = SQLITE_CORRUPT_PAGE(pPage);
    செல் cleardatabasepage_out;
  }
  hdr = pPage->hdrOffset;
  க்கு(i=0; i<pPage->nCell; i++){
    pCell = findCell(pPage, i);
    எனில்( !pPage->leaf ){
      rc = clearDatabasePage(pBt, get4byte(pCell),
                             (freePageFlag+2)|1, pnChange);
      எனில்( rc ) செல் cleardatabasepage_out;
    }
    BTREE_CLEAR_CELL(rc, pPage, pCell, info);
    எனில்( rc ) செல் cleardatabasepage_out;
  }
  எனில்( !pPage->leaf ){
    rc = clearDatabasePage(pBt, get4byte(&pPage->aData[hdr+8]), 
                           (freePageFlag+2)|1, pnChange);
    எனில்( rc ) செல் cleardatabasepage_out;
    எனில்( pPage->int_சாவி ) pnChange = 0;
  }
  எனில்( pnChange ){
    testcase( !pPage->int_சாவி );
    *pnChange += pPage->nCell;
  }
  எனில்( (freePageFlag&1)!=0 ){
    freePage(pPage, &rc);
  }இல்லையெனில் எனில்( (rc = sqlite3PagerWrite(pPage->pDbPage))==0 ){
    zeroPage(pPage, pPage->aData[hdr] | PTF_LEAF);
  }

cleardatabasepage_out:
  releasePage(pPage);
  திரும்பு rc;
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
முழு sqlite3BtreeClearTable(Btree *p, முழு iTable, i64 *pnChange){
  முழு rc;
  BtShared *pBt = p->pBt;
  sqlite3BtreeEnter(p);
  ப்ரதிஜ்ஞா( p->inTrans==TRANS_WRITE );

  rc = saveAllCursors(pBt, (Pgno)iTable, 0);

  எனில்( SQLITE_OK==rc ){
    /* Invalidate all incrblob cursors open on table iTable (assuming iTable
    ** is the root of a table b-tree - if it is not, the following call is
    ** a no-op).  */
    எனில்( p->hasIncrblobCur ){
      invalidateIncrblobCursors(p, (Pgno)iTable, 0, 1);
    }
    rc = clearDatabasePage(pBt, (Pgno)iTable, 0, pnChange);
  }
  sqlite3BtreeLeave(p);
  திரும்பு rc;
}

/*
** Delete all information from the single table that pCur is open on.
**
** This routine only work for pCur on an ephemeral table.
*/
முழு sqlite3BtreeClearTableOfCursor(BtCursor *pCur){
  திரும்பு sqlite3BtreeClearTable(pCur->pBtree, pCur->pgno_வேர், 0);
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
நிலையான முழு btreeDropTable(Btree *p, Pgno iTable, முழு *piMoved){
  முழு rc;
  MemPage *pPage = 0;
  BtShared *pBt = p->pBt;

  ப்ரதிஜ்ஞா( sqlite3BtreeHoldsMutex(p) );
  ப்ரதிஜ்ஞா( p->inTrans==TRANS_WRITE );
  ப்ரதிஜ்ஞா( iTable>=2 );
  எனில்( iTable>btreePagecount(pBt) ){
    திரும்பு SQLITE_CORRUPT_PGNO(iTable);
  }

  rc = sqlite3BtreeClearTable(p, iTable, 0);
  எனில்( rc ) திரும்பு rc;
  rc = btreeGetPage(pBt, (Pgno)iTable, &pPage, 0);
  எனில்( NEVER(rc) ){
    releasePage(pPage);
    திரும்பு rc;
  }

  *piMoved = 0;

#ifdef SQLITE_OMIT_AUTOVACUUM
  freePage(pPage, &rc);
  releasePage(pPage);
#இல்லையெனில்
  எனில்( pBt->autoVacuum ){
    Pgno max_வேர்_Pgno;
    sqlite3BtreeGetMeta(p, BTREE_LARGEST_வேர்_PAGE, &max_வேர்_Pgno);

    எனில்( iTable==max_வேர்_Pgno ){
      /* If the table being dropped is the table with the largest root-page
      ** number in the database, put the root page on the free list.
      */
      freePage(pPage, &rc);
      releasePage(pPage);
      எனில்( rc!=SQLITE_OK ){
        திரும்பு rc;
      }
    }இல்லையெனில்{
      /* The table being dropped does not have the largest root-page
      ** number in the database. So move the page that does into the
      ** gap left by the deleted root-page.
      */
      MemPage *pMove;
      releasePage(pPage);
      rc = btreeGetPage(pBt, max_வேர்_Pgno, &pMove, 0);
      எனில்( rc!=SQLITE_OK ){
        திரும்பு rc;
      }
      rc = relocatePage(pBt, pMove, PTRMAP_ROOTPAGE, 0, iTable, 0);
      releasePage(pMove);
      எனில்( rc!=SQLITE_OK ){
        திரும்பு rc;
      }
      pMove = 0;
      rc = btreeGetPage(pBt, max_வேர்_Pgno, &pMove, 0);
      எனில்( rc==SQLITE_OK ){
        rc = sqlite3PagerWrite(pMove->pDbPage);
      }
      freePage(pMove, &rc);
      releasePage(pMove);
      எனில்( rc!=SQLITE_OK ){
        திரும்பு rc;
      }
      *piMoved = max_வேர்_Pgno;
    }

    /* Set the new 'max-root-page' value in the database header. This
    ** is the old value less one, less one more if that happens to
    ** be a root-page number, less one again if that is the
    ** PENDING_BYTE_PAGE.
    */
    max_வேர்_Pgno--;
    போது( max_வேர்_Pgno==PENDING_BYTE_PAGE(pBt)
           || PTRMAP_ISPAGE(pBt, max_வேர்_Pgno) ){
      max_வேர்_Pgno--;
    }
    ப்ரதிஜ்ஞா( max_வேர்_Pgno!=PENDING_BYTE_PAGE(pBt) );

    rc = sqlite3BtreeUpdateMeta(p, 4, max_வேர்_Pgno);
  }இல்லையெனில்{
    freePage(pPage, &rc);
    releasePage(pPage);
  }
#endif
  திரும்பு rc; 
}
முழு sqlite3BtreeDropTable(Btree *p, முழு iTable, முழு *piMoved){
  முழு rc;
  sqlite3BtreeEnter(p);
  rc = btreeDropTable(p, iTable, piMoved);
  sqlite3BtreeLeave(p);
  திரும்பு rc;
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
வெறுமை sqlite3BtreeGetMeta(Btree *p, முழு idx, u32 *pMeta){
  BtShared *pBt = p->pBt;

  sqlite3BtreeEnter(p);
  ப்ரதிஜ்ஞா( p->inTrans>TRANS_NONE );
  ப்ரதிஜ்ஞா( SQLITE_OK==querySharedCacheTableLock(p, SCHEMA_வேர், READ_LOCK) );
  ப்ரதிஜ்ஞா( pBt->pPage1 );
  ப்ரதிஜ்ஞா( idx>=0 && idx<=15 );

  எனில்( idx==BTREE_DATA_VERSION ){
    *pMeta = sqlite3PagerDataVersion(pBt->pPager) + p->iBDataVersion;
  }இல்லையெனில்{
    *pMeta = get4byte(&pBt->pPage1->aData[36 + idx*4]);
  }

  /* If auto-vacuum is disabled in this build and this is an auto-vacuum
  ** database, mark the database as read-only.  */
#ifdef SQLITE_OMIT_AUTOVACUUM
  எனில்( idx==BTREE_LARGEST_வேர்_PAGE && *pMeta>0 ){
    pBt->btsFlags |= BTS_READ_ONLY;
  }
#endif

  sqlite3BtreeLeave(p);
}

/*
** Write meta-information back into the database.  Meta[0] is
** read-only and may not be written.
*/
முழு sqlite3BtreeUpdateMeta(Btree *p, முழு idx, u32 iMeta){
  BtShared *pBt = p->pBt;
  unsigned எழுத்து *pP1;
  முழு rc;
  ப்ரதிஜ்ஞா( idx>=1 && idx<=15 );
  sqlite3BtreeEnter(p);
  ப்ரதிஜ்ஞா( p->inTrans==TRANS_WRITE );
  ப்ரதிஜ்ஞா( pBt->pPage1!=0 );
  pP1 = pBt->pPage1->aData;
  rc = sqlite3PagerWrite(pBt->pPage1->pDbPage);
  எனில்( rc==SQLITE_OK ){
    put4byte(&pP1[36 + idx*4], iMeta);
#ifndef SQLITE_OMIT_AUTOVACUUM
    எனில்( idx==BTREE_INCR_VACUUM ){
      ப்ரதிஜ்ஞா( pBt->autoVacuum || iMeta==0 );
      ப்ரதிஜ்ஞா( iMeta==0 || iMeta==1 );
      pBt->incrVacuum = (u8)iMeta;
    }
#endif
  }
  sqlite3BtreeLeave(p);
  திரும்பு rc;
}

/*
** The first argument, pCur, is a cursor opened on some b-tree. Count the
** number of entries in the b-tree and write the result to *pnEntry.
**
** SQLITE_OK is returned if the operation is successfully executed.
** Otherwise, if an error is encountered (i.e. an IO error or database
** corruption) an SQLite error code is returned.
*/
முழு sqlite3_Btree_மொத்தம்(sqlite3 *db, BtCursor *pCur, i64 *pnEntry){
  i64 nEntry = 0;                      /* Value to return in *pnEntry */
  முழு rc;                              /* Return code */

  rc = move_To_வேர்(pCur);
  எனில்( rc==SQLITE_EMPTY ){
    *pnEntry = 0;
    திரும்பு SQLITE_OK;
  }

  /* Unless an error occurs, the following loop runs one iteration for each
  ** page in the B-Tree structure (not including overflow pages).
  */
  போது( rc==SQLITE_OK && !AtomicLoad(&db->u1.isInterrupted) ){
    முழு iIdx;                          /* Index of child node in parent */
    MemPage *pPage;                    /* Current page of the b-tree */

    /* If this is a leaf page or the tree is not an int-key tree, then
    ** this page contains countable entries. Increment the entry counter
    ** accordingly.
    */
    pPage = pCur->pPage;
    எனில்( pPage->leaf || !pPage->int_சாவி ){
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
    எனில்( pPage->leaf ){
      செய் {
        எனில்( pCur->iPage==0 ){
          /* All pages of the b-tree have been visited. Return successfully. */
          *pnEntry = nEntry;
          திரும்பு move_To_வேர்(pCur);
        }
        moveToParent(pCur);
      }போது ( pCur->ix>=pCur->pPage->nCell );

      pCur->ix++;
      pPage = pCur->pPage;
    }

    /* Descend to the child node of the cell that the cursor currently
    ** points at. This is the right-child if (iIdx==pPage->nCell).
    */
    iIdx = pCur->ix;
    எனில்( iIdx==pPage->nCell ){
      rc = moveToChild(pCur, get4byte(&pPage->aData[pPage->hdrOffset+8]));
    }இல்லையெனில்{
      rc = moveToChild(pCur, get4byte(findCell(pPage, iIdx)));
    }
  }

  /* An error has occurred. Return an error code. */
  திரும்பு rc;
}

/*
** Return the pager associated with a BTree.  This routine is used for
** testing and debugging only.
*/
Pager *sqlite3BtreePager(Btree *p){
  திரும்பு p->pBt->pPager;
}

#ifndef SQLITE_OMIT_INTEGRITY_CHECK
/*
** Record an OOM error during integrity_check
*/
நிலையான வெறுமை checkOom(IntegrityCk *pCheck){
  pCheck->rc = SQLITE_NOMEM;
  pCheck->mxErr = 0;  /* Causes integrity_check processing to stop */
  எனில்( pCheck->nErr==0 ) pCheck->nErr++;
}

/*
** Invoke the progress handler, if appropriate.  Also check for an
** interrupt.
*/
நிலையான வெறுமை checkProgress(IntegrityCk *pCheck){
  sqlite3 *db = pCheck->db;
  எனில்( AtomicLoad(&db->u1.isInterrupted) ){
    pCheck->rc = SQLITE_INTERRUPT;
    pCheck->nErr++;
    pCheck->mxErr = 0;
  }
#ifndef SQLITE_OMIT_PROGRESS_CALLBACK
  எனில்( db->xProgress ){
    ப்ரதிஜ்ஞா( db->nProgressOps>0 );
    pCheck->nStep++;
    எனில்( (pCheck->nStep % db->nProgressOps)==0
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
நிலையான வெறுமை check_Append_செய்தி(
  IntegrityCk *pCheck,
  மாறிலி எழுத்து *zFormat,
  ...
){
  va_list ap;
  checkProgress(pCheck);
  எனில்( !pCheck->mxErr ) திரும்பு;
  pCheck->mxErr--;
  pCheck->nErr++;
  va_start(ap, zFormat);
  எனில்( pCheck->err_செய்தி.nChar ){
    sqlite3_str_append(&pCheck->err_செய்தி, "\n", 1);
  }
  எனில்( pCheck->zPfx ){
    sqlite3_str_appendf(&pCheck->err_செய்தி, pCheck->zPfx,
                        pCheck->v0, pCheck->v1, pCheck->v2);
  }
  sqlite3_str_vappendf(&pCheck->err_செய்தி, zFormat, ap);
  va_end(ap);
  எனில்( pCheck->err_செய்தி.accError==SQLITE_NOMEM ){
    checkOom(pCheck);
  }
}
#endif /* SQLITE_OMIT_INTEGRITY_CHECK */

#ifndef SQLITE_OMIT_INTEGRITY_CHECK

/*
** Return non-zero if the bit in the IntegrityCk.aPgRef[] array that
** corresponds to page iPg is already set.
*/
நிலையான முழு getPageReferenced(IntegrityCk *pCheck, Pgno iPg){
  ப்ரதிஜ்ஞா( pCheck->aPgRef!=0 );
  ப்ரதிஜ்ஞா( iPg<=pCheck->nCkPage && அளவு(pCheck->aPgRef[0])==1 );
  திரும்பு (pCheck->aPgRef[iPg/8] & (1 << (iPg & 0x07)));
}

/*
** Set the bit in the IntegrityCk.aPgRef[] array that corresponds to page iPg.
*/
நிலையான வெறுமை setPageReferenced(IntegrityCk *pCheck, Pgno iPg){
  ப்ரதிஜ்ஞா( pCheck->aPgRef!=0 );
  ப்ரதிஜ்ஞா( iPg<=pCheck->nCkPage && அளவு(pCheck->aPgRef[0])==1 );
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
நிலையான முழு checkRef(IntegrityCk *pCheck, Pgno iPage){
  எனில்( iPage>pCheck->nCkPage || iPage==0 ){
    check_Append_செய்தி(pCheck, "invalid page number %u", iPage);
    திரும்பு 1;
  }
  எனில்( getPageReferenced(pCheck, iPage) ){
    check_Append_செய்தி(pCheck, "2nd reference to page %u", iPage);
    திரும்பு 1;
  }
  setPageReferenced(pCheck, iPage);
  திரும்பு 0;
}

#ifndef SQLITE_OMIT_AUTOVACUUM
/*
** Check that the entry in the pointer-map for page iChild maps to
** page iParent, pointer type ptrType. If not, append an error message
** to pCheck.
*/
நிலையான வெறுமை checkPtrmap(
  IntegrityCk *pCheck,   /* Integrity check context */
  Pgno iChild,           /* Child page number */
  u8 eType,              /* Expected pointer map type */
  Pgno iParent           /* Expected pointer map parent page number */
){
  முழு rc;
  u8 ePtrmapType;
  Pgno iPtrmapParent;

  rc = ptrmapGet(pCheck->pBt, iChild, &ePtrmapType, &iPtrmapParent);
  எனில்( rc!=SQLITE_OK ){
    எனில்( rc==SQLITE_NOMEM || rc==SQLITE_IOERR_NOMEM ) checkOom(pCheck);
    check_Append_செய்தி(pCheck, "Failed to read ptrmap key=%u", iChild);
    திரும்பு;
  }

  எனில்( ePtrmapType!=eType || iPtrmapParent!=iParent ){
    check_Append_செய்தி(pCheck,
      "Bad ptr map entry key=%u expected=(%u,%u) got=(%u,%u)",
      iChild, eType, iParent, ePtrmapType, iPtrmapParent);
  }
}
#endif

/*
** Check the integrity of the freelist or of an overflow page list.
** Verify that the number of pages on the list is N.
*/
நிலையான வெறுமை checkList(
  IntegrityCk *pCheck,  /* Integrity checking context */
  முழு isFreeList,       /* True for a freelist.  False for overflow page list */
  Pgno iPage,           /* Page number for first page in the list */
  u32 N                 /* Expected number of pages in the list */
){
  முழு i;
  u32 expected = N;
  முழு nErrAtStart = pCheck->nErr;
  போது( iPage!=0 && pCheck->mxErr ){
    DbPage *pOvflPage;
    unsigned எழுத்து *pOvflData;
    எனில்( checkRef(pCheck, iPage) ) நிறுத்து;
    N--;
    எனில்( sqlite3PagerGet(pCheck->pPager, (Pgno)iPage, &pOvflPage, 0) ){
      check_Append_செய்தி(pCheck, "failed to get page %u", iPage);
      நிறுத்து;
    }
    pOvflData = (unsigned எழுத்து *)sqlite3PagerGetData(pOvflPage);
    எனில்( isFreeList ){
      u32 n = (u32)get4byte(&pOvflData[4]);
#ifndef SQLITE_OMIT_AUTOVACUUM
      எனில்( pCheck->pBt->autoVacuum ){
        checkPtrmap(pCheck, iPage, PTRMAP_FREEPAGE, 0);
      }
#endif
      எனில்( n>pCheck->pBt->usableSize/4-2 ){
        check_Append_செய்தி(pCheck,
           "freelist leaf count too big on page %u", iPage);
        N--;
      }இல்லையெனில்{
        க்கு(i=0; i<(முழு)n; i++){
          Pgno iFreePage = get4byte(&pOvflData[8+i*4]);
#ifndef SQLITE_OMIT_AUTOVACUUM
          எனில்( pCheck->pBt->autoVacuum ){
            checkPtrmap(pCheck, iFreePage, PTRMAP_FREEPAGE, 0);
          }
#endif
          checkRef(pCheck, iFreePage);
        }
        N -= n;
      }
    }
#ifndef SQLITE_OMIT_AUTOVACUUM
    இல்லையெனில்{
      /* If this database supports auto-vacuum and iPage is not the last
      ** page in this overflow list, check that the pointer-map entry for
      ** the following page matches iPage.
      */
      எனில்( pCheck->pBt->autoVacuum && N>0 ){
        i = get4byte(pOvflData);
        checkPtrmap(pCheck, i, PTRMAP_OVERFLOW2, iPage);
      }
    }
#endif
    iPage = get4byte(pOvflData);
    sqlite3PagerUnref(pOvflPage);
  }
  எனில்( N && nErrAtStart==pCheck->nErr ){
    check_Append_செய்தி(pCheck,
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
நிலையான வெறுமை btree_Heap_செருகு(u32 *aHeap, u32 x){
  u32 j, i;
  ப்ரதிஜ்ஞா( aHeap!=0 );
  i = ++aHeap[0];
  aHeap[i] = x;
  போது( (j = i/2)>0 && aHeap[j]>aHeap[i] ){
    x = aHeap[j];
    aHeap[j] = aHeap[i];
    aHeap[i] = x;
    i = j;
  }
}
நிலையான முழு btreeHeapPull(u32 *aHeap, u32 *pOut){
  u32 j, i, x;
  எனில்( (x = aHeap[0])==0 ) திரும்பு 0;
  *pOut = aHeap[1];
  aHeap[1] = aHeap[x];
  aHeap[x] = 0xffffffff;
  aHeap[0]--;
  i = 1;
  போது( (j = i*2)<=aHeap[0] ){
    எனில்( aHeap[j]>aHeap[j+1] ) j++;
    எனில்( aHeap[i]<aHeap[j] ) நிறுத்து;
    x = aHeap[i];
    aHeap[i] = aHeap[j];
    aHeap[j] = x;
    i = j;
  }
  திரும்பு 1; 
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
நிலையான முழு check_மரம்_Page(
  IntegrityCk *pCheck,  /* Context for the sanity check */
  Pgno iPage,           /* Page number of the page to check */
  i64 *pi_Min_சாவி,        /* Write minimum integer primary key here */
  i64 max_சாவி            /* Error if integer primary key greater than this */
){
  MemPage *pPage = 0;      /* The page being analyzed */
  முழு i;                   /* Loop counter */
  முழு rc;                  /* Result code from subroutine call */
  முழு depth = -1, d2;      /* Depth of a subtree */
  முழு pgno;                /* Page number */
  முழு nFrag;               /* Number of fragmented bytes on the page */
  முழு hdr;                 /* Offset to the page header */
  முழு cellStart;           /* Offset to the start of the cell pointer array */
  முழு nCell;               /* Number of cells */
  முழு doCoverageCheck = 1; /* True if cell coverage checking should be done */
  முழு சாவி_Can_Be_Equal = 1;   /* True if IPK can be equal to maxKey
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
  மாறிலி எழுத்து *saved_zPfx = pCheck->zPfx;
  முழு saved_v1 = pCheck->v1;
  முழு saved_v2 = pCheck->v2;
  u8 savedIsInit = 0;

  /* Check that the page exists
  */
  checkProgress(pCheck);
  எனில்( pCheck->mxErr==0 ) திரும்பு 0;
  pBt = pCheck->pBt;
  usableSize = pBt->usableSize;
  எனில்( iPage==0 ) திரும்பு 0;
  எனில்( checkRef(pCheck, iPage) ) திரும்பு 0;
  pCheck->zPfx = "Tree %u page %u: ";
  pCheck->v1 = iPage;
  pCheck->nAbove++;
  எனில்( (rc = btreeGetPage(pBt, iPage, &pPage, 0))!=0 ){
    check_Append_செய்தி(pCheck,
       "unable to get the page. error code=%d", rc);
    எனில்( rc==SQLITE_IOERR_NOMEM ) pCheck->rc = SQLITE_NOMEM;
    செல் end_of_check;
  }
  எனில்( pCheck->nAbove > BTCURSOR_MAX_DEPTH ){
    check_Append_செய்தி(pCheck,"btree depth exceeds %d",BTCURSOR_MAX_DEPTH);
    செல் end_of_check;
  }

  /* Clear MemPage.isInit to make sure the corruption detection code in
  ** btreeInitPage() is executed.  */
  savedIsInit = pPage->isInit;
  pPage->isInit = 0;
  எனில்( (rc = btreeInitPage(pPage))!=0 ){
    ப்ரதிஜ்ஞா( rc==SQLITE_CORRUPT );  /* The only possible error from InitPage */
    check_Append_செய்தி(pCheck,
                   "btreeInitPage() returns error code %d", rc);
    செல் end_of_check;
  }
  எனில்( (rc = btreeComputeFreeSpace(pPage))!=0 ){
    ப்ரதிஜ்ஞா( rc==SQLITE_CORRUPT );
    check_Append_செய்தி(pCheck, "free space corruption", rc);
    செல் end_of_check;
  }
  data = pPage->aData;
  hdr = pPage->hdrOffset;

  /* Set up for cell analysis */
  pCheck->zPfx = "Tree %u page %u cell %u: ";
  contentOffset = get2byteNotZero(&data[hdr+5]);
  ப்ரதிஜ்ஞா( contentOffset<=usableSize );  /* Enforced by btreeInitPage() */

  /* EVIDENCE-OF: R-37002-32774 The two-byte integer at offset 3 gives the
  ** number of cells on the page. */
  nCell = get2byte(&data[hdr+3]);
  ப்ரதிஜ்ஞா( pPage->nCell==nCell );
  எனில்( pPage->leaf || pPage->int_சாவி==0 ){
    pCheck->nRow += nCell;
  }

  /* EVIDENCE-OF: R-23882-45353 The cell pointer array of a b-tree page
  ** immediately follows the b-tree page header. */
  cellStart = hdr + 12 - 4*pPage->leaf;
  ப்ரதிஜ்ஞா( pPage->aCellIdx==&data[cellStart] );
  pCellIdx = &data[cellStart + 2*(nCell-1)];

  எனில்( !pPage->leaf ){
    /* Analyze the right-child page of internal pages */
    pgno = get4byte(&data[hdr+8]);
#ifndef SQLITE_OMIT_AUTOVACUUM
    எனில்( pBt->autoVacuum ){
      pCheck->zPfx = "Tree %u page %u right child: ";
      checkPtrmap(pCheck, pgno, PTRMAP_BTREE, iPage);
    }
#endif
    depth = check_மரம்_Page(pCheck, pgno, &max_சாவி, max_சாவி);
    சாவி_Can_Be_Equal = 0;
  }இல்லையெனில்{
    /* For leaf pages, the coverage check will occur in the same loop
    ** as the other cell checks, so initialize the heap.  */
    heap = pCheck->heap;
    heap[0] = 0;
  }

  /* EVIDENCE-OF: R-02776-14802 The cell pointer array consists of K 2-byte
  ** integer offsets to the cell contents. */
  க்கு(i=nCell-1; i>=0 && pCheck->mxErr; i--){
    CellInfo info;

    /* Check cell size */
    pCheck->v2 = i;
    ப்ரதிஜ்ஞா( pCellIdx==&data[cellStart + i*2] );
    pc = get2byteAligned(pCellIdx);
    pCellIdx -= 2;
    எனில்( pc<contentOffset || pc>usableSize-4 ){
      check_Append_செய்தி(pCheck, "Offset %u out of range %u..%u",
                             pc, contentOffset, usableSize-4);
      doCoverageCheck = 0;
      தொடர்;
    }
    pCell = &data[pc];
    pPage->xParseCell(pPage, pCell, &info);
    எனில்( pc+info.nSize>usableSize ){
      check_Append_செய்தி(pCheck, "Extends off end of page");
      doCoverageCheck = 0;
      தொடர்;
    }
    எனில்( info.nPayload && info.pPayload[0]<2 ){
      check_Append_செய்தி(pCheck, "Bad cell header size");
      doCoverageCheck = 0;
      தொடர்;
    }

    /* Check for integer primary key out of range */
    எனில்( pPage->int_சாவி ){
      எனில்( சாவி_Can_Be_Equal ? (info.n_சாவி > max_சாவி) : (info.n_சாவி >= max_சாவி) ){
        check_Append_செய்தி(pCheck, "Rowid %lld out of order", info.n_சாவி);
      }
      max_சாவி = info.n_சாவி;
      சாவி_Can_Be_Equal = 0;     /* Only the first key on the page may ==maxKey */
    }

    /* Check the content overflow list */
    எனில்( info.nPayload>info.nLocal ){
      u32 nPage;       /* Number of pages on the overflow chain */
      Pgno pgnoOvfl;   /* First page of the overflow chain */
      ப்ரதிஜ்ஞா( pc + info.nSize - 4 <= usableSize );
      nPage = (info.nPayload - info.nLocal + usableSize - 5)/(usableSize - 4);
      pgnoOvfl = get4byte(&pCell[info.nSize - 4]);
#ifndef SQLITE_OMIT_AUTOVACUUM
      எனில்( pBt->autoVacuum ){
        checkPtrmap(pCheck, pgnoOvfl, PTRMAP_OVERFLOW1, iPage);
      }
#endif
      checkList(pCheck, 0, pgnoOvfl, nPage);
    }

    எனில்( !pPage->leaf ){
      /* Check sanity of left child page for internal pages */
      pgno = get4byte(pCell);
#ifndef SQLITE_OMIT_AUTOVACUUM
      எனில்( pBt->autoVacuum ){
        checkPtrmap(pCheck, pgno, PTRMAP_BTREE, iPage);
      }
#endif
      d2 = check_மரம்_Page(pCheck, pgno, &max_சாவி, max_சாவி);
      சாவி_Can_Be_Equal = 0;
      எனில்( d2!=depth ){
        check_Append_செய்தி(pCheck, "Child page depth differs");
        depth = d2;
      }
    }இல்லையெனில்{
      /* Populate the coverage-checking heap for leaf pages */
      ப்ரதிஜ்ஞா( heap!=0 && heap[0] < pCheck->mxHeap );
      btree_Heap_செருகு(heap, (pc<<16)|(pc+info.nSize-1));
    }
  }
  *pi_Min_சாவி = max_சாவி;

  /* Check for complete coverage of the page
  */
  pCheck->zPfx = 0;
  எனில்( doCoverageCheck && pCheck->mxErr>0 ){
    /* For leaf pages, the min-heap has already been initialized and the
    ** cells have already been inserted.  But for internal pages, that has
    ** not yet been done, so do it now */
    எனில்( !pPage->leaf ){
      heap = pCheck->heap;
      heap[0] = 0;
      க்கு(i=nCell-1; i>=0; i--){
        u32 size;
        pc = get2byteAligned(&data[cellStart+i*2]);
        size = pPage->xCellSize(pPage, &data[pc]);
        ப்ரதிஜ்ஞா( heap!=0 && heap[0] < pCheck->mxHeap );
        btree_Heap_செருகு(heap, (pc<<16)|(pc+size-1));
      }
    }
    ப்ரதிஜ்ஞா( heap!=0 );
    /* Add the freeblocks to the min-heap
    **
    ** EVIDENCE-OF: R-20690-50594 The second field of the b-tree page header
    ** is the offset of the first freeblock, or zero if there are no
    ** freeblocks on the page.
    */
    i = get2byte(&data[hdr+1]);
    போது( i>0 ){
      முழு size, j;
      ப்ரதிஜ்ஞா( (u32)i<=usableSize-4 ); /* Enforced by btreeComputeFreeSpace() */
      size = get2byte(&data[i+2]);
      ப்ரதிஜ்ஞா( (u32)(i+size)<=usableSize ); /* due to btreeComputeFreeSpace() */
      ப்ரதிஜ்ஞா( heap!=0 && heap[0] < pCheck->mxHeap );
      btree_Heap_செருகு(heap, (((u32)i)<<16)|(i+size-1));
      /* EVIDENCE-OF: R-58208-19414 The first 2 bytes of a freeblock are a
      ** big-endian integer which is the offset in the b-tree page of the next
      ** freeblock in the chain, or zero if the freeblock is the last on the
      ** chain. */
      j = get2byte(&data[i]);
      /* EVIDENCE-OF: R-06866-39125 Freeblocks are always connected in order of
      ** increasing offset. */
      ப்ரதிஜ்ஞா( j==0 || j>i+size );     /* Enforced by btreeComputeFreeSpace() */
      ப்ரதிஜ்ஞா( (u32)j<=usableSize-4 ); /* Enforced by btreeComputeFreeSpace() */
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
    போது( btreeHeapPull(heap,&x) ){
      எனில்( (prev&0xffff)>=(x>>16) ){
        check_Append_செய்தி(pCheck,
          "Multiple uses for byte %u of page %u", x>>16, iPage);
        நிறுத்து;
      }இல்லையெனில்{
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
    எனில்( heap[0]==0 && nFrag!=data[hdr+7] ){
      check_Append_செய்தி(pCheck,
          "Fragmentation of %u bytes reported as %u on page %u",
          nFrag, data[hdr+7], iPage);
    }
  }

end_of_check:
  எனில்( !doCoverageCheck ) pPage->isInit = savedIsInit;
  releasePage(pPage);
  pCheck->zPfx = saved_zPfx;
  pCheck->v1 = saved_v1;
  pCheck->v2 = saved_v2;
  pCheck->nAbove--;
  திரும்பு depth+1;
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
முழு sqlite3BtreeIntegrityCheck(
  sqlite3 *db,  /* Database connection that is running the check */
  Btree *p,     /* The btree to be checked */
  Pgno *a_வேர்,  /* An array of root pages numbers for individual trees */
  Mem *aCnt,    /* Memory cells to write counts for each tree to */
  முழு n_வேர்,    /* Number of entries in aRoot[] */
  முழு mxErr,    /* Stop reporting errors after this many */
  முழு *pnErr,   /* OUT: Write number of errors seen to this variable */
  எழுத்து **pzOut  /* OUT: Write the error message string here */
){
  Pgno i;
  IntegrityCk sCheck;
  BtShared *pBt = p->pBt;
  u64 savedDbFlags = pBt->db->flags;
  எழுத்து zErr[100];
  முழு bPartial = 0;            /* True if not checking all btrees */
  முழு bCkFreelist = 1;         /* True to scan the freelist */
  VVA_ONLY( முழு nRef );

  ப்ரதிஜ்ஞா( n_வேர்>0 );
  ப்ரதிஜ்ஞா( aCnt!=0 );

  /* aRoot[0]==0 means this is a partial check */
  எனில்( a_வேர்[0]==0 ){
    ப்ரதிஜ்ஞா( n_வேர்>1 );
    bPartial = 1;
    எனில்( a_வேர்[1]!=1 ) bCkFreelist = 0;
  }

  sqlite3BtreeEnter(p);
  ப்ரதிஜ்ஞா( p->inTrans>TRANS_NONE && pBt->inTransaction>TRANS_NONE );
  VVA_ONLY( nRef = sqlite3PagerRefcount(pBt->pPager) );
  ப்ரதிஜ்ஞா( nRef>=0 );
  memset(&sCheck, 0, அளவு(sCheck));
  sCheck.db = db;
  sCheck.pBt = pBt;
  sCheck.pPager = pBt->pPager;
  sCheck.nCkPage = btreePagecount(sCheck.pBt);
  sCheck.mxErr = mxErr;
  sqlite3StrAccumInit(&sCheck.err_செய்தி, 0, zErr, அளவு(zErr), SQLITE_MAX_LENGTH);
  sCheck.err_செய்தி.printfFlags = SQLITE_PRINTF_INTERNAL;
  எனில்( sCheck.nCkPage==0 ){
    செல் integrity_ck_cleanup;
  }

  sCheck.aPgRef = sqlite3MallocZero((sCheck.nCkPage / 8)+ 1);
  எனில்( !sCheck.aPgRef ){
    checkOom(&sCheck);
    செல் integrity_ck_cleanup;
  }
  sCheck.heap = (u32*)sqlite3Malloc( pBt->pageSize*2 );
#ifdef SQLITE_DEBUG
  sCheck.mxHeap = pBt->pageSize/2 - 1;
#endif
  எனில்( sCheck.heap==0 ){
    checkOom(&sCheck);
    செல் integrity_ck_cleanup;
  }

  i = PENDING_BYTE_PAGE(pBt);
  எனில்( i<=sCheck.nCkPage ) setPageReferenced(&sCheck, i);

  /* Check the integrity of the freelist
  */
  எனில்( bCkFreelist ){
    sCheck.zPfx = "Freelist: ";
    checkList(&sCheck, 1, get4byte(&pBt->pPage1->aData[32]),
              get4byte(&pBt->pPage1->aData[36]));
    sCheck.zPfx = 0;
  }

  /* Check all the tables.
  */
#ifndef SQLITE_OMIT_AUTOVACUUM
  எனில்( !bPartial ){
    எனில்( pBt->autoVacuum ){
      Pgno mx = 0;
      Pgno mxInHdr;
      க்கு(i=0; (முழு)i<n_வேர்; i++) எனில்( mx<a_வேர்[i] ) mx = a_வேர்[i];
      mxInHdr = get4byte(&pBt->pPage1->aData[52]);
      எனில்( mx!=mxInHdr ){
        check_Append_செய்தி(&sCheck,
          "max rootpage (%u) disagrees with header (%u)",
          mx, mxInHdr
        );
      }
    }இல்லையெனில் எனில்( get4byte(&pBt->pPage1->aData[64])!=0 ){
      check_Append_செய்தி(&sCheck,
        "incremental_vacuum enabled with a max rootpage of zero"
      );
    }
  }
#endif
  testcase( pBt->db->flags & SQLITE_CellSizeCk );
  pBt->db->flags &= ~(u64)SQLITE_CellSizeCk;
  க்கு(i=0; (முழு)i<n_வேர் && sCheck.mxErr; i++){
    sCheck.nRow = 0;
    எனில்( a_வேர்[i] ){
      i64 notUsed;
#ifndef SQLITE_OMIT_AUTOVACUUM
      எனில்( pBt->autoVacuum && a_வேர்[i]>1 && !bPartial ){
        checkPtrmap(&sCheck, a_வேர்[i], PTRMAP_ROOTPAGE, 0);
      }
#endif
      sCheck.v0 = a_வேர்[i];
      ப்ரதிஜ்ஞா( sCheck.nAbove==0 );
      check_மரம்_Page(&sCheck, a_வேர்[i], &notUsed, LARGEST_INT64);
      ப்ரதிஜ்ஞா( sCheck.nAbove==0 );
    }
    sqlite3MemSetArrayInt64(aCnt, i, sCheck.nRow);
  }
  pBt->db->flags = savedDbFlags;

  /* Make sure every page in the file is referenced
  */
  எனில்( !bPartial ){
    க்கு(i=1; i<=sCheck.nCkPage && sCheck.mxErr; i++){
#ifdef SQLITE_OMIT_AUTOVACUUM
      எனில்( getPageReferenced(&sCheck, i)==0 ){
        check_Append_செய்தி(&sCheck, "Page %u: never used", i);
      }
#இல்லையெனில்
      /* If the database supports auto-vacuum, make sure no tables contain
      ** references to pointer-map pages.
      */
      எனில்( getPageReferenced(&sCheck, i)==0 &&
         (PTRMAP_PAGENO(pBt, i)!=i || !pBt->autoVacuum) ){
        check_Append_செய்தி(&sCheck, "Page %u: never used", i);
      }
      எனில்( getPageReferenced(&sCheck, i)!=0 &&
         (PTRMAP_PAGENO(pBt, i)==i && pBt->autoVacuum) ){
        check_Append_செய்தி(&sCheck, "Page %u: pointer map referenced", i);
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
  எனில்( sCheck.nErr==0 ){
    sqlite3_str_reset(&sCheck.err_செய்தி);
    *pzOut = 0;
  }இல்லையெனில்{
    *pzOut = sqlite3StrAccumFinish(&sCheck.err_செய்தி);
  }
  /* Make sure this analysis did not leave any unref() pages. */
  ப்ரதிஜ்ஞா( nRef==sqlite3PagerRefcount(pBt->pPager) );
  sqlite3BtreeLeave(p);
  திரும்பு sCheck.rc;
}
#endif /* SQLITE_OMIT_INTEGRITY_CHECK */

/*
** Return the full pathname of the underlying database file.  Return
** an empty string if the database is in-memory or a TEMP database.
**
** The pager filename is invariant as long as the pager is
** open so it is safe to access without the BtShared mutex.
*/
மாறிலி எழுத்து *sqlite3BtreeGetFilename(Btree *p){
  ப்ரதிஜ்ஞா( p->pBt->pPager!=0 );
  திரும்பு sqlite3PagerFilename(p->pBt->pPager, 1);
}

/*
** Return the pathname of the journal file for this database. The return
** value of this routine is the same regardless of whether the journal file
** has been created or not.
**
** The pager journal filename is invariant as long as the pager is
** open so it is safe to access without the BtShared mutex.
*/
மாறிலி எழுத்து *sqlite3BtreeGetJournalname(Btree *p){
  ப்ரதிஜ்ஞா( p->pBt->pPager!=0 );
  திரும்பு sqlite3PagerJournalname(p->pBt->pPager);
}

/*
** Return one of SQLITE_TXN_NONE, SQLITE_TXN_READ, or SQLITE_TXN_WRITE
** to describe the current transaction state of Btree p.
*/
முழு sqlite3BtreeTxnState(Btree *p){
  ப்ரதிஜ்ஞா( p==0 || sqlite3_mutex_held(p->db->mutex) );
  திரும்பு p ? p->inTrans : 0;
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
முழு sqlite3BtreeCheckpoint(Btree *p, முழு eMode, முழு *pnLog, முழு *pnCkpt){
  முழு rc = SQLITE_OK;
  எனில்( p ){
    BtShared *pBt = p->pBt;
    sqlite3BtreeEnter(p);
    எனில்( pBt->inTransaction!=TRANS_NONE ){
      rc = SQLITE_LOCKED;
    }இல்லையெனில்{
      rc = sqlite3PagerCheckpoint(pBt->pPager, p->db, eMode, pnLog, pnCkpt);
    }
    sqlite3BtreeLeave(p);
  }
  திரும்பு rc;
}
#endif

/*
** Return true if there is currently a backup running on Btree p.
*/
முழு sqlite3BtreeIsInBackup(Btree *p){
  ப்ரதிஜ்ஞா( p );
  ப்ரதிஜ்ஞா( sqlite3_mutex_held(p->db->mutex) );
  திரும்பு p->nBackup!=0;
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
வெறுமை *sqlite3BtreeSchema(Btree *p, முழு nBytes, வெறுமை(*xFree)(வெறுமை *)){
  BtShared *pBt = p->pBt;
  ப்ரதிஜ்ஞா( nBytes==0 || nBytes==அளவு(Schema) );
  sqlite3BtreeEnter(p);
  எனில்( !pBt->pSchema && nBytes ){
    pBt->pSchema = sqlite3DbMallocZero(0, nBytes);
    pBt->xFreeSchema = xFree;
  }
  sqlite3BtreeLeave(p);
  திரும்பு pBt->pSchema;
}

/*
** Return SQLITE_LOCKED_SHAREDCACHE if another user of the same shared
** btree as the argument handle holds an exclusive lock on the
** sqlite_schema table. Otherwise SQLITE_OK.
*/
முழு sqlite3BtreeSchemaLocked(Btree *p){
  முழு rc;
  UNUSED_PARAMETER(p);  /* only used in DEBUG builds */
  ப்ரதிஜ்ஞா( sqlite3_mutex_held(p->db->mutex) );
  sqlite3BtreeEnter(p);
  rc = querySharedCacheTableLock(p, SCHEMA_வேர், READ_LOCK);
  ப்ரதிஜ்ஞா( rc==SQLITE_OK || rc==SQLITE_LOCKED_SHAREDCACHE );
  sqlite3BtreeLeave(p);
  திரும்பு rc;
}


#ifndef SQLITE_OMIT_SHARED_CACHE
/*
** Obtain a lock on the table whose root page is iTab.  The
** lock is a write lock if isWritelock is true or a read lock
** if it is false.
*/
முழு sqlite3BtreeLockTable(Btree *p, முழு iTab, u8 isWriteLock){
  முழு rc = SQLITE_OK;
  ப்ரதிஜ்ஞா( p->inTrans!=TRANS_NONE );
  எனில்( p->sharable ){
    u8 lockType = READ_LOCK + isWriteLock;
    ப்ரதிஜ்ஞா( READ_LOCK+1==WRITE_LOCK );
    ப்ரதிஜ்ஞா( isWriteLock==0 || isWriteLock==1 );

    sqlite3BtreeEnter(p);
    rc = querySharedCacheTableLock(p, iTab, lockType);
    எனில்( rc==SQLITE_OK ){
      rc = setSharedCacheTableLock(p, iTab, lockType);
    }
    sqlite3BtreeLeave(p);
  }
  திரும்பு rc;
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
முழு sqlite3BtreePutData(BtCursor *pCsr, u32 offset, u32 amt, வெறுமை *z){
  முழு rc;
  ப்ரதிஜ்ஞா( cursorOwnsBtShared(pCsr) );
  ப்ரதிஜ்ஞா( sqlite3_mutex_held(pCsr->pBtree->db->mutex) );
  ப்ரதிஜ்ஞா( pCsr->curFlags & BTCF_Incrblob );

  rc = restoreCursorPosition(pCsr);
  எனில்( rc!=SQLITE_OK ){
    திரும்பு rc;
  }
  ப்ரதிஜ்ஞா( pCsr->eState!=CURSOR_REQUIRESEEK );
  எனில்( pCsr->eState!=CURSOR_VALID ){
    திரும்பு SQLITE_ABORT;
  }

  /* Save the positions of all other cursors open on this table. This is
  ** required in case any of them are holding references to an xFetch
  ** version of the b-tree page modified by the accessPayload call below.
  **
  ** Note that pCsr must be open on a INTKEY table and saveCursorPosition()
  ** and hence saveAllCursors() cannot fail on a BTREE_INTKEY table, hence
  ** saveAllCursors can only return SQLITE_OK.
  */
  VVA_ONLY(rc =) saveAllCursors(pCsr->pBt, pCsr->pgno_வேர், pCsr);
  ப்ரதிஜ்ஞா( rc==SQLITE_OK );

  /* Check some assumptions:
  **   (a) the cursor is open for writing,
  **   (b) there is a read/write transaction open,
  **   (c) the connection holds a write-lock on the table (if required),
  **   (d) there are no conflicting read-locks, and
  **   (e) the cursor points at a valid row of an intKey table.
  */
  எனில்( (pCsr->curFlags & BTCF_WriteFlag)==0 ){
    திரும்பு SQLITE_READONLY;
  }
  ப்ரதிஜ்ஞா( (pCsr->pBt->btsFlags & BTS_READ_ONLY)==0
              && pCsr->pBt->inTransaction==TRANS_WRITE );
  ப்ரதிஜ்ஞா( hasSharedCacheTableLock(pCsr->pBtree, pCsr->pgno_வேர், 0, 2) );
  ப்ரதிஜ்ஞா( !hasReadConflicts(pCsr->pBtree, pCsr->pgno_வேர்) );
  ப்ரதிஜ்ஞா( pCsr->pPage->int_சாவி );

  திரும்பு accessPayload(pCsr, offset, amt, (unsigned எழுத்து *)z, 1);
}

/*
** Mark this cursor as an incremental blob cursor.
*/
வெறுமை sqlite3BtreeIncrblobCursor(BtCursor *pCur){
  pCur->curFlags |= BTCF_Incrblob;
  pCur->pBtree->hasIncrblobCur = 1;
}
#endif

/*
** Set both the "read version" (single byte at byte offset 18) and
** "write version" (single byte at byte offset 19) fields in the database
** header to iVersion.
*/
முழு sqlite3BtreeSetVersion(Btree *pBtree, முழு iVersion){
  BtShared *pBt = pBtree->pBt;
  முழு rc;                         /* Return code */

  ப்ரதிஜ்ஞா( iVersion==1 || iVersion==2 );

  /* If setting the version fields to 1, do not automatically open the
  ** WAL connection, even if the version fields are currently set to 2.
  */
  pBt->btsFlags &= ~BTS_NO_WAL;
  எனில்( iVersion==1 ) pBt->btsFlags |= BTS_NO_WAL;

  rc = sqlite3BtreeBeginTrans(pBtree, 0, 0);
  எனில்( rc==SQLITE_OK ){
    u8 *aData = pBt->pPage1->aData;
    எனில்( aData[18]!=(u8)iVersion || aData[19]!=(u8)iVersion ){
      rc = sqlite3BtreeBeginTrans(pBtree, 2, 0);
      எனில்( rc==SQLITE_OK ){
        rc = sqlite3PagerWrite(pBt->pPage1->pDbPage);
        எனில்( rc==SQLITE_OK ){
          aData[18] = (u8)iVersion;
          aData[19] = (u8)iVersion;
        }
      }
    }
  }

  pBt->btsFlags &= ~BTS_NO_WAL;
  திரும்பு rc;
}

/*
** Return true if the cursor has a hint specified.  This routine is
** only used from within assert() statements
*/
முழு sqlite3BtreeCursorHasHint(BtCursor *pCsr, unsigned முழு mask){
  திரும்பு (pCsr->hints & mask)!=0;
}

/*
** Return true if the given Btree is read-only.
*/
முழு sqlite3BtreeIsReadonly(Btree *p){
  திரும்பு (p->pBt->btsFlags & BTS_READ_ONLY)!=0;
}

/*
** Return the size of the header added to each page by this module.
*/
முழு sqlite3HeaderSizeBtree(வெறுமை){ திரும்பு ROUND8(அளவு(MemPage)); }

/*
** If no transaction is active and the database is not a temp-db, clear
** the in-memory pager cache.
*/
வெறுமை sqlite3BtreeClearCache(Btree *p){
  BtShared *pBt = p->pBt;
  எனில்( pBt->inTransaction==TRANS_NONE ){
    sqlite3PagerClearCache(pBt->pPager);
  }
}

#எனில் !defined(SQLITE_OMIT_SHARED_CACHE)
/*
** Return true if the Btree passed as the only argument is sharable.
*/
முழு sqlite3BtreeSharable(Btree *p){
  திரும்பு p->sharable;
}

/*
** Return the number of connections to the BtShared object accessed by
** the Btree handle passed as the only argument. For private caches
** this is always 1. For shared caches it may be 1 or greater.
*/
முழு sqlite3_Btree_Connection_மொத்தம்(Btree *p){
  testcase( p->sharable );
  திரும்பு p->pBt->nRef;
}
#endif

/* Re-enable GCC compiler warnings that were suppressed at the top
** of this source file to prevent annoying false-positives.
*/
#எனில் defined(__GNUC__) && __GNUC__>=11
# pragma GCC diagnostic எடு
#endif
